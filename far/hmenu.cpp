/*
hmenu.cpp

Горизонтальное меню
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

#include "hmenu.hpp"
#include "colors.hpp"
#include "keys.hpp"
#include "dialog.hpp"
#include "vmenu.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "macroopcode.hpp"
#include "panel.hpp"
#include "savescr.hpp"
#include "lockscrn.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "palette.hpp"

HMenu::HMenu(HMenuData *Item,int ItemCount):
	SubMenu(nullptr),
	Item(Item),
	ItemCount(ItemCount),
	VExitCode(-1)
{
	SetDynamicallyBorn(FALSE);
	SetRestoreScreenMode(TRUE);
	FrameManager->ModalizeFrame(this);
}


void HMenu::DisplayObject()
{
	SetScreen(X1,Y1,X2,Y2,L' ',ColorIndexToColor(COL_HMENUTEXT));
	SetCursorType(0,10);
	ShowMenu();
}


void HMenu::ShowMenu()
{
	string strTmpStr;
	GotoXY(X1+2,Y1);

	for (int i=0; i<ItemCount; i++)
	{
		ItemX[i]=WhereX();

		if (Item[i].Selected)
			SetColor(COL_HMENUSELECTEDTEXT);
		else
			SetColor(COL_HMENUTEXT);

		strTmpStr=L"  ";
		strTmpStr+=Item[i].Name;
		strTmpStr+=L"  ";
		HiText(strTmpStr,ColorIndexToColor(Item[i].Selected ? COL_HMENUSELECTEDHIGHLIGHT:COL_HMENUHIGHLIGHT));
	}

	ItemX[ItemCount]=WhereX();
}


__int64 HMenu::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	SelectPos=0;
	for (int i=0; i<ItemCount; i++)
	{
		if (Item[i].Selected)
		{
			SelectPos=i;
			break;
		}
	}

	string strName;

	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return ItemCount<=0;
		case MCODE_C_EOF:
			return SelectPos==ItemCount-1;
		case MCODE_C_BOF:
			return !SelectPos;
		case MCODE_C_SELECTED:
			return ItemCount > 0 && SelectPos >= 0;
		case MCODE_V_ITEMCOUNT:
			return ItemCount;
		case MCODE_V_CURPOS:
			return SelectPos+1;
		case MCODE_F_MENU_CHECKHOTKEY:
		{
			return (__int64)(CheckHighlights(*(const wchar_t *)vParam, (int)iParam)+1);
		}
		case MCODE_F_MENU_GETHOTKEY:
		case MCODE_F_MENU_GETVALUE: // S=Menu.GetValue([N])
		{
			if (iParam == -1)
				iParam=SelectPos;

			if ((int)iParam < ItemCount)
			{
				if (OpCode == MCODE_F_MENU_GETVALUE)
				{
					*(string *)vParam=Item[(int)iParam].Name;
					return 1;
				}
				else
				{
					return (__int64)((DWORD)GetHighlights((const HMenuData *)(Item+(int)iParam)));
				}
			}

			return 0;
		}
		case MCODE_F_MENU_ITEMSTATUS: // N=Menu.ItemStatus([N])
		{
			__int64 RetValue=-1;

			if (iParam == -1)
				iParam=SelectPos;

			if ((int)iParam < ItemCount)
			{
				RetValue=0;
				if (Item[(int)iParam].Selected)
					RetValue |= 1;
			}

			return RetValue;
		}
		case MCODE_V_MENU_VALUE: // Menu.Value
		{
			*(string *)vParam=Item[SelectPos].Name;
			return 1;
		}
	}

	return 0;
}

int HMenu::ProcessKey(int Key)
{
	SelectPos=0;
	for (int i=0; i<ItemCount; i++)
	{
		if (Item[i].Selected)
		{
			SelectPos=i;
			break;
		}
	}

	switch (Key)
	{
		case KEY_ALTF9:
		case KEY_RALTF9:
			FrameManager->ProcessKey(KEY_ALTF9);
			break;
		case KEY_OP_PLAINTEXT:
		{
			const wchar_t *str = eStackAsString();

			if (!*str)
				return FALSE;

			Key=*str;
			break;
		}
		case KEY_NONE:
		case KEY_IDLE:
		{
			return FALSE;
		}
		case KEY_F1:
		{
			ShowHelp();
			return TRUE;
		}
		case KEY_NUMENTER:
		case KEY_ENTER:
		case KEY_DOWN:    case KEY_NUMPAD2:
		{
			if (Item[SelectPos].SubMenu)
			{
				ProcessSubMenu(Item[SelectPos].SubMenu,Item[SelectPos].SubMenuSize,
				               Item[SelectPos].SubMenuHelp,ItemX[SelectPos],
				               Y1+1,VExitCode);

				if (VExitCode!=-1)
				{
					EndLoop=TRUE;
					Modal::ExitCode=SelectPos;
				}

				return TRUE;
			}

			return FALSE;
		}
		case KEY_TAB:
		{
			Item[SelectPos].Selected=0;

			/* Кусок для "некрайних" меню - прыжок к меню пассивной панели */
			if (SelectPos  && SelectPos != ItemCount-1)
			{
				if (CtrlObject->Cp()->ActivePanel==CtrlObject->Cp()->RightPanel)
					SelectPos=0;
				else
					SelectPos=ItemCount-1;
			}
			else
				/**/
			{
				if (!SelectPos)
					SelectPos=ItemCount-1;
				else
					SelectPos=0;
			}

			Item[SelectPos].Selected=1;
			ShowMenu();
			return TRUE;
		}
		case KEY_ESC:
		case KEY_F10:
		{
			EndLoop=TRUE;
			Modal::ExitCode=-1;
			return FALSE;
		}
		case KEY_HOME:      case KEY_NUMPAD7:
		case KEY_CTRLHOME:  case KEY_CTRLNUMPAD7:
		case KEY_RCTRLHOME: case KEY_RCTRLNUMPAD7:
		case KEY_CTRLPGUP:  case KEY_CTRLNUMPAD9:
		case KEY_RCTRLPGUP: case KEY_RCTRLNUMPAD9:
		{
			Item[SelectPos].Selected=0;
			Item[0].Selected=1;
			SelectPos=0;
			ShowMenu();
			return TRUE;
		}
		case KEY_END:       case KEY_NUMPAD1:
		case KEY_CTRLEND:   case KEY_CTRLNUMPAD1:
		case KEY_RCTRLEND:  case KEY_RCTRLNUMPAD1:
		case KEY_CTRLPGDN:  case KEY_CTRLNUMPAD3:
		case KEY_RCTRLPGDN: case KEY_RCTRLNUMPAD3:
		{
			Item[SelectPos].Selected=0;
			Item[ItemCount-1].Selected=1;
			SelectPos=ItemCount-1;
			ShowMenu();
			return TRUE;
		}
		case KEY_LEFT:      case KEY_NUMPAD4:      case KEY_MSWHEEL_LEFT:
		{
			Item[SelectPos].Selected=0;

			if (--SelectPos<0)
				SelectPos=ItemCount-1;

			Item[SelectPos].Selected=1;
			ShowMenu();
			return TRUE;
		}
		case KEY_RIGHT:     case KEY_NUMPAD6:      case KEY_MSWHEEL_RIGHT:
		{
			Item[SelectPos].Selected=0;

			if (++SelectPos==ItemCount)
				SelectPos=0;

			Item[SelectPos].Selected=1;
			ShowMenu();
			return TRUE;
		}
		default:
		{
			for (int i=0; i<ItemCount; i++)
			{
				if (IsKeyHighlighted(Item[i].Name,Key,FALSE))
				{
					Item[SelectPos].Selected=0;
					Item[i].Selected=1;
					SelectPos=static_cast<int>(i);
					ShowMenu();
					ProcessKey(KEY_ENTER);
					return TRUE;
				}
			}

			for (int i=0; i<ItemCount; i++)
			{
				if (IsKeyHighlighted(Item[i].Name,Key,TRUE))
				{
					Item[SelectPos].Selected=0;
					Item[i].Selected=1;
					SelectPos=static_cast<int>(i);
					ShowMenu();
					ProcessKey(KEY_ENTER);
					return TRUE;
				}
			}

			return FALSE;
		}
	}

	return FALSE;
}


