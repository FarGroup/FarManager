#ifndef __FARGLOBAL_HPP__
#define __FARGLOBAL_HPP__
/*
global.hpp

Описание глобальных переменных
Включать последним.

*/

/* Revision: 1.51 06.05.2003 $ */

/*
Modify:
  06.05.2003 SVS
    + BoxSymbols[] - массив символов пвсевдографики
  21.04.2003 SVS
    + RegistrationBugs - =TRUE, если трид создать не удалось.
    + PrevFarAltEnterMode - для тестирования "Alt-Enetr"
  25.02.2003 SVS
    ! применим счетчик CallNewDelete/CallMallocFree для отладки
  04.02.2003 SVS
    + В общем, теперь в дебажной версии есть ключ "/cr", отключающий трид
      проверки регистрации. Под TD32 иногда жутчайшие тормоза наблюдаются.
  10.01.2003 SVS
    + Глобальная переменная CriticalInternalError говорит, что
      нужно срочно терминировать ФАР, т.к. дальнейшее продолжение исполнения
      кода проблематично
    + UsedInternalClipboard - применять эмуляцию клипборда :-)
  06.01.2003 SVS
    + GlobalSaveScrPtr - глобальная переменная-указатель, для того, чтобы
      при изменении размеров экрана молча "убить" буфер сохранения.
  23.12.2002 SVS
    ! OnlyEditorViewerUsed стал частью структуры Options
  10.12.2002 SVS
    + StartSysLog - для (потом будет) управляемого писания логов!
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
    ! Про заголовок - FarTitleAddons
  30.03.2002 OT
    - После исправления бага №314 (патч 1250) отвалилось закрытие
      фара по кресту.
  19.03.2002 SVS
    + MAC_EOL_fmt
  01.03.2002 SVS
    ! FarTmpXXXXXX - удалено за ненадобностью.
  21.02.2002 SVS
    + ProcessException - признак процесса обработки исключения
  22.01.2002 SVS
    + OnliEditorViewerUsed,  =TRUE, если старт был /e или /v
  14.01.2002 SVS
    + DOS_EOL_fmt[], UNIX_EOL_fmt (из editor.cpp)
  26.11.2001 SVS
    + MouseEventFlags, PreMouseEventFlags - типы эвентов мыши
  23.10.2001 SVS
    + WidthNameForMessage - 38% для размера усечения имени в месагах-процессах
  21.10.2001 SVS
    ! PREREDRAWFUNC и PISDEBUGGERPRESENT переехали в farconst.hpp
    + PrevScrX,PrevScrY - предыдущие размеры консоли (для позиционирования
      диалогов)
  19.10.2001 SVS
    + PreRedraw* - для исправления BugZ#85
  03.10.2001 SVS
    ! В некоторых источниках говорится, что IsDebuggerPresent() есть только
      в NT, так что... бум юзать ее динамически!
  07.08.2001 SVS
    + IsProcessVE_FindFile - идет процесс "вьювер/редактор" во время
      поиска файлов?
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

#endif  // __FARGLOBAL_HPP__
