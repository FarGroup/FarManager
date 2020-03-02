#include <all_far.h>
#pragma hdrstop

#include "Int.h"

BOOL FTP::DoFtpConnect(int blocked)
{
	char  hst[FAR_MAX_NAME],
	   usr[FAR_MAX_NAME],
	   pwd[FAR_MAX_NAME];
	const char *m;
	BOOL  askPwd = Host.AskLogin;
	hConnect = NULL;
	SetLastError(ERROR_SUCCESS);

	if(!Host.Host[0])
		return FALSE;

	CurrentState = fcsConnecting;
	//Create`n`Init connection
	hConnect = new Connection;

	if(!hConnect)
	{
		CurrentState = fcsNormal;
		return FALSE;
	}

	hConnect->InitData(&Host,blocked);
	hConnect->InitCmdBuff();
	hConnect->InitIOBuff();

	do
	{
		hConnect->Host.ExtCmdView = Host.ExtCmdView;
		StrCpy(hst, Host.Host,     ARRAYSIZE(hst));
		StrCpy(usr, Host.User,     ARRAYSIZE(usr));
		StrCpy(pwd, Host.Password, ARRAYSIZE(pwd));

		if(!hConnect->LoginComplete)
			if(!GetLoginData(usr, pwd, Host.AskLogin || askPwd))
				break;

		//Firewall
		if(Host.UseFirewall && *Opt.Firewall)
		{
			strcat(usr,"@");
			StrCat(usr,hst,ARRAYSIZE(usr));
			StrCpy(hst,Opt.Firewall,ARRAYSIZE(hst));
		}

		// Find port
		m = strrchr(hst,':');

		if(m)
		{
			*(char *)m = 0;
			m++;
		}
		else
			m = "0";

		//Connect
		Log(("Connecting"));

		if(hConnect->Init(hst,m,usr,pwd))
		{
			Log(("Connecting succ %d",hConnect->GetResultCode()));

			if(hConnect->GetResultCode() > 420)
			{
				Log(("Connect error from server [%d]",hConnect->GetResultCode()));
				return FALSE;
			}

			FtpSystemInfo(hConnect,NULL,-1);

			if(!FtpGetFtpDirectory(hConnect))
			{
				Log(("!Connect - get dir"));
				return FALSE;
			}

			hConnect->CheckResume();
			CurrentState = fcsFTP;
			return TRUE;
		}

		Log(("!Init"));

		if(!hConnect->LoginComplete &&
		        Opt.AskLoginFail && hConnect->GetResultCode() == 530)
		{
			Log(("Reask password"));
			hConnect->Host.ExtCmdView = TRUE;
			hConnect->ConnectMessage(MNone__,hst,MNone__);
			askPwd = TRUE;
			continue;
		}

		//!Connected
		SetLastError(hConnect->ErrorCode);

		//!Init
		if((UINT)hConnect->SocketError != INVALID_SOCKET)
			hConnect->ConnectMessage(MWSANotInit,GetSocketErrorSTR(hConnect->SocketError),-MOk);
		else

			//!Cancel, but error
			if(hConnect->ErrorCode != ERROR_CANCELLED)
				hConnect->ConnectMessage(MCannotConnectTo,Host.Host,-MOk);

		break;
	}
	while(true);

	CurrentState = fcsNormal;
	delete hConnect;
	hConnect = NULL;
	return FALSE;
}

int FTP::Connect()
{
	PROC(("FTP::Connect",NULL))
	ConnectionState  cst;
	FP_Screen        _scr;
	LogCmd("-------- CONNECTING (plugin: " __DATE__ ", " __TIME__ ") --------",ldInt);

	do
	{
//Close old
		if(hConnect)
		{
			Log(("old connection %p",hConnect));
			hConnect->GetState(&cst);
			Log(("del connection %p",hConnect));
			delete hConnect;
			hConnect = NULL;
			Log(("del connection done"));
		}

//Connect
		if(Opt.ShowIdle && IS_SILENT(FP_LastOpMode))
		{
			Log(("Say connecting"));
			SetLastError(ERROR_SUCCESS);
			IdleMessage(FP_GetMsg(MConnecting),Opt.ProcessColor);
		}

		if(!DoFtpConnect(cst.Inited ? cst.Blocked : (-1)) ||
		        !hConnect)
		{
			Log(("!Connected"));
			break;
		}

		if(cst.Inited)
		{
			//Restore old connection values or set defaults
			Log(("Restore connection datas"));
			hConnect->SetState(&cst);
		}
		else
		{
			//Init new caonnection values
			Log(("Set connection defaults"));
			hConnect->SetTable(TableNameToValue((*Host.HostTable) ? Host.HostTable : Opt.Table));
			hConnect->Host.PassiveMode = Host.PassiveMode;
		}

//Set start FTP mode
		ShowHosts       = FALSE;
		SwitchingToFTP  = TRUE;
		SelectFile      = "";
		FP_Info->Control(this,FCTL_REDRAWANOTHERPANEL,NULL);

//Set base directory
		if(hConnect && *Host.Home)
		{
			if(Host.Home[0]=='/'&&Host.Home[1]=='\'')
				memmove(Host.Home,Host.Home+1,sizeof(Host.Home)-1);

			Log(("HomeDir [%s]",Host.Home));
			FtpSetCurrentDirectory(hConnect,Host.Home);
		}

		IdleMessage(NULL,0);
		return TRUE;
	}
	while(0);

	if(hConnect)
	{
		hConnect->ConnectMessage(MCannotConnectTo, Host.Host, -MOk);
		delete hConnect;
		hConnect = NULL;
	}

	Host.Init();
	IdleMessage(NULL,0);
	return FALSE;
}

BOOL FTP::FullConnect()
{
	if(Connect())
	{
		if(!ShowHosts)
		{
			BOOL isSaved = FP_Screen::isSaved() != 0;
			FP_Screen::FullRestore();
			FP_Info->Control(this,FCTL_SETVIEWMODE,&StartViewMode);
			FP_Info->Control(this,FCTL_UPDATEPANEL,NULL);
			PanelRedrawInfo ri;
			ri.CurrentItem = ri.TopPanelItem = 0;
			FP_Info->Control(this,FCTL_REDRAWPANEL,&ri);

			if(isSaved)
				FP_Screen::Save();

			SwitchingToFTP = FALSE;
			return TRUE;
		}
	}

	return FALSE;
}
