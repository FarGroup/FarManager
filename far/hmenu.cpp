/*
hmenu.cpp

Горизонтальное меню
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

HMenu::HMenu(struct HMenuData *Item,int ItemCount)
{
  SetDynamicallyBorn(FALSE);
  SubMenu=NULL;
  HMenu::Item=Item;
  HMenu::ItemCount=ItemCount;

  SetRestoreScreenMode (TRUE);
  VExitCode = -1;
  FrameManager->ModalizeFrame(this);
}


void HMenu::DisplayObject()
{
  SetScreen(X1,Y1,X2,Y2,L' ',COL_HMENUTEXT);
  SetCursorType(0,10);
  ShowMenu();
}


void HMenu::ShowMenu()
{
    string strTmpStr;
  int I;
  GotoXY(X1+2,Y1);
  for (I=0;I<ItemCount;I++)
  {
    ItemX[I]=WhereX();
    if (Item[I].Selected)
      SetColor(COL_HMENUSELECTEDTEXT);
    else
      SetColor(COL_HMENUTEXT);

		strTmpStr=L"  ";
		strTmpStr+=Item[I].Name;
		strTmpStr+=L"  ";
    HiText(strTmpStr,Item[I].Selected ? COL_HMENUSELECTEDHIGHLIGHT:COL_HMENUHIGHLIGHT);
  }
  ItemX[ItemCount]=WhereX();
}


__int64 HMenu::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
  int I;

  for (SelectPos=0,I=0;I<ItemCount;I++)
    if (Item[I].Selected)
    {
      SelectPos=I;
      break;
    }

    string strName;

  switch(OpCode)
  {
    case MCODE_C_EMPTY:
      return (__int64)(ItemCount<=0);
    case MCODE_C_EOF:
      return (__int64)(SelectPos==ItemCount-1);
    case MCODE_C_BOF:
      return (__int64)(SelectPos==0);
    case MCODE_C_SELECTED:
      return (__int64)(ItemCount > 0 && SelectPos >= 0);
    case MCODE_V_ITEMCOUNT:
      return (__int64)ItemCount;
    case MCODE_V_CURPOS:
      return (__int64)(SelectPos+1);
    case MCODE_F_MENU_CHECKHOTKEY:
    {
      const wchar_t *str = (const wchar_t *)vParam;
      if ( *str )
        return (__int64)CheckHighlights((WORD)*str);
      return _i64(0);
    }
    case MCODE_F_MENU_GETHOTKEY:
    {
      if(iParam == _i64(-1))
        iParam=(__int64)SelectPos;

      if((int)iParam < ItemCount)
        return (__int64)((DWORD)GetHighlights((const struct HMenuData *)(Item+(int)iParam)));
      return _i64(0);
    }
  }

  return _i64(0);
}

int HMenu::ProcessKey(int Key)
{
  int I;

  for (SelectPos=0,I=0;I<ItemCount;I++)
    if (Item[I].Selected)
    {
      SelectPos=I;
      break;
    }

  switch(Key)
  {
    case KEY_ALTF9:
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
      return(FALSE);
    }

    case KEY_F1:
    {
      ShowHelp();
      return(TRUE);
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
        return(TRUE);
      }
      return(FALSE);
    }

    case KEY_TAB:
    {
      Item[SelectPos].Selected=0;
      /* Кусок для "некрайних" меню - прыжок к меню пассивной панели */
      if(SelectPos != 0 && SelectPos != ItemCount-1)
      {
        if (CtrlObject->Cp()->ActivePanel==CtrlObject->Cp()->RightPanel)
          SelectPos=0;
        else
          SelectPos=ItemCount-1;
      }
      else
      /**/
      {
        if(SelectPos==0)
          SelectPos=ItemCount-1;
        else
          SelectPos=0;
      }
      Item[SelectPos].Selected=1;
      ShowMenu();
      return(TRUE);
    }

    case KEY_ESC:
    case KEY_F10:
    {
      EndLoop=TRUE;
      Modal::ExitCode=-1;
      return(FALSE);
    }

    case KEY_HOME:      case KEY_NUMPAD7:
    case KEY_CTRLHOME:  case KEY_CTRLNUMPAD7:
    case KEY_CTRLPGUP:  case KEY_CTRLNUMPAD9:
    {
      Item[SelectPos].Selected=0;
      Item[0].Selected=1;
      SelectPos=0;
      ShowMenu();
      return(TRUE);
    }

    case KEY_END:       case KEY_NUMPAD1:
    case KEY_CTRLEND:   case KEY_CTRLNUMPAD1:
    case KEY_CTRLPGDN:  case KEY_CTRLNUMPAD3:
    {
      Item[SelectPos].Selected=0;
      Item[ItemCount-1].Selected=1;
      SelectPos=ItemCount-1;
      ShowMenu();
      return(TRUE);
    }

    case KEY_LEFT:      case KEY_NUMPAD4:      case KEY_MSWHEEL_LEFT:
    {
      Item[SelectPos].Selected=0;
      if (--SelectPos<0)
        SelectPos=ItemCount-1;
      Item[SelectPos].Selected=1;
      ShowMenu();
      return(TRUE);
    }

    case KEY_RIGHT:     case KEY_NUMPAD6:      case KEY_MSWHEEL_RIGHT:
    {
      Item[SelectPos].Selected=0;
      if (++SelectPos==ItemCount)
        SelectPos=0;
      Item[SelectPos].Selected=1;
      ShowMenu();
      return(TRUE);
    }

    default:
    {
      for (I=0;I<ItemCount;I++)
      {
        if (Dialog::IsKeyHighlighted(Item[I].Name,Key,FALSE))
        {
          Item[SelectPos].Selected=0;
          Item[I].Selected=1;
          SelectPos=I;
          ShowMenu();
          ProcessKey(KEY_ENTER);
          return(TRUE);
        }
      }
      for (I=0;I<ItemCount;I++)
      {
        if (Dialog::IsKeyHighlighted(Item[I].Name,Key,TRUE))
        {
          Item[SelectPos].Selected=0;
          Item[I].Selected=1;
          SelectPos=I;
          ShowMenu();
          ProcessKey(KEY_ENTER);
          return(TRUE);
        }
      }
      return(FALSE);
    }
  }

  return FALSE;
}


