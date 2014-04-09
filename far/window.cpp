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
#include "notification.hpp"

static const wchar_t* devices_notify = L"devices";
static const wchar_t* power_notify = L"power";
static const wchar_t* environment_notify = L"environment";
static const wchar_t* intl_notify = L"intl";

static LRESULT CALLBACK WndProc(HWND Hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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
			//bool Arrival=false;
			switch(wParam)
			{
			case DBT_DEVICEARRIVAL:
				//Arrival=true;
			case DBT_DEVICEREMOVECOMPLETE:
				{

					auto Pbh = reinterpret_cast<PDEV_BROADCAST_HDR>(lParam);
					if(Pbh->dbch_devicetype==DBT_DEVTYP_VOLUME)
					{
						// currently we don't care what actually happened, "just a notification" is OK

						//auto Pdv=reinterpret_cast<PDEV_BROADCAST_VOLUME>(Pbh);
						//bool Media = Pdv->dbcv_flags & DBTF_MEDIA != 0;
						Notifier().at(devices_notify).notify(std::make_unique<payload>());
					}
				}
				break;

			}
		}
		break;

	case WM_SETTINGCHANGE:
		if(lParam)
		{
			if (!StrCmp(reinterpret_cast<LPCWSTR>(lParam),L"Environment"))
			{
				if (Global->Opt->UpdateEnvironment) 
				{
					Notifier().at(environment_notify).notify(std::make_unique<payload>());
				}
			}
			else if (!StrCmp(reinterpret_cast<LPCWSTR>(lParam),L"intl"))
			{
				Notifier().at(intl_notify).notify(std::make_unique<payload>());
			}
		}
		break;

	case WM_POWERBROADCAST:
		switch(wParam)
		{
		case PBT_APMPOWERSTATUSCHANGE: // change status

		case PBT_POWERSETTINGCHANGE:   // change percent
			Notifier().at(power_notify).notify(std::make_unique<payload>());
			break;
		// TODO:
		// PBT_APMSUSPEND & PBT_APMRESUMEAUTOMATIC handlers

		}

		break;

	}
	return DefWindowProc(Hwnd, Msg, wParam, lParam);
}

window_handler::window_handler(notifier* owner):
	m_Owner(owner),
	m_Hwnd(nullptr)
{
	m_Owner->add(std::make_unique<notification>(devices_notify));
	m_Owner->add(std::make_unique<notification>(power_notify));
	m_Owner->add(std::make_unique<notification>(environment_notify));
	m_Owner->add(std::make_unique<notification>(intl_notify));

	m_exitEvent.Open();

	Check();
}

window_handler::~window_handler()
{
	m_exitEvent.Set();
	if(m_Hwnd)
	{
		SendMessage(m_Hwnd,WM_CLOSE, 0, 0);
	}
	if(m_Thread.Opened())
	{
		m_Thread.Wait();
	}
}

void window_handler::Check()
{
	if (!m_Thread.Opened() || m_Thread.Signaled())
	{
		m_Thread.Close();
		m_Thread.Start(&window_handler::WindowThreadRoutine, this);
	}
}

unsigned int window_handler::WindowThreadRoutine(void* Param)
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
			HPOWERNOTIFY hpn=Imports().RegisterPowerSettingNotification(m_Hwnd,&GUID_BATTERY_PERCENTAGE_REMAINING,DEVICE_NOTIFY_WINDOW_HANDLE);

			MSG Msg;
			while(!m_exitEvent.Signaled() && GetMessage(&Msg, nullptr, 0, 0) > 0)
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}

			if (hpn) // for PBT_POWERSETTINGCHANGE
				Imports().UnregisterPowerSettingNotification(hpn);

		}
		UnregisterClass(wc.lpszClassName, 0);
	}
	return 0;
}
