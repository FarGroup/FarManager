/*
menubar.cpp

Показ горизонтального меню при включенном "Always show menu bar"

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


void MenuBar::DisplayObject()
{
  char Msg[100],FullMsg[500];
  sprintf(Msg,"    %s    %s    %s    %s    %s  ",MSG(MMenuLeftTitle),
          MSG(MMenuFilesTitle),MSG(MMenuCommandsTitle),
          MSG(MMenuOptionsTitle),MSG(MMenuRightTitle));
  RemoveHighlights(Msg);
  int Length=X2-X1+1;
  sprintf(FullMsg,"%-*.*s",Length,Length,Msg);
  GotoXY(X1,Y1);
  SetColor(COL_HMENUTEXT);
  Text(FullMsg);
}
