#include "newarc.h"

#include "processname.cpp"

void QuoteSpaceOnly(string& strSrc)
{
	TCHAR* lpBuffer = strSrc.GetBuffer(strSrc.GetLength()+5);

	FSF.QuoteSpaceOnly(lpBuffer);

	strSrc.ReleaseBuffer();
}


struct ExecuteStruct {
	const ArchiveItemArray& items;
	const TCHAR* lpCommand;
	const TCHAR* lpCurrentDiskPath;
	const TCHAR* lpAdditionalCommandLine;
	const TCHAR* lpPassword;
	bool bHideOutput;

	ExecuteStruct(const ArchiveItemArray& items) : items(items)
	{
		lpCommand = nullptr;
		lpCurrentDiskPath = nullptr;
		lpAdditionalCommandLine = nullptr;
		lpPassword = nullptr;
		bHideOutput = false;
	}
};

int ExecuteCommand(Archive* pArchive, void* pParam)
{
	ExecuteStruct* pES = (ExecuteStruct*)pParam;

	int nResult = RESULT_ERROR;

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

	string strFileName = pArchive->GetFileName();
	string strPath;
		
	if ( pES->lpCurrentDiskPath )
		strPath = pES->lpCurrentDiskPath;
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
	psParam.strPassword = pES->lpPassword;
	psParam.strPathInArchive = pArchive->GetCurrentDirectory();
	psParam.strAdditionalCommandLine = pES->lpAdditionalCommandLine;

	string strExecuteString;
	int nStartItemNumber = 0;

	while ( true )
	{
		int nParseResult = ParseString(
				pES->items,
				pES->lpCommand,
				strExecuteString,
				&psParam,
				nStartItemNumber
				);

		if ( (nParseResult == PE_SUCCESS) || (nParseResult == PE_MORE_FILES) )
		{
			PROCESS_INFORMATION pInfo;
			STARTUPINFO sInfo;

			memset (&sInfo, 0, sizeof (STARTUPINFO));
			sInfo.cb = sizeof (STARTUPINFO);

			apiExpandEnvironmentStrings(strExecuteString, strExecuteString);

			HANDLE hScreen = Info.SaveScreen(0, 0, -1, -1);

#ifdef UNICODE
			Info.Control(INVALID_HANDLE_VALUE, FCTL_GETUSERSCREEN, 0, 0);
#else
			Info.Control(INVALID_HANDLE_VALUE, FCTL_GETUSERSCREEN, 0);
#endif
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

				nResult = (dwExitCode == 0)?RESULT_SUCCESS:RESULT_ERROR;
			}
			else
			{
				string strError;
				strError.Format(_T("CreateProcess failed - %d\n%s"), GetLastError(), strExecuteString.GetString());
				msgError(strError);
			}

#ifdef UNICODE
			Info.Control(INVALID_HANDLE_VALUE, FCTL_SETUSERSCREEN, 0, 0);
#else
			Info.Control(INVALID_HANDLE_VALUE, FCTL_SETUSERSCREEN, 0);
#endif

			Info.RestoreScreen(NULL);
			Info.RestoreScreen(hScreen);
		}

		if ( nParseResult != PE_MORE_FILES )
			break;
	}
		
	DeleteFile (psParam.strListFileName); //WARNING!!!

	return nResult;
}



bool CheckForEsc ()
{
	bool EC = false;
/*
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
	}*/

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

	m_bMultiVolume = false;
}

Array<ArchiveFormat*>& ArchivePanel::GetFormats()
{
	return m_pFormats;
}

