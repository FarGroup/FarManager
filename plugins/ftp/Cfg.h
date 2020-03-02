#ifndef __FAR_PLUGIN_CONFIGDATA
#define __FAR_PLUGIN_CONFIGDATA

//Traffic message [ftp_TraficCB.cpp]
#include "pwd.h"
#define FTR_MAXTIME           FTR_HOURSEC*99        //max calculated estimated time (sec)
#define FTR_MINBUFFSIZE       50UL

//Dialog edit history
#define FTP_GETHISTORY        "FTPGet"
#define FTP_PUTHISTORY        "FTPPut"
#define FTP_HOSTHISTORY       "FTPHost"
#define FTP_FOLDHISTORY       "FTPFolder"
#define FTP_USERHISTORY       "FTPUser"
#define FTP_INCHISTORY        "FTPIncludeMask"
#define FTP_EXCHISTORY        "FTPExcludeMask"

#define FTP_CMDPREFIX         "ftp"
#define FTP_CMDPREFIX_SIZE    3

#define FTP_HOSTID            MK_ID( 'F','H','s','t' )

#define FTR_HOURSEC           3600
#define FTR_MINSEC            60

#define MAXHOSTNAMELEN        64

#define DIALOG_EDIT_SIZE      8000  //Size of max dialog edit field

#define FTP_FILENAME( p )     (FPIL_ADDEXIST(p) ? ((char*)FPIL_ADDDATA(p)) : (p)->FindData.cFileName)

//Security flags
#define SEC_PLAINPWD          0x0001        //plugin can place password in generated url in a plain text

//transfer types
enum ftTypes
{
	TYPE_A = 'A',
	TYPE_I = 'I',
	TYPE_E = 'E',
	TYPE_L = 'L',
	TYPE_NONE = 0
};

//Socket wait state
enum
{
	ws_connect,
	ws_read,
	ws_write,
	ws_accept,
	ws_error
};

//FTP overwrite mode
enum overCode
{
	ocOverAll,
	ocSkip,
	ocSkipAll,
	ocOver,
	ocResume,
	ocResumeAll,
	ocCancel,
	ocNone,
	ocNewer,
	ocNewerAll
};

//Current FTP state
enum FTPCurrentStates
{
	fcsNormal       = 0,
	fcsExpandList   = 1,
	fcsClose        = 2,
	fcsConnecting   = 3,
	fcsFTP          = 4,
	fcsOperation    = 5,
	fcsProcessFile  = 6,
	fcs_None
};

//CMDLog output directions
enum CMDOutputDir
{
	ldOut,     //plugin to server
	ldIn,      //server to plugin
	ldInt,     //internal log
	ldRaw,     //Log raw data dump (w\o header lines)
	ld_None
};

//ExpandList callback
typedef BOOL (*ExpandListCB)(PluginPanelItem *p,LPVOID Param);

//Save list options
enum sliTypes
{
	sltUrlList,
	sltTree,
	sltGroup,
	sltNone
};

struct SaveListInfo
{
	char     path[MAX_PATH];
	BOOL     Append;
	BOOL     AddPrefix;             //all
	BOOL     AddPasswordAndUser;
	BOOL     Quote;
	BOOL     Size;                  //tree and group
	int      RightBound;            //tree
	sliTypes ListType;
};

//Configuration options
struct Options: public OptionsPlugin
{
	char    DefaultPassword[FAR_MAX_NAME];

//Configurable
	char    CmdLogFile[MAX_PATH];  //Log file where FTP commands placed                           "" (none)
	int     CmdLogLimit;                   //Limit of cmd log file (*1000 bytes)                          100 (100.000 bytes)
	BOOL    CloseDots;                     //Switch on ".." to hosts                                      TRUE
	BOOL    QuoteClipboardNames;           //Quote names placed to clipboard
	BOOL    SetHiddenOnAbort;              //Set hidden attribute on uncomplete files

//Techinfos
	DWORD   PwdSecurity;                   //Set of SEC_xxx

	char    fmtDateFormat[ 50 ];           //Server date-time format                                      "%*s %04d%02d%02d%02d%02d%02d"
	BOOL    TruncateLogFile;               //Truncate log file on plugin start                            FALSE
	char    InvalidSymbols[MAX_PATH];
	char    CorrectedSymbols[MAX_PATH];

//Queque processing
	BOOL    RestoreState;
	BOOL    RemoveCompleted;

	SaveListInfo sli;

	BOOL    _ShowPassword;                 //Show paswords in any places                                  FALSE

	char    cmdPut[ 20 ];                  //"STOR" /*APPE*/
	char    cmdAppe[ 20 ];                 //"APPE"
	char    cmdStor[ 20 ];                 //"STOR"
	char    cmdPutUniq[ 20 ];              //"STOU"
	char    cmdPasv[ 20 ];                 //"PASV"
	char    cmdPort[ 20 ];                 //"PORT"
	char    cmdMDTM[ 20 ];                 //"MDTM"
	char    cmdRetr[ 20 ];                 //"RETR"
	char    cmdRest[ 20 ];                 //"REST"
	char    cmdAllo[ 20 ];                 //"ALLO"
	char    cmdCwd[ 20 ];                  //"CWD"
	char    cmdXCwd[ 20 ];                 //"XCWD"
	char    cmdDel[ 20 ];                  //"DELE"
	char    cmdRen[ 20 ];                  //"RNFR"
	char    cmdRenTo[ 20 ];                //"RNTO"
	char    cmdList[ 20 ];                 //"LIST"
	char    cmdNList[ 20 ];                //"NLIST"
	char    cmdUser[ 20 ];                 //"USER"
	char    cmdPass[ 20 ];                 //"PASS"
	char    cmdAcct[ 20 ];                 //"ACCT"
	char    cmdPwd[ 20 ];                  //"PWD"
	char    cmdXPwd[ 20 ];                 //"XPWD"
	char    cmdMkd[ 20 ];                  //"MKD"
	char    cmdXMkd[ 20 ];                 //"XMKD"
	char    cmdRmd[ 20 ];                  //"RMD"
	char    cmdXRmd[ 20 ];                 //"XRMD"
	char    cmdSite[ 20 ];                 //"SITE"
	char    cmdChmod[ 20 ];                //"CHMOD"
	char    cmdUmask[ 20 ];                //"UMASK"
	char    cmdIdle[ 20 ];                 //"IDLE"
	char    cmdHelp[ 20 ];                 //"HELP"
	char    cmdQuit[ 20 ];                 //"QUIT"
	char    cmdCDUp[ 20 ];                 //"CDUP"
	char    cmdXCDUp[ 20 ];                //"XCUP"
	char    cmdSyst[ 20 ];                 //"SYST"
	char    cmdSize[ 20 ];                 //"SISE"
	char    cmdStat[ 20 ];                 //"STAT"
};

#endif
