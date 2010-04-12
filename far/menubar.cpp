/*
menubar.cpp

Показ горизонтального меню при включенном "Always show menu bar"

*/

#include "headers.hpp"
#pragma hdrstop

#include "menubar.hpp"
#include "lang.hpp"
#include "colors.hpp"
#include "fn.hpp"

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
