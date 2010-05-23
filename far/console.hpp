#pragma once

/*
console.hpp

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

class console
{
public:
	bool Allocate();
	bool Free();

	HANDLE GetInputHandle();
	HANDLE GetOutputHandle();
	HANDLE GetErrorHandle();

	HWND GetWindow();

	bool GetSize(COORD& Size);
	bool SetSize(COORD Size);

	bool GetWindowRect(SMALL_RECT& ConsoleWindow);
	bool SetWindowRect(const SMALL_RECT& ConsoleWindow);
	
	bool GetTitle(string &strTitle);
	bool SetTitle(LPCWSTR Title);
	
	bool GetKeyboardLayoutName(string &strName);
	
	UINT GetInputCodepage();
	bool SetInputCodepage(UINT Codepage);
	
	UINT GetOutputCodepage();
	bool SetOutputCodepage(UINT Codepage);
	
	bool SetControlHandler(PHANDLER_ROUTINE HandlerRoutine, bool Add);
	
	bool GetMode(HANDLE ConsoleHandle, DWORD& Mode);
	bool SetMode(HANDLE ConsoleHandle, DWORD Mode);
	
	bool PeekInput(INPUT_RECORD& Buffer, DWORD Length, DWORD& NumberOfEventsRead);
	bool ReadInput(INPUT_RECORD& Buffer, DWORD Length, DWORD& NumberOfEventsRead);
	bool WriteInput(INPUT_RECORD& Buffer, DWORD Length, DWORD& NumberOfEventsWritten);
	bool ReadOutput(CHAR_INFO& Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT& lpReadRegion);
	bool WriteOutput(const CHAR_INFO& Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT& WriteRegion);
	bool Write(LPCWSTR Buffer, DWORD NumberOfCharsToWrite);

	bool GetTextAttributes(WORD& Attributes);
	bool SetTextAttributes(WORD Attributes);

	bool GetCursorInfo(CONSOLE_CURSOR_INFO& ConsoleCursorInfo);
	bool SetCursorInfo(const CONSOLE_CURSOR_INFO& ConsoleCursorInfo);

	bool GetCursorPosition(COORD& Position);
	bool SetCursorPosition(COORD Position);

	bool FlushInputBuffer();

	bool GetNumberOfInputEvents(DWORD& NumberOfEvents);

	DWORD GetAlias(LPCWSTR Source, LPWSTR TargetBuffer, DWORD TargetBufferLength, LPCWSTR ExeName);

	bool GetDisplayMode(DWORD& Mode);

	COORD GetLargestWindowSize();

	bool SetActiveScreenBuffer(HANDLE ConsoleOutput);

	bool ClearExtraRegions(WORD Color);

	bool ScrollWindow(int Lines);

	bool ScrollScreenBuffer(int Lines);

	bool ResetPosition();

private:
	int GetDelta();
};

extern console Console;
