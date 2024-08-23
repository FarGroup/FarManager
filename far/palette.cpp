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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "palette.hpp"

// Internal:
#include "farcolor.hpp"
#include "colormix.hpp"
#include "configdb.hpp"
#include "plugin.hpp"

// Platform:

// Common:
#include "common/function_ref.hpp"
#include "common/view/enumerate.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static const struct ColorsInit
{
	string_view Name;
	unsigned char Index;

	static constexpr unsigned char Default = 0xFF;
}
Init[]
{
	{L"Menu.Text"sv,                                   F_WHITE|B_CYAN,         }, // COL_MENUTEXT,
	{L"Menu.Text.Selected"sv,                          F_WHITE|B_BLACK,        }, // COL_MENUSELECTEDTEXT,
	{L"Menu.Highlight"sv,                              F_YELLOW|B_CYAN,        }, // COL_MENUHIGHLIGHT,
	{L"Menu.Highlight.Selected"sv,                     F_YELLOW|B_BLACK,       }, // COL_MENUSELECTEDHIGHLIGHT,
	{L"Menu.Box"sv,                                    F_WHITE|B_CYAN,         }, // COL_MENUBOX,
	{L"Menu.Title"sv,                                  F_WHITE|B_CYAN,         }, // COL_MENUTITLE,
	{L"HMenu.Text"sv,                                  F_BLACK|B_CYAN,         }, // COL_HMENUTEXT,
	{L"HMenu.Text.Selected"sv,                         F_WHITE|B_BLACK,        }, // COL_HMENUSELECTEDTEXT,
	{L"HMenu.Highlight"sv,                             F_YELLOW|B_CYAN,        }, // COL_HMENUHIGHLIGHT,
	{L"HMenu.Highlight.Selected"sv,                    F_YELLOW|B_BLACK,       }, // COL_HMENUSELECTEDHIGHLIGHT,
	{L"Panel.Text"sv,                                  F_LIGHTCYAN|B_BLUE,     }, // COL_PANELTEXT,
	{L"Panel.Text.Selected"sv,                         F_YELLOW|B_BLUE,        }, // COL_PANELSELECTEDTEXT,
	{L"Panel.Text.Highlight"sv,                        F_LIGHTGRAY|B_BLUE,     }, // COL_PANELHIGHLIGHTTEXT,
	{L"Panel.Text.Info"sv,                             F_YELLOW|B_BLUE,        }, // COL_PANELINFOTEXT,
	{L"Panel.Cursor"sv,                                F_BLACK|B_CYAN,         }, // COL_PANELCURSOR,
	{L"Panel.Cursor.Selected"sv,                       F_YELLOW|B_CYAN,        }, // COL_PANELSELECTEDCURSOR,
	{L"Panel.Title"sv,                                 F_LIGHTCYAN|B_BLUE,     }, // COL_PANELTITLE,
	{L"Panel.Title.Selected"sv,                        F_BLACK|B_CYAN,         }, // COL_PANELSELECTEDTITLE,
	{L"Panel.Title.Column"sv,                          F_YELLOW|B_BLUE,        }, // COL_PANELCOLUMNTITLE,
	{L"Panel.Info.Total"sv,                            F_LIGHTCYAN|B_BLUE,     }, // COL_PANELTOTALINFO,
	{L"Panel.Info.Selected"sv,                         F_YELLOW|B_CYAN,        }, // COL_PANELSELECTEDINFO,
	{L"Dialog.Text"sv,                                 F_BLACK|B_LIGHTGRAY,    }, // COL_DIALOGTEXT,
	{L"Dialog.Text.Highlight"sv,                       F_YELLOW|B_LIGHTGRAY,   }, // COL_DIALOGHIGHLIGHTTEXT,
	{L"Dialog.Box"sv,                                  F_BLACK|B_LIGHTGRAY,    }, // COL_DIALOGBOX,
	{L"Dialog.Box.Title"sv,                            F_BLACK|B_LIGHTGRAY,    }, // COL_DIALOGBOXTITLE,
	{L"Dialog.Box.Title.Highlight"sv,                  F_YELLOW|B_LIGHTGRAY,   }, // COL_DIALOGHIGHLIGHTBOXTITLE,
	{L"Dialog.Edit"sv,                                 F_BLACK|B_CYAN,         }, // COL_DIALOGEDIT,
	{L"Dialog.Button"sv,                               F_BLACK|B_LIGHTGRAY,    }, // COL_DIALOGBUTTON,
	{L"Dialog.Button.Selected"sv,                      F_BLACK|B_CYAN,         }, // COL_DIALOGSELECTEDBUTTON,
	{L"Dialog.Button.Highlight"sv,                     F_YELLOW|B_LIGHTGRAY,   }, // COL_DIALOGHIGHLIGHTBUTTON,
	{L"Dialog.Button.Highlight.Selected"sv,            F_YELLOW|B_CYAN,        }, // COL_DIALOGHIGHLIGHTSELECTEDBUTTON,
	{L"Dialog.List.Text"sv,                            F_BLACK|B_LIGHTGRAY,    }, // COL_DIALOGLISTTEXT,
	{L"Dialog.List.Text.Selected"sv,                   F_WHITE|B_BLACK,        }, // COL_DIALOGLISTSELECTEDTEXT,
	{L"Dialog.List.Highlight"sv,                       F_YELLOW|B_LIGHTGRAY,   }, // COL_DIALOGLISTHIGHLIGHT,
	{L"Dialog.List.Highlight.Selected"sv,              F_YELLOW|B_BLACK,       }, // COL_DIALOGLISTSELECTEDHIGHLIGHT,
	{L"WarnDialog.Text"sv,                             F_WHITE|B_RED,          }, // COL_WARNDIALOGTEXT,
	{L"WarnDialog.Text.Highlight"sv,                   F_YELLOW|B_RED,         }, // COL_WARNDIALOGHIGHLIGHTTEXT,
	{L"WarnDialog.Box"sv,                              F_WHITE|B_RED,          }, // COL_WARNDIALOGBOX,
	{L"WarnDialog.Box.Title"sv,                        F_WHITE|B_RED,          }, // COL_WARNDIALOGBOXTITLE,
	{L"WarnDialog.Box.Title.Highlight"sv,              F_YELLOW|B_RED,         }, // COL_WARNDIALOGHIGHLIGHTBOXTITLE,
	{L"WarnDialog.Edit"sv,                             F_BLACK|B_CYAN,         }, // COL_WARNDIALOGEDIT,
	{L"WarnDialog.Button"sv,                           F_WHITE|B_RED,          }, // COL_WARNDIALOGBUTTON,
	{L"WarnDialog.Button.Selected"sv,                  F_BLACK|B_LIGHTGRAY,    }, // COL_WARNDIALOGSELECTEDBUTTON,
	{L"WarnDialog.Button.Highlight"sv,                 F_YELLOW|B_RED,         }, // COL_WARNDIALOGHIGHLIGHTBUTTON,
	{L"WarnDialog.Button.Highlight.Selected"sv,        F_YELLOW|B_LIGHTGRAY,   }, // COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON,
	{L"Keybar.Num"sv,                                  F_LIGHTGRAY|B_BLACK,    }, // COL_KEYBARNUM,
	{L"Keybar.Text"sv,                                 F_BLACK|B_CYAN,         }, // COL_KEYBARTEXT,
	{L"Keybar.Background"sv,                           F_LIGHTGRAY|B_BLACK,    }, // COL_KEYBARBACKGROUND,
	{L"CommandLine"sv,                                 ColorsInit::Default,    }, // COL_COMMANDLINE,
	{L"Clock"sv,                                       F_BLACK|B_CYAN,         }, // COL_CLOCK,
	{L"Viewer.Text"sv,                                 F_LIGHTCYAN|B_BLUE,     }, // COL_VIEWERTEXT,
	{L"Viewer.Text.Selected"sv,                        F_BLACK|B_CYAN,         }, // COL_VIEWERSELECTEDTEXT,
	{L"Viewer.Status"sv,                               F_BLACK|B_CYAN,         }, // COL_VIEWERSTATUS,
	{L"Editor.Text"sv,                                 F_LIGHTCYAN|B_BLUE,     }, // COL_EDITORTEXT,
	{L"Editor.Text.Selected"sv,                        F_BLACK|B_CYAN,         }, // COL_EDITORSELECTEDTEXT,
	{L"Editor.Status"sv,                               F_BLACK|B_CYAN,         }, // COL_EDITORSTATUS,
	{L"Help.Text"sv,                                   F_BLACK|B_CYAN,         }, // COL_HELPTEXT,
	{L"Help.Text.Highlight"sv,                         F_WHITE|B_CYAN,         }, // COL_HELPHIGHLIGHTTEXT,
	{L"Help.Topic"sv,                                  F_YELLOW|B_CYAN,        }, // COL_HELPTOPIC,
	{L"Help.Topic.Selected"sv,                         F_WHITE|B_BLACK,        }, // COL_HELPSELECTEDTOPIC,
	{L"Help.Box"sv,                                    F_BLACK|B_CYAN,         }, // COL_HELPBOX,
	{L"Help.Box.Title"sv,                              F_BLACK|B_CYAN,         }, // COL_HELPBOXTITLE,
	{L"Panel.DragText"sv,                              F_YELLOW|B_CYAN,        }, // COL_PANELDRAGTEXT,
	{L"Dialog.Edit.Unchanged"sv,                       F_LIGHTGRAY|B_CYAN,     }, // COL_DIALOGEDITUNCHANGED,
	{L"Panel.Scrollbar"sv,                             F_LIGHTCYAN|B_BLUE,     }, // COL_PANELSCROLLBAR,
	{L"Help.Scrollbar"sv,                              F_BLACK|B_CYAN,         }, // COL_HELPSCROLLBAR,
	{L"Panel.Box"sv,                                   F_LIGHTCYAN|B_BLUE,     }, // COL_PANELBOX,
	{L"Panel.ScreensNumber"sv,                         F_LIGHTCYAN|B_BLACK,    }, // COL_PANELSCREENSNUMBER,
	{L"Dialog.Edit.Selected"sv,                        F_WHITE|B_BLACK,        }, // COL_DIALOGEDITSELECTED,
	{L"CommandLine.Selected"sv,                        F_BLACK|B_CYAN,         }, // COL_COMMANDLINESELECTED,
	{L"Viewer.Arrows"sv,                               F_YELLOW|B_BLUE,        }, // COL_VIEWERARROWS,
	{L"Dialog.List.Scrollbar"sv,                       F_BLACK|B_LIGHTGRAY,    }, // COL_DIALOGLISTSCROLLBAR,
	{L"Menu.Scrollbar"sv,                              F_WHITE|B_CYAN,         }, // COL_MENUSCROLLBAR,
	{L"Viewer.Scrollbar"sv,                            F_LIGHTCYAN|B_BLUE,     }, // COL_VIEWERSCROLLBAR,
	{L"CommandLine.Prefix"sv,                          ColorsInit::Default,    }, // COL_COMMANDLINEPREFIX,
	{L"Dialog.Disabled"sv,                             F_DARKGRAY|B_LIGHTGRAY, }, // COL_DIALOGDISABLED,
	{L"Dialog.Edit.Disabled"sv,                        F_DARKGRAY|B_CYAN,      }, // COL_DIALOGEDITDISABLED,
	{L"Dialog.List.Disabled"sv,                        F_DARKGRAY|B_LIGHTGRAY, }, // COL_DIALOGLISTDISABLED,
	{L"WarnDialog.Disabled"sv,                         F_DARKGRAY|B_RED,       }, // COL_WARNDIALOGDISABLED,
	{L"WarnDialog.Edit.Disabled"sv,                    F_DARKGRAY|B_CYAN,      }, // COL_WARNDIALOGEDITDISABLED,
	{L"WarnDialog.List.Disabled"sv,                    F_DARKGRAY|B_RED,       }, // COL_WARNDIALOGLISTDISABLED,
	{L"Menu.Text.Disabled"sv,                          F_DARKGRAY|B_CYAN,      }, // COL_MENUDISABLEDTEXT,
	{L"Editor.Clock"sv,                                F_BLACK|B_CYAN,         }, // COL_EDITORCLOCK,
	{L"Viewer.Clock"sv,                                F_BLACK|B_CYAN,         }, // COL_VIEWERCLOCK,
	{L"Dialog.List.Title"sv,                           F_BLACK|B_LIGHTGRAY,    }, // COL_DIALOGLISTTITLE
	{L"Dialog.List.Box"sv,                             F_BLACK|B_LIGHTGRAY,    }, // COL_DIALOGLISTBOX,
	{L"WarnDialog.Edit.Selected"sv,                    F_WHITE|B_BLACK,        }, // COL_WARNDIALOGEDITSELECTED,
	{L"WarnDialog.Edit.Unchanged"sv,                   F_LIGHTGRAY|B_CYAN,     }, // COL_WARNDIALOGEDITUNCHANGED,
	{L"Dialog.Combo.Text"sv,                           F_WHITE|B_CYAN,         }, // COL_DIALOGCBOXTEXT,
	{L"Dialog.Combo.Text.Selected"sv,                  F_WHITE|B_BLACK,        }, // COL_DIALOGCBOXSELECTEDTEXT,
	{L"Dialog.Combo.Highlight"sv,                      F_YELLOW|B_CYAN,        }, // COL_DIALOGCBOXHIGHLIGHT,
	{L"Dialog.Combo.Highlight.Selected"sv,             F_YELLOW|B_BLACK,       }, // COL_DIALOGCBOXSELECTEDHIGHLIGHT,
	{L"Dialog.Combo.Box"sv,                            F_WHITE|B_CYAN,         }, // COL_DIALOGCBOXBOX,
	{L"Dialog.Combo.Title"sv,                          F_WHITE|B_CYAN,         }, // COL_DIALOGCBOXTITLE,
	{L"Dialog.Combo.Disabled"sv,                       F_DARKGRAY|B_CYAN,      }, // COL_DIALOGCBOXDISABLED,
	{L"Dialog.Combo.Scrollbar"sv,                      F_WHITE|B_CYAN,         }, // COL_DIALOGCBOXSCROLLBAR,
	{L"WarnDialog.List.Text"sv,                        F_WHITE|B_RED,          }, // COL_WARNDIALOGLISTTEXT,
	{L"WarnDialog.List.Text.Selected"sv,               F_BLACK|B_LIGHTGRAY,    }, // COL_WARNDIALOGLISTSELECTEDTEXT,
	{L"WarnDialog.List.Highlight"sv,                   F_YELLOW|B_RED,         }, // COL_WARNDIALOGLISTHIGHLIGHT,
	{L"WarnDialog.List.Highlight.Selected"sv,          F_YELLOW|B_LIGHTGRAY,   }, // COL_WARNDIALOGLISTSELECTEDHIGHLIGHT,
	{L"WarnDialog.List.Box"sv,                         F_WHITE|B_RED,          }, // COL_WARNDIALOGLISTBOX,
	{L"WarnDialog.List.Title"sv,                       F_WHITE|B_RED,          }, // COL_WARNDIALOGLISTTITLE,
	{L"WarnDialog.List.Scrollbar"sv,                   F_WHITE|B_RED,          }, // COL_WARNDIALOGLISTSCROLLBAR,
	{L"WarnDialog.Combo.Text"sv,                       F_WHITE|B_CYAN,         }, // COL_WARNDIALOGCBOXTEXT,
	{L"WarnDialog.Combo.Text.Selected"sv,              F_WHITE|B_BLACK,        }, // COL_WARNDIALOGCBOXSELECTEDTEXT,
	{L"WarnDialog.Combo.Highlight"sv,                  F_YELLOW|B_CYAN,        }, // COL_WARNDIALOGCBOXHIGHLIGHT,
	{L"WarnDialog.Combo.Highlight.Selected"sv,         F_YELLOW|B_BLACK,       }, // COL_WARNDIALOGCBOXSELECTEDHIGHLIGHT,
	{L"WarnDialog.Combo.Box"sv,                        F_WHITE|B_CYAN,         }, // COL_WARNDIALOGCBOXBOX,
	{L"WarnDialog.Combo.Title"sv,                      F_WHITE|B_CYAN,         }, // COL_WARNDIALOGCBOXTITLE,
	{L"WarnDialog.Combo.Disabled"sv,                   F_DARKGRAY|B_CYAN,      }, // COL_WARNDIALOGCBOXDISABLED,
	{L"WarnDialog.Combo.Scrollbar"sv,                  F_WHITE|B_CYAN,         }, // COL_WARNDIALOGCBOXSCROLLBAR,
	{L"Dialog.List.Arrows"sv,                          F_YELLOW|B_LIGHTGRAY,   }, // COL_DIALOGLISTARROWS,
	{L"Dialog.List.Arrows.Disabled"sv,                 F_DARKGRAY|B_LIGHTGRAY, }, // COL_DIALOGLISTARROWSDISABLED,
	{L"Dialog.List.Arrows.Selected"sv,                 F_YELLOW|B_BLACK,       }, // COL_DIALOGLISTARROWSSELECTED,
	{L"Dialog.Combo.Arrows"sv,                         F_YELLOW|B_CYAN,        }, // COL_DIALOGCOMBOARROWS,
	{L"Dialog.Combo.Arrows.Disabled"sv,                F_DARKGRAY|B_CYAN,      }, // COL_DIALOGCOMBOARROWSDISABLED,
	{L"Dialog.Combo.Arrows.Selected"sv,                F_YELLOW|B_BLACK,       }, // COL_DIALOGCOMBOARROWSSELECTED,
	{L"WarnDialog.List.Arrows"sv,                      F_YELLOW|B_RED,         }, // COL_WARNDIALOGLISTARROWS,
	{L"WarnDialog.List.Arrows.Disabled"sv,             F_DARKGRAY|B_RED,       }, // COL_WARNDIALOGLISTARROWSDISABLED,
	{L"WarnDialog.List.Arrows.Selected"sv,             F_YELLOW|B_LIGHTGRAY,   }, // COL_WARNDIALOGLISTARROWSSELECTED,
	{L"WarnDialog.Combo.Arrows"sv,                     F_YELLOW|B_CYAN,        }, // COL_WARNDIALOGCOMBOARROWS,
	{L"WarnDialog.Combo.Arrows.Disabled"sv,            F_DARKGRAY|B_CYAN,      }, // COL_WARNDIALOGCOMBOARROWSDISABLED,
	{L"WarnDialog.Combo.Arrows.Selected"sv,            F_YELLOW|B_BLACK,       }, // COL_WARNDIALOGCOMBOARROWSSELECTED,
	{L"Menu.Arrows"sv,                                 F_YELLOW|B_CYAN,        }, // COL_MENUARROWS,
	{L"Menu.Arrows.Disabled"sv,                        F_DARKGRAY|B_CYAN,      }, // COL_MENUARROWSDISABLED,
	{L"Menu.Arrows.Selected"sv,                        F_YELLOW|B_BLACK,       }, // COL_MENUARROWSSELECTED,
	{L"CommandLine.UserScreen"sv,                      ColorsInit::Default,    }, // COL_COMMANDLINEUSERSCREEN,
	{L"Editor.Scrollbar"sv,                            F_LIGHTCYAN|B_BLUE,     }, // COL_EDITORSCROLLBAR,
	{L"Menu.GrayText"sv,                               F_DARKGRAY|B_CYAN,      }, // COL_MENUGRAYTEXT,
	{L"Menu.GrayText.Selected"sv,                      F_LIGHTGRAY|B_BLACK,    }, // COL_MENUSELECTEDGRAYTEXT,
	{L"Dialog.Combo.GrayText"sv,                       F_DARKGRAY|B_CYAN,      }, // COL_DIALOGCOMBOGRAY,
	{L"Dialog.Combo.GrayText.Selected"sv,              F_LIGHTGRAY|B_BLACK,    }, // COL_DIALOGCOMBOSELECTEDGRAYTEXT,
	{L"Dialog.List.GrayText"sv,                        F_DARKGRAY|B_LIGHTGRAY, }, // COL_DIALOGLISTGRAY,
	{L"Dialog.List.GrayText.Selected"sv,               F_LIGHTGRAY|B_BLACK,    }, // COL_DIALOGLISTSELECTEDGRAYTEXT,
	{L"WarnDialog.Combo.GrayText"sv,                   F_DARKGRAY|B_CYAN,      }, // COL_WARNDIALOGCOMBOGRAY,
	{L"WarnDialog.Combo.GrayText.Selected"sv,          F_LIGHTGRAY|B_BLACK,    }, // COL_WARNDIALOGCOMBOSELECTEDGRAYTEXT,
	{L"WarnDialog.List.GrayText"sv,                    F_DARKGRAY|B_RED,       }, // COL_WARNDIALOGLISTGRAY,
	{L"WarnDialog.List.GrayText.Selected"sv,           F_BLACK|B_LIGHTGRAY,    }, // COL_WARNDIALOGLISTSELECTEDGRAYTEXT,
	{L"Dialog.DefaultButton"sv,                        F_BLACK|B_LIGHTGRAY,    }, // COL_DIALOGDEFAULTBUTTON,
	{L"Dialog.DefaultButton.Selected"sv,               F_BLACK|B_CYAN,         }, // COL_DIALOGSELECTEDDEFAULTBUTTON,
	{L"Dialog.DefaultButton.Highlight"sv,              F_YELLOW|B_LIGHTGRAY,   }, // COL_DIALOGHIGHLIGHTDEFAULTBUTTON,
	{L"Dialog.DefaultButton.Highlight.Selected"sv,     F_YELLOW|B_CYAN,        }, // COL_DIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON,
	{L"WarnDialog.DefaultButton"sv,                    F_WHITE|B_RED,          }, // COL_WARNDIALOGDEFAULTBUTTON,
	{L"WarnDialog.DefaultButton.Selected"sv,           F_BLACK|B_LIGHTGRAY,    }, // COL_WARNDIALOGSELECTEDDEFAULTBUTTON,
	{L"WarnDialog.DefaultButton.Highlight"sv,          F_YELLOW|B_RED,         }, // COL_WARNDIALOGHIGHLIGHTDEFAULTBUTTON,
	{L"WarnDialog.DefaultButton.Highlight.Selected"sv, F_YELLOW|B_LIGHTGRAY,   }, // COL_WARNDIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON,
};

