/*

   г======================== Предупреждение ========================¬
   ¦                      Файл уже существует                       ¦
   ¦ installer.png                                                  ¦
   ¦----------------------------------------------------------------¦
   ¦ Новый                                 3947 27.11.2009 04:49:00 ¦
   ¦ Существующий                          3984 27.11.2009 04:44:09 ¦
   ¦----------------------------------------------------------------¦
   ¦ [ ] Запомнить выбор                                            ¦
   ¦----------------------------------------------------------------¦
   ¦[ Вместо ]  [ Пропустить ]  [ Имя ]  [ Дописать ]  [ Отменить ] ¦
   L================================================================-
*/


bool GetFileInfo(const TCHAR* lpFileName, WIN32_FIND_DATA* pData)
{
	HANDLE hSearch = FindFirstFile(lpFileName, pData);

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		FindClose(hSearch);

		return true;
	}

	return false;
}

void TimeToStr(const FILETIME& ftime, string& strTime)
{
	SYSTEMTIME stime;
	FILETIME ltime;

	FileTimeToLocalFileTime(&ftime, &ltime);
	FileTimeToSystemTime(&ltime, &stime);

	strTime.Format(_T("%02d.%02d.%04d %02d:%02d"), stime.wDay, stime.wMonth, stime.wYear, stime.wHour, stime.wMinute, stime.wSecond);
}


int msgFileAlreadyExists(const TCHAR* lpFileName, const ArchiveItem* pItem)
{
	WIN32_FIND_DATA fdata;
	memset(&fdata, 0, sizeof(WIN32_FIND_DATA));

	GetFileInfo(lpFileName, &fdata);

	if ( (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
		return PROCESS_OVERWRITE;

	FarDialog D(-1, -1, 72, 13);

	D.SetDialogFlags(FDLG_WARNING);

	D.DoubleBox(3, 1, 68, 11, _T("Warning")); //0

	D.Text(5, 2, 66, _T("File already exists")); //1
	D.SetFlags(DIF_CENTERTEXT);

	D.Text(5, 3, 66, lpFileName); //2
	D.SetFlags(DIF_CENTERTEXT);

	D.Separator(4); //3

	string strTemp;
	string strSize;
	string strTime;

	TimeToStr(pItem->ftLastWriteTime, strTime);
	strSize.Format(_T("%I64u"), pItem->nFileSize);
	strTemp.Format(_T("%-17s %25.25s %s"), _T("&New"), strSize.GetString(), strTime.GetString());
	
	D.Text(5, 5, strTemp); //4

	TimeToStr(fdata.ftLastWriteTime, strTime);
	strSize.Format(_T("%I64u"), ((unsigned __int64)fdata.nFileSizeHigh << 32)+(unsigned __int64)fdata.nFileSizeLow);
	strTemp.Format(_T("%-17s %25.25s %s"), _T("E&xisting"), strSize.GetString(), strTime.GetString());
	D.Text(5, 6, strTemp); //5

	D.Separator(7); //6

	D.CheckBox(5, 8, false, _T("&Remember choice")); //7

	D.Separator(9); //8

	D.Button(0, 10, _T("&Overwrite"));
	D.SetFlags(DIF_CENTERGROUP);
	D.DefaultButton();

	D.Button(0, 10, _T("&Skip"));
	D.SetFlags(DIF_CENTERGROUP);

	D.Button(0, 10, _T("&Cancel"));
	D.SetFlags(DIF_CENTERGROUP);
	
	int nResult = D.Run();
	
	if ( nResult != -1 )
	{
		bool bAll = D.GetResultCheck(7);
		int nIndex = D.FirstButton();

		if ( nResult == nIndex+0 )
			return bAll?PROCESS_OVERWRITE_ALL:PROCESS_OVERWRITE;

		if ( nResult == nIndex+1 )
			return bAll?PROCESS_SKIP_ALL:PROCESS_SKIP;

		if ( nResult == nIndex+2 )
			return PROCESS_CANCEL;
	}

	return PROCESS_CANCEL;
}