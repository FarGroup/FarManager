#ifndef __PLUGIN_HPP__
#define __PLUGIN_HPP__
/*
  PLUGIN.HPP

  Plugin API for FAR Manager 1.70

*/
/* Revision: 1.26 23.08.2000 $ */

/*
Modify:
  23.08.2000 SVS
    ! Уточнения категорий DMSG_* -> DM_ (месаг) & DN_ (нотифи)
    + DM_KEY        - послать/получить клавишу(ы)
    + DM_GETDLGDATA - взять данные диалога.
    + DM_SETDLGDATA - установить данные диалога.
    + DM_SHOWDIALOG - показать/спрятать диалог
    ! Все Flags приведены к одному виду -> DWORD.
      Модифицированы:
        * функции   FarMenuFn, FarMessageFn, FarShowHelp
        * структуры FarListItem, FarDialogItem
  22.08.2000 SVS
    ! DMSG_PAINT -> DMSG_DRAWDIALOG
    ! DMSG_DRAWITEM -> DMSG_DRAWDLGITEM
    ! DMSG_CHANGELIST -> DMSG_LISTCHANGE
  21.08.2000 SVS 1.23
    ! DMSG_CHANGEITEM -> DMSG_EDITCHANGE
    + DMSG_BTNCLICK
  18.08.2000 tran
    + Flags in ShowHelp
  12.08.2000 KM 1.22
    + DIF_MASKEDIT - новый флаг, реализующий функциональность ввода
      по маске в строках ввода.
    ! В структуре FarDialogItem новое поле, включенное в union, char *Mask
  17.08.2000 SVS
    ! struct FarListItems -> struct FarList, а то совсем запутался :-)
    + Сообщения диалога: DMSG_ENABLEREDRAW, DMSG_MOUSECLICK,
    + Флаг для DI_BUTTON - DIF_BTNNOCLOSE - "кнопка не для закрытия диалога"
  17.08.2000 SVS
    ! Изменение номера весрии :-)
  09.08.2000 SVS
    + FIB_NOUSELASTHISTORY - флаг для использовании пред значения из
      истории задается отдельно!!!
  09.08.2000 tran
    + #define CONSOLE_*
  04.08.2000 SVS
    + ECTL_SETKEYBAR - функция установки KeyBar Labels в редакторе
  04.08.2000 SVS
    + FarListItems.CountItems -> FarListItems.ItemsNumber
  03.08.2000 SVS
    + Функция от AT: GetMinFarVersion
  03.08.2000 SVS
    + ACTL_GETSYSWORDDIV получить строку с символами разделителями слов
  02.08.2000 SVS
    + Дополнения для KeyBarTitles:
        CtrlShiftTitles
        AltShiftTitles,
        CtrlAltTitles
    + Добавка в OpenPluginInfo для того, чтобы различить FAR <= 1.65 и > 1.65
  01.08.2000 SVS
    ! Функция ввода строки имеет один параметр для всех флагов
    ! дополнительный параметра у KeyToText - размер данных
    + Флаг DIF_USELASTHISTORY для строк ввода.
      если у строки ввода есть история то начальное значение брать первым
      из истории
    ! Полная переделка структуры списка и "оболочка" вокруг списка
    + флаги для FarListItem.Flags
      LIF_SELECTED, LIF_CHECKED, LIF_SEPARATOR
    + Сообщения для обработки диалога, имееющий место быть :-)
      DMSG_SETDLGITEM, DMSG_CHANGELIST
    ! Изменено наименование типа функции обработчика на универсальное
      FARDIALOGPROC -> FARWINDOWPROC
  28.07.2000 SVS
    + Введен новый элемент DI_LISTBOX (зарезервировано место)
    + Сообщения для обработки диалога, имееющий место быть :-)
        DMSG_INITDIALOG, DMSG_ENTERIDLE, DMSG_HELP, DMSG_PAINT,
        DMSG_SETREDRAW, DMSG_DRAWITEM, DMSG_GETDLGITEM, DMSG_KILLFOCUS,
        DMSG_GOTFOCUS, DMSG_SETFOCUS, DMSG_GETTEXTLENGTH, DMSG_GETTEXT,
        DMSG_CTLCOLORDIALOG, DMSG_CTLCOLORDLGITEM, DMSG_CTLCOLORDLGLIST,
        DMSG_SETTEXTLENGTH, DMSG_SETTEXT, DMSG_CHANGEITEM, DMSG_HOTKEY,
        DMSG_CLOSE,
  25.07.2000 SVS
    ! Некоторое упорядочение в FarStandardFunctions
    + Программое переключение FulScreen <-> Windowed (ACTL_CONSOLEMODE)
    + FSF-функция KeyToText
    ! WINAPI для сервисных дополнительных функций
    + Функция-диалог ввода тестовой строки InputBox
  23.07.2000 SVS
    + DialogEx, SendDlgMessage, DefDlgProc,
    ! WINAPI для сервисных дополнительных функций
  18.07.2000 SVS
    + Введен новый элемент: DI_COMBOBOX и флаг DIF_DROPDOWNLIST
      (для нередактируемого DI_COMBOBOX - пока не реализовано!)
  12.07.2000 IS
    + Флаги  редактора:
      EF_NONMODAL - открытие немодального редактора
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  10.07.2000 IS
    ! Некоторые изменения с учетом голого C (по совету SVS)
  07.07.2000 IS
    + Указатели на функции в FarStandardFunctions:
      atoi, _atoi64, itoa, RemoveLeadingSpaces, RemoveTrailingSpaces,
      RemoveExternalSpaces, TruncStr, TruncPathStr, QuoteSpaceOnly,
      PointToName, GetPathRoot, AddEndSlash
  06.07.2000 IS
    + Функция AdvControl (PluginStartupInfo)
    + Команда ACTL_GETFARVERSION для AdvControl
    + Указатель на структуру FarStandardFunctions в PluginStartupInfo - она
      содержит указатели на полезные функции. Плагин должен обязательно
      скопировать ее себе, если хочет использовать в дальнейшем.
    + Указатели на функции в FarStandardFunctions:
      Unquote, ExpandEnvironmentStr,
      sprintf, sscanf, qsort, memcpy, memmove, memcmp, strchr, strrchr, strstr,
      strtok, memset, strpbrk
  05.06.2000 SVS
    + DI_EDIT имеет флаг DIF_EDITEXPAND - расширение переменных среды
      в enum FarDialogItemFlags
  03.07.2000 IS
    + Функция вывода помощи в api
  28.06.2000 SVS
    + Для MSVC тоже требуется extern "C" при декларации
      экспортируемых функций + коррекция на Borland C++ 5.5
  26.06.2000 SVS
    ! Подготовка Master Copy
*/

