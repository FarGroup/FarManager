#pragma once
#include "newarc.h"

class ArchiveFilter {

private:

	bool m_bUseRemaining;

	ObjectArray<ArchiveFilterEntry*> m_pFilters; //this is NOT ArchiveFilterArray
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