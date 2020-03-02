enum enumModifyCreateArchive {
	ID_MCA_TITLE,
	ID_MCA_NAME,
	ID_MCA_NAMEEDIT,
	ID_MCA_SEPARATOR1,
	ID_MCA_TEMPLATE,
	ID_MCA_TEMPLATELIST,
	ID_MCA_ADD,
	ID_MCA_REMOVE,
	ID_MCA_EDIT,
	ID_MCA_SEPARATOR2,
	ID_MCA_FORMAT,
	ID_MCA_FORMATLIST,
	ID_MCA_CONFIGURE,
	ID_MCA_PARAMS,
	ID_MCA_PARAMSEDIT,
	ID_MCA_SEPARATOR3,
	ID_MCA_PASSWORD,
	ID_MCA_PASSWORDEDIT,
	ID_MCA_CONFIRM,
	ID_MCA_CONFIRMEDIT,
	ID_MCA_SEPARATOR4,
	ID_MCA_PASSWORDMATCH,
	ID_MCA_EXACTFILENAME,
	ID_MCA_EACHFILEINSEPARATEARCHIVE,
	ID_MCA_SEPARATOR5,
	ID_MCA_ADDCONFIRM,
	ID_MCA_CANCEL
};


void SetFormatTitle(FarDialog *D)
{
	string strFormat;
	string strTitle;

	if ( D->GetCheck(ID_MCA_TEMPLATE) == BSTATE_CHECKED )
		strFormat = D->GetConstTextPtr(ID_MCA_TEMPLATELIST);
	else
		strFormat = D->GetConstTextPtr(ID_MCA_FORMATLIST);

	strTitle.Format(_M(MAddToTitle), strFormat.GetString());

	D->SetTextPtr(ID_MCA_TITLE, strTitle);
}


void SetTemplate(FarDialog *D, ArchiveTemplate *ptpl = NULL)
{
	ArchiveManagerConfig* pCfg = pManager->GetConfig();

	Array<ArchiveTemplate*> templates;
	pCfg->GetTemplates(templates);

	static bool bInit = false;

	if ( !bInit )
	{
		bInit = true;

		FarListPos arcPos;
		FarListInfo li;

		int iCount = templates.count();

		D->ListInfo(ID_MCA_TEMPLATELIST, &li);

		if ( li.ItemsNumber != iCount )
		{
			D->ListDelete(ID_MCA_TEMPLATELIST, NULL);

			if ( iCount )
			{
				int pos = ptpl?-1:li.SelectPos;

				for (int i = 0; i < iCount; i++)
				{
					D->ListAddStr(ID_MCA_TEMPLATELIST, templates[i]->GetName());

					if ( templates[i] == ptpl )
						pos = i;
				}

				arcPos.TopPos = -1;
				arcPos.SelectPos = pos;

				D->ListSetCurrentPos(ID_MCA_TEMPLATELIST, &arcPos);
			}
			else
				D->SetTextPtr (ID_MCA_TEMPLATELIST, _T(""));
		}

		D->RedrawDialog();

		bInit = false;
	}

	bool bEnable = (templates.count() > 0);
	int nFocus = D->GetFocus ();

	D->Enable(ID_MCA_TEMPLATELIST, bEnable);
	D->Enable(ID_MCA_REMOVE, bEnable);
	D->Enable(ID_MCA_EDIT, bEnable);
}


void EnableControls(FarDialog* D)
{
	bool bChecked = D->GetCheck(ID_MCA_TEMPLATE);

	if ( bChecked )
		SetTemplate(D);

	D->Enable(ID_MCA_FORMAT, !bChecked);
	D->Enable(ID_MCA_FORMATLIST, !bChecked);
	D->Enable(ID_MCA_CONFIGURE, !bChecked);
	D->Enable(ID_MCA_PARAMS, !bChecked);
	D->Enable(ID_MCA_PARAMSEDIT, !bChecked);
}

