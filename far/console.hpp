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

class console
{
public:
	console();
	~console();

	bool Allocate() const;
	bool Free() const;

	HANDLE GetInputHandle() const;
	HANDLE GetOutputHandle() const;
	HANDLE GetErrorHandle() const;

	HWND GetWindow() const;

	bool GetSize(COORD& Size) const;
	bool SetSize(COORD Size) const;

	bool GetWindowRect(SMALL_RECT& ConsoleWindow) const;
	bool SetWindowRect(const SMALL_RECT& ConsoleWindow) const;

	bool GetWorkingRect(SMALL_RECT& WorkingRect) const;

	bool GetTitle(string &strTitle) const;
	bool SetTitle(LPCWSTR Title) const;

	bool GetKeyboardLayoutName(string &strName) const;

	UINT GetInputCodepage() const;
	bool SetInputCodepage(UINT Codepage) const;

	UINT GetOutputCodepage() const;
	bool SetOutputCodepage(UINT Codepage) const;

	bool SetControlHandler(PHANDLER_ROUTINE HandlerRoutine, bool Add) const;

	bool GetMode(HANDLE ConsoleHandle, DWORD& Mode) const;
	bool SetMode(HANDLE ConsoleHandle, DWORD Mode) const;

	bool PeekInput(INPUT_RECORD* Buffer, size_t Length, size_t& NumberOfEventsRead) const;
	bool ReadInput(INPUT_RECORD* Buffer, size_t Length, size_t& NumberOfEventsRead) const;
	bool WriteInput(INPUT_RECORD* Buffer, size_t Length, size_t& NumberOfEventsWritten) const;
	bool ReadOutput(FAR_CHAR_INFO* Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT& ReadRegion) const ;
	bool WriteOutput(const FAR_CHAR_INFO* Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT& WriteRegion) const;
	bool Write(LPCWSTR Buffer) const;
	bool Write(const string& Buffer) const;
	bool Write(LPCWSTR Buffer, size_t NumberOfCharsToWrite) const;
	bool Commit() const;

	bool GetTextAttributes(FarColor& Attributes) const;
	bool SetTextAttributes(const FarColor& Attributes) const;

	bool GetCursorInfo(CONSOLE_CURSOR_INFO& ConsoleCursorInfo) const;
	bool SetCursorInfo(const CONSOLE_CURSOR_INFO& ConsoleCursorInfo) const;

	bool GetCursorPosition(COORD& Position) const;
	bool SetCursorPosition(COORD Position) const;

	bool FlushInputBuffer() const;

	bool GetNumberOfInputEvents(size_t& NumberOfEvents) const;

	bool GetAlias(LPCWSTR Source, LPWSTR TargetBuffer, size_t TargetBufferLength, LPCWSTR ExeName) const;

	bool GetDisplayMode(DWORD& Mode) const;

	COORD GetLargestWindowSize() const;

	bool SetActiveScreenBuffer(HANDLE ConsoleOutput) const;

	bool ClearExtraRegions(const FarColor& Color) const;

	bool ScrollWindow(int Lines,int Columns=0) const;

	bool ScrollScreenBuffer(int Lines) const;

	bool IsFullscreenSupported() const;

	bool ResetPosition() const;

	bool GetColorDialog(FarColor& Color, bool Centered = false, bool AddTransparent = false) const;

private:
	class ConsoleCore* Core;
};

extern console Console;
