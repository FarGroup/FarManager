/*
interf.cpp

Консольные функции ввода-вывода
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "interf.hpp"

// Internal:
#include "keyboard.hpp"
#include "keys.hpp"
#include "farcolor.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "manager.hpp"
#include "scrbuf.hpp"
#include "strmix.hpp"
#include "console.hpp"
#include "colormix.hpp"
#include "imports.hpp"
#include "res.hpp"
#include "plugins.hpp"
#include "lang.hpp"
#include "taskbar.hpp"
#include "global.hpp"

// Platform:
#include "platform.concurrency.hpp"
#include "platform.security.hpp"

// Common:
#include "common/function_ref.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static HICON load_icon(int IconId, bool Big)
{
	return static_cast<HICON>(LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IconId), IMAGE_ICON, GetSystemMetrics(Big? SM_CXICON : SM_CXSMICON), GetSystemMetrics(Big? SM_CYICON : SM_CYSMICON), LR_SHARED));
}

static HICON set_icon(HWND Wnd, bool Big, HICON Icon)
{
	return reinterpret_cast<HICON>(SendMessage(Wnd, WM_SETICON, Big? ICON_BIG : ICON_SMALL, reinterpret_cast<LPARAM>(Icon)));
}

void consoleicons::set_icon()
{
	if (!Global->Opt->SetIcon)
		return restore_icon();

	const auto hWnd = console.GetWindow();
	if (!hWnd)
		return;

	if (Global->Opt->IconIndex < 0 || static_cast<size_t>(Global->Opt->IconIndex) >= size())
		return;

	const int IconId = (Global->Opt->SetAdminIcon && os::security::is_admin())? FAR_ICON_RED : FAR_ICON + Global->Opt->IconIndex;

	const auto Set = [&](icon& Icon)
	{
		const auto RawIcon = load_icon(IconId, Icon.IsBig);
		if (!RawIcon)
			return;

		const auto PreviousIcon = ::set_icon(hWnd, Icon.IsBig, RawIcon);

		if (!Icon.InitialIcon)
		{
			Icon.InitialIcon = PreviousIcon;
		}
	};

	Set(m_Large);
	Set(m_Small);
}

void consoleicons::restore_icon()
{
	const auto hWnd = console.GetWindow();
	if (!hWnd)
		return;

	const auto Restore = [hWnd](icon& Icon)
	{
		if (!Icon.InitialIcon)
			return;

		::set_icon(hWnd, Icon.IsBig, *Icon.InitialIcon);
		Icon.InitialIcon.reset();
	};

	Restore(m_Large);
	Restore(m_Small);
}

size_t consoleicons::size() const
{
	return FAR_ICON_COUNT;
}

static int CurX,CurY;
static FarColor CurColor;

static CONSOLE_CURSOR_INFO InitialCursorInfo;

static rectangle windowholder_rect;

wchar_t BoxSymbols[BS_COUNT];

point InitSize{};
point CurSize{};
int ScrX=0, ScrY=0;
int PrevScrX=-1, PrevScrY=-1;
std::optional<console_mode> InitialConsoleMode;
static rectangle InitWindowRect;
static point InitialSize;

static os::event& CancelIoInProgress()
{
	static os::event s_CancelIoInProgress;
	return s_CancelIoInProgress;
}

static unsigned int CancelSynchronousIoWrapper(void* Thread)
{
	// TODO: SEH guard, try/catch, exception_ptr
	const auto Result = imports.CancelSynchronousIo(Thread);
	CancelIoInProgress().reset();
	return Result;
}

static BOOL WINAPI CtrlHandler(DWORD CtrlType)
{
	switch(CtrlType)
	{
	case CTRL_C_EVENT:
		return TRUE;

	case CTRL_BREAK_EVENT:
		if(!CancelIoInProgress().is_signaled())
		{
			CancelIoInProgress().set();
			os::thread(os::thread::mode::detach, &CancelSynchronousIoWrapper, Global->MainThreadHandle());
		}
		WriteInput(KEY_BREAK);

		if (Global->CtrlObject && Global->CtrlObject->Cp())
		{
			const auto ProcessEvent = [&](Panel const* const p)
			{
				if (p && p->GetMode() == panel_mode::PLUGIN_PANEL)
					Global->CtrlObject->Plugins->ProcessEvent(p->GetPluginHandle(), FE_BREAK, ToPtr(CtrlType));
			};

			ProcessEvent(Global->CtrlObject->Cp()->LeftPanel().get());
			ProcessEvent(Global->CtrlObject->Cp()->RightPanel().get());
		}
		return TRUE;

	case CTRL_CLOSE_EVENT:
		Global->CloseFAR = true;
		Global->AllowCancelExit = false;

		// trick to let wmain() finish correctly
		ExitThread(1);
		//return TRUE;
	}
	return FALSE;
}

static bool ConsoleScrollHook(const Manager::Key& key)
{
	// Удалить после появления макрофункции Scroll
	if (Global->Opt->WindowMode && Global->WindowManager->IsPanelsActive())
	{
		switch (key())
		{
		case KEY_CTRLALTUP:
		case KEY_RCTRLRALTUP:
		case KEY_CTRLRALTUP:
		case KEY_RCTRLALTUP:
		case KEY_CTRLALTNUMPAD8:
		case KEY_RCTRLALTNUMPAD8:
		case KEY_CTRLRALTNUMPAD8:
		case KEY_RCTRLRALTNUMPAD8:
			console.ScrollWindow(-1);
			return true;

		case KEY_CTRLALTDOWN:
		case KEY_RCTRLRALTDOWN:
		case KEY_CTRLRALTDOWN:
		case KEY_RCTRLALTDOWN:
		case KEY_CTRLALTNUMPAD2:
		case KEY_RCTRLALTNUMPAD2:
		case KEY_CTRLRALTNUMPAD2:
		case KEY_RCTRLRALTNUMPAD2:
			console.ScrollWindow(1);
			return true;

		case KEY_CTRLALTPGUP:
		case KEY_RCTRLRALTPGUP:
		case KEY_CTRLRALTPGUP:
		case KEY_RCTRLALTPGUP:
		case KEY_CTRLALTNUMPAD9:
		case KEY_RCTRLALTNUMPAD9:
		case KEY_CTRLRALTNUMPAD9:
		case KEY_RCTRLRALTNUMPAD9:
			console.ScrollWindow(-ScrY);
			return true;

		case KEY_CTRLALTHOME:
		case KEY_RCTRLRALTHOME:
		case KEY_CTRLRALTHOME:
		case KEY_RCTRLALTHOME:
		case KEY_CTRLALTNUMPAD7:
		case KEY_RCTRLALTNUMPAD7:
		case KEY_CTRLRALTNUMPAD7:
		case KEY_RCTRLRALTNUMPAD7:
			console.ScrollWindowToBegin();
			return true;

		case KEY_CTRLALTPGDN:
		case KEY_RCTRLRALTPGDN:
		case KEY_CTRLRALTPGDN:
		case KEY_RCTRLALTPGDN:
		case KEY_CTRLALTNUMPAD3:
		case KEY_RCTRLALTNUMPAD3:
		case KEY_CTRLRALTNUMPAD3:
		case KEY_RCTRLRALTNUMPAD3:
			console.ScrollWindow(ScrY);
			return true;

		case KEY_CTRLALTEND:
		case KEY_RCTRLRALTEND:
		case KEY_CTRLRALTEND:
		case KEY_RCTRLALTEND:
		case KEY_CTRLALTNUMPAD1:
		case KEY_RCTRLALTNUMPAD1:
		case KEY_CTRLRALTNUMPAD1:
		case KEY_RCTRLRALTNUMPAD1:
			console.ScrollWindowToEnd();
			return true;
		}
	}
	return false;
}

void InitConsole()
{
	static bool FirstInit = true;

	if (FirstInit)
	{
		CancelIoInProgress() = os::event(os::event::type::manual, os::event::state::nonsignaled);

		DWORD Mode;
		if(!console.GetMode(console.GetInputHandle(), Mode))
		{
			static const auto ConIn = os::OpenConsoleInputBuffer();
			SetStdHandle(STD_INPUT_HANDLE, ConIn.native_handle());
		}

		if(!console.GetMode(console.GetOutputHandle(), Mode))
		{
			static const auto ConOut = os::OpenConsoleActiveScreenBuffer();
			SetStdHandle(STD_OUTPUT_HANDLE, ConOut.native_handle());
			SetStdHandle(STD_ERROR_HANDLE, ConOut.native_handle());
		}

		Global->WindowManager->AddGlobalKeyHandler(ConsoleScrollHook);
	}

	console.SetControlHandler(CtrlHandler, true);

	console_mode Mode;
	console.GetMode(console.GetInputHandle(), Mode.Input);
	console.GetMode(console.GetOutputHandle(), Mode.Output);
	console.GetMode(console.GetErrorHandle(), Mode.Error);
	InitialConsoleMode = Mode;

	Global->strInitTitle = console.GetPhysicalTitle();
	console.GetWindowRect(InitWindowRect);
	console.GetSize(InitialSize);
	console.GetCursorInfo(InitialCursorInfo);

	if (FirstInit)
	{
		rectangle WindowRect;
		console.GetWindowRect(WindowRect);
		console.GetSize(InitSize);

		if(Global->Opt->WindowMode)
		{
			AdjustConsoleScreenBufferSize();
			console.ResetViewportPosition();
		}
		else
		{
			if (WindowRect.left || WindowRect.top || WindowRect.right != InitSize.x - 1 || WindowRect.bottom != InitSize.y - 1)
			{
				console.SetSize({ WindowRect.width(), WindowRect.height() });
				console.GetSize(InitSize);
			}
		}
		if (IsZoomed(console.GetWindow()))
		{
			ChangeVideoMode(true);
		}
		else
		{
			point CurrentSize;
			if (console.GetSize(CurrentSize))
			{
				SaveNonMaximisedBufferSize(CurrentSize);
			}
		}
	}


	SetFarConsoleMode();

	UpdateScreenSize();
	Global->ScrBuf->FillBuf();

	consoleicons::instance().set_icon();

	FirstInit = false;
}

void CloseConsole()
{
	Global->ScrBuf->Flush();
	MoveRealCursor(0, ScrY);
	console.SetCursorInfo(InitialCursorInfo);

	if (InitialConsoleMode)
	{
		ChangeConsoleMode(console.GetInputHandle(), InitialConsoleMode->Input);
		ChangeConsoleMode(console.GetOutputHandle(), InitialConsoleMode->Output);
		ChangeConsoleMode(console.GetErrorHandle(), InitialConsoleMode->Error);
	}

	console.SetTitle(Global->strInitTitle);
	console.SetSize(InitialSize);

	point CursorPos = {};
	console.GetCursorPosition(CursorPos);

	const auto Height = InitWindowRect.bottom - InitWindowRect.top;
	const auto Width = InitWindowRect.right - InitWindowRect.left;

	if (!in_closed_range(InitWindowRect.top, CursorPos.y, InitWindowRect.bottom))
		InitWindowRect.top = std::max(0, CursorPos.y - Height);

	if (!in_closed_range(InitWindowRect.left, CursorPos.x, InitWindowRect.right))
		InitWindowRect.left = std::max(0, CursorPos.x - Width);

	InitWindowRect.bottom = InitWindowRect.top + Height;
	InitWindowRect.right = InitWindowRect.left + Width;

	rectangle CurrentRect{};
	console.GetWindowRect(CurrentRect);
	if (CurrentRect != InitWindowRect)
	{
		console.SetWindowRect(InitWindowRect);
		console.SetSize(InitialSize);
	}

	ClearKeyQueue();
	consoleicons::instance().restore_icon();
	CancelIoInProgress().close();
}


void SetFarConsoleMode(bool SetsActiveBuffer)
{
	// Inherit existing mode. We don't want to build these flags from scratch,
	// as MS might introduce some new flags in future Windows versions.
	auto InputMode = InitialConsoleMode->Input;

	// We need this one unconditionally
	InputMode |= ENABLE_WINDOW_INPUT;

	// We don't need these guys unconditionally
	InputMode &= ~(ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_VIRTUAL_TERMINAL_INPUT);

	// And this one depends on interface settings
	if (Global->Opt->Mouse)
	{
		InputMode |= ENABLE_MOUSE_INPUT;

		// Setting ENABLE_MOUSE_INPUT is not enough, we must also clear the ENABLE_QUICK_EDIT_MODE
		InputMode |= ENABLE_EXTENDED_FLAGS;
		InputMode &= ~ENABLE_QUICK_EDIT_MODE;
	}
	else
	{
		InputMode &= ~ENABLE_MOUSE_INPUT;
	}

	// Feature: if window rect is in unusual position (shifted up or right) - enable mouse selection
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (Global->Opt->WindowMode
			&& GetConsoleScreenBufferInfo(console.GetOutputHandle(), &csbi)
			&& (csbi.srWindow.Bottom != csbi.dwSize.Y - 1 || csbi.srWindow.Left))
		{
			InputMode &= ~ENABLE_MOUSE_INPUT;
			InputMode |= ENABLE_EXTENDED_FLAGS | ENABLE_QUICK_EDIT_MODE;
		}
	}

	ChangeConsoleMode(console.GetInputHandle(), InputMode);

	if (SetsActiveBuffer)
		console.SetActiveScreenBuffer(console.GetOutputHandle());

	const auto OutputMode =
		InitialConsoleMode->Output |
		ENABLE_PROCESSED_OUTPUT |
		ENABLE_WRAP_AT_EOL_OUTPUT |
		(::console.IsVtSupported() && Global->Opt->VirtualTerminalRendering? ENABLE_VIRTUAL_TERMINAL_PROCESSING : 0);

	ChangeConsoleMode(console.GetOutputHandle(), OutputMode);
	ChangeConsoleMode(console.GetErrorHandle(), OutputMode);
}

bool ChangeConsoleMode(HANDLE ConsoleHandle, DWORD Mode)
{
	DWORD CurrentConsoleMode;
	if (console.GetMode(ConsoleHandle, CurrentConsoleMode) && CurrentConsoleMode == Mode)
		return true;

	return console.SetMode(ConsoleHandle, Mode);
}

void SaveConsoleWindowRect()
{
	console.GetWindowRect(windowholder_rect);
}

void RestoreConsoleWindowRect()
{
	rectangle WindowRect;
	console.GetWindowRect(WindowRect);
	if (WindowRect.width() < windowholder_rect.width() || WindowRect.height() < windowholder_rect.height())
	{
		console.SetWindowRect(windowholder_rect);
	}
}

void FlushInputBuffer()
{
	console.FlushInputBuffer();
	IntKeyState.MouseButtonState=0;
	IntKeyState.MouseEventFlags=0;
}

void SetVideoMode()
{
	if (!IsConsoleFullscreen() && Global->Opt->AltF9) // hardware full-screen check
	{
		DWORD dmode = 0;
		if (IsWindows10OrGreater() && console.GetDisplayMode(dmode) && (dmode & CONSOLE_FULLSCREEN) != 0)
			return; // ignore Alt-F9 in Win10 full-screen mode

		ChangeVideoMode(!IsZoomed(console.GetWindow()));
	}
	else
	{
		ChangeVideoMode(ScrY == 24 ? 50 : 25, 80);
	}
}

void ChangeVideoMode(bool Maximize)
{
	point coordScreen;

	if (Maximize)
	{
		SendMessage(console.GetWindow(),WM_SYSCOMMAND,SC_MAXIMIZE,0);
		coordScreen = console.GetLargestWindowSize();
		coordScreen.x += Global->Opt->ScrSize.DeltaX;
		coordScreen.y += Global->Opt->ScrSize.DeltaY;
	}
	else
	{
		SendMessage(console.GetWindow(),WM_SYSCOMMAND,SC_RESTORE,0);
		auto LastSize = GetNonMaximisedBufferSize();
		if (!LastSize.x && !LastSize.y)
		{
			// Not initialised yet - could happen if initial window state was maximiseds
			console.GetSize(LastSize);
		}
		coordScreen = LastSize;
	}

	ChangeVideoMode(coordScreen.y,coordScreen.x);
}

void ChangeVideoMode(int NumLines,int NumColumns)
{
	const short xSize = NumColumns, ySize = NumLines;

	point Size;
	console.GetSize(Size);

	rectangle srWindowRect;
	srWindowRect.right = xSize - 1;
	srWindowRect.bottom = ySize - 1;
	srWindowRect.left = srWindowRect.top = 0;

	point const coordScreen = { xSize, ySize };

	if (xSize > Size.x || ySize > Size.y)
	{
		if (Size.x < xSize - 1)
		{
			srWindowRect.right = Size.x - 1;
			console.SetWindowRect(srWindowRect);
			srWindowRect.right = xSize - 1;
		}

		if (Size.y < ySize - 1)
		{
			srWindowRect.bottom = Size.y - 1;
			console.SetWindowRect(srWindowRect);
			srWindowRect.bottom = ySize - 1;
		}

		console.SetSize(coordScreen);
	}

	if (!console.SetWindowRect(srWindowRect))
	{
		console.SetSize(coordScreen);
		console.SetWindowRect(srWindowRect);
	}
	else
	{
		console.SetSize(coordScreen);
	}

	UpdateScreenSize();
	GenerateWINDOW_BUFFER_SIZE_EVENT();
}

bool IsConsoleSizeChanged()
{
	point ConSize;
	console.GetSize(ConSize);
	// GetSize returns virtual size, so this covers WindowMode=true too
	return ConSize.y != ScrY + 1 || ConSize.x != ScrX + 1;
}

void GenerateWINDOW_BUFFER_SIZE_EVENT()
{
	INPUT_RECORD Rec{ WINDOW_BUFFER_SIZE_EVENT };
	size_t Writes;
	console.WriteInput({ &Rec, 1 }, Writes);
}

void UpdateScreenSize()
{
	point NewSize;
	if (!console.GetSize(NewSize))
		return;

	//чтоб решить баг винды приводящий к появлению скролов и т.п. после потери фокуса
	SaveConsoleWindowRect();

	CurSize = NewSize;
	ScrX = NewSize.x - 1;
	ScrY = NewSize.y - 1;

	if (PrevScrX == -1)
		PrevScrX = ScrX;

	if (PrevScrY == -1)
		PrevScrY = ScrY;

	Global->ScrBuf->AllocBuf(NewSize.y, NewSize.x);
}

void ShowTime()
{
	if (Global->SuppressClock)
		return;

	Global->CurrentTime.update();

	if (const auto CurrentWindow = Global->WindowManager->GetCurrentWindow())
	{
		// TODO: This is rubbish, consider moving the clock drawing to the ScrBuf
		if (Global->CurrentTime.size() < Global->LastShownTimeSize)
		{
			const auto CurrentClockPos = ScrX + 1 - static_cast<int>(Global->LastShownTimeSize);
			matrix<FAR_CHAR_INFO> Char(1, 1);
			Global->ScrBuf->Read({ CurrentClockPos - 1, 0, CurrentClockPos - 1, 0 }, Char);
			Global->ScrBuf->FillRect({ CurrentClockPos, 0, CurrentClockPos + static_cast<int>(Global->LastShownTimeSize), 0 }, Char[0][0]);
		}
		GotoXY(static_cast<int>(ScrX + 1 - Global->CurrentTime.size()), 0);
		const auto ModType = CurrentWindow->GetType();
		SetColor(ModType==windowtype_viewer?COL_VIEWERCLOCK:(ModType==windowtype_editor?COL_EDITORCLOCK:COL_CLOCK));
		Text(Global->CurrentTime.get());
		Global->LastShownTimeSize = Global->CurrentTime.size();
	}
}

void GotoXY(int X,int Y)
{
	CurX=X;
	CurY=Y;
}


int WhereX()
{
	return CurX;
}


int WhereY()
{
	return CurY;
}


void MoveCursor(point const Point)
{
	Global->ScrBuf->MoveCursor(Point);
}


point GetCursorPos()
{
	return Global->ScrBuf->GetCursorPos();
}

void SetCursorType(bool const Visible, size_t Size)
{
	if (Size == static_cast<size_t>(-1) || !Visible)
	{
		const size_t index = IsConsoleFullscreen()? 1 : 0;
		Size = Global->Opt->CursorSize[index]? static_cast<int>(Global->Opt->CursorSize[index]) : InitialCursorInfo.dwSize;
	}
	Global->ScrBuf->SetCursorType(Visible, Size);
}

void SetInitialCursorType()
{
	Global->ScrBuf->SetCursorType(InitialCursorInfo.bVisible!=FALSE,InitialCursorInfo.dwSize);
}


void GetCursorType(bool& Visible, size_t& Size)
{
	Global->ScrBuf->GetCursorType(Visible,Size);
}


void MoveRealCursor(int X,int Y)
{
	console.SetCursorPosition({ X, Y });
}

void Text(point Where, const FarColor& Color, string_view const Str)
{
	CurColor=Color;
	CurX = Where.x;
	CurY = Where.y;
	Text(Str);
}

void Text(string_view const Str)
{
	if (Str.empty())
		return;

	std::vector<FAR_CHAR_INFO> Buffer;
	Buffer.reserve(Str.size());
	std::transform(ALL_CONST_RANGE(Str), std::back_inserter(Buffer), [](wchar_t c) { return FAR_CHAR_INFO{ c, CurColor }; });

	Global->ScrBuf->Write(CurX, CurY, Buffer);
	CurX += static_cast<int>(Buffer.size());
}


void Text(lng MsgId)
{
	Text(msg(MsgId));
}

void VText(string_view const Str)
{
	if (Str.empty())
		return;

	const auto StartCurX = CurX;

	for (const auto i: Str)
	{
		GotoXY(CurX, CurY);
		Text(i);
		++CurY;
		CurX = StartCurX;
	}
}

static void HiTextBase(string_view const Str, function_ref<void(string_view, bool)> const TextHandler, function_ref<void(wchar_t)> const HilightHandler)
{
	bool Unescape = false;
	for (size_t Offset = 0;;)
	{
		const auto AmpBegin = Str.find(L'&', Offset);
		if (AmpBegin == string::npos)
		{
			TextHandler(Str, Offset != 0);
			return;
		}

		/*
		&&      = '&'
		&&&     = '&'
		           ^H
		&&&&    = '&&'
		&&&&&   = '&&'
		           ^H
		&&&&&&  = '&&&'
		&&&&&&& = '&&&'
		           ^H
		*/

		auto AmpEnd = Str.find_first_not_of(L'&', AmpBegin);
		if (AmpEnd == string::npos)
			AmpEnd = Str.size();

		if (!((AmpEnd - AmpBegin) & 1))
		{
			Offset = AmpEnd;
			Unescape = true;
			continue;
		}

		if (AmpBegin)
			TextHandler(Str.substr(0, AmpBegin), Unescape);

		if (AmpBegin + 1 == Str.size())
			return;

		HilightHandler(Str[AmpBegin + 1]);

		if (AmpBegin + 2 == Str.size())
			return;

		const auto HiAmpCollapse = Str[AmpBegin + 1] == L'&' && Str[AmpBegin + 2] == L'&';
		const auto Tail = Str.substr(AmpBegin + (HiAmpCollapse ? 3 : 2));
		TextHandler(Tail, Tail.find(L'&') != Tail.npos);

		return;
	}
}


