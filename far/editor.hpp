#ifndef EDITOR_HPP_79DE09D5_8F9C_467E_A3BF_8E1BB34E4BD3
#define EDITOR_HPP_79DE09D5_8F9C_467E_A3BF_8E1BB34E4BD3
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
#include "mix.hpp"

class FileEditor;
class KeyBar;
class Edit;

class Editor: public SimpleScreenObject
{
public:
	Editor(window_ptr Owner, bool DialogUsed = false);
	virtual ~Editor() override;

	virtual int ProcessKey(const Manager::Key& Key) override;
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual long long VMProcess(int OpCode, void *vParam = nullptr, long long iParam = 0) override;

	void SetCacheParams(EditorPosCache &pc, bool count_bom = false);
	void GetCacheParams(EditorPosCache &pc) const;
	bool TryCodePage(uintptr_t codepage, int &X, int &Y);
	bool SetCodePage(uintptr_t codepage, bool *BOM=nullptr, bool ShowMe=true); //BUGBUG
	uintptr_t GetCodePage() const; //BUGBUG
	void KeepInitParameters() const;
	void SetStartPos(int LineNum, int CharNum);
	bool IsFileModified() const;
	bool IsFileChanged() const;
	long long GetCurPos(bool file_pos = false, bool add_bom = false) const;
	int EditorControl(int Command, intptr_t Param1, void *Param2);
	void SetHostFileEditor(FileEditor *Editor) { HostFileEditor = Editor; }
	void SetOptions(const Options::EditorOptions& Options);
	void SetTabSize(int NewSize);
	size_t GetTabSize() const { return EdOpt.TabSize; }
	void SetConvertTabs(int NewMode);
	EXPAND_TABS GetConvertTabs() const { return static_cast<EXPAND_TABS>(static_cast<int>(EdOpt.ExpandTabs)); }
	void SetDelRemovesBlocks(bool NewMode);
	bool GetDelRemovesBlocks() const { return EdOpt.DelRemovesBlocks; }
	void SetPersistentBlocks(bool NewMode);
	bool GetPersistentBlocks() const { return EdOpt.PersistentBlocks; }
	void SetAutoIndent(bool NewMode) { EdOpt.AutoIndent = NewMode; }
	bool GetAutoIndent() const { return EdOpt.AutoIndent; }
	void SetAutoDetectCodePage(bool NewMode) { EdOpt.AutoDetectCodePage = NewMode; }
	bool GetAutoDetectCodePage() const { return EdOpt.AutoDetectCodePage; }
	void SetCursorBeyondEOL(bool NewMode);
	bool GetCursorBeyondEOL() const { return EdOpt.CursorBeyondEOL; }
	void SetBSLikeDel(bool NewMode) { EdOpt.BSLikeDel = NewMode; }
	bool GetBSLikeDel() const { return EdOpt.BSLikeDel; }
	void SetCharCodeBase(int NewMode) { EdOpt.CharCodeBase = NewMode % 3; }
	int GetCharCodeBase() const { return EdOpt.CharCodeBase; }
	void SetReadOnlyLock(int NewMode) { EdOpt.ReadOnlyLock = NewMode & 3; }
	int GetReadOnlyLock() const { return EdOpt.ReadOnlyLock; }
	void SetShowScrollBar(bool NewMode) { EdOpt.ShowScrollBar = NewMode; }
	void SetSearchCursorAtEnd(bool NewMode) { EdOpt.SearchCursorAtEnd = NewMode; }
	void SetWordDiv(const wchar_t *WordDiv) { EdOpt.strWordDiv = WordDiv; }
	const string& GetWordDiv() const { return EdOpt.strWordDiv; }
	void SetAllowEmptySpaceAfterEof(bool NewMode) { EdOpt.AllowEmptySpaceAfterEof = NewMode; }
	bool GetAllowEmptySpaceAfterEof() const { return EdOpt.AllowEmptySpaceAfterEof; }
	void SetSearchSelFound(bool NewMode) { EdOpt.SearchSelFound = NewMode; }
	bool GetSearchSelFound() const { return EdOpt.SearchSelFound; }
	void SetSearchRegexp(bool NewMode) { EdOpt.SearchRegexp = NewMode; }
	bool GetSearchRegexp() const { return EdOpt.SearchRegexp; }
	void SetShowWhiteSpace(int NewMode);
	void GetSavePosMode(int &SavePos, int &SaveShortPos) const;
	// передавайте в качестве значения параметра "-1" для параметра,
	// который не нужно менять
	void SetSavePosMode(int SavePos, int SaveShortPos);
	void GetRowCol(const string& argv, int& row, int& col);
	int GetLineCurPos() const;
	void BeginVBlockMarking();
	void ProcessVBlockMarking();
	void AdjustVBlock(int PrevX);
	void Xlat();
	void FreeAllocatedData();
	void SetDialogParent(DWORD Sets);
	void SetReadOnly(bool NewReadOnly) { m_Flags.Change(FEDITOR_LOCKMODE, NewReadOnly); }
	bool GetReadOnly() const { return m_Flags.Check(FEDITOR_LOCKMODE); }
	void SetOvertypeMode(int Mode);
	bool GetOvertypeMode() const;
	void SetEditBeyondEnd(int Mode);
	void SetClearFlag(bool Flag);
	bool GetClearFlag() const;
	int GetCurCol() const;
	int GetCurRow() const { return static_cast<int>(m_it_CurLine.Number()); }
	void SetCurPos(int NewCol, int NewRow = -1);
	void DrawScrollbar();
	bool EditorControlLocked() const { return EditorControlLock != 0; }
	const FarColor& GetNormalColor() const { return Color; }
	const FarColor& GetSelectedColor() const { return SelColor; }
	int GetMacroSelectionStart() const { return MacroSelectionStart; }
	void SetMacroSelectionStart(int Value) { MacroSelectionStart = Value; }
	int GetLineCursorPos() const { return CursorPos; }
	void SetLineCursorPos(int Value) { CursorPos = Value; }
	bool IsLastLine(const Edit* line) const;
	void AutoDeleteColors();
	int GetId() const { return EditorID; }

