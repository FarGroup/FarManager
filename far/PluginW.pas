{
  PluginW.pas

  Plugin API for Far Manager <%VERSION%>
}

{
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

EXCEPTION:
Far Manager plugins that use this header file can be distributed under any
other possible license with no implications from the above license on them.
}

{$Align Off}
{$RangeChecks Off}

{$ifdef CPUX86_64}
 {$PACKRECORDS C}
{$endif CPUX86_64}

Unit PluginW;

interface

uses Windows;

const
  FARMANAGERVERSION_MAJOR = 2;
  FARMANAGERVERSION_MINOR = 0;
  FARMANAGERVERSION_BUILD = 1658;

type
//TFarChar = AnsiChar;
//PFarChar = PAnsiChar;

  TFarChar = WideChar;
  PFarChar = PWideChar;

 {$ifdef CPUX86_64}
  INT_PTR = PtrInt;
  INT_PTR = PtrInt;
  DWORD_PTR = PtrUInt;
  SIZE_T = PtrUInt;
 {$else}
  INT_PTR = Integer;
  INT_PTR = Integer;
  DWORD_PTR = Cardinal;
  SIZE_T = Cardinal;
 {$endif CPUX86_64}

  PPCharArray = ^TPCharArray;
  TPCharArray = packed array[0..MaxInt div SizeOf(PFarChar) - 1] of PFarChar;

  PIntegerArray = ^TIntegerArray;
  TIntegerArray = packed array[0..Pred(MaxLongint div SizeOf(Integer))] of Integer;

const
  FARMACRO_KEY_EVENT = KEY_EVENT or $8000;

  CP_UNICODE = 1200;
  CP_REVERSEBOM = 1201;
  CP_AUTODETECT = UINT(-1);


{ FARMESSAGEFLAGS }

const
  FMSG_WARNING             = $00000001;
  FMSG_ERRORTYPE           = $00000002;
  FMSG_KEEPBACKGROUND      = $00000004;
  FMSG_LEFTALIGN           = $00000010;
  FMSG_ALLINONE            = $00000020;

  FMSG_MB_OK               = $00010000;
  FMSG_MB_OKCANCEL         = $00020000;
  FMSG_MB_ABORTRETRYIGNORE = $00030000;
  FMSG_MB_YESNO            = $00040000;
  FMSG_MB_YESNOCANCEL      = $00050000;
  FMSG_MB_RETRYCANCEL      = $00060000;

(*
typedef int (WINAPI *FARAPIMESSAGE)(
  INT_PTR PluginNumber,
  DWORD Flags,
  const wchar_t *HelpTopic,
  const wchar_t * const *Items,
  int ItemsNumber,
  int ButtonsNumber
);
*)
type
  TFarApiMessage = function (
    PluginNumber :INT_PTR;
    Flags :DWORD;
    HelpTopic :PFarChar;
    Items :PPCharArray;
    ItemsNumber :Integer;
    ButtonsNumber :Integer
  ) :Integer; stdcall;


{ DialogItemTypes }

const
  DI_TEXT         = 0;
  DI_VTEXT        = 1;
  DI_SINGLEBOX    = 2;
  DI_DOUBLEBOX    = 3;
  DI_EDIT         = 4;
  DI_PSWEDIT      = 5;
  DI_FIXEDIT      = 6;
  DI_BUTTON       = 7;
  DI_CHECKBOX     = 8;
  DI_RADIOBUTTON  = 9;
  DI_COMBOBOX     = 10;
  DI_LISTBOX      = 11;
  DI_USERCONTROL  = 255;


{ FarDialogItemFlags }

const
  DIF_NONE                  = 0;
  DIF_COLORMASK             = $000000ff;
  DIF_SETCOLOR              = $00000100;
  DIF_BOXCOLOR              = $00000200;
  DIF_GROUP                 = $00000400;
  DIF_LEFTTEXT              = $00000800;
  DIF_MOVESELECT            = $00001000;
  DIF_SHOWAMPERSAND         = $00002000;
  DIF_CENTERGROUP           = $00004000;
  DIF_NOBRACKETS            = $00008000;
  DIF_MANUALADDHISTORY      = $00008000;
  DIF_SEPARATOR             = $00010000;
  DIF_SEPARATOR2            = $00020000;
  DIF_EDITOR                = $00020000;
  DIF_LISTNOAMPERSAND       = $00020000;
  DIF_LISTNOBOX             = $00040000;
  DIF_HISTORY               = $00040000;
  DIF_BTNNOCLOSE            = $00040000;
  DIF_CENTERTEXT            = $00040000;
  DIF_SETSHIELD             = $00080000;
  DIF_EDITEXPAND            = $00080000;
  DIF_DROPDOWNLIST          = $00100000;
  DIF_USELASTHISTORY        = $00200000;
  DIF_MASKEDIT              = $00400000;
  DIF_SELECTONENTRY         = $00800000;
  DIF_3STATE                = $00800000;
  DIF_EDITPATH              = $01000000;
  DIF_LISTWRAPMODE          = $01000000;
  DIF_NOAUTOCOMPLETE        = $02000000;
  DIF_LISTAUTOHIGHLIGHT     = $02000000;
  DIF_LISTNOCLOSE           = $04000000;
  DIF_HIDDEN                = $10000000;
  DIF_READONLY              = $20000000;
  DIF_NOFOCUS               = $40000000;
  DIF_DISABLE               = $80000000;


{ FarMessagesProc }

const
  DM_FIRST                = 0;
  DM_CLOSE                = DM_FIRST+1;
  DM_ENABLE               = DM_FIRST+2;
  DM_ENABLEREDRAW         = DM_FIRST+3;
  DM_GETDLGDATA           = DM_FIRST+4;
  DM_GETDLGITEM           = DM_FIRST+5;
  DM_GETDLGRECT           = DM_FIRST+6;
  DM_GETTEXT              = DM_FIRST+7;
  DM_GETTEXTLENGTH        = DM_FIRST+8;
  DM_KEY                  = DM_FIRST+9;
  DM_MOVEDIALOG           = DM_FIRST+10;
  DM_SETDLGDATA           = DM_FIRST+11;
  DM_SETDLGITEM           = DM_FIRST+12;
  DM_SETFOCUS             = DM_FIRST+13;
  DM_REDRAW               = DM_FIRST+14;
  DM_SETREDRAW            = DM_REDRAW;
  DM_SETTEXT              = DM_FIRST+15;
  DM_SETMAXTEXTLENGTH     = DM_FIRST+16;
  DM_SETTEXTLENGTH        = DM_SETMAXTEXTLENGTH;
  DM_SHOWDIALOG           = DM_FIRST+17;
  DM_GETFOCUS             = DM_FIRST+18;
  DM_GETCURSORPOS         = DM_FIRST+19;
  DM_SETCURSORPOS         = DM_FIRST+20;
  DM_GETTEXTPTR           = DM_FIRST+21;
  DM_SETTEXTPTR           = DM_FIRST+22;
  DM_SHOWITEM             = DM_FIRST+23;
  DM_ADDHISTORY           = DM_FIRST+24;

  DM_GETCHECK             = DM_FIRST+25;
  DM_SETCHECK             = DM_FIRST+26;
  DM_SET3STATE            = DM_FIRST+27;

  DM_LISTSORT             = DM_FIRST+28;
  DM_LISTGETITEM          = DM_FIRST+29;
  DM_LISTGETCURPOS        = DM_FIRST+30;
  DM_LISTSETCURPOS        = DM_FIRST+31;
  DM_LISTDELETE           = DM_FIRST+32;
  DM_LISTADD              = DM_FIRST+33;
  DM_LISTADDSTR           = DM_FIRST+34;
  DM_LISTUPDATE           = DM_FIRST+35;
  DM_LISTINSERT           = DM_FIRST+36;
  DM_LISTFINDSTRING       = DM_FIRST+37;
  DM_LISTINFO             = DM_FIRST+38;
  DM_LISTGETDATA          = DM_FIRST+39;
  DM_LISTSETDATA          = DM_FIRST+40;
  DM_LISTSETTITLES        = DM_FIRST+41;
  DM_LISTGETTITLES        = DM_FIRST+42;

  DM_RESIZEDIALOG         = DM_FIRST+43;
  DM_SETITEMPOSITION      = DM_FIRST+44;

  DM_GETDROPDOWNOPENED    = DM_FIRST+45;
  DM_SETDROPDOWNOPENED    = DM_FIRST+46;

  DM_SETHISTORY           = DM_FIRST+47;

  DM_GETITEMPOSITION      = DM_FIRST+48;
  DM_SETMOUSEEVENTNOTIFY  = DM_FIRST+49;

  DM_EDITUNCHANGEDFLAG    = DM_FIRST+50;

  DM_GETITEMDATA          = DM_FIRST+51;
  DM_SETITEMDATA          = DM_FIRST+52;

  DM_LISTSET              = DM_FIRST+53;
  DM_LISTSETMOUSEREACTION = DM_FIRST+54;

  DM_GETCURSORSIZE        = DM_FIRST+55;
  DM_SETCURSORSIZE        = DM_FIRST+56;

  DM_LISTGETDATASIZE      = DM_FIRST+57;

  DM_GETSELECTION         = DM_FIRST+58;
  DM_SETSELECTION         = DM_FIRST+59;

  DM_GETEDITPOSITION      = DM_FIRST+60;
  DM_SETEDITPOSITION      = DM_FIRST+61;

  DM_SETCOMBOBOXEVENT     = DM_FIRST+62;
  DM_GETCOMBOBOXEVENT     = DM_FIRST+63;

  DM_GETCONSTTEXTPTR      = DM_FIRST+64;
  DM_GETDLGITEMSHORT      = DM_FIRST+65;
  DM_SETDLGITEMSHORT      = DM_FIRST+66;

  DM_GETDIALOGINFO        = DM_FIRST+67;

  DN_FIRST                = $1000;
  DN_BTNCLICK             = DN_FIRST+1;
  DN_CTLCOLORDIALOG       = DN_FIRST+2;
  DN_CTLCOLORDLGITEM      = DN_FIRST+3;
  DN_CTLCOLORDLGLIST      = DN_FIRST+4;
  DN_DRAWDIALOG           = DN_FIRST+5;
  DN_DRAWDLGITEM          = DN_FIRST+6;
  DN_EDITCHANGE           = DN_FIRST+7;
  DN_ENTERIDLE            = DN_FIRST+8;
  DN_GOTFOCUS             = DN_FIRST+9;
  DN_HELP                 = DN_FIRST+10;
  DN_HOTKEY               = DN_FIRST+11;
  DN_INITDIALOG           = DN_FIRST+12;
  DN_KILLFOCUS            = DN_FIRST+13;
  DN_LISTCHANGE           = DN_FIRST+14;
  DN_MOUSECLICK           = DN_FIRST+15;
  DN_DRAGGED              = DN_FIRST+16;
  DN_RESIZECONSOLE        = DN_FIRST+17;
  DN_MOUSEEVENT           = DN_FIRST+18;
  DN_DRAWDIALOGDONE       = DN_FIRST+19;
  DN_LISTHOTKEY           = DM_FIRST+20;

  DN_GETDIALOGINFO        = DM_GETDIALOGINFO;

  DN_CLOSE                = DM_CLOSE;
  DN_KEY                  = DM_KEY;

  DM_USER                 = $4000;

{ FARCHECKEDSTATE }

const
  BSTATE_UNCHECKED = 0;
  BSTATE_CHECKED   = 1;
  BSTATE_3STATE    = 2;
  BSTATE_TOGGLE    = 3;


{ FARLISTMOUSEREACTIONTYPE }

const
  LMRT_ONLYFOCUS = 0;
  LMRT_ALWAYS    = 1;
  LMRT_NEVER     = 2;

{FARCOMBOBOXEVENTTYPE}

const
  CBET_KEY         = 1;
  CBET_MOUSE       = 2;


{------------------------------------------------------------------------------}
{ List                                                                         }
{------------------------------------------------------------------------------}

{ LISTITEMFLAGS }

const
  LIF_SELECTED       = $00010000;
  LIF_CHECKED        = $00020000;
  LIF_SEPARATOR      = $00040000;
  LIF_DISABLE        = $00080000;
  LIF_GRAYED         = $00100000;
  LIF_HIDDEN         = $00200000;
  LIF_DELETEUSERDATA = $80000000;

(*
struct FarListItem
{
  DWORD Flags;
  const wchar_t *Text;
  DWORD Reserved[3];
};
*)
type
  PFarListItem = ^TFarListItem;
  TFarListItem = record
    Flags    :DWORD;
    TextPtr  :PFarChar;
    Reserved :array [0..2] of DWORD;
  end;

type
  PFarListItemArray = ^TFarListItemArray;
  TFarListItemArray = packed array[0..MaxInt div SizeOf(TFarListItem) - 1] of TFarListItem;

(*
struct FarListUpdate
{
  int Index;
  struct FarListItem Item;
};
*)
type
  PFarListUpdate = ^TFarListUpdate;
  TFarListUpdate = record
    Index :Integer;
    Item :TFarListItem;
  end;

(*
struct FarListInsert
{
  int Index;
  struct FarListItem Item;
};
*)
type
  PFarListInsert = ^TFarListInsert;
  TFarListInsert = record
    Index :Integer;
    Item :TFarListItem;
  end;

