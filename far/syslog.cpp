/*
syslog.cpp

Системный отладочный лог :-)

*/

/* Revision: 1.41 16.04.2003 $ */

/*
Modify:
  16.04.2003 SVS
    + Логирование DM_GETSELECTION и DM_SETSELECTION.
  31.03.2003 SVS
    + _EE_ToName(), _EEREDRAW_ToName()
    + _SYS_EE_REDRAW
  05.03.2003 SVS
    + SYSLOG_COPYR
  18.02.2003 SVS
    + _ESPT_ToName + _SysLog_LinearDump
    + DEF_ECTL_(DELETEBLOCK)
  11.02.2003 SVS
    ! Немного красоты в SysLog.
  21.01.2003 SVS
    + xf_malloc,xf_realloc,xf_free - обертки вокруг malloc,realloc,free
      Просьба блюсти порядок и прописывать именно xf_* вместо простых.
    + INPUT_RECORD_DumpBuffer() - дамп оставшихся эвентов в консольной очереди
    + IsLogON() - теперь любые логи будут писаться (только в дебажной версии)
      если включен ScrollLock
  10.12.2002 SVS
    + ManagerClass_Dump()
  04.11.2002 SVS
    + Для отладочных целей, для плагинов - FarSysLog_INPUT_RECORD_Dump()
      (доступно только под дебугинфой)
  23.08.2002 SVS
    + SaveScreenDumpBuffer()
  02.07.2002 SVS
    - ошибки в _VCTL_ToName
    + _PluginsStackItem_Dump() - дамп стека плагинов
  25.06.2002 SVS
    - Ошибки проектирования в _INPUT_RECORD_Dump
  24.05.2002 SVS
    + _INPUT_RECORD_Dump - вывод в лог информацию о INPUT_RECORD
  22.05.2002 SVS
    + _VCTL_ToName
  05.05.2002 SVS
    ! Немного усовершенствуем лог по хипу
  04.04.2002 SVS
    ! ECTL_TURNOFFMARKINGBLOK -> ECTL_TURNOFFMARKINGBLOCK
  04.04.2002 IS
    + ECTL_TURNOFFMARKINGBLOK в _ECTL_ToName
  04.04.2002 SVS
    + DN_ACTIVATEAPP
    + _ACTL_ToName()
  13.02.2002 SVS
    + SysLogLastError() - вывод в лог результата GetLastError()
  11.02.2002 SVS
    ! dialog.cpp::MsgToName() - syslog.cpp::_DLGMSG_ToName()
    ! Уточнения в _*_ToName()
  05.02.2002 SVS
    + Пара новых функций для отладочных мероприятий
       _FARKEY_ToName() - формирует строку из HEX-кода клавиши и названия онной
       _VK_KEY_ToName() - то же самое. но для виртуального кода.
    ! ECTLToName переимована в _ECTL_ToName
    + _FCTL_ToName()
  10.01.2002 SVS
    + SYSLOG_ECTL
  24.12.2001 SVS
    + Добавим в функции FarSysLog() в LOG-файл имя модуля.
  15.10.2001 SVS
    + Экспортируемые FarSysLog и FarSysLogDump только под SYSLOG_FARSYSLOG
  03.10.2001 SVS
    ! В некоторых источниках говорится, что IsDebuggerPresent() есть только
      в NT, так что... бум юзать ее динамически!
  24.09.2001 SVS
    ! CleverSysLog - параметр у конструктора - "заголовок"
  18.09.2001 SVS
    + класс CleverSysLog - что бы при выходе из функции делал SysLog(-1)
  15.08.2001 OT
    - "Перевод строки" в дебагере среды VC
  25.07.2001 SVS
    - Обломы с компиляцией под VC
  24.07.2001 SVS
    + Если запущены под дебагером (IsDebuggerPresent), то выводим строку
      в LOG-окно дебагера (OutputDebugString)
      Внимание! Винды ниже 98-х не подойдут.
  04.07.2001 SVS
    ! очередное уточнение LOG-файла :-)
    + Функции про хип
  27.06.2001 SVS
    ! Небольшая переделка LOG-файла :-)
      Теперь файлы складывается в каталог %FAR%\$Log и имеют имя
      Far.YYYYMMDD.BILD.log - BILD=%05d
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
#include "filelist.hpp"
#include "manager.hpp"
#include "frame.hpp"

#if !defined(SYSLOG)
 #if defined(SYSLOG_OT) || defined(SYSLOG_SVS) || defined(SYSLOG_DJ) || defined(VVM) || defined(SYSLOG_AT) || defined(SYSLOG_IS) || defined(SYSLOG_tran) || defined(SYSLOG_SKV) || defined(SYSLOG_NWZ) || defined(SYSLOG_KM) || defined(SYSLOG_KEYMACRO) || defined(SYSLOG_ECTL) || defined(SYSLOG_COPYR) || defined(SYSLOG_EE_REDRAW)
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
static BOOL IsLogON(void)
{
  return GetKeyState(VK_SCROLL)?TRUE:FALSE;
}
#endif

#if defined(SYSLOG)
static char *MakeSpace(void)
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
  char RealLogName[NM*2];
  SYSTEMTIME st;

  GetLocalTime(&st);
  sprintf(RealLogName,"%s\\Far.%04d%02d%02d.%05d.log",
      file,st.wYear,st.wMonth,st.wDay,HIWORD(FAR_VERSION));
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
  DWORD Attr;

  GetModuleFileName(NULL,LogFileName,sizeof(LogFileName));
  char *Ptr=strrchr(LogFileName,'\\');
  strcpy(Ptr,"\\$Log");
  Attr=GetFileAttributes(LogFileName);
  if(Attr == -1)
  {
    if(!CreateDirectory(LogFileName,NULL))
      *Ptr=0;
  }
  else if(!(Attr&FILE_ATTRIBUTE_DIRECTORY))
    *Ptr=0;
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

void ShowHeap()
{
#if defined(SYSLOG) && defined(HEAPLOG)
  if(!IsLogON())
    return;

  OpenSysLog();
  if ( LogStream)
  {
    char timebuf[64];
    fprintf(LogStream,"%s %s%s\n",PrintTime(timebuf),MakeSpace(),"Heap Status");

    fprintf(LogStream,"   Size   Status\n");
    fprintf(LogStream,"   ----   ------\n");

    DWORD Sz=0;
    _HEAPINFO hi;
    hi._pentry=NULL;
    //    int     *__pentry;
    while( _rtl_heapwalk( &hi ) == _HEAPOK )
    {
      fprintf(LogStream,"%7u    %s  (%p)\n", hi._size, (hi._useflag ? "used" : "free"),hi.__pentry);
      Sz+=hi._useflag?hi._size:0;
    }
    fprintf(LogStream,"   ----   ------\n" );
    fprintf(LogStream,"%7u      \n", Sz);

    fflush(LogStream);
  }
  CloseSysLog();
#endif
}


void CheckHeap(int NumLine)
{
#if defined(SYSLOG) && defined(HEAPLOG)
  if(!IsLogON())
    return;

  int HeapStatus=_heapchk();
  if (HeapStatus ==_HEAPBADNODE)
  {
    SysLog("Error: Heap broken, Line=%d",NumLine);
  }
  else if (HeapStatus < 0)
  {
    SysLog("Error: Heap corrupt, Line=%d, HeapStatus=%d",NumLine,HeapStatus);
  }
  else
    SysLog("Heap OK, HeapStatus=%d",HeapStatus);
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
//  sprintf(timebuf,"%02d.%02d.%04d %2d:%02d:%02d.%03d",
//      st.wDay,st.wMonth,st.wYear,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
  sprintf(timebuf,"%02d:%02d:%02d.%03d",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
  return timebuf;
}
#endif

void SysLog(char *fmt,...)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

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
  if(pIsDebuggerPresent && pIsDebuggerPresent())
  {
    OutputDebugString(msg);
#ifdef _MSC_VER
    OutputDebugString("\n");
#endif _MSC_VER
  }
#endif
}

void SysLogLastError(void)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

  LPSTR lpMsgBuf;

  DWORD LastErr=GetLastError();
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,LastErr,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,0,NULL);

  OpenSysLog();
  if (LogStream)
  {
    char timebuf[64];
    RemoveUnprintableCharacters(lpMsgBuf);
    fprintf(LogStream,"%s %sGetLastError()=[%d/0x%X] \"%s\"\n",PrintTime(timebuf),MakeSpace(),LastErr,LastErr,lpMsgBuf);
    fflush(LogStream);
  }
  CloseSysLog();
  if(pIsDebuggerPresent && pIsDebuggerPresent())
  {
    OutputDebugString(lpMsgBuf);
#ifdef _MSC_VER
    OutputDebugString("\n");
#endif _MSC_VER
  }
  LocalFree(lpMsgBuf);
  SetLastError(LastErr);
#endif
}

///
void SysLog(int l,char *fmt,...)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

  char msg[MAX_LOG_LINE];

  va_list argptr;
  va_start( argptr, fmt );

  vsprintf( msg, fmt, argptr );
  va_end(argptr);

  OpenSysLog();
  if ( LogStream )
  {
    if(l < 0) SysLog(l);
    char timebuf[64];
    fprintf(LogStream,"%s %s%s\n",PrintTime(timebuf),MakeSpace(),msg);
    fflush(LogStream);
    if(l > 0) SysLog(l);
  }
  CloseSysLog();
  if(pIsDebuggerPresent && pIsDebuggerPresent())
  {
    OutputDebugString(msg);
#ifdef _MSC_VER
    OutputDebugString("\n");
#endif _MSC_VER
  }
#endif
}


void SysLogDump(char *Title,DWORD StartAddress,LPBYTE Buf,int SizeBuf,FILE *fp)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

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


void SaveScreenDumpBuffer(const char *Title,const CHAR_INFO *Buffer,int X1,int Y1,int X2,int Y2,int RealScreen,FILE *fp)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

  int InternalLog=fp==NULL?TRUE:FALSE;

  if(InternalLog)
  {
    OpenSysLog();
    fp=LogStream;
    if(fp)
    {
      char timebuf[64];
      fprintf(fp,"%s %s(CHAR_INFO DumpBuffer: '%s')\n",PrintTime(timebuf),MakeSpace(),NullToEmpty(Title));
    }
  }

  char *line=new char[X2-X1+4];
  if (fp && line)
  {
    int x,y,i;

    if(!InternalLog && Title && *Title)
      fprintf(fp,"CHAR_INFO DumpBuffer: %s\n",Title);

    fprintf(fp,"XY={%i,%i - %i,%i}, RealScreen=%i\n",X1,Y1,X2,Y2,RealScreen);

    for (y=Y1; y <= Y2; y++)
    {
      fprintf(fp,"%04d: ",y);
      for (i=0, x=X1; x <= X2; x++, ++i)
      {
        line[i]=Buffer->Char.AsciiChar?Buffer->Char.AsciiChar:' ';
        Buffer++;
      }
      line[i]=0;
      fprintf(fp,"%s\n",line);
    }
    fprintf(fp,"\n");
    fflush(fp);
  }

  if (line)
    delete[] line;

  if(InternalLog)
    CloseSysLog();
#endif
}



void PluginsStackItem_Dump(char *Title,const struct PluginsStackItem *StackItems,int ItemNumber,FILE *fp)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

  int InternalLog=fp==NULL?TRUE:FALSE;

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
    #define DEF_SORTMODE_(m) { m , #m }
    static struct SORTMode{
      int Mode;
      const char *Name;
    } __SORT[]={
      DEF_SORTMODE_(UNSORTED),  DEF_SORTMODE_(BY_NAME),  DEF_SORTMODE_(BY_EXT),
      DEF_SORTMODE_(BY_MTIME),  DEF_SORTMODE_(BY_CTIME), DEF_SORTMODE_(BY_ATIME),
      DEF_SORTMODE_(BY_SIZE),   DEF_SORTMODE_(BY_DIZ),   DEF_SORTMODE_(BY_OWNER),
      DEF_SORTMODE_(BY_COMPRESSEDSIZE),DEF_SORTMODE_(BY_NUMLINKS)
    };

    if(!StackItems || !ItemNumber)
      fprintf(fp,"\tPluginsStackItem <EMPTY>");
    else
    {
      for(int I=ItemNumber-1; I >= 0; --I)
        fprintf(fp,"\t[%d]: "
                  "hPlugin=%p "
                  "Modified=%s "
                  "PrevViewMode=VIEW_%02d "
                  "PrevSortMode=%d/%-17s "
                  "PrevSortOrder=%02d "
                  "HostFile=%s\n",
         I,
         StackItems[I].hPlugin,
         (StackItems[I].Modified?"True ":"False"),
         StackItems[I].PrevViewMode,
         StackItems[I].PrevSortMode,
           (StackItems[I].PrevSortMode<BY_NUMLINKS?__SORT[StackItems[I].PrevSortMode].Name:"<Unknown>"),
         StackItems[I].PrevSortOrder,
         StackItems[I].HostFile);
    }
    fprintf(fp,"\n");
    fflush(fp);
  }

  if(InternalLog)
    CloseSysLog();
#endif
}

void ManagerClass_Dump(char *Title,const Manager *m,FILE *fp)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

  int InternalLog=fp==NULL?TRUE:FALSE;

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
    const Manager *Man=(m==NULL?FrameManager:m);
//StartSysLog
    int I;
    char Type[NM],Name[NM*2];
    fprintf(fp,"**** Очередь модальных фреймов ***\nFrameListSize=%d, FramePos=%d\n",Man->FrameListSize,Man->FramePos);
    if(Man->FrameList)
    {
      for(I=0; I < Man->FrameCount; ++I)
      {
        if(Man->FrameList[I])
        {
          Man->FrameList[I]->GetTypeAndName(Type,Name);
          fprintf(fp,"\tFrameList[%d] %p  Type='%s' Name='%s'\n",
                      I,Man->FrameList[I],Type,Name);
        }
        else
          fprintf(fp,"\tFrameList[%d] NULL\n",I,Man->FrameList[I]);
      }
    }
    else
     fprintf(fp,"\tFrameList = NULL\n");

    fprintf(fp,"**** Стек модальных фреймов ***\nModalStackSize=%d\n",Man->ModalStackSize);
    if(Man->ModalStack)
    {
      for(I=0; I < Man->ModalStackCount; ++I)
      {
        if(Man->ModalStack[I])
        {
          Man->ModalStack[I]->GetTypeAndName(Type,Name);
          fprintf(fp,"\tModalStack[%d] %p  Type='%s' Name='%s'\n",
                      I,Man->ModalStack[I],Type,Name);
        }
        else
          fprintf(fp,"\tModalStack[%d] NULL\n",I,Man->ModalStack[I]);
      }
    }
    else
     fprintf(fp,"\tModalStack = NULL\n");

    fprintf(fp,"**** Претенденты на ... ***\n");

    if(!Man->InsertedFrame)
      *Type=*Name=0;
    else
      Man->InsertedFrame->GetTypeAndName(Type,Name);
    fprintf(fp,"\tInsertedFrame=%p (Type='%s' Name='%s') - Фрейм, который будет добавлен в конец немодальной очереди\n",
      Man->InsertedFrame,Type,Name);

    if(!Man->DeletedFrame)
      *Type=*Name=0;
    else
      Man->DeletedFrame->GetTypeAndName(Type,Name);
    fprintf(fp,"\tDeletedFrame=%p (Type='%s' Name='%s') - Фрейм, предназначений для удаления из модальной очереди, из модального стека, либо одиночный (которого нет ни там, ни там)\n",
       Man->DeletedFrame,Type,Name);

    if(!Man->ActivatedFrame)
      *Type=*Name=0;
    else
      Man->ActivatedFrame->GetTypeAndName(Type,Name);
    fprintf(fp,"\tActivatedFrame=%p (Type='%s' Name='%s') - Фрейм, который необходимо активировать после каких нибудь изменений\n",
      Man->ActivatedFrame,Type,Name);

    if(!Man->RefreshedFrame)
      *Type=*Name=0;
    else
      Man->RefreshedFrame->GetTypeAndName(Type,Name);
    fprintf(fp,"\tRefreshedFrame=%p (Type='%s' Name='%s') - Фрейм, который нужно просто освежить, т.е. перерисовать\n",
      Man->RefreshedFrame,Type,Name);

    if(!Man->ModalizedFrame)
      *Type=*Name=0;
    else
      Man->ModalizedFrame->GetTypeAndName(Type,Name);
    fprintf(fp,"\tModalizedFrame=%p (Type='%s' Name='%s') - Фрейм, который становится в 'очередь' к текущему немодальному фрейму\n",
      Man->ModalizedFrame,Type,Name);

    if(!Man->UnmodalizedFrame)
      *Type=*Name=0;
    else
      Man->UnmodalizedFrame->GetTypeAndName(Type,Name);
    fprintf(fp,"\tUnmodalizedFrame=%p (Type='%s' Name='%s') -Фрейм, убираюющийся из 'очереди' немодального фрейма\n",
      Man->UnmodalizedFrame,Type,Name);

    if(!Man->DeactivatedFrame)
      *Type=*Name=0;
    else
      Man->DeactivatedFrame->GetTypeAndName(Type,Name);
    fprintf(fp,"\tDeactivatedFrame=%p (Type='%s' Name='%s') -Фрейм, который указывает на предудущий активный фрейм\n",
      Man->DeactivatedFrame,Type,Name);


    if(!Man->ExecutedFrame)
      *Type=*Name=0;
    else
      Man->ExecutedFrame->GetTypeAndName(Type,Name);
    fprintf(fp,"\tExecutedFrame=%p (Type='%s' Name='%s') - Фрейм, которого вскорости нужно будет поставить на вершину модального сттека\n",
      Man->ExecutedFrame,Type,Name);

    if(!Man->CurrentFrame)
      *Type=*Name=0;
    else
      Man->CurrentFrame->GetTypeAndName(Type,Name);
    fprintf(fp,"\tCurrentFrame=%p (Type='%s' Name='%s') - текущий фрейм. Он может нахлодиться как в немодальной очереди, так и в можальном стеке\n",
      Man->CurrentFrame,Type,Name);

    fprintf(fp,"\n");
    fflush(fp);
  }

  if(InternalLog)
    CloseSysLog();
#endif
}


#if defined(SYSLOG_FARSYSLOG)
void WINAPIV _export FarSysLog(char *ModuleName,int l,char *fmt,...)
{
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
    fprintf(LogStream,"%s %s%s:: %s\n",PrintTime(timebuf),MakeSpace(),PointToName(ModuleName),msg);
    fflush(LogStream);
  }
  CloseSysLog();
  if(pIsDebuggerPresent && pIsDebuggerPresent())
  {
    OutputDebugString(msg);
#ifdef _MSC_VER
    OutputDebugString("\n");
#endif _MSC_VER
  }
}

void WINAPI _export FarSysLogDump(char *ModuleName,DWORD StartAddress,LPBYTE Buf,int SizeBuf)
{
  SysLogDump(ModuleName,StartAddress,Buf,SizeBuf,NULL);
}

void WINAPI _export FarSysLog_INPUT_RECORD_Dump(char *ModuleName,INPUT_RECORD *rec)
{
  SysLog("%s {%s}",ModuleName,_INPUT_RECORD_Dump(rec));
}
#endif

// "Умный класс для SysLog
CleverSysLog::CleverSysLog(char *Title)
{
#if defined(SYSLOG)
  SysLog(1,"%s{",Title?Title:"");
#endif
}
CleverSysLog::~CleverSysLog()
{
#if defined(SYSLOG)
  SysLog(-1,"}");
#endif
}

const char *_ECTL_ToName(int Command)
{
#if defined(SYSLOG_KEYMACRO) || defined(SYSLOG_ECTL)
#define DEF_ECTL_(m) { ECTL_##m , #m }
  static struct ECTLName{
    int Msg;
    const char *Name;
  } ECTL[]={
    DEF_ECTL_(GETSTRING),      DEF_ECTL_(SETSTRING),
    DEF_ECTL_(INSERTSTRING),   DEF_ECTL_(DELETESTRING),
    DEF_ECTL_(DELETECHAR),     DEF_ECTL_(INSERTTEXT),
    DEF_ECTL_(GETINFO),        DEF_ECTL_(SETPOSITION),
    DEF_ECTL_(SELECT),         DEF_ECTL_(REDRAW),
    DEF_ECTL_(EDITORTOOEM),    DEF_ECTL_(OEMTOEDITOR),
    DEF_ECTL_(TABTOREAL),      DEF_ECTL_(REALTOTAB),
    DEF_ECTL_(EXPANDTABS),     DEF_ECTL_(SETTITLE),
    DEF_ECTL_(READINPUT),      DEF_ECTL_(PROCESSINPUT),
    DEF_ECTL_(ADDCOLOR),       DEF_ECTL_(GETCOLOR),
    DEF_ECTL_(SAVEFILE),       DEF_ECTL_(QUIT),
    DEF_ECTL_(SETKEYBAR),      DEF_ECTL_(PROCESSKEY),
    DEF_ECTL_(SETPARAM),       DEF_ECTL_(GETBOOKMARKS),
    DEF_ECTL_(TURNOFFMARKINGBLOCK),
    DEF_ECTL_(DELETEBLOCK),
  };
  int I;
  static char Name[512];
  for(I=0; I < sizeof(ECTL)/sizeof(ECTL[0]); ++I)
    if(ECTL[I].Msg == Command)
    {
      sprintf(Name,"\"ECTL_%s\" [%d/0x%04X]",ECTL[I].Name,Command,Command);
      return Name;
    }
  sprintf(Name,"\"ECTL_????\" [%d/0x%04X]",Command,Command);
  return Name;
#else
  return "";
#endif
}

const char *_EE_ToName(int Command)
{
#if defined(SYSLOG)
#define DEF_EE_(m) { EE_##m , #m }
  static struct EEName{
    int Msg;
    const char *Name;
  } EE[]={
    DEF_EE_(READ),     DEF_EE_(SAVE),     DEF_EE_(REDRAW),     DEF_EE_(CLOSE),
  };
  int I;
  static char Name[512];
  for(I=0; I < sizeof(EE)/sizeof(EE[0]); ++I)
    if(EE[I].Msg == Command)
    {
      sprintf(Name,"\"EE_%s\" [%d/0x%04X]",EE[I].Name,Command,Command);
      return Name;
    }
  sprintf(Name,"\"EE_????\" [%d/0x%04X]",Command,Command);
  return Name;
#else
  return "";
#endif
}

const char *_EEREDRAW_ToName(int Command)
{
#if defined(SYSLOG)
#define DEF_EEREDRAW_(m) { (int)EEREDRAW_##m , #m }
  static struct EEREDRAWName{
    int Msg;
    const char *Name;
  } EEREDRAW[]={
    DEF_EEREDRAW_(ALL),  DEF_EEREDRAW_(CHANGE),  DEF_EEREDRAW_(LINE),
  };
  int I;
  static char Name[512];
  for(I=0; I < sizeof(EEREDRAW)/sizeof(EEREDRAW[0]); ++I)
    if(EEREDRAW[I].Msg == Command)
    {
      sprintf(Name,"\"EEREDRAW_%s\" [%d/0x%04X]",EEREDRAW[I].Name,Command,Command);
      return Name;
    }
  sprintf(Name,"\"EEREDRAW_????\" [%d/0x%04X]",Command,Command);
  return Name;
#else
  return "";
#endif
}

const char *_ESPT_ToName(int Command)
{
#if defined(SYSLOG_KEYMACRO) || defined(SYSLOG_ECTL)
#define DEF_ESPT_(m) { ESPT_##m , #m }
  static struct ESPTName{
    int Msg;
    const char *Name;
  } ESPT[]={
    DEF_ESPT_(TABSIZE),
    DEF_ESPT_(EXPANDTABS),
    DEF_ESPT_(AUTOINDENT),
    DEF_ESPT_(CURSORBEYONDEOL),
    DEF_ESPT_(CHARCODEBASE),
    DEF_ESPT_(CHARTABLE),
    DEF_ESPT_(SAVEFILEPOSITION),
    DEF_ESPT_(LOCKMODE),
  };
  int I;
  static char Name[512];
  for(I=0; I < sizeof(ESPT)/sizeof(ESPT[0]); ++I)
    if(ESPT[I].Msg == Command)
    {
      sprintf(Name,"\"ESPT_%s\" [%d/0x%04X]",ESPT[I].Name,Command,Command);
      return Name;
    }
  sprintf(Name,"\"ESPT_????\" [%d/0x%04X]",Command,Command);
  return Name;
#else
  return "";
#endif
}

const char *_FCTL_ToName(int Command)
{
#if defined(SYSLOG)
#define DEF_FCTL_(m) { FCTL_##m , #m }
  static struct FCTLName{
    int Msg;
    const char *Name;
  } FCTL[]={
     DEF_FCTL_(CLOSEPLUGIN),           DEF_FCTL_(GETPANELINFO),
     DEF_FCTL_(GETANOTHERPANELINFO),   DEF_FCTL_(UPDATEPANEL),
     DEF_FCTL_(UPDATEANOTHERPANEL),    DEF_FCTL_(REDRAWPANEL),
     DEF_FCTL_(REDRAWANOTHERPANEL),    DEF_FCTL_(SETANOTHERPANELDIR),
     DEF_FCTL_(GETCMDLINE),            DEF_FCTL_(SETCMDLINE),
     DEF_FCTL_(SETSELECTION),          DEF_FCTL_(SETANOTHERSELECTION),
     DEF_FCTL_(SETVIEWMODE),           DEF_FCTL_(SETANOTHERVIEWMODE),
     DEF_FCTL_(INSERTCMDLINE),         DEF_FCTL_(SETUSERSCREEN),
     DEF_FCTL_(SETPANELDIR),           DEF_FCTL_(SETCMDLINEPOS),
     DEF_FCTL_(GETCMDLINEPOS),         DEF_FCTL_(SETSORTMODE),
     DEF_FCTL_(SETANOTHERSORTMODE),    DEF_FCTL_(SETSORTORDER),
     DEF_FCTL_(SETANOTHERSORTORDER),   DEF_FCTL_(GETCMDLINESELECTEDTEXT),
     DEF_FCTL_(SETCMDLINESELECTION),   DEF_FCTL_(GETCMDLINESELECTION),
  };
  int I;
  static char Name[512];
  for(I=0; I < sizeof(FCTL)/sizeof(FCTL[0]); ++I)
    if(FCTL[I].Msg == Command)
    {
      sprintf(Name,"\"FCTL_%s\" [%d/0x%04X]",FCTL[I].Name,Command,Command);
      return Name;
    }
  sprintf(Name,"\"FCTL_????\" [%d/0x%04X]",Command,Command);
  return Name;
#else
  return "";
#endif
}


const char *_FARKEY_ToName(int Key)
{
#if defined(SYSLOG)
  static char Name[512];
  if(KeyToText(Key,Name,0))
  {
    InsertQuote(Name);
    sprintf(Name+strlen(Name)," [%u/0x%08X]",Key,Key);
    return Name;
  }
  sprintf(Name,"\"KEY_????\" [%u/0x%08X]",Key,Key);
  return Name;
#else
  return "";
#endif
}

#if defined(SYSLOG)
#define DEF_VK(k) { VK_##k , #k }
#define VK_XBUTTON1       0x05    /* NOT contiguous with L & RBUTTON */
#define VK_XBUTTON2       0x06    /* NOT contiguous with L & RBUTTON */
#define VK_CONVERT        0x1C
#define VK_NONCONVERT     0x1D
#define VK_ACCEPT         0x1E
#define VK_MODECHANGE     0x1F
#define VK_SLEEP          0x5F

