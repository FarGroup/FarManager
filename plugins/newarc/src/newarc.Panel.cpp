#include "newarc.h"

char *lpINIFileName;

ArchivePanel::ArchivePanel (
		Archive** pArchives,
		int nArchivesCount
		)
{
	m_pArchives = pArchives;
	m_nArchivesCount = nArchivesCount;

	m_pArchive = NULL;//pArchive;
	m_lpCurrentFolder = StrCreate (260);

	m_pArchiveFiles = NULL;
	m_nArchiveFilesCount = 0;

	m_lpPanelTitle = StrCreate (260);

	m_Editors.Create (5);

	m_bFirstTime = true;
	m_nCurrentMode = 0;

	lpINIFileName = StrDuplicate (Info.ModuleName, 260);
	CutToSlash (lpINIFileName);
	strcat (lpINIFileName, "templates.ini");
}


ArchivePanel::~ArchivePanel ()
{
	StrFree (m_lpCurrentFolder);
	StrFree (m_lpPanelTitle);

	for (int i = 0; i < m_nArchiveFilesCount; i++)
		StrFree (m_pArchiveFiles[i].lpPassword);

	free (m_pArchiveFiles);

	free (m_pArchives);

	m_Editors.Free ();

	StrFree (lpINIFileName);
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

	for (int i = 0; i < m_nArchiveFilesCount; i++)
		StrFree (m_pArchiveFiles[i].lpPassword);

	free (m_pArchiveFiles);

	dword dwStartTime = GetTickCount ();
	bool bProgressMessage = false;

	HANDLE hScreen = NULL;

	if ( !bSilent )
		hScreen = Info.SaveScreen (0, 0, -1, -1);

	m_nArrayCount = 256;
	m_nArchiveFilesCount = 0;

	m_pArchiveFiles = (InternalArchiveItemInfo*)malloc (
			m_nArrayCount*sizeof (InternalArchiveItemInfo)
			);

	int nResult = E_SUCCESS;

	while ( (nResult == E_SUCCESS) && !CheckForEsc() )
	{
		nResult = m_pArchive->pGetArchiveItem (
				&m_pArchiveFiles[m_nArchiveFilesCount].ItemInfo
				);

		if ( nResult == E_SUCCESS )
		{
			if ( !bSilent &&
				 ((m_nArchiveFilesCount & 0x1f) == 0) && (GetTickCount ()-dwStartTime > 500) )
			{
				char szFileCount[100];
				char *pMsgs[4];

				pMsgs[0] = "��������";
				pMsgs[1] = "�⥭�� ��娢�";
				pMsgs[2] = m_pArchive->m_lpFileName;
				pMsgs[3] = (char*)&szFileCount;

				FSF.sprintf ((char*)&szFileCount, "%d 䠩���", m_nArchiveFilesCount);

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

			m_nArchiveFilesCount++;

			if ( m_nArchiveFilesCount == m_nArrayCount )
			{
				m_nArrayCount = m_nArrayCount+256+m_nArrayCount/4;

				m_pArchiveFiles	= (InternalArchiveItemInfo*)realloc (
						m_pArchiveFiles,
						m_nArrayCount*sizeof (InternalArchiveItemInfo)
						);
			}
		}

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
	int nArrayCount = 256;
	int nCount = 0;

	if ( m_bFirstTime )
	{
		if ( m_nArchivesCount )
		{
			FarMenuItem *pItems = (FarMenuItem*)malloc (
					m_nArchivesCount*sizeof (FarMenuItem)
					);

			for (int i = 0; i < m_nArchivesCount; i++)
				FSF.sprintf (pItems[i].Text, "%s", m_pArchives[i]->m_pInfo->lpName);

			int nResult = 0;

			if ( m_nArchivesCount > 1 )
			{
				nResult = Info.Menu (
						Info.ModuleNumber,
						-1,
						-1,
						0,
						FMENU_WRAPMODE,
						"������ ���",
						NULL,
						NULL,
						NULL,
						NULL,
						(const FarMenuItem*)pItems,
						m_nArchivesCount
						);
			}

			for (int i = 0; i < m_nArchivesCount; i++)
				if ( i != nResult )
					m_pArchives[i]->m_pPlugin->FinalizeArchive (m_pArchives[i]);

			free (pItems);

			if ( nResult != -1 )
				m_pArchive = m_pArchives[nResult];
			else
				return FALSE;
		}

		m_bFirstTime = false;
	}


	if ( m_pArchive->WasUpdated () )
	{
		bool bSilent = OpMode & (OPM_SILENT|OPM_FIND);

		if ( !ReadArchive (bSilent) )
			return FALSE;
	}

	PluginPanelItem *pPanelItems = (PluginPanelItem*)malloc (
			nArrayCount*sizeof (PluginPanelItem)
			);

	bool bAppend;
	bool bIsFolder;

	char *lpFoder = StrDuplicate (m_lpCurrentFolder, 260);

	if ( *lpFoder )
		FSF.AddEndSlash (lpFoder);

	for (int i = 0; i < m_nArchiveFilesCount; i++)
	{
		PluginPanelItem *pCurrentPanelItem = &m_pArchiveFiles[i].ItemInfo.pi;

		bAppend = true;
		bIsFolder = false;

		char *lpName = StrDuplicate (pCurrentPanelItem->FindData.cFileName);

		if ( !*m_lpCurrentFolder || !FSF.LStrnicmp (lpName, lpFoder, strlen (lpFoder)) )
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

			for (int k = 0; k < nCount; k++)
			{
				if ( !FSF.LStricmp (
						lpName2,
						pPanelItems[k].FindData.cFileName
						) )
				{
					pPanelItems[k].FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY; //ohhha-ha-ha!!!
					pPanelItems[k].UserData = 0;
					bAppend = false;
					goto l_1;
				}
			}

			if ( bAppend )
			{
				memcpy (&pPanelItems[nCount], pCurrentPanelItem, sizeof (PluginPanelItem));
				strcpy (pPanelItems[nCount].FindData.cFileName, lpName2);

				if ( bIsFolder )
				{
					pPanelItems[nCount].FindData.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
					pPanelItems[nCount].UserData = 0;
				}
				else
					pPanelItems[nCount].UserData = (dword)&m_pArchiveFiles[i];

				nCount++;

				if ( nCount == nArrayCount )
				{
					nArrayCount = nArrayCount+256+nArrayCount/4;

					pPanelItems = (PluginPanelItem*)realloc (
								pPanelItems,
								nArrayCount*sizeof (PluginPanelItem)
								);
				}
			}
		}
l_1:

		StrFree (lpName);
	}

	*pPanelItem = pPanelItems;
	*pItemsNumber = nCount;

	return TRUE;
}

void __stdcall ArchivePanel::pGetOpenPluginInfo (
		OpenPluginInfo *pInfo
		)
{
	pInfo->StructSize = sizeof (OpenPluginInfo);
	pInfo->Flags = OPIF_USEHIGHLIGHTING | OPIF_USESORTGROUPS | OPIF_ADDDOTS;
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
		n += strlen (string); \
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

			char *lpCRLF = "\r\n";

			if ( OptionIsOn (dwFlags, PF_FLAG_DIR_NAME_AS_MASK) )
			{
				if ( OptionIsOn (pItems[i].FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
				{
					FSF.AddEndSlash (lpProcessedString);
					strcat (lpProcessedString, "*.*");

					WriteFile (hListFile, lpProcessedString, strlen (lpProcessedString), &dwWritten, NULL);
					WriteFile (hListFile, lpCRLF, 2, &dwWritten, NULL);

					CutTo (lpProcessedString, '\\', true);
				}
			}

			WriteFile (hListFile, lpProcessedString, strlen (lpProcessedString), &dwWritten, NULL);
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
						n += strlen (lpProcessedName);
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
						n += strlen (lpProcessedName);
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
						n += strlen (pParam->lpListFileName);
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
						n += strlen (lpProcessedName);

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
	int nLength = strlen (lpCurrentPath);

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
	int nArrayCount = 256;
	int nCount = 0;

	memset (pOS, 0, sizeof (OperationStruct));

	PluginPanelItem *pResult = (PluginPanelItem*)malloc (
			nArrayCount*sizeof (PluginPanelItem)
			);

	for (int i = 0; i < nItemsNumber; i++)
	{
		InternalArchiveItemInfo *pItemInfo = (InternalArchiveItemInfo*)pPanelItems[i].UserData;

		if ( pItemInfo )
		{
			memcpy (&pResult[nCount++], &pItemInfo->ItemInfo.pi, sizeof (PluginPanelItem));

		//	MessageBox (0, pItemInfo->ItemInfo.pi.FindData.cFileName, "asd", MB_OK);
		}
		else
		{
			char *lpFullName = StrCreate (260);

			if ( *pPanel->m_lpCurrentFolder )
			{
				strcpy (lpFullName, pPanel->m_lpCurrentFolder);
				FSF.AddEndSlash (lpFullName);
			}

			strcat (lpFullName, pPanelItems[i].FindData.cFileName);

			memcpy (&pResult[nCount], &pPanelItems[i], sizeof (PluginPanelItem));

			pResult[nCount].UserData = 0;

			strcpy ((char*)&pResult[nCount++].FindData.cFileName, lpFullName);

			//__debug ("no data - %s", lpFullName);

			StrFree (lpFullName);
		}

        pOS->uTotalSize += pPanelItems[i].FindData.nFileSizeHigh*0x100000000ull+pPanelItems[i].FindData.nFileSizeLow;

		if ( nCount == nArrayCount )
		{
			nArrayCount = nArrayCount+256+nArrayCount/4;

        	pResult = (PluginPanelItem*)realloc (
					pResult,
					nArrayCount*sizeof (PluginPanelItem)
					);
		}

		if ( OptionIsOn(pPanelItems[i].FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
		{
			char *lpPath = StrDuplicate (pPanel->m_lpCurrentFolder, 260);

			if ( *pPanel->m_lpCurrentFolder )
				strcat (lpPath, "\\");

			strcat (lpPath, pPanelItems[i].FindData.cFileName);
			strcat (lpPath, "\\");

			for (int k = 0; k < pPanel->m_nArchiveFilesCount; k++)
			{
				PluginPanelItem *pCurrentPanelItem = &pPanel->m_pArchiveFiles[k].ItemInfo.pi;

				if ( IsFileInFolder (lpPath, pCurrentPanelItem->FindData.cFileName) )
				{
					if ( (pCurrentPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 )
					{
						//MessageBox (0, pCurrentPanelItem->FindData.cFileName, pCurrentPanelItem->FindData.cFileName, MB_OK);

						memcpy (&pResult[nCount++], pCurrentPanelItem, sizeof (PluginPanelItem));

						pOS->uTotalSize += pCurrentPanelItem->FindData.nFileSizeHigh*0x100000000ull+pCurrentPanelItem->FindData.nFileSizeLow;

						if ( nCount == nArrayCount )
						{
							nArrayCount = nArrayCount+256+nArrayCount/4;

							pResult = (PluginPanelItem*)realloc (
										pResult,
										nArrayCount*sizeof (PluginPanelItem)
										);
						}
					}
				}
			}

			StrFree (lpPath);
		}

	}

	*pItemsToProcessNumber = nCount;
	*pItemsToProcess = pResult;

	pOS->uTotalFiles = nCount;

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
		pMsgs[1] = "�믮������ ��娢��� ������";

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

	D->GetTextPtr (12, lpFormat);

	FSF.sprintf (lpTitle, "�������� � %s", lpFormat);

	D->SetTextPtr (0, lpTitle);

	StrFree (lpFormat);
	StrFree (lpTitle);
}

struct ArchiveTemplate {
	char *lpName;
	int nArchiver;
	char *lpParams;
};

Collection <ArchiveTemplate*> Templates;

int __stdcall hndAddEditTemplate (
		FarDialogHandler *D,
		int nMsg,
		int nParam1,
		int nParam2
		)
{
	int iPos, *pPos = (int*) D->m_Owner->m_Items[5].Data;

	iPos = *pPos; *pPos = 0;

	if ( nMsg == DN_INITDIALOG )
	{
		for (int i = 0; i < Plugins.GetCount(); i++)
		{
			for (int j = 0; j < Plugins[i]->m_ArchivePluginInfo.nFormats; j++)
				D->ListAddStr (5, Plugins[i]->m_ArchivePluginInfo.pFormatInfo[j].lpName);
		}

		if ( iPos != -1 )
		{
			FarListPos arcPos;

			arcPos.SelectPos = Templates[iPos]->nArchiver;
			arcPos.TopPos = -1;

			D->ListSetCurrentPos (5, &arcPos);

			D->SetTextPtr (2, Templates[iPos]->lpName);
			D->SetTextPtr (7, Templates[iPos]->lpParams);
		}
	}

	return D->DefDlgProc (nMsg, nParam1, nParam2);
}

void LoadTemplates ()
{
	char *szNames = StrCreate (4096); //� ����, �� �� ������⢮, �� � ����� �� ᮡ����� �࠭��� 蠡���� � ���. ???

	Templates.Create (5);

	if ( GetPrivateProfileSectionNames(szNames, 4096, lpINIFileName) )
	{
		const char *lpName = szNames;

		while ( *lpName )
		{
			ArchiveTemplate *pTemplate = new ArchiveTemplate;

			pTemplate->lpName = StrDuplicate (lpName);

			Templates.Add (pTemplate);

			lpName += strlen (lpName)+1;
		}
	}

	for (int i = 0; i < Templates.GetCount(); i++)
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

		Templates[i]->nArchiver = GetPrivateProfileInt (Templates[i]->lpName, "Pos", -1, lpINIFileName);
	}

	StrFree (szNames);
}

void SaveTemplates ()
{
	int iCount = Templates.GetCount();

	DeleteFile (lpINIFileName);

	if ( iCount > 0 )
	{
		char szArchiver[8];

		for ( int i = 0; i < iCount; i++ )
		{
			WritePrivateProfileString (
					Templates[i]->lpName,
					"Params",
					Templates[i]->lpParams,
					lpINIFileName
					);

			FSF.itoa (Templates[i]->nArchiver, szArchiver, 10);

			WritePrivateProfileString (
					Templates[i]->lpName,
					"Pos",
					szArchiver,
					lpINIFileName
					);
		}
	}

	Templates.Free ();
}

int dlgAddEditTemplate (FarDialogHandler *DD, bool bAdd)
{
	int iPos = ( bAdd ) ? Templates.GetCount() : DD->ListGetCurrentPos(7,NULL);

	FarDialog *D = new FarDialog (-1, -1, 55, 11);

	D->DoubleBox (3, 1, 51, 9, ( bAdd ) ? "�������� 蠡���" : "�������� 蠡���"); //0

	D->Text (5, 2, "��� 蠡����"); //1
	D->Edit (5, 3, 45); //2

	D->Separator (4); //3

	D->Text (5, 5, "��娢���"); //4
	D->ComboBox (5, 6, 15, NULL, 0); //5
	D->SetFlags (DIF_DROPDOWNLIST);

	*(int*)D->m_Items[5].Data = ( bAdd ) ? -1 : iPos ;

	D->Text (22, 5, "�������⥫�� ��ࠬ����"); //6
	D->ComboBox (22, 6, 27, NULL, 0); //7

	D->Separator(7); //8

	D->Button(-1, 8, ( bAdd ) ? "��������" : "�ਬ�����"); //9
	D->DefaultButton ();

	D->Button(-1, 8, "�⬥����"); //10

	if ( D->ShowEx ((void *)hndAddEditTemplate) == 9 )
	{
		if ( bAdd )
		{
			ArchiveTemplate *pTemplate = new ArchiveTemplate;
			Templates.Add (pTemplate);
		}

		Templates[iPos]->lpName = StrDuplicate (D->m_Items[2].Data);
		Templates[iPos]->lpParams = StrDuplicate (D->m_Items[7].Data);
		Templates[iPos]->nArchiver = D->m_Items[5].ListPos;
	}

	delete D;

	return iPos;
}

void SetTemplate (FarDialogHandler *D)
{
	static bool bInit = false;

	if (!bInit)
	{
		bInit = true;

		FarListPos arcPos;
		FarListInfo li;

		int iCount = Templates.GetCount();

		D->ListInfo(7, &li);

		if (li.SelectPos >= iCount || li.SelectPos == -1 )
			li.SelectPos = iCount-1;

		if ( li.ItemsNumber != iCount )
		{
			D->ListDelete (7, NULL);

			for (int i = 0; i < iCount; i++)
			{
				D->ListAddStr (7, Templates[i]->lpName);
			}

			arcPos.TopPos = -1;
			arcPos.SelectPos = li.SelectPos;
			D->ListSetCurrentPos(7, &arcPos);
		}

		if ( li.SelectPos != -1 )
		{
			arcPos.SelectPos = Templates[li.SelectPos]->nArchiver;
			D->SetTextPtr (14, Templates[li.SelectPos]->lpParams);
		}
		else
		{
			arcPos.SelectPos = 0;
			D->SetTextPtr(7,"");
			D->SetTextPtr(14,"");
		}

		arcPos.TopPos = -1;
		D->ListSetCurrentPos(12, &arcPos);

		D->RedrawDialog();

		bInit = false;
	}

	bool bEnable = (Templates.GetCount() > 0);
	int nFocus = D->GetFocus ();

	D->Enable (9, bEnable);
	D->Enable (10, bEnable);

	if ( !bEnable && ((nFocus == 9) || (nFocus == 10)) )
		D->SetFocus (8);
}



int __stdcall hndModifyCreateArchive (
		FarDialogHandler *D,
		int nMsg,
		int nParam1,
		int nParam2
		)
{
	if ( nMsg == DN_INITDIALOG )
	{
		for (int i = 0; i < Plugins.GetCount(); i++)
		{
			for (int j = 0; j < Plugins[i]->m_ArchivePluginInfo.nFormats; j++)
			{
				int index = D->ListAddStr (12, Plugins[i]->m_ArchivePluginInfo.pFormatInfo[j].lpName);

				formatStruct fs;

				fs.uid = Plugins[i]->m_ArchivePluginInfo.pFormatInfo[j].uid;
				fs.pPlugin = Plugins[i];

				D->ListSetDataEx (12, index, &fs, sizeof (formatStruct));
			}

		}

		for (int i = 0; i < Templates.GetCount(); i++)
			D->ListAddStr (7, Templates[i]->lpName);

		SetTemplate (D);
	}

	if ( nMsg == DN_BTNCLICK )
	{
		if ( (nParam1 == 5) && (nParam2 != 0) )
			SetTemplate (D);

		if ( nParam1 == 8 )
		{
			FarListPos arcPos;

			arcPos.SelectPos = dlgAddEditTemplate (D, true);
			arcPos.TopPos = -1;

			D->ListSetCurrentPos (7, &arcPos);

			SetTemplate (D);
		}

		if ( nParam1 == 9 )
			if (Templates.GetCount ()>0)
			{
				int iPos = D->ListGetCurrentPos (7, NULL);

				Templates.Remove(Templates[iPos]);
				SetTemplate (D);
			}

		if ( nParam1 == 10 )
			if (Templates.GetCount ()>0)
			{
				dlgAddEditTemplate (D, false);
				SetTemplate (D);
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
					D->SetTextPtr (21, "(��஫� ᮢ������)");
				else
					D->SetTextPtr (21, "(��஫� �� ᮢ������)");
			}

			StrFree (lpPassword1);
			StrFree (lpPassword2);
		}

		if (nParam1 == 7 )
			SetTemplate (D);
	}

	if ( nMsg == DN_CLOSE )
	{
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


  			FarListPos pos;

  			D->ListGetCurrentPos (12, &pos);

  			formatStruct *pfsresult = (formatStruct*)D->GetDlgData ();
  			formatStruct *pfs = (formatStruct*)D->ListGetData (12, pos.SelectPos);

  			memcpy (pfsresult, pfs, sizeof (formatStruct));

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

	FarDialog *D = new FarDialog (-1, -1, 75, 20);

	D->DoubleBox (3, 1, 71, 18, NULL); //0

	D->Text (5, 2, "&�������� � ��娢�");//1
	D->Edit (5, 3, 65, lpArchiveName, AUTO_LENGTH, "sdfas"); //2

	D->Separator (4); //3

	D->Text (5, 5, "����ன�� ��娢���:"); //4

	D->RadioButton (6, 6, true, "������:"); //5
	D->RadioButton (6, 8, false, "�����।�⢥��� ����ன��:"); //6

	D->ComboBox (9, 7, 48, NULL, 0); //7
	D->SetFlags (DIF_DROPDOWNLIST);

	D->Button (59, 7, "[+]"); //8
	D->SetFlags(DIF_BTNNOCLOSE);

	D->Button (63, 7, "[-]"); //9
	D->SetFlags(DIF_BTNNOCLOSE);

	D->Button (67, 7, "[*]"); //10
	D->SetFlags(DIF_BTNNOCLOSE);

	D->Text (9, 9, "��娢���:");//11
	D->ComboBox (19, 9, 15, NULL, 0, "123");//12
///	D->ListBox (50, 5, 65, 10, NULL);
	D->SetFlags (DIF_DROPDOWNLIST);

	D->Text (37, 9, "�&��. ��ࠬ����:");//13
	D->Edit (52, 9, 18, NULL, AUTO_LENGTH, "adsaf");//14

	D->Separator (10); //15

	D->Text (5, 11, "&��஫�"); //16
	D->PswEdit (5, 12, 32); //17

	D->Text (38, 11, "&���⢥ত���� ��஫�"); //18
	D->PswEdit (38, 12, 32); //19

	D->Separator (13); //20
	D->Text (48, 13); //21

	D->CheckBox (5, 14, false, "��筮� ᮮ⢥��⢨� ����� 䠩��"); //22
	D->CheckBox (5, 15, false, "����� ����� � �⤥��� ��娢"); //23

	D->Separator (16); //24

	D->Button (-1, 17, "��������"); //25
	D->DefaultButton ();

	D->Button (-1, 17, "�⬥����"); //26

	formatStruct fs;

	if ( D->ShowEx (
			(PVOID)hndModifyCreateArchive,
			&fs
			) == 25 )
	{
		char *lpCommand = StrCreate (260);
		char *lpPassword = StrCreate (260);
		char *lpAdditionalCommandLine = StrCreate (260);

		strcpy (lpArchiveName, D->m_Items[2].Data);
		strcpy (lpPassword, D->m_Items[17].Data);
		strcpy (lpAdditionalCommandLine, D->m_Items[14].Data);

		ArchivePlugin *pPlugin = fs.pPlugin;
		const ArchiveFormatInfo *info = pPlugin->GetArchiveFormatInfo (fs.uid);

		bool bSeparately = D->m_Items[23].Selected && (pnInfo.SelectedItemsNumber > 1);
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
			
			if ( !D->m_Items[22].Selected )
				strcat (lpArchiveName, info->lpDefaultExtention);

			bool bResult = false;

   			if ( OptionIsOn (info->dwFlags, AFF_SUPPORT_INTERNAL_CREATE) )
   			{
   				Archive *pArchive = pPlugin->CreateArchive (info->uid, lpArchiveName);

   				if ( pArchive )
   				{
   					if ( pArchive->pOpenArchive (OM_ADD) ) //!!! ���� ����, ��� �� 㢥७ ����� OpMode, �������� 㡥��.
   					{
   						pPanel->pPutFilesNew (
								pArchive,
								pnInfo.SelectedItems+item,
								(bSeparately)?1:pnInfo.SelectedItemsNumber,
								0,
								0
								);

   						bResult = true;

   						pArchive->pCloseArchive (); //!!! � ���� ��?
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

	StrFree (lpArchiveName);

	SaveTemplates ();

	delete D;
}

struct ScanStruct {
	const char *lpSourcePath;
	PluginPanelItem *files;
	int nCurrentFile;
	int nFilesCount;
};



int __stdcall ScanDirectory (
		const WIN32_FIND_DATA *fdata,
		const char *lpFullName,
		ScanStruct *pSS
		)
{
	if ( pSS->nCurrentFile == pSS->nFilesCount )
	{
		pSS->nFilesCount += 256;
		pSS->files = (PluginPanelItem*)realloc (pSS->files, pSS->nFilesCount*sizeof (PluginPanelItem));
	}

	char szFileNameCopy[MAX_PATH];
	const char *lpFileName = lpFullName+strlen(pSS->lpSourcePath);

	if ( (*lpFileName == '\\') || (*lpFileName == '/') )
		lpFileName++;

	strcpy (szFileNameCopy, lpFileName);

	PluginPanelItem *pitem = &pSS->files[pSS->nCurrentFile++];

	memset (pitem, 0, sizeof (PluginPanelItem));
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

			int nFilesCount = 1;
			int nCurrentFile = 0;

			PluginPanelItem *files = (PluginPanelItem*)malloc (nFilesCount*sizeof (PluginPanelItem));
			memset (files, 0, nFilesCount*sizeof (PluginPanelItem));

			for (int i = 0; i < ItemsNumber; i++)
			{
				if ( nCurrentFile == nFilesCount )
				{
					nFilesCount += 256;
					files = (PluginPanelItem*)realloc (files, nFilesCount*sizeof (PluginPanelItem));
				}

				PluginPanelItem *pitem = &files[nCurrentFile++];
				memcpy (pitem, &PanelItem[i], sizeof (PluginPanelItem));

				if ( (pitem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
				{
					char *lpFullName = StrDuplicate (szCurDir, MAX_PATH);

					FSF.AddEndSlash (lpFullName);
					strcat (lpFullName, PanelItem[i].FindData.cFileName);

					ScanStruct SS;

					SS.lpSourcePath = (const char*)&szCurDir;
					SS.files = files;
					SS.nCurrentFile = nCurrentFile;
					SS.nFilesCount = nFilesCount;

					FSF.FarRecursiveSearch (lpFullName, "*.*", (FRSUSERFUNC)ScanDirectory, FRS_RECUR, &SS);

					nCurrentFile = SS.nCurrentFile;
					nFilesCount = SS.nFilesCount;
					files = SS.files;

					StrFree (lpFullName);
				}

			}

			PanelInfo pnThis;
			bool bPanel = Info.Control (this, FCTL_GETPANELSHORTINFO, &pnThis);

			m_pArchive->pAddFiles (
					(const char*)&szCurDir,
					bPanel?(const char*)&pnThis.CurDir:NULL,
					files,
					nCurrentFile
					);

			free (files);
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

	FarDialog *D = new FarDialog (-1, -1, 75, 13);

	D->DoubleBox (3, 1, 71, 11, bMove?"��ᯠ����� � 㤠������ �� ��娢�":"��ᯠ�����"); //0

	D->Text (5, 2, "&��ᯠ������ �"); //1
	D->Edit (5, 3, 65, lpResultDestPath, AUTO_LENGTH, "123"); //2

	D->Separator (4);

	D->Text (5, 5, "&��஫�");
	D->PswEdit (5, 6, 40);

	D->Separator (7);

	D->CheckBox (5, 8, false, "��ᯠ����� ��� ��⥩");

	D->Separator (9);

	D->Button (-1, 10, "��ᯠ������");
	D->DefaultButton ();

	D->Button (-1, 10, "�⬥����");

	if ( D->Show () == D->m_nFirstButton )
	{
		strcpy (lpResultDestPath, D->m_Items[2].Data);

		bResult = true;
	}


	delete D;

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
	char *pMsgs[4];

	pMsgs[0] = "�������� �� ��娢�";
	pMsgs[1] = "�� ��� 㤠���� �� ��娢� 䠩��";
	pMsgs[2] = "�������";
	pMsgs[3] = "�⬥����";

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
	//__debug ("dir - %s", Dir);

	if ( !strcmp (Dir, "..") )
	{
		if ( strchr (m_lpCurrentFolder, '\\') )
			CutTo (m_lpCurrentFolder, '\\', true);
		else
			*m_lpCurrentFolder = '\0';
	}
	else

	if ( !strcmp (Dir, "\\") )
	{
		*m_lpCurrentFolder = '\0';
	}
	else
	{
		if ( *m_lpCurrentFolder )
			strcat (m_lpCurrentFolder, "\\");

		strcat (m_lpCurrentFolder, Dir);
	}

	//__debug ("real dir - %s", m_lpCurrentFolder);

	return TRUE;
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
			"��娢�� ��������",
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

			m_pArchive->m_pPlugin->m_pfnPluginEntry (FID_TEST, (void*)&TS); //������⢮, ����ࠫ쭮�!!! ��ࠢ��� ��易⥫쭮

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
				"�������� �����",
				"������� �����",
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
