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

TaskBarCore TBC;

TaskBarCore::TaskBarCore():
	State(TBPF_NOPROGRESS),
	pTaskbarList(nullptr)
{
	HRESULT hRes=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);

	switch (hRes)
	{
		case S_OK:
		case S_FALSE:
			CoInited=true;
		case RPC_E_CHANGED_MODE:
			CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER,
#ifdef __GNUC__
				IID_ITaskbarList3, IID_PPV_ARGS_Helper(&pTaskbarList)
#else
				IID_PPV_ARGS(&pTaskbarList)
#endif
			);
			break;
		default:
			CoInited=false;
			break;
	}
}

TaskBarCore::~TaskBarCore()
{
	if (pTaskbarList)
	{
		pTaskbarList->Release();
	}

	if (CoInited)
	{
		CoUninitialize();
	}
}

void TaskBarCore::SetProgressState(TBPFLAG tbpFlags)
{
	if (pTaskbarList)
	{
		State=tbpFlags;
		pTaskbarList->SetProgressState(Console.GetWindow(),tbpFlags);
	}
}

void TaskBarCore::SetProgressValue(UINT64 Completed, UINT64 Total)
{
	if (pTaskbarList)
	{
		State=TBPF_NORMAL;
		pTaskbarList->SetProgressValue(Console.GetWindow(),Completed,Total);
	}
}

TBPFLAG TaskBarCore::GetProgressState()
{
	return State;
}

void TaskBarCore::Flash()
{
	WINDOWINFO WI={sizeof(WI)};

	if (GetWindowInfo(Console.GetWindow(),&WI))
	{
		if (WI.dwWindowStatus!=WS_ACTIVECAPTION)
		{
			FLASHWINFO FWI={sizeof(FWI),Console.GetWindow(),FLASHW_ALL|FLASHW_TIMERNOFG,5,0};
			FlashWindowEx(&FWI);
		}
	}
}



TaskBar::TaskBar(bool EndFlash):
	EndFlash(EndFlash)
{
	if (TBC.GetProgressState()!=TBPF_INDETERMINATE)
	{
		TBC.SetProgressState(TBPF_INDETERMINATE);
	}
}

TaskBar::~TaskBar()
{
	if (TBC.GetProgressState()!=TBPF_NOPROGRESS)
	{
		TBC.SetProgressState(TBPF_NOPROGRESS);
	}
	if(EndFlash)
	{
		TBC.Flash();
	}
}
