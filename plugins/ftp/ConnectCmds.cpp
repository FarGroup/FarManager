#include <all_far.h>
#pragma hdrstop

#include "Int.h"

/*
 * Set transfer type.
 */
BOOL Connection::settype(ftTypes Mode,LPCSTR Arg)
{
	PROC(("settype","[%c,%s]",Mode,Arg));
	int comret;

	if(Arg)
		comret = command("TYPE %c %s", Mode, Arg);
	else
		comret = command("TYPE %c", Mode);

	if(comret == RPL_COMPLETE)
	{
		type = Mode;
		Log(("TYPE = %c",Mode));
		return TRUE;
	}
	else
		return FALSE;
}

BOOL Connection::setascii()
{
	return settype(TYPE_A,NULL);
}
BOOL Connection::setbinary()
{
	return settype(TYPE_I,NULL);
}
BOOL Connection::setebcdic()
{
	return settype(TYPE_E,NULL);
}

/*
 * Connect to peer server and auto-login, if possible.
 * 1      2       3       4
 * <site> [<port> [<user> [<pwd>]]]
 */
int Connection::setpeer(int argc, char *argv[])
{
	PROC(("setpeer","%d [%s,%s,%s,%s]",argc,(argc>1)?argv[1]:"nil",(argc>2)?argv[2]:"nil",(argc>3)?argv[3]:"nil",(argc>4)?argv[4]:"nil"));
	FP_Screen _scr;
	WORD      port;

	if(argc < 2)
	{
		code = -1;
		return FALSE;
	}

	if(!SocketStartup)
	{
		WORD    wVerReq = MAKEWORD(1,1);
		WSADATA WSAData;

		if(WSAStartup(wVerReq, &WSAData) == 0)
		{
			SocketStartup = TRUE;
		}
		else
		{
			SocketInitializeError = WSAGetLastError();
			SocketError   = SocketInitializeError;
			SetLastError(SocketInitializeError);
			return FALSE;
		}
	}

	//port
	if(argc > 2)
		port = htons(atoi(argv[2]));
	else
		port = 0;

	if(!port)
	{
		servent *sp = getservbyname("ftp", "tcp");

		if(!sp)
		{
			Log(("ftp: ftp/tcp: unknown service. Assume port number %d (default)",IPPORT_FTP));
			port = htons(IPPORT_FTP);
		}
		else
		{
			port = sp->s_port;
			Log(("ftp: port %08X",port));
		}
	}

	//Check if need to close connection
	if(connected)
	{
		if(StrCmpI(argv[1], hostname) != 0 ||
		        port != portnum)
			disconnect();
	}

	do
	{
		//Make connection
		if(!connected)
		{
			if(!hookup(argv[1], port))        //Open connection
			{
				code = -1;
				break;
			}

			connected = 1;
		}

		//Login
		UserName[0]     = 0;
		UserPassword[0] = 0;

		if(argc > 3) StrCpy(UserName, argv[3], ARRAYSIZE(UserName));

		if(argc > 4) StrCpy(UserPassword, argv[4], ARRAYSIZE(UserPassword));

		if(!login())
		{
			disconnect();
			break;
		}

		return TRUE;
	}
	while(0);

	return FALSE;
}

/*
 * Send a single file.
 */
