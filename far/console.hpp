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

enum CLEAR_REGION
{
	CR_TOP=0x1,
	CR_RIGHT=0x2,
	CR_BOTH=CR_TOP|CR_RIGHT,
};

class console
{
public:
	static console* CreateInstance(bool exnended);
	virtual ~console(){};

	virtual bool Allocate() const = 0;
	virtual bool Free() const = 0;

	virtual HANDLE GetInputHandle() const = 0;
	virtual HANDLE GetOutputHandle() const = 0;
	virtual HANDLE GetErrorHandle() const = 0;

	virtual HWND GetWindow() const = 0;

	virtual bool GetSize(COORD& Size) const = 0;
	virtual bool SetSize(COORD Size) const = 0;

	virtual bool GetWindowRect(SMALL_RECT& ConsoleWindow) const = 0;
	virtual bool SetWindowRect(const SMALL_RECT& ConsoleWindow) const = 0;

	virtual bool GetWorkingRect(SMALL_RECT& WorkingRect) const = 0;

	virtual bool GetTitle(string &strTitle) const = 0;
	virtual bool SetTitle(LPCWSTR Title) const = 0;

	virtual bool GetKeyboardLayoutName(string &strName) const = 0;

	virtual uintptr_t GetInputCodepage() const = 0;
	virtual bool SetInputCodepage(uintptr_t Codepage) const = 0;

	virtual uintptr_t GetOutputCodepage() const = 0;
	virtual bool SetOutputCodepage(uintptr_t Codepage) const = 0;

	virtual bool SetControlHandler(PHANDLER_ROUTINE HandlerRoutine, bool Add) const = 0;

	virtual bool GetMode(HANDLE ConsoleHandle, DWORD& Mode) const = 0;
	virtual bool SetMode(HANDLE ConsoleHandle, DWORD Mode) const = 0;

	virtual bool PeekInput(INPUT_RECORD* Buffer, size_t Length, size_t& NumberOfEventsRead) const = 0;
	virtual bool ReadInput(INPUT_RECORD* Buffer, size_t Length, size_t& NumberOfEventsRead) const = 0;
	virtual bool WriteInput(INPUT_RECORD* Buffer, size_t Length, size_t& NumberOfEventsWritten) const = 0;
	virtual bool ReadOutput(FAR_CHAR_INFO* Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT& ReadRegion) const = 0;
	virtual bool WriteOutput(const FAR_CHAR_INFO* Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT& WriteRegion) const = 0;
	virtual bool Write(LPCWSTR Buffer) const = 0;
	virtual bool Write(const string& Buffer) const = 0;
	virtual bool Write(LPCWSTR Buffer, size_t NumberOfCharsToWrite) const = 0;
	virtual bool Commit() const = 0;

	virtual bool GetTextAttributes(FarColor& Attributes) const = 0;
	virtual bool SetTextAttributes(const FarColor& Attributes) const = 0;

	virtual bool GetCursorInfo(CONSOLE_CURSOR_INFO& ConsoleCursorInfo) const = 0;
	virtual bool SetCursorInfo(const CONSOLE_CURSOR_INFO& ConsoleCursorInfo) const = 0;

	virtual bool GetCursorPosition(COORD& Position) const = 0;
	virtual bool SetCursorPosition(COORD Position) const = 0;

	virtual bool FlushInputBuffer() const = 0;

	virtual bool GetNumberOfInputEvents(size_t& NumberOfEvents) const = 0;

	virtual bool GetAlias(LPCWSTR Source, LPWSTR TargetBuffer, size_t TargetBufferLength, LPCWSTR ExeName) const = 0;

	virtual bool GetDisplayMode(DWORD& Mode) const = 0;

	virtual COORD GetLargestWindowSize() const = 0;

	virtual bool SetActiveScreenBuffer(HANDLE ConsoleOutput) const = 0;

	virtual bool ClearExtraRegions(const FarColor& Color, int Mode) const = 0;

	virtual bool ScrollWindow(int Lines,int Columns=0) const = 0;

	virtual bool ScrollWindowToBegin() const = 0;

	virtual bool ScrollWindowToEnd() const = 0;

	virtual bool ScrollScreenBuffer(int Lines) const = 0;

	virtual bool IsFullscreenSupported() const = 0;

	virtual bool ResetPosition() const = 0;

	virtual bool GetColorDialog(FarColor& Color, bool Centered = false, bool AddTransparent = false) const = 0;

protected:
	console(){};
};
