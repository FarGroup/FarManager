#pragma once
#include "newarc.h"

struct ArchiveFormatCommand {
	string strCommand;
	XML_BOOL bEnabled;
};

class ArchiveFormatConfig {

private:

	string m_strFormat;

	GUID m_uidFormat;
	GUID m_uidPlugin;
	GUID m_uidModule;

	ArchiveFormat* m_pFormat;

	ArchiveFormatCommand m_Commands[MAX_COMMANDS];

public:

	ArchiveFormatConfig();

	void Clear();

	ArchiveFormat* GetFormat();
	void SetFormat(ArchiveFormat* pFormat);

	bool SetCommand(int nCommand, const TCHAR* lpCommand, bool bEnabled);
	bool GetCommand(int nCommand, string& strCommand, bool& bEnabled);

	static ArchiveFormatConfig* FromXml(ArchiveModuleManager* pManager, TiXmlNode& node);
	void ToXml(TiXmlNode& node);

	bool IsValid();
};
