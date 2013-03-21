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
#include "savescr.hpp"
#include "TPreRedrawFunc.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "filestr.hpp"
#include "codepage.hpp"
#include "cache.hpp"

DizList::DizList():
	Modified(false),
	OrigCodePage(CP_DEFAULT),
	AnsiBuf(nullptr)
{
}

DizList::~DizList()
{
	Reset();

	if (AnsiBuf)
		xf_free(AnsiBuf);
}

void DizList::Reset()
{
	DizData.clear();
	Modified=false;
	OrigCodePage=CP_DEFAULT;
}

void DizList::PR_ReadingMsg()
{
	Message(0,0,L"",MSG(MReadingDiz));
}

void DizList::Read(const string& Path, const string* DizName)
{
	Reset();
	TPreRedrawFuncGuard preRedrawFuncGuard(DizList::PR_ReadingMsg);
	const wchar_t *NamePtr=Global->Opt->Diz.strListNames;

	for (;;)
	{
		if (DizName)
		{
			strDizFileName = *DizName;
		}
		else
		{
			strDizFileName = Path;

			if (!PathCanHoldRegularFile(strDizFileName))
				break;

			string strArgName;

			if (!(NamePtr=GetCommaWord(NamePtr,strArgName)))
				break;

			AddEndSlash(strDizFileName);
			strDizFileName += strArgName;
		}

		File DizFile;
		if (DizFile.Open(strDizFileName,GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING))
		{
			GetFileString GetStr(DizFile);
			wchar_t *DizText;
			int DizLength;
			clock_t StartTime=clock();
			uintptr_t CodePage=CP_DEFAULT;
			bool bSigFound=false;

			if (!GetFileFormat(DizFile,CodePage,&bSigFound,false) || !bSigFound)
				CodePage = Global->Opt->Diz.AnsiByDefault ? CP_ACP : CP_OEMCP;

			auto LastAdded = DizData.end(); 
			while (GetStr.GetString(&DizText, CodePage, DizLength) > 0)
			{
				if (!(DizData.size() & 127) && clock()-StartTime>1000)
				{
					SetCursorType(FALSE,0);
					PR_ReadingMsg();

					if (CheckForEsc())
						break;
				}

				RemoveTrailingSpaces(DizText);

				if (*DizText)
				{
					if(!IsSpace(*DizText))
					{
						LastAdded = AddRecord(DizText);
					}
					else
					{
						if (LastAdded != DizData.end())
						{
							LastAdded->second.push_back(DizText);
						}
					}
				}
			}

			OrigCodePage=CodePage;
			Modified=false;
			DizFile.Close();
			return;
		}

		if (DizName)
			break;
	}

	Modified=false;
	strDizFileName.Clear();
}
desc_map::iterator DizList::AddRecord(const string& Name, const string& Description)
{
	Modified=true;
	std::list<string> DescStrings;
	DescStrings.push_back(Description);
	return DizData.insert(DizData.begin(), VALUE_TYPE(DizData)(Name, DescStrings));
}

desc_map::iterator DizList::AddRecord(const string& DizText)
{
	size_t NameStart = 0, NameLength = 0;
	const wchar_t* DizTextPtr = DizText;
	if (*DizTextPtr == L'\"')
	{
		DizTextPtr++;
		NameStart++;

		while (*DizTextPtr && *DizTextPtr!=L'\"')
		{
			DizTextPtr++;
			NameLength++;
		}
	}
	else
	{
		while (!IsSpaceOrEos(*DizTextPtr))
		{
			DizTextPtr++;
			NameLength++;
		}
	}

	return AddRecord(DizText.SubStr(NameStart, NameLength), DizText.SubStr(NameLength + (NameStart? 2: 0)));
}

const wchar_t* DizList::GetDizTextAddr(const string& Name, const string& ShortName, const __int64 FileSize)
{
	const wchar_t *DizText=nullptr;
	auto DizPos=Find(Name,ShortName);

	if (DizPos != DizData.end())
	{
		DizText=DizPos->second.front();

		if (iswdigit(*DizText))
		{
			FormatString SizeText;
			SizeText << FileSize;
			const wchar_t *DizPtr=DizText;
			bool SkipSize=true;

			for (size_t i = 0; i < SizeText.GetLength(); ++i, ++DizPtr)
			{
				if (*DizPtr!=L',' && *DizPtr!=L'.' && *DizPtr != SizeText.At(i))
				{
					SkipSize=false;
					break;
				}
			}

			if (SkipSize && IsSpace(*DizPtr))
			{
				DizText=DizPtr;
			}
		}
		while (*DizText && IsSpace(*DizText))
			DizText++;
	}

	return DizText;
}

desc_map::iterator DizList::Find(const string& Name, const string& ShortName)
{
	auto i = DizData.find(Name);
	if(i == DizData.end())
		i = DizData.find(ShortName);

	//если файл описаний был в OEM/ANSI то имена файлов могут не совпадать с юникодными
	if (i == DizData.end() && !IsUnicodeOrUtfCodePage(OrigCodePage) && OrigCodePage!=CP_DEFAULT)
	{
		size_t len = Name.GetLength();
		char *tmp = (char *)xf_realloc_nomove(AnsiBuf, len+1);

		AnsiBuf = tmp;
		WideCharToMultiByte(OrigCodePage, 0, Name, static_cast<int>(len), AnsiBuf, static_cast<int>(len), nullptr, nullptr);
		AnsiBuf[len]=0;
		string strRecoded(AnsiBuf, OrigCodePage);

		if (strRecoded==Name)
			return DizData.end();

		return DizData.find(strRecoded);
	}

	return i;
}

