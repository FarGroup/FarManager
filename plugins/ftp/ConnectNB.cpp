#include <all_far.h>
#pragma hdrstop

#include "Int.h"

static DWORD tmWait = 0;

int Connection::nb_waitstate(SOCKET *peer, int state,DWORD tmW)
{
	fd_set           _readfds;
	fd_set           _writefds;
	fd_set           _excptfds;
	fd_set           *readfds,*writefds,*excptfds;
	timeval          timeout;
	DWORD        tmB,tmE,tm,tmCheck;
	int              res;
	double           tmDiff;
	BOOL             rc = FALSE;
	String           buff;
	HANDLE           hStart = NULL;
	FPPeriod         hTm;

	if(*peer == INVALID_SOCKET)
		goto Exit;

	if(tmW != MAX_DWORD)
		hTm.Create(tmW);

	timeout.tv_sec  = 0;
	timeout.tv_usec = 10000;
	FP_PeriodReset(hIdle);
	GET_TIME(tmB);
	hStart = Opt.IdleStartPeriod ? FP_PeriodCreate(Opt.IdleStartPeriod) : NULL;

	while(true)
	{
//Check full timeout
		GET_TIME(tmE);
		tmDiff = CMP_TIME(tmE,tmB);

		if(tmDiff > Opt.WaitTimeout)
		{
			Log(("global timeout finished %08X-%08X=%3.3f > %d",tmE,tmB,tmDiff,Opt.WaitTimeout));
			ErrorCode = ERROR_TIMEOUT;
			goto Exit;
		}

		//Empty
		FD_ZERO(&_readfds);
		FD_ZERO(&_writefds);
		FD_ZERO(&_excptfds);
		FD_SET(*peer, &_readfds);
		FD_SET(*peer, &_writefds);
		FD_SET(*peer, &_excptfds);

		if(state == ws_connect || state == ws_write)
		{
			readfds  = NULL;
			writefds = &_writefds;
			excptfds = &_excptfds;
		}
		else if(state == ws_read || state == ws_accept)
		{
			readfds  = &_readfds;
			writefds = NULL;
			excptfds = &_excptfds;
		}
		else if(state == ws_error)
		{
			readfds  = NULL;
			writefds = NULL;
			excptfds = &_excptfds;
		}
		else
		{
			readfds  = &_readfds;
			writefds = &_writefds;
			excptfds = &_excptfds;
		}

//Select
		// in windows nfds ignored
		res = select(0/*ignored*/, readfds, writefds, excptfds, &timeout);
//Idle
		static bool inIdle = false;
		static DWORD idleB;

		if(Opt.WaitIdle && Opt.WaitCounter &&
		        (!inIdle || CMP_TIME(tmE,idleB) > (1.0 / Opt.WaitCounter)))
		{
			Sleep(Opt.WaitIdle);
			idleB  = tmE;
			inIdle = true;
		}

//DISPATCH SELECT
		if(res == 0)
		{
			if(hTm.Handle && hTm.End())
			{
				rc = RPL_TIMEOUT;
				goto Exit;
			}

			if(Opt.ShowIdle && hIdle && FP_PeriodEnd(hIdle) &&
			        (!hStart || FP_PeriodEnd(hStart)))
			{
				if(hStart)
				{
					FP_PeriodDestroy(hStart);
					hStart = NULL;
				}

				if(brk_flag)
					buff.printf("%s (%d%s) ",
					            FP_GetMsg(MWaiting), (int)tmDiff, FP_GetMsg(MSeconds));
				else
					buff.printf("%s (%.1f%s): %2d%% ",
					            FP_GetMsg(MWaiting), tmDiff, FP_GetMsg(MSeconds), (int)(tmDiff*100/Opt.WaitTimeout)+1);

				SetLastError(ERROR_SUCCESS);
				IdleMessage(buff.c_str(),Opt.IdleColor);
			}
		}
		else

//Write
			if(writefds && FD_ISSET(*peer, writefds))
			{
				rc = TRUE;
				goto Exit;
			}
			else

//Read
				if(readfds && FD_ISSET(*peer, readfds))
				{
					rc = TRUE;
					goto Exit;
				}
				else

//Error
					if(res < 0 || (res != 0 && FD_ISSET(*peer,excptfds)))
					{
						int sz = sizeof(SocketError);
						getsockopt(*peer,SOL_SOCKET,SO_ERROR,(char FAR*)&SocketError,&sz);
						Log(("!select: res=%d err: %s",res,GetSocketErrorSTR(SocketError)));
						goto Exit;
					}

//Check cancel
		GET_TIME(tm);
		tmCheck = tm;

		if(CMP_TIME(tm,tmWait) > 0.3 /*&& CMP_TIME(tmCheck,tmB) > 1*/)
		{
			/*
			      BOOL mayAsk = FtpSetBreakable(this,-1) ||                                  // isBreakable
			                    ( (data_peer != INVALID_SOCKET && *peer == data_peer) ||   // Queryed data connection
			                      (data_peer == INVALID_SOCKET) );                         // Data conn does not exist
			*/
			BOOL mayAsk = FtpSetBreakable(this,-1) &&
			              (CurrentState != fcsProcessFile || CMP_TIME(tmCheck,tmB) > 3);

			if(mayAsk)
			{
				tmWait = tm;

				if(CheckForEsc(data_peer == INVALID_SOCKET))
				{
					brk_flag  = TRUE;
					ErrorCode = ERROR_CANCELLED;
					Log(("ESC: Conn breaked by user"));
					goto Exit;
				}
			}
		}
	}/*waiting while(true)*/

Exit:

	if(hStart)
		FP_PeriodDestroy(hStart);

	return rc;
}

BOOL Connection::nb_connect(SOCKET *peer, struct sockaddr FAR* addr, int addrlen)
{
	if(connect(*peer, addr, addrlen) == 0)
		return TRUE;

	if(WSAGetLastError() == WSAEWOULDBLOCK)
		return nb_waitstate(peer, ws_connect);
	else
		return FALSE;
}

int Connection::nb_recv(SOCKET *peer, LPVOID buf, int len, int flags,DWORD tm)
{
	int rc = nb_waitstate(peer, ws_read, tm);

	if(rc == RPL_TIMEOUT)
		return RPL_TIMEOUT;
	else if(rc != 0)
		return recv(*peer, (char*)buf, len, flags);
	else
	{
		Log(("!nb_waitstate"));
		return SOCKET_ERROR;
	}
}

int Connection::nb_send(SOCKET *peer, LPCVOID buf, int len, int flags)
{
	int result = 0;

	while(result == 0)
	{
		result = send(*peer, (char*)buf, len, flags);

		if(result            == SOCKET_ERROR &&
		        WSAGetLastError() == WSAEWOULDBLOCK)
		{
#if 0
			fd_set  _readfds;
			timeval timeout;

			do
			{
				FD_ZERO(&_readfds);
				FD_SET(*peer, &_readfds);
				timeout.tv_sec = 0;
				timeout.tv_usec = 10000; /* 10 ms */
				result = select((*peer) + 1, &_readfds, 0, 0, &timeout);

				if(result == 1)
				{
					if(!nb_waitstate(peer, ws_write))
						return SOCKET_ERROR;

					break;
				}
				else if(result == 0)
					continue;
				else
					return SOCKET_ERROR;
			}
			while(true);

#else

			if(!nb_waitstate(peer, ws_write))
				return SOCKET_ERROR;

#endif
			result = 0;
		}
	}

	return result;
}
