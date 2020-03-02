#include "newarc.h"

extern char *lpINIFileName;

ArchivePanel::ArchivePanel (
		pointer_array<Archive*> *pArchives
		)
{
	m_pArchives = pArchives;

	m_pArchive = NULL;//pArchive;
	m_lpCurrentFolder = StrCreate (260);

	m_lpPanelTitle = StrCreate (260);

	m_Editors.create (ARRAY_OPTIONS_DELETE);

	m_bFirstTime = true;
	m_nCurrentMode = 0;
}


ArchivePanel::~ArchivePanel ()
{
	StrFree (m_lpCurrentFolder);
	StrFree (m_lpPanelTitle);

	for (int i = 0; i < m_pArchiveFiles.count(); i++)
		StrFree (m_pArchiveFiles[i].lpPassword);

	m_pArchiveFiles.free ();
	delete m_pArchives;

	m_Editors.free ();
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



bool ArchivePanel::ReadArchive (bool bSilent)
{
	if ( !m_pArchive->pOpenArchive (OM_LIST) )
		return false;

	for (int i = 0; i < m_pArchiveFiles.count(); i++)
		StrFree (m_pArchiveFiles[i].lpPassword);

	m_pArchiveFiles.free();

	dword dwStartTime = GetTickCount ();
	bool bProgressMessage = false;

	HANDLE hScreen = NULL;

	if ( !bSilent )
		hScreen = Info.SaveScreen (0, 0, -1, -1);

	m_pArchiveFiles.create (ARRAY_OPTIONS_SKIP, 256);

	int nResult = E_SUCCESS;

	while ( (nResult == E_SUCCESS) && !CheckForEsc() )
	{
		InternalArchiveItemInfo *item = m_pArchiveFiles.add();

		nResult = m_pArchive->pGetArchiveItem (&item->ItemInfo);

		if ( nResult == E_SUCCESS )
		{
			if ( !bSilent && ((m_pArchiveFiles.count() & 0x1f) == 0) && (GetTickCount ()-dwStartTime > 500) )
			{
				char szFileCount[100];
				char *pMsgs[4];

				pMsgs[0] = _M(MReadArchiveWait);
				pMsgs[1] = _M(MReadArchiveReading);
				pMsgs[2] = m_pArchive->m_lpFileName;
				pMsgs[3] = (char*)&szFileCount;

				FSF.sprintf ((char*)&szFileCount, _M(MReadArchiveFileCount), m_pArchiveFiles.count());

				Info.Message(
						Info.ModuleNumber,
						bProgressMessage?FMSG_KEEPBACKGROUND:0,
						NULL,
						pMsgs,
						4,
						0
					);

				bProgressMessage = true;
			}
		}
		else
			m_pArchiveFiles.remove();

		#ifdef _DEBUG
		if ( nResult == E_UNEXPECTED_EOF )
			__debug ("unexpected eof");

		if ( (nResult != E_EOF) && (nResult != E_SUCCESS) )
			__debug ("%d error", nResult);
		#endif
	}

	m_pArchive->pCloseArchive ();

	if ( hScreen )
		Info.RestoreScreen (hScreen);

	return true;
}

int __stdcall ArchivePanel::pGetFindData(
		PluginPanelItem **pPanelItem,
		int *pItemsNumber,
		int OpMode
		)
{
	bool bSilent = OpMode & (OPM_SILENT|OPM_FIND);

	if ( m_bFirstTime )
	{
		if ( m_pArchives->count() )
		{
			int nResult = 0;

			if ( m_pArchives->count() > 1 && !bSilent)
			{
				FarMenuItem *pItems = (FarMenuItem*)malloc (m_pArchives->count()*sizeof (FarMenuItem));

				memset (pItems, 0, m_pArchives->count()*sizeof (FarMenuItem));

				for (int i = 0; i < m_pArchives->count(); i++)
					FSF.sprintf (pItems[i].Text, "%s", m_pArchives->at(i)->m_pInfo->lpName);

				nResult = Info.Menu (
						Info.ModuleNumber,
						-1,
						-1,
						0,
						FMENU_WRAPMODE,
						_M(MOpenArchiveAs),
						NULL,
						NULL,
						NULL,
						NULL,
						(const FarMenuItem*)pItems,
						m_pArchives->count()
						);

				free (pItems);
			}

			if ( nResult != -1 )
			{
				m_pArchive = m_pArchives->at(nResult);
				m_pArchives->remove(nResult, false); //remove without delete
			}

			while ( m_pArchives->count() )
			{
				m_pArchives->at(0)->m_pPlugin->FinalizeArchive (m_pArchives->at(0));
				m_pArchives->remove(0, false); //remove without delete
			}

			delete m_pArchives;
			m_pArchives = NULL;

			if ( nResult == -1 )
				return FALSE;
		}

		m_bFirstTime = false;
	}

	if ( m_pArchive->WasUpdated () )
	{
		if ( !ReadArchive (bSilent) )
			return FALSE;
	}

	array<PluginPanelItem> pPanelItems (ARRAY_OPTIONS_KEEP, 256);

	bool bAppend;
	bool bIsFolder;

	char *lpFoder = StrDuplicate (m_lpCurrentFolder, 260);

	if ( *lpFoder )
		FSF.AddEndSlash (lpFoder);

	for (int i = 0; i < m_pArchiveFiles.count(); i++)
	{
		PluginPanelItem *pCurrentPanelItem = &m_pArchiveFiles[i].ItemInfo.pi;

		bAppend = true;
		bIsFolder = false;

		char *lpName = StrDuplicate (pCurrentPanelItem->FindData.cFileName);

		if ( !*m_lpCurrentFolder || !FSF.LStrnicmp (lpName, lpFoder, StrLength(lpFoder)) )
		{
			char *lpName2 = lpName;

			if ( *lpFoder )
				lpName2 += strlen(lpFoder);

			char *lpPos;

			if ( lpName2 && (*lpName2 == '\\') )
				lpName2++;

			lpPos = strchr (lpName2, '\\');

			if ( lpPos )
			{
				bIsFolder = true;
				*lpPos = '\0';
			}

			if ( !*lpName2 )
				goto l_1;

			for (int k = 0; k < pPanelItems.count(); k++)
			{
				if ( !FSF.LStricmp (
						lpName2,
						pPanelItems[k].FindData.cFileName
						) )
				{
					//pPanelItems[k].FindData.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY; //ohhha-ha-ha!!!
					//pPanelItems[k].UserData = 0;
					bAppend = false;
					goto l_1;
				}
			}

			if ( bAppend )
			{
				PluginPanelItem *item = pPanelItems.add(*pCurrentPanelItem);

				if ( item )
				{
					strcpy (item->FindData.cFileName, lpName2);

					if ( bIsFolder )
					{
						item->FindData.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
						item->UserData = 0;
					}
					else
						item->UserData = (DWORD_PTR)&m_pArchiveFiles[i];
				}
			}
		}
l_1:

		StrFree (lpName);
	}

	*pPanelItem = pPanelItems.data();
	*pItemsNumber = pPanelItems.count();

	return TRUE;
}

void __stdcall ArchivePanel::pGetOpenPluginInfo (
		OpenPluginInfo *pInfo
		)
{
	pInfo->StructSize = sizeof (OpenPluginInfo);
	pInfo->Flags = OPIF_USEFILTER | OPIF_USEHIGHLIGHTING | OPIF_USESORTGROUPS | OPIF_ADDDOTS;
	pInfo->CurDir = m_lpCurrentFolder;

	if ( m_pArchive )
	{
		memset (m_lpPanelTitle, 0, 260);

		const char *lpFormatName = m_pArchive->m_pInfo?m_pArchive->m_pInfo->lpName:NULL;

		FSF.sprintf (
				m_lpPanelTitle,
				" %s%s%s%s%s ",
				lpFormatName?lpFormatName:"",
				lpFormatName?":":"",
				FSF.PointToName (m_pArchive->m_lpFileName),
				*m_lpCurrentFolder?"\\":"",
				m_lpCurrentFolder
				);

		pInfo->PanelTitle = m_lpPanelTitle;
	}
}

#define PF_FLAG_QUOTE_SPACES		1 //Q
#define PF_FLAG_QUOTE_ALL			2 //q
#define PF_FLAG_USE_BACKSLASH		4 //S
#define PF_FLAG_DIR_NAME_AS_MASK	8 //M
#define PF_FLAG_DIR_NAME_AS_NAME	16 //N
#define PF_FLAG_NAME_ONLY			32 //W
#define PF_FLAG_PATH_ONLY			64 //P
#define PF_FLAG_ANSI_CHARSET		128 //A

#define QUERY_AND_SET_PARAM_FLAG(c, flag) \
	case c: \
		dwFlags |= flag; \
		break;


char *GetFlags (char *p, dword &dwFlags)
{
	dwFlags = 0;

	while ( *p && (*p != ' ') && (*p != '}') )
	{
		switch ( *p )
		{
			QUERY_AND_SET_PARAM_FLAG ('Q', PF_FLAG_QUOTE_SPACES);
			QUERY_AND_SET_PARAM_FLAG ('q', PF_FLAG_QUOTE_ALL);
			QUERY_AND_SET_PARAM_FLAG ('S', PF_FLAG_USE_BACKSLASH);
			QUERY_AND_SET_PARAM_FLAG ('M', PF_FLAG_DIR_NAME_AS_MASK);
			QUERY_AND_SET_PARAM_FLAG ('N', PF_FLAG_DIR_NAME_AS_NAME);
			QUERY_AND_SET_PARAM_FLAG ('W', PF_FLAG_NAME_ONLY);
			QUERY_AND_SET_PARAM_FLAG ('P', PF_FLAG_PATH_ONLY);
			QUERY_AND_SET_PARAM_FLAG ('A', PF_FLAG_ANSI_CHARSET);
		}

		p++;
	}

	return p;
}

#define QUERY_AND_SET_PARAM(string) \
	p++; \
	p = GetFlags (p, dwFlags); \
	if ( string && *string ) \
	{\
		strcat (lpResult, string); \
		n += StrLength(string); \
		bEmpty = false; \
	};\
	break;

struct ParamStruct {

	char *lpArchiveName;
	char *lpShortArchiveName;
	char *lpPassword;
	char *lpAdditionalCommandLine;

	char *lpTempPath;
	char *lpCurrentArchiveFolder;
	char *lpListFileName;
};

void ProcessName (
		const char *lpFileName,
		char *lpResult,
		int dwFlags,
		bool bForList
		)
{
	strcpy (lpResult, lpFileName);

	if ( OptionIsOn (dwFlags, PF_FLAG_PATH_ONLY) )
		CutToSlash (lpResult);

	if ( bForList )
	{
		if ( OptionIsOn (dwFlags, PF_FLAG_NAME_ONLY) )
			if ( !OptionIsOn (dwFlags, PF_FLAG_PATH_ONLY) )
				strcpy (lpResult, FSF.PointToName (lpFileName));

		if ( OptionIsOn (dwFlags, PF_FLAG_USE_BACKSLASH) )
			for (size_t i = 0; i < strlen (lpResult); i++ )
				if ( lpResult[i] == '\\' )
					lpResult[i] = '/';

		if ( OptionIsOn (dwFlags, PF_FLAG_QUOTE_SPACES) )
			FSF.QuoteSpaceOnly (lpResult);

		if ( OptionIsOn (dwFlags, PF_FLAG_QUOTE_ALL) );
			//NOT SUPPORTED YET!
	}

	if ( OptionIsOn (dwFlags, PF_FLAG_ANSI_CHARSET) )
		OemToChar (lpResult, lpResult);
}

void CreateListFile (
		const char *lpListFileName,
		PluginPanelItem *pItems,
		int nItemsNumber,
		int dwFlags
		)
{
	HANDLE hListFile = CreateFile (
			lpListFileName,
			GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			0,
			NULL
			);

	if ( hListFile != INVALID_HANDLE_VALUE )
	{
		char *lpProcessedString = StrCreate (260);
		dword dwWritten;

		for (int i = 0; i < nItemsNumber; i++)
		{
			ProcessName (
					pItems[i].FindData.cFileName,
					lpProcessedString,
					dwFlags,
					true
					);

			const char *lpCRLF = "\r\n";

			if ( OptionIsOn (dwFlags, PF_FLAG_DIR_NAME_AS_MASK) )
			{
				if ( OptionIsOn (pItems[i].FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
				{
					FSF.AddEndSlash (lpProcessedString);
					strcat (lpProcessedString, "*.*");

					WriteFile (hListFile, lpProcessedString, StrLength(lpProcessedString), &dwWritten, NULL);
					WriteFile (hListFile, lpCRLF, 2, &dwWritten, NULL);

					CutTo (lpProcessedString, '\\', true);
				}
			}

			WriteFile (hListFile, lpProcessedString, StrLength(lpProcessedString), &dwWritten, NULL);
			WriteFile (hListFile, lpCRLF, 2, &dwWritten, NULL);
		}

		StrFree (lpProcessedString);

		CloseHandle (hListFile);
	}
}


#define PE_SUCCESS		0
#define PE_MORE_FILES	1

int ParseString (
		const char *lpString,
		char *lpResult,
		PluginPanelItem *pItems,
		int nItemsNumber,
		ParamStruct *pParam,
		int &pStartItemNumber
		)
{
	char *p = (char*)lpString;

	int bOnlyIfExists = 0;

	dword dwFlags;

	bool bHaveList = false;
	bool bHaveAdditionalOptions = false;

	bool bEmpty = false;

	int n = 0;
	int nSavedPos = 0;

	char *lpProcessedName = StrCreate (260);

	int nResult = PE_SUCCESS;

	while ( *p )
	{
		switch ( *p )
		{

		case '{':

        	bOnlyIfExists++;
			p++;

			bEmpty = true;
			nSavedPos = n-1;
			break;

		case '}':

			bOnlyIfExists--;
			p++;

			if ( bEmpty )
			{
				lpResult[nSavedPos] = '\0';
				n = nSavedPos;
			}

			break;

		case '%':

			if ( *(p+1) && (*(p+1) == '%') )
			{
				p += 2;

				switch ( *p )
				{

				case 'A':
					p++;
					p = GetFlags (p, dwFlags);

					if ( pParam->lpArchiveName )
					{
						ProcessName (
								pParam->lpArchiveName,
								lpProcessedName,
								dwFlags,
								false
								);

						strcat (lpResult, lpProcessedName);
						n += StrLength (lpProcessedName);
						bEmpty = false;
					};
					break;

				case 'a':

					p++;
					p = GetFlags (p, dwFlags);

					if ( pParam->lpShortArchiveName )
					{
						ProcessName (
								pParam->lpShortArchiveName,
								lpProcessedName,
								dwFlags,
								false
								);

						strcat (lpResult, lpProcessedName);
						n += StrLength (lpProcessedName);
						bEmpty = false;
					};

					break;

				case 'W':
					QUERY_AND_SET_PARAM (pParam->lpTempPath);

				case 'P':
					QUERY_AND_SET_PARAM (pParam->lpPassword);

				case 'R':
					QUERY_AND_SET_PARAM (pParam->lpCurrentArchiveFolder);

				case 'S':
					bHaveAdditionalOptions = true;
					QUERY_AND_SET_PARAM (pParam->lpAdditionalCommandLine);

				case 'L':
				case 'l':

					p++;
					p = GetFlags (p, dwFlags);

					if ( !bHaveList && pParam->lpListFileName )
					{
						bHaveList = true;

						CreateListFile (
								pParam->lpListFileName,
								pItems,
								nItemsNumber,
								dwFlags
								);

						strcat (lpResult, pParam->lpListFileName);
						n += StrLength (pParam->lpListFileName);
						bEmpty = false;
					};

					break;

				case 'f':

					p++;
					p = GetFlags (p, dwFlags);

					if ( pItems[pStartItemNumber].FindData.cFileName )
					{
						ProcessName (
								pItems[pStartItemNumber].FindData.cFileName,
								lpProcessedName,
								dwFlags,
								true
								);

						strcat (lpResult, lpProcessedName);
						n += StrLength (lpProcessedName);

						bEmpty = false;

						pStartItemNumber++;

						if ( pStartItemNumber != nItemsNumber )
							nResult = PE_MORE_FILES;
					};

					break;

				default:
					p++;
					break;
				}
			}
			else
			{
				lpResult[n] = *p;
				lpResult[n+1] = '\0';

				n++;
				p++;
			}

			break;

		default:

			lpResult[n] = *p;
			lpResult[n+1] = '\0';

			n++;
			p++;
		}
	}

	StrFree (lpProcessedName);

	if ( pParam->lpAdditionalCommandLine &&
		 *pParam->lpAdditionalCommandLine &&
		 !bHaveAdditionalOptions )
	{
		strcat (lpResult, " ");
		strcat (lpResult, pParam->lpAdditionalCommandLine);
	}

	return nResult;
}

bool IsFileInFolder (const char *lpCurrentPath, const char *lpFileName)
{
	int nLength = StrLength (lpCurrentPath);

	return !FSF.LStrnicmp (lpCurrentPath, lpFileName, nLength);
}

void GetArchiveItemsToProcess (
		ArchivePanel *pPanel,
		PluginPanelItem *pPanelItems,
		int nItemsNumber,
		PluginPanelItem **pItemsToProcess,
		int *pItemsToProcessNumber,
		OperationStruct *pOS
		)
{
	array<PluginPanelItem> pResult(ARRAY_OPTIONS_KEEP, 256);

	memset (pOS, 0, sizeof (OperationStruct));

	PluginPanelItem *item = NULL;

	for (int i = 0; i < nItemsNumber; i++)
	{
		InternalArchiveItemInfo *pItemInfo = (InternalArchiveItemInfo*)pPanelItems[i].UserData;
		item = pResult.add();

		if ( pItemInfo )
		{
			memcpy (item, &pItemInfo->ItemInfo.pi, sizeof (PluginPanelItem));
		}
		else
		{
			//хёыш ь√ ё■фр яюярыш - є эрё яюїюцх яЁюсыхь√.

			char *lpFullName = StrCreate (260);

			if ( *pPanel->m_lpCurrentFolder )
			{
				strcpy (lpFullName, pPanel->m_lpCurrentFolder);
				FSF.AddEndSlash (lpFullName);
			}

			strcat (lpFullName, pPanelItems[i].FindData.cFileName);

			memcpy (item, &pPanelItems[i], sizeof (PluginPanelItem));

			item->UserData = 0;
			strcpy ((char*)&item->FindData.cFileName, lpFullName);

			StrFree (lpFullName);
		}

        pOS->uTotalSize += pPanelItems[i].FindData.nFileSizeHigh*0x100000000ull+pPanelItems[i].FindData.nFileSizeLow;

		if ( OptionIsOn(pPanelItems[i].FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
		{
			char *lpPath = StrDuplicate (pPanel->m_lpCurrentFolder, 260);

			if ( *pPanel->m_lpCurrentFolder )
				strcat (lpPath, "\\");

			strcat (lpPath, pPanelItems[i].FindData.cFileName);
			strcat (lpPath, "\\");

			for (int k = 0; k < pPanel->m_pArchiveFiles.count(); k++)
			{
				PluginPanelItem *pCurrentPanelItem = &pPanel->m_pArchiveFiles[k].ItemInfo.pi;

				if ( IsFileInFolder (lpPath, pCurrentPanelItem->FindData.cFileName) )
				{
					item = pResult.add();
					memcpy (item, pCurrentPanelItem, sizeof (PluginPanelItem));

					pOS->uTotalSize += pCurrentPanelItem->FindData.nFileSizeHigh*0x100000000ull+pCurrentPanelItem->FindData.nFileSizeLow;
				}
			}

			StrFree (lpPath);
		}

	}

	*pItemsToProcessNumber = pResult.count();
	*pItemsToProcess = pResult.data();

	pOS->uTotalFiles = pResult.count();

	//__debug ("total - %I64u", pOS->uTotalSize);
}


void ExecuteCommand (
		int nCommand,
		const char *lpCommand,
		const char *lpArchiveFileName,
		const char *lpArchivePassword,
		const char *lpArchiveCurrentFolder,
		const char *lpAdditionalCommandLine,
		PluginPanelItem *pItems,
		int nItemsNumber
		)
{
	ParamStruct psParam;

	if ( *lpCommand )
	{
		char *lpExecuteString = StrCreate (260);
		char *lpFileName = StrDuplicate (lpArchiveFileName, 260);
		char *lpListFileName = StrCreate (260);
		char *lpTempPath = StrCreate (260);


		GetTempPath (260, lpTempPath);
		FSF.MkTemp (lpListFileName, "NALT");

		FSF.QuoteSpaceOnly (lpTempPath);
		FSF.QuoteSpaceOnly (lpListFileName);
		FSF.QuoteSpaceOnly (lpFileName);

		psParam.lpArchiveName = lpFileName;
		psParam.lpShortArchiveName = lpFileName;
		psParam.lpPassword = (char*)lpArchivePassword;
		psParam.lpCurrentArchiveFolder = (char*)lpArchiveCurrentFolder;
		psParam.lpAdditionalCommandLine = (char*)lpAdditionalCommandLine;
		psParam.lpTempPath = lpTempPath;
		psParam.lpListFileName = lpListFileName;

		HANDLE hScreen = Info.SaveScreen (0, 0, -1, -1);

		int nStartItemNumber = 0;

/*		char *pMsgs[2];

		pMsgs[0] = "";
		pMsgs[1] = "Выполняется архивная операция";

		Info.Message (
				Info.ModuleNumber,
				0,
				NULL,
				(const char* const*)pMsgs,
				2,
				0
				);

		Info.Text(0, 0, 0, NULL);*/

		CONSOLE_SCREEN_BUFFER_INFO csbi;

		GetConsoleScreenBufferInfo (GetStdHandle (STD_OUTPUT_HANDLE), &csbi);

		char Blank[1024];

		FSF.sprintf (Blank,"%*s", csbi.dwSize.X, "");

		for (int Y = 0; Y < csbi.dwSize.Y; Y++)
			Info.Text(0, Y, 0x07, Blank);

		Info.Text(0,0,0,NULL);

		COORD C;
		C.X = 0;
		C.Y = csbi.dwCursorPosition.Y;
		SetConsoleCursorPosition (GetStdHandle (STD_OUTPUT_HANDLE), C);

		while ( true )
		{
			int nResult = ParseString (
					lpCommand,
					lpExecuteString,
					pItems,
					nItemsNumber,
					&psParam,
					nStartItemNumber
					);

			if ( (nResult == PE_SUCCESS) || (nResult == PE_MORE_FILES) )
			{
				PROCESS_INFORMATION pInfo;
				STARTUPINFO sInfo;

				memset (&sInfo, 0, sizeof (STARTUPINFO));
				sInfo.cb = sizeof (STARTUPINFO);

/////
/*	sInfo.dwFlags = STARTF_USESTDHANDLES;

	sInfo.hStdError = NULL;
	sInfo.hStdInput = NULL;
	sInfo.hStdOutput = NULL;*/
/////
				FSF.ExpandEnvironmentStr(lpExecuteString,lpExecuteString,260);

				if ( CreateProcess (
						NULL,
						lpExecuteString,
						NULL,
						NULL,
						TRUE,
						0,
						NULL,
						NULL,
						&sInfo,
						&pInfo
						) )
				{
					WaitForSingleObject(pInfo.hProcess, INFINITE);

					CloseHandle (pInfo.hProcess);
					CloseHandle (pInfo.hThread);

					Info.Control (INVALID_HANDLE_VALUE, FCTL_SETUSERSCREEN, NULL);
				}
			}

			if ( nResult != PE_MORE_FILES )
				break;
		}

		Info.RestoreScreen (NULL);
		Info.RestoreScreen (hScreen);

		DeleteFile (lpListFileName); //WARNING!!!

		StrFree (lpTempPath);
		StrFree (lpFileName);
		StrFree (lpListFileName);

		StrFree (lpExecuteString);

	}
}


void ArchivePanel::pExecuteCommand (
		int nCommand,
		const char *lpArchivePassword,
		const char *lpAdditionalCommandLine,
		PluginPanelItem *pItems,
		int nItemsNumber
		)
{
	char *lpCommand = StrCreate (260);

	m_pArchive->pGetDefaultCommand (
			nCommand,
			lpCommand
			);

	ExecuteCommand (
			nCommand,
			lpCommand,
			m_pArchive->m_lpFileName,
			lpArchivePassword,
			m_lpCurrentFolder,
			lpAdditionalCommandLine,
			pItems,
			nItemsNumber
			);

	StrFree (lpCommand);
}


//struct ArchiveTemplate {
//	int nID; // format ID
//	const char *lpAdditionalParams;
//};


void SetFormatTitle (FarDialogHandler *D)
{
	char *lpFormat = StrCreate (260);
	char *lpTitle = StrCreate (260);

	if ( D->GetCheck (5) )
		D->GetTextPtr (7, lpFormat);
	else
		D->GetTextPtr (12, lpFormat);

	FSF.sprintf (lpTitle, _M(MAddToTitle), lpFormat);

	D->SetTextPtr (0, lpTitle);

	StrFree (lpFormat);
	StrFree (lpTitle);
}

struct ArchiveTemplate {
	char *lpName;
	char *lpParams;
	GUID uid;
};

pointer_array <ArchiveTemplate*> Templates;

LONG_PTR __stdcall hndAddEditTemplate (
		FarDialogHandler *D,
		int nMsg,
		int nParam1,
		LONG_PTR nParam2
		)
{
	ArchiveTemplate *ptpl = (ArchiveTemplate*)D->GetDlgData();

	bool bAdd = (ptpl->lpName == NULL);

	if ( nMsg == DN_INITDIALOG )
	{
		int curpos = -1;
		int pos = 0;

		for (int i = 0; i < Plugins.count(); i++)
		{
			for (int j = 0; j < Plugins[i]->m_ArchivePluginInfo.nFormats; j++)
			{
				int index = D->ListAddStr (5, Plugins[i]->m_ArchivePluginInfo.pFormatInfo[j].lpName);

				GUID uid = Plugins[i]->m_ArchivePluginInfo.pFormatInfo[j].uid;
				D->ListSetDataEx (5, index, &uid, sizeof (uid));

				if ( !bAdd && (ptpl->uid == uid) )
					curpos = pos;

				pos++;
			}

		}

		if ( !bAdd && (curpos != -1) )
		{
			FarListPos arcPos;

			arcPos.SelectPos = curpos;
			arcPos.TopPos = -1;

			D->ListSetCurrentPos (5, &arcPos);

			D->SetTextPtr (2, ptpl->lpName);
			D->SetTextPtr (7, ptpl->lpParams);
		}
	}

	if ( nMsg == DN_CLOSE )
	{
		if ( nParam1 == 9 )
		{
			free (ptpl->lpName);
			free (ptpl->lpParams);

			ptpl->lpName = (char*)malloc (D->GetTextLength(2)+1);
			D->GetTextPtr (2, ptpl->lpName);

			ptpl->lpParams = (char*)malloc (D->GetTextLength(7)+1);
			D->GetTextPtr (7, ptpl->lpParams);

			FarListPos pos;

			D->ListGetCurrentPos (5, &pos);

			if ( pos.SelectPos != -1 )
				ptpl->uid = *(GUID*)D->ListGetData (5, pos.SelectPos);
		}
	}

	return D->DefDlgProc (nMsg, nParam1, nParam2);
}

void LoadTemplates ()
{
	char *szNames = StrCreate (4096); //я знаю, что это ламерство, но я вообще не собираюсь хранить шаблоны в ини. ???

	Templates.create (ARRAY_OPTIONS_DELETE);

	if ( GetPrivateProfileSectionNames(szNames, 4096, lpINIFileName) )
	{
		const char *lpName = szNames;

		while ( *lpName )
		{
			ArchiveTemplate *pTemplate = new ArchiveTemplate;

			pTemplate->lpName = StrDuplicate (lpName);

			Templates.add (pTemplate);

			lpName += strlen (lpName)+1;
		}
	}

	for (int i = 0; i < Templates.count(); i++)
	{
		Templates[i]->lpParams = StrCreate (260);

		GetPrivateProfileString (
				Templates[i]->lpName,
				"Params",
				"",
				Templates[i]->lpParams,
				260,
				lpINIFileName
				);

		char szGUID[64];

		GetPrivateProfileString (
				Templates[i]->lpName,
				"uid",
				"",
				szGUID,
				64,
				lpINIFileName
				);

        Templates[i]->uid = STR2GUID(szGUID);
	}

	StrFree (szNames);
}

void SaveTemplates ()
{
	int iCount = Templates.count();

	DeleteFile (lpINIFileName);

	if ( iCount > 0 )
	{
		//char szArchiver[8];

		for ( int i = 0; i < iCount; i++ )
		{
			WritePrivateProfileString (
					Templates[i]->lpName,
					"Params",
					Templates[i]->lpParams,
					lpINIFileName
					);

			WritePrivateProfileString (
					Templates[i]->lpName,
					"uid",
					GUID2STR(Templates[i]->uid),
					lpINIFileName
					);
		}
	}
}

bool dlgAddEditTemplate (ArchiveTemplate *ptpl, bool bAdd)
{
	FarDialog D(-1, -1, 55, 11);

	D.DoubleBox (3, 1, 51, 9, ( bAdd ) ? "Добавить шаблон" : "Изменить шаблон"); //0

	D.Text (5, 2, "Имя шаблона"); //1
	D.Edit (5, 3, 45); //2

	D.Separator (4); //3

	D.Text (5, 5, "Архиватор"); //4
	D.ComboBox (5, 6, 15, NULL, 0); //5
	D.SetFlags (DIF_DROPDOWNLIST);

//	const ArchiveFormatInfo *info = pPlugin?pPlugin->GetArchiveFormatInfo (ptpl->uid):NULL;

//	if ( info && OptionIsOn(info->dwFlags, AFF_SUPPORT_INTERNAL_CONFIG) )
//		D.Button (22, 5, "Internal config");
//	else
//	{
		D.Text (22, 5, "Дополнительные параметры"); //6
		D.Edit (22, 6, 27); //7
//	}

	D.Separator(7); //8

	D.Button(-1, 8, ( bAdd ) ? "Добавить" : "Применить"); //9
	D.DefaultButton ();

	D.Button(-1, 8, "Отменить"); //10


	if ( D.ShowEx (hndAddEditTemplate, ptpl) == 9 )
		return true;

	return false;
}

void SetTemplate (FarDialogHandler *D, ArchiveTemplate *ptpl = NULL)
{
	static bool bInit = false;

	if (!bInit)
	{
		bInit = true;

		FarListPos arcPos;
		FarListInfo li;

		int iCount = Templates.count();

		D->ListInfo(7, &li);

		if ( li.ItemsNumber != iCount )
		{
			D->ListDelete (7, NULL);

			if ( iCount )
			{
				int pos = ptpl?-1:li.SelectPos;

				for (int i = 0; i < iCount; i++)
				{
					D->ListAddStr (7, Templates[i]->lpName);

					if ( Templates[i] == ptpl )
						pos = i;
				}

				arcPos.TopPos = -1;
				arcPos.SelectPos = pos;
				D->ListSetCurrentPos(7, &arcPos);
			}
			else
				D->SetTextPtr (7, "");
		}

		D->RedrawDialog();

		bInit = false;
	}

	bool bEnable = (Templates.count() > 0);
	int nFocus = D->GetFocus ();

	D->Enable (5, bEnable);
	D->Enable (7, bEnable);
	D->Enable (9, bEnable);
	D->Enable (10, bEnable);

	if ( !bEnable && ((nFocus == 9) || (nFocus == 10)) )
		D->SetFocus (8);

	if ( !bEnable )
		D->SetCheck (6, BSTATE_CHECKED);
}



LONG_PTR __stdcall hndModifyCreateArchive (
		FarDialogHandler *D,
		int nMsg,
		int nParam1,
		LONG_PTR nParam2
		)
{
	if ( nMsg == DN_INITDIALOG )
	{
		for (int i = 0; i < Plugins.count(); i++)
		{
			for (int j = 0; j < Plugins[i]->m_ArchivePluginInfo.nFormats; j++)
			{
				int index = D->ListAddStr (12, Plugins[i]->m_ArchivePluginInfo.pFormatInfo[j].lpName);

				GUID uid = Plugins[i]->m_ArchivePluginInfo.pFormatInfo[j].uid;
				D->ListSetDataEx (12, index, &uid, sizeof(uid));
			}

		}

		for (int i = 0; i < Templates.count(); i++)
			D->ListAddStr (7, Templates[i]->lpName);

		SetTemplate (D);
	}

	if ( nMsg == DN_BTNCLICK )
	{
		if ( (nParam1 == 5) && (nParam2 != 0) )
			SetTemplate (D);

		if ( nParam1 == 8 )
		{
			ArchiveTemplate *ptpl = new ArchiveTemplate;
			memset (ptpl, 0, sizeof (ArchiveTemplate));

			if ( dlgAddEditTemplate (ptpl, true) )
			{
				Templates.add (ptpl);

				SetTemplate (D, ptpl);
				SaveTemplates ();
			}
			else
				delete ptpl;
		}

		if ( (nParam1 == 9) && Templates.count() )
		{
			int iPos = D->ListGetCurrentPos (7, NULL);

			Templates.remove(iPos);

			SetTemplate (D);
			SaveTemplates ();
		}

		if ( (nParam1 == 10) && Templates.count() )
		{
			int iPos = D->ListGetCurrentPos (7, NULL);

			if ( dlgAddEditTemplate (Templates[iPos], false) )
			{
				SetTemplate (D);
				SaveTemplates ();
			}
		}
	}

	if ( nMsg == DN_GOTFOCUS )
	{
		if ( nParam1 == 7 )
		{
			D->SetCheck (5, BSTATE_CHECKED);
			return 0;
		}

		if ( (nParam1 == 12) || (nParam1 == 14) )
		{
			D->SetCheck (6, BSTATE_CHECKED);
			return 0;
		}
	}

	if ( nMsg == DN_DRAWDIALOG )
		SetFormatTitle (D);


	if ( nMsg == DN_EDITCHANGE )
	{
		if ((nParam1 == 17 ) || ( nParam1 == 19 ) )
		{
			char *lpPassword1 = StrCreate (260);
			char *lpPassword2 = StrCreate (260);

			D->GetTextPtr (17, lpPassword1);
			D->GetTextPtr (19, lpPassword2);

			if ( !*lpPassword1 && !*lpPassword2 )
				D->SetTextPtr (21, "");
			else
			{
				if ( !strcmp (
						lpPassword1,
						lpPassword2
						) )
					D->SetTextPtr (21, "(пароли совпадают)");
				else
					D->SetTextPtr (21, "(пароли НЕ совпадают)");
			}

			StrFree (lpPassword1);
			StrFree (lpPassword2);
		}

		if (nParam1 == 7 )
			SetTemplate (D);
	}

	if ( nMsg == DN_CLOSE )
	{
//*************************************/
		if ( nParam1 == 27 )
		{
  			int pos = D->ListGetCurrentPos (12, NULL);

  			if ( pos != -1 )
  			{
				GUID uid = *(GUID*)D->ListGetData (12, pos);//Templates[pos];

				ArchivePlugin *pPlugin = GetPluginFromUID (uid);

				if ( pPlugin )
				{
					ConfigureFormatStruct CF;

					CF.uid = uid;

					if ( pPlugin->m_pfnPluginEntry (FID_CONFIGUREFORMAT, (void*)&CF) == NAERROR_SUCCESS )
					{
					}
				}
			}

			return false;
		}
//*************************************/

		if ( nParam1 == 25 )
		{
			bool bResult;

	  		char *lpPassword1 = StrCreate (260);
  			char *lpPassword2 = StrCreate (260);

  			D->GetTextPtr (17, lpPassword1);
	  		D->GetTextPtr (19, lpPassword2);

  			bResult = !strcmp (
  					lpPassword1,
  					lpPassword2
  					);

	  		StrFree (lpPassword1);
  			StrFree (lpPassword2);

  			ArchiveTemplate *ptpl = (ArchiveTemplate*)D->GetDlgData ();

  			if ( D->GetCheck (5) == BSTATE_CHECKED )
  			{
	  			int pos = D->ListGetCurrentPos (7, NULL);

	  			if ( pos != -1 )
	  			{
					ArchiveTemplate *psrc = Templates[pos];

		  			ptpl->lpName = StrDuplicate (psrc->lpName);
	  				ptpl->lpParams = StrDuplicate (psrc->lpParams);
	  				ptpl->uid = psrc->uid;
				}
  			}
  			else
  			{
				int pos = D->ListGetCurrentPos (12, NULL);

				if ( pos != -1 )
				{
	  				ptpl->lpName = NULL;

  					ptpl->lpParams = (char*)malloc (D->GetTextLength(14)+1);
  					D->GetTextPtr (14, ptpl->lpParams);

  					ptpl->uid = *(GUID*)D->ListGetData (12, pos);
				}
			}

  			return bResult;
		}
	}

	return D->DefDlgProc (nMsg, nParam1, nParam2);
}

void dlgModifyCreateArchive (ArchivePanel *pPanel)
{
	PanelInfo pnInfo;

	LoadTemplates ();

	Info.Control (INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, &pnInfo);

	char *lpArchiveName = StrCreate (260);

	if ( !pnInfo.SelectedItemsNumber || (pnInfo.SelectedItemsNumber > 1) )
		strcpy (lpArchiveName, FSF.PointToName (pnInfo.CurDir));
	else
	{
		strcpy (lpArchiveName, FSF.PointToName (pnInfo.SelectedItems->FindData.cFileName));
		if ( !(pnInfo.SelectedItems->FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) )
			CutTo (lpArchiveName, '.', true);
	}

	FarDialog D (-1, -1, 75, 20);

	D.DoubleBox (3, 1, 71, 18, NULL); //0

	D.Text (5, 2, "&Добавить к архиву");//1
	D.Edit (5, 3, 65, lpArchiveName, AUTO_LENGTH, "sdfas"); //2

	D.Separator (4); //3

	D.Text (5, 5, "Настройки архиватора:"); //4

	D.RadioButton (6, 6, true, "Шаблон:"); //5
	D.RadioButton (6, 8, false, "Непосредственные настройки:"); //6

	D.ComboBox (9, 7, 48, NULL, 0); //7
	D.SetFlags (DIF_DROPDOWNLIST);

	D.Button (59, 7, "[+]"); //8
	D.SetFlags(DIF_BTNNOCLOSE);

	D.Button (63, 7, "[-]"); //9
	D.SetFlags(DIF_BTNNOCLOSE);

	D.Button (67, 7, "[*]"); //10
	D.SetFlags(DIF_BTNNOCLOSE);

	D.Text (9, 9, "Архиватор:");//11
	D.ComboBox (19, 9, 15, NULL, 0, "123");//12
///	D->ListBox (50, 5, 65, 10, NULL);
	D.SetFlags (DIF_DROPDOWNLIST);

	D.Text (37, 9, "Д&оп. параметры:");//13
	D.Edit (52, 9, 18, NULL, AUTO_LENGTH, "adsaf");//14

	D.Separator (10); //15

	D.Text (5, 11, "&Пароль"); //16
	D.PswEdit (5, 12, 32); //17

	D.Text (38, 11, "&Подтверждение пароля"); //18
	D.PswEdit (38, 12, 32); //19

	D.Separator (13); //20
	D.Text (48, 13); //21

	D.CheckBox (5, 14, false, "Точное соответствие имени файла"); //22
	D.CheckBox (5, 15, false, "Каждый элемент в отдельный архив"); //23

	D.Separator (16); //24

	D.Button (-1, 17, "Добавить"); //25
	D.DefaultButton ();

	D.Button (-1, 17, "Отменить"); //26

	D.Button (-1, 17, "[?]"); //27

	ArchiveTemplate tpl;
	memset (&tpl, 0, sizeof (tpl));

	if ( D.ShowEx (
			hndModifyCreateArchive,
			&tpl
			) == 25 )
	{
		char *lpCommand = StrCreate (260);
		char *lpPassword = StrCreate (260);
		char *lpAdditionalCommandLine = StrCreate (260);

		strcpy (lpArchiveName, D[2].Data);
		strcpy (lpPassword, D[17].Data);
		strcpy (lpAdditionalCommandLine, tpl.lpParams);

		ArchivePlugin *pPlugin = GetPluginFromUID (tpl.uid);
		const ArchiveFormatInfo *info = pPlugin->GetArchiveFormatInfo (tpl.uid);

		bool bSeparately = D[23].Selected && (pnInfo.SelectedItemsNumber > 1);
		int	nCount = (bSeparately)?pnInfo.SelectedItemsNumber:1;

		for (int item = 0; item < nCount; item++)
		{
			if ( bSeparately )
			{
				strcpy (lpArchiveName, FSF.PointToName (pnInfo.SelectedItems[item].FindData.cFileName));
				if ( !OptionIsOn (pnInfo.SelectedItems[item].FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
					CutTo (lpArchiveName, '.', true);
			}

			strcat (lpArchiveName, ".");

			if ( !D[22].Selected )
				strcat (lpArchiveName, info->lpDefaultExtention);

			bool bResult = false;

			if ( OptionIsOn (info->dwFlags, AFF_SUPPORT_INTERNAL_CREATE) )
			{
				Archive *pArchive = pPlugin->CreateArchive (info->uid, lpArchiveName);

				if ( pArchive )
				{
					if ( pArchive->pOpenArchive (OM_ADD) ) //!!! пока надо, хотя не уверен насчет OpMode, возможно уберу.
					{
						pPanel->pPutFilesNew (
							pArchive,
							pnInfo.SelectedItems+item,
							(bSeparately)?1:pnInfo.SelectedItemsNumber,
							0,
							0
							);

						bResult = true;

						pArchive->pCloseArchive (); //!!! а надо ли?
					}

					pPlugin->FinalizeArchive (pArchive);
				}
			}

			if ( !bResult )
			{
				GetDefaultCommandStruct GDC;

				GDC.uid = info->uid;
				GDC.nCommand = COMMAND_ADD;
				GDC.lpCommand = lpCommand;

				if ( pPlugin->m_pfnPluginEntry (FID_GETDEFAULTCOMMAND, (void*)&GDC) == NAERROR_SUCCESS )
				{
					ExecuteCommand (
							COMMAND_ADD, //????
							lpCommand,
							lpArchiveName,
							lpPassword,
							NULL,
							lpAdditionalCommandLine,
							pnInfo.SelectedItems+item,
						(bSeparately)?1:pnInfo.SelectedItemsNumber
							);
				}
			}
		}

		StrFree (lpPassword);
		StrFree (lpCommand);
		StrFree (lpAdditionalCommandLine);
	}

	StrFree (tpl.lpName); //BUGBUG
	StrFree (tpl.lpParams);

	StrFree (lpArchiveName);

	Templates.free ();
}

struct ScanStruct {
	const char *lpSourcePath;
	array<PluginPanelItem> *files;
};



int __stdcall ScanDirectory (
		const WIN32_FIND_DATA *fdata,
		const char *lpFullName,
		ScanStruct *pSS
		)
{
	char szFileNameCopy[MAX_PATH];
	const char *lpFileName = lpFullName+strlen(pSS->lpSourcePath);

	if ( (*lpFileName == '\\') || (*lpFileName == '/') )
		lpFileName++;

	strcpy (szFileNameCopy, lpFileName);

	PluginPanelItem *pitem = pSS->files->add();

	memcpy (&pitem->FindData, fdata, sizeof (WIN32_FIND_DATA));
	strcpy ((char*)&pitem->FindData.cFileName, szFileNameCopy); //???

	return TRUE;
}

int __stdcall ArchivePanel::pPutFilesNew(
		Archive *pArchive,
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int Move,
		int OpMode
        )
{
	m_pArchive = pArchive;

	int nResult = pPutFiles(PanelItem,ItemsNumber,Move,OpMode);

	m_pArchive = NULL;

	return nResult;
}

int __stdcall ArchivePanel::pPutFiles(
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int Move,
		int OpMode
        )
{
	if ( m_pArchive == NULL )
		dlgModifyCreateArchive (this);
	else
	{
		bool bExternal = !OptionIsOn (m_pArchive->m_pInfo->dwFlags, AFF_SUPPORT_INTERNAL_ADD);

		if ( bExternal )
		{
			m_pArchive->pNotify ((HANDLE)this, NOTIFY_EXTERNAL_ADD_START, NULL);

			PanelInfo pnInfo;

			Info.Control (INVALID_HANDLE_VALUE, FCTL_GETPANELSHORTINFO, &pnInfo);

			SetCurrentDirectory (pnInfo.CurDir);

			pExecuteCommand (
					COMMAND_ADD,
					NULL,
					NULL,
					PanelItem,
					ItemsNumber
					);

			m_pArchive->pNotify ((HANDLE)this, NOTIFY_EXTERNAL_ADD_END, NULL);
		}
		else
		{
			char szCurDir[MAX_PATH];
			GetCurrentDirectory (MAX_PATH, szCurDir);

			array<PluginPanelItem> files(ARRAY_OPTIONS_SKIP, 256);

			for (int i = 0; i < ItemsNumber; i++)
			{
				PluginPanelItem *pitem = files.add(PanelItem[i]);

				if ( (pitem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
				{
					char *lpFullName = StrDuplicate (szCurDir, MAX_PATH);

					FSF.AddEndSlash (lpFullName);
					strcat (lpFullName, PanelItem[i].FindData.cFileName);

					ScanStruct SS;

					SS.lpSourcePath = (const char*)&szCurDir;
					SS.files = &files;

					FSF.FarRecursiveSearch (lpFullName, "*.*", (FRSUSERFUNC)ScanDirectory, FRS_RECUR, &SS);

					StrFree (lpFullName);
				}

			}

			PanelInfo pnThis;
			bool bPanel = Info.Control (this, FCTL_GETPANELSHORTINFO, &pnThis);

			m_pArchive->pAddFiles (
					(const char*)&szCurDir,
					bPanel?(const char*)&pnThis.CurDir:NULL,
					files.data(),
					files.count()
					);

			files.free();
		}
	}

	return TRUE;
}


bool dlgGetArchiveFiles (
		const char *lpDestPath,
		bool bMove,
		char *lpResultDestPath
		)
{
	bool bResult = false;

	FSF.AddEndSlash (lpResultDestPath);

	FarDialog D (-1, -1, 75, 13);

	D.DoubleBox (3, 1, 71, 11, bMove?"Распаковка с удалением из архива":"Распаковка"); //0

	D.Text (5, 2, "&Распаковать в"); //1
	D.Edit (5, 3, 65, lpResultDestPath, AUTO_LENGTH, "123"); //2

	D.Separator (4);

	D.Text (5, 5, "&Пароль");
	D.PswEdit (5, 6, 40);

	D.Separator (7);

	D.CheckBox (5, 8, false, "Распаковка без путей");

	D.Separator (9);

	D.Button (-1, 10, "Распаковать");
	D.DefaultButton ();

	D.Button (-1, 10, "Отменить");

	if ( D.Show () == D.FirstButton() )
	{
		strcpy (lpResultDestPath, D[2].Data);

		bResult = true;
	}

	return bResult;
}

bool doDeleteFiles (ArchivePanel *pPanel, Archive *pArchive, PluginPanelItem *pItemsToProcess, int nItemsToProcessNumber)
{
	bool bResult = true; //!!ERROR!!!

	bool bExternal = !OptionIsOn (pArchive->m_pInfo->dwFlags, AFF_SUPPORT_INTERNAL_DELETE);

	if ( bExternal )
	{
		pArchive->pNotify ((HANDLE)pPanel, NOTIFY_EXTERNAL_DELETE_START, NULL);

		pPanel->pExecuteCommand (
				COMMAND_DELETE,
				NULL,
				NULL,
				pItemsToProcess,
				nItemsToProcessNumber
				);

		pArchive->pNotify ((HANDLE)pPanel, NOTIFY_EXTERNAL_DELETE_END, NULL);
	}
	else
	{
		//	if ( m_pArchive->pOpenArchive (OM_EXTRACT) ) //to cache opened state!!!
		//{
		bResult = pArchive->pDelete (
				pItemsToProcess,
				nItemsToProcessNumber
				);

		Info.Control ((HANDLE)pPanel, FCTL_UPDATEPANEL, NULL);
		//m_pArchive->pCloseArchive ();
		//	}
	}

	return bResult;
}



int __stdcall ArchivePanel::pGetFiles (
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int Move,
		char *DestPath,
		int OpMode
		)
{
	int bResult = true; ///!!! error!!!

	char *lpResultDestPath = StrDuplicate (DestPath, 260);
	char *lpCurrentDirectory = StrCreate (MAX_PATH);

	GetCurrentDirectory (MAX_PATH, lpCurrentDirectory);

	if ( ((OpMode & OPM_SILENT) == OPM_SILENT) || dlgGetArchiveFiles(DestPath, Move, lpResultDestPath) )
	{
		m_pArchive->m_nMode = OpMode;

		SetCurrentDirectory (lpResultDestPath);

		PluginPanelItem *pItemsToProcess;
		int nItemsToProcessNumber;

		GetArchiveItemsToProcess (
				this,
				PanelItem,
				ItemsNumber,
				&pItemsToProcess,
				&nItemsToProcessNumber,
				&m_pArchive->m_OS
				);

		bool bExternal = !OptionIsOn (m_pArchive->m_pInfo->dwFlags, AFF_SUPPORT_INTERNAL_EXTRACT);

		HANDLE hScreen = NULL;

		if ( !OptionIsOn (OpMode, OPM_SILENT) )
			hScreen = Info.SaveScreen (0, 0, -1, -1);

		if ( bExternal )
		{
			m_pArchive->pNotify ((HANDLE)this, NOTIFY_EXTERNAL_EXTRACT_START, NULL);

			pExecuteCommand (
				((OpMode & (OPM_FIND|OPM_VIEW|OPM_EDIT)) ? COMMAND_EXTRACT_WITHOUT_PATH : COMMAND_EXTRACT),
					NULL,
					NULL,
					pItemsToProcess,
					nItemsToProcessNumber
					);

			m_pArchive->pNotify ((HANDLE)this, NOTIFY_EXTERNAL_EXTRACT_END, NULL);
		}
		else
		{
			if ( m_pArchive->pOpenArchive (OM_EXTRACT) ) //to cache opened state!!!
			{
				bResult = m_pArchive->pExtract (
						pItemsToProcess,
						nItemsToProcessNumber,
						lpResultDestPath,
						m_lpCurrentFolder
						);

				m_pArchive->pCloseArchive ();
			}
		}

		if ( Move )
			doDeleteFiles (this, m_pArchive, pItemsToProcess, nItemsToProcessNumber);

		if ( hScreen )
		{
			Info.RestoreScreen (NULL);
			Info.RestoreScreen (hScreen);
		}

		free (pItemsToProcess);
	}

	SetCurrentDirectory (lpCurrentDirectory);

	StrFree (lpResultDestPath);
	StrFree (lpCurrentDirectory);

	return bResult;
}

bool msgDeleteFiles ()
{
	const char *pMsgs[4];

	pMsgs[0] = "Удаление из архива";
	pMsgs[1] = "Вы хотите удалить из архива файлы";
	pMsgs[2] = "Удалить";
	pMsgs[3] = "Отменить";

	return !Info.Message (
			Info.ModuleNumber,
			0,
			NULL,
			(const char *const*)pMsgs,
			4,
			2
			);
}



int __stdcall ArchivePanel::pDeleteFiles (
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int OpMode
		)
{
	PluginPanelItem *pItemsToProcess;
	int nItemsToProcessNumber;

	int bResult = FALSE;

	if ( msgDeleteFiles () )
	{
		GetArchiveItemsToProcess (
				this,
				PanelItem,
				ItemsNumber,
				&pItemsToProcess,
				&nItemsToProcessNumber,
				&m_pArchive->m_OS
				);

		bResult = doDeleteFiles (this, m_pArchive, pItemsToProcess, nItemsToProcessNumber);

		free (pItemsToProcess);
	}

	return bResult;
}



void __stdcall ArchivePanel::pFreeFindData (
		PluginPanelItem *pPanelItem,
		int nItemsNumber
		)
{
	free (pPanelItem);
}

int __stdcall ArchivePanel::pSetDirectory (
		const char *Dir,
		int nOpMode
		)
{
	if ( !strcmp (Dir, "..") )
	{
		if ( strchr (m_lpCurrentFolder, '\\') )
			CutTo (m_lpCurrentFolder, '\\', true);
		else
			*m_lpCurrentFolder = '\0';

		return TRUE;
	}
	else

	if ( !strcmp (Dir, "\\") )
	{
		*m_lpCurrentFolder = '\0';
		return TRUE;
	}
	else
	{
		int nCurDirLength = StrLength(m_lpCurrentFolder);
		int nDirLength = StrLength(Dir);

		if ( nCurDirLength )
			nCurDirLength++;

		for (int i = 0; i < m_pArchiveFiles.count(); i++)
		{
			InternalArchiveItemInfo *item = &m_pArchiveFiles[i];

			const char *lpCurName = item->ItemInfo.pi.FindData.cFileName;

			if ( ((int)StrLength(lpCurName) >= nCurDirLength+nDirLength) &&
					!FSF.LStrnicmp (Dir, lpCurName+nCurDirLength, nDirLength) )
			{
				const char *p = lpCurName+nCurDirLength+nDirLength;

				if ( (*p == '\\') || (*p == '/') || !*p )
				{
					if ( *m_lpCurrentFolder )
						strcat (m_lpCurrentFolder, "\\");

					strcat (m_lpCurrentFolder, Dir);

					return TRUE;
				}
			}
		}
	}

	return FALSE;
}


void __stdcall ArchivePanel::pClosePlugin ()
{
	if ( m_pArchive )
		m_pArchive->m_pPlugin->FinalizeArchive (
				m_pArchive
				);
}

#define OPERATION_TEST					0
#define OPERATION_ADD_ARCHIVE_COMMENT	1
#define OPERATION_ADD_FILE_COMMENT		2
#define OPERATION_CONVERT_TO_SFX		3
#define OPERATION_RECOVER				4
#define OPERATION_ADD_RECOVERY_RECORD	5
#define OPERATION_LOCK					6

int mnuChooseOperation ()
{
	FarMenuItem *pItems = (FarMenuItem*)malloc (
			7*sizeof (FarMenuItem)
			);

	memset (pItems, 0, 7*sizeof (FarMenuItem));

	strcpy (pItems[0].Text, _M(MSG_mnuCO_S_TEST_ARCHIVE));
	strcpy (pItems[1].Text, _M(MSG_mnuCO_S_ADD_ARCHIVE_COMMENT));
	strcpy (pItems[2].Text, _M(MSG_mnuCO_S_ADD_FILE_COMMENT));
	strcpy (pItems[3].Text, _M(MSG_mnuCO_S_CONVERT_TO_SFX));
	strcpy (pItems[4].Text, _M(MSG_mnuCO_S_RECOVER));
	strcpy (pItems[5].Text, _M(MSG_mnuCO_S_ADD_RECOVERY_RECORD));
	strcpy (pItems[6].Text, _M(MSG_mnuCO_S_LOCK));

	int nResult = Info.Menu (
			Info.ModuleNumber,
			-1,
			-1,
			0,
			FMENU_WRAPMODE,
			"Архивные комманды",
			NULL,
			NULL,
			NULL,
			NULL,
			(const FarMenuItem*)pItems,
			7
			);

	free (pItems);

	return nResult;
}


int __stdcall ArchivePanel::pProcessHostFile(
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int OpMode
		)
{
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

		PluginPanelItem *pItemsToProcess;
		int nItemsToProcessNumber;

		GetArchiveItemsToProcess (
				this,
				PanelItem,
				ItemsNumber,
				&pItemsToProcess,
				&nItemsToProcessNumber,
				&m_pArchive->m_OS
				);

		if ( nCommand == COMMAND_TEST )
		{
			m_pArchive->pOpenArchive (OM_TEST);

			TestStruct TS;

			TS.hArchive = m_pArchive->m_hArchive;
			TS.pItems = pItemsToProcess;
			TS.nItemsNumber = nItemsToProcessNumber;

			m_pArchive->m_pPlugin->m_pfnPluginEntry (FID_TEST, (void*)&TS); //ламерство, натуральное!!! исправить обязательно

			m_pArchive->pCloseArchive ();
		}
		else
			pExecuteCommand (
					nCommand,
					NULL,
					NULL,
					pItemsToProcess,
					nItemsToProcessNumber
					);

		free (pItemsToProcess);

		return TRUE;
	}

	return FALSE;
}

int __stdcall ArchivePanel::pProcessKey (
		int nKey,
		dword dwControlState
		)
{

	if ( (nKey == VK_F7) && (dwControlState == 0) )
	{
		char szFolderPath[MAX_PATH];

		if ( Info.InputBox (
				"Создание папки",
				"Создать папку",
				NULL,
				NULL,
				szFolderPath,
				MAX_PATH,
				NULL,
				FIB_EXPANDENV|FIB_BUTTONS
				) )
		{
			PluginPanelItem item;

			memset (&item, 0, sizeof (PluginPanelItem));

			strcpy (item.FindData.cFileName, szFolderPath);
			item.FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;

			PanelInfo pnThis;

			Info.Control (this, FCTL_GETPANELSHORTINFO, &pnThis);

			m_pArchive->pAddFiles (
					NULL,
					(const char*)&pnThis.CurDir,
					&item,
					1
					);
		}
	}

	return FALSE;
}
