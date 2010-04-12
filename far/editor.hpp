#ifndef __EDITOR_HPP__
#define __EDITOR_HPP__
/*
editor.hpp

Редактор

*/

#include "scrobj.hpp"
#include "struct.hpp"
#include "plugin.hpp"
#include "farconst.hpp"
#include "bitflags.hpp"
#include "fn.hpp"

class FileEditor;
class KeyBar;

struct InternalEditorBookMark
{
	DWORD Line[BOOKMARK_COUNT];
	DWORD Cursor[BOOKMARK_COUNT];
	DWORD ScreenLine[BOOKMARK_COUNT];
	DWORD LeftPos[BOOKMARK_COUNT];
};

struct InternalEditorStackBookMark
{
	DWORD Line;
	DWORD Cursor;
	DWORD ScreenLine;
	DWORD LeftPos;
	struct InternalEditorStackBookMark *prev, *next;
};

struct EditorCacheParams
{
	DWORD Line;
	DWORD LinePos;
	DWORD ScreenLine;
	DWORD LeftPos;
	DWORD Table;

	InternalEditorBookMark SavePos;
};

struct EditorUndoData
{
	int Type;
	int UndoNext;
	int StrPos;
	int StrNum;
	char EOL[10];
	int Length;
	char *Str;
};

// Младший байт (маска 0xFF) юзается классом ScreenObject!!!
enum FLAGS_CLASS_EDITOR
{
	FEDITOR_MODIFIED              = 0x00000200,
	FEDITOR_JUSTMODIFIED          = 0x00000400,   // 10.08.2000 skv: need to send EE_REDRAW 2.
	// set to 1 by TextChanged, no matter what
	// is value of State.
	FEDITOR_MARKINGBLOCK          = 0x00000800,
	FEDITOR_MARKINGVBLOCK         = 0x00001000,
	FEDITOR_WASCHANGED            = 0x00002000,
	FEDITOR_OVERTYPE              = 0x00004000,
	FEDITOR_UNDOOVERFLOW          = 0x00008000,   // Переполнение в ундо?
	FEDITOR_NEWUNDO               = 0x00010000,
	FEDITOR_DISABLEUNDO           = 0x00040000,   // возможно процесс Undo уже идет?
	FEDITOR_LOCKMODE              = 0x00080000,
	FEDITOR_CURPOSCHANGEDBYPLUGIN = 0x00100000,   // TRUE, если позиция в редакторе была изменена
	// плагином (ECTL_SETPOSITION)
	FEDITOR_TABLECHANGEDBYUSER    = 0x00200000,
	FEDITOR_ISRESIZEDCONSOLE      = 0x00800000,

	FEDITOR_PROCESSCTRLQ          = 0x02000000,   // нажата Ctrl-Q и идет процесс вставки кода символа

	FEDITOR_DIALOGMEMOEDIT        = 0x80000000,   // Editor используется в диалоге в качестве DI_MEMOEDIT
};

class Edit;

class Editor:public ScreenObject
{
		friend class DlgEdit;
		friend class FileEditor;
	private:

		/* $ 04.11.2003 SKV
		  на любом выходе если была нажата кнопка выделения,
		  и она его "сняла" (сделала 0-й ширины), то его надо убрать.
		*/
		struct EditorBlockGuard
		{
			Editor& ed;
			void (Editor::*method)();
			bool needCheckUnmark;
			EditorBlockGuard(Editor& ed,void (Editor::*method)()):ed(ed),method(method),needCheckUnmark(false)
			{
			}
			~EditorBlockGuard()
			{
				if (needCheckUnmark)(ed.*method)();
			}
		};

		Edit *TopList;
		Edit *EndList;
		Edit *TopScreen;
		Edit *CurLine;

		struct EditorUndoData *UndoData;  // $ 03.12.2001 IS: теперь указатель, т.к. размер может меняться
		int UndoDataPos;
		int UndoSavePos;

		int LastChangeStrPos;
		int NumLastLine;
		int NumLine;

		struct EditorOptions EdOpt;

