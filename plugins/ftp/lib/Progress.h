#ifndef __FTP_PLUGIN_PROGRESS
#define __FTP_PLUGIN_PROGRESS

/*
 * TrafficInformation
 */
#define FTP_PROGRESS_MAGIC  MK_ID( 'F','P','g',1 )

typedef void (WINAPI *Pg_ResumeFile_t)(HANDLE Object, LPCSTR LocalFileName);
typedef void (WINAPI *Pg_Resume_t)(HANDLE Object, __int64 size);
typedef BOOL (WINAPI *Pg_Callback_t)(HANDLE Object, int Size);
typedef void (WINAPI *Pg_Init_t)(HANDLE Object, HANDLE Connection,int tMsg,int OpMode,FP_SizeItemList* il);
typedef void (WINAPI *Pg_InitFile_t)(HANDLE Object, __int64 sz, LPCSTR SrcName, LPCSTR DestName);
typedef void (WINAPI *Pg_Skip_t)(HANDLE Object);
typedef void (WINAPI *Pg_Waiting_t)(HANDLE Object, time_t paused);
typedef void (WINAPI *Pg_SetConn_t)(HANDLE Object,HANDLE Connection);

struct ProgressInterface: public FTPPluginInterface
{
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
