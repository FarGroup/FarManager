/*
menubar.cpp

Показ горизонтального меню при включенном "Always show menu bar"

*/

/* Revision: 1.01 05.01.2001 $ */

/*
Modify:
  05.01.2001 SVS
    ! Вместо "толпы" инклудов - стандартный интернал заголовок
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 05.01.2001 SVS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */

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
