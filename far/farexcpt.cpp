/*
farexcpt.cpp

Все про исключения

*/

/* Revision: 1.22 02.11.2004 $ */

/*
Modify:
  02.11.2004 SVS
    ! Поддержка EXCEPTION_ACCESS_VIOLATION (execute) для WinXP SP2
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  07.05.2004 SVS
    - Для STATUS_STACK_OVERFLOW не вызываем внешний модуль, а выдаем (пытаемся)
      сообщение средствами ФАР, после чего падаем.
      Без этого ФАР молча схлопывается.
  13.01.2003 SVS
    ! Переименование ren exception.?pp farexcpt.?pp
    ! Научим исключатор понимать (и реагировать предсмертным воплем) исключения
      в самом ФАРе.
  04.11.2002 SVS
    ! Дадим возможность вызова внешнего обработчика исключений ("FarEvent.svc")
    + Попытка корректно обработать переполнение стека!
  10.09.2002 SVS
    ! strcpy -> strncpy
  02.07.2002 SVS
    - BugZ#374 - Исключение плагина при выходе из фара
      Ха! А манагер то уже зашутдовен. А в это время вылазит месагбокс...
      В общем... раз вываливаем, то вывалим молча ;-)
  21.02.2002 SVS
    + В обработчике исключений выставляем флаг процесса обработки этого
      самого исключения (ProcessException) и блокируем некоторые клавиши.
  19.02.2002 SVS
    ! Из четверки удаляем писателя. Кода подсократим, да и выглядеть
      (фунциклировать) он будет совсем по другой схеме.
      Тем более, что багов в нем - уйма была, блин :-(
  25.01.2002 SVS
    - Блин, с вашим сраным долбанных MSVC... :-((
  25.01.2002 SVS
    ! Уточнения в писателе (WriteEvent) с учетом изменений в структурах дампа.
    - ошибка в farexcpt.cpp::GetLogicalAddress()
    ! вместо pXXXXXX выставим нужные флаги (FuncFlags)
    ! Куча разных уточнений ;-)
  24.01.2002 SVS
    ! Все же вызовим писателя (WriteEvent) - а уже в нем есть проверка на
      запись
  22.01.2002 SVS
    ! Уточнение в PLUGINRECORD
    + System/UseFarevent:DWORD = 1 писать в дамп, по умолчанию отключено
  02.11.2001 SVS
    ! НЕЛЬЗЯ ИЗ ОБРАБОТЧИКА ВЫГРУЖАТЬ ПЛАГИН!
    ! Временно, до точной отработки формата файла дампа, отключим вызов
      WriteEvent()
  03.10.2001 SVS
    ! добавим некоторое бестыдное количество проверок в "писателя" событий!
  18.09.2001 SVS
    + EXCEPTION_FLT_* - немного обработки плавающей точки
  16.09.2001 SVS
    ! Отключаемые исключения
    ! Удалена кнопка "Debug" за ненадобностью
  10.07.2001 SVS
    + FARAREARECORD.ScrWH - размеры экрана - ширина, высота
  26.06.2001 SVS
    + IsDebuggerPresent() - первая попытка (закомментированная)
  06.06.2001 SVS
    ! Mix/Max
  17.05.2001 SVS
    -  local variable 'xpn' used without having been initialized
  16.05.2001 SVS
    ! Добавлена пользовательская функция EVENTPROC в параметры WriteEvent
    ! Запись рекорда PLUGINRECORD вынесена в отдельную функцию WritePLUGINRECORD()
  16.05.2001 SVS
    + Created
    ! "farexcpt.dmp" переименован в более общее -  "farevent.dmp"
      А если дать плагинам возможность в него писать, то получим то,
      чем является EventLog для виндов :-)))
    ! DumpExceptionInfo from syslog.cpp
    ! DumpExceptionInfo переименован в WriteEvent
    ! xfilter from exception.cpp
    ! Уточнение структуры дампа
*/

#include "headers.hpp"
#pragma hdrstop

