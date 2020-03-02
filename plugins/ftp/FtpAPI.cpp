#include <all_far.h>
#pragma hdrstop

#include "Int.h"

static BOOL ParseDirLine(Connection *Connect, FTPFileInfo* lpFindFileData);

//--------------------------------------------------------------------------------
BOOL FtpKeepAlive(Connection *hConnect)
{
	if(!hConnect)
	{
		SetLastError(ERROR_INTERNET_CONNECTION_ABORTED);
		return FALSE;
	}

	hConnect->ErrorCode = ERROR_SUCCESS;

	if(!hConnect->ProcessCommand("pwd"))
	{
		SetLastError(hConnect->ErrorCode);
		return FALSE;
	}
	else
		return TRUE;
}

BOOL FtpIsResume(Connection *hConnect)
{
	if(!hConnect) return FALSE;

	return ((Connection *)hConnect)->ResumeSupport;
}

BOOL FtpCmdLineAlive(Connection *hConnect)
{
	return hConnect &&
	       hConnect->connected &&
	       hConnect->cmd_peer != INVALID_SOCKET;
}

void FtpSetRetryCount(Connection *hConnect,int cn)
{
	Assert(hConnect && "FtpSetRetryCount");
	hConnect->RetryCount = cn;
}

BOOL FtpSetBreakable(Connection *hConnect,int cn)
{
	if(!hConnect)
		return FALSE;

	BOOL rc = hConnect->Breakable;

	if(cn != -1)
	{
		Log(("ESC: set brk %d->%d", hConnect->Breakable, cn));
		hConnect->Breakable = cn;
	}

	return rc;
}

int FtpGetRetryCount(Connection *hConnect)
{
	Assert(hConnect && "FtpGetRetryCount");
	return hConnect->RetryCount;
}

int FtpConnectMessage(Connection *hConnect,int Msg,LPCSTR HostName,int btn /*= MNone__*/,int btn1 /*= MNone__*/,int btn2 /*= MNone__*/)
{
	return hConnect ? hConnect->ConnectMessage(Msg,HostName,btn,btn1,btn2) : FALSE;
}

int FtpCmdBlock(Connection *hConnect,int block)
{
	int rc = -1;

	do
	{
		if(!hConnect)
			break;

		rc = ((Connection *)hConnect)->CmdVisible == FALSE;

		if(block != -1)
			((Connection *)hConnect)->CmdVisible = block == FALSE;
	}
	while(0);

	return rc;
}

BOOL FtpFindFirstFile(Connection *hConnect, LPCSTR lpszSearchFile,FTPFileInfo* lpFindFileData, BOOL *ResetCache)
{
	Assert(hConnect && "FtpFindFirstFile");
	String Command;
	int    AllFiles = StrCmp(lpszSearchFile,"*")==0 || StrCmp(lpszSearchFile,"*.*")==0;
	int    FromCache = 0;

	if(ResetCache && *ResetCache == TRUE)
	{
		hConnect->CacheReset();
		FtpGetFtpDirectory(hConnect);
		*ResetCache = FALSE;
	}

	if(AllFiles)
		Command = "dir";
	else
	{
		if(*lpszSearchFile=='\\' || lpszSearchFile[0] && lpszSearchFile[1]==':')
			lpszSearchFile = PointToName((char *)lpszSearchFile);

		Command.printf("dir \x1%s\x1", lpszSearchFile);
	}

	SetLastError(ERROR_SUCCESS);

	if(!AllFiles || (FromCache=hConnect->CacheGet()) == 0)
	{
		if(AllFiles && !IS_SILENT(FP_LastOpMode) &&
		        hConnect->CmdVisible &&
		        hConnect->CurrentState != fcsExpandList)
			hConnect->ConnectMessage(MRequestingFolder,
			                         hConnect->ToOEMDup(hConnect->CurDir.c_str()));

		int pc = hConnect->ProcessCommand(Command);

		if(!pc)
		{
			if(hConnect->Host.ServerType==FTP_TYPE_MVS)
			{
				if(hConnect->code==550)
				{
					pc = 1;  //550 No members found.
				}

				if(hConnect->code==501&&AllFiles)
				{
					Command="dir *";
					pc = hConnect->ProcessCommand(Command);
				}
			}

			if(!pc)
			{
				SetLastError(hConnect->ErrorCode);
				return FALSE;
			}
		}
	}

	if(AllFiles && !FromCache)
		hConnect->CacheAdd();

	return ParseDirLine(hConnect, lpFindFileData);
}


