#ifndef __FARGLOBAL_HPP__
#define __FARGLOBAL_HPP__
/*
global.hpp

Описание глобальных переменных
Включать последним.

*/

/* Revision: 1.11 30.12.2000 $ */

/*
Modify:
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

#ifdef __LANGUAGE_HPP__
extern Language Lang;
#endif

#ifdef __CONTROLOBJECT_HPP__
extern ControlObject *CtrlObject;
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

#ifdef __FARSTRUCT_HPP__
extern struct Options Opt;
#endif

#ifdef __SCREENBUF_HPP__
extern ScreenBuf ScrBuf;
#endif

#ifdef __EDITOR_HPP__
extern Editor *CurrentEditor;
#endif

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

extern char FarPath[NM];
/* $ 03.08.2000 SVS
    + путь для поиска основных плагинов
*/
extern char MainPluginsPath[NM];
/* SVS $ */

extern char GlobalSearchString[512];
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

#endif	// __FARGLOBAL_HPP__