LONG_PTR __stdcall hndModifyCreateArchive(
		FarDialog *D,
		int nMsg,
		int nParam1,
		LONG_PTR nParam2
		)
{
	ArchiveManagerConfig* pCfg = pManager->GetConfig();

	Array<ArchiveTemplate*> templates;
	pCfg->GetTemplates(templates);

	if ( nMsg == DN_INITDIALOG )
	{
		Array<ArchiveFormat*> formats;

		pManager->GetFormats(formats);

		for (unsigned int i = 0; i < formats.count(); i++)
		{
			ArchiveFormat* pFormat = formats[i];

			string strCommand;
			bool bHasAddCommand = pManager->GetCommand(pFormat, COMMAND_ADD, strCommand);

			if ( pFormat->QueryCapability(AFF_SUPPORT_INTERNAL_CREATE) || bHasAddCommand )
			{
				int index = D->ListAddStr(ID_MCA_FORMATLIST, pFormat->GetName());
				D->ListSetDataEx(ID_MCA_FORMATLIST, index, (void*)pFormat, sizeof(void*));
			}
		}

		EnableControls(D);
	}

	if ( nMsg == DN_BTNCLICK )
	{
		if ( nParam1 == ID_MCA_TEMPLATE )
			EnableControls(D);

		if ( nParam1 == ID_MCA_ADD )
		{
			ArchiveTemplate *ptpl = new ArchiveTemplate;

			if ( dlgAddEditTemplate(ptpl, true) )
			{
				pCfg->AddTemplate(ptpl);
				pCfg->Save(SAVE_TEMPLATES);

				SetTemplate(D, ptpl);
			}
			else
				delete ptpl;

			return TRUE;
		}

		if ( (nParam1 == ID_MCA_REMOVE) && templates.count() )
		{
			int iPos = D->ListGetCurrentPos(ID_MCA_TEMPLATELIST, NULL);

			pCfg->RemoveTemplate(templates[iPos]);
			pCfg->Save(SAVE_TEMPLATES);

			SetTemplate(D);

			return TRUE;
		}

		if ( (nParam1 == ID_MCA_EDIT) && templates.count() )
		{
			int iPos = D->ListGetCurrentPos(ID_MCA_TEMPLATELIST, NULL);

			if ( dlgAddEditTemplate(templates[iPos], false) )
			{
				pCfg->Save(SAVE_TEMPLATES);
				SetTemplate(D);
			}

			return TRUE;
		}

		if ( nParam1 == ID_MCA_CONFIGURE )
		{
			ArchiveFormat* pFormat = nullptr;
			const TCHAR* lpInitialConfig = nullptr;

			int pos = D->ListGetCurrentPos(ID_MCA_FORMATLIST, NULL);

			if ( pos != -1 )
				 pFormat = (ArchiveFormat*)D->ListGetData(ID_MCA_FORMATLIST, pos);

			string strConfig;

			if ( pFormat != nullptr )
				pFormat->Configure(lpInitialConfig, strConfig);

			ArchiveTemplate *ptpl = (ArchiveTemplate*)D->GetDlgData();

			ptpl->SetConfig(strConfig);

			return TRUE;
		}
	}

	if ( nMsg == DN_DRAWDIALOG )
		SetFormatTitle(D);

	if ( nMsg == DN_EDITCHANGE )
	{
		if ((nParam1 == ID_MCA_PASSWORDEDIT ) || ( nParam1 == ID_MCA_CONFIRMEDIT ) )
		{
			string strPassword1, strPassword2;

			strPassword1 = D->GetConstTextPtr(ID_MCA_PASSWORDEDIT);
			strPassword2 = D->GetConstTextPtr(ID_MCA_CONFIRMEDIT);

			if ( strPassword1.IsEmpty() && strPassword2.IsEmpty() )
				D->SetTextPtr(ID_MCA_PASSWORDMATCH, _T(""));
			else
			{
				if ( strPassword1 == strPassword2 )
					D->SetTextPtr(ID_MCA_PASSWORDMATCH, _M(MCreateArchivePasswordsMatch));
				else
					D->SetTextPtr(ID_MCA_PASSWORDMATCH, _M(MCreateArchivePasswordsNotMatch));
			}
		}

		if (nParam1 == ID_MCA_TEMPLATELIST )
			SetTemplate(D);
	}

	if ( nMsg == DN_CLOSE )
	{
		if ( nParam1 == ID_MCA_ADDCONFIRM )
		{
			string strPassword1, strPassword2;

			strPassword1 = D->GetConstTextPtr (ID_MCA_PASSWORDEDIT);
			strPassword2 = D->GetConstTextPtr (ID_MCA_CONFIRMEDIT);

			if ( strPassword1 != strPassword2 )
			{
				msgError(_T("passwords dont'match"));
				return FALSE;
			}

			ArchiveTemplate *ptpl = (ArchiveTemplate*)D->GetDlgData ();

			if ( D->GetCheck(ID_MCA_TEMPLATE) )
			{
				int pos = D->ListGetCurrentPos (ID_MCA_TEMPLATELIST, NULL);

				if ( pos != -1 )
				{
					ArchiveTemplate *pSrc = templates[pos];

					ptpl->SetName(pSrc->GetName());
					ptpl->SetParams(pSrc->GetParams());
					ptpl->SetConfig(pSrc->GetConfig());
					ptpl->SetFormat(pSrc->GetFormat());
				}
			}
			else
			{
				int pos = D->ListGetCurrentPos(ID_MCA_FORMATLIST, NULL);

				if ( pos != -1 )
				{
					ptpl->SetName(NULL);
					ptpl->SetParams(D->GetConstTextPtr(ID_MCA_PARAMSEDIT));

					ArchiveFormat* pFormat = (ArchiveFormat*)D->ListGetData(ID_MCA_FORMATLIST, pos);

					ptpl->SetFormat(pFormat);
				}
			}

			/*if ( !ptpl->IsValid() )
			{
				msgError(_T("may not use an invalid template!"));
				return FALSE;
			}*/

			return TRUE;
		}
	}

	return D->DefDlgProc (nMsg, nParam1, nParam2);
}

