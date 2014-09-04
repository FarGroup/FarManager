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

#include "headers.hpp"
#pragma hdrstop

#include "edit.hpp"
#include "keyboard.hpp"
#include "macroopcode.hpp"
#include "ctrlobj.hpp"
#include "plugins.hpp"
#include "scrbuf.hpp"
#include "interf.hpp"
#include "clipboard.hpp"
#include "xlat.hpp"
#include "shortcuts.hpp"
#include "pathmix.hpp"
#include "panelmix.hpp"
#include "manager.hpp"
#include "fileedit.hpp"


const GUID& ColorItem::GetOwner() const
{
	return *reinterpret_cast<const GUID*>(Owner);
}

void ColorItem::SetOwner(const GUID& Value)
{
	static std::unordered_set<GUID, uuid_hash, uuid_equal> GuidSet;
	Owner = &*GuidSet.emplace(Value).first;
}

const FarColor& ColorItem::GetColor() const
{
	return *reinterpret_cast<const FarColor*>(Color);
}

void ColorItem::SetColor(const FarColor& Value)
{
	static std::unordered_set<FarColor, color_hash> ColorSet;
	Color = &*ColorSet.emplace(Value).first;
}

static int Recurse=0;

enum {EOL_NONE,EOL_CR,EOL_LF,EOL_CRLF,EOL_CRCRLF};
static const wchar_t *EOL_TYPE_CHARS[]={L"",L"\r",L"\n",L"\r\n",L"\r\r\n"};

static const wchar_t EDMASK_ANY    = L'X'; // позволяет вводить в строку ввода любой символ;
static const wchar_t EDMASK_DSS    = L'#'; // позволяет вводить в строку ввода цифры, пробел и знак минуса;
static const wchar_t EDMASK_DIGIT  = L'9'; // позволяет вводить в строку ввода только цифры;
static const wchar_t EDMASK_DIGITS = L'N'; // позволяет вводить в строку ввода только цифры и пробелы;
static const wchar_t EDMASK_ALPHA  = L'A'; // позволяет вводить в строку ввода только буквы.
static const wchar_t EDMASK_HEX    = L'H'; // позволяет вводить в строку ввода шестнадцатиричные символы.

Edit::Edit(SimpleScreenObject *pOwner, bool bAllocateData):
	m_Str(bAllocateData ? static_cast<wchar_t*>(xf_malloc(sizeof(wchar_t))) : nullptr),
	m_StrSize(0),
	m_CurPos(0),
	ColorList(nullptr),
	ColorCount(0),
	MaxColorCount(0),
	m_SelStart(-1),
	m_SelEnd(0),
	LeftPos(0),
	EndType(EOL_NONE)
{
	SetOwner(pOwner);

	if (bAllocateData)
		*m_Str=0;

	m_Flags.Set(FEDITLINE_EDITBEYONDEND);
	m_Flags.Change(FEDITLINE_DELREMOVESBLOCKS,Global->Opt->EdOpt.DelRemovesBlocks);
	m_Flags.Change(FEDITLINE_PERSISTENTBLOCKS,Global->Opt->EdOpt.PersistentBlocks);
	m_Flags.Change(FEDITLINE_SHOWWHITESPACE,Global->Opt->EdOpt.ShowWhiteSpace!=0);
	m_Flags.Change(FEDITLINE_SHOWLINEBREAK,Global->Opt->EdOpt.ShowWhiteSpace==1);
}

Edit::Edit(Edit&& rhs):
	m_Str(),
	m_StrSize(),
	m_CurPos(),
	ColorList(),
	ColorCount(),
	MaxColorCount(),
	m_SelStart(-1),
	m_SelEnd(),
	LeftPos(),
	EndType(EOL_NONE)
{
	*this = std::move(rhs);
}


Edit::~Edit()
{
	xf_free(ColorList);
	xf_free(m_Str);
}

void Edit::DisplayObject()
{
	if (m_Flags.Check(FEDITLINE_DROPDOWNBOX))
	{
		m_Flags.Clear(FEDITLINE_CLEARFLAG);  // при дроп-даун нам не нужно никакого unchanged text
		m_SelStart=0;
		m_SelEnd=m_StrSize; // а также считаем что все выделено -
		//    надо же отличаться от обычных Edit
	}

	//   Вычисление нового положения курсора в строке с учётом Mask.
	int Value=(GetPrevCurPos()>m_CurPos)?-1:1;
	m_CurPos=GetNextCursorPos(m_CurPos,Value);
	FastShow();

	/* $ 26.07.2000 tran
	   при DropDownBox курсор выключаем
	   не знаю даже - попробовал но не очень красиво вышло */
	if (m_Flags.Check(FEDITLINE_DROPDOWNBOX))
		::SetCursorType(0,10);
	else
	{
		if (m_Flags.Check(FEDITLINE_OVERTYPE))
		{
			int NewCursorSize=IsConsoleFullscreen()?
			                  (Global->Opt->CursorSize[3]?(int)Global->Opt->CursorSize[3]:99):
					                  (Global->Opt->CursorSize[2]?(int)Global->Opt->CursorSize[2]:99);
			::SetCursorType(1,GetCursorSize()==-1?NewCursorSize:GetCursorSize());
		}
		else
{
			int NewCursorSize=IsConsoleFullscreen()?
			                  (Global->Opt->CursorSize[1]?(int)Global->Opt->CursorSize[1]:10):
					                  (Global->Opt->CursorSize[0]?(int)Global->Opt->CursorSize[0]:10);
			::SetCursorType(1,GetCursorSize()==-1?NewCursorSize:GetCursorSize());
		}
	}

	MoveCursor(m_X1+GetLineCursorPos()-LeftPos,m_Y1);
}

void Edit::SetCursorType(bool Visible, DWORD Size)
{
	m_Flags.Change(FEDITLINE_CURSORVISIBLE,Visible);
	SetCursorSize(Size);
	::SetCursorType(Visible,Size);
}

void Edit::GetCursorType(bool& Visible, DWORD& Size) const
{
	Visible=m_Flags.Check(FEDITLINE_CURSORVISIBLE);
	Size = GetCursorSize();
}

//   Вычисление нового положения курсора в строке с учётом Mask.
int Edit::GetNextCursorPos(int Position,int Where) const
{
	int Result = Position;
	auto Mask = GetInputMask();

	if (!Mask.empty() && (Where==-1 || Where==1))
	{
		int PosChanged=FALSE;
		int MaskLen = static_cast<int>(Mask.size());

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
			for (int i=Position; i>=0; i--)
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
			for (int i=Position; i<MaskLen; i++)
			{
				if (CheckCharMask(Mask[i]))
				{
					Result=i;
					break;
				}
			}
		}
	}

	return Result;
}

