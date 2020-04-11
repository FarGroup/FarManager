#ifndef __FAR_FTP_PLUGIN
#define __FAR_FTP_PLUGIN

#ifdef __GNUC__
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

//------------------------------------------------------------------------
// TYPES
#if !defined(__FAR_PLUGIN_FTP)
struct FTPInterface;
struct FTPPluginInterface;

//After FTP call to QueryInterface function this variable is set to
//  valid FTP interface object.
extern FTPInterface* FTP_Info;
extern HMODULE       FTP_Module;

// Each plugin must declare this function and include "All.cpp" file
extern FTPPluginInterface* WINAPI FTPPluginGetInterface(void);

// Each plugin must declare this function. Even if you set /NoEntry option
extern BOOL WINAPI FTP_PluginStartup(DWORD Reason);

//Checker for FTP_Info is valid. Aborts with `fnm` title if check fail
#if defined(__DEBUG__)
extern void WINAPI _RTLCheck(LPCSTR fnm);
#define RTLCheck(v) _RTLCheck(v);
#else
#define RTLCheck(v)
#endif

//Helpers
#undef PROC
#undef Log
#if defined(__FILELOG__)
#define PROC(v) PluginINProc _inp( FTP_Info->LogProcStart v );
#define Log(v)  FTP_Info->SayLog v
#else
#define PROC(v)
#define Log(v)
#endif

#undef  THROW_ERROR
#define THROW_ERROR(err,fl,nm)  FTP_Info->Assertion( "Assertion...\nCondition: \"%s\"\nAt file: \"%s:%d\"",err,fl,nm )
#endif

//------------------------------------------------------------------------
// MAIN LANGUAGE`s
#include "../lib/Lang.h"

//------------------------------------------------------------------------
#define IDLE_CONSOLE          1
#define IDLE_CAPTION          2

struct OptionsPlugin
{
	int     AddToDisksMenu;
	int     AddToPluginsMenu;
	int     DisksMenuDigit;
	int     ReadDescriptions;
	int     UpdateDescriptions;
	int     UploadLowCase;
	int     ShowUploadDialog;
	int     ResumeDefault;
	char    Firewall[MAX_PATH];
	int     PassiveMode;
	char    DescriptionNames[MAX_PATH];
	char    Table[MAX_PATH];
//Configurable
	BOOL    dDelimit;                      //Delimite digits with spec chars                              TRUE
	char    dDelimiter;                    //Character delimiter                                          '.'
	BOOL    AskAbort;                      //Ask Yes/No on abort pressed                                  TRUE
	int     CmdLine;                       //Length of one line in command buffer                         70
	int     CmdLength;                     //Length of command lines in cache                             7
	DWORD   IOBuffSize;                    //Size of IO buffer                                            10.000 (bytes)
	int     ExtCmdView;                    //Extended CMD window                                          TRUE
	int     KeepAlive;                     //Keep alive period (sec)                                      60
	int     PluginColumnMode;              //Default mode for hosts panel                                 -1 (have no preferred mode)
	BOOL    TimeoutRetry;                  //Auto retry operation if timeout error occured                TRUE
	int     RetryCount;                    //Count of auto retryes                                        0
	BOOL    LogOutput;                     //Write output data to log                                     FALSE
	int     LongBeepTimeout;               //long operation beeper
	int     WaitTimeout;                   //Maximum timeout to wait data receiving  (sec)                300
	BOOL    ShowIdle;                      //Show idle percent                                            TRUE
	int     IdleColor;                     //idle percent text color                                      FAR_COLOR(fccCYAN,fccBLUE)
	int     IdleMode;                      //Show mode of idle info (set of IDLE_xxx flags)               IDLE_CONSOLE
	int     ProcessColor;                  //color of processing string in quite mode                     FAR_COLOR(fccBLACK,fccLIGHTGRAY) | COL_DIALOGBOX
	WORD    ServerType;                    //Type of server
	BOOL    FFDup;                         //Duplicate FF symbols on transfer to server
	BOOL    UndupFF;                       //Remove FF duplicate from PWD
	BOOL    ShowSilentProgress;            //Show normal progress on silent operations
	BOOL    ProcessCmd;                    //Default for command line processing
	BOOL    UseBackups;                    //Use FTP backups
	int     WaitIdle;                      //Delay int socket wait function (bigger value decr CPU usage) 0
	int     WaitCounter;                   //Number of waits in second
	int     IdleShowPeriod;                //Period to refresh idle state (ms)
	int     IdleStartPeriod;               //Period before first idle message shown (ms)
	BOOL    AutoAnonymous;                 //Fill blank name with "anonimous"
	int     RetryTimeout;                  //Timeout of auto-retry (sec)
	BOOL    DoNotExpandErrors;             //Do not expand CMD window on error
	int     AskLoginFail;                  //Reask user name and password if login fail                   TRUE
	char   *Months[12];                    //Months names
};

