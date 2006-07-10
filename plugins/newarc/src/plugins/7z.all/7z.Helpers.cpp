#include "7z.h"
#include <objbase.h>

bool CInFile::Open (const char *lpFileName)
{
	HANDLE hFile = CreateFile (
			lpFileName,
			GENERIC_READ,
			FILE_SHARE_READ,
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

CInFile::CInFile ()
{
	m_nRefCount = 1;
	m_hFile = INVALID_HANDLE_VALUE;
}

CInFile::~CInFile ()
{
	if ( m_hFile != INVALID_HANDLE_VALUE )
		CloseHandle (m_hFile);
}

unsigned __int64 CInFile::GetSize ()
{
	DWORD dwLoPart, dwHiPart;

	dwLoPart = GetFileSize (m_hFile, &dwHiPart);

	return ((unsigned __int64)dwHiPart)*0x100000000ull+(unsigned __int64)dwLoPart;

}


HRESULT __stdcall CInFile::QueryInterface (const IID &iid, void ** ppvObject)
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

ULONG __stdcall CInFile::AddRef ()
{
	return ++m_nRefCount;
}

ULONG __stdcall CInFile::Release ()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}



HRESULT __stdcall CInFile::Read (void *data, unsigned int size, unsigned int *processedSize)
{
	return ReadFile (m_hFile, data, size, (LPDWORD)processedSize, NULL)?S_OK:E_FAIL;
}

HRESULT __stdcall CInFile::Seek (__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition)
{
	DWORD hi, lo;

	hi = offset >> 32;
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


CArchiveExtractCallback::CArchiveExtractCallback (
		SevenZipArchive *pArchive,
		ArchiveItem *pItems,
		int nItemsNumber,
		const char *lpDestPath,
		const char *lpCurrentFolder
		)
{
	m_pArchive = pArchive;
	m_nRefCount = 1;

	m_pItems = pItems;
	m_nItemsNumber = nItemsNumber;

	m_lpDestPath = StrDuplicate (lpDestPath);
	m_lpCurrentFolder = StrDuplicate (lpCurrentFolder);

	FSF.AddEndSlash (m_lpDestPath);
	FSF.AddEndSlash (m_lpCurrentFolder);

	m_nLastProcessed = -1;

	m_pGetTextPassword = NULL;
}


CArchiveExtractCallback::~CArchiveExtractCallback ()
{
	StrFree (m_lpDestPath);
	StrFree (m_lpCurrentFolder);

	if ( m_pGetTextPassword )
		m_pGetTextPassword->Release ();
}



ULONG __stdcall CArchiveExtractCallback::AddRef ()
{
	return ++m_nRefCount;
}

ULONG __stdcall CArchiveExtractCallback::Release ()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}


HRESULT __stdcall CArchiveExtractCallback::QueryInterface (const IID &iid, void ** ppvObject)
{
	*ppvObject = NULL;

	if ( iid == IID_IArchiveExtractCallback )
	{
		*ppvObject = this;
		AddRef ();

		return S_OK;
	}

	if ( iid == IID_ICryptoGetTextPassword )
	{
		if ( !m_pGetTextPassword )
			m_pGetTextPassword = new CCryptoGetTextPassword (m_pArchive);

		m_pGetTextPassword->AddRef ();

		*ppvObject = m_pGetTextPassword; //????

		return S_OK;
	}


	return E_NOINTERFACE;
}


HRESULT __stdcall CArchiveExtractCallback::SetTotal (unsigned __int64 total)
{
	m_nLastProcessed = -1;
	return S_OK;
}

HRESULT CArchiveExtractCallback::SetCompleted (const unsigned __int64* completeValue)
{
	if ( m_nLastProcessed != -1 )
	{
		unsigned __int64 diff = *completeValue-m_nLastProcessed;

		if ( m_pArchive->m_pfnCallback && !m_pArchive->m_pfnCallback (AM_PROCESS_DATA, 0, (int)diff) )
			return E_ABORT;

		m_nLastProcessed = *completeValue;
	}

	return S_OK;
}


void CreateDirectory(char *FullPath) //$ 16.05.2002 AA
{
//	if( !FileExists (FullPath) )
	{
		for (char *c = FullPath; *c; c++)
		{
			if(*c!=' ')
			{
				for(; *c; c++)
					if(*c=='\\')
						{
							*c=0;
							CreateDirectory(FullPath, NULL);
							*c='\\';
						}
				CreateDirectory(FullPath, NULL);
				break;
			}
		}
	}
}


void CreateDirs (const char *lpFileName)
{
	char *lpNameCopy = StrDuplicate (lpFileName);

	CutToSlash (lpNameCopy);

	CreateDirectory (lpNameCopy);

	StrFree (lpNameCopy);
}


int GetItemIndex (CArchiveExtractCallback *pcb, int index)
{
	for (int i = 0; i < pcb->m_nItemsNumber; i++)
	{
		if ( pcb->m_pItems[i].nIndex == index )
			return i;
	}

	return -1;
}



HRESULT __stdcall CArchiveExtractCallback::GetStream (
		unsigned int index,
		ISequentialOutStream **outStream,
		int askExtractMode
		)

{
	PROPVARIANT value;

	IInArchive *archive = m_pArchive->m_pArchive;
	VariantInit ((VARIANTARG*)&value);

	if ( askExtractMode == 0 ) //extract
	{
		if ( SUCCEEDED (archive->GetProperty (
				index,
				kpidPath,
				&value
				)) )
		{
			char szArcFileName[NM];
			char szFullName[NM];

			if ( value.vt == VT_BSTR )
				WideCharToMultiByte (CP_OEMCP, 0, value.bstrVal, -1, szArcFileName, NM, NULL, NULL);
			else
			{
				strcpy (szArcFileName, FSF.PointToName (m_pArchive->m_lpFileName));
				CutTo (szArcFileName, '.', true);
			}

			strcpy (szFullName, m_lpDestPath);

			FSF.AddEndSlash (szFullName);

			if ( !FSF.LStrnicmp (szArcFileName, m_lpCurrentFolder, strlen (m_lpCurrentFolder)) )
				strcat (szFullName, szArcFileName+strlen (m_lpCurrentFolder));
			else
				strcat (szFullName, szArcFileName);

			int itemindex = GetItemIndex (this, index);
			PluginPanelItem *item = m_pItems[itemindex].pItem;

			if ( m_pArchive->m_pfnCallback )
				m_pArchive->m_pfnCallback (AM_START_EXTRACT_FILE, (int)item, (int)szFullName);

			if ( m_nLastProcessed == -1 )
				m_nLastProcessed = 0;

			CreateDirs (szFullName);

			COutFile *file = new COutFile;
			file->Open (szFullName);

			*outStream = file;
			VariantClear ((VARIANTARG*)&value);
		}
	}
	else
		*outStream = NULL;

	return S_OK;
}

HRESULT __stdcall CArchiveExtractCallback::PrepareOperation (int askExtractMode)
{
	return S_OK;
}

HRESULT __stdcall CArchiveExtractCallback::SetOperationResult (int resultEOperationResult)
{
	return S_OK;
}



CCryptoGetTextPassword::CCryptoGetTextPassword(SevenZipArchive *pArchive)
{
	m_pArchive = pArchive;
	m_nRefCount = 1;
}


ULONG __stdcall CCryptoGetTextPassword::AddRef ()
{
	return ++m_nRefCount;
}

ULONG __stdcall CCryptoGetTextPassword::Release ()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}


