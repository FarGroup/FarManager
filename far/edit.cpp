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

class positions_cache
{
public:
	NONCOPYABLE(positions_cache);

	using accessor_type = function_ref<int(int, position_parser_state*)>;

	explicit positions_cache(accessor_type const Accessor):
		m_Accessor(Accessor)
	{
	}

	int get(int const Position)
	{
		if (static_cast<size_t>(Position) < std::size(m_SmallPositions))
		{
			auto& Value = m_SmallPositions[Position].second;
			if (!Value)
				Value = m_Accessor(Position, &m_State);

			return *Value;
		}

		const auto [Iterator, IsNew] = m_BigPositions.emplace(Position, 0);
		if (IsNew)
			Iterator->second = m_Accessor(Position, &m_State);

		return Iterator->second;
	}

private:
	std::pair<int, std::optional<int>> m_SmallPositions[512];
	std::unordered_map<int, int> m_BigPositions;
	position_parser_state m_State;
	accessor_type m_Accessor;
};

void ColorItem::SetOwner(const UUID& Value)
{
	static std::unordered_set<UUID> UuidSet;
	Owner = std::to_address(UuidSet.emplace(Value).first);
}

void ColorItem::SetColor(const FarColor& Value)
{
	Color = colors::StoreColor(Value);
}

static int Recurse=0;

static const wchar_t EDMASK_ANY    = L'X'; // позволяет вводить в строку ввода любой символ;
static const wchar_t EDMASK_DSS    = L'#'; // позволяет вводить в строку ввода цифры, пробел и знак минуса;
static const wchar_t EDMASK_DIGIT  = L'9'; // позволяет вводить в строку ввода только цифры;
static const wchar_t EDMASK_ALPHA  = L'A'; // позволяет вводить в строку ввода только буквы.
static const wchar_t EDMASK_HEX    = L'H'; // позволяет вводить в строку ввода шестнадцатиричные символы.

// Unofficial
static const wchar_t EDMASK_DIGITS = L'N';  // позволяет вводить в строку ввода только цифры и пробелы;
static const wchar_t EDMASK_BIN    = L'\1'; // 0 and 1 only

Edit::Edit(window_ptr Owner):
	SimpleScreenObject(std::move(Owner))
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
	//   Вычисление нового положения курсора в строке с учётом Mask.
	const auto Value = GetPrevCurPos() > m_CurPos? -1 : 1;
	m_CurPos=GetNextCursorPos(m_CurPos,Value);
	FastShow();

	/* $ 26.07.2000 tran
	   при DropDownBox курсор выключаем
	   не знаю даже - попробовал но не очень красиво вышло */
	if (m_Flags.Check(FEDITLINE_DROPDOWNBOX))
		HideCursor();
	else
	{
		::SetCursorType(true, [this]
		{
			if (const auto Size = GetCursorSize(); Size != -1)
				return Size;

			const auto FullScreenShift = IsConsoleFullscreen()? 1 : 0;
			const auto OvertypeShift = m_Flags.Check(FEDITLINE_OVERTYPE)? 2 : 0;
			return static_cast<int>(Global->Opt->CursorSize[FullScreenShift + OvertypeShift]);
		}());
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
			const auto It = std::ranges::find_if(Mask, CheckCharMask);
			if (It != Mask.cend())
			{
				Result = It - Mask.cbegin();
			}
		}
	}

	return Result;
}

