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

#include "headers.hpp"
#pragma hdrstop

#include "console.hpp"
#include "imports.hpp"
#include "config.hpp"
#include "colormix.hpp"
#include "interf.hpp"
#include "setcolor.hpp"
#include "strmix.hpp"

class basic_console: public console, public singleton<basic_console>
{
	IMPLEMENTS_SINGLETON(basic_console);

public:
virtual bool Allocate() const override
{
	return AllocConsole()!=FALSE;
}

virtual bool Free() const override
{
	return FreeConsole()!=FALSE;
}

virtual HANDLE GetInputHandle() const override
{
	return GetStdHandle(STD_INPUT_HANDLE);
}

virtual HANDLE GetOutputHandle() const override
{
	return GetStdHandle(STD_OUTPUT_HANDLE);
}

virtual HANDLE GetErrorHandle() const override
{
	return GetStdHandle(STD_ERROR_HANDLE);
}

virtual HANDLE GetOriginalInputHandle() const override
{
	return m_OriginalInputHandle;
}

virtual HWND GetWindow() const override
{
	return GetConsoleWindow();
}

virtual bool GetSize(COORD& Size) const override
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

virtual bool SetSize(COORD Size) const override
{
	if(!Global->Opt->WindowMode)
		return SetConsoleScreenBufferSize(GetOutputHandle(), Size) != FALSE;

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	csbi.srWindow.Left=0;
	csbi.srWindow.Right=Size.X-1;
	csbi.srWindow.Bottom=csbi.dwSize.Y-1;
	csbi.srWindow.Top=csbi.srWindow.Bottom-(Size.Y-1);
	COORD WindowCoord={static_cast<SHORT>(csbi.srWindow.Right-csbi.srWindow.Left+1), static_cast<SHORT>(csbi.srWindow.Bottom-csbi.srWindow.Top+1)};
	if(WindowCoord.X>csbi.dwSize.X || WindowCoord.Y>csbi.dwSize.Y)
	{
		WindowCoord.X=std::max(WindowCoord.X,csbi.dwSize.X);
		WindowCoord.Y=std::max(WindowCoord.Y,csbi.dwSize.Y);
		SetConsoleScreenBufferSize(GetOutputHandle(), WindowCoord);

		if(WindowCoord.X>csbi.dwSize.X)
		{
			// windows sometimes uses existing colors to init right region of screen buffer
			FarColor Color;
			Console().GetTextAttributes(Color);
			Console().ClearExtraRegions(Color, CR_RIGHT);
		}
	}

	return SetWindowRect(csbi.srWindow);
}

virtual bool GetWindowRect(SMALL_RECT& ConsoleWindow) const override
{
	CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
	if (!GetConsoleScreenBufferInfo(GetOutputHandle(), &ConsoleScreenBufferInfo))
		return false;

	ConsoleWindow=ConsoleScreenBufferInfo.srWindow;
	return true;
}

virtual bool SetWindowRect(const SMALL_RECT& ConsoleWindow) const override
{
	return SetConsoleWindowInfo(GetOutputHandle(), true, &ConsoleWindow)!=FALSE;
}

virtual bool GetWorkingRect(SMALL_RECT& WorkingRect) const override
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

virtual string GetPhysicalTitle() const override
{
	// Don't use GetConsoleTitle here, it's buggy.
	string Title;
	os::GetWindowText(GetWindow(), Title);
	return Title;
}

virtual string GetTitle() const override
{
	return m_Title;
}

virtual bool SetTitle(const string& Title) const override
{
	m_Title = Title;
	return SetConsoleTitle(Title.data())!=FALSE;
}

virtual bool GetKeyboardLayoutName(string &strName) const override
{
	wchar_t Buffer[KL_NAMELENGTH];
	if (!Imports().GetConsoleKeyboardLayoutNameW(Buffer))
		return false;

	strName = Buffer;
	return true;
}

virtual uintptr_t GetInputCodepage() const override
{
	return GetConsoleCP();
}

virtual bool SetInputCodepage(uintptr_t Codepage) const override
{
	return SetConsoleCP(Codepage)!=FALSE;
}

virtual uintptr_t GetOutputCodepage() const override
{
	return GetConsoleOutputCP();
}

virtual bool SetOutputCodepage(uintptr_t Codepage) const override
{
	return SetConsoleOutputCP(Codepage)!=FALSE;
}

virtual bool SetControlHandler(PHANDLER_ROUTINE HandlerRoutine, bool Add) const override
{
	return SetConsoleCtrlHandler(HandlerRoutine, Add)!=FALSE;
}

virtual bool GetMode(HANDLE ConsoleHandle, DWORD& Mode) const override
{
	return GetConsoleMode(ConsoleHandle, &Mode)!=FALSE;
}

virtual bool SetMode(HANDLE ConsoleHandle, DWORD Mode) const override
{
	return SetConsoleMode(ConsoleHandle, Mode)!=FALSE;
}

static void AdjustMouseEvents(INPUT_RECORD* Buffer, size_t Length, short Delta, short MaxX)
{
	for (auto& i: make_range(Buffer, Length))
	{
		if (i.EventType == MOUSE_EVENT)
		{
			i.Event.MouseEvent.dwMousePosition.Y = std::max(0, i.Event.MouseEvent.dwMousePosition.Y - Delta);
			i.Event.MouseEvent.dwMousePosition.X = std::min(i.Event.MouseEvent.dwMousePosition.X, MaxX);
		}
	}
}

virtual bool PeekInput(INPUT_RECORD* Buffer, size_t Length, size_t& NumberOfEventsRead) const override
{
	DWORD dwNumberOfEventsRead = 0;
	bool Result=PeekConsoleInput(GetInputHandle(), Buffer, static_cast<DWORD>(Length), &dwNumberOfEventsRead)!=FALSE;
	NumberOfEventsRead = dwNumberOfEventsRead;
	if(Global->Opt->WindowMode)
	{
		COORD Size = {};
		GetSize(Size);
		AdjustMouseEvents(Buffer, NumberOfEventsRead, GetDelta(), Size.X - 1);
	}
	return Result;
}

virtual bool ReadInput(INPUT_RECORD* Buffer, size_t Length, size_t& NumberOfEventsRead) const override
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

virtual bool WriteInput(INPUT_RECORD* Buffer, size_t Length, size_t& NumberOfEventsWritten) const override
{
	if(Global->Opt->WindowMode)
	{
		const auto Delta = GetDelta();

		for (auto& i: make_range(Buffer, Length))
		{
			if (i.EventType == MOUSE_EVENT)
			{
				i.Event.MouseEvent.dwMousePosition.Y += Delta;
			}
		}
	}
	DWORD dwNumberOfEventsWritten = 0;
	bool Result = WriteConsoleInput(GetInputHandle(), Buffer, static_cast<DWORD>(Length), &dwNumberOfEventsWritten)!=FALSE;
	NumberOfEventsWritten = dwNumberOfEventsWritten;
	return Result;
}

virtual bool ReadOutput(matrix<FAR_CHAR_INFO>& Buffer, COORD BufferCoord, SMALL_RECT& ReadRegion) const override
{
	int Delta=Global->Opt->WindowMode?GetDelta():0;
	ReadRegion.Top+=Delta;
	ReadRegion.Bottom+=Delta;

	COORD BufferSize = { static_cast<SHORT>(Buffer.width()), static_cast<SHORT>(Buffer.height()) };

	// skip unused region
	const size_t Offset = BufferCoord.Y * BufferSize.X;
	matrix<CHAR_INFO> ConsoleBuffer(BufferSize.Y - BufferCoord.Y, BufferSize.X);
	
	BufferSize.Y-=BufferCoord.Y;
	BufferCoord.Y=0;

	if(BufferSize.X*BufferSize.Y*sizeof(CHAR_INFO)>MAXSIZE)
	{
		const auto SavedY = BufferSize.Y;
		BufferSize.Y = std::max(static_cast<int>(MAXSIZE/(BufferSize.X*sizeof(CHAR_INFO))),1);
		size_t Height = ReadRegion.Bottom - ReadRegion.Top + 1;
		int Start=ReadRegion.Top;
		SMALL_RECT SavedReadRegion=ReadRegion;
		for(size_t i = 0; i < Height; i += BufferSize.Y)
		{
			ReadRegion=SavedReadRegion;
			ReadRegion.Top = static_cast<SHORT>(Start+i);
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

	if(Global->Opt->WindowMode)
	{
		ReadRegion.Top-=Delta;
		ReadRegion.Bottom-=Delta;
	}

	return true;
}

virtual bool WriteOutput(const matrix<FAR_CHAR_INFO>& Buffer, COORD BufferCoord, SMALL_RECT& WriteRegion) const override
{
	int Delta=Global->Opt->WindowMode?GetDelta():0;
	WriteRegion.Top+=Delta;
	WriteRegion.Bottom+=Delta;

	COORD BufferSize = { static_cast<SHORT>(Buffer.width()), static_cast<SHORT>(Buffer.height()) };

	// skip unused region
	const size_t Offset = BufferCoord.Y * BufferSize.X;
	const size_t Size = BufferSize.X * (BufferSize.Y - BufferCoord.Y);

	matrix<CHAR_INFO> ConsoleBuffer(BufferSize.Y - BufferCoord.Y, BufferSize.X);
	std::transform(Buffer.data() + Offset, Buffer.data() + Offset + Size, ConsoleBuffer.data(), [](const auto& i)
	{
		return CHAR_INFO{i.Char, colors::FarColorToConsoleColor(i.Attributes)};
	});

	BufferSize.Y-=BufferCoord.Y;
	BufferCoord.Y=0;

	if(BufferSize.X*BufferSize.Y*sizeof(CHAR_INFO)>MAXSIZE)
	{
		const auto SavedY = BufferSize.Y;
		BufferSize.Y=static_cast<SHORT>(std::max(static_cast<int>(MAXSIZE/(BufferSize.X*sizeof(CHAR_INFO))),1));
		size_t Height = WriteRegion.Bottom - WriteRegion.Top + 1;
		int Start=WriteRegion.Top;
		SMALL_RECT SavedWriteRegion=WriteRegion;
		for (size_t i = 0; i < Height; i += BufferSize.Y)
		{
			WriteRegion=SavedWriteRegion;
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

	if(Global->Opt->WindowMode)
	{
		WriteRegion.Top-=Delta;
		WriteRegion.Bottom-=Delta;
	}

	return true;
}

virtual bool Write(const string_view& Str) const override
{
	DWORD NumberOfCharsWritten;
	const auto OutputHandle = GetOutputHandle();

	DWORD Mode;
	return GetMode(OutputHandle, Mode)?
	       WriteConsole(OutputHandle, Str.data(), static_cast<DWORD>(Str.size()), &NumberOfCharsWritten, nullptr) != FALSE :
	       WriteFile(OutputHandle, Str.data(), static_cast<DWORD>(Str.size() * sizeof(wchar_t)), &NumberOfCharsWritten, nullptr) != FALSE;
}

virtual bool Commit() const override
{
	// reserved
	return true;
}

virtual bool GetTextAttributes(FarColor& Attributes) const override
{
	CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
	if (!GetConsoleScreenBufferInfo(GetOutputHandle(), &ConsoleScreenBufferInfo))
		return false;

	Attributes = colors::ConsoleColorToFarColor(ConsoleScreenBufferInfo.wAttributes);
	return true;
}

virtual bool SetTextAttributes(const FarColor& Attributes) const override
{
	return SetConsoleTextAttribute(GetOutputHandle(), colors::FarColorToConsoleColor(Attributes))!=FALSE;
}

virtual bool GetCursorInfo(CONSOLE_CURSOR_INFO& ConsoleCursorInfo) const override
{
	return GetConsoleCursorInfo(GetOutputHandle(), &ConsoleCursorInfo)!=FALSE;
}

virtual bool SetCursorInfo(const CONSOLE_CURSOR_INFO& ConsoleCursorInfo) const override
{
	return SetConsoleCursorInfo(GetOutputHandle(), &ConsoleCursorInfo)!=FALSE;
}

virtual bool GetCursorPosition(COORD& Position) const override
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

virtual bool SetCursorPosition(COORD Position) const override
{
	if(Global->Opt->WindowMode)
	{
		ResetPosition();
		COORD Size={};
		GetSize(Size);
		Position.X=std::min(Position.X,static_cast<SHORT>(Size.X-1));
		Position.Y=std::max(static_cast<SHORT>(0),Position.Y);
		Position.Y+=GetDelta();
	}
	return SetConsoleCursorPosition(GetOutputHandle(), Position)!=FALSE;
}

virtual bool FlushInputBuffer() const override
{
	return FlushConsoleInputBuffer(GetInputHandle())!=FALSE;
}

virtual bool GetNumberOfInputEvents(size_t& NumberOfEvents) const override
{
	DWORD dwNumberOfEvents = 0;
	bool Result = GetNumberOfConsoleInputEvents(GetInputHandle(), &dwNumberOfEvents)!=FALSE;
	NumberOfEvents = dwNumberOfEvents;
	return Result;
}

virtual bool GetAlias(LPCWSTR Source, LPWSTR TargetBuffer, size_t TargetBufferLength, LPCWSTR ExeName) const override
{
	return GetConsoleAlias(const_cast<LPWSTR>(Source), TargetBuffer, static_cast<DWORD>(TargetBufferLength), const_cast<LPWSTR>(ExeName))!=0;
}

virtual std::unordered_map<string, std::unordered_map<string, string>> GetAllAliases() const override
{
	FN_RETURN_TYPE(console::GetAllAliases) Result;

	const auto ExeLength = GetConsoleAliasExesLength();
	if (!ExeLength)
		return Result;

	std::vector<wchar_t> ExeBuffer(ExeLength / sizeof(wchar_t) + 1); // +1 for double \0
	if (!GetConsoleAliasExes(ExeBuffer.data(), ExeLength))
		return Result;

	std::vector<wchar_t> AliasesBuffer;
	for (const auto& ExeToken: enum_substrings(ExeBuffer.data()))
	{
		const auto ExeNamePtr = const_cast<wchar_t*>(ExeToken.data());
		const auto AliasesLength = GetConsoleAliasesLength(ExeNamePtr);
		AliasesBuffer.resize(AliasesLength / sizeof(wchar_t) + 1); // +1 for double \0
		if (!GetConsoleAliases(AliasesBuffer.data(), AliasesLength, ExeNamePtr))
			continue;

		auto& ExeMap = Result[ExeNamePtr];
		for (const auto& AliasToken: enum_substrings(AliasesBuffer.data()))
		{
			auto Pair = split_name_value(AliasToken);
			ExeMap.emplace(std::move(Pair.first), std::move(Pair.second));
		}
	}

	return Result;
}

virtual void SetAllAliases(const std::unordered_map<string, std::unordered_map<string, string>>& Aliases) const override
{
	for (const auto& ExeItem: Aliases)
	{
		for (const auto& AliasesItem: ExeItem.second)
		{
			AddConsoleAlias(const_cast<wchar_t*>(AliasesItem.first.data()), const_cast<wchar_t*>(AliasesItem.second.data()), const_cast<wchar_t*>(ExeItem.first.data()));
		}
	}
}

virtual bool GetDisplayMode(DWORD& Mode) const override
{
	return GetConsoleDisplayMode(&Mode)!=FALSE;
}

virtual COORD GetLargestWindowSize() const override
{
	COORD Result = GetLargestConsoleWindowSize(GetOutputHandle());
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	if(csbi.dwSize.Y > Result.Y)
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

virtual bool SetActiveScreenBuffer(HANDLE ConsoleOutput) const override
{
	return SetConsoleActiveScreenBuffer(ConsoleOutput)!=FALSE;
}

virtual bool ClearExtraRegions(const FarColor& Color, int Mode) const override
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	DWORD CharsWritten;
	WORD ConColor = colors::FarColorToConsoleColor(Color);

	if(Mode&CR_TOP)
	{
		DWORD TopSize = csbi.dwSize.X*csbi.srWindow.Top;
		COORD TopCoord = {};
		FillConsoleOutputCharacter(GetOutputHandle(), L' ', TopSize, TopCoord, &CharsWritten);
		FillConsoleOutputAttribute(GetOutputHandle(), ConColor, TopSize, TopCoord, &CharsWritten );
	}

	if(Mode&CR_RIGHT)
	{
		DWORD RightSize = csbi.dwSize.X-csbi.srWindow.Right;
		COORD RightCoord={csbi.srWindow.Right,GetDelta()};
		for(; RightCoord.Y<csbi.dwSize.Y; RightCoord.Y++)
		{
			FillConsoleOutputCharacter(GetOutputHandle(), L' ', RightSize, RightCoord, &CharsWritten);
			FillConsoleOutputAttribute(GetOutputHandle(), ConColor, RightSize, RightCoord, &CharsWritten);
		}
	}
	return true;
}

virtual bool ScrollWindow(int Lines,int Columns) const override
{
	bool process=false;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);

	if((Lines<0 && csbi.srWindow.Top) || (Lines>0 && csbi.srWindow.Bottom!=csbi.dwSize.Y-1))
	{
		csbi.srWindow.Top+=Lines;
		csbi.srWindow.Bottom+=Lines;

		if(csbi.srWindow.Top<0)
		{
			csbi.srWindow.Bottom-=csbi.srWindow.Top;
			csbi.srWindow.Top=0;
		}

		if(csbi.srWindow.Bottom>=csbi.dwSize.Y)
		{
			csbi.srWindow.Top-=(csbi.srWindow.Bottom-(csbi.dwSize.Y-1));
			csbi.srWindow.Bottom=csbi.dwSize.Y-1;
		}
		process=true;
	}

	if((Columns<0 && csbi.srWindow.Left) || (Columns>0 && csbi.srWindow.Right!=csbi.dwSize.X-1))
	{
		csbi.srWindow.Left+=Columns;
		csbi.srWindow.Right+=Columns;

		if(csbi.srWindow.Left<0)
		{
			csbi.srWindow.Right-=csbi.srWindow.Left;
			csbi.srWindow.Left=0;
		}

		if(csbi.srWindow.Right>=csbi.dwSize.X)
		{
			csbi.srWindow.Left-=(csbi.srWindow.Right-(csbi.dwSize.X-1));
			csbi.srWindow.Right=csbi.dwSize.X-1;
		}
		process=true;
	}

	if (process)
	{
		SetWindowRect(csbi.srWindow);
		return true;
	}

	return false;
}

virtual bool ScrollWindowToBegin() const override
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);

	if(csbi.srWindow.Top > 0)
	{
		csbi.srWindow.Bottom-=csbi.srWindow.Top;
		csbi.srWindow.Top=0;
		SetWindowRect(csbi.srWindow);
		return true;
	}

	return false;
}

virtual bool ScrollWindowToEnd() const override
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);

