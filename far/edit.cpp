/*
edit.cpp

Строка редактора
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
#include "edit.hpp"

// Internal:
#include "keyboard.hpp"
#include "macroopcode.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "interf.hpp"
#include "clipboard.hpp"
#include "xlat.hpp"
#include "shortcuts.hpp"
#include "panelmix.hpp"
#include "manager.hpp"
#include "editor.hpp"
#include "window.hpp"
#include "colormix.hpp"
#include "strmix.hpp"
#include "global.hpp"

// Platform:
#include "platform.env.hpp"

// Common:

// External:

//----------------------------------------------------------------------------

void ColorItem::SetOwner(const UUID& Value)
{
	static std::unordered_set<UUID> UuidSet;
	Owner = &*UuidSet.emplace(Value).first;
}

void ColorItem::SetColor(const FarColor& Value)
{
	Color = colors::StoreColor(Value);
}

static int Recurse=0;

static const wchar_t EDMASK_ANY    = L'X'; // позволяет вводить в строку ввода любой символ;
static const wchar_t EDMASK_DSS    = L'#'; // позволяет вводить в строку ввода цифры, пробел и знак минуса;
static const wchar_t EDMASK_DIGIT  = L'9'; // позволяет вводить в строку ввода только цифры;
static const wchar_t EDMASK_DIGITS = L'N'; // позволяет вводить в строку ввода только цифры и пробелы;
static const wchar_t EDMASK_ALPHA  = L'A'; // позволяет вводить в строку ввода только буквы.
static const wchar_t EDMASK_HEX    = L'H'; // позволяет вводить в строку ввода шестнадцатиричные символы.

Edit::Edit(window_ptr Owner):
	SimpleScreenObject(std::move(Owner)),
	m_CurPos(0),
	m_SelStart(-1),
	m_SelEnd(0),
	LeftPos(0),
	m_Eol(eol::none)
{
	m_Flags.Set(FEDITLINE_EDITBEYONDEND);
	const auto& EdOpt = Global->Opt->EdOpt;
	m_Flags.Change(FEDITLINE_DELREMOVESBLOCKS, EdOpt.DelRemovesBlocks);
	m_Flags.Change(FEDITLINE_PERSISTENTBLOCKS, EdOpt.PersistentBlocks);
	m_Flags.Change(FEDITLINE_SHOWWHITESPACE, EdOpt.ShowWhiteSpace != 0);
	m_Flags.Change(FEDITLINE_SHOWLINEBREAK, EdOpt.ShowWhiteSpace == 1);
}

void Edit::DisplayObject()
{
	if (m_Flags.Check(FEDITLINE_DROPDOWNBOX))
	{
		m_Flags.Clear(FEDITLINE_CLEARFLAG);  // при дроп-даун нам не нужно никакого unchanged text
		m_SelStart=0;
		m_SelEnd = m_Str.size(); // а также считаем что все выделено -
		//    надо же отличаться от обычных Edit
	}

	//   Вычисление нового положения курсора в строке с учётом Mask.
	const auto Value = GetPrevCurPos() > m_CurPos? -1 : 1;
	m_CurPos=GetNextCursorPos(m_CurPos,Value);
	FastShow();

	/* $ 26.07.2000 tran
	   при DropDownBox курсор выключаем
	   не знаю даже - попробовал но не очень красиво вышло */
	if (m_Flags.Check(FEDITLINE_DROPDOWNBOX))
		::SetCursorType(false, 10);
	else
	{
		if (m_Flags.Check(FEDITLINE_OVERTYPE))
		{
			const auto NewCursorSize = IsConsoleFullscreen()?
			                  (Global->Opt->CursorSize[3]? static_cast<int>(Global->Opt->CursorSize[3]) : 99):
			                  (Global->Opt->CursorSize[2]? static_cast<int>(Global->Opt->CursorSize[2]) : 99);
			::SetCursorType(true, GetCursorSize() == -1?NewCursorSize:GetCursorSize());
		}
		else
		{
			const auto NewCursorSize = IsConsoleFullscreen()?
			                  (Global->Opt->CursorSize[1]? static_cast<int>(Global->Opt->CursorSize[1]) : 10):
			                  (Global->Opt->CursorSize[0]? static_cast<int>(Global->Opt->CursorSize[0]) : 10);
			::SetCursorType(true, GetCursorSize() == -1?NewCursorSize:GetCursorSize());
		}
	}

	MoveCursor({ m_Where.left + GetLineCursorPos() - LeftPos, m_Where.top });
}

void Edit::SetCursorType(bool const Visible, size_t const Size)
{
	m_Flags.Change(FEDITLINE_CURSORVISIBLE,Visible);
	SetCursorSize(Size);
	::SetCursorType(Visible,Size);
}

void Edit::GetCursorType(bool& Visible, size_t& Size) const
{
	Visible=m_Flags.Check(FEDITLINE_CURSORVISIBLE);
	Size = GetCursorSize();
}

//   Вычисление нового положения курсора в строке с учётом Mask.
int Edit::GetNextCursorPos(int Position,int Where) const
{
	int Result = Position;
	const auto Mask = GetInputMask();

	if (!Mask.empty() && (Where==-1 || Where==1))
	{
		int PosChanged=FALSE;
		const auto MaskLen = static_cast<int>(Mask.size());

		for (int i=Position; i<MaskLen && i>=0; i+=Where)
		{
			if (CheckCharMask(Mask[i]))
			{
				Result=i;
				PosChanged=TRUE;
				break;
			}
		}

		if (!PosChanged)
		{
			for (int i=std::min(Position, static_cast<int>(Mask.size())); i>=0; i--)
			{
				if (CheckCharMask(Mask[i]))
				{
					Result=i;
					PosChanged=TRUE;
					break;
				}
			}
		}

		if (!PosChanged)
		{
			const auto It = std::find_if(ALL_CONST_RANGE(Mask), CheckCharMask);
			if (It != Mask.cend())
			{
				Result = It - Mask.cbegin();
			}
		}
	}

	return Result;
}

