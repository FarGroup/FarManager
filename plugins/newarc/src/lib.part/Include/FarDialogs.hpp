#pragma once
#include <Rtl.Base.h>
#include <FarPluginBase.hpp>
#include <array.hpp>

#if defined(_MSC_VER)
#pragma warning(disable:4800) // force value to bool
#pragma warning(disable:4018) // signed/unsigned mismatch
#endif

#define CreateList(l, items, default) \
		l->Items = (FarListItem*)malloc(items*sizeof(FarListItem)); \
		l->ItemsNumber = items; \
		l->Items[default].Flags |= LIF_SELECTED;

#define DeleteList(l) \
		free(l->Items); \
		delete l;

#define AUTO_LENGTH -1
#define CURRENT_ITEM -1


// STADNDARD FAR DIALOG

class FarDialogHandler;

typedef LONG_PTR (__stdcall *DIALOGHANDLER) (FarDialogHandler*, int, int, LONG_PTR);

class FarDialog : public AutoArray<FarDialogItem> {

protected:

	int m_X1, m_Y1, m_X2, m_Y2;
	int m_nFlags;

	PluginStartupInfo *m_Info;

	const char *m_lpHelpTopic;

	int m_nFirstButton;

public:

	FarDialog (int X1, int Y1, int X2, int Y2);
	virtual ~FarDialog ();

	int SetFlags (int Flags, int nCounter = CURRENT_ITEM);

	void SetDialogFlags (int nFlags)
		{	m_nFlags = nFlags; };

	int GetDialogFlags ()
		{	return m_nFlags; }

	int FirstButton ()
		{	return m_nFirstButton; }

	void SetFocus (int nCounter = CURRENT_ITEM);
	void DefaultButton (int nCounter = CURRENT_ITEM);

	void Control (int Type, int X, int Y, int X2, int Y2, const char *Data = NULL, int DataLength = AUTO_LENGTH);
	void SingleBox (int X, int Y, int X2, int Y2, const char *Caption = NULL, int CaptionLength = AUTO_LENGTH);
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
	void Separator (int Y, const char *lpCaption = NULL) {	Text (-1, Y, lpCaption); SetFlags (DIF_SEPARATOR); }

	int Show (const char *lpHelpTopic = NULL);
	virtual int ShowEx (DIALOGHANDLER DlgProc = NULL, void *Param = NULL, const char *lpHelpTopic = NULL);

friend class FarDialogHandler;
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

	virtual int ShowEx (DIALOGHANDLER DlgProc = NULL, void *Param = NULL, const char *lpHelpTopic = NULL);

friend class FarPagedDialogHandler;
friend class FarDialogHandler;
};

