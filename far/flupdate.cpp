/*
flupdate.cpp

Файловая панель - чтение имен файлов

*/

/* Revision: 1.21 14.12.2001 $ */

/*
Modify:
  14.12.2001 VVM
    ! При обновлении списка файлов не прерываемся при обломе в SetCurDir(),
      т.к. там мы все-равно куда-нибудь и встали. Может и не на тот-же диск...
  23.11.2001 SVS
    ! небольшая оптимизация в "запроснике" цветов - цвета запрашиваем
      только после сбора информации о файловых объектах и применяем
      новую функцию HighlightFiles::GetHiColor(), работающая с кипой
      структур FileListItem
    - ФАР жрал кучу процессорного времени - во время простоя все
      время перекраска каталога шла.
  22.11.2001 VVM
    + Раскрасить ".." при OPIF_ADDDOTS
  13.11.2001 OT
    ! Попытка исправить создание каталогов на пассивной панели по F7
  24.10.2001 SVS
    ! сначала проапдейтим пассивную панель, а потом активную.
  02.10.2001 SVS
    - UpdateColorItems() предназначена не для плагиновых панелей!
  01.10.2001 SVS
    ! Немного оптимизации - для ускорения считывания директории
    + AddParentPoint() - общий код по добавлению ".."
    + UpdateColorItems() - колоризация итемов
  27.09.2001 IS
    - Левый размер при использовании strncpy
  26.09.2001 SVS
    + Opt.AutoUpdateLimit -  выше этого количество не обновлять панели.
  05.09.2001 SVS
    ! Вместо полей Color* в структе FileListItem используется
      структура HighlightDataColor
  17.08.2001 VVM
    + FileListItem.CRC32
  22.06.2001 SKV
    ! UpdateIfChanged - добавлен параметр Force.
  06.05.2001 DJ
    ! перетрях #include
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  26.03.2001 SVS
    ! для любых сетевых путей добавляем ".." - иногда операционка этого
      сама не делает :-(
  28.02.2001 IS
    ! "CtrlObject->CmdLine." -> "CtrlObject->CmdLine->"
  27.02.2001 VVM
    ! Символы, зависимые от кодовой страницы
      /[\x01-\x08\x0B-\x0C\x0E-\x1F\xB0-\xDF\xF8-\xFF]/
      переведены в коды.
  25.02.2001 VVM
    + Обработка флага RDF_NO_UPDATE в ReadDiz()
  11.11.2000 SVS
    ! FarMkTemp() - убираем (как всегда - то ставим, то тут же убираем :-(((
  11.11.2000 SVS
    ! Используем конструкцию FarMkTemp()
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
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
#include "filter.hpp"
#include "hilight.hpp"
#include "grpsort.hpp"
#include "ctrlobj.hpp"

int _cdecl SortSearchList(const void *el1,const void *el2);

void FileList::Update(int Mode)
{
  if (EnableUpdate)
    switch(PanelMode)
    {
      case NORMAL_PANEL:
        ReadFileNames(Mode & UPDATE_KEEP_SELECTION);
        break;
      case PLUGIN_PANEL:
        {
          struct OpenPluginInfo Info;
          CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
          ProcessPluginCommand();
          if (PanelMode!=PLUGIN_PANEL)
            ReadFileNames(Mode & UPDATE_KEEP_SELECTION);
          else
            if ((Info.Flags & OPIF_REALNAMES) ||
                CtrlObject->Cp()->GetAnotherPanel(this)->GetMode()==PLUGIN_PANEL ||
                (Mode & UPDATE_SECONDARY)==0)
              UpdatePlugin(Mode & UPDATE_KEEP_SELECTION);
        }
        ProcessPluginCommand();
        break;
    }
  LastUpdateTime=clock();
}


// ЭТО ЕСТЬ УЗКОЕ МЕСТО ДЛЯ СКОРОСТНЫХ ХАРАКТЕРИСТИК Far Manafer
// при считывании дирректории
void FileList::ReadFileNames(int KeepSelection)
{
  if (!IsVisible())
  {
    UpdateRequired=TRUE;
    UpdateRequiredMode=KeepSelection;
    return;
  }
  AccessTimeUpdateRequired=FALSE;
  DizRead=FALSE;
  HANDLE FindHandle;
  WIN32_FIND_DATA fdata;
  struct FileListItem *CurPtr,*OldData;
  char CurName[NM],NextCurName[NM];
  long OldFileCount;
  int Done;

  clock_t StartTime=clock();

  CloseChangeNotification();

  if (this!=CtrlObject->Cp()->LeftPanel && this!=CtrlObject->Cp()->RightPanel )
    return;

  char SaveDir[NM];
  *(int*)SaveDir=0;
  GetCurrentDirectory(NM, SaveDir);
  char OldCurDir[NM];
  strncpy(OldCurDir, CurDir, NM-1);
  if (!SetCurPath()){
    if (strcmp(CurDir, OldCurDir) == 0)
    /* При смене каталога путь не изменился */
      return;
  }

  SortGroupsRead=FALSE;

  if (Filter==NULL)
    Filter=new PanelFilter(this);

  if (GetFocus())
    CtrlObject->CmdLine->SetCurDir(CurDir);

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
    int64 TotalSize,TotalFree;
    char DriveRoot[NM];
    GetPathRoot(CurDir,DriveRoot);
    if (!GetDiskSize(DriveRoot,&TotalSize,&TotalFree,&FreeDiskSize))
      FreeDiskSize=0;
  }

  *CurName=*NextCurName=0;
  if (FileCount>0)
  {
    strcpy(CurName,ListData[CurFile].Name);
    if (ListData[CurFile].Selected)
      for (int I=CurFile+1;I<FileCount;I++)
        if (!ListData[I].Selected)
        {
          strcpy(NextCurName,ListData[I].Name);
          break;
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
    char NetDir[NM];
    *NetDir=0;
    if (CurDir[0]=='\\' && CurDir[1]=='\\')
      strcpy(NetDir,CurDir);
    else
    {
      char LocalName[NM];
      DWORD RemoteNameSize=sizeof(NetDir);
      sprintf(LocalName,"%c:",*CurDir);

      SetFileApisToANSI();
      if (WNetGetConnection(LocalName,NetDir,&RemoteNameSize)==NO_ERROR)
        CharToOem(NetDir,NetDir);
      SetFileApisToOEM();
    }
    if (NetDir[0]=='\\' && NetDir[1]=='\\')
    {
      strcpy(ComputerName,NetDir+2);
      char *EndSlash=strchr(ComputerName,'\\');
      if (EndSlash==NULL)
        *ComputerName=0;
      else
        *EndSlash=0;
    }
  }

  SetLastError(0);

  Done=((FindHandle=FindFirstFile("*.*",&fdata))==INVALID_HANDLE_VALUE);

  int AllocatedCount=0;
  struct FileListItem NewPtr;

  // вне цикла получим указатель.
  char *PointToName_CurDir=PointToName(CurDir);

  // сформируем заголовок вне цикла
  char Title[2048];
  int TitleLength=Min((int)X2-X1-1,(int)sizeof(Title)-1);
  memset(Title,0x0CD,TitleLength);
  Title[TitleLength]=0;
  BOOL IsShowTitle=FALSE;

  for (FileCount=0; !Done; )
  {
    if ((fdata.cFileName[0]!='.' || fdata.cFileName[1]!=0) &&
        (Opt.ShowHidden || (fdata.dwFileAttributes & (FA_HIDDEN|FA_SYSTEM))==0) &&
        ((fdata.dwFileAttributes & FA_DIREC) ||
        Filter->CheckName(fdata.cFileName)))
    {
      int UpperDir=FALSE;
      if (fdata.cFileName[0]=='.' && fdata.cFileName[1]=='.' && fdata.cFileName[2]==0)
      {
        UpperDir=TRUE;
        DotsPresent=TRUE;
        if (*PointToName_CurDir==0)
        {
          Done=!FindNextFile(FindHandle,&fdata);
          continue;
        }
      }
      if (FileCount>=AllocatedCount)
      {
        AllocatedCount=AllocatedCount+256+AllocatedCount/4;
        if ((CurPtr=(struct FileListItem *)realloc(ListData,AllocatedCount*sizeof(*ListData)))==NULL)
          break;
        ListData=CurPtr;
      }

      memset(&NewPtr,0,sizeof(NewPtr));
      memcpy(&NewPtr.FileAttr,&fdata,sizeof(fdata));
      NewPtr.Position=FileCount;
      NewPtr.NumberOfLinks=1;

      if ((fdata.dwFileAttributes & FA_DIREC) == 0)
      {
        TotalFileSize+=int64(fdata.nFileSizeHigh,fdata.nFileSizeLow);
        int Compressed=FALSE;
        if (ReadPacked && (fdata.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED))
        {
          NewPtr.PackSize=GetCompressedFileSize(fdata.cFileName,&NewPtr.PackSizeHigh);
          if (CurPtr->PackSize!=0xFFFFFFFF || GetLastError()==NO_ERROR)
            Compressed=TRUE;
        }
        if (!Compressed)
        {
          NewPtr.PackSizeHigh=fdata.nFileSizeHigh;
          NewPtr.PackSize=fdata.nFileSizeLow;
        }
        if (ReadNumLinks)
          NewPtr.NumberOfLinks=GetNumberOfLinks(fdata.cFileName);
      }
      else
      {
        NewPtr.PackSizeHigh=NewPtr.PackSize=0;
      }

      NewPtr.SortGroup=DEFAULT_SORT_GROUP;
      if (ReadOwners)
      {
        char Owner[NM];
        GetFileOwner(*ComputerName ? ComputerName:NULL,NewPtr.Name,Owner);
        strncpy(NewPtr.Owner,Owner,sizeof(NewPtr.Owner)-1);
      }
      if (!UpperDir && (NewPtr.FileAttr & FA_DIREC)==0)
        TotalFileCount++;

      memcpy(ListData+FileCount,&NewPtr,sizeof(NewPtr));
      FileCount++;

      if ((FileCount & 0x3f)==0 && clock()-StartTime>1000)
      {
        if (IsVisible())
        {
          char ReadMsg[100];
          if(!IsShowTitle)
          {
            Text(X1+1,Y1,COL_PANELBOX,Title);
            IsShowTitle=TRUE;
            SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
          }

          sprintf(ReadMsg,MSG(MReadingFiles),FileCount);
          TruncStr(ReadMsg,TitleLength-2);
          int MsgLength=strlen(ReadMsg);
          GotoXY(X1+1+(TitleLength-MsgLength-1)/2,Y1);
          mprintf(" %s ",ReadMsg);
        }
        if (CheckForEsc())
        {
          Message(MSG_WARNING,1,MSG(MUserBreakTitle),MSG(MOperationNotCompleted),MSG(MOk));
          break;
        }
      }
    }
    Done=!FindNextFile(FindHandle,&fdata);
  }

  int ErrCode=GetLastError();
  if (ErrCode!=ERROR_SUCCESS && ErrCode!=ERROR_NO_MORE_FILES && ErrCode!=ERROR_FILE_NOT_FOUND)
    Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MReadFolderError),MSG(MOk));

  FindClose(FindHandle);

  // "перекраску" вынесем в отдельный цикл - на медленных сетевых соединениях
  // вежнее считать конкент, а остальное потом.
  UpdateColorItems();

  if (IsColumnDisplayed(DIZ_COLUMN))
    ReadDiz();

  int NetRoot=FALSE;
  if (CurDir[0]=='\\' && CurDir[1]=='\\')
  {
    char *ChPtr=strchr(CurDir+2,'\\');
    if (ChPtr==NULL || strchr(ChPtr+1,'\\')==NULL)
      NetRoot=TRUE;
  }
  // пока кусок закомментим, возможно он даже и не пригодится.
  if (!DotsPresent && *PointToName(CurDir)!=0)// && !NetRoot)
  {
    if (FileCount>=AllocatedCount)
    {
      if ((CurPtr=(struct FileListItem *)realloc(ListData,(FileCount+1)*sizeof(*ListData)))!=NULL)
        ListData=CurPtr;
    }
    if (CurPtr!=NULL)
    {
      AddParentPoint(ListData+FileCount,FileCount);
      FileCount++;
    }
  }

  if (AnotherPanel->GetMode()==PLUGIN_PANEL)
  {
    HANDLE hAnotherPlugin=AnotherPanel->GetPluginHandle();
    PluginPanelItem *PanelData=NULL;
    char Path[NM];
    int PanelCount=0;
    strcpy(Path,CurDir);
    AddEndSlash(Path);
    if (CtrlObject->Plugins.GetVirtualFindData(hAnotherPlugin,&PanelData,&PanelCount,Path))
    {
      if ((CurPtr=(struct FileListItem *)realloc(ListData,(FileCount+PanelCount)*sizeof(*ListData)))!=NULL)
      {
        ListData=CurPtr;
        for (int I=0;I<PanelCount;I++)
        {
          WIN32_FIND_DATA &fdata=PanelData[I].FindData;
          CurPtr=ListData+FileCount+I;
          PluginToFileListItem(&PanelData[I],CurPtr);
          CurPtr->Position=FileCount;
          TotalFileSize+=int64(fdata.nFileSizeHigh,fdata.nFileSizeLow);
          CurPtr->PrevSelected=CurPtr->Selected=0;
          CurPtr->ShowFolderSize=0;
          if ((CurPtr->FileAttr & FA_DIREC)==0)
            CurPtr->SortGroup=CtrlObject->GrpSort->GetGroup(CurPtr->Name);
          else
            CurPtr->SortGroup=DEFAULT_SORT_GROUP;
          if (strcmp(fdata.cFileName,"..")!=0 && (CurPtr->FileAttr & FA_DIREC)==0)
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
    ReadSortGroups();

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
  if (SaveDir) {
    SetCurrentDirectory(SaveDir);
  }
}

/*$ 22.06.2001 SKV
  Добавлен параметр для вызова после исполнения команды.
*/
int FileList::UpdateIfChanged(int Force)
{
  //_SVS(SysLog("CurDir='%s' Opt.AutoUpdateLimit=%d <= FileCount=%d",CurDir,Opt.AutoUpdateLimit,FileCount));
  if(!Opt.AutoUpdateLimit || FileCount <= Opt.AutoUpdateLimit)
  {
    if (IsVisible() && (clock()-LastUpdateTime>2000 || Force))
    {
      if(!Force)ProcessPluginEvent(FE_IDLE,NULL);
      if (PanelMode==NORMAL_PANEL && hListChange!=NULL)
        if (WaitForSingleObject(hListChange,0)==WAIT_OBJECT_0)
        {
          Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
          // В этом случае - просто перекрасим
          UpdateColorItems();
          if (AnotherPanel->GetType()==INFO_PANEL)
          {
            AnotherPanel->Update(UPDATE_KEEP_SELECTION);
            if(!Force)
              AnotherPanel->Redraw();
          }
          Update(UPDATE_KEEP_SELECTION);
          if(!Force)
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
  char AnsiName[NM];
  OemToChar(CurDir,AnsiName);
  CloseChangeNotification();
  SetFileApisToANSI();
  hListChange=FindFirstChangeNotification(AnsiName,CheckTree,
                      FILE_NOTIFY_CHANGE_FILE_NAME|
                      FILE_NOTIFY_CHANGE_DIR_NAME|
                      FILE_NOTIFY_CHANGE_ATTRIBUTES|
                      FILE_NOTIFY_CHANGE_SIZE|
                      FILE_NOTIFY_CHANGE_LAST_WRITE);
  SetFileApisToOEM();
}


void FileList::CloseChangeNotification()
{
  if (hListChange!=NULL)
  {
    FindCloseChangeNotification(hListChange);
    hListChange=NULL;
  }
}


void FileList::MoveSelection(struct FileListItem *ListData,long FileCount,
                             struct FileListItem *OldData,long OldFileCount)
{
  struct FileListItem *OldPtr;
  SelFileCount=0;
  SelFileSize=0;
  qsort((void *)OldData,OldFileCount,sizeof(*OldData),SortSearchList);
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


void FileList::UpdatePlugin(int KeepSelection)
{
  if (!IsVisible())
  {
    UpdateRequired=TRUE;
    UpdateRequiredMode=KeepSelection;
    return;
  }
  DizRead=FALSE;

  struct FileListItem *OldData;
  char CurName[NM],NextCurName[NM];
  long OldFileCount;

  CloseChangeNotification();

  LastCurFile=-1;

  struct OpenPluginInfo Info;
  CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

  if (Opt.ShowPanelFree && (Info.Flags & OPIF_REALNAMES))
  {
    int64 TotalSize,TotalFree;
    char DriveRoot[NM];
    GetPathRoot(CurDir,DriveRoot);
    if (!GetDiskSize(DriveRoot,&TotalSize,&TotalFree,&FreeDiskSize))
      FreeDiskSize=0;
  }

  PluginPanelItem *PanelData=NULL;
  int PluginFileCount;

  if (!CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&PluginFileCount,0))
  {
    DeleteListData(ListData,FileCount);
    PopPlugin(TRUE);
    Update(KeepSelection);
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
    strcpy(CurName,ListData[CurFile].Name);
    if (ListData[CurFile].Selected)
      for (int I=CurFile+1;I<FileCount;I++)
        if (!ListData[I].Selected)
        {
          strcpy(NextCurName,ListData[I].Name);
          break;
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
  ListData=(struct FileListItem*)malloc(sizeof(struct FileListItem)*(FileCount+1));

  if (ListData==NULL)
  {
    FileCount=0;
    return;
  }

  if (Filter==NULL)
    Filter=new PanelFilter(this);

  int DotsPresent=FALSE;

  int FileListCount=0;

  for (int I=0;I<FileCount;I++)
  {
    struct FileListItem *CurListData=&ListData[FileListCount];
    struct PluginPanelItem *CurPanelData=&PanelData[I];
    if (Info.Flags & OPIF_USEFILTER)
      if ((CurPanelData->FindData.dwFileAttributes & FA_DIREC)==0)
        if (!Filter->CheckName(CurPanelData->FindData.cFileName))
          continue;
    if (!Opt.ShowHidden && (CurPanelData->FindData.dwFileAttributes & (FA_HIDDEN|FA_SYSTEM)))
      continue;
    memset(CurListData,0,sizeof(*CurListData));
    PluginToFileListItem(CurPanelData,CurListData);
    CurListData->Position=I;
    if ((Info.Flags & OPIF_USEHIGHLIGHTING) || (Info.Flags & OPIF_USEATTRHIGHLIGHTING))
      CtrlObject->HiFiles->GetHiColor(
          (Info.Flags & OPIF_USEATTRHIGHLIGHTING) ? NULL:CurListData->Name,
          CurListData->FileAttr,&CurListData->Colors);
    if ((Info.Flags & OPIF_USESORTGROUPS) && (CurListData->FileAttr & FA_DIREC)==0)
      CurListData->SortGroup=CtrlObject->GrpSort->GetGroup(CurListData->Name);
    else
      CurListData->SortGroup=DEFAULT_SORT_GROUP;
    if (CurListData->DizText==NULL)
    {
      CurListData->DeleteDiz=FALSE;
      CurListData->DizText=NULL;
    }
    if (strcmp(CurListData->Name,"..")==0)
    {
      DotsPresent=TRUE;
      CurListData->FileAttr|=FA_DIREC;
    }
    else
      if ((CurListData->FileAttr & FA_DIREC)==0)
        TotalFileCount++;
    TotalFileSize+=int64(CurListData->UnpSizeHigh,CurListData->UnpSize);
    FileListCount++;
  }

  FileCount=FileListCount;

  if ((Info.Flags & OPIF_ADDDOTS) && !DotsPresent)
  {
    struct FileListItem *CurPtr;
    AddParentPoint((CurPtr=ListData+FileCount),FileCount);
    /* $ 22.11.2001 VVM
      + Не забыть раскрасить :) */
    if ((Info.Flags & OPIF_USEHIGHLIGHTING) || (Info.Flags & OPIF_USEATTRHIGHLIGHTING))
      CtrlObject->HiFiles->GetHiColor(
          (Info.Flags & OPIF_USEATTRHIGHLIGHTING) ? NULL:CurPtr->Name,
          CurPtr->FileAttr,&CurPtr->Colors);
    /* VVM $ */
    if (Info.HostFile && *Info.HostFile)
    {
      WIN32_FIND_DATA FindData;
      HANDLE FindHandle;
      FindHandle=FindFirstFile(Info.HostFile,&FindData);
      FindClose(FindHandle);

      if (FindHandle!=INVALID_HANDLE_VALUE)
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
        for (int J=0;J<PluginFileCount;J++)
          if (LocalStricmp(PanelData[J].FindData.cFileName,Info.DescrFiles[I])==0)
          {
            char TempDir[NM],DizName[NM];
            strcpy(TempDir,Opt.TempPath);
            strcat(TempDir,FarTmpXXXXXX);
            if (mktemp(TempDir)!=NULL && CreateDirectory(TempDir,NULL))
            //if (FarMkTemp(TempDir,"Far")!=NULL && CreateDirectory(TempDir,NULL))
            {
              if (CtrlObject->Plugins.GetFile(hPlugin,&PanelData[J],TempDir,DizName,OPM_SILENT|OPM_VIEW|OPM_DESCR))
              {
                strcpy(PluginDizName,Info.DescrFiles[I]);
                Diz.Read("",DizName);
                DeleteFileWithFolder(DizName);
                I=Info.DescrFilesNumber;
                break;
              }
              RemoveDirectory(TempDir);
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
      CurPtr->DizText=Diz.GetDizTextAddr(CurPtr->Name,CurPtr->ShortName,CurPtr->UnpSize);
    }
  }
}


void FileList::ReadSortGroups()
{
  if (SortGroupsRead)
    return;
  SortGroupsRead=TRUE;
  struct FileListItem *CurPtr=ListData;
  for (int I=0;I<FileCount;I++,CurPtr++)
  {
    if ((CurPtr->FileAttr & FA_DIREC)==0)
      CurPtr->SortGroup=CtrlObject->GrpSort->GetGroup(CurPtr->Name);
    else
      CurPtr->SortGroup=DEFAULT_SORT_GROUP;
  }
}

// Обнулить текущий CurPtr и занести предопределенные данные для каталога ".."
void FileList::AddParentPoint(struct FileListItem *CurPtr,long CurFilePos)
{
  memset(CurPtr,0,sizeof(struct FileListItem));
  strcpy(CurPtr->ShortName,"..");
  strcpy(CurPtr->Name,"..");
  CurPtr->Position=CurFilePos;
  CurPtr->FileAttr=FA_DIREC;
}
