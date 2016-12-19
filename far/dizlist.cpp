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

#include "headers.hpp"
#pragma hdrstop

#include "dizlist.hpp"
#include "language.hpp"
#include "TPreRedrawFunc.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "filestr.hpp"
#include "encoding.hpp"
#include "cache.hpp"

DizList::DizList():
	m_CodePage(CP_DEFAULT),
	m_Modified()
{
}

size_t DizList::hasher::operator()(const string& Key) const
{
	return make_hash(Lower(Key));
}

bool DizList::key_equal::operator()(const string& a, const string& b) const
{
	return !StrCmpI(a, b);
}

void DizList::Reset()
{
	m_DizData.clear();
	m_OrderForWrite.clear();
	m_Modified = false;
	m_CodePage = CP_DEFAULT;
}

static void PR_ReadingMsg()
{
	Message(0,0,L"",MSG(lng::MReadingDiz));
};

void DizList::Read(const string& Path, const string* DizName)
{
	Reset();

	struct DizPreRedrawItem : public PreRedrawItem
	{
		DizPreRedrawItem() : PreRedrawItem(PR_ReadingMsg) {}
	};

	SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<DizPreRedrawItem>());
	const wchar_t *NamePtr=Global->Opt->Diz.strListNames.data();

	for (;;)
	{
		if (DizName)
		{
			m_DizFileName = *DizName;
		}
		else
		{
			m_DizFileName = Path;

			if (!PathCanHoldRegularFile(m_DizFileName))
				break;

			string strArgName;
			NamePtr = GetCommaWord(NamePtr, strArgName);

			if (!NamePtr)
				break;

			AddEndSlash(m_DizFileName);
			m_DizFileName += strArgName;
		}

		if (const auto DizFile = os::fs::file(m_DizFileName,GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING))
		{
			clock_t StartTime=clock();
			uintptr_t CodePage=CP_DEFAULT;
			bool bSigFound=false;

			if (!GetFileFormat(DizFile,CodePage,&bSigFound,false) || !bSigFound)
				CodePage = Global->Opt->Diz.AnsiByDefault ? CP_ACP : CP_OEMCP;

			GetFileString GetStr(DizFile, CodePage);

			auto LastAdded = m_DizData.end();
			string DizText;
			while (GetStr.GetString(DizText))
			{
				if (!(m_DizData.size() & 127) && clock() - StartTime > CLOCKS_PER_SEC)
				{
					SetCursorType(false, 0);
					PR_ReadingMsg();

					if (CheckForEsc())
						break;
				}

				RemoveTrailingSpaces(DizText);

				if (!DizText.empty())
				{
					if(!IsSpace(DizText.front()))
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
							DescBegin = NameEnd = std::find(NameBegin, DizText.cend(), L' ');
						}

						// Insert unconditionally
						LastAdded = Insert(string(NameBegin, NameEnd));
						LastAdded->second.emplace_back(DescBegin, DizText.cend());
					}
					else
					{
						if (LastAdded != m_DizData.end())
						{
							LastAdded->second.emplace_back(DizText);
						}
					}
				}
			}

			m_CodePage=CodePage;
			m_Modified = false;
			return;
		}

		if (DizName)
			break;
	}

	m_Modified = false;
	m_DizFileName.clear();
}

const wchar_t* DizList::Get(const string& Name, const string& ShortName, const long long FileSize) const
{
	const auto Iterator = Find(Name, ShortName);

	if (Iterator == m_DizData.end())
	{
		return nullptr;
	}

	const auto& Description = Iterator->second.front();
	if (Description.empty())
	{
		return nullptr;
	}

	auto Begin = Description.begin();

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

		if (SkipSize && IsSpace(*DescrIterator))
		{
			Begin = DescrIterator;
		}
	}

	Begin = std::find_if_not(Begin, Description.cend(), IsSpace);
	if (Begin == Description.cend())
	{
		return nullptr;
	}

	return &*Begin;
}

template<class T>
static auto Find_t(T& Map, const string& Name, const string& ShortName, uintptr_t Codepage)
{
	auto Iterator = Map.find(Name);
	if (Iterator == Map.end())
		Iterator = Map.find(ShortName);

	//если файл описаний был в OEM/ANSI то имена файлов могут не совпадать с юникодными
	if (Iterator == Map.end() && !IsUnicodeOrUtfCodePage(Codepage) && Codepage != CP_DEFAULT)
	{
		const auto strRecoded = encoding::get_chars(Codepage, encoding::get_bytes(Codepage, Name));
		if (strRecoded == Name)
		{
			return Iterator;
		}
		return Map.find(strRecoded);
	}

	return Iterator;
}

DizList::desc_map::iterator DizList::Find(const string& Name, const string& ShortName)
{
	return Find_t(m_DizData, Name, ShortName, m_CodePage);
}

DizList::desc_map::const_iterator DizList::Find(const string& Name, const string& ShortName) const
{
	return Find_t(m_DizData, Name, ShortName, m_CodePage);
}

DizList::desc_map::iterator DizList::Insert(const string& Name)
{
	auto Iterator = m_DizData.emplace(Name, std::list<string>());
	m_OrderForWrite.push_back(&*Iterator);
	return Iterator;
}

