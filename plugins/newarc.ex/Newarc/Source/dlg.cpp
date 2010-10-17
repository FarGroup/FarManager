
#ifdef UNICODE
static wchar_t cBoxSymbols[] = {0x2551, 0x0000, 0x2550, 0x0000, 0x2554, 0x0000, 0x255D, 0x0000, 0x255A, 0x0000, 0x2557, 0x0000};
#else
static char cBoxSymbols[] = {'º', 0, 'Í', 0, 'É', 0, '¼', 0, 'È', 0, '»', 0};
#endif

#define FarGetColor(Index) (BYTE)Info.AdvControl (Info.ModuleNumber, ACTL_GETCOLOR, (void*)Index)

void doFrame (int X, int Y, int Width, int Height, const TCHAR *Header, bool Shadow)
{
	int X2 = X+Width-1;
	int Y2 = Y+Height;

	TCHAR *Line = new TCHAR[Width+1];
	
	for (int i = 0; i < Width; i++)
		Line[i] = _T(' ');

	Line[Width] = 0;

	int Color = FarGetColor (COL_DIALOGTEXT);

	for (int y = Y; y <= Y2; y++)
	{
		Info.Text (X,y,Color,Line);

		if (Shadow)
		{
			Info.Text (X2+1, y+1, 0, _T(" "));
			Info.Text (X2+2, y+1, 0, _T(" "));
		}
	}

	for (int i = 0; i < Width+1; i++)
		Line[i] = 0;

	for (int i = 0; i < Width-6; i++)
		Line[i] = cBoxSymbols[2];

	Info.Text (X+3, Y+1, Color, Line);
	Info.Text (X+3, Y2-1, Color, Line);

	for (int i = 0; i < Width+1; i++)
		Line[i] = 0;

	for (int i = 0; i < Width-1; i++)
		Line[i] = _T(' ');

	if (Shadow)
		Info.Text (X+3, Y2+1, 0, Line);


	for (int y = Y+1; y <= Y2-1; y++)
   	{
		Info.Text (X+3, y, Color, &cBoxSymbols[0]);
		Info.Text (X2-3, y, Color, &cBoxSymbols[0]);
	}

	Color = FarGetColor(COL_DIALOGBOX);

	Info.Text (X+3, Y+1, Color, &cBoxSymbols[4]);
	Info.Text (X2-3, Y2-1, Color, &cBoxSymbols[6]);
	Info.Text (X+3, Y2-1, Color, &cBoxSymbols[8]);
	Info.Text (X2-3, Y+1, Color, &cBoxSymbols[10]);

	if (Header)
	{
		Color = FarGetColor (COL_DIALOGBOXTITLE);
		Info.Text (X+(Width-StrLength(Header))/2,Y+1,Color,Header);
	}

	delete [] Line;
}

#ifdef UNICODE
static wchar_t cIndicator[] = {0x2591, 0x0000, 0x2588, 0x0000};
#else
static char cIndicator[] = {177, 0, 219, 0};
#endif

void OperationDialog::SetFileName(bool bTotal, const TCHAR* lpFileName)
{
	string strTemp;

	strTemp.Format(_T("%-45s"), FSF.TruncPathStr(lpFileName, 45));

	if ( bTotal )
		Info.Text(m_Coord.X+5, m_Coord.Y+3, FarGetColor(COL_DIALOGTEXT), strTemp);
	else
		Info.Text(m_Coord.X+5, m_Coord.Y+5, FarGetColor(COL_DIALOGTEXT), strTemp);
}

void OperationDialog::SetIndicator(bool bTotal, double dRatio)
{
	if ( dRatio < 0 )
		dRatio = 0;

	if ( dRatio > 1 )
		dRatio = 1;

    DWORD dwPercent = (DWORD)(dRatio*40);
    DWORD dwRealPercent = (DWORD)(dRatio*100);

	TCHAR *lpTemp = new TCHAR[100];

	for (unsigned int i = 0; i < 40; i++)
		lpTemp[i] = cIndicator[0];

	for (unsigned int i = 0; i < dwPercent; i++)
		lpTemp[i] = cIndicator[2];

	lpTemp[40] = 0;

	string strPercent;
	strPercent.Format(_T("%4u%%"), dwRealPercent);

	if ( bTotal )
	{
		Info.Text(m_Coord.X+5, m_Coord.Y+6, FarGetColor (COL_DIALOGTEXT), lpTemp);
		Info.Text(m_Coord.X+45, m_Coord.Y+6, FarGetColor (COL_DIALOGTEXT), strPercent);
	}
	else
	{
		Info.Text(m_Coord.X+5, m_Coord.Y+8, FarGetColor (COL_DIALOGTEXT), lpTemp);
		Info.Text(m_Coord.X+45, m_Coord.Y+8, FarGetColor (COL_DIALOGTEXT), strPercent);
	}

	delete [] lpTemp;

	if ( bTotal )
	{
		string strTitle;
		strTitle.Format(_M(MProcessDataWindowTitle), dwRealPercent);

		if ( strTitle != m_strPanelTitle )
		{
			SetConsoleTitle(strTitle);
			m_strPanelTitle = strTitle;
		}
	}
}

void OperationDialog::Show(const TCHAR* lpTitle)
{
	CONSOLE_SCREEN_BUFFER_INFO SInfo;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &SInfo);


	SHORT sWidth = SInfo.srWindow.Right-SInfo.srWindow.Left+1;
	SHORT sHeight = SInfo.srWindow.Bottom-SInfo.srWindow.Top+1;

	m_Coord.X = sWidth/2-28;
	m_Coord.Y = sHeight/2-5;

	doFrame (m_Coord.X, m_Coord.Y, 55, 10, lpTitle, true);

	/*
	string strSubTitle;

	strSubTitle.Format(_T("40.40s"), m_strSubTitle.GetString()); //BADBAD
	Info.Text(m_Coord.X+5, m_Coord.Y+2, FarGetColor (COL_DIALOGTEXT), strSubTitle);
	  */
	Info.Text(m_Coord.X+5, m_Coord.Y+4, FarGetColor (COL_DIALOGTEXT), _M(MProcessFileTo));

	SetIndicator(false, 0);
	SetIndicator(true, 0);
}