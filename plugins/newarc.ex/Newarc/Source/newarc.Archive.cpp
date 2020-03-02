#include "newarc.h"

#include "msg/msgError.cpp"


bool GetFileInfo(
		const TCHAR *lpFileName,
		FILETIME *pAccessTime,
		FILETIME *pWriteTime,
		DWORD *pSize
		)
{
	bool bResult = false;

	WIN32_FIND_DATA fData;

	HANDLE hSearch = FindFirstFile (
			lpFileName,
			&fData
			);

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		memcpy(pAccessTime, &fData.ftLastAccessTime, sizeof(FILETIME));
		memcpy(pWriteTime, &fData.ftLastWriteTime, sizeof(FILETIME));
		
		*pSize = fData.nFileSizeLow;

		bResult = true;

		FindClose (hSearch);
	}

	return bResult;
}

Archive::Archive(
		HANDLE hArchive,
		ArchiveFormat* pFormat,
		const TCHAR *lpFileName
		)
{
	m_strFileName = lpFileName;

	m_hArchive = hArchive;
	m_pModule = pFormat->GetModule();
	m_pFormat = pFormat;

	memset(&m_ArchiveLastAccessTime, 0, sizeof(FILETIME));
	memset(&m_ArchiveLastWriteTime, 0, sizeof(FILETIME));

	m_dwArchiveFileSizeLow = 0;

	m_uid = pFormat->GetUID();

	_tree = nullptr;
}

bool Archive::SetCurrentDirectory(const TCHAR* lpPathInArchive, bool bRelative)
{
	ArchiveTreeNode* pRoot = bRelative?_current:_tree;
	ArchiveTreeNode* pResult = pRoot->GetNode(lpPathInArchive);

	if ( pResult )
	{
		_current = pResult;
		pResult->GetPath(m_strPathInArchive);

		return TRUE;
	}

	return FALSE;
}

const TCHAR* Archive::GetCurrentDirectory()
{
	return m_strPathInArchive.GetString();
}

const GUID& Archive::GetUID() const
{
	return m_uid;
}

HANDLE Archive::GetHandle() const
{
	return m_hArchive;
}


bool Archive::QueryCapability(DWORD dwFlags) const
{
	return m_pFormat->QueryCapability(dwFlags);
}



extern bool CheckForEsc();


bool Archive::ReadArchiveItems(bool bForce)
{
	if ( bForce || WasUpdated() )
	{
		FreeArchiveItems();

		_tree = new ArchiveTree();
		_current = _tree;

		if ( StartOperation(OPERATION_LIST, true) )
		{
			int nResult = E_SUCCESS;

			while ( (nResult == E_SUCCESS) && !CheckForEsc() )
			{
				ArchiveItem* item = new ArchiveItem;
				memset(item, 0, sizeof(ArchiveItem));

				nResult = GetArchiveItem(item);

				if ( nResult == E_SUCCESS )
					_tree->AddItem(item->lpFileName, item);
			}

			EndOperation(OPERATION_LIST, true);
		}
	}

	return true;
}


void Archive::FreeArchiveItemsHelper(ArchiveTree* tree)
{
	if ( tree )
	{
		for (ArchiveTreeNodesIterator itr = tree->children.begin(); itr != tree->children.end(); ++itr)
			FreeArchiveItemsHelper(itr->second);

		if ( tree->item != nullptr )
		{
			FreeArchiveItem(tree->item);
			delete tree->item;
		}
	}
}

void Archive::FreeArchiveItems()
{
	FreeArchiveItemsHelper(_tree);

	delete _tree;

	_tree = nullptr;
	_current = nullptr;
}

ArchiveTreeNode* Archive::GetRoot()
{
	return _tree;
}

void Archive::GetArchiveTreeItems(Array<ArchiveTreeNode*>& items, bool bRecursive)
{
	for (ArchiveTreeNodesIterator itr = _current->children.begin(); itr != _current->children.end(); ++itr)
		items.add(itr->second);
}


bool Archive::WasUpdated()
{
	FILETIME NewAccessTime;
	FILETIME NewWriteTime;
	DWORD dwFileSizeLow;

	GetFileInfo(m_strFileName, &NewAccessTime, &NewWriteTime, &dwFileSizeLow);

	bool bResult = CompareFileTime(&NewAccessTime, &m_ArchiveLastAccessTime) ||
			CompareFileTime(&NewWriteTime, &m_ArchiveLastWriteTime) ||
			(dwFileSizeLow != m_dwArchiveFileSizeLow);

	if ( bResult )
	{			
		memcpy(&m_ArchiveLastAccessTime, &NewAccessTime, sizeof(FILETIME));
		memcpy(&m_ArchiveLastWriteTime, &NewWriteTime, sizeof(FILETIME));
		m_dwArchiveFileSizeLow = dwFileSizeLow;
	}

	return bResult;
}

Archive::~Archive ()
{
	//MessageBox(0, _T("delete archive"), _T("asdf"), MB_OK);

	FreeArchiveItems();

	//MessageBox(0, _T("delete archive 2"), _T("asdf"), MB_OK);

}