static size_t unescape(string_view const Str, function_ref<bool(wchar_t)> const PutChar)
{
	bool LastAmpersand = false;

	for (const auto& i: Str)
	{
		if (i == L'&')
		{
			if (!LastAmpersand)
			{
				LastAmpersand = true;
			}
			else
			{
				if (!PutChar(i))
					return &i - Str.data();

				LastAmpersand = false;
			}
		}
		else
		{
			if (!PutChar(i))
				return &i - Str.data();

			LastAmpersand = false;
		}
	}

	return Str.size();
}

class text_unescape
{
public:
	explicit text_unescape(function_ref<void(string_view)> const PutString, function_ref<bool(wchar_t)> const PutChar, function_ref<void()> const Commit):
		m_PutString(PutString),
		m_PutChar(PutChar),
		m_Commit(Commit)
	{
	}

	void operator()(string_view const Str, bool const Unescape) const
	{
		if (!Unescape)
			return m_PutString(Str);

		unescape(Str, m_PutChar);
		m_Commit();
	}

private:
	function_ref<void(string_view)> m_PutString;
	function_ref<bool(wchar_t)> m_PutChar;
	function_ref<void()> m_Commit;
};

void HiText(string_view const Str,const FarColor& HiColor, bool const isVertText)
{
	using text_func = void (*)(string_view);
	const text_func fText = Text, fVText = VText; //BUGBUG
	const auto TextFunc  = isVertText ? fVText : fText;

	string Buffer;
	const auto PutChar = [&](wchar_t const Ch){ Buffer.push_back(Ch); return true; };
	const auto Commit = [&]{ TextFunc(Buffer); Buffer.clear(); };

	HiTextBase(Str, text_unescape(TextFunc, PutChar, Commit), [&TextFunc, &HiColor](wchar_t c)
	{
		const auto SaveColor = CurColor;
		SetColor(HiColor);
		TextFunc({ &c, 1 });
		SetColor(SaveColor);
	});
}