(*
struct FarListGetItem
{
  int ItemIndex;
  struct FarListItem Item;
};
*)
type
  PFarListGetItem = ^TFarListGetItem;
  TFarListGetItem = record
    ItemIndex :Integer;
    Item :TFarListItem;
  end;

(*
struct FarListPos
{
  int SelectPos;
  int TopPos;
};
*)
type
  PFarListPos = ^TFarListPos;
  TFarListPos = record
    SelectPos : Integer;
    TopPos : Integer;
  end;

{ FARLISTFINDFLAGS }

const
   LIFIND_EXACTMATCH = $00000001;

(*
struct FarListFind
{
  int StartIndex;
  const wchar_t *Pattern;
  DWORD Flags;
  DWORD Reserved;
};
*)
type
  PFarListFind = ^TFarListFind;
  TFarListFind = record
    StartIndex : Integer;
    Pattern : PFarChar;
    Flags : DWORD;
    Reserved : DWORD;
  end;

(*
struct FarListDelete
{
  int StartIndex;
  int Count;
};
*)
type
  PFarListDelete = ^TFarListDelete;
  TFarListDelete = record
    StartIndex : Integer;
    Count : Integer;
  end;

{ FARLISTINFOFLAGS }

const
  LINFO_SHOWNOBOX         = $00000400;
  LINFO_AUTOHIGHLIGHT     = $00000800;
  LINFO_REVERSEHIGHLIGHT  = $00001000;
  LINFO_WRAPMODE          = $00008000;
  LINFO_SHOWAMPERSAND     = $00010000;

(*
struct FarListInfo
{
  DWORD Flags;
  int ItemsNumber;
  int SelectPos;
  int TopPos;
  int MaxHeight;
  int MaxLength;
  DWORD Reserved[6];
};
*)
type
  PFarListInfo = ^TFarListInfo;
  TFarListInfo = record
    Flags :DWORD;
    ItemsNumber :Integer;
    SelectPos :Integer;
    TopPos :Integer;
    MaxHeight :Integer;
    MaxLength :Integer;
    Reserved :array [0..5] of DWORD;
 end;

(*
struct FarListItemData
{
  int   Index;
  int   DataSize;
  void *Data;
  DWORD Reserved;
};
*)
type
  PFarListItemData = ^TFarListItemData;
  TFarListItemData = record
    Index :Integer;
    DataSize :Integer;
    Data :Pointer;
    Reserved :DWORD;
  end;

(*
struct FarList
{
  int ItemsNumber;
  struct FarListItem *Items;
};
*)
type
  PFarList = ^TFarList;
  TFarList = record
    ItemsNumber : Integer;
    Items :PFarListItemArray;
  end;

(*
struct FarListTitles
{
  int   TitleLen;
  const wchar_t *Title;
  int   BottomLen;
  const wchar_t *Bottom;
};
*)
type
  PFarListTitles = ^TFarListTitles;
  TFarListTitles = record
    TitleLen :Integer;
    Title :PFarChar;
    BottomLen :Integer;
    Bottom :PFarChar;
 end;

(*
struct FarListColors{
  DWORD  Flags;
  DWORD  Reserved;
  int    ColorCount;
  LPBYTE Colors;
};
*)
type
  PFarListColors = ^TFarListColors;
  TFarListColors = record
    Flags :DWORD;
    Reserved :DWORD;
    ColorCount :Integer;
    Colors :PAnsiChar;
  end;


{------------------------------------------------------------------------------}
{ Dialogs                                                                      }
{------------------------------------------------------------------------------}

(*
struct FarDialogItem
{
  int Type;
  int X1,Y1,X2,Y2;
  int Focus;
  union
  {
    DWORD_PTR Reserved;
    int Selected;
    const wchar_t *History;
    const wchar_t *Mask;
    struct FarList *ListItems;
    int  ListPos;
    CHAR_INFO *VBuf;
  }
#ifdef _FAR_NO_NAMELESS_UNIONS
  Param
#endif
  ;
  DWORD Flags;
  int DefaultButton;

  const wchar_t *PtrData;
  size_t MaxLen; // terminate 0 not included (if == 0 string size is unlimited)
};
*)

type
  PFarDialogItem = ^TFarDialogItem;
  TFarDialogItem = record
    ItemType : Integer;
    X1, Y1, X2, Y2 : Integer;
    Focus : Integer;

    Param : record case Integer of
       0 : (Selected : Integer);
       1 : (History : PFarChar);
       2 : (Mask : PFarChar);
       3 : (ListItems : PFarList);
       4 : (ListPos : Integer);
       5 : (VBuf : PCharInfo);
    end;

    Flags : DWORD;
    DefaultButton : Integer;

    PtrData :PFarChar;
    MaxLen :SIZE_T; // terminate 0 not included (if == 0 string size is unlimited)
  end;

type
  PFarDialogItemArray = ^TFarDialogItemArray;
  TFarDialogItemArray = packed array[0..MaxInt div SizeOf(TFarDialogItem) - 1] of TFarDialogItem;

(*
struct FarDialogItemData
{
  size_t  PtrLength;
  wchar_t *PtrData;
};
*)
type
  PFarDialogItemData = ^TFarDialogItemData;
  TFarDialogItemData = record
    PtrLength : SIZE_T;
    PtrData : PFarChar;
  end;

(*
struct FarDialogEvent
{
  HANDLE hDlg;
  int Msg;
  int Param1;
  INT_PTR Param2;
  INT_PTR Result;
};
*)
type
  PFarDialogEvent = ^TFarDialogEvent;
  TFarDialogEvent = record
    hDlg :THandle;
    Msg :Integer;
    Param1 :Integer;
    Param2 :INT_PTR;
    Result :INT_PTR;
  end;

(*
struct OpenDlgPluginData
{
  int ItemNumber;
  HANDLE hDlg;
};
*)
type
  POpenDlgPluginData = ^TOpenDlgPluginData;
  TOpenDlgPluginData = record
    ItemNumber :Integer;
    hDlg :THandle;
  end;

(*
struct DialogInfo
{
  int StructSize;
  GUID Id;
};
*)
type
  PDialogInfo = ^TDialogInfo;
  TDialogInfo = record
    StructSize :Integer;
    Id :TGUID;
  end;

{ FARDIALOGFLAGS }

const
  FDLG_WARNING      = $00000001;
  FDLG_SMALLDIALOG  = $00000002;
  FDLG_NODRAWSHADOW = $00000004;
  FDLG_NODRAWPANEL  = $00000008;

type
(*
typedef INT_PTR (WINAPI *FARWINDOWPROC)(
  HANDLE   hDlg,
  int      Msg,
  int      Param1,
  INT_PTR Param2
);
*)
  TFarApiWindowProc = function (
    hDlg :THandle;
    Msg :Integer;
    Param1 :Integer;
    Param2 :INT_PTR
  ) :INT_PTR; stdcall;

(*
typedef INT_PTR (WINAPI *FARAPISENDDLGMESSAGE)(
  HANDLE   hDlg,
  int      Msg,
  int      Param1,
  INT_PTR Param2
);
*)
  TFarApiSendDlgMessage = function (
    hDlg : THandle;
    Msg : Integer;
    Param1 : Integer;
    Param2 : INT_PTR
  ) :INT_PTR; stdcall;

(*
typedef INT_PTR (WINAPI *FARAPIDEFDLGPROC)(
  HANDLE   hDlg,
  int      Msg,
  int      Param1,
  INT_PTR Param2
);
*)
  TFarApiDefDlgProc = function (
    hDlg :THandle;
    Msg :Integer;
    Param1 :Integer;
    Param2 :INT_PTR
  ) :INT_PTR; stdcall;

(*
typedef HANDLE (WINAPI *FARAPIDIALOGINIT)(
  INT_PTR               PluginNumber,
  int                   X1,
  int                   Y1,
  int                   X2,
  int                   Y2,
  const wchar_t        *HelpTopic,
  struct FarDialogItem *Item,
  unsigned int          ItemsNumber,
  DWORD                 Reserved,
  DWORD                 Flags,
  FARWINDOWPROC         DlgProc,
  INT_PTR              Param
);
*)
  TFarApiDialogInit = function (
    PluginNumber :INT_PTR;
    X1, Y1, X2, Y2 :Integer;
    HelpTopic :PFarChar;
    Item :PFarDialogItemArray;
    ItemsNumber :Integer;
    Reserved :DWORD;
    Flags :DWORD;
    DlgProc :TFarApiWindowProc;
    Param :INT_PTR
  ) :THandle; stdcall;

(*
typedef int (WINAPI *FARAPIDIALOGRUN)(
  HANDLE hDlg
);
*)
  TFarApiDialogRun = function(
    hDlg :THandle
  ) :Integer;  stdcall;

(*
typedef void (WINAPI *FARAPIDIALOGFREE)(
  HANDLE hDlg
);
*)
  TFarApiDialogFree = procedure(
    hDlg :THandle
  ); stdcall;


{------------------------------------------------------------------------------}
{ Menu                                                                         }
{------------------------------------------------------------------------------}

(*
struct FarMenuItem
{
  const wchar_t *Text;
  int  Selected;
  int  Checked;
  int  Separator;
};
*)
type
  PFarMenuItem = ^TFarMenuItem;
  TFarMenuItem = record
    TextPtr :PFarChar;
    Selected :Integer;
    Checked :Integer;
    Separator :Integer;
  end;

type
  PFarMenuItemArray = ^TFarMenuItemArray;
  TFarMenuItemArray = packed array[0..MaxInt div SizeOf(TFarMenuItem) - 1] of TFarMenuItem;


{ MENUITEMFLAGS }

const
  MIF_SELECTED   = $00010000;
  MIF_CHECKED    = $00020000;
  MIF_SEPARATOR  = $00040000;
  MIF_DISABLE    = $00080000;
  MIF_GRAYED     = $00100000;
  MIF_HIDDEN     = $00200000;

(*
struct FarMenuItemEx
{
  DWORD Flags;
  const wchar_t *Text;
  DWORD AccelKey;
  DWORD Reserved;
  DWORD_PTR UserData;
};
*)
type
  PFarMenuItemEx = ^TFarMenuItemEx;
  TFarMenuItemEx = record
    Flags : DWORD;
    TextPtr : PFarChar;
    AccelKey : DWORD;
    Reserved : DWORD;
    UserData : DWORD_PTR;
  end;


{ FARMENUFLAGS }

const
  FMENU_SHOWAMPERSAND        = $00000001;
  FMENU_WRAPMODE             = $00000002;
  FMENU_AUTOHIGHLIGHT        = $00000004;
  FMENU_REVERSEAUTOHIGHLIGHT = $00000008;
  FMENU_USEEXT               = $00000020;
  FMENU_CHANGECONSOLETITLE   = $00000040;

  {Obsolete}
//FMENU_TRUNCPATH            = $10000000;
//FMENU_TRUNCSTR             = $20000000;
//FMENU_TRUNCSTREND          = $30000000;

(*
typedef int (WINAPI *FARAPIMENU)(
  INT_PTR             PluginNumber,
  int                 X,
  int                 Y,
  int                 MaxHeight,
  DWORD               Flags,
  const wchar_t      *Title,
  const wchar_t      *Bottom,
  const wchar_t      *HelpTopic,
  const int          *BreakKeys,
  int                *BreakCode,
  const struct FarMenuItem *Item,
  int                 ItemsNumber
);
*)
type
  TFarApiMenu = function (
    PluginNumber : INT_PTR;
    X, Y : Integer;
    MaxHeight : Integer;
    Flags : DWORD;
    Title :PFarChar;
    Bottom :PFarChar;
    HelpTopic :PFarChar;
    BreakKeys :PIntegerArray;
    BreakCode :PIntegerArray;
    Item :PFarMenuItemArray;
    ItemsNumber :Integer
  ) : Integer; stdcall;


{------------------------------------------------------------------------------}
{ Panel                                                                        }
{------------------------------------------------------------------------------}

{ PLUGINPANELITEMFLAGS }

const
   PPIF_PROCESSDESCR = $80000000;
   PPIF_SELECTED     = $40000000;
   PPIF_USERDATA     = $20000000;

(*
struct FAR_FIND_DATA
{
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    unsigned __int64 nFileSize;
    unsigned __int64 nPackSize;
    wchar_t *lpwszFileName;
    wchar_t *lpwszAlternateFileName;
};
*)
type
  PFarFindData = ^TFarFindData;
  TFarFindData = record
    dwFileAttributes : DWORD;
    ftCreationTime : TFileTime;
    ftLastAccessTime : TFileTime;
    ftLastWriteTime : TFileTime;
    nFileSize :Int64;  // nFileSizeLow, nFileSizeHigh : DWORD;
    nPackSize :Int64;  // nPackSizeLow, nPackSizeHigh : DWORD;
    cFileName :PFarChar;
    cAlternateFileName :PFarChar;
  end;

  TFarFindDataArray = packed array [0..MaxInt div sizeof(TFarFindData) - 1] of TFarFindData;
  PFarFindDataArray = ^TFarFindDataArray;

