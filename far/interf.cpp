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
#include "log.hpp"
#include "char_width.hpp"
#include "exception_handler.hpp"

// Platform:
#include "platform.concurrency.hpp"
#include "platform.debug.hpp"
#include "platform.security.hpp"

// Common:
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static HICON load_icon(int IconId, bool Big)
{
	return static_cast<HICON>(LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IconId), IMAGE_ICON, GetSystemMetrics(Big? SM_CXICON : SM_CXSMICON), GetSystemMetrics(Big? SM_CYICON : SM_CYSMICON), LR_SHARED));
}

static HICON set_icon(HWND Wnd, bool Big, HICON Icon)
{
	return std::bit_cast<HICON>(SendMessage(Wnd, WM_SETICON, Big? ICON_BIG : ICON_SMALL, std::bit_cast<LPARAM>(Icon)));
}

void consoleicons::update_icon()
{
	if (!Global->Opt->SetIcon)
		return restore_icon();

	if (Global->Opt->IconIndex < 0 || static_cast<size_t>(Global->Opt->IconIndex) >= size())
		return;

	const int IconId = (Global->Opt->SetAdminIcon && os::security::is_admin())? FAR_ICON_RED : FAR_ICON + Global->Opt->IconIndex;
	set_icon(IconId);
}

void consoleicons::set_icon(int const IconId)
{
	const auto hWnd = console.GetWindow();
	if (!hWnd)
		return;

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

static os::event& CancelIoInProgress()
{
	static os::event s_CancelIoInProgress(os::event::type::manual, os::event::state::nonsignaled);
	return s_CancelIoInProgress;
}

static void CancelSynchronousIoWrapper(void* Thread)
{
	os::debug::set_thread_name(L"CancelSynchronousIo caller");

	if (!imports.CancelSynchronousIo)
		return;

	// TODO: SEH guard, try/catch, exception_ptr
	imports.CancelSynchronousIo(Thread);
	CancelIoInProgress().reset();
}

static BOOL control_handler(DWORD CtrlType)
{
	switch(CtrlType)
	{
	case CTRL_C_EVENT:
		return TRUE;

	case CTRL_BREAK_EVENT:
		if(!CancelIoInProgress().is_signaled())
		{
			CancelIoInProgress().set();
			os::thread Thread(&CancelSynchronousIoWrapper, Global->MainThreadHandle());
			Thread.detach();
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
		if (!Global)
			return FALSE;

		Global->CloseFAR = true;
		Global->AllowCancelExit = false;
		main_loop_process_messages();

		LOGNOTICE(L"CTRL_CLOSE_EVENT: exiting the thread"sv);

		// trick to let wmain() finish correctly
		ExitThread(1);
		//return TRUE;
	}
	return FALSE;
}

static BOOL WINAPI CtrlHandler(DWORD CtrlType)
{
	return cpp_try(
	[&]
	{
		return control_handler(CtrlType);
	},
	[&](source_location const&)
	{
		return FALSE;
	});
}

static bool ConsoleGlobalKeysHook(const Manager::Key& key)
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

	switch (key())
	{
	case KEY_CTRLSHIFTL:
	case KEY_RCTRLSHIFTL:
		logging::show();
		return true;
	}

	return false;
}

void InitConsole()
{
	if (static bool FirstInit = true; FirstInit)
	{
		Global->WindowManager->AddGlobalKeyHandler(ConsoleGlobalKeysHook);
		FirstInit = false;
	}

	if (DWORD Mode; !console.GetMode(console.GetInputHandle(), Mode))
	{
		static os::handle ConIn;
		// Separately to allow reinitialization
		ConIn = os::OpenConsoleInputBuffer();

		SetStdHandle(STD_INPUT_HANDLE, ConIn.native_handle());
	}

	if (DWORD Mode; !console.GetMode(console.GetOutputHandle(), Mode))
	{
		static os::handle ConOut;
		// Separately to allow reinitialization
		ConOut = os::OpenConsoleActiveScreenBuffer();

		SetStdHandle(STD_OUTPUT_HANDLE, ConOut.native_handle());
		SetStdHandle(STD_ERROR_HANDLE, ConOut.native_handle());
	}

	console.SetControlHandler(CtrlHandler, true);

	console_mode Mode;
	console.GetMode(console.GetInputHandle(), Mode.Input);
	console.GetMode(console.GetOutputHandle(), Mode.Output);
	console.GetMode(console.GetErrorHandle(), Mode.Error);
	InitialConsoleMode = Mode;

	Global->strInitTitle = console.GetPhysicalTitle();
	console.GetCursorInfo(InitialCursorInfo);

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

	SetFarConsoleMode();
	SetPalette();

	UpdateScreenSize();

	consoleicons::instance().update_icon();
}

void CloseConsole()
{
	Global->ScrBuf->Flush();
	MoveRealCursor(0, ScrY);
	console.SetCursorInfo(InitialCursorInfo);

	SetRealColor(colors::default_color());

	if (InitialConsoleMode)
	{
		ChangeConsoleMode(console.GetInputHandle(), InitialConsoleMode->Input);
		ChangeConsoleMode(console.GetOutputHandle(), InitialConsoleMode->Output);
		ChangeConsoleMode(console.GetErrorHandle(), InitialConsoleMode->Error);
	}

	console.SetTitle(Global->strInitTitle);

	ClearKeyQueue();
	consoleicons::instance().restore_icon();
	CancelIoInProgress().close();
}


void SetFarConsoleMode(bool SetsActiveBuffer)
{
	// Inherit existing mode. We don't want to build these flags from scratch,
	// as MS might introduce some new flags in future Windows versions.
	std::optional<DWORD> CurrentInputMode = 0;
	if (!console.GetMode(console.GetInputHandle(), *CurrentInputMode))
		CurrentInputMode.reset();

	auto InputMode = CurrentInputMode? *CurrentInputMode : InitialConsoleMode->Input;

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

	// Feature: if window rect is in unusual position (shifted up or right), of if an alternative buffer is active - enable mouse selection
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (const auto Buffer = console.GetActiveScreenBuffer(); (Buffer && Buffer != console.GetOutputHandle()) ||
			(
				Global->Opt->WindowMode &&
				get_console_screen_buffer_info(console.GetOutputHandle(), &csbi) &&
				(csbi.srWindow.Bottom != csbi.dwSize.Y - 1 || csbi.srWindow.Left)
			)
		)
		{
			InputMode |= ENABLE_EXTENDED_FLAGS | ENABLE_QUICK_EDIT_MODE;
		}
	}

	if (InputMode != CurrentInputMode)
		console.SetMode(console.GetInputHandle(), InputMode);

	if (SetsActiveBuffer)
		console.SetActiveScreenBuffer(console.GetOutputHandle());

	const auto OutputMode =
		InitialConsoleMode->Output |
		ENABLE_PROCESSED_OUTPUT |
		ENABLE_WRAP_AT_EOL_OUTPUT |
		// ENABLE_VIRTUAL_TERMINAL_PROCESSING is required for [x] Use Virtual Terminal for rendering (extended colors and styles),
		// but we also use it to send various service commands regardless of that option.
		// It should be set by default, but apparently it is not always the case (see M#4080),
		// and when it's not, flipping it back and forth too frequently impacts the overall performance.
		// Setting it unconditionally should prevent that and make the flow more predictable in general.
		(::console.IsVtSupported()? ENABLE_LVB_GRID_WORLDWIDE | ENABLE_VIRTUAL_TERMINAL_PROCESSING : 0);

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
		SendMessage(console.GetWindow(), WM_SYSCOMMAND, SC_MAXIMIZE, 0);

		coordScreen = console.GetLargestWindowSize(console.GetOutputHandle());

		if (!coordScreen.x || !coordScreen.y)
			return;

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

	point const coordScreen{ xSize, ySize };

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

bool IsConsoleViewportSizeChanged()
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
	if (!Global->Opt->Clock || Global->ScreenSaverActive || (Global->SuppressClock && Global->WindowManager->GetCurrentWindowType() == windowtype_desktop))
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
	if (Size == static_cast<size_t>(-1))
	{
		Size = static_cast<size_t>(Global->Opt->CursorSize[IsConsoleFullscreen()? 1 : 0]);
	}

	if (!Size)
		Size = InitialCursorInfo.dwSize;

	Global->ScrBuf->SetCursorType(Visible, Size);
}

