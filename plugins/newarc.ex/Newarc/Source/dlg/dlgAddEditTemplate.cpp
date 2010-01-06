extern ArchiveModuleManager* pManager;

LONG_PTR __stdcall hndAddEditTemplate (
		FarDialog *D,
		int nMsg,
		int nParam1,
		LONG_PTR nParam2
		)
{
	ArchiveTemplate *ptpl = (ArchiveTemplate*)D->GetDlgData();

	bool bAdd = StrLength(ptpl->GetName()) == 0; //BUGBUG

	if ( nMsg == DN_INITDIALOG )
	{
		int curpos = -1;
		int pos = 0;

		Array<ArchiveFormat*> formats;
		
		pManager->GetFormats(formats);

		for (int i = 0; i < formats.count(); i++)
		{
			ArchiveFormat* pFormat = formats[i];

			string strCommand;
			pFormat->GetDefaultCommand(COMMAND_ADD, strCommand);

			if ( pFormat->QueryCapability(AFF_SUPPORT_INTERNAL_CREATE) || !strCommand.IsEmpty() )
			{
				int index = D->ListAddStr (5, pFormat->GetName());

				D->ListSetDataEx (5, index, (void*)pFormat, sizeof(void*));
		
				if ( !bAdd && (ptpl->HasFormat(pFormat)) ) 
					curpos = pos;

				pos++;
			}
		}

		if ( !bAdd )
		{
			if ( curpos != -1 )
			{
				FarListPos arcPos;

				arcPos.SelectPos = curpos;
				arcPos.TopPos = -1;

				D->ListSetCurrentPos (5, &arcPos);
			}

			D->SetTextPtr (2, ptpl->GetName());
			D->SetTextPtr (7, ptpl->GetParams());
		}
	}

	if ( nMsg == DN_CLOSE )
	{
		if ( nParam1 == 9 )
		{
			ptpl->SetName(D->GetConstTextPtr (2));
			ptpl->SetParams(D->GetConstTextPtr (7));

			FarListPos pos;

			D->ListGetCurrentPos (5, &pos);

			if ( pos.SelectPos != -1 )
				ptpl->SetFormat((ArchiveFormat*)D->ListGetData (5, pos.SelectPos));
		}
	}

	return D->DefDlgProc (nMsg, nParam1, nParam2);
}


bool dlgAddEditTemplate (ArchiveTemplate *ptpl, bool bAdd)
{
	if ( !ptpl->IsValid() )
		msgError(_T("this is an invalid template, reset!"));

	FarDialog D(-1, -1, 55, 11);

	D.DoubleBox (3, 1, 51, 9, ( bAdd ) ? _M(MAddTemplateTitle) : _M(MModifyTemplateTitle)); //0

	D.Text (5, 2, _M(MAddTemplateName)); //1
	D.Edit (5, 3, 45); //2

	D.Separator (4); //3

	D.Text (5, 5, _M(MAddTemplateFormat)); //4
	D.ComboBox (5, 6, 15, NULL, 0); //5
	D.SetFlags (DIF_DROPDOWNLIST);

//	const ArchiveFormatInfo *info = pModule?pModule->GetArchiveFormatInfo (ptpl->uid):NULL;

//	if ( info && OptionIsOn(info->dwFlags, AFF_SUPPORT_INTERNAL_CONFIG) )
//		D.Button (22, 5, "Internal config");
//	else
//	{
		D.Text (22, 5, _M(MAddTemplateAdditionalParams)); //6
		D.Edit (22, 6, 27); //7
//	}

	D.Separator(7); //8

	D.Button(-1, 8, ( bAdd ) ? _M(MAddTemplateAdd) : _M(MAddTemplateConfirm)); //9
	D.DefaultButton ();

	D.Button(-1, 8, _M(MAddTemplateCancel)); //10


	if ( D.Run(hndAddEditTemplate, ptpl) == 9 )
		return true;

	return false;
}