string HiText2Str(string_view const Str, size_t* HotkeyVisualPos)
{
	string Result;

	if (HotkeyVisualPos)
		*HotkeyVisualPos = string::npos;

	const auto PutString = [&](string_view const s){ Result += s; };
	const auto PutChar = [&](wchar_t const Ch){ Result.push_back(Ch); return true; };

	HiTextBase(Str, text_unescape(PutString, PutChar, []{}), [&](wchar_t const Ch)
	{
		if (HotkeyVisualPos)
			*HotkeyVisualPos = Result.size();
		(void)PutChar(Ch);
	});

	return Result;
}

bool HiTextHotkey(string_view Str, wchar_t& Hotkey, size_t* HotkeyVisualPos)
{
	bool Result = false;

	size_t Size{};

	const auto PutString = [&](string_view const s) { Size += s.size(); };
	const auto PutChar = [&](wchar_t const Ch) { ++Size; return true; };

	HiTextBase(Str, text_unescape(PutString, PutChar, []{}), [&](wchar_t const Ch)
	{
		Hotkey = Ch;
		if (HotkeyVisualPos)
			*HotkeyVisualPos = Size;
		Result = true;
	});

	return Result;
}

// removes single '&', turns '&&' into '&'
void RemoveHighlights(string& Str)
{
	auto Iterator = Str.begin();
	unescape(Str, [&](wchar_t Ch){ *Iterator = Ch; ++Iterator; return true; });
	Str.resize(Iterator - Str.begin());
}