	static void PR_EditorShowMsg();
	static void SetReplaceMode(bool Mode);
	static const wchar_t* GetDefaultEOL();

	struct EditorUndoData;

private:
	friend class DlgEdit;
	friend class FileEditor;
	friend class undo_block;

	using editor_container = std::list<Edit>;
	using iterator = editor_container::iterator;
	using const_iterator = editor_container::const_iterator;

	struct InternalEditorSessionBookMark;
	using bookmark_list = std::list<InternalEditorSessionBookMark>;

	struct InternalEditorBookmark;

	template<class T, class ConstT = T>
	class numbered_iterator_t: public T
	{
	public:
		TRIVIALLY_COPYABLE(numbered_iterator_t);
		TRIVIALLY_MOVABLE(numbered_iterator_t);

		numbered_iterator_t(const T& Iterator, size_t Number):
			T(Iterator),
			m_Number(Number)
		{
		}

		// BUGBUG, size_t
		int Number() const { return static_cast<int>(m_Number); }
		void IncrementNumber() { ++m_Number; }
		void DecrementNumber() { --m_Number; }

		numbered_iterator_t& operator++() { ++base(); ++m_Number; return *this; }
		numbered_iterator_t& operator--() { --base(); --m_Number; return *this; }

		bool operator==(const numbered_iterator_t& rhs) const { return base() == rhs.base(); }
		bool operator==(const T& rhs) const { return rhs == *this; }
		template<class Y>
		bool operator !=(const Y& rhs) const { return !(*this == rhs); }

		T& base() { return *this; }
		const std::conditional_t<std::is_base_of<ConstT, T>::value, ConstT, T>& base() const { return *this; }
		std::conditional_t<std::is_base_of<ConstT, T>::value, const ConstT&, ConstT> cbase() const { return *this; }

	private:
		// Intentionally not implemented, use prefix forms.
		numbered_iterator_t operator++(int) = delete;
		numbered_iterator_t operator--(int) = delete;

		size_t m_Number;
	};

	using numbered_iterator = numbered_iterator_t<iterator, const_iterator>;
	using numbered_const_iterator = numbered_iterator_t<const_iterator>;

