/*
plugins.cpp

Работа с плагинами (низкий уровень, кое-что повыше в flplugin.cpp)

*/

/* Revision: 1.50 25.01.2001 $ */

/*
Modify:
  25.01.2001 SVS
    ! попытка локализовать причину исключения при неверно заполненных полях
      структур OpenPluginInfo и PluginInfo
  23.01.2001 SVS
    + DumpExeptionInfo(xp); - запись информации об исключении в дамп.
  23.01.2001 skv
    + Добавил EXCEPTION_BREAKPOINT в список известных
    + Unknown Exception на не известные.
  29.12.2000 IS
    ! При настройке "параметров внешних модулей" закрывать окно с их
      списком только при нажатии на ESC
  19.12.2000 IS
    + Shift-F9 в списке плагинов вызывает настройки редактора/программы
      просмотра, если этот список был вызван соответственно из
      редактора/программы просмотра. Эта возможность нужна для макросов, да
      и просто удобно.
  18.12.2000 SVS
    + Shift-F1 в списке плагинов вызывает хелп по данному плагину
  07.12.2000 SVS
    + Проверка не только версии, но и номера билда в функции проверки версии.
  27.11.2000 SVS
    ! Введение кнопки Debug в диалоги исключений с последующим вызовом
      системного дебагера.
  02.11.2000 OT
    ! Введение проверки на длину буфера, отведенного под имя файла.
  31.10.2000 SVS
    + Функция TestOpenPluginInfo - проверка на вшивость переданных
      плагином данных
    ! Уточнения в эксепшинах
  26.10.2000 SVS
    - ошибки с "int Ret;" :-)
  23.10.2000 SVS
    + Функция TestPluginInfo - проверка на вшивость переданных плагином данных
  19.10.2000 tran
    + /co & PF_PRELOAD = friendship forever
      теперь верно :)
  17.10.2000 SVS
    + Везде, в экспортируемых функция введена спарка try-__except
  16.10.2000 SVS
    + Обработка исключений при вызове галимого плагина (пока только при вызове
      двух функций - OpenPlugin и OpenFilePlugin).
  12.10.2000 tran
    + /co & PF_PRELOAD = friendship.
    + PluginsSet::DumpPluginsInfo(), call by AltF11 in plugins menu
  12.10.2000 IS
    + Указатель на ProcessName в StandardFunctions
  27.09.2000 SVS
    + Указатель на текущий Viewer
    + ProcessViewerEvent
    + CallPlugin
  27.09.2000 skv
    + DeleteBuffer
  24.09.2000 SVS
    + Функция FarNameToKey - получение кода клавиши по имени
      Если имя не верно или нет такого - возвращается -1
  21.09.2000 SVS
    + Работа с  PluginItem.SysID - системный идентификатор плагина
  20.09.2000 SVS
    ! удалил FolderPresent (блин, совсем крышу сорвало :-(
  19.09.2000 SVS
    + функция FolderPresent - "сужествует ли каталог"
  18.09.2000 SVS
    ! PluginsSet::SetPluginStartupInfo - заполним стркутуру PluginStartupInfo
      нулями.
    ! FarRecurseSearch -> FarRecursiveSearch
  14.09.2000 SVS
    + FSF.MkTemp
  10.09.2000 IS 1.21
    - Забыли проверку Info.CommandPrefix на NULL сделать, соответственно фар
      иногда с конвульсиями помирал, теперь - нет.
  10.09.2000 tran 1.21
    + FSF/FarRecurseSearch
  10.09.2000 SVS
    ! Наконец-то нашлось приемлемое имя для QWERTY -> Xlat.
  08.09.2000 SVS
    ! QWERTY -> Transliterate
  07.09.2000 SVS 1.20
    - MultiPrefix
      По каким-то непонятным причинам из кэше для Flags возвращалось
      значение равное 0 (хотя вижу что в реестре стоит 0x10) :-(
  07.09.2000 VVM 1.19
    + Несколько префиксов у плагина, разделенных через ":"
    + Если флаг PF_FULLCMDLINE - отдавать с префиксом
  07.09.2000 SVS
    + Функция GetFileOwner тоже доступна плагинам :-)
    + Функция GetNumberOfLinks тоже доступна плагинам :-)
    + Оболочка FarBsearch для плагинов (функция bsearch)
  05.09.2000 SVS 1.17
    + QWERTY - перекодировщик - StandardFunctions.EDQwerty
  01.09.2000 tran 1.16
    + PluginsSet::LoadPluginsFromCache()
  31.08.2000 tran
    + FSF/FarInputRecordTokey
  31.08.2000 SVS
    ! изменение FSF-функций
      FSF.RemoveLeadingSpaces =FSF.LTrim
      FSF.RemoveTrailingSpaces=FSF.RTrim
      FSF.RemoveExternalSpaces=FSF.Trim
  28.08.2000 SVS
    + Добавка для Local*
    ! не FarStandardFunctions._atoi64, но FarStandardFunctions.atoi64
    + FARSTDITOA64
  25.08.2000 SVS
    ! Удалены из FSF функции:
      memset, memcpy, memmove, memcmp, strchr, strrchr, strstr, strtok, strpbrk
  03.08.2000 tran 1.12
    + GetMinFarVersion export
      для определения минимально-неодходимой версии фара.
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

#ifdef _MSC_VER
#pragma warning(disable:4509)
#endif

static void CheckScreenLock();
static char FmtPluginsCache_PluginD[]="PluginsCache\\Plugin%d";
static char FmtDiskMenuStringD[]="DiskMenuString%d";
static char FmtDiskMenuNumberD[]="DiskMenuNumber%d";

// требуется в plugapi.cpp (было раньше static)
int KeepUserScreen;
char DirToSet[NM];

static int _cdecl PluginsSort(const void *el1,const void *el2);


/* $ 16.10.2000 SVS
   Простенький обработчик исключений.
*/
static char* xFromMSGTitle(int From)
{
  if(From == EXCEPT_SETSTARTUPINFO || From == EXCEPT_MINFARVERSION)
    return MSG(MExceptTitleLoad);
  else
    return MSG(MExceptTitle);
}

