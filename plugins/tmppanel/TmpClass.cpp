/*
TMPCLASS.CPP

Temporary panel plugin class implementation

*/


#include <cwchar>
#include "plugin.hpp"
#include <shellapi.h>
#include <PluginSettings.hpp>
#include <DlgBuilder.hpp>

#include "TmpLng.hpp"
#include "TmpCfg.hpp"
#include "TmpClass.hpp"
#include "TmpPanel.hpp"
#include <initguid.h>
#include "guid.hpp"
#include <SimpleString.hpp>

TmpPanel::TmpPanel(const wchar_t* pHostFile)
{
	LastOwnersRead=FALSE;
	LastLinksRead=FALSE;
	UpdateNotNeeded=FALSE;
	TmpPanelItem=NULL;
	TmpItemsNumber=0;
	PanelIndex=CurrentCommonPanel;
	IfOptCommonPanel();
	HostFile=nullptr;
	if (pHostFile)
	{
		HostFile = (wchar_t*)malloc((lstrlen(pHostFile)+1)*sizeof(wchar_t));
		lstrcpy(HostFile, pHostFile);
	}
}


TmpPanel::~TmpPanel()
{
	if (!StartupOptCommonPanel)
		FreePanelItems(TmpPanelItem, TmpItemsNumber);
	if (HostFile)
		free(HostFile);
}

int TmpPanel::GetFindData(PluginPanelItem **pPanelItem,size_t *pItemsNumber,const OPERATION_MODES OpMode)
{
	IfOptCommonPanel();
	size_t Size=Info.PanelControl(this,FCTL_GETCOLUMNTYPES,0,NULL);
	wchar_t* ColumnTypes=new wchar_t[Size];
	Info.PanelControl(this,FCTL_GETCOLUMNTYPES,static_cast<int>(Size),ColumnTypes);
	UpdateItems(IsOwnersDisplayed(ColumnTypes),IsLinksDisplayed(ColumnTypes));
	delete[] ColumnTypes;
	*pPanelItem=TmpPanelItem;
	*pItemsNumber=TmpItemsNumber;
	return(TRUE);
}


void TmpPanel::GetOpenPanelInfo(struct OpenPanelInfo *opInfo)
{
	opInfo->StructSize=sizeof(*opInfo);
	opInfo->Flags=OPIF_ADDDOTS|OPIF_SHOWNAMESONLY;

	if (!Opt.SafeModePanel) opInfo->Flags|=OPIF_REALNAMES;

	opInfo->HostFile=NULL;
	if (this->HostFile)
	{
		FarGetPluginPanelItem fgppi = { sizeof(FarGetPluginPanelItem),0,NULL };
		if (0 != (fgppi.Size=Info.PanelControl(PANEL_ACTIVE,FCTL_GETCURRENTPANELITEM,0,&fgppi)))
		{
			if (NULL != (fgppi.Item=(PluginPanelItem*)malloc(fgppi.Size)))
			{
				if (Info.PanelControl(PANEL_ACTIVE,FCTL_GETCURRENTPANELITEM,0,&fgppi) && !wcscmp(fgppi.Item->FileName,L".."))
					opInfo->HostFile = this->HostFile;
				free(fgppi.Item);
			}
		}
	}

	opInfo->CurDir=L"";
	opInfo->Format=(wchar_t*)GetMsg(MTempPanel);
	static wchar_t Title[100];

	if (StartupOptCommonPanel)
		FSF.sprintf(Title,GetMsg(MTempPanelTitleNum),(Opt.SafeModePanel ? L"(R) " : L""),PanelIndex);
	else
		FSF.sprintf(Title,L" %s%s ",(Opt.SafeModePanel ? L"(R) " : L""),GetMsg(MTempPanel));

	opInfo->PanelTitle=Title;
	static struct PanelMode PanelModesArray[10];
	PanelModesArray[4].Flags=PMFLAGS_CASECONVERSION;
	if ((StartupOpenFrom==OPEN_COMMANDLINE)?Opt.FullScreenPanel:StartupOptFullScreenPanel)
		PanelModesArray[4].Flags|=PMFLAGS_FULLSCREEN;
	PanelModesArray[4].ColumnTypes=Opt.ColumnTypes;
	PanelModesArray[4].ColumnWidths=Opt.ColumnWidths;
	PanelModesArray[4].StatusColumnTypes=Opt.StatusColumnTypes;
	PanelModesArray[4].StatusColumnWidths=Opt.StatusColumnWidths;
	opInfo->PanelModesArray=PanelModesArray;
	opInfo->PanelModesNumber=ARRAYSIZE(PanelModesArray);
	opInfo->StartPanelMode=L'4';

	static WORD FKeys[]=
	{
		VK_F7,0,MF7,
		VK_F2,SHIFT_PRESSED|LEFT_ALT_PRESSED,MAltShiftF2,
		VK_F3,SHIFT_PRESSED|LEFT_ALT_PRESSED,MAltShiftF3,
		VK_F12,SHIFT_PRESSED|LEFT_ALT_PRESSED,MAltShiftF12,
	};

	static struct KeyBarLabel kbl[ARRAYSIZE(FKeys)/3];
	static struct KeyBarTitles kbt = {ARRAYSIZE(kbl), kbl};

	for (size_t j=0,i=0; i < ARRAYSIZE(FKeys); i+=3, ++j)
	{
		kbl[j].Key.VirtualKeyCode = FKeys[i];
		kbl[j].Key.ControlKeyState = FKeys[i+1];

		if (FKeys[i+2])
		{
			kbl[j].Text = kbl[j].LongText = GetMsg(FKeys[i+2]);
			if (!StartupOptCommonPanel && kbl[j].Key.VirtualKeyCode == VK_F12 && kbl[j].Key.ControlKeyState == (SHIFT_PRESSED|LEFT_ALT_PRESSED))
				kbl[j].Text = kbl[j].LongText = L"";
		}
		else
		{
			kbl[j].Text = kbl[j].LongText = L"";
		}
	}

	opInfo->KeyBar=&kbt;
}


