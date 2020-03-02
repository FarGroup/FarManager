#include <all_far.h>
#pragma hdrstop

#include "Int.h"

/* ANY state */
BOOL FTP::ExecCmdLineANY(LPCSTR str, BOOL Prefix)
{
	BOOL             iscmd = ShowHosts || Prefix;
	FTPUrl           ui;
	QueueExecOptions op;

//Help
	if((iscmd && StrCmpI(str,"HELP") == 0) || StrCmpI(str,"?") == 0)
	{
		FP_Info->ShowHelp(FP_Info->ModuleName,"FTPCommandLineHelp",FHELP_SELFHELP);
		return TRUE;
	}
	else

//Add to queue
		if(iscmd && StrNCmpI(str,"QADD ",5) == 0)
		{
			UrlInit(&ui);

			for(str += 5; *str && isspace(*str); str++);

			if(*str)
			{
				ui.SrcPath = str;

				if(EditUrlItem(&ui))
					AddToQueque(&ui);
			}

			return TRUE;
		}
		else

//Add and execute
			if(iscmd && StrNCmpI(str,"XADD ",5) == 0)
			{
				UrlInit(&ui);

				for(str += 5; *str && isspace(*str); str++);

				if(*str)
				{
					ui.SrcPath = str;

					if(EditUrlItem(&ui))
						AddToQueque(&ui);
				}

				SetupQOpt(&op);

				if(QuequeSize &&
				        WarnExecuteQueue(&op))
				{
					ExecuteQueue(&op);

					if(QuequeSize) QuequeMenu();
				}

				return TRUE;
			}
			else

//Show queue
				if(iscmd && StrCmpI(str,"Q") == 0)
				{
					QuequeMenu();
				}
				else

//Execute queue
					if(iscmd && StrCmpI(str,"QX") == 0)
					{
						SetupQOpt(&op);

						if(QuequeSize &&
						        WarnExecuteQueue(&op))
						{
							ExecuteQueue(&op);

							if(QuequeSize) QuequeMenu();
						}

						return TRUE;
					}

	return FALSE;
}

/* HOSTS state */
BOOL FTP::ExecCmdLineHOST(LPCSTR str, BOOL Prefix)
{
//Exit
	if(StrCmpI(str,"EXIT") == 0 ||
	        StrCmpI(str,"QUIT") == 0)
	{
		CurrentState = fcsClose;
		FP_Info->Control(this,FCTL_CLOSEPLUGIN,NULL);
		return TRUE;
	}
	else

//Connect to ftp
		if(str[0] == '/' && str[1] == '/')
		{
			Host.Init();
			Host.SetHostName(Message("ftp:%s",str),NULL,NULL);
			FullConnect();
			return TRUE;
		}
		else

//Connect to http
			if(StrNCmpI(str, "HTTP://", 7) == 0)
			{
				Host.Init();
				Host.SetHostName(str,NULL,NULL);
				FullConnect();
				return TRUE;
			}
			else

//Change dir
				if(StrNCmpI(str,"CD ",3) == 0)
				{
					str += 3;
					return SetDirectory(str,0);
				}

	return FALSE;
}

#define FCMD_SINGLE_COMMAND 0
#define FCMD_FULL_COMMAND   1

#define FCMD_SHOW_MSG       0x0001
#define FCMD_SHOW_EMSG      0x0002