#define VK_BROWSER_BACK        0xA6
#define VK_BROWSER_FORWARD     0xA7
#define VK_BROWSER_REFRESH     0xA8
#define VK_BROWSER_STOP        0xA9
#define VK_BROWSER_SEARCH      0xAA
#define VK_BROWSER_FAVORITES   0xAB
#define VK_BROWSER_HOME        0xAC

#define VK_VOLUME_MUTE         0xAD
#define VK_VOLUME_DOWN         0xAE
#define VK_VOLUME_UP           0xAF
#define VK_MEDIA_NEXT_TRACK    0xB0
#define VK_MEDIA_PREV_TRACK    0xB1
#define VK_MEDIA_STOP          0xB2
#define VK_MEDIA_PLAY_PAUSE    0xB3
#define VK_LAUNCH_MAIL         0xB4
#define VK_LAUNCH_MEDIA_SELECT 0xB5
#define VK_LAUNCH_APP1         0xB6
#define VK_LAUNCH_APP2         0xB7

/*
 * 0xB8 - 0xB9 : reserved
 */

#define VK_OEM_1          0xBA   // ';:' for US
#define VK_OEM_PLUS       0xBB   // '+' any country
#define VK_OEM_COMMA      0xBC   // ',' any country
#define VK_OEM_MINUS      0xBD   // '-' any country
#define VK_OEM_PERIOD     0xBE   // '.' any country
#define VK_OEM_2          0xBF   // '/?' for US
#define VK_OEM_3          0xC0   // '`~' for US