void inplace::escape_ampersands(string& Str)
{
	replace(Str, L"&"sv, L"&&"sv);
}

string escape_ampersands(string_view const Str)
{
	string Copy(Str);
	inplace::escape_ampersands(Copy);
	return Copy;
}

void SetScreen(rectangle const Where, wchar_t Ch, const FarColor& Color)
{
	Global->ScrBuf->FillRect(Where, { Ch, Color });
}

void MakeShadow(rectangle const Where)
{
	Global->ScrBuf->ApplyShadow(Where);
}

void ChangeBlockColor(rectangle const Where, const FarColor& Color)
{
	Global->ScrBuf->ApplyColor(Where, Color, true);
}

void SetColor(int Color)
{
	CurColor = colors::ConsoleColorToFarColor(Color);
}

void SetColor(PaletteColors Color)
{
	CurColor=colors::PaletteColorToFarColor(Color);
}

void SetColor(const FarColor& Color)
{
	CurColor=Color;
}

void SetRealColor(const FarColor& Color)
{
	console.SetTextAttributes(Color);
}

void ClearScreen(const FarColor& Color)
{
	Global->ScrBuf->FillRect({ 0, 0, ScrX, ScrY }, { L' ', Color });
	if(Global->Opt->WindowMode)
	{
		console.ClearExtraRegions(Color, CR_BOTH);
	}
	Global->ScrBuf->Flush();
	console.SetTextAttributes(Color);
}

