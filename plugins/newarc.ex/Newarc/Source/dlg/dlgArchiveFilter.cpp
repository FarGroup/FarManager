void UpdateFormats(FarDialog* D, ArchivePlugin* pPlugin, ArchiveFilterEntry* pFE)
{
	int index = 0;

	D->ListDelete(14, NULL);
	
	index = D->ListAddStr(14, _T("All"));
	D->ListSetDataEx(14, index, (void*)0, sizeof(void*));

	FarListPos pos;

	pos.SelectPos = 0; //all
	pos.TopPos = -1;

	if ( pPlugin )
	{
		if ( pPlugin->QueryCapability(APF_SUPPORT_SINGLE_FORMAT_QUERY) )
		{
			Array<ArchiveFormat*> formats;
    		pPlugin->GetFormats(formats);  //BUGBUG
    
			for (unsigned int i = 0; i < formats.count(); i++)
			{
				ArchiveFormat* pFormat = formats[i];

				index = D->ListAddStr(14, pFormat->GetName());
				D->ListSetDataEx(14, index, (void*)pFormat, sizeof(void*));

				if ( pFE && !pFE->bAllFormats && (pFE->uidFormat == pFormat->GetUID()) )
					pos.SelectPos = index;
			}
		}
	}

	D->ListSetCurrentPos(14, &pos);
}


void UpdatePlugins(FarDialog* D, ArchiveModule* pModule, ArchiveFilterEntry* pFE)
{
	int index = 0;

	D->ListDelete(12, NULL);
	
	index = D->ListAddStr(12, _T("All"));
	D->ListSetDataEx(12, index, (void*)0, sizeof(void*));

	FarListPos pos;

	pos.SelectPos = 0; //all
	pos.TopPos = -1;

	if ( pModule )
	{
	   	Array<ArchivePlugin*>& plugins = pModule->GetPlugins();

		if ( pModule->QueryCapability(AMF_SUPPORT_SINGLE_PLUGIN_QUERY) )
		{
			for (unsigned int i = 0; i < plugins.count(); i++)
			{
				ArchivePlugin* pPlugin = plugins[i];

				index = D->ListAddStr(12, FSF.PointToName(pPlugin->GetModuleName()));
				D->ListSetDataEx(12, index, (void*)pPlugin, sizeof(void*));

				if ( pFE && !pFE->bAllPlugins && (pFE->uidPlugin == pPlugin->GetUID()) )
					pos.SelectPos = index;
			}
		}
	}


	D->ListSetCurrentPos(12, &pos);
	ArchivePlugin* pPlugin = (ArchivePlugin*)D->ListGetData(12, pos.SelectPos);

	UpdateFormats(D, pPlugin, pFE);
}


