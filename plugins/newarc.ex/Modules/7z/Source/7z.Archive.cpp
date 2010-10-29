#include "7z.h"

SevenZipArchive::SevenZipArchive(
		SevenZipPlugin *pPlugin,
		const GUID& uid,
		const TCHAR *lpFileName,
		HANDLE hCallback,
		ARCHIVECALLBACK pfnCallback,
		bool bCreated
		)
{
	m_pArchive = NULL;
	m_pPlugin = pPlugin;
	m_strFileName = lpFileName;
	m_uid = uid;

	m_uItemsCount = 0;

	m_pfnCallback = pfnCallback;
	m_hCallback = hCallback;

	m_bCreated = bCreated;
	m_bOpened = false;

	m_pFile = NULL;
}

struct PropertyToName {
	PROPID propId;
	const TCHAR* lpName;
};

static PropertyToName PropertyToNameTable[] =
{
	{ kpidName, _T("Name") },
	{ kpidExtension, _T("Extension") },
	{ kpidIsDir, _T("IsFolder") },
	{ kpidSize, _T("Size") },
	{ kpidPackSize, _T("PackSize") },
	{ kpidAttrib, _T("Attributes") },
	{ kpidCTime, _T("CTime") },
	{ kpidATime, _T("ATime") },
	{ kpidMTime, _T("MTime") },
	{ kpidSolid, _T("Solid") },
	{ kpidCommented, _T("Commented") },
	{ kpidEncrypted, _T("Encrypted") },
	{ kpidSplitBefore, _T("SplitBefore") },
	{ kpidSplitAfter, _T("SplitAfter") },
	{ kpidDictionarySize, _T("DictionarySize") },
	{ kpidCRC, _T("CRC") },
	{ kpidType, _T("Type") },
	{ kpidIsAnti, _T("Anti") },
	{ kpidMethod, _T("Method") },
	{ kpidHostOS, _T("HostOS") },
	{ kpidFileSystem, _T("FileSystem") },
	{ kpidUser, _T("User") },
	{ kpidGroup, _T("Group") },
	{ kpidBlock, _T("Block") },
	{ kpidComment, _T("Comment") },
	{ kpidPosition, _T("Position") },
	{ kpidNumSubDirs, _T("NumSubFolders") },
	{ kpidNumSubFiles, _T("NumSubFiles") },
	{ kpidUnpackVer, _T("UnpackVer") },
	{ kpidVolume, _T("Volume") },
	{ kpidIsVolume, _T("IsVolume") },
	{ kpidOffset, _T("Offset") },
	{ kpidLinks, _T("Links") },
	{ kpidNumBlocks, _T("NumBlocks") },
	{ kpidNumVolumes, _T("NumVolumes") },
  
	{ kpidBit64, _T("Bit64") },
	{ kpidBigEndian, _T("BigEndian") },
	{ kpidCpu, _T("Cpu") },
	{ kpidPhySize, _T("PhySize") },
	{ kpidHeadersSize, _T("HeadersSize") },
	{ kpidChecksum, _T("Checksum") },
	{ kpidCharacts, _T("Characts") },
	{ kpidVa, _T("Va") },
	{ kpidId, _T("Id") },
	{ kpidShortName, _T("ShortName") },
	{ kpidCreatorApp, _T("CreatorApp") },
	{ kpidSectorSize, _T("SectorSize") },
	{ kpidPosixAttrib, _T("PosixAttrib") },
	{ kpidLink, _T("Link") },

	{ kpidTotalSize, _T("TotalSize") },
	{ kpidFreeSpace, _T("FreeSpace") },
	{ kpidClusterSize, _T("ClusterSize") },
	{ kpidVolumeName, _T("Label") }
};


