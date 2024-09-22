/*
language.cpp

Работа с lng файлами
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
#include "language.hpp"

// Internal:
#include "lang.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "manager.hpp"
#include "message.hpp"
#include "config.hpp"
#include "filestr.hpp"
#include "interf.hpp"
#include "string_utils.hpp"
#include "pathmix.hpp"
#include "exception.hpp"
#include "global.hpp"
#include "log.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/function_traits.hpp"
#include "common/scope_exit.hpp"
#include "common/string_utils.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static const auto LangFileMask = L"*.lng"sv;

static lang_file open_impl(string_view const FileName)
{
	lang_file Result;
	if (!Result.File.Open(FileName, FILE_READ_DATA, os::fs::file_share_read, nullptr, OPEN_EXISTING))
		return {};

	Result.Codepage = GetFileCodepage(Result.File, encoding::codepage::oem(), nullptr, false);
	Result.TryUtf8 = !IsUtfCodePage(Result.Codepage);

	string Language;
	if (!GetLangParam(Result, L"Language"sv, Language))
		return {};

	std::tie(Result.Name, Result.Description) = split(Language, L',');

	return Result;
}

lang_file OpenLangFile(string_view const Path, string_view const Mask, string_view const Language)
{
	lang_file CurrentFile, EnglishFile;

	for (const auto& FindData: os::fs::enum_files(path::join(Path, Mask)))
	{
		if (!os::fs::is_file(FindData))
			continue;

		CurrentFile = open_impl(path::join(Path, FindData.FileName));
		if (!CurrentFile)
			continue;

		if (equal_icase(CurrentFile.Name, Language))
			return CurrentFile;

		if (equal_icase(CurrentFile.Name, L"English"sv))
		{
			EnglishFile = std::move(CurrentFile);
		}
	}

	if (EnglishFile)
		return EnglishFile;

	return CurrentFile;
}


bool GetLangParam(lang_file& LangFile, string_view const ParamName, string& Param)
{
	const auto strFullParamName = concat(L'.', ParamName);
	const auto CurFilePos = LangFile.File.GetPointer();
	SCOPE_EXIT{ LangFile.File.SetPointer(CurFilePos, nullptr, FILE_BEGIN); };

	os::fs::filebuf StreamBuffer(LangFile.File, std::ios::in);
	std::istream Stream(&StreamBuffer);
	Stream.exceptions(Stream.badbit | Stream.failbit);

	for (const auto& i: enum_lines(Stream, LangFile.Codepage, &LangFile.TryUtf8))
	{
		if (starts_with_icase(i.Str, strFullParamName))
		{
			const auto EqPos = i.Str.find(L'=', strFullParamName.size());
			if (EqPos == string::npos)
				continue;

			Param = i.Str.substr(EqPos + 1);
			inplace::trim_right(Param);
			return true;
		}

		if (i.Str.starts_with(L'"'))
		{
			// '"' indicates some meaningful string.
			// Parameters can be only in the header, no point to go deeper
			return false;
		}
	}

	return false;
}

static bool SelectLanguage(bool HelpLanguage, string& Dest)
{
	const auto Title = HelpLanguage? lng::MHelpLangTitle : lng::MLangTitle;
	const auto Mask = HelpLanguage? Global->HelpFileMask : LangFileMask;

	const auto LangMenu = VMenu2::create(msg(Title), {}, ScrY - 4);
	LangMenu->SetMenuFlags(VMENU_WRAPMODE);
	LangMenu->SetPosition({ ScrX / 2 - 8 + 5 * HelpLanguage, ScrY / 2 - 4 + 2 * HelpLanguage, 0, 0 });

	for (const auto& FindData: os::fs::enum_files(path::join(Global->g_strFarPath, Mask)))
	{
		if (!os::fs::is_file(FindData))
			continue;

		auto LangFile = open_impl(path::join(Global->g_strFarPath, FindData.FileName));
		if (!LangFile)
			continue;

		string strEntryName;
		if (HelpLanguage && (
			GetLangParam(LangFile, L"PluginContents"sv, strEntryName) ||
			GetLangParam(LangFile, L"DocumentContents"sv, strEntryName)
		))
			continue;

		MenuItemEx LangMenuItem(!LangFile.Description.empty()? LangFile.Description: LangFile.Name);

		// No duplicate languages
		if (LangMenu->FindItem(0, LangMenuItem.Name, LIFIND_EXACTMATCH) != -1)
			continue;

		LangMenuItem.SetSelect(equal_icase(Dest, LangFile.Name));
		LangMenuItem.ComplexUserData = LangFile.Name;
		LangMenu->AddItem(LangMenuItem);
	}

	LangMenu->AssignHighlights();
	LangMenu->Run();

	if (LangMenu->GetExitCode()<0)
		return false;

	Dest = *LangMenu->GetComplexUserDataPtr<string>();
	return true;
}

bool SelectInterfaceLanguage(string& Dest) {return SelectLanguage(false, Dest);}
bool SelectHelpLanguage(string& Dest) {return SelectLanguage(true, Dest);}

static wchar_t extract(string_view::const_iterator& Iterator, string_view::const_iterator const End)
{
	switch (*Iterator)
	{
	case L'\\':
		if (++Iterator == End)
			return L'\\';

		switch (*Iterator)
		{
		case L'\\': return L'\\';
		case L'"':  return L'"';
		case L'n':  return L'\n';
		case L'r':  return L'\r';
		case L'b':  return L'\b';
		case L't':  return L'\t';

		default:
			--Iterator;
			return L'\\';
		}

	case L'"':
		if (++Iterator != End && *Iterator != L'"')
			--Iterator;
		return L'"';

	default:
		return *Iterator;
	}
}

static string ConvertString(const string_view Src)
{
	const auto SpecialPos = Src.find_first_of(L"\\\""sv);
	if (SpecialPos == Src.npos)
		return string(Src);

	string Result;
	Result.reserve(Src.size());

	Result = Src.substr(0, SpecialPos);

	for (auto i = Src.begin() + SpecialPos, End = Src.end(); i != End; ++i)
	{
		Result.push_back(extract(i, End));
		if (i == End)
			break;
	}

	return Result;
}

enum class lng_line_type
{
	none,
	label,
	text,
	both
};

struct lng_line
{
	lng_line_type Type;
	string_view
		Label,
		Text;
};

static lng_line parse_lng_line(const string_view str, bool ParseLabels)
{
	//-- "Text"
	if (str.starts_with(L'"'))
	{
		return { lng_line_type::text, {}, str.substr(1, str.size() - (str.ends_with(L'"')? 2 : 1)) };
	}

	//-- //[Label]
	if (ParseLabels)
	{
		const auto Prefix = L"//["sv, Suffix = L"]"sv;
		if (str.starts_with(Prefix) && str.ends_with(Suffix))
		{
			return { lng_line_type::label, str.substr(Prefix.size(), str.size() - Prefix.size() - Suffix.size()) };
		}
	}

	//-- MLabel="Text"
	if (ParseLabels && str.ends_with(L'"') && std::iswalpha(str.front()))
	{
		auto [Name, Value] = split(str);
		inplace::trim(Name);
		inplace::trim(Value);

		if (!Name.empty() && Value.size() > 1 && Value.starts_with(L'"'))
		{
			return { lng_line_type::both, Name, Value.substr(1, Value.size() - 2) };
		}
	}

	return { lng_line_type::none };
}

class language_data final: public i_language_data
{
public:
	std::unique_ptr<i_language_data> create() override { return std::make_unique<language_data>(); }

	void reserve(size_t Size) override { return m_Messages.reserve(Size); }
	void add(string&& Str) override { m_Messages.emplace_back(std::move(Str)); }
	void set_at(size_t Index, string&& Str) override { m_Messages[Index] = std::move(Str); }
	size_t size() const override { return m_Messages.size(); }

	const string& at(size_t Index) const { return m_Messages[Index]; }

private:
	std::vector<string> m_Messages;
};

static void LoadCustomStrings(string_view const FileName, unordered_string_map<string>& Strings)
{
	const os::fs::file CustomFile(FileName, FILE_READ_DATA, os::fs::file_share_read, nullptr, OPEN_EXISTING);
	if (!CustomFile)
		return;

	const auto CustomFileCodepage = GetFileCodepage(CustomFile, encoding::codepage::oem(), nullptr, false);
	auto TryUtf8 = !IsUtfCodePage(CustomFileCodepage);

	string SavedLabel;

	os::fs::filebuf StreamBuffer(CustomFile, std::ios::in);
	std::istream Stream(&StreamBuffer);
	Stream.exceptions(Stream.badbit | Stream.failbit);

	const auto LastSize = Strings.size();

	for (const auto& i: enum_lines(Stream, CustomFileCodepage, &TryUtf8))
	{
		switch (const auto Line = parse_lng_line(trim(i.Str), true); Line.Type)
		{
		case lng_line_type::label:
			SavedLabel = Line.Label;
			break;

		case lng_line_type::text:
			Strings.emplace(std::move(SavedLabel), ConvertString(Line.Text));
			SavedLabel.clear();
			break;

		case lng_line_type::both:
			Strings.emplace(Line.Label, ConvertString(Line.Text));
			break;

		default:
			break;
		}
	}

	LOGINFO(L"Loaded {} strings from {}"sv, Strings.size() - LastSize, FileName);
}

void language::load(string_view const Path, string_view const Language, int CountNeed)
{
	SCOPED_ACTION(os::last_error_guard);

	auto Data = m_Data->create();

	auto LangFile = OpenLangFile(Path, LangFileMask, Language);
	if (!LangFile)
	{
		throw far_known_exception(far::format(L"Cannot find any language files in \"{}\""sv, Path));
	}

	Data->m_FileName = LangFile.File.GetName();

	if (CountNeed != -1)
	{
		Data->reserve(CountNeed);
	}

	// try to load Far<LNG>.lng.custom file(s)
	unordered_string_map<string> CustomStrings;

	const auto CustomLngInSameDir = Data->m_FileName + L".custom"sv;
	const auto CustomLngInProfileDir = path::join(Global->Opt->ProfilePath, ExtractFileName(CustomLngInSameDir));

	// LoadCustomStrings uses map.emplace (does not overwrite existing entires) so the high priority location should come first.
	// If for whatever reason it will use insert_or_assign one day - change the order here.
	LoadCustomStrings(CustomLngInProfileDir, CustomStrings);
	LoadCustomStrings(CustomLngInSameDir, CustomStrings);

	const auto LoadLabels = !CustomStrings.empty();

	string SavedLabel;

	os::fs::filebuf StreamBuffer(LangFile.File, std::ios::in);
	std::istream Stream(&StreamBuffer);
	Stream.exceptions(Stream.badbit | Stream.failbit);

	for (const auto& i: enum_lines(Stream, LangFile.Codepage, &LangFile.TryUtf8))
	{
		switch (auto Line = parse_lng_line(trim(i.Str), LoadLabels); Line.Type)
		{
		case lng_line_type::label:
			SavedLabel = Line.Label;
			break;

		case lng_line_type::text:
			if (LoadLabels)
			{
				const auto Iterator = CustomStrings.find(SavedLabel);
				if (Iterator != CustomStrings.cend())
				{
					Line.Text = Iterator->second;
				}
				SavedLabel.clear();
			}

			Data->add(ConvertString(Line.Text));
			break;

		default:
			break;
		}
	}

	//   Проведем проверку на количество строк в LNG-файлах
	if (CountNeed != -1 && static_cast<size_t>(CountNeed) != Data->size())
	{
		throw far_known_exception(far::format(
			L"Language file \"{}\" is malformed: expected {} items, found {}"sv,
			Data->m_FileName,
			CountNeed,
			Data->size()));
	}

	m_Data = std::move(Data);
}

bool i_language_data::validate(size_t MsgId) const
{
	if (MsgId < size())
		return true;

	if (!Global || !Global->WindowManager || Global->WindowManager->ManagerIsDown())
		return false;

	if (Message(MSG_WARNING,
		msg(lng::MError),
		{
			msg(lng::MBadLanguageFile),
			m_FileName,
			far::vformat(msg(lng::MLanguageStringNotFound), MsgId)
		},
		{ lng::MOk, lng::MQuit }) == message_result::second_button)
	{
		Global->WindowManager->ExitMainLoop(FALSE);
	}

	return false;
}

plugin_language::plugin_language(string_view const Path, string_view const Language):
	language(std::make_unique<language_data>())
{
	load(Path, Language);
}

const wchar_t* plugin_language::Msg(intptr_t Id) const
{
	return m_Data->validate(Id)? static_cast<const language_data&>(*m_Data).at(Id).c_str() : L"";
}


far_language::far_language():
	language(std::make_unique<language_data>())
{
}

bool far_language::is_loaded() const
{
	return static_cast<const language_data&>(*m_Data).size() != 0;
}

const string& far_language::Msg(lng Id) const
{
	return static_cast<const language_data&>(*m_Data).at(static_cast<size_t>(Id));
}


const string& msg(lng Id)
{
	return far_language::instance().Msg(Id);
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("language.parser")
{
	static const struct
	{
		string_view Input;
		lng_line Result;
	}
	Tests[]
	{
		{ L"\"Text\""sv,      { lng_line_type::text,  {},          L"Text"sv, }, },
		{ L"\"Text"sv,        { lng_line_type::text,  {},          L"Text"sv, }, },
		{ L"//[Label]"sv,     { lng_line_type::label, L"Label"sv,  {},        }, },
		{ L"//[Lab"sv,        { lng_line_type::none,  {},          {},        }, },
		{ L"foo = \"bar\""sv, { lng_line_type::both,  L"foo"sv,    L"bar"sv,  }, },
		{ L"foo=\"bar"sv,     { lng_line_type::none,  {},          {},        }, },
		{ L"foo=bar\""sv,     { lng_line_type::none,  {},          {},        }, },
		{ L"foo=bar"sv,       { lng_line_type::none,  {},          {},        }, },
		{ L"foo="sv,          { lng_line_type::none,  {},          {},        }, },
		{ L"foo"sv,           { lng_line_type::none,  {},          {},        }, },
	};

	for (const auto& i: Tests)
	{
		const auto Result = parse_lng_line(i.Input, true);
		REQUIRE(i.Result.Type == Result.Type);
		REQUIRE(i.Result.Label == Result.Label);
		REQUIRE(i.Result.Text == Result.Text);
	}
}

TEST_CASE("language.escape")
{
	static const struct
	{
		string_view Str, Result;
	}
	Tests[]
	{
		{ {},          {},         },
		{ L"y"sv,      L"y"sv,     },
		{ L"\\y"sv,    L"\\y"sv,   },
		{ L"\\"sv,     L"\\"sv,    },
		{ L"\\x"sv,    L"\\x"sv,   },
		{ L"\\r"sv,    L"\r"sv,    },
		{ L"\\n"sv,    L"\n"sv,    },
		{ L"\\t"sv,    L"\t"sv,    },
		{ L"\""sv,     L"\""sv,    },
		{ L"\\\\"sv,   L"\\"sv,    },
		{ L"\\b"sv,    L"\b"sv,    },
	};

	for (const auto& i: Tests)
	{
		const auto Result = ConvertString(i.Str);
		REQUIRE(i.Result == Result);
	}
}
#endif
