/*
palette.cpp

Таблица цветов
*/
/*
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
*/

#include "headers.hpp"
#pragma hdrstop

#include "palette.hpp"
#include "colors.hpp"
#include "colormix.hpp"
#include "config.hpp"
#include "configdb.hpp"

struct ColorsInit
{
	const wchar_t* Name;
	int DefaultIndex;
	int MonoIndex;
}
Init[]=
{
	{L"Menu.Text",                                   F_WHITE|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_MENUTEXT,
	{L"Menu.Text.Selected",                          F_WHITE|B_BLACK,        F_LIGHTGRAY|B_BLACK,   }, // COL_MENUSELECTEDTEXT,
	{L"Menu.Highlight",                              F_YELLOW|B_CYAN,        F_WHITE|B_LIGHTGRAY,   }, // COL_MENUHIGHLIGHT,
	{L"Menu.Highlight.Selected",                     F_YELLOW|B_BLACK,       F_WHITE|B_BLACK,       }, // COL_MENUSELECTEDHIGHLIGHT,
	{L"Menu.Box",                                    F_WHITE|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_MENUBOX,
	{L"Menu.Title",                                  F_WHITE|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_MENUTITLE,
	{L"HMenu.Text",                                  F_BLACK|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_HMENUTEXT,
	{L"HMenu.Text.Selected",                         F_WHITE|B_BLACK,        F_LIGHTGRAY|B_BLACK,   }, // COL_HMENUSELECTEDTEXT,
	{L"HMenu.Highlight",                             F_YELLOW|B_CYAN,        F_WHITE|B_LIGHTGRAY,   }, // COL_HMENUHIGHLIGHT,
	{L"HMenu.Highlight.Selected",                    F_YELLOW|B_BLACK,       F_WHITE|B_BLACK,       }, // COL_HMENUSELECTEDHIGHLIGHT,
	{L"Panel.Text",                                  F_LIGHTCYAN|B_BLUE,     F_LIGHTGRAY|B_BLACK,   }, // COL_PANELTEXT,
	{L"Panel.Text.Selected",                         F_YELLOW|B_BLUE,        F_WHITE|B_BLACK,       }, // COL_PANELSELECTEDTEXT,
	{L"Panel.Text.Highlight",                        F_LIGHTGRAY|B_BLUE,     F_LIGHTGRAY|B_BLACK,   }, // COL_PANELHIGHLIGHTTEXT,
	{L"Panel.Text.Info",                             F_YELLOW|B_BLUE,        F_WHITE|B_BLACK,       }, // COL_PANELINFOTEXT,
	{L"Panel.Cursor",                                F_BLACK|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_PANELCURSOR,
	{L"Panel.Cursor.Selected",                       F_YELLOW|B_CYAN,        F_WHITE|B_LIGHTGRAY,   }, // COL_PANELSELECTEDCURSOR,
	{L"Panel.Title",                                 F_LIGHTCYAN|B_BLUE,     F_LIGHTGRAY|B_BLACK,   }, // COL_PANELTITLE,
	{L"Panel.Title.Selected",                        F_BLACK|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_PANELSELECTEDTITLE,
	{L"Panel.Title.Column",                          F_YELLOW|B_BLUE,        F_WHITE|B_BLACK,       }, // COL_PANELCOLUMNTITLE,
	{L"Panel.Info.Total",                            F_LIGHTCYAN|B_BLUE,     F_LIGHTGRAY|B_BLACK,   }, // COL_PANELTOTALINFO,
	{L"Panel.Info.Selected",                         F_YELLOW|B_CYAN,        F_BLACK|B_LIGHTGRAY,   }, // COL_PANELSELECTEDINFO,
	{L"Dialog.Text",                                 F_BLACK|B_LIGHTGRAY,    F_BLACK|B_LIGHTGRAY,   }, // COL_DIALOGTEXT,
	{L"Dialog.Text.Highlight",                       F_YELLOW|B_LIGHTGRAY,   F_WHITE|B_LIGHTGRAY,   }, // COL_DIALOGHIGHLIGHTTEXT,
	{L"Dialog.Box",                                  F_BLACK|B_LIGHTGRAY,    F_BLACK|B_LIGHTGRAY,   }, // COL_DIALOGBOX,
	{L"Dialog.Box.Title",                            F_BLACK|B_LIGHTGRAY,    F_BLACK|B_LIGHTGRAY,   }, // COL_DIALOGBOXTITLE,
	{L"Dialog.Box.Title.Highlight",                  F_YELLOW|B_LIGHTGRAY,   F_WHITE|B_BLACK,       }, // COL_DIALOGHIGHLIGHTBOXTITLE,
	{L"Dialog.Edit",                                 F_BLACK|B_CYAN,         F_LIGHTGRAY|B_BLACK,   }, // COL_DIALOGEDIT,
	{L"Dialog.Button",                               F_BLACK|B_LIGHTGRAY,    F_BLACK|B_LIGHTGRAY,   }, // COL_DIALOGBUTTON,
	{L"Dialog.Button.Selected",                      F_BLACK|B_CYAN,         F_LIGHTGRAY|B_BLACK,   }, // COL_DIALOGSELECTEDBUTTON,
	{L"Dialog.Button.Highlight",                     F_YELLOW|B_LIGHTGRAY,   F_WHITE|B_LIGHTGRAY,   }, // COL_DIALOGHIGHLIGHTBUTTON,
	{L"Dialog.Button.Highlight.Selected",            F_YELLOW|B_CYAN,        F_WHITE|B_BLACK,       }, // COL_DIALOGHIGHLIGHTSELECTEDBUTTON,
	{L"Dialog.List.Text",                            F_BLACK|B_LIGHTGRAY,    F_BLACK|B_LIGHTGRAY,   }, // COL_DIALOGLISTTEXT,
	{L"Dialog.List.Text.Selected",                   F_WHITE|B_BLACK,        F_WHITE|B_BLACK,       }, // COL_DIALOGLISTSELECTEDTEXT,
	{L"Dialog.List.Highlight",                       F_YELLOW|B_LIGHTGRAY,   F_WHITE|B_LIGHTGRAY,   }, // COL_DIALOGLISTHIGHLIGHT,
	{L"Dialog.List.Highlight.Selected",              F_YELLOW|B_BLACK,       F_WHITE|B_BLACK,       }, // COL_DIALOGLISTSELECTEDHIGHLIGHT,
	{L"WarnDialog.Text",                             F_WHITE|B_RED,          F_BLACK|B_LIGHTGRAY,   }, // COL_WARNDIALOGTEXT,
	{L"WarnDialog.Text.Highlight",                   F_YELLOW|B_RED,         F_WHITE|B_LIGHTGRAY,   }, // COL_WARNDIALOGHIGHLIGHTTEXT,
	{L"WarnDialog.Box",                              F_WHITE|B_RED,          F_BLACK|B_LIGHTGRAY,   }, // COL_WARNDIALOGBOX,
	{L"WarnDialog.Box.Title",                        F_WHITE|B_RED,          F_BLACK|B_LIGHTGRAY,   }, // COL_WARNDIALOGBOXTITLE,
	{L"WarnDialog.Box.Title.Highlight",              F_YELLOW|B_RED,         F_WHITE|B_BLACK,       }, // COL_WARNDIALOGHIGHLIGHTBOXTITLE,
	{L"WarnDialog.Edit",                             F_BLACK|B_CYAN,         F_LIGHTGRAY|B_BLACK,   }, // COL_WARNDIALOGEDIT,
	{L"WarnDialog.Button",                           F_WHITE|B_RED,          F_BLACK|B_LIGHTGRAY,   }, // COL_WARNDIALOGBUTTON,
	{L"WarnDialog.Button.Selected",                  F_BLACK|B_LIGHTGRAY,    F_LIGHTGRAY|B_BLACK,   }, // COL_WARNDIALOGSELECTEDBUTTON,
	{L"WarnDialog.Button.Highlight",                 F_YELLOW|B_RED,         F_WHITE|B_LIGHTGRAY,   }, // COL_WARNDIALOGHIGHLIGHTBUTTON,
	{L"WarnDialog.Button.Highlight.Selected",        F_YELLOW|B_LIGHTGRAY,   F_WHITE|B_BLACK,       }, // COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON,
	{L"Keybar.Num",                                  F_LIGHTGRAY|B_BLACK,    F_LIGHTGRAY|B_BLACK,   }, // COL_KEYBARNUM,
	{L"Keybar.Text",                                 F_BLACK|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_KEYBARTEXT,
	{L"Keybar.Background",                           F_LIGHTGRAY|B_BLACK,    F_LIGHTGRAY|B_BLACK,   }, // COL_KEYBARBACKGROUND,
	{L"CommandLine",                                 F_LIGHTGRAY|B_BLACK,    F_LIGHTGRAY|B_BLACK,   }, // COL_COMMANDLINE,
	{L"Clock",                                       F_BLACK|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_CLOCK,
	{L"Viewer.Text",                                 F_LIGHTCYAN|B_BLUE,     F_LIGHTGRAY|B_BLACK,   }, // COL_VIEWERTEXT,
	{L"Viewer.Text.Selected",                        F_BLACK|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_VIEWERSELECTEDTEXT,
	{L"Viewer.Status",                               F_BLACK|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_VIEWERSTATUS,
	{L"Editor.Text",                                 F_LIGHTCYAN|B_BLUE,     F_LIGHTGRAY|B_BLACK,   }, // COL_EDITORTEXT,
	{L"Editor.Text.Selected",                        F_BLACK|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_EDITORSELECTEDTEXT,
	{L"Editor.Status",                               F_BLACK|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_EDITORSTATUS,
	{L"Help.Text",                                   F_BLACK|B_CYAN,         F_LIGHTGRAY|B_BLACK,   }, // COL_HELPTEXT,
	{L"Help.Text.Highlight",                         F_WHITE|B_CYAN,         F_LIGHTGRAY|B_BLACK,   }, // COL_HELPHIGHLIGHTTEXT,
	{L"Help.Topic",                                  F_YELLOW|B_CYAN,        F_WHITE|B_BLACK,       }, // COL_HELPTOPIC,
	{L"Help.Topic.Selected",                         F_WHITE|B_BLACK,        F_BLACK|B_LIGHTGRAY,   }, // COL_HELPSELECTEDTOPIC,
	{L"Help.Box",                                    F_BLACK|B_CYAN,         F_LIGHTGRAY|B_BLACK,   }, // COL_HELPBOX,
	{L"Help.Box.Title",                              F_BLACK|B_CYAN,         F_LIGHTGRAY|B_BLACK,   }, // COL_HELPBOXTITLE,
	{L"Panel.DragText",                              F_YELLOW|B_CYAN,        F_BLACK|B_LIGHTGRAY,   }, // COL_PANELDRAGTEXT,
	{L"Dialog.Edit.Unchanged",                       F_LIGHTGRAY|B_CYAN,     F_WHITE|B_BLACK,       },  // COL_DIALOGEDITUNCHANGED,
	{L"Panel.Scrollbar",                             F_LIGHTCYAN|B_BLUE,     F_LIGHTGRAY|B_BLACK,   }, // COL_PANELSCROLLBAR,
	{L"Help.Scrollbar",                              F_BLACK|B_CYAN,         F_LIGHTGRAY|B_BLACK,   }, // COL_HELPSCROLLBAR,
	{L"Panel.Box",                                   F_LIGHTCYAN|B_BLUE,     F_LIGHTGRAY|B_BLACK,   }, // COL_PANELBOX,
	{L"Panel.ScreensNumber",                         F_LIGHTCYAN|B_BLACK,    F_WHITE|B_BLACK,       }, // COL_PANELSCREENSNUMBER,
	{L"Dialog.Edit.Selected",                        F_WHITE|B_BLACK,        F_BLACK|B_LIGHTGRAY,   }, // COL_DIALOGEDITSELECTED,
	{L"CommandLine.Selected",                        F_BLACK|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_COMMANDLINESELECTED,
	{L"Viewer.Arrows",                               F_YELLOW|B_BLUE,        F_WHITE|B_BLACK,       }, // COL_VIEWERARROWS,
	{L"Dialog.List.Scrollbar",                       F_BLACK|B_LIGHTGRAY,    F_BLACK|B_LIGHTGRAY,   }, // COL_DIALOGLISTSCROLLBAR,
	{L"Menu.Scrollbar",                              F_WHITE|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_MENUSCROLLBAR,
	{L"Viewer.Scrollbar",                            F_LIGHTCYAN|B_BLUE,     F_LIGHTGRAY|B_BLACK,   }, // COL_VIEWERSCROLLBAR,
	{L"CommandLine.Prefix",                          F_LIGHTGRAY|B_BLACK,    F_LIGHTGRAY|B_BLACK,   }, // COL_COMMANDLINEPREFIX,
	{L"Dialog.Disabled",                             F_DARKGRAY|B_LIGHTGRAY, F_LIGHTGRAY|B_BLACK,   }, // COL_DIALOGDISABLED,
	{L"Dialog.Edit.Disabled",                        F_DARKGRAY|B_CYAN,      F_DARKGRAY|B_LIGHTGRAY,}, // COL_DIALOGEDITDISABLED,
	{L"Dialog.List.Disabled",                        F_DARKGRAY|B_LIGHTGRAY, F_DARKGRAY|B_LIGHTGRAY,}, // COL_DIALOGLISTDISABLED,
	{L"WarnDialog.Disabled",                         F_DARKGRAY|B_RED,       F_DARKGRAY|B_LIGHTGRAY,}, // COL_WARNDIALOGDISABLED,
	{L"WarnDialog.Edit.Disabled",                    F_DARKGRAY|B_CYAN,      F_DARKGRAY|B_LIGHTGRAY,}, // COL_WARNDIALOGEDITDISABLED,
	{L"WarnDialog.List.Disabled",                    F_DARKGRAY|B_RED,       F_DARKGRAY|B_LIGHTGRAY,}, // COL_WARNDIALOGLISTDISABLED,
	{L"Menu.Text.Disabled",                          F_DARKGRAY|B_CYAN,      F_DARKGRAY|B_LIGHTGRAY,}, // COL_MENUDISABLEDTEXT,
	{L"Editor.Clock",                                F_BLACK|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_EDITORCLOCK,
	{L"Viewer.Clock",                                F_BLACK|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_VIEWERCLOCK,
	{L"Dialog.List.Title",                           F_BLACK|B_LIGHTGRAY,    F_BLACK|B_LIGHTGRAY,   }, // COL_DIALOGLISTTITLE
	{L"Dialog.List.Box",                             F_BLACK|B_LIGHTGRAY,    F_BLACK|B_LIGHTGRAY,   }, // COL_DIALOGLISTBOX,
	{L"WarnDialog.Edit.Selected",                    F_WHITE|B_BLACK,        F_BLACK|B_WHITE,       }, // COL_WARNDIALOGEDITSELECTED,
	{L"WarnDialog.Edit.Unchanged",                   F_LIGHTGRAY|B_CYAN,     F_WHITE|B_BLACK,       }, // COL_WARNDIALOGEDITUNCHANGED,
	{L"Dialog.Combo.Text",                           F_WHITE|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_DIALOGCBOXTEXT,
	{L"Dialog.Combo.Text.Selected",                  F_WHITE|B_BLACK,        F_LIGHTGRAY|B_BLACK,   }, // COL_DIALOGCBOXSELECTEDTEXT,
	{L"Dialog.Combo.Highlight",                      F_YELLOW|B_CYAN,        F_WHITE|B_LIGHTGRAY,   }, // COL_DIALOGCBOXHIGHLIGHT,
	{L"Dialog.Combo.Highlight.Selected",             F_YELLOW|B_BLACK,       F_WHITE|B_BLACK,       }, // COL_DIALOGCBOXSELECTEDHIGHLIGHT,
	{L"Dialog.Combo.Box",                            F_WHITE|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_DIALOGCBOXBOX,
	{L"Dialog.Combo.Title",                          F_WHITE|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_DIALOGCBOXTITLE,
	{L"Dialog.Combo.Disabled",                       F_DARKGRAY|B_CYAN,      F_DARKGRAY|B_LIGHTGRAY,}, // COL_DIALOGCBOXDISABLED,
	{L"Dialog.Combo.Scrollbar",                      F_WHITE|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_DIALOGCBOXSCROLLBAR,
	{L"WarnDialog.List.Text",                        F_WHITE|B_RED,          F_BLACK|B_LIGHTGRAY,   }, // COL_WARNDIALOGLISTTEXT,
	{L"WarnDialog.List.Text.Selected",               F_BLACK|B_LIGHTGRAY,    F_WHITE|B_BLACK,       }, // COL_WARNDIALOGLISTSELECTEDTEXT,
	{L"WarnDialog.List.Highlight",                   F_YELLOW|B_RED,         F_WHITE|B_LIGHTGRAY,   }, // COL_WARNDIALOGLISTHIGHLIGHT,
	{L"WarnDialog.List.Highlight.Selected",          F_YELLOW|B_LIGHTGRAY,   F_WHITE|B_BLACK,       }, // COL_WARNDIALOGLISTSELECTEDHIGHLIGHT,
	{L"WarnDialog.List.Box",                         F_WHITE|B_RED,          F_BLACK|B_LIGHTGRAY,   }, // COL_WARNDIALOGLISTBOX,
	{L"WarnDialog.List.Title",                       F_WHITE|B_RED,          F_BLACK|B_LIGHTGRAY,   }, // COL_WARNDIALOGLISTTITLE,
	{L"WarnDialog.List.Scrollbar",                   F_WHITE|B_RED,          F_BLACK|B_LIGHTGRAY,   }, // COL_WARNDIALOGLISTSCROLLBAR,
	{L"WarnDialog.Combo.Text",                       F_WHITE|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_WARNDIALOGCBOXTEXT,
	{L"WarnDialog.Combo.Text.Selected",              F_WHITE|B_BLACK,        F_WHITE|B_BLACK,       }, // COL_WARNDIALOGCBOXSELECTEDTEXT,
	{L"WarnDialog.Combo.Highlight",                  F_YELLOW|B_CYAN,        F_WHITE|B_LIGHTGRAY,   }, // COL_WARNDIALOGCBOXHIGHLIGHT,
	{L"WarnDialog.Combo.Highlight.Selected",         F_YELLOW|B_BLACK,       F_WHITE|B_BLACK,       }, // COL_WARNDIALOGCBOXSELECTEDHIGHLIGHT,
	{L"WarnDialog.Combo.Box",                        F_WHITE|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_WARNDIALOGCBOXBOX,
	{L"WarnDialog.Combo.Title",                      F_WHITE|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_WARNDIALOGCBOXTITLE,
	{L"WarnDialog.Combo.Disabled",                   F_DARKGRAY|B_CYAN,      F_DARKGRAY|B_LIGHTGRAY,}, // COL_WARNDIALOGCBOXDISABLED,
	{L"WarnDialog.Combo.Scrollbar",                  F_WHITE|B_CYAN,         F_BLACK|B_LIGHTGRAY,   }, // COL_WARNDIALOGCBOXSCROLLBAR,
	{L"Dialog.List.Arrows",                          F_YELLOW|B_LIGHTGRAY,   F_WHITE|B_LIGHTGRAY,   }, // COL_DIALOGLISTARROWS,
	{L"Dialog.List.Arrows.Disabled",                 F_DARKGRAY|B_LIGHTGRAY, F_DARKGRAY|B_LIGHTGRAY,}, // COL_DIALOGLISTARROWSDISABLED,
	{L"Dialog.List.Arrows.Selected",                 F_YELLOW|B_BLACK,       F_WHITE|B_BLACK,       }, // COL_DIALOGLISTARROWSSELECTED,
	{L"Dialog.Combo.Arrows",                         F_YELLOW|B_CYAN,        F_WHITE|B_LIGHTGRAY,   }, // COL_DIALOGCOMBOARROWS,
	{L"Dialog.Combo.Arrows.Disabled",                F_DARKGRAY|B_CYAN,      F_DARKGRAY|B_LIGHTGRAY,}, // COL_DIALOGCOMBOARROWSDISABLED,
	{L"Dialog.Combo.Arrows.Selected",                F_YELLOW|B_BLACK,       F_WHITE|B_BLACK,       }, // COL_DIALOGCOMBOARROWSSELECTED,
	{L"WarnDialog.List.Arrows",                      F_YELLOW|B_RED,         F_WHITE|B_LIGHTGRAY,   }, // COL_WARNDIALOGLISTARROWS,
	{L"WarnDialog.List.Arrows.Disabled",             F_DARKGRAY|B_RED,       F_DARKGRAY|B_LIGHTGRAY,}, // COL_WARNDIALOGLISTARROWSDISABLED,
	{L"WarnDialog.List.Arrows.Selected",             F_YELLOW|B_LIGHTGRAY,   F_WHITE|B_BLACK,       }, // COL_WARNDIALOGLISTARROWSSELECTED,
	{L"WarnDialog.Combo.Arrows",                     F_YELLOW|B_CYAN,        F_WHITE|B_LIGHTGRAY,   }, // COL_WARNDIALOGCOMBOARROWS,
	{L"WarnDialog.Combo.Arrows.Disabled",            F_DARKGRAY|B_CYAN,      F_DARKGRAY|B_LIGHTGRAY,}, // COL_WARNDIALOGCOMBOARROWSDISABLED,
	{L"WarnDialog.Combo.Arrows.Selected",            F_YELLOW|B_BLACK,       F_WHITE|B_BLACK,       }, // COL_WARNDIALOGCOMBOARROWSSELECTED,
	{L"Menu.Arrows",                                 F_YELLOW|B_CYAN,        F_WHITE|B_LIGHTGRAY,   }, // COL_MENUARROWS,
	{L"Menu.Arrows.Disabled",                        F_DARKGRAY|B_CYAN,      F_DARKGRAY|B_LIGHTGRAY,}, // COL_MENUARROWSDISABLED,
	{L"Menu.Arrows.Selected",                        F_YELLOW|B_BLACK,       F_WHITE|B_BLACK,       }, // COL_MENUARROWSSELECTED,
	{L"CommandLine.UserScreen",                      F_LIGHTGRAY|B_BLACK,    F_LIGHTGRAY|B_BLACK,   }, // COL_COMMANDLINEUSERSCREEN,
	{L"Editor.Scrollbar",                            F_LIGHTCYAN|B_BLUE,     F_LIGHTGRAY|B_BLACK,   }, // COL_EDITORSCROLLBAR,
	{L"Menu.GrayText",                               F_DARKGRAY|B_CYAN,      F_DARKGRAY|B_LIGHTGRAY,}, // COL_MENUGRAYTEXT,
	{L"Menu.GrayText.Selected",                      F_LIGHTGRAY|B_BLACK,    F_LIGHTGRAY|B_BLACK,   }, // COL_MENUSELECTEDGRAYTEXT,
	{L"Dialog.Combo.GrayText",                       F_DARKGRAY|B_CYAN,      F_DARKGRAY|B_LIGHTGRAY,}, // COL_DIALOGCOMBOGRAY,
	{L"Dialog.Combo.GrayText.Selected",              F_LIGHTGRAY|B_BLACK,    F_LIGHTGRAY|B_BLACK,   }, // COL_DIALOGCOMBOSELECTEDGRAYTEXT,
	{L"Dialog.List.GrayText",                        F_DARKGRAY|B_LIGHTGRAY, F_DARKGRAY|B_LIGHTGRAY,}, // COL_DIALOGLISTGRAY,
	{L"Dialog.List.GrayText.Selected",               F_LIGHTGRAY|B_BLACK,    F_WHITE|B_BLACK,       }, // COL_DIALOGLISTSELECTEDGRAYTEXT,
	{L"WarnDialog.Combo.GrayText",                   F_DARKGRAY|B_CYAN,      F_DARKGRAY|B_LIGHTGRAY,}, // COL_WARNDIALOGCOMBOGRAY,
	{L"WarnDialog.Combo.GrayText.Selected",          F_LIGHTGRAY|B_BLACK,    F_WHITE|B_BLACK,       }, // COL_WARNDIALOGCOMBOSELECTEDGRAYTEXT,
	{L"WarnDialog.List.GrayText",                    F_DARKGRAY|B_RED,       F_DARKGRAY|B_LIGHTGRAY,}, // COL_WARNDIALOGLISTGRAY,
	{L"WarnDialog.List.GrayText.Selected",           F_BLACK|B_LIGHTGRAY,    F_WHITE|B_BLACK,       }, // COL_WARNDIALOGLISTSELECTEDGRAYTEXT,
	{L"Dialog.DefaultButton",                        F_BLACK|B_LIGHTGRAY,    F_BLACK|B_LIGHTGRAY,   }, // COL_DIALOGDEFAULTBUTTON,
	{L"Dialog.DefaultButton.Selected",               F_BLACK|B_CYAN,         F_LIGHTGRAY|B_BLACK,   }, // COL_DIALOGSELECTEDDEFAULTBUTTON,
	{L"Dialog.DefaultButton.Highlight",              F_YELLOW|B_LIGHTGRAY,   F_WHITE|B_LIGHTGRAY,   }, // COL_DIALOGHIGHLIGHTDEFAULTBUTTON,
	{L"Dialog.DefaultButton.Highlight.Selected",     F_YELLOW|B_CYAN,        F_WHITE|B_BLACK,       }, // COL_DIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON,
	{L"WarnDialog.DefaultButton",                    F_WHITE|B_RED,          F_BLACK|B_LIGHTGRAY,   }, // COL_WARNDIALOGDEFAULTBUTTON,
	{L"WarnDialog.DefaultButton.Selected",           F_BLACK|B_LIGHTGRAY,    F_LIGHTGRAY|B_BLACK,   }, // COL_WARNDIALOGSELECTEDDEFAULTBUTTON,
	{L"WarnDialog.DefaultButton.Highlight",          F_YELLOW|B_RED,         F_WHITE|B_LIGHTGRAY,   }, // COL_WARNDIALOGHIGHLIGHTDEFAULTBUTTON,
	{L"WarnDialog.DefaultButton.Highlight.Selected", F_YELLOW|B_LIGHTGRAY,   F_WHITE|B_BLACK,       }, // COL_WARNDIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON,
};

FarColor CurrentPaletteData[ARRAYSIZE(Init)];
FarColor DefaultPaletteData[ARRAYSIZE(Init)];
FarColor BlackPaletteData[ARRAYSIZE(Init)];

palette::palette():
	SizeArrayPalette(ARRAYSIZE(Init)),
	CurrentPalette(CurrentPaletteData),
	DefaultPalette(DefaultPaletteData),
	BlackPalette(BlackPaletteData),
	PaletteChanged(false)
{
	for(size_t i = 0; i < SizeArrayPalette; ++i)
	{
		Colors::ConsoleColorToFarColor(Init[i].DefaultIndex, DefaultPalette[i]);
		Colors::ConsoleColorToFarColor(Init[i].MonoIndex, BlackPalette[i]);
	}
	MAKE_TRANSPARENT(DefaultPalette[COL_PANELTEXT-COL_FIRSTPALETTECOLOR].BackgroundColor);
	MAKE_TRANSPARENT(DefaultPalette[COL_PANELSELECTEDTEXT-COL_FIRSTPALETTECOLOR].BackgroundColor);
	memcpy(CurrentPalette, DefaultPalette, SizeArrayPalette*sizeof(FarColor));
}

void palette::ResetToDefault()
{
	memcpy(CurrentPalette, DefaultPalette, SizeArrayPalette*sizeof(FarColor));
	PaletteChanged = true;
}

void palette::ResetToBlack()
{
	memcpy(CurrentPalette, BlackPalette, SizeArrayPalette*sizeof(FarColor));
	PaletteChanged = true;
}

const FarColor ColorIndexToColor(PaletteColors ColorIndex)
{
	FarColor Result = {};
	if(ColorIndex<COL_FIRSTPALETTECOLOR)
	{
		Colors::ConsoleColorToFarColor(ColorIndex, Result);
	}
	else
	{
		Result = Opt.Palette.CurrentPalette[(ColorIndex-COL_FIRSTPALETTECOLOR)%Opt.Palette.SizeArrayPalette];
	}
	return Result;
}

void palette::Load()
{
	for (size_t i = 0; i < SizeArrayPalette; ++i)
	{
		ColorsCfg->GetValue(Init[i].Name, CurrentPalette[i]);
	}
	PaletteChanged = false;
}


void palette::Save()
{
	if (PaletteChanged)
	{
		ColorsCfg->BeginTransaction();
		for (size_t i = 0; i < SizeArrayPalette; ++i)
		{
			ColorsCfg->SetValue(Init[i].Name, CurrentPalette[i]);
		}
		ColorsCfg->EndTransaction();
	}
	PaletteChanged = false;
}
