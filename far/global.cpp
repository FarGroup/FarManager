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
/* tran $ */

/* $ 07.12.2000 SVS
   + Версия берется из файла farversion.inc
*/
#include "farversion.inc"
/* SVS $ */

OSVERSIONINFOW WinVer={0};

struct Options Opt;// BUG !! ={0};

// функции шифрования (Win2K) назначены? (для SetAttr!)
BOOL IsCryptFileASupport=FALSE;

// языковой файл загружен?
BOOL LanguageLoaded=FALSE;
wchar_t InitedLanguage[LANGUAGENAME_SIZE];

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

string g_strFarPath;

string strLastFarTitle;
int  TitleModified=FALSE;
wchar_t RegColorsHighlight[]=L"Colors\\Highlight";


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

int CmpNameSearchMode=FALSE;
int DisablePluginsOutput=FALSE;
int CmdMode=FALSE;

PISDEBUGGERPRESENT pIsDebuggerPresent=NULL;

PREREDRAWFUNC PreRedrawFunc=NULL;
struct PreRedrawParamStruct PreRedrawParam={0};

int WidthNameForMessage=0;

const wchar_t DOS_EOL_fmt[]  = L"\r\n";
const wchar_t UNIX_EOL_fmt[] = L"\n";
const wchar_t MAC_EOL_fmt[]  = L"\r";
const wchar_t WIN_EOL_fmt[]  = L"\r\r\n";

const char DOS_EOL_fmtA[]  = "\r\n";
const char UNIX_EOL_fmtA[] = "\n";
const char MAC_EOL_fmtA[]  = "\r";
const char WIN_EOL_fmtA[]  = "\r\r\n";


BOOL ProcessException=FALSE;
BOOL ProcessShowClock=FALSE;

const wchar_t *FarTitleAddons=L" - Far";

const wchar_t FAR_VerticalBlock[]= L"FAR_VerticalBlock";

int InGrabber=FALSE;

const wchar_t *HelpFileMask=L"*.hlf";
const wchar_t *HelpFormatLinkModule=L"<%s>%s";

#if defined(SYSLOG)
BOOL StartSysLog=0;
long CallNewDelete=0;
long CallMallocFree=0;
#endif

class SaveScreen;
SaveScreen *GlobalSaveScrPtr=NULL;

int CriticalInternalError=FALSE;

int UsedInternalClipboard=0;

#if defined(DETECT_ALT_ENTER)
int PrevFarAltEnterMode=-1;
#endif

WCHAR BoxSymbols[64];

int _localLastError=0;

const wchar_t *ReservedFilenameSymbols=L"<>|";

int KeepUserScreen;
string g_strDirToSet;

BOOL IsFn_FAR_CopyFileEx=FALSE;

int ViewerInitUseDecodeTable=TRUE,ViewerInitTableNum=0,ViewerInitAnsiText=TRUE;
int EditorInitUseDecodeTable=TRUE,EditorInitTableNum=0,EditorInitAnsiText=TRUE;

int Macro_DskShowPosType=0; // для какой панели вызывали меню выбора дисков (0 - ничерта не вызывали, 1 - левая (AltF1), 2 - правая (AltF2))
