#pragma once

/*
cmdline.hpp

Командная строка
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

#include "scrobj.hpp"
#include "edit.hpp"
#include "TStack.hpp"


enum
{
	FCMDOBJ_LOCKUPDATEPANEL   = 0x00010000,
};

struct PushPopRecord
{
	string strName;

	PushPopRecord& operator=(const PushPopRecord &rhs) {strName=rhs.strName; return *this;}
};


class CommandLine:public ScreenObject
{
	friend class SetAutocomplete;
	private:
		EditControl CmdStr;
		SaveScreen *BackgroundScreen;
		string strCurDir;
		string strLastCmdStr;
		int LastCmdPartLength;
		TStack<PushPopRecord> ppstack;

	private:
		virtual void DisplayObject();
		int ProcessOSCommands(const string& CmdLine,bool SeparateWindow, bool &PrintCommand);
		void GetPrompt(string &strDestStr);
		bool IntChDir(const string& CmdLine,int ClosePanel,bool Selent=false);
		bool CheckCmdLineForHelp(const wchar_t *CmdLine);
		bool CheckCmdLineForSet(const string& CmdLine);

	public:
		CommandLine();
		virtual ~CommandLine();

	public:
		virtual int ProcessKey(int Key);
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
#ifdef FAR_LUA_TEMP
#else
		virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0);
#endif

		virtual void ResizeConsole();

		int GetCurDir(string &strCurDir);
		void SetCurDir(const string& CurDir);

		void GetString(string &strStr) { CmdStr.GetString(strStr); };
		int GetLength() { return CmdStr.GetLength(); };
		void SetString(const string& Str,bool Redraw=true);
		void InsertString(const string& Str);

		int ExecString(const string& Str, bool AlwaysWaitFinish, bool SeparateWindow = false, bool DirectRun = false, bool WaitForIdle = false, bool Silent = false, bool RunAs = false);

		void ShowViewEditHistory();

		void SetCurPos(int Pos, int LeftPos=0);
		int GetCurPos() { return CmdStr.GetCurPos(); };
		int GetLeftPos() { return CmdStr.GetLeftPos(); };

		void SetPersistentBlocks(int Mode);
		void SetDelRemovesBlocks(int Mode);
		void SetAutoComplete(int Mode);

		void GetSelString(string &strStr) { CmdStr.GetSelString(strStr); };
		void GetSelection(int &Start,int &End) { CmdStr.GetSelection(Start,End); };
		void Select(int Start, int End) { CmdStr.Select(Start,End); };

		void SaveBackground(int X1,int Y1,int X2,int Y2);
		void SaveBackground();
		void ShowBackground();
		void CorrectRealScreenCoord();
		void LockUpdatePanel(int Mode) {Flags.Change(FCMDOBJ_LOCKUPDATEPANEL,Mode);};
};
