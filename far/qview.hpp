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

#include "panel.hpp"
#include "synchro.hpp"
#include "dirinfo.hpp"
#include "viewer.hpp"

class QuickView:public Panel
{
	struct private_tag {};
public:
	static qview_panel_ptr create(window_ptr Owner);
	QuickView(private_tag, window_ptr Owner);
	virtual ~QuickView();
	void ShowFile(const string& FileName, bool TempFile, PluginHandle* hDirPlugin);

private:
	virtual int ProcessKey(const Manager::Key& Key) override;
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual __int64 VMProcess(int OpCode, void *vParam = nullptr, __int64 iParam = 0) override;
	virtual void Update(int Mode) override;
	virtual void CloseFile() override;
	virtual void QViewDelTempName() override;
	virtual bool UpdateIfChanged(bool Idle) override;
	virtual void SetTitle() override;
	virtual string GetTitle() const override;
	virtual void UpdateKeyBar() override;
	virtual int GetCurName(string &strName, string &strShortName) const override;
	virtual void DisplayObject() override;
	virtual Viewer* GetViewer(void) override {return QView.get();}
	virtual Viewer* GetById(int ID) override;

	void PrintText(const string& Str);
	void DynamicUpdateKeyBar() const;

	unique_ptr_with_ondestroy<Viewer> QView;

	string strCurFileName;
	string strCurFileType;

	CriticalSection CS;

	int Directory;
	DirInfoData Data;
	bool OldWrapMode;
	bool OldWrapType;
	bool m_TemporaryFile;
	bool uncomplete_dirscan;
};

#endif // QVIEW_HPP_944492CA_F3F6_49F8_854A_2C5D30567B9E