	if(csbi.srWindow.Bottom < csbi.dwSize.Y - 1)
	{
		csbi.srWindow.Top += csbi.dwSize.Y - 1 - csbi.srWindow.Bottom;
		csbi.srWindow.Bottom = csbi.dwSize.Y - 1;
		SetWindowRect(csbi.srWindow);
		return true;
	}

	return false;
}

virtual bool IsFullscreenSupported() const override
{
#ifdef _WIN64
	return false;
#else
	CONSOLE_SCREEN_BUFFER_INFOEX csbiex{ sizeof(csbiex) };
	if(Imports().GetConsoleScreenBufferInfoEx(GetOutputHandle(), &csbiex))
		return csbiex.bFullscreenSupported != FALSE;

	return true;
#endif
}

virtual bool ResetPosition() const override
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	if(csbi.srWindow.Left || csbi.srWindow.Bottom!=csbi.dwSize.Y-1)
	{
		csbi.srWindow.Right-=csbi.srWindow.Left;
		csbi.srWindow.Left=0;
		csbi.srWindow.Top+=csbi.dwSize.Y-1-csbi.srWindow.Bottom;
		csbi.srWindow.Bottom=csbi.dwSize.Y-1;
		SetWindowRect(csbi.srWindow);
	}
	return true;
}

