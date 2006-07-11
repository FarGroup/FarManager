/*
flupdate.cpp

�������� ������ - ������ ���� ������

*/

/* Revision: 1.74 12.07.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "filelist.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "plugin.hpp"
#include "colors.hpp"
#include "lang.hpp"
#include "filepanels.hpp"
#include "cmdline.hpp"
#include "filter.hpp"
#include "hilight.hpp"
#include "grpsort.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"

int _cdecl SortSearchList(const void *el1,const void *el2);

/* $ 19.03.2002 DJ
   ��������� UPDATE_IGNORE_VISIBLE
*/

void FileList::Update(int Mode)
{
  _ALGO(CleverSysLog clv("FileList::Update"));
  _ALGO(SysLog("(Mode=[%d/0x%08X] %s)",Mode,Mode,(Mode==UPDATE_KEEP_SELECTION?"UPDATE_KEEP_SELECTION":"")));

  if (EnableUpdate)
    switch(PanelMode)
    {
      case NORMAL_PANEL:
        ReadFileNames(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE,Mode & UPDATE_DRAW_MESSAGE);
        break;
      case PLUGIN_PANEL:
        {
          struct OpenPluginInfoW Info;
          CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
          ProcessPluginCommand();
          if (PanelMode!=PLUGIN_PANEL)
            ReadFileNames(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE,Mode & UPDATE_DRAW_MESSAGE);
          else
            if ((Info.Flags & OPIF_REALNAMES) ||
                CtrlObject->Cp()->GetAnotherPanel(this)->GetMode()==PLUGIN_PANEL ||
                (Mode & UPDATE_SECONDARY)==0)
              UpdatePlugin(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE);
        }
        ProcessPluginCommand();
        break;
    }
  LastUpdateTime=clock();
}

/* DJ $ */

/* $ 19.03.2002 DJ
*/

void FileList::UpdateIfRequired()
{
  if (UpdateRequired)
  {
    UpdateRequired = FALSE;
    Update (UpdateRequiredMode | UPDATE_IGNORE_VISIBLE);
  }
}

/* DJ $ */


void ReadFileNamesMsg(const wchar_t *Msg)
{
  MessageW(0,0,UMSG(MReadingTitleFiles),Msg);
  PreRedrawParam.Param1=(void*)Msg;
}

static void PR_ReadFileNamesMsg(void)
{
  ReadFileNamesMsg((wchar_t *)PreRedrawParam.Param1);
}


// ��� ���� ����� ����� ��� ���������� ������������� Far Manafer
// ��� ���������� �����������

/* $ 19.03.2002 DJ
   IgnoreVisible
*/

