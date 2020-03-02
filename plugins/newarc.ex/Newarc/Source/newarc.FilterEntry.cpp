#include "newarc.h"

ArchiveFilterEntry::ArchiveFilterEntry()
{
	Clear();
}

void ArchiveFilterEntry::Clear()
{
	m_strMask = nullptr;
	m_strName = nullptr;

	m_bExclude = false;
	m_bEnabled = false;
	m_bContinue = false;

	m_bAllFormats = false;
	m_bAllModules = false;
	m_bAllPlugins = false;

	m_pModule = nullptr;
	m_pPlugin = nullptr;
	m_pFormat = nullptr;

	memset(&m_uidFormat, 0, sizeof(GUID));
	memset(&m_uidPlugin, 0, sizeof(GUID));
	memset(&m_uidModule, 0, sizeof(GUID));
}

const TCHAR* ArchiveFilterEntry::GetName() const
{
	return m_strName.GetString();
}

void ArchiveFilterEntry::SetName(const TCHAR* lpName)
{
	m_strName = lpName;
}

const TCHAR* ArchiveFilterEntry::GetMask() const
{
	return m_strMask.GetString();
}

void ArchiveFilterEntry::SetMask(const TCHAR* lpMask)
{
	m_strMask = lpMask;
}

bool ArchiveFilterEntry::IsExclude() const
{
	return m_bExclude;
}

void ArchiveFilterEntry::SetExclude(bool bExclude)
{
	m_bExclude = bExclude;
}

bool ArchiveFilterEntry::IsEnabled() const
{
	return m_bEnabled;
}

void ArchiveFilterEntry::SetEnabled(bool bEnabled)
{
	m_bEnabled = bEnabled;
}

bool ArchiveFilterEntry::IsContinueProcessing() const
{
	return m_bContinue;
}

void ArchiveFilterEntry::SetContinueProcessing(bool bContinue)
{
	m_bContinue = bContinue;
}

bool ArchiveFilterEntry::IsAllFormats() const
{
	return m_bAllFormats;
}

void ArchiveFilterEntry::SetAllFormats(bool bAllFormats)
{
	m_bAllFormats = bAllFormats;

	if ( m_bAllFormats )
		m_pFormat = nullptr;
}

bool ArchiveFilterEntry::IsAllPlugins() const
{
	return m_bAllPlugins;
}

void ArchiveFilterEntry::SetAllPlugins(bool bAllPlugins)
{
	m_bAllPlugins = bAllPlugins;

	if ( m_bAllPlugins )
		m_pPlugin = nullptr;
}

bool ArchiveFilterEntry::IsAllModules() const
{
	return m_bAllModules;
}

void ArchiveFilterEntry::SetAllModules(bool bAllModules)
{
	m_bAllModules = bAllModules;

	if ( m_bAllModules )
		m_pModule = nullptr;
}

ArchiveModule* ArchiveFilterEntry::GetModule() const
{
	return m_pModule;
}

void ArchiveFilterEntry::SetModule(ArchiveModule* pModule)
{
	m_pModule = pModule;

	if ( m_pModule )
	{
		m_uidModule = pModule->GetUID();
		m_bAllModules = false;
	}
	else
		m_bAllModules = true;
}

ArchivePlugin* ArchiveFilterEntry::GetPlugin() const
{
	return m_pPlugin;
}

void ArchiveFilterEntry::SetPlugin(ArchivePlugin* pPlugin)
{
	m_pPlugin = pPlugin;

	if ( m_pPlugin )
	{
		m_uidPlugin = pPlugin->GetUID();
		m_bAllPlugins = false;
	}
	else
		m_bAllPlugins = true;
}

ArchiveFormat* ArchiveFilterEntry::GetFormat() const
{
	return m_pFormat;
}

void ArchiveFilterEntry::SetFormat(ArchiveFormat* pFormat)
{
	m_pFormat = pFormat;

	if ( m_pFormat )
	{
		m_uidFormat = pFormat->GetUID();
		m_bAllFormats = false;
	}
	else
		m_bAllFormats = true;
}