virtual bool GetColorDialog(FarColor& Color, bool Centered, bool AddTransparent) const override
{
	return GetColorDialogInternal(Color, Centered, AddTransparent);
}

virtual short GetDelta() const
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	return csbi.dwSize.Y-(csbi.srWindow.Bottom-csbi.srWindow.Top+1);
}

protected:

virtual bool ScrollScreenBuffer(const SMALL_RECT& ScrollRectangle, const SMALL_RECT* ClipRectangle, COORD DestinationOrigin, const FAR_CHAR_INFO& Fill) const override
{
	const CHAR_INFO SysFill = { Fill.Char, colors::FarColorToConsoleColor(Fill.Attributes) };
	return ScrollConsoleScreenBuffer(GetOutputHandle(), &ScrollRectangle, ClipRectangle, DestinationOrigin, &SysFill) != FALSE;
}

protected:
	basic_console():
		// пишем/читаем порциями по 32 K, иначе проблемы.
		MAXSIZE(0x8000),
		m_OriginalInputHandle(GetStdHandle(STD_INPUT_HANDLE))
	{
	}

private:
	const unsigned int MAXSIZE;
	HANDLE m_OriginalInputHandle;
	mutable string m_Title;
};

bool console::ScrollNonClientArea(size_t NumLines, const FAR_CHAR_INFO& Fill) const
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi))
		return false;

	SMALL_RECT ScrollRectangle = { 0, 0, static_cast<SHORT>(csbi.dwSize.X - 1), static_cast<SHORT>(csbi.dwSize.Y - (ScrY + 1) - 1) };
	COORD DestinationOigin = { 0, static_cast<SHORT>(-static_cast<SHORT>(NumLines)) };
	return ScrollScreenBuffer(ScrollRectangle, nullptr, DestinationOigin, Fill) != FALSE;
}


