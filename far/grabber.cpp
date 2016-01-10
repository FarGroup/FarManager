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

#include "headers.hpp"
#pragma hdrstop

#include "grabber.hpp"
#include "keyboard.hpp"
#include "keys.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "window.hpp"
#include "interf.hpp"
#include "clipboard.hpp"
#include "config.hpp"
#include "help.hpp"
#include "strmix.hpp"

Grabber::Grabber(private_tag):
	PrevArea(),
	GArea(),
	ResetArea(true),
	m_VerticalBlock(false),
	m_StreamSelection()
{
}

grabber_ptr Grabber::create()
{
	auto GrabberPtr = std::make_shared<Grabber>(private_tag());
	GrabberPtr->init();
	return GrabberPtr;
}

void Grabber::init()
{
	const auto CurrentWindow = Global->WindowManager->GetCurrentWindow();
	CurrentWindow->Lock();
	SaveScr = std::make_unique<SaveScreen>();
	bool Visible=false;
	DWORD Size=0;
	GetCursorType(Visible,Size);

	if (Visible)
		GetCursorPos(GArea.Current.X, GArea.Current.Y);
	else
	{
		GArea.Current.X = 0;
		GArea.Current.Y = 0;
	}

	GArea.Begin.X = -1;
	SetCursorType(true, 60);
	PrevArea=GArea;
	DisplayObject();
	Process();
	SaveScr.reset();
	CurrentWindow->Unlock();
	Global->WindowManager->RefreshWindow();
}


Grabber::~Grabber()
{
}

static wchar_t GetChar(const FAR_CHAR_INFO& Cell)
{
	WORD Chr2 = Cell.Char;
	wchar_t Chr = Cell.Char;
	if (Global->Opt->CleanAscii)
	{
		switch (Chr2)
		{
		case 0x07: Chr = L'*'; break;
		case 0x10: Chr = L'>'; break;
		case 0x11: Chr = L'<'; break;
		case 0x15: Chr = L'$'; break;
		case 0x16: Chr = L'-'; break;
		case 0x18: Chr = L'|'; break;
		case 0x19: Chr = L'|'; break;
		case 0x1A: Chr = L'>'; break;
		case 0x1B: Chr = L'<'; break;
		case 0x1E: Chr = L'X'; break;
		case 0x1F: Chr = L'X'; break;
		case 0x7F: Chr = L'X'; break;
		case 0xFF: Chr = L' '; break;
		default:
			if (Chr2 < L' ')
				Chr = L'.';
			else if (Chr2 <= UCHAR_MAX)
				Chr = Chr2;
			break;
		}
	}

	if (Global->Opt->NoGraphics && InRange(BoxSymbols[BS_V1], static_cast<wchar_t>(Chr2), BoxSymbols[BS_LT_H1V1]))
	{
		if (Chr2 == BoxSymbols[BS_V1] || Chr2 == BoxSymbols[BS_V2])
		{
			Chr = L'|';
		}
		else if (Chr2 == BoxSymbols[BS_H1])
		{
			Chr = L'-';
		}
		else if (Chr2 == BoxSymbols[BS_H2])
		{
			Chr = L'=';
		}
		else
		{
			Chr = L'+';
		}
	}
	return Chr;
}

