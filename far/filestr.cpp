/*
filestr.cpp

Класс GetFileString

*/

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"
#include "global.hpp"
#include "filestr.hpp"

GetFileString::GetFileString(FILE *SrcFile)
{
	Str=(char*)xf_malloc(1024);
	StrLength=1024;
	GetFileString::SrcFile=SrcFile;
	ReadPos=ReadSize=0;
}


GetFileString::~GetFileString()
{
	if (Str) xf_free(Str);
}


int GetFileString::GetString(char **DestStr,int &Length)
{
	int CurLength=0;
	int ExitCode=1;
	int Eol=0;
	char EOL[16];
	char *PtrEol=EOL;
	int x=0;
	memset(EOL,0,sizeof(EOL));
	Length=0;

	while (1)
	{
		if (ReadPos>=ReadSize)
		{
			if ((ReadSize=(int)fread(ReadBuf,1,sizeof(ReadBuf),SrcFile))==0)
			{
				if (CurLength==0)
					ExitCode=0;

				break;
			}

			ReadPos=0;
		}

		int Ch=ReadBuf[ReadPos]; // Mantis#295
#if 0

		if (Ch!='\n' && CurLength>0 && Str[CurLength-1]=='\r')
			break;

#else

		if ((Eol && Ch != '\n' && Ch != '\r') || Eol >= (int)sizeof(EOL))
			break;

//    if(Eol && (Ch == '\n' || Ch == '\r') && (!strcmp(EOL,DOS_EOL_fmt) || !strcmp(EOL,UNIX_EOL_fmt) || !strcmp(EOL,WIN_EOL_fmt)))
		if (Eol && (Ch == '\n' || Ch == '\r') && (!strcmp(EOL,WIN_EOL_fmt) || !strcmp(EOL,DOS_EOL_fmt) || !strcmp(EOL,UNIX_EOL_fmt)))
			break;

		if (Ch == '\n' || Ch == '\r')
		{
			*PtrEol++=Ch;
			Eol++;
		}

#endif
		ReadPos++; // Mantis#295

		if (CurLength>=StrLength-1)
		{
			char *NewStr=(char *)xf_realloc(Str,StrLength+(1024<<x));

			if (NewStr==NULL)
				return(-1);

			Str=NewStr;
			StrLength+=1024<<x;
			x++;
		}

		Str[CurLength++]=Ch;
#if 0

		if (Ch=='\n')
			break;

#else

		if (Eol && (Ch == '\n' || Ch == '\r') && (!strcmp(EOL,WIN_EOL_fmt) || !strcmp(EOL,DOS_EOL_fmt) || !strcmp(EOL,UNIX_EOL_fmt)))
			break;

#endif
	}

	Str[CurLength]=0;
	*DestStr=Str;
	Length=CurLength;
	return(ExitCode);
}
