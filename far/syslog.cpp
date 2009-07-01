/*
syslog.cpp

Системный отладочный лог :-)
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

#include "syslog.hpp"
#include "filelist.hpp"
#include "manager.hpp"
#include "frame.hpp"
#include "macroopcode.hpp"
#include "keyboard.hpp"
#include "iswind.hpp"
#include "datetime.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"

#if !defined(SYSLOG)
 #if defined(SYSLOG_OT)             || \
     defined(SYSLOG_SVS)            || \
     defined(SYSLOG_DJ)             || \
     defined(SYSLOG_WARP)           || \
     defined(VVM)                   || \
     defined(SYSLOG_AT)             || \
     defined(SYSLOG_IS)             || \
     defined(SYSLOG_tran)           || \
     defined(SYSLOG_SKV)            || \
     defined(SYSLOG_NWZ)            || \
     defined(SYSLOG_KM)             || \
     defined(SYSLOG_KEYMACRO)       || \
     defined(SYSLOG_ECTL)           || \
     defined(SYSLOG_COPYR)          || \
     defined(SYSLOG_EE_REDRAW)      || \
     defined(SYSLOG_TREX)           || \
     defined(SYSLOG_KEYMACRO_PARSE) || \
     defined(SYSLOG_YJH)            || \
     defined(SYSLOG_MANAGER)
  #define SYSLOG
 #endif
#endif


#if defined(SYSLOG)
wchar_t      LogFileName[MAX_FILE];
static FILE *LogStream=0;
static int   Indent=0;
static wchar_t *PrintTime(wchar_t *timebuf);


static BOOL IsLogON(void)
{
  return GetKeyState(VK_SCROLL)?TRUE:FALSE;
}

static const wchar_t *MakeSpace(void)
{
  static wchar_t Buf[60]=L" ";
  Buf[0]=L' ';
  for(int I=1; I <= Indent; ++I)
    Buf[I]=L'|';
  Buf[1+Indent]=0;
  return Buf;
}

static wchar_t *PrintTime(wchar_t *timebuf)
{
  SYSTEMTIME st;
  GetLocalTime(&st);
//  sprintf(timebuf,"%02d.%02d.%04d %2d:%02d:%02d.%03d",
//      st.wDay,st.wMonth,st.wYear,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
  swprintf(timebuf,L"%02d:%02d:%02d.%03d",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
  return timebuf;
}

static FILE* PrintBaner(FILE *fp,const wchar_t *Category,const wchar_t *Title)
{
  fp=LogStream;
  if(fp)
  {
    static wchar_t timebuf[64];
    fwprintf(fp,L"%s %s(%s) %s\n",PrintTime(timebuf),MakeSpace(),NullToEmpty(Title),NullToEmpty(Category));
  }
  return fp;
}

#endif

FILE * OpenLogStream(const wchar_t *file)
{
#if defined(SYSLOG)
	string strRealLogName;
  SYSTEMTIME st;

  GetLocalTime(&st);
	strRealLogName.Format(L"%s\\Far.%04d%02d%02d.%05d.log",file,st.wYear,st.wMonth,st.wDay,HIWORD(FAR_VERSION));
	return _wfsopen(strRealLogName,L"a+t",SH_DENYWR);
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

	GetModuleFileName(NULL,LogFileName,countof(LogFileName));
	wchar_t *Ptr=(wchar_t*)PointToName(LogFileName);
	wcscpy(Ptr,L"$Log");
	Attr=apiGetFileAttributes(LogFileName);
  if(Attr == INVALID_FILE_ATTRIBUTES)
  {
		if(!apiCreateDirectory(LogFileName,NULL))
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
    wchar_t timebuf[64];
    fwprintf(LogStream,L"%s %s%s\n",PrintTime(timebuf),MakeSpace(),L"Heap Status");

    fwprintf(LogStream,L"   Size   Status\n");
    fwprintf(LogStream,L"   ----   ------\n");

    DWORD Sz=0;
    _HEAPINFO hi;
    hi._pentry=NULL;
    //    int     *__pentry;
    while( _rtl_heapwalk( &hi ) == _HEAPOK )
    {
      fwprintf(LogStream,L"%7u    %s  (%p)\n", hi._size, (hi._useflag ? L"used" : L"free"),hi.__pentry);
      Sz+=hi._useflag?hi._size:0;
    }
    fwprintf(LogStream,L"   ----   ------\n" );
    fwprintf(LogStream,L"%7u      \n", Sz);

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
    SysLog(L"Error: Heap broken, Line=%d",NumLine);
  }
  else if (HeapStatus < 0)
  {
    SysLog(L"Error: Heap corrupt, Line=%d, HeapStatus=%d",NumLine,HeapStatus);
  }
  else
    SysLog(L"Heap OK, HeapStatus=%d",HeapStatus);
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


void SysLog(const wchar_t *fmt,...)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

  wchar_t msg[MAX_LOG_LINE];

  va_list argptr;
  va_start( argptr, fmt );
	_vsnwprintf( msg, countof(msg)-1, fmt, argptr );
  va_end(argptr);

  OpenSysLog();
  if ( LogStream )
  {
    wchar_t timebuf[64];
    fwprintf(LogStream,L"%s %s%s\n",PrintTime(timebuf),MakeSpace(),msg);
    fflush(LogStream);
  }
  CloseSysLog();
  if(IsDebuggerPresent())
  {
		OutputDebugString(msg);
#ifdef _MSC_VER
		OutputDebugString(L"\n");
#endif _MSC_VER
  }
#endif
}

void SysLogLastError(void)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

  wchar_t *lpMsgBuf;

  DWORD LastErr=GetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,LastErr,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
                (LPWSTR) &lpMsgBuf,0,NULL);

  OpenSysLog();
  if (LogStream)
  {
    wchar_t timebuf[64];
		// RemoveUnprintableCharacters(lpMsgBuf);
    fwprintf(LogStream,L"%s %sGetLastError()=[%d/0x%X] \"%s\"\n",PrintTime(timebuf),MakeSpace(),LastErr,LastErr,lpMsgBuf);
    fflush(LogStream);
  }
  CloseSysLog();
  if(IsDebuggerPresent())
  {
		OutputDebugString(lpMsgBuf);
#ifdef _MSC_VER
		OutputDebugString(L"\n");
#endif _MSC_VER
  }
  LocalFree(lpMsgBuf);
  SetLastError(LastErr);
#endif
}

///
void SysLog(int l,const wchar_t *fmt,...)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

  wchar_t msg[MAX_LOG_LINE];

  va_list argptr;
  va_start( argptr, fmt );
	_vsnwprintf( msg, countof(msg)-1, fmt, argptr );
  va_end(argptr);

  OpenSysLog();
  if ( LogStream )
  {
    if(l < 0) SysLog(l);
    wchar_t timebuf[64];
    fwprintf(LogStream,L"%s %s%s\n",PrintTime(timebuf),MakeSpace(),msg);
    fflush(LogStream);
    if(l > 0) SysLog(l);
  }
  CloseSysLog();
  if(IsDebuggerPresent())
  {
		OutputDebugString(msg);
#ifdef _MSC_VER
		OutputDebugString(L"\n");
#endif _MSC_VER
  }
#endif
}


void SysLogDump(const wchar_t *Title,DWORD StartAddress,LPBYTE Buf,int SizeBuf,FILE *fp)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

  int CY=(SizeBuf+15)/16;
  int X,Y;
  int InternalLog=fp==NULL?TRUE:FALSE;
  static wchar_t timebuf[64];

//  char msg[MAX_LOG_LINE];

  if(InternalLog)
  {
    OpenSysLog();
    fp=LogStream;
    fwprintf(fp,L"%s %s<%s> [%u bytes]{\n",PrintTime(timebuf),MakeSpace(),NullToEmpty(Title),SizeBuf);
  }

  if (fp)
  {
    if(!InternalLog && Title && *Title)
      fwprintf(fp,L"%s\n",Title);

    wchar_t TmpBuf[17];
    int I;
    TmpBuf[16]=0;

    for(Y=0; Y < CY; ++Y)
    {
      //memset(TmpBuf,' ',16);
      fwprintf(fp,L"%s %s ",PrintTime(timebuf),MakeSpace());
      fwprintf(fp, L" %08X: ",StartAddress+Y*16);
      for(X=0; X < 16; ++X)
      {
        if((I=Y*16+X < SizeBuf) != 0)
          fwprintf(fp,L"%02X ",Buf[Y*16+X]&0xFF);
        else
          fwprintf(fp,L"   ");
        TmpBuf[X]=I?(Buf[Y*16+X] < 32?L'.':Buf[Y*16+X]):L' ';
        if(X == 7)
          fwprintf(fp,L" ");
      }
      //for(X < 16; ++X)

      fwprintf(fp,L"| %s\n",TmpBuf);
    }
    fwprintf(fp,L"%s %s}</%s>\n",PrintTime(timebuf),MakeSpace(),NullToEmpty(Title));
    fflush(fp);
  }

  if(InternalLog)
    CloseSysLog();
#endif
}


void SaveScreenDumpBuffer(const wchar_t *Title,const CHAR_INFO *Buffer,int X1,int Y1,int X2,int Y2,int RealScreen,FILE *fp)
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
      wchar_t timebuf[64];
      fwprintf(fp,L"%s %s(CHAR_INFO DumpBuffer: '%s')\n",PrintTime(timebuf),MakeSpace(),NullToEmpty(Title));
    }
  }

  wchar_t *line=new wchar_t[X2-X1+4];
  if (fp && line)
  {
    int x,y,i;

    if(!InternalLog && Title && *Title)
      fwprintf(fp,L"CHAR_INFO DumpBuffer: %s\n",Title);

    fwprintf(fp,L"XY={%i,%i - %i,%i}, RealScreen=%i\n",X1,Y1,X2,Y2,RealScreen);

    for (y=Y1; y <= Y2; y++)
    {
      fwprintf(fp,L"%04d: ",y);
      for (i=0, x=X1; x <= X2; x++, ++i)
      {
        line[i]=Buffer->Char.UnicodeChar?Buffer->Char.UnicodeChar:L' ';
        Buffer++;
      }
      line[i]=0;
      fwprintf(fp,L"%s\n",line);
    }
    fwprintf(fp,L"\n");
    fflush(fp);
  }

  if (line)
    delete[] line;

  if(InternalLog)
    CloseSysLog();
#endif
}



void PluginsStackItem_Dump(const wchar_t *Title,const PluginsStackItem *StackItems,int ItemNumber,FILE *fp)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

  int InternalLog=fp==NULL?TRUE:FALSE;

  if(InternalLog)
  {
    OpenSysLog();
    fp=PrintBaner(fp,L"",Title);
  }

  if (fp)
  {
    #define DEF_SORTMODE_(m) { m , L#m }
    static struct SORTMode{
      int Mode;
      const wchar_t *Name;
    } __SORT[]={
      DEF_SORTMODE_(UNSORTED),  DEF_SORTMODE_(BY_NAME),  DEF_SORTMODE_(BY_EXT),
      DEF_SORTMODE_(BY_MTIME),  DEF_SORTMODE_(BY_CTIME), DEF_SORTMODE_(BY_ATIME),
      DEF_SORTMODE_(BY_SIZE),   DEF_SORTMODE_(BY_DIZ),   DEF_SORTMODE_(BY_OWNER),
			DEF_SORTMODE_(BY_COMPRESSEDSIZE),DEF_SORTMODE_(BY_NUMLINKS),
			DEF_SORTMODE_(BY_NUMSTREAMS),DEF_SORTMODE_(BY_STREAMSSIZE),
    };

    if(!StackItems || !ItemNumber)
      fwprintf(fp,L"\tPluginsStackItem <EMPTY>");
    else
    {
      for(int I=ItemNumber-1; I >= 0; --I)
        fwprintf(fp,L"\t[%d]: "
                  L"hPlugin=%p "
                  L"Modified=%s "
                  L"PrevViewMode=VIEW_%02d "
                  L"PrevSortMode=%d/%-17s "
                  L"PrevSortOrder=%02d "
                  L"PrevNumericSort=%02d "
                  L"HostFile=%s\n",
         I,
         StackItems[I].hPlugin,
         (StackItems[I].Modified?L"True ":L"False"),
         StackItems[I].PrevViewMode,
         StackItems[I].PrevSortMode,
           (StackItems[I].PrevSortMode<BY_STREAMSSIZE?__SORT[StackItems[I].PrevSortMode].Name:L"<Unknown>"),
         StackItems[I].PrevSortOrder,
         StackItems[I].PrevNumericSort,
         (const wchar_t*)StackItems[I].strHostFile);
    }
    fwprintf(fp,L"\n");
    fflush(fp);
  }

  if(InternalLog)
    CloseSysLog();
#endif
}

void GetOpenPluginInfo_Dump(const wchar_t *Title,const OpenPluginInfo *Info,FILE *fp)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

  int InternalLog=fp==NULL?TRUE:FALSE;

  if(InternalLog)
  {
    OpenSysLog();
    fp=PrintBaner(fp,L"OpenPluginInfo",Title);
  }

  if (fp)
  {
    fwprintf(fp,L"\tStructSize      =%d\n",Info->StructSize);
    fwprintf(fp,L"\tFlags           =0x%08X\n",Info->Flags);
    fwprintf(fp,L"\tHostFile        ='%s'\n",NullToEmpty(Info->HostFile));
    fwprintf(fp,L"\tCurDir          ='%s'\n",NullToEmpty(Info->CurDir));
    fwprintf(fp,L"\tFormat          ='%s'\n",NullToEmpty(Info->Format));
    fwprintf(fp,L"\tPanelTitle      ='%s'\n",NullToEmpty(Info->PanelTitle));
    fwprintf(fp,L"\tInfoLines       =%p\n",Info->InfoLines);
    fwprintf(fp,L"\tInfoLinesNumber =%d\n",Info->InfoLinesNumber);
    if(Info->InfoLines)
    {
      for(int I=0;I<Info->InfoLinesNumber;++I)
      {
        fwprintf(fp,L"\t\t%d) Text=[%s], Data=[%s], Separator=[%d]\n",I,
           NullToEmpty(Info->InfoLines[I].Text),NullToEmpty(Info->InfoLines[I].Data),Info->InfoLines[I].Separator);
      }
    }
    fwprintf(fp,L"\tDescrFiles      =%p\n",Info->DescrFiles);
    fwprintf(fp,L"\tDescrFilesNumber=%d\n",Info->DescrFilesNumber);
    fwprintf(fp,L"\tPanelModesArray =%p\n",Info->PanelModesArray);
    fwprintf(fp,L"\tPanelModesNumber=%d\n",Info->PanelModesNumber);
    if(Info->PanelModesArray)
    {
      for(int I=0;I<Info->PanelModesNumber;++I)
      {
        fwprintf(fp,L"\t%d) ------------------\n",I);
        fwprintf(fp,L"\t\tColumnTypes       ='%s'\n",NullToEmpty(Info->PanelModesArray[I].ColumnTypes));
        fwprintf(fp,L"\t\tColumnWidths      ='%s'\n",NullToEmpty(Info->PanelModesArray[I].ColumnWidths));
        fwprintf(fp,L"\t\tColumnTitles      =%p\n",Info->PanelModesArray[I].ColumnTitles);
        fwprintf(fp,L"\t\tFullScreen        =%d\n",Info->PanelModesArray[I].FullScreen);
        fwprintf(fp,L"\t\tDetailedStatus    =%d\n",Info->PanelModesArray[I].DetailedStatus);
        fwprintf(fp,L"\t\tAlignExtensions   =%d\n",Info->PanelModesArray[I].AlignExtensions);
        fwprintf(fp,L"\t\tCaseConversion    =%d\n",Info->PanelModesArray[I].CaseConversion);
        fwprintf(fp,L"\t\tStatusColumnTypes ='%s'\n",NullToEmpty(Info->PanelModesArray[I].StatusColumnTypes));
        fwprintf(fp,L"\t\tStatusColumnWidths='%s'\n",NullToEmpty(Info->PanelModesArray[I].StatusColumnWidths));
      }
    }
    fwprintf(fp,L"\tStartPanelMode  =%d\n",Info->StartPanelMode);
    fwprintf(fp,L"\tStartSortMode   =%d\n",Info->StartSortMode);
    fwprintf(fp,L"\tStartSortOrder  =%d\n",Info->StartSortOrder);
    fwprintf(fp,L"\tKeyBar          =%p\n",Info->KeyBar);
    fwprintf(fp,L"\tShortcutData    =%p\n",Info->ShortcutData);

    fwprintf(fp,L"\n");
    fflush(fp);
  }

  if(InternalLog)
    CloseSysLog();
#endif
}


void ManagerClass_Dump(const wchar_t *Title,const Manager *m,FILE *fp)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

  int InternalLog=fp==NULL?TRUE:FALSE;

  if(InternalLog)
  {
    OpenSysLog();
    fp=PrintBaner(fp,L"",Title);
  }

  if (fp)
  {
    const Manager *Man=(m==NULL?FrameManager:m);
//StartSysLog
    int I;
    string Type,Name;
    fwprintf(fp,L"**** Queue modal frames ***\nFrameListSize=%d, FramePos=%d, FrameCount=%d\n",Man->FrameListSize,Man->FramePos,Man->FrameCount);
    if(Man->FrameList)
    {
      for(I=0; I < Man->FrameCount; ++I)
      {
        if(Man->FrameList[I])
        {
          Man->FrameList[I]->GetTypeAndName(Type,Name);
          fwprintf(fp,L"\tFrameList[%d] %p  Type='%s' Name='%s'\n",I,Man->FrameList[I],(const wchar_t*)Type,(const wchar_t*)Name);
        }
        else
          fwprintf(fp,L"\tFrameList[%d] NULL\n",I);
      }
    }
    else
     fwprintf(fp,L"\tFrameList = NULL\n");

    fwprintf(fp,L"**** Stack modal frames ***\nModalStackSize=%d\n",Man->ModalStackSize);
    if(Man->ModalStack)
    {
      for(I=0; I < Man->ModalStackCount; ++I)
      {
        if(Man->ModalStack[I])
        {
          Man->ModalStack[I]->GetTypeAndName(Type,Name);
          fwprintf(fp,L"\tModalStack[%d] %p  Type='%s' Name='%s'\n",
                      I,Man->ModalStack[I],(const wchar_t*)Type,(const wchar_t*)Name);
        }
        else
          fwprintf(fp,L"\tModalStack[%d] NULL\n",I);
      }
    }
    else
     fwprintf(fp,L"\tModalStack = NULL\n");

    fwprintf(fp,L"**** Detail... ***\n");

    if(!Man->InsertedFrame)
      Type=L"", Name=L"";
    else
      Man->InsertedFrame->GetTypeAndName(Type,Name);
    fwprintf(fp,L"\tInsertedFrame=%p (Type='%s' Name='%s')\n", //  - Фрейм, который будет добавлен в конец немодальной очереди
      Man->InsertedFrame,(const wchar_t*)Type,(const wchar_t*)Name);

    if(!Man->DeletedFrame)
      Type=L"", Name=L"";
    else
      Man->DeletedFrame->GetTypeAndName(Type,Name);
    fwprintf(fp,L"\tDeletedFrame=%p (Type='%s' Name='%s')\n", //  - Фрейм, предназначен для удаления из модальной очереди, из модального стека, либо одиночный (которого нет ни там, ни там)
       Man->DeletedFrame,(const wchar_t*)Type,(const wchar_t*)Name);

    if(!Man->ActivatedFrame)
      Type=L"", Name=L"";
    else
      Man->ActivatedFrame->GetTypeAndName(Type,Name);
    fwprintf(fp,L"\tActivatedFrame=%p (Type='%s' Name='%s')\n", //  - Фрейм, который необходимо активировать после каких ни будь изменений
      Man->ActivatedFrame,(const wchar_t*)Type,(const wchar_t*)Name);

    if(!Man->RefreshedFrame)
      Type=L"", Name=L"";
    else
      Man->RefreshedFrame->GetTypeAndName(Type,Name);
    fwprintf(fp,L"\tRefreshedFrame=%p (Type='%s' Name='%s')\n", //  - Фрейм, который нужно просто освежить, т.е. перерисовать
      Man->RefreshedFrame,(const wchar_t*)Type,(const wchar_t*)Name);

    if(!Man->ModalizedFrame)
      Type=L"", Name=L"";
    else
      Man->ModalizedFrame->GetTypeAndName(Type,Name);
    fwprintf(fp,L"\tModalizedFrame=%p (Type='%s' Name='%s')\n", //  - Фрейм, который становится в 'очередь' к текущему немодальному фрейму
      Man->ModalizedFrame,(const wchar_t*)Type,(const wchar_t*)Name);

    if(!Man->UnmodalizedFrame)
      Type=L"", Name=L"";
    else
      Man->UnmodalizedFrame->GetTypeAndName(Type,Name);
    fwprintf(fp,L"\tUnmodalizedFrame=%p (Type='%s' Name='%s')\n", //  - Фрейм, убирающийся из 'очереди' немодального фрейма
      Man->UnmodalizedFrame,(const wchar_t*)Type,(const wchar_t*)Name);

    if(!Man->DeactivatedFrame)
      Type=L"", Name=L"";
    else
      Man->DeactivatedFrame->GetTypeAndName(Type,Name);
    fwprintf(fp,L"\tDeactivatedFrame=%p (Type='%s' Name='%s')\n", //  - Фрейм, который указывает на предыдущий активный фрейм
      Man->DeactivatedFrame,(const wchar_t*)Type,(const wchar_t*)Name);


    if(!Man->ExecutedFrame)
      Type=L"", Name=L"";
    else
      Man->ExecutedFrame->GetTypeAndName(Type,Name);
    fwprintf(fp,L"\tExecutedFrame=%p (Type='%s' Name='%s')\n", //  - Фрейм, которого вскорости нужно будет поставить на вершину модального стека
      Man->ExecutedFrame,(const wchar_t*)Type,(const wchar_t*)Name);

    if(!Man->CurrentFrame)
      Type=L"", Name=L"";
    else
      Man->CurrentFrame->GetTypeAndName(Type,Name);
    fwprintf(fp,L"\tCurrentFrame=%p (Type='%s' Name='%s')\n", //  - текущий фрейм. Он может находиться как в немодальной очереди, так и в модальном стеке
      Man->CurrentFrame,(const wchar_t*)Type,(const wchar_t*)Name);

    fwprintf(fp,L"\n");
    fflush(fp);
  }

  if(InternalLog)
    CloseSysLog();
#endif
}


#if defined(SYSLOG_FARSYSLOG)
void WINAPIV _export FarSysLog(const wchar_t *ModuleName,int l,const wchar_t *fmt,...)
{
  if(!IsLogON())
    return;

  wchar_t msg[MAX_LOG_LINE];

  va_list argptr;
  va_start( argptr, fmt );
	_vsnwprintf( msg, countof(msg)-1, fmt, argptr );
  va_end(argptr);

  SysLog(l);
  OpenSysLog();
  if ( LogStream )
  {
    wchar_t timebuf[64];
    fwprintf(LogStream,L"%s %s%s:: %s\n",PrintTime(timebuf),MakeSpace(),PointToName(ModuleName),msg);
    fflush(LogStream);
  }
  CloseSysLog();
  if(IsDebuggerPresent())
  {
		OutputDebugString(msg);
#ifdef _MSC_VER
		OutputDebugString(L"\n");
#endif _MSC_VER
  }
}

void WINAPI _export FarSysLogDump(const wchar_t *ModuleName,DWORD StartAddress,LPBYTE Buf,int SizeBuf)
{
  if(!IsLogON())
    return;

  SysLogDump(ModuleName,StartAddress,Buf,SizeBuf,NULL);
}

void WINAPI _export FarSysLog_INPUT_RECORD_Dump(const wchar_t *ModuleName,INPUT_RECORD *rec)
{
  if(!IsLogON())
    return;

  SysLog(L"%s {%s}",ModuleName,_INPUT_RECORD_Dump(rec));
}
#endif

// "Умный класс для SysLog
CleverSysLog::CleverSysLog(const wchar_t *Title)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

  SysLog(1,L"%s{",Title?Title:L"");
#endif
}
CleverSysLog::~CleverSysLog()
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

  SysLog(-1,L"}");
#endif
}

#if defined(SYSLOG)
struct __XXX_Name{
  DWORD Val;
  const wchar_t *Name;
};

static string _XXX_ToName(int Val,const wchar_t *Pref,__XXX_Name *arrDef,int cntArr)
{
  int I;
  string Name;
  for(I=0; I < cntArr; ++I)
    if(arrDef[I].Val == Val)
    {
      Name.Format(L"\"%s_%s\" [%d/0x%04X]",Pref,arrDef[I].Name,Val,Val);
      return Name;
    }
  Name.Format(L"\"%s_????\" [%d/0x%04X]",Pref,Val,Val);
  return Name;
}
#endif


string __ECTL_ToName(int Command)
{
#if defined(SYSLOG_KEYMACRO) || defined(SYSLOG_ECTL)
  #define DEF_ECTL_(m) { ECTL_##m , L#m }
	__XXX_Name ECTL[]={
    DEF_ECTL_(GETSTRING),      DEF_ECTL_(SETSTRING),
    DEF_ECTL_(INSERTSTRING),   DEF_ECTL_(DELETESTRING),
    DEF_ECTL_(DELETECHAR),     DEF_ECTL_(INSERTTEXT),
    DEF_ECTL_(GETINFO),        DEF_ECTL_(SETPOSITION),
    DEF_ECTL_(SELECT),         DEF_ECTL_(REDRAW),
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
	return _XXX_ToName(Command,L"ECTL",ECTL,countof(ECTL));
#else
  return L"";
#endif
}

string __EE_ToName(int Command)
{
#if defined(SYSLOG)
  #define DEF_EE_(m) { EE_##m , L#m }
	__XXX_Name EE[]={
    DEF_EE_(READ),     DEF_EE_(SAVE),     DEF_EE_(REDRAW),     DEF_EE_(CLOSE),
    DEF_EE_(GOTFOCUS), DEF_EE_(KILLFOCUS),
  };

	return _XXX_ToName(Command,L"EE",EE,countof(EE));
#else
  return L"";
#endif
}

string __EEREDRAW_ToName(int Command)
{
#if defined(SYSLOG)
  #define DEF_EEREDRAW_(m) { (int)(INT_PTR)EEREDRAW_##m , L#m }
	__XXX_Name EEREDRAW[]={
    DEF_EEREDRAW_(ALL),  DEF_EEREDRAW_(CHANGE),  DEF_EEREDRAW_(LINE),
  };

	return _XXX_ToName(Command,L"EEREDRAW",EEREDRAW,countof(EEREDRAW));
#else
  return L"";
#endif
}

string __ESPT_ToName(int Command)
{
#if defined(SYSLOG_KEYMACRO) || defined(SYSLOG_ECTL)
  #define DEF_ESPT_(m) { ESPT_##m , L#m }
	__XXX_Name ESPT[]={
    DEF_ESPT_(TABSIZE),
    DEF_ESPT_(EXPANDTABS),
    DEF_ESPT_(AUTOINDENT),
    DEF_ESPT_(CURSORBEYONDEOL),
    DEF_ESPT_(CHARCODEBASE),
    //DEF_ESPT_(CHARTABLE),
    DEF_ESPT_(SAVEFILEPOSITION),
    DEF_ESPT_(LOCKMODE),
    DEF_ESPT_(SETWORDDIV),
  };
	return _XXX_ToName(Command,L"ESPT",ESPT,countof(ESPT));
#else
  return L"";
#endif
}

string __VE_ToName(int Command)
{
#if defined(SYSLOG)
  #define DEF_VE_(m) { VE_##m , L#m }
	__XXX_Name VE[]={
    DEF_VE_(READ),     DEF_VE_(CLOSE),
    DEF_VE_(GOTFOCUS), DEF_VE_(KILLFOCUS),
  };

	return _XXX_ToName(Command,L"VE",VE,countof(VE));
#else
  return L"";
#endif
}

string __FCTL_ToName(int Command)
{
#if defined(SYSLOG)
  #define DEF_FCTL_(m) { FCTL_##m , L#m }
	__XXX_Name FCTL[]={
     DEF_FCTL_(CLOSEPLUGIN),           DEF_FCTL_(GETPANELINFO),
     DEF_FCTL_(UPDATEPANEL),
     DEF_FCTL_(REDRAWPANEL),
     DEF_FCTL_(GETCMDLINE),            DEF_FCTL_(SETCMDLINE),
     DEF_FCTL_(SETSELECTION),
     DEF_FCTL_(SETVIEWMODE),
     DEF_FCTL_(INSERTCMDLINE),         DEF_FCTL_(SETUSERSCREEN),
     DEF_FCTL_(SETPANELDIR),           DEF_FCTL_(SETCMDLINEPOS),
     DEF_FCTL_(GETCMDLINEPOS),         DEF_FCTL_(SETSORTMODE),
     DEF_FCTL_(SETSORTORDER),
     DEF_FCTL_(GETCMDLINESELECTEDTEXT),
     DEF_FCTL_(SETCMDLINESELECTION),   DEF_FCTL_(GETCMDLINESELECTION),
     DEF_FCTL_(CHECKPANELSEXIST),      DEF_FCTL_(SETNUMERICSORT),
  };
	return _XXX_ToName(Command,L"FCTL",FCTL,countof(FCTL));
#else
  return L"";
#endif
}

string __ACTL_ToName(int Command)
{
#if defined(SYSLOG_ACTL)
#define DEF_ACTL_(m) { ACTL_##m , L#m }
	__XXX_Name ACTL[]={
    DEF_ACTL_(GETFARVERSION),          DEF_ACTL_(CONSOLEMODE),
    DEF_ACTL_(GETSYSWORDDIV),          DEF_ACTL_(WAITKEY),
    DEF_ACTL_(GETCOLOR),               DEF_ACTL_(GETARRAYCOLOR),
    DEF_ACTL_(EJECTMEDIA),             DEF_ACTL_(KEYMACRO),
    DEF_ACTL_(POSTKEYSEQUENCE),        DEF_ACTL_(GETWINDOWINFO),
    DEF_ACTL_(GETWINDOWCOUNT),         DEF_ACTL_(SETCURRENTWINDOW),
    DEF_ACTL_(COMMIT),                 DEF_ACTL_(GETFARHWND),
    DEF_ACTL_(GETSYSTEMSETTINGS),      DEF_ACTL_(GETPANELSETTINGS),
    DEF_ACTL_(GETINTERFACESETTINGS),   DEF_ACTL_(GETCONFIRMATIONS),
    DEF_ACTL_(GETDESCSETTINGS),        DEF_ACTL_(SETARRAYCOLOR),
    DEF_ACTL_(GETWCHARMODE),           DEF_ACTL_(GETPLUGINMAXREADDATA),
    DEF_ACTL_(GETDIALOGSETTINGS),      DEF_ACTL_(GETSHORTWINDOWINFO),
    DEF_ACTL_(REMOVEMEDIA),            DEF_ACTL_(GETMEDIATYPE),
    DEF_ACTL_(GETPOLICIES),            DEF_ACTL_(REDRAWALL),
  };
	return _XXX_ToName(Command,L"ACTL",ACTL,countof(ACTL));

#else
  return L"";
#endif
}


string __VCTL_ToName(int Command)
{
#if defined(SYSLOG_VCTL)
#define DEF_VCTL_(m) { VCTL_##m , L#m }
	__XXX_Name VCTL[]={
    DEF_VCTL_(GETINFO),
    DEF_VCTL_(QUIT),
    DEF_VCTL_(REDRAW),
    DEF_VCTL_(SETKEYBAR),
    DEF_VCTL_(SETPOSITION),
    DEF_VCTL_(SELECT),
    DEF_VCTL_(SETMODE),
  };
	return _XXX_ToName(Command,L"VCTL",VCTL,countof(VCTL));
#else
  return L"";
#endif
}


string __MCODE_ToName(int OpCode)
{
#if defined(SYSLOG)
#define DEF_MCODE_(m) { MCODE_##m , L#m }
	__XXX_Name MCODE[]={
     DEF_MCODE_(C_APANEL_BOF),
     DEF_MCODE_(C_APANEL_EOF),
     DEF_MCODE_(C_APANEL_FILEPANEL),
     DEF_MCODE_(C_APANEL_FOLDER),
     DEF_MCODE_(C_APANEL_ISEMPTY),
     DEF_MCODE_(C_APANEL_LEFT),
     DEF_MCODE_(C_APANEL_LFN),
     DEF_MCODE_(C_APANEL_PLUGIN),
     DEF_MCODE_(C_APANEL_ROOT),
     DEF_MCODE_(C_APANEL_SELECTED),
     DEF_MCODE_(C_APANEL_VISIBLE),
     DEF_MCODE_(C_AREA_DIALOG),
     DEF_MCODE_(C_AREA_DISKS),
     DEF_MCODE_(C_AREA_EDITOR),
     DEF_MCODE_(C_AREA_FINDFOLDER),
     DEF_MCODE_(C_AREA_HELP),
     DEF_MCODE_(C_AREA_INFOPANEL),
     DEF_MCODE_(C_AREA_MAINMENU),
     DEF_MCODE_(C_AREA_MENU),
     DEF_MCODE_(C_AREA_OTHER),
     DEF_MCODE_(C_AREA_QVIEWPANEL),
     DEF_MCODE_(C_AREA_SEARCH),
     DEF_MCODE_(C_AREA_SHELL),
     DEF_MCODE_(C_AREA_TREEPANEL),
     DEF_MCODE_(C_AREA_USERMENU),
     DEF_MCODE_(C_AREA_VIEWER),
     DEF_MCODE_(C_BOF),
     DEF_MCODE_(C_CMDLINE_BOF),
     DEF_MCODE_(C_CMDLINE_EMPTY),
     DEF_MCODE_(C_CMDLINE_EOF),
     DEF_MCODE_(C_CMDLINE_SELECTED),
     DEF_MCODE_(C_DISABLEOUTPUT),
     DEF_MCODE_(C_EMPTY),
     DEF_MCODE_(C_EOF),
     DEF_MCODE_(C_ICLIP),
     DEF_MCODE_(C_PPANEL_BOF),
     DEF_MCODE_(C_PPANEL_EOF),
     DEF_MCODE_(C_PPANEL_FILEPANEL),
     DEF_MCODE_(C_PPANEL_FOLDER),
     DEF_MCODE_(C_PPANEL_ISEMPTY),
     DEF_MCODE_(C_PPANEL_LEFT),
     DEF_MCODE_(C_PPANEL_LFN),
     DEF_MCODE_(C_PPANEL_PLUGIN),
     DEF_MCODE_(C_PPANEL_ROOT),
     DEF_MCODE_(C_PPANEL_SELECTED),
     DEF_MCODE_(C_PPANEL_VISIBLE),
     DEF_MCODE_(C_ROOTFOLDER),
     DEF_MCODE_(C_SELECTED),
     DEF_MCODE_(C_WINDOWEDMODE),
     DEF_MCODE_(F_ABS),
     DEF_MCODE_(F_AKEY),
     DEF_MCODE_(F_ASC),
     DEF_MCODE_(F_CHR),
     DEF_MCODE_(F_CLIP),
     DEF_MCODE_(F_DATE),
     DEF_MCODE_(F_DLG_GETVALUE),
     DEF_MCODE_(F_EDITOR_SET),
     DEF_MCODE_(F_EDITOR_SEL),
     DEF_MCODE_(F_KEY),
     DEF_MCODE_(F_CALLPLUGIN),
     DEF_MCODE_(F_ENVIRON),
     DEF_MCODE_(F_EVAL),
     DEF_MCODE_(F_FATTR),
     DEF_MCODE_(F_FEXIST),
     DEF_MCODE_(F_FLOCK),
     DEF_MCODE_(F_FSPLIT),
     DEF_MCODE_(F_TRIM),
     DEF_MCODE_(F_IIF),
     DEF_MCODE_(F_INDEX),
     DEF_MCODE_(F_INT),
     DEF_MCODE_(F_ITOA),
     DEF_MCODE_(F_ATOI),
     DEF_MCODE_(F_LCASE),
     DEF_MCODE_(F_LEN),
     DEF_MCODE_(F_MAX),
     DEF_MCODE_(F_MENU_CHECKHOTKEY),
     DEF_MCODE_(F_MENU_GETHOTKEY),           // S=gethotkey()
     DEF_MCODE_(F_MIN),
     DEF_MCODE_(F_MSAVE),
     DEF_MCODE_(F_MSGBOX),
     DEF_MCODE_(F_PROMPT),              // S=prompt("Title"[,"Prompt"[,flags[, "Src"[, "History"]]]])
     DEF_MCODE_(F_NOFUNC),
     DEF_MCODE_(F_PANEL_FATTR),
     DEF_MCODE_(F_PANEL_FEXIST),
     DEF_MCODE_(F_PANEL_SETPOS),
     DEF_MCODE_(F_PANEL_SETPOSIDX),
     DEF_MCODE_(F_PANELITEM),
     DEF_MCODE_(F_RINDEX),
     DEF_MCODE_(F_SLEEP),
     DEF_MCODE_(F_STRING),
     DEF_MCODE_(F_SUBSTR),
     DEF_MCODE_(F_UCASE),
     DEF_MCODE_(F_WAITKEY),
     DEF_MCODE_(F_XLAT),
     DEF_MCODE_(F_BM_ADD),              // N=BM.Add()
     DEF_MCODE_(F_BM_CLEAR),            // N=BM.Clear()
     DEF_MCODE_(F_BM_NEXT),             // N=BM.Next()
     DEF_MCODE_(F_BM_PREV),             // N=BM.Prev()
     DEF_MCODE_(F_BM_STAT),             // N=BM.Stat()
     DEF_MCODE_(F_BM_GET),
     DEF_MCODE_(F_BM_DEL),
     DEF_MCODE_(OP_ADD),
     DEF_MCODE_(OP_AKEY),
     DEF_MCODE_(OP_AND),
     DEF_MCODE_(OP_BITAND),
     DEF_MCODE_(OP_BITOR),
     DEF_MCODE_(OP_BITXOR),
     DEF_MCODE_(OP_COPY),
     DEF_MCODE_(OP_DATE),
     DEF_MCODE_(OP_DISCARD),
     DEF_MCODE_(OP_DIV),
     DEF_MCODE_(OP_ELSE),
     DEF_MCODE_(OP_END),
     DEF_MCODE_(OP_ENDKEYS),
     DEF_MCODE_(OP_EQ),
     DEF_MCODE_(OP_EXIT),
     DEF_MCODE_(OP_GE),
     DEF_MCODE_(OP_GT),
     DEF_MCODE_(OP_ICLIP),
     DEF_MCODE_(OP_IF),
     DEF_MCODE_(OP_JGE),
     DEF_MCODE_(OP_JGT),
     DEF_MCODE_(OP_JLE),
     DEF_MCODE_(OP_JLT),
     DEF_MCODE_(OP_JMP),
     DEF_MCODE_(OP_NOP),
     DEF_MCODE_(OP_JNZ),
     DEF_MCODE_(OP_JZ),
     DEF_MCODE_(OP_KEYS),
     DEF_MCODE_(OP_LE),
     DEF_MCODE_(OP_LT),
     DEF_MCODE_(OP_MACROMODE),
     DEF_MCODE_(OP_MUL),
     DEF_MCODE_(OP_NE),
     DEF_MCODE_(OP_NEGATE),
     DEF_MCODE_(OP_NOT),
     DEF_MCODE_(OP_OR),
     DEF_MCODE_(OP_PLAINTEXT),
     DEF_MCODE_(OP_POP),
     DEF_MCODE_(OP_PUSHINT),
     DEF_MCODE_(OP_PUSHSTR),
     DEF_MCODE_(OP_PUSHVAR),
     DEF_MCODE_(OP_PUSHCONST),
     DEF_MCODE_(OP_REP),
     DEF_MCODE_(OP_SAVE),
     DEF_MCODE_(OP_SAVEREPCOUNT),
     DEF_MCODE_(OP_SELWORD),
     DEF_MCODE_(OP_SUB),
     DEF_MCODE_(OP_SWITCHKBD),
     DEF_MCODE_(OP_WHILE),
     DEF_MCODE_(OP_XLAT),
     DEF_MCODE_(V_APANEL_COLUMNCOUNT),
     DEF_MCODE_(V_APANEL_CURPOS),
     DEF_MCODE_(V_APANEL_CURRENT),
     DEF_MCODE_(V_APANEL_DRIVETYPE),
     DEF_MCODE_(V_APANEL_HEIGHT),
     DEF_MCODE_(V_APANEL_ITEMCOUNT),
     DEF_MCODE_(V_APANEL_OPIFLAGS),
     DEF_MCODE_(V_APANEL_PATH),
     DEF_MCODE_(V_APANEL_SELCOUNT),
     DEF_MCODE_(V_APANEL_TYPE),
     DEF_MCODE_(V_APANEL_UNCPATH),
     DEF_MCODE_(V_APANEL_WIDTH),
     DEF_MCODE_(V_CMDLINE_CURPOS),
     DEF_MCODE_(V_CMDLINE_ITEMCOUNT),
     DEF_MCODE_(V_CMDLINE_VALUE),
     DEF_MCODE_(V_CURPOS),
     DEF_MCODE_(V_DLGCURPOS),
     DEF_MCODE_(V_DLGITEMCOUNT),
     DEF_MCODE_(V_DLGITEMTYPE),
     DEF_MCODE_(V_DRVSHOWMODE),
     DEF_MCODE_(V_DRVSHOWPOS),
     DEF_MCODE_(V_EDITORCURLINE),
     DEF_MCODE_(V_EDITORCURPOS),
     DEF_MCODE_(V_EDITORREALPOS),      // Editor.RealPos - текущая поз. в редакторе без привязки к размеру табуляции
     DEF_MCODE_(V_EDITORFILENAME),
     DEF_MCODE_(V_EDITORLINES),
     DEF_MCODE_(V_EDITORSTATE),
     DEF_MCODE_(V_EDITORVALUE),
     DEF_MCODE_(V_FAR_HEIGHT),
     DEF_MCODE_(V_FAR_TITLE),
     DEF_MCODE_(V_FAR_WIDTH),
     DEF_MCODE_(V_HEIGHT),
     DEF_MCODE_(V_HELPFILENAME),
     DEF_MCODE_(V_HELPSELTOPIC),
     DEF_MCODE_(V_HELPTOPIC),
     DEF_MCODE_(V_ITEMCOUNT),
     DEF_MCODE_(V_PPANEL_COLUMNCOUNT),
     DEF_MCODE_(V_PPANEL_CURPOS),
     DEF_MCODE_(V_PPANEL_CURRENT),
     DEF_MCODE_(V_PPANEL_DRIVETYPE),
     DEF_MCODE_(V_PPANEL_HEIGHT),
     DEF_MCODE_(V_PPANEL_ITEMCOUNT),
     DEF_MCODE_(V_PPANEL_OPIFLAGS),
     DEF_MCODE_(V_PPANEL_PATH),
     DEF_MCODE_(V_PPANEL_SELCOUNT),
     DEF_MCODE_(V_PPANEL_TYPE),
     DEF_MCODE_(V_PPANEL_UNCPATH),
     DEF_MCODE_(V_PPANEL_WIDTH),
     DEF_MCODE_(V_TITLE),
     DEF_MCODE_(V_VIEWERFILENAME),
     DEF_MCODE_(V_VIEWERSTATE),
     DEF_MCODE_(V_WIDTH),
  };

  int I;
  string Name;
	for(I=0; I < countof(MCODE); ++I)
    if(MCODE[I].Val == OpCode)
    {
      Name.Format(L"%08X | MCODE_%-20s",OpCode,MCODE[I].Name);
      return Name;
    }
  Name.Format(L"%08X | MCODE_%-20s",OpCode,L"???");
  return Name;
#else
  return L"";
#endif
}

string __FARKEY_ToName(int Key)
{
#if defined(SYSLOG)
  string Name;
  if(KeyToText(Key,Name))
  {
    string tmp;
    InsertQuote(Name);
    tmp.Format(L"%s [%u/0x%08X]",(const wchar_t*)Name,Key,Key);
    Name = tmp;
    return Name;
  }
  Name.Format(L"\"KEY_????\" [%u/0x%08X]",Key,Key);
  return Name;
#else
  return L"";
#endif
}


string __DLGMSG_ToName(int Msg)
{
#if defined(SYSLOG)
  #define DEF_MESSAGE(m) { m , L#m }
	__XXX_Name Message[]={
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
    DEF_MESSAGE(DN_DRAWDIALOGDONE),
  };
  int I;

  string Name;
	for(I=0; I < countof(Message); ++I)
    if(Message[I].Val == Msg)
    {
      Name.Format(L"\"%s\" [%d/0x%08X]",Message[I].Name,Msg,Msg);
      return Name;
    }
  Name.Format(L"\"%s+[%d/0x%08X]\"",(Msg>=DN_FIRST?L"DN_FIRST":(Msg>=DM_USER?L"DM_USER":L"DM_FIRST")),Msg,Msg);
  return Name;
#else
  return L"";
#endif
}

string __VK_KEY_ToName(int VkKey)
{
#if defined(SYSLOG)
  #define DEF_VK(k) { VK_##k , L#k }
	__XXX_Name VK[]={
    DEF_VK(ACCEPT),                           DEF_VK(ADD),
    DEF_VK(APPS),                             DEF_VK(ATTN),
    DEF_VK(BACK),                             DEF_VK(BROWSER_BACK),
    DEF_VK(BROWSER_FAVORITES),                DEF_VK(BROWSER_FORWARD),
    DEF_VK(BROWSER_HOME),                     DEF_VK(BROWSER_REFRESH),
    DEF_VK(BROWSER_SEARCH),                   DEF_VK(BROWSER_STOP),
    DEF_VK(CANCEL),                           DEF_VK(CAPITAL),
    DEF_VK(CLEAR),                            DEF_VK(CONTROL),
    DEF_VK(CONVERT),                          DEF_VK(CRSEL),
    DEF_VK(CRSEL),                            DEF_VK(DECIMAL),
    DEF_VK(DELETE),                           DEF_VK(DIVIDE),
    DEF_VK(DOWN),                             DEF_VK(END),
    DEF_VK(EREOF),                            DEF_VK(ESCAPE),
    DEF_VK(EXECUTE),                          DEF_VK(EXSEL),
    DEF_VK(F1),                               DEF_VK(F10),
    DEF_VK(F11),                              DEF_VK(F12),
    DEF_VK(F13),                              DEF_VK(F14),
    DEF_VK(F15),                              DEF_VK(F16),
    DEF_VK(F17),                              DEF_VK(F18),
    DEF_VK(F19),                              DEF_VK(F2),
    DEF_VK(F20),                              DEF_VK(F21),
    DEF_VK(F22),                              DEF_VK(F23),
    DEF_VK(F24),                              DEF_VK(F3),
    DEF_VK(F4),                               DEF_VK(F5),
    DEF_VK(F6),                               DEF_VK(F7),
    DEF_VK(F8),                               DEF_VK(F9),
    DEF_VK(HELP),                             DEF_VK(HOME),
    DEF_VK(ICO_00),                           DEF_VK(ICO_CLEAR),
    DEF_VK(ICO_HELP),                         DEF_VK(INSERT),
    DEF_VK(LAUNCH_APP1),                      DEF_VK(LAUNCH_APP2),
    DEF_VK(LAUNCH_MAIL),                      DEF_VK(LAUNCH_MEDIA_SELECT),
    DEF_VK(LBUTTON),                          DEF_VK(LCONTROL),
    DEF_VK(LEFT),                             DEF_VK(LMENU),
    DEF_VK(LSHIFT),                           DEF_VK(LWIN),
    DEF_VK(MBUTTON),                          DEF_VK(MEDIA_NEXT_TRACK),
    DEF_VK(MEDIA_PLAY_PAUSE),                 DEF_VK(MEDIA_PREV_TRACK),
    DEF_VK(MEDIA_STOP),                       DEF_VK(MENU),
    DEF_VK(MODECHANGE),                       DEF_VK(MULTIPLY),
    DEF_VK(NEXT),                             DEF_VK(NONAME),
    DEF_VK(NONCONVERT),                       DEF_VK(NUMLOCK),
    DEF_VK(NUMPAD0),                          DEF_VK(NUMPAD1),
    DEF_VK(NUMPAD2),                          DEF_VK(NUMPAD3),
    DEF_VK(NUMPAD4),                          DEF_VK(NUMPAD5),
    DEF_VK(NUMPAD6),                          DEF_VK(NUMPAD7),
    DEF_VK(NUMPAD8),                          DEF_VK(NUMPAD9),
    DEF_VK(OEM_1),                            DEF_VK(OEM_102),
    DEF_VK(OEM_2),                            DEF_VK(OEM_3),
    DEF_VK(OEM_4),                            DEF_VK(OEM_5),
    DEF_VK(OEM_6),                            DEF_VK(OEM_7),
    DEF_VK(OEM_8),                            DEF_VK(OEM_ATTN),
    DEF_VK(OEM_AUTO),                         DEF_VK(OEM_AX),
    DEF_VK(OEM_BACKTAB),                      DEF_VK(OEM_CLEAR),
    DEF_VK(OEM_COMMA),                        DEF_VK(OEM_COPY),
    DEF_VK(OEM_CUSEL),                        DEF_VK(OEM_ENLW),
    DEF_VK(OEM_FINISH),                       DEF_VK(OEM_JUMP),
    DEF_VK(OEM_MINUS),                        DEF_VK(OEM_PA1),
    DEF_VK(OEM_PA2),                          DEF_VK(OEM_PA3),
    DEF_VK(OEM_PERIOD),                       DEF_VK(OEM_PLUS),
    DEF_VK(OEM_RESET),                        DEF_VK(OEM_WSCTRL),
    DEF_VK(PA1),                              DEF_VK(PACKET),
    DEF_VK(PAUSE),                            DEF_VK(PLAY),
    DEF_VK(PRINT),                            DEF_VK(PRIOR),
    DEF_VK(PROCESSKEY),                       DEF_VK(RBUTTON),
    DEF_VK(RCONTROL),                         DEF_VK(RETURN),
    DEF_VK(RIGHT),                            DEF_VK(RMENU),
    DEF_VK(RSHIFT),                           DEF_VK(RWIN),
    DEF_VK(SCROLL),                           DEF_VK(SELECT),
    DEF_VK(SEPARATOR),                        DEF_VK(SHIFT),
    DEF_VK(SLEEP),                            DEF_VK(SNAPSHOT),
    DEF_VK(SPACE),                            DEF_VK(SUBTRACT),
    DEF_VK(TAB),                              DEF_VK(UP),
    DEF_VK(VOLUME_DOWN),                      DEF_VK(VOLUME_MUTE),
    DEF_VK(VOLUME_UP),                        DEF_VK(XBUTTON1),
    DEF_VK(XBUTTON2),                         DEF_VK(ZOOM),
  };


  if(VkKey >= L'0' && VkKey <= L'9' || VkKey >= L'A' && VkKey <= L'Z')
  {
    string Name;
    Name.Format(L"\"VK_%c\" [%d/0x%04X]",VkKey,VkKey,VkKey);
    return Name;
  }
  else
		return _XXX_ToName(VkKey,L"VK",VK,countof(VK));
#else
  return L"";
#endif
}

string __MOUSE_EVENT_RECORD_Dump(MOUSE_EVENT_RECORD *rec)
{
#if defined(SYSLOG)
  string Records;

  Records.Format(
        L"MOUSE_EVENT_RECORD: [%d,%d], Btn=0x%08X (%c%c%c%c%c), Ctrl=0x%08X (%c%c%c%c%c - %c%c%c%c), Flgs=0x%08X (%s)",
      rec->dwMousePosition.X,
      rec->dwMousePosition.Y,
      rec->dwButtonState,
        (rec->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED?L'L':L'l'),
        (rec->dwButtonState&RIGHTMOST_BUTTON_PRESSED?L'R':L'r'),
        (rec->dwButtonState&FROM_LEFT_2ND_BUTTON_PRESSED?L'2':L' '),
        (rec->dwButtonState&FROM_LEFT_3RD_BUTTON_PRESSED?L'3':L' '),
        (rec->dwButtonState&FROM_LEFT_4TH_BUTTON_PRESSED?L'4':L' '),
      rec->dwControlKeyState,
        (rec->dwControlKeyState&LEFT_CTRL_PRESSED?L'C':L'c'),
        (rec->dwControlKeyState&LEFT_ALT_PRESSED?L'A':L'a'),
        (rec->dwControlKeyState&SHIFT_PRESSED?L'S':L's'),
        (rec->dwControlKeyState&RIGHT_ALT_PRESSED?L'A':L'a'),
        (rec->dwControlKeyState&RIGHT_CTRL_PRESSED?L'C':L'c'),
        (rec->dwControlKeyState&ENHANCED_KEY?L'E':L'e'),
        (rec->dwControlKeyState&CAPSLOCK_ON?L'C':L'c'),
        (rec->dwControlKeyState&NUMLOCK_ON?L'N':L'n'),
        (rec->dwControlKeyState&SCROLLLOCK_ON?L'S':L's'),
      rec->dwEventFlags,
        (rec->dwEventFlags==DOUBLE_CLICK?L"(DblClick)":
         (rec->dwEventFlags==MOUSE_MOVED?L"(Moved)":
          (rec->dwEventFlags==MOUSE_WHEELED?L"(Wheel)":
           (rec->dwEventFlags==MOUSE_HWHEELED?L"(HWheel)":L""))))
    );
  if(rec->dwEventFlags==MOUSE_WHEELED  || rec->dwEventFlags==MOUSE_HWHEELED)
  {
    string tmp;
    tmp.Format(L" (Delta=%d)",(short)HIWORD(rec->dwButtonState));
    Records+=tmp;
  }
  return Records;
#else
  return L"";
#endif
}


string __INPUT_RECORD_Dump(INPUT_RECORD *rec)
{
#if defined(SYSLOG)
  string Records;

  switch(rec->EventType)
  {
    case FOCUS_EVENT:
      Records.Format(
            L"FOCUS_EVENT_RECORD: %s",
          (rec->Event.FocusEvent.bSetFocus?L"TRUE":L"FALSE")
        );
      break;
    case WINDOW_BUFFER_SIZE_EVENT:
      Records.Format(
            L"WINDOW_BUFFER_SIZE_RECORD: Size = [%d, %d]",
          rec->Event.WindowBufferSizeEvent.dwSize.X,
          rec->Event.WindowBufferSizeEvent.dwSize.Y
        );
      break;
    case MENU_EVENT:
      Records.Format(
            L"MENU_EVENT_RECORD: CommandId = %d (0x%X) ",
          rec->Event.MenuEvent.dwCommandId,
          rec->Event.MenuEvent.dwCommandId
        );
      break;
    case FARMACRO_KEY_EVENT:
    case KEY_EVENT:
    case 0:
    {
      WORD AsciiChar = (WORD)(BYTE)rec->Event.KeyEvent.uChar.AsciiChar;
      Records.Format(
            L"%s: %s, %d, Vk=%s, Scan=0x%04X uChar=[U='%c' (0x%04X): A='%C' (0x%02X)] Ctrl=0x%08X (%c%c%c%c%c - %c%c%c%c)",
          (rec->EventType==KEY_EVENT?L"KEY_EVENT_RECORD":(rec->EventType==FARMACRO_KEY_EVENT?L"FARMACRO_KEY_EVENT":L"(internal, macro)_KEY_EVENT")),
          (rec->Event.KeyEvent.bKeyDown?L"Dn":L"Up"),
          rec->Event.KeyEvent.wRepeatCount,
          _VK_KEY_ToName(rec->Event.KeyEvent.wVirtualKeyCode),
          rec->Event.KeyEvent.wVirtualScanCode,
          (rec->Event.KeyEvent.uChar.UnicodeChar && !(rec->Event.KeyEvent.uChar.UnicodeChar == L'\t' || rec->Event.KeyEvent.uChar.UnicodeChar == L'\r' || rec->Event.KeyEvent.uChar.UnicodeChar == L'\n')?rec->Event.KeyEvent.uChar.UnicodeChar:L' '),
              rec->Event.KeyEvent.uChar.UnicodeChar,
          (AsciiChar && AsciiChar != '\r' && AsciiChar != '\t' && AsciiChar !='\n' ?AsciiChar:' '),
              AsciiChar,
          rec->Event.KeyEvent.dwControlKeyState,
            (rec->Event.KeyEvent.dwControlKeyState&LEFT_CTRL_PRESSED?L'C':L'c'),
            (rec->Event.KeyEvent.dwControlKeyState&LEFT_ALT_PRESSED?L'A':L'a'),
            (rec->Event.KeyEvent.dwControlKeyState&SHIFT_PRESSED?L'S':L's'),
            (rec->Event.KeyEvent.dwControlKeyState&RIGHT_ALT_PRESSED?L'A':L'a'),
            (rec->Event.KeyEvent.dwControlKeyState&RIGHT_CTRL_PRESSED?L'C':L'c'),
            (rec->Event.KeyEvent.dwControlKeyState&ENHANCED_KEY?L'E':L'e'),
            (rec->Event.KeyEvent.dwControlKeyState&CAPSLOCK_ON?L'C':L'c'),
            (rec->Event.KeyEvent.dwControlKeyState&NUMLOCK_ON?L'N':L'n'),
            (rec->Event.KeyEvent.dwControlKeyState&SCROLLLOCK_ON?L'S':L's')
        );
      break;
    }
    case MOUSE_EVENT:
      Records=__MOUSE_EVENT_RECORD_Dump(&rec->Event.MouseEvent);
      break;
    default:
      Records.Format(
            L"??????_EVENT_RECORD: EventType = %d",
          rec->EventType
        );
      break;
  }
  string tmp;
  tmp.Format(L" (%s)",FarAltEnter(FAR_CONSOLE_GET_MODE)==FAR_CONSOLE_WINDOWED?L"Widowed":L"Full Screen");
  Records+=tmp;
  return Records;
#else
  return L"";
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
      wchar_t timebuf[64];
      fwprintf(fp,L"%s %s(Number Of Console Input Events = %d)\n",PrintTime(timebuf),MakeSpace(),ReadCount2);
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

				PeekConsoleInput(hConInp,TmpRec,ReadCount2,&ReadCount3);

        for(DWORD I=0; I < ReadCount2; ++I)
        {
          fwprintf(fp,L"             %s%04d: %s\n",MakeSpace(),I,_INPUT_RECORD_Dump(TmpRec+I));
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

// после вызова этой функции нужно освободить память!!!
string __SysLog_LinearDump(LPBYTE Buf,int SizeBuf)
{
#if defined(SYSLOG)
  string OutBuf, tmp;
  for(int I=0; I < SizeBuf; ++I)
  {
    tmp.Format(L"%02X ",Buf[I]&0xFF);
    OutBuf+=tmp;
  }
  return OutBuf;
#else
  return L"";
#endif
}

void GetVolumeInformation_Dump(const wchar_t *Title,LPCWSTR lpRootPathName,LPCWSTR lpVolumeNameBuffer,DWORD nVolumeNameSize,
                                           DWORD lpVolumeSerialNumber, DWORD lpMaximumComponentLength, DWORD lpFileSystemFlags,
                                           LPCWSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize,FILE *fp)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

  int InternalLog=fp==NULL?TRUE:FALSE;
  const wchar_t *space=MakeSpace();

  if(InternalLog)
  {
    OpenSysLog();
    fp=PrintBaner(fp,L"",Title);
  }

  if (fp)
  {
    fwprintf(fp,L"%*s %s  GetVolumeInformation{\n",12,L"",space);
    fwprintf(fp,L"%*s %s    lpRootPathName            ='%s'\n",12,L"",space,lpRootPathName);
    fwprintf(fp,L"%*s %s    lpVolumeNameBuffer        ='%s'\n",12,L"",space,lpVolumeNameBuffer);
    fwprintf(fp,L"%*s %s    nVolumeNameSize           =%u\n",12,L"",space,nVolumeNameSize);
    fwprintf(fp,L"%*s %s    lpVolumeSerialNumber      =%04X-%04X\n",12,L"",space,lpVolumeSerialNumber>>16,lpVolumeSerialNumber&0xffff);
    fwprintf(fp,L"%*s %s    lpMaximumComponentLength  =%u\n",12,L"",space,lpMaximumComponentLength);

    fwprintf(fp,L"%*s %s    lpFileSystemFlags         =%u\n",12,L"",space,lpFileSystemFlags);
		if(lpFileSystemFlags&FILE_CASE_PRESERVED_NAMES)
			fwprintf(fp,L"%*s %s         FILE_CASE_PRESERVED_NAMES\n",12,L"",space);
		if(lpFileSystemFlags&FILE_CASE_SENSITIVE_SEARCH)
			fwprintf(fp,L"%*s %s         FILE_CASE_SENSITIVE_SEARCH\n",12,L"",space);
		if(lpFileSystemFlags&FILE_FILE_COMPRESSION)
			fwprintf(fp,L"%*s %s         FILE_FILE_COMPRESSION\n",12,L"",space);
		if(lpFileSystemFlags&FILE_NAMED_STREAMS)
			fwprintf(fp,L"%*s %s         FILE_NAMED_STREAMS\n",12,L"",space);
		if(lpFileSystemFlags&FILE_PERSISTENT_ACLS)
			fwprintf(fp,L"%*s %s         FILE_PERSISTENT_ACLS\n",12,L"",space);
		if(lpFileSystemFlags&FILE_READ_ONLY_VOLUME)
			fwprintf(fp,L"%*s %s         FILE_READ_ONLY_VOLUME\n",12,L"",space);
		if(lpFileSystemFlags&FILE_SEQUENTIAL_WRITE_ONCE)
			fwprintf(fp,L"%*s %s         FILE_SEQUENTIAL_WRITE_ONCE\n",12,L"",space);
		if(lpFileSystemFlags&FILE_SUPPORTS_ENCRYPTION)
			fwprintf(fp,L"%*s %s         FILE_SUPPORTS_ENCRYPTION\n",12,L"",space);
		if(lpFileSystemFlags&FILE_SUPPORTS_OBJECT_IDS)
			fwprintf(fp,L"%*s %s         FILE_SUPPORTS_OBJECT_IDS\n",12,L"",space);
		if(lpFileSystemFlags&FILE_SUPPORTS_REPARSE_POINTS)
			fwprintf(fp,L"%*s %s         FILE_SUPPORTS_REPARSE_POINTS\n",12,L"",space);
		if(lpFileSystemFlags&FILE_SUPPORTS_SPARSE_FILES)
			fwprintf(fp,L"%*s %s         FILE_SUPPORTS_SPARSE_FILES\n",12,L"",space);
		if(lpFileSystemFlags&FILE_SUPPORTS_TRANSACTIONS)
			fwprintf(fp,L"%*s %s         FILE_SUPPORTS_TRANSACTIONS\n",12,L"",space);
		if(lpFileSystemFlags&FILE_UNICODE_ON_DISK)
			fwprintf(fp,L"%*s %s         FILE_UNICODE_ON_DISK\n",12,L"",space);
		if(lpFileSystemFlags&FILE_VOLUME_IS_COMPRESSED)
			fwprintf(fp,L"%*s %s         FILE_VOLUME_IS_COMPRESSED\n",12,L"",space);
		if(lpFileSystemFlags&FILE_VOLUME_QUOTAS)
			fwprintf(fp,L"%*s %s         FILE_VOLUME_QUOTAS\n",12,L"",space);

    fwprintf(fp,L"%*s %s    lpFileSystemNameBuffer    ='%s'\n",12,L"",space,lpFileSystemNameBuffer);
    fwprintf(fp,L"%*s %s    nFileSystemNameSize       =%u\n",12,L"",space,nFileSystemNameSize);
    fwprintf(fp,L"%*s %s  }\n",12,L"",space);
    fflush(fp);
  }

  if(InternalLog)
    CloseSysLog();
#endif
}


void WIN32_FIND_DATA_Dump(const wchar_t *Title,const WIN32_FIND_DATA &wfd,FILE *fp)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

  int InternalLog=fp==NULL?TRUE:FALSE;
  const wchar_t *space=MakeSpace();

  if(InternalLog)
  {
    OpenSysLog();
    fp=PrintBaner(fp,L"WIN32_FIND_DATA",Title);
  }

  if (fp)
  {
    fwprintf(fp,L"%*s %s  dwFileAttributes      =0x%08X\n",12,L"",space,wfd.dwFileAttributes);
    if(wfd.dwFileAttributes&FILE_ATTRIBUTE_READONLY)
      fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_READONLY            (0x00000001)\n",12,L"",space);
    if(wfd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)
      fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_HIDDEN              (0x00000002)\n",12,L"",space);
    if(wfd.dwFileAttributes&FILE_ATTRIBUTE_SYSTEM)
      fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_SYSTEM              (0x00000004)\n",12,L"",space);
    if(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
      fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_DIRECTORY           (0x00000010)\n",12,L"",space);
    if(wfd.dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE)
      fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_ARCHIVE             (0x00000020)\n",12,L"",space);
    if(wfd.dwFileAttributes&FILE_ATTRIBUTE_DEVICE)
      fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_DEVICE              (0x00000040)\n",12,L"",space);
    if(wfd.dwFileAttributes&FILE_ATTRIBUTE_NORMAL)
      fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_NORMAL              (0x00000080)\n",12,L"",space);
    if(wfd.dwFileAttributes&FILE_ATTRIBUTE_TEMPORARY)
      fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_TEMPORARY           (0x00000100)\n",12,L"",space);
    if(wfd.dwFileAttributes&FILE_ATTRIBUTE_SPARSE_FILE)
      fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_SPARSE_FILE         (0x00000200)\n",12,L"",space);
    if(wfd.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
      fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_REPARSE_POINT       (0x00000400)\n",12,L"",space);
    if(wfd.dwFileAttributes&FILE_ATTRIBUTE_COMPRESSED)
      fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_COMPRESSED          (0x00000800)\n",12,L"",space);
    if(wfd.dwFileAttributes&FILE_ATTRIBUTE_OFFLINE)
      fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_OFFLINE             (0x00001000)\n",12,L"",space);
    if(wfd.dwFileAttributes&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)
      fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_NOT_CONTENT_INDEXED (0x00002000)\n",12,L"",space);
    if(wfd.dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED)
      fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_ENCRYPTED           (0x00004000)\n",12,L"",space);
    if(wfd.dwFileAttributes&FILE_ATTRIBUTE_VIRTUAL)
      fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_VIRTUAL             (0x00010000)\n",12,L"",space);

    string D, T;
    ConvertDate(wfd.ftCreationTime,D,T,8,FALSE,FALSE,TRUE);
    fwprintf(fp,L"%*s %s  ftCreationTime        =0x%08X 0x%08X\n",12,L"",space,wfd.ftCreationTime.dwHighDateTime,wfd.ftCreationTime.dwLowDateTime);
    ConvertDate(wfd.ftLastAccessTime,D,T,8,FALSE,FALSE,TRUE);
    fwprintf(fp,L"%*s %s  ftLastAccessTime      =0x%08X 0x%08X\n",12,L"",space,wfd.ftLastAccessTime.dwHighDateTime,wfd.ftLastAccessTime.dwLowDateTime);
    ConvertDate(wfd.ftLastWriteTime,D,T,8,FALSE,FALSE,TRUE);
    fwprintf(fp,L"%*s %s  ftLastWriteTime       =0x%08X 0x%08X\n",12,L"",space,wfd.ftLastWriteTime.dwHighDateTime,wfd.ftLastWriteTime.dwLowDateTime);

    LARGE_INTEGER Number;
    Number.u.HighPart=wfd.nFileSizeHigh;
    Number.u.LowPart=wfd.nFileSizeLow;

    fwprintf(fp,L"%*s %s  nFileSize             =0x%08X, 0x%08X (%I64u)\n",12,L"",space,wfd.nFileSizeHigh,wfd.nFileSizeLow,Number.QuadPart);
    fwprintf(fp,L"%*s %s  dwReserved0           =0x%08X (%d)\n",12,L"",space,wfd.dwReserved0,wfd.dwReserved0);
    if(wfd.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
    {
      if(wfd.dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT)
        fwprintf(fp,L"%*s %s     IO_REPARSE_TAG_MOUNT_POINT (0xA0000003L)\n",12,L"",space);
      if(wfd.dwReserved0 == IO_REPARSE_TAG_HSM)
        fwprintf(fp,L"%*s %s     IO_REPARSE_TAG_HSM         (0xC0000004L)\n",12,L"",space);
      if(wfd.dwReserved0 == IO_REPARSE_TAG_SIS)
        fwprintf(fp,L"%*s %s     IO_REPARSE_TAG_SIS         (0x80000007L)\n",12,L"",space);
      if(wfd.dwReserved0 == IO_REPARSE_TAG_DFS)
        fwprintf(fp,L"%*s %s     IO_REPARSE_TAG_DFS         (0x8000000AL)\n",12,L"",space);
      if(wfd.dwReserved0 == IO_REPARSE_TAG_SYMLINK)
        fwprintf(fp,L"%*s %s     IO_REPARSE_TAG_SYMLINK     (0xA000000CL)\n",12,L"",space);
      if(wfd.dwReserved0 == IO_REPARSE_TAG_DFSR)
        fwprintf(fp,L"%*s %s     IO_REPARSE_TAG_DFSR        (0x80000012L)\n",12,L"",space);
    }
    fwprintf(fp,L"%*s %s  dwReserved1           =0x%08X (%d)\n",12,L"",space,wfd.dwReserved1,wfd.dwReserved1);
    fwprintf(fp,L"%*s %s  cFileName             =\"%s\"\n",12,L"",space,wfd.cFileName);
    fwprintf(fp,L"%*s %s  cAlternateFileName    =\"%s\"\n",12,L"",space,wfd.cAlternateFileName);

    fwprintf(fp,L"%*s %s  }\n",12,L"",space);
    fflush(fp);
  }

  if(InternalLog)
    CloseSysLog();
#endif
}

void PanelViewSettings_Dump(const wchar_t *Title,const PanelViewSettings &ViewSettings,FILE *fp)
{
#if defined(SYSLOG)
  if(!IsLogON())
    return;

  int InternalLog=fp==NULL?TRUE:FALSE;
  const wchar_t *space=MakeSpace();

  if(InternalLog)
  {
    OpenSysLog();
    fp=PrintBaner(fp,L"PanelViewSettings",Title);
  }

  if (fp)
  {
    int I;
    fwprintf(fp,L"%*s %s  PanelViewSettings{\n",12,L"",space);
    fwprintf(fp,L"%*s %s  ColumnType           = [",12,L"",space);
		for(I=0; I < countof(ViewSettings.ColumnType)-1;++I)
      fwprintf(fp,L"%d, ",ViewSettings.ColumnType[I]);
    fwprintf(fp,L"%d]\n",ViewSettings.ColumnType[I]);
    fwprintf(fp,L"%*s %s  ColumnWidth          = [",12,L"",space);
		for(I=0; I < countof(ViewSettings.ColumnWidth)-1;++I)
      fwprintf(fp,L"%d, ",ViewSettings.ColumnWidth[I]);
    fwprintf(fp,L"%d]\n",ViewSettings.ColumnWidth[I]);
    fwprintf(fp,L"%*s %s  ColumnCount          = %d\n",12,L"",space,ViewSettings.ColumnCount);
    fwprintf(fp,L"%*s %s  StatusColumnType     = [",12,L"",space);
		for(I=0; I < countof(ViewSettings.StatusColumnType)-1;++I)
      fwprintf(fp,L"%08X, ",ViewSettings.StatusColumnType[I]);
    fwprintf(fp,L"%08X]\n",ViewSettings.StatusColumnType[I]);
    fwprintf(fp,L"%*s %s  StatusColumnWidth    = [",12,L"",space);
		for(I=0; I < countof(ViewSettings.StatusColumnWidth)-1;++I)
      fwprintf(fp,L"%d, ",ViewSettings.StatusColumnWidth[I]);
    fwprintf(fp,L"%d]\n",ViewSettings.StatusColumnWidth[I]);
    fwprintf(fp,L"%*s %s  StatusColumnCount    = %d\n",12,L"",space,ViewSettings.StatusColumnCount);
    fwprintf(fp,L"%*s %s  FullScreen           = %d\n",12,L"",space,ViewSettings.FullScreen);
    fwprintf(fp,L"%*s %s  AlignExtensions      = %d\n",12,L"",space,ViewSettings.AlignExtensions);
    fwprintf(fp,L"%*s %s  FolderAlignExtensions= %d\n",12,L"",space,ViewSettings.FolderAlignExtensions);
    fwprintf(fp,L"%*s %s  FolderUpperCase      = %d\n",12,L"",space,ViewSettings.FolderUpperCase);
    fwprintf(fp,L"%*s %s  FileLowerCase        = %d\n",12,L"",space,ViewSettings.FileLowerCase);
    fwprintf(fp,L"%*s %s  FileUpperToLowerCase = %d\n",12,L"",space,ViewSettings.FileUpperToLowerCase);
    fwprintf(fp,L"%*s %s  CaseSensitiveSort    = %d\n",12,L"",space,ViewSettings.CaseSensitiveSort);

    fwprintf(fp,L"%*s %s  }\n",12,L"",space);
    fflush(fp);
  }

  if(InternalLog)
    CloseSysLog();
#endif
}
