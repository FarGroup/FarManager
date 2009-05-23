/*
farexcpt.cpp

Все про исключения
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "farexcpt.hpp"
#include "lang.hpp"
#include "plugins.hpp"
#include "macro.hpp"
#include "filepanels.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "BlockExtKey.hpp"
#include "registry.hpp"
#include "message.hpp"
#include "config.hpp"

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
    return MSG(MExceptTitleLoad);
  else if(From == (int)(INT_PTR)INVALID_HANDLE_VALUE)
    return MSG(MExceptTitleFAR);
  else
    return MSG(MExceptTitle);
}


static BOOL Is_STACK_OVERFLOW=FALSE;

// Some parametes for _xfilter function
static int From=0;                     // откуда: 0 = OpenPlugin, 1 = OpenFilePlugin
static EXCEPTION_POINTERS *xp=NULL;    // данные ситуации
static Plugin *Module=NULL;     // модуль, приведший к исключению.
static DWORD Flags=0;                  // дополнительные флаги - пока только один
                                       //        0x1 - спрашивать про выгрузку?

extern void CreatePluginStartupInfo (Plugin *pPlugin, PluginStartupInfo *PSI, FarStandardFunctions *FSF);

static DWORD WINAPI _xfilter (LPVOID dummy=NULL)
{
   ProcessException=TRUE;
   BlockExtKey blockExtKey;

   DWORD Result = EXCEPTION_EXECUTE_HANDLER;
   BOOL Res=FALSE;

//   if(From == (int)INVALID_HANDLE_VALUE)
//     CriticalInternalError=TRUE;

   if(!Is_STACK_OVERFLOW && GetRegKey(L"System\\Exception",L"Used",0))
   {
     static string strFarEventSvc;
     if(GetRegKey(L"System\\Exception",L"FarEvent.svc",strFarEventSvc,L"") && !strFarEventSvc.IsEmpty())
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

           CreatePluginStartupInfo (NULL, &LocalStartupInfo, &LocalStandardFunctions);
           LocalStartupInfo.ModuleName = (const wchar_t *)strFarEventSvc;

           static string strRootKey;
           strRootKey = Opt.strRegRoot;
           LocalStartupInfo.RootKey=strRootKey;

           static struct PLUGINRECORD PlugRec;
           if (Module)
           {
             memset(&PlugRec,0,sizeof(PlugRec));

             PlugRec.TypeRec=RTYPE_PLUGIN;
             PlugRec.SizeRec=sizeof(struct PLUGINRECORD);

             PlugRec.ModuleName=Module->GetModuleName();

             PlugRec.SysID=Module->GetSysID();
             PlugRec.WorkFlags=Module->GetWorkFlags();
             PlugRec.CallFlags=Module->GetFuncFlags();

             PlugRec.FuncFlags=0;
             PlugRec.FuncFlags|=Module->HasSetStartupInfo()?PICFF_SETSTARTUPINFO:0;
             PlugRec.FuncFlags|=Module->HasOpenPlugin()?PICFF_OPENPLUGIN:0;
             PlugRec.FuncFlags|=Module->HasOpenFilePlugin()?PICFF_OPENFILEPLUGIN:0;
             PlugRec.FuncFlags|=Module->HasClosePlugin()?PICFF_CLOSEPLUGIN:0;
             PlugRec.FuncFlags|=Module->HasGetPluginInfo()?PICFF_GETPLUGININFO:0;
             PlugRec.FuncFlags|=Module->HasGetOpenPluginInfo()?PICFF_GETOPENPLUGININFO:0;
             PlugRec.FuncFlags|=Module->HasGetFindData()?PICFF_GETFINDDATA:0;
             PlugRec.FuncFlags|=Module->HasFreeFindData()?PICFF_FREEFINDDATA:0;
             PlugRec.FuncFlags|=Module->HasGetVirtualFindData()?PICFF_GETVIRTUALFINDDATA:0;
             PlugRec.FuncFlags|=Module->HasFreeVirtualFindData()?PICFF_FREEVIRTUALFINDDATA:0;
             PlugRec.FuncFlags|=Module->HasSetDirectory()?PICFF_SETDIRECTORY:0;
             PlugRec.FuncFlags|=Module->HasGetFiles()?PICFF_GETFILES:0;
             PlugRec.FuncFlags|=Module->HasPutFiles()?PICFF_PUTFILES:0;
             PlugRec.FuncFlags|=Module->HasDeleteFiles()?PICFF_DELETEFILES:0;
             PlugRec.FuncFlags|=Module->HasMakeDirectory()?PICFF_MAKEDIRECTORY:0;
             PlugRec.FuncFlags|=Module->HasProcessHostFile()?PICFF_PROCESSHOSTFILE:0;
             PlugRec.FuncFlags|=Module->HasSetFindList()?PICFF_SETFINDLIST:0;
             PlugRec.FuncFlags|=Module->HasConfigure()?PICFF_CONFIGURE:0;
             PlugRec.FuncFlags|=Module->HasExitFAR()?PICFF_EXITFAR:0;
             PlugRec.FuncFlags|=Module->HasProcessKey()?PICFF_PROCESSKEY:0;
             PlugRec.FuncFlags|=Module->HasProcessEvent()?PICFF_PROCESSEVENT:0;
             PlugRec.FuncFlags|=Module->HasProcessEditorEvent()?PICFF_PROCESSEDITOREVENT:0;
             PlugRec.FuncFlags|=Module->HasCompare()?PICFF_COMPARE:0;
             PlugRec.FuncFlags|=Module->HasProcessEditorInput()?PICFF_PROCESSEDITORINPUT:0;
             PlugRec.FuncFlags|=Module->HasMinFarVersion()?PICFF_MINFARVERSION:0;
             PlugRec.FuncFlags|=Module->HasProcessViewerEvent()?PICFF_PROCESSVIEWEREVENT:0;
             PlugRec.FuncFlags|=Module->HasProcessDialogEvent()?PICFF_PROCESSDIALOGEVENT:0;
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
       //TerminateProcess( GetCurrentProcess(), 1);
     }
     return (::From=Result);
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
     {EXCEPTION_FLT_DIVIDE_BY_ZERO,MExcFloatDivideByZero,EXCEPTION_EXECUTE_HANDLER}, // BUGBUG: Floating-point exceptions (VC) are disabled by default. See http://msdn2.microsoft.com/en-us/library/aa289157(vs.71).aspx#floapoint_topic8
     {EXCEPTION_FLT_OVERFLOW,MExcFloatOverflow,EXCEPTION_EXECUTE_HANDLER},           // BUGBUG:  ^^^
     {EXCEPTION_FLT_STACK_CHECK,MExcFloatStackOverflow,EXCEPTION_EXECUTE_HANDLER},   // BUGBUG:  ^^^
     {EXCEPTION_FLT_UNDERFLOW,MExcFloatUnderflow,EXCEPTION_EXECUTE_HANDLER},         // BUGBUG:  ^^^
     {EXCEPTION_ILLEGAL_INSTRUCTION,MExcBadInstruction,EXCEPTION_EXECUTE_HANDLER},
     {EXCEPTION_PRIV_INSTRUCTION,MExcBadInstruction,EXCEPTION_EXECUTE_HANDLER},
     {EXCEPTION_DATATYPE_MISALIGNMENT, MExcDatatypeMisalignment, EXCEPTION_EXECUTE_HANDLER},
  // сюды добавляем.
   };
   // EXCEPTION_CONTINUE_EXECUTION  ??????
   const wchar_t *pName;
   DWORD rc;
   string strBuf1, strBuf2, strBuf3;

   string strFileName;
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
     apiGetModuleFileName (NULL, strFileName);
   else
     strFileName = Module->GetModuleName();

   /* $ 26.02.2001 VVM
       ! Обработка STATUS_INVALIDFUNCTIONRESULT */
   // Этот кусок обрабатываем в первую очередь, т.к. это проверки "на вшивость"
   if (From == EXCEPT_GETPLUGININFO_DATA || From == EXCEPT_GETOPENPLUGININFO_DATA)
   {
     int I = 0;
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
           MSG(MExcStructField),
           pName,
           NameField[I][xr->ExceptionCode-STATUS_STRUCTWRONGFILLED]);
     }
     else
       strBuf1.Format (MSG(MExcStructWrongFilled),pName);

     if(FrameManager && !FrameManager->ManagerIsDown())
     {
       Message(MSG_WARNING,1,
            xFromMSGTitle(From),
            MSG(MExcTrappedException),
            MSG(MExcCheckOnLousys),
            strFileName,
            strBuf1,
            L"\1",
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

     strBuf1.Format (MSG(MExcInvalidFuncResult),pName);
     if(FrameManager && !FrameManager->ManagerIsDown())
     {
       Message(MSG_WARNING, 1,
                 xFromMSGTitle(From),
                 MSG(MExcTrappedException),
                 MSG(MExcCheckOnLousys),
                 strFileName,
                 strBuf1,
                 L"\1",
                 MSG(MExcUnloadYes),
                 MSG(MOk));
       ShowMessages=TRUE;
     }
   }
   else
   {
     // просмотрим "знакомые" FAR`у исключения и обработаем...
     for (size_t I=0; I < countof(ECode); ++I)
     {
       if (ECode[I].Code == xr->ExceptionCode)
       {
         pName=MSG(ECode[I].IdMsg);
         rc=ECode[I].RetCode;
         if (xr->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
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

           strBuf2.Format (MSG(Offset+MExcRAccess),xr->ExceptionInformation[1]);
           pName=strBuf2;
         }
         break;
       }
     }

     if (!pName)
     {
       strBuf2.Format(L"%s (0x%X)", MSG(MExcUnknown), xr->ExceptionCode);
       pName = strBuf2;
     }

     strBuf1.Format (MSG(MExcAddress),xr->ExceptionAddress);
     if (FrameManager && !FrameManager->ManagerIsDown())
     {
       Message(MSG_WARNING,1,
               xFromMSGTitle(From),
               MSG(MExcTrappedException),
               pName,
               strBuf1,
               strFileName, L"\1",
               MSG((From == (int)(INT_PTR)INVALID_HANDLE_VALUE)?MExcFARTerminateYes:MExcUnloadYes),
               MSG(MOk));
       ShowMessages=TRUE;
     }
   } /* else */

   if (ShowMessages && (Is_STACK_OVERFLOW || From == (int)(INT_PTR)INVALID_HANDLE_VALUE))
   {
     CriticalInternalError=TRUE;
     TerminateProcess( GetCurrentProcess(), 1);
   }

   rc = EXCEPTION_EXECUTE_HANDLER;

   if (xr->ExceptionFlags&EXCEPTION_NONCONTINUABLE)
     rc=EXCEPTION_CONTINUE_SEARCH; //?

//   return UnhandledExceptionFilter(xp);

   return (::From=rc);
}


