/*
interf.cpp

Консольные функции ввода-вывода

*/

/* Revision: 1.50 22.03.2002 $ */

/*
Modify:
  22.03.2002 SVS
    ! Масдай - отстой, Alt-F9 ему недоступно!!!
  04.03.2002 DJ
    ! Appli -> Apply
  03.03.2002 SVS
    ! Если для VC вставить ключ /Gr, то видим кучу багов :-/
  03.03.2002 SVS
    + ChangeBlockColor() - изменение цвета в блоке
    ! уточнение юникодовой таблицы
  13.02.2002 SVS
    ! Уборка варнингов
  08.01.2002 SVS
    - Бага с макросом, в котором есть Alt-F9 (смена режима)
    ! Обработаем коды символов 0x10 и 0x11
  26.12.2001 SVS
    ! При закрытии окна "по кресту"... теперь настраиваемо!
  24.12.2001 VVM
    ! При ожидании окончания регистрации отдаем время виндам...
  10.12.2001 SVS
    ! SetFarTitle() - уточнение для макросов (в режиме исполнения если стоит
      флаг MFLAGS_DISABLEOUTPUT не выставлять заголовок консоли)
  23.10.2001 SVS
    + WidthNameForMessage - 38% для размера усечения имени в месагах-процессах
  21.10.2001 SVS
    + PrevScrX,PrevScrY - предыдущие размеры консоли (для позиционирования
      диалогов)
  04.10.2001 SVS
    ! В SetFarTitle сначала сравним старое значение заголовка консоли,
      а уж потом выставим значение... Инача получаем моргание (при
      перетаскивании диалога, например)
  27.09.2001 IS
    - Левый размер при использовании strncpy
  30.08.2001 IS
    - Утечка ресурсов. Теперь при закрытии окна "по кресту" всегда говорим
      системе, что "разберемся сами", в противном случае Фар будет закрыт
      системой принудительно и не будут выполнены обычные при закрытии
      процедуры: оповещение плагинов, вызов деструкторов, сохранение настроек
      и т.п.
  25.07.2001 SVS
    - Бага с детачем - лишний раз инициализировали исходные значения
    ! Немного оптимизации в функциях отрисовки
  24.07.2001 SVS
    ! Немного оптимизации в функциях отрисовки
  06.07.2001 SKV
    - KeyQueue=NULL; в CloseConsole();
  04.07.2001 SVS
    ! BoxText может рисовать вертикальный сепаратор
  25.06.2001 IS
    ! Внедрение const
  08.06.2001 SVS
    + GenerateWINDOW_BUFFER_SIZE_EVENT()
  07.06.2001 SVS
    + "цветные часы" :-)
  06.06.2001 SVS
    ! W-функции юзаем пока только в режиме USE_WFUNC
  06.06.2001 SVS
    ! Добавлена перекодировочная таблица OEM->UNICODE
  23.05.2001 OT
    + Добавление опции "AltF9 - по старому"
  23.05.2001 SVS
    ! немного увеличим размеры локальных буферов для отрисовки (по мотивам
      Alt-F9)
  22.05.2001 tran
    ! по результам прогона на CodeGuard
  21.05.2001 OT
    - Исправление поведения AltF9
  06.05.2001 DJ
    ! перетрях #include
  06.05.2001 ОТ
    ! Переименование Window в Frame :)
  05.05.2001 DJ
    + перетрях NWZ
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  06.03.2001 SVS
   ! SetScreen() - размер строки 256 заменен на 4096 во избежании проблем
     в будущем (new или malloc делать не стоит, т.к. функция часто вызывается)
  27.02.2001 SVS
   + BoxText(Char) - вывод одного символа
   ! В MakeSeparator добавлена отрисовка двойной разделительной линии (Type=3)
  20.02.2001 SVS
   ! ShowSeparator - дополнительный параметр - тип сепаратора
   + MakeSeparator - создание разделителя в памяти
   ! Символы, зависимые от кодовой страницы
     /[\x01-\x08\x0B-\x0C\x0E-\x1F\xB0-\xDF\xF8-\xFF]/
     переведены в коды.
  14.02.2001 SKV
    ! параметр setpal в InitConsole.
  02.02.2001 VVM
    ! Переделал функции Text(...). Т.к. они вызываются очень часто,
      то основной вывод будет идти в Text(char *Str)
  24.01.2001 SVS
    + Инициализация клавиатурной очереди KeyQueue,
      размером  = 1024 кода клавиши
  22.01.2001 SVS
    ! Для FS & WM применяются 2 разных курсора - их параметры можно
      задавать через реестр.
  22.01.2001 SVS
    ! Проблемы с курсором при больших разрешениях - ставим тот курсор в
      SetCursorType, который был до запуска ФАРа.
  03.01.2001 SVS
    + Функции SetFarTitle, ScrollBar, ShowSeparator
      переехали из mix.cpp
  22.12.2000 SVS
    ! Перетасовка исходников
      IsMouseButtonPressed -> keyboard.cpp
      GetInputRecord -> keyboard.cpp
      CheckForEsc -> keyboard.cpp
      WriteInput -> keyboard.cpp
      WaitKey -> keyboard.cpp
      PeekInputRecord -> keyboard.cpp
      CalcKeyCode -> keyboard.cpp
    ! hConOut,hConInp - public статус -> global.cpp
  08.09.2000 SVS
    + KEY_CTRLSHIFTDEL, KEY_ALTSHIFTDEL в функции CalcKeyCode
  24.08.2000 SVS
    + Пераметр у фунции WaitKey - возможность ожидать конкретную клавишу
    + Добавление на реакцию KEY_CTRLALTSHIFTPRESS & KEY_CTRLALTSHIFTRELEASE
  23.08.2000 SVS
    + Код для средней клавиши мыши :-) (ну есть у меня дома эта хрень...)
  09.08.2000 KM
    ! Изменена MakeShadow таким образом, что теперь при достижении
      окном (диалогом) правой или нижней границы экрана и при выходе за неё
      тень продолжает рисоваться, что при нынешней идеологии движения окон
      более правильно.
  23.07.2000 SVS
    ! Немного оптимизации в функциях Box(), HiText() :-)
    + Клавиши (CalcKeyCode):
       Ctrl- Shift- Alt- CtrlShift- AltShift- CtrlAlt- Apps :-)
       KEY_LWIN (VK_LWIN), KEY_RWIN (VK_RWIN)
    + Text(int X, int Y, int Color, char *Str);
    + Text(int X, int Y, int Color, int MsgId);
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  06.07.2000 SVS
    - Временная отмена патча 11 (NT Console resize bug) до лучших времен :-)
  28.06.2000 tran
    - NT Console resize bug
      adding resize support in GetInputRecord for modal window like
      Viewer, Editor, Help
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "farqueue.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "keys.hpp"
#include "colors.hpp"
#include "plugin.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "editor.hpp"
#include "manager.hpp"
#include "scrbuf.hpp"


BOOL __stdcall CtrlHandler(DWORD CtrlType);
static void InitRecodeOutTable();

static int CurX,CurY;
static int CurColor;

static int OutputCP;
static BYTE RecodeOutTable[256];
static int InitCurVisible,InitCurSize;

#if defined(USE_WFUNC)
WCHAR Oem2Unicode[256] = {
/*00*/ 0x0000, 0x263A, 0x263B, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022,
       0x25D8, 0x25CB, 0x25D9, 0x2642, 0x2640, 0x266A, 0x266B, 0x263C,