LONG_PTR __stdcall hndFilterOneFormat(FarDialog* D, int nMsg, int nParam1, LONG_PTR nParam2)
{
	ArchiveFilterEntry* pFE = (ArchiveFilterEntry*)D->GetDlgData();

	if ( (nMsg == DN_LISTCHANGE) && (nParam1 == 2) )
	{
		if ( nParam2 == 1 ) //do block
		{	
			FarListPos pos;

			D->ListGetCurrentPos(10, &pos);


			FarListInsert ins;

			ins.Index = 0;
			ins.Item.Flags = 0;
#ifdef UNICODE
			ins.Item.Text = _T("All");
#else
			strcpy(ins.Item.Text, _T("All"));
#endif

			int index = D->ListInsert(10, &ins);
			D->ListSetDataEx(10, index, (void*)0, sizeof(void*));

			pos.SelectPos++;
			D->ListSetCurrentPos(10, &pos);

		}
		else
		{
			FarListDelete del;

			del.StartIndex = 0;
			del.Count = 1;

			D->ListDelete(10, &del);
////////////
			FarListPos pos;
		D->ListGetCurrentPos(10, &pos);

		if ( pos.SelectPos != -1 )
		{
			ArchiveModule* pModule = (ArchiveModule*)D->ListGetData(10, pos.SelectPos);

			UpdatePlugins(D, pModule, NULL);
		}

		return TRUE;
//////////////


		}

		return TRUE;
	}
	else

	if ( (nMsg == DN_LISTCHANGE) && (nParam1 == 10) )
	{
		FarListPos pos;
		D->ListGetCurrentPos(10, &pos);

		if ( pos.SelectPos != -1 )
		{
			ArchiveModule* pModule = (ArchiveModule*)D->ListGetData(10, pos.SelectPos);

			UpdatePlugins(D, pModule, NULL);
		}

		return TRUE;
	}
	else
	              
	if ( (nMsg == DN_LISTCHANGE) && (nParam1 == 12) )
	{
		FarListPos pos;
		D->ListGetCurrentPos(12, &pos);

		if ( pos.SelectPos != -1 )
		{
			ArchivePlugin* pPlugin = (ArchivePlugin*)D->ListGetData(12, pos.SelectPos);

			UpdateFormats(D, pPlugin, NULL);
		}

		return TRUE;
	}
	else

	if ( nMsg == DN_INITDIALOG )
	{
		FarListPos pos;

		D->ListAddStr(2, _T("Process"));
		D->ListAddStr(2, _T("Block"));

		pos.SelectPos = pFE->bExcludeFilter?1:0;

		D->ListSetCurrentPos(2, &pos);
		
		pos.SelectPos = 0;
		pos.TopPos = -1;

		Array<ArchiveModule*>& modules = pManager->GetModules();

		for (unsigned int i = 0; i < modules.count(); i++)
		{
			const ArchiveModule* pModule = modules[i];

			int index = D->ListAddStr(10, FSF.PointToName(pModule->GetModuleName()));
			D->ListSetDataEx(10, index, (void*)pModule, sizeof(void*));

			if ( pFE && !pFE->bAllModules && (pFE->uidModule == pModule->GetUID()) )
				pos.SelectPos = index;
		}

		D->ListSetCurrentPos(10, &pos); 

		ArchiveModule* pModule = (ArchiveModule*)D->ListGetData(10, pos.SelectPos);
		UpdatePlugins(D, pModule, pFE); 

		//return TRUE; hmm
	}
	else

	if ( (nMsg == DN_CLOSE) && (nParam1 == D->FirstButton()) )
	{
		string strName = D->GetConstTextPtr(5);

		if ( strName.IsEmpty() )
		{
			msgError(_T("name empty"));
			return FALSE;
		}

		string strMask = D->GetConstTextPtr(7);

		if ( strMask.IsEmpty() )
		{
			msgError(_T("mask empty"));
			return FALSE;
		}

		

		FarListPos pos;

		D->ListGetCurrentPos(2, &pos);

		pFE->bExcludeFilter = (pos.SelectPos == 1);

		D->ListGetCurrentPos(10, &pos);

		ArchiveModule* pModule = (ArchiveModule*)D->ListGetData(10, pos.SelectPos);

		pFE->strName = strName;
		pFE->strMask = strMask;

		pFE->bAllFormats = true;
		pFE->bAllModules = true;
		pFE->bAllPlugins = true;

		pFE->pModule = NULL;
		pFE->pPlugin = NULL;
		pFE->pFormat = NULL;

		if ( pModule )
		{
			D->ListGetCurrentPos(12, &pos);
			ArchivePlugin* pPlugin = (ArchivePlugin*)D->ListGetData(12, pos.SelectPos);

			pFE->pModule = pModule;
			pFE->uidModule = pModule->GetUID();
			pFE->bAllModules = false;

			if ( pPlugin )
			{
				D->ListGetCurrentPos(14, &pos);
				ArchiveFormat* pFormat = (ArchiveFormat*)D->ListGetData(14, pos.SelectPos);

				pFE->pPlugin = pPlugin;
				pFE->uidPlugin = pPlugin->GetUID();
				pFE->bAllPlugins = false;

				if ( pFormat )
				{
					pFE->bAllFormats = false;
					pFE->uidFormat = pFormat->GetUID();
					pFE->pFormat = pFormat;
				}
			}
		}

		return TRUE;
	}
	
	return D->DefDlgProc(nMsg, nParam1, nParam2);
}