(*
struct PluginPanelItem
{
  struct FAR_FIND_DATA FindData;
  DWORD         Flags;
  DWORD         NumberOfLinks;
  wchar_t      *Description;
  wchar_t      *Owner;
  wchar_t     **CustomColumnData;
  int           CustomColumnNumber;
  DWORD_PTR     UserData;
  DWORD         CRC32;
  DWORD_PTR     Reserved[2];
};
*)
type
  PPluginPanelItem = ^TPluginPanelItem;
  TPluginPanelItem = record
    FindData : TFarFindData;
    Flags : DWORD;
    NumberOfLinks : DWORD;
    Description : PFarChar;
    Owner : PFarChar;
    CustomColumnData : PPCharArray;
    CustomColumnNumber : Integer;
    UserData : DWORD_PTR;
    CRC32 : DWORD;
    Reserved : array [0..1] of DWORD_PTR;
  end;

  TPluginPanelItemArray = packed array[0..MaxInt div sizeof(TPluginPanelItem) - 1] of TPluginPanelItem;
  PPluginPanelItemArray = ^TPluginPanelItemArray;


{ PANELINFOFLAGS }

const
  PFLAGS_SHOWHIDDEN       = $00000001;
  PFLAGS_HIGHLIGHT        = $00000002;
  PFLAGS_REVERSESORTORDER = $00000004;
  PFLAGS_USESORTGROUPS    = $00000008;
  PFLAGS_SELECTEDFIRST    = $00000010;
  PFLAGS_REALNAMES        = $00000020;
  PFLAGS_NUMERICSORT      = $00000040;
  PFLAGS_PANELLEFT        = $00000080;
  PFLAGS_DIRECTORIESFIRST = $00000100;


{ PANELINFOTYPE }

const
  PTYPE_FILEPANEL  = 0;
  PTYPE_TREEPANEL  = 1;
  PTYPE_QVIEWPANEL = 2;
  PTYPE_INFOPANEL  = 3;

(*
struct PanelInfo
{
  int PanelType;
  int Plugin;
  RECT PanelRect;
  int ItemsNumber;
  int SelectedItemsNumber;
  int CurrentItem;
  int TopPanelItem;
  int Visible;
  int Focus;
  int ViewMode;
  int ShortNames;
  int SortMode;
  DWORD Flags;
  DWORD Reserved;
};
*)
type
  PPanelInfo = ^TPanelInfo;
  TPanelInfo = record
    PanelType : Integer;
    Plugin : Integer;
    PanelRect : TRect;
    ItemsNumber : Integer;
    SelectedItemsNumber : Integer;
    CurrentItem : Integer;
    TopPanelItem : Integer;
    Visible : Integer;
    Focus : Integer;
    ViewMode : Integer;
    ShortNames : Integer;
    SortMode : Integer;
    Flags : DWORD;
    Reserved : DWORD;
  end;

(*
struct PanelRedrawInfo
{
  int CurrentItem;
  int TopPanelItem;
};
*)
type
  PPanelRedrawInfo = ^TPanelRedrawInfo;
  TPanelRedrawInfo = record
    CurrentItem : Integer;
    TopPanelItem : Integer;
  end;


{------------------------------------------------------------------------------}
{                                                                              }
{------------------------------------------------------------------------------}

(*
struct CmdLineSelect
{
  int SelStart;
  int SelEnd;
};
*)
type
  PCmdLineSelect = ^TCmdLineSelect;
  TCmdLineSelect = record
    SelStart : Integer;
    SelEnd : Integer;
  end;

{ FILE_CONTROL_COMMANDS }

const
  PANEL_NONE                    = THandle(-1);
  PANEL_ACTIVE                  = THandle(-1);
  PANEL_PASSIVE                 = THandle(-2);

const
  FCTL_CLOSEPLUGIN              = 0;
  FCTL_GETPANELINFO             = 1;
  FCTL_UPDATEPANEL              = 2;
  FCTL_REDRAWPANEL              = 3;
  FCTL_GETCMDLINE               = 4;
  FCTL_SETCMDLINE               = 5;
  FCTL_SETSELECTION             = 6;
  FCTL_SETVIEWMODE              = 7;
  FCTL_INSERTCMDLINE            = 8;
  FCTL_SETUSERSCREEN            = 9;
  FCTL_SETPANELDIR              = 10;
  FCTL_SETCMDLINEPOS            = 11;
  FCTL_GETCMDLINEPOS            = 12;
  FCTL_SETSORTMODE              = 13;
  FCTL_SETSORTORDER             = 14;
  FCTL_GETCMDLINESELECTEDTEXT   = 15;
  FCTL_SETCMDLINESELECTION      = 16;
  FCTL_GETCMDLINESELECTION      = 17;
  FCTL_CHECKPANELSEXIST         = 18;
  FCTL_SETNUMERICSORT           = 19;
  FCTL_GETUSERSCREEN            = 20;
  FCTL_ISACTIVEPANEL            = 21;
  FCTL_GETPANELITEM             = 22;
  FCTL_GETSELECTEDPANELITEM     = 23;
  FCTL_GETCURRENTPANELITEM      = 24;
  FCTL_GETPANELDIR              = 25;
  FCTL_GETCOLUMNTYPES           = 26;
  FCTL_GETCOLUMNWIDTHS          = 27;
  FCTL_BEGINSELECTION           = 28;
  FCTL_ENDSELECTION             = 29;
  FCTL_CLEARSELECTION           = 30;
  FCTL_SETDIRECTORIESFIRST      = 31;
  FCTL_GETPANELFORMAT           = 32;
  FCTL_GETPANELHOSTFILE         = 33;


type
(*
typedef int (WINAPI *FARAPICONTROL)(
  HANDLE hPlugin,
  int Command,
    int Param1,
    INT_PTR Param2
);
*)
  TFarApiControl = function (
    hPlugin :THandle;
    Command :Integer;
    Param1 :Integer;
    Param2 :Pointer // INT_PTR
  ) :Integer; stdcall;

(*
typedef void (WINAPI *FARAPITEXT)(
  int X,
  int Y,
  int Color,
  const wchar_t *Str
);
*)
  TFarApiText = procedure (
    X, Y : Integer;
    Color : Integer;
    Str : PFarChar
   ); stdcall;

(*
typedef HANDLE (WINAPI *FARAPISAVESCREEN)(int X1, int Y1, int X2, int Y2);
*)
  TFarApiSaveScreen = function (
    X1, Y1, X2, Y2 : Integer
  ) :THandle; stdcall;

(*
typedef void (WINAPI *FARAPIRESTORESCREEN)(HANDLE hScreen);
*)
  TFarApiRestoreScreen = procedure (
     hScreen : THandle
  ); stdcall;

(*
typedef int (WINAPI *FARAPIGETDIRLIST)(
  const wchar_t *Dir,
  struct FAR_FIND_DATA **pPanelItem,
  int *pItemsNumber
);
*)
  TFarApiGetDirList = function (
    Dir : PFarChar;
    var Items : PFarFindDataArray;
    var ItemsNumber : Integer
  ) : Integer; stdcall;

(*
typedef void (WINAPI *FARAPIFREEDIRLIST)(
  struct FAR_FIND_DATA *PanelItem,
  int nItemsNumber
);
*)
  TFarApiFreeDirList = procedure (
    Items : PFarFindDataArray;
    ItemsNumber :Integer
  ); stdcall;


(*
typedef int (WINAPI *FARAPIGETPLUGINDIRLIST)(
  INT_PTR PluginNumber,
  HANDLE hPlugin,
  const wchar_t *Dir,
  struct PluginPanelItem **pPanelItem,
  int *pItemsNumber
);
*)
  TFarApiGetPluginDirList = function (
    PluginNumber : INT_PTR;
    hPlugin : THandle;
    Dir : PFarChar;
    var PanelItem :PPluginPanelItemArray;
    var ItemsNumber :Integer
  ) : Integer; stdcall;

(*
typedef void (WINAPI *FARAPIFREEPLUGINDIRLIST)(struct PluginPanelItem *PanelItem, int nItemsNumber);
*)
  TFarApiFreePluginDirList = procedure (
    PanelItem : PPluginPanelItemArray;
    ItemsNumber :Integer
  ); stdcall;


{------------------------------------------------------------------------------}
{ Viewer / Editor                                                              }
{------------------------------------------------------------------------------}

{ VIEWER_FLAGS }

const
  VF_NONMODAL              = $00000001;
  VF_DELETEONCLOSE         = $00000002;
  VF_ENABLE_F6             = $00000004;
  VF_DISABLEHISTORY        = $00000008;
  VF_IMMEDIATERETURN       = $00000100;
  VF_DELETEONLYFILEONCLOSE = $00000200;

(*
typedef int (WINAPI *FARAPIVIEWER)(
  const wchar_t *FileName,
  const wchar_t *Title,
  int X1,
  int Y1,
  int X2,
  int Y2,
  DWORD Flags,
  UINT CodePage
);
*)
type
  TFarApiViewer = function (
    FileName : PFarChar;
    Title : PFarChar;
    X1, Y1, X2, Y2 : Integer;
    Flags : DWORD;
    CodePage : UINT
  ) : Integer; stdcall;


{ EDITOR_FLAGS }

const
  EF_NONMODAL              = $00000001;
  EF_CREATENEW             = $00000002;
  EF_ENABLE_F6             = $00000004;
  EF_DISABLEHISTORY        = $00000008;
  EF_DELETEONCLOSE         = $00000010;
  EF_IMMEDIATERETURN       = $00000100;
  EF_DELETEONLYFILEONCLOSE = $00000200;

{ EDITOR_EXITCODE }

const
  EEC_OPEN_ERROR          = 0;
  EEC_MODIFIED            = 1;
  EEC_NOT_MODIFIED        = 2;
  EEC_LOADING_INTERRUPTED = 3;

type
(*
typedef int (WINAPI *FARAPIEDITOR)(
  const wchar_t *FileName,
  const wchar_t *Title,
  int X1,
  int Y1,
  int X2,
  int Y2,
  DWORD Flags,
  int StartLine,
  int StartChar,
  UINT CodePage
);
*)
  TFarApiEditor = function (
    FileName : PFarChar;
    Title : PFarChar;
    X1, Y1, X2, Y2 : Integer;
    Flags : DWORD;
    StartLine : Integer;
    StartChar : Integer;
    CodePage : UINT
  ) : Integer; stdcall;

(*
typedef int (WINAPI *FARAPICMPNAME)(
  const wchar_t *Pattern,
  const wchar_t *String,
  int SkipPath
);
*)
  TFarApiCmpName = function (
    Pattern : PFarChar;
    aString : PFarChar;
    SkipPath : Integer
  ) : Integer; stdcall;


type
(*
typedef const wchar_t* (WINAPI *FARAPIGETMSG)(
  INT_PTR PluginNumber,
  int MsgId
);
*)
  TFarApiGetMsg = function (
    PluginNumber :INT_PTR;
    MsgId :Integer
  ) :PFarChar; stdcall;


{ FarHelpFlags }

const
  FHELP_NOSHOWERROR = $80000000;
  FHELP_SELFHELP    = $00000000;
  FHELP_FARHELP     = $00000001;
  FHELP_CUSTOMFILE  = $00000002;
  FHELP_CUSTOMPATH  = $00000004;
  FHELP_USECONTENTS = $40000000;


(*
typedef BOOL (WINAPI *FARAPISHOWHELP)(
  const wchar_t *ModuleName,
  const wchar_t *Topic,
  DWORD Flags
);
*)
type
  TFarApiShowHelp = function (
    ModuleName : PFarChar;
    Topic : PFarChar;
    Flags : DWORD
  ) :LongBool; stdcall;


{ ADVANCED_CONTROL_COMMANDS }

const
  ACTL_GETFARVERSION        = 0;
//ACTL_CONSOLEMODE          = 1;
  ACTL_GETSYSWORDDIV        = 2;
  ACTL_WAITKEY              = 3;
  ACTL_GETCOLOR             = 4;
  ACTL_GETARRAYCOLOR        = 5;
  ACTL_EJECTMEDIA           = 6;
  ACTL_KEYMACRO             = 7;
  ACTL_POSTKEYSEQUENCE      = 8;
  ACTL_GETWINDOWINFO        = 9;
  ACTL_GETWINDOWCOUNT       = 10;
  ACTL_SETCURRENTWINDOW     = 11;
  ACTL_COMMIT               = 12;
  ACTL_GETFARHWND           = 13;
  ACTL_GETSYSTEMSETTINGS    = 14;
  ACTL_GETPANELSETTINGS     = 15;
  ACTL_GETINTERFACESETTINGS = 16;
  ACTL_GETCONFIRMATIONS     = 17;
  ACTL_GETDESCSETTINGS      = 18;
  ACTL_SETARRAYCOLOR        = 19;
//ACTL_GETWCHARMODE         = 20;
  ACTL_GETPLUGINMAXREADDATA = 21;
  ACTL_GETDIALOGSETTINGS    = 22;
  ACTL_GETSHORTWINDOWINFO   = 23;
  ACTL_REDRAWALL            = 27;
  ACTL_SYNCHRO              = 28;
  ACTL_SETPROGRESSSTATE     = 29;
  ACTL_SETPROGRESSVALUE     = 30;
  ACTL_QUIT                 = 31;
  ACTL_GETFARRECT           = 32;
  ACTL_GETCURSORPOS         = 33;
  ACTL_SETCURSORPOS         = 34;


{ FarSystemSettings }

const
  FSS_CLEARROATTRIBUTE           = $00000001;
  FSS_DELETETORECYCLEBIN         = $00000002;
  FSS_USESYSTEMCOPYROUTINE       = $00000004;
  FSS_COPYFILESOPENEDFORWRITING  = $00000008;
  FSS_CREATEFOLDERSINUPPERCASE   = $00000010;
  FSS_SAVECOMMANDSHISTORY        = $00000020;
  FSS_SAVEFOLDERSHISTORY         = $00000040;
  FSS_SAVEVIEWANDEDITHISTORY     = $00000080;
  FSS_USEWINDOWSREGISTEREDTYPES  = $00000100;
  FSS_AUTOSAVESETUP              = $00000200;
  FSS_SCANSYMLINK                = $00000400;

