#ifndef MACRO_INCLUDED

#include <tchar.h>
#include "plugin.hpp"
#include "strclass.hpp"
#include "regclass.hpp"

#define GROUPNAMELEN 128
#define CAPTIONLEN 128
#define TITLELEN 64

#ifndef UNICODE
#define GetCheck(i) DialogItems[i].Param.Selected
#else
#define GetCheck(i) (int)Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0)
#endif

#ifndef UNICODE
#define GetDataPtr(i) DialogItems[i].Data.Data
#else
#define GetDataPtr(i) ((const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0))
#endif

//----
HINSTANCE hInstance;
PluginStartupInfo Info;
FarStandardFunctions FSF;
OSVERSIONINFO vi;


//----
TCHAR PluginRootKey[128];
TCHAR FarKey[128];               // default "\\Software\\Far"
TCHAR FarUsersKey[128];          // default "\\Software\\Far\\Users"
TCHAR KeyMacros[128];            // default "\\Software\\Far\\KeyMacros"
TCHAR FarUserName[MAX_PATH_LEN];
TCHAR FarFullName[MAX_PATH_LEN]; // default "C:\\Program Files\\Far\\Far.exe"


//----
int OpenFrom;
static int AltPressed=FALSE,CtrlPressed=FALSE,ShiftPressed=FALSE;


//----
const TCHAR *HKCU=_T("HKEY_CURRENT_USER");
const TCHAR *KeyMacros_KEY=_T("KeyMacros");
const TCHAR *Module_KEY=_T("MacroView");
const TCHAR *Plugins_KEY=_T("\\Plugins");
#ifndef UNICODE
const char *Default_KEY="\\Software\\Far";
const char *Users_KEY="\\Software\\Far\\Users";
#else
const wchar_t *Default_KEY=L"\\Software\\Far2";
const wchar_t *Users_KEY=L"\\Software\\Far2\\Users";
#endif

//----
const TCHAR *MacroGroupShort[]=
{
	_T("Dialog"),_T("Disks"),_T("Editor"),_T("Help"),_T("Info"),_T("MainMenu"),
	_T("Menu"),_T("QView"),_T("Search"),_T("Shell"),_T("Tree"),_T("Viewer"),
	_T("Other"),_T("Common"),_T("FindFolder"),_T("UserMenu"),
#ifdef UNICODE
	_T("AutoCompletion"),
#endif
};


//----
// позиция в этом массиве содержит номер позиции в массиве
// MacroGroupShort, само значение означает откуда был запущен
// плагин
int GroupIndex[]=
{
	-1,9,-1,-1,-1,2,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,
#ifdef UNICODE
	-1,
#endif
};


//----
enum GroupNameConvert
{
	GRP_TOLONGNAME,
	GRP_TOSHORTNAME,
};

enum
{
	DM_NONE,
	DM_DELETED,
	DM_DEACTIVATED,
};

enum
{
	KB_COMMON,
	KB_ALT,
	KB_CTRL,
	KB_SHIFT,
	KB_DIALOG,
	KB_SHIFTDIALOG,
};

enum
{
	MAC_MENUACTIVE,
	MAC_EDITACTIVE,
	MAC_DELETEACTIVE,
	MAC_EXPORTACTIVE,
	MAC_COPYACTIVE,
	MAC_ERRORACTIVE,
};

enum EditMode
{
	EM_NONE,
	EM_INSERT,
	EM_EDIT,
};


//----
struct InitDialogItem
{
	int Type;
	int X1,Y1,X2,Y2;
	int Focus;
	DWORD_PTR Selected;
	int Flags;
	int DefaultButton;
	const TCHAR *Data;
};


struct Config
{
	int AddDescription;
	int AutomaticSave;
	int ViewShell;
	int ViewViewer;
	int ViewEditor;
	int UseHighlight;
	int StartDependentSort;
	int LongGroupNames;
	int MenuCycle;
	int DblClick;
	int GroupDivider;
	int SaveOnStart;
};


struct MenuData
{
	TCHAR Group[ARRAYSIZE(MacroGroupShort)];
	TCHAR Key[32];
};


