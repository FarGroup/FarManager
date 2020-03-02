
LONG_PTR __stdcall hndCommandLinesAndParams(FarDialog* D, int nMsg, int nParam1, LONG_PTR nParam2)
{
	ArchiveFormat* pFormat = (ArchiveFormat*)D->GetDlgData();

	if ( nMsg == DN_CLOSE )
	{
		if ( nParam1 == D->FirstButton()+2 )
		{
			unsigned int uStartIndex = 1;

			for (int i = 0; i < MAX_COMMANDS; i++)
			{
				string strCommand;
				bool bEnabled;

				pFormat->GetDefaultCommand(i, strCommand, bEnabled);

				D->SetCheck(uStartIndex, bEnabled?BSTATE_CHECKED:BSTATE_UNCHECKED);
				D->SetTextPtr(uStartIndex+2, strCommand);

				uStartIndex += 3;
			}

			return FALSE;
		}
	}

	return D->DefDlgProc(nMsg, nParam1, nParam2);
}

void dlgCommandLinesAndParams(ArchiveManagerConfig* pCfg, ArchiveFormat* pFormat)
{
	int nHeight = 19;
	int Y = 2;

	string strTitle;

	FarDialog D(-1, -1, 79, nHeight);

	strTitle.Format(_M(MCommandLinesAndParamsTitleDialog), pFormat->GetName());

	D.DoubleBox (3, 1, 75, nHeight-2, strTitle); //0
	
	ArchiveFormatConfig* pCurrentCfg = pCfg->GetFormatConfig(pFormat);

	for (int i = 0; i < MAX_COMMANDS; i++)
	{
		bool bEnabled;
		string strCommand;

		if ( pCurrentCfg ) 
			pCurrentCfg->GetCommand(i, strCommand, bEnabled);
		else
			pFormat->GetDefaultCommand(i, strCommand, bEnabled);

		D.CheckBox(5, Y, bEnabled);
		D.Text(9, Y, _M(MSG_dlgCLAP_S_EXTRACT+i));

		D.Edit(33, Y++, 42, strCommand);
	}

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

	if ( D.Run(hndCommandLinesAndParams, (void*)pFormat) == D.FirstButton() )
	{
		ArchiveFormatConfig* pNewCfg = pCurrentCfg?pCurrentCfg:(new ArchiveFormatConfig);

		pNewCfg->SetFormat(pFormat);

		unsigned int uStartIndex = 1;

		for (unsigned int i = 0; i < MAX_COMMANDS; i++)
		{
			pNewCfg->SetCommand(i, D.GetResultData(uStartIndex+2), D.GetCheck(uStartIndex));
			uStartIndex += 3;
		}

		if ( !pCurrentCfg )
			pCfg->AddFormatConfig(pNewCfg);

		pCfg->Save(SAVE_CONFIGS);
	}
}
