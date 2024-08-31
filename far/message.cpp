/*
message.cpp

Вывод MessageBox
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
#include "message.hpp"

// Internal:
#include "ctrlobj.hpp"
#include "farcolor.hpp"
#include "dialog.hpp"
#include "scrbuf.hpp"
#include "keys.hpp"
#include "interf.hpp"
#include "colormix.hpp"
#include "config.hpp"
#include "keyboard.hpp"
#include "FarDlgBuilder.hpp"
#include "clipboard.hpp"
#include "lang.hpp"
#include "strmix.hpp"
#include "global.hpp"
#include "eol.hpp"

// Platform:

// Common:
#include "common/view/enumerate.hpp"

// External:
#include "format.hpp"


//----------------------------------------------------------------------------

struct message_context
{
	int FirstButtonIndex{};
	int LastButtonIndex{};
	bool IsWarningStyle{};
	std::optional<error_state_ex> ErrorState;

	intptr_t DlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
};

intptr_t message_context::DlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	switch (Msg)
	{
		case DN_INITDIALOG:
		{
			FarDialogItem di;

			for (int i=0; Dlg->SendMessage(DM_GETDLGITEMSHORT,i,&di); i++)
			{
				if (di.Type==DI_EDIT)
				{
					COORD pos{};
					Dlg->SendMessage(DM_SETCURSORPOS,i,&pos);
				}
			}
		}
		break;
		case DN_CTLCOLORDLGITEM:
		{
			FarDialogItem di;
			Dlg->SendMessage(DM_GETDLGITEMSHORT,Param1,&di);

			if (di.Type==DI_EDIT)
			{
				const auto& Color = colors::PaletteColorToFarColor(IsWarningStyle? COL_WARNDIALOGTEXT : COL_DIALOGTEXT);
				const auto& Colors = *static_cast<FarDialogItemColors const*>(Param2);
				Colors.Colors[0] = Color;
				Colors.Colors[2] = Color;
			}
		}
		break;
		case DN_CONTROLINPUT:
		{
			const auto record = static_cast<const INPUT_RECORD *>(Param2);
			if (record->EventType==KEY_EVENT)
			{
				switch(InputRecordToKey(record))
				{
				case KEY_F3:
					if(ErrorState)
					{
						const string Errors[]
						{
							ErrorState->ErrnoStr(),
							ErrorState->Win32ErrorStr(),
							ErrorState->NtErrorStr(),
						};

						const auto MaxStr = std::max(Errors[0].size(), Errors[1].size());
						const auto SysArea = 5 * 2;
						const auto FieldsWidth = std::max(80 - SysArea, std::min(static_cast<int>(MaxStr), ScrX - SysArea));

						DialogBuilder Builder(lng::MError);
						Builder.AddText(L"errno:");
						Builder.AddConstEditField(Errors[0], FieldsWidth);
						Builder.AddText(L"LastError:");
						Builder.AddConstEditField(Errors[1], FieldsWidth);
						Builder.AddText(L"NTSTATUS:");
						Builder.AddConstEditField(Errors[2], FieldsWidth);
						Builder.AddOK();
						Builder.ShowDialog();
					}
					break;

				case KEY_TAB:
				case KEY_RIGHT:
				case KEY_NUMPAD6:
					if(Param1==LastButtonIndex)
					{
						Dlg->SendMessage(DM_SETFOCUS, FirstButtonIndex, nullptr);
						return TRUE;
					}
					break;

				case KEY_SHIFTTAB:
				case KEY_LEFT:
				case KEY_NUMPAD4:
					if(Param1==FirstButtonIndex)
					{
						Dlg->SendMessage(DM_SETFOCUS, LastButtonIndex, nullptr);
						return TRUE;
					}
					break;

				case KEY_CTRLC:
				case KEY_RCTRLC:
				case KEY_CTRLINS:
				case KEY_RCTRLINS:
				case KEY_CTRLNUMPAD0:
				case KEY_RCTRLNUMPAD0:
					{
						SetClipboardText(view_as<string>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr)));
					}
					break;
				}
			}
		}
		break;
	default:
		break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

static message_result MessageImpl(
	unsigned const Flags,
	string_view const Title,
	std::vector<string>&& Strings,
	std::vector<string>&& Buttons,
	const error_state_ex* const ErrorState,
	std::span<string const> const Inserts,
	string_view const HelpTopic,
	Plugin* const PluginNumber,
	const UUID* const Id
)
{
	message_context Context;

	Context.IsWarningStyle = (Flags&MSG_WARNING) != 0;

	string ErrorMessage, SystemErrorMessage;

	if (ErrorState)
	{
		Context.ErrorState = *ErrorState;
		ErrorMessage = Context.ErrorState->What;

		if (Context.ErrorState->any())
			SystemErrorMessage = Context.ErrorState->system_error();

		if (!SystemErrorMessage.empty())
		{
			size_t index = 1;
			for (const auto& i: Inserts)
			{
				replace(SystemErrorMessage, L'%' + str(index), i);
				++index;
			}
		}
	}

	auto MaxLength = !Strings.empty()? std::ranges::fold_left(Strings, 0uz, [](size_t const Value, string const& i){ return std::max(Value, i.size()); }) : 0;

	string strClipText;

	const auto Eol = eol::system.str();

	if (!Title.empty())
	{
		MaxLength = std::max(MaxLength, Title.size() + 2); // 2 for surrounding spaces
		append(strClipText, Title, Eol, Eol);
	}

	size_t BtnLength = std::ranges::fold_left(Buttons, 0uz, [](size_t Result, const auto& i)
	{
		return Result + HiStrlen(i) + 2 + 2 + 1; // "[ ", " ]", " "
	});

	if (BtnLength)
	{
		BtnLength--; // last space
		MaxLength = std::max(MaxLength, BtnLength);
	}

	// should fit in the screen, unless buttons > screen width
	const auto MAX_MESSAGE_WIDTH = std::max(static_cast<size_t>(ScrX - 10), BtnLength);

	MaxLength = std::min(MaxLength, MAX_MESSAGE_WIDTH);

	join(strClipText, Eol, Strings);
	append(strClipText, Eol, Eol);

	if (!ErrorMessage.empty() || !SystemErrorMessage.empty())
	{
		if (!ErrorMessage.empty())
			append(strClipText, ErrorMessage, Eol);

		if (!SystemErrorMessage.empty())
			append(strClipText, SystemErrorMessage, Eol, Eol);

		if (!Strings.empty())
			Strings.emplace_back(L"\x1"sv);

		const auto add_wrapped = [&](string_view const Str)
		{
			for (const auto& i : wrapped_text(Str, MAX_MESSAGE_WIDTH))
			{
				Strings.emplace_back(i);
				MaxLength = std::max(MaxLength, i.size());
			}
		};

		add_wrapped(ErrorMessage);
		add_wrapped(SystemErrorMessage);
	}

	join(strClipText, L" "sv, Buttons);

	rectangle Position;

	int MessageWidth = static_cast<int>(MaxLength + 6 + 2 + 2); // 6 for frame, 2 for border, 2 for inner margin
	if (MessageWidth < ScrX)
	{
		Position.left = (ScrX + 1 - MessageWidth) / 2;
	}
	else
	{
		Position.left = 0;
	}

	Position.right = Position.left + MessageWidth - 1;


	int MessageHeight = static_cast<int>(Strings.size() + 2 + 2); // 2 for frame, 2 for border
	if (!Buttons.empty())
	{
		MessageHeight += 2; // 1 for separator, 1 for buttons line
	}

	if (MessageHeight < ScrY)
	{
		// Should be +1 here to center the message properly,
		// but it was shifted up one position for years
		// and some buggy plugins depends on it
		Position.top = (ScrY - MessageHeight) / 2; // + 1;
	}
	else
	{
		Position.top = 0;
	}

	Position.bottom = Position.top + MessageHeight - 1;

	// *** Вариант с Диалогом ***

	if (!Buttons.empty())
	{
		std::vector<DialogItemEx> MsgDlg;
		MsgDlg.reserve(Strings.size() + Buttons.size() + 1 + 1); // 1 for border, 1 for separator

		{
			DialogItemEx Item;
			Item.Type = DI_DOUBLEBOX;
			Item.X1 = 3;
			Item.Y1 = 1;
			Item.X2 = Position.width() - 4;
			Item.Y2 = Position.height() - 2;
			Item.strData = Title;
			MsgDlg.emplace_back(std::move(Item));
		}

		bool StrSeparator=false;

		for (const auto& [Str, Index]: enumerate(Strings))
		{
			DialogItemEx Item;

			Item.Type = DI_TEXT;
			Item.Flags |= DIF_SHOWAMPERSAND;

			Item.X1 = (Flags & MSG_LEFTALIGN) ? 5 : -1;
			Item.Y1 = Index + 2;

			if (!Str.empty() && any_of(Str.front(), L'\1', L'\2'))
			{
				Item.Flags |= (Str.front() == L'\2'? DIF_SEPARATOR2 : DIF_SEPARATOR);
				if(Index == Strings.size() - 1)
				{
					StrSeparator=true;
				}
			}
			else
			{
				if (Str.size() + 6 + 2 + 2 > static_cast<size_t>(MessageWidth)) // 6 for frame, 2 for border, 2 for inner margin
				{
					Item.Type = DI_EDIT;
					Item.Flags |= DIF_READONLY | DIF_BTNNOCLOSE | DIF_SELECTONENTRY;
					Item.X1 = 5;
					Item.X2 = Position.width() - 6;
				}

				Item.strData = std::move(Str);
			}
			MsgDlg.emplace_back(std::move(Item));
		}


		if (!StrSeparator)
		{
			DialogItemEx Item;

			Item.Type = DI_TEXT;
			Item.Flags = DIF_SEPARATOR;
			Item.Y1 = Item.Y2 = Strings.size() + 2;

			MsgDlg.emplace_back(std::move(Item));
		}
		else
		{
			// BUGBUG
			--MessageHeight;
			--Position.bottom;
			--MsgDlg[0].Y2;
		}

		Context.FirstButtonIndex = static_cast<int>(MsgDlg.size());
		Context.LastButtonIndex = static_cast<int>(Context.FirstButtonIndex + Buttons.size() - 1);

		const auto ButtonsCount = Buttons.size();

		for (auto& i: Buttons)
		{
			DialogItemEx Item;

			Item.Type = DI_BUTTON;
			Item.Flags = DIF_CENTERGROUP;

			Item.Y1 = Position.height() - 3;
			Item.strData = std::move(i);

			MsgDlg.emplace_back(std::move(Item));
		}

		MsgDlg[MsgDlg.size() - ButtonsCount].Flags |= DIF_DEFAULTBUTTON | DIF_FOCUS;

		clear_and_shrink(Strings);
		clear_and_shrink(Buttons);

		const auto Dlg = Dialog::create(MsgDlg, std::bind_front(&message_context::DlgProc, &Context), &strClipText);
		if (Position.left == -1)
			Position.left = 0;
		if (Position.top == -1)
			Position.top = 0;

		// Make it "legally" centered:
		Position.right = Position.width();
		Position.left = -1;
		Position.bottom = Position.height();
		Position.top = -1;

		Dlg->SetPosition(Position);
		if(Id)
			Dlg->SetId(*Id);

		if (!HelpTopic.empty())
			Dlg->SetHelp(HelpTopic);

		Dlg->SetPluginOwner(PluginNumber); // Запомним номер плагина

		if (Context.IsWarningStyle)
		{
			Dlg->SetDialogMode(DMODE_WARNINGSTYLE);
		}

		if (Flags & MSG_NOPLUGINS)
			Dlg->SetDialogMode(DMODE_NOPLUGINS);

		FlushInputBuffer();

		if (Flags & MSG_KILLSAVESCREEN)
			Dlg->SendMessage(DM_KILLSAVESCREEN, 0, nullptr);

		Dlg->Process();
		const auto RetCode = Dlg->GetExitCode();

		return RetCode < 0? message_result::cancelled : static_cast<message_result>(RetCode - Context.FirstButtonIndex);
	}


	// *** Без Диалога! ***
	HideCursor();

	if (!(Flags & MSG_KEEPBACKGROUND))
	{
		SetScreen(Position, L' ', colors::PaletteColorToFarColor((Flags & MSG_WARNING)? COL_WARNDIALOGTEXT : COL_DIALOGTEXT));
		DropShadow(Position);
		Box({ Position.left + 3, Position.top + 1, Position.right - 3, Position.bottom - 1 }, colors::PaletteColorToFarColor((Flags & MSG_WARNING)? COL_WARNDIALOGBOX : COL_DIALOGBOX), DOUBLE_BOX);
	}

	SetColor((Flags & MSG_WARNING)?COL_WARNDIALOGTEXT:COL_DIALOGTEXT);

	if (!Title.empty())
	{
		const auto strTempTitle = cut_right(Title, MaxLength);

		GotoXY(Position.left + (Position.width() - 2 - static_cast<int>(strTempTitle.size())) / 2, Position.top + 1);
		Text(concat(L' ', strTempTitle, L' '));
	}

	for (const auto i: std::views::iota(0uz, Strings.size()))
	{
		const auto& SrcItem = Strings[i];

		if (!SrcItem.empty() && any_of(SrcItem.front(), L'\1', L'\2'))
		{
			int Length = Position.width() - 1;
			if (Length > 5)
				Length -= 5;

			if (Length>1)
			{
				SetColor((Flags & MSG_WARNING)?COL_WARNDIALOGBOX:COL_DIALOGBOX);
				GotoXY(Position.left + 3, Position.top + static_cast<int>(i) + 2);
				DrawLine(Length, SrcItem.front() == L'\2'? line_type::h2_to_v2 : line_type::h1_to_v2);
				string SeparatorText = SrcItem.substr(1);
				if (!SeparatorText.empty())
				{
					if (SeparatorText.front() != L' ')
						SeparatorText.insert(0, 1, L' ');
					if (SeparatorText.back() != L' ')
						SeparatorText.push_back(L' ');
				}

				if (SeparatorText.size() < static_cast<size_t>(Length))
				{
					GotoXY(Position.left + 3 + static_cast<int>(Length - SeparatorText.size()) / 2, Position.top + static_cast<int>(i)+2);
					Text(SeparatorText);
				}

				SetColor((Flags & MSG_WARNING)?COL_WARNDIALOGBOX:COL_DIALOGTEXT);
			}

			continue;
		}

		GotoXY(Position.left + 5, Position.top + static_cast<int>(i) + 2);
		Text((Flags & MSG_LEFTALIGN? fit_to_left : fit_to_center)(SrcItem, Position.width() - 10));
	}

	/* $ 13.01.2003 IS
	   - Принудительно уберем запрет отрисовки экрана, если количество кнопок
	     в сообщении равно нулю и макрос закончил выполняться. Это необходимо,
	     чтобы заработал прогресс-бар от плагина, который был запущен при помощи
	     макроса запретом отрисовки (bugz#533).
	*/

	if (Global->ScrBuf->GetLockCount()>0 && !Global->CtrlObject->Macro.PeekKey())
		Global->ScrBuf->SetLockCount(0);

	Global->ScrBuf->Flush();

	return message_result::cancelled;
}

