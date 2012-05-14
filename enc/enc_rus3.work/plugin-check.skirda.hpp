#define FARMANAGERVERSION_MAJOR 3
#define FARMANAGERVERSION_MINOR 0
#define FARMANAGERVERSION_REVISION 0
#define FARMANAGERVERSION_BUILD 2672
#define FARMANAGERVERSION_STAGE VS_RELEASE

#define FARMACRO_KEY_EVENT  (KEY_EVENT|0x8000)

#define CP_UNICODE    1200
#define CP_REVERSEBOM 1201
#define CP_DEFAULT    ((UINT)-1)
#define CP_REDETECT   ((UINT)-2)

#define INDEXMASK 0x0000000f
#define COLORMASK 0x00ffffff
#define ALPHAMASK 0xff000000

#define INDEXVALUE(x) ((x)&INDEXMASK)
#define COLORVALUE(x) ((x)&COLORMASK)
#define ALPHAVALUE(x) ((x)&ALPHAMASK)

#define IS_OPAQUE(x) (ALPHAVALUE(x)==ALPHAMASK)
#define IS_TRANSPARENT(x) (!ALPHAVALUE(x))
#define MAKE_OPAQUE(x) (x|=ALPHAMASK)
#define MAKE_TRANSPARENT(x) (x&=COLORMASK)

typedef unsigned __int64 COLORDIALOGFLAGS;
static const COLORDIALOGFLAGS
    CDF_NONE = 0;

typedef BOOL (WINAPI *FARAPICOLORDIALOG)(
    const GUID* PluginId,
    COLORDIALOGFLAGS Flags,
    struct FarColor *Color
);

typedef unsigned __int64 FARMESSAGEFLAGS;
static const FARMESSAGEFLAGS
	FMSG_WARNING             = 0x0000000000000001ULL,
	FMSG_ERRORTYPE           = 0x0000000000000002ULL,
	FMSG_KEEPBACKGROUND      = 0x0000000000000004ULL,
	FMSG_LEFTALIGN           = 0x0000000000000008ULL,
	FMSG_ALLINONE            = 0x0000000000000010ULL,
	FMSG_MB_OK               = 0x0000000000010000ULL,
	FMSG_MB_OKCANCEL         = 0x0000000000020000ULL,
	FMSG_MB_ABORTRETRYIGNORE = 0x0000000000030000ULL,
	FMSG_MB_YESNO            = 0x0000000000040000ULL,
	FMSG_MB_YESNOCANCEL      = 0x0000000000050000ULL,
	FMSG_MB_RETRYCANCEL      = 0x0000000000060000ULL,
	FMSG_NONE                = 0;

typedef int (WINAPI *FARAPIMESSAGE)(
    const GUID* PluginId,
    const GUID* Id,
    FARMESSAGEFLAGS Flags,
    const wchar_t *HelpTopic,
    const wchar_t * const *Items,
    size_t ItemsNumber,
    int ButtonsNumber
);

enum FARDIALOGITEMTYPES
{
	DI_TEXT                         =  0,
	DI_VTEXT                        =  1,
	DI_SINGLEBOX                    =  2,
	DI_DOUBLEBOX                    =  3,
	DI_EDIT                         =  4,
	DI_PSWEDIT                      =  5,
	DI_FIXEDIT                      =  6,
	DI_BUTTON                       =  7,
	DI_CHECKBOX                     =  8,
	DI_RADIOBUTTON                  =  9,
	DI_COMBOBOX                     = 10,
	DI_LISTBOX                      = 11,

	DI_USERCONTROL                  =255,
};

/*
   Check diagol element type has inputstring?
   (DI_EDIT, DI_FIXEDIT, DI_PSWEDIT, etc)
*/
static __inline BOOL IsEdit(enum FARDIALOGITEMTYPES Type)
{
	switch (Type)
	{
		case DI_EDIT:
		case DI_FIXEDIT:
		case DI_PSWEDIT:
		case DI_COMBOBOX:
			return TRUE;
		default:
			return FALSE;
	}
}

typedef unsigned __int64 FARDIALOGITEMFLAGS;
static const FARDIALOGITEMFLAGS
	DIF_BOXCOLOR              = 0x0000000000000200ULL,
	DIF_GROUP                 = 0x0000000000000400ULL,
	DIF_LEFTTEXT              = 0x0000000000000800ULL,
	DIF_MOVESELECT            = 0x0000000000001000ULL,
	DIF_SHOWAMPERSAND         = 0x0000000000002000ULL,
	DIF_CENTERGROUP           = 0x0000000000004000ULL,
	DIF_NOBRACKETS            = 0x0000000000008000ULL,
	DIF_MANUALADDHISTORY      = 0x0000000000008000ULL,
	DIF_SEPARATOR             = 0x0000000000010000ULL,
	DIF_SEPARATOR2            = 0x0000000000020000ULL,
	DIF_EDITOR                = 0x0000000000020000ULL,
	DIF_LISTNOAMPERSAND       = 0x0000000000020000ULL,
	DIF_LISTNOBOX             = 0x0000000000040000ULL,
	DIF_HISTORY               = 0x0000000000040000ULL,
	DIF_BTNNOCLOSE            = 0x0000000000040000ULL,
	DIF_CENTERTEXT            = 0x0000000000040000ULL,
	DIF_SETSHIELD             = 0x0000000000080000ULL,
	DIF_EDITEXPAND            = 0x0000000000080000ULL,
	DIF_DROPDOWNLIST          = 0x0000000000100000ULL,
	DIF_USELASTHISTORY        = 0x0000000000200000ULL,
	DIF_MASKEDIT              = 0x0000000000400000ULL,
	DIF_LISTTRACKMOUSE        = 0x0000000000400000ULL,
	DIF_LISTTRACKMOUSEINFOCUS = 0x0000000000800000ULL,
	DIF_SELECTONENTRY         = 0x0000000000800000ULL,
	DIF_3STATE                = 0x0000000000800000ULL,
	DIF_EDITPATH              = 0x0000000001000000ULL,
	DIF_LISTWRAPMODE          = 0x0000000001000000ULL,
	DIF_NOAUTOCOMPLETE        = 0x0000000002000000ULL,
	DIF_LISTAUTOHIGHLIGHT     = 0x0000000002000000ULL,
	DIF_LISTNOCLOSE           = 0x0000000004000000ULL,
	DIF_EDITPATHEXEC          = 0x0000000004000000ULL,
	DIF_HIDDEN                = 0x0000000010000000ULL,
	DIF_READONLY              = 0x0000000020000000ULL,
	DIF_NOFOCUS               = 0x0000000040000000ULL,
	DIF_DISABLE               = 0x0000000080000000ULL,
	DIF_DEFAULTBUTTON         = 0x0000000100000000ULL,
	DIF_FOCUS                 = 0x0000000200000000ULL,
	DIF_NONE                  = 0;

