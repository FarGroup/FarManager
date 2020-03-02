#include <all_far.h>
#pragma hdrstop

#include "Int.h"

BOOL Connection::Init(LPCSTR Host, LPCSTR Port, LPCSTR User, LPCSTR Password)
{
	FP_Screen _scr;
	PROC(("Connection::Init","[%s] [%s] [%s]",Host,User,Password))
	_fmode        = _O_BINARY; // This causes an error somewhere.
	SocketError   = (int)INVALID_SOCKET;
	SetLastError(ERROR_SUCCESS);
	/* Set up defaults for FTP. */
	type = TYPE_A;
	strcpy(bytename, "8"), bytesize = 8;
	cpend = 0;  /* no pending replies */
	proxy = 0;      /* proxy not active */
	ResetCmdBuff();
	char *argv[5];
	argv[0] = (char*)"open";
	argv[1] = (char*)Host;
	argv[2] = (char*)Port;
	argv[3] = (char*)User;
	argv[4] = (char*)Password;

	if(!setpeer(5, argv))
	{
		Log(("!setpeer"));
		return FALSE;
	}

	Log(("OK"));
	return TRUE;
}

//--------------------------------------------------------------------------------
BOOL Connection::hookup(char *host, int port)
{
	FP_Screen _scr;
	hostent *he;
	SOCKET   sock = INVALID_SOCKET;
	int      len = sizeof(myctladdr);
	Log(("Connecting: [%s]:%d",host,port));
	hostname[0] = 0;
	memset(&hisctladdr, 0, sizeof(hisctladdr));
	memset(&myctladdr,  0, sizeof(myctladdr));
	hisctladdr.sin_port = port;
	ConnectMessage(MResolving,host);
	hisctladdr.sin_addr.s_addr = inet_addr(host);

	do
	{
//Server addr
		if(hisctladdr.sin_addr.s_addr != INADDR_NONE)
		{
			hisctladdr.sin_family = AF_INET;
		}
		else if((he=gethostbyname(host)) != NULL)
		{
			memmove(&hisctladdr.sin_addr,he->h_addr,he->h_length);
			hisctladdr.sin_family = he->h_addrtype;
		}
		else
			break;

//Sock
		if(!scValid(sock=scCreate(hisctladdr.sin_family))) break;

//Connect
		ConnectMessage(MWaitingForConnect,NULL);

		if(!nb_connect(&sock,(sockaddr*)&hisctladdr,sizeof(hisctladdr)))
			break;

//Local addr
		if(getsockname(sock,(sockaddr*)&myctladdr,&len) != 0)
			break;

//Read startup message from server
		cin = cout = sock;
		StrCpy(hostname, host, ARRAYSIZE(hostname));
		portnum = port;
		ConnectMessage(MWaitingForResponse,NULL);
		int repl = getreply(0);

		if(repl > RPL_COMPLETE || repl == RPL_ERROR) break;

//OK
		Log(("Connected: %d",sock));
		cmd_peer = sock;
		return TRUE;
	}
	while(0);

//ERROR
	Log(("!connect"));
	scClose(sock);
	cin      = 0;
	cout     = 0;
	code     = -1;
	cmd_peer = INVALID_SOCKET;
	portnum  = 0;
	return FALSE;
}

//--------------------------------------------------------------------------------
int Connection::login(void)
{
	//char *acct=NULL;
	//int   aflag = 0;
	int   n;
	ConnectMessage(MSendingName);
	n = command("%s %s", Opt.cmdUser, *UserName ? UserName : "anonymous");

	if(n == RPL_CONTINUE)
	{
		ConnectMessage(MPasswordName);
		n = command("%s %s",Opt.cmdPass, *UserPassword ? UserPassword : Opt.DefaultPassword);
	}

	if(n == RPL_CONTINUE)
	{
		//aflag++;
		//acct = "";
		n = command("%s %s",Opt.cmdAcct,""/*acct*/);
	}

	if(n != RPL_COMPLETE)
	{
		SetLastError(ERROR_INTERNET_LOGIN_FAILURE);
		return 0;
	}

	LoginComplete = TRUE;
	/*
	  if (!aflag && acct != NULL)
	    command( "%s %s",Opt.cmdAcct,acct );
	*/
	return 1;
}

void Connection::CheckResume(void)
{
	int oldcode = code;

	if(Host.AsciiMode)
	{
		if(setascii() &&
		        command("%s 0",Opt.cmdRest) != RPL_ERROR)
		{
			if(code == 350)
				ResumeSupport = TRUE;
		}
	}
	else
	{
		if(setbinary() &&
		        command("%s 0",Opt.cmdRest) != RPL_ERROR)
		{
			if(code == 350)
				ResumeSupport = TRUE;
		}
	}

	code = oldcode;
}