	virtual void DisplayObject() override;
	void ShowEditor();
	numbered_iterator DeleteString(numbered_iterator DelPtr, bool DeleteLast);
	void InsertString();
	void Up();
	void Down();
	void ScrollDown();
	void ScrollUp();
	BOOL Search(int Next);
	void GoToLine(int Line);
	void GoToLineAndShow(int Line);
	void GoToPosition();
	void TextChanged(bool State);
	static int CalcDistance(const numbered_iterator& From, const numbered_iterator& To);
	void PasteFromClipboard();
	void Paste(const string& Data);
	void ProcessChar(wchar_t Char);
	void Copy(int Append);
	void DeleteBlock();
	void UnmarkBlock();
	void UnmarkEmptyBlock();
	void UnmarkMacroBlock();
	void AddUndoData(int Type) { return AddUndoData(Type, {}, nullptr, 0, 0); }
	void AddUndoData(int Type, const string& Str, const wchar_t *Eol, int StrNum, int StrPos);
	void Undo(int redo);
	void SelectAll();
	void BlockLeft();
	void BlockRight();
	void DeleteVBlock();
	void VCopy(int Append);
	void VPaste(const string& Data);
	void VBlockShift(int Left);
	numbered_iterator GetStringByNumber(int DestLine);
	// Set the numbered bookmark (CtrlShift-0..9)
	bool SetBookmark(int Pos);
	// Restore the numbered bookmark (LeftCtrl-0..9)
	bool GotoBookmark(int Pos);
	// Remove all session bookmarks. SessionPos will be invalid
	void ClearSessionBookmarks();
	// Remove a particular session bookmark. Adjusts SessionPos if deleting the current bookmark
	bool DeleteSessionBookmark(bookmark_list::iterator sb_delete);
	bool MoveSessionBookmarkToUndoList(bookmark_list::iterator sb_delete);
	// Restore the current session bookmark
	bool RestoreSessionBookmark();
	// Add current cursor pos to session bookmarks, after the current bookmark, and adjust SessionPos to it
	void AddSessionBookmark(bool NewPos = true);
	// Insert each saved session bookmark on its original position in the list of session bookmarks, if it still exists, otherwise do nothing.
	// hashed to better differentiate among session bookmarks
	void MoveSavedSessionBookmarksBack(bookmark_list& SavedList);
	//Return the session bookmark of index iIdx (and do not change SessionPos).
	// for iIdx==-1 the current bookmark is returned
	bookmark_list::iterator PointerToSessionBookmark(int iIdx);
	bool BackSessionBookmark();
	// Restore the previous session bookmark, if there is any, the current one otherwise
	bool PrevSessionBookmark();
	// Restore the next session bookmark, if there is any, the current one otherwise
	bool NextSessionBookmark();
	// Restore the last session bookmark
	bool LastSessionBookmark();
	// Restore the session bookmark at the specified index;
	// does nothing if there is no such index
	bool GotoSessionBookmark(int iIdx);
	// Append new session bookmark as the last session bookmark, and move SessionPos to it
	void PushSessionBookMark();
	// Restore the last session bookmark and remove it
	bool PopSessionBookMark();
	// Return index of the current session bookmark, -1 if there is none
	int CurrentSessionBookmarkIdx();
	// Get session bookmark at specified index into Param
	bool GetSessionBookmark(int iIdx, InternalEditorBookmark *Param);
	// Get all session bookmarks into the Param array
	size_t GetSessionBookmarks(EditorBookmarks *Param);
	size_t GetSessionBookmarksForPlugin(EditorBookmarks *Param);
	void UpdateCurrentSessionBookmark();
	int BlockStart2NumLine(int *Pos);
	int BlockEnd2NumLine(int *Pos);
	bool CheckLine(const iterator& line);
	string Block2Text();
	string VBlock2Text();
	void Change(EDITOR_CHANGETYPE Type,int StrNum);
	DWORD EditSetCodePage(const iterator& edit, uintptr_t codepage, bool check_only);
	numbered_iterator InsertString(const wchar_t *Str, int Length, const numbered_iterator& Where);
	numbered_iterator PushString(const wchar_t* Str, size_t Size) { return InsertString(Str, static_cast<int>(Size), EndIterator()); }
	void TurnOffMarkingBlock();
	void SwapState(Editor& swap_state);
	bool ProcessKeyInternal(const Manager::Key& Key, bool& Refresh);

	template<class F>
	void UpdateIteratorAndKeepPos(numbered_iterator& Iter, const F& Func);

	numbered_iterator EndIterator();
	numbered_const_iterator EndIterator() const;

	numbered_iterator FirstLine();
	numbered_const_iterator FirstLine() const;

	numbered_iterator LastLine();
	numbered_const_iterator LastLine() const;

	bool IsLastLine(const iterator& Line) const;

	static bool InitSessionBookmarksForPlugin(EditorBookmarks *Param, size_t Count, size_t& Size);
	static void EditorShowMsg(const string& Title, const string& Msg, const string& Name, int Percent);

	bool IsAnySelection() const { assert(Lines.end() == m_it_AnyBlockStart || m_BlockType != BTYPE_NONE); return Lines.end() != m_it_AnyBlockStart; }
	bool IsStreamSelection() const { return IsAnySelection() && m_BlockType == BTYPE_STREAM; }
	bool IsVerticalSelection() const { return IsAnySelection() && m_BlockType == BTYPE_COLUMN; }

