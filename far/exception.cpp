/*
farexcpt.cpp

Все про исключения

*/

/* Revision: 1.01 16.05.2001 $ */

/*
Modify:
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

BOOL GetLogicalAddress(void* addr, char *szModule, DWORD len, DWORD& section, DWORD& offset);
void IntelStackWalk(HANDLE fp,PCONTEXT pContext, DWORD& SizeOfRecord, DWORD& StackCount);
int WritePLUGINRECORD(HANDLE fp,struct PluginItem *Module,DWORD *DumpSize);

int WriteEvent(DWORD DumpType, // FLOG_*
               EXCEPTION_POINTERS *xp,
               struct PluginItem *Module,
               void *RawData,DWORD RawDataSize,
               DWORD RawDataFlags,
               EVENTPROC CallBackProc)
{
  static char FarEventFileName[MAX_PATH];
  GetModuleFileName(NULL,FarEventFileName,sizeof(FarEventFileName));
  strcpy(strrchr(FarEventFileName,'\\')+1,"farevent.dmp");
  HANDLE fp=CreateFile(FarEventFileName,
               GENERIC_READ|GENERIC_WRITE,
               FILE_SHARE_READ|FILE_SHARE_WRITE,
               NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_ARCHIVE,NULL);

  if(fp == INVALID_HANDLE_VALUE)
    return 0;

  DWORD Temp, Pos;
  DWORD CountDump;
  DWORD BeginEventFilePos;
  struct DUMPHEADER DumpHeader;
  memset(&DumpHeader,0,sizeof(DumpHeader));

  // подготовим заголовок
  if((Pos=GetFileSize(fp,NULL)) == -1 || !Pos)
    CountDump=0;
  else
  {
    SetFilePointer(fp,0,NULL,FILE_BEGIN);
    ReadFile(fp,&CountDump,sizeof(CountDump),&Temp,NULL);
  }
  CountDump++;
  SetFilePointer(fp,0,NULL,FILE_BEGIN);
  WriteFile(fp,&CountDump,sizeof(CountDump),&Temp,NULL);
  // Чтобы вернутся для записи размера!
  BeginEventFilePos=SetFilePointer(fp,0,NULL,FILE_END);

  // готовим реальный заголовок
  DumpHeader.TypeRec=RTYPE_HEADER;
  DumpHeader.SizeRec=sizeof(struct DUMPHEADER)-sizeof(struct RECHEADER);
  DumpHeader.DumpFlags=DumpType;
  GetSystemTime(&DumpHeader.DumpTime);
  DumpHeader.RecordsCount++;
  DumpHeader.DumpSize+=DumpHeader.SizeRec+sizeof(struct RECHEADER);

  // пишем заголовок  первый раз
  WriteFile(fp,&DumpHeader,sizeof(DumpHeader),&Temp,NULL);

  if(DumpType&FLOG_SYSINFO)
  {
    struct SYSINFOHEADER SysHdr;
    memset(&SysHdr,0,sizeof(SysHdr));
    SysHdr.TypeRec=RTYPE_SYSINFO;
    SysHdr.SizeRec=sizeof(struct SYSINFOHEADER)-sizeof(struct RECHEADER);
    memcpy(&SysHdr.WinVer,&WinVer,sizeof(OSVERSIONINFO));
    SysHdr.FARVersion=FAR_VERSION;
    WriteFile(fp,&SysHdr,sizeof(SysHdr),&Temp,NULL);

    DumpHeader.RecordsCount++;
    DumpHeader.DumpSize+=sizeof(struct SYSINFOHEADER);
  }

  // **** информация о плагинах
  if(DumpType&FLOG_PLUGINSINFO)
  {
    struct PLUGINSINFORECORD PlugRecs;
    PlugRecs.TypeRec=FLOG_PLUGINSINFO;
    PlugRecs.SizeRec=sizeof(struct PLUGINSINFORECORD)-sizeof(struct RECHEADER);
    PlugRecs.PluginsCount=CtrlObject->Plugins.PluginsCount;
    WriteFile(fp,&PlugRecs,sizeof(PlugRecs),&Temp,NULL);
    DumpHeader.RecordsCount++;
    DumpHeader.DumpSize+=PlugRecs.SizeRec+sizeof(struct RECHEADER);

    if(PlugRecs.PluginsCount)
    {
      for(int I=0; I < PlugRecs.PluginsCount; ++I)
         DumpHeader.RecordsCount+=WritePLUGINRECORD(fp,CtrlObject->Plugins.PluginsData+I,&DumpHeader.DumpSize);
    }
  }

  if((DumpType&FLOG_PLUGIN) && Module)
    DumpHeader.RecordsCount+=WritePLUGINRECORD(fp,Module,&DumpHeader.DumpSize);

  if(xp)
  {
    struct EXCEPTIONRECORD Excpt;
    memset(&Excpt,0,sizeof(Excpt));

    CONTEXT *cn;
    struct _EXCEPTION_RECORD *xpn;

    cn=xp->ContextRecord;

    if((DumpType&FLOG_REGISTERS) && cn)
    {
      struct CONTEXTRECORD Regs;
      Regs.TypeRec=RTYPE_CONTEXT;
      Regs.SizeRec=sizeof(struct CONTEXTRECORD)-sizeof(struct RECHEADER);
      memcpy(&Regs.Regs,cn,Regs.SizeRec); //???? Regs.SizeRec ????
      WriteFile(fp,&Regs,sizeof(Regs),&Temp,NULL);
      DumpHeader.RecordsCount++;
      DumpHeader.DumpSize+=sizeof(struct CONTEXTRECORD)+sizeof(struct RECHEADER);

//_SVS(SysLog("Begin"));
      // данные из стека возврата
      // ЭТО ПОКА НЕ ТРОГАТЬ!!!!
//      IntelStackWalk(fp,cn,DumpHeader.DumpSize,DumpHeader.CountStack);
//_SVS(SysLog("End"));
      DumpHeader.RecordsCount+=DumpHeader.CountStack;
    }

    // исключения
    if((DumpType&FLOG_EXCEPTION) && xpn)
    {
      xpn=xp->ExceptionRecord;
      Excpt.TypeRec=RTYPE_EXCEPTION;
      while(xpn)
      {
        char FaultingModule[MAX_PATH];
        Excpt.SizeRec=sizeof(Excpt)-sizeof(struct RECHEADER);
        Excpt.ExceptionCode=xpn->ExceptionCode;    // код исключения
        Excpt.ExceptionFlags=xpn->ExceptionFlags;   // флаги исключения
        Excpt.ExceptionAddress=(DWORD)xpn->ExceptionAddress; // адрес исключения
        Excpt.NumberParameters=xpn->NumberParameters; // количество параметров доп. информации
        memcpy(Excpt.ExceptionInformation, // доп. информация
               xpn->ExceptionInformation,
               sizeof(Excpt.ExceptionInformation));
               //(sizeof(xpn->ExceptionInformation)/sizeof(xpn->ExceptionInformation[0]))*xpn->NumberParameters);

        if(xpn->ExceptionAddress)
        {
          if(GetLogicalAddress(xpn->ExceptionAddress, FaultingModule,sizeof(FaultingModule),Excpt.Section, Excpt.Offset))
            Excpt.SizeModuleName=strlen(FaultingModule);
        }
        Excpt.SizeRec+=Excpt.SizeModuleName;

        WriteFile(fp,&Excpt,sizeof(Excpt),&Temp,NULL);
        if(Excpt.SizeModuleName)
          WriteFile(fp,FaultingModule,Excpt.SizeModuleName,&Temp,NULL);

        Excpt.CurItem++;
        DumpHeader.DumpSize+=Excpt.SizeRec+sizeof(struct RECHEADER);
        xpn=xpn->ExceptionRecord;
      }
    }
    DumpHeader.CountException=Excpt.CurItem;
    DumpHeader.RecordsCount+=DumpHeader.CountException;

    // *** баговый кусок в дамп.
    if((DumpType&FLOG_FAULTCODE))
    {
      xpn=xp->ExceptionRecord;
      if(xpn && xpn->ExceptionAddress)
      {
        struct FAULTCODERECORD FuiltCode;   // кусок кода
        FuiltCode.TypeRec=RTYPE_FAULTCODE;
        FuiltCode.SizeRec=sizeof(FuiltCode)-sizeof(struct RECHEADER);
        FuiltCode.ExceptionAddress=(DWORD)xpn->ExceptionAddress;
        if(Excpt.ExceptionAddress)
        {
          if(!ReadProcessMemory(GetCurrentProcess(),
                (LPCVOID)((BYTE*)(xpn->ExceptionAddress)-32),
                FuiltCode.Code,sizeof(FuiltCode.Code),&Temp))
            memset(FuiltCode.Code,0,sizeof(FuiltCode.Code));
        }
        WriteFile(fp,&FuiltCode,sizeof(FuiltCode),&Temp,NULL);

        DumpHeader.RecordsCount++;
        DumpHeader.DumpSize+=FuiltCode.SizeRec+sizeof(struct RECHEADER);
      }
    }

  }

  // Про макросы
  if((DumpType&FLOG_MACRO) && CtrlObject)
  {
    struct MacroRecord RBuf;
    struct MACRORECORD MacroRec;
    int KeyPos;
    MacroRec.TypeRec=RTYPE_MACRO;
    MacroRec.SizeRec=sizeof(struct MACRORECORD)-sizeof(struct RECHEADER);
    MacroRec.MacroStatus=CtrlObject->Macro.GetCurRecord(&RBuf,&KeyPos);
    MacroRec.MacroPos=KeyPos;
    MacroRec.MacroFlags=RBuf.Flags;
    MacroRec.MacroKey=RBuf.Key;
    MacroRec.MacroBufferSize=RBuf.BufferSize;
    MacroRec.MacroKeyBuffer[0]=RBuf.Buffer?RBuf.Buffer[0]:0;
    MacroRec.SizeRec+=sizeof(DWORD)*(MacroRec.MacroBufferSize-1);

    WriteFile(fp,&MacroRec,sizeof(MacroRec),&Temp,NULL);
    if(MacroRec.MacroStatus && RBuf.Buffer && MacroRec.MacroBufferSize > 1)
      WriteFile(fp,&RBuf.Buffer[1],(MacroRec.MacroBufferSize-1)*sizeof(DWORD),&Temp,NULL);

    DumpHeader.RecordsCount++;
    DumpHeader.DumpSize+=MacroRec.SizeRec+sizeof(struct RECHEADER);
  }

  if((DumpType&FLOG_RAWDARA) && RawData && RawDataSize)
  {
    struct RAWDARARECORD RawDataRec;
    RawDataRec.TypeRec=RTYPE_RAWDARA;
    RawDataRec.SizeRec=RawDataSize+(sizeof(struct RAWDARARECORD)-sizeof(struct RECHEADER));
    RawDataRec.RawFlags=RawDataFlags;

    WriteFile(fp,&RawDataRec,sizeof(RawDataRec),&Temp,NULL);
    if(RawDataSize)
      WriteFile(fp,RawData,RawDataSize,&Temp,NULL);

    DumpHeader.RecordsCount++;
    DumpHeader.DumpSize+=RawDataRec.SizeRec+sizeof(struct RECHEADER);
  }


  // "где мы сейчас находимся?"
  if((DumpType&FLOG_FARAREA) && CtrlObject)
  {
    struct FARAREARECORD FarArea;
    memset(&FarArea,0,sizeof(FarArea));
    FarArea.TypeRec=RTYPE_FARAREA;
    FarArea.SizeRec=sizeof(struct FARAREARECORD)-sizeof(struct RECHEADER);

    // Тип объекта (где мы сейчас?)
    FarArea.ObjectType=FrameManager->GetCurrentFrame()->GetType();

    // Пока формат не устаканился дописывать остальное, не забывая
    // ....

    WriteFile(fp,&FarArea,sizeof(FarArea),&Temp,NULL);
    DumpHeader.RecordsCount++;
    DumpHeader.DumpSize+=FarArea.SizeRec+sizeof(struct RECHEADER);
  }

  if(CallBackProc)
  {
    int Iteration=0;
    while(CallBackProc(fp,&DumpHeader.DumpSize,Iteration++))
      DumpHeader.RecordsCount++;
  }

  // Вернемся для окончательной записи главного заголовка
  SetFilePointer(fp,BeginEventFilePos,NULL,FILE_BEGIN);
  WriteFile(fp,&DumpHeader,sizeof(DumpHeader),&Temp,NULL);
  CloseHandle(fp);
  return DumpHeader.RecordsCount;//EXCEPTION_EXECUTE_HANDLER; //??
}


static int WritePLUGINRECORD(HANDLE fp,struct PluginItem *Module,DWORD *DumpSize)
{
  DWORD Temp;
  struct PLUGINRECORD Plug;
  memset(&Plug,0,sizeof(Plug));
  Plug.TypeRec=RTYPE_PLUGIN;
  Plug.SizeRec=sizeof(struct PLUGINRECORD)-sizeof(struct RECHEADER);
  Plug.hModule=Module->hModule;
  memcpy(&Plug.FindData,&Module->FindData,sizeof(WIN32_FIND_DATA));
  int Len=strlen(Plug.FindData.cFileName);
  memset(&Plug.FindData.cFileName[Len],0,sizeof(Plug.FindData.cFileName)-Len); //??
  Plug.SysID=Module->SysID;
  Plug.Cached=Module->Cached;
  Plug.CachePos=Module->CachePos;
  Plug.EditorPlugin=Module->EditorPlugin;
  Plug.DontLoadAgain=Module->DontLoadAgain;
  Plug.pSetStartupInfo=Module->pSetStartupInfo;
  Plug.pOpenPlugin=Module->pOpenPlugin;
  Plug.pOpenFilePlugin=Module->pOpenFilePlugin;
  Plug.pClosePlugin=Module->pClosePlugin;
  Plug.pGetPluginInfo=Module->pGetPluginInfo;
  Plug.pGetOpenPluginInfo=Module->pGetOpenPluginInfo;
  Plug.pGetFindData=Module->pGetFindData;
  Plug.pFreeFindData=Module->pFreeFindData;
  Plug.pGetVirtualFindData=Module->pGetVirtualFindData;
  Plug.pFreeVirtualFindData=Module->pFreeVirtualFindData;
  Plug.pSetDirectory=Module->pSetDirectory;
  Plug.pGetFiles=Module->pGetFiles;
  Plug.pPutFiles=Module->pPutFiles;
  Plug.pDeleteFiles=Module->pDeleteFiles;
  Plug.pMakeDirectory=Module->pMakeDirectory;
  Plug.pProcessHostFile=Module->pProcessHostFile;
  Plug.pSetFindList=Module->pSetFindList;
  Plug.pConfigure=Module->pConfigure;
  Plug.pExitFAR=Module->pExitFAR;
  Plug.pProcessKey=Module->pProcessKey;
  Plug.pProcessEvent=Module->pProcessEvent;
  Plug.pProcessEditorEvent=Module->pProcessEditorEvent;
  Plug.pCompare=Module->pCompare;
  Plug.pProcessEditorInput=Module->pProcessEditorInput;
  Plug.pMinFarVersion=Module->pMinFarVersion;
  Plug.pProcessViewerEvent=Module->pProcessViewerEvent;

  Plug.SizeModuleName=strlen(Module->ModuleName);
  Plug.SizeRec+=Plug.SizeModuleName;
  WriteFile(fp,&Plug,sizeof(Plug),&Temp,NULL);
  if(Plug.SizeModuleName)
    WriteFile(fp,Module->ModuleName,Plug.SizeModuleName,&Temp,NULL);

  *DumpSize+=Plug.SizeRec+sizeof(struct RECHEADER);
  return 1;
}


//==============================================================================
// Given a linear address, locates the module, section, and offset containing
// that address.
//
// Note: the szModule paramater buffer is an output buffer of length specified
// by the len parameter (in characters!)
//==============================================================================
static BOOL GetLogicalAddress(void* addr, char *szModule, DWORD len, DWORD& section, DWORD& offset)
{
  MEMORY_BASIC_INFORMATION mbi;
  section=offset=0;

  if ( !VirtualQuery( addr, &mbi, sizeof(mbi) ) )
      return FALSE;

  DWORD hMod = (DWORD)mbi.AllocationBase;

  if ( !GetModuleFileName( (HMODULE)hMod, szModule, len ) )
      return FALSE;
//_SVS(SysLog("hMod=0x%08X %s",hMod,szModule));

  // Point to the DOS header in memory
  PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;
  // From the DOS header, find the NT (PE) header
  PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);
  PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );
  DWORD rva = (DWORD)addr - hMod; // RVA is offset from module load address

  // Iterate through the section table, looking for the one that encompasses
  // the linear address.
  for (unsigned i = 0;
          i < pNtHdr->FileHeader.NumberOfSections;
          i++, pSection++ )
  {
      DWORD sectionStart = pSection->VirtualAddress;
      DWORD sectionEnd = sectionStart
                  + max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

      // Is the address in this section???
      if ( (rva >= sectionStart) && (rva <= sectionEnd) )
      {
          // Yes, address is in the section.  Calculate section and offset,
          // and store in the "section" & "offset" params, which were
          // passed by reference.
          section = i+1;
          offset = rva - sectionStart;
          return TRUE;
      }
  }
  return FALSE;   // Should never get here!
}

#if 0
//============================================================
// Walks the stack, and writes the results to the report file
//============================================================
static void IntelStackWalk(HANDLE fp,PCONTEXT pContext, DWORD& SizeOfRecord, DWORD& StackCount)
{
  // Call stack:
  struct STACKRECORD StackRecord;

  memset(&StackRecord,0,sizeof(StackRecord));
  StackRecord.TypeRec=RTYPE_STACK;
  StackRecord.SizeRec=sizeof(StackRecord)-sizeof(struct RECHEADER);

  DWORD pc = pContext->Eip;
  PDWORD pFrame, pPrevFrame;
  DWORD Temp;

  pFrame = (PDWORD)pContext->Ebp;
  do
  {
    char ModuleName[MAX_PATH]="";

    if(!GetLogicalAddress((PVOID)pc, ModuleName,sizeof(ModuleName),StackRecord.Section,StackRecord.Offset))
      break;
_SVS(SysLog("%4d) pc=0x%08X %s",StackRecord.CurItem,pc,ModuleName));
    StackRecord.EIP=(DWORD)pc;
    StackRecord.EBP=(DWORD)pFrame;
    StackRecord.SizeModuleName=strlen(ModuleName);
    if(StackRecord.SizeModuleName)
      StackRecord.SizeRec+=StackRecord.SizeModuleName; // скорректируем размер

    WriteFile(fp,&StackRecord,sizeof(StackRecord),&Temp,NULL);
    if(StackRecord.SizeModuleName)
      WriteFile(fp,ModuleName,StackRecord.SizeModuleName,&Temp,NULL);

    StackRecord.CurItem++;
    // корректирем общий размер эвента
    SizeOfRecord+=StackRecord.SizeRec+sizeof(struct RECHEADER);

    pc = pFrame[1];
    pPrevFrame = pFrame;
    pFrame = (PDWORD)pFrame[0]; // precede to next higher frame on stack

    if ( (DWORD)pFrame & 3 )    // Frame pointer must be aligned on a
        break;                  // DWORD boundary.  Bail if not so.

    if ( pFrame <= pPrevFrame )
        break;

    // Can two DWORDs be read from the supposed frame address?
    if ( IsBadReadPtr (pFrame, sizeof(PVOID)*2) ) //???? IsBadWritePtr
      break;
  } while ( 1 );

  // коррекция количества стеков
  StackCount=StackRecord.CurItem;
}
#endif


/* ************************************************************************
   $ 16.10.2000 SVS
   Простенький обработчик исключений.
*/
static char* xFromMSGTitle(int From)
{
  if(From == EXCEPT_SETSTARTUPINFO || From == EXCEPT_MINFARVERSION)
    return MSG(MExceptTitleLoad);
  else
    return MSG(MExceptTitle);
}

