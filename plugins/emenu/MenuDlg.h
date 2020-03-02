#ifndef _MENUDLG_H_
#define _MENUDLG_H_

#include <windows.h>
#include <shlobj.h>

struct SMenuDlgParam
{
  LPCONTEXTMENU pMenu1;
  LPCONTEXTMENU2 pMenu2;
  LPCONTEXTMENU3 pMenu3;
};

BOOL CALLBACK MenuDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif
