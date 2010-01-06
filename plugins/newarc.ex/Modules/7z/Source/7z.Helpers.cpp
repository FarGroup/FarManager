#include "7z.h"
#include <objbase.h>

CArchiveExtractCallback::CArchiveExtractCallback(
		SevenZipArchive* pArchive,
		ArchiveItemEx *pItems,
		int nItemsNumber,
		const TCHAR *lpDestDiskPath,
		const TCHAR *lpPathInArchive
		)
{
	m_pArchive = pArchive;

	m_nRefCount = 1;

	m_pItems = pItems;
	m_nItemsNumber = nItemsNumber;

	m_strDestDiskPath = lpDestDiskPath;
	m_strPathInArchive = lpPathInArchive;

	m_pGetTextPassword = NULL;

	m_bUserAbort = false;

	///???
	m_pArchive->OnStartOperation(OPERATION_EXTRACT, 0, 0);
}


CArchiveExtractCallback::~CArchiveExtractCallback()
{
	if ( m_pGetTextPassword )
		m_pGetTextPassword->Release ();
}

ULONG __stdcall CArchiveExtractCallback::AddRef()
{
	return ++m_nRefCount;
}

ULONG __stdcall CArchiveExtractCallback::Release()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}


HRESULT __stdcall CArchiveExtractCallback::QueryInterface(const IID &iid, void ** ppvObject)
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
			m_pGetTextPassword = new CCryptoGetTextPassword(m_pArchive, TYPE_FILE);

		m_pGetTextPassword->AddRef ();
		*ppvObject = m_pGetTextPassword; //????

		return S_OK;
	}


	return E_NOINTERFACE;
}


HRESULT __stdcall CArchiveExtractCallback::SetTotal(unsigned __int64 total)
{
	m_uProcessedBytes = (unsigned __int64)-1;
	return S_OK;
}

HRESULT CArchiveExtractCallback::SetCompleted(const unsigned __int64* completeValue)
{
	if ( m_uProcessedBytes != (unsigned __int64)-1 )
	{
		unsigned __int64 diff = *completeValue-m_uProcessedBytes;

		if ( !m_pArchive->OnProcessData(diff) )
			return E_ABORT;

		m_uProcessedBytes = *completeValue;
	}

	return S_OK;
}



