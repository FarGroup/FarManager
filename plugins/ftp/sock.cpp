#include <all_far.h>
#pragma hdrstop

#include "Int.h"

void WINAPI scClose(SOCKET& sock,int how)
{
	DWORD err = GetLastError();
	int   serr = WSAGetLastError();

	if(sock != INVALID_SOCKET)
	{
		Log(("SOCK: closed %d as %d",sock,how));

		if(how != -1)
			shutdown(sock,how);

		int rc = closesocket(sock);
		Log(("SOCK: closesocket()=%d (%s)", rc, GetSocketErrorSTR()));

		if(rc && WSAGetLastError() == WSAEWOULDBLOCK)
		{
			linger l;
			l.l_onoff  = 0;
			l.l_linger = 0;

			if((rc=setsockopt(sock,SOL_SOCKET,SO_LINGER,(char FAR *)&l,sizeof(l))) != 0)
			{
				Log(("SOCK: Error set linger: %d,%s", rc, GetSocketErrorSTR()));
			}
			else
			{
				rc = closesocket(sock);
				Log(("SOCK: 2 closesocket()=%d (%s)", rc, GetSocketErrorSTR()));
			}
		}

		sock = INVALID_SOCKET;
	}

	SetLastError(err);
	WSASetLastError(serr);
}

BOOL WINAPI scValid(SOCKET sock)
{
	return sock != INVALID_SOCKET;
}

SOCKET WINAPI scCreate(short addr_type)
{
	SOCKET s;
	int    iv;
	u_long ul;
	s = socket(addr_type,SOCK_STREAM,0);

	if(!scValid(s)) return INVALID_SOCKET;

	Log(("SOCK: created %d",s));

	do
	{
		iv = TRUE;

		if(setsockopt(s,SOL_SOCKET,SO_DONTLINGER,(char FAR *)&iv,sizeof(iv)) != 0) break;

		iv = TRUE;

		if(setsockopt(s,SOL_SOCKET,SO_DONTROUTE,(char FAR *)&iv,sizeof(iv)) != 0) break;

		iv = TRUE;

		if(setsockopt(s,IPPROTO_TCP,TCP_NODELAY,(char FAR *)&iv,sizeof(iv)) != 0) break;

#ifdef SO_OOBINLINE
		iv = 1;

		if(setsockopt(s, SOL_SOCKET, SO_OOBINLINE, (char FAR *)&iv, sizeof(iv)) != 0) break;

#endif
		ul = 1;

		if(ioctlsocket(s,FIONBIO,&ul) != 0) break;

		return s;
	}
	while(0);

	scClose(s);
	return INVALID_SOCKET;
}

SOCKET WINAPI scAccept(SOCKET *peer, struct sockaddr FAR* addr, int* addrlen)
{
	SOCKET s = accept(*peer, addr, addrlen);

	if(!scValid(s)) return INVALID_SOCKET;

	Log(("SOCK: accepted %d at %d",s,*peer));
	return s;
}
