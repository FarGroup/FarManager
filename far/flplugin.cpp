/*
flplugin.cpp

Файловая панель - работа с плагинами

*/

/* Revision: 1.53 11.08.2005 $ */

/*
Modify:
  11.08.2005 WARP
    ! see 02039.Mix.txt
  04.08.2005 SVS
    - косяки в стеке. Вроде разобрался КАК ЭТО РАБОТАЕТ!
  28.07.2005 SVS
    - Не восстановление режима сортировки при выходе из вложенного плагина
  25.07.2005 SVS
    - Забыл, что формат Info.StartPanelMode as '0'+номер режима.
  22.07.2005 SVS
    - нужно было уменьшать значение PluginsStackSize до вызова ClosePlugin()
    + пока закомменчено про PluginsStackItem.PrevViewSettings
  13.07.2005 SVS
    - Бага со стеком (см. 02023.Mix.txt)
    ! При удалении панели из стека вернем режимы сортировки
  29.06.2005 SVS
    - BugZ#1253 - некорректная обработка PanelMode.FullScreen
  21.04.2005 SVS
    ! При юзании FileList::ViewSettingsToText нужно учитывать, что ОНО думает,
      что два последних параметра размером с NM
  16.11.2004 WARP
    - FCTL_GET[ANOTHER]PANELSHORTINFO нарушали принцип "непрекосновенности" и
      нагло херили данные, полученные предыдущим вызовом FCTL_GET[ANOTHER]PANELINFO.
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  24.05.2004 SVS
    - BugZ#1085 - сбос цифровой сортировки на панели
  13.11.2003 SVS
    + _ALGO()
  06.10.2003 SVS
    - BugZ#964 - Ложное сообщение об Access Denied
      Не передавались атрибуты "файла"
    ! FileList::CreatePluginItemList() имеет доп.параметр - "добавлять '..'?"
      по умолчанию - "добавлять".
      В FileList::PluginGetPanelInfo() этот параметр = FALSE ("не добавлять")
  26.09.2003 SVS
    - BugZ#886 - FAR неверно реагирует на смену типа панели на лету.
  04.09.2003 SVS
    ! Вместо юзания CompareFileTime() применим трюк с сортировщиком файлов:
      приведем FILETIME к __int64
  02.09.2003 SVS
    - BugZ#937 - Необходимо выдача сообщения об Access Denied
    ! у FileList::OpenPluginForFile() новый параметр - файловые атрибуты
      (для того, чтобы сразу исключить DIR)
  30.07.2003 SVS
    - BugZ#856 - Сброс обратной сортировки после входа и выхода из плагина
  08.07.2003 SVS
    - Если текущий элемент 1 и нет более выделения и этот элемент "..",
      то имеем багу с PI.SelectedItems[0] - здесь мусор!
  14.05.2003 SVS
    + _ALGO()
  05.03.2003 SVS
    ! Закоментим _SVS
  20.02.2003 SVS
    ! Заменим strcmp(FooBar,"..") на TestParentFolderName(FooBar)
    ! В FileList::PluginPutFilesToNew() вместо индексного массива
      применим указатели.
  21.01.2003 SVS
    + xf_malloc,xf_realloc,xf_free - обертки вокруг malloc,realloc,free
      Просьба блюсти порядок и прописывать именно xf_* вместо простых.
  02.07.2002 SVS
    + _PluginsStackItem_Dump() - дамп стека плагинов
  25.06.2002 SVS
    ! При передаче плагину очередного итема (FileListToPluginItem) так же
      передадим поле Owner (если оно конечно заполнено!)
  08.05.2002 SVS
    ! Временно отменим 1248 (чем исправляем ситуацию Ctrl-7 Enter в архив Ctrl-2 и выходим)
  12.04.2002 SVS
    - BugZ#452 - Ctrl+N на ТмпПанели
  12.04.2002 IS
    ! PluginPutFilesToAnother теперь int - возвращает то, что возвращает
      PutFiles:
      -1 - прервано пользовтелем
       0 - неудача
       1 - удача
       2 - удача, курсор принудительно установлен на файл и заново его
           устанавливать не нужно
    + PluginPutFilesToNew учитывает код возврата PluginPutFilesToAnother
  11.04.2002 SVS
    ! Доп.Параметр у PluginGetPanelInfo - получать полную инфу или не полную
  10.04.2002 SVS
    - BugZ#353 - Команды из меню Shift-F3 не работают на нескольких выделенных архивах
  05.04.2002 SVS
    ! Вместо числа 0x20000 заюзаем Opt.PluginMaxReadData
  22.03.2002 SVS
    - strcpy - Fuck!
  20.03.2002 SVS
    ! GetCurrentDirectory -> FarGetCurDir
  01.03.2002 SVS
    ! Есть только одна функция создания временного файла - FarMkTempEx
  19.02.2002 SVS
    ! Восстановим режимы панелей после выталкивания плагина из стека.
  14.01.2002 IS
    ! chdir -> FarChDir
  25.12.2001 SVS
    ! немного оптимизации (если VC сам умеет это делать, то
      борманду нужно помочь)
  12.12.2001 SVS
    - Bug: после ClosePlugin переменная SortOrder по каким-то волшебным
      причинам становится = -1 (хотя допустимы 0 или 1)...
  27.09.2001 IS
    - Левый размер при использовании strncpy
  24.09.2001 SVS
    ! немного оптимизации (сокращение кода)
  17.08.2001 VVM
    + Обработка PluginPanelItem.CRC32
  06.07.2001 IS
    + Сохраним старое выделение в PluginSetSelection и в PluginClearSelection
  14.06.2001 SVS
    - Коррекция по поводу: "Надоело распаковывать some.foo.rar в каталог some\"
  17.05.2001 SVS
    ! Немного модификации типов параметров (чтобы doxygen матом не ругался :-)
  06.05.2001 DJ
    ! перетрях #include
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  26.04.2001 DJ
    - в ProcessHostFile() не передавался OPM_TOPLEVEL
  04.01.2001 SVS
    ! TranslateKeyToVK() -> keyboard.cpp
  11.11.2000 SVS
    ! FarMkTemp() - убираем (как всегда - то ставим, то тут же убираем :-(((
  11.11.2000 SVS
    ! Используем конструкцию FarMkTemp()
  08.09.2000 SVS
    + Добавка в FileList::TranslateKeyToVK для трансляции
      KEY_SHIFTDEL, KEY_ALTSHIFTDEL, KEY_CTRLSHIFTDEL
  23.07.2000 SVS
    + Клавиши (FileList::TranslateKeyToVK):
       Ctrl- Shift- Alt- CtrlShift- AltShift- CtrlAlt- Apps :-)
       KEY_LWIN (VK_LWIN), KEY_RWIN (VK_RWIN)
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "lang.hpp"
#include "filelist.hpp"
#include "plugin.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "filepanels.hpp"
#include "history.hpp"
#include "ctrlobj.hpp"
/*
   В стеке ФАРова панель не хранится - только плагиновые!
*/

