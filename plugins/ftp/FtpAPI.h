#ifndef __FAR_PLUGIN_FTP_IEFTP
#define __FAR_PLUGIN_FTP_IEFTP

BOOL   FtpChmod(Connection *hConnect,LPCSTR lpszFileName,DWORD Mode);
int    FtpCmdBlock(Connection *hConnect,int block /*TRUE,FALSE,-1*/);
int    FtpConnectMessage(Connection *hConnect,int Msg,LPCSTR HostName,int BtnMsg = MNone__,int btn1 = MNone__, int btn2 = MNone__);
BOOL   FtpDeleteFile(Connection *hConnect,LPCSTR lpszFileName);
BOOL   FtpGetFtpDirectory(Connection *Connect);
BOOL   FtpFindFirstFile(Connection *hConnect,LPCSTR lpszSearchFile,FTPFileInfo* lpFindFileData,BOOL *ResetCache);
BOOL   FtpFindNextFile(Connection *hConnect,FTPFileInfo* lpFindFileData);
BOOL   FtpGetCurrentDirectory(Connection *hConnect, String& lpszCurrentDirectory);
BOOL   FtpGetFile(Connection *hConnect,LPCSTR lpszRemoteFile,LPCSTR lpszNewFile,BOOL Reget,int AsciiMode);
int    FtpGetRetryCount(Connection *hConnect);
BOOL   FtpIsResume(Connection *hConnect);
BOOL   FtpPutFile(Connection *hConnect,LPCSTR lpszLocalFile,LPCSTR lpszNewRemoteFile,BOOL Reput,int AsciiMode);
BOOL   FtpRemoveDirectory(Connection *hConnect,LPCSTR lpszDirectory);
BOOL   FtpRenameFile(Connection *hConnect,LPCSTR lpszExisting,LPCSTR lpszNew);
BOOL   FtpSetCurrentDirectory(Connection *hConnect,LPCSTR lpszDirectory);
void   FtpSetRetryCount(Connection *hConnect,int cn);
BOOL   FtpSetBreakable(Connection *hConnect,BOOL cn);
BOOL   FtpSystemInfo(Connection *hConnect,char *Buffer,int MaxSize);
BOOL   FtpKeepAlive(Connection *hConnect);
BOOL   FtpCmdLineAlive(Connection *hConnect);
__int64  FtpFileSize(Connection *Connect,LPCSTR fnm);

//------------------------------------------------------------------------
struct FTPConnectionBreakable
{
	Connection *hConnect;
	int         fBreakable;

	FTPConnectionBreakable(Connection *cn, int Breakable)
	{
		hConnect   = cn;
		fBreakable = FtpSetBreakable(hConnect,Breakable);
		Log(("ESC: FTPBreakable %d -> %d", fBreakable, Breakable));
	}
	~FTPConnectionBreakable()
	{
		FtpSetBreakable(hConnect,fBreakable);
		Log(("ESC: FTPBreakable restore to %d", fBreakable));
	}
};

#endif
