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

void consoleicons::setFarIcons()
{
	if (!Global->Opt->SetIcon)
		return;

	const auto hWnd = console.GetWindow();
	if (!hWnd)
		return;

	if (!m_Loaded)
	{
		const int IconId = (Global->Opt->SetAdminIcon && os::security::is_admin())? FAR_ICON_RED : FAR_ICON;

		m_Large.Icon = load_icon(IconId, m_Large.IsBig);
		m_Small.Icon = load_icon(IconId, m_Small.IsBig);
		m_Loaded = true;
	}

	const auto Set = [hWnd](icon& Icon)
	{
		if (Icon.Icon)
		{
			Icon.PreviousIcon = set_icon(hWnd, Icon.IsBig, Icon.Icon);
			Icon.Changed = true;
		}
	};

	Set(m_Large);
	Set(m_Small);
}

void consoleicons::restorePreviousIcons()
{
	if (!Global->Opt->SetIcon)
		return;

	const auto hWnd = console.GetWindow();
	if (!hWnd)
		return;

	const auto Restore = [hWnd](icon& Icon)
	{
		if (!Icon.Changed)
			return;

		set_icon(hWnd, Icon.IsBig, Icon.PreviousIcon);
		Icon.Changed = false;
	};

	Restore(m_Large);
	Restore(m_Small);
}

static int CurX,CurY;
static FarColor CurColor;

static CONSOLE_CURSOR_INFO InitialCursorInfo;

static SMALL_RECT windowholder_rect;

WCHAR BoxSymbols[BS_COUNT];

COORD InitSize={};
COORD CurSize={};
SHORT ScrX=0,ScrY=0;
SHORT PrevScrX=-1,PrevScrY=-1;
std::optional<console_mode> InitialConsoleMode;
static SMALL_RECT InitWindowRect;
static COORD InitialSize;

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
			os::thread(&os::thread::detach, &CancelSynchronousIoWrapper, Global->MainThreadHandle());
		}
		WriteInput(KEY_BREAK);

		if (Global->CtrlObject && Global->CtrlObject->Cp())
		{
			if (Global->CtrlObject->Cp()->LeftPanel() && Global->CtrlObject->Cp()->LeftPanel()->GetMode() == panel_mode::PLUGIN_PANEL)
				Global->CtrlObject->Plugins->ProcessEvent(Global->CtrlObject->Cp()->LeftPanel()->GetPluginHandle(),FE_BREAK, ToPtr(CtrlType));

			if (Global->CtrlObject->Cp()->RightPanel() && Global->CtrlObject->Cp()->RightPanel()->GetMode() == panel_mode::PLUGIN_PANEL)
				Global->CtrlObject->Plugins->ProcessEvent(Global->CtrlObject->Cp()->RightPanel()->GetPluginHandle(),FE_BREAK, ToPtr(CtrlType));
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
		SMALL_RECT WindowRect;
		console.GetWindowRect(WindowRect);
		console.GetSize(InitSize);

		if(Global->Opt->WindowMode)
		{
			AdjustConsoleScreenBufferSize();
			console.ResetViewportPosition();
		}
		else
		{
			if (WindowRect.Left || WindowRect.Top || WindowRect.Right != InitSize.X - 1 || WindowRect.Bottom != InitSize.Y - 1)
			{
				COORD newSize;
				newSize.X = WindowRect.Right - WindowRect.Left + 1;
				newSize.Y = WindowRect.Bottom - WindowRect.Top + 1;
				console.SetSize(newSize);
				console.GetSize(InitSize);
			}
		}
		if (IsZoomed(console.GetWindow()))
		{
			ChangeVideoMode(true);
		}
		else
		{
			COORD CurrentSize;
			if (console.GetSize(CurrentSize))
			{
				SaveNonMaximisedBufferSize(CurrentSize);
			}
		}
	}


	SetFarConsoleMode();

	UpdateScreenSize();
	Global->ScrBuf->FillBuf();

	consoleicons::instance().setFarIcons();

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

	COORD CursorPos = {};
	console.GetCursorPosition(CursorPos);
	const auto Height = InitWindowRect.Bottom-InitWindowRect.Top;
	const auto Width = InitWindowRect.Right-InitWindowRect.Left;
	if (CursorPos.Y > InitWindowRect.Bottom || CursorPos.Y < InitWindowRect.Top)
		InitWindowRect.Top = std::max(0, CursorPos.Y-Height);
	if (CursorPos.X > InitWindowRect.Right || CursorPos.X < InitWindowRect.Left)
		InitWindowRect.Left = std::max(0, CursorPos.X-Width);
	InitWindowRect.Bottom = InitWindowRect.Top + Height;
	InitWindowRect.Right = InitWindowRect.Left + Width;

	SMALL_RECT CurrentRect{};
	console.GetWindowRect(CurrentRect);
	if (CurrentRect.Left != InitWindowRect.Left ||
		CurrentRect.Top != InitWindowRect.Top ||
		CurrentRect.Right != InitWindowRect.Right ||
		CurrentRect.Bottom != InitWindowRect.Bottom)
	{
		console.SetWindowRect(InitWindowRect);
		console.SetSize(InitialSize);
	}

	ClearKeyQueue();
	consoleicons::instance().restorePreviousIcons();
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

	static bool VirtualTerminalAttempted = false;
	static bool VirtualTerminalSupported = false;

	auto OutputMode = InitialConsoleMode->Output;

	OutputMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT;

	if (VirtualTerminalSupported && Global->Opt->VirtualTerminalRendering)
		OutputMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

	ChangeConsoleMode(console.GetOutputHandle(), OutputMode);
	ChangeConsoleMode(console.GetErrorHandle(), OutputMode);


	if (Global->Opt->VirtualTerminalRendering)
		OutputMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

	if (!VirtualTerminalAttempted)
	{
		VirtualTerminalAttempted = true;

		const auto ResultOut = ChangeConsoleMode(console.GetOutputHandle(), OutputMode);
		const auto ResultErr = ChangeConsoleMode(console.GetErrorHandle(), OutputMode);

		VirtualTerminalSupported = ResultOut || ResultErr;
	}
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
	SMALL_RECT WindowRect;
	console.GetWindowRect(WindowRect);
	if(WindowRect.Right-WindowRect.Left<windowholder_rect.Right-windowholder_rect.Left ||
		WindowRect.Bottom-WindowRect.Top<windowholder_rect.Bottom-windowholder_rect.Top)
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
	COORD coordScreen;

	if (Maximize)
	{
		SendMessage(console.GetWindow(),WM_SYSCOMMAND,SC_MAXIMIZE,0);
		coordScreen = console.GetLargestWindowSize();
		coordScreen.X+=Global->Opt->ScrSize.DeltaX;
		coordScreen.Y+=Global->Opt->ScrSize.DeltaY;
	}
	else
	{
		SendMessage(console.GetWindow(),WM_SYSCOMMAND,SC_RESTORE,0);
		auto LastSize = GetNonMaximisedBufferSize();
		if (!LastSize.X && !LastSize.Y)
		{
			// Not initialised yet - could happen if initial window state was maximiseds
			console.GetSize(LastSize);
		}
		coordScreen = LastSize;
	}

	ChangeVideoMode(coordScreen.Y,coordScreen.X);
}