HRESULT __stdcall CCryptoGetTextPassword::QueryInterface (const IID &iid, void ** ppvObject)
{
	*ppvObject = NULL;

	if ( iid == IID_ICryptoGetTextPassword )
	{
		*ppvObject = this;
		AddRef ();

		return S_OK;
	}

	return E_NOINTERFACE;
}

HRESULT __stdcall CCryptoGetTextPassword::CryptoGetTextPassword (BSTR *password)
{
	ArchivePassword Password;

	if ( m_pArchive->GetPasswordLength () == -1 ) //password not defined
	{
		Password.lpBuffer = StrCreate (260);
		Password.dwBufferSize = 260;

		if ( m_pArchive->m_pfnCallback && m_pArchive->m_pfnCallback (AM_NEED_PASSWORD, PASSWORD_FILE, (int)&Password) );
		{
			wchar_t wszPassword[MAX_PATH];

			MultiByteToWideChar(CP_OEMCP, 0, Password.lpBuffer, -1, wszPassword, MAX_PATH);

			m_pArchive->SetPassword (Password.lpBuffer, strlen (Password.lpBuffer)+1); //LAME!!!
   			*password = SysAllocString (wszPassword);
		}

		StrFree (Password.lpBuffer);
	}
	else
	{
		char *lpPassword = (char*)malloc (m_pArchive->GetPasswordLength());

		m_pArchive->GetPassword (lpPassword);

		wchar_t wszPassword[MAX_PATH];

		MultiByteToWideChar(CP_OEMCP, 0, lpPassword, -1, wszPassword, MAX_PATH);

   		*password = SysAllocString (wszPassword);

   		free (lpPassword);
	}

	return S_OK;
}



