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
	m_VerticalBlock(false)
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
		GetCursorPos(GArea.CurX,GArea.CurY);
	else
	{
		GArea.CurX=0;
		GArea.CurY=0;
	}

	GArea.X1=-1;
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


void Grabber::CopyGrabbedArea(bool Append, bool VerticalBlock)
{
	if (GArea.X1 < 0)
		return;

	int X1,Y1,X2,Y2;
	X1=std::min(GArea.X1,GArea.X2);
	X2=std::max(GArea.X1,GArea.X2);
	Y1=std::min(GArea.Y1,GArea.Y2);
	Y2=std::max(GArea.Y1,GArea.Y2);

	matrix<FAR_CHAR_INFO> CharBuf(Y2 - Y1 + 1, X2 - X1 + 1);
	GetText(X1, Y1, X2, Y2, CharBuf);

	string CopyBuf;
	CopyBuf.reserve(CharBuf.height() * (CharBuf.width() + 2));

	string Line;
	Line.reserve(CharBuf.width() + 2);

	for (size_t I = 0; I != CharBuf.height(); ++I)
	{
		Line.clear();

		if (I)
		{
			CopyBuf.append(L"\r\n");
		}

		FOR(const auto& Cell, CharBuf[I])
		{
			WORD Chr2 = Cell.Char;
			wchar_t Chr = Cell.Char;
			if (Global->Opt->CleanAscii)
			{
				switch (Chr2)
				{
					case 0x07: Chr=L'*'; break;
					case 0x10: Chr=L'>'; break;
					case 0x11: Chr=L'<'; break;
					case 0x15: Chr=L'$'; break;
					case 0x16: Chr=L'-'; break;
					case 0x18: Chr=L'|'; break;
					case 0x19: Chr=L'|'; break;
					case 0x1A: Chr=L'>'; break;
					case 0x1B: Chr=L'<'; break;
					case 0x1E: Chr=L'X'; break;
					case 0x1F: Chr=L'X'; break;
					case 0x7F: Chr=L'X'; break;
					case 0xFF: Chr=L' '; break;
					default:

						if (Chr2 < L' ')
							Chr=L'.';
						else if (Chr2 <= UCHAR_MAX)
							Chr=Chr2;

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
					Chr=L'+';
				}
			}

			Line.push_back(Chr);
		}
		CopyBuf += RemoveTrailingSpaces(Line);
	}

	Clipboard clip;

	if (clip.Open())
	{
		if (Append)
		{
			string OldData;
			if (clip.GetText(OldData))
			{
				if (!OldData.empty() && OldData.back() != L'\n')
				{
					OldData += L"\r\n";
				}
				CopyBuf.insert(0, OldData);
			}
		}

		if (VerticalBlock)
			clip.SetVText(CopyBuf);
		else
			clip.SetText(CopyBuf);
	}
}


