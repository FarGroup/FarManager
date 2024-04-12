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

// Internal:
#include "scrobj.hpp"
#include "stddlg.hpp"
#include "poscache.hpp"
#include "bitflags.hpp"
#include "config.hpp"
#include "mix.hpp"
#include "eol.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

class FileEditor;
class KeyBar;
class Edit;
struct ColorItem;

class Editor final: public SimpleScreenObject
{
public:
	explicit Editor(window_ptr Owner, bool DialogUsed = false);
	~Editor() override;

	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	long long VMProcess(int OpCode, void *vParam = nullptr, long long iParam = 0) override;

	void SetCacheParams(EditorPosCache &pc, bool count_bom = false);
	void GetCacheParams(EditorPosCache &pc) const;
	bool TryCodePage(uintptr_t CurrentCodepage, uintptr_t NewCodepage, uintptr_t& ErrorCodepage, size_t& ErrorLine, size_t& ErrorPos, wchar_t& ErrorChar); //BUGBUG does not belong here
	bool SetCodePage(uintptr_t CurrentCodepage, uintptr_t NewCodepage, bool *BOM=nullptr, bool ShowMe=true); //BUGBUG does not belong here
	void KeepInitParameters() const;
	void SetStartPos(int LineNum, int CharNum);
	bool IsModified() const;
	long long GetCurPos(bool file_pos = false, bool add_bom = false) const;
	int EditorControl(int Command, intptr_t Param1, void *Param2);
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
	void SetWordDiv(string_view const WordDiv) { EdOpt.strWordDiv = string(WordDiv); }
	const string& GetWordDiv() const { return EdOpt.strWordDiv; }
	void SetAllowEmptySpaceAfterEof(bool NewMode) { EdOpt.AllowEmptySpaceAfterEof = NewMode; }
	bool GetAllowEmptySpaceAfterEof() const { return EdOpt.AllowEmptySpaceAfterEof; }
	void SetSearchSelFound(bool NewMode) { EdOpt.SearchSelFound = NewMode; }
	bool GetSearchSelFound() const { return EdOpt.SearchSelFound; }
	void SetShowWhiteSpace(int NewMode);
	void GetSavePosMode(int &SavePos, int &SaveShortPos) const;
	// передавайте в качестве значения параметра "-1" для параметра,
	// который не нужно менять
	void SetSavePosMode(int SavePos, int SaveShortPos);
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
	int GetCurRow() const { return m_it_CurLine.Number(); }
	void SetCurPos(int NewCol, int NewRow = -1);
	void DrawScrollbar();
	bool EditorControlLocked() const { return EditorControlLock != 0; }
	const FarColor& GetNormalColor() const { return Color; }
	const FarColor& GetSelectedColor() const { return SelColor; }
	int GetMacroSelectionStart() const { return MacroSelectionStart; }
	void SetMacroSelectionStart(int Value) { MacroSelectionStart = Value; }
	int GetLineCursorPos() const { return CursorPos; }
	void SetLineCursorPos(int Value) { CursorPos = Value; }
	bool IsLastLine(const Edit* Line) const;
	void AutoDeleteColors();
	int GetId() const { return EditorID; }

	static eol GetDefaultEOL();

	struct EditorUndoData;

private:
	friend class Edit;
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
		COPYABLE(numbered_iterator_t);
		MOVABLE(numbered_iterator_t);

		numbered_iterator_t(const T& Iterator, size_t Number):
			T(Iterator),
			m_Number(Number)
		{
		}

		// BUGBUG, migrate to uNumber, rename uNumber to Number
		int Number() const { return static_cast<int>(m_Number); }
		size_t uNumber() const { return m_Number; }

		void IncrementNumber() { ++m_Number; }
		void DecrementNumber() { --m_Number; }

		numbered_iterator_t& operator++() { ++base(); ++m_Number; return *this; }
		numbered_iterator_t& operator--() { --base(); --m_Number; return *this; }

		T& base() { return *this; }
		std::conditional_t<std::derived_from<T, ConstT>, ConstT, T> const& base() const { return *this; }
		std::conditional_t<std::derived_from<T, ConstT>, const ConstT&, ConstT> cbase() const { return *this; }