message_result Message(unsigned const Flags, string_view const Title, std::vector<string> Strings, span<lng const> const Buttons, string_view const HelpTopic, const UUID* const Id)
{
	std::vector<string> StrButtons;
	StrButtons.reserve(Buttons.size());
	std::ranges::transform(Buttons, std::back_inserter(StrButtons), msg);

	return MessageImpl(Flags, Title, std::move(Strings), std::move(StrButtons), nullptr, {}, HelpTopic, nullptr, Id);
}

message_result Message(unsigned const Flags, const error_state_ex& ErrorState, string_view  const Title, std::vector<string> Strings, span<lng const> const Buttons, string_view const HelpTopic, const UUID* const Id, span<string const> const Inserts)
{
	std::vector<string> StrButtons;
	StrButtons.reserve(Buttons.size());
	std::ranges::transform(Buttons, std::back_inserter(StrButtons), msg);

	return MessageImpl(Flags, Title, std::move(Strings), std::move(StrButtons), &ErrorState, Inserts, HelpTopic, nullptr, Id);
}

message_result Message(unsigned const Flags, const error_state_ex* const ErrorState, string_view const Title, std::vector<string> Strings, std::vector<string> Buttons, string_view const HelpTopic, const UUID* const Id, Plugin* const PluginNumber)
{
	return MessageImpl(Flags, Title, std::move(Strings), std::move(Buttons), ErrorState, {}, HelpTopic, PluginNumber, Id);
}