int HMenu::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	int MsX,MsY;

	SelectPos=0;
	for (int i=0; i<ItemCount; i++)
	{
		if (Item[i].Selected)
		{
			SelectPos=i;
			break;
		}
	}

	MsX=MouseEvent->dwMousePosition.X;
	MsY=MouseEvent->dwMousePosition.Y;

	if (MsY==Y1 && MsX>=X1 && MsX<=X2)
	{
		for (int i=0; i<ItemCount; i++)
			if (MsX>=ItemX[i] && MsX<ItemX[i+1])
			{
				if (SubMenu && SelectPos==i)
					return FALSE;

				Item[SelectPos].Selected=0;
				Item[i].Selected=1;
				SelectPos=i;
				ShowMenu();
				ProcessKey(KEY_ENTER);
			}
	}
	else if (!(MouseEvent->dwButtonState & 3) && !MouseEvent->dwEventFlags)
		ProcessKey(KEY_ESC);

	return TRUE;
}


void HMenu::GetExitCode(int &ExitCode,int &VExitCode)
{
	ExitCode=Modal::ExitCode;
	VExitCode=HMenu::VExitCode;
}


void HMenu::ProcessSubMenu(MenuDataEx *Data,int DataCount,
                           const wchar_t *SubMenuHelp,int X,int Y,int &Position)
{
	if (SubMenu)
		delete SubMenu;

	Position=-1;
	SubMenu=new VMenu(L"",Data,DataCount);
	SubMenu->SetFlags(VMENU_NOTCHANGE);
	SubMenu->SetBoxType(SHORT_DOUBLE_BOX);
	SubMenu->SetFlags(VMENU_WRAPMODE);
	SubMenu->SetHelp(SubMenuHelp);
	SubMenu->SetPosition(X,Y,0,0);
	SubMenu->Show();

	while (!SubMenu->Done() && !CloseFARMenu)
	{
		INPUT_RECORD rec;
		int Key;
		Key=GetInputRecord(&rec);

		if (Key==KEY_CONSOLE_BUFFER_RESIZE)
		{
			LockScreen LckScr;
			ResizeConsole();
			Show();
			SubMenu->Hide();
			SubMenu->Show();
		}
		else if (rec.EventType==MOUSE_EVENT)
		{
			if (rec.Event.MouseEvent.dwMousePosition.Y==Y1)
				if (ProcessMouse(&rec.Event.MouseEvent))
				{
					delete SubMenu;
					SubMenu=nullptr;
					return;
				}
			if(Key == KEY_MSWHEEL_UP || Key == KEY_MSWHEEL_DOWN || Key == KEY_MSWHEEL_RIGHT || Key == KEY_MSWHEEL_LEFT)
			{
				SubMenu->ProcessKey(Key);
			}
			else
			{
				SubMenu->ProcessMouse(&rec.Event.MouseEvent);
			}
		}
		else
		{
			if (Key == KEY_LEFT || Key == KEY_RIGHT ||Key == KEY_TAB ||
			        Key == KEY_NUMPAD4 || Key == KEY_NUMPAD6 ||
			        Key == KEY_MSWHEEL_LEFT || Key == KEY_MSWHEEL_RIGHT)
			{
				delete SubMenu;
				SubMenu=nullptr;
				ProcessKey(Key);
				ProcessKey(KEY_ENTER);
				return;
			}

			SubMenu->ProcessKey(Key);
		}
	}

	Position=SubMenu->Modal::GetExitCode();
	delete SubMenu;
	SubMenu=nullptr;
}