		POSTFIX_OPS()

	private:
		size_t m_Number;
	};

	using numbered_iterator = numbered_iterator_t<iterator, const_iterator>;
	using numbered_const_iterator = numbered_iterator_t<const_iterator>;
	enum class undo_type: char;

	void DisplayObject() override;

	void ShowEditor();
	numbered_iterator DeleteString(numbered_iterator DelPtr, bool DeleteLast);
	void InsertString();
	void Up();
	void Down();
	void ScrollDown();
	void ScrollUp();
	void GoToLine(size_t Line);
	void GoToLineAndShow(size_t Line);
	void GoToPosition();
	void TextChanged(bool State);
	static int CalcDistance(const numbered_iterator& From, const numbered_iterator& To);
	void PasteFromClipboard();
	void Paste(string_view Data);
	void ProcessChar(wchar_t Char);
	void Copy(bool Append);
	void DeleteBlock();
	void UnmarkBlock();
	void UnmarkEmptyBlock();
	void UnmarkMacroBlock();
	void AddUndoData(undo_type Type) { return AddUndoData(Type, {}, eol::none, 0, 0); }
	void AddUndoData(undo_type Type, string_view Str, eol Eol, int StrNum, int StrPos);
	void Undo(int redo);
	void SelectAll();
	void BlockLeft();
	void BlockRight();
	void DeleteVBlock();
	void VCopy(int Append);
	void VPaste(string_view Data);
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
	bool MoveSessionBookmarkToUndoList(bookmark_list::iterator sb_move);
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
	bool SetLineCodePage(Edit& Line, uintptr_t CurrentCodepage, uintptr_t NewCodepage, bool Validate);
	numbered_iterator InsertString(string_view Str, const numbered_iterator& Where);
	numbered_iterator PushString(const string_view Str) { return InsertString(Str, EndIterator()); }
	void TurnOffMarkingBlock();
	void SwapState(Editor& swap_state);
	bool ProcessKeyInternal(const Manager::Key& Key, bool& Refresh);

	enum class SearchReplaceDisposition;
	SearchReplaceDisposition ShowSearchReplaceDialog(bool ReplaceMode);
	void DoSearchReplace(SearchReplaceDisposition Disposition);
	int CalculateSearchStartPosition(bool Continue, bool Backward, bool Regex) const;
	int CalculateSearchNextPositionInTheLine(bool Backward, bool Regex) const;

	void UpdateIteratorAndKeepPos(numbered_iterator& Iter, const auto& Func);

	numbered_iterator EndIterator();
	numbered_const_iterator EndIterator() const;

	numbered_iterator FirstLine();
	numbered_const_iterator FirstLine() const;

	numbered_iterator LastLine();
	numbered_const_iterator LastLine() const;

	bool IsLastLine(const iterator& Line) const;

	static bool InitSessionBookmarksForPlugin(EditorBookmarks *Param, size_t Count, size_t& Size);

	bool IsAnySelection() const { assert(Lines.end() == m_it_AnyBlockStart || m_BlockType != BTYPE_NONE); return Lines.end() != m_it_AnyBlockStart; }
	bool IsStreamSelection() const { return IsAnySelection() && m_BlockType == BTYPE_STREAM; }
	bool IsVerticalSelection() const { return IsAnySelection() && m_BlockType == BTYPE_COLUMN; }

	void Unselect() { m_it_AnyBlockStart = EndIterator(); m_BlockType = BTYPE_NONE; TurnOffMarkingBlock(); }

	void BeginStreamMarking(const numbered_iterator& Where);
	void BeginVBlockMarking(const numbered_iterator& Where);

	uintptr_t GetCodePage() const;

	void AddColor(numbered_iterator const& It, ColorItem const& Item);
	using delete_color_condition = function_ref<bool(ColorItem const&)>;
	void DeleteColor(Edit const* It, delete_color_condition Condition);
	void DeleteColor(numbered_iterator const& It, delete_color_condition Condition);
	bool GetColor(numbered_iterator const& It, ColorItem& Item, size_t Index) const;
	std::multiset<ColorItem> const* GetColors(Edit* It) const;
	// Младший байт (маска 0xFF) юзается классом ScreenObject!!!
	enum editor_flags
	{
		FEDITOR_MODIFIED              = 8_bit,
		FEDITOR_MARKINGBLOCK          = 9_bit,
		FEDITOR_MARKINGVBLOCK         = 10_bit,
		FEDITOR_OVERTYPE              = 12_bit,
		FEDITOR_NEWUNDO               = 13_bit,
		FEDITOR_UNDOSAVEPOSLOST       = 14_bit,
		FEDITOR_DISABLEUNDO           = 15_bit,   // возможно процесс Undo уже идет?
		FEDITOR_LOCKMODE              = 16_bit,
		FEDITOR_CURPOSCHANGEDBYPLUGIN = 17_bit,   // установлен, если позиция в редакторе была изменена плагином (ECTL_SETPOSITION)
		FEDITOR_PROCESSCTRLQ          = 19_bit,   // нажата Ctrl-Q и идет процесс вставки кода символа
		FEDITOR_DIALOGMEMOEDIT        = 20_bit,   // Editor используется в диалоге в качестве DI_MEMOEDIT
	};
	// Editor content state
	editor_container Lines;
	numbered_iterator m_it_TopScreen{ EndIterator() };
	numbered_iterator m_it_CurLine{ EndIterator() };
	numbered_iterator m_it_LastGetLine{ EndIterator() };
	std::list<EditorUndoData> UndoData;
	std::list<EditorUndoData>::iterator UndoPos{ UndoData.end() };
	std::list<EditorUndoData>::iterator UndoSavePos{ UndoData.end() };
	int UndoSkipLevel{};
	int LastChangeStrPos{};
	eol GlobalEOL;
	numbered_iterator m_it_MBlockStart{ EndIterator() };
	numbered_iterator m_it_AnyBlockStart{ EndIterator() };
	EDITOR_BLOCK_TYPES m_BlockType{ BTYPE_NONE };
	// работа с блоками из макросов (MCODE_F_EDITOR_SEL)
	int MBlockStartX{};
	int VBlockX{};
	int VBlockSizeX{};
	int VBlockSizeY{};
	int MacroSelectionStart{ -1 };
	uintptr_t m_codepageBUGBUG;
	int m_StartLine{ -1 };
	int StartChar{ -1 };
	//numbered bookmarks (accessible by Ctrl-0..9)
	Bookmarks<editor_bookmark> m_SavePos;
	bookmark_list SessionBookmarks;
	//pointer to the current "session" bookmark (in the list of "session" bookmarks accessible through BM.Goto(n))
	bookmark_list::iterator SessionPos{ SessionBookmarks.end() };
	bool NewSessionPos{};
	std::vector<char> decoded;
	numbered_iterator m_FoundLine{ EndIterator() };
	int m_FoundPos{ -1 };
	int m_FoundSize{ -1 };
	std::unordered_map<Edit const*, std::multiset<ColorItem>> m_ColorList;
	std::unordered_set<Edit*> m_AutoDeletedColors;
	struct
	{
		std::optional<std::pair<const_iterator, int>> m_LastState;

		int Position{};
	}
	MaxRightPosState;

	bool fake_editor{};
	// Editor content state end
	// No iterators or anything content-related after this point
	// Don't forget to update SwapState() when needed.


	std::unordered_set<UUID> ChangeEventSubscribers;
	Options::EditorOptions EdOpt;
	int Pasting{};
	int XX2{}; //scrollbar

	SearchReplaceDlgParams m_SearchDlgParams;
	bool IsReplaceMode{};
	bool IsReplaceAll{};

	int EditorID{};
	int EditorControlLock{};
	FarColor Color;
	FarColor SelColor;
	int CursorPos{};
	int m_InEERedraw{};
	bool m_GotoHex{};
};

class EditorContainer
{
public:
	virtual ~EditorContainer() = default;
	virtual Editor* GetEditor() = 0;
};

#endif // EDITOR_HPP_79DE09D5_8F9C_467E_A3BF_8E1BB34E4BD3
