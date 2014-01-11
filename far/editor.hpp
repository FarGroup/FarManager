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
#include "poscache.hpp"
#include "bitflags.hpp"
#include "config.hpp"
#include "TPreRedrawFunc.hpp"

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

struct InternalEditorBookmark
{
	intptr_t Line;
	intptr_t Cursor;
	intptr_t ScreenLine;
	intptr_t LeftPos;
};

struct EditorUndoData: NonCopyable
{
	int Type;
	int StrPos;
	int StrNum;
	wchar_t EOL[10];
	size_t Length;
	wchar_t_ptr Str;
	InternalEditorSessionBookMark *BM; //treat as uni-directional linked list

private:
	static size_t UndoDataSize;

public:
	static size_t GetUndoDataSize() { return UndoDataSize; }

	EditorUndoData(int Type,const wchar_t *Str,const wchar_t *Eol,int StrNum,int StrPos,size_t Length):
		Type(),
		StrPos(),
		StrNum(),
		EOL(),
		Length(),
		BM()
	{
		SetData(Type, Str, Eol, StrNum, StrPos, Length);
	}

	~EditorUndoData()
	{
		UndoDataSize -= Length;
		DeleteAllSessionBM();
	}

	EditorUndoData(EditorUndoData&& rhs):
		Type(),
		StrPos(),
		StrNum(),
		EOL(),
		Length(),
		BM()
	{
		*this = std::move(rhs);
	}

	MOVE_OPERATOR_BY_SWAP(EditorUndoData);

	void swap(EditorUndoData& rhs)
	{
		std::swap(Type, rhs.Type);
		std::swap(StrPos, rhs.StrPos);
		std::swap(StrNum, rhs.StrNum);
		std::swap(Length, rhs.Length);
		Str.swap(rhs.Str);
		std::swap(EOL, rhs.EOL);
		std::swap(BM, rhs.BM);
	}

	void SetData(int Type,const wchar_t *Str,const wchar_t *Eol,int StrNum,int StrPos,size_t Length)
	{
		this->Type=Type;
		this->StrPos=StrPos;
		this->StrNum=StrNum;
		UndoDataSize -= this->Length;
		UndoDataSize += Length;
		this->Length=Length;
		xwcsncpy(EOL,Eol?Eol:L"",ARRAYSIZE(EOL)-1);

		if (Str)
		{
			this->Str.reset(Length + 1);

			if (this->Str)
				wmemmove(this->Str.get(), Str, Length);
		}
		else
			this->Str.reset();
		DeleteAllSessionBM();
	}

	void AddSessionBM(const InternalEditorSessionBookMark *BM)
	{
		InternalEditorSessionBookMark *BM_new = new InternalEditorSessionBookMark(*BM);
		if (BM_new)
		{
			BM_new->prev = HashBM(BM->prev);
			BM_new->next = this->BM;
			this->BM = BM_new;
		}
	}

	void DeleteAllSessionBM()
	{
		InternalEditorSessionBookMark *BM=this->BM, *BM_next;
		this->BM = NULL;
		while (BM)
		{
			BM_next = BM->next;
			delete BM;
			BM = BM_next;
		}
	}

	static InternalEditorSessionBookMark *HashBM(InternalEditorSessionBookMark *BM)
	{
		if (BM)
		{
			size_t x = reinterpret_cast<size_t>(BM);
			x = x ^ (BM->Line<<16) ^ (BM->Cursor);
			return reinterpret_cast<InternalEditorSessionBookMark *>(x);
		}
		else
			return NULL;
	}
};

