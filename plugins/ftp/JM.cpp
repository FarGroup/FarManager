#include <all_far.h>
#pragma hdrstop

#include "Int.h"

/*  1 -       title
    2 -       --- betweeen commands and status
    3 - [OPT] status
    4 - [OPT] --- between status and buttons
              commands
    5 - [OPT] --- betweeen commands and status
    6 - [OPT] button
    7 - [OPT] button1
    8 - [OPT] button2
    9 - [OPT] button Cancel
*/
#define NBUTTONSADDON 10

//------------------------------------------------------------------------
// DEBUG ONLY
//------------------------------------------------------------------------
#if defined(__FILELOG__)
void LogPanelItems(struct PluginPanelItem *pi,int cn)
{
	Log(("Items in list %p: %d", pi, cn));

	if(pi)
		for(int n = 0; n < cn; n++)
		{
			Log(("%2d) [%s] attr: %08X (%s)",
			     n+1, FTP_FILENAME(&pi[n]),
			     pi[n].FindData.dwFileAttributes, IS_FLAG(pi[n].FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY)?"DIR":"FILE"
			    ));
		}
}
#endif

//------------------------------------------------------------------------
int FARINProc::Counter = 0;

FARINProc::FARINProc(LPCSTR nm,LPCSTR s,...)
	: Name(nm)
{
	va_list  ap;
	char     str[500];
	DWORD    err = GetLastError();

	if(s)
	{
		va_start(ap,s);
		_snprintf(str, ARRAYSIZE(str), "%*c%s(%s) {",
		          Counter*2,' ', nm, MessageV(s,ap));
		va_end(ap);
	}
	else
		_snprintf(str, ARRAYSIZE(str), "%*c%s() {",
		          Counter*2,' ', nm);

	LogCmd(str, ldInt);
	Counter++;
	SetLastError(err);
}

FARINProc::~FARINProc()
{
	DWORD err = GetLastError();
	char  str[500];
	Counter--;
	_snprintf(str, ARRAYSIZE(str), "%*c}<%s>", Counter*2,' ',Name);
	LogCmd(str,ldInt);
	SetLastError(err);
}