BOOL FTP::DoCommand(LPCSTR str, int type, DWORD flags)
{
	FP_Screen _scr;
	BOOL ext = hConnect->Host.ExtCmdView,
	     dex = Opt.DoNotExpandErrors;
	int  rc=0;
	char *m;
	hConnect->Host.ExtCmdView = TRUE;
	Opt.DoNotExpandErrors     = FALSE;

	if(hConnect->Host.DecodeCmdLine)
		m = hConnect->FromOEMDup(str);
	else
		m = (char*)str;

	//Process command types
	switch(type)
	{
		case FCMD_SINGLE_COMMAND:
			rc = hConnect->command("%s", m);

			if(rc == RPL_PRELIM)
				while((rc=hConnect->getreply(0)) == RPL_PRELIM);

			rc = rc == RPL_COMPLETE ||
			     rc == RPL_OK;
			break;
		case   FCMD_FULL_COMMAND: //Convert string quoting to '\x1'

			for(rc = 0; m[rc]; rc++)
				if(m[rc] == '\\')
					rc++;
				else if(m[rc] == '\"')
					m[rc] = '\x1';

			rc = hConnect->ProcessCommand(m);
			break;
	}

	if((rc && IS_FLAG(flags,FCMD_SHOW_MSG)) ||
	        (!rc && IS_FLAG(flags,FCMD_SHOW_EMSG)))
		hConnect->ConnectMessage(MOk,"",rc ? MOk : (-MOk));

	//Special process of known commands
	if(type == FCMD_SINGLE_COMMAND)
	{
		if(StrNCmpI(str,"CWD ",4) == 0)
		{
			ResetCache = TRUE;
		}
	}

	hConnect->Host.ExtCmdView = ext;
	Opt.DoNotExpandErrors     = dex;
	return rc;
}

/* FTP state */
BOOL FTP::ExecCmdLineFTP(LPCSTR str, BOOL Prefix)
{
	PROC(("FTP::ExecCmdLineFTP", "[%s],%d", str, Prefix))

	if(Prefix && *str == '/')
	{
		FTPHost tmp;
		tmp.Assign(&Host);

		if(StrCmp(str,"FTP:",4,FALSE) != 0)
			str = Message("ftp:%s",str);

		if(tmp.SetHostName(str,NULL,NULL))
		{
			if(Host.CmpConnected(&tmp))
			{
				SetDirectory(tmp.Home,0);
				return TRUE;
			}

			Host.Assign(&tmp);
			FullConnect();
			return TRUE;
		}
	}

//Switch to hosts
	if(StrCmpI(str,"TOHOSTS") == 0)
	{
		hConnect->disconnect();
		return TRUE;
	}

//DIRFILE
	if(StrNCmpI(str,"DIRFILE ",8) == 0)
	{
		for(str+=7; *str && *str == ' '; str++);

		if(*str)
			StrCpy(hConnect->DirFile, str, ARRAYSIZE(hConnect->DirFile));
		else
			hConnect->DirFile[0] = 0;

		return TRUE;
	}

//CMD
	if(StrNCmpI(str,"CMD ",4) == 0)
	{
		for(str+=4; *str && *str == ' '; str++);

		if(*str)
			DoCommand(str, FCMD_FULL_COMMAND, FCMD_SHOW_MSG|FCMD_SHOW_EMSG);

		return TRUE;
	}

	Log(("ProcessCmd=%d", Host.ProcessCmd));

	if(!Host.ProcessCmd)
		return FALSE;

//CD
	if(StrNCmpI(str,"CD ",3) == 0)
	{
		str+=3;

		if(*str)
		{
			SetDirectory(str,0);
			return TRUE;
		}
	}

//Manual command line to server
	if(!Prefix)
	{
		DoCommand(str, FCMD_SINGLE_COMMAND, FCMD_SHOW_MSG|FCMD_SHOW_EMSG);
		return TRUE;
	}

	return FALSE;
}

