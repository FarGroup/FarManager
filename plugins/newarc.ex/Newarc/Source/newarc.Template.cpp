#include "newarc.h"

extern const GUID& STR2GUID(const TCHAR *lpStr);
extern const TCHAR *GUID2STR(const GUID &uid);

const TCHAR* ArchiveTemplate::GetName() const
{
	return m_strName;
}

void ArchiveTemplate::SetName(const TCHAR* lpName)
{
	m_strName = lpName;
}

const TCHAR* ArchiveTemplate::GetParams() const
{
	return m_strParams;
}

void ArchiveTemplate::SetParams(const TCHAR* lpParams)
{
	m_strParams = lpParams;
}


const TCHAR* ArchiveTemplate::GetConfig() const
{
	return m_strConfig;
}

void ArchiveTemplate::SetConfig(const TCHAR* lpConfig)
{
	m_strConfig = lpConfig;
}

/*
const GUID& ArchiveTemplate::GetModuleUID() const
{
	return m_uidModule;
}

const GUID& ArchiveTemplate::GetPluginUID() const
{
	return m_uidPlugin;
}

const GUID& ArchiveTemplate::GetFormatUID() const
{
	return m_uidFormat;
}
*/

ArchiveFormat* ArchiveTemplate::GetFormat()
{
	return m_pFormat;
}

void ArchiveTemplate::SetFormat(ArchiveFormat* pFormat)
{
	m_pFormat = pFormat;

	if ( m_pFormat )
	{
		m_uidFormat = pFormat->GetUID();
		m_uidPlugin = pFormat->GetPlugin()->GetUID();
		m_uidModule = pFormat->GetModule()->GetUID();
	}
}

bool ArchiveTemplate::IsValid() const
{
	return !m_strName.IsEmpty() && (m_pFormat != nullptr);
}

ArchiveTemplate::ArchiveTemplate()
{
	Clear();
}

void ArchiveTemplate::Clear()
{
	m_pFormat = nullptr;

	m_strName = nullptr;
	m_strParams = nullptr;
	m_strConfig = nullptr;

	memset(&m_uidFormat, 0, sizeof(GUID));
	memset(&m_uidPlugin, 0, sizeof(GUID));
	memset(&m_uidModule, 0, sizeof(GUID));
}


ArchiveTemplate* ArchiveTemplate::FromXml(ArchiveModuleManager* pManager, TiXmlNode& node) //pNode == <template name="">
{
	string strName = (FakeUtf8String)node.ToElement()->Attribute("name");

	if ( strName.IsEmpty() )
		return nullptr;

	ArchiveTemplate* pResult = new ArchiveTemplate();

	pResult->m_strName = strName;

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

	if ( el = handle.FirstChild("config").ToElement() )
		pResult->m_strConfig = (FakeUtf8String)el->GetText();

	if ( el = handle.FirstChild("params").ToElement() )
		pResult->m_strParams = (FakeUtf8String)el->GetText();

	pResult->m_pFormat = pManager->GetFormat(pResult->m_uidModule, pResult->m_uidPlugin, pResult->m_uidFormat);

	return pResult;
}

void ArchiveTemplate::ToXml(TiXmlNode& node)
{
	FakeUtf8String strName = m_strName;
	FakeUtf8String strParams = m_strParams;
	FakeUtf8String strConfig = m_strConfig;

	FakeUtf8String strFormatUID = GUID2STR(m_uidFormat);
	FakeUtf8String strPluginUID = GUID2STR(m_uidPlugin);
	FakeUtf8String strModuleUID = GUID2STR(m_uidModule);

	node.ToElement()->SetAttribute("name", strName);

	//нечего мусорить в xml

	if ( !strParams.IsEmpty() )
	{
		TiXmlElement* xmlParams = new TiXmlElement("params");

		xmlParams->LinkEndChild(new TiXmlText(strParams));
		node.LinkEndChild(xmlParams);
	}

	if ( !strConfig.IsEmpty() )
	{
		TiXmlElement* xmlConfig = new TiXmlElement("config");

		xmlConfig->LinkEndChild(new TiXmlText(strConfig));
		node.LinkEndChild(xmlConfig);
	}

	TiXmlElement* xmlFormatUID = new TiXmlElement("fid");

	xmlFormatUID->LinkEndChild(new TiXmlText(strFormatUID));
	node.LinkEndChild(xmlFormatUID);

	TiXmlElement* xmlPluginUID = new TiXmlElement("pid");

	xmlPluginUID->LinkEndChild(new TiXmlText(strPluginUID));
	node.LinkEndChild(xmlPluginUID);

	TiXmlElement* xmlModuleUID = new TiXmlElement("mid");
	
	xmlModuleUID->LinkEndChild(new TiXmlText(strModuleUID));
	node.LinkEndChild(xmlModuleUID);
}
