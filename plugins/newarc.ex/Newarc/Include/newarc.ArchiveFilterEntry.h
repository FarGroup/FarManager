#pragma once
#include "newarc.h"

class ArchiveFilterEntry {

private:

	XML_BOOL m_bExclude;
	XML_BOOL m_bEnabled;
	XML_BOOL m_bContinue;

	string m_strMask;
	string m_strName;

	XML_BOOL m_bAllFormats;
	XML_BOOL m_bAllPlugins;
	XML_BOOL m_bAllModules;

	GUID m_uidFormat;
	GUID m_uidPlugin;
	GUID m_uidModule;

	ArchiveModule* m_pModule;
	ArchivePlugin* m_pPlugin;
	ArchiveFormat* m_pFormat;

public:

	ArchiveFilterEntry();

	void Clear();

	const TCHAR* GetName() const;
	void SetName(const TCHAR* lpName);

	const TCHAR* GetMask() const;
	void SetMask(const TCHAR* lpMask);

	bool IsExclude() const;
	void SetExclude(bool bExclude);

	bool IsEnabled() const;
	void SetEnabled(bool bEnabled);

	bool IsContinueProcessing() const;
	void SetContinueProcessing(bool bContinue);

	bool IsAllFormats() const;
	void SetAllFormats(bool bAllFormats);

	bool IsAllPlugins() const;
	void SetAllPlugins(bool bAllPlugins);

	bool IsAllModules() const;
	void SetAllModules(bool bAllFormats);

	ArchiveModule* GetModule() const;
	void SetModule(ArchiveModule* pModule);

	ArchivePlugin* GetPlugin() const;
	void SetPlugin(ArchivePlugin* pPlugin);

	ArchiveFormat* GetFormat() const;
	void SetFormat(ArchiveFormat* pFormat);

	void Clone(ArchiveFilterEntry* dest);

	bool IsValid();

	static ArchiveFilterEntry* FromXml(ArchiveModuleManager* pManager, TiXmlNode& node);
	void ToXml(TiXmlNode& node);
};