enum FARMESSAGE
{
	DM_FIRST                        = 0,
	DM_CLOSE                        = 1,
	DM_ENABLE                       = 2,
	DM_ENABLEREDRAW                 = 3,
	DM_GETDLGDATA                   = 4,
	DM_GETDLGITEM                   = 5,
	DM_GETDLGRECT                   = 6,
	DM_GETTEXT                      = 7,
	DM_GETTEXTLENGTH                = 8,
	DM_KEY                          = 9,
	DM_MOVEDIALOG                   = 10,
	DM_SETDLGDATA                   = 11,
	DM_SETDLGITEM                   = 12,
	DM_SETFOCUS                     = 13,
	DM_REDRAW                       = 14,
	DM_SETTEXT                      = 15,
	DM_SETMAXTEXTLENGTH             = 16,
	DM_SHOWDIALOG                   = 17,
	DM_GETFOCUS                     = 18,
	DM_GETCURSORPOS                 = 19,
	DM_SETCURSORPOS                 = 20,
	DM_GETTEXTPTR                   = 21,
	DM_SETTEXTPTR                   = 22,
	DM_SHOWITEM                     = 23,
	DM_ADDHISTORY                   = 24,

	DM_GETCHECK                     = 25,
	DM_SETCHECK                     = 26,
	DM_SET3STATE                    = 27,

	DM_LISTSORT                     = 28,
	DM_LISTGETITEM                  = 29,
	DM_LISTGETCURPOS                = 30,
	DM_LISTSETCURPOS                = 31,
	DM_LISTDELETE                   = 32,
	DM_LISTADD                      = 33,
	DM_LISTADDSTR                   = 34,
	DM_LISTUPDATE                   = 35,
	DM_LISTINSERT                   = 36,
	DM_LISTFINDSTRING               = 37,
	DM_LISTINFO                     = 38,
	DM_LISTGETDATA                  = 39,
	DM_LISTSETDATA                  = 40,
	DM_LISTSETTITLES                = 41,
	DM_LISTGETTITLES                = 42,

	DM_RESIZEDIALOG                 = 43,
	DM_SETITEMPOSITION              = 44,

	DM_GETDROPDOWNOPENED            = 45,
	DM_SETDROPDOWNOPENED            = 46,

	DM_SETHISTORY                   = 47,

	DM_GETITEMPOSITION              = 48,
	DM_SETMOUSEEVENTNOTIFY          = 49,

	DM_EDITUNCHANGEDFLAG            = 50,

	DM_GETITEMDATA                  = 51,
	DM_SETITEMDATA                  = 52,

	DM_LISTSET                      = 53,

	DM_GETCURSORSIZE                = 54,
	DM_SETCURSORSIZE                = 55,

	DM_LISTGETDATASIZE              = 56,

	DM_GETSELECTION                 = 57,
	DM_SETSELECTION                 = 58,

	DM_GETEDITPOSITION              = 59,
	DM_SETEDITPOSITION              = 60,

	DM_SETCOMBOBOXEVENT             = 61,
	DM_GETCOMBOBOXEVENT             = 62,

	DM_GETCONSTTEXTPTR              = 63,
	DM_GETDLGITEMSHORT              = 64,
	DM_SETDLGITEMSHORT              = 65,

	DM_GETDIALOGINFO                = 66,

	DN_FIRST                        = 4096,
	DN_BTNCLICK                     = 4097,
	DN_CTLCOLORDIALOG               = 4098,
	DN_CTLCOLORDLGITEM              = 4099,
	DN_CTLCOLORDLGLIST              = 4100,
	DN_DRAWDIALOG                   = 4101,
	DN_DRAWDLGITEM                  = 4102,
	DN_EDITCHANGE                   = 4103,
	DN_ENTERIDLE                    = 4104,
	DN_GOTFOCUS                     = 4105,
	DN_HELP                         = 4106,
	DN_HOTKEY                       = 4107,
	DN_INITDIALOG                   = 4108,
	DN_KILLFOCUS                    = 4109,
	DN_LISTCHANGE                   = 4110,
	DN_DRAGGED                      = 4111,
	DN_RESIZECONSOLE                = 4112,
	DN_DRAWDIALOGDONE               = 4113,
	DN_LISTHOTKEY                   = 4114,
	DN_INPUT                        = 4115,
	DN_CONTROLINPUT                 = 4116,
	DN_CLOSE                        = 4117,
	DN_GETVALUE                     = 4118,

	DM_USER                         = 0x4000,

};

enum FARCHECKEDSTATE
{
	BSTATE_UNCHECKED = 0,
	BSTATE_CHECKED   = 1,
	BSTATE_3STATE    = 2,
	BSTATE_TOGGLE    = 3,
};

enum FARCOMBOBOXEVENTTYPE
{
	CBET_KEY         = 0x00000001,
	CBET_MOUSE       = 0x00000002,
};

struct FarDialogItem
{
	enum FARDIALOGITEMTYPES Type;
	int X1,Y1,X2,Y2;
	union
	{
		int Selected;
		struct FarList *ListItems;
		struct FAR_CHAR_INFO *VBuf;
		DWORD_PTR Reserved;
	}
#ifndef __cplusplus
	Param
#endif
	;
	const wchar_t *History;
	const wchar_t *Mask;
	FARDIALOGITEMFLAGS Flags;
	const wchar_t *Data;
	size_t MaxLength; // terminate 0 not included (if == 0 string size is unlimited)
	DWORD_PTR UserData;
};

#define Dlg_RedrawDialog(Info,hDlg)            Info.SendDlgMessage(hDlg,DM_REDRAW,0,0)

#define Dlg_GetDlgData(Info,hDlg)              Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0)
#define Dlg_SetDlgData(Info,hDlg,Data)         Info.SendDlgMessage(hDlg,DM_SETDLGDATA,0,(INT_PTR)Data)

#define Dlg_GetDlgItemData(Info,hDlg,ID)       Info.SendDlgMessage(hDlg,DM_GETITEMDATA,0,0)
#define Dlg_SetDlgItemData(Info,hDlg,ID,Data)  Info.SendDlgMessage(hDlg,DM_SETITEMDATA,0,(INT_PTR)Data)

