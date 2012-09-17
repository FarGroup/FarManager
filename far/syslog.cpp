/*
syslog.cpp

Системный отладочный лог :-)
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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
#include "datetime.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "interf.hpp"
#include "console.hpp"
#include "farversion.hpp"

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

#define MAX_LOG_LINE 10240

static FILE *LogStream=0;
static int   Indent=0;
static wchar_t *PrintTime(wchar_t *timebuf,size_t size);


static BOOL IsLogON()
{
	return GetKeyState(VK_SCROLL)?TRUE:FALSE;
}

static const wchar_t *MakeSpace()
{
	static wchar_t Buf[60]=L" ";
	Buf[0]=L' ';

	for (int I=1; I <= Indent; ++I)
		Buf[I]=L'|';

	Buf[1+Indent]=0;
	return Buf;
}

static wchar_t *PrintTime(wchar_t *timebuf,size_t size)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
//  sprintf(timebuf,"%02d.%02d.%04d %2d:%02d:%02d.%03d",
//      st.wDay,st.wMonth,st.wYear,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
	_snwprintf(timebuf,size,L"%02d:%02d:%02d.%03d",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
	return timebuf;
}

static FILE* PrintBaner(FILE *fp,const wchar_t *Category,const wchar_t *Title)
{
	fp=LogStream;

	if (fp)
	{
		static wchar_t timebuf[64];
		fwprintf(fp,L"%s %s(%s) %s\n",PrintTime(timebuf,ARRAYSIZE(timebuf)),MakeSpace(),NullToEmpty(Title),NullToEmpty(Category));
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
	strRealLogName.Format(L"%s\\Far.%04d%02d%02d.%05d.log",file,st.wYear,st.wMonth,st.wDay,FAR_VERSION.Build);
	return _wfsopen(strRealLogName,L"a+t",SH_DENYWR);
#else
	return nullptr;
#endif
}

void OpenSysLog()
{
#if defined(SYSLOG)

	if (LogStream)
		fclose(LogStream);

	string strLogFileName=g_strFarPath+L"$Log";
	DWORD Attr=apiGetFileAttributes(strLogFileName);

	if (Attr == INVALID_FILE_ATTRIBUTES)
	{
		if (!apiCreateDirectory(strLogFileName,nullptr))
			strLogFileName.SetLength(g_strFarPath.GetLength());
	}
	else if (!(Attr&FILE_ATTRIBUTE_DIRECTORY))
		strLogFileName.SetLength(g_strFarPath.GetLength());

	LogStream=OpenLogStream(strLogFileName);
	//if ( !LogStream )
	//{
	//    fprintf(stderr,"Can't open log file '%s'\n",LogFileName);
	//}
#endif
}

void CloseSysLog()
{
#if defined(SYSLOG)
	fclose(LogStream);
	LogStream=0;
#endif
}

void ShowHeap()
{
#if defined(SYSLOG) && defined(HEAPLOG)

	if (!IsLogON())
		return;

	OpenSysLog();

	if (LogStream)
	{
		wchar_t timebuf[64];
		fwprintf(LogStream,L"%s %s%s\n",PrintTime(timebuf,ARRAYSIZE(timebuf)),MakeSpace(),L"Heap Status");
		fwprintf(LogStream,L"   Size   Status\n");
		fwprintf(LogStream,L"   ----   ------\n");
		DWORD Sz=0;
		_HEAPINFO hi;
		hi._pentry=nullptr;

		//    int     *__pentry;
		while (_rtl_heapwalk(&hi) == _HEAPOK)
		{
			fwprintf(LogStream,L"%7u    %s  (%p)\n", hi._size, (hi._useflag ? L"used" : L"free"),hi.__pentry);
			Sz+=hi._useflag?hi._size:0;
		}

		fwprintf(LogStream,L"   ----   ------\n");
		fwprintf(LogStream,L"%7u      \n", Sz);
		fflush(LogStream);
	}

	CloseSysLog();
#endif
}


void CheckHeap(int NumLine)
{
#if defined(SYSLOG) && defined(HEAPLOG)

	if (!IsLogON())
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

	if (Indent<0)
		Indent=0;

#endif
}


void SysLog(const wchar_t *fmt,...)
{
#if defined(SYSLOG)

	if (!IsLogON())
		return;

	LPWSTR msg=new WCHAR[MAX_LOG_LINE];
	if(msg)
	{
		va_list argptr;
		va_start(argptr, fmt);
		_vsnwprintf(msg, MAX_LOG_LINE, fmt, argptr);
		va_end(argptr);
		OpenSysLog();

		if (LogStream)
		{
			wchar_t timebuf[64];
			fwprintf(LogStream,L"%s %s%s\n",PrintTime(timebuf,ARRAYSIZE(timebuf)),MakeSpace(),msg);
			fflush(LogStream);
		}

		CloseSysLog();

		if (IsDebuggerPresent())
		{
			OutputDebugString(msg);
#ifdef _MSC_VER
			OutputDebugString(L"\n");
#endif _MSC_VER
		}
		delete[] msg;
	}
#endif
}

void SysLogLastError()
{
#if defined(SYSLOG)

	if (!IsLogON())
		return;

	wchar_t *lpMsgBuf = nullptr;
	DWORD LastErr=GetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
	              nullptr,LastErr,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
	              (LPWSTR) &lpMsgBuf,0,nullptr);
	OpenSysLog();

	if (LogStream)
	{
		wchar_t timebuf[64];
		// RemoveUnprintableCharacters(lpMsgBuf);
		fwprintf(LogStream,L"%s %sGetLastError()=[%u/0x%X] \"%s\"\n",PrintTime(timebuf,ARRAYSIZE(timebuf)),MakeSpace(),LastErr,LastErr,lpMsgBuf);
		fflush(LogStream);
	}

	CloseSysLog();

	if (IsDebuggerPresent())
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

	if (!IsLogON())
		return;

	LPWSTR msg=new WCHAR[MAX_LOG_LINE];
	if(msg)
	{
		va_list argptr;
		va_start(argptr, fmt);
		_vsnwprintf(msg, MAX_LOG_LINE, fmt, argptr);
		va_end(argptr);
		OpenSysLog();

		if (LogStream)
		{
			if (l < 0) SysLog(l);

			wchar_t timebuf[64];
			fwprintf(LogStream,L"%s %s%s\n",PrintTime(timebuf,ARRAYSIZE(timebuf)),MakeSpace(),msg);
			fflush(LogStream);

			if (l > 0) SysLog(l);
		}

		CloseSysLog();

		if (IsDebuggerPresent())
		{
			OutputDebugString(msg);
#ifdef _MSC_VER
			OutputDebugString(L"\n");
#endif _MSC_VER
		}
		delete[] msg;
	}

#endif
}


void SysLogDump(const wchar_t *Title,DWORD StartAddress,LPBYTE Buf,unsigned SizeBuf,FILE *fp)
{
#if defined(SYSLOG)

	if (!IsLogON())
		return;

	int CY=(SizeBuf+15)/16;
	int InternalLog=fp?FALSE:TRUE;
	static wchar_t timebuf[64];
//  char msg[MAX_LOG_LINE];

	if (InternalLog)
	{
		OpenSysLog();
		fp=LogStream;
		fwprintf(fp,L"%s %s<%s> [%u bytes]{\n",PrintTime(timebuf,ARRAYSIZE(timebuf)),MakeSpace(),NullToEmpty(Title),SizeBuf);
	}

	if (fp)
	{
		if (!InternalLog && Title && *Title)
			fwprintf(fp,L"%s\n",Title);

		wchar_t TmpBuf[17]={};

		for (int Y=0; Y < CY; ++Y)
		{
			//memset(TmpBuf,' ',16);
			fwprintf(fp,L"%s %s ",PrintTime(timebuf,ARRAYSIZE(timebuf)),MakeSpace());
			fwprintf(fp, L" %08X: ",StartAddress+Y*16);

			for (size_t X=0; X < 16; ++X)
			{
				size_t I=Y*16+X;
				if ((I < SizeBuf) )
					fwprintf(fp,L"%02X ",Buf[Y*16+X]&0xFF);
				else
					fwprintf(fp,L"   ");

				TmpBuf[X]=I?(Buf[Y*16+X] < 32?L'.':Buf[Y*16+X]):L' ';

				if (X == 7)
					fwprintf(fp,L" ");
			}

			//for(X < 16; ++X)
			fwprintf(fp,L"| %s\n",TmpBuf);
		}

		fwprintf(fp,L"%s %s}</%s>\n",PrintTime(timebuf,ARRAYSIZE(timebuf)),MakeSpace(),NullToEmpty(Title));
		fflush(fp);
	}

	if (InternalLog)
		CloseSysLog();

#endif
}


void SaveScreenDumpBuffer(const wchar_t *Title,const FAR_CHAR_INFO *Buffer,int X1,int Y1,int X2,int Y2,FILE *fp)
{
#if defined(SYSLOG)

	if (!IsLogON())
		return;

	int InternalLog=fp?FALSE:TRUE;

	if (InternalLog)
	{
		OpenSysLog();
		fp=LogStream;

		if (fp)
		{
			wchar_t timebuf[64];
			fwprintf(fp,L"%s %s(FAR_CHAR_INFO DumpBuffer: '%s')\n",PrintTime(timebuf,ARRAYSIZE(timebuf)),MakeSpace(),NullToEmpty(Title));
		}
	}

	wchar_t *line=new wchar_t[X2-X1+4];

	if (fp && line)
	{
		int x,y,i;

		if (!InternalLog && Title && *Title)
			fwprintf(fp,L"FAR_CHAR_INFO DumpBuffer: %s\n",Title);

		fwprintf(fp,L"XY={%i,%i - %i,%i}\n",X1,Y1,X2,Y2);

		for (y=Y1; y <= Y2; y++)
		{
			fwprintf(fp,L"%04d: ",y);

			for (i=0, x=X1; x <= X2; x++, ++i)
			{
				line[i]=Buffer->Char?Buffer->Char:L' ';
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

	if (InternalLog)
		CloseSysLog();

#endif
}



void PluginsStackItem_Dump(const wchar_t *Title,const PluginsListItem *ListItems,int ItemNumber,FILE *fp)
{
#if defined(SYSLOG)

	if (!IsLogON())
		return;

	int InternalLog=fp?FALSE:TRUE;

	if (InternalLog)
	{
		OpenSysLog();
		fp=PrintBaner(fp,L"",Title);
	}

	if (fp)
	{
#define DEF_SORTMODE_(m) { m , L#m }
		static struct SORTMode
		{
			int Mode;
			const wchar_t *Name;
		} __SORT[]=
		{
			DEF_SORTMODE_(UNSORTED),  DEF_SORTMODE_(BY_NAME),  DEF_SORTMODE_(BY_EXT),
			DEF_SORTMODE_(BY_MTIME),  DEF_SORTMODE_(BY_CTIME), DEF_SORTMODE_(BY_ATIME), DEF_SORTMODE_(BY_CHTIME),
			DEF_SORTMODE_(BY_SIZE),   DEF_SORTMODE_(BY_DIZ),   DEF_SORTMODE_(BY_OWNER),
			DEF_SORTMODE_(BY_COMPRESSEDSIZE),DEF_SORTMODE_(BY_NUMLINKS),
			DEF_SORTMODE_(BY_NUMSTREAMS),DEF_SORTMODE_(BY_STREAMSSIZE),
			DEF_SORTMODE_(BY_FULLNAME),DEF_SORTMODE_(BY_CUSTOMDATA)
		};

		if (!ListItems || !ItemNumber)
			fwprintf(fp,L"\tPluginsStackItem <EMPTY>");
		else
		{
			for (int I=ItemNumber-1; I >= 0; --I)
				fwprintf(fp,L"\t[%d]: "
				         L"hPlugin=%p "
				         L"Modified=%s "
				         L"PrevViewMode=VIEW_%02d "
				         L"PrevSortMode=%d/%-17s "
				         L"PrevSortOrder=%02d "
				         L"PrevNumericSort=%02d "
						 L"PrevCaseSensitiveSort=%02d "
				         L"PrevDirectoriesFirst=%02d "
				         L"HostFile=%s\n",
				         I,
				         ListItems[I].hPlugin,
				         (ListItems[I].Modified?L"True ":L"False"),
				         ListItems[I].PrevViewMode,
				         ListItems[I].PrevSortMode,
				         (ListItems[I].PrevSortMode<BY_CUSTOMDATA?__SORT[ListItems[I].PrevSortMode].Name:L"<Unknown>"),
				         ListItems[I].PrevSortOrder,
				         ListItems[I].PrevNumericSort,
						 ListItems[I].PrevCaseSensitiveSort,
				         ListItems[I].PrevDirectoriesFirst,
				         ListItems[I].strHostFile.CPtr());
		}

		fwprintf(fp,L"\n");
		fflush(fp);
	}

	if (InternalLog)
		CloseSysLog();

#endif
}

void GetOpenPanelInfo_Dump(const wchar_t *Title,const OpenPanelInfo *Info,FILE *fp)
{
#if defined(SYSLOG)

	if (!IsLogON())
		return;

	int InternalLog=fp?FALSE:TRUE;

	if (InternalLog)
	{
		OpenSysLog();
		fp=PrintBaner(fp,L"OpenPanelInfo",Title);
	}

	if (fp)
	{
		fwprintf(fp,L"\tStructSize      =%u\n",static_cast<unsigned>(Info->StructSize));
		fwprintf(fp,L"\tFlags           =0x%08I64X\n",Info->Flags);
		fwprintf(fp,L"\tHostFile        ='%s'\n",NullToEmpty(Info->HostFile));
		fwprintf(fp,L"\tCurDir          ='%s'\n",NullToEmpty(Info->CurDir));
		fwprintf(fp,L"\tFormat          ='%s'\n",NullToEmpty(Info->Format));
		fwprintf(fp,L"\tPanelTitle      ='%s'\n",NullToEmpty(Info->PanelTitle));
		fwprintf(fp,L"\tInfoLines       =%p\n",Info->InfoLines);
		fwprintf(fp,L"\tInfoLinesNumber =%u\n",static_cast<unsigned>(Info->InfoLinesNumber));

		if (Info->InfoLines)
		{
			for (size_t I=0; I<Info->InfoLinesNumber; ++I)
			{
				fwprintf(fp,L"\t\t%u) Text=[%s], Data=[%s], Separator=[%d]\n",static_cast<unsigned>(I),
				         NullToEmpty(Info->InfoLines[I].Text),NullToEmpty(Info->InfoLines[I].Data),Info->InfoLines[I].Separator);
			}
		}

		fwprintf(fp,L"\tDescrFiles      =%p\n",Info->DescrFiles);
		fwprintf(fp,L"\tDescrFilesNumber=%u\n",static_cast<unsigned>(Info->DescrFilesNumber));
		fwprintf(fp,L"\tPanelModesArray =%p\n",Info->PanelModesArray);
		fwprintf(fp,L"\tPanelModesNumber=%u\n",static_cast<unsigned>(Info->PanelModesNumber));

		if (Info->PanelModesArray)
		{
			for (size_t I=0; I<Info->PanelModesNumber; ++I)
			{
				fwprintf(fp,L"\t%u) ------------------\n",static_cast<unsigned>(I));
				fwprintf(fp,L"\t\tColumnTypes       ='%s'\n",NullToEmpty(Info->PanelModesArray[I].ColumnTypes));
				fwprintf(fp,L"\t\tColumnWidths      ='%s'\n",NullToEmpty(Info->PanelModesArray[I].ColumnWidths));
				fwprintf(fp,L"\t\tColumnTitles      =%p\n",Info->PanelModesArray[I].ColumnTitles);
				fwprintf(fp,L"\t\tFullScreen        =%d\n",(Info->PanelModesArray[I].Flags&PMFLAGS_FULLSCREEN)!=0);
				fwprintf(fp,L"\t\tDetailedStatus    =%d\n",(Info->PanelModesArray[I].Flags&PMFLAGS_DETAILEDSTATUS)!=0);
				fwprintf(fp,L"\t\tAlignExtensions   =%d\n",(Info->PanelModesArray[I].Flags&PMFLAGS_ALIGNEXTENSIONS)!=0);
				fwprintf(fp,L"\t\tCaseConversion    =%d\n",(Info->PanelModesArray[I].Flags&PMFLAGS_CASECONVERSION)!=0);
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

	if (InternalLog)
		CloseSysLog();

#endif
}


void ManagerClass_Dump(const wchar_t *Title,const Manager *m,FILE *fp)
{
#if defined(SYSLOG)

	if (!IsLogON())
		return;

	int InternalLog=fp?FALSE:TRUE;

	if (InternalLog)
	{
		OpenSysLog();
		fp=PrintBaner(fp,L"",Title);
	}

	if (fp)
	{
		const Manager *Man=m?m:FrameManager;
//StartSysLog
		string Type,Name;
		fwprintf(fp,L"**** Queue modal frames ***\nFrameListSize=%d, FramePos=%d, FrameCount=%d\n",Man->FrameListSize,Man->FramePos,Man->FrameCount);

		if (Man->FrameList)
		{
			for (int i=0; i < Man->FrameCount; i++)
			{
				if (Man->FrameList[i])
				{
					Man->FrameList[i]->GetTypeAndName(Type,Name);
					fwprintf(fp,L"\tFrameList[%d] %p  Type='%s' Name='%s'\n",i,Man->FrameList[i],Type.CPtr(),Name.CPtr());
				}
				else
					fwprintf(fp,L"\tFrameList[%d] nullptr\n",i);
			}
		}
		else
			fwprintf(fp,L"\tFrameList = nullptr\n");

		fwprintf(fp,L"**** Stack modal frames ***\nModalStackSize=%d\n",Man->ModalStackSize);

		if (Man->ModalStack)
		{
			for (int i=0; i < Man->ModalStackCount; i++)
			{
				if (Man->ModalStack[i])
				{
					Man->ModalStack[i]->GetTypeAndName(Type,Name);
					fwprintf(fp,L"\tModalStack[%d] %p  Type='%s' Name='%s'\n",
					         i,Man->ModalStack[i],Type.CPtr(),Name.CPtr());
				}
				else
					fwprintf(fp,L"\tModalStack[%d] nullptr\n",i);
			}
		}
		else
			fwprintf(fp,L"\tModalStack = nullptr\n");

		fwprintf(fp,L"**** Detail... ***\n");

		if (!Man->InsertedFrame)
			Type.Clear(), Name.Clear();
		else
			Man->InsertedFrame->GetTypeAndName(Type,Name);

		fwprintf(fp,L"\tInsertedFrame=%p (Type='%s' Name='%s')\n", //  - Фрейм, который будет добавлен в конец немодальной очереди
		         Man->InsertedFrame,Type.CPtr(),Name.CPtr());

		if (!Man->DeletedFrame)
			Type.Clear(), Name.Clear();
		else
			Man->DeletedFrame->GetTypeAndName(Type,Name);

		fwprintf(fp,L"\tDeletedFrame=%p (Type='%s' Name='%s')\n", //  - Фрейм, предназначен для удаления из модальной очереди, из модального стека, либо одиночный (которого нет ни там, ни там)
		         Man->DeletedFrame,Type.CPtr(),Name.CPtr());

		if (!Man->ActivatedFrame)
			Type.Clear(), Name.Clear();
		else
			Man->ActivatedFrame->GetTypeAndName(Type,Name);

		fwprintf(fp,L"\tActivatedFrame=%p (Type='%s' Name='%s')\n", //  - Фрейм, который необходимо активировать после каких ни будь изменений
		         Man->ActivatedFrame,Type.CPtr(),Name.CPtr());

		if (!Man->RefreshedFrame)
			Type.Clear(), Name.Clear();
		else
			Man->RefreshedFrame->GetTypeAndName(Type,Name);

		fwprintf(fp,L"\tRefreshedFrame=%p (Type='%s' Name='%s')\n", //  - Фрейм, который нужно просто освежить, т.е. перерисовать
		         Man->RefreshedFrame,Type.CPtr(),Name.CPtr());

		if (!Man->ModalizedFrame)
			Type.Clear(), Name.Clear();
		else
			Man->ModalizedFrame->GetTypeAndName(Type,Name);

		fwprintf(fp,L"\tModalizedFrame=%p (Type='%s' Name='%s')\n", //  - Фрейм, который становится в 'очередь' к текущему немодальному фрейму
		         Man->ModalizedFrame,Type.CPtr(),Name.CPtr());

		if (!Man->UnmodalizedFrame)
			Type.Clear(), Name.Clear();
		else
			Man->UnmodalizedFrame->GetTypeAndName(Type,Name);

		fwprintf(fp,L"\tUnmodalizedFrame=%p (Type='%s' Name='%s')\n", //  - Фрейм, убирающийся из 'очереди' немодального фрейма
		         Man->UnmodalizedFrame,Type.CPtr(),Name.CPtr());

		if (!Man->DeactivatedFrame)
			Type.Clear(), Name.Clear();
		else
			Man->DeactivatedFrame->GetTypeAndName(Type,Name);

		fwprintf(fp,L"\tDeactivatedFrame=%p (Type='%s' Name='%s')\n", //  - Фрейм, который указывает на предыдущий активный фрейм
		         Man->DeactivatedFrame,Type.CPtr(),Name.CPtr());

		if (!Man->ExecutedFrame)
			Type.Clear(), Name.Clear();
		else
			Man->ExecutedFrame->GetTypeAndName(Type,Name);

		fwprintf(fp,L"\tExecutedFrame=%p (Type='%s' Name='%s')\n", //  - Фрейм, которого вскорости нужно будет поставить на вершину модального стека
		         Man->ExecutedFrame,Type.CPtr(),Name.CPtr());

		if (!Man->CurrentFrame)
			Type.Clear(), Name.Clear();
		else
			Man->CurrentFrame->GetTypeAndName(Type,Name);

		fwprintf(fp,L"\tCurrentFrame=%p (Type='%s' Name='%s')\n", //  - текущий фрейм. Он может находиться как в немодальной очереди, так и в модальном стеке
		         Man->CurrentFrame,Type.CPtr(),Name.CPtr());
		fwprintf(fp,L"\n");
		fflush(fp);
	}

	if (InternalLog)
		CloseSysLog();

#endif
}


#if defined(SYSLOG_FARSYSLOG)
void WINAPIV FarSysLog(const wchar_t *ModuleName,int l,const wchar_t *fmt,...)
{
	if (!IsLogON())
		return;

	LPWSTR msg=new WCHAR[MAX_LOG_LINE];
	if(msg)
	{
		va_list argptr;
		va_start(argptr, fmt);
		_vsnwprintf(msg, MAX_LOG_LINE, fmt, argptr);
		va_end(argptr);
		SysLog(l);
		OpenSysLog();

		if (LogStream)
		{
			wchar_t timebuf[64];
			fwprintf(LogStream,L"%s %s%s:: %s\n",PrintTime(timebuf,ARRAYSIZE(timebuf)),MakeSpace(),PointToName(ModuleName),msg);
			fflush(LogStream);
		}

		CloseSysLog();

		if (IsDebuggerPresent())
		{
			OutputDebugString(msg);
#ifdef _MSC_VER
			OutputDebugString(L"\n");
#endif _MSC_VER
		}
		delete[] msg;
	}
}

void WINAPI FarSysLogDump(const wchar_t *ModuleName,DWORD StartAddress,LPBYTE Buf,int SizeBuf)
{
	if (!IsLogON())
		return;

	SysLogDump(ModuleName,StartAddress,Buf,SizeBuf,nullptr);
}

void WINAPI FarSysLog_INPUT_RECORD_Dump(const wchar_t *ModuleName,INPUT_RECORD *rec)
{
	if (!IsLogON())
		return;

	SysLog(L"%s {%s}",ModuleName,_INPUT_RECORD_Dump(rec));
}
#endif

// "Умный класс для SysLog
CleverSysLog::CleverSysLog(const wchar_t *Title)
{
#if defined(SYSLOG)

	if (!IsLogON())
		return;

	SysLog(1,L"%s{",Title?Title:L"");
#endif
}

CleverSysLog::CleverSysLog(int Line,const wchar_t *Title)
{
#if defined(SYSLOG)

	if (!IsLogON())
		return;

	SysLog(1,L"[%d] %s{",Line,Title?Title:L"");
#endif
}

CleverSysLog::~CleverSysLog()
{
#if defined(SYSLOG)

	if (!IsLogON())
		return;

	SysLog(-1,L"}");
#endif
}

#if defined(SYSLOG)
struct __XXX_Name
{
	DWORD Val;
	const wchar_t *Name;
};

static string _XXX_ToName(DWORD Val,const wchar_t *Pref,__XXX_Name *arrDef,size_t cntArr)
{
	string Name;

	for (size_t i=0; i<cntArr; i++)
	{
		if (arrDef[i].Val == Val)
		{
			Name.Format(L"\"%s_%s\" [%d/0x%04X]",Pref,arrDef[i].Name,Val,Val);
			return Name;
		}
	}

	Name.Format(L"\"%s_????\" [%d/0x%04X]",Pref,Val,Val);
	return Name;
}
#endif


string __ECTL_ToName(int Command)
{
#if defined(SYSLOG_KEYMACRO) || defined(SYSLOG_ECTL)
#define DEF_ECTL_(m) { ECTL_##m , L#m }
	__XXX_Name ECTL[]=
	{
		DEF_ECTL_(GETSTRING),
		DEF_ECTL_(SETSTRING),
		DEF_ECTL_(INSERTSTRING),
		DEF_ECTL_(DELETESTRING),
		DEF_ECTL_(DELETECHAR),
		DEF_ECTL_(INSERTTEXT),
		DEF_ECTL_(GETINFO),
		DEF_ECTL_(SETPOSITION),
		DEF_ECTL_(SELECT),
		DEF_ECTL_(REDRAW),
		DEF_ECTL_(TABTOREAL),
		DEF_ECTL_(REALTOTAB),
		DEF_ECTL_(EXPANDTABS),
		DEF_ECTL_(SETTITLE),
		DEF_ECTL_(READINPUT),
		DEF_ECTL_(PROCESSINPUT),
		DEF_ECTL_(ADDCOLOR),
		DEF_ECTL_(GETCOLOR),
		DEF_ECTL_(SAVEFILE),
		DEF_ECTL_(QUIT),
		DEF_ECTL_(SETKEYBAR),
		DEF_ECTL_(SETPARAM),
		DEF_ECTL_(GETBOOKMARKS),
		DEF_ECTL_(DELETEBLOCK),
		DEF_ECTL_(ADDSESSIONBOOKMARK),
		DEF_ECTL_(PREVSESSIONBOOKMARK),
		DEF_ECTL_(NEXTSESSIONBOOKMARK),
		DEF_ECTL_(CLEARSESSIONBOOKMARKS),
		DEF_ECTL_(DELETESESSIONBOOKMARK),
		DEF_ECTL_(GETSESSIONBOOKMARKS),
		DEF_ECTL_(UNDOREDO),
		DEF_ECTL_(GETFILENAME),
		DEF_ECTL_(DELCOLOR),
		DEF_ECTL_(SERVICEREGION),
	};
	return _XXX_ToName(Command,L"ECTL",ECTL,ARRAYSIZE(ECTL));
#else
	return L"";
#endif
}

string __EE_ToName(int Command)
{
#if defined(SYSLOG)
#define DEF_EE_(m) { EE_##m , L#m }
	__XXX_Name EE[]=
	{
		DEF_EE_(READ),
		DEF_EE_(SAVE),
		DEF_EE_(REDRAW),
		DEF_EE_(CLOSE),
		DEF_EE_(GOTFOCUS),
		DEF_EE_(KILLFOCUS),
	};
	return _XXX_ToName(Command,L"EE",EE,ARRAYSIZE(EE));
#else
	return L"";
#endif
}

string __EEREDRAW_ToName(int Command)
{
#if defined(SYSLOG)
#define DEF_EEREDRAW_(m) { (int)(intptr_t)EEREDRAW_##m , L#m }
	__XXX_Name EEREDRAW[]=
	{
		DEF_EEREDRAW_(ALL),
	};
	return _XXX_ToName(Command,L"EEREDRAW",EEREDRAW,ARRAYSIZE(EEREDRAW));
#else
	return L"";
#endif
}

string __ESPT_ToName(int Command)
{
#if defined(SYSLOG_KEYMACRO) || defined(SYSLOG_ECTL)
#define DEF_ESPT_(m) { ESPT_##m , L#m }
	__XXX_Name ESPT[]=
	{
		DEF_ESPT_(TABSIZE),
		DEF_ESPT_(EXPANDTABS),
		DEF_ESPT_(AUTOINDENT),
		DEF_ESPT_(CURSORBEYONDEOL),
		DEF_ESPT_(CHARCODEBASE),
		DEF_ESPT_(CODEPAGE),
		DEF_ESPT_(SAVEFILEPOSITION),
		DEF_ESPT_(LOCKMODE),
		DEF_ESPT_(SETWORDDIV),
		DEF_ESPT_(GETWORDDIV),
		DEF_ESPT_(SHOWWHITESPACE),
		DEF_ESPT_(SETBOM),
	};
	return _XXX_ToName(Command,L"ESPT",ESPT,ARRAYSIZE(ESPT));
#else
	return L"";
#endif
}

string __MCTL_ToName(int Command)
{
#if defined(SYSLOG_KEYMACRO) || defined(SYSLOG_MCTL)
#define DEF_MCTL_(m) { MCTL_##m , L#m }
	__XXX_Name MCTL[]=
	{
		DEF_MCTL_(LOADALL),
		DEF_MCTL_(SAVEALL),
		DEF_MCTL_(SENDSTRING),
		DEF_MCTL_(GETSTATE),
		DEF_MCTL_(GETAREA),
		DEF_MCTL_(ADDMACRO),
		DEF_MCTL_(DELMACRO),
		DEF_MCTL_(GETLASTERROR),
	};
	return _XXX_ToName(Command,L"MCTL",MCTL,ARRAYSIZE(MCTL));
#else
	return L"";
#endif
}

string __VE_ToName(int Command)
{
#if defined(SYSLOG)
#define DEF_VE_(m) { VE_##m , L#m }
	__XXX_Name VE[]=
	{
		DEF_VE_(READ),
		DEF_VE_(CLOSE),
		DEF_VE_(GOTFOCUS),
		DEF_VE_(KILLFOCUS),
	};
	return _XXX_ToName(Command,L"VE",VE,ARRAYSIZE(VE));
#else
	return L"";
#endif
}

string __FCTL_ToName(int Command)
{
#if defined(SYSLOG)
#define DEF_FCTL_(m) { FCTL_##m , L#m }
	__XXX_Name FCTL[]=
	{
		DEF_FCTL_(CLOSEPANEL),
		DEF_FCTL_(GETPANELINFO),
		DEF_FCTL_(UPDATEPANEL),
		DEF_FCTL_(REDRAWPANEL),
		DEF_FCTL_(GETCMDLINE),
		DEF_FCTL_(SETCMDLINE),
		DEF_FCTL_(SETSELECTION),
		DEF_FCTL_(SETVIEWMODE),
		DEF_FCTL_(INSERTCMDLINE),
		DEF_FCTL_(SETUSERSCREEN),
		DEF_FCTL_(SETPANELDIRECTORY),
		DEF_FCTL_(SETCMDLINEPOS),
		DEF_FCTL_(GETCMDLINEPOS),
		DEF_FCTL_(SETSORTMODE),
		DEF_FCTL_(SETSORTORDER),
		DEF_FCTL_(SETCMDLINESELECTION),
		DEF_FCTL_(GETCMDLINESELECTION),
		DEF_FCTL_(CHECKPANELSEXIST),
		DEF_FCTL_(SETNUMERICSORT),
		DEF_FCTL_(GETUSERSCREEN),
		DEF_FCTL_(ISACTIVEPANEL),
		DEF_FCTL_(GETPANELITEM),
		DEF_FCTL_(GETSELECTEDPANELITEM),
		DEF_FCTL_(GETCURRENTPANELITEM),
		DEF_FCTL_(GETPANELDIRECTORY),
		DEF_FCTL_(GETCOLUMNTYPES),
		DEF_FCTL_(GETCOLUMNWIDTHS),
		DEF_FCTL_(BEGINSELECTION),
		DEF_FCTL_(ENDSELECTION),
		DEF_FCTL_(CLEARSELECTION),
		DEF_FCTL_(SETDIRECTORIESFIRST),
		DEF_FCTL_(GETPANELFORMAT),
		DEF_FCTL_(GETPANELHOSTFILE),
		DEF_FCTL_(SETCASESENSITIVESORT),
		DEF_FCTL_(GETPANELPREFIX),
	};
	return _XXX_ToName(Command,L"FCTL",FCTL,ARRAYSIZE(FCTL));
#else
	return L"";
#endif
}

string __ACTL_ToName(int Command)
{
#if defined(SYSLOG_ACTL)
#define DEF_ACTL_(m) { ACTL_##m , L#m }
	__XXX_Name ACTL[]=
	{
		DEF_ACTL_(GETFARMANAGERVERSION),
		DEF_ACTL_(WAITKEY),
		DEF_ACTL_(GETCOLOR),
		DEF_ACTL_(GETARRAYCOLOR),
		DEF_ACTL_(EJECTMEDIA),
		DEF_ACTL_(GETWINDOWINFO),
		DEF_ACTL_(GETWINDOWCOUNT),
		DEF_ACTL_(SETCURRENTWINDOW),
		DEF_ACTL_(COMMIT),
		DEF_ACTL_(GETFARHWND),
		DEF_ACTL_(SETARRAYCOLOR),
		DEF_ACTL_(REDRAWALL),
		DEF_ACTL_(SYNCHRO),
		DEF_ACTL_(SETPROGRESSSTATE),
		DEF_ACTL_(SETPROGRESSVALUE),
		DEF_ACTL_(QUIT),
		DEF_ACTL_(GETFARRECT),
		DEF_ACTL_(GETCURSORPOS),
		DEF_ACTL_(SETCURSORPOS),
		DEF_ACTL_(PROGRESSNOTIFY),
		DEF_ACTL_(GETWINDOWTYPE),
		DEF_ACTL_(REMOVEMEDIA),
		DEF_ACTL_(GETMEDIATYPE),
	};
	return _XXX_ToName(Command,L"ACTL",ACTL,ARRAYSIZE(ACTL));
#else
	return L"";
#endif
}


string __VCTL_ToName(int Command)
{
#if defined(SYSLOG_VCTL)
#define DEF_VCTL_(m) { VCTL_##m , L#m }
	__XXX_Name VCTL[]=
	{
		DEF_VCTL_(GETINFO),
		DEF_VCTL_(QUIT),
		DEF_VCTL_(REDRAW),
		DEF_VCTL_(SETKEYBAR),
		DEF_VCTL_(SETPOSITION),
		DEF_VCTL_(SELECT),
		DEF_VCTL_(SETMODE),
	};
	return _XXX_ToName(Command,L"VCTL",VCTL,ARRAYSIZE(VCTL));
#else
	return L"";
#endif
}


string __MCODE_ToName(DWORD OpCode)
{
#if defined(SYSLOG)
#define DEF_MCODE_(m) { MCODE_##m , L#m }
	__XXX_Name MCODE[]=
	{
		DEF_MCODE_(OP_EXIT),  // принудительно закончить выполнение макропоследовательности
		DEF_MCODE_(OP_JMP),                     // Jumps..
		DEF_MCODE_(OP_JZ),
		DEF_MCODE_(OP_JNZ),
		DEF_MCODE_(OP_JLT),
		DEF_MCODE_(OP_JLE),
		DEF_MCODE_(OP_JGT),
		DEF_MCODE_(OP_JGE),
		DEF_MCODE_(OP_NOP),                     // нет операции
		DEF_MCODE_(OP_SAVE),                    // Присваивание переменной. Имя переменной следующие DWORD (как в $Text).
		DEF_MCODE_(OP_SAVEREPCOUNT),
		DEF_MCODE_(OP_PUSHUNKNOWN),             // неиницализированное значение (опускаемые параметры функций)
		DEF_MCODE_(OP_PUSHINT),                 // Положить значение на стек. Само
		DEF_MCODE_(OP_PUSHFLOAT),               // Положить значение на стек. double
		DEF_MCODE_(OP_PUSHSTR),                 // значение - следующий DWORD
		DEF_MCODE_(OP_PUSHVAR),                 // или несколько таковых (как в $Text)
		DEF_MCODE_(OP_PUSHCONST),               // в стек положить константу
		DEF_MCODE_(OP_REP),                     // $rep - признак начала цикла
		DEF_MCODE_(OP_END),                     // $end - признак конца цикла/условия
		DEF_MCODE_(OP_NEGATE),                  // -a
		DEF_MCODE_(OP_UPLUS),                   // +a
		DEF_MCODE_(OP_NOT),                     // !a
		DEF_MCODE_(OP_BITNOT),                  // ~a
		DEF_MCODE_(OP_MUL),                     // a *  b
		DEF_MCODE_(OP_DIV),                     // a /  b
		DEF_MCODE_(OP_ADD),                     // a +  b
		DEF_MCODE_(OP_SUB),                     // a -  b
		DEF_MCODE_(OP_BITSHR),                  // a >> b
		DEF_MCODE_(OP_BITSHL),                  // a << b
		DEF_MCODE_(OP_LT),                      // a <  b
		DEF_MCODE_(OP_LE),                      // a <= b
		DEF_MCODE_(OP_GT),                      // a >  b
		DEF_MCODE_(OP_GE),                      // a >= b
		DEF_MCODE_(OP_EQ),                      // a == b
		DEF_MCODE_(OP_NE),                      // a != b
		DEF_MCODE_(OP_PREINC),                  // ++a
		DEF_MCODE_(OP_PREDEC),                  // --a
		DEF_MCODE_(OP_POSTINC),                 // a++
		DEF_MCODE_(OP_POSTDEC),                 // a--
		DEF_MCODE_(OP_BITAND),                  // a &  b
		DEF_MCODE_(OP_BITXOR),                  // a ^  b
		DEF_MCODE_(OP_BITOR),                   // a |  b
		DEF_MCODE_(OP_AND),                     // a && b
		DEF_MCODE_(OP_XOR),                     // a ^^ b
		DEF_MCODE_(OP_OR),                      // a || b
		DEF_MCODE_(OP_ADDEQ),                   // a +=  b
		DEF_MCODE_(OP_SUBEQ),                   // a -=  b
		DEF_MCODE_(OP_MULEQ),                   // a *=  b
		DEF_MCODE_(OP_DIVEQ),                   // a /=  b
		DEF_MCODE_(OP_BITSHREQ),                // a >>= b
		DEF_MCODE_(OP_BITSHLEQ),                // a <<= b
		DEF_MCODE_(OP_BITANDEQ),                // a &=  b
		DEF_MCODE_(OP_BITXOREQ),                // a ^=  b
		DEF_MCODE_(OP_BITOREQ),                 // a |=  b
		DEF_MCODE_(OP_DISCARD),                 // убрать значение с вершины стека
		DEF_MCODE_(OP_DUP),                     // продублировать верхнее значение в стеке
		DEF_MCODE_(OP_SWAP),                    // обменять местами два значения в вершине стека
		DEF_MCODE_(OP_POP),                     // присвоить значение переменной и убрать из вершины стека
		DEF_MCODE_(OP_COPY),                    // %a=%d, стек не используется
		DEF_MCODE_(OP_KEYS),                    // за этим кодом следуют ФАРовы коды клавиш
		DEF_MCODE_(OP_ENDKEYS),                 // ФАРовы коды закончились.
		DEF_MCODE_(OP_IF),                      // Вообще-то эта группа в байткод
		DEF_MCODE_(OP_ELSE),                    // не попадет никогда :)
		DEF_MCODE_(OP_WHILE),
		DEF_MCODE_(OP_CONTINUE),                // $continue
		DEF_MCODE_(OP_BREAK),
		DEF_MCODE_(OP_XLAT),
		DEF_MCODE_(OP_PLAINTEXT),
		DEF_MCODE_(OP_AKEY),                    // $AKey - клавиша, которой вызвали макрос
		DEF_MCODE_(OP_SELWORD),                 // $SelWord - выделить "слово"
		DEF_MCODE_(F_NOFUNC),
		DEF_MCODE_(F_ABS),                      // N=abs(N)
		DEF_MCODE_(F_AKEY),                     // V=akey(Mode[,Type])
		DEF_MCODE_(F_ASC),                      // N=asc(S)
		DEF_MCODE_(F_ATOI),                     // N=atoi(S[,radix])
		DEF_MCODE_(F_CLIP),                     // V=clip(N[,V])
		DEF_MCODE_(F_CHR),                      // S=chr(N)
		DEF_MCODE_(F_DATE),                     // S=date([S])
		DEF_MCODE_(F_DLG_GETVALUE),             // V=Dlg.GetValue([Pos[,InfoID]])
		DEF_MCODE_(F_EDITOR_SEL),               // V=Editor.Sel(Action[,Opt])
		DEF_MCODE_(F_EDITOR_SET),               // N=Editor.Set(N,Var)
		DEF_MCODE_(F_EDITOR_UNDO),              // V=Editor.Undo(N)
		DEF_MCODE_(F_EDITOR_POS),               // N=Editor.Pos(Op,What[,Where])
		DEF_MCODE_(F_ENVIRON),                  // S=Env(S[,Mode[,Value]])
		DEF_MCODE_(F_FATTR),                    // N=fattr(S)
		DEF_MCODE_(F_FEXIST),                   // S=fexist(S)
		DEF_MCODE_(F_FSPLIT),                   // S=fsplit(S,N)
		DEF_MCODE_(F_IIF),                      // V=iif(C,V1,V2)
		DEF_MCODE_(F_INDEX),                    // S=index(S1,S2[,Mode])
		DEF_MCODE_(F_INT),                      // N=int(V)
		DEF_MCODE_(F_ITOA),                     // S=itoa(N[,radix])
		DEF_MCODE_(F_KEY),                      // S=key(V)
		DEF_MCODE_(F_LCASE),                    // S=lcase(S1)
		DEF_MCODE_(F_LEN),                      // N=len(S)
		DEF_MCODE_(F_MAX),                      // N=max(N1,N2)
		DEF_MCODE_(F_MENU_CHECKHOTKEY),         // N=checkhotkey(S[,N])
		DEF_MCODE_(F_MENU_GETHOTKEY),           // S=gethotkey([N])
		DEF_MCODE_(F_MENU_SELECT),              // N=Menu.Select(S[,N[,Dir]])
		DEF_MCODE_(F_MENU_SHOW),                // S=Menu.Show(Items[,Title[,Flags[,FindOrFilter[,X[,Y]]]]])
		DEF_MCODE_(F_MIN),                      // N=min(N1,N2)
		DEF_MCODE_(F_MOD),                      // N=mod(a,b) == a %  b
		DEF_MCODE_(F_MLOAD),                    // B=mload(var)
		DEF_MCODE_(F_MSAVE),                    // B=msave(var)
		DEF_MCODE_(F_MSGBOX),                   // N=msgbox(["Title"[,"Text"[,flags]]])
		DEF_MCODE_(F_PANEL_FATTR),              // N=Panel.FAttr(panelType,fileMask)
		DEF_MCODE_(F_PANEL_SETPATH),            // N=panel.SetPath(panelType,pathName[,fileName])
		DEF_MCODE_(F_PANEL_FEXIST),             // N=Panel.FExist(panelType,fileMask)
		DEF_MCODE_(F_PANEL_SETPOS),             // N=Panel.SetPos(panelType,fileName)
		DEF_MCODE_(F_PANEL_SETPOSIDX),          // N=Panel.SetPosIdx(panelType,Idx[,InSelection])
		DEF_MCODE_(F_PANEL_SELECT),             // V=Panel.Select(panelType,Action[,Mode[,Items]])
		DEF_MCODE_(F_PANELITEM),                // V=PanelItem(Panel,Index,TypeInfo)
		DEF_MCODE_(F_EVAL),                     // N=eval(S[,N])
		DEF_MCODE_(F_RINDEX),                   // S=rindex(S1,S2[,Mode])
		DEF_MCODE_(F_SLEEP),                    // Sleep(N)
		DEF_MCODE_(F_STRING),                   // S=string(V)
		DEF_MCODE_(F_STRPAD),                   // S=StrPad(V,Cnt[,Fill[,Op]])
		DEF_MCODE_(F_SUBSTR),                   // S=substr(S,start[,length])
		DEF_MCODE_(F_UCASE),                    // S=ucase(S1)
		DEF_MCODE_(F_WAITKEY),                  // V=waitkey([N,[T]])
		DEF_MCODE_(F_XLAT),                     // S=xlat(S)
		DEF_MCODE_(F_FLOCK),                    // N=FLock(N,N)
		DEF_MCODE_(F_CALLPLUGIN),               // V=callplugin(SysID[,param])
		DEF_MCODE_(F_REPLACE),                  // S=replace(sS,sF,sR[,Count[,Mode]])
		DEF_MCODE_(F_PROMPT),                   // S=prompt(["Title"[,"Prompt"[,flags[, "Src"[, "History"]]]]])
		DEF_MCODE_(F_BM_ADD),                   // N=BM.Add()  - добавить текущие координаты и обрезать хвост
		DEF_MCODE_(F_BM_CLEAR),                 // N=BM.Clear() - очистить все закладки
		DEF_MCODE_(F_BM_DEL),                   // N=BM.Del([Idx]) - удаляет закладку с указанным индексом (x=1...), 0 - удаляет текущую закладку
		DEF_MCODE_(F_BM_GET),                   // N=BM.Get(Idx,M) - возвращает координаты строки (M==0) или колонки (M==1) закладки с индексом (Idx=1...)
		DEF_MCODE_(F_BM_GOTO),                  // N=BM.Goto([n]) - переход на закладку с указанным индексом (0 --> текущую)
		DEF_MCODE_(F_BM_NEXT),                  // N=BM.Next() - перейти на следующую закладку
		DEF_MCODE_(F_BM_POP),                   // N=BM.Pop() - восстановить текущую позицию из закладки в конце стека и удалить закладку
		DEF_MCODE_(F_BM_PREV),                  // N=BM.Prev() - перейти на предыдущую закладку
		DEF_MCODE_(F_BM_BACK),                  // N=BM.Back() - перейти на предыдущую закладку с возможным сохранением текущей позиции
		DEF_MCODE_(F_BM_PUSH),                  // N=BM.Push() - сохранить текущую позицию в виде закладки в конце стека
		DEF_MCODE_(F_BM_STAT),                  // N=BM.Stat([M]) - возвращает информацию о закладках, N=0 - текущее количество закладок
		DEF_MCODE_(F_TRIM),                     // S=trim(S[,N])
		DEF_MCODE_(F_FLOAT),                    // N=float(V)
		DEF_MCODE_(F_TESTFOLDER),               // N=testfolder(S)
		DEF_MCODE_(F_PRINT),                    // N=Print(Str)
		DEF_MCODE_(F_MMODE),                    // N=MMode(Action[,Value])
		DEF_MCODE_(F_EDITOR_SETTITLE),          // N=Editor.SetTitle([Title])
		DEF_MCODE_(F_MENU_GETVALUE),            // S=Menu.GetValue([N])
		DEF_MCODE_(F_MENU_ITEMSTATUS),          // N=Menu.ItemStatus([N])
		DEF_MCODE_(F_BEEP),                     // N=beep([N])
		DEF_MCODE_(F_KBDLAYOUT),                // N=kbdLayout([N])
		DEF_MCODE_(F_WINDOW_SCROLL),            // N=Window.Scroll(Lines[,Axis])
		DEF_MCODE_(F_KEYBAR_SHOW),              // N=KeyBar.Show([N])
		DEF_MCODE_(F_HISTIORY_DISABLE),         // N=History.Disable([State])
		DEF_MCODE_(F_FMATCH),                   // N=FMatch(S,Mask)
		DEF_MCODE_(F_PLUGIN_MENU),              // N=Plugin.Menu(Guid[,MenuGuid])
		DEF_MCODE_(F_PLUGIN_CONFIG),            // N=Plugin.Config(Guid[,MenuGuid])
		DEF_MCODE_(F_PLUGIN_CALL),              // N=Plugin.Call(Guid[,Item])
		DEF_MCODE_(F_PLUGIN_LOAD),              // N=Plugin.Load(DllPath[,ForceLoad])
		DEF_MCODE_(F_PLUGIN_COMMAND),           // N=Plugin.Command(Guid[,Command])
		DEF_MCODE_(F_PLUGIN_UNLOAD),            // N=Plugin.UnLoad(DllPath)
		DEF_MCODE_(F_PLUGIN_EXIST),             // N=Plugin.Exist(Guid)
		DEF_MCODE_(F_MENU_FILTER),              // N=Menu.Filter(Action[,Mode])
		DEF_MCODE_(F_MENU_FILTERSTR),           // S=Menu.FilterStr([Action[,S]])
		DEF_MCODE_(F_DLG_SETFOCUS),             // N=Dlg.SetFocus([ID])
		DEF_MCODE_(F_FAR_CFG_GET),              // V=Far.Cfg.Get(Key,Name)
		DEF_MCODE_(F_SIZE2STR),                 // S=Size2Str(N,Flags[,Width])
		DEF_MCODE_(F_STRWRAP),                  // S=StrWrap(Text,Width[,Break[,Flags]])
		DEF_MCODE_(F_MACRO_KEYWORD),            // S=Macro.Keyword(Index[,Type])
		DEF_MCODE_(F_MACRO_FUNC),               // S=Macro.Func(Index[,Type])
		DEF_MCODE_(F_MACRO_VAR),                // S=Macro.Var(Index[,Type])
		DEF_MCODE_(F_MACRO_CONST),              // S=Macro.Const(Index[,Type])
		DEF_MCODE_(F_EDITOR_DELLINE),           // N=Editor.DelLine([Line])
		DEF_MCODE_(F_EDITOR_GETSTR),            // S=Editor.GetStr([Line])
		DEF_MCODE_(F_EDITOR_INSSTR),            // N=Editor.InsStr([S[,Line]])
		DEF_MCODE_(F_EDITOR_SETSTR),            // N=Editor.SetStr([S[,Line]])
		DEF_MCODE_(C_AREA_OTHER),               // Режим копирования текста с экрана, вертикальные меню
		DEF_MCODE_(C_AREA_SHELL),               // Файловые панели
		DEF_MCODE_(C_AREA_VIEWER),              // Внутренняя программа просмотра
		DEF_MCODE_(C_AREA_EDITOR),              // Редактор
		DEF_MCODE_(C_AREA_DIALOG),              // Диалоги
		DEF_MCODE_(C_AREA_SEARCH),              // Быстрый поиск в панелях
		DEF_MCODE_(C_AREA_DISKS),               // Меню выбора дисков
		DEF_MCODE_(C_AREA_MAINMENU),            // Основное меню
		DEF_MCODE_(C_AREA_MENU),                // Прочие меню
		DEF_MCODE_(C_AREA_HELP),                // Система помощи
		DEF_MCODE_(C_AREA_INFOPANEL),           // Информационная панель
		DEF_MCODE_(C_AREA_QVIEWPANEL),          // Панель быстрого просмотра
		DEF_MCODE_(C_AREA_TREEPANEL),           // Панель дерева папок
		DEF_MCODE_(C_AREA_FINDFOLDER),          // Поиск папок
		DEF_MCODE_(C_AREA_USERMENU),            // Меню пользователя
		DEF_MCODE_(C_AREA_SHELL_AUTOCOMPLETION),// Список автодополнения в панелях в ком.строке
		DEF_MCODE_(C_AREA_DIALOG_AUTOCOMPLETION),// Список автодополнения в диалоге
		DEF_MCODE_(C_FULLSCREENMODE),           // полноэкранный режим?
		DEF_MCODE_(C_ISUSERADMIN),              // Administrator status
		DEF_MCODE_(C_BOF),                      // начало файла/активного каталога?
		DEF_MCODE_(C_EOF),                      // конец файла/активного каталога?
		DEF_MCODE_(C_EMPTY),                    // ком.строка пуста?
		DEF_MCODE_(C_SELECTED),                 // выделенный блок есть?
		DEF_MCODE_(C_ROOTFOLDER),               // аналог MCODE_C_APANEL_ROOT для активной панели
		DEF_MCODE_(C_APANEL_BOF),               // начало активного  каталога?
		DEF_MCODE_(C_PPANEL_BOF),               // начало пассивного каталога?
		DEF_MCODE_(C_APANEL_EOF),               // конец активного  каталога?
		DEF_MCODE_(C_PPANEL_EOF),               // конец пассивного каталога?
		DEF_MCODE_(C_APANEL_ISEMPTY),           // активная панель:  пуста?
		DEF_MCODE_(C_PPANEL_ISEMPTY),           // пассивная панель: пуста?
		DEF_MCODE_(C_APANEL_SELECTED),          // активная панель:  выделенные элементы есть?
		DEF_MCODE_(C_PPANEL_SELECTED),          // пассивная панель: выделенные элементы есть?
		DEF_MCODE_(C_APANEL_ROOT),              // это корневой каталог активной панели?
		DEF_MCODE_(C_PPANEL_ROOT),              // это корневой каталог пассивной панели?
		DEF_MCODE_(C_APANEL_VISIBLE),           // активная панель:  видима?
		DEF_MCODE_(C_PPANEL_VISIBLE),           // пассивная панель: видима?
		DEF_MCODE_(C_APANEL_PLUGIN),            // активная панель:  плагиновая?
		DEF_MCODE_(C_PPANEL_PLUGIN),            // пассивная панель: плагиновая?
		DEF_MCODE_(C_APANEL_FILEPANEL),         // активная панель:  файловая?
		DEF_MCODE_(C_PPANEL_FILEPANEL),         // пассивная панель: файловая?
		DEF_MCODE_(C_APANEL_FOLDER),            // активная панель:  текущий элемент каталог?
		DEF_MCODE_(C_PPANEL_FOLDER),            // пассивная панель: текущий элемент каталог?
		DEF_MCODE_(C_APANEL_LEFT),              // активная панель левая?
		DEF_MCODE_(C_PPANEL_LEFT),              // пассивная панель левая?
		DEF_MCODE_(C_APANEL_LFN),               // на активной панели длинные имена?
		DEF_MCODE_(C_PPANEL_LFN),               // на пассивной панели длинные имена?
		DEF_MCODE_(C_APANEL_FILTER),            // на активной панели включен фильтр?
		DEF_MCODE_(C_PPANEL_FILTER),            // на пассивной панели включен фильтр?
		DEF_MCODE_(C_CMDLINE_BOF),              // курсор в начале cmd-строки редактирования?
		DEF_MCODE_(C_CMDLINE_EOF),              // курсор в конце cmd-строки редактирования?
		DEF_MCODE_(C_CMDLINE_EMPTY),            // ком.строка пуста?
		DEF_MCODE_(C_CMDLINE_SELECTED),         // в ком.строке есть выделение блока?
		DEF_MCODE_(V_FAR_WIDTH),                // Far.Width - ширина консольного окна
		DEF_MCODE_(V_FAR_HEIGHT),               // Far.Height - высота консольного окна
		DEF_MCODE_(V_FAR_TITLE),                // Far.Title - текущий заголовок консольного окна
		DEF_MCODE_(V_FAR_UPTIME),               // Far.UpTime - время работы Far в миллисекундах
		DEF_MCODE_(V_FAR_PID),                  // Far.PID - содержит ИД текущей запущенной копии Far Manager
		DEF_MCODE_(V_MACRO_AREA),               // Macro.Area - имя текущей макрос области
		DEF_MCODE_(V_APANEL_CURRENT),           // APanel.Current - имя файла на активной панели
		DEF_MCODE_(V_PPANEL_CURRENT),           // PPanel.Current - имя файла на пассивной панели
		DEF_MCODE_(V_APANEL_SELCOUNT),          // APanel.SelCount - активная панель:  число выделенных элементов
		DEF_MCODE_(V_PPANEL_SELCOUNT),          // PPanel.SelCount - пассивная панель: число выделенных элементов
		DEF_MCODE_(V_APANEL_PATH),              // APanel.Path - активная панель:  путь на панели
		DEF_MCODE_(V_PPANEL_PATH),              // PPanel.Path - пассивная панель: путь на панели
		DEF_MCODE_(V_APANEL_PATH0),             // APanel.Path0 - активная панель:  путь на панели до вызова плагинов
		DEF_MCODE_(V_PPANEL_PATH0),             // PPanel.Path0 - пассивная панель: путь на панели до вызова плагинов
		DEF_MCODE_(V_APANEL_UNCPATH),           // APanel.UNCPath - активная панель:  UNC-путь на панели
		DEF_MCODE_(V_PPANEL_UNCPATH),           // PPanel.UNCPath - пассивная панель: UNC-путь на панели
		DEF_MCODE_(V_APANEL_WIDTH),             // APanel.Width - активная панель:  ширина панели
		DEF_MCODE_(V_PPANEL_WIDTH),             // PPanel.Width - пассивная панель: ширина панели
		DEF_MCODE_(V_APANEL_TYPE),              // APanel.Type - тип активной панели
		DEF_MCODE_(V_PPANEL_TYPE),              // PPanel.Type - тип пассивной панели
		DEF_MCODE_(V_APANEL_ITEMCOUNT),         // APanel.ItemCount - активная панель:  число элементов
		DEF_MCODE_(V_PPANEL_ITEMCOUNT),         // PPanel.ItemCount - пассивная панель: число элементов
		DEF_MCODE_(V_APANEL_CURPOS),            // APanel.CurPos - активная панель:  текущий индекс
		DEF_MCODE_(V_PPANEL_CURPOS),            // PPanel.CurPos - пассивная панель: текущий индекс
		DEF_MCODE_(V_APANEL_OPIFLAGS),          // APanel.OPIFlags - активная панель: флаги открытого плагина
		DEF_MCODE_(V_PPANEL_OPIFLAGS),          // PPanel.OPIFlags - пассивная панель: флаги открытого плагина
		DEF_MCODE_(V_APANEL_DRIVETYPE),         // APanel.DriveType - активная панель: тип привода
		DEF_MCODE_(V_PPANEL_DRIVETYPE),         // PPanel.DriveType - пассивная панель: тип привода
		DEF_MCODE_(V_APANEL_HEIGHT),            // APanel.Height - активная панель:  высота панели
		DEF_MCODE_(V_PPANEL_HEIGHT),            // PPanel.Height - пассивная панель: высота панели
		DEF_MCODE_(V_APANEL_COLUMNCOUNT),       // APanel.ColumnCount - активная панель:  количество колонок
		DEF_MCODE_(V_PPANEL_COLUMNCOUNT),       // PPanel.ColumnCount - пассивная панель: количество колонок
		DEF_MCODE_(V_APANEL_HOSTFILE),          // APanel.HostFile - активная панель:  имя Host-файла
		DEF_MCODE_(V_PPANEL_HOSTFILE),          // PPanel.HostFile - пассивная панель: имя Host-файла
		DEF_MCODE_(V_APANEL_PREFIX),            // APanel.Prefix
		DEF_MCODE_(V_PPANEL_PREFIX),            // PPanel.Prefix
		DEF_MCODE_(V_APANEL_FORMAT),            // APanel.Format
		DEF_MCODE_(V_PPANEL_FORMAT),            // PPanel.Format
		DEF_MCODE_(V_ITEMCOUNT),                // ItemCount - число элементов в текущем объекте
		DEF_MCODE_(V_CURPOS),                   // CurPos - текущий индекс в текущем объекте
		DEF_MCODE_(V_TITLE),                    // Title - заголовок текущего объекта
		DEF_MCODE_(V_HEIGHT),                   // Height - высота текущего объекта
		DEF_MCODE_(V_WIDTH),                    // Width - ширина текущего объекта
		DEF_MCODE_(V_EDITORFILENAME),           // Editor.FileName - имя редактируемого файла
		DEF_MCODE_(V_EDITORLINES),              // Editor.Lines - количество строк в редакторе
		DEF_MCODE_(V_EDITORCURLINE),            // Editor.CurLine - текущая линия в редакторе (в дополнении к Count)
		DEF_MCODE_(V_EDITORCURPOS),             // Editor.CurPos - текущая поз. в редакторе
		DEF_MCODE_(V_EDITORREALPOS),            // Editor.RealPos - текущая поз. в редакторе без привязки к размеру табуляции
		DEF_MCODE_(V_EDITORSTATE),              // Editor.State
		DEF_MCODE_(V_EDITORVALUE),              // Editor.Value - содержимое текущей строки
		DEF_MCODE_(V_EDITORSELVALUE),           // Editor.SelValue - содержит содержимое выделенного блока
		DEF_MCODE_(V_DLGITEMTYPE),              // Dlg.ItemType
		DEF_MCODE_(V_DLGITEMCOUNT),             // Dlg.ItemCount
		DEF_MCODE_(V_DLGCURPOS),                // Dlg.CurPos
		DEF_MCODE_(V_DLGPREVPOS),               // Dlg.PrevPos
		DEF_MCODE_(V_DLGINFOID),                // Dlg.Info.Id
		DEF_MCODE_(V_DLGINFOOWNER),             // Dlg.Info.Owner
		DEF_MCODE_(V_VIEWERFILENAME),           // Viewer.FileName - имя просматриваемого файла
		DEF_MCODE_(V_VIEWERSTATE),              // Viewer.State
		DEF_MCODE_(V_CMDLINE_ITEMCOUNT),        // CmdLine.ItemCount
		DEF_MCODE_(V_CMDLINE_CURPOS),           // CmdLine.CurPos
		DEF_MCODE_(V_CMDLINE_VALUE),            // CmdLine.Value
		DEF_MCODE_(V_DRVSHOWPOS),               // Drv.ShowPos - меню выбора дисков отображено: 1=слева (Alt-F1), 2=справа (Alt-F2), 0="нету его"
		DEF_MCODE_(V_DRVSHOWMODE),              // Drv.ShowMode - режимы отображения меню выбора дисков
		DEF_MCODE_(V_HELPFILENAME),             // Help.FileName
		DEF_MCODE_(V_HELPTOPIC),                // Help.Topic
		DEF_MCODE_(V_HELPSELTOPIC),             // Help.SelTopic
		DEF_MCODE_(V_MENU_VALUE),               // Menu.Value
		DEF_MCODE_(V_MENUINFOID),               // Menu.Info.Id
	};
	string Name;

	for (size_t i=0; i<ARRAYSIZE(MCODE); i++)
	{
		if (MCODE[i].Val == OpCode)
		{
			Name.Format(L"%08X | MCODE_%-20s",OpCode,MCODE[i].Name);
			return Name;
		}
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

	if (!(Key >= KEY_MACRO_BASE && Key <= KEY_MACRO_ENDBASE) && KeyToText(Key,Name))
	{
		string tmp;
		InsertQuote(Name);
		tmp.Format(L"%s [%u/0x%08X]",Name.CPtr(),Key,Key);
		Name = tmp;
		return Name;
	}

	Name.Format(L"\"KEY_????\" [%u/0x%08X]",Key,Key);
	return Name;
#else
	return L"";
#endif
}


string __DLGMSG_ToName(DWORD Msg)
{
#if defined(SYSLOG)
#define DEF_DM_(m) { DM_##m , L#m }
#define DEF_DN_(m) { DN_##m , L#m }
	__XXX_Name Message[]=
	{
		DEF_DM_(FIRST),
		DEF_DM_(CLOSE),
		DEF_DM_(ENABLE),
		DEF_DM_(ENABLEREDRAW),
		DEF_DM_(GETDLGDATA),
		DEF_DM_(GETDLGITEM),
		DEF_DM_(GETDLGRECT),
		DEF_DM_(GETTEXT),
		DEF_DM_(KEY),
		DEF_DM_(MOVEDIALOG),
		DEF_DM_(SETDLGDATA),
		DEF_DM_(SETDLGITEM),
		DEF_DM_(SETFOCUS),
		DEF_DM_(REDRAW),
		DEF_DM_(SETTEXT),
		DEF_DM_(SETMAXTEXTLENGTH),
		DEF_DM_(SHOWDIALOG),
		DEF_DM_(GETFOCUS),
		DEF_DM_(GETCURSORPOS),
		DEF_DM_(SETCURSORPOS),
		DEF_DM_(SETTEXTPTR),
		DEF_DM_(SHOWITEM),
		DEF_DM_(ADDHISTORY),

		DEF_DM_(GETCHECK),
		DEF_DM_(SETCHECK),
		DEF_DM_(SET3STATE),

		DEF_DM_(LISTSORT),
		DEF_DM_(LISTGETITEM),
		DEF_DM_(LISTGETCURPOS),
		DEF_DM_(LISTSETCURPOS),
		DEF_DM_(LISTDELETE),
		DEF_DM_(LISTADD),
		DEF_DM_(LISTADDSTR),
		DEF_DM_(LISTUPDATE),
		DEF_DM_(LISTINSERT),
		DEF_DM_(LISTFINDSTRING),
		DEF_DM_(LISTINFO),
		DEF_DM_(LISTGETDATA),
		DEF_DM_(LISTSETDATA),
		DEF_DM_(LISTSETTITLES),
		DEF_DM_(LISTGETTITLES),

		DEF_DM_(RESIZEDIALOG),
		DEF_DM_(SETITEMPOSITION),

		DEF_DM_(GETDROPDOWNOPENED),
		DEF_DM_(SETDROPDOWNOPENED),

		DEF_DM_(SETHISTORY),

		DEF_DM_(GETITEMPOSITION),
		DEF_DM_(SETMOUSEEVENTNOTIFY),

		DEF_DM_(EDITUNCHANGEDFLAG),

		DEF_DM_(GETITEMDATA),
		DEF_DM_(SETITEMDATA),

		DEF_DM_(LISTSET),

		DEF_DM_(GETCURSORSIZE),
		DEF_DM_(SETCURSORSIZE),

		DEF_DM_(LISTGETDATASIZE),

		DEF_DM_(GETSELECTION),
		DEF_DM_(SETSELECTION),

		DEF_DM_(GETEDITPOSITION),
		DEF_DM_(SETEDITPOSITION),

		DEF_DM_(SETCOMBOBOXEVENT),
		DEF_DM_(GETCOMBOBOXEVENT),

		DEF_DM_(GETCONSTTEXTPTR),
		DEF_DM_(GETDLGITEMSHORT),
		DEF_DM_(SETDLGITEMSHORT),

		DEF_DM_(GETDIALOGINFO),

		DEF_DN_(FIRST),
		DEF_DN_(BTNCLICK),
		DEF_DN_(CTLCOLORDIALOG),
		DEF_DN_(CTLCOLORDLGITEM),
		DEF_DN_(CTLCOLORDLGLIST),
		DEF_DN_(DRAWDIALOG),
		DEF_DN_(DRAWDLGITEM),
		DEF_DN_(EDITCHANGE),
		DEF_DN_(ENTERIDLE),
		DEF_DN_(GOTFOCUS),
		DEF_DN_(HELP),
		DEF_DN_(HOTKEY),
		DEF_DN_(INITDIALOG),
		DEF_DN_(KILLFOCUS),
		DEF_DN_(LISTCHANGE),
		DEF_DN_(DRAGGED),
		DEF_DN_(RESIZECONSOLE),
		DEF_DN_(DRAWDIALOGDONE),
		DEF_DN_(LISTHOTKEY),
		DEF_DN_(INPUT),
		DEF_DN_(CONTROLINPUT),
		DEF_DN_(CLOSE),
		DEF_DN_(GETVALUE),

		DEF_DM_(USER),

		DEF_DM_(KILLSAVESCREEN),
		DEF_DM_(ALLKEYMODE),
		DEF_DN_(ACTIVATEAPP),
	};
	string Name;

	for (size_t i=0; i<ARRAYSIZE(Message); i++)
	{
		if (Message[i].Val == Msg)
		{
			Name.Format(L"\"%s\" [%d/0x%08X]",Message[i].Name,Msg,Msg);
			return Name;
		}
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
	__XXX_Name VK[]=
	{
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

	if (VkKey >= L'0' && VkKey <= L'9' || VkKey >= L'A' && VkKey <= L'Z')
	{
		string Name;
		Name.Format(L"\"VK_%c\" [%d/0x%04X]",VkKey,VkKey,VkKey);
		return Name;
	}
	else
		return _XXX_ToName(VkKey,L"VK",VK,ARRAYSIZE(VK));

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

	if (rec->dwEventFlags==MOUSE_WHEELED  || rec->dwEventFlags==MOUSE_HWHEELED)
	{
		string tmp;
		tmp.Format(L" (Delta=%d)",HIWORD(rec->dwButtonState));
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

	if (!rec)
		return L"(null)";

	switch (rec->EventType)
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
			    ((rec->Event.KeyEvent.uChar.UnicodeChar && !(rec->Event.KeyEvent.uChar.UnicodeChar == L'\t' || rec->Event.KeyEvent.uChar.UnicodeChar == L'\r' || rec->Event.KeyEvent.uChar.UnicodeChar == L'\n'))?rec->Event.KeyEvent.uChar.UnicodeChar:L' '),
			    rec->Event.KeyEvent.uChar.UnicodeChar,
			    ((AsciiChar && AsciiChar != '\r' && AsciiChar != '\t' && AsciiChar !='\n')? AsciiChar : ' '),
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

	Records += FormatString() << L" (" << (IsConsoleFullscreen()?L"Fullscreen":L"Widowed") << L")";
	return Records;
#else
	return L"";
#endif
}

void INPUT_RECORD_DumpBuffer(FILE *fp)
{
#if defined(SYSLOG)

	if (!IsLogON())
		return;

	int InternalLog=fp?FALSE:TRUE;
	size_t ReadCount2;
	// берем количество оставшейся порции эвентов
	Console.GetNumberOfInputEvents(ReadCount2);

	if (ReadCount2 <= 1)
		return;

	if (InternalLog)
	{
		OpenSysLog();
		fp=LogStream;

		if (fp)
		{
			wchar_t timebuf[64];
			fwprintf(fp,L"%s %s(Number Of Console Input Events = %u)\n",PrintTime(timebuf,ARRAYSIZE(timebuf)),MakeSpace(),static_cast<unsigned>(ReadCount2));
		}
	}

	if (fp)
	{
		if (ReadCount2 > 1)
		{
			INPUT_RECORD *TmpRec=(INPUT_RECORD*)xf_malloc(sizeof(INPUT_RECORD)*ReadCount2);

			if (TmpRec)
			{
				size_t ReadCount3;
				Console.PeekInput(TmpRec, ReadCount2, ReadCount3);

				for (DWORD I=0; I < ReadCount2; ++I)
				{
					fwprintf(fp,L"             %s%04u: %s\n",MakeSpace(),I,_INPUT_RECORD_Dump(TmpRec+I));
				}

				// освободим память
				xf_free(TmpRec);
			}
		}

		fflush(fp);
	}

	if (InternalLog)
		CloseSysLog();

#endif
}

// после вызова этой функции нужно освободить память!!!
string __SysLog_LinearDump(LPBYTE Buf,int SizeBuf)
{
#if defined(SYSLOG)
	string OutBuf, tmp;

	for (int I=0; I < SizeBuf; ++I)
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

	if (!IsLogON())
		return;

	int InternalLog=fp?FALSE:TRUE;
	const wchar_t *space=MakeSpace();

	if (InternalLog)
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

		if (lpFileSystemFlags&FILE_CASE_PRESERVED_NAMES)
			fwprintf(fp,L"%*s %s         FILE_CASE_PRESERVED_NAMES\n",12,L"",space);

		if (lpFileSystemFlags&FILE_CASE_SENSITIVE_SEARCH)
			fwprintf(fp,L"%*s %s         FILE_CASE_SENSITIVE_SEARCH\n",12,L"",space);

		if (lpFileSystemFlags&FILE_FILE_COMPRESSION)
			fwprintf(fp,L"%*s %s         FILE_FILE_COMPRESSION\n",12,L"",space);

		if (lpFileSystemFlags&FILE_NAMED_STREAMS)
			fwprintf(fp,L"%*s %s         FILE_NAMED_STREAMS\n",12,L"",space);

		if (lpFileSystemFlags&FILE_PERSISTENT_ACLS)
			fwprintf(fp,L"%*s %s         FILE_PERSISTENT_ACLS\n",12,L"",space);

		if (lpFileSystemFlags&FILE_READ_ONLY_VOLUME)
			fwprintf(fp,L"%*s %s         FILE_READ_ONLY_VOLUME\n",12,L"",space);

		if (lpFileSystemFlags&FILE_SEQUENTIAL_WRITE_ONCE)
			fwprintf(fp,L"%*s %s         FILE_SEQUENTIAL_WRITE_ONCE\n",12,L"",space);

		if (lpFileSystemFlags&FILE_SUPPORTS_ENCRYPTION)
			fwprintf(fp,L"%*s %s         FILE_SUPPORTS_ENCRYPTION\n",12,L"",space);

		if (lpFileSystemFlags&FILE_SUPPORTS_OBJECT_IDS)
			fwprintf(fp,L"%*s %s         FILE_SUPPORTS_OBJECT_IDS\n",12,L"",space);

		if (lpFileSystemFlags&FILE_SUPPORTS_REPARSE_POINTS)
			fwprintf(fp,L"%*s %s         FILE_SUPPORTS_REPARSE_POINTS\n",12,L"",space);

		if (lpFileSystemFlags&FILE_SUPPORTS_SPARSE_FILES)
			fwprintf(fp,L"%*s %s         FILE_SUPPORTS_SPARSE_FILES\n",12,L"",space);

		if (lpFileSystemFlags&FILE_SUPPORTS_TRANSACTIONS)
			fwprintf(fp,L"%*s %s         FILE_SUPPORTS_TRANSACTIONS\n",12,L"",space);

		if (lpFileSystemFlags&FILE_UNICODE_ON_DISK)
			fwprintf(fp,L"%*s %s         FILE_UNICODE_ON_DISK\n",12,L"",space);

		if (lpFileSystemFlags&FILE_VOLUME_IS_COMPRESSED)
			fwprintf(fp,L"%*s %s         FILE_VOLUME_IS_COMPRESSED\n",12,L"",space);

		if (lpFileSystemFlags&FILE_VOLUME_QUOTAS)
			fwprintf(fp,L"%*s %s         FILE_VOLUME_QUOTAS\n",12,L"",space);

		fwprintf(fp,L"%*s %s    lpFileSystemNameBuffer    ='%s'\n",12,L"",space,lpFileSystemNameBuffer);
		fwprintf(fp,L"%*s %s    nFileSystemNameSize       =%u\n",12,L"",space,nFileSystemNameSize);
		fwprintf(fp,L"%*s %s  }\n",12,L"",space);
		fflush(fp);
	}

	if (InternalLog)
		CloseSysLog();

#endif
}


void WIN32_FIND_DATA_Dump(const wchar_t *Title,const WIN32_FIND_DATA &wfd,FILE *fp)
{
#if defined(SYSLOG)

	if (!IsLogON())
		return;

	int InternalLog=fp?FALSE:TRUE;
	const wchar_t *space=MakeSpace();

	if (InternalLog)
	{
		OpenSysLog();
		fp=PrintBaner(fp,L"WIN32_FIND_DATA",Title);
	}

	if (fp)
	{
		fwprintf(fp,L"%*s %s  dwFileAttributes      =0x%08X\n",12,L"",space,wfd.dwFileAttributes);

		if (wfd.dwFileAttributes&FILE_ATTRIBUTE_READONLY)
			fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_READONLY            (0x00000001)\n",12,L"",space);

		if (wfd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)
			fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_HIDDEN              (0x00000002)\n",12,L"",space);

		if (wfd.dwFileAttributes&FILE_ATTRIBUTE_SYSTEM)
			fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_SYSTEM              (0x00000004)\n",12,L"",space);

		if (wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_DIRECTORY           (0x00000010)\n",12,L"",space);

		if (wfd.dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE)
			fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_ARCHIVE             (0x00000020)\n",12,L"",space);

		if (wfd.dwFileAttributes&FILE_ATTRIBUTE_DEVICE)
			fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_DEVICE              (0x00000040)\n",12,L"",space);

		if (wfd.dwFileAttributes&FILE_ATTRIBUTE_NORMAL)
			fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_NORMAL              (0x00000080)\n",12,L"",space);

		if (wfd.dwFileAttributes&FILE_ATTRIBUTE_TEMPORARY)
			fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_TEMPORARY           (0x00000100)\n",12,L"",space);

		if (wfd.dwFileAttributes&FILE_ATTRIBUTE_SPARSE_FILE)
			fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_SPARSE_FILE         (0x00000200)\n",12,L"",space);

		if (wfd.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
			fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_REPARSE_POINT       (0x00000400)\n",12,L"",space);

		if (wfd.dwFileAttributes&FILE_ATTRIBUTE_COMPRESSED)
			fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_COMPRESSED          (0x00000800)\n",12,L"",space);

		if (wfd.dwFileAttributes&FILE_ATTRIBUTE_OFFLINE)
			fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_OFFLINE             (0x00001000)\n",12,L"",space);

		if (wfd.dwFileAttributes&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)
			fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_NOT_CONTENT_INDEXED (0x00002000)\n",12,L"",space);

		if (wfd.dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED)
			fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_ENCRYPTED           (0x00004000)\n",12,L"",space);

		if (wfd.dwFileAttributes&FILE_ATTRIBUTE_VIRTUAL)
			fwprintf(fp,L"%*s %s     FILE_ATTRIBUTE_VIRTUAL             (0x00010000)\n",12,L"",space);

		string D, T;
		ConvertDate(wfd.ftCreationTime,D,T,8,FALSE,FALSE,TRUE);
		fwprintf(fp,L"%*s %s  ftCreationTime        =0x%08X 0x%08X\n",12,L"",space,wfd.ftCreationTime.dwHighDateTime,wfd.ftCreationTime.dwLowDateTime);
		ConvertDate(wfd.ftLastAccessTime,D,T,8,FALSE,FALSE,TRUE);
		fwprintf(fp,L"%*s %s  ftLastAccessTime      =0x%08X 0x%08X\n",12,L"",space,wfd.ftLastAccessTime.dwHighDateTime,wfd.ftLastAccessTime.dwLowDateTime);
		ConvertDate(wfd.ftLastWriteTime,D,T,8,FALSE,FALSE,TRUE);
		fwprintf(fp,L"%*s %s  ftLastWriteTime       =0x%08X 0x%08X\n",12,L"",space,wfd.ftLastWriteTime.dwHighDateTime,wfd.ftLastWriteTime.dwLowDateTime);
		LARGE_INTEGER Number = {wfd.nFileSizeLow, wfd.nFileSizeHigh};
		fwprintf(fp,L"%*s %s  nFileSize             =0x%08X, 0x%08X (%I64u)\n",12,L"",space,wfd.nFileSizeHigh,wfd.nFileSizeLow,static_cast<UINT64>(Number.QuadPart));
		fwprintf(fp,L"%*s %s  dwReserved0           =0x%08X (%u)\n",12,L"",space,wfd.dwReserved0,wfd.dwReserved0);

		if (wfd.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
		{
			if (wfd.dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT)
				fwprintf(fp,L"%*s %s     IO_REPARSE_TAG_MOUNT_POINT (0xA0000003L)\n",12,L"",space);

			if (wfd.dwReserved0 == IO_REPARSE_TAG_HSM)
				fwprintf(fp,L"%*s %s     IO_REPARSE_TAG_HSM         (0xC0000004L)\n",12,L"",space);

			if (wfd.dwReserved0 == IO_REPARSE_TAG_SIS)
				fwprintf(fp,L"%*s %s     IO_REPARSE_TAG_SIS         (0x80000007L)\n",12,L"",space);

			if (wfd.dwReserved0 == IO_REPARSE_TAG_DFS)
				fwprintf(fp,L"%*s %s     IO_REPARSE_TAG_DFS         (0x8000000AL)\n",12,L"",space);

			if (wfd.dwReserved0 == IO_REPARSE_TAG_SYMLINK)
				fwprintf(fp,L"%*s %s     IO_REPARSE_TAG_SYMLINK     (0xA000000CL)\n",12,L"",space);

			if (wfd.dwReserved0 == IO_REPARSE_TAG_DFSR)
				fwprintf(fp,L"%*s %s     IO_REPARSE_TAG_DFSR        (0x80000012L)\n",12,L"",space);
		}

		fwprintf(fp,L"%*s %s  dwReserved1           =0x%08X (%u)\n",12,L"",space,wfd.dwReserved1,wfd.dwReserved1);
		fwprintf(fp,L"%*s %s  cFileName             =\"%s\"\n",12,L"",space,wfd.cFileName);
		fwprintf(fp,L"%*s %s  cAlternateFileName    =\"%s\"\n",12,L"",space,wfd.cAlternateFileName);
		fwprintf(fp,L"%*s %s  }\n",12,L"",space);
		fflush(fp);
	}

	if (InternalLog)
		CloseSysLog();

#endif
}

void PanelViewSettings_Dump(const wchar_t *Title,const PanelViewSettings &ViewSettings,FILE *fp)
{
#if defined(SYSLOG)

	if (!IsLogON())
		return;

	int InternalLog=fp?FALSE:TRUE;
	const wchar_t *space=MakeSpace();

	if (InternalLog)
	{
		OpenSysLog();
		fp=PrintBaner(fp,L"PanelViewSettings",Title);
	}

	if (fp)
	{
		int I;
		fwprintf(fp,L"%*s %s  PanelViewSettings{\n",12,L"",space);
		fwprintf(fp,L"%*s %s  ColumnType           = [",12,L"",space);

		for (I=0; I < ARRAYSIZE(ViewSettings.ColumnType)-1; ++I)
			fwprintf(fp,L"%I64u, ",ViewSettings.ColumnType[I]);

		fwprintf(fp,L"%I64u]\n",ViewSettings.ColumnType[I]);
		fwprintf(fp,L"%*s %s  ColumnWidth          = [",12,L"",space);

		for (I=0; I < ARRAYSIZE(ViewSettings.ColumnWidth)-1; ++I)
			fwprintf(fp,L"%d, ",ViewSettings.ColumnWidth[I]);

		fwprintf(fp,L"%d]\n",ViewSettings.ColumnWidth[I]);
		fwprintf(fp,L"%*s %s  ColumnCount          = %d\n",12,L"",space,ViewSettings.ColumnCount);
		fwprintf(fp,L"%*s %s  StatusColumnType     = [",12,L"",space);

		for (I=0; I < ARRAYSIZE(ViewSettings.StatusColumnType)-1; ++I)
			fwprintf(fp,L"%08I64X, ",ViewSettings.StatusColumnType[I]);

		fwprintf(fp,L"%08I64X]\n",ViewSettings.StatusColumnType[I]);
		fwprintf(fp,L"%*s %s  StatusColumnWidth    = [",12,L"",space);

		for (I=0; I < ARRAYSIZE(ViewSettings.StatusColumnWidth)-1; ++I)
			fwprintf(fp,L"%d, ",ViewSettings.StatusColumnWidth[I]);

		fwprintf(fp,L"%d]\n",ViewSettings.StatusColumnWidth[I]);
		fwprintf(fp,L"%*s %s  StatusColumnCount    = %d\n",12,L"",space,ViewSettings.StatusColumnCount);
		fwprintf(fp,L"%*s %s  FullScreen           = %d\n",12,L"",space,(ViewSettings.Flags&PVS_FULLSCREEN)?1:0);
		fwprintf(fp,L"%*s %s  AlignExtensions      = %d\n",12,L"",space,(ViewSettings.Flags&PVS_ALIGNEXTENSIONS)?1:0);
		fwprintf(fp,L"%*s %s  FolderAlignExtensions= %d\n",12,L"",space,(ViewSettings.Flags&PVS_FOLDERALIGNEXTENSIONS)?1:0);
		fwprintf(fp,L"%*s %s  FolderUpperCase      = %d\n",12,L"",space,(ViewSettings.Flags&PVS_FOLDERUPPERCASE)?1:0);
		fwprintf(fp,L"%*s %s  FileLowerCase        = %d\n",12,L"",space,(ViewSettings.Flags&PVS_FILELOWERCASE)?1:0);
		fwprintf(fp,L"%*s %s  FileUpperToLowerCase = %d\n",12,L"",space,(ViewSettings.Flags&PVS_FILEUPPERTOLOWERCASE)?1:0);
		fwprintf(fp,L"%*s %s  }\n",12,L"",space);
		fflush(fp);
	}

	if (InternalLog)
		CloseSysLog();

#endif
}
