/*
keybar.cpp

Keybar

*/

/* Revision: 1.15 06.08.2004 $ */

/*
Modify:
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  01.04.2002 SVS
    ! Вместо KEY_FOCUS_CHANGED заведем KEY_KILLFOCUS и KEY_GOTFOCUS.
  03.03.2002 SVS
    ! Зачем делать лишние телодвижения, когда все за один раз можно сделать.
  27.09.2001 IS
    - Левый размер при использовании strncpy
  22.06.2001
    ! ProcessMouse: Owner меняем на FrameManager.
  14.06.2001 OT
    ! "Бунт" ;-)
  06.05.2001 DJ
    ! перетрях #include
  30.04.2001 DJ
    ! Все нафиг переделано :-) Убран (почти) весь дублирующийся код.
      Публичный API сохранен.
  28.04.2001 VVM
    + ProcessKey()
  04.04.2001 SVS
    - Избавляемся от "залипания" :-)
    ! убран "мусор" - ненужные новые переменные CtrlShiftState
  17.01.2001 SVS
    - Вернем обратно предыдущее изменение в связи с очередным уточнением клавиш
  07.01.2001 OT
    - После смены клавиатуры выскочил баг:
      "Кликаю мышкой на кейбаре, например, на f10 - ноль реакции."
  19.09.2000 SVS
    ! При нажатии Ctrl-Alt-Shift неверно отображается KeyBar
  07.08.2000 SVS
    + Изменение любого Label - функция Change(Group,...)
  02.08.2000 SVS
    + Дополнительные индикаторы
      CtrlShiftName, AltShiftName, CtrlAltName
    + К этим индикаторам - функции
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "keybar.hpp"
#include "fn.hpp"
#include "colors.hpp"
#include "global.hpp"
#include "keys.hpp"
#include "manager.hpp"

KeyBar::KeyBar()
{
  _OT(SysLog("[%p] KeyBar::KeyBar()", this));
  DisableMask=0;
  Owner=NULL;
  AltState=CtrlState=ShiftState=0;
  memset (KeyTitles, 0, sizeof (KeyTitles));
  memset (KeyCounts, 0, sizeof (KeyCounts));
}


void KeyBar::SetOwner(ScreenObject *Owner)
{
  KeyBar::Owner=Owner;
}


/* $ 02.08.2000 SVS
   Переработка с учетом новых индикаторов
*/
void KeyBar::DisplayObject()
{
  int I;
  GotoXY(X1,Y1);
  AltState=CtrlState=ShiftState=0;

  int KeyWidth=(X2-X1-1)/12;
  if (KeyWidth<8)
    KeyWidth=8;

  int LabelWidth=KeyWidth-2;
  for (I=0; I<KEY_COUNT; I++)
  {
    if (WhereX()+LabelWidth>=X2)
      break;
    SetColor(COL_KEYBARNUM);
    mprintf("%d",I+1);
    SetColor(COL_KEYBARTEXT);
    char *Label="";

    if (ShiftPressed)
    {
      ShiftState=ShiftPressed;
      if (CtrlPressed)
      {
        CtrlState=CtrlPressed;
        if(!AltPressed) // Ctrl-Alt-Shift - это особый случай :-)
        {
          if (I<KeyCounts [KBL_CTRLSHIFT])
            Label=KeyTitles [KBL_CTRLSHIFT][I];
        }
      }
      else if (AltPressed)
      {
        if (I<KeyCounts [KBL_ALTSHIFT])
          Label=KeyTitles [KBL_ALTSHIFT][I];
        AltState=AltPressed;
      }
      else
      {
        if (I<KeyCounts [KBL_SHIFT])
          Label=KeyTitles [KBL_SHIFT][I];
      }
    }
    else if (CtrlPressed)
    {
      CtrlState=CtrlPressed;
      if (AltPressed)
      {
        if (I<KeyCounts [KBL_CTRLALT])
          Label=KeyTitles [KBL_CTRLALT][I];
        AltState=AltPressed;
      }
      else
      {
        if (I<KeyCounts [KBL_CTRL])
          Label=KeyTitles [KBL_CTRL][I];
      }
    }
    else if (AltPressed)
    {
      AltState=AltPressed;
      if (I<KeyCounts [KBL_ALT])
        Label=KeyTitles [KBL_ALT][I];
    }
    else
      if (I<KeyCounts [KBL_MAIN] && (DisableMask & (1<<I))==0)
        Label=KeyTitles [KBL_MAIN][I];

    mprintf("%-*.*s",LabelWidth,LabelWidth,Label);
    if (I<KEY_COUNT-1)
    {
      SetColor(COL_KEYBARBACKGROUND);
      Text(" ");
    }
  }
  int Width=X2-WhereX()+1;
  if (Width>0)
  {
    SetColor(COL_KEYBARTEXT);
    mprintf("%*s",Width,"");
  }
}
/* SVS $ */

