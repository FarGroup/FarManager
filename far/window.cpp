/*
window.cpp

Обработка оконных сообщений
*/
/*
Copyright © 2010 Far Group
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

#include "window.hpp"
#include "config.hpp"
#include "imports.hpp"

events Events;

LRESULT CALLBACK WndProc(HWND Hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_CLOSE:
		DestroyWindow(Hwnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_DEVICECHANGE:
		{
			bool Arrival=false;
			switch(wParam)
			{
			case DBT_DEVICEARRIVAL:
				Arrival=true;
			case DBT_DEVICEREMOVECOMPLETE:
				{

					PDEV_BROADCAST_HDR Pbh=reinterpret_cast<PDEV_BROADCAST_HDR>(lParam);
					if(Pbh->dbch_devicetype==DBT_DEVTYP_VOLUME)
					{
						PDEV_BROADCAST_VOLUME Pdv=reinterpret_cast<PDEV_BROADCAST_VOLUME>(Pbh);
						if(Pdv->dbcv_flags&DBTF_MEDIA)
						{
							Arrival?Events.MediaArivalEvent.Set():Events.MediaRemoveEvent.Set();
						}
						else
						{
							Arrival?Events.DeviceArivalEvent.Set():Events.DeviceRemoveEvent.Set();
						}
					}
				}
				break;

			}
		}
		break;

	case WM_SETTINGCHANGE:
		if(Opt.UpdateEnvironment && lParam && !StrCmp(reinterpret_cast<LPCWSTR>(lParam),L"Environment"))
		{
			Events.EnvironmentChangeEvent.Set();
			break;
		}

	case WM_POWERBROADCAST:
		switch(wParam)
		{
		case PBT_APMPOWERSTATUSCHANGE: // change status

		case PBT_POWERSETTINGCHANGE:   // change percent
			Events.PowerChangeEvent.Set();
			break;
		// TODO:
		// PBT_APMSUSPEND & PBT_APMRESUMEAUTOMATIC handlers

		}

		break;

	}
	return DefWindowProc(Hwnd, Msg, wParam, lParam);
}

unsigned int WINAPI WindowThreadRoutine(LPVOID Param)
{
	WNDCLASSEX wc={sizeof(wc)};
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = L"FarHiddenWindowClass";
	UnregisterClass(wc.lpszClassName, 0);
	if(RegisterClassEx(&wc))
	{
		HWND* pHwnd=static_cast<HWND*>(Param);
		*pHwnd=CreateWindowEx(0, wc.lpszClassName, nullptr, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, nullptr, nullptr);
		if(*pHwnd)
		{
			// for PBT_POWERSETTINGCHANGE
			HPOWERNOTIFY hpn=ifn.RegisterPowerSettingNotification(*pHwnd,&GUID_BATTERY_PERCENTAGE_REMAINING,DEVICE_NOTIFY_WINDOW_HANDLE);

			MSG Msg;
			while(GetMessage(&Msg, nullptr, 0, 0)>0)
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}

			if (hpn) // for PBT_POWERSETTINGCHANGE
				ifn.UnregisterPowerSettingNotification(hpn);

		}
		UnregisterClass(wc.lpszClassName, 0);
	}
	return 0;
}

WindowHandler::WindowHandler()
{
	Check();
}

WindowHandler::~WindowHandler()
{
	if(Hwnd)
	{
		SendMessage(Hwnd,WM_CLOSE, 0, 0);
	}
	if(Thread)
	{
		CloseHandle(Thread);
	}
}

void WindowHandler::Check()
{
	if(!Thread || WaitForSingleObject(Thread, 0)!=WAIT_TIMEOUT)
	{
		Thread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, WindowThreadRoutine, &Hwnd, 0, nullptr));
	}
}

WindowHandler Window;
