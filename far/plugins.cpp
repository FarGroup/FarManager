/*
plugins.cpp

Работа с плагинами (низкий уровень, кое-что повыше в flplugin.cpp)

*/

/* Revision: 1.12 03.08.2000 $ */

/*
Modify:
  03.08.2000 SVS
    + Учтем, что можут быть указан параметр -P в командной строке...
  01.08.2000 SVS
    ! Расширение дополнительного пути для поиска персональных плагином
      происходит непосредственно перед поиском
    + Исключаем фичу при совпадении двух путей поиска
  23.07.2000 SVS
    + Функции
       - Ввод тестовой строки
       - FSF-функция KeyToName
       - FSF: работа с буфером обмена CopyToClipboard, PasteFromClipboard
  23.07.2000 SVS
    + Функции для обработчика диалога
       - расширенная функция диалога FarDialogEx;
       - обмен сообщениями SendDlgMessage;
       - функция по умолчанию DefDlgProc;
  15.07.2000 SVS
    + Добавка в виде задания дополнительного пути для поиска плагинов
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  13.07.2000 IS
    - Пофикшен трап при входе в редактор (PluginsSet::ProcessEditorInput)
      Решения подсказал tran.
    - Исправлен глюк в PluginsSet::SavePluginSettings, допущенный при
      неправильном переводе под VC: переменная I изменялась во всех циклах
      (внимательнее над быть, вашу мать $%#...)
      Решения подсказал tran.
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  07.07.2000 IS
    + Инициализация: atoi, _atoi64, itoa, RemoveLeadingSpaces,
      RemoveTrailingSpaces, RemoveExternalSpaces, TruncStr, TruncPathStr,
      QuoteSpaceOnly, PointToName, GetPathRoot, AddEndSlash
  06.07.2000 IS
    + Объявление структуры типа FarStandardFunctions (см. plugin.hpp)
      Инициализация ее членов:
      StructSize, Unquote, ExpandEnvironmentStr, sprintf, sscanf, qsort,
      memcpy, memmove, memcmp, strchr, strrchr, strstr, strtok, memset, strpbrk
  05.07.2000 IS
    + Функция AdvControl
  01.07.2000 IS
    + Функция вывода помощи в api
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */

static void CheckScreenLock();

// требуется в plugapi.cpp (было раньше static)
int KeepUserScreen;
char DirToSet[NM];

static int _cdecl PluginsSort(const void *el1,const void *el2);

PluginsSet::PluginsSet()
{
  PluginsData=NULL;
  PluginsCount=0;
  CurEditor=NULL;
}


PluginsSet::~PluginsSet()
{
  for (int I=0;I<PluginsCount;I++)
  {
    if (PluginsData[I].Cached)
      continue;
    FreeLibrary(PluginsData[I].hModule);
    PluginsData[I].Lang.Close();
  }
  /* $ 13.07.2000 SVS
    Ни кто не запрашивал память через new
  */
  free(PluginsData);
  /* SVS $ */
}


void PluginsSet::SendExit()
{
  for (int I=0;I<PluginsCount;I++)
    if (!PluginsData[I].Cached && PluginsData[I].pExitFAR)
      PluginsData[I].pExitFAR();
}


void PluginsSet::LoadPlugins()
{
  /* $ 15.07.2000 SVS
     Плагины ищутся сначала в персональном каталоге, а потом в "системном"
     для того, чтобы перекрыть "системные" своими... :-)
  */
  int IPath;
  char PluginsDir[NM],FullName[NM];
  WIN32_FIND_DATA FindData;
  ScanTree ScTree(FALSE,TRUE);
  for(IPath=0; IPath < 2; ++IPath)
  {
    /* $ 03.08.2000 SVS
       Учтем, что можут быть указан параметр -P в командной строке
    */
    if(Opt.MainPluginDir)
      sprintf(PluginsDir,"%s%s",FarPath,PluginsFolderName);
    else
      strcpy(PluginsDir,MainPluginsPath);
    /* SVS $ */

    if(!IPath)
    {
      // если пусто то прерываем поиск :-) независимо ни от чего...
      if(Opt.PersonalPluginsPath[0])
      {
        /* $ 01.08.2000 SVS
           Вот здесь и расширяем значение пути!!!
        */
        ExpandEnvironmentStr(Opt.PersonalPluginsPath,FullName,sizeof(FullName));
        // проверка на вшивость!
        if(stricmp(PluginsDir,FullName))
          strcpy(PluginsDir,FullName);
        else
          continue; // продолжем дальше
        /* SVS $ */
      }
      else
        continue; // продолжем дальше
    }
    /* $ 03.08.2000 SVS
       Может быть случай, когда вообще без плагинов запускаемся!!!
    */
    if(!PluginsDir[0])
      continue;
    /* SVS $ */

    ScTree.SetFindPath(PluginsDir,"*.*");
    while (ScTree.GetNextName(&FindData,FullName))
      if (CmpName("*.dll",FindData.cFileName,FALSE) && (FindData.dwFileAttributes & FA_DIREC)==0)
      {
        struct PluginItem CurPlugin;
        memset(&CurPlugin,0,sizeof(CurPlugin));
        strcpy(CurPlugin.ModuleName,FullName);
        int CachePos=GetCacheNumber(FullName,&FindData,0);
        int LoadCached=(CachePos!=-1);
        if (LoadCached)
        {
          char RegKey[100];
          sprintf(RegKey,"PluginsCache\\Plugin%d\\Exports",CachePos);
          CurPlugin.pOpenPlugin=(PLUGINOPENPLUGIN)GetRegKey(RegKey,"OpenPlugin",0);
          CurPlugin.pOpenFilePlugin=(PLUGINOPENFILEPLUGIN)GetRegKey(RegKey,"OpenFilePlugin",0);
          CurPlugin.pSetFindList=(PLUGINSETFINDLIST)GetRegKey(RegKey,"SetFindList",0);
          CurPlugin.pProcessEditorInput=(PLUGINPROCESSEDITORINPUT)GetRegKey(RegKey,"ProcessEditorInput",0);
          CurPlugin.pProcessEditorEvent=(PLUGINPROCESSEDITOREVENT)GetRegKey(RegKey,"ProcessEditorEvent",0);
          CurPlugin.CachePos=CachePos;
        }
        if (LoadCached || LoadPlugin(CurPlugin,-1,TRUE))
        {
          struct PluginItem *NewPluginsData=(struct PluginItem *)realloc(PluginsData,sizeof(*PluginsData)*(PluginsCount+1));
          if (NewPluginsData==NULL)
            break;
          PluginsData=NewPluginsData;
          CurPlugin.Cached=LoadCached;
          CurPlugin.FindData=FindData;
          PluginsData[PluginsCount]=CurPlugin;
          PluginsCount++;
        }
      }
  }
  qsort(PluginsData,PluginsCount,sizeof(*PluginsData),PluginsSort);

  int NewPlugin=FALSE;

  for (int I=0;I<PluginsCount;I++)
    if (!PluginsData[I].Cached)
    {
      SetPluginStartupInfo(PluginsData[I],I);
      if (SavePluginSettings(PluginsData[I],PluginsData[I].FindData))
        NewPlugin=TRUE;
    }

  if (NewPlugin)
    for (int I=0;;I++)
    {
      char RegKey[100],PluginName[NM];
      sprintf(RegKey,"PluginsCache\\Plugin%d",I);
      GetRegKey(RegKey,"Name",PluginName,"",sizeof(PluginName));
      if (*PluginName==0)
        break;
      if (GetFileAttributes(PluginName)==0xFFFFFFFF)
      {
        DeleteKeyRecord("PluginsCache\\Plugin%d",I);
        I--;
      }
    }
}


