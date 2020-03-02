enum enumFilterOneFormat {
	ID_FOF_FILTER,
	ID_FOF_ACTION,
	ID_FOF_ACTIONLIST,
	ID_FOF_SEPARATOR1,
	ID_FOF_NAME,
	ID_FOF_NAMEEDIT,
	ID_FOF_MASK,
	ID_FOF_MASKEDIT,
	ID_FOF_SEPARATOR2,
	ID_FOF_MODULE,
	ID_FOF_MODULELIST,
	ID_FOF_PLUGIN,
	ID_FOF_PLUGINLIST,
	ID_FOF_FORMAT,
	ID_FOF_FORMATLIST,
	ID_FOF_SEPARATOR3,
	ID_FOF_ENABLED,
	ID_FOF_CONTINUEPROCESSING,
	ID_FOF_SEPARATOR4,
	ID_FOF_ADD,
	ID_FOF_CANCEL
};


void UpdateFormats(FarDialog* D, ArchivePlugin* pPlugin, ArchiveFilterEntry* pFE)
{
	int index = 0;

	D->ListDelete(ID_FOF_FORMATLIST, NULL);
	
	index = D->ListAddStr(ID_FOF_FORMATLIST, _T("All"));
	D->ListSetDataEx(ID_FOF_FORMATLIST, index, (void*)0, sizeof(void*));

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

				index = D->ListAddStr(ID_FOF_FORMATLIST, pFormat->GetName());
				D->ListSetDataEx(ID_FOF_FORMATLIST, index, (void*)pFormat, sizeof(void*));

				if ( pFE && !pFE->IsAllFormats() && pFE->GetFormat() && (pFE->GetFormat()->GetUID() == pFormat->GetUID()) )
					pos.SelectPos = index;
			}
		}
	}

	D->ListSetCurrentPos(ID_FOF_FORMATLIST, &pos);
}


void UpdatePlugins(FarDialog* D, ArchiveModule* pModule, ArchiveFilterEntry* pFE)
{
	int index = 0;

	D->ListDelete(ID_FOF_PLUGINLIST, NULL);
	
	index = D->ListAddStr(ID_FOF_PLUGINLIST, _T("All"));
	D->ListSetDataEx(ID_FOF_PLUGINLIST, index, (void*)0, sizeof(void*));

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

				index = D->ListAddStr(ID_FOF_PLUGINLIST, FSF.PointToName(pPlugin->GetModuleName()));
				D->ListSetDataEx(ID_FOF_PLUGINLIST, index, (void*)pPlugin, sizeof(void*));

				if ( pFE && !pFE->IsAllPlugins() && pFE->GetPlugin() && (pFE->GetPlugin()->GetUID() == pPlugin->GetUID()) )
					pos.SelectPos = index;
			}
		}
	}


	D->ListSetCurrentPos(ID_FOF_PLUGINLIST, &pos);
	ArchivePlugin* pPlugin = (ArchivePlugin*)D->ListGetData(ID_FOF_PLUGINLIST, pos.SelectPos);

	UpdateFormats(D, pPlugin, pFE);
}


