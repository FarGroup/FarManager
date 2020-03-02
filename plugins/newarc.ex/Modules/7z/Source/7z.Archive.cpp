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

	m_bSolid = false;
	m_bMultiVolume = false;
}

struct PropertyToName {
	PROPID propId;
	unsigned int uName;
};

static PropertyToName PropertyToNameTable[] =
{
	{ kpidName, MArchivePropertyName },
	{ kpidExtension, MArchivePropertyExtension },
	{ kpidIsDir, MArchivePropertyIsFolder },
	{ kpidSize, MArchivePropertySize },
	{ kpidPackSize, MArchivePropertyPackSize },
	{ kpidAttrib, MArchivePropertyAttributes },
	{ kpidCTime, MArchivePropertyCTime },
	{ kpidATime, MArchivePropertyATime },
	{ kpidMTime, MArchivePropertyMTime },
	{ kpidSolid, MArchivePropertySolid },
	{ kpidCommented, MArchivePropertyCommented },
	{ kpidEncrypted, MArchivePropertyEncrypted },
	{ kpidSplitBefore, MArchivePropertySplitBefore },
	{ kpidSplitAfter, MArchivePropertySplitAfter },
	{ kpidDictionarySize, MArchivePropertyDictionarySize },
	{ kpidCRC, MArchivePropertyCRC },
	{ kpidType, MArchivePropertyType },
	{ kpidIsAnti, MArchivePropertyAnti },
	{ kpidMethod, MArchivePropertyMethod },
	{ kpidHostOS, MArchivePropertyHostOS },
	{ kpidFileSystem, MArchivePropertyFileSystem },
	{ kpidUser, MArchivePropertyUser },
	{ kpidGroup, MArchivePropertyGroup },
	{ kpidBlock, MArchivePropertyBlock },
	{ kpidComment, MArchivePropertyComment },
	{ kpidPosition, MArchivePropertyPosition },
	{ kpidNumSubDirs, MArchivePropertyNumSubFolders },
	{ kpidNumSubFiles, MArchivePropertyNumSubFiles },
	{ kpidUnpackVer, MArchivePropertyUnpackVer },
	{ kpidVolume, MArchivePropertyVolume },
	{ kpidIsVolume, MArchivePropertyIsVolume },
	{ kpidOffset, MArchivePropertyOffset },
	{ kpidLinks, MArchivePropertyLinks },
	{ kpidNumBlocks, MArchivePropertyNumBlocks },
	{ kpidNumVolumes, MArchivePropertyNumVolumes },
  
	{ kpidBit64, MArchivePropertyBit64 },
	{ kpidBigEndian, MArchivePropertyBigEndian },
	{ kpidCpu, MArchivePropertyCpu },
	{ kpidPhySize, MArchivePropertyPhySize },
	{ kpidHeadersSize, MArchivePropertyHeadersSize },
	{ kpidChecksum, MArchivePropertyChecksum },
	{ kpidCharacts, MArchivePropertyCharacts },
	{ kpidVa, MArchivePropertyVa },
	{ kpidId, MArchivePropertyId },
	{ kpidShortName, MArchivePropertyShortName },
	{ kpidCreatorApp, MArchivePropertyCreatorApp },
	{ kpidSectorSize, MArchivePropertySectorSize },
	{ kpidPosixAttrib, MArchivePropertyPosixAttrib },
	{ kpidLink, MArchivePropertyLink },

	{ kpidTotalSize, MArchivePropertyTotalSize },
	{ kpidFreeSpace, MArchivePropertyFreeSpace },
	{ kpidClusterSize, MArchivePropertyClusterSize },
	{ kpidVolumeName, MArchivePropertyLabel }
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
					strName = _M(PropertyToNameTable[j].uName);
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
						strValue = (value.boolVal == VARIANT_TRUE)?_M(MYes):_M(MNo);
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

					if ( propId == kpidSolid ) 
						m_bSolid = (value.boolVal == VARIANT_TRUE);

					if ( propId == kpidIsVolume )
						m_bMultiVolume  = (value.boolVal == VARIANT_TRUE);
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
			//CPropVariant value;

			//if ( m_pArchive->GetArchiveProperty(kpidNumVolumes, &value) == S_OK )
			//	m_nNumberOfVolumes = (int)value.uintVal;

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

bool SevenZipArchive::IsSolid()
{
	return m_bSolid;
}

const GUID& SevenZipArchive::GetUID()
{
	return m_uid;
}

const TCHAR* SevenZipArchive::GetFileName()
{
	return m_strFileName;
}

int SevenZipArchive::GetNumberOfVolumes()
{
	return m_nNumberOfVolumes;
}

CInFile* SevenZipArchive::GetFile()
{
	return m_pFile;
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


bool SevenZipArchive::GetArchiveItem(unsigned int uIndex, ArchiveItem* pItem)
{
	CPropVariant value;

	if ( m_pArchive->GetProperty(uIndex, kpidPath, &value) == S_OK )
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

		if ( m_pArchive->GetProperty(uIndex, kpidAttrib, &value) == S_OK )
		{
			if ( value.vt == VT_UI4 )
				pItem->dwFileAttributes = value.ulVal;
		}

		if ( m_pArchive->GetProperty(uIndex, kpidIsDir, &value) == S_OK )
		{
			if ( value.vt == VT_BOOL )
			{
				if ( value.boolVal == VARIANT_TRUE )
					pItem->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
			}
		}

		if ( m_pArchive->GetProperty(uIndex, kpidSize, &value) == S_OK )
		{
			unsigned __int64 size = VariantToInt64(&value);
			pItem->nFileSize = size;
		}

		if ( m_pArchive->GetProperty(uIndex, kpidPackSize, &value) == S_OK )
		{
			unsigned __int64 size = VariantToInt64 (&value);
			pItem->nPackSize = size;
		}

		if ( m_pArchive->GetProperty(uIndex, kpidCRC, &value) == S_OK )
		{
			if ( value.vt == VT_UI4 )
				pItem->dwCRC32 = value.ulVal;
		}

		if ( m_pArchive->GetProperty(uIndex, kpidCTime, &value) == S_OK )
		{
			if ( value.vt == VT_FILETIME )
				memcpy (&pItem->ftCreationTime, &value.filetime, sizeof (FILETIME));
		}

		if ( m_pArchive->GetProperty(uIndex, kpidATime, &value) == S_OK )
		{
			if ( value.vt == VT_FILETIME )
				memcpy (&pItem->ftLastAccessTime, &value.filetime, sizeof (FILETIME));
		}

		if ( m_pArchive->GetProperty(uIndex, kpidMTime, &value) == S_OK )
		{
			if ( value.vt == VT_FILETIME )
				memcpy (&pItem->ftLastWriteTime, &value.filetime, sizeof (FILETIME));
		}

		pItem->lpFileName = StrDuplicate(strFileName);
		pItem->lpAlternateFileName = NULL;
		pItem->UserData = uIndex;

		return true;
	}

	return false;
}

int SevenZipArchive::GetArchiveItem(ArchiveItem* pItem)
{
	if ( m_uItemsCount == 0 )
		return E_EOF;

	int nResult = E_BROKEN;

	if ( GetArchiveItem(m_uItemsCount-1, pItem) )
		nResult = E_SUCCESS;

	m_uItemsCount--;

	return nResult;
}

bool SevenZipArchive::FreeArchiveItem(ArchiveItem *pItem)
{
	StrFree((void*)pItem->lpFileName);
	StrFree((void*)pItem->lpAlternateFileName);

	return true;
}

void CreateTempName(const TCHAR *lpArchiveName, string& strResultName) 
{
	int i = 0;

	do {
		strResultName.Format(_T("%s.%d"), lpArchiveName, i++);
	} while ( GetFileAttributes(strResultName) != INVALID_FILE_ATTRIBUTES );
}


int SevenZipArchive::Delete(const ArchiveItem* pItems, int nItemsNumber)
{
	IOutArchive *outArchive;

	int nResult = RESULT_ERROR;

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
				if ( i == (pItems[j].UserData) )
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

		CArchiveDeleteCallback Callback(this, indicies);

		COutFile* pFile = new COutFile(strTempName);

		if ( pFile )
		{
			if ( pFile->Open () )
			{
				if ( outArchive->UpdateItems(
						(ISequentialOutStream*)pFile,
						indicies.count(),
						&Callback
						) == S_OK )
					nResult = Callback.GetResult();
			}

			delete pFile;
		}

		outArchive->Release();

		if ( nResult == RESULT_SUCCESS ) 
		{
			Close();
			MoveFileEx(strTempName, m_strFileName, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING);
		}
		else
			DeleteFile(strTempName);

	}

	return nResult;
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

int SevenZipArchive::Test(
		const ArchiveItem* pItems,
		int nItemsNumber
		)
{
	return RESULT_ERROR;
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

	for (int i = 0; i < nItemsNumber; i++)
		indices[i] = (unsigned int)pItems[i].UserData;

	FSF.qsort(indices, nItemsNumber, sizeof(unsigned int), CompareIndicies);

	CArchiveExtractCallback Callback(this, pItems, nItemsNumber, lpDestDiskPath, lpPathInArchive);

	if ( m_pArchive->Extract(
			indices,
			(unsigned int)nItemsNumber,
			0,
			&Callback
			) == S_OK )
		nResult = Callback.GetResult();

	delete [] indices;

	return nResult;
}

//#include "7z.Properties.cpp"

int SevenZipArchive::AddFiles(
		const ArchiveItem* pItems,
		int nItemsNumber,
		const TCHAR* lpSourceDiskPath,
		const TCHAR* lpPathInArchive,
		const TCHAR* lpConfig
		)
{
	IOutArchive *outArchive;
	int nResult = RESULT_ERROR;

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

				indicies.add(item);
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

		if ( outArchive->QueryInterface(
				IID_ISetProperties,
				(void**)&setProperties
				) == S_OK )
		{
			const CompressionFormatInfo* pFormat = GetCompressionFormatInfo(m_uid);

			if ( pFormat )
			{
				SevenZipCompressionConfig* pCfg = SevenZipCompressionConfig::FromString(pFormat, lpConfig);

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
			memset(szPassword, 0, sizeof(szPassword));

			OnPasswordOperation(PASSWORD_COMPRESSION, szPassword, 512);

			COutFile* pFile = new COutFile(strTempName);

			if ( pFile )
			{
				CArchiveUpdateCallback Callback(this, szPassword, indicies, lpSourceDiskPath, lpPathInArchive);

				if ( pFile->Open () )
				{
					if ( outArchive->UpdateItems (
							(ISequentialOutStream*)pFile,
							indicies.count(),
							&Callback
							) == S_OK )
						nResult = Callback.GetResult();
				}

				pFile->Release();
			}
		}

		outArchive->Release(); //ниже не переносить!!

		if ( (nResult == RESULT_SUCCESS) || (nResult == RESULT_PARTIAL) )
		{
			Close();
			MoveFileEx(strTempName, m_strFileName, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING);
		}
		else
			DeleteFile(strTempName);

	}

	return nResult;
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

	if ( !m_bSolid && !(nOperation == OPERATION_DELETE) )
		SO.dwFlags |= OS_FLAG_SUPPORT_SINGLE_FILE_PROGRESS;

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


LONG_PTR SevenZipArchive::OnProcessData(
		unsigned __int64 uProcessedBytesFile,
		unsigned __int64 uTotalBytesFile,
		unsigned __int64 uProcessedBytesTotal,
		unsigned __int64 uTotalBytes
		)
{
	ProcessDataStruct DS;

	DS.nMode = PROGRESS_DETAILS;

	DS.uProcessedBytesFile = uProcessedBytesFile;
	DS.uTotalBytesFile = uTotalBytesFile;
	DS.uProcessedBytesTotal = uProcessedBytesTotal;
	DS.uTotalBytes = uTotalBytes;

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

LONG_PTR SevenZipArchive::OnReportError(const ArchiveItem* pItem, int nError)
{
	ReportErrorStruct RE;

	RE.pItem = pItem;
	RE.nError = nError;

	return Callback(AM_REPORT_ERROR, 0, (LONG_PTR)&RE);
}

LONG_PTR SevenZipArchive::OnEnterStage(int nStage)
{
	return Callback(AM_ENTER_STAGE, nStage, (LONG_PTR)nullptr);
}

LONG_PTR SevenZipArchive::OnNeedVolume(const TCHAR* lpSuggestedName, DWORD dwBufferSize, TCHAR* lpBuffer)
{
	VolumeStruct VS;

	VS.lpSuggestedName = lpSuggestedName;
	VS.dwBufferSize = dwBufferSize;
	VS.lpBuffer = lpBuffer;

	return Callback(AM_NEED_VOLUME, 0, (LONG_PTR)&VS);
}

int SevenZipArchive::GetArchiveInfo(bool& bMultiVolume, const ArchiveInfoItem** pItems)
{
	bMultiVolume = m_bMultiVolume;

	if ( m_pArchiveInfo.count() )
	{
		*pItems = m_pArchiveInfo.data();
		return m_pArchiveInfo.count();
	}

	return 0;
}
