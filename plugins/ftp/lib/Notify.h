#ifndef __FTP_PLUGIN_NOTIFY
#define __FTP_PLUGIN_NOTIFY

//------------------------------------------------------------------------
/*
  IO notify
*/

#define FTP_NOTIFY_MAGIC  MK_ID( 'F','n','t','f' )

struct FTNNotify
{
	__int64           RestartPoint;
	BOOL            Upload;
	BOOL            Starting;
	BOOL            Success;
	char            HostName[MAX_PATH];
	char            User[MAX_PATH];
	char            Password[MAX_PATH];
	WORD            Port;
	char            LocalFile[MAX_PATH];
	char            RemoteFile[MAX_PATH];
};

struct NotifyInterface: public FTPPluginInterface
{
	void (WINAPI *Notify)(const FTNNotify* p);
};

#endif
