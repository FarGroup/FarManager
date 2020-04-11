#ifndef __FAR_PLUGIN_CONNECT_CONNECTIONS
#define __FAR_PLUGIN_CONNECT_CONNECTIONS

struct Connection;

struct comvars
{
	int          connect;
	char         name[ MAXHOSTNAMELEN ];
	sockaddr_in  mctl;
	sockaddr_in  hctl;
	SOCKET       in;
	SOCKET       out;
	ftTypes      tpe;
	int          cpnd;
	int          sunqe;
	int          runqe;
};

/*
 * Format of command table.
 */
struct cmd
{
	const char *c_name;   /* name of command */
	BYTE  c_conn;         /* must be connected to use command */
	BYTE  c_proxy;        /* proxy server may execute */
	BYTE  c_args;         /* minimal parameters number */
};

//[ftp_Connect.cpp]
extern cmd cmdtabdata[];

struct FFtpCacheItem
{
	char  DirName[1024];
	char *Listing;
	int   ListingSize;
};

struct ConnectionState
{
	BOOL      Inited;
	int       Blocked;
	int       RetryCount;
	int       TableNum;
	int       Passive;
	HANDLE    Object;

	ConnectionState(void)
	{
		Inited = FALSE;
	}
};

struct Connection
{
		void       ExecCmdTab(struct cmd *c,int argc,char *argv[]);
		void       Gcat(char *s1, char *s2);
		void       Gmatch(char *s, char *p);
		void       abortpt();
		void       account(int argc,char **argv);
		void       acollect(char *as);
		void       addpath(char c);
		int        amatch(char *s, char *p);
		int        any(int c, char *s);
		char**     blkcpy(char **oav, char **bv);
		int        blklen(char **av);
		void       cd(int argc, char *argv[]);
		void       cdup();
		void       collect(char *as);
		char**     copyblk(char **v);
		SOCKET     dataconn(void);
		void       deleteFile(int argc, char *argv[]);
		int        digit(char c);
		void       disconnect();
		void       do_chmod(int argc, char *argv[]);
		void       do_umask(int argc, char *argv[]);
		void       doproxy(int argc, char *argv[]);
		int        empty(struct fd_set *mask, int sec);
		int        execbrc(char *p, char *s);
		void       expand(char *as);
		void       get(int argc, char *argv[]);
		cmd       *getcmd(char *name);
		int        gethdir(char *home);
		int        getit(int argc, char *argv[], int restartit, const char *mode);
		int        getreply(BOOL expecteof, DWORD tm = MAX_DWORD);
		void       ginit(char **agargv);
		BOOL       hookup(char *host, int port);
		void       idle(int argc, char *argv[]);
		BOOL       initconn();
		int        letter(char c);
		int        login(void);
		void       lostpeer();
		void       ls(int argc, char *argv[]);
		void       makeargv();
		void       makedir(int argc, char *argv[]);
		int        match(char *s, char *p);
		void       matchdir(char *pattern);
		void       modtime(int argc, char *argv[]);
		void       newer(int argc, char *argv[]);
		void       proxtrans(char *cmd, char *local, char *remote);
		void       pswitch(int flag);
		void       put(int argc, char *argv[]);
		void       pwd();
		void       quit();
		void       quote(int argc, char *argv[]);
		void       recvrequestINT(char *cmd, char *local, char *remote, const char *mode);
		void       recvrequest(char *cmd, char *local, char *remote, const char *mode);
		void       reget(int argc, char *argv[]);
		void       removedir(int argc, char *argv[]);
		void       renamefile(int argc, char *argv[]);
		void       reset();
		void       restart(int argc, char *argv[]);
		void       rmthelp(int argc, char *argv[]);
		void       rmtstatus(int argc, char *argv[]);
		void       rscan(char **t, int (*f)());
		void       sendrequestINT(char *cmd, char *local, char *remote);
		void       sendrequest(char *cmd, char *local, char *remote);
		void       setcase();
		int        setpeer(int argc, char *argv[]);
		void       setport();
		void       setrunique();
		void       setsunique();

		BOOL       setascii();
		BOOL       setbinary();
		BOOL       setebcdic();
		BOOL       settenex();
		BOOL       settype(ftTypes Mode,LPCSTR Arg);

		void       site(int argc, char *argv[]);
		void       sizecmd(int argc, char *argv[]);
		const char *slurpstring();
		char*      strend(char *cp);
		char*      strspl(char *cp, char *dp);
		void       syst();
		int        user(int argc, char **argv);

		int        nb_waitstate(SOCKET *peer, int state,DWORD tm = MAX_DWORD);
		BOOL       nb_connect(SOCKET *peer, struct sockaddr FAR* addr, int addrlen);
		int        nb_recv(SOCKET *peer, LPVOID buf, int len, int flags,DWORD tm = MAX_DWORD);
		int        nb_send(SOCKET *peer, LPCVOID buf, int len, int flags);
		int        fgetcSocket(SOCKET s,DWORD tm = MAX_DWORD);
		BOOL       fprintfSocket(SOCKET s, LPCSTR format, ...);
		BOOL       fputsSocket(LPCSTR format, SOCKET s);

		BOOL       SetType(int type);
		void       ResetOutput();
		void       AddOutput(BYTE *Data,int Size);

	protected:
		int            cmdLineSize;
		int            cmdSize;
		String         StartReply;
		String         reply_string;
		char *      *CmdBuff;               //[cmdSize+1][cmdLineSize+1]
		char *      *RplBuff;               //[cmdSize+1][1024+1]
		const char*     *CmdMsg;
		int            cmdCount;
		char           LastHost[MAX_PATH];
		char           LastMsg[MAX_PATH];
		char          *IOBuff;
		HANDLE         hIdle;
	public:
		comvars       proxstruct, tmpstruct;
		sockaddr_in   hisctladdr;
		sockaddr_in   data_addr;
		BOOL          brk_flag;
		sockaddr_in   myctladdr;
		__int64       restart_point;
		SOCKET        cin, cout;
		char          hostname[MAX_PATH];
		int           slrflag;
		int           portnum;

