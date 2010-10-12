#include "newarc.h"

bool IsFileInFolder(const TCHAR *lpCurrentPath, const TCHAR *lpFileName)
{
	int nLength = StrLength(lpCurrentPath);

	bool bResult = nLength && !_tcsncmp(lpCurrentPath, lpFileName, nLength); //ў®Їа®б, ­г¦­® «Ё §¤Ґбм в®¦Ґ ЁЈ­®аЁа®ў вм аҐЈЁбва
	
	return bResult && ((
			(lpFileName[nLength] == 0) || 
			(lpFileName[nLength] == '/') || 
			(lpFileName[nLength] == '\\')
			) || (
			(lpCurrentPath[nLength-1] == '\\') ||
			(lpCurrentPath[nLength-1] == '/')
			));
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


extern string strIniFileName;

ArchivePanel::ArchivePanel(
		ArchiveModuleManager* pManager,
		const TCHAR* lpFileName
		)
{
	m_strFileName = lpFileName;
	m_pManager = pManager;

	m_pArchive = NULL;
	m_bFirstTime = true;

	m_pArchiveInfo = NULL;
	m_nArchiveInfoItems = 0;

	m_bPasswordSet = false;
}

Array<ArchiveFormat*>& ArchivePanel::GetFormats()
{
	return m_pFormats;
}

ArchivePanel::~ArchivePanel ()
{
}


int ArchivePanel::pGetFindData(
		PluginPanelItem **pPanelItem,
		int* pItemsNumber,
		int OpMode
		)
{
	bool bSilent = OpMode & (OPM_SILENT|OPM_FIND);

	if ( m_bFirstTime )
	{
		if ( m_pFormats.count() )
		{
			int nResult = 0;

			if ( m_pFormats.count() > 1 && !bSilent)
			{
				FarMenu menu(_M(MOpenArchiveAs));

				string strText;

				for (unsigned int i = 0; i < m_pFormats.count(); i++)
				{
					const ArchiveFormat* pFormat = m_pFormats[i];

					strText.Format(_T("%s"), pFormat->GetName());
					menu.Add(strText);
				}

				nResult = menu.Run();
			}

			if ( nResult != -1 )
				m_pArchive = m_pManager->OpenCreateArchive(m_pFormats[nResult], m_strFileName, this, Callback, false);

			if ( nResult == -1 )
				return FALSE;
		}

		m_bFirstTime = false;
	}

#pragma message("check if pArchive exists!!")

	if ( m_pArchive->WasUpdated() )
	{
		for (unsigned int i = 0; i < m_pArchiveFiles.count(); i++)
			m_pArchive->FreeArchiveItem(&m_pArchiveFiles[i]);

		m_pArchiveFiles.reset();

		if ( !m_pArchive->ReadArchive(m_pArchiveFiles) )
			return FALSE;
	}

	const ArchiveInfoItem* pInfoItems;

	m_nArchiveInfoItems = m_pArchive->GetArchiveInfo(&pInfoItems);

	if ( m_nArchiveInfoItems )
	{
		m_pArchiveInfo = new InfoPanelLine[m_nArchiveInfoItems];
		memset(m_pArchiveInfo, 0, sizeof(InfoPanelLine)*m_nArchiveInfoItems);

		for (int i = 0; i < m_nArchiveInfoItems; i++)
		{
#ifdef UNICODE
			m_pArchiveInfo[i].Text = StrDuplicate(pInfoItems[i].lpName);
			m_pArchiveInfo[i].Data = StrDuplicate(pInfoItems[i].lpValue);
#else
			strcpy(m_pArchiveInfo[i].Text, pInfoItems[i].lpName);
			strcpy(m_pArchiveInfo[i].Data, pInfoItems[i].lpValue);
#endif
		}
	}

	ConstArray<PluginPanelItem> pPanelItems(100);

	bool bIsFolder;

	for (unsigned int i = 0; i < m_pArchiveFiles.count(); i++)
	{
		ArchiveItem* pItem = &m_pArchiveFiles[i];

		bIsFolder = false;

		bool bFileInFolder = IsFileInFolder(m_strPathInArchive, pItem->lpFileName);

		if ( m_strPathInArchive.IsEmpty() || bFileInFolder ) 
		{
			const TCHAR* lpRealName = pItem->lpFileName+m_strPathInArchive.GetLength();

			if ( bFileInFolder )
				lpRealName++;

			if ( !*lpRealName )
				continue;

			TCHAR* lpFileName = StrDuplicate(lpRealName);

			TCHAR* p = _tcschr(lpFileName, _T('\\'));

			if ( p )
			{
				bIsFolder = true;
				*p = 0;
			}

			bool bSkip = false;

			for (unsigned int j = 0; j < pPanelItems.count(); j++)
			{
				PluginPanelItem* item = &pPanelItems[j];

				if ( (item->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
				{
#ifdef UNICODE
					if ( IsFileInFolder(item->FindData.lpwszFileName, lpFileName) )
					//if ( !_tcscmp(item->FindData.lpwszFileName, lpFileName) )
#else
					if ( IsFileInFolder(item->FindData.cFileName, lpFileName) )
					//if ( !_tcscmp(item->FindData.cFileName, lpFileName) )
#endif
					{
						bSkip = true;
						break;
					}
				}
			}

			if ( bSkip )
				continue;

			PluginPanelItem *item = pPanelItems.add();

			//CHECK!!!
			item->FindData.dwFileAttributes = pItem->dwFileAttributes;

#ifdef UNICODE
			item->FindData.nFileSize = pItem->nFileSize;
			item->FindData.nPackSize = pItem->nPackSize;
#else
			item->FindData.nFileSizeHigh = (DWORD)(pItem->nFileSize >> 32);
			item->FindData.nFileSizeLow = (DWORD)(pItem->nFileSize);

			item->FindData.dwReserved0 = (DWORD)(pItem->nPackSize >> 32);
			item->FindData.dwReserved1 = (DWORD)(pItem->nPackSize);
#endif

			item->CRC32 = pItem->dwCRC32;
				
			memcpy(&item->FindData.ftCreationTime, &pItem->ftCreationTime, sizeof(FILETIME));
			memcpy(&item->FindData.ftLastAccessTime, &pItem->ftLastAccessTime, sizeof(FILETIME));
			memcpy(&item->FindData.ftLastWriteTime, &pItem->ftLastWriteTime, sizeof(FILETIME));


			if ( item )
			{
#ifdef UNICODE
				item->FindData.lpwszFileName = StrDuplicate(lpFileName); 
#else
				_tcscpy (item->FindData.cFileName, lpFileName);
#endif
				if ( bIsFolder )
				{
					item->FindData.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
					item->UserData = 0;
				}
				else
					item->UserData = (DWORD_PTR)pItem;
			}

			StrFree(lpFileName);
		}
	}

	*pPanelItem = pPanelItems.data();
	*pItemsNumber = pPanelItems.count();

	return TRUE;
}

void ArchivePanel::pGetOpenPluginInfo (
		OpenPluginInfo *pInfo
		)
{
	pInfo->StructSize = sizeof (OpenPluginInfo);

	pInfo->Flags = OPIF_USEFILTER | OPIF_USEHIGHLIGHTING | OPIF_USESORTGROUPS | OPIF_ADDDOTS;
	pInfo->CurDir = m_strPathInArchive;

	if ( m_pArchive )
	{
		ArchiveFormat *pFormat = m_pArchive->GetFormat();

		m_strPanelTitle.Format(
				_T(" %s%s%s%s%s "),
				pFormat->GetName(),
				_T(":"),
				FSF.PointToName(m_pArchive->GetFileName()),
				m_strPathInArchive.IsEmpty()?_T(""):_T("\\"),
				m_strPathInArchive.GetString()
				);

		pInfo->HostFile = m_pArchive->GetFileName();
		pInfo->PanelTitle = m_strPanelTitle;

		pInfo->InfoLines = m_pArchiveInfo;
		pInfo->InfoLinesNumber = m_nArchiveInfoItems;

		m_strShortcutData.Format(
				_T("%s%s%s"),
				GUID2STR(pFormat->GetUID()),
				GUID2STR(pFormat->GetPlugin()->GetUID()),
				GUID2STR(pFormat->GetModule()->GetUID())
				);

		pInfo->ShortcutData = m_strShortcutData;
	}
}



void ArchivePanel::GetArchiveItemsToProcess (
		const PluginPanelItem *pPanelItems,
		int nItemsNumber,
		ArchiveItemArray &items
		)
{
	ArchiveItem *dest = NULL;

	m_OS.uTotalFiles = 0;
	m_OS.uTotalSize = 0;

	for (int i = 0; i < nItemsNumber; i++)
	{
		const FAR_FIND_DATA *data = &pPanelItems[i].FindData;
		ArchiveItem *src = (ArchiveItem*)pPanelItems[i].UserData;

		if ( src )
		{
			dest = items.add(*src);

			dest->lpFileName = StrDuplicate(src->lpFileName);
			dest->lpAlternateFileName = StrDuplicate(src->lpAlternateFileName);
		}
		else
		{
			dest = items.add();
			//если мы сюда попали - у нас похоже проблемы.

			string strFullName;

			if ( !m_strPathInArchive.IsEmpty() )
			{
				strFullName = m_strPathInArchive;
				AddEndSlash(strFullName);
			}

			strFullName += FINDDATA_GET_NAME_PTR(data);

			FindDataToArchiveItem(data, dest);

			StrFree((void*)dest->lpFileName);
			dest->lpFileName = StrDuplicate(strFullName);

			dest->UserData = 0;
		}

		m_OS.uTotalSize += FINDDATA_GET_SIZE_PTR(data);

		if ( OptionIsOn(data->dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
		{
			string strPath = m_strPathInArchive;

			if ( !m_strPathInArchive.IsEmpty() )
				strPath += _T("\\");

			strPath += FINDDATA_GET_NAME_PTR(data);
			strPath += _T("\\");

			for (unsigned int k = 0; k < m_pArchiveFiles.count(); k++)
			{
				ArchiveItem *src = &m_pArchiveFiles[k];

				if ( IsFileInFolder (strPath, src->lpFileName) )
				{
					dest = items.add(*src);

					dest->lpFileName = StrDuplicate(src->lpFileName);
					dest->lpAlternateFileName = StrDuplicate(src->lpAlternateFileName);

					m_OS.uTotalSize += src->nFileSize;
				}
			}
		}
	}

	m_OS.uTotalFiles = items.count();
}



struct ScanStruct {
	OperationStructEx* pOS;
	const TCHAR *lpSourcePath;
	ArchiveItemArray* items;
};


int __stdcall ScanDirectory (
		const FAR_FIND_DATA *fdata,
		const TCHAR *lpFullName,
		ScanStruct *pSS
		)
{
	string strFileNameCopy;

	const TCHAR *lpFileName = lpFullName+_tcslen(pSS->lpSourcePath);

	if ( (*lpFileName == _T('\\')) || (*lpFileName == _T('/')) )
		lpFileName++;

	strFileNameCopy = lpFileName;

	ArchiveItem *item = pSS->items->add();

	FindDataToArchiveItem(fdata, item);

	StrFree((void*)item->lpFileName);

	item->lpFileName = StrDuplicate(strFileNameCopy);

	if ( (item->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY )
	{
		pSS->pOS->uTotalFiles++;
		pSS->pOS->uTotalSize += item->nFileSize;
	}

	return TRUE;
}

void ArchivePanel::GetPanelItemsToProcess(
		const PluginPanelItem* pPanelItems,
		int nItemsNumber,
		ArchiveItemArray &items
		)
{
	FarPanelInfo info;
	
	m_OS.uTotalFiles = 0;
	m_OS.uTotalSize = 0;

	ScanStruct ss;

	ss.pOS = &m_OS;
	ss.lpSourcePath = info.GetCurrentDirectory();
	ss.items = &items;

	for (int i = 0; i < nItemsNumber; i++)
	{
		ArchiveItem* pItem = items.add();

		FindDataToArchiveItem(&pPanelItems[i].FindData, pItem);

		if ( (pItem->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
		{
			string strFullName = info.GetCurrentDirectory();

			AddEndSlash(strFullName);

			strFullName += pItem->lpFileName;

			FSF.FarRecursiveSearch (strFullName, _T("*.*"), (FRSUSERFUNC)ScanDirectory, FRS_RECUR, &ss);
		}
		else
		{
			m_OS.uTotalFiles++;
			m_OS.uTotalSize += pItem->nFileSize;
		}
	}
}


#include "dlg/dlgAddEditTemplate.cpp"
#include "dlg/dlgModifyCreateArchive.cpp"

int ArchivePanel::pPutFiles(
		const PluginPanelItem *PanelItem,
		int ItemsNumber,
		int Move,
		int OpMode
		)
{
	bool bResult = false;

	FarPanelInfo info;
	ArchiveItemArray items;

	m_bPasswordSet = false;

	if ( m_pArchive == NULL )
	{
		CreateArchiveParams params;

		if ( dlgModifyCreateArchive(
				this, 
				&params
				) )
		{
			m_bPasswordSet = true;

			int nSelectedCount = info.GetSelectedItemsCount();

			bool bSeparately = params.bSeparateArchives && (nSelectedCount > 1);
			int	nCount = (bSeparately)?nSelectedCount:1;

			string strArchiveName;

			for (int el = 0; el < nCount; el++)
			{
				if ( bSeparately )
				{
					PluginPanelItem Item;
					info.GetSelectedItem(el, &Item);

					items.reset(); //???

					GetPanelItemsToProcess(&Item, 1, items);

#ifdef UNICODE
					strArchiveName = FSF.PointToName(Item.FindData.lpwszFileName);
#else
					strArchiveName = FSF.PointToName(Item.FindData.cFileName);
#endif

				
					if ( (Item.FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY )
						CutTo(strArchiveName, _T('.'), true);

					info.FreePanelItem(&Item);
				}
				else
				{
					strArchiveName = params.strFileName;

					PluginPanelItem* pItems = new PluginPanelItem[nSelectedCount];

					for (int i = 0; i < nSelectedCount; i++)
						info.GetSelectedItem(i, &pItems[i]);

					GetPanelItemsToProcess(pItems, nSelectedCount, items);

					for (int i = 0; i < nSelectedCount; i++)
						info.FreePanelItem(&pItems[i]);

					delete pItems;
				}

				if ( !params.bExactName )
				{
					strArchiveName += _T(".");
					strArchiveName += params.pFormat->GetDefaultExtention();
				}

				string strFullArchiveName = info.GetCurrentDirectory();
				AddEndSlash(strFullArchiveName);

				strFullArchiveName += strArchiveName;
				
				//Archive* pArchive = pManager->OpenCreateArchive(params.pFormat, strFullArchiveName, this, Callback, true);
				//BADBAD, надо убедиться, что отсюда сразу в ClosePlugin попадаем
				m_pArchive = pManager->OpenCreateArchive(params.pFormat, strFullArchiveName, this, Callback, true);

				if ( m_pArchive )
					bResult = AddFiles(items, info.GetCurrentDirectory());
			}
		}
	}
	else
	{
		GetPanelItemsToProcess(PanelItem, ItemsNumber, items);
		bResult = AddFiles(items, info.GetCurrentDirectory());
	}

	return bResult;
}

#include "dlg/dlgUnpackFiles.cpp"


int ArchivePanel::pGetFiles(
		const PluginPanelItem *PanelItem,
		int ItemsNumber,
		int Move,
		const TCHAR* DestPath,
		int OpMode
		)
{
	int bResult = false; ///!!! error!!!

#ifdef UNICODE
	DestPath = *(TCHAR**)DestPath;
#endif

	string strDestPath = DestPath;

	if ( ((OpMode & OPM_SILENT) == OPM_SILENT) || dlgUnpackFiles(DestPath, Move, strDestPath) )
	{
		farPrepareFileName(strDestPath);

		ArchiveItemArray items; //100??

		GetArchiveItemsToProcess(PanelItem, ItemsNumber, items);

		bResult = Extract(items, strDestPath, (OpMode == OPM_VIEW) || (OpMode == OPM_EDIT));

		if ( Move && bResult )
			bResult = Delete(items);
	}

	return bResult;
}


#include "msg/msgDeleteFiles.cpp"

int ArchivePanel::pDeleteFiles(
		const PluginPanelItem* PanelItem,
		int ItemsNumber,
		int OpMode
		)
{
	ArchiveItemArray items;

	if ( msgDeleteFiles () )
	{
		GetArchiveItemsToProcess(PanelItem, ItemsNumber, items);

		//m_pArchive->SetOperationStruct(&os);

		return Delete(items);
	}

	return false;
}



void ArchivePanel::pFreeFindData (
		PluginPanelItem *pPanelItem,
		int nItemsNumber
		)
{
#ifdef UNICODE

	for (int i = 0; i < nItemsNumber; i++)
		StrFree((void*)pPanelItem[i].FindData.lpwszFileName);
#endif
}

int ArchivePanel::pSetDirectory(
		const TCHAR *Dir,
		int nOpMode
		)
{
	bool bResult = FALSE;

	if ( !_tcscmp (Dir, _T("..")) )
	{
		if ( _tcschr (m_strPathInArchive, _T('\\')) )
			CutToSlash(m_strPathInArchive, true);
		else
			m_strPathInArchive = NULL;

		bResult = TRUE;
	}
	else

	if ( !_tcscmp (Dir, _T("\\")) )
	{
		m_strPathInArchive = NULL;

		bResult = TRUE;
	}
	else
	{
		int nCurDirLength = m_strPathInArchive.GetLength();
		int nDirLength = StrLength(Dir);

		if ( nCurDirLength )
			nCurDirLength++;

		for (unsigned int i = 0; i < m_pArchiveFiles.count(); i++)
		{
			ArchiveItem *item = &m_pArchiveFiles[i];

			const TCHAR *lpCurName = item->lpFileName;

			if ( ((int)StrLength(lpCurName) >= nCurDirLength+nDirLength) &&
					!_tcsncmp(Dir, lpCurName+nCurDirLength, nDirLength) )
			{
				const TCHAR *p = lpCurName+nCurDirLength+nDirLength;

				if ( (*p == _T('\\')) || (*p == _T('/')) || !*p )
				{
					if ( !m_strPathInArchive.IsEmpty() )
						m_strPathInArchive += _T("\\");

					m_strPathInArchive += Dir;

					bResult = TRUE;

					break;
				}
			}
		}
	}

	m_pArchive->SetCurrentDirectory(m_strPathInArchive);

	return bResult;
}


void ArchivePanel::pClosePlugin ()
{
	for (unsigned int i = 0; i < m_pArchiveFiles.count(); i++)
		m_pArchive->FreeArchiveItem(&m_pArchiveFiles[i]);

	if ( m_pArchive )
		m_pManager->CloseArchive(m_pArchive);

	if ( m_pArchiveInfo )
	{
#ifdef UNICODE
		for (int i = 0; i < m_nArchiveInfoItems; i++)
		{
			StrFree((void*)m_pArchiveInfo[i].Data);
			StrFree((void*)m_pArchiveInfo[i].Text);
		}
#endif
		delete m_pArchiveInfo;
	}
}

/*
#define OPERATION_TEST					0
#define OPERATION_ADD_ARCHIVE_COMMENT	1
#define OPERATION_ADD_FILE_COMMENT		2
#define OPERATION_CONVERT_TO_SFX		3
#define OPERATION_RECOVER				4
#define OPERATION_ADD_RECOVERY_RECORD	5
#define OPERATION_LOCK					6

  */

int ArchivePanel::pProcessHostFile(
		const PluginPanelItem *PanelItem,
		int ItemsNumber,
		int OpMode
		)
{
	return FALSE;

/*
	int nResult = mnuChooseOperation ();

	if ( nResult != -1 )
	{
		int nCommand = -1;

		switch ( nResult ) {

		case OPERATION_TEST:
			nCommand = COMMAND_TEST;
			break;

		case OPERATION_ADD_ARCHIVE_COMMENT:
			nCommand = COMMAND_ARCHIVE_COMMENT;
			break;

		case OPERATION_ADD_FILE_COMMENT:
			nCommand = COMMAND_FILE_COMMENT;
			break;

		case OPERATION_CONVERT_TO_SFX:
			nCommand = COMMAND_CONVERT_TO_SFX;
			break;

		case OPERATION_RECOVER:
			nCommand = COMMAND_RECOVER;
			break;

		case OPERATION_ADD_RECOVERY_RECORD:
			nCommand = COMMAND_ADD_RECOVERY_RECORD;
			break;

		case OPERATION_LOCK:
			nCommand = COMMAND_LOCK;
			break;
		};
*/
	/*	ArchiveItemArray items;
		OperationStruct os;

		GetArchiveItemsToProcess (
				PanelItem,
				ItemsNumber,
				items,
				&os
				);

		m_pArchive->SetOperationStruct(&os);

		if ( nCommand == COMMAND_TEST )
		{
			if ( m_pArchive->StartOperation(OPERATION_TEST) )
			{
				m_pArchive->Test(items);
				m_pArchive->EndOperation(OPERATION_TEST);
			}
		}
		else
			ExecuteCommand (
					items,
					nCommand,
					NULL,
					NULL
					);
	*/
/*		return TRUE;
	}

	return FALSE; */
}

int ArchivePanel::pMakeDirectory(const TCHAR* lpDirectory, int nOpMode)
{
	return TRUE;
}

int ArchivePanel::pProcessKey (
		int nKey,
		DWORD dwControlState
		)
{
	if ( (nKey == VK_F7) && (dwControlState == 0) )
	{
		TCHAR szFolderPath[MAX_PATH];

		if ( Info.InputBox (
				_M(MMakeFolderTitle),
				_M(MMakeFolderPrompt),
				NULL,
				NULL,
				szFolderPath,
				MAX_PATH,
				NULL,
				FIB_EXPANDENV|FIB_BUTTONS
				) )
		{
			bool bResult = m_pArchive->MakeDirectory(szFolderPath);

			if ( bResult )
				Update();

			return FALSE;
		}
	}   

	return FALSE;
}


void ArchivePanel::Update()
{
	FarPanelInfo info(this);

	info.Update();
	info.Redraw();
}

#include "dlg.cpp"

LONG_PTR __stdcall ArchivePanel::Callback(HANDLE hPanel, int nMsg, int nParam1, LONG_PTR nParam2)
{
	int nResult = 0;

	ArchivePanel* pPanel = (ArchivePanel*)hPanel;

	if ( pPanel )
	{
		if ( nMsg == AM_NEED_PASSWORD )
			nResult = pPanel->OnQueryPassword(nParam1, (PasswordStruct*)nParam2);

		if ( nMsg == AM_START_OPERATION )
			nResult = pPanel->OnStartOperation(nParam1, (StartOperationStruct*)nParam2);

		if ( nMsg == AM_PROCESS_FILE )
			nResult = pPanel->OnProcessFile((ProcessFileStruct*)nParam2);

		if ( nMsg == AM_PROCESS_DATA )
			nResult = pPanel->OnProcessData((ProcessDataStruct*)nParam2);

		//if ( nMsg == AM_FILE_ALREADY_EXISTS )
		//	nResult = pPanel->OnFileAlreadyExists((OverwriteStruct*)nParam2);
	}

	return nResult;
}

#include "dlg/dlgFileAlreadyExists.cpp"

int ArchivePanel::OnFileAlreadyExists(OverwriteStruct* pOS)
{
	if ( m_OS.overwrite == RESULT_OVERWRITE_ALL )
		return RESULT_OVERWRITE;
	
	if ( m_OS.overwrite == RESULT_SKIP_ALL )
		return RESULT_SKIP;
	
	if ( m_OS.overwrite == RESULT_CANCEL )
		return RESULT_CANCEL;
	
	if ( GetFileAttributes(pOS->lpFileName) != INVALID_FILE_ATTRIBUTES )
	{
		m_OS.overwrite = msgFileAlreadyExists(pOS->lpFileName, pOS->pItem);

		if ( (m_OS.overwrite == RESULT_OVERWRITE_ALL) || (m_OS.overwrite == RESULT_OVERWRITE) )
			return RESULT_OVERWRITE;

		if ( m_OS.overwrite == RESULT_CANCEL )
			return RESULT_CANCEL;

		m_OS.uTotalSize -= pOS->pItem->nFileSize; //he?

		return RESULT_SKIP;
	}

	return RESULT_OVERWRITE;
}

int ArchivePanel::OnStartOperation(int nOperation, StartOperationStruct *pOS)
{
	if ( pOS )
	{
		if ( OptionIsOn(pOS->dwFlags, OS_FLAG_TOTALSIZE) )
			m_OS.uTotalSize = pOS->uTotalSize;

		if ( OptionIsOn(pOS->dwFlags, OS_FLAG_TOTALFILES) )
			m_OS.uTotalFiles = pOS->uTotalFiles;
	}

	m_OS.nOperation = nOperation;
	m_OS.bFirstFile = true;
	m_OS.overwrite = RESULT_UNKNOWN;

	return 1;
}

int ArchivePanel::OnQueryPassword(int nMode, PasswordStruct* pPassword)
{
   	if ( nMode == PASSWORD_RESET )
   	{
   		m_bPasswordSet = false;
		m_strPassword = NULL;
		return TRUE;
   	}

   	bool bResult = true;

   	if ( !m_bPasswordSet )
   	{
   		TCHAR *buffer = m_strPassword.GetBuffer(512);

   		bResult = Info.InputBox (
   				(nMode == PASSWORD_LIST)?_M(MQueryPasswordFileList):_M(MQueryPasswordContents),
   				_M(MQueryPasswordEnterPassword),
   				NULL,
   				NULL,
   				buffer,
   				512,
   				NULL,
   				0
   				);

		m_strPassword.ReleaseBuffer();
    
       	if ( !bResult )
			m_strPassword = NULL;
		else
		{
			m_bPasswordSet = true;

			if ( m_pArchive )
				m_pArchive->SetPassword(m_strPassword);
		}
   	}

   	if ( m_bPasswordSet && bResult )
   	{
   		_tcscpy (pPassword->lpBuffer, m_strPassword);
   		return TRUE;
   	}

   	return FALSE;
}

int ArchivePanel::OnProcessFile(ProcessFileStruct *pfs)
{
	if ( !pfs )
		__debug(_T("ERROR, EMPTY PFS!"));

	int nOverwrite = RESULT_OVERWRITE;

	if ( m_OS.nOperation == OPERATION_EXTRACT )
	{
		OverwriteStruct OS;

		OS.pItem = pfs->pItem;
		OS.lpFileName = pfs->lpDestFileName;

		nOverwrite = OnFileAlreadyExists(&OS);
	}

	if ( nOverwrite != RESULT_CANCEL )
	{

	m_OS.pCurrentItem = pfs?pfs->pItem:NULL;

	if ( m_OS.bFirstFile )
	{
		//if ( !OptionIsOn (m_OS.nMode, OPM_SILENT) )
		{
			if ( m_OS.nOperation == OPERATION_EXTRACT )
				m_OS.Dlg.Show(_M(MProcessFileExtractionTitle)/*, _M(MProcessFileExtraction)*/);
			else
			if ( m_OS.nOperation == OPERATION_ADD )
				m_OS.Dlg.Show(_M(MProcessFileAdditionTitle)/*, _M(MProcessFileAddition)*/);
			else
			if ( m_OS.nOperation == OPERATION_DELETE )
				m_OS.Dlg.Show(_M(MProcessFileDeletionTitle)/*, _M(MProcessFileDeletion)*/);
			else
				__debug(_T("BAD OPERATION"));

			Info.Text(0, 0, 0, 0); //BUGBUG
		}

		m_OS.bFirstFile = false;
		m_OS.uTotalProcessedSize = 0;
	}

	//if ( !OptionIsOn(m_OS.nMode, OPM_SILENT) )
	{
		m_OS.Dlg.SetFileName(false, m_OS.pCurrentItem->lpFileName);

		if ( pfs && pfs->lpDestFileName )
			m_OS.Dlg.SetFileName(true, pfs->lpDestFileName);

		Info.Text(0, 0, 0, 0);
	}

	if ( m_OS.pCurrentItem )
		m_OS.uFileSize = m_OS.pCurrentItem->nFileSize;
	else
		m_OS.uFileSize = m_OS.uTotalSize;

	m_OS.uProcessedSize = 0;
	}

	return nOverwrite;
}

int ArchivePanel::OnProcessData(ProcessDataStruct* pDS)
{
	m_OS.uTotalProcessedSize += pDS->uProcessedSize;
	m_OS.uProcessedSize += pDS->uProcessedSize;

	double div;

	if ( m_OS.uFileSize )
		div = (double)m_OS.uProcessedSize/(double)m_OS.uFileSize;
	else
		div = 1;

	m_OS.Dlg.SetIndicator(false, div);

	if ( m_OS.uTotalSize )
		div = (double)m_OS.uTotalProcessedSize/(double)m_OS.uTotalSize;
	else
		div = 1;

	m_OS.Dlg.SetIndicator(true, div);

	Info.Text(0, 0, 0, 0);

	if ( CheckForEsc () )
	{
	/*	if ( !OptionIsOn(m_OS.nMode, OPM_SILENT) )
		{
			Info.Text(m_OS.Dlg.Coord.X+5, m_OS.Dlg.Coord.Y+2, FarGetColor (COL_DIALOGTEXT), _M(MProcessDataOperationCanceled));
			Info.Text(0, 0, 0, 0);
		}
	*/
		return FALSE;
	}

	return TRUE;
}

bool ArchivePanel::Extract(
		const ArchiveItemArray& items, 
		const TCHAR *lpDestDiskPath, 
		bool bWithoutPath
		)
{
	OnStartOperation(OPERATION_EXTRACT, nullptr);
	return m_pArchive->Extract(items, lpDestDiskPath, bWithoutPath);
}

bool ArchivePanel::Delete(const ArchiveItemArray& items)
{
	OnStartOperation(OPERATION_DELETE, nullptr);
	return m_pArchive->Delete(items);
}

bool ArchivePanel::AddFiles(const ArchiveItemArray& items, const TCHAR* lpSourceDiskPath)
{
	OnStartOperation(OPERATION_ADD, nullptr);
	return m_pArchive->AddFiles(items, lpSourceDiskPath);
}

bool ArchivePanel::Test(const ArchiveItemArray& items)
{
	OnStartOperation(OPERATION_TEST, nullptr);
	return m_pArchive->Test(items);
}

