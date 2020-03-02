#pragma once
#include "FarPluginBase.hpp"

#if defined(_MSC_VER)
	#pragma warning(disable:4800) // force value to bool
#endif

#define AUTO_LENGTH -1
#define CURRENT_ITEM -1

class FarDialog;

typedef LONG_PTR (__stdcall *DIALOGHANDLER) (FarDialog*, int, int, LONG_PTR);

class FarDialog {

protected:

	int m_X1, m_Y1, m_X2, m_Y2;
	int m_nFlags;

	PluginStartupInfo *m_Info;

	const TCHAR *m_lpHelpTopic;
                      
	int m_nFirstButton;
	HANDLE m_hDlg;

	Array<FarDialogItem> m_items;

	DIALOGHANDLER m_pfnDlgHandler;
	void* m_pUserData;

#ifndef UNICODE
	TCHAR* m_pTempBuffer;
#endif

public:

	FarDialog();
	FarDialog(int X1, int Y1, int X2, int Y2);
	virtual ~FarDialog ();

	HANDLE GetDlg()
		{	return m_hDlg; }

	void* GetDlgData()
		{	return m_pUserData; }

	void SetDlg(HANDLE hDlg)
		{	m_hDlg = hDlg; }


	int SetFlags (int Flags, int nCounter = CURRENT_ITEM);

	void SetDialogFlags (int nFlags) 
		{	m_nFlags = nFlags; }; 

	int GetDialogFlags ()
		{	return m_nFlags; }

	int FirstButton () 
		{	return m_nFirstButton; }

	void Focus (int nCounter = CURRENT_ITEM);
	void DefaultButton (int nCounter = CURRENT_ITEM);

	void SetPosition(int X1, int Y1, int X2, int Y2);

	void Control (int Type, int X, int Y, int X2, int Y2, const TCHAR *Data = NULL, int DataLength = AUTO_LENGTH);
	void SingleBox (int X, int Y, int X2, int Y2, const TCHAR *Caption = NULL, int CaptionLength = AUTO_LENGTH);
	void DoubleBox (int X, int Y, int X2, int Y2, const TCHAR *Caption = NULL, int CaptionLength = AUTO_LENGTH); 
	void Edit (int X, int Y, int Length, const TCHAR *Data = NULL, int DataLength = AUTO_LENGTH, const TCHAR *History = NULL);
	void FixEdit (int X, int Y, int Length, const TCHAR *Data = NULL, int DataLength = AUTO_LENGTH, const TCHAR *History = NULL, const TCHAR *Mask =NULL);
	void PswEdit (int X, int Y, int Length, const TCHAR *Data = NULL, int DataLength = AUTO_LENGTH);
	int Button (int X, int Y, const TCHAR *Caption = NULL, int CaptionLength = AUTO_LENGTH);
	void Text(int X, int Y, const TCHAR *Caption = NULL, int CaptionLength = AUTO_LENGTH);
	void Text(int X1, int Y, int X2, const TCHAR *Caption = NULL, int CaptionLength = AUTO_LENGTH);

	void VText (int X, int Y, int Y2, const TCHAR *Caption = NULL, int CaptionLength = AUTO_LENGTH);

	void TextEx (int X, int Y, const TCHAR *lpFormat, ...);
	void CheckBox (int X, int Y, bool Checked = false, const TCHAR *Caption = NULL, int CaptionLength = AUTO_LENGTH);
	void RadioButton (int X, int Y, bool Selected = false, const TCHAR *Caption = NULL, bool First = false, int CaptionLength = AUTO_LENGTH); 
	void ListBox (int X, int Y, int X2, int Y2, FarList *ListItems, int ListPos = 0);
	void ComboBox (int X, int Y, int Length, FarList *ListItems, int ListPos = 0, const TCHAR *Data = NULL, int DataLength = AUTO_LENGTH);
	void Separator (int Y, const TCHAR *lpCaption = NULL) {	Text (-1, Y, lpCaption); SetFlags (DIF_SEPARATOR); }

	int Run(const TCHAR *lpHelpTopic = NULL);
	virtual int Run(DIALOGHANDLER DlgProc, void *Param = NULL, const TCHAR *lpHelpTopic = NULL);

	void Done ();

	int GetResultCheck(int ID)
	{
#ifdef UNICODE
		return GetCheck(ID);	
#else
		return m_items[ID].Selected;
#endif
	}

	const TCHAR* GetResultData(int ID)
	{
#ifdef UNICODE
		return GetConstTextPtr(ID);
#else
		return m_items[ID].Data;
#endif
	}

	FarDialogItem& operator[] (int index) { return m_items[index]; }

//DialogHandler, use only in handler in 1.7x and anywhere in 2.x

   	LONG_PTR Message (int Msg, int Param1, LONG_PTR Param2)
   		{ return m_Info->SendDlgMessage (m_hDlg, Msg, Param1, Param2); }

	LONG_PTR DefDlgProc (int Msg, int Param1, LONG_PTR Param2)
		{ return m_Info->DefDlgProc (m_hDlg, Msg, Param1, Param2); }

	LONG_PTR DlgProc (int Msg, int Param1, LONG_PTR Param2)
		{ 
			if (m_pfnDlgHandler)
				return m_pfnDlgHandler (this, Msg, Param1, Param2);
			else
				return m_Info->DefDlgProc (m_hDlg, Msg, Param1, Param2); 
		} 

	bool AddHistory (int ID, const TCHAR *History)
		{ return (bool)Message (DM_ADDHISTORY, ID, (LONG_PTR)History); }

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

	int GetTextPtr (int ID, TCHAR *Buffer)
		{ return (int)Message (DM_GETTEXTPTR, ID, (LONG_PTR)Buffer); }

	const TCHAR* GetConstTextPtr(int ID)
		{
#ifdef UNICODE				
			return (const TCHAR*)Message (DM_GETCONSTTEXTPTR, ID, 0);
#else
			if ( m_pTempBuffer )
				delete [] m_pTempBuffer;

			int nLength = GetTextLength(ID);

			m_pTempBuffer = new TCHAR[nLength+1];
			GetTextPtr(ID, m_pTempBuffer);

			return (const TCHAR*)m_pTempBuffer;
#endif
		}

	int GetTextLength (int ID)
		{ return (int)Message (DM_GETTEXTLENGTH, ID, 0); }

	void SendKey (int Count, PDWORD Keys)
		{ Message (DM_KEY, Count, (LONG_PTR)Keys); }

	bool ListAdd (int ID, FarList *List)
		{ return (bool)Message (DM_LISTADD, ID, (LONG_PTR)List); }

	int ListAddStr (int ID, const TCHAR *Str)
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

	int ListSetDataEx (int ID, int Index, void *pData, DWORD dwDataSize)
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

	bool SetHistory (int ID, const TCHAR *History)
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

	int SetTextPtr (int ID, const TCHAR *Text)
		{ return (int)Message (DM_SETTEXTPTR, ID, (LONG_PTR)Text); }

	int SetTextLength (int ID, int Length)
		{ return (int)Message (DM_SETTEXTLENGTH, ID, Length); }

	void ShowDialog (bool bShow)
		{ Message (DM_SHOWDIALOG, bShow, 0); }

	int ShowItem (int ID, int State)
		{ return (int)Message (DM_SHOWITEM, ID, State); }

};