BOOL FtpFindNextFile(Connection *hConnect,FTPFileInfo* lpFindFileData)
{
	Assert(hConnect && "FtpFindNextFile");
	return ParseDirLine(hConnect, lpFindFileData);
}

BOOL FtpGetCurrentDirectory(Connection *hConnect,String& s)
{
	s.Null();

	if(!hConnect)
		return TRUE;

	if(!hConnect->CurDir.Length())
	{
		if(!FtpGetFtpDirectory(hConnect))
			return FALSE;
	}

	s = hConnect->CurDir;
	hConnect->ToOEM(s);
	return s.Length() != 0;
}

BOOL FtpSetCurrentDirectory(Connection *hConnect, LPCSTR dir)
{
	String Command;
	Assert(hConnect && "FtpSetCurrentDirectory");

	if(*dir == 0)
		return FALSE;

	do
	{
		if(StrCmp(dir, "..") == 0)
		{
			Command.printf("cdup");
		}
		else
		{
			Command.printf("cd \x1%s\x1",dir);
		}

		if(hConnect->ProcessCommand(Command))
			break;

		return FALSE;
	}
	while(0);

	FtpGetFtpDirectory(hConnect);
	return TRUE;
}

BOOL FtpRemoveDirectory(Connection *hConnect,LPCSTR dir)
{
	String Command;
	Assert(hConnect && "FtpRemoveDirectory");

	if(StrCmp(dir,".") == 0 ||
	        StrCmp(dir,"..") == 0)
		return TRUE;

	hConnect->CacheReset();
	//Dir
	Command.printf("rmdir \x1%s\x1", dir);

	if(hConnect->ProcessCommand(Command))
		return TRUE;

	//Dir+slash
	Command.printf("rmdir \x1%s/\x1", dir);

	if(hConnect->ProcessCommand(Command))
		return TRUE;

	//Full dir
	Command.printf("rmdir \x1%s/%s\x1",
	               hConnect->SToOEM(hConnect->CurDir.c_str()).c_str(), dir + (dir[0] == '/'));

	//??FixFTPSlash( Command );
	if(hConnect->ProcessCommand(Command))
		return TRUE;

	//Full dir+slash
	Command.printf("rmdir \"%s/%s/\"",
	               hConnect->SToOEM(hConnect->CurDir.c_str()).c_str(), dir + (dir[0] == '/'));

	if(hConnect->ProcessCommand(Command))
		return TRUE;

	return FALSE;
}

static bool correct_vms_name(String& name)
{
	bool ret = false;
	FTPDirList dl;
	FTPType *tp = dl.GetType(FTP_TYPE_VMS);
	if (tp && tp->PWDParse)
	{
		char tmp[1024];
		if (tp->PWDParse(nullptr, name.c_str(), tmp, sizeof(tmp)))
		{
			name = tmp;
			ret = true;
		}
	}
	return ret;
}

BOOL FtpRenameFile(Connection *Connect,LPCSTR lpszExisting,LPCSTR lpszNew)
{
	String Command, name1, name2;
	Assert(Connect && "FtpRenameFile");
	Connect->CacheReset();
	if (Connect->Host.ServerType == FTP_TYPE_VMS)
	{
		name1 = lpszExisting;
		correct_vms_name(name1);
		lpszExisting = name1.c_str();
		name2 = lpszNew;
		correct_vms_name(name2);
		lpszNew = name2.c_str();
	}
	Command.printf("ren \x1%s\x1 \x1%s\x1",lpszExisting,lpszNew);
	return Connect->ProcessCommand(Command);
}


BOOL FtpDeleteFile(Connection *hConnect,LPCSTR lpszFileName)
{
	String Command;
	Assert(hConnect && "FtpDeleteFile");
	hConnect->CacheReset();
	Command.printf("del \x1%s\x1", lpszFileName);
	return hConnect->ProcessCommand(Command);
}