#define DlgItem_GetFocus(Info,hDlg)            Info.SendDlgMessage(hDlg,DM_GETFOCUS,0,0)
#define DlgItem_SetFocus(Info,hDlg,ID)         Info.SendDlgMessage(hDlg,DM_SETFOCUS,ID,0)
#define DlgItem_Enable(Info,hDlg,ID)           Info.SendDlgMessage(hDlg,DM_ENABLE,ID,TRUE)
#define DlgItem_Disable(Info,hDlg,ID)          Info.SendDlgMessage(hDlg,DM_ENABLE,ID,FALSE)
#define DlgItem_IsEnable(Info,hDlg,ID)         Info.SendDlgMessage(hDlg,DM_ENABLE,ID,-1)
#define DlgItem_SetText(Info,hDlg,ID,Str)      Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,ID,(INT_PTR)Str)

#define DlgItem_GetCheck(Info,hDlg,ID)         Info.SendDlgMessage(hDlg,DM_GETCHECK,ID,0)
#define DlgItem_SetCheck(Info,hDlg,ID,State)   Info.SendDlgMessage(hDlg,DM_SETCHECK,ID,State)

#define DlgEdit_AddHistory(Info,hDlg,ID,Str)   Info.SendDlgMessage(hDlg,DM_ADDHISTORY,ID,(INT_PTR)Str)

#define DlgList_AddString(Info,hDlg,ID,Str)    Info.SendDlgMessage(hDlg,DM_LISTADDSTR,ID,(INT_PTR)Str)
#define DlgList_GetCurPos(Info,hDlg,ID)        Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,ID,0)
#define DlgList_SetCurPos(Info,hDlg,ID,NewPos) {struct FarListPos LPos={sizeof(FarListPos),NewPos,-1};Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,ID,(INT_PTR)&LPos);}
#define DlgList_ClearList(Info,hDlg,ID)        Info.SendDlgMessage(hDlg,DM_LISTDELETE,ID,0)
#define DlgList_DeleteItem(Info,hDlg,ID,Index) {struct FarListDelete FLDItem={sizeof(FarListDelete),Index,1}; Info.SendDlgMessage(hDlg,DM_LISTDELETE,ID,(INT_PTR)&FLDItem);}
#define DlgList_SortUp(Info,hDlg,ID)           Info.SendDlgMessage(hDlg,DM_LISTSORT,ID,0)
#define DlgList_SortDown(Info,hDlg,ID)         Info.SendDlgMessage(hDlg,DM_LISTSORT,ID,1)
#define DlgList_GetItemData(Info,hDlg,ID,Index)          Info.SendDlgMessage(hDlg,DM_LISTGETDATA,ID,Index)
#define DlgList_SetItemStrAsData(Info,hDlg,ID,Index,Str) {struct FarListItemData FLID{sizeof(FarListItemData),Index,0,Str,0}; Info.SendDlgMessage(hDlg,DM_LISTSETDATA,ID,(INT_PTR)&FLID);}

typedef unsigned __int64 FARDIALOGFLAGS;
static const FARDIALOGFLAGS
	FDLG_WARNING             = 0x0000000000000001ULL,
	FDLG_SMALLDIALOG         = 0x0000000000000002ULL,
	FDLG_NODRAWSHADOW        = 0x0000000000000004ULL,
	FDLG_NODRAWPANEL         = 0x0000000000000008ULL,
	FDLG_KEEPCONSOLETITLE    = 0x0000000000000010ULL,
	FDLG_NONE                = 0;

typedef INT_PTR(WINAPI *FARWINDOWPROC)(
    HANDLE   hDlg,
    int Msg,
    int      Param1,
    void* Param2
);

typedef INT_PTR(WINAPI *FARAPISENDDLGMESSAGE)(
    HANDLE   hDlg,
    int Msg,
    int      Param1,
    void* Param2
);

typedef INT_PTR(WINAPI *FARAPIDEFDLGPROC)(
    HANDLE   hDlg,
    int Msg,
    int      Param1,
    void* Param2
);

typedef HANDLE(WINAPI *FARAPIDIALOGINIT)(
    const GUID*           PluginId,
    const GUID*           Id,
    int                   X1,
    int                   Y1,
    int                   X2,
    int                   Y2,
    const wchar_t        *HelpTopic,
    const struct FarDialogItem *Item,
    size_t                ItemsNumber,
    DWORD_PTR             Reserved,
    FARDIALOGFLAGS        Flags,
    FARWINDOWPROC         DlgProc,
    void*                 Param
);

typedef int (WINAPI *FARAPIDIALOGRUN)(
    HANDLE hDlg
);

typedef void (WINAPI *FARAPIDIALOGFREE)(
    HANDLE hDlg
);

typedef unsigned __int64 FARMENUFLAGS;
static const FARMENUFLAGS
	FMENU_SHOWAMPERSAND        = 0x0000000000000001ULL,
	FMENU_WRAPMODE             = 0x0000000000000002ULL,
	FMENU_AUTOHIGHLIGHT        = 0x0000000000000004ULL,
	FMENU_REVERSEAUTOHIGHLIGHT = 0x0000000000000008ULL,
	FMENU_CHANGECONSOLETITLE   = 0x0000000000000010ULL,
	FMENU_NONE                 = 0;

typedef int (WINAPI *FARAPIMENU)(
	const GUID*         PluginId,
    const GUID*         Id,
    int                 X,
    int                 Y,
    int                 MaxHeight,
    FARMENUFLAGS        Flags,
    const wchar_t      *Title,
    const wchar_t      *Bottom,
    const wchar_t      *HelpTopic,
    const struct FarKey *BreakKeys,
    int                *BreakCode,
    const struct FarMenuItem *Item,
    size_t              ItemsNumber
);


#define PANEL_NONE    ((HANDLE)(-1))
#define PANEL_ACTIVE  ((HANDLE)(-1))
#define PANEL_PASSIVE ((HANDLE)(-2))
#define PANEL_STOP ((HANDLE)(-1))

enum FILE_CONTROL_COMMANDS
{
	FCTL_CLOSEPANEL                 = 0,
	FCTL_GETPANELINFO               = 1,
	FCTL_UPDATEPANEL                = 2,
	FCTL_REDRAWPANEL                = 3,
	FCTL_GETCMDLINE                 = 4,
	FCTL_SETCMDLINE                 = 5,
	FCTL_SETSELECTION               = 6,
	FCTL_SETVIEWMODE                = 7,
	FCTL_INSERTCMDLINE              = 8,
	FCTL_SETUSERSCREEN              = 9,
	FCTL_SETPANELDIRECTORY          = 10,
	FCTL_SETCMDLINEPOS              = 11,
	FCTL_GETCMDLINEPOS              = 12,
	FCTL_SETSORTMODE                = 13,
	FCTL_SETSORTORDER               = 14,
	FCTL_SETCMDLINESELECTION        = 15,
	FCTL_GETCMDLINESELECTION        = 16,
	FCTL_CHECKPANELSEXIST           = 17,
	FCTL_SETNUMERICSORT             = 18,
	FCTL_GETUSERSCREEN              = 19,
	FCTL_ISACTIVEPANEL              = 20,
	FCTL_GETPANELITEM               = 21,
	FCTL_GETSELECTEDPANELITEM       = 22,
	FCTL_GETCURRENTPANELITEM        = 23,
	FCTL_GETPANELDIRECTORY          = 24,
	FCTL_GETCOLUMNTYPES             = 25,
	FCTL_GETCOLUMNWIDTHS            = 26,
	FCTL_BEGINSELECTION             = 27,
	FCTL_ENDSELECTION               = 28,
	FCTL_CLEARSELECTION             = 29,
	FCTL_SETDIRECTORIESFIRST        = 30,
	FCTL_GETPANELFORMAT             = 31,
	FCTL_GETPANELHOSTFILE           = 32,
	FCTL_SETCASESENSITIVESORT       = 33,
	FCTL_GETPANELPREFIX             = 34,
};

