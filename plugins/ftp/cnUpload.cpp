#include <all_far.h>
#pragma hdrstop

#include "Int.h"

//--------------------------------------------------------------------------------
void Connection::sendrequest(char *cmd, char *local, char *remote)
{
	//??SaveConsoleTitle _title;
	sendrequestINT(cmd,local,remote);
	IdleMessage(NULL,0);
}

void Connection::sendrequestINT(char *cmd, char *local, char *remote)
{
	PROC(("sendrequestINT","%s,%s,%s",cmd,local,remote))

	if(type == TYPE_A)
		restart_point=0;

	WIN32_FIND_DATA   ffi;
	FHandle           fin;
	SOCKET            dout = 0;
	LONG              hi;
	FTPCurrentStates  oState = CurrentState;
	BOOL              oldBrk = FtpSetBreakable(this, -1);
	__int64           fsz;
	FTNNotify         ni;
	ni.Upload       = TRUE;
	ni.Starting     = TRUE;
	ni.Success      = TRUE;
	ni.RestartPoint = restart_point;
	ni.Port         = ntohs(portnum);
	ni.Password[0] = 0; //StrCpy( ni.Password, UserPassword, ARRAYSIZE(ni.Password));
	StrCpy(ni.User, UserName, ARRAYSIZE(ni.User));
	StrCpy(ni.HostName, hostname, ARRAYSIZE(ni.HostName));
	StrCpy(ni.LocalFile, local, ARRAYSIZE(ni.LocalFile));
	StrCpy(ni.RemoteFile, remote, ARRAYSIZE(ni.RemoteFile));

	if(proxy)
	{
		proxtrans(cmd, local, remote);
		return;
	}

	HANDLE ff = FindFirstFile(local,&ffi);

	if(ff == INVALID_HANDLE_VALUE)
	{
		ErrorCode = ERROR_OPEN_FAILED;
		SysError = TRUE;
		return;
	}

	FindClose(ff);

	if(IS_FLAG(ffi.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY))
	{
		ErrorCode = ERROR_OPEN_FAILED;
		SysError = TRUE;
		Log(("local is directory [%s]",local));

		if(!ConnectMessage(MNotPlainFile,local,-MRetry))
			ErrorCode = ERROR_CANCELLED;

		return;
	}

	fin.Handle = Fopen(local,"r");

	if(!fin.Handle)
	{
		Log(("!open local"));
		ErrorCode = ERROR_OPEN_FAILED;
		SysError = TRUE;

		if(!ConnectMessage(MErrorOpenFile,local,-MRetry))
			ErrorCode = ERROR_CANCELLED;

		return;
	}

	fsz = Fsize(fin.Handle);

	if(restart_point && fsz == restart_point)
	{
		AddCmdLine(FMSG(MFileComplete));
		ErrorCode = ERROR_SUCCESS;
		return;
	}

	if(!initconn())
	{
		Log(("!initconn"));
		return;
	}

	if(Host.SendAllo)
	{
		if(cmd[0] != Opt.cmdAppe[0])
		{
			__int64 v = ((__int64)ffi.nFileSizeHigh) << 32 | ffi.nFileSizeLow;
			Log(("ALLO %I64u", v));

			if(command("%s %I64u",Opt.cmdAllo,v) != RPL_COMPLETE)
			{
				Log(("!allo"));
				return;
			}
		}
	}

	if(restart_point)
	{
		if(!ResumeSupport && cmd[0] != Opt.cmdAppe[0])
		{
			AddCmdLine(FMSG(MResumeRestart));
			restart_point = 0;
		}
		else
		{
			Log(("restart_point %I64u",restart_point));

			if(!Fmove(fin.Handle, restart_point))
			{
				SysError = TRUE;
				ErrorCode = GetLastError();
				Log(("!setfilepointer(%I64u)",restart_point));

				if(!ConnectMessage(MErrorPosition,local,-MRetry))
					ErrorCode = ERROR_CANCELLED;

				return;
			}

			if(cmd[0] != Opt.cmdAppe[0] &&
			        command("%s %I64u",Opt.cmdRest,restart_point) != RPL_CONTINUE)
				return;

			TrafficInfo->Resume(restart_point);
		}
	}

	if(Host.PassiveMode)
	{
		Log(("pasv"));
		dout = dataconn();

		if(dout == INVALID_SOCKET)
			goto abort;
	}

	if(remote)
	{
		Log(("Upload remote [%s]",remote));

		if(command("%s %s", cmd, remote) != RPL_PRELIM)
		{
			return;
		}
	}
	else
	{
		Log(("!remote"));

		if(command("%s", cmd) != RPL_PRELIM)
		{
			return;
		}
	}

	if(!Host.PassiveMode)
	{
		dout = dataconn();

		if(dout == INVALID_SOCKET) goto abort;
	}

	switch(type)
	{
		case TYPE_I:
		case TYPE_L:
		case TYPE_A:

			if(fsz != 0)
			{
				unsigned rsz = Host.IOBuffSize;

				if(rsz < fsz && (rsz & 511) && rsz > 512) rsz &= ~511;

				{
					unsigned osz = 0, sz = sizeof(osz);

					if(getsockopt(dout, SOL_SOCKET, SO_SNDBUF, (char*)&osz, (int*)&sz))
						osz = 8*1024; // default - 8K

					if(rsz >= osz && fsz >= osz)
					{
						if(rsz > 1024*1024) rsz = 1024*1024;  // 1M

						osz = rsz;

						if(fsz < osz) osz = (unsigned)fsz;

						++osz;
						setsockopt(dout, SOL_SOCKET, SO_SNDBUF, (char*)&osz, sizeof(osz));
					}
				}

				if(rsz >= fsz || rsz > 64*1024)       // 64K (start winsize)
				{
					BOOL one = TRUE;
					setsockopt(dout, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(one));
				}

				if(PluginAvailable(PLUGIN_NOTIFY)) FTPNotify().Notify(&ni);

				Log(("Uploading %s->%s from %I64u",local,remote,restart_point));
				FTPConnectionBreakable _brk(this,FALSE);
				CurrentState = fcsProcessFile;
				//-------- READ
				DWORD ind = 0;
				DWORD b,e;
				GET_TIME(b);

				while(true)
				{
					hi = 0;
					Log(("read %d",rsz));

					if(!ReadFile(fin.Handle,IOBuff,rsz,(LPDWORD)&hi,NULL))
					{
						SysError = TRUE;
						ErrorCode = GetLastError();
						Log(("pf: !read buff"));
						goto abort;
					}

					if(hi == 0)
					{
						ErrorCode = GetLastError();
						SysError = ErrorCode != ERROR_SUCCESS;
						break;
					}

					//-------- SEND
					LONG  d;
					char *bufp;
					Log(("doSend"));

					for(bufp = IOBuff; hi > 0; hi -= d, bufp += d)
					{
						Log(("ndsend %d",hi));

						if((d=(LONG)nb_send(&dout, bufp,(int)hi, 0)) <= 0)
						{
							Log(("pf(%d,%s): !send %d!=%d",code,GetSocketErrorSTR(),d,hi));
							code = RPL_TRANSFERERROR;
							goto abort;
						}

						Log(("sent %d",d));
						ind += d;
						GET_TIME(e);

						if(CMP_TIME(e,b) >= 0.5)
						{
							b = e;
							d = ind;
							ind = 0;

							if(IOCallback && !TrafficInfo->Callback((int)d))
							{
								Log(("pf(%d,%s): canceled",code,GetSocketErrorSTR()));
								ErrorCode = ERROR_CANCELLED;
								goto abort;
							}
						}
					}//-- SEND

					Log(("sended"));
					//Sleep(1);
				}//-- READ

				if(IOCallback) TrafficInfo->Callback(0);

				Log(("done"));
			} /*fsz != 0*/

			break;
	}/*SWITCH*/

//NormExit
	FtpSetBreakable(this, oldBrk);
	CurrentState = oState;

	if(data_peer != INVALID_SOCKET)
	{
		scClose(data_peer,1);

		if(getreply(0) > RPL_COMPLETE)
		{
			ErrorCode = ERROR_WRITE_FAULT;
		}
	}
	else
		getreply(0);

	if(PluginAvailable(PLUGIN_NOTIFY))
	{
		ni.Starting = FALSE;
		ni.Success  = TRUE;
		FTPNotify().Notify(&ni);
	}

	return;
abort:
	FtpSetBreakable(this, oldBrk);
	CurrentState = oState;

	if(!cpend)
	{
		Log(("!!!cpend"));
	}

	int ocode = code,
	    oecode = ErrorCode;
	scClose(data_peer, SD_SEND);

	if(!SendAbort(data_peer))
	{
		Log(("!send abort"));
		lostpeer();
	}
	else
	{
		setascii();
		ProcessCommand("pwd");
		code      = ocode;
		ErrorCode = oecode;
	}

	if(PluginAvailable(PLUGIN_NOTIFY))
	{
		ni.Starting = FALSE;
		ni.Success  = FALSE;
		FTPNotify().Notify(&ni);
	}
}
