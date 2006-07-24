#include "newarc.h"

bool GetFileInfo (
		const char *lpFileName,
		FILETIME *pTime,
		dword *pSize
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
		memcpy (pTime, &fData.ftLastAccessTime, sizeof (FILETIME));
		*pSize = fData.nFileSizeLow;

		bResult = true;

		FindClose (hSearch);
	}

	return bResult;
}


Archive::Archive (
		ArchivePlugin *pPlugin,
		const char *lpFileName,
		HANDLE hArchive
		)
{

	m_hArchive = hArchive;
	m_pPlugin = pPlugin;
	m_lpFileName = StrDuplicate (lpFileName);
//	m_lpListPassword = NULL;
	m_lpLastUsedPassword = NULL;

	m_nMode = 0;

	CreateClassThunk (Archive, ArchiveCallback, m_pCallbackThunk);

}

bool Archive::WasUpdated ()
{
	FILETIME NewTime;
	dword dwFileSizeLow;

	GetFileInfo (m_lpFileName, &NewTime, &dwFileSizeLow);

	bool bResult = CompareFileTime (&NewTime, &m_ArchiveLastAccessTime) ||
			(dwFileSizeLow != m_dwArchiveFileSizeLow);

	memcpy (&m_ArchiveLastAccessTime, &NewTime, sizeof (FILETIME));
	m_dwArchiveFileSizeLow = dwFileSizeLow;

	return bResult;
}

Archive::~Archive ()
{
//	StrFree (m_lpListPassword);
	StrFree (m_lpLastUsedPassword);
	StrFree (m_lpFileName);
	free (m_pCallbackThunk);

	//for (int i = 0; i < 11; i++)
	//	StrFree (m_pCommands[i]);
}

void doFrame (int X, int Y, int Width, int Height, char *Header, bool Shadow)
{
	int X2 = X+Width-1;
	int Y2 = Y+Height;
	char *Line = (char*)malloc (Width);
	memset(Line,32,Width);
	Line[Width] = 0;

	int Color = FarGetColor (COL_DIALOGTEXT);

	int y;
	for (y = Y; y <= Y2; y++)
	{
		Info.Text (X,y,Color,Line);
		if (Shadow)
		{
			Info.Text (X2+1,y+1,0," ");
			Info.Text (X2+2,y+1,0," ");
		}
	}

	memset (Line, 0, Width+1);
	memset (Line,'═',Width-6);

	Info.Text (X+3,Y+1,Color,Line);
	Info.Text (X+3,Y2-1,Color,Line);

	memset (Line, 0, Width+1);
	memset (Line,' ',Width-1);

	if (Shadow)
		Info.Text (X+3,Y2+1,0,Line);

	for (y = Y+1; y <= Y2-1; y++)
   	{
		Info.Text (X+3,y,Color,"║");
		Info.Text (X2-3,y,Color,"║");
	}

	Color = FarGetColor(COL_DIALOGBOX);
	Info.Text (X+3,Y+1,Color,"╔");
	Info.Text (X2-3,Y2-1,Color,"╝");
	Info.Text (X+3,Y2-1,Color,"╚");
	Info.Text (X2-3,Y+1,Color,"╗");

	if (Header)
	{
		Color = FarGetColor (COL_DIALOGBOXTITLE);
		Info.Text (X+(Width-strlen(Header))/2,Y+1,Color,Header);
	}

	free (Line);
}

void doIndicator (
		int x,
		int y,
		dword dwPercent
		)
{
	char *lpTemp = StrCreate (100);

	memset (lpTemp, 177, 40);
	memset (lpTemp, 219, dwPercent);

	Info.Text (x, y, FarGetColor (COL_DIALOGTEXT), lpTemp);

	StrFree (lpTemp);
}


void doEmptyDialog (
		bool bShowTotalProgress,
		COORD &c
		)
{
	int Diff = 0;
	CONSOLE_SCREEN_BUFFER_INFO SInfo;

	GetConsoleScreenBufferInfo (GetStdHandle(STD_OUTPUT_HANDLE), &SInfo);

	c.X = SInfo.dwSize.X/2-28;
	c.Y = SInfo.dwSize.Y/2-9;

	if ( bShowTotalProgress)
	{
		Diff = 2;
		c.Y = SInfo.dwSize.Y/2-9;
	}

	doFrame (c.X, c.Y, 55, 10+Diff, " Распаковка ", true);
}


