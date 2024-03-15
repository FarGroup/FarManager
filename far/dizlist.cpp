/*
dizlist.cpp

Описания файлов
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
#include "dizlist.hpp"

// Internal:
#include "lang.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "filestr.hpp"
#include "encoding.hpp"
#include "exception.hpp"
#include "datetime.hpp"
#include "global.hpp"
#include "file_io.hpp"
#include "log.hpp"
#include "stddlg.hpp"
#include "encoding.hpp"

// Platform:
#include "platform.hpp"
#include "platform.fs.hpp"

// Common:
#include "common/enum_tokens.hpp"
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

void DizList::Reset()
{
	m_DizData.clear();
	m_RemovedEntries.clear();
	m_OrderForWrite.clear();
	m_DizFileName.clear();
	m_Modified = false;
	m_CodePage.reset();
}

void DizList::Read(string_view const Path, const string* DizName)
{
	Reset();

	const auto ReadDizFile = [this](const string_view Name)
	{
		const os::fs::file DizFile(Name, FILE_READ_DATA, os::fs::file_share_read, nullptr, OPEN_EXISTING);
		if (!DizFile)
			return false;

		const time_check TimeCheck;

		const auto CodePage = GetFileCodepage(DizFile, Global->Opt->Diz.AnsiByDefault? encoding::codepage::ansi() : encoding::codepage::oem(), nullptr, false);

		auto LastAdded = m_DizData.end();
		string DizText;

		os::fs::filebuf StreamBuffer(DizFile, std::ios::in);
		std::istream Stream(&StreamBuffer);
		Stream.exceptions(Stream.badbit | Stream.failbit);

		for (const auto& i: enum_lines(Stream, CodePage))
		{
			DizText = i.Str;

			if (TimeCheck)
			{
				HideCursor();

				if (CheckForEscAndConfirmAbort())
					break;

				// MReadingDiz unused
			}

			inplace::trim_right(DizText);

			if (DizText.empty())
				continue;

			if(!std::iswblank(DizText.front()))
			{
				auto NameBegin = DizText.cbegin();
				auto NameEnd = DizText.cend();
				auto DescBegin = NameEnd;

				if (DizText.front() == L'"')
				{
					++NameBegin;
					NameEnd = std::find(NameBegin, DizText.cend(), L'"');
					if (NameEnd != DizText.cend())
					{
						DescBegin = NameEnd + 1;
					}
				}
				else
				{
					DescBegin = NameEnd = std::find_if(NameBegin, DizText.cend(), std::iswblank);
				}

				// Insert unconditionally
				LastAdded = Insert({ std::to_address(NameBegin), static_cast<size_t>(NameEnd - NameBegin) });
				LastAdded->second.emplace_back(DescBegin, DizText.cend());
			}
			else if (LastAdded != m_DizData.end())
			{
				LastAdded->second.emplace_back(DizText);
			}
		}

		m_CodePage = CodePage;
		m_Modified = false;
		m_DizFileName = Name;

		return true;
	};

	const auto try_read_diz_file = [&](const string_view Name)
	{
		try
		{
			return ReadDizFile(Name);
		}
		catch (std::exception const& e)
		{
			m_DizData.clear();
			m_DizFileName.clear();
			m_Modified = false;

			LOGERROR(L"{}"sv, e);

			return false;
		}
	};

	if (DizName)
	{
		try_read_diz_file(*DizName);
	}
	else if (PathCanHoldRegularFile(Path))
	{
		for (const auto& i: enum_tokens_with_quotes(Global->Opt->Diz.strListNames.Get(), L",;"sv))
		{
			if (i.empty())
				continue;

			if (try_read_diz_file(path::join(Path, i)))
				break;
		}
	}
}

string_view DizList::Get(const string_view Name, const string_view ShortName, const long long FileSize) const
{
	const auto Iterator = Find(Name, ShortName);

	if (Iterator == m_DizData.end())
	{
		return {};
	}

	const auto& Description = Iterator->second.front();
	if (Description.empty())
	{
		return {};
	}

	auto Begin = std::ranges::find_if_not(Description, std::iswblank);

	if (std::iswdigit(*Begin))
	{
		const auto SizeText = str(FileSize);
		auto DescrIterator = Begin;
		auto SkipSize = true;

		for (size_t i = 0; i < SizeText.size() && DescrIterator != Description.cend() ; ++i, ++DescrIterator)
		{
			if (*DescrIterator != L',' && *DescrIterator != L'.' && *DescrIterator != SizeText[i])
			{
				SkipSize=false;
				break;
			}
		}

		if (SkipSize && DescrIterator != Description.cend() && std::iswblank(*DescrIterator))
		{
			Begin = DescrIterator;
		}
	}

	Begin = std::find_if_not(Begin, Description.cend(), std::iswblank);
	if (Begin == Description.cend())
	{
		return {};
	}

	return string_view(Description).substr(Begin - Description.begin());
}

DizList::desc_map::iterator DizList::Find(const string_view Name, const string_view ShortName)
{
	auto Iterator = m_DizData.find(Name);

	if (Iterator == m_DizData.end())
		Iterator = m_DizData.find(ShortName);

	//если файл описаний был в OEM/ANSI то имена файлов могут не совпадать с юникодными
	if (Iterator == m_DizData.end() && m_CodePage && !IsUtfCodePage(*m_CodePage))
	{
		const auto strRecoded = encoding::get_chars(*m_CodePage, encoding::get_bytes(*m_CodePage, Name));
		if (strRecoded == Name)
		{
			return Iterator;
		}
		return m_DizData.find(strRecoded);
	}

	return Iterator;
}

DizList::desc_map::const_iterator DizList::Find(const string_view Name, const string_view ShortName) const
{
	return const_cast<DizList&>(*this).Find(Name, ShortName);
}

DizList::desc_map::iterator DizList::Insert(string_view const Name)
{
	auto Iterator = m_DizData.emplace(Name, description_data{});
	m_OrderForWrite.push_back(std::to_address(Iterator));
	return Iterator;
}

bool DizList::Erase(const string_view Name, const string_view ShortName)
{
	const auto Iterator = Find(Name, ShortName);
	if (Iterator == m_DizData.end())
	{
		return false;
	}

	m_OrderForWrite.erase(std::ranges::find(m_OrderForWrite, std::to_address(Iterator)));

	// Sometimes client can keep the pointer after erasure and use it,
	// e. g. if a description has been deleted during file moving and filelist decided to redraw in the process.
	// Zeroing the pointer via some callback could be quite complex, so we just keep the data alive for a while:
	m_RemovedEntries.emplace_back(std::move(Iterator->second));
	m_DizData.erase(Iterator);
	m_Modified = true;
	return true;
}

bool DizList::Flush(string_view const Path, const string* DizName)
{
	if (!m_Modified)
		return true;

	SCOPE_SUCCESS{ m_Modified = false; };


	if (DizName)
	{
		m_DizFileName = *DizName;
	}
	else if (m_DizFileName.empty())
	{
		if (m_DizData.empty() || Path.empty())
			return false;

		const auto Enum = enum_tokens_with_quotes(Global->Opt->Diz.strListNames.Get(), L",;"sv);
		auto Begin = Enum.begin();

		while (Begin != Enum.end() && Begin->empty())
			++Begin;

		if (Begin != Enum.end())
			m_DizFileName = path::join(Path, *Begin);

		if (m_DizFileName.empty())
			return false;
	}

	const auto FileAttr = os::fs::get_file_attributes(m_DizFileName);

	if (FileAttr != INVALID_FILE_ATTRIBUTES && FileAttr & FILE_ATTRIBUTE_READONLY)
	{
		if (!Global->Opt->Diz.ROUpdate &&
			Message(MSG_WARNING,
				msg(lng::MError),
				{
					m_DizFileName,
					msg(lng::MEditRO),
					msg(lng::MEditOvr)
				},
				{ lng::MYes, lng::MNo }) != message_result::first_button)
			return false;

		if (!os::fs::set_file_attributes(m_DizFileName, FileAttr & ~FILE_ATTRIBUTE_READONLY)) //BUGBUG
		{
			LOGWARNING(L"set_file_attributes({}): {}"sv, m_DizFileName, os::last_error());
		}
	}

	try
	{
		if (m_OrderForWrite.empty())
		{
			if (!os::fs::delete_file(m_DizFileName))
				throw far_exception(L"Can't delete the file"sv);

			return true;
		}

		if (!m_CodePage || Global->Opt->Diz.SaveInUTF)
		{
			m_CodePage =
				Global->Opt->Diz.SaveInUTF?
				CP_UTF8 :
				Global->Opt->Diz.AnsiByDefault?
					encoding::codepage::ansi() :
					encoding::codepage::oem();
		}

		// Encoding could fail, so we need to prepare the data before touching the file
		std::stringstream StrStream;
		StrStream.exceptions(StrStream.badbit | StrStream.failbit);
		encoding::writer Writer(StrStream, *m_CodePage, true, !Global->Opt->Diz.ValidateConversion);
		const auto Eol = eol::win.str();

		for (const auto& i_ptr: m_OrderForWrite)
		{
			const auto& [Name, Lines] = *i_ptr;
			Writer.write(quote_space(Name));
			for (const auto& Description: Lines)
			{
				Writer.write(Description, Eol);
			}
		}

		save_file_with_replace(m_DizFileName, FileAttr, Global->Opt->Diz.SetHidden? FILE_ATTRIBUTE_HIDDEN : 0, false, [&](std::ostream& Stream)
		{
			Stream << StrStream.rdbuf();
		});
	}
	catch (far_exception const& e)
	{
		Message(MSG_WARNING, e,
			msg(lng::MError),
			{
				msg(lng::MCannotUpdateDiz)
			},
			{ lng::MOk });
		return false;
	}

	m_Modified=false;
	return true;
}

void DizList::Set(const string_view Name, const string_view ShortName, const string_view DizText)
{
	auto Iterator = Find(Name, ShortName);
	if (Iterator == m_DizData.end())
	{
		Iterator = Insert(Name);
	}

	auto& List = Iterator->second;
	// Keep the record alive for a while just in case if filelist decides to redraw in the process.
	m_RemovedEntries.emplace_back(std::move(List));

	List.clear();

	const auto KeySize = Iterator->first.size();
	const auto NumberOfSpaces = std::max(static_cast<int>(Global->Opt->Diz.StartPos - 1), static_cast<int>(KeySize + 1)) - KeySize;
	List.emplace_back(string(NumberOfSpaces, L' ') + DizText);
	m_Modified = true;
}

bool DizList::CopyDiz(const string_view Name, const string_view ShortName, const string_view DestName, const string_view DestShortName, DizList* DestDiz) const
{
	const auto Iterator = Find(Name, ShortName);
	if (Iterator == m_DizData.end())
	{
		return false;
	}

	auto DestIterator = DestDiz->Find(DestName, DestShortName);
	if (DestIterator == DestDiz->m_DizData.end())
	{
		DestIterator = DestDiz->Insert(DestName);
	}

	DestIterator->second = Iterator->second;
	DestDiz->m_Modified = true;

	return true;
}
