/*
TaskBar.cpp

Windows 7 taskbar support
*/

#include "headers.hpp"
#pragma hdrstop

#include "TaskBar.hpp"
#include "global.hpp"

#ifdef __GNUC__
const IID IID_ITaskbarList3 = { 0xEA1AFB91, 0x9E28, 0x4B86, 0x90, 0xE9, 0x9E, 0x9F, 0x8A, 0x5E, 0xEF, 0xAF };
#endif

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
	static FLASHWINDOWEX pFlashWindowEx=NULL;
	if(!pFlashWindowEx)
	{
		HMODULE hUser32=GetModuleHandle("user32.dll");
		if(hUser32)
		{
			pFlashWindowEx=(FLASHWINDOWEX)GetProcAddress(hUser32,"FlashWindowEx");
		}
	}
	
	if(pFlashWindowEx)
	{
		WINDOWINFO WI={sizeof(WI)};
		if(GetWindowInfo(hFarWnd,&WI))
		{
			if(WI.dwWindowStatus!=WS_ACTIVECAPTION)
			{
				FLASHWINFO FWI={sizeof(FWI),hFarWnd,FLASHW_ALL|FLASHW_TIMERNOFG,0,0};
				pFlashWindowEx(&FWI);
			}
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
