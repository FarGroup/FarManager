﻿#ifndef CONSOLE_HPP_DB857D87_FD76_4E96_A9EE_4C06712C6B6D
#define CONSOLE_HPP_DB857D87_FD76_4E96_A9EE_4C06712C6B6D
#pragma once

/*
console.hpp

Console functions
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

#include "matrix.hpp"
#include "plugin.hpp"

#include "common/nifty_counter.hpp"

enum CLEAR_REGION
{
	CR_TOP=0x1,
	CR_RIGHT=0x2,
	CR_BOTH=CR_TOP|CR_RIGHT,
};

namespace console_detail
{
	class console
	{
	public:
		NONCOPYABLE(console);

		console();
		~console();

		bool Allocate() const;
		bool Free() const;

		HANDLE GetInputHandle() const;
		HANDLE GetOutputHandle() const;
		HANDLE GetErrorHandle() const;

		HANDLE GetOriginalInputHandle() const;

		HWND GetWindow() const;

		bool GetSize(COORD& Size) const;
		bool SetSize(COORD Size) const;

		bool GetWindowRect(SMALL_RECT& ConsoleWindow) const;
		bool SetWindowRect(const SMALL_RECT& ConsoleWindow) const;

		bool GetWorkingRect(SMALL_RECT& WorkingRect) const;

		string GetPhysicalTitle() const;
		string GetTitle() const;
		bool SetTitle(const string& Title) const;

		bool GetKeyboardLayoutName(string &strName) const;

		uintptr_t GetInputCodepage() const;
		bool SetInputCodepage(uintptr_t Codepage) const;

		uintptr_t GetOutputCodepage() const;
		bool SetOutputCodepage(uintptr_t Codepage) const;

		bool SetControlHandler(PHANDLER_ROUTINE HandlerRoutine, bool Add) const;

		bool GetMode(HANDLE ConsoleHandle, DWORD& Mode) const;
		bool SetMode(HANDLE ConsoleHandle, DWORD Mode) const;

		bool PeekInput(INPUT_RECORD* Buffer, size_t Length, size_t& NumberOfEventsRead) const;
		bool ReadInput(INPUT_RECORD* Buffer, size_t Length, size_t& NumberOfEventsRead) const;
		bool WriteInput(INPUT_RECORD* Buffer, size_t Length, size_t& NumberOfEventsWritten) const;
		bool ReadOutput(matrix<FAR_CHAR_INFO>& Buffer, COORD BufferCoord, SMALL_RECT& ReadRegion) const;
		bool ReadOutput(matrix<FAR_CHAR_INFO>& Buffer, SMALL_RECT& ReadRegion) const { return ReadOutput(Buffer, {}, ReadRegion); }
		bool WriteOutput(const matrix<FAR_CHAR_INFO>& Buffer, COORD BufferCoord, SMALL_RECT& WriteRegion) const;
		bool WriteOutput(const matrix<FAR_CHAR_INFO>& Buffer, SMALL_RECT& WriteRegion) const { return WriteOutput(Buffer, {}, WriteRegion); }
		bool Read(std::vector<wchar_t>& Buffer, size_t& Size) const;
		bool Write(string_view Str) const;
		bool Commit() const;

		bool GetTextAttributes(FarColor& Attributes) const;
		bool SetTextAttributes(const FarColor& Attributes) const;

		bool GetCursorInfo(CONSOLE_CURSOR_INFO& ConsoleCursorInfo) const;
		bool SetCursorInfo(const CONSOLE_CURSOR_INFO& ConsoleCursorInfo) const;

		bool GetCursorPosition(COORD& Position) const;
		bool SetCursorPosition(COORD Position) const;

		bool FlushInputBuffer() const;

		bool GetNumberOfInputEvents(size_t& NumberOfEvents) const;

		bool GetAlias(string_view Source, wchar_t* TargetBuffer, size_t TargetBufferLength, string_view ExeName) const;

		std::unordered_map<string, std::unordered_map<string, string>> GetAllAliases() const;

		void SetAllAliases(const std::unordered_map<string, std::unordered_map<string, string>>& Aliases) const;

		bool GetDisplayMode(DWORD& Mode) const;

		COORD GetLargestWindowSize() const;

		bool SetActiveScreenBuffer(HANDLE ConsoleOutput) const;

		bool ClearExtraRegions(const FarColor& Color, int Mode) const;

		bool ScrollWindow(int Lines, int Columns = 0) const;

		bool ScrollWindowToBegin() const;

		bool ScrollWindowToEnd() const;

		bool IsFullscreenSupported() const;

		bool ResetPosition() const;

		bool GetColorDialog(FarColor& Color, bool Centered = false, bool AddTransparent = false) const;

		bool ScrollNonClientArea(size_t NumLines, const FAR_CHAR_INFO& Fill) const;

	private:
		short GetDelta() const;
		bool ScrollScreenBuffer(const SMALL_RECT& ScrollRectangle, const SMALL_RECT* ClipRectangle, COORD DestinationOrigin, const FAR_CHAR_INFO& Fill) const;

		HANDLE m_OriginalInputHandle;
		mutable string m_Title;
		mutable int m_FileHandle{ -1 };
	};
}

NIFTY_DECLARE(console_detail::console, console);

class consolebuf : public std::wstreambuf
{
public:
	NONCOPYABLE(consolebuf);

	consolebuf();

	void color(const FarColor& Color);

protected:
	int_type underflow() override;
	int_type overflow(int_type Ch) override;
	int sync() override;

private:
	bool Write(string_view Str);

	std::vector<wchar_t> m_InBuffer, m_OutBuffer;
	std::pair<FarColor, bool> m_Colour;
};

#endif // CONSOLE_HPP_DB857D87_FD76_4E96_A9EE_4C06712C6B6D
