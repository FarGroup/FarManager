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

#include "panel.hpp"
#include "notification.hpp"

class DizViewer;

class InfoList:public Panel
{
public:
	InfoList();

private:
	virtual ~InfoList();

	virtual void DisplayObject() override;
	virtual int ProcessKey(const Manager::Key& Key) override;
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual __int64 VMProcess(int OpCode, void *vParam = nullptr, __int64 iParam = 0) override;
	virtual void Update(int Mode) override;
	virtual void SetFocus() override;
	virtual void KillFocus() override;
	virtual const string& GetTitle(string &Title) const override;
	virtual void UpdateKeyBar() override;
	virtual void CloseFile() override;
	virtual int GetCurName(string &strName, string &strShortName) const override;

	bool ShowDirDescription(int YPos);
	bool ShowPluginDescription(int YPos);
	void PrintText(const string& Str) const;
	void PrintText(LNGID MsgID) const;
	void PrintInfo(const string& Str) const;
	void PrintInfo(LNGID MsgID) const;
	void SelectShowMode();
	void DrawTitle(string &strTitle, int Id, int &CurY);
	int  OpenDizFile(const string& DizFile, int YPos);
	void SetMacroMode(int Restore = FALSE);
	void DynamicUpdateKeyBar() const;

	DizViewer *DizView;
	FARMACROAREA PrevMacroMode;
	bool OldWrapMode;
	bool OldWrapType;
	string strDizFileName;
	struct InfoListSectionState;
	std::vector<struct InfoListSectionState> SectionState;
	listener PowerListener;
};
