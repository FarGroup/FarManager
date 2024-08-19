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
#include "log.hpp"
#include "mix.hpp"

// Platform:
#include "platform.hpp"
#include "platform.com.hpp"
#include "platform.concurrency.hpp"
#include "platform.debug.hpp"

// Common:
#include "common/singleton.hpp"

// External:

//----------------------------------------------------------------------------

class taskbar_impl : public singleton<taskbar_impl>
{
	IMPLEMENTS_SINGLETON;

public:
	void set_state(TBPFLAG const State)
	{
		if (m_State == State)
			return;

		m_State = State;

		console.set_progress_state(m_State);

		m_StateEvent.set();
	}

	void set_value(unsigned long long const Completed, unsigned long long const Total)
	{
		const auto NewState = any_of(m_State, TBPF_NOPROGRESS, TBPF_INDETERMINATE)? TBPF_NORMAL : m_State.load();

		if (m_State == NewState && m_Completed == Completed && m_Total == Total)
			return;

		m_State = NewState;
		m_Completed = Completed;
		m_Total = Total;

		console.set_progress_value(m_State, ToPercent(m_Completed, m_Total));

		m_ValueEvent.set();
	}


	[[nodiscard]]
	TBPFLAG last_state() const
	{
		return m_State;
	}

private:
	taskbar_impl() = default;

	~taskbar_impl()
	{
		m_ExitEvent.set();
	}

	void handler() const
	{
		os::debug::set_thread_name(L"Taskbar processor");

		SCOPED_ACTION(os::com::initialize);

		os::com::ptr<ITaskbarList3> TaskbarList;
		if (const auto Result = CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, IID_PPV_ARGS_Helper(&ptr_setter(TaskbarList))); FAILED(Result))
		{
			LOGWARNING(L"CoCreateInstance(CLSID_TaskbarList): {}"sv, os::format_error(Result));
			return;
		}

		if (!TaskbarList)
		{
			LOGWARNING(L"!TaskbarList"sv);
			return;
		}

		for (;;)
		{
			switch (os::handle::wait_any(
			{
				m_ExitEvent.native_handle(),
				m_StateEvent.native_handle(),
				m_ValueEvent.native_handle(),
			}))
			{
			case 0:
				return;

			case 1:
				TaskbarList->SetProgressState(console.GetWindow(), m_State);
				break;

			case 2:
				TaskbarList->SetProgressValue(console.GetWindow(), m_Completed, m_Total);
				break;
			}
		}
	}

	std::atomic<TBPFLAG> m_State{ TBPF_NOPROGRESS };
	std::atomic_uint64_t
		m_Completed{},
		m_Total{};

	os::event
		m_ExitEvent{ os::event::type::manual, os::event::state::nonsignaled },
		m_StateEvent{ os::event::type::automatic, os::event::state::nonsignaled },
		m_ValueEvent{ os::event::type::automatic, os::event::state::nonsignaled };

	os::thread m_ComThread{ IsWindows7OrGreater()? os::thread(&taskbar_impl::handler, this) : os::thread() };
};

void taskbar::set_state(TBPFLAG const State)
{
	taskbar_impl::instance().set_state(State);
}

void taskbar::set_value(unsigned long long const Completed, unsigned long long const Total)
{
	taskbar_impl::instance().set_value(Completed, Total);
}

static auto last_state()
{
	return taskbar_impl::instance().last_state();
}

void taskbar::flash()
{
	const auto ConsoleWindow = console.GetWindow();
	WINDOWINFO WindowInfo{ sizeof(WindowInfo)};

	if (!GetWindowInfo(ConsoleWindow, &WindowInfo))
	{
		LOGWARNING(L"GetWindowInfo(ConsoleWindow): {}"sv, os::last_error());
		return;
	}

	if (WindowInfo.dwWindowStatus == WS_ACTIVECAPTION)
		return;

	FLASHWINFO FlashInfo{sizeof(FlashInfo), ConsoleWindow, FLASHW_ALL | FLASHW_TIMERNOFG, 5, 0};
	// https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-flashwindowex#return-value
	// The return value specifies the window's state before the call to the FlashWindowEx function.
	// We don't care.
	FlashWindowEx(&FlashInfo);
}


taskbar::indeterminate::indeterminate(bool const EndFlash):
	m_EndFlash(EndFlash)
{
	set_state(TBPF_INDETERMINATE);
}

taskbar::indeterminate::~indeterminate()
{
	set_state(TBPF_NOPROGRESS);

	if(m_EndFlash)
	{
		flash();
	}
}

taskbar::state::state(TBPFLAG const State):
	m_PreviousState(last_state())
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
