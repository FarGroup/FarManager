/*
farexcpt.cpp

Все про исключения

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

static DWORD _xfilter(int From,EXCEPTION_POINTERS *xp, Plugin *Module,DWORD Flags);

int WriteEvent(DWORD DumpType, // FLOG_*
               EXCEPTION_POINTERS *xp,
               Plugin *Module,
               void *RawData,DWORD RawDataSize,
               DWORD RawDataFlags,DWORD RawType)
{
  return 0;
}

/* ************************************************************************
   $ 16.10.2000 SVS
   Простенький обработчик исключений.
*/
static const wchar_t* xFromMSGTitle(int From)
{
  if(From == EXCEPT_SETSTARTUPINFO || From == EXCEPT_MINFARVERSION)
    return UMSG(MExceptTitleLoad);
  else if(From == (int)(INT_PTR)INVALID_HANDLE_VALUE)
    return UMSG(MExceptTitleFAR);
  else
    return UMSG(MExceptTitle);
}

static BOOL Is_STACK_OVERFLOW=FALSE;

DWORD WINAPI xfilter(int From,EXCEPTION_POINTERS *xp, Plugin *Module,DWORD Flags)
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
    Plugin *Module,// модуль, приведший к исключению.
    DWORD Flags)              // дополнительные флаги - пока только один
                              //        0x1 - спрашивать про выгрузку?
{
   ProcessException=TRUE;
   BlockExtKey blockExtKey;

   DWORD Result = EXCEPTION_EXECUTE_HANDLER;
   BOOL Res=FALSE;

//   if(From == (int)INVALID_HANDLE_VALUE)
//     CriticalInternalError=TRUE;

   if(!Is_STACK_OVERFLOW && GetRegKeyW(L"System\\Exception",L"Used",0))
   {
     static string strFarEventSvc;
     if(GetRegKeyW(L"System\\Exception",L"FarEvent.svc",strFarEventSvc,L"") && !strFarEventSvc.IsEmpty())
     {
       HMODULE m = LoadLibraryW(strFarEventSvc);
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
                                           strFarEventSvc,
                                           -1);
           static string strRootKey;
           strRootKey = Opt.strRegRoot;
           LocalStartupInfo.RootKey=strRootKey;

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
     if(From == (int)(INT_PTR)INVALID_HANDLE_VALUE)
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
   const wchar_t *pName;
   int  I, Ret=1;
   DWORD rc;
   string strBuf1, strBuf2;

   string strTruncFileName;
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
   if(From == (int)(INT_PTR)INVALID_HANDLE_VALUE || !Module)
     apiGetModuleFileName (NULL, strTruncFileName);
   else
     strTruncFileName = Module->strModuleName;

   /* $ 26.02.2001 VVM
       ! Обработка STATUS_INVALIDFUNCTIONRESULT */
   // Этот кусок обрабатываем в первую очередь, т.к. это проверки "на вшивость"
   if (From == EXCEPT_GETPLUGININFO_DATA || From == EXCEPT_GETOPENPLUGININFO_DATA)
   {
     I = 0;
     static const wchar_t *NameField[2][3]={
       {L"DiskMenuStrings",L"PluginMenuStrings",L"PluginConfigStrings"},
       {L"InfoLines",L"DescrFiles",L"PanelModesArray"},};
     switch(From)
     {
       case EXCEPT_GETPLUGININFO_DATA:
         pName = L"PluginInfo";
         I = 0;
         break;
       case EXCEPT_GETOPENPLUGININFO_DATA:
         pName = L"OpenPluginInfo";
         I = 1;
         break;
     }

     if(xr->ExceptionCode >= STATUS_STRUCTWRONGFILLED &&
        xr->ExceptionCode <= STATUS_STRUCTWRONGFILLED+2)
     {
       strBuf1.Format (
           UMSG(MExcStructField),
           pName,
           NameField[I][xr->ExceptionCode-STATUS_STRUCTWRONGFILLED]);
     }
     else
       strBuf1.Format (UMSG(MExcStructWrongFilled),pName);

     if(FrameManager && !FrameManager->ManagerIsDown())
     {
       TruncPathStrW(strTruncFileName,40);

       MessageW(MSG_WARNING,1,
            xFromMSGTitle(From),
            UMSG(MExcTrappedException),
            UMSG(MExcCheckOnLousys),
            strTruncFileName,
            strBuf1,
            L"\1",
            UMSG(MExcUnloadYes),
            UMSG(MOk));
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
         pName=L"OpenPlugin";
         break;
       case EXCEPT_OPENFILEPLUGIN:
         pName=L"OpenFilePlugin";
         break;
       case EXCEPT_OPENPLUGIN_FINDLIST:
         pName=L"OpenPlugin(OPEN_FINDLIST)";
         break;
       default:
         pName=L"???";
         break;
     }

     strBuf1.Format (UMSG(MExcInvalidFuncResult),pName);
     if(FrameManager && !FrameManager->ManagerIsDown())
     {
       TruncPathStrW(strTruncFileName,40);

       MessageW(MSG_WARNING, 1,
                 xFromMSGTitle(From),
                 UMSG(MExcTrappedException),
                 UMSG(MExcCheckOnLousys),
                 strTruncFileName,
                 strBuf1,
                 L"\1",
                 UMSG(MExcUnloadYes),
                 UMSG(MOk));
       ShowMessages=TRUE;
     }
   }

   else
   {
     // просмотрим "знакомые" FAR`у исключения и обработаем...
     for(I=0; I < sizeof(ECode)/sizeof(ECode[0]); ++I)
       if(ECode[I].Code == xr->ExceptionCode)
       {
         pName=UMSG(ECode[I].IdMsg);
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

           strBuf2.Format (UMSG(Offset+MExcRAccess),xr->ExceptionInformation[1]);
           pName=strBuf2;
         }
         break;
       }

     if (!pName) pName=UMSG(MExcUnknown);

     strBuf1.Format (UMSG(MExcAddress),xr->ExceptionAddress);
     if(FrameManager && !FrameManager->ManagerIsDown())
     {
       TruncPathStrW(strTruncFileName, 40);

       MessageW(MSG_WARNING,1,
               xFromMSGTitle(From),
               UMSG(MExcTrappedException),
               pName,
               strBuf1,
               strTruncFileName, L"\1",
               UMSG((From == (int)(INT_PTR)INVALID_HANDLE_VALUE)?MExcFARTerminateYes:MExcUnloadYes),
               UMSG(MOk));
       ShowMessages=TRUE;
     }
   } /* else */

   if(From == (int)(INT_PTR)INVALID_HANDLE_VALUE && ShowMessages)
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
