/*
savescr.cpp

Сохраняем и восстанавливааем экран кусками и целиком

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#define STRICT

#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
#include <windows.h>
#endif
#if !defined(__NEW_H)
#pragma option -p-
#include <new.h>
#pragma option -p.
#endif

#ifndef __FARCONST_HPP__
#include "farconst.hpp"
#endif
#ifndef __KEYS_HPP__
#include "keys.hpp"
#endif
#ifndef __FARLANG_HPP__
#include "lang.hpp"
#endif
#ifndef __COLOROS_HPP__
#include "colors.hpp"
#endif
#ifndef __FARSTRUCT_HPP__
#include "struct.hpp"
#endif
#ifndef __PLUGIN_HPP__
#include "plugin.hpp"
#endif
#ifndef __CLASSES_HPP__
#include "classes.hpp"
#endif
#ifndef __FARFUNC_HPP__
#include "fn.hpp"
#endif
#ifndef __FARGLOBAL_HPP__
#include "global.hpp"
#endif


SaveScreen::SaveScreen()
{
  RealScreen=FALSE;
  SaveArea(0,0,ScrX,ScrY);
}


SaveScreen::SaveScreen(int RealScreen)
{
  SaveScreen::RealScreen=RealScreen;
  SaveArea(0,0,ScrX,ScrY);
}


SaveScreen::SaveScreen(int X1,int Y1,int X2,int Y2,int RealScreen)
{
  SaveScreen::RealScreen=RealScreen;
  SaveArea(X1,Y1,X2,Y2);
}


SaveScreen::~SaveScreen()
{
  if (!ScreenBuf)
    return;
  RestoreArea();
  delete ScreenBuf;
}


void SaveScreen::Discard()
{
  delete ScreenBuf;
  ScreenBuf=NULL;
}


void SaveScreen::RestoreArea(int RestoreCursor)
{
  if (!ScreenBuf)
    return;
  if (RealScreen)
    PutRealText(X1,Y1,X2,Y2,ScreenBuf);
  else
    PutText(X1,Y1,X2,Y2,ScreenBuf);
  if (RestoreCursor)
    if (RealScreen)
    {
      SetRealCursorType(CurVisible,CurSize);
      MoveRealCursor(CurPosX,CurPosY);
    }
    else
    {
      SetCursorType(CurVisible,CurSize);
      MoveCursor(CurPosX,CurPosY);
    }
}


void SaveScreen::SaveArea(int X1,int Y1,int X2,int Y2)
{
  ScreenBuf=new char[(X2-X1+1)*(Y2-Y1+1)*4+10];
  if (!ScreenBuf)
    return;
  SaveScreen::X1=X1;
  SaveScreen::Y1=Y1;
  SaveScreen::X2=X2;
  SaveScreen::Y2=Y2;
  if (RealScreen)
  {
    GetRealText(X1,Y1,X2,Y2,ScreenBuf);
    GetRealCursorPos(CurPosX,CurPosY);
    GetRealCursorType(CurVisible,CurSize);
  }
  else
  {
    GetText(X1,Y1,X2,Y2,ScreenBuf);
    GetCursorPos(CurPosX,CurPosY);
    GetCursorType(CurVisible,CurSize);
  }
}


void SaveScreen::AppendArea(SaveScreen *NewArea)
{
  CHAR_INFO *Buf=(CHAR_INFO *)ScreenBuf,*NewBuf=(CHAR_INFO *)NewArea->ScreenBuf;
  if (Buf==NULL || NewBuf==NULL)
    return;
  for (int X=X1;X<=X2;X++)
    if (X>=NewArea->X1 && X<=NewArea->X2)
      for (int Y=Y1;Y<=Y2;Y++)
        if (Y>=NewArea->Y1 && Y<=NewArea->Y2)
          Buf[X-X1+(X2-X1+1)*(Y-Y1)]=NewBuf[X-NewArea->X1+(NewArea->X2-NewArea->X1+1)*(Y-NewArea->Y1)];
}
