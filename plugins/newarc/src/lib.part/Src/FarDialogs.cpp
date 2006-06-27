#include <FarDialogs.h>

int cstrlen (const char *str)
{
	return strlen (str)-(strchr (str,'&') != NULL);
}


FarDialog::FarDialog (
		int X1,
		int Y1,
		int X2,
		int Y2
		)
{
	m_X1 = X1;
	m_Y1 = Y1;
	m_X2 = X2;
	m_Y2 = Y2;

	m_Count = 3;
	m_RealCount = 0;

	m_Flags       = 0;
	m_Counter     = 0;
	m_Items = (FarDialogItem*)malloc (m_Count*sizeof (FarDialogItem));

	m_Info        = &Info;

	m_nFirstButton = -1;
	m_lpHelpTopic = NULL;
}

FarDialog::~FarDialog ()
{
	free (m_Items);
}



int FarDialog::SetFlags (int Flags, int nCounter)
{
	if ( nCounter == CURRENT_ITEM )
		nCounter = m_Counter-1;

	m_Items[nCounter].Flags |= Flags;

	return m_Items[nCounter].Flags;
}

void FarDialog::SetFocus (int nCounter)
{
	if ( nCounter == CURRENT_ITEM )
		nCounter = m_Counter-1;

	m_Items[nCounter].Focus = TRUE;
}

void FarDialog::DefaultButton (int nCounter)
{
	if ( nCounter == CURRENT_ITEM )
		nCounter = m_Counter-1;

	m_Items[nCounter].DefaultButton = TRUE;
}

void FarDialog::Control (int Type, int X, int Y, int X2, int Y2, const char *Data, int DataLength)
{
	if ( m_RealCount == m_Count )
	{
		m_Count += 3;
		m_Items = (FarDialogItem*)realloc (m_Items, m_Count*sizeof (FarDialogItem));
	}

//#ifdef _DEBUG

	memset (&m_Items[m_Counter], 0, sizeof (FarDialogItem));

//#endif

	m_Items[m_Counter].Type = Type;
	m_Items[m_Counter].X1   = X;
	m_Items[m_Counter].Y1   = Y;
	m_Items[m_Counter].X2   = X2;
	m_Items[m_Counter].Y2   = Y2;

	if ( Data )
	{
		if (DataLength == -1)
			DataLength = strlen (Data);

		memcpy (m_Items[m_Counter].Data, Data, DataLength);
	}

	m_RealCount++;
	m_Counter++;
}

void FarDialog::Text (int X, int Y, const char *Caption, int CaptionLength)
{
	Control (DI_TEXT, X, Y, 0, 0, Caption, CaptionLength);
}

void FarDialog::VText (int X, int Y, int Y2, const char *Caption, int CaptionLength)
{
	Control (DI_VTEXT, X, Y, 0, Y2, Caption, CaptionLength);
}


void FarDialog::TextEx (int X, int Y, const char *lpFormat, ...)
{
	char Caption[512];
	va_list list;

	va_start (list, lpFormat);
	wvsprintf (Caption, lpFormat, list);
	va_end (list);

	Text (X, Y, Caption, AUTO_LENGTH);
}

void FarDialog::SingleBox (int X, int Y, int X2, int Y2, const char *Caption, int CaptionLength)
{
	Control (DI_SINGLEBOX, X, Y, X2, Y2, Caption, CaptionLength);
}

void FarDialog::DoubleBox (int X, int Y, int X2, int Y2, const char *Caption, int CaptionLength)
{
	Control (DI_DOUBLEBOX, X, Y, X2, Y2, Caption, CaptionLength);
}

void FarDialog::CheckBox (int X, int Y, bool Checked, const char *Caption, int CaptionLength)
{
	Control (DI_CHECKBOX, X, Y, 0, 0, Caption, CaptionLength);
	m_Items[m_Counter-1].Selected = Checked;
}

