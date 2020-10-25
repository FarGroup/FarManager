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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "setcolor.hpp"

// Internal:
#include "farcolor.hpp"
#include "vmenu.hpp"
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
#include "lang.hpp"
#include "manager.hpp"
#include "global.hpp"
#include "strmix.hpp"
#include "lockscrn.hpp"

// Platform:

// Common:
#include "common.hpp"
#include "common/2d/point.hpp"
#include "common/null_iterator.hpp"
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static void ChangeColor(PaletteColors PaletteIndex)
{
	auto NewColor = Global->Opt->Palette[PaletteIndex];

	if (!console.GetColorDialog(NewColor))
		return;

	Global->Opt->Palette.Set(PaletteIndex, { &NewColor, 1 });

	{
		SCOPED_ACTION(LockScreen);

		Global->CtrlObject->Cp()->LeftPanel()->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->LeftPanel()->Redraw();
		Global->CtrlObject->Cp()->RightPanel()->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->RightPanel()->Redraw();

		Global->WindowManager->ResizeAllWindows(); // рефрешим
		Global->WindowManager->PluginCommit(); // коммитим.

		if (Global->Opt->Clock)
			ShowTime();
	}

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
	PaletteColors Color;
	span<const color_item> SubColor;
};

static void SetItemColors(span<const color_item> const Items, point Position = {})
{
	const auto ItemsMenu = VMenu2::create(msg(lng::MSetColorItemsTitle), {});

	for (const auto& i: Items)
	{
		ItemsMenu->AddItem(msg(i.LngId));
	}

	ItemsMenu->SetPosition({ Position.x += 10, Position.y += 5, 0, 0 });
	ItemsMenu->SetMenuFlags(VMENU_WRAPMODE);
	ItemsMenu->RunEx([&](int Msg, void *param)
	{
		const auto ItemsCode = reinterpret_cast<intptr_t>(param);
		if (Msg != DN_CLOSE || ItemsCode < 0)
			return 0;

		if (!Items[ItemsCode].SubColor.empty())
		{
			SetItemColors(Items[ItemsCode].SubColor, Position);
		}
		else
		{
			ChangeColor(Items[ItemsCode].Color);
		}

		return 1;
	});
}