void Grabber::DisplayObject()
{
	MoveCursor(GArea.CurX,GArea.CurY);

	if (PrevArea.X1!=GArea.X1 || PrevArea.X2!=GArea.X2 ||
	        PrevArea.Y1!=GArea.Y1 || PrevArea.Y2!=GArea.Y2)
	{
		int X1,Y1,X2,Y2;
		X1=std::min(GArea.X1,GArea.X2);
		X2=std::max(GArea.X1,GArea.X2);
		Y1=std::min(GArea.Y1,GArea.Y2);
		Y2=std::max(GArea.Y1,GArea.Y2);

		if (X1>std::min(PrevArea.X1,PrevArea.X2) || X2<std::max(PrevArea.X1,PrevArea.X2) ||
		        Y1>std::min(PrevArea.Y1,PrevArea.Y2) || Y2<std::max(PrevArea.Y1,PrevArea.Y2))
			SaveScr->RestoreArea(FALSE);

		if (GArea.X1!=-1)
		{
			matrix<FAR_CHAR_INFO> CharBuf(Y2 - Y1 + 1, X2 - X1 + 1);
			GetText(X1, Y1, X2, Y2, CharBuf);

			for (int Y = Y1; Y <= Y2; Y++)
			{
				for (int X = X1; X <= X2; X++)
				{
					const FarColor& CurColor = SaveScr->ScreenBuf[Y][X].Attributes;
					CharBuf[Y - Y1][X - X1].Attributes.BackgroundColor = (CurColor.Flags&FCF_BG_4BIT? ~INDEXVALUE(CurColor.BackgroundColor) : ~COLORVALUE(CurColor.BackgroundColor)) | ALPHAVALUE(CurColor.BackgroundColor);
					CharBuf[Y - Y1][X - X1].Attributes.ForegroundColor = (CurColor.Flags&FCF_FG_4BIT ? ~INDEXVALUE(CurColor.ForegroundColor) : ~COLORVALUE(CurColor.ForegroundColor)) | ALPHAVALUE(CurColor.ForegroundColor);
				}
			}
			PutText(X1, Y1, X2, Y2, CharBuf.data());
		}

		if (GArea.X1==-2)
		{
			SaveScr->RestoreArea(FALSE);
			GArea.X1=GArea.X2;
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

	switch (LocalKey)
	{
		case KEY_F1:
		{
			Help::create(L"MiscCmd");
			break;
		}

		case KEY_CTRLU:
		case KEY_RCTRLU:
			Reset();
			GArea.X1=-2;
			break;
		case KEY_ESC:
			Close(0);
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

			if (GArea.CurX>0)
				GArea.CurX--;

			break;
		case KEY_RIGHT:     case KEY_NUMPAD6:   case L'6':

			if (GArea.CurX<ScrX)
				GArea.CurX++;

			break;
		case KEY_UP:        case KEY_NUMPAD8:   case L'8':

			if (GArea.CurY>0)
				GArea.CurY--;

			break;
		case KEY_DOWN:      case KEY_NUMPAD2:   case L'2':

			if (GArea.CurY<ScrY)
				GArea.CurY++;

			break;
		case KEY_HOME:      case KEY_NUMPAD7:   case L'7':
			GArea.CurX=0;
			break;
		case KEY_END:       case KEY_NUMPAD1:   case L'1':
			GArea.CurX=ScrX;
			break;
		case KEY_PGUP:      case KEY_NUMPAD9:   case L'9':
			GArea.CurY=0;
			break;
		case KEY_PGDN:      case KEY_NUMPAD3:   case L'3':
			GArea.CurY=ScrY;
			break;
		case KEY_CTRLHOME:  case KEY_CTRLNUMPAD7:
		case KEY_RCTRLHOME: case KEY_RCTRLNUMPAD7:
			GArea.CurX=GArea.CurY=0;
			break;
		case KEY_CTRLEND:   case KEY_CTRLNUMPAD1:
		case KEY_RCTRLEND:  case KEY_RCTRLNUMPAD1:
			GArea.CurX=ScrX;
			GArea.CurY=ScrY;
			break;
		case KEY_CTRLLEFT:       case KEY_CTRLNUMPAD4:
		case KEY_RCTRLLEFT:      case KEY_RCTRLNUMPAD4:
		case KEY_CTRLSHIFTLEFT:  case KEY_CTRLSHIFTNUMPAD4:
		case KEY_RCTRLSHIFTLEFT: case KEY_RCTRLSHIFTNUMPAD4:

			if ((GArea.CurX-=10)<0)
				GArea.CurX=0;

			if (LocalKey == KEY_CTRLSHIFTLEFT || LocalKey == KEY_RCTRLSHIFTLEFT || LocalKey == KEY_CTRLSHIFTNUMPAD4 || LocalKey == KEY_RCTRLSHIFTNUMPAD4)
				GArea.X1=GArea.CurX;

			break;
		case KEY_CTRLRIGHT:       case KEY_CTRLNUMPAD6:
		case KEY_RCTRLRIGHT:      case KEY_RCTRLNUMPAD6:
		case KEY_CTRLSHIFTRIGHT:  case KEY_CTRLSHIFTNUMPAD6:
		case KEY_RCTRLSHIFTRIGHT: case KEY_RCTRLSHIFTNUMPAD6:

			if ((GArea.CurX+=10)>ScrX)
				GArea.CurX=ScrX;

			if (LocalKey == KEY_CTRLSHIFTRIGHT || LocalKey == KEY_RCTRLSHIFTRIGHT || LocalKey == KEY_CTRLSHIFTNUMPAD6 || LocalKey == KEY_RCTRLSHIFTNUMPAD6)
				GArea.X1=GArea.CurX;

			break;
		case KEY_CTRLUP:        case KEY_CTRLNUMPAD8:
		case KEY_RCTRLUP:       case KEY_RCTRLNUMPAD8:
		case KEY_CTRLSHIFTUP:   case KEY_CTRLSHIFTNUMPAD8:
		case KEY_RCTRLSHIFTUP:  case KEY_RCTRLSHIFTNUMPAD8:

			if ((GArea.CurY-=5)<0)
				GArea.CurY=0;

			if (LocalKey == KEY_CTRLSHIFTUP || LocalKey == KEY_RCTRLSHIFTUP || LocalKey == KEY_CTRLSHIFTNUMPAD8 || LocalKey == KEY_RCTRLSHIFTNUMPAD8)
				GArea.Y1=GArea.CurY;

			break;
		case KEY_CTRLDOWN:       case KEY_CTRLNUMPAD2:
		case KEY_RCTRLDOWN:      case KEY_RCTRLNUMPAD2:
		case KEY_CTRLSHIFTDOWN:  case KEY_CTRLSHIFTNUMPAD2:
		case KEY_RCTRLSHIFTDOWN: case KEY_RCTRLSHIFTNUMPAD2:

			if ((GArea.CurY+=5)>ScrY)
				GArea.CurY=ScrY;

			if (LocalKey == KEY_CTRLSHIFTDOWN || LocalKey == KEY_RCTRLSHIFTDOWN || LocalKey == KEY_CTRLSHIFTNUMPAD8 || LocalKey == KEY_RCTRLSHIFTNUMPAD8)
				GArea.Y1=GArea.CurY;

			break;
		case KEY_SHIFTLEFT:  case KEY_SHIFTNUMPAD4:

			if (GArea.X1>0)
				GArea.X1--;

			GArea.CurX=GArea.X1;
			GArea.CurY=GArea.Y1;
			break;
		case KEY_SHIFTRIGHT: case KEY_SHIFTNUMPAD6:

			if (GArea.X1<ScrX)
				GArea.X1++;

			GArea.CurX=GArea.X1;
			GArea.CurY=GArea.Y1;
			break;
		case KEY_SHIFTUP:    case KEY_SHIFTNUMPAD8:

			if (GArea.Y1>0)
				GArea.Y1--;

			GArea.CurX=GArea.X1;
			GArea.CurY=GArea.Y1;
			break;
		case KEY_SHIFTDOWN:  case KEY_SHIFTNUMPAD2:

			if (GArea.Y1<ScrY)
				GArea.Y1++;

			GArea.CurX=GArea.X1;
			GArea.CurY=GArea.Y1;
			break;
		case KEY_SHIFTHOME:  case KEY_SHIFTNUMPAD7:
			GArea.CurX=GArea.X1=0;
			break;
		case KEY_SHIFTEND:   case KEY_SHIFTNUMPAD1:
			GArea.CurX=GArea.X1=ScrX;
			break;
		case KEY_SHIFTPGUP:  case KEY_SHIFTNUMPAD9:
			GArea.CurY=GArea.Y1=0;
			break;
		case KEY_SHIFTPGDN:  case KEY_SHIFTNUMPAD3:
			GArea.CurY=GArea.Y1=ScrY;
			break;

		case KEY_ALTSHIFTHOME:  case KEY_ALTSHIFTNUMPAD7:
		case KEY_RALTSHIFTHOME: case KEY_RALTSHIFTNUMPAD7:
			GArea.X2=0;
			break;
		case KEY_ALTSHIFTEND:   case KEY_ALTSHIFTNUMPAD1:
		case KEY_RALTSHIFTEND:  case KEY_RALTSHIFTNUMPAD1:
			GArea.X2=ScrX;
			break;
		case KEY_ALTSHIFTPGUP:  case KEY_ALTSHIFTNUMPAD9:
		case KEY_RALTSHIFTPGUP: case KEY_RALTSHIFTNUMPAD9:
			GArea.Y2=0;
			break;
		case KEY_ALTSHIFTPGDN:  case KEY_ALTSHIFTNUMPAD3:
		case KEY_RALTSHIFTPGDN: case KEY_RALTSHIFTNUMPAD3:
			GArea.Y2=ScrY;
			break;

		case KEY_ALTSHIFTLEFT:  case KEY_ALTSHIFTNUMPAD4:
		case KEY_RALTSHIFTLEFT: case KEY_RALTSHIFTNUMPAD4:
			if (GArea.X2>0)
				GArea.X2--;
			break;
		case KEY_ALTSHIFTRIGHT:  case KEY_ALTSHIFTNUMPAD6:
		case KEY_RALTSHIFTRIGHT: case KEY_RALTSHIFTNUMPAD6:
			if (GArea.X2<ScrX)
				GArea.X2++;
			break;
		case KEY_ALTSHIFTUP:    case KEY_ALTSHIFTNUMPAD8:
		case KEY_RALTSHIFTUP:   case KEY_RALTSHIFTNUMPAD8:
			if (GArea.Y2>0)
				GArea.Y2--;
			break;
		case KEY_ALTSHIFTDOWN:  case KEY_ALTSHIFTNUMPAD2:
		case KEY_RALTSHIFTDOWN: case KEY_RALTSHIFTNUMPAD2:
			if (GArea.Y2<ScrY)
				GArea.Y2++;
			break;

		case KEY_CTRLA:
		case KEY_RCTRLA:
			GArea.X1=GArea.CurX=ScrX;
			GArea.X2=0;
			GArea.Y1=GArea.CurY=ScrY;
			GArea.Y2=0;
			break;

		case KEY_ALTLEFT:
		case KEY_RALTLEFT:
			if ((GArea.X1>0) && (GArea.X2>0))
			{
				GArea.X1--;
				GArea.X2--;
				GArea.CurX=GArea.X1;
				GArea.CurY=GArea.Y1;
			}
			break;
		case KEY_ALTRIGHT:
		case KEY_RALTRIGHT:
			if ((GArea.X1<ScrX) && (GArea.X2<ScrX))
			{
				GArea.X1++;
				GArea.X2++;
				GArea.CurX=GArea.X1;
				GArea.CurY=GArea.Y1;
			}
			break;
		case KEY_ALTUP:
		case KEY_RALTUP:
			if ((GArea.Y1>0) && (GArea.Y2>0))
			{
				GArea.Y1--;
				GArea.Y2--;
				GArea.CurX=GArea.X1;
				GArea.CurY=GArea.Y1;
			}
			break;
		case KEY_ALTDOWN:
		case KEY_RALTDOWN:
			if ((GArea.Y1<ScrY) && (GArea.Y2<ScrY))
			{
				GArea.Y1++;
				GArea.Y2++;
				GArea.CurX=GArea.X1;
				GArea.CurY=GArea.Y1;
			}
			break;

		case KEY_ALTHOME:
		case KEY_RALTHOME:
			GArea.X1=GArea.CurX=abs(GArea.X1-GArea.X2);
			GArea.X2=0;
			break;
		case KEY_ALTEND:
		case KEY_RALTEND:
			GArea.X2=ScrX-abs(GArea.X1-GArea.X2);
			GArea.X1=GArea.CurX=ScrX;
			break;
		case KEY_ALTPGUP:
		case KEY_RALTPGUP:
			GArea.Y1=GArea.CurY=abs(GArea.Y1-GArea.Y2);
			GArea.Y2=0;
			break;
		case KEY_ALTPGDN:
		case KEY_RALTPGDN:
			GArea.Y2=ScrY-abs(GArea.Y1-GArea.Y2);
			GArea.Y1=GArea.CurY=ScrY;
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

	GArea.CurX=std::min(std::max(SHORT(0), IntKeyState.MouseX), ScrX);
	GArea.CurY=std::min(std::max(SHORT(0), IntKeyState.MouseY), ScrY);

	if (!MouseEvent->dwEventFlags)
		ResetArea = true;
	else if (MouseEvent->dwEventFlags==MOUSE_MOVED)
	{
		if (ResetArea)
		{
			GArea.X2=GArea.CurX;
			GArea.Y2=GArea.CurY;
			ResetArea = false;
		}

		GArea.X1=GArea.CurX;
		GArea.Y1=GArea.CurY;
	}

	//VerticalBlock=MouseEvent->dwControlKeyState&(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED);
	DisplayObject();
	return TRUE;
}

void Grabber::Reset()
{
	GArea.X1=GArea.X2=GArea.CurX;
	GArea.Y1=GArea.Y2=GArea.CurY;
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
