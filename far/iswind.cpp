/*
iswind.cpp

Проверка fullscreen/windowed

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
#ifndef __FARSTRUCT_HPP__
#include "struct.hpp"
#endif

#ifndef __FARGLOBAL_HPP__
#include "global.hpp"
#endif

static BOOL CALLBACK IsWindowedEnumProc(HWND hwnd,LPARAM lParam);

static HWND hFarWnd;
static BOOL WindowedMode=FALSE;
static HICON hOldLargeIcon,hOldSmallIcon;

void DetectWindowedMode()
{
  if (hFarWnd)
    WindowedMode=!IsIconic(hFarWnd);
}

void InitDetectWindowedMode()
{
  EnumWindows(IsWindowedEnumProc,(LPARAM)GetCurrentProcessId());

  if (hFarWnd && Opt.SmallIcon)
  {
    char FarName[NM];
    GetModuleFileName(NULL,FarName,sizeof(FarName));
    HICON hSmallIcon=NULL,hLargeIcon=NULL;
    ExtractIconEx(FarName,0,&hLargeIcon,&hSmallIcon,1);

    if (hLargeIcon!=NULL)
      hOldLargeIcon=(HICON)SendMessage(hFarWnd,WM_SETICON,1,(LPARAM)hLargeIcon);
    if (hSmallIcon!=NULL)
      hOldSmallIcon=(HICON)SendMessage(hFarWnd,WM_SETICON,0,(LPARAM)hSmallIcon);
  }

  DetectWindowedMode();
}


int IsWindowed()
{
  return(WindowedMode);
}


BOOL CALLBACK IsWindowedEnumProc(HWND hwnd,LPARAM FARpid)
{
  DWORD pid;
  GetWindowThreadProcessId(hwnd,&pid);
  if (pid==FARpid)
  {
    hFarWnd=hwnd;
    return(FALSE);
  }
  return(TRUE);
}


void RestoreIcons()
{
  if (hFarWnd && Opt.SmallIcon)
  {
    if (hOldLargeIcon!=NULL)
    {
      SendMessage(hFarWnd,WM_SETICON,1,(LPARAM)hOldLargeIcon);
      SendMessage(hFarWnd,WM_SETICON,0,(LPARAM)(hOldSmallIcon!=NULL ? hOldSmallIcon:hOldLargeIcon));
    }
  }
}