void ArchiveFilterEntry::Clone(ArchiveFilterEntry* dest)
{
	dest->m_bExclude= m_bExclude;
	dest->m_bEnabled = m_bEnabled;
	dest->m_bContinue = m_bContinue;
	dest->m_strMask = m_strMask;
	dest->m_strName = m_strName;

	dest->m_bAllFormats = m_bAllFormats;
	dest->m_bAllPlugins = m_bAllPlugins;
	dest->m_bAllModules = m_bAllModules;

	dest->m_pFormat = m_pFormat;
	dest->m_pPlugin = m_pPlugin;
	dest->m_pModule = m_pModule;
	
	dest->m_uidFormat = m_uidFormat;
	dest->m_uidPlugin = m_uidPlugin;
	dest->m_uidModule = m_uidModule;
}

void ArchiveFilterEntry::ToXml(TiXmlNode& node)
{
	FakeUtf8String strFormatUID = GUID2STR(m_uidFormat);
	FakeUtf8String strPluginUID = GUID2STR(m_uidPlugin);
	FakeUtf8String strModuleUID = GUID2STR(m_uidModule);

	node.ToElement()->SetAttribute("name", (FakeUtf8String)m_strName);
	node.ToElement()->SetAttribute("mask", (FakeUtf8String)m_strMask);
	node.ToElement()->SetAttribute("enabled", m_bEnabled);
	node.ToElement()->SetAttribute("exclude", m_bExclude);
	node.ToElement()->SetAttribute("continue", m_bContinue);
	node.ToElement()->SetAttribute("allf", m_bAllFormats);
	node.ToElement()->SetAttribute("allp", m_bAllPlugins);
	node.ToElement()->SetAttribute("allm", m_bAllModules);

	if ( !m_bAllModules )
	{
		TiXmlElement* xmlModuleUID = new TiXmlElement("mid");
	
		xmlModuleUID->LinkEndChild(new TiXmlText(strModuleUID));
		node.LinkEndChild(xmlModuleUID);
	}

	if ( !m_bAllPlugins )
	{
		TiXmlElement* xmlPluginUID = new TiXmlElement("pid");

		xmlPluginUID->LinkEndChild(new TiXmlText(strPluginUID));
		node.LinkEndChild(xmlPluginUID);
	}

	if ( !m_bAllFormats )
	{
		TiXmlElement* xmlFormatUID = new TiXmlElement("fid");

		xmlFormatUID->LinkEndChild(new TiXmlText(strFormatUID));
		node.LinkEndChild(xmlFormatUID);
	}
}

ArchiveFilterEntry* ArchiveFilterEntry::FromXml(ArchiveModuleManager* pManager, TiXmlNode& node)
{
	string strName = (FakeUtf8String)node.ToElement()->Attribute("name");

	if ( strName.IsEmpty() )
		return nullptr;

	string strMask = (FakeUtf8String)node.ToElement()->Attribute("mask");

	if ( strMask.IsEmpty() )
		return nullptr;

	ArchiveFilterEntry* pResult = new ArchiveFilterEntry();

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

	pResult->m_strName = strName;
	pResult->m_strMask = strMask;

	node.ToElement()->Attribute("exclude", &pResult->m_bExclude);
	node.ToElement()->Attribute("enabled", &pResult->m_bEnabled);
	node.ToElement()->Attribute("continue", &pResult->m_bContinue);

	node.ToElement()->Attribute("allf", &pResult->m_bAllFormats);
	node.ToElement()->Attribute("allp", &pResult->m_bAllPlugins);
	node.ToElement()->Attribute("allm", &pResult->m_bAllModules);

	pResult->SetModule(pManager->GetModule(pResult->m_uidModule));
	pResult->SetPlugin(pManager->GetPlugin(pResult->m_uidModule, pResult->m_uidPlugin));
	pResult->SetFormat(pManager->GetFormat(pResult->m_uidModule, pResult->m_uidPlugin, pResult->m_uidFormat));

	return pResult;
}

bool ArchiveFilterEntry::IsValid()
{
	return !m_strName.IsEmpty() && !m_strMask.IsEmpty();
}
