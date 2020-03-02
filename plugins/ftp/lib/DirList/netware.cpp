#include <all_far.h>
#pragma hdrstop

#include "Int.h"

/*          1         2         3         4         5         6         7
  01234567890123456789012345678901234567890123456789012345678901234567890123456789
  - [RWCEAFMS] IGOR                         106278016 May 07  2001 w2ksp2.ex
  - [RWCEAFMS] SERG                           102400 Oct 16  2000 UUUUUU.LZH
  - [RWCEAFMS] SERG                            24576 Jun 14  2001 WINSOC.RAR
  - [RWCEAFMS] SERG                             3335 Jan 27  2001 ZD001206.ARJ
  - [RWCEAFMS] COM                            276043 Mar 05  2002 basa_iva.ARJ
  - [RWCEAFMS] SERG                             7680 Apr 27  2001 12032001.xls
  - [RWCEAFMS] COM                           1142784 Jan 29 16:03 Дай мне быть с тобою рядом (Фристайл).mp3
  - [RWCEAFMS] COM                             12288 Dec 20  2001 nds10009.xls
  - [RWCEAFMS] COM                             30000 Jul 16  2002 TRACK22.WAV
  - [RWCEAFMS] ASDU                            22528 Dec 16  2002 '_'-Rбв вRЄ.xls
  - [RWCEAFMS] COM                             96256 Jul 09  2001 ZP010621.XLS
  - [RWCEAFMS] COM                            266240 Jul 27  2001 rgp18i.zip
  d [RWCEAFMS] SERG                              512 Mar 13 14:29 baza_arz
  d [RWCEAFMS] SERG                              512 Mar 10 01:09 chebocs
  d [RWCEAFMS] SERG                              512 Mar 10 01:09 volgskoe
  d [RWCEAFMS] PHD3                              512 Mar 10 01:09 a
  d [RWCEAFMS] TB1                               512 Mar 13 16:08 GEYKO
  d [RWCEAFMS]          0                        512 Apr 11 08:47 admin
  d [RWCEAFMS]          0                        512 Apr 06 04:12 aviso
  -[RWCEMFA]  1 lipinl1       253 Apr 11 00:20 jednicka.asm
*/
BOOL WINAPI idPRParceNETWARE(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len)
{
	NET_FileEntryInfo entry_info;
	BOOL              remove_size = FALSE;
	char             *m;

	if(entry_len < 43) return FALSE;

//Dir
	if(NET_TO_UPPER(*entry) == 'D')
	{
		entry_info.FileType = NET_DIRECTORY;
		remove_size          = TRUE; /* size is not useful */
	}
	else

//File
		if(NET_TO_UPPER(*entry) == '-')
		{
			//Plain file
		}
		else
//unk
			return FALSE;

//Attrs
	entry++;

	if(NET_IS_SPACE(*entry)) entry++;

	if(*entry != '[') return FALSE;

	m = SkipNX(entry,']');

	if(*m != ']') return FALSE;

	entry = m+1;
	entry_info.FindData.dwFileAttributes = 0;
//Owner
	entry = SkipSpace(entry);
	m = SkipNSpace(entry);
	StrCpy(entry_info.FTPOwner, entry, (int)(m-entry+1));
	entry = SkipSpace(m);

	if(!NET_IS_DIGIT(*entry))
	{
		m = SkipNSpace(entry);
		StrCpy(entry_info.FTPOwner, entry, (int)(m-entry+1));
		entry = SkipSpace(m);
	}

//Size
	m = SkipDigit(entry);

	if(m[0] != ' ') return FALSE;

	*m = 0;
	entry_info.size = AtoI(entry, (__int64)-1);
	*m = ' ';

	if(entry_info.size == -1) return FALSE;

	entry = SkipSpace(m);

//Date
	if(!net_convert_unix_date(entry, entry_info.date))
		return FALSE;

	entry = SkipSpace(entry);
//FileName
	StrCpy(entry_info.FindData.cFileName, entry, ARRAYSIZE(entry_info.FindData.cFileName));

	if(!entry_info.FindData.cFileName[0])
		return FALSE;

	if(remove_size) entry_info.size = 0;

	return ConvertEntry(&entry_info,p);
}
