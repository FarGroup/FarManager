/*
console.cpp

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

#include "console.hpp"

#include "imports.hpp"
#include "config.hpp"
#include "colormix.hpp"
#include "interf.hpp"
#include "setcolor.hpp"
#include "strmix.hpp"
#include "exception.hpp"
#include "global.hpp"

#include "common/enum_substrings.hpp"
#include "common/function_traits.hpp"
#include "common/io.hpp"
#include "common/range.hpp"
#include "common/scope_exit.hpp"

static void override_stream_buffers()
{
	std::ios::sync_with_stdio(false);

	static consolebuf
		BufIn,
		BufOut,
		BufErr,
		BufLog;

	auto Color = colors::ConsoleColorToFarColor(F_LIGHTRED);
	colors::make_transparent(Color.BackgroundColor);
	BufErr.color(Color);

	static const io::wstreambuf_override
		In(std::wcin, BufIn),
		Out(std::wcout, BufOut),
		Err(std::wcerr, BufErr),
		Log(std::wclog, BufLog);
}

namespace console_detail
{
	// пишем/читаем порциями по 32 K, иначе проблемы.
	const unsigned int MAXSIZE = 32768;

	class external_console
	{
	public:
		NONCOPYABLE(external_console);
		external_console() = default;

		struct ModuleImports
		{
		private:
			os::rtdl::module m_Module{ L"extendedconsole.dll"sv };

		public:
#define DECLARE_IMPORT_FUNCTION(name, ...) os::rtdl::function_pointer<__VA_ARGS__> p ## name{ m_Module, #name }

			DECLARE_IMPORT_FUNCTION(ReadOutput,           BOOL(WINAPI*)(FAR_CHAR_INFO* Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT* ReadRegion));
			DECLARE_IMPORT_FUNCTION(WriteOutput,          BOOL(WINAPI*)(const FAR_CHAR_INFO* Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT* WriteRegion));
			DECLARE_IMPORT_FUNCTION(Commit,               BOOL(WINAPI*)());
			DECLARE_IMPORT_FUNCTION(GetTextAttributes,    BOOL(WINAPI*)(FarColor* Attributes));
			DECLARE_IMPORT_FUNCTION(SetTextAttributes,    BOOL(WINAPI*)(const FarColor* Attributes));
			DECLARE_IMPORT_FUNCTION(ClearExtraRegions,    BOOL(WINAPI*)(const FarColor* Color, int Mode));
			DECLARE_IMPORT_FUNCTION(GetColorDialog,       BOOL(WINAPI*)(FarColor* Color, BOOL Centered, BOOL AddTransparent));

#undef DECLARE_IMPORT_FUNCTION
		}
		Imports;
	};

	static nifty_counter::buffer<external_console> Storage;
	static auto& ExternalConsole = reinterpret_cast<external_console&>(Storage);

	console::console():
		m_OriginalInputHandle(GetStdHandle(STD_INPUT_HANDLE))
	{
		placement::construct(ExternalConsole);

		override_stream_buffers();
	}

	console::~console()
	{
		if (m_FileHandle != -1)
			_close(m_FileHandle);

		placement::destruct(ExternalConsole);
	}

	bool console::Allocate() const
	{
		return AllocConsole() != FALSE;
	}

	bool console::Free() const
	{
		return FreeConsole() != FALSE;
	}

	HANDLE console::GetInputHandle() const
	{
		return GetStdHandle(STD_INPUT_HANDLE);
	}

	HANDLE console::GetOutputHandle() const
	{
		return GetStdHandle(STD_OUTPUT_HANDLE);
	}

	HANDLE console::GetErrorHandle() const
	{
		return GetStdHandle(STD_ERROR_HANDLE);
	}

	HANDLE console::GetOriginalInputHandle() const
	{
		return m_OriginalInputHandle;
	}

	HWND console::GetWindow() const
	{
		return GetConsoleWindow();
	}

	bool console::GetSize(COORD& Size) const
	{
		bool Result = false;
		CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
		if (GetConsoleScreenBufferInfo(GetOutputHandle(), &ConsoleScreenBufferInfo))
		{
			if (Global->Opt->WindowMode)
			{
				Size.X = ConsoleScreenBufferInfo.srWindow.Right - ConsoleScreenBufferInfo.srWindow.Left + 1;
				Size.Y = ConsoleScreenBufferInfo.srWindow.Bottom - ConsoleScreenBufferInfo.srWindow.Top + 1;
			}
			else
			{
				Size = ConsoleScreenBufferInfo.dwSize;
			}
			Result = true;
		}
		return Result;
	}

	bool console::SetSize(COORD Size) const
	{
		if (!Global->Opt->WindowMode)
			return SetConsoleScreenBufferSize(GetOutputHandle(), Size) != FALSE;

		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
		csbi.srWindow.Left = 0;
		csbi.srWindow.Right = Size.X - 1;
		csbi.srWindow.Bottom = csbi.dwSize.Y - 1;
		csbi.srWindow.Top = csbi.srWindow.Bottom - (Size.Y - 1);
		COORD WindowCoord = { static_cast<SHORT>(csbi.srWindow.Right - csbi.srWindow.Left + 1), static_cast<SHORT>(csbi.srWindow.Bottom - csbi.srWindow.Top + 1) };
		if (WindowCoord.X > csbi.dwSize.X || WindowCoord.Y > csbi.dwSize.Y)
		{
			WindowCoord.X = std::max(WindowCoord.X, csbi.dwSize.X);
			WindowCoord.Y = std::max(WindowCoord.Y, csbi.dwSize.Y);
			SetConsoleScreenBufferSize(GetOutputHandle(), WindowCoord);

			if (WindowCoord.X > csbi.dwSize.X)
			{
				// windows sometimes uses existing colors to init right region of screen buffer
				FarColor Color;
				GetTextAttributes(Color);
				ClearExtraRegions(Color, CR_RIGHT);
			}
		}

		return SetWindowRect(csbi.srWindow);
	}

	bool console::GetWindowRect(SMALL_RECT& ConsoleWindow) const
	{
		CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
		if (!GetConsoleScreenBufferInfo(GetOutputHandle(), &ConsoleScreenBufferInfo))
			return false;

		ConsoleWindow = ConsoleScreenBufferInfo.srWindow;
		return true;
	}

	bool console::SetWindowRect(const SMALL_RECT& ConsoleWindow) const
	{
		return SetConsoleWindowInfo(GetOutputHandle(), true, &ConsoleWindow) != FALSE;
	}

	bool console::GetWorkingRect(SMALL_RECT& WorkingRect) const
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi))
			return false;

		WorkingRect.Bottom = csbi.dwSize.Y - 1;
		WorkingRect.Left = 0;
		WorkingRect.Right = WorkingRect.Left + ScrX;
		WorkingRect.Top = WorkingRect.Bottom - ScrY;
		return true;
	}

	string console::GetPhysicalTitle() const
	{
		// Don't use GetConsoleTitle here, it's buggy.
		string Title;
		os::GetWindowText(GetWindow(), Title);
		return Title;
	}

	string console::GetTitle() const
	{
		return m_Title;
	}

	bool console::SetTitle(const string& Title) const
	{
		m_Title = Title;
		return SetConsoleTitle(Title.c_str()) != FALSE;
	}

	bool console::GetKeyboardLayoutName(string &strName) const
	{
		wchar_t Buffer[KL_NAMELENGTH];
		if (!imports.GetConsoleKeyboardLayoutNameW(Buffer))
			return false;

		strName = Buffer;
		return true;
	}

	uintptr_t console::GetInputCodepage() const
	{
		return GetConsoleCP();
	}

	bool console::SetInputCodepage(uintptr_t Codepage) const
	{
		return SetConsoleCP(Codepage) != FALSE;
	}

	uintptr_t console::GetOutputCodepage() const
	{
		return GetConsoleOutputCP();
	}

	bool console::SetOutputCodepage(uintptr_t Codepage) const
	{
		return SetConsoleOutputCP(Codepage) != FALSE;
	}

	bool console::SetControlHandler(PHANDLER_ROUTINE HandlerRoutine, bool Add) const
	{
		return SetConsoleCtrlHandler(HandlerRoutine, Add) != FALSE;
	}

	bool console::GetMode(HANDLE ConsoleHandle, DWORD& Mode) const
	{
		return GetConsoleMode(ConsoleHandle, &Mode) != FALSE;
	}

	bool console::SetMode(HANDLE ConsoleHandle, DWORD Mode) const
	{
		return SetConsoleMode(ConsoleHandle, Mode) != FALSE;
	}

	static void AdjustMouseEvents(INPUT_RECORD* Buffer, size_t Length, short Delta, short MaxX)
	{
		for (auto& i : make_range(Buffer, Length))
		{
			if (i.EventType == MOUSE_EVENT)
			{
				i.Event.MouseEvent.dwMousePosition.Y = std::max(0, i.Event.MouseEvent.dwMousePosition.Y - Delta);
				i.Event.MouseEvent.dwMousePosition.X = std::min(i.Event.MouseEvent.dwMousePosition.X, MaxX);
			}
		}
	}

	bool console::PeekInput(INPUT_RECORD* Buffer, size_t Length, size_t& NumberOfEventsRead) const
	{
		DWORD dwNumberOfEventsRead = 0;
		bool Result = PeekConsoleInput(GetInputHandle(), Buffer, static_cast<DWORD>(Length), &dwNumberOfEventsRead) != FALSE;
		NumberOfEventsRead = dwNumberOfEventsRead;
		if (Global->Opt->WindowMode)
		{
			COORD Size = {};
			GetSize(Size);
			AdjustMouseEvents(Buffer, NumberOfEventsRead, GetDelta(), Size.X - 1);
		}
		return Result;
	}

	bool console::ReadInput(INPUT_RECORD* Buffer, size_t Length, size_t& NumberOfEventsRead) const
	{
		DWORD dwNumberOfEventsRead = 0;
		if (!ReadConsoleInput(GetInputHandle(), Buffer, static_cast<DWORD>(Length), &dwNumberOfEventsRead))
			return false;

		NumberOfEventsRead = dwNumberOfEventsRead;

		if (Global->Opt->WindowMode)
		{
			COORD Size = {};
			GetSize(Size);
			AdjustMouseEvents(Buffer, NumberOfEventsRead, GetDelta(), Size.X - 1);
		}

		return true;
	}

	bool console::WriteInput(INPUT_RECORD* Buffer, size_t Length, size_t& NumberOfEventsWritten) const
	{
		if (Global->Opt->WindowMode)
		{
			const auto Delta = GetDelta();

			for (auto& i : make_range(Buffer, Length))
			{
				if (i.EventType == MOUSE_EVENT)
				{
					i.Event.MouseEvent.dwMousePosition.Y += Delta;
				}
			}
		}
		DWORD dwNumberOfEventsWritten = 0;
		bool Result = WriteConsoleInput(GetInputHandle(), Buffer, static_cast<DWORD>(Length), &dwNumberOfEventsWritten) != FALSE;
		NumberOfEventsWritten = dwNumberOfEventsWritten;
		return Result;
	}

	bool console::ReadOutput(matrix<FAR_CHAR_INFO>& Buffer, COORD BufferCoord, SMALL_RECT& ReadRegion) const
	{
		if (ExternalConsole.Imports.pReadOutput)
		{
			const COORD SizeCoord = { static_cast<SHORT>(Buffer.width()), static_cast<SHORT>(Buffer.height()) };
			return ExternalConsole.Imports.pReadOutput(Buffer.data(), SizeCoord, BufferCoord, &ReadRegion) != FALSE;
		}

		int Delta = Global->Opt->WindowMode? GetDelta() : 0;
		ReadRegion.Top += Delta;
		ReadRegion.Bottom += Delta;

		COORD BufferSize = { static_cast<SHORT>(Buffer.width()), static_cast<SHORT>(Buffer.height()) };

		// skip unused region
		const size_t Offset = BufferCoord.Y * BufferSize.X;
		matrix<CHAR_INFO> ConsoleBuffer(BufferSize.Y - BufferCoord.Y, BufferSize.X);

		BufferSize.Y -= BufferCoord.Y;
		BufferCoord.Y = 0;

		if (BufferSize.X*BufferSize.Y * sizeof(CHAR_INFO) > MAXSIZE)
		{
			const auto SavedY = BufferSize.Y;
			BufferSize.Y = std::max(static_cast<int>(MAXSIZE / (BufferSize.X * sizeof(CHAR_INFO))), 1);
			size_t Height = ReadRegion.Bottom - ReadRegion.Top + 1;
			int Start = ReadRegion.Top;
			SMALL_RECT SavedReadRegion = ReadRegion;
			for (size_t i = 0; i < Height; i += BufferSize.Y)
			{
				ReadRegion = SavedReadRegion;
				ReadRegion.Top = static_cast<SHORT>(Start + i);
				if (!ReadConsoleOutput(GetOutputHandle(), ConsoleBuffer[i].data(), BufferSize, BufferCoord, &ReadRegion))
					return false;
			}
			BufferSize.Y = SavedY;
		}
		else
		{
			if (!ReadConsoleOutput(GetOutputHandle(), ConsoleBuffer.data(), BufferSize, BufferCoord, &ReadRegion))
				return false;
		}

		auto& ConsoleBufferVector = ConsoleBuffer.vector();
		std::transform(ALL_CONST_RANGE(ConsoleBufferVector), Buffer.data() + Offset, [](const auto& i)
		{
			return FAR_CHAR_INFO{ i.Char.UnicodeChar, colors::ConsoleColorToFarColor(i.Attributes) };
		});

		if (Global->Opt->WindowMode)
		{
			ReadRegion.Top -= Delta;
			ReadRegion.Bottom -= Delta;
		}

		return true;
	}

	bool console::WriteOutput(const matrix<FAR_CHAR_INFO>& Buffer, COORD BufferCoord, SMALL_RECT& WriteRegion) const
	{
		if (ExternalConsole.Imports.pWriteOutput)
		{
			const COORD BufferSize = { static_cast<SHORT>(Buffer.width()), static_cast<SHORT>(Buffer.height()) };
			return ExternalConsole.Imports.pWriteOutput(Buffer.data(), BufferSize, BufferCoord, &WriteRegion) != FALSE;
		}

		int Delta = Global->Opt->WindowMode? GetDelta() : 0;
		WriteRegion.Top += Delta;
		WriteRegion.Bottom += Delta;

		COORD BufferSize = { static_cast<SHORT>(Buffer.width()), static_cast<SHORT>(Buffer.height()) };

		// skip unused region
		const size_t Offset = BufferCoord.Y * BufferSize.X;
		const size_t Size = BufferSize.X * (BufferSize.Y - BufferCoord.Y);

		matrix<CHAR_INFO> ConsoleBuffer(BufferSize.Y - BufferCoord.Y, BufferSize.X);
		std::transform(Buffer.data() + Offset, Buffer.data() + Offset + Size, ConsoleBuffer.data(), [](const auto& i)
		{
			return CHAR_INFO{ i.Char, colors::FarColorToConsoleColor(i.Attributes) };
		});

		BufferSize.Y -= BufferCoord.Y;
		BufferCoord.Y = 0;

		if (BufferSize.X*BufferSize.Y * sizeof(CHAR_INFO) > MAXSIZE)
		{
			const auto SavedY = BufferSize.Y;
			BufferSize.Y = static_cast<SHORT>(std::max(static_cast<int>(MAXSIZE / (BufferSize.X * sizeof(CHAR_INFO))), 1));
			size_t Height = WriteRegion.Bottom - WriteRegion.Top + 1;
			int Start = WriteRegion.Top;
			SMALL_RECT SavedWriteRegion = WriteRegion;
			for (size_t i = 0; i < Height; i += BufferSize.Y)
			{
				WriteRegion = SavedWriteRegion;
				WriteRegion.Top = static_cast<SHORT>(Start + i);
				if (!WriteConsoleOutput(GetOutputHandle(), ConsoleBuffer[i].data(), BufferSize, BufferCoord, &WriteRegion))
					return false;
			}
			BufferSize.Y = SavedY;
		}
		else
		{
			if (!WriteConsoleOutput(GetOutputHandle(), ConsoleBuffer.data(), BufferSize, BufferCoord, &WriteRegion))
				return false;
		}

		if (Global->Opt->WindowMode)
		{
			WriteRegion.Top -= Delta;
			WriteRegion.Bottom -= Delta;
		}

		return true;
	}

	bool console::Read(std::vector<wchar_t>& Buffer, size_t& Size) const
	{
		const auto InputHandle = GetInputHandle();

		DWORD NumberOfCharsRead;

		DWORD Mode;
		if (GetMode(InputHandle, Mode))
		{
			if (!ReadConsole(InputHandle, Buffer.data(), static_cast<DWORD>(Buffer.size()), &NumberOfCharsRead, nullptr))
				return false;
		}
		else
		{
			if (!ReadFile(InputHandle, Buffer.data(), static_cast<DWORD>(Buffer.size() * sizeof(wchar_t)), &NumberOfCharsRead, nullptr))
				return false;

			NumberOfCharsRead /= sizeof(wchar_t);
		}

		Size = NumberOfCharsRead;
		return true;
	}

	bool console::Write(const string_view Str) const
	{
		DWORD NumberOfCharsWritten;
		const auto OutputHandle = GetOutputHandle();

		DWORD Mode;
		if (GetMode(OutputHandle, Mode))
			return WriteConsole(OutputHandle, Str.data(), static_cast<DWORD>(Str.size()), &NumberOfCharsWritten, nullptr) != FALSE;

		// Redirected output

		if (m_FileHandle == -1)
		{
			HANDLE OsHandle;
			if (!DuplicateHandle(GetCurrentProcess(), OutputHandle, GetCurrentProcess(), &OsHandle, 0, FALSE, DUPLICATE_SAME_ACCESS))
				return false;

			m_FileHandle = _open_osfhandle(reinterpret_cast<intptr_t>(OsHandle), _O_U8TEXT);
			if (m_FileHandle == -1)
				return false;

			_setmode(m_FileHandle, _O_U8TEXT);
		}

		return _write(m_FileHandle, Str.data(), static_cast<unsigned int>(Str.size() * sizeof(wchar_t))) != -1;
	}

	bool console::Commit() const
	{
		if (ExternalConsole.Imports.pCommit)
			return ExternalConsole.Imports.pCommit() != FALSE;

		// reserved
		return true;
	}

	bool console::GetTextAttributes(FarColor& Attributes) const
	{
		if (ExternalConsole.Imports.pGetTextAttributes)
			return ExternalConsole.Imports.pGetTextAttributes(&Attributes) != FALSE;

		CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
		if (!GetConsoleScreenBufferInfo(GetOutputHandle(), &ConsoleScreenBufferInfo))
			return false;

		Attributes = colors::ConsoleColorToFarColor(ConsoleScreenBufferInfo.wAttributes);
		return true;
	}

	bool console::SetTextAttributes(const FarColor& Attributes) const
	{
		if (ExternalConsole.Imports.pSetTextAttributes)
			return ExternalConsole.Imports.pSetTextAttributes(&Attributes) != FALSE;

		return SetConsoleTextAttribute(GetOutputHandle(), colors::FarColorToConsoleColor(Attributes)) != FALSE;
	}

	bool console::GetCursorInfo(CONSOLE_CURSOR_INFO& ConsoleCursorInfo) const
	{
		return GetConsoleCursorInfo(GetOutputHandle(), &ConsoleCursorInfo) != FALSE;
	}

	bool console::SetCursorInfo(const CONSOLE_CURSOR_INFO& ConsoleCursorInfo) const
	{
		return SetConsoleCursorInfo(GetOutputHandle(), &ConsoleCursorInfo) != FALSE;
	}

	bool console::GetCursorPosition(COORD& Position) const
	{
		CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
		if (!GetConsoleScreenBufferInfo(GetOutputHandle(), &ConsoleScreenBufferInfo))
			return false;

		Position = ConsoleScreenBufferInfo.dwCursorPosition;
		if (Global->Opt->WindowMode)
		{
			Position.Y -= GetDelta();
		}
		return true;
	}

	bool console::SetCursorPosition(COORD Position) const
	{
		if (Global->Opt->WindowMode)
		{
			ResetPosition();
			COORD Size = {};
			GetSize(Size);
			Position.X = std::min(Position.X, static_cast<SHORT>(Size.X - 1));
			Position.Y = std::max(static_cast<SHORT>(0), Position.Y);
			Position.Y += GetDelta();
		}
		return SetConsoleCursorPosition(GetOutputHandle(), Position) != FALSE;
	}

	bool console::FlushInputBuffer() const
	{
		return FlushConsoleInputBuffer(GetInputHandle()) != FALSE;
	}

	bool console::GetNumberOfInputEvents(size_t& NumberOfEvents) const
	{
		DWORD dwNumberOfEvents = 0;
		bool Result = GetNumberOfConsoleInputEvents(GetInputHandle(), &dwNumberOfEvents) != FALSE;
		NumberOfEvents = dwNumberOfEvents;
		return Result;
	}

	bool console::GetAlias(string_view const Source, wchar_t* TargetBuffer, size_t TargetBufferLength, string_view const ExeName) const
	{
		return GetConsoleAlias(const_cast<LPWSTR>(null_terminated(Source).c_str()), TargetBuffer, static_cast<DWORD>(TargetBufferLength), const_cast<LPWSTR>(null_terminated(ExeName).c_str())) != 0;
	}

	std::unordered_map<string, std::unordered_map<string, string>> console::GetAllAliases() const
	{
		FN_RETURN_TYPE(console::GetAllAliases) Result;

		const auto ExeLength = GetConsoleAliasExesLength();
		if (!ExeLength)
			return Result;

		std::vector<wchar_t> ExeBuffer(ExeLength / sizeof(wchar_t) + 1); // +1 for double \0
		if (!GetConsoleAliasExes(ExeBuffer.data(), ExeLength))
			return Result;

		std::vector<wchar_t> AliasesBuffer;
		for (const auto& ExeToken : enum_substrings(ExeBuffer.data()))
		{
			// It's ok, ExeToken is guaranteed to be null-terminated
			const auto ExeNamePtr = const_cast<wchar_t*>(ExeToken.data());
			const auto AliasesLength = GetConsoleAliasesLength(ExeNamePtr);
			AliasesBuffer.resize(AliasesLength / sizeof(wchar_t) + 1); // +1 for double \0
			if (!GetConsoleAliases(AliasesBuffer.data(), AliasesLength, ExeNamePtr))
				continue;

			auto& ExeMap = Result[ExeNamePtr];
			for (const auto& AliasToken : enum_substrings(AliasesBuffer.data()))
			{
				const auto Pair = split_name_value(AliasToken);
				ExeMap.emplace(string(Pair.first), string(Pair.second));
			}
		}

		return Result;
	}

	void console::SetAllAliases(const std::unordered_map<string, std::unordered_map<string, string>>& Aliases) const
	{
		for (const auto& ExeItem : Aliases)
		{
			for (const auto& AliasesItem : ExeItem.second)
			{
				AddConsoleAlias(const_cast<wchar_t*>(AliasesItem.first.c_str()), const_cast<wchar_t*>(AliasesItem.second.c_str()), const_cast<wchar_t*>(ExeItem.first.c_str()));
			}
		}
	}

	bool console::GetDisplayMode(DWORD& Mode) const
	{
		return GetConsoleDisplayMode(&Mode) != FALSE;
	}

	COORD console::GetLargestWindowSize() const
	{
		COORD Result = GetLargestConsoleWindowSize(GetOutputHandle());
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
		if (csbi.dwSize.Y > Result.Y)
		{
			CONSOLE_FONT_INFO FontInfo;
			if (GetCurrentConsoleFont(GetOutputHandle(), FALSE, &FontInfo))
			{
				// in XP FontInfo.dwFontSize contains something else than size in pixels.
				FontInfo.dwFontSize = GetConsoleFontSize(GetOutputHandle(), FontInfo.nFont);
				Result.X -= Round(static_cast<SHORT>(GetSystemMetrics(SM_CXVSCROLL)), FontInfo.dwFontSize.X);
			}
		}
		return Result;
	}

	bool console::SetActiveScreenBuffer(HANDLE ConsoleOutput) const
	{
		return SetConsoleActiveScreenBuffer(ConsoleOutput) != FALSE;
	}

	bool console::ClearExtraRegions(const FarColor& Color, int Mode) const
	{
		if (ExternalConsole.Imports.pClearExtraRegions)
			return ExternalConsole.Imports.pClearExtraRegions(&Color, Mode) != FALSE;

		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
		DWORD CharsWritten;
		WORD ConColor = colors::FarColorToConsoleColor(Color);

		if (Mode&CR_TOP)
		{
			DWORD TopSize = csbi.dwSize.X*csbi.srWindow.Top;
			COORD TopCoord = {};
			FillConsoleOutputCharacter(GetOutputHandle(), L' ', TopSize, TopCoord, &CharsWritten);
			FillConsoleOutputAttribute(GetOutputHandle(), ConColor, TopSize, TopCoord, &CharsWritten);
		}

		if (Mode&CR_RIGHT)
		{
			DWORD RightSize = csbi.dwSize.X - csbi.srWindow.Right;
			COORD RightCoord = { csbi.srWindow.Right,GetDelta() };
			for (; RightCoord.Y < csbi.dwSize.Y; RightCoord.Y++)
			{
				FillConsoleOutputCharacter(GetOutputHandle(), L' ', RightSize, RightCoord, &CharsWritten);
				FillConsoleOutputAttribute(GetOutputHandle(), ConColor, RightSize, RightCoord, &CharsWritten);
			}
		}
		return true;
	}

	bool console::ScrollWindow(int Lines, int Columns) const
	{
		bool process = false;
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);

		if ((Lines < 0 && csbi.srWindow.Top) || (Lines > 0 && csbi.srWindow.Bottom != csbi.dwSize.Y - 1))
		{
			csbi.srWindow.Top += Lines;
			csbi.srWindow.Bottom += Lines;

			if (csbi.srWindow.Top < 0)
			{
				csbi.srWindow.Bottom -= csbi.srWindow.Top;
				csbi.srWindow.Top = 0;
			}

			if (csbi.srWindow.Bottom >= csbi.dwSize.Y)
			{
				csbi.srWindow.Top -= (csbi.srWindow.Bottom - (csbi.dwSize.Y - 1));
				csbi.srWindow.Bottom = csbi.dwSize.Y - 1;
			}
			process = true;
		}

		if ((Columns < 0 && csbi.srWindow.Left) || (Columns > 0 && csbi.srWindow.Right != csbi.dwSize.X - 1))
		{
			csbi.srWindow.Left += Columns;
			csbi.srWindow.Right += Columns;

			if (csbi.srWindow.Left < 0)
			{
				csbi.srWindow.Right -= csbi.srWindow.Left;
				csbi.srWindow.Left = 0;
			}

			if (csbi.srWindow.Right >= csbi.dwSize.X)
			{
				csbi.srWindow.Left -= (csbi.srWindow.Right - (csbi.dwSize.X - 1));
				csbi.srWindow.Right = csbi.dwSize.X - 1;
			}
			process = true;
		}

		if (process)
		{
			SetWindowRect(csbi.srWindow);
			return true;
		}

		return false;
	}

	bool console::ScrollWindowToBegin() const
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);

		if (csbi.srWindow.Top > 0)
		{
			csbi.srWindow.Bottom -= csbi.srWindow.Top;
			csbi.srWindow.Top = 0;
			SetWindowRect(csbi.srWindow);
			return true;
		}

		return false;
	}

	bool console::ScrollWindowToEnd() const
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);

		if (csbi.srWindow.Bottom < csbi.dwSize.Y - 1)
		{
			csbi.srWindow.Top += csbi.dwSize.Y - 1 - csbi.srWindow.Bottom;
			csbi.srWindow.Bottom = csbi.dwSize.Y - 1;
			SetWindowRect(csbi.srWindow);
			return true;
		}

		return false;
	}

	bool console::IsFullscreenSupported() const
	{
#ifdef _WIN64
		return false;
#else
		CONSOLE_SCREEN_BUFFER_INFOEX csbiex{ sizeof(csbiex) };
		if (imports.GetConsoleScreenBufferInfoEx(GetOutputHandle(), &csbiex))
			return csbiex.bFullscreenSupported != FALSE;

		return true;
#endif
	}

	bool console::ResetPosition() const
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
		if (csbi.srWindow.Left || csbi.srWindow.Bottom != csbi.dwSize.Y - 1)
		{
			csbi.srWindow.Right -= csbi.srWindow.Left;
			csbi.srWindow.Left = 0;
			csbi.srWindow.Top += csbi.dwSize.Y - 1 - csbi.srWindow.Bottom;
			csbi.srWindow.Bottom = csbi.dwSize.Y - 1;
			SetWindowRect(csbi.srWindow);
		}
		return true;
	}

	bool console::GetColorDialog(FarColor& Color, bool Centered, bool AddTransparent) const
	{
		if (ExternalConsole.Imports.pGetColorDialog)
			return ExternalConsole.Imports.pGetColorDialog(&Color, Centered, AddTransparent) != FALSE;

		return GetColorDialogInternal(Color, Centered, AddTransparent);
	}

	short console::GetDelta() const
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
		return csbi.dwSize.Y - (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
	}

	bool console::ScrollScreenBuffer(const SMALL_RECT& ScrollRectangle, const SMALL_RECT* ClipRectangle, COORD DestinationOrigin, const FAR_CHAR_INFO& Fill) const
	{
		const CHAR_INFO SysFill = { Fill.Char, colors::FarColorToConsoleColor(Fill.Attributes) };
		return ScrollConsoleScreenBuffer(GetOutputHandle(), &ScrollRectangle, ClipRectangle, DestinationOrigin, &SysFill) != FALSE;
	}

	bool console::ScrollNonClientArea(size_t NumLines, const FAR_CHAR_INFO& Fill) const
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi))
			return false;

		SMALL_RECT ScrollRectangle = { 0, 0, static_cast<SHORT>(csbi.dwSize.X - 1), static_cast<SHORT>(csbi.dwSize.Y - (ScrY + 1) - 1) };
		COORD DestinationOigin = { 0, static_cast<SHORT>(-static_cast<SHORT>(NumLines)) };
		return ScrollScreenBuffer(ScrollRectangle, nullptr, DestinationOigin, Fill) != FALSE;
	}
}

NIFTY_DEFINE(console_detail::console, console);

enum
{
	BufferSize = 10240
};

consolebuf::consolebuf():
	m_InBuffer(BufferSize),
	m_OutBuffer(BufferSize)

{
	setg(m_InBuffer.data(), m_InBuffer.data() + m_InBuffer.size(), m_InBuffer.data() + m_InBuffer.size());
	setp(m_OutBuffer.data(), m_OutBuffer.data() + m_OutBuffer.size());
}

void consolebuf::color(const FarColor& Color)
{
	m_Colour.first = Color;
	m_Colour.second = true;
}

consolebuf::int_type consolebuf::underflow()
{
	size_t Read;
	if (!console.Read(m_InBuffer, Read))
		throw MAKE_FAR_EXCEPTION(L"Console read error"sv);

	if (!Read)
		return traits_type::eof();

	setg(m_InBuffer.data(), m_InBuffer.data(), m_InBuffer.data() + Read);
	return m_InBuffer[0];
}

consolebuf::int_type consolebuf::overflow(consolebuf::int_type Ch)
{
	if (!Write({ pbase(), static_cast<size_t>(pptr() - pbase()) }))
		return traits_type::eof();

	setp(m_OutBuffer.data(), m_OutBuffer.data() + m_OutBuffer.size());

	if (traits_type::eq_int_type(Ch, traits_type::eof()))
	{
		console.Commit();
	}
	else
	{
		sputc(Ch);
	}

	return 0;
}

int consolebuf::sync()
{
	overflow(traits_type::eof());
	return 0;
}

bool consolebuf::Write(const string_view Str)
{
	if (Str.empty())
		return true;

	FarColor CurrentColor;
	const auto ChangeColour = m_Colour.second && console.GetTextAttributes(CurrentColor);

	if (ChangeColour)
	{
		console.SetTextAttributes(colors::merge(CurrentColor, m_Colour.first));
	}

	SCOPE_EXIT{ if (ChangeColour) console.SetTextAttributes(CurrentColor); };

	return console.Write(Str);
}