/*
 * 0xC1 - 0xD7 : reserved
 */

/*
 * 0xD8 - 0xDA : unassigned
 */

#define VK_OEM_4          0xDB  //  '[{' for US
#define VK_OEM_5          0xDC  //  '\|' for US
#define VK_OEM_6          0xDD  //  ']}' for US
#define VK_OEM_7          0xDE  //  ''"' for US
#define VK_OEM_8          0xDF

/*
 * 0xE0 : reserved
 */

/*
 * Various extended or enhanced keyboards
 */
#define VK_OEM_AX         0xE1  //  'AX' key on Japanese AX kbd
#define VK_OEM_102        0xE2  //  "<>" or "\|" on RT 102-key kbd.
#define VK_ICO_HELP       0xE3  //  Help key on ICO
#define VK_ICO_00         0xE4  //  00 key on ICO

#define VK_PROCESSKEY     0xE5
#define VK_ICO_CLEAR      0xE6

#define VK_PACKET         0xE7

/*
 * 0xE8 : unassigned
 */

/*
 * Nokia/Ericsson definitions
 */
#define VK_OEM_RESET      0xE9
#define VK_OEM_JUMP       0xEA
#define VK_OEM_PA1        0xEB
#define VK_OEM_PA2        0xEC
#define VK_OEM_PA3        0xED
#define VK_OEM_WSCTRL     0xEE
#define VK_OEM_CUSEL      0xEF
#define VK_OEM_ATTN       0xF0
#define VK_OEM_FINISH     0xF1
#define VK_OEM_COPY       0xF2
#define VK_OEM_AUTO       0xF3
#define VK_OEM_ENLW       0xF4
#define VK_OEM_BACKTAB    0xF5

