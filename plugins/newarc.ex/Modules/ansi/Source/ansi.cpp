#include "ansi.h"

oldfar::PluginStartupInfo oldInfo;
oldfar::FARSTANDARDFUNCTIONS oldFSF;

#pragma data_seg("shared")
oldfar::PluginStartupInfo* pInfo = NULL;
oldfar::FARSTANDARDFUNCTIONS* pFSF = NULL;
#pragma comment(linker, "/section:shared,rws")

void __stdcall SetStartupInfo(oldfar::PluginStartupInfo* pi)
{
	oldInfo = *pi;
	oldFSF = *oldInfo.FSF;

	pInfo = &oldInfo;
	pFSF = &oldFSF;
}

void __stdcall ExitFAR()
{
	pInfo = NULL;
	pFSF = NULL;
}

void __stdcall GetPluginInfo(oldfar::PluginInfo* pi)
{
	pi->StructSize = sizeof(oldfar::PluginInfo);
	pi->Flags = PF_PRELOAD;
}


bool __stdcall GetPluginStartupInfo(oldfar::PluginStartupInfo** ppInfo, oldfar::FARSTANDARDFUNCTIONS** ppFSF)
{
	if ( ppInfo )
		*ppInfo = pInfo;

	if ( ppFSF )
		*ppFSF = pFSF;

	return (pInfo && pFSF);
}