void Connection::put(int argc, char *argv[])
{
	PROC(("put","%d [\"%s\",\"%s\",\"%s\"]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));
	char *cmd;

	if(argc == 2)
	{
		argc++;
		argv[2] = argv[1];
	}

	if(argc < 3)
	{
		code = -1;
		return;
	}

	cmd = (argv[0][0] == 'a')
	      ? Opt.cmdAppe                                   //APPEND
	      : ((sunique) ? Opt.cmdPutUniq : Opt.cmdStor);   //STORE or PUT
	sendrequest(cmd,argv[1],argv[2]);
	restart_point = 0;
}


void Connection::reget(int argc, char *argv[])
{
	PROC(("reget","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));
	getit(argc, argv, 1, "a+");
}

void Connection::get(int argc, char *argv[])
{
	PROC(("get","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));
	getit(argc, argv, 0, restart_point ? "r+" : "w");
}

/*
 * get file if modtime is more recent than current file
 */
void Connection::newer(int argc, char *argv[])
{
	PROC(("newer","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));
	getit(argc, argv, -1, "w");
}

/*
 * Receive one file.
 */
int Connection::getit(int argc, char *argv[], int restartit, const char *mode)
{
	PROC(("getit"," %d %s %d [%s,%s,%s]",restartit,mode,argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));

	if(argc == 2)
	{
		argc++;
		argv[2] = argv[1];
	}

	if(argc < 3)
	{
		code = -1;
		return (0);
	}

	if(restartit == -1)
		restart_point = -1;
	else if(restartit)
	{
		restart_point = Fsize(argv[2]);
		Log(("Restart from %I64u",restart_point));
	}

	recvrequest(Opt.cmdRetr, argv[2], argv[1], mode);
	restart_point = 0;
	return 0;
}


/*
 * Toggle PORT cmd use before each data connection.
 */
void Connection::setport()
{
	sendport = !sendport;
	code = sendport;
}


/*
 * Set current working directory
 * on remote machine.
 */
void Connection::cd(int argc, char *argv[])
{
	PROC(("cd","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));

	if(argc==2)
		if(command("%s %s",Opt.cmdCwd,argv[1]) >= RPL_COMPLETE && code == 500)
			command("%s %s",Opt.cmdXCwd,argv[1]);
}


/*
 * Delete a single file.
 */
void Connection::deleteFile(int argc, char *argv[])
{
	PROC(("deleteFile","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));

	if(argc < 2)
	{
		code = -1;
		return;
	}

	command("%s %s",Opt.cmdDel,argv[1]);
}


/*
 * Rename a remote file.
 */
void Connection::renamefile(int argc, char *argv[])
{
	PROC(("renamefile","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));

	if(argc < 3)
	{
		code = -1;
		return;
	}

	if(command("%s %s",Opt.cmdRen,argv[1]) == RPL_CONTINUE)
		command("%s %s",Opt.cmdRenTo,argv[2]);
}

/*
 * Get a directory listing
 * of remote files.
 */
void Connection::ls(int argc, char *argv[])
{
	PROC(("ls","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));

	if(argc < 2)
		argc++, argv[1] = NULL;

	if(argc < 3)
		argc++, argv[2] = (char *)"-";

	if(argc > 3)
	{
		code = -1;
		return;
	}

	if(argv[0][0] == 'n')
		recvrequest(Opt.cmdNList, argv[2], argv[1], "w");
	else if(!Host.ExtList)
		recvrequest(Opt.cmdList, argv[2], argv[1], "w");
	else
	{
		recvrequest(Host.ListCMD, argv[2], argv[1], "w");

		if(code > 500 && argv[1] != NULL)
			recvrequest(Opt.cmdList, argv[2], argv[1], "w");
	}
}


/*
 * Send new user information (re-login)
 */
int Connection::user(int argc, char **argv)
{
	PROC(("user","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));
	char acct[80];
	int n, aflag = 0;

	if(argc < 2 || argc > 4)
	{
		code = -1;
		return (0);
	}

	n = command("%s %s", Opt.cmdUser,argv[1]);

	if(n == RPL_CONTINUE)
	{
		if(argc < 3)
			argv[2] = UserPassword, argc++;

		n = command("%s %s",Opt.cmdPass,argv[2]);
	}

	if(n == RPL_CONTINUE)
	{
		if(argc < 4)
		{
			*acct=0;
			argv[3] = acct;
			argc++;
		}

		n = command("%s %s",Opt.cmdAcct,argv[3]);
		aflag++;
	}

	if(n != RPL_COMPLETE)
	{
		SetLastError(ERROR_INTERNET_LOGIN_FAILURE);
		return 0;
	}

	if(!aflag && argc == 4)
		command("%s %s",Opt.cmdAcct,argv[3]);

	return 1;
}

/*
 * Print working directory.
 */
void Connection::pwd()
{
	PROC(("pwd",NULL));

	if(command(Opt.cmdPwd) >= RPL_COMPLETE && code == 500)
	{
		Log(("Try XPWD [%s]",Opt.cmdXPwd));
		command(Opt.cmdXPwd);
	}
}

/*
 * Make a directory.
 */
void Connection::makedir(int argc, char *argv[])
{
	PROC(("makedir","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));

	if(argc < 2)
	{
		code = -1;
		return;
	}

	if(command("%s %s",Opt.cmdMkd,argv[1]) >= RPL_COMPLETE && code == 500)
		command("%s %s",Opt.cmdXMkd,argv[1]);
}

/*
 * Remove a directory.
 */
void Connection::removedir(int argc, char *argv[])
{
	PROC(("removedir","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));

	if(argc < 2)
	{
		code = -1;
		return;
	}

	if(command("%s %s",Opt.cmdRmd,argv[1]) >= RPL_COMPLETE && code == 500)
		command("%s %s",Opt.cmdXRmd,argv[1]);
}

/*
 * Send a line, verbatim, to the remote machine.
 */
void Connection::quote(int argc, char *argv[])
{
	PROC(("quote","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));
	int i;
	char buf[BUFSIZ];

	if(argc < 2)
	{
		code = -1;
		return;
	}

	strcpy(buf, argv[1]);

	for(i = 2; i < argc; i++)
	{
		strcat(buf, " ");
		strcat(buf, argv[i]);
	}

	if(command(buf) == RPL_PRELIM)
		while(getreply(0) == RPL_PRELIM);
}

/*
 * Send a SITE command to the remote machine.  The line
 * is sent almost verbatim to the remote machine, the
 * first argument is changed to SITE.
 */

void Connection::site(int argc, char *argv[])
{
	PROC(("site","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));
	int  i;
	char buf[BUFSIZ];

	if(argc < 2)
	{
		code = -1;
		return;
	}

	strcpy(buf, Opt.cmdSite);
	strcat(buf, " ");
	strcat(buf, argv[1]);

	for(i = 2; i < argc; i++)
	{
		strcat(buf, " ");
		strcat(buf, argv[i]);
	}

	if(command(buf) == RPL_PRELIM)
		while(getreply(0) == RPL_PRELIM);
}

void Connection::do_chmod(int argc, char *argv[])
{
	PROC(("do_chmod","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));

	if(argc != 3)
	{
		code = -1;
		return;
	}

	command("%s %s %s %s", Opt.cmdSite, Opt.cmdChmod, argv[1], argv[2]);
}

void Connection::do_umask(int argc, char *argv[])
{
	PROC(("do_umask","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));
	command(argc == 1 ? "%s %s" : "%s %s %s",
	        Opt.cmdSite,
	        Opt.cmdUmask,
	        argv[1]);
}

void Connection::idle(int argc, char *argv[])
{
	PROC(("idle","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));
	command(argc == 1 ? "%s %s" : "%s %s %s",
	        Opt.cmdSite,
	        Opt.cmdIdle,
	        argv[1]);
}

/*
 * Terminate session and exit.
 */
/*VARARGS*/
void Connection::quit()
{
	PROC(("quit",NULL));

	if(connected)
		disconnect();

	pswitch(1);

	if(connected)
		disconnect();
}


/*
 * Terminate session, but don't exit.
 */
void Connection::disconnect()
{
	PROC(("disconnect",NULL));

	if(!connected)
		return;

	command(Opt.cmdQuit);
	cout = 0;
	connected = 0;
	scClose(data_peer, -1);
}


void Connection::account(int argc,char **argv)
{
	PROC(("account","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));
	String  acct;
	const char *ap;

	if(argc > 1)
	{
		++argv;
		--argc;
		acct = *argv;

		while(argc > 1)
		{
			--argc;
			++argv;
			acct.cat(*argv);
		}

		ap = acct.c_str();
	}
	else
		ap=""; //    ap = getpass("Account:");

	command("%s %s",Opt.cmdAcct,ap);
}


void Connection::doproxy(int argc, char *argv[])
{
	PROC(("doproxy","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));
	struct cmd *c;

	if(argc < 2)
	{
		code = -1;
		return;
	}

	c = getcmd(argv[1]);

	//Not found
	if(c == (cmd*)-1 || c == 0)
	{
		SetLastError(ERROR_BAD_DRIVER_LEVEL);
		code = -1;
		return;
	}

	//Unsupported in proxy mode
	if(!c->c_proxy)
	{
		SetLastError(ERROR_BAD_NET_RESP);
		code = -1;
		return;
	}

	//Allready in proxy mode
	if(proxy > 0)
		ExecCmdTab(c, argc-1, argv+1);
	else
	{
		//Temp switch to proxy mode
		pswitch(1);

		if(c->c_conn && !connected)
		{
			pswitch(0);
			code = -1;
			return;
		}

		ExecCmdTab(c, argc-1, argv+1);
		proxflag = (connected) ? 1 : 0;
		pswitch(0);
	}
}

void Connection::setsunique()
{
	sunique = !sunique;
	code = sunique;
}


void Connection::setrunique()
{
	runique = !runique;
	code = runique;
}

/* change directory to parent directory */
void Connection::cdup()
{
	PROC(("cdup",NULL));

	if(command(Opt.cmdCDUp) >= RPL_COMPLETE && code == 500)
		command(Opt.cmdXCDUp);
}

/* restart transfer at specific point */
void Connection::restart(int argc, char *argv[])
{
	PROC(("restart","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));

	if(argc == 2)
		restart_point = atol(argv[1]);
}


/* show remote system type */
void Connection::syst()
{
	PROC(("syst",NULL));
	command(Opt.cmdSyst);
}


/*
 * get size of file on remote machine
 */
void Connection::sizecmd(int argc, char *argv[])
{
	PROC(("sizecmd","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));

	if(argc < 2)
	{
		code = -1;
		return;
	}

	command("%s %s",Opt.cmdSize,argv[1]);
}

/*
 * get last modification time of file on remote machine
 */
void Connection::modtime(int argc, char *argv[])
{
	PROC(("modtime","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));

	if(argc < 2)
	{
		code = -1;
		return;
	}

	if(command("%s %s",Opt.cmdMDTM,argv[1]) == RPL_COMPLETE)
	{
		int yy, mo, day, hour, min, sec;
		sscanf(reply_string.c_str(),
		       Opt.fmtDateFormat,
		       &yy, &mo, &day, &hour, &min, &sec);
	}
}

/*
 * show status on remote machine
 */
void Connection::rmtstatus(int argc, char *argv[])
{
	PROC(("rmtstatus","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));
	command(argc > 1 ? "%s %s" : "%s" ,Opt.cmdStat,argv[1]);
}

/*
 * Ask the other side for help.
 */
void Connection::rmthelp(int argc, char *argv[])
{
	PROC(("rmthelp","%d [%s,%s,%s]",argc,(argc>=1)?argv[0]:"nil",(argc>=2)?argv[1]:"nil",(argc>=3)?argv[2]:"nil"));
	command(argc == 1 ? "%s" : "%s %s",
	        Opt.cmdHelp,
	        argv[1]);
}