void Grabber::CopyGrabbedArea(bool Append, bool VerticalBlock)
{
	if (GArea.Begin.X < 0)
		return;

	const auto X1 = std::min(GArea.Begin.X, GArea.End.X);
	const auto X2 = std::max(GArea.Begin.X, GArea.End.X);
	const auto Y1 = std::min(GArea.Begin.Y, GArea.End.Y);
	const auto Y2 = std::max(GArea.Begin.Y, GArea.End.Y);

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
	GetText(FromX, FromY, ToX, ToY, CharBuf);

	string CopyBuf;

	CopyBuf.reserve(CharBuf.height() * (CharBuf.width() + 2));

	string Line;
	Line.reserve(CharBuf.width() + 2);

	const auto& SelectionBegin = GArea.Begin.Y == GArea.End.Y?
		GArea.Begin.X < GArea.End.X? GArea.Begin : GArea.End :
		GArea.Begin.Y < GArea.End.Y? GArea.Begin : GArea.End;
	const auto& SelectionEnd = &SelectionBegin == &GArea.Begin? GArea.End : GArea.Begin;


	for (size_t i = 0; i != CharBuf.height(); ++i)
	{
		const auto& MatrixLine = CharBuf[i];
		auto Begin = MatrixLine.cbegin(), End = MatrixLine.cend();

		const auto IsFirstLine = i == 0;
		const auto IsLastLine = i == CharBuf.height() - 1;

		if (m_StreamSelection)
		{
			Begin += IsFirstLine? SelectionBegin.X : 0;
			End -= IsLastLine? ScrX - SelectionEnd.X : 0;
		}
		Line.clear();
		std::transform(Begin, End, std::back_inserter(Line), GetChar);
		bool AddEol = true;
		if (m_StreamSelection)
		{
			if (IsLastLine)
			{
				AddEol = false;
			}
			else
			{
				// in stream mode we want to preserve existing line breaks,
				// but at the same time join lines that were split because of the text wrapping.
				// The Windows console doesn't keep EOL characters at all, so we will try to guess.
				// If the line ends with an alphanumeric character, it's probably has been wrapped.
				// TODO: consider analysing the beginning of the next line too.
				AddEol = !IsAlphaNum(Line.back());
			}
		}
		if (AddEol)
		{
			RemoveTrailingSpaces(Line);
			Line += L"\r\n";
		}
		CopyBuf += Line;
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
					OldData += L"\r\n";
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
	MoveCursor(GArea.Current.X, GArea.Current.Y);

	if (PrevArea.Begin.X != GArea.Begin.X || PrevArea.End.X != GArea.End.X ||
	    PrevArea.Begin.Y != GArea.Begin.Y || PrevArea.End.Y != GArea.End.Y)
	{
		const auto X1 = std::min(GArea.Begin.X,GArea.End.X);
		const auto X2 = std::max(GArea.Begin.X,GArea.End.X);
		const auto Y1 = std::min(GArea.Begin.Y,GArea.End.Y);
		const auto Y2 = std::max(GArea.Begin.Y,GArea.End.Y);

		if (X1 > std::min(PrevArea.Begin.X, PrevArea.End.X) || X2 < std::max(PrevArea.Begin.X, PrevArea.End.X) ||
		    Y1 > std::min(PrevArea.Begin.Y, PrevArea.End.Y) || Y2 < std::max(PrevArea.Begin.Y, PrevArea.End.Y))
			SaveScr->RestoreArea(FALSE);

		if (GArea.Begin.X != -1)
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
			GetText(FromX, FromY, ToX, ToY, CharBuf);

			for (int Y = FromY; Y <= ToY; Y++)
			{
				for (int X = FromX; X <= ToX; X++)
				{
					const auto& CurColor = SaveScr->ScreenBuf[Y][X].Attributes;
					auto& Destination = CharBuf[Y - Y1][X - FromX].Attributes;
					Destination = CurColor;

					if (m_StreamSelection)
					{
						bool ToUp = GArea.Begin.Y < GArea.End.Y;
						bool ToDown = !ToUp;
						bool FirstLine = Y == FromY;
						bool LastLine = Y == ToY;

						if (ToDown)
						{
							if (FirstLine && LastLine)
							{
								if (X < X1 || X > X2)
								{
									continue;
								}
							}
							else if ((FirstLine && X < GArea.End.X) || (LastLine && X > GArea.Begin.X))
								continue;
						}
						else
						{
							if ((FirstLine && X < GArea.Begin.X) || (LastLine && X > GArea.End.X))
								continue;
						}
					}

					Destination.BackgroundColor = (CurColor.Flags & FCF_BG_4BIT? ~INDEXVALUE(CurColor.BackgroundColor) : ~COLORVALUE(CurColor.BackgroundColor)) | ALPHAVALUE(CurColor.BackgroundColor);
					Destination.ForegroundColor = (CurColor.Flags & FCF_FG_4BIT? ~INDEXVALUE(CurColor.ForegroundColor) : ~COLORVALUE(CurColor.ForegroundColor)) | ALPHAVALUE(CurColor.ForegroundColor);
				}
			}

			PutText(FromX, FromY, ToX, ToY, CharBuf.data());
		}

		if (GArea.Begin.X == -2)
		{
			SaveScr->RestoreArea(FALSE);
			GArea.Begin.X = GArea.End.X;
		}

		PrevArea=GArea;
	}
}


