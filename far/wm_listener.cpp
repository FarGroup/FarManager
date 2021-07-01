﻿/*
wm_listener.cpp

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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "wm_listener.hpp"

// Internal:
#include "config.hpp"
#include "imports.hpp"
#include "notification.hpp"
#include "global.hpp"
#include "exception.hpp"
#include "exception_handler.hpp"
#include "log.hpp"

// Platform:

// Common:
#include "common/scope_exit.hpp"

// External:

//----------------------------------------------------------------------------

static std::exception_ptr* WndProcExceptionPtr;
static LRESULT CALLBACK WndProc(HWND Hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	cpp_try(
	[&]
	{
		switch (Msg)
		{
		case WM_CLOSE:
			DestroyWindow(Hwnd);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_DEVICECHANGE:
			{
				switch (wParam)
				{
				case DBT_DEVICEARRIVAL:
				case DBT_DEVICEREMOVECOMPLETE:
					{
						const auto& BroadcastHeader = *reinterpret_cast<const DEV_BROADCAST_HDR*>(lParam);
						if (BroadcastHeader.dbch_devicetype == DBT_DEVTYP_VOLUME)
						{
							LOGINFO(L"WM_DEVICECHANGE(DBT_DEVTYP_VOLUME)"sv);
							const auto& BroadcastVolume = *reinterpret_cast<const DEV_BROADCAST_VOLUME*>(&BroadcastHeader);
							message_manager::instance().notify(update_devices, update_devices_message
							{
								BroadcastVolume.dbcv_unitmask,
								wParam == DBT_DEVICEARRIVAL,
								(BroadcastVolume.dbcv_flags & DBTF_MEDIA) != 0
							});
						}
					}
					break;
				}
			}
			break;

		case WM_SETTINGCHANGE:
			if (lParam)
			{
				const auto Area = reinterpret_cast<const wchar_t*>(lParam);

				if (Area == L"Environment"sv)
				{
					if (Global->Opt->UpdateEnvironment)
					{
						LOGINFO(L"WM_SETTINGCHANGE(Environment)"sv);
						message_manager::instance().notify(update_environment);
					}
				}
				else if (Area == L"intl"sv)
				{
					LOGINFO(L"WM_SETTINGCHANGE(intl)"sv);
					message_manager::instance().notify(update_intl);
				}
			}
			break;

		case WM_POWERBROADCAST:
			switch (wParam)
			{
			case PBT_APMPOWERSTATUSCHANGE: // change status
			case PBT_POWERSETTINGCHANGE:   // change percent
				LOGINFO(L"WM_POWERBROADCAST"sv);
				message_manager::instance().notify(update_power);
				break;

			// TODO:
			// PBT_APMSUSPEND & PBT_APMRESUMEAUTOMATIC handlers

			}

			break;

		}
	},
	[]
	{
		SAVE_EXCEPTION_TO(*WndProcExceptionPtr);
	});

	return DefWindowProc(Hwnd, Msg, wParam, lParam);
}

wm_listener::wm_listener()
{
	os::event ReadyEvent(os::event::type::automatic, os::event::state::nonsignaled);
	m_Thread = os::thread(os::thread::mode::join, &wm_listener::WindowThreadRoutine, this, &ReadyEvent);
	ReadyEvent.wait();
}

wm_listener::~wm_listener()
{
	if(m_Hwnd)
	{
		SendMessage(m_Hwnd, WM_CLOSE, 0, 0);
	}
}

void wm_listener::Check()
{
	rethrow_if(m_ExceptionPtr);
}

void wm_listener::WindowThreadRoutine(const os::event* ReadyEvent)
{
	os::debug::set_thread_name(L"Window messages processor");

	WNDCLASSEX wc={sizeof(wc)};
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = L"FarHiddenWindowClass";
	UnregisterClass(wc.lpszClassName, nullptr);
	if (!RegisterClassEx(&wc))
	{
		LOGERROR(L"RegisterClassEx(): {}"sv, last_error());
		ReadyEvent->set();
		return;
	}

	SCOPE_EXIT{ UnregisterClass(wc.lpszClassName, nullptr); };

	m_Hwnd = CreateWindowEx(0, wc.lpszClassName, nullptr, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, nullptr, nullptr);
	if (!m_Hwnd)
	{
		LOGERROR(L"CreateWindowEx(): {}"sv, last_error());
		ReadyEvent->set();
		return;
	}

	// for PBT_POWERSETTINGCHANGE
	const auto hpn = imports.RegisterPowerSettingNotification?
		imports.RegisterPowerSettingNotification(m_Hwnd, &GUID_BATTERY_PERCENTAGE_REMAINING, DEVICE_NOTIFY_WINDOW_HANDLE) :
		nullptr;

	SCOPE_EXIT{ if (hpn) imports.UnregisterPowerSettingNotification(hpn); };

	MSG Msg;
	WndProcExceptionPtr = &m_ExceptionPtr;

	ReadyEvent->set();

	while (!m_ExceptionPtr)
	{
		const auto Result = GetMessage(&Msg, nullptr, 0, 0);
		if (!Result)
			return;

		if (Result < 0)
		{
			LOGERROR(L"GetMessage(): {}"sv, last_error());
			return;
		}

		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
}