void Edit::FastShow(const Edit::ShowInfo* Info)
{
	const size_t EditLength=ObjWidth();

	if (!m_Flags.Check(FEDITLINE_EDITBEYONDEND) && m_CurPos > m_Str.size())
		m_CurPos = m_Str.size();

	if (GetMaxLength()!=-1)
	{
		if (m_Str.size() > GetMaxLength())
		{
			m_Str.resize(GetMaxLength());
		}

		if (m_CurPos>GetMaxLength()-1)
			m_CurPos=GetMaxLength()>0 ? (GetMaxLength()-1):0;
	}

	const auto TabCurPos = GetTabCurPos();

	/* $ 31.07.2001 KM
	  ! Для комбобокса сделаем отображение строки
	    с первой позиции.
	*/
	if (!m_Flags.Check(FEDITLINE_DROPDOWNBOX))
	{
		FixLeftPos(TabCurPos);
	}

	int FocusedLeftPos = LeftPos, XPos = TabCurPos - LeftPos;
	if(Info)
	{
		FocusedLeftPos = Info->LeftPos;
		XPos = Info->CurTabPos - Info->LeftPos;
	}

	GotoXY(m_Where.left, m_Where.top);
	int TabSelStart=(m_SelStart==-1) ? -1:RealPosToTab(m_SelStart);
	int TabSelEnd=(m_SelEnd<0) ? -1:RealPosToTab(m_SelEnd);

	/* $ 17.08.2000 KM
	   Если есть маска, сделаем подготовку строки, то есть
	   все "постоянные" символы в маске, не являющиеся шаблонными
	   должны постоянно присутствовать в Str
	*/
	const auto Mask = GetInputMask();
	if (!Mask.empty())
		RefreshStrByMask();

	string OutStr, OutStrTmp;
	OutStr.reserve(EditLength);
	OutStrTmp.reserve(EditLength);

	SetLineCursorPos(TabCurPos);
	const auto RealLeftPos = TabPosToReal(LeftPos);

	if (m_Str.size() > RealLeftPos)
	{
		OutStrTmp.assign(m_Str, RealLeftPos, std::min(static_cast<int>(EditLength), m_Str.size() - RealLeftPos));
	}

	{
		auto TrailingSpaces = OutStrTmp.cend();
		if (m_Flags.Check(FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE) && Mask.empty() && !OutStrTmp.empty())
		{
			TrailingSpaces = std::find_if_not(OutStrTmp.crbegin(), OutStrTmp.crend(), [](wchar_t i) { return std::iswblank(i);}).base();
		}

		FOR_RANGE(OutStrTmp, i)
		{
			if ((m_Flags.Check(FEDITLINE_SHOWWHITESPACE) && m_Flags.Check(FEDITLINE_EDITORMODE)) || i >= TrailingSpaces)
			{
				if (*i==L' ') // *p==L'\xA0' ==> NO-BREAK SPACE
				{
					*i=L'\xB7';
				}
			}

			if (*i == L'\t')
			{
				const size_t S = GetTabSize() - ((FocusedLeftPos + OutStr.size()) % GetTabSize());
				OutStr.push_back((((m_Flags.Check(FEDITLINE_SHOWWHITESPACE) && m_Flags.Check(FEDITLINE_EDITORMODE)) || i >= TrailingSpaces) && (!OutStr.empty() || S==GetTabSize()))?L'\x2192':L' ');
				const auto PaddedSize = std::min(OutStr.size() + S - 1, EditLength);
				OutStr.resize(PaddedSize, L' ');
			}
			else
			{
				OutStr.push_back(!*i? L' ' : *i);
			}

			if (OutStr.size() >= EditLength)
			{
				break;
			}
		}

		if (m_Flags.Check(FEDITLINE_PASSWORDMODE))
			OutStr.assign(OutStr.size(), L'*');

		if (m_Flags.Check(FEDITLINE_SHOWLINEBREAK) && m_Flags.Check(FEDITLINE_EDITORMODE) && (m_Str.size() >= RealLeftPos) && (OutStr.size() < EditLength))
		{
			if (m_Eol == eol::mac)
			{
				OutStr.push_back(L'\x266A');
			}
			else if (m_Eol == eol::unix)
			{
				OutStr.push_back(L'\x25D9');
			}
			else if (m_Eol == eol::win)
			{
				OutStr.push_back(L'\x266A');
				if(OutStr.size() < EditLength)
				{
					OutStr.push_back(L'\x25D9');
				}
			}
			else if (m_Eol == eol::bad_win)
			{
				OutStr.push_back(L'\x266A');
				if(OutStr.size() < EditLength)
				{
					OutStr.push_back(L'\x266A');
					if(OutStr.size() < EditLength)
					{
						OutStr.push_back(L'\x25D9');
					}
				}
			}
		}

		if (m_Flags.Check(FEDITLINE_SHOWWHITESPACE) && m_Flags.Check(FEDITLINE_EDITORMODE) && (m_Str.size() >= RealLeftPos) && (OutStr.size() < EditLength) && GetEditor()->IsLastLine(this))
		{
			OutStr.push_back(L'\x25a1');
		}
	}

	SetColor(GetNormalColor());

	if (TabSelStart==-1)
	{
		if (m_Flags.Check(FEDITLINE_CLEARFLAG))
		{
			SetColor(GetUnchangedColor());

			if (!Mask.empty())
				inplace::trim_right(OutStr);

			Text(OutStr);
			SetColor(GetNormalColor());
			const auto BlankLength = EditLength - OutStr.size();

			if (BlankLength > 0)
			{
				Text(string(BlankLength, L' '));
			}
		}
		else
		{
			Text(fit_to_left(OutStr, EditLength));
		}
	}
	else
	{
		TabSelStart = std::max(TabSelStart - LeftPos, 0);

		TabSelEnd = TabSelEnd == -1?
			static_cast<int>(EditLength) :
			std::max(TabSelEnd - LeftPos, 0);

		OutStr.append(EditLength - OutStr.size(), L' ');

		Text(cut_right(OutStr, TabSelStart));
		SetColor(GetSelectedColor());

		if (!m_Flags.Check(FEDITLINE_DROPDOWNBOX))
		{
			if (TabSelStart < static_cast<int>(EditLength))
			{
				Text(cut_right(string_view(OutStr).substr(TabSelStart), TabSelEnd - TabSelStart));

				if (TabSelEnd < static_cast<int>(EditLength))
				{
					//SetColor(Flags.Check(FEDITLINE_CLEARFLAG)? SelColor : Color);
					SetColor(GetNormalColor());
					Text(string_view(OutStr).substr(TabSelEnd));
				}
			}

		}
		else
		{
			Text(cut_right(OutStr, m_Where.width()));
		}
	}

	/* $ 26.07.2000 tran
	   при дроп-даун цвета нам не нужны */
	if (!m_Flags.Check(FEDITLINE_DROPDOWNBOX))
		ApplyColor(GetSelectedColor(), XPos, FocusedLeftPos);
}

bool Edit::RecurseProcessKey(int Key)
{
	Recurse++;
	const auto RetCode=ProcessKey(Manager::Key(Key));
	Recurse--;
	return RetCode;
}

// Функция вставки всякой хреновени - от шорткатов до имен файлов
bool Edit::ProcessInsPath(unsigned int Key,int PrevSelStart,int PrevSelEnd)
{
	Shortcuts::data Data;

	if (Key >= KEY_RCTRL0 && Key <= KEY_RCTRL9)
	{
		if (!Shortcuts(Key - KEY_RCTRL0).Get(Data))
			return false;
		Data.Folder = os::env::expand(Data.Folder);
	}
	else
	{
		if (!MakePathForUI(Key, Data.Folder))
			return false;
	}

	if (PrevSelStart != -1)
	{
		m_SelStart = PrevSelStart;
		m_SelEnd = PrevSelEnd;
	}

	InsertString(Data.Folder);

	return true;
}

long long Edit::VMProcess(int OpCode, void* vParam, long long iParam)
{
	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return !GetLength();
		case MCODE_C_SELECTED:
			return m_SelStart != -1 && m_SelStart < m_SelEnd;
		case MCODE_C_EOF:
			return m_CurPos >= m_Str.size();
		case MCODE_C_BOF:
			return !m_CurPos;
		case MCODE_V_ITEMCOUNT:
			return m_Str.size();
		case MCODE_V_CURPOS:
			return GetLineCursorPos()+1;
		case MCODE_F_EDITOR_SEL:
		{
			const auto Action = static_cast<int>(reinterpret_cast<intptr_t>(vParam));
			if (Action) m_Flags.Clear(FEDITLINE_CLEARFLAG);

			switch (Action)
			{
				case 0:  // Get Param
				{
					switch (iParam)
					{
						case 0:  // return FirstLine
						case 2:  // return LastLine
							return IsSelection()?1:0;
						case 1:  // return FirstPos
							return IsSelection()?m_SelStart+1:0;
						case 3:  // return LastPos
							return IsSelection()?m_SelEnd:0;
						case 4: // return block type (0=nothing 1=stream, 2=column)
							return IsSelection()?1:0;
					}

					break;
				}
				case 1:  // Set Pos
				{
					if (IsSelection())
					{
						switch (iParam)
						{
							case 0: // begin block (FirstLine & FirstPos)
							case 1: // end block (LastLine & LastPos)
							{
								SetTabCurPos(iParam?m_SelEnd:m_SelStart);
								Show();
								return 1;
							}
						}
					}

					break;
				}
				case 2: // Set Stream Selection Edge
				case 3: // Set Column Selection Edge
				{
					switch (iParam)
					{
						case 0:  // selection start
						{
							SetMacroSelectionStart(GetTabCurPos());
							return 1;
						}
						case 1:  // selection finish
						{
							if (GetMacroSelectionStart() != -1)
							{
								if (GetMacroSelectionStart() != GetTabCurPos())
									Select(GetMacroSelectionStart(),GetTabCurPos());
								else
									RemoveSelection();

								Show();
								SetMacroSelectionStart(-1);
								return 1;
							}

							return 0;
						}
					}

					break;
				}
				case 4: // UnMark sel block
				{
					RemoveSelection();
					SetMacroSelectionStart(-1);
					Show();
					return 1;
				}
			}

			break;
		}
	}

	return 0;
}