#if defined(__BORLANDC__) && (__BORLANDC__ <= 0x550)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
  #if defined(__LCC__)
    #define _export __declspec(dllexport)
  #endif
#else
  #pragma pack(push,1)
  #if _MSC_VER
    #define _export
  #endif
#endif

#define NM 260

struct PluginPanelItem
{
  WIN32_FIND_DATA FindData;
  DWORD           PackSizeHigh;
  DWORD           PackSize;
  DWORD           Flags;
  DWORD           NumberOfLinks;
  char           *Description;
  char           *Owner;
  char          **CustomColumnData;
  int             CustomColumnNumber;
  DWORD           UserData;
  DWORD           Reserved[3];
};

#define PPIF_PROCESSDESCR 0x80000000
#define PPIF_SELECTED     0x40000000
#define PPIF_USERDATA     0x20000000

enum {
  FMENU_SHOWAMPERSAND=1,
  FMENU_WRAPMODE=2,
  FMENU_AUTOHIGHLIGHT=4,
  FMENU_REVERSEAUTOHIGHLIGHT=8
};


typedef int (WINAPI *FARAPIMENU)(
  int                 PluginNumber,
  int                 X,
  int                 Y,
  int                 MaxHeight,
  DWORD               Flags,
  char               *Title,
  char               *Bottom,
  char               *HelpTopic,
  int                *BreakKeys,
  int                *BreakCode,
  struct FarMenuItem *Item,
  int                 ItemsNumber
);

/* $ 23.07.2000 SVS
   Для обработчика диалога
*/
// тип функции обработчика окна
/* $ 01.08.2000 SVS
   Изменено наименование типа функции обработчика на универсальное
*/
typedef long (WINAPI *FARWINDOWPROC)(
  HANDLE hDlg,
  int    Msg,
  int    Param1,
  long   Param2
);

// обмен сообщениями с обработчиком диалога
typedef long (WINAPI *FARAPISENDDLGMESSAGE)(
  HANDLE hDlg,
  int    Msg,
  int    Param1,
  long   Param2
);

typedef long (WINAPI *FARAPIDEFDLGPROC)(
  HANDLE hDlg,
  int    Msg,
  int    Param1,
  long   Param2
);
/* SVS $ */

typedef int (WINAPI *FARAPIDIALOG)(
  int                   PluginNumber,
  int                   X1,
  int                   Y1,
  int                   X2,
  int                   Y2,
  char                 *HelpTopic,
  struct FarDialogItem *Item,
  int                   ItemsNumber
);
/* $ 25.07.2000 SVS
   Дополнительный параметр Param, который будет передан
   в обработчик диалога в WM_INITDIALOG
*/
typedef int (WINAPI *FARAPIDIALOGEX)(
  int                   PluginNumber,
  int                   X1,
  int                   Y1,
  int                   X2,
  int                   Y2,
  char                 *HelpTopic,
  struct FarDialogItem *Item,
  int                   ItemsNumber,
  FARWINDOWPROC         DlgProc,
  long                  Param
);
/* SVS $ */

