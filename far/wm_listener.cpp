/*
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
#include "imports.hpp"
#include "notification.hpp"
#include "global.hpp"
#include "exception_handler.hpp"
#include "log.hpp"

// Platform:
#include "platform.hpp"
#include "platform.debug.hpp"
#include "platform.fs.hpp"

// Common:
#include "common.hpp"
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
						const auto& BroadcastHeader = view_as<DEV_BROADCAST_HDR>(lParam);
						if (BroadcastHeader.dbch_devicetype == DBT_DEVTYP_VOLUME)
						{
							const auto& BroadcastVolume = view_as<DEV_BROADCAST_VOLUME>(&BroadcastHeader);

							const auto Drives = BroadcastVolume.dbcv_unitmask;
							const auto IsArrival = wParam == DBT_DEVICEARRIVAL;
							const auto IsMedia = (BroadcastVolume.dbcv_flags & DBTF_MEDIA) != 0;

							LOGINFO(L"WM_DEVICECHANGE(type: {}, media: {}, drives: {})"sv, IsArrival? L"arrival"sv : L"removal"sv, IsMedia, join({}, os::fs::enum_drives(Drives)));

							message_manager::instance().notify(update_devices, update_devices_message{ Drives, IsArrival, IsMedia });
						}
					}
					break;
				}
			}
			break;

		case WM_SETTINGCHANGE:
			if (lParam)
			{
				// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-settingchange
				// Some applications send this message with lParam set to NULL
				const auto Area = NullToEmpty(std::bit_cast<const wchar_t*>(lParam));

				if (Area == L"Environment"sv)
				{
					LOGINFO(L"WM_SETTINGCHANGE(Environment)"sv);
					message_manager::instance().notify(update_environment);
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
				LOGINFO(L"WM_POWERBROADCAST(PBT_APMPOWERSTATUSCHANGE)"sv);
				message_manager::instance().notify(update_power);
				break;

			case PBT_POWERSETTINGCHANGE: // change percent
				{
					const auto& Data = view_as<POWERBROADCAST_SETTING>(lParam);
					if (Data.PowerSetting == GUID_BATTERY_PERCENTAGE_REMAINING)
					{
						LOGINFO(L"WM_POWERBROADCAST(PBT_POWERSETTINGCHANGE): {}% battery remaining"sv, view_as<DWORD>(Data.Data));
						message_manager::instance().notify(update_power);
					}
				}
				break;

			// TODO:
			// PBT_APMSUSPEND & PBT_APMRESUMEAUTOMATIC handlers

			}

			break;

		}
	},
	save_exception_to(*WndProcExceptionPtr)
	);

	return DefWindowProc(Hwnd, Msg, wParam, lParam);
}


void wm_listener::powernotify_deleter::operator()(HPOWERNOTIFY const Ptr) const
{
	if (!imports.UnregisterPowerSettingNotification)
		return;

	if (!imports.UnregisterPowerSettingNotification(Ptr))
		LOGWARNING(L"UnregisterPowerSettingNotification(): {}"sv, os::last_error());
}

// for PBT_POWERSETTINGCHANGE
void wm_listener::enable_power_notifications()
{
	if (!imports.RegisterPowerSettingNotification)
		return;

	assert(!m_PowerNotify);

	m_PowerNotify.reset(imports.RegisterPowerSettingNotification(m_Hwnd, &GUID_BATTERY_PERCENTAGE_REMAINING, DEVICE_NOTIFY_WINDOW_HANDLE));

	if (!m_PowerNotify)
		LOGWARNING(L"RegisterPowerSettingNotification(): {}"sv, os::last_error());
}

void wm_listener::disable_power_notifications()
{
	m_PowerNotify.reset();
}

wm_listener::wm_listener()
{
	os::event ReadyEvent(os::event::type::automatic, os::event::state::nonsignaled);
	m_Thread = os::thread(&wm_listener::WindowThreadRoutine, this, std::ref(ReadyEvent));
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

void wm_listener::WindowThreadRoutine(const os::event& ReadyEvent)
{
	os::debug::set_thread_name(L"Window messages processor");

	WNDCLASSEX wc{ sizeof(wc) };
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = L"FarHiddenWindowClass";

	if (UnregisterClass(wc.lpszClassName, nullptr))
		LOGWARNING(L"Class {} was already registered"sv, wc.lpszClassName);

	if (!RegisterClassEx(&wc))
	{
		LOGERROR(L"RegisterClassEx(): {}"sv, os::last_error());
		ReadyEvent.set();
		return;
	}

	SCOPE_EXIT
	{
		if (!UnregisterClass(wc.lpszClassName, {}))
			LOGWARNING(L"UnregisterClass(): {}"sv, os::last_error());
	};

	m_Hwnd = CreateWindowEx(0, wc.lpszClassName, nullptr, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, nullptr, nullptr);
	if (!m_Hwnd)
	{
		LOGERROR(L"CreateWindowEx(): {}"sv, os::last_error());
		ReadyEvent.set();
		return;
	}

	MSG Msg;
	WndProcExceptionPtr = &m_ExceptionPtr;

	ReadyEvent.set();

	while (!m_ExceptionPtr)
	{
		const auto Result = GetMessage(&Msg, nullptr, 0, 0);
		if (!Result)
			return;

		if (Result < 0)
		{
			LOGERROR(L"GetMessage(): {}"sv, os::last_error());
			return;
		}

		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
}
