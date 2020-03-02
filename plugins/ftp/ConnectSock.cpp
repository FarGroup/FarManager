#include <all_far.h>
#pragma hdrstop

#include "Int.h"

int Connection::fgetcSocket(SOCKET s,DWORD tm)
{
	int c;
	char buffer;
	c = nb_recv(&s, &buffer, 1, 0,tm);

	if(c == SOCKET_ERROR || c < 0)
	{
		Log(("Error receiving: char: %02X (se: %02X) %s",c,SOCKET_ERROR,GetSocketErrorSTR()));
		return 0;
	}
	else if(c == RPL_TIMEOUT)
		return RPL_TIMEOUT;
	else if(c == 0)
	{
		Log(("End of recv c=0"));
		return ffEOF;
	}
	else
		return (int)buffer;
}

BOOL Connection::fprintfSocket(SOCKET s, LPCSTR format, ...)
{
	va_list  argptr;
	String   buffer;
	va_start(argptr, format);
	buffer.vprintf(format, argptr);
	va_end(argptr);
	return fputsSocket(buffer.c_str(), s);
}

BOOL Connection::fputsSocket(LPCSTR str, SOCKET s)
{
	String buf;

	if(!str[0])
		return TRUE;

	AddCmdLine(str);

	if(!Host.FFDup)
	{
		int len = static_cast<int>(strlen(str));
		return nb_send(&s, str, len, 0) == len;
	}

	for(int n = 0; str[n]; n++)
		if(str[n] == '\xFF')
		{
			buf.Add('\xFF');
			buf.Add('\xFF');
		}
		else
			buf.Add(str[n]);

	return nb_send(&s, buf.c_str(), buf.Length(), 0) == buf.Length();
}