{ FarPanelSettings }

const
  FPS_SHOWHIDDENANDSYSTEMFILES    = $00000001;
  FPS_HIGHLIGHTFILES              = $00000002;
  FPS_AUTOCHANGEFOLDER            = $00000004;
  FPS_SELECTFOLDERS               = $00000008;
  FPS_ALLOWREVERSESORTMODES       = $00000010;
  FPS_SHOWCOLUMNTITLES            = $00000020;
  FPS_SHOWSTATUSLINE              = $00000040;
  FPS_SHOWFILESTOTALINFORMATION   = $00000080;
  FPS_SHOWFREESIZE                = $00000100;
  FPS_SHOWSCROLLBAR               = $00000200;
  FPS_SHOWBACKGROUNDSCREENSNUMBER = $00000400;
  FPS_SHOWSORTMODELETTER          = $00000800;

{ FarDialogSettings }

const
  FDIS_HISTORYINDIALOGEDITCONTROLS    = $00000001;
  FDIS_PERSISTENTBLOCKSINEDITCONTROLS = $00000002;
  FDIS_AUTOCOMPLETEININPUTLINES       = $00000004;
  FDIS_BSDELETEUNCHANGEDTEXT          = $00000008;
  FDIS_DELREMOVESBLOCKS               = $00000010;
  FDIS_MOUSECLICKOUTSIDECLOSESDIALOG  = $00000020;

{ FarInterfaceSettings }

const
  FIS_CLOCKINPANELS                  = $00000001;
  FIS_CLOCKINVIEWERANDEDITOR         = $00000002;
  FIS_MOUSE                          = $00000004;
  FIS_SHOWKEYBAR                     = $00000008;
  FIS_ALWAYSSHOWMENUBAR              = $00000010;
  FIS_USERIGHTALTASALTGR             = $00000080;
  FIS_SHOWTOTALCOPYPROGRESSINDICATOR = $00000100;
  FIS_SHOWCOPYINGTIMEINFO            = $00000200;
  FIS_USECTRLPGUPTOCHANGEDRIVE       = $00000800;
  FIS_SHOWTOTALDELPROGRESSINDICATOR  = $00001000;

{ FarConfirmationsSettings }

const
  FCS_COPYOVERWRITE          = $00000001;
  FCS_MOVEOVERWRITE          = $00000002;
  FCS_DRAGANDDROP            = $00000004;
  FCS_DELETE                 = $00000008;
  FCS_DELETENONEMPTYFOLDERS  = $00000010;
  FCS_INTERRUPTOPERATION     = $00000020;
  FCS_DISCONNECTNETWORKDRIVE = $00000040;
  FCS_RELOADEDITEDFILE       = $00000080;
  FCS_CLEARHISTORYLIST       = $00000100;
  FCS_EXIT                   = $00000200;
  FCS_OVERWRITEDELETEROFILES = $00000400;

{ FarDescriptionSettings }

const
  FDS_UPDATEALWAYS      = $00000001;
  FDS_UPDATEIFDISPLAYED = $00000002;
  FDS_SETHIDDEN         = $00000004;
  FDS_UPDATEREADONLY    = $00000008;


//const
//  FAR_CONSOLE_GET_MODE       = -2;
//  FAR_CONSOLE_TRIGGER        = -1;
//  FAR_CONSOLE_SET_WINDOWED   = 0;
//  FAR_CONSOLE_SET_FULLSCREEN = 1;
//  FAR_CONSOLE_WINDOWED       = 0;
//  FAR_CONSOLE_FULLSCREEN     = 1;


{ FAREJECTMEDIAFLAGS }

const
  EJECT_NO_MESSAGE          = $00000001;
  EJECT_LOAD_MEDIA          = $00000002;

(*
struct ActlEjectMedia {
  DWORD Letter;
  DWORD Flags;
};
*)
type
  PActlEjectMedia = ^TActlEjectMedia;
  TActlEjectMedia = record
    Letter :DWORD;
    Flags :DWORD;
  end;


{ FARKEYSEQUENCEFLAGS }

const
  KSFLAGS_DISABLEOUTPUT       = $00000001;
  KSFLAGS_NOSENDKEYSTOPLUGINS = $00000002;
  KSFLAGS_REG_MULTI_SZ        = $00100000;
  KSFLAGS_SILENTCHECK         = $00000001;

(*
struct KeySequence{
  DWORD Flags;
  int Count;
  DWORD *Sequence;
};
*)
type
  PKeySequence = ^TKeySequence;
  TKeySequence = record
    Flags :DWORD;
    Count :Integer;
    Sequence :^DWORD;
  end;

{ FARMACROCOMMAND }

const
  MCMD_LOADALL         = 0;
  MCMD_SAVEALL         = 1;
  MCMD_POSTMACROSTRING = 2;
  MCMD_CHECKMACRO      = 4;
  MCMD_GETSTATE        = 5;

{ FARMACROSTATE }

const
  MACROSTATE_NOMACRO          = 0;
  MACROSTATE_EXECUTING        = 1;
  MACROSTATE_EXECUTING_COMMON = 2;
  MACROSTATE_RECORDING        = 3;
  MACROSTATE_RECORDING_COMMON = 4;

{FARMACROPARSEERRORCODE}

const
  MPEC_SUCCESS                =  0;
  MPEC_UNRECOGNIZED_KEYWORD   =  1;
  MPEC_UNRECOGNIZED_FUNCTION  =  2;
  MPEC_FUNC_PARAM             =  3;
  MPEC_NOT_EXPECTED_ELSE      =  4;
  MPEC_NOT_EXPECTED_END       =  5;
  MPEC_UNEXPECTED_EOS         =  6;
  MPEC_EXPECTED_TOKEN         =  7;
  MPEC_BAD_HEX_CONTROL_CHAR   =  8;
  MPEC_BAD_CONTROL_CHAR       =  9;
  MPEC_VAR_EXPECTED           = 10;
  MPEC_EXPR_EXPECTED          = 11;
  MPEC_ZEROLENGTHMACRO        = 12;
  MPEC_INTPARSERERROR         = 13;
  MPEC_CONTINUE_OTL           = 14;

(*
struct MacroParseResult
{
    DWORD ErrCode;
    COORD ErrPos;
    const wchar_t *ErrSrc;
};
*)
type
  PMacroParseResult = ^TMacroParseResult;
  TMacroParseResult = record
    ErrCode :DWORD;
    ErrPos  :COORD;
    ErrSrc  :PFarChar;
  end;

(*
struct ActlKeyMacro
{
    int Command;
    union
    {
        struct
        {
            const wchar_t *SequenceText;
            DWORD Flags;
        } PlainText;
        struct MacroParseResult MacroResult;
        DWORD_PTR Reserved[3];
    } Param;
};
*)
type
  PPlainText = ^TPlainText;
  TPlainText = record
    SequenceText : PFarChar;
    Flags : DWORD;
  end;

  PActlKeyMacro = ^TActlKeyMacro;
  TActlKeyMacro = record
    Command : Integer;

    Param : record case Integer of
      0 : (PlainText :TPlainText);
      1 : (MacroResult :TMacroParseResult);
      2 : (Reserved :array [0..2] of DWORD_PTR);
    end;
  end;


{ FARCOLORFLAGS }

const
  FCLR_REDRAW = $00000001;

(*
struct FarSetColors{
  DWORD Flags;
  int StartIndex;
  int ColorCount;
  LPBYTE Colors;
};
*)
type
  PFarSetColors = ^TFarSetColors;
  TFarSetColors = record
    Flags : DWORD;
    StartIndex : Integer;
    ColorCount : Integer;
    Colors : PAnsiChar;
  end;


{ WINDOWINFO_TYPE }

const
  WTYPE_PANELS     = 1;
  WTYPE_VIEWER     = 2;
  WTYPE_EDITOR     = 3;
  WTYPE_DIALOG     = 4;
  WTYPE_VMENU      = 5;
  WTYPE_HELP       = 6;

(*
struct WindowInfo
{
  int  Pos;
  int  Type;
  int  Modified;
  int  Current;
  wchar_t *TypeName;
  int TypeNameSize;
  wchar_t *Name;
  int NameSize;
};
*)
type
  PWindowInfo = ^TWindowInfo;
  TWindowInfo = record
    Pos :Integer;
    WindowType :Integer;
    Modified :Integer;
    Current :Integer;
    TypeName :PFarChar;
    TypeNameSize :Integer;
    Name :PFarChar;
    NameSize :Integer;
  end;


{ PROGRESSTATE }

const
  PS_NOPROGRESS    = 0;
  PS_INDETERMINATE = 1;
  PS_NORMAL        = 2;
  PS_ERROR         = 4;
  PS_PAUSED        = 8;

(*
struct PROGRESSVALUE
{
    unsigned __int64 Completed;
    unsigned __int64 Total;
};
*)
type
  PProgressValue = ^TProgressValue;
  TProgressValue = record
    Completed :Int64;
    Total :Int64;
  end;

(*
typedef INT_PTR (WINAPI *FARAPIADVCONTROL)(
  INT_PTR ModuleNumber,
  int Command,
  void *Param
);
*)
type
  TFarApiAdvControl = function (
    ModuleNumber :INT_PTR;
    Command :Integer;
    Param :Pointer
  ) :INT_PTR; stdcall;


{ VIEWER_CONTROL_COMMANDS }

const
  VCTL_GETINFO     = 0;
  VCTL_QUIT        = 1;
  VCTL_REDRAW      = 2;
  VCTL_SETKEYBAR   = 3;
  VCTL_SETPOSITION = 4;
  VCTL_SELECT      = 5;
  VCTL_SETMODE     = 6;


{ VIEWER_OPTIONS }

const
  VOPT_SAVEFILEPOSITION = 1;
  VOPT_AUTODETECTTABLE  = 2;


{VIEWER_SETMODE_TYPES}

const
  VSMT_HEX       = 0;
  VSMT_WRAP      = 1;
  VSMT_WORDWRAP  = 2;


{VIEWER_SETMODEFLAGS_TYPES}

const
  VSMFL_REDRAW   = $00000001;

(*
struct ViewerSetMode {
  int Type;
  union {
    int iParam;
    wchar_t *wszParam;
  } Param;
  DWORD Flags;
  DWORD Reserved;
};
*)

type
  PViewerSetMode = ^TViewerSetMode;
  TViewerSetMode = record
    ParamType : Integer;

    Param : record case Integer of
      0 : (iParam : Integer);
      1 : (wszParam : PFarChar);
    end;

    Flags : DWORD;
    Reserved : DWORD;
  end;

(*
struct ViewerSelect
{
  __int64 BlockStartPos;
  int     BlockLen;
};
*)
type
  TFarInt64Part = record
    LowPart :DWORD;
    HighPart :DWORD;
  end;

  TFarInt64 = record
    case Integer of
      0 : (i64 : Int64);
      1 : (Part : TFarInt64Part);
  end;

  PViewerSelect = ^TViewerSelect;
  TViewerSelect = record
    BlockStartPos :TFarInt64;
    BlockLen :Integer;
  end;

{ VIEWER_SETPOS_FLAGS }

const
  VSP_NOREDRAW    = $0001;
  VSP_PERCENT     = $0002;
  VSP_RELATIVE    = $0004;
  VSP_NORETNEWPOS = $0008;

(*
struct ViewerSetPosition
{
  DWORD Flags;
  __int64 StartPos;
  __int64 LeftPos;
};
*)
type
  PViewerSetPosition = ^TViewerSetPosition;
  TViewerSetPosition = record
    Flags : DWORD;
    StartPos : TFarInt64;
    LeftPos : TFarInt64;
  end;

(*
struct ViewerMode{
  UINT CodePage;
  int Wrap;
  int WordWrap;
  int Hex;
  DWORD Reserved[4];
};
*)
type
  PViewerMode = ^TViewerMode;
  TViewerMode = record
    CodePage :UINT;
    Wrap : Integer;
    WordWrap : Integer;
    Hex : Integer;
    Reserved : array [0..3] of DWORD;
  end;

(*
struct ViewerInfo
{
  int    StructSize;
  int    ViewerID;
  const wchar_t *FileName;
  __int64 FileSize;
  __int64 FilePos;
  int    WindowSizeX;
  int    WindowSizeY;
  DWORD  Options;
  int    TabSize;
  struct ViewerMode CurMode;
  __int64 LeftPos;
};
*)
type
  PViewerInfo = ^TViewerInfo;
  TViewerInfo = record
    StructSize : Integer;
    ViewerID : Integer;
    FileName : PFarChar;
    FileSize : TFarInt64;
    FilePos : TFarInt64;
    WindowSizeX : Integer;
    WindowSizeY : Integer;
    Options : DWORD;
    TabSize : Integer;
    CurMode : TViewerMode;
    LeftPos : TFarInt64;
  end;

(*
typedef int (WINAPI *FARAPIVIEWERCONTROL)(
  int Command,
  void *Param
);
*)
type
  TFarApiViewerControl = function (
    Command : Integer;
    Param : Pointer
  ) : Integer; stdcall;


{ VIEWER_EVENTS }

const
  VE_READ      = 0;
  VE_CLOSE     = 1;
  VE_GOTFOCUS  = 6;
  VE_KILLFOCUS = 7;


