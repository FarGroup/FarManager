#include "ace.h"

// {0394EC83-D41A-4C61-979E-8F274B9E3F0D}
MY_DEFINE_GUID(CLSID_FormatACE, 0x394ec83, 0xd41a, 0x4c61, 0x97, 0x9e, 0x8f, 0x27, 0x4b, 0x9e, 0x3f, 0xd);


AceArchive::AceArchive(
		AcePlugin* pPlugin, 
		const TCHAR *lpFileName, 
		HANDLE hCallback, 
		ARCHIVECALLBACK pfnCallback,
		bool bCreate
		)
{
	m_pPlugin = pPlugin;
	m_strFileName = lpFileName;

	m_pfnCallback = pfnCallback;
	m_hCallback = hCallback;

	m_hArchive = NULL;
}

const GUID& AceArchive::GetUID()
{
	return CLSID_FormatACE;
}

AceArchive::~AceArchive()
{
}

bool AceArchive::StartOperation(int nOperation, bool bInternal)
{
	m_hArchive = m_pPlugin->OpenArchive(m_strFileName, nOperation == OPERATION_LIST);
	return (m_hArchive != NULL);
}

void AceArchive::EndOperation(int nOperation, bool bInternal)
{
	m_pPlugin->CloseArchive(m_hArchive);
}


int AceArchive::GetArchiveItem(ArchiveItem* pItem)
{
	return m_pPlugin->GetArchiveItem(pItem);
}

void AceArchive::FreeArchiveItem(ArchiveItem* pItem)
{
	m_pPlugin->FreeArchiveItem(pItem);
}

bool AceArchive::Extract(
		const ArchiveItem* pItems,
		int nItemsNumber,
		const TCHAR* lpDestDiskPath,
		const TCHAR* lpPathInArchive
		)
{
	return m_pPlugin->Extract(m_hArchive, pItems, nItemsNumber, lpDestDiskPath, lpPathInArchive);
}

bool AceArchive::Delete(
		const ArchiveItem* pItems,
		int nItemsNumber
		)
{
	return m_pPlugin->Delete(m_hArchive, pItems, nItemsNumber);
}

bool AceArchive::AddFiles(
		const ArchiveItem* pItems,
		int nItemsNumber,
		const TCHAR* lpSourceDiskPath,
		const TCHAR* lpPathInArchive
		)
{
	return m_pPlugin->AddFiles(m_hArchive, pItems, nItemsNumber, lpSourceDiskPath, lpPathInArchive);
}

/*
LONG_PTR AceArchive::Callback (int nMsg, int nParam1, LONG_PTR nParam2)
{
	if ( m_pfnCallback )
		return m_pfnCallback (m_hCallback, nMsg, nParam1, nParam2);

	return FALSE;
}
*/
