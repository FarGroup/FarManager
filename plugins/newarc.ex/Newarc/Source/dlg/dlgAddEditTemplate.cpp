extern ArchiveModuleManager* pManager;

enum enumAddEditTemplate {
	ID_AET_TITLE,
	ID_AET_NAME,
	ID_AET_NAMEEDIT,
	ID_AET_SEPARATOR1,
	ID_AET_FORMAT,
	ID_AET_FORMATLIST,
	ID_AET_CONFIG,
	ID_AET_SEPARATOR2,
	ID_AET_ADDITIONALPARAMS,
	ID_AET_ADDITIONALPARAMSEDIT,
	ID_AET_SEPARATOR3,
	ID_AET_ADDCONFIRM,
	ID_AET_CANCEL
};


void SetVisibility(FarDialog* D, ArchiveFormat* pFormat)
{
	if ( pFormat->QueryCapability(AFF_SUPPORT_CONFIG_CREATE) && 
		 pFormat->QueryCapability(AFF_SUPPORT_INTERNAL_CREATE) )
		D->Enable(ID_AET_CONFIG, true);
	else
		D->Enable(ID_AET_CONFIG, false);
}

LONG_PTR __stdcall hndAddEditTemplate(
		FarDialog *D,
		int nMsg,
		int nParam1,
		LONG_PTR nParam2
		)
{
	ArchiveTemplate *ptpl = (ArchiveTemplate*)D->GetDlgData();

	bool bAdd = StrLength(ptpl->GetName()) == 0;

	if ( (nMsg == DN_LISTCHANGE) && (nParam1 == ID_AET_FORMATLIST) )
	{
		FarListPos pos;

		D->ListGetCurrentPos (ID_AET_FORMATLIST, &pos);

		if ( pos.SelectPos != -1 )
			SetVisibility(D, (ArchiveFormat*)D->ListGetData(ID_AET_FORMATLIST, pos.SelectPos));

		return TRUE;
	}

	if ( nMsg == DN_INITDIALOG )
	{
		int curpos = -1;
		int pos = 0;

		Array<ArchiveFormat*> formats;
		
		pManager->GetFormats(formats);

		ArchiveFormat* pDefaultFormat = nullptr;

		for (unsigned int i = 0; i < formats.count(); i++)
		{
			pDefaultFormat = formats[i];

			string strCommand;

			bool bHasAddCommand = pManager->GetCommand(pDefaultFormat, COMMAND_ADD, strCommand);

			if ( pDefaultFormat->QueryCapability(AFF_SUPPORT_INTERNAL_CREATE) || bHasAddCommand )
			{
				int index = D->ListAddStr(ID_AET_FORMATLIST, pDefaultFormat->GetName());

				D->ListSetDataEx(ID_AET_FORMATLIST, index, (void*)pDefaultFormat, sizeof(void*));
		
				if ( !bAdd && (ptpl->GetFormat() == pDefaultFormat) ) 
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

				D->ListSetCurrentPos(ID_AET_FORMATLIST, &arcPos);
			}

			D->SetTextPtr(ID_AET_NAMEEDIT, ptpl->GetName());
			D->SetTextPtr(ID_AET_ADDITIONALPARAMSEDIT, ptpl->GetParams());

			SetVisibility(D, ptpl->GetFormat());
		}
		else
			SetVisibility(D, pDefaultFormat);
	}



	if ( nMsg == DN_CLOSE )
	{
		if ( nParam1 == ID_AET_ADDCONFIRM )
		{
			ptpl->SetName(D->GetConstTextPtr(ID_AET_NAMEEDIT));
			ptpl->SetParams(D->GetConstTextPtr(ID_AET_ADDITIONALPARAMSEDIT));

			FarListPos pos;

			D->ListGetCurrentPos (ID_AET_FORMATLIST, &pos);

			if ( pos.SelectPos != -1 )
				ptpl->SetFormat((ArchiveFormat*)D->ListGetData(ID_AET_FORMATLIST, pos.SelectPos));

			return TRUE;
		}

		if ( nParam1 == ID_AET_CONFIG )
		{
			FarListPos pos;

			D->ListGetCurrentPos (ID_AET_FORMATLIST, &pos);

			if ( pos.SelectPos != -1 )
			{
				ArchiveFormat* pFormat = (ArchiveFormat*)D->ListGetData(ID_AET_FORMATLIST, pos.SelectPos);

				string strResult;

				if ( pFormat->Configure(ptpl->GetConfig(), strResult) )
					ptpl->SetConfig(strResult);
			}

			return FALSE;
		}
	}

	return D->DefDlgProc(nMsg, nParam1, nParam2);
}



bool dlgAddEditTemplate(ArchiveTemplate *ptpl, bool bAdd)
{
	if ( !bAdd && !ptpl->IsValid() )
		msgError(_T("this is an invalid template!"));

	FarDialog D(-1, -1, 55, 14);

	D.DoubleBox(3, 1, 51, 12, ( bAdd ) ? _M(MAddTemplateTitle) : _M(MModifyTemplateTitle)); 

	D.Text(5, 2, _M(MAddTemplateName)); 
	D.Edit(5, 3, 45, NULL, AUTO_LENGTH, _T("adsaf")); 

	D.Separator(4); 

	D.Text(5, 5, _M(MAddTemplateFormat));
	D.ComboBox(5, 6, 20, NULL, 0);
	D.SetFlags(DIF_DROPDOWNLIST);

	D.Button(27, 6, _T("Config"));

	D.Separator(7);

	D.Text(5, 8, _M(MAddTemplateAdditionalParams));
	D.Edit(5, 9, 45, NULL, AUTO_LENGTH, _T("adsaf"));

	D.Separator(10);

	D.Button(-1, 11, ( bAdd ) ? _M(MAddTemplateAdd) : _M(MAddTemplateConfirm));
	D.DefaultButton ();

	D.Button(-1, 11, _M(MAddTemplateCancel)); //10

	if ( D.Run(hndAddEditTemplate, ptpl) == ID_AET_ADDCONFIRM )
		return true;

	return false;
}
