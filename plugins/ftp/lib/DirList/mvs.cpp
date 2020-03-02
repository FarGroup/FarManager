#include <all_far.h>
#pragma hdrstop

#include "Int.h"

//2008/10/14 12:42
BOOL net_parse_mvs_date_time(LPCSTR datestr, Time_t& decoded, BOOL useTime = FALSE)
{
	if(datestr[4] != '/' || datestr[7] != '/')
		return FALSE;

	SYSTEMTIME st;
	GetSystemTime(&st);
	st.wMilliseconds = st.wSecond = st.wMinute = st.wHour = 0;

	if(useTime)
	{
		if(datestr[13] != ':')
			return FALSE;

		int year,month,day,hour,minute;
		sscanf(datestr,"%4d/%2d/%2d %2d:%2d",&year,&month,&day,&hour,&minute);
		st.wYear=year;
		st.wMonth=month;
		st.wDay=day;
		st.wHour=hour;
		st.wMinute=minute;
	}
	else
	{
		int year,month,day;
		sscanf(datestr,"%4d/%2d/%2d",&year,&month,&day);
		st.wYear=year;
		st.wMonth=month;
		st.wDay=day;
	}

	return SystemTimeToFileTime(&st, decoded);
}


/*
DIR DATASETMODE
            1         2         3         4         5         6         7
  01234567890123456789012345678901234567890123456789012345678901234567890123456789
  Volume Unit    Referred Ext Used Recfm Lrecl BlkSz Dsorg Dsname
  Z18WR1 3390   **NONE**    1    1  FB     133  1330  PS  HCD.MSGLOG
  Z18WR1 3390   2008/11/26  1    1  FB      80  1600  PS  HCD.TERM
  Z18WR1 3390   2008/11/26  1    3  FB      80  6160  PS  HCD.TRACE
  Z18WR1 3390   2008/11/26  1    1  VB     256  6233  PS  IN.EDDS.RUSJAPON.TXT
  Z18WR2 3390   2008/11/27  1    1  VB     256  6233  PS  IN.EDDS.RUSJAPON.TXT1
  Z18WR1 3390   2008/11/27  1    1  VB     256  6233  PS  IN.EDDS.RUSYAPON.TXT
  Z18WR1 3390   2009/06/11  1    1  VB     256  6233  PO  IN.JENNY
  Z18WR2 3390   2009/06/10  1   75  FB      80  6160  PO  ISPF.ISPPROF
  Z18WR1 3390   2009/05/19  1    1  FB      80  6160  PO  OUT.FF.FB80
  Z18WR1 3390   2008/11/27  1  112  VBA    133  1632  PO  OUT.FF.VBA133
  Z18WR1 3390   2008/11/27  1   64  FB      80 27920  PO  OUT.JK
  Z18WR2 3390   2008/11/27  1   62  FB     128 27904  PO  OUT.JK.FB128
  Z18WR2 3390   2008/11/27  1   62  FB      80 27920  PO  OUT.JK.FB80
  Z18WR1 3390   2008/11/26  1   30  VBA    133  1632  PS  OU2.FF.EXPLIST
  Z18WR1 3390   2008/11/26  1   30  VBA    133  1632  PS  OU3.FF.EXPLIST
  Z18WR1 3390   2008/11/26  1    1  FB      80   800  PS  SPFTEMP2.CNTL
  Z18WR2 3390   2008/11/26  1    1  FBA    121  3146  PS  SPFTEMP2.LIST
                                                     VSAM
                                                      PO
                                                      PS
                                                     PATH
                                                     GDG
DIR DIRECTORYMODE
            1         2         3         4         5         6         7
  01234567890123456789012345678901234567890123456789012345678901234567890123456789
  Volume Unit    Referred Ext Used Recfm Lrecl BlkSz Dsorg Dsname
  Pseudo Directory                                        IN
  Pseudo Directory                                        ISPF
  Pseudo Directory                                        OUT
  Z18WR1 3390   2009/06/11  1    1  VB     256  6160  PS  README
  Z18WR1 3390   2009/06/11  1    1  VB     256  6233  PS  ROUTE
  Pseudo Directory                                        TMP
  User catalog connector                                  CAI115.UCAT
  250 List completed successfully.

PO F
            1         2         3         4         5         6         7
  01234567890123456789012345678901234567890123456789012345678901234567890123456789
   Name     VV.MM   Created       Changed      Size  Init   Mod   Id
  ADDUSER   01.01 2008/10/14 2008/10/14 12:42    15    12    15 IBMUSER
  COPYDSDT  01.18 1998/09/17 2007/02/01 15:27    30    32     0 IRA
  COPYDSTD  01.17 1997/10/17 2005/11/01 10:43    24    18     0 ANNA
  DELCAT    01.01 2009/02/26 2009/02/27 13:29    12    12     1 ANNAP
  DELIODF   01.01 2008/11/11 2008/11/11 11:26    12    13    12 ANNAP
  DMPDISKD  01.04 2003/03/21 2003/03/26 11:01    13    12     4 ANNA
  DSSCOPA   01.28 2002/05/07 2008/10/01 12:18    19    14    11 IBMUSER
  DSSCOPLS  01.02 2007/04/12 2007/04/12 11:11    20    20     5 ANNA
  DSSCOPN   01.23 2008/10/03 2009/02/19 13:40     8    19     8 ANNAP
  DSSCOPN1  01.04 2009/01/15 2009/01/15 14:12     8     8     5 ANNAP
  DSSCOPPR  01.02 2002/07/04 2003/03/20 15:04    10    10     2 SMEN
  DSSCOPRA  01.10 2002/06/18 2007/04/12 11:09    20    17     8 ANNA
  DSSCOPY   01.08 1997/04/15 2003/03/27 16:23    12    11     8 ANNA
  DSSCPDMP  01.05 1997/05/21 2002/07/10 14:12    10    10     4 TG
  DSSICK    01.05 2003/03/31 2003/04/01 10:47    10    10     5 SMEN
  DSSREST   01.22 1997/05/05 2003/04/09 09:40    13    10     8 TG
   Name     VV.MM   Created       Changed      Size  Init   Mod   Id
  BLSGPROF
  BPXWPROF
  CBDPROF
  CSQOPROF

PO U
            1         2         3         4         5         6         7
  01234567890123456789012345678901234567890123456789012345678901234567890123456789
   Name      Size   TTR   Alias-of AC --------- Attributes --------- Amode Rmode
  DYNPRGA   005128 000209          01 FO                              24    24
  FTLGOT    01CEE8 002508          01 FO                              24    24
  FTLINPR   008AF8 000507          01 FO                              24    24
  FTSNG     00E878 00270E          01 FO                              24    24
  FTSTAN    00A5C0 000406          01 FO                              24    24
  IOCHECK   074DA0 000607          01 FO                              24    24
  KCFACDBB  0000F8 002C16          00 FO             RN RU            31    ANY
  KCFARSM   0003B0 002C1D          00 FO             RN RU            31    ANY
  KCFCRST2  09B798 002C25          00 FO             RN RU            31    ANY
  KCFSASDL  001828 004316          00 FO                              31    ANY
  L$CLEAI   001828 004316 KCFSASDL 00 FO                              31    ANY

*/
/*
JES responce
            1         2         3         4         5         6         7
  01234567890123456789012345678901234567890123456789012345678901234567890123456789
  FTPMOL0   JOB07136  OUTPUT    4 Spool Files

  No jobs found on Held queue
*/

