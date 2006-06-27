(*
   plugin.pas

   Plugin API for FAR Manager 1.70

   Copyright (c) 1996-2000 Eugene Roshal
   Copyright (c) 2000-2006 FAR group
*)

{$IFDEF VIRTUALPASCAL}

  {$DEFINE VP}
  {&Delphi+}
  {&AlignData+}
  {&AlignRec+}
  {&StdCall+}
  {&H+}
  {&Z+}
  {&Use32+}

{$ELSE}

  {$ALIGN OFF}
  {$MINENUMSIZE 4}
  {$R-}

  {$WRITEABLECONST ON}

  {$IFNDEF VER80}           { Delphi 1.0     }
   {$IFNDEF VER90}          { Delphi 2.0     }
    {$IFNDEF VER93}         { C++Builder 1.0 }
      {$IFNDEF VER100}
        {$IFNDEF VER110}
          {$DEFINE USE_DELPHI4}   { Delphi 4.0 or higher }
        {$ENDIF}
      {$ENDIF}
    {$ENDIF}
   {$ENDIF}
  {$ENDIF}

{$ENDIF}


unit Plugin;

interface

uses Windows;

const
   NM = 260;
   FARMACRO_KEY_EVENT = KEY_EVENT or $8000;
   MAXSIZE_SHORTCUTDATA = 8192;
   FARMANAGERVERSION : DWORD = 0;

type
   PPCharArray = ^TPCharArray;
   TPCharArray = packed array[0..Pred(MaxLongint div SizeOf(PChar))] of PChar;

type
   PIntegerArray = ^TIntegerArray;
   TIntegerArray = packed array[0..Pred(MaxLongint div SizeOf(Integer))] of Integer;


{ FARMESSAGEFLAGS }

const
   FMSG_WARNING             = $00000001;
   FMSG_ERRORTYPE           = $00000002;
   FMSG_KEEPBACKGROUND      = $00000004;
   FMSG_DOWN                = $00000008;
   FMSG_LEFTALIGN           = $00000010;

   FMSG_ALLINONE            = $00000020;

   FMSG_MB_OK               = $00010000;
   FMSG_MB_OKCANCEL         = $00020000;
   FMSG_MB_ABORTRETRYIGNORE = $00030000;
   FMSG_MB_YESNO            = $00040000;
   FMSG_MB_YESNOCANCEL      = $00050000;
   FMSG_MB_RETRYCANCEL      = $00060000;