//----
BOOL InterceptDllCall(HMODULE hLocalModule,const char* c_szDllName,const char* c_szApiName,
                      PVOID pApiNew,PVOID* p_pApiOrg);

typedef BOOL (WINAPI *TReadConsoleInput)(HANDLE hConsole,INPUT_RECORD *ir,DWORD nNumber,LPDWORD nNumberOfRead);
typedef BOOL (WINAPI *TPeekConsoleInput)(HANDLE hConsole,INPUT_RECORD *ir,DWORD nNumber,LPDWORD nNumberOfRead);
TReadConsoleInput p_fnReadConsoleInputOrgA;
TReadConsoleInput p_fnReadConsoleInputOrgW;
TPeekConsoleInput p_fnPeekConsoleInputOrgA;
TPeekConsoleInput p_fnPeekConsoleInputOrgW;

LONG_PTR WINAPI MacroDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2);
LONG_PTR WINAPI MenuDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2);
LONG_PTR WINAPI DefKeyDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2);
LONG_PTR WINAPI CopyDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2);
//BOOL WINAPI ProcessKey(PINPUT_RECORD ir);
BOOL __fastcall ProcessPeekKey(PINPUT_RECORD ir);


//----
TCHAR *__fastcall AllTrim(TCHAR *S);
TCHAR *__fastcall UnQuoteText(TCHAR *S);
TCHAR *__fastcall QuoteText(TCHAR *S,BOOL Force=FALSE);
TCHAR *__fastcall GetMsg(int MsgId);
TCHAR *__fastcall CheckFirstBackSlash(TCHAR *S,BOOL mustbe);
TCHAR *__fastcall CheckLen(TCHAR *S,unsigned ln,BOOL AddDots=TRUE);
TCHAR *__fastcall CheckRLen(TCHAR *S,unsigned ln,BOOL AddDots=TRUE);
int   __fastcall CmpStr(const TCHAR *String1,const TCHAR *String2,int ln1=-1,int ln2=-1);


//----
class TMacroView
{
		friend LONG_PTR WINAPI MacroDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2);
		friend LONG_PTR WINAPI MenuDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2);
		friend LONG_PTR WINAPI DefKeyDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2);
		friend LONG_PTR WINAPI CopyDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2);
		friend BOOL WINAPI myReadConsoleInputA(HANDLE hConsole,PINPUT_RECORD ir,DWORD nNumber,LPDWORD nNumberOfRead);
		friend BOOL WINAPI myReadConsoleInputW(HANDLE hConsole,PINPUT_RECORD ir,DWORD nNumber,LPDWORD nNumberOfRead);
		friend BOOL WINAPI myPeekConsoleInputA(HANDLE hConsole,PINPUT_RECORD ir,DWORD nNumber,LPDWORD nNumberOfRead);
		friend BOOL WINAPI myPeekConsoleInputW(HANDLE hConsole,PINPUT_RECORD ir,DWORD nNumber,LPDWORD nNumberOfRead);
//		friend BOOL WINAPI ProcessKey(PINPUT_RECORD ir);
		friend BOOL __fastcall ProcessPeekKey(PINPUT_RECORD ir);
		friend void __fastcall FlushInputBuffer();

	private:
		const TCHAR *MacroText;
		const TCHAR *MacroCmdHistory;
		const TCHAR *MacroKeyHistory;
		const TCHAR *MacroDescrHistory;
		const TCHAR *MacroExpHistory;
		const TCHAR *MacroCopyHistory;

		Config        Conf;