int Archive::Extract(
		const ArchiveItemArray& items,
		const TCHAR *lpDestDiskPath,
		bool bWithoutPath
		)
{
	int nResult = RESULT_ERROR;

	string strDestDiskPath = lpDestDiskPath;
	AddEndSlash(strDestDiskPath);

	if ( QueryCapability(AFF_SUPPORT_INTERNAL_EXTRACT) && m_hArchive )
	{
		if ( StartOperation(OPERATION_EXTRACT, true) )
		{
			nResult = m_pModule->Extract(
						m_hArchive, 
						items, 
						strDestDiskPath, 
						m_strPathInArchive
						);

			EndOperation(OPERATION_EXTRACT, true);
		}
	}


	return nResult;
}

int Archive::Test(const ArchiveItemArray& items)
{
	int nResult = RESULT_ERROR;

	if ( QueryCapability(AFF_SUPPORT_INTERNAL_TEST) && m_hArchive )
	{
		if ( StartOperation(OPERATION_TEST, true) )
		{
			nResult = m_pModule->Test(m_hArchive, items);
			EndOperation(OPERATION_TEST, true);
		}
	}

	return nResult;
}

int Archive::AddFiles(
		const ArchiveItemArray& items,
		const TCHAR* lpSourceDiskPath,
		const TCHAR* lpConfig
		)
{
	int nResult = RESULT_ERROR;

	string strSourceDiskPath = lpSourceDiskPath;
	AddEndSlash(strSourceDiskPath);

	string strPathInArchive = m_strPathInArchive;
	AddEndSlash(strPathInArchive);

	if ( QueryCapability(AFF_SUPPORT_INTERNAL_ADD) && m_hArchive )
	{
		if ( StartOperation(OPERATION_ADD, true) )
		{
			nResult = m_pModule->AddFiles(
					m_hArchive, 
					items, 
					strSourceDiskPath, 
					strPathInArchive,
					lpConfig
					);
		
			EndOperation(OPERATION_ADD, true);
		}
	}

	return nResult;
}


int Archive::Delete(const ArchiveItemArray& items)
{
	int nResult = false;

	if ( QueryCapability(AFF_SUPPORT_INTERNAL_DELETE) && m_hArchive )
	{
		if ( StartOperation(OPERATION_DELETE, true) )
		{
			nResult = m_pModule->Delete(
					m_hArchive,
					items
					);

			EndOperation(OPERATION_DELETE, true);
		}
	}

	return nResult;
}

bool Archive::StartOperation(int nOperation, bool bInternal)
{
	bool bResult = true;
	
	if ( m_hArchive ) //dummy archive has m_hArchive == 0
	{
		if ( bInternal || m_pFormat->QueryCapability(AFF_NEED_EXTERNAL_NOTIFICATIONS) )
			bResult = m_pModule->StartOperation(m_hArchive, nOperation, bInternal);

		if ( bResult )
			m_pModule->GetArchiveFormat(m_hArchive, &m_uid); //уточним, а вдруг это другой формат )))
	}
	
	return bResult;
}

void Archive::EndOperation(int nOperation, bool bInternal)
{
	if ( m_hArchive )
	{
		if ( bInternal || m_pFormat->QueryCapability(AFF_NEED_EXTERNAL_NOTIFICATIONS) )
			m_pModule->EndOperation(m_hArchive, nOperation, bInternal);
	}
}

int Archive::GetArchiveInfo(bool& bMultiVolume, const ArchiveInfoItem** pItems)
{
	return m_pModule->GetArchiveInfo(m_hArchive, bMultiVolume, pItems);
}

int Archive::GetArchiveItem(ArchiveItem* pItem)
{
	return m_pModule->GetArchiveItem(m_hArchive, pItem);
}

bool Archive::FreeArchiveItem(ArchiveItem* pItem)
{
	return m_pModule->FreeArchiveItem(m_hArchive, pItem);
}


const TCHAR* Archive::GetFileName() const
{
	return m_strFileName;
}

ArchiveModule* Archive::GetModule()
{
	return m_pModule;
}

ArchiveFormat* Archive::GetFormat()
{
	return m_pFormat;
}

ArchivePlugin* Archive::GetPlugin()
{
	return m_pFormat->GetPlugin();
}


bool Archive::GetDefaultCommand(
		int nCommand,
		string& strCommand,
		bool& bEnabled
		)
{
	return m_pModule->GetDefaultCommand(GetPlugin()->GetUID(), m_uid, nCommand, strCommand, bEnabled);
}

int Archive::ExecuteAsOperation(
		int nOperation,
		EXECUTEFUNCTION pfnExecute,
		void* pParam
		)
{
	int nResult = RESULT_ERROR;

	if ( StartOperation(nOperation, false) )
	{
		nResult = pfnExecute(this, pParam);
		EndOperation(nOperation, false);
	}

	return nResult;
}