void ChangeVideoMode(int NumLines,int NumColumns)
{
	const short xSize = NumColumns, ySize = NumLines;

	COORD Size;
	console.GetSize(Size);

	SMALL_RECT srWindowRect;
	srWindowRect.Right = xSize-1;
	srWindowRect.Bottom = ySize-1;
	srWindowRect.Left = srWindowRect.Top = 0;

	const COORD coordScreen = {xSize, ySize};

	if (xSize>Size.X || ySize > Size.Y)
	{
		if (Size.X < xSize-1)
		{
			srWindowRect.Right = Size.X - 1;
			console.SetWindowRect(srWindowRect);
			srWindowRect.Right = xSize-1;
		}

		if (Size.Y < ySize-1)
		{
			srWindowRect.Bottom=Size.Y - 1;
			console.SetWindowRect(srWindowRect);
			srWindowRect.Bottom = ySize-1;
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
	COORD ConSize;
	console.GetSize(ConSize);
	// GetSize returns virtual size, so this covers WindowMode=true too
	return ConSize.Y != ScrY + 1 || ConSize.X != ScrX + 1;
}

void GenerateWINDOW_BUFFER_SIZE_EVENT()
{
	INPUT_RECORD Rec{ WINDOW_BUFFER_SIZE_EVENT };
	size_t Writes;
	console.WriteInput({ &Rec, 1 }, Writes);
}

void UpdateScreenSize()
{
	COORD NewSize;
	if (!console.GetSize(NewSize))
		return;

	//чтоб решить баг винды приводящий к появлению скролов и т.п. после потери фокуса
	SaveConsoleWindowRect();

	CurSize = NewSize;
	ScrX = NewSize.X - 1;
	ScrY = NewSize.Y - 1;

	if (PrevScrX == -1)
		PrevScrX = ScrX;

	if (PrevScrY == -1)
		PrevScrY = ScrY;

	Global->ScrBuf->AllocBuf(NewSize.Y, NewSize.X);
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

void SetCursorType(bool Visible, DWORD Size)
{
	if (Size == static_cast<DWORD>(-1) || !Visible)
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


void GetCursorType(bool& Visible, DWORD& Size)
{
	Global->ScrBuf->GetCursorType(Visible,Size);
}


void MoveRealCursor(int X,int Y)
{
	console.SetCursorPosition({ static_cast<SHORT>(X),static_cast<SHORT>(Y) });
}


void GetRealCursorPos(SHORT& X,SHORT& Y)
{
	COORD CursorPosition;
	console.GetCursorPosition(CursorPosition);
	X=CursorPosition.X;
	Y=CursorPosition.Y;
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

static void HiTextBase(const string& Str, function_ref<void(const string&)> const TextHandler, function_ref<void(wchar_t)> const HilightHandler)
{
	const auto AmpBegin = Str.find(L'&');
	if (AmpBegin != string::npos)
	{
		/*
		&&      = '&'
		&&&     = '&'
		^H
		&&&&    = '&&'
		&&&&&   = '&&'
		^H
		&&&&&&  = '&&&'
		*/

		auto AmpEnd = Str.find_first_not_of(L'&', AmpBegin);
		if (AmpEnd == string::npos)
			AmpEnd = Str.size();

		if ((AmpEnd - AmpBegin) & 1) // нечет?
		{
			TextHandler(Str.substr(0, AmpBegin));

			if (AmpBegin + 1 != Str.size())
			{
				HilightHandler(Str[AmpBegin + 1]);

				string RightPart = Str.substr(AmpBegin + 1);
				replace(RightPart, L"&&"sv, L"&"sv);
				TextHandler(RightPart.substr(1));
			}
		}
		else
		{
			string StrCopy(Str);
			replace(StrCopy, L"&&"sv, L"&"sv);
			TextHandler(StrCopy);
		}
	}
	else
	{
		TextHandler(Str);
	}
}

void HiText(const string& Str,const FarColor& HiColor,int isVertText)
{
	using text_func = void (*)(string_view);
	const text_func fText = Text, fVText = VText; //BUGBUG
	const auto TextFunc  = isVertText ? fVText : fText;

	HiTextBase(Str, [&TextFunc](const string& s){ TextFunc(s); }, [&TextFunc, &HiColor](wchar_t c)
	{
		const auto SaveColor = CurColor;
		SetColor(HiColor);
		TextFunc({ &c, 1 });
		SetColor(SaveColor);
	});
}

string HiText2Str(const string& Str)
{
	string Result;
	HiTextBase(Str, [&Result](const string& s){ Result += s; }, [&Result](wchar_t c) { Result += c; });
	return Result;
}

// removes single '&', turns '&&' into '&'
void RemoveHighlights(string& Str)
{
	const auto Target = L'&';
	auto pos = Str.find(Target);
	if (pos != string::npos)
	{
		auto pos1 = pos;
		const auto len = Str.size();
		while (pos < len)
		{
			++pos;
			if (pos < len && Str[pos] == Target)
			{
				Str[pos1++] = Target;
				++pos;
			}
			while (pos < len && Str[pos] != Target)
				Str[pos1++] = Str[pos++];
		}
		Str.resize(pos1);
	}
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

bool ScrollBarRequired(UINT Length, unsigned long long ItemsCount)
{
	return Length >= 2 && ItemsCount && Length<ItemsCount;
}

bool ScrollBarEx(UINT X1, UINT Y1, UINT Length, unsigned long long TopItem, unsigned long long ItemsCount)
{
	return ScrollBarRequired(Length, ItemsCount) && ScrollBarEx3(X1, Y1, Length, TopItem,TopItem+Length,ItemsCount);
}

bool ScrollBarEx3(UINT X1, UINT Y1, UINT Length, unsigned long long Start, unsigned long long End, unsigned long long Size)
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
		const auto SliderSize = std::max(1u, static_cast<UINT>(((End - Start) * FieldSize) / Size));

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
			SliderBegin = std::min(FieldBegin + static_cast<UINT>((Start*FieldSize) / Size), FieldEnd);
			SliderEnd = std::min(SliderBegin + SliderSize, FieldEnd);
		}
	}

	std::fill(SliderBegin, SliderEnd, BoxSymbols[BS_X_DB]);

	GotoXY(X1, Y1);
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
	/*
			&&      = '&'
			&&&     = '&'
			           ^H
			&&&&    = '&&'
			&&&&&   = '&&'
			           ^H
			&&&&&&  = '&&&'
	*/

	size_t Length = 0;
	bool Hi = false;

	for (size_t i = 0, size = Str.size(); i != size; ++i)
	{
		if (Str[i] == L'&')
		{
			auto AmpEnd = Str.find_first_not_of(L'&', i);
			if (AmpEnd == string::npos)
				AmpEnd = Str.size();
			const auto Count = AmpEnd - i;
			i = AmpEnd - 1;

			if (Count & 1) //нечёт?
			{
				if (Hi)
					++Length;
				else
					Hi = true;
			}

			Length+=Count/2;
		}
		else
		{
			++Length;
		}
	}

	return Length;

}

int HiFindRealPos(const string& Str, int Pos, bool ShowAmp)
{
	/*
			&&      = '&'
			&&&     = '&'
			           ^H
			&&&&    = '&&'
			&&&&&   = '&&'
			           ^H
			&&&&&&  = '&&&'
	*/

	if (ShowAmp)
	{
		return Pos;
	}

	int RealPos = 0;
	int VisPos = 0;

	for (auto i = Str.cbegin(); i != Str.cend() && VisPos != Pos; ++i)
	{
		if (*i == L'&')
		{
			++i;
			++RealPos;

			if (i == Str.cend())
				break;

			if (*i == L'&')
			{
				const auto Next1 = std::next(i);
				if (Next1 != Str.cend() && *Next1 == L'&')
				{
					const auto Next2 = std::next(Next1);
					if (Next2 == Str.cend() || *Next2 != L'&')
					{
						++i;
						++RealPos;
					}
				}

				if (i == Str.cend())
					break;
			}
		}

		++VisPos;
		++RealPos;
	}

	return RealPos;
}

int HiFindNextVisualPos(const string& Str, int Pos, int Direct)
{
	/*
			&&      = '&'
			&&&     = '&'
                       ^H
			&&&&    = '&&'
			&&&&&   = '&&'
                       ^H
			&&&&&&  = '&&&'
	*/

	if (Direct < 0)
	{
		if (!Pos || Pos == 1)
			return 0;

		if (Str[Pos-1] != L'&')
		{
			if (Str[Pos-2] == L'&')
			{
				if (Pos-3 >= 0 && Str[Pos-3] == L'&')
					return Pos-1;

				return Pos-2;
			}

			return Pos-1;
		}
		else
		{
			if (Pos-3 >= 0 && Str[Pos-3] == L'&')
				return Pos-3;

			return Pos-2;
		}
	}
	else
	{
		if (!Str[Pos])
			return Pos+1;

		if (Str[Pos] == L'&')
		{
			if (Str[Pos+1] == L'&' && Str[Pos+2] == L'&')
				return Pos+3;

			return Pos+2;
		}
		else
		{
			return Pos+1;
		}
	}
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

	COORD Size;
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
				Size.X = csbi.dwSize.X;
			}
			if (!Global->Opt->WindowModeStickyY)
			{
				Size.Y = csbi.dwSize.Y;
			}
		}
	}

	console.SetScreenBufferSize(Size);
}