ArchivePanel::~ArchivePanel()
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

	if ( !m_pArchive->ReadArchiveItems() )
		return FALSE; //??? в ¬ ­Ґв FALSE

	const ArchiveInfoItem* pInfoItems;

	m_nArchiveInfoItems = m_pArchive->GetArchiveInfo(m_bMultiVolume, &pInfoItems);

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
	Array<ArchiveTreeNode*> items;

	m_pArchive->SetCurrentDirectory(m_strPathInArchive, false); //а вдруг архив перезагрузили, вернем путь (с рута)
	m_pArchive->GetArchiveTreeItems(items, false); //no recursive

	for (unsigned int i = 0; i < items.count(); i++)
	{
		PluginPanelItem item;
		memset(&item, 0, sizeof(PluginPanelItem));

		ArchiveTree* node = items.at(i);
		const ArchiveItem* src = node->GetOriginalItem();

		item.FindData.lpwszFileName = StrDuplicate(node->GetFileName());
		item.FindData.lpwszAlternateFileName = StrDuplicate(node->GetFileName());
		item.UserData = (DWORD_PTR)node;

		if ( node->IsDummy() )
			item.FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
		else
		{
			item.FindData.dwFileAttributes = src->dwFileAttributes;
			item.FindData.nFileSize = src->nFileSize;
			item.FindData.nPackSize = src->nPackSize;

			memcpy(&item.FindData.ftCreationTime, &src->ftCreationTime, sizeof(FILETIME));
			memcpy(&item.FindData.ftLastAccessTime, &src->ftLastAccessTime, sizeof(FILETIME));
			memcpy(&item.FindData.ftLastWriteTime, &src->ftLastWriteTime, sizeof(FILETIME));

			item.CRC32 = src->dwCRC32;
		}

		pPanelItems.add(item);
	}

	*pPanelItem = pPanelItems.data();
	*pItemsNumber = pPanelItems.count();

	return TRUE;
}

