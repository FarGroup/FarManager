#include <all_far.h>
#pragma hdrstop

#include "Int.h"

//--------------------------------------------------------------------------------
static unsigned get_cluster_size(const char *fname)
{
	char root[MAX_PATH];

	if(GetFullPathName(fname, ARRAYSIZE(root), root, NULL)
	        && root[1] == ':' && root[2] == '\\')
	{
		DWORD t1, t2, bps = 0, spc = 0;
		root[3] = '\0';
		spc = bps = 0;

		if(GetDiskFreeSpace(root, &spc, &bps, &t1, &t2)) return(spc*bps);
	}

	return(0);
}

//--------------------------------------------------------------------------------
void Connection::recvrequest(char *cmd, char *local, char *remote, const char *mode)
{
	//??SaveConsoleTitle _title;
	recvrequestINT(cmd,local,remote,mode);
	IdleMessage(NULL,0);
}

void Connection::recvrequestINT(char *cmd, char *local, char *remote, const char *mode)
{
	int              oldtype = 0,
	                 is_retr;
	FHandle          fout;
	SOCKET           din = INVALID_SOCKET;
	int              ocode, oecode;
	BOOL             oldBrk = FtpSetBreakable(this, -1);
	FTPCurrentStates oState = CurrentState;
	FTNNotify        ni;

	if(type == TYPE_A)
		restart_point = 0;

	ni.Upload       = FALSE;
	ni.Starting     = TRUE;
	ni.Success      = TRUE;
	ni.RestartPoint = restart_point;
	ni.Port         = ntohs(portnum);
	ni.Password[0] = 0; //StrCpy( ni.Password,   UserPassword, ARRAYSIZE(ni.Password));
	StrCpy(ni.User,       UserName, ARRAYSIZE(ni.User));
	StrCpy(ni.HostName,   hostname, ARRAYSIZE(ni.HostName));
	StrCpy(ni.LocalFile,  local, ARRAYSIZE(ni.LocalFile));
	StrCpy(ni.RemoteFile, remote, ARRAYSIZE(ni.RemoteFile));

	if(local[0] == '-' && local[1] == 0)
	{
		;
	}
	else
	{
		fout.Handle = Fopen(local, mode, Opt.SetHiddenOnAbort ? FILE_ATTRIBUTE_HIDDEN : FILE_ATTRIBUTE_NORMAL);

		if(!fout.Handle)
		{
			ErrorCode = GetLastError();
			SysError = TRUE;
			Log(("!Fopen [%s] %s",mode,__WINError()));

			if(!ConnectMessage(MErrorOpenFile,local,-MRetry))
				ErrorCode = ERROR_CANCELLED;

			//goto abort;
			return;
		}

		Log(("recv file [%d] \"%s\"=%p",Host.IOBuffSize,local,fout.Handle));

		if(restart_point != -1)
		{
			if(!Fmove(fout.Handle,restart_point))
			{
				ErrorCode = GetLastError();
				SysError = TRUE;

				if(!ConnectMessage(MErrorPosition,local,-MRetry))
					ErrorCode = ERROR_CANCELLED;

				return;
			}
		}

		TrafficInfo->Resume(restart_point == -1 ? 0 : restart_point);
	}

	is_retr = StrCmp(cmd,Opt.cmdRetr) == 0;

	if(proxy && is_retr)
	{
		proxtrans(cmd, local, remote);
		return;
	}

	if(!initconn())
	{
		Log(("!initconn"));
		return;
	}

	if(!is_retr)
	{
		if(type != TYPE_A)
		{
			oldtype = type;
			setascii();
		}
	}
	else if(restart_point)
	{
		if(!ResumeSupport)
		{
			AddCmdLine(FMSG(MResumeRestart));
			restart_point = 0;
		}
		else if(restart_point != -1)
		{
			if(command("%s %I64u",Opt.cmdRest,restart_point) != RPL_CONTINUE)
			{
				Log(("!restart SIZE"));
				return;
			}
		}
	}

	if(Host.PassiveMode)
	{
		din = dataconn();

		if(din == INVALID_SOCKET)
		{
			Log(("!dataconn: PASV ent"));
			goto abort;
		}
	}

	if(remote)
	{
		if(command("%s %s", cmd, remote) != RPL_PRELIM)
		{
			if(oldtype)
				SetType(oldtype);

			Log(("!command [%s]",cmd));
			fout.Close();

			if(Fsize(local))
				DeleteFile(local);

			return;
		}
	}
	else if(command("%s", cmd) != RPL_PRELIM)
	{
		if(oldtype)
			SetType(oldtype);

		return;
	}

	if(!Host.PassiveMode)
	{
		din = dataconn();

		if(din == INVALID_SOCKET)
		{
			Log(("!dataconn: PASV ret"));
			goto abort;
		}
	}

	/**/

	switch(type)
	{
		case TYPE_A:
		case TYPE_I:
		case TYPE_L:
		{
			FtpSetBreakable(this, FALSE);
			CurrentState = fcsProcessFile;

			if(fout.Handle && PluginAvailable(PLUGIN_NOTIFY))
				FTPNotify().Notify(&ni);

			DWORD b,e,bw;
			__int64 totalValue;
			int b_done;
			DWORD ind;
			int b_ost = Host.IOBuffSize, wsz = get_cluster_size(local)*2;

			if(!wsz || (wsz > b_ost && (wsz /= 2) > b_ost)) wsz = 512;

			ind = Min(1024*1024, Max(4*wsz, 256*1024));  // 256K - 1M
			setsockopt(din, SOL_SOCKET, SO_RCVBUF, (char*)&ind, sizeof(ind));
			b_done = ind = 0;
			totalValue = 0;
			bool unalign = false;
			GET_TIME(b);
			bw = b;

			while(true)
			{
				int c;

				if(wsz != 512 && b_done >= wsz)       // pseudo ansync io
				{
					DWORD off = 0, rdy = 0, ost = b_done % wsz, top = b_done - ost;

					while(ioctlsocket(din, FIONREAD, &rdy) && !rdy)
					{
						if(Fwrite(fout.Handle,IOBuff+off,wsz) != wsz) goto write_error;

						if((off += wsz) >= top) break;
					}

					if(off)
					{
						b_done -= off;

						if(b_done) memmove(IOBuff, IOBuff+off, b_done);

						b_ost = Host.IOBuffSize - b_done;
					}
				}

				//Recv
				c = nb_recv(&din, IOBuff+b_done, b_ost, 0);

				if(c <= 0)
				{
					if(b_done && Fwrite(fout.Handle,IOBuff,b_done) != b_done) goto write_error;

					if(c < 0)
					{
						Log(("gf(%d,%s)=%I64u: !read buff",code,GetSocketErrorSTR(),totalValue));
						code = RPL_TRANSFERERROR;
						goto NormExit;
					}

					Log(("gf(%d,%s)=%I64u: read zero",code,GetSocketErrorSTR(),totalValue));
					break;
				}

				totalValue += c;
				GET_TIME(e);

				if(!fout.Handle)
				{
					//Add readed to buffer
					Log(("AddOutput: +%d bytes", c));
					AddOutput((BYTE*)IOBuff,c);
				}
				else      //Write to file
				{
					b_done += c;
					b_ost -= c;

					if(b_ost < wsz || CMP_TIME(e,bw) >= 3.0)
					{
						DWORD ost = 0;

						if(wsz == 512 || b_done <= wsz)    // timeout or very small buffer
						{
							if(Fwrite(fout.Handle,IOBuff,b_done) != b_done) goto write_error;

							if(b_done < wsz) unalign = true;  // flag of timeout witing (optimize)
						}
						else
						{
							// scatter-gatter for RAID in win32 is very bad on large buffer
							// and when work without RAID synchronous write speed is independ
							// if buffer size is >= 2*cluster size
							int off = 0;

							if(unalign)    // was 'timeouted unaligned write'
							{
								unalign = false;
								off = (DWORD)(totalValue % wsz);

								if(off)
								{
									if(Fwrite(fout.Handle,IOBuff,off) != off) goto write_error;

									b_done -= off;

									if(b_done < wsz)
									{
										memmove(IOBuff, IOBuff+off, b_done);
										goto skip_sg;
									}
								}
							}

							ost = b_done % wsz;
							b_done -= ost;

							do
								if(Fwrite(fout.Handle,IOBuff+off,wsz) != wsz) goto write_error;

							while((off += wsz) < b_done);

							if(ost) memmove(IOBuff, IOBuff+off, ost);
						}

						b_done = ost;
skip_sg:
						b_ost = Host.IOBuffSize - b_done;
						GET_TIME(e);
						bw = e;
					}
				}

				ind += c;

				if(CMP_TIME(e,b) >= 0.5)
				{
					b = e;
					c = ind;
					ind = 0;

					//Call user CB
					if(IOCallback)
					{
						if(!TrafficInfo->Callback(c))
						{
							Log(("gf: canceled by CB"));
do_cancel:
							ErrorCode = ERROR_CANCELLED;

							if(b_done && Fwrite(fout.Handle,IOBuff,b_done) != b_done)
							{
write_error:
								SysError = TRUE;
								ErrorCode = GetLastError();

								if(ErrorCode == ERROR_SUCCESS)
									ErrorCode = ERROR_WRITE_FAULT;  // for non equal counter

								Log(("!write local"));
							}

							goto abort;
						}
					}
					else

						//Show Quite progressing
						if(Opt.ShowIdle && !remote)
						{
							char   digit[ 20 ];
							String str;
							str.printf("%s%s ", FP_GetMsg(MReaded), FCps(digit,(double)totalValue));
							SetLastError(ERROR_SUCCESS);
							IdleMessage(str.c_str(),Opt.ProcessColor);

							if(CheckForEsc(FALSE)) goto do_cancel;
						}
				}
			}

			if(IOCallback)
				TrafficInfo->Callback(0);

			break;
		}
	}

NormExit:
	FtpSetBreakable(this, oldBrk);
	ocode              = code;
	oecode             = ErrorCode;
	CurrentState       = oState;
	scClose(data_peer,-1);

	if(getreply(FALSE) == RPL_ERROR ||
	        oldtype && !SetType(oldtype))
	{
		lostpeer();
	}
	else
	{
		code      = ocode;
		ErrorCode = oecode;
	}

	if(fout.Handle && PluginAvailable(PLUGIN_NOTIFY))
	{
		ni.Starting = FALSE;
		ni.Success  = TRUE;
		FTPNotify().Notify(&ni);
	}

	return;
abort:
	FtpSetBreakable(this, oldBrk);

	if(!cpend)
	{
		Log(("!!!cpend"));
	}

	ocode        = code;
	oecode       = ErrorCode;
	CurrentState = oState;

	if(!SendAbort(din) ||
	        (oldtype && !SetType(oldtype)))
		lostpeer();
	else
	{
		code      = ocode;
		ErrorCode = oecode;
	}

	scClose(data_peer,-1);

	if(fout.Handle && PluginAvailable(PLUGIN_NOTIFY))
	{
		ni.Starting = FALSE;
		ni.Success  = FALSE;
		FTPNotify().Notify(&ni);
	}

	return;
}

