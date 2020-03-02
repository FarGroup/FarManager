//========================================================

#ifdef PAGED_DIALOGS

FarPagedDialog::FarPagedDialog(int X1, int Y1, int X2, int Y2) : FarDialog (X1, Y1, X2, Y2)
{
	m_Pages = new pointer_array<PageInfo*>(ARRAY_OPTIONS_DELETE);
	m_CurrentPage = 0;
}

FarPagedDialog::~FarPagedDialog()
{
	m_Pages->free ();
	delete m_Pages;
}

void FarPagedDialog::NewPage(int nItems)
{
	PageInfo *Info = new PageInfo;

	Info->StartItem = count();
	Info->nItems = nItems;

	m_Pages->add (Info);
}

void FarPagedDialogHandler::PreparePage (bool bDirect)
{
	array<PageInfo*> *Pages = m_Owner->m_Pages;

	if ( Pages->count() > 1)
	{

		TCHAR *temp = StrCreate (128);
		FSF.sprintf (temp, _T("(%d/%d)"), m_Owner->m_CurrentPage+1, Pages->count());
		SetTextPtr (m_Owner->count()-3, temp);
		StrFree (temp);

		int btnNext = m_Owner->count()-1;
		int btnPrev = m_Owner->count()-2;


		if ( m_Owner->m_CurrentPage == Pages->count()-1 )
		{
			Enable  (btnPrev, true);

			if ( Focus == btnNext )
				Focus = Focus-1;

			Enable (btnNext, false);
		}
		else
			Enable (btnNext, true);

		if ( m_Owner->m_CurrentPage == 0 )
		{
			Enable  (btnNext, true);

			if ( Focus == btnPrev )
				Focus = Focus+1;

			Enable (btnPrev, false);
		}
		else
			Enable (btnPrev, true);

		EnableRedraw (false);

		for (unsigned int Page = 0; Page < Pages->count(); Page++)
		{
			PageInfo *Info = (*Pages)[Page];

			for (int i = Info->StartItem; i < Info->StartItem+Info->nItems; i++)
				ShowItem (i, Page == m_Owner->m_CurrentPage);
		}

		if ( bDirect )
		{
			int f = (*Pages)[m_Owner->m_CurrentPage]->StartItem;

			while ( !SetFocus (f) ) f++;
		}

		EnableRedraw (true);

	}

}


LONG_PTR __stdcall PagedDialogHandler (
		HANDLE hDlg,
		int Msg,
		int Param1,
		LONG_PTR Param2
		)
{
	FarPagedDialogHandler *D;
	FarPagedDialog *Dialog;

	if (Msg == DN_INITDIALOG)
	{
		D = (FarPagedDialogHandler*)Param2;
		D->hDlg = hDlg;

		D->PreparePage (false);

		return D->DlgProc (Msg, Param1, (LONG_PTR)D->Data);
	}
	else
	{
		D = (FarPagedDialogHandler*)CurrentInfo->SendDlgMessage (hDlg, DM_GETDLGDATA, NULL, NULL);
		Dialog = D->m_Owner;

		if ((Msg == DN_BTNCLICK) &&
		    ((Param1 == Dialog->count()-1) ||
		     (Param1 == Dialog->count()-2)))
		{

			if (Param1 == Dialog->count()-1)
				Dialog->m_CurrentPage++;
			else
				Dialog->m_CurrentPage--;

			D->PreparePage (false);

			D->RedrawDialog();

			return TRUE;
		}
		else
			return D->DlgProc (Msg, Param1, Param2);
	}
}


int FarPagedDialog::ShowEx (void *DlgProc, void *Param, const TCHAR *lpHelpTopic)
{
	PVOID              DialogProc;
	FarPagedDialogHandler *Handler;

	CurrentInfo = m_Info;

	m_lpHelpTopic = lpHelpTopic;

	Handler = new FarPagedDialogHandler;
	Handler->Create (this, (DIALOGHANDLER)DlgProc, Param);

	Handler->m_Owner = this;

	if (m_Pages->count() > 1)
	{
		Text (m_X2-10, 1);

		Button (Button (m_X2-17, m_Y2-2, _T("<<")), m_Y2-2, _T(">>"));

		SetFlags (DIF_BTNNOCLOSE | DIF_DISABLE, count()-1);
		SetFlags (DIF_BTNNOCLOSE | DIF_DISABLE, count()-2);

		DialogProc = PagedDialogHandler;
	}
	else
		DialogProc = DialogHandler;

	int Result;

#ifdef _UNICODE

	m_hDlg = m_Info->DialogInit(
			m_Info->ModuleNumber,
			m_X1,
			m_Y1,
			m_X2,
			m_Y2,
			m_lpHelpTopic,
			m_data,
			count(),
			0,
			m_nFlags,
			(FARWINDOWPROC)DialogProc,
			(LONG_PTR)Handler
			);

	Result = m_Info->DialogRun (m_hDlg);

	//m_Info->DialogFree(hDlg);

#else
	
	Result = m_Info->DialogEx (
			m_Info->ModuleNumber,
			m_X1,
			m_Y1,
			m_X2,
			m_Y2,
			m_lpHelpTopic,
			m_data,
			count(),
			0,
			m_nFlags,
			(FARWINDOWPROC)DialogProc,
			(LONG_PTR)Handler
			);

#endif

/*	int Result = m_Info->DialogEx (
			m_Info->ModuleNumber,
			m_X1,
			m_Y1,
			m_X2,
			m_Y2,
			m_lpHelpTopic,
			m_data,
			count(),
			0,
			m_nFlags,
			(FARWINDOWPROC)DialogProc,
			(LONG_PTR)Handler
			);*/

	delete Handler;

	return Result;

}

#endif PAGED_DIALOGS