enum {
  FMSG_WARNING=1,
  FMSG_ERRORTYPE=2,
  FMSG_KEEPBACKGROUND=4,
  FMSG_DOWN=8,
  FMSG_LEFTALIGN=16
};

typedef int (WINAPI *FARAPIMESSAGE)(
  int PluginNumber,
  DWORD Flags,
  char *HelpTopic,
  char **Items,
  int ItemsNumber,
  int ButtonsNumber
);

typedef char* (WINAPI *FARAPIGETMSG)(
  int PluginNumber,
  int MsgId
);

/* $ 18.07.2000 SVS
  + Введены новые элементы (зарезервированы!)
    DI_COMBOBOX и флаг DIF_DROPDOWNLIST
*/
/* $ 28.07.2000 SVS
  + Введен новый элемент DI_LISTBOX (пока зарезервирован!!!)
*/
enum DialogItemTypes {
  DI_TEXT,
  DI_VTEXT,
  DI_SINGLEBOX,
  DI_DOUBLEBOX,
  DI_EDIT,
  DI_PSWEDIT,
  DI_FIXEDIT,
  DI_BUTTON,
  DI_CHECKBOX,
  DI_RADIOBUTTON,
  DI_COMBOBOX,
  DI_LISTBOX,
};
/* 28.07.2000 SVS $*/

/* $ 12.08.2000 KM
   Новый флаг DIF_MASKEDIT, который добавляет
   в строку ввода возможность ввода данных по
   заданной маске.
*/
enum FarDialogItemFlags {
  DIF_COLORMASK       =    0xff,
  DIF_SETCOLOR        =   0x100,
  DIF_BOXCOLOR        =   0x200,
  DIF_GROUP           =   0x400,
  DIF_LEFTTEXT        =   0x800,
  DIF_MOVESELECT      =  0x1000,
  DIF_SHOWAMPERSAND   =  0x2000,
  DIF_CENTERGROUP     =  0x4000,
  DIF_NOBRACKETS      =  0x8000,
  DIF_SEPARATOR       = 0x10000,
  DIF_EDITOR          = 0x20000,
  DIF_HISTORY         = 0x40000,
  DIF_EDITEXPAND      = 0x80000,
  DIF_DROPDOWNLIST    =0x100000,
/* $ 01.08.2000 SVS
  если у строки ввода есть история
  то начальное значение брать первым из истории
*/
  DIF_USELASTHISTORY  =0x200000,
/* SVS $ */
/* $ 17.08.2000 SVS
  + Флаг для DI_BUTTON - DIF_BTNNOCLOSE - "кнопка не для закрытия диалога"
*/
  DIF_BTNNOCLOSE      = 0x40000,
/* SVS $ */
  DIF_MASKEDIT        =0x400000,
};
/* KM $ */
/* SVS $ */


/* $ 28.07.2000 SVS
   Сообщения и события для обработчиков для Dialog API
*/
enum FarMessagesProc{
  DM_FIRST=0,
  DM_CLOSE,
  DM_ENABLEREDRAW,
  DM_GETDLGDATA,
  DM_GETDLGITEM,
  DM_GETTEXT,
  DM_GETTEXTLENGTH,
  DM_KEY,
  DM_SETDLGDATA,
  DM_SETDLGITEM,
  DM_SETFOCUS,
  DM_SETREDRAW,
  DM_SETTEXT,
  DM_SETTEXTLENGTH,
  DM_SHOWDIALOG,

  DN_FIRST=1000,
  DN_BTNCLICK,
  DN_CTLCOLORDIALOG,
  DN_CTLCOLORDLGITEM,
  DN_CTLCOLORDLGLIST,
  DN_DRAWDIALOG,
  DN_DRAWDLGITEM,
  DN_EDITCHANGE,
  DN_ENTERIDLE,
  DN_GOTFOCUS,
  DN_HELP,
  DN_HOTKEY,
  DN_INITDIALOG,
  DN_KILLFOCUS,
  DN_LISTCHANGE,
  DN_MOUSECLICK,
};

/* SVS $*/

/* $ 18.07.2000 SVS
   Список Items для DI_COMBOBOX & DI_LISTBOX
*/
/* $ 01.08.2000 SVS
   Полная переделка структуры списка и "оболочка" вокруг списка
*/

// флаги для FarListItem.Flags
enum LISTITEMFLAGS{
  LIF_SELECTED = 0x0001,
  LIF_CHECKED  = 0x0002,
  LIF_SEPARATOR= 0x0004,
};

// описывает один пункт списка
struct FarListItem
{
  DWORD Flags;       // набор флагов, описывающих состояние пункта
  char Text[124];   // данные пункта
};

// весь список
struct FarList
{
  /* $ 04.08.2000 SVS
    + FarListItems.CountItems -> FarListItems.ItemsNumber
  */
  int ItemsNumber;
  /* SVS $*/
  struct FarListItem *Items;
};

/* 01.08.2000 SVS $ */
/* SVS $ */