/*
UNIX like
            1         2         3         4         5         6         7
  01234567890123456789012345678901234567890123456789012345678901234567890123456789
  drwxr-xr-x   2 IBMUSER  OMVSGRP    16384 May 26  2006 IBM
  drwxr-xr-x   2 IBMUSER  OMVSGRP     8192 May 17  2006 X11
  lrwxrwxrwx   1 IBMUSER  OMVSGRP       27 Aug 10  2007 acl_edit -> ../usr/lpp/dce/bin/acl_edit
  -rwxr-xr-x  17 IBMUSER  OMVSGRP    61440 May 17  2006 alias
  -rwxr-xr-x   2 IBMUSER  OMVSGRP   184320 Oct 17  2006 ar
  -rwxr-xr-x   2 IBMUSER  OMVSGRP   397312 May 17  2006 awk
  lrwxrwxrwx   1 IBMUSER  OMVSGRP       29 Aug 10  2007 bak -> ../usr/lpp/dfs/global/bin/bak
  -rwxr-xr-x   2 IBMUSER  OMVSGRP    57344 May 17  2006 basename
  -rwsr-xr-x   3 IBMUSER  OMVSGRP   172032 May 17  2006 batch
  -rwxr-xr-x   2 IBMUSER  OMVSGRP   237568 May 17  2006 bc
  -rwxr-xr-x  17 IBMUSER  OMVSGRP    61440 May 17  2006 bg
  lrwxrwxrwx   1 IBMUSER  OMVSGRP       29 Aug 10  2007 bos -> ../usr/lpp/dfs/global/bin/bos
 *(UNIX REAL)
  -rw-r--r--    1 montulli eng        14615 Nov  9 17:03 Makefile
  -rw-r--r--  1 panfilov users 139264 2005-06-20 14:22 AddTransToStacks.doc

*/

