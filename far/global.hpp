#ifndef __FARGLOBAL_HPP__
#define __FARGLOBAL_HPP__
/*
global.hpp

Описание глобальных переменных
Включать последним.

*/

/* Revision: 1.24 25.07.2001 $ */

/*
Modify:
  25.07.2001 SVS
    + IsProcessAssignMacroKey - идет процесс назначения клавиши в макросе?
  25.07.2001 SVS
    ! Copyright переехала из ctrlobj.cpp.
  24.07.2001 SVS
    + NotUseCAS: флаг на запрет юзание Ctrl-Alt-Shift
  27.06.2001 SVS
    + LanguageLoaded
  25.06.2001 SVS
    ! Юзаем SEARCHSTRINGBUFSIZE
  21.05.2001 OT
    + Переменные CONSOLE_SCREEN_BUFFER_INFO InitScreenBufferInfo, CurScreenBufferInfo
      нужны для более естественного поведения AltF9
  12.05.2001 DJ
    ! еще перетряхи #include: убран #include "ctrlobj.hpp", а указатель на
      CtrlObject переехал в ctrlobj.hpp; еще немного переездов
  06.05.2001 DJ
    ! перетрях #include
  26.04.2001 VVM
    - Выкинул нафиг MouseWheeled
  24.04.2001 SVS
    + MouseWheeled - признак того, что крутанули колесо
  01.03.2001 SVS
    + RegColorsHighlight - для сокращения
  24.01.2001 SVS
    + KeyQueue - внутренняя очередь клавиатуры
  09.01.2001 SVS
    + WaitInFastFind - требуется ли трасляция буковок для правила ShiftsKeyRules
  30.12.2000 SVS
    + IsCryptFileASupport
  22.12.2000 SVS
    + hConOut,hConInp
  07.12.2000 SVS
    + Описание версии FAR_VERSION - как константа.
  11.11.2000 SVS
    ! Косметика: "FarTmpXXXXXX" заменена на переменную FarTmpXXXXXX
  20.09.2000 SVS
    ! hFarWnd глобальна
  23.08.2000 SVS
    + MButtonPressed - для средней клавиши мыши.
  03.08.2000 KM 1.03
    + Добавлена переменная int GlobalSearchWholeWords.
  03.08.2000 SVS
    ! WordDiv -> Opt.WordDiv
  03.08.2000 SVS
    + путь для поиска основных плагинов
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  07.07.2000 SVS
    + Разграничитель слов из реестра (общий для редактирования)
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "farconst.hpp"
#include "struct.hpp"

#ifdef __FARQUEUE_HPP__
extern FarQueue<DWORD> *KeyQueue;
#endif

#if defined(__BORLANDC__)
 #ifdef __TIME_H
 extern clock_t StartIdleTime;
 extern clock_t StartExecTime;
 #endif
#else
 extern clock_t StartIdleTime;
 extern clock_t StartExecTime;
#endif

#if defined(_INC_WINDOWS) || defined(_WINDOWS_)
extern DWORD InitialConsoleMode;
extern OSVERSIONINFO WinVer;
#endif

extern struct Options Opt;

class Editor;
extern Editor *CurrentEditor;


extern CONSOLE_SCREEN_BUFFER_INFO InitScreenBufferInfo, CurScreenBufferInfo;
extern int ScrX,ScrY;
extern HANDLE hConOut,hConInp;

extern int AltPressed,CtrlPressed,ShiftPressed;
extern int LButtonPressed,RButtonPressed,PrevMouseX,PrevMouseY,MouseX,MouseY;
/* $ 23.08.2000 SVS
    + MButtonPressed - для средней клавиши мыши.
*/
extern int MButtonPressed;
/* SVS $ */

extern int WaitInMainLoop;
extern int WaitInFastFind;

extern char FarPath[NM];
/* $ 03.08.2000 SVS
    + путь для поиска основных плагинов
*/
extern char MainPluginsPath[NM];
/* SVS $ */

extern char GlobalSearchString[SEARCHSTRINGBUFSIZE];
extern int GlobalSearchCase;
/* $ 29.07.2000 KM
   Глобальная переменная, хранящая значение "Whole words" для поиска
*/
extern int GlobalSearchWholeWords;
/* KM $*/
extern int GlobalSearchReverse;

extern int ScreenSaverActive;

extern char LastFarTitle[512];

extern int CloseFAR;

extern int RegVer;
extern char RegName[256];

extern int CmpNameSearchMode;
extern int DisablePluginsOutput;
extern int CmdMode;

extern unsigned char DefaultPalette[];
extern unsigned char Palette[];
extern unsigned char BlackPalette[];
extern int SizeArrayPalette;

/* $ 20.09.2000 SVS
  ! hFarWnd глобальна
*/
extern HWND hFarWnd;
/* SVS $ */

/* $ 09.11.2000 SVS
  + Для того, чтобы...
*/
extern const char FarTmpXXXXXX[];
/* SVS $ */

/* $ 07.12.2000 SVS
  + Описание версии FAR_VERSION - как константа.
*/
extern const DWORD FAR_VERSION;
/* SVS $ */

extern BOOL IsCryptFileASupport;


extern char RegColorsHighlight[];

extern BOOL LanguageLoaded;

extern BOOL NotUseCAS;
extern BOOL IsProcessAssignMacroKey;

extern char *Copyright;

#endif	// __FARGLOBAL_HPP__
