#ifndef CONSOLE_HPP_DB857D87_FD76_4E96_A9EE_4C06712C6B6D
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

// Internal:
#include "plugin.hpp"

// Platform:

// Common:
#include "common/2d/matrix.hpp"
#include "common/2d/point.hpp"
#include "common/2d/rectangle.hpp"
#include "common/nifty_counter.hpp"
#include "common/range.hpp"

// External:

//----------------------------------------------------------------------------

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

		bool GetSize(point& Size) const;
		bool SetSize(point Size) const;

		bool SetScreenBufferSize(point Size) const;

		bool GetWindowRect(rectangle& ConsoleWindow) const;
		bool SetWindowRect(rectangle const& ConsoleWindow) const;

		bool GetWorkingRect(rectangle& WorkingRect) const;

		string GetPhysicalTitle() const;
		string GetTitle() const;
		bool SetTitle(string_view Title) const;

		bool GetKeyboardLayoutName(string &strName) const;

		uintptr_t GetInputCodepage() const;
		bool SetInputCodepage(uintptr_t Codepage) const;

		uintptr_t GetOutputCodepage() const;
		bool SetOutputCodepage(uintptr_t Codepage) const;

		bool SetControlHandler(PHANDLER_ROUTINE HandlerRoutine, bool Add) const;

		bool GetMode(HANDLE ConsoleHandle, DWORD& Mode) const;
		bool SetMode(HANDLE ConsoleHandle, DWORD Mode) const;

		bool IsVtSupported() const;

		bool PeekInput(span<INPUT_RECORD> Buffer, size_t& NumberOfEventsRead) const;
		bool PeekOneInput(INPUT_RECORD& Record) const;
		bool ReadInput(span<INPUT_RECORD> Buffer, size_t& NumberOfEventsRead) const;
		bool ReadOneInput(INPUT_RECORD& Record) const;
		bool WriteInput(span<INPUT_RECORD> Buffer, size_t& NumberOfEventsWritten) const;
		bool ReadOutput(matrix<FAR_CHAR_INFO>& Buffer, point BufferCoord, rectangle const& ReadRegionRelative) const;
		bool ReadOutput(matrix<FAR_CHAR_INFO>& Buffer, const rectangle& ReadRegion) const { return ReadOutput(Buffer, {}, ReadRegion); }
		bool WriteOutput(const matrix<FAR_CHAR_INFO>& Buffer, point BufferCoord, rectangle const& WriteRegionRelative) const;
		bool WriteOutput(const matrix<FAR_CHAR_INFO>& Buffer, rectangle const& WriteRegion) const { return WriteOutput(Buffer, {}, WriteRegion); }
		bool Read(string& Buffer, size_t& Size) const;
		bool Write(string_view Str) const;
		bool Commit() const;

		bool GetTextAttributes(FarColor& Attributes) const;
		bool SetTextAttributes(const FarColor& Attributes) const;

		bool GetCursorInfo(CONSOLE_CURSOR_INFO& ConsoleCursorInfo) const;
		bool SetCursorInfo(const CONSOLE_CURSOR_INFO& ConsoleCursorInfo) const;

		bool GetCursorPosition(point& Position) const;
		bool SetCursorPosition(point Position) const;

		bool FlushInputBuffer() const;

		bool GetNumberOfInputEvents(size_t& NumberOfEvents) const;

		bool GetAlias(string_view Source, wchar_t* TargetBuffer, size_t TargetBufferLength, string_view ExeName) const;

		std::unordered_map<string, std::unordered_map<string, string>> GetAllAliases() const;

		void SetAllAliases(const std::unordered_map<string, std::unordered_map<string, string>>& Aliases) const;

		bool GetDisplayMode(DWORD& Mode) const;

		point GetLargestWindowSize() const;

		bool SetActiveScreenBuffer(HANDLE ConsoleOutput) const;

		bool ClearExtraRegions(const FarColor& Color, int Mode) const;

		bool ScrollWindow(int Lines, int Columns = 0) const;

		bool ScrollWindowToBegin() const;

		bool ScrollWindowToEnd() const;

		bool IsFullscreenSupported() const;

		bool ResetPosition() const;
		bool ResetViewportPosition() const;

		bool GetColorDialog(FarColor& Color, bool Centered = false, const FarColor* BaseColor = nullptr) const;

		bool ScrollNonClientArea(size_t NumLines, const FAR_CHAR_INFO& Fill) const;

		bool IsViewportVisible() const;
		bool IsViewportShifted() const;
		bool IsPositionVisible(point Position) const;
		bool IsScrollbackPresent() const;

		bool GetPalette(std::array<COLORREF, 16>& Palette) const;

		static void EnableWindowMode(bool Value);
		static void EnableVirtualTerminal(bool Value);

	private:
		class implementation;
		friend class implementation;

		short GetDelta() const;
		bool ScrollScreenBuffer(rectangle const& ScrollRectangle, point DestinationOrigin, const FAR_CHAR_INFO& Fill) const;
		bool GetCursorRealPosition(point& Position) const;
		bool SetCursorRealPosition(point Position) const;

		HANDLE m_OriginalInputHandle;
		mutable string m_Title;
		mutable int m_FileHandle{ -1 };

		class stream_buffers_overrider;
		std::unique_ptr<stream_buffers_overrider> m_StreamBuffersOverrider;

		class temporary_stream_buffers_overrider
		{
		public:
			temporary_stream_buffers_overrider();
			~temporary_stream_buffers_overrider();

		private:
			std::unique_ptr<stream_buffers_overrider> m_StreamBuffersOverrider;
		};

	public:
		static std::unique_ptr<temporary_stream_buffers_overrider> create_temporary_stream_buffers_overrider();
	};
}

NIFTY_DECLARE(console_detail::console, console);

#endif // CONSOLE_HPP_DB857D87_FD76_4E96_A9EE_4C06712C6B6D
