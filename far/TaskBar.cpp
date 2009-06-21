/*
TaskBar.cpp

Windows 7 taskbar support
*/
/*
Copyright (c) 2009 Far Group
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
#include "iswind.hpp"

TaskBarCore TBC;

TaskBarCore::TaskBarCore()
{
	pTaskbarList=NULL;
	HRESULT hRes=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
	switch(hRes)
	{
	case S_OK:
	case S_FALSE:
		CoInited=true;
	case RPC_E_CHANGED_MODE:
#ifndef __GNUC__ //BUGBUG
		CoCreateInstance(CLSID_TaskbarList,NULL,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&pTaskbarList));
#endif
		break;

	default:
		CoInited=false;
		break;
	}
}

TaskBarCore::~TaskBarCore()
{
	if(pTaskbarList)
	{
		pTaskbarList->Release();
	}

	if(CoInited)
	{
		CoUninitialize();
	}
}

void TaskBarCore::SetProgressState(TBPFLAG tbpFlags)
{
	if(pTaskbarList)
	{
		State=tbpFlags;
		pTaskbarList->SetProgressState(hFarWnd,tbpFlags);
	}
}

void TaskBarCore::SetProgressValue(UINT64 Completed, UINT64 Total)
{
	if(pTaskbarList)
	{
		State=TBPF_NORMAL;
		pTaskbarList->SetProgressValue(hFarWnd,Completed,Total);
	}
}

TBPFLAG TaskBarCore::GetProgressState()
{
	return State;
}

void TaskBarCore::Flash()
{
	WINDOWINFO WI={sizeof(WI)};
	if(GetWindowInfo(hFarWnd,&WI))
	{
		if(WI.dwWindowStatus!=WS_ACTIVECAPTION)
		{
			FLASHWINFO FWI={sizeof(FWI),hFarWnd,FLASHW_ALL|FLASHW_TIMERNOFG,0,0};
			FlashWindowEx(&FWI);
		}
	}
}



TaskBar::TaskBar()
{
	if(TBC.GetProgressState()!=TBPF_INDETERMINATE)
		TBC.SetProgressState(TBPF_INDETERMINATE);
}

TaskBar::~TaskBar()
{
	if(TBC.GetProgressState()!=TBPF_NOPROGRESS)
		TBC.SetProgressState(TBPF_NOPROGRESS);
}



TaskBarPause::TaskBarPause()
{
	PrevState=TBC.GetProgressState();
	if(PrevState!=TBPF_ERROR && PrevState!=TBPF_PAUSED)
	{
		if(PrevState==TBPF_INDETERMINATE||PrevState==TBPF_NOPROGRESS)
		{
			TBC.SetProgressValue(1,1);
		}
		TBC.SetProgressState(TBPF_PAUSED);
		TBC.Flash();
	}
}

TaskBarPause::~TaskBarPause()
{
	TBC.SetProgressState(PrevState);
}



TaskBarError::TaskBarError()
{
	PrevState=TBC.GetProgressState();
	if(PrevState!=TBPF_ERROR && PrevState!=TBPF_PAUSED)
	{
		if(PrevState==TBPF_INDETERMINATE||PrevState==TBPF_NOPROGRESS)
		{
			TBC.SetProgressValue(1,1);
		}
		TBC.SetProgressState(TBPF_ERROR);
		TBC.Flash();
	}
}

TaskBarError::~TaskBarError()
{
	TBC.SetProgressState(PrevState);
}