#define VK_ATTN           0xF6
#define VK_CRSEL          0xF7
#define VK_EXSEL          0xF8
#define VK_EREOF          0xF9
#define VK_PLAY           0xFA
#define VK_ZOOM           0xFB
#define VK_NONAME         0xFC
#define VK_PA1            0xFD
#define VK_OEM_CLEAR      0xFE


struct VirtKeyDef{
  WORD vk;
  char *n;
} vk_def[]={
  DEF_VK(LBUTTON),     DEF_VK(RBUTTON),    DEF_VK(CANCEL),    DEF_VK(MBUTTON),
  DEF_VK(XBUTTON1),    DEF_VK(XBUTTON2),
  DEF_VK(BACK),        DEF_VK(TAB),        DEF_VK(CLEAR),     DEF_VK(RETURN),
  DEF_VK(SHIFT),       DEF_VK(CONTROL),    DEF_VK(MENU),      DEF_VK(PAUSE),
  DEF_VK(CAPITAL),     DEF_VK(ESCAPE),
  DEF_VK(CONVERT),     DEF_VK(NONCONVERT), DEF_VK(ACCEPT),    DEF_VK(MODECHANGE),
  DEF_VK(SPACE),       DEF_VK(PRIOR),
  DEF_VK(NEXT),        DEF_VK(END),        DEF_VK(HOME),      DEF_VK(LEFT),
  DEF_VK(UP),          DEF_VK(RIGHT),      DEF_VK(DOWN),      DEF_VK(SELECT),
  DEF_VK(PRINT),       DEF_VK(EXECUTE),    DEF_VK(SNAPSHOT),  DEF_VK(INSERT),
  DEF_VK(DELETE),      DEF_VK(HELP),       DEF_VK(LWIN),      DEF_VK(RWIN),
  DEF_VK(APPS),        DEF_VK(SLEEP),
  DEF_VK(NUMPAD0),     DEF_VK(NUMPAD1),    DEF_VK(NUMPAD2),
  DEF_VK(NUMPAD3),     DEF_VK(NUMPAD4),    DEF_VK(NUMPAD5),   DEF_VK(NUMPAD6),
  DEF_VK(NUMPAD7),     DEF_VK(NUMPAD8),    DEF_VK(NUMPAD9),   DEF_VK(MULTIPLY),
  DEF_VK(ADD),         DEF_VK(SEPARATOR),  DEF_VK(SUBTRACT),  DEF_VK(DECIMAL),
  DEF_VK(DIVIDE),
  DEF_VK(F1),          DEF_VK(F2),         DEF_VK(F3),        DEF_VK(F4),
  DEF_VK(F5),          DEF_VK(F6),         DEF_VK(F7),        DEF_VK(F8),
  DEF_VK(F9),          DEF_VK(F10),        DEF_VK(F11),       DEF_VK(F12),
  DEF_VK(F13),         DEF_VK(F14),        DEF_VK(F15),       DEF_VK(F16),
  DEF_VK(F17),         DEF_VK(F18),        DEF_VK(F19),       DEF_VK(F20),
  DEF_VK(F21),         DEF_VK(F22),        DEF_VK(F23),       DEF_VK(F24),
  DEF_VK(NUMLOCK),     DEF_VK(SCROLL),     DEF_VK(LSHIFT),
  DEF_VK(RSHIFT),      DEF_VK(LCONTROL),   DEF_VK(RCONTROL),  DEF_VK(LMENU),
  DEF_VK(RMENU),
  DEF_VK(ATTN),        DEF_VK(CRSEL),      DEF_VK(EXSEL),     DEF_VK(EREOF),
  DEF_VK(PLAY),        DEF_VK(ZOOM),       DEF_VK(NONAME),    DEF_VK(PA1),
  DEF_VK(OEM_CLEAR),   DEF_VK(OEM_1),      DEF_VK(OEM_PLUS),  DEF_VK(OEM_COMMA),
  DEF_VK(OEM_MINUS),   DEF_VK(OEM_PERIOD), DEF_VK(OEM_2),     DEF_VK(OEM_3),
  DEF_VK(OEM_4),       DEF_VK(OEM_5),      DEF_VK(OEM_6),     DEF_VK(OEM_7),
  DEF_VK(OEM_8),       DEF_VK(PROCESSKEY), DEF_VK(OEM_RESET), DEF_VK(OEM_JUMP),
  DEF_VK(OEM_PA1),     DEF_VK(OEM_PA2),    DEF_VK(OEM_PA3),   DEF_VK(OEM_WSCTRL),
  DEF_VK(OEM_CUSEL),   DEF_VK(OEM_ATTN),   DEF_VK(OEM_FINISH),DEF_VK(OEM_COPY),
  DEF_VK(OEM_AUTO),    DEF_VK(OEM_ENLW),   DEF_VK(OEM_BACKTAB),
};
#endif