const FarColor& GetColor()
{
	return CurColor;
}


void ScrollScreen(int Count)
{
	Global->ScrBuf->Scroll(Count);
}

bool DoWeReallyHaveToScroll(short Rows)
{
	/*
	Q: WTF is this magic?
	A: The whole point of scrolling here is to move the output up to make room for:
		- an empty line after the output
		- prompt
		- keybar (optional).

	Sometimes the output happens at the very top of the buffer (say, a bat file that does 'cls' before anything else),
	or just ends with a few empty lines so there could be enough room for us already, in which case there's no point in scrolling it further.

	This function reads the specified number of the last lines from the screen buffer and checks if there's anything else in them but spaces.
	*/

	rectangle const Region{ 0, ScrY - Rows + 1, ScrX, ScrY };

	// TODO: matrix_view to avoid copying
	matrix<FAR_CHAR_INFO> BufferBlock(Rows, ScrX + 1);
	Global->ScrBuf->Read(Region, BufferBlock);

	return !std::all_of(ALL_CONST_RANGE(BufferBlock.vector()), [](const FAR_CHAR_INFO& i) { return i.Char == L' '; });
}

void GetText(rectangle Where, matrix<FAR_CHAR_INFO>& Dest)
{
	Global->ScrBuf->Read(Where, Dest);
}