void Edit::FastShow(const ShowInfo* Info)
{
	const auto RealToVisualAccessor = [this](int const Pos, position_parser_state* const State){ return RealPosToVisual(Pos, State); };
	const auto VisualToRealAccessor = [this](int const Pos, position_parser_state* const State){ return VisualPosToReal(Pos, State); };

	positions_cache
		RealToVisual(RealToVisualAccessor),
		VisualToReal(VisualToRealAccessor);

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

	const auto TabCurPos = RealToVisual.get(m_CurPos);

	/* $ 31.07.2001 KM
	  ! Для комбобокса сделаем отображение строки
	    с первой позиции.
	*/
	if (!m_Flags.CheckAny(FEDITLINE_DROPDOWNBOX | FEDITLINE_EDITORMODE))
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
	int TabSelStart = m_SelStart == -1? -1 : RealToVisual.get(m_SelStart);
	int TabSelEnd = m_SelEnd < 0? -1 : RealToVisual.get(m_SelEnd);

	/* $ 17.08.2000 KM
	   Если есть маска, сделаем подготовку строки, то есть
	   все "постоянные" символы в маске, не являющиеся шаблонными
	   должны постоянно присутствовать в Str
	*/
	const auto Mask = GetInputMask();
	if (!Mask.empty())
		RefreshStrByMask();

	SetLineCursorPos(TabCurPos);

	auto RealLeftPos = VisualToReal.get(LeftPos);
	const auto ReconstructedVisualLeftPos = RealToVisual.get(RealLeftPos);

	const size_t EditLength = ObjWidth();

	string OutStr;
	OutStr.reserve(EditLength);

	if (ReconstructedVisualLeftPos < LeftPos)
	{
		auto NextReal = RealLeftPos;
		int NextVisual;

		for (;;)
		{
			NextVisual = RealToVisual.get(++NextReal);
			if (NextVisual > LeftPos)
				break;
		}

		OutStr.insert(0, NextVisual - LeftPos, L' ');
		RealLeftPos = NextReal;
	}

	{
		auto TrailingSpaces = m_Str.cend();
		if (m_Flags.Check(FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE) && Mask.empty() && !m_Str.empty())
		{
			TrailingSpaces = std::find_if_not(m_Str.crbegin(), m_Str.crend(), [](wchar_t i) { return std::iswblank(i);}).base();
		}

		const auto Begin = m_Str.cbegin() + std::min(static_cast<size_t>(RealLeftPos), m_Str.usize());
		for(auto i = Begin, End = m_Str.cend(); i != End && OutStr.size() < EditLength; ++i)
		{
			if (*i == L' ')
			{
				if ((m_Flags.Check(FEDITLINE_SHOWWHITESPACE) && m_Flags.Check(FEDITLINE_EDITORMODE)) || i >= TrailingSpaces)
					OutStr.push_back(L'·');
				else
					OutStr.push_back(*i);
			}
			else if (*i == L'\t')
			{
				const auto TabStart = RealToVisual.get(i - m_Str.begin());
				const auto TabEnd = RealToVisual.get(i + 1 - m_Str.begin());

				OutStr.push_back(((m_Flags.Check(FEDITLINE_SHOWWHITESPACE) && m_Flags.Check(FEDITLINE_EDITORMODE)) || i >= TrailingSpaces)? L'→' : L' ');
				OutStr.resize(OutStr.size() + TabEnd - TabStart - 1, L' ');
			}
			else
			{
				OutStr.push_back(*i);
			}
		}

		if (m_Flags.Check(FEDITLINE_PASSWORDMODE))
			OutStr.assign(OutStr.size(), L'*');

		if (m_Flags.Check(FEDITLINE_SHOWLINEBREAK) && m_Flags.Check(FEDITLINE_EDITORMODE) && (m_Str.size() >= RealLeftPos) && (OutStr.size() < EditLength))
		{
			const auto Cr = L'♪', Lf = L'◙';

			switch (get_eol())
			{
			case eol::eol_type::none:
				break;

			case eol::eol_type::mac:
				OutStr.push_back(Cr);
				break;

			case eol::eol_type::unix:
				OutStr.push_back(Lf);
				break;

			case eol::eol_type::win:
				OutStr.push_back(Cr);
				if(OutStr.size() < EditLength)
					OutStr.push_back(Lf);
				break;

			case eol::eol_type::bad_win:
				OutStr.push_back(Cr);
				if(OutStr.size() < EditLength)
				{
					OutStr.push_back(Cr);
					if(OutStr.size() < EditLength)
						OutStr.push_back(Lf);
				}
				break;

			default:
				std::unreachable();
			}
		}

		if (m_Flags.Check(FEDITLINE_SHOWWHITESPACE) && m_Flags.Check(FEDITLINE_EDITORMODE) && (m_Str.size() >= RealLeftPos) && (OutStr.size() < EditLength) && GetEditor()->IsLastLine(this))
		{
			OutStr.push_back(L'□');
		}
	}

	// Basic color
	SetColor(GetNormalColor());

	const auto VisualTextLength = Text(OutStr, EditLength);

	if (VisualTextLength < EditLength)
		Text(string(EditLength - VisualTextLength, L' '), EditLength);

	if (m_Flags.Check(FEDITLINE_DROPDOWNBOX))
	{
		// Combobox has neither highlight nor selection
		Global->ScrBuf->ApplyColor(m_Where, m_Flags.Check(FEDITLINE_CLEARFLAG)? GetUnchangedColor() : GetSelectedColor());
	}
	else
	{
		if (m_Flags.Check(FEDITLINE_EDITORMODE))
		{
			// Editor highlight
			if (const auto Colors = GetEditor()->GetColors(this); Colors)
				ApplyColor(*Colors, XPos, FocusedLeftPos, RealToVisual);
		}

		if (TabSelStart == -1)
		{
			if (m_Flags.Check(FEDITLINE_CLEARFLAG))
			{
				// BUGBUG multiline
				auto UnchangedArea = m_Where;
				if (VisualTextLength < static_cast<size_t>(UnchangedArea.width()))
					UnchangedArea.right = static_cast<short>(UnchangedArea.left + VisualTextLength - 1);

				Global->ScrBuf->ApplyColor(UnchangedArea, GetUnchangedColor());
			}
		}
		else
		{
			// Selection, on top of everything else
			TabSelStart = std::max(TabSelStart - LeftPos, 0);

			TabSelEnd = TabSelEnd == -1 ?
				static_cast<int>(EditLength) :
				std::max(TabSelEnd - LeftPos, 0);

			Global->ScrBuf->ApplyColor(
				{
					std::min(m_Where.left + TabSelStart, m_Where.right + 1),
					m_Where.top,
					std::min(m_Where.left + TabSelEnd - 1, m_Where.right + 1),
					m_Where.top
				},
				GetSelectedColor()
			);
		}
	}
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
		if (!Shortcuts::Get(Key - KEY_RCTRL0, Data))
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
			return m_CurPos+1;
		case MCODE_F_EDITOR_SEL:
		{
			const auto Action = static_cast<int>(std::bit_cast<intptr_t>(vParam));
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

static void flatten_string(string& Str)
{
	for (auto i = Str.begin(); i != Str.end();)
	{
		if (!IsEol(*i))
		{
			++i;
			continue;
		}

		const auto NotEol = std::find_if_not(i + 1, Str.end(), IsEol);

		if (i == Str.begin() || i + 1 == Str.end() || NotEol == Str.end())
		{
			i = Str.erase(i, NotEol);
			continue;
		}

		*i = L' ';
		i = Str.erase(i + 1, NotEol);

		++i;
	}
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
		!Recurse &&
		is_clear_selection_key(LocalKey)
	)
	{
		m_Flags.Clear(FEDITLINE_MARKINGBLOCK);

		if (!m_Flags.CheckAny(FEDITLINE_PERSISTENTBLOCKS | FEDITLINE_EDITORMODE))
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
		!m_Flags.Check(FEDITLINE_READONLY | FEDITLINE_DROPDOWNBOX) &&
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

				const auto SavedCurPos = m_CurPos;
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
					int EndPos = SavedCurPos;
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

			const auto SavedCurPos = m_CurPos;
			RecurseProcessKey(KEY_RIGHT);

			if (!m_Flags.Check(FEDITLINE_MARKINGBLOCK))
			{
				RemoveSelection();
				m_Flags.Set(FEDITLINE_MARKINGBLOCK);
			}

			if ((m_SelStart != -1 && m_SelEnd == -1) || (SavedCurPos != m_CurPos && m_SelEnd > SavedCurPos))
			{
				if (m_CurPos == m_SelEnd)
					RemoveSelection();
				else
					Select(m_CurPos, m_SelEnd);
			}
			else
				AddSelect(SavedCurPos, m_CurPos == SavedCurPos && m_CurPos < m_Str.size()? m_Str.size() : m_CurPos);

			Show();

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
				const auto Decrement = m_CurPos > 1 && m_Str.size() >= m_CurPos && is_valid_surrogate_pair(string_view(m_Str).substr(m_CurPos - 2))? 2 : 1;
				m_CurPos -= Decrement;
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
					repeat(ptr - m_CurPos, [&]{ RecurseProcessKey(KEY_DEL); });
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

				if (m_CurPos && is_valid_surrogate_pair_at(m_CurPos - 1))
					--m_CurPos;

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

			if (m_CurPos && is_valid_surrogate_pair_at(m_CurPos - 1))
				++m_CurPos;

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
				if (Mask[m_CurPos] == EDMASK_BIN)
				{
					m_Str[m_CurPos] = L'0';
				}
				else
				{
					const auto MaskLen = Mask.size();
					size_t j = m_CurPos;
					for (const auto i: std::views::iota(static_cast<size_t>(m_CurPos), MaskLen))
					{
						if (i + 1 < MaskLen && CheckCharMask(Mask[i + 1]))
						{
							while (j < MaskLen && !CheckCharMask(Mask[j]))
								j++;

							if (!CharInMask(m_Str[i + 1], Mask[j]))
								break;

							m_Str[j] = m_Str[i + 1];
							j++;
						}
					}

					m_Str[j] = L' ';
				}
			}
			else
			{
				m_Str.erase(m_CurPos, is_valid_surrogate_pair(string_view(m_Str).substr(m_CurPos))? 2 : 1);
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

			flatten_string(ClipText);
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
			if (any_of(LocalKey, KEY_NONE, KEY_ENTER, KEY_NUMENTER) || LocalKey >= 65536)
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
	for (;;)
	{
		INPUT_RECORD rec;
		const auto Key = GetInputRecord(&rec);
		if (Key == KEY_NONE)
			continue;

		if (rec.Event.KeyEvent.uChar.UnicodeChar)
			return InsertKey(rec.Event.KeyEvent.uChar.UnicodeChar);
	}
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
	set_eol(Eol.type());
}

eol Edit::GetEOL() const
{
	return get_eol();
}

void Edit::ProcessMask(string_view const Str, string_view const Mask, size_t const From)
{
	for (size_t i = From, j = 0, MaskLen = Mask.size(); i < MaskLen && j < MaskLen && j < Str.size();)
	{
		// After 5050 InsertKey below redraws the dialog.
		// This might affect m_CurPos in mysterious ways.
		m_CurPos = static_cast<int>(i);

		if (CheckCharMask(Mask[i]))
		{
			bool goLoop = false;

			if (j < Str.size() && CharInMask(Str[j], Mask[i]))
				InsertKey(Str[j]);
			else
				goLoop = true;

			j++;

			if (goLoop)
				continue;
		}
		else
		{
			if (Mask[j] == Str[j])
			{
				j++;
			}
		}

		i++;
	}
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

	string StrCopy;
	if (within(m_Str, Str))
	{
		StrCopy = Str;
		Str = StrCopy;
	}

	// коррекция вставляемого размера, если определен GetMaxLength()
	if (GetMaxLength() != -1 && Str.size() > static_cast<size_t>(GetMaxLength()))
	{
		Str = Str.substr(0, GetMaxLength()); // ??
	}

	if (!KeepSelection)
		RemoveSelection();

	if (!m_Flags.Check(FEDITLINE_PARENT_SINGLELINE))
	{
		if (Str.ends_with(L'\r'))
		{
			set_eol(eol::eol_type::mac);
			Str.remove_suffix(1);
		}
		else
		{
			if (Str.ends_with(L'\n'))
			{
				Str.remove_suffix(1);

				if (Str.ends_with(L'\r'))
				{
					Str.remove_suffix(1);

					if (Str.ends_with(L'\r'))
					{
						Str.remove_suffix(1);
						set_eol(eol::eol_type::bad_win);
					}
					else
						set_eol(eol::eol_type::win);
				}
				else
					set_eol(eol::eol_type::unix);
			}
			else
				set_eol(eol::eol_type::none);
		}
	}

	m_CurPos=0;

	if (const auto Mask = GetInputMask(); !Mask.empty())
	{
		RefreshStrByMask(TRUE);
		ProcessMask(Str, Mask, 0);

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

	if (const auto Mask = GetInputMask(); !Mask.empty())
	{
		ProcessMask(Str, Mask, m_CurPos);
		RefreshStrByMask();
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
			IsMouseButtonEvent(MouseEvent->dwEventFlags) &&
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
	const auto S = static_cast<int>(GetTabSize() - (RealPosToVisual(Pos) % GetTabSize()));

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
	return RealPosToVisual(m_CurPos);
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

	if (NewPos != RealPosToVisual(m_CurPos))
		m_CurPos = VisualPosToReal(NewPos);
}

int Edit::RealPosToVisual(int const Pos, position_parser_state* const  State) const
{
	const auto TabSize = GetTabExpandMode() == EXPAND_ALLTABS? 1 : GetTabSize();

	return static_cast<int>(string_pos_to_visual_pos(m_Str, Pos, TabSize));
}

int Edit::VisualPosToReal(int const Pos, position_parser_state*const  State) const
{
	const auto TabSize = GetTabExpandMode() == EXPAND_ALLTABS? 1 : GetTabSize();

	return static_cast<int>(visual_pos_to_string_pos(m_Str, Pos, TabSize));
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
	if (m_SelStart < 0 || m_Flags.Check(FEDITLINE_MARKINGBLOCK | FEDITLINE_EDITORMODE))
		return;

	bool persistent;
	if (m_Flags.Check(FEDITLINE_PARENT_SINGLELINE))
		persistent = m_Flags.Check(FEDITLINE_PERSISTENTBLOCKS); // dlgedit
	else if (!m_Flags.Check(FEDITLINE_PARENT_MULTILINE))
		persistent = Global->Opt->CmdLine.EditBlock; // cmdline
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
		for (const auto i: std::views::iota(m_SelStart, m_SelEnd))
		{
			if (CheckCharMask(Mask[i]))
			{
				m_Str[i] = MaskDefaultChar(Mask[i]);
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

void Edit::ApplyColor(std::multiset<ColorItem> const& Colors, int XPos, int FocusedLeftPos, positions_cache& RealToVisual)
{
	const auto Width = ObjWidth();

	for (const auto& CurItem: Colors)
	{
		// Skip invalid
		if (CurItem.StartPos > CurItem.EndPos)
			continue;

		auto First = RealToVisual.get(CurItem.StartPos);
		const auto LastFirst = RealToVisual.get(CurItem.EndPos);
		int LastLast = LastFirst;

		for (const auto i: std::views::iota(0, 2))
		{
			LastLast = RealToVisual.get(CurItem.EndPos + 1 + i);
			if (LastLast > LastFirst)
			{
				--LastLast;
				break;
			}
		}

		if (First - FocusedLeftPos >= Width)
			continue;

		if (LastLast < FocusedLeftPos)
			continue;

		// Special tab handling only makes sense if the color range ends with a tab and that tab is within the viewport
		if (CurItem.EndPos < m_Str.size() && m_Str[CurItem.EndPos] == L'\t' && LastFirst - FocusedLeftPos < Width)
		{
			const auto TabVisualFirst = LastFirst;
			const auto TabVisualLast = LastLast;

			if (CurItem.Flags & ECF_TABMARKCURRENT)
			{
				// ECF_TABMARKCURRENT only makes sense if the color range includes only a tab
				if (CurItem.StartPos == CurItem.EndPos && in_closed_range(TabVisualFirst, XPos + FocusedLeftPos, TabVisualLast))
					First = LastLast = XPos + FocusedLeftPos;
			}
			else
				LastLast = CurItem.Flags & ECF_TABMARKFIRST? TabVisualFirst : TabVisualLast;
		}

		First -= FocusedLeftPos;
		LastLast -= FocusedLeftPos;

		LastLast = std::min(LastLast, Width - 1);

		Global->ScrBuf->ApplyColor({ m_Where.left + First, m_Where.top, m_Where.left + LastLast, m_Where.top }, CurItem.GetColor());
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
	switch (Mask)
	{
	case EDMASK_ANY:
		return true;

	case EDMASK_DSS:
		return std::iswdigit(Char) || Char == L' ' || Char == L'-';

	case EDMASK_DIGIT:
		return std::iswdigit(Char) != 0;

	case EDMASK_ALPHA:
		return is_alpha(Char);

	case EDMASK_HEX:
		return std::iswxdigit(Char) != 0;

	case EDMASK_DIGITS:
		return std::iswdigit(Char) || Char == L' ';

	case EDMASK_BIN:
		return Char == L'0' || Char == L'1';

	default:
		return false;
	}
}

int Edit::CheckCharMask(wchar_t Chr)
{
	return any_of(
		Chr,
		EDMASK_ANY,
		EDMASK_DIGIT,
		EDMASK_DSS,
		EDMASK_ALPHA,
		EDMASK_HEX,
		EDMASK_DIGITS,
		EDMASK_BIN
	);
}

int Edit::MaskDefaultChar(wchar_t const Mask)
{
	switch (Mask)
	{
	case EDMASK_BIN:
		return L'0';

	default:
		return L' ';
	}
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
	{
		LeftPos=TabCurPos-ObjWidth()+1;
	}

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

bool Edit::is_valid_surrogate_pair_at(size_t const Position) const
{
	string_view const Str(m_Str);
	return Position < Str.size() && is_valid_surrogate_pair(Str.substr(Position));
}

static constexpr auto eol_shift = std::countr_zero(as_unsigned(std::to_underlying(FEDITLINE_EOL_MASK)));

eol::eol_type Edit::get_eol() const
{
	return static_cast<eol::eol_type>((m_Flags.Flags() & FEDITLINE_EOL_MASK) >> eol_shift);
}

void Edit::set_eol(eol::eol_type const Eol)
{
	m_Flags.Clear(FEDITLINE_EOL_MASK);
	m_Flags.Set(std::to_underlying(Eol) << eol_shift);
}

bool Edit::is_clear_selection_key(unsigned const Key)
{
	static const unsigned int Keys[]
	{
		KEY_LEFT,      KEY_NUMPAD4,
		KEY_RIGHT,     KEY_NUMPAD6,
		KEY_HOME,      KEY_NUMPAD7,
		KEY_END,       KEY_NUMPAD1,
		KEY_CTRLHOME,  KEY_RCTRLHOME,  KEY_CTRLNUMPAD7,  KEY_RCTRLNUMPAD7,
		KEY_CTRLEND,   KEY_RCTRLEND,   KEY_CTRLNUMPAD1,  KEY_RCTRLNUMPAD1,
		KEY_CTRLLEFT,  KEY_RCTRLLEFT,  KEY_CTRLNUMPAD4,  KEY_RCTRLNUMPAD4,
		KEY_CTRLRIGHT, KEY_RCTRLRIGHT, KEY_CTRLNUMPAD6,  KEY_RCTRLNUMPAD6,
		KEY_CTRLS,     KEY_RCTRLS,
	};

	return contains(Keys, Key);
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("flatten_string")
{
	static const struct
	{
		string_view Src, Expected;
	}
	Tests[]
	{
		{ {},                         {} },
		{ L"\r"sv,                    {} },
		{ L"\n"sv,                    {} },
		{ L"\r\n"sv,                  {} },
		{ L"\r\r\n"sv,                {} },
		{ L"\n1\n2\n"sv,              L"1 2"sv },
		{ L"1\r2\n3"sv,               L"1 2 3"sv },
		{ L"\n\n12\n\n\n34\n\n\n"sv,  L"12 34"sv },
	};

	string Buffer;
	for (const auto& i: Tests)
	{
		Buffer = i.Src;
		flatten_string(Buffer);
		REQUIRE(i.Expected == Buffer);
	}
}
#endif
