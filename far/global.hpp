#ifndef __FARGLOBAL_HPP__
#define __FARGLOBAL_HPP__
/*
global.hpp

ќписание глобальных переменных
¬ключать последним.

*/

/* Revision: 1.18 12.05.2001 $ */

/*
Modify:
  12.05.2001 DJ
    ! еще перетр€хи #include: убран #include "ctrlobj.hpp", а указатель на 
      CtrlObject переехал в ctrlobj.hpp; еще немного переездов
  06.05.2001 DJ
    ! перетр€х #include
  26.04.2001 VVM
    - ¬ыкинул нафиг MouseWheeled
  24.04.2001 SVS
    + MouseWheeled - признак того, что крутанули колесо
  01.03.2001 SVS
    + RegColorsHighlight - дл€ сокращени€
  24.01.2001 SVS
    + KeyQueue - внутренн€€ очередь клавиатуры
  09.01.2001 SVS
    + WaitInFastFind - требуетс€ ли трасл€ци€ буковок дл€ правила ShiftsKeyRules
  30.12.2000 SVS
    + IsCryptFileASupport
  22.12.2000 SVS
    + hConOut,hConInp
  07.12.2000 SVS
    + ќписание версии FAR_VERSION - как константа.
  11.11.2000 SVS
    !  осметика: "FarTmpXXXXXX" заменена на переменную FarTmpXXXXXX
  20.09.2000 SVS
    ! hFarWnd глобальна
  23.08.2000 SVS
    + MButtonPressed - дл€ средней клавиши мыши.
  03.08.2000 KM 1.03
    + ƒобавлена переменна€ int GlobalSearchWholeWords.
  03.08.2000 SVS
    ! WordDiv -> Opt.WordDiv
  03.08.2000 SVS
    + путь дл€ поиска основных плагинов
  11.07.2000 SVS
    ! »зменени€ дл€ возможности компил€ции под BC & VC
  07.07.2000 SVS
    + –азграничитель слов из реестра (общий дл€ редактировани€)
  25.06.2000 SVS
    ! ѕодготовка Master Copy
    ! ¬ыделение в качестве самосто€тельного модул€
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

extern int ScrX,ScrY;
extern HANDLE hConOut,hConInp;

extern int AltPressed,CtrlPressed,ShiftPressed;
extern int LButtonPressed,RButtonPressed,PrevMouseX,PrevMouseY,MouseX,MouseY;
/* $ 23.08.2000 SVS
    + MButtonPressed - дл€ средней клавиши мыши.
*/
extern int MButtonPressed;
/* SVS $ */

extern int WaitInMainLoop;
extern int WaitInFastFind;

extern char FarPath[NM];
/* $ 03.08.2000 SVS
    + путь дл€ поиска основных плагинов
*/
extern char MainPluginsPath[NM];
/* SVS $ */

extern char GlobalSearchString[512];
extern int GlobalSearchCase;
/* $ 29.07.2000 KM
   √лобальна€ переменна€, хран€ща€ значение "Whole words" дл€ поиска
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
  + ƒл€ того, чтобы...
*/
extern const char FarTmpXXXXXX[];
/* SVS $ */

/* $ 07.12.2000 SVS
  + ќписание версии FAR_VERSION - как константа.
*/
extern const DWORD FAR_VERSION;
/* SVS $ */

extern BOOL IsCryptFileASupport;


extern char RegColorsHighlight[];


#endif	// __FARGLOBAL_HPP__