BOOL FTP::ExecCmdLine(LPCSTR _str, BOOL WasPrefix)
{
	PROC(("ExecCmdLine","%s",_str))
	String    buff;
	BOOL      isConn = hConnect && hConnect->connected;
	FP_Screen _scr;

	if(!_str || !_str[0]) return FALSE;

//Trim spaces
	//Remove start
	while(*_str && isspace(*_str)) _str++;

	if(!_str[0]) return FALSE;

//Split command
	BOOL Prefix = StrCmp(_str,"FTP:",4,FALSE) == 0;

	if(Prefix) _str += 4;
	else Prefix = WasPrefix;

	buff = _str;

	do
	{
		//Any state
		if(ExecCmdLineANY(buff.c_str(),Prefix)) break;

		//HOSTS state
		if(ShowHosts &&
		        ExecCmdLineHOST(buff.c_str(),Prefix))
			break;

		//CONNECTED state
		if(!ShowHosts && hConnect &&
		        ExecCmdLineFTP(buff.c_str(),Prefix))
			break;

		//Unprocessed
		if(Prefix)
		{
			FP_Info->Control(this,FCTL_SETCMDLINE,(void*)"");
			FP_Info->ShowHelp(FP_Info->ModuleName,"FTPCommandLineHelp",FHELP_SELFHELP);
			return TRUE;
		}
		else
			return FALSE;
	}
	while(0);

//processed
	FP_Info->Control(this,FCTL_SETCMDLINE,(void*)"");

	if(isConn && (!hConnect || !hConnect->connected))
		BackToHosts();

	if(CurrentState != fcsClose)
		Invalidate();
	else
		FP_Info->Control(0,FCTL_REDRAWPANEL,NULL);

	return TRUE;
}

//------------------------------------------------
int FTP::ProcessCommandLine(char *CommandLine)
{
	PROC(("ProcessCommandLine","%s",CommandLine))
	BOOL isHostName;

	if(!CommandLine)
		return FALSE;

	//Trim spaces from start
	while(*CommandLine && *CommandLine == ' ') CommandLine++;

	if(!CommandLine[0]) return TRUE;

	isHostName = *CommandLine != '/';

	while(*CommandLine && *CommandLine == '/') CommandLine++;

	if(!CommandLine[0]) return TRUE;

	//Trim at end
	for(char *m = CommandLine + strlen(CommandLine) - 1;
	        m >= CommandLine && isspace(*m);
	        m--)
		*m = 0;

	if(!CommandLine[0]) return TRUE;

	//Dummy call with "."
	if(CommandLine[0] == '.' && !CommandLine[1])
		return TRUE;

	//Hosts
	if(isHostName)
		return ExecCmdLine(CommandLine,TRUE);

	//Connect
	FTPUrl            ui;
	QueueExecOptions  op;
	char             *UrlName = (CommandLine[ strlen(CommandLine)-1 ] != '/' &&
	                             strchr(CommandLine,'/') != NULL)
	                            ? strrchr(CommandLine,'/')
	                            : NULL;
	Host.Init();
	UrlInit(&ui);
	SetupQOpt(&op);

	if(UrlName)
		ui.SrcPath.printf("ftp://%s", CommandLine);

	//Get URL
	if(ui.SrcPath.Length())
	{
		if(!EditUrlItem(&ui))
		{
			static LPCSTR itms[] = { FMSG(MRejectTitle), FMSG(MQItemCancelled), FMSG(MYes), FMSG(MNo) };

			if(FMessage(FMSG_WARNING,NULL,itms,ARRAYSIZE(itms),2) != 0)
				return FALSE;

			*UrlName = 0;
			UrlName++;
		}
		else
		{
			//User confirm downloading: execute queue
			AddToQueque(&ui);

			if(QuequeSize &&
			        WarnExecuteQueue(&op))
			{
				ExecuteQueue(&op);
				return TRUE;
			}

			delete UrlsList;
			UrlsList = UrlsTail = NULL;
			QuequeSize = 0;
			return FALSE;
		}
	}

	//Connect to host
	ClearQueue();
	//Fill`n`Connect
	Host.SetHostName(CommandLine,NULL,NULL);

	if(!FullConnect())        return FALSE;

	FP_Screen::FullRestore();

	do
	{
		if(!UrlName || ShowHosts) break;

		SelectFile = UrlName;
	}
	while(0);

	return TRUE;
}