const char *_VK_KEY_ToName(int VkKey)
{
#if defined(SYSLOG)
  static char Name[512];
  int I;

  Name[0]=0;
  if(VkKey >= '0' && VkKey <= '9' || VkKey >= 'A' && VkKey <= 'Z')
  {
    sprintf(Name,"\"VK_%c\" [%d/0x%04X]",VkKey,VkKey,VkKey);
    return Name;
  }
  else
  {
    for(I=0; I < sizeof(vk_def)/sizeof(vk_def[0]); ++I)
    {
      if(VkKey == vk_def[I].vk)  // c || KeyCode
      {
        sprintf(Name,"\"VK_%s\" [%d/0x%04X]",vk_def[I].n,VkKey,VkKey);
        return Name;
      }
    }
  }
  sprintf(Name,"\"VK_??????\" [%d/0x%04X]",VkKey,VkKey);
  return Name;
#else
  return "";
#endif
}

const char *_INPUT_RECORD_Dump(INPUT_RECORD *rec)
{
#if defined(SYSLOG)
  static char Records[8192];

  switch(rec->EventType)
  {
    case FOCUS_EVENT:
      sprintf(Records,
            "FOCUS_EVENT_RECORD: %s",
          (rec->Event.FocusEvent.bSetFocus?"TRUE":"FALSE")
        );
      break;
    case WINDOW_BUFFER_SIZE_EVENT:
      sprintf(Records,
            "WINDOW_BUFFER_SIZE_RECORD: Size = [%d, %d]",
          rec->Event.WindowBufferSizeEvent.dwSize.X,
          rec->Event.WindowBufferSizeEvent.dwSize.Y
        );
      break;
    case MENU_EVENT:
      sprintf(Records,
            "MENU_EVENT_RECORD: CommandId = %d (0x%X) ",
          rec->Event.MenuEvent.dwCommandId,
          rec->Event.MenuEvent.dwCommandId
        );
      break;
    case KEY_EVENT:
      sprintf(Records,
            "KEY_EVENT_RECORD: %s, %d, Vk=%s, Scan=0x%04X uChar=[U='%C' (0x%04X): A='%c' (0x%02X)] Ctrl=0x%08X (%c%c%c%c - %c%c%c%c)",
          (rec->Event.KeyEvent.bKeyDown?"Dn":"Up"),
          rec->Event.KeyEvent.wRepeatCount,
          _VK_KEY_ToName(rec->Event.KeyEvent.wVirtualKeyCode),
          rec->Event.KeyEvent.wVirtualScanCode,
          (rec->Event.KeyEvent.uChar.UnicodeChar && !(rec->Event.KeyEvent.uChar.UnicodeChar == 9 || rec->Event.KeyEvent.uChar.UnicodeChar == 0xd || rec->Event.KeyEvent.uChar.UnicodeChar == 0xa)?rec->Event.KeyEvent.uChar.UnicodeChar:L' '),
              rec->Event.KeyEvent.uChar.UnicodeChar,
          (rec->Event.KeyEvent.uChar.AsciiChar && rec->Event.KeyEvent.uChar.AsciiChar != 0x0d && rec->Event.KeyEvent.uChar.AsciiChar != 0x9 && rec->Event.KeyEvent.uChar.AsciiChar !=0xA ?rec->Event.KeyEvent.uChar.AsciiChar:' '),
              rec->Event.KeyEvent.uChar.AsciiChar,
          rec->Event.KeyEvent.dwControlKeyState,
            (rec->Event.KeyEvent.dwControlKeyState&LEFT_CTRL_PRESSED?'C':'c'),
            (rec->Event.KeyEvent.dwControlKeyState&LEFT_ALT_PRESSED?'A':'a'),
            (rec->Event.KeyEvent.dwControlKeyState&SHIFT_PRESSED?'S':'s'),
            (rec->Event.KeyEvent.dwControlKeyState&RIGHT_ALT_PRESSED?'A':'a'),
            (rec->Event.KeyEvent.dwControlKeyState&RIGHT_CTRL_PRESSED?'C':'c'),
            (rec->Event.KeyEvent.dwControlKeyState&ENHANCED_KEY?'E':'e'),
            (rec->Event.KeyEvent.dwControlKeyState&CAPSLOCK_ON?'C':'c'),
            (rec->Event.KeyEvent.dwControlKeyState&NUMLOCK_ON?'N':'n'),
            (rec->Event.KeyEvent.dwControlKeyState&SCROLLLOCK_ON?'S':'s')
        );
        //sprintf(Records+strlen(Records)," (%C %C)",(rec->Event.KeyEvent.uChar.UnicodeChar && !(rec->Event.KeyEvent.uChar.UnicodeChar == 9 || rec->Event.KeyEvent.uChar.UnicodeChar == 0xd || rec->Event.KeyEvent.uChar.UnicodeChar == 0xa)?rec->Event.KeyEvent.uChar.UnicodeChar:L' '),rec->Event.KeyEvent.uChar.UnicodeChar);
      break;
    case MOUSE_EVENT:
      sprintf(Records,
            "MOUSE_EVENT_RECORD: [%d,%d], Btn=0x%08X (%c%c%c%c%c), Ctrl=0x%08X (%c%c%c%c%c - %c%c%c%c), Flgs=0x%08X (%s)",
          rec->Event.MouseEvent.dwMousePosition.X,
          rec->Event.MouseEvent.dwMousePosition.Y,
          rec->Event.MouseEvent.dwButtonState,
            (rec->Event.MouseEvent.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED?'L':'l'),
            (rec->Event.MouseEvent.dwButtonState&RIGHTMOST_BUTTON_PRESSED?'R':'r'),
            (rec->Event.MouseEvent.dwButtonState&FROM_LEFT_2ND_BUTTON_PRESSED?'2':' '),
            (rec->Event.MouseEvent.dwButtonState&FROM_LEFT_3RD_BUTTON_PRESSED?'3':' '),
            (rec->Event.MouseEvent.dwButtonState&FROM_LEFT_4TH_BUTTON_PRESSED?'4':' '),
          rec->Event.MouseEvent.dwControlKeyState,
            (rec->Event.MouseEvent.dwControlKeyState&LEFT_CTRL_PRESSED?'C':'c'),
            (rec->Event.MouseEvent.dwControlKeyState&LEFT_ALT_PRESSED?'A':'a'),
            (rec->Event.MouseEvent.dwControlKeyState&SHIFT_PRESSED?'S':'s'),
            (rec->Event.MouseEvent.dwControlKeyState&RIGHT_ALT_PRESSED?'A':'a'),
            (rec->Event.MouseEvent.dwControlKeyState&RIGHT_CTRL_PRESSED?'C':'c'),
            (rec->Event.MouseEvent.dwControlKeyState&ENHANCED_KEY?'E':'e'),
            (rec->Event.MouseEvent.dwControlKeyState&CAPSLOCK_ON?'C':'c'),
            (rec->Event.MouseEvent.dwControlKeyState&NUMLOCK_ON?'N':'n'),
            (rec->Event.MouseEvent.dwControlKeyState&SCROLLLOCK_ON?'S':'s'),
          rec->Event.MouseEvent.dwEventFlags,
            (rec->Event.MouseEvent.dwEventFlags==DOUBLE_CLICK?"(DblClick)":
             (rec->Event.MouseEvent.dwEventFlags==MOUSE_MOVED?"(Moved)":
              (rec->Event.MouseEvent.dwEventFlags==MOUSE_WHEELED?"(Wheel)":"")))
        );
      if(rec->Event.MouseEvent.dwEventFlags==MOUSE_WHEELED)
      {
        sprintf(Records+strlen(Records)," (Delta=%d)",(short)HIWORD(rec->Event.MouseEvent.dwButtonState));
      }
      break;
    default:
      sprintf(Records,
            "??????_EVENT_RECORD: EventType = %d",
          rec->EventType
        );
      break;
  }
  sprintf(Records+strlen(Records)," (%s)",FarAltEnter(FAR_CONSOLE_GET_MODE)==FAR_CONSOLE_WINDOWED?"Widowed":"Full Screen");
  return Records;
#else
  return "";
#endif
}