extern bool CheckForEsc();

int __stdcall Archive::OnStartOperation (int nOperation, OperationStructPlugin *pOS)
{
	if ( pOS )
	{
		if ( OptionIsOn (pOS->dwFlags, OS_FLAG_TOTALSIZE) )
			m_OS.uTotalSize = pOS->uTotalSize;

		if ( OptionIsOn (pOS->dwFlags, OS_FLAG_TOTALFILES) )
			m_OS.uTotalFiles = pOS->uTotalFiles;

	}

	m_OS.nOperation = nOperation;
	m_OS.bFirstFile = true;

	return 1;
}

int __stdcall Archive::OnQueryPassword (int nMode, ArchivePassword *pPassword)
{
   	if ( nMode == PASSWORD_RESET )
   	{
   		if ( m_lpLastUsedPassword )
   		{
   			free (m_lpLastUsedPassword);
   			m_lpLastUsedPassword = NULL;
   		}
   	}

   	if ( (nMode == PASSWORD_LIST) || (nMode == PASSWORD_FILE) )
   	{
   		bool bResult = true;

   		if ( !m_lpLastUsedPassword )
   		{
   			m_lpLastUsedPassword = StrCreate (512);

   			bResult = Info.InputBox (
   					(nMode == PASSWORD_LIST)?"Информация об именах файлов защищена паролем":"Содержимое файла защищено паролем",
   					"Введите пароль",
   					NULL,
   					NULL,
   					m_lpLastUsedPassword,
   					512,
   					NULL,
   					0
   					);

   			if ( !bResult )
   			{
   				StrFree (m_lpLastUsedPassword);
   				m_lpLastUsedPassword = NULL;
   			}
   		}

   		if ( m_lpLastUsedPassword && bResult )
   		{
   			strcpy (pPassword->lpBuffer, m_lpLastUsedPassword);
   			return TRUE;
   		}

   		return FALSE;
   	}

   	return FALSE;
}

static COORD c;

int __stdcall Archive::OnProcessFile (PluginPanelItem *item, const char *lpDestName)
{
	char *lpTemp;

   	m_pCurrentItem = item;

   	if ( m_OS.bFirstFile )
   	{
   		if ( !OptionIsOn (m_nMode, OPM_SILENT) )
   		{
   			doEmptyDialog (false, c);

   			if ( m_OS.nOperation == OPERATION_EXTRACT )
	   			Info.Text (c.X+5, c.Y+2, FarGetColor (COL_DIALOGTEXT), "Распаковка файла");

			if ( m_OS.nOperation == OPERATION_ADD )
	   			Info.Text (c.X+5, c.Y+2, FarGetColor (COL_DIALOGTEXT), "Добавление файла");

			if ( m_OS.nOperation == OPERATION_DELETE )
	   			Info.Text (c.X+5, c.Y+2, FarGetColor (COL_DIALOGTEXT), "Удаление файла");

   			Info.Text (c.X+5, c.Y+4, FarGetColor (COL_DIALOGTEXT), "в");

   			doIndicator (c.X+5, c.Y+6, 0);
   			doIndicator (c.X+5, c.Y+8, 0);

   			Info.Text (0, 0, 0, 0);
   		}

   		m_OS.bFirstFile = false;
   		m_OS.uTotalProcessedSize = 0;
   	}

   	//MessageBox (0, m_pCurrentItem->FindData.cFileName, m_pCurrentItem->FindData.cFileName, MB_OK);

   	if ( !OptionIsOn (m_nMode, OPM_SILENT) )
   	{
   		lpTemp = StrCreate (260);

   		memset (lpTemp, 32, 40);
   		Info.Text (c.X+5, c.Y+3, FarGetColor (COL_DIALOGTEXT), lpTemp);
   		Info.Text (c.X+5, c.Y+5, FarGetColor (COL_DIALOGTEXT), lpTemp);

   		strcpy (lpTemp, m_pCurrentItem->FindData.cFileName);

   		FSF.TruncPathStr (lpTemp, 40);
   		Info.Text (c.X+5, c.Y+3, FarGetColor (COL_DIALOGTEXT), lpTemp);

   		strcpy (lpTemp, lpDestName);
   		FSF.TruncPathStr (lpTemp, 40);

   		Info.Text (c.X+5, c.Y+5, FarGetColor (COL_DIALOGTEXT), lpTemp);

   		memset (lpTemp, 0, 260);

   		StrFree (lpTemp);

   		Info.Text (0, 0, 0, 0);
   	}

   	m_OS.uFileSize = m_pCurrentItem->FindData.nFileSizeHigh*0x100000000+m_pCurrentItem->FindData.nFileSizeLow;
   	m_OS.uProcessedSize = 0;

   	return TRUE;
}