		int Pasting;
		char GlobalEOL[10];

		// работа с блоками из макросов (MCODE_F_EDITOR_SEL)
		Edit *MBlockStart;
		int   MBlockStartX;

		Edit *BlockStart;
		int BlockStartLine;
		Edit *VBlockStart;
		int VBlockX;
		int VBlockSizeX;
		int VBlockY;
		int VBlockSizeY;
		int BlockUndo;

		int MaxRightPos;

		int XX2; //scrollbar

		unsigned char LastSearchStr[SEARCHSTRINGBUFSIZE];
		/* $ 30.07.2000 KM
		   Новая переменная для поиска "Whole words"
		*/
		int LastSearchCase,LastSearchWholeWords,LastSearchReverse;
		/* $ KM */

		struct CharTableSet TableSet;
		int UseDecodeTable;
		int TableNum;
		int AnsiText;

		int StartLine;
		int StartChar;

		struct InternalEditorBookMark SavePos;

		struct InternalEditorStackBookMark *StackPos;
		BOOL NewStackPos;

		int EditorID;

		FileEditor *HostFileEditor;

	private:
		virtual void DisplayObject();
		void ShowEditor(int CurLineOnly);
		void DeleteString(Edit *DelPtr,int DeleteLast,int UndoLine);
		void InsertString();
		void Up();
		void Down();
		void ScrollDown();
		void ScrollUp();
		BOOL Search(int Next);

		void GoToLine(int Line);
		void GoToPosition();

		/* $ 10.08.2000 skv
		  Call this when text changed to set Modified to
		  specified State and JustModified to 1
		*/
		void TextChanged(int State);

		int  CalcDistance(Edit *From,Edit *To,int MaxDist);
		void Paste(char *Src=NULL);
		void Copy(int Append);
		void DeleteBlock();
		void UnmarkBlock();

		// $ 07.03.2002 IS - удалить выделение, если оно пустое (выделено ноль символов в ширину)
		void UnmarkEmptyBlock();
		void UnmarkMacroBlock();

		void AddUndoData(const char *Str,const char *Eol,int StrNum,int StrPos,int Type,int Length=-1);
		void Undo();
		void SelectAll();
		void SetStringsTable();
		void BlockLeft();
		void BlockRight();
		void DeleteVBlock();
		void VCopy(int Append);
		void VPaste(char *ClipText);
		void VBlockShift(int Left);
		Edit * GetStringByNumber(int DestLine);
		static void EditorShowMsg(const char *Title,const char *Msg, const char* Name);

		int SetBookmark(DWORD Pos);
		int GotoBookmark(DWORD Pos);
		int AddStackBookmark();
		int RestoreStackBookmark();
		int PrevStackBookmark();
		int NextStackBookmark();
		int ClearStackBookmarks();
		InternalEditorStackBookMark* PointerToStackBookmark(int iIdx);
		int DeleteStackBookmark(InternalEditorStackBookMark *sb_delete);
		int GetStackBookmark(int iIdx,EditorBookMarks *Param);
		int GetStackBookmarks(EditorBookMarks *Param);

		bool AddString(const char *lpszStr, int nLength);
		Edit *CreateString(const char *lpszStr, int nLength);

		int BlockStart2NumLine(int *Pos);
		int BlockEnd2NumLine(int *Pos);
		bool CheckLine(Edit* line);

	public:
		Editor(ScreenObject *pOwner=NULL,bool DialogUsed=false);
		virtual ~Editor();

	public:
		int ReadData(LPCSTR SrcBuf,int SizeSrcBuf);                    // преобразование из буфера в список
		int SaveData(char **DestBuf,int& SizeDestBuf,int TextFormat);  // преобразование из списка в буфер

		void SetCacheParams(EditorCacheParams *pp);
		bool GetCacheParams(EditorCacheParams *pp);
		void SetTable(unsigned int Table);

