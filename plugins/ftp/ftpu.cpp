#include <all_lib.h>
#pragma hdrstop

#include "pwd.h"
#include "Crypt.inc"

void PPwd(LPBYTE b)
{
	for(int n = 0; n < FTP_PWD_LEN; n++)
	{
		if(n && n%16 == 0) printf("\n");

		printf(" %02X",b[n]);
	}
}

BYTE HexDigit(char ch)
{
	if(ch >= 'a' && ch <= 'f') return ch - 'a' + 10;

	if(ch >= 'A' && ch <= 'F') return ch - 'A' + 10;

	if(ch >= '0' && ch <= '9') return ch - '0';

	HAbort("Unknown hex digit \"%c\"", ch);
	return 0;
}

void ReadHEXPwd(LPBYTE b, LPCSTR src)
{
	for(int n = 0; *src; n++)
	{
		if(n >= FTP_PWD_LEN)
			HAbort("Password code too long!");

		if(*src == ',' || isspace(*src))
		{
			src++;
			continue;
		}

		*b++ = (HexDigit(src[0]) << 4) + HexDigit(src[1]);
		src += 2;
	}
}

void ReadDUMPPwd(BYTE *b, LPCSTR src)
{
	ReadHEXPwd(b, src);
}

void ReadPwd(BYTE *b, LPCSTR src)
{
	memset(b, 0, FTP_PWD_LEN);

	if(StrCmp(src,"hex:", 4, FALSE) == 0)
		ReadHEXPwd(b, src+4);
	else
		ReadDUMPPwd(b, src);
}

void main(int argc,char *argv[])
{
	CTArgInit(argc,argv,FALSE);

	if(argc < 2 || CTArgCheck("?"))
	{
		printf("USAGE: FTPU <command>\n"
		       "Where commands are:\n"
		       " -<H>ash=<text>    - creates password hash\n"
		       " -<P>wd=<hash>     - decode password from hash\n"
		       " -<P>wd=hex:<hash> - decode password from FTP file or registry format\n"
		       "");
		return;
	}

	LPCSTR m;
	BYTE     b[FTP_PWD_LEN];
	char     str[MAX_PATH];

	if((m=CTArgGet("h;hash")) != NULL)
	{
		MakeCryptPassword(m, b);
		printf("Hash for \"%s\" is:\n",m);
		PPwd(b);
	}
	else if((m=CTArgGet("p;pwd")) != NULL)
	{
		ReadPwd(b,m);
		DecryptPassword(b, str);
		printf("The password is: [%s]\n",str);
	}
	else
		printf("Command missing or mitsmatch!\n");;
}