void SetColors()
{
	static const color_item
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
		{ lng::MSetColorDialogListBoxControl,                           {}, ListItemsNormal },
		{ lng::MSetColorDialogComboBoxControl,                          {}, ComboItemsNormal },
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
		{ lng::MSetColorDialogListBoxControl,                           {}, ListItemsWarn },
		{ lng::MSetColorDialogComboBoxControl,                          {}, ComboItemsWarn },
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
			span<const color_item> Subitems;
		}
		Groups[]
		{
			{ lng::MSetColorPanel,       PanelItems },
			{ lng::MSetColorDialog,      DialogItems },
			{ lng::MSetColorWarning,     WarnDialogItems },
			{ lng::MSetColorMenu,        MenuItems },
			{ lng::MSetColorHMenu,       HMenuItems },
			{ lng::MSetColorKeyBar,      KeyBarItems },
			{ lng::MSetColorCommandLine, CommandLineItems },
			{ lng::MSetColorClock,       ClockItems },
			{ lng::MSetColorViewer,      ViewerItems },
			{ lng::MSetColorEditor,      EditorItems },
			{ lng::MSetColorHelp,        HelpItems },
		};

		const auto GroupsMenu = VMenu2::create(msg(lng::MSetColorGroupsTitle), {});

		for (const auto& i: Groups)
		{
			GroupsMenu->AddItem(msg(i.MenuId));
		}

		{
			MenuItemEx tmp;
			tmp.Flags = LIF_SEPARATOR;
			GroupsMenu->AddItem(tmp);
		}

		const auto DefaultId = static_cast<int>(GroupsMenu->size());
		GroupsMenu->AddItem(msg(lng::MSetDefaultColors));
		const auto BlackWhiteId = static_cast<int>(GroupsMenu->size());
		GroupsMenu->AddItem(msg(lng::MSetBW));

		GroupsMenu->SetPosition({ 2, 1, 0, 0 });
		GroupsMenu->SetMenuFlags(VMENU_WRAPMODE);
		const auto GroupsCode=GroupsMenu->RunEx([&](int Msg, void *param)
		{
			const auto ItemsCode = reinterpret_cast<intptr_t>(param);
			if (Msg != DN_CLOSE || ItemsCode < 0 || static_cast<size_t>(ItemsCode) >= std::size(Groups))
				return 0;
			SetItemColors(Groups[ItemsCode].Subitems);
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

static const int ColorIndex[]
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

enum color_dialog_items
{
	cd_border,

	cd_fg_box,
	cd_fg_color_first,
	cd_fg_color_last = cd_fg_color_first + 15,

	cd_fg_colorcode,
	cd_fg_advanced,
	cd_fg_transparent,

	cd_bg_box,
	cd_bg_color_first,
	cd_bg_color_last = cd_bg_color_first + 15,

	cd_bg_colorcode,
	cd_bg_advanced,
	cd_bg_transparent,

	cd_sample_first,
	cd_sample_last = cd_sample_first + 2,

	cd_separator,

	cd_button_ok,
	cd_button_cancel,

	cd_count
};

static string color_code(COLORREF Color)
{
	return format(FSTR(L"{0:06X}"), colors::color_value(colors::ARGB2ABGR(Color)));
}

// BUGBUG
static bool IgnoreEditChange = false;

static intptr_t GetColorDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	const auto ColorState = reinterpret_cast<FarColor*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
	auto& CurColor = ColorState[0];

	const auto GetColor = [Param1](size_t const Offset)
	{
		return colors::ConsoleColorToFarColor(ColorIndex[Param1 - Offset]);
	};

	const auto Flag4Bit = [](bool const IsFg)
	{
		return IsFg? FCF_FG_4BIT : FCF_BG_4BIT;
	};

	const auto SetComponentColorValue = [&CurColor](bool IsFg, COLORREF const Value)
	{
		auto& Component = IsFg? CurColor.ForegroundColor : CurColor.BackgroundColor;
		Component = colors::alpha_value(Component) | colors::color_value(Value);
	};


	const auto DM_UPDATECOLORCODE = DM_USER + 1;

	switch (Msg)
	{
		case DN_CTLCOLORDLGITEM:
			{
				const auto Colors = static_cast<FarDialogItemColors*>(Param2);
				if (Param1 >= cd_fg_color_first && Param1 <= cd_fg_color_last)
				{
					Colors->Colors[0] = GetColor(cd_fg_color_first);
					return TRUE;
				}
				else if (Param1 >= cd_bg_color_first && Param1 <= cd_bg_color_last)
				{
					Colors->Colors[0] = GetColor(cd_bg_color_first);
					return TRUE;
				}
				else if (Param1 >= cd_sample_first && Param1 <= cd_sample_last)
				{
					Colors->Colors[0] = colors::merge(ColorState[1], ColorState[0]);
					return TRUE;
				}
				else
					return FALSE;
			}

		case DN_BTNCLICK:
			{
				if (Param2 && ((Param1 >= cd_fg_color_first && Param1 <= cd_fg_color_last) || (Param1 >= cd_bg_color_first && Param1 <= cd_bg_color_last)))
				{
					const auto IsFg = Param1 >= cd_fg_color_first && Param1 <= cd_fg_color_last;
					const auto First = IsFg? cd_fg_color_first : cd_bg_color_first;
					const auto ChosenColor = GetColor(First).BackgroundColor;

					SetComponentColorValue(IsFg, ChosenColor);
					CurColor.Flags |= Flag4Bit(IsFg);

					Dlg->SendMessage(DM_UPDATECOLORCODE, IsFg? cd_fg_colorcode : cd_bg_colorcode, ToPtr(colors::ConsoleIndexToTrueColor(ChosenColor)));

					return TRUE;
				}
				else if (Param1 == cd_fg_transparent || Param1 == cd_bg_transparent)
				{
					const auto IsFg = Param1 == cd_fg_transparent;
					auto& Component = IsFg? CurColor.ForegroundColor : CurColor.BackgroundColor;
					Param2? colors::make_transparent(Component) : colors::make_opaque(Component);
				}
				else if (Param1 == cd_fg_advanced || Param1 == cd_bg_advanced)
				{
					const auto IsFg = Param1 == cd_fg_advanced;
					auto CustomColors = Global->Opt->Palette.GetCustomColors();

					CHOOSECOLOR Params{sizeof(Params)};
					Params.hwndOwner = console.GetWindow();

					Params.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
					Params.lpCustColors = CustomColors.data();

					auto& Component = IsFg? CurColor.ForegroundColor : CurColor.BackgroundColor;

					Params.rgbResult = colors::color_value(CurColor.Flags & Flag4Bit(IsFg)?
						colors::ConsoleIndexToTrueColor(Component) :
						Component
					);

					if (ChooseColor(&Params))
					{
						SetComponentColorValue(IsFg, Params.rgbResult);
						CurColor.Flags &= ~Flag4Bit(IsFg);

						Dlg->SendMessage(DM_SETCHECK, IsFg? cd_fg_color_first : cd_bg_color_first, ToPtr(BSTATE_3STATE));
						Dlg->SendMessage(DM_UPDATECOLORCODE, IsFg? cd_fg_colorcode : cd_bg_colorcode, ToPtr(Params.rgbResult));

						Global->Opt->Palette.SetCustomColors(CustomColors);
					}

					return TRUE;
				}
			}
			break;

		case DN_EDITCHANGE:
			if (!IgnoreEditChange && (Param1 == cd_fg_colorcode || Param1 == cd_bg_colorcode))
			{
				const auto& Item = *static_cast<const FarDialogItem*>(Param2);
				const auto Iterator = null_iterator(Item.Data);
				if (std::any_of(Iterator, Iterator.end(), std::iswxdigit))
				{
					const auto IsFg = Param1 == cd_fg_colorcode;
					const auto ChosenColor = colors::ARGB2ABGR(std::wcstoul(Item.Data, nullptr, 16));

					SetComponentColorValue(IsFg, ChosenColor);
					CurColor.Flags &= ~Flag4Bit(IsFg);

					Dlg->SendMessage(DM_SETCHECK, IsFg? cd_fg_color_first : cd_bg_color_first, ToPtr(BSTATE_3STATE));
				}
			}
			return TRUE;

		case DM_UPDATECOLORCODE:
			{
				IgnoreEditChange = true;
				SCOPE_EXIT{ IgnoreEditChange = false; };
				Dlg->SendMessage(DM_SETTEXTPTR, Param1, UNSAFE_CSTR(color_code(static_cast<int>(reinterpret_cast<intptr_t>(Param2)))));
			}
			return TRUE;

		default:
			break;
	}

	return Dlg->DefProc(Msg, Param1, Param2);
}

bool GetColorDialogInternal(FarColor& Color, bool const bCentered, const FarColor* const BaseColor)
{
	auto ColorDlg = MakeDialogItems<cd_count>(
	{
		{ DI_DOUBLEBOX,   {{3,  1 }, {35, 14}}, DIF_NONE, msg(lng::MSetColorTitle), },
		{ DI_SINGLEBOX,   {{5,  2 }, {18, 7 }}, DIF_NONE, msg(lng::MSetColorForeground), },
		{ DI_RADIOBUTTON, {{6,  3 }, {0,  3 }}, DIF_MOVESELECT | DIF_GROUP,},
		{ DI_RADIOBUTTON, {{6,  4 }, {0,  4 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{6,  5 }, {0,  5 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{6,  6 }, {0,  6 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{9,  3 }, {0,  3 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{9,  4 }, {0,  4 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{9,  5 }, {0,  5 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{9,  6 }, {0,  6 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{12, 3 }, {0,  3 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{12, 4 }, {0,  4 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{12, 5 }, {0,  5 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{12, 6 }, {0,  6 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{15, 3 }, {0,  3 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{15, 4 }, {0,  4 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{15, 5 }, {0,  5 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{15, 6 }, {0,  6 }}, DIF_MOVESELECT, },
		{ DI_FIXEDIT,     {{5,  8 }, {10, 8 }}, DIF_MASKEDIT, },
		{ DI_BUTTON,      {{12, 8 }, {18, 8 }}, DIF_NONE, msg(lng::MSetColorForeRGB), },
		{ DI_CHECKBOX,    {{5,  9 }, {0,  9 }}, DIF_NONE, msg(lng::MSetColorForeTransparent), },
		{ DI_SINGLEBOX,   {{20, 2 }, {33, 7 }}, DIF_NONE, msg(lng::MSetColorBackground), },
		{ DI_RADIOBUTTON, {{21, 3 }, {0,  3 }}, DIF_MOVESELECT | DIF_GROUP, },
		{ DI_RADIOBUTTON, {{21, 4 }, {0,  4 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{21, 5 }, {0,  5 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{21, 6 }, {0,  6 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{24, 3 }, {0,  3 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{24, 4 }, {0,  4 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{24, 5 }, {0,  5 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{24, 6 }, {0,  6 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{27, 3 }, {0,  3 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{27, 4 }, {0,  4 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{27, 5 }, {0,  5 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{27, 6 }, {0,  6 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{30, 3 }, {0,  3 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{30, 4 }, {0,  4 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{30, 5 }, {0,  5 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{30, 6 }, {0,  6 }}, DIF_MOVESELECT, },
		{ DI_FIXEDIT,     {{20, 8 }, {25, 8 }}, DIF_MASKEDIT, },
		{ DI_BUTTON,      {{27, 8 }, {33, 8 }}, DIF_NONE, msg(lng::MSetColorBackRGB), },
		{ DI_CHECKBOX,    {{22, 9 }, {0,  9 }}, DIF_NONE, msg(lng::MSetColorBackTransparent), },
		{ DI_TEXT,        {{5,  9 }, {33, 9 }}, DIF_NONE, msg(lng::MSetColorSample), },
		{ DI_TEXT,        {{5,  10}, {33, 10}}, DIF_NONE, msg(lng::MSetColorSample), },
		{ DI_TEXT,        {{5,  11}, {33, 11}}, DIF_NONE, msg(lng::MSetColorSample), },
		{ DI_TEXT,        {{-1, 12}, {0,  12}}, DIF_SEPARATOR, },
		{ DI_BUTTON,      {{0,  13}, {0,  13}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MSetColorSet), },
		{ DI_BUTTON,      {{0,  13}, {0,  13}}, DIF_CENTERGROUP, msg(lng::MSetColorCancel), },
	});

	ColorDlg[cd_fg_colorcode].strData = color_code(Color.IsFg4Bit()? colors::ConsoleIndexToTrueColor(Color.ForegroundColor) : Color.ForegroundColor);
	ColorDlg[cd_bg_colorcode].strData = color_code(Color.IsBg4Bit()? colors::ConsoleIndexToTrueColor(Color.BackgroundColor) : Color.BackgroundColor);
	ColorDlg[cd_fg_colorcode].strMask = ColorDlg[cd_bg_colorcode].strMask = L"HHHHHH"sv;

	FarColor CurColor[]{ Color, BaseColor? *BaseColor : colors::ConsoleColorToFarColor(F_BLACK | B_BLACK) };

	if (Color.IsFg4Bit() || Color.IsBg4Bit())
	{
		const auto ConsoleColor = colors::FarColorToConsoleColor(Color);

		if (Color.IsFg4Bit())
		{
			for (size_t i = cd_fg_color_first; i <= cd_fg_color_last; ++i)
			{
				if (((ColorIndex[i - cd_fg_color_first] & B_MASK) >> 4) == (ConsoleColor & F_MASK))
				{
					ColorDlg[i].Selected = true;
					ColorDlg[i].Flags |= DIF_FOCUS;
					break;
				}
			}
		}

		if (Color.IsBg4Bit())
		{
			for (size_t i = cd_bg_color_first; i <= cd_bg_color_last; ++i)
			{
				if ((ColorIndex[i - cd_bg_color_first] & B_MASK) == (ConsoleColor & B_MASK))
				{
					ColorDlg[i].Selected = true;
					break;
				}
			}
		}
	}
	else
	{
		ColorDlg[cd_fg_colorcode].Flags |= DIF_FOCUS;
	}

	if (BaseColor)
	{
		ColorDlg[0].Y2++;

		for (size_t i = cd_sample_first; i < cd_count; ++i)
		{
			ColorDlg[i].Y1+=1;
			ColorDlg[i].Y2+=1;
		}

		ColorDlg[cd_border].X2 += 4;
		ColorDlg[cd_fg_box].X2 += 2;
		ColorDlg[cd_bg_box].X1 += 2;
		ColorDlg[cd_bg_box].X2 += 4;

		for (size_t i = cd_fg_color_first; i <= cd_fg_color_last; ++i)
		{
			ColorDlg[i].X1+=1;
		}

		for (size_t i = cd_bg_color_first; i <= cd_bg_color_last; ++i)
		{
			ColorDlg[i].X1+=3;
		}

		for (size_t i = cd_sample_first; i <= cd_sample_last; ++i)
		{
			ColorDlg[i].X2+=4;
		}

		ColorDlg[cd_bg_advanced].X1 += 2;
		ColorDlg[cd_bg_advanced].X2 += 2;

		ColorDlg[cd_bg_colorcode].X1 += 2;
		ColorDlg[cd_bg_colorcode].X2 += 2;

		ColorDlg[cd_fg_transparent].Selected = colors::is_transparent(Color.ForegroundColor);
		ColorDlg[cd_bg_transparent].Selected = colors::is_transparent(Color.BackgroundColor);
	}
	else
	{
		ColorDlg[cd_fg_transparent].Flags|=DIF_HIDDEN;
		ColorDlg[cd_bg_transparent].Flags|=DIF_HIDDEN;
	}

	const auto Dlg = Dialog::create(ColorDlg, GetColorDlgProc, CurColor);

	if (bCentered)
		Dlg->SetPosition({ -1, -1, 39 + (BaseColor? 4 : 0), 16 + (BaseColor? 1 : 0) });
	else
		Dlg->SetPosition({ 37, 2, 75 + (BaseColor? 4 : 0), 17 + (BaseColor? 1 : 0)});

	Dlg->Process();
	if (Dlg->GetExitCode() != cd_button_ok)
		return false;

	Color = CurColor[0];
	return true;
}
