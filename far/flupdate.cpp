/*
flupdate.cpp

Файловая панель - чтение имен файлов
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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
#include "flink.hpp"
#include "colors.hpp"
#include "lang.hpp"
#include "filepanels.hpp"
#include "cmdline.hpp"
#include "filefilter.hpp"
#include "hilight.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "TaskBar.hpp"
#include "cddrv.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "config.hpp"
#include "fileowner.hpp"
#include "delete.hpp"
#include "pathmix.hpp"
#include "network.hpp"
#include "dirmix.hpp"
#include "strmix.hpp"
#include "mix.hpp"

// Флаги для ReadDiz()
enum ReadDizFlags
{
	RDF_NO_UPDATE         = 0x00000001UL,
};

void FileList::Update(int Mode)
{
	_ALGO(CleverSysLog clv(L"FileList::Update"));
	_ALGO(SysLog(L"(Mode=[%d/0x%08X] %s)",Mode,Mode,(Mode==UPDATE_KEEP_SELECTION?L"UPDATE_KEEP_SELECTION":L"")));

	if (EnableUpdate)
		switch (PanelMode)
		{
			case NORMAL_PANEL:
				ReadFileNames(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE,Mode & UPDATE_DRAW_MESSAGE);
				break;
			case PLUGIN_PANEL:
			{
				OpenPluginInfo Info;
				CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
				ProcessPluginCommand();

				if (PanelMode!=PLUGIN_PANEL)
					ReadFileNames(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE,Mode & UPDATE_DRAW_MESSAGE);
				else if ((Info.Flags & OPIF_REALNAMES) ||
				         CtrlObject->Cp()->GetAnotherPanel(this)->GetMode()==PLUGIN_PANEL ||
				         (Mode & UPDATE_SECONDARY)==0)
					UpdatePlugin(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE);
			}
			ProcessPluginCommand();
			break;
		}

	LastUpdateTime=clock();
}

void FileList::UpdateIfRequired()
{
	if (UpdateRequired && !UpdateDisabled)
	{
		UpdateRequired = FALSE;
		Update(UpdateRequiredMode | UPDATE_IGNORE_VISIBLE);
	}
}

void ReadFileNamesMsg(const wchar_t *Msg)
{
	Message(0,0,MSG(MReadingTitleFiles),Msg);
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	preRedrawItem.Param.Param1=(void*)Msg;
	PreRedraw.SetParam(preRedrawItem.Param);
}

static void PR_ReadFileNamesMsg()
{
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	ReadFileNamesMsg((wchar_t *)preRedrawItem.Param.Param1);
}


// ЭТО ЕСТЬ УЗКОЕ МЕСТО ДЛЯ СКОРОСТНЫХ ХАРАКТЕРИСТИК Far Manager
// при считывании дирректории

void FileList::ReadFileNames(int KeepSelection, int IgnoreVisible, int DrawMessage)
{
	TPreRedrawFuncGuard preRedrawFuncGuard(PR_ReadFileNamesMsg);
	TaskBar TB;

	if (!IsVisible() && !IgnoreVisible)
	{
		UpdateRequired=TRUE;
		UpdateRequiredMode=KeepSelection;
		return;
	}

	UpdateRequired=FALSE;
	AccessTimeUpdateRequired=FALSE;
	DizRead=FALSE;
	HANDLE FindHandle;
	FAR_FIND_DATA_EX fdata;
	FileListItem *CurPtr=0,**OldData=0;
	string strCurName, strNextCurName;
	int OldFileCount=0;
	int Done;
	int I;
	clock_t StartTime=clock();
	CloseChangeNotification();

	if (this!=CtrlObject->Cp()->LeftPanel && this!=CtrlObject->Cp()->RightPanel)
		return;

	string strSaveDir;
	apiGetCurrentDirectory(strSaveDir);
	{
		string strOldCurDir = strCurDir;

		if (!SetCurPath())
		{
			FlushInputBuffer(); // Очистим буффер ввода, т.к. мы уже можем быть в другом месте...

			if (StrCmp(strCurDir, strOldCurDir) == 0) //?? i??
			{
				GetPathRoot(strOldCurDir,strOldCurDir);

				if (!apiIsDiskInDrive(strOldCurDir))
					IfGoHome(strOldCurDir.At(0));

				/* При смене каталога путь не изменился */
			}

			return;
		}
	}
	SortGroupsRead=FALSE;

	if (GetFocus())
		CtrlObject->CmdLine->SetCurDir(strCurDir);

	LastCurFile=-1;
	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
	AnotherPanel->QViewDelTempName();
	int PrevSelFileCount=SelFileCount;
	SelFileCount=0;
	SelFileSize=0;
	TotalFileCount=0;
	TotalFileSize=0;
	CacheSelIndex=-1;

	if (Opt.ShowPanelFree)
	{
		unsigned __int64 TotalSize,TotalFree;

		if (!apiGetDiskSize(strCurDir,&TotalSize,&TotalFree,&FreeDiskSize))
			FreeDiskSize=0;
	}

	if (FileCount>0)
	{
		strCurName = ListData[CurFile]->strName;

		if (ListData[CurFile]->Selected)
		{
			for (I=CurFile+1; I < FileCount; I++)
			{
				CurPtr = ListData[I];

				if (!CurPtr->Selected)
				{
					strNextCurName = CurPtr->strName;
					break;
				}
			}
		}
	}

	if (KeepSelection || PrevSelFileCount>0)
	{
		OldData=ListData;
		OldFileCount=FileCount;
	}
	else
		DeleteListData(ListData,FileCount);

	ListData=NULL;
	int ReadOwners=IsColumnDisplayed(OWNER_COLUMN);
	int ReadPacked=IsColumnDisplayed(PACKED_COLUMN);
	int ReadNumLinks=IsColumnDisplayed(NUMLINK_COLUMN);
	int ReadNumStreams=IsColumnDisplayed(NUMSTREAMS_COLUMN);
	int ReadStreamsSize=IsColumnDisplayed(STREAMSSIZE_COLUMN);
	string strComputerName;

	if (ReadOwners)
	{
		CurPath2ComputerName(strCurDir, strComputerName);
		// сбросим кэш SID`ов
		SIDCacheFlush();
	}

	SetLastError(0);
	//BUGBUG!!!
	Done=((FindHandle=apiFindFirstFile(L"*",&fdata))==INVALID_HANDLE_VALUE);
	int AllocatedCount=0;
	FileListItem *NewPtr;
	// сформируем заголовок вне цикла
	wchar_t Title[2048];
	int TitleLength=Min((int)X2-X1-1,(int)(countof(Title))-1);
	//wmemset(Title,0x0CD,TitleLength); //BUGBUG
	//Title[TitleLength]=0;
	MakeSeparator(TitleLength, Title, 9, NULL);
	BOOL IsShowTitle=FALSE;
	BOOL NeedHighlight=Opt.Highlight && PanelMode != PLUGIN_PANEL;

	if (Filter==NULL)
		Filter=new FileFilter(this,FFT_PANEL);

	//Рефреш текущему времени для фильтра перед началом операции
	Filter->UpdateCurrentTime();
	CtrlObject->HiFiles->UpdateCurrentTime();
	bool bDotExists=false, bTwoDotsExists=false;
	bool bDotSeen=false, bTwoDotsSeen=false;
	bool bCurDirRoot=IsLocalRootPath(strCurDir)||IsLocalPrefixRootPath(strCurDir)||IsLocalVolumeRootPath(strCurDir);
	FILETIME TwoDotsTimes[3]={0};
	string TwoDotsOwner;

	if (!Done)
	{
		string strTemp(NTPath(strCurDir).Str);
		DWORD dw;
		strTemp += L"\\.";
		dw = apiGetFileAttributes(strTemp);
		bDotExists = dw!=INVALID_FILE_ATTRIBUTES && dw&FILE_ATTRIBUTE_DIRECTORY;
		strTemp += L".";
		dw = apiGetFileAttributes(strTemp);
		bTwoDotsExists = dw!=INVALID_FILE_ATTRIBUTES && dw&FILE_ATTRIBUTE_DIRECTORY;
	}

	for (FileCount=0; !Done;)
	{
		// Весь смысл этого ужаса в том что на FAT можно создать папки "." и "..".
		// Поэтому извращаемся вот по такой системе:
		// если существует реальная папка "." или ".." (проверка сверху)
		// то если мы не в корне диска (так как только в корне нету обычных "." и "..")
		// то скипаем первое вхождение "." или ".." так как это обычные, а потом не скипаем
		// так как это уже те которые буратино создал.
		// Есть один прокол, если на замапленом диске есть такие папки и к тому же листинг такого
		// диска возвращает обычные тоже, то уж увольте но это просто у вас в глазах двоится/троится будет,
		// ни чем помочь не могу. Смерть фату и всё такое.
		if (fdata.strFileName.At(0) == L'.' && fdata.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			if (fdata.strFileName.At(1) == L'.' && fdata.strFileName.At(2) == 0)
			{
				if (!bTwoDotsExists || (!bCurDirRoot && !bTwoDotsSeen))
				{
					bTwoDotsSeen=true;
					TwoDotsTimes[0]=fdata.ftCreationTime;
					TwoDotsTimes[1]=fdata.ftLastAccessTime;
					TwoDotsTimes[2]=fdata.ftLastWriteTime;

					if (ReadOwners)
					{
						GetFileOwner(strComputerName,fdata.strFileName,TwoDotsOwner);
					}

					Done=!apiFindNextFile(FindHandle,&fdata);
					continue;
				}
			}
			else if (fdata.strFileName.At(1) == 0)
			{
				if (!bDotExists || (!bCurDirRoot && !bDotSeen))
				{
					bDotSeen=true;
					Done=!apiFindNextFile(FindHandle,&fdata);
					continue;
				}
			}
		}

		if ((Opt.ShowHidden || (fdata.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))==0) && Filter->FileInFilter(&fdata))
		{
			if (FileCount>=AllocatedCount)
			{
				AllocatedCount=AllocatedCount+256+AllocatedCount/4;
				FileListItem **pTemp;

				if ((pTemp=(FileListItem **)xf_realloc(ListData,AllocatedCount*sizeof(*ListData)))==NULL)
					break;

				ListData=pTemp;
			}

			ListData[FileCount] = new FileListItem;
			ListData[FileCount]->Clear();
			NewPtr=ListData[FileCount];
			NewPtr->FileAttr = fdata.dwFileAttributes;
			NewPtr->CreationTime = fdata.ftCreationTime;
			NewPtr->AccessTime = fdata.ftLastAccessTime;
			NewPtr->WriteTime = fdata.ftLastWriteTime;
			NewPtr->UnpSize = fdata.nFileSize;
			NewPtr->strName = fdata.strFileName;
			NewPtr->strShortName = fdata.strAlternateFileName;
			NewPtr->Position=FileCount++;
			NewPtr->NumberOfLinks=1;

			if (fdata.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			{
				NewPtr->ReparseTag=fdata.dwReserved0; //MSDN
			}

			if ((fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				TotalFileSize += NewPtr->UnpSize;
				bool Compressed=false;

				if (ReadPacked && ((fdata.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) || (fdata.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)))
				{
					if (apiGetCompressedFileSize(fdata.strFileName,NewPtr->PackSize))
					{
						Compressed=true;
					}
				}

				if (!Compressed)
					NewPtr->PackSize = fdata.nFileSize;

				if (ReadNumLinks)
					NewPtr->NumberOfLinks=GetNumberOfLinks(fdata.strFileName);
			}
			else
			{
				NewPtr->PackSize = 0;
			}

			NewPtr->SortGroup=DEFAULT_SORT_GROUP;

			if (ReadOwners)
			{
				string strOwner;
				GetFileOwner(strComputerName, NewPtr->strName,strOwner);
				NewPtr->strOwner = strOwner;
			}

			NewPtr->NumberOfStreams=NewPtr->FileAttr&FILE_ATTRIBUTE_DIRECTORY?0:1;
			NewPtr->StreamsSize=NewPtr->UnpSize;

			if (ReadNumStreams||ReadStreamsSize)
			{
				EnumStreams(TestParentFolderName(fdata.strFileName)?strCurDir:fdata.strFileName,NewPtr->StreamsSize,NewPtr->NumberOfStreams);
			}

			if (NeedHighlight)
				CtrlObject->HiFiles->GetHiColor(&NewPtr,1);

			if ((fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
				TotalFileCount++;

			//memcpy(ListData+FileCount,&NewPtr,sizeof(NewPtr));
//      FileCount++;

			if ((FileCount & 0x3f)==0 && clock()-StartTime>1000)
			{
				if (IsVisible())
				{
					string strReadMsg;

					if (!IsShowTitle)
					{
						if (!DrawMessage)
						{
							Text(X1+1,Y1,COL_PANELBOX,Title);
							IsShowTitle=TRUE;
							SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
						}
					}

					strReadMsg.Format(MSG(MReadingFiles),FileCount);

					if (DrawMessage)
					{
						ReadFileNamesMsg(strReadMsg);
					}
					else
					{
						TruncStr(strReadMsg,TitleLength-2);
						int MsgLength=(int)strReadMsg.GetLength();
						GotoXY(X1+1+(TitleLength-MsgLength-1)/2,Y1);
						mprintf(L" %s ", (const wchar_t*)strReadMsg);
					}
				}

				if (CheckForEsc())
				{
					Message(MSG_WARNING,1,MSG(MUserBreakTitle),MSG(MOperationNotCompleted),MSG(MOk));
					break;
				}
			}
		}

		Done=!apiFindNextFile(FindHandle,&fdata);
	}

	int ErrCode=GetLastError();

	if (!(ErrCode==ERROR_SUCCESS || ErrCode==ERROR_NO_MORE_FILES || ErrCode==ERROR_FILE_NOT_FOUND))
		Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MReadFolderError),MSG(MOk));

	apiFindClose(FindHandle);
	/*
	int NetRoot=FALSE;
	if (strCurDir.At(0)==L'\\' && strCurDir.At(1)==L'\\')
	{
		const wchar_t *ChPtr=wcschr((const wchar_t*)strCurDir+2,'\\');
		if (ChPtr==NULL || wcschr(ChPtr+1,L'\\')==NULL)
			NetRoot=TRUE;
	}
	*/

	// пока кусок закомментим, возможно он даже и не пригодится.
	if (!bCurDirRoot) // && !NetRoot)
	{
		if (FileCount>=AllocatedCount)
		{
			FileListItem **pTemp;

			if ((pTemp=(FileListItem **)xf_realloc(ListData,(FileCount+1)*sizeof(*ListData)))!=NULL)
				ListData=pTemp;
		}

		if (ListData!=NULL)
		{
			ListData[FileCount] = new FileListItem;
			AddParentPoint(ListData[FileCount],FileCount,TwoDotsTimes,TwoDotsOwner);

			if (NeedHighlight)
				CtrlObject->HiFiles->GetHiColor(&ListData[FileCount],1);

			FileCount++;
		}
	}

	if (IsColumnDisplayed(DIZ_COLUMN))
		ReadDiz();

	if (AnotherPanel->GetMode()==PLUGIN_PANEL)
	{
		HANDLE hAnotherPlugin=AnotherPanel->GetPluginHandle();
		PluginPanelItem *PanelData=NULL, *PtrPanelData;
		string strPath;
		int PanelCount=0;
		strPath = strCurDir;
		AddEndSlash(strPath);

		if (CtrlObject->Plugins.GetVirtualFindData(hAnotherPlugin,&PanelData,&PanelCount,strPath))
		{
			FileListItem **pTemp;

			if ((pTemp=(FileListItem **)xf_realloc(ListData,(FileCount+PanelCount)*sizeof(*ListData)))!=NULL)
			{
				ListData=pTemp;

				for (PtrPanelData=PanelData, I=0; I < PanelCount; I++, CurPtr++, PtrPanelData++)
				{
					CurPtr = ListData[FileCount+I];
					FAR_FIND_DATA &fdata=PtrPanelData->FindData;
					PluginToFileListItem(PtrPanelData,CurPtr);
					CurPtr->Position=FileCount;
					TotalFileSize += fdata.nFileSize;
					CurPtr->PrevSelected=CurPtr->Selected=0;
					CurPtr->ShowFolderSize=0;
					CurPtr->SortGroup=CtrlObject->HiFiles->GetGroup(CurPtr);

					if (!TestParentFolderName(fdata.lpwszFileName) && (CurPtr->FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0)
						TotalFileCount++;
				}

				// цветовую боевую раскраску в самом конце, за один раз
				CtrlObject->HiFiles->GetHiColor(&ListData[FileCount],PanelCount);
				FileCount+=PanelCount;
			}

			CtrlObject->Plugins.FreeVirtualFindData(hAnotherPlugin,PanelData,PanelCount);
		}
	}

	CreateChangeNotification(FALSE);
	CorrectPosition();

	if (KeepSelection || PrevSelFileCount>0)
	{
		MoveSelection(ListData,FileCount,OldData,OldFileCount);
		DeleteListData(OldData,OldFileCount);
	}

	if (SortGroups)
		ReadSortGroups(false);

	if (!KeepSelection && PrevSelFileCount>0)
	{
		SaveSelection();
		ClearSelection();
	}

	SortFileList(FALSE);

	if (CurFile>=FileCount || StrCmpI(ListData[CurFile]->strName,strCurName)!=0)
		if (!GoToFile(strCurName) && !strNextCurName.IsEmpty())
			GoToFile(strNextCurName);

	/* $ 13.02.2002 DJ
		SetTitle() - только если мы текущий фрейм!
	*/
	if (CtrlObject->Cp() == FrameManager->GetCurrentFrame())
		SetTitle();

	FarChDir(strSaveDir); //???
}

/*$ 22.06.2001 SKV
  Добавлен параметр для вызова после исполнения команды.
*/
int FileList::UpdateIfChanged(int UpdateMode)
{
	//_SVS(SysLog(L"CurDir='%s' Opt.AutoUpdateLimit=%d <= FileCount=%d",CurDir,Opt.AutoUpdateLimit,FileCount));
	if (!Opt.AutoUpdateLimit || static_cast<DWORD>(FileCount) <= Opt.AutoUpdateLimit)
	{
		/* $ 19.12.2001 VVM
		  ! Сменим приоритеты. При Force обновление всегда! */
		if ((IsVisible() && (clock()-LastUpdateTime>2000)) || (UpdateMode != UIC_UPDATE_NORMAL))
		{
			if (UpdateMode == UIC_UPDATE_NORMAL)
				ProcessPluginEvent(FE_IDLE,NULL);

			/* $ 24.12.2002 VVM
			  ! Поменяем логику обновления панелей. */
			if (// Нормальная панель, на ней установлено уведомление и есть сигнал
			    (PanelMode==NORMAL_PANEL && hListChange!=INVALID_HANDLE_VALUE && WaitForSingleObject(hListChange,0)==WAIT_OBJECT_0) ||
			    // Или Нормальная панель, но нет уведомления и мы попросили обновить через UPDATE_FORCE
			    (PanelMode==NORMAL_PANEL && hListChange==INVALID_HANDLE_VALUE && UpdateMode==UIC_UPDATE_FORCE) ||
			    // Или плагинная панель и обновляем через UPDATE_FORCE
			    (PanelMode!=NORMAL_PANEL && UpdateMode==UIC_UPDATE_FORCE)
			)
			{
				Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

				if (AnotherPanel->GetType()==INFO_PANEL)
				{
					AnotherPanel->Update(UPDATE_KEEP_SELECTION);

					if (UpdateMode==UIC_UPDATE_NORMAL)
						AnotherPanel->Redraw();
				}

				Update(UPDATE_KEEP_SELECTION);

				if (UpdateMode==UIC_UPDATE_NORMAL)
					Show();

				return(TRUE);
			}
		}
	}

	return(FALSE);
}

void FileList::CreateChangeNotification(int CheckTree)
{
	wchar_t RootDir[4]=L" :\\";
	DWORD DriveType=DRIVE_REMOTE;
	CloseChangeNotification();

	if (IsLocalPath(strCurDir))
	{
		RootDir[0]=strCurDir.At(0);
		DriveType=FAR_GetDriveType(RootDir);
	}

	if (Opt.AutoUpdateRemoteDrive || (!Opt.AutoUpdateRemoteDrive && DriveType != DRIVE_REMOTE))
	{
		hListChange=FindFirstChangeNotification(strCurDir,CheckTree,
		                                        FILE_NOTIFY_CHANGE_FILE_NAME|
		                                        FILE_NOTIFY_CHANGE_DIR_NAME|
		                                        FILE_NOTIFY_CHANGE_ATTRIBUTES|
		                                        FILE_NOTIFY_CHANGE_SIZE|
		                                        FILE_NOTIFY_CHANGE_LAST_WRITE);
	}
}


void FileList::CloseChangeNotification()
{
	if (hListChange!=INVALID_HANDLE_VALUE)
	{
		FindCloseChangeNotification(hListChange);
		hListChange=INVALID_HANDLE_VALUE;
	}
}

static int _cdecl SortSearchList(const void *el1,const void *el2)
{
	FileListItem **SPtr1=(FileListItem **)el1,**SPtr2=(FileListItem **)el2;
	return StrCmp(SPtr1[0]->strName,SPtr2[0]->strName);
}

void FileList::MoveSelection(FileListItem **ListData,long FileCount,
                             FileListItem **OldData,long OldFileCount)
{
	FileListItem **OldPtr;
	SelFileCount=0;
	SelFileSize=0;
	CacheSelIndex=-1;
	far_qsort((void *)OldData,OldFileCount,sizeof(*OldData),SortSearchList);

	while (FileCount--)
	{
		OldPtr=(FileListItem **)bsearch((void *)ListData,(void *)OldData,
		                                OldFileCount,sizeof(*ListData),SortSearchList);

		if (OldPtr!=NULL)
		{
			if (OldPtr[0]->ShowFolderSize)
			{
				ListData[0]->ShowFolderSize=2;
				ListData[0]->UnpSize=OldPtr[0]->UnpSize;
				ListData[0]->PackSize=OldPtr[0]->PackSize;
			}

			Select(ListData[0],OldPtr[0]->Selected);
			ListData[0]->PrevSelected=OldPtr[0]->PrevSelected;
		}

		ListData++;
	}
}

void FileList::UpdatePlugin(int KeepSelection, int IgnoreVisible)
{
	_ALGO(CleverSysLog clv(L"FileList::UpdatePlugin"));
	_ALGO(SysLog(L"(KeepSelection=%d, IgnoreVisible=%d)",KeepSelection,IgnoreVisible));

	if (!IsVisible() && !IgnoreVisible)
	{
		UpdateRequired=TRUE;
		UpdateRequiredMode=KeepSelection;
		return;
	}

	DizRead=FALSE;
	int I;
	FileListItem *CurPtr, **OldData=0;
	string strCurName, strNextCurName;
	int OldFileCount=0;
	CloseChangeNotification();
	LastCurFile=-1;
	OpenPluginInfo Info;
	CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

	if (Opt.ShowPanelFree && (Info.Flags & OPIF_REALNAMES))
	{
		unsigned __int64 TotalSize,TotalFree;

		if (!apiGetDiskSize(strCurDir,&TotalSize,&TotalFree,&FreeDiskSize))
			FreeDiskSize=0;
	}

	PluginPanelItem *PanelData=NULL;
	int PluginFileCount;

	if (!CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&PluginFileCount,0))
	{
		DeleteListData(ListData,FileCount);
		PopPlugin(TRUE);
		Update(KeepSelection);

		// WARP> явный хак, но очень способствует - восстанавливает позицию на панели при ошибке чтения архива.
		if (!PrevDataList.Empty())
			GoToFile((*PrevDataList.Last())->strPrevName);

		return;
	}

	int PrevSelFileCount=SelFileCount;
	SelFileCount=0;
	SelFileSize=0;
	TotalFileCount=0;
	TotalFileSize=0;
	CacheSelIndex=-1;
	strPluginDizName.Clear();

	if (FileCount>0)
	{
		CurPtr=ListData[CurFile];
		strCurName = CurPtr->strName;

		if (CurPtr->Selected)
		{
			for (I=CurFile+1; I < FileCount; I++)
			{
				CurPtr = ListData[I];

				if (!CurPtr->Selected)
				{
					strNextCurName = CurPtr->strName;
					break;
				}
			}
		}
	}
	else if (Info.Flags & OPIF_ADDDOTS)
	{
		strCurName = L"..";
	}

	if (KeepSelection || PrevSelFileCount>0)
	{
		OldData=ListData;
		OldFileCount=FileCount;
	}
	else
	{
		DeleteListData(ListData,FileCount);
	}

	FileCount=PluginFileCount;
	ListData=(FileListItem**)xf_malloc(sizeof(FileListItem*)*(FileCount+1));

	if (ListData==NULL)
	{
		FileCount=0;
		return;
	}

	if (Filter==NULL)
		Filter=new FileFilter(this,FFT_PANEL);

	//Рефреш текущему времени для фильтра перед началом операции
	Filter->UpdateCurrentTime();
	CtrlObject->HiFiles->UpdateCurrentTime();
	int DotsPresent=FALSE;
	int FileListCount=0;
	PluginPanelItem *CurPanelData=PanelData;

	for (I=0; I < FileCount; I++, CurPanelData++)
	{
		ListData[FileListCount] = new FileListItem;
		FileListItem *CurListData=ListData[FileListCount];
		CurListData->Clear();

		if (Info.Flags & OPIF_USEFILTER)

			//if ((CurPanelData->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
			if (!Filter->FileInFilter(&CurPanelData->FindData))
				continue;

		if (!Opt.ShowHidden && (CurPanelData->FindData.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)))
			continue;

		//memset(CurListData,0,sizeof(*CurListData));
		PluginToFileListItem(CurPanelData,CurListData);
		CurListData->Position=I;

		if ((Info.Flags & OPIF_USESORTGROUPS)/* && (CurListData->FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0*/)
			CurListData->SortGroup=CtrlObject->HiFiles->GetGroup(CurListData);
		else
			CurListData->SortGroup=DEFAULT_SORT_GROUP;

		if (CurListData->DizText==NULL)
		{
			CurListData->DeleteDiz=FALSE;
			//CurListData->DizText=NULL;
		}

		if (TestParentFolderName(CurListData->strName))
		{
			DotsPresent=TRUE;
			CurListData->FileAttr|=FILE_ATTRIBUTE_DIRECTORY;
		}
		else if ((CurListData->FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0)
		{
			TotalFileCount++;
		}

		TotalFileSize += CurListData->UnpSize;
		FileListCount++;
	}

	if ((Info.Flags & OPIF_USEHIGHLIGHTING) || (Info.Flags & OPIF_USEATTRHIGHLIGHTING))
		CtrlObject->HiFiles->GetHiColor(ListData,FileListCount,(Info.Flags&OPIF_USEATTRHIGHLIGHTING)!=0);

	FileCount=FileListCount;

	if ((Info.Flags & OPIF_ADDDOTS) && !DotsPresent)
	{
		ListData[FileCount] = new FileListItem;
		FileListItem *CurPtr = ListData[FileCount];
		CurPtr->Clear();
		AddParentPoint(CurPtr,FileCount);

		if ((Info.Flags & OPIF_USEHIGHLIGHTING) || (Info.Flags & OPIF_USEATTRHIGHLIGHTING))
			CtrlObject->HiFiles->GetHiColor(&CurPtr,1,(Info.Flags&OPIF_USEATTRHIGHLIGHTING)!=0);

		if (Info.HostFile && *Info.HostFile)
		{
			FAR_FIND_DATA_EX FindData;

			if (apiGetFindDataEx(Info.HostFile,&FindData))
			{
				CurPtr->WriteTime=FindData.ftLastWriteTime;
				CurPtr->CreationTime=FindData.ftCreationTime;
				CurPtr->AccessTime=FindData.ftLastAccessTime;
			}
		}

		FileCount++;
	}

	/* $ 25.02.2001 VVM
	    ! Не считывать повторно список файлов с панели плагина */
	if (IsColumnDisplayed(DIZ_COLUMN))
		ReadDiz(PanelData,PluginFileCount,RDF_NO_UPDATE);

	CorrectPosition();
	CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,PluginFileCount);

	if (KeepSelection || PrevSelFileCount>0)
	{
		MoveSelection(ListData,FileCount,OldData,OldFileCount);
		DeleteListData(OldData,OldFileCount);
	}

	if (!KeepSelection && PrevSelFileCount>0)
	{
		SaveSelection();
		ClearSelection();
	}

	SortFileList(FALSE);

	if (CurFile>=FileCount || StrCmpI(ListData[CurFile]->strName,strCurName)!=0)
		if (!GoToFile(strCurName) && !strNextCurName.IsEmpty())
			GoToFile(strNextCurName);

	SetTitle();
}


