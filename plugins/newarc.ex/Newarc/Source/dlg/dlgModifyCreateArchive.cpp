enum enumModifyCreateArchive {
	ID_MCA_TITLE,
	ID_MCA_NAME,
	ID_MCA_NAMEEDIT,
	ID_MCA_SEPARATOR1,
	ID_MCA_ARCHIVER,
	ID_MCA_TEMPLATE,
	ID_MCA_DIRECTSETTINGS,
	ID_MCA_TEMPLATELIST,
	ID_MCA_ADD,
	ID_MCA_REMOVE,
	ID_MCA_EDIT,
	ID_MCA_FORMAT,
	ID_MCA_FORMATLIST,
	ID_MCA_PARAMS,
	ID_MCA_PARAMSEDIT,
	ID_MCA_SEPARATOR2,
	ID_MCA_PASSWORD,
	ID_MCA_PASSWORDEDIT,
	ID_MCA_CONFIRM,
	ID_MCA_CONFIRMEDIT,
	ID_MCA_SEPARATOR3,
	ID_MCA_PASSWORDMATCH,
	ID_MCA_EXACTFILENAME,
	ID_MCA_EACHFILEINSEPARATEARCHIVE,
	ID_MCA_SEPARATOR4,
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
	Array<ArchiveTemplate*>& templates = pManager->GetTemplates();

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

	D->Enable(ID_MCA_TEMPLATE, bEnable);
	D->Enable(ID_MCA_TEMPLATELIST, bEnable);
	D->Enable(ID_MCA_REMOVE, bEnable);
	D->Enable(ID_MCA_EDIT, bEnable);

	if ( !bEnable && ((nFocus == ID_MCA_REMOVE) || (nFocus == ID_MCA_EDIT)) )
		D->SetFocus(ID_MCA_ADD);

	if ( !bEnable )
		D->SetCheck(ID_MCA_DIRECTSETTINGS, BSTATE_CHECKED);
}



LONG_PTR __stdcall hndModifyCreateArchive (
		FarDialog *D,
		int nMsg,
		int nParam1,
		LONG_PTR nParam2
		)
{
	Array<ArchiveTemplate*>& templates = pManager->GetTemplates();

	if ( nMsg == DN_INITDIALOG )
	{
		Array<ArchiveFormat*> formats;

		pManager->GetFormats(formats);

		for (unsigned int i = 0; i < formats.count(); i++)
		{
			ArchiveFormat* pFormat = formats[i];

			string strCommand;
			bool bEnabled;

			pFormat->GetDefaultCommand(COMMAND_ADD, strCommand, bEnabled);

			if ( pFormat->QueryCapability(AFF_SUPPORT_INTERNAL_CREATE) || !strCommand.IsEmpty() )
			{
				int index = D->ListAddStr(ID_MCA_FORMATLIST, pFormat->GetName());
				D->ListSetDataEx(ID_MCA_FORMATLIST, index, (void*)pFormat, sizeof(void*));
			}
		}

		for (unsigned int i = 0; i < templates.count(); i++)
			D->ListAddStr(ID_MCA_TEMPLATELIST, templates[i]->GetName());

		SetTemplate(D);
	}

	if ( nMsg == DN_BTNCLICK )
	{
		if ( (nParam1 == ID_MCA_TEMPLATE) && (nParam2 != 0) )
			SetTemplate(D);

		if ( nParam1 == ID_MCA_ADD )
		{
			ArchiveTemplate *ptpl = new ArchiveTemplate;

			if ( dlgAddEditTemplate(ptpl, true) )
			{
				templates.add(ptpl);

				SetTemplate(D, ptpl);

				pManager->SaveTemplates(_T("templates.ini"));
			}
			else
				delete ptpl;
		}

		if ( (nParam1 == ID_MCA_REMOVE) && templates.count() )
		{
			int iPos = D->ListGetCurrentPos(ID_MCA_TEMPLATELIST, NULL);

			templates.remove(iPos);

			SetTemplate(D);
			pManager->SaveTemplates(_T("templates.ini"));
		}

		if ( (nParam1 == ID_MCA_EDIT) && templates.count() )
		{
			int iPos = D->ListGetCurrentPos(ID_MCA_TEMPLATELIST, NULL);

			if ( dlgAddEditTemplate(templates[iPos], false) )
			{
				SetTemplate(D);
				pManager->SaveTemplates(_T("templates.ini"));
			}
		}
	}

	if ( nMsg == DN_GOTFOCUS )
	{
		if ( nParam1 == ID_MCA_TEMPLATELIST )
		{
			D->SetCheck(ID_MCA_TEMPLATE, BSTATE_CHECKED);
			return 0;
		}

		if ( (nParam1 == ID_MCA_FORMATLIST) || (nParam1 == ID_MCA_PARAMSEDIT) )
		{
			D->SetCheck (ID_MCA_DIRECTSETTINGS, BSTATE_CHECKED);
			return 0;
		}
	}

	if ( nMsg == DN_DRAWDIALOG )
		SetFormatTitle (D);

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
			SetTemplate (D);
	}

	if ( nMsg == DN_CLOSE )
	{
		if ( nParam1 == ID_MCA_CANCEL+1 ) //BUGBUG, да вы прикалываетесь
		{
			ArchiveModule* pModule = nullptr;
			GUID uidPlugin, uidFormat;
			const TCHAR* lpInitialConfig = nullptr;

			if ( D->GetCheck(ID_MCA_TEMPLATE) == BSTATE_CHECKED )
			{
				int pos = D->ListGetCurrentPos(ID_MCA_TEMPLATELIST, NULL);

				if ( pos != -1 )
				{
					ArchiveTemplate *pTemplate = templates[pos];

					pModule = pManager->GetModule(pTemplate->GetModuleUID());
					
					uidPlugin = pTemplate->GetPluginUID();
					uidFormat = pTemplate->GetFormatUID();

					lpInitialConfig = pTemplate->GetConfig();
				}
			}
			else
			{
				int pos = D->ListGetCurrentPos(ID_MCA_FORMATLIST, NULL);

				if ( pos != -1 )
				{
					ArchiveFormat* pFormat = (ArchiveFormat*)D->ListGetData(ID_MCA_FORMATLIST, pos);

					pModule = pFormat->GetModule();

					uidFormat = pFormat->GetUID();
					uidPlugin = pFormat->GetPlugin()->GetUID();
				}
			}

			string strConfig;

			if ( pModule != nullptr )
				pModule->ConfigureFormat(uidPlugin, uidFormat, lpInitialConfig, strConfig);


			ArchiveTemplate *ptpl = (ArchiveTemplate*)D->GetDlgData ();

			ptpl->SetConfig(strConfig);

			return FALSE;
		}

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

			if ( D->GetCheck(ID_MCA_TEMPLATE) == BSTATE_CHECKED )
			{
				int pos = D->ListGetCurrentPos (ID_MCA_TEMPLATELIST, NULL);

				if ( pos != -1 )
				{
					ArchiveTemplate *pSrc = templates[pos];

					ptpl->SetData(
							pManager, 
							pSrc->GetName(), 
							pSrc->GetParams(),
							pSrc->GetConfig(),
							pSrc->GetModuleUID(),
							pSrc->GetPluginUID(),
							pSrc->GetFormatUID()
							);
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

			if ( !ptpl->IsValid() )
			{
				msgError(_T("may not use an invalid template!"));
				return FALSE;
			}

			return TRUE;
		}
	}

	return D->DefDlgProc (nMsg, nParam1, nParam2);
}

