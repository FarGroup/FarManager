/*
setcolor.cpp

Установка фаровских цветов
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

#include "setcolor.hpp"
#include "keys.hpp"
#include "colors.hpp"
#include "vmenu2.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "ctrlobj.hpp"
#include "savescr.hpp"
#include "scrbuf.hpp"
#include "panel.hpp"
#include "chgmmode.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "console.hpp"
#include "colormix.hpp"
#include "language.hpp"
#include "keyboard.hpp"
#include "manager.hpp"

static void SetItemColors(const LNGID* ItemIds, const int* PaletteItems, size_t Size, int TypeSub = 0);

void GetColor(int PaletteIndex)
{
	SCOPED_ACTION(ChangeMacroMode)(MACROAREA_MENU);
	FarColor NewColor = Global->Opt->Palette[PaletteIndex-COL_FIRSTPALETTECOLOR];

	if (Console().GetColorDialog(NewColor))
	{
		Global->Opt->Palette.Set(PaletteIndex-COL_FIRSTPALETTECOLOR, &NewColor, 1);
		Global->ScrBuf->Lock(); // отменяем всякую прорисовку
		Global->CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->LeftPanel->Redraw();
		Global->CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->RightPanel->Redraw();


		Global->FrameManager->ResizeAllFrame(); // рефрешим
		Global->FrameManager->PluginCommit(); // коммитим.

		if (Global->Opt->Clock)
			ShowTime(1);

		Global->ScrBuf->Unlock(); // разрешаем прорисовку
		Global->FrameManager->PluginCommit(); // коммитим.
	}
}

#define CheckSize(a, b) static_assert(ARRAYSIZE(a) == ARRAYSIZE(b), "wrong array size");

enum list_mode
{
	lm_list_normal,
	lm_list_warning,
	lm_combo_normal,
	lm_combo_warning,

	list_modes_count
};

void SetColors()
{
	static const LNGID PanelItems[] =
	{
		MSetColorPanelNormal,
		MSetColorPanelSelected,
		MSetColorPanelHighlightedText,
		MSetColorPanelHighlightedInfo,
		MSetColorPanelDragging,
		MSetColorPanelBox,
		MSetColorPanelNormalCursor,
		MSetColorPanelSelectedCursor,
		MSetColorPanelNormalTitle,
		MSetColorPanelSelectedTitle,
		MSetColorPanelColumnTitle,
		MSetColorPanelTotalInfo,
		MSetColorPanelSelectedInfo,
		MSetColorPanelScrollbar,
		MSetColorPanelScreensNumber,
	};
	static const int PanelPaletteItems[]=
	{
		COL_PANELTEXT,
		COL_PANELSELECTEDTEXT,
		COL_PANELHIGHLIGHTTEXT,
		COL_PANELINFOTEXT,
		COL_PANELDRAGTEXT,
		COL_PANELBOX,
		COL_PANELCURSOR,
		COL_PANELSELECTEDCURSOR,
		COL_PANELTITLE,
		COL_PANELSELECTEDTITLE,
		COL_PANELCOLUMNTITLE,
		COL_PANELTOTALINFO,
		COL_PANELSELECTEDINFO,
		COL_PANELSCROLLBAR,
		COL_PANELSCREENSNUMBER,
	};
	CheckSize(PanelItems, PanelPaletteItems);

	static const LNGID DialogItems[] =
	{
		MSetColorDialogNormal,
		MSetColorDialogHighlighted,
		MSetColorDialogDisabled,
		MSetColorDialogBox,
		MSetColorDialogBoxTitle,
		MSetColorDialogHighlightedBoxTitle,
		MSetColorDialogTextInput,
		MSetColorDialogUnchangedTextInput,
		MSetColorDialogSelectedTextInput,
		MSetColorDialogEditDisabled,
		MSetColorDialogButtons,
		MSetColorDialogSelectedButtons,
		MSetColorDialogHighlightedButtons,
		MSetColorDialogSelectedHighlightedButtons,
		MSetColorDialogDefaultButton,
		MSetColorDialogSelectedDefaultButton,
		MSetColorDialogHighlightedDefaultButton,
		MSetColorDialogSelectedHighlightedDefaultButton,
		MSetColorDialogListBoxControl,
		MSetColorDialogComboBoxControl,
	};
	static const int DialogPaletteItems[] =
	{
		COL_DIALOGTEXT,
		COL_DIALOGHIGHLIGHTTEXT,
		COL_DIALOGDISABLED,
		COL_DIALOGBOX,
		COL_DIALOGBOXTITLE,
		COL_DIALOGHIGHLIGHTBOXTITLE,
		COL_DIALOGEDIT,
		COL_DIALOGEDITUNCHANGED,
		COL_DIALOGEDITSELECTED,
		COL_DIALOGEDITDISABLED,
		COL_DIALOGBUTTON,
		COL_DIALOGSELECTEDBUTTON,
		COL_DIALOGHIGHLIGHTBUTTON,
		COL_DIALOGHIGHLIGHTSELECTEDBUTTON,
		COL_DIALOGDEFAULTBUTTON,
		COL_DIALOGSELECTEDDEFAULTBUTTON,
		COL_DIALOGHIGHLIGHTDEFAULTBUTTON,
		COL_DIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON,
		lm_list_normal,
		lm_combo_normal,
	};
	CheckSize(DialogItems, DialogPaletteItems);
	static const int WarnDialogPaletteItems[] =
	{
		COL_WARNDIALOGTEXT,
		COL_WARNDIALOGHIGHLIGHTTEXT,
		COL_WARNDIALOGDISABLED,
		COL_WARNDIALOGBOX,
		COL_WARNDIALOGBOXTITLE,
		COL_WARNDIALOGHIGHLIGHTBOXTITLE,
		COL_WARNDIALOGEDIT,
		COL_WARNDIALOGEDITUNCHANGED,
		COL_WARNDIALOGEDITSELECTED,
		COL_WARNDIALOGEDITDISABLED,
		COL_WARNDIALOGBUTTON,
		COL_WARNDIALOGSELECTEDBUTTON,
		COL_WARNDIALOGHIGHLIGHTBUTTON,
		COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON,
		COL_WARNDIALOGDEFAULTBUTTON,
		COL_WARNDIALOGSELECTEDDEFAULTBUTTON,
		COL_WARNDIALOGHIGHLIGHTDEFAULTBUTTON,
		COL_WARNDIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON,
		lm_list_warning,
		lm_combo_warning,
	};
	CheckSize(DialogItems, WarnDialogPaletteItems);

	static const LNGID MenuItems[] =
	{
		MSetColorMenuNormal,
		MSetColorMenuSelected,
		MSetColorMenuHighlighted,
		MSetColorMenuSelectedHighlighted,
		MSetColorMenuDisabled,
		MSetColorMenuBox,
		MSetColorMenuTitle,
		MSetColorMenuScrollBar,
		MSetColorMenuArrows,
		MSetColorMenuArrowsSelected,
		MSetColorMenuArrowsDisabled,
		MSetColorMenuGrayed,
		MSetColorMenuSelectedGrayed,
	};
	static const int MenuPaletteItems[] =
	{
		COL_MENUTEXT,COL_MENUSELECTEDTEXT,COL_MENUHIGHLIGHT,
		COL_MENUSELECTEDHIGHLIGHT,COL_MENUDISABLEDTEXT,
		COL_MENUBOX,COL_MENUTITLE,COL_MENUSCROLLBAR,
		COL_MENUARROWS,                             // Arrow
		COL_MENUARROWSSELECTED,                     // Выбранный - Arrow
		COL_MENUARROWSDISABLED,
		COL_MENUGRAYTEXT,                          // "серый"
		COL_MENUSELECTEDGRAYTEXT,                  // выбранный "серый"
	};
	CheckSize(MenuItems, MenuPaletteItems);

	static const LNGID HMenuItems[] =
	{
		MSetColorHMenuNormal,
		MSetColorHMenuSelected,
		MSetColorHMenuHighlighted,
		MSetColorHMenuSelectedHighlighted,
	};
	static const int HMenuPaletteItems[] =
	{
		COL_HMENUTEXT,
		COL_HMENUSELECTEDTEXT,
		COL_HMENUHIGHLIGHT,
		COL_HMENUSELECTEDHIGHLIGHT,
	};
	CheckSize(HMenuItems, HMenuPaletteItems);

	static const LNGID KeyBarItems[] =
	{
		MSetColorKeyBarNumbers,
		MSetColorKeyBarNames,
		MSetColorKeyBarBackground,
	};
	static const int KeyBarPaletteItems[] =
	{
		COL_KEYBARNUM,
		COL_KEYBARTEXT,
		COL_KEYBARBACKGROUND,
	};
	CheckSize(KeyBarItems, KeyBarPaletteItems);

	static const LNGID CommandLineItems[] =
	{
		MSetColorCommandLineNormal,
		MSetColorCommandLineSelected,
		MSetColorCommandLinePrefix,
		MSetColorCommandLineUserScreen,
	};
	static const int CommandLinePaletteItems[] =
	{
		COL_COMMANDLINE,
		COL_COMMANDLINESELECTED,
		COL_COMMANDLINEPREFIX,
		COL_COMMANDLINEUSERSCREEN,
	};
	CheckSize(CommandLineItems, CommandLinePaletteItems);

	static const LNGID ClockItems[] =
	{
		MSetColorClockNormal,
		MSetColorClockNormalEditor,
		MSetColorClockNormalViewer,
	};
	static const int ClockPaletteItems[] =
	{
		COL_CLOCK,
		COL_EDITORCLOCK,
		COL_VIEWERCLOCK,
	};
	CheckSize(ClockItems, ClockPaletteItems);

	static const LNGID ViewerItems[] =
	{
		MSetColorViewerNormal,
		MSetColorViewerSelected,
		MSetColorViewerStatus,
		MSetColorViewerArrows,
		MSetColorViewerScrollbar,
	};
	static const int ViewerPaletteItems[] =
	{
		COL_VIEWERTEXT,
		COL_VIEWERSELECTEDTEXT,
		COL_VIEWERSTATUS,
		COL_VIEWERARROWS,
		COL_VIEWERSCROLLBAR,
	};
	CheckSize(ViewerItems, ViewerPaletteItems);

	static const LNGID EditorItems[] =
	{
		MSetColorEditorNormal,
		MSetColorEditorSelected,
		MSetColorEditorStatus,
		MSetColorEditorScrollbar,
	};
	static const int EditorPaletteItems[] =
	{
		COL_EDITORTEXT,
		COL_EDITORSELECTEDTEXT,
		COL_EDITORSTATUS,
		COL_EDITORSCROLLBAR,
	};
	CheckSize(EditorItems, EditorPaletteItems);

	static const LNGID HelpItems[] =
	{
		MSetColorHelpNormal,
		MSetColorHelpHighlighted,
		MSetColorHelpReference,
		MSetColorHelpSelectedReference,
		MSetColorHelpBox,
		MSetColorHelpBoxTitle,
		MSetColorHelpScrollbar,
	};
	static const int HelpPaletteItems[] =
	{
		COL_HELPTEXT,
		COL_HELPHIGHLIGHTTEXT,
		COL_HELPTOPIC,
		COL_HELPSELECTEDTOPIC,
		COL_HELPBOX,
		COL_HELPBOXTITLE,
		COL_HELPSCROLLBAR,
	};
	CheckSize(HelpItems, HelpPaletteItems);

	{
		static const struct group_item
		{
			LNGID MenuId;
			const LNGID* SubiemIds;
			const int* SubitemPalette;
			size_t SubItemsSize;
			int TypeSub;
		}
		Groups[] =
		{
			{ MSetColorPanel, PanelItems, PanelPaletteItems, ARRAYSIZE(PanelItems) },
			{ MSetColorDialog, DialogItems, DialogPaletteItems, ARRAYSIZE(DialogItems), 1 },
			{ MSetColorWarning, DialogItems, WarnDialogPaletteItems, ARRAYSIZE(DialogItems), 1 },
			{ MSetColorMenu, MenuItems, MenuPaletteItems, ARRAYSIZE(MenuItems) },
			{ MSetColorHMenu, HMenuItems, HMenuPaletteItems, ARRAYSIZE(HMenuItems) },
			{ MSetColorKeyBar, KeyBarItems, KeyBarPaletteItems, ARRAYSIZE(KeyBarItems) },
			{ MSetColorCommandLine, CommandLineItems, CommandLinePaletteItems, ARRAYSIZE(CommandLineItems) },
			{ MSetColorClock, ClockItems, ClockPaletteItems, ARRAYSIZE(ClockItems) },
			{ MSetColorViewer, ViewerItems, ViewerPaletteItems, ARRAYSIZE(ViewerItems) },
			{ MSetColorEditor, EditorItems, EditorPaletteItems, ARRAYSIZE(EditorItems) },
			{ MSetColorHelp, HelpItems, HelpPaletteItems, ARRAYSIZE(HelpItems) },
		};

		VMenu2 GroupsMenu(MSG(MSetColorGroupsTitle), nullptr, 0);

		FOR(const auto& i, Groups)
		{
			GroupsMenu.AddItem(MSG(i.MenuId));
		}

		{
			MenuItemEx tmp;
			tmp.Flags = LIF_SEPARATOR;
			GroupsMenu.AddItem(tmp);
		}

		const int DefaultId = GroupsMenu.GetItemCount();
		GroupsMenu.AddItem(MSG(MSetDefaultColors));
		const int BlackWhiteId = GroupsMenu.GetItemCount();
		GroupsMenu.AddItem(MSG(MSetBW));

		GroupsMenu.SetPosition(2,1,0,0);
		GroupsMenu.SetFlags(VMENU_WRAPMODE);
		int GroupsCode=GroupsMenu.RunEx([&](int Msg, void *param)->int
		{
			intptr_t ItemsCode=(intptr_t)param;
			if (Msg != DN_CLOSE || ItemsCode < 0 || ItemsCode >= ARRAYSIZE(Groups))
				return 0;
			GroupsMenu.SendMessage(DM_ENABLEREDRAW, 1, nullptr);
			SetItemColors(Groups[ItemsCode].SubiemIds, Groups[ItemsCode].SubitemPalette, Groups[ItemsCode].SubItemsSize, Groups[ItemsCode].TypeSub);
			return 1;
		});

		if (GroupsCode == DefaultId)
		{
			Global->Opt->Palette.ResetToDefault();
		}
		else if (GroupsCode == BlackWhiteId)
		{
			Global->Opt->Palette.ResetToBlack();
		}
	}
	Global->CtrlObject->Cp()->SetScreenPosition();
	Global->CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
	Global->CtrlObject->Cp()->LeftPanel->Redraw();
	Global->CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
	Global->CtrlObject->Cp()->RightPanel->Redraw();
}


static void SetItemColors(const LNGID* ItemIds, const int* PaletteItems, size_t Size, int TypeSub)
{
	VMenu2 ItemsMenu(MSG(MSetColorItemsTitle), nullptr, 0);

	FOR(const auto& i, make_range(ItemIds, ItemIds + Size))
	{
		ItemsMenu.AddItem(MSG(i));
	}

	ItemsMenu.SetPosition(17-(TypeSub == 2?7:0),5+(TypeSub == 2?2:0),0,0);
	ItemsMenu.SetFlags(VMENU_WRAPMODE);
	ItemsMenu.RunEx([&](int Msg, void *param)->int
	{
		intptr_t ItemsCode=(intptr_t)param;
		if (Msg!=DN_CLOSE || ItemsCode<0)
			return 0;

		static const LNGID ListItems[] =
		{
			MSetColorDialogListText,
			MSetColorDialogListHighLight,
			MSetColorDialogListSelectedText,
			MSetColorDialogListSelectedHighLight,
			MSetColorDialogListDisabled,
			MSetColorDialogListBox,
			MSetColorDialogListTitle,
			MSetColorDialogListScrollBar,
			MSetColorDialogListArrows,
			MSetColorDialogListArrowsSelected,
			MSetColorDialogListArrowsDisabled,
			MSetColorDialogListGrayed,
			MSetColorDialogSelectedListGrayed,
		};

		static const int ListPaletteItemsNormal[] =
		{
			COL_DIALOGLISTTEXT,
			COL_DIALOGLISTHIGHLIGHT,
			COL_DIALOGLISTSELECTEDTEXT,
			COL_DIALOGLISTSELECTEDHIGHLIGHT,
			COL_DIALOGLISTDISABLED,
			COL_DIALOGLISTBOX,
			COL_DIALOGLISTTITLE,
			COL_DIALOGLISTSCROLLBAR,
			COL_DIALOGLISTARROWS,             // Arrow
			COL_DIALOGLISTARROWSSELECTED,     // Выбранный - Arrow
			COL_DIALOGLISTARROWSDISABLED,     // Arrow disabled
			COL_DIALOGLISTGRAY,               // "серый"
			COL_DIALOGLISTSELECTEDGRAYTEXT,   // выбранный "серый"
		};
		CheckSize(ListItems, ListPaletteItemsNormal);

		static const int ListPaletteItemsWarn[] =
		{
			COL_WARNDIALOGLISTTEXT,
			COL_WARNDIALOGLISTHIGHLIGHT,
			COL_WARNDIALOGLISTSELECTEDTEXT,
			COL_WARNDIALOGLISTSELECTEDHIGHLIGHT,
			COL_WARNDIALOGLISTDISABLED,
			COL_WARNDIALOGLISTBOX,
			COL_WARNDIALOGLISTTITLE,
			COL_WARNDIALOGLISTSCROLLBAR,
			COL_WARNDIALOGLISTARROWS,            // Arrow
			COL_WARNDIALOGLISTARROWSSELECTED,    // Выбранный - Arrow
			COL_WARNDIALOGLISTARROWSDISABLED,    // Arrow disabled
			COL_WARNDIALOGLISTGRAY,              // "серый"
			COL_WARNDIALOGLISTSELECTEDGRAYTEXT,  // выбранный "серый"
		};
		CheckSize(ListItems, ListPaletteItemsWarn);

		static const int ComboPaletteItemsNormal[] =
		{
			COL_DIALOGCOMBOTEXT,
			COL_DIALOGCOMBOHIGHLIGHT,
			COL_DIALOGCOMBOSELECTEDTEXT,
			COL_DIALOGCOMBOSELECTEDHIGHLIGHT,
			COL_DIALOGCOMBODISABLED,
			COL_DIALOGCOMBOBOX,
			COL_DIALOGCOMBOTITLE,
			COL_DIALOGCOMBOSCROLLBAR,
			COL_DIALOGCOMBOARROWS,               // Arrow
			COL_DIALOGCOMBOARROWSSELECTED,       // Выбранный - Arrow
			COL_DIALOGCOMBOARROWSDISABLED,       // Arrow disabled
			COL_DIALOGCOMBOGRAY,                 // "серый"
			COL_DIALOGCOMBOSELECTEDGRAYTEXT,     // выбранный "серый"
		};
		CheckSize(ListItems, ComboPaletteItemsNormal);

		static const int ComboPaletteItemsWarn[] =
		{
			// warn
			COL_WARNDIALOGCOMBOTEXT,
			COL_WARNDIALOGCOMBOHIGHLIGHT,
			COL_WARNDIALOGCOMBOSELECTEDTEXT,
			COL_WARNDIALOGCOMBOSELECTEDHIGHLIGHT,
			COL_WARNDIALOGCOMBODISABLED,
			COL_WARNDIALOGCOMBOBOX,
			COL_WARNDIALOGCOMBOTITLE,
			COL_WARNDIALOGCOMBOSCROLLBAR,
			COL_WARNDIALOGCOMBOARROWS,            // Arrow
			COL_WARNDIALOGCOMBOARROWSSELECTED,    // Выбранный - Arrow
			COL_WARNDIALOGCOMBOARROWSDISABLED,    // Arrow disabled
			COL_WARNDIALOGCOMBOGRAY,              // "серый"
			COL_WARNDIALOGCOMBOSELECTEDGRAYTEXT,  // выбранный "серый"
		};
		CheckSize(ListItems, ComboPaletteItemsNormal);

		static const int* ListPaletteItems[] =
		{
			ListPaletteItemsNormal,
			ListPaletteItemsWarn,
			ComboPaletteItemsNormal,
			ComboPaletteItemsWarn,
		};
		static_assert(ARRAYSIZE(ListPaletteItems) == list_modes_count, "wrong array size");

		ItemsMenu.SendMessage(DM_ENABLEREDRAW, 1, nullptr);
		if (TypeSub == 1 && PaletteItems[ItemsCode] < list_modes_count)
		{
			SetItemColors(ListItems,ListPaletteItems[PaletteItems[ItemsCode]],ARRAYSIZE(ListItems),2);
		}
		else
			GetColor(PaletteItems[ItemsCode]);

		return 1;
	});
}

int ColorIndex[]=
{
	F_LIGHTGRAY|B_BLACK,
	F_BLACK|B_RED,
	F_LIGHTGRAY|B_DARKGRAY,
	F_BLACK|B_LIGHTRED,
	F_LIGHTGRAY|B_BLUE,
	F_BLACK|B_MAGENTA,
	F_BLACK|B_LIGHTBLUE,
	F_BLACK|B_LIGHTMAGENTA,
	F_BLACK|B_GREEN,
	F_BLACK|B_BROWN,
	F_BLACK|B_LIGHTGREEN,
	F_BLACK|B_YELLOW,
	F_BLACK|B_CYAN,
	F_BLACK|B_LIGHTGRAY,
	F_BLACK|B_LIGHTCYAN,
	F_BLACK|B_WHITE
};

static intptr_t GetColorDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	switch (Msg)
	{
		case DN_CTLCOLORDLGITEM:

			if (Param1 >= 2 && Param1 <= 17) // Fore
			{
				auto Colors = static_cast<FarDialogItemColors*>(Param2);
				Colors->Colors[0] = Colors::ConsoleColorToFarColor(ColorIndex[Param1-2]);
			}

			if (Param1 >= 19 && Param1 <= 34) // Back
			{
				auto Colors = static_cast<FarDialogItemColors*>(Param2);
				Colors->Colors[0] = Colors::ConsoleColorToFarColor(ColorIndex[Param1-19]);
			}

			if (Param1 >= 37 && Param1 <= 39)
			{
				auto CurColor = reinterpret_cast<FarColor*>(Dlg->SendMessage(DM_GETDLGDATA, 0, 0));
				auto Colors = static_cast<FarDialogItemColors*>(Param2);
				Colors->Colors[0] = *CurColor;
			}
			break;

		case DN_BTNCLICK:

			if (Param1 >= 2 && Param1 <= 34)
			{
				FarColor NewColor;
				auto CurColor = reinterpret_cast<FarColor*>(Dlg->SendMessage(DM_GETDLGDATA, 0, 0));
				FarDialogItem DlgItem = {};
				Dlg->SendMessage( DM_GETDLGITEMSHORT, Param1, &DlgItem);
				NewColor=*CurColor;

				if (Param1 >= 2 && Param1 <= 17) // Fore
				{
					UINT B = NewColor.BackgroundColor;
					NewColor = Colors::ConsoleColorToFarColor(ColorIndex[Param1-2]);
					NewColor.ForegroundColor = NewColor.BackgroundColor;
					NewColor.BackgroundColor = B;
				}

				if (Param1 >= 19 && Param1 <= 34) // Back
				{
					UINT F = NewColor.ForegroundColor;
					NewColor = Colors::ConsoleColorToFarColor(ColorIndex[Param1-19]);
					//NewColor.BackgroundColor = NewColor.ForegroundColor;
					NewColor.ForegroundColor = F;
				}

				if (NewColor.BackgroundColor!=CurColor->BackgroundColor || NewColor.ForegroundColor!=CurColor->ForegroundColor)
					*CurColor=NewColor;

				return TRUE;
			}

			break;
		default:
			break;
	}

	return Dlg->DefProc(Msg, Param1, Param2);
}


bool GetColorDialogInternal(FarColor& Color,bool bCentered,bool bAddTransparent)
{
	FarDialogItem ColorDlgData[]=
	{
		{DI_DOUBLEBOX,   3, 1,35,13, 0,nullptr,nullptr,0,MSG(MSetColorTitle)},
		{DI_SINGLEBOX,   5, 2,18, 7, 0,nullptr,nullptr,0,MSG(MSetColorForeground)},
		{DI_RADIOBUTTON, 6, 3, 0, 3, 0,nullptr,nullptr,DIF_GROUP|DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON, 6, 4, 0, 4, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON, 6, 5, 0, 5, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON, 6, 6, 0, 6, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON, 9, 3, 0, 3, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON, 9, 4, 0, 4, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON, 9, 5, 0, 5, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON, 9, 6, 0, 6, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,12, 3, 0, 3, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,12, 4, 0, 4, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,12, 5, 0, 5, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,12, 6, 0, 6, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,15, 3, 0, 3, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,15, 4, 0, 4, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,15, 5, 0, 5, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,15, 6, 0, 6, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_SINGLEBOX,  20, 2,33, 7, 0,nullptr,nullptr,0,MSG(MSetColorBackground)},
		{DI_RADIOBUTTON,21, 3, 0, 3, 0,nullptr,nullptr,DIF_GROUP|DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,21, 4, 0, 4, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,21, 5, 0, 5, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,21, 6, 0, 6, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,24, 3, 0, 3, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,24, 4, 0, 4, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,24, 5, 0, 5, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,24, 6, 0, 6, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,27, 3, 0, 3, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,27, 4, 0, 4, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,27, 5, 0, 5, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,27, 6, 0, 6, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,30, 3, 0, 3, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,30, 4, 0, 4, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,30, 5, 0, 5, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,30, 6, 0, 6, 0,nullptr,nullptr,DIF_MOVESELECT,L""},

		{DI_CHECKBOX,    5, 10,0, 10,0,nullptr,nullptr,0,MSG(MSetColorForeTransparent)},
		{DI_CHECKBOX,   22, 10,0, 10,0,nullptr,nullptr,0,MSG(MSetColorBackTransparent)},

		{DI_TEXT,        5, 8, 33,8, 0,nullptr,nullptr,0,MSG(MSetColorSample)},
		{DI_TEXT,        5, 9, 33,9, 0,nullptr,nullptr,0,MSG(MSetColorSample)},
		{DI_TEXT,        5,10, 33,10,0,nullptr,nullptr,0,MSG(MSetColorSample)},
		{DI_TEXT,       -1,11, 0, 11,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,      0,12, 0, 12,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MSetColorSet)},
		{DI_BUTTON,      0,12, 0, 12,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MSetColorCancel)},

	};
	auto ColorDlg = MakeDialogItemsEx(ColorDlgData);
	int ExitCode;
	FarColor CurColor=Color;
	int ConsoleColor = Colors::FarColorToConsoleColor(Color);
	for (size_t i=2; i<18; i++)
	{
		if (((ColorIndex[i-2]&B_MASK)>>4) == (ConsoleColor&F_MASK))
		{
			ColorDlg[i].Selected=1;
			ColorDlg[i].Flags|=DIF_FOCUS;
			break;
		}
	}

	for (size_t i=19; i<35; i++)
	{
		if ((ColorIndex[i-19]&B_MASK) == (ConsoleColor&B_MASK))
		{
			ColorDlg[i].Selected=1;
			break;
		}
	}

	if (bAddTransparent)
	{
		ColorDlg[0].Y2++;

		for (size_t i=37; i<=42; i++)
		{
			ColorDlg[i].Y1+=3;
			ColorDlg[i].Y2+=3;
		}

		ColorDlg[0].X2+=4;
		ColorDlg[0].Y2+=2;
		ColorDlg[1].X2+=2;
		ColorDlg[1].Y2+=2;
		ColorDlg[18].X1+=2;
		ColorDlg[18].X2+=4;
		ColorDlg[18].Y2+=2;

		for (size_t i=2; i<=17; i++)
		{
			ColorDlg[i].X1+=1;
			ColorDlg[i].Y1+=1;
			ColorDlg[i].Y2+=1;
		}

		for (size_t i=19; i<=34; i++)
		{
			ColorDlg[i].X1+=3;
			ColorDlg[i].Y1+=1;
			ColorDlg[i].Y2+=1;
		}

		for (size_t i=37; i<=39; i++)
		{
			ColorDlg[i].X2+=4;
		}

		ColorDlg[35].Selected=IS_TRANSPARENT(Color.ForegroundColor);
		ColorDlg[36].Selected=IS_TRANSPARENT(Color.BackgroundColor);
	}
	else
	{
		ColorDlg[35].Flags|=DIF_HIDDEN;
		ColorDlg[36].Flags|=DIF_HIDDEN;
	}

	{
		Dialog Dlg(ColorDlg, GetColorDlgProc, &CurColor);

		if (bCentered)
			Dlg.SetPosition(-1,-1,39+(bAddTransparent?4:0),15+(bAddTransparent?3:0));
		else
			Dlg.SetPosition(37,2,75+(bAddTransparent?4:0),16+(bAddTransparent?3:0));

		Dlg.Process();
		ExitCode=Dlg.GetExitCode();
	}

	if (ExitCode==41)
	{
		Color=CurColor;
		ColorDlg[35].Selected? MAKE_TRANSPARENT(Color.ForegroundColor) : MAKE_OPAQUE(Color.ForegroundColor);
		ColorDlg[36].Selected? MAKE_TRANSPARENT(Color.BackgroundColor) : MAKE_OPAQUE(Color.BackgroundColor);
		return true;
	}

	return false;
}
