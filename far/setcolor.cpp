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
#include "farcolor.hpp"
#include "vmenu2.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "panel.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "console.hpp"
#include "colormix.hpp"
#include "language.hpp"
#include "manager.hpp"

void GetColor(PaletteColors PaletteIndex)
{
	auto NewColor = Global->Opt->Palette[PaletteIndex];

	if (!Console().GetColorDialog(NewColor))
		return;

	Global->Opt->Palette.Set(PaletteIndex, &NewColor, 1);
	Global->ScrBuf->Lock(); // отменяем всякую прорисовку
	Global->CtrlObject->Cp()->LeftPanel()->Update(UPDATE_KEEP_SELECTION);
	Global->CtrlObject->Cp()->LeftPanel()->Redraw();
	Global->CtrlObject->Cp()->RightPanel()->Update(UPDATE_KEEP_SELECTION);
	Global->CtrlObject->Cp()->RightPanel()->Redraw();

	Global->WindowManager->ResizeAllWindows(); // рефрешим
	Global->WindowManager->PluginCommit(); // коммитим.

	if (Global->Opt->Clock)
		ShowTime();

	Global->ScrBuf->Unlock(); // разрешаем прорисовку
	Global->WindowManager->PluginCommit(); // коммитим.
}

enum list_mode
{
	lm_list_normal,
	lm_list_warning,
	lm_combo_normal,
	lm_combo_warning,

	list_modes_count
};

struct color_item
{
	LNGID LngId;
	union
	{
		size_t SubColorCount;
		PaletteColors Color;
	};
	const color_item* SubColor;
};

static void SetItemColors(const color_item* Items, size_t Size);