void HideCursor()
{
	SetCursorType(false, 0);
}

void ShowCursor()
{
	SetCursorType(true, -1);
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

using real_cells = std::vector<FAR_CHAR_INFO>;
using cells = std::variant<size_t, real_cells>;

static void string_to_cells_simple(string_view const Str, size_t& CharsConsumed, cells& Cells, size_t const CellsAvailable)
{
	const auto From = Str.substr(0, CellsAvailable);
	CharsConsumed = From.size();

	std::visit(overload
	{
		[&](size_t& Size)
		{
			Size = From.size();
		},
		[&](real_cells& Buffer)
		{
			Buffer.clear();
			Buffer.reserve(From.size());
			std::ranges::transform(From, std::back_inserter(Buffer), [](wchar_t c){ return FAR_CHAR_INFO{ c, {}, {}, CurColor }; });
		}
	}, Cells);
}

static void string_to_cells_full_width_aware(string_view Str, size_t& CharsConsumed, cells& Cells, size_t const CellsAvailable)
{
	std::visit(overload
	{
		[&](size_t&){},
		[&](real_cells& Buffer){ Buffer.reserve(Str.size()); }
	}, Cells);

	const auto get_cells_count = [&]
	{
		return std::visit(overload
		{
			[&](size_t const& Size){ return Size; },
			[&](real_cells const& Buffer){ return Buffer.size(); }
		}, Cells);
	};

	const auto push_back = [&](wchar_t const Char)
	{
		std::visit(overload
		{
			[&](size_t& Size) { ++Size; },
			[&](std::vector<FAR_CHAR_INFO>& Buffer) { Buffer.push_back({ Char, {}, {}, CurColor }); }
		}, Cells);
	};

	const auto pop_back = [&]
	{
		std::visit(overload
		{
			[&](size_t& Size) { --Size; },
			[&](real_cells& Buffer) { Buffer.pop_back(); }
		}, Cells);
	};

	CharsConsumed = 0;

	while(!Str.empty() && get_cells_count() != CellsAvailable)
	{
		size_t CharsConsumedNow{};
		SCOPE_EXIT{ CharsConsumed += CharsConsumedNow; };

		wchar_t Char[]{ Str[0], 0 };

		const auto Codepoint = encoding::utf16::extract_codepoint(Str);

		if (Codepoint > std::numeric_limits<char16_t>::max())
		{
			Char[1] = Str[1];
			Str.remove_prefix(2);
			CharsConsumedNow = 2;
		}
		else
		{
			Str.remove_prefix(1);
			CharsConsumedNow = 1;
		}

		push_back(Char[0]);

		if (char_width::is_wide(Codepoint))
		{
			if (get_cells_count() == CellsAvailable)
			{
				// No space left for the trailing char
				CharsConsumedNow = 0;
				pop_back();
				break;
			}

			if (Char[1])
			{
				// It's wide and it already occupies two cells - awesome
				push_back(Char[1]);
			}
			else
			{
				// It's wide and we need to add a bogus cell
				std::visit(overload
				{
					[&](size_t& Size){ ++Size; },
					[&](std::vector<FAR_CHAR_INFO>& Buffer)
					{
						Buffer.back().Attributes.Flags |= COMMON_LVB_LEADING_BYTE;
						Buffer.push_back({ Char[0], {}, {}, CurColor });
						Buffer.back().Attributes.Flags |= COMMON_LVB_TRAILING_BYTE;
					}
				}, Cells);
			}
		}
		else
		{
			if (Char[1])
			{
				// It's a surrogate pair that occupies one cell only. Here be dragons.
				if (console.IsVtActive())
				{
					std::visit(overload
					{
						[&](size_t&){},
						[&](std::vector<FAR_CHAR_INFO>& Buffer)
						{
							// Put *one* fake character:
							Buffer.back().Char = encoding::replace_char;
							// Stash the actual codepoint. The drawing code will restore it from here:
							Buffer.back().Reserved1 = Codepoint;
						}
					}, Cells);
				}
				else
				{
					// Classic grid mode, nothing we can do :(
					// Expect the broken UI

					if (get_cells_count() == CellsAvailable)
					{
						// No space left for the trailing char
						CharsConsumedNow = 0;
						pop_back();
						break;
					}

					push_back(Char[1]);
				}
			}
			else
			{
				// It's not wide and not surrogate - the most common case, nothing to do
			}
		}
	}
}

void chars_to_cells(string_view Str, size_t& CharsConsumed, size_t const CellsAvailable, size_t& CellsConsumed)
{
	cells Cells;
	const auto& CellsToBeConsumed = Cells.emplace<0>();
	(char_width::is_enabled()? string_to_cells_full_width_aware : string_to_cells_simple)(Str, CharsConsumed, Cells, CellsAvailable);
	CellsConsumed = CellsToBeConsumed;

#ifdef _DEBUG
	if (CharsConsumed == Str.size())
		assert(CellsConsumed == visual_string_length(Str));
#endif
}

size_t Text(string_view Str, size_t const CellsAvailable)
{
	if (Str.empty())
		return 0;

	cells Cells;
	const auto& Buffer = Cells.emplace<1>();

	size_t CharsConsumed = 0;

	(char_width::is_enabled()? string_to_cells_full_width_aware : string_to_cells_simple)(Str, CharsConsumed, Cells, CellsAvailable);

	Global->ScrBuf->Write(CurX, CurY, Buffer);
	CurX += static_cast<int>(Buffer.size());

	return Buffer.size();
}

size_t Text(string_view Str)
{
	return Text(Str, Str.size());
}

size_t Text(wchar_t const Char, size_t const CellsAvailable)
{
	return Text({ &Char, 1 }, CellsAvailable);
}

size_t Text(wchar_t const Char)
{
	return Text(Char, 1);
}

size_t Text(lng const MsgId, size_t const CellsAvailable)
{
	return Text(msg(MsgId), CellsAvailable);
}

size_t Text(lng const MsgId)
{
	const auto& Str = msg(MsgId);
	return Text(Str, Str.size());
}

size_t VText(string_view const Str, size_t const CellsAvailable)
{
	if (Str.empty())
		return 0;

	size_t OccupiedWidth = 0;

	const auto StartCurX = CurX;

	for (const auto i: Str)
	{
		GotoXY(CurX, CurY);
		OccupiedWidth = std::max(OccupiedWidth, Text(i, CellsAvailable));
		++CurY;
		CurX = StartCurX;
	}

	return OccupiedWidth;
}

size_t VText(string_view const Str)
{
	return VText(Str, 1);
}

enum class hi_string_state
{
	ready,
	needs_unescape,
	highlight,
};

static void HiTextBase(string_view const Str, function_ref<void(string_view, hi_string_state)> const TextHandler)
{
	bool Unescape = false;
	for (size_t Offset = 0;;)
	{
		const auto AmpBegin = Str.find(L'&', Offset);
		if (AmpBegin == string::npos)
		{
			TextHandler(Str, Offset? hi_string_state::needs_unescape : hi_string_state::ready);
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
			TextHandler(Str.substr(0, AmpBegin), Unescape? hi_string_state::needs_unescape : hi_string_state::ready);

		auto CurPos = AmpBegin + 1;

		if (CurPos == Str.size())
			return;

		// We can only use single characters as hotkeys now
		if (const auto IsSurogate = CurPos + 1 != Str.size() && is_valid_surrogate_pair(Str[CurPos], Str[CurPos + 1]); !IsSurogate)
		{
			TextHandler(Str.substr(CurPos, 1), hi_string_state::highlight);
			++CurPos;
		}

		if (CurPos == Str.size())
			return;

		const auto HiAmpCollapse = Str[AmpBegin + 1] == L'&' && Str[AmpBegin + 2] == L'&';
		const auto Tail = Str.substr(CurPos + (HiAmpCollapse? 1 : 0));
		TextHandler(Tail, Tail.find(L'&') == Tail.npos? hi_string_state::ready : hi_string_state::needs_unescape);

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

static size_t HiTextImpl(string_view const Str, const FarColor& HiColor, bool const Vertical, size_t CellsAvailable)
{
	using text_func = size_t(*)(string_view, size_t);
	const text_func fText = Text, fVText = VText; //BUGBUG
	const auto TextFunc  = Vertical? fVText : fText;

	string Buffer;
	size_t CellsConsumed = 0;

	HiTextBase(Str, [&](string_view const Part, hi_string_state const State)
	{
		switch (State)
		{
		case hi_string_state::ready:
			CellsConsumed += TextFunc(Part, CellsAvailable - CellsConsumed);
			break;

		case hi_string_state::needs_unescape:
			unescape(Part, [&](wchar_t const Ch){ Buffer.push_back(Ch); return true; });
			CellsConsumed += TextFunc(Buffer, CellsAvailable - CellsConsumed); Buffer.clear();
			break;

		case hi_string_state::highlight:
			{
				const auto SaveColor = CurColor;
				SetColor(HiColor);
				CellsConsumed += TextFunc(Part, CellsAvailable - CellsConsumed);
				SetColor(SaveColor);
			}
			break;
		}
	});

	return CellsConsumed;
}

size_t HiText(string_view const Str, const FarColor& Color, size_t const CellsAvailable)
{
	return HiTextImpl(Str, Color, false, CellsAvailable);
}

size_t HiText(string_view const Str, const FarColor& Color)
{
	return HiText(Str, Color, Str.size());
}

size_t HiVText(string_view const Str, const FarColor& Color, size_t const CellsAvailable)
{
	return HiTextImpl(Str, Color, true, CellsAvailable);
}

size_t HiVText(string_view const Str, const FarColor& Color)
{
	return HiVText(Str, Color, Str.size());
}

string HiText2Str(string_view const Str, size_t* HotkeyVisualPos)
{
	string Result;

	if (HotkeyVisualPos)
		*HotkeyVisualPos = string::npos;

	HiTextBase(Str, [&](string_view const Part, hi_string_state const State)
	{
		switch (State)
		{
		case hi_string_state::ready:
			Result += Part;
			break;

		case hi_string_state::needs_unescape:
			unescape(Part, [&](wchar_t const Ch){ Result.push_back(Ch); return true; });
			break;

		case hi_string_state::highlight:
			if (HotkeyVisualPos)
				*HotkeyVisualPos = Result.size();
			Result += Part;
			break;
		}
	});

	return Result;
}

bool HiTextHotkey(string_view Str, wchar_t& Hotkey, size_t* HotkeyVisualPos)
{
	bool Result = false;

	size_t Size{};

	HiTextBase(Str, [&](string_view const Part, hi_string_state const State)
	{
		switch (State)
		{
		case hi_string_state::ready:
			Size += Part.size();
			break;

		case hi_string_state::needs_unescape:
			unescape(Part, [&](wchar_t const Ch){ ++Size; return true; });
			break;

		case hi_string_state::highlight:
			Hotkey = Part[0];
			if (HotkeyVisualPos)
				*HotkeyVisualPos = Size;
			Result = true;
			break;
		}
	});

	return Result;
}

// removes single '&', turns '&&' into '&'
void inplace::remove_highlight(string& Str)
{
	auto Iterator = Str.begin();
	unescape(Str, [&](wchar_t Ch){ *Iterator = Ch; ++Iterator; return true; });
	Str.resize(Iterator - Str.begin());
}

void inplace::escape_ampersands(string& Str)
{
	replace(Str, L"&"sv, L"&&"sv);
}

string remove_highlight(string_view const Str)
{
	return remove_highlight(string(Str));
}

string remove_highlight(string Str)
{
	inplace::remove_highlight(Str);
	return Str;
}

string escape_ampersands(string Str)
{
	inplace::escape_ampersands(Str);
	return Str;
}

string escape_ampersands(string_view const Str)
{
	return escape_ampersands(string(Str));
}

void SetScreen(rectangle const Where, wchar_t Ch, const FarColor& Color)
{
	Global->ScrBuf->FillRect(Where, { Ch, {}, {}, Color });
}

void MakeShadow(rectangle const Where)
{
	Global->ScrBuf->ApplyShadow(Where);
}

void DropShadow(rectangle const Where)
{
	MakeShadow({ Where.left + 2, Where.bottom + 1, Where.right + 2, Where.bottom + 1 });
	MakeShadow({ Where.right + 1, Where.top + 1, Where.right + 2, Where.bottom });
}

void SetColor(int Color)
{
	CurColor = colors::NtColorToFarColor(Color);
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

const FarColor& GetColor()
{
	return CurColor;
}

size_t NumberOfEmptyLines(size_t const Desired)
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

	rectangle const Region{ 0, static_cast<int>(ScrY - Desired + 1), ScrX, ScrY };

	// TODO: matrix_view to avoid copying
	matrix<FAR_CHAR_INFO> BufferBlock(Desired, ScrX + 1);
	Global->ScrBuf->Read(Region, BufferBlock);

	for (const auto Row: std::views::reverse(BufferBlock))
	{
		if (!std::ranges::all_of(Row, [](const FAR_CHAR_INFO& i){ return i.Char == L' '; }))
			return BufferBlock.height() - 1 - BufferBlock.row_number(Row);
	}

	return Desired;
}

size_t string_pos_to_visual_pos(string_view Str, size_t const StringPos, size_t const TabSize, position_parser_state* SavedState)
{
	if (!StringPos || Str.empty())
		return StringPos;

	const auto CharWidthEnabled = char_width::is_enabled();
	if (!CharWidthEnabled)
	{
		if (TabSize == 1 || !contains(Str, L'\t'))
			return StringPos;
	}

	position_parser_state State;

	if (SavedState && StringPos >= SavedState->StringIndex)
		State = *SavedState;

	const auto End = std::min(Str.size(), StringPos);
	while (State.StringIndex < End)
	{
		size_t
			CharStringIncrement,
			CharVisualIncrement;

		if (Str[State.StringIndex] == L'\t')
		{
			CharStringIncrement = 1;
			CharVisualIncrement = TabSize - State.VisualIndex % TabSize;
		}
		else if (CharWidthEnabled)
		{
			const auto Codepoint = encoding::utf16::extract_codepoint(Str.substr(State.StringIndex));
			CharStringIncrement = Codepoint > std::numeric_limits<char16_t>::max()? 2 : 1;
			CharVisualIncrement = char_width::get(Codepoint);
		}
		else
		{
			CharStringIncrement = 1;
			CharVisualIncrement = 1;
		}

		const auto NextStringIndex = State.StringIndex + static_cast<unsigned>(CharStringIncrement);

		if (NextStringIndex > End)
			break;

		State.StringIndex = NextStringIndex;
		State.VisualIndex += static_cast<unsigned>(CharVisualIncrement);
	}

	if (SavedState)
		*SavedState = State;

	return State.VisualIndex + (StringPos > Str.size()? StringPos - Str.size() : 0);
}

size_t visual_pos_to_string_pos(string_view Str, size_t const VisualPos, size_t const TabSize, position_parser_state* SavedState)
{
	if (!VisualPos || Str.empty())
		return VisualPos;

	const auto CharWidthEnabled = char_width::is_enabled();
	if (!CharWidthEnabled)
	{
		if (TabSize == 1 || !contains(Str, L'\t'))
			return VisualPos;
	}

	position_parser_state State;

	if (SavedState && VisualPos >= SavedState->VisualIndex)
		State = *SavedState;

	const auto End = Str.size();
	bool Overflow{};

	while (State.VisualIndex < VisualPos && State.StringIndex != End)
	{
		size_t
			CharVisualIncrement,
			CharStringIncrement;

		if (Str[State.StringIndex] == L'\t')
		{
			CharVisualIncrement = TabSize - State.VisualIndex % TabSize;
			CharStringIncrement = 1;
		}
		else if (CharWidthEnabled)
		{
			const auto Codepoint = encoding::utf16::extract_codepoint(Str.substr(State.StringIndex));
			CharVisualIncrement = char_width::get(Codepoint);
			CharStringIncrement = Codepoint > std::numeric_limits<char16_t>::max()? 2 : 1;
		}
		else
		{
			CharVisualIncrement = 1;
			CharStringIncrement = 1;
		}

		const auto NextVisualIndex = State.VisualIndex + static_cast<unsigned>(CharVisualIncrement);
		if (NextVisualIndex > VisualPos)
		{
			Overflow = true;
			break;
		}

		State.VisualIndex = NextVisualIndex;
		State.StringIndex += static_cast<unsigned>(CharStringIncrement);
	}

	if (SavedState)
		*SavedState = State;

	return State.StringIndex + (Overflow? 0 : VisualPos - State.VisualIndex);
}

size_t visual_string_length(string_view Str)
{
	// In theory, this function doesn't need to care about tabs:
	// they're not allowed in file names and everywhere else
	// we replace them with spaces for display purposes.
	return string_pos_to_visual_pos(Str, Str.size(), 1, {});
}

bool is_valid_surrogate_pair(string_view const Str)
{
	if (Str.size() < 2)
		return false;

	return encoding::utf16::is_valid_surrogate_pair(Str[0], Str[1]);
}

bool is_valid_surrogate_pair(wchar_t First, wchar_t Second)
{
	return encoding::utf16::is_valid_surrogate_pair(First, Second);
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
	if (!ScrollBarRequired(Length, ItemsCount))
		return false;

	return ScrollBarEx(X1, Y1, Length, TopItem, TopItem + Length, ItemsCount);
}

static string MakeScrollBarEx(
	size_t const Length,
	unsigned long long const Start,
	unsigned long long End,
	unsigned long long const Size,
	wchar_t const FirstButton,
	wchar_t const SecondButton,
	wchar_t const BackgroundChar,
	wchar_t const SliderChar
)
{
	assert(Start <= End);

	if (Length < 2)
		return {};

	string Buffer(Length, BackgroundChar);
	Buffer.front() = FirstButton;
	Buffer.back() = SecondButton;

	if (Buffer.size() == 2)
		return Buffer;

	const auto FieldBegin = Buffer.begin() + 1;
	const auto FieldSize = static_cast<unsigned>(Buffer.size() - 2);

	if (FieldSize == 1)
	{
		Buffer[1] = SliderChar;
		return Buffer;
	}

	End = std::min(End, Size);

	const auto rounded = [FieldSize](unsigned long long const Nom, unsigned long long const Den)
	{
		return static_cast<unsigned long long>(std::round(ToPercent(Nom, Den, FieldSize * 10) / 10.0));
	};

	auto SliderBegin = std::max(Start? 1ull : 0ull, rounded(Start, Size));
	if (!SliderBegin && Start)
		++SliderBegin;
	if (SliderBegin == FieldSize)
		--SliderBegin;

	const auto SliderSize = std::max(1ull, rounded(End - Start, Size));

	auto SliderEnd = End == Size? FieldSize : SliderBegin + SliderSize;
	if (SliderEnd == FieldSize && End < Size)
	{
		--SliderEnd;
		if (SliderBegin > 1)
			--SliderBegin;
	}

	if (SliderEnd > SliderBegin)
		std::fill(FieldBegin + SliderBegin, FieldBegin + SliderEnd, SliderChar);

	return Buffer;
}

bool ScrollBarEx(size_t X1, size_t Y1, size_t Length, unsigned long long Start, unsigned long long End, unsigned long long Size)
{
	const auto Scrollbar = MakeScrollBarEx(Length, Start, End, Size, L'▲', L'▼', BoxSymbols[BS_X_B0], BoxSymbols[BS_X_DB]);
	if (Scrollbar.empty())
		return false;

	GotoXY(static_cast<int>(X1), static_cast<int>(Y1));
	VText(Scrollbar);

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
		std::ranges::transform(Predefined[static_cast<size_t>(Type)], Buffer, [](size_t i){ return BoxSymbols[i]; });
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
		StrPercent = far::format(L" {:3}%"sv, Percent);
		Size = Size > StrPercent.size()? Size - StrPercent.size(): 0;
	}
	string Str(Size, BoxSymbols[BS_X_B0]);
	const auto Pos = std::min(Percent, 100uz) * Size / 100;
	std::fill_n(Str.begin(), Pos, BoxSymbols[BS_X_DB]);
	if (ShowPercent)
	{
		Str += StrPercent;
	}
	if (PropagateToTasbkar)
	{
		taskbar::set_value(Percent, 100);
	}
	return Str;
}

size_t HiStrlen(string_view const Str)
{
	size_t Result = 0;
	std::optional<wchar_t> First;

	unescape(Str, [&](wchar_t const Char)
	{
		if (encoding::utf16::is_high_surrogate(Char))
		{
			First = Char;
			return true;
		}

		const auto IsLow = encoding::utf16::is_low_surrogate(Char);
		if (!IsLow)
			First.reset();

		const auto Codepoint = First && IsLow? encoding::utf16::extract_codepoint(*First, Char) : Char;

		Result += char_width::get(Codepoint);
		return true;
	});

	if (First)
	{
		++Result;
	}

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
	Where.left = std::clamp(Where.left, 0, ScrX);
	Where.top = std::clamp(Where.top, 0, ScrY);
	Where.right = std::clamp(Where.right, 0, ScrX);
	Where.bottom = std::clamp(Where.bottom, 0, ScrY);
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
		if (get_console_screen_buffer_info(console.GetOutputHandle(), &csbi))
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

void SetPalette()
{
	if (Global->Opt->SetPalette)
		console.SetPalette(colors::default_palette());
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

static bool s_SuppressConsoleConfirmations;

void suppress_console_confirmations()
{
	s_SuppressConsoleConfirmations = true;
}

size_t ConsoleChoice(string_view const Message, string_view const Choices, size_t const Default, function_ref<void()> const MessagePrinter)
{
	if (InitialConsoleMode)
	{
		ChangeConsoleMode(console.GetInputHandle(), InitialConsoleMode->Input);
		ChangeConsoleMode(console.GetOutputHandle(), InitialConsoleMode->Output);
		ChangeConsoleMode(console.GetErrorHandle(), InitialConsoleMode->Error);
	}

	console.SetCursorInfo(InitialCursorInfo);

	MessagePrinter();

	for (;;)
	{
		std::wcout << far::format(L"\n{} ({})? "sv, Message, join(L"/"sv, Choices)) << std::flush;

		if (s_SuppressConsoleConfirmations)
			return Default;

		wchar_t Input;
		std::wcin.clear();
		std::wcin.get(Input).ignore(std::numeric_limits<std::streamsize>::max(), L'\n');

		if (const auto Index = Choices.find(upper(Input)); Index != Choices.npos)
			return Index;
	}
}

bool ConsoleYesNo(string_view const Message, bool const Default, function_ref<void()> const MessagePrinter)
{
	return ConsoleChoice(Message, L"YN"sv, Default? 0 : 1, MessagePrinter) == 0;
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
			size_t Visual;
			size_t Real;
		}
		Pos;
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
		REQUIRE(remove_highlight(i.Input) == i.Result);

		wchar_t Hotkey{};
		HotkeyPos = np;
		REQUIRE(HiTextHotkey(i.Input, Hotkey, &HotkeyPos) == (i.Hotkey != 0));
		REQUIRE(Hotkey == i.Hotkey);
		REQUIRE(HotkeyPos == i.HotkeyVisualPos);

		REQUIRE(HiStrlen(i.Input) == i.Result.size());

		REQUIRE(HiFindRealPos(i.Input, i.Pos.Visual) == i.Pos.Real);
	}
}

TEST_CASE("wide_chars")
{
	static const struct
	{
		string_view Str;
		std::initializer_list<std::pair<size_t, int>>
			StringToVisual,
			VisualToString;
	}
	Tests[]
	{
		{
			{}, // Baseline, half width
			{ { 0, +0 }, { 1, +0 }, { 2, +0 }, { 3, +0 }, { 4, +0 }, },
			{ { 0, +0 }, { 1, +0 }, { 2, +0 }, { 3, +0 }, { 4, +0 }, },
		},
		{
			L"1"sv, // ANSI, half width
			{ { 0, +0 }, { 1, +0 }, { 2, +0 }, { 3, +0 }, { 4, +0 }, },
			{ { 0, +0 }, { 1, +0 }, { 2, +0 }, { 3, +0 }, { 4, +0 }, },
		},
		{
			L"あ"sv, // Hiragana, full width
			{ { 0, +0 }, { 1, +1 }, { 2, +1 }, { 3, +1 }, { 4, +1 }, },
			{ { 0, +0 }, { 1, -1 }, { 2, -1 }, { 3, -1 }, { 4, -1 }, },
		},
		{
			L"ああ"sv, // Hiragana, full width
			{ { 0, +0 }, { 1, +1 }, { 2, +2 }, { 3, +2 }, { 4, +2 }, },
			{ { 0, +0 }, { 1, -1 }, { 2, -1 }, { 3, -2 }, { 4, -2 }, },
		},
		{
			L"𐀀"sv, // Surrogate, half width
			{ { 0, +0 }, { 1, -1 }, { 2, -1 }, { 3, -1 }, { 4, -1 }, },
			{ { 0, +0 }, { 1, +1 }, { 2, +1 }, { 3, +1 }, { 4, +1 }, },
		},
		{
			L"𐀀𐀀"sv, // Surrogate, half width
			{ { 0, +0 }, { 1, -1 }, { 2, -1 }, { 3, -2 }, { 4, -2 }, },
			{ { 0, +0 }, { 1, +1 }, { 2, +2 }, { 3, +2 }, { 4, +2 }, },
		},
		{
			L"𠲖"sv, // Surrogate, full width
			{ { 0, +0 }, { 1, -1 }, { 2, +0 }, { 3, +0 }, { 4, +0 }, },
			{ { 0, +0 }, { 1, -1 }, { 2, +0 }, { 3, +0 }, { 4, +0 }, },
		},
		{
			L"𠲖𠲖"sv, // Surrogate, full width
			{ { 0, +0 }, { 1, -1 }, { 2, +0 }, { 3, -1 }, { 4, +0 }, },
			{ { 0, +0 }, { 1, -1 }, { 2, +0 }, { 3, -1 }, { 4, +0 }, },
		},
		{
			L"残酷な天使のように少年よ神話になれ"sv,
			{ {17, +17}, },
			{ {34, -17} } },

	};

	char_width::enable(1);

	for (const auto& i: Tests)
	{
		position_parser_state State[2];

		for (const auto& [StringPos, VisualShift]: i.StringToVisual)
		{
			REQUIRE(string_pos_to_visual_pos(i.Str, StringPos, 1, &State[0]) == StringPos + VisualShift);
		}

		for (const auto& [VisualPos, StringShift] : i.VisualToString)
		{
			REQUIRE(visual_pos_to_string_pos(i.Str, VisualPos, 1, &State[1]) == VisualPos + StringShift);
		}
	}

	char_width::enable(0);
}

TEST_CASE("tabs")
{
	static string_view const Strs[]
	{
		{},
		L"1\t2"sv,
		L"\t1\t12\t123\t1234\t"sv,
		L"\t𐀀\t12\t𐀀天\t日本\t"sv,
	};

	static const struct
	{
		size_t Str, TabSize, VisualPos, RealPos;
		bool TestRealToVisual;
	}
	Tests[]
	{
		{ 0, 0,  0,  0, true,  },
		{ 0, 0,  1,  1, true,  },

		{ 1, 0,  0,  0, true,  },

		{ 1, 1,  0,  0, true,  },
		{ 1, 1,  1,  1, true,  },
		{ 1, 1,  2,  2, true,  },
		{ 1, 1,  3,  3, true,  },

		{ 1, 2,  0,  0, true,  },
		{ 1, 2,  1,  1, true,  },
		{ 1, 2,  2,  2, true,  },
		{ 1, 2,  3,  3, true,  },

		{ 1, 3,  0,  0, true,  },
		{ 1, 3,  1,  1, true,  },
		{ 1, 3,  2,  1, false, },
		{ 1, 3,  3,  2, true,  },

		{ 2, 4,  0,  0, true,  },
		{ 2, 4,  1,  0, false, },
		{ 2, 4,  2,  0, false, },
		{ 2, 4,  3,  0, false, },
		{ 2, 4,  4,  1, true,  },
		{ 2, 4,  5,  2, true,  },
		{ 2, 4,  6,  2, false, },
		{ 2, 4,  7,  2, false, },
		{ 2, 4,  8,  3, true,  },
		{ 2, 4,  9,  4, true,  },
		{ 2, 4, 10,  5, true,  },
		{ 2, 4, 11,  5, false, },
		{ 2, 4, 12,  6, true,  },
		{ 2, 4, 13,  7, true,  },
		{ 2, 4, 14,  8, true,  },
		{ 2, 4, 15,  9, true,  },
		{ 2, 4, 16, 10, true,  },
		{ 2, 4, 17, 11, true,  },
		{ 2, 4, 18, 12, true,  },
		{ 2, 4, 19, 13, true,  },
		{ 2, 4, 20, 14, true,  },
		{ 2, 4, 21, 14, false, },
		{ 2, 4, 22, 14, false, },
		{ 2, 4, 23, 14, false, },
		{ 2, 4, 24, 15, false, },
		{ 2, 4, 25, 16, true,  },
	};

	for (const auto& i : Tests)
	{
		REQUIRE(i.RealPos == visual_pos_to_string_pos(Strs[i.Str], i.VisualPos, i.TabSize));

		if (i.TestRealToVisual)
			REQUIRE(i.VisualPos == string_pos_to_visual_pos(Strs[i.Str], i.RealPos, i.TabSize));
	}
}

TEST_CASE("tabs.cache")
{
	const auto Str = L"\t0"sv;

	{
		position_parser_state State;
		REQUIRE(visual_pos_to_string_pos(Str, 1, 8, &State) == 0uz);
		REQUIRE(string_pos_to_visual_pos(Str, 1, 8, &State) == 8uz);
	}

	{
		position_parser_state State;
		REQUIRE(visual_pos_to_string_pos(Str, 1, 8, &State) == 0uz);
		REQUIRE(string_pos_to_visual_pos(Str, 1, 8, &State) == 8uz);
	}

}

TEST_CASE("Scrollbar")
{
	static const struct
	{
		size_t const Length;
		unsigned long long const Start, Size;
		string_view Expected;
	}
	Tests[]
	{
		{},
		{  1,  0,   1 },

		{  2,  0,   1, L"<>"sv },
		{  2,  0,   2, L"<>"sv },
		{  2,  0,   3, L"<>"sv },
		{  2,  1,   3, L"<>"sv },

		{  3,  0,   4, L"<->"sv },

		{  4,  0,   5, L"<- >"sv },
		{  4,  1,   5, L"< ->"sv },

		{  5,  0,   6, L"<-- >"sv },
		{  5,  1,   6, L"< -->"sv },
		{  5,  0,   7, L"<-- >"sv },
		{  5,  1,   7, L"< - >"sv },
		{  5,  0,   8, L"<-- >"sv },
		{  5,  1,   8, L"< - >"sv },
		{  5,  2,   8, L"< - >"sv },
		{  5,  3,   8, L"< -->"sv },

		{ 10,  0,   1, L"<-------->"sv },
		{ 10,  0,  10, L"<-------->"sv },
		{ 10,  0,  11, L"<------- >"sv },
		{ 10,  1,  11, L"< ------->"sv },
		{ 10,  0,  12, L"<------- >"sv },
		{ 10,  1,  12, L"< ------ >"sv },
		{ 10,  2,  12, L"< ------->"sv },

		{ 8,  0,   50, L"<-     >"sv },
		{ 8,  1,   50, L"< -    >"sv },
		{ 8, 10,   50, L"< -    >"sv },
		{ 8, 11,   50, L"< -    >"sv },
		{ 8, 12,   50, L"< -    >"sv },
		{ 8, 13,   50, L"<  -   >"sv },
		{ 8, 41,   50, L"<    - >"sv },
		{ 8, 42,   50, L"<     ->"sv },

		{ 8, 0, 10000, L"<-     >"sv },
	};

	for (const auto& i: Tests)
	{
		const auto Scrollbar = MakeScrollBarEx(i.Length, i.Start, i.Start + i.Length, i.Size, L'<', L'>', L' ', L'-');
		REQUIRE(i.Expected == Scrollbar);
	}
}
#endif
