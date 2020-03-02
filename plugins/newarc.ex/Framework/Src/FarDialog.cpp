#include "FarPluginBase.hpp"

unsigned int cstrlen(const TCHAR *str)
{
	return StrLength(str)-(_tcschr (str, _T('&')) != NULL);
}

FarDialog::FarDialog()
{
	m_nFlags = 0;
	m_Info = &Info;

	m_nFirstButton = -1;
	m_lpHelpTopic = NULL;

	m_pfnDlgHandler = NULL;
	m_pUserData = NULL;

	m_hDlg = INVALID_HANDLE_VALUE;

#ifndef UNICODE
	m_pTempBuffer = NULL;
#endif
}

FarDialog::FarDialog (
		int X1,
		int Y1,
		int X2,
		int Y2
		)
{

	m_nFlags = 0;
	m_Info = &Info;

	m_nFirstButton = -1;
	m_lpHelpTopic = NULL;

	m_pfnDlgHandler = NULL;
	m_pUserData = NULL;

	m_hDlg = INVALID_HANDLE_VALUE;

#ifndef UNICODE
	m_pTempBuffer = NULL;
#endif

	SetPosition(X1, Y1, X2, Y2);
}

FarDialog::~FarDialog ()
{
	Done();
}

void FarDialog::SetPosition(int X1, int Y1, int X2, int Y2)
{
	m_X1 = X1;
	m_Y1 = Y1;
	m_X2 = X2;
	m_Y2 = Y2;
}

int FarDialog::SetFlags(int Flags, int nCounter)
{
	if ( nCounter == CURRENT_ITEM )
		nCounter = m_items.count()-1;

	m_items[nCounter].Flags |= Flags;

	return m_items[nCounter].Flags;
}

void FarDialog::Focus(int nCounter)
{
	if ( nCounter == CURRENT_ITEM )
		nCounter = m_items.count()-1;

	m_items[nCounter].Focus = TRUE;
}

void FarDialog::DefaultButton(int nCounter)
{
	if ( nCounter == CURRENT_ITEM )
		nCounter = m_items.count()-1;

	m_items[nCounter].DefaultButton = TRUE;

	if ( m_items[nCounter].Type == DI_BUTTON )
		m_nFirstButton = nCounter;
}

void FarDialog::Control (int Type, int X, int Y, int X2, int Y2, const TCHAR *Data, int DataLength)
{
	FarDialogItem *item = m_items.add();

	if ( item )
	{
		item->Type = Type;
		item->X1   = X;
		item->Y1   = Y;
		item->X2   = X2;
		item->Y2   = Y2;

		if ( Data )
		{
			if (DataLength == -1)
				DataLength = StrLength(Data);

#ifdef _UNICODE
			item->PtrData = StrDuplicate(Data);
			item->MaxLen = 0;
			//item->, DataLength);
#else
			memcpy (item->Data, Data, DataLength);
#endif
		}
	}
}

void FarDialog::Text (int X, int Y, const TCHAR *Caption, int CaptionLength)
{
	Control (DI_TEXT, X, Y, 0, 0, Caption, CaptionLength);
}

void FarDialog::Text(int X1, int Y, int X2, const TCHAR *Caption, int CaptionLength)
{
	Control (DI_TEXT, X1, Y, X2, Y, Caption, CaptionLength);
}

void FarDialog::VText (int X, int Y, int Y2, const TCHAR *Caption, int CaptionLength)
{
	Control (DI_VTEXT, X, Y, 0, Y2, Caption, CaptionLength);
}


void FarDialog::TextEx (int X, int Y, const TCHAR *lpFormat, ...)
{
	TCHAR Caption[512];
	va_list list;

	va_start (list, lpFormat);
	wvsprintf (Caption, lpFormat, list);
	va_end (list);

	Text (X, Y, Caption, AUTO_LENGTH);
}

void FarDialog::SingleBox (int X, int Y, int X2, int Y2, const TCHAR *Caption, int CaptionLength)
{
	Control (DI_SINGLEBOX, X, Y, X2, Y2, Caption, CaptionLength);
}

void FarDialog::DoubleBox (int X, int Y, int X2, int Y2, const TCHAR *Caption, int CaptionLength)
{
	Control (DI_DOUBLEBOX, X, Y, X2, Y2, Caption, CaptionLength);
}

void FarDialog::CheckBox (int X, int Y, bool Checked, const TCHAR *Caption, int CaptionLength)
{
	Control (DI_CHECKBOX, X, Y, 0, 0, Caption, CaptionLength);
	m_items[m_items.count()-1].Selected = Checked;
}

void FarDialog::RadioButton (int X, int Y, bool Selected, const TCHAR *Caption,  bool First, int CaptionLength)
{
	Control (DI_RADIOBUTTON, X, Y, 0, 0, Caption, CaptionLength);

	m_items[m_items.count()-1].Selected = Selected;

	if ( First )
		SetFlags (DIF_GROUP);
}

void FarDialog::ListBox (int X, int Y, int X2, int Y2, FarList *ListItems, int ListPos)
{
	Control (DI_LISTBOX, X, Y, X2, Y2, NULL, -1);
	m_items[m_items.count()-1].ListItems = ListItems;
	//m_Items[m_Counter-1].ListPos = ListPos;

}