static int xfilter(
    int From,                 // откуда: 0 = OpenPlugin, 1 = OpenFilePlugin
    EXCEPTION_POINTERS *xp,   // данные ситуации
    struct PluginItem *Module,// модуль, приведший к исключению.
    DWORD Flags)              // дополнительные флаги - пока только один
                              //        0x1 - спрашивать про выгрузку?
{
   struct __ECODE {
     DWORD Code;     // код исключения
     DWORD IdMsg;    // ID сообщения из LNG-файла
     DWORD RetCode;  // Что вернем?
   } ECode[]={
     {EXCEPTION_ACCESS_VIOLATION, MExcRAccess, EXCEPTION_EXECUTE_HANDLER},
     {EXCEPTION_ARRAY_BOUNDS_EXCEEDED, MExcOutOfBounds, EXCEPTION_EXECUTE_HANDLER},
     {EXCEPTION_INT_DIVIDE_BY_ZERO,MExcDivideByZero, EXCEPTION_EXECUTE_HANDLER},
     {EXCEPTION_STACK_OVERFLOW,MExcStackOverflow, EXCEPTION_EXECUTE_HANDLER},
     {EXCEPTION_BREAKPOINT,MExcBreakPoint, EXCEPTION_EXECUTE_HANDLER},
     // сюды добавляем.
   };
   // EXCEPTION_CONTINUE_EXECUTION  ??????
   char *Ptr;
   int I;
   int rc, Ret=1;
   char Buf[2][64];
   char TruncFileName[2*NM];

   // получим запись исключения
   EXCEPTION_RECORD *xr = xp->ExceptionRecord;

   // CONTEXT можно использовать для отображения или записи в лог
   //         содержимого регистров...
   // CONTEXT *xc = xp->ContextRecord;

   rc = EXCEPTION_EXECUTE_HANDLER;

   /*$ 23.01.2001 skv
     Неизвестное исключение не стоит игнорировать.
   */
   Ptr=NULL;
   strcpy(TruncFileName,NullToEmpty(Module->ModuleName));
   if(From != EXCEPT_GETPLUGININFO_DATA && From != EXCEPT_GETOPENPLUGININFO_DATA)
   {
     // просмотрим "знакомые" FAR`у исключения и обработаем...
     for(I=0; I < sizeof(ECode)/sizeof(ECode[0]); ++I)
       if(ECode[I].Code == xr->ExceptionCode)
       {
         Ptr=MSG(ECode[I].IdMsg);
         rc=ECode[I].RetCode;
         if(xr->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
         {
           sprintf(Buf[1],MSG(xr->ExceptionInformation[0]+MExcRAccess),xr->ExceptionInformation[1]);
           Ptr=Buf[1];
         }
         break;
       }

     if(!Ptr)Ptr=MSG(MExcUnknown);

     sprintf(Buf[0],MSG(MExcAddress),xr->ExceptionAddress);
     if(Flags&1)
     {
       Ret=Message(MSG_WARNING,(Opt.ExceptRules?3:2),
               xFromMSGTitle(From),
               MSG(MExcTrappedException),
               Ptr,
               Buf[0],
               TruncPathStr(TruncFileName,40),"\1",
               MSG(MExcUnload),
               (Opt.ExceptRules?MSG(MExcDebugger):MSG(MYes)),
               (Opt.ExceptRules?MSG(MYes):MSG(MNo)),
               (Opt.ExceptRules?MSG(MNo):NULL));
       if(Opt.ExceptRules && Ret == 1 || !Opt.ExceptRules && !Ret)
         CtrlObject->Plugins.UnloadPlugin(*Module);
     }
     else
       Ret=Message(MSG_WARNING,(Opt.ExceptRules?2:1),
               xFromMSGTitle(From),
               MSG(MExcTrappedException),
               Ptr,
               Buf[0],
               TruncPathStr(TruncFileName,40),"\1",
               MSG(MExcUnloadYes),
               (Opt.ExceptRules?MSG(MExcDebugger):MSG(MOk)),
               (Opt.ExceptRules?MSG(MOk):NULL));
     /* skv$*/
   }
   else // однозначно выгружаем эту бяку :-(
   {
     char OutBuf[128];
     char *PtrNameStruct=NULL;
     int NStructFrom=0;
     static const char *NameField[2][3]={
       {"DiskMenuStrings","PluginMenuStrings","PluginConfigStrings"},
       {"InfoLines","DescrFiles","PanelModesArray"},
     };
     switch(From)
     {
       case EXCEPT_GETPLUGININFO_DATA:
         PtrNameStruct="PluginInfo";
         NStructFrom=0;
         break;
       case EXCEPT_GETOPENPLUGININFO_DATA:
         PtrNameStruct="OpenPluginInfo";
         NStructFrom=1;
         break;
     }

     if(xr->ExceptionCode >= STATUS_STRUCTWRONGFILLED &&
        xr->ExceptionCode <= STATUS_STRUCTWRONGFILLED+2)
     {
       sprintf(OutBuf,
           MSG(MExcStructField),
           PtrNameStruct,
           NameField[NStructFrom][xr->ExceptionCode-STATUS_STRUCTWRONGFILLED]);
     }
     else
       sprintf(OutBuf,MSG(MExcStructWrongFilled),PtrNameStruct);

     Ret=Message(MSG_WARNING,
            (Opt.ExceptRules?2:1),
            xFromMSGTitle(From),
            MSG(MExcTrappedException),
            MSG(MExcCheckOnLousys),
            TruncPathStr(TruncFileName,40),
            OutBuf,
            "\1",
            MSG(MExcUnloadYes),
            (Opt.ExceptRules?MSG(MExcDebugger):MSG(MOk)),
            (Opt.ExceptRules?MSG(MOk):NULL));

     if(!Opt.ExceptRules || Ret == 1)
       CtrlObject->Plugins.UnloadPlugin(*Module);

     if(!Opt.ExceptRules)
       // не забудем про продолжение исполнения
       rc=EXCEPTION_EXECUTE_HANDLER;
   }

   // Вот здесь есть подозрение - нужно ли вообще это EXCEPTION_CONTINUE_SEARCH?
   if(Opt.ExceptRules && !Ret)
   {
     rc = EXCEPTION_CONTINUE_SEARCH;
   }

   DumpExeptionInfo(xp);
   if(xr->ExceptionFlags&EXCEPTION_NONCONTINUABLE)
     rc=EXCEPTION_CONTINUE_SEARCH; //?
   return rc;
}
/* SVS $ */


PluginsSet::PluginsSet()
{
  PluginsData=NULL;
  PluginsCount=0;
  CurEditor=NULL;
  CurViewer=NULL;
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
    {
      //EXCEPTION_POINTERS *xp;

      TRY{
        PluginsData[I].pExitFAR();
      }
      __except ( xfilter(EXCEPT_EXITFAR,
                     //xp = GetExceptionInformation(),&PluginsData[I],1) )
                     GetExceptionInformation(),&PluginsData[I],1) )
      {
        ;
      }
    }
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

  /* $ 01.09.2000 tran
     '/co' switch */
  if ( Opt.PluginsCacheOnly )
  {
      LoadPluginsFromCache();
      return ;
  }
  /* tran $ */
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
        char RegKey[100];
        memset(&CurPlugin,0,sizeof(CurPlugin));
        strcpy(CurPlugin.ModuleName,FullName);
        int CachePos=GetCacheNumber(FullName,&FindData,0);
        int LoadCached=(CachePos!=-1);
        /* $ 12.10.2000 tran
           Preload=1 нужно для корректной обработки -co */
        sprintf(RegKey,"PluginsCache\\Plugin%d",CachePos);
        if ( GetRegKey(RegKey,"Preload",0)==1 )
        {
          LoadCached=0;
          CachePos=-1;
        }
        /* tran $ */
        if (LoadCached)
        {
          char RegKey[100];
          sprintf(RegKey,"PluginsCache\\Plugin%d\\Exports",CachePos);
          CurPlugin.SysID=GetRegKey(RegKey,"SysID",0);
          CurPlugin.pOpenPlugin=(PLUGINOPENPLUGIN)GetRegKey(RegKey,"OpenPlugin",0);
          CurPlugin.pOpenFilePlugin=(PLUGINOPENFILEPLUGIN)GetRegKey(RegKey,"OpenFilePlugin",0);
          CurPlugin.pSetFindList=(PLUGINSETFINDLIST)GetRegKey(RegKey,"SetFindList",0);
          CurPlugin.pProcessEditorInput=(PLUGINPROCESSEDITORINPUT)GetRegKey(RegKey,"ProcessEditorInput",0);
          CurPlugin.pProcessEditorEvent=(PLUGINPROCESSEDITOREVENT)GetRegKey(RegKey,"ProcessEditorEvent",0);
          CurPlugin.pProcessViewerEvent=(PLUGINPROCESSVIEWEREVENT)GetRegKey(RegKey,"ProcessViewerEvent",0);
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
      sprintf(RegKey,FmtPluginsCache_PluginD,I);
      GetRegKey(RegKey,"Name",PluginName,"",sizeof(PluginName));
      if (*PluginName==0)
        break;
      if (GetFileAttributes(PluginName)==0xFFFFFFFF)
      {
        DeleteKeyRecord(FmtPluginsCache_PluginD,I);
        I--;
      }
    }
}

/* $ 01.09.2000 tran
   Load cache only plugins  - '/co' switch */
void PluginsSet::LoadPluginsFromCache()
{
   /*
    [HKEY_CURRENT_USER\Software\Far\PluginsCache\Plugin0]
    "Name"="C:\\PROGRAM FILES\\FAR\\Plugins\\ABOOK\\AddrBook.dll"
    "ID"="e400a14def00a37ea900"
    "DiskMenuString0"="Address Book"
    "PluginMenuString0"="Address Book"
    "PluginConfigString0"="Address Book"
    "PluginConfigString1"="Address: E-Mail"
    "PluginConfigString2"="Address: Birthday"
    "PluginConfigString3"="Address: Phone number"
    "PluginConfigString4"="Address: Fidonet"
    "CommandPrefix"=""
    "Flags"=dword:00000000

    [HKEY_CURRENT_USER\Software\Far\PluginsCache\Plugin0\Exports]
    "OpenPlugin"=dword:00000001
    "OpenFilePlugin"=dword:00000000
    "SetFindList"=dword:00000000
    "ProcessEditorInput"=dword:00000000
    "ProcessEditorEvent"=dword:00000000
  */
    int I;
    char PlgKey[512];
    char RegKey[100];
    struct PluginItem CurPlugin;

    for (I=0;;I++)
    {
        if (!EnumRegKey("PluginsCache",I,PlgKey,sizeof(PlgKey)))
        break;

        memset(&CurPlugin,0,sizeof(CurPlugin));
                            //  012345678901234567890
        strcpy(RegKey,PlgKey); // "PLuginsCache\PluginXX"
        GetRegKey(RegKey,"Name",CurPlugin.ModuleName,"",NM);
        /* $ 12.10.2000 tran
          -co должен понимать PRELOAD плагины */
        if ( GetRegKey(RegKey,"Preload",0)==1 )
        {

          if (!LoadPlugin(CurPlugin,-1,TRUE))
            continue; // загрузка не удалась
          CurPlugin.Cached=FALSE;
        }
        else
        {
        /* tran $ */
          strcat(RegKey,"\\");
          strcat(RegKey,"Exports");
          CurPlugin.SysID=GetRegKey(RegKey,"SysID",0);
          CurPlugin.pOpenPlugin=(PLUGINOPENPLUGIN)GetRegKey(RegKey,"OpenPlugin",0);
          CurPlugin.pOpenFilePlugin=(PLUGINOPENFILEPLUGIN)GetRegKey(RegKey,"OpenFilePlugin",0);
          CurPlugin.pSetFindList=(PLUGINSETFINDLIST)GetRegKey(RegKey,"SetFindList",0);
          CurPlugin.pProcessEditorInput=(PLUGINPROCESSEDITORINPUT)GetRegKey(RegKey,"ProcessEditorInput",0);
          CurPlugin.pProcessEditorEvent=(PLUGINPROCESSEDITOREVENT)GetRegKey(RegKey,"ProcessEditorEvent",0);
          CurPlugin.pProcessViewerEvent=(PLUGINPROCESSVIEWEREVENT)GetRegKey(RegKey,"ProcessViewerEvent",0);
          CurPlugin.CachePos=atoi(PlgKey+19);
          CurPlugin.Cached=TRUE;
          // вот тут это поле не заполнено, надеюсь, что оно не критично
          // CurPlugin.FindData=FindData;
        }
        struct PluginItem *NewPluginsData=(struct PluginItem *)realloc(PluginsData,sizeof(*PluginsData)*(PluginsCount+1));
        if (NewPluginsData==NULL)
            break;
        PluginsData=NewPluginsData;
        PluginsData[PluginsCount]=CurPlugin;
        PluginsCount++;
    }
    qsort(PluginsData,PluginsCount,sizeof(*PluginsData),PluginsSort);
    /* $ 19.10.2000 tran
       забыл вызвать SetStartupInfo :) */
    for (I=0;I<PluginsCount;I++)
    {
        if (!PluginsData[I].Cached)
        {
            SetPluginStartupInfo(PluginsData[I],I);
        }
    }
    /* tran $ */
}
/* tran $ */

int _cdecl PluginsSort(const void *el1,const void *el2)
{
  struct PluginItem *Plugin1=(struct PluginItem *)el1;
  struct PluginItem *Plugin2=(struct PluginItem *)el2;
  return(LocalStricmp(PointToName(Plugin1->ModuleName),PointToName(Plugin2->ModuleName)));
}


int PluginsSet::LoadPlugin(struct PluginItem &CurPlugin,int ModuleNumber,int Init)
{
  HMODULE hModule=CurPlugin.hModule ? CurPlugin.hModule:LoadLibraryEx(CurPlugin.ModuleName,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
  if ( CurPlugin.DontLoadAgain )
  {
    return (FALSE);
  }
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
  CurPlugin.pProcessViewerEvent=(PLUGINPROCESSVIEWEREVENT)GetProcAddress(hModule,"ProcessViewerEvent");
  CurPlugin.pMinFarVersion=(PLUGINMINFARVERSION)GetProcAddress(hModule,"GetMinFarVersion");
  if (ModuleNumber!=-1 && Init)
  {
    /* $ 03.08.2000 tran
       проверка на минимальную версию фара */
    if ( CheckMinVersion(CurPlugin) )
    {
      SetPluginStartupInfo(CurPlugin,ModuleNumber);
    }
    else
    {
        UnloadPlugin(CurPlugin); // тест не пройден, выгружаем его
        return (FALSE);
    }
  }
  return(TRUE);
}

/* $ 03.08.2000 tran
   функция проверки минимальной версии */
/* $ 07.12.2000 SVS
   Проверка не только версии, но и номера билда
*/
int  PluginsSet::CheckMinVersion(struct PluginItem &CurPlugin)
{
    if ( CurPlugin.pMinFarVersion==0 ) // плагин не эскпортирует, ему или неважно, или он для <1.65
    {
        return (TRUE);
    }
    DWORD v;

    //EXCEPTION_POINTERS *xp;
    TRY {
      v=(DWORD)CurPlugin.pMinFarVersion();
    }
    __except ( xfilter(EXCEPT_MINFARVERSION,
                     GetExceptionInformation(),&CurPlugin,0) )
    {
       UnloadPlugin(CurPlugin); // тест не пройден, выгружаем его
       return (FALSE);
    }

    if (LOWORD(v) > LOWORD(FAR_VERSION) ||
        (LOWORD(v) == LOWORD(FAR_VERSION) &&
         HIWORD(v) > HIWORD(FAR_VERSION)
        )
    ) // кранты - плагин требует старший фар
    {
        ShowMessageAboutIllegialPluginVersion(CurPlugin.ModuleName,v);
        return (FALSE);
    }
    return (TRUE); // нормально, свой парень
}

// выгрузка плагина
// причем без всяких ему объяснений
void PluginsSet::UnloadPlugin(struct PluginItem &CurPlugin)
{
    FreeLibrary(CurPlugin.hModule);
    memset(&CurPlugin,0,sizeof(CurPlugin));
    CurPlugin.DontLoadAgain=1;
}

void PluginsSet::ShowMessageAboutIllegialPluginVersion(char* plg,int required)
{
    char msg[512];
    char PlgName[NM];
    strcpy(PlgName,plg);
    TruncPathStr(PlgName,ScrX-20);
    sprintf(msg,MSG(MPlgRequired),
           HIBYTE(LOWORD(required)),LOBYTE(LOWORD(required)),HIWORD(required),
           HIBYTE(LOWORD(FAR_VERSION)),LOBYTE(LOWORD(FAR_VERSION)),HIWORD(FAR_VERSION));
    Message(MSG_WARNING,1,MSG(MError),MSG(MPlgBadVers),PlgName,msg,MSG(MOk));
}
/* tran 03.08.2000 $ */
/* SVS $ */

void PluginsSet::SetPluginStartupInfo(struct PluginItem &CurPlugin,int ModuleNumber)
{
  if (CurPlugin.pSetStartupInfo!=NULL)
  {
    struct PluginStartupInfo StartupInfo;
    /* $ 18.09.2000 SVS
      Заполним _сразу_ всю структуру нулями.
    */
    memset(&StartupInfo,0,sizeof(struct PluginStartupInfo));
    /* SVS $ */
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
    memset(&StandardFunctions,0,sizeof(struct FarStandardFunctions));

    StandardFunctions.StructSize=sizeof(StandardFunctions);
    StandardFunctions.sprintf=FarSprintf;
    StandardFunctions.sscanf=FarSscanf;
    StandardFunctions.qsort=FarQsort;
    StandardFunctions.atoi=FarAtoi;
    StandardFunctions.atoi64=FarAtoi64;
    StandardFunctions.itoa=FarItoa;
    StandardFunctions.itoa64=FarItoa64;

    StandardFunctions.qsort=FarQsort;
    StandardFunctions.bsearch=FarBsearch;

    /* $ 28.08.2000 SVS
       + Функции работы с...
    */
    StandardFunctions.LIsLower   =LocalIslower;
    StandardFunctions.LIsUpper   =LocalIsupper;
    StandardFunctions.LIsAlpha   =LocalIsalpha;
    StandardFunctions.LIsAlphanum=LocalIsalphanum;
    StandardFunctions.LUpper     =LocalUpper;
    StandardFunctions.LUpperBuf  =LocalUpperBuf;
    StandardFunctions.LLowerBuf  =LocalLowerBuf;
    StandardFunctions.LLower     =LocalLower;
    StandardFunctions.LStrupr    =LocalStrupr;
    StandardFunctions.LStrlwr    =LocalStrlwr;
    StandardFunctions.LStricmp   =LStricmp;
    StandardFunctions.LStrnicmp  =LStrnicmp;
    /* SVS $ */

    StandardFunctions.Unquote=Unquote;
    StandardFunctions.ExpandEnvironmentStr=ExpandEnvironmentStr;
    StandardFunctions.LTrim=RemoveLeadingSpaces;
    StandardFunctions.RTrim=RemoveTrailingSpaces;
    StandardFunctions.Trim=RemoveExternalSpaces;
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
    StandardFunctions.FarKeyToName=KeyToText;
    /* SVS $ */
    /* $ 24.09.2000 SVS
     + Функция FarNameToKey - получение кода клавиши по имени
       Если имя не верно или нет такого - возвращается -1
    */
    StandardFunctions.FarNameToKey=KeyNameToKey;
    /* SVS $ */
    /* $ 31.08.2000 tran
       + InputRecordToKey*/
    StandardFunctions.FarInputRecordToKey=InputRecordToKey;
    /* tran 31.08.2000 $ */
    /* $ 05.09.2000 SVS 1.17
       + QWERTY - перекодировщик
    */
    StandardFunctions.XLat=Xlat;
    /* SVS $ */
    /* $ 07.09.2000 SVS 1.17
       + Функция GetFileOwner тоже доступна плагинам :-)
       + Функция GetNumberOfLinks тоже доступна плагинам :-)
    */
    StandardFunctions.GetFileOwner=GetFileOwner;
    StandardFunctions.GetNumberOfLinks=GetNumberOfLinks;
    /* SVS $ */
    /* $ 10.09.2000 tran
      + нижеуказанное */
    StandardFunctions.FarRecursiveSearch=FarRecursiveSearch;
    /* tran 08.09.2000 $ */
    /* $ 14.09.2000 SVS
      Функция получения временного файла с полным путем.
    */
    StandardFunctions.MkTemp=FarMkTemp;
    /* SVS $ */
    /*$ 27.09.2000 skv
      + Delete buffer allocated in PasteFromClipboard
    */
    StandardFunctions.DeleteBuffer=DeleteBuffer;
    /* skv$*/
    /* $ 12.10.2000 IS
      + ProcessName - обработать имя файла: сравнить с маской, масками,
        сгенерировать по маске
    */
    StandardFunctions.ProcessName=ProcessName;
    /* IS $ */

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
    StartupInfo.ViewerControl=FarViewerControl;
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
    //EXCEPTION_POINTERS *xp;
      TRY {
        CurPlugin.pSetStartupInfo(&StartupInfo);
      }
      __except ( xfilter(EXCEPT_SETSTARTUPINFO,
                         GetExceptionInformation(),&CurPlugin,0) )
      {
         UnloadPlugin(CurPlugin); // тест не пройден, выгружаем его
      }
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
    sprintf(RegKey,FmtPluginsCache_PluginD,Pos);
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
  //EXCEPTION_POINTERS *xp;
  TRY {
    CurPlugin.pGetPluginInfo(&Info);
  }
  __except ( xfilter(EXCEPT_GETPLUGININFO,
                     GetExceptionInformation(),&CurPlugin,0) )
  {
     UnloadPlugin(CurPlugin); // тест не пройден, выгружаем его
     return FALSE;
  }

  if(!TestPluginInfo(CurPlugin,&Info))
    return FALSE;

  CurPlugin.SysID=Info.SysID;
  /* $ 12.10.2000 tran
     при PF_PRELOAD в кеш будет записано, что плагин не кешируется
     так будет работать -co */
//  if (Info.Flags & PF_PRELOAD)
//    return(FALSE);
  /* tran $ */

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
    sprintf(RegKey,FmtPluginsCache_PluginD,I0);
    GetRegKey(RegKey,"Name",PluginName,"",sizeof(PluginName));
    if (*PluginName==0 || LocalStricmp(PluginName,CurPlugin.ModuleName)==0)
    {
      DeleteKeyTree(RegKey);

      SetRegKey(RegKey,"Name",CurPlugin.ModuleName);
      sprintf(CurPluginID,"%x%x%x",FindData.nFileSizeLow,
              FindData.ftCreationTime.dwLowDateTime,
              FindData.ftLastWriteTime.dwLowDateTime);
      SetRegKey(RegKey,"ID",CurPluginID);
      /* $ 12.10.2000 tran
         если плагин PRELOAD, в кеш пишется об этом */
      if (Info.Flags & PF_PRELOAD)
      {
        SetRegKey(RegKey,"Preload",1);
        break;
      }
      else
        SetRegKey(RegKey,"Preload",(DWORD)0);
      /* tran $ */

      for (I=0;I<Info.DiskMenuStringsNumber;I++)
      {
        char Value[100];
        sprintf(Value,FmtDiskMenuStringD,I);
        SetRegKey(RegKey,Value,Info.DiskMenuStrings[I]);
        if (Info.DiskMenuNumbers)
        {
          sprintf(Value,FmtDiskMenuNumberD,I);
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
      SetRegKey(RegKey,"SysID",CurPlugin.SysID);
      SetRegKey(RegKey,"OpenPlugin",CurPlugin.pOpenPlugin!=NULL);
      SetRegKey(RegKey,"OpenFilePlugin",CurPlugin.pOpenFilePlugin!=NULL);
      SetRegKey(RegKey,"SetFindList",CurPlugin.pSetFindList!=NULL);
      SetRegKey(RegKey,"ProcessEditorInput",CurPlugin.pProcessEditorInput!=NULL);
      SetRegKey(RegKey,"ProcessEditorEvent",CurPlugin.pProcessEditorEvent!=NULL);
      SetRegKey(RegKey,"ProcessViewerEvent",CurPlugin.pProcessViewerEvent!=NULL);
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
      /* $ 16.10.2000 SVS
         + Обработка исключений при вызове галимого плагина.
      */
      HANDLE hInternal;
      //EXCEPTION_POINTERS *xp;

      TRY {
         hInternal=PluginsData[PluginNumber].pOpenPlugin(OpenFrom,Item);
      }
      __except ( xfilter(EXCEPT_OPENPLUGIN,
                    GetExceptionInformation(),&PluginsData[PluginNumber],1) )  {
        hInternal=INVALID_HANDLE_VALUE;
      }
      /* SVS $ */
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
        ConvertNameToFull(Name,FullName, sizeof(FullName));
        NamePtr=FullName;
      }
      /* $ 16.10.2000 SVS
         + Обработка исключений при вызове галимого плагина.
      */
      HANDLE hInternal;
      //EXCEPTION_POINTERS *xp;

      TRY
      {
         hInternal=PluginsData[I].pOpenFilePlugin(NamePtr,Data,DataSize);
      }
      __except ( xfilter(EXCEPT_OPENFILEPLUGIN,
                   GetExceptionInformation(),&PluginsData[I],1) )  {
        hInternal=INVALID_HANDLE_VALUE;
      }
      /* SVS $ */

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
        //EXCEPTION_POINTERS *xp;
        BOOL Ret;

        TRY {
          Ret=PluginsData[I].pSetFindList(hInternal,PanelItem,ItemsNumber);
        }
        __except ( xfilter(EXCEPT_SETFINDLIST,
                         GetExceptionInformation(),&PluginsData[I],1) )
        {
           Ret=FALSE;
        }
        if (!Ret)
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
  {
    //EXCEPTION_POINTERS *xp;
    TRY {
      PluginsData[ph->PluginNumber].pClosePlugin(ph->InternalHandle);
    }
    __except ( xfilter(EXCEPT_CLOSEPLUGIN,
          GetExceptionInformation(),&PluginsData[ph->PluginNumber],1) )
    {
      ;
    }
  }
  delete ph;
}


int PluginsSet::ProcessEditorInput(INPUT_RECORD *Rec)
{
  //EXCEPTION_POINTERS *xp;
  for (int I=0;I<PluginsCount;I++)
    if (PluginsData[I].pProcessEditorInput && PreparePlugin(I))
      /* $ 13.07.2000 IS
         Фиксит трап при входе в редактор (подсказал tran)
      */
      if (PluginsData[I].pProcessEditorInput)
      {
        int Ret;

        TRY {
          Ret=PluginsData[I].pProcessEditorInput(Rec);
        }
        __except ( xfilter(EXCEPT_PROCESSEDITORINPUT,
                     GetExceptionInformation(),&PluginsData[I],1) )
        {
          Ret=FALSE;
        }
        if(Ret)
          return(TRUE);
        /* IS $ */
      }
  return(FALSE);
}


void PluginsSet::ProcessEditorEvent(int Event,void *Param)
{
  //EXCEPTION_POINTERS *xp;
  for (int I=0;I<PluginsCount;I++)
    if (PluginsData[I].pProcessEditorEvent && PreparePlugin(I))
    {
      int Ret=0;
      TRY {
        Ret=PluginsData[I].pProcessEditorEvent(Event,Param);
      }
      __except ( xfilter(EXCEPT_PROCESSEDITOREVENT,
                     GetExceptionInformation(),&PluginsData[I],1) )
      {
        ;
      }
    }
}


/* $ 27.09.2000 SVS
   События во вьювере
*/
void PluginsSet::ProcessViewerEvent(int Event,void *Param)
{
  //EXCEPTION_POINTERS *xp;
  for (int I=0;I<PluginsCount;I++)
    if (PluginsData[I].pProcessViewerEvent && PreparePlugin(I))
    {
      int Ret=0;
      TRY {
        Ret=PluginsData[I].pProcessViewerEvent(Event,Param);
      }
      __except ( xfilter(EXCEPT_PROCESSVIEWEREVENT,
                       GetExceptionInformation(),&PluginsData[I],1) )
      {
        ;
      }
    }

}
/* SVS $ */

int PluginsSet::GetFindData(HANDLE hPlugin,PluginPanelItem **pPanelData,int *pItemsNumber,int OpMode)
{
  //EXCEPTION_POINTERS *xp;
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  *pItemsNumber=0;
  if (PluginsData[ph->PluginNumber].pGetFindData)
  {
    int Ret;
    TRY {
      Ret=PluginsData[ph->PluginNumber].pGetFindData(ph->InternalHandle,pPanelData,pItemsNumber,OpMode);
    }
    __except ( xfilter(EXCEPT_GETFINDDATA,
                     GetExceptionInformation(),&PluginsData[ph->PluginNumber],1) )
    {
      Ret=FALSE;
    }
    return(Ret);
  }
  return(FALSE);
}


void PluginsSet::FreeFindData(HANDLE hPlugin,PluginPanelItem *PanelItem,
                              int ItemsNumber)
{
  //EXCEPTION_POINTERS *xp;
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pFreeFindData)
  {
    TRY {
      PluginsData[ph->PluginNumber].pFreeFindData(ph->InternalHandle,PanelItem,ItemsNumber);
    }
    __except ( xfilter(EXCEPT_FREEFINDDATA,
                     GetExceptionInformation(),&PluginsData[ph->PluginNumber],1) )
    {
      ;
    }
  }
}


int PluginsSet::GetVirtualFindData(HANDLE hPlugin,PluginPanelItem **pPanelData,
                                   int *pItemsNumber,char *Path)
{
  //EXCEPTION_POINTERS *xp;
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  *pItemsNumber=0;
  if (PluginsData[ph->PluginNumber].pGetVirtualFindData)
  {
    int Ret;
    TRY{
      Ret=PluginsData[ph->PluginNumber].pGetVirtualFindData(ph->InternalHandle,pPanelData,pItemsNumber,Path);
    }
    __except ( xfilter(EXCEPT_GETVIRTUALFINDDATA,
                     GetExceptionInformation(),&PluginsData[ph->PluginNumber],1) )
    {
      Ret=FALSE;
    }
    return(Ret);
  }
  return(FALSE);
}


void PluginsSet::FreeVirtualFindData(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber)
{
  //EXCEPTION_POINTERS *xp;
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pFreeVirtualFindData)
  {
    TRY {
      PluginsData[ph->PluginNumber].pFreeVirtualFindData(ph->InternalHandle,PanelItem,ItemsNumber);
    }
    __except ( xfilter(EXCEPT_FREEVIRTUALFINDDATA,
                     GetExceptionInformation(),&PluginsData[ph->PluginNumber],1) )
    {
      ;
    }
  }
}


int PluginsSet::SetDirectory(HANDLE hPlugin,char *Dir,int OpMode)
{
  //EXCEPTION_POINTERS *xp;
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pSetDirectory)
  {
    int Ret;
    TRY{
      Ret=PluginsData[ph->PluginNumber].pSetDirectory(ph->InternalHandle,Dir,OpMode);
    }
    __except ( xfilter(EXCEPT_SETDIRECTORY,
                     GetExceptionInformation(),&PluginsData[ph->PluginNumber],1) )
    {
      Ret=FALSE;
    }
    return(Ret);
  }
  return(FALSE);
}


int PluginsSet::GetFile(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                        char *DestPath,char *ResultName,int OpMode)
{
  //EXCEPTION_POINTERS *xp;
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  SaveScreen *SaveScr=NULL;
  if ((OpMode & OPM_FIND)==0)
    SaveScr=new SaveScreen;
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  int Found=FALSE;
  KeepUserScreen=FALSE;
  if (PluginsData[ph->PluginNumber].pGetFiles)
  {
    int GetCode;
    TRY{
      GetCode=PluginsData[ph->PluginNumber].pGetFiles(ph->InternalHandle,PanelItem,1,0,DestPath,OpMode);
    }
    __except ( xfilter(EXCEPT_GETFILES,
                     GetExceptionInformation(),&PluginsData[ph->PluginNumber],1))
    {
      // ??????????
      ReadUserBackgound(SaveScr);
      delete SaveScr;
      // ??????????
      return(Found);
    }
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
    int Code;
    //EXCEPTION_POINTERS *xp;
    TRY{
      Code=PluginsData[ph->PluginNumber].pDeleteFiles(ph->InternalHandle,PanelItem,ItemsNumber,OpMode);
    }
    __except ( xfilter(EXCEPT_DELETEFILES,
                     GetExceptionInformation(),&PluginsData[ph->PluginNumber],1) )
    {
      Code=FALSE;
    }
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
    int Code;
    //EXCEPTION_POINTERS *xp;
    TRY{
      Code=PluginsData[ph->PluginNumber].pMakeDirectory(ph->InternalHandle,Name,OpMode);
    }
    __except ( xfilter(EXCEPT_MAKEDIRECTORY,
                     GetExceptionInformation(),&PluginsData[ph->PluginNumber],1) )
    {
      Code=-1;
    }
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
    int Code;
    //EXCEPTION_POINTERS *xp;
    TRY{
      Code=PluginsData[ph->PluginNumber].pProcessHostFile(ph->InternalHandle,PanelItem,ItemsNumber,OpMode);
    }
    __except ( xfilter(EXCEPT_PROCESSHOSTFILE,
                     GetExceptionInformation(),&PluginsData[ph->PluginNumber],1) )
    {
      Code=FALSE;
    }
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
      //EXCEPTION_POINTERS *xp;
      TRY{
        ExitCode=PluginsData[ph->PluginNumber].pGetFiles(ph->InternalHandle,PanelItem,ItemsNumber,Move,DestPath,OpMode);
      }
      __except ( xfilter(EXCEPT_GETFILES,
                     GetExceptionInformation(),&PluginsData[ph->PluginNumber],1))
      {
        ExitCode=0;
      }
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
    int Code;
    //EXCEPTION_POINTERS *xp;
    TRY{
      Code=PluginsData[ph->PluginNumber].pPutFiles(ph->InternalHandle,PanelItem,ItemsNumber,Move,OpMode);
    }
    __except ( xfilter(EXCEPT_PUTFILES,
                      GetExceptionInformation(),&PluginsData[ph->PluginNumber],1) )
    {
      Code=0;
    }
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
  {
    //EXCEPTION_POINTERS *xp;
    TRY{
      PluginsData[PluginNumber].pGetPluginInfo(Info);
    }
    __except ( xfilter(EXCEPT_GETPLUGININFO,
                      GetExceptionInformation(),&PluginsData[PluginNumber],1) )
    {
      return FALSE;
    }
    if(!TestPluginInfo(PluginsData[PluginNumber],Info))
      return FALSE;
  }
  return(TRUE);
}


void PluginsSet::GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
  memset(Info,0,sizeof(*Info));
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pGetOpenPluginInfo)
  {
    //EXCEPTION_POINTERS *xp;
    TRY{
      PluginsData[ph->PluginNumber].pGetOpenPluginInfo(ph->InternalHandle,Info);
    }
    __except ( xfilter(EXCEPT_GETOPENPLUGININFO,
                      GetExceptionInformation(),&PluginsData[ph->PluginNumber],1) )
    {
      return;
    }
    if(!TestOpenPluginInfo(PluginsData[ph->PluginNumber],Info))
      return;
  }
  if (Info->CurDir==NULL)
    Info->CurDir="";
}


int PluginsSet::ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pProcessKey)
  {
    //EXCEPTION_POINTERS *xp;
    int Ret;
    TRY{
      Ret=PluginsData[ph->PluginNumber].pProcessKey(ph->InternalHandle,Key,ControlState);
    }
    __except ( xfilter(EXCEPT_PROCESSKEY,
                      GetExceptionInformation(),&PluginsData[ph->PluginNumber],1) )
    {
      Ret=FALSE;
    }
    return(Ret);
  }
  return(FALSE);
}