	void Unselect() { m_it_AnyBlockStart = EndIterator(); m_BlockType = BTYPE_NONE; TurnOffMarkingBlock(); }

	void BeginStreamMarking(const numbered_iterator& Where);
	void BeginVBlockMarking(const numbered_iterator& Where);

	// Младший байт (маска 0xFF) юзается классом ScreenObject!!!
	enum editor_flags
	{
		FEDITOR_MODIFIED = bit(8),
		FEDITOR_MARKINGBLOCK = bit(9),
		FEDITOR_MARKINGVBLOCK = bit(10),
		FEDITOR_WASCHANGED =  bit(11),
		FEDITOR_OVERTYPE = bit(12),
		FEDITOR_NEWUNDO = bit(13),
		FEDITOR_UNDOSAVEPOSLOST = bit(14),
		FEDITOR_DISABLEUNDO = bit(15),   // возможно процесс Undo уже идет?
		FEDITOR_LOCKMODE = bit(16),
		FEDITOR_CURPOSCHANGEDBYPLUGIN = bit(17),   // установлен, если позиция в редакторе была изменена плагином (ECTL_SETPOSITION)
		FEDITOR_INEEREDRAW = bit(18),
		FEDITOR_PROCESSCTRLQ = bit(19),   // нажата Ctrl-Q и идет процесс вставки кода символа
		FEDITOR_DIALOGMEMOEDIT = bit(20),   // Editor используется в диалоге в качестве DI_MEMOEDIT
	};

	editor_container Lines;
	numbered_iterator m_it_TopScreen{ EndIterator() };
	numbered_iterator m_it_CurLine{ EndIterator() };
	numbered_iterator m_it_LastGetLine{ EndIterator() };

	std::unordered_set<GUID, uuid_hash, uuid_equal> ChangeEventSubscribers;
	std::list<EditorUndoData> UndoData;
	std::list<EditorUndoData>::iterator UndoPos{ UndoData.end() };
	std::list<EditorUndoData>::iterator UndoSavePos{ UndoData.end() };
	int UndoSkipLevel{};
	int LastChangeStrPos{};
	int m_LinesCount{};
	/* $ 26.02.2001 IS
	Сюда запомним размер табуляции и в дальнейшем будем использовать его,
	а не Global->Opt->TabSize
	*/
	Options::EditorOptions EdOpt;
	int Pasting{};
	string GlobalEOL;
	// работа с блоками из макросов (MCODE_F_EDITOR_SEL)
	numbered_iterator m_it_MBlockStart{ EndIterator() };
	numbered_iterator m_it_AnyBlockStart{ EndIterator() };
	EDITOR_BLOCK_TYPES m_BlockType{ BTYPE_NONE };
	int MBlockStartX{};
	int VBlockX{};
	int VBlockSizeX{};
	int VBlockSizeY{};
	int MaxRightPos{};
	int XX2{}; //scrollbar
	string strLastSearchStr;
	bool LastSearchCase{}, LastSearchWholeWords{}, LastSearchReverse{}, LastSearchRegexp{}, LastSearchPreserveStyle{};
	uintptr_t m_codepage{ CP_DEFAULT }; //BUGBUG
	int m_StartLine{-1};
	int StartChar{-1};
	//numbered bookmarks (accessible by Ctrl-0..9)
	Bookmarks<editor_bookmark> m_SavePos;

	bookmark_list SessionBookmarks;
	//pointer to the current "session" bookmark (in the list of "session" bookmarks accessible through BM.Goto(n))
	bookmark_list::iterator SessionPos{ SessionBookmarks.end() };

	bool NewSessionPos{};
	int EditorID{};
	FileEditor *HostFileEditor{};
	int EditorControlLock{};
	std::vector<char> decoded;
	FarColor Color;
	FarColor SelColor;
	int MacroSelectionStart{-1};
	int CursorPos{};
	std::unordered_set<Edit*> m_AutoDeletedColors;

	bool fake_editor{};

	numbered_iterator m_FoundLine{ EndIterator() };
	int m_FoundPos{};
	int m_FoundSize{};

	struct EditorPreRedrawItem : public PreRedrawItem
	{
		EditorPreRedrawItem();
		string Title;
		string Msg;
		string Name;
		int Percent;
	};
};

class EditorContainer
{
public:
	virtual ~EditorContainer() = default;
	virtual Editor* GetEditor(void) = 0;
};

#endif // EDITOR_HPP_79DE09D5_8F9C_467E_A3BF_8E1BB34E4BD3