		virtual int ProcessKey(int Key);
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
		virtual __int64 VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);

		void KeepInitParameters();
		void SetStartPos(int LineNum,int CharNum);
		int IsFileModified();
		int IsFileChanged();
		void SetTitle(const char *Title);
		long GetCurPos();
		int EditorControl(int Command,void *Param);
		void SetHostFileEditor(FileEditor *Editor) {HostFileEditor=Editor;};
		static void SetReplaceMode(int Mode);
		FileEditor *GetHostFileEditor() {return HostFileEditor;};
		void PrepareResizedConsole() {Flags.Set(FEDITOR_ISRESIZEDCONSOLE);}

		void SetTabSize(int NewSize);
		int  GetTabSize(void) const {return EdOpt.TabSize; }

		void SetConvertTabs(int NewMode);
		int  GetConvertTabs(void) const {return EdOpt.ExpandTabs; }

		void SetDelRemovesBlocks(int NewMode);
		int  GetDelRemovesBlocks(void) const {return EdOpt.DelRemovesBlocks; }

		void SetPersistentBlocks(int NewMode);
		int  GetPersistentBlocks(void) const {return EdOpt.PersistentBlocks; }

		void SetAutoIndent(int NewMode) { EdOpt.AutoIndent=NewMode; }
		int  GetAutoIndent(void) const {return EdOpt.AutoIndent; }

		void SetAutoDetectTable(int NewMode) { EdOpt.AutoDetectTable=NewMode; }
		int  GetAutoDetectTable(void) const {return EdOpt.AutoDetectTable; }

		void SetCursorBeyondEOL(int NewMode);
		int  GetCursorBeyondEOL(void) const {return EdOpt.CursorBeyondEOL; }

		void SetBSLikeDel(int NewMode) { EdOpt.BSLikeDel=NewMode; }
		int  GetBSLikeDel(void) const {return EdOpt.BSLikeDel; }

		void SetReadOnlyLock(int NewMode)  { EdOpt.ReadOnlyLock=NewMode&3; }
		int  GetReadOnlyLock(void) const {return EdOpt.ReadOnlyLock; }

		void SetShowScrollBar(int NewMode) {EdOpt.ShowScrollBar=NewMode;}

		void SetCharCodeBase(int NewMode) { EdOpt.CharCodeBase=NewMode%3; }
		int  GetCharCodeBase(void) const {return EdOpt.CharCodeBase; }

		void SetWordDiv(const char *WordDiv) { xstrncpy(EdOpt.WordDiv,WordDiv,sizeof(EdOpt.WordDiv)-1); }
		const char *GetWordDiv() { return EdOpt.WordDiv; }
		/* $ 29.10.2001 IS
		     Работа с настройками "сохранять позицию файла" и
		     "сохранять закладки" после смены настроек по alt-shift-f9.
		*/
		void GetSavePosMode(int &SavePos, int &SaveShortPos);

		// передавайте в качестве значения параметра "-1" для параметра,
		// который не нужно менять
		void SetSavePosMode(int SavePos, int SaveShortPos);

		void GetRowCol(char *argv,int *row,int *col);

		int  GetLineCurPos();
		void BeginVBlockMarking();
		void AdjustVBlock(int PrevX);

		void Xlat();
		static void PR_EditorShowMsg(void);

		void FreeAllocatedData();
		void SetDialogParent(DWORD Sets);
		void SetReadOnly(int NewReadOnly) {Flags.Change(FEDITOR_LOCKMODE,NewReadOnly);};
		int  GetReadOnly() {return Flags.Check(FEDITOR_LOCKMODE);};
		void SetOvertypeMode(int Mode);
		int  GetOvertypeMode();
		void SetEditBeyondEnd(int Mode);
		void SetClearFlag(int Flag);
		int  GetClearFlag(void);

		int  GetCurCol();
		int  GetCurRow() {return NumLine;};
		void SetCurPos(int NewCol, int NewRow=-1);
		void SetCursorType(int Visible,int Size);
		void GetCursorType(int &Visible,int &Size);
		void SetObjectColor(int Color,int SelColor,int ColorUnChanged);
		void DrawScrollbar();
};

#endif // __EDITOR_HPP__
