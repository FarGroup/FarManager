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

std::tuple<os::fs::file, string, uintptr_t> OpenLangFile(string_view const Path, string_view const Mask, string_view const Language)
{
	FN_RETURN_TYPE(OpenLangFile) CurrentFileData, EnglishFileData;

	for (const auto& FindData: os::fs::enum_files(path::join(Path, Mask)))
	{
		const auto CurrentFileName = path::join(Path, FindData.FileName);

		auto& [CurrentFile, CurrentLngName, CurrentCodepage] = CurrentFileData;

		CurrentFile = os::fs::file(CurrentFileName, FILE_READ_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING);
		if (!CurrentFile)
			continue;

		CurrentCodepage = GetFileCodepage(CurrentFile, encoding::codepage::oem(), nullptr, false);

		if (GetLangParam(CurrentFile, L"Language"sv, CurrentLngName, nullptr, CurrentCodepage) && equal_icase(CurrentLngName, Language))
		{
			return CurrentFileData;
		}

		if (equal_icase(CurrentLngName, L"English"sv))
		{
			EnglishFileData = std::move(CurrentFileData);
		}
	}

	if (std::get<0>(EnglishFileData))
		return EnglishFileData;

	return CurrentFileData;
}


bool GetLangParam(const os::fs::file& LangFile, string_view const ParamName, string& strParam1, string* strParam2, uintptr_t CodePage)
{
	const auto strFullParamName = concat(L'.', ParamName);
	const auto CurFilePos = LangFile.GetPointer();
	SCOPE_EXIT{ LangFile.SetPointer(CurFilePos, nullptr, FILE_BEGIN); };

	os::fs::filebuf StreamBuffer(LangFile, std::ios::in);
	std::istream Stream(&StreamBuffer);
	Stream.exceptions(Stream.badbit | Stream.failbit);

	for (const auto& i: enum_lines(Stream, CodePage))
	{
		if (starts_with_icase(i.Str, strFullParamName))
		{
			const auto EqPos = i.Str.find(L'=');
			if (EqPos == string::npos)
				continue;

			strParam1 = i.Str.substr(EqPos + 1);

			if (strParam2)
				strParam2->clear();

			if (const auto pos = strParam1.find(L','); pos != string::npos)
			{
				if (strParam2)
					*strParam2 = trim_right(strParam1.substr(pos + 1));

				strParam1.resize(pos);
			}

			inplace::trim_right(strParam1);
			return true;
		}

		if (starts_with(i.Str, L'"'))
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
	lng Title;
	string_view Mask;

	if (HelpLanguage)
	{
		Title = lng::MHelpLangTitle;
		Mask=Global->HelpFileMask;
	}
	else
	{
		Title = lng::MLangTitle;
		Mask=LangFileMask;
	}

	const auto LangMenu = VMenu2::create(msg(Title), {}, ScrY - 4);
	LangMenu->SetMenuFlags(VMENU_WRAPMODE);
	LangMenu->SetPosition({ ScrX / 2 - 8 + 5 * HelpLanguage, ScrY / 2 - 4 + 2 * HelpLanguage, 0, 0 });

	for (const auto& FindData: os::fs::enum_files(path::join(Global->g_strFarPath, Mask)))
	{
		const os::fs::file LangFile(path::join(Global->g_strFarPath, FindData.FileName), FILE_READ_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING);
		if (!LangFile)
			continue;

		const auto Codepage = GetFileCodepage(LangFile, encoding::codepage::oem(), nullptr, false);

		string strLangName, strLangDescr;

		if (!GetLangParam(LangFile, L"Language"sv, strLangName, &strLangDescr, Codepage))
			continue;

		string strEntryName;

		if (HelpLanguage && (
			GetLangParam(LangFile, L"PluginContents"sv, strEntryName, nullptr, Codepage) ||
			GetLangParam(LangFile, L"DocumentContents"sv, strEntryName, nullptr, Codepage)
		))
			continue;

		MenuItemEx LangMenuItem(!strLangDescr.empty()? strLangDescr : strLangName);

		// No duplicate languages
		if (LangMenu->FindItem(0, LangMenuItem.Name, LIFIND_EXACTMATCH) != -1)
			continue;

		LangMenuItem.SetSelect(equal_icase(Dest, strLangName));
		LangMenuItem.ComplexUserData = strLangName;
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

static string ConvertString(const string_view Src)
{
	string Result;
	Result.reserve(Src.size());

	for (auto i = Src.begin(); i != Src.end(); ++i)
	{
		switch (*i)
		{
		case L'\\':
			if (++i == Src.end())
			{
				Result.push_back(L'\\');
				return Result;
			}

			switch (*i)
			{
			case L'\\':
				Result.push_back(L'\\');
				break;

			case L'"':
				Result.push_back(L'"');
				break;

			case L'n':
				Result.push_back(L'\n');
				break;

			case L'r':
				Result.push_back(L'\r');
				break;

			case L'b':
				Result.push_back(L'\b');
				break;

			case L't':
				Result.push_back('\t');
				break;

			default:
				Result.push_back(L'\\');
				--i;
				break;
			}
			break;

		case L'"':
			Result.push_back(L'"');
			if (++i == Src.end())
				return Result;

			if (*i != L'"')
				--i;
			break;

		default:
			Result.push_back(*i);
			break;
		}
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

static lng_line_type parse_lng_line(const string_view str, bool ParseLabels, string_view& Label, string_view& Data)
{
	Label = {};
	Data = {};

	//-- "Text"
	if (starts_with(str, L'"'))
	{
		Data = str.substr(1, str.size() - (ends_with(str, L'"')? 2 : 1));
		return lng_line_type::text;
	}

	//-- //[Label]
	if (ParseLabels)
	{
		const auto Prefix = L"//["sv, Suffix = L"]"sv;
		if (starts_with(str, Prefix) && ends_with(str, Suffix))
		{
			Label = str.substr(Prefix.size(), str.size() - Prefix.size() - Suffix.size());
			return lng_line_type::label;
		}
	}

	//-- MLabel="Text"
	if (ParseLabels && !str.empty() && str.back() == L'"' && std::iswalpha(str.front()))
	{
		auto [Name, Value] = split(str);
		inplace::trim(Name);
		inplace::trim(Value);

		if (!Name.empty() && Value.size() > 1 && starts_with(Value, L'"'))
		{
			Label = Name;
			Data = Value.substr(1, Value.size() - 2);
			return lng_line_type::both;
		}
	}

	return lng_line_type::none;
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

static void LoadCustomStrings(string_view const FileName, std::unordered_map<string, string>& Strings)
{
	const os::fs::file CustomFile(FileName, FILE_READ_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING);
	if (!CustomFile)
		return;

	const auto CustomFileCodepage = GetFileCodepage(CustomFile, encoding::codepage::oem(), nullptr, false);

	string SavedLabel;

	os::fs::filebuf StreamBuffer(CustomFile, std::ios::in);
	std::istream Stream(&StreamBuffer);
	Stream.exceptions(Stream.badbit | Stream.failbit);

	for (const auto& i: enum_lines(Stream, CustomFileCodepage))
	{
		string_view Label, Text;
		switch (parse_lng_line(trim(i.Str), true, Label, Text))
		{
		case lng_line_type::label:
			SavedLabel = Label;
			break;

		case lng_line_type::text:
			Strings.emplace(std::move(SavedLabel), ConvertString(Text));
			SavedLabel.clear();
			break;

		case lng_line_type::both:
			Strings.emplace(Label, ConvertString(Text));
			break;

		default:
			break;
		}
	}
}

void language::load(string_view const Path, string_view const Language, int CountNeed)
{
	SCOPED_ACTION(os::last_error_guard);

	auto Data = m_Data->create();

	const auto [LangFile, LangFileName, LangFileCodePage] = OpenLangFile(Path, LangFileMask, Language);
	if (!LangFile)
	{
		throw MAKE_FAR_KNOWN_EXCEPTION(format(FSTR(L"Cannot find any language files in \"{0}\""), Path));
	}

	Data->m_FileName = LangFile.GetName();

	if (CountNeed != -1)
	{
		Data->reserve(CountNeed);
	}

	// try to load Far<LNG>.lng.custom file(s)
	std::unordered_map<string, string> CustomStrings;

	const auto CustomLngInSameDir = Data->m_FileName + L".custom"sv;
	const auto CustomLngInProfileDir = concat(Global->Opt->ProfilePath, L'\\', ExtractFileName(CustomLngInSameDir));

	// LoadCustomStrings uses map.emplace (does not overwrite existing entires) so the high priority location should come first.
	// If for whatever reason it will use insert_or_assign one day - change the order here.
	LoadCustomStrings(CustomLngInProfileDir, CustomStrings);
	LoadCustomStrings(CustomLngInSameDir, CustomStrings);

	const auto LoadLabels = !CustomStrings.empty();

	string SavedLabel;

	os::fs::filebuf StreamBuffer(LangFile, std::ios::in);
	std::istream Stream(&StreamBuffer);
	Stream.exceptions(Stream.badbit | Stream.failbit);

	for (const auto& i: enum_lines(Stream, LangFileCodePage))
	{
		string_view Label, Text;
		switch (parse_lng_line(trim(i.Str), LoadLabels, Label, Text))
		{
		case lng_line_type::label:
			SavedLabel = Label;
			break;

		case lng_line_type::text:
			if (LoadLabels)
			{
				const auto Iterator = CustomStrings.find(SavedLabel);
				if (Iterator != CustomStrings.cend())
				{
					Text = Iterator->second;
				}
				SavedLabel.clear();
			}

			Data->add(ConvertString(Text));
			break;

		default:
			break;
		}
	}

	//   Проведем проверку на количество строк в LNG-файлах
	if (CountNeed != -1 && static_cast<size_t>(CountNeed) != Data->size())
	{
		throw MAKE_FAR_KNOWN_EXCEPTION(format(
			FSTR(L"Language file \"{0}\" is malformed: expected {1} items, found {2}"),
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
			format(msg(lng::MLanguageStringNotFound), MsgId)
		},
		{ lng::MOk, lng::MQuit }) == Message::second_button)
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
		string_view Line, Label, Data;
		lng_line_type Result;
	}
	Tests[]
	{
		{ L"\"Text\""sv,      {},          L"Text"sv,  lng_line_type::text,  },
		{ L"\"Text"sv,        {},          L"Text"sv,  lng_line_type::text,  },
		{ L"//[Label]"sv,     L"Label"sv,  {},         lng_line_type::label, },
		{ L"//[Lab"sv,        {},          {},         lng_line_type::none,  },
		{ L"foo = \"bar\""sv, L"foo"sv,    L"bar"sv,   lng_line_type::both,  },
		{ L"foo=\"bar"sv,     {},          {},         lng_line_type::none,  },
		{ L"foo=bar\""sv,     {},          {},         lng_line_type::none,  },
		{ L"foo=bar"sv,       {},          {},         lng_line_type::none,  },
		{ L"foo="sv,          {},          {},         lng_line_type::none,  },
		{ L"foo"sv,           {},          {},         lng_line_type::none,  },
	};

	for (const auto& i: Tests)
	{
		string_view Label, Data;
		const auto Result = parse_lng_line(i.Line, true, Label, Data);
		REQUIRE(i.Result == Result);
		REQUIRE(i.Label == Label);
		REQUIRE(i.Data == Data);
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
