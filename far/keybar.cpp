/*
keybar.cpp

Keybar

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */

KeyBar::KeyBar()
{
  KeyCount=ShiftKeyCount=AltKeyCount=CtrlKeyCount=0;
  AltState=CtrlState=ShiftState=0;
  DisableMask=0;
  Owner=NULL;
  memset(KeyName,0,sizeof(KeyName));
  memset(AltKeyName,0,sizeof(AltKeyName));
  memset(CtrlKeyName,0,sizeof(CtrlKeyName));
  memset(ShiftKeyName,0,sizeof(ShiftKeyName));
}


void KeyBar::SetOwner(BaseInput *Owner)
{
  KeyBar::Owner=Owner;
}


void KeyBar::DisplayObject()
{
  int I;
  GotoXY(X1,Y1);
  AltState=CtrlState=ShiftState=0;
  int KeyWidth=(X2-X1-1)/12;
  if (KeyWidth<8)
    KeyWidth=8;
  int LabelWidth=KeyWidth-2;
  for (I=0;I<sizeof(KeyName)/sizeof(KeyName[0]);I++)
  {
    if (WhereX()+LabelWidth>=X2)
      break;
    SetColor(COL_KEYBARNUM);
    mprintf("%d",I+1);
    SetColor(COL_KEYBARTEXT);
    char *Label="";
    if (ShiftPressed)
    {
      if (I<ShiftKeyCount)
        Label=ShiftKeyName[I];
      ShiftState=1;
    }
    else
      if (CtrlPressed)
      {
        if (I<CtrlKeyCount)
          Label=CtrlKeyName[I];
        CtrlState=1;
      }
      else
        if (AltPressed)
        {
          if (I<AltKeyCount)
            Label=AltKeyName[I];
          AltState=1;
        }
        else
          if (I<KeyCount && (DisableMask & (1<<I))==0)
            Label=KeyName[I];
    mprintf("%-*.*s",LabelWidth,LabelWidth,Label);
    if (I<sizeof(KeyName)/sizeof(KeyName[0])-1)
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


void KeyBar::Set(char **Key,int KeyCount)
{
  int I;
  for (I=0;I<KeyCount && I<sizeof(KeyName)/sizeof(KeyName[0]);I++)
    strncpy(KeyName[I],Key[I],sizeof(KeyName[I]));
  KeyBar::KeyCount=KeyCount;
}


void KeyBar::SetShift(char **Key,int KeyCount)
{
  int I;
  for (I=0;I<KeyCount && I<sizeof(ShiftKeyName)/sizeof(ShiftKeyName[0]);I++)
    strncpy(ShiftKeyName[I],Key[I],sizeof(ShiftKeyName[I]));
  KeyBar::ShiftKeyCount=KeyCount;
}


void KeyBar::SetAlt(char **Key,int KeyCount)
{
  int I;
  for (I=0;I<KeyCount && I<sizeof(AltKeyName)/sizeof(AltKeyName[0]);I++)
    strncpy(AltKeyName[I],Key[I],sizeof(AltKeyName[I]));
  KeyBar::AltKeyCount=KeyCount;
}


void KeyBar::SetCtrl(char **Key,int KeyCount)
{
  int I;
  for (I=0;I<KeyCount && I<sizeof(CtrlKeyName)/sizeof(CtrlKeyName[0]);I++)
    strncpy(CtrlKeyName[I],Key[I],sizeof(CtrlKeyName[I]));
  KeyBar::CtrlKeyCount=KeyCount;
}


void KeyBar::Change(char *NewStr,int Pos)
{
  strncpy(KeyName[Pos],NewStr,sizeof(KeyName[Pos]));
}


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

  if (MouseEvent->dwControlKeyState & (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED) ||
      (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED))
    Key+=KEY_ALTF1;
  else
    if (MouseEvent->dwControlKeyState & (RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED))
      Key+=KEY_CTRLF1;
    else
      if (MouseEvent->dwControlKeyState & SHIFT_PRESSED)
        Key+=KEY_SHIFTF1;
      else
        Key+=KEY_F1;
  if (Owner)
    Owner->ProcessKey(Key);
  return(TRUE);
}


void KeyBar::RedrawIfChanged()
{
  if (ShiftPressed!=ShiftState || CtrlPressed!=CtrlState ||
      AltPressed!=AltState)
    Redraw();
}


void KeyBar::SetDisableMask(int Mask)
{
  DisableMask=Mask;
}

