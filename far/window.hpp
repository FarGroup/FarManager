#pragma once

/*
window.hpp

Немодальное окно (базовый класс для FilePanels, FileEditor, FileViewer)
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
#include "macro.hpp"

class KeyBar;

enum window_type
{
	windowtype_desktop,
	windowtype_panels,
	windowtype_viewer,
	windowtype_editor,
	windowtype_dialog,
	windowtype_menu,
	windowtype_help,
	windowtype_combobox,
	windowtype_findfolder,
	windowtype_grabber,
	windowtype_hmenu,
	windowtype_search,
};

class window: public ScreenObjectWithShadow, public std::enable_shared_from_this<window>
{
public:
	virtual ~window();

	virtual int GetCanLoseFocus(int DynamicMode=FALSE) const { return m_CanLoseFocus; }
	virtual void SetExitCode(int Code) { m_ExitCode=Code; }
	virtual BOOL IsFileModified() const { return FALSE; }
	virtual int GetTypeAndName(string &strType, string &strName) = 0;
	virtual int GetType() const = 0;
	virtual void OnDestroy() {}  // вызывается перед уничтожением окна
	virtual void OnChangeFocus(bool focus); // вызывается при смене фокуса
	virtual void InitKeyBar() {}
	virtual void RedrawKeyBar() { UpdateKeyBar(); }
	virtual FARMACROAREA GetMacroArea() const { return m_MacroArea; }
	virtual bool CanFastHide() const;
	virtual string GetTitle() const = 0;
	virtual bool ProcessEvents() {return true;}

	virtual void Refresh(void) override;

	void SetCanLoseFocus(int Mode) { m_CanLoseFocus=Mode; }
	int GetExitCode() const { return m_ExitCode; }
	void UpdateKeyBar();
	int IsTitleBarVisible() const {return m_TitleBarVisible;}
	int IsTopWindow() const;
	bool HasSaveScreen() const;
	void SetFlags( DWORD flags ) { m_Flags.Set(flags); }
	virtual void SetDeleting(void);
	bool IsDeleting(void) const;
	void Pin(void);
	void UnPin(void);
	bool IsPinned(void) const;
	void SetMacroMode(FARMACROAREA Area);
	int ID(void) const {return m_ID;}

	typedef raii_wrapper<window_ptr, void (window::*)(), void (window::*)()> pinner;
	pinner GetPinner() { return pinner(shared_from_this(), &window::Pin, &window::UnPin); }

protected:
	window();

	int m_ID;
	int m_CanLoseFocus;
	int m_ExitCode;
	int m_KeyBarVisible;
	int m_TitleBarVisible;
	std::unique_ptr<KeyBar> m_windowKeyBar;
	void SetID(int Value) {m_ID=Value;}

private:
	friend class Manager;

	bool m_Deleting;
	long m_BlockCounter;
	FARMACROAREA m_MacroArea;
};
