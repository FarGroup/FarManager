/*
iswind.cpp

Проверка fullscreen/windowed
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "plugin.hpp"
#include "global.hpp"
#include "fn.hpp"

static BOOL CALLBACK IsWindowedEnumProc(HWND hwnd,LPARAM lParam);

HWND hFarWnd;
static BOOL WindowedMode=FALSE;
static HICON hOldLargeIcon,hOldSmallIcon;

/*
    dwFlags - Specifies the display mode. Options are:

        CONSOLE_FULLSCREEN_MODE - data is displayed fullscreen
        CONSOLE_WINDOWED_MODE - data is displayed in a window

    lpNewScreenBufferDimensions - On output, contains the new dimensions of
        the screen buffer.  The dimensions are in rows and columns for
        textmode screen buffers.

Return Values
  If the function succeeds, the return value is nonzero
*/
typedef BOOL (WINAPI *PROCSETCONSOLEDISPLAYMODEELLWND)(HANDLE hConsoleOutput,DWORD dwFlags,PCOORD lpNewScreenBufferDimensions);

/*
lpModeFlags - [out] Display mode of the console. This parameter can be one or more of the following values.
  CONSOLE_FULLSCREEN Full-screen console. The console is in this mode as soon as the window is maximized. At this point, the transition to full-screen mode can still fail.
  CONSOLE_FULLSCREEN_HARDWARE Full-screen console communicating directly with the video hardware. This mode is set after the console is in CONSOLE_FULLSCREEN mode to indicate that the transition to full-screen mode has completed.
*/
typedef BOOL (WINAPI *PROCGETCONSOLEDISPLAYMODE)(LPDWORD lpModeFlags);
static PROCSETCONSOLEDISPLAYMODEELLWND pfnSetConsoleDisplayMode=NULL;
static PROCGETCONSOLEDISPLAYMODE pfnGetConsoleDisplayMode=NULL;

void DetectWindowedMode()
{
  if (hFarWnd)
    WindowedMode=!IsIconic(hFarWnd);
}

BOOL CALLBACK IsWindowedEnumProc2(HWND hwnd,LPARAM FARTitl)
{
  int LenTitle=GetWindowTextLengthW(hwnd);
  if (LenTitle)
  {
    wchar_t *lpwszTitle=(wchar_t *)xf_malloc((LenTitle+1)*sizeof(wchar_t));
    if (lpwszTitle!=NULL)
    {
      if ((LenTitle=GetWindowTextW(hwnd, lpwszTitle, LenTitle+1)) != 0)
      {
        lpwszTitle[LenTitle]=0;
        if (wcsstr(lpwszTitle,(const wchar_t *)FARTitl))
        {
          hFarWnd=hwnd;
          xf_free(lpwszTitle);
          return(FALSE);
        }
      }
      xf_free(lpwszTitle);
    }
  }
  return(TRUE);
}

void FindFarWndByTitle()
{
  string strOldTitle;
  string strNewTitle;

  apiGetConsoleTitle(strOldTitle);

  strNewTitle.Format (L"%d - %s",clock(),(const wchar_t*)strOldTitle);

  SetConsoleTitleW (strNewTitle);
    //hFarWnd = FindWindow(NULL,NewTitle);

  EnumWindows(IsWindowedEnumProc2,(LPARAM)(const wchar_t*)strNewTitle);
  SetConsoleTitleW (strOldTitle);
}

void InitDetectWindowedMode()
{
  typedef HWND WINAPI GetConsoleWindow_t(VOID);
  static GetConsoleWindow_t *GetConsoleWindow_f=(GetConsoleWindow_t*)GetProcAddress(GetModuleHandleW(L"KERNEL32.DLL"),"GetConsoleWindow");
  if(GetConsoleWindow_f)
    hFarWnd=GetConsoleWindow_f();
  else
  {
    // попытка найти окно по pid
    EnumWindows(IsWindowedEnumProc,(LPARAM)GetCurrentProcessId());
    if(!hFarWnd)
      // Если не нашли ФАР по pid, то ищем по уникальному заголовку окна
      FindFarWndByTitle();
  }

  if (hFarWnd && Opt.SmallIcon)
  {
    string strFarName;
    apiGetModuleFileName (NULL, strFarName);
    HICON hSmallIcon=NULL,hLargeIcon=NULL;
    ExtractIconExW(strFarName,0,&hLargeIcon,&hSmallIcon,1);

    if (hLargeIcon!=NULL)
      hOldLargeIcon=(HICON)SendMessageW(hFarWnd,WM_SETICON,1,(LPARAM)hLargeIcon);
    if (hSmallIcon!=NULL)
      hOldSmallIcon=(HICON)SendMessageW(hFarWnd,WM_SETICON,0,(LPARAM)hSmallIcon);
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
      SendMessageW(hFarWnd,WM_SETICON,1,(LPARAM)hOldLargeIcon);
      SendMessageW(hFarWnd,WM_SETICON,0,(LPARAM)(hOldSmallIcon!=NULL ? hOldSmallIcon:hOldLargeIcon));
    }
  }
}

/* $ 25.07.2000 SVS
   Программое переключение FulScreen <-> Windowed
   (с подачи "Vasily V. Moshninov" <vmoshninov@newmail.ru>)
   mode = -2 - получить текущее состояние
          -1 - как тригер
           0 - Windowed
           1 - FulScreen
   Return
           0 - Windowed
           1 - FulScreen
*/
int FarAltEnter(int mode)
{
  if(mode != FAR_CONSOLE_GET_MODE)
  {
    if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
    {
      COORD dwOldMode;
      if(!pfnSetConsoleDisplayMode)
      {
        HMODULE hKernel32 = GetModuleHandleW(L"KERNEL32.DLL");
        pfnSetConsoleDisplayMode = (PROCSETCONSOLEDISPLAYMODEELLWND)GetProcAddress(hKernel32,"SetConsoleDisplayMode");
        //pfnGetConsoleDisplayMode = (PROCGETCONSOLEDISPLAYMODE)GetProcAddress(hKernel32,"GetConsoleDisplayMode");
      }
      pfnSetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE),
           (mode == FAR_CONSOLE_TRIGGER)?(IsWindowed()?FAR_CONSOLE_SET_FULLSCREEN:FAR_CONSOLE_SET_WINDOWED):(mode&1),&dwOldMode);
    }
    else if (hFarWnd) // win9x
    {
      //Windows9X посылает сообщение WM_COMMAND со специальным идентификатором,
      //когда пользователь нажимает ALT+ENTER:
      #define ID_SWITCH_CONSOLEMODE 0xE00F
      SendMessageW(hFarWnd,WM_COMMAND,ID_SWITCH_CONSOLEMODE,
           (mode == FAR_CONSOLE_TRIGGER)?(IsWindowed()?FAR_CONSOLE_SET_FULLSCREEN:FAR_CONSOLE_SET_WINDOWED):(mode&1));
    }
  }
  DetectWindowedMode();
  return IsWindowed()?FAR_CONSOLE_WINDOWED:FAR_CONSOLE_FULLSCREEN;
}