void FileList::PushPlugin(HANDLE hPlugin,char *HostFile)
{
  _ALGO(CleverSysLog("FileList::PushPlugin()"));
  _ALGO(SysLog("hPlugin=%p, HostFile='%s'",hPlugin,HostFile?HostFile:"(NULL)"));
  _ALGO(PanelViewSettings_Dump("Prev",ViewSettings));
  DeleteAllDataToDelete();

  struct PluginsStackItem stItem;
  stItem.hPlugin=hPlugin;
  strcpy(stItem.HostFile,NullToEmpty(HostFile)); //??NULL??
  stItem.Modified=FALSE;
  stItem.PrevViewMode=ViewMode;
  stItem.PrevSortMode=SortMode;
  stItem.PrevSortOrder=SortOrder;
  stItem.PrevNumericSort=NumericSort;
  memmove(&stItem.PrevViewSettings,&ViewSettings,sizeof(struct PanelViewSettings));

  PluginsStack=(struct PluginsStackItem *)xf_realloc(PluginsStack,(PluginsStackSize+1)*sizeof(*PluginsStack));
  memmove(PluginsStack+PluginsStackSize,&stItem,sizeof(struct PluginsStackItem));
  PluginsStackSize++;
  _ALGO(PluginsStackItem_Dump("FileList::PushPlugin",PluginsStack,PluginsStackSize));
}

int FileList::PopPlugin(int EnableRestoreViewMode)
{
  struct OpenPluginInfo Info;
  Info.StructSize=0;

  DeleteAllDataToDelete();

  if (PluginsStackSize==0)
  {
    PanelMode=NORMAL_PANEL;
    return(FALSE);
  }

  PluginsStackSize--;

  // закрываем текущий плагин.
  CtrlObject->Plugins.ClosePlugin(hPlugin);

  struct PluginsStackItem *PStack=PluginsStack+PluginsStackSize; // указатель на плагин, с которого уходим

  if (PluginsStackSize>0)
  {
    hPlugin=PluginsStack[PluginsStackSize-1].hPlugin;

    if (EnableRestoreViewMode)
    {
      SetViewMode (PStack->PrevViewMode);

      SortMode=PStack->PrevSortMode;
      NumericSort=PStack->PrevNumericSort;
      SortOrder=PStack->PrevSortOrder;
    }

    if (PStack->Modified)
    {
      struct PluginPanelItem PanelItem;

      char SaveDir[NM];

      FarGetCurDir(sizeof(SaveDir),SaveDir);

      if (FileNameToPluginItem(PStack->HostFile,&PanelItem))
        CtrlObject->Plugins.PutFiles(hPlugin,&PanelItem,1,FALSE,0);
      else
      {
        memset(&PanelItem,0,sizeof(PanelItem));
        xstrncpy(PanelItem.FindData.cFileName,PointToName(PStack->HostFile),sizeof(PanelItem.FindData.cFileName)-1);
        CtrlObject->Plugins.DeleteFiles(hPlugin,&PanelItem,1,0);
      }

      FarChDir(SaveDir);
    }

    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

    if ((Info.Flags & OPIF_REALNAMES)==0)
      DeleteFileWithFolder(PStack->HostFile); // удаление файла от предыдущего плагина

  }
  else
  {
    PanelMode=NORMAL_PANEL;

    if(EnableRestoreViewMode)
    {
      SetViewMode (PStack->PrevViewMode);

      SortMode=PStack->PrevSortMode;
      NumericSort=PStack->PrevNumericSort;
      SortOrder=PStack->PrevSortOrder;
    }
  }

  PluginsStack=(struct PluginsStackItem *)xf_realloc(PluginsStack,PluginsStackSize*sizeof(*PluginsStack));

  if (EnableRestoreViewMode)
    CtrlObject->Cp()->RedrawKeyBar();

  return(TRUE);
}


