enum enumUnpackFiles {
	ID_UP_TITLE,
	ID_UP_UNPACKTO,
	ID_UP_UNPACKTOEDIT,
	ID_UP_SEPARATOR1,
	ID_UP_UNPACKPASSWORD,
	ID_UP_UNPACKPASSWORDEDIT,
	ID_UP_SEPARATOR2,
	ID_UP_UNPACKWITHOUTPATHS,
	ID_UP_SEPARATOR3,
	ID_UP_UNPACK,
	ID_UP_CANCEL
};

bool dlgUnpackFiles(
		const TCHAR* lpDestPath,
		bool bMove,
		string& strResultDestPath,
		bool& bExtractWithoutPath
		)
{
	bool bResult = false;

	AddEndSlash(strResultDestPath);

	FarDialog D(-1, -1, 75, 13);

	D.DoubleBox(3, 1, 71, 11, bMove?_M(MUnpackFilesTitleMove):_M(MUnpackFilesTitle)); //0

	D.Text(5, 2, _M(MUnpackFilesUnpackTo)); //1
	D.Edit(5, 3, 65, lpDestPath, AUTO_LENGTH, _T("")); //2

	D.Separator(4);

	D.Text(5, 5, _M(MUnpackFilesPassword));
	D.PswEdit(5, 6, 40);

	D.Separator(7);

	D.CheckBox(5, 8, true, _M(MUnpackFilesWithoutPaths));

	D.Separator(9);

	D.Button(-1, 10, _M(MUnpackFilesUnpack));
	D.DefaultButton();

	D.Button(-1, 10, _M(MUnpackFilesCancel));

	if ( D.Run() == D.FirstButton() )
	{
		strResultDestPath = D.GetResultData(ID_UP_UNPACKTOEDIT);
		bExtractWithoutPath = D.GetResultCheck(ID_UP_UNPACKWITHOUTPATHS);

		bResult = true;
	}

	return bResult;
}
