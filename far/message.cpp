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

#include "headers.hpp"
#pragma hdrstop

#include "message.hpp"
#include "ctrlobj.hpp"
#include "colors.hpp"
#include "dialog.hpp"
#include "scrbuf.hpp"
#include "keys.hpp"
#include "TaskBar.hpp"
#include "interf.hpp"
#include "colormix.hpp"
#include "config.hpp"
#include "keyboard.hpp"
#include "imports.hpp"
#include "FarDlgBuilder.hpp"
#include "clipboard.hpp"
#include "language.hpp"
#include "constitle.hpp"

static string FormatErrorString(bool Nt, DWORD Code)
{
	LPWSTR lpBuffer=nullptr;
	size_t size = FormatMessage((Nt?FORMAT_MESSAGE_FROM_HMODULE:FORMAT_MESSAGE_FROM_SYSTEM)|FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS, (Nt?GetModuleHandle(L"ntdll.dll"):nullptr), Code, 0, reinterpret_cast<LPWSTR>(&lpBuffer), 0, nullptr);
	string Result(lpBuffer, size);
	LocalFree(lpBuffer);
	RemoveUnprintableCharacters(Result);
	return Result;
}

static string GetWin32ErrorString(DWORD LastWin32Error)
{
	return FormatErrorString(false, LastWin32Error);
}

static string GetNtErrorString(NTSTATUS LastNtStatus)
{
	return FormatErrorString(true, LastNtStatus);
}