int FileList::FileNameToPluginItem(char *Name,PluginPanelItem *pi)
{
  char TempDir[NM],*ChPtr;
  xstrncpy(TempDir,Name,sizeof(TempDir)-1);
  if ((ChPtr=strrchr(TempDir,'\\'))==NULL)
    return(FALSE);
  *ChPtr=0;
  FarChDir(TempDir);
  memset(pi,0,sizeof(*pi));
  HANDLE FindHandle;
  FindHandle=FindFirstFile(Name,&pi->FindData);
  if (FindHandle==INVALID_HANDLE_VALUE)
    return(FALSE);
  FindClose(FindHandle);
  return(TRUE);
}


void FileList::FileListToPluginItem(struct FileListItem *fi,struct PluginPanelItem *pi)
{
  strcpy(pi->FindData.cFileName,fi->Name);
  xstrncpy(pi->FindData.cAlternateFileName,fi->ShortName,sizeof(pi->FindData.cAlternateFileName)-1);
  pi->FindData.nFileSizeHigh=fi->UnpSizeHigh;
  pi->FindData.nFileSizeLow=fi->UnpSize;
  pi->FindData.dwReserved0=pi->FindData.dwReserved1=0;
  pi->PackSizeHigh=fi->PackSizeHigh;
  pi->PackSize=fi->PackSize;
  pi->FindData.dwFileAttributes=fi->FileAttr;
  pi->FindData.ftLastWriteTime=fi->WriteTime;
  pi->FindData.ftCreationTime=fi->CreationTime;
  pi->FindData.ftLastAccessTime=fi->AccessTime;
  pi->NumberOfLinks=fi->NumberOfLinks;
  pi->Flags=fi->UserFlags;
  if (fi->Selected)
    pi->Flags|=PPIF_SELECTED;
  pi->CustomColumnData=fi->CustomColumnData;
  pi->CustomColumnNumber=fi->CustomColumnNumber;
  pi->Description=fi->DizText;
  if (fi->UserData && (fi->UserFlags & PPIF_USERDATA))
  {
    DWORD Size=*(DWORD *)fi->UserData;
    /* $ 13.07.2000 SVS
       заменим new на malloc
    */
    pi->UserData=(DWORD)xf_malloc(Size);
    /* SVS $ */
    memcpy((void *)pi->UserData,(void *)fi->UserData,Size);
  }
  else
    pi->UserData=fi->UserData;
  pi->CRC32=fi->CRC32;
  pi->Reserved[0]=pi->Reserved[1]=0;
  pi->Owner=fi->Owner[0]?fi->Owner:NULL;
}


void FileList::PluginToFileListItem(struct PluginPanelItem *pi,struct FileListItem *fi)
{
  xstrncpy(fi->Name,pi->FindData.cFileName,sizeof(fi->Name)-1);
  xstrncpy(fi->ShortName,pi->FindData.cAlternateFileName,sizeof(fi->ShortName)-1);
  xstrncpy(fi->Owner,NullToEmpty(pi->Owner),sizeof(fi->Owner)-1);
  if (pi->Description)
  {
    fi->DizText=new char[strlen(pi->Description)+1];
    strcpy(fi->DizText,pi->Description);
    fi->DeleteDiz=TRUE;
  }
  else
    fi->DizText=NULL;
  fi->UnpSizeHigh=pi->FindData.nFileSizeHigh;
  fi->UnpSize=pi->FindData.nFileSizeLow;
  fi->PackSizeHigh=pi->PackSizeHigh;
  fi->PackSize=pi->PackSize;
  fi->FileAttr=pi->FindData.dwFileAttributes;
  fi->WriteTime=pi->FindData.ftLastWriteTime;
  fi->CreationTime=pi->FindData.ftCreationTime;
  fi->AccessTime=pi->FindData.ftLastAccessTime;
  fi->NumberOfLinks=pi->NumberOfLinks;
  fi->UserFlags=pi->Flags;

  if (pi->UserData && (pi->Flags & PPIF_USERDATA))
  {
    DWORD Size=*(DWORD *)pi->UserData;
    /* $ 13.07.2000 SVS
       заменим new на malloc
    */
    fi->UserData=(DWORD)xf_malloc(Size);
    /* SVS $ */
    memcpy((void *)fi->UserData,(void *)pi->UserData,Size);
  }
  else
    fi->UserData=pi->UserData;
  if (pi->CustomColumnNumber>0)
  {
    fi->CustomColumnData=new LPSTR[pi->CustomColumnNumber];
    for (int I=0;I<pi->CustomColumnNumber;I++)
      if (pi->CustomColumnData!=NULL && pi->CustomColumnData[I]!=NULL)
      {
        fi->CustomColumnData[I]=new char[strlen(pi->CustomColumnData[I])+1];
        strcpy(fi->CustomColumnData[I],pi->CustomColumnData[I]);
      }
      else
        fi->CustomColumnData[I]="";
  }
  fi->CustomColumnNumber=pi->CustomColumnNumber;
  fi->CRC32=pi->CRC32;
}


