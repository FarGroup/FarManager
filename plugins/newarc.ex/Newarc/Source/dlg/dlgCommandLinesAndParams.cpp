#define DECLARE_COMMAND(str, command) \
	{\
		pFormat->GetDefaultCommand(command, strCommand, bEnabled); \
		D.CheckBox(5, Y, bEnabled); \
		D.Text (9, Y, str); \
		D.Edit (33, Y++, 42, strCommand); \
	}

void dlgCommandLinesAndParams(ArchiveFormat* pFormat)
{
	int nHeight = 19;
	int Y = 2;

	bool bEnabled;

	string strCommand;
	string strTitle;

	FarDialog D(-1, -1, 79, nHeight);

	strTitle.Format(_M(MCommandLinesAndParamsTitleDialog), pFormat->GetName());

	D.DoubleBox (3, 1, 75, nHeight-2, strTitle); //0

	for (int i = 0; i < 11; i++)
		DECLARE_COMMAND (_M(MSG_dlgCLAP_S_EXTRACT+i), i);

	D.Separator (Y++);

	D.Text (5, Y, _M(MCommandLinesAndParamsFileExtension));
	D.Edit (25, Y, 10, pFormat->GetDefaultExtention());

	D.Text (40, Y, _M(MCommandLinesAndParamsAllFilesMask));
	D.Edit (60, Y++, 10);

	D.Separator (Y++);

	D.Button (-1, Y, _M(MSG_cmn_B_OK));
	D.DefaultButton ();

	D.Button (-1, Y, _M(MSG_cmn_B_CANCEL));
	D.Button (-1, Y++, _M(MReset));

	if ( D.Run() == D.FirstButton() )
	{
	//	SaveArchiveModuleParams (pModule, nFormat);

		/*HKEY hKey;

		string strRegKey;

		strRegKey.Format(
				_T("%s\\newarc\\formats\\%s"),
				Info.RootKey,
				GUID2STR (info->uid)
				);

		if ( RegCreateKeyEx (
				HKEY_CURRENT_USER,
				strRegKey,
				0,
				NULL,
				0,
				KEY_WRITE,
				NULL,
				&hKey,
				NULL
				) == ERROR_SUCCESS )
		{
			for (int i = 0; i < 11; i++)
			{
				RegSetStringValue (
						hKey,
						pCommandNames[i],
						D.GetResultData(2+i*2)
						);
			}

			RegCloseKey (hKey);
		}*/
	}
}
