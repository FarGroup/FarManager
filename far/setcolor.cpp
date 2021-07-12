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
#include "common/from_string.hpp"
#include "common/null_iterator.hpp"
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static void ChangeColor(PaletteColors PaletteIndex, PaletteColors const* const BottomPaletteIndex)
{
	auto NewColor = Global->Opt->Palette[PaletteIndex];
	const auto BottomColor = BottomPaletteIndex? &Global->Opt->Palette[*BottomPaletteIndex] : nullptr;

	if (!console.GetColorDialog(NewColor, false, BottomColor))
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
	PaletteColors const* BottomColor;
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
			ChangeColor(Items[ItemsCode].Color, Items[ItemsCode].BottomColor);
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
		{ lng::MSetColorEditorSelected,               COL_EDITORSELECTEDTEXT, {}, &EditorItems[0].Color },
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
	distinct(B_DARKGRAY),
	distinct(B_BLUE),
	distinct(B_LIGHTBLUE),
	distinct(B_GREEN),
	distinct(B_LIGHTGREEN),
	distinct(B_CYAN),
	distinct(B_LIGHTCYAN),
	distinct(B_RED),
	distinct(B_LIGHTRED),
	distinct(B_MAGENTA),
	distinct(B_LIGHTMAGENTA),
	distinct(B_BROWN),
	distinct(B_YELLOW),
	distinct(B_LIGHTGRAY),
	distinct(B_WHITE)
};

enum color_dialog_items
{
	cd_border,

	cd_separator1,
	cd_separator2,
	cd_separator3,
	cd_separator4,

	cd_fg_text,
	cd_fg_active,
	cd_fg_color_first,
	cd_fg_color_last = cd_fg_color_first + 15,

	cd_fg_colorcode_title,
	cd_fg_colorcode,
	cd_fg_advanced,

	cd_bg_text,
	cd_bg_active,
	cd_bg_color_first,
	cd_bg_color_last = cd_bg_color_first + 15,

	cd_bg_colorcode_title,
	cd_bg_colorcode,
	cd_bg_advanced,

	cd_style,
	cd_style_first,
	cd_style_inherit = cd_style_first,
	cd_style_bold,
	cd_style_italic,
	cd_style_underline,
	cd_style_underline2,
	cd_style_overline,
	cd_style_strikeout,
	cd_style_faint,
	cd_style_blink,
	cd_style_last = cd_style_blink,

	cd_sample_first,
	cd_sample_last = cd_sample_first + 2,

	cd_button_ok,
	cd_button_cancel,

	cd_count
};

static string color_code(COLORREF const Color, bool const Is4Bit)
{
	return Is4Bit?
		format(FSTR(L"{:02X}     {:X}"sv), colors::alpha_value(Color), colors::index_value(Color)) :
		format(FSTR(L"{:08X}"sv), colors::ARGB2ABGR(Color));
}

static std::optional<COLORREF> parse_color(string_view const Str, bool const Is4Bit)
{
	if (Is4Bit)
	{
		unsigned Alpha;
		if (!from_string(Str.substr(0, 2), Alpha, {}, 16))
			return {};

		unsigned Index;
		if (!from_string(Str.substr(7, 1), Index, {}, 16))
			return {};

		return Alpha << 24 | Index;
	}
	else
	{
		unsigned Value;
		if (!from_string(Str, Value, {}, 16))
			return {};

		return colors::ARGB2ABGR(Value);
	}
}

// BUGBUG
static bool IgnoreEditChange = false;
static bool IgnoreColorIndexClick = false;

static const std::pair<color_dialog_items, FARCOLORFLAGS> StyleMapping[]
{
	{ cd_style_inherit,     FCF_INHERIT_STYLE  },
	{ cd_style_bold,        FCF_FG_BOLD        },
	{ cd_style_italic,      FCF_FG_ITALIC      },
	{ cd_style_underline,   FCF_FG_UNDERLINE   },
	{ cd_style_underline2,  FCF_FG_UNDERLINE2  },
	{ cd_style_overline,    FCF_FG_OVERLINE    },
	{ cd_style_strikeout,   FCF_FG_STRIKEOUT   },
	{ cd_style_faint,       FCF_FG_FAINT       },
	{ cd_style_blink,       FCF_FG_BLINK       },
};

struct color_state
{
	FarColor CurColor, BaseColor;
	bool TransparencyEnabled;
};

