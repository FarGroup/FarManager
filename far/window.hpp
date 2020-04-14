#ifndef WINDOW_HPP_1A177508_4749_4C46_AE24_0D274332C03A
#define WINDOW_HPP_1A177508_4749_4C46_AE24_0D274332C03A
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

// Internal:
#include "scrobj.hpp"
#include "plugin.hpp"

// Platform:

// Common:
#include "common/smart_ptr.hpp"

// External:

//----------------------------------------------------------------------------

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
	~window() override;
	void Refresh() override;

	virtual bool GetCanLoseFocus(bool DynamicMode = false) const { return m_CanLoseFocus; }
	virtual void SetExitCode(int Code) { m_ExitCode=Code; }
	virtual bool IsFileModified() const { return false; }
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
	virtual bool IsTitleBarVisible() const { return false; }
	virtual bool IsKeyBarVisible() const { return false; }
	virtual void SetDeleting();

	void SetCanLoseFocus(bool Value) { m_CanLoseFocus = Value; }
	int GetExitCode() const { return m_ExitCode; }
	void UpdateKeyBar() const;
	bool IsTopWindow() const;
	bool HasSaveScreen() const;
	void SetFlags( DWORD flags ) { m_Flags.Set(flags); }
	bool IsDeleting() const;
	void Pin();
	void UnPin();
	bool IsPinned() const;
	void SetMacroMode(FARMACROAREA Area);
	int ID() const {return m_ID;}

	[[nodiscard]]
	auto GetPinner() { return make_raii_wrapper(this, &window::Pin, &window::UnPin); }

protected:
	window();

	int m_ID;
	bool m_CanLoseFocus{};
	int m_ExitCode{ -1 };
	std::unique_ptr<KeyBar> m_windowKeyBar;

private:
	friend class Manager;

	void SetID(int Value) {m_ID=Value;}

	bool m_Deleting{};
	long m_BlockCounter{};
	FARMACROAREA m_MacroArea{ MACROAREA_OTHER };
};

#endif // WINDOW_HPP_1A177508_4749_4C46_AE24_0D274332C03A