/* $ 12.08.2000 KM
   Добавлено поле char *Mask в union, в котором
   будет содержаться маска ввода, если тип будет
   DI_FIXEDIT и флаг будет DIF_MASKEDIT
*/
struct FarDialogItem
{
  int Type;
  int X1,Y1,X2,Y2;
  int Focus;
  union {
    int Selected;
    char *History;
    char *Mask;
    struct FarList *ListItems;
  };
  DWORD Flags;
  int DefaultButton;
  char Data[512];
};
/* KM $ */

struct FarMenuItem
{
  char Text[128];
  int Selected;
  int Checked;
  int Separator;
};


enum {FCTL_CLOSEPLUGIN,FCTL_GETPANELINFO,FCTL_GETANOTHERPANELINFO,
      FCTL_UPDATEPANEL,FCTL_UPDATEANOTHERPANEL,
      FCTL_REDRAWPANEL,FCTL_REDRAWANOTHERPANEL,
      FCTL_SETANOTHERPANELDIR,FCTL_GETCMDLINE,FCTL_SETCMDLINE,
      FCTL_SETSELECTION,FCTL_SETANOTHERSELECTION,
      FCTL_SETVIEWMODE,FCTL_SETANOTHERVIEWMODE,FCTL_INSERTCMDLINE,
      FCTL_SETUSERSCREEN,FCTL_SETPANELDIR,FCTL_SETCMDLINEPOS,
      FCTL_GETCMDLINEPOS
};

/* $ 06.07.2000 IS
  Для AdvControl
*/
enum {
  ACTL_GETFARVERSION,
  ACTL_CONSOLEMODE,
  ACTL_GETSYSWORDDIV, // получить строку с символами разделителями слов
};
/* IS $ */

/* $ 09.08.2000 tran
   константы для ACTL_CONSOLE_MODE */
#define CONSOLE_GET_MODE       (-2)
#define CONSOLE_TRIGGER        (-1)
#define CONSOLE_SET_WINDOWED   (0)
#define CONSOLE_SET_FULLSCREEN (1)
#define CONSOLE_WINDOWED       (0)
#define CONSOLE_FULLSCREEN     (1)
/* tran 09.08.2000 $ */

enum {PTYPE_FILEPANEL,PTYPE_TREEPANEL,PTYPE_QVIEWPANEL,PTYPE_INFOPANEL};

struct PanelInfo
{
  int PanelType;
  int Plugin;
  RECT PanelRect;
  struct PluginPanelItem *PanelItems;
  int ItemsNumber;
  struct PluginPanelItem *SelectedItems;
  int SelectedItemsNumber;
  int CurrentItem;
  int TopPanelItem;
  int Visible;
  int Focus;
  int ViewMode;
  char ColumnTypes[80];
  char ColumnWidths[80];
  char CurDir[NM];
  int ShortNames;
  int SortMode;
  DWORD Reserved[2];
};


struct PanelRedrawInfo
{
  int CurrentItem;
  int TopPanelItem;
};


typedef int (WINAPI *FARAPICONTROL)(
  HANDLE hPlugin,
  int Command,
  void *Param
);

typedef HANDLE (WINAPI *FARAPISAVESCREEN)(int X1,int Y1,int X2,int Y2);

typedef void (WINAPI *FARAPIRESTORESCREEN)(HANDLE hScreen);

typedef int (WINAPI *FARAPIGETDIRLIST)(
  char *Dir,
  struct PluginPanelItem **pPanelItem,
  int *pItemsNumber
);

typedef int (WINAPI *FARAPIGETPLUGINDIRLIST)(
  int PluginNumber,
  HANDLE hPlugin,
  char *Dir,
  struct PluginPanelItem **pPanelItem,
  int *pItemsNumber
);

typedef void (WINAPI *FARAPIFREEDIRLIST)(struct PluginPanelItem *PanelItem);

enum VIEWER_FLAGS {
  VF_NONMODAL=1,VF_DELETEONCLOSE=2
};

/* $ 12.07.2000 IS
  Флаги редактора:
  EF_NONMODAL - открытие немодального редактора
*/
enum EDITOR_FLAGS {
  EF_NONMODAL=1
};
/* IS $ */

typedef int (WINAPI *FARAPIVIEWER)(
  char *FileName,
  char *Title,
  int X1,
  int Y1,
  int X2,
  int Y2,
  DWORD Flags
);

typedef int (WINAPI *FARAPIEDITOR)(
  char *FileName,
  char *Title,
  int X1,
  int Y1,
  int X2,
  int Y2,
  DWORD Flags,
  int StartLine,
  int StartChar
);

typedef int (WINAPI *FARAPICMPNAME)(
  char *Pattern,
  char *String,
  int SkipPath
);


#define FCT_DETECT 0x40000000

struct CharTableSet
{
  char DecodeTable[256];
  char EncodeTable[256];
  char UpperTable[256];
  char LowerTable[256];
  char TableName[128];
};