int HMenu::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  int I;
  int MsX,MsY;

  for (SelectPos=0,I=0;I<ItemCount;I++)
    if (Item[I].Selected)
    {
      SelectPos=I;
      break;
    }

  MsX=MouseEvent->dwMousePosition.X;
  MsY=MouseEvent->dwMousePosition.Y;
  if (MsY==Y1 && MsX>=X1 && MsX<=X2)
  {
    for (I=0;I<ItemCount;I++)
      if (MsX>=ItemX[I] && MsX<ItemX[I+1])
      {
        if (SubMenu!=NULL && SelectPos==I)
          return(FALSE);
        Item[SelectPos].Selected=0;
        Item[I].Selected=1;
        SelectPos=I;
        ShowMenu();
        ProcessKey(KEY_ENTER);
      }
  }
  else
    if ((MouseEvent->dwButtonState & 3)==0 && MouseEvent->dwEventFlags==0)
      ProcessKey(KEY_ESC);
  return(TRUE);
}


void HMenu::GetExitCode(int &ExitCode,int &VExitCode)
{
  ExitCode=Modal::ExitCode;
  VExitCode=HMenu::VExitCode;
}


void HMenu::ProcessSubMenu(struct MenuDataEx *Data,int DataCount,
                           const wchar_t *SubMenuHelp,int X,int Y,int &Position)
{
  if (SubMenu!=NULL)
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
    if(Key==KEY_CONSOLE_BUFFER_RESIZE)
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
          SubMenu=NULL;
          return;
        }
      SubMenu->ProcessMouse(&rec.Event.MouseEvent);
    }
    else
    {
      if (Key == KEY_LEFT || Key == KEY_RIGHT ||Key == KEY_TAB ||
          Key == KEY_NUMPAD4 || Key == KEY_NUMPAD6 ||
          Key == KEY_MSWHEEL_LEFT || Key == KEY_MSWHEEL_RIGHT)
      {
        delete SubMenu;
        SubMenu=NULL;
        ProcessKey(Key);
        ProcessKey(KEY_ENTER);
        return;
      }
      SubMenu->ProcessKey(Key);
    }
  }

  Position=SubMenu->Modal::GetExitCode();
  delete SubMenu;
  SubMenu=NULL;
}

void HMenu::ResizeConsole()
{
  if(SaveScr)
  {
    SaveScr->Discard();
    delete SaveScr;
    SaveScr=NULL;
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

wchar_t HMenu::GetHighlights(const struct HMenuData *_item)
{
  CriticalSectionLock Lock(CS);

  wchar_t Ch=0;

  if(_item)
  {
    const wchar_t *Name=_item->Name;
    if(Name && *Name)
    {
      const wchar_t *ChPtr=wcschr(Name,L'&');

      if (ChPtr)
        Ch=ChPtr[1];
    }
  }

  return Ch;
}

BOOL HMenu::CheckHighlights(WORD CheckSymbol)
{
  CriticalSectionLock Lock(CS);

  for (int I=0; I < ItemCount; I++)
  {
    wchar_t Ch=GetHighlights((const struct HMenuData *)(Item+I));

    if(Ch && Upper(CheckSymbol) == Upper(Ch))
      return TRUE;
  }
  return FALSE;
}