{ EDITOR_EVENTS }

const
  EE_READ       = 0;
  EE_SAVE       = 1;
  EE_REDRAW     = 2;
  EE_CLOSE      = 3;
  EE_GOTFOCUS   = 6;
  EE_KILLFOCUS  = 7;


{ DIALOG_EVENTS }

const
  DE_DLGPROCINIT    = 0;
  DE_DEFDLGPROCINIT = 1;
  DE_DLGPROCEND     = 2;


{ SYNCHRO_EVENTS }

const
  SE_COMMONSYNCHRO  = 0;


const
  EEREDRAW_ALL    = Pointer(0);


{ EDITOR_CONTROL_COMMANDS }

const
  ECTL_GETSTRING           = 0;
  ECTL_SETSTRING           = 1;
  ECTL_INSERTSTRING        = 2;
  ECTL_DELETESTRING        = 3;
  ECTL_DELETECHAR          = 4;
  ECTL_INSERTTEXT          = 5;
  ECTL_GETINFO             = 6;
  ECTL_SETPOSITION         = 7;
  ECTL_SELECT              = 8;
  ECTL_REDRAW              = 9;
  ECTL_TABTOREAL           = 10;
  ECTL_REALTOTAB           = 11;
  ECTL_EXPANDTABS          = 12;
  ECTL_SETTITLE            = 13;
  ECTL_READINPUT           = 14;
  ECTL_PROCESSINPUT        = 15;
  ECTL_ADDCOLOR            = 16;
  ECTL_GETCOLOR            = 17;
  ECTL_SAVEFILE            = 18;
  ECTL_QUIT                = 19;
  ECTL_SETKEYBAR           = 20;
  ECTL_PROCESSKEY          = 21;
  ECTL_SETPARAM            = 22;
  ECTL_GETBOOKMARKS        = 23;
  ECTL_TURNOFFMARKINGBLOCK = 24;
  ECTL_DELETEBLOCK         = 25;
  ECTL_ADDSTACKBOOKMARK    = 26;
  ECTL_PREVSTACKBOOKMARK   = 27;
  ECTL_NEXTSTACKBOOKMARK   = 28;
  ECTL_CLEARSTACKBOOKMARKS = 29;
  ECTL_DELETESTACKBOOKMARK = 30;
  ECTL_GETSTACKBOOKMARKS   = 31;
  ECTL_UNDOREDO            = 32;
  ECTL_GETFILENAME         = 33;


{ EDITOR_SETPARAMETER_TYPES }

const
  ESPT_TABSIZE          = 0;
  ESPT_EXPANDTABS       = 1;
  ESPT_AUTOINDENT       = 2;
  ESPT_CURSORBEYONDEOL  = 3;
  ESPT_CHARCODEBASE     = 4;
  ESPT_CODEPAGE         = 5;
  ESPT_SAVEFILEPOSITION = 6;
  ESPT_LOCKMODE         = 7;
  ESPT_SETWORDDIV       = 8;
  ESPT_GETWORDDIV       = 9;
  ESPT_SHOWWHITESPACE   = 10;
  ESPT_SETBOM           = 11;


(*
struct EditorSetParameter
{
  int Type;
  union {
    int iParam;
    wchar_t *wszParam;
    DWORD Reserved1;
  } Param;
  DWORD Flags;
  DWORD Size;
};
*)
type
  PEditorSetParameter = ^TEditorSetParameter;
  TEditorSetParameter = record
    ParamType : Integer;
    Param : record case Integer of
       0 : (iParam : Integer);
       1 : (wszParam : PFarChar);
       2 : (Reserved : DWORD);
    end;
    Flags : DWORD;
    Size : DWORD;
  end;


{EDITOR_UNDOREDO_COMMANDS}

const
  EUR_BEGIN = 0;
  EUR_END   = 1;
  EUR_UNDO  = 2;
  EUR_REDO  = 3;

(*
struct EditorUndoRedo
{
  int Command;
  DWORD_PTR Reserved[3];
};
*)
type
  PEditorUndoRedo = ^TEditorUndoRedo;
  TEditorUndoRedo = record
    Command :Integer;
    Reserved :array[0..2] of DWORD_PTR;
  end;

(*
struct EditorGetString
{
  int StringNumber;
  const wchar_t *StringText;
  const wchar_t *StringEOL;
  int StringLength;
  int SelStart;
  int SelEnd;
};
*)
type
  PEditorGetString = ^TEditorGetString;
  TEditorGetString = record
    StringNumber : Integer;
    StringText : PFarChar;
    StringEOL : PFarChar;
    StringLength : Integer;
    SelStart : Integer;
    SelEnd : Integer;
  end;

(*
struct EditorSetString
{
  int StringNumber;
  const wchar_t *StringText;
  const wchar_t *StringEOL;
  int StringLength;
};
*)
type
  PEditorSetString = ^TEditorSetString;
  TEditorSetString = record
    StringNumber : Integer;
    StringText : PFarChar;
    StringEOL : PFarChar;
    StringLength : Integer;
  end;

{ EXPAND_TABS }

const
  EXPAND_NOTABS  = 0;
  EXPAND_ALLTABS = 1;
  EXPAND_NEWTABS = 2;


{ EDITOR_OPTIONS }

const
  EOPT_EXPANDALLTABS      = $00000001;
  EOPT_PERSISTENTBLOCKS   = $00000002;
  EOPT_DELREMOVESBLOCKS   = $00000004;
  EOPT_AUTOINDENT         = $00000008;
  EOPT_SAVEFILEPOSITION   = $00000010;
  EOPT_AUTODETECTCODEPAGE = $00000020;
  EOPT_CURSORBEYONDEOL    = $00000040;
  EOPT_EXPANDONLYNEWTABS  = $00000080;
  EOPT_SHOWWHITESPACE     = $00000100;
  EOPT_BOM                = $00000200;


{ EDITOR_BLOCK_TYPES }

const
  BTYPE_NONE   = 0;
  BTYPE_STREAM = 1;
  BTYPE_COLUMN = 2;


{ EDITOR_CURRENTSTATE }

const
  ECSTATE_MODIFIED = $00000001;
  ECSTATE_SAVED    = $00000002;
  ECSTATE_LOCKED   = $00000004;

(*
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
  DWORD Reserved[5];
};
*)
type
  PEditorInfo = ^TEditorInfo;
  TEditorInfo = record
    EditorID : Integer;
    WindowSizeX : Integer;
    WindowSizeY : Integer;
    TotalLines : Integer;
    CurLine : Integer;
    CurPos : Integer;
    CurTabPos : Integer;
    TopScreenLine : Integer;
    LeftPos : Integer;
    Overtype : Integer;
    BlockType : Integer;
    BlockStartLine : Integer;
    Options : DWORD;
    TabSize : Integer;
    BookMarkCount : Integer;
    CurState : DWORD;
    CodePage : UINT;
    Reserved : array [0..4] of DWORD;
  end;

(*
struct EditorBookMarks
{
  long *Line;
  long *Cursor;
  long *ScreenLine;
  long *LeftPos;
  DWORD Reserved[4];
};
*)
type
  PEditorBookMarks = ^TEditorBookMarks;
  TEditorBookMarks = record
    Line : PIntegerArray;
    Cursor : PIntegerArray;
    ScreenLine : PIntegerArray;
    LeftPos : PIntegerArray;
    Reserved : array [0..3] of DWORD;
  end;

(*
struct EditorSetPosition
{
  int CurLine;
  int CurPos;
  int CurTabPos;
  int TopScreenLine;
  int LeftPos;
  int Overtype;
};
*)
type
  PEditorSetPosition = ^TEditorSetPosition;
  TEditorSetPosition = record
    CurLine : Integer;
    CurPos : Integer;
    CurTabPos : Integer;
    TopScreenLine : Integer;
    LeftPos : Integer;
    Overtype : Integer;
  end;

(*
struct EditorSelect
{
  int BlockType;
  int BlockStartLine;
  int BlockStartPos;
  int BlockWidth;
  int BlockHeight;
};
*)
type
  PEditorSelect = ^TEditorSelect;
  TEditorSelect = record
    BlockType : Integer;
    BlockStartLine : Integer;
    BlockStartPos : Integer;
    BlockWidth : Integer;
    BlockHeight : Integer;
  end;

(*
struct EditorConvertPos
{
  int StringNumber;
  int SrcPos;
  int DestPos;
};
*)
type
  PEditorConvertPos = ^TEditorConvertPos;
  TEditorConvertPos = record
    StringNumber : Integer;
    SrcPos : Integer;
    DestPos : Integer;
  end;


{ EDITORCOLORFLAGS }

const
   ECF_TAB1 = $10000;

(*
struct EditorColor
{
  int StringNumber;
  int ColorItem;
  int StartPos;
  int EndPos;
  int Color;
};
*)
type
  PEditorColor = ^TEditorColor;
  TEditorColor = record
    StringNumber :Integer;
    ColorItem :Integer;
    StartPos :Integer;
    EndPos :Integer;
    Color :Integer;
  end;

(*
struct EditorSaveFile
{
  const wchar_t *FileName;
  const wchar_t *FileEOL;
  UINT CodePage;
};
*)
type
  PEditorSaveFile = ^TEditorSaveFile;
  TEditorSaveFile = record
    FileName :PFarChar;
    FileEOL :PFarChar;
    CodePage :UINT;
  end;


(*
typedef int (WINAPI *FARAPIEDITORCONTROL)(
  int Command,
  void *Param
);
*)
type
  TFarApiEditorControl = function (
    Command : Integer;
    Param : Pointer
  ) : Integer; stdcall;


{ INPUTBOXFLAGS }

const
  FIB_ENABLEEMPTY      = $00000001;
  FIB_PASSWORD         = $00000002;
  FIB_EXPANDENV        = $00000004;
  FIB_NOUSELASTHISTORY = $00000008;
  FIB_BUTTONS          = $00000010;
  FIB_NOAMPERSAND      = $00000020;
  FIB_EDITPATH         = $01000000;

(*
typedef int (WINAPI *FARAPIINPUTBOX)(
  const wchar_t *Title,
  const wchar_t *SubTitle,
  const wchar_t *HistoryName,
  const wchar_t *SrcText,
  wchar_t *DestText,
  int   DestLength,
  const wchar_t *HelpTopic,
  DWORD Flags
);
*)
type
  TFarApiInputBox = function (
    Title : PFarChar;
    SubTitle : PFarChar;
    HistoryName : PFarChar;
    SrcText : PFarChar;
    DestText : PFarChar;
    DestLength : Integer;
    HelpTopic : PFarChar;
    Flags : DWORD
  ) : Integer; stdcall;

(*
typedef int (WINAPI *FARAPIPLUGINSCONTROL)(
  HANDLE hHandle,
  int Command,
  int Param1,
  INT_PTR Param2
);
*)
type
  TFarApiPluginsControl = function(
    hHandle :THandle;
    Command :Integer;
    Param1 :Integer;
    Param2 :Pointer //INT_PTR
  ) : Integer; stdcall;

(*
typedef int (WINAPI *FARAPIFILEFILTERCONTROL)(
  HANDLE hHandle,
  int Command,
  int Param1,
  INT_PTR Param2
);
*)
type
  TFarApiFilterControl = function(
    hHandle :THandle;
    Command :Integer;
    Param1 :Integer;
    Param2 :Pointer //INT_PTR
  ) : Integer; stdcall;


(*
typedef int (WINAPI *FARAPIREGEXPCONTROL)(
  HANDLE hHandle,
  int Command,
  INT_PTR Param
);
*)
type
  TFarApiRegexpControl = function(
    hHandle :THandle;
    Command :Integer;
    Param :Pointer  //INT_PTR
  ) :Integer; stdcall;