#include "plugin.hpp"
#include "lang.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "plugins.hpp"
#include "macro.hpp"
#include "filepanels.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "farexcpt.hpp"
#include "BlockExtKey.hpp"

#define MAX_DELTA_CODE  100

static DWORD _xfilter(int From,EXCEPTION_POINTERS *xp,struct PluginItem *Module,DWORD Flags);

int WriteEvent(DWORD DumpType, // FLOG_*
               EXCEPTION_POINTERS *xp,
               struct PluginItem *Module,
               void *RawData,DWORD RawDataSize,
               DWORD RawDataFlags,DWORD RawType)
{
  return 0;
}

/* ************************************************************************
   $ 16.10.2000 SVS
   Простенький обработчик исключений.
*/
static char* xFromMSGTitle(int From)
{
  if(From == EXCEPT_SETSTARTUPINFO || From == EXCEPT_MINFARVERSION)
    return MSG(MExceptTitleLoad);
  else if(From == (int)INVALID_HANDLE_VALUE)
    return MSG(MExceptTitleFAR);
  else
    return MSG(MExceptTitle);
}

static BOOL Is_STACK_OVERFLOW=FALSE;

DWORD WINAPI xfilter(int From,EXCEPTION_POINTERS *xp,struct PluginItem *Module,DWORD Flags)
{
  static DWORD_PTR stack[1024];
  DWORD Result;

  if (xp->ExceptionRecord->ExceptionCode == STATUS_STACK_OVERFLOW)
  {
    Is_STACK_OVERFLOW=TRUE;
    stack[0] = 0;
    stack[1] = (DWORD_PTR)From;
    stack[2] = (DWORD_PTR)xp;
    stack[3] = (DWORD_PTR)Module;
    stack[4] = Flags;
    #ifdef _WIN64
    xp->ContextRecord->Rsp = (DWORD_PTR)(&stack);
    xp->ContextRecord->Rip = (DWORD_PTR)(&_xfilter);
    #else
    xp->ContextRecord->Esp = (DWORD_PTR)(&stack);
    xp->ContextRecord->Eip = (DWORD_PTR)(&_xfilter);
    #endif

    Result=(DWORD)EXCEPTION_CONTINUE_EXECUTION;
    //Result=_xfilter(From,xp,Module,Flags);
  }
  else
    Result=_xfilter(From,xp,Module,Flags);

  return Result;
}


