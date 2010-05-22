/*
console.cpp

Console functions
*/
/*
Copyright (c) 2010 Far Group
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

console Console;

bool console::Allocate()
{
	return AllocConsole()!=FALSE;
}

bool console::Free()
{
	return FreeConsole()!=FALSE;
}

HANDLE console::GetInputHandle()
{
	return GetStdHandle(STD_INPUT_HANDLE);
}

HANDLE console::GetOutputHandle()
{
	return GetStdHandle(STD_OUTPUT_HANDLE);
}

HANDLE console::GetErrorHandle()
{
	return GetStdHandle(STD_ERROR_HANDLE);
}

HWND console::GetWindow()
{
	return GetConsoleWindow();
}

bool console::GetSize(COORD& Size)
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

bool console::SetSize(COORD Size)
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

bool console::GetWindowRect(SMALL_RECT& ConsoleWindow)
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

bool console::SetWindowRect(const SMALL_RECT& ConsoleWindow)
{
	return SetConsoleWindowInfo(GetOutputHandle(), true, &ConsoleWindow)!=FALSE;
}

bool console::GetTitle(string &strTitle)
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

bool console::SetTitle(LPCWSTR Title)
{
	return SetConsoleTitle(Title)!=FALSE;
}

bool console::GetKeyboardLayoutName(string &strName)
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

UINT console::GetInputCodepage()
{
	return GetConsoleCP();
}

bool console::SetInputCodepage(UINT Codepage)
{
	return SetConsoleCP(Codepage)!=FALSE;
}

UINT console::GetOutputCodepage()
{
	return GetConsoleOutputCP();
}

bool console::SetOutputCodepage(UINT Codepage)
{
	return SetConsoleOutputCP(Codepage)!=FALSE;
}

bool console::SetControlHandler(PHANDLER_ROUTINE HandlerRoutine, bool Add)
{
	return SetConsoleCtrlHandler(HandlerRoutine, Add)!=FALSE;
}

bool console::GetMode(HANDLE ConsoleHandle, DWORD& Mode)
{
	return GetConsoleMode(ConsoleHandle, &Mode)!=FALSE;
}

bool console::SetMode(HANDLE ConsoleHandle, DWORD Mode)
{
	return SetConsoleMode(ConsoleHandle, Mode)!=FALSE;
}

bool console::PeekInput(INPUT_RECORD& Buffer, DWORD Length, DWORD& NumberOfEventsRead)
{
	bool Result=PeekConsoleInput(GetInputHandle(), &Buffer, Length, &NumberOfEventsRead)!=FALSE;
	if(Opt.WindowMode && Buffer.EventType==MOUSE_EVENT)
	{
		Buffer.Event.MouseEvent.dwMousePosition.Y=Max(0, Buffer.Event.MouseEvent.dwMousePosition.Y-GetDelta());
		COORD Size;
		GetSize(Size);
		Buffer.Event.MouseEvent.dwMousePosition.X=Min(Buffer.Event.MouseEvent.dwMousePosition.X, static_cast<SHORT>(Size.X-1));
	}
	return Result;
}

bool console::ReadInput(INPUT_RECORD& Buffer, DWORD Length, DWORD& NumberOfEventsRead)
{
	bool Result=ReadConsoleInput(GetInputHandle(), &Buffer, Length, &NumberOfEventsRead)!=FALSE;
	if(Opt.WindowMode && Buffer.EventType==MOUSE_EVENT)
	{
		Buffer.Event.MouseEvent.dwMousePosition.Y=Max(0, Buffer.Event.MouseEvent.dwMousePosition.Y-GetDelta());
		COORD Size;
		GetSize(Size);
		Buffer.Event.MouseEvent.dwMousePosition.X=Min(Buffer.Event.MouseEvent.dwMousePosition.X, static_cast<SHORT>(Size.X-1));
	}
	return Result;
}

bool console::WriteInput(INPUT_RECORD& Buffer, DWORD Length, DWORD& NumberOfEventsWritten)
{
	if(Opt.WindowMode && Buffer.EventType==MOUSE_EVENT)
	{
		Buffer.Event.MouseEvent.dwMousePosition.Y+=GetDelta();
	}
	return WriteConsoleInput(GetInputHandle(), &Buffer, Length, &NumberOfEventsWritten)!=FALSE;
}

// пишем/читаем порциями по 32 K, иначе проблемы.
#define MAXSIZE 0x8000

bool console::ReadOutput(CHAR_INFO& Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT& ReadRegion)
{
	bool Result=false;
	int Delta=Opt.WindowMode?GetDelta():0;
	if(BufferSize.X*BufferSize.Y*sizeof(CHAR_INFO)>MAXSIZE)
	{
		BufferCoord.Y=0;
		int ReadY2=ReadRegion.Bottom;
		for (int yy=ReadRegion.Top; yy<=ReadY2;)
		{
			ReadRegion.Top=yy;
			PCHAR_INFO BufPtr=&Buffer+yy*BufferSize.X;
			BufferSize.Y=Min(Max(MAXSIZE/static_cast<int>(BufferSize.X*sizeof(CHAR_INFO)),1),ReadY2-yy+1);
			yy+=BufferSize.Y;
			ReadRegion.Bottom=yy-1;
			ReadRegion.Top+=Delta;
			ReadRegion.Bottom+=Delta;
			Result=ReadConsoleOutput(GetOutputHandle(), BufPtr, BufferSize, BufferCoord, &ReadRegion)!=FALSE;
			if(!Result)
			{
				break;
			}
		}
	}
	else
	{
		ReadRegion.Top+=Delta;
		ReadRegion.Bottom+=Delta;
		Result=ReadConsoleOutput(GetOutputHandle(), &Buffer, BufferSize, BufferCoord, &ReadRegion)!=FALSE;
	}

	return Result;
}

bool console::WriteOutput(const CHAR_INFO& Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT& WriteRegion)
{
	bool Result=false;
	int Delta=Opt.WindowMode?GetDelta():0;
	if(BufferSize.X*BufferSize.Y*sizeof(CHAR_INFO)>MAXSIZE)
	{
		BufferCoord.Y=0;
		int WriteY2=WriteRegion.Bottom;
		for (int yy=WriteRegion.Top; yy<=WriteY2;)
		{
			WriteRegion.Top=yy;
			const CHAR_INFO* BufPtr=&Buffer+yy*BufferSize.X;
			BufferSize.Y=Min(Max(MAXSIZE/static_cast<int>(BufferSize.X*sizeof(CHAR_INFO)),1),WriteY2-yy+1);
			yy+=BufferSize.Y;
			WriteRegion.Bottom=yy-1;
			WriteRegion.Top+=Delta;
			WriteRegion.Bottom+=Delta;
			Result=WriteConsoleOutput(GetOutputHandle(), BufPtr, BufferSize, BufferCoord, &WriteRegion)!=FALSE;
			if(!Result)
			{
				break;
			}
		}
	}
	else
	{
		WriteRegion.Top+=Delta;
		WriteRegion.Bottom+=Delta;
		Result=WriteConsoleOutput(GetOutputHandle(), &Buffer, BufferSize, BufferCoord, &WriteRegion)!=FALSE;
	}

	if(Opt.WindowMode)
	{
		int Delta=GetDelta();
		WriteRegion.Top-=Delta;
		WriteRegion.Bottom-=Delta;
	}

	return Result;
}

bool console::Write(LPCWSTR Buffer, DWORD NumberOfCharsToWrite)
{
	return WriteConsole(GetOutputHandle(), Buffer, NumberOfCharsToWrite, nullptr, nullptr)!=FALSE;
}

bool console::GetTextAttributes(WORD& Attributes)
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

bool console::SetTextAttributes(WORD Attributes)
{
	return SetConsoleTextAttribute(GetOutputHandle(), Attributes)!=FALSE;
}

bool console::GetCursorInfo(CONSOLE_CURSOR_INFO& ConsoleCursorInfo)
{
	return GetConsoleCursorInfo(GetOutputHandle(), &ConsoleCursorInfo)!=FALSE;
}

bool console::SetCursorInfo(const CONSOLE_CURSOR_INFO& ConsoleCursorInfo)
{
	if(Opt.WindowMode)
	{
		ResetPosition();
	}
	return SetConsoleCursorInfo(GetOutputHandle(), &ConsoleCursorInfo)!=FALSE;
}

bool console::GetCursorPosition(COORD& Position)
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

bool console::SetCursorPosition(COORD Position)
{

	if(Opt.WindowMode)
	{
		ResetPosition();
		COORD Size;
		GetSize(Size);
		Position.X=Min(Position.X,static_cast<SHORT>(Size.X-1));
		Position.Y=Max(static_cast<SHORT>(0),Position.Y);
		Position.Y+=GetDelta();
	}
	return SetConsoleCursorPosition(GetOutputHandle(), Position)!=FALSE;
}

bool console::FlushInputBuffer()
{
	return FlushConsoleInputBuffer(GetInputHandle())!=FALSE;
}

bool console::GetNumberOfInputEvents(DWORD& NumberOfEvents)
{
	return GetNumberOfConsoleInputEvents(GetInputHandle(), &NumberOfEvents)!=FALSE;
}

DWORD console::GetAlias(LPCWSTR Source, LPWSTR TargetBuffer, DWORD TargetBufferLength, LPCWSTR ExeName)
{
	return GetConsoleAlias(const_cast<LPWSTR>(Source), TargetBuffer, TargetBufferLength, const_cast<LPWSTR>(ExeName));
}

bool console::GetDisplayMode(DWORD& Mode)
{
	return GetConsoleDisplayMode(&Mode)!=FALSE;
}

COORD console::GetLargestWindowSize()
{
	return GetLargestConsoleWindowSize(GetOutputHandle());
}

bool console::SetActiveScreenBuffer(HANDLE ConsoleOutput)
{
	return SetConsoleActiveScreenBuffer(ConsoleOutput)!=FALSE;
}

bool console::ClearExtraRegions(WORD Color)
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

bool console::Scroll(int Lines)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
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
	//if(csbi.srWindow.Top>=0 && csbi.srWindow.Bottom<csbi.dwSize.Y)
	{
		SetWindowRect(csbi.srWindow);
	}
	return true;
}

bool console::ResetPosition()
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

int console::GetDelta()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetOutputHandle(), &csbi);
	return csbi.dwSize.Y-(csbi.srWindow.Bottom-csbi.srWindow.Top+1);
}