struct CreateArchiveParams {

	bool bUseTemplate; //HMM

	string strFileName;
	string strPassword;
	string strAdditionalCommandLine;
	string strConfig;

	ArchiveFormat* pFormat;

	bool bExactName;
	bool bSeparateArchives;
};



bool dlgModifyCreateArchive(
			ArchivePanel *pPanel,
			CreateArchiveParams* pParams
			)
{
	FarPanelInfo info;

	string strArchiveName;

	int nSelectedCount = info.GetSelectedItemsCount();

	if ( !nSelectedCount || (nSelectedCount > 1) )
		strArchiveName = FSF.PointToName(info.GetCurrentDirectory());
	else
	{
		PluginPanelItem item;
		
		info.GetSelectedItem(0, &item);

#ifdef UNICODE
		strArchiveName = FSF.PointToName(item.FindData.lpwszFileName);
#else
		strArchiveName = FSF.PointToName(item.FindData.cFileName);
#endif

		if ( !(item.FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			CutTo(strArchiveName, _T('.'), true);

		info.FreePanelItem(&item);
	}

	FarDialog D(-1, -1, 75, 21);

	D.DoubleBox(3, 1, 71, 19, NULL); //0

	D.Text(5, 2, _M(MCreateArchiveAddToArchive));//1
	D.Edit(5, 3, 65, strArchiveName, AUTO_LENGTH, _T("")); //2

	D.Separator (4); //3

	unsigned int uCaptionLength = StrLength(_T("Template"))+4;
	unsigned int uComboStart = 5+uCaptionLength+1;
	unsigned int uComboWidth = 71-uCaptionLength-7-13;

	D.CheckBox(5, 5, pParams->bUseTemplate, _T("Template"));
	D.ComboBox(uComboStart, 5, uComboWidth, nullptr);
	D.SetFlags(DIF_DROPDOWNLIST);

	D.Button(uComboStart+uComboWidth+2, 5, _T("[+]"));
	D.Button(uComboStart+uComboWidth+2+4, 5, _T("[-]"));
	D.Button(uComboStart+uComboWidth+2+8, 5, _T("[*]"));

	D.Separator(6);

	D.Text(5, 7, _T("Format"));
	D.ComboBox(5, 8, 30, nullptr);
	D.SetFlags(DIF_DROPDOWNLIST);

	D.Button(36, 8, _T("Config"));

	D.Text(5, 9, _T("Additional params"));
	D.Edit(5, 10, 50);

	D.Separator(11);

	D.Text(5, 12, _M(MCreateArchivePassword)); //16
	D.PswEdit(5, 13, 32); //17

	D.Text(38, 12, _M(MCreateArchiveConfirmPassword)); //18
	D.PswEdit(38, 13, 32); //19

	D.Separator(14);
	D.Text(48, 14); //match

	D.CheckBox(5, 15, false, _M(MCreateArchiveExactFilename)); 
	D.CheckBox(5, 16, false, _M(MCreateArchiveEachFileInSeparateArchive)); 

	D.Separator(17);

	D.Button(-1, 18, _T("Ok"));
	D.DefaultButton();

	D.Button(-1, 18, _T("Cancel"));

	ArchiveTemplate tpl;

	if ( D.Run(hndModifyCreateArchive, &tpl) == ID_MCA_ADDCONFIRM )
	{
		//if ( tpl.IsValid() )
		{
			pParams->strFileName = D.GetResultData(ID_MCA_NAMEEDIT);
			pParams->strPassword = D.GetResultData(ID_MCA_PASSWORDEDIT);

			pParams->strAdditionalCommandLine = tpl.GetParams();
			pParams->strConfig = tpl.GetConfig();
			pParams->pFormat = tpl.GetFormat();

			pParams->bExactName = D.GetResultCheck(ID_MCA_EXACTFILENAME);
			pParams->bSeparateArchives = D.GetResultCheck(ID_MCA_EACHFILEINSEPARATEARCHIVE);

			return true;
		}
	}

	return false;
}