void PutText(rectangle Where, const FAR_CHAR_INFO *Src)
{
	const size_t Width = Where.width();
	for (int Y = Where.top; Y <= Where.bottom; ++Y, Src += Width)
		Global->ScrBuf->Write(Where.left, Y, { Src, Width });
}

void BoxText(string_view const Str, bool const IsVert)
{
	IsVert? VText(Str) : Text(Str);
}

/*
   Отрисовка прямоугольника.
*/
void Box(rectangle Where, const FarColor& Color, int Type)
{
	if (Where.left >= Where.right || Where.top >= Where.bottom)
		return;

	enum line { LineV, LineH, LineLT, LineRT, LineLB, LineRB, LineCount };

	static const BOX_DEF_SYMBOLS BoxInit[][LineCount] =
	{
		{ BS_V1, BS_H1, BS_LT_H1V1, BS_RT_H1V1, BS_LB_H1V1, BS_RB_H1V1, },
		{ BS_V2, BS_H2, BS_LT_H2V2, BS_RT_H2V2, BS_LB_H2V2, BS_RB_H2V2, },
	};

	const auto Box = BoxInit[(Type == DOUBLE_BOX || Type == SHORT_DOUBLE_BOX)? 1 : 0];
	const auto Symbol = [Box](line Line) { return BoxSymbols[Box[Line]]; };

	SetColor(Color);

	string Buffer(Where.height() - 2, Symbol(LineV));

	GotoXY(Where.left, Where.top + 1);
	VText(Buffer);

	GotoXY(Where.right, Where.top + 1);
	VText(Buffer);

	Buffer.assign(Where.width(), Symbol(LineH));
	Buffer.front() = Symbol(LineLT);
	Buffer.back() = Symbol(LineRT);

	GotoXY(Where.left, Where.top);
	Text(Buffer);

	Buffer.front() = Symbol(LineLB);
	Buffer.back() = Symbol(LineRB);

	GotoXY(Where.left, Where.bottom);
	Text(Buffer);
}

bool ScrollBarRequired(size_t Length, unsigned long long ItemsCount)
{
	return Length >= 2 && ItemsCount && Length<ItemsCount;
}

bool ScrollBar(size_t X1, size_t Y1, size_t Length, unsigned long long TopItem, unsigned long long ItemsCount)
{
	return ScrollBarRequired(Length, ItemsCount) && ScrollBarEx(X1, Y1, Length, TopItem, TopItem + Length, ItemsCount);
}

bool ScrollBarEx(size_t X1, size_t Y1, size_t Length, unsigned long long Start, unsigned long long End, unsigned long long Size)
{
	if ( Length < 2)
		return false;

	string Buffer(Length, BoxSymbols[BS_X_B0]);
	Buffer.front() = L'\x25B2';
	Buffer.back() = L'\x25BC';

	const auto FieldBegin = Buffer.begin() + 1;
	const auto FieldEnd = Buffer.end() - 1;
	const size_t FieldSize = FieldEnd - FieldBegin;

	End = std::min(End, Size);

	auto SliderBegin = FieldBegin, SliderEnd = SliderBegin;

	if (Size && Start < End)
	{
		const auto SliderSize = std::max(1ull, (End - Start) * FieldSize / Size);

		if (SliderSize >= FieldSize)
		{
			SliderBegin = FieldBegin;
			SliderEnd = FieldEnd;
		}
		else if (End >= Size)
		{
			SliderBegin = FieldEnd - SliderSize;
			SliderEnd = FieldEnd;
		}
		else
		{
			SliderBegin = std::min(FieldBegin + Start * FieldSize / Size, FieldEnd);
			SliderEnd = std::min(SliderBegin + SliderSize, FieldEnd);
		}
	}

	std::fill(SliderBegin, SliderEnd, BoxSymbols[BS_X_DB]);

	GotoXY(static_cast<int>(X1), static_cast<int>(Y1));
	VText(Buffer);

	return true;
}