int _cdecl PluginsSort(const void *el1,const void *el2)
{
  struct PluginItem *Plugin1=(struct PluginItem *)el1;
  struct PluginItem *Plugin2=(struct PluginItem *)el2;
  return(LocalStricmp(PointToName(Plugin1->ModuleName),PointToName(Plugin2->ModuleName)));
}


int PluginsSet::LoadPlugin(struct PluginItem &CurPlugin,int ModuleNumber,int Init)
{
  HMODULE hModule=CurPlugin.hModule ? CurPlugin.hModule:LoadLibraryEx(CurPlugin.ModuleName,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
  if (hModule==NULL)
    return(FALSE);
  CurPlugin.hModule=hModule;
  CurPlugin.Cached=FALSE;
  CurPlugin.pSetStartupInfo=(PLUGINSETSTARTUPINFO)GetProcAddress(hModule,"SetStartupInfo");
  CurPlugin.pOpenPlugin=(PLUGINOPENPLUGIN)GetProcAddress(hModule,"OpenPlugin");
  CurPlugin.pOpenFilePlugin=(PLUGINOPENFILEPLUGIN)GetProcAddress(hModule,"OpenFilePlugin");
  CurPlugin.pClosePlugin=(PLUGINCLOSEPLUGIN)GetProcAddress(hModule,"ClosePlugin");
  CurPlugin.pGetPluginInfo=(PLUGINGETPLUGININFO)GetProcAddress(hModule,"GetPluginInfo");
  CurPlugin.pGetOpenPluginInfo=(PLUGINGETOPENPLUGININFO)GetProcAddress(hModule,"GetOpenPluginInfo");
  CurPlugin.pGetFindData=(PLUGINGETFINDDATA)GetProcAddress(hModule,"GetFindData");
  CurPlugin.pFreeFindData=(PLUGINFREEFINDDATA)GetProcAddress(hModule,"FreeFindData");
  CurPlugin.pGetVirtualFindData=(PLUGINGETVIRTUALFINDDATA)GetProcAddress(hModule,"GetVirtualFindData");
  CurPlugin.pFreeVirtualFindData=(PLUGINFREEVIRTUALFINDDATA)GetProcAddress(hModule,"FreeVirtualFindData");
  CurPlugin.pSetDirectory=(PLUGINSETDIRECTORY)GetProcAddress(hModule,"SetDirectory");
  CurPlugin.pGetFiles=(PLUGINGETFILES)GetProcAddress(hModule,"GetFiles");
  CurPlugin.pPutFiles=(PLUGINPUTFILES)GetProcAddress(hModule,"PutFiles");
  CurPlugin.pDeleteFiles=(PLUGINDELETEFILES)GetProcAddress(hModule,"DeleteFiles");
  CurPlugin.pMakeDirectory=(PLUGINMAKEDIRECTORY)GetProcAddress(hModule,"MakeDirectory");
  CurPlugin.pProcessHostFile=(PLUGINPROCESSHOSTFILE)GetProcAddress(hModule,"ProcessHostFile");
  CurPlugin.pSetFindList=(PLUGINSETFINDLIST)GetProcAddress(hModule,"SetFindList");
  CurPlugin.pConfigure=(PLUGINCONFIGURE)GetProcAddress(hModule,"Configure");
  CurPlugin.pExitFAR=(PLUGINEXITFAR)GetProcAddress(hModule,"ExitFAR");
  CurPlugin.pProcessKey=(PLUGINPROCESSKEY)GetProcAddress(hModule,"ProcessKey");
  CurPlugin.pProcessEvent=(PLUGINPROCESSEVENT)GetProcAddress(hModule,"ProcessEvent");
  CurPlugin.pCompare=(PLUGINCOMPARE)GetProcAddress(hModule,"Compare");
  CurPlugin.pProcessEditorInput=(PLUGINPROCESSEDITORINPUT)GetProcAddress(hModule,"ProcessEditorInput");
  CurPlugin.pProcessEditorEvent=(PLUGINPROCESSEDITOREVENT)GetProcAddress(hModule,"ProcessEditorEvent");
  if (ModuleNumber!=-1 && Init)
    SetPluginStartupInfo(CurPlugin,ModuleNumber);
  return(TRUE);
}


void PluginsSet::SetPluginStartupInfo(struct PluginItem &CurPlugin,int ModuleNumber)
{
  if (CurPlugin.pSetStartupInfo!=NULL)
  {
    struct PluginStartupInfo StartupInfo;
    StartupInfo.StructSize=sizeof(StartupInfo);
    /* $ 06.07.2000 IS
      Объявление структуры типа FarStandardFunctions (см. plugin.hpp)
      Инициализация ее членов:
         StructSize, Unquote, ExpandEnvironmentStr,
         sprintf, sscanf, qsort, memcpy, memmove, memcmp, strchr,
         strrchr, strstr, strtok, memset, strpbrk
     $ 07.07.2000 IS
       Эпопея продолжается... Инициализация: atoi, _atoi64, itoa,
       RemoveLeadingSpaces, RemoveTrailingSpaces, RemoveExternalSpaces,
       TruncStr, TruncPathStr, QuoteSpaceOnly, PointToName, GetPathRoot,
       AddEndSlash
    */
    struct FarStandardFunctions StandardFunctions;
    StandardFunctions.StructSize=sizeof(StandardFunctions);
    StandardFunctions.Unquote=Unquote;
    StandardFunctions.ExpandEnvironmentStr=ExpandEnvironmentStr;
    StandardFunctions.sprintf=FarSprintf;
    StandardFunctions.sscanf=FarSscanf;
    StandardFunctions.qsort=FarQsort;
    StandardFunctions.memcpy=memcpy;
    StandardFunctions.memmove=memmove;
    StandardFunctions.memcmp=memcmp;
    StandardFunctions.strchr=strchr;
    StandardFunctions.strrchr=strrchr;
    StandardFunctions.strstr=strstr;
    StandardFunctions.strtok=strtok;
    StandardFunctions.memset=memset;
    StandardFunctions.strpbrk=strpbrk;
    StandardFunctions.atoi=FarAtoi;
    StandardFunctions._atoi64=FarAtoa64;
    StandardFunctions.itoa=FarItoa;
    StandardFunctions.RemoveLeadingSpaces=RemoveLeadingSpaces;
    StandardFunctions.RemoveTrailingSpaces=RemoveTrailingSpaces;
    StandardFunctions.RemoveExternalSpaces=RemoveExternalSpaces;
    StandardFunctions.TruncStr=TruncStr;
    StandardFunctions.TruncPathStr=TruncPathStr;
    StandardFunctions.QuoteSpaceOnly=QuoteSpaceOnly;
    StandardFunctions.PointToName=PointToName;
    StandardFunctions.GetPathRoot=GetPathRoot;
    StandardFunctions.AddEndSlash=AddEndSlash;
    /* IS $ */
    /* $ 25.07.2000 SVS
       Моя очередь продолжать эпопею :-)
    */
    StandardFunctions.CopyToClipboard=CopyToClipboard;
    StandardFunctions.PasteFromClipboard=PasteFromClipboard;
    StandardFunctions.FarKeyToText=KeyToText;
    /* SVS $ */

    strcpy(StartupInfo.ModuleName,CurPlugin.ModuleName);
    StartupInfo.ModuleNumber=ModuleNumber;
    strcpy(CurPlugin.RootKey,Opt.RegRoot);
    strcat(CurPlugin.RootKey,"\\Plugins");
    StartupInfo.RootKey=CurPlugin.RootKey;
    StartupInfo.Menu=FarMenuFn;
    StartupInfo.Dialog=FarDialogFn;
    StartupInfo.GetMsg=FarGetMsgFn;
    StartupInfo.Message=FarMessageFn;
    StartupInfo.Control=FarControl;
    StartupInfo.SaveScreen=FarSaveScreen;
    StartupInfo.RestoreScreen=FarRestoreScreen;
    StartupInfo.GetDirList=FarGetDirList;
    StartupInfo.GetPluginDirList=FarGetPluginDirList;
    StartupInfo.FreeDirList=FarFreeDirList;
    StartupInfo.Viewer=FarViewer;
    StartupInfo.Editor=FarEditor;
    StartupInfo.CmpName=FarCmpName;
    StartupInfo.CharTable=FarCharTable;
    StartupInfo.Text=FarText;
    StartupInfo.EditorControl=FarEditorControl;
    /* 01.07.2000 IS
       Функция вывода помощи
    */
    StartupInfo.ShowHelp=FarShowHelp;
    /* IS $ */
    /* 05.07.2000 IS
       Функция, которая будет действовать и в редакторе, и в панелях, и...
    */
    StartupInfo.AdvControl=FarAdvControl;
    /* IS $ */
    /* $ 23.07.2000 SVS
       Функции для обработчика диалога
         - расширенная функция диалога
         - обмен сообщениями
         - функция по умолчанию
    */
    StartupInfo.DialogEx=FarDialogEx;
    StartupInfo.SendDlgMessage=FarSendDlgMessage;
    StartupInfo.DefDlgProc=FarDefDlgProc;
    /* $ 25.07.2000 SVS
       Функция-стандартный диалог ввода текста
    */
    StartupInfo.InputBox=GetString;
    /* SVS $ */
    /* 06.07.2000 IS
      Указатель на структуру с адресами полезных функций из far.exe
      Плагин должен обязательно скопировать ее себе, если хочет использовать.
    */
    StartupInfo.FSF=&StandardFunctions;
    /* IS $ */
    CurPlugin.pSetStartupInfo(&StartupInfo);
  }
}


int PluginsSet::PreparePlugin(int PluginNumber)
{
  if (!PluginsData[PluginNumber].Cached)
    return(TRUE);
  return(LoadPlugin(PluginsData[PluginNumber],PluginNumber,TRUE));
}


int PluginsSet::GetCacheNumber(char *FullName,WIN32_FIND_DATA *FindData,int CachePos)
{
  for (int I=-1;;I++)
  {
    if (I==-1 && CachePos==0)
      continue;
    int Pos=(I==-1) ? CachePos:I;

    char RegKey[100],PluginName[NM],PluginID[100],CurPluginID[100];
    sprintf(RegKey,"PluginsCache\\Plugin%d",Pos);
    GetRegKey(RegKey,"Name",PluginName,"",sizeof(PluginName));
    if (*PluginName==0)
      break;
    if (LocalStricmp(PluginName,FullName)!=0)
      continue;
    if (FindData!=NULL)
    {
      GetRegKey(RegKey,"ID",PluginID,"",sizeof(PluginID));
      sprintf(CurPluginID,"%x%x%x",FindData->nFileSizeLow,
              FindData->ftCreationTime.dwLowDateTime,
              FindData->ftLastWriteTime.dwLowDateTime);
      if (strcmp(PluginID,CurPluginID)!=0)
        continue;
    }
    return(Pos);
  }
  return(-1);
}


int PluginsSet::SavePluginSettings(struct PluginItem &CurPlugin,
                                    WIN32_FIND_DATA &FindData)
{
  if (CurPlugin.pGetPluginInfo==NULL)
    return(FALSE);
  struct PluginInfo Info;
  memset(&Info,0,sizeof(Info));
  CurPlugin.pGetPluginInfo(&Info);
  if (Info.Flags & PF_PRELOAD)
    return(FALSE);
  /* $ 13.07.2000 IS
    Исправлен глюк, допущенный при неправильном переводе под VC:
    переменная I изменялась во всех циклах (внимательнее над быть,
    вашу мать $%#...)
    (подсказал tran)
  */
  int I,I0;
  for (I0=0;;I0++)
  {
    char RegKey[100],PluginName[NM],CurPluginID[100];
    sprintf(RegKey,"PluginsCache\\Plugin%d",I0);
    GetRegKey(RegKey,"Name",PluginName,"",sizeof(PluginName));
    if (*PluginName==0 || LocalStricmp(PluginName,CurPlugin.ModuleName)==0)
    {
      DeleteKeyTree(RegKey);

      SetRegKey(RegKey,"Name",CurPlugin.ModuleName);
      sprintf(CurPluginID,"%x%x%x",FindData.nFileSizeLow,
              FindData.ftCreationTime.dwLowDateTime,
              FindData.ftLastWriteTime.dwLowDateTime);
      SetRegKey(RegKey,"ID",CurPluginID);
      for (I=0;I<Info.DiskMenuStringsNumber;I++)
      {
        char Value[100];
        sprintf(Value,"DiskMenuString%d",I);
        SetRegKey(RegKey,Value,Info.DiskMenuStrings[I]);
        if (Info.DiskMenuNumbers)
        {
          sprintf(Value,"DiskMenuNumber%d",I);
          SetRegKey(RegKey,Value,Info.DiskMenuNumbers[I]);
        }
      }
      for (I=0;I<Info.PluginMenuStringsNumber;I++)
      {
        char Value[100];
        sprintf(Value,"PluginMenuString%d",I);
        SetRegKey(RegKey,Value,Info.PluginMenuStrings[I]);
      }
      for (I=0;I<Info.PluginConfigStringsNumber;I++)
      {
        char Value[100];
        sprintf(Value,"PluginConfigString%d",I);
        SetRegKey(RegKey,Value,Info.PluginConfigStrings[I]);
      }
      SetRegKey(RegKey,"CommandPrefix",NullToEmpty(Info.CommandPrefix));
      SetRegKey(RegKey,"Flags",Info.Flags);
      sprintf(RegKey,"PluginsCache\\Plugin%d\\Exports",I0);
      SetRegKey(RegKey,"OpenPlugin",CurPlugin.pOpenPlugin!=NULL);
      SetRegKey(RegKey,"OpenFilePlugin",CurPlugin.pOpenFilePlugin!=NULL);
      SetRegKey(RegKey,"SetFindList",CurPlugin.pSetFindList!=NULL);
      SetRegKey(RegKey,"ProcessEditorInput",CurPlugin.pProcessEditorInput!=NULL);
      SetRegKey(RegKey,"ProcessEditorEvent",CurPlugin.pProcessEditorEvent!=NULL);
      break;
    }
  }
  /* IS $ */
  return(TRUE);
}


HANDLE PluginsSet::OpenPlugin(int PluginNumber,int OpenFrom,int Item)
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  CheckScreenLock();
  char CurDir[NM];
  CtrlObject->CmdLine.GetCurDir(CurDir);
  chdir(CurDir);
  *DirToSet=0;
  if (PluginNumber<PluginsCount && PluginsData[PluginNumber].pOpenPlugin)
    if (PreparePlugin(PluginNumber))
    {
      HANDLE hInternal=PluginsData[PluginNumber].pOpenPlugin(OpenFrom,Item);
      if (hInternal!=INVALID_HANDLE_VALUE)
      {
        PluginHandle *hPlugin=new PluginHandle;
        hPlugin->InternalHandle=hInternal;
        hPlugin->PluginNumber=PluginNumber;
        return((HANDLE)hPlugin);
      }
      else
        if (*DirToSet)
        {
          CtrlObject->ActivePanel->SetCurDir(DirToSet,TRUE);
          CtrlObject->ActivePanel->Redraw();
        }
    }
  return(INVALID_HANDLE_VALUE);
}