bool Edit::ProcessKey(const Manager::Key& Key)
{
	auto LocalKey = Key();
	const auto Mask = GetInputMask();

	const auto CharOrPredefined = [&](wchar_t const Predefined)
	{
		if (const auto Char = Key.Event().Event.KeyEvent.uChar.UnicodeChar)
			return Char;

		return Predefined;
	};

	switch (LocalKey)
	{
		case KEY_ADD:
			LocalKey = CharOrPredefined(L'+');
			break;

		case KEY_SUBTRACT:
			LocalKey = CharOrPredefined(L'-');
			break;

		case KEY_MULTIPLY:
			LocalKey = CharOrPredefined(L'*');
			break;

		case KEY_DIVIDE:
			LocalKey = CharOrPredefined(L'/');
			break;

		case KEY_DECIMAL:
			LocalKey = CharOrPredefined(L'.');
			break;

		case KEY_CTRLC:
		case KEY_RCTRLC:
			LocalKey=KEY_CTRLINS;
			break;

		case KEY_CTRLV:
		case KEY_RCTRLV:
			LocalKey=KEY_SHIFTINS;
			break;

		case KEY_CTRLX:
		case KEY_RCTRLX:
			LocalKey=KEY_SHIFTDEL;
			break;
	}

	int PrevSelStart=-1,PrevSelEnd=0;

	if (!m_Flags.Check(FEDITLINE_DROPDOWNBOX) && any_of(LocalKey, KEY_CTRLL, KEY_RCTRLL))
	{
		m_Flags.Invert(FEDITLINE_READONLY);
	}

	if (
		((m_Flags.Check(FEDITLINE_DELREMOVESBLOCKS) && any_of(LocalKey, KEY_BS, KEY_DEL, KEY_NUMDEL)) || any_of(LocalKey, KEY_CTRLD, KEY_RCTRLD)) &&
		!m_Flags.Check(FEDITLINE_EDITORMODE) && m_SelStart != -1 && m_SelStart < m_SelEnd
	)
	{
		DeleteBlock();
		Show();
		return true;
	}

	if (
		!IntKeyState.ShiftPressed() &&
		(!Global->CtrlObject->Macro.IsExecuting() || IsNavKey(LocalKey)) &&
		!IsShiftKey(LocalKey) &&
		!Recurse &&
		// No single ctrl, alt, shift key or their combination
		LocalKey & ~KEY_CTRLMASK &&
		none_of(LocalKey, KEY_NONE, KEY_INS, KEY_KILLFOCUS, KEY_GOTFOCUS) &&
		none_of(LocalKey & ~KEY_CTRLMASK, KEY_LWIN, KEY_RWIN, KEY_APPS)
	)
	{
		m_Flags.Clear(FEDITLINE_MARKINGBLOCK);

		if (
			!m_Flags.CheckAny(FEDITLINE_PERSISTENTBLOCKS | FEDITLINE_EDITORMODE) &&
			none_of(LocalKey,
				KEY_CTRLINS, KEY_RCTRLINS, KEY_CTRLNUMPAD0, KEY_RCTRLNUMPAD0,
				KEY_SHIFTDEL, KEY_SHIFTNUMDEL, KEY_SHIFTDECIMAL,
				KEY_CTRLQ, KEY_RCTRLQ,
				KEY_SHIFTINS, KEY_SHIFTNUMPAD0)
			)
		{
			if (m_SelStart != -1 || m_SelEnd )
			{
				PrevSelStart=m_SelStart;
				PrevSelEnd=m_SelEnd;
				RemoveSelection();
				Show();
			}
		}
	}

	if (((Global->Opt->Dialogs.EULBsClear && LocalKey == KEY_BS) || any_of(LocalKey, KEY_DEL, KEY_NUMDEL)) && m_Flags.Check(FEDITLINE_CLEARFLAG) && m_CurPos >= m_Str.size())
		LocalKey=KEY_CTRLY;

	if (any_of(LocalKey, KEY_SHIFTDEL, KEY_SHIFTNUMDEL, KEY_SHIFTDECIMAL) && m_Flags.Check(FEDITLINE_CLEARFLAG) && m_CurPos >= m_Str.size() && m_SelStart == -1)
	{
		m_SelStart=0;
		m_SelEnd = m_Str.size();
	}

	if (
		m_Flags.Check(FEDITLINE_CLEARFLAG) &&
		(
			(LocalKey <= 0xFFFF && LocalKey!=KEY_BS) ||
			any_of(LocalKey,
				KEY_CTRLBRACKET,
				KEY_RCTRLBRACKET,
				KEY_CTRLBACKBRACKET,
				KEY_RCTRLBACKBRACKET,
				KEY_CTRLSHIFTBRACKET,
				KEY_RCTRLSHIFTBRACKET,
				KEY_CTRLSHIFTBACKBRACKET,
				KEY_RCTRLSHIFTBACKBRACKET,
				KEY_SHIFTENTER,
				KEY_SHIFTNUMENTER)
		)
	)
	{
		LeftPos=0;
		{
			SCOPED_ACTION(auto)(CallbackSuppressor());
			ClearString(); // mantis#0001722
		}
		Show();
	}

	// Здесь - вызов функции вставки путей/файлов
	if (ProcessInsPath(LocalKey,PrevSelStart,PrevSelEnd))
	{
		Show();
		return true;
	}

	switch (LocalKey)
	{
		case KEY_CTRLA: case KEY_RCTRLA:
			{
				Select(0, GetLength());
				Show();
			}
			break;
		case KEY_SHIFTLEFT: case KEY_SHIFTNUMPAD4:
		{
			if (m_CurPos>0)
			{
				AdjustPersistentMark();

				RecurseProcessKey(KEY_LEFT);

				if (!m_Flags.Check(FEDITLINE_MARKINGBLOCK))
				{
					RemoveSelection();
					m_Flags.Set(FEDITLINE_MARKINGBLOCK);
				}

				if (m_SelStart!=-1 && m_SelStart<=m_CurPos)
					Select(m_SelStart,m_CurPos);
				else
				{
					int EndPos=m_CurPos+1;
					int NewStartPos=m_CurPos;

					if (EndPos>m_Str.size())
						EndPos = m_Str.size();

					if (NewStartPos>m_Str.size())
						NewStartPos = m_Str.size();

					AddSelect(NewStartPos,EndPos);
				}

				Show();
			}

			return true;
		}
		case KEY_SHIFTRIGHT: case KEY_SHIFTNUMPAD6:
		{
			AdjustPersistentMark();

			if (!m_Flags.Check(FEDITLINE_MARKINGBLOCK))
			{
				RemoveSelection();
				m_Flags.Set(FEDITLINE_MARKINGBLOCK);
			}

			if ((m_SelStart!=-1 && m_SelEnd==-1) || m_SelEnd>m_CurPos)
			{
				if (m_CurPos+1==m_SelEnd)
					RemoveSelection();
				else
					Select(m_CurPos+1,m_SelEnd);
			}
			else
				AddSelect(m_CurPos,m_CurPos+1);

			RecurseProcessKey(KEY_RIGHT);
			return true;
		}
		case KEY_CTRLSHIFTLEFT:  case KEY_CTRLSHIFTNUMPAD4:
		case KEY_RCTRLSHIFTLEFT: case KEY_RCTRLSHIFTNUMPAD4:
		{
			if (m_CurPos>m_Str.size())
			{
				SetPrevCurPos(m_CurPos);
				m_CurPos = m_Str.size();
			}

			if (m_CurPos>0)
				RecurseProcessKey(KEY_SHIFTLEFT);

			while (m_CurPos>0 && !(!IsWordDiv(WordDiv(), m_Str[m_CurPos]) &&
			                     IsWordDiv(WordDiv(),m_Str[m_CurPos-1]) && !std::iswblank(m_Str[m_CurPos])))
			{
				if (!std::iswblank(m_Str[m_CurPos]) && (std::iswblank(m_Str[m_CurPos-1]) ||
				                              IsWordDiv(WordDiv(), m_Str[m_CurPos-1])))
					break;

				RecurseProcessKey(KEY_SHIFTLEFT);
			}

			Show();
			return true;
		}
		case KEY_CTRLSHIFTRIGHT:  case KEY_CTRLSHIFTNUMPAD6:
		case KEY_RCTRLSHIFTRIGHT: case KEY_RCTRLSHIFTNUMPAD6:
		{
			if (m_CurPos >= m_Str.size())
				return false;

			RecurseProcessKey(KEY_SHIFTRIGHT);

			while (m_CurPos < m_Str.size() && !(IsWordDiv(WordDiv(), m_Str[m_CurPos]) &&
			                           !IsWordDiv(WordDiv(), m_Str[m_CurPos-1])))
			{
				if (!std::iswblank(m_Str[m_CurPos]) && (std::iswblank(m_Str[m_CurPos-1]) || IsWordDiv(WordDiv(), m_Str[m_CurPos-1])))
					break;

				RecurseProcessKey(KEY_SHIFTRIGHT);

				if (GetMaxLength()!=-1 && m_CurPos==GetMaxLength()-1)
					break;
			}

			Show();
			return true;
		}
		case KEY_SHIFTHOME:  case KEY_SHIFTNUMPAD7:
		{
			while (m_CurPos>0)
				RecurseProcessKey(KEY_SHIFTLEFT);
			Show();
			return true;
		}
		case KEY_SHIFTEND:  case KEY_SHIFTNUMPAD1:
		{
			int Len;

			if (!Mask.empty())
			{
				Len = static_cast<int>(trim_right(string_view(m_Str)).size());
			}
			else
				Len = m_Str.size();

			int LastCurPos=m_CurPos;

			while (m_CurPos<Len/*StrSize*/)
			{
				RecurseProcessKey(KEY_SHIFTRIGHT);

				if (LastCurPos==m_CurPos)break;

				LastCurPos=m_CurPos;
			}
			Show();
			return true;
		}
		case KEY_BS:
		{
			if (m_CurPos<=0)
				return false;

			SetPrevCurPos(m_CurPos);
			do
			{
				--m_CurPos;
			}
			while (!Mask.empty() && m_CurPos > 0 && !CheckCharMask(Mask[m_CurPos]));

			if (m_CurPos<=LeftPos)
			{
				LeftPos-=15;

				if (LeftPos<0)
					LeftPos=0;
			}

			if (!RecurseProcessKey(KEY_DEL))
				Show();

			return true;
		}
		case KEY_CTRLSHIFTBS:
		case KEY_RCTRLSHIFTBS:
		{
			{
				SCOPED_ACTION(auto)(CallbackSuppressor());

				// BUGBUG
				for (int i = m_CurPos; i >= 0; i--)
				{
					RecurseProcessKey(KEY_BS);
				}
			}
			Changed(true);
			Show();
			return true;
		}
		case KEY_CTRLBS:
		case KEY_RCTRLBS:
		{
			if (m_CurPos > m_Str.size())
			{
				SetPrevCurPos(m_CurPos);
				m_CurPos = m_Str.size();
			}

			{
				SCOPED_ACTION(auto)(CallbackSuppressor());

				// BUGBUG
				for (;;)
				{
					int StopDelete = FALSE;

					if (m_CurPos > 1 && std::iswblank(m_Str[m_CurPos - 1]) != std::iswblank(m_Str[m_CurPos - 2]))
						StopDelete = TRUE;

					RecurseProcessKey(KEY_BS);

					if (!m_CurPos || StopDelete)
						break;

					if (IsWordDiv(WordDiv(), m_Str[m_CurPos - 1]))
						break;
				}

			}
			Changed(true);
			Show();
			return true;
		}
		case KEY_CTRLQ:
		case KEY_RCTRLQ:
		{
			if (!m_Flags.Check(FEDITLINE_PERSISTENTBLOCKS) && (m_SelStart != -1 || m_Flags.Check(FEDITLINE_CLEARFLAG)))
				RecurseProcessKey(KEY_DEL);
			ProcessCtrlQ();
			Show();
			return true;
		}
		case KEY_OP_SELWORD:
		{
			const auto OldCurPos=m_CurPos;
			PrevSelStart=m_SelStart;
			PrevSelEnd=m_SelEnd;
#if defined(MOUSEKEY)

			if (m_CurPos >= m_SelStart && m_CurPos <= m_SelEnd)
			{ // выделяем ВСЮ строку при повторном двойном клике
				Select(0,m_StrSize);
			}
			else
#endif
			{
				size_t SBegin, SEnd;

				if (FindWordInString(m_Str, m_CurPos, SBegin, SEnd, WordDiv()))
					Select(static_cast<int>(SBegin), static_cast<int>(SEnd));
			}

			m_CurPos=OldCurPos; // возвращаем обратно
			Show();
			return true;
		}
		case KEY_OP_PLAINTEXT:
		{
			InsertString(Global->CtrlObject->Macro.GetStringToPrint());
			Show();
			return true;
		}
		case KEY_CTRLT:
		case KEY_CTRLDEL:
		case KEY_CTRLNUMDEL:
		case KEY_CTRLDECIMAL:
		case KEY_RCTRLT:
		case KEY_RCTRLDEL:
		case KEY_RCTRLNUMDEL:
		case KEY_RCTRLDECIMAL:
		{
			if (m_CurPos >= m_Str.size())
				return false;
			{
				SCOPED_ACTION(auto)(CallbackSuppressor());
				if (!Mask.empty())
				{
					const auto MaskLen = static_cast<int>(Mask.size());
					int ptr = m_CurPos;

					while (ptr < MaskLen)
					{
						ptr++;

						if (!CheckCharMask(Mask[ptr]) ||
							(std::iswblank(m_Str[ptr]) && !std::iswblank(m_Str[ptr + 1])) ||
							(IsWordDiv(WordDiv(), m_Str[ptr])))
							break;
					}

					// BUGBUG
					for (int i = 0; i < ptr - m_CurPos; i++)
						RecurseProcessKey(KEY_DEL);
				}
				else
				{
					for (;;)
					{
						int StopDelete = FALSE;

						if (m_CurPos < m_Str.size() - 1 && std::iswblank(m_Str[m_CurPos]) && !std::iswblank(m_Str[m_CurPos + 1]))
							StopDelete = TRUE;

						RecurseProcessKey(KEY_DEL);

						if (m_CurPos >= m_Str.size() || StopDelete)
							break;

						if (IsWordDiv(WordDiv(), m_Str[m_CurPos]))
							break;
					}
				}
			}
			Changed(true);
			Show();
			return true;
		}
		case KEY_CTRLY:
		case KEY_RCTRLY:
		{
			if (m_Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
				return true;

			SetPrevCurPos(m_CurPos);
			LeftPos=m_CurPos=0;
			clear_and_shrink(m_Str);
			RemoveSelection();
			Changed();
			Show();
			return true;
		}
		case KEY_CTRLK:
		case KEY_RCTRLK:
		{
			if (m_Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
				return true;

			if (m_CurPos >= m_Str.size())
				return false;

			if (!m_Flags.Check(FEDITLINE_EDITBEYONDEND))
			{
				if (m_CurPos<m_SelEnd)
					m_SelEnd=m_CurPos;

				if (m_SelEnd<m_SelStart && m_SelEnd!=-1)
				{
					m_SelEnd=0;
					m_SelStart=-1;
				}
			}

			m_Str.resize(m_CurPos);
			Changed();
			Show();
			return true;
		}
		case KEY_HOME:        case KEY_NUMPAD7:
		case KEY_CTRLHOME:    case KEY_CTRLNUMPAD7:
		case KEY_RCTRLHOME:   case KEY_RCTRLNUMPAD7:
		{
			SetPrevCurPos(m_CurPos);
			m_CurPos=0;
			Show();
			return true;
		}
		case KEY_END:           case KEY_NUMPAD1:
		case KEY_CTRLEND:       case KEY_CTRLNUMPAD1:
		case KEY_RCTRLEND:      case KEY_RCTRLNUMPAD1:
		case KEY_CTRLSHIFTEND:  case KEY_CTRLSHIFTNUMPAD1:
		case KEY_RCTRLSHIFTEND: case KEY_RCTRLSHIFTNUMPAD1:
		{
			SetPrevCurPos(m_CurPos);

			if (!Mask.empty())
			{
				m_CurPos = static_cast<int>(trim_right(string_view(m_Str)).size());
			}
			else
				m_CurPos = m_Str.size();

			Show();
			return true;
		}
		case KEY_LEFT:        case KEY_NUMPAD4:        case KEY_MSWHEEL_LEFT:
		case KEY_CTRLS:       case KEY_RCTRLS:
		{
			if (m_CurPos>0)
			{
				SetPrevCurPos(m_CurPos);
				m_CurPos--;
				Show();
			}

			return true;
		}
		case KEY_RIGHT:       case KEY_NUMPAD6:        case KEY_MSWHEEL_RIGHT:
		case KEY_CTRLD:       case KEY_RCTRLD:
		{
			SetPrevCurPos(m_CurPos);

			if (!Mask.empty())
			{
				const auto Len = static_cast<int>(trim_right(string_view(m_Str)).size());

				if (Len>m_CurPos)
					m_CurPos++;
			}
			else
				m_CurPos++;

			Show();
			return true;
		}
		case KEY_INS:         case KEY_NUMPAD0:
		{
			m_Flags.Invert(FEDITLINE_OVERTYPE);
			Show();
			return true;
		}
		case KEY_NUMDEL:
		case KEY_DEL:
		{
			if (m_Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
				return true;

			if (m_CurPos >= m_Str.size())
				return false;

			if (m_SelStart!=-1)
			{
				if (m_SelEnd!=-1 && m_CurPos<m_SelEnd)
					m_SelEnd--;

				if (m_CurPos<m_SelStart)
					m_SelStart--;

				if (m_SelEnd!=-1 && m_SelEnd<=m_SelStart)
				{
					m_SelStart=-1;
					m_SelEnd=0;
				}
			}

			if (!Mask.empty())
			{
				const size_t MaskLen = Mask.size();
				size_t j = m_CurPos;
				for (size_t i = m_CurPos; i < MaskLen; ++i)
				{
					if (i+1 < MaskLen && CheckCharMask(Mask[i+1]))
					{
						while (j < MaskLen && !CheckCharMask(Mask[j]))
							j++;

						if (!CharInMask(m_Str[i + 1], Mask[j]))
							break;

						m_Str[j]=m_Str[i+1];
						j++;
					}
				}

				m_Str[j]=L' ';
			}
			else
			{
				m_Str.erase(m_CurPos, 1);
			}

			Changed(true);
			Show();
			return true;
		}
		case KEY_CTRLLEFT:  case KEY_CTRLNUMPAD4:
		case KEY_RCTRLLEFT: case KEY_RCTRLNUMPAD4:
		{
			SetPrevCurPos(m_CurPos);

			if (m_CurPos > m_Str.size())
				m_CurPos = m_Str.size();

			if (m_CurPos>0)
				m_CurPos--;

			while (m_CurPos>0 && !(!IsWordDiv(WordDiv(), m_Str[m_CurPos]) &&
			                     IsWordDiv(WordDiv(), m_Str[m_CurPos-1]) && !std::iswblank(m_Str[m_CurPos])))
			{
				if (!std::iswblank(m_Str[m_CurPos]) && std::iswblank(m_Str[m_CurPos-1]))
					break;

				m_CurPos--;
			}

			Show();
			return true;
		}
		case KEY_CTRLRIGHT:   case KEY_CTRLNUMPAD6:
		case KEY_RCTRLRIGHT:  case KEY_RCTRLNUMPAD6:
		{
			if (m_CurPos >= m_Str.size())
				return false;

			SetPrevCurPos(m_CurPos);
			int Len;

			if (!Mask.empty())
			{
				Len = static_cast<int>(trim_right(string_view(m_Str)).size());

				if (Len>m_CurPos)
					m_CurPos++;
			}
			else
			{
				Len = m_Str.size();
				m_CurPos++;
			}

			while (m_CurPos<Len/*StrSize*/ && !(IsWordDiv(WordDiv(),m_Str[m_CurPos]) &&
			                                  !IsWordDiv(WordDiv(), m_Str[m_CurPos-1])))
			{
				if (!std::iswblank(m_Str[m_CurPos]) && std::iswblank(m_Str[m_CurPos-1]))
					break;

				m_CurPos++;
			}

			Show();
			return true;
		}
		case KEY_SHIFTNUMDEL:
		case KEY_SHIFTDECIMAL:
		case KEY_SHIFTDEL:
		{
			if (m_SelStart==-1 || m_SelStart>=m_SelEnd)
				return false;

			RecurseProcessKey(KEY_CTRLINS);
			DeleteBlock();
			Show();
			return true;
		}
		case KEY_CTRLINS:     case KEY_CTRLNUMPAD0:
		case KEY_RCTRLINS:    case KEY_RCTRLNUMPAD0:
		{
			if (!m_Flags.Check(FEDITLINE_PASSWORDMODE))
			{
				if (m_SelStart==-1 || m_SelStart>=m_SelEnd)
				{
					if (!Mask.empty())
					{
						SetClipboardText(trim_right(string_view(m_Str)));
					}
					else
					{
						SetClipboardText(m_Str);
					}
				}
				else if (m_SelEnd <= m_Str.size()) // TODO: если в начало условия добавить "StrSize &&", то пропадет баг "Ctrl-Ins в пустой строке очищает клипборд"
				{
					SetClipboardText(string_view(m_Str).substr(m_SelStart, m_SelEnd - m_SelStart));
				}
			}

			return true;
		}
		case KEY_SHIFTINS:    case KEY_SHIFTNUMPAD0:
		{
			string ClipText;

			if (!GetClipboardText(ClipText))
				return true;

			const auto MaxLength = GetMaxLength();
			if (MaxLength != -1 && ClipText.size() > static_cast<size_t>(MaxLength))
			{
				ClipText.resize(MaxLength);
			}

			for (size_t i=0; i < ClipText.size(); ++i)
			{
				if (IsEol(ClipText[i]))
				{
					if (i + 1 < ClipText.size() && IsEol(ClipText[i + 1]))
						ClipText.erase(i, 1);

					if (i+1 == ClipText.size())
						ClipText.resize(i);
					else
						ClipText[i] = L' ';
				}
			}

			InsertString(ClipText);
			Show();
			return true;
		}
		case KEY_SHIFTTAB:
		{
			SetPrevCurPos(m_CurPos);
			const auto Pos = GetLineCursorPos();
			SetLineCursorPos(Pos-((Pos-1) % GetTabSize()+1));

			if (GetLineCursorPos()<0) SetLineCursorPos(0); //CursorPos=0,TabSize=1 case

			SetTabCurPos(GetLineCursorPos());
			Show();
			return true;
		}
		case KEY_SHIFTSPACE:
			LocalKey = KEY_SPACE;
			[[fallthrough]];
		default:
		{
//      _D(SysLog(L"Key=0x%08X",LocalKey));
			if (any_of(LocalKey, KEY_NONE, KEY_IDLE, KEY_ENTER, KEY_NUMENTER) || LocalKey >= 65536)
				break;

			if (!m_Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
			{
				if (PrevSelStart!=-1)
				{
					m_SelStart=PrevSelStart;
					m_SelEnd=PrevSelEnd;
				}

				SCOPED_ACTION(auto)(CallbackSuppressor());
				DeleteBlock();
			}

			if (InsertKey(static_cast<wchar_t>(LocalKey)))
			{
				const auto CurWindowType = Global->WindowManager->GetCurrentWindow()->GetType();
				if (CurWindowType == windowtype_dialog || CurWindowType == windowtype_panels)
				{
					Show();
				}
			}
			return true;
		}
	}

	return false;
}

// обработка Ctrl-Q
bool Edit::ProcessCtrlQ()
{
	INPUT_RECORD rec;
	for (;;)
	{
		const auto Key = GetInputRecord(&rec);

		if (none_of(Key, KEY_NONE, KEY_IDLE) && rec.Event.KeyEvent.uChar.UnicodeChar)
			break;

		if (Key==KEY_CONSOLE_BUFFER_RESIZE)
		{
			// BUGBUG currently GetInputRecord will never return anything but KEY_NONE if manager queue isn't empty,
			// and it will be non-empty if we allow async resizing here.
			// It's better to exit from Ctrl-Q mode at all than hang.
			return false;
		}
	}

	return InsertKey(rec.Event.KeyEvent.uChar.UnicodeChar);
}

bool Edit::InsertKey(wchar_t const Key)
{
	bool changed=false;

	if (m_Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
		return true;

	if (Key==KEY_TAB && m_Flags.Check(FEDITLINE_OVERTYPE))
	{
		SetPrevCurPos(m_CurPos);
		const auto Pos = GetLineCursorPos();
		SetLineCursorPos(static_cast<int>(Pos + (GetTabSize() - (Pos % GetTabSize()))));
		SetTabCurPos(GetLineCursorPos());
		return true;
	}

	const auto Mask = GetInputMask();
	if (!Mask.empty())
	{
		const auto MaskLen = static_cast<int>(Mask.size());

		if (m_CurPos<MaskLen)
		{
			if (CharInMask(Key, Mask[m_CurPos]))
			{
				if (!m_Flags.Check(FEDITLINE_OVERTYPE))
				{
					int i=MaskLen-1;

					while (i > m_CurPos && !CheckCharMask(Mask[i]))
						i--;

					for (int j=i; i>m_CurPos; i--)
					{
						if (CheckCharMask(Mask[i]))
						{
							while (!CheckCharMask(Mask[j-1]))
							{
								if (j<=m_CurPos)
									break;

								j--;
							}

							m_Str[i]=m_Str[j-1];
							j--;
						}
					}
				}

				SetPrevCurPos(m_CurPos);
				m_Str[m_CurPos++]=Key;
				changed=true;
			}
			else
			{
				// Здесь вариант для "ввели символ из маски", например для SetAttr - ввели '.'
				// char *Ptr=strchr(Mask+CurPos,Key);
			}
		}
		else if (m_CurPos<m_Str.size())
		{
			SetPrevCurPos(m_CurPos);
			m_Str[m_CurPos++]=Key;
			changed=true;
		}
	}
	else
	{
		if (GetMaxLength() == -1 || m_Str.size() + 1 <= GetMaxLength())
		{
			if (m_CurPos > m_Str.size())
			{
				m_Str.resize(m_CurPos, L' ');
			}

			if (Key == KEY_TAB && any_of(GetTabExpandMode(), EXPAND_NEWTABS, EXPAND_ALLTABS))
			{
				InsertTab();
				return true;
			}

			SetPrevCurPos(m_CurPos);

			if (!m_Flags.Check(FEDITLINE_OVERTYPE) || m_CurPos >= m_Str.size())
			{
				m_Str.insert(m_CurPos, 1, Key);

				if (m_SelStart!=-1)
				{
					if (m_SelEnd!=-1 && m_CurPos<m_SelEnd)
						m_SelEnd++;

					if (m_CurPos<m_SelStart)
						m_SelStart++;
				}
			}
			else
			{
				m_Str[m_CurPos] = Key;
			}

			++m_CurPos;
			changed=true;
		}
		else
		{
			if (m_CurPos < m_Str.size())
			{
				SetPrevCurPos(m_CurPos);
				m_Str[m_CurPos++]=Key;
				changed=true;
			}
		}
	}

	if (changed)
		Changed();

	return true;
}

void Edit::SetHiString(string_view const Str)
{
	if (m_Flags.Check(FEDITLINE_READONLY))
		return;

	const auto NewStr = HiText2Str(Str);
	RemoveSelection();
	SetString(NewStr);
}

void Edit::SetEOL(eol Eol)
{
	m_Eol = Eol;
}

eol Edit::GetEOL() const
{
	return m_Eol;
}

/* $ 25.07.2000 tran
   примечание:
   в этом методе DropDownBox не обрабатывается
   ибо он вызывается только из SetString и из класса Editor
   в Dialog он нигде не вызывается */
void Edit::SetString(string_view Str, bool const KeepSelection)
{
	if (m_Flags.Check(FEDITLINE_READONLY))
		return;

	// коррекция вставляемого размера, если определен GetMaxLength()
	if (GetMaxLength() != -1 && Str.size() > static_cast<size_t>(GetMaxLength()))
	{
		Str = Str.substr(0, GetMaxLength()); // ??
	}

	if (!KeepSelection)
		RemoveSelection();

	if (!m_Flags.Check(FEDITLINE_PARENT_SINGLELINE))
	{
		if (!Str.empty() && Str.back() == L'\r')
		{
			m_Eol = eol::mac;
			Str.remove_suffix(1);
		}
		else
		{
			if (!Str.empty() && Str.back() == L'\n')
			{
				Str.remove_suffix(1);

				if (!Str.empty() && Str.back() == L'\r')
				{
					Str.remove_suffix(1);

					if (!Str.empty() && Str.back() == L'\r')
					{
						Str.remove_suffix(1);
						m_Eol = eol::bad_win;
					}
					else
						m_Eol = eol::win;
				}
				else
					m_Eol = eol::unix;
			}
			else
				m_Eol = eol::none;
		}
	}

	m_CurPos=0;

	const auto Mask = GetInputMask();
	if (!Mask.empty())
	{
		RefreshStrByMask(TRUE);
		for (size_t i = 0, j = 0, maskLen = Mask.size(); i < maskLen && j < maskLen && j < Str.size();)
		{
			if (CheckCharMask(Mask[i]))
			{
				int goLoop=FALSE;

				if (CharInMask(Str[j], Mask[m_CurPos]))
					InsertKey(Str[j]);
				else
					goLoop=TRUE;

				j++;

				if (goLoop) continue;
			}
			else
			{
				SetPrevCurPos(m_CurPos);
				m_CurPos++;
			}

			i++;
		}

		/* Здесь необходимо условие (!*Str), т.к. для очистки строки
		   обычно вводится нечто вроде SetString("",0)
		   Т.е. таким образом мы добиваемся "инициализации" строки с маской
		*/
		RefreshStrByMask(Str.empty());
	}
	else
	{
		m_Str.assign(Str);

		if (GetTabExpandMode() == EXPAND_ALLTABS)
			ReplaceTabs();

		SetPrevCurPos(m_CurPos);
		m_CurPos = m_Str.size();
	}

	Changed();
}

string Edit::GetSelString() const
{
	if (m_SelStart==-1 || (m_SelEnd!=-1 && m_SelEnd<=m_SelStart) || m_SelStart >= m_Str.size())
	{
		return {};
	}

	return { m_Str.cbegin() + m_SelStart, m_SelEnd == -1 ? m_Str.cend() : m_Str.cbegin() + std::min(m_Str.size(), m_SelEnd) };
}

void Edit::AppendString(const string_view Str)
{
	const auto LastPos = m_CurPos;
	m_CurPos = GetLength();
	InsertString(Str);
	m_CurPos = LastPos;
}

void Edit::InsertString(string_view Str)
{
	if (m_Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
		return;

	{
		SCOPED_ACTION(auto)(CallbackSuppressor());
		if (!m_Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
			DeleteBlock();
	}

	if (m_Flags.Check(FEDITLINE_CLEARFLAG))
	{
		LeftPos = 0;
		m_Flags.Clear(FEDITLINE_CLEARFLAG);
		SetString(Str);
		return;
	}

	const auto Mask = GetInputMask();
	if (!Mask.empty())
	{
		const auto MaskLen = Mask.size();

		if (static_cast<size_t>(m_CurPos) < MaskLen)
		{
			const auto StrLen = std::min(MaskLen - m_CurPos, Str.size());

			for (size_t i = m_CurPos, j = 0; i != MaskLen && j != StrLen;)
			{
				if (CheckCharMask(Mask[i]))
				{
					bool goLoop = false;

					if (j < Str.size() && CharInMask(Str[j], Mask[m_CurPos]))
					{
						InsertKey(Str[j]);
					}
					else
						goLoop = true;

					j++;

					if (goLoop)
						continue;
				}
				else
				{
					if(Mask[j] == Str[j])
					{
						j++;
					}
					SetPrevCurPos(m_CurPos);
					m_CurPos++;
				}

				i++;
			}
		}

		RefreshStrByMask();
		//_SVS(SysLog(L"InsertString ==> Str='%s'", Str));
	}
	else
	{
		if (GetMaxLength() != -1 && m_Str.size() + Str.size() > static_cast<size_t>(GetMaxLength()))
		{
			// коррекция вставляемого размера, если определен GetMaxLength()
			if (m_Str.size() < GetMaxLength())
			{
				Str = Str.substr(0, GetMaxLength() - m_Str.size());
			}
		}

		if (GetMaxLength() == -1 || m_Str.size() + Str.size() <= static_cast<size_t>(GetMaxLength()))
		{
			if (m_CurPos > m_Str.size())
			{
				m_Str.resize(m_CurPos, L' ');
			}

			m_Str.insert(m_CurPos, Str);

			SetPrevCurPos(m_CurPos);
			m_CurPos += static_cast<int>(Str.size());

			if (GetTabExpandMode() == EXPAND_ALLTABS)
				ReplaceTabs();

			Changed();
		}
	}
}

int Edit::GetLength() const
{
	return m_Str.size();
}

bool Edit::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (!(MouseEvent->dwButtonState & 3))
		return false;

	if (!m_Where.contains(MouseEvent->dwMousePosition))
		return false;

	//SetClearFlag(0); // пусть едитор сам заботится о снятии clear-текста?
	SetTabCurPos(MouseEvent->dwMousePosition.X - m_Where.left + LeftPos);

	if (!m_Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
		RemoveSelection();

	if (MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED)
	{
		static std::chrono::steady_clock::time_point PrevDoubleClick;
		static point PrevPosition;

		const auto CurrentTime = std::chrono::steady_clock::now();

		if (
			CurrentTime - PrevDoubleClick <= std::chrono::milliseconds(GetDoubleClickTime()) &&
			MouseEvent->dwEventFlags != MOUSE_MOVED &&
			PrevPosition == MouseEvent->dwMousePosition
		)
		{
			Select(0, m_Str.size());
			PrevDoubleClick = {};
			PrevPosition = {};
		}

		if (MouseEvent->dwEventFlags==DOUBLE_CLICK)
		{
			ProcessKey(Manager::Key(KEY_OP_SELWORD));
			PrevDoubleClick = CurrentTime;
			PrevPosition=MouseEvent->dwMousePosition;
		}
		else
		{
			PrevDoubleClick = {};
			PrevPosition = {};
		}
	}

	Show();
	return true;
}

void Edit::InsertTab()
{
	if (m_Flags.Check(FEDITLINE_READONLY))
		return;

	const auto Pos = m_CurPos;
	const auto S = static_cast<int>(GetTabSize() - (RealPosToTab(Pos) % GetTabSize()));

	if (m_SelStart!=-1)
	{
		if (Pos<=m_SelStart)
		{
			m_SelStart+=S-(Pos==m_SelStart?0:1);
		}

		if (m_SelEnd!=-1 && Pos<m_SelEnd)
		{
			m_SelEnd+=S;
		}
	}

	m_Str.insert(Pos, S, L' ');
	m_CurPos += S;
	Changed();
}

bool Edit::ReplaceTabs()
{
	if (m_Flags.Check(FEDITLINE_READONLY))
		return false;

	int Pos = 0;
	bool changed = false;

	auto TabPtr = m_Str.end();
	while ((TabPtr = std::find(m_Str.begin() + Pos, m_Str.end(), L'\t')) != m_Str.end())
	{
		changed=true;
		Pos = static_cast<int>(TabPtr - m_Str.begin());
		const auto S = static_cast<int>(GetTabSize() - (Pos % GetTabSize()));

		if (m_SelStart!=-1)
		{
			if (Pos<=m_SelStart)
			{
				m_SelStart+=S-(Pos==m_SelStart?0:1);
			}

			if (m_SelEnd!=-1 && Pos<m_SelEnd)
			{
				m_SelEnd+=S-1;
			}
		}

		*TabPtr = L' ';
		m_Str.insert(TabPtr, S - 1, L' ');

		if (m_CurPos>Pos)
			m_CurPos+=S-1;
	}

	if (changed) Changed();
	return changed;
}

int Edit::GetTabCurPos() const
{
	return RealPosToTab(m_CurPos);
}

void Edit::SetTabCurPos(int NewPos)
{
	const auto Mask = GetInputMask();
	if (!Mask.empty())
	{
		const auto Pos = static_cast<int>(trim_right(string_view(m_Str)).size());

		if (NewPos>Pos)
			NewPos=Pos;
	}

	m_CurPos=TabPosToReal(NewPos);
}

int Edit::RealPosToTab(int Pos) const
{
	return RealPosToTab(0, 0, Pos);
}

static size_t real_pos_to_tab(string_view const Str, size_t const TabSize, size_t Pos, size_t const PrevLength, size_t const PrevPos, int* CorrectPos)
{
	auto TabPos = PrevLength;

	// Начинаем вычисление с предыдущей позиции
	auto Index = PrevPos;
	const auto Size = Str.size();

	// Корректировка табов
	bool bCorrectPos = CorrectPos && *CorrectPos;

	// Проходим по всем символам до позиции поиска, если она ещё в пределах строки,
	// либо до конца строки, если позиция поиска за пределами строки
	for (; Index < std::min(Pos, Size); Index++)
	{
		if (Str[Index] != L'\t')
		{
			++TabPos;
			continue;
		}

		// Если есть необходимость делать корректировку табов и эта корректировка
		// ещё не проводилась, то увеличиваем длину обрабатываемой строки на единицу
		if (bCorrectPos)
		{
			++Pos;
			*CorrectPos = 1;
			bCorrectPos = false;
		}

		// Рассчитываем длину таба с учётом настроек и текущей позиции в строке
		TabPos += TabSize - TabPos % TabSize;
	}

	// Если позиция находится за пределами строки, то там точно нет табов и всё просто
	if (Pos >= Size)
		TabPos += Pos - Index;

	return TabPos;
}

int Edit::RealPosToTab(int PrevLength, int PrevPos, int Pos, int* CorrectPos) const
{
	if (CorrectPos)
		*CorrectPos = 0;

	if (GetTabExpandMode() == EXPAND_ALLTABS || PrevPos >= m_Str.size())
		return PrevLength + Pos - PrevPos;

	return static_cast<int>(real_pos_to_tab(m_Str, GetTabSize(), Pos, PrevLength, PrevPos, CorrectPos));
}

static size_t tab_pos_to_real(string_view const Str, size_t const TabSize, size_t const Pos)
{
	size_t Index{};
	const auto Size = Str.size();

	for (size_t TabPos{}; TabPos < Pos; ++Index)
	{
		if (Index == Size)
		{
			Index += Pos - TabPos;
			break;
		}

		if (Str[Index] != L'\t')
		{
			++TabPos;
			continue;
		}

		const auto NewTabPos = TabPos + TabSize - TabPos % TabSize;

		if (NewTabPos > Pos)
			break;

		TabPos = NewTabPos;
	}

	return Index;
}

int Edit::TabPosToReal(int Pos) const
{
	if (GetTabExpandMode() == EXPAND_ALLTABS)
		return Pos;

	return static_cast<int>(tab_pos_to_real(m_Str, GetTabSize(), Pos));
}

void Edit::Select(int Start,int End)
{
	m_SelStart=Start;
	m_SelEnd=End;

	if (m_SelEnd<m_SelStart && m_SelEnd!=-1)
	{
		m_SelStart=-1;
		m_SelEnd=0;
	}

	if (m_SelStart==-1 && m_SelEnd==-1)
	{
		m_SelEnd=0;
	}
}

void Edit::RemoveSelection()
{
	Select(-1, 0);
	m_Flags.Clear(FEDITLINE_MARKINGBLOCK);
}

void Edit::AddSelect(int Start,int End)
{
	if (Start<m_SelStart || m_SelStart==-1)
		m_SelStart=Start;

	if (End==-1 || (End>m_SelEnd && m_SelEnd!=-1))
		m_SelEnd=End;

	if (m_SelEnd>m_Str.size())
		m_SelEnd = m_Str.size();

	if (m_SelEnd<m_SelStart && m_SelEnd!=-1)
	{
		m_SelStart=-1;
		m_SelEnd=0;
	}
}

void Edit::GetSelection(intptr_t &Start,intptr_t &End) const
{
	Start = m_SelStart;
	End = m_SelEnd;

	if (End > m_Str.size())
		End = -1;

	if (Start > m_Str.size())
		Start = m_Str.size();
}

void Edit::AdjustMarkBlock()
{
	bool mark = false;
	if (m_SelStart >= 0)
	{
		const auto end = m_SelEnd > m_Str.size() || m_SelEnd == -1? m_Str.size() : m_SelEnd;
		mark = end > m_SelStart && (m_CurPos==m_SelStart || m_CurPos==end);
	}
	m_Flags.Change(FEDITLINE_MARKINGBLOCK, mark);
}

void Edit::AdjustPersistentMark()
{
	if (m_SelStart < 0 || m_Flags.Check(FEDITLINE_MARKINGBLOCK | FEDITLINE_PARENT_EDITOR))
		return;

	bool persistent;
	if (m_Flags.Check(FEDITLINE_PARENT_SINGLELINE))
		persistent = m_Flags.Check(FEDITLINE_PERSISTENTBLOCKS); // dlgedit
	else if (!m_Flags.Check(FEDITLINE_PARENT_MULTILINE))
		persistent = Global->Opt->CmdLine.EditBlock;				// cmdline
	else
		persistent = false;

	if (!persistent)
		return;

	const auto end = m_SelEnd > m_Str.size() || m_SelEnd == -1? m_Str.size() : m_SelEnd;
	if (end > m_SelStart && (m_CurPos==m_SelStart || m_CurPos==end))
		m_Flags.Set(FEDITLINE_MARKINGBLOCK);
}

void Edit::GetRealSelection(intptr_t &Start,intptr_t &End) const
{
	Start=m_SelStart;
	End=m_SelEnd;
}

void Edit::DeleteBlock()
{
	if (m_Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
		return;

	if (m_SelStart==-1 || m_SelStart>=m_SelEnd)
		return;

	SetPrevCurPos(m_CurPos);

	const auto Mask = GetInputMask();
	if (!Mask.empty())
	{
		for (auto i = m_SelStart; i != m_SelEnd; ++i)
		{
			if (CheckCharMask(Mask[i]))
			{
				m_Str[i] = L' ';
			}
		}
		m_CurPos=m_SelStart;
	}
	else
	{
		const auto From = std::min(m_SelStart, m_Str.size());
		const auto To = std::min(m_SelEnd, m_Str.size());

		m_Str.erase(From, To - From);

		if (m_CurPos>From)
		{
			if (m_CurPos<To)
				m_CurPos=From;
			else
				m_CurPos-=To-From;
		}
	}

	m_SelStart=-1;
	m_SelEnd=0;
	m_Flags.Clear(FEDITLINE_MARKINGBLOCK);

	// OT: Проверка на корректность поведения строки при удалении и вставке
	if (m_Flags.Check((FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE)))
	{
		LeftPos = std::min(LeftPos, m_CurPos);
	}

	Changed(true);
}

void Edit::AddColor(const ColorItem& col)
{
	ColorList.insert(col);
}

void Edit::DeleteColor(delete_color_condition const Condition)
{
	std::erase_if(ColorList, Condition);
}

bool Edit::GetColor(ColorItem& col, size_t Item) const
{
	if (Item >= ColorList.size())
		return false;

	auto it = ColorList.begin();
	std::advance(it, Item);
	col = *it;
	return true;
}

void Edit::ApplyColor(const FarColor& SelColor, int XPos, int FocusedLeftPos)
{
	// Для оптимизации сохраняем вычисленные позиции между итерациями цикла
	int Pos = INT_MIN, TabPos = INT_MIN, TabEditorPos = INT_MIN;

	// Обрабатываем элементы раскраски
	for (const auto& CurItem: ColorList)
	{
		// Пропускаем элементы у которых начало больше конца
		if (CurItem.StartPos > CurItem.EndPos)
			continue;

		const auto Width = ObjWidth();

		// Получаем начальную позицию
		int RealStart, Start;

		// Если предыдущая позиция равна текущей, то ничего не вычисляем
		// и сразу берём ранее вычисленное значение
		if (Pos == CurItem.StartPos)
		{
			RealStart = TabPos;
			Start = TabEditorPos;
		}
		// Если вычисление идёт первый раз или предыдущая позиция больше текущей,
		// то производим вычисление с начала строки
		else if (Pos == INT_MIN || CurItem.StartPos < Pos)
		{
			RealStart = RealPosToTab(CurItem.StartPos);
			Start = RealStart-FocusedLeftPos;
		}
		// Для оптимизации делаем вычисление относительно предыдущей позиции
		else
		{
			RealStart = RealPosToTab(TabPos, Pos, CurItem.StartPos);
			Start = RealStart-FocusedLeftPos;
		}

		// Запоминаем вычисленные значения для их дальнейшего повторного использования
		Pos = CurItem.StartPos;
		TabPos = RealStart;
		TabEditorPos = Start;

		// Пропускаем элементы раскраски у которых начальная позиция за экраном
		if (Start >= Width)
			continue;

		// Корректировка относительно табов (отключается, если присутствует флаг ECF_TABMARKFIRST)
		int CorrectPos = CurItem.Flags & ECF_TABMARKFIRST ? 0 : 1;

		// Получаем конечную позицию
		int EndPos = CurItem.EndPos;
		int RealEnd, End;

		bool TabMarkCurrent=false;

		// Обрабатываем случай, когда предыдущая позиция равна текущей, то есть
		// длина раскрашиваемой строки равна 1
		if (Pos == EndPos)
		{
			// Если необходимо делать корректировку относительно табов и единственный
			// символ строки -- это таб, то делаем расчёт с учётом корректировки,
			// иначе ничего не вычисляем и берём старые значения
			if (CorrectPos && EndPos < m_Str.size() && m_Str[EndPos] == L'\t')
			{
				RealEnd = RealPosToTab(TabPos, Pos, ++EndPos);
				End = RealEnd-FocusedLeftPos;
				TabMarkCurrent = (CurItem.Flags & ECF_TABMARKCURRENT) && XPos>=Start && XPos<End;
			}
			else
			{
				RealEnd = TabPos;
				CorrectPos = 0;
				End = TabEditorPos;
			}
		}
		// Если предыдущая позиция больше текущей, то производим вычисление
		// с начала строки (с учётом корректировки относительно табов)
		else if (EndPos < Pos)
		{
			// TODO: возможно так же нужна коррекция с учетом табов (на предмет Mantis#0001718)
			RealEnd = RealPosToTab(0, 0, EndPos, &CorrectPos);
			EndPos += CorrectPos;
			End = RealEnd-FocusedLeftPos;
		}
		// Для оптимизации делаем вычисление относительно предыдущей позиции (с учётом
		// корректировки относительно табов)
		else
		{
			// Mantis#0001718: Отсутствие ECF_TABMARKFIRST не всегда корректно отрабатывает
			// Коррекция с учетом последнего таба
			if (CorrectPos && EndPos < m_Str.size() && m_Str[EndPos] == L'\t')
				RealEnd = RealPosToTab(TabPos, Pos, ++EndPos);
			else
			{
				RealEnd = RealPosToTab(TabPos, Pos, EndPos, &CorrectPos);
				EndPos += CorrectPos;
			}
			End = RealEnd-FocusedLeftPos;
		}

		// Запоминаем вычисленные значения для их дальнейшего повторного использования
		Pos = EndPos;
		TabPos = RealEnd;
		TabEditorPos = End;

		if(TabMarkCurrent)
		{
			Start = XPos;
			End = XPos;
		}
		else
		{
			// Пропускаем элементы раскраски у которых конечная позиция меньше левой границы экрана
			if (End < 0)
				continue;

			// Обрезаем раскраску элемента по экрану
			if (Start < 0)
				Start = 0;

			if (End >= Width)
				End = Width-1;
			else
				End -= CorrectPos;
		}

		// Раскрашиваем элемент, если есть что раскрашивать
		if (End >= Start)
		{
			Global->ScrBuf->ApplyColor(
			    { m_Where.left + Start, m_Where.top, m_Where.left + End, m_Where.top },
			    CurItem.GetColor(),
			    // Не раскрашиваем выделение
			    SelColor,
			    true
			);
		}
	}
}

/* $ 24.09.2000 SVS $
  Функция Xlat - перекодировка по принципу QWERTY <-> ЙЦУКЕН
*/
void Edit::Xlat(bool All)
{
	int StartPos, EndPos;

	//   Для CmdLine - если нет выделения, преобразуем всю строку
	if (All && m_SelStart == -1 && !m_SelEnd)
	{
		StartPos = 0;
		EndPos = m_Str.size();
	}
	else if (m_SelStart != -1 && m_SelStart != m_SelEnd)
	{
		if (m_SelEnd == -1)
			m_SelEnd = m_Str.size();

		StartPos = m_SelStart;
		EndPos = m_SelEnd;
	}
	/* $ 25.11.2000 IS
	 Если нет выделения, то обработаем текущее слово. Слово определяется на
	 основе специальной группы разделителей.
	*/
	else
	{
		/* $ 10.12.2000 IS
		   Обрабатываем только то слово, на котором стоит курсор, или то слово, что
		   находится левее позиции курсора на 1 символ
		*/
		StartPos = m_CurPos;
		const auto StrSize = m_Str.size();

		const string& WordDivForXlat = Global->Opt->XLat.strWordDivForXlat;
		if (IsWordDiv(WordDivForXlat, m_Str[StartPos]))
		{
			if (StartPos)
				--StartPos;

			if (IsWordDiv(WordDivForXlat, m_Str[StartPos]))
				return;
		}

		while (StartPos >= 0 && !IsWordDiv(WordDivForXlat, m_Str[StartPos]))
			StartPos--;

		++StartPos;
		EndPos = StartPos + 1;

		while (EndPos < StrSize && !IsWordDiv(WordDivForXlat, m_Str[EndPos]))
			++EndPos;
	}

	::Xlat({ m_Str.data() + StartPos, m_Str.data() + EndPos }, Global->Opt->XLat.Flags);
	Changed();
	Show();
}

bool Edit::CharInMask(wchar_t const Char, wchar_t const Mask)
{
	return
		(Mask == EDMASK_ANY) ||
		(Mask == EDMASK_DSS && (std::iswdigit(Char) || Char == L' ' || Char == L'-')) ||
		(Mask == EDMASK_DIGITS && (std::iswdigit(Char) || Char == L' ')) ||
		(Mask == EDMASK_DIGIT && (std::iswdigit(Char))) ||
		(Mask == EDMASK_ALPHA && is_alpha(Char)) ||
		(Mask == EDMASK_HEX && std::iswxdigit(Char));
}

int Edit::CheckCharMask(wchar_t Chr)
{
	return Chr==EDMASK_ANY || Chr==EDMASK_DIGIT || Chr==EDMASK_DIGITS || Chr==EDMASK_DSS || Chr==EDMASK_ALPHA || Chr==EDMASK_HEX;
}

void Edit::SetDialogParent(DWORD Sets)
{
	if ((Sets&(FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE)) == (FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE) ||
	        !(Sets&(FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE)))
		m_Flags.Clear(FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE);
	else if (Sets&FEDITLINE_PARENT_SINGLELINE)
	{
		m_Flags.Clear(FEDITLINE_PARENT_MULTILINE);
		m_Flags.Set(FEDITLINE_PARENT_SINGLELINE);
	}
	else if (Sets&FEDITLINE_PARENT_MULTILINE)
	{
		m_Flags.Clear(FEDITLINE_PARENT_SINGLELINE);
		m_Flags.Set(FEDITLINE_PARENT_MULTILINE);
	}
}

void Edit::FixLeftPos(int TabCurPos)
{
	if (TabCurPos<0) TabCurPos=GetTabCurPos(); //оптимизация, чтобы два раза не дёргать
	if (TabCurPos-LeftPos>ObjWidth()-1)
		LeftPos=TabCurPos-ObjWidth()+1;

	if (TabCurPos<LeftPos)
		LeftPos=TabCurPos;
}

const FarColor& Edit::GetNormalColor() const
{
	return GetEditor()->GetNormalColor();
}

const FarColor& Edit::GetSelectedColor() const
{
	return GetEditor()->GetSelectedColor();
}

const FarColor& Edit::GetUnchangedColor() const
{
	return GetNormalColor();
}

size_t Edit::GetTabSize() const
{
	return GetEditor()->GetTabSize();
}

EXPAND_TABS Edit::GetTabExpandMode() const
{
	return GetEditor()->GetConvertTabs();
}

const string& Edit::WordDiv() const
{
	return GetEditor()->GetWordDiv();
}

int Edit::GetCursorSize() const
{
	return -1;
}

int Edit::GetMacroSelectionStart() const
{
	return GetEditor()->GetMacroSelectionStart();
}

void Edit::SetMacroSelectionStart(int Value)
{
	GetEditor()->SetMacroSelectionStart(Value);
}

int Edit::GetLineCursorPos() const
{
	return GetEditor()->GetLineCursorPos();
}

void Edit::SetLineCursorPos(int Value)
{
	return GetEditor()->SetLineCursorPos(Value);
}

Editor* Edit::GetEditor() const
{
	if (const auto owner = dynamic_cast<EditorContainer*>(GetOwner().get()))
	{
		return owner->GetEditor();
	}
	return nullptr;
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("tab_pos")
{
	static string_view const Strs[]
	{
		{},
		L"1\t2"sv,
		L"\t1\t12\t123\t1234\t"sv,
	};

	static const struct
	{
		size_t Str, TabSize, TabPos, RealPos;
		bool TestRealToTab;
	}
	Tests[]
	{
		{ 0, 0,  0,  0, true,  },
		{ 0, 0,  1,  1, true,  },

		{ 1, 0,  0,  0, true,  },

		{ 1, 1,  0,  0, true,  },
		{ 1, 1,  1,  1, true,  },
		{ 1, 1,  2,  2, true,  },
		{ 1, 1,  3,  3, true,  },

		{ 1, 2,  0,  0, true,  },
		{ 1, 2,  1,  1, true,  },
		{ 1, 2,  2,  2, true,  },
		{ 1, 2,  3,  3, true,  },

		{ 1, 3,  0,  0, true,  },
		{ 1, 3,  1,  1, true,  },
		{ 1, 3,  2,  1, false, },
		{ 1, 3,  3,  2, true,  },

		{ 2, 4,  0,  0, true,  },
		{ 2, 4,  1,  0, false, },
		{ 2, 4,  2,  0, false, },
		{ 2, 4,  3,  0, false, },
		{ 2, 4,  4,  1, true,  },
		{ 2, 4,  5,  2, true,  },
		{ 2, 4,  6,  2, false, },
		{ 2, 4,  7,  2, false, },
		{ 2, 4,  8,  3, true,  },
		{ 2, 4,  9,  4, true,  },
		{ 2, 4, 10,  5, true,  },
		{ 2, 4, 11,  5, false, },
		{ 2, 4, 12,  6, true,  },
		{ 2, 4, 13,  7, true,  },
		{ 2, 4, 14,  8, true,  },
		{ 2, 4, 15,  9, true,  },
		{ 2, 4, 16, 10, true,  },
		{ 2, 4, 17, 11, true,  },
		{ 2, 4, 18, 12, true,  },
		{ 2, 4, 19, 13, true,  },
		{ 2, 4, 20, 14, true,  },
		{ 2, 4, 21, 14, false, },
		{ 2, 4, 22, 14, false, },
		{ 2, 4, 23, 14, false, },
		{ 2, 4, 24, 15, false, },
		{ 2, 4, 25, 16, true,  },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.RealPos == tab_pos_to_real(Strs[i.Str], i.TabSize, i.TabPos));

		if (i.TestRealToTab)
			REQUIRE(i.TabPos == real_pos_to_tab(Strs[i.Str], i.TabSize, i.RealPos, 0, 0, {}));
	}
}
#endif
