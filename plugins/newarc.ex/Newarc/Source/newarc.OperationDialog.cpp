#include "newarc.h"

#ifdef UNICODE
static wchar_t cBoxSymbols[] = {0x2551, 0x0000, 0x2550, 0x0000, 0x2554, 0x0000, 0x255D, 0x0000, 0x255A, 0x0000, 0x2557, 0x0000};
static wchar_t cIndicator[] = {0x2591, 0x0000, 0x2588, 0x0000};
#else
static char cBoxSymbols[] = {186, 0, 205, 0, 201, 0, 188, 0, 200, 0, 187, 0};
static char cIndicator[] = {177, 0, 219, 0};
#endif

#define FarGetColor(Index) (BYTE)Info.AdvControl (Info.ModuleNumber, ACTL_GETCOLOR, (void*)Index)

void doFrame(int X, int Y, unsigned int uWidth, unsigned int uHeight, const TCHAR* lpHeader, bool bShadow)
{
	int X2 = X+uWidth-1;
	int Y2 = Y+uHeight;

	TCHAR* pLine = new TCHAR[uWidth+1];
	
	for (unsigned int i = 0; i < uWidth; i++)
		pLine[i] = _T(' ');

	pLine[uWidth] = 0;

	int Color = FarGetColor(COL_DIALOGTEXT);

	for (int y = Y; y <= Y2; y++)
	{
		Info.Text(X, y, Color, pLine);

		if ( bShadow )
		{
			Info.Text(X2+1, y+1, 0, _T(" "));
			Info.Text(X2+2, y+1, 0, _T(" "));
		}
	}

	for (unsigned int i = 0; i < uWidth+1; i++)
		pLine[i] = 0;

	for (unsigned int i = 0; i < uWidth-6; i++)
		pLine[i] = cBoxSymbols[2];

	Info.Text(X+3, Y+1, Color, pLine);
	Info.Text(X+3, Y2-1, Color, pLine);

	for (unsigned int i = 0; i < uWidth+1; i++)
		pLine[i] = 0;

	for (unsigned int i = 0; i < uWidth-1; i++)
		pLine[i] = _T(' ');

	if ( bShadow )
		Info.Text(X+3, Y2+1, 0, pLine);

	for (int y = Y+1; y <= Y2-1; y++)
	{
		Info.Text(X+3, y, Color, &cBoxSymbols[0]);
		Info.Text(X2-3, y, Color, &cBoxSymbols[0]);
	}

	Color = FarGetColor(COL_DIALOGBOX);

	Info.Text(X+3, Y+1, Color, &cBoxSymbols[4]);
	Info.Text(X2-3, Y2-1, Color, &cBoxSymbols[6]);
	Info.Text(X+3, Y2-1, Color, &cBoxSymbols[8]);
	Info.Text(X2-3, Y+1, Color, &cBoxSymbols[10]);

	if ( lpHeader )
	{
		Color = FarGetColor(COL_DIALOGBOXTITLE);
		Info.Text(X+(uWidth-StrLength(lpHeader))/2, Y+1, Color, lpHeader);
	}

	delete [] pLine;
}

//ratio 0..1

void doIndicator(int X, int Y, unsigned int nWidth, double dPercent)
{
	unsigned int uIndicatorPercent = (unsigned int)(dPercent*nWidth);

	TCHAR* pIndicator = new TCHAR[nWidth+1];

	for (unsigned int i = 0; i < nWidth; i++)
		pIndicator[i] = cIndicator[0];

	for (unsigned int i = 0; i < uIndicatorPercent; i++)
		pIndicator[i] = cIndicator[2];

	pIndicator[nWidth] = 0;

	Info.Text(X, Y, FarGetColor(COL_DIALOGTEXT), pIndicator);
}

OperationDialog::OperationDialog()
{
	m_dPercent = 0;
	m_dTotalPercent = 0;
}

void OperationDialog::SetOperation(int nOperation, int nStage)
{
	m_nOperation = nOperation;
	m_nStage = nStage;
}

void OperationDialog::SetPercent(double dPercent, double dTotalPercent)
{
	if ( dPercent > 1 )
		dPercent = 1;

	if ( dTotalPercent > 1 )
		dTotalPercent = 1;

	if ( dPercent < 0 )
		dPercent = 0;

	if ( dTotalPercent < 0 )
		dTotalPercent = 0;

	m_dPercent = dPercent;
	m_dTotalPercent = dTotalPercent;
}

void OperationDialog::SetTitle(string strTitle)
{
	m_strTitle = strTitle;
}

void OperationDialog::SetSrcFileName(string strFileName)
{
	m_strSrcFileName = strFileName;
}

void OperationDialog::SetDestFileName(string strFileName)
{
	m_strDestFileName = strFileName;
}

void OperationDialog::SetShowSingleFileProgress(bool bShow)
{
	m_bShowSingleFileProgress = bShow;
}

void OperationDialog::Show()
{
	CONSOLE_SCREEN_BUFFER_INFO SInfo;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &SInfo);

	SHORT sWidth = SInfo.srWindow.Right-SInfo.srWindow.Left+1;
	SHORT sHeight = SInfo.srWindow.Bottom-SInfo.srWindow.Top+1;

	int X = sWidth/2-28;
	int Y = sHeight/2-5;

	Info.Text(0, 0, 0, nullptr);

	string strTitle;

	switch ( m_nOperation ) {

	case OPERATION_ADD:
		strTitle = _T("Adding");
		break;

	case OPERATION_EXTRACT:
		strTitle = _T("Extracting");
		break;
	};


	if ( m_bShowSingleFileProgress )
		doFrame(X, Y, 55, 10, strTitle, true);
	else
		doFrame(X, Y, 55, 8, strTitle, true);

	string strStage;

	switch ( m_nStage ) {

	case STAGE_EXTRACTING:
		strStage = _T("Extracting file...");
		break;
	
	case STAGE_SKIPPING:
		strStage = _T("Skipping file...");
		break;

	case STAGE_ADDING:
		strStage = _T("Adding file...");
		break;

	case STAGE_UPDATING:
		strStage = _T("Updating file...");
		break;
	}

	Info.Text(X+5, Y+2, FarGetColor(COL_DIALOGTEXT), strStage);

	string strSrc = m_strSrcFileName;
	farTruncPathStr(strSrc, 45);

	string strDest = m_strDestFileName;
	farTruncPathStr(strDest, 45);

	string strTemp;

	strTemp.Format(_T("%-45s"), strSrc.GetString());
	Info.Text(X+5, Y+3, FarGetColor(COL_DIALOGTEXT), strTemp);

	if ( m_nStage != STAGE_SKIPPING )
	{
		Info.Text(X+5, Y+4, FarGetColor(COL_DIALOGTEXT), _M(MProcessFileTo));

		strTemp.Format(_T("%-45s"), strDest.GetString());
		Info.Text(X+5, Y+5, FarGetColor(COL_DIALOGTEXT), strTemp);
	}

	if ( m_bShowSingleFileProgress )
	{
		doIndicator(X+5, Y+6, 40, m_dPercent);
		doIndicator(X+5, Y+8, 40, m_dTotalPercent);
	}
	else
		doIndicator(X+5, Y+6, 40, m_dTotalPercent);

	Info.Text(0, 0, 0, nullptr);
}


