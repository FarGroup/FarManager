/*
syslog.cpp

Системный отладочный лог :-)

*/

/* Revision: 1.01 23.01.2001 $ */

/*
Modify:
  23.01.2001 SVS
    + DumpExeptionInfo()
  22.12.2000 SVS
    + Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop
#include "internalheaders.hpp"

#if defined(SYSLOG)
char         LogFileName[MAX_FILE];
static FILE *LogStream=0;
static int   Indent=0;
#endif

FILE * OpenLogStream(char *file)
{
#if defined(SYSLOG)
    time_t t;
    struct tm *time_now;
    char RealLogName[MAX_FILE];
//    char rfile[MAX_FILE];

    time(&t);
    time_now=localtime(&t);

    strftime(RealLogName,MAX_FILE,file,time_now);
    return _fsopen(RealLogName,"a+t",SH_DENYWR);
#else
    return NULL;
#endif
}

void OpenSysLog()
{
#if defined(SYSLOG)
    if ( LogStream )
        fclose(LogStream);

    GetModuleFileName(NULL,LogFileName,sizeof(LogFileName));
    strcpy(strrchr(LogFileName,'\\')+1,"far.log");
    LogStream=OpenLogStream(LogFileName);
    //if ( !LogStream )
    //{
    //    fprintf(stderr,"Can't open log file '%s'\n",LogFileName);
    //}
#endif
}

void CloseSysLog(void)
{
#if defined(SYSLOG)
    fclose(LogStream);
    LogStream=0;
#endif
}

void SysLog(int i)
{
#if defined(SYSLOG)
    Indent+=i;
    if ( Indent<0 )
        Indent=0;
#endif
}

void SysLog(char *fmt,...)
{
#if defined(SYSLOG)
    char spaces[]="                                                                                                                                                       ";
    char msg[MAX_LOG_LINE];
    time_t t;
    struct tm *tm;
    char timebuf[64];

    va_list argptr;
    va_start( argptr, fmt );

    vsprintf( msg, fmt, argptr );
    va_end(argptr);

    time (&t);
    tm = localtime (&t);
    strftime (timebuf, sizeof (timebuf), "%d.%m.%Y %H:%M:%S", tm);

    OpenSysLog();
    if ( LogStream )
    {
        spaces[Indent]=0;
        fprintf(LogStream,"%s %s %s\n",timebuf,spaces,msg);
        fflush(LogStream);
    }
    CloseSysLog();
#endif
}


int DumpExeptionInfo(EXCEPTION_POINTERS *xp)
{
#if defined(SYSLOG)
  if(xp)
  {
    static char DumpExcptFileName[MAX_FILE];
    GetModuleFileName(NULL,DumpExcptFileName,sizeof(DumpExcptFileName));
    strcpy(strrchr(DumpExcptFileName,'\\')+1,"farexcpt.log");
    FILE *fp=fopen(DumpExcptFileName,"a+t");
    if(fp)
    {
      time_t t;
      struct tm *tm;
      char timebuf[64];

      CONTEXT *cn=xp->ContextRecord;
      struct _EXCEPTION_RECORD *xpn=xp->ExceptionRecord;
      time (&t);
      tm = localtime (&t);
      strftime (timebuf, sizeof (timebuf), "\n-----------------\n%d.%m.%Y %H:%M:%S\n", tm);

      while(xpn)
      {
        fprintf(fp,timebuf);
        fprintf(fp,"Exception Code=0x%08X %s\nException Address=0x%08X\nException Flags=0x%08X (%scontinuable farexcpt.\n",
                xpn->ExceptionCode,
                ((xpn->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)?
                  "(Exception Access Violation)":""),
                xpn->ExceptionAddress,
                xpn->ExceptionFlags,
                (xpn->ExceptionFlags&EXCEPTION_NONCONTINUABLE?"non":""));
        if(xpn->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
        {
          if(!xpn->ExceptionInformation[0])
            fprintf(fp,"Read data from 0x%08X\n",xpn->ExceptionInformation[1]);
          else
            fprintf(fp,"Write data to 0x%08X\n",xpn->ExceptionInformation[1]);
          if(xpn->NumberParameters > 2)
          {
            fprintf(fp,"Additional data:\n",xpn->ExceptionInformation[1]);
            for(int I=2; I < xpn->NumberParameters; ++I)
            {
              fprintf(fp,"0x%08 ",xpn->ExceptionInformation[I]);
            }
          }
        }
        fprintf(fp,"\n");
        xpn=xpn->ExceptionRecord;
      }
      fprintf(fp,"\nRegisters:\n");
      // This section is specified/returned if the
      // ContextFlags word contians the flag CONTEXT_CONTROL.
      if((cn->ContextFlags&CONTEXT_CONTROL))
      {
        fprintf(fp,"CS= 0x%08X EIP=0x%08X\nSS =0x%08X ESP=0x%08X EBP=0x%08X\nEFLAGS=0x%08X\n",
          cn->SegCs,cn->Eip,cn->SegSs,cn->Esp,cn->Ebp,cn->EFlags);
      }
      // This section is specified/returned if the
      // ContextFlags word contians the flag CONTEXT_SEGMENTS.
      if((cn->ContextFlags&CONTEXT_SEGMENTS))
      {
        fprintf(fp,"GS =0x%08X FS =0x%08X ES =0x%08X DS =0x%08X\n",
           cn->SegGs,cn->SegFs,cn->SegEs,cn->SegDs);
      }
      // This section is specified/returned if the
      // ContextFlags word contians the flag CONTEXT_INTEGER.
      if((cn->ContextFlags&CONTEXT_SEGMENTS))
      {
        fprintf(fp,"EDI=0x%08X ESI=0x%08X EBX=0x%08X\nEDX=0x%08X ECX=0x%08X EAX=0x%08X\n",
           cn->Edi,cn->Esi,cn->Ebx,cn->Edx,cn->Ecx,cn->Eax);
      }
      // This section is specified/returned if CONTEXT_DEBUG_REGISTERS is
      // set in ContextFlags.  Note that CONTEXT_DEBUG_REGISTERS is NOT
      // included in CONTEXT_FULL.
      if((cn->ContextFlags&CONTEXT_DEBUG_REGISTERS) &&
         (cn->ContextFlags&CONTEXT_FULL) != CONTEXT_FULL)
        fprintf(fp,"Dr0=0x%08X Dr1=0x%08X Dr2=0x%08X\nDr3=0x%08X Dr6=0x%08X Dr7=0x%08X\n",
           cn->Dr0,cn->Dr1,cn->Dr2,cn->Dr3,cn->Dr6,cn->Dr7);

      // This section is specified/returned if the
      // ContextFlags word contians the flag CONTEXT_FLOATING_POINT.
      //
      if((cn->ContextFlags&CONTEXT_DEBUG_REGISTERS))
      {
      // FLOATING_SAVE_AREA FloatSave;
      }
      fclose(fp);
    }
  }
#endif
  return EXCEPTION_EXECUTE_HANDLER;
}
