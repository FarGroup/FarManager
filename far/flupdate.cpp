/*
flupdate.cpp

Файловая панель - чтение имен файлов
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
#include "flink.hpp"
#include "colors.hpp"
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
				OpenPanelInfo Info;
				Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
				ProcessPluginCommand();

				if (PanelMode!=PLUGIN_PANEL)
					ReadFileNames(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE,Mode & UPDATE_DRAW_MESSAGE);
				else if ((Info.Flags & OPIF_REALNAMES) ||
				         Global->CtrlObject->Cp()->GetAnotherPanel(this)->GetMode()==PLUGIN_PANEL ||
				         !(Mode & UPDATE_SECONDARY))
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
	if (!Global->PreRedraw->empty())
	{
		Global->PreRedraw->top().Param.Param1=(void*)Msg;
	}
}

static void PR_ReadFileNamesMsg()
{
	if (!Global->PreRedraw->empty())
	{
		ReadFileNamesMsg((wchar_t *)Global->PreRedraw->top().Param.Param1);
	}
}

// ЭТО ЕСТЬ УЗКОЕ МЕСТО ДЛЯ СКОРОСТНЫХ ХАРАКТЕРИСТИК Far Manager
// при считывании дирректории

void FileList::ReadFileNames(int KeepSelection, int IgnoreVisible, int DrawMessage)
{
	TPreRedrawFuncGuard preRedrawFuncGuard(PR_ReadFileNamesMsg);
	TaskBar TB(false);

	strOriginalCurDir = strCurDir;

	if (!IsVisible() && !IgnoreVisible)
	{
		UpdateRequired=TRUE;
		UpdateRequiredMode=KeepSelection;
		return;
	}

	UpdateRequired=FALSE;
	AccessTimeUpdateRequired=FALSE;
	DizRead=FALSE;
	FAR_FIND_DATA fdata;
	decltype(ListData) OldData;
	string strCurName, strNextCurName;
	StopFSWatcher();

	if (this!=Global->CtrlObject->Cp()->LeftPanel && this!=Global->CtrlObject->Cp()->RightPanel)
		return;

	string strSaveDir;
	apiGetCurrentDirectory(strSaveDir);
	{
		string strOldCurDir(strCurDir);

		if (!SetCurPath())
		{
			FlushInputBuffer(); // Очистим буффер ввода, т.к. мы уже можем быть в другом месте...

			if (strCurDir == strOldCurDir) //?? i??
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
		Global->CtrlObject->CmdLine->SetCurDir(strCurDir);

	LastCurFile=-1;
	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
	AnotherPanel->QViewDelTempName();
	size_t PrevSelFileCount=SelFileCount;
	SelFileCount=0;
	SelFileSize=0;
	TotalFileCount=0;
	TotalFileSize=0;
	CacheSelIndex=-1;
	CacheSelClearIndex=-1;
	FreeDiskSize = -1;
	if (Global->Opt->ShowPanelFree)
	{
		apiGetDiskSize(strCurDir, nullptr, nullptr, &FreeDiskSize);
	}

	if (!ListData.empty())
	{
		strCurName = ListData[CurFile]->strName;

		if (ListData[CurFile]->Selected)
		{
			for (size_t i=CurFile+1; i < ListData.size(); i++)
			{
				if (!ListData[i]->Selected)
				{
					strNextCurName = ListData[i]->strName;
					break;
				}
			}
		}
	}

	if (KeepSelection || PrevSelFileCount>0)
	{
		OldData = ListData;
	}
	else
		DeleteListData(ListData);

	DWORD FileSystemFlags = 0;
	string PathRoot;
	GetPathRoot(strCurDir, PathRoot);
	apiGetVolumeInformation(PathRoot, nullptr, nullptr, nullptr, &FileSystemFlags, nullptr);

	ListData.clear();

	bool ReadOwners = IsColumnDisplayed(OWNER_COLUMN);
	bool ReadNumLinks = IsColumnDisplayed(NUMLINK_COLUMN);
	bool ReadNumStreams = IsColumnDisplayed(NUMSTREAMS_COLUMN);
	bool ReadStreamsSize = IsColumnDisplayed(STREAMSSIZE_COLUMN);

	if (!(FileSystemFlags&FILE_SUPPORTS_HARD_LINKS) && Global->WinVer() >= _WIN32_WINNT_WIN7)
	{
		ReadNumLinks = false;
	}

	if(!(FileSystemFlags&FILE_NAMED_STREAMS))
	{
		ReadNumStreams = false;
		ReadStreamsSize = false;
	}

	string strComputerName;

	if (ReadOwners)
	{
		CurPath2ComputerName(strCurDir, strComputerName);
		// сбросим кэш SID`ов
		SIDCacheFlush();
	}

	SetLastError(ERROR_SUCCESS);
	// сформируем заголовок вне цикла
	wchar_t Title[2048];
	int TitleLength=std::min((int)X2-X1-1,(int)(ARRAYSIZE(Title))-1);
	//wmemset(Title,0x0CD,TitleLength); //BUGBUG
	//Title[TitleLength]=0;
	MakeSeparator(TitleLength, Title, 9, nullptr);
	BOOL IsShowTitle=FALSE;
	BOOL NeedHighlight=Global->Opt->Highlight && PanelMode != PLUGIN_PANEL;

	if (!Filter)
		Filter=new FileFilter(this,FFT_PANEL);

	//Рефреш текущему времени для фильтра перед началом операции
	Filter->UpdateCurrentTime();
	Global->CtrlObject->HiFiles->UpdateCurrentTime();
	bool bCurDirRoot = false;
	ParsePath(strCurDir, nullptr, &bCurDirRoot);
	PATH_TYPE Type = ParsePath(strCurDir, nullptr, &bCurDirRoot);
	bool NetRoot = bCurDirRoot && (Type == PATH_REMOTE || Type == PATH_REMOTEUNC);

	string strFind(strCurDir);
	AddEndSlash(strFind);
	strFind+=L'*';
	::FindFile Find(strFind, true);
	DWORD FindErrorCode = ERROR_SUCCESS;
	bool UseFilter=Filter->IsEnabledOnPanel();
	bool ReadCustomData=IsColumnDisplayed(CUSTOM_COLUMN0)!=0;

	DWORD StartTime = GetTickCount();

	while (Find.Get(fdata))
	{
		FindErrorCode = GetLastError();

		if ((Global->Opt->ShowHidden || !(fdata.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))) && (!UseFilter || Filter->FileInFilter(fdata, nullptr, &fdata.strFileName)))
		{
			if (ListData.size() == ListData.capacity())
				ListData.reserve(ListData.size() + 4096);

			auto NewItem = new FileListItem;
			NewItem->Clear();
			NewItem->FileAttr = fdata.dwFileAttributes;
			NewItem->CreationTime = fdata.ftCreationTime;
			NewItem->AccessTime = fdata.ftLastAccessTime;
			NewItem->WriteTime = fdata.ftLastWriteTime;
			NewItem->ChangeTime = fdata.ftChangeTime;
			NewItem->FileSize = fdata.nFileSize;
			NewItem->AllocationSize = fdata.nAllocationSize;
			NewItem->strName = fdata.strFileName;
			NewItem->strShortName = fdata.strAlternateFileName;
			NewItem->Position = ListData.size();
			NewItem->NumberOfLinks=1;

			if (fdata.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			{
				NewItem->ReparseTag=fdata.dwReserved0; //MSDN
			}
			if (!(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				TotalFileSize += NewItem->FileSize;

				if (ReadNumLinks)
					NewItem->NumberOfLinks = GetNumberOfLinks(fdata.strFileName, true);
			}
			else
			{
				NewItem->AllocationSize = 0;
			}

			NewItem->SortGroup=DEFAULT_SORT_GROUP;

			if (ReadOwners)
			{
				string strOwner;
				GetFileOwner(strComputerName, NewItem->strName,strOwner);
				NewItem->strOwner = strOwner;
			}

			NewItem->NumberOfStreams=NewItem->FileAttr&FILE_ATTRIBUTE_DIRECTORY?0:1;
			NewItem->StreamsSize=NewItem->FileSize;

			if (ReadNumStreams||ReadStreamsSize)
			{
				EnumStreams(TestParentFolderName(fdata.strFileName)? strCurDir : fdata.strFileName, NewItem->StreamsSize, NewItem->NumberOfStreams);
			}

			if (ReadCustomData)
				Global->CtrlObject->Plugins->GetCustomData(NewItem);

			ListData.emplace_back(NewItem);

			if (!(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				TotalFileCount++;

			DWORD CurTime = GetTickCount();
			if (CurTime - StartTime > (DWORD)Global->Opt->RedrawTimeout)
			{
				StartTime = CurTime;
				if (IsVisible())
				{
					if (!IsShowTitle)
					{
						if (!DrawMessage)
						{
							Text(X1+1,Y1,ColorIndexToColor(COL_PANELBOX),Title);
							IsShowTitle=TRUE;
							SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
						}
					}

					LangString strReadMsg(MReadingFiles);
					strReadMsg << ListData.size();

					if (DrawMessage)
					{
						ReadFileNamesMsg(strReadMsg);
					}
					else
					{
						TruncStr(strReadMsg,TitleLength-2);
						int MsgLength=(int)strReadMsg.GetLength();
						GotoXY(X1+1+(TitleLength-MsgLength-1)/2,Y1);
						Global->FS << L" "<<strReadMsg<<L" ";
					}
				}

				if (CheckForEsc())
				{
					break;
				}
			}
		}
	}

	if (!(FindErrorCode==ERROR_SUCCESS || FindErrorCode==ERROR_NO_MORE_FILES || FindErrorCode==ERROR_FILE_NOT_FOUND))
		Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MReadFolderError),MSG(MOk));

	if ((Global->Opt->ShowDotsInRoot || !bCurDirRoot) || (NetRoot && Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Network))) // NetWork Plugin
	{
		auto NewItem = new FileListItem;

		string TwoDotsOwner;
		if (ReadOwners)
		{
			GetFileOwner(strComputerName,strCurDir,TwoDotsOwner);
		}

		FILETIME TwoDotsTimes[4]={};
		if(apiGetFindDataEx(strCurDir,fdata))
		{
			TwoDotsTimes[0]=fdata.ftCreationTime;
			TwoDotsTimes[1]=fdata.ftLastAccessTime;
			TwoDotsTimes[2]=fdata.ftLastWriteTime;
			TwoDotsTimes[3]=fdata.ftChangeTime;
		}

		AddParentPoint(NewItem, ListData.size(), TwoDotsTimes, TwoDotsOwner);

		ListData.emplace_back(NewItem);
	}

	if (IsColumnDisplayed(DIZ_COLUMN))
		ReadDiz();

	if (NeedHighlight)
	{
		Global->CtrlObject->HiFiles->GetHiColor(ListData.begin(), ListData.size());
	}

	if (AnotherPanel->GetMode()==PLUGIN_PANEL)
	{
		HANDLE hAnotherPlugin=AnotherPanel->GetPluginHandle();
		PluginPanelItem *PanelData=nullptr;
		string strPath(strCurDir);
		AddEndSlash(strPath);
		size_t PanelCount=0;

		if (Global->CtrlObject->Plugins->GetVirtualFindData(hAnotherPlugin,&PanelData,&PanelCount,strPath))
		{
			auto OldSize = ListData.size(), Position = OldSize - 1;
			ListData.resize(ListData.size() + PanelCount);

			auto PluginPtr = PanelData;
			for (auto i = ListData.begin() + OldSize; i != ListData.end(); ++i, ++PluginPtr, ++Position)
			{
				PluginToFileListItem(PluginPtr, *i);
				(*i)->Position = Position;
				TotalFileSize += PluginPtr->FileSize;
				(*i)->PrevSelected = (*i)->Selected=0;
				(*i)->ShowFolderSize = 0;
				(*i)->SortGroup=Global->CtrlObject->HiFiles->GetGroup(*i);

				if (!TestParentFolderName(PluginPtr->FileName) && !((*i)->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
					TotalFileCount++;
			}

			// цветовую боевую раскраску в самом конце, за один раз
			Global->CtrlObject->HiFiles->GetHiColor(ListData.begin() + OldSize, PanelCount);
			Global->CtrlObject->Plugins->FreeVirtualFindData(hAnotherPlugin,PanelData,PanelCount);
		}
	}

	InitFSWatcher(false);
	CorrectPosition();

   string strLastSel, strGetSel;

	if (KeepSelection || PrevSelFileCount>0)
	{
		if (LastSelPosition >= 0 && static_cast<size_t>(LastSelPosition) < OldData.size())
			strLastSel = OldData[LastSelPosition]->strName;
		if (GetSelPosition >= 0 && static_cast<size_t>(GetSelPosition) < OldData.size())
			strGetSel = OldData[GetSelPosition]->strName;

		MoveSelection(OldData, ListData);
		DeleteListData(OldData);
	}

	if (SortGroups)
		ReadSortGroups(false);

	if (!KeepSelection && PrevSelFileCount>0)
	{
		SaveSelection();
		ClearSelection();
	}

	SortFileList(FALSE);

	if (!strLastSel.IsEmpty())
		LastSelPosition = FindFile(strLastSel, FALSE);
	if (!strGetSel.IsEmpty())
		GetSelPosition = FindFile(strGetSel, FALSE);

	if (CurFile >= static_cast<int>(ListData.size()) || StrCmpI(ListData[CurFile]->strName,strCurName))
		if (!GoToFile(strCurName) && !strNextCurName.IsEmpty())
			GoToFile(strNextCurName);

	/* $ 13.02.2002 DJ
		SetTitle() - только если мы текущий фрейм!
	*/
	if (Global->CtrlObject->Cp() == FrameManager->GetCurrentFrame())
		SetTitle();

	FarChDir(strSaveDir); //???
}

