#pragma once
#include <Rtl.Base.h>
#include <Collections.h>

#pragma warning(disable:4800) // force value to bool
#pragma warning(disable:4018) // signed/unsigned mismatch


#define CreateList(l, items, default) \
		l->Items = (FarListItem*)malloc (items*sizeof (FarListItem)); \
		l->ItemsNumber = items; \
		l->Items[default].Flags |= LIF_SELECTED;

#define DeleteList(l) \
		free (l->Items); \
		delete l;


#define AUTO_LENGTH -1
#define CURRENT_ITEM -1


// STADNDARD FAR DIALOG

class FarDialogHandler;

class FarDialog {

private:

	int m_Count;

public:

	int m_RealCount;

	FarDialogItem *m_Items;
	int m_Counter;
	int m_X1, m_Y1, m_X2, m_Y2;
	int m_Flags;
	PluginStartupInfo *m_Info;

	const char *m_lpHelpTopic;

	int m_nFirstButton;

public:

	FarDialog (int X1, int Y1, int X2, int Y2);
	virtual ~FarDialog ();

	int GetCounter ()
		{	return m_Counter; }

	int SetFlags (int Flags, int nCounter = CURRENT_ITEM);

	void SetDialogFlags (int Flags)
		{ m_Flags = Flags; };

	int GetDialogFlags()
		{ return m_Flags; }

	void SetFocus (int nCounter = CURRENT_ITEM);
	void DefaultButton (int nCounter = CURRENT_ITEM);

	void Control (int Type, int X, int Y, int X2, int Y2, const char *Data = NULL, int DataLength = AUTO_LENGTH);
/*inline --//--*/	void SingleBox (int X, int Y, int X2, int Y2, const char *Caption = NULL, int CaptionLength = AUTO_LENGTH);
	void DoubleBox (int X, int Y, int X2, int Y2, const char *Caption = NULL, int CaptionLength = AUTO_LENGTH);
	void Edit (int X, int Y, int Length, const char *Data = NULL, int DataLength = AUTO_LENGTH, const char *History = NULL);
	void FixEdit (int X, int Y, int Length, const char *Data = NULL, int DataLength = AUTO_LENGTH, const char *History = NULL, const char *Mask =NULL);
	void PswEdit (int X, int Y, int Length, const char *Data = NULL, int DataLength = AUTO_LENGTH);
	int Button (int X, int Y, const char *Caption = NULL, int CaptionLength = AUTO_LENGTH);
	void Text (int X, int Y, const char *Caption = NULL, int CaptionLength = AUTO_LENGTH);

	void VText (int X, int Y, int Y2, const char *Caption = NULL, int CaptionLength = AUTO_LENGTH);

	void TextEx (int X, int Y, const char *lpFormat, ...);
	void CheckBox (int X, int Y, bool Checked = false, const char *Caption = NULL, int CaptionLength = AUTO_LENGTH);
	void RadioButton (int X, int Y, bool Selected = false, const char *Caption = NULL, bool First = false, int CaptionLength = AUTO_LENGTH);
	void ListBox (int X, int Y, int X2, int Y2, FarList *ListItems, int ListPos = 0);
	void ComboBox (int X, int Y, int Length, FarList *ListItems, int ListPos = 0, const char *Data = NULL, int DataLength = AUTO_LENGTH);

	int Show (const char *lpHelpTopic = NULL);
	virtual int ShowEx (void *DlgProc = NULL, void *Param = NULL, const char *lpHelpTopic = NULL);

	inline void Separator (int Y, const char *lpCaption = NULL) {	Text (-1, Y, lpCaption); SetFlags (DIF_SEPARATOR); }

friend class FarPagedDialog;
};

#ifdef PAGED_DIALOGS
// PAGED FAR DIALOG (remove, if not used!)

struct PageInfo {
	int StartItem;
	int nItems;
};


class FarPagedDialog : public FarDialog {

private:

	Collection<PageInfo*> *m_Pages;

public:

	int m_CurrentPage;

	FarPagedDialog (int X1, int Y1, int X2, int Y2);
	~FarPagedDialog ();

	void NewPage (int nItems);

	virtual int ShowEx (void *DlgProc = NULL, void *Param = NULL);

friend class FarPagedDialogHandler;
};

#endif //PAGED_DIALOGS

typedef int (__stdcall *DIALOGHANDLER) (FarDialogHandler*, int, int, int);

class FarDialogHandler {
public:
	HANDLE              m_hDlg;
	DIALOGHANDLER       m_DlgHandler;
	PVOID               m_Param;
	FarDialog         *m_Owner;