void CreateDirs (const TCHAR *lpFileName)
{
	TCHAR *lpNameCopy = StrDuplicate (lpFileName);

	CutToSlash (lpNameCopy);

	apiCreateDirectoryEx (lpNameCopy);

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



HRESULT __stdcall CArchiveExtractCallback::GetStream(
		unsigned int index,
		ISequentialOutStream** outStream,
		int askExtractMode
		)

{
	CPropVariant value;

	IInArchive *pArchive = m_pArchive->GetArchive();

	if ( askExtractMode == 0 ) //extract
	{
		if ( pArchive->GetProperty(index, kpidPath, &value) != S_OK )
			return S_OK; //!!! to return error

		string strArcFileName;
		string strFullName;

		if ( value.vt == VT_BSTR )
		{
#ifdef UNICODE
			strArcFileName = value.bstrVal;
#else
			strArcFileName.SetData(value.bstrVal, CP_OEMCP);
#endif
		}
		else
		{
			strArcFileName = FSF.PointToName (m_pArchive->GetFileName());
			CutTo (strArcFileName, _T('.'), true);
		}

		strFullName = m_strDestDiskPath;

		if ( !FSF.LStrnicmp (strArcFileName, m_strPathInArchive, m_strPathInArchive.GetLength()) )
			strFullName += (const TCHAR*)strArcFileName+m_strPathInArchive.GetLength(); //FIX ASAP!!!
		else
			strFullName += strArcFileName;

		int itemindex = GetItemIndex (this, index);
		const ArchiveItem* item = m_pItems[itemindex].pItem;

		int nOverwrite = m_pArchive->OnProcessFile(item, strFullName);

		if ( nOverwrite == RESULT_CANCEL )
		{
			m_bUserAbort = true;
			*outStream = NULL;
			return S_OK;
		}

		if ( nOverwrite == RESULT_SKIP )
		{
			*outStream = NULL;
			return S_OK;
		}

		//а это что за бред?
		if ( m_uProcessedBytes == (unsigned __int64)-1 )
			m_uProcessedBytes = 0;

   		FILETIME ftCreationTime, ftLastAccessTime, ftLastWriteTime;
   		DWORD dwFileAttributes = 0;

   		memset (&ftCreationTime, 0, sizeof (FILETIME));
   		memset (&ftLastAccessTime, 0, sizeof (FILETIME));
   		memset (&ftLastWriteTime, 0, sizeof (FILETIME));

   		if ( pArchive->GetProperty(index, kpidAttrib, &value) == S_OK )
   		{
   			if ( value.vt == VT_UI4 )
   				dwFileAttributes = value.ulVal;
   		}

   		if ( pArchive->GetProperty(index, kpidCTime, &value) == S_OK )
   		{
   			if ( value.vt == VT_FILETIME )
   				memcpy (&ftCreationTime, &value.filetime, sizeof (FILETIME));
   		}

   		if ( pArchive->GetProperty(index, kpidATime, &value) == S_OK )
   		{
   			if ( value.vt == VT_FILETIME )
   				memcpy (&ftLastAccessTime, &value.filetime, sizeof (FILETIME));
   		}

   		if ( pArchive->GetProperty(index, kpidMTime, &value) == S_OK )
   		{
   			if ( value.vt == VT_FILETIME )
   				memcpy (&ftLastWriteTime, &value.filetime, sizeof (FILETIME));
   		}

   		bool bIsFolder = false;

   		if ( pArchive->GetProperty(index, kpidIsDir, &value) == S_OK )
   		{
   			if (value.vt == VT_BOOL)
   				bIsFolder = (value.boolVal == VARIANT_TRUE);
		}

		if ( bIsFolder ||
			 OptionIsOn (dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )//||
			 //OptionIsOn (item->FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
		{
			*outStream = NULL;
			apiCreateDirectoryEx(strFullName); //, dwFileAttributes);
		}
		else
		{
			CreateDirs(strFullName);

			COutFile *file = new COutFile(strFullName);

			if ( file->Open() )
			{
				file->SetAttributes(dwFileAttributes);
				file->SetTime(&ftCreationTime, &ftLastAccessTime, &ftLastWriteTime);
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

HRESULT __stdcall CArchiveExtractCallback::PrepareOperation(int askExtractMode)
{
	return S_OK;
}

HRESULT __stdcall CArchiveExtractCallback::SetOperationResult(int resultEOperationResult)
{
	switch( resultEOperationResult )
	{
		case NArchive::NExtract::NOperationResult::kCRCError:
			m_pArchive->OnPasswordOperation(PASSWORD_RESET, NULL, 0);
			return E_FAIL;
	}

	return S_OK;
}


CCryptoGetTextPassword::CCryptoGetTextPassword(
		SevenZipArchive* pArchive,
		int nType
		)
{
	m_pArchive = pArchive;

	m_nRefCount = 1;
	m_nType = nType;
}


ULONG __stdcall CCryptoGetTextPassword::AddRef()
{
	return ++m_nRefCount;
}

ULONG __stdcall CCryptoGetTextPassword::Release()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}


HRESULT __stdcall CCryptoGetTextPassword::QueryInterface(const IID &iid, void ** ppvObject)
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

HRESULT __stdcall CCryptoGetTextPassword::CryptoGetTextPassword(BSTR *password)
{
	TCHAR szPassword[512];

	if ( m_pArchive->OnPasswordOperation(PASSWORD_FILE, szPassword, 512) ) //not that good
	{
		string strPassword = szPassword;

		*password = strPassword.ToBSTR();
	}

	return S_OK;
}


CArchiveOpenCallback::CArchiveOpenCallback(SevenZipArchive* pArchive)
{
	m_nRefCount = 1;

	m_pArchive = pArchive;

	m_bProgressMessage = false;
	m_dwStartTime = GetTickCount();

	m_hScreen = Info.SaveScreen(0, 0, -1, -1);

	m_pGetTextPassword = NULL;
	m_pArchiveOpenVolumeCallback = NULL;
}

CArchiveOpenCallback::~CArchiveOpenCallback ()
{
	if ( m_pGetTextPassword )
		m_pGetTextPassword->Release ();

	if ( m_pArchiveOpenVolumeCallback )
		m_pArchiveOpenVolumeCallback->Release ();

	Info.RestoreScreen (m_hScreen);
}


ULONG __stdcall CArchiveOpenCallback::AddRef()
{
	return ++m_nRefCount;
}

ULONG __stdcall CArchiveOpenCallback::Release()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}


HRESULT __stdcall CArchiveOpenCallback::QueryInterface(const IID &iid, void ** ppvObject)
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
			m_pArchiveOpenVolumeCallback = new CArchiveOpenVolumeCallback(m_pArchive);

		m_pArchiveOpenVolumeCallback->AddRef ();

		*ppvObject = m_pArchiveOpenVolumeCallback; 

		return S_OK;
	}

	if ( iid == IID_ICryptoGetTextPassword )
	{
		if ( !m_pGetTextPassword )
			m_pGetTextPassword = new CCryptoGetTextPassword(m_pArchive, TYPE_LISTING);

		m_pGetTextPassword->AddRef ();

		*ppvObject = m_pGetTextPassword; 

		return S_OK;
	}


	return E_NOINTERFACE;
}



HRESULT __stdcall CArchiveOpenCallback::SetTotal(const UInt64 *files, const UInt64 *bytes)
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


HRESULT __stdcall CArchiveOpenCallback::SetCompleted(const UInt64 *files, const UInt64 *bytes)
{
	if ( CheckForEsc() )
		return E_FAIL;

	if ( files && !(*files & 0x1f)  && (GetTickCount ()-m_dwStartTime > 500) )
	{
		//это надо нафиг устранить, здесь должен быть callback
		FarMessage message(m_bProgressMessage?FMSG_KEEPBACKGROUND:0);

		string strFileCount;

		strFileCount.Format(_T("%I64u файлов"), *files);

	   	message.Add(_T("Подождите"));
   		message.Add(_T("Чтение архива [7z.all]"));
		message.Add(m_pArchive->GetFileName());
	   	message.Add(strFileCount);

	   	message.Run();

	   	m_bProgressMessage = true;
	}

	return S_OK;
}

CArchiveOpenVolumeCallback::CArchiveOpenVolumeCallback(SevenZipArchive* pArchive)
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


HRESULT __stdcall CArchiveOpenVolumeCallback::QueryInterface(const IID &iid, void ** ppvObject)
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


HRESULT __stdcall CArchiveOpenVolumeCallback::GetProperty(PROPID propID, PROPVARIANT *value)
{
	if ( propID == kpidName )
	{
		string strNameOnly;

		if ( m_pVolumeFile )
			strNameOnly = FSF.PointToName(m_pVolumeFile->GetName());
		else
			strNameOnly = FSF.PointToName(m_pArchive->GetFileName());

		value->vt = VT_BSTR;
		value->bstrVal = strNameOnly.ToBSTR();
	}

	if ( propID == kpidSize )
	{
		/*value->vt = VT_UI8;

		if ( m_pVolumeFile )
			value->uhVal.QuadPart = m_pVolumeFile->GetSize();
		else
			value->uhVal.QuadPart = m_pHandle->pFile->GetSize(); */

		__debug(_T("BUGBUG!!!"));
	}

	return S_OK;
}

HRESULT __stdcall CArchiveOpenVolumeCallback::GetStream(const wchar_t *name, IInStream **inStream)
{
	string strFullName;
	string strFileName;

#ifdef UNICODE
	strFileName = name;
#else
	strFileName.SetData(name, CP_OEMCP);
#endif

	strFullName = m_pArchive->GetFileName();
	CutTo(strFullName, _T('\\'), false);
	strFullName += strFileName;

	CInFile *file = new CInFile (strFullName);

	bool bResult = file->Open();

	*inStream = file;
	m_pVolumeFile = file;

	return bResult?S_OK:S_FALSE;
}



CArchiveUpdateCallback::CArchiveUpdateCallback(
		SevenZipArchive* pArchive,
		const TCHAR* lpPassword,
		const Array<ArchiveUpdateItem*>* indicies,
		const TCHAR *lpSourceDiskPath,
		const TCHAR *lpPathInArchive
		)
{
	m_nRefCount = 1;

	m_indicies = indicies;

	m_pArchive = pArchive;
	m_strPassword = lpPassword;

	m_strSourceDiskPath = lpSourceDiskPath;
	m_strPathInArchive = lpPathInArchive;
}

CArchiveUpdateCallback::~CArchiveUpdateCallback()
{
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
/*	if ( iid == IID_ICryptoGetTextPassword )
	{
		m_pGetTextPassword = new CCryptoGetTextPassword(m_pArchive, PASSWORD_LIST);
		m_pGetTextPassword->AddRef(); //??
		//new CCryp
		*ppvObject = m_pGetTextPassword;
		//(void*)(ICryptoGetTextPassword2*)this;
		//AddRef ();
	}
	else */

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
	m_uProcessedBytes = 0;
	m_pArchive->OnStartOperation(OPERATION_ADD, 1, total);

	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::SetCompleted(const unsigned __int64* completeValue)
{
	unsigned __int64 diff = *completeValue-m_uProcessedBytes;

	if ( !m_pArchive->OnProcessData((unsigned int)diff) )
		return E_ABORT;

	m_uProcessedBytes = *completeValue;

	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::GetUpdateItemInfo(
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

HRESULT __stdcall CArchiveUpdateCallback::GetProperty(unsigned int index, PROPID propID, PROPVARIANT *value)
{
	ArchiveUpdateItem *item = m_indicies->at(index);

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
			value->boolVal = OptionIsOn (pitem->dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY)?VARIANT_TRUE:VARIANT_FALSE;
		}
		else

		if ( propID == kpidSize )
		{
			value->vt = VT_UI8;
			value->uhVal.QuadPart = pitem->nFileSize;
		}
	}
	else
		return m_pArchive->GetArchive()->GetProperty (item->index, propID, value);

	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::GetStream(unsigned int index, ISequentialInStream **inStream)
{
	ArchiveUpdateItem *item = m_indicies->at(index);

	*inStream = NULL;

	if ( item->bNewFile )
	{
		const ArchiveItem *pitem = item->pItem;

		string strFullName;

		if ( !m_strSourceDiskPath.IsEmpty() )
			strFullName = m_strSourceDiskPath;

		strFullName += pitem->lpFileName;

		if ( !OptionIsOn(pitem->dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
		{
			CInFile *file = new CInFile(strFullName);

			if ( file->Open () )
			{
				m_pArchive->OnProcessFile(pitem, strFullName);
				*inStream = file;
			}
			else
				delete file;
		}
	}

	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::SetOperationResult (int operationResult)
{
	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::GetVolumeSize (unsigned int index, unsigned __int64 *size)
{
	return S_OK;
}

HRESULT __stdcall CArchiveUpdateCallback::GetVolumeStream (unsigned int index, ISequentialOutStream **volumeStream)
{
	return S_OK;
}


HRESULT __stdcall CArchiveUpdateCallback::CryptoGetTextPassword2 (int *passwordIsDefined, BSTR *password)
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
