/*
iswind.cpp

Проверка fullscreen/windowed

*/

/* Revision: 1.09 17.01.2003 $ */

/*
Modify:
  17.01.2003 IS
    + InitDetectWindowedMode: там, где можно, используем для поиска окна
      соответствующую функцию ОС (GetConsoleWindow)
  10.06.2002 SVS
    - Заголовок окна пустой - hFarWnd равен NULL
  25.03.2002 SVS
    ! Вместо чисел - константы FAR_CONSOLE_*
  17.01.2002 SVS
    - какой хрен меня понес... нужно было GetWindowText() юзать!
  16.01.2002 SVS
    ! Немного другая математика поиска заголовка (ищем как часть заголовка)
      Почему? К Сергею Обломову UIN: 12411939 (есть у него интересная фишка,
      не плагин!)
  06.05.2001 DJ
    ! перетрях #include
  19.01.2001 VVM
    + Если не нашли ФАР по pid, то ищем по уникальному заголовку окна
      Такое бывает, если ФАР запущен из под ФАР-а или другой консольной
      программы
  20.09.2000 SVS
    ! hFarWnd глобальна
  25.07.2000 SVS
    + Программое переключение FulScreen <-> Windowed
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "plugin.hpp"
#include "global.hpp"

static BOOL CALLBACK IsWindowedEnumProc(HWND hwnd,LPARAM lParam);

HWND hFarWnd;
static BOOL WindowedMode=FALSE;
static HICON hOldLargeIcon,hOldSmallIcon;

typedef BOOL (WINAPI *PROCSETCONSOLEDISPLAYMODEELLWND)(HANDLE,DWORD,LPDWORD);
typedef BOOL (WINAPI *PROCGETCONSOLEDISPLAYMODE)(LPDWORD);
static PROCSETCONSOLEDISPLAYMODEELLWND SetConsoleDisplayMode=NULL;
static PROCGETCONSOLEDISPLAYMODE GetConsoleDisplayMode=NULL;

void DetectWindowedMode()
{
  if (hFarWnd)
    WindowedMode=!IsIconic(hFarWnd);
}

BOOL CALLBACK IsWindowedEnumProc2(HWND hwnd,LPARAM FARTitl)
{
  char Title[256];
  int LenTitle=GetWindowText(hwnd,Title, sizeof(Title));
  if (LenTitle)
  {
    Title[LenTitle]=0;
    if(strstr(Title,(char *)FARTitl))
    {
      hFarWnd=hwnd;
      return(FALSE);
    }
  }
  return(TRUE);
}

/* $ 19.01.2001 VVM
    + Если не нашли ФАР по pid, то ищем по уникальному заголовку окна */
void FindFarWndByTitle()
{
  char OldTitle[256];
  char NewTitle[256];

  OldTitle[0]=0;

  GetConsoleTitle(OldTitle, sizeof(OldTitle));

  {
    sprintf(NewTitle,"%d - %s",clock(),OldTitle);
    SetConsoleTitle(NewTitle);
    //hFarWnd = FindWindow(NULL,NewTitle);
    EnumWindows(IsWindowedEnumProc2,(LPARAM)NewTitle);
    SetConsoleTitle(OldTitle);
  } /* if */
} /* void FindFarWndByTitle */
/* VVM $ */

void InitDetectWindowedMode()
{
  /* $ 17.01.2003 IS
       Там, где можно, используем для поиска окна соответствующую функцию ОС
  */
  typedef HWND WINAPI GetConsoleWindow_t(VOID);
  static GetConsoleWindow_t *GetConsoleWindow_f=
    (GetConsoleWindow_t*)GetProcAddress(GetModuleHandle("kernel32.dll"),
    "GetConsoleWindow");
  if(GetConsoleWindow_f)
    hFarWnd=GetConsoleWindow_f();
  else
  {
    // попытка найти окно по pid
    EnumWindows(IsWindowedEnumProc,(LPARAM)GetCurrentProcessId());
    if(!hFarWnd)
      /* $ 19.01.2001 VVM
         + Если не нашли ФАР по pid, то ищем по уникальному заголовку окна */

      FindFarWndByTitle();
      /* VVM $ */
  }
  /* IS $ */
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
      DWORD dwOldMode;
      if(!SetConsoleDisplayMode)
      {
        HMODULE hKernel32 = GetModuleHandle("kernel32");
        SetConsoleDisplayMode = (PROCSETCONSOLEDISPLAYMODEELLWND)GetProcAddress(hKernel32,"SetConsoleDisplayMode");
        GetConsoleDisplayMode = (PROCGETCONSOLEDISPLAYMODE)GetProcAddress(hKernel32,"GetConsoleDisplayMode");
      }
      SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE),
           (mode == FAR_CONSOLE_TRIGGER)?(IsWindowed()?FAR_CONSOLE_SET_FULLSCREEN:FAR_CONSOLE_SET_WINDOWED):(mode&1),&dwOldMode);
    }
    else if (hFarWnd) // win9x
    {
      //Windows9X посылает сообщение WM_COMMAND со специальным идентификатором,
      //когда пользователь нажимает ALT+ENTER:
      #define ID_SWITCH_CONSOLEMODE 0xE00F
      SendMessage(hFarWnd,WM_COMMAND,ID_SWITCH_CONSOLEMODE,
           (mode == FAR_CONSOLE_TRIGGER)?(IsWindowed()?FAR_CONSOLE_SET_FULLSCREEN:FAR_CONSOLE_SET_WINDOWED):(mode&1));
    }
  }
  DetectWindowedMode();
  return IsWindowed()?FAR_CONSOLE_WINDOWED:FAR_CONSOLE_FULLSCREEN;
}
/* SVS $*/