BOOL FtpChmod(Connection *hConnect,LPCSTR lpszFileName,DWORD Mode)
{
	Assert(hConnect && "FtpChmod");
	String Command;
	hConnect->CacheReset();
	Command.printf("chmod %o \x1%s\x1", Mode, lpszFileName);
	return hConnect->ProcessCommand(Command);
}

BOOL FtpGetFile(Connection *Connect,LPCSTR lpszRemoteFile,LPCSTR lpszNewFile,BOOL Reget,int AsciiMode)
{
	PROC(("FtpGetFile","[%s]->[%s] %s %s",lpszRemoteFile,lpszNewFile,Reget?"REGET":"NEW",AsciiMode?"ASCII":"BIN"));
	String Command,
	       full_name;
	int  ExitCode;
	Assert(Connect && "FtpGetFile");

//mode
	if(AsciiMode && !Connect->ProcessCommand("ascii"))
	{
		Log(("!ascii ascii:%d",AsciiMode));
		return FALSE;
	}
	else if(!AsciiMode && !Connect->ProcessCommand("bin"))
	{
		Log(("!bin ascii:%d",AsciiMode));
		return FALSE;
	}

//Create directory
	Command = lpszNewFile;
	int m = Command.RChr('\\');

	if(m != -1)
	{
		Command.SetLength(m);

		if(!DoCreateDirectory(Command.c_str()))
		{
			Log(("!CreateDirectory [%s]",Command.c_str()));
			return FALSE;
		}
	}

//Remote file
	if(Connect->Host.ServerType!=FTP_TYPE_MVS && *lpszRemoteFile != '/')
	{
		full_name = Connect->ToOEMDup(Connect->CurDir.c_str());
		AddEndSlash(full_name, '/');
		full_name.Add(lpszRemoteFile);
		if (Connect->Host.ServerType == FTP_TYPE_VMS)
			correct_vms_name(full_name);

		lpszRemoteFile = full_name.c_str();
	}

//Get file
	Connect->IOCallback = TRUE;

	if(Reget && !Connect->ResumeSupport)
	{
		Connect->AddCmdLine(FMSG(MResumeRestart));
		Reget = FALSE;
	}

	Command.printf("%s \x1%s\x1 \x1%s\x1",
	               Reget ? "reget":"get",
	               lpszRemoteFile, lpszNewFile);
	ExitCode = Connect->ProcessCommand(Command);
	Connect->IOCallback = FALSE;
	return ExitCode;
}

__int64 FtpFileSize(Connection *Connect,LPCSTR fnm)
{
	String Command, name;
	BYTE Line[20];

	if(!Connect) return -1;

	if (Connect->Host.ServerType == FTP_TYPE_VMS)
	{
		name = fnm;
		correct_vms_name(name);
		fnm = name.c_str();
	}
	Command.printf("size \x1%s\x1",fnm);

	if(!Connect->ProcessCommand(Command))
	{
		Log(("!size"));
		return -1;
	}
	else
	{
		Connect->GetReply(Line,sizeof(Line));
		return AtoI((LPCSTR)(Line+4),(__int64)-1);
	}
}