typedef void (WINAPI *FARAPITEXT)(
    int X,
    int Y,
    const struct FarColor* Color,
    const wchar_t *Str
);

typedef HANDLE(WINAPI *FARAPISAVESCREEN)(int X1, int Y1, int X2, int Y2);

typedef void (WINAPI *FARAPIRESTORESCREEN)(HANDLE hScreen);


typedef int (WINAPI *FARAPIGETDIRLIST)(
    const wchar_t *Dir,
    struct PluginPanelItem **pPanelItem,
    size_t *pItemsNumber
);

typedef int (WINAPI *FARAPIGETPLUGINDIRLIST)(
    const GUID* PluginId,
    HANDLE hPanel,
    const wchar_t *Dir,
    struct PluginPanelItem **pPanelItem,
    size_t *pItemsNumber
);

typedef void (WINAPI *FARAPIFREEDIRLIST)(struct PluginPanelItem *PanelItem, size_t nItemsNumber);
typedef void (WINAPI *FARAPIFREEPLUGINDIRLIST)(struct PluginPanelItem *PanelItem, size_t nItemsNumber);

typedef unsigned __int64 VIEWER_FLAGS;
static const VIEWER_FLAGS
	VF_NONMODAL              = 0x0000000000000001ULL,
	VF_DELETEONCLOSE         = 0x0000000000000002ULL,
	VF_ENABLE_F6             = 0x0000000000000004ULL,
	VF_DISABLEHISTORY        = 0x0000000000000008ULL,
	VF_IMMEDIATERETURN       = 0x0000000000000100ULL,
	VF_DELETEONLYFILEONCLOSE = 0x0000000000000200ULL,
	VF_NONE                  = 0;

typedef int (WINAPI *FARAPIVIEWER)(
    const wchar_t *FileName,
    const wchar_t *Title,
    int X1,
    int Y1,
    int X2,
    int Y2,
    VIEWER_FLAGS Flags,
    UINT CodePage
);

typedef unsigned __int64 EDITOR_FLAGS;
static const EDITOR_FLAGS
	EF_NONMODAL              = 0x0000000000000001ULL,
	EF_CREATENEW             = 0x0000000000000002ULL,
	EF_ENABLE_F6             = 0x0000000000000004ULL,
	EF_DISABLEHISTORY        = 0x0000000000000008ULL,
	EF_DELETEONCLOSE         = 0x0000000000000010ULL,
	EF_IMMEDIATERETURN       = 0x0000000000000100ULL,
	EF_DELETEONLYFILEONCLOSE = 0x0000000000000200ULL,
	EF_LOCKED                = 0x0000000000000400ULL,
	EF_DISABLESAVEPOS        = 0x0000000000000800ULL,
	EN_NONE                  = 0;

enum EDITOR_EXITCODE
{
	EEC_OPEN_ERROR          = 0,
	EEC_MODIFIED            = 1,
	EEC_NOT_MODIFIED        = 2,
	EEC_LOADING_INTERRUPTED = 3,
};

typedef int (WINAPI *FARAPIEDITOR)(
    const wchar_t *FileName,
    const wchar_t *Title,
    int X1,
    int Y1,
    int X2,
    int Y2,
    EDITOR_FLAGS Flags,
    int StartLine,
    int StartChar,
    UINT CodePage
);

typedef const wchar_t*(WINAPI *FARAPIGETMSG)(
    const GUID* PluginId,
    int MsgId
);

typedef unsigned __int64 FARHELPFLAGS;
static const FARHELPFLAGS
	FHELP_NOSHOWERROR = 0x0000000080000000ULL,
	FHELP_SELFHELP    = 0x0000000000000000ULL,
	FHELP_FARHELP     = 0x0000000000000001ULL,
	FHELP_CUSTOMFILE  = 0x0000000000000002ULL,
	FHELP_CUSTOMPATH  = 0x0000000000000004ULL,
	FHELP_USECONTENTS = 0x0000000040000000ULL,
	FHELP_NONE        = 0;

typedef BOOL (WINAPI *FARAPISHOWHELP)(
    const wchar_t *ModuleName,
    const wchar_t *Topic,
    FARHELPFLAGS Flags
);

enum ADVANCED_CONTROL_COMMANDS
{
	ACTL_GETFARMANAGERVERSION       = 0,
	ACTL_WAITKEY                    = 2,
	ACTL_GETCOLOR                   = 3,
	ACTL_GETARRAYCOLOR              = 4,
	ACTL_EJECTMEDIA                 = 5,
	ACTL_GETWINDOWINFO              = 6,
	ACTL_GETWINDOWCOUNT             = 7,
	ACTL_SETCURRENTWINDOW           = 8,
	ACTL_COMMIT                     = 9,
	ACTL_GETFARHWND                 = 10,
	ACTL_SETARRAYCOLOR              = 16,
	ACTL_REDRAWALL                  = 19,
	ACTL_SYNCHRO                    = 20,
	ACTL_SETPROGRESSSTATE           = 21,
	ACTL_SETPROGRESSVALUE           = 22,
	ACTL_QUIT                       = 23,
	ACTL_GETFARRECT                 = 24,
	ACTL_GETCURSORPOS               = 25,
	ACTL_SETCURSORPOS               = 26,
	ACTL_PROGRESSNOTIFY             = 27,
	ACTL_GETWINDOWTYPE              = 28,


};

enum FAR_MACRO_CONTROL_COMMANDS
{
	MCTL_LOADALL           = 0,
	MCTL_SAVEALL           = 1,
	MCTL_SENDSTRING        = 2,
	MCTL_GETSTATE          = 5,
	MCTL_GETAREA           = 6,
	MCTL_ADDMACRO          = 7,
	MCTL_DELMACRO          = 8,
	MCTL_GETLASTERROR      = 9,
};

