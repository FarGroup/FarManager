#include <all_far.h>
#pragma hdrstop

#include "Int.h"

BOOL FTP::FTPCreateDirectory(LPCSTR dir,int OpMode)
{
	String Command;
	int    len = (int)strlen(dir);

	if(!len)
		return FALSE;

	len--;
	Assert(hConnect && "FtpCreateDirectory");

	do
	{
		//Try relative path
		if(!isspace(dir[len]))
		{
			hConnect->CacheReset();
			Command.printf("mkdir \x1%s\x1", dir);

			if(hConnect->ProcessCommand(Command))
				break;
		}

		//Try relative path with end slash
		hConnect->CacheReset();
		Command.printf("mkdir \x1%s/\x1", dir);

		if(hConnect->ProcessCommand(Command))
			break;

		//Try absolute path
		if(!isspace(dir[len]))
		{
			Command.printf("mkdir \x1%s/%s\x1",
			               hConnect->SToOEM(hConnect->CurDir).c_str(), dir + (dir[0] == '/'));

			if(hConnect->ProcessCommand(Command))
				break;
		}

		//Try absolute path with end slash
		Command.printf("mkdir \x1%s/%s/\x1",
		               hConnect->SToOEM(hConnect->CurDir).c_str(), dir + (dir[0] == '/'));

		if(hConnect->ProcessCommand(Command))
			break;

		//Noone work
		return FALSE;
	}
	while(0);

	if(!IS_SILENT(OpMode))
	{
		int b = Command.Chr('\x1'),
		    e = Command.Chr('\x1', b+1);

		if(b == -1)
			SelectFile = "";
		else if(e == -1)
			SelectFile.Set(Command.c_str(), b+1, -1);
		else
			SelectFile.Set(Command.c_str(), b+1, e);
	}

	return TRUE;
}

int FTP::MakeDirectory(String& Name,int OpMode)
{
	PROC(("FTP::MakeDirectory",NULL))
	FTPHost h;

	if(!ShowHosts && !hConnect)
		return FALSE;

	h.Init();

//Edit name
	if(!IS_SILENT(OpMode) && !IS_FLAG(OpMode,OPM_NODIALOG) &&
	        !EditDirectory(Name, ShowHosts ? h.HostDescr : NULL, TRUE))
		return -1;

//Correct name
	if(!Name[0])
		return -1;

//HOSTS
	if(ShowHosts)
	{
		if(Name.Cmp(".")  ||
		        Name.Cmp("..") ||
		        FTPHost::CheckHost(HostsPath, Name.c_str()))
		{
			SetLastError(ERROR_ALREADY_EXISTS);
			return FALSE;
		}

		char str[MAX_PATH];
		h.Folder = TRUE;
		StrCpy(h.Host, Name.c_str(), ARRAYSIZE(h.Host));
		h.MkINIFile(str,NULL,"");
		StrCpy(h.Host, str, ARRAYSIZE(h.Host));
		h.Write(HostsPath);
		SelectFile=Name;
		return TRUE;
	}

//FTP
	FP_Screen   scr;

	//Create directory
	do
	{
		//Try to create
		if(hConnect &&
		        FTPCreateDirectory(Name.c_str(), OpMode))
			return TRUE;

		//If conection alive - report error
		if(FtpCmdLineAlive(hConnect) ||
		        IS_SILENT(OpMode))
			return FALSE;

		//Try to reconnect
		if(!Reread())
			return FALSE;

		//Repeat operation
	}
	while(true);
}
