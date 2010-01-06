bool dlgUnpackFiles (
		const TCHAR *lpDestPath,
		bool bMove,
		string& strResultDestPath //source and dest
		)
{
	bool bResult = false;

	AddEndSlash (strResultDestPath);

	FarDialog D (-1, -1, 75, 13);

	D.DoubleBox (3, 1, 71, 11, bMove?_M(MUnpackFilesTitleMove):_M(MUnpackFilesTitle)); //0

	D.Text (5, 2, _M(MUnpackFilesUnpackTo)); //1
	D.Edit (5, 3, 65, strResultDestPath, AUTO_LENGTH, _T("123")); //2

	D.Separator (4);

	D.Text (5, 5, _M(MUnpackFilesPassword));
	D.PswEdit (5, 6, 40);

	D.Separator (7);

	D.CheckBox (5, 8, false, _M(MUnpackFilesWithoutPaths));

	D.Separator (9);

	D.Button (-1, 10, _M(MUnpackFilesUnpack));
	D.DefaultButton ();

	D.Button (-1, 10, _M(MUnpackFilesCancel));

	if ( D.Run() == D.FirstButton() )
	{
		strResultDestPath = D.GetResultData(2);
		bResult = true;
	}

	return bResult;
}