void Edit::FastShow()
{
	const size_t EditLength=ObjWidth();

	if (!m_Flags.Check(FEDITLINE_EDITBEYONDEND) && m_CurPos>m_StrSize && m_StrSize>=0)
		m_CurPos=m_StrSize;

	if (GetMaxLength()!=-1)
	{
		if (m_StrSize>GetMaxLength())
		{
			m_Str[GetMaxLength()]=0;
			m_StrSize=GetMaxLength();
		}

		if (m_CurPos>GetMaxLength()-1)
			m_CurPos=GetMaxLength()>0 ? (GetMaxLength()-1):0;
	}

	int TabCurPos=GetTabCurPos();

	/* $ 31.07.2001 KM
	  ! Для комбобокса сделаем отображение строки
	    с первой позиции.
	*/
	int UnfixedLeftPos = LeftPos;
	if (!m_Flags.Check(FEDITLINE_DROPDOWNBOX))
	{
		FixLeftPos(TabCurPos);
	}

	GotoXY(m_X1,m_Y1);
	int TabSelStart=(m_SelStart==-1) ? -1:RealPosToTab(m_SelStart);
	int TabSelEnd=(m_SelEnd<0) ? -1:RealPosToTab(m_SelEnd);

	/* $ 17.08.2000 KM
	   Если есть маска, сделаем подготовку строки, то есть
	   все "постоянные" символы в маске, не являющиеся шаблонными
	   должны постоянно присутствовать в Str
	*/
	auto Mask = GetInputMask();
	if (!Mask.empty())
		RefreshStrByMask();

	string OutStr, OutStrTmp;
	OutStr.reserve(EditLength);
	OutStrTmp.reserve(EditLength);

	SetLineCursorPos(TabCurPos);
	int RealLeftPos=TabPosToReal(LeftPos);

	OutStrTmp.assign(m_Str + RealLeftPos, std::max(0, std::min(static_cast<int>(EditLength), m_StrSize-RealLeftPos)));

	{
		auto TrailingSpaces = OutStrTmp.cend();
		if (m_Flags.Check(FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE) && Mask.empty() && !OutStrTmp.empty())
		{
			TrailingSpaces = std::find_if_not(OutStrTmp.crbegin(), OutStrTmp.crend(), [](wchar_t i) { return IsSpace(i);}).base();
		}

		for (auto i = OutStrTmp.begin(), end = OutStrTmp.end(); OutStr.size() < EditLength && i != end; ++i)
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
				int S=GetTabSize()-((UnfixedLeftPos+OutStr.size()) % GetTabSize());
				OutStr.push_back((((m_Flags.Check(FEDITLINE_SHOWWHITESPACE) && m_Flags.Check(FEDITLINE_EDITORMODE)) || i >= TrailingSpaces) && (!OutStr.empty() || S==GetTabSize()))?L'\x2192':L' ');
				for (int j=1; j<S && OutStr.size() < EditLength; ++j)
				{
					OutStr.push_back(L' ');
				}
			}
			else
			{
				if (!*i)
					OutStr.push_back(L' ');
				else
					OutStr.push_back(*i);
			}
		}

		if (m_Flags.Check(FEDITLINE_PASSWORDMODE))
			OutStr.assign(OutStr.size(), L'*');

		if (m_Flags.Check(FEDITLINE_SHOWLINEBREAK) && m_Flags.Check(FEDITLINE_EDITORMODE) && (m_StrSize >= RealLeftPos) && (OutStr.size() < EditLength))
		{
			switch(EndType)
			{
			case EOL_CR:
				OutStr.push_back(Oem2Unicode[13]);
				break;
			case EOL_LF:
				OutStr.push_back(Oem2Unicode[10]);
				break;
			case EOL_CRLF:
				OutStr.push_back(Oem2Unicode[13]);
				if(OutStr.size() < EditLength)
				{
					OutStr.push_back(Oem2Unicode[10]);
				}
				break;
			case EOL_CRCRLF:
				OutStr.push_back(Oem2Unicode[13]);
				if(OutStr.size() < EditLength)
				{
					OutStr.push_back(Oem2Unicode[13]);
					if(OutStr.size() < EditLength)
					{
						OutStr.push_back(Oem2Unicode[10]);
					}
				}
				break;
			}
		}

		if (m_Flags.Check(FEDITLINE_SHOWWHITESPACE) && m_Flags.Check(FEDITLINE_EDITORMODE) && (m_StrSize >= RealLeftPos) && (OutStr.size() < EditLength) && static_cast<Editor*>(GetOwner())->IsLastLine(this))
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
				RemoveTrailingSpaces(OutStr);

			Global->FS << fmt::LeftAlign() << OutStr;
			SetColor(GetNormalColor());
			size_t BlankLength=EditLength-OutStr.size();

			if (BlankLength > 0)
			{
				Global->FS << fmt::MinWidth(BlankLength)<<L"";
			}
		}
		else
		{
			Global->FS << fmt::LeftAlign()<<fmt::ExactWidth(EditLength)<<OutStr;
		}
	}
	else
	{
		if ((TabSelStart-=LeftPos)<0)
			TabSelStart=0;

		int AllString=(TabSelEnd==-1);

		if (AllString)
			TabSelEnd=static_cast<int>(EditLength);
		else if ((TabSelEnd-=LeftPos)<0)
			TabSelEnd=0;

		OutStr.append(EditLength - OutStr.size(), L' ');

		/* $ 24.08.2000 SVS
		   ! У DropDowList`а выделение по полной программе - на всю видимую длину
		     ДАЖЕ ЕСЛИ ПУСТАЯ СТРОКА
		*/
		if (TabSelStart>=static_cast<int>(EditLength) /*|| !AllString && TabSelStart>=StrSize*/ ||
		        TabSelEnd<TabSelStart)
		{
			if (m_Flags.Check(FEDITLINE_DROPDOWNBOX))
			{
				SetColor(GetSelectedColor());
				Global->FS << fmt::MinWidth(m_X2-m_X1+1)<<OutStr;
			}
			else
				Text(OutStr);
		}
		else
		{
			Global->FS << fmt::MaxWidth(TabSelStart)<<OutStr;
			SetColor(GetSelectedColor());

			if (!m_Flags.Check(FEDITLINE_DROPDOWNBOX))
			{
				Global->FS << fmt::MaxWidth(TabSelEnd-TabSelStart) << OutStr.data() + TabSelStart;

				if (TabSelEnd<static_cast<int>(EditLength))
				{
					//SetColor(Flags.Check(FEDITLINE_CLEARFLAG) ? SelColor:Color);
					SetColor(GetNormalColor());
					Text(OutStr.data()+TabSelEnd);
				}
			}
			else
			{
				Global->FS << fmt::MinWidth(m_X2-m_X1+1)<<OutStr;
			}
		}
	}

	/* $ 26.07.2000 tran
	   при дроп-даун цвета нам не нужны */
	if (!m_Flags.Check(FEDITLINE_DROPDOWNBOX))
		ApplyColor(GetSelectedColor());
}

int Edit::RecurseProcessKey(int Key)
{
	Recurse++;
	int RetCode=ProcessKey(Manager::Key(Key));
	Recurse--;
	return RetCode;
}

