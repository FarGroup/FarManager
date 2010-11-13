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

#define INTERNET_ERROR_BASE                     12000

#define ERROR_INTERNET_OUT_OF_HANDLES           (INTERNET_ERROR_BASE + 1)
#define ERROR_INTERNET_TIMEOUT                  (INTERNET_ERROR_BASE + 2)
#define ERROR_INTERNET_EXTENDED_ERROR           (INTERNET_ERROR_BASE + 3)
#define ERROR_INTERNET_INTERNAL_ERROR           (INTERNET_ERROR_BASE + 4)
#define ERROR_INTERNET_INVALID_URL              (INTERNET_ERROR_BASE + 5)
#define ERROR_INTERNET_UNRECOGNIZED_SCHEME      (INTERNET_ERROR_BASE + 6)
#define ERROR_INTERNET_NAME_NOT_RESOLVED        (INTERNET_ERROR_BASE + 7)
#define ERROR_INTERNET_PROTOCOL_NOT_FOUND       (INTERNET_ERROR_BASE + 8)
#define ERROR_INTERNET_INVALID_OPTION           (INTERNET_ERROR_BASE + 9)
#define ERROR_INTERNET_BAD_OPTION_LENGTH        (INTERNET_ERROR_BASE + 10)
#define ERROR_INTERNET_OPTION_NOT_SETTABLE      (INTERNET_ERROR_BASE + 11)
#define ERROR_INTERNET_SHUTDOWN                 (INTERNET_ERROR_BASE + 12)
#define ERROR_INTERNET_INCORRECT_USER_NAME      (INTERNET_ERROR_BASE + 13)
#define ERROR_INTERNET_INCORRECT_PASSWORD       (INTERNET_ERROR_BASE + 14)
#define ERROR_INTERNET_LOGIN_FAILURE            (INTERNET_ERROR_BASE + 15)
#define ERROR_INTERNET_INVALID_OPERATION        (INTERNET_ERROR_BASE + 16)
#define ERROR_INTERNET_OPERATION_CANCELLED      (INTERNET_ERROR_BASE + 17)
#define ERROR_INTERNET_INCORRECT_HANDLE_TYPE    (INTERNET_ERROR_BASE + 18)
#define ERROR_INTERNET_INCORRECT_HANDLE_STATE   (INTERNET_ERROR_BASE + 19)
#define ERROR_INTERNET_NOT_PROXY_REQUEST        (INTERNET_ERROR_BASE + 20)
#define ERROR_INTERNET_REGISTRY_VALUE_NOT_FOUND (INTERNET_ERROR_BASE + 21)
#define ERROR_INTERNET_BAD_REGISTRY_PARAMETER   (INTERNET_ERROR_BASE + 22)
#define ERROR_INTERNET_NO_DIRECT_ACCESS         (INTERNET_ERROR_BASE + 23)
#define ERROR_INTERNET_NO_CONTEXT               (INTERNET_ERROR_BASE + 24)
#define ERROR_INTERNET_NO_CALLBACK              (INTERNET_ERROR_BASE + 25)
#define ERROR_INTERNET_REQUEST_PENDING          (INTERNET_ERROR_BASE + 26)
#define ERROR_INTERNET_INCORRECT_FORMAT         (INTERNET_ERROR_BASE + 27)
#define ERROR_INTERNET_ITEM_NOT_FOUND           (INTERNET_ERROR_BASE + 28)
#define ERROR_INTERNET_CANNOT_CONNECT           (INTERNET_ERROR_BASE + 29)
#define ERROR_INTERNET_CONNECTION_ABORTED       (INTERNET_ERROR_BASE + 30)
#define ERROR_INTERNET_CONNECTION_RESET         (INTERNET_ERROR_BASE + 31)
#define ERROR_INTERNET_FORCE_RETRY              (INTERNET_ERROR_BASE + 32)
#define ERROR_INTERNET_INVALID_PROXY_REQUEST    (INTERNET_ERROR_BASE + 33)
#define ERROR_INTERNET_NEED_UI                  (INTERNET_ERROR_BASE + 34)

#define ERROR_INTERNET_HANDLE_EXISTS            (INTERNET_ERROR_BASE + 36)
#define ERROR_INTERNET_SEC_CERT_DATE_INVALID    (INTERNET_ERROR_BASE + 37)
#define ERROR_INTERNET_SEC_CERT_CN_INVALID      (INTERNET_ERROR_BASE + 38)
#define ERROR_INTERNET_HTTP_TO_HTTPS_ON_REDIR   (INTERNET_ERROR_BASE + 39)
#define ERROR_INTERNET_HTTPS_TO_HTTP_ON_REDIR   (INTERNET_ERROR_BASE + 40)
#define ERROR_INTERNET_MIXED_SECURITY           (INTERNET_ERROR_BASE + 41)
#define ERROR_INTERNET_CHG_POST_IS_NON_SECURE   (INTERNET_ERROR_BASE + 42)
#define ERROR_INTERNET_POST_IS_NON_SECURE       (INTERNET_ERROR_BASE + 43)
#define ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED  (INTERNET_ERROR_BASE + 44)
#define ERROR_INTERNET_INVALID_CA               (INTERNET_ERROR_BASE + 45)
#define ERROR_INTERNET_CLIENT_AUTH_NOT_SETUP    (INTERNET_ERROR_BASE + 46)
#define ERROR_INTERNET_ASYNC_THREAD_FAILED      (INTERNET_ERROR_BASE + 47)
#define ERROR_INTERNET_REDIRECT_SCHEME_CHANGE   (INTERNET_ERROR_BASE + 48)

//
// FTP API errors
//
#define ERROR_FTP_TRANSFER_IN_PROGRESS          (INTERNET_ERROR_BASE + 110)
#define ERROR_FTP_DROPPED                       (INTERNET_ERROR_BASE + 111)

#endif
