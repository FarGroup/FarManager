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

#include "headers.hpp"
#pragma hdrstop

#include "language.hpp"
#include "lang.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "manager.hpp"
#include "message.hpp"
#include "config.hpp"
#include "strmix.hpp"
#include "filestr.hpp"
#include "interf.hpp"
#include "lasterror.hpp"
#include "string_utils.hpp"
#include "pathmix.hpp"
#include "exception.hpp"

static const wchar_t LangFileMask[] = L"*.lng";

std::tuple<os::fs::file, string, uintptr_t> OpenLangFile(const string& Path,const string& Mask,const string& Language)
{
	FN_RETURN_TYPE(OpenLangFile) CurrentFileData, EnglishFileData;

	auto PathWithSlash = Path;
	AddEndSlash(PathWithSlash);

	for (const auto& FindData: os::fs::enum_files(PathWithSlash + Mask))
	{
		const auto CurrentFileName = PathWithSlash + FindData.strFileName;

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

	string ReadStr;
	GetFileString GetStr(LangFile, nCodePage);
	while (GetStr.GetString(ReadStr))
	{
		if (starts_with_icase(ReadStr, strFullParamName))
		{
			size_t Pos = ReadStr.find(L'=');

			if (Pos != string::npos)
			{
				strParam1 = ReadStr.substr(Pos + 1);

				if (strParam2)
					strParam2->clear();

				size_t pos = strParam1.find(L',');

				if (pos != string::npos)
				{
					if (strParam2)
					{
						*strParam2 = strParam1;
						strParam2->erase(0, pos+1);
						RemoveTrailingSpaces(*strParam2);
					}

					strParam1.resize(pos);
				}

				RemoveTrailingSpaces(strParam1);
				return true;
			}
		}
		else if (!ReadStr.empty() && ReadStr.front() != L'.')
		{
			// Parameters can be only in the header, no point to go deeper
			return false;
		}
	}

	return false;
}

static bool SelectLanguage(bool HelpLanguage)
{
	lng Title;
	const wchar_t* Mask;
	StringOption *strDest;

	if (HelpLanguage)
	{
		Title = lng::MHelpLangTitle;
		Mask=Global->HelpFileMask;
		strDest=&Global->Opt->strHelpLanguage;
	}
	else
	{
		Title = lng::MLangTitle;
		Mask=LangFileMask;
		strDest=&Global->Opt->strLanguage;
	}

	const auto LangMenu = VMenu2::create(msg(Title), nullptr, 0, ScrY - 4);
	LangMenu->SetMenuFlags(VMENU_WRAPMODE);
	LangMenu->SetPosition(ScrX/2-8+5*HelpLanguage,ScrY/2-4+2*HelpLanguage,0,0);

	auto PathWithSlash = Global->g_strFarPath;
	AddEndSlash(PathWithSlash);
	for (const auto& FindData: os::fs::enum_files(PathWithSlash + Mask))
	{
		os::fs::file LangFile(PathWithSlash + FindData.strFileName, FILE_READ_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING);
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
				if (LangMenu->FindItem(0,LangMenuItem.strName,LIFIND_EXACTMATCH) == -1)
				{
					LangMenuItem.SetSelect(equal_icase(strDest->Get(), strLangName));
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

	*strDest = *LangMenu->GetUserDataPtr<string>();
	return true;
}

bool SelectInterfaceLanguage() {return SelectLanguage(false);}
bool SelectHelpLanguage() {return SelectLanguage(true);}

static string ConvertString(const wchar_t *Src, size_t size)
{
	string strDest;
	strDest.reserve(size);

	while (*Src)
	{
		switch (*Src)
		{
		case L'\\':
			switch (Src[1])
			{
			case L'\\':
				strDest.push_back(L'\\');
				Src+=2;
				break;
			case L'\"':
				strDest.push_back(L'\"');
				Src+=2;
				break;
			case L'n':
				strDest.push_back(L'\n');
				Src+=2;
				break;
			case L'r':
				strDest.push_back(L'\r');
				Src+=2;
				break;
			case L'b':
				strDest.push_back(L'\b');
				Src+=2;
				break;
			case L't':
				strDest.push_back('\t');
				Src+=2;
				break;
			default:
				strDest.push_back(L'\\');
				Src++;
				break;
			}
			break;
		case L'"':
			strDest.push_back(L'"');
			Src+=(Src[1]==L'"') ? 2:1;
			break;
		default:
			strDest.push_back(*(Src++));
			break;
		}
	}

	return strDest;
}

static void parse_lng_line(const string& str, string& label, string& data, bool& have_data)
{
	have_data = false;

	//-- //[Label]
	if (str.size() > 4 && str[0] == L'/' && str[1] == L'/' && str[2] == L'[' && str.back() == L']')
	{
		label = str.substr(3);
		label.pop_back();
		auto eq_pos = label.find(L'=');
		if (eq_pos != string::npos)
			label.erase(eq_pos); //-- //[Label=0]
		return;
	}

	//-- "Text"
	if (!str.empty() && str.front() == L'\"')
	{
		have_data = true;
		data = str.substr(1);
		if (!data.empty() && data.back() == L'"')
			data.pop_back();
		return;
	}

	//-- MLabel="Text"
	if (!str.empty() && str.back() == L'"')
	{
		auto eq_pos = str.find(L'=');
		if (eq_pos != string::npos && upper(str[0]) >= L'A' && upper(str[0]) <= L'Z')
		{
			data = str.substr(eq_pos + 1);
			RemoveExternalSpaces(data);
			if (data.size() > 1 && data[0] == L'"')
			{
				label = str.substr(0, eq_pos);
				RemoveExternalSpaces(label);
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
	~language_data() override = default;

	void reserve(size_t Size) override { return m_Messages.reserve(Size); }
	void add(string&& Str) override { m_Messages.emplace_back(std::move(Str)); }
	void set_at(size_t Index, string&& Str) override { m_Messages[Index] = std::move(Str); }
	const string& at(size_t Index) const override { return m_Messages[Index]; }
	size_t size() const override { return m_Messages.size(); }

private:
	std::vector<string> m_Messages;
};

void language::load(const string& Path, int CountNeed)
{
	SCOPED_ACTION(GuardLastError);

	auto Data = m_Data->create();

	const auto LangFileData = OpenLangFile(Path, LangFileMask, Global->Opt->strLanguage);
	const auto& LangFile = std::get<0>(LangFileData);
	auto LangFileCodePage = std::get<2>(LangFileData);

	if (!LangFile)
	{
		throw MAKE_FAR_EXCEPTION(L"Cannot find language data");
	}

	Data->m_FileName = LangFile.GetName();

	GetFileString GetStr(LangFile, LangFileCodePage);

	if (CountNeed != -1)
	{
		Data->reserve(CountNeed);
	}

	std::unordered_map<string, size_t> id_map;
	string label, Buffer, text;
	while (GetStr.GetString(Buffer))
	{
		RemoveExternalSpaces(Buffer);
		bool have_text;
		parse_lng_line(Buffer, label, text, have_text);
		if (have_text)
		{
			auto idx = Data->size();
			Data->add(ConvertString(text.data(), text.size()));
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
		throw MAKE_FAR_EXCEPTION(Data->m_FileName + L": language data is incorrect or damaged");
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
			GetFileString get_str(CustomFile, CustomFileCodePage);
			label.clear();
			while (get_str.GetString(Buffer))
			{
				RemoveExternalSpaces(Buffer);
				bool have_text;
				parse_lng_line(Buffer, label, text, have_text);
				if (have_text && !label.empty())
				{
					const auto found = id_map.find(label);
					if (found != id_map.end())
					{
						Data->set_at(found->second, ConvertString(text.data(), text.size()));
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

plugin_language::plugin_language(const string & Path):
	language(m_Data),
	m_Data(std::make_unique<language_data>())
{
	load(Path);
}

const wchar_t* plugin_language::GetMsg(intptr_t Id) const
{
	return m_Data->validate(Id)? m_Data->at(Id).data() : L"";
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
