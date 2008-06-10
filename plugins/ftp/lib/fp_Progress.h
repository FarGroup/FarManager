#ifndef __FTP_PLUGIN_PROGRESS
#define __FTP_PLUGIN_PROGRESS

/*
 * TrafficInformation
 */
#define FTP_PROGRESS_MAGIC  MK_ID( 'F','P','g',1 )

typedef void (DECLSPEC *Pg_ResumeFile_t) ( HANDLE Object, CONSTSTR LocalFileName );
typedef void (DECLSPEC *Pg_Resume_t)     ( HANDLE Object, __int64 size );
typedef BOOL (DECLSPEC *Pg_Callback_t)   ( HANDLE Object, int Size );
typedef void (DECLSPEC *Pg_Init_t)       ( HANDLE Object, HANDLE Connection,int tMsg,int OpMode,PFP_SizeItemList il );
typedef void (DECLSPEC *Pg_InitFile_t)   ( HANDLE Object, __int64 sz, CONSTSTR SrcName, CONSTSTR DestName );
typedef void (DECLSPEC *Pg_Skip_t)       ( HANDLE Object );
typedef void (DECLSPEC *Pg_Waiting_t)    ( HANDLE Object, time_t paused );
typedef void (DECLSPEC *Pg_SetConn_t)    ( HANDLE Object,HANDLE Connection );

STRUCTBASE( ProgressInterface, public FTPPluginInterface )
  FTP_CreateObject_t    CreateObject;
  FTP_DestroyObject_t   DestroyObject;

  Pg_ResumeFile_t       ResumeFile;
  Pg_Resume_t           Resume;
  Pg_Callback_t         Callback;
  Pg_Init_t             Init;
  Pg_InitFile_t         InitFile;
  Pg_Skip_t             Skip;
  Pg_Waiting_t          Waiting;
  Pg_SetConn_t          SetConnection;
};

#endif
