/*
hmenu.cpp

Горизонтальное меню

*/

/* Revision: 1.13 10.11.2004 $ */

/*
Modify:
  10.11.2004 SVS
    + В HMenu и TreeList добавлена обработка MCODE_*
  11.12.2002 SVS
    - учтем вариант с KEY_CONSOLE_BUFFER_RESIZE (динамическое изменение размера консоли)
  24.05.2002 SVS
    + Дублирование Numpad-клавиш
  30.03.2002 OT
    - После исправления бага №314 (патч 1250) отвалилось закрытие
      фара по кресту.
  23.02.2002 DJ
    - косметика от BoundsChecker
  13.08.2001 OT
    - исправление бага в вертикальном меню
  26.07.2001 OT
    - Поправлен ResizeConsole()
  26.07.2001 SVS
    ! VFMenu уничтожен как класс
    ! HMenu прикручен к фреймам - попытка "раз"
  26.06.2001 SVS
    + Обработка KEY_TAB на внешних менюхах - для макросов.
  14.06.2001 OT
    ! "Бунт" ;-)
  20.05.2001 SVS
    ! Константы MENU_ - в морг
  12.05.2001 DJ
    - EnableRestoreScreen=TRUE
  06.05.2001 DJ
    ! перетрях #include
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "hmenu.hpp"
#include "fn.hpp"
#include "colors.hpp"
#include "keys.hpp"
#include "global.hpp"
#include "dialog.hpp"
#include "vmenu.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "macroopcode.hpp"
#include "panel.hpp"
#include "savescr.hpp"
#include "lockscrn.hpp"

HMenu::HMenu(struct HMenuData *Item,int ItemCount)
{
  SetDynamicallyBorn(FALSE);
  SubMenu=NULL;
  HMenu::Item=Item;
  HMenu::ItemCount=ItemCount;
  /* $ 12.05.2001 DJ */
  SetRestoreScreenMode (TRUE);
  /* DJ $ */
  /* $ 23.02.2002 DJ */
  VExitCode = -1;
  /* DJ $ */
  FrameManager->ModalizeFrame(this);
}


void HMenu::DisplayObject()
{
  SetScreen(X1,Y1,X2,Y2,' ',COL_HMENUTEXT);
  SetCursorType(0,10);
  ShowMenu();
}


void HMenu::ShowMenu()
{
  char TmpStr[256];
  int I;
  GotoXY(X1+2,Y1);
  for (I=0;I<ItemCount;I++)
  {
    ItemX[I]=WhereX();
    if (Item[I].Selected)
      SetColor(COL_HMENUSELECTEDTEXT);
    else
      SetColor(COL_HMENUTEXT);
    sprintf(TmpStr,"  %s  ",Item[I].Name);
    HiText(TmpStr,Item[I].Selected ? COL_HMENUSELECTEDHIGHLIGHT:COL_HMENUHIGHLIGHT);
  }
  ItemX[ItemCount]=WhereX();
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
    case MCODE_OP_PLAINTEXT:
    {
      const char *str = eStackAsString();
      if (!*str)
        return FALSE;
      Key=*str;
      break;
    }
    case MCODE_C_EMPTY:
      return ItemCount<=0;
    case MCODE_C_EOF:
      return SelectPos==ItemCount-1;
    case MCODE_C_BOF:
      return SelectPos==0;
    case MCODE_C_SELECTED:
      return ItemCount > 0 && SelectPos >= 0;
/*
    case MCODE_F_MENU_CHECKHOTKEY:
    {
      const char *str = eStackAsString(1);
      if ( *str )
        return CheckHighlights(*str);
      return FALSE;
    }
*/
  }

  switch(Key)
  {
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

    case KEY_LEFT:      case KEY_NUMPAD4:
    {
      Item[SelectPos].Selected=0;
      if (--SelectPos<0)
        SelectPos=ItemCount-1;
      Item[SelectPos].Selected=1;
      ShowMenu();
      return(TRUE);
    }

    case KEY_RIGHT:     case KEY_NUMPAD6:
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
        if (Dialog::IsKeyHighlighted(Item[I].Name,Key,FALSE))
        {
          Item[SelectPos].Selected=0;
          Item[I].Selected=1;
          SelectPos=I;
          ShowMenu();
          ProcessKey(KEY_ENTER);
          return(TRUE);
        }
      for (I=0;I<ItemCount;I++)
        if (Dialog::IsKeyHighlighted(Item[I].Name,Key,TRUE))
        {
          Item[SelectPos].Selected=0;
          Item[I].Selected=1;
          SelectPos=I;
          ShowMenu();
          ProcessKey(KEY_ENTER);
          return(TRUE);
        }
      return(FALSE);
    }
  }
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


void HMenu::ProcessSubMenu(struct MenuData *Data,int DataCount,
                           char *SubMenuHelp,int X,int Y,int &Position)
{
  if (SubMenu!=NULL)
    delete SubMenu;
  Position=-1;
  SubMenu=new VMenu("",Data,DataCount);
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
      if (Key==KEY_LEFT || Key==KEY_RIGHT || Key == KEY_TAB)
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
