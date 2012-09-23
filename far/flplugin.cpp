/*
flplugin.cpp

Файловая панель - работа с плагинами
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "filelist.hpp"
#include "filepanels.hpp"
#include "history.hpp"
#include "ctrlobj.hpp"
#include "syslog.hpp"
#include "message.hpp"
#include "config.hpp"
#include "delete.hpp"
#include "datetime.hpp"
#include "dirmix.hpp"
#include "pathmix.hpp"
#include "panelmix.hpp"
#include "mix.hpp"

/*
   В стеке ФАРова панель не хранится - только плагиновые!
*/

void FileList::PushPlugin(HANDLE hPlugin,const wchar_t *HostFile)
{
	PluginsListItem* stItem = new PluginsListItem;
	stItem->hPlugin=hPlugin;
	stItem->strHostFile = HostFile;
	stItem->strPrevOriginalCurDir = strOriginalCurDir;
	strOriginalCurDir = strCurDir;
	stItem->Modified=FALSE;
	stItem->PrevViewMode=ViewMode;
	stItem->PrevSortMode=SortMode;
	stItem->PrevSortOrder=SortOrder;
	stItem->PrevNumericSort=NumericSort;
	stItem->PrevCaseSensitiveSort=CaseSensitiveSort;
	stItem->PrevViewSettings=ViewSettings;
	stItem->PrevDirectoriesFirst=DirectoriesFirst;
	PluginsList.Push(&stItem);
	++PluginPanelsCount;
}


int FileList::PopPlugin(int EnableRestoreViewMode)
{
	OpenPanelInfo Info={};

	if (PluginsList.Empty())
	{
		PanelMode=NORMAL_PANEL;
		return FALSE;
	}

	// указатель на плагин, с которого уходим
	PluginsListItem *PStack=*PluginsList.Last();

	// закрываем текущий плагин.
	PluginsList.Delete(PluginsList.Last());

	--PluginPanelsCount;

	CtrlObject->Plugins->ClosePanel(hPlugin);

	if (!PluginsList.Empty())
	{
		hPlugin=(*PluginsList.Last())->hPlugin;
		strOriginalCurDir=PStack->strPrevOriginalCurDir;

		if (EnableRestoreViewMode)
		{
			SetViewMode(PStack->PrevViewMode);
			SortMode=PStack->PrevSortMode;
			NumericSort=PStack->PrevNumericSort;
			CaseSensitiveSort=PStack->PrevCaseSensitiveSort;
			SortOrder=PStack->PrevSortOrder;
			DirectoriesFirst=PStack->PrevDirectoriesFirst;
		}

		if (PStack->Modified)
		{
			PluginPanelItem PanelItem={};
			string strSaveDir;
			apiGetCurrentDirectory(strSaveDir);

			if (FileNameToPluginItem(PStack->strHostFile,&PanelItem))
			{
				CtrlObject->Plugins->PutFiles(hPlugin,&PanelItem,1,FALSE,0);
			}
			else
			{
				PanelItem.FileName = xf_wcsdup(PointToName(PStack->strHostFile));
				CtrlObject->Plugins->DeleteFiles(hPlugin,&PanelItem,1,0);
				xf_free(PanelItem.FileName);
			}

			FarChDir(strSaveDir);
		}


		CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

		if (!(Info.Flags & OPIF_REALNAMES))
		{
			DeleteFileWithFolder(PStack->strHostFile);  // удаление файла от предыдущего плагина
		}
	}
	else
	{
		PanelMode=NORMAL_PANEL;
		hPlugin = nullptr;

		if (EnableRestoreViewMode)
		{
			SetViewMode(PStack->PrevViewMode);
			SortMode=PStack->PrevSortMode;
			NumericSort=PStack->PrevNumericSort;
			CaseSensitiveSort=PStack->PrevCaseSensitiveSort;
			SortOrder=PStack->PrevSortOrder;
			DirectoriesFirst=PStack->PrevDirectoriesFirst;
		}
	}

	delete PStack;

	if (EnableRestoreViewMode)
		CtrlObject->Cp()->RedrawKeyBar();

	return TRUE;
}

/*
	DefaultName - имя элемента на которое позиционируемся.
	Closed - панель закрывается, если в PrevDataList что-то есть - восстанавливаемчся оттуда.
	UsePrev - если востанавливаемся из PrevDataList, элемент для позиционирования брать оттуда же.
	Position - надо ли вообще устанавливать текущий элемент.
*/
void FileList::PopPrevData(const string& DefaultName,bool Closed,bool UsePrev,bool Position,bool SetDirectorySuccess)
{
    string strName(DefaultName);
	if (Closed && !PrevDataList.Empty())
	{
		PrevDataItem* Item=*PrevDataList.Last();
		PrevDataList.Delete(PrevDataList.Last());
		if (Item->PrevFileCount>0)
		{
			MoveSelection(ListData,FileCount,Item->PrevListData,Item->PrevFileCount);
			UpperFolderTopFile = Item->PrevTopFile;

			if (UsePrev)
				strName = Item->strPrevName;

			DeleteListData(Item->PrevListData,Item->PrevFileCount);
			delete Item;

			if (SelectedFirst)
				SortFileList(FALSE);
			else if (FileCount>0)
				SortFileList(TRUE);
		}
	}
	if (Position)
	{
		long Pos=FindFile(PointToName(strName));

		if (Pos!=-1)
			CurFile=Pos;
		else
			GoToFile(strName);

		CurTopFile=UpperFolderTopFile;
		UpperFolderTopFile=0;
		CorrectPosition();
	}
	/* $ 26.04.2001 DJ
	   доделка про несброс выделения при неудаче SetDirectory
	*/
	else if (SetDirectorySuccess)
		CurFile=CurTopFile=0;
}

