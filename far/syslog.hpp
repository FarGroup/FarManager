#ifndef __SYSLOG_HPP__
#define __SYSLOG_HPP__

/*
syslog.hpp

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



void SysLog(int l);
void SysLog(const wchar_t *fmt,...);
void SysLog(int l,const wchar_t *fmt,...); ///
void SysLogLastError();
void ShowHeap();
void CheckHeap(int NumLine);

string __FARKEY_ToName(int Key);
#define _FARKEY_ToName(K) (const wchar_t*)__FARKEY_ToName(K)
string __MCODE_ToName(int OpCode);
#define _MCODE_ToName(K) (const wchar_t*)__MCODE_ToName(K)
string __VK_KEY_ToName(int VkKey);
#define _VK_KEY_ToName(K) (const wchar_t*)__VK_KEY_ToName(K)
string __ECTL_ToName(int Command);
#define _ECTL_ToName(K) (const wchar_t*)__ECTL_ToName(K)
string __EE_ToName(int Command);
#define _EE_ToName(K) (const wchar_t*)__EE_ToName(K)
string __EEREDRAW_ToName(int Command);
#define _EEREDRAW_ToName(K) (const wchar_t*)__EEREDRAW_ToName(K)
string __ESPT_ToName(int Command);
#define _ESPT_ToName(K) (const wchar_t*)__ESPT_ToName(K)
string __VE_ToName(int Command);
#define _VE_ToName(K) (const wchar_t*)__VE_ToName(K)
string __FCTL_ToName(int Command);
#define _FCTL_ToName(K) (const wchar_t*)__FCTL_ToName(K)
string __DLGMSG_ToName(int Msg);
#define _DLGMSG_ToName(K) (const wchar_t*)__DLGMSG_ToName(K)
string __ACTL_ToName(int Command);
#define _ACTL_ToName(K) (const wchar_t*)__ACTL_ToName(K)
string __VCTL_ToName(int Command);
#define _VCTL_ToName(K) (const wchar_t*)__VCTL_ToName(K)
string __INPUT_RECORD_Dump(INPUT_RECORD *Rec);
#define _INPUT_RECORD_Dump(K) (const wchar_t*)__INPUT_RECORD_Dump(K)
string __MOUSE_EVENT_RECORD_Dump(MOUSE_EVENT_RECORD *Rec);
#define _MOUSE_EVENT_RECORD_Dump(K) (const wchar_t*)__MOUSE_EVENT_RECORD_Dump(K)
string __SysLog_LinearDump(LPBYTE Buf,int SizeBuf);
#define _SysLog_LinearDump(B,S) (const wchar_t*)__SysLog_LinearDump((B),(S))

void GetOpenPluginInfo_Dump(const wchar_t *Title,const struct OpenPluginInfo *Info,FILE *fp);
void INPUT_RECORD_DumpBuffer(FILE *fp=NULL);
void PanelViewSettings_Dump(const wchar_t *Title,const struct PanelViewSettings &ViewSettings,FILE *fp=NULL);
void PluginsStackItem_Dump(const wchar_t *Title,const struct PluginsStackItem *StackItems,int ItemNumber,FILE *fp=NULL);
void SaveScreenDumpBuffer(const wchar_t *Title,const CHAR_INFO *Buffer,int X1,int Y1,int X2,int Y2,int RealScreen,FILE *fp=NULL);
class Manager;
void ManagerClass_Dump(const wchar_t *Title,const Manager *m=NULL,FILE *fp=NULL);
void GetVolumeInformation_Dump(const wchar_t *Title,LPCWSTR lpRootPathName,LPCWSTR lpVolumeNameBuffer,DWORD nVolumeNameSize,
                                           DWORD lpVolumeSerialNumber, DWORD lpMaximumComponentLength, DWORD lpFileSystemFlags,
                                           LPCWSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize,FILE *fp=NULL);

void WIN32_FIND_DATA_Dump(const wchar_t *Title,const WIN32_FIND_DATA &fd,FILE *fp=NULL);

#if defined(SYSLOG_FARSYSLOG)
#ifdef __cplusplus
extern "C" {
#endif
void WINAPIV _export FarSysLog(const wchar_t *ModuleName,int Level,char *fmt,...);
void WINAPI  _export FarSysLogDump(const wchar_t *ModuleName,DWORD StartAddress,LPBYTE Buf,int SizeBuf);
void WINAPI _export FarSysLog_INPUT_RECORD_Dump(const wchar_t *ModuleName,INPUT_RECORD *rec);
#ifdef __cplusplus
};
#endif
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

void OpenSysLog();
void CloseSysLog();

struct TUserLog
{
    FILE *Stream;
    int   Level;
};

void SysLogDump(const wchar_t *Title,DWORD StartAddress,LPBYTE Buf,int SizeBuf,FILE *fp=NULL);

FILE *OpenLogStream(const wchar_t *file);

#define L_ERR      1
#define L_WARNING  2
#define L_INFO     3
#define L_DEBUG1   4
#define L_DEBUG2   5
#define L_DEBUG3   6

class CleverSysLog{ // ;-)
  public:
    CleverSysLog(const wchar_t *Title=NULL);
    ~CleverSysLog();
};


#define MAX_ARG_LEN   4096
#define MAX_LOG_LINE 10240

#define MAX_FILE 260

#endif // __SYSLOG_HPP__