void ArchivePanel::pGetOpenPluginInfo(
		OpenPluginInfo *pInfo
		)
{
	pInfo->StructSize = sizeof(OpenPluginInfo);

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


unsigned __int64 GetArchiveItemsToProcessFromNode(ArchiveTreeNode* node, ArchiveItemArray& items)
{
	unsigned __int64 uTotalSize = 0;

	if ( !node->IsDummy() )
	{
		const ArchiveItem* item = node->GetOriginalItem();
		uTotalSize = item->nFileSize;

		items.add(*item);
	}

	for (ArchiveTreeNodesIterator itr = node->children.begin(); itr != node->children.end(); ++itr)
		uTotalSize += GetArchiveItemsToProcessFromNode(itr->second, items);

	return uTotalSize;
}

void ArchivePanel::GetArchiveItemsToProcess(
		const PluginPanelItem *pPanelItems,
		int nItemsNumber,
		ArchiveItemArray &items
		)
{
	m_OS.uTotalFiles = 0;
	m_OS.uTotalSize = 0;

	for (int i = 0; i < nItemsNumber; i++)
	{
		const FAR_FIND_DATA *data = &pPanelItems[i].FindData;

		ArchiveTreeNode* node = (ArchiveTreeNode*)pPanelItems[i].UserData;

		//ад и кромешный пиздец. отдаем обратно модулю то, что он сам и навыделял. т.е. хоть эти ArchiveItem и новые, данные в них 
		//старые. т.е. удалять их нельзя ни при каких обстоятельствах!

		m_OS.uTotalSize += GetArchiveItemsToProcessFromNode(node, items);
	}

	m_OS.uTotalFiles = items.count(); 
}


void FindDataToArchiveItem(const FAR_FIND_DATA *src, ArchiveItem *dest)
{
	dest->nFileSize = FINDDATA_GET_SIZE_PTR(src);
	dest->dwFileAttributes = src->dwFileAttributes;
			
	memcpy(&dest->ftCreationTime, &src->ftCreationTime, sizeof(FILETIME));
	memcpy(&dest->ftLastWriteTime, &src->ftLastWriteTime, sizeof(FILETIME));
	memcpy(&dest->ftLastAccessTime, &src->ftLastAccessTime, sizeof(FILETIME));

	dest->lpFileName = StrDuplicate(FINDDATA_GET_NAME_PTR(src));
	dest->lpAlternateFileName = NULL;
}


struct ScanStruct {
	OperationStructEx* pOS;
	const TCHAR *lpSourcePath;
	ArchiveItemArray* items;
};


int __stdcall ScanDirectory(
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
#ifdef UNICODE
		const wchar_t* SrcPath,
#endif
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
			m_strPassword = params.strPassword;

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

					delete [] pItems;
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
					bResult = AddFiles(items, info.GetCurrentDirectory(), params.strConfig);
			}
		}
	}
	else
	{
		GetPanelItemsToProcess(PanelItem, ItemsNumber, items);

#ifdef UNICODE
		bResult = AddFiles(items, SrcPath ? SrcPath : info.GetCurrentDirectory(), nullptr);
#else
		bResult = AddFiles(items, info.GetCurrentDirectory(), nullptr);
#endif
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

	bool bExtractWithoutPath = true;

#ifdef UNICODE
	DestPath = *(TCHAR**)DestPath;
#endif

	if ( OpMode & (OPM_VIEW | OPM_EDIT | OPM_FIND | OPM_QUICKVIEW) ) //hmm...
		m_strLastDestPath = DestPath;

	if ( ((OpMode & OPM_SILENT) == OPM_SILENT) || dlgUnpackFiles(DestPath, Move, m_strLastDestPath, bExtractWithoutPath) )
	{
		farPrepareFileName(m_strLastDestPath);

		ArchiveItemArray items; //100??

		GetArchiveItemsToProcess(PanelItem, ItemsNumber, items);

		bResult = Extract(items, m_strLastDestPath, bExtractWithoutPath);

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
	if ( msgDeleteFiles () )
	{
		ArchiveItemArray items;
		GetArchiveItemsToProcess(PanelItem, ItemsNumber, items);

		return Delete(items);
	}

	return false;
}



void ArchivePanel::pFreeFindData(
		PluginPanelItem *pPanelItem,
		int nItemsNumber
		)
{
#ifdef UNICODE

	for (int i = 0; i < nItemsNumber; i++)
	{
		StrFree((void*)pPanelItem[i].FindData.lpwszFileName);
		StrFree((void*)pPanelItem[i].FindData.lpwszAlternateFileName);
	}
#endif
}

int ArchivePanel::pSetDirectory(
		const TCHAR *Dir,
		int nOpMode
		)
{
	if ( m_pArchive->SetCurrentDirectory(Dir) )
	{
		m_strPathInArchive = m_pArchive->GetCurrentDirectory();
		return TRUE;
	}

	return FALSE;
}


void ArchivePanel::pClosePlugin()
{
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

#include "mnu\\mnuChooseOperation.cpp"

#define MENU_OPERATION_TEST					0
#define MENU_OPERATION_ADD_ARCHIVE_COMMENT	1
#define MENU_OPERATION_ADD_FILE_COMMENT		2
#define MENU_OPERATION_CONVERT_TO_SFX		3
#define MENU_OPERATION_RECOVER				4
#define MENU_OPERATION_ADD_RECOVERY_RECORD	5
#define MENU_OPERATION_LOCK					6

int ArchivePanel::pProcessHostFile(
		const PluginPanelItem *PanelItem,
		int ItemsNumber,
		int OpMode
		)
{
	int nMenuItem = -1;

	if ( (nMenuItem = mnuChooseOperation()) == -1 )
		return FALSE;

	int nCommand = -1;

	switch ( nMenuItem ) {

	case MENU_OPERATION_TEST:
		nCommand = COMMAND_TEST;
		break;

	case MENU_OPERATION_ADD_ARCHIVE_COMMENT:
		nCommand = COMMAND_ARCHIVE_COMMENT;
		break;

	case MENU_OPERATION_ADD_FILE_COMMENT:
		nCommand = COMMAND_FILE_COMMENT;
		break;

	case MENU_OPERATION_CONVERT_TO_SFX:
		nCommand = COMMAND_CONVERT_TO_SFX;
		break;

	case MENU_OPERATION_RECOVER:
		nCommand = COMMAND_RECOVER;
		break;

	case MENU_OPERATION_ADD_RECOVERY_RECORD:
		nCommand = COMMAND_ADD_RECOVERY_RECORD;
		break;

	case MENU_OPERATION_LOCK:
		nCommand = COMMAND_LOCK;
		break;
	};

	if ( nCommand == -1 )
		return FALSE;
	
	string strCommand;

	bool bCommandEnabled = GetCommand(nCommand, strCommand);
	bool bInternalTest = (nCommand == COMMAND_TEST) && m_pArchive->QueryCapability(AFF_SUPPORT_INTERNAL_TEST);

	if ( bCommandEnabled || bInternalTest )
	{
		ArchiveItemArray items;

		if ( OptionIsOn(OpMode, OPM_TOPLEVEL) )
		{
			ArchiveTreeNode* root = m_pArchive->GetRoot();

			m_OS.uTotalSize = GetArchiveItemsToProcessFromNode(root, items);
			m_OS.uTotalFiles = items.count();
		}
		else
			GetArchiveItemsToProcess(PanelItem, ItemsNumber, items);

		int nOperationResult = RESULT_ERROR;

		if ( bInternalTest )
			nOperationResult = Test(items);

		if ( bCommandEnabled && (nOperationResult == RESULT_ERROR) )
		{
			ExecuteStruct ES(items);

			ES.lpCommand = strCommand;

			nOperationResult = m_pArchive->ExecuteAsOperation(bInternalTest?OPERATION_TEST:OPERATION_OTHER, ExecuteCommand, &ES); 
		}

		return nOperationResult == RESULT_SUCCESS;
	}

	return FALSE;
}

int ArchivePanel::pMakeDirectory(const TCHAR* lpDirectory, int nOpMode)
{
	return TRUE;
}

int ArchivePanel::pProcessKey(
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
			bool bResult = MakeDirectory(szFolderPath);

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

		if ( nMsg == AM_ENTER_STAGE )
			nResult = pPanel->OnEnterStage(nParam1);

		if ( nMsg == AM_PROCESS_FILE )
			nResult = pPanel->OnProcessFile((ProcessFileStruct*)nParam2);

		if ( nMsg == AM_PROCESS_DATA )
			nResult = pPanel->OnProcessData((ProcessDataStruct*)nParam2);

		if ( nMsg == AM_REPORT_ERROR )
			nResult = pPanel->OnReportError((ReportErrorStruct*)nParam2);

		if ( nMsg == AM_NEED_VOLUME )
			nResult = pPanel->OnNeedVolume((VolumeStruct*)nParam2);
		//if ( nMsg == AM_FILE_ALREADY_EXISTS )
		//	nResult = pPanel->OnFileAlreadyExists((OverwriteStruct*)nParam2);
	}

	return nResult;
}

#include "dlg/dlgFileAlreadyExists.cpp"

int ArchivePanel::OnFileAlreadyExists(OverwriteStruct* pOS)
{
	if ( m_OS.overwrite == PROCESS_OVERWRITE_ALL )
		return PROCESS_OVERWRITE;
	
	if ( m_OS.overwrite == PROCESS_SKIP_ALL )
		return PROCESS_SKIP;
	
	if ( m_OS.overwrite == PROCESS_CANCEL )
		return PROCESS_CANCEL;
	
	if ( GetFileAttributes(pOS->lpFileName) != INVALID_FILE_ATTRIBUTES )
	{
		m_OS.overwrite = msgFileAlreadyExists(pOS->lpFileName, pOS->pItem);

		if ( (m_OS.overwrite == PROCESS_OVERWRITE_ALL) || (m_OS.overwrite == PROCESS_OVERWRITE) )
			return PROCESS_OVERWRITE;

		if ( m_OS.overwrite == PROCESS_CANCEL )
			return PROCESS_CANCEL;

		m_OS.uTotalSize -= pOS->pItem->nFileSize; //he?

		return PROCESS_SKIP;
	}

	return PROCESS_OVERWRITE;
}

int ArchivePanel::OnNeedVolume(VolumeStruct* pVS)
{
	return Info.InputBox(
			_T("Enter volume"), 
			_T("Volume file name"),
			nullptr,
			pVS->lpSuggestedName,
			pVS->lpBuffer,
			pVS->dwBufferSize,
			nullptr,
			0
			);
}

int ArchivePanel::OnEnterStage(int nStage)
{
	m_OS.nStage = nStage;
	m_OS.Dlg.SetOperation(m_OS.nOperation, m_OS.nStage);

	return 1;
}

int ArchivePanel::OnStartOperation(int nOperation, StartOperationStruct *pOS)
{
	if ( pOS )
	{
		if ( OptionIsOn(pOS->dwFlags, OS_FLAG_TOTALSIZE) )
			m_OS.uTotalSize = pOS->uTotalSize;

		if ( OptionIsOn(pOS->dwFlags, OS_FLAG_TOTALFILES) )
			m_OS.uTotalFiles = pOS->uTotalFiles;

		m_OS.Dlg.SetShowSingleFileProgress(OptionIsOn(pOS->dwFlags, OS_FLAG_SUPPORT_SINGLE_FILE_PROGRESS));
	}

	m_OS.nOperation = nOperation;

	if ( m_OS.nOperation == OPERATION_EXTRACT )
		m_OS.nStage = STAGE_EXTRACTING;

	if ( m_OS.nOperation == OPERATION_ADD )
		m_OS.nStage = STAGE_ADDING;

	if ( m_OS.nOperation == OPERATION_TEST )
		m_OS.nStage = STAGE_TESTING;

	if ( m_OS.nOperation == OPERATION_DELETE )
		m_OS.nStage = STAGE_DELETING;

	m_OS.bFirstFile = true;
	m_OS.overwrite = PROCESS_UNKNOWN;

	m_OS.Dlg.SetOperation(m_OS.nOperation, m_OS.nStage);

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
		{
			m_strPassword = NULL;
			m_bPasswordSet = false;
		}
		else
			m_bPasswordSet = true;
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

	int nOverwrite = PROCESS_OVERWRITE;

	if ( m_OS.nOperation == OPERATION_EXTRACT )
	{
		OverwriteStruct OS;

		OS.pItem = pfs->pItem;
		OS.lpFileName = pfs->lpDestFileName;

		nOverwrite = OnFileAlreadyExists(&OS);
	}

	if ( nOverwrite != PROCESS_CANCEL )
	{
		m_OS.pCurrentItem = pfs?pfs->pItem:NULL;

		m_OS.Dlg.SetSrcFileName(pfs->pItem->lpFileName);
		m_OS.Dlg.SetDestFileName(pfs->lpDestFileName);

		/*if ( m_OS.bFirstFile )
		{
			m_OS.bFirstFile = false;
			m_OS.uTotalProcessedSize = 0;
		}*/

		//if ( !OptionIsOn(m_OS.nMode, OPM_SILENT) )
			m_OS.Dlg.Show();

		if ( m_OS.pCurrentItem )
			m_OS.uFileSize = m_OS.pCurrentItem->nFileSize;
		else
			m_OS.uFileSize = m_OS.uTotalSize;

		m_OS.uProcessedSize = 0;
	}

	return nOverwrite;
}

int ArchivePanel::OnReportError(ReportErrorStruct* pRE)
{
	__debug(_T("Error - %s, %d"), _T("filename"), pRE->nError);

//	m_OS.ErrorList.AddError(pRE->pItem->lpFileName);

	return 0;
}

int ArchivePanel::OnProcessData(ProcessDataStruct* pDS)
{
	double dPercent, dTotalPercent;

	if ( pDS->nMode == PROGRESS_PROCESSED_SIZE )
	{
		m_OS.uTotalProcessedSize += pDS->uProcessedSize;
		m_OS.uProcessedSize += pDS->uProcessedSize;

		if ( m_OS.uFileSize )
			dPercent = (double)m_OS.uProcessedSize/(double)m_OS.uFileSize;
		else
			dPercent = 1;

		if ( m_OS.uTotalSize )
			dTotalPercent = (double)m_OS.uTotalProcessedSize/(double)m_OS.uTotalSize;
		else
			dTotalPercent = 1;

		m_OS.Dlg.SetPercent(dPercent, dTotalPercent);
	}

	if ( pDS->nMode == PROGRESS_DETAILS )
	{
		m_OS.uTotalProcessedSize = pDS->uProcessedBytesTotal;
		m_OS.uProcessedSize += pDS->uProcessedBytesFile;

		dPercent = (double)pDS->uProcessedBytesFile/(double)pDS->uTotalBytesFile;
		dTotalPercent = (double)pDS->uProcessedBytesTotal/(double)pDS->uTotalBytes;

		m_OS.Dlg.SetPercent(dPercent, dTotalPercent);
	}

	//if ( !OptionIsOn(m_OS.nMode, OPM_SILENT) )
		m_OS.Dlg.Show();

	//if ( CheckForEsc () ) //clear screen?
		//return FALSE;

	return TRUE;
}

bool ArchivePanel::GetCommand(int nCommand, string& strCommand)
{
	if ( !m_pArchive )
		return false;

	return m_pManager->GetCommand(m_pArchive->GetFormat(), nCommand, strCommand);
}

int ArchivePanel::Extract(
		const ArchiveItemArray& items, 
		const TCHAR* lpDestDiskPath, 
		bool bWithoutPath
		)
{
	OnStartOperation(OPERATION_EXTRACT, nullptr);

	string strDestDiskPath = lpDestDiskPath;
	AddEndSlash(strDestDiskPath);

	int nResult = m_pArchive->Extract(items, strDestDiskPath, bWithoutPath);

	if ( nResult == RESULT_ERROR )
	{
		string strCommand;

		if ( GetCommand(bWithoutPath?COMMAND_EXTRACT_WITHOUT_PATH:COMMAND_EXTRACT, strCommand) )
		{
			ExecuteStruct ES(items);

			ES.lpCommand = strCommand;
			ES.lpCurrentDiskPath = strDestDiskPath;

			nResult = m_pArchive->ExecuteAsOperation(OPERATION_EXTRACT, ExecuteCommand, &ES);
		}
	}

	if ( nResult == RESULT_ERROR )
		msgError(_T("Extract failed"));

	if ( nResult == RESULT_PARTIAL )
		msgError(_T("Extract succeded partially"));

	if ( nResult == RESULT_CANCEL )
		msgError(_T("Extract was aborted by user"));

//	if ( nResult == RESULT_SUCCESS )
//		msgError(_T("Extract success"));

	return nResult;
}

int ArchivePanel::Delete(const ArchiveItemArray& items)
{
	OnStartOperation(OPERATION_DELETE, nullptr);

	int nResult = m_pArchive->Delete(items);

	if ( nResult == RESULT_ERROR )
	{
		string strCommand;

		if ( GetCommand(COMMAND_DELETE, strCommand) )
		{
			ExecuteStruct ES(items);

			ES.lpCommand = strCommand;

			nResult = m_pArchive->ExecuteAsOperation(OPERATION_DELETE, ExecuteCommand, &ES);
		}
	}

	return nResult;
}

int ArchivePanel::AddFiles(const ArchiveItemArray& items, const TCHAR* lpSourceDiskPath, const TCHAR* lpConfig)
{
	OnStartOperation(OPERATION_ADD, nullptr);

	string strSourceDiskPath = lpSourceDiskPath;
	AddEndSlash(strSourceDiskPath);

	int nResult = m_pArchive->AddFiles(items, strSourceDiskPath, lpConfig);

	if ( nResult == RESULT_ERROR )
	{
		string strCommand;

		if ( GetCommand(COMMAND_ADD, strCommand) )
		{
			ExecuteStruct ES(items);

			ES.lpCommand = strCommand;
			ES.lpCurrentDiskPath = strSourceDiskPath;

			nResult = m_pArchive->ExecuteAsOperation(OPERATION_ADD, ExecuteCommand, &ES);
		}
	}

	return nResult;
}

int ArchivePanel::Test(const ArchiveItemArray& items)
{
	OnStartOperation(OPERATION_TEST, nullptr);

	int nResult = m_pArchive->Test(items);

	if ( nResult == RESULT_ERROR )
	{
		string strCommand;

		if ( GetCommand(COMMAND_TEST, strCommand) )
		{
			ExecuteStruct ES(items);

			ES.lpCommand = strCommand;

			nResult = m_pArchive->ExecuteAsOperation(OPERATION_TEST, ExecuteCommand, &ES);
		}
	}

	return nResult;
}

int ArchivePanel::MakeDirectory(const TCHAR* lpDirectory)
{
	OnStartOperation(OPERATION_ADD, nullptr);

	int nResult = RESULT_ERROR;

	ArchiveItemArray items;

	ArchiveItem *item = items.add();

	item->lpFileName = lpDirectory;
	item->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;

	nResult = m_pArchive->AddFiles(items, _T(""), nullptr);

	if ( nResult == RESULT_ERROR )
	{
		string strCommand;

		if ( GetCommand(COMMAND_ADD, strCommand) )
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

			ExecuteStruct ES(items);

			ES.lpCurrentDiskPath = strTempPath;
			ES.lpCommand = strCommand;

			nResult = m_pArchive->ExecuteAsOperation(OPERATION_ADD, ExecuteCommand, &ES);
		}
	}

	return nResult;
}