/* abort using RFC959 recommended ffIP,SYNC sequence  */
/*
  WarFTP
  |->âABOR
  |<-226 Transfer complete. 39600000 bytes in 18.84 sec. (2052.974 Kb/s)
  |<-225 ABOR command successful.

  FreeBSD, SunOS
  |->òABOR
  |<-426 Transfer aborted. Data connection closed.
  |<-226 Abort successful

  QNX
  |->âABOR
  |<-226 Transfer complete.
  |<-225 ABOR command successful.

  IIS
  |->âABOR
  |<-225 ABOR command successful.

  QNX
  |->òABOR
  |<-452 Error writing file: No child processes.
  |<-225 ABOR command successful.

  GuildFTP (have no ABOR notify at all)
  -> <ABORT>
  <- 226 Transfer complete. 11200000 bytes in 2 sec. (5600.00 Kb/s).

  ??
  ->òABOR
  <-450 Transfer aborted. Link to file server lost.
  <-500 ÿôÿòABOR not understood
*/
BOOL Connection::SendAbort(SOCKET din)
{
	do
	{
		if(!fprintfSocket(cout,"%c%c",ffIAC,ffIP))
		{
			Log(("!send ffIAC"));
			break;
		}

		char msg = ffIAC;

		/* send ffIAC in urgent mode instead of DM because UNIX places oob mark */
		/* after urgent byte rather than before as now is protocol            */
		if(nb_send(&cout,&msg,1,MSG_OOB) != 1)
		{
			Log(("rr: !send urgent"));
			break;
		}

		if(!fprintfSocket(cout,"%cABOR\r\n",ffDM))
		{
			Log(("!send ABOR"));
			break;
		}

		scClose(data_peer,-1);
		int res;
		Log(("Wait ABOT reply"));
		res = getreply(FALSE);

		if(res == RPL_TIMEOUT)
		{
			Log(("Error waiting first ABOR reply"));
			return FALSE;
		}

		//Single line
		if(code == 225)
		{
			Log(("Single line ABOR reply"));
			return TRUE;
		}
		else

			//Wait OPTIONAL second line
			if(code == 226)
			{
				Log(("Wait OPT second reply"));

				do
				{
					res = getreply(FALSE,500);

					if(res == RPL_TIMEOUT)
					{
						Log(("Timeout: res: %d, code: %d", res, code));
						return TRUE;
					}
					else if(res == RPL_ERROR)
					{
						Log(("Error: res: %d, code: %d", res, code));
						return FALSE;
					}

					Log(("Result: res: %d, code: %d", res, code));
				}
				while(true);
			}
			else
			{
				//Wait second line
				Log(("Wait second reply"));
				res = getreply(FALSE);

				if(res == RPL_TIMEOUT)
				{
					Log(("Error waiting second ABOR reply"));
					return FALSE;
				}

				Log(("Second reply: res: %d, code: %d", res,code));
				return TRUE;
			}
	}
	while(0);  /*Error sending ABOR*/

	return FALSE;
}
