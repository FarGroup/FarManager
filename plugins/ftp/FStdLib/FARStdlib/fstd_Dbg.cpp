#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

CONSTSTR DECLSPEC GetDMStr( int Msg )
  {
    switch( Msg ) {
      case DM_FIRST: return "DM_FIRST";
      case DM_CLOSE: return "DM_CLOSE";
      case DM_ENABLE: return "DM_ENABLE";
      case DM_ENABLEREDRAW: return "DM_ENABLEREDRAW";
      case DM_GETDLGDATA: return "DM_GETDLGDATA";
      case DM_GETDLGITEM: return "DM_GETDLGITEM";
      case DM_GETDLGRECT: return "DM_GETDLGRECT";
      case DM_GETTEXT: return "DM_GETTEXT";
      case DM_GETTEXTLENGTH: return "DM_GETTEXTLENGTH";
      case DM_KEY: return "DM_KEY";
      case DM_MOVEDIALOG: return "DM_MOVEDIALOG";
      case DM_SETDLGDATA: return "DM_SETDLGDATA";
      case DM_SETDLGITEM: return "DM_SETDLGITEM";
      case DM_SETFOCUS: return "DM_SETFOCUS";
      case DM_REDRAW: return "DM_REDRAW";
      case DM_SETTEXT: return "DM_SETTEXT";
      case DM_SETMAXTEXTLENGTH: return "DM_SETMAXTEXTLENGTH";
      case DM_SHOWDIALOG: return "DM_SHOWDIALOG";
      case DM_GETFOCUS: return "DM_GETFOCUS";
      case DM_GETCURSORPOS: return "DM_GETCURSORPOS";
      case DM_SETCURSORPOS: return "DM_SETCURSORPOS";
      case DM_GETTEXTPTR: return "DM_GETTEXTPTR";
      case DM_SETTEXTPTR: return "DM_SETTEXTPTR";
      case DM_SHOWITEM: return "DM_SHOWITEM";
      case DM_ADDHISTORY: return "DM_ADDHISTORY";
      case DM_GETCHECK: return "DM_GETCHECK";
      case DM_SETCHECK: return "DM_SETCHECK";
      case DM_SET3STATE: return "DM_SET3STATE";
      case DM_LISTSORT: return "DM_LISTSORT";
      case DM_LISTGETITEM: return "DM_LISTGETITEM";
      case DM_LISTGETCURPOS: return "DM_LISTGETCURPOS";
      case DM_LISTSETCURPOS: return "DM_LISTSETCURPOS";
      case DM_LISTDELETE: return "DM_LISTDELETE";
      case DM_LISTADD: return "DM_LISTADD";
      case DM_LISTADDSTR: return "DM_LISTADDSTR";
      case DM_LISTUPDATE: return "DM_LISTUPDATE";
      case DM_LISTINSERT: return "DM_LISTINSERT";
      case DM_LISTFINDSTRING: return "DM_LISTFINDSTRING";
      case DM_LISTINFO: return "DM_LISTINFO";
      case DM_LISTGETDATA: return "DM_LISTGETDATA";
      case DM_LISTSETDATA: return "DM_LISTSETDATA";
      case DM_LISTSETTITLES: return "DM_LISTSETTITLES";
      case DM_LISTGETTITLES: return "DM_LISTGETTITLES";
      case DM_RESIZEDIALOG: return "DM_RESIZEDIALOG";
      case DM_SETITEMPOSITION: return "DM_SETITEMPOSITION";
      case DM_GETDROPDOWNOPENED: return "DM_GETDROPDOWNOPENED";
      case DM_SETDROPDOWNOPENED: return "DM_SETDROPDOWNOPENED";
      case DM_SETHISTORY: return "DM_SETHISTORY";
      case DM_GETITEMPOSITION: return "DM_GETITEMPOSITION";
      case DM_SETMOUSEEVENTNOTIFY: return "DM_SETMOUSEEVENTNOTIFY";
      case DM_EDITUNCHANGEDFLAG: return "DM_EDITUNCHANGEDFLAG";
      case DM_GETITEMDATA: return "DM_GETITEMDATA";
      case DM_SETITEMDATA: return "DM_SETITEMDATA";
      case DM_LISTSET: return "DM_LISTSET";
      case DM_LISTSETMOUSEREACTION: return "DM_LISTSETMOUSEREACTION";
      case DM_GETCURSORSIZE: return "DM_GETCURSORSIZE";
      case DM_SETCURSORSIZE: return "DM_SETCURSORSIZE";
      case DM_LISTGETDATASIZE: return "DM_LISTGETDATASIZE";
      case DN_FIRST: return "DN_FIRST";
      case DN_BTNCLICK: return "DN_BTNCLICK";
      case DN_CTLCOLORDIALOG: return "DN_CTLCOLORDIALOG";
      case DN_CTLCOLORDLGITEM: return "DN_CTLCOLORDLGITEM";
      case DN_CTLCOLORDLGLIST: return "DN_CTLCOLORDLGLIST";
      case DN_DRAWDIALOG: return "DN_DRAWDIALOG";
      case DN_DRAWDLGITEM: return "DN_DRAWDLGITEM";
      case DN_EDITCHANGE: return "DN_EDITCHANGE";
      case DN_ENTERIDLE: return "DN_ENTERIDLE";
      case DN_GOTFOCUS: return "DN_GOTFOCUS";
      case DN_HELP: return "DN_HELP";
      case DN_HOTKEY: return "DN_HOTKEY";
      case DN_INITDIALOG: return "DN_INITDIALOG";
      case DN_KILLFOCUS: return "DN_KILLFOCUS";
      case DN_LISTCHANGE: return "DN_LISTCHANGE";
      case DN_MOUSECLICK: return "DN_MOUSECLICK";
      case DN_DRAGGED: return "DN_DRAGGED";
      case DN_RESIZECONSOLE: return "DN_RESIZECONSOLE";
      case DN_MOUSEEVENT: return "DN_MOUSEEVENT";
      case DN_DRAWDIALOGDONE: return "DN_DRAWDIALOGDONE";
    }

    static char str[20];

    if ( Msg > DM_USER )
      sprintf( str,"usr: %d",Msg-DM_USER );
     else
      sprintf( str,"unk: %08X",Msg );

 return str;
}
