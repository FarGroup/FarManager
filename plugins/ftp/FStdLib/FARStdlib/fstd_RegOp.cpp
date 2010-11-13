#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

HKEY WINAPI FP_OpenRegKey(const char *Key)
{
	HKEY hKey;
	char  name[ FAR_MAX_REG ];
	CHK_INITED

	if(Key && *Key)
		SNprintf(name,sizeof(name),"%s" SLASH_STR "%s",FP_PluginRootKey,Key);
	else
		SNprintf(name,sizeof(name),"%s",FP_PluginRootKey);

	if(RegOpenKey(HKEY_CURRENT_USER,name,&hKey) != ERROR_SUCCESS)
		return NULL;

	return hKey;
}