int FileList::FileNameToPluginItem(const string& Name,PluginPanelItem *pi)
{
	string strTempDir = Name;

	if (!CutToSlash(strTempDir,true))
		return FALSE;

	FarChDir(strTempDir);
	ClearStruct(*pi);
	FAR_FIND_DATA_EX fdata;

	if (apiGetFindDataEx(Name, fdata))
	{
		FindDataExToPluginPanelItem(&fdata, pi);
		return TRUE;
	}

	return FALSE;
}


void FileList::FileListToPluginItem(FileListItem *fi,PluginPanelItem *pi)
{
	pi->FileName = xf_wcsdup(fi->strName);
	pi->AlternateFileName = xf_wcsdup(fi->strShortName);
	pi->FileSize=fi->FileSize;
	pi->AllocationSize=fi->AllocationSize;
	pi->FileAttributes=fi->FileAttr;
	pi->LastWriteTime=fi->WriteTime;
	pi->CreationTime=fi->CreationTime;
	pi->LastAccessTime=fi->AccessTime;
	pi->NumberOfLinks=fi->NumberOfLinks;
	pi->Flags=fi->UserFlags;

	if (fi->Selected)
		pi->Flags|=PPIF_SELECTED;

	pi->CustomColumnData=fi->CustomColumnData;
	pi->CustomColumnNumber=fi->CustomColumnNumber;
	pi->Description=fi->DizText; //BUGBUG???

	pi->UserData.Data=fi->UserData;
	pi->UserData.FreeData=fi->Callback;

	pi->CRC32=fi->CRC32;
	pi->Reserved[0]=pi->Reserved[1]=0;
	pi->Owner=fi->strOwner.IsEmpty()?nullptr:(wchar_t*)fi->strOwner.CPtr();
}

void FileList::FreePluginPanelItem(PluginPanelItem *pi)
{
	::FreePluginPanelItem(pi);
}

size_t FileList::FileListToPluginItem2(FileListItem *fi,FarGetPluginPanelItem *gpi)
{
	size_t size=ALIGN(sizeof(PluginPanelItem)),offset=size;
	size+=fi->CustomColumnNumber*sizeof(wchar_t*);
	size+=sizeof(wchar_t)*(fi->strName.GetLength()+1);
	size+=sizeof(wchar_t)*(fi->strShortName.GetLength()+1);
	for (size_t ii=0; ii<fi->CustomColumnNumber; ii++)
	{
		size+=fi->CustomColumnData[ii]?sizeof(wchar_t)*(wcslen(fi->CustomColumnData[ii])+1):0;
	}
	size+=fi->DizText?sizeof(wchar_t)*(wcslen(fi->DizText)+1):0;
	size+=fi->strOwner.IsEmpty()?0:sizeof(wchar_t)*(fi->strOwner.GetLength()+1);

	if (gpi)
	{
		if(gpi->Item && gpi->Size >= size)
		{
			char* data=(char*)(gpi->Item)+offset;

			gpi->Item->FileSize=fi->FileSize;
			gpi->Item->AllocationSize=fi->AllocationSize;
			gpi->Item->FileAttributes=fi->FileAttr;
			gpi->Item->LastWriteTime=fi->WriteTime;
			gpi->Item->CreationTime=fi->CreationTime;
			gpi->Item->LastAccessTime=fi->AccessTime;
			gpi->Item->NumberOfLinks=fi->NumberOfLinks;
			gpi->Item->Flags=fi->UserFlags;
			if (fi->Selected)
				gpi->Item->Flags|=PPIF_SELECTED;
			gpi->Item->CustomColumnNumber=fi->CustomColumnNumber;
			gpi->Item->CRC32=fi->CRC32;
			gpi->Item->Reserved[0]=gpi->Item->Reserved[1]=0;

			gpi->Item->CustomColumnData=(wchar_t**)data;
			data+=fi->CustomColumnNumber*sizeof(wchar_t*);

			gpi->Item->UserData.Data=fi->UserData;
			gpi->Item->UserData.FreeData=fi->Callback;

			gpi->Item->FileName=wcscpy((wchar_t*)data,fi->strName);
			data+=sizeof(wchar_t)*(fi->strName.GetLength()+1);

			gpi->Item->AlternateFileName=wcscpy((wchar_t*)data,fi->strShortName);
			data+=sizeof(wchar_t)*(fi->strShortName.GetLength()+1);

			for (size_t ii=0; ii<fi->CustomColumnNumber; ii++)
			{
				if (!fi->CustomColumnData[ii])
				{
					((const wchar_t**)(gpi->Item->CustomColumnData))[ii]=nullptr;
				}
				else
				{
					((const wchar_t**)(gpi->Item->CustomColumnData))[ii]=wcscpy((wchar_t*)data,fi->CustomColumnData[ii]);
					data+=sizeof(wchar_t)*(wcslen(fi->CustomColumnData[ii])+1);
				}
			}

			if (!fi->DizText)
			{
				gpi->Item->Description=nullptr;
			}
			else
			{
				gpi->Item->Description=wcscpy((wchar_t*)data,fi->DizText);
				data+=sizeof(wchar_t)*(wcslen(fi->DizText)+1);
			}


			if (fi->strOwner.IsEmpty())
			{
				gpi->Item->Owner=nullptr;
			}
			else
			{
				gpi->Item->Owner=wcscpy((wchar_t*)data,fi->strOwner);
			}
		}
	}
	return size;
}