void FarDialog::RadioButton (int X, int Y, bool Selected, const char *Caption,  bool First, int CaptionLength)
{
	Control (DI_RADIOBUTTON, X, Y, 0, 0, Caption, CaptionLength);

	m_Items[m_Counter-1].Selected = Selected;

	if ( First )
		SetFlags (DIF_GROUP);
}

void FarDialog::ListBox (int X, int Y, int X2, int Y2, FarList *ListItems, int ListPos)
{
	Control (DI_LISTBOX, X, Y, X2, Y2, NULL, -1);
	m_Items[m_Counter-1].ListItems = ListItems;
	//m_Items[m_Counter-1].ListPos = ListPos;

}

void FarDialog::ComboBox (int X, int Y, int Length, FarList *ListItems, int ListPos, const char *Data, int DataLength)
{
	Control (DI_COMBOBOX, X, Y, X+Length-1, Y, Data, DataLength);
	m_Items[m_Counter-1].ListItems = ListItems;
	//m_Items[m_Counter-1].ListPos = ListPos;
}

int FarDialog::Button (int X, int Y, const char *Caption, int CaptionLength)
{
	Control (DI_BUTTON, X, Y, 0, 0, Caption, CaptionLength);

	int Length = 0;

	if (Caption)
		Length = cstrlen (Caption);

	if ( (CaptionLength != -1) && (CaptionLength < Length) )
		Length = CaptionLength;

	if ( m_nFirstButton == -1 )
		m_nFirstButton = m_Counter-1;

	if ( X == -1 ) // TO MAKE ButtonEx?
		SetFlags (DIF_CENTERGROUP);

	return X+Length+4; // ?
}



void FarDialog::Edit (int X, int Y, int Length, const char *Data, int DataLength, const char *History)
{
	Control (DI_EDIT, X, Y, X+Length-1, 0, Data, DataLength);

	if ( History )
	{
		SetFlags (DIF_HISTORY);
		m_Items[m_Counter-1].History = (char*)History;
	}

}

void FarDialog::PswEdit (int X, int Y, int Length, const char *Data, int DataLength)
{
	Edit (X, Y, Length, Data, DataLength, NULL);
	m_Items[m_Counter-1].Type = DI_PSWEDIT;
}

void FarDialog::FixEdit (int X, int Y, int Length, const char *Data, int DataLength, const char *History, const char *Mask)
{
	Edit (X, Y, Length, Data, DataLength, History);

	m_Items[m_Counter-1].Type = DI_FIXEDIT;

	if (Mask)
	{
		SetFlags (DIF_MASKEDIT);
		m_Items[m_Counter-1].Mask = (char*)Mask;
	}
}


int FarDialog::Show(const char *lpHelpTopic)
{
	m_lpHelpTopic = lpHelpTopic;

	return m_Info->Dialog (
			m_Info->ModuleNumber,
			m_X1,
			m_Y1,
			m_X2,
			m_Y2,
			m_lpHelpTopic,
			m_Items,
			m_RealCount
			);
}


PluginStartupInfo *CurrentInfo;

int __stdcall DialogHandler (
		HANDLE hDlg,
		int Msg,
		int Param1,
		int Param2
		)
{
	FarDialogHandler *D;

	if (Msg == DN_INITDIALOG)
	{
		D = (FarDialogHandler*)Param2;
		D->SetDlg(hDlg);
		return D->DlgProc (Msg, Param1, (int)D->GetDlgData());
	}
	else
	{
		D = (FarDialogHandler*)CurrentInfo->SendDlgMessage (hDlg, DM_GETDLGDATA, 0, 0);
		return D->DlgProc (Msg, Param1, Param2);
	}
}


int FarDialog::ShowEx(PVOID DlgProc, PVOID Param, const char *lpHelpTopic)
{
	PVOID              DialogProc;
	FarDialogHandler *Handler;

	CurrentInfo = m_Info;

	m_lpHelpTopic = lpHelpTopic;

	Handler = new FarDialogHandler;
	Handler->Create (this, (DIALOGHANDLER)DlgProc, Param);

	DialogProc = (PVOID)DialogHandler;

	int Result = m_Info->DialogEx (
			m_Info->ModuleNumber,
			m_X1,
			m_Y1,
			m_X2,
			m_Y2,
			m_lpHelpTopic,
			m_Items,
			m_RealCount,
			0,
			m_Flags,
			(FARWINDOWPROC)DialogProc,
			(int)Handler
			);

	delete Handler;

	return Result;
}