enum FARMACROSENDSTRINGCOMMAND
{
	MSSC_POST              =0,
	MSSC_CHECK             =2,
};

enum FARMACROAREA
{
	MACROAREA_OTHER                      =   0,
	MACROAREA_SHELL                      =   1,
	MACROAREA_VIEWER                     =   2,
	MACROAREA_EDITOR                     =   3,
	MACROAREA_DIALOG                     =   4,
	MACROAREA_SEARCH                     =   5,
	MACROAREA_DISKS                      =   6,
	MACROAREA_MAINMENU                   =   7,
	MACROAREA_MENU                       =   8,
	MACROAREA_HELP                       =   9,
	MACROAREA_INFOPANEL                  =  10,
	MACROAREA_QVIEWPANEL                 =  11,
	MACROAREA_TREEPANEL                  =  12,
	MACROAREA_FINDFOLDER                 =  13,
	MACROAREA_USERMENU                   =  14,
	MACROAREA_SHELLAUTOCOMPLETION        =  15,
	MACROAREA_DIALOGAUTOCOMPLETION       =  16,

	MACROAREA_COMMON                     = 255,
};

enum FARMACROSTATE
{
	MACROSTATE_NOMACRO          = 0,
	MACROSTATE_EXECUTING        = 1,
	MACROSTATE_EXECUTING_COMMON = 2,
	MACROSTATE_RECORDING        = 3,
	MACROSTATE_RECORDING_COMMON = 4,
};

typedef unsigned __int64 FARADDKEYMACROFLAGS;
static const FARADDKEYMACROFLAGS
	AKMFLAGS_NONE                = 0;

typedef int (WINAPI *FARMACROCALLBACK)(void* Id,FARADDKEYMACROFLAGS Flags);


typedef unsigned __int64 FARSETCOLORFLAGS;
static const FARSETCOLORFLAGS

	FSETCLR_REDRAW                 = 0x0000000000000001ULL,
	FSETCLR_NONE                   = 0;

struct FarSetColors
{
	FARSETCOLORFLAGS Flags;
	size_t StartIndex;
	size_t ColorsCount;
	struct FarColor* Colors;
};

enum WINDOWINFO_TYPE
{
	WTYPE_PANELS                    = 1,
	WTYPE_VIEWER                    = 2,
	WTYPE_EDITOR                    = 3,
	WTYPE_DIALOG                    = 4,
	WTYPE_VMENU                     = 5,
	WTYPE_HELP                      = 6,
};

typedef unsigned __int64 WINDOWINFO_FLAGS;
static const WINDOWINFO_FLAGS
	WIF_MODIFIED = 0x0000000000000001ULL,
	WIF_CURRENT  = 0x0000000000000002ULL,
	WIF_MODAL    = 0x0000000000000004ULL;

struct WindowInfo
{
	size_t StructSize;
	INT_PTR Id;
	wchar_t *TypeName;
	wchar_t *Name;
	int TypeNameSize;
	int NameSize;
	int Pos;
	enum WINDOWINFO_TYPE Type;
	WINDOWINFO_FLAGS Flags;
};

struct WindowType
{
	size_t StructSize;
	enum WINDOWINFO_TYPE Type;
};

enum TASKBARPROGRESSTATE
{
	TBPS_NOPROGRESS   =0x0,
	TBPS_INDETERMINATE=0x1,
	TBPS_NORMAL       =0x2,
	TBPS_ERROR        =0x4,
	TBPS_PAUSED       =0x8,
};

struct ProgressValue
{
	unsigned __int64 Completed;
	unsigned __int64 Total;
};

enum VIEWER_CONTROL_COMMANDS
{
	VCTL_GETINFO                    = 0,
	VCTL_QUIT                       = 1,
	VCTL_REDRAW                     = 2,
	VCTL_SETKEYBAR                  = 3,
	VCTL_SETPOSITION                = 4,
	VCTL_SELECT                     = 5,
	VCTL_SETMODE                    = 6,
};

typedef unsigned __int64 VIEWER_OPTIONS;
static const VIEWER_OPTIONS
	VOPT_SAVEFILEPOSITION   = 0x0000000000000001ULL,
	VOPT_AUTODETECTCODEPAGE = 0x0000000000000002ULL,
	VOPT_NONE               = 0;

enum VIEWER_SETMODE_TYPES
{
	VSMT_HEX                        = 0,
	VSMT_WRAP                       = 1,
	VSMT_WORDWRAP                   = 2,
};

typedef unsigned __int64 VIEWER_SETMODEFLAGS_TYPES;
static const VIEWER_SETMODEFLAGS_TYPES
	VSMFL_REDRAW    = 0x0000000000000001ULL;

struct ViewerSetMode
{
	enum VIEWER_SETMODE_TYPES Type;
	union
	{
		int iParam;
		wchar_t *wszParam;
	}
#ifndef __cplusplus
	Param
#endif
	;
	VIEWER_SETMODEFLAGS_TYPES Flags;
	DWORD_PTR Reserved;
};

struct ViewerSelect
{
	__int64 BlockStartPos;
	int     BlockLen;
};

typedef unsigned __int64 VIEWER_SETPOS_FLAGS;
static const VIEWER_SETPOS_FLAGS
	VSP_NOREDRAW    = 0x0000000000000001ULL,
	VSP_PERCENT     = 0x0000000000000002ULL,
	VSP_RELATIVE    = 0x0000000000000004ULL,
	VSP_NORETNEWPOS = 0x0000000000000008ULL;

struct ViewerSetPosition
{
	VIEWER_SETPOS_FLAGS Flags;
	__int64 StartPos;
	__int64 LeftPos;
};

struct ViewerMode
{
	UINT CodePage;
	int Wrap;
	int WordWrap;
	int Hex;
	DWORD_PTR Reserved[4];
};

struct ViewerInfo
{
	size_t StructSize;
	int ViewerID;
	int TabSize;
	const wchar_t *FileName;
	struct ViewerMode CurMode;
	__int64 FileSize;
	__int64 FilePos;
	__int64 LeftPos;
	VIEWER_OPTIONS Options;
	int WindowSizeX;
	int WindowSizeY;
};

enum VIEWER_EVENTS
{
	VE_READ       =0,
	VE_CLOSE      =1,

	VE_GOTFOCUS   =6,
	VE_KILLFOCUS  =7,
};


enum EDITOR_EVENTS
{
	EE_READ       =0,
	EE_SAVE       =1,
	EE_REDRAW     =2,
	EE_CLOSE      =3,

	EE_GOTFOCUS   =6,
	EE_KILLFOCUS  =7,
	EE_CHANGE     =8,
};

