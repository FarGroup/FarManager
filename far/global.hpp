#ifndef __FARGLOBAL_HPP__
#define __FARGLOBAL_HPP__
/*
global.hpp

Описание глобальных переменных
Включать последним.

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

#if defined(_INC_WINDOWS) || defined(_WINDOWS_) || defined(_WINDOWS_H)
extern DWORD InitialConsoleMode;
extern OSVERSIONINFO WinVer;
#endif

extern struct Options Opt;

class FileEditor;
extern FileEditor *CurrentEditor;


extern CONSOLE_SCREEN_BUFFER_INFO InitScreenBufferInfo, CurScreenBufferInfo;
extern int ScrX,ScrY;
extern int PrevScrX,PrevScrY;
extern HANDLE hConOut,hConInp;

extern int AltPressed,CtrlPressed,ShiftPressed;
extern int RightAltPressed,RightCtrlPressed,RightShiftPressed;
extern int LButtonPressed, PrevLButtonPressed;
extern int RButtonPressed, PrevRButtonPressed;
extern int MButtonPressed, PrevMButtonPressed;
extern int PrevMouseX,PrevMouseY,MouseX,MouseY;
extern int PreMouseEventFlags,MouseEventFlags;
extern int ReturnAltValue;

extern int WaitInMainLoop;
extern int WaitInFastFind;

extern char FarPath[NM];

extern char GlobalSearchString[SEARCHSTRINGBUFSIZE];
extern int GlobalSearchCase;
/* $ 29.07.2000 KM
   Глобальная переменная, хранящая значение "Whole words" для поиска
*/
extern int GlobalSearchWholeWords;
/* KM $*/
/* $ 22.09.2003 KM
   Глобальная переменная, хранящая значение "Search for hex" для поиска
*/
extern int GlobalSearchHex;
/* KM $ */
extern int GlobalSearchReverse;

extern int ScreenSaverActive;

extern char LastFarTitle[512];
extern int  TitleModified;
extern int CloseFAR, CloseFARMenu;

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

/* $ 07.12.2000 SVS
  + Описание версии FAR_VERSION - как константа.
*/
extern const DWORD FAR_VERSION;
/* SVS $ */

extern BOOL IsCryptFileASupport;


extern char RegColorsHighlight[];

extern BOOL LanguageLoaded;
extern char InitedLanguage[];

extern BOOL NotUseCAS;
extern BOOL IsProcessAssignMacroKey;
extern BOOL IsProcessVE_FindFile;
extern BOOL IsRedrawFramesInProcess;

extern char *Copyright;

extern PISDEBUGGERPRESENT pIsDebuggerPresent;

extern int WidthNameForMessage;

extern const char DOS_EOL_fmt[];
extern const char UNIX_EOL_fmt[];
extern const char MAC_EOL_fmt[];
extern const char WIN_EOL_fmt[];

extern BOOL ProcessException;

extern BOOL ProcessShowClock;

extern const char *FarTitleAddons;

extern const char FAR_VerticalBlock[];
extern const char FAR_VerticalBlock_Unicode[];

extern int InGrabber;    // Мы сейчас в грабере?

extern const char *PluginsFolderName;
extern const char *HelpFileMask;
extern const char *HelpFormatLinkModule;

#if defined(SYSLOG)
extern BOOL StartSysLog;
extern long CallNewDelete;
extern long CallMallocFree;
#endif

class SaveScreen;
extern SaveScreen *GlobalSaveScrPtr;

extern int CriticalInternalError;

extern int UsedInternalClipboard;

#ifdef _DEBUGEXC
extern int CheckRegistration;
#endif

extern int RegistrationBugs;

#if defined(DETECT_ALT_ENTER)
extern int PrevFarAltEnterMode;
#endif

extern WCHAR BoxSymbols[];
extern BYTE  BoxSymbolsA[];

extern int _localLastError;

extern const char *ReservedFilenameSymbols;

extern int KeepUserScreen;
extern char DirToSet[NM];

extern BOOL IsFn_FAR_CopyFileEx;

extern int EditorInitUseDecodeTable,EditorInitTableNum,EditorInitAnsiText;
extern int ViewerInitUseDecodeTable,ViewerInitTableNum,ViewerInitAnsiText;

extern int Macro_DskShowPosType; // для какой панели вызывали меню выбора дисков (0 - ничерта не вызывали, 1 - левая (AltF1), 2 - правая (AltF2))

// Macro Const
extern const char constMsX[];
extern const char constMsY[];
extern const char constMsButton[];
extern const char constMsCtrlState[];

#endif  // __FARGLOBAL_HPP__
