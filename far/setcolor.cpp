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
	lng LngId;
	union
	{
		size_t SubColorCount;
		PaletteColors Color;
	};
	const color_item* SubColor;
};

static void SetItemColors(const color_item* Items, size_t Size, COORD Position = {})
{
	const auto ItemsMenu = VMenu2::create(MSG(lng::MSetColorItemsTitle), nullptr, 0);

	for (const auto& i : make_range(Items, Size))
	{
		ItemsMenu->AddItem(MSG(i.LngId));
	}

	ItemsMenu->SetPosition(Position.X += 10, Position.Y += 5, 0, 0);
	ItemsMenu->SetMenuFlags(VMENU_WRAPMODE);
	ItemsMenu->RunEx([&](int Msg, void *param)
	{
		const auto ItemsCode = reinterpret_cast<intptr_t>(param);
		if (Msg != DN_CLOSE || ItemsCode < 0)
			return 0;

		ItemsMenu->SendMessage(DM_ENABLEREDRAW, 1, nullptr);
		if (Items[ItemsCode].SubColor)
		{
			SetItemColors(Items[ItemsCode].SubColor, Items[ItemsCode].SubColorCount, Position);
		}
		else
		{
			GetColor(Items[ItemsCode].Color);
		}

		return 1;
	});
}
void SetColors()
{
	static constexpr color_item
	PanelItems[] =
	{
		{ lng::MSetColorPanelNormal,                COL_PANELTEXT },
		{ lng::MSetColorPanelSelected,              COL_PANELSELECTEDTEXT },
		{ lng::MSetColorPanelHighlightedText,       COL_PANELHIGHLIGHTTEXT },
		{ lng::MSetColorPanelHighlightedInfo,       COL_PANELINFOTEXT },
		{ lng::MSetColorPanelDragging,              COL_PANELDRAGTEXT },
		{ lng::MSetColorPanelBox,                   COL_PANELBOX },
		{ lng::MSetColorPanelNormalCursor,          COL_PANELCURSOR },
		{ lng::MSetColorPanelSelectedCursor,        COL_PANELSELECTEDCURSOR },
		{ lng::MSetColorPanelNormalTitle,           COL_PANELTITLE },
		{ lng::MSetColorPanelSelectedTitle,         COL_PANELSELECTEDTITLE },
		{ lng::MSetColorPanelColumnTitle,           COL_PANELCOLUMNTITLE },
		{ lng::MSetColorPanelTotalInfo,             COL_PANELTOTALINFO },
		{ lng::MSetColorPanelSelectedInfo,          COL_PANELSELECTEDINFO },
		{ lng::MSetColorPanelScrollbar,             COL_PANELSCROLLBAR },
		{ lng::MSetColorPanelScreensNumber,         COL_PANELSCREENSNUMBER },
	},

	ListItemsNormal[] =
	{
		{ lng::MSetColorDialogListText,                          COL_DIALOGLISTTEXT },
		{ lng::MSetColorDialogListHighLight,                     COL_DIALOGLISTHIGHLIGHT },
		{ lng::MSetColorDialogListSelectedText,                  COL_DIALOGLISTSELECTEDTEXT },
		{ lng::MSetColorDialogListSelectedHighLight,             COL_DIALOGLISTSELECTEDHIGHLIGHT },
		{ lng::MSetColorDialogListDisabled,                      COL_DIALOGLISTDISABLED },
		{ lng::MSetColorDialogListBox,                           COL_DIALOGLISTBOX },
		{ lng::MSetColorDialogListTitle,                         COL_DIALOGLISTTITLE },
		{ lng::MSetColorDialogListScrollBar,                     COL_DIALOGLISTSCROLLBAR },
		{ lng::MSetColorDialogListArrows,                        COL_DIALOGLISTARROWS },
		{ lng::MSetColorDialogListArrowsSelected,                COL_DIALOGLISTARROWSSELECTED },
		{ lng::MSetColorDialogListArrowsDisabled,                COL_DIALOGLISTARROWSDISABLED },
		{ lng::MSetColorDialogListGrayed,                        COL_DIALOGLISTGRAY },
		{ lng::MSetColorDialogSelectedListGrayed,                COL_DIALOGLISTSELECTEDGRAYTEXT },
	},

	ListItemsWarn[] =
	{
		{ lng::MSetColorDialogListText,                          COL_WARNDIALOGLISTTEXT },
		{ lng::MSetColorDialogListHighLight,                     COL_WARNDIALOGLISTHIGHLIGHT },
		{ lng::MSetColorDialogListSelectedText,                  COL_WARNDIALOGLISTSELECTEDTEXT },
		{ lng::MSetColorDialogListSelectedHighLight,             COL_WARNDIALOGLISTSELECTEDHIGHLIGHT },
		{ lng::MSetColorDialogListDisabled,                      COL_WARNDIALOGLISTDISABLED },
		{ lng::MSetColorDialogListBox,                           COL_WARNDIALOGLISTBOX },
		{ lng::MSetColorDialogListTitle,                         COL_WARNDIALOGLISTTITLE },
		{ lng::MSetColorDialogListScrollBar,                     COL_WARNDIALOGLISTSCROLLBAR },
		{ lng::MSetColorDialogListArrows,                        COL_WARNDIALOGLISTARROWS },
		{ lng::MSetColorDialogListArrowsSelected,                COL_WARNDIALOGLISTARROWSSELECTED },
		{ lng::MSetColorDialogListArrowsDisabled,                COL_WARNDIALOGLISTARROWSDISABLED },
		{ lng::MSetColorDialogListGrayed,                        COL_WARNDIALOGLISTGRAY },
		{ lng::MSetColorDialogSelectedListGrayed,                COL_WARNDIALOGLISTSELECTEDGRAYTEXT },
	},

	ComboItemsNormal[] =
	{
		{ lng::MSetColorDialogListText,                          COL_DIALOGCOMBOTEXT },
		{ lng::MSetColorDialogListHighLight,                     COL_DIALOGCOMBOHIGHLIGHT },
		{ lng::MSetColorDialogListSelectedText,                  COL_DIALOGCOMBOSELECTEDTEXT },
		{ lng::MSetColorDialogListSelectedHighLight,             COL_DIALOGCOMBOSELECTEDHIGHLIGHT },
		{ lng::MSetColorDialogListDisabled,                      COL_DIALOGCOMBODISABLED },
		{ lng::MSetColorDialogListBox,                           COL_DIALOGCOMBOBOX },
		{ lng::MSetColorDialogListTitle,                         COL_DIALOGCOMBOTITLE },
		{ lng::MSetColorDialogListScrollBar,                     COL_DIALOGCOMBOSCROLLBAR },
		{ lng::MSetColorDialogListArrows,                        COL_DIALOGCOMBOARROWS },
		{ lng::MSetColorDialogListArrowsSelected,                COL_DIALOGCOMBOARROWSSELECTED },
		{ lng::MSetColorDialogListArrowsDisabled,                COL_DIALOGCOMBOARROWSDISABLED },
		{ lng::MSetColorDialogListGrayed,                        COL_DIALOGCOMBOGRAY },
		{ lng::MSetColorDialogSelectedListGrayed,                COL_DIALOGCOMBOSELECTEDGRAYTEXT },
	},

	ComboItemsWarn[] =
	{
		{ lng::MSetColorDialogListText,                          COL_WARNDIALOGCOMBOTEXT },
		{ lng::MSetColorDialogListHighLight,                     COL_WARNDIALOGCOMBOHIGHLIGHT },
		{ lng::MSetColorDialogListSelectedText,                  COL_WARNDIALOGCOMBOSELECTEDTEXT },
		{ lng::MSetColorDialogListSelectedHighLight,             COL_WARNDIALOGCOMBOSELECTEDHIGHLIGHT },
		{ lng::MSetColorDialogListDisabled,                      COL_WARNDIALOGCOMBODISABLED },
		{ lng::MSetColorDialogListBox,                           COL_WARNDIALOGCOMBOBOX },
		{ lng::MSetColorDialogListTitle,                         COL_WARNDIALOGCOMBOTITLE },
		{ lng::MSetColorDialogListScrollBar,                     COL_WARNDIALOGCOMBOSCROLLBAR },
		{ lng::MSetColorDialogListArrows,                        COL_WARNDIALOGCOMBOARROWS },
		{ lng::MSetColorDialogListArrowsSelected,                COL_WARNDIALOGCOMBOARROWSSELECTED },
		{ lng::MSetColorDialogListArrowsDisabled,                COL_WARNDIALOGCOMBOARROWSDISABLED },
		{ lng::MSetColorDialogListGrayed,                        COL_WARNDIALOGCOMBOGRAY },
		{ lng::MSetColorDialogSelectedListGrayed,                COL_WARNDIALOGCOMBOSELECTEDGRAYTEXT },
	},

	DialogItems[] =
	{
		{ lng::MSetColorDialogNormal,                                   COL_DIALOGTEXT },
		{ lng::MSetColorDialogHighlighted,                              COL_DIALOGHIGHLIGHTTEXT },
		{ lng::MSetColorDialogDisabled,                                 COL_DIALOGDISABLED },
		{ lng::MSetColorDialogBox,                                      COL_DIALOGBOX },
		{ lng::MSetColorDialogBoxTitle,                                 COL_DIALOGBOXTITLE },
		{ lng::MSetColorDialogHighlightedBoxTitle,                      COL_DIALOGHIGHLIGHTBOXTITLE },
		{ lng::MSetColorDialogTextInput,                                COL_DIALOGEDIT },
		{ lng::MSetColorDialogUnchangedTextInput,                       COL_DIALOGEDITUNCHANGED },
		{ lng::MSetColorDialogSelectedTextInput,                        COL_DIALOGEDITSELECTED },
		{ lng::MSetColorDialogEditDisabled,                             COL_DIALOGEDITDISABLED },
		{ lng::MSetColorDialogButtons,                                  COL_DIALOGBUTTON },
		{ lng::MSetColorDialogSelectedButtons,                          COL_DIALOGSELECTEDBUTTON },
		{ lng::MSetColorDialogHighlightedButtons,                       COL_DIALOGHIGHLIGHTBUTTON },
		{ lng::MSetColorDialogSelectedHighlightedButtons,               COL_DIALOGHIGHLIGHTSELECTEDBUTTON },
		{ lng::MSetColorDialogDefaultButton,                            COL_DIALOGDEFAULTBUTTON },
		{ lng::MSetColorDialogSelectedDefaultButton,                    COL_DIALOGSELECTEDDEFAULTBUTTON },
		{ lng::MSetColorDialogHighlightedDefaultButton,                 COL_DIALOGHIGHLIGHTDEFAULTBUTTON },
		{ lng::MSetColorDialogSelectedHighlightedDefaultButton,         COL_DIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON },
		{ lng::MSetColorDialogListBoxControl,                           std::size(ListItemsNormal), ListItemsNormal },
		{ lng::MSetColorDialogComboBoxControl,                          std::size(ComboItemsNormal), ComboItemsNormal },
	},

	WarnDialogItems[] =
	{
		{ lng::MSetColorDialogNormal,                                   COL_WARNDIALOGTEXT },
		{ lng::MSetColorDialogHighlighted,                              COL_WARNDIALOGHIGHLIGHTTEXT },
		{ lng::MSetColorDialogDisabled,                                 COL_WARNDIALOGDISABLED },
		{ lng::MSetColorDialogBox,                                      COL_WARNDIALOGBOX },
		{ lng::MSetColorDialogBoxTitle,                                 COL_WARNDIALOGBOXTITLE },
		{ lng::MSetColorDialogHighlightedBoxTitle,                      COL_WARNDIALOGHIGHLIGHTBOXTITLE },
		{ lng::MSetColorDialogTextInput,                                COL_WARNDIALOGEDIT },
		{ lng::MSetColorDialogUnchangedTextInput,                       COL_WARNDIALOGEDITUNCHANGED },
		{ lng::MSetColorDialogSelectedTextInput,                        COL_WARNDIALOGEDITSELECTED },
		{ lng::MSetColorDialogEditDisabled,                             COL_WARNDIALOGEDITDISABLED },
		{ lng::MSetColorDialogButtons,                                  COL_WARNDIALOGBUTTON },
		{ lng::MSetColorDialogSelectedButtons,                          COL_WARNDIALOGSELECTEDBUTTON },
		{ lng::MSetColorDialogHighlightedButtons,                       COL_WARNDIALOGHIGHLIGHTBUTTON },
		{ lng::MSetColorDialogSelectedHighlightedButtons,               COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON },
		{ lng::MSetColorDialogDefaultButton,                            COL_WARNDIALOGDEFAULTBUTTON },
		{ lng::MSetColorDialogSelectedDefaultButton,                    COL_WARNDIALOGSELECTEDDEFAULTBUTTON },
		{ lng::MSetColorDialogHighlightedDefaultButton,                 COL_WARNDIALOGHIGHLIGHTDEFAULTBUTTON },
		{ lng::MSetColorDialogSelectedHighlightedDefaultButton,         COL_WARNDIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON },
		{ lng::MSetColorDialogListBoxControl,                           std::size(ListItemsWarn), ListItemsWarn },
		{ lng::MSetColorDialogComboBoxControl,                          std::size(ComboItemsWarn), ComboItemsWarn },
	},

	MenuItems[] =
	{
		{ lng::MSetColorMenuNormal,                   COL_MENUTEXT },
		{ lng::MSetColorMenuSelected,                 COL_MENUSELECTEDTEXT },
		{ lng::MSetColorMenuHighlighted,              COL_MENUHIGHLIGHT },
		{ lng::MSetColorMenuSelectedHighlighted,      COL_MENUSELECTEDHIGHLIGHT },
		{ lng::MSetColorMenuDisabled,                 COL_MENUDISABLEDTEXT },
		{ lng::MSetColorMenuBox,                      COL_MENUBOX },
		{ lng::MSetColorMenuTitle,                    COL_MENUTITLE },
		{ lng::MSetColorMenuScrollBar,                COL_MENUSCROLLBAR },
		{ lng::MSetColorMenuArrows,                   COL_MENUARROWS },
		{ lng::MSetColorMenuArrowsSelected,           COL_MENUARROWSSELECTED },
		{ lng::MSetColorMenuArrowsDisabled,           COL_MENUARROWSDISABLED },
		{ lng::MSetColorMenuGrayed,                   COL_MENUGRAYTEXT },
		{ lng::MSetColorMenuSelectedGrayed,           COL_MENUSELECTEDGRAYTEXT },
	},

	HMenuItems[] =
	{
		{ lng::MSetColorHMenuNormal,                  COL_HMENUTEXT },
		{ lng::MSetColorHMenuSelected,                COL_HMENUSELECTEDTEXT },
		{ lng::MSetColorHMenuHighlighted,             COL_HMENUHIGHLIGHT },
		{ lng::MSetColorHMenuSelectedHighlighted,     COL_HMENUSELECTEDHIGHLIGHT },
	},

	KeyBarItems[] =
	{
		{ lng::MSetColorKeyBarNumbers,                COL_KEYBARNUM },
		{ lng::MSetColorKeyBarNames,                  COL_KEYBARTEXT },
		{ lng::MSetColorKeyBarBackground,             COL_KEYBARBACKGROUND },
	},

	CommandLineItems[] =
	{
		{ lng::MSetColorCommandLineNormal,            COL_COMMANDLINE },
		{ lng::MSetColorCommandLineSelected,          COL_COMMANDLINESELECTED },
		{ lng::MSetColorCommandLinePrefix,            COL_COMMANDLINEPREFIX },
		{ lng::MSetColorCommandLineUserScreen,        COL_COMMANDLINEUSERSCREEN },
	},

	ClockItems[] =
	{
		{ lng::MSetColorClockNormal,                  COL_CLOCK },
		{ lng::MSetColorClockNormalEditor,            COL_EDITORCLOCK },
		{ lng::MSetColorClockNormalViewer,            COL_VIEWERCLOCK },
	},

	ViewerItems[] =
	{
		{ lng::MSetColorViewerNormal,                 COL_VIEWERTEXT },
		{ lng::MSetColorViewerSelected,               COL_VIEWERSELECTEDTEXT },
		{ lng::MSetColorViewerStatus,                 COL_VIEWERSTATUS },
		{ lng::MSetColorViewerArrows,                 COL_VIEWERARROWS },
		{ lng::MSetColorViewerScrollbar,              COL_VIEWERSCROLLBAR },
	},

	EditorItems[] =
	{
		{ lng::MSetColorEditorNormal,                 COL_EDITORTEXT },
		{ lng::MSetColorEditorSelected,               COL_EDITORSELECTEDTEXT },
		{ lng::MSetColorEditorStatus,                 COL_EDITORSTATUS },
		{ lng::MSetColorEditorScrollbar,              COL_EDITORSCROLLBAR },
	},

	HelpItems[] =
	{
		{ lng::MSetColorHelpNormal,                   COL_HELPTEXT },
		{ lng::MSetColorHelpHighlighted,              COL_HELPHIGHLIGHTTEXT },
		{ lng::MSetColorHelpReference,                COL_HELPTOPIC },
		{ lng::MSetColorHelpSelectedReference,        COL_HELPSELECTEDTOPIC },
		{ lng::MSetColorHelpBox,                      COL_HELPBOX },
		{ lng::MSetColorHelpBoxTitle,                 COL_HELPBOXTITLE },
		{ lng::MSetColorHelpScrollbar,                COL_HELPSCROLLBAR },
	};

	{
		// NOT constexpr, see VS bug #3103404
		static const struct
		{
			lng MenuId;
			range<const color_item*> Subitems;
		}
		Groups[] =
		{
			{ lng::MSetColorPanel,       make_range(PanelItems) },
			{ lng::MSetColorDialog,      make_range(DialogItems) },
			{ lng::MSetColorWarning,     make_range(WarnDialogItems) },
			{ lng::MSetColorMenu,        make_range(MenuItems) },
			{ lng::MSetColorHMenu,       make_range(HMenuItems) },
			{ lng::MSetColorKeyBar,      make_range(KeyBarItems) },
			{ lng::MSetColorCommandLine, make_range(CommandLineItems) },
			{ lng::MSetColorClock,       make_range(ClockItems) },
			{ lng::MSetColorViewer,      make_range(ViewerItems) },
			{ lng::MSetColorEditor,      make_range(EditorItems) },
			{ lng::MSetColorHelp,        make_range(HelpItems) },
		};

		const auto GroupsMenu = VMenu2::create(MSG(lng::MSetColorGroupsTitle), nullptr, 0);

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
		GroupsMenu->AddItem(MSG(lng::MSetDefaultColors));
		const int BlackWhiteId = static_cast<int>(GroupsMenu->size());
		GroupsMenu->AddItem(MSG(lng::MSetBW));

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
	const auto& GetColor = [Param1](size_t Offset)
	{
		return colors::ConsoleColorToFarColor(ColorIndex[Param1 - Offset]);
	};

	switch (Msg)
	{
		case DN_CTLCOLORDLGITEM:
			{
				const auto Colors = static_cast<FarDialogItemColors*>(Param2);
				if (Param1 >= 2 && Param1 <= 17) // Fore
				{
					Colors->Colors[0] = GetColor(2);
				}
				else if (Param1 >= 19 && Param1 <= 34) // Back
				{
					Colors->Colors[0] = GetColor(19);
				}
				else if (Param1 >= 37 && Param1 <= 39)
				{
					Colors->Colors[0] = *reinterpret_cast<FarColor*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
				}
			}
			break;

		case DN_BTNCLICK:

			if (Param1 >= 2 && Param1 <= 34)
			{
				const auto CurColor = reinterpret_cast<FarColor*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
				FarDialogItem DlgItem = {};
				Dlg->SendMessage( DM_GETDLGITEMSHORT, Param1, &DlgItem);

				if (Param1 >= 2 && Param1 <= 17) // Fore
				{
					CurColor->ForegroundColor = GetColor(2).BackgroundColor;
				}
				else if (Param1 >= 19 && Param1 <= 34) // Back
				{
					CurColor->BackgroundColor = GetColor(19).BackgroundColor;
				}

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
		{DI_DOUBLEBOX,   3, 1,35,13, 0,nullptr,nullptr,0,MSG(lng::MSetColorTitle)},
		{DI_SINGLEBOX,   5, 2,18, 7, 0,nullptr,nullptr,0,MSG(lng::MSetColorForeground)},
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
		{DI_SINGLEBOX,  20, 2,33, 7, 0,nullptr,nullptr,0,MSG(lng::MSetColorBackground)},
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

		{DI_CHECKBOX,    5, 10,0, 10,0,nullptr,nullptr,0,MSG(lng::MSetColorForeTransparent)},
		{DI_CHECKBOX,   22, 10,0, 10,0,nullptr,nullptr,0,MSG(lng::MSetColorBackTransparent)},

		{DI_TEXT,        5, 8, 33,8, 0,nullptr,nullptr,0,MSG(lng::MSetColorSample)},
		{DI_TEXT,        5, 9, 33,9, 0,nullptr,nullptr,0,MSG(lng::MSetColorSample)},
		{DI_TEXT,        5,10, 33,10,0,nullptr,nullptr,0,MSG(lng::MSetColorSample)},
		{DI_TEXT,       -1,11, 0, 11,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,      0,12, 0, 12,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(lng::MSetColorSet)},
		{DI_BUTTON,      0,12, 0, 12,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(lng::MSetColorCancel)},

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