typedef int (WINAPI *FARAPICHARTABLE)(
  int Command,
  char *Buffer,
  int BufferSize
);

typedef void (WINAPI *FARAPITEXT)(
  int X,
  int Y,
  int Color,
  char *Str
);


typedef int (WINAPI *FARAPIEDITORCONTROL)(
  int Command,
  void *Param
);

/* $ 18.08.2000 tran
  add Flags parameter for future */
/* $ 03.07.2000 IS
   Функция вывода помощи
  */
typedef void (WINAPI *FARAPISHOWHELP)(
  char   *ModuleName,
  char   *HelpTopic,
  DWORD   Flags
);
/* IS $ */
/* tran 18.08.2000 $ */

/* $ 06.07.2000 IS
   Функция AdvControl
  */
typedef int (WINAPI *FARAPIADVCONTROL)(
  int ModuleNumber,
  int Command,
  void *Param
);
/* IS $ */

enum EDITOR_EVENTS {
  EE_READ,EE_SAVE,EE_REDRAW,EE_CLOSE
};

/* $ 07.08.2000 SVS
   + ECTL_SETKEYBAR - управление метками в редакторе
*/
enum EDITOR_CONTROL_COMMANDS {
  ECTL_GETSTRING,ECTL_SETSTRING,ECTL_INSERTSTRING,ECTL_DELETESTRING,
  ECTL_DELETECHAR,ECTL_INSERTTEXT,ECTL_GETINFO,ECTL_SETPOSITION,
  ECTL_SELECT,ECTL_REDRAW,ECTL_EDITORTOOEM,ECTL_OEMTOEDITOR,
  ECTL_TABTOREAL,ECTL_REALTOTAB,ECTL_EXPANDTABS,ECTL_SETTITLE,
  ECTL_READINPUT,ECTL_PROCESSINPUT,ECTL_ADDCOLOR,ECTL_GETCOLOR,
  ECTL_SAVEFILE,ECTL_QUIT,ECTL_SETKEYBAR
};
/* SVS $*/

struct EditorGetString
{
  int StringNumber;
  char *StringText;
  char *StringEOL;
  int StringLength;
  int SelStart;
  int SelEnd;
};


struct EditorSetString
{
  int StringNumber;
  char *StringText;
  char *StringEOL;
  int StringLength;
};


enum EDITOR_OPTIONS {
  EOPT_EXPANDTABS=1,EOPT_PERSISTENTBLOCKS=2,EOPT_DELREMOVESBLOCKS=4,
  EOPT_AUTOINDENT=8,EOPT_SAVEFILEPOSITION=16,EOPT_AUTODETECTTABLE=32,
  EOPT_CURSORBEYONDEOL=64
};


enum EDITOR_BLOCK_TYPES {
  BTYPE_NONE,BTYPE_STREAM,BTYPE_COLUMN
};


struct EditorInfo
{
  int EditorID;
  char *FileName;
  int WindowSizeX;
  int WindowSizeY;
  int TotalLines;
  int CurLine;
  int CurPos;
  int CurTabPos;
  int TopScreenLine;
  int LeftPos;
  int Overtype;
  int BlockType;
  int BlockStartLine;
  int AnsiMode;
  int TableNum;
  DWORD Options;
  int TabSize;
  DWORD Reserved[8];
};


struct EditorSetPosition
{
  int CurLine;
  int CurPos;
  int CurTabPos;
  int TopScreenLine;
  int LeftPos;
  int Overtype;
};


struct EditorSelect
{
  int BlockType;
  int BlockStartLine;
  int BlockStartPos;
  int BlockWidth;
  int BlockHeight;
};


struct EditorConvertText
{
  char *Text;
  int TextLength;
};


struct EditorConvertPos
{
  int StringNumber;
  int SrcPos;
  int DestPos;
};


struct EditorColor
{
  int StringNumber;
  int ColorItem;
  int StartPos;
  int EndPos;
  int Color;
};

struct EditorSaveFile
{
  char FileName[NM];
  char *FileEOL;
};

/* $ 06.07.2000 IS
  Убирает ВСЕ начальные и заключительные кавычки
*/
typedef void (WINAPI *FARSTDUNQUOTE)(char *Str);
/* IS $ */

/* $ 06.07.2000 IS
  Расширение строки с учетом переменных окружения
*/
typedef DWORD (WINAPI *FARSTDEXPANDENVIRONMENTSTR)(
  char *src,
  char *dst,
  size_t size
);
/* IS $ */


/* $ 01.08.2000 SVS
   Функция ввода строки имеет один параметр для всех флагов
*/
enum INPUTBOXFLAGS{
  FIB_ENABLEEMPTY      = 0x0001,
  FIB_PASSWORD         = 0x0002,
  FIB_EXPANDENV        = 0x0004,
/* $ 09.08.2000 SVS
   если не нужно пред значение - ставим этот флаг
*/
  FIB_NOUSELASTHISTORY = 0x0008,
/* SVS $ */
};

