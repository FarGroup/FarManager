/*
menubar.cpp

����� ��������������� ���� ��� ���������� "Always show menu bar"

*/

/* Revision: 1.03 29.11.2005 $ */

#include "headers.hpp"
#pragma hdrstop

#include "menubar.hpp"
#include "lang.hpp"
#include "colors.hpp"
#include "fn.hpp"

void MenuBar::DisplayObject()
{
  string strMsg, strFullMsg;

  strMsg.Format (L"    %s    %s    %s    %s    %s  ", UMSG(MMenuLeftTitle), UMSG(MMenuFilesTitle), UMSG(MMenuCommandsTitle), UMSG(MMenuOptionsTitle),UMSG(MMenuRightTitle));
  RemoveHighlightsW(strMsg);

  int Length=X2-X1+1;

  strFullMsg.Format (L"%-*.*s", Length,Length, (const wchar_t*)strMsg);
  GotoXY(X1,Y1);
  SetColor(COL_HMENUTEXT);
  TextW(strFullMsg);
}