bool DizList::Erase(const string& Name,const string& ShortName)
{
	const auto Iterator = Find(Name, ShortName);
	if (Iterator == m_DizData.end())
	{
		return false;
	}

	m_OrderForWrite.erase(std::find(ALL_RANGE(m_OrderForWrite), &*Iterator));
	m_DizData.erase(Iterator);
	m_Modified = true;
	return true;
}

bool DizList::Flush(const string& Path,const string* DizName)
{
	if (!m_Modified)
	{
		return true;
	}

	if (DizName)
	{
		m_DizFileName = *DizName;
	}
	else if (m_DizFileName.empty())
	{
		if (m_DizData.empty() || Path.empty())
			return false;

		m_DizFileName = Path;
		AddEndSlash(m_DizFileName);
		string strArgName;
		GetCommaWord(Global->Opt->Diz.strListNames.data(),strArgName);
		m_DizFileName += strArgName;
	}

	DWORD FileAttr=os::GetFileAttributes(m_DizFileName);

	if (FileAttr != INVALID_FILE_ATTRIBUTES)
	{
		if (FileAttr&FILE_ATTRIBUTE_READONLY)
		{
			if(Global->Opt->Diz.ROUpdate)
			{
				if(os::SetFileAttributes(m_DizFileName,FileAttr))
				{
					FileAttr^=FILE_ATTRIBUTE_READONLY;
				}
			}
		}

		if(!(FileAttr&FILE_ATTRIBUTE_READONLY))
		{
			os::SetFileAttributes(m_DizFileName,FILE_ATTRIBUTE_ARCHIVE);
		}
		else
		{
			Message(MSG_WARNING,1,MSG(lng::MError),MSG(lng::MCannotUpdateDiz),MSG(lng::MCannotUpdateRODiz),MSG(lng::MOk));
			return false;
		}
	}

	string Content;
	for (const auto& i_ptr: m_OrderForWrite)
	{
		const auto& i = *i_ptr;
		auto FileName = i.first;
		QuoteSpaceOnly(FileName);
		Content += FileName;
		for (const auto& Description : i.second)
		{
			append(Content, Description, L"\r\n"s);
		}
	}

	std::string SerialisedContent;
	blob_view Blob;
	if (!Content.empty())
	{
		const auto CodePage = Global->Opt->Diz.SaveInUTF? CP_UTF8 : Global->Opt->Diz.AnsiByDefault? CP_ACP : CP_OEMCP;
		if (IsUnicodeOrUtfCodePage(CodePage))
		{
			Content.insert(0, 1, SIGN_UNICODE);
		}

		SerialisedContent = encoding::get_bytes(CodePage, Content);
		Blob = make_blob_view(SerialisedContent.data(), SerialisedContent.size());
	}

	try
	{
		if (!Blob.empty())
		{
			if(const auto DizFile = os::fs::file(m_DizFileName, GENERIC_WRITE, FILE_SHARE_READ, nullptr, FileAttr == INVALID_FILE_ATTRIBUTES? CREATE_NEW : TRUNCATE_EXISTING))
			{
				size_t Written;
				if (!DizFile.Write(Blob.data(), Blob.size(), Written) || Written != Blob.size())
				{
					throw MAKE_FAR_EXCEPTION("Write error");
				}
			}
			else
			{
				throw MAKE_FAR_EXCEPTION("Can't open file");
			}

			if (FileAttr == INVALID_FILE_ATTRIBUTES)
			{
				FileAttr = FILE_ATTRIBUTE_ARCHIVE | (Global->Opt->Diz.SetHidden? FILE_ATTRIBUTE_HIDDEN : 0);
			}
			// No error checking - non-critical (TODO: log)
			os::SetFileAttributes(m_DizFileName, FileAttr);
		}
		else
		{
			if (!os::DeleteFile(m_DizFileName))
			{
				throw MAKE_FAR_EXCEPTION("Can't delete file");
			}
		}
	}
	catch (const far_exception& e)
	{
		Global->CatchError(e.get_error_codes());
		Message(MSG_WARNING | MSG_ERRORTYPE, 1, MSG(lng::MError), MSG(lng::MCannotUpdateDiz), encoding::utf8::get_chars(e.get_message()).data(), MSG(lng::MOk));
		return false;
	}

	m_Modified=false;
	return true;
}

void DizList::Set(const string& Name,const string& ShortName,const string& DizText)
{
	auto Iterator = Find(Name, ShortName);
	if (Iterator == m_DizData.end())
	{
		Iterator = Insert(Name);
	}

	auto& List = Iterator->second;
	List.clear();

	const auto KeySize = Iterator->first.size();
	const auto NumberOfSpaces = std::max(static_cast<int>(Global->Opt->Diz.StartPos - 1), static_cast<int>(KeySize + 1)) - KeySize;
	List.emplace_back(string(NumberOfSpaces, L' ') + DizText);
	m_Modified = true;
}

bool DizList::CopyDiz(const string& Name, const string& ShortName, const string& DestName, const string& DestShortName, DizList* DestDiz) const
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
