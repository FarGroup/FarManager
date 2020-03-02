#include "newarc.h"

ArchiveManagerConfig::ArchiveManagerConfig(ArchiveModuleManager* pManager)
{
	m_pManager = pManager;
	m_pFilter = new ArchiveFilter(pManager, true);
}

ArchiveManagerConfig::~ArchiveManagerConfig()
{
	delete m_pFilter;
}

bool ArchiveManagerConfig::Load()
{
	string strModulePath = Info.ModuleName;
	CutToSlash(strModulePath);

	LoadConfigs(strModulePath+_T("configs.xml"));
	LoadTemplates(strModulePath+_T("templates.xml"));

	m_pFilter->Load(strModulePath+_T("filters.xml"));

	return true;
}

bool ArchiveManagerConfig::Save(SaveOptions Opt)
{
	string strModulePath = Info.ModuleName;
	CutToSlash(strModulePath);

	if ( Opt & SAVE_CONFIGS )
		SaveConfigs(strModulePath+_T("configs.xml"));

	if ( Opt & SAVE_TEMPLATES )
		SaveTemplates(strModulePath+_T("templates.xml"));

	if ( Opt & SAVE_FILTER )
		m_pFilter->Save(strModulePath+_T("filters.xml"));

	return true;
}

ArchiveFilter* ArchiveManagerConfig::GetFilter()
{
	return m_pFilter;
}

void ArchiveManagerConfig::AddTemplate(ArchiveTemplate* pTemplate)
{
	m_pTemplates.add(pTemplate);
}

void ArchiveManagerConfig::RemoveTemplate(ArchiveTemplate* pTemplate)
{
	m_pTemplates.remove(pTemplate);
}

void ArchiveManagerConfig::GetTemplates(Array<ArchiveTemplate*>& templates)
{
	for (unsigned int i = 0; i < m_pTemplates.count(); i++)
		templates.add(m_pTemplates.at(i));
}

void ArchiveManagerConfig::AddFormatConfig(ArchiveFormatConfig* pConfig)
{
	m_pConfigs.add(pConfig);
}

void ArchiveManagerConfig::RemoveFormatConfig(ArchiveFormatConfig* pConfig)
{
	m_pConfigs.remove(pConfig);
}

void ArchiveManagerConfig::GetFormatConfigs(Array<ArchiveFormatConfig*>& configs)
{
	for (unsigned int i = 0; i < m_pConfigs.count(); i++)
		configs.add(m_pConfigs.at(i));
}

ArchiveFormatConfig* ArchiveManagerConfig::GetFormatConfig(ArchiveFormat* pFormat)
{
	if ( !pFormat )
		return nullptr;

	for (unsigned int i = 0; i < m_pConfigs.count(); i++)
	{
		ArchiveFormatConfig* pConfig = m_pConfigs[i];

		if ( pConfig->GetFormat() == pFormat )
			return pConfig;
	}

	return nullptr;
}



bool ArchiveManagerConfig::GetCommand(
		ArchiveFormat* pFormat,
		int nCommand,
		string& strCommand
		)
{
	if ( !pFormat )
		return false;

	for (unsigned int i = 0; i < m_pConfigs.count(); i++)
	{
		ArchiveFormatConfig* pConfig = m_pConfigs[i];

		if ( pConfig->GetFormat() == pFormat )
		{
			bool bEnabled;

			if ( pConfig->GetCommand(
					nCommand, 
					strCommand, 
					bEnabled
					) && bEnabled && !strCommand.IsEmpty() )
				return true;
			
			return false;
		}
	}
	
	return false;
}


bool ArchiveManagerConfig::SaveConfigs(const TCHAR* lpFileName)
{
	DeleteFile(lpFileName);

	TiXmlDocument doc;

	TiXmlElement* root = new TiXmlElement("configs");

	for (unsigned int i = 0; i < m_pConfigs.count(); i++)
	{
		ArchiveFormatConfig* pAC = m_pConfigs[i];
		TiXmlElement* tpl = new TiXmlElement("config");

		pAC->ToXml(*tpl);

		root->LinkEndChild(tpl);
	}

	doc.LinkEndChild(root);
	doc.SaveFile((FakeUtf8String)lpFileName);

	return true;
}



bool ArchiveManagerConfig::LoadConfigs(const TCHAR* lpFileName)
{
	m_pConfigs.reset();

	TiXmlDocument doc;

	doc.LoadFile((FakeUtf8String)lpFileName);

	TiXmlHandle handle(&doc);

	TiXmlNode* pNode = handle.FirstChild("configs").ToNode();

	if ( pNode )
	{
		TiXmlNode* pChild = pNode->FirstChild("config");

		while ( pChild )
		{
			ArchiveFormatConfig* pAC = ArchiveFormatConfig::FromXml(m_pManager, *pChild);

			if ( pAC )
				m_pConfigs.add(pAC);

			pChild = pChild->NextSibling("config");
		}
	}

	return true;
}


bool ArchiveManagerConfig::LoadTemplates(const TCHAR* lpFileName)
{
	m_pTemplates.reset();

	TiXmlDocument doc;

	doc.LoadFile((FakeUtf8String)lpFileName);

	TiXmlHandle handle(&doc);

	TiXmlNode* pNode = handle.FirstChild("templates").ToNode();

	if ( pNode )
	{
		TiXmlNode* pChild = pNode->FirstChild("template");

		while ( pChild )
		{
			ArchiveTemplate* pAT = ArchiveTemplate::FromXml(m_pManager, *pChild);

			if ( pAT )
				m_pTemplates.add(pAT);

			pChild = pChild->NextSibling("template");
		}
	}

	return true;
}

bool ArchiveManagerConfig::SaveTemplates(const TCHAR* lpFileName)
{
	DeleteFile(lpFileName);

	TiXmlDocument doc;

	TiXmlElement* root = new TiXmlElement("templates");

	for (unsigned int i = 0; i < m_pTemplates.count(); i++)
	{
		ArchiveTemplate* pAT = m_pTemplates[i];
		TiXmlElement* tpl = new TiXmlElement("template");

		pAT->ToXml(*tpl);

		root->LinkEndChild(tpl);
	}

	doc.LinkEndChild(root);
	doc.SaveFile((FakeUtf8String)lpFileName);

	return true;
}