//--------------------------------------------------------------------------------
SOCKET Connection::dataconn(void)
{
	PROC(("dataconn", NULL))
	struct sockaddr_in from;
	int                fromlen = sizeof(from);
	SOCKET             s;

	if(brk_flag)
		return INVALID_SOCKET;

	if(!Host.PassiveMode)
	{
		if(!nb_waitstate(&data_peer, ws_accept) ||
		        (s=scAccept(&data_peer, (struct sockaddr *) &from, &fromlen)) == INVALID_SOCKET)
		{
			scClose(data_peer,-1);
			return INVALID_SOCKET;
		}

		Log(("SOCK: accepted data %d -> %d",data_peer,s));
		closesocket(data_peer);
		data_peer = s;
	}
	else
	{
		if(data_peer == INVALID_SOCKET)
		{
			InternalError();
			return INVALID_SOCKET;
		}

		if(brk_flag ||
		        !nb_connect(&data_peer,(sockaddr*)&data_addr,sizeof(data_addr)))
		{
			scClose(data_peer,-1);
			return INVALID_SOCKET;
		}
	}

	return data_peer;
}

//--------------------------------------------------------------------------------
/*
 * Need to start a listen on the data channel
 * before we send the command, otherwise the
 * server's connect may fail.
 */
BOOL Connection::initconn()
{
	int result, len, tmpno = 0;
	int on = 1;
noport:
	data_addr = myctladdr;

	if(sendport)
		data_addr.sin_port = 0; /* let system pick one */

	scClose(data_peer,-1);

	if(brk_flag ||
	        !scValid(data_peer=socket(AF_INET,SOCK_STREAM,0)))
	{
		Log(("!socket"));

		if(tmpno)
			sendport = 1;

		ErrorCode = WSAGetLastError();
		return FALSE;
	}

	Log(("SOCK: created data %d",data_peer));
	/* $ VVM  Switch socket to nonblocking mode */
	u_long mode = 1;
	ioctlsocket(data_peer, FIONBIO, &mode);

	/* VVM $ */
	if(!sendport)
		if(setsockopt(data_peer, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0)
		{
			Log(("!setsockopt (reuse address)"));
			ErrorCode = WSAGetLastError();
			goto bad;
		}

	if(!Host.PassiveMode || !sendport)
	{
		if(bind(data_peer, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0)
		{
			Log(("!bind"));
			ErrorCode = WSAGetLastError();
			goto bad;
		}

		if(setsockopt(data_peer, SOL_SOCKET, SO_DEBUG, (char*)&on, sizeof(on)) < 0)
		{
			Log(("!setsockopt SO_DEBUG (ignored)"));
		}

		len = sizeof(data_addr);

		if(getsockname(data_peer, (struct sockaddr *)&data_addr, &len) < 0)
		{
			Log(("!getsockname"));
			ErrorCode = WSAGetLastError();
			goto bad;
		}

		if(listen(data_peer, 1) < 0)
		{
			Log(("!listen"));
			ErrorCode = WSAGetLastError();
			goto bad;
		}
	}

	if(sendport)
	{
		if(Host.PassiveMode)
			result = command(Opt.cmdPasv);
		else
			result =  command("%s %d,%d,%d,%d,%d,%d",
			                  Opt.cmdPort,
			                  data_addr.sin_addr.S_un.S_un_b.s_b1,
			                  data_addr.sin_addr.S_un.S_un_b.s_b2,
			                  data_addr.sin_addr.S_un.S_un_b.s_b3,
			                  data_addr.sin_addr.S_un.S_un_b.s_b4,
			                  (unsigned char)data_addr.sin_port,
			                  (unsigned char)(data_addr.sin_port>>8));

		if(/*2*/result == ERROR && sendport == -1)
		{
			sendport = 0;
			tmpno = 1;
			goto noport;
		}

		if(result != RPL_COMPLETE)
		{
			Log(("!Complete"));
			goto bad;
		}

		if(Host.PassiveMode)
		{
			char LastReply2[100];  //Not need to use String - PORT string is small.
			char *p;
			GetReply((BYTE*)LastReply2, sizeof(LastReply2));

			if(code != 227 || (p=strchr(LastReply2,'('))==NULL)
			{
				Log(("!Pasv port reply"));
				return FALSE;
			}

			unsigned a1, a2, a3, a4, p1, p2;
#define TOP(a) (a&~0xFF)

			if(sscanf(p, "(%u,%u,%u,%u,%u,%u)", &a1, &a2, &a3, &a4, &p1, &p2) !=6 ||
			        TOP(a1) || TOP(a2) || TOP(a3) || TOP(a4) || TOP(p1) || TOP(p2))
			{
				Log(("Bad psv port reply"));
				return FALSE;
			}

			data_addr.sin_addr.s_addr = (a4<<24) | (a3<<16) | (a2<<8) | a1;
			data_addr.sin_port        = (p2<<8) | p1;

			if(!data_addr.sin_addr.s_addr || data_addr.sin_addr.s_addr == INADDR_ANY) return 1;

			if(data_addr.sin_addr.s_addr != hisctladdr.sin_addr.s_addr)
			{
				char Msg[30];
				_snprintf(Msg, ARRAYSIZE(Msg),
				          "[%s -> %s]",
				          inet_ntoa(hisctladdr.sin_addr), inet_ntoa(data_addr.sin_addr));
				ConnectMessage(0,Msg,0);
			}
		}

		return TRUE;
	}

	if(tmpno)
		sendport = 1;

	return TRUE;
bad:
	scClose(data_peer,-1);

	if(tmpno)
		sendport = 1;

	return FALSE;
}
