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
	m_lpListPassword = NULL;
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
	StrFree (m_lpListPassword);
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

	memset (lpTemp, 177, 44);
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

	doFrame (c.X, c.Y, 54, 10+Diff, " Распаковка ", true);
}


bool CheckForEsc()
{
	bool EC=false;
	INPUT_RECORD rec;
	DWORD ReadCount;
	while (true)
	{
		PeekConsoleInput(GetStdHandle (STD_INPUT_HANDLE),&rec,1,&ReadCount);
		if (ReadCount==0)
			break;

		ReadConsoleInput(GetStdHandle (STD_INPUT_HANDLE),&rec,1,&ReadCount);
		if (rec.EventType==KEY_EVENT)
		{
			if ( (rec.Event.KeyEvent.wVirtualScanCode == VK_ESCAPE) &&
				 rec.Event.KeyEvent.bKeyDown )
			{
				EC=true;
			}
		}
	}
	return(EC);
}



int __stdcall Archive::ArchiveCallback (
		int nMsg,
		int nParam1,
		int nParam2
		)
{
	char *lpTemp;

	static COORD c;

	if ( nMsg == AM_NEED_PASSWORD )
	{
		ArchivePassword *pPassword = (ArchivePassword*)nParam2;

		if ( nParam1 == PASSWORD_LIST )
		{
			bool bResult = true;

			if ( !m_lpListPassword )
			{
				m_lpListPassword = StrCreate (512);

				bResult = Info.InputBox (
						"Информация об именах файлов защищена паролем",
						"Введите пароль",
						NULL,
						NULL,
						m_lpListPassword,
						512,
						NULL,
						0
						);

				if ( !bResult )
					StrFree (m_lpListPassword);
			}

			if ( m_lpListPassword && bResult )
			{
				strcpy (pPassword->lpBuffer, m_lpListPassword);
				return TRUE;
			}

			return FALSE;
		}

		if ( nParam1 == PASSWORD_FILE )
		{
			bool bResult = Info.InputBox (
					"Содержимое файла защищено паролем",
					"Введите пароль",
					NULL,
					NULL,
					pPassword->lpBuffer,
					pPassword->dwBufferSize,
					NULL,
					0
					);

			if ( bResult )
			{
				m_lpLastUsedPassword = StrReplace (
						m_lpLastUsedPassword,
						pPassword->lpBuffer
						);

				return TRUE;
			}
		}

		return FALSE;
	}

	if ( nMsg == AM_START_EXTRACT_FILE )
	{
		m_pCurrentItem = (PluginPanelItem*)nParam1;

		if ( m_bFirstFile )
		{
			if ( !OptionIsOn (m_nMode, OPM_SILENT) )
			{
				doEmptyDialog (false, c);

				Info.Text (c.X+5, c.Y+2, FarGetColor (COL_DIALOGTEXT), "Распаковка файла");
				Info.Text (c.X+5, c.Y+4, FarGetColor (COL_DIALOGTEXT), "в");

				doIndicator (c.X+5, c.Y+6, 0);
				doIndicator (c.X+5, c.Y+8, 0);

				Info.Text (0, 0, 0, 0);
			}

			m_bFirstFile = false;
			m_nTotalSize2 = 0;
		}

		if ( !OptionIsOn (m_nMode, OPM_SILENT) )
		{
			lpTemp = StrCreate (260);

			memset (lpTemp, 32, 44);
			Info.Text (c.X+5, c.Y+3, FarGetColor (COL_DIALOGTEXT), lpTemp);
			Info.Text (c.X+5, c.Y+5, FarGetColor (COL_DIALOGTEXT), lpTemp);

			strcpy (lpTemp, m_pCurrentItem->FindData.cFileName);

			FSF.TruncPathStr (lpTemp, 44);
			Info.Text (c.X+5, c.Y+3, FarGetColor (COL_DIALOGTEXT), lpTemp);

			strcpy (lpTemp, (char*)nParam2);
			FSF.TruncPathStr (lpTemp, 44);

			Info.Text (c.X+5, c.Y+5, FarGetColor (COL_DIALOGTEXT), lpTemp);

			memset (lpTemp, 0, 260);

			StrFree (lpTemp);

			Info.Text (0, 0, 0, 0);
		}

		m_nTotalSize = 0;
	}

	if ( nMsg == AM_PROCESS_DATA )
	{
		m_nTotalSize += nParam2;
		m_nTotalSize2 += nParam2;

		double div;

		if (m_pCurrentItem->FindData.nFileSizeLow>0)
			div = (double)m_nTotalSize/(double)m_pCurrentItem->FindData.nFileSizeLow;
		else
			div = 1;
		dword dwPercent = (int)(div*44);

		if ( !OptionIsOn (m_nMode, OPM_SILENT) )
			doIndicator (c.X+5, c.Y+6, dwPercent);

		if (m_nFullSize>0)
        	div = (double)m_nTotalSize2/(double)m_nFullSize;
        else
        	div = 1;
		dwPercent = (int)(div*44);

		if ( !OptionIsOn (m_nMode, OPM_SILENT) )
		{
			doIndicator (c.X+5, c.Y+8, dwPercent);
			Info.Text (0, 0, 0, 0);
		}

		char *lpTitle = StrCreate (260);

		FSF.sprintf (lpTitle, "{%d%%} Распаковка - Far", (int)(div*100));

		SetConsoleTitle (lpTitle);

		StrFree (lpTitle);

		if ( !OptionIsOn (m_nMode, OPM_SILENT) )
		{
			if ( CheckForEsc () )
			{
				Info.Text (c.X+5, c.Y+2, FarGetColor (COL_DIALOGTEXT), "Операция прерывается...");
				Info.Text (0, 0, 0, 0);

				return 0;
			}
		}
	}

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


bool Archive::pOpenArchive (
		int nMode
		)
{
	m_bFirstFile = true;

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