/*10*/ 0x25BA, 0x25C4, 0x2195, 0x203C, 0x00B6, 0x00A7, 0x25A0, 0x21A8,
       0x2191, 0x2193, 0x2192, 0x2190, 0x221F, 0x2194, 0x25B2, 0x25BC,
/*20*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
       0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*30*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
       0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*40*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
       0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*50*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
       0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*60*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
       0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*70*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
       0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x2206,
/*80*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
       0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*90*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
       0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*A0*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
       0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*B0*/ 0x0000, 0x0000, 0x0000, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
       0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
/*C0*/ 0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
       0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
/*D0*/ 0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
       0x256A, 0x2518, 0x250C, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*E0*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
       0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*F0*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
       0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};
#endif

void InitConsole(int FirstInit)
{
  OutputCP=GetConsoleOutputCP();
  InitRecodeOutTable();
  hConOut=CreateFile("CONOUT$",GENERIC_READ|GENERIC_WRITE,
          FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  ScrBuf.SetHandle(hConOut);
  hConInp=CreateFile("CONIN$",GENERIC_READ|GENERIC_WRITE,
          FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  SetConsoleCtrlHandler(CtrlHandler,TRUE);
  GetConsoleMode(hConInp,&InitialConsoleMode);
  GetRealCursorType(InitCurVisible,InitCurSize);
  GetRegKey("Interface","Mouse",Opt.Mouse,1);

  // размер клавиатурной очереди = 1024 кода клавиши
  if(!KeyQueue)
    KeyQueue=new FarQueue<DWORD>(1024);

  SetFarConsoleMode();
  SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
  if(FirstInit)
    GetVideoMode(InitScreenBufferInfo);
  GetVideoMode(CurScreenBufferInfo);
  ScrBuf.FillBuf();
  // было sizeof(Palette)
  /*$ 14.02.2001 SKV
    для consoledetach не нужно, что бы инитилась палитра.
  */
  if(FirstInit)
    memcpy(Palette,DefaultPalette,SizeArrayPalette);
  /* SKV$*/
}


void ReopenConsole()
{
  HANDLE hOldOut=hConOut;
  hConOut=CreateFile("CONOUT$",GENERIC_READ|GENERIC_WRITE,
          FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  ScrBuf.SetHandle(hConOut);
  SetStdHandle(STD_OUTPUT_HANDLE,hConOut);
  CloseHandle(hOldOut);
}


void CloseConsole()
{
  ScrBuf.Flush();
  SetRealCursorType(InitCurVisible,InitCurSize);
  ChangeConsoleMode(InitialConsoleMode);
  CloseHandle(hConInp);
  CloseHandle(hConOut);
  /* $ 22.05.2001 tran
     codeguard says... */
  delete KeyQueue;
  /* tran $ */
  /*$ 27.06.2001 SKV
    ... а обNULLить?
  */
  KeyQueue=NULL;
  /* SKV$*/
}


void SetFarConsoleMode()
{
  int Mode=ENABLE_WINDOW_INPUT;
  if (Opt.Mouse)
    Mode|=ENABLE_MOUSE_INPUT;
  ChangeConsoleMode(Mode);
}


void InitRecodeOutTable()
{
  int I;
  for (I=0;I<sizeof(RecodeOutTable)/sizeof(RecodeOutTable[0]);I++)
    RecodeOutTable[I]=I;
  if (Opt.CleanAscii)
  {
    for (I=0;I<32;I++)
      RecodeOutTable[I]='.';
    RecodeOutTable[7]='*';
    RecodeOutTable[24]=RecodeOutTable[25]='|';
    RecodeOutTable[30]='X';
    RecodeOutTable[31]='X';
    RecodeOutTable[255]=' ';
    RecodeOutTable[0x10]='>';
    RecodeOutTable[0x11]='<';
  }
  if (Opt.NoGraphics)
  {
    for (I=179;I<=218;I++)
      RecodeOutTable[I]='+';
    RecodeOutTable[179]=RecodeOutTable[186]='|';
    RecodeOutTable[196]='-';
    RecodeOutTable[205]='=';
  }
#if defined(USE_WFUNC)
  if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
  {
    for (I=0;I<sizeof(RecodeOutTable)/sizeof(RecodeOutTable[0]);I++)
    {
      if(!Oem2Unicode[I])
      {
        CHAR Chr = (CHAR)RecodeOutTable[I];
        MultiByteToWideChar(CP_OEMCP, 0, &Chr, 1, Oem2Unicode+I, 1);
      }
    }
  }
#endif
}


void ChangeConsoleMode(int Mode)
{
  DWORD CurrentConsoleMode;
  GetConsoleMode(hConInp,&CurrentConsoleMode);
  if (CurrentConsoleMode!=(DWORD)Mode)
    SetConsoleMode(hConInp,Mode);
}


void FlushInputBuffer()
{
  FlushConsoleInputBuffer(hConInp);
}

//OT
int operator==(CONSOLE_SCREEN_BUFFER_INFO &csbi1,CONSOLE_SCREEN_BUFFER_INFO &csbi2)
{
  return csbi1.dwSize.X==csbi2.dwSize.X && csbi1.dwSize.Y==csbi2.dwSize.Y;
}

#define _OT_ConsoleInfo(_hCon) \
  GetConsoleScreenBufferInfo(_hCon, &_csbi);\
  _OT(SysLog("Current Screen Buffer info[0x%p]:\n\
    dwSize           = (%3i,%3i)\n\
    dwCursorPosition = (%3i,%3i)\n\
    wAttributes        =  0x%02x\n\
    srWindow           = (%3i,%3i), (%3i,%3i)\n\
    dwMaximumWindowSize= (%3i,%3i)\n"\
    ,_hCon\
    ,_csbi.dwSize.X,_csbi.dwSize.Y\
    ,_csbi.dwCursorPosition.X,_csbi.dwCursorPosition.Y\
    ,_csbi.wAttributes\
    ,_csbi.srWindow.Left,_csbi.srWindow.Top,_csbi.srWindow.Right,_csbi.srWindow.Bottom\
    ,_csbi.dwMaximumWindowSize.X,_csbi.dwMaximumWindowSize.Y\
    ))

_OT(void ViewConsoleInfo()\
{\
  CONSOLE_SCREEN_BUFFER_INFO _csbi;\
  _OT_ConsoleInfo(hConOut);\
  _OT_ConsoleInfo(hConInp);\
})

void SetVideoMode(int ScreenMode)
{
  if (!ScreenMode && Opt.AltF9 && WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
  {
    ChangeVideoMode(InitScreenBufferInfo==CurScreenBufferInfo);
  }
  else
  {
    ChangeVideoMode(ScrY==24?50:25,80);
  }
}

void ChangeVideoMode(int Maximized)
{
  COORD coordScreen;
  if (Maximized)
  {
    SendMessage(hFarWnd,WM_SYSCOMMAND,SC_MAXIMIZE,(LPARAM)0);
    coordScreen = GetLargestConsoleWindowSize(hConOut);
  }
  else
  {
    SendMessage(hFarWnd,WM_SYSCOMMAND,SC_RESTORE,(LPARAM)0);
    coordScreen = InitScreenBufferInfo.dwSize;
  }
  ChangeVideoMode(coordScreen.Y,coordScreen.X);

}

void ChangeVideoMode(int NumLines,int NumColumns)
{
  _OT(DWORD le);
  _OT(SysLog("ChangeVideoMode(NumLines=%i, NumColumns=%i)",NumLines,NumColumns ));

  int retSetConsole;
  CONSOLE_SCREEN_BUFFER_INFO csbi; /* hold current console buffer info */
  SMALL_RECT srWindowRect; /* hold the new console size */
  COORD coordScreen;
  int xSize=NumColumns,ySize=NumLines;

  GetConsoleScreenBufferInfo(hConOut, &csbi);

  if(WinVer.dwPlatformId != VER_PLATFORM_WIN32_NT ||
      (NumColumns == 80 && (NumLines == 25 || NumLines == 50)) // обеспечим выполнение !Opt.AltF9
    )
  {
    /* get the largest size we can size the console window to */
    coordScreen = GetLargestConsoleWindowSize(hConOut);
  /* define the new console window size and scroll position */
    srWindowRect.Right = (SHORT) (Min((short)xSize, coordScreen.X) - 1);
    srWindowRect.Bottom = (SHORT) (Min((short)ySize, coordScreen.Y) - 1);
    srWindowRect.Left = srWindowRect.Top = (SHORT) 0;
    /* define the new console buffer size */
    coordScreen.X = xSize;
    coordScreen.Y = ySize;
    /* if the current buffer is larger than what we want, resize the */
    /* console window first, then the buffer */
    if ((DWORD) csbi.dwSize.X * csbi.dwSize.Y > (DWORD) xSize * ySize)
    {
      SetConsoleWindowInfo(hConOut, TRUE, &srWindowRect);
      SetConsoleScreenBufferSize(hConOut, coordScreen);
    }
    /* if the current buffer is smaller than what we want, resize the */
    /* buffer first, then the console window */
    if ((DWORD) csbi.dwSize.X * csbi.dwSize.Y < (DWORD) xSize * ySize)
    {
      SetConsoleScreenBufferSize(hConOut, coordScreen);
      SetConsoleWindowInfo(hConOut, TRUE, &srWindowRect);
    }
//    GetVideoMode();
  }
  else
  {
    srWindowRect.Right = xSize-1;
    srWindowRect.Bottom = ySize-1;
    srWindowRect.Left = srWindowRect.Top = (SHORT) 0;
    /* define the new console buffer size */
    coordScreen.X = xSize;
    coordScreen.Y = ySize;

    if (xSize>csbi.dwSize.X || ySize > csbi.dwSize.Y)
    {
      if (csbi.dwSize.X < xSize-1)
      {
        srWindowRect.Right = csbi.dwSize.X-1;
        retSetConsole=SetConsoleWindowInfo(hConOut, TRUE, &srWindowRect);
        if (!retSetConsole)
        {
          _OT(SysLog("LastError=%i, srWindowRect=(%i.%i)(%i.%i)",le=GetLastError(),srWindowRect.Left,srWindowRect.Top,srWindowRect.Right,srWindowRect.Bottom));
        }
        srWindowRect.Right = xSize-1;
      }

      if (csbi.dwSize.Y < ySize-1)
      {
        srWindowRect.Bottom=csbi.dwSize.Y-1;
        retSetConsole=SetConsoleWindowInfo(hConOut, TRUE, &srWindowRect);
        if (!retSetConsole)
        {
          _OT(SysLog("LastError=%i, srWindowRect",le=GetLastError()));
        }
        srWindowRect.Bottom = ySize-1;
      }

      retSetConsole=SetConsoleScreenBufferSize(hConOut, coordScreen);
      if (!retSetConsole)
      {
        _OT(SysLog("SetConsoleScreenBufferSize failed... coordScreen=(%i,%i)\n\
          LastError=%i",coordScreen.X,coordScreen.Y,le=GetLastError()));
      }
    }
    if ((retSetConsole=SetConsoleWindowInfo(hConOut, TRUE, &srWindowRect)) == NULL)
    {
        _OT(SysLog("LastError=%i, srWindowRect",le=GetLastError()));
      retSetConsole=SetConsoleScreenBufferSize(hConOut, coordScreen);
        _OT(le=GetLastError());
        _OT(SysLog("SetConsoleScreenBufferSize(hConOut, coordScreen),  retSetConsole=%i",retSetConsole));
      retSetConsole=SetConsoleWindowInfo(hConOut, TRUE, &srWindowRect);
        _OT(le=GetLastError());
        _OT(SysLog("SetConsoleWindowInfo(hConOut, TRUE, &srWindowRect),  retSetConsole=%i",retSetConsole));
        _OT(ViewConsoleInfo());
    }
    else
    {
      retSetConsole=SetConsoleScreenBufferSize(hConOut, coordScreen);
        _OT(le=GetLastError());
        _OT(SysLog("SetConsoleScreenBufferSize(hConOut, coordScreen),  retSetConsole=%i",retSetConsole));
    }
  }

  // зашлем эвент только в случае, когда макросы не исполняются
  if(CtrlObject && !CtrlObject->Macro.IsExecuting())
    GenerateWINDOW_BUFFER_SIZE_EVENT(NumColumns,NumLines);
}

void GenerateWINDOW_BUFFER_SIZE_EVENT(int Sx, int Sy)
{
  INPUT_RECORD Rec;
  DWORD Writes;
  CONSOLE_SCREEN_BUFFER_INFO csbi; /* hold current console buffer info */
  if(Sx==-1 || Sy==-1)
    GetConsoleScreenBufferInfo(hConOut, &csbi);

  Rec.EventType=WINDOW_BUFFER_SIZE_EVENT;
  Rec.Event.WindowBufferSizeEvent.dwSize.X=Sx==-1?csbi.dwSize.X:Sx;
  Rec.Event.WindowBufferSizeEvent.dwSize.Y=Sy==-1?csbi.dwSize.Y:Sy;
//_SVS(SysLog("[%d:%d] = [%d:%d, %d:%d]",csbi.dwSize.X,csbi.dwSize.Y,csbi.srWindow.Left,csbi.srWindow.Top,csbi.srWindow.Right,csbi.srWindow.Bottom));
  WriteConsoleInput(hConInp,&Rec,1,&Writes);
}

void GetVideoMode(CONSOLE_SCREEN_BUFFER_INFO &csbi)
{
  memset(&csbi,0,sizeof(csbi));
  GetConsoleScreenBufferInfo(hConOut,&csbi);
  ScrX=csbi.dwSize.X-1;
  ScrY=csbi.dwSize.Y-1;
  WidthNameForMessage=(ScrX*38)/100+1;
  if(PrevScrX == -1) PrevScrX=ScrX;
  if(PrevScrY == -1) PrevScrY=ScrY;
  _OT(SysLog("ScrX=%d ScrY=%d",ScrX,ScrY));
  ScrBuf.AllocBuf(csbi.dwSize.X,csbi.dwSize.Y);
  _OT(ViewConsoleInfo());
}

BOOL __stdcall CtrlHandler(DWORD CtrlType)
{
  if (CtrlType==CTRL_C_EVENT || CtrlType==CTRL_BREAK_EVENT)
  {
    if (CtrlType==CTRL_BREAK_EVENT)
      WriteInput(KEY_BREAK);
    if (CtrlObject->Cp()->LeftPanel!=NULL && CtrlObject->Cp()->LeftPanel->GetMode()==PLUGIN_PANEL)
      CtrlObject->Plugins.ProcessEvent(CtrlObject->Cp()->LeftPanel->GetPluginHandle(),FE_BREAK,(void *)CtrlType);
    if (CtrlObject->Cp()->RightPanel!=NULL && CtrlObject->Cp()->RightPanel->GetMode()==PLUGIN_PANEL)
      CtrlObject->Plugins.ProcessEvent(CtrlObject->Cp()->RightPanel->GetPluginHandle(),FE_BREAK,(void *)CtrlType);
    return(TRUE);
  }
  CloseFAR=TRUE;
  /* $ 30.08.2001 IS
     При закрытии окна "по кресту" всегда возвращаем TRUE, в противном случае
     Фар будет закрыт системой и не будут выполнены обычные при закрытии
     процедуры: оповещены плагины, вызваны деструкторы, сохранены настройки и
     т.п.
  */
  if(!Opt.CloseConsoleRule)
  {
    if (CurrentEditor!=NULL && CurrentEditor->IsFileModified() ||
        FrameManager->IsAnyFrameModified (FALSE))
      return(TRUE);
    return(FALSE);
  }
  return TRUE;
  /* IS $ */
}


void GotoXY(int X,int Y)
{
  if (X<0) X=0;
  if (Y<0) Y=0;
  CurX=X;
  CurY=Y;
}


int WhereX()
{
  return(CurX);
}


int WhereY()
{
  return(CurY);
}


void MoveCursor(int X,int Y)
{
  ScrBuf.MoveCursor(X,Y);
}


void GetCursorPos(int& X,int& Y)
{
  ScrBuf.GetCursorPos(X,Y);
}


void SetCursorType(int Visible,int Size)
{
  if (Size==-1 || !Visible)
    Size=IsWindowed()?
       (Opt.CursorSize[0]?Opt.CursorSize[0]:InitCurSize):
       (Opt.CursorSize[1]?Opt.CursorSize[1]:InitCurSize);

  ScrBuf.SetCursorType(Visible,Size);
}


void GetCursorType(int &Visible,int &Size)
{
  ScrBuf.GetCursorType(Visible,Size);
}


void MoveRealCursor(int X,int Y)
{
  COORD C;
  C.X=X;
  C.Y=Y;
  SetConsoleCursorPosition(hConOut,C);
}


void GetRealCursorPos(int& X,int& Y)
{
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(hConOut,&csbi);
  X=csbi.dwCursorPosition.X;
  Y=csbi.dwCursorPosition.Y;
}


void SetRealCursorType(int Visible,int Size)
{
  CONSOLE_CURSOR_INFO cci;
  cci.dwSize=Size;
  cci.bVisible=Visible;
  SetConsoleCursorInfo(hConOut,&cci);
}


void GetRealCursorType(int &Visible,int &Size)
{
  CONSOLE_CURSOR_INFO cci;
  GetConsoleCursorInfo(hConOut,&cci);
  Size=cci.dwSize;
  Visible=cci.bVisible;
}

/* $ 23.07.2000 SVS
   + две полных функции Text
*/
/* $ 02.02.2001 VVM
    ! Переделал функции Text(...). Т.к. они вызываются очень часто,
      то основной вывод будет идти в Text(char *Str) */
void Text(int X, int Y, int Color, const char *Str)
{
  CurColor=FarColorToReal(Color);
  if (X<0) X=0;
  if (Y<0) Y=0;
  CurX=X;
  CurY=Y;
  Text(Str);
}

void Text(const char *Str)
{
  int Length=strlen(Str);
  if (CurX+Length>ScrX)
    Length=ScrX-CurX+1;
  if (Length<=0)
    return;
  CHAR_INFO CharBuf[1024];
  for (int I=0;I<Length;I++)
  {
#if defined(USE_WFUNC)
    if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
      CharBuf[I].Char.UnicodeChar = Oem2Unicode[RecodeOutTable[static_cast<BYTE>(Str[I])];
    else
      CharBuf[I].Char.AsciiChar=RecodeOutTable[static_cast<BYTE>(Str[I])];
#else
    CharBuf[I].Char.AsciiChar=RecodeOutTable[static_cast<BYTE>(Str[I])];
#endif
    CharBuf[I].Attributes=CurColor;
  }
  ScrBuf.Write(CurX,CurY,CharBuf,Length);
  CurX+=Length;
}

void Text(int X, int Y, int Color,int MsgId)
{
  Text(X,Y,Color,MSG(MsgId));
}

void Text(int MsgId)
{
  Text(MSG(MsgId));
}
/* VVM $ */
/* SVS $ */

void VText(const char *Str)
{
  int Length=strlen(Str);
  if (CurY+Length>ScrY)
    Length=ScrY-CurY+1;
  if (Length<=0)
    return;
  int StartCurX=CurX;
  char ChrStr[2]={0,0};
  for (int I=0;I<Length;I++)
  {
    GotoXY(CurX,CurY);
    ChrStr[0]=Str[I];
    Text(ChrStr);
    CurY++;
    CurX=StartCurX;
  }
}


void HiText(const char *Str,int HiColor)
{
  char TextStr[300];
  int SaveColor;
  char *ChPtr;
  strncpy(TextStr,Str,sizeof(TextStr)-1);
  if ((ChPtr=strchr(TextStr,'&'))==NULL)
    Text(TextStr);
  else
  {
    *ChPtr=0;
    Text(TextStr);
    if (ChPtr[1])
    {
      /* $ 23.07.2000 SVS
         Немного оптимизации :-)
      */
      char Chr[2];
      SaveColor=CurColor;
      SetColor(HiColor);
      Chr[0]=ChPtr[1]; Chr[1]=0;
      Text(Chr);
      SetColor(SaveColor);
      Text(ChPtr+2);
      /* SVS $ */
    }
  }
}


void SetScreen(int X1,int Y1,int X2,int Y2,int Ch,int Color)
{
  if (X1<0) X1=0;
  if (Y1<0) Y1=0;
  if (X2>ScrX) X2=ScrX;
  if (Y2>ScrY) Y2=ScrY;

  ScrBuf.FillRect(X1,Y1,X2,Y2,RecodeOutTable[Ch],FarColorToReal(Color));
//  SetColor(Color);
//  GotoXY(X2,Y2);
}


void MakeShadow(int X1,int Y1,int X2,int Y2)
{
  if (X1<0) X1=0;
  if (Y1<0) Y1=0;
  if (X2>ScrX) X2=ScrX;
  if (Y2>ScrY) Y2=ScrY;

  ScrBuf.ApplyColorMask(X1,Y1,X2,Y2,0xF8);
}

void ChangeBlockColor(int X1,int Y1,int X2,int Y2,int Color)
{
  if (X1<0) X1=0;
  if (Y1<0) Y1=0;
  if (X2>ScrX) X2=ScrX;
  if (Y2>ScrY) Y2=ScrY;

  ScrBuf.ApplyColor(X1,Y1,X2,Y2,FarColorToReal(Color));
}


void mprintf(char *fmt,...)
{
  va_list argptr;
  va_start(argptr,fmt);
  char OutStr[2048];
  vsprintf(OutStr,fmt,argptr);
  Text(OutStr);
  va_end(argptr);
}

void mprintf(int MsgId,...)
{
  va_list argptr;
  va_start(argptr,MsgId);
  char OutStr[2048];
  vsprintf(OutStr,MSG(MsgId),argptr);
  Text(OutStr);
  va_end(argptr);
}

void vmprintf(char *fmt,...)
{
  va_list argptr;
  va_start(argptr,fmt);
  char OutStr[2048];
  vsprintf(OutStr,fmt,argptr);
  VText(OutStr);
  va_end(argptr);
}

void SetColor(int Color)
{
  CurColor=FarColorToReal(Color);
}


int GetColor()
{
  return(CurColor);
}


void ScrollScreen(int Count)
{
  ScrBuf.Scroll(Count);
  ScrBuf.FillRect(0,ScrY+1-Count,ScrX,ScrY,' ',FarColorToReal(F_LIGHTGRAY|B_BLACK));
}


void GetText(int X1,int Y1,int X2,int Y2,void *Dest)
{
  ScrBuf.Read(X1,Y1,X2,Y2,(CHAR_INFO *)Dest);
}


void PutText(int X1,int Y1,int X2,int Y2,void *Src)
{
  int Width=X2-X1+1;
  int I,Y;
#if 1
  CHAR_INFO *SrcPtr=(CHAR_INFO*)Src;

  for (Y=Y1;Y<=Y2;++Y,SrcPtr+=Width)
    ScrBuf.Write(X1,Y,SrcPtr,Width);
#else
  for (I=0,Y=Y1;I<Y2-Y1+1;I++,++Y)
    ScrBuf.Write(X1,Y,((PCHAR_INFO)Src)+Width*I,Width);
#endif
}


void ShowTime(int ShowAlways)
{
  char ClockText[10];
  static SYSTEMTIME lasttm={0,0,0,0,0,0,0,0};
  SYSTEMTIME tm;
  GetLocalTime(&tm);
  CHAR_INFO ScreenClockText[5];
  GetText(ScrX-4,0,ScrX,0,ScreenClockText);
  if (ShowAlways==2)
  {
    memset(&lasttm,0,sizeof(lasttm));
    return;
  }

  if (!ShowAlways && lasttm.wMinute==tm.wMinute && lasttm.wHour==tm.wHour &&
      GetVidChar(ScreenClockText[2])==':' || ScreenSaverActive)
    return;

  lasttm=tm;
  sprintf(ClockText,"%02d:%02d",tm.wHour,tm.wMinute);
  GotoXY(ScrX-4,0);
  // Здесь хрень какая-то получается с ModType - все время не верное значение!
  int ModType=FrameManager->GetCurrentFrame()->GetType();
  SetColor(ModType==MODALTYPE_VIEWER?COL_VIEWERCLOCK:
           (ModType==MODALTYPE_EDITOR?COL_EDITORCLOCK:COL_CLOCK));
  Text(ClockText);
  ScrBuf.Flush();
  static int RegChecked=FALSE;
  if (!RegChecked && clock()>10000)
  {
    RegChecked=TRUE;
    struct RegInfo Reg;
    Reg.Done=0;
    _beginthread(CheckReg,0x10000,&Reg);
    while (!Reg.Done)
      Sleep(10);
    if (*Reg.RegName)
    {
      unsigned char Add=158,Xor1=211;
      for (int I=0;Reg.RegName[I];I++)
      {
        Add+=Reg.RegName[I];
        Xor1^=(RegName[I]<<I)^I;
      }
      if ((Add & 0xf)!=ToHex(Reg.RegCode[1]) || ((Add>>3) & 0xf)!=ToHex(Reg.RegCode[2]))
      {
        void *ErrRegFnPtr=((char *)ErrRegFn-111);
        _beginthread((void (__cdecl *)(void *))((char *)ErrRegFnPtr+111),0x10000,NULL);
      }
      if (RegVer!=3 && (((Xor1>>5) & 0xf)!=ToHex(Reg.RegCode[4]) || (~Xor1 & 0xf)!=ToHex(Reg.RegCode[6])))
      {
        void *ErrRegFnPtr=((char *)ErrRegFn-50);
        _beginthread((void (__cdecl *)(void *))((char *)ErrRegFnPtr+50),0x10000,NULL);
      }
    }
  }
}

void BoxText(unsigned char Chr)
{
  char Str[2];
  Str[0]=Chr;
  Str[1]=0;
  BoxText(Str);
}

void BoxText(char *Str,int IsVert)
{
  if (OutputCP!=437 && OutputCP!=866)
    for (int I=0;Str[I]!=0;I++)
      switch(Str[I])
      {
        case 199:
        case 182:
          Str[I]=186;
          break;
        case 207:
        case 209:
          Str[I]=205;
          break;
      }
  if(IsVert)
    VText(Str);
  else
    Text(Str);
}

/*
   Отрисовка прямоугольника.
*/
/* $ 23.07.2000 SVS
   Немного оптимизации :-)
*/
void Box(int x1,int y1,int x2,int y2,int Color,int Type)
{
  char OutStr[4096];
  static char ChrBox[2][6]={
    {0xC4,0xB3,0xDA,0xC0,0xD9,0xBF},
    {0xCD,0xBA,0xC9,0xC8,0xBC,0xBB},
  };

  if (x1>=x2 || y1>=y2)
    return;

  SetColor(Color);
  Type=(Type==SINGLE_BOX || Type==SHORT_SINGLE_BOX)?0:1;

  memset(OutStr,ChrBox[Type][0],sizeof(OutStr));
  OutStr[x2-x1]=0;

  OutStr[0]=ChrBox[Type][2];
  OutStr[x2-x1]=ChrBox[Type][5];
  GotoXY(x1,y1);
  mprintf("%.*s",x2-x1+1,OutStr);

  OutStr[0]=ChrBox[Type][3];
  OutStr[x2-x1]=ChrBox[Type][4];
  GotoXY(x1,y2);
  mprintf("%.*s",x2-x1+1,OutStr);

  memset(OutStr,ChrBox[Type][1],sizeof(OutStr));
  OutStr[y2-y1]=0;

  GotoXY(x1,y1+1);
  vmprintf("%.*s",y2-y1-1,OutStr);
  GotoXY(x2,y1+1);
  vmprintf("%.*s",y2-y1-1,OutStr);
}
/* SVS $ */

void PutRealText(int X1,int Y1,int X2,int Y2,void *Src)
{
  COORD Size,Corner;
  SMALL_RECT Coord;
  if (X2>ScrX) X2=ScrX;
  if (Y2>ScrY) Y2=ScrY;
  if (X1>X2) X1=X2;
  if (Y1>Y2) Y1=Y2;
  Size.X=X2-X1+1;
  Size.Y=Y2-Y1+1;
  Corner.X=0;
  Corner.Y=0;
  Coord.Left=X1;
  Coord.Top=Y1;
  Coord.Right=X2;
  Coord.Bottom=Y2;
  WriteConsoleOutput(hConOut,(PCHAR_INFO)Src,Size,Corner,&Coord);
}

void GetRealText(int X1,int Y1,int X2,int Y2,void *Dest)
{
  COORD Size,Corner;
  SMALL_RECT Coord;
  if (X2>ScrX) X2=ScrX;
  if (Y2>ScrY) Y2=ScrY;
  if (X1>X2) X1=X2;
  if (Y1>Y2) Y1=Y2;
  Size.X=X2-X1+1;
  Size.Y=Y2-Y1+1;
  Corner.X=0;
  Corner.Y=0;
  Coord.Left=X1;
  Coord.Top=Y1;
  Coord.Right=X2;
  Coord.Bottom=Y2;
  ReadConsoleOutput(hConOut,(PCHAR_INFO)Dest,Size,Corner,&Coord);
}

void SetFarTitle(const char *Title)
{
  static char FarTitle[2*NM];
  char OldFarTitle[2*NM];
  GetConsoleTitle(OldFarTitle,sizeof(OldFarTitle));
  if(Title)
  {
    sprintf(FarTitle,"%.256s - Far",Title);
    if (WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT)
      OemToChar(FarTitle,FarTitle);
    if(strcmp(OldFarTitle,FarTitle) && !CtrlObject->Macro.IsDsableOutput())
      SetConsoleTitle(FarTitle);
  }
  else
  {
    /*
      Title=NULL для случая, когда нужно выставить пред.заголовок
      SetFarTitle(NULL) - это не для всех!
      Этот вызов имеет право делать только макро-движок!
    */
    SetConsoleTitle(FarTitle);
  }
}

void ScrollBar(int X1,int Y1,int Length,unsigned long Current,unsigned long Total)
{
  /* $ 06.07.2000 tran
     - trap under NT with console height > 210
       was char OutStr[200] :) */
  char OutStr[4096];
  /* tran 06.07.2000 $ */

  int ThumbPos;
  if ((Length-=2)<1)
    return;
  if (Total>0)
    ThumbPos=Length*Current/Total;
  else
    ThumbPos=0;
  if (ThumbPos>=Length)
    ThumbPos=Length-1;
  GotoXY(X1,Y1);
  memset(OutStr,0xB0,Length);
  OutStr[ThumbPos]=0xB2;
  OutStr[Length]=0;
  vmprintf("%c%s%c",0x1E,OutStr,0x1F);
}

void ShowSeparator(int Length,int Type)
{
  if (Length>1)
  {
    char Separator[4096];
    MakeSeparator(Length,Separator,Type);
    BoxText(Separator,Type>=4);
  }
}

// "Нарисовать" сепаратор в памяти.
char* MakeSeparator(int Length,char *DestStr,int Type)
{
  if (Length>1 && DestStr)
  {
    static unsigned char BoxType[8][4]={
      {0x20,0x20,0xC4,0x00},
      {0xC7,0xB6,0xC4,0x00},
      {0xC3,0xB4,0xC4,0x00},
      {0xCC,0xB9,0xCD,0x00},

      {0x20,0x20,0xB3,0x00},
      {0xD1,0xCF,0xB3,0x00},
      {0xC2,0xC1,0xB3,0x00},
      {0xCB,0xCA,0xBA,0x00},
    };
    Type%=(sizeof(BoxType)/sizeof(BoxType[0]));
    memset(DestStr,BoxType[Type][2],Length);
    DestStr[0]=BoxType[Type][0];
    DestStr[Length-1]=BoxType[Type][1];
    DestStr[Length]=0;
  }
  return DestStr;
}

#if defined(USE_WFUNC)
char GetVidChar(CHAR_INFO CI)
{
  if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
  {
    for(int I=0; I < 256; ++I)
      if(CI.Char.UnicodeChar == Oem2Unicode[I])
        return I;
    return 0;
    /*
    char AsciiChar;
    BOOL UsedDefChar=FALSE;
    WideCharToMultiByte(CP_OEMCP,0,
          &CI.Char.UnicodeChar,1,
          &AsciiChar,1,
          NULL,&UsedDefChar);
    return AsciiChar;
    */
  }
  return CI.Char.AsciiChar;
}

void SetVidChar(CHAR_INFO& CI,char Chr)
{
  if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
    CI.Char.UnicodeChar = Oem2Unicode[Chr];
  else
    CI.Char.AsciiChar=Chr;
}
#endif
