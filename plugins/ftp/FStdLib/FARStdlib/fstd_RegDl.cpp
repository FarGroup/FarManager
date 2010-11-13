#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

BOOL WINAPI FP_DeleteRegKey(const char *Key)
{
	char  name[ FAR_MAX_REG ];
	CHK_INITED

	if(!Key || !Key[0])
		return FALSE;

	sprintf(name,"%s" SLASH_STR "%s",FP_PluginRootKey,Key);
	return FP_DeleteRegKeyFull(name);
}

BOOL WINAPI FP_DeleteRegKeyFull(const char *Key)
{
	SetLastError(RegDeleteKey(HKEY_CURRENT_USER,Key));
	return GetLastError() == ERROR_SUCCESS;
}