void SevenZipArchive::QueryArchiveInfo()
{
	m_pArchiveInfo.reset();

	unsigned int uProperties;

	m_pArchive->GetNumberOfArchiveProperties(&uProperties);

	PROPID propId;
	BSTR bstrName;
	VARTYPE vt;

	for (unsigned int i = 0; i < uProperties; i++)
	{
		if ( m_pArchive->GetArchivePropertyInfo(i, &bstrName, &propId, &vt) == S_OK )
		{
			string strName;

			for (unsigned int j = 0; j < sizeof(PropertyToNameTable)/sizeof(PropertyToNameTable[0]); j++)
			{
				if ( PropertyToNameTable[j].propId == propId )
				{
					strName = PropertyToNameTable[j].lpName;
					break;
				}
			}

			if ( !strName.IsEmpty() )
			{
				CPropVariant value;

				if ( m_pArchive->GetArchiveProperty(propId, &value) == S_OK && (value.vt != VT_EMPTY) )
				{
					string strValue;

					if ( vt == VT_UI8 )
						strValue.Format(_T("%d"), value.bVal);
					else

					if ( vt == VT_UI4 )
						strValue.Format(_T("%d"), value.uintVal);
					else
		
					if ( vt == VT_BSTR )
#ifdef UNICODE
						strValue = value.bstrVal;
#else
						strValue.SetData(value.bstrVal);
#endif
					else

					if ( vt == VT_BOOL )
						strValue = (value.boolVal == VARIANT_TRUE)?_T("Yes"):_T("No");
					else
				
					if ( vt == VT_FILETIME )
					{
						SYSTEMTIME sTime;
						FileTimeToSystemTime(&value.filetime, &sTime);

						strValue.Format(_T("%02d/%02d/%4d"), sTime.wDay, sTime.wMonth, sTime.wYear);
					}


					ArchiveInfoItem* item;
			
					item = m_pArchiveInfo.add();
					item->lpValue = StrDuplicate(strValue);
					item->lpName = StrDuplicate(strName);
				}
			}

			SysFreeString(bstrName);
		}
	}
}


bool SevenZipArchive::Open()
{
	if ( m_bOpened )
		return true;

	if ( m_pPlugin->CreateObject(m_uid, IID_IInArchive, (void**)&m_pArchive) == S_OK )
	{
		m_pFile = new CInFile(m_strFileName);

		if ( m_pFile->Open() )
		{
			unsigned __int64 max = Info.AdvControl(Info.ModuleNumber, ACTL_GETPLUGINMAXREADDATA, 0);
			//unsigned __int64 max = 1 << 16;

			CArchiveOpenCallback Callback(this);

			if ( m_pArchive->Open(m_pFile, &max, &Callback) == S_OK )
			{
				m_bOpened = true;

				QueryArchiveInfo();

				return true;
			}

			m_pFile->Close();
		}

		m_pFile->Release();
		m_pArchive->Release();
	} 

	return false;
}

void SevenZipArchive::Close()
{
	if ( m_bOpened )
	{
		m_pArchive->Release(); 
		m_pFile->Release();

		m_pArchive = NULL;
		m_pFile = NULL;

		m_bOpened = false;
	}
}


const GUID& SevenZipArchive::GetUID()
{
	return m_uid;
}

const TCHAR* SevenZipArchive::GetFileName()
{
	return m_strFileName;
}

IInArchive* SevenZipArchive::GetArchive()
{
	return m_pArchive;
}

SevenZipArchive::~SevenZipArchive()
{
	for (unsigned int i = 0; i < m_pArchiveInfo.count(); i++)
	{
		StrFree((void*)m_pArchiveInfo[i].lpName);
		StrFree((void*)m_pArchiveInfo[i].lpValue);
	}

	Close();
}

bool SevenZipArchive::StartOperation(int nOperation, bool bInternal)
{
	if ( !bInternal )
		Close();
	else
	{
		if ( !m_bCreated )
		{
			if ( Open() )
				return m_pArchive->GetNumberOfItems(&m_uItemsCount) == S_OK;
			else
				return false;
		}
	}

	return true;
}

bool SevenZipArchive::EndOperation(int nOperation, bool bInternal)
{
	return true;
}


unsigned __int64 VariantToInt64 (CPropVariant *value)
{
	switch ( value->vt )
	{
		case VT_UI1:
			return value->bVal;
		case VT_UI2:
			return value->uiVal;
		case VT_UI4:
			return value->ulVal;
		case VT_UI8:
			return (unsigned __int64)value->uhVal.QuadPart;
		default:
			return 0;
	}
}