HANDLE FileList::OpenPluginForFile(char *FileName,DWORD FileAttr)
{
  _ALGO(CleverSysLog clv("FileList::OpenPluginForFile()"));
  _ALGO(SysLog("FileName='%s'",(FileName?FileName:"(NULL)")));

  if(!FileName || !*FileName || (FileAttr&FA_DIREC))
    return(INVALID_HANDLE_VALUE);

  SetCurPath();

  HANDLE hFile=INVALID_HANDLE_VALUE;
  if(WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
    hFile=FAR_CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE ,NULL,
                         OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN|FILE_FLAG_POSIX_SEMANTICS,
                         NULL);
  if(hFile==INVALID_HANDLE_VALUE)
    hFile=FAR_CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE ,NULL,
                         OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN, NULL);

  if (hFile==INVALID_HANDLE_VALUE)
  {
    //Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MEditTitle),MSG(MCannotOpenFile),FileName,MSG(MOk));
    Message(MSG_WARNING|MSG_ERRORTYPE,1,"",MSG(MOpenPluginCannotOpenFile),FileName,MSG(MOk));
    return(INVALID_HANDLE_VALUE);
  }

  char *Buffer=new char[Opt.PluginMaxReadData];
  if(Buffer)
  {
    DWORD BytesRead;
    _ALGO(SysLog("Read %d byte(s)",Opt.PluginMaxReadData));
    if(ReadFile(hFile,Buffer,Opt.PluginMaxReadData,&BytesRead,NULL))
    {
      CloseHandle(hFile);
      _ALGO(SysLogDump("First 128 bytes",0,(LPBYTE)Buffer,128,NULL));

      _ALGO(SysLog("close AnotherPanel file"));
      CtrlObject->Cp()->GetAnotherPanel(this)->CloseFile();

      _ALGO(SysLog("call Plugins.OpenFilePlugin {"));
      HANDLE hNewPlugin=CtrlObject->Plugins.OpenFilePlugin(FileName,(unsigned char *)Buffer,BytesRead);
      _ALGO(SysLog("}"));

      delete[] Buffer;

      return(hNewPlugin);
    }
    else
    {
      delete[] Buffer;
      _ALGO(SysLogLastError());
    }
  }

  CloseHandle(hFile);
  return(INVALID_HANDLE_VALUE);
}


void FileList::CreatePluginItemList(struct PluginPanelItem *(&ItemList),int &ItemNumber,BOOL AddTwoDot)
{
  if (!ListData)
    return;

  long SaveSelPosition=GetSelPosition;
  long OldLastSelPosition=LastSelPosition;

  char SelName[NM];
  int FileAttr;
  ItemNumber=0;
  ItemList=new PluginPanelItem[SelFileCount+1];
  if (ItemList!=NULL)
  {
    memset(ItemList,0,sizeof(struct PluginPanelItem) * (SelFileCount+1));
    GetSelName(NULL,FileAttr);
    while (GetSelName(SelName,FileAttr))
      if (((FileAttr & FA_DIREC)==0 || !TestParentFolderName(SelName))
          && LastSelPosition>=0 && LastSelPosition<FileCount)
      {
        FileListToPluginItem(ListData+LastSelPosition,ItemList+ItemNumber);
        ItemNumber++;
      }

    if(AddTwoDot && !ItemNumber && (FileAttr & FA_DIREC)) // это про ".."
    {
      strcpy(ItemList->FindData.cFileName,ListData->Name);
      ItemList->FindData.dwFileAttributes=ListData->FileAttr;
      ItemNumber++;
    }
  }

  LastSelPosition=OldLastSelPosition;
  GetSelPosition=SaveSelPosition;
}


void FileList::DeletePluginItemList(struct PluginPanelItem *(&ItemList),int &ItemNumber)
{
  struct PluginPanelItem *PItemList=ItemList;
  if(PItemList)
  {
    for (int I=0;I<ItemNumber;I++,PItemList++)
      if ((PItemList->Flags & PPIF_USERDATA) && PItemList->UserData)
        xf_free((void *)PItemList->UserData);
    delete[] ItemList;
  }
}


void FileList::PluginDelete()
{
  _ALGO(CleverSysLog clv("FileList::PluginDelete()"));
  struct PluginPanelItem *ItemList;
  int ItemNumber;
  SaveSelection();
  CreatePluginItemList(ItemList,ItemNumber);
  if (ItemList!=NULL && ItemNumber>0)
  {
    if (CtrlObject->Plugins.DeleteFiles(hPlugin,ItemList,ItemNumber,0))
    {
      SetPluginModified();
      PutDizToPlugin(this,ItemList,ItemNumber,TRUE,FALSE,NULL,&Diz);
    }
    DeletePluginItemList(ItemList,ItemNumber);
    Update(UPDATE_KEEP_SELECTION);
    Redraw();
    Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
    AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
    AnotherPanel->Redraw();
  }
}