#endif //PAGED_DIALOGS

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

	void* GetDlgData ()
		{	return m_Param; }

	void* SetDlgData (void *pData)
   		{	void *pResult = m_Param; m_Param = pData; return pResult; }

   	LONG_PTR Message (int Msg, int Param1, LONG_PTR Param2)
   		{ return m_Owner->m_Info->SendDlgMessage (m_hDlg, Msg, Param1, Param2); }

	LONG_PTR DefDlgProc (int Msg, int Param1, LONG_PTR Param2)
		{ return m_Owner->m_Info->DefDlgProc (m_hDlg, Msg, Param1, Param2); }

	LONG_PTR DlgProc (int Msg, int Param1, LONG_PTR Param2)
		{
			if (m_DlgHandler)
				return m_DlgHandler (this, Msg, Param1, Param2);
			else
				return m_Owner->m_Info->DefDlgProc (m_hDlg, Msg, Param1, Param2);
		}

	bool AddHistory (int ID, const char *History)
		{ return Message (DM_ADDHISTORY, ID, (LONG_PTR)History); }

	int CloseDialog (int ID)
		{ return (int)Message (DM_CLOSE, ID, 0); }

	int EditUnchangedFlag (int ID, int UnchangedFlag)
		{ return (int)Message (DM_EDITUNCHANGEDFLAG, ID, UnchangedFlag); }

	int Enable (int ID, int State)
		{ return (int)Message (DM_ENABLE, ID, State); }

	void EnableRedraw (bool bEnable)
		{ Message (DM_ENABLEREDRAW, bEnable, 0); }

	int GetCheck (int ID)
		{ return (int)Message (DM_GETCHECK, ID, 0); }

	bool GetCursorPos (int ID, COORD **Pos)
		{ return (bool)Message (DM_GETCURSORPOS, ID, (LONG_PTR)*Pos); }

	int GetCursorSize (int ID)
		{ return (int)Message (DM_GETCURSORSIZE, ID, 0); }

	bool GetDlgItem (int ID, FarDialogItem *Item)
		{ return (bool)Message (DM_GETDLGITEM, ID, (LONG_PTR)Item); }

	bool GetDlgRect (SMALL_RECT *Rect)
		{ return (bool)Message (DM_GETDLGRECT, 0, (LONG_PTR)Rect); }

	bool GetDropDownOpened ()
		{ return (bool)Message (DM_GETDROPDOWNOPENED, 0, 0); }

	int GetItemData (int ID)
		{ return (int)Message (DM_GETITEMDATA, ID, 0); }

	bool GetItemPosition (int ID, SMALL_RECT *Pos)
		{ return (bool)Message (DM_GETITEMPOSITION, ID, (LONG_PTR)Pos); }

	int GetFocus ()
		{ return (int)Message (DM_GETFOCUS, 0, 0); }

	int GetText (int ID, FarDialogItemData **Data)
		{ return (int)Message (DM_GETTEXT, ID, (LONG_PTR)*Data); }

	int GetTextPtr (int ID, char *Buffer)
		{ return (int)Message (DM_GETTEXTPTR, ID, (LONG_PTR)Buffer); }

	int GetTextLength (int ID)
		{ return (int)Message (DM_GETTEXTLENGTH, ID, 0); }

	void SendKey (int Count, PDWORD Keys)
		{ Message (DM_KEY, Count, (LONG_PTR)Keys); }

	bool ListAdd (int ID, FarList *List)
		{ return (bool)Message (DM_LISTADD, ID, (LONG_PTR)List); }

	int ListAddStr (int ID, const char *Str)
		{ return (int)Message (DM_LISTADDSTR, ID, (LONG_PTR)Str); }

    bool ListDelete (int ID, FarListDelete *Delete)
    	{ return (int)Message (DM_LISTDELETE, ID, (LONG_PTR)Delete); }

	int ListFindString    (int ID, FarListFind *Find)
		{ return (int)Message (DM_LISTFINDSTRING, ID, (LONG_PTR)Find); }

	int ListGetCurrentPos (int ID, FarListPos *Pos)
		{ return (int)Message (DM_LISTGETCURPOS, ID, (LONG_PTR)Pos); }

	void* ListGetData (int ID, int Index)
		{ return (PVOID)Message (DM_LISTGETDATA, ID, (LONG_PTR)Index); }

	bool ListGetItem (int ID, FarListGetItem *Item)
		{ return (bool)Message (DM_LISTGETITEM, ID, (LONG_PTR)Item); }

	bool ListGetTitles (int ID, FarListTitles **Titles)
		{ return (bool)Message (DM_LISTGETTITLES, ID, (LONG_PTR)*Titles); }

	bool ListInfo (int ID, FarListInfo *pInfo)
		{ return (bool)Message (DM_LISTINFO, ID, (LONG_PTR)pInfo); }

	int ListInsert (int ID, FarListInsert *Insert)
		{ return (int)Message (DM_LISTINSERT, ID, (LONG_PTR)Insert); }

	int ListSet (int ID, FarList *List)
		{ return (int)Message (DM_LISTSET, ID, (LONG_PTR)List); }

	int ListSetCurrentPos (int ID, FarListPos *Pos)
		{ return (int)Message (DM_LISTSETCURPOS, ID, (LONG_PTR)Pos); }

	int ListSetData (int ID, FarListItemData *Data)
		{ return (int)Message (DM_LISTSETDATA, ID, (LONG_PTR)Data); }

	int ListSetDataEx (int ID, int Index, void *pData, dword dwDataSize)
		{	FarListItemData Data = {Index, dwDataSize, pData, 0};
			return (int)Message (DM_LISTSETDATA, ID, (LONG_PTR)&Data); }

	bool ListSetMouseReaction (int ID, int bReaction)
		{ return (bool)Message (DM_LISTSETMOUSEREACTION, ID, bReaction); }

	bool ListSetTitles (int ID, FarListTitles *Titles)
		{ return (bool)Message (DM_LISTSETTITLES, ID, (LONG_PTR)Titles); }

	bool ListSort (int ID, bool bReverse)
		{ return (bool)Message (DM_LISTSORT, ID, bReverse); }

	bool ListUpdate (int ID, FarListUpdate *Update)
		{ return (bool)Message (DM_LISTUPDATE, ID, (LONG_PTR)Update); }

	COORD MoveDialog (bool bAbsolute, COORD *NewPos)
		{ return *(COORD*)Message (DM_MOVEDIALOG, bAbsolute, (LONG_PTR)NewPos); }

	void RedrawDialog ()
		{ Message (DM_REDRAW, 0, 0); }

	COORD ResizeDialog (COORD *Size)
		{ return *(COORD*)Message (DM_RESIZEDIALOG, 0, (LONG_PTR)Size); }

	bool Set3State (int ID, bool b3State)
		{ return (bool)Message (DM_SET3STATE, ID, b3State); }

	int SetCheck (int ID, int State)
		{ return (int)Message (DM_SETCHECK, ID, State); }

	bool SetCursorPos (int ID, COORD *Pos)
		{ return (bool)Message (DM_SETCURSORPOS, ID, (LONG_PTR)Pos); }

	int SetCursorSize (int ID, int Cursor)
		{ return (int)Message (DM_SETCURSORSIZE, ID, Cursor); }

	bool SetDlgItem (int ID, FarDialogItem *Item)
		{ return (bool)Message (DM_SETDLGITEM, ID, (LONG_PTR)Item); }

	bool SetDropDownOpened (int ID, bool bOpen)
		{ return (bool)Message (DM_SETDROPDOWNOPENED, ID, bOpen); }

	bool SetFocus (int ID)
		{ return (bool)Message (DM_SETFOCUS, ID, 0); }

	bool SetHistory (int ID, const char *History)
		{ return (bool)Message (DM_SETHISTORY, ID, (LONG_PTR)History); }

	int SetItemData (int ID, int Data)
		{ return (int)Message (DM_SETITEMDATA, ID, Data); }

	bool SetItemPosition (int ID, SMALL_RECT *Pos)
		{ return (bool)Message (DM_SETITEMPOSITION, ID, (LONG_PTR)Pos); }

	int SetMaxTextLength (int ID, int Length)
		{ return (int)Message (DM_SETMAXTEXTLENGTH, ID, Length); }

	int SetMouseEventNotify  (int State)
		{ return (int)Message (DM_SETMOUSEEVENTNOTIFY, State, 0); }

	int SetText (int ID, FarDialogItemData *Data)
		{ return (int)Message (DM_SETTEXT, ID, (LONG_PTR)Data); }

	int SetTextPtr (int ID, const char *Text)
		{ return (int)Message (DM_SETTEXTPTR, ID, (LONG_PTR)Text); }

	int SetTextLength (int ID, int Length)
		{ return (int)Message (DM_SETTEXTLENGTH, ID, Length); }

	void ShowDialog (bool bShow)
		{ Message (DM_SHOWDIALOG, bShow, 0); }

	int ShowItem (int ID, int State)
		{ return (int)Message (DM_SHOWITEM, ID, State); }
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
