/*
flplugin.cpp

Файловая панель - работа с плагинами

*/

/* Revision: 1.29 08.05.2002 $ */

/*
Modify:
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

#include "filelist.hpp"
#include "plugin.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "filepanels.hpp"
#include "history.hpp"
#include "ctrlobj.hpp"

void FileList::PushPlugin(HANDLE hPlugin,char *HostFile)
{
  DeleteAllDataToDelete();
  PluginsStack=(struct PluginsStackItem *)realloc(PluginsStack,(PluginsStackSize+1)*sizeof(*PluginsStack));
  struct PluginsStackItem *PStack=PluginsStack+PluginsStackSize;
  PStack->hPlugin=hPlugin;
  strcpy(PStack->HostFile,HostFile);
  PStack->Modified=FALSE;
  PStack->PrevViewMode=ViewMode;
  PStack->PrevSortMode=SortMode;
  PStack->PrevSortOrder=SortOrder;
  PluginsStackSize++;
}


int FileList::PopPlugin(int EnableRestoreViewMode)
{
  DeleteAllDataToDelete();
  if (PluginsStackSize==0)
  {
    PanelMode=NORMAL_PANEL;
    return(FALSE);
  }
  PluginsStackSize--;
  struct PluginsStackItem *PStack=PluginsStack+PluginsStackSize;
  if (EnableRestoreViewMode)
  {
    struct OpenPluginInfo Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
    if (Info.StartPanelMode)
      SetViewMode(PStack->PrevViewMode);
    if (Info.StartSortMode)
    {
      SortMode=PStack->PrevSortMode;
      SortOrder=PStack->PrevSortOrder;
    }
  }
  CtrlObject->Plugins.ClosePlugin(hPlugin);
  // после ClosePlugin переменная SortOrder по каким-то волшебным причинам
  // становится = -1 (хотя допустимы 0 или 1)...
  if(SortOrder==-1) // ...восстановим.
  {
    SortOrder=1; // как в конструкторе заказывали ;-)
  }
  if (PluginsStackSize>0)
  {
    hPlugin=PluginsStack[PluginsStackSize-1].hPlugin;
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
        strncpy(PanelItem.FindData.cFileName,PointToName(PStack->HostFile),sizeof(PanelItem.FindData.cFileName)-1);
        CtrlObject->Plugins.DeleteFiles(hPlugin,&PanelItem,1,0);
      }
      FarChDir(SaveDir);
    }
    struct OpenPluginInfo Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
    if ((Info.Flags & OPIF_REALNAMES)==0)
      DeleteFileWithFolder(PStack->HostFile);
  }
  else
  {
    PanelMode=NORMAL_PANEL;
    /* <TODO>
       Нужно учесть тот факт, что кто-то или что-то может менять
       принудительно пареметры не своей панели.
    */
    /*
    ViewMode=PStack->PrevViewMode;
    SortMode=PStack->PrevSortMode;
    SortOrder=PStack->PrevSortOrder;
    */
    /* </TODO>*/
  }
  PluginsStack=(struct PluginsStackItem *)realloc(PluginsStack,PluginsStackSize*sizeof(*PluginsStack));
  if (EnableRestoreViewMode)
    CtrlObject->Cp()->RedrawKeyBar();
  return(TRUE);
}


int FileList::FileNameToPluginItem(char *Name,PluginPanelItem *pi)
{
  char TempDir[NM],*ChPtr;
  strncpy(TempDir,Name,sizeof(TempDir)-1);
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
  strncpy(pi->FindData.cAlternateFileName,fi->ShortName,sizeof(pi->FindData.cAlternateFileName)-1);
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
    pi->UserData=(DWORD)malloc(Size);
    /* SVS $ */
    memcpy((void *)pi->UserData,(void *)fi->UserData,Size);
  }
  else
    pi->UserData=fi->UserData;
  pi->CRC32=fi->CRC32;
  pi->Reserved[0]=pi->Reserved[1]=0;
  pi->Owner=NULL;
}