void INPUT_RECORD_DumpBuffer(FILE *fp)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

  int InternalLog=fp==NULL?TRUE:FALSE;
  DWORD ReadCount2;
  // берем количество оставшейся порции эвентов
  GetNumberOfConsoleInputEvents(hConInp,&ReadCount2);
  if(ReadCount2 <= 1)
    return;

  if(InternalLog)
  {
    OpenSysLog();
    fp=LogStream;
    if(fp)
    {
      char timebuf[64];
      fprintf(fp,"%s %s(Number Of Console Input Events = %d)\n",PrintTime(timebuf),MakeSpace(),ReadCount2);
    }
  }

  if (fp)
  {
    if(ReadCount2 > 1)
    {
      INPUT_RECORD *TmpRec=(INPUT_RECORD*)xf_malloc(sizeof(INPUT_RECORD)*ReadCount2);
      if(TmpRec)
      {
        DWORD ReadCount3;
        INPUT_RECORD TmpRec2;
        int I;

        #if defined(USE_WFUNC_IN)
        if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
          PeekConsoleInputW(hConInp,TmpRec,ReadCount2,&ReadCount3);
        else
          PeekConsoleInputA(hConInp,TmpRec,ReadCount2,&ReadCount3);
        #else
        PeekConsoleInput(hConInp,TmpRec,ReadCount2,&ReadCount3);
        #endif

        for(I=0; I < ReadCount2; ++I)
        {
          fprintf(fp,"             %s%04d: %s\n",MakeSpace(),I,_INPUT_RECORD_Dump(TmpRec+I));
        }
        // освободим память
        xf_free(TmpRec);
      }
    }
    fflush(fp);
  }

  if(InternalLog)
    CloseSysLog();
