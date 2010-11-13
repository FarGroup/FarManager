#ifndef __FAR_STDLIB_MULTILINE_EDITOR
#define __FAR_STDLIB_MULTILINE_EDITOR

/** @defgroup Multiline Multiline editor control.
    @ingroup ExtendedControls
    @{

    Multiline editor control for FAR dialog.
    Looks like FAR menu item editor but have a lot of file editor possibilities.

    Contains next classes:
      - FP_Hilight   - structure to store selection in multiline editor;
      - FP_Multiline - a multiline editor control for FAR dialogs;
*/

/* ------------------------------------------------------------------------ */
/** @defgroup Multiline_Messages FP_Multiline custom messages.
    @{

    Messages sended to DialogProc by @ref FP_Multiline multiline editor:
      - DN_POSCHANGED    Notify that line or position in editor changed.
        -# Param1        0
        -# Param2        PFP_Multiline "this" of sender.

      - DN_TEXTCHHANGED  Notify that `changed` state of editor changed.
        -# Param1        BOOL. New `changed` state.
        -# Param2        PFP_Multiline "this" of sender.
*/
#define DN_POSCHANGED        (DM_USER+1)
#define DN_TEXTCHHANGED      (DM_USER+2)
/**@}*/

/** @brief Hilight element in miltiline control.

    Defines one hilight element in @ref FP_Multiline editor control.
*/
struct FP_Hilight
{
	SRect Rect;     ///< Selection rectangle inside @ref FP_Multiline editor control.
	int   Color;    ///< Selection color
};

/**@brief Multiline editor control.
*/
class FP_Multiline
{
	public:
		FP_Dialog*   dlg;                  ///< Pointer to @ref FP_Dialog dialog control assigned to.

		char       **Lines;                ///< Array of text lines.
		int          Count;                ///< Number of currently used lines.

		int         *Items;                ///< Array of dialog ID`s of editors.
		int          ItemsCount;           ///< Number of ID`s id `Items` array.

		bool         Changed;              ///< Changed flag (Use SetChanged to change it).
		bool         dlgVisible;           ///< true when dialog are drawn.

		int          CurrentPos;           ///< Last X position in last used edit.
		int          CurrentLine;          ///< Number of current line (index in Items).
	public:
		FP_Editor*   Editor;               ///<
		SRect        DlgBounds;            ///<

		int          MaxCount;             ///< Maximum number of allocated char* in `Lines`.
		int          top;                  ///< Number of top visible line.

		FP_Hilight*  Selections;           ///< List of created selections.
		int          SelCount;             ///< Number of created selections.

	private:
		void N_PosChanged(void) { dlg->User(DN_POSCHANGED,0,(LONG_PTR)this); }
		void N_Changed(void)    { SetChanged(true); }

	private:
		void        Init(int maxLines);     ///<Allocate buffers
		void        ClearLines(void);       ///<Clear Lines contents
		bool        DoArrows(int id,int num,long key);
		bool        DoSelection(int id,int num,long key);
		void        DoDrawHilight(const FP_Hilight& sel);
		void        DoDrawHilights(void);
		bool        DoEdit(int id,int num,long key);
		void        ParseText(char *Text,char **Lines,int& Count,int MaxCount);
		void        WordLeft(int& x,int& y);
		void        WordRight(int& x,int& y);
		void        AddLineTail(int nLine);
	public:
		FP_Multiline(int *itms,int maxLines = 100);
		FP_Multiline(int from,int to,int maxLines = 100);
		virtual ~FP_Multiline();

		virtual BOOL DlgProc(FP_Dialog* d, int Msg, int Param1, LONG_PTR Param2, long& rc);
		virtual void SetChanged(bool ch);

		int         AddSelection(void);
		FP_Hilight* GetSelection(int num);
		void        DeleteSelection(int num);
		int         GetSelectionText(int num,char *buff,int sz);          ///<Gets text from selection ('\n' delimited)
		int         GetSelectionText(const SRect& r,char *buff,int sz);   ///<Gets text from selection-like rect ('\n' delimited)
		void        DeleteSelectionText(int num);
		void        PasteSelection(int num);

		void        SetItems(void);                       ///<Set dialog items with Lines contants
		int         TextLength(void);                     ///<Calculate full length of text in all lines
		int         GetText(char *buff,int sz);           ///<Gets full text '\n' delimited

		void        SetText(char *text);                  ///<Fill Lines from `text` + SetItems
		void        SetTextFromEditor(int fromLine);      ///<Fill Lines from editor starting at `fromLine` up to MaxCount in size
		void        SetTextFromCipboard(void);            ///<Fill Lines from clipboard text

		void        TextToCipboard(void);                 ///<Copyes all text from Lines to clipboard (\n delimited)

		bool        GoTo(int y,int x = -1,                ///<Move `top` pointer
		                 bool ForceUpdate = false);

		int         FindEdit(int id);                     ///<Rets Items index by ID of dialog edit (-1 on ID OOB)
		void        GetLine(int num);                     ///<Sets a text to Lines[top+num] from dialog edit indexed by num
		int         CurLine(void);                        ///<Rets Lines index of current line
		void        NewLine(int iNum,int CharPos);        ///<Inserts new line, OPT split current line by two parts
		///< lineNum - `Items` index
		///< CharPos - position to split from (OPT = -1)
		void        DelLine(int iNum);                    ///<Deletes line by `Items` index.
};

/**@}*/
#endif
