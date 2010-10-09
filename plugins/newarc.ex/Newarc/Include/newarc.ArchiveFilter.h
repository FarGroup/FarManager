#pragma once
#include "newarc.h"


struct ArchiveFilterEntry {

	bool bExcludeFilter;
	
	bool bInvalid;
	bool bEnabled;
	bool bContinue;

	string strMask;
	string strName;

	bool bAllFormats;
	bool bAllPlugins;
	bool bAllModules;

	GUID uidFormat;
	GUID uidPlugin;
	GUID uidModule;

	ArchiveModule* pModule;
	ArchivePlugin* pPlugin;
	ArchiveFormat* pFormat;

	ArchiveFilterEntry()
	{
		bInvalid = false;

		bExcludeFilter = false;
		bEnabled = false;
		bContinue = false;

		bAllFormats = false;
		bAllModules = false;
		bAllPlugins = false;

		pModule = NULL;
		pPlugin = NULL;
		pFormat = NULL;

		memset(&uidFormat, 0, sizeof(GUID));
		memset(&uidPlugin, 0, sizeof(GUID));
		memset(&uidModule, 0, sizeof(GUID));
	}

	void Clone(ArchiveFilterEntry* dest)
	{
		dest->bInvalid = bInvalid;
		dest->bExcludeFilter = bExcludeFilter;

		dest->bEnabled = bEnabled;
		dest->bContinue = bContinue;
		dest->strMask = strMask;
		dest->strName = strName;

		dest->bAllFormats = bAllFormats;
		dest->bAllPlugins = bAllPlugins;
		dest->bAllModules = bAllModules;

		dest->pFormat = pFormat;
		dest->pPlugin = pPlugin;
		dest->pModule = pModule;
		
		dest->uidFormat = uidFormat;
		dest->uidPlugin = uidPlugin;
		dest->uidModule = uidModule;
	}
};


class ArchiveFilter {

private:

	bool m_bUseRemaining;

	ObjectArray<ArchiveFilterEntry*> m_pFilters; //this is NOT archive filter array
	ArchiveFilterArray m_pStopFilters;

	ArchiveModuleManager* m_pManager;

public:

	ArchiveFilter(ArchiveModuleManager* pManager, bool bUseRemaining);
	~ArchiveFilter();

	bool Load(const TCHAR* lpFileName);
	bool Save(const TCHAR* lpFileName);

	bool UseRemaining();
	void SetRemaining(bool bUseRemaining);

	void Clear();
	void Reset();

	void AddFilter(ArchiveFilterEntry* pFE);

	//добавлять сюда только фильтры из m_pFilters, не вновь созданные
	void AddStopFilter(ArchiveFilterEntry* pFE);

	int GetFilters(Array<ArchiveFilterEntry*>& filters);
	int QueryFilters(const TCHAR* lpFileName, ArchiveFilterArray& filters, bool& bStopped);
	
	bool Filtered(const GUID* puidModule, const GUID* puidPlugin, const GUID* puidFormat);
};