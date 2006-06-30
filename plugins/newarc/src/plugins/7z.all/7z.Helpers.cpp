#include "7z.h"

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
			*newPosition = ((unsigned __int64)hi)*0x100000000ll+(unsigned __int64)lo;
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
}


CArchiveExtractCallback::~CArchiveExtractCallback ()
{
	StrFree (m_lpDestPath);
	StrFree (m_lpCurrentFolder);
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

		m_pArchive->m_pfnCallback (AM_PROCESS_DATA, 0, (int)diff);

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
		*ppvObject = this;
		AddRef ();

		return S_OK;
	}


	return E_NOINTERFACE;
}

CArchiveOpenCallback::CArchiveOpenCallback ()
{
	m_nRefCount = 1;
}

HRESULT __stdcall CArchiveOpenCallback::SetTotal (const UInt64 *files, const UInt64 *bytes)
{
	return S_OK;
}

HRESULT __stdcall CArchiveOpenCallback::SetCompleted (const UInt64 *files, const UInt64 *bytes)
{
	return S_OK;
}


HRESULT __stdcall CArchiveOpenCallback::GetProperty (PROPID propID, PROPVARIANT *value)
{
	return S_OK;
}

HRESULT __stdcall CArchiveOpenCallback::GetStream (const wchar_t *name, IInStream **inStream)
{
	return S_OK;
}
