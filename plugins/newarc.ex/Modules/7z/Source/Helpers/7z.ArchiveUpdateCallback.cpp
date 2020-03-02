#include "7z.h"
#include <objbase.h>


CArchiveUpdateCallback::CArchiveUpdateCallback(
		SevenZipArchive* pArchive,
		const TCHAR* lpPassword,
		const Array<ArchiveUpdateItem*>& indicies,
		const TCHAR *lpSourceDiskPath,
		const TCHAR *lpPathInArchive
		) : m_indicies(indicies)
{
	m_nRefCount = 1;

	//m_indicies = indicies;

	m_pArchive = pArchive;
	m_strPassword = lpPassword;

	m_strSourceDiskPath = lpSourceDiskPath;
	m_strPathInArchive = lpPathInArchive;

	m_uItemsNumber = indicies.count();

	m_uSuccessCount = 0;
}

CArchiveUpdateCallback::~CArchiveUpdateCallback()
{
}

int CArchiveUpdateCallback::GetResult()
{
///	if ( m_bUserAbort )
//		return RESULT_CANCEL;

	if ( m_uSuccessCount > m_uItemsNumber )
		__debug(_T("LOGIC ERROR, PLEASE REPORT"));

	//if ( m_uSuccessCount == 0 )
		//return RESULT_ERROR;

	if ( m_uSuccessCount < m_uItemsNumber )
		return RESULT_PARTIAL;

	return RESULT_SUCCESS;
}


ULONG __stdcall CArchiveUpdateCallback::AddRef()
{
	return ++m_nRefCount;
}

ULONG __stdcall CArchiveUpdateCallback::Release()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}


HRESULT __stdcall CArchiveUpdateCallback::QueryInterface(const IID &iid, void ** ppvObject)
{
	*ppvObject = NULL;

	if ( iid == IID_IArchiveUpdateCallback )
	{
		*ppvObject = (void*)(IArchiveUpdateCallback*)this;
		AddRef ();

		return S_OK;
	}
	else

	if ( iid == IID_ICryptoGetTextPassword2 )
	{
		*ppvObject = (void*)(ICryptoGetTextPassword2*)this;
		AddRef ();
	}
/*	else

	if ( iid == IID_IArchiveUpdateCallback2 )
	{
		*ppvObject = (void*)(IArchiveUpdateCallback2*)this;
		AddRef ();

		return S_OK;
	}
	*/
	return E_NOINTERFACE;
}