int TmpPanel::SetDirectory(const wchar_t *Dir,const OPERATION_MODES OpMode)
{
	if ((OpMode & OPM_FIND)/* || lstrcmp(Dir,L"\\")==0*/)
		return(FALSE);

	if (lstrcmp(Dir,L"\\")==0)
		Info.PanelControl(this,FCTL_CLOSEPANEL,0,NULL);
	else
		Info.PanelControl(this,FCTL_CLOSEPANEL,0,(void*)Dir);
	return(TRUE);
}


int TmpPanel::PutFiles(struct PluginPanelItem *PanelItem,size_t ItemsNumber,int,const wchar_t *SrcPath,const OPERATION_MODES)
{
	UpdateNotNeeded=FALSE;
	HANDLE hScreen = BeginPutFiles();

	for (size_t i=0; i<ItemsNumber; i++)
	{
		if (!PutOneFile(SrcPath, PanelItem[i]))
		{
			CommitPutFiles(hScreen, FALSE);
			return FALSE;
		}
	}

	CommitPutFiles(hScreen, TRUE);
	return(1);
}

HANDLE TmpPanel::BeginPutFiles()
{
	IfOptCommonPanel();
	Opt.SelectedCopyContents = Opt.CopyContents;
	HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);
	const wchar_t *MsgItems[]={GetMsg(MTempPanel),GetMsg(MTempSendFiles)};
	Info.Message(&MainGuid, nullptr,0,NULL,MsgItems,ARRAYSIZE(MsgItems),0);
	return hScreen;
}

static inline int cmp_names(const WIN32_FIND_DATA &wfd, const PluginPanelItem &ffd)
{
	return lstrcmp(wfd.cFileName, FSF.PointToName(ffd.FileName));
}

int TmpPanel::PutDirectoryContents(const wchar_t* Path)
{
	if (Opt.SelectedCopyContents==2)
	{
		const wchar_t *MsgItems[]={GetMsg(MWarning),GetMsg(MCopyContensMsg)};
		Opt.SelectedCopyContents=!Info.Message(&MainGuid, nullptr,FMSG_MB_YESNO,L"Config",MsgItems,ARRAYSIZE(MsgItems),0);
	}

	if (Opt.SelectedCopyContents)
	{
		PluginPanelItem *DirItems;
		size_t DirItemsNumber;

		if (!Info.GetDirList(Path, &DirItems, &DirItemsNumber))
		{
			FreePanelItems(TmpPanelItem, TmpItemsNumber);
			TmpPanelItem=NULL;
			TmpItemsNumber=0;
			return FALSE;
		}

		struct PluginPanelItem *NewPanelItem=(struct PluginPanelItem *)realloc(TmpPanelItem,sizeof(*TmpPanelItem)*(TmpItemsNumber+DirItemsNumber));

		if (NewPanelItem==NULL)
			return FALSE;

		TmpPanelItem=NewPanelItem;
		memset(&TmpPanelItem[TmpItemsNumber],0,sizeof(*TmpPanelItem)*DirItemsNumber);

		for (size_t i=0; i<DirItemsNumber; i++)
		{
			struct PluginPanelItem *CurPanelItem=&TmpPanelItem[TmpItemsNumber];
			CurPanelItem->UserData.Data = (void*)TmpItemsNumber;
			TmpItemsNumber++;
			CurPanelItem->FileAttributes=DirItems[i].FileAttributes;
			CurPanelItem->CreationTime=DirItems[i].CreationTime;
			CurPanelItem->LastAccessTime=DirItems[i].LastAccessTime;
			CurPanelItem->LastWriteTime=DirItems[i].LastWriteTime;
			CurPanelItem->ChangeTime=DirItems[i].ChangeTime;
			CurPanelItem->FileSize=DirItems[i].FileSize;
			CurPanelItem->AllocationSize=DirItems[i].AllocationSize;

			CurPanelItem->FileName = wcsdup(DirItems[i].FileName);
			CurPanelItem->AlternateFileName = NULL;
		}

		Info.FreeDirList(DirItems, DirItemsNumber);
	}

	return TRUE;
}