STD_SWAP_SPEC(EditorUndoData);

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
		friend class AutoUndoBlock;

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

		std::list<GUID> ChangeEventSubscribers;

		std::list<EditorUndoData> UndoData;
		std::list<EditorUndoData>::iterator UndoPos;
		std::list<EditorUndoData>::iterator UndoSavePos;
		int UndoSkipLevel;

		int LastChangeStrPos;
		int NumLastLine;
		int NumLine;
		/* $ 26.02.2001 IS
		     Сюда запомним размер табуляции и в дальнейшем будем использовать его,
		     а не Global->Opt->TabSize
		*/
		Options::EditorOptions EdOpt;

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
		bool LastSearchCase,LastSearchWholeWords,LastSearchReverse,LastSearchRegexp,LastSearchPreserveStyle;

		uintptr_t m_codepage; //BUGBUG

		int StartLine;
		int StartChar;

		//numbered bookmarks (accessible by Ctrl-0..9)
		Bookmarks<editor_bookmark> SavePos;
		//pointer to the current "session" bookmark (in the list of "session" bookmarks accessible through BM.Goto(n))
		InternalEditorSessionBookMark *SessionPos;
		BOOL NewSessionPos;

		int EditorID;

		FileEditor *HostFileEditor;

		int SortColorLockCount;
		bool SortColorUpdate;
		int EditorControlLock;

		std::vector<char> decoded;

		FarColor Color;
		FarColor SelColor;

		int MacroSelectionStart;
		int CursorPos;

	private:
		virtual void DisplayObject() override;
		void ShowEditor();
		void DeleteString(Edit *DelPtr,int LineNumber,int DeleteLast,int UndoLine);
		void InsertString();
		void Up();
		void Down();
		void ScrollDown();
		void ScrollUp();
		BOOL Search(int Next);

		void GoToLine(int Line);
		void GoToPosition();

		void TextChanged(bool State);

		int  CalcDistance(const Edit *From, const Edit *To,int MaxDist);
		void Paste(const wchar_t *Src=nullptr);
		void Copy(int Append);
		void DeleteBlock();
		void UnmarkBlock();
		void UnmarkEmptyBlock();
		void UnmarkMacroBlock();

		void AddUndoData(int Type) { return AddUndoData(Type, nullptr, nullptr, 0, 0, 0); }
		void AddUndoData(int Type,const wchar_t *Str, const wchar_t *Eol, int StrNum, int StrPos, int Length);
		void AddUndoData(const InternalEditorSessionBookMark *BM);
		void Undo(int redo);
		void SelectAll();
		//void SetStringsTable();
		void BlockLeft();
		void BlockRight();
		void DeleteVBlock();
		void VCopy(int Append);
		void VPaste(const wchar_t *ClipText);
		void VBlockShift(int Left);
		Edit* GetStringByNumber(int DestLine);
		static void EditorShowMsg(const string& Title,const string& Msg, const string& Name,int Percent);

		// Set the numbered bookmark (CtrlShift-0..9)
		int SetBookmark(int Pos);
		// Restore the numbered bookmark (LeftCtrl-0..9)
		int GotoBookmark(int Pos);
		// Remove all session bookmarks. SessionPos will be NULL
		int ClearSessionBookmarks();
		// Remove a particular session bookmark. Adjusts SessionPos if deleting the current bookmark
		int DeleteSessionBookmark(InternalEditorSessionBookMark *sb_delete);
		// Restore the current session bookmark
		int RestoreSessionBookmark();
		// Add current cursor pos to session bookmarks, after the current bookmark, and adjust SessionPos to it
		int AddSessionBookmark(BOOL blNewPos=TRUE);
		// Insert the session bookmark on its original position in the list of session bookmarks, if it still exists, otherwise do nothing.
		// sb_ins->prev is hashed to better differentiate among session bookmarks
		int InsertSessionBookmarkBack(InternalEditorSessionBookMark *sb_ins);
		// Return the first session bookmark (and do not change SessionPos).
		// On return, piCount will be set to the count of session bookmark preceding the current one
		InternalEditorSessionBookMark* PointerToFirstSessionBookmark(int *piCount=nullptr);
		//Return the last session bookmark (and do not change SessionPos).
		// piCount must be set to the count of session bookmarks preceding the current one
		// On return piCount will be set to the total count of session bookmarks
		InternalEditorSessionBookMark* PointerToLastSessionBookmark(int *piCount=nullptr);
		//Return the session bookmark of index iIdx (and do not change SessionPos).
		// for iIdx==-1 the current bookmark is returned
		InternalEditorSessionBookMark* PointerToSessionBookmark(int iIdx);
		int BackSessionBookmark();
		// Restore the previous session bookmark, if there is any, the current one otherwise
		int PrevSessionBookmark();
		// Restore the next session bookmark, if there is any, the current one otherwise
		int NextSessionBookmark();
		// Restore the last session bookmark
		int LastSessionBookmark();
		// Restore the session bookmark at the specified index;
		// does nothing if there is no such index
		int GotoSessionBookmark(int iIdx);
		// Append new session bookmark as the last session bookmark, and move SessionPos to it
		int PushSessionBookMark();
		// Restore the last session bookmark and remove it
		int PopSessionBookMark();
		// Return index of the current session bookmark, -1 if there is none
		int CurrentSessionBookmarkIdx();
		// Get session bookmark at specified index into Param
		int GetSessionBookmark(int iIdx,InternalEditorBookmark *Param);
		// Get all session bookmarks into the Param array
		int GetSessionBookmarks(EditorBookmarks *Param);
		size_t GetSessionBookmarksForPlugin(EditorBookmarks *Param);
		static bool InitSessionBookmarksForPlugin(EditorBookmarks *Param,size_t Count,size_t& Size);

		int BlockStart2NumLine(int *Pos);
		int BlockEnd2NumLine(int *Pos);
		bool CheckLine(const Edit* line);

		string Block2Text(const wchar_t* InitData, size_t size);
		string Block2Text(const string& InitData) { return Block2Text(InitData.data(), InitData.size()); }
		string Block2Text() { return Block2Text(nullptr, 0); }

		string VBlock2Text(const wchar_t* InitData, size_t size);
		string VBlock2Text(const string& InitData) { return VBlock2Text(InitData.data(), InitData.size()); }
		string VBlock2Text() { return VBlock2Text(nullptr, 0); }

		void Change(EDITOR_CHANGETYPE Type,int StrNum);

	protected:
		void TurnOffMarkingBlock();

	public:
		Editor(ScreenObject *pOwner=nullptr,bool DialogUsed=false);
		virtual ~Editor();

	public:

		void SetCacheParams(EditorPosCache &pc, bool count_bom=false);
		void GetCacheParams(EditorPosCache &pc);

		DWORD EditSetCodePage(Edit *edit, uintptr_t codepage, bool check_only);
		bool TryCodePage(uintptr_t codepage, int &X, int &Y);
		bool SetCodePage(uintptr_t codepage);  //BUGBUG
		uintptr_t GetCodePage();  //BUGBUG

		int SetRawData(const wchar_t *SrcBuf,int SizeSrcBuf,int TextFormat); // преобразование из буфера в список
		int GetRawData(wchar_t **DestBuf,int& SizeDestBuf,int TextFormat=0);   // преобразование из списка в буфер

		virtual int ProcessKey(int Key) override;
		virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
		virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0) override;

		void KeepInitParameters();
		void SetStartPos(int LineNum,int CharNum);
		BOOL IsFileModified() const;
		BOOL IsFileChanged() const;
		void SetTitle(const wchar_t *Title);
		long GetCurPos( bool file_pos=false, bool add_bom=false );
		int EditorControl(int Command, intptr_t Param1, void *Param2);
		void SetHostFileEditor(FileEditor *Editor) {HostFileEditor=Editor;}
		static void SetReplaceMode(bool Mode);
		FileEditor *GetHostFileEditor() {return HostFileEditor;}
		void PrepareResizedConsole() {Flags.Set(FEDITOR_ISRESIZEDCONSOLE);}

		void SetOptions(const Options::EditorOptions& Options);
		void SetTabSize(int NewSize);
		int  GetTabSize() const {return EdOpt.TabSize; }

		void SetConvertTabs(int NewMode);
		EXPAND_TABS GetConvertTabs() const {return static_cast<EXPAND_TABS>(static_cast<int>(EdOpt.ExpandTabs)); }

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

		void SetSearchCursorAtEnd(bool NewMode) {EdOpt.SearchCursorAtEnd=NewMode;}

		void SetWordDiv(const wchar_t *WordDiv) { EdOpt.strWordDiv = WordDiv; }
		const string& GetWordDiv() { return EdOpt.strWordDiv; }

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

		void GetRowCol(const string& argv,int& row,int& col);

		int  GetLineCurPos();
		void BeginVBlockMarking();
		void ProcessVBlockMarking();
		void AdjustVBlock(int PrevX);

		void Xlat();

		static void PR_EditorShowMsg();

		struct EditorPreRedrawItem : public PreRedrawItem
		{
			EditorPreRedrawItem() : PreRedrawItem(PR_EditorShowMsg), Percent(0) {}

			string Title;
			string Msg;
			string Name;
			int Percent;
		};

		void FreeAllocatedData(bool FreeUndo=true);

		Edit *CreateString(const wchar_t *lpwszStr, int nLength);
		Edit *InsertString(const wchar_t *lpwszStr, int nLength, Edit *pAfter = nullptr, int AfterLineNumber=-1);

		void SetDialogParent(DWORD Sets);
		void SetReadOnly(bool NewReadOnly) {Flags.Change(FEDITOR_LOCKMODE,NewReadOnly);}
		bool  GetReadOnly() {return Flags.Check(FEDITOR_LOCKMODE);}
		void SetOvertypeMode(int Mode);
		bool  GetOvertypeMode();
		void SetEditBeyondEnd(int Mode);
		void SetClearFlag(int Flag);
		int  GetClearFlag();

		int  GetCurCol();
		int  GetCurRow() {return NumLine;}
		void SetCurPos(int NewCol, int NewRow=-1);
		void SetCursorType(bool Visible, DWORD Size);
		void GetCursorType(bool& Visible, DWORD& Size);
		void DrawScrollbar();

		void SortColorLock();
		void SortColorUnlock();
		bool SortColorLocked();
		bool EditorControlLocked() {return EditorControlLock != 0;}

		const FarColor& GetNormalColor() const { return Color; }
		const FarColor& GetSelectedColor() const { return SelColor; }

		int GetMacroSelectionStart() const {return MacroSelectionStart;}
		void SetMacroSelectionStart(int Value) {MacroSelectionStart = Value;}

		int GetLineCursorPos() const {return CursorPos;}
		void SetLineCursorPos(int Value) {CursorPos = Value;}

};