class extended_console: public basic_console, public singleton<extended_console>
{
	IMPLEMENTS_SINGLETON(extended_console);

public:
	using singleton<extended_console>::instance;

	virtual bool ReadOutput(matrix<FAR_CHAR_INFO>& Buffer, COORD BufferCoord, SMALL_RECT& ReadRegion) const override
	{
		if(!Imports.pReadOutput)
			return basic_console::ReadOutput(Buffer, BufferCoord, ReadRegion);

		const COORD SizeCoord = { static_cast<SHORT>(Buffer.width()), static_cast<SHORT>(Buffer.height()) };
		return Imports.pReadOutput(Buffer.data(), SizeCoord, BufferCoord, &ReadRegion) != FALSE;
	}

	virtual bool WriteOutput(const matrix<FAR_CHAR_INFO>& Buffer, COORD BufferCoord, SMALL_RECT& WriteRegion) const override
	{
		if(!Imports.pWriteOutput)
			return basic_console::WriteOutput(Buffer, BufferCoord, WriteRegion);

		const COORD BufferSize = { static_cast<SHORT>(Buffer.width()), static_cast<SHORT>(Buffer.height()) };
		return Imports.pWriteOutput(Buffer.data(), BufferSize, BufferCoord, &WriteRegion) != FALSE;
	}

