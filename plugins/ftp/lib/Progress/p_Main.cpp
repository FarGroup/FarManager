#include <all_far.h>
#pragma hdrstop

#include "p_Int.h"

#define CH_OBJECT Assert( Object );

void DECLSPEC Pg_ResumeFile( HANDLE Object, CONSTSTR LocalFileName )                                   { CH_OBJECT ((PTrafficInformation)Object)->Resume(LocalFileName); }
void DECLSPEC Pg_Resume    ( HANDLE Object, __int64 size )                                               { CH_OBJECT ((PTrafficInformation)Object)->Resume(size); }
BOOL DECLSPEC Pg_Callback  ( HANDLE Object, int Size )                                                 { CH_OBJECT return ((PTrafficInformation)Object)->Callback(Size); }
void DECLSPEC Pg_Init      ( HANDLE Object, HANDLE h,int tMsg,int OpMode,PFP_SizeItemList il )             { CH_OBJECT ((PTrafficInformation)Object)->Init(h,tMsg,OpMode,il); }
void DECLSPEC Pg_InitFile  ( HANDLE Object, __int64 sz, CONSTSTR SrcName, CONSTSTR DestName )            { CH_OBJECT ((PTrafficInformation)Object)->InitFile(sz,SrcName,DestName); }
void DECLSPEC Pg_Skip      ( HANDLE Object )                                                           { CH_OBJECT ((PTrafficInformation)Object)->Skip(); }
void DECLSPEC Pg_Waiting   ( HANDLE Object, time_t paused )                                            { CH_OBJECT ((PTrafficInformation)Object)->Waiting(paused); }
void DECLSPEC Pg_SetConn   ( HANDLE Object, HANDLE Connection )                                        { CH_OBJECT ((PTrafficInformation)Object)->SetConnection(Connection); }

HANDLE DECLSPEC Pg_CreateObject( void )
  {
  return new TrafficInformation;
}

void DECLSPEC Pg_DestroyObject( HANDLE Object )
  {
    delete ((PTrafficInformation)Object);
}

PFTPPluginInterface DECLSPEC FTPPluginGetInterface( void )
  {  static ProgressInterface Interface;

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

BOOL DECLSPEC FTP_PluginStartup( DWORD Reason )
  {
 return TRUE;
}
