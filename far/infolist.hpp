#ifndef INFOLIST_HPP_938248E2_BB38_43DF_BDD3_D582C383A102
#define INFOLIST_HPP_938248E2_BB38_43DF_BDD3_D582C383A102
#pragma once

/*
infolist.hpp

Информационная панель
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
#include "panel.hpp"
#include "notification.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

enum class lng : int;
class DizViewer;

class InfoList final: public Panel
{
	struct private_tag { explicit private_tag() = default; };

public:
	static info_panel_ptr create(window_ptr Owner);
	InfoList(private_tag, window_ptr Owner);
	~InfoList() override;

private:
	void DisplayObject() override;
	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	long long VMProcess(int OpCode, void* vParam = nullptr, long long iParam = 0) override;
	void Update(int Mode) override;
	string GetTitle() const override;
	void UpdateKeyBar() override;
	void CloseFile() override;
	bool GetCurName(string &strName, string &strShortName) const override;
	Viewer* GetViewer() override;
	Viewer* GetById(int ID) override;

	bool ShowDirDescription(int YPos);
	bool ShowPluginDescription(int YPos) const;
	void PrintText(string_view Str) const;
	void PrintText(lng MsgID) const;
	void PrintInfo(string_view Str) const;
	void PrintInfo(lng MsgID) const;
	void SelectShowMode();
	void DrawTitle(string_view Title, int Id, int &CurY);
	bool OpenDizFile(string_view DizFile, int YPos);
	void DynamicUpdateKeyBar() const;

	unique_ptr_with_ondestroy<DizViewer> DizView;
	bool OldWrapMode;
	bool OldWrapType;
	string strDizFileName;
	struct InfoListSectionState;
	std::vector<InfoListSectionState> SectionState;

	class power_listener: listener
	{
	public:
		explicit power_listener(std::function<void()> EventHandler);
		~power_listener();
	}
	PowerListener;
};

#endif // INFOLIST_HPP_938248E2_BB38_43DF_BDD3_D582C383A102
