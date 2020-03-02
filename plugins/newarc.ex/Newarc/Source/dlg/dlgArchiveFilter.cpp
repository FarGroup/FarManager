#include "dlgFilterOneFormat.cpp"

enum enumArchiveFilter {
	ID_AF_TITLE,
	ID_AF_USEALLREMAINING,
	ID_AF_SEPARATOR,
	ID_AF_FILTERLIST,
	ID_AF_SEPARATOR2,
	ID_AF_UP,
	ID_AF_DOWN,
	ID_AF_ADD,
	ID_AF_REMOVE,
	ID_AF_EDIT,
	ID_AF_SEPARATOR3,
	ID_AF_OK,
	ID_AF_CANCEL
};


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
					pFE->IsExclude()?_T('-'):_T(' '),
					_T(' '),
					pFE->GetName(), 
					pFE->GetMask(), 
					_T("")//pFE->bAllModules?_T("All"):FSF.PointToName(pFE->pModule->GetModuleName())
					);

			int index = D->ListAddStr(3, strTitle);

			ArchiveFilterEntry* pCFE = new ArchiveFilterEntry;
			pFE->Clone(pCFE);

			D->ListSetDataEx(ID_AF_FILTERLIST, index, (void*)pCFE, sizeof(void*));
		}
	}

	if ( nMsg == DN_CLOSE )
	{
		if ( nParam1 == ID_AF_FILTERLIST ) //list
			return FALSE;

		FarListInfo info;
		D->ListInfo(ID_AF_FILTERLIST, &info);

		if ( nParam1 == D->FirstButton() )
		{
			pFilter->Clear();

			for (int i = 0; i < info.ItemsNumber; i++)
			{
				ArchiveFilterEntry* pFE = (ArchiveFilterEntry*)D->ListGetData(ID_AF_FILTERLIST, i);
				pFilter->AddFilter(pFE);
			}

			return TRUE;
		}

		for (int i = 0; i < info.ItemsNumber; i++)
		{
			ArchiveFilterEntry* pFE = (ArchiveFilterEntry*)D->ListGetData(ID_AF_FILTERLIST, i);
			delete pFE;
		}

		return TRUE;
	}

	if ( nMsg == DN_BTNCLICK )
	{
		if ( nParam1 == ID_AF_ADD )
		{
			ArchiveFilterEntry* pFE = new ArchiveFilterEntry;

			pFE->SetEnabled(true);

			if ( dlgFilterOneFormat(pFE) )
			{
				string strTitle;

				strTitle.Format(
						_T("%c%c | %-20.20s | %-5.5s | %-10.10s"), 
						pFE->IsExclude()?_T('-'):_T(' '),
						_T(' '),
						pFE->GetName(), 
						pFE->GetMask(),
						_T("")//pFE->bAllModules?_T("All"):FSF.PointToName(pFE->pModule->GetModuleName())
						);

				int index = D->ListAddStr(ID_AF_FILTERLIST, strTitle);
				D->ListSetDataEx(ID_AF_FILTERLIST, index, (void*)pFE, sizeof(void*));
			}
			else
				delete pFE;
		}
		else

		if ( (nParam1 == ID_AF_REMOVE) || (nParam1 == ID_AF_EDIT) )
		{
			FarListPos pos;

			D->ListGetCurrentPos(ID_AF_FILTERLIST, &pos);

			if ( pos.SelectPos != -1 )
			{
				if ( nParam1 == ID_AF_REMOVE )
				{
					ArchiveFilterEntry* pFE = (ArchiveFilterEntry*)D->ListGetData(ID_AF_FILTERLIST, pos.SelectPos);

					delete pFE;

					FarListDelete del;

					del.StartIndex = pos.SelectPos;
					del.Count = 1;

					D->ListDelete(ID_AF_FILTERLIST, &del);
				}

				if ( nParam1 == ID_AF_EDIT )
				{
					ArchiveFilterEntry* pFE = (ArchiveFilterEntry*)D->ListGetData(ID_AF_FILTERLIST, pos.SelectPos);

			 		if ( dlgFilterOneFormat(pFE) )
			 		{
						string strTitle;
				
						strTitle.Format(
								_T("%c%c | %-20.20s | %-5.5s | %-10.10s"), 
								pFE->IsExclude()?_T('-'):_T(' '),
								_T(' '),
								pFE->GetName(), 
								pFE->GetMask(), 
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
						D->ListUpdate(ID_AF_FILTERLIST, &update);
					}
				}
			}
		}
	}

	return D->DefDlgProc(nMsg, nParam1, nParam2);
}


bool dlgArchiveFilter(ArchiveManagerConfig* pCfg, ArchiveFilter* pFilter)
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
		pFilter->SetRemaining(D.GetResultCheck(ID_AF_USEALLREMAINING));
		return true;
	}

	return false;
}