string MakeLine(int const Length, line_type const Type, string_view const UserLine)
{
	if (Length < 2)
		return {};

	// left-center-right or top-center-bottom
	static const size_t Predefined[][3]
	{
		{BS_H1,      BS_H1,  BS_H1},     // "───"  h1
		{BS_H2,      BS_H2,  BS_H2},     // "═══"  h2
		{BS_SPACE,   BS_H1,  BS_SPACE},  // " ─ "  h1_to_none
		{BS_SPACE,   BS_H2,  BS_SPACE},  // " ═ "  h2_to_none
		{BS_L_H1V1,  BS_H1,  BS_R_H1V1}, // "├─┤"  h1_to_v1
		{BS_L_H1V2,  BS_H1,  BS_R_H1V2}, // "╟─╢"  h1_to_v2
		{BS_L_H2V1,  BS_H2,  BS_R_H2V1}, // "╞═╡"  h2_to_v1
		{BS_L_H2V2,  BS_H2,  BS_R_H2V2}, // "╠═╣"  h2_to_v2
		{BS_L_H1V2,  BS_H1,  BS_R_H1V2}, // "╟─╢"  h_user, h1_to_v2 by default

		{BS_V1,      BS_V1,  BS_V1},     // "|||"  v1
		{BS_V2,      BS_V2,  BS_V2},     // "║║║"  v2
		{BS_SPACE,   BS_V1,  BS_SPACE},  // " | "  v1_to_none
		{BS_SPACE,   BS_V1,  BS_SPACE},  // " ║ "  v2_to_none
		{BS_T_H1V1,  BS_V1,  BS_B_H1V1}, // "┬│┴"  v1_to_h1
		{BS_T_H2V1,  BS_V1,  BS_B_H2V1}, // "╤│╧"  v1_to_h2
		{BS_T_H1V2,  BS_V2,  BS_B_H1V2}, // "╥║╨"  v2_to_h1
		{BS_T_H2V2,  BS_V2,  BS_B_H2V2}, // "╦║╩"  v2_to_h2
		{BS_T_H2V1,  BS_V1,  BS_B_H2V1}, // "╤│╧"  v_user, v1_to_h2 by default
	};

	static_assert(std::size(Predefined) == static_cast<size_t>(line_type::count));

	wchar_t Buffer[3];

	if ((Type == line_type::h_user || Type == line_type::v_user) && !UserLine.empty())
	{
		const auto Size = std::min(UserLine.size(), std::size(Buffer));
		std::copy_n(UserLine.cbegin(), Size, std::begin(Buffer));
		std::fill(std::begin(Buffer) + Size, std::end(Buffer), L' ');
	}
	else
	{
		std::transform(ALL_CONST_RANGE(Predefined[static_cast<size_t>(Type)]), Buffer, [](size_t i){ return BoxSymbols[i]; });
	}

	string Result(Length, Buffer[1]);
	Result.front() = Buffer[0];
	Result.back() = Buffer[2];

	return Result;
}

void DrawLine(int const Length, line_type const Type, string_view const UserLine)
{
	if (Length < 2)
		return;

	const auto Line = MakeLine(Length, Type, UserLine);

	Type == line_type::v1_to_none ||
		Type == line_type::v1_to_h2 ||
		Type == line_type::v1_to_h1 ||
		Type == line_type::v2_to_h2 ||
		Type == line_type::v1 ||
		Type == line_type::v2 ||
		Type == line_type::v_user ?
		VText(Line) :
		Text(Line);
}

string make_progressbar(size_t Size, size_t Percent, bool ShowPercent, bool PropagateToTasbkar)
{
	string StrPercent;
	if (ShowPercent)
	{
		StrPercent = format(FSTR(L" {0:3}%"), Percent);
		Size = Size > StrPercent.size()? Size - StrPercent.size(): 0;
	}
	string Str(Size, BoxSymbols[BS_X_B0]);
	const auto Pos = std::min(Percent, size_t(100)) * Size / 100;
	std::fill_n(Str.begin(), Pos, BoxSymbols[BS_X_DB]);
	if (ShowPercent)
	{
		Str += StrPercent;
	}
	if (PropagateToTasbkar)
	{
		taskbar::instance().set_value(Percent, 100);
	}
	return Str;
}

size_t HiStrlen(string_view const Str)
{
	size_t Result = 0;
	unescape(Str, [&](wchar_t){ ++Result; return true; });
	return Result;
}

size_t HiFindRealPos(string_view const Str, size_t const Pos)
{
	size_t Unescaped = 0;
	return unescape(Str, [&](wchar_t)
	{
		if (Unescaped == Pos)
			return false;

		++Unescaped;
		return true;
	});
}

bool IsConsoleFullscreen()
{
	static const auto Supported = console.IsFullscreenSupported();
	if (!Supported)
		return false;

	DWORD ModeFlags=0;
	return console.GetDisplayMode(ModeFlags) && ModeFlags & CONSOLE_FULLSCREEN_HARDWARE;
}

