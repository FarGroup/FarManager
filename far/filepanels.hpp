#ifndef FILEPANELS_HPP_B2D6495E_DA8B_4E72_80F5_37282A14C316
#define FILEPANELS_HPP_B2D6495E_DA8B_4E72_80F5_37282A14C316
#pragma once

/*
filepanels.hpp

файловые панели
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
#include "window.hpp"
#include "viewer.hpp"
#include "panelfwd.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

class CommandLine;
class MenuBar;

class FilePanels final: public window, public ViewerContainer
{
	struct private_tag { explicit private_tag() = default; };

public:
	static filepanels_ptr create(bool CreateRealPanels);

	explicit FilePanels(private_tag);
	~FilePanels() override;

	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	long long VMProcess(int OpCode, void* vParam = nullptr, long long iParam = 0) override;
	void SetScreenPosition() override;
	int GetTypeAndName(string &strType, string &strName) override;
	int GetType() const override { return windowtype_panels; }
	void RedrawKeyBar() override;
	void ShowConsoleTitle() override;
	void ResizeConsole() override;
	bool CanFastHide() const override;
	FARMACROAREA GetMacroArea() const override;
	void Show() override;
	void DisplayObject() override;
	string GetTitle() const override { return {}; }

	bool IsKeyBarVisible() const override;
	Viewer* GetViewer() override;
	Viewer* GetById(int ID) override;

	panel_ptr LeftPanel() const { return m_Panels[panel_left].m_Panel; }
	panel_ptr RightPanel() const { return m_Panels[panel_right].m_Panel; }
	panel_ptr ActivePanel() const { return m_Panels[m_ActivePanelIndex].m_Panel; }
	panel_ptr PassivePanel() const { return m_Panels[!m_ActivePanelIndex].m_Panel; }

	bool IsLeft(const Panel* What) const { return What == LeftPanel().get(); }
	bool IsRight(const Panel* What) const { return What == RightPanel().get(); }
	bool IsLeft(const panel_ptr& What) const { return What == LeftPanel(); }
	bool IsRight(const panel_ptr& What) const { return What == RightPanel(); }
	bool IsLeftActive() const { return m_ActivePanelIndex == panel_left; }
	bool IsRightActive() const { return m_ActivePanelIndex == panel_right; }


	panel_ptr GetAnotherPanel(panel_ptr Current) const { return GetAnotherPanel(Current.get()); }
	panel_ptr GetAnotherPanel(const Panel* Current) const;
	panel_ptr ChangePanelToFilled(panel_ptr Current, panel_type NewType);
	panel_ptr ChangePanel(panel_ptr Current, panel_type NewType, int CreateNew, int Force);
	void GoToFile(string_view FileName);
	bool ChangePanelViewMode(panel_ptr Current, int Mode, bool RefreshWindow);
	void SetActivePanel(panel_ptr p) { return SetActivePanel(p.get()); }
	void SetActivePanel(Panel* ToBeActive);

	KeyBar& GetKeybar() const { return *m_windowKeyBar; }
	CommandLine* GetCmdLine() const;

private:

	void Init();
	static void SetPassivePanelInternal(panel_ptr ToBePassive);
	void SetActivePanelInternal(panel_ptr ToBeActive);

	panel_ptr CreatePanel(panel_type Type);
	void SetPanelPositions(bool LeftFullScreen, bool RightFullScreen) const;
	int SetAnhoterPanelFocus();
	int SwapPanels();

	std::unique_ptr<CommandLine> CmdLine;
	std::unique_ptr<MenuBar> TopMenuBar;

	enum panel_index
	{
		panel_left,
		panel_right,

		panels_count
	};

	struct panel_data
	{
		panel_data(): m_LastType(panel_type::FILE_PANEL), m_StateBeforeHide() {}

		panel_ptr m_Panel;
		panel_ptr m_LastFilePanel;
		panel_type m_LastType;
		int m_StateBeforeHide;
	}
	m_Panels[panels_count];

	panel_index m_ActivePanelIndex;
};

#endif // FILEPANELS_HPP_B2D6495E_DA8B_4E72_80F5_37282A14C316
