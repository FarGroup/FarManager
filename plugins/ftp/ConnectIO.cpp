#include <all_far.h>
#pragma hdrstop

#include "Int.h"

//--------------------------------------------------------------------------------
BOOL Connection::SetType(int type)
{
	switch(type)
	{
		case TYPE_A:
			return setascii();
		case TYPE_I:
			return setbinary();
		case TYPE_E:
			return setebcdic();
	}

	return FALSE;
}

void Connection::reset()
{
	struct fd_set mask;
	int           nfnd;

	do
	{
		FD_ZERO(&mask);
		FD_SET(cin, &mask);  /*!*/
		nfnd = empty(&mask,0);

		if(nfnd < 0)
		{
			Log(("!reset"));
			code = -1;
			lostpeer();
		}
		else if(nfnd)
			getreply(0);
	}
	while(nfnd > 0);
}

void Connection::AbortAllRequest(int BrkFlag)
{
	if(BrkFlag)
		brk_flag = TRUE;
	else
	{
		scClose(data_peer,-1);
		scClose(cmd_peer,-1);
	}
}

int Connection::empty(struct fd_set *mask, int sec)
{
	struct timeval t;
	t.tv_sec = (long) sec;
	t.tv_usec = 0;
	return select(FD_SETSIZE,mask,NULL,NULL,&t);
}

//--------------------------------------------------------------------------------
/*VARARGS1*/
int Connection::command(const char *fmt, ...)
{
	PROC(("Connection::command", "%s", fmt));
	va_list ap;
	int     r;
	String  buffer;

	if(!cout)
	{
		Log(("!control connection for command [%s]",fmt));
		code = -1;
		return (0);
	}

	va_start(ap, fmt);
	buffer.vprintf(fmt, ap);
	va_end(ap);
	buffer.cat("\r\n");

	if(!fputsSocket(buffer.c_str(), cout))
	{
		Log(("!puts"));
		lostpeer();
		code = -1;
		return RPL_ERROR;
	}

	cpend = 1;
	r = getreply(StrCmp(fmt,Opt.cmdQuit) == 0);
	Log(("reply rc=%d",r));
#if 0/*?*/

	while(r != RPL_ERROR &&
	        !brk_flag &&
	        ((StrCmp(fmt,Opt.cmdPwd)==0 && (reply_string.Chr('\"') == -1) ||
	          !isdigit(reply_string[0]) ||
	          !isdigit(reply_string[1]) ||
	          !isdigit(reply_string[2]) ||
	          !isspace(reply_string[3]))))
	{
		r = getreply(FALSE);
	}

#endif
	Log(("rc=%d",r));
	return r;
}

//--------------------------------------------------------------------------------
int Connection::getreply(BOOL expecteof, DWORD tm)
{
	int   c, n, dig;
	int   originalcode = 0,
	      continuation = 0,
	      pflag        = 0;
	char *pt = pasv;

	if(cin == INVALID_SOCKET)
	{
		Log(("gr: inv socket"));
		code = 421;
		return RPL_ERROR;
	}

	for(;;)
	{
		dig = n = code = 0;
		reply_string = "";

		//Read line
		while((c=fgetcSocket(cin,tm)) != '\n')
		{
			if(c == RPL_TIMEOUT)
			{
				return RPL_TIMEOUT;
			}

			if(!c)
			{
				Log(("gr conn closed"));
				lostpeer();
				code = 421;
				return RPL_ERROR;
			}

			if(!dig)
			{
				// handle telnet commands
				if(c == ffIAC)
				{
					switch(c = fgetcSocket(cin))
					{
						case ffWILL:
						case ffWONT:
							c = fgetcSocket(cin);
							fprintfSocket(cout, "%c%c%c",ffIAC,ffDONT,c);
							break;
						case ffDO:
						case ffDONT:
							c = fgetcSocket(cin);
							fprintfSocket(cout, "%c%c%c",ffIAC,ffWONT,c);
							break;
						default:
							break;
					}

					continue;
				}

				if(!c || c == ffEOF)
				{
					Log(("gr EOF: c: %d (%d)",c,expecteof));

					if(expecteof)
					{
						code = 221;
						return RPL_OK;
					}

					lostpeer();
					code = 421;
					return RPL_TRANSIENT;
				}
			}

			dig++;

			if(dig < 4 && isdigit(c))
				code = code * 10 + (c - '0');

			if(!pflag && code == 227)
				pflag = 1;

			if(dig > 4 && pflag == 1 && isdigit(c))
				pflag = 2;

			if(pflag == 2)
			{
				if(c != '\r' && c != ')')
					*pt++ = c;
				else
				{
					*pt = '\0';
					pflag = 3;
				}
			}

			if(dig == 4 && c == '-')
			{
				if(continuation)
					code = 0;

				continuation++;
			}

			if(n == 0)
				n = c;

			reply_string.Add((char)c);
		}

		AddCmdLine(NULL);

		if(continuation && code != originalcode)
		{
			//Log(( "Continue: Cont: %d Code: %d Orig: %d", continuation,code,originalcode ));
			if(originalcode == 0)
				originalcode = code;

			continue;
		}

		if(n != '1')
			cpend = 0;

		if(code == 421 || originalcode == 421)
		{
			//Log(( "Code=%d, Orig=%d -> lostpeer",code,originalcode ));
			lostpeer();
		}

		//Log(( "rc = %d", (n - '0') ));
		return (n - '0');
	}
}

