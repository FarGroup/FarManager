#include <all_far.h>
#pragma hdrstop

#include "p_Int.h"

/* directories identified by trailing "/" characters
*/
BOOL DECLSPEC idPRParceTCPC( const PFTPServerInfo Server, PFTPFileInfo p, char *entry, int entry_len )
  {
     StrCpy( p->FindData.cFileName, entry, sizeof(p->FindData.cFileName) );
     if ( entry_len && entry[--entry_len] == '/') {
       entry[entry_len] = 0;
       p->FileType = NET_DIRECTORY;
     }

 return TRUE;
}