HANDLE PluginsSet::OpenFilePlugin(char *Name,const unsigned char *Data,int DataSize)
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  for (int I=0;I<PluginsCount;I++)
    if (PluginsData[I].pOpenFilePlugin && PreparePlugin(I))
    {
      char FullName[NM],*NamePtr=NULL;
      if (Name!=NULL)
      {
        ConvertNameToFull(Name,FullName);
        NamePtr=FullName;
      }
      HANDLE hInternal=PluginsData[I].pOpenFilePlugin(NamePtr,Data,DataSize);
      if (hInternal==(HANDLE)-2)
        return((HANDLE)-2);
      if (hInternal!=INVALID_HANDLE_VALUE)
      {
        PluginHandle *hPlugin=new PluginHandle;
        hPlugin->InternalHandle=hInternal;
        hPlugin->PluginNumber=I;
        return((HANDLE)hPlugin);
      }
    }
  return(INVALID_HANDLE_VALUE);
}


HANDLE PluginsSet::OpenFindListPlugin(PluginPanelItem *PanelItem,int ItemsNumber)
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  for (int I=0;I<PluginsCount;I++)
    if (PluginsData[I].pOpenPlugin && PluginsData[I].pSetFindList &&
        PreparePlugin(I))
    {
      HANDLE hInternal=PluginsData[I].pOpenPlugin(OPEN_FINDLIST,0);
      if (hInternal!=INVALID_HANDLE_VALUE)
      {
        if (!PluginsData[I].pSetFindList(hInternal,PanelItem,ItemsNumber))
          continue;
        PluginHandle *hPlugin=new PluginHandle;
        hPlugin->InternalHandle=hInternal;
        hPlugin->PluginNumber=I;
        return((HANDLE)hPlugin);
      }
    }
  return(INVALID_HANDLE_VALUE);
}