bool dlgFilterOneFormat(ArchiveFilterEntry* pFE)
{
	bool bResult = false;

	FarDialog D(-1, -1, 50, 17);

	D.DoubleBox(3, 1, 46, 15, _T("Add filter")); //0

	int Y = 2;

	D.Text(5, Y, _T("Action:")); //1
	D.ComboBox(15, Y++, 20, NULL); //2
	D.SetFlags(DIF_DROPDOWNLIST); 

	D.Separator(Y++); //3

	D.Text(5, Y, _T("Name:")); //4
	D.Edit(15, Y++, 20, pFE->strName); //5

	D.Text(5, Y, _T("Mask:")); //6
	D.Edit(15, Y++, 20, pFE->strMask);  //7

	D.Separator(Y++); //8

	D.Text(5, Y, _T("Module:")); //9
	D.ComboBox(5+8, Y++, 30, NULL, 0); //10
	D.SetFlags(DIF_DROPDOWNLIST); 

	D.Text(5, Y, _T("Plugin:")); //11
	D.ComboBox(5+8, Y++, 30, NULL, 0); //12
	D.SetFlags(DIF_DROPDOWNLIST); 

	D.Text(5, Y, _T("Format:")); //13
	D.ComboBox(5+8, Y++, 30, NULL, 0); //14
	D.SetFlags(DIF_DROPDOWNLIST);

	D.Separator(Y++); //15

	D.CheckBox(5, Y++, pFE->bEnabled, _T("Enabled")); //16
	D.CheckBox(5, Y++, pFE->bContinue, _T("Continue processing")); //17
	D.Separator(Y++); //18

	D.Button (-1, Y, _T("Add")); //19
	D.DefaultButton ();

	D.Button (-1, Y, _M(MSG_cmn_B_CANCEL)); //20

	if ( D.Run(hndFilterOneFormat, (void*)pFE) == D.FirstButton() )
	{
		pFE->bEnabled = D.GetResultCheck(16);
		pFE->bContinue = D.GetResultCheck(17);

		bResult = true;
	}

	return bResult;
}


