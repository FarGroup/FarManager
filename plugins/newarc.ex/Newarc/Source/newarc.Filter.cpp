#include "newarc.h"

ArchiveFilter::ArchiveFilter(ArchiveModuleManager* pManager, bool bUseRemaining)
{
	m_bUseRemaining = bUseRemaining;
	m_pManager = pManager;
}


bool ArchiveFilter::UseRemaining()
{
	return m_bUseRemaining;
}

void ArchiveFilter::SetRemaining(bool bUseRemaining)
{
	m_bUseRemaining = bUseRemaining;
}

ArchiveFilter::~ArchiveFilter()
{
}

void ArchiveFilter::Clear()
{
	m_pFilters.reset();
	m_pStopFilters.reset();
}

void ArchiveFilter::Reset()
{
	m_pStopFilters.reset();
}

void ArchiveFilter::AddFilter(ArchiveFilterEntry* pFE)
{
	m_pFilters.add(pFE);
}


int ArchiveFilter::GetFilters(Array<ArchiveFilterEntry*>& filters)
{
	for (unsigned int i = 0; i < m_pFilters.count(); i++)
		filters.add(m_pFilters[i]);

	return 0;
}

int ArchiveFilter::QueryFilters(const TCHAR* lpFileName, ArchiveFilterArray& filters, bool& bStopped)
{
	bStopped = false;

	for (unsigned int i = 0; i < m_pFilters.count(); i++)
	{
		ArchiveFilterEntry* pFE = m_pFilters[i];

		if ( !pFE->IsEnabled() || !pFE->IsValid() )
			continue;

#ifdef UNICODE
		if ( FSF.ProcessName(pFE->GetMask(), (TCHAR*)lpFileName, 0, PN_CMPNAME|PN_SKIPPATH) )
#else
		if ( FSF.ProcessName(pFE->GetMask(), (TCHAR*)lpFileName, PN_CMPNAME|PN_SKIPPATH) )
#endif
		{
			if ( !pFE->IsExclude() )
				filters.add(pFE);
			else
				m_pStopFilters.add(pFE);

			if ( !pFE->IsContinueProcessing() )
			{
				bStopped = true;
				break;
			}
		}
	}

	return 0;
}


bool ArchiveFilter::Filtered(const GUID* puidModule, const GUID* puidPlugin, const GUID* puidFormat)
{
	for (unsigned int i = 0; i < m_pStopFilters.count(); i++)
	{
		ArchiveFilterEntry* pFE = m_pStopFilters[i];

		if ( !pFE->IsEnabled() || !pFE->IsValid() )
			continue;

		bool bFormatFiltered = (pFE->IsAllFormats() || (puidFormat && (pFE->GetFormat()->GetUID() == *puidFormat)));
		bool bPluginFiltered = bFormatFiltered && (pFE->IsAllPlugins() || (puidPlugin && (pFE->GetPlugin()->GetUID() == *puidPlugin)));
		bool bModuleFiltered = bPluginFiltered && (pFE->IsAllModules() || (puidModule && (pFE->GetModule()->GetUID() == *puidModule)));

		if ( bModuleFiltered )
			return true;
	}

	return false;
}


bool ArchiveFilter::Load(const TCHAR* lpFileName)
{
	m_pFilters.reset();

	TiXmlDocument doc;

	doc.LoadFile((FakeUtf8String)lpFileName);

	TiXmlHandle handle(&doc);

	TiXmlNode* pNode = handle.FirstChild("filters").ToNode();

	if ( pNode )
	{
		TiXmlNode* pChild = pNode->FirstChild("filter");

		while ( pChild )
		{
			ArchiveFilterEntry* pAE = ArchiveFilterEntry::FromXml(m_pManager, *pChild);
			m_pFilters.add(pAE);

			pChild = pChild->NextSibling("filter");
		}
	}

	return true;
}

bool ArchiveFilter::Save(const TCHAR* lpFileName)
{
	DeleteFile(lpFileName);

	TiXmlDocument doc;

	TiXmlElement* root = new TiXmlElement("filters");

	for (unsigned int i = 0; i < m_pFilters.count(); i++)
	{
		ArchiveFilterEntry* pAE = m_pFilters[i];
		TiXmlElement* tpl = new TiXmlElement("filter");

		pAE->ToXml(*tpl);

		root->LinkEndChild(tpl);
	}

	doc.LinkEndChild(root);
	doc.SaveFile((FakeUtf8String)lpFileName);

	return true;
}

void ArchiveFilter::AddStopFilter(ArchiveFilterEntry* pFE)
{
	m_pStopFilters.add(pFE);
}