int xfilter(
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
   char *pName;
   int  I, rc, Ret=1;
   char Buf[2][80];
   char TruncFileName[2*NM];
   BOOL Unload = FALSE; // Установить в истину, если плагин нужно выгрузить

   // получим запись исключения
   EXCEPTION_RECORD *xr = xp->ExceptionRecord;

   // выведим дамп перед выдачей сообщений
   if (xr->ExceptionCode != STATUS_INVALIDFUNCTIONRESULT)
     WriteEvent(FLOG_ALL&(~FLOG_PLUGINSINFO),xp,Module,NULL,0);


   // CONTEXT можно использовать для отображения или записи в лог
   //         содержимого регистров...
   // CONTEXT *xc = xp->ContextRecord;

   rc = EXCEPTION_EXECUTE_HANDLER;

   /*$ 23.01.2001 skv
     Неизвестное исключение не стоит игнорировать.
   */
   pName=NULL;
   strcpy(TruncFileName,NullToEmpty(Module->ModuleName));

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

     Ret=Message(MSG_WARNING,
            (Opt.ExceptCallDebugger?2:1),
            xFromMSGTitle(From),
            MSG(MExcTrappedException),
            MSG(MExcCheckOnLousys),
            TruncPathStr(TruncFileName,40),
            Buf[0],
            "\1",
            MSG(MExcUnloadYes),
            (Opt.ExceptCallDebugger?MSG(MExcDebugger):MSG(MOk)),
            (Opt.ExceptCallDebugger?MSG(MOk):NULL));

     if (!Opt.ExceptCallDebugger || Ret!=0)
       Unload = TRUE;
//       CtrlObject->Plugins.UnloadPlugin(*Module);
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
     }

     sprintf(Buf[0],MSG(MExcInvalidFuncResult),pName);
     Ret=Message(MSG_WARNING, 2,
                 xFromMSGTitle(From),
                 MSG(MExcTrappedException),
                 MSG(MExcCheckOnLousys),
                 TruncPathStr(TruncFileName,40),
                 Buf[0],
                 "\1",
                 MSG(MExcUnload),
                 MSG(MYes), MSG(MNo));
     if (Ret == 0)
     {
       Unload = TRUE;
       Ret++; // Исключить вызов дебаггера при Ret == 0
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
           sprintf(Buf[1],MSG(xr->ExceptionInformation[0]+MExcRAccess),xr->ExceptionInformation[1]);
           pName=Buf[1];
         }
         break;
       }

     if (!pName) pName=MSG(MExcUnknown);

     sprintf(Buf[0],MSG(MExcAddress),xr->ExceptionAddress);
     if (Flags&1)
     {
       Ret=Message(MSG_WARNING,(Opt.ExceptCallDebugger?3:2),
               xFromMSGTitle(From),
               MSG(MExcTrappedException),
               pName,
               Buf[0],
               TruncPathStr(TruncFileName,40),"\1",
               MSG(MExcUnload),
               (Opt.ExceptCallDebugger?MSG(MExcDebugger):MSG(MYes)),
               (Opt.ExceptCallDebugger?MSG(MYes):MSG(MNo)),
               (Opt.ExceptCallDebugger?MSG(MNo):NULL));
       if ((Opt.ExceptCallDebugger && Ret == 1) ||
           (!Opt.ExceptCallDebugger && Ret == 0))
         Unload = TRUE; // CtrlObject->Plugins.UnloadPlugin(*Module);
     }
     else
       Ret=Message(MSG_WARNING,(Opt.ExceptCallDebugger?2:1),
               xFromMSGTitle(From),
               MSG(MExcTrappedException),
               pName,
               Buf[0],
               TruncPathStr(TruncFileName,40),"\1",
               MSG(MExcUnloadYes),
               (Opt.ExceptCallDebugger?MSG(MExcDebugger):MSG(MOk)),
               (Opt.ExceptCallDebugger?MSG(MOk):NULL));
     /* skv$*/
   } /* else */

   if (Unload)
     CtrlObject->Plugins.UnloadPlugin(*Module);

   if (Opt.ExceptCallDebugger && Ret==0)
     rc = EXCEPTION_CONTINUE_SEARCH;
   else
     rc = EXCEPTION_EXECUTE_HANDLER;
   /* VVM $ */

   if(xr->ExceptionFlags&EXCEPTION_NONCONTINUABLE)
     rc=EXCEPTION_CONTINUE_SEARCH; //?
   return rc;
}
/* SVS $ */
