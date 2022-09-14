/*
color_picker_rgb.cpp

RGB colors extension to the standard color picker
*/
/*
Copyright © 2022 Far Group
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
#include "color_picker_rgb.hpp"

// Internal:
#include "colormix.hpp"
#include "config.hpp"
#include "console.hpp"
#include "dialog.hpp"
#include "exception.hpp"
#include "lang.hpp"
#include "log.hpp"
#include "strmix.hpp"

// Platform:
#include "platform.hpp"

// Common:

// External:

//----------------------------------------------------------------------------

static bool pick_color_rgb_gui(COLORREF& Color, std::array<COLORREF, 16>& CustomColors)
{
	CHOOSECOLOR Params{ sizeof(Params) };

	// Console can be "invisible" in certain fancy scenarios, e.g. a terminal embedded into VS.
	if (IsWindowVisible(console.GetWindow()))
		Params.hwndOwner = console.GetWindow();

	Params.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;

	Params.lpCustColors = CustomColors.data();
	Params.rgbResult = Color;

	if (!ChooseColor(&Params))
	{
		if (const auto LastError = last_error(); LastError.Win32Error)
		{
			LOGWARNING(L"ChooseColor(): {} ({:04X})"sv, LastError, CommDlgExtendedError());
		}
		return false;
	}

	Color = Params.rgbResult;

	return true;
}


struct color_rgb_state
{
	COLORREF CurColor;
};

enum color_rgb_dialog_items
{
	cd_border,

	cd_text_rgb,

	cd_text_r,
	cd_button_r_plus,
	cd_button_r_minus,
	cd_text_g,
	cd_button_g_plus,
	cd_button_g_minus,
	cd_text_b,
	cd_button_b_plus,
	cd_button_b_minus,

	cd_separator,

	cd_button_ok,
	cd_button_cancel,

	cd_count
};

struct rgb_context
{
	uint8_t rgba::*Channel;
	color_rgb_dialog_items TextId;
};

static rgb_context get_rgb_context(color_rgb_dialog_items const Item)
{
	switch (Item)
	{
	case cd_text_r:
	case cd_button_r_plus:
	case cd_button_r_minus:
		return { &rgba::r, cd_text_r };

	case cd_text_g:
	case cd_button_g_plus:
	case cd_button_g_minus:
		return { &rgba::g, cd_text_g };

	case cd_text_b:
	case cd_button_b_plus:
	case cd_button_b_minus:
		return { &rgba::b, cd_text_b };

	default:
		assert(false);
		UNREACHABLE;
	}
}

static auto get_channel_operation(color_rgb_dialog_items const Button)
{
	switch (Button)
	{
	case cd_button_r_plus:
	case cd_button_g_plus:
	case cd_button_b_plus:
		return +1;

	case cd_button_r_minus:
	case cd_button_g_minus:
	case cd_button_b_minus:
		return -1;

	default:
		assert(false);
		UNREACHABLE;
	}
}

static auto channel_value(uint8_t const Channel)
{
	return format(FSTR(L"{:>3}"sv), Channel);
}

static intptr_t GetColorDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	auto& ColorState = edit_as<color_rgb_state>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));

	switch (Msg)
	{
	case DN_CTLCOLORDLGITEM:
		{
			const auto& Colors = *static_cast<FarDialogItemColors const*>(Param2);
			switch (const auto Item = static_cast<color_rgb_dialog_items>(Param1))
			{
			case cd_text_rgb:
				{
					flags::clear(Colors.Colors[0].Flags, FCF_INDEXMASK | FCF_INHERIT_STYLE);
					Colors.Colors[0].BackgroundColor = colors::opaque(ColorState.CurColor);
					Colors.Colors[0].ForegroundColor = Colors.Colors[0].BackgroundColor;
					return TRUE;
				}

			case cd_text_r:
			case cd_text_g:
			case cd_text_b:
				{
					const auto Context = get_rgb_context(Item);
					auto RGBA = colors::to_rgba(ColorState.CurColor);
					auto& Channel = std::invoke(Context.Channel, RGBA);
					const auto SavedValue = Channel;
					RGBA = {};
					Channel = SavedValue;
					flags::clear(Colors.Colors[0].Flags, FCF_INDEXMASK | FCF_INHERIT_STYLE);
					Colors.Colors[0].BackgroundColor = colors::opaque(colors::to_color(RGBA));
					Colors.Colors[0].ForegroundColor = colors::invert(Colors.Colors[0].BackgroundColor, false);
					return TRUE;
				}
			default:
				break;
			}
		}
		break;

	case DN_BTNCLICK:
		{
			switch (const auto Button = static_cast<color_rgb_dialog_items>(Param1))
			{
			case cd_button_r_plus:
			case cd_button_r_minus:
			case cd_button_g_plus:
			case cd_button_g_minus:
			case cd_button_b_plus:
			case cd_button_b_minus:
				{
					const auto Context = get_rgb_context(Button);
					auto RGBA = colors::to_rgba(ColorState.CurColor);
					auto& Channel = std::invoke(Context.Channel, RGBA);

					switch (get_channel_operation(Button))
					{
					case -1:
						if (Channel)
							--Channel;
						break;

					case +1:
						if (Channel < 255)
							++Channel;
						break;
					}

					ColorState.CurColor = colors::to_color(RGBA);
					Dlg->SendMessage(DM_SETTEXTPTR, Context.TextId, UNSAFE_CSTR(channel_value(Channel)));
					return TRUE;
				}

			default:
				break;
			}
		}
		break;

	default:
		break;
	}

	return Dlg->DefProc(Msg, Param1, Param2);
}

static bool pick_color_rgb_tui(COLORREF& Color, [[maybe_unused]] std::array<COLORREF, 16>& CustomColors)
{
	const auto
		RGBX = 10,
		RGBY = 2,
		RGBW = 9,
		RGBH = 4,
		ButtonY = RGBY + RGBH + 1;

	auto ColorDlg = MakeDialogItems<cd_count>(
	{
		{ DI_DOUBLEBOX,   {{3, 1}, {RGBX*2+RGBW-4, ButtonY+1}}, DIF_NONE, msg(lng::MSetColorTitle), },
		{ DI_TEXT,        {{RGBX+3, RGBY+0}, {0, RGBY+0}}, DIF_NONE,       L"   "sv, },
		{ DI_TEXT,        {{RGBX+0, RGBY+1}, {0, RGBY+1}}, DIF_NONE,       L"  0"sv, },
		{ DI_BUTTON,      {{RGBX+0, RGBY+2}, {0, RGBY+2}}, DIF_NOBRACKETS, L"[▲]"sv, },
		{ DI_BUTTON,      {{RGBX+0, RGBY+3}, {0, RGBY+3}}, DIF_NOBRACKETS, L"[▼]"sv, },
		{ DI_TEXT,        {{RGBX+3, RGBY+1}, {0, RGBY+1}}, DIF_NONE,       L"  0"sv, },
		{ DI_BUTTON,      {{RGBX+3, RGBY+2}, {0, RGBY+2}}, DIF_NOBRACKETS, L"[▲]"sv, },
		{ DI_BUTTON,      {{RGBX+3, RGBY+3}, {0, RGBY+3}}, DIF_NOBRACKETS, L"[▼]"sv, },
		{ DI_TEXT,        {{RGBX+6, RGBY+1}, {0, RGBY+1}}, DIF_NONE,       L"  0"sv, },
		{ DI_BUTTON,      {{RGBX+6, RGBY+2}, {0, RGBY+2}}, DIF_NOBRACKETS, L"[▲]"sv, },
		{ DI_BUTTON,      {{RGBX+6, RGBY+3}, {0, RGBY+3}}, DIF_NOBRACKETS, L"[▼]"sv, },

		{ DI_TEXT,        {{-1, ButtonY-1}, {0, ButtonY-1}}, DIF_SEPARATOR, },

		{ DI_BUTTON,      {{0, ButtonY}, {0, ButtonY}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MOk), },
		{ DI_BUTTON,      {{0, ButtonY}, {0, ButtonY}}, DIF_CENTERGROUP, msg(lng::MCancel), },
	});

	color_rgb_state ColorState
	{
		Color,
	};

	{
		const auto RGBA = colors::to_rgba(Color);

		ColorDlg[cd_text_r].strData = channel_value(RGBA.r);
		ColorDlg[cd_text_g].strData = channel_value(RGBA.g);
		ColorDlg[cd_text_b].strData = channel_value(RGBA.b);
	}

	const auto Dlg = Dialog::create(ColorDlg, GetColorDlgProc, &ColorState);

	const auto
		DlgWidth = static_cast<int>(ColorDlg[cd_border].X2) + 4,
		DlgHeight = static_cast<int>(ColorDlg[cd_border].Y2) + 2;

	Dlg->SetPosition({ -1, -1, DlgWidth, DlgHeight });
	Dlg->SetHelp(L"ColorPicker"sv);
	Dlg->Process();

	if (Dlg->GetExitCode() != cd_button_ok)
		return false;

	Color = ColorState.CurColor;
	return true;
}

bool pick_color_rgb(COLORREF& Color, std::array<COLORREF, 16>& CustomColors)
{
	// Calling common dialogs in a non-interactive session is not the best idea.
	return (os::is_interactive_user_session()?
		pick_color_rgb_gui :
		pick_color_rgb_tui
	)(Color, CustomColors);
}
