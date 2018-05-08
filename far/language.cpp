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

#include "language.hpp"

#include "lang.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "manager.hpp"
#include "message.hpp"
#include "config.hpp"
#include "filestr.hpp"
#include "interf.hpp"
#include "lasterror.hpp"
#include "string_utils.hpp"
#include "pathmix.hpp"
#include "exception.hpp"
#include "global.hpp"

#include "platform.fs.hpp"

#include "common/function_traits.hpp"
#include "common/scope_exit.hpp"

#include "format.hpp"

static const wchar_t LangFileMask[] = L"*.lng";

std::tuple<os::fs::file, string, uintptr_t> OpenLangFile(const string& Path,const string& Mask,const string& Language)
{
	FN_RETURN_TYPE(OpenLangFile) CurrentFileData, EnglishFileData;

	for (const auto& FindData: os::fs::enum_files(path::join(Path, Mask)))
	{
		const auto CurrentFileName = path::join(Path, FindData.FileName);

		auto& CurrentFile = std::get<0>(CurrentFileData);
		auto& CurrentLngName = std::get<1>(CurrentFileData);
		auto& CurrentCodepage = std::get<2>(CurrentFileData);

		CurrentFile = os::fs::file(CurrentFileName, FILE_READ_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING);
		if (CurrentFile)
		{
			// Default
			CurrentCodepage = CP_OEMCP;

			GetFileFormat(CurrentFile, CurrentCodepage, nullptr, false);

			if (GetLangParam(CurrentFile, L"Language", CurrentLngName, nullptr, CurrentCodepage) && equal_icase(CurrentLngName, Language))
			{
				return CurrentFileData;
			}

			if (equal_icase(CurrentLngName, L"English"_sv))
			{
				EnglishFileData = std::move(CurrentFileData);
			}
		}
	}

	if (std::get<0>(EnglishFileData))
		return EnglishFileData;

	return CurrentFileData;
}


