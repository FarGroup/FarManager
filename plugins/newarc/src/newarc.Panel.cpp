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

				pMsgs[0] = "Подождите";
				pMsgs[1] = "Чтение архива";
				pMsgs[2] = m_pArchive->m_lpFileName;
				pMsgs[3] = (char*)&szFileCount;

				FSF.sprintf ((char*)&szFileCount, "%d файлов", m_nArchiveFilesCount);

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
				FSF.sprintf (pItems[i].Text, "%s", m_pArchives[i]->pGetArchiveFormatName());

			int nResult = 0;

			if ( m_nArchivesCount > 1 )
			{
				nResult = Info.Menu (
						Info.ModuleNumber,
						-1,
						-1,
						0,
						FMENU_WRAPMODE,
						"Открыть как",
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
					bAppend = false;
					goto l_1;
				}
			}

			if ( bAppend )
			{
				memcpy (&pPanelItems[nCount], pCurrentPanelItem, sizeof (PluginPanelItem));
				strcpy (pPanelItems[nCount].FindData.cFileName, lpName2);

				if ( bIsFolder )
					pPanelItems[nCount].FindData.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;

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

		char *lpFormatName = m_pArchive->pGetArchiveFormatName ();

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



void GetArchiveItemsToProcess (
		ArchivePanel *pPanel,
		PluginPanelItem *pPanelItems,
		int nItemsNumber,
		PluginPanelItem **pItemsToProcess,
		int *pItemsToProcessNumber
		)
{
	int nArrayCount = 256;
	int nCount = 0;

	PluginPanelItem *pResult = (PluginPanelItem*)malloc (
			nArrayCount*sizeof (PluginPanelItem)
			);

	for (int i = 0; i < nItemsNumber; i++)
	{
		if ( (pPanelItems[i].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
		{
			char *lpPath = StrDuplicate (pPanel->m_lpCurrentFolder, 260);

			if ( *pPanel->m_lpCurrentFolder )
				strcat (lpPath, "\\");

			strcat (lpPath, pPanelItems[i].FindData.cFileName);

			for (int k = 0; k < pPanel->m_nArchiveFilesCount; k++)
			{
				PluginPanelItem *pCurrentPanelItem = &pPanel->m_pArchiveFiles[k].ItemInfo.pi;

				if ( !FSF.LStrnicmp (
						lpPath,
						pCurrentPanelItem->FindData.cFileName,
						strlen (lpPath)
						) )
				{
					//if ( (pCurrentPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 )
					{
						memcpy (&pResult[nCount++], pCurrentPanelItem, sizeof (PluginPanelItem));

						pPanel->m_pArchive->m_uFullSize += pCurrentPanelItem->FindData.nFileSizeHigh*0x100000000ull+pCurrentPanelItem->FindData.nFileSizeLow;

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

		InternalArchiveItemInfo *pItemInfo = (InternalArchiveItemInfo*)pPanelItems[i].UserData;

		if ( pItemInfo )
		{
			memcpy (&pResult[nCount++], &pItemInfo->ItemInfo.pi, sizeof (PluginPanelItem));

		//	MessageBox (0, pItemInfo->ItemInfo.pi.FindData.cFileName, "asd", MB_OK);
		}
		else
		{
			//MessageBox (0, "error no data", "asd", MB_OK);

			char *lpFullName = StrCreate (260);

			if ( *pPanel->m_lpCurrentFolder )
			{
				strcpy (lpFullName, pPanel->m_lpCurrentFolder);
				FSF.AddEndSlash (lpFullName);
			}

			strcat (lpFullName, pPanelItems[i].FindData.cFileName);

			memcpy (&pResult[nCount], &pPanelItems[i], sizeof (PluginPanelItem));

			strcpy ((char*)&pResult[nCount++].FindData.cFileName, lpFullName);

			StrFree (lpFullName);
		}

        pPanel->m_pArchive->m_uFullSize += pPanelItems[i].FindData.nFileSizeHigh*0x100000000ull+pPanelItems[i].FindData.nFileSizeLow;

		if ( nCount == nArrayCount )
		{
			nArrayCount = nArrayCount+256+nArrayCount/4;

        	pResult = (PluginPanelItem*)realloc (
					pResult,
					nArrayCount*sizeof (PluginPanelItem)
					);
		}
	}

	*pItemsToProcessNumber = nCount;
	*pItemsToProcess = pResult;
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

				OemToChar(lpExecuteString,lpExecuteString);

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

	FSF.sprintf (lpTitle, "Добавить к %s", lpFormat);

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
	char *szNames = StrCreate (4096); //я знаю, что это ламерство, но я вообще не собираюсь хранить шаблоны в ини. ???

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

			itoa (Templates[i]->nArchiver, szArchiver, 10);

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

	D->DoubleBox (3, 1, 51, 9, ( bAdd ) ? "Добавить шаблон" : "Изменить шаблон"); //0

	D->Text (5, 2, "Имя шаблона"); //1
	D->Edit (5, 3, 45); //2

	D->Separator (4); //3

	D->Text (5, 5, "Архиватор"); //4
	D->ComboBox (5, 6, 15, NULL, 0); //5
	D->SetFlags (DIF_DROPDOWNLIST);

	*(int*)D->m_Items[5].Data = ( bAdd ) ? -1 : iPos ;

	D->Text (22, 5, "Дополнительные параметры"); //6
	D->ComboBox (22, 6, 27, NULL, 0); //7

	D->Separator(7); //8

	D->Button(-1, 8, ( bAdd ) ? "Добавить" : "Применить"); //9
	D->DefaultButton ();

	D->Button(-1, 8, "Отменить"); //10

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
				D->ListAddStr (12, Plugins[i]->m_ArchivePluginInfo.pFormatInfo[j].lpName);
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
		if ( nParam1 == 24 )
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

  			return bResult;
		}
	}

	return D->DefDlgProc (nMsg, nParam1, nParam2);
}

void dlgModifyCreateArchive ()
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

	D->Text (5, 2, "&Добавить к архиву");//1
	D->Edit (5, 3, 65, lpArchiveName, AUTO_LENGTH, "sdfas"); //2

	D->Separator (4); //3

	D->Text (5, 5, "Настройки архиватора:"); //4

	D->RadioButton (6, 6, true, "Шаблон:"); //5
	D->RadioButton (6, 8, false, "Непосредственные настройки:"); //6

	D->ComboBox (9, 7, 48, NULL, 0); //7
	D->SetFlags (DIF_DROPDOWNLIST);

	D->Button (59, 7, "[+]"); //8
	D->SetFlags(DIF_BTNNOCLOSE);

	D->Button (63, 7, "[-]"); //9
	D->SetFlags(DIF_BTNNOCLOSE);

	D->Button (67, 7, "[*]"); //10
	D->SetFlags(DIF_BTNNOCLOSE);

	D->Text (9, 9, "Архиватор:");//11
	D->ComboBox (19, 9, 15, NULL, 0, "123");//12
///	D->ListBox (50, 5, 65, 10, NULL);
	D->SetFlags (DIF_DROPDOWNLIST);

	D->Text (37, 9, "Д&оп. параметры:");//13
	D->Edit (52, 9, 18, NULL, AUTO_LENGTH, "adsaf");//14

	D->Separator (10); //15

	D->Text (5, 11, "&Пароль"); //16
	D->PswEdit (5, 12, 32); //17

	D->Text (38, 11, "&Подтверждение пароля"); //18
	D->PswEdit (38, 12, 32); //19

	D->Separator (13); //20
	D->Text (48, 13); //21

	D->CheckBox (5, 14, false, "Точное соответствие имени файла"); //22
	D->CheckBox (5, 15, false, "Каждый элемент в отдельный архив"); //23

	D->Separator (16); //24

	D->Button (-1, 17, "Добавить"); //25
	D->DefaultButton ();

	D->Button (-1, 17, "Отменить"); //26

	if ( D->ShowEx (
			(PVOID)hndModifyCreateArchive
			) == 25 )
	{
		char *lpCommand = StrCreate (260);
		char *lpPassword = StrCreate (260);
		char *lpAdditionalCommandLine = StrCreate (260);

		int nPos = D->m_Items[12].ListPos;
		int nIndex = 0;

		strcpy (lpArchiveName, D->m_Items[2].Data);
		strcpy (lpPassword, D->m_Items[17].Data);
		strcpy (lpAdditionalCommandLine, D->m_Items[14].Data);

		ArchivePlugin *pPlugin = NULL;

		for (int i = 0; i < Plugins.GetCount(); i++)
		{
			for (int j = 0; j < Plugins[i]->m_ArchivePluginInfo.nFormats; j++)
			{
				if ( nIndex == nPos )
				{

					pPlugin = Plugins[i];
					nIndex = j;

					bool bSeparately = D->m_Items[23].Selected && (pnInfo.SelectedItemsNumber > 1);
					int	iCount = ( bSeparately ) ? pnInfo.SelectedItemsNumber : 1;

					for (int k = 0; k < iCount; k++)
					{
						if ( bSeparately )
						{
							strcpy (lpArchiveName, FSF.PointToName (pnInfo.SelectedItems[k].FindData.cFileName));
							if ( !(pnInfo.SelectedItems[k].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) )
								CutTo (lpArchiveName, '.', true);
						}

						strcat (lpArchiveName, ".");

						if ( !D->m_Items[22].Selected )
							strcat (lpArchiveName, Plugins[i]->m_ArchivePluginInfo.pFormatInfo[j].lpDefaultExtention);

						if ( pPlugin )
						{
							GetDefaultCommandStruct GDC;

							GDC.nFormat = nIndex;
							GDC.nCommand = COMMAND_ADD;
							GDC.lpCommand = lpCommand;

							if ( pPlugin->m_pfnPluginEntry (FID_GETDEFAULTCOMMAND, (void*)&GDC) == NAERROR_SUCCESS )
							{
								ExecuteCommand (
										nIndex,
										lpCommand,
										lpArchiveName,
										lpPassword,
										NULL,
										lpAdditionalCommandLine,
										pnInfo.SelectedItems+k,
										( bSeparately ) ? 1 : pnInfo.SelectedItemsNumber
										);
							}
						}
					}

					goto l_Found;
				}
				nIndex++;
			}
		}

l_Found:

		StrFree (lpPassword);
		StrFree (lpCommand);
		StrFree (lpAdditionalCommandLine);
	}

	StrFree (lpArchiveName);

	SaveTemplates ();

	delete D;
}


int __stdcall ArchivePanel::pPutFiles(
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int Move,
		int OpMode
        )
{
	if ( m_pArchive == NULL )
		dlgModifyCreateArchive ();
	else
	{
		bool bExternal = !((m_pArchive->m_pPlugin->m_ArchivePluginInfo).pFormatInfo[m_pArchive->pGetArchiveFormatType()].dwFlags&AFF_SUPPORT_INTERNAL_ADD);

		if ( bExternal )
		{
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
		}
		else
		{
			m_pArchive->pAddFiles (
					NULL, 
					0
					);
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

	D->DoubleBox (3, 1, 71, 11, bMove?"Распаковка с удалением из архива":"Распаковка"); //0

	D->Text (5, 2, "&Распаковать в"); //1
	D->Edit (5, 3, 65, lpResultDestPath, AUTO_LENGTH, "123"); //2

	D->Separator (4);

	D->Text (5, 5, "&Пароль");
	D->PswEdit (5, 6, 40);

	D->Separator (7);

	D->CheckBox (5, 8, false, "Распаковка без путей");

	D->Separator (9);

	D->Button (-1, 10, "Распаковать");
	D->DefaultButton ();

	D->Button (-1, 10, "Отменить");

	if ( D->Show () == D->m_nFirstButton )
	{
		strcpy (lpResultDestPath, D->m_Items[2].Data);

		bResult = true;
	}


	delete D;

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
	char *lpResultDestPath = StrDuplicate (DestPath, 260);
	char *lpCurrentDirectory = StrCreate (MAX_PATH);

	GetCurrentDirectory (MAX_PATH, lpCurrentDirectory);

	if ( ((OpMode & OPM_SILENT) == OPM_SILENT) || dlgGetArchiveFiles(DestPath, Move, lpResultDestPath) )
	{
		m_pArchive->m_nMode = OpMode;

		SetCurrentDirectory (lpResultDestPath);

		PluginPanelItem *pItemsToProcess;
		int nItemsToProcessNumber;

		m_pArchive->m_uFullSize = 0;

		GetArchiveItemsToProcess (
				this,
				PanelItem,
				ItemsNumber,
				&pItemsToProcess,
				&nItemsToProcessNumber
				);

		bool bExternal = !((m_pArchive->m_pPlugin->m_ArchivePluginInfo).pFormatInfo[m_pArchive->pGetArchiveFormatType()].dwFlags&AFF_SUPPORT_INTERNAL_EXTRACT);

		HANDLE hScreen = Info.SaveScreen (0, 0, -1, -1);

		if ( bExternal )
			pExecuteCommand (
				((OpMode & (OPM_FIND|OPM_VIEW|OPM_EDIT)) ? COMMAND_EXTRACT_WITHOUT_PATH : COMMAND_EXTRACT),
					NULL,
					NULL,
					pItemsToProcess,
					nItemsToProcessNumber
					);
		else
		{
			if ( m_pArchive->pOpenArchive (OM_EXTRACT) ) //to cache opened state!!!
			{
				/*bool bResult =*/ m_pArchive->pExtract (
						pItemsToProcess,
						nItemsToProcessNumber,
						lpResultDestPath,
						m_lpCurrentFolder
						);

				m_pArchive->pCloseArchive ();
			}
		}

		if ( Move )
		{
			pExecuteCommand (
					COMMAND_DELETE,
					NULL,
					NULL,
					pItemsToProcess,
					nItemsToProcessNumber
					);
		}

		Info.RestoreScreen (NULL);
		Info.RestoreScreen (hScreen);

		free (pItemsToProcess);
	}

	SetCurrentDirectory (lpCurrentDirectory);

	StrFree (lpResultDestPath);
	StrFree (lpCurrentDirectory);

	return TRUE;
}

bool msgDeleteFiles ()
{
	char *pMsgs[4];

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

	if ( msgDeleteFiles () )
	{
		GetArchiveItemsToProcess (
				this,
				PanelItem,
				ItemsNumber,
				&pItemsToProcess,
				&nItemsToProcessNumber
				);

		bool bExternal = !((m_pArchive->m_pPlugin->m_ArchivePluginInfo).pFormatInfo[m_pArchive->pGetArchiveFormatType()].dwFlags&AFF_SUPPORT_INTERNAL_DELETE);

		if ( bExternal )
		{
			pExecuteCommand (
					COMMAND_DELETE,
					NULL,
					NULL,
					pItemsToProcess,
					nItemsToProcessNumber
					);
		}
		else
		{
		//	if ( m_pArchive->pOpenArchive (OM_EXTRACT) ) //to cache opened state!!!
			//{
				m_pArchive->pDelete (
						pItemsToProcess,
						nItemsToProcessNumber
						);

			Info.Control (INVALID_HANDLE_VALUE, FCTL_UPDATEPANEL, NULL);
				//m_pArchive->pCloseArchive ();
		//	}
		}


		free (pItemsToProcess);

		return TRUE;
	}

	return FALSE;
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
				&nItemsToProcessNumber
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
	return FALSE;
}