/* $ 25.07.2000 SVS
   Функция ввода строки
*/
typedef int (WINAPI *FARAPIINPUTBOX)(
  char *Title,
  char *SubTitle,
  char *HistoryName,
  char *SrcText,
  char *DestText,
  int   DestLength,
  char *HelpTopic,
  DWORD Flags
);
/* SVS $*/
/* 01.08.2000 SVS $*/

/* $ 06.07.2000 IS
  А это что? Отгадайте сами :-)
  sprintf, sscanf, qsort, memcpy, memmove, memcmp, strchr, strrchr, strstr,
  strtok, memset, strpbrk
*/
typedef int   (WINAPIV *FARSTDSPRINTF)(char *buffer,const char *format,...);
typedef int   (WINAPIV *FARSTDSSCANF)(const char *s, const char *format,...);
typedef void  (WINAPI *FARSTDQSORT)(void *base, size_t nelem, size_t width, int (WINAPIV *fcmp)(const void *, const void *));

// Only C/C++!!!
typedef void *(*FARSTDMEMCPY)(void *dest, const void *src, size_t n);
typedef void *(*FARSTDMEMMOVE)(void *dest, const void *src, size_t n);
typedef int   (*FARSTDMEMCMP)(const void *s1, const void *s2, size_t n);
#if defined(__BORLANDC__)
typedef char *(*FARSTDSTRCHR)(char *s, int c);
typedef char *(*FARSTDSTRRCHR)(char *s, int c);
typedef char *(*FARSTDSTRSTR)(char *s1, const char *s2);
typedef char *(*FARSTDSTRTOK)(char *s1, const char *s2);
typedef char *(*FARSTDSTRPBRK)(char *s1, const char *s2);
#else
typedef char *(*FARSTDSTRCHR)(const char *s, int c);
typedef char *(*FARSTDSTRRCHR)(const char *s, int c);
typedef char *(*FARSTDSTRSTR)(const char *s1, const char *s2);
typedef char *(*FARSTDSTRTOK)(char *s1, const char *s2);
typedef char *(*FARSTDSTRPBRK)(const char *s1, const char *s2);
#endif
typedef void *(*FARSTDMEMSET)(void *s, int c, size_t n);
// End Only C/C++!!!
/* IS $ */

/* $ 07.07.2000 IS
  Эпопея продолжается :-)
  atoi, _atoi64, itoa, RemoveLeadingSpaces, RemoveTrailingSpaces,
  RemoveExternalSpaces, TruncStr, TruncPathStr, QuoteSpaceOnly,
  PointToName, GetPathRoot, AddEndSlash
*/
typedef int (WINAPI *FARSTDATOI)(const char *s);
typedef __int64 (WINAPI *FARSTDATOI64)(const char *s);
typedef char *(WINAPI *FARSTDITOA)(int value, char *string, int radix);
typedef char *(WINAPI *FARSTDREMOVELEADINGSPACES)(char *Str);
typedef char *(WINAPI *FARSTDREMOVETRAILINGSPACES)(char *Str);
typedef char *(WINAPI *FARSTDREMOVEEXTERNALSPACES)(char *Str);
typedef char *(WINAPI *FARSTDTRUNCSTR)(char *Str,int MaxLength);
typedef char *(WINAPI *FARSTDTRUNCPATHSTR)(char *Str,int MaxLength);
typedef char *(WINAPI *FARSTDQUOTESPACEONLY)(char *Str);
typedef char *(WINAPI *FARSTDPOINTTONAME)(char *Path);
typedef void  (WINAPI *FARSTDGETPATHROOT)(char *Path,char *Root);
typedef void  (WINAPI *FARSTDADDENDSLASH)(char *Path);
/* IS $ */
/* $ 25.07.2000 SVS
   Дополнительные функции
*/
typedef int (WINAPI *FARSTDCOPYTOCLIPBOARD)(char *Data);
typedef char* (WINAPI *FARSTDPASTEFROMCLIPBOARD)(void);
/* $ 01.08.2000 SVS
   ! дополнительный параметра у KeyToText - размер данных
*/
typedef void (WINAPI *FARSTDKEYTOTEXT)(int Key,char *KeyText,int Size);
/* 01.08.2000 SVS $*/

