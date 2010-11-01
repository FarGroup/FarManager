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
}

void Archive::SetPassword(const TCHAR* lpPassword)
{
	m_strPassword = lpPassword;
}

void Archive::SetCurrentDirectory(const TCHAR* lpPathInArchive)
{
	m_strPathInArchive = lpPathInArchive;
	AddEndSlash(m_strPathInArchive);
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

bool Archive::ReadArchive(ArchiveItemArray& items)
{
	if ( StartOperation(OPERATION_LIST, true) )
	{
		int nResult = E_SUCCESS;

		while ( (nResult == E_SUCCESS) && !CheckForEsc() )
		{
			ArchiveItem* item = items.add();

			nResult = GetArchiveItem(item);

			if ( nResult != E_SUCCESS )
				items.remove();
		}

		EndOperation(OPERATION_LIST, true);
	}

	return true;
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
}

int Archive::Extract(
		const ArchiveItemArray& items,
		const TCHAR *lpDestDiskPath,
		bool bWithoutPath
		)
{
	bool bInternalFailed = true;
	int nResult = false;

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

			bInternalFailed = false;
		}
	}

	if ( bInternalFailed )
	{
		if ( StartOperation(OPERATION_EXTRACT, false) )
		{
			nResult = ExecuteCommand(
					items, 
					bWithoutPath?COMMAND_EXTRACT_WITHOUT_PATH:COMMAND_EXTRACT,
					strDestDiskPath
					)?RESULT_SUCCESS:RESULT_ERROR; //BADBAD

			EndOperation(OPERATION_EXTRACT, false);
		}
	}

	return nResult;
}

bool Archive::Test(const ArchiveItemArray& items)
{
	bool bResult = false;
	bool bInternalFailed = true;

	if ( QueryCapability(AFF_SUPPORT_INTERNAL_TEST) && m_hArchive )
	{
		if ( StartOperation(OPERATION_TEST, true) )
		{
			bResult = m_pModule->Test(m_hArchive, items);

			EndOperation(OPERATION_TEST, false);

			bInternalFailed = false;
		}
	}

	if ( bInternalFailed )
	{
		if ( StartOperation(OPERATION_TEST, false) )
		{
			bResult = ExecuteCommand(items, COMMAND_TEST);
			EndOperation(OPERATION_TEST, false);
		}
	}

	if ( !bResult )
		msgError(_T("Test error!"));

	return bResult;
}

bool Archive::AddFiles(
		const ArchiveItemArray& items,
		const TCHAR *lpSourceDiskPath
		)
{
	bool bResult = false;
	bool bInternalFailed = true;

	string strSourceDiskPath = lpSourceDiskPath;
	AddEndSlash(strSourceDiskPath);

	if ( QueryCapability(AFF_SUPPORT_INTERNAL_ADD) && m_hArchive )
	{
		if ( StartOperation(OPERATION_ADD, true) )
		{
			bResult = m_pModule->AddFiles(
					m_hArchive, 
					items, 
					strSourceDiskPath, 
					m_strPathInArchive
					);
		
			EndOperation(OPERATION_ADD, true);

			bInternalFailed = false;
		}
	}

	if ( bInternalFailed )
	{
		if ( StartOperation(OPERATION_ADD, false) )
		{
			bResult = ExecuteCommand(
					items, 
					COMMAND_ADD, 
					strSourceDiskPath
					);

			EndOperation(OPERATION_ADD, false);
		}
	}

	if ( !bResult )
		msgError(_T("Add error!"));
	
	return bResult;
}


bool Archive::MakeDirectory(const TCHAR* lpDirectory)
{
	bool bResult = false;
	bool bInternalFailed = true;

	ArchiveItemArray items;

	ArchiveItem *item = items.add();

	item->lpFileName = lpDirectory;
	item->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;

	if ( QueryCapability(AFF_SUPPORT_INTERNAL_ADD) && m_hArchive )
	{
		if ( StartOperation(OPERATION_ADD, true) )
		{
			item->lpFileName = lpDirectory;

			bResult = m_pModule->AddFiles(
					m_hArchive, 
					items, 
					_T(""), 
					m_strPathInArchive
					);
		
			EndOperation(OPERATION_ADD, true);

			bInternalFailed = false;
		}
	}

	if ( bInternalFailed )
	{
		if ( StartOperation(OPERATION_ADD, false) )
		{
			string strTempPath;

			TCHAR* lpTempPath = strTempPath.GetBuffer(260);
#ifdef UNICODE
			FSF.MkTemp(lpTempPath, 260, _T("NADT"));
#else
			FSF.MkTemp(lpTempPath, _T("NADT"));
#endif
			strTempPath.ReleaseBuffer();
			
			string strFullTempPath = strTempPath;
			AddEndSlash(strFullTempPath);

			strFullTempPath += lpDirectory;

			apiCreateDirectoryEx(strFullTempPath);

			bResult = ExecuteCommand(items, COMMAND_ADD, strTempPath);

			RemoveDirectory(strTempPath);

			EndOperation(OPERATION_ADD, false);
		}
	}

	if ( !bResult )
		msgError(_T("Make directory error!"));

	return bResult;
}