void FarDialog::ComboBox (int X, int Y, int Length, FarList *ListItems, int ListPos, const TCHAR *Data, int DataLength)
{
	Control (DI_COMBOBOX, X, Y, X+Length-1, Y, Data, DataLength);
	m_items[m_items.count()-1].ListItems = ListItems;
}

int FarDialog::Button (int X, int Y, const TCHAR *Caption, int CaptionLength)
{
	Control (DI_BUTTON, X, Y, 0, 0, Caption, CaptionLength);

	int Length = 0;

	if (Caption)
		Length = cstrlen (Caption);

	if ( (CaptionLength != -1) && (CaptionLength < Length) )
		Length = CaptionLength;

	if ( m_nFirstButton == -1 )
		m_nFirstButton = m_items.count()-1;

	if ( X == -1 ) // TO MAKE ButtonEx?
		SetFlags(DIF_CENTERGROUP);

	return X+Length+4; // ?
}



void FarDialog::Edit (int X, int Y, int Length, const TCHAR *Data, int DataLength, const TCHAR *History)
{
	Control (DI_EDIT, X, Y, X+Length-1, 0, Data, DataLength);

	if ( History )
	{
		SetFlags (DIF_HISTORY);
		m_items[m_items.count()-1].History = (TCHAR*)History;
	}

}

void FarDialog::PswEdit(int X, int Y, int Length, const TCHAR *Data, int DataLength)
{
	Edit (X, Y, Length, Data, DataLength, NULL);
	m_items[m_items.count()-1].Type = DI_PSWEDIT;
}

void FarDialog::FixEdit(int X, int Y, int Length, const TCHAR *Data, int DataLength, const TCHAR *History, const TCHAR *Mask)
{
	Edit (X, Y, Length, Data, DataLength, History);

	m_items[m_items.count()-1].Type = DI_FIXEDIT;

	if (Mask)
	{
		SetFlags (DIF_MASKEDIT);
		m_items[m_items.count()-1].Mask = (TCHAR*)Mask;
	}
}


int FarDialog::Run(const TCHAR *lpHelpTopic)
{
	int nResult;

	m_lpHelpTopic = lpHelpTopic;

#ifdef _UNICODE

	m_hDlg = m_Info->DialogInit(
			m_Info->ModuleNumber,
			m_X1,
			m_Y1,
			m_X2,
			m_Y2,
			m_lpHelpTopic,
			m_items.data(),
			m_items.count(),
			0,
			m_nFlags,
			0,
			0
			);

	nResult = m_Info->DialogRun (m_hDlg);

#else

	nResult = m_Info->Dialog(
			m_Info->ModuleNumber,
			m_X1,
			m_Y1,
			m_X2,
			m_Y2,
			m_lpHelpTopic,
			m_items.data(),
			m_items.count()
			);
#endif

	return nResult;
}

void FarDialog::Done()
{
#ifdef UNICODE
	for (unsigned int i = 0; i < m_items.count(); i++)
		StrFree((void*)m_items[i].PtrData);

	if ( m_hDlg != INVALID_HANDLE_VALUE )
		m_Info->DialogFree(m_hDlg);

#else
	if ( m_pTempBuffer )
		delete [] m_pTempBuffer;
#endif

}
	

PluginStartupInfo *CurrentInfo;

LONG_PTR __stdcall DialogHandler (
		HANDLE hDlg,
		int Msg,
		int Param1,
		LONG_PTR Param2
		)
{
	FarDialog *D;

	if (Msg == DN_INITDIALOG)
	{
		D = (FarDialog*)Param2;
		D->SetDlg(hDlg);
		return D->DlgProc (Msg, Param1, (LONG_PTR)D->GetDlgData());
	}
	else
	{
		D = (FarDialog*)CurrentInfo->SendDlgMessage (hDlg, DM_GETDLGDATA, NULL, NULL);
		return D->DlgProc (Msg, Param1, Param2);
	}
}


int FarDialog::Run(
		DIALOGHANDLER DlgProc, 
		PVOID Param, 
		const TCHAR *lpHelpTopic
		)
{
	int nResult;

	CurrentInfo = m_Info;

	m_lpHelpTopic = lpHelpTopic;

	m_pfnDlgHandler = DlgProc;
	m_pUserData = Param;

#ifdef _UNICODE

	m_hDlg = m_Info->DialogInit(
			m_Info->ModuleNumber,
			m_X1,
			m_Y1,
			m_X2,
			m_Y2,
			m_lpHelpTopic,
			m_items.data(),
			m_items.count(),
			0,
			m_nFlags,
			(FARWINDOWPROC)DialogHandler,
			(LONG_PTR)this
			);

	nResult = m_Info->DialogRun(m_hDlg);

#else
	
	nResult = m_Info->DialogEx(
			m_Info->ModuleNumber,
			m_X1,
			m_Y1,
			m_X2,
			m_Y2,
			m_lpHelpTopic,
			m_items.data(),
			m_items.count(),
			0,
			m_nFlags,
			(FARWINDOWPROC)DialogHandler,
			(LONG_PTR)this
			);

#endif

	return nResult;
}