BOOL FtpPutFile(Connection *Connect,LPCSTR loc,LPCSTR rem,BOOL Reput,int AsciiMode)
{
	PROC(("FtpPutFile","[%s]->[%s] %s %s",loc,rem,Reput?"REGET":"NEW",AsciiMode?"ASCII":"BIN"));
	String  Command, full_name;
	int     ExitCode;
	__int64 Position;
	Assert(Connect && "FtpPutFile");
	Connect->CacheReset();

	if(AsciiMode && !Connect->ProcessCommand("ascii") ||
	        !AsciiMode && !Connect->ProcessCommand("bin"))
	{
		Log(("!Set mode"));
		return FALSE;
	}

	if(AsciiMode)
		Reput = FALSE;

//Remote file
	if(*rem=='\\' || (rem[0] && rem[1]==':'))
		rem = PointToName((char *)rem);

	if(Connect->Host.ServerType == FTP_TYPE_VMS)
	{
		full_name = rem;
		correct_vms_name(full_name);
		rem = full_name.c_str();
	}
	else if(Connect->Host.ServerType!=FTP_TYPE_MVS)
	{
		if(*rem != '/')
		{
			full_name.printf("%s/%s", Connect->ToOEMDup(Connect->CurDir.c_str()), rem);
			rem = full_name.c_str();
		}
	}

	if(Reput)
	{
		Position = FtpFileSize(Connect, rem);

		if(Position == -1)
			return FALSE;
	}
	else
		Position = 0;

	Connect->IOCallback = TRUE;
	//Append
	Connect->restart_point = Position;
	Command.printf("%s \x1%s\x1 \x1%s\x1",
	               Position ? "appe" : "put", loc, rem);
	Log(("%s upload", Position ? "Try APPE" : "Use PUT"));
	ExitCode = Connect->ProcessCommand(Command);

	//Error APPE, try to resume using REST
	if(!ExitCode && Position && Connect->ResumeSupport)
	{
		Log(("APPE fail, try REST upload"));
		Connect->restart_point = Position;
		Command.printf("put \x1%s\x1 \x1%s\x1", loc, rem);
		ExitCode = Connect->ProcessCommand(Command);
	}

	Connect->IOCallback = FALSE;
	return ExitCode;
}

BOOL FtpSystemInfo(Connection *Connect,char *Buffer,int MaxSize)
{
	char *ChPtr;
	Assert(Connect && "FtpSystemInfo");

	if(Buffer && MaxSize < 1)
		return FALSE;

	if(!Connect->SystemInfoFilled)
	{
		FP_Screen _scr;
		Connect->SystemInfoFilled = TRUE;

		if(Connect->ProcessCommand("syst"))
		{
			char tmp[ 200 ];  //Do not need to use String. Limit system info by 200 chars.
			Connect->GetReply((BYTE*)tmp,sizeof(tmp));

			if((ChPtr=strchr(tmp,'\r')) != NULL)
				*ChPtr=0;

			if((ChPtr=strchr(tmp,'\n')) != NULL)
				*ChPtr=0;

			if(isdigit(tmp[0]) && isdigit(tmp[1]) && isdigit(tmp[2]))
				StrCpy(Connect->SystemInfo,tmp+4,ARRAYSIZE(Connect->SystemInfo));
			else
				StrCpy(Connect->SystemInfo,tmp,ARRAYSIZE(Connect->SystemInfo));
		}
		else
		{
			*Connect->SystemInfo = 0;
		}

		FTPDirList    dl;
		FTPServerInfo si;
		String        Line;
		si.ServerType = Connect->Host.ServerType;
		StrCpy(si.ServerInfo, Connect->SystemInfo, ARRAYSIZE(si.ServerInfo));
		WORD idx = dl.DetectStringType(&si, Line.c_str(), Line.Length());

		if(idx==FTP_TYPE_MVS)
		{
			Log(("site directorymode"));

			if(Connect->ProcessCommand("site directorymode"))
			{
				char tmp[ 200 ];  //Do not need to use String. Limit system info by 200 chars.
				Connect->GetReply((BYTE*)tmp,sizeof(tmp));
			}
		}
	}

	if(Buffer)
	{
		StrCpy(Buffer,Connect->SystemInfo,MaxSize);
		return *Buffer != 0;
	}
	else
		return TRUE;
}

