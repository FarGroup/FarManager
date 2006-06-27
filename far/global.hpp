#ifndef __FARGLOBAL_HPP__
#define __FARGLOBAL_HPP__
/*
global.hpp

�������� ���������� ����������
�������� ���������.

*/

/* Revision: 1.73 21.05.2006 $ */

#include "UnicodeString.hpp"
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
extern OSVERSIONINFOW WinVer;
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

extern string g_strFarPath;

extern char GlobalSearchString[SEARCHSTRINGBUFSIZE];
extern int GlobalSearchCase;
/* $ 29.07.2000 KM
   ���������� ����������, �������� �������� "Whole words" ��� ������
*/
extern int GlobalSearchWholeWords;
/* KM $*/
/* $ 22.09.2003 KM
   ���������� ����������, �������� �������� "Search for hex" ��� ������
*/
extern int GlobalSearchHex;
/* KM $ */
extern int GlobalSearchReverse;

extern int ScreenSaverActive;

extern string strLastFarTitle;
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
  ! hFarWnd ���������
*/
extern HWND hFarWnd;
/* SVS $ */

/* $ 07.12.2000 SVS
  + �������� ������ FAR_VERSION - ��� ���������.
*/
extern const DWORD FAR_VERSION;
/* SVS $ */

extern BOOL IsCryptFileASupport;


extern wchar_t RegColorsHighlight[];

extern BOOL LanguageLoaded;

extern BOOL NotUseCAS;
extern BOOL IsProcessAssignMacroKey;
extern BOOL IsProcessVE_FindFile;
extern BOOL IsRedrawFramesInProcess;

extern PREREDRAWFUNC PreRedrawFunc;
extern struct PreRedrawParamStruct PreRedrawParam;

extern char *Copyright;

extern PISDEBUGGERPRESENT pIsDebuggerPresent;

extern int WidthNameForMessage;

extern const char DOS_EOL_fmt[], UNIX_EOL_fmt[], MAC_EOL_fmt[];
extern const wchar_t DOS_EOL_fmtW[], UNIX_EOL_fmtW[], MAC_EOL_fmtW[];

extern BOOL ProcessException;

extern BOOL ProcessShowClock;

extern const char *FarTitleAddons;
extern const wchar_t *FarTitleAddonsW;

extern const char FAR_VerticalBlock[];
extern const wchar_t FAR_VerticalBlockW[];

extern int InGrabber;    // �� ������ � �������?

extern const char *PluginsFolderName;

extern const wchar_t *PluginsFolderNameW;

extern const wchar_t *HelpFileMask;
extern const wchar_t *HelpFormatLinkModule;

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

extern int _localLastError;

extern const char *ReservedFilenameSymbols;
extern const wchar_t *ReservedFilenameSymbolsW;

extern int KeepUserScreen;
extern string g_strDirToSet; //RAVE!!!

extern BOOL IsFn_FAR_CopyFileEx;

extern int EditorInitUseDecodeTable,EditorInitTableNum,EditorInitAnsiText;
extern int ViewerInitUseDecodeTable,ViewerInitTableNum,ViewerInitAnsiText;

extern int Macro_DskShowPosType; // ��� ����� ������ �������� ���� ������ ������ (0 - ������� �� ��������, 1 - ����� (AltF1), 2 - ������ (AltF2))

#endif  // __FARGLOBAL_HPP__