// Функция вставки всякой хреновени - от шорткатов до имен файлов
int Edit::ProcessInsPath(int Key,int PrevSelStart,int PrevSelEnd)
{
	int RetCode=FALSE;
	string strPathName;

	if (Key>=KEY_RCTRL0 && Key<=KEY_RCTRL9) // шорткаты?
	{
		if (Shortcuts().Get(Key-KEY_RCTRL0,&strPathName,nullptr,nullptr,nullptr))
			RetCode=TRUE;
	}
	else // Пути/имена?
	{
		RetCode=_MakePath1(Key,strPathName,L"");
	}

	// Если что-нить получилось, именно его и вставим (PathName)
	if (RetCode)
	{
		if (m_Flags.Check(FEDITLINE_CLEARFLAG))
		{
			LeftPos=0;
			SetString(L"");
		}

		if (PrevSelStart!=-1)
		{
			m_SelStart=PrevSelStart;
			m_SelEnd=PrevSelEnd;
		}

		if (!m_Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
			DeleteBlock();

		InsertString(strPathName);
		m_Flags.Clear(FEDITLINE_CLEARFLAG);
	}

	return RetCode;
}

__int64 Edit::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return !GetLength();
		case MCODE_C_SELECTED:
			return m_SelStart != -1 && m_SelStart < m_SelEnd;
		case MCODE_C_EOF:
			return m_CurPos >= m_StrSize;
		case MCODE_C_BOF:
			return !m_CurPos;
		case MCODE_V_ITEMCOUNT:
			return m_StrSize;
		case MCODE_V_CURPOS:
			return GetLineCursorPos()+1;
		case MCODE_F_EDITOR_SEL:
		{
			int Action=(int)((intptr_t)vParam);
			if (Action) SetClearFlag(0);

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
									Select(-1,0);

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
					Select(-1,0);
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

int Edit::ProcessKey(const Manager::Key& Key)
{
	int LocalKey=Key.FarKey;
	auto Mask = GetInputMask();
	switch (LocalKey)
	{
		case KEY_ADD:
			LocalKey=L'+';
			break;
		case KEY_SUBTRACT:
			LocalKey=L'-';
			break;
		case KEY_MULTIPLY:
			LocalKey=L'*';
			break;
		case KEY_DIVIDE:
			LocalKey=L'/';
			break;
		case KEY_DECIMAL:
			LocalKey=L'.';
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

	if (!m_Flags.Check(FEDITLINE_DROPDOWNBOX) && (LocalKey==KEY_CTRLL || LocalKey==KEY_RCTRLL))
	{
		m_Flags.Swap(FEDITLINE_READONLY);
	}

	if ((((LocalKey==KEY_BS || LocalKey==KEY_DEL || LocalKey==KEY_NUMDEL) && m_Flags.Check(FEDITLINE_DELREMOVESBLOCKS)) || LocalKey==KEY_CTRLD || LocalKey==KEY_RCTRLD) &&
	        !m_Flags.Check(FEDITLINE_EDITORMODE) && m_SelStart!=-1 && m_SelStart<m_SelEnd)
	{
		DeleteBlock();
		Show();
		return TRUE;
	}

	int _Macro_IsExecuting=Global->CtrlObject->Macro.IsExecuting();

	if (!IntKeyState.ShiftPressed && (!_Macro_IsExecuting || (IsNavKey(LocalKey) && _Macro_IsExecuting)) &&
	        !IsShiftKey(LocalKey) && !Recurse &&
	        LocalKey!=KEY_SHIFT && LocalKey!=KEY_CTRL && LocalKey!=KEY_ALT &&
	        LocalKey!=KEY_RCTRL && LocalKey!=KEY_RALT && LocalKey!=KEY_NONE &&
	        LocalKey!=KEY_INS &&
	        LocalKey!=KEY_KILLFOCUS && LocalKey != KEY_GOTFOCUS &&
	        ((LocalKey&(~KEY_CTRLMASK)) != KEY_LWIN && (LocalKey&(~KEY_CTRLMASK)) != KEY_RWIN && (LocalKey&(~KEY_CTRLMASK)) != KEY_APPS)
	   )
	{
		m_Flags.Clear(FEDITLINE_MARKINGBLOCK);

		if (!m_Flags.Check(FEDITLINE_PERSISTENTBLOCKS) && !(LocalKey==KEY_CTRLINS || LocalKey==KEY_RCTRLINS || LocalKey==KEY_CTRLNUMPAD0 || LocalKey==KEY_RCTRLNUMPAD0) &&
		        !(LocalKey==KEY_SHIFTDEL||LocalKey==KEY_SHIFTNUMDEL||LocalKey==KEY_SHIFTDECIMAL) && !m_Flags.Check(FEDITLINE_EDITORMODE) &&
		        (LocalKey != KEY_CTRLQ && LocalKey != KEY_RCTRLQ) &&
		        !(LocalKey == KEY_SHIFTINS || LocalKey == KEY_SHIFTNUMPAD0))
		{
			if (m_SelStart != -1 || m_SelEnd )
			{
				PrevSelStart=m_SelStart;
				PrevSelEnd=m_SelEnd;
				Select(-1,0);
				Show();
			}
		}
	}

	if (((Global->Opt->Dialogs.EULBsClear && LocalKey==KEY_BS) || LocalKey==KEY_DEL || LocalKey==KEY_NUMDEL) &&
	        m_Flags.Check(FEDITLINE_CLEARFLAG) && m_CurPos>=m_StrSize)
		LocalKey=KEY_CTRLY;

	if ((LocalKey == KEY_SHIFTDEL || LocalKey == KEY_SHIFTNUMDEL || LocalKey == KEY_SHIFTDECIMAL) && m_Flags.Check(FEDITLINE_CLEARFLAG) && m_CurPos>=m_StrSize && m_SelStart==-1)
	{
		m_SelStart=0;
		m_SelEnd=m_StrSize;
	}

	if (m_Flags.Check(FEDITLINE_CLEARFLAG) && ((LocalKey <= 0xFFFF && LocalKey!=KEY_BS) || LocalKey==KEY_CTRLBRACKET || LocalKey==KEY_RCTRLBRACKET ||
	        LocalKey==KEY_CTRLBACKBRACKET || LocalKey==KEY_RCTRLBACKBRACKET || LocalKey==KEY_CTRLSHIFTBRACKET || LocalKey==KEY_RCTRLSHIFTBRACKET ||
	        LocalKey==KEY_CTRLSHIFTBACKBRACKET || LocalKey==KEY_RCTRLSHIFTBACKBRACKET || LocalKey==KEY_SHIFTENTER || LocalKey==KEY_SHIFTNUMENTER))
	{
		LeftPos=0;
		DisableCallback();
		SetString(L""); // mantis#0001722
		RevertCallback();
		Show();
	}

	// Здесь - вызов функции вставки путей/файлов
	if (ProcessInsPath(LocalKey,PrevSelStart,PrevSelEnd))
	{
		Show();
		return TRUE;
	}

	if (LocalKey!=KEY_NONE && LocalKey!=KEY_IDLE && LocalKey!=KEY_SHIFTINS && LocalKey!=KEY_SHIFTNUMPAD0 && (LocalKey!=KEY_CTRLINS && LocalKey!=KEY_RCTRLINS) &&
	        ((unsigned int)LocalKey<KEY_F1 || (unsigned int)LocalKey>KEY_F12) && LocalKey!=KEY_ALT && LocalKey!=KEY_SHIFT &&
	        LocalKey!=KEY_CTRL && LocalKey!=KEY_RALT && LocalKey!=KEY_RCTRL &&
	        !((LocalKey>=KEY_ALT_BASE && LocalKey <= KEY_ALT_BASE+0xFFFF) || (LocalKey>=KEY_RALT_BASE && LocalKey <= KEY_RALT_BASE+0xFFFF)) && // ???? 256 ???
	        !(((unsigned int)LocalKey>=KEY_MACRO_BASE && (unsigned int)LocalKey<=KEY_MACRO_ENDBASE) || ((unsigned int)LocalKey>=KEY_OP_BASE && (unsigned int)LocalKey <=KEY_OP_ENDBASE)) &&
	        (LocalKey!=KEY_CTRLQ && LocalKey!=KEY_RCTRLQ))
	{
		m_Flags.Clear(FEDITLINE_CLEARFLAG);
		Show();
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
					Select(-1,0);
					m_Flags.Set(FEDITLINE_MARKINGBLOCK);
				}

				if (m_SelStart!=-1 && m_SelStart<=m_CurPos)
					Select(m_SelStart,m_CurPos);
				else
				{
					int EndPos=m_CurPos+1;
					int NewStartPos=m_CurPos;

					if (EndPos>m_StrSize)
						EndPos=m_StrSize;

					if (NewStartPos>m_StrSize)
						NewStartPos=m_StrSize;

					AddSelect(NewStartPos,EndPos);
				}

				Show();
			}

			return TRUE;
		}
		case KEY_SHIFTRIGHT: case KEY_SHIFTNUMPAD6:
		{
			AdjustPersistentMark();

			if (!m_Flags.Check(FEDITLINE_MARKINGBLOCK))
			{
				Select(-1,0);
				m_Flags.Set(FEDITLINE_MARKINGBLOCK);
			}

			if ((m_SelStart!=-1 && m_SelEnd==-1) || m_SelEnd>m_CurPos)
			{
				if (m_CurPos+1==m_SelEnd)
					Select(-1,0);
				else
					Select(m_CurPos+1,m_SelEnd);
			}
			else
				AddSelect(m_CurPos,m_CurPos+1);

			RecurseProcessKey(KEY_RIGHT);
			return TRUE;
		}
		case KEY_CTRLSHIFTLEFT:  case KEY_CTRLSHIFTNUMPAD4:
		case KEY_RCTRLSHIFTLEFT: case KEY_RCTRLSHIFTNUMPAD4:
		{
			if (m_CurPos>m_StrSize)
			{
				SetPrevCurPos(m_CurPos);
				m_CurPos=m_StrSize;
			}

			if (m_CurPos>0)
				RecurseProcessKey(KEY_SHIFTLEFT);

			while (m_CurPos>0 && !(!IsWordDiv(WordDiv(), m_Str[m_CurPos]) &&
			                     IsWordDiv(WordDiv(),m_Str[m_CurPos-1]) && !IsSpace(m_Str[m_CurPos])))
			{
				if (!IsSpace(m_Str[m_CurPos]) && (IsSpace(m_Str[m_CurPos-1]) ||
				                              IsWordDiv(WordDiv(), m_Str[m_CurPos-1])))
					break;

				RecurseProcessKey(KEY_SHIFTLEFT);
			}

			Show();
			return TRUE;
		}
		case KEY_CTRLSHIFTRIGHT:  case KEY_CTRLSHIFTNUMPAD6:
		case KEY_RCTRLSHIFTRIGHT: case KEY_RCTRLSHIFTNUMPAD6:
		{
			if (m_CurPos>=m_StrSize)
				return FALSE;

			RecurseProcessKey(KEY_SHIFTRIGHT);

			while (m_CurPos<m_StrSize && !(IsWordDiv(WordDiv(), m_Str[m_CurPos]) &&
			                           !IsWordDiv(WordDiv(), m_Str[m_CurPos-1])))
			{
				if (!IsSpace(m_Str[m_CurPos]) && (IsSpace(m_Str[m_CurPos-1]) || IsWordDiv(WordDiv(), m_Str[m_CurPos-1])))
					break;

				RecurseProcessKey(KEY_SHIFTRIGHT);

				if (GetMaxLength()!=-1 && m_CurPos==GetMaxLength()-1)
					break;
			}

			Show();
			return TRUE;
		}
		case KEY_SHIFTHOME:  case KEY_SHIFTNUMPAD7:
		{
			Lock();

			while (m_CurPos>0)
				RecurseProcessKey(KEY_SHIFTLEFT);

			Unlock();
			Show();
			return TRUE;
		}
		case KEY_SHIFTEND:  case KEY_SHIFTNUMPAD1:
		{
			Lock();
			int Len;

			if (!Mask.empty())
			{
				string ShortStr(m_Str, m_StrSize);
				Len = static_cast<int>(RemoveTrailingSpaces(ShortStr).size());
			}
			else
				Len=m_StrSize;

			int LastCurPos=m_CurPos;

			while (m_CurPos<Len/*StrSize*/)
			{
				RecurseProcessKey(KEY_SHIFTRIGHT);

				if (LastCurPos==m_CurPos)break;

				LastCurPos=m_CurPos;
			}

			Unlock();
			Show();
			return TRUE;
		}
		case KEY_BS:
		{
			if (m_CurPos<=0)
				return FALSE;

			SetPrevCurPos(m_CurPos);
			m_CurPos--;

			if (m_CurPos<=LeftPos)
			{
				LeftPos-=15;

				if (LeftPos<0)
					LeftPos=0;
			}

			if (!RecurseProcessKey(KEY_DEL))
				Show();

			return TRUE;
		}
		case KEY_CTRLSHIFTBS:
		case KEY_RCTRLSHIFTBS:
		{
			DisableCallback();

			// BUGBUG
			for (int i=m_CurPos; i>=0; i--)
			{
				RecurseProcessKey(KEY_BS);
			}
			RevertCallback();
			Changed(true);
			Show();
			return TRUE;
		}
		case KEY_CTRLBS:
		case KEY_RCTRLBS:
		{
			if (m_CurPos>m_StrSize)
			{
				SetPrevCurPos(m_CurPos);
				m_CurPos=m_StrSize;
			}

			Lock();

			DisableCallback();

			// BUGBUG
			for (;;)
			{
				int StopDelete=FALSE;

				if (m_CurPos>1 && IsSpace(m_Str[m_CurPos-1])!=IsSpace(m_Str[m_CurPos-2]))
					StopDelete=TRUE;

				RecurseProcessKey(KEY_BS);

				if (!m_CurPos || StopDelete)
					break;

				if (IsWordDiv(WordDiv(),m_Str[m_CurPos-1]))
					break;
			}

			Unlock();
			RevertCallback();
			Changed(true);
			Show();
			return TRUE;
		}
		case KEY_CTRLQ:
		case KEY_RCTRLQ:
		{
			Lock();

			if (!m_Flags.Check(FEDITLINE_PERSISTENTBLOCKS) && (m_SelStart != -1 || m_Flags.Check(FEDITLINE_CLEARFLAG)))
				RecurseProcessKey(KEY_DEL);

			ProcessCtrlQ();
			Unlock();
			Show();
			return TRUE;
		}
		case KEY_OP_SELWORD:
		{
			int OldCurPos=m_CurPos;
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
				int SStart, SEnd;

				if (CalcWordFromString(m_Str,m_CurPos,&SStart,&SEnd,WordDiv()))
					Select(SStart,SEnd+(SEnd < m_StrSize?1:0));
			}

			m_CurPos=OldCurPos; // возвращаем обратно
			Show();
			return TRUE;
		}
		case KEY_OP_PLAINTEXT:
		{
			if (!m_Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
			{
				if (m_SelStart != -1 || m_Flags.Check(FEDITLINE_CLEARFLAG)) // BugZ#1053 - Неточности в $Text
					RecurseProcessKey(KEY_DEL);
			}

			ProcessInsPlainText(Global->CtrlObject->Macro.GetStringToPrint());

			Show();
			return TRUE;
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
			if (m_CurPos>=m_StrSize)
				return FALSE;

			Lock();
			DisableCallback();
			if (!Mask.empty())
			{
				int MaskLen = static_cast<int>(Mask.size());
				int ptr=m_CurPos;

				while (ptr<MaskLen)
				{
					ptr++;

					if (!CheckCharMask(Mask[ptr]) ||
					        (IsSpace(m_Str[ptr]) && !IsSpace(m_Str[ptr+1])) ||
					        (IsWordDiv(WordDiv(), m_Str[ptr])))
						break;
				}

				// BUGBUG
				for (int i=0; i<ptr-m_CurPos; i++)
					RecurseProcessKey(KEY_DEL);
			}
			else
			{
				for (;;)
				{
					int StopDelete=FALSE;

					if (m_CurPos<m_StrSize-1 && IsSpace(m_Str[m_CurPos]) && !IsSpace(m_Str[m_CurPos+1]))
						StopDelete=TRUE;

					RecurseProcessKey(KEY_DEL);

					if (m_CurPos>=m_StrSize || StopDelete)
						break;

					if (IsWordDiv(WordDiv(), m_Str[m_CurPos]))
						break;
				}
			}

			Unlock();
			RevertCallback();
			Changed(true);
			Show();
			return TRUE;
		}
		case KEY_CTRLY:
		case KEY_RCTRLY:
		{
			if (m_Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
				return TRUE;

			SetPrevCurPos(m_CurPos);
			LeftPos=m_CurPos=0;
			*m_Str=0;
			m_StrSize=0;
			m_Str=(wchar_t *)xf_realloc(m_Str,1*sizeof(wchar_t));
			Select(-1,0);
			Changed();
			Show();
			return TRUE;
		}
		case KEY_CTRLK:
		case KEY_RCTRLK:
		{
			if (m_Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
				return TRUE;

			if (m_CurPos>=m_StrSize)
				return FALSE;

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

			m_Str[m_CurPos]=0;
			m_StrSize=m_CurPos;
			m_Str=(wchar_t *)xf_realloc(m_Str,(m_StrSize+1)*sizeof(wchar_t));
			Changed();
			Show();
			return TRUE;
		}
		case KEY_HOME:        case KEY_NUMPAD7:
		case KEY_CTRLHOME:    case KEY_CTRLNUMPAD7:
		case KEY_RCTRLHOME:   case KEY_RCTRLNUMPAD7:
		{
			SetPrevCurPos(m_CurPos);
			m_CurPos=0;
			Show();
			return TRUE;
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
				string ShortStr(m_Str, m_StrSize);
				m_CurPos = static_cast<int>(RemoveTrailingSpaces(ShortStr).size());
			}
			else
				m_CurPos=m_StrSize;

			Show();
			return TRUE;
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

			return TRUE;
		}
		case KEY_RIGHT:       case KEY_NUMPAD6:        case KEY_MSWHEEL_RIGHT:
		case KEY_CTRLD:       case KEY_RCTRLD:
		{
			SetPrevCurPos(m_CurPos);

			if (!Mask.empty())
			{
				string ShortStr(m_Str, m_StrSize);
				int Len = static_cast<int>(RemoveTrailingSpaces(ShortStr).size());

				if (Len>m_CurPos)
					m_CurPos++;
			}
			else
				m_CurPos++;

			Show();
			return TRUE;
		}
		case KEY_INS:         case KEY_NUMPAD0:
		{
			m_Flags.Swap(FEDITLINE_OVERTYPE);
			Show();
			return TRUE;
		}
		case KEY_NUMDEL:
		case KEY_DEL:
		{
			if (m_Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
				return TRUE;

			if (m_CurPos>=m_StrSize)
				return FALSE;

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

						m_Str[j]=m_Str[i+1];
						j++;
					}
				}

				m_Str[j]=L' ';
			}
			else
			{
				wmemmove(m_Str+m_CurPos,m_Str+m_CurPos+1,m_StrSize-m_CurPos);
				m_StrSize--;
				m_Str=(wchar_t *)xf_realloc(m_Str,(m_StrSize+1)*sizeof(wchar_t));
			}

			Changed(true);
			Show();
			return TRUE;
		}
		case KEY_CTRLLEFT:  case KEY_CTRLNUMPAD4:
		case KEY_RCTRLLEFT: case KEY_RCTRLNUMPAD4:
		{
			SetPrevCurPos(m_CurPos);

			if (m_CurPos>m_StrSize)
				m_CurPos=m_StrSize;

			if (m_CurPos>0)
				m_CurPos--;

			while (m_CurPos>0 && !(!IsWordDiv(WordDiv(), m_Str[m_CurPos]) &&
			                     IsWordDiv(WordDiv(), m_Str[m_CurPos-1]) && !IsSpace(m_Str[m_CurPos])))
			{
				if (!IsSpace(m_Str[m_CurPos]) && IsSpace(m_Str[m_CurPos-1]))
					break;

				m_CurPos--;
			}

			Show();
			return TRUE;
		}
		case KEY_CTRLRIGHT:   case KEY_CTRLNUMPAD6:
		case KEY_RCTRLRIGHT:  case KEY_RCTRLNUMPAD6:
		{
			if (m_CurPos>=m_StrSize)
				return FALSE;

			SetPrevCurPos(m_CurPos);
			int Len;

			if (!Mask.empty())
			{
				string ShortStr(m_Str, m_StrSize);
				Len = static_cast<int>(RemoveTrailingSpaces(ShortStr).size());

				if (Len>m_CurPos)
					m_CurPos++;
			}
			else
			{
				Len=m_StrSize;
				m_CurPos++;
			}

			while (m_CurPos<Len/*StrSize*/ && !(IsWordDiv(WordDiv(),m_Str[m_CurPos]) &&
			                                  !IsWordDiv(WordDiv(), m_Str[m_CurPos-1])))
			{
				if (!IsSpace(m_Str[m_CurPos]) && IsSpace(m_Str[m_CurPos-1]))
					break;

				m_CurPos++;
			}

			Show();
			return TRUE;
		}
		case KEY_SHIFTNUMDEL:
		case KEY_SHIFTDECIMAL:
		case KEY_SHIFTDEL:
		{
			if (m_SelStart==-1 || m_SelStart>=m_SelEnd)
				return FALSE;

			RecurseProcessKey(KEY_CTRLINS);
			DeleteBlock();
			Show();
			return TRUE;
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
						string ShortStr(m_Str, m_StrSize);
						SetClipboard(RemoveTrailingSpaces(ShortStr));
					}
					else
					{
						SetClipboard(m_Str);
					}
				}
				else if (m_SelEnd<=m_StrSize) // TODO: если в начало условия добавить "StrSize &&", то пропадет баг "Ctrl-Ins в пустой строке очищает клипборд"
				{
					int Ch=m_Str[m_SelEnd];
					m_Str[m_SelEnd]=0;
					SetClipboard(m_Str+m_SelStart);
					m_Str[m_SelEnd]=Ch;
				}
			}

			return TRUE;
		}
		case KEY_SHIFTINS:    case KEY_SHIFTNUMPAD0:
		{
			string ClipText;

			if (GetMaxLength()==-1)
			{
				if (!GetClipboard(ClipText))
					return TRUE;
			}
			else
			{
				if (!GetClipboardEx(GetMaxLength(), ClipText))
					return TRUE;
			}

			if (!m_Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
			{
				DisableCallback();
				DeleteBlock();
				RevertCallback();
			}

			for (int i=StrLength(m_Str)-1; i>=0 && IsEol(m_Str[i]); i--)
				m_Str[i]=0;

			for (size_t i=0; i < ClipText.size(); ++i)
			{
				if (IsEol(ClipText[i]))
				{
					if (IsEol(i + i < ClipText.size() && ClipText[i+1]))
						ClipText.erase(i, 1);

					if (i+1 == ClipText.size())
						ClipText.resize(i);
					else
						ClipText[i] = L' ';
				}
			}

			if (m_Flags.Check(FEDITLINE_CLEARFLAG))
			{
				LeftPos=0;
				m_Flags.Clear(FEDITLINE_CLEARFLAG);
				SetString(ClipText.data());
			}
			else
			{
				InsertString(ClipText.data());
			}

			Show();
			return TRUE;
		}
		case KEY_SHIFTTAB:
		{
			SetPrevCurPos(m_CurPos);
			int Pos = GetLineCursorPos();
			SetLineCursorPos(Pos-((Pos-1) % GetTabSize()+1));

			if (GetLineCursorPos()<0) SetLineCursorPos(0); //CursorPos=0,TabSize=1 case

			SetTabCurPos(GetLineCursorPos());
			Show();
			return TRUE;
		}
		case KEY_SHIFTSPACE:
			LocalKey = KEY_SPACE;
		default:
		{
//      _D(SysLog(L"Key=0x%08X",LocalKey));
			if (LocalKey==KEY_NONE || LocalKey==KEY_IDLE || LocalKey==KEY_ENTER || LocalKey==KEY_NUMENTER || LocalKey>=65536)
				break;

			if (!m_Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
			{
				if (PrevSelStart!=-1)
				{
					m_SelStart=PrevSelStart;
					m_SelEnd=PrevSelEnd;
				}
				DisableCallback();
				DeleteBlock();
				RevertCallback();
			}

			if (InsertKey(LocalKey))
			{
				int CurWindowType = Global->WindowManager->GetCurrentWindow()->GetType();
				if (CurWindowType == windowtype_dialog || CurWindowType == windowtype_panels)
				{
					Show();
				}
			}
			return TRUE;
		}
	}

	return FALSE;
}

