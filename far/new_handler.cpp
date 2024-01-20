/*
new_handler.cpp

*/
/*
Copyright © 2017 Far Group
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
#include "new_handler.hpp"

// Internal:
#include "encoding.hpp"
#include "farversion.hpp"
#include "log.hpp"

// Platform:
#include "platform.hpp"

// Common:
#include "common/compiler.hpp"
#include "common/scope_exit.hpp"

// External:

//----------------------------------------------------------------------------

// We don't use malloc directly, so setting the standard new handler only would do.
// However, thirdparty libs (e.g. SQLite) and the CRT itself do use malloc,
// so it's better to handle it.
#define SET_CRT_NEW_HANDLER 1

// operator new is implemented in terms of malloc, so handling both doesn't make much sense.
// Enable only if we switch to an external allocator.
#define SET_STD_NEW_HANDLER 0

static new_handler* NewHandler;

#if SET_CRT_NEW_HANDLER
#if IS_MICROSOFT_SDK()
// New.h pollutes global namespace and causes name conflicts
extern "C"
{
	crt_new_handler _set_new_handler(crt_new_handler Handler);
	int _set_new_mode(int NewMode);
}
#else
namespace
{
	// GCC can't statically link _set_new_handler & _set_new_mode due to incompatible name mangling.
	const auto
		crt_set_new_handler_name =
#ifdef _WIN64
			"?_set_new_handler@@YAP6AH_K@ZP6AH0@Z@Z",
#else
			"?_set_new_handler@@YAP6AHI@ZP6AHI@Z@Z",
#endif
		crt_set_new_mode_name =
			"?_set_new_mode@@YAHH@Z";
}

template<auto Function>
static decltype(Function) get_address(const char* const Name)
{
	if (!Name)
	{
		LOGWARNING(L"Required name is empty, check compilation settings"sv);
		return nullptr;
	}

	const auto Crt = GetModuleHandle(L"msvcrt.dll");
	if (!Crt)
	{
		LOGWARNING(L"GetModuleHandle(msvcrt): {}"sv, os::last_error());
		return nullptr;
	}

	const auto Ptr = GetProcAddress(Crt, Name);
	if (!Ptr)
	{
		LOGWARNING(L"GetProcAddress({}): {}"sv, encoding::ansi::get_chars(Name), os::last_error());
		return nullptr;
	}

	return std::bit_cast<decltype(Function)>(Ptr);
}

static crt_new_handler _set_new_handler(crt_new_handler const Handler)
{
	static const auto _set_new_handler_impl = get_address<&_set_new_handler>(crt_set_new_handler_name);
	if (!_set_new_handler_impl)
		return {};

	return _set_new_handler_impl(Handler);
}

static int _set_new_mode(int const NewMode)
{
	static const auto _set_new_mode_impl = get_address<&_set_new_mode>(crt_set_new_mode_name);
	if (!_set_new_mode_impl)
		return 0;

	return _set_new_mode_impl(NewMode);
}

#endif

static int invoke_crt_new_handler(size_t)
{
	return NewHandler && NewHandler->retry();
}
#endif

#if SET_STD_NEW_HANDLER
static void invoke_new_handler()
{
	if (!NewHandler || !NewHandler->retry())
		throw std::bad_alloc{};
}
#endif

namespace
{
	enum size
	{
		X = 80,
		Y = 25
	};
}

static os::handle create_ui(COORD const BufferSize)
{
	os::handle Screen{ CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, {}, CONSOLE_TEXTMODE_BUFFER, {}) };

	if (!Screen)
	{
		LOGWARNING(L"CreateConsoleScreenBuffer(): {}"sv, os::last_error());
		return {};
	}

	CONSOLE_SCREEN_BUFFER_INFO Info;
	if (!GetConsoleScreenBufferInfo(Screen.native_handle(), &Info))
	{
		LOGWARNING(L"GetConsoleScreenBufferInfo(): {}"sv, os::last_error());
		return {};
	}

	// The new and improved console is full of wonders.
	// You can't make the window size larger than the buffer.
	// And you can't make the buffer size smaller than the window.
	// So you have to make the buffer larger first, but not too large,
	// then set the window size to what you want, and then make
	// the buffer size smaller if needed.
	// This is insane.
	if (Info.dwSize.X < X || Info.dwSize.Y < Y)
	{
		const COORD CompatibleBufferSize
		{
			std::max(static_cast<short>(X), Info.dwSize.X),
			std::max(static_cast<short>(X), Info.dwSize.Y)
		};

		if (!SetConsoleScreenBufferSize(Screen.native_handle(), CompatibleBufferSize))
		{
			LOGWARNING(L"SetConsoleScreenBufferSize(): {}"sv, os::last_error());
			return {};
		}
	}

	if (const SMALL_RECT WindowInfo{ 0, 0, X - 1, Y - 1 }; !SetConsoleWindowInfo(Screen.native_handle(), true, &WindowInfo))
	{
		LOGWARNING(L"SetConsoleWindowInfo(): {}"sv, os::last_error());
		return {};
	}

	if (Info.dwSize.X > X || Info.dwSize.Y > Y)
	{
		if (!SetConsoleScreenBufferSize(Screen.native_handle(), { X, Y }))
		{
			LOGWARNING(L"SetConsoleScreenBufferSize(): {}"sv, os::last_error());
			return {};
		}
	}

	const auto WhiteOnBlue = 0x9F;
	if (!SetConsoleTextAttribute(Screen.native_handle(), WhiteOnBlue))
	{
		LOGWARNING(L"SetConsoleTextAttribute(): {}"sv, os::last_error());
		return {};
	}

	if (DWORD AttrWritten; !FillConsoleOutputAttribute(Screen.native_handle(), WhiteOnBlue, X * Y, { 0, 0 }, &AttrWritten))
	{
		LOGWARNING(L"FillConsoleOutputAttribute(): {}"sv, os::last_error());
		return {};
	}


	if (const CONSOLE_CURSOR_INFO cci{ 1 }; !SetConsoleCursorInfo(Screen.native_handle(), &cci))
	{
		LOGWARNING(L"SetConsoleCursorInfo(): {}"sv, os::last_error());
		return {};
	}

	string_view const Strings[]
	{
		build::version_string(),
		{},
		L"Not enough memory is available to complete this operation."sv,
		L"Press Enter to retry or Esc to continue..."sv
	};

	const auto Write = [&](string_view const Str, size_t const Y)
	{
		if (!SetConsoleCursorPosition(Screen.native_handle(), { static_cast<short>((BufferSize.X - Str.size()) / 2), static_cast<short>(Y) }))
		{
			LOGWARNING(L"SetConsoleCursorPosition(): {}"sv, os::last_error());
			return false;
		}

		DWORD CharWritten;
		if (!WriteConsole(Screen.native_handle(), Str.data(), static_cast<DWORD>(Str.size()), &CharWritten, nullptr) && CharWritten == Str.size())
		{
			LOGWARNING(L"WriteConsole(): {}"sv, os::last_error());
			return false;
		}

		return true;
	};

	auto InitialY = (BufferSize.Y - std::size(Strings)) / 2;
	for (const auto& Str : Strings)
	{
		if (!Write(Str, InitialY++))
			return {};
	}

	return Screen;
}

void new_handler::initialize_ui()
{
	m_Screen = create_ui(m_BufferSize);
}

new_handler::new_handler():
	m_BufferSize{ X, Y }
{
	initialize_ui();

	if (!m_Screen)
		return;

	NewHandler = this;

#if SET_STD_NEW_HANDLER
	m_OldHandler = std::set_new_handler(invoke_new_handler);
#else
	(void)m_OldHandler;
#endif

#if SET_CRT_NEW_HANDLER
	m_OldCrtHandler = _set_new_handler(invoke_crt_new_handler);
	m_OldCrtMode = _set_new_mode(1);
#else
	(void)m_OldCrtHandler;
	(void)m_OldCrtMode;
#endif
}

new_handler::~new_handler()
{
	if (!NewHandler)
		return;

#if SET_CRT_NEW_HANDLER
	_set_new_mode(m_OldCrtMode);
	_set_new_handler(m_OldCrtHandler);
#endif

#if SET_STD_NEW_HANDLER
	std::set_new_handler(m_OldHandler);
#endif

	NewHandler = nullptr;
}

bool new_handler::retry()
{
	const auto CurrentBuffer = GetStdHandle(STD_OUTPUT_HANDLE);

	while (!SetConsoleActiveScreenBuffer(m_Screen.native_handle()))
	{
		LOGWARNING(L"SetConsoleActiveScreenBuffer(): {}"sv, os::last_error());

		if (GetLastError() != ERROR_INVALID_HANDLE)
			return false;

		LOGINFO(L"Reinitializing");
		initialize_ui();
	}

	SCOPE_EXIT { SetConsoleActiveScreenBuffer(CurrentBuffer); };

	INPUT_RECORD ir{};
	const auto& KeyEvent = ir.Event.KeyEvent;

	do
	{
		DWORD Read;
		if (!ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &ir, 1, &Read) || Read != 1)
		{
			LOGWARNING(L"ReadConsoleInput(): {}"sv, os::last_error());
			return false;
		}
	}
	while (!(ir.EventType == KEY_EVENT && !KeyEvent.bKeyDown && (KeyEvent.wVirtualKeyCode == VK_RETURN || KeyEvent.wVirtualKeyCode == VK_ESCAPE)));

	return KeyEvent.wVirtualKeyCode != VK_ESCAPE;
}