enum DIALOG_EVENTS
{
	DE_DLGPROCINIT    =0,
	DE_DEFDLGPROCINIT =1,
	DE_DLGPROCEND     =2,
};

enum SYNCHRO_EVENTS
{
	SE_COMMONSYNCHRO  =0,
};

#define CURRENT_EDITOR -1

enum EDITOR_CONTROL_COMMANDS
{
	ECTL_GETSTRING                  = 0,
	ECTL_SETSTRING                  = 1,
	ECTL_INSERTSTRING               = 2,
	ECTL_DELETESTRING               = 3,
	ECTL_DELETECHAR                 = 4,
	ECTL_INSERTTEXT                 = 5,
	ECTL_GETINFO                    = 6,
	ECTL_SETPOSITION                = 7,
	ECTL_SELECT                     = 8,
	ECTL_REDRAW                     = 9,
	ECTL_TABTOREAL                  = 10,
	ECTL_REALTOTAB                  = 11,
	ECTL_EXPANDTABS                 = 12,
	ECTL_SETTITLE                   = 13,
	ECTL_READINPUT                  = 14,
	ECTL_PROCESSINPUT               = 15,
	ECTL_ADDCOLOR                   = 16,
	ECTL_GETCOLOR                   = 17,
	ECTL_SAVEFILE                   = 18,
	ECTL_QUIT                       = 19,
	ECTL_SETKEYBAR                  = 20,

	ECTL_SETPARAM                   = 22,
	ECTL_GETBOOKMARKS               = 23,
	ECTL_DELETEBLOCK                = 25,
	ECTL_ADDSESSIONBOOKMARK         = 26,
	ECTL_PREVSESSIONBOOKMARK        = 27,
	ECTL_NEXTSESSIONBOOKMARK        = 28,
	ECTL_CLEARSESSIONBOOKMARKS      = 29,
	ECTL_DELETESESSIONBOOKMARK      = 30,
	ECTL_GETSESSIONBOOKMARKS        = 31,
	ECTL_UNDOREDO                   = 32,
	ECTL_GETFILENAME                = 33,
	ECTL_DELCOLOR                   = 34,
};

enum EDITOR_SETPARAMETER_TYPES
{
	ESPT_TABSIZE                    = 0,
	ESPT_EXPANDTABS                 = 1,
	ESPT_AUTOINDENT                 = 2,
	ESPT_CURSORBEYONDEOL            = 3,
	ESPT_CHARCODEBASE               = 4,
	ESPT_CODEPAGE                   = 5,
	ESPT_SAVEFILEPOSITION           = 6,
	ESPT_LOCKMODE                   = 7,
	ESPT_SETWORDDIV                 = 8,
	ESPT_GETWORDDIV                 = 9,
	ESPT_SHOWWHITESPACE             = 10,
	ESPT_SETBOM                     = 11,
};



struct EditorSetParameter
{
	enum EDITOR_SETPARAMETER_TYPES Type;
	union
	{
		int iParam;
		wchar_t *wszParam;
		DWORD_PTR Reserved;
	}
#ifndef __cplusplus
	Param
#endif
	;
	unsigned __int64 Flags;
	size_t Size;
};


enum EDITOR_UNDOREDO_COMMANDS
{
	EUR_BEGIN                       = 0,
	EUR_END                         = 1,
	EUR_UNDO                        = 2,
	EUR_REDO                        = 3,
};


struct EditorUndoRedo
{
	enum EDITOR_UNDOREDO_COMMANDS Command;
	DWORD_PTR Reserved[3];
};

struct EditorGetString
{
	int StringNumber;
	int StringLength;
	const wchar_t *StringText;
	const wchar_t *StringEOL;
	int SelStart;
	int SelEnd;
};


struct EditorSetString
{
	int StringNumber;
	int StringLength;
	const wchar_t *StringText;
	const wchar_t *StringEOL;
};

enum EXPAND_TABS
{
	EXPAND_NOTABS                   = 0,
	EXPAND_ALLTABS                  = 1,
	EXPAND_NEWTABS                  = 2,
};


enum EDITOR_OPTIONS
{
	EOPT_EXPANDALLTABS     = 0x00000001,
	EOPT_PERSISTENTBLOCKS  = 0x00000002,
	EOPT_DELREMOVESBLOCKS  = 0x00000004,
	EOPT_AUTOINDENT        = 0x00000008,
	EOPT_SAVEFILEPOSITION  = 0x00000010,
	EOPT_AUTODETECTCODEPAGE= 0x00000020,
	EOPT_CURSORBEYONDEOL   = 0x00000040,
	EOPT_EXPANDONLYNEWTABS = 0x00000080,
	EOPT_SHOWWHITESPACE    = 0x00000100,
	EOPT_BOM               = 0x00000200,
	EOPT_SHOWLINEBREAK     = 0x00000400,
};


enum EDITOR_BLOCK_TYPES
{
	BTYPE_NONE                      = 0,
	BTYPE_STREAM                    = 1,
	BTYPE_COLUMN                    = 2,
};

enum EDITOR_CURRENTSTATE
{
	ECSTATE_MODIFIED       = 0x00000001,
	ECSTATE_SAVED          = 0x00000002,
	ECSTATE_LOCKED         = 0x00000004,
};


struct EditorInfo
{
	int EditorID;
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
	DWORD Options;
	int TabSize;
	int BookMarkCount;
	DWORD CurState;
	UINT CodePage;
	DWORD_PTR Reserved[5];
};

