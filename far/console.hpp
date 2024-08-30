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
#include "platform.hpp"

// Common:
#include "common/2d/matrix.hpp"
#include "common/2d/point.hpp"
#include "common/2d/rectangle.hpp"
#include "common/nifty_counter.hpp"

// External:

//----------------------------------------------------------------------------

enum CLEAR_REGION
{
	CR_TOP=0x1,
	CR_RIGHT=0x2,
	CR_BOTH=CR_TOP|CR_RIGHT,
};

wchar_t ReplaceControlCharacter(wchar_t Char);
bool sanitise_pair(FAR_CHAR_INFO& First, FAR_CHAR_INFO& Second);
bool get_console_screen_buffer_info(HANDLE ConsoleOutput, CONSOLE_SCREEN_BUFFER_INFO* ConsoleScreenBufferInfo);

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

		HKL GetKeyboardLayout() const;

		uintptr_t GetInputCodepage() const;
		bool SetInputCodepage(uintptr_t Codepage) const;

		uintptr_t GetOutputCodepage() const;
		bool SetOutputCodepage(uintptr_t Codepage) const;

		bool SetControlHandler(PHANDLER_ROUTINE HandlerRoutine, bool Add) const;

		bool GetMode(HANDLE ConsoleHandle, DWORD& Mode) const;
		bool SetMode(HANDLE ConsoleHandle, DWORD Mode) const;
		std::optional<DWORD> UpdateMode(HANDLE ConsoleHandle, DWORD ToSet, DWORD ToClear) const;

		bool IsVtSupported() const;

		bool PeekOneInput(INPUT_RECORD& Record) const;
		bool ReadOneInput(INPUT_RECORD& Record) const;
		bool WriteInput(std::span<INPUT_RECORD> Buffer, size_t& NumberOfEventsWritten) const;
		bool ReadOutput(matrix<FAR_CHAR_INFO>& Buffer, point BufferCoord, rectangle const& ReadRegionRelative) const;
		bool ReadOutput(matrix<FAR_CHAR_INFO>& Buffer, const rectangle& ReadRegion) const { return ReadOutput(Buffer, {}, ReadRegion); }
		bool WriteOutput(matrix<FAR_CHAR_INFO>& Buffer, point BufferCoord, rectangle const& WriteRegionRelative) const;
		bool WriteOutput(matrix<FAR_CHAR_INFO>& Buffer, rectangle const& WriteRegion) const { return WriteOutput(Buffer, {}, WriteRegion); }
		bool WriteOutputGather(matrix<FAR_CHAR_INFO>& Buffer, std::span<rectangle const> WriteRegions) const;
		bool Read(std::span<wchar_t> Buffer, size_t& Size) const;
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

		bool GetAlias(string_view Name, string& Value, string_view ExeName) const;

		struct console_aliases
		{
			console_aliases();
			~console_aliases();

			MOVE_CONSTRUCTIBLE(console_aliases);

			struct data;
			std::unique_ptr<data> m_Data;
		};

		console_aliases GetAllAliases() const;

		void SetAllAliases(console_aliases&& Aliases) const;

		bool GetDisplayMode(DWORD& Mode) const;

		point GetLargestWindowSize(HANDLE ConsoleOutput) const;

		bool SetActiveScreenBuffer(HANDLE ConsoleOutput);

		HANDLE GetActiveScreenBuffer() const;

		bool ClearExtraRegions(const FarColor& Color, int Mode) const;

		bool Clear(const FarColor& Color) const;

		bool ScrollWindow(int Lines, int Columns = 0) const;

		bool ScrollWindowToBegin() const;

		bool ScrollWindowToEnd() const;

		bool IsFullscreenSupported() const;

		bool ResetViewportPosition() const;

		bool ScrollNonClientArea(size_t NumLines, const FAR_CHAR_INFO& Fill) const;

		bool IsViewportVisible() const;
		bool IsViewportShifted() const;
		bool IsPositionVisible(point Position) const;
		bool IsScrollbackPresent() const;

		[[nodiscard]]
		bool IsVtActive() const;

		[[nodiscard]]
		bool ExternalRendererLoaded() const;

		[[nodiscard]]
		size_t GetWidthPreciseExpensive(char32_t Codepoint);
		void ClearWideCache();

		bool GetPalette(std::array<COLORREF, 256>& Palette) const;
		bool SetPalette(std::array<COLORREF, 256> const& Palette) const;

		static void EnableWindowMode(bool Value);
		static void EnableVirtualTerminal(bool Value);

		void set_progress_state(TBPFLAG State) const;
		void set_progress_value(TBPFLAG State, size_t Percent) const;

		void stash_output() const;
		void unstash_output(rectangle Coordinates) const;

		void start_prompt() const;
		void start_command() const;
		void start_output() const;
		void command_finished() const;
		void command_finished(int ExitCode) const;
		void command_not_found(string_view Command) const;

		[[nodiscard]]
		short GetDelta() const;

		class input_queue_inspector
		{
		public:
			bool search(function_ref<bool(INPUT_RECORD const&)> Predicate);

		private:
			std::vector<INPUT_RECORD> m_Buffer{ 256 };
		};

	private:
		class implementation;
		friend class implementation;

		[[nodiscard]]
		bool IsVtEnabled() const;
		bool ScrollScreenBuffer(rectangle const& ScrollRectangle, point DestinationOrigin, const FAR_CHAR_INFO& Fill) const;
		bool GetCursorRealPosition(point& Position) const;
		bool SetCursorRealPosition(point Position) const;

		bool send_vt_command(string_view Command) const;

		std::optional<KEY_EVENT_RECORD> queued() const;

		HANDLE m_OriginalInputHandle;
		HANDLE m_ActiveConsoleScreenBuffer{};
		mutable string m_Title;

		class stream_buffers_overrider;
		std::unique_ptr<stream_buffers_overrider> m_StreamBuffersOverrider;

		os::handle m_WidthTestScreen;

		KEY_EVENT_RECORD mutable m_QueuedKeys{};
	};
}

NIFTY_DECLARE(console_detail::console, console);

#endif // CONSOLE_HPP_DB857D87_FD76_4E96_A9EE_4C06712C6B6D