BOOL net_convert_unix_date(LPSTR& datestr, Time_t& decoded);

BOOL WINAPI idPRParceMVS(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len)
{
	NET_FileEntryInfo ei;
	BOOL needDot = FALSE;
	BOOL hidden = FALSE;
	memset(&ei,0,sizeof(ei));

	if(strncmp(entry,"Volume Unit    Referred Ext Used Recfm Lrecl BlkSz Dsorg Dsname",63)==0) return FALSE;

	if(strncmp(entry," Name     VV.MM   Created       Changed      Size  Init   Mod   Id",66)==0) return FALSE;

	if(strncmp(entry," Name      Size   TTR   Alias-of AC --------- Attributes --------- Amode Rmode",78)==0) return FALSE;

	if(strncmp(entry,"No jobs found on Held queue",27)==0) return FALSE;

	if(strlen(entry)>51&&(strncmp(entry+51," PO ",4)==0))
		ei.FileType = NET_DIRECTORY;
	else if(strncmp(entry,"Pseudo Directory",16)==0)
	{
		ei.FileType = NET_DIRECTORY;
//      ei.FileType = NET_SYM_LINK_TO_DIR;
		needDot = TRUE;
	}
	else if(strncmp(entry,"User catalog connector",22)==0)
	{
		ei.FileType = NET_DIRECTORY;
		hidden = TRUE;
		strcpy(ei.UnixMode,"VSAM");
//      ei.FileType = NET_SYM_LINK_TO_DIR;
		needDot = TRUE;
	}
	else if(strncmp(entry+7,"Error determining attributes",28)==0||
	        strncmp(entry+7,"Not Mounted",11)==0)
	{
		strcpy(ei.UnixMode,"-offline-");
		hidden = TRUE;
	}

#define BYTES_PER_TRACK_3380 47476
#define BYTES_PER_TRACK_3390 56664

	if(entry_len>8&&entry[6]!=' '&&entry[8]!=' '&&!needDot) // UNIX like
	{
		memcpy(ei.UnixMode,entry,10);
		ei.UnixMode[10]=0;

		if((entry[0]|0x20)=='d')
			ei.FileType = NET_DIRECTORY;

		if((entry[0]|0x20)=='l')
			ei.FileType = NET_SYM_LINK;

		if(entry[39])
		{
			int sz;
			sscanf(entry+33,"%d",&sz);
			ei.size=sz;
		}

		char* pdata = entry + 41;
		net_convert_unix_date(pdata, ei.date);

		if(entry[15]!=' ')
		{
			memcpy(ei.FTPOwner,entry+15,8+1+8);
			ei.FTPOwner[8+1+8]=0;
		}

		entry += 54;
		entry_len-=54;
		char* s = strstr(entry," -> ");

		if(s)
		{
			*s=0;

			if(s[4]) sscanf(s+4,"%s",ei.Link);
		}
	}
	else if(entry_len>56&&entry[56]!=' ')
	{
		int usedTrk, lrecl, blkSize, type;
		usedTrk=lrecl=blkSize=type=0;

		if(entry[7]!=' ') sscanf(entry+7,"%d",&type);

		if(entry[31]!=' ') sscanf(entry+27,"%d",&usedTrk);

		if(entry[43]!=' ') sscanf(entry+39,"%d",&lrecl);

		if(entry[49]!=' ') sscanf(entry+45,"%d",&blkSize);

		if(type==3380) ei.size = usedTrk * BYTES_PER_TRACK_3380;

		if(type==3390) ei.size = usedTrk * BYTES_PER_TRACK_3390;

		net_parse_mvs_date_time(entry + 14, ei.acc_date);

		if(!needDot) sscanf(entry,"%s",ei.FTPOwner);

		if(entry[52]!=' ')
		{
			memcpy(ei.UnixMode,entry+51,5);
			memcpy(ei.UnixMode+5,entry+33,5);
			ei.UnixMode[10]=0;

			if(strncmp(ei.UnixMode,"VSAM",4)==0||
			        strncmp(ei.UnixMode,"PATH",4)==0||
			        strncmp(ei.UnixMode,"GDG",3)==0)
				hidden=TRUE;
		}

		entry += 56;
		entry_len-=56;
	}
	else if(entry_len>8) // PO
	{
		if(entry_len>62)
		{
			if(entry[15]==' ') // PO F
			{
				net_parse_mvs_date_time(entry + 27, ei.date, TRUE);
				net_parse_mvs_date_time(entry + 16, ei.cr_date);
				int sz;
				sscanf(entry+44,"%d",&sz);
				ei.size=sz;
				sscanf(entry+62,"%s",ei.FTPOwner);
			}
			else if(entry[19]!=' ') // PO U
			{
				UINT sz;
				sscanf(entry+10,"%X",&sz);
				ei.size=sz;

				if(entry[24]!=' ')
					sscanf(entry+24,"%s",ei.Link);
			}
			else               // JES
			{
				UINT sz;
				sscanf(entry+30,"%X",&sz);
				ei.size=sz;
				entry+=10;
			}
		}

		entry[8]=0;
	}

//Name
	entry = SkipSpace(entry);

	if(*entry=='\'') entry++;

	StrCpy(ei.FindData.cFileName, entry, ARRAYSIZE(ei.FindData.cFileName));
	size_t le = strlen(ei.FindData.cFileName);

	if(le&&ei.FindData.cFileName[le-1]=='\'')ei.FindData.cFileName[le-1]=0;

	//e=strchr(ei.FindData.cFileName,'.');
	//if(e) { *(e+1)=0; ei.FileType = NET_DIRECTORY; }
	if(needDot) strcat(ei.FindData.cFileName, ".");

	XP_StripLine(ei.FindData.cFileName);

	if(hidden) ei.FindData.dwFileAttributes|=FILE_ATTRIBUTE_HIDDEN;

//     if(ei.FileType == NET_SYM_LINK_TO_DIR )
//      strcpy(ei.Link,ei.FindData.cFileName);
	return ConvertEntry(&ei,p);
}

BOOL WINAPI idDirParceMVS(const FTPServerInfo* Server, LPCSTR Line, char *CurDir, size_t CurDirSize)
{
	// 01234
	// 250 "'xxxx.yyy.'"
	Line+=5;

	if(*Line=='\'')
	{
		Line++;
		strcpy(CurDir,Line);
		char* Ptr=CurDir;

		while(!isspace(*Ptr))
			Ptr++;

		if(Ptr > CurDir+1)
			Ptr--;

		*Ptr=0;
		size_t le = strlen(CurDir);

		if(le&&CurDir[le-1]=='\'')CurDir[le-1]=0;

		if(CurDir[0]==0)strcpy(CurDir,"*");

		return TRUE;
	}

	return FALSE;
}
