#pragma once

/*
editor.hpp

–едактор
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
class Edit;

class Editor: public SimpleScreenObject
{
public:
	Editor(SimpleScreenObject *pOwner = nullptr, bool DialogUsed = false);
	virtual ~Editor();

	virtual int ProcessKey(const Manager::Key& Key) override;
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual __int64 VMProcess(int OpCode, void *vParam = nullptr, __int64 iParam = 0) override;

	void SetCacheParams(EditorPosCache &pc, bool count_bom = false);
	void GetCacheParams(EditorPosCache &pc);
	bool TryCodePage(uintptr_t codepage, int &X, int &Y);
	bool SetCodePage(uintptr_t codepage, bool *BOM=nullptr); //BUGBUG
	uintptr_t GetCodePage() const; //BUGBUG
	int SetRawData(const wchar_t *SrcBuf, int SizeSrcBuf, int TextFormat); // преобразование из буфера в список
	int GetRawData(wchar_t **DestBuf, int& SizeDestBuf, int TextFormat = 0); // преобразование из списка в буфер
	void KeepInitParameters();
	void SetStartPos(int LineNum, int CharNum);
	bool IsFileModified() const;
	bool IsFileChanged() const;
	void SetTitle(const wchar_t *Title);
	long GetCurPos(bool file_pos = false, bool add_bom = false) const;
	int EditorControl(int Command, intptr_t Param1, void *Param2);
	void SetHostFileEditor(FileEditor *Editor) { HostFileEditor = Editor; }
	void PrepareResizedConsole() { m_Flags.Set(FEDITOR_ISRESIZEDCONSOLE); }
	void SetOptions(const Options::EditorOptions& Options);
	void SetTabSize(int NewSize);
	int GetTabSize() const { return EdOpt.TabSize; }
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
	void SetSearchPickUpWord(bool NewMode) { EdOpt.SearchPickUpWord = NewMode; }
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
	// передавайте в качестве значени€ параметра "-1" дл€ параметра,
	// который не нужно мен€ть
	void SetSavePosMode(int SavePos, int SaveShortPos);
	void GetRowCol(const string& argv, int& row, int& col) const;
	int GetLineCurPos() const;
	void BeginVBlockMarking();
	void ProcessVBlockMarking();
	void AdjustVBlock(int PrevX);
	void Xlat();
	void FreeAllocatedData(bool FreeUndo = true);
	void SetDialogParent(DWORD Sets);
	void SetReadOnly(bool NewReadOnly) { m_Flags.Change(FEDITOR_LOCKMODE, NewReadOnly); }
	bool GetReadOnly() const { return m_Flags.Check(FEDITOR_LOCKMODE); }
	void SetOvertypeMode(int Mode);
	bool GetOvertypeMode() const;
	void SetEditBeyondEnd(int Mode);
	void SetClearFlag(int Flag);
	int GetClearFlag() const;
	int GetCurCol() const;
	int GetCurRow() const { return NumLine; }
	void SetCurPos(int NewCol, int NewRow = -1);
	void SetCursorType(bool Visible, DWORD Size);
	void GetCursorType(bool& Visible, DWORD& Size);
	void DrawScrollbar();
	void SortColorLock();
	void SortColorUnlock();
	bool SortColorLocked() const;
	bool EditorControlLocked() const { return EditorControlLock != 0; }
	const FarColor& GetNormalColor() const { return Color; }
	const FarColor& GetSelectedColor() const { return SelColor; }
	int GetMacroSelectionStart() const { return MacroSelectionStart; }
	void SetMacroSelectionStart(int Value) { MacroSelectionStart = Value; }
	int GetLineCursorPos() const { return CursorPos; }
	void SetLineCursorPos(int Value) { CursorPos = Value; }
	bool IsLastLine(const Edit* line) const;
	void AutoDeleteColors() const;

	static void PR_EditorShowMsg();
	static void SetReplaceMode(bool Mode);

	struct EditorUndoData;

private:
	friend class DlgEdit;
	friend class FileEditor;
	friend class AutoUndoBlock;

	typedef std::list<Edit> editor_container;
	typedef editor_container::iterator iterator;

	struct InternalEditorSessionBookMark;
	typedef std::list<InternalEditorSessionBookMark> bookmark_list;

	struct InternalEditorBookmark;

	virtual void DisplayObject() override;
	void ShowEditor();
	void DeleteString(iterator DelPtr, int LineNumber, int DeleteLast, int UndoLine);
	void InsertString();
	void Up();
	void Down();
	void ScrollDown();
	void ScrollUp();
	BOOL Search(int Next);
	void GoToLine(int Line);
	void GoToPosition();
	void TextChanged(bool State);
	int CalcDistance(iterator From, iterator To, int MaxDist);
	void Paste(const wchar_t *Src=nullptr);
	void Copy(int Append);
	void DeleteBlock();
	void UnmarkBlock();
	void UnmarkEmptyBlock();
	void UnmarkMacroBlock();
	void AddUndoData(int Type) { return AddUndoData(Type, nullptr, nullptr, 0, 0, 0); }
	void AddUndoData(int Type,const wchar_t *Str, const wchar_t *Eol, int StrNum, int StrPos, int Length);
	void AddBookmarkUndo(bookmark_list::iterator BM);
	void Undo(int redo);
	void SelectAll();
	void BlockLeft();
	void BlockRight();
	void DeleteVBlock();
	void VCopy(int Append);
	void VPaste(const wchar_t *ClipText);
	void VBlockShift(int Left);
	iterator GetStringByNumber(int DestLine);
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
	bool CheckLine(iterator line);
	string Block2Text(const wchar_t* InitData, size_t size);
	string Block2Text(const string& InitData) { return Block2Text(InitData.data(), InitData.size()); }
	string Block2Text() { return Block2Text(nullptr, 0); }
	string VBlock2Text(const wchar_t* InitData, size_t size);
	string VBlock2Text(const string& InitData) { return VBlock2Text(InitData.data(), InitData.size()); }
	string VBlock2Text() { return VBlock2Text(nullptr, 0); }
	void Change(EDITOR_CHANGETYPE Type,int StrNum);
	DWORD EditSetCodePage(iterator edit, uintptr_t codepage, bool check_only);
	iterator InsertString(const wchar_t *lpwszStr, int nLength, iterator pAfter, int AfterLineNumber = -1);
	void TurnOffMarkingBlock();
	void SwapState(Editor& swap_state);

	static bool InitSessionBookmarksForPlugin(EditorBookmarks *Param, size_t Count, size_t& Size);
	static void EditorShowMsg(const string& Title, const string& Msg, const string& Name, int Percent);

	// ћладший байт (маска 0xFF) юзаетс€ классом ScreenObject!!!
	enum editor_flags
	{
		FEDITOR_MODIFIED = BIT(8),
		FEDITOR_MARKINGBLOCK = BIT(9),
		FEDITOR_MARKINGVBLOCK = BIT(10),
		FEDITOR_WASCHANGED =  BIT(11),
		FEDITOR_OVERTYPE = BIT(12),
		FEDITOR_NEWUNDO = BIT(13),
		FEDITOR_UNDOSAVEPOSLOST = BIT(14),
		FEDITOR_DISABLEUNDO = BIT(15),   // возможно процесс Undo уже идет?
		FEDITOR_LOCKMODE = BIT(16),
		FEDITOR_CURPOSCHANGEDBYPLUGIN = BIT(17),   // установлен, если позици€ в редакторе была изменена плагином (ECTL_SETPOSITION)
		FEDITOR_ISRESIZEDCONSOLE = BIT(18),
		FEDITOR_PROCESSCTRLQ = BIT(19),   // нажата Ctrl-Q и идет процесс вставки кода символа
		FEDITOR_DIALOGMEMOEDIT = BIT(20),   // Editor используетс€ в диалоге в качестве DI_MEMOEDIT
	};

	editor_container Lines;
	iterator FirstLine;
	iterator LastLine;
	iterator TopScreen;
	iterator CurLine;
	iterator LastGetLine;
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
	—юда запомним размер табул€ции и в дальнейшем будем использовать его,
	а не Global->Opt->TabSize
	*/
	Options::EditorOptions EdOpt;
	int Pasting;
	string GlobalEOL;
	// работа с блоками из макросов (MCODE_F_EDITOR_SEL)
	iterator MBlockStart;
	int MBlockStartX;
	iterator BlockStart;
	int BlockStartLine;
	iterator VBlockStart;
	int VBlockX;
	int VBlockSizeX;
	int VBlockY;
	int VBlockSizeY;
	int MaxRightPos;
	int XX2; //scrollbar
	string strLastSearchStr;
	bool LastSearchCase, LastSearchWholeWords, LastSearchReverse, LastSearchRegexp, LastSearchPreserveStyle;
	uintptr_t m_codepage; //BUGBUG
	int m_StartLine;
	int StartChar;
	//numbered bookmarks (accessible by Ctrl-0..9)
	Bookmarks<editor_bookmark> m_SavePos;

	bookmark_list SessionBookmarks;
	//pointer to the current "session" bookmark (in the list of "session" bookmarks accessible through BM.Goto(n))
	bookmark_list::iterator SessionPos;

	bool NewSessionPos;
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

	bool fake_editor;

	struct EditorPreRedrawItem : public PreRedrawItem
	{
		EditorPreRedrawItem():
			PreRedrawItem(PR_EditorShowMsg),
			Percent()
		{}

		string Title;
		string Msg;
		string Name;
		int Percent;
	};
};