/*$ 22.06.2001 SKV
  Добавлен параметр для вызова после исполнения команды.
*/
int FileList::UpdateIfChanged(int UpdateMode)
{
	//_SVS(SysLog(L"CurDir='%s' Global->Opt->AutoUpdateLimit=%d <= FileCount=%d",CurDir,Global->Opt->AutoUpdateLimit,FileCount));
	if (!Global->Opt->AutoUpdateLimit || ListData.size() <= static_cast<size_t>(Global->Opt->AutoUpdateLimit))
	{
		/* $ 19.12.2001 VVM
		  ! Сменим приоритеты. При Force обновление всегда! */
		if ((IsVisible() && (clock()-LastUpdateTime>2000)) || (UpdateMode != UIC_UPDATE_NORMAL))
		{
			if (UpdateMode == UIC_UPDATE_NORMAL)
				ProcessPluginEvent(FE_IDLE,nullptr);

			/* $ 24.12.2002 VVM
			  ! Поменяем логику обновления панелей. */
			if (// Нормальная панель, на ней установлено уведомление и есть сигнал
			    (PanelMode==NORMAL_PANEL && FSWatcher.Signaled()) ||
			    // Или Нормальная панель, но нет уведомления и мы попросили обновить через UPDATE_FORCE
			    (PanelMode==NORMAL_PANEL && UpdateMode==UIC_UPDATE_FORCE) ||
			    // Или плагинная панель и обновляем через UPDATE_FORCE
			    (PanelMode!=NORMAL_PANEL && UpdateMode==UIC_UPDATE_FORCE)
			)
			{
				Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

				if (AnotherPanel->GetType()==INFO_PANEL)
				{
					AnotherPanel->Update(UPDATE_KEEP_SELECTION);

					if (UpdateMode==UIC_UPDATE_NORMAL)
						AnotherPanel->Redraw();
				}

				Update(UPDATE_KEEP_SELECTION);

				if (UpdateMode==UIC_UPDATE_NORMAL)
					Show();

				return TRUE;
			}
		}
	}

	return FALSE;
}

