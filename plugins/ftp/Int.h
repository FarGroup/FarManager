#ifndef __FAR_PLUGIN_FTP
#define __FAR_PLUGIN_FTP

#include "fstdlib.h"         //FAR plugin stdlib
#include "lib/Plugin.h"  //Plugin

#include "Plugins.h"   //plugins
#include "Cfg.h"       //Config constants and Opt structure
#include "JM.h"        //`JM` changes
#include "var.h"       //class cmd
#include "Connect.h"   //class Connection
#include "FtpAPI.h"    //FtpXXX API
#include "Ftp.h"       //class Ftp

//[ftp_FAR.cpp]
extern FTP     *WINAPI OtherPlugin(FTP *p);
extern int      WINAPI PluginPanelNumber(FTP *p);
extern int      WINAPI PluginUsed(void);

//[ftp_Config.cpp]
extern void     WINAPI ReadCfg(void);
extern void     WINAPI WriteCfg(void);
extern int      WINAPI Config(void);

//[ftp_Dlg.cpp]
extern BOOL     WINAPI AskSaveList(SaveListInfo* sli);
extern BOOL     WINAPI GetLoginData(char *User, char *Password, BOOL forceAsk);

//[ftp_Mix.cpp]
extern BOOL     WINAPI DoCreateDirectory(char *directoryPath);           //Create all directoryes from path
extern void     WINAPI AddEndSlash(char *Path,char slash, size_t ssz);
extern void     WINAPI AddEndSlash(String& p, char slash);
extern void     WINAPI DelEndSlash(char *Path,char slash);
extern void     WINAPI DelEndSlash(String& Path,char slash);
extern void     WINAPI FixFTPSlash(char *Path);
extern void     WINAPI FixFTPSlash(String& Path);
extern void     WINAPI FixLocalSlash(char *Path);
extern void     WINAPI FixLocalSlash(String& Path);
extern char    *WINAPI TruncStr(char *Str,int MaxLength);
extern char    *WINAPI PointToName(char *Path);
extern BOOL     WINAPI CheckForEsc(BOOL isConnection,BOOL IgnoreSilent = FALSE);
extern int      WINAPI CheckForKeyPressed(WORD *Codes,int NumCodes);
extern int      WINAPI IsCaseMixed(char *Str);
extern void     WINAPI LocalLower(char *Str);
extern BOOL     WINAPI IsDirExist(LPCSTR nm);
extern BOOL     WINAPI IsAbsolutePath(LPCSTR nm);

extern HANDLE   WINAPI Fopen(LPCSTR nm,LPCSTR mode /*R|W|A[+]*/, DWORD attr = FILE_ATTRIBUTE_NORMAL);
extern __int64  WINAPI Fsize(LPCSTR nm);
extern __int64  WINAPI Fsize(HANDLE nm);
extern BOOL     WINAPI Fmove(HANDLE file,__int64 restart_point);
extern void     WINAPI Fclose(HANDLE file);
extern int      WINAPI Fwrite(HANDLE File,LPCVOID Buff,int Size);
extern int      WINAPI Fread(HANDLE File,LPVOID Buff,int Size);
extern BOOL     WINAPI Ftrunc(HANDLE h,DWORD move = FILE_CURRENT);

extern BOOL     WINAPI FTestOpen(LPCSTR nm);
extern BOOL     WINAPI FTestFind(LPCSTR nm,FAR_FIND_DATA* ufd = NULL);
extern BOOL     WINAPI FRealFile(LPCSTR nm,FAR_FIND_DATA* ufd = NULL);

extern int      WINAPI FMessage(unsigned int Flags,LPCSTR HelpTopic,LPCSTR *Items,
                                int ItemsNumber,int ButtonsNumber);
extern int      WINAPI FDialog(int X2,int Y2,LPCSTR HelpTopic,struct FarDialogItem *Item,int ItemsNumber);
extern int      WINAPI FDialogEx(int X2,int Y2,LPCSTR HelpTopic,struct FarDialogItem *Item,int ItemsNumber, DWORD Flags = 0,FARWINDOWPROC DlgProc = (FARWINDOWPROC)(size_t)-1,LONG_PTR Param = 0);

extern void     WINAPI IdleMessage(LPCSTR str,int color);
extern int      WINAPI StrSlashCount(LPCSTR m);        //Rets number af any slash chars in string
extern void     WINAPI Size2Str(char *buff,DWORD size);
extern DWORD    WINAPI Str2Size(char *str);
extern void     WINAPI QuoteStr(char *str);
extern void     WINAPI QuoteStr(String& str);

//[ftp_JM.cpp]
extern LPCSTR WINAPI GetSocketErrorSTR(int err);
extern LPCSTR WINAPI GetSocketErrorSTR(void);
extern char    *WINAPI PDigit(char *buff,__int64 val,int sz /*-1*/);            // Converts digit to string.
extern char    *WINAPI FDigit(char *buff,__int64 Value,int BuffSize /*-1*/);    // Output digit to string. Delimits thousands.

extern int      WINAPI AskYesNoMessage(LPCSTR LngMsgNum);
extern BOOL     WINAPI AskYesNo(LPCSTR LngMsgNum);
extern void     WINAPI SayMsg(LPCSTR LngMsgNum);
extern void     WINAPI LogCmd(LPCSTR src,CMDOutputDir out,DWORD Size = MAX_DWORD);
extern BOOL     WINAPI IsCmdLogFile(void);
extern LPCSTR WINAPI GetCmdLogFile(void);
extern char    *WINAPI FixFileNameChars(char *fnm,BOOL slashes = FALSE);
extern char    *WINAPI FixFileNameChars(String& fnm,BOOL slashes = FALSE);

extern void     WINAPI OperateHidden(LPCSTR fnm, BOOL set);

//[ftp_sock.cpp]
extern void     WINAPI scClose(SOCKET& sock,int how = SD_BOTH);
extern BOOL     WINAPI scValid(SOCKET sock);
extern SOCKET   WINAPI scCreate(short addr_type = AF_INET);
extern SOCKET   WINAPI scAccept(SOCKET *peer, struct sockaddr FAR* addr, int* addrlen);

#ifdef _DEBUG
void ShowMemInfo(void);
#else
inline void ShowMemInfo(void) {}
#endif

#ifdef __FILELOG__
void LogPanelItems(struct PluginPanelItem *PanelItem,int ItemsNumber);
#else
#define LogPanelItems( PanelItem,ItemsNumber )
#endif

//------------------------------------------------------------------------
struct FHandle
{
		HANDLE Handle;
	public:
		FHandle(void)     : Handle(NULL) {}
		FHandle(HANDLE h) : Handle(h)    {}
		~FHandle()
		{
			Close();
		}

		void Close(void)
		{
			if(Handle)
			{
				Fclose(Handle);
				Handle = NULL;
			}
		}
};

//[ftp_FAR.cpp]
extern Options        Opt;
extern FTP           *FTPPanels[3];
extern BOOL           SocketStartup;
extern int            SocketInitializeError;
extern FTP           *LastUsedPlugin;
extern char           DialogEditBuffer[];

struct InitDialogItem
{
	int Type;
	int X1,Y1,X2,Y2;
	int Focus;
	DWORD_PTR Selected;
	DWORD Flags;
	int DefaultButton;
	const TCHAR *Data;
};

void InitDialogItems(const InitDialogItem *Init,FarDialogItem *Item, int ItemsNumber);
#endif