void SetColors()
{
	static constexpr color_item
	PanelItems[] =
	{
		{ MSetColorPanelNormal,                COL_PANELTEXT },
		{ MSetColorPanelSelected,              COL_PANELSELECTEDTEXT },
		{ MSetColorPanelHighlightedText,       COL_PANELHIGHLIGHTTEXT },
		{ MSetColorPanelHighlightedInfo,       COL_PANELINFOTEXT },
		{ MSetColorPanelDragging,              COL_PANELDRAGTEXT },
		{ MSetColorPanelBox,                   COL_PANELBOX },
		{ MSetColorPanelNormalCursor,          COL_PANELCURSOR },
		{ MSetColorPanelSelectedCursor,        COL_PANELSELECTEDCURSOR },
		{ MSetColorPanelNormalTitle,           COL_PANELTITLE },
		{ MSetColorPanelSelectedTitle,         COL_PANELSELECTEDTITLE },
		{ MSetColorPanelColumnTitle,           COL_PANELCOLUMNTITLE },
		{ MSetColorPanelTotalInfo,             COL_PANELTOTALINFO },
		{ MSetColorPanelSelectedInfo,          COL_PANELSELECTEDINFO },
		{ MSetColorPanelScrollbar,             COL_PANELSCROLLBAR },
		{ MSetColorPanelScreensNumber,         COL_PANELSCREENSNUMBER },
	},

	ListItemsNormal[] =
	{
		{ MSetColorDialogListText,                          COL_DIALOGLISTTEXT },
		{ MSetColorDialogListHighLight,                     COL_DIALOGLISTHIGHLIGHT },
		{ MSetColorDialogListSelectedText,                  COL_DIALOGLISTSELECTEDTEXT },
		{ MSetColorDialogListSelectedHighLight,             COL_DIALOGLISTSELECTEDHIGHLIGHT },
		{ MSetColorDialogListDisabled,                      COL_DIALOGLISTDISABLED },
		{ MSetColorDialogListBox,                           COL_DIALOGLISTBOX },
		{ MSetColorDialogListTitle,                         COL_DIALOGLISTTITLE },
		{ MSetColorDialogListScrollBar,                     COL_DIALOGLISTSCROLLBAR },
		{ MSetColorDialogListArrows,                        COL_DIALOGLISTARROWS },
		{ MSetColorDialogListArrowsSelected,                COL_DIALOGLISTARROWSSELECTED },
		{ MSetColorDialogListArrowsDisabled,                COL_DIALOGLISTARROWSDISABLED },
		{ MSetColorDialogListGrayed,                        COL_DIALOGLISTGRAY },
		{ MSetColorDialogSelectedListGrayed,                COL_DIALOGLISTSELECTEDGRAYTEXT },
	},

	ListItemsWarn[] =
	{
		{ MSetColorDialogListText,                          COL_WARNDIALOGLISTTEXT },
		{ MSetColorDialogListHighLight,                     COL_WARNDIALOGLISTHIGHLIGHT },
		{ MSetColorDialogListSelectedText,                  COL_WARNDIALOGLISTSELECTEDTEXT },
		{ MSetColorDialogListSelectedHighLight,             COL_WARNDIALOGLISTSELECTEDHIGHLIGHT },
		{ MSetColorDialogListDisabled,                      COL_WARNDIALOGLISTDISABLED },
		{ MSetColorDialogListBox,                           COL_WARNDIALOGLISTBOX },
		{ MSetColorDialogListTitle,                         COL_WARNDIALOGLISTTITLE },
		{ MSetColorDialogListScrollBar,                     COL_WARNDIALOGLISTSCROLLBAR },
		{ MSetColorDialogListArrows,                        COL_WARNDIALOGLISTARROWS },
		{ MSetColorDialogListArrowsSelected,                COL_WARNDIALOGLISTARROWSSELECTED },
		{ MSetColorDialogListArrowsDisabled,                COL_WARNDIALOGLISTARROWSDISABLED },
		{ MSetColorDialogListGrayed,                        COL_WARNDIALOGLISTGRAY },
		{ MSetColorDialogSelectedListGrayed,                COL_WARNDIALOGLISTSELECTEDGRAYTEXT },
	},

	ComboItemsNormal[] =
	{
		{ MSetColorDialogListText,                          COL_DIALOGCOMBOTEXT },
		{ MSetColorDialogListHighLight,                     COL_DIALOGCOMBOHIGHLIGHT },
		{ MSetColorDialogListSelectedText,                  COL_DIALOGCOMBOSELECTEDTEXT },
		{ MSetColorDialogListSelectedHighLight,             COL_DIALOGCOMBOSELECTEDHIGHLIGHT },
		{ MSetColorDialogListDisabled,                      COL_DIALOGCOMBODISABLED },
		{ MSetColorDialogListBox,                           COL_DIALOGCOMBOBOX },
		{ MSetColorDialogListTitle,                         COL_DIALOGCOMBOTITLE },
		{ MSetColorDialogListScrollBar,                     COL_DIALOGCOMBOSCROLLBAR },
		{ MSetColorDialogListArrows,                        COL_DIALOGCOMBOARROWS },
		{ MSetColorDialogListArrowsSelected,                COL_DIALOGCOMBOARROWSSELECTED },
		{ MSetColorDialogListArrowsDisabled,                COL_DIALOGCOMBOARROWSDISABLED },
		{ MSetColorDialogListGrayed,                        COL_DIALOGCOMBOGRAY },
		{ MSetColorDialogSelectedListGrayed,                COL_DIALOGCOMBOSELECTEDGRAYTEXT },
	},

	ComboItemsWarn[] =
	{
		{ MSetColorDialogListText,                          COL_WARNDIALOGCOMBOTEXT },
		{ MSetColorDialogListHighLight,                     COL_WARNDIALOGCOMBOHIGHLIGHT },
		{ MSetColorDialogListSelectedText,                  COL_WARNDIALOGCOMBOSELECTEDTEXT },
		{ MSetColorDialogListSelectedHighLight,             COL_WARNDIALOGCOMBOSELECTEDHIGHLIGHT },
		{ MSetColorDialogListDisabled,                      COL_WARNDIALOGCOMBODISABLED },
		{ MSetColorDialogListBox,                           COL_WARNDIALOGCOMBOBOX },
		{ MSetColorDialogListTitle,                         COL_WARNDIALOGCOMBOTITLE },
		{ MSetColorDialogListScrollBar,                     COL_WARNDIALOGCOMBOSCROLLBAR },
		{ MSetColorDialogListArrows,                        COL_WARNDIALOGCOMBOARROWS },
		{ MSetColorDialogListArrowsSelected,                COL_WARNDIALOGCOMBOARROWSSELECTED },
		{ MSetColorDialogListArrowsDisabled,                COL_WARNDIALOGCOMBOARROWSDISABLED },
		{ MSetColorDialogListGrayed,                        COL_WARNDIALOGCOMBOGRAY },
		{ MSetColorDialogSelectedListGrayed,                COL_WARNDIALOGCOMBOSELECTEDGRAYTEXT },
	},

	DialogItems[] =
	{
		{ MSetColorDialogNormal,                                   COL_DIALOGTEXT },
		{ MSetColorDialogHighlighted,                              COL_DIALOGHIGHLIGHTTEXT },
		{ MSetColorDialogDisabled,                                 COL_DIALOGDISABLED },
		{ MSetColorDialogBox,                                      COL_DIALOGBOX },
		{ MSetColorDialogBoxTitle,                                 COL_DIALOGBOXTITLE },
		{ MSetColorDialogHighlightedBoxTitle,                      COL_DIALOGHIGHLIGHTBOXTITLE },
		{ MSetColorDialogTextInput,                                COL_DIALOGEDIT },
		{ MSetColorDialogUnchangedTextInput,                       COL_DIALOGEDITUNCHANGED },
		{ MSetColorDialogSelectedTextInput,                        COL_DIALOGEDITSELECTED },
		{ MSetColorDialogEditDisabled,                             COL_DIALOGEDITDISABLED },
		{ MSetColorDialogButtons,                                  COL_DIALOGBUTTON },
		{ MSetColorDialogSelectedButtons,                          COL_DIALOGSELECTEDBUTTON },
		{ MSetColorDialogHighlightedButtons,                       COL_DIALOGHIGHLIGHTBUTTON },
		{ MSetColorDialogSelectedHighlightedButtons,               COL_DIALOGHIGHLIGHTSELECTEDBUTTON },
		{ MSetColorDialogDefaultButton,                            COL_DIALOGDEFAULTBUTTON },
		{ MSetColorDialogSelectedDefaultButton,                    COL_DIALOGSELECTEDDEFAULTBUTTON },
		{ MSetColorDialogHighlightedDefaultButton,                 COL_DIALOGHIGHLIGHTDEFAULTBUTTON },
		{ MSetColorDialogSelectedHighlightedDefaultButton,         COL_DIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON },
		{ MSetColorDialogListBoxControl,                           std::size(ListItemsNormal), ListItemsNormal },
		{ MSetColorDialogComboBoxControl,                          std::size(ComboItemsNormal), ComboItemsNormal },
	},

	WarnDialogItems[] =
	{
		{ MSetColorDialogNormal,                                   COL_WARNDIALOGTEXT },
		{ MSetColorDialogHighlighted,                              COL_WARNDIALOGHIGHLIGHTTEXT },
		{ MSetColorDialogDisabled,                                 COL_WARNDIALOGDISABLED },
		{ MSetColorDialogBox,                                      COL_WARNDIALOGBOX },
		{ MSetColorDialogBoxTitle,                                 COL_WARNDIALOGBOXTITLE },
		{ MSetColorDialogHighlightedBoxTitle,                      COL_WARNDIALOGHIGHLIGHTBOXTITLE },
		{ MSetColorDialogTextInput,                                COL_WARNDIALOGEDIT },
		{ MSetColorDialogUnchangedTextInput,                       COL_WARNDIALOGEDITUNCHANGED },
		{ MSetColorDialogSelectedTextInput,                        COL_WARNDIALOGEDITSELECTED },
		{ MSetColorDialogEditDisabled,                             COL_WARNDIALOGEDITDISABLED },
		{ MSetColorDialogButtons,                                  COL_WARNDIALOGBUTTON },
		{ MSetColorDialogSelectedButtons,                          COL_WARNDIALOGSELECTEDBUTTON },
		{ MSetColorDialogHighlightedButtons,                       COL_WARNDIALOGHIGHLIGHTBUTTON },
		{ MSetColorDialogSelectedHighlightedButtons,               COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON },
		{ MSetColorDialogDefaultButton,                            COL_WARNDIALOGDEFAULTBUTTON },
		{ MSetColorDialogSelectedDefaultButton,                    COL_WARNDIALOGSELECTEDDEFAULTBUTTON },
		{ MSetColorDialogHighlightedDefaultButton,                 COL_WARNDIALOGHIGHLIGHTDEFAULTBUTTON },
		{ MSetColorDialogSelectedHighlightedDefaultButton,         COL_WARNDIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON },
		{ MSetColorDialogListBoxControl,                           std::size(ListItemsWarn), ListItemsWarn },
		{ MSetColorDialogComboBoxControl,                          std::size(ComboItemsWarn), ComboItemsWarn },
	},

	MenuItems[] =
	{
		{ MSetColorMenuNormal,                   COL_MENUTEXT },
		{ MSetColorMenuSelected,                 COL_MENUSELECTEDTEXT },
		{ MSetColorMenuHighlighted,              COL_MENUHIGHLIGHT },
		{ MSetColorMenuSelectedHighlighted,      COL_MENUSELECTEDHIGHLIGHT },
		{ MSetColorMenuDisabled,                 COL_MENUDISABLEDTEXT },
		{ MSetColorMenuBox,                      COL_MENUBOX },
		{ MSetColorMenuTitle,                    COL_MENUTITLE },
		{ MSetColorMenuScrollBar,                COL_MENUSCROLLBAR },
		{ MSetColorMenuArrows,                   COL_MENUARROWS },
		{ MSetColorMenuArrowsSelected,           COL_MENUARROWSSELECTED },
		{ MSetColorMenuArrowsDisabled,           COL_MENUARROWSDISABLED },
		{ MSetColorMenuGrayed,                   COL_MENUGRAYTEXT },
		{ MSetColorMenuSelectedGrayed,           COL_MENUSELECTEDGRAYTEXT },
	},

	HMenuItems[] =
	{
		{ MSetColorHMenuNormal,                  COL_HMENUTEXT },
		{ MSetColorHMenuSelected,                COL_HMENUSELECTEDTEXT },
		{ MSetColorHMenuHighlighted,             COL_HMENUHIGHLIGHT },
		{ MSetColorHMenuSelectedHighlighted,     COL_HMENUSELECTEDHIGHLIGHT },
	},

	KeyBarItems[] =
	{
		{ MSetColorKeyBarNumbers,                COL_KEYBARNUM },
		{ MSetColorKeyBarNames,                  COL_KEYBARTEXT },
		{ MSetColorKeyBarBackground,             COL_KEYBARBACKGROUND },
	},

	CommandLineItems[] =
	{
		{ MSetColorCommandLineNormal,            COL_COMMANDLINE },
		{ MSetColorCommandLineSelected,          COL_COMMANDLINESELECTED },
		{ MSetColorCommandLinePrefix,            COL_COMMANDLINEPREFIX },
		{ MSetColorCommandLineUserScreen,        COL_COMMANDLINEUSERSCREEN },
	},

	ClockItems[] =
	{
		{ MSetColorClockNormal,                  COL_CLOCK },
		{ MSetColorClockNormalEditor,            COL_EDITORCLOCK },
		{ MSetColorClockNormalViewer,            COL_VIEWERCLOCK },
	},

	ViewerItems[] =
	{
		{ MSetColorViewerNormal,                 COL_VIEWERTEXT },
		{ MSetColorViewerSelected,               COL_VIEWERSELECTEDTEXT },
		{ MSetColorViewerStatus,                 COL_VIEWERSTATUS },
		{ MSetColorViewerArrows,                 COL_VIEWERARROWS },
		{ MSetColorViewerScrollbar,              COL_VIEWERSCROLLBAR },
	},

	EditorItems[] =
	{
		{ MSetColorEditorNormal,                 COL_EDITORTEXT },
		{ MSetColorEditorSelected,               COL_EDITORSELECTEDTEXT },
		{ MSetColorEditorStatus,                 COL_EDITORSTATUS },
		{ MSetColorEditorScrollbar,              COL_EDITORSCROLLBAR },
	},

	HelpItems[] =
	{
		{ MSetColorHelpNormal,                   COL_HELPTEXT },
		{ MSetColorHelpHighlighted,              COL_HELPHIGHLIGHTTEXT },
		{ MSetColorHelpReference,                COL_HELPTOPIC },
		{ MSetColorHelpSelectedReference,        COL_HELPSELECTEDTOPIC },
		{ MSetColorHelpBox,                      COL_HELPBOX },
		{ MSetColorHelpBoxTitle,                 COL_HELPBOXTITLE },
		{ MSetColorHelpScrollbar,                COL_HELPSCROLLBAR },
	};

	{
		// NOT static, see VS bug #3103404
		constexpr struct
		{
			LNGID MenuId;
			range<const color_item*> Subitems;
		}
		Groups[] =
		{
			{ MSetColorPanel,       make_range(PanelItems) },
			{ MSetColorDialog,      make_range(DialogItems) },
			{ MSetColorWarning,     make_range(WarnDialogItems) },
			{ MSetColorMenu,        make_range(MenuItems) },
			{ MSetColorHMenu,       make_range(HMenuItems) },
			{ MSetColorKeyBar,      make_range(KeyBarItems) },
			{ MSetColorCommandLine, make_range(CommandLineItems) },
			{ MSetColorClock,       make_range(ClockItems) },
			{ MSetColorViewer,      make_range(ViewerItems) },
			{ MSetColorEditor,      make_range(EditorItems) },
			{ MSetColorHelp,        make_range(HelpItems) },
		};

		const auto GroupsMenu = VMenu2::create(MSG(MSetColorGroupsTitle), nullptr, 0);

		for (const auto& i: Groups)
		{
			GroupsMenu->AddItem(MSG(i.MenuId));
		}

		{
			MenuItemEx tmp;
			tmp.Flags = LIF_SEPARATOR;
			GroupsMenu->AddItem(tmp);
		}

		const int DefaultId = static_cast<int>(GroupsMenu->size());
		GroupsMenu->AddItem(MSG(MSetDefaultColors));
		const int BlackWhiteId = static_cast<int>(GroupsMenu->size());
		GroupsMenu->AddItem(MSG(MSetBW));

		GroupsMenu->SetPosition(2,1,0,0);
		GroupsMenu->SetMenuFlags(VMENU_WRAPMODE);
		int GroupsCode=GroupsMenu->RunEx([&](int Msg, void *param)
		{
			const auto ItemsCode = reinterpret_cast<intptr_t>(param);
			if (Msg != DN_CLOSE || ItemsCode < 0 || static_cast<size_t>(ItemsCode) >= std::size(Groups))
				return 0;
			GroupsMenu->SendMessage(DM_ENABLEREDRAW, 1, nullptr);
			SetItemColors(Groups[ItemsCode].Subitems.data(), Groups[ItemsCode].Subitems.size());
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
	Global->CtrlObject->Cp()->LeftPanel()->Redraw();
	Global->CtrlObject->Cp()->RightPanel()->Redraw();
}


static void SetItemColors(const color_item* Items, size_t Size)
{
	const auto ItemsMenu = VMenu2::create(MSG(MSetColorItemsTitle), nullptr, 0);

	for (const auto& i: make_range(Items, Size))
	{
		ItemsMenu->AddItem(MSG(i.LngId));
	}

	static int MenuX = 0, MenuY = 0;
	MenuX += 10; MenuY += 5;
	SCOPE_EXIT{ MenuX -= 10; MenuY -= 5; };

	ItemsMenu->SetPosition(MenuX, MenuY, 0, 0);
	ItemsMenu->SetMenuFlags(VMENU_WRAPMODE);
	ItemsMenu->RunEx([&](int Msg, void *param)
	{
		const auto ItemsCode = reinterpret_cast<intptr_t>(param);
		if (Msg!=DN_CLOSE || ItemsCode<0)
			return 0;

		ItemsMenu->SendMessage(DM_ENABLEREDRAW, 1, nullptr);
		if (Items[ItemsCode].SubColor)
		{
			SetItemColors(Items[ItemsCode].SubColor, Items[ItemsCode].SubColorCount);
		}
		else
			GetColor(Items[ItemsCode].Color);

		return 1;
	});
}

template<class T>
constexpr auto distinct(T value)
{
	return (~value & 0xff) >> 4 | value;
}

int ColorIndex[] =
{
	distinct(B_BLACK),
	distinct(B_RED),
	distinct(B_DARKGRAY),
	distinct(B_LIGHTRED),
	distinct(B_BLUE),
	distinct(B_MAGENTA),
	distinct(B_LIGHTBLUE),
	distinct(B_LIGHTMAGENTA),
	distinct(B_GREEN),
	distinct(B_BROWN),
	distinct(B_LIGHTGREEN),
	distinct(B_YELLOW),
	distinct(B_CYAN),
	distinct(B_LIGHTGRAY),
	distinct(B_LIGHTCYAN),
	distinct(B_WHITE)
};

static intptr_t GetColorDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	switch (Msg)
	{
		case DN_CTLCOLORDLGITEM:

			if (Param1 >= 2 && Param1 <= 17) // Fore
			{
				const auto Colors = static_cast<FarDialogItemColors*>(Param2);
				Colors->Colors[0] = colors::ConsoleColorToFarColor(ColorIndex[Param1-2]);
			}

			if (Param1 >= 19 && Param1 <= 34) // Back
			{
				const auto Colors = static_cast<FarDialogItemColors*>(Param2);
				Colors->Colors[0] = colors::ConsoleColorToFarColor(ColorIndex[Param1-19]);
			}

			if (Param1 >= 37 && Param1 <= 39)
			{
				const auto CurColor = reinterpret_cast<FarColor*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
				const auto Colors = static_cast<FarDialogItemColors*>(Param2);
				Colors->Colors[0] = *CurColor;
			}
			break;

		case DN_BTNCLICK:

			if (Param1 >= 2 && Param1 <= 34)
			{
				FarColor NewColor;
				const auto CurColor = reinterpret_cast<FarColor*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
				FarDialogItem DlgItem = {};
				Dlg->SendMessage( DM_GETDLGITEMSHORT, Param1, &DlgItem);
				NewColor=*CurColor;

				if (Param1 >= 2 && Param1 <= 17) // Fore
				{
					UINT B = NewColor.BackgroundColor;
					NewColor = colors::ConsoleColorToFarColor(ColorIndex[Param1-2]);
					NewColor.ForegroundColor = NewColor.BackgroundColor;
					NewColor.BackgroundColor = B;
				}

				if (Param1 >= 19 && Param1 <= 34) // Back
				{
					UINT F = NewColor.ForegroundColor;
					NewColor = colors::ConsoleColorToFarColor(ColorIndex[Param1-19]);
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
	int ConsoleColor = colors::FarColorToConsoleColor(Color);
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
		const auto Dlg = Dialog::create(ColorDlg, GetColorDlgProc, &CurColor);

		if (bCentered)
			Dlg->SetPosition(-1,-1,39+(bAddTransparent?4:0),15+(bAddTransparent?3:0));
		else
			Dlg->SetPosition(37,2,75+(bAddTransparent?4:0),16+(bAddTransparent?3:0));

		Dlg->Process();
		ExitCode=Dlg->GetExitCode();
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
