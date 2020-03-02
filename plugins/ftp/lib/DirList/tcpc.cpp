#include <all_far.h>
#pragma hdrstop

#include "Int.h"

/* directories identified by trailing "/" characters
*/
BOOL WINAPI idPRParceTCPC(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len)
{
	StrCpy(p->FindData.cFileName, entry, ARRAYSIZE(p->FindData.cFileName));

	if(entry_len && entry[--entry_len] == '/')
	{
		entry[entry_len] = 0;
		p->FileType = NET_DIRECTORY;
	}

	return TRUE;
}
