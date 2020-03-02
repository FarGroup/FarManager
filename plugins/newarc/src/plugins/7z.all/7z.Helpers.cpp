#include "7z.h"
#include <objbase.h>
#include <debug.h>

bool CInFile::Open ()
{
	HANDLE hFile = CreateFile (
			m_lpFileName,
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

bool CInFile::Create ()
{
	HANDLE hFile = CreateFile (
			m_lpFileName,
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

void CInFile::Close ()
{
	if ( m_hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle (m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
}

CInFile::CInFile (const char *lpFileName)
{
	m_nRefCount = 1;
	m_hFile = INVALID_HANDLE_VALUE;

	m_lpFileName = StrDuplicate (lpFileName);
}

CInFile::~CInFile ()
{
	if ( m_hFile != INVALID_HANDLE_VALUE )
		CloseHandle (m_hFile);

	StrFree (m_lpFileName);
}

unsigned __int64 CInFile::GetSize ()
{
	DWORD dwLoPart, dwHiPart;

	dwLoPart = GetFileSize (m_hFile, &dwHiPart);

	return ((unsigned __int64)dwHiPart)*0x100000000ull+(unsigned __int64)dwLoPart;

}

const char *CInFile::GetName ()
{
	return m_lpFileName;
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
			m_lpFileName,
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
	return (bool)SetFileAttributes (m_lpFileName, dwFileAttributes);
}

COutFile::COutFile (const char *lpFileName)
{
	m_nRefCount = 1;
	m_hFile = INVALID_HANDLE_VALUE;
	m_lpFileName = StrDuplicate (lpFileName);
}

COutFile::~COutFile ()
{
	StrFree (m_lpFileName);

	if ( m_hFile != INVALID_HANDLE_VALUE )
		CloseHandle (m_hFile);
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


HRESULT __stdcall COutFile::Write (const void *data, unsigned int size, unsigned int *processedSize)
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

HRESULT __stdcall COutFile::Seek (__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition)
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

HRESULT __stdcall COutFile::SetSize (__int64 newSize)
{
	return S_OK;
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

	m_nLastProcessed = (unsigned __int64)-1;

	m_pGetTextPassword = NULL;

	m_nState = -1;
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
			m_pGetTextPassword = new CCryptoGetTextPassword (m_pArchive, TYPE_FILE);

		m_pGetTextPassword->AddRef ();

		*ppvObject = m_pGetTextPassword; //????

		return S_OK;
	}


	return E_NOINTERFACE;
}


HRESULT __stdcall CArchiveExtractCallback::SetTotal (unsigned __int64 total)
{
	m_nLastProcessed = (unsigned __int64)-1;
	return S_OK;
}

HRESULT CArchiveExtractCallback::SetCompleted (const unsigned __int64* completeValue)
{
	if ( (__int64)m_nLastProcessed != -1 )
	{
		unsigned __int64 diff = *completeValue-m_nLastProcessed;

		if ( !m_pArchive->Callback (AM_PROCESS_DATA, 0, (unsigned int)diff) )
			return E_ABORT;

		m_nLastProcessed = *completeValue;
	}

	return S_OK;
}


void CreateDirectoryEx (const char *lpFullPath, DWORD dwFileAttributes = INVALID_FILE_ATTRIBUTES) //$ 16.05.2002 AA
{
	char *lpFullPathCopy = StrDuplicate(lpFullPath);

//	if( !FileExists (FullPath) )
	{
		for (char *c = lpFullPathCopy; *c; c++)
		{
			if(*c!=' ')
			{
				for(; *c; c++)
					if(*c=='\\')
						{
							*c=0;
							CreateDirectory (lpFullPathCopy, NULL);
							*c='\\';
						}

				CreateDirectory(lpFullPathCopy, NULL);

				if ( dwFileAttributes != INVALID_FILE_ATTRIBUTES )
					SetFileAttributes(lpFullPathCopy, dwFileAttributes);

				break;
			}
		}
	}

	free (lpFullPathCopy);
}


void CreateDirs (const char *lpFileName)
{
	char *lpNameCopy = StrDuplicate (lpFileName);

	CutToSlash (lpNameCopy);

	CreateDirectoryEx (lpNameCopy);

	StrFree (lpNameCopy);
}


int GetItemIndex (CArchiveExtractCallback *pcb, int index)
{
	for (int i = 0; i < pcb->m_nItemsNumber; i++)
	{
		if ( (int)pcb->m_pItems[i].nIndex == index )
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
	CPropVariant value;

	IInArchive *archive = m_pArchive->m_pArchive;

	if ( askExtractMode == 0 ) //extract
	{
		if ( archive->GetProperty (index, kpidPath, &value) != S_OK )
			return S_OK; //!!! to return error

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

		if ( !FSF.LStrnicmp (szArcFileName, m_lpCurrentFolder, StrLength(m_lpCurrentFolder)) )
			strcat (szFullName, szArcFileName+StrLength(m_lpCurrentFolder));
		else
			strcat (szFullName, szArcFileName);

		int itemindex = GetItemIndex (this, index);
		PluginPanelItem *item = m_pItems[itemindex].pItem;

		ProcessFileStruct pfs;

		pfs.pItem = item;
		pfs.lpDestFileName = szFullName;

		m_pArchive->Callback (AM_PROCESS_FILE, 0, (LONG_PTR)&pfs);

		if ( (int)m_nLastProcessed == -1 )
			m_nLastProcessed = 0;

   		FILETIME ftCreationTime, ftLastAccessTime, ftLastWriteTime;
   		DWORD dwFileAttributes = 0;

   		memset (&ftCreationTime, 0, sizeof (FILETIME));
   		memset (&ftLastAccessTime, 0, sizeof (FILETIME));
   		memset (&ftLastWriteTime, 0, sizeof (FILETIME));

   		if ( archive->GetProperty (index, kpidAttrib, &value) == S_OK )
   		{
   			if ( value.vt == VT_UI4 )
   				dwFileAttributes = value.ulVal;
   		}

   		if ( archive->GetProperty (index, kpidCTime, &value) == S_OK )
   		{
   			if ( value.vt == VT_FILETIME )
   				memcpy (&ftCreationTime, &value.filetime, sizeof (FILETIME));
   		}

   		if ( archive->GetProperty (index, kpidATime, &value) == S_OK )
   		{
   			if ( value.vt == VT_FILETIME )
   				memcpy (&ftLastAccessTime, &value.filetime, sizeof (FILETIME));
   		}

   		if ( archive->GetProperty (index, kpidMTime, &value) == S_OK )
   		{
   			if ( value.vt == VT_FILETIME )
   				memcpy (&ftLastWriteTime, &value.filetime, sizeof (FILETIME));
   		}

   		bool bIsFolder = false;

   		if ( archive->GetProperty (index, kpidIsDir, &value) == S_OK )
   		{
   			if (value.vt == VT_BOOL)
   				bIsFolder = (value.boolVal == VARIANT_TRUE);
		}

		if ( bIsFolder ||
			 OptionIsOn (dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )//||
			 //OptionIsOn (item->FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
		{
			*outStream = NULL;
			CreateDirectoryEx (szFullName, dwFileAttributes);
		}
		else
		{
			CreateDirs (szFullName);

			COutFile *file = new COutFile (szFullName);

			if ( file->Open () )
			{
				file->SetAttributes (dwFileAttributes);
				file->SetTime (&ftCreationTime, &ftLastAccessTime, &ftLastWriteTime);
				*outStream = file;
			}
			else
                delete file;
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
	switch( resultEOperationResult )
	{
		case NArchive::NExtract::NOperationResult::kCRCError:
			m_pArchive->Callback (AM_NEED_PASSWORD, PASSWORD_RESET, 0);
			return E_FAIL;
	}

	return S_OK;
}



CCryptoGetTextPassword::CCryptoGetTextPassword(SevenZipArchive *pArchive, int nType)
{
	m_pArchive = pArchive;
	m_nRefCount = 1;
	m_nType = nType;
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

	if ( m_nType == TYPE_LISTING )
		m_pArchive->m_bListPassword = true;

	Password.lpBuffer = StrCreate (260);
	Password.dwBufferSize = 260;

	if ( m_pArchive->Callback (AM_NEED_PASSWORD, (m_nType == TYPE_FILE)?PASSWORD_FILE:PASSWORD_LIST, (LONG_PTR)&Password) )
	{
		wchar_t wszPassword[MAX_PATH];

		MultiByteToWideChar(CP_OEMCP, 0, Password.lpBuffer, -1, wszPassword, MAX_PATH);

		//m_pArchive->SetPassword (Password.lpBuffer, strlen (Password.lpBuffer)+1); //LAME!!!
		*password = SysAllocString (wszPassword);
	}

	StrFree (Password.lpBuffer);

	return S_OK;
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
			m_pGetTextPassword = new CCryptoGetTextPassword (m_pArchive, TYPE_LISTING);

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
	m_bProgressMessage = false;
	m_dwStartTime = GetTickCount ();

	m_hScreen = Info.SaveScreen (0, 0, -1, -1);
}

CArchiveOpenCallback::~CArchiveOpenCallback ()
{
	if ( m_pGetTextPassword )
		m_pGetTextPassword->Release ();

	if ( m_pArchiveOpenVolumeCallback )
		m_pArchiveOpenVolumeCallback->Release ();

	Info.RestoreScreen (m_hScreen);
}

HRESULT __stdcall CArchiveOpenCallback::SetTotal (const UInt64 *files, const UInt64 *bytes)
{
	return S_OK;
}


bool CheckForEsc ()
{
	bool EC = false;

	INPUT_RECORD rec;
	DWORD ReadCount;

	while (true)
	{
		PeekConsoleInput (GetStdHandle (STD_INPUT_HANDLE),&rec,1,&ReadCount);

		if ( ReadCount==0 )
			break;

		ReadConsoleInput (GetStdHandle (STD_INPUT_HANDLE),&rec,1,&ReadCount);

		if ( rec.EventType==KEY_EVENT )
		{
			if ( (rec.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE) &&
				 rec.Event.KeyEvent.bKeyDown )
				EC = true;
		}
	}

	return EC;
}


HRESULT __stdcall CArchiveOpenCallback::SetCompleted (const UInt64 *files, const UInt64 *bytes)
{
	if ( CheckForEsc() )
		return E_FAIL;

	if ( !(*files & 0x1f)  && (GetTickCount ()-m_dwStartTime > 500) )
	{
	   	char szFileCount[100];
   		const char *pMsgs[4];

	   	pMsgs[0] = "Подождите";
   		pMsgs[1] = "Чтение архива [7z.all]";
	   	pMsgs[2] = m_pArchive->m_lpFileName;
   		pMsgs[3] = szFileCount;

	   	FSF.sprintf (szFileCount, "%I64u файлов", *files);

   		Info.Message(
   				Info.ModuleNumber,
   				m_bProgressMessage?FMSG_KEEPBACKGROUND:0,
	   			NULL,
   				pMsgs,
   				4,
   				0
	   			);

	   	m_bProgressMessage = true;
	}

	return S_OK;
}

CArchiveOpenVolumeCallback::CArchiveOpenVolumeCallback(SevenZipArchive *pArchive)
{
	m_nRefCount = 1;
	m_pArchive = pArchive;
	m_pVolumeFile = NULL;
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

		if ( m_pVolumeFile )
			MultiByteToWideChar (CP_OEMCP, 0, FSF.PointToName (m_pVolumeFile->GetName()), -1, wszNameOnly, MAX_PATH);
		else
			MultiByteToWideChar (CP_OEMCP, 0, FSF.PointToName (m_pArchive->m_pInFile->GetName()), -1, wszNameOnly, MAX_PATH);


		value->vt = VT_BSTR;
		value->bstrVal = SysAllocString(wszNameOnly);
	}

	if ( propID == kpidSize )
	{
		value->vt = VT_UI8;

		if ( m_pVolumeFile )
			value->uhVal.QuadPart = m_pVolumeFile->GetSize ();
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

	CInFile *file = new CInFile (szFullName);

	bool bResult = file->Open();

	*inStream = file;
	m_pVolumeFile = file;

	return bResult?S_OK:S_FALSE;
}



CArchiveUpdateCallback::CArchiveUpdateCallback (
		SevenZipArchive *pArchive,
		pointer_array<ArchiveUpdateItem*> *indicies,
		const char *lpSourcePath,
		const char *lpCurrentPath
		)
{
	m_nRefCount = 1;
	m_indicies = indicies;
	m_pArchive = pArchive;

	m_lpSourcePath = lpSourcePath;
	m_lpCurrentPath = lpCurrentPath;
}

CArchiveUpdateCallback::~CArchiveUpdateCallback()
{
}


ULONG __stdcall CArchiveUpdateCallback::AddRef ()
{
	return ++m_nRefCount;
}

ULONG __stdcall CArchiveUpdateCallback::Release ()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}


HRESULT __stdcall CArchiveUpdateCallback::QueryInterface (const IID &iid, void ** ppvObject)
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
	else

	if ( iid == IID_IArchiveUpdateCallback2 )
	{
		*ppvObject = (void*)(IArchiveUpdateCallback2*)this;
		AddRef ();

		return S_OK;
	}

/*	else
	{
		BSTR bstr;

		StringFromIID(iid, &bstr);

		//MessageBoxW (0, bstr, L"inteface req", MB_OK);

		SysFreeString(bstr);
	}*/

	return E_NOINTERFACE;
}



HRESULT __stdcall CArchiveUpdateCallback::SetTotal (unsigned __int64 total)
{
	OperationStructPlugin os;

	os.uTotalFiles = 1;
	os.uTotalSize = total;
	os.dwFlags = OS_FLAG_TOTALFILES|OS_FLAG_TOTALSIZE;

	m_nLastProcessed = 0;

	m_pArchive->Callback (AM_START_OPERATION, OPERATION_ADD, (LONG_PTR)&os);
	m_pArchive->Callback (AM_PROCESS_FILE, 0, 0);

	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::SetCompleted (const unsigned __int64* completeValue)
{
	unsigned __int64 diff = *completeValue-m_nLastProcessed;

	if ( !m_pArchive->Callback (AM_PROCESS_DATA, 0, (LONG_PTR)diff) )
		return E_ABORT;

	m_nLastProcessed = *completeValue;

	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::GetUpdateItemInfo (
			unsigned int index,
			int *newData, // 1 - new data, 0 - old data
			int *newProperties, // 1 - new properties, 0 - old properties
			unsigned int *indexInArchive // -1 if there is no in archive, or if doesn't matter
			)
{
	ArchiveUpdateItem *item = m_indicies->at(index);

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

	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::GetProperty (unsigned int index, PROPID propID, PROPVARIANT *value)
{
	ArchiveUpdateItem *item = m_indicies->at(index);

	if ( item->bNewFile )
	{
		PluginPanelItem *pitem = item->pItem;

		if ( propID == kpidPath )
		{
			wchar_t wszNameOnly[MAX_PATH];

			MultiByteToWideChar (CP_OEMCP, 0, pitem->FindData.cFileName, -1, wszNameOnly, MAX_PATH);

			wchar_t wszFullPath[MAX_PATH];

			memset (wszFullPath, 0, sizeof (wszFullPath));

			if ( m_lpCurrentPath && m_lpCurrentPath )
			{
				MultiByteToWideChar (CP_OEMCP, 0, m_lpCurrentPath, -1, wszFullPath, MAX_PATH);

				if ( wszFullPath[0] )
					wcscat (wszFullPath, L"\\");

				wcscat (wszFullPath, wszNameOnly);
			}
			else
				wcscpy (wszFullPath, wszNameOnly);

			value->vt = VT_BSTR;
			value->bstrVal = SysAllocString(wszFullPath);
		}
		else

		if ( propID == kpidAttrib )
		{
			value->vt = VT_UI4;
			value->ulVal = pitem->FindData.dwFileAttributes;
		}
		else

		if ( propID == kpidMTime )
		{
			value->vt = VT_FILETIME;
			memcpy (&value->filetime, &pitem->FindData.ftLastWriteTime, sizeof (FILETIME));
		}
		else

		if ( propID == kpidCTime )
		{
			value->vt = VT_FILETIME;
			memcpy (&value->filetime, &pitem->FindData.ftCreationTime, sizeof (FILETIME));
		}
		else

		if ( propID == kpidATime )
		{
			value->vt = VT_FILETIME;
			memcpy (&value->filetime, &pitem->FindData.ftLastAccessTime, sizeof (FILETIME));
		}
		else

		if ( propID == kpidIsDir )
		{
			value->vt = VT_BOOL;
			value->boolVal = OptionIsOn (pitem->FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY)?VARIANT_TRUE:VARIANT_FALSE;
		}
		else

		if ( propID == kpidSize )
		{
			DWORD dwHi = pitem->FindData.nFileSizeHigh;
			DWORD dwLo = pitem->FindData.nFileSizeLow;

			value->vt = VT_UI8;
			value->uhVal.QuadPart = dwHi*0x1000000000ull+dwLo;
		}
	}
	else
		return m_pArchive->m_pArchive->GetProperty (item->index, propID, value);

	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::GetStream (unsigned int index, ISequentialInStream **inStream)
{
	ArchiveUpdateItem *item = m_indicies->at(index);

	*inStream = NULL;

	if ( item->bNewFile )
	{
		PluginPanelItem *pitem = item->pItem;

		char *lpFullName = (char*)malloc (MAX_PATH);

		if ( m_lpSourcePath && m_lpSourcePath )
		{
			strcpy (lpFullName, m_lpSourcePath);
			FSF.AddEndSlash (lpFullName);
		}

		strcat (lpFullName, pitem->FindData.cFileName);

		if ( !OptionIsOn (pitem->FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
		{
			CInFile *file = new CInFile (lpFullName);

			if ( file->Open () )
			{
				ProcessFileStruct pfs;

				pfs.pItem = pitem;
				pfs.lpDestFileName = lpFullName;

				m_pArchive->Callback (AM_PROCESS_FILE, 0, (LONG_PTR)&pfs);

				*inStream = file;
			}
			else
				delete file;
		}

		StrFree (lpFullName);
	}

	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::SetOperationResult (int operationResult)
{
	//MessageBox (0, "setres", "as", MB_OK);
	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::GetVolumeSize (unsigned int index, unsigned __int64 *size)
{
	//MessageBox (0, "get volume size", "as", MB_OK);
	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::GetVolumeStream (unsigned int index, ISequentialOutStream **volumeStream)
{
	//MessageBox (0, "get vol stream", "as", MB_OK);
	return S_OK;
}


HRESULT __stdcall CArchiveUpdateCallback::CryptoGetTextPassword2 (int *passwordIsDefined, BSTR *password)
{
	//MessageBox (0, "getpass", "as", MB_OK);

	if ( passwordIsDefined )
		*passwordIsDefined = 0;

	return S_OK;
}