HRESULT __stdcall CArchiveUpdateCallback::SetTotal(unsigned __int64 total)
{
	m_uProcessedBytesTotal = 0;
	m_uTotalBytes = total;

	m_pArchive->OnStartOperation(OPERATION_ADD, m_uTotalBytes, m_uItemsNumber);

	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::SetCompleted(const unsigned __int64* completeValue)
{
	m_uProcessedBytesFile += *completeValue-m_uProcessedBytesTotal;
	m_uProcessedBytesTotal = *completeValue;

	if ( !m_pArchive->OnProcessData(
			m_uProcessedBytesFile,
			m_uTotalBytesFile,
			m_uProcessedBytesTotal,
			m_uTotalBytes
			) )
		return E_ABORT;

	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::GetUpdateItemInfo(
			unsigned int index,
			int *newData, // 1 - new data, 0 - old data
			int *newProperties, // 1 - new properties, 0 - old properties
			unsigned int *indexInArchive // -1 if there is no in archive, or if doesn't matter
			)
{
	ArchiveUpdateItem* item = m_indicies.at(index);

	if ( indexInArchive )
		*indexInArchive = item->index; //-1 on new file

	if ( newData )
	{
		if ( item->bNewFile )
			*newData = 1;
		else
			*newData = 0;
	}

	if ( newProperties )
	{
		if ( item->bNewFile )
			*newProperties = 1;
		else
			*newProperties = 0;
	}

	if ( !item->bNewFile )
		m_uSuccessCount++; //???AAAA

	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::GetProperty(unsigned int index, PROPID propID, PROPVARIANT *value)
{
	ArchiveUpdateItem* item = m_indicies.at(index);

	if ( item->bNewFile )
	{
		const ArchiveItem* pitem = item->pItem;

		if ( propID == kpidPath )
		{
			string strFullPath;

			if ( !m_strPathInArchive.IsEmpty() )
				strFullPath = m_strPathInArchive;
			
			strFullPath += pitem->lpFileName;

			value->vt = VT_BSTR;

#ifdef UNICODE
			value->bstrVal = SysAllocString(strFullPath);
#else	
			value->bstrVal = strFullPath.ToBSTR(CP_OEMCP);
#endif
		}
		else

		if ( propID == kpidAttrib )
		{
			value->vt = VT_UI4;
			value->ulVal = pitem->dwFileAttributes;
		}
		else

		if ( propID == kpidMTime )
		{
			value->vt = VT_FILETIME;
			memcpy (&value->filetime, &pitem->ftLastWriteTime, sizeof (FILETIME));
		}
		else

		if ( propID == kpidCTime )
		{
			value->vt = VT_FILETIME;
			memcpy (&value->filetime, &pitem->ftCreationTime, sizeof (FILETIME));
		}
		else

		if ( propID == kpidATime )
		{
			value->vt = VT_FILETIME;
			memcpy (&value->filetime, &pitem->ftLastAccessTime, sizeof (FILETIME));
		}
		else

		if ( propID == kpidIsDir )
		{
			value->vt = VT_BOOL;
			value->boolVal = OptionIsOn(pitem->dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY)?VARIANT_TRUE:VARIANT_FALSE;
		}
		else

		if ( propID == kpidSize )
		{
			value->vt = VT_UI8;
			value->uhVal.QuadPart = pitem->nFileSize;
		}
	}
	else
		return m_pArchive->GetArchive()->GetProperty(item->index, propID, value);

	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::GetStream(unsigned int index, ISequentialInStream **inStream)
{
	ArchiveUpdateItem* item = m_indicies.at(index);

	*inStream = NULL;

	if ( item->bNewFile )
	{
		const ArchiveItem *pitem = item->pItem;

		string strFullName;

		if ( !m_strSourceDiskPath.IsEmpty() )
			strFullName = m_strSourceDiskPath;

		strFullName += pitem->lpFileName;

		m_uProcessedBytesFile = 0;
		m_uTotalBytesFile = pitem->nFileSize;

		m_pArchive->OnEnterStage(STAGE_ADDING);
		m_pArchive->OnProcessFile(pitem, strFullName);

		if ( !OptionIsOn(pitem->dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
		{
			CInFile *file = new CInFile(strFullName);

			if ( file->Open () )
				*inStream = file;
			else
				delete file;
		}
	}
	else
	{
		ArchiveItem item;

		m_pArchive->GetArchiveItem(index, &item);

		m_uProcessedBytesFile = 0;
		m_uTotalBytesFile = item.nFileSize;

		m_pArchive->OnEnterStage(STAGE_UPDATING);
		m_pArchive->OnProcessFile(&item, nullptr);

		m_pArchive->FreeArchiveItem(&item);
	}

	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::SetOperationResult(int operationResult)
{
	if ( operationResult == NArchive::NUpdate::NOperationResult::kOK )
		m_uSuccessCount++; //BUGBUG, to check errors
	else
		m_pArchive->OnReportError(nullptr, ADD_ERROR_UNKNOWN);

	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::GetVolumeSize(unsigned int index, unsigned __int64 *size)
{
	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::GetVolumeStream(unsigned int index, ISequentialOutStream **volumeStream)
{
	return S_OK;
}


HRESULT __stdcall CArchiveUpdateCallback::CryptoGetTextPassword2(int *passwordIsDefined, BSTR *password)
{
	if ( passwordIsDefined )
		*passwordIsDefined = !m_strPassword.IsEmpty();

	if ( !m_strPassword.IsEmpty() )
	{
#ifdef UNICODE
		*password = SysAllocString(m_strPassword);
#else
		wchar_t* lpBuffer = AnsiToUnicode(m_strPassword);
		*password = SysAllocString(lpBuffer);
		free(lpBuffer);
#endif
	}

	return S_OK;
}
