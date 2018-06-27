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

#include "message.hpp"

#include "ctrlobj.hpp"
#include "farcolor.hpp"
#include "dialog.hpp"
#include "scrbuf.hpp"
#include "keys.hpp"
#include "taskbar.hpp"
#include "interf.hpp"
#include "colormix.hpp"
#include "config.hpp"
#include "keyboard.hpp"
#include "FarDlgBuilder.hpp"
#include "clipboard.hpp"
#include "lang.hpp"
#include "strmix.hpp"
#include "global.hpp"

#include "common/enum_tokens.hpp"

#include "format.hpp"

static string GetWin32ErrorString(const error_state_ex& ErrorState)
{
	return os::GetErrorString(false, ErrorState.Win32Error);
}

static string GetNtErrorString(const error_state_ex& ErrorState)
{
	return os::GetErrorString(true, ErrorState.NtError);
}

string GetErrorString(const error_state_ex& ErrorState)
{
	auto Str = ErrorState.What;
	if (!Str.empty())
		append(Str, L": "sv);

	const auto UseNtMessages = false;

	Str += (UseNtMessages? GetNtErrorString : GetWin32ErrorString)(ErrorState);

	return Str;
}

intptr_t Message::MsgDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
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
					COORD pos={};
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
				const auto& Color = colors::PaletteColorToFarColor(IsWarningStyle ? COL_WARNDIALOGTEXT : COL_DIALOGTEXT);
				const auto Colors = static_cast<FarDialogItemColors*>(Param2);
				Colors->Colors[0] = Color;
				Colors->Colors[2] = Color;
			}
		}
		break;
		case DN_CONTROLINPUT:
		{
			const auto record = static_cast<const INPUT_RECORD *>(Param2);
			if (record->EventType==KEY_EVENT)
			{
				int key = InputRecordToKey(record);
				switch(key)
				{
				case KEY_F3:
					if(IsErrorType)
					{
						DialogBuilder Builder(lng::MError);
						Builder.AddConstEditField(format(L"LastError: 0x{0:0>8X} - {1}", as_unsigned(m_ErrorState.Win32Error), GetWin32ErrorString(m_ErrorState)), 65);
						Builder.AddConstEditField(format(L"NTSTATUS: 0x{0:0>8X} - {1}", as_unsigned(m_ErrorState.NtError), GetNtErrorString(m_ErrorState)), 65);
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
						SetClipboardText(*reinterpret_cast<string*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr)));
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

Message::Message(DWORD const Flags, string_view const Title, std::vector<string> Strings, const std::vector<lng>& Buttons, string_view const HelpTopic, const GUID* const Id):
	m_ExitCode(0)
{
	std::vector<string> StrButtons;
	StrButtons.reserve(Buttons.size());
	std::transform(ALL_CONST_RANGE(Buttons), std::back_inserter(StrButtons), msg);
	Init(Flags, Title, std::move(Strings), std::move(StrButtons), nullptr, {}, HelpTopic, nullptr, Id);
}

Message::Message(DWORD const Flags, const error_state_ex& ErrorState, string_view  const Title, std::vector<string> Strings, const std::vector<lng>& Buttons, string_view const HelpTopic, const GUID* const Id, const std::vector<string>& Inserts):
	m_ExitCode(0)
{
	std::vector<string> StrButtons;
	StrButtons.reserve(Buttons.size());
	std::transform(ALL_CONST_RANGE(Buttons), std::back_inserter(StrButtons), msg);
	Init(Flags, Title, std::move(Strings), std::move(StrButtons), &ErrorState, Inserts, HelpTopic, nullptr, Id);
}

Message::Message(DWORD const Flags, const error_state_ex* const ErrorState, string_view const Title, std::vector<string> Strings, std::vector<string> Buttons, string_view const HelpTopic, const GUID* const Id, Plugin* const PluginNumber):
	m_ExitCode(0)
{
	Init(Flags, Title, std::move(Strings), std::move(Buttons), ErrorState, {}, HelpTopic, PluginNumber, Id);
}

void Message::Init(
	DWORD const Flags,
	string_view const Title,
	std::vector<string>&& Strings,
	std::vector<string>&& Buttons,
	const error_state_ex* const ErrorState,
	const std::vector<string>& Inserts,
	string_view const HelpTopic,
	Plugin* const PluginNumber,
	const GUID* const Id
	)
{
	IsWarningStyle = (Flags&MSG_WARNING) != 0;
	IsErrorType = ErrorState != nullptr;

	string strErrStr;

	if (IsErrorType)
	{
		m_ErrorState = *ErrorState;
		strErrStr = GetErrorString(m_ErrorState);
		if (!strErrStr.empty())
		{
			size_t index = 1;
			for (const auto& i: Inserts)
			{
				ReplaceStrings(strErrStr, L'%' + str(index), i);
				++index;
			}
		}
	}

	auto MaxLength = !Strings.empty()? std::max_element(ALL_CONST_RANGE(Strings), [](const auto& a, const auto &b) { return a.size() < b.size(); })->size() : 0;

	string strClipText;

	if (!Title.empty())
	{
		MaxLength = std::max(MaxLength, Title.size() + 2); // 2 for surrounding spaces
		append(strClipText, Title, L"\r\n\r\n"sv);
	}

	size_t BtnLength = std::accumulate(Buttons.cbegin(), Buttons.cend(), size_t(0), [](size_t Result, const auto& i)
	{
		return Result + HiStrlen(i.c_str()) + 2 + 2 + 1; // "[ ", " ]", " "
	});

	if (BtnLength)
	{
		BtnLength--; // last space
		MaxLength = std::max(MaxLength, BtnLength);
	}

	// should fit in the screen, unless buttons > screen width
	const auto MAX_MESSAGE_WIDTH = std::max(size_t(ScrX - 10), BtnLength);

	MaxLength = std::min(MaxLength, MAX_MESSAGE_WIDTH);

	for (const auto& i : Strings)
	{
		append(strClipText, i, L"\r\n"sv);
	}
	append(strClipText, L"\r\n"sv);

	if (!strErrStr.empty())
	{
		append(strClipText, strErrStr, L"\r\n\r\n"sv);

		// вычисление "красивого" размера
		auto LenErrStr = strErrStr.size();

		if (LenErrStr > MAX_MESSAGE_WIDTH)
		{
			// половина меньше?
			if (LenErrStr / 2 < MAX_MESSAGE_WIDTH)
			{
				// а половина + 1/3?
				if ((LenErrStr + LenErrStr / 3) / 2 < MAX_MESSAGE_WIDTH)
					LenErrStr=(LenErrStr+LenErrStr/3)/2;
				else
					LenErrStr/=2;
			}
			else
				LenErrStr = MAX_MESSAGE_WIDTH;
		}

		MaxLength = std::max(MaxLength, LenErrStr);

		if (!Strings.empty())
			Strings.emplace_back(L"\1"s);

		for (const auto& i : wrapped_text(strErrStr, LenErrStr))
		{
			Strings.emplace_back(ALL_CONST_RANGE(i));
		}
	}

	join(strClipText, Buttons, L" "sv);

	int X1;

	int MessageWidth = static_cast<int>(MaxLength + 6 + 2 + 2); // 6 for frame, 2 for border, 2 for inner margin
	if (MessageWidth < ScrX)
	{
		X1 = (ScrX - MessageWidth) / 2 + 1;
	}
	else
	{
		X1 = 0;
	}

	int X2 = X1 + MessageWidth - 1;

	MessageX1 = X1;
	MessageX2 = X2; 

	int MessageHeight = static_cast<int>(Strings.size() + 2 + 2); // 2 for frame, 2 for border
	if (!Buttons.empty())
	{
		MessageHeight += 2; // 1 for separator, 1 for buttons line
	}

	int Y1;

	if (MessageHeight < ScrY)
	{
		// Should be +1 here to center the message properly,
		// but it was shifted up one position for years
		// and some buggy plugins depends on it
		Y1 = (ScrY - MessageHeight) / 2; // + 1;
	}
	else
	{
		Y1 = 0;
	}

	int Y2 = Y1 + MessageHeight - 1;

	MessageY1 = Y1;
	MessageY2 = Y2;

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
			Item.X2 = X2 - X1 - 3;
			Item.Y2 = Y2 - Y1 - 1;
			assign(Item.strData, Title);
			MsgDlg.emplace_back(std::move(Item));
		}

		bool StrSeparator=false;

		for (size_t i = 0; i != Strings.size(); ++i)
		{
			DialogItemEx Item;

			Item.Type = DI_TEXT;
			Item.Flags |= DIF_SHOWAMPERSAND;

			Item.X1 = (Flags & MSG_LEFTALIGN) ? 5 : -1;
			Item.Y1 = i + 2;

			if (!Strings[i].empty() && (Strings[i].front() == L'\1' || Strings[i].front() == L'\2'))
			{
				Item.Flags |= (Strings[i].front() == L'\2' ? DIF_SEPARATOR2 : DIF_SEPARATOR);
				if(i == Strings.size() - 1)
				{
					StrSeparator=true;
				}
			}
			else
			{
				if (Strings[i].size() + 6 + 2 + 2 > static_cast<size_t>(MessageWidth)) // 6 for frame, 2 for border, 2 for inner margin
				{
					Item.Type = DI_EDIT;
					Item.Flags |= DIF_READONLY | DIF_BTNNOCLOSE | DIF_SELECTONENTRY;
					Item.X1 = 5;
					Item.X2 = X2-X1-5;
				}

				Item.strData = std::move(Strings[i]);
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
			--Y2;
			--MessageY2;
			--MsgDlg[0].Y2;
		}

		FirstButtonIndex = static_cast<int>(MsgDlg.size());
		LastButtonIndex = static_cast<int>(FirstButtonIndex + Buttons.size() - 1);

		for (size_t i = 0; i != Buttons.size(); ++i)
		{
			DialogItemEx Item;

			Item.Type = DI_BUTTON;
			Item.Flags = DIF_CENTERGROUP;

			Item.Y1 = Y2 - Y1 - 2;
			Item.strData = std::move(Buttons[i]);

			if (!i)
			{
				Item.Flags |= DIF_DEFAULTBUTTON | DIF_FOCUS;
			}

			MsgDlg.emplace_back(std::move(Item));
		}

		clear_and_shrink(Strings);
		clear_and_shrink(Buttons);

		const auto Dlg = Dialog::create(MsgDlg, &Message::MsgDlgProc, this, &strClipText);
		if (X1 == -1) X1 = 0;
		if (Y1 == -1) Y1 = 0;
		Dlg->SetPosition(X1,Y1,X2,Y2);
		if(Id) Dlg->SetId(*Id);

		if (!HelpTopic.empty())
			Dlg->SetHelp(HelpTopic);

		Dlg->SetPluginOwner(PluginNumber); // Запомним номер плагина

		if (IsWarningStyle)
		{
			Dlg->SetDialogMode(DMODE_WARNINGSTYLE);
		}

		Dlg->SetDialogMode(DMODE_MSGINTERNAL);
		if (Flags & MSG_NOPLUGINS)
			Dlg->SetDialogMode(DMODE_NOPLUGINS);
		FlushInputBuffer();

		if (Flags & MSG_KILLSAVESCREEN)
			Dlg->SendMessage(DM_KILLSAVESCREEN, 0, nullptr);

		Dlg->Process();
		const auto RetCode = Dlg->GetExitCode();

		m_ExitCode = RetCode < 0?
			RetCode:
			RetCode - FirstButtonIndex;
	}
	else
	{
	// *** Без Диалога! ***
	SetCursorType(false, 0);

	if (!(Flags & MSG_KEEPBACKGROUND))
	{
		SetScreen(X1,Y1,X2,Y2,L' ',colors::PaletteColorToFarColor((Flags & MSG_WARNING)?COL_WARNDIALOGTEXT:COL_DIALOGTEXT));
		MakeShadow(X1+2,Y2+1,X2+2,Y2+1);
		MakeShadow(X2+1,Y1+1,X2+2,Y2+1);
		Box(X1+3,Y1+1,X2-3,Y2-1,colors::PaletteColorToFarColor((Flags & MSG_WARNING)?COL_WARNDIALOGBOX:COL_DIALOGBOX),DOUBLE_BOX);
	}

	SetColor((Flags & MSG_WARNING)?COL_WARNDIALOGTEXT:COL_DIALOGTEXT);

	if (!Title.empty())
	{
		const auto strTempTitle = cut_right(Title, MaxLength);

		GotoXY(X1+(X2-X1-1-(int)strTempTitle.size())/2,Y1+1);
		Text(concat(L' ', strTempTitle, L' '));
	}

	for (size_t i = 0; i != Strings.size(); ++i)
	{
		const auto& SrcItem = Strings[i];

		if (!SrcItem.empty() && (SrcItem.front() == L'\1' || SrcItem.front() == L'\2'))
		{
			int Length = X2 - X1;
			if (Length > 5)
				Length -= 5;

			if (Length>1)
			{
				SetColor((Flags & MSG_WARNING)?COL_WARNDIALOGBOX:COL_DIALOGBOX);
				GotoXY(X1 + 3, Y1 + static_cast<int>(i) + 2);
				DrawLine(Length, SrcItem.front() == L'\2'? 3 : 1);
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
					GotoXY(X1 + 3 + static_cast<int>(Length - SeparatorText.size()) / 2, Y1 + static_cast<int>(i)+2);
					Text(SeparatorText);
				}

				SetColor((Flags & MSG_WARNING)?COL_WARNDIALOGBOX:COL_DIALOGTEXT);
			}

			continue;
		}

		const auto Width = X2 - X1 + 1;
		GotoXY(X1 + 5, Y1 + static_cast<int>(i) + 2);
		Text((Flags & MSG_LEFTALIGN? fit_to_left : fit_to_center)(SrcItem, Width - 10));
	}

	/* $ 13.01.2003 IS
	   - Принудительно уберем запрет отрисовки экрана, если количество кнопок
	     в сообщении равно нулю и макрос закончил выполняться. Это необходимо,
	     чтобы заработал прогресс-бар от плагина, который был запущен при помощи
	     макроса запретом отрисовки (bugz#533).
	*/

	if (!(Flags & MSG_NOFLUSH))
	{
		if (Global->ScrBuf->GetLockCount()>0 && !Global->CtrlObject->Macro.PeekKey())
			Global->ScrBuf->SetLockCount(0);

		Global->ScrBuf->Flush();
	}
	}
}


void Message::GetMessagePosition(int &X1,int &Y1,int &X2,int &Y2) const
{
	X1=MessageX1;
	Y1=MessageY1;
	X2=MessageX2;
	Y2=MessageY2;
}

/* $ 12.03.2002 VVM
  Новая функция - пользователь попытался прервать операцию.
  Зададим вопрос.
  Возвращает:
   FALSE - продолжить операцию
   TRUE  - прервать операцию
*/
bool AbortMessage()
{
	if(Global->CloseFAR)
	{
		return true;
	}

	SCOPED_ACTION(TaskbarPause);
	const auto Result = Message(MSG_WARNING | MSG_KILLSAVESCREEN,
		msg(lng::MKeyESCWasPressed),
		{
			msg(Global->Opt->Confirm.EscTwiceToInterrupt? lng::MDoYouWantToContinue : lng::MDoYouWantToCancel)
		},
		{ lng::MYes, lng::MNo });

	return Global->Opt->Confirm.EscTwiceToInterrupt.Get() == (Result != Message::first_button);
}
