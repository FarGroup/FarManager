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
#include "viewer.hpp"

//class Viewer;

/* $ 12.10.2001 SKV
  заврапим Viewer что бы отслеживать рекурсивность вызова
  методов DizView и случайно не удалить его во время вызова.
*/
class DizViewer: public Viewer
{
	public:
		int InRecursion;

	public:
		DizViewer():InRecursion(0) {}
		virtual ~DizViewer() {}

	public:
		virtual int ProcessKey(int Key)
		{
			InRecursion++;
			int res=Viewer::ProcessKey(Key);
			InRecursion--;
			return res;
		}
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
		{
			InRecursion++;
			int res=Viewer::ProcessMouse(MouseEvent);
			InRecursion--;
			return res;
		}
};

class InfoList:public Panel
{
	// Состояние секций
	struct InfoListSectionState
	{
		bool Show;   // раскрыть/свернуть?
		SHORT Y;     // Где?
	};

	enum InfoListSectionStateIndex
	{
		// Порядок не менять! Только добавлять в конец!
		ILSS_DISKINFO,
		ILSS_MEMORYINFO,
		ILSS_DIRDESCRIPTION,
		ILSS_PLDESCRIPTION,
		ILSS_POWERSTATUS,

		ILSS_LAST
	};

	private:
		DizViewer *DizView;
		int PrevMacroMode;
		bool OldWrapMode;
		bool OldWrapType;
		string strDizFileName;
		InfoListSectionState SectionState[ILSS_LAST];

	private:
		virtual void DisplayObject();
		bool ShowDirDescription(int YPos);
		bool ShowPluginDescription(int YPos);

		void PrintText(const wchar_t *Str);
		void PrintText(LNGID MsgID);
		void PrintInfo(const wchar_t *Str);
		void PrintInfo(LNGID MsgID);
		void SelectShowMode(void);
		void DrawTitle(string &strTitle,int Id,int &CurY);

		int  OpenDizFile(const wchar_t *DizFile,int YPos);
		void SetMacroMode(int Restore = FALSE);
		void DynamicUpdateKeyBar();

	public:
		InfoList();
		virtual ~InfoList();

	public:
		virtual int ProcessKey(int Key);
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
		virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0);
		virtual void Update(int Mode);
		virtual void SetFocus();
		virtual void KillFocus();
		virtual string &GetTitle(string &Title,int SubLen=-1,int TruncSize=0);
		virtual BOOL UpdateKeyBar();
		virtual void CloseFile();
		virtual int GetCurName(string &strName, string &strShortName);
		virtual int UpdateIfChanged(int UpdateMode);
};