	virtual bool Commit() const override
	{
		if(!Imports.pCommit)
			return basic_console::Commit();

		return Imports.pCommit() != FALSE;
	}

	virtual bool GetTextAttributes(FarColor& Attributes) const override
	{
		if(!Imports.pGetTextAttributes)
			return basic_console::GetTextAttributes(Attributes);

		return Imports.pGetTextAttributes(&Attributes) != FALSE;
	}

	virtual bool SetTextAttributes(const FarColor& Attributes) const override
	{
		if(!Imports.pSetTextAttributes)
			return basic_console::SetTextAttributes(Attributes);

		return Imports.pSetTextAttributes(&Attributes) != FALSE;
	}

	virtual bool ClearExtraRegions(const FarColor& Color, int Mode) const override
	{
		if(!Imports.pClearExtraRegions)
			return basic_console::ClearExtraRegions(Color, Mode);

		return Imports.pClearExtraRegions(&Color, Mode) != FALSE;
	}

	virtual bool GetColorDialog(FarColor& Color, bool Centered, bool AddTransparent) const override
	{
		if(!Imports.pGetColorDialog)
			return basic_console::GetColorDialog(Color, Centered, AddTransparent);

		return Imports.pGetColorDialog(&Color, Centered, AddTransparent) != FALSE;
	}

private:
	extended_console():
		Module(L"extendedconsole.dll"),
		Imports(Module)
	{
	}

