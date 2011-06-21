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
#include "palette.hpp"
#include "colors.hpp"
#include "colormix.hpp"
#include "interf.hpp"

console Console;

bool console::Allocate() const
{
	return AllocConsole()!=FALSE;
}

bool console::Free() const
{
	return FreeConsole()!=FALSE;
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

HWND console::GetWindow() const
{
	return GetConsoleWindow();
}

bool console::GetSize(COORD& Size) const
{
	bool Result=false;
	CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
	if(GetConsoleScreenBufferInfo(GetOutputHandle(), &ConsoleScreenBufferInfo))
	{
		if(Opt.WindowMode)
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

bool console::SetSize(COORD Size) const
{
	bool Result=false;
	if(Opt.WindowMode)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
		csbi.srWindow.Left=0;
		csbi.srWindow.Right=Size.X-1;
		csbi.srWindow.Bottom=csbi.dwSize.Y-1;
		csbi.srWindow.Top=csbi.srWindow.Bottom-(Size.Y-1);
		COORD WindowCoord={csbi.srWindow.Right-csbi.srWindow.Left+1, csbi.srWindow.Bottom-csbi.srWindow.Top+1};
		if(WindowCoord.X>csbi.dwSize.X || WindowCoord.Y>csbi.dwSize.Y)
		{
			WindowCoord.X=Max(WindowCoord.X,csbi.dwSize.X);
			WindowCoord.Y=Max(WindowCoord.Y,csbi.dwSize.Y);
			SetConsoleScreenBufferSize(GetOutputHandle(), WindowCoord);
		}
		Result=SetWindowRect(csbi.srWindow);
	}
	else
	{
		Result=SetConsoleScreenBufferSize(GetOutputHandle(), Size)!=FALSE;
	}
	return Result;
}

bool console::GetWindowRect(SMALL_RECT& ConsoleWindow) const
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

bool console::SetWindowRect(const SMALL_RECT& ConsoleWindow) const
{
	return SetConsoleWindowInfo(GetOutputHandle(), true, &ConsoleWindow)!=FALSE;
}

bool console::GetWorkingRect(SMALL_RECT& WorkingRect) const
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

bool console::GetTitle(string &strTitle) const
{
	DWORD dwSize = 0;
	DWORD dwBufferSize = MAX_PATH;
	wchar_t *lpwszTitle = nullptr;

	do
	{
		dwBufferSize <<= 1;
		lpwszTitle = (wchar_t*)xf_realloc_nomove(lpwszTitle, dwBufferSize*sizeof(wchar_t));
		dwSize = GetConsoleTitle(lpwszTitle, dwBufferSize);
	}
	while (!dwSize && GetLastError() == ERROR_SUCCESS);

	if (dwSize)
		strTitle = lpwszTitle;

	xf_free(lpwszTitle);
	return dwSize!=0;
}

bool console::SetTitle(LPCWSTR Title) const
{
	return SetConsoleTitle(Title)!=FALSE;
}

bool console::GetKeyboardLayoutName(string &strName) const
{
	bool Result=false;
	strName.Clear();
	if (ifn.pfnGetConsoleKeyboardLayoutName)
	{
		wchar_t *p = strName.GetBuffer(KL_NAMELENGTH+1);
		if (p && ifn.pfnGetConsoleKeyboardLayoutName(p))
		{
			Result=true;
		}
		strName.ReleaseBuffer();
	}
	else
	{
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	}
	return Result;
}

UINT console::GetInputCodepage() const
{
	return GetConsoleCP();
}

bool console::SetInputCodepage(UINT Codepage) const
{
	return SetConsoleCP(Codepage)!=FALSE;
}

UINT console::GetOutputCodepage() const
{
	return GetConsoleOutputCP();
}

bool console::SetOutputCodepage(UINT Codepage) const
{
	return SetConsoleOutputCP(Codepage)!=FALSE;
}

bool console::SetControlHandler(PHANDLER_ROUTINE HandlerRoutine, bool Add) const
{
	return SetConsoleCtrlHandler(HandlerRoutine, Add)!=FALSE;
}

bool console::GetMode(HANDLE ConsoleHandle, DWORD& Mode) const
{
	return GetConsoleMode(ConsoleHandle, &Mode)!=FALSE;
}

bool console::SetMode(HANDLE ConsoleHandle, DWORD Mode) const
{
	return SetConsoleMode(ConsoleHandle, Mode)!=FALSE;
}

bool console::PeekInput(INPUT_RECORD* Buffer, DWORD Length, DWORD& NumberOfEventsRead) const
{
	bool Result=PeekConsoleInput(GetInputHandle(), Buffer, Length, &NumberOfEventsRead)!=FALSE;
	if(Opt.WindowMode && Buffer->EventType==MOUSE_EVENT)
	{
		Buffer->Event.MouseEvent.dwMousePosition.Y=Max(0, Buffer->Event.MouseEvent.dwMousePosition.Y-GetDelta());
		COORD Size={};
		GetSize(Size);
		Buffer->Event.MouseEvent.dwMousePosition.X=Min(Buffer->Event.MouseEvent.dwMousePosition.X, static_cast<SHORT>(Size.X-1));
	}
	return Result;
}

bool console::ReadInput(INPUT_RECORD* Buffer, DWORD Length, DWORD& NumberOfEventsRead) const
{
	bool Result=ReadConsoleInput(GetInputHandle(), Buffer, Length, &NumberOfEventsRead)!=FALSE;
	if(Opt.WindowMode && Buffer->EventType==MOUSE_EVENT)
	{
		Buffer->Event.MouseEvent.dwMousePosition.Y=Max(0, Buffer->Event.MouseEvent.dwMousePosition.Y-GetDelta());
		COORD Size={};
		GetSize(Size);
		Buffer->Event.MouseEvent.dwMousePosition.X=Min(Buffer->Event.MouseEvent.dwMousePosition.X, static_cast<SHORT>(Size.X-1));
	}
	return Result;
}

bool console::WriteInput(INPUT_RECORD* Buffer, DWORD Length, DWORD& NumberOfEventsWritten) const
{
	if(Opt.WindowMode && Buffer->EventType==MOUSE_EVENT)
	{
		Buffer->Event.MouseEvent.dwMousePosition.Y+=GetDelta();
	}
	return WriteConsoleInput(GetInputHandle(), Buffer, Length, &NumberOfEventsWritten)!=FALSE;
}

// пишем/читаем порциями по 32 K, иначе проблемы.
const unsigned int MAXSIZE=0x8000;

bool console::ReadOutput(FAR_CHAR_INFO* Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT& ReadRegion) const
{
	bool Result=false;
	int Delta=Opt.WindowMode?GetDelta():0;
	ReadRegion.Top+=Delta;
	ReadRegion.Bottom+=Delta;

	// skip unused region
	int Offset = BufferCoord.Y*BufferSize.X;
	PCHAR_INFO ConsoleBuffer = new CHAR_INFO[BufferSize.X*BufferSize.Y-Offset]();
	BufferSize.Y-=BufferCoord.Y;
	BufferCoord.Y=0;

	if(BufferSize.X*BufferSize.Y*sizeof(CHAR_INFO)>MAXSIZE)
	{
		BufferSize.Y=static_cast<SHORT>(Max(static_cast<int>(MAXSIZE/(BufferSize.X*sizeof(CHAR_INFO))),1));
		int Height=ReadRegion.Bottom-ReadRegion.Top+1;
		int Start=ReadRegion.Top;
		SMALL_RECT SavedWriteRegion=ReadRegion;
		for(int i=0;i<Height;i+=BufferSize.Y)
		{
			ReadRegion=SavedWriteRegion;
			ReadRegion.Top=Start+i;
			PCHAR_INFO BufPtr=ConsoleBuffer+i*BufferSize.X;
			Result=ReadConsoleOutput(GetOutputHandle(), BufPtr, BufferSize, BufferCoord, &ReadRegion)!=FALSE;
		}
	}
	else
	{
		Result=ReadConsoleOutput(GetOutputHandle(), ConsoleBuffer, BufferSize, BufferCoord, &ReadRegion)!=FALSE;
	}

	for(int i = 0; i < BufferSize.X*BufferSize.Y; ++i)
	{
		Buffer[i+Offset].Char = ConsoleBuffer[i].Char.UnicodeChar;
		Colors::ConsoleColorToFarColor(ConsoleBuffer[i].Attributes, Buffer[i+Offset].Attributes);
	}

	delete[] ConsoleBuffer; 

	if(Opt.WindowMode)
	{
		ReadRegion.Top-=Delta;
		ReadRegion.Bottom-=Delta;
	}

	return Result;
}

bool console::WriteOutput(const FAR_CHAR_INFO* Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT& WriteRegion) const
{
	bool Result=false;
	int Delta=Opt.WindowMode?GetDelta():0;
	WriteRegion.Top+=Delta;
	WriteRegion.Bottom+=Delta;

	// skip unused region
	int Offset = BufferCoord.Y*BufferSize.X;
	PCHAR_INFO ConsoleBuffer = new CHAR_INFO[BufferSize.X*BufferSize.Y-Offset];
	for(int i = Offset; i < BufferSize.X*BufferSize.Y; ++i)
	{
		ConsoleBuffer[i-Offset].Char.UnicodeChar = Buffer[i].Char;
		ConsoleBuffer[i-Offset].Attributes = Colors::FarColorToConsoleColor(Buffer[i].Attributes);
	}

	BufferSize.Y-=BufferCoord.Y;
	BufferCoord.Y=0;

	if(BufferSize.X*BufferSize.Y*sizeof(CHAR_INFO)>MAXSIZE)
	{
		BufferSize.Y=static_cast<SHORT>(Max(static_cast<int>(MAXSIZE/(BufferSize.X*sizeof(CHAR_INFO))),1));
		int Height=WriteRegion.Bottom-WriteRegion.Top+1;
		int Start=WriteRegion.Top;
		SMALL_RECT SavedWriteRegion=WriteRegion;
		for(int i=0;i<Height;i+=BufferSize.Y)
		{
			WriteRegion=SavedWriteRegion;
			WriteRegion.Top=Start+i;
			const CHAR_INFO* BufPtr=ConsoleBuffer+i*BufferSize.X;
			Result=WriteConsoleOutput(GetOutputHandle(), BufPtr, BufferSize, BufferCoord, &WriteRegion)!=FALSE;
		}
	}
	else
	{
		Result=WriteConsoleOutput(GetOutputHandle(), ConsoleBuffer, BufferSize, BufferCoord, &WriteRegion)!=FALSE;
	}

	delete[] ConsoleBuffer;

	if(Opt.WindowMode)
	{
		WriteRegion.Top-=Delta;
		WriteRegion.Bottom-=Delta;
	}

	return Result;
}

bool console::Write(LPCWSTR Buffer, DWORD NumberOfCharsToWrite) const
{
	DWORD NumberOfCharsWritten;
	return WriteConsole(GetOutputHandle(), Buffer, NumberOfCharsToWrite, &NumberOfCharsWritten, nullptr)!=FALSE;
}

bool console::GetTextAttributes(WORD& Attributes) const
{
	bool Result=false;
	CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
	if(GetConsoleScreenBufferInfo(GetOutputHandle(), &ConsoleScreenBufferInfo))
	{
		Attributes=ConsoleScreenBufferInfo.wAttributes;
		Result=true;
	}
	return Result;
}

bool console::SetTextAttributes(WORD Attributes) const
{
	return SetConsoleTextAttribute(GetOutputHandle(), Attributes)!=FALSE;
}

bool console::GetCursorInfo(CONSOLE_CURSOR_INFO& ConsoleCursorInfo) const
{
	return GetConsoleCursorInfo(GetOutputHandle(), &ConsoleCursorInfo)!=FALSE;
}

bool console::SetCursorInfo(const CONSOLE_CURSOR_INFO& ConsoleCursorInfo) const
{
	return SetConsoleCursorInfo(GetOutputHandle(), &ConsoleCursorInfo)!=FALSE;
}

bool console::GetCursorPosition(COORD& Position) const
{
	bool Result=false;
	CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
	if(GetConsoleScreenBufferInfo(GetOutputHandle(), &ConsoleScreenBufferInfo))
	{
		Position=ConsoleScreenBufferInfo.dwCursorPosition;
		if(Opt.WindowMode)
		{
			Position.Y-=GetDelta();
		}
		Result=true;
	}
	return Result;
}

bool console::SetCursorPosition(COORD Position) const
{

	if(Opt.WindowMode)
	{
		ResetPosition();
		COORD Size={};
		GetSize(Size);
		Position.X=Min(Position.X,static_cast<SHORT>(Size.X-1));
		Position.Y=Max(static_cast<SHORT>(0),Position.Y);
		Position.Y+=GetDelta();
	}
	return SetConsoleCursorPosition(GetOutputHandle(), Position)!=FALSE;
}

bool console::FlushInputBuffer() const
{
	return FlushConsoleInputBuffer(GetInputHandle())!=FALSE;
}

bool console::GetNumberOfInputEvents(DWORD& NumberOfEvents) const
{
	return GetNumberOfConsoleInputEvents(GetInputHandle(), &NumberOfEvents)!=FALSE;
}

DWORD console::GetAlias(LPCWSTR Source, LPWSTR TargetBuffer, DWORD TargetBufferLength, LPCWSTR ExeName) const
{
	return GetConsoleAlias(const_cast<LPWSTR>(Source), TargetBuffer, TargetBufferLength, const_cast<LPWSTR>(ExeName));
}

bool console::GetDisplayMode(DWORD& Mode) const
{
	return GetConsoleDisplayMode(&Mode)!=FALSE;
}

COORD console::GetLargestWindowSize() const
{
	return GetLargestConsoleWindowSize(GetOutputHandle());
}

bool console::SetActiveScreenBuffer(HANDLE ConsoleOutput) const
{
	return SetConsoleActiveScreenBuffer(ConsoleOutput)!=FALSE;
}

bool console::ClearExtraRegions(WORD Color) const
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	DWORD TopSize = csbi.dwSize.X*csbi.srWindow.Top;
	DWORD CharsWritten;
	COORD TopCoord={0,0};
	FillConsoleOutputCharacter(GetOutputHandle(), L' ', TopSize, TopCoord, &CharsWritten);
	FillConsoleOutputAttribute(GetOutputHandle(), Color, TopSize, TopCoord, &CharsWritten );

	DWORD RightSize = csbi.dwSize.X-csbi.srWindow.Right;
	COORD RightCoord={csbi.srWindow.Right,GetDelta()};
	for(; RightCoord.Y<csbi.dwSize.Y; RightCoord.Y++)
	{
		FillConsoleOutputCharacter(GetOutputHandle(), L' ', RightSize, RightCoord, &CharsWritten);
		FillConsoleOutputAttribute(GetOutputHandle(), Color, RightSize, RightCoord, &CharsWritten);
	}
	return true;
}

bool console::ScrollWindow(int Lines,int Columns) const
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

bool console::ScrollScreenBuffer(int Lines) const
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	SMALL_RECT ScrollRectangle={0, 0, csbi.dwSize.X-1, csbi.dwSize.Y-1};
	COORD DestinationOrigin={0,-Lines};
	CHAR_INFO Fill={L' ', Colors::FarColorToConsoleColor(ColorIndexToColor(COL_COMMANDLINEUSERSCREEN))};
	return ScrollConsoleScreenBuffer(GetOutputHandle(), &ScrollRectangle, nullptr, DestinationOrigin, &Fill)!=FALSE;
}

bool console::ResetPosition() const
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

int console::GetDelta() const
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	return csbi.dwSize.Y-(csbi.srWindow.Bottom-csbi.srWindow.Top+1);
}