int PluginsSet::ProcessEvent(HANDLE hPlugin,int Event,void *Param)
{
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pProcessEvent)
  {
    //EXCEPTION_POINTERS *xp;
    int Ret;
    TRY{
      Ret=PluginsData[ph->PluginNumber].pProcessEvent(ph->InternalHandle,Event,Param);
    }
    __except ( xfilter(EXCEPT_PROCESSEVENT,
                      GetExceptionInformation(),&PluginsData[ph->PluginNumber],1) )
    {
      Ret=FALSE;
    }
    return(Ret);
  }
  return(FALSE);
}


int PluginsSet::Compare(HANDLE hPlugin,struct PluginPanelItem *Item1,struct PluginPanelItem *Item2,unsigned int Mode)
{
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  if (PluginsData[ph->PluginNumber].pCompare)
  {
    //EXCEPTION_POINTERS *xp;
    int Ret;
    TRY{
      Ret=PluginsData[ph->PluginNumber].pCompare(ph->InternalHandle,Item1,Item2,Mode);
    }
    __except ( xfilter(EXCEPT_COMPARE,
                      GetExceptionInformation(),&PluginsData[ph->PluginNumber],1) )
    {
      Ret=-3;
    }
    return(Ret);
  }
  return(-3);
}


void PluginsSet::Configure()
{
  unsigned char Data[2];
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
          sprintf(RegKey,FmtPluginsCache_PluginD,RegNumber);
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

  /* $ 29.12.2000 IS
     ! При настройке "параметров внешних модулей" закрывать окно с их
       списком только при нажатии на ESC
  */
  while(1)
  {
      PluginList.ClearDone();
      /* $ 18.12.2000 SVS
         Shift-F1 в списке плагинов вызывает хелп по данному плагину
      */
      PluginList.Show();
      while (!PluginList.Done())
      {
        int SelPos=PluginList.GetSelectPos();
        PluginList.GetUserData(Data,2,SelPos);
        switch(PluginList.ReadInput())
        {
          case KEY_F1:
          case KEY_SHIFTF1:
            char PluginModuleName[NM*2];
            strcpy(PluginModuleName,PluginsData[Data[0]].ModuleName);
            if(!FarShowHelp(PluginModuleName,"Config",FHELP_SELFHELP|FHELP_NOSHOWERROR) &&
               !FarShowHelp(PluginModuleName,"Configure",FHELP_SELFHELP|FHELP_NOSHOWERROR))
            {
              //strcpy(PluginModuleName,PluginsData[Data[0]].ModuleName);
              FarShowHelp(PluginModuleName,NULL,FHELP_SELFHELP|FHELP_NOSHOWERROR);
            }
            break;
          default:
            PluginList.ProcessInput();
            break;
        }
      }
      /* SVS $ */

      int ExitCode=PluginList.GetExitCode();
      PluginList.Hide();
      if (ExitCode<0)
        return;

      PluginList.GetUserData(Data,2,ExitCode);
      int PNum=Data[0];
      if (PreparePlugin(PNum) && PluginsData[PNum].pConfigure!=NULL)
      {
        //EXCEPTION_POINTERS *xp;
        int Ret;
        TRY{
          Ret=PluginsData[PNum].pConfigure(Data[1]);
        }
        __except ( xfilter(EXCEPT_CONFIGURE,
                          GetExceptionInformation(),&PluginsData[PNum],1) )
        {
          return;
        }
        if (Ret)
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
  /* IS $ */
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
        sprintf(RegKey,FmtPluginsCache_PluginD,RegNumber);
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
      /* $ 18.12.2000 SVS
         Shift-F1 в списке плагинов вызывает хелп по данному плагину
      */
      case KEY_SHIFTF1:
      {
        FarShowHelp(PluginsData[Data[0]].ModuleName,NULL,FHELP_SELFHELP|FHELP_NOSHOWERROR);
        break;
      }
      /* SVS $ */
      /* $ 19.12.2000 IS
         Shift-F9 в списке плагинов вызывает настройки редактора/программы
         просмотра, если этот список был вызван соответственно из
         редактора/программы просмотра. Эта возможность нужна для макросов, да
         и просто удобно.
      */
      case KEY_SHIFTF9:
        if(Editor) EditorConfig();
        else if(Viewer) ViewerConfig();
        break;
      /* IS $ */
      case KEY_ALTF11:
        DumpPluginsInfo();
        break;
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
      sprintf(RegKey,FmtPluginsCache_PluginD,RegNumber);
      sprintf(Value,FmtDiskMenuStringD,PluginItem);
      GetRegKey(RegKey,Value,PluginText,"",100);
      sprintf(Value,FmtDiskMenuNumberD,PluginItem);
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
  /* $ 07.09.2000 VVM 1.18
     + Несколько префиксов у плагина, разделенных через ":"
  */
  DWORD PluginFlags = 0;
  char PluginPrefix[512]="";

  for (int I=0;I<PluginsCount;I++)
  {
    if (PluginsData[I].Cached)
    {
      int RegNumber=GetCacheNumber(PluginsData[I].ModuleName,NULL,PluginsData[I].CachePos);
      if (RegNumber!=-1)
      {
        char RegKey[100];
        sprintf(RegKey,FmtPluginsCache_PluginD,RegNumber);
        GetRegKey(RegKey,"CommandPrefix",PluginPrefix,"",sizeof(PluginPrefix));
        /* $ 07.09.2000 SVS
             По каким-то непонятным причинам из кэше для Flags возвращалось
             значение равное 0 (хотя вижу что в реестре стоит 0x10) :-(
        */
        PluginFlags=GetRegKey(RegKey,"Flags",0);
        /* SVS $ */
      } /* if */
      else
        continue;
    } /* if */
    else
    {
      struct PluginInfo Info;
      if (GetPluginInfo(I,&Info))
      {
        /* $ 10.09.2000 IS
             Вай, вай, как не хорошо... Забыли проверку Info.CommandPrefix на
             NULL сделать, соответственно фар иногда с конвульсиями помирал,
             теперь - нет.
        */
        strcpy(PluginPrefix,NullToEmpty(Info.CommandPrefix));
        /* IS $ */
        PluginFlags = Info.Flags;
      } /* if */
      else
        continue;
    } /* else */
    if (PluginPrefix[0]==0)
      continue;
    char *PrStart = PluginPrefix;
    while(1)
    {
      char *PrEnd = strchr(PrStart, ':');
      if (LocalStrnicmp(Prefix, PrStart, PrEnd==NULL ? strlen(PrStart):(PrEnd-PrStart))==0)
      {
        PluginPos=I;
        break;
      } /* if */
      if (PrEnd == NULL)
        break;
      PrStart = ++PrEnd;
    } /* while */
    if (PluginPos >= 0)
      break;
  } /* for */
  /* VVM $ */

  if (PluginPos==-1)
    return(FALSE);
  if (!PreparePlugin(PluginPos) || PluginsData[PluginPos].pOpenPlugin==NULL)
    return(FALSE);
  Panel *ActivePanel=CtrlObject->ActivePanel;
  if (ActivePanel->ProcessPluginEvent(FE_CLOSE,NULL))
    return(FALSE);
  CtrlObject->CmdLine.SetString("");

  char PluginCommand[512];
  /* $ 07.09.2000 VVM 1.18
    + Если флаг PF_FULLCMDLINE - отдавать с префиксом
  */
  strcpy(PluginCommand,Command+(PluginFlags & PF_FULLCMDLINE ? 0:PrefixLength+1));
  /* VVM $ */
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

/* $ 27.09.2000 SVS
  Функция CallPlugin - найти плагин по ID и запустить
  в зачаточном состоянии!
*/
int PluginsSet::CallPlugin(DWORD SysID,int OpenFrom, void *Data)
{
  int I;
  if((I=FindPlugin(SysID)) != -1)
  {
    if (PluginsData[I].pOpenPlugin)
    {
      HANDLE hNewPlugin=OpenPlugin(I,OpenFrom,(int)Data);

      if (hNewPlugin!=INVALID_HANDLE_VALUE &&
        (OpenFrom == OPEN_PLUGINSMENU || OpenFrom == OPEN_FILEPANEL))
      {
        int CurFocus=CtrlObject->ActivePanel->GetFocus();
        Panel *NewPanel=CtrlObject->ChangePanel(CtrlObject->ActivePanel,FILE_PANEL,TRUE,TRUE);
        NewPanel->SetPluginMode(hNewPlugin,"");
        if (Data && *(char *)Data)
          SetDirectory(hNewPlugin,(char *)Data,0);
        NewPanel->Update(0);
        if (CurFocus || !CtrlObject->GetAnotherPanel(NewPanel)->IsVisible())
          NewPanel->SetFocus();
        NewPanel->Show();
      }
      return TRUE;
    }
  }
  return FALSE;
}

int PluginsSet::FindPlugin(DWORD SysID)
{
  if(SysID != 0 && SysID != 0xFFFFFFFFUl) // не допускается 0 и -1
    for (int I=0;I<PluginsCount;I++)
      if (PluginsData[I].SysID == SysID)
        return I;
  return -1;
}
/* SVS $ */

/* $ 12.10.2000 tran
  новый метод - сбрасывает в файл список плагинов */
void PluginsSet::DumpPluginsInfo()
{
    char file[NM];
    FILE *s;
    PluginItem *p;

    strcpy(file,FarPath);
    strcat(file,"far_plugins.txt");

    s=fopen(file,"w+t");

    if ( s==0 )
        return;

    fprintf(s,"Plugins count = %3i\n"
              "=======================================\n",PluginsCount);
    for ( int i=0; i<PluginsCount; i++ )
    {
        p=&PluginsData[i];
        fprintf(s,"#%3i. ModuleName      = '%s'\n"
               "      hModule         = 0x%08x\n"
               "      SysId           = 0x%08x\n"
               "      Cached          = %s\n"
               "      CachePos        = %i\n"
               "      EditorPlugin    = %i\n"
               "      RootKey         = '%s'\n"
               "      pSetStartupInfo = 0x%p\n\n",
               i,
               p->ModuleName,p->hModule,p->SysID,
               (p->Cached?"yes":"no"),p->CachePos,p->EditorPlugin,
               p->RootKey,p->pSetStartupInfo);
    }
    fprintf(s,"=======================================\n");
    fclose(s);
}

/* $ 23.10.2000 SVS
   Функция TestPluginInfo - проверка на вшивость переданных плагином данных
*/
BOOL PluginsSet::TestPluginInfo(struct PluginItem& Item,struct PluginInfo *Info)
{
  char Buf[1];
  int I=FALSE;
  //EXCEPTION_POINTERS *xp;
  TRY {
    if(Info->DiskMenuStringsNumber > 0 && !Info->DiskMenuStrings)
      RaiseException( STATUS_STRUCTWRONGFILLED, 0, 0, 0);
    else for (I=0; I<Info->DiskMenuStringsNumber; I++)
        memcpy(Buf,Info->DiskMenuStrings[I],1);

    if(Info->PluginMenuStringsNumber > 0 && !Info->PluginMenuStrings)
      RaiseException( STATUS_STRUCTWRONGFILLED+1, 0, 0, 0);
    else for (I=0; I<Info->PluginMenuStringsNumber; I++)
     memcpy(Buf,Info->PluginMenuStrings[I],1);

    if(Info->PluginConfigStringsNumber > 0 && !Info->PluginConfigStrings)
      RaiseException( STATUS_STRUCTWRONGFILLED+2, 0, 0, 0);
    else for (I=0; I<Info->PluginConfigStringsNumber; I++)
      memcpy(Buf,Info->PluginConfigStrings[I],1);

    if (Info->CommandPrefix)
      memcpy(Buf,Info->CommandPrefix,1);

    I=TRUE;
  }
  __except ( xfilter(EXCEPT_GETPLUGININFO_DATA,
                     GetExceptionInformation(),&Item,1) )
  {
     I=FALSE;
  }
  return I;
}
/* SVS $ */

/* $ 31.10.2000 SVS
   Функция TestOpenPluginInfo - проверка на вшивость переданных плагином данных
*/
BOOL PluginsSet::TestOpenPluginInfo(struct PluginItem& Item,struct OpenPluginInfo *Info)
{
  char Buf[1];
  int I=FALSE;
  //EXCEPTION_POINTERS *xp;
  TRY {
    if(Info->HostFile) memcpy(Buf,Info->HostFile,1);
    if(Info->CurDir) memcpy(Buf,Info->CurDir,1);
    if(Info->Format) memcpy(Buf,Info->Format,1);
    if(Info->PanelTitle) memcpy(Buf,Info->PanelTitle,1);

    if(Info->InfoLinesNumber > 0 && !Info->InfoLines)
      RaiseException( STATUS_STRUCTWRONGFILLED, 0, 0, 0);
    else for (I=0; I<Info->InfoLinesNumber; I++)
      memcpy(Buf,&Info->InfoLines[I],1);

    if(Info->DescrFilesNumber > 0 && !Info->DescrFiles)
      RaiseException( STATUS_STRUCTWRONGFILLED+1, 0, 0, 0);
    else for (I=0; I<Info->DescrFilesNumber; I++)
      memcpy(Buf,Info->DescrFiles[I],1);

    if(Info->PanelModesNumber > 0 && !Info->PanelModesArray)
      RaiseException( STATUS_STRUCTWRONGFILLED+2, 0, 0, 0);
    for (I=0; I<Info->PanelModesNumber; I++)
      memcpy(Buf,&Info->PanelModesArray[I],1);

    if(Info->KeyBar) memcpy(Buf,Info->KeyBar,1);
    if(Info->ShortcutData) memcpy(Buf,Info->ShortcutData,1);
    I=TRUE;
  }
  __except ( xfilter(EXCEPT_GETOPENPLUGININFO_DATA,
                     GetExceptionInformation(),&Item,1) )
  {
     I=FALSE;
  }
  return I;
}
/* SVS $ */
