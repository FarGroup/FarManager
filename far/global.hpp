#ifndef __FARGLOBAL_HPP__
#define __FARGLOBAL_HPP__
/*
global.hpp

Описание глобальных переменных
Включать последним.

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
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

#ifdef __TIME_H
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

extern int AltPressed,CtrlPressed,ShiftPressed;
extern int LButtonPressed,RButtonPressed,PrevMouseX,PrevMouseY,MouseX,MouseY;

extern int WaitInMainLoop;

extern char FarPath[NM];

extern char GlobalSearchString[512];
extern int GlobalSearchCase;
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

#endif	// __FARGLOBAL_HPP__