void FileList::ReadFileNames(int KeepSelection, int IgnoreVisible, int DrawMessage)
{
  if (!IsVisible() && !IgnoreVisible)   /* DJ $ */
  {
    UpdateRequired=TRUE;
    UpdateRequiredMode=KeepSelection;
    return;
  }
  Is_FS_NTFS=FALSE;
  AccessTimeUpdateRequired=FALSE;
  DizRead=FALSE;
  HANDLE FindHandle;
  FAR_FIND_DATA_EX fdata;
  struct FileListItem *CurPtr=0,**OldData=0;
  string strCurName, strNextCurName;
  long OldFileCount=0;
  int Done;
  int I;

  clock_t StartTime=clock();

  CloseChangeNotification();

  if (this!=CtrlObject->Cp()->LeftPanel && this!=CtrlObject->Cp()->RightPanel )
    return;

  string strSaveDir;
  FarGetCurDirW(strSaveDir);
  {
    string strOldCurDir = strCurDir;
    if (!SetCurPath())
    {
      FlushInputBuffer(); // ������� ������ �����, �.�. �� ��� ����� ���� � ������ �����...
      if (wcscmp(strCurDir, strOldCurDir) == 0) //?? i??
      {
        GetPathRootOneW(strOldCurDir,strOldCurDir);
        if(!IsDiskInDriveW(strOldCurDir))
          IfGoHomeW(strOldCurDir.At(0));
        /* ��� ����� �������� ���� �� ��������� */
        return;
      }
    }
  }

  SortGroupsRead=FALSE;

  if (Filter==NULL)
    Filter=new PanelFilter(this);

  if (GetFocus())
    CtrlObject->CmdLine->SetCurDirW(strCurDir);

  {
    string strFileSysName;
    string strRootDir;

    ConvertNameToFullW(strCurDir,strRootDir);
    GetPathRootW(strRootDir, strRootDir);
    if ( apiGetVolumeInformation (strRootDir,NULL,NULL,NULL,NULL,&strFileSysName))
      Is_FS_NTFS=!LocalStricmpW(strFileSysName,L"NTFS")?TRUE:FALSE;
  }

  LastCurFile=-1;

  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  AnotherPanel->QViewDelTempName();

  int PrevSelFileCount=SelFileCount;

  SelFileCount=0;
  SelFileSize=0;
  TotalFileCount=0;
  TotalFileSize=0;

  if (Opt.ShowPanelFree)
  {
    unsigned __int64 TotalSize,TotalFree;
    string strDriveRoot;
    GetPathRootW(strCurDir,strDriveRoot);
    if (!GetDiskSizeW(strDriveRoot,&TotalSize,&TotalFree,&FreeDiskSize))
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

  int DotsPresent=0;

  int ReadOwners=IsColumnDisplayed(OWNER_COLUMN);
  int ReadPacked=IsColumnDisplayed(PACKED_COLUMN);
  int ReadNumLinks=IsColumnDisplayed(NUMLINK_COLUMN);

  string strComputerName;

  if (ReadOwners)
  {
    CurPath2ComputerName(strCurDir, strComputerName);
    // ������� ��� SID`��
    SIDCacheFlush();
  }

  SetLastError(0);

  //BUGBUG!!!
  Done=((FindHandle=apiFindFirstFile(L"*.*",&fdata))==INVALID_HANDLE_VALUE);

  int AllocatedCount=0;
  struct FileListItem *NewPtr;

  // ��� ����� ������� ���������.
  const wchar_t *PointToName_CurDir=PointToNameW(strCurDir);

  // ���������� ��������� ��� �����
  char Title[2048];
  int TitleLength=Min((int)X2-X1-1,(int)sizeof(Title)-1);
  memset(Title,0x0CD,TitleLength);
  Title[TitleLength]=0;
  BOOL IsShowTitle=FALSE;
  BOOL NeedHighlight=Opt.Highlight && PanelMode != PLUGIN_PANEL;

  for (FileCount=0; !Done; )
  {
    if ((fdata.strFileName.At(0) != L'.' || fdata.strFileName.At(1) != 0) &&
        (Opt.ShowHidden || (fdata.dwFileAttributes & (FA_HIDDEN|FA_SYSTEM))==0) &&
        ((fdata.dwFileAttributes & FA_DIREC) ||
        Filter->CheckNameW(fdata.strFileName)))
    {
      int UpperDir=FALSE;
      if (fdata.strFileName.At(0) == L'.' && fdata.strFileName.At(1) == L'.' && fdata.strFileName.At(2) == 0)
      {
        UpperDir=TRUE;
        DotsPresent=TRUE;
        if (*PointToName_CurDir==0)
        {
          Done=!apiFindNextFile (FindHandle,&fdata);
          continue;
        }
      }
      if (FileCount>=AllocatedCount)
      {
        AllocatedCount=AllocatedCount+256+AllocatedCount/4;

        FileListItem **pTemp;

        if ((pTemp=(struct FileListItem **)xf_realloc(ListData,AllocatedCount*4))==NULL)
          break;
        ListData=pTemp;
      }

      ListData[FileCount] = new FileListItem;

      memset (ListData[FileCount], 0, sizeof(FileListItem)); //BUGBUG, Clear!

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

      if ((fdata.dwFileAttributes & FA_DIREC) == 0)
      {
        TotalFileSize += NewPtr->UnpSize;
        int Compressed=FALSE;
        if (ReadPacked && (fdata.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED))
        {
          DWORD dwLoPart, dwHighPart;

          dwLoPart = GetCompressedFileSizeW (fdata.strFileName, &dwHighPart);

          if ( (dwLoPart != (DWORD)-1) || (GetLastError () != NO_ERROR) )
          {
            NewPtr->PackSize = dwHighPart*0x100000000+dwLoPart;
            Compressed=TRUE;
          }
        }
        if (!Compressed)
          NewPtr->PackSize = fdata.nFileSize;
        if (ReadNumLinks)
          NewPtr->NumberOfLinks=GetNumberOfLinksW(fdata.strFileName);
      }
      else
        NewPtr->PackSize = 0;

      NewPtr->SortGroup=DEFAULT_SORT_GROUP;
      if (ReadOwners)
      {
        string strOwner;
        GetFileOwnerW(strComputerName, NewPtr->strName,strOwner);
        NewPtr->strOwner = strOwner;
      }

      if (NeedHighlight)
        CtrlObject->HiFiles->GetHiColor(&NewPtr,1);

      if (!UpperDir && (fdata.dwFileAttributes & FA_DIREC)==0)
        TotalFileCount++;

      //memcpy(ListData+FileCount,&NewPtr,sizeof(NewPtr));
//      FileCount++;

      if ((FileCount & 0x3f)==0 && clock()-StartTime>1000)
      {

        if (IsVisible())
        {
          string strReadMsg;
          if(!IsShowTitle)
          {
            if(DrawMessage)
              SetPreRedrawFunc(PR_ReadFileNamesMsg);
            else
            {
              Text(X1+1,Y1,COL_PANELBOX,Title);
              IsShowTitle=TRUE;
              SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
            }
          }

          strReadMsg.Format (UMSG(MReadingFiles),FileCount);
          if(DrawMessage)
            ReadFileNamesMsg(strReadMsg);
          else
          {
            TruncStrW(strReadMsg,TitleLength-2);
            int MsgLength=strReadMsg.GetLength();
            GotoXY(X1+1+(TitleLength-MsgLength-1)/2,Y1);
            mprintfW(L" %s ", (const wchar_t*)strReadMsg);
          }
        }
        if (CheckForEsc())
        {
          MessageW(MSG_WARNING,1,UMSG(MUserBreakTitle),UMSG(MOperationNotCompleted),UMSG(MOk));
          break;
        }
      }
    }
    Done=!apiFindNextFile(FindHandle,&fdata);
  }

  SetPreRedrawFunc(NULL);

  int ErrCode=GetLastError();
  if (!(ErrCode==ERROR_SUCCESS || ErrCode==ERROR_NO_MORE_FILES || ErrCode==ERROR_FILE_NOT_FOUND ||
        (ErrCode==ERROR_BAD_PATHNAME && WinVer.dwPlatformId != VER_PLATFORM_WIN32_NT && Opt.IgnoreErrorBadPathName)))
    MessageW(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MError),UMSG(MReadFolderError),UMSG(MOk));

  FindClose(FindHandle);

  // "����������" ������� � ��������� ���� - �� ��������� ������� �����������
  // ������ ������� �������, � ��������� �����.
//  UpdateColorItems();

  if (IsColumnDisplayed(DIZ_COLUMN))
    ReadDiz();

  int NetRoot=FALSE;
  if (strCurDir.At(0)==L'\\' && strCurDir.At(1)==L'\\')
  {
    const wchar_t *ChPtr=wcschr((const wchar_t*)strCurDir+2,'\\');
    if (ChPtr==NULL || wcschr(ChPtr+1,L'\\')==NULL)
      NetRoot=TRUE;
  }
  // ���� ����� �����������, �������� �� ���� � �� ����������.
  const wchar_t *lpwszName = PointToNameW(strCurDir);

  if (!DotsPresent && *lpwszName )// && !NetRoot)
  {
    if (FileCount>=AllocatedCount)
    {
        FileListItem **pTemp;
      if ((pTemp=(struct FileListItem **)xf_realloc(ListData,(FileCount+1)*4))!=NULL)
        ListData=pTemp;
    }
    if (CurPtr!=NULL)
    {
      AddParentPoint(ListData[FileCount],FileCount);
      FileCount++;
    }
  }

  if (AnotherPanel->GetMode()==PLUGIN_PANEL)
  {
    HANDLE hAnotherPlugin=AnotherPanel->GetPluginHandle();
    PluginPanelItemW *PanelData=NULL, *PtrPanelData;
    string strPath;
    int PanelCount=0;

    strPath = strCurDir;
    AddEndSlashW(strPath);
    if (CtrlObject->Plugins.GetVirtualFindData(hAnotherPlugin,&PanelData,&PanelCount,strPath))
    {
        FileListItem **pTemp;
      if ((pTemp=(struct FileListItem **)xf_realloc(ListData,(FileCount+PanelCount)*4))!=NULL)
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
          if ((CurPtr->FileAttr & FA_DIREC)==0)
            CurPtr->SortGroup=CtrlObject->GrpSort->GetGroup(CurPtr->strName);
          else
            CurPtr->SortGroup=DEFAULT_SORT_GROUP;
          if (!TestParentFolderNameW(fdata.lpwszFileName) && (CurPtr->FileAttr & FA_DIREC)==0)
            TotalFileCount++;
        }
        // �������� ������ ��������� � ����� �����, �� ���� ���
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
    ReadSortGroups();

  if (!KeepSelection && PrevSelFileCount>0)
  {
    SaveSelection();
    ClearSelection();
  }

  SortFileList(FALSE);

  if (CurFile>=FileCount || LocalStricmpW(ListData[CurFile]->strName,strCurName)!=0)
    if (!GoToFileW(strCurName) && !strNextCurName.IsEmpty())
      GoToFileW(strNextCurName);

  /* $ 13.02.2002 DJ
     SetTitle() - ������ ���� �� ������� �����!
  */
  if (CtrlObject->Cp() == FrameManager->GetCurrentFrame())
    SetTitle();
  /* DJ $ */

  FarChDirW (strSaveDir); //???
}

/*$ 22.06.2001 SKV
  �������� �������� ��� ������ ����� ���������� �������.
*/
int FileList::UpdateIfChanged(int UpdateMode)
{
  //_SVS(SysLog("CurDir='%s' Opt.AutoUpdateLimit=%d <= FileCount=%d",CurDir,Opt.AutoUpdateLimit,FileCount));
  if(!Opt.AutoUpdateLimit || static_cast<DWORD>(FileCount) <= Opt.AutoUpdateLimit)
  {
    /* $ 19.12.2001 VVM
      ! ������ ����������. ��� Force ���������� ������! */
    if ((IsVisible() && (clock()-LastUpdateTime>2000)) || (UpdateMode != UIC_UPDATE_NORMAL))
    {
      if(UpdateMode == UIC_UPDATE_NORMAL)
        ProcessPluginEvent(FE_IDLE,NULL);
      /* $ 24.12.2002 VVM
        ! �������� ������ ���������� �������. */
      if(// ���������� ������, �� ��� ����������� ����������� � ���� ������
         (PanelMode==NORMAL_PANEL && hListChange!=INVALID_HANDLE_VALUE && WaitForSingleObject(hListChange,0)==WAIT_OBJECT_0) ||
         // ��� ���������� ������, �� ��� ����������� � �� ��������� �������� ����� UPDATE_FORCE
         (PanelMode==NORMAL_PANEL && hListChange==INVALID_HANDLE_VALUE && UpdateMode==UIC_UPDATE_FORCE) ||
         // ��� ��������� ������ � ��������� ����� UPDATE_FORCE
         (PanelMode!=NORMAL_PANEL && UpdateMode==UIC_UPDATE_FORCE)
        )
      /* VVM $ */
        {
          Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
          // � ���� ������ - ������ ����������
//          UpdateColorItems();
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
/* SKV$*/

void FileList::UpdateColorItems(void)
{
  if (Opt.Highlight && PanelMode != PLUGIN_PANEL)
    CtrlObject->HiFiles->GetHiColor(ListData,FileCount);
}


void FileList::CreateChangeNotification(int CheckTree)
{
  wchar_t RootDir[4]=L" :\\";
  DWORD DriveType=DRIVE_REMOTE;

  CloseChangeNotification();

  if(IsLocalPathW(strCurDir))
  {
    RootDir[0]=strCurDir.At(0);
    DriveType=FAR_GetDriveTypeW(RootDir);
  }

  if(Opt.AutoUpdateRemoteDrive || (!Opt.AutoUpdateRemoteDrive && DriveType != DRIVE_REMOTE))
  {
    hListChange=FindFirstChangeNotificationW(strCurDir,CheckTree,
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


void FileList::MoveSelection(struct FileListItem **ListData,long FileCount,
                             struct FileListItem **OldData,long OldFileCount)
{
  struct FileListItem **OldPtr;
  SelFileCount=0;
  SelFileSize=0;
  far_qsort((void *)OldData,OldFileCount,4,SortSearchList);
  while (FileCount--)
  {
    OldPtr=(struct FileListItem **)bsearch((void *)ListData,(void *)OldData,
                                  OldFileCount,4,SortSearchList);
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

/* $ 19.03.2002 DJ
   IgnoreVisible
*/

void FileList::UpdatePlugin(int KeepSelection, int IgnoreVisible)
{
  _ALGO(CleverSysLog clv("FileList::UpdatePlugin"));
  _ALGO(SysLog("(KeepSelection=%d, IgnoreVisible=%d)",KeepSelection,IgnoreVisible));
  if (!IsVisible() && !IgnoreVisible)    /* DJ $ */
  {
    UpdateRequired=TRUE;
    UpdateRequiredMode=KeepSelection;
    return;
  }
  DizRead=FALSE;

  int I;
  struct FileListItem *CurPtr, **OldData=0;
  string strCurName, strNextCurName;
  long OldFileCount=0;

  CloseChangeNotification();

  LastCurFile=-1;

  struct OpenPluginInfoW Info;
  CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

  if (Opt.ShowPanelFree && (Info.Flags & OPIF_REALNAMES))
  {
    unsigned __int64 TotalSize,TotalFree;
    string strDriveRoot;
    GetPathRootW(strCurDir,strDriveRoot);
    if (!GetDiskSizeW(strDriveRoot,&TotalSize,&TotalFree,&FreeDiskSize))
      FreeDiskSize=0;
  }

  PluginPanelItemW *PanelData=NULL;
  int PluginFileCount;

  if (!CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&PluginFileCount,0))
  {
    DeleteListData(ListData,FileCount);
    PopPlugin(TRUE);
    Update(KeepSelection);

    // WARP> ����� ���, �� ����� ������������ - ��������������� ������� �� ������ ��� ������ ������ ������.
    if ( PrevDataStackSize )
      GoToFileW (PrevDataStack[PrevDataStackSize-1]->strPrevName);

    return;
  }

  int PrevSelFileCount=SelFileCount;
  SelFileCount=0;
  SelFileSize=0;
  TotalFileCount=0;
  TotalFileSize=0;

  strPluginDizName = L"";

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
  else
    if (Info.Flags & OPIF_ADDDOTS)
      strCurName = L"..";
  if (KeepSelection || PrevSelFileCount>0)
  {
    OldData=ListData;
    OldFileCount=FileCount;
  }
  else
    DeleteListData(ListData,FileCount);

  FileCount=PluginFileCount;
  ListData=(struct FileListItem**)xf_malloc(4*(FileCount+1));

  if (ListData==NULL)
  {
    FileCount=0;
    return;
  }

  if (Filter==NULL)
    Filter=new PanelFilter(this);

  int DotsPresent=FALSE;

  int FileListCount=0;

  struct PluginPanelItemW *CurPanelData=PanelData;
  for (I=0; I < FileCount; I++, CurPanelData++)
  {
    struct FileListItem *CurListData=ListData[FileListCount];

    if (Info.Flags & OPIF_USEFILTER)
      if ((CurPanelData->FindData.dwFileAttributes & FA_DIREC)==0)
        if (!Filter->CheckNameW(CurPanelData->FindData.lpwszFileName))
          continue;
    if (!Opt.ShowHidden && (CurPanelData->FindData.dwFileAttributes & (FA_HIDDEN|FA_SYSTEM)))
      continue;
    memset(CurListData,0,sizeof(*CurListData));
    PluginToFileListItem(CurPanelData,CurListData);
    if(Info.Flags & OPIF_REALNAMES)
    {
        ConvertNameToShortW (CurListData->strName, CurListData->strShortName);
    }
    CurListData->Position=I;
    if ((Info.Flags & OPIF_USEHIGHLIGHTING) || (Info.Flags & OPIF_USEATTRHIGHLIGHTING))
      CtrlObject->HiFiles->GetHiColor(
          (Info.Flags & OPIF_USEATTRHIGHLIGHTING) ? NULL:(const wchar_t*)CurListData->strName,
          CurListData->FileAttr,&CurListData->Colors);
    if ((Info.Flags & OPIF_USESORTGROUPS) && (CurListData->FileAttr & FA_DIREC)==0)
      CurListData->SortGroup=CtrlObject->GrpSort->GetGroup(CurListData->strName);
    else
      CurListData->SortGroup=DEFAULT_SORT_GROUP;
    if (CurListData->DizText==NULL)
    {
      CurListData->DeleteDiz=FALSE;
      //CurListData->DizText=NULL;
    }
    if (TestParentFolderNameW(CurListData->strName))
    {
      DotsPresent=TRUE;
      CurListData->FileAttr|=FA_DIREC;
    }
    else
      if ((CurListData->FileAttr & FA_DIREC)==0)
        TotalFileCount++;
    TotalFileSize += CurListData->UnpSize;
    FileListCount++;
  }

  FileCount=FileListCount;

  if ((Info.Flags & OPIF_ADDDOTS) && !DotsPresent)
  {
    struct FileListItem *CurPtr;
    AddParentPoint((CurPtr=ListData[FileCount]),FileCount);
    /* $ 22.11.2001 VVM
      + �� ������ ���������� :) */
    if ((Info.Flags & OPIF_USEHIGHLIGHTING) || (Info.Flags & OPIF_USEATTRHIGHLIGHTING))
      CtrlObject->HiFiles->GetHiColor(
          (Info.Flags & OPIF_USEATTRHIGHLIGHTING) ? NULL:(const wchar_t*)CurPtr->strName,
          CurPtr->FileAttr,&CurPtr->Colors);
    /* VVM $ */
    if (Info.HostFile && *Info.HostFile)
    {
      FAR_FIND_DATA_EX FindData;

      if ( apiGetFindDataEx (Info.HostFile,&FindData) )
      {
        CurPtr->WriteTime=FindData.ftLastWriteTime;
        CurPtr->CreationTime=FindData.ftCreationTime;
        CurPtr->AccessTime=FindData.ftLastAccessTime;
      }
    }
    FileCount++;
  }

  /* $ 25.02.2001 VVM
      ! �� ��������� �������� ������ ������ � ������ ������� */
  if (IsColumnDisplayed(DIZ_COLUMN))
    ReadDiz(PanelData,PluginFileCount,RDF_NO_UPDATE);
  /* VVM $ */

  CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,PluginFileCount);

  CorrectPosition();

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

  if (CurFile>=FileCount || LocalStricmpW(ListData[CurFile]->strName,strCurName)!=0)
      if (!GoToFileW(strCurName) && !strNextCurName.IsEmpty() )
        GoToFileW(strNextCurName);
  SetTitle();
}


void FileList::ReadDiz(struct PluginPanelItemW *ItemList,int ItemLength,DWORD dwFlags)
{
  if (DizRead)
    return;
  DizRead=TRUE;
  Diz.Reset();

  if (PanelMode==NORMAL_PANEL)
    Diz.Read(strCurDir);
  else
  {
    PluginPanelItemW *PanelData=NULL;
    int PluginFileCount=0;

    struct OpenPluginInfoW Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

    if (Info.DescrFilesNumber==0)
      return;

    int GetCode=TRUE;
    /* $ 25.02.2001 VVM
        + ��������� ����� RDF_NO_UPDATE */
    if ((ItemList==NULL) && ((dwFlags & RDF_NO_UPDATE) == 0))
      GetCode=CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&PluginFileCount,0);
    /* VVM $ */
    else
    {
      PanelData=ItemList;
      PluginFileCount=ItemLength;
    }
    if (GetCode)
    {
      for (int I=0;I<Info.DescrFilesNumber;I++)
      {
        PluginPanelItemW *CurPanelData=PanelData;
        for (int J=0; J < PluginFileCount; J++, CurPanelData++)
        {
            string strFileName = CurPanelData->FindData.lpwszFileName;

          if (LocalStricmpW(strFileName,Info.DescrFiles[I])==0)
          {
            string strTempDir, strDizName;
            if (FarMkTempExW(strTempDir) && CreateDirectoryW(strTempDir,NULL))
            {
              if (CtrlObject->Plugins.GetFile(hPlugin,CurPanelData,strTempDir,strDizName,OPM_SILENT|OPM_VIEW|OPM_QUICKVIEW|OPM_DESCR))
              {
                strPluginDizName = Info.DescrFiles[I];

                Diz.Read(L"",strDizName);

                DeleteFileWithFolderW(strDizName);
                I=Info.DescrFilesNumber;
                break;
              }
              FAR_RemoveDirectoryW(strTempDir);
              //ViewPanel->ShowFile(NULL,FALSE,NULL);
            }
          }
        }
      }
      /* $ 25.02.2001 VVM
          + ��������� ����� RDF_NO_UPDATE */
      if ((ItemList==NULL) && ((dwFlags & RDF_NO_UPDATE) == 0))
        CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,PluginFileCount);
      /* VVM $ */
    }
  }
  struct FileListItem *CurPtr;
  for (int I=0;I<FileCount;I++)
  {
    CurPtr = ListData[I];
    if (CurPtr->DizText==NULL)
    {
      CurPtr->DeleteDiz=FALSE;
      CurPtr->DizText=(wchar_t*)Diz.GetDizTextAddr(CurPtr->strName,CurPtr->strShortName,CurPtr->UnpSize);
    }
  }
}


void FileList::ReadSortGroups()
{
  if (SortGroupsRead)
    return;
  SortGroupsRead=TRUE;
  struct FileListItem *CurPtr;
  for (int I=0;I<FileCount;I++)
  {
      CurPtr = ListData[I];
    if ((CurPtr->FileAttr & FA_DIREC)==0)
      CurPtr->SortGroup=CtrlObject->GrpSort->GetGroup(CurPtr->strName);
    else
      CurPtr->SortGroup=DEFAULT_SORT_GROUP;
  }
}

// �������� ������� CurPtr � ������� ���������������� ������ ��� �������� ".."
void FileList::AddParentPoint(struct FileListItem *CurPtr,long CurFilePos)
{
#if defined(__BORLANDC__)
  static struct FileListItem ParentItem;
#else
  static struct FileListItem ParentItem={0};
#endif
  if(ParentItem.FileAttr == 0)
  {
    ParentItem.strName = L"..";
    ParentItem.strShortName = L"..";

    ParentItem.FileAttr=FA_DIREC;
  }
  ParentItem.Position=CurFilePos;
  memcpy(CurPtr,&ParentItem,sizeof(struct FileListItem));
}
