/*
fastfind.cpp

Fast Find
*/
/*
Copyright © 2018 Far Group
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
#include "fastfind.hpp"

// Internal:
#include "clipboard.hpp"
#include "colormix.hpp"
#include "ctrlobj.hpp"
#include "filelist.hpp"
#include "global.hpp"
#include "help.hpp"
#include "editcontrol.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "keys.hpp"
#include "lang.hpp"
#include "panel.hpp"

// Platform:

// Common:
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

namespace
{
	// корректировка букв
	DWORD CorrectFastFindKbdLayout(const INPUT_RECORD& rec, DWORD Key)
	{
		if ((Key&(KEY_ALT | KEY_RALT)))// && Key!=(KEY_ALT|0x3C))
		{
			if (rec.Event.KeyEvent.uChar.UnicodeChar && (Key&KEY_MASKF) != rec.Event.KeyEvent.uChar.UnicodeChar) //???
				Key = (Key & 0xFFF10000) | rec.Event.KeyEvent.uChar.UnicodeChar;   //???
		}

		return Key;
	}
}

fastfind_ptr FastFind::create(Panel* Owner, const Manager::Key& FirstKey)
{
	const auto FastFindPtr = std::make_shared<FastFind>(private_tag(), Owner, FirstKey);
	FastFindPtr->init();
	return FastFindPtr;
}

FastFind::FastFind(private_tag, Panel* Owner, const Manager::Key& FirstKey):
	m_Owner(Owner),
	m_FirstKey(FirstKey)
{
}

bool FastFind::ProcessKey(const Manager::Key& Key)
{
	auto LocalKey = Key;

	// для вставки воспользуемся макродвижком...
	if (any_of(LocalKey(), KEY_CTRLV, KEY_RCTRLV, KEY_SHIFTINS, KEY_SHIFTNUMPAD0))
	{
		string ClipText;
		if (GetClipboardText(ClipText) && !ClipText.empty())
		{
			ProcessName(ClipText);
			ShowBorder();
		}

		return true;
	}
	else if (LocalKey() == KEY_OP_XLAT)
	{
		m_FindEdit->Xlat();
		const auto strTempName = m_FindEdit->GetString();
		m_FindEdit->ClearString();
		ProcessName(strTempName);
		Redraw();
		return true;
	}
	else if (LocalKey() == KEY_OP_PLAINTEXT)
	{
		m_FindEdit->ProcessKey(LocalKey);
		const auto strTempName = m_FindEdit->GetString();
		m_FindEdit->ClearString();
		ProcessName(strTempName);
		Redraw();
		return true;
	}
	else
		LocalKey = CorrectFastFindKbdLayout(Key.Event(), LocalKey());

	if (any_of(LocalKey(), KEY_ESC, KEY_F10))
	{
		Close(-1);
		return true;
	}

	if (LocalKey() >= KEY_ALT_BASE + 0x01 && LocalKey() <= KEY_ALT_BASE + 65535)
		LocalKey = lower(static_cast<wchar_t>(LocalKey() - KEY_ALT_BASE));
	else if (LocalKey() >= KEY_RALT_BASE + 0x01 && LocalKey() <= KEY_RALT_BASE + 65535)
		LocalKey = lower(static_cast<wchar_t>(LocalKey() - KEY_RALT_BASE));

	if (LocalKey() >= KEY_ALTSHIFT_BASE + 0x01 && LocalKey() <= KEY_ALTSHIFT_BASE + 65535)
		LocalKey = lower(static_cast<wchar_t>(LocalKey() - KEY_ALTSHIFT_BASE));
	else if (LocalKey() >= KEY_RALTSHIFT_BASE + 0x01 && LocalKey() <= KEY_RALTSHIFT_BASE + 65535)
		LocalKey = lower(static_cast<wchar_t>(LocalKey() - KEY_RALTSHIFT_BASE));

	if (LocalKey() == KEY_MULTIPLY)
		LocalKey = L'*';

	switch (LocalKey())
	{
	case KEY_F1:
	{
		Hide();
		{
			help::show(L"FastFind"sv);
		}
		Show();
		break;
	}

	case KEY_CTRLNUMENTER:   case KEY_RCTRLNUMENTER:
	case KEY_CTRLENTER:      case KEY_RCTRLENTER:
		m_Owner->FindPartName(m_FindEdit->GetString(), TRUE, 1);
		Redraw();
		break;

	case KEY_CTRLSHIFTNUMENTER:  case KEY_RCTRLSHIFTNUMENTER:
	case KEY_CTRLSHIFTENTER:     case KEY_RCTRLSHIFTENTER:
		m_Owner->FindPartName(m_FindEdit->GetString(), TRUE, -1);
		Redraw();
		break;

	case KEY_NONE:
		break;

	default:
		if ((LocalKey() < 32 || LocalKey() >= 65536) &&
			none_of(LocalKey(), KEY_BS, KEY_CTRLY, KEY_RCTRLY, KEY_CTRLBS, KEY_RCTRLBS, KEY_CTRLINS, KEY_CTRLNUMPAD0, KEY_SHIFTINS, KEY_SHIFTNUMPAD0) &&
			!IsModifKey(LocalKey()) &&
			!(any_of(LocalKey(), KEY_KILLFOCUS, KEY_GOTFOCUS) && IsWindowsVistaOrGreater()) // Mantis #2903
			)
		{
			m_KeyToProcess = LocalKey;
			Close(1);
			return true;
		}
		auto strLastName = m_FindEdit->GetString();
		if (m_FindEdit->ProcessKey(LocalKey))
		{
			auto strName = m_FindEdit->GetString();

			// уберем двойные '**'
			if (strName.size() > 1
				&& strName.back() == L'*'
				&& strName[strName.size() - 2] == L'*')
			{
				strName.pop_back();
				m_FindEdit->SetString(strName);
			}

			/* $ 09.04.2001 SVS
			   проблемы с быстрым поиском.
			   Подробнее в 00573.ChangeDirCrash.txt
			*/
			if (strName.starts_with(L'"'))
			{
				strName.erase(0, 1);
				m_FindEdit->SetString(strName);
			}

			if (m_Owner->FindPartName(strName, FALSE, 1))
			{
				strLastName = strName;
			}
			else
			{
				if (Global->CtrlObject->Macro.IsExecuting())// && Global->CtrlObject->Macro.GetLevelState() > 0) // если вставка макросом...
				{
					//Global->CtrlObject->Macro.DropProcess(); // ... то дропнем макропроцесс
					//Global->CtrlObject->Macro.PopState();
					// ;
				}

				m_FindEdit->SetString(strLastName);
			}

			Redraw();
		}

		break;
	}
	return true;
}