void FileList::PluginToFileListItem(PluginPanelItem *pi,FileListItem *fi)
{
	fi->strName = pi->FileName;
	fi->strShortName = pi->AlternateFileName;
	fi->strOwner = pi->Owner;

	if (pi->Description)
	{
		fi->DizText=new wchar_t[StrLength(pi->Description)+1];
		wcscpy(fi->DizText, pi->Description);
		fi->DeleteDiz=TRUE;
	}
	else
		fi->DizText=nullptr;

	fi->FileSize=pi->FileSize;
	fi->AllocationSize=pi->AllocationSize;
	fi->FileAttr=pi->FileAttributes;
	fi->WriteTime=pi->LastWriteTime;
	fi->CreationTime=pi->CreationTime;
	fi->AccessTime=pi->LastAccessTime;
	fi->ChangeTime.dwHighDateTime = 0;
	fi->ChangeTime.dwLowDateTime = 0;
	fi->NumberOfLinks=pi->NumberOfLinks;
	fi->NumberOfStreams=1;
	fi->UserFlags=pi->Flags;

	fi->UserData=pi->UserData.Data;
	fi->Callback=pi->UserData.FreeData;

	if (pi->CustomColumnNumber>0)
	{
		fi->CustomColumnData=new wchar_t*[pi->CustomColumnNumber];

		for (size_t I=0; I<pi->CustomColumnNumber; I++)
			if (pi->CustomColumnData && pi->CustomColumnData[I])
			{
				fi->CustomColumnData[I]=new wchar_t[StrLength(pi->CustomColumnData[I])+1];
				wcscpy(fi->CustomColumnData[I],pi->CustomColumnData[I]);
			}
			else
			{
				fi->CustomColumnData[I]=new wchar_t[1];
				fi->CustomColumnData[I][0]=0;
			}
	}

	fi->CustomColumnNumber=pi->CustomColumnNumber;
	fi->CRC32=pi->CRC32;
}


HANDLE FileList::OpenPluginForFile(const string* FileName, DWORD FileAttr, OPENFILEPLUGINTYPE Type)
{
	HANDLE Result = nullptr;
	if(FileName && *FileName && !(FileAttr&FILE_ATTRIBUTE_DIRECTORY))
	{
		SetCurPath();
		_ALGO(SysLog(L"close AnotherPanel file"));
		CtrlObject->Cp()->GetAnotherPanel(this)->CloseFile();
		_ALGO(SysLog(L"call Plugins.OpenFilePlugin {"));
		Result = CtrlObject->Plugins->OpenFilePlugin(FileName, 0, Type);
		_ALGO(SysLog(L"}"));
	}
	return Result;
}


void FileList::CreatePluginItemList(PluginPanelItem *(&ItemList),int &ItemNumber,BOOL AddTwoDot)
{
	if (!ListData)
		return;

	long SaveSelPosition=GetSelPosition;
	long OldLastSelPosition=LastSelPosition;
	string strSelName;
	DWORD FileAttr;
	ItemNumber=0;
	ItemList=new PluginPanelItem[SelFileCount+1]();

	if (ItemList)
	{
		GetSelName(nullptr,FileAttr);

		while (GetSelName(&strSelName,FileAttr))
			if ((!(FileAttr & FILE_ATTRIBUTE_DIRECTORY) || !TestParentFolderName(strSelName))
			        && LastSelPosition>=0 && LastSelPosition<FileCount)
			{
				FileListToPluginItem(ListData[LastSelPosition],ItemList+ItemNumber);
				ItemNumber++;
			}

		if (AddTwoDot && !ItemNumber && (FileAttr & FILE_ATTRIBUTE_DIRECTORY)) // это про ".."
		{
			FileListToPluginItem(ListData[0],ItemList+ItemNumber);
			//ItemList->FindData.lpwszFileName = xf_wcsdup (ListData[0]->strName);
			//ItemList->FindData.dwFileAttributes=ListData[0]->FileAttr;
			ItemNumber++;
		}
	}

	LastSelPosition=OldLastSelPosition;
	GetSelPosition=SaveSelPosition;
}


void FileList::DeletePluginItemList(PluginPanelItem *(&ItemList),int &ItemNumber)
{
	PluginPanelItem *PItemList=ItemList;

	if (PItemList)
	{
		for (int I=0; I<ItemNumber; I++,PItemList++)
		{
			FreePluginPanelItem(PItemList);
		}

		delete[] ItemList;
	}
}


