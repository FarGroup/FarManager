#ifndef __FARGLOBAL_HPP__
#define __FARGLOBAL_HPP__
/*
global.hpp

ќписание глобальных переменных
¬ключать последним.

*/

/* Revision: 1.46 23.12.2002 $ */

/*
Modify:
  23.12.2002 SVS
    ! OnlyEditorViewerUsed стал частью структуры Options
  10.12.2002 SVS
    + StartSysLog - дл€ (потом будет) управл€емого писани€ логов!
  04.11.2002 SVS
    ! ReturnAltValue уехала из keyboard.cpp в global.cpp
  14.07.2002 SVS
    ! новые константы PluginsFolderName, HelpFileMask, HelpFormatLinkModule
      переехали из farconst.hpp в global.?pp
  24.05.2002 SVS
    + InGrabber, FAR_VerticalBlock
  22.05.2002 SVS
    ! Editor -> FileEditor
  16.05.2002 SVS
    + ProcessShowClock - "мы в часиках?"
  07.04.2002 KM
    + IsRedrawFramesInProcess
  05.04.2002 SVS
    + Prev?ButtonPressed
  01.04.2002 SVS
    ! ѕро заголовок - FarTitleAddons
  30.03.2002 OT
    - ѕосле исправлени€ бага є314 (патч 1250) отвалилось закрытие
      фара по кресту.
  19.03.2002 SVS
    + MAC_EOL_fmt
  01.03.2002 SVS
    ! FarTmpXXXXXX - удалено за ненадобностью.
  21.02.2002 SVS
    + ProcessException - признак процесса обработки исключени€
  22.01.2002 SVS
    + OnliEditorViewerUsed,  =TRUE, если старт был /e или /v
  14.01.2002 SVS
    + DOS_EOL_fmt[], UNIX_EOL_fmt (из editor.cpp)
  26.11.2001 SVS
    + MouseEventFlags, PreMouseEventFlags - типы эвентов мыши
  23.10.2001 SVS
    + WidthNameForMessage - 38% дл€ размера усечени€ имени в месагах-процессах
  21.10.2001 SVS
    ! PREREDRAWFUNC и PISDEBUGGERPRESENT переехали в farconst.hpp
    + PrevScrX,PrevScrY - предыдущие размеры консоли (дл€ позиционировани€
      диалогов)
  19.10.2001 SVS
    + PreRedraw* - дл€ исправлени€ BugZ#85
  03.10.2001 SVS
    ! ¬ некоторых источниках говоритс€, что IsDebuggerPresent() есть только
      в NT, так что... бум юзать ее динамически!
  07.08.2001 SVS
    + IsProcessVE_FindFile - идет процесс "вьювер/редактор" во врем€
      поиска файлов?
  25.07.2001 SVS
    + IsProcessAssignMacroKey - идет процесс назначени€ клавиши в макросе?
  25.07.2001 SVS
    ! Copyright переехала из ctrlobj.cpp.
  24.07.2001 SVS
    + NotUseCAS: флаг на запрет юзание Ctrl-Alt-Shift
  27.06.2001 SVS
    + LanguageLoaded
  25.06.2001 SVS
    ! ёзаем SEARCHSTRINGBUFSIZE
  21.05.2001 OT
    + ѕеременные CONSOLE_SCREEN_BUFFER_INFO InitScreenBufferInfo, CurScreenBufferInfo
      нужны дл€ более естественного поведени€ AltF9
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

class FileEditor;
extern FileEditor *CurrentEditor;


extern CONSOLE_SCREEN_BUFFER_INFO InitScreenBufferInfo, CurScreenBufferInfo;
extern int ScrX,ScrY;
extern int PrevScrX,PrevScrY;
extern HANDLE hConOut,hConInp;

extern int AltPressed,CtrlPressed,ShiftPressed;
extern int LButtonPressed, PrevLButtonPressed;
extern int RButtonPressed, PrevRButtonPressed;
extern int MButtonPressed, PrevMButtonPressed;
extern int PrevMouseX,PrevMouseY,MouseX,MouseY;
extern int PreMouseEventFlags,MouseEventFlags;
extern int ReturnAltValue;

extern int WaitInMainLoop;
extern int WaitInFastFind;

extern char FarPath[NM];
/* $ 03.08.2000 SVS
    + путь дл€ поиска основных плагинов
*/
extern char MainPluginsPath[NM];
/* SVS $ */

extern char GlobalSearchString[SEARCHSTRINGBUFSIZE];
extern int GlobalSearchCase;
/* $ 29.07.2000 KM
   √лобальна€ переменна€, хран€ща€ значение "Whole words" дл€ поиска
*/
extern int GlobalSearchWholeWords;
/* KM $*/
extern int GlobalSearchReverse;

extern int ScreenSaverActive;

extern char LastFarTitle[512];

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
  + ќписание версии FAR_VERSION - как константа.
*/
extern const DWORD FAR_VERSION;
/* SVS $ */

extern BOOL IsCryptFileASupport;


extern char RegColorsHighlight[];

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

extern BOOL ProcessException;

extern BOOL ProcessShowClock;

extern const char *FarTitleAddons;

extern const char FAR_VerticalBlock[];

extern int InGrabber;    // ћы сейчас в грабере?

extern const char *PluginsFolderName;
extern const char *HelpFileMask;
extern const char *HelpFormatLinkModule;

#if defined(SYSLOG)
extern BOOL StartSysLog;
#endif

#endif  // __FARGLOBAL_HPP__