	void Create (FarDialog *Owner, DIALOGHANDLER Handler, PVOID Param)
		{ m_Owner = Owner; m_DlgHandler = Handler; m_Param = Param; }

	HANDLE GetDlg ()
		{ return m_hDlg;  }

	void SetDlg (HANDLE hDlg)
		{ m_hDlg = hDlg;  }

//__declspec (property (get=GetDlg, put=SetDlg)) HANDLE hDlg;

	void* GetDlgData ()
		{	return m_Param; }

	void* SetDlgData (void *pData)
   		{	void *pResult = m_Param; m_Param = pData; return pResult; }

//__declspec (property (get=GetDlgData, put=SetDlgData)) void* Data;

   	int Message (int Msg, int Param1, int Param2)
   		{ return m_Owner->m_Info->SendDlgMessage (m_hDlg, Msg, Param1, Param2); }

	int DefDlgProc (int Msg, int Param1, int Param2)
		{ return m_Owner->m_Info->DefDlgProc (m_hDlg, Msg, Param1, Param2); }

	int DlgProc (int Msg, int Param1, int Param2)
		{
			if (m_DlgHandler)
				return m_DlgHandler (this, Msg, Param1, Param2);
			else
				return m_Owner->m_Info->DefDlgProc (m_hDlg, Msg, Param1, Param2);
		}

	bool AddHistory (int ID, const char *History)
		{ return Message (DM_ADDHISTORY, ID, (int)History); }

	int CloseDialog (int ID)
		{ return Message (DM_CLOSE, ID, 0); }

	int EditUnchangedFlag (int ID, int UnchangedFlag)
		{ return Message (DM_EDITUNCHANGEDFLAG, ID, UnchangedFlag); }

	int Enable (int ID, int State)
		{ return Message (DM_ENABLE, ID, State); }

	void EnableRedraw (bool bEnable)
		{ Message (DM_ENABLEREDRAW, bEnable, 0); }

	int GetCheck (int ID)
		{ return Message (DM_GETCHECK, ID, 0); }

	bool GetCursorPos (int ID, COORD **Pos)
		{ return Message (DM_GETCURSORPOS, ID, (int)*Pos); }

	int GetCursorSize (int ID)
		{ return Message (DM_GETCURSORSIZE, ID, 0); }

	bool GetDlgItem (int ID, FarDialogItem **Item)
		{ return Message (DM_GETDLGITEM, ID, (int)*Item); }

	bool GetDlgRect (SMALL_RECT *Rect)
		{ return Message (DM_GETDLGRECT, 0, (int)Rect); }

	bool GetDropDownOpened ()
		{ return Message (DM_GETDROPDOWNOPENED, 0, 0); }

	int GetItemData (int ID)
		{ return Message (DM_GETITEMDATA, ID, 0); }

	bool GetItemPosition (int ID, SMALL_RECT *Pos)
		{ return Message (DM_GETITEMPOSITION, ID, (int)Pos); }

	int GetFocus ()
		{ return Message (DM_GETFOCUS, 0, 0); }

	int GetText (int ID, FarDialogItemData **Data)
		{ return Message (DM_GETTEXT, ID, (int)*Data); }

	int GetTextPtr (int ID, char *Buffer)
		{ return Message (DM_GETTEXTPTR, ID, (int)Buffer); }

	int GetTextLength (int ID)
		{ return Message (DM_GETTEXTLENGTH, ID, 0); }

	void SendKey (int Count, PDWORD Keys)
		{ Message (DM_KEY, Count, (int)Keys); }

	bool ListAdd (int ID, FarList *List)
		{ return Message (DM_LISTADD, ID, (int)List); }

	int ListAddStr (int ID, const char *Str)
		{ return Message (DM_LISTADDSTR, ID, (int)Str); }

    bool ListDelete (int ID, FarListDelete *Delete)
    	{ return Message (DM_LISTDELETE, ID, (int)Delete); }

	int ListFindString    (int ID, FarListFind *Find)
		{ return Message (DM_LISTFINDSTRING, ID, (int)Find); }

	int ListGetCurrentPos (int ID, FarListPos *Pos)
		{ return Message (DM_LISTGETCURPOS, ID, (int)Pos); }

	void* ListGetData (int ID, int Index)
		{ return (PVOID)Message (DM_LISTGETDATA, ID, Index); }

	bool ListGetItem (int ID, FarListGetItem *Item)
		{ return Message (DM_LISTGETITEM, ID, (int)Item); }

