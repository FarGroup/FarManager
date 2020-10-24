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
#include "farversion.hpp"

// Platform:

// Common:
#include "common/scope_exit.hpp"

// External:

//----------------------------------------------------------------------------

static new_handler* NewHandler;

namespace
{
	enum size
	{
		X = 80,
		Y = 25
	};
}

new_handler::new_handler():
	m_BufferSize{ X, Y },
	m_Screen(CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr))
{
	if (!m_Screen)
		return;

	const COORD BufferSize{ X, Y };

	const SMALL_RECT WindowInfo{ 0, 0, X - 1, Y - 1 };
	if (!SetConsoleWindowInfo(m_Screen.native_handle(), true, &WindowInfo))
		return;

	if (!SetConsoleScreenBufferSize(m_Screen.native_handle(), BufferSize))
		return;

	const auto WhiteOnBlue = 0x9F;
	if (!SetConsoleTextAttribute(m_Screen.native_handle(), WhiteOnBlue))
		return;

	DWORD AttrWritten;
	if (!FillConsoleOutputAttribute(m_Screen.native_handle(), WhiteOnBlue, BufferSize.X * BufferSize.Y, { 0, 0 }, &AttrWritten))
		return;

	const CONSOLE_CURSOR_INFO cci{ 1 };
	if (!SetConsoleCursorInfo(m_Screen.native_handle(), &cci))
		return;

	const string_view Strings[]
	{
		build::version_string(),
		{},
		L"Not enough memory is available to complete this operation."sv,
		L"Press Enter to retry or Esc to continue..."sv
	};

	const auto Write = [this](const string_view Str, size_t Y)
	{
		SetConsoleCursorPosition(m_Screen.native_handle(), { static_cast<short>((m_BufferSize.X - Str.size()) / 2), static_cast<short>(Y) });
		DWORD CharWritten;
		return WriteConsole(m_Screen.native_handle(), Str.data(), static_cast<DWORD>(Str.size()), &CharWritten, nullptr) && CharWritten == Str.size();
	};

	auto InitialY = (m_BufferSize.Y - std::size(Strings)) / 2;
	for (const auto& Str : Strings)
	{
		if (!Write(Str, InitialY++))
			return;
	}

	NewHandler = this;
	m_OldHandler = std::set_new_handler(invoke_new_handler);
}

new_handler::~new_handler()
{
	if (!NewHandler)
		return;

	std::set_new_handler(m_OldHandler);
	NewHandler = nullptr;
}

bool new_handler::retry() const
{
	const auto CurrentBuffer = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!SetConsoleActiveScreenBuffer(m_Screen.native_handle()))
		return false;

	SCOPE_EXIT { SetConsoleActiveScreenBuffer(CurrentBuffer); };

	INPUT_RECORD ir{};
	const auto& KeyEvent = ir.Event.KeyEvent;

	do
	{
		DWORD Read;
		if (!ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &ir, 1, &Read) || Read != 1)
			return false;
	}
	while (!(ir.EventType == KEY_EVENT && !KeyEvent.bKeyDown && (KeyEvent.wVirtualKeyCode == VK_RETURN || KeyEvent.wVirtualKeyCode == VK_ESCAPE)));

	return KeyEvent.wVirtualKeyCode != VK_ESCAPE;
}

void invoke_new_handler()
{
	if (!NewHandler || !NewHandler->retry())
		throw std::bad_alloc{};
}