bool COutFile::Open (const char *lpFileName)
{
	HANDLE hFile = CreateFile (
			lpFileName,
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

COutFile::COutFile ()
{
	m_nRefCount = 1;
	m_hFile = INVALID_HANDLE_VALUE;
}

COutFile::~COutFile ()
{
	if ( m_hFile != INVALID_HANDLE_VALUE )
		CloseHandle (m_hFile);
}


HRESULT __stdcall COutFile::QueryInterface (const IID &iid, void ** ppvObject)
{
	*ppvObject = NULL;

	if ( iid == IID_ISequentialOutStream )
	{
		*ppvObject = this;
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


HRESULT __stdcall COutFile::Write (const void *data, unsigned int size, unsigned int *processedSize)
{
	return WriteFile (m_hFile, data, size, (LPDWORD)processedSize, NULL)?S_OK:E_FAIL;
}


ULONG __stdcall CArchiveOpenCallback::AddRef ()
{
	return ++m_nRefCount;
}

ULONG __stdcall CArchiveOpenCallback::Release ()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}


HRESULT __stdcall CArchiveOpenCallback::QueryInterface (const IID &iid, void ** ppvObject)
{
	*ppvObject = NULL;

	if ( iid == IID_IArchiveOpenCallback )
	{
		*ppvObject = this;
		AddRef ();

		return S_OK;
	}

	if ( iid == IID_IArchiveOpenVolumeCallback )
	{
		if ( !m_pArchiveOpenVolumeCallback )
			m_pArchiveOpenVolumeCallback = new CArchiveOpenVolumeCallback (m_pArchive);

		m_pArchiveOpenVolumeCallback->AddRef ();

		*ppvObject = m_pArchiveOpenVolumeCallback; //????

		return S_OK;
	}

	if ( iid == IID_ICryptoGetTextPassword )
	{
		if ( !m_pGetTextPassword )
			m_pGetTextPassword = new CCryptoGetTextPassword (m_pArchive);

		m_pGetTextPassword->AddRef ();

		*ppvObject = m_pGetTextPassword; //????

		return S_OK;
	}


	return E_NOINTERFACE;
}

CArchiveOpenCallback::CArchiveOpenCallback (SevenZipArchive *pArchive)
{
	m_nRefCount = 1;
	m_pArchive = pArchive;
	m_pGetTextPassword = NULL;

	m_pArchiveOpenVolumeCallback = NULL;
}

CArchiveOpenCallback::~CArchiveOpenCallback ()
{
	if ( m_pGetTextPassword )
		m_pGetTextPassword->Release ();

	if ( m_pArchiveOpenVolumeCallback )
		m_pArchiveOpenVolumeCallback->Release ();
}

HRESULT __stdcall CArchiveOpenCallback::SetTotal (const UInt64 *files, const UInt64 *bytes)
{
	return S_OK;
}

HRESULT __stdcall CArchiveOpenCallback::SetCompleted (const UInt64 *files, const UInt64 *bytes)
{
	return S_OK;
}

CArchiveOpenVolumeCallback::CArchiveOpenVolumeCallback(SevenZipArchive *pArchive)
{
	m_nRefCount = 1;
	m_pArchive = pArchive;
	m_pCurrentFile = NULL;
}

CArchiveOpenVolumeCallback::~CArchiveOpenVolumeCallback ()
{
}


ULONG __stdcall CArchiveOpenVolumeCallback::AddRef ()
{
	return ++m_nRefCount;
}

ULONG __stdcall CArchiveOpenVolumeCallback::Release ()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}


HRESULT __stdcall CArchiveOpenVolumeCallback::QueryInterface (const IID &iid, void ** ppvObject)
{
	*ppvObject = NULL;

	if ( iid == IID_IArchiveOpenVolumeCallback )
	{
		*ppvObject = this;
		AddRef ();

		return S_OK;
	}

	return E_NOINTERFACE;
}


HRESULT __stdcall CArchiveOpenVolumeCallback::GetProperty (PROPID propID, PROPVARIANT *value)
{
	if ( propID == kpidName )
	{
		wchar_t wszNameOnly[MAX_PATH];

		//m_pCurrentFile!!! check here!!!
		MultiByteToWideChar (CP_OEMCP, 0, FSF.PointToName (m_pArchive->m_lpFileName), -1, wszNameOnly, MAX_PATH);

		value->vt = VT_BSTR;
		value->bstrVal = SysAllocString(wszNameOnly);
	}

	if ( propID == kpidSize )
	{
		value->vt = VT_UI8;

		if ( m_pCurrentFile )
			value->uhVal.QuadPart = m_pCurrentFile->GetSize ();
		else
			value->uhVal.QuadPart = m_pArchive->m_pInFile->GetSize ();
	}

	return S_OK;
}

HRESULT __stdcall CArchiveOpenVolumeCallback::GetStream (const wchar_t *name, IInStream **inStream)
{
	char szFullName[MAX_PATH];
	char szFileName[MAX_PATH];

	WideCharToMultiByte(CP_OEMCP, 0, name, -1, szFileName, MAX_PATH, NULL, NULL);

	strcpy (szFullName, m_pArchive->m_lpFileName);
	CutTo (szFullName, '\\', false);
	strcat (szFullName, szFileName);

	CInFile *file = new CInFile;

	bool bResult = file->Open(szFullName);

	*inStream = file;
	m_pCurrentFile = file;

	return bResult?S_OK:S_FALSE;
}
