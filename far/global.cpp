#ifdef EXTERNALVAR
  #define EXTVAR extern
#else
  #define EXTVAR
#endif

EXTVAR Language Lang;

EXTVAR ControlObject *CtrlObject;

EXTVAR int ScrX,ScrY;

EXTVAR int AltPressed,CtrlPressed,ShiftPressed;
EXTVAR int LButtonPressed,RButtonPressed,PrevMouseX,PrevMouseY,MouseX,MouseY;
EXTVAR clock_t StartIdleTime;

EXTVAR DWORD InitialConsoleMode;

EXTVAR int WaitInMainLoop;
EXTVAR clock_t StartExecTime;

EXTVAR struct Options Opt;

EXTVAR OSVERSIONINFO WinVer;

EXTVAR char FarPath[NM];

EXTVAR char GlobalSearchString[512];
EXTVAR int GlobalSearchCase;
EXTVAR int GlobalSearchReverse;

EXTVAR int ScreenSaverActive;

EXTVAR char LastFarTitle[512];

EXTVAR ScreenBuf ScrBuf;

EXTVAR Editor *CurrentEditor;
EXTVAR int CloseFAR;

EXTVAR int RegVer;
EXTVAR char RegName[256];

EXTVAR int CmpNameSearchMode;
EXTVAR int DisablePluginsOutput;
EXTVAR int CmdMode;