BOOL FtpGetFtpDirectory(Connection *Connect)
{
	String         s, s1;
	FTPDirList     dl;
	char          *m;
	FTPServerInfo  si;
	WORD           idx;
	//Exec
	{
		FP_Screen  _scr;

		if(!Connect->ProcessCommand("pwd"))
			return FALSE;
	}
	Connect->GetReply(s);

	do
	{
		//Detect if unknown
		si.ServerType = Connect->Host.ServerType;
		StrCpy(si.ServerInfo, Connect->SystemInfo, ARRAYSIZE(si.ServerInfo));
		idx = Connect->Host.ServerType;

		if(idx == FTP_TYPE_DETECT || idx == FTP_TYPE_INVALID)
			idx = dl.DetectDirStringType(&si, s.c_str());

		//Parse
		FTPType*   tp;
		char       tmp[ 1024 ];  //There is not way to use String.

		if((tp=dl.GetType(idx)) != NULL &&
		        tp->PWDParse &&
		        tp->PWDParse(&si, s.c_str(), tmp, sizeof(tmp)))
		{
			Connect->CurDir = tmp;
			break;
		}

		//Del digits
		m = s.c_str();

		while(*m && (isdigit(*m) || strchr("\t\b ",*m) != NULL)) m++;

		if(!m[0])
			return FALSE;

		s.Del(0, (int)(m-s.c_str()));

		//Decode FF
		if(Connect->Host.UndupFF)
		{
			for(m = s.c_str(); *m; m++)
			{
				if(m[0] == (char)0xFF && m[1] == (char)0xFF)
				{
					s1.Add((char)0xFF);
					m++;
				}
				else
					s1.Add(*m);
			}

			s = s1;
		}

		//Set classic path
		/* Unix:
		     - name enclosed with '"'
		     - if '"' is in name it doubles
		*/
		int b;

		if((b=s.Chr('\"')) != -1)
		{
			s1.Null();

			for(int n = b+1; n < s.Length(); n++)
				if(s[n] == '\"')
				{
					if(s[n+1] == '\"')
					{
						s1.Add(s[n]);
						n++;
					}
					else
						break;
				}
				else
					s1.Add(s[n]);

			Connect->CurDir = s1;
		}
		else
			//Raw
			Connect->CurDir = s;
	}
	while(0);

	//Remove NL\CR
	int num;

	while((num=Connect->CurDir.Chr("\r\n")) != -1)
		Connect->CurDir.SetLength(num);

	return TRUE;
}

//------------------------------------------------------------------------
void BadFormat(Connection *Connect,LPCSTR Line,BOOL inParce)
{
	Connect->AddCmdLine(Line);
	FtpConnectMessage(Connect, MNone__,
	                  inParce
	                  ? "Error parsing files list. Please read \"BugReport_*.txt\" and report to developer."
	                  : "Can not find listing parser. Please read \"BugReport_*.txt\" and report to developer.",
	                  -MOk);
}

LPCSTR Parser2Str(WORD sType, FTPDirList* dl/*=NULL*/)
{
	BOOL     isNew = dl == NULL;
	LPCSTR rc;

	if(sType == FTP_TYPE_DETECT)
		return "Autodetect";
	else if(sType == FTP_TYPE_INVALID)
		return "Invalid";

	if(!dl)
		dl = new FTPDirList;

	FTPType* tp;

	if((tp=dl->GetType(sType)) == NULL)
		rc = "Unknown";
	else
		rc = tp->TypeName;

	if(isNew)
		delete dl;

	return rc;
}

WORD FTP::SelectServerType(WORD Type)
{
	FarMenuItem MenuItems[50];
	FTPDirList  dl;
	WORD        n,cn;
	memset(MenuItems, 0, sizeof(MenuItems));
	StrCpy(MenuItems[0].Text, FP_GetMsg(MTableAuto), ARRAYSIZE(MenuItems[0].Text));
	MenuItems[1].Separator = TRUE;
	cn = dl.GetNumberOfSupportedTypes();

	for(n = 0; n < ARRAYSIZE(MenuItems) && n < cn; n++)
	{
		FTPType* tp = dl.GetType(n);
		_snprintf(MenuItems[n+2].Text, ARRAYSIZE(MenuItems[0].Text),
		          "%s %c %s",
		          tp->TypeName, FAR_VERT_CHAR, tp->TypeDescription);
	}

	if(Type >= n)
		MenuItems[0].Selected      = TRUE;
	else
		MenuItems[Type+2].Selected = TRUE;

	int rc = FP_Info->Menu(FP_Info->ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT,
	                       FP_GetMsg(MTableTitle), NULL,NULL,NULL,NULL,MenuItems,n+2);

	if(rc == -1)
		return Type;
	else
		return ((WORD)rc) == 0 ? FTP_TYPE_DETECT : ((WORD)rc-2);
}