#endif
}

const char *_DLGMSG_ToName(int Msg)
{
#if defined(SYSLOG)
#define DEF_MESSAGE(m) { m , #m }
  static struct MsgName{
    int Msg;
    const char *Name;
  } Message[]={
    DEF_MESSAGE(DM_FIRST),              DEF_MESSAGE(DM_CLOSE),
    DEF_MESSAGE(DM_ENABLE),             DEF_MESSAGE(DM_ENABLEREDRAW),
    DEF_MESSAGE(DM_GETDLGDATA),         DEF_MESSAGE(DM_GETDLGITEM),
    DEF_MESSAGE(DM_GETDLGRECT),         DEF_MESSAGE(DM_GETTEXT),
    DEF_MESSAGE(DM_GETTEXTLENGTH),      DEF_MESSAGE(DM_KEY),
    DEF_MESSAGE(DM_MOVEDIALOG),         DEF_MESSAGE(DM_SETDLGDATA),
    DEF_MESSAGE(DM_SETDLGITEM),         DEF_MESSAGE(DM_SETFOCUS),
    DEF_MESSAGE(DM_REDRAW),             DEF_MESSAGE(DM_SETTEXT),
    DEF_MESSAGE(DM_SETMAXTEXTLENGTH),   DEF_MESSAGE(DM_SHOWDIALOG),
    DEF_MESSAGE(DM_GETFOCUS),           DEF_MESSAGE(DM_GETCURSORPOS),
    DEF_MESSAGE(DM_SETCURSORPOS),       DEF_MESSAGE(DM_GETTEXTPTR),
    DEF_MESSAGE(DM_SETTEXTPTR),         DEF_MESSAGE(DM_SHOWITEM),
    DEF_MESSAGE(DM_ADDHISTORY),         DEF_MESSAGE(DM_GETCHECK),
    DEF_MESSAGE(DM_SETCHECK),           DEF_MESSAGE(DM_SET3STATE),
    DEF_MESSAGE(DM_LISTSORT),           DEF_MESSAGE(DM_LISTGETITEM),
    DEF_MESSAGE(DM_LISTSET),            DEF_MESSAGE(DM_LISTGETCURPOS),
    DEF_MESSAGE(DM_LISTSETCURPOS),      DEF_MESSAGE(DM_LISTDELETE),
    DEF_MESSAGE(DM_LISTADD),            DEF_MESSAGE(DM_LISTADDSTR),
    DEF_MESSAGE(DM_LISTUPDATE),         DEF_MESSAGE(DM_LISTINSERT),
    DEF_MESSAGE(DM_LISTFINDSTRING),     DEF_MESSAGE(DM_LISTINFO),
    DEF_MESSAGE(DM_LISTGETDATA),        DEF_MESSAGE(DM_LISTSETDATA),
    DEF_MESSAGE(DM_LISTSETTITLES),      DEF_MESSAGE(DM_LISTGETTITLES),
    DEF_MESSAGE(DM_RESIZEDIALOG),       DEF_MESSAGE(DM_SETITEMPOSITION),
    DEF_MESSAGE(DM_GETDROPDOWNOPENED),  DEF_MESSAGE(DM_SETDROPDOWNOPENED),
    DEF_MESSAGE(DM_SETHISTORY),         DEF_MESSAGE(DM_GETITEMPOSITION),
    DEF_MESSAGE(DM_SETMOUSEEVENTNOTIFY),DEF_MESSAGE(DN_FIRST),
    DEF_MESSAGE(DN_BTNCLICK),           DEF_MESSAGE(DN_CTLCOLORDIALOG),
    DEF_MESSAGE(DN_CTLCOLORDLGITEM),    DEF_MESSAGE(DN_CTLCOLORDLGLIST),
    DEF_MESSAGE(DN_DRAWDIALOG),         DEF_MESSAGE(DN_DRAWDLGITEM),
    DEF_MESSAGE(DN_EDITCHANGE),         DEF_MESSAGE(DN_ENTERIDLE),
    DEF_MESSAGE(DN_GOTFOCUS),           DEF_MESSAGE(DN_HELP),
    DEF_MESSAGE(DN_HOTKEY),             DEF_MESSAGE(DN_INITDIALOG),
    DEF_MESSAGE(DN_KILLFOCUS),          DEF_MESSAGE(DN_LISTCHANGE),
    DEF_MESSAGE(DN_MOUSECLICK),         DEF_MESSAGE(DN_DRAGGED),
    DEF_MESSAGE(DN_RESIZECONSOLE),      DEF_MESSAGE(DN_MOUSEEVENT),
    DEF_MESSAGE(DN_CLOSE),              DEF_MESSAGE(DN_KEY),
    DEF_MESSAGE(DM_USER),               DEF_MESSAGE(DM_KILLSAVESCREEN),
    DEF_MESSAGE(DM_ALLKEYMODE),         DEF_MESSAGE(DM_LISTGETDATASIZE),
    DEF_MESSAGE(DN_ACTIVATEAPP),
    DEF_MESSAGE(DM_GETSELECTION),       DEF_MESSAGE(DM_SETSELECTION),
  };
  int I;

  static char Name[512];
  for(I=0; I < sizeof(Message)/sizeof(Message[0]); ++I)
    if(Message[I].Msg == Msg)
    {
      sprintf(Name,"\"%s\" [%d/0x%08X]",Message[I].Name,Msg,Msg);
      return Name;
    }
  sprintf(Name,"\"%s+[%d/0x%08X]\"",(Msg>=DN_FIRST?"DN_FIRST":(Msg>=DM_USER?"DM_USER":"DM_FIRST")),Msg,Msg);
  return Name;
#else
  return "";
#endif
}