//--------------------------------------------------------------------------------
void Connection::pswitch(int flag)
{
	struct comvars *ip, *op;

	if(flag)
	{
		if(proxy) return;

		ip = &tmpstruct;
		op = &proxstruct;
		proxy++;
	}
	else
	{
		if(!proxy) return;

		ip = &proxstruct;
		op = &tmpstruct;
		proxy = 0;
	}

	ip->connect = connected;
	connected = op->connect;

	if(hostname)
	{
		StrCpy(ip->name, hostname, ARRAYSIZE(ip->name) - 1);
		ip->name[strlen(ip->name)] = '\0';
	}
	else
		ip->name[0] = 0;

	StrCpy(hostname,op->name,ARRAYSIZE(hostname));
	ip->hctl = hisctladdr;
	hisctladdr = op->hctl;
	ip->mctl = myctladdr;
	myctladdr = op->mctl;
	ip->in = cin; // What the hell am I looking at...?
	cin = op->in;
	ip->out = cout; // Same again...
	cout = op->out;
	ip->tpe = type;
	type = op->tpe;

	if(!type)
		type = TYPE_A;

	ip->cpnd = cpend;
	cpend = op->cpnd;
	ip->sunqe = sunique;
	sunique = op->sunqe;
	ip->runqe = runique;
	runique = op->runqe;
}