static DWORD _xfilter(
    int From,                 // откуда: 0 = OpenPlugin, 1 = OpenFilePlugin
    EXCEPTION_POINTERS *xp,   // данные ситуации
    struct PluginItem *Module,// модуль, приведший к исключению.
    DWORD Flags)              // дополнительные флаги - пока только один
                              //        0x1 - спрашивать про выгрузку?
{
   ProcessException=TRUE;
   BlockExtKey blockExtKey;

   DWORD Result = EXCEPTION_EXECUTE_HANDLER;
   BOOL Res=FALSE;

//   if(From == (int)INVALID_HANDLE_VALUE)
//     CriticalInternalError=TRUE;

   if(!Is_STACK_OVERFLOW && GetRegKey("System\\Exception","Used",0))
   {
     static char FarEventSvc[512];
     if(GetRegKey("System\\Exception","FarEvent.svc",FarEventSvc,"",sizeof(FarEventSvc)-1) && FarEventSvc[0])
     {
       HMODULE m = LoadLibrary(FarEventSvc);
       if (m)
       {
         typedef BOOL (WINAPI *ExceptionProc_t)(EXCEPTION_POINTERS *xp,
                                                const struct PLUGINRECORD *Module,
                                                const struct PluginStartupInfo *LocalStartupInfo,
                                                LPDWORD Result);

         ExceptionProc_t p = (ExceptionProc_t)GetProcAddress(m,"ExceptionProc");

         if (p)
         {
           static struct PluginStartupInfo LocalStartupInfo;
           memset(&LocalStartupInfo,0,sizeof(LocalStartupInfo));
           static struct FarStandardFunctions LocalStandardFunctions;
           memset(&LocalStandardFunctions,0,sizeof(LocalStandardFunctions));

           CtrlObject->Plugins.CreatePluginStartupInfo(&LocalStartupInfo,
                                           &LocalStandardFunctions,
                                           FarEventSvc,
                                           -1);
           static char RootKey[NM];
           xstrncpy(RootKey,Opt.RegRoot,sizeof(RootKey)-1);
           LocalStartupInfo.RootKey=RootKey;

           static struct PLUGINRECORD PlugRec;
           if(Module)
           {
             memset(&PlugRec,0,sizeof(PlugRec));

             PlugRec.TypeRec=RTYPE_PLUGIN;
             PlugRec.SizeRec=sizeof(struct PLUGINRECORD);

             memcpy(&PlugRec.FindData,&Module->FindData,sizeof(PlugRec.FindData));
             PlugRec.SysID=Module->SysID;
             PlugRec.WorkFlags=Module->WorkFlags.Flags;
             PlugRec.CallFlags=Module->FuncFlags.Flags;

             PlugRec.FuncFlags=0;
             PlugRec.FuncFlags|=Module->pSetStartupInfo?PICFF_SETSTARTUPINFO:0;
             PlugRec.FuncFlags|=Module->pOpenPlugin?PICFF_OPENPLUGIN:0;
             PlugRec.FuncFlags|=Module->pOpenFilePlugin?PICFF_OPENFILEPLUGIN:0;
             PlugRec.FuncFlags|=Module->pClosePlugin?PICFF_CLOSEPLUGIN:0;
             PlugRec.FuncFlags|=Module->pGetPluginInfo?PICFF_GETPLUGININFO:0;
             PlugRec.FuncFlags|=Module->pGetOpenPluginInfo?PICFF_GETOPENPLUGININFO:0;
             PlugRec.FuncFlags|=Module->pGetFindData?PICFF_GETFINDDATA:0;
             PlugRec.FuncFlags|=Module->pFreeFindData?PICFF_FREEFINDDATA:0;
             PlugRec.FuncFlags|=Module->pGetVirtualFindData?PICFF_GETVIRTUALFINDDATA:0;
             PlugRec.FuncFlags|=Module->pFreeVirtualFindData?PICFF_FREEVIRTUALFINDDATA:0;
             PlugRec.FuncFlags|=Module->pSetDirectory?PICFF_SETDIRECTORY:0;
             PlugRec.FuncFlags|=Module->pGetFiles?PICFF_GETFILES:0;
             PlugRec.FuncFlags|=Module->pPutFiles?PICFF_PUTFILES:0;
             PlugRec.FuncFlags|=Module->pDeleteFiles?PICFF_DELETEFILES:0;
             PlugRec.FuncFlags|=Module->pMakeDirectory?PICFF_MAKEDIRECTORY:0;
             PlugRec.FuncFlags|=Module->pProcessHostFile?PICFF_PROCESSHOSTFILE:0;
             PlugRec.FuncFlags|=Module->pSetFindList?PICFF_SETFINDLIST:0;
             PlugRec.FuncFlags|=Module->pConfigure?PICFF_CONFIGURE:0;
             PlugRec.FuncFlags|=Module->pExitFAR?PICFF_EXITFAR:0;
             PlugRec.FuncFlags|=Module->pProcessKey?PICFF_PROCESSKEY:0;
             PlugRec.FuncFlags|=Module->pProcessEvent?PICFF_PROCESSEVENT:0;
             PlugRec.FuncFlags|=Module->pProcessEditorEvent?PICFF_PROCESSEDITOREVENT:0;
             PlugRec.FuncFlags|=Module->pCompare?PICFF_COMPARE:0;
             PlugRec.FuncFlags|=Module->pProcessEditorInput?PICFF_PROCESSEDITORINPUT:0;
             PlugRec.FuncFlags|=Module->pMinFarVersion?PICFF_MINFARVERSION:0;
             PlugRec.FuncFlags|=Module->pProcessViewerEvent?PICFF_PROCESSVIEWEREVENT:0;
             PlugRec.CachePos=Module->CachePos;
           }

           Res=p(xp,(Module?&PlugRec:NULL),&LocalStartupInfo,&Result);
         }
         FreeLibrary(m);
       }
     }
   }

   if(Res)
   {
     if(From == (int)INVALID_HANDLE_VALUE)
     {
       CriticalInternalError=TRUE;
       TerminateProcess( GetCurrentProcess(), 1);
     }
     return Result;
   }


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
     {EXCEPTION_FLT_DIVIDE_BY_ZERO,MExcFloatDivideByZero,EXCEPTION_EXECUTE_HANDLER},
     {EXCEPTION_FLT_OVERFLOW,MExcFloatOverflow,EXCEPTION_EXECUTE_HANDLER},
     {EXCEPTION_FLT_STACK_CHECK,MExcFloatStackOverflow,EXCEPTION_EXECUTE_HANDLER},
     {EXCEPTION_FLT_UNDERFLOW,MExcFloatUnderflow,EXCEPTION_EXECUTE_HANDLER},
     // сюды добавляем.
   };
   // EXCEPTION_CONTINUE_EXECUTION  ??????
   char *pName;
   int  I, Ret=1;
   DWORD rc;
   char Buf[2][NM];
   char TruncFileName[2*NM];
   BOOL Unload = FALSE; // Установить в истину, если плагин нужно выгрузить
   BOOL ShowMessages=FALSE;

   // получим запись исключения
   EXCEPTION_RECORD *xr = xp->ExceptionRecord;

   // выведим дамп перед выдачей сообщений
   if (xr->ExceptionCode != STATUS_INVALIDFUNCTIONRESULT)
     WriteEvent(FLOG_ALL&(~FLOG_PLUGINSINFO),xp,Module,NULL,0);

   // CONTEXT можно использовать для отображения или записи в лог
   //         содержимого регистров...
   // CONTEXT *xc = xp->ContextRecord;

   rc = Result;// EXCEPTION_EXECUTE_HANDLER;

   /*$ 23.01.2001 skv
     Неизвестное исключение не стоит игнорировать.
   */
   pName=NULL;
   if(From == (int)INVALID_HANDLE_VALUE || !Module)
     GetModuleFileName(NULL,TruncFileName,sizeof(TruncFileName));
   else
     xstrncpy(TruncFileName,NullToEmpty(Module->ModuleName),sizeof(TruncFileName));

   /* $ 26.02.2001 VVM
       ! Обработка STATUS_INVALIDFUNCTIONRESULT */
   // Этот кусок обрабатываем в первую очередь, т.к. это проверки "на вшивость"
   if (From == EXCEPT_GETPLUGININFO_DATA || From == EXCEPT_GETOPENPLUGININFO_DATA)
   {
     I = 0;
     static const char *NameField[2][3]={
       {"DiskMenuStrings","PluginMenuStrings","PluginConfigStrings"},
       {"InfoLines","DescrFiles","PanelModesArray"},};
     switch(From)
     {
       case EXCEPT_GETPLUGININFO_DATA:
         pName = "PluginInfo";
         I = 0;
         break;
       case EXCEPT_GETOPENPLUGININFO_DATA:
         pName = "OpenPluginInfo";
         I = 1;
         break;
     }

     if(xr->ExceptionCode >= STATUS_STRUCTWRONGFILLED &&
        xr->ExceptionCode <= STATUS_STRUCTWRONGFILLED+2)
     {
       sprintf(Buf[0],
           MSG(MExcStructField),
           pName,
           NameField[I][xr->ExceptionCode-STATUS_STRUCTWRONGFILLED]);
     }
     else
       sprintf(Buf[0],MSG(MExcStructWrongFilled),pName);

     if(FrameManager && !FrameManager->ManagerIsDown())
     {
       Message(MSG_WARNING,1,
            xFromMSGTitle(From),
            MSG(MExcTrappedException),
            MSG(MExcCheckOnLousys),
            TruncPathStr(TruncFileName,40),
            Buf[0],
            "\1",
            MSG(MExcUnloadYes),
            MSG(MOk));
       ShowMessages=TRUE;
     }
   } /* EXCEPT_GETPLUGININFO_DATA && EXCEPT_GETOPENPLUGININFO_DATA */

   // теперь обработаем исключение по возврату 0 вместо INVALID_HANDLE_VALUE
   // из Open*Plugin()
   else if (xr->ExceptionCode == STATUS_INVALIDFUNCTIONRESULT)
   {
     switch (From)
     {
       case EXCEPT_OPENPLUGIN:
         pName="OpenPlugin";
         break;
       case EXCEPT_OPENFILEPLUGIN:
         pName="OpenFilePlugin";
         break;
       case EXCEPT_OPENPLUGIN_FINDLIST:
         pName="OpenPlugin(OPEN_FINDLIST)";
         break;
       default:
         pName="???";
         break;
     }

     sprintf(Buf[0],MSG(MExcInvalidFuncResult),pName);
     if(FrameManager && !FrameManager->ManagerIsDown())
     {
       Message(MSG_WARNING, 1,
                 xFromMSGTitle(From),
                 MSG(MExcTrappedException),
                 MSG(MExcCheckOnLousys),
                 TruncPathStr(TruncFileName,40),
                 Buf[0],
                 "\1",
                 MSG(MExcUnloadYes),
                 MSG(MOk));
       ShowMessages=TRUE;
     }
   }

   else
   {
     // просмотрим "знакомые" FAR`у исключения и обработаем...
     for(I=0; I < sizeof(ECode)/sizeof(ECode[0]); ++I)
       if(ECode[I].Code == xr->ExceptionCode)
       {
         pName=MSG(ECode[I].IdMsg);
         rc=ECode[I].RetCode;
         if(xr->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
         {
           int Offset = 0;

           // вот только не надо здесь неочевидных оптимизаций вида
           // if ( xr->ExceptionInformation[0] == 8 ) Offset = 2 else Offset = xr->ExceptionInformation[0],
           // а то M$ порадует нас как-нибудь xr->ExceptionInformation[0] == 4 и все будет в полной жопе.

           switch ( xr->ExceptionInformation[0] )
           {
             case 0:
               Offset = 0;
               break;

             case 1:
               Offset = 1;
               break;

             case 8:
               Offset = 2;
               break;
           }

           sprintf(Buf[1],MSG(Offset+MExcRAccess),xr->ExceptionInformation[1]);
           pName=Buf[1];
         }
         break;
       }

     if (!pName) pName=MSG(MExcUnknown);

     sprintf(Buf[0],MSG(MExcAddress),xr->ExceptionAddress);
     if(FrameManager && !FrameManager->ManagerIsDown())
     {
       Message(MSG_WARNING,1,
               xFromMSGTitle(From),
               MSG(MExcTrappedException),
               pName,
               Buf[0],
               TruncPathStr(TruncFileName,40),"\1",
               MSG((From == (int)INVALID_HANDLE_VALUE)?MExcFARTerminateYes:MExcUnloadYes),
               MSG(MOk));
       ShowMessages=TRUE;
     }
   } /* else */

   if(From == (int)INVALID_HANDLE_VALUE && ShowMessages)
   {
     CriticalInternalError=TRUE;
     TerminateProcess( GetCurrentProcess(), 1);
   }

   rc = EXCEPTION_EXECUTE_HANDLER;
   /* VVM $ */

   if(xr->ExceptionFlags&EXCEPTION_NONCONTINUABLE)
     rc=EXCEPTION_CONTINUE_SEARCH; //?

//   return UnhandledExceptionFilter(xp);

   return rc;
}
