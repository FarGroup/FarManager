#include "d5d.h"

D5DArchive::D5DArchive(
		D5DPlugin* pPlugin, 
		const GUID& uid, 
		const TCHAR *lpFileName,
		HANDLE hCallback,
		ARCHIVECALLBACK pfnCallback
		)
{
	m_uid = uid;
	m_pPlugin = pPlugin;

	m_strFileName = lpFileName;
	m_hArchive = NULL;

	m_pfnCallback = pfnCallback;
	m_hCallback = hCallback;

	m_nIndex = 0;
	m_nItemsNumber = 0;

	m_bOpened = false;
}

const GUID& D5DArchive::GetUID()
{
	return m_uid;
}

D5DArchive::~D5DArchive()
{
	Close();
}


bool D5DArchive::Open()
{
	if ( !m_bOpened )
	{
		m_nItemsNumber = m_pPlugin->ReadFormat(m_strFileName);

		if ( m_nItemsNumber >= 0 )
			m_bOpened = true;
	}

	m_nIndex = m_nItemsNumber;

	return m_bOpened;
}

void D5DArchive::Close()
{
	if ( m_bOpened )
	{
		m_pPlugin->CloseFormat();
		m_bOpened = false;
	}
}

bool D5DArchive::StartOperation(int nOperation, bool bInternal)
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

bool D5DArchive::EndOperation(int nOperation, bool bInternal)
{
	return true;
}

int D5DArchive::GetArchiveItem(ArchiveItem* pItem)
{
	if ( m_nIndex < 0 )
		return E_EOF;

	return m_pPlugin->GetEntry(m_nIndex, pItem);
}

void D5DArchive::FreeArchiveItem(ArchiveItem *pItem)
{
	m_pPlugin->FreeEntry(pItem);
}


bool D5DArchive::Extract(
		const ArchiveItem* pItems,
		int nItemsNumber,
		const TCHAR* lpDestPath,
		const TCHAR* lpCurrentFolder
		)
{
	return m_pPlugin->Extract(pItems, nItemsNumber, lpDestPath, lpCurrentFolder);
}

LONG_PTR D5DArchive::Callback (int nMsg, int nParam1, LONG_PTR nParam2)
{
	if ( m_pfnCallback )
		return m_pfnCallback (m_hCallback, nMsg, nParam1, nParam2);

	return FALSE;
}