int SevenZipArchive::GetArchiveItem(ArchiveItem* pItem)
{
	if ( m_uItemsCount == 0 )
		return E_EOF;

	int nResult = E_BROKEN;

	CPropVariant value;

	int nIndex = m_uItemsCount-1;

	if ( m_pArchive->GetProperty(nIndex, kpidPath, &value) == S_OK )
	{
		string strFileName;

		if ( value.vt == VT_BSTR )
		{
#ifdef UNICODE
			strFileName = value.bstrVal;
#else
			strFileName.SetData(value.bstrVal, CP_OEMCP);
#endif
		}
		else
		{
			strFileName = FSF.PointToName(m_strFileName); //file inside TAR
			CutTo(strFileName, _T('.'), true);
		}

		if ( m_pArchive->GetProperty(nIndex, kpidAttrib, &value) == S_OK )
		{
			if ( value.vt == VT_UI4 )
				pItem->dwFileAttributes = value.ulVal;
		}

		if ( m_pArchive->GetProperty(nIndex, kpidIsDir, &value) == S_OK )
		{
			if ( value.vt == VT_BOOL )
			{
				if ( value.boolVal == VARIANT_TRUE )
					pItem->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
			}
		}

		if ( m_pArchive->GetProperty(nIndex, kpidSize, &value) == S_OK )
		{
			unsigned __int64 size = VariantToInt64(&value);
			pItem->nFileSize = size;
		}

		if ( m_pArchive->GetProperty(nIndex, kpidPackSize, &value) == S_OK )
		{
			unsigned __int64 size = VariantToInt64 (&value);
			pItem->nPackSize = size;
		}

		if ( m_pArchive->GetProperty(nIndex, kpidCRC, &value) == S_OK )
		{
			if ( value.vt == VT_UI4 )
				pItem->dwCRC32 = value.ulVal;
		}

		if ( m_pArchive->GetProperty(nIndex, kpidCTime, &value) == S_OK )
		{
			if ( value.vt == VT_FILETIME )
				memcpy (&pItem->ftCreationTime, &value.filetime, sizeof (FILETIME));
		}

		if ( m_pArchive->GetProperty(nIndex, kpidATime, &value) == S_OK )
		{
			if ( value.vt == VT_FILETIME )
				memcpy (&pItem->ftLastAccessTime, &value.filetime, sizeof (FILETIME));
		}

		if ( m_pArchive->GetProperty(nIndex, kpidMTime, &value) == S_OK )
		{
			if ( value.vt == VT_FILETIME )
				memcpy (&pItem->ftLastWriteTime, &value.filetime, sizeof (FILETIME));
		}

		pItem->lpFileName = StrDuplicate(strFileName);
		pItem->lpAlternateFileName = NULL;
		pItem->UserData = nIndex+1; //to skip 0

		nResult = E_SUCCESS;
	}

	m_uItemsCount--;

	return nResult;
}

bool SevenZipArchive::FreeArchiveItem(ArchiveItem *pItem)
{
	StrFree((void*)pItem->lpFileName);
	StrFree((void*)pItem->lpAlternateFileName);

	return true;
}

void CreateTempName (const TCHAR *lpArchiveName, string& strResultName) 
{
	int i = 0;

	do {
		strResultName.Format(_T("%s.%d"), lpArchiveName, i++);
	} while ( GetFileAttributes(strResultName) != INVALID_FILE_ATTRIBUTES );
}


bool SevenZipArchive::Delete(const ArchiveItem* pItems, int nItemsNumber)
{
	IOutArchive *outArchive;

	bool bResult = false;

	if ( m_pArchive->QueryInterface(
			IID_IOutArchive,
			(void**)&outArchive
			) == S_OK )
	{
		ObjectArray<ArchiveUpdateItem*> indicies;

		for (unsigned int i = 0; i < m_uItemsCount; i++) //какая жесть
		{
			bool bFound = false;

			for (int j = 0; j < nItemsNumber; j++)
			{
				if ( i == (pItems[j].UserData-1) )
				{
					bFound = true;
					break;
				}
			}

			if ( !bFound )
			{
				ArchiveUpdateItem *item = new ArchiveUpdateItem;

				item->index = i;
				item->bNewFile = false;

				indicies.add (item);
			}
		}

		string strTempName;
		CreateTempName (m_strFileName, strTempName);

		TCHAR szPassword[512];
		memset(szPassword, 0, 512);

		OnPasswordOperation(PASSWORD_LIST, szPassword, 512);
		CArchiveUpdateCallback Callback(this, szPassword, &indicies, NULL, NULL);

		COutFile* pFile = new COutFile(strTempName);

		if ( pFile )
		{
			if ( pFile->Open () )
			{
				if ( outArchive->UpdateItems (
						(ISequentialOutStream*)pFile,
						indicies.count(),
						&Callback
						) == S_OK )
					bResult = true;
			}

			delete pFile;
		}

		outArchive->Release ();

		if ( bResult )
		{
			Close();
			MoveFileEx(strTempName, m_strFileName, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING);
		}
		else
			DeleteFile(strTempName);

	}

	return bResult;
}