void fix_coordinates(rectangle& Where)
{
	Where.left = std::clamp(Where.left, 0, static_cast<int>(ScrX));
	Where.top = std::clamp(Where.top, 0, static_cast<int>(ScrY));
	Where.right = std::clamp(Where.right, 0, static_cast<int>(ScrX));
	Where.bottom = std::clamp(Where.bottom, 0, static_cast<int>(ScrY));
}

void AdjustConsoleScreenBufferSize()
{
	if (!Global->Opt->WindowMode)
		return;

	point Size;
	if (!console.GetSize(Size))
		return;

	if (!Global->Opt->WindowModeStickyX || !Global->Opt->WindowModeStickyY)
	{
		// TODO: Do not use console functions directly
		// Add a way to bypass console buffer abstraction layer
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (GetConsoleScreenBufferInfo(console.GetOutputHandle(), &csbi))
		{
			if (!Global->Opt->WindowModeStickyX)
			{
				Size.x = csbi.dwSize.X;
			}
			if (!Global->Opt->WindowModeStickyY)
			{
				Size.y = csbi.dwSize.Y;
			}
		}
	}

	console.SetScreenBufferSize(Size);
}

static point& NonMaximisedBufferSize()
{
	static point s_Size;
	return s_Size;
}

void SaveNonMaximisedBufferSize(point const& Size)
{
	// We can't trust the size that windows sets automatically after restoring the window -
	// it could be less than previous because of horizontal scrollbar

	// TODO: this might also fix the issue with exiting from the fullscreen mode on Windows 10,
	// need to check and remove corresponding workarounds if it works
	NonMaximisedBufferSize() = Size;
}

point GetNonMaximisedBufferSize()
{
	return NonMaximisedBufferSize();
}

bool ConsoleYesNo(string_view const Message, bool const Default)
{
	{
		// The output can be redirected
		DWORD Mode;
		if (!console.GetMode(console.GetOutputHandle(), Mode))
			return Default;
	}

	if (InitialConsoleMode)
	{
		ChangeConsoleMode(console.GetInputHandle(), InitialConsoleMode->Input);
		ChangeConsoleMode(console.GetOutputHandle(), InitialConsoleMode->Output);
		ChangeConsoleMode(console.GetErrorHandle(), InitialConsoleMode->Error);
	}

	for (;;)
	{
		std::wcout << L'\n' << Message << L" (Y/N)? "sv << std::flush;

		wchar_t Input;
		std::wcin.clear();
		std::wcin.get(Input).ignore(std::numeric_limits<std::streamsize>::max(), L'\n');

		switch (upper(Input))
		{
		case L'Y':
			return true;

		case L'N':
			return false;

		default:
			break;
		}
	}
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("interf.highlight")
{
	const auto np = string::npos;

	static const struct
	{
		string_view Input;
		string_view Result;
		wchar_t Hotkey;
		size_t HotkeyVisualPos;
		struct
		{
			size_t PosVisual;
			size_t PosReal;
		};
	}
	Tests[]
	{
		{ {},                {},             0,     np, {  0,  0 }, },
		{ L"1"sv,            L"1"sv,         0,     np, {  0,  0 }, },
		{ L"&"sv,            {},             0,     np, {  0,  1 }, },
		{ L"1&2"sv,          L"12"sv,        L'2',   1, {  1,  2 }, },
		{ L"&1"sv,           L"1"sv,         L'1',   0, {  0,  1 }, },
		{ L"1&"sv,           L"1"sv,         0,     np, {  0,  0 }, },
		{ L"&1&"sv,          L"1"sv,         L'1',   0, {  0,  1 }, },
		{ L"&1&2"sv,         L"12"sv,        L'1',   0, {  1,  3 }, },
		{ L"&&"sv,           L"&"sv,         0,     np, {  0,  1 }, },
		{ L"1&&"sv,          L"1&"sv,        0,     np, {  1,  2 }, },
		{ L"&&1"sv,          L"&1"sv,        0,     np, {  1,  2 }, },
		{ L"1&&2"sv,         L"1&2"sv,       0,     np, {  2,  3 }, },
		{ L"&&&"sv,          L"&"sv,         L'&',   0, {  0,  1 }, },
		{ L"&&&1"sv,         L"&1"sv,        L'&',   0, {  1,  3 }, },
		{ L"1&&&"sv,         L"1&"sv,        L'&',   1, {  0,  0 }, },
		{ L"1&&&2"sv,        L"1&2"sv,       L'&',   1, {  0,  0 }, },
		{ L"&1&&&2"sv,       L"1&2"sv,       L'1',   0, {  0,  1 }, },
		{ L"&1&2&3&"sv,      L"123"sv,       L'1',   0, {  2,  5 }, },
	};

	for (const auto& i: Tests)
	{
		size_t HotkeyPos{};
		REQUIRE(HiText2Str(i.Input, &HotkeyPos) == i.Result);
		REQUIRE(HotkeyPos == i.HotkeyVisualPos);

		wchar_t Hotkey{};
		HotkeyPos = np;
		REQUIRE(HiTextHotkey(i.Input, Hotkey, &HotkeyPos) == (i.Hotkey != 0));
		REQUIRE(Hotkey == i.Hotkey);
		REQUIRE(HotkeyPos == i.HotkeyVisualPos);

		REQUIRE(HiStrlen(i.Input) == i.Result.size());

		REQUIRE(HiFindRealPos(i.Input, i.PosVisual) == i.PosReal);
	}
}
#endif
