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
							Arrival? Global->Window->MediaArivalEvent().Set() : Global->Window->MediaRemoveEvent().Set();
						}
						else
						{
							Arrival? Global->Window->DeviceArivalEvent().Set() : Global->Window->DeviceRemoveEvent().Set();
						}
					}
				}
				break;

			}
		}
		break;

	case WM_SETTINGCHANGE:
		if(Global->Opt->UpdateEnvironment && lParam && !StrCmp(reinterpret_cast<LPCWSTR>(lParam),L"Environment"))
		{
			Global->Window->EnvironmentChangeEvent().Set();
			break;
		}

	case WM_POWERBROADCAST:
		switch(wParam)
		{
		case PBT_APMPOWERSTATUSCHANGE: // change status

		case PBT_POWERSETTINGCHANGE:   // change percent
			Global->Window->PowerChangeEvent().Set();
			break;
		// TODO:
		// PBT_APMSUSPEND & PBT_APMRESUMEAUTOMATIC handlers

		}

		break;

	}
	return DefWindowProc(Hwnd, Msg, wParam, lParam);
}

WindowHandler::WindowHandler():
	m_Thread(nullptr),
	m_Hwnd(nullptr)
{
	Check();
}

WindowHandler::~WindowHandler()
{
	m_exitEvent.Set();
	if(m_Hwnd)
	{
		SendMessage(m_Hwnd,WM_CLOSE, 0, 0);
	}
	if(m_Thread)
	{
		WaitForSingleObject(m_Thread, INFINITE);
		CloseHandle(m_Thread);
	}
}

void WindowHandler::Check()
{
	if(!m_Thread || WaitForSingleObject(m_Thread, 0)!=WAIT_TIMEOUT)
	{
		m_Thread = apiCreateThread(nullptr, 0, this, &WindowHandler::WindowThreadRoutine, nullptr, 0, nullptr);
	}
}

unsigned int WindowHandler::WindowThreadRoutine(void* Param)
{
	WNDCLASSEX wc={sizeof(wc)};
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = L"FarHiddenWindowClass";
	UnregisterClass(wc.lpszClassName, 0);
	if(RegisterClassEx(&wc))
	{
		m_Hwnd = CreateWindowEx(0, wc.lpszClassName, nullptr, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, nullptr, nullptr);
		if(m_Hwnd)
		{
			// for PBT_POWERSETTINGCHANGE
			HPOWERNOTIFY hpn=Global->ifn->RegisterPowerSettingNotification(m_Hwnd,&GUID_BATTERY_PERCENTAGE_REMAINING,DEVICE_NOTIFY_WINDOW_HANDLE);

			MSG Msg;
			while(!m_exitEvent.Signaled() && GetMessage(&Msg, nullptr, 0, 0) > 0)
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}

			if (hpn) // for PBT_POWERSETTINGCHANGE
				Global->ifn->UnregisterPowerSettingNotification(hpn);

		}
		UnregisterClass(wc.lpszClassName, 0);
	}
	return 0;
}
