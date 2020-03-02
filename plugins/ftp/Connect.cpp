#include <all_far.h>
#pragma hdrstop

#include "Int.h"

cmd cmdtabdata[] =
{
	/*00*/{ "account",    1,  1, 0 },  //  [...]
	/*01*/{ "append",     1,  1, 2 },  //  <local> [<remote>]
	/*02*/{ "ascii",      1,  1, 0 },  //
	/*03*/{ "binary",     1,  1, 0 },  //
	/*04*/{ "bye",        0,  0, 0 },  //
	/*05*/{ "cd",         1,  1, 1 },  //  <path>
	/*06*/{ "cdup",       1,  1, 0 },  //
	/*07*/{ "chmod",      1,  1, 2 },  //  <file> <mode>
	/*08*/{ "close",      1,  1, 0 },  //
	/*09*/{ "delete",     1,  1, 1 },  //  <file>
	/*10*/{ "dir",        1,  1, 0 },  //  [<path>]
	/*11*/{ "disconnect", 1,  1, 0 },  //
	/*12*/{ "get",        1,  1, 1 },  //  <remote> [<local>]
	/*13*/{ "idle",       1,  1, 1 },  //  <time>
	/*14*/{ "image",      1,  1, 0 },  //
	/*15*/{ "ls",         1,  1, 0 },  //  [<path>]
	/*16*/{ "mkdir",      1,  1, 1 },  //  <dir>
	/*17*/{ "modtime",    1,  1, 1 },  //  <file> (get last modification time)
	/*18*/{ "newer",      1,  1, 1 },  //  <remote> [<local>] (get files only if new)
	/*19*/{ "nlist",      1,  1, 0 },  //  [<path>]
	/*20*/{ "open",       0,  1, 1 },  //  <site> [<port> [<user> [<pwd>]]]
	/*21*/{ "proxy",      0,  1, 1 },  //  <command> [<cmd params>] (exec cmd into proxy mode; tempror switch to proxy if not yet)
	/*22*/{ "sendport",   0,  0, 0 },  //
	/*23*/{ "put",        1,  1, 1 },  //  <local> [<remote>]
	/*24*/{ "pwd",        1,  1, 0 },  //
	/*25*/{ "quit",       0,  0, 0 },  //
	/*26*/{ "quote",      1,  1, 1 },  //  <command> [...]
	/*27*/{ "recv",       1,  1, 1 },  //  <remote> [<local>]
	/*28*/{ "reget",      1,  1, 1 },  //  <remote> [<local>]
	/*29*/{ "rstatus",    1,  1, 0 },  //  [<command>]
	/*30*/{ "rhelp",      1,  1, 0 },  //  [<command>]
	/*31*/{ "rename",     1,  1, 2 },  //  <old name> <new name>
	/*32*/{ "reset",      1,  1, 0 },  //  (read one server reply string)
	/*33*/{ "restart",    1,  1, 1 },  //  <restart point> (set internal restart offset)
	/*34*/{ "rmdir",      1,  1, 1 },  //  <dirname>
	/*35*/{ "runique",    0,  1, 0 },  //  (set internal `runique` flag)
	/*36*/{ "send",       1,  1, 1 },  //  <local> [<remote>]
	/*37*/{ "site",       1,  1, 1 },  //  <command> [...]
	/*38*/{ "size",       1,  1, 1 },  //  <file>
	/*39*/{ "system",     1,  1, 0 },  //  (exec SYST command)
	/*40*/{ "sunique",    0,  1, 0 },  //  (set internal `sunique` flag)
	/*41*/{ "user",       1,  1, 1 },  //  <user> [<pwd>] [<account command>]
	/*42*/{ "umask",      1,  1, 1 },  //  <umask>
	{ 0 },
};

