/*
syslog.cpp

Системный отладочный лог :-)

*/

/* Revision: 1.10 25.06.2001 $ */

/*
Modify:
  25.06.2001 SVS
    ! Заюзаем внутреннюю функцию StrFTime вместо стандартной.
  16.05.2001 SVS
    ! DumpExceptionInfo -> farexcpt.cpp
    + _SYSLOG_KM()
  09.05.2001 OT
    ! Макросы, аналогичные _D(x), которые зависят от разработчика или функционала
  07.05.2001 SVS
    + В DumpExceptionInfo добавлена версия ФАРа в формате FAR_VERSION
  06.05.2001 DJ
    ! перетрях #include
  06.05.2001 SVS
    ! немного изменений в SysLog* ;-)
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  28.04.2001 SVS
    - Неверно выставлен флаг, вместо теста CONTEXT_INTEGER стояло
      CONTEXT_SEGMENTS
    ! Полная переделка функций семейства SysLog()
    + Функция печати дампа памяти SysLogDump()
      Если указатель на файл = NULL, то будет писаться в стандартный лог,
      иначе в указанный открытый файл.
  26.01.2001 SVS
    ! DumpExeptionInfo -> DumpExceptionInfo ;-)
  23.01.2001 SVS
    + DumpExeptionInfo()
  22.12.2000 SVS
    + Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "plugin.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "plugins.hpp"

#if !defined(SYSLOG)
 #if defined(SYSLOG_OT) || defined(SYSLOG_SVS) || defined(SYSLOG_DJ) || defined(VVM) || defined(SYSLOG_AT) || defined(SYSLOG_IS) || defined(SYSLOG_tran) || defined(SYSLOG_SKV) || defined(SYSLOG_NWZ) || defined(SYSLOG_KM)
  #define SYSLOG
 #endif
#endif

#if defined(SYSLOG)
char         LogFileName[MAX_FILE];
static FILE *LogStream=0;
static int   Indent=0;
static char *PrintTime(char *timebuf);
#endif

#if defined(SYSLOG)
char *MakeSpace(void)
{
  static char Buf[60]=" ";
  if(Indent)
    memset(Buf+1,0xB3,Indent);
  Buf[1+Indent]=0;
  return Buf;
}
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

    StrFTime(RealLogName,MAX_FILE,file,time_now);
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

#if defined(SYSLOG)
static char *PrintTime(char *timebuf)
{
  SYSTEMTIME st;
  GetLocalTime(&st);
  sprintf(timebuf,"%02d.%02d.%04d %2d:%02d:%02d.%04d",
      st.wDay,st.wMonth,st.wYear,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
  return timebuf;
}
#endif

void SysLog(char *fmt,...)
{
#if defined(SYSLOG)
  char msg[MAX_LOG_LINE];

  va_list argptr;
  va_start( argptr, fmt );

  vsprintf( msg, fmt, argptr );
  va_end(argptr);

  OpenSysLog();
  if ( LogStream )
  {
    char timebuf[64];
    fprintf(LogStream,"%s %s%s\n",PrintTime(timebuf),MakeSpace(),msg);
    fflush(LogStream);
  }
  CloseSysLog();
#endif
}

///
void SysLog(int l,char *fmt,...)
{
#if defined(SYSLOG)
    char msg[MAX_LOG_LINE];

    va_list argptr;
    va_start( argptr, fmt );

    vsprintf( msg, fmt, argptr );
    va_end(argptr);

    SysLog(l);
    OpenSysLog();
    if ( LogStream )
    {
      char timebuf[64];
      fprintf(LogStream,"%s %s%s\n",PrintTime(timebuf),MakeSpace(),msg);
      fflush(LogStream);
    }
    CloseSysLog();
#endif
}
///

void SysLogDump(char *Title,DWORD StartAddress,LPBYTE Buf,int SizeBuf,FILE *fp)
{
#if defined(SYSLOG)
  int CY=(SizeBuf+15)/16;
  int X,Y;
  int InternalLog=fp==NULL?TRUE:FALSE;

//  char msg[MAX_LOG_LINE];

  if(InternalLog)
  {
    OpenSysLog();
    fp=LogStream;
    if(fp)
    {
      char timebuf[64];
      fprintf(fp,"%s %s(%s)\n",PrintTime(timebuf),MakeSpace(),NullToEmpty(Title));
    }
  }

  if (fp)
  {
    char TmpBuf[17];
    int I;
    TmpBuf[16]=0;
    if(!InternalLog && Title && *Title)
      fprintf(fp,"%s\n",Title);
    for(Y=0; Y < CY; ++Y)
    {
      memset(TmpBuf,' ',16);
      fprintf(fp, " %08X: ",StartAddress+Y*16);
      for(X=0; X < 16; ++X)
      {
        if((I=Y*16+X < SizeBuf) != 0)
          fprintf(fp,"%02X ",Buf[Y*16+X]&0xFF);
        else
          fprintf(fp,"   ");
        TmpBuf[X]=I?(Buf[Y*16+X] < 32?'.':Buf[Y*16+X]):' ';
        if(X == 7)
          fprintf(fp, " ");
      }
      fprintf(fp,"| %s\n",TmpBuf);
    }
    fprintf(fp,"\n");
    fflush(fp);
  }

  if(InternalLog)
    CloseSysLog();
#endif
}