static BOOL ParseDirLine(Connection *Connect, FTPFileInfo* p)
{
	PROC(("ParseDirLine", "%p", Connect))
	String        Line, Line1;
	FTPDirList    dl;
	FTPServerInfo si;

	while(true)
	{
		Connect->GetOutput(Line);

		if(!Line.Length())
			break;

		if(strstr(Line.c_str(),": Permission denied"))
		{
			SetLastError(ERROR_ACCESS_DENIED);
			return FALSE;
		}

		if(Line.Length() < 20)
			continue;

		//Check contains skipped text
		static LPCSTR FTPMsg[] =
		{
			"data connection",
			"transfer complete",
			"bytes received",
			"DEVICE:[",
			"Total of "
		};
		BOOL Found = FALSE;

		for(size_t n = 0; n < ARRAYSIZE(FTPMsg); n++)
			if(strstr(Line.c_str(),FTPMsg[n]))
			{
				Found = TRUE;
				break;
			}

		if(Found) continue;

		//Check special skip strings
		if(StrCmp(Line.c_str(), "Directory ", 10) == 0 &&
		        strchr(Line.c_str()+10,'[') != NULL)
			continue;

		//Set start detect info
		memset(p, 0, sizeof(*p));
		si.ServerType = Connect->Host.ServerType;
		StrCpy(si.ServerInfo, Connect->SystemInfo, ARRAYSIZE(si.ServerInfo));
		//Use temp buffer
		Line1 = Line;
		//Detect
		WORD     idx;
		FTPType* tp = dl.GetType(Connect->Host.ServerType);

		if(Connect->Host.ServerType == FTP_TYPE_DETECT ||
		        Connect->Host.ServerType == FTP_TYPE_INVALID ||
		        tp == NULL)
		{
			idx = dl.DetectStringType(&si, Line.c_str(), Line.Length());

			if(idx == FTP_TYPE_INVALID || (tp=dl.GetType(idx)) == NULL)
			{
				LogCmd(Message("ParserDETECT: %s->%s [%s]", Parser2Str(Connect->Host.ServerType,&dl), Parser2Str(idx,&dl), Line1.c_str()), ldInt);

				if(Connect->Host.ServerType != FTP_TYPE_DETECT &&
				        Connect->Host.ServerType != FTP_TYPE_INVALID)
				{
					LogCmd(Message("ParserIGNORE: [%s]", Line1.c_str()), ldInt);
					Connect->AddCmdLine(Message("ParserIGNORE: [%s]", Line1.c_str()));
					continue;
				}

				BadFormat(Connect,Line1.c_str(),FALSE);
				break;
			}
			else
			{
				Log(("ParserDETECTED: %s->%s [%s]",Parser2Str(Connect->Host.ServerType,&dl),Parser2Str(idx,&dl),Line1.c_str()));
				Connect->Host.ServerType = idx;
			}
		}
		else
			idx = Connect->Host.ServerType;

		//Use temp buffer
		Line = Line1;
		Log(("toParse: %d,[%s], %d",Line.Length(),Line.c_str()));

		//Parse
		if(!tp->Parser(&si,p,Line.c_str(),Line.Length()))
		{
			LogCmd(Message("ParserFAIL: %s->%s [%s]",
			               Parser2Str(Connect->Host.ServerType,&dl),
			               Parser2Str(idx,&dl),
			               Line1.c_str()),
			       ldInt);
			Connect->AddCmdLine(Message("ParserFAIL: (%s) [%s]", Parser2Str(idx,&dl), Line1.c_str()));
			continue;
		}

		//Skip entryes
		char *CurName = FTP_FILENAME(p);

		if(p->FileType==NET_SKIP || !CurName[0] || StrCmp(CurName,".")==0 || StrCmp(CurName,"..")==0)
			continue;

		//Correct attrs
		if(p->FileType == NET_DIRECTORY ||
		        p->FileType == NET_SYM_LINK_TO_DIR)
			SET_FLAG(p->FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY);

		if(p->FileType == NET_SYM_LINK_TO_DIR ||
		        p->FileType == NET_SYM_LINK)
			SET_FLAG(p->FindData.dwFileAttributes,FILE_ATTRIBUTE_REPARSE_POINT);

		//Convert name text
		Connect->ToOEM(CurName);
		//OK
		return TRUE;
	}

	SetLastError(ERROR_NO_MORE_FILES);
	return FALSE;
}
