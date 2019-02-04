﻿/*
fnparce.cpp

Парсер файловых ассоциаций
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

#include "fnparce.hpp"

#include "panel.hpp"
#include "ctrlobj.hpp"
#include "flink.hpp"
#include "cmdline.hpp"
#include "filepanels.hpp"
#include "dialog.hpp"
#include "DlgGuid.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "mix.hpp"
#include "lang.hpp"
#include "cvtname.hpp"
#include "global.hpp"
#include "filelist.hpp"

#include "platform.env.hpp"
#include "platform.fs.hpp"

#include "format.hpp"

list_names::names::~names()
{
	const auto& Delete = [](string_view const Filename)
	{
		if (!Filename.empty())
			os::fs::delete_file(Filename);
	};

	Delete(Name);
	Delete(ShortName);
}

subst_context::subst_context(string_view NameStr, string_view ShortNameStr):
	Name(NameStr),
	ShortName(ShortNameStr),
	Path(NameStr)
{
	if (ContainsSlash(Path) && CutToParent(Path))
	{
		Name = PointToName(Name);
		ShortName = PointToName(ShortNameStr);
	}
	else
	{
		Path = {};
	}
}

bool list_names::any() const
{
	return !This.Name.empty() || !This.ShortName.empty() || !Another.Name.empty() || !Another.ShortName.empty();
}

struct subst_data
{
	struct
	{
		struct
		{
			string_view Name;
			string_view NameOnly;
			string* ListName{};
		}
		Normal, Short;
		panel_ptr Panel;

		string_view GetDescription() const
		{
			if (!m_Description.second)
			{
				if (const auto FList = dynamic_cast<FileList*>(Panel.get()))
				{
					FList->ReadDiz();
					// BUGBUG size
					m_Description.first = NullToEmpty(FList->GetDescription(string(Normal.Name), string(Short.Name), 0));
				}
				m_Description.second = true;
			}

			return m_Description.first;
		}

	private:
		// TODO: std::optional
		mutable std::pair<string_view, bool> m_Description;
	}
	This, Another;

	auto& Default() { return PassivePanel? Another : This; }
	const auto& Default() const { return PassivePanel? Another : This; }

	string CmdDir;
	bool PreserveLFN;
	bool PassivePanel;
	bool EscapeAmpersands;
};


// Str=if exist !#!\!^!.! far:edit < diff -c -p "!#!\!^!.!" !\!.!

namespace tokens
{
	const auto
		passive_panel                = L"!#"sv,
		active_panel                 = L"!^"sv,
		exclamation                  = L"!!"sv,
		name_extension               = L"!.!"sv,
		short_name                   = L"!~"sv,
		short_extension              = L"!`~"sv,
		extension                    = L"!`"sv,
		short_list                   = L"!&~"sv,
		list                         = L"!&"sv,
		list_file                    = L"!@"sv,
		short_list_file              = L"!$"sv,
		short_name_extension         = L"!-!"sv,
		short_name_extension_safe    = L"!+!"sv,
		current_drive                = L"!:"sv,
		path                         = L"!\\"sv,
		short_path                   = L"!/"sv,
		real_path                    = L"!=\\"sv,
		real_short_path              = L"!=/"sv,
		description                  = L"!?!"sv,
		input                        = L"!?"sv,
		name                         = L"!"sv;

	class skip
	{
	public:
		skip(string_view const Str, string_view const Test) :
			m_Tail(starts_with(Str, Test) ? Str.substr(Test.size()) : string_view{})
		{
		}

		explicit operator bool() const { return m_Tail.data() != nullptr; }
		operator string_view() const { assert(*this); return m_Tail; }

	private:
		string_view m_Tail;
	};
}

struct subst_strings
{
	struct
	{
		string_view
			All,
			Sub;

		auto prefix() const
		{
			// 1 for bracket
			return All.substr(0, Sub.data() - All.data() - 1);
		}

		auto suffix() const
		{
			// 2 for brackets
			return All.substr(Sub.data() - All.data() - 1 + Sub.size() + 2);
		}
	}
	Title,
	Text;
};

struct brackets
{
	int BracketsCount;
	bool Bracket;
	const wchar_t* BeginBracket;
	const wchar_t* EndBracket;

	string_view str() const
	{
		if (!Bracket)
			return {};

		return { BeginBracket + 1, static_cast<size_t>(EndBracket - BeginBracket - 1) };
	}
};

static int ProcessBrackets(string_view const Str, wchar_t const EndMark, brackets& Brackets)
{
	for (auto Iterator = Str.begin(); Iterator != Str.end(); ++Iterator)
	{
		if (*Iterator == L'(')
		{
			if (!Brackets.Bracket)
			{
				Brackets.Bracket = true;
				Brackets.BeginBracket = &*Iterator;
			}

			++Brackets.BracketsCount;
		}
		else if (*Iterator == L')')
		{
			if (!Brackets.BracketsCount)
				return 0;

			--Brackets.BracketsCount;

			if (!Brackets.BracketsCount)
			{
				if (!Brackets.EndBracket)
					Brackets.EndBracket = &*Iterator;
			}
		}
		else if (*Iterator == EndMark && !!Brackets.BeginBracket == !!Brackets.EndBracket)
		{
			if (Brackets.BracketsCount)
				return 0;

			return Iterator - Str.begin() + 1;
		}
	}

	return 0;
};

static size_t SkipInputToken(string_view const Str, subst_strings* const Strings = nullptr)
{
	const auto cTail = tokens::skip(Str, tokens::input);

	if (!cTail)
		return 0;

	string_view Tail(cTail);

	auto Range = Tail;

	brackets TitleBrackets{};
	const auto TitleSize = ProcessBrackets(Range, L'?', TitleBrackets);
	if (!TitleSize)
		return 0;

	Range.remove_prefix(TitleSize);

	brackets TextBrackets{};
	const auto TextSize = ProcessBrackets(Range, L'!', TextBrackets);
	if (!TextSize)
		return 0;

	Range.remove_prefix(TextSize);

	if (Strings)
	{
		Strings->Title.All = Tail.substr(0, TitleSize - 1);
		Strings->Text.All = Tail.substr(TitleSize, TextSize - 1);
		Strings->Title.Sub = TitleBrackets.str();
		Strings->Text.Sub = TextBrackets.str();
	}

	return tokens::input.size() + TitleSize + TextSize;
}

static string_view ProcessMetasymbol(string_view const CurStr, subst_data& SubstData, string& Out)
{
	const auto& append_with_escape = [EscapeAmpersands = SubstData.EscapeAmpersands](string& Destination, string_view const Str)
	{
		if (EscapeAmpersands && contains(Str, L"&"sv))
		{
			string Escaped(Str);
			replace(Escaped, L"&"sv, L"&&"sv);
			append(Destination, Escaped);
		}
		else
		{
			append(Destination, Str);
		}
	};

	if (const auto Tail = tokens::skip(CurStr, tokens::passive_panel))
	{
		SubstData.PassivePanel = true;
		return Tail;
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::active_panel))
	{
		SubstData.PassivePanel = false;
		return Tail;
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::exclamation))
	{
		if (!starts_with(Tail, L'?'))
		{
			Out.push_back(L'!');
			return Tail;
		}
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::name_extension))
	{
		if (!starts_with(Tail, L'?'))
		{
			append_with_escape(Out, SubstData.Default().Normal.Name);
			return Tail;
		}
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::short_name))
	{
		append_with_escape(Out, SubstData.Default().Short.NameOnly);
		return Tail;
	}

	const auto& GetExtension = [](string_view const Name)
	{
		const auto Extension = PointToExt(Name);
		return Extension.empty()? Extension : Extension.substr(1);
	};

	if (const auto Tail = tokens::skip(CurStr, tokens::short_extension))
	{
		append_with_escape(Out, GetExtension(SubstData.Default().Short.Name));
		return Tail;
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::extension))
	{
		append_with_escape(Out, GetExtension(SubstData.Default().Normal.Name));
		return Tail;
	}

	const auto CollectNames = [&SubstData, &append_with_escape](string& Str, auto const Selector)
	{
		append_with_escape(Str, join(select(SubstData.Default().Panel->enum_selected(), Selector), L" "sv));
	};

	if (const auto Tail = tokens::skip(CurStr, tokens::short_list))
	{
		if (!starts_with(Tail, L'?'))
		{
			CollectNames(Out, &os::fs::find_data::AlternateFileName);
			return Tail;
		}
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::list))
	{
		if (!starts_with(Tail, L'?'))
		{
			CollectNames(Out, [](const os::fs::find_data& Data)
			{
				auto Name = Data.FileName;
				QuoteSpaceOnly(Name);
				return Name;
			});

			return Tail;
		}
	}

	const auto& GetListName = [&Out, &append_with_escape](string_view const Tail, auto& Data, bool Short)
	{
		const auto ExclPos = Tail.find(L'!');
		if (ExclPos == Tail.npos || starts_with(Tail.substr(ExclPos + 1), L'?'))
			return size_t{};

		const auto Modifiers = Tail.substr(0, ExclPos);

		const auto ListName = Short? Data.Normal.ListName : Data.Short.ListName;

		if (ListName)
		{
			if(!ListName->empty() || Data.Panel->MakeListFile(*ListName, Short, Modifiers))
			{
				if (Short)
					*ListName = ConvertNameToShort(*ListName);

				append_with_escape(Out, *ListName);
			}
		}
		else
		{
			append(Out, L'!', Short? L'$' : L'@', Modifiers, L'!');
		}

		return Modifiers.size() + 1;
	};

	if (const auto Tail = tokens::skip(CurStr, tokens::list_file))
	{
		if (const auto Offset = GetListName(Tail, SubstData.Default(), false))
			return string_view(Tail).substr(Offset);
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::short_list_file))
	{
		if (const auto Offset = GetListName(Tail, SubstData.Default(), true))
			return string_view(Tail).substr(Offset);
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::short_name_extension))
	{
		if (!starts_with(Tail, L'?'))
		{
			append_with_escape(Out, SubstData.Default().Short.Name);
			return Tail;
		}
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::short_name_extension_safe))
	{
		if (!starts_with(Tail, L'?'))
		{
			append_with_escape(Out, SubstData.Default().Short.Name);
			SubstData.PreserveLFN = true;
			return Tail;
		}
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::current_drive))
	{
		const auto CurDir =
			IsAbsolutePath(SubstData.This.Normal.Name)?
				SubstData.This.Normal.Name :
				SubstData.PassivePanel?
					SubstData.Another.Panel->GetCurDir() :
					SubstData.CmdDir;

		auto RootDir = GetPathRoot(CurDir);
		DeleteEndSlash(RootDir);
		append_with_escape(Out, RootDir);
		return Tail;
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::description))
	{
		Out += SubstData.Default().GetDescription();
		return Tail;
	}

	const auto& GetPath = [](string_view const Tail, const subst_data& Data, bool Short, bool Real)
	{
		// TODO: paths on plugin panels are ambiguous

		auto CurDir = Data.PassivePanel? Data.Another.Panel->GetCurDir() : Data.CmdDir;

		if (Real)
			CurDir = ConvertNameToReal(CurDir);

		if (Short)
			CurDir = ConvertNameToShort(CurDir);

		AddEndSlash(CurDir);
		return CurDir;
	};

	if (const auto Tail = tokens::skip(CurStr, tokens::path))
	{
		Out += GetPath(Tail, SubstData, false, false);
		return Tail;
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::short_path))
	{
		Out += GetPath(Tail, SubstData, true, false);
		return Tail;
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::real_path))
	{
		Out += GetPath(Tail, SubstData, false, true);
		return Tail;
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::real_short_path))
	{
		Out += GetPath(Tail, SubstData, true, true);
		return Tail;
	}

	// !?<title>?<init>!
	if (const auto Tail = tokens::skip(CurStr, tokens::input))
	{
		auto SkipSize = SkipInputToken(CurStr);
		// if bad format string skip 1 char
		if (!SkipSize)
			SkipSize = 1;

		Out.append(CurStr.data(), SkipSize);
		return CurStr.substr(SkipSize);
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::name))
	{
		append(Out, PointToName(SubstData.Default().Normal.NameOnly));
		return Tail;
	}

	return CurStr;
}

static string ProcessMetasymbols(string_view Str, subst_data& Data)
{
	string Result;
	Result.reserve(Str.size());

	while (!Str.empty())
	{
		if (Str.front() == L'!')
		{
			Str = ProcessMetasymbol(Str, Data, Result);
		}
		else
		{
			Result.push_back(Str.front());
			Str.remove_prefix(1);
		}
	}

	return Result;
}

static bool InputVariablesDialog(string& strStr, subst_data& SubstData, string_view const DlgTitle)
{
	// TODO: use DialogBuilder

	std::vector<DialogItemEx> DlgData;
	DlgData.reserve(30);

	struct pos_item
	{
		size_t Pos;
		size_t EndPos;
	};
	std::vector<pos_item> Positions;
	Positions.reserve(128);

	{
		DialogItemEx Item;
		Item.Type = DI_DOUBLEBOX;
		Item.X1 = 3;
		Item.Y1 = 1;
		Item.X2 = 72;
		Item.strData = DlgTitle;
		DlgData.emplace_back(Item);
	}

	string_view Range(strStr);

	while (!Range.empty())
	{
		// теперича все не просто
		// придется сразу определить наличие операторных скобок
		// запомнить их позицию

		subst_strings Strings;
		const auto SkipSize = SkipInputToken(Range, &Strings);

		if (!SkipSize)
		{
			Range.remove_prefix(1);
			continue;
		}

		Positions.emplace_back(pos_item{ strStr.size() - Range.size(), strStr.size() - Range.size() + SkipSize - 1 });

		{
			DialogItemEx Item;
			Item.Type = DI_TEXT;
			Item.X1 = 5;
			Item.Y1 = Item.Y2 = DlgData.size() + 1;
			DlgData.emplace_back(Item);
		}

		{
			DialogItemEx Item;
			Item.Type = DI_EDIT;
			Item.X1 = 5;
			Item.X2 = 70;
			Item.Y1 = Item.Y2 = DlgData.size() + 1;
			Item.Flags = DIF_HISTORY | DIF_USELASTHISTORY;
			Item.strHistory = concat(L"UserVar"sv, str((DlgData.size() - 1) / 2));
			DlgData.emplace_back(Item);
		}

		if (!Strings.Title.All.empty())
		{
			// Something between "!?" and '?'
			if (Strings.Title.All[0] == L'$')
			{
				// History between '$' and '$'
				const size_t HistoryBegin = 1;
				const auto HistoryEnd = Strings.Title.All.find(L'$', HistoryBegin);

				if (HistoryEnd != string_view::npos)
				{
					DlgData.back().strHistory.assign(Strings.Title.All.data(), HistoryBegin, HistoryEnd - HistoryBegin);
					const auto HistorySize = HistoryEnd - HistoryBegin + 2;
					Strings.Title.All.remove_prefix(HistorySize);
				}
			}

			if (!Strings.Title.Sub.empty())
			{
				// Something between '(' and ')'
				DlgData[DlgData.size() - 2].strData = os::env::expand(concat(Strings.Title.prefix(), ProcessMetasymbols(Strings.Title.Sub, SubstData), Strings.Title.suffix()));
			}
			else
			{
				DlgData[DlgData.size() - 2].strData = os::env::expand(Strings.Title.All);
			}
		}

		if (!Strings.Text.All.empty())
		{
			// Something between '?' and '!'
			if (!Strings.Text.Sub.empty())
			{
				// Something between '(' and ')'
				DlgData.back().strData = concat(Strings.Text.prefix(), ProcessMetasymbols(Strings.Text.Sub, SubstData), Strings.Text.suffix());
			}
			else
			{
				DlgData.back().strData = Strings.Text.All;
			}
		}

		Range.remove_prefix(SkipSize);
	}

	if (DlgData.size() == 1)
		return true;

	{
		DialogItemEx Item;
		Item.Type = DI_TEXT;
		Item.Flags = DIF_SEPARATOR;
		Item.Y1 = Item.Y2 = DlgData.size() + 1;
		DlgData.emplace_back(Item);
	}

	const auto OkButtonId = static_cast<int>(DlgData.size());

	{
		DialogItemEx Item;
		Item.Type = DI_BUTTON;
		Item.Flags = DIF_DEFAULTBUTTON|DIF_CENTERGROUP;
		Item.Y1 = Item.Y2 = DlgData.size() + 1;
		Item.strData = msg(lng::MOk);
		DlgData.emplace_back(Item);

		Item.strData = msg(lng::MCancel);
		Item.Flags &= ~DIF_DEFAULTBUTTON;
		DlgData.emplace_back(Item);
	}

	// correct Dlg Title
	DlgData[0].Y2 = DlgData.size();

	int ExitCode;
	{
		const auto Dlg = Dialog::create(DlgData);
		Dlg->SetPosition({ -1, -1, 76, static_cast<int>(DlgData.size() + 2) });
		Dlg->SetId(UserMenuUserInputId);
		Dlg->Process();
		ExitCode=Dlg->GetExitCode();
	}

	if (ExitCode != OkButtonId)
	{
		return false;
	}

	string strTmpStr;

	for (size_t n = 0; n != strStr.size(); ++n)
	{
		const auto ItemIterator = std::find_if(CONST_RANGE(Positions, i) { return i.Pos == n; });
		if (ItemIterator != Positions.cend())
		{
			strTmpStr += DlgData[(ItemIterator - Positions.cbegin()) * 2 + 2].strData;
			n = ItemIterator->EndPos;
		}
		else
		{
			strTmpStr.push_back(strStr[n]);
		}
	}

	strStr = os::env::expand(strTmpStr);
	return true;
}

/*
  SubstFileName()
  Преобразование метасимволов ассоциации файлов в реальные значения

*/
bool SubstFileName(
	string &strStr,                  // результирующая строка
	const subst_context& Context,
	list_names* ListNames,
	bool* PreserveLongName,
	bool IgnoreInput,                // true - не исполнять "!?<title>?<init>!"
	string_view const DlgTitle,
	bool const EscapeAmpersands
)
{
	const auto& Name = Context.Name;
	const auto& ShortName = Context.ShortName;
	const auto& CmdLineDir = Context.Path;

	if (PreserveLongName)
		*PreserveLongName = false;

	/* $ 19.06.2001 SVS
	  ВНИМАНИЕ! Для альтернативных метасимволов, не основанных на "!",
	  нужно будет либо убрать эту проверку либо изменить условие (последнее
	  предпочтительнее!)
	*/
	if (!contains(strStr, L'!'))
		return true;

	subst_data SubstData;
	SubstData.This.Normal.Name = Name;
	SubstData.This.Short.Name = ShortName;
	if (ListNames)
	{
		SubstData.This.Normal.ListName = &ListNames->This.Name;
		SubstData.This.Short.ListName = &ListNames->This.ShortName;
		SubstData.Another.Normal.ListName = &ListNames->Another.Name;
		SubstData.Another.Short.ListName = &ListNames->Another.ShortName;
	}

	SubstData.CmdDir = CmdLineDir.empty()? Global->CtrlObject->CmdLine()->GetCurDir() : CmdLineDir;

	const auto& GetNameOnly = [](string_view Str)
	{
		Str.remove_suffix(PointToExt(Str).size());
		return Str;
	};

	// Предварительно получим некоторые "константы" :-)
	SubstData.This.Normal.NameOnly = GetNameOnly(Name);
	SubstData.This.Short.NameOnly = GetNameOnly(ShortName);

	SubstData.This.Panel = Global->CtrlObject->Cp()->ActivePanel();
	SubstData.Another.Panel = Global->CtrlObject->Cp()->PassivePanel();

	string AnotherName, AnotherShortName;
	SubstData.Another.Panel->GetCurName(AnotherName, AnotherShortName);
	SubstData.Another.Normal.Name = AnotherName;
	SubstData.Another.Short.Name = AnotherShortName;

	SubstData.Another.Normal.NameOnly = GetNameOnly(SubstData.Another.Normal.Name);
	SubstData.Another.Short.NameOnly = GetNameOnly(SubstData.Another.Short.Name);

	SubstData.PreserveLFN = false;
	SubstData.PassivePanel = false; // первоначально речь идет про активную панель!
	SubstData.EscapeAmpersands = EscapeAmpersands;

	strStr = ProcessMetasymbols(strStr, SubstData);

	const auto Result = IgnoreInput || InputVariablesDialog(strStr, SubstData, DlgTitle.empty()? DlgTitle : os::env::expand(DlgTitle));

	if (PreserveLongName)
		*PreserveLongName = SubstData.PreserveLFN;

	return Result;
}
