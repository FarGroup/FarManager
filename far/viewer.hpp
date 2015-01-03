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
#include "codepage.hpp"

class FileViewer;
class KeyBar;
class Dialog;


class Viewer:public SimpleScreenObject
{
public:
	Viewer(window_ptr Owner, bool bQuickView = false, uintptr_t aCodePage = CP_DEFAULT);
	virtual ~Viewer();

	virtual int ProcessKey(const Manager::Key& Key) override;
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0) override;
	virtual void ShowConsoleTitle() override;

	int OpenFile(const string& Name,int warning);
	void SetViewKeyBar(KeyBar *ViewKeyBar);
	void UpdateViewKeyBar(KeyBar& ViewKeyBar);
	void SetStatusMode(int Mode);
	void EnableHideCursor(int HideCursor);
	bool GetWrapMode() const;
	void SetWrapMode(bool Wrap);
	bool GetWrapType() const;
	void SetWrapType(bool TypeWrap);
	void KeepInitParameters() const;
	void GetFileName(string &strName) const;
	void SetTempViewName(const string& Name, BOOL DeleteFolder);
	void SetTitle(const string& Title);
	string GetTitle() const;
	void SetFilePos(__int64 Pos);
	__int64 GetFilePos() const { return FilePos; }
	__int64 GetViewFilePos() const { return FilePos; }
	__int64 GetViewFileSize() const { return FileSize; }
	void SetPluginData(const wchar_t *PluginData);
	void SetNamesList(NamesList& List);
	int  ViewerControl(int Command, intptr_t Param1, void *Param2);
	void SetHostFileViewer(FileViewer *Viewer) {HostFileViewer=Viewer;}
	void GoTo(int ShowDlg=TRUE,__int64 NewPos=0,UINT64 Flags=0);
	void GetSelectedParam(__int64 &Pos, __int64 &Length, DWORD &Flags) const;
	void SelectText(const __int64 &MatchPos,const __int64 &SearchLength, const DWORD Flags=0x1);
	bool GetShowScrollbar() const { return ViOpt.ShowScrollbar; }
	void SetShowScrollbar(bool newValue) { ViOpt.ShowScrollbar=newValue; }
	int GetHexMode() const { return VM.Hex; }
	uintptr_t GetCodePage() const { return VM.CodePage; }
	NamesList& GetNamesList() { return ViewNamesList; }
	bool isTemporary() const;
	int ProcessHexMode(int newMode, bool isRedraw = true);
	int ProcessWrapMode(int newMode, bool isRedraw = true);
	int ProcessTypeWrapMode(int newMode, bool isRedraw = true);
	void SearchTextTransform(string& to, const wchar_t *from, bool hex2text, intptr_t &pos);
	int GetId(void) const { return ViewerID; }
	void OnDestroy(void);

private:
	struct ViewerString;
	ENUM(SEARCHER_RESULT);

	virtual void DisplayObject() override;
	void ShowPage(int nMode);
	void Up(int n, bool adjust);
	void CacheLine(__int64 start, int length, bool have_eol);
	int CacheFindUp(__int64 start);
	void ShowHex();
	void ShowDump();
	void ShowStatus();
	/* $ 27.04.2001 DJ
		функции для рисования скроллбара, для корректировки ширины в
		зависимости от наличия скроллбара и для корректировки позиции файла
		на границу строки
	*/
	void DrawScrollbar();
	void AdjustWidth();
	void AdjustFilePos();
	void ReadString(ViewerString *pString, int MaxSize, bool update_cache=true);
	__int64 EndOfScreen( int line );
	__int64 BegOfScreen();
	void ChangeViewKeyBar();
	void Search(int Next,int FirstChar);
	struct search_data;
	SEARCHER_RESULT search_hex_forward( search_data* sd );
	SEARCHER_RESULT search_hex_backward( search_data* sd );
	SEARCHER_RESULT search_text_forward( search_data* sd );
	SEARCHER_RESULT search_text_backward( search_data* sd );
	SEARCHER_RESULT search_regex_forward( search_data* sd );
	SEARCHER_RESULT search_regex_backward( search_data* sd );
	int read_line(wchar_t *buf, wchar_t *tbuf, INT64 cpos, int adjust, INT64 &lpos, int &lsize);
	int vread(wchar_t *Buf, int Count, wchar_t *Buf2 = nullptr);
	bool vseek(__int64 Offset, int Whence);
	__int64 vtell();
	bool vgetc(wchar_t *ch);
	bool veof();
	wchar_t vgetc_prev();
	void SetFileSize();
	int GetStrBytesNum(const wchar_t *Str, int Length);
	bool isBinaryFile(uintptr_t cp);
	void SavePosition();
	intptr_t ViewerSearchDlgProc(Dialog* Dlg, intptr_t Msg,intptr_t Param1,void* Param2);
	int getCharSize() const;
	int txt_dump(const unsigned char *line, size_t nr, int width, wchar_t *outstr, wchar_t zch, int tail) const;

	static uintptr_t GetDefaultCodePage();

protected:
	void ReadEvent(void);
	void CloseEvent(void);

private:
	friend class FileViewer;

	Options::ViewerOptions ViOpt;

	bool Signature;

	NamesList ViewNamesList;
	KeyBar *m_ViewKeyBar;

	std::list<ViewerString> Strings;

	string strFileName;
	string strFullFileName;

	api::fs::file ViewFile;
	CachedRead Reader;

	api::FAR_FIND_DATA ViewFindData;

	string strTempViewName;

	BOOL m_DeleteFolder;

	string strLastSearchStr;
	bool LastSearchCase,LastSearchWholeWords,LastSearchReverse,LastSearchHex,LastSearchRegexp;
	int LastSearchDirection;
	__int64 StartSearchPos;

	struct ViewerModeInternal: ::noncopyable
	{
		uintptr_t CodePage;
		int Wrap;
		int WordWrap;
		int Hex;
	}
	VM;

	MultibyteCodepageDecoder MB;

	__int64 FilePos;
	__int64 SecondPos;
	__int64 FileSize;
	__int64 LastSelectPos, LastSelectSize;

	__int64 LeftPos;
	__int64 LastPage;
	__int64 SelectPos,SelectSize;
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

	bool m_bQuickView;

	uintptr_t DefCodePage;

	int update_check_period;
	DWORD last_update_check;

	std::vector<char> vread_buffer;

	__int64  lcache_first;
	__int64  lcache_last;
	std::vector<__int64> lcache_lines;
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
		__int64 nFilePos;
		__int64 nSelStart;
		__int64 nSelEnd;
		int  linesize;
		int  have_eol;
		bool bSelection;
	}
	vString;

	unsigned char vgetc_buffer[32];
	bool vgetc_ready;
	int  vgetc_cb;
	int  vgetc_ib;
	wchar_t vgetc_composite;

	int dump_text_mode;

	std::vector<wchar_t> ReadBuffer;
	F8CP f8cps;
};

class ViewerContainer
{
public:
	virtual Viewer* GetViewer(void)=0;
	virtual Viewer* GetById(int ID)=0;
};