Connection::Connection()
{
	cmdLineSize = 0;
	cmdSize = 0;
	CmdBuff = NULL;
	RplBuff = NULL;
	CmdMsg = NULL;
	cmdCount = 0;;
	*LastHost = 0;
	*LastMsg = 0;
	IOBuff = NULL;
	hIdle = NULL;
	memset(&proxstruct, 0, sizeof(proxstruct));
	memset(&tmpstruct, 0, sizeof(tmpstruct));
	brk_flag = FALSE;
	restart_point = 0;
	cin = 0;
	cout = 0;
	*hostname = 0;
	slrflag = 0;
	portnum = 0;

	sendport = 0;
	proxy = 0;
	proxflag = 0;
	sunique = 0;
	runique = 0;
	code = 0;
	*pasv = 0;
	altarg = NULL;

	stru = 0;
	*bytename=0;
	bytesize=0;

	stringbase = NULL;
	argbase = NULL;
	margc = 0;
	cpend = 0;
	mflag = 0;
	memset(ListCache,0,sizeof(ListCache));
	ListCachePos = 0;
	*UserName = 0;
	*UserPassword = 0;
	Output = NULL;
	OutputSize = 0;
	OutputPos = 0;
	LastUsedTableNum = 0;
	connected = 0;
	cmd_peer = 0;
	data_peer = 0;
	SocketError = 0;
	*SystemInfo = 0;
	SystemInfoFilled = 0;
	TableNum = 0;
	ErrorCode = 0;
	SysError = FALSE;
	Breakable = FALSE;
	*DirFile = 0;
	RetryCount = 0;
	TrafficInfo = NULL;
	CmdVisible = FALSE;
	ResumeSupport = FALSE;
	IOCallback = FALSE;
	LoginComplete = FALSE;


	SocketError      = (int)INVALID_SOCKET;
	sendport         = -1;
	cmd_peer         = INVALID_SOCKET;
	data_peer        = INVALID_SOCKET;
	LastUsedTableNum = 1;
	TrafficInfo      = new FTPProgress;
	CurrentState     = fcsNormal;
	Breakable        = TRUE;
	Host.ServerType  = FTP_TYPE_DETECT;
}

Connection::~Connection()
{
	PROC(("Connection::~Connection","%p",this))
	int LastError = GetLastError();
	ResetOutput();
	CacheReset();
	AbortAllRequest(0);
	SetLastError(LastError);
	CloseIOBuff();
	delete TrafficInfo;
	CloseCmdBuff();
}

