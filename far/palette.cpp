/*
palette.cpp

Таблица цветов

*/

/* Revision: 1.10 19.09.2002 $ */

/*
Modify:
  19.09.2002 SVS
    ! Немного антуража ;-)
  07.06.2001 SVS
    + COL_EDITORCLOCK, COL_VIEWERCLOCK
  21.05.2001 SVS
    + COL_MENUDISABLEDTEXT
  06.05.2001 DJ
    ! перетрях #include
  04.12.2000 SVS
    + COL_*DIALOG*DISABLED
  06.10.2000 SVS
    - попытка ?1 исправить падение ФАРы в ночное время :-)
  13.09.2000 tran 1.04
    + COL_COMMANDLINEPREFIX
  18.07.2000 tran 1.03
    + скролбар во вьювере
  11.07.2000
    + 1 Служебная позиция перед новыми цветами :-)
  29.06.2000 SVS
    + Цветовые атрибуты для Menu ScrollBar
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "colors.hpp"

unsigned char DefaultPalette[]=
{
  F_WHITE|B_CYAN,                         // COL_MENUTEXT,
  F_WHITE|B_BLACK,                        // COL_MENUSELECTEDTEXT,
  F_YELLOW|B_CYAN,                        // COL_MENUHIGHLIGHT,
  F_YELLOW|B_BLACK,                       // COL_MENUSELECTEDHIGHLIGHT,
  F_WHITE|B_CYAN,                         // COL_MENUBOX,
  F_WHITE|B_CYAN,                         // COL_MENUTITLE,

  F_BLACK|B_CYAN,                         // COL_HMENUTEXT,
  F_WHITE|B_BLACK,                        // COL_HMENUSELECTEDTEXT,
  F_YELLOW|B_CYAN,                        // COL_HMENUHIGHLIGHT,
  F_YELLOW|B_BLACK,                       // COL_HMENUSELECTEDHIGHLIGHT,

  F_LIGHTCYAN|B_BLUE,                     // COL_PANELTEXT,
  F_YELLOW|B_BLUE,                        // COL_PANELSELECTEDTEXT,
  F_LIGHTGRAY|B_BLUE,                     // COL_PANELHIGHLIGHTTEXT,
  F_YELLOW|B_BLUE,                        // COL_PANELINFOTEXT,
  F_BLACK|B_CYAN,                         // COL_PANELCURSOR,
  F_YELLOW|B_CYAN,                        // COL_PANELSELECTEDCURSOR,
  F_LIGHTCYAN|B_BLUE,                     // COL_PANELTITLE,
  F_BLACK|B_CYAN,                         // COL_PANELSELECTEDTITLE,
  F_YELLOW|B_BLUE,                        // COL_PANELCOLUMNTITLE,
  F_LIGHTCYAN|B_BLUE,                     // COL_PANELTOTALINFO,
  F_YELLOW|B_CYAN,                        // COL_PANELSELECTEDINFO,

  F_BLACK|B_LIGHTGRAY,                    // COL_DIALOGTEXT,
  F_YELLOW|B_LIGHTGRAY,                   // COL_DIALOGHIGHLIGHTTEXT,
  F_BLACK|B_LIGHTGRAY,                    // COL_DIALOGBOX,
  F_BLACK|B_LIGHTGRAY,                    // COL_DIALOGBOXTITLE,
  F_YELLOW|B_LIGHTGRAY,                   // COL_DIALOGHIGHLIGHTBOXTITLE,
  F_BLACK|B_CYAN,                         // COL_DIALOGEDIT,
  F_BLACK|B_LIGHTGRAY,                    // COL_DIALOGBUTTON,
  F_BLACK|B_CYAN,                         // COL_DIALOGSELECTEDBUTTON,
  F_YELLOW|B_LIGHTGRAY,                   // COL_DIALOGHIGHLIGHTBUTTON,
  F_YELLOW|B_CYAN,                        // COL_DIALOGHIGHLIGHTSELECTEDBUTTON,

  F_BLACK|B_LIGHTGRAY,                    // COL_DIALOGMENUTEXT,
  F_WHITE|B_BLACK,                        // COL_DIALOGMENUSELECTEDTEXT,
  F_YELLOW|B_LIGHTGRAY,                   // COL_DIALOGMENUHIGHLIGHT,
  F_YELLOW|B_BLACK,                       // COL_DIALOGMENUSELECTEDHIGHLIGHT,

  F_WHITE|B_RED,                          // COL_WARNDIALOGTEXT,
  F_YELLOW|B_RED,                         // COL_WARNDIALOGHIGHLIGHTTEXT,
  F_WHITE|B_RED,                          // COL_WARNDIALOGBOX,
  F_WHITE|B_RED,                          // COL_WARNDIALOGBOXTITLE,
  F_YELLOW|B_RED,                         // COL_WARNDIALOGHIGHLIGHTBOXTITLE,
  F_BLACK|B_CYAN,                         // COL_WARNDIALOGEDIT,
  F_WHITE|B_RED,                          // COL_WARNDIALOGBUTTON,
  F_BLACK|B_LIGHTGRAY,                    // COL_WARNDIALOGSELECTEDBUTTON,
  F_YELLOW|B_RED,                         // COL_WARNDIALOGHIGHLIGHTBUTTON,
  F_YELLOW|B_LIGHTGRAY,                   // COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON,

  F_LIGHTGRAY|B_BLACK,                    // COL_KEYBARNUM,
  F_BLACK|B_CYAN,                         // COL_KEYBARTEXT,
  F_LIGHTGRAY|B_BLACK,                    // COL_KEYBARBACKGROUND,

  F_LIGHTGRAY|B_BLACK,                    // COL_COMMANDLINE,

  F_BLACK|B_CYAN,                         // COL_CLOCK,

  F_LIGHTCYAN|B_BLUE,                     // COL_VIEWERTEXT,
  F_BLACK|B_CYAN,                         // COL_VIEWERSELECTEDTEXT,
  F_BLACK|B_CYAN,                         // COL_VIEWERSTATUS,

  F_LIGHTCYAN|B_BLUE,                     // COL_EDITORTEXT,
  F_BLACK|B_CYAN,                         // COL_EDITORSELECTEDTEXT,
  F_BLACK|B_CYAN,                         // COL_EDITORSTATUS,

  F_BLACK|B_CYAN,                         // COL_HELPTEXT,
  F_WHITE|B_CYAN,                         // COL_HELPHIGHLIGHTTEXT,
  F_YELLOW|B_CYAN,                        // COL_HELPTOPIC,
  F_WHITE|B_BLACK,                        // COL_HELPSELECTEDTOPIC,
  F_BLACK|B_CYAN,                         // COL_HELPBOX,
  F_BLACK|B_CYAN,                         // COL_HELPBOXTITLE,

  F_YELLOW|B_CYAN,                        // COL_PANELDRAGTEXT,
  F_DARKGRAY|B_CYAN,                      // COL_DIALOGEDITUNCHANGED,
  F_LIGHTCYAN|B_BLUE,                     // COL_PANELSCROLLBAR,
  F_BLACK|B_CYAN,                         // COL_HELPSCROLLBAR,
  F_LIGHTCYAN|B_BLUE,                     // COL_PANELBOX,
  F_LIGHTCYAN|B_BLACK,                    // COL_PANELSCREENSNUMBER,
  F_WHITE|B_BLACK,                        // COL_DIALOGEDITSELECTED,
  F_BLACK|B_LIGHTGRAY,                    // COL_COMMANDLINESELECTED,
  F_YELLOW|B_BLUE,                        // COL_VIEWERARROWS,

  0,                                      // COL_RESERVED0                   // Служебная позиция: 1 - это есть default color

  F_BLACK|B_LIGHTGRAY,                    // COL_DIALOGMENUSCROLLBAR,
  F_WHITE|B_CYAN,                         // COL_MENUSCROLLBAR,
  F_LIGHTCYAN|B_BLUE,                     // COL_VIEWERSCROLLBAR,

  F_LIGHTGRAY|B_BLACK,                    // COL_COMMANDLINEPREFIX,

  F_DARKGRAY|B_LIGHTGRAY,                 // COL_DIALOGDISABLED,
  F_DARKGRAY|B_CYAN,                      // COL_DIALOGEDITDISABLED,
  F_DARKGRAY|B_CYAN,                      // COL_DIALOGLISTDISABLED,

  F_DARKGRAY|B_RED,                       // COL_WARNDIALOGDISABLED,
  F_DARKGRAY|B_CYAN,                      // COL_WARNDIALOGEDITDISABLED,
  F_DARKGRAY|B_CYAN,                      // COL_WARNDIALOGLISTDISABLED,

  F_DARKGRAY|B_CYAN,                      // COL_MENUDISABLEDTEXT,

  F_BLACK|B_CYAN,                         // COL_EDITORCLOCK,
  F_BLACK|B_CYAN,                         // COL_VIEWERCLOCK,

  0,
  0,
  0,
  0,
  0,
};


unsigned char BlackPalette[]=
{
  F_BLACK|B_LIGHTGRAY,                    // COL_MENUTEXT,
  F_LIGHTGRAY|B_BLACK,                    // COL_MENUSELECTEDTEXT,
  F_WHITE|B_LIGHTGRAY,                    // COL_MENUHIGHLIGHT,
  F_WHITE|B_BLACK,                        // COL_MENUSELECTEDHIGHLIGHT,
  F_BLACK|B_LIGHTGRAY,                    // COL_MENUBOX,
  F_BLACK|B_LIGHTGRAY,                    // COL_MENUTITLE,

  F_BLACK|B_LIGHTGRAY,                    // COL_HMENUTEXT,
  F_LIGHTGRAY|B_BLACK,                    // COL_HMENUSELECTEDTEXT,
  F_WHITE|B_LIGHTGRAY,                    // COL_HMENUHIGHLIGHT,
  F_WHITE|B_BLACK,                        // COL_HMENUSELECTEDHIGHLIGHT,

  F_LIGHTGRAY|B_BLACK,                    // COL_PANELTEXT,
  F_WHITE|B_BLACK,                        // COL_PANELSELECTEDTEXT,
  F_LIGHTGRAY|B_BLACK,                    // COL_PANELHIGHLIGHTTEXT,
  F_WHITE|B_BLACK,                        // COL_PANELINFOTEXT,
  F_BLACK|B_LIGHTGRAY,                    // COL_PANELCURSOR,
  F_WHITE|B_LIGHTGRAY,                    // COL_PANELSELECTEDCURSOR,
  F_LIGHTGRAY|B_BLACK,                    // COL_PANELTITLE,
  F_BLACK|B_LIGHTGRAY,                    // COL_PANELSELECTEDTITLE,
  F_WHITE|B_BLACK,                        // COL_PANELCOLUMNTITLE,
  F_LIGHTGRAY|B_BLACK,                    // COL_PANELTOTALINFO,
  F_BLACK|B_LIGHTGRAY,                    // COL_PANELSELECTEDINFO,

  F_BLACK|B_LIGHTGRAY,                    // COL_DIALOGTEXT,
  F_WHITE|B_LIGHTGRAY,                    // COL_DIALOGHIGHLIGHTTEXT,
  F_BLACK|B_LIGHTGRAY,                    // COL_DIALOGBOX,
  F_BLACK|B_LIGHTGRAY,                    // COL_DIALOGBOXTITLE,
  F_WHITE|B_BLACK,                        // COL_DIALOGHIGHLIGHTBOXTITLE,
  F_LIGHTGRAY|B_BLACK,                    // COL_DIALOGEDIT,
  F_BLACK|B_LIGHTGRAY,                    // COL_DIALOGBUTTON,
  F_LIGHTGRAY|B_BLACK,                    // COL_DIALOGSELECTEDBUTTON,
  F_WHITE|B_LIGHTGRAY,                    // COL_DIALOGHIGHLIGHTBUTTON,
  F_WHITE|B_BLACK,                        // COL_DIALOGHIGHLIGHTSELECTEDBUTTON,

  F_BLACK|B_LIGHTGRAY,                    // COL_DIALOGMENUTEXT,
  F_WHITE|B_BLACK,                        // COL_DIALOGMENUSELECTEDTEXT,
  F_WHITE|B_LIGHTGRAY,                    // COL_DIALOGMENUHIGHLIGHT,
  F_WHITE|B_BLACK,                        // COL_DIALOGMENUSELECTEDHIGHLIGHT,

  F_BLACK|B_LIGHTGRAY,                    // COL_WARNDIALOGTEXT,
  F_WHITE|B_LIGHTGRAY,                    // COL_WARNDIALOGHIGHLIGHTTEXT,
  F_BLACK|B_LIGHTGRAY,                    // COL_WARNDIALOGBOX,
  F_BLACK|B_LIGHTGRAY,                    // COL_WARNDIALOGBOXTITLE,
  F_WHITE|B_BLACK,                        // COL_WARNDIALOGHIGHLIGHTBOXTITLE,
  F_LIGHTGRAY|B_BLACK,                    // COL_WARNDIALOGEDIT,
  F_BLACK|B_LIGHTGRAY,                    // COL_WARNDIALOGBUTTON,
  F_LIGHTGRAY|B_BLACK,                    // COL_WARNDIALOGSELECTEDBUTTON,
  F_WHITE|B_LIGHTGRAY,                    // COL_WARNDIALOGHIGHLIGHTBUTTON,
  F_WHITE|B_BLACK,                        // COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON,

  F_LIGHTGRAY|B_BLACK,                    // COL_KEYBARNUM,
  F_BLACK|B_LIGHTGRAY,                    // COL_KEYBARTEXT,
  F_LIGHTGRAY|B_BLACK,                    // COL_KEYBARBACKGROUND,

  F_LIGHTGRAY|B_BLACK,                    // COL_COMMANDLINE,

  F_BLACK|B_LIGHTGRAY,                    // COL_CLOCK,

  F_LIGHTGRAY|B_BLACK,                    // COL_VIEWERTEXT,
  F_BLACK|B_LIGHTGRAY,                    // COL_VIEWERSELECTEDTEXT,
  F_BLACK|B_LIGHTGRAY,                    // COL_VIEWERSTATUS,

  F_LIGHTGRAY|B_BLACK,                    // COL_EDITORTEXT,
  F_BLACK|B_LIGHTGRAY,                    // COL_EDITORSELECTEDTEXT,
  F_BLACK|B_LIGHTGRAY,                    // COL_EDITORSTATUS,

  F_LIGHTGRAY|B_BLACK,                    // COL_HELPTEXT,
  F_LIGHTGRAY|B_BLACK,                    // COL_HELPHIGHLIGHTTEXT,
  F_WHITE|B_BLACK,                        // COL_HELPTOPIC,
  F_BLACK|B_LIGHTGRAY,                    // COL_HELPSELECTEDTOPIC,
  F_LIGHTGRAY|B_BLACK,                    // COL_HELPBOX,
  F_LIGHTGRAY|B_BLACK,                    // COL_HELPBOXTITLE,

  F_BLACK|B_LIGHTGRAY,                    // COL_PANELDRAGTEXT,
  F_WHITE|B_BLACK,                        // COL_DIALOGEDITUNCHANGED,
  F_LIGHTGRAY|B_BLACK,                    // COL_PANELSCROLLBAR,
  F_LIGHTGRAY|B_BLACK,                    // COL_HELPSCROLLBAR,
  F_LIGHTGRAY|B_BLACK,                    // COL_PANELBOX,
  F_WHITE|B_BLACK,                        // COL_PANELSCREENSNUMBER,
  F_BLACK|B_LIGHTGRAY,                    // COL_DIALOGEDITSELECTED,
  F_BLACK|B_LIGHTGRAY,                    // COL_COMMANDLINESELECTED,
  F_WHITE|B_BLACK,                        // COL_VIEWERARROWS,

  1,                                      // COL_RESERVED0                   // Служебная позиция: 1 - это есть default color

  F_BLACK|B_LIGHTGRAY,                    // COL_DIALOGMENUSCROLLBAR,
  F_BLACK|B_LIGHTGRAY,                    // COL_MENUSCROLLBAR,
  F_LIGHTGRAY|B_BLACK,                    // COL_VIEWERSCROLLBAR,

  F_LIGHTGRAY|B_BLACK,                    // COL_COMMANDLINEPREFIX,

  F_LIGHTGRAY|B_BLACK,                    // COL_DIALOGDISABLED,
  F_DARKGRAY|B_LIGHTGRAY,                 // COL_DIALOGEDITDISABLED,
  F_DARKGRAY|B_LIGHTGRAY,                 // COL_DIALOGLISTDISABLED,

  F_DARKGRAY|B_LIGHTGRAY,                 // COL_WARNDIALOGDISABLED,
  F_LIGHTGRAY|B_DARKGRAY,                 // COL_WARNDIALOGEDITDISABLED,
  F_LIGHTGRAY|B_DARKGRAY,                 // COL_WARNDIALOGLISTDISABLED,

  F_DARKGRAY|B_LIGHTGRAY,                 // COL_MENUDISABLEDTEXT,

  F_BLACK|B_LIGHTGRAY,                    // COL_EDITORCLOCK,
  F_BLACK|B_LIGHTGRAY,                    // COL_VIEWERCLOCK,

  0,
  0,
  0,
  0,
  0,
};

int SizeArrayPalette=sizeof(DefaultPalette)/sizeof(DefaultPalette[0]);
unsigned char Palette[sizeof(DefaultPalette)/sizeof(DefaultPalette[0])];

int FarColorToReal(int FarColor)
{
  if (FarColor<COL_FIRSTPALETTECOLOR)
    return(FarColor);
  else
    return(Palette[(FarColor-COL_FIRSTPALETTECOLOR)%SizeArrayPalette]);
}