void FileList::PutDizToPlugin(FileList *DestPanel,struct PluginPanelItem *ItemList,
                              int ItemNumber,int Delete,int Move,DizList *SrcDiz,
                              DizList *DestDiz)
{
  _ALGO(CleverSysLog clv("FileList::PutDizToPlugin()"));
  struct OpenPluginInfo Info;
  CtrlObject->Plugins.GetOpenPluginInfo(DestPanel->hPlugin,&Info);
  if (*DestPanel->PluginDizName==0 && Info.DescrFilesNumber>0)
    strcpy(DestPanel->PluginDizName,Info.DescrFiles[0]);
  if ((Opt.Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && IsDizDisplayed() ||
      Opt.Diz.UpdateMode==DIZ_UPDATE_ALWAYS) && *DestPanel->PluginDizName &&
      (Info.HostFile==NULL || *Info.HostFile==0 || DestPanel->GetModalMode() ||
      GetFileAttributes(Info.HostFile)!=0xFFFFFFFF))
  {
    CtrlObject->Cp()->LeftPanel->ReadDiz();
    CtrlObject->Cp()->RightPanel->ReadDiz();

    if (DestPanel->GetModalMode())
      DestPanel->ReadDiz();

    int DizPresent=FALSE;
    for (int I=0;I<ItemNumber;I++)
      if (ItemList[I].Flags & PPIF_PROCESSDESCR)
      {
        char *Name=ItemList[I].FindData.cFileName;
        char *ShortName=ItemList[I].FindData.cAlternateFileName;
        int Code;
        if (Delete)
          Code=DestDiz->DeleteDiz(Name,ShortName);
        else
        {
          Code=SrcDiz->CopyDiz(Name,ShortName,Name,ShortName,DestDiz);
          if (Code && Move)
            SrcDiz->DeleteDiz(Name,ShortName);
        }
        if (Code)
          DizPresent=TRUE;
      }
    if (DizPresent)
    {
      char TempDir[NM],DizName[NM];
      if (FarMkTempEx(TempDir) && CreateDirectory(TempDir,NULL))
      {
        char SaveDir[NM];
        FarGetCurDir(sizeof(SaveDir),SaveDir);
        sprintf(DizName,"%s\\%s",TempDir,DestPanel->PluginDizName);
        DestDiz->Flush("",DizName);
        if (Move)
          SrcDiz->Flush("",NULL);
        struct PluginPanelItem PanelItem;
        if (FileNameToPluginItem(DizName,&PanelItem))
          CtrlObject->Plugins.PutFiles(DestPanel->hPlugin,&PanelItem,1,FALSE,OPM_SILENT|OPM_DESCR);
        else
          if (Delete)
          {
            PluginPanelItem pi;
            memset(&pi,0,sizeof(pi));
            strcpy(pi.FindData.cFileName,DestPanel->PluginDizName);
            CtrlObject->Plugins.DeleteFiles(DestPanel->hPlugin,&pi,1,OPM_SILENT);
          }
        FarChDir(SaveDir);
        DeleteFileWithFolder(DizName);
      }
    }
  }
}


