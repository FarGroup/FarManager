/*
flupdate.cpp

Файловая панель - чтение имен файлов

*/

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
#include "filefilter.hpp"
#include "hilight.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "TPreRedrawFunc.hpp"
#include "TaskBar.hpp"

int _cdecl SortSearchList(const void *el1,const void *el2);

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
          struct OpenPluginInfo Info;
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

void FileList::UpdateIfRequired()
{
  if (UpdateRequired)
  {
    UpdateRequired = FALSE;
    Update (UpdateRequiredMode | UPDATE_IGNORE_VISIBLE);
  }
}

void ReadFileNamesMsg(char *Msg)
{
  Message(0,0,MSG(MReadingTitleFiles),Msg);
  PreRedrawItem preRedrawItem=PreRedraw.Peek();
  preRedrawItem.Param.Param1=Msg;
  PreRedraw.SetParam(preRedrawItem.Param);
}

static void PR_ReadFileNamesMsg(void)
{
  PreRedrawItem preRedrawItem=PreRedraw.Peek();
  ReadFileNamesMsg((char *)preRedrawItem.Param.Param1);
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

  Is_FS_NTFS=FALSE;
  UpdateRequired=FALSE;
  AccessTimeUpdateRequired=FALSE;
  DizRead=FALSE;

  HANDLE FindHandle;
  WIN32_FIND_DATA fdata;
  struct FileListItem *CurPtr=0,*OldData=0;
  char CurName[NM],NextCurName[NM];
  long OldFileCount=0;
  int Done;
  int I;

  clock_t StartTime=clock();

  CloseChangeNotification();

  if (this!=CtrlObject->Cp()->LeftPanel && this!=CtrlObject->Cp()->RightPanel )
    return;

  char SaveDir[NM];
  *(int*)SaveDir=0;
  FarGetCurDir(NM, SaveDir);
  {
    char OldCurDir[NM*2];
    xstrncpy(OldCurDir, CurDir, NM-1);
    if (!SetCurPath())
    {
      FlushInputBuffer(); // Очистим буффер ввода, т.к. мы уже можем быть в другом месте...
      if (strcmp(CurDir, OldCurDir) == 0)
      {
        GetPathRootOne(OldCurDir,OldCurDir);
        if(!IsDiskInDrive(OldCurDir))
          IfGoHome(*OldCurDir);
        /* При смене каталога путь не изменился */
      }
      return;
    }
  }

  SortGroupsRead=FALSE;

  if (GetFocus())
    CtrlObject->CmdLine->SetCurDir(CurDir);

  Is_FS_NTFS=CheckFileSystem(CurDir);

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
    char DriveRoot[NM];
    GetPathRoot(CurDir,DriveRoot);
    if (!GetDiskSize(DriveRoot,&TotalSize,&TotalFree,(unsigned __int64*)&FreeDiskSize))
      FreeDiskSize=0;
  }

  *CurName=*NextCurName=0;
  if (FileCount>0)
  {
    strcpy(CurName,ListData[CurFile].Name);
    if (ListData[CurFile].Selected)
    {
      for (I=CurFile+1, CurPtr=ListData+I; I < FileCount; I++, CurPtr++)
        if (!CurPtr->Selected)
        {
          strcpy(NextCurName,CurPtr->Name);
          break;
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

  char ComputerName[NM];
  *ComputerName=0;
  if (ReadOwners)
  {
    CurPath2ComputerName(CurDir,ComputerName,sizeof(ComputerName)-1);
    // сбросим кэш SID`ов
    SIDCacheFlush();
  }

  SetLastError(0);

  Done=((FindHandle=FAR_FindFirstFile("*.*",&fdata))==INVALID_HANDLE_VALUE);

  int AllocatedCount=0;
  struct FileListItem *NewPtr;

  // сформируем заголовок вне цикла
  char Title[2048];
  int TitleLength=Min((int)X2-X1-1,(int)sizeof(Title)-1);
  memset(Title,0x0CD,TitleLength);
  Title[TitleLength]=0;
  BOOL IsShowTitle=FALSE;
  BOOL NeedHighlight=Opt.Highlight && PanelMode != PLUGIN_PANEL;

  if (Filter==NULL)
    Filter=new FileFilter(this,FFT_PANEL);

  //Рефреш текущему времени для фильтра перед началом операции
  Filter->UpdateCurrentTime();
  CtrlObject->HiFiles->UpdateCurrentTime();

  for (FileCount=0; !Done; )
  {
    if ((fdata.cFileName[0]!='.' || fdata.cFileName[1]!=0) &&
        (Opt.ShowHidden || (fdata.dwFileAttributes & (FA_HIDDEN|FA_SYSTEM))==0) &&
        (/*(fdata.dwFileAttributes & FA_DIREC) ||*/
        Filter->FileInFilter(&fdata)))
    {
      int UpperDir=FALSE;
      if (fdata.cFileName[0]=='.' && fdata.cFileName[1]=='.' && fdata.cFileName[2]==0)
      {
        UpperDir=TRUE;
        DotsPresent=TRUE;
        if (IsLocalRootPath(CurDir))
        {
          Done=!FAR_FindNextFile(FindHandle,&fdata);
          continue;
        }
      }
      if (FileCount>=AllocatedCount)
      {
        AllocatedCount=AllocatedCount+256+AllocatedCount/4;
        if ((CurPtr=(struct FileListItem *)xf_realloc(ListData,AllocatedCount*sizeof(*ListData)))==NULL)
          break;
        ListData=CurPtr;
      }

      NewPtr=ListData+FileCount;
      memset(NewPtr,0,sizeof(struct FileListItem));
      memcpy(&NewPtr->FileAttr,&fdata,sizeof(fdata));
      NewPtr->Position=FileCount++;
      NewPtr->NumberOfLinks=1;

      if(fdata.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
      {
        NewPtr->ReparseTag=fdata.dwReserved0; //MSDN
      }

      if ((fdata.dwFileAttributes & FA_DIREC) == 0)
      {
        TotalFileSize+=MKUINT64(fdata.nFileSizeHigh,fdata.nFileSizeLow);
        int Compressed=FALSE;
        if (ReadPacked && ((fdata.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) || (fdata.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)))
        {
          NewPtr->PackSize=GetCompressedFileSize(fdata.cFileName,&NewPtr->PackSizeHigh);
          if (CurPtr->PackSize!=0xFFFFFFFF || GetLastError()==NO_ERROR)
            Compressed=TRUE;
        }
        if (!Compressed)
        {
          NewPtr->PackSizeHigh=fdata.nFileSizeHigh;
          NewPtr->PackSize=fdata.nFileSizeLow;
        }
        if (ReadNumLinks)
          NewPtr->NumberOfLinks=GetNumberOfLinks(fdata.cFileName);
      }
      else
      {
        NewPtr->PackSizeHigh=NewPtr->PackSize=0;
      }

      NewPtr->SortGroup=DEFAULT_SORT_GROUP;
      if (ReadOwners)
      {
        char Owner[NM];
        GetFileOwner(*ComputerName ? ComputerName:NULL,NewPtr->Name,Owner);
        xstrncpy(NewPtr->Owner,Owner,sizeof(NewPtr->Owner)-1);
      }

      if (NeedHighlight)
        CtrlObject->HiFiles->GetHiColor(NewPtr,1);

      if (!UpperDir && (fdata.dwFileAttributes & FA_DIREC)==0)
        TotalFileCount++;

      //memcpy(ListData+FileCount,&NewPtr,sizeof(NewPtr));
//      FileCount++;

      if ((FileCount & 0x3f)==0 && clock()-StartTime>1000)
      {

        if (IsVisible())
        {
          char ReadMsg[100];
          if(!IsShowTitle)
          {
            if(!DrawMessage)
            {
              Text(X1+1,Y1,COL_PANELBOX,Title);
              IsShowTitle=TRUE;
              SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
            }
          }

          sprintf(ReadMsg,MSG(MReadingFiles),FileCount);
          if(DrawMessage)
            ReadFileNamesMsg(ReadMsg);
          else
          {
            TruncStr(ReadMsg,TitleLength-2);
            int MsgLength=(int)strlen(ReadMsg);
            GotoXY(X1+1+(TitleLength-MsgLength-1)/2,Y1);
            mprintf(" %s ",ReadMsg);
          }
        }
        if (CheckForEsc())
        {
          Message(MSG_WARNING,1,MSG(MUserBreakTitle),MSG(MOperationNotCompleted),MSG(MOk));
          break;
        }
      }
    }
    Done=!FAR_FindNextFile(FindHandle,&fdata);
  }

  int ErrCode=GetLastError();
  if (!(ErrCode==ERROR_SUCCESS || ErrCode==ERROR_NO_MORE_FILES || ErrCode==ERROR_FILE_NOT_FOUND ||
        (ErrCode==ERROR_BAD_PATHNAME && WinVer.dwPlatformId != VER_PLATFORM_WIN32_NT && Opt.IgnoreErrorBadPathName)))
    Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MReadFolderError),MSG(MOk));

  FAR_FindClose(FindHandle);

  if (IsColumnDisplayed(DIZ_COLUMN))
    ReadDiz();

  /*
  int NetRoot=FALSE;
  if (CurDir[0]=='\\' && CurDir[1]=='\\')
  {
    char *ChPtr=strchr(CurDir+2,'\\');
    if (ChPtr==NULL || strchr(ChPtr+1,'\\')==NULL)
      NetRoot=TRUE;
  }
  */
  // пока кусок закомментим, возможно он даже и не пригодится.
  if (!DotsPresent && !IsLocalRootPath(CurDir))// && !NetRoot)
  {
    if (FileCount>=AllocatedCount)
    {
      if ((CurPtr=(struct FileListItem *)xf_realloc(ListData,(FileCount+1)*sizeof(*ListData)))!=NULL)
        ListData=CurPtr;
    }
    if (CurPtr!=NULL)
    {
      AddParentPoint(ListData+FileCount,FileCount);
      if (NeedHighlight)
        CtrlObject->HiFiles->GetHiColor(ListData+FileCount,1);
      FileCount++;
    }
  }

  if (AnotherPanel->GetMode()==PLUGIN_PANEL)
  {
    HANDLE hAnotherPlugin=AnotherPanel->GetPluginHandle();
    PluginPanelItem *PanelData=NULL, *PtrPanelData;
    char Path[NM];
    int PanelCount=0;
    strcpy(Path,CurDir);
    AddEndSlash(Path);
    if (CtrlObject->Plugins.GetVirtualFindData(hAnotherPlugin,&PanelData,&PanelCount,Path))
    {
      if ((CurPtr=(struct FileListItem *)xf_realloc(ListData,(FileCount+PanelCount)*sizeof(*ListData)))!=NULL)
      {
        ListData=CurPtr;
        for (CurPtr=ListData+FileCount, PtrPanelData=PanelData, I=0; I < PanelCount; I++, CurPtr++, PtrPanelData++)
        {
          WIN32_FIND_DATA &fdata=PtrPanelData->FindData;
          PluginToFileListItem(PtrPanelData,CurPtr);
          CurPtr->Position=FileCount;
          TotalFileSize+=MKUINT64(fdata.nFileSizeHigh,fdata.nFileSizeLow);
          CurPtr->PrevSelected=CurPtr->Selected=0;
          CurPtr->ShowFolderSize=0;
          //if ((CurPtr->FileAttr & FA_DIREC)==0)
            CurPtr->SortGroup=CtrlObject->HiFiles->GetGroup(&fdata);
          //else
            //CurPtr->SortGroup=DEFAULT_SORT_GROUP;
          if (!TestParentFolderName(fdata.cFileName) && (CurPtr->FileAttr & FA_DIREC)==0)
            TotalFileCount++;
        }
        // цветовую боевую раскраску в самом конце, за один раз
        CtrlObject->HiFiles->GetHiColor(ListData+FileCount,PanelCount);
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

  if (CurFile>=FileCount || LocalStricmp((ListData+CurFile)->Name,CurName)!=0)
    if (!GoToFile(CurName) && *NextCurName)
      GoToFile(NextCurName);

  /* $ 13.02.2002 DJ
     SetTitle() - только если мы текущий фрейм!
  */
  if (CtrlObject->Cp() == FrameManager->GetCurrentFrame())
    SetTitle();
  /* DJ $ */

  if (*SaveDir) {
    SetCurrentDirectory(SaveDir);
  }
}

/*$ 22.06.2001 SKV
  Добавлен параметр для вызова после исполнения команды.
*/
int FileList::UpdateIfChanged(int UpdateMode)
{
  //_SVS(SysLog("CurDir='%s' Opt.AutoUpdateLimit=%d <= FileCount=%d",CurDir,Opt.AutoUpdateLimit,FileCount));
  if(!Opt.AutoUpdateLimit || static_cast<DWORD>(FileCount) <= Opt.AutoUpdateLimit)
  {
    /* $ 19.12.2001 VVM
      ! Сменим приоритеты. При Force обновление всегда! */
    if ((IsVisible() && (clock()-LastUpdateTime>2000)) || (UpdateMode != UIC_UPDATE_NORMAL))
    {
      if(UpdateMode == UIC_UPDATE_NORMAL)
        ProcessPluginEvent(FE_IDLE,NULL);
      /* $ 24.12.2002 VVM
        ! Поменяем логику обновления панелей. */
      if(// Нормальная панель, на ней установлено уведомление и есть сигнал
         (PanelMode==NORMAL_PANEL && hListChange!=INVALID_HANDLE_VALUE && WaitForSingleObject(hListChange,0)==WAIT_OBJECT_0) ||
         // Или Нормальная панель, но нет уведомления и мы попросили обновить через UPDATE_FORCE
         (PanelMode==NORMAL_PANEL && hListChange==INVALID_HANDLE_VALUE && UpdateMode==UIC_UPDATE_FORCE) ||
         // Или плагинная панель и обновляем через UPDATE_FORCE
         (PanelMode!=NORMAL_PANEL && UpdateMode==UIC_UPDATE_FORCE)
        )
      /* VVM $ */
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
/* SKV$*/

void FileList::CreateChangeNotification(int CheckTree)
{
  char RootDir[4]=" :\\";
  DWORD DriveType=DRIVE_REMOTE;

  CloseChangeNotification();

  if(IsLocalPath(CurDir))
  {
    RootDir[0]=*CurDir;
    DriveType=FAR_GetDriveType(RootDir);
  }

  if(Opt.AutoUpdateRemoteDrive || (!Opt.AutoUpdateRemoteDrive && DriveType != DRIVE_REMOTE))
  {
    char AnsiName[NM];
    FAR_OemToChar(CurDir,AnsiName);
    SetFileApisTo(APIS2ANSI);
    hListChange=FindFirstChangeNotification(AnsiName,CheckTree,
                        FILE_NOTIFY_CHANGE_FILE_NAME|
                        FILE_NOTIFY_CHANGE_DIR_NAME|
                        FILE_NOTIFY_CHANGE_ATTRIBUTES|
                        FILE_NOTIFY_CHANGE_SIZE|
                        FILE_NOTIFY_CHANGE_LAST_WRITE);
    SetFileApisTo(APIS2OEM);
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


void FileList::MoveSelection(struct FileListItem *ListData,long FileCount,
                             struct FileListItem *OldData,long OldFileCount)
{
  struct FileListItem *OldPtr;
  SelFileCount=0;
  SelFileSize=0;
  far_qsort((void *)OldData,OldFileCount,sizeof(*OldData),SortSearchList);
  while (FileCount--)
  {
    OldPtr=(struct FileListItem *)bsearch((void *)ListData,(void *)OldData,
                                  OldFileCount,sizeof(*OldData),SortSearchList);
    if (OldPtr!=NULL)
    {
      if (OldPtr->ShowFolderSize)
      {
        ListData->ShowFolderSize=2;
        ListData->UnpSize=OldPtr->UnpSize;
        ListData->UnpSizeHigh=OldPtr->UnpSizeHigh;
        ListData->PackSize=OldPtr->PackSize;
        ListData->PackSizeHigh=OldPtr->PackSizeHigh;
      }
      Select(ListData,OldPtr->Selected);
      ListData->PrevSelected=OldPtr->PrevSelected;
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
  struct FileListItem *CurPtr, *OldData=0;
  char CurName[NM],NextCurName[NM];
  long OldFileCount=0;

  CloseChangeNotification();

  LastCurFile=-1;

  struct OpenPluginInfo Info;
  CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

  if (Opt.ShowPanelFree && (Info.Flags & OPIF_REALNAMES))
  {
    unsigned __int64 TotalSize,TotalFree;
    char DriveRoot[NM];
    GetPathRoot(CurDir,DriveRoot);
    if (!GetDiskSize(DriveRoot,&TotalSize,&TotalFree,(unsigned __int64*)&FreeDiskSize))
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
    if ( PrevDataStackSize )
      GoToFile (PrevDataStack[PrevDataStackSize-1].PrevName);

    return;
  }

  int PrevSelFileCount=SelFileCount;
  SelFileCount=0;
  SelFileSize=0;
  TotalFileCount=0;
  TotalFileSize=0;
  *PluginDizName=0;

  *CurName=*NextCurName=0;
  if (FileCount>0)
  {
    CurPtr=ListData+CurFile;
    strcpy(CurName,CurPtr->Name);
    if (CurPtr->Selected)
    {
      for (++CurPtr, I=CurFile+1; I < FileCount; I++, CurPtr++)
        if (!CurPtr->Selected)
        {
          strcpy(NextCurName,CurPtr->Name);
          break;
        }
    }
  }
  else
    if (Info.Flags & OPIF_ADDDOTS)
      strcpy(CurName,"..");
  if (KeepSelection || PrevSelFileCount>0)
  {
    OldData=ListData;
    OldFileCount=FileCount;
  }
  else
    DeleteListData(ListData,FileCount);

  FileCount=PluginFileCount;
  ListData=(struct FileListItem*)xf_malloc(sizeof(struct FileListItem)*(FileCount+1));

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

  struct PluginPanelItem *CurPanelData=PanelData;
  for (I=0; I < FileCount; I++, CurPanelData++)
  {
    struct FileListItem *CurListData=ListData+FileListCount;

    if (Info.Flags & OPIF_USEFILTER)
      //if ((CurPanelData->FindData.dwFileAttributes & FA_DIREC)==0)
        if (!Filter->FileInFilter(&(CurPanelData->FindData)))
          continue;
    if (!Opt.ShowHidden && (CurPanelData->FindData.dwFileAttributes & (FA_HIDDEN|FA_SYSTEM)))
      continue;
    memset(CurListData,0,sizeof(*CurListData));
    PluginToFileListItem(CurPanelData,CurListData);
    if(Info.Flags & OPIF_REALNAMES)
    {
      ConvertNameToShort(CurListData->Name,CurListData->ShortName,sizeof(CurListData->ShortName)-1);
    }
    CurListData->Position=I;
    if ((Info.Flags & OPIF_USEHIGHLIGHTING) || (Info.Flags & OPIF_USEATTRHIGHLIGHTING))
      CtrlObject->HiFiles->GetHiColor(&CurPanelData->FindData,&CurListData->Colors,(Info.Flags&OPIF_USEATTRHIGHLIGHTING)!=0);
    if ((Info.Flags & OPIF_USESORTGROUPS) /*&& (CurListData->FileAttr & FA_DIREC)==0*/)
      CurListData->SortGroup=CtrlObject->HiFiles->GetGroup(&(CurPanelData->FindData));
    else
      CurListData->SortGroup=DEFAULT_SORT_GROUP;
    if (CurListData->DizText==NULL)
    {
      CurListData->DeleteDiz=FALSE;
      //CurListData->DizText=NULL;
    }
    if (TestParentFolderName(CurListData->Name))
    {
      DotsPresent=TRUE;
      CurListData->FileAttr|=FA_DIREC;
    }
    else
      if ((CurListData->FileAttr & FA_DIREC)==0)
        TotalFileCount++;
    TotalFileSize+=MKUINT64(CurListData->UnpSizeHigh,CurListData->UnpSize);
    FileListCount++;
  }

  FileCount=FileListCount;

  if ((Info.Flags & OPIF_ADDDOTS) && !DotsPresent)
  {
    struct FileListItem *CurPtr;
    AddParentPoint((CurPtr=ListData+FileCount),FileCount);
    if ((Info.Flags & OPIF_USEHIGHLIGHTING) || (Info.Flags & OPIF_USEATTRHIGHLIGHTING))
      CtrlObject->HiFiles->GetHiColor(CurPtr,1,(Info.Flags&OPIF_USEATTRHIGHLIGHTING)!=0);
    if (Info.HostFile && *Info.HostFile)
    {
      WIN32_FIND_DATA FindData;
      if(GetFileWin32FindData(Info.HostFile,&FindData))
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

  if (CurFile>=FileCount || LocalStricmp((ListData+CurFile)->Name,CurName)!=0)
    if (!GoToFile(CurName) && *NextCurName)
      GoToFile(NextCurName);
  SetTitle();
}


void FileList::ReadDiz(struct PluginPanelItem *ItemList,int ItemLength,DWORD dwFlags)
{
  if (DizRead)
    return;
  DizRead=TRUE;
  Diz.Reset();

  if (PanelMode==NORMAL_PANEL)
    Diz.Read(CurDir);
  else
  {
    PluginPanelItem *PanelData=NULL;
    int PluginFileCount=0;

    struct OpenPluginInfo Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

    if (Info.DescrFilesNumber==0)
      return;

    int GetCode=TRUE;
    /* $ 25.02.2001 VVM
        + Обработка флага RDF_NO_UPDATE */
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
        PluginPanelItem *CurPanelData=PanelData;
        for (int J=0; J < PluginFileCount; J++, CurPanelData++)
        {
          if (LocalStricmp(CurPanelData->FindData.cFileName,Info.DescrFiles[I])==0)
          {
            char TempDir[NM],DizName[NM];
            if (FarMkTempEx(TempDir) && CreateDirectory(TempDir,NULL))
            {
              if (CtrlObject->Plugins.GetFile(hPlugin,CurPanelData,TempDir,DizName,OPM_SILENT|OPM_VIEW|OPM_QUICKVIEW|OPM_DESCR))
              {
                strcpy(PluginDizName,Info.DescrFiles[I]);
                Diz.Read("",DizName);
                DeleteFileWithFolder(DizName);
                I=Info.DescrFilesNumber;
                break;
              }
              FAR_RemoveDirectory(TempDir);
              //ViewPanel->ShowFile(NULL,FALSE,NULL);
            }
          }
        }
      }
      /* $ 25.02.2001 VVM
          + Обработка флага RDF_NO_UPDATE */
      if ((ItemList==NULL) && ((dwFlags & RDF_NO_UPDATE) == 0))
        CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,PluginFileCount);
      /* VVM $ */
    }
  }
  struct FileListItem *CurPtr=ListData;
  for (int I=0;I<FileCount;I++,CurPtr++)
  {
    if (CurPtr->DizText==NULL)
    {
      CurPtr->DeleteDiz=FALSE;
      CurPtr->DizText=Diz.GetDizTextAddr(CurPtr->Name,CurPtr->ShortName,MKUINT64(CurPtr->UnpSizeHigh,CurPtr->UnpSize));
    }
  }
}


void FileList::ReadSortGroups(bool UpdateFilterCurrentTime)
{
  if (SortGroupsRead)
    return;
  if (UpdateFilterCurrentTime)
    CtrlObject->HiFiles->UpdateCurrentTime();
  SortGroupsRead=TRUE;
  struct FileListItem *CurPtr=ListData;
  for (int I=0;I<FileCount;I++,CurPtr++)
  {
    CurPtr->SortGroup=CtrlObject->HiFiles->GetGroup(CurPtr);
  }
}

// Обнулить текущий CurPtr и занести предопределенные данные для каталога ".."
void FileList::AddParentPoint(struct FileListItem *CurPtr,long CurFilePos)
{
  static struct FileListItem ParentItem={0};
  if(ParentItem.FileAttr == 0)
  {
    strcpy(ParentItem.ShortName,"..");
    strcpy(ParentItem.Name,"..");
    ParentItem.FileAttr=FA_DIREC;
  }
  ParentItem.Position=CurFilePos;
  memcpy(CurPtr,&ParentItem,sizeof(struct FileListItem));
}
