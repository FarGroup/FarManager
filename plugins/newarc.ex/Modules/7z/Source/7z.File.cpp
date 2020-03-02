#include "7z.h"

bool CInFile::Open()
{
	HANDLE hFile = CreateFile (
			m_strFileName,
			GENERIC_READ,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL
			);

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		m_hFile = hFile;
		return true;
	}

	return false;
}

bool CInFile::Create()
{
	HANDLE hFile = CreateFile (
			m_strFileName,
			GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL
			);

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		m_hFile = hFile;
		return true;
	}

	return false;
}

void CInFile::Close()
{
	if ( m_hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle (m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
}

CInFile::CInFile(const TCHAR *lpFileName)
{
	m_nRefCount = 1;
	m_hFile = INVALID_HANDLE_VALUE;

	m_strFileName = lpFileName;
}

CInFile::~CInFile()
{
	Close();
}

bool CInFile::GetFindData(WIN32_FIND_DATA& fData)
{
	return apiGetFindData(m_strFileName, fData);
}

const TCHAR *CInFile::GetName()
{
	return m_strFileName;
}


HRESULT __stdcall CInFile::QueryInterface(const IID &iid, void ** ppvObject)
{
	*ppvObject = NULL;

	if ( iid == IID_IInStream )
	{
		*ppvObject = this;
		AddRef ();

		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG __stdcall CInFile::AddRef()
{
	return ++m_nRefCount;
}

ULONG __stdcall CInFile::Release()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}


HRESULT __stdcall CInFile::Read(void *data, unsigned int size, unsigned int *processedSize)
{
	DWORD dwRead;

	if ( ReadFile (m_hFile, data, size, &dwRead, NULL) )
	{
		if ( processedSize )
			*processedSize = dwRead;

		return S_OK;
	}

	return E_FAIL;
}

HRESULT __stdcall CInFile::Seek (__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition)
{
	DWORD hi, lo;

	hi = (DWORD)(offset >> 32);
	lo = (DWORD)offset;

	lo = SetFilePointer (m_hFile, lo, (PLONG)&hi, seekOrigin);

	if ( (lo == INVALID_SET_FILE_POINTER) && (GetLastError () != NO_ERROR) )
		return E_FAIL;
	else
	{
		if ( newPosition )
			*newPosition = ((unsigned __int64)hi)*0x100000000ull+(unsigned __int64)lo;
		return S_OK;
	}
}

bool COutFile::Open ()
{
	HANDLE hFile = CreateFile (
			m_strFileName,
			GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL
			);

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		m_hFile = hFile;
		return true;
	}
/*	else
	{
		int error = GetLastError();
		return false;
	}*/

	return false;
}

void COutFile::Close ()
{
	if ( m_hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle (m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
}


bool COutFile::SetTime (const FILETIME* lpCreationTime, const FILETIME* lpLastAccessTime, const FILETIME* lpLastWriteTime)
{
	return (bool)SetFileTime (m_hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime);
}

bool COutFile::SetAttributes (DWORD dwFileAttributes)
{
	return (bool)SetFileAttributes (m_strFileName, dwFileAttributes);
}

COutFile::COutFile (const TCHAR *lpFileName)
{
	m_nRefCount = 1;
	m_hFile = INVALID_HANDLE_VALUE;
	m_strFileName = lpFileName;
}

COutFile::~COutFile ()
{
	Close();
}


HRESULT __stdcall COutFile::QueryInterface (const IID &iid, void ** ppvObject)
{
	*ppvObject = NULL;

	if ( iid == IID_ISequentialOutStream )
	{
		*ppvObject = (void*)(ISequentialOutStream*)this;
		AddRef ();

		return S_OK;
	}

	if ( iid == IID_IOutStream )
	{
		*ppvObject = (void*)(IOutStream*)this;
		AddRef ();

		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG __stdcall COutFile::AddRef ()
{
	return ++m_nRefCount;
}

ULONG __stdcall COutFile::Release ()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}


HRESULT __stdcall COutFile::Write(const void *data, unsigned int size, unsigned int *processedSize)
{
	DWORD dwWritten;

	if ( WriteFile (m_hFile, data, size, &dwWritten, NULL) )
	{
		if ( processedSize )
			*processedSize = dwWritten;

		return S_OK;
	}

	return E_FAIL;
}

HRESULT __stdcall COutFile::Seek(__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition)
{
	DWORD hi, lo;

	hi = (DWORD)(offset >> 32);
	lo = (DWORD)offset;

	lo = SetFilePointer (m_hFile, lo, (PLONG)&hi, seekOrigin);

	if ( (lo == INVALID_SET_FILE_POINTER) && (GetLastError () != NO_ERROR) )
		return E_FAIL;
	else
	{
		if ( newPosition )
			*newPosition = ((unsigned __int64)hi)*0x100000000ull+(unsigned __int64)lo;
		return S_OK;
	}
}

HRESULT __stdcall COutFile::SetSize(__int64 newSize)
{
	return S_OK;
}

///

CVolumeOutFile::CVolumeOutFile(const TCHAR* lpFileName, unsigned __int64 uVolumeSize)
{
	m_nRefCount = 1;

	m_strFileName = lpFileName;
	m_uVolumeSize = uVolumeSize;

	_streamIndex = 0;
	_offsetPos = 0;
	_absPos = 0;
	_length = 0;
}

CVolumeOutFile::~CVolumeOutFile()
{
	Close();
}

void CVolumeOutFile::Close()
{
	for (unsigned int i = 0; i < m_streams.count(); i++)
	{
		VolumeInfo* pVolume = m_streams[i];

		if ( pVolume->stream )
			pVolume->stream->Close();
	}
}

HRESULT __stdcall CVolumeOutFile::Write(const void *data, unsigned int size, unsigned int* processedSize)
{
	if ( processedSize != NULL )
		*processedSize = 0;

	while ( size > 0 )
	{		
		if ( _streamIndex >= m_streams.count() )
		{
			VolumeInfo* pVolume = new VolumeInfo;

			string strNameOnly = m_strFileName;
			CutTo(strNameOnly, _T('.'), true);

			pVolume->strFileName.Format(_T("%s.%03d"), strNameOnly.GetString(), _streamIndex+1);
			
			pVolume->stream = new COutFile(pVolume->strFileName);
			pVolume->stream->Open(); //check result!!

			pVolume->pos = 0;
			pVolume->realSize = 0;

			m_streams.add(pVolume);
			continue;
		}

		VolumeInfo* pVolume = m_streams[_streamIndex];

		unsigned __int64 volSize = m_uVolumeSize;

		if ( _offsetPos >= volSize )
		{
			_offsetPos -= volSize;
			_streamIndex++;
			continue;
		}

		if ( _offsetPos != pVolume->pos )
		{
			pVolume->stream->Seek(_offsetPos, FILE_BEGIN, NULL); //???file_begin???
			pVolume->pos = _offsetPos;
		}

		unsigned int curSize = (unsigned int)min((unsigned __int64)size, volSize-pVolume->pos);
		unsigned int realProcessed;

		pVolume->stream->Write(data, curSize, &realProcessed);

		data = (void*)((unsigned char*)data+realProcessed);

		size -= realProcessed;
		pVolume->pos += realProcessed;
		_offsetPos += realProcessed;
		_absPos += realProcessed;

		if ( _absPos > _length )
			_length = _absPos;

		if ( _offsetPos > pVolume->realSize )
			pVolume->realSize = _offsetPos;

		if ( processedSize )
			*processedSize += realProcessed;

		if ( pVolume->pos == volSize )
		{
			_streamIndex++;
			_offsetPos = 0;
		}

		if ( (realProcessed == 0) && (curSize != 0) )
			return E_FAIL;

		break; //с чего бы вдруг?
	}
	
	return S_OK;
}

HRESULT __stdcall CVolumeOutFile::Seek(__int64 offset, unsigned int seekOrigin, unsigned __int64* newPosition)
{
	switch( seekOrigin )
	{

	case FILE_BEGIN:
		_absPos = offset;
		break;

    case FILE_CURRENT:
		_absPos += offset;
		break;

	case FILE_END:
		_absPos = _length + offset;
		break;
	}

	_offsetPos = _absPos;

	if ( newPosition )
		*newPosition = _absPos;
	
	_streamIndex = 0;

	return S_OK;
}

HRESULT __stdcall CVolumeOutFile::SetSize(__int64 newSize)
{
	if ( newSize < 0 )
		return E_INVALIDARG;

	for (unsigned int i = 0; i < m_streams.count(); i++)
	{
		VolumeInfo* pVolume = m_streams[i];

		if ( (unsigned __int64)newSize < pVolume->realSize )
		{
			pVolume->stream->SetSize(newSize);
			pVolume->realSize = newSize;
			break;
		}

		newSize -= pVolume->realSize;
	}

	for (unsigned int i = 0; i < m_streams.count(); i++)
		DeleteFile(m_streams[i]->strFileName);

	m_streams.reset();

	_offsetPos = _absPos;
	_streamIndex = 0;
	_length = newSize;
	
	return S_OK;
}

HRESULT __stdcall CVolumeOutFile::QueryInterface(const IID &iid, void ** ppvObject)
{
	*ppvObject = NULL;

	if ( iid == IID_ISequentialOutStream )
	{
		*ppvObject = (void*)(ISequentialOutStream*)this;
		AddRef ();

		return S_OK;
	}

	if ( iid == IID_IOutStream )
	{
		*ppvObject = (void*)(IOutStream*)this;
		AddRef ();

		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG __stdcall CVolumeOutFile::AddRef()
{
	return ++m_nRefCount;
}

ULONG __stdcall CVolumeOutFile::Release()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}