string GetErrorString()
{
#ifdef USE_NT_MESSAGES
	return GetNtErrorString(Global->CaughtStatus());
#else
	return GetWin32ErrorString(Global->CaughtError());
#endif
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
				auto Color = ColorIndexToColor(IsWarningStyle ? COL_WARNDIALOGTEXT : COL_DIALOGTEXT);
				auto Colors = static_cast<FarDialogItemColors*>(Param2);
				Colors->Colors[0] = Color;
				Colors->Colors[2] = Color;
			}
		}
		break;
		case DN_CONTROLINPUT:
		{
			auto record = static_cast<const INPUT_RECORD *>(Param2);
			if (record->EventType==KEY_EVENT)
			{
				int key = InputRecordToKey(record);
				switch(key)
				{
				case KEY_F3:
					if(IsErrorType)
					{
						DialogBuilder Builder(MError, nullptr);
						Builder.AddConstEditField(FormatString() << L"LastError: 0x" << fmt::MinWidth(8) << fmt::FillChar(L'0') << fmt::Radix(16) << Global->CaughtError() << L" - " << GetWin32ErrorString(Global->CaughtError()), 65);
						Builder.AddConstEditField(FormatString() << L"NTSTATUS: 0x" << fmt::MinWidth(8) << fmt::FillChar(L'0') << fmt::Radix(16) << static_cast<DWORD>(Global->CaughtStatus()) << L" - " << GetNtErrorString(Global->CaughtStatus()), 65);
						Builder.AddOK();
						Builder.ShowDialog();
					}
					break;

				case KEY_TAB:
				case KEY_RIGHT:
				case KEY_NUMPAD6:
					if(Param1==LastButtonIndex)
					{
						Dlg->SendMessage(DM_SETFOCUS,FirstButtonIndex,0);
						return TRUE;
					}
					break;

				case KEY_SHIFTTAB:
				case KEY_LEFT:
				case KEY_NUMPAD4:
					if(Param1==FirstButtonIndex)
					{
						Dlg->SendMessage(DM_SETFOCUS,LastButtonIndex,0);
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
						SetClipboard(*reinterpret_cast<string*>(Dlg->SendMessage(DM_GETDLGDATA, 0, 0)));
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

Message::Message(DWORD Flags, const string& Title, const std::vector<string>& Strings, const std::vector<string>& Buttons, const wchar_t* HelpTopic, Plugin* PluginNumber, const GUID* Id, const std::vector<string>& Inserts):
	m_ExitCode(0)
{
	Init(Flags, Title, Strings, Buttons, Inserts, HelpTopic, PluginNumber, Id);
}

Message::Message(DWORD Flags,size_t Buttons,const string& Title,const wchar_t *Str1, const wchar_t *Str2, const wchar_t *Str3, const wchar_t *Str4, const wchar_t *Str5,
                 const wchar_t *Str6, const wchar_t *Str7, const wchar_t *Str8, const wchar_t *Str9, const wchar_t *Str10, const wchar_t *Str11,const wchar_t *Str12):
	m_ExitCode(0)
{
	// BUGBUG
	const wchar_t *Str[] = { Str1, Str2, Str3, Str4, Str5, Str6, Str7, Str8, Str9, Str10, Str11, Str12 };
	size_t Size = std::count_if(CONST_RANGE(Str, i) { return i != nullptr; });

	std::vector<string> StringsVector(std::cbegin(Str), std::cbegin(Str) + Size - Buttons);
	std::vector<string> ButtonsVector(std::cbegin(Str) + Size - Buttons, std::cbegin(Str) + Size);

	Init(Flags, Title, StringsVector, ButtonsVector, std::vector<string>(), nullptr, nullptr, nullptr);
}

void Message::Init(
	DWORD Flags,
	const string& Title,
	const std::vector<string>& Strings,
	const std::vector<string>& Buttons,
	const std::vector<string>& Inserts,
	const wchar_t* HelpTopic,
	Plugin* PluginNumber,
	const GUID* Id
	)
{
	IsWarningStyle = (Flags&MSG_WARNING) != 0;
	IsErrorType = (Flags&MSG_ERRORTYPE) != 0;

	string strErrStr;

	if (IsErrorType)
	{
		strErrStr = GetErrorString();
		if (!strErrStr.empty())
		{
			FOR(const auto& i, Inserts)
			{
				strErrStr = LangString(strErrStr) << i;
			}
		}
	}

	size_t MaxLength = 0;
	FOR(const auto& i, Strings)
	{
		MaxLength = std::max(MaxLength, i.size());
	}

	string strClipText;

	if (!Title.empty())
	{
		MaxLength = std::max(MaxLength, Title.size() + 2); // 2 for surrounding spaces
		strClipText.append(Title).append(L"\r\n\r\n");
	}

	size_t BtnLength = std::accumulate(Buttons.cbegin(), Buttons.cend(), size_t(0), [](size_t Result, CONST_REFERENCE(Buttons) i)
	{
		return Result + HiStrlen(i.data()) + 2 + 2 + 1; // "[ ", " ]", " "
	});

	if (BtnLength)
	{
		BtnLength--; // last space
		MaxLength = std::max(MaxLength, BtnLength);
	}

	// should fit in the screen, unless buttons > screen width
	auto MAX_MESSAGE_WIDTH = std::max(size_t(ScrX - 10), BtnLength);

	MaxLength = std::min(MaxLength, MAX_MESSAGE_WIDTH);

	auto MessageStrings = Strings;

	if (!strErrStr.empty())
	{
		strClipText += strErrStr + L"\r\n";

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

		// а теперь проврапим
		FarFormatText(strErrStr, static_cast<int>(LenErrStr), strErrStr, L"\n", 0); //?? MaxLength ??
		std::vector<string> ErrorStrings;
		split(ErrorStrings, strErrStr, 0, L"\n");
		MessageStrings.insert(MessageStrings.end(), ALL_CONST_RANGE(ErrorStrings));
	}

	if (MessageStrings.empty())
	{
		MessageStrings.resize(1);
	}

	FOR(const auto& i, Strings)
	{
		strClipText.append(i).append(L"\r\n");
	}
	strClipText += L"\r\n";

	if (!Buttons.empty())
	{
		FOR(const auto& i, Buttons)
		{
			strClipText.append(i).append(L" ");
		}
		strClipText.pop_back();
	}

	int X1, Y1, X2, Y2;

	int MessageWidth = static_cast<int>(MaxLength + 6 + 2 + 2); // 6 for frame, 2 for border, 2 for inner margin
	if (MessageWidth < ScrX)
	{
		X1 = (ScrX - MessageWidth) / 2 + 1;
	}
	else
	{
		X1 = 0;
	}

	X2 = X1 + MessageWidth - 1;

	MessageX1 = X1;
	MessageX2 = X2; 

	int MessageHeight = static_cast<int>(MessageStrings.size() + 2 + 2); // 2 for frame, 2 for border
	if (!Buttons.empty())
	{
		MessageHeight += 2; // 1 for separator, 1 for buttons line
	}

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

	Y2 = Y1 + MessageHeight - 1;

	MessageY1 = Y1;
	MessageY2 = Y2;

	// *** Вариант с Диалогом ***

	if (!Buttons.empty())
	{
		std::vector<DialogItemEx> MsgDlg;
		MsgDlg.reserve(MessageStrings.size() + Buttons.size() + 1 + 1); // 1 for border, 1 for separator

		int RetCode;

		{
			DialogItemEx Item;
			Item.Type = DI_DOUBLEBOX;
			Item.X1 = 3;
			Item.Y1 = 1;
			Item.X2 = X2 - X1 - 3;
			Item.Y2 = Y2 - Y1 - 1;
			Item.strData = Title;
			MsgDlg.emplace_back(std::move(Item));
		}

		bool StrSeparator=false;

		for (size_t i = 0; i != MessageStrings.size(); ++i)
		{
			DialogItemEx Item;

			Item.Type = DI_TEXT;
			Item.Flags |= DIF_SHOWAMPERSAND;

			Item.X1 = (Flags & MSG_LEFTALIGN) ? 5 : -1;
			Item.Y1 = i + 2;

			if (!MessageStrings[i].empty() && (MessageStrings[i].front() == L'\1' || MessageStrings[i].front() == L'\2'))
			{
				Item.Flags |= (MessageStrings[i].front() == L'\2' ? DIF_SEPARATOR2 : DIF_SEPARATOR);
				if(i == MessageStrings.size() - 1)
				{
					StrSeparator=true;
				}
			}
			else
			{
				if (MessageStrings[i].size() + 6 + 2 + 2 > static_cast<size_t>(MessageWidth)) // 6 for frame, 2 for border, 2 for inner margin
				{
					Item.Type = DI_EDIT;
					Item.Flags |= DIF_READONLY | DIF_BTNNOCLOSE | DIF_SELECTONENTRY;
					Item.X1 = 5;
					Item.X2 = X2-X1-5;
					Item.strData = MessageStrings[i];
				}
				else
				{
					Item.strData = MessageStrings[i];
				}
			}
			MsgDlg.emplace_back(std::move(Item));
		}


		if (!StrSeparator)
		{
			DialogItemEx Item;

			Item.Type = DI_TEXT;
			Item.Flags = DIF_SEPARATOR;
			Item.Y1 = Item.Y2 = MessageStrings.size() + 2;

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
			Item.strData = Buttons[i];

			if (!i)
			{
				Item.Flags |= DIF_DEFAULTBUTTON | DIF_FOCUS;
			}

			MsgDlg.emplace_back(std::move(Item));
		}

		auto Dlg = Dialog::create(MsgDlg, this, &Message::MsgDlgProc, &strClipText);
		if (X1 == -1) X1 = 0;
		if (Y1 == -1) Y1 = 0;
		Dlg->SetPosition(X1,Y1,X2,Y2);
		if(Id) Dlg->SetId(*Id);

		if (HelpTopic)
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
			Dlg->SendMessage(DM_KILLSAVESCREEN,0,0);

		Dlg->Process();
		RetCode=Dlg->GetExitCode();

		m_ExitCode = RetCode < 0?
			RetCode:
			RetCode - static_cast<int>(MessageStrings.size()) - 2;
	}
	else
	{
	// *** Без Диалога! ***
	SetCursorType(0,0);

	if (!(Flags & MSG_KEEPBACKGROUND))
	{
		SetScreen(X1,Y1,X2,Y2,L' ',ColorIndexToColor((Flags & MSG_WARNING)?COL_WARNDIALOGTEXT:COL_DIALOGTEXT));
		MakeShadow(X1+2,Y2+1,X2+2,Y2+1);
		MakeShadow(X2+1,Y1+1,X2+2,Y2+1);
		Box(X1+3,Y1+1,X2-3,Y2-1,ColorIndexToColor((Flags & MSG_WARNING)?COL_WARNDIALOGBOX:COL_DIALOGBOX),DOUBLE_BOX);
	}

	SetColor((Flags & MSG_WARNING)?COL_WARNDIALOGTEXT:COL_DIALOGTEXT);

	if (!Title.empty())
	{
		string strTempTitle = Title;

		if (strTempTitle.size() > MaxLength)
			strTempTitle.resize(MaxLength);

		GotoXY(X1+(X2-X1-1-(int)strTempTitle.size())/2,Y1+1);
		Global->FS << L" "<<strTempTitle<<L" ";
	}

	for (size_t i = 0; i != MessageStrings.size(); ++i)
	{
		const auto& SrcItem = MessageStrings[i];

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
				const auto Ptr = SrcItem.data() + 1;
				auto TextLength = wcslen(Ptr);

				if (TextLength < static_cast<size_t>(Length))
				{
					GotoXY(X1 + 3 + static_cast<int>(Length - TextLength) / 2, Y1 + static_cast<int>(i)+2);
					Text(Ptr, TextLength);
				}

				SetColor((Flags & MSG_WARNING)?COL_WARNDIALOGBOX:COL_DIALOGTEXT);
			}

			continue;
		}

		int Length = static_cast<int>(SrcItem.size());
		if (Length + 15 > ScrX)
			Length = ScrX - 15;

		int Width=X2-X1+1;
		FormatString Temp;
		if (Flags & MSG_LEFTALIGN)
		{
			Temp << fmt::LeftAlign() << fmt::MinWidth(Width - 10) << SrcItem;
			GotoXY(X1 + 5, Y1 + static_cast<int>(i)+2);
		}
		else
		{
			auto PosX=X1+(Width-Length)/2;
			Temp << fmt::ExactWidth(PosX - X1 - 4) << L"" << fmt::ExactWidth(Length) << SrcItem << fmt::ExactWidth(X2 - PosX - Length - 3) << L"";
			GotoXY(X1 + 4, Y1 + static_cast<int>(i) + 2);
		}

		Text(Temp);
	}

	/* $ 13.01.2003 IS
	   - Принудительно уберем запрет отрисовки экрана, если количество кнопок
	     в сообщении равно нулю и макрос закончил выполняться. Это необходимо,
	     чтобы заработал прогресс-бар от плагина, который был запущен при помощи
	     макроса запретом отрисовки (bugz#533).
	*/

	if (Buttons.empty())
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

	TaskBarPause TBP;
	int Res = Message(MSG_WARNING|MSG_KILLSAVESCREEN,2,MSG(MKeyESCWasPressed),
	                  MSG((Global->Opt->Confirm.EscTwiceToInterrupt)?MDoYouWantToStopWork2:MDoYouWantToStopWork),
	                  MSG(MYes),MSG(MNo));

	if (Res == -1) // Set "ESC" equal to "NO" button
		Res = 1;

	return (Global->Opt->Confirm.EscTwiceToInterrupt && Res) || (!Global->Opt->Confirm.EscTwiceToInterrupt && !Res);
}
