/*
syslog.cpp

Системный отладочный лог :-)

*/

/* Revision: 1.00 22.12.2000 $ */

/*
Modify:
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