bool Archive::Delete(const ArchiveItemArray& items)
{
	bool bResult = false;
	bool bInternalFailed = true;

	if ( QueryCapability(AFF_SUPPORT_INTERNAL_DELETE) && m_hArchive )
	{
		if ( StartOperation(OPERATION_DELETE, true) )
		{
			bResult = m_pModule->Delete(
					m_hArchive,
					items
					);

			EndOperation(OPERATION_DELETE, true);

			bInternalFailed = false;
		}
	}

	if ( bInternalFailed )
	{
		if ( StartOperation(OPERATION_DELETE, false) )
		{
			bResult = ExecuteCommand(items, COMMAND_DELETE);
			EndOperation(OPERATION_DELETE, false);
		}
	}

	if ( !bResult )
		msgError(_T("Delete error!"));

	return bResult;
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

int Archive::GetArchiveInfo(const ArchiveInfoItem** pItems)
{
	return m_pModule->GetArchiveInfo(m_hArchive, pItems);
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
		string &strCommand,
		bool& bEnabledByDefault
		)
{
	return m_pModule->GetDefaultCommand(GetPlugin()->GetUID(), m_uid, nCommand, strCommand, bEnabledByDefault);
}


#include "processname.cpp"

void QuoteSpaceOnly(string& strSrc)
{
	TCHAR* lpBuffer = strSrc.GetBuffer(strSrc.GetLength()+5);

	FSF.QuoteSpaceOnly(lpBuffer);

	strSrc.ReleaseBuffer();
}


bool Archive::ExecuteCommand(
		const ArchiveItemArray& items,
		int nCommand,
		const TCHAR* lpCurrentDiskPath 
		)
{
	bool bResult = false;

	string strCommand;
	bool bEnabledByDefault;

	if ( GetDefaultCommand(nCommand, strCommand, bEnabledByDefault) && !strCommand.IsEmpty() && bEnabledByDefault )
	{
		ParamStruct psParam;
		FarPanelInfo info;
		
		TCHAR* lpTempPath = psParam.strTempPath.GetBuffer(260);
		GetTempPath (260, lpTempPath);
		psParam.strTempPath.ReleaseBuffer();

		TCHAR* lpListFileName = psParam.strListFileName.GetBuffer(260);
#ifdef UNICODE
		FSF.MkTemp (lpListFileName, 260, _T("NALT"));
#else
		FSF.MkTemp (lpListFileName, _T("NALT"));
#endif
		psParam.strListFileName.ReleaseBuffer();

		string strFileName = m_strFileName;
		string strPath;
		
		if ( lpCurrentDiskPath )
			strPath = lpCurrentDiskPath;
		else
		{
			strPath = strFileName;
			CutToSlash(strPath);
		}

		QuoteSpaceOnly(psParam.strTempPath);
		QuoteSpaceOnly(psParam.strListFileName);
		QuoteSpaceOnly(strFileName);
		
		psParam.strArchiveName = strFileName;
		psParam.strShortArchiveName = strFileName;
		psParam.strPassword = m_strPassword;
		psParam.strPathInArchive = m_strPathInArchive;
//		psParam.strAdditionalCommandLine = m_strAdditionalCommandLine;
		
		string strExecuteString;
		int nStartItemNumber = 0;

		while ( true )
		{
			int nResult = ParseString (
					items,
					strCommand,
					strExecuteString,
					&psParam,
					nStartItemNumber
					);

			if ( (nResult == PE_SUCCESS) || (nResult == PE_MORE_FILES) )
			{
				PROCESS_INFORMATION pInfo;
				STARTUPINFO sInfo;

				memset (&sInfo, 0, sizeof (STARTUPINFO));
				sInfo.cb = sizeof (STARTUPINFO);

				apiExpandEnvironmentStrings(strExecuteString, strExecuteString);

				HANDLE hScreen = Info.SaveScreen(0, 0, -1, -1);

				if ( CreateProcess (
						NULL,
						strExecuteString.GetBuffer(),
						NULL,
						NULL,
						TRUE,
						0,
						NULL,
						strPath, 
						&sInfo,
						&pInfo
						) )
				{
					WaitForSingleObject(pInfo.hProcess, INFINITE);

					DWORD dwExitCode;
					GetExitCodeProcess(pInfo.hProcess, &dwExitCode);

					CloseHandle (pInfo.hProcess);
					CloseHandle (pInfo.hThread);

					bResult = (dwExitCode == 0);
				}
				else
				{
					string strError;
					strError.Format(_T("CreateProcess failed - %d\n%s"), GetLastError(), strExecuteString.GetString());
					msgError(strError);
				}

				Info.RestoreScreen(NULL);
				Info.RestoreScreen(hScreen);
			}

			if ( nResult != PE_MORE_FILES )
				break;
		}

		DeleteFile (psParam.strListFileName); //WARNING!!!
	}

	return bResult;
}

