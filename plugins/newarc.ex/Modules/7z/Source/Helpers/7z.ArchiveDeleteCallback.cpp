#include "7z.h"
#include <objbase.h>

CArchiveDeleteCallback::CArchiveDeleteCallback(
		SevenZipArchive* pArchive,
		const Array<ArchiveUpdateItem*>& indicies
		) : m_indicies(indicies)
{
	m_nRefCount = 1;
	m_pArchive = pArchive;
}

CArchiveDeleteCallback::~CArchiveDeleteCallback()
{
}

int CArchiveDeleteCallback::GetResult()
{
	return RESULT_SUCCESS; //no way to check for errors
}


ULONG __stdcall CArchiveDeleteCallback::AddRef()
{
	return ++m_nRefCount;
}

ULONG __stdcall CArchiveDeleteCallback::Release()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}


HRESULT __stdcall CArchiveDeleteCallback::QueryInterface(const IID &iid, void ** ppvObject)
{
	*ppvObject = NULL;

	if ( iid == IID_IArchiveUpdateCallback )
	{
		*ppvObject = (void*)(IArchiveUpdateCallback*)this;
		AddRef();

		return S_OK;
	}
	else

	if ( iid == IID_ICryptoGetTextPassword2 )
	{
		*ppvObject = (void*)(ICryptoGetTextPassword2*)this;
		AddRef ();
	}

	return E_NOINTERFACE;
}

HRESULT __stdcall CArchiveDeleteCallback::SetTotal(unsigned __int64 total)
{
	m_uProcessedBytesTotal = 0;
	m_uTotalBytes = total;

	m_pArchive->OnStartOperation(OPERATION_DELETE, m_uTotalBytes, 0);

	return S_OK;
}

HRESULT __stdcall CArchiveDeleteCallback::SetCompleted(const unsigned __int64* completeValue)
{
	m_uProcessedBytesTotal = *completeValue;

	if ( !m_pArchive->OnProcessData(
			0,
			0,
			m_uProcessedBytesTotal,
			m_uTotalBytes
			) )
		return E_ABORT;

	return S_OK;
}

HRESULT __stdcall CArchiveDeleteCallback::GetUpdateItemInfo(
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
		*newData = 0;

	if ( newProperties )
		*newProperties = 0;

	return S_OK;
}

//not called
HRESULT __stdcall CArchiveDeleteCallback::GetProperty(unsigned int index, PROPID propID, PROPVARIANT *value)
{
	return S_OK; //hmm...
}

//not called
HRESULT __stdcall CArchiveDeleteCallback::GetStream(unsigned int index, ISequentialInStream **inStream)
{
	*inStream = nullptr;
	return S_OK;
}

//not called
HRESULT __stdcall CArchiveDeleteCallback::SetOperationResult(int operationResult)
{
	return S_OK;
}


HRESULT __stdcall CArchiveDeleteCallback::CryptoGetTextPassword2(int *passwordIsDefined, BSTR *password)
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