void PluginsSet::ClosePlugin(HANDLE hPlugin)
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pClosePlugin)
    PluginsData[ph->PluginNumber].pClosePlugin(ph->InternalHandle);
  delete ph;
}


int PluginsSet::ProcessEditorInput(INPUT_RECORD *Rec)
{
  for (int I=0;I<PluginsCount;I++)
    if (PluginsData[I].pProcessEditorInput && PreparePlugin(I))
    /* $ 13.07.2000 IS
       Фиксит трап при входе в редактор (подсказал tran)
    */
      if (PluginsData[I].pProcessEditorInput &&
         PluginsData[I].pProcessEditorInput(Rec))
    /* IS $ */
        return(TRUE);
  return(FALSE);
}


void PluginsSet::ProcessEditorEvent(int Event,void *Param)
{
  for (int I=0;I<PluginsCount;I++)
    if (PluginsData[I].pProcessEditorEvent && PreparePlugin(I))
      PluginsData[I].pProcessEditorEvent(Event,Param);
}


int PluginsSet::GetFindData(HANDLE hPlugin,PluginPanelItem **pPanelData,int *pItemsNumber,int OpMode)
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  *pItemsNumber=0;
  if (PluginsData[ph->PluginNumber].pGetFindData)
    return(PluginsData[ph->PluginNumber].pGetFindData(ph->InternalHandle,pPanelData,pItemsNumber,OpMode));
  return(FALSE);
}


