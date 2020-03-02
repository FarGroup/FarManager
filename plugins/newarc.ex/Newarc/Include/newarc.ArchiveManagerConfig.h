#pragma once
#include "newarc.h"


enum SaveOptions {
	SAVE_TEMPLATES = 1,
	SAVE_CONFIGS = 2,
	SAVE_FILTER = 4,
	SAVE_ALL = SAVE_TEMPLATES|SAVE_CONFIGS|SAVE_FILTER
	};

class ArchiveManagerConfig {

private:

	ArchiveModuleManager* m_pManager;

	ObjectArray<ArchiveTemplate*> m_pTemplates;
	ObjectArray<ArchiveFormatConfig*> m_pConfigs;

	std::map<const ArchiveFormat*, ArchiveFormatConfig*> pActualConfigs;

	ArchiveFilter* m_pFilter;

public:

	ArchiveManagerConfig(ArchiveModuleManager* pManager);
	~ArchiveManagerConfig();

	bool Load();
	bool Save(SaveOptions Opt);

	ArchiveFilter* GetFilter();

	void AddTemplate(ArchiveTemplate* pTemplate);
	void RemoveTemplate(ArchiveTemplate* pTemplate);
	void GetTemplates(Array<ArchiveTemplate*>& templates);

	void AddFormatConfig(ArchiveFormatConfig* pConfig);
	void RemoveFormatConfig(ArchiveFormatConfig* pConfig);
	void GetFormatConfigs(Array<ArchiveFormatConfig*>& configs);

	ArchiveFormatConfig* GetFormatConfig(ArchiveFormat* pFormat);


	//deprecated
	bool GetCommand(ArchiveFormat* pFormat, int nCommand, string& strCommand);

private:

	bool LoadTemplates(const TCHAR* lpFileName);
	bool SaveTemplates(const TCHAR* lpFileName);

	bool LoadConfigs(const TCHAR* lpFileName);
	bool SaveConfigs(const TCHAR* lpFileName);
};