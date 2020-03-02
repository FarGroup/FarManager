#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

static char     *CT_Def_argv[] = { (char*)"",NULL };
static char    **CT_argv = CT_Def_argv;
static int       CT_argc = 1;
static BOOL      CT_CaseSensitive;

void WINAPI CTArgInit(int argc, char **argv,BOOL CaseSensitive)
{
	CT_argc = argc;
	CT_argv = argv;
	CT_CaseSensitive = CaseSensitive;
#if defined(__QNX__)

	if(strchr(argv[0],'\\') == NULL)
		CT_argv[0] = strdup(MakePathName(GetCurDir(),argv[0]).Text());

#endif
}

char *WINAPI CTArgGet(int num)
{
	return (num < 0 || num >= CT_argc)?NULL:CT_argv[num];
}

char *WINAPI CTArgGetArg(int num)
{
	for(int n = 1; num >= 0 && n < CT_argc && CT_argv[n]; n++)
		if((CT_argv[n][0] == '-' || CT_argv[n][0] == '/'))
			continue;
		else
		{
			if(!num) return CT_argv[n];

			num--;
		}

	return NULL;
}

char *WINAPI CTArgGet(LPCSTR name)
{
	int         cn = StrColCount(name,";"),
	            n,i,len;
	LPCSTR m;

	for(n = 1; n < CT_argc && CT_argv[n]; n++)
		if((CT_argv[n][0] == '-' || CT_argv[n][0] == '/'))
			for(i = 1; i <= cn; i++)
			{
				m = StrGetCol(name,i,";");

				if(StrCmp(CT_argv[n]+1,m,len=(int)strlen(m),CT_CaseSensitive) == 0 &&
				        CT_argv[n][1+len] == '=') return CT_argv[n]+1+len+1;
			}

	return NULL;
}

BOOL WINAPI CTArgCheck(LPCSTR name)
{
	int      cn = StrColCount(name,";"),
	         n,i;
	LPCSTR m;

	for(n = 1; n < CT_argc && CT_argv[n]; n++)
		if((CT_argv[n][0] == '-' || CT_argv[n][0] == '/'))
			for(i = 1; i <= cn; i++)
			{
				m = StrGetCol(name,i,";");

				if(StrCmp(CT_argv[n]+1,m,-1,CT_CaseSensitive) == 0) return TRUE;
			}

	return FALSE;
}