int __stdcall Archive::OnProcessData (unsigned int uDataSize)
{
	m_OS.uTotalProcessedSize += uDataSize;
	m_OS.uProcessedSize += uDataSize;

	if ( m_pCurrentItem )
   	{
   		double div;
   		char szPercents[MAX_PATH];

   		if ( m_OS.uFileSize )
   			div = (double)m_OS.uProcessedSize/(double)m_OS.uFileSize;
   		else
   			div = 1;
   		if (div > 1)
   			div = 1;
   		dword dwPercent = (int)(div*40);
   		dword dwRealPercent = (int)(div*100);

   		if ( !OptionIsOn (m_nMode, OPM_SILENT) )
   		{
   			doIndicator (c.X+5, c.Y+6, dwPercent);

   			FSF.sprintf (szPercents, "%4u%%", dwRealPercent);
   			Info.Text (c.X+45, c.Y+6, FarGetColor (COL_DIALOGTEXT), szPercents);
   		}

   		if ( m_OS.uTotalSize )
       		div = (double)m_OS.uTotalProcessedSize/(double)m_OS.uTotalSize;
           else
   	    	div = 1;
   		if (div > 1)
   			div = 1;

   		dwPercent = (int)(div*40);
   		dwRealPercent = (int)(div*100);

   		if ( !OptionIsOn (m_nMode, OPM_SILENT) )
   		{
   			doIndicator (c.X+5, c.Y+8, dwPercent);

   			FSF.sprintf (szPercents, "%4u%%", dwRealPercent);
   			Info.Text (c.X+45, c.Y+8, FarGetColor (COL_DIALOGTEXT), szPercents);

   			Info.Text (0, 0, 0, 0);
   		}

   		char *lpTitle = StrCreate (260);

   		FSF.sprintf (lpTitle, "{%d%%} Распаковка - Far", (int)(div*100));

   		SetConsoleTitle (lpTitle);

   		StrFree (lpTitle);
   	}

   	if ( CheckForEsc () )
   	{
   		if ( !OptionIsOn (m_nMode, OPM_SILENT) )
   		{
   			Info.Text (c.X+5, c.Y+2, FarGetColor (COL_DIALOGTEXT), "Операция прерывается...");
   			Info.Text (0, 0, 0, 0);
   		}

   		return FALSE;
   	}

   	return TRUE;
}


int __stdcall Archive::ArchiveCallback (
		int nMsg,
		int nParam1,
		int nParam2
		)
{
	if ( nMsg == AM_NEED_PASSWORD )
		return OnQueryPassword (nParam1, (ArchivePassword*)nParam2);

	if ( nMsg == AM_START_OPERATION )
		return OnStartOperation (nParam1, (OperationStructPlugin *)nParam2); 

	if ( nMsg == AM_PROCESS_FILE )
		return OnProcessFile ((PluginPanelItem*)nParam1, (const char*)nParam2);

	if ( nMsg == AM_PROCESS_DATA )
		return OnProcessData ((unsigned int)nParam2);

	return 1;
}