type
   TFarApiMessage = function (
         PluginNumber : Integer;
         Flags : DWORD;
         const HelpTopic : PChar;
         const Items : PPCharArray;
         ItemsNumber : Integer;
         ButtonsNumber : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

{ DialogItemTypes }

const
   DI_TEXT        = 0;
   DI_VTEXT       = 1;
   DI_SINGLEBOX   = 2;
   DI_DOUBLEBOX   = 3;
   DI_EDIT        = 4;
   DI_PSWEDIT     = 5;
   DI_FIXEDIT     = 6;
   DI_BUTTON      = 7;
   DI_CHECKBOX    = 8;
   DI_RADIOBUTTON = 9;
   DI_COMBOBOX    = 10;
   DI_LISTBOX     = 11;

   DI_USERCONTROL = 255;


{ FarDialogItemFlags }

const
   DIF_COLORMASK         = $000000ff;
   DIF_SETCOLOR          = $00000100;
   DIF_BOXCOLOR          = $00000200;
   DIF_GROUP             = $00000400;
   DIF_LEFTTEXT          = $00000800;
   DIF_MOVESELECT        = $00001000;
   DIF_SHOWAMPERSAND     = $00002000;
   DIF_CENTERGROUP       = $00004000;
   DIF_NOBRACKETS        = $00008000;
   DIF_MANUALADDHISTORY  = $00008000;
   DIF_SEPARATOR         = $00010000;
   DIF_VAREDIT           = $00010000;
   DIF_SEPARATOR2        = $00020000;
   DIF_EDITOR            = $00020000;
   DIF_LISTNOAMPERSAND   = $00020000;
   DIF_LISTNOBOX         = $00040000;
   DIF_HISTORY           = $00040000;
   DIF_BTNNOCLOSE        = $00040000;
   DIF_CENTERTEXT        = $00040000;
   DIF_EDITEXPAND        = $00080000;
   DIF_DROPDOWNLIST      = $00100000;
   DIF_USELASTHISTORY    = $00200000;
   DIF_MASKEDIT          = $00400000;
   DIF_SELECTONENTRY     = $00800000;
   DIF_3STATE            = $00800000;
   DIF_LISTWRAPMODE      = $01000000;
   DIF_LISTAUTOHIGHLIGHT = $02000000;
   DIF_LISTNOCLOSE       = $04000000;
   DIF_HIDDEN            = $10000000;
   DIF_READONLY          = $20000000;
   DIF_NOFOCUS           = $40000000;
   DIF_DISABLE           = $80000000;

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
   DN_LISTHOTKEY           = DM_FIRST+60;

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

{ LISTITEMFLAGS }

const
  LIF_SELECTED       = $00010000;
  LIF_CHECKED        = $00020000;
  LIF_SEPARATOR      = $00040000;
  LIF_DISABLE        = $00080000;
  LIF_DELETEUSERDATA = $80000000;

type
   PFarListItem = ^TFarListItem;
   TFarListItem = packed record
      Flags : DWORD;
      Text : array [0..127] of Char;
      Reserved : array [0..2] of DWORD;
   end;

type
   PFarListItemArray = ^TFarListItemArray;
   TFarListItemArray = packed array[0..Pred(MaxLongint div SizeOf(TFarListItem))] of TFarListItem;


type
   PFarListUpdate = ^TFarListUpdate;
   TFarListUpdate = packed record
      Index : Integer;
      Item : TFarListItem;
   end;

type
   PFarListInsert = ^TFarListInsert;
   TFarListInsert = packed record
      Index : Integer;
      Item : TFarListItem;
   end;

type
   PFarListGetItem = ^TFarListGetItem;
   TFarListGetItem = packed record
      ItemIndex : Integer;
      Item : TFarListItem;
   end;

type
   PFarListPos = ^TFarListPos;
   TFarListPos = packed record
      SelectPos : Integer;
      TopPos : Integer;
   end;

{ FARLISTFINDFLAGS }

const
   LIFIND_EXACTMATCH = $00000001;

type
   PFarListFind = ^TFarListFind;
   TFarListFind = packed record
      StartIndex : Integer;
      Pattern : PChar;
      Flags : DWORD;
      Reserved : DWORD;
   end;

type
   PFarListDelete = ^TFarListDelete;
   TFarListDelete = packed record
      StartIndex : Integer;
      Count : Integer;
   end;

{ FARLISTINFOFLAGS }

const
   LINFO_SHOWNOBOX        = $00000400;
   LINFO_AUTOHIGHLIGHT    = $00000800;
   LINFO_REVERSEHIGHLIGHT = $00001000;
   LINFO_WRAPMODE         = $00008000;
   LINFO_SHOWAMPERSAND    = $00010000;

type
   PFarListInfo = ^TFarListInfo;
   TFarListInfo = packed record
      Flags : DWORD;
      ItemsNumber : Integer;
      SelectPos : Integer;
      TopPos : Integer;
      MaxHeight : Integer;
      MaxLength : Integer;
      Reserved : array [0..5] of DWORD;
   end;


type
   PFarListItemData = ^TFarListItemData;
   TFarListItemData = packed record
      Index : Integer;
      DataSize : Integer;
      Data : Pointer;
      Reserved : DWORD;
   end;

type
   PFarList = ^TFarList;
   TFarList = packed record
      ItemsNumber : Integer;
      Items : PFarListItemArray;
   end;

type
   PFarListTitles = ^TFarListTitles;
   TFarListTitles = packed record
      TitleLen : Integer;
      Title : PChar;
      BottomLen : Integer;
      Bottom : PChar;
   end;

type
   PFarListColors = ^TFarListColors;
   TFarListColors = packed record
      Flags : DWORD;
      Reserved : DWORD;
      ColorCount : Integer;
      Colors : PChar;
   end;

type
   PFarDataPtr = ^TFarDataPtr;
   TFarDataPtr = packed record
      PtrFlags : DWORD;
      PtrLength : Integer;
      PtrData : PChar;
      PtrTail : array [0..0] of Char;
   end;

type
   PFarDialogItem = ^TFarDialogItem;
   TFarDialogItem = packed record
      ItemType : Integer;
      X1 : Integer;
      Y1 : Integer;
      X2 : Integer;
      Y2 : Integer;
      Focus : Integer;

      Param : record case Integer of
         0 : (Selected : Integer);
         1 : (History : PChar);
         2 : (Mask : PChar);
         3 : (ListItems : PFarList);
         4 : (ListPos : Integer);
         5 : (VBuf : PCharInfo);
      end;

      Flags : DWORD;
      DefaultButton : Integer;

      Data : record case Integer of
         0 : (Data : array [0..511] of Char);
         1 : (Ptr : PFarDataPtr);
      end;
   end;

type
   PFarDialogItemArray = ^TFarDialogItemArray;
   TFarDialogItemArray = packed array[0..Pred(MaxLongint div SizeOf(TFarDialogItem))] of TFarDialogItem;

type
   PFarDialogItemData = ^TFarDialogItemData;
   TFarDialogItemData = packed record
      PtrLength : Integer;
      PtrData : PChar;
   end;


{ FARDIALOGFLAGS }

const
   FDLG_WARNING      = $00000001;
   FDLG_SMALLDIALOG  = $00000002;
   FDLG_NODRAWSHADOW = $00000004;
   FDLG_NODRAWPANEL  = $00000008;

type
   TFarApiWindowProc = function (
         hDlg : THandle;
         Msg : Integer;
         Param1 : Integer;
         Param2 : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarApiSendDlgMessage = function (
         hDlg : THandle;
         Msg : Integer;
         Param1 : Integer;
         Param2 : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarApiDefDlgProc = function (
         hDlg : THandle;
         Msg : Integer;
         Param1 : Integer;
         Param2 : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarApiDialog = function (
         PluginNumber : Integer;
         X1, Y1, X2, Y2 : Integer;
         const HelpTopic : PChar;
         Item : PFarDialogItemArray;
         ItemsNumber : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarApiDialogEx = function (
         PluginNumber : Integer;
         X1, Y1, X2, Y2 : Integer;
         const HelpTopic : PChar;
         Item : PFarDialogItemArray;
         ItemsNumber : Integer;
         Reserved : DWORD;
         Flags : DWORD;
{$IFNDEF VP}
         DlgProc : TFarApiWindowProc;
{$ELSE}
         DlgProc : Pointer;
{$ENDIF}
         Param : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

type
   PFarMenuItem = ^TFarMenuItem;
   TFarMenuItem = packed record
      Text : array [0..127] of Char;
      Selected : Integer;
      Checked : Integer;
      Separator : Integer;
   end;

type
   PFarMenuItemArray = ^TFarMenuItemArray;
   TFarMenuItemArray = packed array[0..Pred(MaxLongint div SizeOf(TFarMenuItem))] of TFarMenuItem;


{ MENUITEMFLAGS }

const
   MIF_SELECTED   = $00010000;
   MIF_CHECKED    = $00020000;
   MIF_SEPARATOR  = $00040000;
   MIF_DISABLE    = $00080000;
   MIF_USETEXTPTR = $80000000;

type
   PFarMenuItemEx = ^TFarMenuItemEx;
   TFarMenuItemEx = packed record
      Flags : DWORD;

      Text : record case Integer of
         0 : (Text : array [0..127] of Char);
         1 : (TextPtr : PChar);
      end;

      AccelKey : DWORD;
      Reserved : DWORD;
      UserData : DWORD;
   end;

{ FARMENUFLAGS }

const
   FMENU_SHOWAMPERSAND        = $0001;
   FMENU_WRAPMODE             = $0002;
   FMENU_AUTOHIGHLIGHT        = $0004;
   FMENU_REVERSEAUTOHIGHLIGHT = $0008;
   FMENU_USEEXT               = $0020;
   FMENU_CHANGECONSOLETITLE   = $0040;

type
   TFarApiMenu = function (
         PluginNumber : Integer;
         X, Y : Integer;
         MaxHeight : Integer;
         Flags : DWORD;
         const Title : PChar;
         const Bottom : PChar;
         const HelpTopic : PChar;
         const BreakKeys : PIntegerArray;
         BreakCode : PIntegerArray;
         const Item : PFarMenuItemArray;
         ItemsNumber : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

{ PLUGINPANELITEMFLAGS }

const
   PPIF_PROCESSDESCR = $80000000;
   PPIF_SELECTED     = $40000000;
   PPIF_USERDATA     = $20000000;

type
   PFarFindData = ^TFarFindData;
   TFarFindData = packed record
      dwFileAttributes : DWORD;
      ftCreationTime : TFileTime;
      ftLastAccessTime : TFileTime;
      ftLastWriteTime : TFileTime;
      nFileSizeHigh : DWORD;
      nFileSizeLow : DWORD;
      dwReserved0 : DWORD;
      dwReserved1 : DWORD;
      cFileName : array [0..MAX_PATH-1] of Char;
      cAlternateFileName : array [0..13] of Char;
   end;

type
   PPluginPanelItem = ^TPluginPanelItem;
   TPluginPanelItem = packed record
      FindData : TFarFindData;
      PackSizeHigh : DWORD;
      PackSize : DWORD;
      Flags : DWORD;
      NumberOfLinks : DWORD;
      Description : PChar;
      Owner : PChar;
      CustomColumnData : PPCharArray;
      CustomColumnNumber : Integer;
      UserData : DWORD;
      CRC32 : DWORD;
      Reserved : array [0..1] of DWORD;
   end;

type
   TPluginPanelItemArray = packed array[0..Pred(MaxLongint div sizeof(TPluginPanelItem))] of TPluginPanelItem;
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

{ PANELINFOTYPE }

const
   PTYPE_FILEPANEL  = 0;
   PTYPE_TREEPANEL  = 1;
   PTYPE_QVIEWPANEL = 2;
   PTYPE_INFOPANEL  = 3;

type
   PPanelInfo = ^TPanelInfo;
   TPanelInfo = packed record
      PanelType : Integer;
      Plugin : Integer;
      PanelRect : TRect;
      PanelItems : PPluginPanelItemArray;
      ItemsNumber : Integer;
      SelectedItems : PPluginPanelItemArray;
      SelectedItemsNumber : Integer;
      CurrentItem : Integer;
      TopPanelItem : Integer;
      Visible : Integer;
      Focus : Integer;
      ViewMode : Integer;
      ColumnTypes : array [0..79] of Char;
      ColumnWidths : array [0..79] of Char;
      CurDir : array [0..NM-1] of Char;
      ShortNames : Integer;
      SortMode : Integer;
      Flags : DWORD;
      Reserved : DWORD;
   end;

type
   PPanelRedrawInfo = ^TPanelRedrawInfo;
   TPanelRedrawInfo = packed record
      CurrentItem : Integer;
      TopPanelItem : Integer;
   end;

type
   PCmdLineSelect = ^TCmdLineSelect;
   TCmdLineSelect = packed record
      SelStart : Integer;
      SelEnd : Integer;
   end;

{ FILE_CONTROL_COMMANDS }

const
   FCTL_CLOSEPLUGIN              = 0;
   FCTL_GETPANELINFO             = 1;
   FCTL_GETANOTHERPANELINFO      = 2;
   FCTL_UPDATEPANEL              = 3;
   FCTL_UPDATEANOTHERPANEL       = 4;
   FCTL_REDRAWPANEL              = 5;
   FCTL_REDRAWANOTHERPANEL       = 6;
   FCTL_SETANOTHERPANELDIR       = 7;
   FCTL_GETCMDLINE               = 8;
   FCTL_SETCMDLINE               = 9;
   FCTL_SETSELECTION             = 10;
   FCTL_SETANOTHERSELECTION      = 11;
   FCTL_SETVIEWMODE              = 12;
   FCTL_SETANOTHERVIEWMODE       = 13;
   FCTL_INSERTCMDLINE            = 14;
   FCTL_SETUSERSCREEN            = 15;
   FCTL_SETPANELDIR              = 16;
   FCTL_SETCMDLINEPOS            = 17;
   FCTL_GETCMDLINEPOS            = 18;
   FCTL_SETSORTMODE              = 19;
   FCTL_SETANOTHERSORTMODE       = 20;
   FCTL_SETSORTORDER             = 21;
   FCTL_SETANOTHERSORTORDER      = 22;
   FCTL_GETCMDLINESELECTEDTEXT   = 23;
   FCTL_SETCMDLINESELECTION      = 24;
   FCTL_GETCMDLINESELECTION      = 25;
   FCTL_GETPANELSHORTINFO        = 26;
   FCTL_GETANOTHERPANELSHORTINFO = 27;
   FCTL_CHECKPANELSEXIST         = 28;
   FCTL_SETNUMERICSORT           = 29;
   FCTL_SETANOTHERNUMERICSORT    = 30;

type
   TFarApiControl = function (
         hPlugin : THandle;
         Command : Integer;
         Param : Pointer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarApiText = function (
         X, Y : Integer;
         Color : Integer;
         const Str : PChar
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarApiSaveScreen = function (
         X1, Y1, X2, Y2 : Integer
         ) : THandle; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarApiRestoreScreen = procedure (
         hScreen : THandle
         ); {$IFNDEF VP} stdcall; {$ENDIF}

   TFarApiGetDirList = function (
         const Dir : PChar;
         var PanelIten : PPluginPanelItemArray;
         var ItemsNumber : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarApiGetPluginDirList = function (
         PluginNumber : Integer;
         hPlugin : THandle;
         const Dir : PChar;
         var PanelItem : PPluginPanelItemArray;
         var ItemsNumber : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarApiFreeDirList = procedure (
         const PanelItem : PPluginPanelItemArray
         ); {$IFNDEF VP} stdcall; {$ENDIF}

{ VIEWER_FLAGS }

const
   VF_NONMODAL              = $00000001;
   VF_DELETEONCLOSE         = $00000002;
   VF_ENABLE_F6             = $00000004;
   VF_DISABLEHISTORY        = $00000008;
   VF_IMMEDIATERETURN       = $00000100;
   VF_DELETEONLYFILEONCLOSE = $00000200;

type
   TFarApiViewer = function (
         const FileName : PChar;
         const Title : PChar;
         X1, Y1, X2, Y2 : Integer;
         Flags : DWORD
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

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
   TFarApiEditor = function (
         const FileName : PChar;
         const Title : PChar;
         X1, Y1, X2, Y2 : Integer;
         Flags : DWORD;
         StartLine : Integer;
         StartChar : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarApiCmpName = function (
         const Pattern : PChar;
         const aString : PChar;
         SkipPath : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

{ FARCHARTABLE_COMMAND }

const
   FCT_DETECT = $40000000;

type
   PCharTableSet= ^TCharTableSet;
   TCharTableSet = packed record
      DecodeTable : array [0..255] of Byte;
      EncodeTable : array [0..255] of Byte;
      UpperTable : array [0..255] of Byte;
      LowerTable : array [0..255] of Byte;
      TableName : array [0..127] of Char;
   end;

type
   TFarApiCharTable = function (
         Command : Integer;
         Buffer : PChar;
         BufferSize : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarApiGetMsg = function (
         PluginNumber : Integer;
         MsgId : Integer
         ) : PChar; {$IFNDEF VP} stdcall; {$ENDIF}

{ FarHelpFlags }

const
   FHELP_NOSHOWERROR = $80000000;
   FHELP_SELFHELP    = $00000000;
   FHELP_FARHELP     = $00000001;
   FHELP_CUSTOMFILE  = $00000002;
   FHELP_CUSTOMPATH  = $00000004;
   FHELP_USECONTENTS = $40000000;

type
   TFarApiShowHelp = function (
         const ModuleName : PChar;
         const Topic : PChar;
         Flags : DWORD
         ) : LongBool; {$IFNDEF VP} stdcall; {$ENDIF}

{ ADVANCED_CONTROL_COMMANDS }

const
   ACTL_GETFARVERSION        = 0;
   ACTL_CONSOLEMODE          = 1;
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
   ACTL_GETWCHARMODE         = 20;
   ACTL_GETPLUGINMAXREADDATA = 21;
   ACTL_GETDIALOGSETTINGS    = 22;
   ACTL_GETSHORTWINDOWINFO   = 23;

{ FarSystemSettings }

const
   FSS_CLEARROATTRIBUTE          = $00000001;
   FSS_DELETETORECYCLEBIN        = $00000002;
   FSS_USESYSTEMCOPYROUTINE      = $00000004;
   FSS_COPYFILESOPENEDFORWRITING = $00000008;
   FSS_CREATEFOLDERSINUPPERCASE  = $00000010;
   FSS_SAVECOMMANDSHISTORY       = $00000020;
   FSS_SAVEFOLDERSHISTORY        = $00000040;
   FSS_SAVEVIEWANDEDITHISTORY    = $00000080;
   FSS_USEWINDOWSREGISTEREDTYPES = $00000100;
   FSS_AUTOSAVESETUP             = $00000200;
   FSS_SCANSYMLINK               = $00000400;

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

{ FarDescriptionSettings }

const
   FDS_UPDATEALWAYS      = $00000001;
   FDS_UPDATEIFDISPLAYED = $00000002;
   FDS_SETHIDDEN         = $00000004;
   FDS_UPDATEREADONLY    = $00000008;

const
   FAR_CONSOLE_GET_MODE = -2;
   FAR_CONSOLE_TRIGGER = -1;
   FAR_CONSOLE_SET_WINDOWED = 0;
   FAR_CONSOLE_SET_FULLSCREEN = 1;
   FAR_CONSOLE_WINDOWED = 0;
   FAR_CONSOLE_FULLSCREEN = 1;

{ FAREJECTMEDIAFLAGS }

const
   EJECT_NO_MESSAGE = $00000001;
   EJECT_LOAD_MEDIA = $00000002;

type
   PActlEjectMedia = ^TActlEjectMedia;
   TActlEjectMedia = packed record
      Letter : DWORD;
      Flags : DWORD;
   end;

{ FARKEYSEQUENCEFLAGS }

const
   KSFLAGS_DISABLEOUTPUT       = $00000001;
   KSFLAGS_NOSENDKEYSTOPLUGINS = $00000002;

type
   PKeySequence = ^TKeySequence;
   TKeySequence = packed record
      Flags : DWORD;
      Count : Integer;
      Sequence : ^DWORD;
   end;

{ FARMACROCOMMAND }

const
   MCMD_LOADALL         = 0;
   MCMD_SAVEALL         = 1;
   MCMD_POSTMACROSTRING = 2;

type
   PPlainText = ^TPlainText;
   TPlainText = packed record
      SequenceText : PChar;
      Flags : DWORD;
  end;

type
   PActlKeyMacro = ^TActlKeyMacro;
   TActlKeyMacro = packed record
      Command : Integer;

      Param : record case Integer of
         0 : (PlainText : TPlainText);
         1 : (Reserved : array [0..2] of DWORD);
      end;
   end;

{ FARCOLORFLAGS }

const
   FCLR_REDRAW = $00000001;

type
   PFarSetColors = ^TFarSetColors;
   TFarSetColors = packed record
      Flags : DWORD;
      StartIndex : Integer;
      ColorCount : Integer;
      Colors : PChar;
   end;

{ WINDOWINFO_TYPE }

const
   WTYPE_PANELS = 1;
   WTYPE_VIEWER = 2;
   WTYPE_EDITOR = 3;
   WTYPE_DIALOG = 4;
   WTYPE_VMENU  = 5;
   WTYPE_HELP   = 6;

type
   PWindowInfo = ^TWindowInfo;
   TWindowInfo = packed record
      Pos : Integer;
      WindowType : Integer;
      Modified : Integer;
      Current : Integer;
      TypeName : array [0..63] of Char;
      Name : array [0..NM-1] of Char;
   end;

type
   TFarApiAdvControl = function (
         ModuleNumber : Integer;
         Command : Integer;
         Param : Pointer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

{ VIEWER_CONTROL_COMMANDS }

const
   VCTL_GETINFO     = 0;
   VCTL_QUIT        = 1;
   VCTL_REDRAW      = 2;
   VCTL_SETKEYBAR   = 3;
   VCTL_SETPOSITION = 4;
   VCTL_SELECT      = 5;

{ VIEWER_OPTIONS }

const
   VOPT_SAVEFILEPOSITION = 1;
   VOPT_AUTODETECTTABLE  = 2;

type
   TFarInt64Part = packed record
      LowPart : DWORD;
      HighPart : DWORD;
   end;

type
   TFarInt64 = packed record
      case Integer of

{$IFDEF USE_DELPHI4}
         0 : (i64 : Int64);
{$ENDIF}
         1 : (Part : TFarInt64Part);
   end;

type
   PViewerSelect = ^TViewerSelect;
   TViewerSelect = packed record
      BlockStartPos : TFarInt64;
      BlockLen : Integer;
   end;

{ VIEWER_SETPOS_FLAGS }

const
   VSP_NOREDRAW    = $0001;
   VSP_PERCENT     = $0002;
   VSP_RELATIVE    = $0004;
   VSP_NORETNEWPOS = $0008;

type
   PViewerSetPosition = ^TViewerSetPosition;
   TViewerSetPosition = packed record
      Flags : DWORD;
      StartPos : TFarInt64;
      LeftPos : Integer;
   end;

type
   PViewerMode = ^TViewerMode;
   TViewerMode = packed record
      UseDecodeTable : Integer;
      TableNum : Integer;
      AnsiMode : Integer;
      Unicode : Integer;
      Wrap : Integer;
      TypeWrap : Integer;
      Hex : Integer;
      Reserved : array [0..3] of DWORD;
   end;

type
   PViewerInfo = ^TViewerInfo;
   TViewerInfo = packed record
      StructSize : Integer;
      ViewerID : Integer;
      FileName : PChar;
      FileSize : TFarInt64;
      FilePos : TFarInt64;
      WindowSizeX : Integer;
      WindowSizeY : Integer;
      Options : DWORD;
      TabSize : Integer;
      CurMode : TViewerMode;
      LeftPos : Integer;
      Reserved3 : DWORD;
   end;

type
   TFarApiViewerControl = function (
         Command : Integer;
         Param : Pointer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

{ VIEWER_EVENTS }

const
   VE_READ  = 0;
   VE_CLOSE = 1;

{ EDITOR_EVENTS }

const
   EE_READ   = 0;
   EE_SAVE   = 1;
   EE_REDRAW = 2;
   EE_CLOSE  = 3;

const
   EEREDRAW_ALL    = Pointer(0);
   EEREDRAW_CHANGE = Pointer(1);
   EEREDRAW_LINE   = Pointer(2);

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
   ECTL_EDITORTOOEM         = 10;
   ECTL_OEMTOEDITOR         = 11;
   ECTL_TABTOREAL           = 12;
   ECTL_REALTOTAB           = 13;
   ECTL_EXPANDTABS          = 14;
   ECTL_SETTITLE            = 15;
   ECTL_READINPUT           = 16;
   ECTL_PROCESSINPUT        = 17;
   ECTL_ADDCOLOR            = 18;
   ECTL_GETCOLOR            = 19;
   ECTL_SAVEFILE            = 20;
   ECTL_QUIT                = 21;
   ECTL_SETKEYBAR           = 22;
   ECTL_PROCESSKEY          = 23;
   ECTL_SETPARAM            = 24;
   ECTL_GETBOOKMARKS        = 25;
   ECTL_TURNOFFMARKINGBLOCK = 26;
   ECTL_DELETEBLOCK         = 27;

{ EDITOR_SETPARAMETER_TYPES }

const
   ESPT_TABSIZE          = 0;
   ESPT_EXPANDTABS       = 1;
   ESPT_AUTOINDENT       = 2;
   ESPT_CURSORBEYONDEOL  = 3;
   ESPT_CHARCODEBASE     = 4;
   ESPT_CHARTABLE        = 5;
   ESPT_SAVEFILEPOSITION = 6;
   ESPT_LOCKMODE         = 7;
   ESPT_SETWORDDIV       = 8;
   ESPT_GETWORDDIV       = 9;

type
   PEditorSetParameter = ^TEditorSetParameter;
   TEditorSetParameter = packed record
      ParamType : Integer;

      Param : record case Integer of
         0 : (iParam : Integer);
         1 : (cParam : PChar);
         2 : (Reserved : DWORD);
      end;

      Flags : DWORD;
      Reserved2 : DWORD;
   end;

type
   PEditorGetString = ^TEditorGetString;
   TEditorGetString = packed record
      StringNumber : Integer;
      StringText : PChar;
      StringEOL : PChar;
      StringLength : Integer;
      SelStart : Integer;
      SelEnd : Integer;
   end;

type
   PEditorSetString = ^TEditorSetString;
   TEditorSetString = packed record
      StringNumber : Integer;
      StringText : PChar;
      StringEOL : PChar;
      StringLength : Integer;
   end;

{ EXPAND_TABS }

const
   EXPAND_NOTABS  = 0;
   EXPAND_ALLTABS = 1;
   EXPAND_NEWTABS = 2;

{ EDITOR_OPTIONS }

const
   EOPT_EXPANDALLTABS     = $00000001;
   EOPT_PERSISTENTBLOCKS  = $00000002;
   EOPT_DELREMOVESBLOCKS  = $00000004;
   EOPT_AUTOINDENT        = $00000008;
   EOPT_SAVEFILEPOSITION  = $00000010;
   EOPT_AUTODETECTTABLE   = $00000020;
   EOPT_CURSORBEYONDEOL   = $00000040;
   EOPT_EXPANDONLYNEWTABS = $00000080;

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

type
   PEditorInfo = ^TEditorInfo;
   TEditorInfo = packed record
      EditorID : Integer;
      FileName : PChar;
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
      AnsiMode : Integer;
      TableNum : Integer;
      Options : DWORD;
      TabSize : Integer;
      BookMarkCount : Integer;
      CurState : DWORD;
      Reserved : array [0..5] of DWORD;
   end;

type
   PEditorBookMarks = ^TEditorBookMarks;
   TEditorBookMarks = packed record
      Line : PIntegerArray;
      Cursor : PIntegerArray;
      ScreenLine : PIntegerArray;
      LeftPos : PIntegerArray;
      Reserved : array [0..3] of DWORD;
   end;

type
   PEditorSetPosition = ^TEditorSetPosition;
   TEditorSetPosition = packed record
      CurLine : Integer;
      CurPos : Integer;
      CurTabPos : Integer;
      TopScreenLine : Integer;
      LeftPos : Integer;
      Overtype : Integer;
   end;

type
   PEditorSelect = ^TEditorSelect;
   TEditorSelect = packed record
      BlockType : Integer;
      BlockStartLine : Integer;
      BlockStartPos : Integer;
      BlockWidth : Integer;
      BlockHeight : Integer;
   end;

type
   PEditorConvertText = ^TEditorConvertText;
   TEditorConvertText = packed record
      Text : PChar;
      TextLength : Integer;
   end;

type
   PEditorConvertPos = ^TEditorConvertPos;
   TEditorConvertPos = packed record
      StringNumber : Integer;
      SrcPos : Integer;
      DestPos : Integer;
   end;

{ EDITORCOLORFLAGS }

const
   ECF_TAB1 = $10000;

type
   PEditorColor = ^TEditorColor;
   TEditorColor = packed record
      StringNumber : Integer;
      ColorItem : Integer;
      StartPos : Integer;
      EndPos : Integer;
      Color : Integer;
   end;

type
   PEditorSaveFile = ^TEditorSaveFile;
   TEditorSaveFile = packed record
      FileName : array [0..NM-1] of Char;
      FileEOL : PChar;
   end;

type
   TFarApiEditorControl = function (
         Command : Integer;
         Param : Pointer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

{ INPUTBOXFLAGS }

const
   FIB_ENABLEEMPTY      = $00000001;
   FIB_PASSWORD         = $00000002;
   FIB_EXPANDENV        = $00000004;
   FIB_NOUSELASTHISTORY = $00000008;
   FIB_BUTTONS          = $00000010;
   FIB_NOAMPERSAND      = $00000020;

type
   TFarApiInputBox = function (
         const Title : PChar;
         const SubTitle : PChar;
         const HistoryName : PChar;
         const SrcText : PChar;
         DestText : PChar;
         DestLength : Integer;
         const HelpTopic : PChar;
         Flags : DWORD
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   {&Cdecl+}
   TFarStdQSortFunc = function (
         Param1 : Pointer;
         Param2 : Pointer
         ) : Integer; {$IFNDEF VP} cdecl; {$ENDIF}

   {&StdCall+}
   TFarStdQSort = procedure (
         Base : Pointer;
         NElem : DWORD;
         Width : DWORD;
{$IFNDEF VP}
         FCmp : TFarStdQSortFunc
{$ELSE}
         FCmp : Pointer
{$ENDIF}
         ); {$IFNDEF VP} stdcall; {$ENDIF}

   {&Cdecl+}
   TFarStdQSortExFunc = function (
         Param1 : Pointer;
         Param2 : Pointer;
         UserParam : Pointer
         ) : Integer; {$IFNDEF VP} cdecl; {$ENDIF}

   {&StdCall+}
   TFarStdQSortEx = procedure (
         Base : Pointer;
         NElem : DWORD;
         Width : DWORD;
{$IFNDEF VP}
         FCmp : TFarStdQSortExFunc;
{$ELSE}
         FCmp : Pointer;
{$ENDIF}
         UserParam : Pointer

         ); {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdBSearch = procedure (
         const Key : Pointer;
         const Base : Pointer;
         NElem : DWORD;
         Width : DWORD;
{$IFNDEF VP}
         FCmp : TFarStdQSortFunc
{$ELSE}
         FCmp : Pointer
{$ENDIF}
         ); {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdGetFileOwner = function (
         const Computer : PChar;
         const Name : PChar;
         Owner : PChar
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdGetNumberOfLinks = function (
         const Name : PChar
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdAtoi = function (
         const S : PChar
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

{$IFDEF USE_DELPHI4}

   TFarStdAtoi64 = function (
         const S : PChar
         ) : Int64; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdItoa64 = function (
         Value : Int64;
         Str : PChar;
         Radix : Integer
         ) : PChar; {$IFNDEF VP} stdcall; {$ENDIF}

{$ENDIF}

   TFarStdItoa = function (
         Value : Integer;
         Str : PChar;
         Radix : Integer
         ) : PChar; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdLTrim = function (
         Str : PChar
         ) : PChar; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdRTrim = function (
         Str : PChar
         ) : PChar; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdTrim = function (
         Str : PChar
         ) : PChar; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdTruncStr = function (
         Str : PChar;
         MaLength : Integer
         ) : PChar; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdTruncPathStr = function (
         Str : PChar;
         MaxLength : Integer
         ) : PChar; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdQuoteSpaceOnly = function (
         Str : PChar
         ) : PChar; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdPointToName = function (
         const Path : PChar
         ) : PChar; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdGetPathRoot = procedure (
         const Path : PChar;
         Root : PChar
         ); {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdAddEndSlash = function (
         Path : PChar
         ) : LongBool; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdCopyToClipBoard = function (
         const Data : PChar
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdPasteFromClipboard = function : PChar; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdInputRecordToKey = function (
         const R : TInputRecord
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdLocalIsLower = function (
         Ch : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdLocalIsUpper = function (
         Ch : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdLocalIsAlpha = function (
         Ch : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdLocalIsAlphaNum = function (
         Ch : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdLocalUpper = function (
         LowerChar : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdLocalLower = function (
         UpperChar : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdLocalUpperBuf = function (
         Buf : PChar;
         Length : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdLocalLowerBuf = function (
         Buf : PChar;
         Length : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdLocalStrUpr = function (
         S1 : PChar
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdLocalStrLwr = function (
         S1 : PChar
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdLocalStrICmp = function (
         const S1 : PChar;
         const S2 : PChar
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdLocalStrNICmp = function (
         const S1 : PChar;
         const S2 : PChar;
         N : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

{ PROCESSNAME_FLAGS }

const
   PN_CMPNAME      = $00000000;
   PN_CMPNAMELIST  = $00001000;
   PN_GENERATENAME = $00002000;
   PN_SKIPPATH     = $00100000;


type
   TFarStdProcessName = function (
         const Param1 : PChar;
         Param2 : PChar;
         Flags : DWORD
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdUnquote = procedure (
         Str : PChar
         ); {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdExpandEnvironmentStr = function (
         const Src : PChar;
         Dst : PChar;
         Size : DWORD
         ) : DWORD; {$IFNDEF VP} stdcall; {$ENDIF}

{ XLATMODE }

const
   XLAT_SWITCHKEYBLAYOUT = $00000001;
   XLAT_SWITCHKEYBBEEP   = $00000002;

type
   TFarStdXLat = function (
        Line : PChar;
        StartPos : Integer;
        EndPos : Integer;
        TableSet : PCharTableSet;
        Flags : DWORD
        ) : PChar; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdKeyToKeyName = function (
        Key : Integer;
        KeyText : PChar;
        Size : Integer
        ) : LongBool; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdKeyNameToKey = function (
        const Name : PChar
        ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFRSUserFunc = function (
        const FData : PFarFindData;
        const FullName : PChar;
        Param : Pointer
        ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}


{ FRSMODE }

const
   FRS_RETUPDIR    = $01;
   FRS_RECUR       = $02;
   FRS_SCANSYMLINK = $04;

type
   TFarStdRecursiveSearch = procedure (
         const InitDir : PChar;
         const Mask : PChar;
{$IFNDEF VP}
         Func : TFRSUserFunc;
{$ELSE}
         Func : Pointer;
{$ENDIF}
         Flags : DWORD;
         Param : Pointer
         ); {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdMkTemp = function (
         Dest : PChar;
         const Prefix : PChar
         ) : PChar; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdDeleteBuffer = procedure (
         Buffer : PChar
         ); {$IFNDEF VP} stdcall; {$ENDIF}

{ MKLINKOP }

const
   FLINK_HARDLINK         = 1;
   FLINK_SYMLINK          = 2;
   FLINK_VOLMOUNT         = 3;

   FLINK_SHOWERRMSG       = $10000;
   FLINK_DONOTUPDATEPANEL = $20000;

type
   TFarStdMkLink = function (
         const Src : PChar;
         const Dest : PChar;
         Flags : DWORD
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdConvertNameToReal = function (
         const Src : PChar;
         Dest : PChar;
         DestSize : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

   TFarStdGetReparsePointInfo = function (
         const Src : PChar;
         Dest : PChar;
         DestSize : Integer
         ) : Integer; {$IFNDEF VP} stdcall; {$ENDIF}

type
   PFarStandardFunctions = ^TFarStandardFunctions;
   TFarStandardFunctions = packed record
      StructSize : Integer;

      atoi : TFarStdAtoi;

{$IFDEF USE_DELPHI4}
      atoi64 : TFarStdAtoi64;
{$ELSE}
      atoi64 : Pointer;
{$ENDIF}

      itoa : TFarStdItoa;

{$IFDEF USE_DELPHI4}
      itoa64 : TFarStdItoa64;
{$ELSE}
      itoa64 : Pointer;
{$ENDIF}

      sprintf : Pointer;
      sscanf : Pointer;

      qsort : TFarStdQSort;
      bsearch : TFarStdBSearch;
      qsortex : TFarStdQSortEx;

      Reserved : array [0..8] of DWORD;

      LIsLower : TFarStdLocalIsLower;
      LIsUpper : TFarStdLocalIsUpper;
      LIsAlpha : TFarStdLocalIsAlpha;
      LIsAlphaNum : TFarStdLocalIsAlphaNum;
      LUpper : TFarStdLocalUpper;
      LLower : TFarStdLocalLower;
      LUpperBuf : TFarStdLocalUpperBuf;
      LLowerBuf : TFarStdLocalLowerBuf;
      LStrupr : TFarStdLocalStrUpr;
      LStrlwr : TFarStdLocalStrLwr;
      LStricmp : TFarStdLocalStrICmp;
      LStrnicmp : TFarStdLocalStrNICmp;

      Unquote : TFarStdUnquote;
      ExpandEnvironmentStr : TFarStdExpandEnvironmentStr;
      LTrim : TFarStdLTrim;
      RTrim : TFarStdRTrim;
      Trim : TFarStdTrim;
      TruncStr : TFarStdTruncStr;
      TruncPathStr : TFarStdTruncPathStr;
      QuoteSpaceOnly : TFarStdQuoteSpaceOnly;
      PointToName : TFarStdPointToName;
      GetPathRoot : TFarStdGetPathRoot;
      AddEndSlash : TFarStdAddEndSlash;
      CopyToClipboard : TFarStdCopyToClipboard;
      PasteFromClipboard : TFarStdPasteFromClipboard;
      FarKeyToName : TFarStdKeyToKeyName;
      FarNameToKey : TFarStdKeyNameToKey;
      FarInputRecordToKey : TFarStdInputRecordToKey;
      XLat : TFarStdXLat;
      GetFileOwner : TFarStdGetFileOwner;
      GetNumberOfLinks : TFarStdGetNumberOfLinks;
      FarRecursiveSearch : TFarStdRecursiveSearch;
      MkTemp : TFarStdMkTemp;
      DeleteBuffer : TFarStdDeleteBuffer;
      ProcessName : TFarStdProcessName;
      MkLink : TFarStdMkLink;
      ConvertNameToReal : TFarStdConvertNameToReal;
      GetReparsePointInfo : TFarStdGetReparsePointInfo;
  end;

type
   PPluginStartupInfo = ^TPluginStartupInfo;
   TPluginStartupInfo = packed record
      StructSize : Integer;
      ModuleName : array [0..NM-1] of Char;
      ModuleNumber : Integer;
      RootKey : PChar;

      Menu : TFarApiMenu;
      Dialog : TFarApiDialog;
      Message : TFarApiMessage;
      GetMsg : TFarApiGetMsg;
      Control : TFarApiControl;
      SaveScreen : TFarApiSaveScreen;
      RestoreScreen : TFarApiRestoreScreen;
      GetDirList : TFarApiGetDirList;
      GetPluginDirList : TFarApiGetPluginDirList;
      FreeDirList : TFarApiFreeDirList;
      Viewer : TFarApiViewer;
      Editor : TFarApiEditor;
      CmpName : TFarApiCmpName;
      CharTable : TFarApiCharTable;
      Text : TFarApiText;
      EditorControl : TFarApiEditorControl;

      FSF : TFarStandardFunctions;

      ShowHelp : TFarApiShowHelp;
      AdvControl : TFarApiAdvControl;
      InputBox : TFarApiInputBox;
      DialogEx : TFarApiDialogEx;
      SendDlgMessage : TFarApiSendDlgMessage;
      DefDlgProc : TFarApiDefDlgProc;
      Reserved : DWORD;
      ViewerControl : TFarApiViewerControl;
   end;

{ PLUGIN_FLAGS }

const
   PF_PRELOAD       = $0001;
   PF_DISABLEPANELS = $0002;
   PF_EDITOR        = $0004;
   PF_VIEWER        = $0008;
   PF_FULLCMDLINE   = $0010;

type
   PPluginInfo = ^TPluginInfo;
   TPluginInfo = packed record
      StructSize : Integer;
      Flags : DWORD;
      DiskMenuStrings : PPCharArray;
      DiskMenuNumbers : PIntegerArray;
      DiskMenuStringsNumber : Integer;
      PluginMenuStrings : PPCharArray;
      PluginMenuStringsNumber : Integer;
      PluginConfigStrings : PPCharArray;
      PluginConfigStringsNumber : Integer;
      CommandPrefix : PChar;
      Reserved : DWORD;
   end;

type
   PInfoPanelLine = ^TInfoPanelLine;
   TInfoPanelLine = packed record
      Text : array [0..79] of Char;
      Data : array [0..79] of Char;
      Separator : Integer;
   end;

type
   PInfoPanelLineArray = ^TInfoPanelLineArray;
   TInfoPanelLineArray = packed array [0..Pred(MaxLongint div SizeOf(TInfoPanelLine))] of TInfoPanelLine;

type
   PPanelMode = ^TPanelMode;
   TPanelMode = packed record
      ColumnTypes : PChar;
      ColumnWidths : PChar;
      ColumnTitles : PPCharArray;
      FullScreen : Integer;
      DetailedStatus : Integer;
      AlignExtensions : Integer;
      CaseConversion : Integer;
      StatusColumnTypes : PChar;
      StatusColumnWidths : PChar;
      Reserved : array [0..1] of DWORD;
   end;

type
   PPanelModeArray = ^TPanelModeArray;
   TPanelModeArray = packed array [0..Pred(MaxLongint div SizeOf(TPanelMode))] of TPanelMode;

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
   OPIF_FINDFOLDERS         = $00000200;
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

type
   PKeyBarTitles = ^TKeyBarTitles;
   TKeyBarTitles = packed record
      Titles : array [0..11] of PChar;
      CtrlTitles : array [0..11] of PChar;
      AltTitles : array [0..11] of PChar;
      ShiftTitles : array [0..11] of PChar;
      CtrlShiftTitles : array [0..11] of PChar;
      AltShiftTitles : array [0..11] of PChar;
      CtrlAltTitles : array [0..11] of PChar;
   end;

type
   PKeyBarTitlesArray = ^TKeyBarTitlesArray;
   TKeyBarTitlesArray = packed array [0..Pred(MaxLongint div SizeOf(TKeyBarTitles))] of TKeyBarTitles;

{ OPERATION_MODES }

const
   OPM_SILENT    = $0001;
   OPM_FIND      = $0002;
   OPM_VIEW      = $0004;
   OPM_EDIT      = $0008;
   OPM_TOPLEVEL  = $0010;
   OPM_DESCR     = $0020;
   OPM_QUICKVIEW = $0040;

type
   POpenPluginInfo = ^TOpenPluginInfo;
   TOpenPluginInfo = packed record
      StructSize : Integer;
      Flags : DWORD;
      HostFile : PChar;
      CurDir : PChar;
      Format : PChar;
      PanelTitle : PChar;
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
      ShortcutData : PChar;
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

{ FAR_PKF_FLAGS }

const
   PKF_CONTROL    = $00000001;
   PKF_ALT        = $00000002;
   PKF_SHIFT      = $00000004;
   PKF_PREPROCESS = $00080000;

{ FAR_EVENTS }

const
   FE_CHANGEVIEWMODE = 0;
   FE_REDRAW         = 1;
   FE_IDLE           = 2;
   FE_CLOSE          = 3;
   FE_BREAK          = 4;
   FE_COMMAND        = 5;

function MakeFarVersion (Major : DWORD; Minor : DWORD; Build : DWORD) : DWORD;

function Dlg_RedrawDialog (Info : TPluginStartupInfo; hDlg : THandle) : Integer;
function Dlg_GetDlgData (Info : TPluginStartupInfo; hDlg : THandle) : Integer;
function Dlg_SetDlgData (Info : TPluginStartupInfo; hDlg : THandle; Data : Pointer) : Integer;
function Dlg_GetDlgItemData (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer) : Integer;
function Dlg_SetDlgItemData (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer; Data : Pointer) : Integer;
function DlgItem_GetFocus (Info : TPluginStartupInfo; hDlg : THandle) : Integer;
function DlgItem_SetFocus (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer) : Integer;
function DlgItem_Enable (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer) : Integer;
function DlgItem_Disable (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer) : Integer;
function DlgItem_IsEnable (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer) : Integer;
function DlgItem_SetText (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer; Str : PChar) : Integer;
function DlgItem_GetCheck (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer) : Integer;
function DlgItem_SetCheck (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer; State : Integer) : Integer;
function DlgEdit_AddHistory (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer; Str : PChar) : Integer;
function DlgList_AddString (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer; Str : PChar) : Integer;
function DlgList_GetCurPos (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer) : Integer;
function DlgList_SetCurPos (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer; NewPos : Integer) : Integer;
function DlgList_ClearList (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer) : Integer;
function DlgList_DeleteItem (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer; Index : Integer) : Integer;
function DlgList_SortUp (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer) : Integer;
function DlgList_SortDown (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer) : Integer;
function DlgList_GetItemData (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer; Index : Integer) : Integer;
function DlgList_SetItemStrAsData (Info : TPluginStartupInfo; hDlg : THandle; ID : Integer; Index : Integer; Str : PChar) : Integer;

implementation

function Dlg_RedrawDialog (
      Info : TPluginStartupInfo;
      hDlg : THandle
      ) : Integer;
begin
   Result := Info.SendDlgMessage (hDlg, DM_REDRAW, 0, 0);
end;

function Dlg_GetDlgData (
      Info : TPluginStartupInfo;
      hDlg : THandle
      ) : Integer;
begin
   Result := Info.SendDlgMessage (hDlg, DM_GETDLGDATA, 0, 0);
end;

function Dlg_SetDlgData (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      Data : Pointer
      ) : Integer;
begin
   Result := Info.SendDlgMessage (hDlg, DM_SETDLGDATA, 0, Integer(Data));
end;

function Dlg_GetDlgItemData (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer
      ) : Integer;
begin
   Result := Info.SendDlgMessage (hDlg, DM_GETITEMDATA, 0, 0);
end;

function Dlg_SetDlgItemData (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer;
      Data : Pointer
      ) : Integer;
begin
   Result := Info.SendDlgMessage (hDlg, DM_SETITEMDATA, 0, Integer(Data));
end;

function DlgItem_GetFocus (
      Info : TPluginStartupInfo;
      hDlg : THandle
      ) : Integer;
begin
   Result := Info.SendDlgMessage(hDlg,DM_GETFOCUS,0,0)
end;

function DlgItem_SetFocus (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer
      ) : Integer;
begin
   Result := Info.SendDlgMessage(hDlg,DM_SETFOCUS,ID,0)
end;

function DlgItem_Enable (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer
      ) : Integer;
begin
   Result := Info.SendDlgMessage (hDlg, DM_ENABLE, ID, 1);
end;

function DlgItem_Disable (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer
      ) : Integer;
begin
   Result := Info.SendDlgMessage (hDlg, DM_ENABLE, ID, 0);
end;

function DlgItem_IsEnable (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer
      ) : Integer;
begin
   Result := Info.SendDlgMessage (hDlg, DM_ENABLE, ID, -1);
end;

function DlgItem_SetText (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer;
      Str : PChar
      ) : Integer;
begin
   Result := Info.SendDlgMessage (hDlg, DM_SETTEXTPTR, ID, Integer(Str));
end;

function DlgItem_GetCheck (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer
      ) : Integer;
begin
   Result := Info.SendDlgMessage (hDlg, DM_GETCHECK, ID, 0);
end;

function DlgItem_SetCheck (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer;
      State : Integer
      ) : Integer;
begin
   Result := Info.SendDlgMessage (hDlg, DM_SETCHECK, ID, State);
end;

function DlgEdit_AddHistory (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer;
      Str : PChar
      ) : Integer;
begin
   Result := Info.SendDlgMessage (hDlg, DM_ADDHISTORY, ID, Integer(Str));
end;

function DlgList_AddString (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer;
      Str : PChar
      ) : Integer;
begin
   Result := Info.SendDlgMessage (hDlg, DM_LISTADDSTR, ID, Integer(Str));
end;

function DlgList_GetCurPos (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer
      ) : Integer;
begin
   Result := Info.SendDlgMessage (hDlg, DM_LISTGETCURPOS, ID, 0);
end;

function DlgList_SetCurPos (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer;
      NewPos : Integer
      ) : Integer;
var
   LPos : TFarListPos;
begin
   LPos.SelectPos := NewPos;
   LPos.TopPos := -1;
   Result := Info.SendDlgMessage (hDlg, DM_LISTSETCURPOS, ID, Integer(@LPos));
end;

function DlgList_ClearList (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer
      ) : Integer;
begin
   Result := Info.SendDlgMessage(hDlg,DM_LISTDELETE,ID,0)
end;

function DlgList_DeleteItem (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer;
      Index : Integer
      ) : Integer;
var
   FLDItem : TFarListDelete;
begin
   FLDItem.StartIndex := Index;
   FLDItem.Count := 1;
   Result := Info.SendDlgMessage (hDlg, DM_LISTDELETE, ID, Integer(@FLDItem));
end;

function DlgList_SortUp (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer
      ) : Integer;
begin
   Result := Info.SendDlgMessage (hDlg, DM_LISTSORT, ID, 0);
end;

function DlgList_SortDown (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer
      ) : Integer;
begin
   Result := Info.SendDlgMessage (hDlg, DM_LISTSORT, ID, 1);
end;

function DlgList_GetItemData (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer;
      Index : Integer
      ) : Integer;
begin
   Result := Info.SendDlgMessage (hDlg, DM_LISTGETDATA, ID, Index);
end;

function DlgList_SetItemStrAsData (
      Info : TPluginStartupInfo;
      hDlg : THandle;
      ID : Integer;
      Index : Integer;
      Str : PChar
      ) : Integer;
var
   FLID : TFarListItemData;
begin
   FLID.Index := Index;
   FLID.DataSize := 0;
   FLID.Data := Str;
   FLID.Reserved := 0;
   Result := Info.SendDlgMessage (hDlg, DM_LISTSETDATA, ID, Integer(@FLID));
end;

function MakeFarVersion (Major : DWORD; Minor : DWORD; Build : DWORD) : DWORD;
begin
   Result := (Major shl 8) or (Minor) or (Build shl 16);
end;

initialization
  FARMANAGERVERSION := MakeFarVersion (1, 70, 2087);

end.