(*
// <C&C++>
typedef int     (WINAPIV *FARSTDSPRINTF)(wchar_t *Buffer,const wchar_t *Format,...);
typedef int     (WINAPIV *FARSTDSNPRINTF)(wchar_t *Buffer,size_t Sizebuf,const wchar_t *Format,...);
typedef int     (WINAPIV *FARSTDSSCANF)(const wchar_t *Buffer, const wchar_t *Format,...);
// </C&C++>
typedef void    (WINAPI *FARSTDQSORT)(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void * ));
typedef void    (WINAPI *FARSTDQSORTEX)(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *,void *userparam),void *userparam);
typedef void   *(WINAPI *FARSTDBSEARCH)(const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void * ));
typedef int     (WINAPI *FARSTDGETFILEOWNER)(const wchar_t *Computer,const wchar_t *Name,wchar_t *Owner,int Size);
typedef int     (WINAPI *FARSTDGETNUMBEROFLINKS)(const wchar_t *Name);
typedef int     (WINAPI *FARSTDATOI)(const wchar_t *s);
typedef __int64 (WINAPI *FARSTDATOI64)(const wchar_t *s);
typedef wchar_t   *(WINAPI *FARSTDITOA64)(__int64 value, wchar_t *string, int radix);
typedef wchar_t   *(WINAPI *FARSTDITOA)(int value, wchar_t *string, int radix);
typedef wchar_t   *(WINAPI *FARSTDLTRIM)(wchar_t *Str);
typedef wchar_t   *(WINAPI *FARSTDRTRIM)(wchar_t *Str);
typedef wchar_t   *(WINAPI *FARSTDTRIM)(wchar_t *Str);
typedef wchar_t   *(WINAPI *FARSTDTRUNCSTR)(wchar_t *Str,int MaxLength);
typedef wchar_t   *(WINAPI *FARSTDTRUNCPATHSTR)(wchar_t *Str,int MaxLength);
typedef wchar_t   *(WINAPI *FARSTDQUOTESPACEONLY)(wchar_t *Str);
typedef const wchar_t*   (WINAPI *FARSTDPOINTTONAME)(const wchar_t *Path);
typedef int     (WINAPI *FARSTDGETPATHROOT)(const wchar_t *Path,wchar_t *Root, int DestSize);
typedef BOOL    (WINAPI *FARSTDADDENDSLASH)(wchar_t *Path);
typedef int     (WINAPI *FARSTDCOPYTOCLIPBOARD)(const wchar_t *Data);
typedef wchar_t *(WINAPI *FARSTDPASTEFROMCLIPBOARD)(void);
typedef int     (WINAPI *FARSTDINPUTRECORDTOKEY)(const INPUT_RECORD *r);
typedef int     (WINAPI *FARSTDLOCALISLOWER)(wchar_t Ch);
typedef int     (WINAPI *FARSTDLOCALISUPPER)(wchar_t Ch);
typedef int     (WINAPI *FARSTDLOCALISALPHA)(wchar_t Ch);
typedef int     (WINAPI *FARSTDLOCALISALPHANUM)(wchar_t Ch);
typedef wchar_t (WINAPI *FARSTDLOCALUPPER)(wchar_t LowerChar);
typedef wchar_t (WINAPI *FARSTDLOCALLOWER)(wchar_t UpperChar);
typedef void    (WINAPI *FARSTDLOCALUPPERBUF)(wchar_t *Buf,int Length);
typedef void    (WINAPI *FARSTDLOCALLOWERBUF)(wchar_t *Buf,int Length);
typedef void    (WINAPI *FARSTDLOCALSTRUPR)(wchar_t *s1);
typedef void    (WINAPI *FARSTDLOCALSTRLWR)(wchar_t *s1);
typedef int     (WINAPI *FARSTDLOCALSTRICMP)(const wchar_t *s1,const wchar_t *s2);
typedef int     (WINAPI *FARSTDLOCALSTRNICMP)(const wchar_t *s1,const wchar_t *s2,int n);
*)

type
  {&Cdecl+}
  TFarStdQSortFunc = function (Param1 : Pointer; Param2 : Pointer) :Integer; cdecl;
  TFarStdQSortExFunc = function (Param1 : Pointer; Param2 : Pointer; UserParam : Pointer) : Integer; cdecl;

  {&StdCall+}
  TFarStdQSort = procedure (Base : Pointer; NElem : SIZE_T; Width : SIZE_T; FCmp : TFarStdQSortFunc); stdcall;
  TFarStdQSortEx = procedure (Base : Pointer; NElem : SIZE_T; Width : SIZE_T; FCmp : TFarStdQSortExFunc; UserParam : Pointer); stdcall;
  TFarStdBSearch = procedure (Key : Pointer; Base : Pointer; NElem : SIZE_T; Width : SIZE_T; FCmp : TFarStdQSortFunc); stdcall;

  TFarStdGetFileOwner = function (Computer : PFarChar; Name : PFarChar; Owner : PFarChar ) : Integer; stdcall;
  TFarStdGetNumberOfLinks = function (Name : PFarChar) : Integer; stdcall;
  TFarStdAtoi = function (S : PFarChar) : Integer; stdcall;
  TFarStdAtoi64 = function (S : PFarChar) : Int64; stdcall;
  TFarStdItoa64 = function (Value : Int64; Str : PFarChar; Radix : Integer) :PFarChar; stdcall;

  TFarStdItoa = function (Value : Integer; Str : PFarChar; Radix : Integer ) : PFarChar; stdcall;
  TFarStdLTrim = function (Str : PFarChar) : PFarChar; stdcall;
  TFarStdRTrim = function (Str : PFarChar) : PFarChar; stdcall;
  TFarStdTrim = function (Str : PFarChar) : PFarChar; stdcall;
  TFarStdTruncStr = function (Str : PFarChar; MaxLength : Integer) : PFarChar; stdcall;
  TFarStdTruncPathStr = function (Str : PFarChar; MaxLength : Integer) : PFarChar; stdcall;
  TFarStdQuoteSpaceOnly = function (Str : PFarChar) : PFarChar; stdcall;
  TFarStdPointToName = function (Path : PFarChar) : PFarChar; stdcall;
  TFarStdGetPathRoot = function (Path : PFarChar; Root : PFarChar; DestSize :Integer) :Integer; stdcall;
  TFarStdAddEndSlash = function (Path : PFarChar) : LongBool; stdcall;
  TFarStdCopyToClipBoard = function (Data : PFarChar) : Integer; stdcall;
  TFarStdPasteFromClipboard = function : PFarChar; stdcall;
  TFarStdInputRecordToKey = function (const R : TInputRecord) : Integer; stdcall;

  TFarStdLocalIsLower = function (Ch : Integer) : Integer; stdcall;
  TFarStdLocalIsUpper = function (Ch : Integer) : Integer; stdcall;
  TFarStdLocalIsAlpha = function (Ch : Integer) : Integer; stdcall;
  TFarStdLocalIsAlphaNum = function (Ch : Integer) : Integer; stdcall;
  TFarStdLocalUpper = function (LowerChar : Integer) : Integer; stdcall;
  TFarStdLocalLower = function (UpperChar : Integer) : Integer; stdcall;

  TFarStdLocalUpperBuf = function (Buf : PFarChar; Length : Integer) : Integer; stdcall;
  TFarStdLocalLowerBuf = function (Buf : PFarChar; Length : Integer) : Integer; stdcall;
  TFarStdLocalStrUpr = function (S1 : PFarChar) : Integer; stdcall;
  TFarStdLocalStrLwr = function (S1 : PFarChar) : Integer; stdcall;
  TFarStdLocalStrICmp = function (S1 : PFarChar; S2 : PFarChar) : Integer; stdcall;
  TFarStdLocalStrNICmp = function (S1 : PFarChar; S2 : PFarChar; N : Integer) : Integer; stdcall;


{ PROCESSNAME_FLAGS }

const
  PN_CMPNAME      = $00000000;
  PN_CMPNAMELIST  = $00001000;
  PN_GENERATENAME = $00002000;
  PN_SKIPPATH     = $00100000;

(*
typedef int (WINAPI *FARSTDPROCESSNAME)(const wchar_t *param1, wchar_t *param2, DWORD size, DWORD flags);
typedef void (WINAPI *FARSTDUNQUOTE)(wchar_t *Str);
*)
type
  TFarStdProcessName = function (Param1, Param2 :PFarChar; Size, Flags :DWORD) : Integer; stdcall;
  TFarStdUnquote = procedure (Str : PFarChar); stdcall;


{ XLATMODE }

const
  XLAT_SWITCHKEYBLAYOUT  = $00000001;
  XLAT_SWITCHKEYBBEEP    = $00000002;

(*
typedef wchar_t* (WINAPI *FARSTDXLAT)(wchar_t *Line,int StartPos,int EndPos,DWORD Flags);
typedef size_t  (WINAPI *FARSTDKEYTOKEYNAME)(int Key,wchar_t *KeyText,size_t Size);
typedef int     (WINAPI *FARSTDKEYNAMETOKEY)(const wchar_t *Name);
*)
type
  TFarStdXLat = function(Line :PFarChar; StartPos, EndPos :Integer; Flags :DWORD) :PFarChar; stdcall;
  TFarStdKeyToKeyName = function(Key :Integer; KeyText :PFarChar; Size :SIZE_T) :SIZE_T; stdcall;
  TFarStdKeyNameToKey = function(Name :PFarChar) :Integer; stdcall;

{ FRSMODE }

const
   FRS_RETUPDIR    = $01;
   FRS_RECUR       = $02;
   FRS_SCANSYMLINK = $04;

(*
typedef int (WINAPI *FRSUSERFUNC)(
  const struct FAR_FIND_DATA *FData,
  const wchar_t *FullName,
  void *Param
);
typedef void     (WINAPI *FARSTDRECURSIVESEARCH)(const wchar_t *InitDir,const wchar_t *Mask,FRSUSERFUNC Func,DWORD Flags,void *Param);
typedef wchar_t* (WINAPI *FARSTDMKTEMP)(wchar_t *Dest, DWORD size, const wchar_t *Prefix);
->
typedef int      (WINAPI *FARSTDMKTEMP)(wchar_t *Dest, DWORD size, const wchar_t *Prefix);
typedef void     (WINAPI *FARSTDDELETEBUFFER)(void *Buffer);
*)
type
  TFRSUserFunc = function (const FData :PFarFindData; const FullName :PFarChar; Param :Pointer) :Integer; stdcall;

  TFarStdRecursiveSearch = procedure (InitDir, Mask :PFarChar; Func :TFRSUserFunc; Flags :DWORD; Param :Pointer); stdcall;
  TFarStdMkTemp = function (Dest :PFarChar; Size :DWORD; Prefix :PFarChar) :Integer; stdcall;
  TFarStdDeleteBuffer = procedure(Buffer :Pointer); stdcall;


{ MKLINKOP }

const
  FLINK_HARDLINK         = 1;
  FLINK_JUNCTION         = 2;
  FLINK_VOLMOUNT         = 3;
  FLINK_SYMLINKFILE      = 4;
  FLINK_SYMLINKDIR       = 5;
  FLINK_SYMLINK          = 6;

  FLINK_SHOWERRMSG       = $10000;
  FLINK_DONOTUPDATEPANEL = $20000;

(*
typedef int     (WINAPI *FARSTDMKLINK)(const wchar_t *Src,const wchar_t *Dest,DWORD Flags);
typedef int     (WINAPI *FARGETREPARSEPOINTINFO)(const wchar_t *Src, wchar_t *Dest,int DestSize);
*)
type
  TFarStdMkLink = function (Src, Dest :PFarChar; Flags :DWORD) :Integer; stdcall;
  TFarGetReparsePointInfo = function (Src, Dest :PFarChar; DestSize :Integer) : Integer; stdcall;

(*
enum CONVERTPATHMODES
{
    CPM_FULL,
    CPM_REAL,
    CPM_NATIVE,
};

typedef int (WINAPI *FARCONVERTPATH)(CONVERTPATHMODES Mode, const wchar_t *Src, wchar_t *Dest, int DestSize);
*)

const
  CPM_FULL   = 0;
  CPM_REAL   = 1;
  CPM_NATIVE = 2;

type
  TFarConvertPath = function(Mode :Integer {TConvertPathModes}; Src :PFarChar; Dest :PFarChar; DestSize :Integer) :Integer; stdcall;

(*
typedef DWORD (WINAPI *FARGETCURRENTDIRECTORY)(DWORD Size,wchar_t* Buffer);
*)
type
  TFarGetCurrentDirectory = function(Size :DWORD; Buffer :PFarChar) :DWORD; stdcall;

