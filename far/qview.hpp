#ifndef QVIEW_HPP_944492CA_F3F6_49F8_854A_2C5D30567B9E
#define QVIEW_HPP_944492CA_F3F6_49F8_854A_2C5D30567B9E
#pragma once

/*
qview.hpp

Quick view panel
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
#include "dirinfo.hpp"
#include "plugin.hpp"

// Platform:

// Common:
#include "common/smart_ptr.hpp"

// External:

//----------------------------------------------------------------------------

class Viewer;

class QuickView final: public Panel
{
	struct private_tag { explicit private_tag() = default; };

public:
	static qview_panel_ptr create(window_ptr Owner);
	QuickView(private_tag, window_ptr Owner);
	~QuickView() override;

	void ShowFile(string_view FileName, const UserDataItem* UserData, bool TempFile, const plugin_panel* hDirPlugin);

private:
	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	long long VMProcess(int OpCode, void* vParam = nullptr, long long iParam = 0) override;
	void Update(int Mode) override;
	void CloseFile() override;
	void QViewDelTempName() override;
	void UpdateIfChanged(bool Changed = false) override;
	void RefreshTitle() override;
	string GetTitle() const override;
	void UpdateKeyBar() override;
	bool GetCurName(string &strName, string &strShortName) const override;
	void DisplayObject() override;
	Viewer* GetViewer() override;
	Viewer* GetById(int ID) override;
	void OnDestroy() override;

	void PrintText(string_view Str) const;
	void DynamicUpdateKeyBar() const;

	unique_ptr_with_ondestroy<Viewer> QView;
	string strCurFileName;
	UserDataItem CurUserData{};
	string strCurFileType;
	DirInfoData Data;
	bool OldWrapMode{};
	bool OldWrapType{};
	bool m_TemporaryFile{};
	bool uncomplete_dirscan{};

	enum class scan_status
	{
		none = 0,
		real_ok = 1,
		real_fail = 2,
		plugin_fail = 3,
		plugin_ok = 4,
	}
	m_DirectoryScanStatus{ scan_status::none };
};

#endif // QVIEW_HPP_944492CA_F3F6_49F8_854A_2C5D30567B9E
