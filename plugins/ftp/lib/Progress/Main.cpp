#include <all_far.h>
#pragma hdrstop

#include "Int.h"

#define CH_OBJECT Assert( Object );

void WINAPI Pg_ResumeFile(HANDLE Object, LPCSTR LocalFileName)                                   { CH_OBJECT((TrafficInformation*)Object)->Resume(LocalFileName); }
void WINAPI Pg_Resume(HANDLE Object, __int64 size)                                               { CH_OBJECT((TrafficInformation*)Object)->Resume(size); }
BOOL WINAPI Pg_Callback(HANDLE Object, int Size)                                                 { CH_OBJECT return ((TrafficInformation*)Object)->Callback(Size); }
void WINAPI Pg_Init(HANDLE Object, HANDLE h,int tMsg,int OpMode,FP_SizeItemList* il)             { CH_OBJECT((TrafficInformation*)Object)->Init(h,tMsg,OpMode,il); }
void WINAPI Pg_InitFile(HANDLE Object, __int64 sz, LPCSTR SrcName, LPCSTR DestName)            { CH_OBJECT((TrafficInformation*)Object)->InitFile(sz,SrcName,DestName); }
void WINAPI Pg_Skip(HANDLE Object)                                                           { CH_OBJECT((TrafficInformation*)Object)->Skip(); }
void WINAPI Pg_Waiting(HANDLE Object, time_t paused)                                            { CH_OBJECT((TrafficInformation*)Object)->Waiting(paused); }
void WINAPI Pg_SetConn(HANDLE Object, HANDLE Connection)                                        { CH_OBJECT((TrafficInformation*)Object)->SetConnection(Connection); }

HANDLE WINAPI Pg_CreateObject(void)
{
	return new TrafficInformation;
}

void WINAPI Pg_DestroyObject(HANDLE Object)
{
	delete((TrafficInformation*)Object);
}

FTPPluginInterface* WINAPI FTPPluginGetInterface(void)
{
	static ProgressInterface Interface;
	Interface.Magic         = FTP_PROGRESS_MAGIC;
	Interface.CreateObject  = Pg_CreateObject;
	Interface.DestroyObject = Pg_DestroyObject;
	Interface.ResumeFile    = Pg_ResumeFile;
	Interface.Resume        = Pg_Resume;
	Interface.Callback      = Pg_Callback;
	Interface.Init          = Pg_Init;
	Interface.InitFile      = Pg_InitFile;
	Interface.Skip          = Pg_Skip;
	Interface.Waiting       = Pg_Waiting;
	Interface.SetConnection = Pg_SetConn;
	return &Interface;
}

BOOL WINAPI FTP_PluginStartup(DWORD Reason)
{
	return TRUE;
}