	bool ListGetTitles (int ID, FarListTitles **Titles)
		{ return Message (DM_LISTGETTITLES, ID, (int)*Titles); }

	bool ListInfo (int ID, FarListInfo *Info)
		{ return Message (DM_LISTINFO, ID, (int)Info); }

	int ListInsert (int ID, FarListInsert *Insert)
		{ return Message (DM_LISTINSERT, ID, (int)Insert); }

	int ListSet (int ID, FarList *List)
		{ return Message (DM_LISTSET, ID, (int)List); }

	int ListSetCurrentPos (int ID, FarListPos *Pos)
		{ return Message (DM_LISTSETCURPOS, ID, (int)Pos); }

	int ListSetData (int ID, FarListItemData *Data)
		{ return Message (DM_LISTSETDATA, ID, (int)Data); }

	int ListSetDataEx (int ID, int Index, void *pData, dword dwDataSize)
		{	FarListItemData Data = {Index, dwDataSize, pData, 0};
			return Message (DM_LISTSETDATA, ID, (int)&Data); }

	bool ListSetMouseReaction (int ID, int bReaction)
		{ return Message (DM_LISTSETMOUSEREACTION, ID, bReaction); }

	bool ListSetTitles (int ID, FarListTitles *Titles)
		{ return Message (DM_LISTSETTITLES, ID, (int)Titles); }

	bool ListSort (int ID, bool bReverse)
		{ return Message (DM_LISTSORT, ID, bReverse); }

	bool ListUpdate (int ID, FarListUpdate *Update)
		{ return Message (DM_LISTUPDATE, ID, (int)Update); }

	COORD MoveDialog (bool bAbsolute, COORD *NewPos)
		{ return *(COORD*)Message (DM_MOVEDIALOG, bAbsolute, (int)NewPos); }

	void RedrawDialog ()
		{ Message (DM_REDRAW, 0, 0); }

	COORD ResizeDialog (COORD *Size)
		{ return *(COORD*)Message (DM_RESIZEDIALOG, 0, (int)Size); }

	bool Set3State (int ID, bool b3State)
		{ return Message (DM_SET3STATE, ID, b3State); }

	int SetCheck (int ID, int State)
		{ return Message (DM_SETCHECK, ID, State); }

	bool SetCursorPos (int ID, COORD *Pos)
		{ return Message (DM_SETCURSORPOS, ID, (int)Pos); }

	int SetCursorSize (int ID, int Cursor)
		{ return Message (DM_SETCURSORSIZE, ID, Cursor); }

	bool SetDlgItem (int ID, FarDialogItem *Item)
		{ return Message (DM_SETDLGITEM, ID, (int)Item); }

	bool SetDropDownOpened (int ID, bool bOpen)
		{ return Message (DM_SETDROPDOWNOPENED, ID, bOpen); }

	bool SetFocus (int ID)
		{ return Message (DM_SETFOCUS, ID, 0); }

	bool SetHistory (int ID, const char *History)
		{ return Message (DM_SETHISTORY, ID, (int)History); }

	int SetItemData (int ID, int Data)
		{ return Message (DM_SETITEMDATA, ID, Data); }

	bool SetItemPosition (int ID, SMALL_RECT *Pos)
		{ return Message (DM_SETITEMPOSITION, ID, (int)Pos); }

	int SetMaxTextLength (int ID, int Length)
		{ return Message (DM_SETMAXTEXTLENGTH, ID, Length); }

	int SetMouseEventNotify  (int State)
		{ return Message (DM_SETMOUSEEVENTNOTIFY, State, 0); }

	int SetText (int ID, FarDialogItemData *Data)
		{ return Message (DM_SETTEXT, ID, (int)Data); }

	int SetTextPtr (int ID, const char *Text)
		{ return Message (DM_SETTEXTPTR, ID, (int)Text); }

	int SetTextLength (int ID, int Length)
		{ return Message (DM_SETTEXTLENGTH, ID, Length); }

	void ShowDialog (bool bShow)
		{ Message (DM_SHOWDIALOG, bShow, 0); }

	int ShowItem (int ID, int State)
		{ return Message (DM_SHOWITEM, ID, State); }

//__declspec (property (get=GetFocus, put=SetFocus)) int Focus;

};

#ifdef PAGED_DIALOGS

class FarPagedDialogHandler : public FarDialogHandler {

public:

	FarPagedDialog *m_Owner;

public:

	void PreparePage (bool bDirect);

	void SetPage (int nPage)
	{
		m_Owner->m_CurrentPage = nPage;
		PreparePage (true);
	}

};

#endif //PAGED_DIALOGS
