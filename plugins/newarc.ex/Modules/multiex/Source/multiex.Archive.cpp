#include "multiex.h"

MultiExArchive::MultiExArchive(
		MultiExPlugin* pPlugin, 
		const GUID& uidFormat, 
		const TCHAR *lpFileName,
		HANDLE hCallback,
		ARCHIVECALLBACK pfnCallback
		)
{
	m_uid = uidFormat;
	m_pPlugin = pPlugin;

	m_strFileName = lpFileName;

	m_hCallback = hCallback;
	m_pfnCallback = pfnCallback;

	m_nFormatIndex = m_pPlugin->GetFormatIndex(m_uid);

	m_bOpened = false;
}

const GUID& MultiExArchive::GetUID()
{
	return m_uid;
}

MultiExArchive::~MultiExArchive()
{
	Close();
}

bool MultiExArchive::Open()
{
	int hArchive = m_pPlugin->OpenArchive(m_nFormatIndex, m_strFileName);

	if ( hArchive )
	{
		m_hArchive = hArchive;
		m_bOpened = true;
	}

	m_hSearch = NULL;

	return m_bOpened;
}

void MultiExArchive::Close()
{
	if ( m_bOpened )
	{
		m_pPlugin->CloseArchive(m_hArchive);
		m_bOpened = false;
	}
}

bool MultiExArchive::StartOperation(int nOperation, bool bInternal)
{
	if ( !bInternal )
		Close();
	else
	{
		if ( !Open() )
			return false;
	}

	return true;
}

void MultiExArchive::EndOperation(int nOperation, bool bInternal)
{
}

int MultiExArchive::GetArchiveItem(ArchiveItem* pItem)
{
	return m_pPlugin->GetArchiveItem(m_hArchive, m_hSearch, pItem);
}

void MultiExArchive::FreeArchiveItem(ArchiveItem *pItem)
{
	return m_pPlugin->FreeArchiveItem(m_hArchive, pItem);
}


bool MultiExArchive::Extract(
		const ArchiveItem* pItems,
		int nItemsNumber,
		const TCHAR* lpDestDiskPath,
		const TCHAR* lpPathInArchive
		)
{
	return m_pPlugin->Extract(m_hArchive, pItems, nItemsNumber, lpDestDiskPath, lpPathInArchive);
}

LONG_PTR MultiExArchive::Callback (int nMsg, int nParam1, LONG_PTR nParam2)
{
	if ( m_pfnCallback )
		return m_pfnCallback(m_hCallback, nMsg, nParam1, nParam2);

	return FALSE;
}


bool MultiExArchive::Delete(const ArchiveItem *pItems, int nItemsNumber)
{
	return m_pPlugin->Delete(m_hArchive, pItems, nItemsNumber);
}

bool MultiExArchive::AddFiles(
		const ArchiveItem *pItems, 
		int nItemsNumber, 
		const TCHAR *lpSourceDiskPath, 
		const TCHAR *lpPathInArchive
		)
{
	return m_pPlugin->AddFiles(m_hArchive, pItems, nItemsNumber, lpSourceDiskPath, lpPathInArchive);
}

