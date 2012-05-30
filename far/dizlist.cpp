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

static DizRecord **SearchDizData;
static int _cdecl SortDizIndex(const void *el1,const void *el2);
static int WINAPI SortDizSearch(const void *key,const void *elem,void*);

DizList::DizList():
	DizData(nullptr),
	DizCount(0),
	IndexData(nullptr),
	IndexCount(0),
	Modified(false),
	NeedRebuild(true),
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
	for (size_t I=0; I<DizCount; I++)
		delete DizData[I];

	if (DizData)
		xf_free(DizData);

	DizData=nullptr;
	DizCount=0;

	delete[] IndexData;

	IndexData=nullptr;
	IndexCount=0;
	Modified=false;
	NeedRebuild=true;
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
	const wchar_t *NamePtr=Opt.Diz.strListNames;

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
			UINT CodePage=CP_DEFAULT;
			bool bSigFound=false;

			if (!GetFileFormat(DizFile,CodePage,&bSigFound,false) || !bSigFound)
				CodePage = Opt.Diz.AnsiByDefault ? CP_ACP : CP_OEMCP;

			while (GetStr.GetString(&DizText, CodePage, DizLength) > 0)
			{
				if (!(DizCount & 127) && clock()-StartTime>1000)
				{
					SetCursorType(FALSE,0);
					PR_ReadingMsg();

					if (CheckForEsc())
						break;
				}

				RemoveTrailingSpaces(DizText);

				if (*DizText)
					AddRecord(DizText);
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


bool DizList::AddRecord(const string& DizText)
{
	DizRecord** NewDizData = DizData;

	if (!(DizCount & 15))
		NewDizData=(DizRecord **)xf_realloc(DizData,(DizCount+16+1)*sizeof(DizRecord *));

	if (!NewDizData)
		return false;

	DizData=NewDizData;
	DizData[DizCount] = new DizRecord;
	DizData[DizCount]->DizText = DizText;
	DizData[DizCount]->NameStart=0;
	DizData[DizCount]->NameLength=0;

	const wchar_t* DizTextPtr = DizText;
	if (*DizTextPtr == L'\"')
	{
		DizTextPtr++;
		DizData[DizCount]->NameStart++;

		while (*DizTextPtr && *DizTextPtr!=L'\"')
		{
			DizTextPtr++;
			DizData[DizCount]->NameLength++;
		}
	}
	else
	{
		while (!IsSpaceOrEos(*DizTextPtr))
		{
			DizTextPtr++;
			DizData[DizCount]->NameLength++;
		}
	}

	DizData[DizCount]->Deleted=false;
	NeedRebuild=true;
	Modified=true;
	DizCount++;
	return true;
}


const wchar_t* DizList::GetDizTextAddr(const string& Name, const string& ShortName, const __int64 FileSize)
{
	const wchar_t *DizText=nullptr;
	int TextPos;
	int DizPos=GetDizPosEx(Name,ShortName,&TextPos);

	if (DizPos!=-1)
	{
		DizText=DizData[DizPos]->DizText+TextPos;

		while (*DizText && IsSpace(*DizText))
			DizText++;

		if (iswdigit(*DizText))
		{
			wchar_t SizeText[30];
			const wchar_t *DizPtr=DizText;
			bool SkipSize=true;
			_snwprintf(SizeText,ARRAYSIZE(SizeText),L"%I64u", FileSize);

			for (int I=0; SizeText[I]; DizPtr++)
			{
				if (*DizPtr!=L',' && *DizPtr!=L'.')
				{
					if (SizeText[I++]!=*DizPtr)
					{
						SkipSize=false;
						break;
					}
				}
			}

			if (SkipSize && IsSpace(*DizPtr))
			{
				DizText=DizPtr;

				while (*DizText && IsSpace(*DizText))
					DizText++;
			}
		}
	}

	return DizText;
}


int DizList::GetDizPosEx(const string& Name, const string& ShortName, int *TextPos)
{
	int DizPos=GetDizPos(Name,TextPos);

	if (DizPos==-1)
		DizPos=GetDizPos(ShortName,TextPos);

	//если файл описаний был в OEM/ANSI то имена файлов могут не совпадать с юникодными
	if (DizPos==-1 && !IsUnicodeOrUtfCodePage(OrigCodePage) && OrigCodePage!=CP_DEFAULT)
	{
		size_t len = Name.GetLength();
		char *tmp = (char *)xf_realloc_nomove(AnsiBuf, len+1);

		if (!tmp)
			return -1;

		AnsiBuf = tmp;
		WideCharToMultiByte(OrigCodePage, 0, Name, static_cast<int>(len), AnsiBuf, static_cast<int>(len), nullptr, nullptr);
		AnsiBuf[len]=0;
		string strRecoded(AnsiBuf, OrigCodePage);

		if (strRecoded==Name)
			return -1;

		return GetDizPos(strRecoded,TextPos);
	}

	return DizPos;
}


int DizList::GetDizPos(const string& Name, int *TextPos)
{
	if (!DizData || !*Name)
		return -1;

	if (NeedRebuild)
		BuildIndex();

	SearchDizData=DizData;
	string DizSearchKey(Name);
	int *DestIndex=(int *)bsearchex(&DizSearchKey,IndexData,IndexCount,sizeof(*IndexData),SortDizSearch,nullptr);

	if (DestIndex)
	{
		if (TextPos)
		{
			*TextPos=DizData[*DestIndex]->NameStart+DizData[*DestIndex]->NameLength;

			if (DizData[*DestIndex]->NameStart && DizData[*DestIndex]->DizText[*TextPos]==L'\"')
				(*TextPos)++;
		}

		return *DestIndex;
	}

	return -1;
}


void DizList::BuildIndex()
{
	if (!IndexData || IndexCount!=DizCount)
	{
		delete[] IndexData;
		IndexData=new size_t[DizCount];

		if(!IndexData)
		{
			Reset();
			return;
		}

		IndexCount=DizCount;
	}

	for (size_t I=0; I<IndexCount; I++)
		IndexData[I]=I;

	SearchDizData=DizData;
	far_qsort((void *)IndexData,IndexCount,sizeof(*IndexData),SortDizIndex);
	NeedRebuild=false;
}


int _cdecl SortDizIndex(const void *el1,const void *el2)
{
	const wchar_t *Diz1=SearchDizData[*(int *)el1]->DizText+SearchDizData[*(int *)el1]->NameStart;
	const wchar_t *Diz2=SearchDizData[*(int *)el2]->DizText+SearchDizData[*(int *)el2]->NameStart;
	int Len1=SearchDizData[*(int *)el1]->NameLength;
	int Len2=SearchDizData[*(int *)el2]->NameLength;
	int CmpCode = StrCmpNI(Diz1,Diz2,Min(Len1,Len2));

	if (!CmpCode)
	{
		if (Len1>Len2)
			return 1;

		if (Len1<Len2)
			return -1;

		//for equal names, deleted is bigger
		bool Del1=SearchDizData[*(int *)el1]->Deleted;
		bool Del2=SearchDizData[*(int *)el2]->Deleted;

		if (Del1 && !Del2)
			return 1;

		if (Del2 && !Del1)
			return -1;
	}

	return CmpCode;
}


int WINAPI SortDizSearch(const void *key,const void *elem,void*)
{
	const string* strKey = reinterpret_cast<const string*>(key);
	const wchar_t *SearchName = strKey->CPtr();
	const wchar_t *DizName=SearchDizData[*(int *)elem]->DizText+SearchDizData[*(int *)elem]->NameStart;
	int DizNameLength=SearchDizData[*(int *)elem]->NameLength;
	int NameLength=static_cast<int>(strKey->GetLength());
	int CmpCode=StrCmpNI(SearchName,DizName, Min(DizNameLength,NameLength));

	if (!CmpCode)
	{
		if (NameLength>DizNameLength)
			return 1;

		if (NameLength+1<DizNameLength)
			return -1;

		//filename == filename.
		if (NameLength+1==DizNameLength && !(DizName[NameLength]==L'.' && wcschr(DizName,L'.')==&DizName[NameLength]))
			return -1;

		//for equal names, deleted is bigger so deleted items are never matched
		if (SearchDizData[*(int *)elem]->Deleted)
			return -1;
	}

	return CmpCode;
}


bool DizList::DeleteDiz(const string& Name,const string& ShortName)
{
	int iDizPos=GetDizPosEx(Name,ShortName,nullptr);

	if (iDizPos==-1)
		return false;
	size_t DizPos = iDizPos;
	DizData[DizPos++]->Deleted=true;

	while (DizPos<DizCount)
	{
		if (*DizData[DizPos]->DizText && !IsSpace(DizData[DizPos]->DizText[0]))
			break;

		DizData[DizPos]->Deleted=true;
		DizPos++;
	}

	Modified=true;
	NeedRebuild=true;
	return true;
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
		if (!DizData || !Path)
			return false;

		strDizFileName = Path;
		AddEndSlash(strDizFileName);
		string strArgName;
		GetCommaWord(Opt.Diz.strListNames,strArgName);
		strDizFileName += strArgName;
	}

	DWORD FileAttr=apiGetFileAttributes(strDizFileName);

	if (FileAttr != INVALID_FILE_ATTRIBUTES)
	{
		if (FileAttr&FILE_ATTRIBUTE_READONLY)
		{
			if(Opt.Diz.ROUpdate)
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
	if(DizCount && DizFile.Open(strDizFileName, GENERIC_WRITE, FILE_SHARE_READ, nullptr, FileAttr==INVALID_FILE_ATTRIBUTES?CREATE_NEW:TRUNCATE_EXISTING))
	{
		UINT CodePage = Opt.Diz.SaveInUTF ? CP_UTF8 : (Opt.Diz.AnsiByDefault ? CP_ACP : CP_OEMCP);

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
			for (size_t I=0; I<DizCount; I++)
			{
				if (!DizData[I]->Deleted)
				{
					DWORD Size=static_cast<DWORD>((DizData[I]->DizText.GetLength()+1)*(CodePage == CP_UTF8?3:1)); //UTF-8, up to 3 bytes per char support
					char* lpDizText = new char[Size];
					if (lpDizText)
					{
						int BytesCount=WideCharToMultiByte(CodePage, 0, DizData[I]->DizText, static_cast<int>(DizData[I]->DizText.GetLength()+1), lpDizText, Size, nullptr, nullptr);
						if (BytesCount && BytesCount-1)
						{
							if(Cache.Write(lpDizText, BytesCount-1))
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
						delete[] lpDizText;
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
			FileAttr=FILE_ATTRIBUTE_ARCHIVE|(Opt.Diz.SetHidden?FILE_ATTRIBUTE_HIDDEN:0);
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


bool DizList::AddDizText(const string& Name,const string& ShortName,const string& DizText)
{
	DeleteDiz(Name,ShortName);
	string strQuotedName = Name;
	QuoteSpaceOnly(strQuotedName);
	return AddRecord(FormatString()<<fmt::LeftAlign()<<fmt::MinWidth(Opt.Diz.StartPos>1?Opt.Diz.StartPos-2:0)<<strQuotedName<<L" "<<DizText);
}


bool DizList::CopyDiz(const string& Name, const string& ShortName, const string& DestName, const string& DestShortName, DizList *DestDiz)
{
	int TextPos;
	int iDizPos=GetDizPosEx(Name,ShortName,&TextPos);

	if (iDizPos==-1)
		return false;
	size_t DizPos = iDizPos;
	while (IsSpace(DizData[DizPos]->DizText[TextPos]))
		TextPos++;

	DestDiz->AddDizText(DestName,DestShortName,&DizData[DizPos]->DizText[TextPos]);

	while (++DizPos<DizCount)
	{
		if (*DizData[DizPos]->DizText && !IsSpace(DizData[DizPos]->DizText[0]))
			break;

		DestDiz->AddRecord(DizData[DizPos]->DizText);
	}

	return true;
}


void DizList::GetDizName(string &strDizName)
{
	strDizName = strDizFileName;
}
