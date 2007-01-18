/*
keybar.cpp

Keybar

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
  _OT(SysLog(L"[%p] KeyBar::KeyBar()", this));
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
    mprintfW(L"%d",I+1);
    SetColor(COL_KEYBARTEXT);
    wchar_t *Label=L"";

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

    mprintfW(L"%-*.*s",LabelWidth,LabelWidth,Label);
    if (I<KEY_COUNT-1)
    {
      SetColor(COL_KEYBARBACKGROUND);
      TextW(L" ");
    }
  }
  int Width=X2-WhereX()+1;
  if (Width>0)
  {
    SetColor(COL_KEYBARTEXT);
    mprintfW(L"%*s",Width,L"");
  }
}
/* SVS $ */

void KeyBar::SetGroup(int Group,const wchar_t * const *Key,int KeyCount)
{
  if(!Key) return;

  for (int i=0; i<KeyCount && i<KEY_COUNT; i++)
    if(Key[i])
      xwcsncpy (KeyTitles [Group][i], Key[i], (sizeof (KeyTitles [Group][i])-1)/sizeof (wchar_t));
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
void KeyBar::Change(int Group,const wchar_t *NewStr,int Pos)
{
  if(NewStr)
    xwcsncpy (KeyTitles [Group][Pos], NewStr, (sizeof (KeyTitles [Group][Pos])-1)/sizeof (wchar_t));
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
    xwcsncpy (KeyTitles [Group][i], UMSG (Index), (sizeof (KeyTitles [Group][i])-1)/sizeof (wchar_t));
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