	os::rtdl::module Module;

	struct ModuleImports
	{
		os::rtdl::function_pointer<BOOL(WINAPI*)(FAR_CHAR_INFO* Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT* ReadRegion)> pReadOutput;
		os::rtdl::function_pointer<BOOL(WINAPI*)(const FAR_CHAR_INFO* Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT* WriteRegion)> pWriteOutput;
		os::rtdl::function_pointer<BOOL(WINAPI*)()> pCommit;
		os::rtdl::function_pointer<BOOL(WINAPI*)(FarColor* Attributes) > pGetTextAttributes;
		os::rtdl::function_pointer<BOOL(WINAPI*)(const FarColor* Attributes)> pSetTextAttributes;
		os::rtdl::function_pointer<BOOL(WINAPI*)(const FarColor* Color, int Mode)> pClearExtraRegions;
		os::rtdl::function_pointer<BOOL(WINAPI*)(FarColor* Color, BOOL Centered, BOOL AddTransparent)> pGetColorDialog;

		explicit ModuleImports(const os::rtdl::module& Module):
#define INIT_IMPORT(name) p ## name(Module, #name)
			INIT_IMPORT(ReadOutput),
			INIT_IMPORT(WriteOutput),
			INIT_IMPORT(Commit),
			INIT_IMPORT(GetTextAttributes),
			INIT_IMPORT(SetTextAttributes),
			INIT_IMPORT(ClearExtraRegions),
			INIT_IMPORT(GetColorDialog)
#undef INIT_IMPORT
		{
		}
	}
	Imports;
};

console& Console()
{
	return extended_console::instance();
}
