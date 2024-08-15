#ifndef SDK_GCC_H_EA5F3F70_C987_4240_AA25_3016DB39C651
#define SDK_GCC_H_EA5F3F70_C987_4240_AA25_3016DB39C651
#pragma once

/*
sdk_gcc.h

Типы и определения, отсутствующие в SDK (GCC).
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

#pragma GCC system_header

// winnt.h
#ifndef IO_REPARSE_TAG_FILE_PLACEHOLDER
#define IO_REPARSE_TAG_FILE_PLACEHOLDER 0x80000015L
#endif

#ifndef IO_REPARSE_TAG_WOF
#define IO_REPARSE_TAG_WOF 0x80000017L
#endif

#ifndef IO_REPARSE_TAG_WCI
#define IO_REPARSE_TAG_WCI 0x80000018L
#endif

#ifndef IO_REPARSE_TAG_WCI_1
#define IO_REPARSE_TAG_WCI_1 0x90001018L
#endif

#ifndef IO_REPARSE_TAG_GLOBAL_REPARSE
#define IO_REPARSE_TAG_GLOBAL_REPARSE 0xA0000019L
#endif

#ifndef IO_REPARSE_TAG_CLOUD
#define IO_REPARSE_TAG_CLOUD 0x9000001AL
#endif

#ifndef IO_REPARSE_TAG_CLOUD_1
#define IO_REPARSE_TAG_CLOUD_1 0x9000101AL
#endif

#ifndef IO_REPARSE_TAG_CLOUD_2
#define IO_REPARSE_TAG_CLOUD_2 0x9000201AL
#endif

#ifndef IO_REPARSE_TAG_CLOUD_3
#define IO_REPARSE_TAG_CLOUD_3 0x9000301AL
#endif

#ifndef IO_REPARSE_TAG_CLOUD_4
#define IO_REPARSE_TAG_CLOUD_4 0x9000401AL
#endif

#ifndef IO_REPARSE_TAG_CLOUD_5
#define IO_REPARSE_TAG_CLOUD_5 0x9000501AL
#endif

#ifndef IO_REPARSE_TAG_CLOUD_6
#define IO_REPARSE_TAG_CLOUD_6 0x9000601AL
#endif

#ifndef IO_REPARSE_TAG_CLOUD_7
#define IO_REPARSE_TAG_CLOUD_7 0x9000701AL
#endif

#ifndef IO_REPARSE_TAG_CLOUD_8
#define IO_REPARSE_TAG_CLOUD_8 0x9000801AL
#endif

#ifndef IO_REPARSE_TAG_CLOUD_9
#define IO_REPARSE_TAG_CLOUD_9 0x9000901AL
#endif

#ifndef IO_REPARSE_TAG_CLOUD_A
#define IO_REPARSE_TAG_CLOUD_A 0x9000A01AL
#endif

#ifndef IO_REPARSE_TAG_CLOUD_B
#define IO_REPARSE_TAG_CLOUD_B 0x9000B01AL
#endif

#ifndef IO_REPARSE_TAG_CLOUD_C
#define IO_REPARSE_TAG_CLOUD_C 0x9000C01AL
#endif

#ifndef IO_REPARSE_TAG_CLOUD_D
#define IO_REPARSE_TAG_CLOUD_D 0x9000D01AL
#endif

#ifndef IO_REPARSE_TAG_CLOUD_E
#define IO_REPARSE_TAG_CLOUD_E 0x9000E01AL
#endif

#ifndef IO_REPARSE_TAG_CLOUD_F
#define IO_REPARSE_TAG_CLOUD_F 0x9000F01AL
#endif

#ifndef IO_REPARSE_TAG_APPEXECLINK
#define IO_REPARSE_TAG_APPEXECLINK 0x8000001BL
#endif

#ifndef FILE_ATTRIBUTE_INTEGRITY_STREAM
#define FILE_ATTRIBUTE_INTEGRITY_STREAM 0x8000
#endif

#ifndef FILE_ATTRIBUTE_NO_SCRUB_DATA
#define FILE_ATTRIBUTE_NO_SCRUB_DATA 0x20000
#endif

#ifndef FILE_ATTRIBUTE_PINNED
#define FILE_ATTRIBUTE_PINNED 0x80000
#endif

#ifndef FILE_ATTRIBUTE_UNPINNED
#define FILE_ATTRIBUTE_UNPINNED 0x100000
#endif

#ifndef FILE_ATTRIBUTE_RECALL_ON_OPEN
#define FILE_ATTRIBUTE_RECALL_ON_OPEN 0x00040000
#endif

#ifndef FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS
#define FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS 0x00400000
#endif

#ifndef FILE_ATTRIBUTE_STRICTLY_SEQUENTIAL
#define FILE_ATTRIBUTE_STRICTLY_SEQUENTIAL 0x20000000
#endif

// winerror.h
#ifndef ERROR_STOPPED_ON_SYMLINK
#define ERROR_STOPPED_ON_SYMLINK 681L
#endif

// wincon.h
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#ifndef ENABLE_LVB_GRID_WORLDWIDE
#define ENABLE_LVB_GRID_WORLDWIDE 0x0010
#endif

#ifndef ENABLE_VIRTUAL_TERMINAL_INPUT
#define ENABLE_VIRTUAL_TERMINAL_INPUT 0x0200
#endif

//winerror.h
#ifndef ERROR_UNHANDLED_EXCEPTION
#define ERROR_UNHANDLED_EXCEPTION 574L
#endif

#ifndef FACILITY_VISUALCPP
#define FACILITY_VISUALCPP 109
#endif

// winternl.h
#ifndef FILE_VALID_SET_FLAGS
typedef struct _SYSTEM_THREAD_INFORMATION
{
	LARGE_INTEGER Reserved1[3];
	ULONG Reserved2;
	PVOID StartAddress;
	CLIENT_ID ClientId;
	KPRIORITY Priority;
	LONG BasePriority;
	ULONG Reserved3;
	ULONG ThreadState;
	ULONG WaitReason;
}
SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;
#endif

// dbgeng.h
#ifndef DEBUG_DISASM_SOURCE_LINE_NUMBER
#define DEBUG_DISASM_SOURCE_LINE_NUMBER 0x00000004
#define DEBUG_DISASM_SOURCE_FILE_NAME   0x00000008

// This particular name is unrelated, it was just added soon after IID_IDebugClient5
// https://sourceforge.net/p/mingw-w64/mingw-w64/ci/aa3060039963b6e0c2950dd78b3db66faf8d79f8/
// https://sourceforge.net/p/mingw-w64/mingw-w64/ci/c37b1cd2058f25b02e8d7ffcf21db255e5cc3774/
#ifndef DBG_FRAME_DEFAULT

DEFINE_GUID(IID_IDebugClient5,0xe3acb9d7,0x7ec2,0x4f0c,0xa0,0xda,0xe8,0x1e,0x0c,0xbb,0xe6,0x28);
DEFINE_GUID(IID_IDebugOutputCallbacksWide,0x4c7fd663,0xc394,0x4e26,0x8e,0xf1,0x34,0xad,0x5e,0xd3,0x76,0x4c);

typedef struct IDebugClient5 *PDEBUG_CLIENT5;
typedef struct IDebugEventCallbacksWide *PDEBUG_EVENT_CALLBACKS_WIDE;
typedef struct IDebugOutputCallbacksWide *PDEBUG_OUTPUT_CALLBACKS_WIDE;

#undef INTERFACE
#define INTERFACE IDebugClient5
DECLARE_INTERFACE_(IDebugClient5, IUnknown)
{
    STDMETHOD(QueryInterface)(THIS_ REFIID InterfaceId,PVOID *Interface) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;
    STDMETHOD(AttachKernel)(THIS_ ULONG Flags,PCSTR ConnectOptions) PURE;
    STDMETHOD(GetKernelConnectionOptions)(THIS_ PSTR Buffer,ULONG BufferSize,PULONG OptionsSize) PURE;
    STDMETHOD(SetKernelConnectionOptions)(THIS_ PCSTR Options) PURE;
    STDMETHOD(StartProcessServer)(THIS_ ULONG Flags,PCSTR Options,PVOID Reserved) PURE;
    STDMETHOD(ConnectProcessServer)(THIS_ PCSTR RemoteOptions,PULONG64 Server) PURE;
    STDMETHOD(DisconnectProcessServer)(THIS_ ULONG64 Server) PURE;
    STDMETHOD(GetRunningProcessSystemIds)(THIS_ ULONG64 Server,PULONG Ids,ULONG Count,PULONG ActualCount) PURE;
    STDMETHOD(GetRunningProcessSystemIdByExecutableName)(THIS_ ULONG64 Server,PCSTR ExeName,ULONG Flags,PULONG Id) PURE;
    STDMETHOD(GetRunningProcessDescription)(THIS_ ULONG64 Server,ULONG SystemId,ULONG Flags,PSTR ExeName,ULONG ExeNameSize,PULONG ActualExeNameSize,PSTR Description,ULONG DescriptionSize,PULONG ActualDescriptionSize) PURE;
    STDMETHOD(AttachProcess)(THIS_ ULONG64 Server,ULONG ProcessId,ULONG AttachFlags) PURE;
    STDMETHOD(CreateProcess)(THIS_ ULONG64 Server,PSTR CommandLine,ULONG CreateFlags) PURE;
    STDMETHOD(CreateProcessAndAttach)(THIS_ ULONG64 Server,PSTR CommandLine,ULONG CreateFlags,ULONG ProcessId,ULONG AttachFlags) PURE;
    STDMETHOD(GetProcessOptions)(THIS_ PULONG Options) PURE;
    STDMETHOD(AddProcessOptions)(THIS_ ULONG Options) PURE;
    STDMETHOD(RemoveProcessOptions)(THIS_ ULONG Options) PURE;
    STDMETHOD(SetProcessOptions)(THIS_ ULONG Options) PURE;
    STDMETHOD(OpenDumpFile)(THIS_ PCSTR DumpFile) PURE;
    STDMETHOD(WriteDumpFile)(THIS_ PCSTR DumpFile,ULONG Qualifier) PURE;
    STDMETHOD(ConnectSession)(THIS_ ULONG Flags,ULONG HistoryLimit) PURE;
    STDMETHOD(StartServer)(THIS_ PCSTR Options) PURE;
    STDMETHOD(OutputServers)(THIS_ ULONG OutputControl,PCSTR Machine,ULONG Flags) PURE;
    STDMETHOD(TerminateProcesses)(THIS) PURE;
    STDMETHOD(DetachProcesses)(THIS) PURE;
    STDMETHOD(EndSession)(THIS_ ULONG Flags) PURE;
    STDMETHOD(GetExitCode)(THIS_ PULONG Code) PURE;
    STDMETHOD(DispatchCallbacks)(THIS_ ULONG Timeout) PURE;
    STDMETHOD(ExitDispatch)(THIS_ PDEBUG_CLIENT Client) PURE;
    STDMETHOD(CreateClient)(THIS_ PDEBUG_CLIENT *Client) PURE;
    STDMETHOD(GetInputCallbacks)(THIS_ PDEBUG_INPUT_CALLBACKS *Callbacks) PURE;
    STDMETHOD(SetInputCallbacks)(THIS_ PDEBUG_INPUT_CALLBACKS Callbacks) PURE;
    STDMETHOD(GetOutputCallbacks)(THIS_ PDEBUG_OUTPUT_CALLBACKS *Callbacks) PURE;
    STDMETHOD(SetOutputCallbacks)(THIS_ PDEBUG_OUTPUT_CALLBACKS Callbacks) PURE;
    STDMETHOD(GetOutputMask)(THIS_ PULONG Mask) PURE;
    STDMETHOD(SetOutputMask)(THIS_ ULONG Mask) PURE;
    STDMETHOD(GetOtherOutputMask)(THIS_ PDEBUG_CLIENT Client,PULONG Mask) PURE;
    STDMETHOD(SetOtherOutputMask)(THIS_ PDEBUG_CLIENT Client,ULONG Mask) PURE;
    STDMETHOD(GetOutputWidth)(THIS_ PULONG Columns) PURE;
    STDMETHOD(SetOutputWidth)(THIS_ ULONG Columns) PURE;
    STDMETHOD(GetOutputLinePrefix)(THIS_ PSTR Buffer,ULONG BufferSize,PULONG PrefixSize) PURE;
    STDMETHOD(SetOutputLinePrefix)(THIS_ PCSTR Prefix) PURE;
    STDMETHOD(GetIdentity)(THIS_ PSTR Buffer,ULONG BufferSize,PULONG IdentitySize) PURE;
    STDMETHOD(OutputIdentity)(THIS_ ULONG OutputControl,ULONG Flags,PCSTR Format) PURE;
    STDMETHOD(GetEventCallbacks)(THIS_ PDEBUG_EVENT_CALLBACKS *Callbacks) PURE;
    STDMETHOD(SetEventCallbacks)(THIS_ PDEBUG_EVENT_CALLBACKS Callbacks) PURE;
    STDMETHOD(FlushCallbacks)(THIS) PURE;
    STDMETHOD(WriteDumpFile2)(THIS_ PCSTR DumpFile,ULONG Qualifier,ULONG FormatFlags,PCSTR Comment) PURE;
    STDMETHOD(AddDumpInformationFile)(THIS_ PCSTR InfoFile,ULONG Type) PURE;
    STDMETHOD(EndProcessServer)(THIS_ ULONG64 Server) PURE;
    STDMETHOD(WaitForProcessServerEnd)(THIS_ ULONG Timeout) PURE;
    STDMETHOD(IsKernelDebuggerEnabled)(THIS) PURE;
    STDMETHOD(TerminateCurrentProcess)(THIS) PURE;
    STDMETHOD(DetachCurrentProcess)(THIS) PURE;
    STDMETHOD(AbandonCurrentProcess)(THIS) PURE;
    STDMETHOD(GetRunningProcessSystemIdByExecutableNameWide)(THIS_ ULONG64 Server,PCWSTR ExeName,ULONG Flags,PULONG Id) PURE;
    STDMETHOD(GetRunningProcessDescriptionWide)(THIS_ ULONG64 Server,ULONG SystemId,ULONG Flags,PWSTR ExeName,ULONG ExeNameSize,PULONG ActualExeNameSize,PWSTR Description,ULONG DescriptionSize,PULONG ActualDescriptionSize) PURE;
    STDMETHOD(CreateProcessWide)(THIS_ ULONG64 Server,PWSTR CommandLine,ULONG CreateFlags) PURE;
    STDMETHOD(CreateProcessAndAttachWide)(THIS_ ULONG64 Server,PWSTR CommandLine,ULONG CreateFlags,ULONG ProcessId,ULONG AttachFlags) PURE;
    STDMETHOD(OpenDumpFileWide)(THIS_ PCWSTR FileName,ULONG64 FileHandle) PURE;
    STDMETHOD(WriteDumpFileWide)(THIS_ PCWSTR FileName,ULONG64 FileHandle,ULONG Qualifier,ULONG FormatFlags,PCWSTR Comment) PURE;
    STDMETHOD(AddDumpInformationFileWide)(THIS_ PCWSTR FileName,ULONG64 FileHandle,ULONG Type) PURE;
    STDMETHOD(GetNumberDumpFiles)(THIS_ PULONG Number) PURE;
    STDMETHOD(GetDumpFile)(THIS_ ULONG Index,PSTR Buffer,ULONG BufferSize,PULONG NameSize,PULONG64 Handle,PULONG Type) PURE;
    STDMETHOD(GetDumpFileWide)(THIS_ ULONG Index,PWSTR Buffer,ULONG BufferSize,PULONG NameSize,PULONG64 Handle,PULONG Type) PURE;
    STDMETHOD(AttachKernelWide)(THIS_ ULONG Flags,PCWSTR ConnectOptions) PURE;
    STDMETHOD(GetKernelConnectionOptionsWide)(THIS_ PWSTR Buffer,ULONG BufferSize,PULONG OptionsSize) PURE;
    STDMETHOD(SetKernelConnectionOptionsWide)(THIS_ PCWSTR Options) PURE;
    STDMETHOD(StartProcessServerWide)(THIS_ ULONG Flags,PCWSTR Options,PVOID Reserved) PURE;
    STDMETHOD(ConnectProcessServerWide)(THIS_ PCWSTR RemoteOptions,PULONG64 Server) PURE;
    STDMETHOD(StartServerWide)(THIS_ PCWSTR Options) PURE;
    STDMETHOD(OutputServersWide)(THIS_ ULONG OutputControl,PCWSTR Machine,ULONG Flags) PURE;
    STDMETHOD(GetOutputCallbacksWide)(THIS_ PDEBUG_OUTPUT_CALLBACKS_WIDE *Callbacks) PURE;
    STDMETHOD(SetOutputCallbacksWide)(THIS_ PDEBUG_OUTPUT_CALLBACKS_WIDE Callbacks) PURE;
    STDMETHOD(GetOutputLinePrefixWide)(THIS_ PWSTR Buffer,ULONG BufferSize,PULONG PrefixSize) PURE;
    STDMETHOD(SetOutputLinePrefixWide)(THIS_ PCWSTR Prefix) PURE;
    STDMETHOD(GetIdentityWide)(THIS_ PWSTR Buffer,ULONG BufferSize,PULONG IdentitySize) PURE;
    STDMETHOD(OutputIdentityWide)(THIS_ ULONG OutputControl,ULONG Flags,PCWSTR Format) PURE;
    STDMETHOD(GetEventCallbacksWide)(THIS_ PDEBUG_EVENT_CALLBACKS_WIDE *Callbacks) PURE;
    STDMETHOD(SetEventCallbacksWide)(THIS_ PDEBUG_EVENT_CALLBACKS_WIDE Callbacks) PURE;
    STDMETHOD(CreateProcess2)(THIS_ ULONG64 Server,PSTR CommandLine,PVOID OptionsBuffer,ULONG OptionsBufferSize,PCSTR InitialDirectory,PCSTR Environment) PURE;
    STDMETHOD(CreateProcess2Wide)(THIS_ ULONG64 Server,PWSTR CommandLine,PVOID OptionsBuffer,ULONG OptionsBufferSize,PCWSTR InitialDirectory,PCWSTR Environment) PURE;
    STDMETHOD(CreateProcessAndAttach2)(THIS_ ULONG64 Server,PSTR CommandLine,PVOID OptionsBuffer,ULONG OptionsBufferSize,PCSTR InitialDirectory,PCSTR Environment,ULONG ProcessId,ULONG AttachFlags) PURE;
    STDMETHOD(CreateProcessAndAttach2Wide)(THIS_ ULONG64 Server,PWSTR CommandLine,PVOID OptionsBuffer,ULONG OptionsBufferSize,PCWSTR InitialDirectory,PCWSTR Environment,ULONG ProcessId,ULONG AttachFlags) PURE;
    STDMETHOD(PushOutputLinePrefix)(THIS_ PCSTR NewPrefix,PULONG64 Handle) PURE;
    STDMETHOD(PushOutputLinePrefixWide)(THIS_ PCWSTR NewPrefix,PULONG64 Handle) PURE;
    STDMETHOD(PopOutputLinePrefix)(THIS_ ULONG64 Handle) PURE;
    STDMETHOD(GetNumberInputCallbacks)(THIS_ PULONG Count) PURE;
    STDMETHOD(GetNumberOutputCallbacks)(THIS_ PULONG Count) PURE;
    STDMETHOD(GetNumberEventCallbacks)(THIS_ ULONG EventFlags,PULONG Count) PURE;
    STDMETHOD(GetQuitLockString)(THIS_ PSTR Buffer,ULONG BufferSize,PULONG StringSize) PURE;
    STDMETHOD(SetQuitLockString)(THIS_ PCSTR String) PURE;
    STDMETHOD(GetQuitLockStringWide)(THIS_ PWSTR Buffer,ULONG BufferSize,PULONG StringSize) PURE;
    STDMETHOD(SetQuitLockStringWide)(THIS_ PCWSTR String) PURE;
};

#undef INTERFACE
#define INTERFACE IDebugOutputCallbacksWide
DECLARE_INTERFACE_(IDebugOutputCallbacksWide, IUnknown)
{
    STDMETHOD(QueryInterface)(THIS_ REFIID InterfaceId,PVOID *Interface) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
    STDMETHOD(Output)(THIS_ ULONG Mask,PCWSTR Text) PURE;
};

#endif

#endif // DBG_FRAME_DEFAULT

#ifndef STACK_FRAME_TYPE_INLINE
#define STACK_FRAME_TYPE_INLINE 0x02
#define STACK_FRAME_TYPE_IGNORE 0xFF

// dbghelp.h
#define SYM_STKWALK_DEFAULT 0x00000000

typedef union _INLINE_FRAME_CONTEXT
{
	DWORD ContextValue;
	struct
	{
		BYTE FrameId;
		BYTE FrameType;
		WORD FrameSignature;
	};
}
INLINE_FRAME_CONTEXT;
#endif

#ifndef INLINE_FRAME_CONTEXT_INIT
#define INLINE_FRAME_CONTEXT_INIT   0
#define INLINE_FRAME_CONTEXT_IGNORE 0xFFFFFFFF

typedef struct _tagSTACKFRAME_EX
{
	ADDRESS64 AddrPC;
	ADDRESS64 AddrReturn;
	ADDRESS64 AddrFrame;
	ADDRESS64 AddrStack;
	ADDRESS64 AddrBStore;
	PVOID FuncTableEntry;
	DWORD64 Params[4];
	WINBOOL Far;
	WINBOOL Virtual;
	DWORD64 Reserved[3];
	KDHELP64 KdHelp;
	DWORD StackFrameSize;
	DWORD InlineFrameContext;
}
STACKFRAME_EX,*LPSTACKFRAME_EX;
#endif

// libuuid.a
DEFINE_GUID(IID_IMultiLanguage2, 0xDCCFC164, 0x2B38, 0x11D2, 0xB7, 0xEC, 0x00, 0xC0, 0x4F, 0x8F, 0x5D, 0x9A);

#endif // SDK_GCC_H_EA5F3F70_C987_4240_AA25_3016DB39C651