struct EditorBookMarks
{
	int *Line;
	int *Cursor;
	int *ScreenLine;
	int *LeftPos;
	DWORD_PTR Reserved[4];
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


struct EditorConvertPos
{
	int StringNumber;
	int SrcPos;
	int DestPos;
};


typedef unsigned __int64 EDITORCOLORFLAGS;
static const EDITORCOLORFLAGS
	ECF_TABMARKFIRST   = 0x0000000000000001ULL,
	ECF_TABMARKCURRENT = 0x0000000000000002ULL;

struct EditorColor
{
	size_t StructSize;
	int StringNumber;
	int ColorItem;
	int StartPos;
	int EndPos;
	unsigned Priority;
	EDITORCOLORFLAGS Flags;
	struct FarColor Color;
	GUID Owner;
};

struct EditorDeleteColor
{
	size_t StructSize;
	GUID Owner;
	int StringNumber;
	int StartPos;
};

#define EDITOR_COLOR_NORMAL_PRIORITY 0x80000000U

struct EditorSaveFile
{
	const wchar_t *FileName;
	const wchar_t *FileEOL;
	UINT CodePage;
};

enum EDITOR_CHANGETYPE
{
	ECTYPE_CHANGED = 0,
	ECTYPE_ADDED   = 1,
	ECTYPE_DELETED = 2,
};

struct EditorChange
{
	size_t StructSize;
	enum EDITOR_CHANGETYPE Type;
	int StringNumber;
};

typedef unsigned __int64 INPUTBOXFLAGS;
static const INPUTBOXFLAGS
	FIB_ENABLEEMPTY      = 0x0000000000000001ULL,
	FIB_PASSWORD         = 0x0000000000000002ULL,
	FIB_EXPANDENV        = 0x0000000000000004ULL,
	FIB_NOUSELASTHISTORY = 0x0000000000000008ULL,
	FIB_BUTTONS          = 0x0000000000000010ULL,
	FIB_NOAMPERSAND      = 0x0000000000000020ULL,
	FIB_EDITPATH         = 0x0000000000000040ULL,
	FIB_EDITPATHEXEC     = 0x0000000000000080ULL,
	FIB_NONE             = 0;

typedef int (WINAPI *FARAPIINPUTBOX)(
    const GUID* PluginId,
    const GUID* Id,
    const wchar_t *Title,
    const wchar_t *SubTitle,
    const wchar_t *HistoryName,
    const wchar_t *SrcText,
    wchar_t *DestText,
    size_t DestSize,
    const wchar_t *HelpTopic,
    INPUTBOXFLAGS Flags
);

enum FAR_PLUGINS_CONTROL_COMMANDS
{
	PCTL_LOADPLUGIN           = 0,
	PCTL_UNLOADPLUGIN         = 1,
	PCTL_FORCEDLOADPLUGIN     = 2,
	PCTL_FINDPLUGIN           = 3,
	PCTL_GETPLUGININFORMATION = 4,
	PCTL_GETPLUGINS           = 5,
};

enum FAR_PLUGIN_LOAD_TYPE
{
	PLT_PATH = 0,
};

enum FAR_PLUGIN_FIND_TYPE
{
	PFM_GUID       = 0,
	PFM_MODULENAME = 1,
};

typedef unsigned __int64 FAR_PLUGIN_FLAGS;
static const FAR_PLUGIN_FLAGS
	FPF_LOADED         = 0x0000000000000001ULL,
	FPF_ANSI           = 0x1000000000000000ULL,
	FPF_NONE           = 0;

enum FAR_FILE_FILTER_CONTROL_COMMANDS
{
	FFCTL_CREATEFILEFILTER          = 0,
	FFCTL_FREEFILEFILTER            = 1,
	FFCTL_OPENFILTERSMENU           = 2,
	FFCTL_STARTINGTOFILTER          = 3,
	FFCTL_ISFILEINFILTER            = 4,
};

enum FAR_FILE_FILTER_TYPE
{
	FFT_PANEL                       = 0,
	FFT_FINDFILE                    = 1,
	FFT_COPY                        = 2,
	FFT_SELECT                      = 3,
	FFT_CUSTOM                      = 4,
};

enum FAR_REGEXP_CONTROL_COMMANDS
{
	RECTL_CREATE                    = 0,
	RECTL_FREE                      = 1,
	RECTL_COMPILE                   = 2,
	RECTL_OPTIMIZE                  = 3,
	RECTL_MATCHEX                   = 4,
	RECTL_SEARCHEX                  = 5,
	RECTL_BRACKETSCOUNT             = 6,
};

struct RegExpMatch
{
	int start,end;
};

struct RegExpSearch
{
	const wchar_t* Text;
	int Position;
	int Length;
	struct RegExpMatch* Match;
	int Count;
	void* Reserved;
};

enum FAR_SETTINGS_CONTROL_COMMANDS
{
	SCTL_CREATE                     = 0,
	SCTL_FREE                       = 1,
	SCTL_SET                        = 2,
	SCTL_GET                        = 3,
	SCTL_ENUM                       = 4,
	SCTL_DELETE                     = 5,
	SCTL_CREATESUBKEY               = 6,
	SCTL_OPENSUBKEY                 = 7,
};

enum FARSETTINGSTYPES
{
	FST_UNKNOWN                     = 0,
	FST_SUBKEY                      = 1,
	FST_QWORD                       = 2,
	FST_STRING                      = 3,
	FST_DATA                        = 4,
};

enum FARSETTINGS_SUBFOLDERS
{
	FSSF_ROOT                       =  0,
	FSSF_HISTORY_CMD                =  1,
	FSSF_HISTORY_FOLDER             =  2,
	FSSF_HISTORY_VIEW               =  3,
	FSSF_HISTORY_EDIT               =  4,
	FSSF_HISTORY_EXTERNAL           =  5,
	FSSF_FOLDERSHORTCUT_0           =  6,
	FSSF_FOLDERSHORTCUT_1           =  7,
	FSSF_FOLDERSHORTCUT_2           =  8,
	FSSF_FOLDERSHORTCUT_3           =  9,
	FSSF_FOLDERSHORTCUT_4           = 10,
	FSSF_FOLDERSHORTCUT_5           = 11,
	FSSF_FOLDERSHORTCUT_6           = 12,
	FSSF_FOLDERSHORTCUT_7           = 13,
	FSSF_FOLDERSHORTCUT_8           = 14,
	FSSF_FOLDERSHORTCUT_9           = 15,
	FSSF_CONFIRMATIONS              = 16,
	FSSF_SYSTEM                     = 17,
	FSSF_PANEL                      = 18,
	FSSF_EDITOR                     = 19,
	FSSF_SCREEN                     = 20,
	FSSF_DIALOG                     = 21,
	FSSF_INTERFACE                  = 22,
	FSSF_PANELLAYOUT                = 23,
};

enum FAR_PLUGIN_SETTINGS_LOCATION
{
	PSL_ROAMING = 0,
	PSL_LOCAL   = 1,
};

struct FarSettingsCreate
{
	size_t StructSize;
	GUID Guid;
	HANDLE Handle;
};

struct FarSettingsItem
{
	size_t Root;
	const wchar_t* Name;
	enum FARSETTINGSTYPES Type;
	union
	{
		unsigned __int64 Number;
		const wchar_t* String;
		struct
		{
			size_t Size;
			const void* Data;
		} Data;
	}
#ifndef __cplusplus
	Value
#endif
	;
};

struct FarSettingsName
{
	const wchar_t* Name;
	enum FARSETTINGSTYPES Type;
};

struct FarSettingsHistory
{
	const wchar_t* Name;
	const wchar_t* Param;
	GUID PluginId;
	const wchar_t* File;
	FILETIME Time;
	BOOL Lock;
};

struct FarSettingsEnum
{
	size_t Root;
	size_t Count;
	union
	{
		const struct FarSettingsName* Items;
		const struct FarSettingsHistory* Histories;
	}
#ifndef __cplusplus
	Value
#endif
	;
};

struct FarSettingsValue
{
	size_t Root;
	const wchar_t* Value;
};

typedef INT_PTR (WINAPI *FARAPIPANELCONTROL)(
    HANDLE hPanel,
    enum FILE_CONTROL_COMMANDS Command,
    int Param1,
    void* Param2
);

typedef INT_PTR(WINAPI *FARAPIADVCONTROL)(
    const GUID* PluginId,
    enum ADVANCED_CONTROL_COMMANDS Command,
    int Param1,
    void* Param2
);

typedef INT_PTR (WINAPI *FARAPIVIEWERCONTROL)(
    int ViewerID,
    enum VIEWER_CONTROL_COMMANDS Command,
    int Param1,
    void* Param2
);

typedef INT_PTR (WINAPI *FARAPIEDITORCONTROL)(
    int EditorID,
    enum EDITOR_CONTROL_COMMANDS Command,
    int Param1,
    void* Param2
);

typedef INT_PTR (WINAPI *FARAPIMACROCONTROL)(
    const GUID* PluginId,
    enum FAR_MACRO_CONTROL_COMMANDS Command,
    int Param1,
    void* Param2
);

typedef INT_PTR (WINAPI *FARAPIPLUGINSCONTROL)(
    HANDLE hHandle,
    enum FAR_PLUGINS_CONTROL_COMMANDS Command,
    int Param1,
    void* Param2
);

typedef INT_PTR (WINAPI *FARAPIFILEFILTERCONTROL)(
    HANDLE hHandle,
    enum FAR_FILE_FILTER_CONTROL_COMMANDS Command,
    int Param1,
    void* Param2
);

typedef INT_PTR (WINAPI *FARAPIREGEXPCONTROL)(
    HANDLE hHandle,
    enum FAR_REGEXP_CONTROL_COMMANDS Command,
    int Param1,
    void* Param2
);

typedef INT_PTR (WINAPI *FARAPISETTINGSCONTROL)(
    HANDLE hHandle,
    enum FAR_SETTINGS_CONTROL_COMMANDS Command,
    int Param1,
    void* Param2
);

struct PluginStartupInfo
{
	size_t StructSize;
	const wchar_t *ModuleName;
	FARAPIMENU             Menu;
	FARAPIMESSAGE          Message;
	FARAPIGETMSG           GetMsg;
	FARAPIPANELCONTROL     PanelControl;
	FARAPISAVESCREEN       SaveScreen;
	FARAPIRESTORESCREEN    RestoreScreen;
	FARAPIGETDIRLIST       GetDirList;
	FARAPIGETPLUGINDIRLIST GetPluginDirList;
	FARAPIFREEDIRLIST      FreeDirList;
	FARAPIFREEPLUGINDIRLIST FreePluginDirList;
	FARAPIVIEWER           Viewer;
	FARAPIEDITOR           Editor;
	FARAPITEXT             Text;
	FARAPIEDITORCONTROL    EditorControl;