#ifdef UNICODE
#define EDITDIALOGCOUNT 33
#else
#define EDITDIALOGCOUNT 32
#endif
		FarDialogItem EditDialog[EDITDIALOGCOUNT];
		FarDialogItem DefKeyDialog[2];
		FarListItem   GroupItems[ARRAYSIZE(MacroGroupShort)];
		FarList       GroupList,ConfList;

		BOOL         CtrlDotPressed;
		BOOL         WaitForKeyToMacro;
		BOOL         AltInsPressed;
		BOOL         HelpInvoked;
		BOOL         HelpActivated;
		BOOL         EditInMove;
		BOOL         MultiLine;
		HANDLE       hand;
		HANDLE       EditDlg;
		HANDLE       MenuDlg;
		HANDLE       DefDlg;
		HANDLE       SaveScr;
		/*SaveBar,*/
		HANDLE       hOut;
		HANDLE       hIn;

		TStrList    *NameList;
		TStrList    *MacNameList;
		TStrList    *DescrList;
		TStrList    *ValueList;
		TStrList    *MenuList;

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		WIN32_FIND_DATA fData;
		COORD ConsoleSize;

		TCHAR       S[MAX_PATH_LEN];
		TCHAR       Str[MAX_PATH_LEN];
		TCHAR       TempPath[MAX_PATH_LEN];
		TCHAR       TempFileName[MAX_PATH_LEN];
		TCHAR       Group[MAX_KEY_LEN];
		TCHAR       Key[MAX_KEY_LEN];

		TCHAR       OldCaption[CAPTIONLEN];
		TCHAR       NewCaption[CAPTIONLEN];
		TCHAR       MenuTitle[TITLELEN];
		TCHAR       MenuBottom[TITLELEN];

		TCHAR         *MacroData;

		int           Deactivated;
		int           ActiveMode;
		int           EditMode;
		int           MenuItemsNumber;
		int           MacroGroupsSize;
		int           KeyWidth;
		int           GroupKeyLen;
		int           MaxMenuItemLen;
		int           SelectPos;
		int           TopPos;
		int           GroupPos;
		int           UserConfPos;
		int           LastFocus;
		int           MenuX,MenuY,MenuH,MenuW;
		int           EditX1,EditY1,EditX2,EditY2;
#ifdef UNICODE
		// for EditDialog
		wchar_t       _Button[/*BUTTONLEN*/70];
		wchar_t       _Group[MAX_KEY_LEN]; //длинное название текущего раздела макроса
		wchar_t       _Data[MAX_PATH_LEN];
		wchar_t      *_DataPtr;
		size_t        _DataPtrSize;
		wchar_t       _Descr[MAX_PATH_LEN];
#endif

	private:
		void          __fastcall InitData();
		void          __fastcall InitMacroAreas();
		void          __fastcall InitDialogs();
//		void          __fastcall ParseMenuItem(FarListGetItem *List);
		void          __fastcall WriteKeyBar(int kbType);
		BOOL          __fastcall CreateDirs(TCHAR *Dir);
		TCHAR         *ConvertGroupName(TCHAR *Group,int nWhere);
		void          InitDialogItems(InitDialogItem *Init,FarDialogItem *Item,int ItemsNumber);
		void          __fastcall InsertMacroToEditor(BOOL AllMacros);
		void          __fastcall ExportMacroToFile(BOOL AllMacros=FALSE);
		void          SwitchOver(const TCHAR *Group,const TCHAR *Key);
		int           DeletingMacro(const TCHAR **Items,int ItemsSize,const TCHAR *HelpTopic);
		BOOL          __fastcall CopyMoveMacro(int Op);
		void          MoveTildeInKey(TStrList *&NameList,BOOL doit=FALSE);
		void          PrepareDependentSort(TStrList *&NameList,BOOL doit=FALSE);
		void          __fastcall FillMenu(HANDLE hDlg,int RebuildList=TRUE);
#ifndef UNICODE
		void          WriteRegValues(FarDialogItem *DialogItems);
#else
		void          WriteRegValues(FarDialogItem *DialogItems,HANDLE hDlg);
#endif
		BOOL          __fastcall CopyMacro(int vKey);
		void          __fastcall ExportMacro(BOOL AllMacros=FALSE);
		BOOL          __fastcall DeleteMacro();
		void          __fastcall SetFocus(int Focus);
		BOOL          __fastcall InsertMacro();
		BOOL          __fastcall EditMacro();
		void          __fastcall ReadConsoleSize();

	public:
		TMacroView();
		~TMacroView();

	public:
		BOOL          __fastcall Configure();
		void          __fastcall ReadConfig();
		int           MacroList();
};

TReg *Reg=NULL;
TMacroView *Macro=NULL;

#define MACRO_INCLUDED
#endif
