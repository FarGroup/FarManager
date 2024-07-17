#ifndef VIEWER_HPP_D8E79984_1BC4_413A_90BA_3CFE88B613B3
#define VIEWER_HPP_D8E79984_1BC4_413A_90BA_3CFE88B613B3
#pragma once

/*
viewer.hpp

Internal viewer
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
#include "namelist.hpp"
#include "poscache.hpp"
#include "config.hpp"
#include "cache.hpp"
#include "encoding.hpp"
#include "codepage_selection.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/monitored.hpp"

// External:

//----------------------------------------------------------------------------

class FileViewer;
class KeyBar;
class Dialog;
enum SEARCHER_RESULT: int;

class Viewer:public SimpleScreenObject
{
public:
	explicit Viewer(window_ptr Owner, bool bQuickView = false, uintptr_t aCodePage = CP_DEFAULT);
	~Viewer() override;

	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	long long VMProcess(int OpCode,void *vParam=nullptr,long long iParam=0) override;

	bool OpenFile(string_view Name, bool Warn);
	void SetViewKeyBar(KeyBar *ViewKeyBar);
	void UpdateViewKeyBar(KeyBar& ViewKeyBar);
	void SetStatusMode(int Mode);
	void EnableHideCursor(int HideCursor);
	bool GetWrapMode() const;
	void SetWrapMode(bool Wrap);
	bool GetWrapType() const;
	void SetWrapType(bool TypeWrap);
	void KeepInitParameters() const;
	const string& GetFileName() const { return strFullFileName; }
	void SetTempViewName(string_view Name, bool DeleteFolder);
	void SetTitle(string_view Title);
	string GetTitle() const;
	void SetFilePos(long long Pos);
	long long GetFilePos() const { return FilePos; }
	long long GetViewFilePos() const { return FilePos; }
	long long GetViewFileSize() const { return FileSize; }
	void SetPluginData(string_view PluginData);
	void SetNamesList(NamesList& List);
	int  ViewerControl(int Command, intptr_t Param1, void *Param2);
	void SetHostFileViewer(FileViewer *Viewer) {HostFileViewer=Viewer;}
	void GoTo(bool ShowDlg = true, long long Offset = 0, unsigned long long Flags = 0);
	void GetSelectedParam(long long &Pos, long long &Length, DWORD &Flags) const;
	void SelectText(const long long &MatchPos,const long long &SearchLength, DWORD Flags=0x1);
	bool GetShowScrollbar() const { return ViOpt.ShowScrollbar; }
	void SetShowScrollbar(bool newValue) { ViOpt.ShowScrollbar=newValue; }
	uintptr_t GetCodePage() const { return m_Codepage; }
	NamesList& GetNamesList() { return ViewNamesList; }
	bool ProcessDisplayMode(VIEWER_MODE_TYPE newMode, bool isRedraw = true);
	int ProcessWrapMode(int newMode, bool isRedraw = true);
	int ProcessTypeWrapMode(int newMode, bool isRedraw = true);
	int GetId() const { return ViewerID; }
	void OnDestroy();

protected:
	void ReadEvent();
	void CloseEvent();

private:
	struct ViewerString;

	void DisplayObject() override;

	bool process_key(const Manager::Key& Key);
	void ShowPage(int nMode);
	void Up(int nlines, bool adjust);
	void CacheLine(long long start, int length, bool have_eol);
	int CacheFindUp(long long start);
	void ShowHex();
	void ShowDump();
	void ShowStatus() const;
	/* $ 27.04.2001 DJ
		функции для рисования скроллбара, для корректировки ширины в
		зависимости от наличия скроллбара и для корректировки позиции файла
		на границу строки
	*/
	void DrawScrollbar();
	void AdjustWidth();
	void AdjustFilePos();
	bool CheckChanged();
	void ReadString(ViewerString *pString, int MaxSize, bool update_cache=true);
	long long EndOfScreen(int line);
	long long BegOfScreen();
	long long XYfilepos(int col, int row);
	void ChangeViewKeyBar();

	enum class SearchDisposition;
	SearchDisposition ShowSearchReplaceDialog();
	void DoSearchReplace(SearchDisposition Disposition);
	struct search_data;
	SEARCHER_RESULT search_hex_forward( search_data* sd );
	SEARCHER_RESULT search_hex_backward( search_data* sd );
	SEARCHER_RESULT search_text_forward( search_data* sd );
	SEARCHER_RESULT search_text_backward( search_data* sd );
	SEARCHER_RESULT search_regex_forward( search_data* sd );
	SEARCHER_RESULT search_regex_backward( search_data* sd );
	int read_line(wchar_t *buf, wchar_t *tbuf, long long cpos, int adjust, long long& lpos, int &lsize);

	int vread(wchar_t *Buf, int Count, wchar_t *Buf2 = nullptr);
	bool vseek(long long Offset, int Whence);
	long long vtell() const;
	bool vgetc(wchar_t *pCh);
	bool veof() const;
	wchar_t vgetc_prev();
	void SetFileSize();
	int GetStrBytesNum(const wchar_t* Str, int Length) const; // BUGBUG not string_view, could be unrelated 🤦
	bool isBinaryFile(uintptr_t cp);
	void SavePosition();
	int getCharSize() const;
	int txt_dump(std::string_view Str, size_t ClientWidth, string& OutStr, wchar_t ZeroChar, int tail) const;

	static uintptr_t GetDefaultCodePage();

	int GetModeDependentCharSize() const;
	int GetModeDependentLineSize() const;

	wchar_t ZeroChar() const;
	int MaxViewLineSize() const { return ViOpt.MaxLineSize; }
	size_t MaxViewLineBufferSize() const { return ViOpt.MaxLineSize + 15; }
	void ChangeHexModeBytesPerLine(int Amount);
	void AdjustHexModeBytesPerLineToViewWidth();

	// Shift applies to the viewport. E.g., negative Shift moves viewport to the left,
	// towards the left content edge or towards the beginning of the file.
	void HorizontalScroll(int Shift);
	void RollContents(long long OffsetInChars);

	friend class FileViewer;

	Options::ViewerOptions ViOpt;

	bool Signature{};

	NamesList ViewNamesList;
	KeyBar *m_ViewKeyBar{};

	std::list<ViewerString> Strings;

	string strFileName;
	string strFullFileName;

	os::fs::file ViewFile;
	CachedRead Reader;

	os::fs::find_data ViewFindData;

	string strTempViewName;

	bool m_DeleteFolder{true};

	SearchReplaceDlgParams m_SearchDlgParams;

	bool LastSearchBackward{}; // Used to adjust StartSearchPos
	long long StartSearchPos{};

	uintptr_t m_DefCodepage;
	uintptr_t m_Codepage;
	monitored<bool> m_Wrap;
	monitored<bool> m_WordWrap;
	monitored<VIEWER_MODE_TYPE> m_DisplayMode;
	bool m_DumpTextMode{};

	MultibyteCodepageDecoder MB;

	long long FilePos{};
	long long SecondPos{};
	long long FileSize{};
	long long LastSelectPos{}, LastSelectSize{-1};

	long long LeftPos{}; // Left viewport edge relative to left content edge; must be non-negative
	bool LastPage{};
	long long SelectPos{}, SelectSize{-1}, ManualSelectPos{-1};
	DWORD SelectFlags{};
	int ShowStatusLine{true};
	int m_HideCursor{true};

	string strTitle;

	string strPluginData;
	int ReadStdin{};
	int InternalKey{};

	Bookmarks<viewer_bookmark> BMSavePos;

	struct ViewerUndoData;
	std::list<ViewerUndoData> UndoData;

	int LastKeyUndo{};
	// используется при расчете ширины при скролбаре
	int ScrollbarAdjustedWidth{};
	int ScrollbarAdjustedRight{};

	int ViewerID;
	bool OpenFailed{};
	bool bVE_READ_Sent{};
	FileViewer *HostFileViewer{};
	bool AdjustSelPosition{};
	bool redraw_selection{};

	bool m_bQuickView;

	std::vector<char> vread_buffer;

	long long  lcache_first{-1};
	long long  lcache_last{-1};
	std::vector<long long> lcache_lines;
	int      lcache_count{};
	int      lcache_base{};
	bool     lcache_ready{};
	int      lcache_wrap{-1};
	int      lcache_wwrap{-1};
	int      lcache_width{-1};

	int      max_backward_size;
	std::vector<int> llengths;

	std::vector<wchar_t> Search_buffer;

	struct ViewerString
	{
		string Data;
		long long nFilePos;
		long long nSelStart;
		long long nSelEnd;
		int  linesize;
		int  eol_length;
		bool bSelection;
	}
	vString{};

	class vgetc_cache
	{
	public:
		char* begin() { return m_Iterator; }
		char* end() { return m_End; }
		const char* begin() const { return m_Iterator; }
		const char* end() const { return m_End; }
		const char* cbegin() const { return m_Iterator; }
		const char* cend() const { return m_End; }

		void clear() { m_End = m_Iterator = m_Buffer; }
		bool ready() const { return m_End != m_Buffer; }
		char top() const { return *m_Iterator; }
		char pop() { return *m_Iterator++; }
		void pop(size_t Count) { m_Iterator += Count; }
		size_t size() const { return m_End - m_Iterator; }
		bool empty() const { return m_Iterator == m_End; }
		size_t free_size() const { return std::end(m_Buffer) - m_End; }
		void compact();

	private:
		char m_Buffer[64]{};

	public: // BUGBUG
		char* m_Iterator{ m_Buffer };
		char* m_End{ m_Buffer };
	}
	VgetcCache;

	wchar_t vgetc_composite{};

	std::vector<wchar_t> ReadBuffer;
	F8CP f8cps{true};
	std::optional<bool> m_GotoHex;
	int m_HexModePrevScrollbarAdjustedWidth{};
	int m_HexModeBytesPerLine{ 16 };
};

class ViewerContainer
{
public:
	virtual ~ViewerContainer() = default;

	virtual Viewer* GetViewer() = 0;
	virtual Viewer* GetById(int ID) = 0;
};

#endif // VIEWER_HPP_D8E79984_1BC4_413A_90BA_3CFE88B613B3