int TmpPanel::PutOneFile(const wchar_t* SrcPath, PluginPanelItem &PanelItem)
{
	struct PluginPanelItem *NewPanelItem=(struct PluginPanelItem *)realloc(TmpPanelItem,sizeof(*TmpPanelItem)*(TmpItemsNumber+1));

	if (NewPanelItem==NULL)
		return FALSE;

	TmpPanelItem=NewPanelItem;
	struct PluginPanelItem *CurPanelItem=&TmpPanelItem[TmpItemsNumber];
	memset(CurPanelItem,0,sizeof(*CurPanelItem));
	CurPanelItem->FileAttributes=PanelItem.FileAttributes;
	CurPanelItem->CreationTime=PanelItem.CreationTime;
	CurPanelItem->LastAccessTime=PanelItem.LastAccessTime;
	CurPanelItem->LastWriteTime=PanelItem.LastWriteTime;
	CurPanelItem->ChangeTime=PanelItem.ChangeTime;
	CurPanelItem->FileSize=PanelItem.FileSize;
	CurPanelItem->AllocationSize=PanelItem.AllocationSize;
	CurPanelItem->UserData.Data = (void*)TmpItemsNumber;
	CurPanelItem->FileName = reinterpret_cast<wchar_t*>(malloc((lstrlen(SrcPath)+1+lstrlen(PanelItem.FileName)+1)*sizeof(wchar_t)));

	if (CurPanelItem->FileName==NULL)
		return FALSE;

	*(wchar_t*)CurPanelItem->FileName = L'\0';
	CurPanelItem->AlternateFileName = NULL;

	if (*SrcPath && !wcschr(PanelItem.FileName, L'\\'))
	{
		lstrcpy((wchar_t*)CurPanelItem->FileName, SrcPath);
		FSF.AddEndSlash((wchar_t*)CurPanelItem->FileName);
	}

	lstrcat((wchar_t*)CurPanelItem->FileName, PanelItem.FileName);
	TmpItemsNumber++;

	if (Opt.SelectedCopyContents && (CurPanelItem->FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return PutDirectoryContents(CurPanelItem->FileName);

	return TRUE;
}

int TmpPanel::PutOneFile(const wchar_t* FilePath)
{
	struct PluginPanelItem *NewPanelItem=(struct PluginPanelItem *)realloc(TmpPanelItem,sizeof(*TmpPanelItem)*(TmpItemsNumber+1));

	if (NewPanelItem==NULL)
		return FALSE;

	TmpPanelItem=NewPanelItem;
	struct PluginPanelItem *CurPanelItem=&TmpPanelItem[TmpItemsNumber];
	memset(CurPanelItem,0,sizeof(*CurPanelItem));
	CurPanelItem->UserData.Data = (void*)TmpItemsNumber;

	if (GetFileInfoAndValidate(FilePath, CurPanelItem, Opt.AnyInPanel))
	{
		TmpItemsNumber++;

		if (Opt.SelectedCopyContents && (CurPanelItem->FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			return PutDirectoryContents(CurPanelItem->FileName);
	}

	return TRUE;
}

void TmpPanel::CommitPutFiles(HANDLE hRestoreScreen, int Success)
{
	if (Success)
		RemoveDups();

	Info.RestoreScreen(hRestoreScreen);
}


int TmpPanel::SetFindList(const struct PluginPanelItem *PanelItem,size_t ItemsNumber)
{
	HANDLE hScreen = BeginPutFiles();
	FindSearchResultsPanel();
	FreePanelItems(TmpPanelItem, TmpItemsNumber);
	TmpItemsNumber=0;
	TmpPanelItem=(PluginPanelItem*)malloc(sizeof(PluginPanelItem)*ItemsNumber);

	if (TmpPanelItem)
	{
		TmpItemsNumber=ItemsNumber;
		memset(TmpPanelItem,0,TmpItemsNumber*sizeof(*TmpPanelItem));

		for (size_t i=0; i<ItemsNumber; ++i)
		{
			TmpPanelItem[i].UserData.Data = (void*)i;

			TmpPanelItem[i].FileAttributes=PanelItem[i].FileAttributes;
			TmpPanelItem[i].CreationTime=PanelItem[i].CreationTime;
			TmpPanelItem[i].LastAccessTime=PanelItem[i].LastAccessTime;
			TmpPanelItem[i].LastWriteTime=PanelItem[i].LastWriteTime;
			TmpPanelItem[i].ChangeTime=PanelItem[i].ChangeTime;
			TmpPanelItem[i].FileSize=PanelItem[i].FileSize;
			TmpPanelItem[i].AllocationSize=PanelItem[i].AllocationSize;

			if (PanelItem[i].FileName)
				TmpPanelItem[i].FileName = wcsdup(PanelItem[i].FileName);

			TmpPanelItem[i].AlternateFileName = NULL;
		}
	}

	CommitPutFiles(hScreen, TRUE);
	UpdateNotNeeded=TRUE;
	return(TRUE);
}


void TmpPanel::FindSearchResultsPanel()
{
	if (StartupOptCommonPanel)
	{
		if (!Opt.NewPanelForSearchResults)
			IfOptCommonPanel();
		else
		{
			int SearchResultsPanel = -1;

			for (int i=0; i<COMMONPANELSNUMBER; i++)
			{
				if (CommonPanels [i].ItemsNumber == 0)
				{
					SearchResultsPanel = i;
					break;
				}
			}

			if (SearchResultsPanel < 0)
			{
				// all panels are full - use least recently used panel
				SearchResultsPanel = Opt.LastSearchResultsPanel++;

				if (Opt.LastSearchResultsPanel >= COMMONPANELSNUMBER)
					Opt.LastSearchResultsPanel = 0;
			}

			if (PanelIndex != SearchResultsPanel)
			{
				CommonPanels[PanelIndex].Items = TmpPanelItem;
				CommonPanels[PanelIndex].ItemsNumber = (UINT)TmpItemsNumber;
				PanelIndex = SearchResultsPanel;
				TmpPanelItem = CommonPanels[PanelIndex].Items;
				TmpItemsNumber = CommonPanels[PanelIndex].ItemsNumber;
			}

			CurrentCommonPanel = PanelIndex;
		}
	}
}

int WINAPI SortListCmp(const void *el1, const void *el2, void *userparam)
{
	PluginPanelItem* TmpPanelItem = reinterpret_cast<PluginPanelItem*>(userparam);
	size_t idx1 = *reinterpret_cast<const size_t*>(el1);
	size_t idx2 = *reinterpret_cast<const size_t*>(el2);
	int res = lstrcmp(TmpPanelItem[idx1].FileName, TmpPanelItem[idx2].FileName);

	if (res == 0)
	{
		if (idx1 < idx2) return -1;
		else if (idx1 == idx2) return 0;
		else return 1;
	}
	else
		return res;
}

void TmpPanel::RemoveDups()
{
	size_t* indices = reinterpret_cast<size_t*>(malloc(TmpItemsNumber*sizeof(size_t)));

	if (indices == NULL)
		return;

	for (size_t i = 0; i < TmpItemsNumber; i++)
		indices[i] = i;

	FSF.qsort(indices, TmpItemsNumber, sizeof(*indices), SortListCmp, TmpPanelItem);

	for (size_t i = 0; i + 1 < TmpItemsNumber; i++)
		if (lstrcmp(TmpPanelItem[indices[i]].FileName, TmpPanelItem[indices[i + 1]].FileName) == 0)
			TmpPanelItem[indices[i + 1]].Flags |= REMOVE_FLAG;

	free(indices);
	RemoveEmptyItems();
}

void TmpPanel::RemoveEmptyItems()
{
	int EmptyCount=0;
	struct PluginPanelItem *CurItem=TmpPanelItem;

	for (size_t i=0; i<TmpItemsNumber; i++,CurItem++)
		if (CurItem->Flags & REMOVE_FLAG)
		{
			if (CurItem->Owner)
			{
				free((void*)CurItem->Owner);
				CurItem->Owner = NULL;
			}

			if (CurItem->FileName)
			{
				free((wchar_t*)CurItem->FileName);
				CurItem->FileName = NULL;
			}
			EmptyCount++;
		}
		else if (EmptyCount)
		{
			CurItem->UserData.Data = (void*)((intptr_t)CurItem->UserData.Data - EmptyCount);
			*(CurItem-EmptyCount)=*CurItem;
			memset(CurItem, 0, sizeof(*CurItem));
		}

	TmpItemsNumber-=EmptyCount;

	if (EmptyCount>1)
		TmpPanelItem=(struct PluginPanelItem *)realloc(TmpPanelItem,sizeof(*TmpPanelItem)*(TmpItemsNumber+1));

	if (StartupOptCommonPanel)
	{
		CommonPanels[PanelIndex].Items=TmpPanelItem;
		CommonPanels[PanelIndex].ItemsNumber=(UINT)TmpItemsNumber;
	}
}


void TmpPanel::UpdateItems(int ShowOwners,int ShowLinks)
{
	if (UpdateNotNeeded || TmpItemsNumber == 0)
	{
		UpdateNotNeeded=FALSE;
		return;
	}

	HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);
	const wchar_t *MsgItems[]={GetMsg(MTempPanel),GetMsg(MTempUpdate)};
	Info.Message(&MainGuid, nullptr,0,NULL,MsgItems,ARRAYSIZE(MsgItems),0);
	LastOwnersRead=ShowOwners;
	LastLinksRead=ShowLinks;
	struct PluginPanelItem *CurItem=TmpPanelItem;

	for (size_t i=0; i<TmpItemsNumber; i++,CurItem++)
	{
		HANDLE FindHandle;
		const wchar_t *lpFullName = CurItem->FileName;
		const wchar_t *lpSlash = wcsrchr(lpFullName,L'\\');
		int Length=lpSlash ? (int)(lpSlash-lpFullName+1):0;
		int SameFolderItems=1;

		/* $ 23.12.2001 DJ
		   если FullName - это каталог, то FindFirstFile (FullName+"*.*")
		   этот каталог не найдет. Поэтому для каталогов оптимизацию с
		   SameFolderItems пропускаем.
		*/
		if (Length>0 && Length > (int)lstrlen(lpFullName))    /* DJ $ */
		{
			for (size_t j=1; i+j<TmpItemsNumber; j++)
			{
				if (memcmp(lpFullName,CurItem[j].FileName,Length*sizeof(wchar_t))==0 &&
				        wcschr((const wchar_t*)CurItem[j].FileName+Length,L'\\')==NULL)
				{
					SameFolderItems++;
				}
				else
				{
					break;
				}
			}
		}

		// SameFolderItems - оптимизация для случая, когда в панели лежат
		// несколько файлов из одного и того же каталога. При этом
		// FindFirstFile() делается один раз на каталог, а не отдельно для
		// каждого файла.
		if (SameFolderItems>2)
		{
			WIN32_FIND_DATA FindData;
			StrBuf FindFile((int)(lpSlash-lpFullName)+1+1+1);
			lstrcpyn(FindFile, lpFullName, (int)(lpSlash-lpFullName)+1);
			lstrcpy(FindFile+(lpSlash+1-lpFullName),L"*");
			StrBuf NtPath;
			FormNtPath(FindFile, NtPath);

			for (int J=0; J<SameFolderItems; J++)
				CurItem[J].Flags|=REMOVE_FLAG;

			int Done=(FindHandle=FindFirstFile(NtPath,&FindData))==INVALID_HANDLE_VALUE;

			while (!Done)
			{
				for (int J=0; J<SameFolderItems; J++)
				{
					if ((CurItem[J].Flags & 1) && cmp_names(FindData, CurItem[J])==0)
					{
						CurItem[J].Flags&=~REMOVE_FLAG;
						const wchar_t *save = CurItem[J].FileName;
						WFD2FFD(FindData,CurItem[J]);
						free((wchar_t*)CurItem[J].FileName);
						CurItem[J].FileName = save;
						break;
					}
				}

				Done=!FindNextFile(FindHandle,&FindData);
			}

			FindClose(FindHandle);
			i+=SameFolderItems-1;
			CurItem+=SameFolderItems-1;
		}
		else
		{
			if (!GetFileInfoAndValidate(lpFullName,CurItem,Opt.AnyInPanel))
				CurItem->Flags|=REMOVE_FLAG;
		}
	}

	RemoveEmptyItems();

	if (ShowOwners || ShowLinks)
	{
		struct PluginPanelItem *CurItem=TmpPanelItem;

		for (size_t i=0; i<TmpItemsNumber; i++,CurItem++)
		{
			if (ShowOwners)
			{
				wchar_t Owner[80];

				if (CurItem->Owner)
				{
					free((void*)CurItem->Owner);
					CurItem->Owner=NULL;
				}

				if (FSF.GetFileOwner(NULL,CurItem->FileName,Owner,80))
					CurItem->Owner=wcsdup(Owner);
			}

			if (ShowLinks)
				CurItem->NumberOfLinks=FSF.GetNumberOfLinks(CurItem->FileName);
		}
	}

	Info.RestoreScreen(hScreen);
}


int TmpPanel::ProcessEvent(intptr_t Event,void *)
{
	if (Event==FE_CHANGEVIEWMODE)
	{
		IfOptCommonPanel();
		size_t Size=Info.PanelControl(this,FCTL_GETCOLUMNTYPES,0,NULL);
		wchar_t* ColumnTypes=new wchar_t[Size];
		Info.PanelControl(this,FCTL_GETCOLUMNTYPES,static_cast<int>(Size),ColumnTypes);
		int UpdateOwners=IsOwnersDisplayed(ColumnTypes) && !LastOwnersRead;
		int UpdateLinks=IsLinksDisplayed(ColumnTypes) && !LastLinksRead;
		delete[] ColumnTypes;

		if (UpdateOwners || UpdateLinks)
		{
			UpdateItems(UpdateOwners,UpdateLinks);
			Info.PanelControl(this,FCTL_UPDATEPANEL,TRUE,NULL);
			Info.PanelControl(this,FCTL_REDRAWPANEL,0,NULL);
		}
	}

	return(FALSE);
}


bool TmpPanel::IsCurrentFileCorrect(wchar_t **pCurFileName)
{
	struct PanelInfo PInfo = {sizeof(PanelInfo)};
	const wchar_t *CurFileName=NULL;

	if (pCurFileName)
		*pCurFileName = NULL;

	Info.PanelControl(this,FCTL_GETPANELINFO,0,&PInfo);
	size_t Size=Info.PanelControl(this,FCTL_GETPANELITEM,PInfo.CurrentItem,0);
	PluginPanelItem* PPI=(PluginPanelItem*)malloc(Size);

	if (PPI)
	{
		FarGetPluginPanelItem gpi={sizeof(FarGetPluginPanelItem), Size, PPI};
		Info.PanelControl(this,FCTL_GETPANELITEM,PInfo.CurrentItem,&gpi);
		CurFileName = PPI->FileName;
	}
	else
	{
		return false;
	}

	bool IsCorrectFile = false;

	if (lstrcmp(CurFileName, L"..") == 0)
	{
		IsCorrectFile = true;
	}
	else
	{
		PluginPanelItem TempFindData;
		IsCorrectFile=GetFileInfoAndValidate(CurFileName,&TempFindData,FALSE);
	}

	if (pCurFileName)
	{
		*pCurFileName = (wchar_t *) malloc((lstrlen(CurFileName)+1)*sizeof(wchar_t));
		lstrcpy(*pCurFileName, CurFileName);
	}

	free(PPI);
	return IsCorrectFile;
}

int TmpPanel::ProcessKey(const INPUT_RECORD *Rec)
{
	if (Rec->EventType != KEY_EVENT)
		return FALSE;

	int Key=Rec->Event.KeyEvent.wVirtualKeyCode;
	unsigned int ControlState=Rec->Event.KeyEvent.dwControlKeyState;

	if (ControlState==(SHIFT_PRESSED|LEFT_ALT_PRESSED) && Key==VK_F3)
	{
		PtrGuard CurFileName;

		if (IsCurrentFileCorrect(CurFileName.PtrPtr()))
		{
			struct PanelInfo PInfo = {sizeof(PanelInfo)};
			Info.PanelControl(this,FCTL_GETPANELINFO,0,&PInfo);

			if (lstrcmp(CurFileName,L"..")!=0)
			{
				size_t Size=Info.PanelControl(this,FCTL_GETPANELITEM,PInfo.CurrentItem,0);
				PluginPanelItem* PPI=(PluginPanelItem*)malloc(Size);
				DWORD attributes=0;

				if (PPI)
				{
					FarGetPluginPanelItem gpi={sizeof(FarGetPluginPanelItem), Size, PPI};
					Info.PanelControl(this,FCTL_GETPANELITEM,PInfo.CurrentItem,&gpi);
					attributes=(DWORD)PPI->FileAttributes;
					free(PPI);
				}

				if (attributes&FILE_ATTRIBUTE_DIRECTORY)
				{
					FarPanelDirectory dirInfo = {sizeof(dirInfo), CurFileName, nullptr, {}, nullptr};
					Info.PanelControl(PANEL_PASSIVE, FCTL_SETPANELDIRECTORY, 0, &dirInfo);
				}
				else
				{
					GoToFile(CurFileName, true);
				}

				Info.PanelControl(PANEL_PASSIVE, FCTL_REDRAWPANEL,0,NULL);
				return(TRUE);
			}
		}
	}

	if (ControlState!=LEFT_CTRL_PRESSED && Key>=VK_F3 && Key<=VK_F8 && Key!=VK_F7)
	{
		if (!IsCurrentFileCorrect(NULL))
			return(TRUE);
	}

	if (ControlState==0 && Key==VK_RETURN && Opt.AnyInPanel)
	{
		PtrGuard CurFileName;

		if (!IsCurrentFileCorrect(CurFileName.PtrPtr()))
		{
			Info.PanelControl(this,FCTL_SETCMDLINE,0,CurFileName.Ptr());
			return(TRUE);
		}
	}

	if (Opt.SafeModePanel && ControlState == LEFT_CTRL_PRESSED && Key == VK_PRIOR)
	{
		PtrGuard CurFileName;

		if (IsCurrentFileCorrect(CurFileName.PtrPtr()))
		{
			if (lstrcmp(CurFileName,L".."))
			{
				GoToFile(CurFileName, false);
				return TRUE;
			}
		}

		if (CurFileName.Ptr() && !lstrcmp(CurFileName,L".."))
		{
			SetDirectory(L".",0);
			return TRUE;
		}
	}

	if (ControlState==0 && Key==VK_F7)
	{
		ProcessRemoveKey();
		return TRUE;
	}
	else if (ControlState == (SHIFT_PRESSED|LEFT_ALT_PRESSED) && Key == VK_F2)
	{
		ProcessSaveListKey();
		return TRUE;
	}
	else
	{
		if (StartupOptCommonPanel && ControlState==(SHIFT_PRESSED|LEFT_ALT_PRESSED))
		{
			if (Key==VK_F12)
			{
				ProcessPanelSwitchMenu();
				return(TRUE);
			}
			else if (Key >= L'0' && Key <= L'9')
			{
				SwitchToPanel(Key - L'0');
				return TRUE;
			}
		}
	}

	return(FALSE);
}


void TmpPanel::ProcessRemoveKey()
{
	IfOptCommonPanel();
	struct PanelInfo PInfo = {sizeof(PanelInfo)};
	Info.PanelControl(this,FCTL_GETPANELINFO,0,&PInfo);

	for (size_t i=0; i<PInfo.SelectedItemsNumber; i++)
	{
		struct PluginPanelItem *RemovedItem=NULL;
		size_t Size=Info.PanelControl(this,FCTL_GETSELECTEDPANELITEM,i,0);
		PluginPanelItem* PPI=(PluginPanelItem*)malloc(Size);

		if (PPI)
		{
			FarGetPluginPanelItem gpi={sizeof(FarGetPluginPanelItem), Size, PPI};
			Info.PanelControl(this,FCTL_GETSELECTEDPANELITEM,i,&gpi);
			RemovedItem = TmpPanelItem + (size_t)PPI->UserData.Data;
		}

		if (RemovedItem!=NULL)
			RemovedItem->Flags|=REMOVE_FLAG;

		free(PPI);
	}

	RemoveEmptyItems();
	Info.PanelControl(this,FCTL_UPDATEPANEL,0,NULL);
	Info.PanelControl(this,FCTL_REDRAWPANEL,0,NULL);
	Info.PanelControl(PANEL_PASSIVE,FCTL_GETPANELINFO,0,&PInfo);

	if (PInfo.PanelType==PTYPE_QVIEWPANEL)
	{
		Info.PanelControl(PANEL_PASSIVE,FCTL_UPDATEPANEL,0,NULL);
		Info.PanelControl(PANEL_PASSIVE,FCTL_REDRAWPANEL,0,NULL);
	}
}

void TmpPanel::ProcessSaveListKey()
{
	IfOptCommonPanel();

	if (TmpItemsNumber == 0)
		return;

	// default path: opposite panel directory\panel<index>.<mask extension>
	string ListPath;
	size_t Size = Info.PanelControl(PANEL_PASSIVE, FCTL_GETPANELDIRECTORY, 0, nullptr);
	FarPanelDirectory* dir = static_cast<FarPanelDirectory*>(malloc(Size));
	dir->StructSize = sizeof(FarPanelDirectory);
	Info.PanelControl(PANEL_PASSIVE, FCTL_GETPANELDIRECTORY, Size, dir);
	ListPath = dir->Name;
	free(dir);
	if(ListPath.At(ListPath.Len()-1) != L'\\')
	{
		ListPath+=L"\\";
	}
	ListPath += L"panel";

	if (Opt.CommonPanel)
	{
		wchar_t Index[32];
		FSF.itoa(PanelIndex, Index, 10);
		ListPath += Index;
	}

	wchar_t ExtBuf [512];
	lstrcpy(ExtBuf, Opt.Mask);
	wchar_t *comma = wcschr(ExtBuf, L',');

	if (comma)
		*comma = L'\0';

	wchar_t *ext = wcschr(ExtBuf, L'.');

	if (ext && !wcschr(ext, L'*') && !wcschr(ext, L'?'))
	{
		ListPath += ext;
	}

	wchar_t* Buffer = new wchar_t[NT_MAX_PATH];
	if (Info.InputBox(&MainGuid, nullptr, GetMsg(MTempPanel), GetMsg(MListFilePath),
	                  L"TmpPanel.SaveList", ListPath, Buffer, NT_MAX_PATH,
	                  NULL, FIB_BUTTONS))
	{
		ListPath = Buffer;
		SaveListFile(ListPath);
		Info.PanelControl(PANEL_PASSIVE, FCTL_UPDATEPANEL,0,NULL);
		Info.PanelControl(PANEL_PASSIVE, FCTL_REDRAWPANEL,0,NULL);
	}
	delete[] Buffer;

#undef _HANDLE
#undef _UPDATE
#undef _REDRAW
#undef _GET
}

void TmpPanel::SaveListFile(const wchar_t *Path)
{
	IfOptCommonPanel();

	if (!TmpItemsNumber)
		return;

	StrBuf FullPath;
	GetFullPath(Path, FullPath);
	StrBuf NtPath;
	FormNtPath(FullPath, NtPath);
	HANDLE hFile = CreateFile(NtPath, GENERIC_WRITE, 0, NULL,
	                          CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		const wchar_t *Items[] = { GetMsg(MError) };
		Info.Message(&MainGuid, nullptr, FMSG_WARNING | FMSG_ERRORTYPE | FMSG_MB_OK, NULL, Items, 1, 0);
		return;
	}

	DWORD BytesWritten;

	if (!Opt.ListUTF8)
	{
		static const unsigned short bom = SIGN_UNICODE;
		WriteFile(hFile, &bom, sizeof(bom), &BytesWritten, NULL);
	}
	else
	{
		static const unsigned char bomhi = SIGN_UTF8_HI;
		static const unsigned short bomlo = SIGN_UTF8_LO;
		WriteFile(hFile, &bomlo, sizeof(bomlo), &BytesWritten, NULL);
		WriteFile(hFile, &bomhi, sizeof(bomhi), &BytesWritten, NULL);
	}

	size_t i = 0;

	do
	{
		const wchar_t *FName = TmpPanelItem[i].FileName;

		if (!Opt.ListUTF8)
		{
			static const wchar_t *CRLF = L"\r\n";
			WriteFile(hFile, FName, sizeof(wchar_t)*lstrlen(FName), &BytesWritten, NULL);
			WriteFile(hFile, CRLF, 2*sizeof(wchar_t), &BytesWritten, NULL);
		}
		else
		{
			static const char *CRLF = "\r\n";
			char *dest=new char[lstrlen(FName)*3+1];
			WideCharToMultiByte(CP_UTF8, 0, FName, -1, dest, (int) lstrlen(FName)*3+1, nullptr, nullptr);
			WriteFile(hFile, dest, (DWORD)strlen(dest), &BytesWritten, NULL);
			delete[] dest;
			WriteFile(hFile, CRLF, 2*sizeof(char), &BytesWritten, NULL);
		}
	}
	while (++i < TmpItemsNumber);

	CloseHandle(hFile);
}

void TmpPanel::SwitchToPanel(int NewPanelIndex)
{
	if ((unsigned)NewPanelIndex<COMMONPANELSNUMBER && NewPanelIndex!=(int)PanelIndex)
	{
		CommonPanels[PanelIndex].Items=TmpPanelItem;
		CommonPanels[PanelIndex].ItemsNumber=(UINT)TmpItemsNumber;

		if (!CommonPanels[NewPanelIndex].Items)
		{
			CommonPanels[NewPanelIndex].ItemsNumber=0;
			CommonPanels[NewPanelIndex].Items=(PluginPanelItem*)calloc(1,sizeof(PluginPanelItem));
		}

		if (CommonPanels[NewPanelIndex].Items)
		{
			CurrentCommonPanel = PanelIndex = NewPanelIndex;
			Info.PanelControl(this,FCTL_UPDATEPANEL,0,NULL);
			Info.PanelControl(this,FCTL_REDRAWPANEL,0,NULL);
		}
	}
}


void TmpPanel::ProcessPanelSwitchMenu()
{
	FarMenuItem fmi[COMMONPANELSNUMBER];
	memset(&fmi,0,sizeof(FarMenuItem)*COMMONPANELSNUMBER);
	const wchar_t *txt=GetMsg(MSwitchMenuTxt);
	wchar_t tmpstr[COMMONPANELSNUMBER][128];
	static const wchar_t fmt1[]=L"&%c. %s %d";

	for (unsigned int i=0; i<COMMONPANELSNUMBER; ++i)
	{
		fmi[i].Text = tmpstr[i];

		if (i<10)
			FSF.sprintf(tmpstr[i],fmt1,L'0'+i,txt,CommonPanels[i].ItemsNumber);
		else if (i<36)
			FSF.sprintf(tmpstr[i],fmt1,L'A'-10+i,txt,CommonPanels[i].ItemsNumber);
		else
			FSF.sprintf(tmpstr[i],L"   %s %d",txt,CommonPanels[i].ItemsNumber);

	}

	fmi[PanelIndex].Flags|=MIF_SELECTED;
	int ExitCode=(int)Info.Menu(&MainGuid, nullptr,-1,-1,0,
	                       FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,
	                       GetMsg(MSwitchMenuTitle),NULL,NULL,
	                       NULL,NULL,fmi,COMMONPANELSNUMBER);
	SwitchToPanel(ExitCode);
}

int TmpPanel::IsOwnersDisplayed(LPCTSTR ColumnTypes)
{
	for (int i=0; ColumnTypes[i]; i++)
		if (ColumnTypes[i]==L'O' && (i==0 || ColumnTypes[i-1]==L',') &&
		        (ColumnTypes[i+1]==L',' || ColumnTypes[i+1]==0))
			return(TRUE);

	return(FALSE);
}


int TmpPanel::IsLinksDisplayed(LPCTSTR ColumnTypes)
{
	for (int i=0; ColumnTypes[i]; i++)
		if (ColumnTypes[i]==L'L' && ColumnTypes[i+1]==L'N' &&
		        (i==0 || ColumnTypes[i-1]==L',') &&
		        (ColumnTypes[i+2]==L',' || ColumnTypes[i+2]==0))
			return(TRUE);

	return(FALSE);
}

inline bool isDevice(const wchar_t* FileName, const wchar_t* dev_begin)
{
	const int len=(int)lstrlen(dev_begin);

	if (FSF.LStrnicmp(FileName, dev_begin, len)) return false;

	FileName+=len;

	if (!*FileName) return false;

	while (*FileName>=L'0' && *FileName<=L'9') FileName++;

	return !*FileName;
}

bool TmpPanel::GetFileInfoAndValidate(const wchar_t *FilePath, /*FAR_FIND_DATA*/PluginPanelItem* FindData, int Any)
{
	StrBuf ExpFilePath;
	ExpandEnvStrs(FilePath,ExpFilePath);
	wchar_t* FileName = ExpFilePath;
	ParseParam(FileName);
	StrBuf FullPath;
	GetFullPath(FileName, FullPath);
	StrBuf NtPath;
	FormNtPath(FullPath, NtPath);

	if (!FSF.LStrnicmp(FileName, L"\\\\.\\", 4) && FSF.LIsAlpha(FileName[4]) && FileName[5]==L':' && FileName[6]==0)
	{
copy_name_set_attr:
		FindData->FileAttributes = FILE_ATTRIBUTE_ARCHIVE;
copy_name:
		if (FindData->FileName)
			free((void*)FindData->FileName);

		FindData->FileName = wcsdup(FileName);
		return(TRUE);
	}

	if (isDevice(FileName, L"\\\\.\\PhysicalDrive") || isDevice(FileName, L"\\\\.\\cdrom"))
		goto copy_name_set_attr;

	if (lstrlen(FileName))
	{
		DWORD dwAttr=GetFileAttributes(NtPath);

		if (dwAttr!=INVALID_FILE_ATTRIBUTES)
		{
			WIN32_FIND_DATA wfd;
			HANDLE fff=FindFirstFile(NtPath, &wfd);

			if (fff != INVALID_HANDLE_VALUE)
			{
				WFD2FFD(wfd,*FindData);
				FindClose(fff);
				FileName = FullPath;
				goto copy_name;
			}
			else
			{
				wfd.dwFileAttributes=dwAttr;
				HANDLE hFile=CreateFile(NtPath,FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_POSIX_SEMANTICS,NULL);

				if (hFile!=INVALID_HANDLE_VALUE)
				{
					GetFileTime(hFile, &wfd.ftCreationTime, &wfd.ftLastAccessTime, &wfd.ftLastWriteTime);
					wfd.nFileSizeLow = GetFileSize(hFile, &wfd.nFileSizeHigh);
					CloseHandle(hFile);
				}

				wfd.dwReserved0=0;
				wfd.dwReserved1=0;
				WFD2FFD(wfd, *FindData);
				FileName = FullPath;
				goto copy_name;
			}
		}

		if (Any)
			goto copy_name_set_attr;
	}

	return(FALSE);
}


void TmpPanel::IfOptCommonPanel(void)
{
	if (StartupOptCommonPanel)
	{
		TmpPanelItem=CommonPanels[PanelIndex].Items;
		TmpItemsNumber=CommonPanels[PanelIndex].ItemsNumber;
	}
}
