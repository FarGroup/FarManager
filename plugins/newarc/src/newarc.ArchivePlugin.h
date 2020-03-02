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

	pointer_array<Archive*> *QueryArchive (const char *lpFileName, const char *lpBuffer, dword dwBufferSize);
	Archive *CreateArchive (const GUID &uid, const char *lpFileName);
	void FinalizeArchive (Archive *pArchive);

	bool pGetDefaultCommand (const GUID &uid, int nCommand, char *lpCommand);

	const ArchiveFormatInfo* GetArchiveFormatInfo (const GUID &uid);

	void ReloadLanguage (const char *lpLanguage);

public:

	virtual const char * __stdcall pGetMsg (int nModuleNumber, int nID);
};