void KeyBar::SetGroup(int Group,char **Key,int KeyCount)
{
  for (int i=0; i<KeyCount && i<KEY_COUNT; i++)
    xstrncpy (KeyTitles [Group][i], Key [i], sizeof (KeyTitles [Group][i])-1);
  KeyCounts [Group]=KeyCount;
}

void KeyBar::ClearGroup(int Group)
{
  memset (KeyTitles[Group], 0, sizeof (KeyTitles[Group]));
  KeyCounts [Group] = 0;
}

/* $ 07.08.2000 SVS
   Изменение любого Label
*/
void KeyBar::Change(int Group,char *NewStr,int Pos)
{
  xstrncpy (KeyTitles [Group][Pos], NewStr, sizeof (KeyTitles [Group][Pos])-1);
}
/* SVS $ */


/* $ 30.04.2001 DJ
   Групповая установка идущих подряд строк LNG для указанной группы
*/

void KeyBar::SetAllGroup (int Group, int StartIndex, int Count)
{
  if (Count > KEY_COUNT)
    Count = KEY_COUNT;
  for (int i=0, Index=StartIndex; i<Count; i++, Index++)
    xstrncpy (KeyTitles [Group][i], MSG (Index), sizeof (KeyTitles [Group][i])-1);
  KeyCounts [Group] = Count;
}

/* DJ $ */

/* $ 28.04.2001 VVM
  + ProcessKey() */
int KeyBar::ProcessKey(int Key)
{
  switch (Key)
  {
    case KEY_KILLFOCUS:
    case KEY_GOTFOCUS:
      RedrawIfChanged();
      return(TRUE);
  } /* switch */
  return(FALSE);
}

/* VVM */
int KeyBar::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  INPUT_RECORD rec;
  int Key;
  if (!IsVisible())
    return(FALSE);
  if ((MouseEvent->dwButtonState & 3)==0 || MouseEvent->dwEventFlags!=0)
    return(FALSE);
  if (MouseEvent->dwMousePosition.X<X1 || MouseEvent->dwMousePosition.X>X2 ||
      MouseEvent->dwMousePosition.Y!=Y1)
    return(FALSE);
  int KeyWidth=(X2-X1-1)/12;
  if (KeyWidth<8)
    KeyWidth=8;
  int X=MouseEvent->dwMousePosition.X-X1;
  if (X<KeyWidth*9)
    Key=X/KeyWidth;
  else
    Key=9+(X-KeyWidth*9)/(KeyWidth+1);


  while (1)
  {
    GetInputRecord(&rec);
    if (rec.EventType==MOUSE_EVENT && (rec.Event.MouseEvent.dwButtonState & 3)==0)
      break;
  }

  if (rec.Event.MouseEvent.dwMousePosition.X<X1 ||
      rec.Event.MouseEvent.dwMousePosition.X>X2 ||
      rec.Event.MouseEvent.dwMousePosition.Y!=Y1)
    return(FALSE);

  int NewKey,NewX=MouseEvent->dwMousePosition.X-X1;
  if (NewX<KeyWidth*9)
    NewKey=NewX/KeyWidth;
  else
    NewKey=9+(NewX-KeyWidth*9)/(KeyWidth+1);

  if (Key!=NewKey)
    return(FALSE);

  if (Key>11)
    Key=11;

  /* $ 02.08.2000 SVS
     Добавка к новым индикаторам
  */
  if (MouseEvent->dwControlKeyState & (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED) ||
      (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED))
  {
    if (MouseEvent->dwControlKeyState & SHIFT_PRESSED)
      Key+=KEY_ALTSHIFTF1;
    else if (MouseEvent->dwControlKeyState & (RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED))
      Key+=KEY_CTRLALTF1;
    else
      Key+=KEY_ALTF1;
  }
  else if (MouseEvent->dwControlKeyState & (RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED))
  {
    if (MouseEvent->dwControlKeyState & SHIFT_PRESSED)
      Key+=KEY_CTRLSHIFTF1;
    else
      Key+=KEY_CTRLF1;
  }
  else if (MouseEvent->dwControlKeyState & SHIFT_PRESSED)
    Key+=KEY_SHIFTF1;
  else
    Key+=KEY_F1;
  /* SVS $ */

  /*$ 22.06.2001 SKV
    Типа всё круто! :)
  */
  //if (Owner)
    //Owner->ProcessKey(Key);
      FrameManager->ProcessKey(Key);
  /* SKV$*/
  return(TRUE);
}


void KeyBar::RedrawIfChanged()
{
  if (ShiftPressed!=ShiftState ||
      CtrlPressed!=CtrlState ||
      AltPressed!=AltState)
  {
    //_SVS("KeyBar::RedrawIfChanged()");
    Redraw();
  }
}


void KeyBar::SetDisableMask(int Mask)
{
  DisableMask=Mask;
}

void KeyBar::ResizeConsole()
{
}
