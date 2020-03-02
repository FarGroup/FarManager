#include "newarc.h"

ArchivePlugin::ArchivePlugin(ArchiveModule* pModule, const ArchivePluginInfo* pInfo)
{
	m_pModule = pModule;

	m_strModuleName = pInfo->lpModuleName;
	m_uid = pInfo->uid;
	m_dwFlags = pInfo->dwFlags;

	for (unsigned int i = 0; i < pInfo->uFormats; i++)
		m_pFormats.add(new ArchiveFormat(this, &pInfo->pFormats[i]));
}

ArchivePlugin::~ArchivePlugin()
{
}

const TCHAR* ArchivePlugin::GetModuleName() const
{
	return m_strModuleName;
}

const GUID& ArchivePlugin::GetUID() const
{
	return m_uid;
}

bool ArchivePlugin::QueryCapability(DWORD dwFlags) const
{
	return (m_dwFlags & dwFlags) == dwFlags;
}

ArchiveModule* ArchivePlugin::GetModule()
{
	return m_pModule;
}

ArchiveFormat* ArchivePlugin::GetFormat(const GUID& uid)
{
	for (unsigned int i = 0; i < m_pFormats.count(); i++)
		if ( m_pFormats[i]->GetUID() == uid )
			return m_pFormats[i];

	return NULL;
}

int ArchivePlugin::GetFormats(Array<ArchiveFormat*>& formats)
{
	for (unsigned int i = 0; i < m_pFormats.count(); i++)
		formats.add(m_pFormats[i]);

	return 0;
}

Array<ArchiveFormat*>& ArchivePlugin::GetFormats()
{
	return m_pFormats;
}
