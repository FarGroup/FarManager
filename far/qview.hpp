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

class Viewer;

class QuickView:public Panel
{
public:
	QuickView();
	virtual int ProcessKey(int Key) override;
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0) override;
	virtual void Update(int Mode) override;
	virtual void CloseFile() override;
	virtual void QViewDelTempName() override;
	virtual int UpdateIfChanged(panel_update_mode UpdateMode) override;
	virtual void SetTitle() override;
	virtual const string& GetTitle(string &Title) override;
	virtual void SetFocus() override;
	virtual void KillFocus() override;
	virtual void UpdateKeyBar() override;
	virtual int GetCurName(string &strName, string &strShortName) override;

	void ShowFile(const string& FileName,int TempFile,HANDLE hDirPlugin);

private:
	virtual ~QuickView();
	virtual void DisplayObject() override;

	void PrintText(const string& Str);
	void SetMacroMode(int Restore = FALSE);
	void DynamicUpdateKeyBar();

	Viewer *QView;

	string strCurFileName;
	string strCurFileType;
	string strTempName;

	CriticalSection CS;

	int Directory;
	FARMACROAREA PrevMacroMode;
	DirInfoData Data;
	bool OldWrapMode;
	bool OldWrapType;

	bool uncomplete_dirscan;
};
