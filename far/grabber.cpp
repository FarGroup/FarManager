/*
grabber.cpp

Screen grabber
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
#include "grabber.hpp"

// Internal:
#include "keyboard.hpp"
#include "keys.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "interf.hpp"
#include "clipboard.hpp"
#include "config.hpp"
#include "help.hpp"
#include "string_utils.hpp"
#include "global.hpp"
#include "colormix.hpp"
#include "eol.hpp"

// Platform:

// Common:
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

Grabber::Grabber(private_tag):
	ResetArea(true),
	m_VerticalBlock(false)
{
	ScreenObject::SetPosition({ 0, 0, ScrX, ScrY });
}

grabber_ptr Grabber::create()
{
	auto GrabberPtr = std::make_shared<Grabber>(private_tag());
	GrabberPtr->init();
	return GrabberPtr;
}

void Grabber::init()
{
	SetMacroMode(MACROAREA_GRABBER);
	SaveScr = std::make_unique<SaveScreen>();
	bool Visible=false;
	size_t Size = 0;
	GetCursorType(Visible,Size);

	if (Visible)
		GArea.Current = GetCursorPos();
	else
		GArea.Current = {};

	GArea.Begin.x = -1;
	Process();
	SaveScr.reset();
	Global->WindowManager->RefreshWindow();
}

std::tuple<point&, point&> Grabber::GetSelection()
{
	auto& SelectionBegin = GArea.Begin.y == GArea.End.y?
	    GArea.Begin.x < GArea.End.x? GArea.Begin : GArea.End :
	    GArea.Begin.y < GArea.End.y? GArea.Begin : GArea.End;
	auto& SelectionEnd = &SelectionBegin == &GArea.Begin? GArea.End : GArea.Begin;
	return std::tie(SelectionBegin, SelectionEnd);
}

void Grabber::CopyGrabbedArea(bool Append, bool VerticalBlock)
{
	if (GArea.Begin.x < 0)
		return;

	const auto X1 = std::min(GArea.Begin.x, GArea.End.x);
	const auto X2 = std::max(GArea.Begin.x, GArea.End.x);
	const auto Y1 = std::min(GArea.Begin.y, GArea.End.y);
	const auto Y2 = std::max(GArea.Begin.y, GArea.End.y);

	auto FromX = X1;
	auto ToX = X2;
	const auto FromY = Y1;
	const auto ToY = Y2;

	if (m_StreamSelection)
	{
		FromX = 0;
		ToX = ScrX;
	}

	matrix<FAR_CHAR_INFO> CharBuf(ToY - FromY + 1, ToX - FromX + 1);
	GetText({ FromX, FromY, ToX, ToY }, CharBuf);

	string CopyBuf;
	CopyBuf.reserve(CharBuf.height() * (CharBuf.width() + 2));

	string Line;
	Line.reserve(CharBuf.width());

	const auto& [SelectionBegin, SelectionEnd] = GetSelection();
	const auto Eol = eol::system.str();

	for (size_t i = 0; i != CharBuf.height(); ++i)
	{
		const auto& MatrixLine = CharBuf[i];
		auto Begin = MatrixLine.cbegin(), End = MatrixLine.cend();

		const auto IsFirstLine = i == 0;
		const auto IsLastLine = i == CharBuf.height() - 1;

		if (m_StreamSelection)
		{
			Begin += IsFirstLine? SelectionBegin.x : 0;
			End -= IsLastLine? ScrX - SelectionEnd.x : 0;
		}
		Line.clear();
		std::transform(Begin, End, std::back_inserter(Line), [](const FAR_CHAR_INFO& Char) { return Char.Char; });
		bool AddEol = !IsLastLine;
		if (m_StreamSelection)
		{
			// in stream mode we want to preserve existing line breaks,
			// but at the same time join lines that were split because of the text wrapping.
			// The Windows console doesn't keep EOL characters at all, so we will try to guess.
			// If the line ends with an alphanumeric character, it's probably has been wrapped.
			// TODO: consider analysing the beginning of the next line too.
			AddEol = !is_alphanumeric(Line.back());
		}
		else
		{
			inplace::trim_right(Line);
		}

		CopyBuf += Line;

		if (AddEol)
		{
			CopyBuf += Eol;
		}
	}

	clipboard_accessor Clip;

	if (Clip->Open())
	{
		if (Append)
		{
			string OldData;
			if (Clip->GetText(OldData))
			{
				if (!OldData.empty() && OldData.back() != L'\n')
				{
					OldData += Eol;
				}
				CopyBuf.insert(0, OldData);
			}
		}

		if (VerticalBlock)
			Clip->SetVText(CopyBuf);
		else
			Clip->SetText(CopyBuf);
	}
}


void Grabber::DisplayObject()
{
	MoveCursor({ GArea.Current.x, GArea.Current.y });

	const auto X1 = std::min(GArea.Begin.x, GArea.End.x);
	const auto X2 = std::max(GArea.Begin.x, GArea.End.x);
	const auto Y1 = std::min(GArea.Begin.y, GArea.End.y);
	const auto Y2 = std::max(GArea.Begin.y, GArea.End.y);

	m_StreamSelection.forget();

	if (GArea.Begin.x != -1)
	{
		auto FromX = X1;
		auto ToX = X2;
		const auto FromY = Y1;
		const auto ToY = Y2;

		if (m_StreamSelection)
		{
			FromX = 0;
			ToX = ScrX;
		}

		matrix<FAR_CHAR_INFO> CharBuf(ToY - FromY + 1, ToX - FromX + 1);
		GetText({ FromX, FromY, ToX, ToY }, CharBuf);

		for (int Y = FromY; Y <= ToY; Y++)
		{
			for (int X = FromX; X <= ToX; X++)
			{
				const auto& CurColor = SaveScr->ScreenBuf[Y][X].Attributes;
				auto& Destination = CharBuf[Y - Y1][X - FromX].Attributes;
				Destination = CurColor;

				if (m_StreamSelection)
				{
					const auto ToUp = GArea.Begin.y < GArea.End.y;
					const auto ToDown = !ToUp;
					const auto FirstLine = Y == FromY;
					const auto LastLine = Y == ToY;

					if (ToDown)
					{
						if (FirstLine && LastLine)
						{
							if (X < X1 || X > X2)
							{
								continue;
							}
						}
						else if ((FirstLine && X < GArea.End.x) || (LastLine && X > GArea.Begin.x))
							continue;
					}
					else
					{
						if ((FirstLine && X < GArea.Begin.x) || (LastLine && X > GArea.End.x))
							continue;
					}
				}

				Destination.BackgroundColor = colors::alpha_value(CurColor.BackgroundColor) | (
					CurColor.IsBg4Bit()?
						colors::index_value(~colors::index_value(CurColor.BackgroundColor)) :
						colors::color_value(~colors::color_value(CurColor.BackgroundColor))
					);

				Destination.ForegroundColor = colors::alpha_value(CurColor.ForegroundColor) | (
					CurColor.IsFg4Bit()?
						colors::index_value(~colors::index_value(CurColor.ForegroundColor)) :
						colors::color_value(~colors::color_value(CurColor.ForegroundColor))
					);
			}
		}

		PutText({ FromX, FromY, ToX, ToY }, CharBuf.data());
	}

	SetCursorType(true, 60);
}


bool Grabber::ProcessKey(const Manager::Key& Key)
{
	auto LocalKey = Key();
	if(Global->CloseFAR)
	{
		LocalKey = KEY_ESC;
	}

	/* $ 14.03.2001 SVS
	  [-] Неправильно воспроизводился макрос в режиме грабления экрана.
	      При воспроизведении клавиша Home перемещала курсор в координаты
	      0,0 консоли.
	  Не было учтено режима выполнения макроса.
	*/
	if (Global->CtrlObject->Macro.IsExecuting())
	{
		if ((LocalKey&KEY_SHIFT) && LocalKey!=KEY_NONE && ResetArea)
			Reset();
		else if (none_of(LocalKey, KEY_IDLE, KEY_NONE) && !(LocalKey&KEY_SHIFT) && !IntKeyState.ShiftPressed() && !IntKeyState.AltPressed())
			ResetArea = true;
	}
	else
	{
		if ((IntKeyState.ShiftPressed() || LocalKey!=KEY_SHIFT) && (LocalKey&KEY_SHIFT) && none_of(LocalKey, KEY_NONE, KEY_CTRLA, KEY_RCTRLA) && !IntKeyState.AltPressed() && ResetArea)
			Reset();
		else if (none_of(LocalKey, KEY_IDLE, KEY_NONE, KEY_SHIFT, KEY_CTRLA, KEY_RCTRLA, KEY_F1, KEY_SPACE) && !IntKeyState.ShiftPressed() && !IntKeyState.AltPressed() && !(LocalKey&KEY_SHIFT))
			ResetArea = true;
	}

	const auto Move = [](point& What, int Count, int Direction, int LimitX, int LimitY, int NewX)
	{
		for (; Count; --Count)
		{
			if (What.x != LimitX)
			{
				What.x += Direction;
			}
			else if (m_StreamSelection)
			{
				if (What.y != LimitY)
				{
					What.y += Direction;
					What.x = NewX;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		return true;
	};

	const auto MovePointLeft = [&](point& What, int Count)
	{
		return Move(What, Count, -1, 0, 0, ScrX);
	};

	const auto MovePointRight = [&](point& What, int Count)
	{
		return Move(What, Count, 1, ScrX, ScrY, 0);
	};

	const auto MoveLeft = [&](int Count)
	{
		return MovePointLeft(GArea.Current, Count);
	};

	const auto MoveRight = [&](int Count)
	{
		return MovePointRight(GArea.Current, Count);
	};

	switch (LocalKey)
	{
		case KEY_F1:
			help::show(L"Grabber"sv);
			break;

		case KEY_CTRLU:
		case KEY_RCTRLU:
			Reset();
			GArea.Begin.x = -1;
			break;

		case KEY_ESC:
		case KEY_F10:
			Close(0);
			break;

		case KEY_SPACE:
			m_StreamSelection = !m_StreamSelection;
			break;

		case KEY_NUMENTER:
		case KEY_ENTER:
		case KEY_CTRLINS:   case KEY_CTRLNUMPAD0:
		case KEY_RCTRLINS:  case KEY_RCTRLNUMPAD0:
		case KEY_CTRLADD:
		case KEY_RCTRLADD:
			CopyGrabbedArea(any_of(LocalKey, KEY_CTRLADD, KEY_RCTRLADD), m_VerticalBlock);
			Close(1);
			break;

		case KEY_LEFT:      case KEY_NUMPAD4:   case L'4':
			MoveLeft(1);
			break;

		case KEY_RIGHT:     case KEY_NUMPAD6:   case L'6':
			MoveRight(1);
			break;

		case KEY_UP:        case KEY_NUMPAD8:   case L'8':
			if (GArea.Current.y > 0)
				--GArea.Current.y;
			break;

		case KEY_DOWN:      case KEY_NUMPAD2:   case L'2':
			if (GArea.Current.y < ScrY)
				++GArea.Current.y;
			break;

		case KEY_HOME:      case KEY_NUMPAD7:   case L'7':
			GArea.Current.x = 0;
			break;

		case KEY_END:       case KEY_NUMPAD1:   case L'1':
			GArea.Current.x = ScrX;
			break;

		case KEY_PGUP:      case KEY_NUMPAD9:   case L'9':
			GArea.Current.y = 0;
			break;

		case KEY_PGDN:      case KEY_NUMPAD3:   case L'3':
			GArea.Current.y = ScrY;
			break;

		case KEY_CTRLHOME:  case KEY_CTRLNUMPAD7:
		case KEY_RCTRLHOME: case KEY_RCTRLNUMPAD7:
			GArea.Current = {};
			break;

		case KEY_CTRLEND:   case KEY_CTRLNUMPAD1:
		case KEY_RCTRLEND:  case KEY_RCTRLNUMPAD1:
			GArea.Current = { ScrX, ScrY };
			break;

		case KEY_CTRLLEFT:       case KEY_CTRLNUMPAD4:
		case KEY_RCTRLLEFT:      case KEY_RCTRLNUMPAD4:
		case KEY_CTRLSHIFTLEFT:  case KEY_CTRLSHIFTNUMPAD4:
		case KEY_RCTRLSHIFTLEFT: case KEY_RCTRLSHIFTNUMPAD4:
			MoveLeft(10);
			if (any_of(LocalKey, KEY_CTRLSHIFTLEFT, KEY_RCTRLSHIFTLEFT, KEY_CTRLSHIFTNUMPAD4, KEY_RCTRLSHIFTNUMPAD4))
			{
				GArea.Begin = GArea.Current;
			}
			break;

		case KEY_CTRLRIGHT:       case KEY_CTRLNUMPAD6:
		case KEY_RCTRLRIGHT:      case KEY_RCTRLNUMPAD6:
		case KEY_CTRLSHIFTRIGHT:  case KEY_CTRLSHIFTNUMPAD6:
		case KEY_RCTRLSHIFTRIGHT: case KEY_RCTRLSHIFTNUMPAD6:
			MoveRight(10);
			if (any_of(LocalKey, KEY_CTRLSHIFTRIGHT, KEY_RCTRLSHIFTRIGHT, KEY_CTRLSHIFTNUMPAD6, KEY_RCTRLSHIFTNUMPAD6))
			{
				GArea.Begin = GArea.Current;
			}
			break;

		case KEY_CTRLUP:        case KEY_CTRLNUMPAD8:
		case KEY_RCTRLUP:       case KEY_RCTRLNUMPAD8:
		case KEY_CTRLSHIFTUP:   case KEY_CTRLSHIFTNUMPAD8:
		case KEY_RCTRLSHIFTUP:  case KEY_RCTRLSHIFTNUMPAD8:
			GArea.Current.y = std::max(GArea.Current.y - 5, 0);
			if (any_of(LocalKey, KEY_CTRLSHIFTUP, KEY_RCTRLSHIFTUP, KEY_CTRLSHIFTNUMPAD8, KEY_RCTRLSHIFTNUMPAD8))
				GArea.Begin.y = GArea.Current.y;
			break;

		case KEY_CTRLDOWN:       case KEY_CTRLNUMPAD2:
		case KEY_RCTRLDOWN:      case KEY_RCTRLNUMPAD2:
		case KEY_CTRLSHIFTDOWN:  case KEY_CTRLSHIFTNUMPAD2:
		case KEY_RCTRLSHIFTDOWN: case KEY_RCTRLSHIFTNUMPAD2:
			GArea.Current.y = std::min(static_cast<int>(ScrY), GArea.Current.y + 5);
			if (any_of(LocalKey, KEY_CTRLSHIFTDOWN, KEY_RCTRLSHIFTDOWN, KEY_CTRLSHIFTNUMPAD2, KEY_RCTRLSHIFTNUMPAD2))
				GArea.Begin.y = GArea.Current.y;
			break;

		case KEY_SHIFTLEFT:  case KEY_SHIFTNUMPAD4:
			MoveLeft(1);
			GArea.Begin = GArea.Current;
			break;

		case KEY_SHIFTRIGHT: case KEY_SHIFTNUMPAD6:
			MoveRight(1);
			GArea.Begin = GArea.Current;
			break;

		case KEY_SHIFTUP:    case KEY_SHIFTNUMPAD8:
			if (GArea.Begin.y > 0)
				--GArea.Begin.y;
			GArea.Current = GArea.Begin;
			break;

		case KEY_SHIFTDOWN:  case KEY_SHIFTNUMPAD2:
			if (GArea.Begin.y < ScrY)
				++GArea.Begin.y;
			GArea.Current = GArea.Begin;
			break;

		case KEY_SHIFTHOME:  case KEY_SHIFTNUMPAD7:
			GArea.Current.x = GArea.Begin.x = 0;
			break;

		case KEY_SHIFTEND:   case KEY_SHIFTNUMPAD1:
			GArea.Current.x = GArea.Begin.x = ScrX;
			break;

		case KEY_SHIFTPGUP:  case KEY_SHIFTNUMPAD9:
			GArea.Current.y = GArea.Begin.y = 0;
			break;

		case KEY_SHIFTPGDN:  case KEY_SHIFTNUMPAD3:
			GArea.Current.y = GArea.Begin.y = ScrY;
			break;

		case KEY_ALTSHIFTHOME:  case KEY_ALTSHIFTNUMPAD7:
		case KEY_RALTSHIFTHOME: case KEY_RALTSHIFTNUMPAD7:
			GArea.End.x = 0;
			break;

		case KEY_ALTSHIFTEND:   case KEY_ALTSHIFTNUMPAD1:
		case KEY_RALTSHIFTEND:  case KEY_RALTSHIFTNUMPAD1:
			GArea.End.x = ScrX;
			break;

		case KEY_ALTSHIFTPGUP:  case KEY_ALTSHIFTNUMPAD9:
		case KEY_RALTSHIFTPGUP: case KEY_RALTSHIFTNUMPAD9:
			GArea.End.y = 0;
			break;

		case KEY_ALTSHIFTPGDN:  case KEY_ALTSHIFTNUMPAD3:
		case KEY_RALTSHIFTPGDN: case KEY_RALTSHIFTNUMPAD3:
			GArea.End.y = ScrY;
			break;

		case KEY_ALTSHIFTLEFT:  case KEY_ALTSHIFTNUMPAD4:
		case KEY_RALTSHIFTLEFT: case KEY_RALTSHIFTNUMPAD4:
			MovePointLeft(GArea.End, 1);
			break;

		case KEY_ALTSHIFTRIGHT:  case KEY_ALTSHIFTNUMPAD6:
		case KEY_RALTSHIFTRIGHT: case KEY_RALTSHIFTNUMPAD6:
			MovePointRight(GArea.End, 1);
			break;

		case KEY_ALTSHIFTUP:    case KEY_ALTSHIFTNUMPAD8:
		case KEY_RALTSHIFTUP:   case KEY_RALTSHIFTNUMPAD8:
			if (GArea.End.y > 0)
				--GArea.End.y;
			break;

		case KEY_ALTSHIFTDOWN:  case KEY_ALTSHIFTNUMPAD2:
		case KEY_RALTSHIFTDOWN: case KEY_RALTSHIFTNUMPAD2:
			if (GArea.End.y < ScrY)
				++GArea.End.y;
			break;

		case KEY_CTRLA:
		case KEY_RCTRLA:
			GArea.Begin.x = ScrX;
			GArea.Begin.y = ScrY;
			GArea.End.x = 0;
			GArea.End.y = 0;
			GArea.Current = GArea.Begin;
			break;

		case KEY_ALTLEFT:
		case KEY_RALTLEFT:
			{
				const auto& [SelectionBegin, SelectionEnd] = GetSelection();
				if (MovePointLeft(SelectionBegin, 1))
				{
					MovePointLeft(SelectionEnd, 1);
					GArea.Current = GArea.Begin;
				}
			}
			break;

		case KEY_ALTRIGHT:
		case KEY_RALTRIGHT:
			{
				const auto& [SelectionBegin, SelectionEnd] = GetSelection();
				if (MovePointRight(SelectionEnd, 1))
				{
					MovePointRight(SelectionBegin, 1);
					GArea.Current = GArea.Begin;
				}
			}
			break;

		case KEY_ALTUP:
		case KEY_RALTUP:
			if (GArea.Begin.y && GArea.End.y)
			{
				--GArea.Begin.y;
				--GArea.End.y;
				GArea.Current = GArea.Begin;
			}
			break;

		case KEY_ALTDOWN:
		case KEY_RALTDOWN:
			if (GArea.Begin.y < ScrY && GArea.End.y < ScrY)
			{
				++GArea.Begin.y;
				++GArea.End.y;
				GArea.Current = GArea.Begin;
			}
			break;

		case KEY_ALTHOME:
		case KEY_RALTHOME:
			GArea.Begin.x = GArea.Current.x = abs(GArea.Begin.x - GArea.End.x);
			GArea.End.x = 0;
			break;

		case KEY_ALTEND:
		case KEY_RALTEND:
			GArea.End.x = ScrX - abs(GArea.Begin.x - GArea.End.x);
			GArea.Begin.x = GArea.Current.x = ScrX;
			break;

		case KEY_ALTPGUP:
		case KEY_RALTPGUP:
			GArea.Begin.y = GArea.Current.y = abs(GArea.Begin.y - GArea.End.y);
			GArea.End.y = 0;
			break;

		case KEY_ALTPGDN:
		case KEY_RALTPGDN:
			GArea.End.y = ScrY - abs(GArea.Begin.y - GArea.End.y);
			GArea.Begin.y = GArea.Current.y = ScrY;
			break;
	}

	Global->WindowManager->RefreshWindow();
	return true;
}


bool Grabber::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (MouseEvent->dwEventFlags==DOUBLE_CLICK ||
	        (!MouseEvent->dwEventFlags && (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED)))
	{
		ProcessKey(Manager::Key(KEY_ENTER));
		return true;
	}

	if (IntKeyState.MouseButtonState!=FROM_LEFT_1ST_BUTTON_PRESSED)
		return false;

	if (!MouseEvent->dwEventFlags)
	{
		ResetArea = true;
	}
	else if (MouseEvent->dwEventFlags == MOUSE_MOVED && ResetArea)
	{
		GArea.End = GArea.Current;
		ResetArea = false;
	}

	GArea.Current.x = std::clamp(IntKeyState.MousePos.x, 0, int(ScrX));
	GArea.Current.y = std::clamp(IntKeyState.MousePos.y, 0, int(ScrY));

	if (MouseEvent->dwEventFlags == MOUSE_MOVED)
	{
		GArea.Begin = GArea.Current;
	}

	Global->WindowManager->RefreshWindow();
	return true;
}

void Grabber::Reset()
{
	GArea.Begin = GArea.End = GArea.Current;
	ResetArea = false;
}

void Grabber::ResizeConsole()
{
	Close(0);
}

bool RunGraber()
{
	static bool InGrabber=false;

	if (!InGrabber)
	{
		InGrabber=true;
		FlushInputBuffer();
		Grabber::create();
		InGrabber=false;
		return true;
	}

	return false;
}
