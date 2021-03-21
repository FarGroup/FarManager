/*
taskbar.cpp

Windows 7 taskbar support
*/
/*
Copyright © 2009 Far Group
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
#include "taskbar.hpp"

// Internal:
#include "console.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

static std::atomic<TBPFLAG> LastState;

static auto create()
{
	os::com::ptr<ITaskbarList3> TaskbarList;
	CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, IID_PPV_ARGS_Helper(&ptr_setter(TaskbarList)));
	return TaskbarList;
}

void taskbar::set_state(TBPFLAG const State)
{
	SCOPED_ACTION(os::com::initialize);

	const auto TaskbarList = create();
	if (!TaskbarList)
		return;

	LastState = State;
	TaskbarList->SetProgressState(console.GetWindow(), State);
}

void taskbar::set_value(unsigned long long const Completed, unsigned long long const Total)
{
	SCOPED_ACTION(os::com::initialize);

	const auto TaskbarList = create();
	if (!TaskbarList)
		return;

	LastState = TBPF_NORMAL;
	TaskbarList->SetProgressValue(console.GetWindow(), Completed, Total);
}

void taskbar::flash()
{
	const auto ConsoleWindow = console.GetWindow();
	WINDOWINFO WindowInfo{ sizeof(WindowInfo)};

	if (!GetWindowInfo(ConsoleWindow, &WindowInfo))
		return;

	if (WindowInfo.dwWindowStatus == WS_ACTIVECAPTION)
		return;

	FLASHWINFO FlashInfo{sizeof(FlashInfo), ConsoleWindow, FLASHW_ALL | FLASHW_TIMERNOFG, 5, 0};
	FlashWindowEx(&FlashInfo);
}


taskbar::indeterminate::indeterminate(bool const EndFlash):
	m_EndFlash(EndFlash)
{
	if (LastState != TBPF_INDETERMINATE)
	{
		set_state(TBPF_INDETERMINATE);
	}
}

taskbar::indeterminate::~indeterminate()
{
	if (LastState != TBPF_NOPROGRESS)
	{
		set_state(TBPF_NOPROGRESS);
	}

	if(m_EndFlash)
	{
		flash();
	}
}

taskbar::state::state(TBPFLAG const State):
	m_PreviousState(LastState)
{
	if (m_PreviousState == State)
		return;

	if (m_PreviousState == TBPF_INDETERMINATE || m_PreviousState == TBPF_NOPROGRESS)
	{
		set_value(1, 1);
	}

	set_state(State);
	flash();
}

taskbar::state::~state()
{
	set_state(m_PreviousState);
}