DWORD WINAPI xfilter(int From,EXCEPTION_POINTERS *xp, Plugin *Module,DWORD Flags)
{
  // dummy parametrs setting
  ::From=From;
  ::xp=xp;
  ::Module=Module;
  ::Flags=Flags;

    if (xp->ExceptionRecord->ExceptionCode == STATUS_STACK_OVERFLOW) // restore stack & call_xfilter ;
  {
    Is_STACK_OVERFLOW=TRUE;

#ifdef _M_IA64
    // TODO: Bad way to restore IA64 stacks (CreateThread)
    // Can you do smartly? See REMINDER file, section IA64Stacks
    static HANDLE hThread = NULL;
    if ( (hThread = CreateThread (NULL, 0, _xfilter, NULL, 0, NULL)) == NULL )
    {
      TerminateProcess (GetCurrentProcess(), 1);
    }
    WaitForSingleObject (hThread, INFINITE);
    CloseHandle( hThread );
#else
    static struct {
      BYTE      stack_space[32768];
      DWORD_PTR ret_addr;
      DWORD_PTR args[4];
    }_stack;

    _stack.ret_addr = 0;
#ifndef _WIN64
    //_stack.args[0] = (DWORD_PTR)From;
    //_stack.args[1] = (DWORD_PTR)xp;
    //_stack.args[2] = (DWORD_PTR)Module;
    //_stack.args[3] = Flags;
    xp->ContextRecord->Esp = (DWORD)(DWORD_PTR)(&_stack.ret_addr);
    xp->ContextRecord->Eip = (DWORD)(DWORD_PTR)(&_xfilter);
#else
    //xp->ContextRecord->Rcx = (DWORD_PTR)From;
    //xp->ContextRecord->Rdx = (DWORD_PTR)xp;
    //xp->ContextRecord->R8  = (DWORD_PTR)Module;
    //xp->ContextRecord->R9  = Flags;
    xp->ContextRecord->Rsp = (DWORD_PTR)(&_stack.ret_addr);
    xp->ContextRecord->Rip = (DWORD_PTR)(&_xfilter);
#endif
#endif
    ::From=(DWORD)EXCEPTION_CONTINUE_EXECUTION;
  }
  else
    _xfilter ();

  return ::From;
}