// обработка Ctrl-Q
int Edit::ProcessCtrlQ()
{
	INPUT_RECORD rec;
	for (;;)
	{
		DWORD Key=GetInputRecord(&rec);

		if (Key!=KEY_NONE && Key!=KEY_IDLE && rec.Event.KeyEvent.uChar.AsciiChar)
			break;

		if (Key==KEY_CONSOLE_BUFFER_RESIZE)
		{
//      int Dis=EditOutDisabled;
//      EditOutDisabled=0;
			Show();
//      EditOutDisabled=Dis;
		}
	}

	/*
	  EditOutDisabled++;
	  if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
	  {
	    DeleteBlock();
	  }
	  else
	    Flags.Clear(FEDITLINE_CLEARFLAG);
	  EditOutDisabled--;
	*/
	return InsertKey(rec.Event.KeyEvent.uChar.AsciiChar);
}

int Edit::ProcessInsPlainText(const wchar_t *str)
{
	if (*str)
	{
		InsertString(str);
		return TRUE;
	}

	return FALSE;
}

int Edit::InsertKey(int Key)
{
	bool changed=false;
	wchar_t *NewStr;

	if (m_Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
		return TRUE;

	if (Key==KEY_TAB && m_Flags.Check(FEDITLINE_OVERTYPE))
	{
		SetPrevCurPos(m_CurPos);
		int Pos = GetLineCursorPos();
		SetLineCursorPos(Pos + (GetTabSize() - (Pos % GetTabSize())));
		SetTabCurPos(GetLineCursorPos());
		return TRUE;
	}

	auto Mask = GetInputMask();
	if (!Mask.empty())
	{
		int MaskLen = static_cast<int>(Mask.size());

		if (m_CurPos<MaskLen)
		{
			if (KeyMatchedMask(Key, Mask))
			{
				if (!m_Flags.Check(FEDITLINE_OVERTYPE))
				{
					int i=MaskLen-1;

					while (!CheckCharMask(Mask[i]) && i>m_CurPos)
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
				// Здесь вариант для "ввели символ из маски", например для SetAttr - ввесли '.'
				;// char *Ptr=strchr(Mask+CurPos,Key);
			}
		}
		else if (m_CurPos<m_StrSize)
		{
			SetPrevCurPos(m_CurPos);
			m_Str[m_CurPos++]=Key;
			changed=true;
		}
	}
	else
	{
		if (GetMaxLength() == -1 || m_StrSize + 1 <= GetMaxLength())
		{
			if (m_CurPos>m_StrSize)
			{
				if (!(NewStr=(wchar_t *)xf_realloc(m_Str,(m_CurPos+1)*sizeof(wchar_t))))
					return FALSE;

				m_Str=NewStr;
				wmemset(m_Str + m_StrSize, L' ', m_CurPos - m_StrSize);
				m_Str[m_CurPos] = 0;
				m_StrSize=m_CurPos;
			}

			if (!m_Flags.Check(FEDITLINE_OVERTYPE) || m_CurPos >= m_StrSize)
				++m_StrSize;

			if (Key==KEY_TAB && (GetTabExpandMode()==EXPAND_NEWTABS || GetTabExpandMode()==EXPAND_ALLTABS))
			{
				m_StrSize--;
				InsertTab();
				return TRUE;
			}

			if (!(NewStr=(wchar_t *)xf_realloc(m_Str,(m_StrSize+1)*sizeof(wchar_t))))
				return FALSE;

			m_Str=NewStr;

			if (!m_Flags.Check(FEDITLINE_OVERTYPE))
			{
				wmemmove(m_Str+m_CurPos+1,m_Str+m_CurPos,m_StrSize-m_CurPos);

				if (m_SelStart!=-1)
				{
					if (m_SelEnd!=-1 && m_CurPos<m_SelEnd)
						m_SelEnd++;

					if (m_CurPos<m_SelStart)
						m_SelStart++;
				}
			}

			SetPrevCurPos(m_CurPos);
			m_Str[m_CurPos++]=Key;
			changed=true;
		}
		else if (m_Flags.Check(FEDITLINE_OVERTYPE))
		{
			if (m_CurPos < m_StrSize)
			{
				SetPrevCurPos(m_CurPos);
				m_Str[m_CurPos++]=Key;
				changed=true;
			}
		}
		else
			MessageBeep(MB_ICONHAND);
	}

	m_Str[m_StrSize]=0;

	if (changed) Changed();

	return TRUE;
}

void Edit::GetString(wchar_t *Str,int MaxSize) const
{
	//xwcsncpy(Str, this->Str,MaxSize);
	wmemmove(Str,m_Str,std::min(m_StrSize,MaxSize-1));
	Str[std::min(m_StrSize,MaxSize-1)]=0;
	Str[MaxSize-1]=0;
}

void Edit::GetString(string &strStr) const
{
	strStr = m_Str;
}

const wchar_t* Edit::GetStringAddr() const
{
	return m_Str;
}

void  Edit::SetHiString(const string& Str)
{
	if (m_Flags.Check(FEDITLINE_READONLY))
		return;

	string NewStr;
	HiText2Str(NewStr, Str);
	Select(-1,0);
	SetBinaryString(NewStr.data(), static_cast<int>(NewStr.size()));
}

void Edit::SetString(const wchar_t *Str, int Length)
{
	if (m_Flags.Check(FEDITLINE_READONLY))
		return;

	Select(-1,0);
	SetBinaryString(Str,Length==-1? StrLength(Str) : Length);
}

void Edit::SetEOL(const wchar_t *EOL)
{
	EndType=EOL_NONE;

	if (EOL && *EOL)
	{
		if (EOL[0]==L'\r')
			if (EOL[1]==L'\n')
				EndType=EOL_CRLF;
			else if (EOL[1]==L'\r' && EOL[2]==L'\n')
				EndType=EOL_CRCRLF;
			else
				EndType=EOL_CR;
		else if (EOL[0]==L'\n')
			EndType=EOL_LF;
	}
}

const wchar_t *Edit::GetEOL() const
{
	return EOL_TYPE_CHARS[EndType];
}

/* $ 25.07.2000 tran
   примечание:
   в этом методе DropDownBox не обрабатывается
   ибо он вызывается только из SetString и из класса Editor
   в Dialog он нигде не вызывается */
void Edit::SetBinaryString(const wchar_t *Str,int Length)
{
	if (m_Flags.Check(FEDITLINE_READONLY))
		return;

	// коррекция вставляемого размера, если определен GetMaxLength()
	if (GetMaxLength() != -1 && Length > GetMaxLength())
	{
		Length=GetMaxLength(); // ??
	}

	if (Length>0 && !m_Flags.Check(FEDITLINE_PARENT_SINGLELINE))
	{
		if (Str[Length-1]==L'\r')
		{
			EndType=EOL_CR;
			Length--;
		}
		else
		{
			if (Str[Length-1]==L'\n')
			{
				Length--;

				if (Length > 0 && Str[Length-1]==L'\r')
				{
					Length--;

					if (Length > 0 && Str[Length-1]==L'\r')
					{
						Length--;
						EndType=EOL_CRCRLF;
					}
					else
						EndType=EOL_CRLF;
				}
				else
					EndType=EOL_LF;
			}
			else
				EndType=EOL_NONE;
		}
	}

	m_CurPos=0;

	auto Mask = GetInputMask();
	if (!Mask.empty())
	{
		RefreshStrByMask(TRUE);
		int maskLen = static_cast<int>(Mask.size());

		for (int i=0,j=0; i < maskLen && j<maskLen && j<Length;)
		{
			if (CheckCharMask(Mask[i]))
			{
				int goLoop=FALSE;

				if (KeyMatchedMask(Str[j], Mask))
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
		   обычно вводится нечто вроде SetBinaryString("",0)
		   Т.е. таким образом мы добиваемся "инициализации" строки с маской
		*/
		RefreshStrByMask(!*Str);
	}
	else
	{
		wchar_t *NewStr=(wchar_t *)xf_realloc_nomove(m_Str,(Length+1)*sizeof(wchar_t));

		if (!NewStr)
			return;

		m_Str=NewStr;
		m_StrSize=Length;
		wmemcpy(m_Str,Str,Length);
		m_Str[Length]=0;

		if (GetTabExpandMode() == EXPAND_ALLTABS)
			ReplaceTabs();

		SetPrevCurPos(m_CurPos);
		m_CurPos=m_StrSize;
	}

	Changed();
}

void Edit::GetBinaryString(const wchar_t **Str,const wchar_t **EOL,intptr_t &Length) const
{
	*Str=m_Str;

	if (EOL)
		*EOL=EOL_TYPE_CHARS[EndType];

	Length=m_StrSize; //???
}

int Edit::GetSelString(wchar_t *Str, int MaxSize)
{
	if (m_SelStart==-1 || (m_SelEnd!=-1 && m_SelEnd<=m_SelStart) ||
	        m_SelStart>=m_StrSize)
	{
		*Str=0;
		return FALSE;
	}

	int CopyLength;

	if (m_SelEnd==-1)
		CopyLength=MaxSize;
	else
		CopyLength=std::min(MaxSize,m_SelEnd-m_SelStart+1);

	xwcsncpy(Str,m_Str+m_SelStart,CopyLength);
	return TRUE;
}

int Edit::GetSelString(string &strStr, size_t MaxSize) const
{
	if (m_SelStart==-1 || (m_SelEnd!=-1 && m_SelEnd<=m_SelStart) ||
	        m_SelStart>=m_StrSize)
	{
		strStr.clear();
		return FALSE;
	}

	size_t CopyLength;

	if (MaxSize == string::npos)
		MaxSize = m_StrSize;

	if (m_SelEnd==-1)
		CopyLength=MaxSize;
	else
		CopyLength=std::min(MaxSize, static_cast<size_t>(m_SelEnd-m_SelStart+1));

	strStr.assign(m_Str + m_SelStart, CopyLength);
	return TRUE;
}

void Edit::AppendString(const wchar_t *Str)
{
	int LastPos = m_CurPos;
	m_CurPos = GetLength();
	InsertString(Str);
	m_CurPos = LastPos;
}

void Edit::InsertString(const string& Str)
{
	if (m_Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
		return;

	if (!m_Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
		DeleteBlock();

	InsertBinaryString(Str.data(), static_cast<int>(Str.size()));
}

void Edit::InsertBinaryString(const wchar_t *Str,int Length)
{
	wchar_t *NewStr;

	if (m_Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
		return;

	m_Flags.Clear(FEDITLINE_CLEARFLAG);

	auto Mask = GetInputMask();
	if (!Mask.empty())
	{
		int Pos=m_CurPos;
		int MaskLen = static_cast<int>(Mask.size());

		if (Pos<MaskLen)
		{
			//_SVS(SysLog(L"InsertBinaryString ==> Str='%s' (Length=%d) Mask='%s'",Str,Length,Mask+Pos));
			int StrLen=(MaskLen-Pos>Length)?Length:MaskLen-Pos;

			/* $ 15.11.2000 KM
			   Внесены исправления для правильной работы PasteFromClipboard
			   в строке с маской
			*/
			for (int i=Pos,j=0; j<StrLen+Pos;)
			{
				if (CheckCharMask(Mask[i]))
				{
					int goLoop=FALSE;

					if (j < Length && KeyMatchedMask(Str[j], Mask))
					{
						InsertKey(Str[j]);
						//_SVS(SysLog(L"InsertBinaryString ==> InsertKey(Str[%d]='%c');",j,Str[j]));
					}
					else
						goLoop=TRUE;

					j++;

					if (goLoop) continue;
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
		//_SVS(SysLog(L"InsertBinaryString ==> this->Str='%s'",this->Str));
	}
	else
	{
		if (GetMaxLength() != -1 && m_StrSize+Length > GetMaxLength())
		{
			// коррекция вставляемого размера, если определен GetMaxLength()
			if (m_StrSize < GetMaxLength())
			{
				Length=GetMaxLength()-m_StrSize;
			}
		}

		if (GetMaxLength() == -1 || m_StrSize + Length <= GetMaxLength())
		{
			if (m_CurPos>m_StrSize)
			{
				if (!(NewStr=(wchar_t *)xf_realloc(m_Str,(m_CurPos+1)*sizeof(wchar_t))))
					return;

				m_Str=NewStr;
				wmemset(m_Str + m_StrSize, L' ', m_CurPos - m_StrSize);
				m_Str[m_CurPos] = 0;
				m_StrSize=m_CurPos;
			}

			string TmpStr(m_Str + m_CurPos, m_StrSize-m_CurPos);

			m_StrSize+=Length;

			if (!(NewStr=(wchar_t *)xf_realloc(m_Str,(m_StrSize+1)*sizeof(wchar_t))))
			{
				return;
			}

			m_Str=NewStr;
			wmemcpy(m_Str + m_CurPos, Str, Length);
			SetPrevCurPos(m_CurPos);
			m_CurPos+=Length;
			wmemcpy(m_Str + m_CurPos, TmpStr.data(), TmpStr.size());
			m_Str[m_StrSize]=0;

			if (GetTabExpandMode() == EXPAND_ALLTABS)
				ReplaceTabs();

			Changed();
		}
		else
			MessageBeep(MB_ICONHAND);
	}
}

int Edit::GetLength() const
{
	return m_StrSize;
}

int Edit::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (!(MouseEvent->dwButtonState & 3))
		return FALSE;

	if (MouseEvent->dwMousePosition.X<m_X1 || MouseEvent->dwMousePosition.X>m_X2 ||
	        MouseEvent->dwMousePosition.Y!=m_Y1)
		return FALSE;

	//SetClearFlag(0); // пусть едитор сам заботится о снятии клеар-текста?
	SetTabCurPos(MouseEvent->dwMousePosition.X - m_X1 + LeftPos);

	if (!m_Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
		Select(-1,0);

	if (MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED)
	{
		static int PrevDoubleClick=0;
		static COORD PrevPosition={};

		if (GetTickCount()-PrevDoubleClick<=GetDoubleClickTime() && MouseEvent->dwEventFlags!=MOUSE_MOVED &&
		        PrevPosition.X == MouseEvent->dwMousePosition.X && PrevPosition.Y == MouseEvent->dwMousePosition.Y)
		{
			Select(0,m_StrSize);
			PrevDoubleClick=0;
			PrevPosition.X=0;
			PrevPosition.Y=0;
		}

		if (MouseEvent->dwEventFlags==DOUBLE_CLICK)
		{
			ProcessKey(Manager::Key(KEY_OP_SELWORD));
			PrevDoubleClick=GetTickCount();
			PrevPosition=MouseEvent->dwMousePosition;
		}
		else
		{
			PrevDoubleClick=0;
			PrevPosition.X=0;
			PrevPosition.Y=0;
		}
	}

	Show();
	return TRUE;
}

/* $ 03.08.2000 KM
   Немного изменён алгоритм из-за необходимости
   добавления поиска целых слов.
*/
int Edit::Search(const string& Str,const string &UpperStr, const string &LowerStr, RegExp &re, RegExpMatch *pm,string& ReplaceStr,int Position,int Case,int WholeWords,int Reverse,int Regexp,int PreserveStyle, int *SearchLength)
{
	return SearchString(m_Str,this->m_StrSize,Str,UpperStr,LowerStr,re,pm,ReplaceStr,m_CurPos,Position,Case,WholeWords,Reverse,Regexp,PreserveStyle,SearchLength,WordDiv().data());
}

void Edit::InsertTab()
{
	wchar_t *TabPtr;
	int Pos,S;

	if (m_Flags.Check(FEDITLINE_READONLY))
		return;

	Pos=m_CurPos;
	S=GetTabSize()-(Pos % GetTabSize());

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

	int PrevStrSize=m_StrSize;
	m_StrSize+=S;
	m_CurPos+=S;
	m_Str=(wchar_t *)xf_realloc(m_Str,(m_StrSize+1)*sizeof(wchar_t));
	TabPtr=m_Str+Pos;
	wmemmove(TabPtr+S,TabPtr,PrevStrSize-Pos);
	wmemset(TabPtr,L' ',S);
	m_Str[m_StrSize]=0;
	Changed();
}

bool Edit::ReplaceTabs()
{
	wchar_t *TabPtr;
	int Pos=0;

	if (m_Flags.Check(FEDITLINE_READONLY))
		return false;

	bool changed=false;

	while ((TabPtr = wmemchr(m_Str+Pos, L'\t', m_StrSize-Pos)))
	{
		changed=true;
		Pos=(int)(TabPtr-m_Str);
		int S=GetTabSize()-((int)(TabPtr-m_Str) % GetTabSize());

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

		int PrevStrSize=m_StrSize;
		m_StrSize+=S-1;

		if (m_CurPos>Pos)
			m_CurPos+=S-1;

		m_Str=(wchar_t *)xf_realloc(m_Str,(m_StrSize+1)*sizeof(wchar_t));
		TabPtr=m_Str+Pos;
		wmemmove(TabPtr+S,TabPtr+1,PrevStrSize-Pos);
		wmemset(TabPtr,L' ',S);
		m_Str[m_StrSize]=0;
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
	auto Mask = GetInputMask();
	if (!Mask.empty())
	{
		string ShortStr(m_Str, m_StrSize);
		int Pos = static_cast<int>(RemoveTrailingSpaces(ShortStr).size());

		if (NewPos>Pos)
			NewPos=Pos;
	}

	m_CurPos=TabPosToReal(NewPos);
}

int Edit::RealPosToTab(int Pos) const
{
	return RealPosToTab(0, 0, Pos, nullptr);
}

int Edit::RealPosToTab(int PrevLength, int PrevPos, int Pos, int* CorrectPos) const
{
	// Корректировка табов
	bool bCorrectPos = CorrectPos && *CorrectPos;

	if (CorrectPos)
		*CorrectPos = 0;

	// Если у нас все табы преобразуются в пробелы, то просто вычисляем расстояние
	if (GetTabExpandMode() == EXPAND_ALLTABS)
		return PrevLength+Pos-PrevPos;

	// Инциализируем результирующую длину предыдущим значением
	int TabPos = PrevLength;

	// Если предыдущая позиция за концом строки, то табов там точно нет и
	// вычислять особо ничего не надо, иначе производим вычисление
	if (PrevPos >= m_StrSize)
		TabPos += Pos-PrevPos;
	else
	{
		// Начинаем вычисление с предыдущей позиции
		int Index = PrevPos;

		// Проходим по всем символам до позиции поиска, если она ещё в пределах строки,
		// либо до конца строки, если позиция поиска за пределами строки
		for (; Index < std::min(Pos, m_StrSize); Index++)

			// Обрабатываем табы
			if (m_Str[Index] == L'\t')
			{
				// Если есть необходимость делать корректировку табов и эта коректировка
				// ещё не проводилась, то увеличиваем длину обрабатываемой строки на еденицу
				if (bCorrectPos)
				{
					++Pos;
					*CorrectPos = 1;
					bCorrectPos = false;
				}

				// Расчитываем длину таба с учётом настроек и текущей позиции в строке
				TabPos += GetTabSize()-(TabPos%GetTabSize());
			}
		// Обрабатываем все отсальные симовлы
			else
				TabPos++;

		// Если позиция находится за пределами строки, то там точно нет табов и всё просто
		if (Pos >= m_StrSize)
			TabPos += Pos-Index;
	}

	return TabPos;
}

int Edit::TabPosToReal(int Pos) const
{
	if (GetTabExpandMode() == EXPAND_ALLTABS)
		return Pos;

	int Index = 0;

	for (int TabPos = 0; TabPos < Pos; Index++)
	{
		if (Index > m_StrSize)
		{
			Index += Pos-TabPos;
			break;
		}

		if (m_Str[Index] == L'\t')
		{
			int NewTabPos = TabPos+GetTabSize()-(TabPos%GetTabSize());

			if (NewTabPos > Pos)
				break;

			TabPos = NewTabPos;
		}
		else
		{
			TabPos++;
		}
	}

	return Index;
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
		m_SelStart=-1;
		m_SelEnd=0;
	}
}

void Edit::AddSelect(int Start,int End)
{
	if (Start<m_SelStart || m_SelStart==-1)
		m_SelStart=Start;

	if (End==-1 || (End>m_SelEnd && m_SelEnd!=-1))
		m_SelEnd=End;

	if (m_SelEnd>m_StrSize)
		m_SelEnd=m_StrSize;

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

	if (End > m_StrSize)
		End = -1;

	if (Start > m_StrSize)
		Start = m_StrSize;
}

void Edit::AdjustMarkBlock()
{
	bool mark = false;
	if (m_SelStart >= 0)
	{
		int end = m_SelEnd > m_StrSize || m_SelEnd == -1 ? m_StrSize : m_SelEnd;
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

	int end = m_SelEnd > m_StrSize || m_SelEnd == -1 ? m_StrSize : m_SelEnd;
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

	auto Mask = GetInputMask();
	if (!Mask.empty())
	{
		for (int i=m_SelStart; i<m_SelEnd; i++)
		{
			if (CheckCharMask(Mask[i]))
				m_Str[i]=L' ';
		}

		m_CurPos=m_SelStart;
	}
	else
	{
		int From=m_SelStart,To=m_SelEnd;

		if (From>m_StrSize)From=m_StrSize;

		if (To>m_StrSize)To=m_StrSize;

		wmemmove(m_Str+From,m_Str+To,m_StrSize-To+1);
		m_StrSize-=To-From;

		if (m_CurPos>From)
		{
			if (m_CurPos<To)
				m_CurPos=From;
			else
				m_CurPos-=To-From;
		}

		m_Str=(wchar_t *)xf_realloc(m_Str,(m_StrSize+1)*sizeof(wchar_t));
	}

	m_SelStart=-1;
	m_SelEnd=0;
	m_Flags.Clear(FEDITLINE_MARKINGBLOCK);

	// OT: Проверка на корректность поведени строки при удалении и вставки
	if (m_Flags.Check((FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE)))
	{
		if (LeftPos>m_CurPos)
			LeftPos=m_CurPos;
	}

	Changed(true);
}

void Edit::AddColor(ColorItem *col,bool skipsort)
{
	if (ColorCount==MaxColorCount)
	{
		if (ColorCount<256)
		{
			if (!(ColorCount & 15))
			{
				MaxColorCount=ColorCount+16;
				ColorList=(ColorItem *)xf_realloc(ColorList,(MaxColorCount)*sizeof(*ColorList));
			}
		}
		else if (ColorCount<2048)
		{
			if (!(ColorCount & 255))
			{
				MaxColorCount=ColorCount+256;
				ColorList=(ColorItem *)xf_realloc(ColorList,(MaxColorCount)*sizeof(*ColorList));
			}
		}
		else if (ColorCount<65536)
		{
			if (!(ColorCount & 2047))
			{
				MaxColorCount=ColorCount+2048;
				ColorList=(ColorItem *)xf_realloc(ColorList,(MaxColorCount)*sizeof(*ColorList));
			}
		}
		else if (!(ColorCount & 65535))
		{
			MaxColorCount=ColorCount+65536;
			ColorList=(ColorItem *)xf_realloc(ColorList,(MaxColorCount)*sizeof(*ColorList));
		}
	}

	#ifdef _DEBUG
	//_ASSERTE(ColorCount<MaxColorCount);
	#endif

	if (skipsort && !ColorListFlags.Check(ECLF_NEEDSORT) && ColorCount && ColorList[ColorCount-1].Priority>col->Priority)
		ColorListFlags.Set(ECLF_NEEDSORT);


	ColorList[ColorCount++]=*col;

	if (!skipsort)
	{
		std::stable_sort(ColorList, ColorList + ColorCount);
	}
}

void Edit::SortColorUnlocked()
{
	if (ColorListFlags.Check(ECLF_NEEDFREE))
	{
		ColorListFlags.Clear(ECLF_NEEDFREE);
		if (!ColorCount)
		{
			xf_free(ColorList);
			ColorList=nullptr;
			MaxColorCount = 0;
		}
	}

	if (ColorListFlags.Check(ECLF_NEEDSORT))
	{
		ColorListFlags.Clear(ECLF_NEEDSORT);
		std::stable_sort(ColorList, ColorList + ColorCount);
	}
}

int Edit::DeleteColor(int ColorPos,const GUID& Owner,bool skipfree)
{
	int Src;

	if (!ColorCount)
		return FALSE;

	int Dest=0;

	if (ColorPos!=-1)
	{
		for (Src=0; Src<ColorCount; Src++)
		{
			if ((ColorList[Src].StartPos!=ColorPos) || (Owner != ColorList[Src].GetOwner()))
			{
				if (Dest!=Src)
					ColorList[Dest]=ColorList[Src];

				Dest++;
			}
		}
	}
	else
	{
		for (Src=0; Src<ColorCount; Src++)
		{
			if (Owner != ColorList[Src].GetOwner())
			{
				if (Dest!=Src)
					ColorList[Dest]=ColorList[Src];

				Dest++;
			}
		}
	}

	int DelCount=ColorCount-Dest;
	ColorCount=Dest;

	if (!ColorCount)
	{
		if (skipfree)
		{
			ColorListFlags.Set(ECLF_NEEDFREE);
		}
		else
		{
			xf_free(ColorList);
			ColorList=nullptr;
			MaxColorCount = 0;
		}
	}

	return DelCount;
}

int Edit::GetColor(ColorItem *col,int Item) const
{
	if (Item >= ColorCount)
		return FALSE;

	*col=ColorList[Item];
	return TRUE;
}

void Edit::ApplyColor(const FarColor& SelColor)
{
	// Для оптимизации сохраняем вычисленные позиции между итерациями цикла
	int Pos = INT_MIN, TabPos = INT_MIN, TabEditorPos = INT_MIN;

	int XPos = 0;
	if(m_Flags.Check(FEDITLINE_EDITORMODE))
	{
		EditorInfo ei={sizeof(EditorInfo)};
		Global->WindowManager->GetCurrentEditor()->EditorControl(ECTL_GETINFO, 0, &ei);
		XPos = ei.CurTabPos - ei.LeftPos;
	}

	// Обрабатываем элементы ракраски
	for (int Col = 0; Col < ColorCount; Col++)
	{
		ColorItem *CurItem = ColorList+Col;

		// Пропускаем элементы у которых начало больше конца
		if (CurItem->StartPos > CurItem->EndPos)
			continue;

		// Отсекаем элементы заведомо не попадающие на экран
		if (CurItem->StartPos-LeftPos > m_X2 && CurItem->EndPos-LeftPos < m_X1)
			continue;

		int Length = CurItem->EndPos-CurItem->StartPos+1;

		if (CurItem->StartPos+Length >= m_StrSize)
			Length = m_StrSize-CurItem->StartPos;

		// Получаем начальную позицию
		int RealStart, Start;

		// Если предыдущая позиция равна текущей, то ничего не вычисляем
		// и сразу берём ранее вычисленное значение
		if (Pos == CurItem->StartPos)
		{
			RealStart = TabPos;
			Start = TabEditorPos;
		}
		// Если вычисление идёт первый раз или предыдущая позиция больше текущей,
		// то производим вычисление с начала строки
		else if (Pos == INT_MIN || CurItem->StartPos < Pos)
		{
			RealStart = RealPosToTab(CurItem->StartPos);
			Start = RealStart-LeftPos;
		}
		// Для отптимизации делаем вычисление относительно предыдущей позиции
		else
		{
			RealStart = RealPosToTab(TabPos, Pos, CurItem->StartPos, nullptr);
			Start = RealStart-LeftPos;
		}

		// Запоминаем вычисленные значения для их дальнейшего повторного использования
		Pos = CurItem->StartPos;
		TabPos = RealStart;
		TabEditorPos = Start;

		// Пропускаем элементы раскраски у которых начальная позиция за экраном
		if (Start > m_X2)
			continue;

		// Корректировка относительно табов (отключается, если присутвует флаг ECF_TABMARKFIRST)
		int CorrectPos = CurItem->Flags & ECF_TABMARKFIRST ? 0 : 1;

		// Получаем конечную позицию
		int EndPos = CurItem->EndPos;
		int RealEnd, End;

		bool TabMarkCurrent=false;

		// Обрабатываем случай, когда предыдущая позиция равна текущей, то есть
		// длина раскрашиваемой строкии равна 1
		if (Pos == EndPos)
		{
			// Если необходимо делать корректироку относительно табов и единственный
			// символ строки -- это таб, то делаем расчёт с учтом корректировки,
			// иначе ничего не вычисялем и берём старые значения
			if (CorrectPos && EndPos < m_StrSize && m_Str[EndPos] == L'\t')
			{
				RealEnd = RealPosToTab(TabPos, Pos, ++EndPos, nullptr);
				End = RealEnd-LeftPos;
				TabMarkCurrent = (CurItem->Flags & ECF_TABMARKCURRENT) && XPos>=Start && XPos<End;
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
			End = RealEnd-LeftPos;
		}
		// Для отптимизации делаем вычисление относительно предыдущей позиции (с учётом
		// корректировки относительно табов)
		else
		{
			// Mantis#0001718: Отсутствие ECF_TABMARKFIRST не всегда корректно отрабатывает
			// Коррекция с учетом последнего таба
			if (CorrectPos && EndPos < m_StrSize && m_Str[EndPos] == L'\t')
				RealEnd = RealPosToTab(TabPos, Pos, ++EndPos, nullptr);
			else
			{
				RealEnd = RealPosToTab(TabPos, Pos, EndPos, &CorrectPos);
				EndPos += CorrectPos;
			}
			End = RealEnd-LeftPos;
		}

		// Запоминаем вычисленные значения для их дальнейшего повторного использования
		Pos = EndPos;
		TabPos = RealEnd;
		TabEditorPos = End;

		// Пропускаем элементы раскраски у которых конечная позиция меньше левой границы экрана
		if (End < m_X1)
			continue;

		// Обрезаем раскраску элемента по экрану
		if (Start < m_X1)
			Start = m_X1;

		if (End > m_X2)
			End = m_X2;

		if(TabMarkCurrent)
		{
			Start = XPos;
			End = XPos+1;
		}

		// Устанавливаем длину раскрашиваемого элемента
		Length = End-Start+1;

		if (Length < m_X2)
			Length -= CorrectPos;

		// Раскрашиваем элемент, если есть что раскрашивать
		if (Length > 0)
		{
			Global->ScrBuf->ApplyColor(
			    Start,
			    m_Y1,
			    Start+Length-1,
			    m_Y1,
			    CurItem->GetColor(),
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
	//   Для CmdLine - если нет выделения, преобразуем всю строку
	if (All && m_SelStart == -1 && !m_SelEnd)
	{
		::Xlat(m_Str,0,StrLength(m_Str),Global->Opt->XLat.Flags);
		Changed();
		Show();
		return;
	}

	if (m_SelStart != -1 && m_SelStart != m_SelEnd)
	{
		if (m_SelEnd == -1)
			m_SelEnd=StrLength(m_Str);

		::Xlat(m_Str,m_SelStart,m_SelEnd,Global->Opt->XLat.Flags);
		Changed();
		Show();
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
		int start=m_CurPos, StrSize=StrLength(m_Str);
		bool DoXlat=true;

		if (IsWordDiv(Global->Opt->XLat.strWordDivForXlat,m_Str[start]))
		{
			if (start) start--;

			DoXlat=(!IsWordDiv(Global->Opt->XLat.strWordDivForXlat,m_Str[start]));
		}

		if (DoXlat)
		{
			while (start>=0 && !IsWordDiv(Global->Opt->XLat.strWordDivForXlat,m_Str[start]))
				start--;

			start++;
			int end=start+1;

			while (end<StrSize && !IsWordDiv(Global->Opt->XLat.strWordDivForXlat,m_Str[end]))
				end++;

			::Xlat(m_Str,start,end,Global->Opt->XLat.Flags);
			Changed();
			Show();
		}
	}
}

/* $ 15.11.2000 KM
   Проверяет: попадает ли символ в разрешённый
   диапазон символов, пропускаемых маской
*/
int Edit::KeyMatchedMask(int Key, const string& Mask) const
{
	int Inserted=FALSE;

	if (Mask[m_CurPos]==EDMASK_ANY)
		Inserted=TRUE;
	else if (Mask[m_CurPos]==EDMASK_DSS && (iswdigit(Key) || Key==L' ' || Key==L'-'))
		Inserted=TRUE;
	else if (Mask[m_CurPos]==EDMASK_DIGITS && (iswdigit(Key) || Key==L' '))
		Inserted=TRUE;
	else if (Mask[m_CurPos]==EDMASK_DIGIT && (iswdigit(Key)))
		Inserted=TRUE;
	else if (Mask[m_CurPos]==EDMASK_ALPHA && IsAlpha(Key))
		Inserted=TRUE;
	else if (Mask[m_CurPos]==EDMASK_HEX && (iswdigit(Key) || (Upper(Key)>=L'A' && Upper(Key)<=L'F') || (Upper(Key)>=L'a' && Upper(Key)<=L'f')))
		Inserted=TRUE;

	return Inserted;
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
	return static_cast<Editor*>(GetOwner())->GetNormalColor();
}

const FarColor& Edit::GetSelectedColor() const
{
	return static_cast<Editor*>(GetOwner())->GetSelectedColor();
}

const FarColor& Edit::GetUnchangedColor() const
{
	return GetNormalColor();
}

const int Edit::GetTabSize() const
{
	return static_cast<Editor*>(GetOwner())->GetTabSize();
}

const EXPAND_TABS Edit::GetTabExpandMode() const
{
	return static_cast<Editor*>(GetOwner())->GetConvertTabs();
}

const string& Edit::WordDiv() const
{
	return static_cast<Editor*>(GetOwner())->GetWordDiv();
}

int Edit::GetCursorSize() const
{
	return -1;
}

int Edit::GetMacroSelectionStart() const
{
	return static_cast<Editor*>(GetOwner())->GetMacroSelectionStart();
}

void Edit::SetMacroSelectionStart(int Value)
{
	static_cast<Editor*>(GetOwner())->SetMacroSelectionStart(Value);
}

int Edit::GetLineCursorPos() const
{
	return static_cast<Editor*>(GetOwner())->GetLineCursorPos();
}

void Edit::SetLineCursorPos(int Value)
{
	return static_cast<Editor*>(GetOwner())->SetLineCursorPos(Value);
}