void FileList::PluginGetFiles(char *DestPath,int Move)
{
  _ALGO(CleverSysLog clv("FileList::PluginGetFiles()"));
  struct PluginPanelItem *ItemList, *PList;
  int ItemNumber;
  SaveSelection();
  CreatePluginItemList(ItemList,ItemNumber);
  if (ItemList!=NULL && ItemNumber>0)
  {
    int GetCode=CtrlObject->Plugins.GetFiles(hPlugin,ItemList,ItemNumber,Move,DestPath,0);
    if (Opt.Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && IsDizDisplayed() ||
        Opt.Diz.UpdateMode==DIZ_UPDATE_ALWAYS)
    {
      DizList DestDiz;
      int DizFound=FALSE;
      PList=ItemList;
      for (int I=0;I<ItemNumber;I++,PList++)
        if (PList->Flags & PPIF_PROCESSDESCR)
        {
          if (!DizFound)
          {
            CtrlObject->Cp()->LeftPanel->ReadDiz();
            CtrlObject->Cp()->RightPanel->ReadDiz();
            DestDiz.Read(DestPath);
            DizFound=TRUE;
          }
          char *Name=PList->FindData.cFileName;
          char *ShortName=PList->FindData.cAlternateFileName;
          CopyDiz(Name,ShortName,Name,Name,&DestDiz);
        }
      DestDiz.Flush(DestPath);
    }
    if (GetCode==1)
    {
      if (!ReturnCurrentFile)
        ClearSelection();
      if (Move)
      {
        SetPluginModified();
        PutDizToPlugin(this,ItemList,ItemNumber,TRUE,FALSE,NULL,&Diz);
      }
    }
    else
      if (!ReturnCurrentFile)
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
  _ALGO(CleverSysLog clv("FileList::PluginToPluginFiles()"));
  struct PluginPanelItem *ItemList;
  int ItemNumber;
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  char TempDir[NM];
  if (AnotherPanel->GetMode()!=PLUGIN_PANEL)
    return;
  FileList *AnotherFilePanel=(FileList *)AnotherPanel;

  if (!FarMkTempEx(TempDir))
    return;
  SaveSelection();
  CreateDirectory(TempDir,NULL);
  CreatePluginItemList(ItemList,ItemNumber);
  if (ItemList!=NULL && ItemNumber>0)
  {
    int PutCode=CtrlObject->Plugins.GetFiles(hPlugin,ItemList,ItemNumber,FALSE,TempDir,OPM_SILENT);
    if (PutCode==1 || PutCode==2)
    {
      char SaveDir[NM];
      FarGetCurDir(sizeof(SaveDir),SaveDir);
      FarChDir(TempDir);
      PutCode=CtrlObject->Plugins.PutFiles(AnotherFilePanel->hPlugin,ItemList,ItemNumber,FALSE,0);
      if (PutCode==1 || PutCode==2)
      {
        if (!ReturnCurrentFile)
          ClearSelection();
        AnotherPanel->SetPluginModified();
        PutDizToPlugin(AnotherFilePanel,ItemList,ItemNumber,FALSE,FALSE,&Diz,&AnotherFilePanel->Diz);
        if (Move)
          if (CtrlObject->Plugins.DeleteFiles(hPlugin,ItemList,ItemNumber,OPM_SILENT))
          {
            SetPluginModified();
            PutDizToPlugin(this,ItemList,ItemNumber,TRUE,FALSE,NULL,&Diz);
          }
      }
      else
        if (!ReturnCurrentFile)
          PluginClearSelection(ItemList,ItemNumber);
      FarChDir(SaveDir);
    }
    DeleteDirTree(TempDir);
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


void FileList::PluginHostGetFiles()
{
  _ALGO(CleverSysLog clv("FileList::PluginHostGetFiles()"));
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  char DestPath[NM],SelName[NM],*ExtPtr;
  int FileAttr;

  SaveSelection();

  GetSelName(NULL,FileAttr);
  if (!GetSelName(SelName,FileAttr))
    return;

  AnotherPanel->GetCurDir(DestPath);
  if ((!AnotherPanel->IsVisible() || AnotherPanel->GetType()!=FILE_PANEL) &&
      SelFileCount==0 || *DestPath==0)
  {
    strcpy(DestPath,PointToName(SelName));
    // SVS: А зачем здесь велся поиск точки с начала?
    if ((ExtPtr=strrchr(DestPath,'.'))!=NULL)
      *ExtPtr=0;
  }

  int OpMode=OPM_TOPLEVEL,ExitLoop=FALSE;
  GetSelName(NULL,FileAttr);
  while (!ExitLoop && GetSelName(SelName,FileAttr))
  {
    HANDLE hCurPlugin;
    _ALGO(SysLog("call OpenPluginForFile('%s')",NullToEmpty(SelName)));
    if ((hCurPlugin=OpenPluginForFile(SelName,FileAttr))!=INVALID_HANDLE_VALUE &&
        hCurPlugin!=(HANDLE)-2)
    {
      struct PluginPanelItem *ItemList;
      int ItemNumber;
      _ALGO(SysLog("call Plugins.GetFindData()"));
      if (CtrlObject->Plugins.GetFindData(hCurPlugin,&ItemList,&ItemNumber,0))
      {
        _ALGO(SysLog("call Plugins.GetFiles()"));
        ExitLoop=CtrlObject->Plugins.GetFiles(hCurPlugin,ItemList,ItemNumber,FALSE,DestPath,OpMode)!=1;
        if (!ExitLoop)
        {
          _ALGO(SysLog("call ClearLastGetSelection()"));
          ClearLastGetSelection();
        }
        _ALGO(SysLog("call Plugins.FreeFindData()"));
        CtrlObject->Plugins.FreeFindData(hCurPlugin,ItemList,ItemNumber);
        OpMode|=OPM_SILENT;
      }
      _ALGO(SysLog("call Plugins.ClosePlugin"));
      CtrlObject->Plugins.ClosePlugin(hCurPlugin);
    }
  }
  Update(UPDATE_KEEP_SELECTION);
  Redraw();
  AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
  AnotherPanel->Redraw();
}


void FileList::PluginPutFilesToNew()
{
  _ALGO(CleverSysLog clv("FileList::PluginPutFilesToNew()"));
  //_ALGO(SysLog("FileName='%s'",(FileName?FileName:"(NULL)")));
  _ALGO(SysLog("call Plugins.OpenFilePlugin(NULL,NULL,0)"));
  HANDLE hNewPlugin=CtrlObject->Plugins.OpenFilePlugin(NULL,NULL,0);
  if (hNewPlugin!=INVALID_HANDLE_VALUE && hNewPlugin!=(HANDLE)-2)
  {
    _ALGO(SysLog("Create: FileList TmpPanel, FileCount=%d",FileCount));
    FileList TmpPanel;
    TmpPanel.SetPluginMode(hNewPlugin,"");
    TmpPanel.SetModalMode(TRUE);
    int PrevFileCount=FileCount;
    /* $ 12.04.2002 IS
       Если PluginPutFilesToAnother вернула число, отличное от 2, то нужно
       попробовать установить курсор на созданный файл.
    */
    int rc=PluginPutFilesToAnother(FALSE,&TmpPanel);
    if (rc!=2 && FileCount==PrevFileCount+1)
    /* IS $ */
    {
      int LastPos=0;
      /* Место, где вычисляются координаты вновьсозданного файла
         Позиционирование происходит на файл с максимальной датой
         создания файла. Посему, если какой-то злобный буратино поимел
         в текущем каталоге файло с датой создания поболее текущей,
         то корреткного позиционирования не произойдет!
      */
      struct FileListItem *PtrListData=ListData+1, *PtrLastPos=ListData;
      for (int I=1; I < FileCount; I++,PtrListData++)
      {
        if ((*(__int64*)&PtrListData->CreationTime - *(__int64*)&PtrLastPos->CreationTime) > 0)
        {
          PtrLastPos=ListData+(LastPos=I);
        }
      }
      CurFile=LastPos;
      Redraw();
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
  struct PluginPanelItem *ItemList;
  int ItemNumber,PutCode=0;
  SaveSelection();
  CreatePluginItemList(ItemList,ItemNumber);
  if (ItemList!=NULL && ItemNumber>0)
  {
    SetCurPath();
    _ALGO(SysLog("call Plugins.PutFiles"));
    PutCode=CtrlObject->Plugins.PutFiles(AnotherFilePanel->hPlugin,ItemList,ItemNumber,Move,0);
    if (PutCode==1 || PutCode==2)
    {
      if (!ReturnCurrentFile)
      {
        _ALGO(SysLog("call ClearSelection()"));
        ClearSelection();
      }
      _ALGO(SysLog("call PutDizToPlugin"));
      PutDizToPlugin(AnotherFilePanel,ItemList,ItemNumber,FALSE,Move,&Diz,&AnotherFilePanel->Diz);
      AnotherPanel->SetPluginModified();
    }
    else
      if (!ReturnCurrentFile)
        PluginClearSelection(ItemList,ItemNumber);
    _ALGO(SysLog("call DeletePluginItemList"));
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


void FileList::GetPluginInfo(struct PluginInfo *Info)
{
  _ALGO(CleverSysLog clv("FileList::GetPluginInfo()"));
  memset(Info,0,sizeof(*Info));
  if (PanelMode==PLUGIN_PANEL)
  {
    struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
    CtrlObject->Plugins.GetPluginInfo(ph->PluginNumber,Info);
  }
}


void FileList::GetOpenPluginInfo(struct OpenPluginInfo *Info)
{
  _ALGO(CleverSysLog clv("FileList::GetOpenPluginInfo()"));
  //_ALGO(SysLog("FileName='%s'",(FileName?FileName:"(NULL)")));
  memset(Info,0,sizeof(*Info));
  if (PanelMode==PLUGIN_PANEL)
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,Info);
}


/*
   Функция для вызова команды "Архивные команды" (Shift-F3)
*/
void FileList::ProcessHostFile()
{
  _ALGO(CleverSysLog clv("FileList::ProcessHostFile()"));
  //_ALGO(SysLog("FileName='%s'",(FileName?FileName:"(NULL)")));
  if (FileCount>0 && SetCurPath())
  {
    int Done=FALSE;

    SaveSelection();

    if (PanelMode==PLUGIN_PANEL && *PluginsStack[PluginsStackSize-1].HostFile)
    {
      struct PluginPanelItem *ItemList;
      int ItemNumber;
      _ALGO(SysLog("call CreatePluginItemList"));
      CreatePluginItemList(ItemList,ItemNumber);
      _ALGO(SysLog("call Plugins.ProcessHostFile"));
      Done=CtrlObject->Plugins.ProcessHostFile(hPlugin,ItemList,ItemNumber,0);
      if (Done)
        SetPluginModified();
      else
      {
        if (!ReturnCurrentFile)
          PluginClearSelection(ItemList,ItemNumber);
        Redraw();
      }
      _ALGO(SysLog("call DeletePluginItemList"));
      DeletePluginItemList(ItemList,ItemNumber);
      if (Done)
        ClearSelection();
    }
    else
    {
      int SCount=GetRealSelCount();
      if(SCount > 0)
      {
        struct FileListItem *CurPtr=ListData;
        for(int I=0; I < FileCount; ++I, CurPtr++)
        {
          if (CurPtr->Selected)
          {
            Done=ProcessOneHostFile(I);
            if(Done == 1)
              Select(CurPtr,0);
            else if(Done == -1)
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
        if((Done=ProcessOneHostFile(CurFile)) == 1)
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
  _ALGO(CleverSysLog clv("FileList::ProcessOneHostFile()"));
  int Done=-1;

  _ALGO(SysLog("call OpenPluginForFile([Idx=%d] '%s')",Idx,ListData[Idx].Name));
  HANDLE hNewPlugin=OpenPluginForFile(ListData[Idx].Name,ListData[Idx].FileAttr);

  if (hNewPlugin!=INVALID_HANDLE_VALUE && hNewPlugin!=(HANDLE)-2)
  {
    struct PluginPanelItem *ItemList;
    int ItemNumber;
    _ALGO(SysLog("call Plugins.GetFindData"));
    if (CtrlObject->Plugins.GetFindData(hNewPlugin,&ItemList,&ItemNumber,OPM_TOPLEVEL))
    {
      /* $ 26.04.2001 DJ
         в ProcessHostFile не передавался OPM_TOPLEVEL
      */
      _ALGO(SysLog("call Plugins.ProcessHostFile"));
      Done=CtrlObject->Plugins.ProcessHostFile(hNewPlugin,ItemList,ItemNumber,OPM_TOPLEVEL);
      /* DJ $ */
      _ALGO(SysLog("call Plugins.FreeFindData"));
      CtrlObject->Plugins.FreeFindData(hNewPlugin,ItemList,ItemNumber);
    }
    _ALGO(SysLog("call Plugins.ClosePlugin"));
    CtrlObject->Plugins.ClosePlugin(hNewPlugin);
  }
  return Done;
}



void FileList::SetPluginMode(HANDLE hPlugin,char *PluginFile)
{
  if (PanelMode!=PLUGIN_PANEL)
    CtrlObject->FolderHistory->AddToHistory(CurDir,NULL,0);

  PushPlugin(hPlugin,PluginFile);

  FileList::hPlugin=hPlugin;
  PanelMode=PLUGIN_PANEL;
  struct OpenPluginInfo Info;
  CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
  if (Info.StartPanelMode)
    SetViewMode(VIEW_0+Info.StartPanelMode-'0');
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


void FileList::PluginGetPanelInfo(struct PanelInfo *Info,int FullInfo)
{
//  DeleteAllDataToDelete();
  Info->PanelItems=NULL;
  Info->SelectedItems=NULL;

  if(FullInfo)
  {
    DeleteAllDataToDelete();

    Info->ItemsNumber=0;
    Info->PanelItems=new PluginPanelItem[FileCount+1];
    if (Info->PanelItems!=NULL)
    {
      struct FileListItem *CurPtr=ListData;
      for (int I=0; I < FileCount; I++, CurPtr++)
      {
        FileListToPluginItem(CurPtr,Info->PanelItems+Info->ItemsNumber);
        Info->ItemsNumber++;
      }
    }
    DataToDelete[DataToDeleteCount]=Info->PanelItems;
    DataSizeToDelete[DataToDeleteCount++]=Info->ItemsNumber;

    CreatePluginItemList(Info->SelectedItems,Info->SelectedItemsNumber,FALSE);

    DataToDelete[DataToDeleteCount]=Info->SelectedItems;
    DataSizeToDelete[DataToDeleteCount++]=Info->SelectedItemsNumber;
  }
  else
  {
    Info->ItemsNumber=FileCount;
    Info->SelectedItemsNumber=GetSelCount();
    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
    /* Вот на счет ЭТОГО не уверен! */
    DataToDelete[DataToDeleteCount]=NULL;
    DataSizeToDelete[DataToDeleteCount++]=0;
    DataToDelete[DataToDeleteCount]=NULL;
    DataSizeToDelete[DataToDeleteCount++]=0;
    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
  }

  Info->CurrentItem=CurFile;
  Info->TopPanelItem=CurTopFile;

  char ColumnTypes[NM],ColumnWidths[NM];
  ViewSettingsToText(ViewSettings.ColumnType,ViewSettings.ColumnWidth,
                     ViewSettings.ColumnCount,ColumnTypes,ColumnWidths);
  xstrncpy(Info->ColumnTypes,ColumnTypes,sizeof(Info->ColumnTypes)-1);
  xstrncpy(Info->ColumnWidths,ColumnWidths,sizeof(Info->ColumnWidths)-1);
  Info->ShortNames=ShowShortNames;
}


void FileList::PluginSetSelection(struct PanelInfo *Info)
{
  /* $ 06.07.2001 IS Сохраним старое выделение */
  SaveSelection();
  /* IS $ */
  struct FileListItem *CurPtr=ListData;
  for (int I=0; I < FileCount && I < Info->ItemsNumber; I++, CurPtr++)
  {
    int Selection=(Info->PanelItems[I].Flags & PPIF_SELECTED)!=0;
    Select(CurPtr,Selection);
  }
  if (SelectedFirst)
    SortFileList(TRUE);
}


void FileList::ProcessPluginCommand()
{
  _ALGO(CleverSysLog clv("FileList::ProcessPluginCommand"));
  _ALGO(SysLog("PanelMode=%s",(PanelMode==PLUGIN_PANEL?"PLUGIN_PANEL":"NORMAL_PANEL")));
  int Command=PluginCommand;
  PluginCommand=-1;
  if (PanelMode==PLUGIN_PANEL)
    switch(Command)
    {
      case FCTL_CLOSEPLUGIN:
        _ALGO(SysLog("Command=FCTL_CLOSEPLUGIN"));
        SetCurDir((char *)PluginParam,TRUE);
        if(!PluginParam || !*(char *)PluginParam)
          Update(UPDATE_KEEP_SELECTION);
        Redraw();
        break;
    }
}

void FileList::SetPluginModified()
{
  if (PluginsStackSize>0)
    PluginsStack[PluginsStackSize-1].Modified=TRUE;
}


HANDLE FileList::GetPluginHandle()
{
  return(hPlugin);
}


int FileList::ProcessPluginEvent(int Event,void *Param)
{
  if (PanelMode==PLUGIN_PANEL)
    return(CtrlObject->Plugins.ProcessEvent(hPlugin,Event,Param));
  return(FALSE);
}


void FileList::PluginClearSelection(struct PluginPanelItem *ItemList,int ItemNumber)
{
  /* $ 06.07.2001 IS Сохраним старое выделение */
  SaveSelection();
  /* IS $ */
  int FileNumber=0,PluginNumber=0;
  while (PluginNumber<ItemNumber)
  {
    struct PluginPanelItem *CurPluginPtr=ItemList+PluginNumber;
    if ((CurPluginPtr->Flags & PPIF_SELECTED)==0)
    {
      while (LocalStricmp(CurPluginPtr->FindData.cFileName,ListData[FileNumber].Name)!=0)
        if (++FileNumber>=FileCount)
          return;
      Select(&ListData[FileNumber++],0);
    }
    PluginNumber++;
  }
}
