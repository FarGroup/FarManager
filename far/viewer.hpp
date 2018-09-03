﻿#ifndef VIEWER_HPP_D8E79984_1BC4_413A_90BA_3CFE88B613B3
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

#include "scrobj.hpp"
#include "namelist.hpp"
#include "poscache.hpp"
#include "config.hpp"
#include "cache.hpp"
#include "encoding.hpp"
#include "codepage_selection.hpp"

#include "platform.fs.hpp"

#include "common/monitored.hpp"

class FileViewer;
class KeyBar;
class Dialog;
class time_check;
enum SEARCHER_RESULT: int;

class Viewer:public SimpleScreenObject
{
public:
	explicit Viewer(window_ptr Owner, bool bQuickView = false, uintptr_t aCodePage = CP_DEFAULT);
	~Viewer() override;

	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	long long VMProcess(int OpCode,void *vParam=nullptr,long long iParam=0) override;

	bool OpenFile(const string& Name, int warning);
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
	void SetTempViewName(const string& Name, bool DeleteFolder);
	void SetTitle(const string& Title);
	string GetTitle() const;
	void SetFilePos(long long Pos);
	long long GetFilePos() const { return FilePos; }
	long long GetViewFilePos() const { return FilePos; }
	long long GetViewFileSize() const { return FileSize; }
	void SetPluginData(const wchar_t *PluginData);
	void SetNamesList(NamesList& List);
	int  ViewerControl(int Command, intptr_t Param1, void *Param2);
	void SetHostFileViewer(FileViewer *Viewer) {HostFileViewer=Viewer;}
	void GoTo(bool ShowDlg = true, long long NewPos = 0, unsigned long long Flags = 0);
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

private:
	struct ViewerString;

	void DisplayObject() override;

	bool process_key(const Manager::Key& Key);
	void ShowPage(int nMode);
	void Up(int n, bool adjust);
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
	void Search(int Next,const Manager::Key* FirstChar);
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
	bool vgetc(wchar_t *ch);
	bool veof() const;
	wchar_t vgetc_prev();
	void SetFileSize();
	int GetStrBytesNum(string_view Str) const;
	bool isBinaryFile(uintptr_t cp);
	void SavePosition();
	intptr_t ViewerSearchDlgProc(Dialog* Dlg, intptr_t Msg,intptr_t Param1,void* Param2);
	int getCharSize() const;
	int txt_dump(const char *line, size_t nr, size_t width, string& outstr, wchar_t zch, int tail) const;

	static uintptr_t GetDefaultCodePage();

	int GetModeDependentCharSize() const;
	int GetModeDependentLineSize() const;

	wchar_t ZeroChar() const;
	size_t MaxViewLineSize() const { return ViOpt.MaxLineSize; }
	size_t MaxViewLineBufferSize() const { return ViOpt.MaxLineSize + 15; }

protected:
	void ReadEvent();
	void CloseEvent();

private:
	friend class FileViewer;

	Options::ViewerOptions ViOpt;

	bool Signature;

	NamesList ViewNamesList;
	KeyBar *m_ViewKeyBar;

	std::list<ViewerString> Strings;

	string strFileName;
	string strFullFileName;

	os::fs::file ViewFile;
	CachedRead Reader;

	os::fs::find_data ViewFindData;

	string strTempViewName;

	bool m_DeleteFolder;

	string strLastSearchStr;
	bool LastSearchCase,LastSearchWholeWords,LastSearchReverse,LastSearchHex,LastSearchRegexp;
	int LastSearchDirection;
	long long StartSearchPos;

	uintptr_t m_DefCodepage;
	uintptr_t m_Codepage;
	monitored<bool> m_Wrap;
	monitored<bool> m_WordWrap;
	monitored<VIEWER_MODE_TYPE> m_DisplayMode;
	bool m_DumpTextMode;

	MultibyteCodepageDecoder MB;

	long long FilePos;
	long long SecondPos;
	long long FileSize;
	long long LastSelectPos, LastSelectSize;

	long long LeftPos;
	bool LastPage;
	long long SelectPos,SelectSize, ManualSelectPos;
	DWORD SelectFlags;
	int ShowStatusLine;
	int m_HideCursor;

	string strTitle;

	string strPluginData;
	int ReadStdin;
	int InternalKey;

	Bookmarks<viewer_bookmark> BMSavePos;

	struct ViewerUndoData;
	std::list<ViewerUndoData> UndoData;

	int LastKeyUndo;
	int Width,XX2;  // , используется при расчете ширины при скролбаре
	int ViewerID;
	bool OpenFailed;
	bool bVE_READ_Sent;
	FileViewer *HostFileViewer;
	bool AdjustSelPosition;
	bool redraw_selection;

	bool m_bQuickView;

	std::unique_ptr<time_check> m_TimeCheck;
	std::unique_ptr<time_check> m_IdleCheck;

	std::vector<char> vread_buffer;

	long long  lcache_first;
	long long  lcache_last;
	std::vector<long long> lcache_lines;
	int      lcache_count;
	int      lcache_base;
	bool     lcache_ready;
	int      lcache_wrap;
	int      lcache_wwrap;
	int      lcache_width;

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
	vString;

	char vgetc_buffer[64];
	bool vgetc_ready;
	int  vgetc_cb;
	int  vgetc_ib;
	wchar_t vgetc_composite;

	std::vector<wchar_t> ReadBuffer;
	F8CP f8cps;
	std::pair<bool, bool> m_GotoHex;
};

class ViewerContainer
{
public:
	virtual ~ViewerContainer() = default;

	virtual Viewer* GetViewer() = 0;
	virtual Viewer* GetById(int ID) = 0;
};

#endif // VIEWER_HPP_D8E79984_1BC4_413A_90BA_3CFE88B613B3
