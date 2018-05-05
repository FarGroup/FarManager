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

#include "taskbar.hpp"

#include "console.hpp"

taskbar::taskbar():
	m_State(TBPF_NOPROGRESS)
{
	CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, IID_PPV_ARGS_Helper(&ptr_setter(m_TaskbarList)));
}

void taskbar::SetProgressState(TBPFLAG tbpFlags)
{
	if (!m_TaskbarList)
		return;

	m_State=tbpFlags;
	m_TaskbarList->SetProgressState(console.GetWindow(),tbpFlags);
}

void taskbar::SetProgressValue(unsigned long long Completed, unsigned long long Total)
{
	if (!m_TaskbarList)
		return;

	m_State=TBPF_NORMAL;
	m_TaskbarList->SetProgressValue(console.GetWindow(),Completed,Total);
}

TBPFLAG taskbar::GetProgressState() const
{
	return m_State;
}

void taskbar::Flash()
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


IndeterminateTaskbar::IndeterminateTaskbar(bool EndFlash):
	EndFlash(EndFlash)
{
	if (taskbar::instance().GetProgressState()!=TBPF_INDETERMINATE)
	{
		taskbar::instance().SetProgressState(TBPF_INDETERMINATE);
	}
}

IndeterminateTaskbar::~IndeterminateTaskbar()
{
	if (taskbar::instance().GetProgressState()!=TBPF_NOPROGRESS)
	{
		taskbar::instance().SetProgressState(TBPF_NOPROGRESS);
	}
	if(EndFlash)
	{
		taskbar::instance().Flash();
	}
}
