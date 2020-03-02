#include <all_far.h>
#pragma hdrstop

#include "Int.h"

int FTP::ProcessEvent(int Event,void *Param)
{
#if defined(__FILELOG__)
	static LPCSTR states[] = { "fcsNormal", "fcsExpandList", "fcsClose", "fcsConnecting","fcsFTP" };
	PROC(("ProcessEvent","%d (%s),%08X",Event,states[CurrentState],Param))
#endif
	PanelInfo pi;
	int       n;
	FTPHost*  p;
	pi.PanelItems = NULL;

	if(Event == FE_BREAK ||
	        CurrentState == fcsClose ||
	        CurrentState == fcsConnecting)
	{
		Log(("Skip event"));
		return FALSE;
	}

//Close notify
	if(Event == FE_CLOSE)
	{
		Log(("Close notify"));
		CurrentState = fcsClose;

		if(ShowHosts)
		{
			if(!pi.PanelItems)
				FP_Info->Control(this, FCTL_GETPANELINFO, &pi);

			FP_SetRegKey("LastHostsMode",pi.ViewMode);
			Log(("Write HostsMode: %d",pi.ViewMode));
		}

		return FALSE;
	}

//Position cursor to added|corrected item on panel
	if(Event == FE_REDRAW)
	{
		if(SelectFile.Length())
		{
			Log(("PositionItem: [%s]",SelectFile.c_str()));

			if(!pi.PanelItems)
				FP_Info->Control(this, FCTL_GETPANELINFO, &pi);

			for(n = 0; n < pi.ItemsNumber; n++)
			{
				if(ShowHosts)
				{
					if((p=FTPHost::Convert(&pi.PanelItems[n])) == NULL ||
					        !SelectFile.Cmp(PointToName(p->RegKey)))
						continue;
				}
				else
				{
					if(!SelectFile.Cmp(FTP_FILENAME(&pi.PanelItems[n])))
						continue;
				}

				Log(("PosItem[%d] [%s]", n, FTP_FILENAME(&pi.PanelItems[n])));
				PanelRedrawInfo pri;
				pri.CurrentItem  = n;
				pri.TopPanelItem = pi.TopPanelItem;
				SelectFile       = "";
				FP_Info->Control(this,FCTL_REDRAWPANEL,&pri);
				break;
			}

			SelectFile = "";
		}
	}/*REDRAW*/

//Correct saved hosts mode on change
	if(ShowHosts && Event == FE_CHANGEVIEWMODE)
	{
		if(!pi.PanelItems)
			FP_Info->Control(this, FCTL_GETPANELINFO, &pi);

		Log(("New ColumnMode %d",pi.ViewMode));
		FP_SetRegKey("LastHostsMode",pi.ViewMode);
	}

//Set start hosts panel mode
	if(Event == FE_REDRAW)
	{
		if(ShowHosts && !PluginColumnModeSet)
		{
			if(Opt.PluginColumnMode != -1)
				FP_Info->Control(this,FCTL_SETVIEWMODE,&Opt.PluginColumnMode);
			else
			{
				int num = FP_GetRegKey("LastHostsMode",2);
				FP_Info->Control(this,FCTL_SETVIEWMODE,&num);
			}

			PluginColumnModeSet = TRUE;
		}
		else if(isBackup() && NeedToSetActiveMode)
		{
			FP_Info->Control(this,FCTL_SETVIEWMODE,&ActiveColumnMode);
			NeedToSetActiveMode = FALSE;
		}
	}

//Keep alive
	if(Event == FE_IDLE          &&
	        !ShowHosts                &&
	        FtpCmdLineAlive(hConnect) &&
	        KeepAlivePeriod           &&
	        FP_PeriodEnd(KeepAlivePeriod))
	{
		Log(("Keep alive"));
		FTPCmdBlock blocked(this,TRUE);

		if(Opt.ShowIdle)
		{
			SetLastError(ERROR_SUCCESS);
			IdleMessage(FP_GetMsg(MKeepAliveExec),Opt.ProcessColor);
		}

		FtpKeepAlive(hConnect);
		Log(("end Keep alive"));
	}

//CMD line
	if(Event == FE_COMMAND)
	{
		Log(("CMD line"));
		return ExecCmdLine((LPCSTR)Param,FALSE);
	}

	return FALSE;
}
