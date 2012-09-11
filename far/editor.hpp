#pragma once

/*
editor.hpp

Редактор
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "scrobj.hpp"
#include "plugin.hpp"
#include "poscache.hpp"
#include "bitflags.hpp"
#include "config.hpp"
#include "DList.hpp"

class FileEditor;
class KeyBar;

struct InternalEditorSessionBookMark
{
	DWORD Line;
	DWORD Cursor;
	DWORD ScreenLine;
	DWORD LeftPos;
	InternalEditorSessionBookMark *prev, *next;
};

struct EditorUndoData
{
	int Type;
	int StrPos;
	int StrNum;
	wchar_t EOL[10];
	int Length;
	wchar_t *Str;

	EditorUndoData()
	{
		ClearStruct(*this);
	}
	~EditorUndoData()
	{
		if (Str)
		{
			delete[] Str;
		}
	}
	void SetData(int Type,const wchar_t *Str,const wchar_t *Eol,int StrNum,int StrPos,int Length=-1)
	{
		if (Length == -1 && Str)
			Length=(int)StrLength(Str);

		this->Type=Type;
		this->StrPos=StrPos;
		this->StrNum=StrNum;
		this->Length=Length;
		xwcsncpy(EOL,Eol?Eol:L"",ARRAYSIZE(EOL)-1);

		if (this->Str)
		{
			delete[] this->Str;
		}

		if (Str)
		{
			this->Str=new wchar_t[Length+1];

			if (this->Str)
				wmemmove(this->Str,Str,Length);
		}
		else
			this->Str=nullptr;
	}
};

// Младший байт (маска 0xFF) юзается классом ScreenObject!!!
enum FLAGS_CLASS_EDITOR
{
	FEDITOR_MODIFIED              = 0x00000200,
	// set to 1 by TextChanged, no matter what
	// is value of State.
	FEDITOR_MARKINGBLOCK          = 0x00000800,
	FEDITOR_MARKINGVBLOCK         = 0x00001000,
	FEDITOR_WASCHANGED            = 0x00002000,
	FEDITOR_OVERTYPE              = 0x00004000,
	FEDITOR_NEWUNDO               = 0x00010000,
	FEDITOR_UNDOSAVEPOSLOST       = 0x00020000,
	FEDITOR_DISABLEUNDO           = 0x00040000,   // возможно процесс Undo уже идет?
	FEDITOR_LOCKMODE              = 0x00080000,
	FEDITOR_CURPOSCHANGEDBYPLUGIN = 0x00100000,   // TRUE, если позиция в редакторе была изменена
	// плагином (ECTL_SETPOSITION)
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
		class EditorBlockGuard:NonCopyable
		{
				Editor& ed;
				void (Editor::*method)();
				bool needCheckUnmark;
			public:
				void SetNeedCheckUnmark(bool State) {needCheckUnmark=State;}
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
		Edit *LastGetLine;
		int LastGetLineNumber;

		DList<EditorUndoData> UndoData;
		EditorUndoData *UndoPos;
		EditorUndoData *UndoSavePos;
		int UndoSkipLevel;

		int LastChangeStrPos;
		int NumLastLine;
		int NumLine;
		/* $ 26.02.2001 IS
		     Сюда запомним размер табуляции и в дальнейшем будем использовать его,
		     а не Opt.TabSize
		*/
		EditorOptions EdOpt;

		int Pasting;
		wchar_t GlobalEOL[10];

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

		int MaxRightPos;

		int XX2; //scrollbar

		string strLastSearchStr;
		/* $ 30.07.2000 KM
		   Новая переменная для поиска "Whole words"
		*/
		bool LastSearchCase,LastSearchWholeWords,LastSearchReverse,LastSearchSelFound,LastSearchRegexp;

		UINT m_codepage; //BUGBUG

		int StartLine;
		int StartChar;

		EditorBookmark SavePos;

		InternalEditorSessionBookMark *SessionPos;
		BOOL NewSessionPos;

		int EditorID;

		FileEditor *HostFileEditor;

		int SortColorLockCount;
		bool SortColorUpdate;
		int EditorControlLock;

		char *buffer_line;
		int   buffer_size;

	private:
		virtual void DisplayObject();
		void ShowEditor(void);
		void DeleteString(Edit *DelPtr,int LineNumber,int DeleteLast,int UndoLine);
		void InsertString();
		void Up();
		void Down();
		void ScrollDown();
		void ScrollUp();
		BOOL Search(int Next);

		void GoToLine(int Line);
		void GoToPosition();

		void TextChanged(int State);

		int  CalcDistance(Edit *From, Edit *To,int MaxDist);
		void Paste(const wchar_t *Src=nullptr);
		void Copy(int Append);
		void DeleteBlock();
		void UnmarkBlock();
		void UnmarkEmptyBlock();
		void UnmarkMacroBlock();

		void AddUndoData(int Type,const wchar_t *Str=nullptr,const wchar_t *Eol=nullptr,int StrNum=0,int StrPos=0,int Length=-1);
		void Undo(int redo);
		void SelectAll();
		//void SetStringsTable();
		void BlockLeft();
		void BlockRight();
		void DeleteVBlock();
		void VCopy(int Append);
		void VPaste(wchar_t *ClipText);
		void VBlockShift(int Left);
		Edit* GetStringByNumber(int DestLine);
		static void EditorShowMsg(const wchar_t *Title,const wchar_t *Msg, const wchar_t* Name,int Percent);

		int SetBookmark(int Pos);
		int GotoBookmark(int Pos);

		int ClearSessionBookmarks();
		int DeleteSessionBookmark(InternalEditorSessionBookMark *sb_delete);
		int RestoreSessionBookmark();
		int AddSessionBookmark(BOOL blNewPos=TRUE);
		InternalEditorSessionBookMark* PointerToFirstSessionBookmark(int *piCount=nullptr);
		InternalEditorSessionBookMark* PointerToLastSessionBookmark(int *piCount=nullptr);
		InternalEditorSessionBookMark* PointerToSessionBookmark(int iIdx);
		int BackSessionBookmark();
		int PrevSessionBookmark();
		int NextSessionBookmark();
		int LastSessionBookmark();
		int GotoSessionBookmark(int iIdx);
		int PushSessionBookMark();
		int PopSessionBookMark();
		int CurrentSessionBookmarkIdx();
		int GetSessionBookmark(int iIdx,EditorBookMarks *Param);
		int GetSessionBookmarks(EditorBookMarks *Param);

		int BlockStart2NumLine(int *Pos);
		int BlockEnd2NumLine(int *Pos);
		bool CheckLine(Edit* line);
		wchar_t *Block2Text(wchar_t *ptrInitData);
		wchar_t *VBlock2Text(wchar_t *ptrInitData);
		void Change(EDITOR_CHANGETYPE Type,int StrNum);

	protected:
		void TurnOffMarkingBlock(void);

	public:
		Editor(ScreenObject *pOwner=nullptr,bool DialogUsed=false);
		virtual ~Editor();

	public:

		void SetCacheParams(EditorPosCache &pc, bool count_bom=false);
		void GetCacheParams(EditorPosCache &pc);

      bool TryCodePage(UINT codepage, int &X, int &Y);
		bool SetCodePage(UINT codepage);  //BUGBUG
		UINT GetCodePage();  //BUGBUG

		int SetRawData(const wchar_t *SrcBuf,int SizeSrcBuf,int TextFormat); // преобразование из буфера в список
		int GetRawData(wchar_t **DestBuf,int& SizeDestBuf,int TextFormat=0);   // преобразование из списка в буфер

		virtual int ProcessKey(int Key);
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
		virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0);

		void KeepInitParameters();
		void SetStartPos(int LineNum,int CharNum);
		BOOL IsFileModified() const;
		BOOL IsFileChanged() const;
		void SetTitle(const wchar_t *Title);
		long GetCurPos( bool file_pos=false, bool add_bom=false );
		int EditorControl(int Command,void *Param);
		void SetHostFileEditor(FileEditor *Editor) {HostFileEditor=Editor;};
		static void SetReplaceMode(bool Mode);
		FileEditor *GetHostFileEditor() {return HostFileEditor;};
		void PrepareResizedConsole() {Flags.Set(FEDITOR_ISRESIZEDCONSOLE);}

		void SetTabSize(int NewSize);
		int  GetTabSize() const {return EdOpt.TabSize; }

		void SetConvertTabs(int NewMode);
		int  GetConvertTabs() const {return EdOpt.ExpandTabs; }

		void SetDelRemovesBlocks(bool NewMode);
		bool  GetDelRemovesBlocks() const {return EdOpt.DelRemovesBlocks; }

		void SetPersistentBlocks(bool NewMode);
		bool  GetPersistentBlocks() const {return EdOpt.PersistentBlocks; }

		void SetAutoIndent(bool NewMode) { EdOpt.AutoIndent=NewMode; }
		bool  GetAutoIndent() const {return EdOpt.AutoIndent; }

		void SetAutoDetectCodePage(bool NewMode) { EdOpt.AutoDetectCodePage=NewMode; }
		bool  GetAutoDetectCodePage() const {return EdOpt.AutoDetectCodePage; }

		void SetCursorBeyondEOL(bool NewMode);
		bool  GetCursorBeyondEOL() const {return EdOpt.CursorBeyondEOL; }

		void SetBSLikeDel(bool NewMode) { EdOpt.BSLikeDel=NewMode; }
		bool  GetBSLikeDel() const {return EdOpt.BSLikeDel; }

		void SetCharCodeBase(int NewMode) { EdOpt.CharCodeBase=NewMode%3; }
		int  GetCharCodeBase() const {return EdOpt.CharCodeBase; }

		void SetReadOnlyLock(int NewMode)  { EdOpt.ReadOnlyLock=NewMode&3; }
		int  GetReadOnlyLock() const {return EdOpt.ReadOnlyLock; }

		void SetShowScrollBar(bool NewMode) {EdOpt.ShowScrollBar=NewMode;}

		void SetSearchPickUpWord(bool NewMode) {EdOpt.SearchPickUpWord=NewMode;}

		void SetWordDiv(const wchar_t *WordDiv) { EdOpt.strWordDiv = WordDiv; }
		const wchar_t *GetWordDiv() { return EdOpt.strWordDiv; }

		void SetF7Rules(bool NewMode) { EdOpt.F7Rules = NewMode; }
		bool GetF7Rules() { return EdOpt.F7Rules; }

		void SetAllowEmptySpaceAfterEof(bool NewMode) { EdOpt.AllowEmptySpaceAfterEof = NewMode; }
		bool GetAllowEmptySpaceAfterEof() { return EdOpt.AllowEmptySpaceAfterEof; }

		void SetSearchSelFound(bool NewMode) { EdOpt.SearchSelFound = NewMode; }
		bool GetSearchSelFound() { return EdOpt.SearchSelFound; }

		void SetSearchRegexp(bool NewMode) { EdOpt.SearchRegexp = NewMode; }
		bool GetSearchRegexp() { return EdOpt.SearchRegexp; }

		void SetShowWhiteSpace(int NewMode);

		void GetSavePosMode(int &SavePos, int &SaveShortPos);

		// передавайте в качестве значения параметра "-1" для параметра,
		// который не нужно менять
		void SetSavePosMode(int SavePos, int SaveShortPos);

		void GetRowCol(const wchar_t *argv,int *row,int *col);

		int  GetLineCurPos();
		void BeginVBlockMarking();
		void ProcessVBlockMarking(void);
		void AdjustVBlock(int PrevX);

		void Xlat();
		static void PR_EditorShowMsg();

		void FreeAllocatedData(bool FreeUndo=true);

		Edit *CreateString(const wchar_t *lpwszStr, int nLength);
		Edit *InsertString(const wchar_t *lpwszStr, int nLength, Edit *pAfter = nullptr, int AfterLineNumber=-1);

		void SetDialogParent(DWORD Sets);
		void SetReadOnly(int NewReadOnly) {Flags.Change(FEDITOR_LOCKMODE,NewReadOnly);};
		int  GetReadOnly() {return Flags.Check(FEDITOR_LOCKMODE);};
		void SetOvertypeMode(int Mode);
		int  GetOvertypeMode();
		void SetEditBeyondEnd(int Mode);
		void SetClearFlag(int Flag);
		int  GetClearFlag();

		int  GetCurCol();
		int  GetCurRow() {return NumLine;}
		void SetCurPos(int NewCol, int NewRow=-1);
		void SetCursorType(bool Visible, DWORD Size);
		void GetCursorType(bool& Visible, DWORD& Size);
		void SetObjectColor(PaletteColors Color,PaletteColors SelColor);
		void DrawScrollbar();

		void SortColorLock();
		void SortColorUnlock();
		bool SortColorLocked();
		bool EditorControlLocked() {return EditorControlLock?true:false;}
};