static_assert(std::size(Init) == COL_LASTPALETTECOLOR);

palette::palette():
	CurrentPalette(std::size(Init))
{
	ResetToDefaultIndex();
	PaletteChanged = false;
}

static auto index_color(ColorsInit const& i)
{
	return i.Index == ColorsInit::Default?
		colors::default_color() :
		colors::NtColorToFarColor(i.Index);
}

void palette::Reset(bool const RGB)
{
	const auto rgb_color = [&Palette = colors::default_palette()](ColorsInit const& i)
	{
		auto Color = index_color(i);

		if (i.Index == ColorsInit::Default)
			return Color;

		Color.SetFgIndex(false);
		Color.SetBgIndex(false);
		colors::set_color_bits(Color.ForegroundColor, Palette[colors::index_bits(Color.ForegroundColor)]);
		colors::set_color_bits(Color.BackgroundColor, Palette[colors::index_bits(Color.BackgroundColor)]);
		return Color;
	};

	const auto set_color = RGB?
		function_ref(rgb_color) :
		function_ref(index_color);

	std::ranges::transform(Init, CurrentPalette.begin(), set_color);

	PaletteChanged = true;
}

void palette::ResetToDefaultIndex()
{
	Reset(false);
}

void palette::ResetToDefaultRGB()
{
	Reset(true);
}