int __cdecl CompareIndicies(const void *p1, const void *p2)
{
	int i1 = *(int*)p1;
	int i2 = *(int*)p2;

	if ( i1 > i2 )
		return 1;

	if ( i1 < i2 )
		return -1;

	return 0;
}

bool SevenZipArchive::Test(
		const ArchiveItem* pItems,
		int nItemsNumber
		)
{
	return false;
}

int SevenZipArchive::Extract(
		const ArchiveItem* pItems,
		int nItemsNumber,
		const TCHAR* lpDestDiskPath,
		const TCHAR* lpPathInArchive
		)
{
	int nResult = RESULT_ERROR;

	unsigned int* indices = new unsigned int[nItemsNumber];
	ArchiveItemEx* items = new ArchiveItemEx[nItemsNumber];

	int lastitem = 0;

	for (int i = 0; i < nItemsNumber; i++)
	{
		if ( pItems[i].UserData )
		{
			indices[lastitem] = (unsigned int)pItems[i].UserData-1; //GetIndex (m_pArchive, pItems[i].FindData.cFileName);

			items[lastitem].nIndex = indices[lastitem];
			items[lastitem].pItem = &pItems[i];

			lastitem++;
		}
	}

	FSF.qsort(indices, lastitem, sizeof(unsigned int), CompareIndicies);

	CArchiveExtractCallback Callback(this, items, lastitem, lpDestDiskPath, lpPathInArchive);

	if ( m_pArchive->Extract(
			indices,
			(unsigned int)lastitem,
			0,
			&Callback
			) == S_OK )
		nResult = Callback.GetResult();

	delete [] indices;
	delete [] items;

	return nResult;
}

#include "7z.Properties.cpp"

bool SevenZipArchive::AddFiles(
		const ArchiveItem* pItems,
		int nItemsNumber,
		const TCHAR* lpSourceDiskPath,
		const TCHAR* lpPathInArchive
		)
{
	IOutArchive *outArchive;
	bool bResult = false;

	HRESULT hr;

	if ( m_bCreated )
		hr = m_pPlugin->CreateObject(m_uid, IID_IOutArchive, (void**)&outArchive);
	else
		hr = m_pArchive->QueryInterface(IID_IOutArchive, (void**)&outArchive);
			
	if ( hr == S_OK )
	{
		ObjectArray<ArchiveUpdateItem*> indicies;
		CPropVariant value;

		if ( !m_bCreated )
		{
			for (unsigned int i = 0; i < m_uItemsCount; i++)
			{
				ArchiveUpdateItem *item = new ArchiveUpdateItem;

				item->index = i;
				item->bNewFile = false;

				indicies.add (item);
			}
		}

		for (int i = 0; i < nItemsNumber; i++)
		{
			bool bFound = false;

			if ( !m_bCreated )
			{
				string strCheckName;

				if ( lpPathInArchive && *lpPathInArchive )
					strCheckName = lpPathInArchive; 

				strCheckName += pItems[i].lpFileName;

				for (unsigned int j = 0; j < m_uItemsCount; j++)
				{
					string strArchiveFileName;

					if ( m_pArchive->GetProperty(j, kpidPath, &value) == S_OK )
					{
						if ( value.vt == VT_BSTR )
						{
#ifdef UNICODE
							strArchiveFileName = value.bstrVal;
#else
							strArchiveFileName.SetData(value.bstrVal, CP_OEMCP);
#endif
						}
					}

					if ( !FSF.LStricmp(strArchiveFileName, strCheckName) ) //где-то тут бред
					{
						bFound = true;

						ArchiveUpdateItem *item = indicies.at(j);

						item->index = (unsigned int)-1;
						item->bNewFile = true;
						item->pItem = &pItems[i];
						break;
					}
				}
			}

			if ( !bFound )
			{
				ArchiveUpdateItem *item = new ArchiveUpdateItem;
				item->index = (unsigned int)-1;
				item->bNewFile = true;
				item->pItem = &pItems[i];
				indicies.add (item);
			}
		}

		bool bAddDialogOk = true;

		ISetProperties *setProperties;

		if ( outArchive->QueryInterface (
				IID_ISetProperties,
				(void**)&setProperties
				) == S_OK )
		{
			const CompressionFormatInfo* pFormat = GetCompressionFormatInfo(m_uid);

			if ( pFormat )
			{
				SevenZipCompressionConfig* pCfg = new SevenZipCompressionConfig;
				memset(pCfg, 0, sizeof(SevenZipCompressionConfig));

				pCfg->pFormat = pFormat;

				ObjectArray<SevenZipProperty*> properties;

				if ( dlgSevenZipPluginConfigure(pCfg) )
				{
					CompressionConfigToProperties(!_tcscmp(pFormat->lpName, _T("7z")), pCfg, properties);

					const wchar_t** pNames = new const wchar_t*[properties.count()];
					PROPVARIANT* pValues = new PROPVARIANT[properties.count()];

					for (unsigned int i = 0; i < properties.count(); i++)
					{
						pNames[i] = properties[i]->GetName();
						pValues[i] = properties[i]->GetValue();
					}

					HRESULT hr = setProperties->SetProperties(pNames, pValues, properties.count());

					delete [] pNames;
					delete [] pValues;

					if ( hr != S_OK )
						__debug(_T("%d"), hr);
				}
				else 
					bAddDialogOk = false;

				delete pCfg;
			}
					
			setProperties->Release ();  //oops
		}

		string strTempName;
		CreateTempName (m_strFileName, strTempName);

		if ( bAddDialogOk )
		{
			TCHAR szPassword[512];
			memset(szPassword, 0, 512);

			OnPasswordOperation(PASSWORD_COMPRESSION, szPassword, 512);

			COutFile* pFile = new COutFile(strTempName);

			if ( pFile )
			{
				CArchiveUpdateCallback Callback(this, szPassword, &indicies, lpSourceDiskPath, lpPathInArchive);

				if ( pFile->Open () )
				{
					if ( outArchive->UpdateItems (
							(ISequentialOutStream*)pFile,
							indicies.count(),
							&Callback
							) == S_OK )
						bResult = true;
				}

				pFile->Release();
			}
		}

		outArchive->Release(); //ниже не переносить!!

		if ( bResult )
		{
			Close();
			MoveFileEx(strTempName, m_strFileName, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING);
		}
		else
			DeleteFile(strTempName);

	}

	return bResult;
}

