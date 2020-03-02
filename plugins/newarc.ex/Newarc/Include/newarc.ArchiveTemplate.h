#pragma once
#include "newarc.h"

class ArchiveTemplate {

private:

	string m_strName;
	string m_strParams;

	string m_strConfig;

	GUID m_uidModule;
	GUID m_uidPlugin;
	GUID m_uidFormat;

	ArchiveFormat* m_pFormat;

public:

	ArchiveTemplate();

	void Clear();

	const TCHAR* GetName() const;
	void SetName(const TCHAR* lpName);

	const TCHAR* GetParams() const;
	void SetParams(const TCHAR* lpParams);

	const TCHAR* GetConfig() const;
	void SetConfig(const TCHAR* lpConfig);

	ArchiveFormat* GetFormat();
	void SetFormat(ArchiveFormat* pFormat);

	bool IsValid() const;

	static ArchiveTemplate* FromXml(ArchiveModuleManager* pManager, TiXmlNode& node); //pNode == <template name="">
	void ToXml(TiXmlNode& node);

};