void __cdecl FARINProc::Say(LPCSTR s,...)
{
	va_list ap;
	char    str[500];
	int     rc;
	DWORD   err = GetLastError();
	va_start(ap,s);
	rc = _snprintf(str, ARRAYSIZE(str), "%*c", Counter*2,' ');

	if(rc < (int)ARRAYSIZE(str))
		vsnprintf(str+rc, ARRAYSIZE(str)-rc, s,ap);

	va_end(ap);
	LogCmd(str,ldInt);
	SetLastError(err);
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
char *WINAPI FixFileNameChars(String& fnm,BOOL slashes)
{
	return FixFileNameChars((char*)fnm.c_str(), slashes);
}

char *WINAPI FixFileNameChars(char *fnm,BOOL slashes)
{
	char *m;
	char *src,
	     *inv = Opt.InvalidSymbols,
	      *cor = Opt.CorrectedSymbols;
	static char buff[MAX_PATH];

	if(!inv[0]) return fnm;

	StrCpy(buff, fnm, ARRAYSIZE(buff));

	for(src = buff; *src; src++)
		if((m=strchr(inv,*src)) != NULL)
			*src = cor[ m-inv ];

	if(slashes)
		for(src = buff; *src; src++)
			if(*src == '\\') *src = '_';
			else if(*src == ':')  *src = '!';

	return buff;
}
//------------------------------------------------------------------------
/*
   Create Socket errr string.
   Returns static buffer

   Windows has no way to create string by socket error, so make it inside plugin
*/
static struct
{
	int   Code;
	int   MCode;
} stdSockErrors[] =
{
	/* Windows Sockets definitions of regular Microsoft C error constants */
	{ WSAEINTR,            MWSAEINTR },
	{ WSAEBADF,            MWSAEBADF },
	{ WSAEACCES,           MWSAEACCES },
	{ WSAEFAULT,           MWSAEFAULT },
	{ WSAEINVAL,           MWSAEINVAL },
	{ WSAEMFILE,           MWSAEMFILE },
	/* Windows Sockets definitions of regular Berkeley error constants */
	{ WSAEWOULDBLOCK,      MWSAEWOULDBLOCK },
	{ WSAEINPROGRESS,      MWSAEINPROGRESS },
	{ WSAEALREADY,         MWSAEALREADY },
	{ WSAENOTSOCK,         MWSAENOTSOCK },
	{ WSAEDESTADDRREQ,     MWSAEDESTADDRREQ },
	{ WSAEMSGSIZE,         MWSAEMSGSIZE },
	{ WSAEPROTOTYPE,       MWSAEPROTOTYPE },
	{ WSAENOPROTOOPT,      MWSAENOPROTOOPT },
	{ WSAEPROTONOSUPPORT,  MWSAEPROTONOSUPPORT },
	{ WSAESOCKTNOSUPPORT,  MWSAESOCKTNOSUPPORT },
	{ WSAEOPNOTSUPP,       MWSAEOPNOTSUPP },
	{ WSAEPFNOSUPPORT,     MWSAEPFNOSUPPORT },
	{ WSAEAFNOSUPPORT,     MWSAEAFNOSUPPORT },
	{ WSAEADDRINUSE,       MWSAEADDRINUSE },
	{ WSAEADDRNOTAVAIL,    MWSAEADDRNOTAVAIL },
	{ WSAENETDOWN,         MWSAENETDOWN },
	{ WSAENETUNREACH,      MWSAENETUNREACH },
	{ WSAENETRESET,        MWSAENETRESET },
	{ WSAECONNABORTED,     MWSAECONNABORTED },
	{ WSAECONNRESET,       MWSAECONNRESET },
	{ WSAENOBUFS,          MWSAENOBUFS },
	{ WSAEISCONN,          MWSAEISCONN },
	{ WSAENOTCONN,         MWSAENOTCONN },
	{ WSAESHUTDOWN,        MWSAESHUTDOWN },
	{ WSAETOOMANYREFS,     MWSAETOOMANYREFS },
	{ WSAETIMEDOUT,        MWSAETIMEDOUT },
	{ WSAECONNREFUSED,     MWSAECONNREFUSED },
	{ WSAELOOP,            MWSAELOOP },
	{ WSAENAMETOOLONG,     MWSAENAMETOOLONG },
	{ WSAEHOSTDOWN,        MWSAEHOSTDOWN },
	{ WSAEHOSTUNREACH,     MWSAEHOSTUNREACH },
	{ WSAENOTEMPTY,        MWSAENOTEMPTY },
	{ WSAEPROCLIM,         MWSAEPROCLIM },
	{ WSAEUSERS,           MWSAEUSERS },
	{ WSAEDQUOT,           MWSAEDQUOT },
	{ WSAESTALE,           MWSAESTALE },
	{ WSAEREMOTE,          MWSAEREMOTE },
	/* Extended Windows Sockets error constant definitions */
	{ WSASYSNOTREADY,      MWSASYSNOTREADY },
	{ WSAVERNOTSUPPORTED,  MWSAVERNOTSUPPORTED },
	{ WSANOTINITIALISED,   MWSANOTINITIALISED },
	{ WSAEDISCON,          MWSAEDISCON },
	{ WSAHOST_NOT_FOUND,   MWSAHOST_NOT_FOUND },
	{ WSATRY_AGAIN,        MWSATRY_AGAIN },
	{ WSANO_RECOVERY,      MWSANO_RECOVERY },
	{ WSANO_DATA,          MWSANO_DATA },
	{ WSANO_ADDRESS,       MWSANO_ADDRESS },
	{ 0, MNone__ }
};

LPCSTR WINAPI GetSocketErrorSTR(void)
{
	return GetSocketErrorSTR(WSAGetLastError());
}

LPCSTR WINAPI GetSocketErrorSTR(int err)
{
	static char estr[70];

	if(!err)
		return FP_GetMsg(MWSAENoError);

	for(int n = 0; stdSockErrors[n].MCode != MNone__; n++)
		if(stdSockErrors[n].Code == err)
			return FP_GetMsg(stdSockErrors[n].MCode);

	sprintf(estr,"%s: %d",FP_GetMsg(MWSAEUnknown),err);
	return estr;
}
//------------------------------------------------------------------------
/*
   Procedure for convert digit to string
   Like an AtoI but can manipulate __int64 and has buffer limit

   Align digit to left, if buffer less when digit the output will be
   truncated at right.
   If size set to -1 the whole value will be output to ctring.
*/
char *WINAPI PDigit(char *buff,__int64 val,int sz /*=-1*/)
{
	static char lbuff[100];
	char str[ 100 ];
	int pos = ARRAYSIZE(str)-1;

	if(!buff) buff = lbuff;

	str[pos] = 0;

	if(!val)
		str[--pos] = '0';
	else
		for(; pos && val; val /= 10)
			str[--pos] = (char)('0'+val%10);

	if(sz != -1)
	{
		if(((int)ARRAYSIZE(str))-1-pos > sz)
		{
			str[ pos+sz-1 ] = FAR_RIGHT_CHAR;
		}
	}

	if(pos <= 0)
		str[0] = FAR_LEFT_CHAR;

	StrCpy(buff,str+pos,sz == -1 ? (-1) : (sz+1));
	return buff;
}
/*
   Output digit to string
   Can delimit thousands by special character

   Fill buffer allways with `sz` characters
   If digit less then `sz` in will be added by ' ' at left
   If digit more then buffer digit will be truncated at left
*/
char *WINAPI FDigit(char *buff,__int64 val,int sz)
{
	static char lbuff[100];
	char str[MAX_PATH];
	int  len,n,d;
	char *s;

	if(!buff) buff = lbuff;

	*buff = 0;

	if(!sz) return buff;

	if(!Opt.dDelimit || !Opt.dDelimiter)
	{
		PDigit(buff,val,sz);
		return buff;
	}

	PDigit(str,val,sz);
	len = (int)strlen(str);
	s   = str + len-1;

	if(sz == -1)
		sz = len + len/3 - ((len%3) == 0);

	d = sz;
	buff[d--] = 0;

	for(n = 0; d >= 0 && n < sz && n < len; n++)
	{
		if(n && (n%3) == 0)
		{
			buff[d--] = Opt.dDelimiter;
			sz--;
		}

		if(d >= 0)
			buff[d--] = *(s--);
	}

	if(n > sz)
		buff[0] = FAR_LEFT_CHAR;

	if(d >= 0)
		for(; d >= 0; d--) buff[d] = ' ';

	return buff;
}
//------------------------------------------------------------------------
/*
    Show `Message` with attention caption and query user to select YES or NO
    Returns nonzero if user select YES
*/
int WINAPI AskYesNoMessage(LPCSTR LngMsgNum)
{
	static LPCSTR MsgItems[] =
	{
		FMSG(MAttention),
		NULL,
		FMSG(MYes), FMSG(MNo)
	};
	MsgItems[1] = LngMsgNum;
	return FMessage(FMSG_WARNING, NULL, MsgItems, ARRAYSIZE(MsgItems),2);
}

BOOL WINAPI AskYesNo(LPCSTR LngMsgNum)
{
	return AskYesNoMessage(LngMsgNum) == 0;
}
/*
    Show void `Message` with attention caption
*/
void WINAPI SayMsg(LPCSTR LngMsgNum)
{
	LPCSTR MsgItems[]=
	{
		FMSG(MAttention),
		LngMsgNum,
		FMSG(MOk)
	};
	FMessage(FMSG_WARNING, NULL, MsgItems,ARRAYSIZE(MsgItems),1);
}
//------------------------------------------------------------------------
BOOL WINAPI IsCmdLogFile(void)
{
	return Opt.CmdLogFile[0] != 0;
}

extern char *FP_PluginStartPath;

LPCSTR WINAPI GetCmdLogFile(void)
{
	static char str[MAX_PATH] = {0};

	if(Opt.CmdLogFile[0])
	{
		if(Opt.CmdLogFile[0] && Opt.CmdLogFile[1] == ':')
			return Opt.CmdLogFile;
		else if(str[0])
			return str;
		else
		{
			StrCpy(str, FP_PluginStartPath, ARRAYSIZE(str));
			StrCat(str, "\\",               ARRAYSIZE(str));
			StrCat(str,Opt.CmdLogFile,      ARRAYSIZE(str));
			return str;
		}
	}
	else
		return "";
}
/*
   Writes one string to log file

   Start from end if file at open bigger then limit size
   `out` is a direction of string (server, plugin, internal)
*/

static HANDLE LogFile = NULL;

void WINAPI LogCmd(LPCSTR src,CMDOutputDir out,DWORD Size)
{
	LPCSTR   m;

//File opened and fail
	if(LogFile == INVALID_HANDLE_VALUE)
		return;

//Params
	if(!IsCmdLogFile() || !src)
		return;

	src = FP_GetMsg(src);

	if(out == ldRaw && (!Size || Size == MAX_DWORD))
		return;
	else if(!src[0])
		return;

//Open file
	//Name
	m = GetCmdLogFile();

	if(!m || !m[0])
		return;

	//Open
	LogFile = Fopen(m, !LogFile && Opt.TruncateLogFile ? "w" : "w+");

	if(!LogFile)
	{
		LogFile = INVALID_HANDLE_VALUE;
		return;
	}

	//Check limitations
	if(Opt.CmdLogLimit &&
	        Fsize(LogFile) >= (__int64)Opt.CmdLogLimit*1000)
		Ftrunc(LogFile,FILE_BEGIN);

//-- USED DATA
	static SYSTEMTIME stOld = { 0 };
	SYSTEMTIME st;
	char       tmstr[ 100 ];

//-- RAW
	if(out == ldRaw)
	{
		_snprintf(tmstr, ARRAYSIZE(tmstr), "%d ", PluginUsed());
		Fwrite(LogFile,tmstr,static_cast<int>(strlen(tmstr)));
		signed n;

		for(n = ((int)Size)-1; n > 0 && strchr("\n\r",src[n]); n--);

		if(n > 0)
		{
			Fwrite(LogFile,"--- RAW ---\r\n",13);
			Fwrite(LogFile,src,Size);
			Fwrite(LogFile,"\r\n--- RAW ---\r\n",15);
		}

		Fclose(LogFile);
		return;
	}

//-- TEXT

//Replace PASW
	if(!Opt._ShowPassword && StrCmp(src,"PASS ",5,TRUE) == 0)
		src = "PASS *hidden*";

//Write multiline to log
	do
	{
		//Plugin
		_snprintf(tmstr, ARRAYSIZE(tmstr), "%d ", PluginUsed());
		Fwrite(LogFile,tmstr,static_cast<int>(strlen(tmstr)));
		//Time
		GetLocalTime(&st);
		_snprintf(tmstr,ARRAYSIZE(tmstr),
		          "%4d.%02d.%02d %02d:%02d:%02d:%04d",
		          st.wYear, st.wMonth,  st.wDay,
		          st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		Fwrite(LogFile,tmstr,static_cast<int>(strlen(tmstr)));

		//Delay
		if(!stOld.wYear)
			sprintf(tmstr," ----");
		else
			sprintf(tmstr," %04d",
			        (st.wSecond-stOld.wSecond)*1000 + (st.wMilliseconds-stOld.wMilliseconds));

		Fwrite(LogFile,tmstr,(int)strlen(tmstr));
		stOld = st;

		//Direction
		if(out == ldInt)
			Fwrite(LogFile,"| ",2);
		else
			Fwrite(LogFile,(out == ldOut) ? "|->" : "|<-",3);

		//Message
		for(; *src && !strchr("\r\n",*src); src++)
			Fwrite(LogFile,src,1);

		Fwrite(LogFile,"\n",1);

		while(*src && strchr("\r\n",*src)) src++;
	}
	while(*src);

	Fclose(LogFile);
}

//------------------------------------------------------------------------
//  Connection
//------------------------------------------------------------------------
void Connection::InitIOBuff(void)
{
	CloseIOBuff();
	IOBuff = (char*)malloc(Host.IOBuffSize+1);
}
void Connection::CloseIOBuff(void)
{
	free(IOBuff);
	IOBuff = NULL;
}

/*
   Initialize CMD buffer data
*/
void Connection::InitCmdBuff(void)
{
	if(CmdBuff)
		CloseCmdBuff();

	hIdle        = Opt.IdleShowPeriod ? FP_PeriodCreate(Opt.IdleShowPeriod) : NULL;
	CmdVisible   = TRUE;
	cmdLineSize  = Opt.CmdLine;
	cmdSize      = Opt.CmdLength;
	RetryCount   = 0;
	CmdBuff      = (char **)malloc(sizeof(char *)*(cmdSize+1));
	RplBuff      = (char **)malloc(sizeof(char *)*(cmdSize+1));

	//Command lines (length + 0)
	for(int n = 0; n < cmdSize; n++)
	{
		CmdBuff[n] = (char*)malloc(cmdLineSize+1);
		RplBuff[n] = (char*)malloc(1024+1);
	}

	//Command lines + status command
	CmdMsg       = (LPCSTR*)malloc(sizeof(LPCSTR)*(cmdSize+NBUTTONSADDON));
	ResetCmdBuff();
}
/*
   Free initialize CMD buffer data
*/
void Connection::CloseCmdBuff(void)
{
	if(CmdBuff)
		for(int n = 0; n < cmdSize; n++)
		{
			free(CmdBuff[n]);
			free(RplBuff[n]);
		}

	free(CmdBuff);
	free(RplBuff);
	free(CmdMsg);
	CmdBuff = NULL;
	RplBuff = NULL;
	CmdMsg  = NULL;
	IOBuff  = NULL;
	FP_PeriodDestroy(hIdle);
}
//------------------------------------------------------------------------
/*
   Output message about internal plugin error
*/
void Connection::InternalError(void)
{
	LPCSTR MsgItems[] =
	{
		FMSG(MFtpTitle),
		FMSG(MIntError),
		FMSG(MOk)
	};
	FMessage(FMSG_WARNING, NULL, MsgItems, ARRAYSIZE(MsgItems), 1);
}
/*
   Set CMD buffer data to zero state

   Used just after new connection established
*/
void Connection::ResetCmdBuff(void)
{
	hostname[0]     = 0;
	LastHost[0]     = 0;
	LastMsg[0]      = 0;
}

//------------------------------------------------------------------------
/*
   Format `src` string to output info CMD window

   dest - buffer of cmd window
   sz   - size of cmd size (string will be grow|truncated to this size)
   out  - signals if this is output or input string

   IF out == -1
     `src` placed as is - do not resize to `sz`; do not add direction text
*/
void Connection::SetCmdLine(char *dest, LPCSTR src, int sz, int out)
{
	int l,off;
	char *m;

	if(!Opt._ShowPassword &&
	        StrCmp(src,Opt.cmdPass,static_cast<int>(strlen(Opt.cmdPass)),TRUE) == 0)
		src = "PASS *hidden*";
	else if(((BYTE)src[0]) == (BYTE)ffDM &&
	        (strncmp(src+1,"ABOR",4) == 0 || strncmp(src,"ABOR",4) == 0))
		src = "<ABORT>";

	if(out != -1)
	{
		dest[0] = out?'-':'<';
		dest[1] = out?'>':'-';
		dest[2] = ' ';
		off = 3;
	}
	else
		off = 0;

	sz--;
	StrCpy(dest+off, src, sz-off);

	if((m=strchr(dest,'\r')) != NULL) *m = 0;

	if((m=strchr(dest,'\n')) != NULL) *m = 0;

	if(out != -1)
	{
		for(l = static_cast<int>(strlen(dest)); l < sz; l++)
			dest[l] = ' ';

		dest[l] = 0;

		if(Host.CodeCmd)
		{
			if(out)
				ToOEM(dest);
			else
				ToOEM(dest);
		}
	}
}
/*
   Add a string to `RPL buffer` and `CMD buffer`

   Do not accept empty strings
   Scrolls buffers up if length of beffers bigger then CMD length
   Call ConnectMessage to refresh CMD window

   IF `str` == NULL
     Place to buffer last response string
*/
void Connection::AddCmdLine(LPCSTR str)
{
	int  n;
	char buff[512];

	if(str)
	{
		str = FP_GetMsg(str);

		//Skip IAC
		if(((BYTE)str[0]) == ffIAC && ((BYTE)str[1]) == ffIP)
			return;

		//Skip empty
		while(*str && strchr("\n\r",*str) != NULL) str++;

		if(!str[0])
			return;
	}

	StrCpy(buff, str ? str : reply_string.c_str(), ARRAYSIZE(buff));

//Remove overflow lines
	for(; cmdCount >= cmdSize; cmdCount--)
		for(n = 1; n < cmdCount; n++)
		{
			StrCpy(CmdBuff[n-1], CmdBuff[n], cmdLineSize);
			StrCpy(RplBuff[n-1], RplBuff[n], 1024);
		}

//Add new line
	LogCmd(buff, (str != NULL) ? ldOut : ldIn);
	SetCmdLine(CmdBuff[cmdCount], buff, cmdLineSize, str != NULL);
	SetCmdLine(RplBuff[cmdCount], buff, 1024,        str != NULL);
	cmdCount++;

//Start
	if(!StartReply.Length() && !str)
	{
		LPCSTR m = reply_string.c_str();

		while(*m && (isdigit(*m) || strchr(" \b\t", *m) != NULL))
			m++;

		StartReply = m;
		ToOEM(StartReply);
	}

	ConnectMessage();
}
//------------------------------------------------------------------------
/*
    Show CMD window
    Use `CmdBuff` string to write in window
    Returns nonzero if `btn` to specified or user select button from modal dialog

    IF `btn` != MNone__
      Show modal message with button `btn`
    IF `btn` < 0
      Show modal message with button Abs(`btn`) and error color of message window
    IF `HostName` != NULL
      Set LastHost to this value
*/
static WORD Keys[] = { VK_ESCAPE, 'C', 'R', VK_RETURN };

BOOL Connection::ConnectMessageTimeout(int Msg /*= MNone__*/,LPCSTR HostName /*= NULL*/,int BtnMsg /*= MNone__*/)
{
	char              str[MAX_PATH];
	char              host[MAX_PATH];
	BOOL              rc,
	   first = TRUE;
	DWORD         b,e;
	double     diff;
	int               secNum;

	if(IS_FLAG(FP_LastOpMode,OPM_FIND) ||
	        !Opt.RetryTimeout ||
	        (BtnMsg != -MRetry && BtnMsg != -MRestore))
		return ConnectMessage(Msg, HostName, BtnMsg);

	StrCpy(host, LastHost, ARRAYSIZE(host));
	secNum = 0;
	GET_TIME(b);

	do
	{
		switch(CheckForKeyPressed(Keys,ARRAYSIZE(Keys)))
		{
			case 1:
			case 2:
				SetLastError(ERROR_CANCELLED);
				return FALSE;
			case 3:
			case 4:
				return TRUE;
		}

		GET_TIME(e);
		diff = CMP_TIME(e,b);

		if(first || diff > 1.0)
		{
			first = FALSE;
			_snprintf(str, ARRAYSIZE(str), "\"%s\" %s %2d%s %s",
			          HostName,
			          FP_GetMsg(MAutoRetryText),
			          Opt.RetryTimeout-secNum,
			          FP_GetMsg(MSeconds),FP_GetMsg(MRetryText));
			ConnectMessage(Msg,str);
			_snprintf(str, ARRAYSIZE(str), "%s %s %d%s",
			          FP_GetMsg(Msg),
			          FP_GetMsg(MAutoRetryText), Opt.RetryTimeout-secNum, FP_GetMsg(MSeconds));
			SaveConsoleTitle::Text(str);
			secNum++;
			b = e;
		}

		if(secNum > Opt.RetryTimeout)
		{
			rc = TRUE;
			break;
		}

		Sleep(100);
	}
	while(true);

	StrCpy(LastHost, host, ARRAYSIZE(LastHost));
	return rc;
}

int Connection::ConnectMessage(int Msg /*= MNone__*/,LPCSTR HostName /*= NULL*/,
                               int btn /*= MNone__*/,int btn1 /*= MNone__*/,int btn2 /*= MNone__*/)
{
	//PROC(( "ConnectMessage", "%d,%s,%d,%d,%d", Msg, HostName, btn, btn1, btn2 ))
	int               num,n;
	BOOL              res;
	LPCSTR          m;
	BOOL              exCmd;

	if(btn < 0 && Opt.DoNotExpandErrors)
		exCmd = FALSE;
	else
		exCmd = Host.ExtCmdView;

	//Called for update CMD window but it disabled
	if(Msg == MNone__ && !HostName && !exCmd)
		return TRUE;

	if(Msg != MNone__)          SetCmdLine(LastMsg,  FP_GetMsg(Msg), ARRAYSIZE(LastMsg),  -1);

	if(HostName && HostName[0]) SetCmdLine(LastHost, HostName,       ARRAYSIZE(LastHost), -1);

	if(btn == MNone__)
		if(IS_FLAG(FP_LastOpMode,OPM_FIND)                       ||    //called from find
		        !CmdVisible                                           ||  //Window disabled
		        (IS_SILENT(FP_LastOpMode) && !Opt.ShowSilentProgress))    //show silent processing disabled
		{
			if(HostName)
			{
				SetLastError(ERROR_SUCCESS);
				IdleMessage(HostName, Opt.ProcessColor);
			}

			return FALSE;
		}

#define ADD_CMD(v)  do{ Assert( num < (cmdSize+NBUTTONSADDON) ); CmdMsg[num++] = v; }while(0)
	//First line
	num = 0;
	//Title
	String str;
	m = FP_GetMsg(MFtpTitle);

	if(hostname[0])
	{
		if(UserPassword[0])
			str.printf("%s \"%s:*@%s\"",m,UserName,hostname);
		else
			str.printf("%s \"%s:%s\"",m,UserName,hostname);
	}
	else
		str = m;

	str.SetLength(cmdLineSize);
	ADD_CMD(str.c_str());

	//Error delimiter
	if(btn != MNone__ && btn < 0)
		switch(GetLastError())
		{
			case   ERROR_SUCCESS:
			case ERROR_CANCELLED:
				break;
			default:
				ADD_CMD("\x1");
		}

	if(GetLastError() == ERROR_CANCELLED)
		SetLastError(ERROR_SUCCESS);

	//Commands
	if(exCmd)
		for(n = 0; n < cmdCount; n++)
			ADD_CMD(CmdBuff[n]);

	//Message
	String msg;

	if(Msg != MOk && LastMsg[0])
	{
		if(exCmd && cmdCount)
			ADD_CMD("\x1");

		if(exCmd)
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			msg.printf("%02d:%02d:%02d \"%s\"", st.wHour, st.wMinute, st.wSecond, LastMsg);
		}
		else
			msg = LastMsg;

		msg.SetLength(cmdLineSize);
		ADD_CMD(msg.c_str());
		Log(("CMSG: %s", LastMsg));
	}

	//Host
	String lh;

	if(LastHost[0] && (!HostName || HostName[0]))
	{
		lh = LastHost;
		lh.SetLength(cmdLineSize);
		ADD_CMD(lh.c_str());
	}

	//Buttons
#define ADD_BTN(v)  do{ ADD_CMD(v); btnAddon++; }while(0)
	int btnAddon = 0;

	if(btn != MNone__)
	{
		ADD_CMD("\x1");
		ADD_BTN(FP_GetMsg(Abs(btn)));

		if(btn1 != MNone__) ADD_BTN(FP_GetMsg(btn1));

		if(btn2 != MNone__) ADD_BTN(FP_GetMsg(btn2));

		if(btn < 0 && btn != -MOk)
			ADD_BTN(FP_GetMsg(MCancel));
	}

	//Display error in title
	if(btn < 0 && btn != MNone__ && FP_Screen::isSaved())
	{
		char errStr[MAX_PATH];
		_snprintf(errStr,ARRAYSIZE(errStr),"%s \"%s\"",LastMsg,LastHost);
		SaveConsoleTitle::Text(errStr);
	}

	//Message
	BOOL isErr = (btn != MNone__ || btn < 0) && GetLastError() != ERROR_SUCCESS,
	     isWarn = btn != MNone__ && btn < 0;
	res = FMessage((isErr ? FMSG_ERRORTYPE : 0) | (isWarn ? FMSG_WARNING : 0) | (exCmd ? FMSG_LEFTALIGN : 0),
	               NULL, CmdMsg, num,
	               btnAddon);
	Log(("CMSG: rc=%d", res));
	//Del auto-added `cancel`
	btnAddon -= btn < 0 && btn != -MOk;

	//If has user buttons: return number of pressed button
	if(btnAddon > 1)
		return res;

	//If single button: return TRUE if button was selected
	return btn != MNone__ && res == 0;
}

void WINAPI OperateHidden(LPCSTR fnm, BOOL set)
{
	if(!Opt.SetHiddenOnAbort) return;

	Log(("%s hidden", set ? "Set" : "Clr"));
	DWORD dw = GetFileAttributes(fnm);

	if(dw == MAX_DWORD) return;

	if(set)
		SET_FLAG(dw, FILE_ATTRIBUTE_HIDDEN);
	else
		CLR_FLAG(dw, FILE_ATTRIBUTE_HIDDEN);

	SetFileAttributes(fnm, dw);
}