void FileList::PluginDelete()
{
	_ALGO(CleverSysLog clv(L"FileList::PluginDelete()"));
	PluginPanelItem *ItemList;
	int ItemNumber;
	SaveSelection();
	CreatePluginItemList(ItemList,ItemNumber);

	if (ItemList && ItemNumber>0)
	{
		if (CtrlObject->Plugins->DeleteFiles(hPlugin,ItemList,ItemNumber,0))
		{
			SetPluginModified();
			PutDizToPlugin(this,ItemList,ItemNumber,TRUE,FALSE,nullptr,&Diz);
		}

		DeletePluginItemList(ItemList,ItemNumber);
		Update(UPDATE_KEEP_SELECTION);
		Redraw();
		Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
		AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
		AnotherPanel->Redraw();
	}
}


void FileList::PutDizToPlugin(FileList *DestPanel,PluginPanelItem *ItemList,
                              int ItemNumber,int Delete,int Move,DizList *SrcDiz,
                              DizList *DestDiz)
{
	_ALGO(CleverSysLog clv(L"FileList::PutDizToPlugin()"));
	OpenPanelInfo Info;
	CtrlObject->Plugins->GetOpenPanelInfo(DestPanel->hPlugin,&Info);

	if (DestPanel->strPluginDizName.IsEmpty() && Info.DescrFilesNumber>0)
		DestPanel->strPluginDizName = Info.DescrFiles[0];

	if (((Opt.Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && IsDizDisplayed()) ||
	        Opt.Diz.UpdateMode==DIZ_UPDATE_ALWAYS) && !DestPanel->strPluginDizName.IsEmpty() &&
	        (!Info.HostFile || !*Info.HostFile || DestPanel->GetModalMode() ||
	         apiGetFileAttributes(Info.HostFile)!=INVALID_FILE_ATTRIBUTES))
	{
		CtrlObject->Cp()->LeftPanel->ReadDiz();
		CtrlObject->Cp()->RightPanel->ReadDiz();

		if (DestPanel->GetModalMode())
			DestPanel->ReadDiz();

		int DizPresent=FALSE;

		for (int I=0; I<ItemNumber; I++)
			if (ItemList[I].Flags & PPIF_PROCESSDESCR)
			{
				string strName = ItemList[I].FileName;
				string strShortName = ItemList[I].AlternateFileName;
				int Code;

				if (Delete)
					Code=DestDiz->DeleteDiz(strName,strShortName);
				else
				{
					Code=SrcDiz->CopyDiz(strName,strShortName,strName,strShortName,DestDiz);

					if (Code && Move)
						SrcDiz->DeleteDiz(strName,strShortName);
				}

				if (Code)
					DizPresent=TRUE;
			}

		if (DizPresent)
		{
			string strTempDir;

			if (FarMkTempEx(strTempDir) && apiCreateDirectory(strTempDir,nullptr))
			{
				string strSaveDir;
				apiGetCurrentDirectory(strSaveDir);
				string strDizName=strTempDir+L"\\"+DestPanel->strPluginDizName;
				DestDiz->Flush(L"", &strDizName);

				if (Move)
					SrcDiz->Flush(L"");

				PluginPanelItem PanelItem;

				if (FileNameToPluginItem(strDizName,&PanelItem))
					CtrlObject->Plugins->PutFiles(DestPanel->hPlugin,&PanelItem,1,FALSE,OPM_SILENT|OPM_DESCR);
				else if (Delete)
				{
					PluginPanelItem pi={};
					pi.FileName = xf_wcsdup(DestPanel->strPluginDizName);
					CtrlObject->Plugins->DeleteFiles(DestPanel->hPlugin,&pi,1,OPM_SILENT);
					xf_free(pi.FileName);
				}

				FarChDir(strSaveDir);
				DeleteFileWithFolder(strDizName);
			}
		}
	}
}


void FileList::PluginGetFiles(const wchar_t **DestPath,int Move)
{
	_ALGO(CleverSysLog clv(L"FileList::PluginGetFiles()"));
	PluginPanelItem *ItemList, *PList;
	int ItemNumber;
	SaveSelection();
	CreatePluginItemList(ItemList,ItemNumber);

	if (ItemList && ItemNumber>0)
	{
		int GetCode=CtrlObject->Plugins->GetFiles(hPlugin,ItemList,ItemNumber,Move!=0,DestPath,0);

		if ((Opt.Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && IsDizDisplayed()) ||
		        Opt.Diz.UpdateMode==DIZ_UPDATE_ALWAYS)
		{
			DizList DestDiz;
			int DizFound=FALSE;
			PList=ItemList;

			for (int I=0; I<ItemNumber; I++,PList++)
				if (PList->Flags & PPIF_PROCESSDESCR)
				{
					if (!DizFound)
					{
						CtrlObject->Cp()->LeftPanel->ReadDiz();
						CtrlObject->Cp()->RightPanel->ReadDiz();
						DestDiz.Read(*DestPath);
						DizFound=TRUE;
					}

					string strName = PList->FileName;
					string strShortName = PList->AlternateFileName;
					CopyDiz(strName,strShortName,strName,strName,&DestDiz);
				}

			DestDiz.Flush(*DestPath);
		}

		if (GetCode==1)
		{
			if (!ReturnCurrentFile)
				ClearSelection();

			if (Move)
			{
				SetPluginModified();
				PutDizToPlugin(this,ItemList,ItemNumber,TRUE,FALSE,nullptr,&Diz);
			}
		}
		else if (!ReturnCurrentFile)
			PluginClearSelection(ItemList,ItemNumber);

		DeletePluginItemList(ItemList,ItemNumber);
		Update(UPDATE_KEEP_SELECTION);
		Redraw();
		Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
		AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
		AnotherPanel->Redraw();
	}
}


