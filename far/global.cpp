/*
global.cpp

√лобальные переменные

*/

/* Revision: 1.21 25.07.2001 $ */

/*
Modify:
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
    + hConOut,hConInp плавно перетекли из interf.cpp
  07.12.2000 SVS
    + ¬ерси€ беретс€ из файла farversion.inc
  11.11.2000 SVS
    ! "FarTmpXXXXXX" заменена на переменную FarTmpXXXXXX
  23.08.2000 SVS
    + MButtonPressed - дл€ средней клавиши мыши.
  03.08.2000 KM 1.04
    + ƒобавлена глобальна€ переменна€ int GlobalSearchWholeWords.
  03.08.2000 SVS
    ! WordDiv -> Opt.WordDiv
  03.08.2000 SVS
    + путь дл€ поиска основных плагинов
  07.07.2000 SVS
    + –азграничитель слов из реестра (общий дл€ редактировани€)
  25.06.2000 SVS
    ! ѕодготовка Master Copy
    ! ¬ыделение в качестве самосто€тельного модул€
*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "farqueue.hpp"

/* $ 07.12.2000 SVS
   + ¬ерси€ беретс€ из файла farversion.inc
*/
#include "farversion.inc"
/* SVS $ */

/* $ 29.06.2000 tran
  берем char *CopyRight из inc файла */
#include "copyright.inc"
/* tran $ */

FarQueue<DWORD> *KeyQueue=NULL;

CONSOLE_SCREEN_BUFFER_INFO InitScreenBufferInfo, CurScreenBufferInfo;
int ScrX,ScrY;
HANDLE hConOut,hConInp;

int AltPressed,CtrlPressed,ShiftPressed;
int LButtonPressed,RButtonPressed,PrevMouseX,PrevMouseY,MouseX,MouseY;
/* $ 23.08.2000 SVS
    + MButtonPressed - дл€ средней клавиши мыши.
*/
int MButtonPressed;
/* SVS $ */

clock_t StartIdleTime;

DWORD InitialConsoleMode;

int WaitInMainLoop=FALSE;
int WaitInFastFind=0;


clock_t StartExecTime;

struct Options Opt;

OSVERSIONINFO WinVer;

char FarPath[NM];
/* $ 03.08.2000 SVS
    + путь дл€ поиска основных плагинов
*/
char MainPluginsPath[NM];
/* SVS $ */

char GlobalSearchString[SEARCHSTRINGBUFSIZE];
int GlobalSearchCase;
/* $ 29.07.2000 KM
   √лобальна€ переменна€, хран€ща€ значение "Whole words" дл€ поиска
*/
int GlobalSearchWholeWords;
/* KM $*/
int GlobalSearchReverse;

int ScreenSaverActive;

char LastFarTitle[512];

Editor *CurrentEditor;
int CloseFAR;

int RegVer;
char RegName[256];

int CmpNameSearchMode;
int DisablePluginsOutput;
int CmdMode;

const char FarTmpXXXXXX[]="FarTmpXXXXXX";

BOOL IsCryptFileASupport=FALSE;

char RegColorsHighlight[]="Colors\\Highlight";

BOOL LanguageLoaded=FALSE;

// флаг на запрет юзание Ctrl-Alt-Shift
BOOL NotUseCAS=FALSE;
