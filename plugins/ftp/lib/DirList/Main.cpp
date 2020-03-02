#include <all_far.h>
#pragma hdrstop

#include "Int.h"

//------------------------------------------------------------------------
char *SkipSpace(char *l)        { while(*l && NET_IS_SPACE(*l))  l++; return l; }
char *SkipDigit(char *l)        { while(*l && NET_IS_DIGIT(*l))  l++; return l; }
char *SkipNSpace(char *l)       { while(*l && !NET_IS_SPACE(*l)) l++; return l; }
char *SkipNX(char *l, char ch)  { while(*l && *l != ch) l++; return l; }

/*
 * Get month number (-1 on error).
 */
BOOL TwoDigits(LPCSTR s,WORD& val)
{
	val = 0;

	if(!NET_IS_DIGIT(s[1]))
		return FALSE;

	if(NET_IS_SPACE(*s))
		val = ((WORD)(s[0]-'0'));
	else
		val = ((WORD)(s[0]-'0'))*10 + ((WORD)(s[1]-'0'));

	return TRUE;
}

/* remove front and back white space
 * modifies the original string
 */
char *XP_StripLine(char *string)
{
	char *ptr;

	/* remove leading blanks */
	while(*string=='\t' || *string==' ' || *string=='\r' || *string=='\n')
		string++;

	//Find end of string
	for(ptr=string; *ptr; ptr++);

	/* remove trailing blanks */
	for(ptr--; ptr >= string; ptr--)
	{
		if(*ptr=='\t' || *ptr==' ' || *ptr=='\r' || *ptr=='\n')
			*ptr = '\0';
		else
			break;
	}

	return string;
}

/*
 * Get month number by month name in range [1..12] or MAX_WORD on error.
 */
WORD NET_MonthNo(LPCSTR month)
{
	char str[200],
	     mn[ 50 ],
	     *b,*e;

	for(WORD n = 0; n < 12; n++)
	{
		StrCpy(str, (char*)FTP_Info->GetOpt()->Months[n], ARRAYSIZE(str));
		b = str;
		e = strchr(str, ';');

		if(e == NULL)
		{
			if(StrNCmpI(month,str,static_cast<int>(strlen(str))) == 0)
				return n+1;

			continue;
		}

		while(e)
		{
			StrCpy(mn, b, Min((int)ARRAYSIZE(mn),(int)(e-b+1)));

			if(StrNCmpI(month,mn,static_cast<int>(strlen(mn))) == 0)
				return n+1;

			b = e+1;
			e = strchr(b,';');
		}
	}

	return MAX_WORD;
}

/*
 * Converts PNET_FileEntryInfo fields to FTPFileInfo*.
 */

BOOL CorrectTime(SYSTEMTIME& st,Time_t& dt, FILETIME *wdt)
{
	SYSTEMTIME ftm;

	if(dt == NOT_TIME)
		memset(wdt, 0, sizeof(*wdt));
	else if(dt == 0)
		SystemTimeToFileTime(&st, dt);

	if(!LocalFileTimeToFileTime(dt, wdt))
		return FALSE;

	if(dt == NOT_TIME)
	{
		;
	}
	else
	{
		FILETIME ttm, ltm, t1;
		SystemTimeToFileTime(&st, &ttm);
		LocalFileTimeToFileTime(&ttm, &ltm);

		while(CompareFileTime(wdt, &ltm) > 0)
		{
			FileTimeToLocalFileTime(wdt, &t1);
			FileTimeToSystemTime(&t1, &ftm);

			if(ftm.wDay == 29 && ftm.wMonth == 2)
				ftm.wYear-=4;
			else
				ftm.wYear--;

			SystemTimeToFileTime(&ftm, &t1);
			LocalFileTimeToFileTime(&t1, wdt);
		}
	}

	return TRUE;
}

BOOL ConvertEntry(NET_FileEntryInfo* inf, FTPFileInfo* p)
{
	SYSTEMTIME st;
	memcpy(p, inf, sizeof(*p));

//Line are ignored
	if(inf->FileType == NET_SKIP)
	{
		p->FileType = NET_SKIP;
		return TRUE;
	}

//Make date-time
	if(inf->cr_date == 0)  inf->cr_date  = inf->date;

	if(inf->acc_date == 0) inf->acc_date = inf->date;

	GetLocalTime(&st);
	st.wMilliseconds = 0;

	if(!CorrectTime(st, inf->date,     &p->FindData.ftLastWriteTime) ||
	        !CorrectTime(st, inf->cr_date,  &p->FindData.ftCreationTime) ||
	        !CorrectTime(st, inf->acc_date, &p->FindData.ftLastAccessTime))
		return FALSE;

//Size
	p->FindData.nFileSizeHigh = (DWORD)((inf->size >> 32) & MAX_DWORD);
	p->FindData.nFileSizeLow  = (DWORD)(inf->size & MAX_DWORD);
	return TRUE;
}

