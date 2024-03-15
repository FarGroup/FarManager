/*
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "fnparce.hpp"

// Internal:
#include "panel.hpp"
#include "ctrlobj.hpp"
#include "flink.hpp"
#include "cmdline.hpp"
#include "filepanels.hpp"
#include "dialog.hpp"
#include "uuids.far.dialogs.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "mix.hpp"
#include "lang.hpp"
#include "cvtname.hpp"
#include "global.hpp"
#include "filelist.hpp"
#include "delete.hpp"
#include "message.hpp"
#include "eol.hpp"
#include "interf.hpp"
#include "datetime.hpp"
#include "log.hpp"

// Platform:
#include "platform.hpp"
#include "platform.env.hpp"
#include "platform.fs.hpp"

// Common:
#include "common/string_utils.hpp"
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

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

struct subst_data
{
	struct
	{
		struct
		{
			string_view Name;
			string_view NameOnly;
		}
		Normal, Short;
		panel_ptr Panel;

		string_view GetDescription() const
		{
			if (!m_Description.has_value())
			{
				if (const auto FList = dynamic_cast<FileList*>(Panel.get()))
				{
					FList->ReadDiz();
					// BUGBUG size
					m_Description = FList->GetDescription(Normal.Name, Short.Name, 0);
				}
				else
				{
					m_Description = L""sv;
				}
			}

			return *m_Description;
		}

	private:
		mutable std::optional<string_view> m_Description;
	}
	This, Another;

	auto& Default() { return PassivePanel? Another : This; }
	const auto& Default() const { return PassivePanel? Another : This; }

	delayed_deleter* ListNames{};
	string CmdDir;
	bool PreserveLFN{};
	bool PassivePanel{};
	bool EscapeAmpersands{};

	unordered_string_map<string>* Variables{};
};


// Str=if exist !#!\!^!.! far:edit < diff -c -p "!#!\!^!.!" !\!.!

namespace tokens
{
	const auto
		passive_panel                = L"!#"sv,
		active_panel                 = L"!^"sv,
		left_panel                   = L"!["sv,
		right_panel                  = L"!]"sv,
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
			m_Tail(Str.starts_with(Test)? Str.substr(Test.size()) : string_view{})
		{
		}

		explicit operator bool() const { return m_Tail.data() != nullptr; }
		explicit(false) operator string_view() const { assert(*this); return m_Tail; }

	private:
		string_view m_Tail;
	};
}

struct subst_strings
{
	struct item
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
	const wchar_t* BeginBracket{};
	const wchar_t* EndBracket{};

	string_view str() const
	{
		if (!BeginBracket || !EndBracket)
			return {};

		return { BeginBracket + 1, static_cast<size_t>(EndBracket - BeginBracket - 1) };
	}
};

static size_t ProcessBrackets(string_view const Str, wchar_t const EndMark, brackets& Brackets)
{
	int BracketsCount = 0;

	for (auto Iterator = Str.begin(); Iterator != Str.end(); ++Iterator)
	{
		if (!Brackets.EndBracket)
		{
			if (*Iterator == L'(')
			{
				if (!Brackets.BeginBracket)
					Brackets.BeginBracket = std::to_address(Iterator);

				++BracketsCount;
				continue;
			}

			if (*Iterator == L')')
			{
				if (!BracketsCount)
					continue;

				--BracketsCount;

				if (BracketsCount)
					continue;

				Brackets.EndBracket = std::to_address(Iterator);
				continue;
			}
		}

		if (!BracketsCount && *Iterator == EndMark)
		{
			return Iterator - Str.begin() + 1;
		}
	}

	return 0;
}

static size_t SkipInputToken(string_view const Str, subst_strings* const Strings = nullptr)
{
	const auto cTail = tokens::skip(Str, tokens::input);

	if (!cTail)
		return 0;

	const string_view Tail(cTail);

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

static void MakeListFile(panel_ptr const& Panel, string& ListFileName, bool const ShortNames, string_view const Modifers)
{
	auto CodePage = encoding::codepage::oem();
	bool UseFullPaths{}, QuotePaths{}, UseForwardSlash{};

	for (const auto& i: Modifers)
	{
		switch (i)
		{
		case L'A':
			CodePage = encoding::codepage::ansi();
			break;

		case L'U':
			CodePage = CP_UTF8;
			break;

		case L'W':
			CodePage = CP_UTF16LE;
			break;

		case L'F':
			UseFullPaths = true;
			break;

		case L'Q':
			QuotePaths = true;
			break;

		case L'S':
			UseForwardSlash = true;
			break;
		}
	}

	const auto transform = [&](string& strFileName)
	{
		if (UseFullPaths && PointToName(strFileName).size() == strFileName.size())
		{
			const auto& CurDir = Panel->GetCurDir();
			strFileName = path::join(ShortNames? ConvertNameToShort(CurDir) : CurDir, strFileName); //BUGBUG?
		}

		if (QuotePaths)
			inplace::quote(strFileName);

		if (UseForwardSlash)
			ReplaceBackslashToSlash(strFileName);
	};

	ListFileName = MakeTemp();

	const os::fs::file ListFile(ListFileName, GENERIC_WRITE, os::fs::file_share_read, nullptr, CREATE_ALWAYS);
	if (!ListFile)
		throw far_exception(msg(lng::MCannotCreateListTemp));

	SCOPE_FAIL
	{
		if (!os::fs::delete_file(ListFileName)) // BUGBUG
		{
			LOGWARNING(L"delete_file({}): {}"sv, ListFileName, os::last_error());
		}
	};

	os::fs::filebuf StreamBuffer(ListFile, std::ios::out);
	std::ostream Stream(&StreamBuffer);
	Stream.exceptions(Stream.badbit | Stream.failbit);
	encoding::writer Writer(Stream, CodePage, false);
	const auto Eol = eol::system.str();

	for (const auto& i: Panel->enum_selected())
	{
		auto Name = ShortNames? i.AlternateFileName() : i.FileName;

		transform(Name);

		Writer.write(Name, Eol);
	}

	Stream.flush();
}

static string_view ProcessMetasymbol(string_view const CurStr, subst_data& SubstData, string& Out)
{
	const auto append_with_escape = [EscapeAmpersands = SubstData.EscapeAmpersands](string& Destination, string_view const Str)
	{
		append(Destination, EscapeAmpersands && contains(Str, L"&"sv)? escape_ampersands(Str) : Str);
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

	if (const auto Tail = tokens::skip(CurStr, tokens::left_panel))
	{
		SubstData.PassivePanel = SubstData.This.Panel->Parent()->IsRightActive();
		return Tail;
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::right_panel))
	{
		SubstData.PassivePanel = SubstData.This.Panel->Parent()->IsLeftActive();
		return Tail;
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::exclamation))
	{
		if (!string_view(Tail).starts_with(L'?'))
		{
			Out.push_back(L'!');
			return Tail;
		}
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::name_extension))
	{
		if (!string_view(Tail).starts_with(L'?'))
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

	const auto GetExtension = [](string_view const Name)
	{
		const auto Extension = name_ext(Name).second;
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

	const auto CollectNames = [&SubstData, &append_with_escape](string_view const Tail, string& Str, auto const Selector)
	{
		const auto ExplicitQuote = Tail.starts_with(L'Q');
		const auto ExplicitNoQuote = Tail.starts_with(L'q');

		const auto Quote = ExplicitQuote || !ExplicitNoQuote;

		append_with_escape(
			Str,
			join(
				L" "sv,
				SubstData.Default().Panel->enum_selected() | std::views::transform(
					[&](os::fs::find_data const& i)
					{
						const auto Data = std::invoke(Selector, i);
						return Quote? quote(Data) : quote_space(Data);
					})
			)
		);

		return Tail.substr(ExplicitQuote || ExplicitNoQuote? 1 : 0);
	};

	if (const auto Tail = tokens::skip(CurStr, tokens::short_list))
	{
		if (!string_view(Tail).starts_with(L'?'))
		{
			return CollectNames(Tail, Out, &os::fs::find_data::AlternateFileName);
		}
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::list))
	{
		if (!string_view(Tail).starts_with(L'?'))
		{
			return CollectNames(Tail, Out, &os::fs::find_data::FileName);
		}
	}

	const auto GetListName = [&Out, &append_with_escape](string_view const Tail, subst_data& Data, bool Short)
	{
		const auto ExclPos = Tail.find(L'!');
		if (ExclPos == Tail.npos || Tail.substr(ExclPos + 1).starts_with(L'?'))
			return 0uz;

		const auto Modifiers = Tail.substr(0, ExclPos);

		if (Data.ListNames)
		{
			string Str;
			MakeListFile(Data.Default().Panel, Str, Short, Modifiers);

			if (Short)
				Str = ConvertNameToShort(Str);

			append_with_escape(Out, Str);
			Data.ListNames->add(std::move(Str));
		}
		else
		{
			append(Out, L'!', Short? L'$' : L'@', Modifiers, L'!');
		}

		return Modifiers.size() + 1;
	};

	if (const auto Tail = tokens::skip(CurStr, tokens::list_file))
	{
		if (const auto Offset = GetListName(Tail, SubstData, false))
			return string_view(Tail).substr(Offset);
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::short_list_file))
	{
		if (const auto Offset = GetListName(Tail, SubstData, true))
			return string_view(Tail).substr(Offset);
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::short_name_extension))
	{
		if (!string_view(Tail).starts_with(L'?'))
		{
			append_with_escape(Out, SubstData.Default().Short.Name);
			return Tail;
		}
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::short_name_extension_safe))
	{
		if (!string_view(Tail).starts_with(L'?'))
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

	const auto GetPath = [](string_view const Tail, const subst_data& Data, bool Short, bool Real)
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
	if (tokens::skip(CurStr, tokens::input))
	{
		auto SkipSize = SkipInputToken(CurStr);
		// if bad format string skip 1 char
		if (!SkipSize)
			SkipSize = 1;

		Out += CurStr.substr(0, SkipSize);
		return CurStr.substr(SkipSize);
	}

	if (const auto Tail = tokens::skip(CurStr, tokens::name))
	{
		append(Out, PointToName(SubstData.Default().Normal.NameOnly));
		return Tail;
	}

	return CurStr;
}

static string_view ProcessVariable(string_view const CurStr, const subst_data& SubstData, string& Out)
{
	const auto Str = CurStr.substr(1);

	const auto Iterator = std::ranges::find_if(*SubstData.Variables, [&](std::pair<string, string> const& i)
	{
		return starts_with_icase(Str, i.first);
	});

	if (Iterator == SubstData.Variables->cend())
	{
		Out += CurStr.front();
		return Str;
	}

	Out += Iterator->second;
	return Str.substr(Iterator->first.size());
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
		else if (Str.front() == L'%')
		{
			Str = ProcessVariable(Str, Data, Result);
		}
		else
		{
			Result.push_back(Str.front());
			Str.remove_prefix(1);
		}
	}

	return Result;
}

static string process_subexpression(const subst_strings::item& Item, subst_data& SubstData)
{
	if (Item.Sub.empty())
		return string(Item.All);

	// Something between '(' and ')'
	const auto Processed = ProcessMetasymbols(Item.Sub, SubstData);
	return Processed == Item.Sub?
		string(Item.All) :
		concat(Item.prefix(), Processed, Item.suffix());
}

static bool InputVariablesDialog(string& strStr, subst_data& SubstData, string_view const DlgTitle)
{
	// TODO: use DialogBuilder

	// TODO: Dynamic?
	const int DlgWidth = 76;

	constexpr auto HistoryAndVariablePrefix = L"UserVar"sv;

	const auto GenerateHistoryName = [&](size_t const Index)
	{
		return far::format(L"{}{}"sv, HistoryAndVariablePrefix, Index);
	};

	constexpr auto ExpectedTokensCount = 64;

	std::vector<DialogItemEx> DlgData;
	DlgData.reserve(ExpectedTokensCount * 2 + 4); // + Box, separator, 2 buttons

	struct pos_item
	{
		size_t Pos;
		size_t EndPos;
	};
	std::vector<pos_item> Positions;
	Positions.reserve(ExpectedTokensCount);

	{
		DialogItemEx Item;
		Item.Type = DI_DOUBLEBOX;
		Item.X1 = 3;
		Item.Y1 = 1;
		Item.X2 = DlgWidth - 4;
		Item.strData = DlgTitle;
		DlgData.emplace_back(std::move(Item));
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
			Item.X2 = DlgWidth - 6;
			DlgData.emplace_back(std::move(Item));
		}

		{
			DialogItemEx Item;
			Item.Type = DI_EDIT;
			Item.X1 = 5;
			Item.X2 = DlgWidth - 6;
			Item.Y1 = Item.Y2 = DlgData.size() + 1;
			Item.Flags = DIF_HISTORY | DIF_USELASTHISTORY;
			Item.strHistory = GenerateHistoryName((DlgData.size() - 1) / 2);
			DlgData.emplace_back(std::move(Item));
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
					DlgData.back().strHistory = Strings.Title.All.substr(HistoryBegin, HistoryEnd - HistoryBegin);
					const auto HistorySize = HistoryEnd - HistoryBegin + 2;
					Strings.Title.All.remove_prefix(HistorySize);
				}
			}

			auto& LatelItem = DlgData[DlgData.size() - 2];

			LatelItem.strData = os::env::expand(process_subexpression(Strings.Title, SubstData));

			inplace::truncate_right(LatelItem.strData, LatelItem.X2 - LatelItem.X1 + 1);
		}

		if (!Strings.Text.All.empty())
		{
			// Something between '?' and '!'
			DlgData.back().strData = process_subexpression(Strings.Text, SubstData);
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
		DlgData.emplace_back(std::move(Item));
	}

	const auto OkButtonId = static_cast<int>(DlgData.size());

	{
		DialogItemEx Item;
		Item.Type = DI_BUTTON;
		Item.Flags = DIF_DEFAULTBUTTON|DIF_CENTERGROUP;
		Item.Y1 = Item.Y2 = DlgData.size() + 1;
		Item.strData = msg(lng::MOk);
		DlgData.emplace_back(std::move(Item));
	}

	{
		DialogItemEx Item;
		Item.Type = DI_BUTTON;
		Item.Flags = DIF_CENTERGROUP;
		Item.Y1 = Item.Y2 = DlgData.size();
		Item.strData = msg(lng::MCancel);
		DlgData.emplace_back(std::move(Item));
	}

	// correct Dlg Title
	DlgData[0].Y2 = DlgData.size();

	int ExitCode;
	{
		const auto Dlg = Dialog::create(DlgData);
		Dlg->SetPosition({ -1, -1, DlgWidth, static_cast<int>(DlgData.size() + 2) });
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
		if (const auto ItemIterator = std::ranges::find(Positions, n, &pos_item::Pos); ItemIterator != Positions.cend())
		{
			strTmpStr += DlgData[(ItemIterator - Positions.cbegin()) * 2 + 2].strData;
			n = ItemIterator->EndPos;
		}
		else
		{
			strTmpStr.push_back(strStr[n]);
		}
	}

	for (const auto& i: DlgData)
	{
		if (i.Type != DI_EDIT)
			continue;

		const auto Index = (&i - DlgData.data() - 1) / 2;
		const auto VariableName = far::format(L"%{}{}"sv, HistoryAndVariablePrefix, Index + 1);
		replace_icase(strTmpStr, VariableName, i.strData);

		if (!i.strHistory.empty() && i.strHistory != GenerateHistoryName(Index))
		{
			replace_icase(strTmpStr, L'%' + i.strHistory, i.strData);
			SubstData.Variables->emplace(i.strHistory, i.strData);
		}
	}

	strStr = os::env::expand(strTmpStr);
	return true;
}

static auto get_delayed_deleter()
{
	static std::optional<delayed_deleter> s_DelayedDeleter(std::in_place, false);

	// Housekeeping
	static time_check TimeCheck(time_check::mode::delayed, 5min);
	if (TimeCheck)
	{
		s_DelayedDeleter.reset();
		s_DelayedDeleter.emplace(false);
	}

	return &*s_DelayedDeleter;
}

/*
  SubstFileName()
  Преобразование метасимволов ассоциации файлов в реальные значения

*/
bool SubstFileName(
	string &Str,                  // результирующая строка
	const subst_context& Context,
	bool* PreserveLongName,
	bool IgnoreInputAndLists,                // true - не исполнять "!?<title>?<init>!"
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
	if (Str.find_first_of(L"!%"sv) == Str.npos)
		return true;

	subst_data SubstData;
	SubstData.This.Normal.Name = Name;
	SubstData.This.Short.Name = ShortName;

	if (!IgnoreInputAndLists)
		SubstData.ListNames = get_delayed_deleter();

	SubstData.CmdDir = CmdLineDir.empty()? Global->CtrlObject->CmdLine()->GetCurDir() : CmdLineDir;

	// Предварительно получим некоторые "константы" :-)
	SubstData.This.Normal.NameOnly = name_ext(Name).first;
	SubstData.This.Short.NameOnly = name_ext(ShortName).first;

	SubstData.This.Panel = Global->CtrlObject->Cp()->ActivePanel();
	SubstData.Another.Panel = Global->CtrlObject->Cp()->PassivePanel();

	string AnotherName, AnotherShortName;
	SubstData.Another.Panel->GetCurName(AnotherName, AnotherShortName);
	SubstData.Another.Normal.Name = AnotherName;
	SubstData.Another.Short.Name = AnotherShortName;

	SubstData.Another.Normal.NameOnly = name_ext(SubstData.Another.Normal.Name).first;
	SubstData.Another.Short.NameOnly = name_ext(SubstData.Another.Short.Name).first;

	SubstData.PreserveLFN = false;
	SubstData.PassivePanel = false; // первоначально речь идет про активную панель!
	SubstData.EscapeAmpersands = EscapeAmpersands;

	SubstData.Variables = &Context.Variables;

	try
	{
		Str = ProcessMetasymbols(Str, SubstData);
	}
	catch (far_exception const& e)
	{
		Message(MSG_WARNING, e,
			msg(lng::MError),
			{
			},
			{ lng::MOk });
		return false;
	}

	const auto Result = IgnoreInputAndLists || InputVariablesDialog(Str, SubstData, DlgTitle.empty()? DlgTitle : os::env::expand(DlgTitle));

	if (PreserveLongName)
		*PreserveLongName = SubstData.PreserveLFN;

	return Result;
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("ProcessBrackets")
{
	static const struct
	{
		string_view Src;
		size_t ExpectedSize;
		std::pair<intptr_t, intptr_t> ExpectedBracketPositions;
	}
	Tests[]
	{
		{ {},                       0, { -1, -1 } },
		{ L"!"sv,                   1, { -1, -1 } },
		{ L"Meow!"sv,               5, { -1, -1 } },
		{ L"()!"sv,                 3, {  0,  1 } },
		{ L"()()!"sv,               5, {  0,  1 } },
		{ L"(())!"sv,               5, {  0,  3 } },
		{ L"((boo))!"sv,            8, {  0,  6 } },
		{ L"((!.!))!"sv,            8, {  0,  6 } },
		{ L"(!.!)(text)!"sv,       12, {  0,  4 } },
		{ L"(text)(!.!)!"sv,        8, {  0,  5 } },
	};

	for (const auto& i: Tests)
	{
		brackets Brackets;
		const auto Size = ProcessBrackets(i.Src, L'!', Brackets);
		const std::pair BracketPositions
		{
			Brackets.BeginBracket? Brackets.BeginBracket - i.Src.data() : -1,
			Brackets.EndBracket? Brackets.EndBracket - i.Src.data() : -1,
		};

		REQUIRE(i.ExpectedSize == Size);
		REQUIRE(i.ExpectedBracketPositions == BracketPositions);
	}
}
#endif