void PluginsSet::FreeFindData(HANDLE hPlugin,PluginPanelItem *PanelItem,
                              int ItemsNumber)
{
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pFreeFindData)
    PluginsData[ph->PluginNumber].pFreeFindData(ph->InternalHandle,PanelItem,ItemsNumber);
}


int PluginsSet::GetVirtualFindData(HANDLE hPlugin,PluginPanelItem **pPanelData,
                                   int *pItemsNumber,char *Path)
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  *pItemsNumber=0;
  if (PluginsData[ph->PluginNumber].pGetVirtualFindData)
    return(PluginsData[ph->PluginNumber].pGetVirtualFindData(ph->InternalHandle,pPanelData,pItemsNumber,Path));
  return(FALSE);
}


void PluginsSet::FreeVirtualFindData(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber)
{
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pFreeVirtualFindData)
    PluginsData[ph->PluginNumber].pFreeVirtualFindData(ph->InternalHandle,PanelItem,ItemsNumber);
}


int PluginsSet::SetDirectory(HANDLE hPlugin,char *Dir,int OpMode)
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pSetDirectory)
    return(PluginsData[ph->PluginNumber].pSetDirectory(ph->InternalHandle,Dir,OpMode));
  return(FALSE);
}


int PluginsSet::GetFile(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                        char *DestPath,char *ResultName,int OpMode)
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  SaveScreen *SaveScr=NULL;
  if ((OpMode & OPM_FIND)==0)
    SaveScr=new SaveScreen;
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  int Found=FALSE;
  KeepUserScreen=FALSE;
  if (PluginsData[ph->PluginNumber].pGetFiles)
  {
    int GetCode=PluginsData[ph->PluginNumber].pGetFiles(ph->InternalHandle,PanelItem,1,0,DestPath,OpMode);
    char FindPath[NM];
    strcpy(FindPath,DestPath);
    AddEndSlash(FindPath);
    strcat(FindPath,"*.*");
    HANDLE FindHandle;
    WIN32_FIND_DATA fdata;
    if ((FindHandle=FindFirstFile(FindPath,&fdata))!=INVALID_HANDLE_VALUE)
    {
      int Done=0;
      while (!Done)
      {
        if ((fdata.dwFileAttributes & FA_DIREC)==0)
          break;
        Done=!FindNextFile(FindHandle,&fdata);
      }
      FindClose(FindHandle);
      if (!Done)
      {
        strcpy(ResultName,DestPath);
        AddEndSlash(ResultName);
        strcat(ResultName,fdata.cFileName);
        if (GetCode!=1)
        {
          SetFileAttributes(ResultName,0);
          remove(ResultName);
        }
        else
          Found=TRUE;
      }
    }
  }

  ReadUserBackgound(SaveScr);

  delete SaveScr;
  return(Found);
}


int PluginsSet::DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                            int ItemsNumber,int OpMode)
{
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pDeleteFiles)
  {
    ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
    SaveScreen SaveScr;
    KeepUserScreen=FALSE;
    int Code=PluginsData[ph->PluginNumber].pDeleteFiles(ph->InternalHandle,PanelItem,ItemsNumber,OpMode);
    ReadUserBackgound(&SaveScr);
    return(Code);
  }
  return(FALSE);
}


int PluginsSet::MakeDirectory(HANDLE hPlugin,char *Name,int OpMode)
{
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pMakeDirectory)
  {
    ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
    SaveScreen SaveScr;
    KeepUserScreen=FALSE;
    int Code=PluginsData[ph->PluginNumber].pMakeDirectory(ph->InternalHandle,Name,OpMode);
    ReadUserBackgound(&SaveScr);
    return(Code);
  }
  return(-1);
}


int PluginsSet::ProcessHostFile(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pProcessHostFile)
  {
    ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
    SaveScreen SaveScr;
    KeepUserScreen=FALSE;
    int Code=PluginsData[ph->PluginNumber].pProcessHostFile(ph->InternalHandle,PanelItem,ItemsNumber,OpMode);
    ReadUserBackgound(&SaveScr);
    return(Code);
  }
  return(FALSE);
}


int PluginsSet::GetFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                int ItemsNumber,int Move,char *DestPath,int OpMode)
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  int ExitCode=FALSE;
  {
    struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
    if (PluginsData[ph->PluginNumber].pGetFiles)
    {
      SaveScreen SaveScr;
      KeepUserScreen=FALSE;
      ExitCode=PluginsData[ph->PluginNumber].pGetFiles(ph->InternalHandle,PanelItem,ItemsNumber,Move,DestPath,OpMode);
      ReadUserBackgound(&SaveScr);
    }
  }
  return(ExitCode);
}


int PluginsSet::PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode)
{
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pPutFiles)
  {
    ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
    SaveScreen SaveScr;
    KeepUserScreen=FALSE;
    int Code=PluginsData[ph->PluginNumber].pPutFiles(ph->InternalHandle,PanelItem,ItemsNumber,Move,OpMode);
    ReadUserBackgound(&SaveScr);
    return(Code);
  }
  return(FALSE);
}