constexpr auto
	MaskIndex = L"HH     H"sv,
	MaskARGB  = L"HHHHHHHH"sv;

static intptr_t GetColorDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	auto& ColorState = *reinterpret_cast<color_state*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
	auto& CurColor = ColorState.CurColor;

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
		Component = colors::alpha_bits(Component) | colors::color_bits(Value);
	};


	const auto DM_UPDATECOLORCODE = DM_USER + 1;

	switch (Msg)
	{
		case DN_INITDIALOG:
			Dlg->SendMessage(DM_EDITUNCHANGEDFLAG, cd_fg_colorcode, {});
			Dlg->SendMessage(DM_EDITUNCHANGEDFLAG, cd_bg_colorcode, {});
			break;

		case DN_CTLCOLORDLGITEM:
			{
				const auto preview_or_disabled = [&](COLORREF const Color, size_t const Index)
				{
					return ColorState.TransparencyEnabled && colors::is_transparent(Color)?
						colors::PaletteColorToFarColor(COL_DIALOGDISABLED) :
						GetColor(Index);
				};

				const auto Colors = static_cast<FarDialogItemColors*>(Param2);
				if (Param1 >= cd_fg_color_first && Param1 <= cd_fg_color_last)
				{
					Colors->Colors[0] = preview_or_disabled(CurColor.ForegroundColor, cd_fg_color_first);
					return TRUE;
				}
				else if (Param1 >= cd_bg_color_first && Param1 <= cd_bg_color_last)
				{
					Colors->Colors[0] = preview_or_disabled(CurColor.BackgroundColor, cd_bg_color_first);
					return TRUE;
				}
				else if (Param1 >= cd_sample_first && Param1 <= cd_sample_last)
				{
					Colors->Colors[0] = colors::merge(ColorState.BaseColor, ColorState.CurColor);
					return TRUE;
				}
				else
					return FALSE;
			}

		case DN_BTNCLICK:
			{
				if (Param2 && ((Param1 >= cd_fg_color_first && Param1 <= cd_fg_color_last) || (Param1 >= cd_bg_color_first && Param1 <= cd_bg_color_last)))
				{
					if (IgnoreColorIndexClick)
						return true;

					const auto IsFg = Param1 >= cd_fg_color_first && Param1 <= cd_fg_color_last;
					const auto First = IsFg? cd_fg_color_first : cd_bg_color_first;

					auto& Component = IsFg? CurColor.ForegroundColor : CurColor.BackgroundColor;
					Component = GetColor(First).BackgroundColor;

					CurColor.Flags |= Flag4Bit(IsFg);

					Dlg->SendMessage(DM_UPDATECOLORCODE, IsFg? cd_fg_colorcode : cd_bg_colorcode, {});

					return TRUE;
				}
				else if (Param1 >= cd_style_first && Param1 <= cd_style_last)
				{
					flags::change(CurColor.Flags, StyleMapping[Param1 - cd_style_first].second, Param2 != nullptr);
				}
				else if (Param1 == cd_fg_active || Param1 == cd_bg_active)
				{
					const auto IsFg = Param1 == cd_fg_active;
					auto& Component = IsFg? CurColor.ForegroundColor : CurColor.BackgroundColor;
					Param2? colors::make_opaque(Component) : colors::make_transparent(Component);

					SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);
					const auto Offset = IsFg? cd_fg_color_first : cd_bg_color_first;
					for (auto i = 0; i != 16; ++i)
					{
						Dlg->SendMessage(DM_ENABLE, i + Offset, Param2);
					}

					Dlg->SendMessage(DM_UPDATECOLORCODE, IsFg? cd_fg_colorcode : cd_bg_colorcode, {});

					Dlg->SendMessage(DM_ENABLE, IsFg? cd_fg_colorcode : cd_bg_colorcode, Param2);
					Dlg->SendMessage(DM_ENABLE, IsFg? cd_fg_advanced : cd_bg_advanced, Param2);
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
						Dlg->SendMessage(DM_UPDATECOLORCODE, IsFg? cd_fg_colorcode : cd_bg_colorcode, {});

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
					const auto Is4Bit = IsFg? CurColor.IsFg4Bit() : CurColor.IsBg4Bit();
					auto& Component = IsFg? CurColor.ForegroundColor : CurColor.BackgroundColor;

					const auto ParsedColor = parse_color(Item.Data, Is4Bit);
					if (!ParsedColor)
						return false;

					const auto OldColorIndex = colors::index_value(Component);
					Component = *ParsedColor;

					if (Is4Bit)
					{
						const auto NewColorIndex = colors::index_value(Component);

						if (NewColorIndex != OldColorIndex)
						{
							IgnoreColorIndexClick = true;
							SCOPE_EXIT{ IgnoreColorIndexClick = false; };
							const auto ColorButtonIndex = NewColorIndex < 8? NewColorIndex * 2 : (NewColorIndex - 8) * 2 + 1;
							Dlg->SendMessage(DM_SETCHECK, (IsFg? cd_fg_color_first : cd_bg_color_first) + ColorButtonIndex, ToPtr(BSTATE_CHECKED));
						}
					}

					if (ColorState.TransparencyEnabled && colors::is_transparent(Component))
					{
						Dlg->SendMessage(DM_SETCHECK, IsFg? cd_fg_active : cd_bg_active, ToPtr(BSTATE_UNCHECKED));
						Dlg->SendMessage(DN_BTNCLICK, IsFg? cd_fg_active : cd_bg_active, ToPtr(BSTATE_UNCHECKED));
					}
				}
			}
			return TRUE;

		case DM_UPDATECOLORCODE:
			{
				IgnoreEditChange = true;
				SCOPE_EXIT{ IgnoreEditChange = false; };
				const auto IsFg = Param1 == cd_fg_colorcode;

				FarDialogItem Item;
				if (!Dlg->SendMessage(DM_GETDLGITEMSHORT, Param1, &Item))
					return false;

				const auto Color = IsFg? CurColor.ForegroundColor : CurColor.BackgroundColor;
				const auto Is4Bit = IsFg? CurColor.IsFg4Bit() : CurColor.IsBg4Bit();
				const auto Value = color_code(Color, Is4Bit);

				Item.Mask = (Is4Bit? MaskIndex :MaskARGB).data();
				Item.Data = UNSAFE_CSTR(Value);

				Dlg->SendMessage(DM_SETDLGITEM, Param1, &Item);

				constexpr lng Titles[][2]
				{
					{ lng::MSetColorForeIndex, lng::MSetColorForeAARRGGBB },
					{ lng::MSetColorBackIndex, lng::MSetColorBackAARRGGBB },
				};

				Dlg->SendMessage(DM_SETTEXTPTR,
					IsFg? cd_fg_colorcode_title : cd_bg_colorcode_title,
					UNSAFE_CSTR(msg(Titles[IsFg? 0 : 1][Is4Bit? 0 : 1]))
				);
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
		{ DI_DOUBLEBOX,   {{3,  1 }, {62, 15}}, DIF_NONE, msg(lng::MSetColorTitle), },

		{ DI_TEXT,        {{-1, 13}, {0,  13}}, DIF_SEPARATOR, },
		{ DI_VTEXT,       {{39, 1 }, {39, 13}}, DIF_SEPARATORUSER, },
		{ DI_TEXT,        {{3,  5 }, {39,  5}}, DIF_SEPARATORUSER, },
		{ DI_TEXT,        {{3,  9 }, {39, 9 }}, DIF_SEPARATORUSER, },

		{ DI_TEXT,        {{5,  2 }, {0,  2 }}, DIF_NONE, msg(lng::MSetColorForeground), },
		{ DI_CHECKBOX,    {{5,  2 }, {0,  2 }}, DIF_NONE, msg(lng::MSetColorForeground), },

		{ DI_RADIOBUTTON, {{5,  3 }, {0,  3 }}, DIF_MOVESELECT | DIF_GROUP,},
		{ DI_RADIOBUTTON, {{5,  4 }, {0,  4 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{8,  3 }, {0,  3 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{8,  4 }, {0,  4 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{11, 3 }, {0,  3 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{11, 4 }, {0,  4 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{14, 3 }, {0,  3 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{14, 4 }, {0,  4 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{17, 3 }, {0,  3 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{17, 4 }, {0,  4 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{20, 3 }, {0,  3 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{20, 4 }, {0,  4 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{23, 3 }, {0,  3 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{23, 4 }, {0,  4 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{26, 3 }, {0,  3 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{26, 4 }, {0,  4 }}, DIF_MOVESELECT, },

		{ DI_TEXT,        {{30, 2 }, {0,  2 }}, DIF_NONE, msg(Color.IsFg4Bit()? lng::MSetColorForeIndex : lng::MSetColorForeAARRGGBB) },
		{ DI_FIXEDIT,     {{30, 3 }, {37, 3 }}, DIF_MASKEDIT, },
		{ DI_BUTTON,      {{30, 4 }, {37, 4 }}, DIF_NONE, msg(lng::MSetColorForeRGB), },

		{ DI_TEXT,        {{5,  6 }, {0,  6 }}, DIF_NONE, msg(lng::MSetColorBackground), },
		{ DI_CHECKBOX,    {{5,  6 }, {0,  6 }}, DIF_NONE, msg(lng::MSetColorBackground), },

		{ DI_RADIOBUTTON, {{5,  7 }, {0,  7 }}, DIF_MOVESELECT | DIF_GROUP, },
		{ DI_RADIOBUTTON, {{5,  8 }, {0,  8 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{8,  7 }, {0,  7 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{8,  8 }, {0,  8 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{11, 7 }, {0,  7 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{11, 8 }, {0,  8 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{14, 7 }, {0,  7 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{14, 8 }, {0,  8 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{17, 7 }, {0,  7 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{17, 8 }, {0,  8 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{20, 7 }, {0,  7 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{20, 8 }, {0,  8 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{23, 7 }, {0,  7 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{23, 8 }, {0,  8 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{26, 7 }, {0,  7 }}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{26, 8 }, {0,  8 }}, DIF_MOVESELECT, },

		{ DI_TEXT,        {{30, 6 }, {0,  6 }}, DIF_NONE, msg(Color.IsBg4Bit()? lng::MSetColorBackIndex : lng::MSetColorBackAARRGGBB) },
		{ DI_FIXEDIT,     {{30, 7 }, {37, 7 }}, DIF_MASKEDIT, },
		{ DI_BUTTON,      {{30, 8 }, {37, 8 }}, DIF_NONE, msg(lng::MSetColorBackRGB), },

		{ DI_TEXT,        {{41, 2 }, {0,  2 }}, DIF_NONE, msg(lng::MSetColorStyle), },
		{ DI_CHECKBOX,    {{41, 3 }, {0,  3 }}, DIF_NONE, msg(lng::MSetColorStyleInherit), },
		{ DI_CHECKBOX,    {{41, 4 }, {0,  4 }}, DIF_NONE, msg(lng::MSetColorStyleBold), },
		{ DI_CHECKBOX,    {{41, 5 }, {0,  5 }}, DIF_NONE, msg(lng::MSetColorStyleItalic), },
		{ DI_CHECKBOX,    {{41, 6 }, {0,  6 }}, DIF_NONE, msg(lng::MSetColorStyleUnderline), },
		{ DI_CHECKBOX,    {{41, 7 }, {0,  7 }}, DIF_NONE, msg(lng::MSetColorStyleUnderline2), },
		{ DI_CHECKBOX,    {{41, 8 }, {0,  8 }}, DIF_NONE, msg(lng::MSetColorStyleOverline), },
		{ DI_CHECKBOX,    {{41, 9 }, {0,  9 }}, DIF_NONE, msg(lng::MSetColorStyleStrikeout), },
		{ DI_CHECKBOX,    {{41, 10}, {0,  10}}, DIF_NONE, msg(lng::MSetColorStyleFaint), },
		{ DI_CHECKBOX,    {{41, 11}, {0,  11}}, DIF_NONE, msg(lng::MSetColorStyleBlink), },

		{ DI_TEXT,        {{5,  10}, {37, 10}}, DIF_NONE, msg(lng::MSetColorSample), },
		{ DI_TEXT,        {{5,  11}, {37, 11}}, DIF_NONE, msg(lng::MSetColorSample), },
		{ DI_TEXT,        {{5,  12}, {37, 12}}, DIF_NONE, msg(lng::MSetColorSample), },

		{ DI_BUTTON,      {{0,  14}, {0,  14}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MSetColorSet), },
		{ DI_BUTTON,      {{0,  14}, {0,  14}}, DIF_CENTERGROUP, msg(lng::MSetColorCancel), },
	});

	ColorDlg[cd_separator2].strMask = { BoxSymbols[BS_T_H2V1], BoxSymbols[BS_V1], BoxSymbols[BS_B_H1V1] };
	ColorDlg[cd_separator3].strMask = ColorDlg[cd_separator4].strMask = { BoxSymbols[BS_L_H1V2], BoxSymbols[BS_H1], BoxSymbols[BS_R_H1V1] };

	ColorDlg[cd_fg_colorcode].strData = color_code(Color.ForegroundColor, Color.IsFg4Bit());
	ColorDlg[cd_bg_colorcode].strData = color_code(Color.BackgroundColor, Color.IsBg4Bit());
	ColorDlg[cd_fg_colorcode].strMask = Color.IsFg4Bit()? MaskIndex : MaskARGB;
	ColorDlg[cd_bg_colorcode].strMask = Color.IsBg4Bit()? MaskIndex : MaskARGB;

	color_state ColorState
	{
		Color,
		BaseColor? *BaseColor : colors::ConsoleColorToFarColor(F_BLACK | B_BLACK),
		BaseColor != nullptr
	};

	if (Color.IsFg4Bit() || Color.IsBg4Bit())
	{
		const auto ConsoleColor = colors::FarColorToConsoleColor(Color);

		if (Color.IsFg4Bit())
		{
			const auto ColorPart = ConsoleColor & F_MASK;
			for (size_t i = cd_fg_color_first; i <= cd_fg_color_last; ++i)
			{
				if (((ColorIndex[i - cd_fg_color_first] & B_MASK) >> 4) == ColorPart)
				{
					ColorDlg[i].Selected = true;
					ColorDlg[i].Flags |= DIF_FOCUS;
					break;
				}
			}
		}

		if (Color.IsBg4Bit())
		{
			const auto ColorPart = ConsoleColor & B_MASK;
			for (size_t i = cd_bg_color_first; i <= cd_bg_color_last; ++i)
			{
				if ((ColorIndex[i - cd_bg_color_first] & B_MASK) == ColorPart)
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

	for (const auto& [Index, Flag]: StyleMapping)
	{
		ColorDlg[Index].Selected = Color.Flags & Flag? BSTATE_CHECKED : BSTATE_UNCHECKED;
	}

	if (BaseColor)
	{
		ColorDlg[cd_fg_text].Flags |= DIF_HIDDEN;
		ColorDlg[cd_bg_text].Flags |= DIF_HIDDEN;

		if (colors::is_transparent(Color.ForegroundColor))
		{
			for (size_t i = cd_fg_color_first; i <= cd_fg_color_last; ++i)
			{
				ColorDlg[i].Flags |= DIF_DISABLE;
			}

			ColorDlg[cd_fg_colorcode].Flags |= DIF_DISABLE;
			ColorDlg[cd_fg_advanced].Flags |= DIF_DISABLE;
		}
		else
		{
			ColorDlg[cd_fg_active].Selected = BSTATE_CHECKED;
		}

		if (colors::is_transparent(Color.BackgroundColor))
		{
			for (size_t i = cd_bg_color_first; i <= cd_bg_color_last; ++i)
			{
				ColorDlg[i].Flags |= DIF_DISABLE;
			}

			ColorDlg[cd_bg_colorcode].Flags |= DIF_DISABLE;
			ColorDlg[cd_bg_advanced].Flags |= DIF_DISABLE;
		}
		else
		{
			ColorDlg[cd_bg_active].Selected = BSTATE_CHECKED;
		}

	}
	else
	{
		ColorDlg[cd_fg_active].Flags|=DIF_HIDDEN;
		ColorDlg[cd_bg_active].Flags|=DIF_HIDDEN;
	}

	const auto Dlg = Dialog::create(ColorDlg, GetColorDlgProc, &ColorState);

	const auto
		DlgWidth = static_cast<int>(ColorDlg[cd_border].X2) + 4,
		DlgHeight = static_cast<int>(ColorDlg[cd_border].Y2) + 2;

	if (bCentered)
		Dlg->SetPosition({ -1, -1, DlgWidth, DlgHeight });
	else
	{
		constexpr auto
			DlgLeft = 37,
			DlgTop = 2;

		Dlg->SetPosition({DlgLeft, DlgTop, DlgLeft + DlgWidth - 1, DlgTop + DlgHeight - 1 });
	}

	Dlg->Process();
	if (Dlg->GetExitCode() != cd_button_ok)
		return false;

	Color = ColorState.CurColor;
	return true;
}