bool FastFind::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (MouseEvent->dwButtonState & 3)
		Close(-1);

	return true;
}

int FastFind::GetType() const
{
	return windowtype_search;
}

int FastFind::GetTypeAndName(string&, string&)
{
	return windowtype_search;
}

void FastFind::ResizeConsole()
{
	InitPositionAndSize();
}

void FastFind::Process()
{
	Global->WindowManager->ExecuteWindow(shared_from_this());
	Global->WindowManager->CallbackWindow([this]{ ProcessKey(m_FirstKey); });
	Global->WindowManager->ExecuteModal(shared_from_this());
}

const Manager::Key & FastFind::KeyToProcess() const
{
	return m_KeyToProcess;
}

void FastFind::DisplayObject()
{
	ShowBorder();
	m_FindEdit->Show();
}

string FastFind::GetTitle() const
{
	return {};
}

void FastFind::InitPositionAndSize()
{
	const auto OwnerRect = m_Owner->GetPosition();
	const auto FindX = std::min(OwnerRect.left + 9, ScrX - 22);
	const auto FindY = std::min(OwnerRect.bottom, ScrY - 2);
	SetPosition({ FindX, FindY, FindX + 21, FindY + 2 });
	m_FindEdit->SetPosition({ FindX + 2, FindY + 1, FindX + 19, FindY + 1 });
}

void FastFind::init()
{
	SetMacroMode(MACROAREA_SEARCH);
	SetRestoreScreenMode(true);

	m_FindEdit = std::make_unique<EditControl>(shared_from_this(), this);
	m_FindEdit->SetEditBeyondEnd(false);
	m_FindEdit->SetObjectColor(COL_DIALOGEDIT);

	InitPositionAndSize();
}

void FastFind::ProcessName(string_view const Src) const
{
	auto Buffer = unquote(m_FindEdit->GetString() + Src);

	while (!Buffer.empty() && !m_Owner->FindPartName(Buffer, FALSE, 1))
		Buffer.pop_back();

	if (!Buffer.empty())
	{
		m_FindEdit->SetString(Buffer);
		m_FindEdit->Show();
	}
}

void FastFind::ShowBorder() const
{
	SetColor(COL_DIALOGTEXT);
	GotoXY(m_Where.left + 1, m_Where.top + 1);
	Text(L' ');
	GotoXY(m_Where.left + 20, m_Where.top + 1);
	Text(L' ');
	Box({ m_Where.left, m_Where.top, m_Where.left + 21, m_Where.top + 2 }, colors::PaletteColorToFarColor(COL_DIALOGBOX), DOUBLE_BOX);
	GotoXY(m_Where.left + 7, m_Where.top);
	SetColor(COL_DIALOGBOXTITLE);
	Text(L' ');
	Text(lng::MSearchFileTitle);
	Text(L' ');
}

void FastFind::Close(int ExitCode)
{
	SetExitCode(ExitCode);
	Hide();
	Global->WindowManager->DeleteWindow(shared_from_this());
}