struct FTPHostPlugin
{
	DWORD   Size;                          // Full size of FTPHost structure (used by FAR to store UserData)
	BOOL    AskLogin;
	BOOL    PassiveMode;
	BOOL    UseFirewall;
	BOOL    AsciiMode;
	BOOL    ExtCmdView;
	BOOL    ProcessCmd;
	BOOL    CodeCmd;
	DWORD   IOBuffSize;                    // Size of buffer used to send|recv data
	BOOL    ExtList;                       // Use extended list command
	char    ListCMD[20];                   // Extended list command
	WORD    ServerType;                    // Type of server
	BOOL    FFDup;                         // Duplicate FF char on string sent to server
	BOOL    UndupFF;                       // Remove FF duplicate from PWD
	BOOL    DecodeCmdLine;                 // Decode OEM cmd line chars to hosts code page
	BOOL    SendAllo;                      // Send allo before upload
	BOOL    UseStartSpaces;             // Ignore spaces from start of file name
};

//------------------------------------------------------------------------
// FTP INTERFACE
#define FTP_INTERFACE_MAGIC   MK_ID( 'F','T','P',1 )

struct FTPInterface
{
	DWORD  Magic;
	DWORD  SizeOf;

	PluginStartupInfo*     Info;
	FarStandardFunctions*  FSF;
	char                  *PluginRootKey;
	char                  *PluginStartPath;
	OSVERSIONINFO         *WinVer;
	HMODULE                FTPModule;

//FAR
	LPCSTR(WINAPI    *GetMsg)(int MsgNum);
	LPCSTR(WINAPI    *GetMsgStr)(LPCSTR Msg);

//Debug
	void (__cdecl *Assertion)(LPCSTR Format,...);
	void (__cdecl *SayLog)(LPCSTR Format,...);
	HANDLE(__cdecl *LogProcStart)(LPCSTR FunctionName,LPCSTR Format,...);
	void (WINAPI    *LogProcEnd)(HANDLE Proc);

//Reg
	int (WINAPI    *GetRegKeyFullInt)(LPCSTR Key,LPCSTR ValueName,DWORD Default);
	char*(WINAPI    *GetRegKeyFullStr)(LPCSTR Key,LPCSTR ValueName,char *ValueData,LPCSTR Default,DWORD DataMaxSize);
	int (WINAPI    *GetRegKeyInt)(LPCSTR ValueName,DWORD Default);
	char*(WINAPI    *GetRegKeyStr)(LPCSTR ValueName,char *ValueData,LPCSTR Default,DWORD DataMaxSize);

//Std
	int (WINAPI    *StrCmp)(LPCSTR str,LPCSTR str1,int maxlen /*= -1*/, BOOL isCaseSens /*= TRUE*/);
	char*(WINAPI    *StrCpy)(char *dest,LPCSTR src,int dest_sz /*=-1*/);
	char*(WINAPI    *StrCat)(char *dest,LPCSTR src,int dest_sz /*=-1*/);

//Utilities
	LPCSTR(__cdecl *Message)(LPCSTR patt,...);
	LPCSTR(WINAPI    *MessageV)(LPCSTR patt,va_list a);
	char*(WINAPI    *PointToName)(char *Path);
	char*(WINAPI    *FDigit)(char *buff,__int64 Value,int BuffSize /*-1*/);        // Output digit to string. Delimits thousands.
	LPCSTR(WINAPI    *FCps)(char *buff,double val);                            // Create CPS value string (Allways 3+1+3+1 length)
	int (WINAPI    *FMessage)(unsigned int Flags,LPCSTR HelpTopic,LPCSTR *Items,int ItemsNumber,int ButtonsNumber);
	BOOL (WINAPI    *CheckForEsc)(BOOL isConnection,BOOL IgnoreSilent /*=FALSE*/);
	void (WINAPI    *IdleMessage)(LPCSTR str,int color);

//FTP related
	int (WINAPI    *FtpGetRetryCount)(void);
	int (WINAPI    *FtpCmdBlock)(int block /*TRUE,FALSE,-1*/);

//info
	OptionsPlugin*(WINAPI *GetOpt)(void);
	FTPHostPlugin*(WINAPI *GetHostOpt)(void);
};

#if !defined(__FAR_PLUGIN_FTP)
class PluginINProc
{
		HANDLE Handle;
	public:
		PluginINProc(HANDLE p)   { Handle = p; }
		~PluginINProc()            { FTP_Info->LogProcEnd(Handle); }
};
#endif
//------------------------------------------------------------------------
// Any plugin
typedef HANDLE(WINAPI *FTP_CreateObject_t)(void);
typedef void (WINAPI *FTP_DestroyObject_t)(HANDLE Object);

struct FTPPluginInterface
{
	DWORD          Magic;
};

typedef FTPPluginInterface*(WINAPI *FTPQueryInterface_t)(FTPInterface* FTPInfo);

//------------------------------------------------------------------------
#include "../lib/Progress.h"
#include "../lib/DirList.h"
#include "../lib/Notify.h"

#ifdef __GNUC__
#pragma pack()
#else
#pragma pack(pop)
#endif
#endif