void FileList::PluginToPluginFiles(int Move)
{
	_ALGO(CleverSysLog clv(L"FileList::PluginToPluginFiles()"));
	PluginPanelItem *ItemList;
	int ItemNumber;
	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
	string strTempDir;

	if (AnotherPanel->GetMode()!=PLUGIN_PANEL)
		return;

	FileList *AnotherFilePanel=(FileList *)AnotherPanel;

	if (!FarMkTempEx(strTempDir))
		return;

	SaveSelection();
	apiCreateDirectory(strTempDir,nullptr);
	CreatePluginItemList(ItemList,ItemNumber);

	if (ItemList && ItemNumber>0)
	{
		const wchar_t *lpwszTempDir=strTempDir;
		int PutCode=CtrlObject->Plugins->GetFiles(hPlugin,ItemList,ItemNumber,FALSE,&lpwszTempDir,OPM_SILENT);
		strTempDir=lpwszTempDir;

		if (PutCode==1 || PutCode==2)
		{
			string strSaveDir;
			apiGetCurrentDirectory(strSaveDir);
			FarChDir(strTempDir);
			PutCode=CtrlObject->Plugins->PutFiles(AnotherFilePanel->hPlugin,ItemList,ItemNumber,FALSE,0);

			if (PutCode==1 || PutCode==2)
			{
				if (!ReturnCurrentFile)
					ClearSelection();

				AnotherPanel->SetPluginModified();
				PutDizToPlugin(AnotherFilePanel,ItemList,ItemNumber,FALSE,FALSE,&Diz,&AnotherFilePanel->Diz);

				if (Move)
					if (CtrlObject->Plugins->DeleteFiles(hPlugin,ItemList,ItemNumber,OPM_SILENT))
					{
						SetPluginModified();
						PutDizToPlugin(this,ItemList,ItemNumber,TRUE,FALSE,nullptr,&Diz);
					}
			}
			else if (!ReturnCurrentFile)
				PluginClearSelection(ItemList,ItemNumber);

			FarChDir(strSaveDir);
		}

		DeleteDirTree(strTempDir);
		DeletePluginItemList(ItemList,ItemNumber);
		Update(UPDATE_KEEP_SELECTION);
		Redraw();

		if (PanelMode==PLUGIN_PANEL)
			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
		else
			AnotherPanel->Update(UPDATE_KEEP_SELECTION);

		AnotherPanel->Redraw();
	}
}

class PluginsTree: public Tree<Plugin*>
{
	public:
		PluginsTree(){}
		~PluginsTree(){clear();}
		long compare(Node<Plugin*>* first,Plugin** second) {return reinterpret_cast<char*>(*first->data)-reinterpret_cast<char*>(*second);}
};

void FileList::PluginHostGetFiles()
{
	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
	string strDestPath;
	string strSelName;
	DWORD FileAttr;
	SaveSelection();
	GetSelName(nullptr,FileAttr);

	if (!GetSelName(&strSelName,FileAttr))
		return;

	AnotherPanel->GetCurDir(strDestPath);

	if (((!AnotherPanel->IsVisible() || AnotherPanel->GetType()!=FILE_PANEL) &&
	        !SelFileCount) || strDestPath.IsEmpty())
	{
		strDestPath = PointToName(strSelName);
		// SVS: А зачем здесь велся поиск точки с начала?
		size_t pos;

		if (strDestPath.RPos(pos,L'.'))
			strDestPath.SetLength(pos);
	}

	int ExitLoop=FALSE;
	GetSelName(nullptr,FileAttr);
	PluginsTree tree;

	while (!ExitLoop && GetSelName(&strSelName,FileAttr))
	{
		HANDLE hCurPlugin;

		if ((hCurPlugin=OpenPluginForFile(&strSelName,FileAttr, OFP_EXTRACT))!=nullptr &&
		        hCurPlugin!=PANEL_STOP)
		{
			PluginHandle *ph = (PluginHandle *)hCurPlugin;
			int OpMode=OPM_TOPLEVEL;
			if(tree.query(&ph->pPlugin)) OpMode|=OPM_SILENT;

			PluginPanelItem *ItemList;
			size_t ItemNumber;
			_ALGO(SysLog(L"call Plugins.GetFindData()"));

			if (CtrlObject->Plugins->GetFindData(hCurPlugin,&ItemList,&ItemNumber,0))
			{
				_ALGO(SysLog(L"call Plugins.GetFiles()"));
				const wchar_t *lpwszDestPath=strDestPath;
				ExitLoop=CtrlObject->Plugins->GetFiles(hCurPlugin,ItemList,ItemNumber,FALSE,&lpwszDestPath,OpMode)!=1;
				strDestPath=lpwszDestPath;

				if (!ExitLoop)
				{
					_ALGO(SysLog(L"call ClearLastGetSelection()"));
					ClearLastGetSelection();
				}

				_ALGO(SysLog(L"call Plugins.FreeFindData()"));
				CtrlObject->Plugins->FreeFindData(hCurPlugin,ItemList,ItemNumber);
				tree.insert(new Plugin*(ph->pPlugin));
			}

			_ALGO(SysLog(L"call Plugins.ClosePanel"));
			CtrlObject->Plugins->ClosePanel(hCurPlugin);
		}
	}

	Update(UPDATE_KEEP_SELECTION);
	Redraw();
	AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	AnotherPanel->Redraw();
}


