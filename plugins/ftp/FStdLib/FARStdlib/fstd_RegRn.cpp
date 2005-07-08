#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

BOOL DECLSPEC FP_RenameRegKeyAll(HKEY hParentKey, CONSTSTR szTarg, CONSTSTR szSrc )
  {
    if ( !FP_CopyRegKeyAll(hParentKey, szSrc, hParentKey, szTarg) )
      return FALSE;

 return FP_DeleteRegKeyAll(hParentKey, szSrc);
}
