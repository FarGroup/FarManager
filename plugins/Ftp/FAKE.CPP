#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <winsock.h>

#define MAX_ASCII 100


int sleep(int time)
{
   return time;
}


int gettimeofday(struct timeval *timenow, struct timezone *tz)
{
	time_t t;

	t = clock();

	timenow->tv_usec = t;
	timenow->tv_sec = t / CLK_TCK;

	return 0;
}



int fgetcSocket(int s)
{
   int c;
   char buffer[10];

   c = recv(s, buffer, 1, 0);

   if (c == INVALID_SOCKET)
      return c;

   if (c == 0)
      return EOF;

   return buffer[0];
}

char *fprintfSocket(int s, char *format, ...)
{
   va_list argptr;
   char buffer[10009];

   va_start(argptr, format);
   vsprintf(buffer, format, argptr);
   va_end(argptr);

   send(s, buffer, strlen(buffer), 0);

   return NULL;
}

char *fputsSocket(char *format, int s)
{
   send(s, format, strlen(format), 0);

   return NULL;
}

int fputcSocket(int s, char putChar)
{
   char buffer[2];

   buffer[0] = putChar;
   buffer[1] = '\0';

   send(s, buffer, 1, 0);

   return putChar;
}

