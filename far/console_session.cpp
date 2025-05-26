/*
console_session.cpp


*/
/*
Copyright © 2017 Far Group
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "console_session.hpp"

// Internal:
#include "desktop.hpp"
#include "global.hpp"
#include "manager.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "console.hpp"
#include "colormix.hpp"
#include "constitle.hpp"
#include "scrbuf.hpp"
#include "ctrlobj.hpp"
#include "cmdline.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

static size_t margin(bool const NewLine, bool const CmdLine)
{
	return Global->Opt->ShowKeyBar + CmdLine + NewLine;
}

static void command(std::optional<string_view> const Command)
{
	if (!Command.has_value())
		return;

	Global->CtrlObject->CmdLine()->DrawFakeCommand(*Command);
	ConsoleTitle::SetFarTitle(*Command);
}

void console_session::activate(std::optional<string_view> const Command, bool const NewLine)
{
	if (m_Activations++)
	{
		scroll(margin(NewLine, Command.has_value()));
		command(Command);
		return;
	}

	++Global->SuppressIndicators;
	++Global->SuppressClock;

	Global->WindowManager->ModalDesktopWindow();
	Global->WindowManager->PluginCommit();


	// BUGBUG, implement better & safer way to do this
	const auto LockCount = Global->ScrBuf->GetLockCount();
	Global->ScrBuf->SetLockCount(0);

	Global->ScrBuf->Flush();

	// BUGBUG, implement better & safer way to do this
	Global->ScrBuf->SetLockCount(LockCount);



	console.SetTextAttributes(colors::PaletteColorToFarColor(COL_COMMANDLINEUSERSCREEN));

	scroll(margin(NewLine, true));

	MoveRealCursor(0, ScrY - (Global->Opt->ShowKeyBar? 1 : 0));
	SetInitialCursorType();

	command(Command);
}

void console_session::deactivate(bool const NewLine)
{
	if (!m_Activations)
		return;

	--m_Activations;

	snap(NewLine);

	if (m_Activations)
		return;

	--Global->SuppressClock;
	--Global->SuppressIndicators;

	Global->WindowManager->UnModalDesktopWindow();
	Global->WindowManager->PluginCommit();

	Global->ScrBuf->Flush();
}

void console_session::snap(bool const NewLine)
{
	Global->ScrBuf->FillBuf();

	if (NewLine && scroll(2))
		Global->ScrBuf->FillBuf();

	if (!m_Activations && scroll(margin(NewLine, true)))
		Global->ScrBuf->FillBuf();

	Global->WindowManager->Desktop()->TakeSnapshot();
}

bool console_session::scroll(size_t const SpaceNeeded)
{
	assert(SpaceNeeded <= 3);

	const auto SpaceAvailable = NumberOfEmptyLines(SpaceNeeded);

	if (SpaceAvailable >= SpaceNeeded)
		return false;

	MoveRealCursor(0, ScrY);
	std::wcout << L"\n\n\n"sv.substr(0, SpaceNeeded - SpaceAvailable) << std::flush;
	return true;
}