LONG_PTR SevenZipArchive::Callback(int nMsg, int nParam1, LONG_PTR nParam2)
{
	if ( m_pfnCallback )
		return m_pfnCallback(m_hCallback, nMsg, nParam1, nParam2);

	return FALSE;
}


LONG_PTR SevenZipArchive::OnStartOperation(
		int nOperation, 
		unsigned __int64 uTotalSize, 
		unsigned __int64 uTotalFiles
		)
{
	StartOperationStruct SO;
	
	SO.dwFlags = 0;
	
	if ( uTotalSize != 0 )
	{
		SO.dwFlags |= OS_FLAG_TOTALSIZE;
		SO.uTotalSize = uTotalSize;
	}

	if ( uTotalFiles != 0 )
	{
		SO.dwFlags |= OS_FLAG_TOTALFILES;
		SO.uTotalFiles = uTotalFiles;
	}

	return Callback(AM_START_OPERATION, nOperation, (LONG_PTR)&SO);
}

LONG_PTR SevenZipArchive::OnProcessFile(
		const ArchiveItem* pItem, 
		const TCHAR* lpDestName
		)
{
	ProcessFileStruct PF;

	PF.pItem = pItem;
	PF.lpDestFileName = lpDestName;

	return Callback(AM_PROCESS_FILE, 0, (LONG_PTR)&PF);
}


LONG_PTR SevenZipArchive::OnProcessData(unsigned __int64 uSize)
{
	ProcessDataStruct DS;

	DS.uProcessedSize = uSize;

	return Callback(AM_PROCESS_DATA, 0, (LONG_PTR)&DS);
}

LONG_PTR SevenZipArchive::OnPasswordOperation(
		int nType, 
		TCHAR* lpBuffer,
		DWORD dwBuferSize
		)
{
	PasswordStruct PS;

	PS.lpBuffer = lpBuffer;
	PS.dwBufferSize = dwBuferSize;

	return Callback(AM_NEED_PASSWORD, nType, (LONG_PTR)&PS);
}

int SevenZipArchive::GetArchiveInfo(const ArchiveInfoItem** pItems)
{
	if ( m_pArchiveInfo.count() )
	{
		*pItems = m_pArchiveInfo.data();
		return m_pArchiveInfo.count();
	}

	return 0;
}
