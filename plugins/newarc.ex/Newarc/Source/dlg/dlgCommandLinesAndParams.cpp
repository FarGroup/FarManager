//pFormat->GetDefaultCommand(command, strCommand, bEnabled); \


#define DECLARE_COMMAND(str, command) \
	{\
		D.CheckBox(5, Y, bEnabled); \
		D.Text (9, Y, str); \
		D.Edit (33, Y++, 42, cfg.pArchiveCommands->Commands[command]); \
	}

void dlgCommandLinesAndParams(ArchiveFormat* pFormat)
{
	int nHeight = 19;
	int Y = 2;

	bool bEnabled = false;

	string strCommand;
	string strTitle;

	FarDialog D(-1, -1, 79, nHeight);

	strTitle.Format(_M(MCommandLinesAndParamsTitleDialog), pFormat->GetName());

	D.DoubleBox (3, 1, 75, nHeight-2, strTitle); //0

	ArchiveFormatCommands* pCommands = nullptr;
	std::map<const ArchiveFormat*, ArchiveFormatCommands*>::iterator itr = cfg.pArchiveCommands.find(pFormat);

	if ( itr != cfg.pArchiveCommands.end() )
		pCommands = itr->second;

	for (int i = 0; i < MAX_COMMANDS; i++)
	{
		D.CheckBox(5, Y, bEnabled);
		D.Text(9, Y, _M(MSG_dlgCLAP_S_EXTRACT+i));

		string strCommand;

		if ( pCommands ) 
			strCommand = pCommands->Commands[i];
		else
			pFormat->GetDefaultCommand(i, strCommand, bEnabled);

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

	if ( D.Run() == D.FirstButton() )
	{
		unsigned int uStartIndex = 3;

		for (unsigned int i = 0; i < MAX_COMMANDS; i++)
		{
			if ( pCommands )
				pCommands->Commands[i] = D.GetResultData(uStartIndex);
			else
			{
				pCommands = new ArchiveFormatCommands;

				pCommands->pFormat = pFormat;
				pCommands->Commands[i] = D.GetResultData(uStartIndex);

				cfg.pArchiveCommands.insert(std::pair<const ArchiveFormat*, ArchiveFormatCommands*>(pFormat, pCommands));
			}

			uStartIndex += 3;
		}

		pManager->SaveCommands(_T("commands.ini")); //не место этому тут
	}
}