void FileList::PluginPutFilesToNew()
{
	_ALGO(CleverSysLog clv(L"FileList::PluginPutFilesToNew()"));
	//_ALGO(SysLog(L"FileName='%s'",(FileName?FileName:"(nullptr)")));
	_ALGO(SysLog(L"call Plugins.OpenFilePlugin(nullptr, 0)"));
	HANDLE hNewPlugin=CtrlObject->Plugins->OpenFilePlugin(nullptr, 0, OFP_CREATE);

	if (hNewPlugin && hNewPlugin!=PANEL_STOP)
	{
		_ALGO(SysLog(L"Create: FileList TmpPanel, FileCount=%d",FileCount));
		FileList TmpPanel;
		TmpPanel.SetPluginMode(hNewPlugin,L"");  // SendOnFocus??? true???
		TmpPanel.SetModalMode(TRUE);
		int PrevFileCount=FileCount;
		/* $ 12.04.2002 IS
		   Если PluginPutFilesToAnother вернула число, отличное от 2, то нужно
		   попробовать установить курсор на созданный файл.
		*/
		int rc=PluginPutFilesToAnother(FALSE,&TmpPanel);

		if (rc!=2 && FileCount==PrevFileCount+1)
		{
			int LastPos = 0;
			/* Место, где вычисляются координаты вновь созданного файла
			   Позиционирование происходит на файл с максимальной датой
			   создания файла. Посему, если какой-то злобный буратино поимел
			   в текущем каталоге файло с датой создания поболее текущей,
			   то корректного позиционирования не произойдет!
			*/
			FileListItem *PtrListData, *PtrLastPos = nullptr;

			for (int i = 0; i < FileCount; i++)
			{
				PtrListData = ListData[i];
				if ((PtrListData->FileAttr & FILE_ATTRIBUTE_DIRECTORY) == 0)
				{
					if (PtrLastPos)
					{
						if (FileTimeDifference(&PtrListData->CreationTime, &PtrLastPos->CreationTime) > 0)
						{
							LastPos = i;
							PtrLastPos = PtrListData;
						}
					}
					else
					{
						LastPos = i;
						PtrLastPos = PtrListData;
					}
				}
			}

			if (PtrLastPos)
			{
				CurFile = LastPos;
				Redraw();
			}
		}
	}
}


/* $ 12.04.2002 IS
     PluginPutFilesToAnother теперь int - возвращает то, что возвращает
     PutFiles:
     -1 - прервано пользовтелем
      0 - неудача
      1 - удача
      2 - удача, курсор принудительно установлен на файл и заново его
          устанавливать не нужно (см. PluginPutFilesToNew)
*/
int FileList::PluginPutFilesToAnother(int Move,Panel *AnotherPanel)
{
	if (AnotherPanel->GetMode()!=PLUGIN_PANEL)
		return 0;

	FileList *AnotherFilePanel=(FileList *)AnotherPanel;
	PluginPanelItem *ItemList;
	int ItemNumber,PutCode=0;
	SaveSelection();
	CreatePluginItemList(ItemList,ItemNumber);

	if (ItemList && ItemNumber>0)
	{
		SetCurPath();
		_ALGO(SysLog(L"call Plugins.PutFiles"));
		PutCode=CtrlObject->Plugins->PutFiles(AnotherFilePanel->hPlugin,ItemList,ItemNumber,Move!=0,0);

		if (PutCode==1 || PutCode==2)
		{
			if (!ReturnCurrentFile)
			{
				_ALGO(SysLog(L"call ClearSelection()"));
				ClearSelection();
			}

			_ALGO(SysLog(L"call PutDizToPlugin"));
			PutDizToPlugin(AnotherFilePanel,ItemList,ItemNumber,FALSE,Move,&Diz,&AnotherFilePanel->Diz);
			AnotherPanel->SetPluginModified();
		}
		else if (!ReturnCurrentFile)
			PluginClearSelection(ItemList,ItemNumber);

		_ALGO(SysLog(L"call DeletePluginItemList"));
		DeletePluginItemList(ItemList,ItemNumber);
		Update(UPDATE_KEEP_SELECTION);
		Redraw();

		if (AnotherPanel==CtrlObject->Cp()->GetAnotherPanel(this))
		{
			AnotherPanel->Update(UPDATE_KEEP_SELECTION);
			AnotherPanel->Redraw();
		}
	}

	return PutCode;
}


