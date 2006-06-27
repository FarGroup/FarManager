#include <all_far.h>
#pragma hdrstop

#include "p_Int.h"

/* can't be directory...
   "entry" already equals the correct FindData.cFileName
*/
BOOL DECLSPEC idPRParceCMS( const PFTPServerInfo Server, PFTPFileInfo p, char *entry, int entry_len )
  {
     StrCpy( p->FindData.cFileName, entry, sizeof(p->FindData.cFileName) );

 return TRUE;
}