void HMenu::ResizeConsole()
{
	if (SaveScr)
	{
		SaveScr->Discard();
		delete SaveScr;
		SaveScr=nullptr;
	}

	Hide();
	Frame::ResizeConsole();
	SetPosition(0,0,::ScrX,0);
}

void HMenu::Process()
{
	Modal::Process();
}

HMenu::~HMenu()
{
	FrameManager->UnmodalizeFrame(this);
	FrameManager->RefreshFrame();
}

wchar_t HMenu::GetHighlights(const HMenuData *_item)
{
	CriticalSectionLock Lock(CS);
	wchar_t Ch=0;

	if (_item)
	{
		const wchar_t *Name=_item->Name;

		if (Name && *Name)
		{
			const wchar_t *ChPtr=wcschr(Name,L'&');

			if (ChPtr)
				Ch=ChPtr[1];
		}
	}

	return Ch;
}

int HMenu::CheckHighlights(WORD CheckSymbol,int StartPos)
{
	CriticalSectionLock Lock(CS);

	if (StartPos < 0)
		StartPos=0;

	for (int I=StartPos; I < ItemCount; I++)
	{
		wchar_t Ch=GetHighlights((const HMenuData *)(Item+I));

		if (Ch)
		{
			if (Upper(CheckSymbol) == Upper(Ch))
				return I;
		}
		else if (!CheckSymbol)
			return I;
	}

	return -1;
}