void FileList::PluginToFileListItem(struct PluginPanelItem *pi,struct FileListItem *fi)
{
  strncpy(fi->Name,pi->FindData.cFileName,sizeof(fi->Name)-1);
  strncpy(fi->ShortName,pi->FindData.cAlternateFileName,sizeof(fi->ShortName)-1);
  strncpy(fi->Owner,NullToEmpty(pi->Owner),sizeof(fi->Owner)-1);
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
    fi->UserData=(DWORD)malloc(Size);
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


HANDLE FileList::OpenPluginForFile(char *FileName)
{
  SetCurPath();
  FILE *ProcessFile=fopen(FileName,"rb");
  if (ProcessFile)
  {
    char *Buffer=new char[Opt.PluginMaxReadData];
    if(Buffer)
    {
      int ReadSize=fread(Buffer,1,Opt.PluginMaxReadData,ProcessFile);
      fclose(ProcessFile);

      CtrlObject->Cp()->GetAnotherPanel(this)->CloseFile();
      HANDLE hNewPlugin=CtrlObject->Plugins.OpenFilePlugin(FileName,(unsigned char *)Buffer,ReadSize);
      delete[] Buffer;
      return(hNewPlugin);
    }
  }
  return(INVALID_HANDLE_VALUE);
}


void FileList::CreatePluginItemList(struct PluginPanelItem *(&ItemList),int &ItemNumber)
{
  long SaveSelPosition=GetSelPosition;
  char SelName[NM];
  int FileAttr;
  ItemNumber=0;
  ItemList=new PluginPanelItem[SelFileCount+1];
  GetSelName(NULL,FileAttr);
  if (ItemList!=NULL)
    while (GetSelName(SelName,FileAttr))
      if (((FileAttr & FA_DIREC)==0 || strcmp(SelName,"..")!=0)
          && LastSelPosition>=0 && LastSelPosition<FileCount)
      {
        FileListToPluginItem(ListData+LastSelPosition,ItemList+ItemNumber);
        ItemNumber++;
      }
  GetSelPosition=SaveSelPosition;
}


void FileList::DeletePluginItemList(struct PluginPanelItem *(&ItemList),int &ItemNumber)
{
  struct PluginPanelItem *PItemList=ItemList;
  if(PItemList)
  {
    for (int I=0;I<ItemNumber;I++,PItemList++)
      if ((PItemList->Flags & PPIF_USERDATA) && PItemList->UserData)
        free((void *)PItemList->UserData);
    delete[] ItemList;
  }
}


void FileList::PluginDelete()
{
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
    if ((hCurPlugin=OpenPluginForFile(SelName))!=INVALID_HANDLE_VALUE &&
        hCurPlugin!=(HANDLE)-2)
    {
      struct PluginPanelItem *ItemList;
      int ItemNumber;
      if (CtrlObject->Plugins.GetFindData(hCurPlugin,&ItemList,&ItemNumber,0))
      {
        ExitLoop=CtrlObject->Plugins.GetFiles(hCurPlugin,ItemList,ItemNumber,FALSE,DestPath,OpMode)!=1;
        if (!ExitLoop)
          ClearLastGetSelection();
        CtrlObject->Plugins.FreeFindData(hCurPlugin,ItemList,ItemNumber);
        OpMode|=OPM_SILENT;
      }
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
  HANDLE hNewPlugin=CtrlObject->Plugins.OpenFilePlugin(NULL,NULL,0);
  if (hNewPlugin!=INVALID_HANDLE_VALUE && hNewPlugin!=(HANDLE)-2)
  {
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
      for (int I=1; I < FileCount; I++)
        if (CompareFileTime(&ListData[I].CreationTime,&ListData[LastPos].CreationTime)==1)
          LastPos=I;
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
    PutCode=CtrlObject->Plugins.PutFiles(AnotherFilePanel->hPlugin,ItemList,ItemNumber,Move,0);
    if (PutCode==1 || PutCode==2)
    {
      if (!ReturnCurrentFile)
        ClearSelection();
      PutDizToPlugin(AnotherFilePanel,ItemList,ItemNumber,FALSE,Move,&Diz,&AnotherFilePanel->Diz);
      AnotherPanel->SetPluginModified();
    }
    else
      if (!ReturnCurrentFile)
        PluginClearSelection(ItemList,ItemNumber);
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
  memset(Info,0,sizeof(*Info));
  if (PanelMode==PLUGIN_PANEL)
  {
    struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
    CtrlObject->Plugins.GetPluginInfo(ph->PluginNumber,Info);
  }
}


void FileList::GetOpenPluginInfo(struct OpenPluginInfo *Info)
{
  memset(Info,0,sizeof(*Info));
  if (PanelMode==PLUGIN_PANEL)
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,Info);
}


/*
   Функция для вызова команды "Архивные команды" (Shift-F3)
*/
void FileList::ProcessHostFile()
{
  if (FileCount>0 && SetCurPath())
  {
    int Done=FALSE;

    SaveSelection();

    if (PanelMode==PLUGIN_PANEL && *PluginsStack[PluginsStackSize-1].HostFile)
    {
      struct PluginPanelItem *ItemList;
      int ItemNumber;
      CreatePluginItemList(ItemList,ItemNumber);
      Done=CtrlObject->Plugins.ProcessHostFile(hPlugin,ItemList,ItemNumber,0);
      if (Done)
        SetPluginModified();
      else
      {
        if (!ReturnCurrentFile)
          PluginClearSelection(ItemList,ItemNumber);
        Redraw();
      }
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
  int Done=-1;

  HANDLE hNewPlugin=OpenPluginForFile(ListData[Idx].Name);

  if (hNewPlugin!=INVALID_HANDLE_VALUE && hNewPlugin!=(HANDLE)-2)
  {
    struct PluginPanelItem *ItemList;
    int ItemNumber;
    if (CtrlObject->Plugins.GetFindData(hNewPlugin,&ItemList,&ItemNumber,OPM_TOPLEVEL))
    {
      /* $ 26.04.2001 DJ
         в ProcessHostFile не передавался OPM_TOPLEVEL
      */
      Done=CtrlObject->Plugins.ProcessHostFile(hNewPlugin,ItemList,ItemNumber,OPM_TOPLEVEL);
      /* DJ $ */
      CtrlObject->Plugins.FreeFindData(hNewPlugin,ItemList,ItemNumber);
    }
    CtrlObject->Plugins.ClosePlugin(hNewPlugin);
  }
  return Done;
}



void FileList::SetPluginMode(HANDLE hPlugin,char *PluginFile)
{
  if (PanelMode!=PLUGIN_PANEL)
    CtrlObject->FolderHistory->AddToHistory(CurDir,NULL,0);

  FileList::hPlugin=hPlugin;
  PushPlugin(hPlugin,PluginFile);
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
  DeleteAllDataToDelete();
  Info->PanelItems=NULL;
  Info->SelectedItems=NULL;

  if(FullInfo)
  {
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

    CreatePluginItemList(Info->SelectedItems,Info->SelectedItemsNumber);

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

  char ColumnTypes[80],ColumnWidths[80];
  ViewSettingsToText(ViewSettings.ColumnType,ViewSettings.ColumnWidth,
                     ViewSettings.ColumnCount,ColumnTypes,ColumnWidths);
  strncpy(Info->ColumnTypes,ColumnTypes,sizeof(Info->ColumnTypes)-1);
  strncpy(Info->ColumnWidths,ColumnWidths,sizeof(Info->ColumnWidths)-1);
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
  int Command=PluginCommand;
  PluginCommand=-1;
  if (PanelMode==PLUGIN_PANEL)
    switch(Command)
    {
      case FCTL_CLOSEPLUGIN:
        SetCurDir((char *)PluginParam,TRUE);
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
