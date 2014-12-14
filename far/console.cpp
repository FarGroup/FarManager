/*
Console().cpp

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
#include "colors.hpp"
#include "colormix.hpp"
#include "interf.hpp"
#include "setcolor.hpp"

class basicconsole:public console {
public:

basicconsole():
	// пишем/читаем порциями по 32 K, иначе проблемы.
	MAXSIZE(0x8000),
	m_OriginalInputHandle(GetStdHandle(STD_INPUT_HANDLE))
{}

virtual ~basicconsole() {}

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
	bool Result=false;
	CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
	if(GetConsoleScreenBufferInfo(GetOutputHandle(), &ConsoleScreenBufferInfo))
	{
		if(Global->Opt->WindowMode)
		{
			Size.X=ConsoleScreenBufferInfo.srWindow.Right-ConsoleScreenBufferInfo.srWindow.Left+1;
			Size.Y=ConsoleScreenBufferInfo.srWindow.Bottom-ConsoleScreenBufferInfo.srWindow.Top+1;
		}
		else
		{
			Size=ConsoleScreenBufferInfo.dwSize;
		}
		Result=true;
	}
	return Result;
}

virtual bool SetSize(COORD Size) const override
{
	bool Result=false;
	if(Global->Opt->WindowMode)
	{
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
		if(SetWindowRect(csbi.srWindow))
		{
			csbi.dwSize.X = Size.X;
			Result=SetConsoleScreenBufferSize(GetOutputHandle(), csbi.dwSize)!=FALSE;
		}
	}
	else
	{
		Result=SetConsoleScreenBufferSize(GetOutputHandle(), Size)!=FALSE;
	}
	return Result;
}

virtual bool GetWindowRect(SMALL_RECT& ConsoleWindow) const override
{
	bool Result=false;
	CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
	if(GetConsoleScreenBufferInfo(GetOutputHandle(), &ConsoleScreenBufferInfo))
	{
		ConsoleWindow=ConsoleScreenBufferInfo.srWindow;
		Result=true;
	}
	return Result;
}

virtual bool SetWindowRect(const SMALL_RECT& ConsoleWindow) const override
{
	return SetConsoleWindowInfo(GetOutputHandle(), true, &ConsoleWindow)!=FALSE;
}

virtual bool GetWorkingRect(SMALL_RECT& WorkingRect) const override
{
	bool Result=false;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if(GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi))
	{
		WorkingRect.Bottom=csbi.dwSize.Y-1;
		WorkingRect.Left=0;
		WorkingRect.Right=WorkingRect.Left+ScrX;
		WorkingRect.Top=WorkingRect.Bottom-ScrY;
		Result=true;
	}
	return Result;
}

virtual bool GetTitle(string &strTitle) const override
{
	// Don't use GetConsoleTitle here, it's buggy.

	int Length = GetWindowTextLength(GetWindow());

	if (Length)
	{
		std::vector<wchar_t> Buffer(Length + 1);
		GetWindowText(GetWindow(), Buffer.data(), Length + 1);
		strTitle.assign(Buffer.data(), Length);
	}

	return true;
}

virtual bool SetTitle(const string& Title) const override
{
	return SetConsoleTitle(Title.data())!=FALSE;
}

virtual bool GetKeyboardLayoutName(string &strName) const override
{
	bool Result=false;
	wchar_t Buffer[KL_NAMELENGTH];
	if (Imports().GetConsoleKeyboardLayoutNameW(Buffer))
	{
		Result=true;
		strName = Buffer;
	}
	else
	{
		strName.clear();
	}
	return Result;
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

virtual bool PeekInput(INPUT_RECORD* Buffer, size_t Length, size_t& NumberOfEventsRead) const override
{
	DWORD dwNumberOfEventsRead = 0;
	bool Result=PeekConsoleInput(GetInputHandle(), Buffer, static_cast<DWORD>(Length), &dwNumberOfEventsRead)!=FALSE;
	NumberOfEventsRead = dwNumberOfEventsRead;
	if(Global->Opt->WindowMode && Buffer->EventType==MOUSE_EVENT)
	{
		Buffer->Event.MouseEvent.dwMousePosition.Y=std::max(0, Buffer->Event.MouseEvent.dwMousePosition.Y-GetDelta());
		COORD Size={};
		GetSize(Size);
		Buffer->Event.MouseEvent.dwMousePosition.X=std::min(Buffer->Event.MouseEvent.dwMousePosition.X, static_cast<SHORT>(Size.X-1));
	}
	return Result;
}

virtual bool ReadInput(INPUT_RECORD* Buffer, size_t Length, size_t& NumberOfEventsRead) const override
{
	DWORD dwNumberOfEventsRead = 0;
	bool Result=ReadConsoleInput(GetInputHandle(), Buffer, static_cast<DWORD>(Length), &dwNumberOfEventsRead)!=FALSE;
	if (Result)
	{
		NumberOfEventsRead = dwNumberOfEventsRead;
		if (Global->Opt->WindowMode && Buffer->EventType == MOUSE_EVENT)
		{
			Buffer->Event.MouseEvent.dwMousePosition.Y=std::max(0, Buffer->Event.MouseEvent.dwMousePosition.Y-GetDelta());
			COORD Size={};
			GetSize(Size);
			Buffer->Event.MouseEvent.dwMousePosition.X=std::min(Buffer->Event.MouseEvent.dwMousePosition.X, static_cast<SHORT>(Size.X-1));
		}
	}
	return Result;
}

virtual bool WriteInput(INPUT_RECORD* Buffer, size_t Length, size_t& NumberOfEventsWritten) const override
{
	if(Global->Opt->WindowMode && Buffer->EventType==MOUSE_EVENT)
	{
		Buffer->Event.MouseEvent.dwMousePosition.Y+=GetDelta();
	}
	DWORD dwNumberOfEventsWritten = 0;
	bool Result = WriteConsoleInput(GetInputHandle(), Buffer, static_cast<DWORD>(Length), &dwNumberOfEventsWritten)!=FALSE;
	NumberOfEventsWritten = dwNumberOfEventsWritten;
	return Result;
}

virtual bool ReadOutput(matrix<FAR_CHAR_INFO>& Buffer, COORD BufferCoord, SMALL_RECT& ReadRegion) const override
{
	bool Result=false;
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
		auto SavedY = BufferSize.Y;
		BufferSize.Y = std::max(static_cast<int>(MAXSIZE/(BufferSize.X*sizeof(CHAR_INFO))),1);
		size_t Height = ReadRegion.Bottom - ReadRegion.Top + 1;
		int Start=ReadRegion.Top;
		SMALL_RECT SavedReadRegion=ReadRegion;
		for(size_t i = 0; i < Height; i += BufferSize.Y)
		{
			ReadRegion=SavedReadRegion;
			ReadRegion.Top = static_cast<SHORT>(Start+i);
			Result = ReadConsoleOutput(GetOutputHandle(), ConsoleBuffer[i].data(), BufferSize, BufferCoord, &ReadRegion) != FALSE;
		}
		BufferSize.Y = SavedY;
	}
	else
	{
		Result=ReadConsoleOutput(GetOutputHandle(), ConsoleBuffer.data(), BufferSize, BufferCoord, &ReadRegion)!=FALSE;
	}

	auto& ConsoleBufferVector = ConsoleBuffer.vector();
	std::transform(ConsoleBufferVector.cbegin(), ConsoleBufferVector.cend(), Buffer.data() + Offset, [](CONST_REFERENCE(ConsoleBufferVector) i)
	{
		return FAR_CHAR_INFO::make(i.Char.UnicodeChar, Colors::ConsoleColorToFarColor(i.Attributes));
	});

	if(Global->Opt->WindowMode)
	{
		ReadRegion.Top-=Delta;
		ReadRegion.Bottom-=Delta;
	}

	return Result;
}

virtual bool WriteOutput(const matrix<FAR_CHAR_INFO>& Buffer, COORD BufferCoord, SMALL_RECT& WriteRegion) const override
{
	bool Result=false;
	int Delta=Global->Opt->WindowMode?GetDelta():0;
	WriteRegion.Top+=Delta;
	WriteRegion.Bottom+=Delta;

	COORD BufferSize = { static_cast<SHORT>(Buffer.width()), static_cast<SHORT>(Buffer.height()) };

	// skip unused region
	const size_t Offset = BufferCoord.Y * BufferSize.X;
	const size_t Size = BufferSize.X * (BufferSize.Y - BufferCoord.Y);

	matrix<CHAR_INFO> ConsoleBuffer(BufferSize.Y - BufferCoord.Y, BufferSize.X);
	std::transform(Buffer.data() + Offset, Buffer.data() + Offset + Size, ConsoleBuffer.data(), [](const FAR_CHAR_INFO& i) -> CHAR_INFO
	{
		CHAR_INFO CI = {i.Char, Colors::FarColorToConsoleColor(i.Attributes)};
		return CI;
	});

	BufferSize.Y-=BufferCoord.Y;
	BufferCoord.Y=0;

	if(BufferSize.X*BufferSize.Y*sizeof(CHAR_INFO)>MAXSIZE)
	{
		auto SavedY = BufferSize.Y;
		BufferSize.Y=static_cast<SHORT>(std::max(static_cast<int>(MAXSIZE/(BufferSize.X*sizeof(CHAR_INFO))),1));
		size_t Height = WriteRegion.Bottom - WriteRegion.Top + 1;
		int Start=WriteRegion.Top;
		SMALL_RECT SavedWriteRegion=WriteRegion;
		for (size_t i = 0; i < Height; i += BufferSize.Y)
		{
			WriteRegion=SavedWriteRegion;
			WriteRegion.Top = static_cast<SHORT>(Start + i);
			Result = WriteConsoleOutput(GetOutputHandle(), ConsoleBuffer[i].data(), BufferSize, BufferCoord, &WriteRegion) != FALSE;
		}
		BufferSize.Y = SavedY;
	}
	else
	{
		Result=WriteConsoleOutput(GetOutputHandle(), ConsoleBuffer.data(), BufferSize, BufferCoord, &WriteRegion)!=FALSE;
	}

	if(Global->Opt->WindowMode)
	{
		WriteRegion.Top-=Delta;
		WriteRegion.Bottom-=Delta;
	}

	return Result;
}

virtual bool Write(LPCWSTR Buffer) const override
{
	return Write(Buffer, wcslen(Buffer));
}

virtual bool Write(const string& Buffer) const override
{
	return Write(Buffer.data(), Buffer.size());
}

virtual bool Write(LPCWSTR Buffer, size_t NumberOfCharsToWrite) const override
{
	bool Result = false;
	DWORD NumberOfCharsWritten;
	HANDLE OutputHandle = GetOutputHandle();
	DWORD Mode;
	if(GetMode(OutputHandle, Mode))
	{
		Result =  WriteConsole(OutputHandle, Buffer, static_cast<DWORD>(NumberOfCharsToWrite), &NumberOfCharsWritten, nullptr)!=FALSE;
	}
	else
	{
		Result = WriteFile(OutputHandle, Buffer, static_cast<DWORD>(NumberOfCharsToWrite*sizeof(wchar_t)), &NumberOfCharsWritten, nullptr)!=FALSE;
	}
	return Result;
}

virtual bool Commit() const override
{
	// reserved
	return true;
}

virtual bool GetTextAttributes(FarColor& Attributes) const override
{
	bool Result=false;
	CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
	if(GetConsoleScreenBufferInfo(GetOutputHandle(), &ConsoleScreenBufferInfo))
	{
		Attributes = Colors::ConsoleColorToFarColor(ConsoleScreenBufferInfo.wAttributes);
		Result=true;
	}
	return Result;
}

virtual bool SetTextAttributes(const FarColor& Attributes) const override
{
	return SetConsoleTextAttribute(GetOutputHandle(), Colors::FarColorToConsoleColor(Attributes))!=FALSE;
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
	bool Result=false;
	CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
	if(GetConsoleScreenBufferInfo(GetOutputHandle(), &ConsoleScreenBufferInfo))
	{
		Position=ConsoleScreenBufferInfo.dwCursorPosition;
		if(Global->Opt->WindowMode)
		{
			Position.Y-=GetDelta();
		}
		Result=true;
	}
	return Result;
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
	WORD ConColor = Colors::FarColorToConsoleColor(Color);

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

virtual bool ScrollScreenBuffer(int Lines) const override
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	SMALL_RECT ScrollRectangle={0, 0, static_cast<SHORT>(csbi.dwSize.X-1), static_cast<SHORT>(csbi.dwSize.Y-1)};
	COORD DestinationOrigin={0,static_cast<SHORT>(-Lines)};
	CHAR_INFO Fill={L' ', Colors::FarColorToConsoleColor(ColorIndexToColor(COL_COMMANDLINEUSERSCREEN))};
	return ScrollConsoleScreenBuffer(GetOutputHandle(), &ScrollRectangle, nullptr, DestinationOrigin, &Fill)!=FALSE;
}

virtual bool IsFullscreenSupported() const override
{
#ifdef _WIN64
	return false;
#else
	bool Result = true;
	CONSOLE_SCREEN_BUFFER_INFOEX csbiex = {sizeof(csbiex)};
	if(Imports().GetConsoleScreenBufferInfoEx(GetOutputHandle(), &csbiex))
	{
		Result = csbiex.bFullscreenSupported != FALSE;
	}
	return Result;
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

private:
	const unsigned int MAXSIZE;
	HANDLE m_OriginalInputHandle;
};

class extendedconsole:public basicconsole
{
public:
	virtual ~extendedconsole()
	{
		if(Module)
		{
			FreeLibrary(Module);
		}
	}

	virtual bool ReadOutput(matrix<FAR_CHAR_INFO>& Buffer, COORD BufferCoord, SMALL_RECT& ReadRegion) const override
	{
		bool Result = false;
		if(Imports.pReadOutput)
		{
			const COORD SizeCoord = { static_cast<SHORT>(Buffer.width()), static_cast<SHORT>(Buffer.height()) };
			Result = Imports.pReadOutput(Buffer.data(), SizeCoord, BufferCoord, &ReadRegion) != FALSE;
		}
		else
		{
			Result = basicconsole::ReadOutput(Buffer, BufferCoord, ReadRegion);
		}
		return Result;
	}

	virtual bool WriteOutput(const matrix<FAR_CHAR_INFO>& Buffer, COORD BufferCoord, SMALL_RECT& WriteRegion) const override
	{
		bool Result = false;
		if(Imports.pWriteOutput)
		{
			const COORD BufferSize = { static_cast<SHORT>(Buffer.width()), static_cast<SHORT>(Buffer.height()) };
			Result = Imports.pWriteOutput(Buffer.data(), BufferSize, BufferCoord, &WriteRegion) != FALSE;
		}
		else
		{
			Result = basicconsole::WriteOutput(Buffer, BufferCoord, WriteRegion);
		}
		return Result;
	}

	virtual bool Commit() const override
	{
		bool Result = false;
		if(Imports.pCommit)
		{
			Result = Imports.pCommit() != FALSE;
		}
		else
		{
			Result = basicconsole::Commit();
		}
		return Result;
	}

	virtual bool GetTextAttributes(FarColor& Attributes) const override
	{
		bool Result = false;
		if(Imports.pGetTextAttributes)
		{
			Result = Imports.pGetTextAttributes(&Attributes) != FALSE;
		}
		else
		{
			Result = basicconsole::GetTextAttributes(Attributes);
		}
		return Result;
	}

	virtual bool SetTextAttributes(const FarColor& Attributes) const override
	{
		bool Result = false;
		if(Imports.pSetTextAttributes)
		{
			Result = Imports.pSetTextAttributes(&Attributes) != FALSE;
		}
		else
		{
			Result = basicconsole::SetTextAttributes(Attributes);
		}
		return Result;
	}

	virtual bool ClearExtraRegions(const FarColor& Color, int Mode) const override
	{
		bool Result = false;
		if(Imports.pClearExtraRegions)
		{
			Result = Imports.pClearExtraRegions(&Color, Mode) != FALSE;
		}
		else
		{
			Result = basicconsole::ClearExtraRegions(Color, Mode);
		}
		return Result;
	}

	virtual bool GetColorDialog(FarColor& Color, bool Centered, bool AddTransparent) const override
	{
		bool Result = false;
		if(Imports.pGetColorDialog)
		{
			Result = Imports.pGetColorDialog(&Color, Centered, AddTransparent) != FALSE;
		}
		else
		{
			Result = basicconsole::GetColorDialog(Color, Centered, AddTransparent);
		}
		return Result;
	}

private:
	friend console& Console();

	extendedconsole():
		Imports(),
		Module(LoadLibrary(L"extendedconsole.dll")),
		ImportsPresent(false)
	{
		if (Module)
		{
			#define InitImport(Name) InitImport(Imports.p##Name, #Name)

			InitImport(ReadOutput);
			InitImport(WriteOutput);
			InitImport(Commit);
			InitImport(GetTextAttributes);
			InitImport(SetTextAttributes);
			InitImport(ClearExtraRegions);
			InitImport(GetColorDialog);

			#undef InitImport

			if (!ImportsPresent)
			{
				FreeLibrary(Module);
				Module = nullptr;
			}
		}
	}

	struct ModuleImports
	{
		BOOL (WINAPI *pReadOutput)(FAR_CHAR_INFO* Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT* ReadRegion);
		BOOL (WINAPI *pWriteOutput)(const FAR_CHAR_INFO* Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT* WriteRegion);
		BOOL (WINAPI *pCommit)();
		BOOL (WINAPI *pGetTextAttributes)(FarColor* Attributes);
		BOOL (WINAPI *pSetTextAttributes)(const FarColor* Attributes);
		BOOL (WINAPI *pClearExtraRegions)(const FarColor* Color, int Mode);
		BOOL (WINAPI *pGetColorDialog)(FarColor* Color, BOOL Centered, BOOL AddTransparent);
	}
	Imports;

	template<typename T>
	inline void InitImport(T& Address, const char * ProcName)
	{
		Address = reinterpret_cast<T>(GetProcAddress(Module, ProcName));
		if (!ImportsPresent)
		{
			ImportsPresent = Address != nullptr;
		}
	}

	HMODULE Module;
	bool ImportsPresent;
};

console& Console()
{
	//static console ec;
	static extendedconsole ec;
	return ec;
}
