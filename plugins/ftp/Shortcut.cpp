#include <all_far.h>
#pragma hdrstop

#include "Int.h"

int FTP::ProcessShortcutLine(char *Line)
{
	char *m,*m1;
	char  str[20]; //Hold single dec number - need not use of String

	if(!Line)
		return FALSE;

	ReadCfg();

	if(StrCmp(Line,"FTP:",4) == 0)
	{
		/*
		FTP
		  Host
		  1
		  AskLogin    + 3
		  AsciiMode   + 3
		  PassiveMode + 3
		  UseFirewall + 3
		  HostTable
		  1
		  User
		  1
		  Password
		  1
		  ExtCmdView + 3
		  IOBuffSize (atoi)
		  1
		  FFDup + '0'
		  DecodeCmdLine + '0'
		  1
		*/
		Line += 4;
		m = strchr(Line,'\x1');

		if(!m || strlen(m) < 4) return FALSE;

		m++;
		Host.Init();
		StrCpy(Host.Host,     Line, Min((int)(m-Line), (int)ARRAYSIZE(Host.Host)));
		strcpy(Host.HostName, Host.Host);
		Host.AskLogin    = *(m++) - '\x3';

		if(*m == 0) return FALSE;

		Host.AsciiMode   = *(m++) - '\x3';

		if(*m == 0) return FALSE;

		Host.PassiveMode = *(m++) - '\x3';

		if(*m == 0) return FALSE;

		Host.UseFirewall = *(m++) - '\x3';

		if(*m == 0) return FALSE;

		m1 = m;
		m = strchr(m1,'\x1');

		if(!m) return FALSE;

		StrCpy(str, m1, (int)(m-m1+1));
		Host.ServerType = (WORD)atoi(str);
		m1 = m+1;
		m = strchr(m1,'\x1');

		if(!m) return FALSE;

		StrCpy(Host.HostTable, m1, (int)(m-m1+1));
		m1 = m+1;
		m  = strchr(m1,'\x1');

		if(!m) return FALSE;

		StrCpy(Host.User,m1,(int)(m-m1+1));
		m1 = m+1;
		m  = strchr(m1,'\x1');

		if(!m) return FALSE;

		StrCpy(Host.Password,m1,(int)(m-m1+1));

		do
		{
			Host.ExtCmdView    = Opt.ExtCmdView;
			Host.IOBuffSize    = Opt.IOBuffSize;
			Host.FFDup         = Opt.FFDup;
			Host.DecodeCmdLine = TRUE;
			//IOBuffSize
			m1 = m+1;

			if(!m1[0])
				break;

			Host.ExtCmdView = *(m1++) - '\x3';
			m = strchr(m1,'\x1');

			if(!m)
				return FALSE;

			StrCpy(str,m1,(int)(m-m1+1));
			Host.IOBuffSize = Max(FTR_MINBUFFSIZE,(DWORD)atoi(str));
			//FFDup
			m1 = m+1;

			if(!m1[0])
				break;

			m = strchr(m1,'\x1');

			if(!m)
				break;

			if(*m1 != '0' && *m1 != '1')
				break;

			Host.FFDup = *m1 - '0';
			//DecodeCmdLine
			m1++;

			if(!m1[0] ||
			        (*m1 != '0' && *m1 != '1'))
				break;

			Host.DecodeCmdLine = *m1 - '0';
		}
		while(0);

		return FullConnect();
	}
	else if(StrCmp(Line,"HOST:",5) == 0)
	{
		Line += 5;
		strcpy(HostsPath,Line);
		return 1;
	}

	return 0;
}