int PluginsSet::GetPluginInfo(int PluginNumber,struct PluginInfo *Info)
{
  if (PluginNumber>=PluginsCount)
    return(FALSE);
  memset(Info,0,sizeof(*Info));
  if (PluginsData[PluginNumber].pGetPluginInfo)
    PluginsData[PluginNumber].pGetPluginInfo(Info);
  return(TRUE);
}


void PluginsSet::GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
  memset(Info,0,sizeof(*Info));
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pGetOpenPluginInfo)
    PluginsData[ph->PluginNumber].pGetOpenPluginInfo(ph->InternalHandle,Info);
  if (Info->CurDir==NULL)
    Info->CurDir="";
}


int PluginsSet::ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pProcessKey)
    return(PluginsData[ph->PluginNumber].pProcessKey(ph->InternalHandle,Key,ControlState));
  return(FALSE);
}


int PluginsSet::ProcessEvent(HANDLE hPlugin,int Event,void *Param)
{
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pProcessEvent)
    return(PluginsData[ph->PluginNumber].pProcessEvent(ph->InternalHandle,Event,Param));
  return(FALSE);
}


int PluginsSet::Compare(HANDLE hPlugin,struct PluginPanelItem *Item1,struct PluginPanelItem *Item2,unsigned int Mode)
{
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pCompare)
    return(PluginsData[ph->PluginNumber].pCompare(ph->InternalHandle,Item1,Item2,Mode));
  return(-3);
}


void PluginsSet::Configure()
{
  int MenuItemNumber=0;
  int I, J;
  VMenu PluginList(MSG(MPluginConfigTitle),NULL,0,ScrY-4);
  PluginList.SetFlags(MENU_WRAPMODE);
  PluginList.SetPosition(-1,-1,0,0);

  LoadIfCacheAbsent();

  for (I=0;I<PluginsCount;I++)
  {
    if (PluginsData[I].Cached)
    {
      char RegKey[100],Value[100];
      int RegNumber=GetCacheNumber(PluginsData[I].ModuleName,NULL,PluginsData[I].CachePos);
      if (RegNumber==-1)
        continue;
      else
        for (J=0;;J++)
        {
          struct MenuItem ListItem;
          ListItem.Checked=ListItem.Separator=0;
          ListItem.UserData[0]=I;
          ListItem.UserData[1]=J;
          ListItem.UserDataSize=2;
          sprintf(RegKey,"PluginsCache\\Plugin%d",RegNumber);
          sprintf(Value,"PluginConfigString%d",J);
          GetRegKey(RegKey,Value,ListItem.Name,"",sizeof(ListItem.Name));
          if (*ListItem.Name==0)
            break;
          ListItem.Selected=(MenuItemNumber++ == 0);
          PluginList.AddItem(&ListItem);
        }
    }
    else
    {
      struct PluginInfo Info;
      if (!GetPluginInfo(I,&Info))
        continue;
      for (J=0;J<Info.PluginConfigStringsNumber;J++)
      {
        struct MenuItem ListItem;
        ListItem.Checked=ListItem.Separator=0;
        ListItem.UserData[0]=I;
        ListItem.UserData[1]=J;
        ListItem.UserDataSize=2;
        strcpy(ListItem.Name,NullToEmpty(Info.PluginConfigStrings[J]));
        ListItem.Selected=(MenuItemNumber++ == 0);
        PluginList.AddItem(&ListItem);
      }
    }
  }
  PluginList.AssignHighlights(FALSE);
  PluginList.Process();
  int ExitCode=PluginList.GetExitCode();
  PluginList.Hide();
  if (ExitCode<0)
    return;
  unsigned char Data[2];
  PluginList.GetUserData(Data,2,ExitCode);
  int PNum=Data[0];
  if (PreparePlugin(PNum) && PluginsData[PNum].pConfigure!=NULL)
  {
    if (PluginsData[PNum].pConfigure(Data[1]))
    {
      if (CtrlObject->LeftPanel->GetMode()==PLUGIN_PANEL)
      {
        CtrlObject->LeftPanel->Update(UPDATE_KEEP_SELECTION);
        CtrlObject->LeftPanel->Redraw();
      }
      if (CtrlObject->RightPanel->GetMode()==PLUGIN_PANEL)
      {
        CtrlObject->RightPanel->Update(UPDATE_KEEP_SELECTION);
        CtrlObject->RightPanel->Redraw();
      }
    }
    SavePluginSettings(PluginsData[PNum],PluginsData[PNum].FindData);
  }
}