const char *_ACTL_ToName(int Command)
{
#if defined(SYSLOG_ACTL)
#define DEF_ACTL_(m) { ACTL_##m , #m }
  static struct ACTLName{
    int Msg;
    const char *Name;
  } ACTL[]={
    DEF_ACTL_(GETFARVERSION),          DEF_ACTL_(CONSOLEMODE),
    DEF_ACTL_(GETSYSWORDDIV),          DEF_ACTL_(WAITKEY),
    DEF_ACTL_(GETCOLOR),               DEF_ACTL_(GETARRAYCOLOR),
    DEF_ACTL_(EJECTMEDIA),             DEF_ACTL_(KEYMACRO),
    DEF_ACTL_(POSTKEYSEQUENCE),        DEF_ACTL_(GETWINDOWINFO),
    DEF_ACTL_(GETWINDOWCOUNT),         DEF_ACTL_(SETCURRENTWINDOW),
    DEF_ACTL_(COMMIT),                 DEF_ACTL_(GETFARHWND),
    DEF_ACTL_(GETSYSTEMSETTINGS),      DEF_ACTL_(GETPANELSETTINGS),
    DEF_ACTL_(GETINTERFACESETTINGS),   DEF_ACTL_(GETCONFIRMATIONS),
    DEF_ACTL_(GETDESCSETTINGS),
  };
  int I;
  static char Name[512];
  for(I=0; I < sizeof(ACTL)/sizeof(ACTL[0]); ++I)
    if(ACTL[I].Msg == Command)
    {
      sprintf(Name,"\"ACTL_%s\" [%d/0x%04X]",ACTL[I].Name,Command,Command);
      return Name;
    }
  sprintf(Name,"\"ACTL_????\" [%d/0x%04X]",Command,Command);
  return Name;
#else
  return "";
#endif
}


const char *_VCTL_ToName(int Command)
{
#if defined(SYSLOG_VCTL)
#define DEF_VCTL_(m) { VCTL_##m , #m }
  static struct VCTLName{
    int Msg;
    const char *Name;
  } VCTL[]={
    DEF_VCTL_(GETINFO),
    DEF_VCTL_(QUIT),
    DEF_VCTL_(REDRAW),
    DEF_VCTL_(SETKEYBAR),
    DEF_VCTL_(SETPOSITION),
    DEF_VCTL_(SELECT),
  };
  int I;
  static char Name[512];
  for(I=0; I < sizeof(VCTL)/sizeof(VCTL[0]); ++I)
    if(VCTL[I].Msg == Command)
    {
      sprintf(Name,"\"VCTL_%s\" [%d/0x%04X]",VCTL[I].Name,Command,Command);
      return Name;
    }
  sprintf(Name,"\"VCTL_????\" [%d/0x%04X]",Command,Command);
  return Name;
#else
  return "";
#endif
}

// после вызова этой функции нужно освободить память!!!
const char *_SysLog_LinearDump(LPBYTE Buf,int SizeBuf)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return NULL;

  char *TmpBuf=(char *)xf_malloc(sizeof(char)*SizeBuf*3), *PtrTmpBuf;
  if(TmpBuf)
  {
    PtrTmpBuf=TmpBuf;
    int I, Cnt=SizeBuf;
    for(I=0; I < Cnt; ++I)
    {
      sprintf(PtrTmpBuf,"%02X ",Buf[I]&0xFF);
      PtrTmpBuf+=3;
    }
    *PtrTmpBuf=0;
  }
  return TmpBuf;
#else
  return NULL;
#endif
}