int Grabber::ProcessKey(const Manager::Key& Key)
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
	SetCursorType(true, 60);

	if (Global->CtrlObject->Macro.IsExecuting())
	{
		if ((LocalKey&KEY_SHIFT) && LocalKey!=KEY_NONE && ResetArea)
			Reset();
		else if (LocalKey!=KEY_IDLE && LocalKey!=KEY_NONE && !(LocalKey&KEY_SHIFT) && !IntKeyState.ShiftPressed && !IntKeyState.AltPressed)
			ResetArea = true;
	}
	else
	{
		if ((IntKeyState.ShiftPressed || LocalKey!=KEY_SHIFT) && (LocalKey&KEY_SHIFT) && LocalKey!=KEY_NONE && LocalKey!=KEY_CTRLA && LocalKey!=KEY_RCTRLA && !IntKeyState.AltPressed && ResetArea)
			Reset();
		else if (LocalKey!=KEY_IDLE && LocalKey!=KEY_NONE && LocalKey!=KEY_SHIFT && LocalKey!=KEY_CTRLA && LocalKey!=KEY_RCTRLA && !IntKeyState.ShiftPressed && !IntKeyState.AltPressed && !(LocalKey&KEY_SHIFT) && LocalKey != KEY_F1)
			ResetArea = true;
	}

	const auto Move = [this](COORD& What, int Count, int Direction, int LimitX, int LimitY, int NewX)
	{
		for (; Count; --Count)
		{
			if (What.X != LimitX)
			{
				What.X += Direction;
			}
			else if (m_StreamSelection)
			{
				if (What.Y != LimitY)
				{
					What.Y += Direction;
					What.X = NewX;
				}
				else
				{
					break;
				}
			}
		}
	};

	const auto MoveCoordLeft = [&](COORD& What, int Count)
	{
		return Move(What, Count, -1, 0, 0, ScrX);
	};

	const auto MoveCoordRight = [&](COORD& What, int Count)
	{
		return Move(What, Count, 1, ScrX, ScrY, 0);
	};

	const auto MoveLeft = [&](int Count)
	{
		return MoveCoordLeft(GArea.Current, Count);
	};

	const auto MoveRight = [&](int Count)
	{
		return MoveCoordRight(GArea.Current, Count);
	};

	switch (LocalKey)
	{
		case KEY_F1:
			Help::create(L"MiscCmd");
			break;

		case KEY_CTRLU:
		case KEY_RCTRLU:
			Reset();
			GArea.Begin.X = -2;
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
			CopyGrabbedArea(LocalKey == KEY_CTRLADD || LocalKey == KEY_RCTRLADD,m_VerticalBlock);
			Close(1);
			break;

		case KEY_LEFT:      case KEY_NUMPAD4:   case L'4':
			MoveLeft(1);
			break;

		case KEY_RIGHT:     case KEY_NUMPAD6:   case L'6':
			MoveRight(1);
			break;

		case KEY_UP:        case KEY_NUMPAD8:   case L'8':
			if (GArea.Current.Y > 0)
				--GArea.Current.Y;
			break;

		case KEY_DOWN:      case KEY_NUMPAD2:   case L'2':
			if (GArea.Current.Y < ScrY)
				++GArea.Current.Y;
			break;

		case KEY_HOME:      case KEY_NUMPAD7:   case L'7':
			GArea.Current.X = 0;
			break;

		case KEY_END:       case KEY_NUMPAD1:   case L'1':
			GArea.Current.X = ScrX;
			break;

		case KEY_PGUP:      case KEY_NUMPAD9:   case L'9':
			GArea.Current.Y = 0;
			break;

		case KEY_PGDN:      case KEY_NUMPAD3:   case L'3':
			GArea.Current.Y = ScrY;
			break;

		case KEY_CTRLHOME:  case KEY_CTRLNUMPAD7:
		case KEY_RCTRLHOME: case KEY_RCTRLNUMPAD7:
			GArea.Current.X = GArea.Current.Y = 0;
			break;

		case KEY_CTRLEND:   case KEY_CTRLNUMPAD1:
		case KEY_RCTRLEND:  case KEY_RCTRLNUMPAD1:
			GArea.Current.X = ScrX;
			GArea.Current.Y = ScrY;
			break;

		case KEY_CTRLLEFT:       case KEY_CTRLNUMPAD4:
		case KEY_RCTRLLEFT:      case KEY_RCTRLNUMPAD4:
		case KEY_CTRLSHIFTLEFT:  case KEY_CTRLSHIFTNUMPAD4:
		case KEY_RCTRLSHIFTLEFT: case KEY_RCTRLSHIFTNUMPAD4:
			MoveLeft(10);
			if (LocalKey == KEY_CTRLSHIFTLEFT || LocalKey == KEY_RCTRLSHIFTLEFT || LocalKey == KEY_CTRLSHIFTNUMPAD4 || LocalKey == KEY_RCTRLSHIFTNUMPAD4)
			{
				GArea.Begin = GArea.Current;
			}
			break;

		case KEY_CTRLRIGHT:       case KEY_CTRLNUMPAD6:
		case KEY_RCTRLRIGHT:      case KEY_RCTRLNUMPAD6:
		case KEY_CTRLSHIFTRIGHT:  case KEY_CTRLSHIFTNUMPAD6:
		case KEY_RCTRLSHIFTRIGHT: case KEY_RCTRLSHIFTNUMPAD6:
			MoveRight(10);
			if (LocalKey == KEY_CTRLSHIFTRIGHT || LocalKey == KEY_RCTRLSHIFTRIGHT || LocalKey == KEY_CTRLSHIFTNUMPAD6 || LocalKey == KEY_RCTRLSHIFTNUMPAD6)
			{
				GArea.Begin = GArea.Current;
			}
			break;

		case KEY_CTRLUP:        case KEY_CTRLNUMPAD8:
		case KEY_RCTRLUP:       case KEY_RCTRLNUMPAD8:
		case KEY_CTRLSHIFTUP:   case KEY_CTRLSHIFTNUMPAD8:
		case KEY_RCTRLSHIFTUP:  case KEY_RCTRLSHIFTNUMPAD8:
			if ((GArea.Current.Y -= 5) < 0)
				GArea.Current.Y = 0;
			if (LocalKey == KEY_CTRLSHIFTUP || LocalKey == KEY_RCTRLSHIFTUP || LocalKey == KEY_CTRLSHIFTNUMPAD8 || LocalKey == KEY_RCTRLSHIFTNUMPAD8)
				GArea.Begin.Y = GArea.Current.Y;
			break;

		case KEY_CTRLDOWN:       case KEY_CTRLNUMPAD2:
		case KEY_RCTRLDOWN:      case KEY_RCTRLNUMPAD2:
		case KEY_CTRLSHIFTDOWN:  case KEY_CTRLSHIFTNUMPAD2:
		case KEY_RCTRLSHIFTDOWN: case KEY_RCTRLSHIFTNUMPAD2:
			if ((GArea.Current.Y += 5) > ScrY)
				GArea.Current.Y = ScrY;
			if (LocalKey == KEY_CTRLSHIFTDOWN || LocalKey == KEY_RCTRLSHIFTDOWN || LocalKey == KEY_CTRLSHIFTNUMPAD8 || LocalKey == KEY_RCTRLSHIFTNUMPAD8)
				GArea.Begin.Y = GArea.Current.Y;
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
			if (GArea.Begin.Y > 0)
				--GArea.Begin.Y;
			GArea.Current = GArea.Begin;
			break;

		case KEY_SHIFTDOWN:  case KEY_SHIFTNUMPAD2:
			if (GArea.Begin.Y < ScrY)
				++GArea.Begin.Y;
			GArea.Current = GArea.Begin;
			break;

		case KEY_SHIFTHOME:  case KEY_SHIFTNUMPAD7:
			GArea.Current.X = GArea.Begin.X = 0;
			break;

		case KEY_SHIFTEND:   case KEY_SHIFTNUMPAD1:
			GArea.Current.X = GArea.Begin.X = ScrX;
			break;

		case KEY_SHIFTPGUP:  case KEY_SHIFTNUMPAD9:
			GArea.Current.Y = GArea.Begin.Y = 0;
			break;

		case KEY_SHIFTPGDN:  case KEY_SHIFTNUMPAD3:
			GArea.Current.Y = GArea.Begin.Y = ScrY;
			break;

		case KEY_ALTSHIFTHOME:  case KEY_ALTSHIFTNUMPAD7:
		case KEY_RALTSHIFTHOME: case KEY_RALTSHIFTNUMPAD7:
			GArea.End.X = 0;
			break;

		case KEY_ALTSHIFTEND:   case KEY_ALTSHIFTNUMPAD1:
		case KEY_RALTSHIFTEND:  case KEY_RALTSHIFTNUMPAD1:
			GArea.End.X = ScrX;
			break;

		case KEY_ALTSHIFTPGUP:  case KEY_ALTSHIFTNUMPAD9:
		case KEY_RALTSHIFTPGUP: case KEY_RALTSHIFTNUMPAD9:
			GArea.End.Y = 0;
			break;

		case KEY_ALTSHIFTPGDN:  case KEY_ALTSHIFTNUMPAD3:
		case KEY_RALTSHIFTPGDN: case KEY_RALTSHIFTNUMPAD3:
			GArea.End.Y = ScrY;
			break;

		case KEY_ALTSHIFTLEFT:  case KEY_ALTSHIFTNUMPAD4:
		case KEY_RALTSHIFTLEFT: case KEY_RALTSHIFTNUMPAD4:
			MoveCoordLeft(GArea.End, 1);
			break;

		case KEY_ALTSHIFTRIGHT:  case KEY_ALTSHIFTNUMPAD6:
		case KEY_RALTSHIFTRIGHT: case KEY_RALTSHIFTNUMPAD6:
			MoveCoordRight(GArea.End, 1);
			break;

		case KEY_ALTSHIFTUP:    case KEY_ALTSHIFTNUMPAD8:
		case KEY_RALTSHIFTUP:   case KEY_RALTSHIFTNUMPAD8:
			if (GArea.End.Y > 0)
				--GArea.End.Y;
			break;

		case KEY_ALTSHIFTDOWN:  case KEY_ALTSHIFTNUMPAD2:
		case KEY_RALTSHIFTDOWN: case KEY_RALTSHIFTNUMPAD2:
			if (GArea.End.Y < ScrY)
				++GArea.End.Y;
			break;

		case KEY_CTRLA:
		case KEY_RCTRLA:
			GArea.Begin.X = ScrX;
			GArea.Begin.Y = ScrY;
			GArea.End.X = 0;
			GArea.End.Y = 0;
			GArea.Current = GArea.Begin;
			break;

		case KEY_ALTLEFT:
		case KEY_RALTLEFT:
			if (GArea.Begin.X && GArea.End.X)
			{
				--GArea.Begin.X;
				--GArea.End.X;
				GArea.Current = GArea.Begin;
			}
			break;

		case KEY_ALTRIGHT:
		case KEY_RALTRIGHT:
			if (GArea.Begin.X < ScrX && GArea.End.X < ScrX)
			{
				++GArea.Begin.X;
				++GArea.End.X;
				GArea.Current = GArea.Begin;
			}
			break;

		case KEY_ALTUP:
		case KEY_RALTUP:
			if (GArea.Begin.Y && GArea.End.Y)
			{
				--GArea.Begin.Y;
				--GArea.End.Y;
				GArea.Current = GArea.Begin;
			}
			break;

		case KEY_ALTDOWN:
		case KEY_RALTDOWN:
			if (GArea.Begin.Y < ScrY && GArea.End.Y < ScrY)
			{
				++GArea.Begin.Y;
				++GArea.End.Y;
				GArea.Current = GArea.Begin;
			}
			break;

		case KEY_ALTHOME:
		case KEY_RALTHOME:
			GArea.Begin.X = GArea.Current.X = abs(GArea.Begin.X - GArea.End.X);
			GArea.End.X = 0;
			break;

		case KEY_ALTEND:
		case KEY_RALTEND:
			GArea.End.X = ScrX - abs(GArea.Begin.X - GArea.End.X);
			GArea.Begin.X = GArea.Current.X = ScrX;
			break;

		case KEY_ALTPGUP:
		case KEY_RALTPGUP:
			GArea.Begin.Y = GArea.Current.Y = abs(GArea.Begin.Y - GArea.End.Y);
			GArea.End.Y = 0;
			break;

		case KEY_ALTPGDN:
		case KEY_RALTPGDN:
			GArea.End.Y = ScrY - abs(GArea.Begin.Y - GArea.End.Y);
			GArea.Begin.Y = GArea.Current.Y = ScrY;
			break;

	}

	DisplayObject();
	return TRUE;
}


