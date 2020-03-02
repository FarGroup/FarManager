#include "newarc.h"

extern const TCHAR* pCommandNames[MAX_COMMANDS];

ArchiveFormatConfig::ArchiveFormatConfig()
{
	Clear();
}

void ArchiveFormatConfig::Clear()
{
	m_strFormat = nullptr;
	m_pFormat = nullptr;

	memset(&m_uidFormat, 0, sizeof(GUID));
	memset(&m_uidPlugin, 0, sizeof(GUID));
	memset(&m_uidModule, 0, sizeof(GUID));

	for (unsigned int i = 0; i < MAX_COMMANDS; i++)
	{
		m_Commands[i].bEnabled = false;
		m_Commands[i].strCommand = nullptr;
	}
}

void ArchiveFormatConfig::ToXml(TiXmlNode& node)
{
	node.ToElement()->SetAttribute("format", (FakeUtf8String)m_strFormat);

	FakeUtf8String strFormatUID = GUID2STR(m_uidFormat);
	FakeUtf8String strPluginUID = GUID2STR(m_uidPlugin);
	FakeUtf8String strModuleUID = GUID2STR(m_uidModule);

	TiXmlElement* xmlFormatUID = new TiXmlElement("fid");
	TiXmlElement* xmlPluginUID = new TiXmlElement("pid");
	TiXmlElement* xmlModuleUID = new TiXmlElement("mid");

	xmlFormatUID->LinkEndChild(new TiXmlText(strFormatUID));
	xmlPluginUID->LinkEndChild(new TiXmlText(strPluginUID));
	xmlModuleUID->LinkEndChild(new TiXmlText(strModuleUID));

	node.LinkEndChild(xmlFormatUID);
	node.LinkEndChild(xmlPluginUID);
	node.LinkEndChild(xmlModuleUID);

	for (unsigned int i = 0; i < MAX_COMMANDS; i++)
	{
		TiXmlElement* pCommand = new TiXmlElement("command");

		pCommand->SetAttribute("id", i);
		pCommand->SetAttribute("name", (FakeUtf8String)pCommandNames[i]);
		pCommand->SetAttribute("enabled", m_Commands[i].bEnabled);
		
		pCommand->LinkEndChild(new TiXmlText((FakeUtf8String)m_Commands[i].strCommand));

		node.LinkEndChild(pCommand);
	}
}

ArchiveFormatConfig* ArchiveFormatConfig::FromXml(ArchiveModuleManager* pManager, TiXmlNode& node)
{
	ArchiveFormatConfig* pResult = new ArchiveFormatConfig;

	pResult->m_strFormat = (FakeUtf8String)node.ToElement()->Attribute("format");

	TiXmlHandle handle(&node);
	TiXmlElement* el = nullptr;

	if ( el = handle.FirstChild("fid").ToElement() )
	{
		string strFormatUID = (FakeUtf8String)el->GetText();
		pResult->m_uidFormat = STR2GUID(strFormatUID);
	}

	if ( el = handle.FirstChild("pid").ToElement() )
	{
		string strPluginUID = (FakeUtf8String)el->GetText();
		pResult->m_uidPlugin = STR2GUID(strPluginUID);
	}

	if ( el = handle.FirstChild("mid").ToElement() )
	{
		string strModuleUID = (FakeUtf8String)el->GetText();
		pResult->m_uidModule = STR2GUID(strModuleUID);
	}

	TiXmlElement* pCommand = handle.FirstChild("command").ToElement();

	while ( pCommand )
	{
		int id = -1;

		if ( pCommand->Attribute("id", &id) && (id >= 0) && (id < MAX_COMMANDS) )
		{
			pResult->m_Commands[id].strCommand = (FakeUtf8String)pCommand->GetText();
			pCommand->Attribute("enabled", &pResult->m_Commands[id].bEnabled); //BUGBUG!!!
		}

		pCommand = pCommand->NextSiblingElement("command");
	}

	pResult->m_pFormat = pManager->GetFormat(
			pResult->m_uidModule,
			pResult->m_uidPlugin,
			pResult->m_uidFormat
			);

	return pResult;
}

bool ArchiveFormatConfig::IsValid()
{
	return (m_pFormat != nullptr);
}

void ArchiveFormatConfig::SetFormat(ArchiveFormat* pFormat)
{
	m_pFormat = pFormat;

	m_uidModule = pFormat->GetModule()->GetUID();
	m_uidPlugin = pFormat->GetPlugin()->GetUID();
	m_uidFormat = pFormat->GetUID();

	m_strFormat = m_pFormat->GetName();
}

ArchiveFormat* ArchiveFormatConfig::GetFormat()
{
	return m_pFormat;
}

bool ArchiveFormatConfig::SetCommand(int nCommand, const TCHAR* lpCommand, bool bEnabled)
{
	if ( (nCommand >= 0) && (nCommand < MAX_COMMANDS) )
	{
		m_Commands[nCommand].bEnabled = bEnabled;
		m_Commands[nCommand].strCommand = lpCommand;
		return true;
	}

	return false;
}


bool ArchiveFormatConfig::GetCommand(int nCommand, string& strCommand, bool& bEnabled)
{
	if ( (nCommand >= 0) && (nCommand < MAX_COMMANDS) )
	{
		bEnabled = m_Commands[nCommand].bEnabled;
		strCommand = m_Commands[nCommand].strCommand;
		return true;
	}

	return false;
}