void FileList::ReadDiz(PluginPanelItem *ItemList,int ItemLength,DWORD dwFlags)
{
	if (DizRead)
		return;

	DizRead=TRUE;
	Diz.Reset();

	if (PanelMode==NORMAL_PANEL)
	{
		Diz.Read(strCurDir);
	}
	else
	{
		PluginPanelItem *PanelData=NULL;
		int PluginFileCount=0;
		OpenPluginInfo Info;
		CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

		if (Info.DescrFilesNumber==0)
			return;

		int GetCode=TRUE;

		/* $ 25.02.2001 VVM
		    + Обработка флага RDF_NO_UPDATE */
		if ((ItemList==NULL) && ((dwFlags & RDF_NO_UPDATE) == 0))
		{
			GetCode=CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&PluginFileCount,0);
		}
		else
		{
			PanelData=ItemList;
			PluginFileCount=ItemLength;
		}

		if (GetCode)
		{
			for (int I=0; I<Info.DescrFilesNumber; I++)
			{
				PluginPanelItem *CurPanelData=PanelData;

				for (int J=0; J < PluginFileCount; J++, CurPanelData++)
				{
					string strFileName = CurPanelData->FindData.lpwszFileName;

					if (StrCmpI(strFileName,Info.DescrFiles[I])==0)
					{
						string strTempDir, strDizName;

						if (FarMkTempEx(strTempDir) && apiCreateDirectory(strTempDir,NULL))
						{
							if (CtrlObject->Plugins.GetFile(hPlugin,CurPanelData,strTempDir,strDizName,OPM_SILENT|OPM_VIEW|OPM_QUICKVIEW|OPM_DESCR))
							{
								strPluginDizName = Info.DescrFiles[I];
								Diz.Read(L"",strDizName);
								DeleteFileWithFolder(strDizName);
								I=Info.DescrFilesNumber;
								break;
							}

							apiRemoveDirectory(strTempDir);
							//ViewPanel->ShowFile(NULL,FALSE,NULL);
						}
					}
				}
			}

			/* $ 25.02.2001 VVM
			    + Обработка флага RDF_NO_UPDATE */
			if ((ItemList==NULL) && ((dwFlags & RDF_NO_UPDATE) == 0))
				CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,PluginFileCount);
		}
	}

	for (int I=0; I<FileCount; I++)
	{
		if (ListData[I]->DizText==NULL)
		{
			ListData[I]->DeleteDiz=FALSE;
			ListData[I]->DizText=(wchar_t*)Diz.GetDizTextAddr(ListData[I]->strName,ListData[I]->strShortName,ListData[I]->UnpSize);
		}
	}
}


void FileList::ReadSortGroups(bool UpdateFilterCurrentTime)
{
	if (!SortGroupsRead)
	{
		if (UpdateFilterCurrentTime)
		{
			CtrlObject->HiFiles->UpdateCurrentTime();
		}

		SortGroupsRead=TRUE;

		for (int i=0; i<FileCount; i++)
		{
			ListData[i]->SortGroup=CtrlObject->HiFiles->GetGroup(ListData[i]);
		}
	}
}

// Обнулить текущий CurPtr и занести предопределенные данные для каталога ".."
void FileList::AddParentPoint(FileListItem *CurPtr,long CurFilePos,FILETIME* Times,string Owner)
{
	CurPtr->Clear();
	CurPtr->FileAttr = FILE_ATTRIBUTE_DIRECTORY;
	CurPtr->strName = L"..";
	CurPtr->strShortName = L"..";

	if (Times)
	{
		CurPtr->CreationTime = Times[0];
		CurPtr->AccessTime = Times[1];
		CurPtr->WriteTime = Times[2];
	}

	CurPtr->strOwner = Owner;
	CurPtr->Position = CurFilePos;
}
