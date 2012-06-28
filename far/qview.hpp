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
#include "CriticalSections.hpp"
#include "dirinfo.hpp"

class Viewer;

class QuickView:public Panel
{
	private:
		Viewer *QView;

		string strCurFileName;
		string strCurFileType;
		string strTempName;

		CriticalSection CS;

		int Directory;
		int PrevMacroMode;
		DirInfoData Data;
		bool OldWrapMode;
		bool OldWrapType;

		bool uncomplete_dirscan;

	private:
		virtual void DisplayObject();
		void PrintText(const wchar_t *Str);

		void SetMacroMode(int Restore = FALSE);

		void DynamicUpdateKeyBar();

	public:
		QuickView();
		virtual ~QuickView();

	public:
		virtual int ProcessKey(int Key);
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
#ifdef FAR_LUA
#else
		virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0);
#endif
		virtual void Update(int Mode);
		void ShowFile(const wchar_t *FileName,int TempFile,HANDLE hDirPlugin);
		virtual void CloseFile();
		virtual void QViewDelTempName();

		virtual int UpdateIfChanged(int UpdateMode);
		virtual void SetTitle();
		virtual string &GetTitle(string &Title,int SubLen=-1,int TruncSize=0);
		virtual void SetFocus();
		virtual void KillFocus();
		virtual BOOL UpdateKeyBar();
		virtual int GetCurName(string &strName, string &strShortName);
};
