void SetFormatTitle (FarDialog *D)
{
	string strFormat;
	string strTitle;

	if ( D->GetCheck (5) )
		strFormat = D->GetConstTextPtr (7);
	else
		strFormat = D->GetConstTextPtr (12);

	strTitle.Format(_M(MAddToTitle), strFormat.GetString());

	D->SetTextPtr (0, strTitle);
}


void SetTemplate (FarDialog *D, ArchiveTemplate *ptpl = NULL)
{
	Array<ArchiveTemplate*>& templates = pManager->GetTemplates();

	static bool bInit = false;

	if (!bInit)
	{
		bInit = true;

		FarListPos arcPos;
		FarListInfo li;

		int iCount = templates.count();

		D->ListInfo(7, &li);

		if ( li.ItemsNumber != iCount )
		{
			D->ListDelete (7, NULL);

			if ( iCount )
			{
				int pos = ptpl?-1:li.SelectPos;

				for (int i = 0; i < iCount; i++)
				{
					D->ListAddStr (7, templates[i]->GetName());

					if ( templates[i] == ptpl )
						pos = i;
				}

				arcPos.TopPos = -1;
				arcPos.SelectPos = pos;
				D->ListSetCurrentPos(7, &arcPos);
			}
			else
				D->SetTextPtr (7, _T(""));
		}

		D->RedrawDialog();

		bInit = false;
	}

	bool bEnable = (templates.count() > 0);
	int nFocus = D->GetFocus ();

	D->Enable (5, bEnable);
	D->Enable (7, bEnable);
	D->Enable (9, bEnable);
	D->Enable (10, bEnable);

	if ( !bEnable && ((nFocus == 9) || (nFocus == 10)) )
		D->SetFocus (8);

	if ( !bEnable )
		D->SetCheck (6, BSTATE_CHECKED);
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
			pFormat->GetDefaultCommand(COMMAND_ADD, strCommand);

			if ( pFormat->QueryCapability(AFF_SUPPORT_INTERNAL_CREATE) || !strCommand.IsEmpty() )
			{
				int index = D->ListAddStr (12, pFormat->GetName());
				D->ListSetDataEx (12, index, (void*)pFormat, sizeof(void*));
			}
		}

		for (unsigned int i = 0; i < templates.count(); i++)
			D->ListAddStr (7, templates[i]->GetName());

		SetTemplate(D);
	}

	if ( nMsg == DN_BTNCLICK )
	{
		if ( (nParam1 == 5) && (nParam2 != 0) )
			SetTemplate (D);

		if ( nParam1 == 8 )
		{
			ArchiveTemplate *ptpl = new ArchiveTemplate;

			if ( dlgAddEditTemplate (ptpl, true) )
			{
				templates.add(ptpl);

				SetTemplate(D, ptpl);

				pManager->SaveTemplates(_T("templates.ini"));
			}
			else
				delete ptpl;
		}

		if ( (nParam1 == 9) && templates.count() )
		{
			int iPos = D->ListGetCurrentPos (7, NULL);

			templates.remove(iPos);

			SetTemplate (D);
			pManager->SaveTemplates(_T("templates.ini"));
		}

		if ( (nParam1 == 10) && templates.count() )
		{
			int iPos = D->ListGetCurrentPos (7, NULL);

			if ( dlgAddEditTemplate(templates[iPos], false) )
			{
				SetTemplate (D);
				pManager->SaveTemplates(_T("templates.ini"));
			}
		}
	}

	if ( nMsg == DN_GOTFOCUS )
	{
		if ( nParam1 == 7 )
		{
			D->SetCheck (5, BSTATE_CHECKED);
			return 0;
		}

		if ( (nParam1 == 12) || (nParam1 == 14) )
		{
			D->SetCheck (6, BSTATE_CHECKED);
			return 0;
		}
	}

	if ( nMsg == DN_DRAWDIALOG )
		SetFormatTitle (D);


	if ( nMsg == DN_EDITCHANGE )
	{
		if ((nParam1 == 17 ) || ( nParam1 == 19 ) )
		{
			string strPassword1, strPassword2;

			strPassword1 = D->GetConstTextPtr (17);
			strPassword2 = D->GetConstTextPtr (19);

			if ( strPassword1.IsEmpty() && strPassword2.IsEmpty() )
				D->SetTextPtr (21, _T(""));
			else
			{
				if ( strPassword1 == strPassword2 )
					D->SetTextPtr (21, _M(MCreateArchivePasswordsMatch));
				else
					D->SetTextPtr (21, _M(MCreateArchivePasswordsNotMatch));
			}
		}

		if (nParam1 == 7 )
			SetTemplate (D);
	}

	if ( nMsg == DN_CLOSE )
	{
		if ( nParam1 == 27 )
		{
  			int pos = D->ListGetCurrentPos (12, NULL);

  			if ( pos != -1 )
  			{
//				ArchiveFormat* pFormat = (ArchiveFormat*)D->ListGetData (12, pos);//Templates[pos];

//				pFormat->GetModule()->Configure(uid);
			}

			return false;
		}

		if ( nParam1 == 25 )
		{
	  		string strPassword1, strPassword2;

  			strPassword1 = D->GetConstTextPtr (17);
	  		strPassword2 = D->GetConstTextPtr (19);

	  		if ( strPassword1 != strPassword2 )
	  		{
	  			msgError(_T("passwords dont'match"));
	  			return FALSE;
			}

  			ArchiveTemplate *ptpl = (ArchiveTemplate*)D->GetDlgData ();

  			if ( D->GetCheck (5) == BSTATE_CHECKED )
  			{
	  			int pos = D->ListGetCurrentPos (7, NULL);

	  			if ( pos != -1 )
	  			{
					ArchiveTemplate *pSrc = templates[pos];

					ptpl->SetData(
							pManager, 
							pSrc->GetName(), 
							pSrc->GetParams(), 
							pSrc->GetModuleUID(),
							pSrc->GetPluginUID(),
							pSrc->GetFormatUID()
							);
				}
  			}
  			else
  			{
				int pos = D->ListGetCurrentPos (12, NULL);

				if ( pos != -1 )
				{
	  				ptpl->SetName(NULL);
  					ptpl->SetParams(D->GetConstTextPtr(14));

  					ArchiveFormat* pFormat = (ArchiveFormat*)D->ListGetData (12, pos);

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
		strArchiveName = FSF.PointToName (info.GetCurrentDirectory());
	else
	{
		PluginPanelItem item;
		
		info.GetSelectedItem(0, &item);

#ifdef UNICODE
		strArchiveName = FSF.PointToName (item.FindData.lpwszFileName);
#else
		strArchiveName = FSF.PointToName (item.FindData.cFileName);
#endif

		if ( !(item.FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) )
			CutTo (strArchiveName, _T('.'), true);

		info.FreePanelItem(&item);
	}

	FarDialog D (-1, -1, 75, 20);

	D.DoubleBox (3, 1, 71, 18, NULL); //0

	D.Text (5, 2, _M(MCreateArchiveAddToArchive));//1
	D.Edit (5, 3, 65, strArchiveName, AUTO_LENGTH, _T("sdfas")); //2

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

	D.Text (9, 9, _M(MCreateArchiveArchiver));//11
	D.ComboBox (19, 9, 15, NULL/*, 0, _T("123")*/);//12
///	D->ListBox (50, 5, 65, 10, NULL);
	D.SetFlags (DIF_DROPDOWNLIST);

	D.Text (37, 9, _M(MCreateArchiveAdditionalParams));//13
	D.Edit (52, 9, 18, NULL, AUTO_LENGTH, _T("adsaf"));//14

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
			pParams->strPassword = D.GetResultData(7);

			pParams->strAdditionalCommandLine = tpl.GetParams();
			pParams->pFormat = tpl.GetFormat();

			pParams->bExactName = D.GetResultCheck(22);
			pParams->bSeparateArchives = D.GetResultCheck(23);

			return true;
		}

		//strAdditionalCommandLine = 

	/*	strArchiveName = D.GetResultData(2);

		string strPassword = D.GetResultData(17);
		string strAdditionalCommandLine = tpl.strParams;

		ArchiveModule *pModule = pManager->GetModuleFromUID (tpl.uid);
		const ArchiveFormatInfo *ainfo = pModule->GetArchiveFormatInfo(tpl.uid);

		bool bSeparately = D.GetResultCheck(23) && (nSelectedCount > 1);
		int	nCount = (bSeparately)?nSelectedCount:1;

		for (int el = 0; el < nCount; el++)
		{
			ArchiveItemArray items;

			if ( bSeparately )
			{
				PluginPanelItem *pitem = info.GetSelectedItem(el);

				ArchiveItem *item = items.add();
				FindDataToArchiveItem(&pitem->FindData, item);

				info.FreePanelItem(pitem);

				strArchiveName = FSF.PointToName(item->lpFileName);
				
				if ( !OptionIsOn (item->dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
					CutTo (strArchiveName, _T('.'), true);
			}
			else
			{
				int nCount = info.GetSelectedItemsCount();

				for (int i = 0; i < nCount; i++)
				{
					ArchiveItem *dest = items.add();
					PluginPanelItem *src = info.GetSelectedItem(i);

					FindDataToArchiveItem(&src->FindData, dest);
				}
			}

			strArchiveName += _T(".");

			if ( !D.GetResultCheck(22) )
				strArchiveName += ainfo->lpDefaultExtention;


			string strFullArchiveName = info.GetCurrentDirectory();
			AddEndSlash(strFullArchiveName);

			strFullArchiveName += strArchiveName;

			bool bResult = false;

			ExpandFilesList(items);

			Archive* pArchive = pModule->CreateArchive(pFormat, strFullArchiveName);

			if ( pArchive )
			{
				pArchive->SetPassword(strPassword);
				bResult = pArchive->AddFiles(items, info.GetCurrentDirectory(), pPanel->GetPathInArchive());

				pModule->FinalizeArchive(pArchive);
			}
		}*/
	}

	return false;
}