FarColor palette::Default(size_t const Index) const
{
	assert(Index < std::size(Init));

	return index_color(Init[Index]);
}

void palette::Set(size_t StartOffset, std::span<FarColor const> Values)
{
	assert(Values.size() + StartOffset <= CurrentPalette.size());

	std::ranges::copy(Values, CurrentPalette.begin() + StartOffset);
	PaletteChanged = true;
}

void palette::CopyTo(std::span<FarColor> const Destination) const
{
	const auto Size = std::min(CurrentPalette.size(), Destination.size());
	std::copy_n(CurrentPalette.begin(), Size, Destination.begin());
}

const FarColor& palette::operator[](size_t const Index) const
{
	assert(Index < CurrentPalette.size());

	return CurrentPalette[Index];
}

size_t palette::size() const
{
	return CurrentPalette.size();
}


static palette::custom_colors CustomColors;

static string CustomLabel(size_t Index)
{
	return far::format(L"CustomColor{}"sv, Index);
}

void palette::Load()
{
	const auto& ColorsCfg = *ConfigProvider().ColorsCfg();

	for (const auto& [i, index]: enumerate(CurrentPalette))
	{
		ColorsCfg.GetValue(Init[index].Name, i);
	}

	for (const auto& [i, index]: enumerate(CustomColors))
	{
		FarColor Color;
		i = ColorsCfg.GetValue(CustomLabel(index), Color)?
			colors::color_value(Color.BackgroundColor) :
			RGB(255,255,255);
	}

	PaletteChanged = false;
	CustomColorsChanged = false;
}

void palette::Save(bool always)
{
	if (!PaletteChanged && !CustomColorsChanged && !always)
		return;

	SCOPED_ACTION(auto)(ConfigProvider().ColorsCfg()->ScopedTransaction());

	if (PaletteChanged)
	{
		for (const auto& [i, index]: enumerate(CurrentPalette))
		{
			ConfigProvider().ColorsCfg()->SetValue(Init[index].Name, i);
		}
		PaletteChanged = false;
	}

	if (CustomColorsChanged)
	{
		for (const auto& [i, index] : enumerate(CustomColors))
		{
			FarColor Color{};
			Color.BackgroundColor = colors::opaque(i);
			ConfigProvider().ColorsCfg()->SetValue(CustomLabel(index), Color);
		}
		CustomColorsChanged = false;
	}
}

palette::custom_colors palette::GetCustomColors() const
{
	return CustomColors;
}

void palette::SetCustomColors(const custom_colors& Colors)
{
	if (CustomColors == Colors)
		return;

	CustomColors = Colors;
	CustomColorsChanged = true;
}