//--------------------------------------------------------------------------------
void Connection::proxtrans(char *cmd, char *local, char *remote)
{
	int tmptype, oldtype = 0, secndflag = 0, nfnd;
	char *cmd2;
	struct fd_set mask;

	if(StrCmp(cmd,Opt.cmdRetr) != 0)
		cmd2 = Opt.cmdRetr;
	else
		cmd2 = runique ? Opt.cmdPutUniq : Opt.cmdStor;

	if(command(Opt.cmdPasv) != RPL_COMPLETE)
	{
		ConnectMessage(MProxyNoThird,NULL,-MOk);
		return;
	}

	tmptype = type;
	pswitch(0);

	if(!connected)
	{
		ConnectMessage(MNotConnected,NULL,-MOk);
		pswitch(1);
		code = -1;
		return;
	}

	if(type != tmptype)
	{
		oldtype = type;
		SetType(tmptype);
	}

	if(command("%s %s",Opt.cmdPort,pasv) != RPL_COMPLETE)
	{
		SetType(oldtype);
		pswitch(1);
		return;
	}

	if(command("%s %s", cmd, remote) != RPL_PRELIM)
	{
		SetType(oldtype);
		pswitch(1);
		return;
	}

	pswitch(1);
	secndflag++;

	if(command("%s %s", cmd2, local) != RPL_PRELIM)
		goto abort;

	getreply(0);
	pswitch(0);
	getreply(0);
	SetType(oldtype);
	pswitch(1);
	return;
abort:

	if(StrCmp(cmd,Opt.cmdRetr) != 0 && !proxy)
		pswitch(1);
	else if(StrCmp(cmd,Opt.cmdRetr) == 0 && proxy)
		pswitch(0);

	if(!cpend && !secndflag)     /* only here if cmd = "STOR" (proxy=1) */
	{
		if(command("%s %s", cmd2, local) != RPL_PRELIM)
		{
			pswitch(0);
			SetType(oldtype);

			if(cpend)
			{
				char msg[2];
				fprintfSocket(cout,"%c%c",ffIAC,ffIP);
				*msg = ffIAC;
				*(msg+1) = ffDM;

				if(nb_send(&cout,msg,2,MSG_OOB) != RPL_COMPLETE)
				{
					Log(("!send [%s:%d]",__FILE__ , __LINE__));
				}

				fprintfSocket(cout,"ABOR\r\n");
				FD_ZERO(&mask);
				FD_SET(cin, &mask); /*!*/ // Chris: Need to correct this

				if((nfnd = empty(&mask,10)) <= 0)
				{
					Log(("abort"));
					lostpeer();
				}

				if(getreply(0) != RPL_ERROR)
					getreply(0);
				else
				{
					lostpeer();
					return;
				}
			}
		}

		pswitch(1);
		return;
	}

	if(cpend)
	{
		char msg[2];
		fprintfSocket(cout,"%c%c",ffIAC,ffIP);
		*msg = ffIAC;
		*(msg+1) = ffDM;

		if(nb_send(&cout,msg,2,MSG_OOB) != RPL_COMPLETE)
		{
			Log(("!send [%s:%d]",__FILE__ , __LINE__));
		}

		fprintfSocket(cout,"ABOR\r\n");
		FD_ZERO(&mask);
		/*!*/
		FD_SET(cin, &mask); // Chris: Need to correct this...

		if((nfnd = empty(&mask,10)) <= 0)
		{
			if(nfnd < 0)
			{
				Log(("abort"));
			}

			lostpeer();
		}

		if(getreply(0) != RPL_ERROR)
			getreply(0);
		else
		{
			lostpeer();
			return;
		}
	}

	pswitch(!proxy);

	if(!cpend && !secndflag)     /* only if cmd = "RETR" (proxy=1) */
	{
		if(command("%s %s", cmd2, local) != RPL_PRELIM)
		{
			pswitch(0);
			SetType(oldtype);

			if(cpend)
			{
				char msg[2];
				fprintfSocket(cout,"%c%c",ffIAC,ffIP);
				*msg = ffIAC;
				*(msg+1) = ffDM;

				if(nb_send(&cout,msg,2,MSG_OOB) != RPL_COMPLETE)
				{
					Log(("!send [%s:%d]",__FILE__ , __LINE__));
				}

				fprintfSocket(cout,"ABOR\r\n");
				FD_ZERO(&mask);
				/*!*/
				FD_SET(cin, &mask); // Chris:

				if((nfnd = empty(&mask,10)) <= 0)
				{
					if(nfnd < 0)
					{
						Log(("abort"));
					}

					lostpeer();
				}

				if(getreply(0) != RPL_ERROR)
					getreply(0);
				else
				{
					lostpeer();
					return;
				}
			}

			pswitch(1);
			return;
		}
	}

	if(cpend)
	{
		char msg[2];
		fprintfSocket(cout,"%c%c",ffIAC,ffIP);
		*msg = ffIAC;
		*(msg+1) = ffDM;

		if(nb_send(&cout,msg,2,MSG_OOB) != RPL_COMPLETE)
		{
			Log(("!send [%s:%d]",__FILE__ , __LINE__));
		}

		fprintfSocket(cout,"ABOR\r\n");
		FD_ZERO(&mask);
		/*!*/
		FD_SET(cin, &mask);

		if((nfnd = empty(&mask,10)) <= 0)
		{
			if(nfnd < 0)
			{
				Log(("abort"));
			}

			lostpeer();
		}

		if(getreply(0) != RPL_ERROR)
			getreply(0);
		else
		{
			lostpeer();
			return;
		}
	}

	pswitch(!proxy);

	if(cpend)
	{
		FD_ZERO(&mask);
		/*!*/
		FD_SET(cin, &mask); // Chris:

		if((nfnd = empty(&mask,10)) <= 0)
		{
			if(nfnd < 0)
			{
				Log(("abort"));
			}

			lostpeer();
		}

		if(getreply(0) != RPL_ERROR)
			getreply(0);
		else
		{
			lostpeer();
			return;
		}
	}

	if(proxy)
		pswitch(0);

	SetType(oldtype);
	pswitch(1);
}
