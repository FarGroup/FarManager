#include "newarc.h"

ArchiveFormat::ArchiveFormat(ArchivePlugin* pPlugin, const ArchiveFormatInfo* pInfo)
{
	m_pPlugin = pPlugin;
	m_strDefaultExtention = pInfo->lpDefaultExtention;
	m_strName = pInfo->lpName;
	m_uid = pInfo->uid;
	m_dwFlags = pInfo->dwFlags;
}

ArchiveFormat::~ArchiveFormat()
{
}

const GUID& ArchiveFormat::GetUID() const
{
	return m_uid;
}

const TCHAR* ArchiveFormat::GetDefaultExtention() const
{
	return m_strDefaultExtention;
}

const TCHAR* ArchiveFormat::GetName() const
{
	return m_strName;
}

bool ArchiveFormat::QueryCapability(DWORD dwFlags) const
{
	return (m_dwFlags & dwFlags) == dwFlags;
}

ArchivePlugin* ArchiveFormat::GetPlugin()
{
	return m_pPlugin;
}

ArchiveModule* ArchiveFormat::GetModule()
{
	return m_pPlugin->GetModule();
}

bool ArchiveFormat::GetDefaultCommand(int nCommand, string& strCommand, bool& bEnabled)
{
	return GetModule()->GetDefaultCommand(GetPlugin()->GetUID(), GetUID(), nCommand, strCommand, bEnabled);
}

bool ArchiveFormat::Configure(const TCHAR* lpInitialConfig, string& strResultConfig)
{
	return GetModule()->ConfigureFormat(GetPlugin()->GetUID(), GetUID(), lpInitialConfig, strResultConfig);
}