int PluginsSet::CommandsMenu(int Editor,int Viewer,int StartPos)
{
  int MenuItemNumber=0;
  VMenu PluginList(MSG(MPluginCommandsMenuTitle),NULL,0,ScrY-4);
  PluginList.SetFlags(MENU_WRAPMODE);
  PluginList.SetPosition(-1,-1,0,0);
  PluginList.SetHelp("Plugins");

  LoadIfCacheAbsent();

  char FirstHotKey[512];
  int HotKeysPresent=EnumRegKey("PluginHotkeys",0,FirstHotKey,sizeof(FirstHotKey));

  for (int I=0;I<PluginsCount;I++)
  {
    char HotRegKey[512],HotKey[100];
    *HotKey=0;
    if (PluginsData[I].Cached)
    {
      char RegKey[100],Value[100];
      int RegNumber=GetCacheNumber(PluginsData[I].ModuleName,NULL,PluginsData[I].CachePos);
      if (RegNumber==-1)
        continue;
      else
      {
        sprintf(RegKey,"PluginsCache\\Plugin%d",RegNumber);
        int Flags=GetRegKey(RegKey,"Flags",0);
        if (Editor && (Flags & PF_EDITOR)==0 ||
            Viewer && (Flags & PF_VIEWER)==0 ||
            !Editor && !Viewer && (Flags & PF_DISABLEPANELS))
          continue;
        for (int J=0;;J++)
        {
          *HotKey=0;
          if (GetHotKeyRegKey(I,J,HotRegKey))
            GetRegKey(HotRegKey,"Hotkey",HotKey,"",sizeof(HotKey));
          struct MenuItem ListItem;
          ListItem.Checked=ListItem.Separator=0;
          ListItem.UserData[0]=I;
          ListItem.UserData[1]=J;
          ListItem.UserDataSize=2;
          sprintf(Value,"PluginMenuString%d",J);
          char Name[sizeof(ListItem.Name)];
          GetRegKey(RegKey,Value,Name,"",sizeof(Name));
          if (*Name==0)
            break;
          if (!HotKeysPresent)
            strcpy(ListItem.Name,Name);
          else
            if (*HotKey)
              sprintf(ListItem.Name,"&%c  %s",*HotKey,Name);
            else
              sprintf(ListItem.Name,"   %s",Name);
          ListItem.Selected=(MenuItemNumber++ == StartPos);
          PluginList.AddItem(&ListItem);
        }
      }
    }
    else
    {
      struct PluginInfo Info;
      if (!GetPluginInfo(I,&Info))
        continue;
      if (Editor && (Info.Flags & PF_EDITOR)==0 ||
          Viewer && (Info.Flags & PF_VIEWER)==0 ||
          !Editor && !Viewer && (Info.Flags & PF_DISABLEPANELS))
        continue;
      for (int J=0;J<Info.PluginMenuStringsNumber;J++)
      {
        *HotKey=0;
        if (GetHotKeyRegKey(I,J,HotRegKey))
          GetRegKey(HotRegKey,"Hotkey",HotKey,"",sizeof(HotKey));
        struct MenuItem ListItem;
        ListItem.Checked=ListItem.Separator=0;
        ListItem.UserData[0]=I;
        ListItem.UserData[1]=J;
        ListItem.UserDataSize=2;
        char Name[sizeof(ListItem.Name)];
        strncpy(Name,NullToEmpty(Info.PluginMenuStrings[J]),sizeof(Name));
        if (!HotKeysPresent)
          strcpy(ListItem.Name,Name);
        else
          if (*HotKey)
            sprintf(ListItem.Name,"&%c  %s",*HotKey,Name);
          else
            sprintf(ListItem.Name,"   %s",Name);
        ListItem.Selected=(MenuItemNumber++ == StartPos);
        PluginList.AddItem(&ListItem);
      }
    }
  }
  PluginList.AssignHighlights(FALSE);
  PluginList.SetBottomTitle(MSG(MPluginHotKeyBottom));

  PluginList.Show();

  while (!PluginList.Done())
  {
    int SelPos=PluginList.GetSelectPos();
    unsigned char Data[2];
    char RegKey[512];

    PluginList.GetUserData(Data,2,SelPos);
    switch(PluginList.ReadInput())
    {
      case KEY_F4:
        if (SelPos<MenuItemNumber && GetHotKeyRegKey(Data[0],Data[1],RegKey))
        {
          int ExitCode;
          static struct DialogData PluginDlgData[]=
          {
            DI_DOUBLEBOX,3,1,60,4,0,0,0,0,(char *)MPluginHotKeyTitle,
            DI_TEXT,5,2,0,0,0,0,0,0,(char *)MPluginHotKey,
            DI_FIXEDIT,5,3,5,3,1,0,0,1,""
          };
          MakeDialogItems(PluginDlgData,PluginDlg);

          {
            GetRegKey(RegKey,"Hotkey",PluginDlg[2].Data,"",sizeof(PluginDlg[2].Data));
            Dialog Dlg(PluginDlg,sizeof(PluginDlg)/sizeof(PluginDlg[0]));
            Dlg.SetPosition(-1,-1,64,6);
            Dlg.Process();
            ExitCode=Dlg.GetExitCode();
          }
          if (ExitCode==2)
          {
            PluginDlg[2].Data[1]=0;
            if (*PluginDlg[2].Data==0 || *PluginDlg[2].Data==' ')
              DeleteRegKey(RegKey);
            else
              SetRegKey(RegKey,"Hotkey",PluginDlg[2].Data);
            PluginList.Hide();
            return(CommandsMenu(Editor,Viewer,SelPos));
          }
        }
        break;
      default:
        PluginList.ProcessInput();
        break;
    }
  }
  int ExitCode=PluginList.GetExitCode();

  PluginList.Hide();
  if (ExitCode<0)
    return(FALSE);
  ScrBuf.Flush();
  unsigned char Data[2];
  PluginList.GetUserData(Data,2,ExitCode);
  if (PreparePlugin(Data[0]) && PluginsData[Data[0]].pOpenPlugin!=NULL)
  {
    Panel *ActivePanel=CtrlObject->ActivePanel;
    if (ActivePanel->ProcessPluginEvent(FE_CLOSE,NULL))
      return(FALSE);
    int OpenCode=OPEN_PLUGINSMENU;
    if (Editor)
      OpenCode=OPEN_EDITOR;
    if (Viewer)
      OpenCode=OPEN_VIEWER;
    HANDLE hPlugin=OpenPlugin(Data[0],OpenCode,Data[1]);
    if (hPlugin!=INVALID_HANDLE_VALUE && !Editor && !Viewer)
    {
      Panel *NewPanel=CtrlObject->ChangePanel(ActivePanel,FILE_PANEL,TRUE,TRUE);
      NewPanel->SetPluginMode(hPlugin,"");
      NewPanel->Update(0);
      NewPanel->Show();
      NewPanel->SetFocus();
    }
  }
  return(TRUE);
}


int PluginsSet::GetHotKeyRegKey(int PluginNumber,int ItemNumber,char *RegKey)
{
  int FarPathLength=strlen(FarPath);
  *RegKey=0;
  if (FarPathLength<strlen(PluginsData[PluginNumber].ModuleName))
  {
    char PluginName[NM];
    strcpy(PluginName,PluginsData[PluginNumber].ModuleName+FarPathLength);
    for (int I=0;PluginName[I]!=0;I++)
      if (PluginName[I]=='\\')
        PluginName[I]='/';
    if (ItemNumber>0)
      sprintf(PluginName+strlen(PluginName),"%%%d",ItemNumber);
    sprintf(RegKey,"PluginHotkeys\\%s",PluginName);
    return(TRUE);
  }
  return(FALSE);
}