	FARSTANDARDFUNCTIONS  *FSF;

	FARAPISHOWHELP         ShowHelp;
	FARAPIADVCONTROL       AdvControl;
	FARAPIINPUTBOX         InputBox;
	FARAPICOLORDIALOG      ColorDialog;
	FARAPIDIALOGINIT       DialogInit;
	FARAPIDIALOGRUN        DialogRun;
	FARAPIDIALOGFREE       DialogFree;

	FARAPISENDDLGMESSAGE   SendDlgMessage;
	FARAPIDEFDLGPROC       DefDlgProc;
	DWORD_PTR              Reserved;
	FARAPIVIEWERCONTROL    ViewerControl;
	FARAPIPLUGINSCONTROL   PluginsControl;
	FARAPIFILEFILTERCONTROL FileFilterControl;
	FARAPIREGEXPCONTROL    RegExpControl;
	FARAPIMACROCONTROL     MacroControl;
	FARAPISETTINGSCONTROL  SettingsControl;
	void                  *Private;
};

typedef HANDLE (WINAPI *FARAPICREATEFILE)(const wchar_t *Object,DWORD DesiredAccess,DWORD ShareMode,LPSECURITY_ATTRIBUTES SecurityAttributes,DWORD CreationDistribution,DWORD FlagsAndAttributes,HANDLE TemplateFile);
typedef DWORD (WINAPI *FARAPIGETFILEATTRIBUTES)(const wchar_t *FileName);
typedef BOOL (WINAPI *FARAPISETFILEATTRIBUTES)(const wchar_t *FileName,DWORD dwFileAttributes);
typedef BOOL (WINAPI *FARAPIMOVEFILEEX)(const wchar_t *ExistingFileName,const wchar_t *NewFileName,DWORD dwFlags);
typedef BOOL (WINAPI *FARAPIDELETEFILE)(const wchar_t *FileName);
typedef BOOL (WINAPI *FARAPIREMOVEDIRECTORY)(const wchar_t *DirName);
typedef BOOL (WINAPI *FARAPICREATEDIRECTORY)(const wchar_t *PathName,LPSECURITY_ATTRIBUTES lpSecurityAttributes);

struct ArclitePrivateInfo
{
	size_t StructSize;
	FARAPICREATEFILE CreateFile;
	FARAPIGETFILEATTRIBUTES GetFileAttributes;
	FARAPISETFILEATTRIBUTES SetFileAttributes;
	FARAPIMOVEFILEEX MoveFileEx;
	FARAPIDELETEFILE DeleteFile;
	FARAPIREMOVEDIRECTORY RemoveDirectory;
	FARAPICREATEDIRECTORY CreateDirectory;
};

static __inline BOOL CheckVersion(const struct VersionInfo* Current, const struct VersionInfo* Required)
{
	return (Current->Major > Required->Major) || (Current->Major == Required->Major && Current->Minor > Required->Minor) || (Current->Major == Required->Major && Current->Minor == Required->Minor && Current->Revision > Required->Revision) || (Current->Major == Required->Major && Current->Minor == Required->Minor && Current->Revision == Required->Revision && Current->Build >= Required->Build);
}

static __inline struct VersionInfo MAKEFARVERSION(DWORD Major, DWORD Minor, DWORD Revision, DWORD Build, enum VERSION_STAGE Stage)
{
	struct VersionInfo Info = {Major, Minor, Revision, Build, Stage};
	return Info;
}

#define FARMANAGERVERSION MAKEFARVERSION(FARMANAGERVERSION_MAJOR,FARMANAGERVERSION_MINOR, FARMANAGERVERSION_REVISION, FARMANAGERVERSION_BUILD, FARMANAGERVERSION_STAGE)
