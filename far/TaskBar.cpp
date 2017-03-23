/*
TaskBar.cpp

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

#include "headers.hpp"
#pragma hdrstop

#include "TaskBar.hpp"
#include "console.hpp"

taskbar& Taskbar()
{
	static taskbar tb;
	return tb;
}

taskbar::taskbar():
	State(TBPF_NOPROGRESS)
{
	CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, IID_PPV_ARGS_Helper(&ptr_setter(mTaskbarList)));
}

void taskbar::SetProgressState(TBPFLAG tbpFlags)
{
	if (!mTaskbarList)
		return;

	State=tbpFlags;
	mTaskbarList->SetProgressState(Console().GetWindow(),tbpFlags);
}

void taskbar::SetProgressValue(unsigned long long Completed, unsigned long long Total)
{
	if (!mTaskbarList)
		return;

	State=TBPF_NORMAL;
	mTaskbarList->SetProgressValue(Console().GetWindow(),Completed,Total);
}

TBPFLAG taskbar::GetProgressState() const
{
	return State;
}

void taskbar::Flash()
{
	WINDOWINFO WI={sizeof(WI)};

	if (!GetWindowInfo(Console().GetWindow(), &WI))
		return;

	if (WI.dwWindowStatus == WS_ACTIVECAPTION)
		return;

	FLASHWINFO FWI={sizeof(FWI),Console().GetWindow(),FLASHW_ALL|FLASHW_TIMERNOFG,5,0};
	FlashWindowEx(&FWI);
}


IndeterminateTaskBar::IndeterminateTaskBar(bool EndFlash):
	EndFlash(EndFlash)
{
	if (Taskbar().GetProgressState()!=TBPF_INDETERMINATE)
	{
		Taskbar().SetProgressState(TBPF_INDETERMINATE);
	}
}

IndeterminateTaskBar::~IndeterminateTaskBar()
{
	if (Taskbar().GetProgressState()!=TBPF_NOPROGRESS)
	{
		Taskbar().SetProgressState(TBPF_NOPROGRESS);
	}
	if(EndFlash)
	{
		Taskbar().Flash();
	}
}
