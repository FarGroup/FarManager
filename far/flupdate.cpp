/*
flupdate.cpp

Файловая панель - чтение имен файлов

*/

/* Revision: 1.55 20.02.2006 $ */

/*
Modify:
  20.02.2006 SVS
    ! У ConvertNameToShort новый параметр - размер для Dest
  07.12.2005 SVS
    + инициализация Is_FS_NTFS
  09.09.2005 SVS
    ! Функционал получения имени компьютера по текущему пути вынесен в
      отдельную функцию CurPath2ComputerName()
  30.08.2004 SVS
    + Opt.IgnoreErrorBadPathName - Игнорировать ошибку ERROR_BAD_PATHNAME под масдаем, по умолчанию = 0
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  08.06.2004 SVS
    ! Вместо GetDriveType теперь вызываем FAR_GetDriveType().
    ! Вместо "DriveType==DRIVE_CDROM" вызываем IsDriveTypeCDROM()
  01.03.2004 SVS
    ! Обертки FAR_OemTo* и FAR_CharTo* вокруг одноименных WinAPI-функций
      (задел на будущее + править впоследствии только 1 файл)
  28.02.2004 SVS
    + Opt.AutoUpdateRemoteDrive - управление автоапдейтом сетевых дисков
  13.11.2003 SVS
    + _ALGO()
  09.10.2003 SVS
    ! SetFileApisToANSI() и SetFileApisToOEM() заменены на SetFileApisTo() с параметром
      APIS2ANSI или APIS2OEM - задел на будущее
  12.09.2003 VVM
    + Очистить буфер, если не смогли сменить каталог.
  12.09.2003 SVS
    ! Немного увеличим буфер для GetPathRootOne
  06.06.2003 SVS
    + сброс кэша SID`ов
  25.02.2003 SVS
    ! "free/malloc/realloc -> xf_*" - что-то в прошлый раз пропустил.
    ! Вернем схему наложения раскраски взад - все за один цикл
  20.02.2003 SVS
    ! Заменим strcmp(FooBar,"..") на TestParentFolderName(FooBar)
    ! В FileList::PluginPutFilesToNew() вместо индексного массива
      применим указатели.
    ! В FileList::AddParentPoint() делаем статическую переменную
      типа FileListItem, при первом обращении к этой функции
      заполняем структуру, а потом просто юзаем.
  26.01.2003 IS
    ! FAR_DeleteFile вместо DeleteFile, FAR_RemoveDirectory вместо
      RemoveDirectory, просьба и впредь их использовать для удаления
      соответственно файлов и каталогов.
  21.12.2002 SVS
    ! Избавимся от "не той" прорисовки пересчетчика файлов в момент выхода
      из редактора в каталоге со значительным количеством файлов.
  18.06.2002 SVS
    ! Перед чтением контента (функция FileList::ReadFileNames - вот здесь
      возможно что код лишний!) проверим на "читаемость" диска и если надо
      вызовем нашу новую функцию IfGoHome(), которая молча...
  12.04.2002 SVS
    - BugZ#452 - Ctrl+N на ТмпПанели
  11.04.2002 SVS
    ! OPM_QUICKVIEW -> OPM_VIEW|OPM_QUICKVIEW
  08.04.2002 SVS
    ! Для QView в плагине (GetFiles) вместо OPM_VIEW передаем OPM_QUICKVIEW
  28.03.2002 KM
    - Падение VC на
      char *LocalName="A:";
      *LocalName=*CurDir;
      Так как память в LocalName ReadOnly.
  20.03.2002 SVS
    ! GetCurrentDirectory -> FarGetCurDir
  20.03.2002 DJ
    ! UpdateIfRequired() должен сам сбрасывать UpdateRequired
  19.03.2002 DJ
    ! возможность форсировать апдейт, даже если панель невидима
    ! UpdateIfRequired()
  01.03.2002 SVS
    ! Есть только одна функция создания временного файла - FarMkTempEx
  14.02.2002 VVM
    ! UpdateIfChanged принимает не булевый Force, а варианты из UIC_*
  13.02.2002 DJ
    ! не выставляем заголовок окна, если мы не текущий фрейм
  24.01.2002 VVM
    ! Поменяем логику обновления панелей
    ! hListChange работает с INVALID_HANDLE_VALUE, а не с NULL
  14.01.2002 IS
    - Опечатка
  27.12.2001 SVS
    ! "Ошибки производства".
  25.12.2001 SVS
    ! Небольшое уточнение
  25.12.2001 SVS
    ! немного оптимизации (если VC сам умеет это делать, то
      борманду нужно помочь)
  19.12.2001 VVM
    ! Сменим приоритеты. При Force обновление всегда! в UpdateIfChanged()
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
#include "manager.hpp"

int _cdecl SortSearchList(const void *el1,const void *el2);

/* $ 19.03.2002 DJ
   поддержка UPDATE_IGNORE_VISIBLE
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


void ReadFileNamesMsg(char *Msg)
{
  Message(0,0,MSG(MReadingTitleFiles),Msg);
  PreRedrawParam.Param1=Msg;
}

static void PR_ReadFileNamesMsg(void)
{
  ReadFileNamesMsg((char *)PreRedrawParam.Param1);
}


// ЭТО ЕСТЬ УЗКОЕ МЕСТО ДЛЯ СКОРОСТНЫХ ХАРАКТЕРИСТИК Far Manafer
// при считывании дирректории

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
        return;
      }
    }
  }

  SortGroupsRead=FALSE;

  if (Filter==NULL)
    Filter=new PanelFilter(this);

  if (GetFocus())
    CtrlObject->CmdLine->SetCurDir(CurDir);

  {
    char FileSysName[NM],RootDir[NM*2];
    ConvertNameToFull(CurDir,RootDir, sizeof(RootDir));
    GetPathRoot(RootDir,RootDir);
    if (GetVolumeInformation(RootDir,NULL,0,NULL,NULL,NULL,FileSysName,sizeof(FileSysName)))
      Is_FS_NTFS=!strcmp(FileSysName,"NTFS")?TRUE:FALSE;
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

  Done=((FindHandle=FindFirstFile("*.*",&fdata))==INVALID_HANDLE_VALUE);

  int AllocatedCount=0;
  struct FileListItem *NewPtr;

  // вне цикла получим указатель.
  char *PointToName_CurDir=PointToName(CurDir);

  // сформируем заголовок вне цикла
  char Title[2048];
  int TitleLength=Min((int)X2-X1-1,(int)sizeof(Title)-1);
  memset(Title,0x0CD,TitleLength);
  Title[TitleLength]=0;
  BOOL IsShowTitle=FALSE;
  BOOL NeedHighlight=Opt.Highlight && PanelMode != PLUGIN_PANEL;

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
        if ((CurPtr=(struct FileListItem *)xf_realloc(ListData,AllocatedCount*sizeof(*ListData)))==NULL)
          break;
        ListData=CurPtr;
      }

      NewPtr=ListData+FileCount;
      memset(NewPtr,0,sizeof(struct FileListItem));
      memcpy(&NewPtr->FileAttr,&fdata,sizeof(fdata));
      NewPtr->Position=FileCount++;
      NewPtr->NumberOfLinks=1;

      if ((fdata.dwFileAttributes & FA_DIREC) == 0)
      {
        TotalFileSize+=int64(fdata.nFileSizeHigh,fdata.nFileSizeLow);
        int Compressed=FALSE;
        if (ReadPacked && (fdata.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED))
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
            if(DrawMessage)
              SetPreRedrawFunc(PR_ReadFileNamesMsg);
            else
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
            int MsgLength=strlen(ReadMsg);
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
    Done=!FindNextFile(FindHandle,&fdata);
  }

  SetPreRedrawFunc(NULL);

  int ErrCode=GetLastError();
  if (!(ErrCode==ERROR_SUCCESS || ErrCode==ERROR_NO_MORE_FILES || ErrCode==ERROR_FILE_NOT_FOUND ||
        (ErrCode==ERROR_BAD_PATHNAME && WinVer.dwPlatformId != VER_PLATFORM_WIN32_NT && Opt.IgnoreErrorBadPathName)))
    Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MReadFolderError),MSG(MOk));

  FindClose(FindHandle);

  // "перекраску" вынесем в отдельный цикл - на медленных сетевых соединениях
  // вежнее считать конкент, а остальное потом.
//  UpdateColorItems();

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
      if ((CurPtr=(struct FileListItem *)xf_realloc(ListData,(FileCount+1)*sizeof(*ListData)))!=NULL)
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
          TotalFileSize+=int64(fdata.nFileSizeHigh,fdata.nFileSizeLow);
          CurPtr->PrevSelected=CurPtr->Selected=0;
          CurPtr->ShowFolderSize=0;
          if ((CurPtr->FileAttr & FA_DIREC)==0)
            CurPtr->SortGroup=CtrlObject->GrpSort->GetGroup(CurPtr->Name);
          else
            CurPtr->SortGroup=DEFAULT_SORT_GROUP;
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
  if(!Opt.AutoUpdateLimit || FileCount <= Opt.AutoUpdateLimit)
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
          // В этом случае - просто перекрасим
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
    Filter=new PanelFilter(this);

  int DotsPresent=FALSE;

  int FileListCount=0;

  struct PluginPanelItem *CurPanelData=PanelData;
  for (I=0; I < FileCount; I++, CurPanelData++)
  {
    struct FileListItem *CurListData=ListData+FileListCount;

    if (Info.Flags & OPIF_USEFILTER)
      if ((CurPanelData->FindData.dwFileAttributes & FA_DIREC)==0)
        if (!Filter->CheckName(CurPanelData->FindData.cFileName))
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
