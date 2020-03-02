#include <all_far.h>
#pragma hdrstop

#include "Int.h"

BOOL FTP::SetDirectoryStepped(LPCSTR Dir, BOOL update)
{
	PROC(("SetDirectoryStepped","%s",Dir))

	if(ShowHosts || !hConnect)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	//Empty dir
	if(!Dir[0])
		return TRUE;

	//Try direct change
	if(FtpSetCurrentDirectory(hConnect,Dir))
	{
		if(update) Invalidate();

		if(ShowHosts || !hConnect)
		{
			SetLastError(ERROR_CANCELLED);
			return FALSE;
		}

		return TRUE;
	}

	//dir name contains only one name
	if(strchr(Dir, '/') == NULL)
	{
		//Cancel
		return FALSE;
	}

	//Cancel changes in automatic mode
	if(IS_SILENT(FP_LastOpMode))
		return FALSE;

	//Ask user
	{
		static LPCSTR MsgItems[] =
		{
			FMSG(MAttention),
			FMSG(MAskDir1),
			NULL,
			FMSG(MAskDir3),
			FMSG(MYes), FMSG(MNo)
		};
		MsgItems[2] = Dir;

		if(FMessage(FMSG_WARNING|FMSG_LEFTALIGN, NULL, MsgItems, ARRAYSIZE(MsgItems),2) != 0)
		{
			SetLastError(ERROR_CANCELLED);
			return FALSE;
		}
	}
	//Try change to directory one-by-one
	char     str[MAX_PATH];
	LPCSTR m = NULL;
	int      num;

	do
	{
		if(*Dir == '/')
		{
			Dir++;

			if(*Dir == 0) break;

			str[0] = '/';
			str[1] = 0;
		}
		else
		{
			m = strchr(Dir, '/');

			if(m)
			{
				num = Min((int)ARRAYSIZE(str)-1, (int)(m-Dir));
				strncpy(str, Dir, num);
				str[num] = 0;
				Dir = m+1;
			}
			else
				StrCpy(str, Dir, ARRAYSIZE(str));
		}

		Log(("Dir: [%s]",str));

		if(!FtpSetCurrentDirectory(hConnect,str))
			return FALSE;

		if(update) Invalidate();

		if(ShowHosts || !hConnect)
		{
			SetLastError(ERROR_CANCELLED);
			return FALSE;
		}
	}
	while(m);

	return TRUE;
}

int FTP::SetDirectoryFAR(LPCSTR _Dir,int OpMode)
{
	FP_Screen _scr;
	return SetDirectory(_Dir, OpMode);
}

BOOL FTP::CheckDotsBack(const String& OldDir,const String& CmdDir)
{
	String str;

	if(!Opt.CloseDots || !CmdDir.Cmp(".."))
		return FALSE;

	GetCurPath(str);
	DelEndSlash(str,'/');
	Log(("UPDIR: [%s] [%s] %d",str.c_str(), OldDir.c_str(), str == OldDir));

	if(str == OldDir)
	{
		BackToHosts();
		return TRUE;
	}

	return FALSE;
}

int FTP::SetDirectory(LPCSTR Dir,int OpMode)
{
	char   *Slash;
	String  OldDir;
	int     rc;
	GetCurPath(OldDir);
	Log(("FTP::SetDirectory [%s] -> [%s]",OldDir.c_str(),Dir));

//Hosts
	if(ShowHosts)
	{
		do
		{
			//Up
			if(StrCmp(Dir,"..") == 0)
			{
				if(!OldDir[0] || (OldDir[0] == '\\' && !OldDir[1]))
				{
					Log(("Close plugin"));
					CurrentState = fcsClose;
					FP_Info->Control((HANDLE)this, FCTL_CLOSEPLUGIN, NULL);
					return TRUE;
				}

				Slash = strrchr(HostsPath,'\\');

				if(Slash)
					*Slash=0;
				else
					*HostsPath=0;

				Log(("DirBack"));
				break;
			}
			else

				//Root
				if(*Dir == 0 || (*Dir == '\\' && Dir[1] == 0))
				{
					HostsPath[0] = 0;
					Log(("Set to root"));
					break;
				}
				else

					//Directory
					if(FTPHost::CheckHostFolder(HostsPath, Dir))
					{
						AddEndSlash(HostsPath,'\\',ARRAYSIZE(HostsPath));
						StrCat(HostsPath, Dir, ARRAYSIZE(HostsPath));
						Log(("InDir"));
						break;
					}
		}
		while(0);

		if(!HostsPath[0])
		{
			HostsPath[0] = '\\';
			HostsPath[1] = 0;
		}

		Log(("HostsDir set to [%s]",HostsPath));
		//Save old directory
		FP_SetRegKey("LastHostsPath",HostsPath);
		return TRUE;
	}

//FTP

	/* Back to prev directory.
	   Directories allways separated by '/'!
	*/
	if(StrCmp(Dir,"..") == 0)
	{
		//Back from root
		if(Opt.CloseDots &&
		        (OldDir[0] == '/' || OldDir[0] == '*') && OldDir[1] == 0)
		{
			if(!IS_SILENT(OpMode))
				BackToHosts();

			return FALSE;
		}

		//Del last slash if any
		DelEndSlash(OldDir, '/');
		//Locate prev slash
		FTPDirList    dl;
		FTPServerInfo si;
		String        Line;
		si.ServerType = Host.ServerType;
		StrCpy(si.ServerInfo, hConnect->SystemInfo, ARRAYSIZE(si.ServerInfo));
		WORD idx = dl.DetectStringType(&si, Line.c_str(), Line.Length());

		if(idx!=FTP_TYPE_MVS)
			Slash = strrchr(OldDir.c_str(),'/');
		else
			Slash = strrchr(OldDir.c_str(),'.');

		//Set currently leaving directory to select in panel
		if(Slash != NULL)
		{
			SelectFile = Slash+1;

			if(idx==FTP_TYPE_MVS&&SelectFile[1]==0)
			{
				char* s1 = Slash;
				*s1=0;
				Slash = strrchr(OldDir.c_str(),'.');
				*s1='.';

				if(Slash != NULL)
					SelectFile = Slash+1;
			}
		}
	}

	//Change directory
	rc = TRUE;

	do
	{
		if(!hConnect)
		{
			rc = FALSE;
			break;
		}

		if(SetDirectoryStepped(Dir,FALSE))
		{
			if(!IS_SILENT(OpMode))
				CheckDotsBack(OldDir,Dir);

			break;
		}

		if(!hConnect ||
		        GetLastError() == ERROR_CANCELLED)
		{
			rc = FALSE;
			break;
		}

		if(!hConnect->ConnectMessageTimeout(MChangeDirError,Dir,-MRetry))
		{
			rc = FALSE;
			break;
		}

		if(FtpCmdLineAlive(hConnect) &&
		        FtpKeepAlive(hConnect))
			continue;

		SaveUsedDirNFile();

		if(!FullConnect())
		{
			rc = FALSE;
			break;
		}
	}
	while(true);

	return rc;
}
