/*
scrobj.cpp

Parent class для всех screen objects

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

ScreenObject::ScreenObject()
{
  Visible=0;
  X1=Y1=X2=Y2=0;
  SaveScr=NULL;
  ShadowSaveScr=NULL;
  EnableRestoreScreen=TRUE;
  SetPositionDone=FALSE;
  ObjWidth=ObjHeight=0;
}


ScreenObject::~ScreenObject()
{
  if (!EnableRestoreScreen)
  {
    if (ShadowSaveScr)
      ShadowSaveScr->Discard();
    if (SaveScr)
      SaveScr->Discard();
  }
  delete ShadowSaveScr;
  delete SaveScr;
}


void ScreenObject::SetPosition(int X1,int Y1,int X2,int Y2)
{
  ScreenObject::X1=X1;
  ScreenObject::Y1=Y1;
  ScreenObject::X2=X2;
  ScreenObject::Y2=Y2;
  ObjWidth=X2-X1+1;
  ObjHeight=Y2-Y1+1;
  SetPositionDone=TRUE;
}


void ScreenObject::GetPosition(int& X1,int& Y1,int& X2,int& Y2)
{
  X1=ScreenObject::X1;
  Y1=ScreenObject::Y1;
  X2=ScreenObject::X2;
  Y2=ScreenObject::Y2;
}


void ScreenObject::Hide()
{
  if (!Visible)
    return;
  Visible=FALSE;
  delete ShadowSaveScr;
  ShadowSaveScr=NULL;
  delete SaveScr;
  SaveScr=NULL;
}


void ScreenObject::Show()
{
  if (!SetPositionDone)
    return;
  SavePrevScreen();
  DisplayObject();
}


void ScreenObject::SavePrevScreen()
{
  if (!SetPositionDone)
    return;
  if (!Visible)
  {
    Visible=TRUE;
    if (EnableRestoreScreen && SaveScr==NULL)
      SaveScr=new SaveScreen(X1,Y1,X2,Y2);
  }
}


void ScreenObject::Redraw()
{
  if (IsVisible())
    Show();
}


void ScreenObject::Shadow()
{
  if (IsVisible())
  {
    if (ShadowSaveScr==NULL)
      ShadowSaveScr=new SaveScreen(X1,Y1,X2+2,Y2+1);
    MakeShadow(X1+2,Y2+1,X2+1,Y2+1);
    MakeShadow(X2+1,Y1+1,X2+2,Y2+1);
  }
}


