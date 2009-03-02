/*
global.cpp

Глобальные переменные

*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "farqueue.hpp"

/* $ 29.06.2000 tran
  берем char *CopyRight из inc файла */
#include "copyright.inc"

/* $ 07.12.2000 SVS
   + Версия берется из файла farversion.inc
*/
#include "farversion.inc"

OSVERSIONINFO WinVer={0};

struct Options Opt={0};

// функции шифрования (Win2K) назначены? (для SetAttr!)
BOOL IsCryptFileASupport=FALSE;

// языковой файл загружен?
BOOL LanguageLoaded=FALSE;
char InitedLanguage[LANGUAGENAME_SIZE];

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
int RightAltPressed=0,RightCtrlPressed=0,RightShiftPressed=0;
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

int WidthNameForMessage=0;

const char MAC_EOL_fmt[]  ="\r";
const char UNIX_EOL_fmt[] ="\n";
const char DOS_EOL_fmt[]  ="\r\n";
const char WIN_EOL_fmt[]  ="\r\r\n";

BOOL ProcessException=FALSE;
BOOL ProcessShowClock=FALSE;

const char *FarTitleAddons=" - Far";

const char FAR_VerticalBlock[]= "FAR_VerticalBlock";
const char FAR_VerticalBlock_Unicode[]= "FAR_VerticalBlock_Unicode";

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
BYTE  BoxSymbolsA[64];

int _localLastError=0;

const char *ReservedFilenameSymbols="<>|";

int KeepUserScreen;
char DirToSet[NM];

BOOL IsFn_FAR_CopyFileEx=FALSE;

int ViewerInitUseDecodeTable=TRUE,ViewerInitTableNum=0,ViewerInitAnsiText=TRUE;
int EditorInitUseDecodeTable=TRUE,EditorInitTableNum=0,EditorInitAnsiText=TRUE;

int Macro_DskShowPosType=0; // для какой панели вызывали меню выбора дисков (0 - ничерта не вызывали, 1 - левая (AltF1), 2 - правая (AltF2))


// Macro Const
const char constMsX[]="MsX";
const char constMsY[]="MsY";
const char constMsButton[]="MsButton";
const char constMsCtrlState[]="MsCtrlState";
