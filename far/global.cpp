/*
global.cpp

Глобальные переменные

*/

/* Revision: 1.58 22.09.2003 $ */

/*
Modify:
  22.09.2003 KM
    + GlobalSearchHex - Глобальная переменная, хранящая значение
      "Search for hex" для поиска
  15.09.2003 SVS
    + ReservedFilenameSymbols - недопустимые символы в имени файла/каталога
  26.08.2003 SVS
    + TitleModified
  06.06.2003 SVS
    ! MainPluginsPath переехал в Opt.LoadPlug.
  01.06.2003 SVS
    + _localLastError
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
  05.12.2002 SVS
    - неверно проинициализированы PrevScrX и PrevScrY
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
    ! _ВСЕ_ проинициализируем. Зодолбала "неизвестность"!
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
  15.01.2002 SVS
    ! Изменен формат шаблона для имен временных файлов FarTmpXXXXXX
  14.01.2002 SVS
    + DOS_EOL_fmt[], UNIX_EOL_fmt (из editor.cpp)
  26.11.2001 SVS
    + MouseEventFlags, PreMouseEventFlags - типы эвентов мыши
  23.10.2001 SVS
    + WidthNameForMessage - 38% для размера усечения имени в месагах-процессах
  21.10.2001 SVS
    + PrevScrX,PrevScrY - предыдущие размеры консоли (для позиционирования
      диалогов)
  19.10.2001 SVS
    + PreRedraw* - для исправления BugZ#85
  03.10.2001 SVS
    ! В некоторых источниках говорится, что IsDebuggerPresent() есть только
      в NT, так что... бум юзать ее динамически!
  18.09.2001 SVS
    ! "FarTmpXXXXXX" -> "FARTMPXXXXXX".
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
    + hConOut,hConInp плавно перетекли из interf.cpp
  07.12.2000 SVS
    + Версия берется из файла farversion.inc
  11.11.2000 SVS
    ! "FarTmpXXXXXX" заменена на переменную FarTmpXXXXXX
  23.08.2000 SVS
    + MButtonPressed - для средней клавиши мыши.
  03.08.2000 KM 1.04
    + Добавлена глобальная переменная int GlobalSearchWholeWords.
  03.08.2000 SVS
    ! WordDiv -> Opt.WordDiv
  03.08.2000 SVS
    + путь для поиска основных плагинов
  07.07.2000 SVS
    + Разграничитель слов из реестра (общий для редактирования)
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "farqueue.hpp"

/* $ 29.06.2000 tran
  берем char *CopyRight из inc файла */
#include "copyright.inc"
/* tran $ */

/* $ 07.12.2000 SVS
   + Версия берется из файла farversion.inc
*/
#include "farversion.inc"
/* SVS $ */

OSVERSIONINFO WinVer={0};

struct Options Opt={0};

// функции шифрования (Win2K) назначены? (для SetAttr!)
BOOL IsCryptFileASupport=FALSE;

// языковой файл загружен?
BOOL LanguageLoaded=FALSE;

// флаг на запрет юзание Ctrl-Alt-Shift
BOOL NotUseCAS=FALSE;

// идет процесс назначения клавиши в макросе?
BOOL IsProcessAssignMacroKey=FALSE;

// идет процесс "вьювер/редактор" во время поиска файлов?
BOOL IsProcessVE_FindFile=FALSE;

// Идёт процесс перерисовки всех фреймов
BOOL IsRedrawFramesInProcess=FALSE;

// идет процесс быстрого поиска в панелях?
int WaitInFastFind=FALSE;

// мы крутимся в основном цикле?
int WaitInMainLoop=FALSE;

// "дополнительная" очередь кодов клавиш
FarQueue<DWORD> *KeyQueue=NULL;
int AltPressed=0,CtrlPressed=0,ShiftPressed=0;
int LButtonPressed=0,RButtonPressed=0,MButtonPressed=0;
int PrevLButtonPressed=0, PrevRButtonPressed=0, PrevMButtonPressed=0;
int PrevMouseX=0,PrevMouseY=0,MouseX=0,MouseY=0;
int PreMouseEventFlags=0,MouseEventFlags=0;

// только что был ввод Alt-Цифира?
int ReturnAltValue=0;


CONSOLE_SCREEN_BUFFER_INFO InitScreenBufferInfo={0};
CONSOLE_SCREEN_BUFFER_INFO CurScreenBufferInfo={0};
int ScrX=0,ScrY=0;
int PrevScrX=-1,PrevScrY=-1;
HANDLE hConOut=NULL,hConInp=NULL;

clock_t StartIdleTime=0;

DWORD InitialConsoleMode=0;

clock_t StartExecTime=0;

char FarPath[NM];

char LastFarTitle[512];
int  TitleModified=FALSE;
char RegColorsHighlight[]="Colors\\Highlight";


char GlobalSearchString[SEARCHSTRINGBUFSIZE];
int GlobalSearchCase=FALSE;
/* $ 29.07.2000 KM
   Глобальная переменная, хранящая значение "Whole words" для поиска
*/
int GlobalSearchWholeWords=FALSE;
/* KM $*/
/* $ 22.09.2003 KM
   Глобальная переменная, хранящая значение "Search for hex" для поиска
*/
int GlobalSearchHex=FALSE;
/* KM $ */
int GlobalSearchReverse=FALSE;

int ScreenSaverActive=FALSE;

FileEditor *CurrentEditor=NULL;
int CloseFAR=FALSE,CloseFARMenu=FALSE;

// Про регистрацию
int  RegVer;
char RegName[256];

int CmpNameSearchMode=FALSE;
int DisablePluginsOutput=FALSE;
int CmdMode=FALSE;

PISDEBUGGERPRESENT pIsDebuggerPresent=NULL;

PREREDRAWFUNC PreRedrawFunc=NULL;
struct PreRedrawParamStruct PreRedrawParam={0};

int WidthNameForMessage=0;

const char DOS_EOL_fmt[]  ="\r\n",
           UNIX_EOL_fmt[] ="\n",
           MAC_EOL_fmt[]  ="\r";

BOOL ProcessException=FALSE;
BOOL ProcessShowClock=FALSE;

const char *FarTitleAddons=" - Far";

const char FAR_VerticalBlock[]= "FAR_VerticalBlock";

int InGrabber=FALSE;

const char *PluginsFolderName="Plugins";
const char *HelpFileMask="*.hlf";
const char *HelpFormatLinkModule="<%s>%s";

#if defined(SYSLOG)
BOOL StartSysLog=0;
long CallNewDelete=0;
long CallMallocFree=0;
#endif

class SaveScreen;
SaveScreen *GlobalSaveScrPtr=NULL;

int CriticalInternalError=FALSE;

int UsedInternalClipboard=0;

#ifdef _DEBUGEXC
int CheckRegistration=TRUE;
#endif

int RegistrationBugs=FALSE;

#if defined(DETECT_ALT_ENTER)
int PrevFarAltEnterMode=-1;
#endif

WCHAR BoxSymbols[64];

int _localLastError=0;

const char *ReservedFilenameSymbols="<>|";
