#ifndef CMDLINE_HPP_7E68C776_4AA9_4A24_BE9F_7F7FA6D50F30
#define CMDLINE_HPP_7E68C776_4AA9_4A24_BE9F_7F7FA6D50F30
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

// Internal:
#include "scrobj.hpp"
#include "editcontrol.hpp"

// Platform:

// Common:
#include "common/function_ref.hpp"

// External:

//----------------------------------------------------------------------------

struct execute_info
{
	enum class wait_mode { no_wait, if_needed, wait_finish };
	enum class source_mode { unknown, known, known_executable, known_external };
	enum class echo { disabled, enabled, ignored };

	string Command;
	string DisplayCommand;
	string Directory;
	wait_mode WaitMode{ wait_mode::if_needed };
	source_mode SourceMode{ source_mode::unknown };
	bool RunAs{};
	bool Echo{ true };
	bool UseAssociations{ true };
};

class CommandLine final: public SimpleScreenObject
{
public:
	explicit CommandLine(window_ptr Owner);
	~CommandLine() override;

	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	long long VMProcess(int OpCode, void* vParam = nullptr, long long iParam=0) override;
	void ResizeConsole() override;

	const string& GetCurDir() const { return m_CurDir; }
	void SetCurDir(string_view CurDir);

	const string& GetString() const { return CmdStr.GetString(); }
	void SetString(string_view Str, bool Redraw);
	void InsertString(string_view Str);
	void ExecString(execute_info& Info);
	void ShowViewEditHistory();
	void SetCurPos(int Pos, int LeftPos=0, bool Redraw=true);
	int GetCurPos() const { return CmdStr.GetCurPos(); }
	int GetLeftPos() const { return CmdStr.GetLeftPos(); }
	void SetPersistentBlocks(bool Mode);
	void SetDelRemovesBlocks(bool Mode);
	void SetAutoComplete(int Mode);
	void GetSelection(intptr_t &Start,intptr_t &End) const { CmdStr.GetSelection(Start,End); }
	void Select(int Start, int End) { CmdStr.Select(Start,End); CmdStr.AdjustMarkBlock(); }
	void LockUpdatePanel(bool Mode);
	int GetPromptSize() const {return PromptSize;}
	void SetPromptSize(int NewSize);
	void DrawFakeCommand(string_view FakeCommand);

private:
	void DisplayObject() override;
	size_t DrawPrompt();
	bool ProcessOSCommands(string_view CmdLine, function_ref<void()> ConsoleActivatior);
	struct segment
	{
		string Text;
		FarColor Colour;
		bool Collapsible;
	};
	std::list<segment> GetPrompt();
	static bool IntChDir(string_view CmdLine, bool ClosePanel, bool Silent = false);

	friend class SetAutocomplete;

	int PromptSize;
	EditControl CmdStr;
	string m_CurDir;
	string strLastCmdStr;
	int LastCmdPartLength;
	string m_CurCmdStr;
	std::stack<string> ppstack;
};

#endif // CMDLINE_HPP_7E68C776_4AA9_4A24_BE9F_7F7FA6D50F30
