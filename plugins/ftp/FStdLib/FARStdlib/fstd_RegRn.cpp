#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

BOOL WINAPI FP_RenameRegKeyAll(HKEY hParentKey, LPCSTR szTarg, LPCSTR szSrc)
{
	if(!FP_CopyRegKeyAll(hParentKey, szSrc, hParentKey, szTarg))
		return FALSE;

	return FP_DeleteRegKeyAll(hParentKey, szSrc);
}