void FileList::GetOpenPanelInfo(OpenPanelInfo *Info)
{
	_ALGO(CleverSysLog clv(L"FileList::GetOpenPanelInfo()"));
	//_ALGO(SysLog(L"FileName='%s'",(FileName?FileName:"(nullptr)")));
	ClearStruct(*Info);

	if (PanelMode==PLUGIN_PANEL)
		CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,Info);
}


/*
   Функция для вызова команды "Архивные команды" (Shift-F3)
*/
void FileList::ProcessHostFile()
{
	_ALGO(CleverSysLog clv(L"FileList::ProcessHostFile()"));

	//_ALGO(SysLog(L"FileName='%s'",(FileName?FileName:"(nullptr)")));
	if (FileCount>0 && SetCurPath())
	{
		int Done=FALSE;
		SaveSelection();

		if (PanelMode==PLUGIN_PANEL && !(*PluginsList.Last())->strHostFile.IsEmpty())
		{
			PluginPanelItem *ItemList;
			int ItemNumber;
			_ALGO(SysLog(L"call CreatePluginItemList"));
			CreatePluginItemList(ItemList,ItemNumber);
			_ALGO(SysLog(L"call Plugins.ProcessHostFile"));
			Done=CtrlObject->Plugins->ProcessHostFile(hPlugin,ItemList,ItemNumber,0);

			if (Done)
				SetPluginModified();
			else
			{
				if (!ReturnCurrentFile)
					PluginClearSelection(ItemList,ItemNumber);

				Redraw();
			}

			_ALGO(SysLog(L"call DeletePluginItemList"));
			DeletePluginItemList(ItemList,ItemNumber);

			if (Done)
				ClearSelection();
		}
		else
		{
			size_t SCount=GetRealSelCount();

			if (SCount > 0)
			{
				for (int I=0; I < FileCount; ++I)
				{
					if (ListData[I]->Selected)
					{
						Done=ProcessOneHostFile(I);

						if (Done == 1)
							Select(ListData[I],0);
						else if (Done == -1)
							continue;
						else       // Если ЭТО убрать, то... будем жать ESC до потере пулься
							break;   //
					}
				}

				if (SelectedFirst)
					SortFileList(TRUE);
			}
			else
			{
				if ((Done=ProcessOneHostFile(CurFile)) == 1)
					ClearSelection();
			}
		}

		if (Done)
		{
			Update(UPDATE_KEEP_SELECTION);
			Redraw();
			Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
			AnotherPanel->Redraw();
		}
	}
}

/*
  Обработка одного хост-файла.
  Return:
    -1 - Этот файл никаким плагином не поддержан
     0 - Плагин вернул FALSE
     1 - Плагин вернул TRUE
*/
int FileList::ProcessOneHostFile(int Idx)
{
	_ALGO(CleverSysLog clv(L"FileList::ProcessOneHostFile()"));
	int Done=-1;
	_ALGO(SysLog(L"call OpenPluginForFile([Idx=%d] '%s')",Idx,ListData[Idx]->strName.CPtr()));
	HANDLE hNewPlugin=OpenPluginForFile(&ListData[Idx]->strName,ListData[Idx]->FileAttr, OFP_COMMANDS);

	if (hNewPlugin && hNewPlugin!=PANEL_STOP)
	{
		PluginPanelItem *ItemList;
		size_t ItemNumber;
		_ALGO(SysLog(L"call Plugins.GetFindData"));

		if (CtrlObject->Plugins->GetFindData(hNewPlugin,&ItemList,&ItemNumber,OPM_TOPLEVEL))
		{
			_ALGO(SysLog(L"call Plugins.ProcessHostFile"));
			Done=CtrlObject->Plugins->ProcessHostFile(hNewPlugin,ItemList,ItemNumber,OPM_TOPLEVEL);
			_ALGO(SysLog(L"call Plugins.FreeFindData"));
			CtrlObject->Plugins->FreeFindData(hNewPlugin,ItemList,ItemNumber);
		}

		_ALGO(SysLog(L"call Plugins.ClosePanel"));
		CtrlObject->Plugins->ClosePanel(hNewPlugin);
	}

	return Done;
}



void FileList::SetPluginMode(HANDLE hPlugin,const wchar_t *PluginFile,bool SendOnFocus)
{
	if (PanelMode!=PLUGIN_PANEL)
	{
		CtrlObject->FolderHistory->AddToHistory(strCurDir);
	}

	PushPlugin(hPlugin,PluginFile);
	FileList::hPlugin=hPlugin;
	PanelMode=PLUGIN_PANEL;

	if (SendOnFocus)
		SetFocus();

	OpenPanelInfo Info;
	CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

	if (Info.StartPanelMode)
		SetViewMode(VIEW_0+Info.StartPanelMode-L'0');

	CtrlObject->Cp()->RedrawKeyBar();

	if (Info.StartSortMode)
	{
		SortMode=Info.StartSortMode-(SM_UNSORTED-UNSORTED);
		SortOrder=Info.StartSortOrder ? -1:1;
	}

	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

	if (AnotherPanel->GetType()!=FILE_PANEL)
	{
		AnotherPanel->Update(UPDATE_KEEP_SELECTION);
		AnotherPanel->Redraw();
	}
}