//========================================================

#ifdef PAGED_DIALOGS

FarPagedDialog::FarPagedDialog (int X1, int Y1, int X2, int Y2) : FarDialog (X1, Y1, X2, Y2)
{
	m_Pages = new Collection<PageInfo*>;
	m_Pages->Create (5);
	m_CurrentPage = 0;
}

FarPagedDialog::~FarPagedDialog ()
{
	m_Pages->Free ();
	delete m_Pages;
}

void FarPagedDialog::NewPage (int nItems)
{
	PageInfo *Info = new PageInfo;

	Info->StartItem = m_Counter;
	Info->nItems     = nItems;

	m_Pages->Add (Info);
}

void FarPagedDialogHandler::PreparePage (bool bDirect)
{
	Collection <PageInfo*> *Pages = m_Owner->m_Pages;
	if ( Pages->Count > 1)
	{

		char *temp = StrCreate (128);
		FSF.sprintf (temp, "(%d/%d)", m_Owner->m_CurrentPage+1, Pages->Count);
		SetTextPtr (m_Owner->m_RealCount-3, temp);
		StrFree (temp);

		int btnNext = m_Owner->m_RealCount-1;
		int btnPrev = m_Owner->m_RealCount-2;


		if ( m_Owner->m_CurrentPage == Pages->Count-1 )
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

		for (int Page = 0; Page < Pages->Count; Page++)
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


int __stdcall PagedDialogHandler (
		HANDLE hDlg,
		int Msg,
		int Param1,
		int Param2
		)
{
	FarPagedDialogHandler *D;
	FarPagedDialog *Dialog;

	if (Msg == DN_INITDIALOG)
	{
		D = (FarPagedDialogHandler*)Param2;
		D->hDlg = hDlg;

		D->PreparePage (false);

		return D->DlgProc (Msg, Param1, (int)D->Data);
	}
	else
	{
		D = (FarPagedDialogHandler*)CurrentInfo->SendDlgMessage (hDlg, DM_GETDLGDATA, NULL, NULL);
		Dialog = D->m_Owner;

		if ((Msg == DN_BTNCLICK) &&
		    ((Param1 == Dialog->m_RealCount-1) ||
		     (Param1 == Dialog->m_RealCount-2)))
		{

			if (Param1 == Dialog->m_RealCount-1)
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


int FarPagedDialog::ShowEx (void *DlgProc, void *Param)
{
	PVOID              DialogProc;
	FarPagedDialogHandler *Handler;

	CurrentInfo = m_Info;

	Handler = new FarPagedDialogHandler;
	Handler->Create (this, (DIALOGHANDLER)DlgProc, Param);

	Handler->m_Owner = this;

	if (m_Pages->Count > 1)
	{
		Text (m_X2-8, 0);

		Button (Button (m_X2-15, m_Y2-1, "<<"), m_Y2-1, ">>");

		SetFlags (DIF_BTNNOCLOSE | DIF_DISABLE, GetCounter()-1);
		SetFlags (DIF_BTNNOCLOSE | DIF_DISABLE, GetCounter()-2);

		DialogProc = PagedDialogHandler;
	}
	else
		DialogProc = DialogHandler;

	int Result = m_Info->DialogEx (
			m_Info->ModuleNumber,
			m_X1,
			m_Y1,
			m_X2,
			m_Y2,
			NULL,
			m_Items,
			m_RealCount,
			0,
			m_Flags,
			(FARWINDOWPROC)DialogProc,
			(int)Handler
			);

	delete Handler;

	return Result;

}

#endif //PAGED_DIALOGS