(*
typedef struct FarStandardFunctions
{
  int StructSize;

  FARSTDATOI                 atoi;
  FARSTDATOI64               atoi64;
  FARSTDITOA                 itoa;
  FARSTDITOA64               itoa64;
  // <C&C++>
  FARSTDSPRINTF              sprintf;
  FARSTDSSCANF               sscanf;
  // </C&C++>
  FARSTDQSORT                qsort;
  FARSTDBSEARCH              bsearch;
  FARSTDQSORTEX              qsortex;
  // <C&C++>
  FARSTDSNPRINTF             snprintf;
  // </C&C++>

  DWORD_PTR                  Reserved[8];

  FARSTDLOCALISLOWER         LIsLower;
  FARSTDLOCALISUPPER         LIsUpper;
  FARSTDLOCALISALPHA         LIsAlpha;
  FARSTDLOCALISALPHANUM      LIsAlphanum;
  FARSTDLOCALUPPER           LUpper;
  FARSTDLOCALLOWER           LLower;
  FARSTDLOCALUPPERBUF        LUpperBuf;
  FARSTDLOCALLOWERBUF        LLowerBuf;
  FARSTDLOCALSTRUPR          LStrupr;
  FARSTDLOCALSTRLWR          LStrlwr;
  FARSTDLOCALSTRICMP         LStricmp;
  FARSTDLOCALSTRNICMP        LStrnicmp;

  FARSTDUNQUOTE              Unquote;
  FARSTDLTRIM                LTrim;
  FARSTDRTRIM                RTrim;
  FARSTDTRIM                 Trim;
  FARSTDTRUNCSTR             TruncStr;
  FARSTDTRUNCPATHSTR         TruncPathStr;
  FARSTDQUOTESPACEONLY       QuoteSpaceOnly;
  FARSTDPOINTTONAME          PointToName;
  FARSTDGETPATHROOT          GetPathRoot;
  FARSTDADDENDSLASH          AddEndSlash;
  FARSTDCOPYTOCLIPBOARD      CopyToClipboard;
  FARSTDPASTEFROMCLIPBOARD   PasteFromClipboard;
  FARSTDKEYTOKEYNAME         FarKeyToName;
  FARSTDKEYNAMETOKEY         FarNameToKey;
  FARSTDINPUTRECORDTOKEY     FarInputRecordToKey;
  FARSTDXLAT                 XLat;
  FARSTDGETFILEOWNER         GetFileOwner;
  FARSTDGETNUMBEROFLINKS     GetNumberOfLinks;
  FARSTDRECURSIVESEARCH      FarRecursiveSearch;
  FARSTDMKTEMP               MkTemp;
  FARSTDDELETEBUFFER         DeleteBuffer;
  FARSTDPROCESSNAME          ProcessName;
  FARSTDMKLINK               MkLink;
  FARCONVERTPATH             ConvertPath;
  FARGETREPARSEPOINTINFO     GetReparsePointInfo;
  FARGETCURRENTDIRECTORY     GetCurrentDirectory;
} FARSTANDARDFUNCTIONS;
*)
type
  PFarStandardFunctions = ^TFarStandardFunctions;
  TFarStandardFunctions = record
    StructSize          : Integer;

    atoi                : TFarStdAtoi;
    atoi64              : TFarStdAtoi64;
    itoa                : TFarStdItoa;
    itoa64              : TFarStdItoa64;

    sprintf             : Pointer;
    sscanf              : Pointer;

    qsort               : TFarStdQSort;
    bsearch             : TFarStdBSearch;
    qsortex             : TFarStdQSortEx;

    snprintf            : Pointer {TFarStdSNPRINTF};

    Reserved            : array [0..7] of DWORD_PTR;

    LIsLower            : TFarStdLocalIsLower;
    LIsUpper            : TFarStdLocalIsUpper;
    LIsAlpha            : TFarStdLocalIsAlpha;
    LIsAlphaNum         : TFarStdLocalIsAlphaNum;
    LUpper              : TFarStdLocalUpper;
    LLower              : TFarStdLocalLower;
    LUpperBuf           : TFarStdLocalUpperBuf;
    LLowerBuf           : TFarStdLocalLowerBuf;
    LStrupr             : TFarStdLocalStrUpr;
    LStrlwr             : TFarStdLocalStrLwr;
    LStricmp            : TFarStdLocalStrICmp;
    LStrnicmp           : TFarStdLocalStrNICmp;

    Unquote             : TFarStdUnquote;
    LTrim               : TFarStdLTrim;
    RTrim               : TFarStdRTrim;
    Trim                : TFarStdTrim;
    TruncStr            : TFarStdTruncStr;
    TruncPathStr        : TFarStdTruncPathStr;
    QuoteSpaceOnly      : TFarStdQuoteSpaceOnly;
    PointToName         : TFarStdPointToName;
    GetPathRoot         : TFarStdGetPathRoot;
    AddEndSlash         : TFarStdAddEndSlash;
    CopyToClipboard     : TFarStdCopyToClipboard;
    PasteFromClipboard  : TFarStdPasteFromClipboard;
    FarKeyToName        : TFarStdKeyToKeyName;
    FarNameToKey        : TFarStdKeyNameToKey;
    FarInputRecordToKey : TFarStdInputRecordToKey;
    XLat                : TFarStdXLat;
    GetFileOwner        : TFarStdGetFileOwner;
    GetNumberOfLinks    : TFarStdGetNumberOfLinks;
    FarRecursiveSearch  : TFarStdRecursiveSearch;
    MkTemp              : TFarStdMkTemp;
    DeleteBuffer        : TFarStdDeleteBuffer;
    ProcessName         : TFarStdProcessName;
    MkLink              : TFarStdMkLink;
    ConvertPath         : TFarConvertPath;
    GetReparsePointInfo : TFarGetReparsePointInfo;
    GetCurrentDirectory : TFarGetCurrentDirectory;
  end; {TFarStandardFunctions}


(*
struct PluginStartupInfo
{
  int StructSize;
  const wchar_t *ModuleName;
  INT_PTR ModuleNumber;
  const wchar_t *RootKey;
  FARAPIMENU             Menu;
  FARAPIMESSAGE          Message;
  FARAPIGETMSG           GetMsg;
  FARAPICONTROL          Control;
  FARAPISAVESCREEN       SaveScreen;
  FARAPIRESTORESCREEN    RestoreScreen;
  FARAPIGETDIRLIST       GetDirList;
  FARAPIGETPLUGINDIRLIST GetPluginDirList;
  FARAPIFREEDIRLIST      FreeDirList;
  FARAPIFREEPLUGINDIRLIST FreePluginDirList;
  FARAPIVIEWER           Viewer;
  FARAPIEDITOR           Editor;
  FARAPICMPNAME          CmpName;
  FARAPITEXT             Text;
  FARAPIEDITORCONTROL    EditorControl;

  FARSTANDARDFUNCTIONS  *FSF;

  FARAPISHOWHELP         ShowHelp;
  FARAPIADVCONTROL       AdvControl;
  FARAPIINPUTBOX         InputBox;
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
};
*)
type
  PPluginStartupInfo = ^TPluginStartupInfo;
  TPluginStartupInfo = record
    StructSize          : Integer;
    ModuleName          : PFarChar;
    ModuleNumber        : INT_PTR;
    RootKey             : PFarChar;

    Menu                : TFarApiMenu;
    Message             : TFarApiMessage;
    GetMsg              : TFarApiGetMsg;
    Control             : TFarApiControl;
    SaveScreen          : TFarApiSaveScreen;
    RestoreScreen       : TFarApiRestoreScreen;
    GetDirList          : TFarApiGetDirList;
    GetPluginDirList    : TFarApiGetPluginDirList;
    FreeDirList         : TFarApiFreeDirList;
    FreePluginDirList   : TFarApiFreePluginDirList;
    Viewer              : TFarApiViewer;
    Editor              : TFarApiEditor;
    CmpName             : TFarApiCmpName;
    Text                : TFarApiText;
    EditorControl       : TFarApiEditorControl;

    FSF                 : PFarStandardFunctions;

    ShowHelp            : TFarApiShowHelp;
    AdvControl          : TFarApiAdvControl;
    InputBox            : TFarApiInputBox;
    DialogInit          : TFarApiDialogInit;
    DialogRun           : TFarApiDialogRun;
    DialogFree          : TFarApiDialogFree;

    SendDlgMessage      : TFarApiSendDlgMessage;
    DefDlgProc          : TFarApiDefDlgProc;
    Reserved            : DWORD_PTR;
    ViewerControl       : TFarApiViewerControl;
    PluginsControl      : TFarApiPluginsControl;
    FileFilterControl   : TFarApiFilterControl;

    RegExpControl       : TFarApiRegexpControl;
  end; {TPluginStartupInfo}


{ PLUGIN_FLAGS }

const
  PF_PRELOAD       = $0001;
  PF_DISABLEPANELS = $0002;
  PF_EDITOR        = $0004;
  PF_VIEWER        = $0008;
  PF_FULLCMDLINE   = $0010;
  PF_DIALOG        = $0020;

(*
struct PluginInfo
{
  int StructSize;
  DWORD Flags;
  const wchar_t * const *DiskMenuStrings;
  int *DiskMenuNumbers;
  int DiskMenuStringsNumber;
  const wchar_t * const *PluginMenuStrings;
  int PluginMenuStringsNumber;
  const wchar_t * const *PluginConfigStrings;
  int PluginConfigStringsNumber;
  const wchar_t *CommandPrefix;
  DWORD Reserved;
};
*)
type
  PPluginInfo = ^TPluginInfo;
  TPluginInfo = record
    StructSize : Integer;
    Flags : DWORD;
    DiskMenuStrings : PPCharArray;
    DiskMenuNumbers : PIntegerArray;
    DiskMenuStringsNumber : Integer;
    PluginMenuStrings : PPCharArray;
    PluginMenuStringsNumber : Integer;
    PluginConfigStrings : PPCharArray;
    PluginConfigStringsNumber : Integer;
    CommandPrefix : PFarChar;
    Reserved : DWORD;
  end;

(*
struct InfoPanelLine
{
  const wchar_t *Text;
  const wchar_t *Data;
  int  Separator;
};
*)
type
  PInfoPanelLine = ^TInfoPanelLine;
  TInfoPanelLine = record
    Text :PFarChar;
    Data :PFarChar;
    Separator :Integer;
  end;

  PInfoPanelLineArray = ^TInfoPanelLineArray;
  TInfoPanelLineArray = packed array [0..MaxInt div SizeOf(TInfoPanelLine) - 1] of TInfoPanelLine;


(*
struct PanelMode
{
  wchar_t  *ColumnTypes;
  wchar_t  *ColumnWidths;
  wchar_t **ColumnTitles;
  int    FullScreen;
  int    DetailedStatus;
  int    AlignExtensions;
  int    CaseConversion;
  wchar_t  *StatusColumnTypes;
  wchar_t  *StatusColumnWidths;
  DWORD  Reserved[2];
};
*)
type
  PPanelMode = ^TPanelMode;
  TPanelMode = record
    ColumnTypes : PFarChar;
    ColumnWidths : PFarChar;
    ColumnTitles : PPCharArray;
    FullScreen : Integer;
    DetailedStatus : Integer;
    AlignExtensions : Integer;
    CaseConversion : Integer;
    StatusColumnTypes : PFarChar;
    StatusColumnWidths : PFarChar;
    Reserved : array [0..1] of DWORD;
  end;

  PPanelModeArray = ^TPanelModeArray;
  TPanelModeArray = packed array [0..MaxInt div SizeOf(TPanelMode) - 1] of TPanelMode;


{ OPENPLUGININFO_FLAGS }

const
  OPIF_USEFILTER           = $00000001;
  OPIF_USESORTGROUPS       = $00000002;
  OPIF_USEHIGHLIGHTING     = $00000004;
  OPIF_ADDDOTS             = $00000008;
  OPIF_RAWSELECTION        = $00000010;
  OPIF_REALNAMES           = $00000020;
  OPIF_SHOWNAMESONLY       = $00000040;
  OPIF_SHOWRIGHTALIGNNAMES = $00000080;
  OPIF_SHOWPRESERVECASE    = $00000100;
//OPIF_FINDFOLDERS         = $00000200;
  OPIF_COMPAREFATTIME      = $00000400;
  OPIF_EXTERNALGET         = $00000800;
  OPIF_EXTERNALPUT         = $00001000;
  OPIF_EXTERNALDELETE      = $00002000;
  OPIF_EXTERNALMKDIR       = $00004000;
  OPIF_USEATTRHIGHLIGHTING = $00008000;


{ OPENPLUGININFO_SORTMODES }

const
  SM_DEFAULT        = 0;
  SM_UNSORTED       = 1;
  SM_NAME           = 2;
  SM_EXT            = 3;
  SM_MTIME          = 4;
  SM_CTIME          = 5;
  SM_ATIME          = 6;
  SM_SIZE           = 7;
  SM_DESCR          = 8;
  SM_OWNER          = 9;
  SM_COMPRESSEDSIZE = 10;
  SM_NUMLINKS       = 11;
  SM_NUMSTREAMS     = 12;
  SM_STREAMSSIZE    = 13;
  SM_FULLNAME       = 14;


(*
struct KeyBarTitles
{
  wchar_t *Titles[12];
  wchar_t *CtrlTitles[12];
  wchar_t *AltTitles[12];
  wchar_t *ShiftTitles[12];

  wchar_t *CtrlShiftTitles[12];
  wchar_t *AltShiftTitles[12];
  wchar_t *CtrlAltTitles[12];
};
*)
type
  PKeyBarTitles = ^TKeyBarTitles;
  TKeyBarTitles = record
    Titles : array [0..11] of PFarChar;
    CtrlTitles : array [0..11] of PFarChar;
    AltTitles : array [0..11] of PFarChar;
    ShiftTitles : array [0..11] of PFarChar;
    CtrlShiftTitles : array [0..11] of PFarChar;
    AltShiftTitles : array [0..11] of PFarChar;
    CtrlAltTitles : array [0..11] of PFarChar;
  end;

  PKeyBarTitlesArray = ^TKeyBarTitlesArray;
  TKeyBarTitlesArray = packed array [0..MaxInt div SizeOf(TKeyBarTitles) - 1] of TKeyBarTitles;


{ OPERATION_MODES }

const
  OPM_SILENT    = $0001;
  OPM_FIND      = $0002;
  OPM_VIEW      = $0004;
  OPM_EDIT      = $0008;
  OPM_TOPLEVEL  = $0010;
  OPM_DESCR     = $0020;
  OPM_QUICKVIEW = $0040;


(*
struct OpenPluginInfo
{
  int                   StructSize;
  DWORD                 Flags;
  const wchar_t           *HostFile;
  const wchar_t           *CurDir;
  const wchar_t           *Format;
  const wchar_t           *PanelTitle;
  const struct InfoPanelLine *InfoLines;
  int                   InfoLinesNumber;
  const wchar_t * const   *DescrFiles;
  int                   DescrFilesNumber;
  const struct PanelMode *PanelModesArray;
  int                   PanelModesNumber;
  int                   StartPanelMode;
  int                   StartSortMode;
  int                   StartSortOrder;
  const struct KeyBarTitles *KeyBar;
  const wchar_t           *ShortcutData;
  long                  Reserved;
};
*)
type
  POpenPluginInfo = ^TOpenPluginInfo;
  TOpenPluginInfo = record
    StructSize : Integer;
    Flags : DWORD;
    HostFile : PFarChar;
    CurDir : PFarChar;
    Format : PFarChar;
    PanelTitle : PFarChar;
    InfoLines : PInfoPanelLineArray;
    InfoLinesNumber : Integer;
    DescrFiles : PPCharArray;
    DescrFilesNumber : Integer;
    PanelModesArray : PPanelModeArray;
    PanelModesNumber : Integer;
    StartPanelMode : Integer;
    StartSortMode : Integer;
    StartSortOrder : Integer;
    KeyBar : PKeyBarTitlesArray;
    ShortcutData : PFarChar;
    Reserved : Integer;
  end;


