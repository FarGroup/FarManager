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

#include "window.hpp"
#include "viewer.hpp"

class Panel;
class CommandLine;
class MenuBar;

class FilePanels:public window,public ViewerContainer
{
public:
	static filepanels_ptr create(bool CreatePanels, int DirCount);
	virtual ~FilePanels();

	virtual int ProcessKey(const Manager::Key& Key) override;
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual __int64 VMProcess(int OpCode, void *vParam = nullptr, __int64 iParam = 0) override;
	virtual void SetScreenPosition() override;
	virtual int GetTypeAndName(string &strType, string &strName) override;
	virtual int GetType() const override { return windowtype_panels; }
	virtual void RedrawKeyBar() override;
	virtual void ShowConsoleTitle() override;
	virtual void ResizeConsole() override;
	virtual bool CanFastHide() const override;
	virtual void Refresh() override;
	virtual FARMACROAREA GetMacroArea() const override;
	virtual Viewer* GetViewer(void) override;
	virtual Viewer* GetById(int ID) override;

	Panel* GetAnotherPanel(const Panel *Current);
	Panel* ChangePanelToFilled(Panel *Current, int NewType);
	Panel* ChangePanel(Panel *Current, int NewType, int CreateNew, int Force);
	void GoToFile(const string& FileName);
	int ChangePanelViewMode(Panel *Current, int Mode, BOOL RefreshWindow);
	Panel* ActivePanel() { return m_ActivePanel; }
	Panel* PassivePanel() { return GetAnotherPanel(m_ActivePanel); }
	// BUGBUG
	void SetActivePanel(Panel* p) { m_ActivePanel = p; }

	KeyBar& GetKeybar() { return *m_windowKeyBar; }
	CommandLine* GetCmdLine(void);
	MenuBar* GetTopMenuBar(void);

private:
	FilePanels();
	void Init(int DirCount);
	virtual void DisplayObject() override;
	virtual string GetTitle() const override { return string(); }

	Panel* CreatePanel(int Type);
	void DeletePanel(Panel *Deleted);
	void SetPanelPositions(bool LeftFullScreen, bool RightFullScreen);
	int SetAnhoterPanelFocus();
	int SwapPanels();
	void Update();

	std::unique_ptr<CommandLine> CmdLine;
	std::unique_ptr<MenuBar> TopMenuBar;

public:
	Panel *LastLeftFilePanel;
	Panel *LastRightFilePanel;
	Panel *LeftPanel;
	Panel *RightPanel;

	int LastLeftType;
	int LastRightType;
	int LeftStateBeforeHide;
	int RightStateBeforeHide;

private:
	Panel *m_ActivePanel;
};
