#ifndef __FAR_FTP_PLUGIN
#define __FAR_FTP_PLUGIN

#include <FARStdlib/pack1.h>

//------------------------------------------------------------------------
// TYPES
#if !defined(__FAR_PLUGIN_FTP)
  PRESTRUCT( FTPInterface );
  PRESTRUCT( FTPPluginInterface );

  //After FTP call to QueryInterface function this variable is set to
  //  valid FTP interface object.
  extern PFTPInterface FTP_Info;
  extern HMODULE       FTP_Module;

  // Each plugin must declare this function and include "p_All.cpp" file
  extern PFTPPluginInterface DECLSPEC FTPPluginGetInterface( void );

  // Each plugin must declare this function. Even if you set /NoEntry option
  extern BOOL DECLSPEC FTP_PluginStartup( DWORD Reason );

  //Checker for FTP_Info is valid. Aborts with `fnm` title if check fail
  #if defined(__DEBUG__)
    extern void WINAPI _RTLCheck( CONSTSTR fnm );
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
#include "../lib/ftp_Lang.h"

//------------------------------------------------------------------------
#define IDLE_CONSOLE          1
#define IDLE_CAPTION          2

STRUCT( OptionsPlugin )
  int     AddToDisksMenu;
  int     AddToPluginsMenu;
  int     DisksMenuDigit;
  int     ReadDescriptions;
  int     UpdateDescriptions;
  int     UploadLowCase;
  int     ShowUploadDialog;
  int     ResumeDefault;
  char    Firewall[FAR_MAX_PATHSIZE];
  int     PassiveMode;
  char    DescriptionNames[FAR_MAX_PATHSIZE];
  char    Table[FAR_MAX_PATHSIZE];
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

STRUCT( FTPHostPlugin )
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

STRUCT( FTPInterface )
   DWORD  Magic;
   DWORD  SizeOf;

   PPluginStartupInfo     Info;
   PFarStandardFunctions  FSF;
   char                  *PluginRootKey;
   char                  *PluginStartPath;
   OSVERSIONINFO         *WinVer;
   HMODULE                FTPModule;

//FAR
   CONSTSTR (DECLSPEC    *GetMsg)( int MsgNum );
   CONSTSTR (DECLSPEC    *GetMsgStr)( CONSTSTR Msg);

//Debug
   void     (DECLSPEC_PT *Assertion)( CONSTSTR Format,... );
   void     (DECLSPEC_PT *SayLog)( CONSTSTR Format,... );
   HANDLE   (DECLSPEC_PT *LogProcStart)( CONSTSTR FunctionName,CONSTSTR Format,... );
   void     (DECLSPEC    *LogProcEnd)( HANDLE Proc );

//Reg
   int      (DECLSPEC    *GetRegKeyFullInt)( CONSTSTR Key,CONSTSTR ValueName,DWORD Default );
   char*    (DECLSPEC    *GetRegKeyFullStr)( CONSTSTR Key,CONSTSTR ValueName,char *ValueData,CONSTSTR Default,DWORD DataMaxSize );
   int      (DECLSPEC    *GetRegKeyInt)( CONSTSTR ValueName,DWORD Default );
   char*    (DECLSPEC    *GetRegKeyStr)( CONSTSTR ValueName,char *ValueData,CONSTSTR Default,DWORD DataMaxSize );

//Std
   LPVOID   (DECLSPEC    *Alloc)( SIZE_T sz );
   void     (DECLSPEC    *Del)( LPVOID ptr );
   LPVOID   (DECLSPEC    *Realloc)( LPVOID ptr,SIZE_T sz );
   SIZE_T   (DECLSPEC    *PtrSize)( LPVOID ptr );
   BOOL     (DECLSPEC    *HeapCheck)( void );

   int      (DECLSPEC    *StrCmp)( CONSTSTR str,CONSTSTR str1,int maxlen /*= -1*/, BOOL isCaseSens /*= TRUE*/ );
   char*    (DECLSPEC    *StrCpy)( char *dest,CONSTSTR src,int dest_sz /*=-1*/ );
   char*    (DECLSPEC    *StrCat)( char *dest,CONSTSTR src,int dest_sz /*=-1*/ );
   int      (DECLSPEC    *strLen)( CONSTSTR str );
   char*    (DECLSPEC    *StrChr)( CONSTSTR s, char ch );
   char*    (DECLSPEC    *StrRChr)( CONSTSTR s, char ch );

   int      (DECLSPEC_PT *Sprintf)( char *Buff,CONSTSTR Fmt,... );
   int      (DECLSPEC_PT *SNprintf)( char *Buff,size_t cn,CONSTSTR Fmt,... );
   int      (DECLSPEC    *VSprintf)( char *Buff,CONSTSTR Fmt,va_list arglist );
   int      (DECLSPEC    *VSNprintf)( char *Buff,size_t cn,CONSTSTR Fmt,va_list arglist );

//Utilities
   CONSTSTR (DECLSPEC_PT *Message)( CONSTSTR patt,... );
   CONSTSTR (DECLSPEC    *MessageV)( CONSTSTR patt,va_list a );
   char*    (DECLSPEC    *PointToName)(char *Path);
   char*    (DECLSPEC    *FDigit)( char *buff,__int64 Value,int BuffSize /*-1*/ );  // Output digit to string. Delimits thousands.
   CONSTSTR (DECLSPEC    *FCps)( char *buff,double val );                         // Create CPS value string (Allways 3+1+3+1 length)
   int      (DECLSPEC    *FMessage)( unsigned int Flags,CONSTSTR HelpTopic,CONSTSTR *Items,int ItemsNumber,int ButtonsNumber );
   BOOL     (DECLSPEC    *CheckForEsc)( BOOL isConnection,BOOL IgnoreSilent /*=FALSE*/ );
   void     (DECLSPEC    *IdleMessage)( CONSTSTR str,int color );

//FTP related
   int      (DECLSPEC    *FtpGetRetryCount)( void );
   int      (DECLSPEC    *FtpCmdBlock)( int block /*TRUE,FALSE,-1*/ );

//info
   POptionsPlugin (DECLSPEC *GetOpt)( void );
   PFTPHostPlugin (DECLSPEC *GetHostOpt)( void );
};

#if !defined(__FAR_PLUGIN_FTP)
  CLASS( PluginINProc )
    HANDLE Handle;
   public:
     PluginINProc( HANDLE p )   { Handle = p; }
     ~PluginINProc()            { FTP_Info->LogProcEnd( Handle ); }
  };
#endif
//------------------------------------------------------------------------
// Any plugin
typedef HANDLE (WINAPI *FTP_CreateObject_t)( void );
typedef void   (WINAPI *FTP_DestroyObject_t)( HANDLE Object );

STRUCT( FTPPluginInterface )
   DWORD          Magic;
};

typedef PFTPPluginInterface (WINAPI *FTPQueryInterface_t)( PFTPInterface FTPInfo );

//------------------------------------------------------------------------
#include "../lib/fp_Progress.h"
#include "../lib/fp_DirList.h"
#include "../lib/fp_Notify.h"

#include <FARStdlib/pop.h>
#endif