struct CreateArchiveParams {
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

	FarDialog D (-1, -1, 75, 20);

	D.DoubleBox (3, 1, 71, 18, NULL); //0

	D.Text(5, 2, _M(MCreateArchiveAddToArchive));//1
	D.Edit(5, 3, 65, strArchiveName, AUTO_LENGTH, _T("")); //2



	D.Separator (4); //3

	D.Text (5, 5, _M(MCreateArchiveArchiverSettings)); //4

	D.RadioButton (6, 6, true, _M(MCreateArchiveTemplate)); //5
	D.RadioButton (6, 8, false, _M(MCreateArchiveDirectSettings)); //6

	D.ComboBox (9, 7, 48, NULL, 0); //7
	D.SetFlags (DIF_DROPDOWNLIST);

	D.Button (59, 7, _T("[+]")); //8
	D.SetFlags(DIF_BTNNOCLOSE);

	D.Button (63, 7, _T("[-]")); //9
	D.SetFlags(DIF_BTNNOCLOSE);

	D.Button (67, 7, _T("[*]")); //10
	D.SetFlags(DIF_BTNNOCLOSE);

	D.Text (8, 9, _M(MCreateArchiveArchiver));//11
	D.ComboBox (18, 9, 18, NULL/*, 0, _T("123")*/);//12
//	D.Text (9, 9, _M(MCreateArchiveArchiver));//11
//	D.ComboBox (19, 9, 15, NULL/*, 0, _T("123")*/);//12
///	D->ListBox (50, 5, 65, 10, NULL);
	D.SetFlags (DIF_DROPDOWNLIST);

	D.Text (38, 9, _M(MCreateArchiveAdditionalParams));//13
	D.Edit (53, 9, 17, NULL, AUTO_LENGTH, _T("adsaf"));//14

//	D.Text (37, 9, _M(MCreateArchiveAdditionalParams));//13
//	D.Edit (52, 9, 18, NULL, AUTO_LENGTH, _T("adsaf"));//14

	D.Separator (10); //15

	D.Text (5, 11, _M(MCreateArchivePassword)); //16
	D.PswEdit (5, 12, 32); //17

	D.Text (38, 11, _M(MCreateArchiveConfirmPassword)); //18
	D.PswEdit (38, 12, 32); //19

	D.Separator (13); //20
	D.Text (48, 13); //21

	D.CheckBox (5, 14, false, _M(MCreateArchiveExactFilename)); //22
	D.CheckBox (5, 15, false, _M(MCreateArchiveEachFileInSeparateArchive)); //23

	D.Separator (16); //24

	D.Button (-1, 17, _M(MCreateArchiveAdd)); //25
	D.DefaultButton ();

	D.Button (-1, 17, _M(MCreateArchiveCancel)); //26

	D.Button (-1, 17, _T("[?]")); //27

	ArchiveTemplate tpl;

	if ( D.Run(
			hndModifyCreateArchive,
			&tpl
			) == 25 )
	{
		if ( tpl.IsValid() )
		{
			pParams->strFileName = D.GetResultData(2);
			pParams->strPassword = D.GetResultData(17);

			pParams->strAdditionalCommandLine = tpl.GetParams();
			pParams->strConfig = tpl.GetConfig();
			pParams->pFormat = tpl.GetFormat();

			pParams->bExactName = D.GetResultCheck(22);
			pParams->bSeparateArchives = D.GetResultCheck(23);

			return true;
		}

	}

	return false;
}