LONG_PTR __stdcall hndArchiveFilter(FarDialog* D, int nMsg, int nParam1, LONG_PTR nParam2)
{
	ArchiveFilter* pFilter = (ArchiveFilter*)D->GetDlgData();

	if ( nMsg == DN_INITDIALOG )
	{
		ArchiveFilterArray filters;

		pFilter->GetFilters(filters);

		for (unsigned int i = 0; i < filters.count(); i++)
		{
			ArchiveFilterEntry* pFE = filters[i];

			string strTitle;

			strTitle.Format(
					_T("%c%c | %-20.20s | %-5.5s | %-10.10s"), 
					pFE->bExcludeFilter?_T('-'):_T(' '),
					_T(' '),
					pFE->strName.GetString(), 
					pFE->strMask.GetString(), 
					_T("")//pFE->bAllModules?_T("All"):FSF.PointToName(pFE->pModule->GetModuleName())
					);

			int index = D->ListAddStr(3, strTitle);

			ArchiveFilterEntry* pCFE = new ArchiveFilterEntry;
			pFE->Clone(pCFE);

			D->ListSetDataEx(3, index, (void*)pCFE, sizeof(void*));
		}
	}

	if ( nMsg == DN_CLOSE )
	{
		if ( nParam1 == 3 ) //list
			return FALSE;

		FarListInfo info;
		D->ListInfo(3, &info);

		if ( nParam1 == D->FirstButton() )
		{
			pFilter->Clear();

			for (int i = 0; i < info.ItemsNumber; i++)
			{
				ArchiveFilterEntry* pFE = (ArchiveFilterEntry*)D->ListGetData(3, i);
				pFilter->AddFilter(pFE);
			}

			return TRUE;
		}

		for (int i = 0; i < info.ItemsNumber; i++)
		{
			ArchiveFilterEntry* pFE = (ArchiveFilterEntry*)D->ListGetData(3, i);
			delete pFE;
		}

		return TRUE;
	}

	if ( nMsg == DN_BTNCLICK )
	{
		if ( nParam1 == 7 )
		{
			ArchiveFilterEntry* pFE = new ArchiveFilterEntry;

			pFE->bEnabled = true;

			if ( dlgFilterOneFormat(pFE) )
			{
				string strTitle;

			strTitle.Format(
					_T("%c%c | %-20.20s | %-5.5s | %-10.10s"), 
					pFE->bExcludeFilter?_T('-'):_T(' '),
					_T(' '),
					pFE->strName.GetString(), 
					pFE->strMask.GetString(),
					_T("")//pFE->bAllModules?_T("All"):FSF.PointToName(pFE->pModule->GetModuleName())
					);

				int index = D->ListAddStr(3, strTitle);
				D->ListSetDataEx(3, index, (void*)pFE, sizeof(void*));
			}
			else
				delete pFE;
		}
		else

		if ( (nParam1 == 8) || (nParam1 == 9) )
		{
			FarListPos pos;

			D->ListGetCurrentPos(3, &pos);

			if ( pos.SelectPos != -1 )
			{
				if ( nParam1 == 8 )
				{
					ArchiveFilterEntry* pFE = (ArchiveFilterEntry*)D->ListGetData(3, pos.SelectPos);

					delete pFE;

					FarListDelete del;

					del.StartIndex = pos.SelectPos;
					del.Count = 1;

					D->ListDelete(3, &del);
				}

				if ( nParam1 == 9 )
				{
					ArchiveFilterEntry* pFE = (ArchiveFilterEntry*)D->ListGetData(3, pos.SelectPos);

			 		if ( dlgFilterOneFormat(pFE) )
			 		{
						string strTitle;
				
			strTitle.Format(
					_T("%c%c | %-20.20s | %-5.5s | %-10.10s"), 
					pFE->bExcludeFilter?_T('-'):_T(' '),
					_T(' '),
					pFE->strName.GetString(), 
					pFE->strMask.GetString(), 
					_T("")//pFE->bAllModules?_T("All"):FSF.PointToName(pFE->pModule->GetModuleName())
					);

			 			FarListUpdate update;

			 			update.Index = pos.SelectPos;

			 			update.Item.Flags = LIF_SELECTED;
#ifdef UNICODE
						update.Item.Text = strTitle;
#else
						strcpy(update.Item.Text, strTitle.GetString());
#endif
						D->ListUpdate(3, &update);
					}
				}
			}
		}
	}

	return D->DefDlgProc(nMsg, nParam1, nParam2);
}

bool dlgArchiveFilter(ArchiveFilter* pFilter)
{
	if ( !pFilter )
	{
		msgError(_T("dlgFilterEntry: empty pFilter error!"));
		return false;
	}

	FarDialog D(-1, -1, 70, 15);

	D.DoubleBox(3, 1, 66, 13, _T("Filter config")); //0

	int Y = 2;

	D.CheckBox(5, Y++, pFilter->UseRemaining(), _T("Use all remaining formats"));  //1
	D.Separator(Y++);  //2

	D.ListBox(5, Y, 60, Y+4, NULL); //3 fill in handler
	D.SetFlags(DIF_LISTNOBOX);

	Y += 5;

	D.Separator(Y++); //4

	D.Button(35, Y, _T("Up")); //5
	D.SetFlags(DIF_BTNNOCLOSE);

	D.Button(41, Y, _T("Down")); //6
	D.SetFlags(DIF_BTNNOCLOSE);

	D.Button(50, Y, _T("[+]")); //7
	D.SetFlags(DIF_BTNNOCLOSE);
	
	D.Button(54, Y, _T("[-]")); //8
	D.SetFlags(DIF_BTNNOCLOSE);

	D.Button(58, Y++, _T("[*]")); //9
	D.SetFlags(DIF_BTNNOCLOSE);

	D.Separator(Y++); //10

	D.Button (-1, Y, _M(MSG_cmn_B_OK)); //11
	D.DefaultButton ();

	D.Button (-1, Y, _M(MSG_cmn_B_CANCEL)); //12

	if ( D.Run(hndArchiveFilter, (void*)pFilter) == D.FirstButton() )
	{
		pFilter->SetRemaining(D.GetResultCheck(1));
		return true;
	}

	return false;
}