{ OPENPLUGIN_OPENFROM }

const
  OPEN_DISKMENU    = 0;
  OPEN_PLUGINSMENU = 1;
  OPEN_FINDLIST    = 2;
  OPEN_SHORTCUT    = 3;
  OPEN_COMMANDLINE = 4;
  OPEN_EDITOR      = 5;
  OPEN_VIEWER      = 6;
  OPEN_FILEPANEL   = 7;
  OPEN_DIALOG      = 8;
  OPEN_ANALYSE     = 9;
  OPEN_FROMMACRO   = $10000;

{ FAR_PKF_FLAGS }

const
  PKF_CONTROL    = $00000001;
  PKF_ALT        = $00000002;
  PKF_SHIFT      = $00000004;
  PKF_PREPROCESS = $00080000; // for "Key", function ProcessKey()

{ FAR_EVENTS }

const
  FE_CHANGEVIEWMODE = 0;
  FE_REDRAW         = 1;
  FE_IDLE           = 2;
  FE_CLOSE          = 3;
  FE_BREAK          = 4;
  FE_COMMAND        = 5;
  FE_GOTFOCUS       = 6;
  FE_KILLFOCUS      = 7;

{ FAR_PLUGINS_CONTROL_COMMANDS }

const
  PCTL_LOADPLUGIN   = 0;
  PCTL_UNLOADPLUGIN = 1;

{ FAR_PLUGIN_LOAD_TYPE }

const
  PLT_PATH = 0;


{FAR_FILE_FILTER_CONTROL_COMMANDS}

const
  FFCTL_CREATEFILEFILTER = 0;
  FFCTL_FREEFILEFILTER   = 1;
  FFCTL_OPENFILTERSMENU  = 2;
  FFCTL_STARTINGTOFILTER = 3;
  FFCTL_ISFILEINFILTER   = 4;

{FAR_FILE_FILTER_TYPE}

const
  FFT_PANEL     = 0;
  FFT_FINDFILE  = 1;
  FFT_COPY      = 2;
  FFT_SELECT    = 3;


{FAR_REGEXP_CONTROL_COMMANDS}

const
  RECTL_CREATE        = 0;
  RECTL_FREE          = 1;
  RECTL_COMPILE       = 2;
  RECTL_OPTIMIZE      = 3;
  RECTL_MATCHEX       = 4;
  RECTL_SEARCHEX      = 5;
  RECTL_BRACKETSCOUNT = 6;


(*
struct RegExpMatch
{
  int start,end;
};
*)
type
  PRegExpMatch = ^TRegExpMatch;
  TRegExpMatch = record
    Start :Integer;
    EndPos :Integer;
  end;

(*
struct RegExpSearch
{
  const wchar_t* Text;
  int Position;
  int Length;
  struct RegExpMatch* Match;
  int Count;
  void* Reserved;
};
*)
type
  PRegExpSearch = ^TRegExpSearch;
  TRegExpSearch = record
    Text :PFarChar;
    Position :Integer;
    Length :Integer;
    Match :PRegExpMatch;
    Count :Integer;
    Reserved :Pointer;
  end;

(*
// Exported Functions

void   WINAPI _export ClosePluginW(HANDLE hPlugin);
int    WINAPI _export CompareW(HANDLE hPlugin,const struct PluginPanelItem *Item1,const struct PluginPanelItem *Item2,unsigned int Mode);
int    WINAPI _export ConfigureW(int ItemNumber);
int    WINAPI _export DeleteFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
void   WINAPI _export ExitFARW(void);
void   WINAPI _export FreeFindDataW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
void   WINAPI _export FreeVirtualFindDataW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
int    WINAPI _export GetFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,const wchar_t **DestPath,int OpMode);
int    WINAPI _export GetFindDataW(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
int    WINAPI _export GetMinFarVersionW(void);
void   WINAPI _export GetOpenPluginInfoW(HANDLE hPlugin,struct OpenPluginInfo *Info);
void   WINAPI _export GetPluginInfoW(struct PluginInfo *Info);
int    WINAPI _export GetVirtualFindDataW(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,const wchar_t *Path);
int    WINAPI _export MakeDirectoryW(HANDLE hPlugin,const wchar_t **Name,int OpMode);
HANDLE WINAPI _export OpenFilePluginW(const wchar_t *Name,const unsigned char *Data,int DataSize,int OpMode);
HANDLE WINAPI _export OpenPluginW(int OpenFrom,INT_PTR Item);
int    WINAPI _export ProcessDialogEventW(int Event,void *Param);
int    WINAPI _export ProcessEditorEventW(int Event,void *Param);
int    WINAPI _export ProcessEditorInputW(const INPUT_RECORD *Rec);
int    WINAPI _export ProcessEventW(HANDLE hPlugin,int Event,void *Param);
int    WINAPI _export ProcessHostFileW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
int    WINAPI _export ProcessKeyW(HANDLE hPlugin,int Key,unsigned int ControlState);
int    WINAPI _export ProcessSynchroEventW(int Event,void *Param);
int    WINAPI _export ProcessViewerEventW(int Event,void *Param);
int    WINAPI _export PutFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
->
int    WINAPI _export PutFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,const wchar_t *SrcPath,int OpMode);
int    WINAPI _export SetDirectoryW(HANDLE hPlugin,const wchar_t *Dir,int OpMode);
int    WINAPI _export SetFindListW(HANDLE hPlugin,const struct PluginPanelItem *PanelItem,int ItemsNumber);
void   WINAPI _export SetStartupInfoW(const struct PluginStartupInfo *Info);
*)


(*
#define MAKEFARVERSION(major,minor,build) ( ((major)<<8) | (minor) | ((build)<<16))

#define FARMANAGERVERSION MAKEFARVERSION(FARMANAGERVERSION_MAJOR,FARMANAGERVERSION_MINOR,FARMANAGERVERSION_BUILD)
*)

function MakeFarVersion (Major : DWORD; Minor : DWORD; Build : DWORD) : DWORD;


(*
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
#define DlgList_SetCurPos(Info,hDlg,ID,NewPos) {struct FarListPos LPos={NewPos,-1};Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,ID,(INT_PTR)&LPos);}
#define DlgList_ClearList(Info,hDlg,ID)        Info.SendDlgMessage(hDlg,DM_LISTDELETE,ID,0)
#define DlgList_DeleteItem(Info,hDlg,ID,Index) {struct FarListDelete FLDItem={Index,1}; Info.SendDlgMessage(hDlg,DM_LISTDELETE,ID,(INT_PTR)&FLDItem);}
#define DlgList_SortUp(Info,hDlg,ID)           Info.SendDlgMessage(hDlg,DM_LISTSORT,ID,0)
#define DlgList_SortDown(Info,hDlg,ID)         Info.SendDlgMessage(hDlg,DM_LISTSORT,ID,1)
#define DlgList_GetItemData(Info,hDlg,ID,Index)          Info.SendDlgMessage(hDlg,DM_LISTGETDATA,ID,Index)
#define DlgList_SetItemStrAsData(Info,hDlg,ID,Index,Str) {struct FarListItemData FLID{Index,0,Str,0}; Info.SendDlgMessage(hDlg,DM_LISTSETDATA,ID,(INT_PTR)&FLID);}
*)

function Dlg_RedrawDialog(const Info :TPluginStartupInfo; hDlg :THandle) :Integer;
function Dlg_GetDlgData(const Info :TPluginStartupInfo; hDlg :THandle) :Integer;
function Dlg_SetDlgData(const Info :TPluginStartupInfo; hDlg :THandle; Data :Pointer) :Integer;
function Dlg_GetDlgItemData(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
function Dlg_SetDlgItemData(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer; Data :Pointer) :Integer;
function DlgItem_GetFocus(const Info :TPluginStartupInfo; hDlg :THandle) :Integer;
function DlgItem_SetFocus(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
function DlgItem_Enable(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
function DlgItem_Disable(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
function DlgItem_IsEnable(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
function DlgItem_SetText(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer; Str :PFarChar) :Integer;
function DlgItem_GetCheck(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
function DlgItem_SetCheck(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer; State :Integer) :Integer;
function DlgEdit_AddHistory(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer; Str :PFarChar) :Integer;
function DlgList_AddString(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer; Str :PFarChar) :Integer;
function DlgList_GetCurPos(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
function DlgList_SetCurPos(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer; NewPos :Integer) :Integer;
function DlgList_ClearList(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
function DlgList_DeleteItem(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer; Index :Integer) :Integer;
function DlgList_SortUp(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
function DlgList_SortDown(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
function DlgList_GetItemData(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer; Index :Integer) :Integer;
function DlgList_SetItemStrAsData(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer; Index :Integer; Str :PFarChar) :Integer;

const
  FindFileId        :TGUID = '{8C9EAD29-910F-4b24-A669-EDAFBA6ED964}';
  CopyOverwriteId   :TGUID = '{9FBCB7E1-ACA2-475d-B40D-0F7365B632FF}';
  FileOpenCreateId  :TGUID = '{1D07CEE2-8F4F-480a-BE93-069B4FF59A2B}';
  FileSaveAsId      :TGUID = '{9162F965-78B8-4476-98AC-D699E5B6AFE7}';
  MakeFolderId      :TGUID = '{FAD00DBE-3FFF-4095-9232-E1CC70C67737}';
  FileAttrDlgId     :TGUID = '{80695D20-1085-44d6-8061-F3C41AB5569C}';
  CopyReadOnlyId    :TGUID = '{879A8DE6-3108-4beb-80DE-6F264991CE98}';
  CopyFilesId       :TGUID = '{FCEF11C4-5490-451d-8B4A-62FA03F52759}';
  HardSymLinkId     :TGUID = '{5EB266F4-980D-46af-B3D2-2C50E64BCA81}';


{******************************************************************************}
{******************************} implementation {******************************}
{******************************************************************************}


function MakeFarVersion (Major :DWORD; Minor :DWORD; Build :DWORD) :DWORD;
begin
  Result := (Major shl 8) or (Minor) or (Build shl 16);
end;


function Dlg_RedrawDialog(const Info :TPluginStartupInfo; hDlg :THandle) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg, DM_REDRAW, 0, 0);
end;

function Dlg_GetDlgData(const Info :TPluginStartupInfo; hDlg :THandle) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
end;

function Dlg_SetDlgData(const Info :TPluginStartupInfo; hDlg :THandle; Data :Pointer) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg, DM_SETDLGDATA, 0, INT_PTR(Data));
end;

function Dlg_GetDlgItemData(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg, DM_GETITEMDATA, 0, 0);
end;

function Dlg_SetDlgItemData(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer; Data :Pointer) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg, DM_SETITEMDATA, 0, INT_PTR(Data));
end;

function DlgItem_GetFocus(const Info :TPluginStartupInfo; hDlg :THandle) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg, DM_GETFOCUS,0,0)
end;

function DlgItem_SetFocus(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg,DM_SETFOCUS,ID,0)
end;

function DlgItem_Enable(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg, DM_ENABLE, ID, 1);
end;

function DlgItem_Disable(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg, DM_ENABLE, ID, 0);
end;

function DlgItem_IsEnable(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg, DM_ENABLE, ID, -1);
end;

function DlgItem_SetText(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer; Str :PFarChar) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, ID, INT_PTR(Str));
end;

function DlgItem_GetCheck(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg, DM_GETCHECK, ID, 0);
end;

function DlgItem_SetCheck(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer; State :Integer) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg, DM_SETCHECK, ID, State);
end;

function DlgEdit_AddHistory(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer; Str :PFarChar) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg, DM_ADDHISTORY, ID, INT_PTR(Str));
end;

function DlgList_AddString(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer; Str :PFarChar) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg, DM_LISTADDSTR, ID, INT_PTR(Str));
end;

function DlgList_GetCurPos(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, ID, 0);
end;

function DlgList_SetCurPos(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer; NewPos :Integer) :Integer;
var
  LPos :TFarListPos;
begin
  LPos.SelectPos := NewPos;
  LPos.TopPos := -1;
  Result := Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, ID, INT_PTR(@LPos));
end;

function DlgList_ClearList(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg,DM_LISTDELETE,ID,0)
end;

function DlgList_DeleteItem(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer; Index :Integer) :Integer;
var
  FLDItem :TFarListDelete;
begin
  FLDItem.StartIndex := Index;
  FLDItem.Count := 1;
  Result := Info.SendDlgMessage(hDlg, DM_LISTDELETE, ID, INT_PTR(@FLDItem));
end;

function DlgList_SortUp(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg, DM_LISTSORT, ID, 0);
end;

function DlgList_SortDown(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg, DM_LISTSORT, ID, 1);
end;

function DlgList_GetItemData(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer; Index :Integer) :Integer;
begin
  Result := Info.SendDlgMessage(hDlg, DM_LISTGETDATA, ID, Index);
end;

function DlgList_SetItemStrAsData(const Info :TPluginStartupInfo; hDlg :THandle; ID :Integer; Index :Integer; Str :PFarChar) :Integer;
var
  FLID :TFarListItemData;
begin
  FLID.Index := Index;
  FLID.DataSize := 0;
  FLID.Data := Str;
  FLID.Reserved := 0;
  Result := Info.SendDlgMessage (hDlg, DM_LISTSETDATA, ID, INT_PTR(@FLID));
end;


end.