bool DizList::DeleteDiz(const string& Name,const string& ShortName)
{
	auto i = Find(Name,ShortName);
	if (i != DizData.end())
	{
		i = DizData.erase(i);
		Modified=true;
		return true;
	}
	return false;
}

bool DizList::Flush(const string& Path,const string* DizName)
{
	if (!Modified)
		return true;

	if (DizName)
	{
		strDizFileName = *DizName;
	}
	else if (strDizFileName.IsEmpty())
	{
		if (DizData.empty() || !Path)
			return false;

		strDizFileName = Path;
		AddEndSlash(strDizFileName);
		string strArgName;
		GetCommaWord(Global->Opt->Diz.strListNames,strArgName);
		strDizFileName += strArgName;
	}

	DWORD FileAttr=apiGetFileAttributes(strDizFileName);

	if (FileAttr != INVALID_FILE_ATTRIBUTES)
	{
		if (FileAttr&FILE_ATTRIBUTE_READONLY)
		{
			if(Global->Opt->Diz.ROUpdate)
			{
				if(apiSetFileAttributes(strDizFileName,FileAttr))
				{
					FileAttr^=FILE_ATTRIBUTE_READONLY;
				}
			}
		}

		if(!(FileAttr&FILE_ATTRIBUTE_READONLY))
		{
			apiSetFileAttributes(strDizFileName,FILE_ATTRIBUTE_ARCHIVE);
		}
		else
		{
			Message(MSG_WARNING,1,MSG(MError),MSG(MCannotUpdateDiz),MSG(MCannotUpdateRODiz),MSG(MOk));
			return false;
		}
	}

	File DizFile;

	bool AnyError=false;

	bool EmptyDiz=true;
	// Don't use CreationDisposition=CREATE_ALWAYS here - it's kills alternate streams
	if(!DizData.empty() && DizFile.Open(strDizFileName, GENERIC_WRITE, FILE_SHARE_READ, nullptr, FileAttr==INVALID_FILE_ATTRIBUTES?CREATE_NEW:TRUNCATE_EXISTING))
	{
		uintptr_t CodePage = Global->Opt->Diz.SaveInUTF ? CP_UTF8 : (Global->Opt->Diz.AnsiByDefault ? CP_ACP : CP_OEMCP);

		CachedWrite Cache(DizFile);

		if (CodePage == CP_UTF8)
		{
			DWORD dwSignature = SIGN_UTF8;
			if(!Cache.Write(&dwSignature, 3))
			{
				AnyError=true;
			}
		}

		if(!AnyError)
		{
			FOR_CONST_RANGE(DizData, i)
			{
				string dump = i->first;
				QuoteSpaceOnly(dump);
				dump += i->second.front();
				if(i->second.size() > 1)
				{
					auto start = i->second.cbegin();
					++start;
					std::for_each(start, i->second.cend(), [&dump](const VALUE_TYPE(i->second)& j)
					{
						dump.Append(L"\r\n ").Append(j);
					});
				}
				DWORD Size=static_cast<DWORD>((dump.GetLength() + 1) * (CodePage == CP_UTF8? 3 : 1)); //UTF-8, up to 3 bytes per char support
				char_ptr DizText(Size);
				if (DizText)
				{
					int BytesCount=WideCharToMultiByte(CodePage, 0, dump, static_cast<int>(dump.GetLength()+1), DizText.get(), Size, nullptr, nullptr);
					if (BytesCount && BytesCount-1)
					{
						if(Cache.Write(DizText.get(), BytesCount-1))
						{
							EmptyDiz=false;
						}
						else
						{
							AnyError=true;
							break;
						}
						if(!Cache.Write("\r\n", 2))
						{
							AnyError=true;
							break;
						}
					}
				}
			}
		}

		if(!AnyError)
		{
			if(!Cache.Flush())
			{
				AnyError=true;
			}
		}

		DizFile.Close();
	}

	if (!EmptyDiz && !AnyError)
	{
		if (FileAttr==INVALID_FILE_ATTRIBUTES)
		{
			FileAttr=FILE_ATTRIBUTE_ARCHIVE|(Global->Opt->Diz.SetHidden?FILE_ATTRIBUTE_HIDDEN:0);
		}
		apiSetFileAttributes(strDizFileName,FileAttr);
	}
	else
	{
		apiDeleteFile(strDizFileName);
		if(AnyError)
		{
			Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotUpdateDiz),MSG(MOk));
			return false;
		}
	}

	Modified=false;
	return true;
}

void DizList::AddDizText(const string& Name,const string& ShortName,const string& DizText)
{
	DeleteDiz(Name,ShortName);
	string strQuotedName = Name;
	QuoteSpaceOnly(strQuotedName);
	AddRecord(FormatString()<<fmt::LeftAlign()<<fmt::MinWidth(Global->Opt->Diz.StartPos>1?Global->Opt->Diz.StartPos-2:0)<<strQuotedName<<L" "<<DizText);
}

bool DizList::CopyDiz(const string& Name, const string& ShortName, const string& DestName, const string& DestShortName, DizList* DestDiz)
{
	auto i = Find(Name,ShortName);

	if (i == DizData.end())
		return false;

	DestDiz->DeleteDiz(DestName, DestShortName);
	DestDiz->DizData.insert(VALUE_TYPE(DizData)(DestName, i->second));
	DestDiz->Modified = true;

	return true;
}

void DizList::GetDizName(string &strDizName)
{
	strDizName = strDizFileName;
}
