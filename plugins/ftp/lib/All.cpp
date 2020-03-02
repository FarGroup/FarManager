#include <all_far.h>
#pragma hdrstop

#include <fstdlib.h>
#include "Plugin.h"

#if !defined(__USE_STD_FUNCIONS__)
#define __FP_INFO_FUNCTIONS__ 1
#endif

FTPInterface* FTP_Info = NULL;
HMODULE       FTP_Module = NULL;

/*******************************************************************
   INTERFACE
 *******************************************************************/
extern "C" FTPPluginInterface* WINAPI FTPQueryInterface(FTPInterface* Info)
{
	if(Info->Magic != FTP_INTERFACE_MAGIC ||
	        Info->SizeOf != sizeof(FTPInterface))
		return NULL;

	FTP_Info = Info;
	return FTPPluginGetInterface();
}

/*******************************************************************
   INTERFACE
 *******************************************************************/
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID ptr)
{
	BOOL rc = FTP_PluginStartup(reason);

	if(reason == DLL_PROCESS_DETACH)
		FTP_Info = NULL;

	return rc;
}

/*******************************************************************
   CPP MEMORY
 *******************************************************************/
#if defined( __DEBUG__ )
void WINAPI _RTLCheck(LPCSTR fnm)
{
	if(!FTP_Info)
	{
		MessageBox(NULL,
		           "FTP_Info is not defined in FTP subplugin",
		           fnm ? fnm : "Error...", MB_OK);
		//abort();
	}
}
#endif

/*******************************************************************
   String functions
 *******************************************************************/
#if defined( __FP_INFO_FUNCTIONS__ )
int   WINAPI StrCmp(LPCSTR str,LPCSTR str1,int maxlen, BOOL isCaseSens) { RTLCheck("StrCmp")  return FTP_Info->StrCmp(str,str1,maxlen,isCaseSens); }
char* WINAPI StrCpy(char *dest,LPCSTR src,int dest_sz) { RTLCheck("StrCpy")  return FTP_Info->StrCpy(dest,src,dest_sz); }
#else
int   WINAPI strlen(LPCSTR s)              { return s ? strlen(s) : 0; }

int WINAPI StrCmp(LPCSTR str,LPCSTR str1,int maxlen /*= -1*/, BOOL isCaseSens /*= FALSE*/)
{
	if(!str)  return (str1 == NULL) ? 0 : (-1);

	if(!str1) return 1;

	int rc;
	rc = CompareString(LOCALE_USER_DEFAULT,
	                   isCaseSens ? 0 : NORM_IGNORECASE,
	                   str,maxlen,str1, maxlen);
	return rc-2;
}

char *WINAPI StrCpy(char *dest,LPCSTR src,int dest_sz)
{
	if(!dest)        return NULL;

	if(dest_sz == 0) return dest;

	if(!src)         { *dest = 0; return dest; }

	if(dest_sz != -1)
	{
		strncpy(dest,src,dest_sz-1);
		dest[dest_sz-1] = 0;
	}
	else
		strcpy(dest,src);

	return dest;
}
#endif