LONG_PTR __stdcall hndFilterOneFormat(FarDialog* D, int nMsg, int nParam1, LONG_PTR nParam2)
{
	ArchiveFilterEntry* pFE = (ArchiveFilterEntry*)D->GetDlgData();

	if ( (nMsg == DN_LISTCHANGE) && (nParam1 == ID_FOF_ACTIONLIST) )
	{
		if ( nParam2 == 1 ) //do block
		{	
			FarListPos pos;

			D->ListGetCurrentPos(ID_FOF_MODULELIST, &pos);


			FarListInsert ins;

			ins.Index = 0;
			ins.Item.Flags = 0;
#ifdef UNICODE
			ins.Item.Text = _T("All");
#else
			strcpy(ins.Item.Text, _T("All"));
#endif

			int index = D->ListInsert(ID_FOF_MODULELIST, &ins);
			D->ListSetDataEx(ID_FOF_MODULELIST, index, (void*)0, sizeof(void*));

			pos.SelectPos++;
			D->ListSetCurrentPos(ID_FOF_MODULELIST, &pos);

		}
		else
		{
			FarListDelete del;

			del.StartIndex = 0;
			del.Count = 1;

			D->ListDelete(ID_FOF_MODULELIST, &del);
////////////
			FarListPos pos;
			D->ListGetCurrentPos(ID_FOF_MODULELIST, &pos);

			if ( pos.SelectPos != -1 )
			{
				ArchiveModule* pModule = (ArchiveModule*)D->ListGetData(ID_FOF_MODULELIST, pos.SelectPos);

				UpdatePlugins(D, pModule, NULL);
			}

			return TRUE;
//////////////


		}

		return TRUE;
	}
	else

	if ( (nMsg == DN_LISTCHANGE) && (nParam1 == ID_FOF_MODULELIST) )
	{
		FarListPos pos;
		D->ListGetCurrentPos(ID_FOF_MODULELIST, &pos);

		if ( pos.SelectPos != -1 )
		{
			ArchiveModule* pModule = (ArchiveModule*)D->ListGetData(ID_FOF_MODULELIST, pos.SelectPos);

			UpdatePlugins(D, pModule, NULL);
		}

		return TRUE;
	}
	else

	if ( (nMsg == DN_LISTCHANGE) && (nParam1 == ID_FOF_PLUGINLIST) )
	{
		FarListPos pos;
		D->ListGetCurrentPos(ID_FOF_PLUGINLIST, &pos);

		if ( pos.SelectPos != -1 )
		{
			ArchivePlugin* pPlugin = (ArchivePlugin*)D->ListGetData(ID_FOF_PLUGINLIST, pos.SelectPos);

			UpdateFormats(D, pPlugin, NULL);
		}

		return TRUE;
	}
	else

	if ( nMsg == DN_INITDIALOG )
	{
		FarListPos pos;

		D->ListAddStr(ID_FOF_ACTIONLIST, _T("Process"));
		D->ListAddStr(ID_FOF_ACTIONLIST, _T("Block"));

		pos.SelectPos = pFE->IsExclude()?1:0;

		D->ListSetCurrentPos(ID_FOF_ACTIONLIST, &pos);
		
		pos.SelectPos = 0;
		pos.TopPos = -1;

		Array<ArchiveModule*> modules;
		
		pManager->GetModules(modules);

		for (unsigned int i = 0; i < modules.count(); i++)
		{
			const ArchiveModule* pModule = modules[i];

			int index = D->ListAddStr(ID_FOF_MODULELIST, FSF.PointToName(pModule->GetModuleName()));
			D->ListSetDataEx(ID_FOF_MODULELIST, index, (void*)pModule, sizeof(void*));

			if ( pFE && !pFE->IsAllModules() && pFE->GetModule() && (pFE->GetModule()->GetUID() == pModule->GetUID()) )
				pos.SelectPos = index;
		}

		D->ListSetCurrentPos(ID_FOF_MODULELIST, &pos); 

		ArchiveModule* pModule = (ArchiveModule*)D->ListGetData(ID_FOF_MODULELIST, pos.SelectPos);
		UpdatePlugins(D, pModule, pFE); 

		//return TRUE; hmm
	}
	else

	if ( (nMsg == DN_CLOSE) && (nParam1 == D->FirstButton()) )
	{
		string strName = D->GetConstTextPtr(ID_FOF_NAMEEDIT);

		if ( strName.IsEmpty() )
		{
			msgError(_T("name empty"));
			return FALSE;
		}

		string strMask = D->GetConstTextPtr(ID_FOF_MASKEDIT);

		if ( strMask.IsEmpty() )
		{
			msgError(_T("mask empty"));
			return FALSE;
		}

		FarListPos pos;

		D->ListGetCurrentPos(ID_FOF_ACTIONLIST, &pos);

		pFE->SetExclude(pos.SelectPos == 1);
		pFE->SetName(strName);
		pFE->SetMask(strMask);

		pFE->SetAllFormats(true);
		pFE->SetAllModules(true);
		pFE->SetAllPlugins(true);

		D->ListGetCurrentPos(ID_FOF_MODULELIST, &pos);
		ArchiveModule* pModule = (ArchiveModule*)D->ListGetData(ID_FOF_MODULELIST, pos.SelectPos);

		if ( pModule )
		{
			pFE->SetModule(pModule);

			D->ListGetCurrentPos(ID_FOF_PLUGINLIST, &pos);
			ArchivePlugin* pPlugin = (ArchivePlugin*)D->ListGetData(ID_FOF_PLUGINLIST, pos.SelectPos);

			if ( pPlugin )
			{
				pFE->SetPlugin(pPlugin);

				D->ListGetCurrentPos(ID_FOF_FORMATLIST, &pos);
				ArchiveFormat* pFormat = (ArchiveFormat*)D->ListGetData(ID_FOF_FORMATLIST, pos.SelectPos);

				if ( pFormat )
					pFE->SetFormat(pFormat);
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
	D.Edit(15, Y++, 20, pFE->GetName()); //5

	D.Text(5, Y, _T("Mask:")); //6
	D.Edit(15, Y++, 20, pFE->GetMask());  //7

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

	D.CheckBox(5, Y++, pFE->IsEnabled(), _T("Enabled")); //16
	D.CheckBox(5, Y++, pFE->IsContinueProcessing(), _T("Continue processing")); //17
	D.Separator(Y++); //18

	D.Button (-1, Y, _T("Add")); //19
	D.DefaultButton ();

	D.Button (-1, Y, _M(MSG_cmn_B_CANCEL)); //20

	if ( D.Run(hndFilterOneFormat, (void*)pFE) == D.FirstButton() )
	{
		pFE->SetEnabled(D.GetResultCheck(ID_FOF_ENABLED));
		pFE->SetContinueProcessing(D.GetResultCheck(ID_FOF_CONTINUEPROCESSING));

		bResult = true;
	}

	return bResult;
}
