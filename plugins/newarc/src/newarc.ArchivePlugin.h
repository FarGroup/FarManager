#pragma once
#include "newarc.h"

typedef int (__stdcall *PLUGINENTRY) (int nFunctionID, void *pParams);

class Archive;

class ArchivePlugin {

private:

	PBYTE m_pfnGetMsgThunk;

public:

	char *m_lpModuleName;

	PLUGINENTRY m_pfnPluginEntry;

	ArchivePluginInfo m_ArchivePluginInfo;

	HMODULE m_hModule;

	char *m_pCommands[11];

	char **m_pLanguageStrings;
	int m_nStringsCount;

public:

	bool Initialize (const char *lpModuleName, const char *lpLanguage);
	void Finalize ();

	Archive *QueryArchive (const char *lpFileName, const char *lpBuffer, dword dwBufferSize);
	Archive *OpenNewArchive (int nFormat, const char *lpFileName);
	void FinalizeArchive (Archive *pArchive);

	bool pGetDefaultCommand (int nType, int nCommand, char *lpCommand);

	void ReloadLanguage (const char *lpLanguage);

	const char * __stdcall pGetMsg (int nModuleNumber, int nID);
};