		/* Lot's of options... */
		/*
		 * Options and other state info.
		 */
		int           sendport;   /* use PORT cmd for each data connection */
		int           proxy;      /* proxy server connection active */
		int           proxflag;   /* proxy connection exists */
		int           sunique;    /* store files on server with unique name */
		int           runique;    /* store local files with unique name */
		int           code;       /* return/reply code for ftp command */
		char          pasv[64];   /* passive port for proxy data connection */
		char         *altarg;     /* argv[1] with no shell-like preprocessing  */

		ftTypes       type;          /* file transfer type */
		int           stru;          /* file transfer structure */
		char          bytename[32];  /* local byte size in ascii */
		int           bytesize;      /* local byte size in binary */

		String        line;          /* input line buffer */
		char         *stringbase;    /* current scan point in line buffer */
		String        argbuf;        /* argument storage buffer */
		char         *argbase;       /* current storage point in arg buffer */
		int           margc;         /* count of arguments on input line */
		char         *margv[20];     /* args parsed from input line */
		int           cpend;         /* flag: if != 0, then pending server reply */
		int           mflag;         /* flag: if != 0, then active multi command */

		FFtpCacheItem ListCache[16];
		int           ListCachePos;
		char          UserName[FAR_MAX_NAME];
		char          UserPassword[FAR_MAX_NAME];
		BYTE         *Output;
		int           OutputSize;
		int           OutputPos;
		int           LastUsedTableNum;
		int           connected;  /* connected to server */
		SOCKET        cmd_peer,
		     data_peer;
		int           SocketError;
		String        CurDir;
		char          SystemInfo[512];
		int           SystemInfoFilled;
		int           TableNum;
		int           ErrorCode;
		BOOL          SysError;
		BOOL          Breakable;
		FTPHostPlugin Host;
		FTPCurrentStates CurrentState;
		char           DirFile[MAX_PATH];
		int            RetryCount;
		FTPProgress*   TrafficInfo;
		BOOL           CmdVisible;
		BOOL           ResumeSupport;
		BOOL           IOCallback;
		//Completitions
		BOOL           LoginComplete;









	protected:
		void           InternalError(void);
		void           CloseCmdBuff(void);
		void           CloseIOBuff(void);
		void           ResetCmdBuff(void);
		void           SetCmdLine(char *dest,LPCSTR src,int sz,int out);
		BOOL           SendAbort(SOCKET din);
	public:
		Connection();
		~Connection();

		void           InitData(FTPHost* p,int blocked /*=TRUE,FALSE,-1*/);
		void           InitCmdBuff(void);
		void           InitIOBuff(void);
		BOOL           Init(LPCSTR Host,LPCSTR Port,LPCSTR User,LPCSTR Password);

		int            command(const char *fmt, ...);
		int            ProcessCommand(LPCSTR LineToProcess);
		int            ProcessCommand(String& s)
		{
			return ProcessCommand(s.c_str());
		}
		void           CheckResume(void);
		void           AbortAllRequest(int brkFlag);

		void           GetOutput(String& s);
		void           GetReply(BYTE *Line,int MaxLength)
		{
			StrCpy((char*)Line, reply_string.c_str(), MaxLength);
		}
		void           GetReply(String& s)
		{
			s = reply_string;
		}
		LPCSTR       GetStartReply(void)
		{
			return StartReply.c_str();
		}

		void           CacheReset();
		int            CacheGet();
		void           CacheAdd();

		void           SetTable(int Table)
		{
			TableNum = Table;
		}
		int            GetTable(void)
		{
			return TableNum;
		}

		int            FromOEM(BYTE *Line,int sz = -1, int fsz = -1);
		int            FromOEM(char *Line)
		{
			return FromOEM((LPBYTE)Line,-1,-1);
		}
		int            FromOEM(String& s)
		{
			s.Alloc(s.Length()*3+1);
			int ret = FromOEM((LPBYTE)s.c_str(),-1,s.Length()*3);
			s.SetLength(ret);
			return ret;
		}

		int            ToOEM(BYTE *Line,int sz = -1);
		int            ToOEM(char *Line)
		{
			return ToOEM((LPBYTE)Line,-1);
		}
		int            ToOEM(String& s)
		{
			int ret = ToOEM((LPBYTE)s.c_str(),-1);
			s.SetLength(ret);
			return ret;
		}

		char          *FromOEMDup(LPCSTR str,int num = 0);
		char          *ToOEMDup(LPCSTR str,int num = 0);
		String         SFromOEM(LPCSTR str);
		String         SToOEM(LPCSTR str);
		String         SFromOEM(const String& str)
		{
			return FromOEMDup(str.c_str());
		}
		String         SToOEM(const String& str)
		{
			return ToOEMDup(str.c_str());
		}

		void           GetState(ConnectionState* p);
		void           SetState(ConnectionState* p);

		BOOL           GetExitCode();
		int            GetResultCode(void)
		{
			return code;
		}
		int            GetErrorCode(void)
		{
			return ErrorCode;
		}
		BOOL           SysErr(void)
		{
			return SysError;
		}

		void           AddCmdLine(LPCSTR str);
		int            ConnectMessage(int Msg = MNone__,LPCSTR HostName = NULL,int BtnMsg = MNone__,int btn1 = MNone__, int btn2 = MNone__);
		BOOL           ConnectMessageTimeout(int Msg = MNone__,LPCSTR HostName = NULL,int BtnMsg = MNone__);
};


#endif