int PluginsSet::GetDiskMenuItem(int PluginNumber,int PluginItem,
                int &ItemPresent,int &PluginTextNumber,char *PluginText)
{
  if (PluginNumber>=PluginsCount)
    return(FALSE);

  LoadIfCacheAbsent();

  if (PluginsData[PluginNumber].Cached)
  {
    char RegKey[100],Value[100];
    int RegNumber=GetCacheNumber(PluginsData[PluginNumber].ModuleName,NULL,PluginsData[PluginNumber].CachePos);
    if (RegNumber==-1)
      ItemPresent=0;
    else
    {
      sprintf(RegKey,"PluginsCache\\Plugin%d",RegNumber);
      sprintf(Value,"DiskMenuString%d",PluginItem);
      GetRegKey(RegKey,Value,PluginText,"",100);
      sprintf(Value,"DiskMenuNumber%d",PluginItem);
      GetRegKey(RegKey,Value,PluginTextNumber,0);
      ItemPresent=*PluginText!=0;
    }
    return(TRUE);
  }
  struct PluginInfo Info;
  if (!GetPluginInfo(PluginNumber,&Info) ||
      Info.DiskMenuStringsNumber<=PluginItem)
    ItemPresent=FALSE;
  else
  {
    if (Info.DiskMenuNumbers)
      PluginTextNumber=Info.DiskMenuNumbers[PluginItem];
    else
      PluginTextNumber=0;
    strcpy(PluginText,Info.DiskMenuStrings[PluginItem]);
    ItemPresent=TRUE;
  }
  return(TRUE);
}


int PluginsSet::UseFarCommand(HANDLE hPlugin,int CommandType)
{
  struct OpenPluginInfo Info;
  GetOpenPluginInfo(hPlugin,&Info);
  if ((Info.Flags & OPIF_REALNAMES)==0)
    return(FALSE);
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  switch(CommandType)
  {
    case PLUGIN_FARGETFILE:
    case PLUGIN_FARGETFILES:
      return(PluginsData[ph->PluginNumber].pGetFiles==NULL || (Info.Flags & OPIF_EXTERNALGET));
    case PLUGIN_FARPUTFILES:
      return(PluginsData[ph->PluginNumber].pPutFiles==NULL || (Info.Flags & OPIF_EXTERNALPUT));
    case PLUGIN_FARDELETEFILES:
      return(PluginsData[ph->PluginNumber].pDeleteFiles==NULL || (Info.Flags & OPIF_EXTERNALDELETE));
    case PLUGIN_FARMAKEDIRECTORY:
      return(PluginsData[ph->PluginNumber].pMakeDirectory==NULL || (Info.Flags & OPIF_EXTERNALMKDIR));
  }
  return(TRUE);
}


void PluginsSet::ReloadLanguage()
{
  for (int I=0;I<PluginsCount;I++)
    PluginsData[I].Lang.Close();
  DiscardCache();
}


void PluginsSet::DiscardCache()
{
  for (int I=0;I<PluginsCount;I++)
    PreparePlugin(I);
  DeleteKeyTree("PluginsCache");
}


void PluginsSet::LoadIfCacheAbsent()
{
  if (!CheckRegKey("PluginsCache"))
    for (int I=0;I<PluginsCount;I++)
      PreparePlugin(I);
}


int PluginsSet::ProcessCommandLine(char *Command)
{
  int PrefixLength=0;
  while (1)
  {
    int Ch=Command[PrefixLength];
    if (Ch==0 || Ch==' ' || Ch=='\t' || Ch=='/' || PrefixLength>64)
      return(FALSE);
    if (Ch==':' && PrefixLength>0)
      break;
    PrefixLength++;
  }

  LoadIfCacheAbsent();

  char Prefix[256];
  strncpy(Prefix,Command,PrefixLength);
  Prefix[PrefixLength]=0;

  int PluginPos=-1;

  for (int I=0;I<PluginsCount;I++)
    if (PluginsData[I].Cached)
    {
      int RegNumber=GetCacheNumber(PluginsData[I].ModuleName,NULL,PluginsData[I].CachePos);
      if (RegNumber!=-1)
      {
        char RegKey[100],PluginPrefix[512];
        sprintf(RegKey,"PluginsCache\\Plugin%d",RegNumber);
        GetRegKey(RegKey,"CommandPrefix",PluginPrefix,"",sizeof(PluginPrefix));
        if (LocalStricmp(Prefix,PluginPrefix)==0)
        {
          PluginPos=I;
          break;
        }
      }
    }
    else
    {
      struct PluginInfo Info;
      if (GetPluginInfo(I,&Info))
        if (LocalStricmp(Prefix,NullToEmpty(Info.CommandPrefix))==0)
        {
          PluginPos=I;
          break;
        }
    }
  if (PluginPos==-1)
    return(FALSE);
  if (!PreparePlugin(PluginPos) || PluginsData[PluginPos].pOpenPlugin==NULL)
    return(FALSE);
  Panel *ActivePanel=CtrlObject->ActivePanel;
  if (ActivePanel->ProcessPluginEvent(FE_CLOSE,NULL))
    return(FALSE);
  CtrlObject->CmdLine.SetString("");

  char PluginCommand[512];
  strcpy(PluginCommand,Command+PrefixLength+1);
  HANDLE hPlugin=OpenPlugin(PluginPos,OPEN_COMMANDLINE,(int)PluginCommand);
  if (hPlugin!=INVALID_HANDLE_VALUE)
  {
    Panel *NewPanel=CtrlObject->ChangePanel(ActivePanel,FILE_PANEL,TRUE,TRUE);
    NewPanel->SetPluginMode(hPlugin,"");
    NewPanel->Update(0);
    NewPanel->Show();
    NewPanel->SetFocus();
  }
  return(TRUE);
}


void PluginsSet::ReadUserBackgound(SaveScreen *SaveScr)
{
  CtrlObject->LeftPanel->ProcessingPluginCommand++;
  CtrlObject->RightPanel->ProcessingPluginCommand++;
  if (KeepUserScreen)
  {
    SaveScr->Discard();
    RedrawDesktop Redraw;
  }
  CtrlObject->LeftPanel->ProcessingPluginCommand--;
  CtrlObject->RightPanel->ProcessingPluginCommand--;
}


void CheckScreenLock()
{
  if (ScrBuf.GetLockCount()>0 && !CtrlObject->Macro.PeekKey())
  {
    ScrBuf.SetLockCount(0);
    ScrBuf.Flush();
  }
}
