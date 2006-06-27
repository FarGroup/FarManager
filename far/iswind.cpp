/*
iswind.cpp

�������� fullscreen/windowed

*/

/* Revision: 1.11 21.05.2006 $ */

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
static PROCSETCONSOLEDISPLAYMODEELLWND SetConsoleDisplayMode=NULL;
static PROCGETCONSOLEDISPLAYMODE GetConsoleDisplayMode=NULL;

void DetectWindowedMode()
{
  if (hFarWnd)
    WindowedMode=!IsIconic(hFarWnd);
}

BOOL CALLBACK IsWindowedEnumProc2(HWND hwnd,LPARAM FARTitl)
{
  wchar_t Title[256]; //BUGBUG, dynamic

  int LenTitle=GetWindowTextW (hwnd, Title, sizeof(Title));

  if (LenTitle)
  {
    Title[LenTitle]=0;
    if(wcsstr(Title,(const wchar_t *)FARTitl))
    {
      hFarWnd=hwnd;
      return(FALSE);
    }
  }
  return(TRUE);
}

/* $ 19.01.2001 VVM
    + ���� �� ����� ��� �� pid, �� ���� �� ����������� ��������� ���� */
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
} /* void FindFarWndByTitle */
/* VVM $ */

void InitDetectWindowedMode()
{
  /* $ 17.01.2003 IS
       ���, ��� �����, ���������� ��� ������ ���� ��������������� ������� ��
  */
  typedef HWND WINAPI GetConsoleWindow_t(VOID);
  static GetConsoleWindow_t *GetConsoleWindow_f=(GetConsoleWindow_t*)GetProcAddress(GetModuleHandle("kernel32.dll"),"GetConsoleWindow");
  if(GetConsoleWindow_f)
    hFarWnd=GetConsoleWindow_f();
  else
  {
    // ������� ����� ���� �� pid
    EnumWindows(IsWindowedEnumProc,(LPARAM)GetCurrentProcessId());
    if(!hFarWnd)
      /* $ 19.01.2001 VVM
         + ���� �� ����� ��� �� pid, �� ���� �� ����������� ��������� ���� */

      FindFarWndByTitle();
      /* VVM $ */
  }
  /* IS $ */
  if (hFarWnd && Opt.SmallIcon)
  {
    string strFarName;
    apiGetModuleFileName (NULL, strFarName);
    HICON hSmallIcon=NULL,hLargeIcon=NULL;
    ExtractIconExW(strFarName,0,&hLargeIcon,&hSmallIcon,1);

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

/* $ 25.07.2000 SVS
   ���������� ������������ FulScreen <-> Windowed
   (� ������ "Vasily V. Moshninov" <vmoshninov@newmail.ru>)
   mode = -2 - �������� ������� ���������
          -1 - ��� ������
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
      if(!SetConsoleDisplayMode)
      {
        HMODULE hKernel32 = GetModuleHandle("kernel32");
        SetConsoleDisplayMode = (PROCSETCONSOLEDISPLAYMODEELLWND)GetProcAddress(hKernel32,"SetConsoleDisplayMode");
        //GetConsoleDisplayMode = (PROCGETCONSOLEDISPLAYMODE)GetProcAddress(hKernel32,"GetConsoleDisplayMode");
      }
      SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE),
           (mode == FAR_CONSOLE_TRIGGER)?(IsWindowed()?FAR_CONSOLE_SET_FULLSCREEN:FAR_CONSOLE_SET_WINDOWED):(mode&1),&dwOldMode);
    }
    else if (hFarWnd) // win9x
    {
      //Windows9X �������� ��������� WM_COMMAND �� ����������� ���������������,
      //����� ������������ �������� ALT+ENTER:
      #define ID_SWITCH_CONSOLEMODE 0xE00F
      SendMessage(hFarWnd,WM_COMMAND,ID_SWITCH_CONSOLEMODE,
           (mode == FAR_CONSOLE_TRIGGER)?(IsWindowed()?FAR_CONSOLE_SET_FULLSCREEN:FAR_CONSOLE_SET_WINDOWED):(mode&1));
    }
  }
  DetectWindowedMode();
  return IsWindowed()?FAR_CONSOLE_WINDOWED:FAR_CONSOLE_FULLSCREEN;
}
/* SVS $*/