/*
 * Checks if line starts with one of specified lines.
 */
BOOL StartsWith(LPCSTR line, LPCSTR *lines)
{
	for(; *lines; lines++)
		if(strncmp(line,*lines,strlen(*lines)) == 0)
			return TRUE;

	return FALSE;
}

/*          1         2         3         4         5         6         7
  01234567890123456789012345678901234567890123456789012345678901234567890123456789
  total 96
  drwxrwxr-x   6 88       0           8192 Aug  9  2002 .
  dr-xr-xr-x   6 0        0           8192 Aug  9  2002 ..
  drwxrwxr-x   2 88       8           8192 Mar 30 11:00 CONFIG
  drwxrwxr-x  31 716      0           8192 Aug  9  2002 comp.sources.unix
  drwxrwxr-x2173 88       8          57344 Mar 23 10:32 control
  drwxrwxr-x  19 10144    9003        8192 Nov 18 21:37 news.announce.newgroups
*/
BOOL is_unix_start(char *s, int len, int *off)
{
	CHECK((len < 10), FALSE)

	if(off) *off = 0;

	if(NET_IS_DIGIT(*s))
	{
		char *e = SkipSpace(SkipDigit(s));

		if(off) *off = (int)(e-s);

		s = e;
	}

	for(int n = 1; n < 10; n++)
		if(strchr("-RWXSTUGOL",NET_TO_UPPER(s[n])) == NULL)
			return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------
static FTPType ListingTypes[] =
{
	/* 0*/ { TRUE,  "Unix   ", "Classic unix listing",                              idPRParceUnix,    NULL },
	/* 1*/ { TRUE,  "Windows", "DIR-like listing",                                  idPRParceDos,     NULL },
	/* 2*/ { TRUE,  "VMS    ", "VMS system",                                        idPRParceVMS,     idDirPRParceVMS },
	/* 3*/ { TRUE,  "EPLF   ", "Comma delimited files list",                        idPRParceEPLF,    NULL  },
	/* 4*/ { FALSE, "CMS    ", "CMS plain files list",                              idPRParceCMS,     NULL },
	/* 5*/ { FALSE, "Mac TCP", "MAC-OS TCP/Connect II",                             idPRParceTCPC,    NULL },
	/* 6*/ { TRUE,  "OS/2   ", "OS/2 native format",                                idPRParceOS2,     NULL },
	/* 7*/ { TRUE,  "Komut  ", "File listing generated by some comutation servers", idPRParceSkirdin, idDirParceSkirdin },
	/* 8*/ { TRUE,  "Netware", "NETWARE file server",                               idPRParceNETWARE, NULL },
	/* 9*/ { TRUE,  "VxWorks", "VX DOS style parser",                               idPRParceVX_DOS,  NULL },
	{ TRUE,  "PC/TCP ", "PC/TCP v 2.11 ftpsrv.exe",                          idPRParcePCTCP,   idDirParcePCTCP },
	{ TRUE,  "OS/400 ", "IBM OS/400",                                        idPRParceOS400,   NULL },
	{ FALSE, "MVS    ", "MVS system",                                        idPRParceMVS,     idDirParceMVS }
};

static WORD ParseSystemInfo(LPCSTR str)
{
	if(!str[0])
		return FTP_TYPE_INVALID;
	if(StrNCmp(str,  "MACOS Peter's Server", 20) == 0)                     return FTP_TYPE_UNIX; else if(StrNCmp(str,  "MACOS WebSTAR FTP", 17) == 0)                        return FTP_TYPE_UNIX; else if(strstr(str,   "Windows_CE") != NULL)                                return FTP_TYPE_NT;   else if(StrNCmp(str,  "UNIX Type: L8 MAC-OS MachTen", 28) == 0)            return FTP_TYPE_UNIX; else if(strstr(str,   "UNIX") != NULL)                                      return FTP_TYPE_UNIX; else if(strstr(str,   "Windows_NT") != NULL)                                return FTP_TYPE_NT;   else if(StrNCmp(str,  "VMS", 3) == 0)                                       return FTP_TYPE_VMS;  else if((StrNCmp(str, "VM/CMS", 6) == 0) || (StrNCmp(str, "VM ", 3) == 0)) return FTP_TYPE_CMS;  else if(strstr(str,   "MAC-OS TCP/Connect II") != NULL)                     return FTP_TYPE_TCPC; else if(strstr(str,   "OS/2") != NULL)                                      return FTP_TYPE_OS2;  else if(strstr(str,   "NETWARE") != NULL)                                   return FTP_TYPE_NETWARE; else if(strstr(str,   "VxWorks") != NULL)                                   return FTP_TYPE_VXDOS; else if(strstr(str,   "PC/TCP ") != NULL)                                   return FTP_TYPE_PCTCP; else if(strstr(str,   "OS/400")  != NULL)                                   return FTP_TYPE_OS400; else if(StrNCmp(str,  "MVS", 3) == 0)                                       return FTP_TYPE_MVS;  else

		return FTP_TYPE_INVALID;
}

// ------------------------------------------------------------------------
// INTERFACE
//   Functions user by master plugin.
// ------------------------------------------------------------------------
WORD WINAPI idPRSDetectString(FTPServerInfo* const Server,char *ListingString, int ListingLength)
{
	WORD        n;
	FTPFileInfo fi;

//Detect by SYST
	if(Server->ServerType == FTP_TYPE_DETECT)
	{
		n = ParseSystemInfo(Server->ServerInfo);

		if(n < ARRAYSIZE(ListingTypes))
		{
			Log(("DETECT: by SYST [%s] to %d",Server->ServerInfo,n));
			Server->ServerType = n;
		}
	}

//Correct the same formats
	//Unix-like WinNT
	if(Server->ServerType == FTP_TYPE_NT &&
	        is_unix_start(ListingString,ListingLength,NULL))
	{
		Log(("DETECT: WinNT -> UNIX"));
		Server->ServerType = FTP_TYPE_UNIX;
	}

	//Unix-like EPLF
	if(Server->ServerType == FTP_TYPE_UNIX &&
	        ListingString[0] == '+')
	{
		Log(("DETECT: Unix -> EPLF"));
		Server->ServerType = FTP_TYPE_EPLF;
	}

	if(Server->ServerType < ARRAYSIZE(ListingTypes))
		return Server->ServerType;

//Try all parcers
	char *tmp = new char[ ListingLength+1 ];

	for(n = 0; n < ARRAYSIZE(ListingTypes); n++)
	{
		StrCpy(tmp, ListingString, ListingLength+1);

		if(ListingTypes[n].Detectable &&
		        ListingTypes[n].Parser(Server,&fi,tmp,ListingLength))
		{
			Log(("DETECT: by contents [%s] to %d",ListingString,n));
			delete[] tmp;
			return n;
		}
	}

	delete[] tmp;
	return FTP_TYPE_INVALID;
}

WORD WINAPI idDetectDirStringType(const FTPServerInfo* Server,LPCSTR String)
{
	//FTP_TYPE_PCTCP
	if(memcmp(String, PCTCP_PWD_Title, PCTCP_PWD_TITLE_LEN)==0)
		return FTP_TYPE_PCTCP;

	return FTP_TYPE_INVALID;
}

WORD WINAPI idPRSTypesCount(void)
{
	return ARRAYSIZE(ListingTypes);
}

FTPType* WINAPI idPRSTypeGet(WORD Index)
{
	return Index < ARRAYSIZE(ListingTypes) ? (&ListingTypes[Index]) : NULL;
}

// ------------------------------------------------------------------------
// Exported interface
// ------------------------------------------------------------------------
FTPPluginInterface* WINAPI FTPPluginGetInterface(void)
{
	static DirListInterface Interface;
	Interface.Magic                     = FTP_DIRLIST_MAGIC;
	Interface.DetectStringType          = idPRSDetectString;
	Interface.DetectDirStringType       = idDetectDirStringType;
	Interface.GetNumberOfSupportedTypes = idPRSTypesCount;
	Interface.GetType                   = idPRSTypeGet;
	return &Interface;
}

//------------------------------------------------------------------------
BOOL WINAPI FTP_PluginStartup(DWORD Reason)
{
	return TRUE;
}