void Connection::ExecCmdTab(struct cmd *c,int argc,char *argv[])
{
	PROC(("ExecCmdTab","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));

	for(size_t I=0; I < ARRAYSIZE(cmdtabdata); I++)
		if(c==&cmdtabdata[I])
		{
			switch(I)
			{
				case  0:
					account(argc,argv);
					break;
				case  1:

					if(argc>2) argv[2] = FromOEMDup(argv[2]);

					put(argc,argv);
					break;
				case  2:
					setascii();
					break;
				case  3:
					setbinary();
					break;
				case  4:
					quit();
					break;
				case  5:
					argv[1] = FromOEMDup(argv[1]);
					cd(argc,argv);
					break;
				case  6:
					cdup();
					break;
				case  7:
					do_chmod(argc,argv);
					break;
				case  8:
					disconnect();
					break;
				case  9:
					argv[1] = FromOEMDup(argv[1]);
					deleteFile(argc,argv);
					break;
				case 10:

					if(argc > 1)
						argv[1] = FromOEMDup(argv[1]);

					ls(argc,argv);
					break;
				case 11:
					disconnect();
					break;
				case 12:
					argv[1] = FromOEMDup(argv[1]);
					get(argc,argv);
					break;
				case 13:
					idle(argc,argv);
					break;
				case 14:
					setbinary();
					break;
				case 15:

					if(argc>1) argv[1] = FromOEMDup(argv[1]);

					ls(argc,argv);
					break;
				case 16:
					argv[1] = FromOEMDup(argv[1]);
					makedir(argc,argv);
					break;
				case 17:
					modtime(argc,argv);
					break;
				case 18:
					newer(argc,argv);
					break;
				case 19:

					if(argc>1) argv[1] = FromOEMDup(argv[1]);

					ls(argc,argv);
					break;
				case 20:
					setpeer(argc,argv);
					break;
				case 21:
					doproxy(argc,argv);
					break;
				case 22:
					setport();
					break;
				case 23:

					if(argc>2) argv[2] = FromOEMDup(argv[2]);

					put(argc,argv);
					break;
				case 24:
					pwd();
					break;
				case 25:
					quit();
					break;
				case 26:
					quote(argc,argv);
					break;
				case 27:
					argv[1] = FromOEMDup(argv[1]);
					get(argc,argv);
					break;
				case 28:
					argv[1] = FromOEMDup(argv[1]);
					reget(argc,argv);
					break;
				case 29:
					rmtstatus(argc,argv);
					break;
				case 30:
					rmthelp(argc,argv);
					break;
				case 31:

					if(argc>1) argv[1] = FromOEMDup(argv[1]);

					if(argc>2) argv[2] = FromOEMDup(argv[2],1);

					renamefile(argc,argv);
					break;
				case 32:
					reset();
					break;
				case 33:
					restart(argc,argv);
					break;
				case 34:
					argv[1] = FromOEMDup(argv[1]);
					removedir(argc,argv);
					break;
				case 35:
					setrunique();
					break;
				case 36:

					if(argc>2) argv[2] = FromOEMDup(argv[2]);

					put(argc,argv);
					break;
				case 37:
					site(argc,argv);
					break;
				case 38:
					argv[1] = FromOEMDup(argv[1]);
					sizecmd(argc,argv);
					break;
				case 39:
					syst();
					break;
				case 40:
					setsunique();
					break;
				case 41:
					user(argc,argv);
					break;
				case 42:
					do_umask(argc,argv);
					break;
			}

			break;
		}
}

String Connection::SFromOEM(LPCSTR str)
{
	String s(str);
	FromOEM(s);
	return s;
}

String Connection::SToOEM(LPCSTR str)
{
	String s(str);
	ToOEM(s);
	return s;
}

char *Connection::FromOEMDup(LPCSTR str,int num)
{
	static String nm[2];
	nm[num] = str;
	FromOEM(nm[num]);
	return nm[num].c_str();
}

char *Connection::ToOEMDup(LPCSTR str,int num)
{
	static String nm[2];
	nm[num] = str;
	ToOEM(nm[num]);
	return nm[num].c_str();
}

void Connection::GetState(ConnectionState* p)
{
	p->Inited     = TRUE;
	p->Blocked    = CmdVisible;
	p->RetryCount = RetryCount;
	p->TableNum   = TableNum;
	p->Passive    = Host.PassiveMode;
	p->Object     = TrafficInfo->Object;
	TrafficInfo->Object = NULL;
}

void Connection::SetState(ConnectionState* p)
{
	if(!p->Inited)
		return;

	CmdVisible           = p->Blocked;
	RetryCount           = p->RetryCount;
	TableNum             = p->TableNum;
	Host.PassiveMode     = p->Passive;
	TrafficInfo->Object  = p->Object;
	TrafficInfo->SetConnection(this);
}

void Connection::InitData(FTPHost* p,int blocked /*=TRUE,FALSE,-1*/)
{
	Host       = *p;
	Host.Size  = sizeof(Host);
	CmdVisible = TRUE;

	if(blocked != -1)
		CmdVisible = blocked == FALSE;
}

void Connection::AddOutput(BYTE *Data,int Size)
{
	if(Size==0) return;

	BYTE *NewOutput=(BYTE*)realloc(Output,OutputSize+Size+1);

	if(NewOutput==NULL)
	{
		/*-*/Log(("!allocate output buffer %d",OutputSize+Size+1));
		return;
	}

	Output=NewOutput;
	memcpy(Output+OutputSize,Data,Size);
	OutputSize+=Size;
}

void Connection::GetOutput(String& s)
{
	s.SetLength(0);

	if(OutputPos == 0)
	{
		if(Opt.LogOutput)
		{
			LogCmd(Message("--LISTING--(Current table: %d Last used: %d)--",TableNum,LastUsedTableNum), ldInt);
			LogCmd((LPCSTR)Output, ldRaw, OutputSize);
			LogCmd("--LISTING--", ldInt);
		}

		//ToOEM( Output,OutputSize );
	}

	while(OutputPos < OutputSize &&
	        (Output[OutputPos]=='\n' || Output[OutputPos]=='\r'))
		OutputPos++;

	if(OutputPos >= OutputSize)
		return;

	while(OutputPos < OutputSize)
	{
		BOOL eol = Output[OutputPos] == '\n' || Output[OutputPos] == '\r';

		if(eol)
		{
			while(OutputPos < OutputSize && strchr("\n\r",Output[OutputPos]) != NULL)
				OutputPos++;

			if(Host.ServerType != FTP_TYPE_VMS)
				break;

			if(Output[OutputPos] != ' ')
				break;
		}

		s.Add((char)Output[OutputPos++]);
	}
}

void Connection::ResetOutput()
{
	if(Output)
		free(Output);

	Output=NULL;
	OutputSize=0;
	OutputPos=0;
}


void Connection::CacheReset()
{
	for(size_t I=0; I<ARRAYSIZE(ListCache); I++)
	{
		if(ListCache[I].Listing)
			free(ListCache[I].Listing);

		ListCache[I].Listing=NULL;
		ListCache[I].ListingSize=0;
	}

	ListCachePos=0;
}


int Connection::CacheGet()
{
	if(DirFile[0])
	{
		HANDLE f = Fopen(DirFile,"r");

		if(f)
		{
			ResetOutput();
			OutputSize = (DWORD)Fsize(f);
			Output     = (BYTE*)malloc(OutputSize+1);

			if(!Output)
				return FALSE;

			Fread(f, Output, OutputSize);
			Fclose(f);
			return TRUE;
		}
	}

	for(size_t I=0; I < ARRAYSIZE(ListCache); I++)
		if(ListCache[I].ListingSize > 0 &&
		        CurDir.Cmp(ListCache[I].DirName))
		{
			ResetOutput();
			BYTE *NewOutput=(BYTE*)malloc(ListCache[I].ListingSize+1);

			if(NewOutput==NULL)
				return FALSE;

			Output     = NewOutput;
			OutputSize = ListCache[I].ListingSize;
			memcpy(Output,ListCache[I].Listing,OutputSize);
			Output[OutputSize] = 0;
			return TRUE;
		}

	return FALSE;
}


void Connection::CacheAdd()
{
	if(ListCache[ListCachePos].Listing)
		free(ListCache[ListCachePos].Listing);

	ListCache[ListCachePos].ListingSize = 0;
	ListCache[ListCachePos].Listing     = (char*)malloc(OutputSize+1);

	if(ListCache[ListCachePos].Listing ==NULL)
		return;

	ListCache[ListCachePos].ListingSize = OutputSize;
	memcpy(ListCache[ListCachePos].Listing, Output, OutputSize);
	ListCache[ListCachePos].Listing[OutputSize] = 0;
	StrCpy(ListCache[ListCachePos].DirName, CurDir.c_str(), ARRAYSIZE(ListCache[ListCachePos].DirName));
	ListCachePos++;

	if(ListCachePos >= (int)(ARRAYSIZE(ListCache)))
		ListCachePos=0;
}

/* Returns if error happen
*/
BOOL Connection::GetExitCode()
{
	static struct
	{
		int Code;
		int WCode;
	} FtpErrCodes[] =
	{
		{ 202, ERROR_CALL_NOT_IMPLEMENTED },
		{ 421, ERROR_INTERNET_CONNECTION_ABORTED },
		{ 451, 0 },
		{ 450, ERROR_IO_INCOMPLETE },
		{ 452, ERROR_DISK_FULL },
		{ 500, ERROR_BAD_COMMAND },
		{ 501, ERROR_BAD_COMMAND },
		{ 502, ERROR_CALL_NOT_IMPLEMENTED },
		{ 503, ERROR_BAD_COMMAND },
		{ 504, ERROR_CALL_NOT_IMPLEMENTED },
		{ 530, ERROR_INTERNET_LOGIN_FAILURE },
		{ 532, ERROR_ACCESS_DENIED },
		{ 550, ERROR_ACCESS_DENIED },
		{ 551, 0 },
		{ 552, ERROR_DISK_FULL },
		{ 553, 0 }
	};

	if(ErrorCode != ERROR_SUCCESS)
	{
		SetLastError(ErrorCode);
		return FALSE;
	}

	if(code == RPL_ERROR ||
	        code == RPL_TRANSFERERROR)
	{
		//SetLastError(ErrorCode);
		return FALSE;
	}

	for(size_t I=0; I < ARRAYSIZE(FtpErrCodes); I++)
		if(FtpErrCodes[I].Code == code)
		{
			if(FtpErrCodes[I].WCode)
				SetLastError(FtpErrCodes[I].WCode);

			return FALSE;
		}

	return TRUE;
}

int encode_UTF8(WCHAR *ws, int wsz, char *utf8s, int sz)
{
	int i = 0, iw = 0;
	char *s;
	s = utf8s;

	while(iw<wsz && *ws && i<sz)
	{
		if(*ws <= 0x007F)                   /* 1 octet */
		{
			*s++ = (char)*ws;
			i++;
		}
		else if(*ws <= 0x07FF)              /* 2 octets */
		{
			*s++ = 0xC0 | ((*ws >> 6) & 0x1F);
			*s++ = 0x80 | (*ws & 0x3F);
			i+=2;
		}
		else if(*ws <= 0xFFFF)              /* 3 octets */
		{
			*s++ = 0xE0 | ((*ws >> 12) & 0x0F);
			*s++ = 0x80 | ((*ws >> 6) & 0x3F);
			*s++ = 0x80 | (*ws & 0x3F);
			i+=3;
		}
		else                                /* >= 4 octets -- not fit in UCS-2 */
		{
			*s++ = '_';
			i++;
		}

		++ws;
		++iw;
	}

	iw = i;

	while(iw < sz)
	{
		*s++ = 0;
		iw++;
	}

	return i;
}

int Connection::FromOEM(BYTE *Line,int _sz,int fsz)
{
	CharTableSet TableSet;
	int          TabNum = LastUsedTableNum;
	int          sz = _sz;

	if(!Line || !sz) return 0;

	if(sz == -1)
		sz = static_cast<int>(strlen((char*)Line));
	else
		sz--;

	if(fsz == -1)
		fsz = sz;

	int ret = sz;

	if(TabNum == 2 ||
	        TabNum > 3 && FP_Info->CharTable(TabNum-4,(char*)&TableSet,sizeof(TableSet)) == -1)
	{
		LogCmd(Message("Not working decode table %d used !!",TabNum), ldInt);
		TabNum = 1;
	}

	switch(TabNum)
	{
		case 0:
			OemToCharBuff((LPCSTR)Line, (char *)Line, sz);
			break;
		case 1:
			break;
		case 3:
		{
			WCHAR *tmp = (WCHAR *)malloc(sz*sizeof(WCHAR));
			MultiByteToWideChar(CP_OEMCP, 0, (LPCSTR)Line, sz, tmp, sz);
			ret = encode_UTF8(tmp, sz, (char *)Line, fsz);
			free(tmp);
			break;
		}
		default:

			for(int I=0; I < sz; I++)
				Line[I] = (BYTE)TableSet.EncodeTable[ Line[I] ];

			break;
	}

	return ret;
}

int decode_UTF8(unsigned char *utf8s, int sz, WCHAR *ws, int wsz)
{
	int i = 0, iw = 0;
	WCHAR     *wc;
	wc = ws;

	while(*utf8s && i<sz && iw<wsz)
	{
		if(!(*utf8s & 0x80))
		{
			*wc = *utf8s++;                       /* 1 octet */
			i++;
		}
		else if((*utf8s & 0xE0) == 0xC0)        /* 2 octets */
		{
			*wc = (*utf8s++ & 0x1F) << 6;       /* 1st */
			i++;

			if((*utf8s & 0xC0) == 0x80)
			{
				*wc |= *utf8s++ & 0x3F;           /* 2nd */
				i++;
			}
			else
				*wc = *utf8s % 26 + 'a';
		}
		else if((*utf8s & 0xF0) == 0xE0)        /* 3 octets */
		{
			*wc = (*utf8s++ & 0x0F) << 12;      /* 1st */
			i++;

			if((*utf8s & 0xC0) == 0x80)         /* 2nd */
			{
				*wc |= (*utf8s++ & 0x3F) << 6;
				i++;

				if((*utf8s & 0xC0) == 0x80)
				{
					*wc |= *utf8s++ & 0x3F;       /* 3d */
					i++;
				}
				else
					*wc = *utf8s % 26 + 'a';      /* bad UTF-8 */
			}
			else *wc = *utf8s % 26 + 'a';       /* bad UTF-8 */
		}
		else if((*utf8s * 0xF0) == 0xF0)       /* >= 4 octets -- not fit in UCS-2 */
		{
			++utf8s;
			i++;

			while(*utf8s && ((*utf8s & 0xC0) == 0x80) && i<sz)
			{
				i++;
				++utf8s;
			}

			*wc =  '_';
		}
		else
		{
			*wc = *utf8s % 26 + 'a';
			++utf8s;
			i++;
		}

		++wc;
		iw++;
	}

	return iw;
}

int Connection::ToOEM(BYTE *Line,int _sz)
{
	CharTableSet TableSet;
	int          TabNum  = TableNum;
	int          sz = _sz;

	if(!Line || !sz) return 0;

	if(sz == -1)
		sz = static_cast<int>(strlen((char*)Line));
	else
		sz--;

	int ret = sz;

	if(TabNum > 3 && FP_Info->CharTable(TabNum-4,(char*)&TableSet,sizeof(TableSet)) == -1)
	{
		LogCmd(Message("Decode table set to %d, but does not exist. Switch to autodetect",TabNum), ldInt);
		TabNum = 2;
	}

	if(TabNum == 2)
	{
		TabNum = FP_Info->CharTable(FCT_DETECT,(char*)Line,sz);

		if(TabNum == -1 || FP_Info->CharTable(TabNum,(char*)&TableSet,sizeof(TableSet)) == -1)
			TabNum = 1;
		else
			TabNum = TabNum+4;

		LogCmd(Message("Autodetected table: %d",TabNum), ldInt);
		TableNum = TabNum;
	}

	switch(TabNum)
	{
		case 0:
			CharToOemBuff((LPCSTR)Line, (char *)Line, sz);
			break;
		case 1:
			break;
		case 3:
		{
			WCHAR *tmp = (WCHAR *)malloc(sz*sizeof(WCHAR));
			ret = decode_UTF8(Line, sz, tmp, sz);
			WideCharToMultiByte(CP_OEMCP, 0, tmp, ret, (LPSTR)Line, ret, NULL, NULL);

			for(int i=ret; i<sz; i++)
				Line[i] = 0;

			free(tmp);
			break;
		}
		default:

			for(int I=0; I < sz; I++)
				Line[I] = (BYTE)TableSet.DecodeTable[ Line[I] ];

			break;
	}

	LastUsedTableNum = TabNum;
	return ret;
}