bool GetLangParam(const os::fs::file& LangFile, const string& ParamName, string& strParam1, string* strParam2, UINT nCodePage)
{
	const auto strFullParamName = concat(L'.', ParamName);
	const auto CurFilePos = LangFile.GetPointer();
	SCOPE_EXIT{ LangFile.SetPointer(CurFilePos, nullptr, FILE_BEGIN); };

	for (const auto& i: enum_file_lines(LangFile, nCodePage))
	{
		if (starts_with_icase(i.Str, strFullParamName))
		{
			const auto EqPos = i.Str.find(L'=');

			if (EqPos != string::npos)
			{
				assign(strParam1, i.Str.substr(EqPos + 1));

				if (strParam2)
					strParam2->clear();

				const auto pos = strParam1.find(L',');

				if (pos != string::npos)
				{
					if (strParam2)
					{
						*strParam2 = trim_right(strParam1.substr(pos + 1));
					}

					strParam1.resize(pos);
				}

				inplace::trim_right(strParam1);
				return true;
			}
		}
		else if (starts_with(i.Str, L'"'))
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
	const wchar_t* Mask;

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
	LangMenu->SetPosition(ScrX/2-8+5*HelpLanguage,ScrY/2-4+2*HelpLanguage,0,0);

	for (const auto& FindData: os::fs::enum_files(path::join(Global->g_strFarPath, Mask)))
	{
		const os::fs::file LangFile(path::join(Global->g_strFarPath, FindData.FileName), FILE_READ_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING);
		if (!LangFile)
			continue;

		uintptr_t nCodePage=CP_OEMCP;
		GetFileFormat(LangFile, nCodePage, nullptr, false);
		string strLangName, strLangDescr;

		if (GetLangParam(LangFile, L"Language", strLangName, &strLangDescr, nCodePage))
		{
			string strEntryName;

			if (!HelpLanguage || (
				!GetLangParam(LangFile, L"PluginContents", strEntryName, nullptr, nCodePage) &&
				!GetLangParam(LangFile, L"DocumentContents", strEntryName, nullptr, nCodePage)))
			{
				MenuItemEx LangMenuItem(!strLangDescr.empty()? strLangDescr : strLangName);

				/* $ 01.08.2001 SVS
				   Не допускаем дубликатов!
				   Если в каталог с ФАРом положить еще один HLF с одноименным
				   языком, то... фигня получается при выборе языка.
				*/
				if (LangMenu->FindItem(0,LangMenuItem.Name,LIFIND_EXACTMATCH) == -1)
				{
					LangMenuItem.SetSelect(equal_icase(Dest, strLangName));
					LangMenuItem.UserData = strLangName;
					LangMenu->AddItem(LangMenuItem);
				}
			}
		}
	}

	LangMenu->AssignHighlights(FALSE);
	LangMenu->Run();

	if (LangMenu->GetExitCode()<0)
		return false;

	Dest = *LangMenu->GetUserDataPtr<string>();
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

static void parse_lng_line(const string_view str, string& label, string& data, bool& have_data)
{
	have_data = false;

	//-- //[Label]
	if (starts_with(str, L"//["_sv) && ends_with(str, L"]"_sv))
	{
		const auto LabelView = str.substr(3, str.size() - 3 - 1);
		//-- //[Label=0]
		assign(label, LabelView.substr(0, LabelView.find(L'=')));
		return;
	}

	//-- "Text"
	if (starts_with(str, L'"'))
	{
		have_data = true;
		assign(data, str.substr(1));
		if (!data.empty() && data.back() == L'"')
			data.pop_back();
		return;
	}

	//-- MLabel="Text"
	if (!str.empty() && str.back() == L'"')
	{
		const auto eq_pos = str.find(L'=');
		if (eq_pos != string::npos && InRange(L'A', upper(str[0]), L'Z'))
		{
			assign(data, trim(str.substr(eq_pos + 1)));
			if (data.size() > 1 && data[0] == L'"')
			{
				assign(label, trim(str.substr(0, eq_pos)));
				have_data = true;
				data.pop_back();
				data.erase(0, 1);
			}
		}
	}
}

class language_data final: public i_language_data
{
public:
	std::unique_ptr<i_language_data> create() override { return std::make_unique<language_data>(); }

	void reserve(size_t Size) override { return m_Messages.reserve(Size); }
	void add(string&& Str) override { m_Messages.emplace_back(std::move(Str)); }
	void set_at(size_t Index, string&& Str) override { m_Messages[Index] = std::move(Str); }
	const string& at(size_t Index) const override { return m_Messages[Index]; }
	size_t size() const override { return m_Messages.size(); }

private:
	std::vector<string> m_Messages;
};

void language::load(const string& Path, const string& Language, int CountNeed)
{
	SCOPED_ACTION(GuardLastError);

	auto Data = m_Data->create();

	const auto LangFileData = OpenLangFile(Path, LangFileMask, Language);
	const auto& LangFile = std::get<0>(LangFileData);
	const auto LangFileCodePage = std::get<2>(LangFileData);

	if (!LangFile)
	{
		throw MAKE_EXCEPTION(exception, L"Cannot find language data");
	}

	Data->m_FileName = LangFile.GetName();

	if (CountNeed != -1)
	{
		Data->reserve(CountNeed);
	}

	std::unordered_map<string, size_t> id_map;
	string label, text;
	for (const auto& i: enum_file_lines(LangFile, LangFileCodePage))
	{
		bool have_text;
		parse_lng_line(trim(i.Str), label, text, have_text);
		if (have_text)
		{
			auto idx = Data->size();
			Data->add(ConvertString(text));
			if (!label.empty())
			{
				id_map[label] = idx;
				label.clear();
			}
		}
	}

	//   Проведем проверку на количество строк в LNG-файлах
	if (CountNeed != -1 && CountNeed != static_cast<int>(Data->size()))
	{
		throw MAKE_EXCEPTION(exception, Data->m_FileName + L": language data is incorrect or damaged");
	}

	// try to load Far<LNG>.lng.custom file(s)
	//
	if (!id_map.empty())
	{
		const auto& LoadStrings = [&](const string& FileName)
		{
			const os::fs::file CustomFile(FileName, FILE_READ_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING);
			if (!CustomFile)
				return;

			uintptr_t CustomFileCodePage = CP_OEMCP;
			GetFileFormat(CustomFile, CustomFileCodePage, nullptr, false);
			label.clear();
			for (const auto& i: enum_file_lines(CustomFile, CustomFileCodePage))
			{
				bool have_text;
				parse_lng_line(trim(i.Str), label, text, have_text);
				if (have_text && !label.empty())
				{
					const auto found = id_map.find(label);
					if (found != id_map.end())
					{
						Data->set_at(found->second, ConvertString(text));
					}
					label.clear();
				}
			}
		};

		const auto CustomLngInSameDir = Data->m_FileName + L".custom";
		const auto CustomLngInProfileDir = concat(Global->Opt->ProfilePath, L'\\', ExtractFileName(CustomLngInSameDir));

		LoadStrings(CustomLngInSameDir);
		LoadStrings(CustomLngInProfileDir);
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
			format(msg(lng::MLanguageStringNotFound), static_cast<size_t>(MsgId))
		},
		{ lng::MOk, lng::MQuit }) == Message::second_button)
	{
		Global->WindowManager->ExitMainLoop(FALSE);
	}

	return false;
}

plugin_language::plugin_language(const string & Path, const string& Language):
	language(m_Data),
	m_Data(std::make_unique<language_data>())
{
	load(Path, Language);
}

const wchar_t* plugin_language::GetMsg(intptr_t Id) const
{
	return m_Data->validate(Id)? m_Data->at(Id).c_str() : L"";
}


far_language::far_language():
	language(m_Data),
	m_Data(std::make_unique<language_data>())
{
}

bool far_language::is_loaded() const
{
	return static_cast<const language_data&>(*m_Data).size() != 0;
}

const string& far_language::GetMsg(lng Id) const
{
	return static_cast<const language_data&>(*m_Data).at(static_cast<size_t>(Id));
}


const string& msg(lng Id)
{
	return far_language::instance().GetMsg(Id);
}