void FileList::PluginGetPanelInfo(PanelInfo &Info)
{
	CorrectPosition();
	Info.CurrentItem=CurFile;
	Info.TopPanelItem=CurTopFile;
	if(ShowShortNames) Info.Flags|=PFLAGS_ALTERNATIVENAMES;
	Info.ItemsNumber=FileCount;
	Info.SelectedItemsNumber=ListData?GetSelCount():0;
}

size_t FileList::PluginGetPanelItem(int ItemNumber,FarGetPluginPanelItem *Item)
{
	size_t result=0;

	if (ListData && ItemNumber<FileCount)
	{
		result=FileListToPluginItem2(ListData[ItemNumber],Item);
	}

	return result;
}

size_t FileList::PluginGetSelectedPanelItem(int ItemNumber,FarGetPluginPanelItem *Item)
{
	size_t result=0;

	if (ListData && ItemNumber<FileCount)
	{
		if (ItemNumber==CacheSelIndex)
		{
			result=FileListToPluginItem2(ListData[CacheSelPos],Item);
		}
		else
		{
			if (ItemNumber<CacheSelIndex) CacheSelIndex=-1;

			int CurSel=CacheSelIndex,StartValue=CacheSelIndex>=0?CacheSelPos+1:0;

			for (int i=StartValue; i<FileCount; i++)
			{
				if (ListData[i]->Selected)
					CurSel++;

				if (CurSel==ItemNumber)
				{
					result=FileListToPluginItem2(ListData[i],Item);
					CacheSelIndex=ItemNumber;
					CacheSelPos=i;
					break;
				}
			}

			if (CurSel==-1 && !ItemNumber)
			{
				result=FileListToPluginItem2(ListData[CurFile],Item);
				CacheSelIndex=-1;
			}
		}
	}

	return result;
}

void FileList::PluginGetColumnTypesAndWidths(string& strColumnTypes,string& strColumnWidths)
{
	ViewSettingsToText(ViewSettings.ColumnType,ViewSettings.ColumnWidth,ViewSettings.ColumnWidthType,
	                   ViewSettings.ColumnCount,strColumnTypes,strColumnWidths);
}

void FileList::PluginBeginSelection()
{
	SaveSelection();
}

void FileList::PluginSetSelection(int ItemNumber,bool Selection)
{
	Select(ListData[ItemNumber],Selection);
}

void FileList::PluginClearSelection(int SelectedItemNumber)
{
	if (ListData && SelectedItemNumber<FileCount)
	{
		if (SelectedItemNumber<=CacheSelClearIndex)
		{
			CacheSelClearIndex=-1;
		}

		int CurSel=CacheSelClearIndex,StartValue=CacheSelClearIndex>=0?CacheSelClearPos+1:0;

		for (int i=StartValue; i<FileCount; i++)
		{
			if (ListData[i]->Selected)
			{
				CurSel++;
			}

			if (CurSel==SelectedItemNumber)
			{
				Select(ListData[i],FALSE);
				CacheSelClearIndex=SelectedItemNumber;
				CacheSelClearPos=i;
				break;
			}
		}
	}
}

void FileList::PluginEndSelection()
{
	if (SelectedFirst)
	{
		SortFileList(TRUE);
	}
}

void FileList::ProcessPluginCommand()
{
	_ALGO(CleverSysLog clv(L"FileList::ProcessPluginCommand"));
	_ALGO(SysLog(L"PanelMode=%s",(PanelMode==PLUGIN_PANEL?"PLUGIN_PANEL":"NORMAL_PANEL")));
	int Command=PluginCommand;
	PluginCommand=-1;

	if (PanelMode==PLUGIN_PANEL)
		switch (Command)
		{
			case FCTL_CLOSEPANEL:
				_ALGO(SysLog(L"Command=FCTL_CLOSEPANEL"));
				SetCurDir(strPluginParam,TRUE);

				if (strPluginParam.IsEmpty())
					Update(UPDATE_KEEP_SELECTION);

				Redraw();
				break;
		}
}

void FileList::SetPluginModified()
{
	if(PluginsList.Last())
	{
		(*PluginsList.Last())->Modified=TRUE;
	}
}


HANDLE FileList::GetPluginHandle()
{
	return(hPlugin);
}


int FileList::ProcessPluginEvent(int Event,void *Param)
{
	if (PanelMode==PLUGIN_PANEL)
		return(CtrlObject->Plugins->ProcessEvent(hPlugin,Event,Param));

	return FALSE;
}


void FileList::PluginClearSelection(PluginPanelItem *ItemList,int ItemNumber)
{
	SaveSelection();
	int FileNumber=0,PluginNumber=0;

	while (PluginNumber<ItemNumber)
	{
		PluginPanelItem *CurPluginPtr=ItemList+PluginNumber;

		if (!(CurPluginPtr->Flags & PPIF_SELECTED))
		{
			while (StrCmpI(CurPluginPtr->FileName,ListData[FileNumber]->strName))
				if (++FileNumber>=FileCount)
					return;

			Select(ListData[FileNumber++],0);
		}

		PluginNumber++;
	}
}