void FileList::InitFSWatcher(bool CheckTree)
{
	wchar_t RootDir[4]=L" :\\";
	DWORD DriveType=DRIVE_REMOTE;
	StopFSWatcher();
	PATH_TYPE Type = ParsePath(strCurDir);

	if (Type == PATH_DRIVELETTER || Type == PATH_DRIVELETTERUNC)
	{
		RootDir[0] = (Type == PATH_DRIVELETTER)? strCurDir.At(0) : strCurDir.At(4);
		DriveType=FAR_GetDriveType(RootDir);
	}

	if (Global->Opt->AutoUpdateRemoteDrive || (!Global->Opt->AutoUpdateRemoteDrive && DriveType != DRIVE_REMOTE) || Type == PATH_VOLUMEGUID)
	{
		FSWatcher.Set(strCurDir, CheckTree);
		StartFSWatcher();
	}
}

void FileList::StartFSWatcher(bool got_focus)
{
	FSWatcher.Watch(got_focus);
}

void FileList::StopFSWatcher()
{
	FSWatcher.Release();
}

struct search_list_less
{
	bool operator()(const FileListItem* a, const FileListItem* b) const
	{
		return a->strName < b->strName;
	}
}
SearchListLess;

void FileList::MoveSelection(std::vector<FileListItem*>& From, std::vector<FileListItem*>& To)
{
	SelFileCount=0;
	SelFileSize=0;
	CacheSelIndex=-1;
	CacheSelClearIndex=-1;

	std::sort(From.begin(), From.end(), SearchListLess);

	std::for_each(CONST_RANGE(To, i)
	{
		auto Iterator = std::lower_bound(From.begin(), From.end(), i, SearchListLess);
		if (Iterator != From.end())
		{
			auto OldItem = *Iterator;
			if (OldItem->strName == i->strName)
			{
				if (OldItem->ShowFolderSize)
				{
					i->ShowFolderSize = 2;
					i->FileSize = OldItem->FileSize;
					i->AllocationSize = OldItem->AllocationSize;
				}

				this->Select(i, OldItem->Selected);
				i->PrevSelected = OldItem->PrevSelected;
			}
		}
	});
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
	std::vector<FileListItem*> OldData;
	string strCurName, strNextCurName;
	int OldFileCount=0;
	StopFSWatcher();
	LastCurFile=-1;
	OpenPanelInfo Info;
	Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

	FreeDiskSize=-1;
	if (Global->Opt->ShowPanelFree)
	{
		if (Info.Flags & OPIF_REALNAMES)
		{
			apiGetDiskSize(strCurDir, nullptr, nullptr, &FreeDiskSize);
		}
		else if (Info.Flags & OPIF_USEFREESIZE)
			FreeDiskSize=Info.FreeSize;
	}

	PluginPanelItem *PanelData=nullptr;
	size_t PluginFileCount;

	if (!Global->CtrlObject->Plugins->GetFindData(hPlugin,&PanelData,&PluginFileCount,0))
	{
		PopPlugin(TRUE);
		Update(KeepSelection);

		// WARP> явный хак, но очень способствует - восстанавливает позицию на панели при ошибке чтения архива.
		if (!PrevDataList.empty())
			GoToFile(PrevDataList.back()->strPrevName);

		return;
	}

	size_t PrevSelFileCount=SelFileCount;
	SelFileCount=0;
	SelFileSize=0;
	TotalFileCount=0;
	TotalFileSize=0;
	CacheSelIndex=-1;
	CacheSelClearIndex=-1;
	strPluginDizName.Clear();

	if (!ListData.empty())
	{
		auto CurPtr=ListData[CurFile];
		strCurName = CurPtr->strName;

		if (CurPtr->Selected)
		{
			for (size_t i = CurFile + 1; i < ListData.size(); ++i)
			{
				CurPtr = ListData[i];

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
	}
	else
	{
		DeleteListData(ListData);
	}

	ListData.resize(PluginFileCount);

	if (!Filter)
		Filter = new FileFilter(this, FFT_PANEL);

	//Рефреш текущему времени для фильтра перед началом операции
	Filter->UpdateCurrentTime();
	Global->CtrlObject->HiFiles->UpdateCurrentTime();
	int DotsPresent=FALSE;
	int FileListCount=0;
	bool UseFilter=Filter->IsEnabledOnPanel();

	for (size_t i = 0; i < ListData.size(); i++)
	{
		ListData[FileListCount] = new FileListItem;
		FileListItem *CurListData=ListData[FileListCount];
		CurListData->Clear();

		if (UseFilter && !(Info.Flags & OPIF_DISABLEFILTER))
		{
			//if (!(CurPanelData->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			if (!Filter->FileInFilter(PanelData[i]))
				continue;
		}

		if (!Global->Opt->ShowHidden && (PanelData[i].FileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)))
			continue;

		//ClearStruct(*CurListData);
		PluginToFileListItem(&PanelData[i],CurListData);
		CurListData->Position=i;

		if (!(Info.Flags & OPIF_DISABLESORTGROUPS)/* && !(CurListData->FileAttr & FILE_ATTRIBUTE_DIRECTORY)*/)
			CurListData->SortGroup=Global->CtrlObject->HiFiles->GetGroup(CurListData);
		else
			CurListData->SortGroup=DEFAULT_SORT_GROUP;

		if (!CurListData->DizText)
		{
			CurListData->DeleteDiz=FALSE;
			//CurListData->DizText=nullptr;
		}

		if (TestParentFolderName(CurListData->strName))
		{
			DotsPresent=TRUE;
			CurListData->FileAttr|=FILE_ATTRIBUTE_DIRECTORY;
		}
		else if (!(CurListData->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			TotalFileCount++;
		}

		TotalFileSize += CurListData->FileSize;
		FileListCount++;
	}

	ListData.resize(FileListCount);

	if (!(Info.Flags & OPIF_DISABLEHIGHLIGHTING) || (Info.Flags & OPIF_USEATTRHIGHLIGHTING))
		Global->CtrlObject->HiFiles->GetHiColor(ListData.begin(), ListData.size(), (Info.Flags&OPIF_USEATTRHIGHLIGHTING)!=0);

	if ((Info.Flags & OPIF_ADDDOTS) && !DotsPresent)
	{
		auto NewItem = new FileListItem;
		NewItem->Clear();
		AddParentPoint(NewItem, ListData.size());
		ListData.emplace_back(NewItem);

		if (!(Info.Flags & OPIF_DISABLEHIGHLIGHTING) || (Info.Flags & OPIF_USEATTRHIGHLIGHTING))
			Global->CtrlObject->HiFiles->GetHiColor(ListData.end() - 1, 1, (Info.Flags&OPIF_USEATTRHIGHLIGHTING)!=0);

		if (Info.HostFile && *Info.HostFile)
		{
			FAR_FIND_DATA FindData;

			if (apiGetFindDataEx(Info.HostFile, FindData))
			{
				NewItem->WriteTime=FindData.ftLastWriteTime;
				NewItem->CreationTime=FindData.ftCreationTime;
				NewItem->AccessTime=FindData.ftLastAccessTime;
				NewItem->ChangeTime=FindData.ftChangeTime;
			}
		}
	}

	if (CurFile >= static_cast<int>(ListData.size()))
		CurFile = ListData.size() ? static_cast<int>(ListData.size() - 1) : 0;

	/* $ 25.02.2001 VVM
	    ! Не считывать повторно список файлов с панели плагина */
	if (IsColumnDisplayed(DIZ_COLUMN))
		ReadDiz(PanelData,static_cast<int>(PluginFileCount),RDF_NO_UPDATE);

	CorrectPosition();
	Global->CtrlObject->Plugins->FreeFindData(hPlugin,PanelData,PluginFileCount,false);

	string strLastSel, strGetSel;

	if (KeepSelection || PrevSelFileCount>0)
	{
		if (LastSelPosition >= 0 && LastSelPosition < OldFileCount)
			strLastSel = OldData[LastSelPosition]->strName;
		if (GetSelPosition >= 0 && GetSelPosition < OldFileCount)
			strGetSel = OldData[GetSelPosition]->strName;

		MoveSelection(OldData, ListData);
		DeleteListData(OldData);
	}

	if (!KeepSelection && PrevSelFileCount>0)
	{
		SaveSelection();
		ClearSelection();
	}

	SortFileList(FALSE);

	if (!strLastSel.IsEmpty())
		LastSelPosition = FindFile(strLastSel, FALSE);
	if (!strGetSel.IsEmpty())
		GetSelPosition = FindFile(strGetSel, FALSE);

	if (CurFile >= static_cast<int>(ListData.size()) || StrCmpI(ListData[CurFile]->strName,strCurName))
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
		PluginPanelItem *PanelData=nullptr;
		size_t PluginFileCount=0;
		OpenPanelInfo Info;
		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

		if (!Info.DescrFilesNumber)
			return;

		int GetCode=TRUE;

		/* $ 25.02.2001 VVM
		    + Обработка флага RDF_NO_UPDATE */
		if (!ItemList && !(dwFlags & RDF_NO_UPDATE))
		{
			GetCode=Global->CtrlObject->Plugins->GetFindData(hPlugin,&PanelData,&PluginFileCount,0);
		}
		else
		{
			PanelData=ItemList;
			PluginFileCount=ItemLength;
		}

		if (GetCode)
		{
			for (size_t I=0; I<Info.DescrFilesNumber; I++)
			{
				PluginPanelItem *CurPanelData=PanelData;

				for (size_t J=0; J < PluginFileCount; J++, CurPanelData++)
				{
					string strFileName = CurPanelData->FileName;

					if (!StrCmpI(strFileName,Info.DescrFiles[I]))
					{
						string strTempDir, strDizName;

						if (FarMkTempEx(strTempDir) && apiCreateDirectory(strTempDir,nullptr))
						{
							if (Global->CtrlObject->Plugins->GetFile(hPlugin,CurPanelData,strTempDir,strDizName,OPM_SILENT|OPM_VIEW|OPM_QUICKVIEW|OPM_DESCR))
							{
								strPluginDizName = Info.DescrFiles[I];
								Diz.Read(L"", &strDizName);
								DeleteFileWithFolder(strDizName);
								I=Info.DescrFilesNumber;
								break;
							}

							apiRemoveDirectory(strTempDir);
							//ViewPanel->ShowFile(nullptr,FALSE,nullptr);
						}
					}
				}
			}

			/* $ 25.02.2001 VVM
			    + Обработка флага RDF_NO_UPDATE */
			if (!ItemList && !(dwFlags & RDF_NO_UPDATE))
				Global->CtrlObject->Plugins->FreeFindData(hPlugin,PanelData,PluginFileCount,true);
		}
	}

	std::for_each(CONST_RANGE(ListData, i)
	{
		if (!i->DizText)
		{
			i->DeleteDiz = FALSE;
			i->DizText = const_cast<wchar_t*>(Diz.GetDizTextAddr(i->strName, i->strShortName, i->FileSize));
		}
	});
}


void FileList::ReadSortGroups(bool UpdateFilterCurrentTime)
{
	if (!SortGroupsRead)
	{
		if (UpdateFilterCurrentTime)
		{
			Global->CtrlObject->HiFiles->UpdateCurrentTime();
		}

		SortGroupsRead=TRUE;

		std::for_each(CONST_RANGE(ListData, i)
		{
			i->SortGroup=Global->CtrlObject->HiFiles->GetGroup(i);
		});
	}
}

// Обнулить текущий CurPtr и занести предопределенные данные для каталога ".."
void FileList::AddParentPoint(FileListItem *CurPtr, size_t CurFilePos, FILETIME* Times, const string& Owner)
{
	CurPtr->Clear();
	CurPtr->FileAttr = FILE_ATTRIBUTE_DIRECTORY;
	CurPtr->strName = L"..";

	if (Times)
	{
		CurPtr->CreationTime = Times[0];
		CurPtr->AccessTime = Times[1];
		CurPtr->WriteTime = Times[2];
		CurPtr->ChangeTime = Times[3];
	}

	CurPtr->strOwner = Owner;
	CurPtr->Position = CurFilePos;
}