int Archive::pGetArchiveFormatType ()
{
	GetArchiveFormatStruct GAF;

	GAF.hArchive = m_hArchive;

	if ( m_pPlugin->m_pfnPluginEntry (FID_GETARCHIVEFORMAT, (void*)&GAF) == NAERROR_SUCCESS )
		return GAF.nFormat;

	return 0;
}

char *Archive::pGetArchiveFormatName ()
{
	int nFormat = pGetArchiveFormatType ();

	if ( nFormat < m_pPlugin->m_ArchivePluginInfo.nFormats)
		return m_pPlugin->m_ArchivePluginInfo.pFormatInfo[nFormat].lpName;

	return NULL;
}

bool Archive::pExtract (
		PluginPanelItem *pItems,
		int nItemsNumber,
		const char *lpDestPath,
		const char *lpCurrentPath
		)
{
	ExtractStruct ES;

	ES.hArchive = m_hArchive;
	ES.pItems = pItems;
	ES.nItemsNumber = nItemsNumber;
	ES.lpDestPath = lpDestPath;
	ES.lpCurrentPath = lpCurrentPath;

	if ( m_pPlugin->m_pfnPluginEntry (FID_EXTRACT, (void*)&ES) == NAERROR_SUCCESS )
		return ES.bResult;

	return false;
}

bool Archive::pAddFiles (
		const char *lpSourcePath,
		const char *lpCurrentPath,
		PluginPanelItem *pItems,
		int nItemsNumber
		)
{
	AddStruct AS;

	AS.hArchive = m_hArchive;
	AS.lpSourcePath = lpSourcePath;
	AS.lpCurrentPath = lpCurrentPath;
	AS.pItems = pItems;
	AS.nItemsNumber = nItemsNumber;

	if ( m_pPlugin->m_pfnPluginEntry (FID_ADD, (void*)&AS) == NAERROR_SUCCESS )
		return AS.bResult;

	return false;
}



bool Archive::pDelete (
		PluginPanelItem *pItems,
		int nItemsNumber
		)
{
	DeleteStruct DS;

	DS.hArchive = m_hArchive;
	DS.pItems = pItems;
	DS.nItemsNumber = nItemsNumber;

	if ( m_pPlugin->m_pfnPluginEntry (FID_DELETE, (void*)&DS) == NAERROR_SUCCESS )
		return DS.bResult;

	return false;
}

bool Archive::pOpenArchive (
		int nMode
		)
{
	OpenArchiveStruct OA;

	OA.hArchive = m_hArchive;
	OA.nMode = nMode;
	OA.pfnCallback = (ARCHIVECALLBACK)m_pCallbackThunk;

	if ( m_pPlugin->m_pfnPluginEntry (FID_OPENARCHIVE, (void*)&OA) == NAERROR_SUCCESS )
		return OA.bResult;

	return false;
}

void Archive::pCloseArchive ()
{
	CloseArchiveStruct CA;

	CA.hArchive = m_hArchive;

	m_pPlugin->m_pfnPluginEntry (FID_CLOSEARCHIVE, (void*)&CA);
}

int Archive::pGetArchiveItem (
		ArchiveItemInfo *pItem
		)
{
	GetArchiveItemStruct GAI;

	GAI.hArchive = m_hArchive;
	GAI.pItem = pItem;

	if ( m_pPlugin->m_pfnPluginEntry (FID_GETARCHIVEITEM, (void*)&GAI) == NAERROR_SUCCESS )
		return GAI.nResult;

	return 1;
}


void Archive::pNotify (HANDLE hPanel, int nEvent, void *pEventData)
{
	NotifyStruct NS;

	NS.hArchive = m_hArchive;
	NS.hPanel = hPanel;
	NS.nEvent = nEvent;
	NS.pEventData = pEventData;

	m_pPlugin->m_pfnPluginEntry (FID_NOTIFY, (void*)&NS);
}

bool Archive::pGetDefaultCommand (
		int nCommand,
		char *lpCommand
		)
{
	return m_pPlugin->pGetDefaultCommand (
			pGetArchiveFormatType (),
			nCommand,
			lpCommand
			);
}