/* SVS $*/
/* $ 06.07.2000 IS
   Полезные функции из far.exe
*/
/* $ 07.07.2000 IS
   По просьбам трудящихся...
  atoi, _atoi64;  itoa;  RemoveLeadingSpaces;  RemoveTrailingSpaces;
 RemoveExternalSpaces;  TruncStr;  TruncPathStr;  QuoteSpaceOnly;
 PointToName;  GetPathRoot;  AddEndSlash;
*/
typedef struct FarStandardFunctions
{
  int StructSize;

  FARSTDATOI    atoi;
  FARSTDATOI64  _atoi64;
  FARSTDITOA    itoa;
  FARSTDSPRINTF sprintf;
  FARSTDSSCANF  sscanf;
  // Only C/C++!!!
  FARSTDMEMSET  memset;
  FARSTDMEMCPY  memcpy;
  FARSTDMEMMOVE memmove;
  FARSTDMEMCMP  memcmp;
  FARSTDSTRCHR  strchr;
  FARSTDSTRRCHR strrchr;
  FARSTDSTRSTR  strstr;
  FARSTDSTRTOK  strtok;
  FARSTDSTRPBRK strpbrk;
  // End Only C/C++!!!
  FARSTDQSORT qsort;

  FARSTDUNQUOTE Unquote;
  FARSTDEXPANDENVIRONMENTSTR ExpandEnvironmentStr;
  FARSTDREMOVELEADINGSPACES RemoveLeadingSpaces;
  FARSTDREMOVETRAILINGSPACES RemoveTrailingSpaces;
  FARSTDREMOVEEXTERNALSPACES RemoveExternalSpaces;
  FARSTDTRUNCSTR TruncStr;
  FARSTDTRUNCPATHSTR TruncPathStr;
  FARSTDQUOTESPACEONLY QuoteSpaceOnly;
  FARSTDPOINTTONAME PointToName;
  FARSTDGETPATHROOT GetPathRoot;
  FARSTDADDENDSLASH AddEndSlash;
  FARSTDCOPYTOCLIPBOARD CopyToClipboard;
  FARSTDPASTEFROMCLIPBOARD PasteFromClipboard;
  FARSTDKEYTOTEXT FarKeyToText;
  /* IS $ */
}FARSTANDARDFUNCTIONS;
/* IS $ */

struct PluginStartupInfo
{
  int StructSize;
  char ModuleName[NM];
  int ModuleNumber;
  char *RootKey;
  FARAPIMENU Menu;
  FARAPIDIALOG Dialog;
  FARAPIMESSAGE Message;
  FARAPIGETMSG GetMsg;
  FARAPICONTROL Control;
  FARAPISAVESCREEN SaveScreen;
  FARAPIRESTORESCREEN RestoreScreen;
  FARAPIGETDIRLIST GetDirList;
  FARAPIGETPLUGINDIRLIST GetPluginDirList;
  FARAPIFREEDIRLIST FreeDirList;
  FARAPIVIEWER Viewer;
  FARAPIEDITOR Editor;
  FARAPICMPNAME CmpName;
  FARAPICHARTABLE CharTable;
  FARAPITEXT Text;
  FARAPIEDITORCONTROL EditorControl;
  /* $ 06.07.2000 IS
     Функция, которая будет действовать и в редакторе, и в панелях, и...
  */
  FARAPIADVCONTROL AdvControl;
  /* IS $ */
  /* $ 23.07.2000 SVS
     Функции для обработчика диалога
       - обмен сообщениями
       - функция по умолчанию
       - функция получения строки
       - функция переключения FulScreen <-> Windowed
  */
  FARAPIDIALOGEX DialogEx;
  FARAPISENDDLGMESSAGE SendDlgMessage;
  FARAPIDEFDLGPROC DefDlgProc;
  FARAPIINPUTBOX InputBox;
  /* SVS $ */
  /* $ 03.07.2000 IS
     Функция вывода помощи
  */
  FARAPISHOWHELP ShowHelp;
  /* IS $ */
  /* $ 06.07.2000 IS
     Указатель на структуру с адресами полезных функций из far.exe
  */
  FARSTANDARDFUNCTIONS *FSF;
  /* IS $ */
};


enum PLUGIN_FLAGS {
  PF_PRELOAD        = 0x0001,
  PF_DISABLEPANELS  = 0x0002,
  PF_EDITOR         = 0x0004,
  PF_VIEWER         = 0x0008
};


struct PluginInfo
{
  int StructSize;
  DWORD Flags;
  char **DiskMenuStrings;
  int *DiskMenuNumbers;
  int DiskMenuStringsNumber;
  char **PluginMenuStrings;
  int PluginMenuStringsNumber;
  char **PluginConfigStrings;
  int PluginConfigStringsNumber;
  char *CommandPrefix;
};


struct InfoPanelLine
{
  char Text[80];
  char Data[80];
  int Separator;
};


struct PanelMode
{
  char *ColumnTypes;
  char *ColumnWidths;
  char **ColumnTitles;
  int FullScreen;
  int DetailedStatus;
  int AlignExtensions;
  int CaseConversion;
  char *StatusColumnTypes;
  char *StatusColumnWidths;
  DWORD Reserved[2];
};


