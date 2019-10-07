#ifndef SYSLOG_HPP_FCA31EB6_90F2_4E46_95C0_C037BA00B624
#define SYSLOG_HPP_FCA31EB6_90F2_4E46_95C0_C037BA00B624
#pragma once

/*
syslog.hpp

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

// Internal:

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

struct FAR_CHAR_INFO;
struct OpenPanelInfo;
void SysLog(int l);
void SysLog(const wchar_t *fmt,...);
void SysLog(int l,const wchar_t *fmt,...); ///
void SysLogLastError();
void ShowHeap();
void CheckHeap(int NumLine);

string __FARKEY_ToName(int Key);
#define _FARKEY_ToName(K) __FARKEY_ToName(K).c_str()
string __MCODE_ToName(DWORD OpCode);
#define _MCODE_ToName(K) __MCODE_ToName(K).c_str()
string __VK_KEY_ToName(int VkKey);
#define _VK_KEY_ToName(K) __VK_KEY_ToName(K).c_str()
string __ECTL_ToName(int Command);
#define _ECTL_ToName(K) __ECTL_ToName(K).c_str()
string __EE_ToName(int Command);
#define _EE_ToName(K) __EE_ToName(K).c_str()
string __ESPT_ToName(int Command);
#define _ESPT_ToName(K) __ESPT_ToName(K).c_str()
string __VE_ToName(int Command);
#define _VE_ToName(K) __VE_ToName(K).c_str()
string __FCTL_ToName(int Command);
#define _FCTL_ToName(K) __FCTL_ToName(K).c_str()
string __MCTL_ToName(int Command);
#define _MCTL_ToName(K) __MCTL_ToName(K).c_str()
string __DLGMSG_ToName(DWORD Msg);
#define _DLGMSG_ToName(K) __DLGMSG_ToName(K).c_str()
string __DLGDIF_ToName(DWORD Msg);
#define _DLGDIF_ToName(K) __DLGDIF_ToName(K).c_str()

string __ACTL_ToName(int Command);
#define _ACTL_ToName(K) __ACTL_ToName(K).c_str()
string __VCTL_ToName(int Command);
#define _VCTL_ToName(K) __VCTL_ToName(K).c_str()
string __INPUT_RECORD_Dump(const INPUT_RECORD *Rec);
#define _INPUT_RECORD_Dump(K) __INPUT_RECORD_Dump(K).c_str()
string __MOUSE_EVENT_RECORD_Dump(const MOUSE_EVENT_RECORD *Rec);
#define _MOUSE_EVENT_RECORD_Dump(K) __MOUSE_EVENT_RECORD_Dump(K).c_str()
string __SysLog_LinearDump(LPBYTE Buf,int SizeBuf);
#define _SysLog_LinearDump(B,S) __SysLog_LinearDump((B),(S)).c_str()

void GetOpenPanelInfo_Dump(const wchar_t *Title,const OpenPanelInfo *Info,FILE *fp);
void INPUT_RECORD_DumpBuffer(FILE *fp=nullptr);
//void PluginsStackItem_Dump(const wchar_t *Title,const struct PluginsStackItem *StackItems,int ItemNumber,FILE *fp=nullptr);
void SaveScreenDumpBuffer(const wchar_t *Title,const FAR_CHAR_INFO *Buffer,int X1,int Y1,int X2,int Y2,FILE *fp=nullptr);
class Manager;
void ManagerClass_Dump(const wchar_t *Title,FILE *fp=nullptr);
void GetVolumeInformation_Dump(const wchar_t *Title,LPCWSTR lpRootPathName,LPCWSTR lpVolumeNameBuffer,DWORD nVolumeNameSize,
                               DWORD lpVolumeSerialNumber, DWORD lpMaximumComponentLength, DWORD lpFileSystemFlags,
                               LPCWSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize,FILE *fp=nullptr);

void WIN32_FIND_DATA_Dump(const wchar_t *Title,const WIN32_FIND_DATA &fd,FILE *fp=nullptr);

#if defined(SYSLOG_FARSYSLOG)
extern "C"
{
	void WINAPIV FarSysLog(const wchar_t *ModuleName, int Level, const wchar_t *fmt, ...);
	void WINAPI FarSysLogDump(const wchar_t *ModuleName,DWORD StartAddress,LPBYTE Buf,int SizeBuf);
	void WINAPI FarSysLog_INPUT_RECORD_Dump(const wchar_t *ModuleName, const INPUT_RECORD *rec);
}
#endif

#if defined(_DEBUG) && defined(SYSLOG)
#define ___FILEFUNCLINE___  SysLog(L"[{%s} %s() #%d] ",__FILE__,__FUNCTION__,__LINE__)
#else
#define ___FILEFUNCLINE___
#endif

#if defined(_DEBUG) && defined(SYSLOG)
#define _D(x)  x
#else
#define _D(x)
#endif

// для "алгоритмов работы" - внимание! лог будет большим!
#if defined(_DEBUG) && defined(SYSLOG_ALGO)
#define _ALGO(x)  x
#else
#define _ALGO(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_DIALOG)
#define _DIALOG(x)  x
#else
#define _DIALOG(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_MANAGER)
#define _MANAGER(x)  x
#else
#define _MANAGER(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_KEYMACRO)
#define _KEYMACRO(x)  x
#else
#define _KEYMACRO(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_KEYMACRO_PARSE)
#define _KEYMACRO_PARSE(x)  x
#else
#define _KEYMACRO_PARSE(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_ECTL)
#define _ECTLLOG(x)  x
#else
#define _ECTLLOG(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_EE_REDRAW)
#define _SYS_EE_REDRAW(x)  x
#else
#define _SYS_EE_REDRAW(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_FCTL)
#define _FCTLLOG(x)  x
#else
#define _FCTLLOG(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_ACTL)
#define _ACTLLOG(x)  x
#else
#define _ACTLLOG(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_MCTL)
#define _MCTLLOG(x)  x
#else
#define _MCTLLOG(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_VCTL)
#define _VCTLLOG(x)  x
#else
#define _VCTLLOG(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_OT)
#define _OT(x)  x
#else
#define _OT(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_SVS)
#define _SVS(x)  x
#else
#define _SVS(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_DJ)
#define _DJ(x)  x
#else
#define _DJ(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_WARP)
#define _WARP(x)  x
#else
#define _WARP(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_VVM)
#define _VVM(x)  x
#else
#define _VVM(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_IS)
#define _IS(x)  x
#else
#define _IS(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_AT)
#define _AT(x)  x
#else
#define _AT(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_tran)
#define _tran(x)  x
#else
#define _tran(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_SKV)
#define _SKV(x)  x
#else
#define _SKV(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_KM)
#define _KM(x)  x
#else
#define _KM(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_NWZ)
#define _NWZ(x)  x
#else
#define _NWZ(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_COPYR)
#define _LOGCOPYR(x)  x
#else
#define _LOGCOPYR(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_TREX)
#define _TREX(x)  x
#else
#define _TREX(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_YJH)
#define _YJH(x)  x
#else
#define _YJH(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_SHMUEL)
#define _SHMUEL(x)  x
#else
#define _SHMUEL(x)
#endif


void OpenSysLog();
void CloseSysLog();

struct TUserLog
{
	FILE *Stream;
	int   Level;
};

void SysLogDump(const wchar_t *Title,DWORD StartAddress,LPBYTE Buf,unsigned SizeBuf,FILE *fp=nullptr);

FILE *OpenLogStream(const string& file);

#define L_ERR      1
#define L_WARNING  2
#define L_INFO     3
#define L_DEBUG1   4
#define L_DEBUG2   5
#define L_DEBUG3   6

class CleverSysLog  // ;-)
{
	public:
	explicit CleverSysLog(const wchar_t *Title=nullptr);
		CleverSysLog(int Line,const wchar_t *Title);
		~CleverSysLog();
};

void PrintSysLogStat();

#endif // SYSLOG_HPP_FCA31EB6_90F2_4E46_95C0_C037BA00B624