int Grabber::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (MouseEvent->dwEventFlags==DOUBLE_CLICK ||
	        (!MouseEvent->dwEventFlags && (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED)))
	{
		ProcessKey(Manager::Key(KEY_ENTER));
		return TRUE;
	}

	if (IntKeyState.MouseButtonState!=FROM_LEFT_1ST_BUTTON_PRESSED)
		return FALSE;

	GArea.Current.X = std::min(std::max(SHORT(0), IntKeyState.MouseX), ScrX);
	GArea.Current.Y = std::min(std::max(SHORT(0), IntKeyState.MouseY), ScrY);

	if (!MouseEvent->dwEventFlags)
		ResetArea = true;
	else if (MouseEvent->dwEventFlags==MOUSE_MOVED)
	{
		if (ResetArea)
		{
			GArea.End = GArea.Current;
			ResetArea = false;
		}

		GArea.Begin = GArea.Current;
	}

	//VerticalBlock=MouseEvent->dwControlKeyState&(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED);
	DisplayObject();
	return TRUE;
}

void Grabber::Reset()
{
	GArea.Begin = GArea.End = GArea.Current;
	ResetArea = false;
	//DisplayObject();
}

void Grabber::ResizeConsole(void)
{
	Close(0);
}

bool RunGraber()
{
	static bool InGrabber=false;

	if (!InGrabber)
	{
		InGrabber=true;
		Global->WaitInMainLoop=FALSE;
		FlushInputBuffer();
		Grabber::create();
		InGrabber=false;
		return true;
	}

	return false;
}