enum OPENPLUGININFO_FLAGS {
  OPIF_USEFILTER               = 0x0001,
  OPIF_USESORTGROUPS           = 0x0002,
  OPIF_USEHIGHLIGHTING         = 0x0004,
  OPIF_ADDDOTS                 = 0x0008,
  OPIF_RAWSELECTION            = 0x0010,
  OPIF_REALNAMES               = 0x0020,
  OPIF_SHOWNAMESONLY           = 0x0040,
  OPIF_SHOWRIGHTALIGNNAMES     = 0x0080,
  OPIF_SHOWPRESERVECASE        = 0x0100,
  OPIF_FINDFOLDERS             = 0x0200,
  OPIF_COMPAREFATTIME          = 0x0400,
  OPIF_EXTERNALGET             = 0x0800,
  OPIF_EXTERNALPUT             = 0x1000,
  OPIF_EXTERNALDELETE          = 0x2000,
  OPIF_EXTERNALMKDIR           = 0x4000,
  OPIF_USEATTRHIGHLIGHTING     = 0x8000
};


enum OPENPLUGININFO_SORTMODES {
  SM_DEFAULT,SM_UNSORTED,SM_NAME,SM_EXT,SM_MTIME,SM_CTIME,
  SM_ATIME,SM_SIZE,SM_DESCR,SM_OWNER,SM_COMPRESSEDSIZE,SM_NUMLINKS
};


struct KeyBarTitles
{
  char *Titles[12];
  char *CtrlTitles[12];
  char *AltTitles[12];
  char *ShiftTitles[12];
  /* $ 02.08.2000 SVS
     Дополнения!
  */
  char *CtrlShiftTitles[12];
  char *AltShiftTitles[12];
  char *CtrlAltTitles[12];
  /* SVS $ */
};


struct OpenPluginInfo
{
  int                   StructSize;
  DWORD                 Flags;
  char                 *HostFile;
  char                 *CurDir;
  char                 *Format;
  char                 *PanelTitle;
  struct InfoPanelLine *InfoLines;
  int                   InfoLinesNumber;
  char                **DescrFiles;
  int                   DescrFilesNumber;
  struct PanelMode     *PanelModesArray;
  int                   PanelModesNumber;
  int                   StartPanelMode;
  int                   StartSortMode;
  int                   StartSortOrder;
  struct KeyBarTitles  *KeyBar;
  char                 *ShortcutData;
  /* $ 02.08.2000 SVS
     + добавка, для того, чтобы различить FAR <= 1.65 и > 1.65
  */
  long                  Reserverd;
  /* SVS $*/
};

enum {
  OPEN_DISKMENU,
  OPEN_PLUGINSMENU,
  OPEN_FINDLIST,
  OPEN_SHORTCUT,
  OPEN_COMMANDLINE,
  OPEN_EDITOR,
  OPEN_VIEWER
};

enum {PKF_CONTROL=1,PKF_ALT=2,PKF_SHIFT=4};

enum FAR_EVENTS {
  FE_CHANGEVIEWMODE,
  FE_REDRAW,
  FE_IDLE,
  FE_CLOSE,
  FE_BREAK,
  FE_COMMAND
};

enum OPERATION_MODES {
  OPM_SILENT=1,
  OPM_FIND=2,
  OPM_VIEW=4,
  OPM_EDIT=8,
  OPM_TOPLEVEL=16,
  OPM_DESCR=32
};

#if defined(__BORLANDC__) || defined(_MSC_VER)
#ifdef __cplusplus
extern "C"{
#endif
// Exported Functions

int    WINAPI _export GetMinFarVersion(void);
void   WINAPI _export ClosePlugin(HANDLE hPlugin);
int    WINAPI _export Compare(HANDLE hPlugin,struct PluginPanelItem *Item1,struct PluginPanelItem *Item2,unsigned int Mode);
int    WINAPI _export Configure(int ItemNumber);
int    WINAPI _export DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
void   WINAPI _export ExitFAR(void);
void   WINAPI _export FreeFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
void   WINAPI _export FreeVirtualFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
int    WINAPI _export GetFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode);
int    WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
void   WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info);
void   WINAPI _export GetPluginInfo(struct PluginInfo *Info);
int    WINAPI _export GetVirtualFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,char *Path);
int    WINAPI _export MakeDirectory(HANDLE hPlugin,char *Name,int OpMode);
HANDLE WINAPI _export OpenFilePlugin(char *Name,const unsigned char *Data,int DataSize);
HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item);
int    WINAPI _export ProcessEvent(HANDLE hPlugin,int Event,void *Param);
int    WINAPI _export ProcessHostFile(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
int    WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState);
int    WINAPI _export PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
int    WINAPI _export SetDirectory(HANDLE hPlugin,char *Dir,int OpMode);
int    WINAPI _export SetFindList(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
void   WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info);
int    WINAPI _export ProcessEditorInput(INPUT_RECORD *Rec);
int    WINAPI _export ProcessEditorEvent(int Event,void *Param);

#ifdef __cplusplus
};
#endif
#endif

#if defined(__BORLANDC__) && (__BORLANDC__ <= 0x550)
  #pragma option -a.
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack()
#else
  #pragma pack(pop)
#endif

#endif /* __PLUGIN_HPP__ */