static COORD& NonMaximisedBufferSize()
{
	static COORD s_Size;
	return s_Size;
}

void SaveNonMaximisedBufferSize(const COORD& Size)
{
	// We can't trust the size that windows sets automatically after restoring the window -
	// it could be less than previous because of horizontal scrollbar

	// TODO: this might also fix the issue with exiting from the fullscreen mode on Windows 10,
	// need to check and remove corresponding workarounds if it works
	NonMaximisedBufferSize() = Size;
}

COORD GetNonMaximisedBufferSize()
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

TEST_CASE("interf.histrlen")
{
	static const struct
	{
		string_view Input;
		size_t Size;
	}
	Tests[]
	{
		{ L""sv,         0 },
		{ L"1"sv,        1 },
		{ L"&"sv,        0 },
		{ L"1&2"sv,      2 },
		{ L"&1"sv,       1 },
		{ L"1&"sv,       1 },
		{ L"&1&"sv,      2 },
		{ L"&1&2"sv,     3 },
		{ L"&&"sv,       1 },
		{ L"1&&"sv,      2 },
		{ L"&&1"sv,      2 },
		{ L"1&&2"sv,     3 },
		{ L"&&&"sv,      1 },
		{ L"&&&1"sv,     2 },
		{ L"1&&&"sv,     2 },
		{ L"1&&&2"sv,    3 },
		{ L"&1&&&2"sv,   4 },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(HiStrlen(i.Input) == i.Size);
	}
}
#endif
