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
#include "scantree.hpp"
#include "vmenu.hpp"
#include "manager.hpp"
#include "message.hpp"
#include "config.hpp"
#include "strmix.hpp"
#include "filestr.hpp"
#include "interf.hpp"
#include "lasterror.hpp"

const wchar_t LangFileMask[] = L"*.lng";

Language Lang;
Language OldLang;

FILE* OpenLangFile(const wchar_t *Path,const wchar_t *Mask,const wchar_t *Language, string &strFileName, UINT &nCodePage, BOOL StrongLang,string *pstrLangName)
{
	strFileName.Clear();
	FILE *LangFile=nullptr;
	string strFullName, strEngFileName;
	FAR_FIND_DATA_EX FindData;
	string strLangName;
	ScanTree ScTree(FALSE,FALSE);
	ScTree.SetFindPath(Path,Mask);

	while (ScTree.GetNextName(&FindData, strFullName))
	{
		strFileName = strFullName;

		if (!Language)
			break;

		if (!(LangFile=_wfopen(strFileName,L"rb")))
		{
			strFileName.Clear();
		}
		else
		{
			OldGetFileFormat(LangFile, nCodePage, nullptr, false);

			if (GetLangParam(LangFile,L"Language",&strLangName,nullptr, nCodePage) && !StrCmpI(strLangName,Language))
				break;

			fclose(LangFile);
			LangFile=nullptr;

			if (StrongLang)
			{
				strFileName.Clear();
				strEngFileName.Clear();
				break;
			}

			if (!StrCmpI(strLangName,L"English"))
				strEngFileName = strFileName;
		}
	}

	if (!LangFile)
	{
		if (!strEngFileName.IsEmpty())
			strFileName = strEngFileName;

		if (!strFileName.IsEmpty())
		{
			LangFile=_wfopen(strFileName,L"rb");

			if (pstrLangName)
				*pstrLangName=strLangName;
		}
	}

	return(LangFile);
}


int GetLangParam(FILE *SrcFile,const wchar_t *ParamName,string *strParam1, string *strParam2, UINT nCodePage)
{
	wchar_t ReadStr[1024];
	string strFullParamName = L".";
	strFullParamName += ParamName;
	int Length=(int)strFullParamName.GetLength();
	/* $ 29.11.2001 DJ
	   не поганим позицию в файле; дальше @Contents не читаем
	*/
	BOOL Found = FALSE;
	long OldPos = ftell(SrcFile);

	while (ReadString(SrcFile, ReadStr, 1024, nCodePage))
	{
		if (!StrCmpNI(ReadStr,strFullParamName,Length))
		{
			wchar_t *Ptr=wcschr(ReadStr,L'=');

			if (Ptr)
			{
				*strParam1 = Ptr+1;

				if (strParam2)
					strParam2->Clear();

				size_t pos;

				if (strParam1->Pos(pos,L','))
				{
					if (strParam2)
					{
						*strParam2 = *strParam1;
						strParam2->LShift(pos+1);
						RemoveTrailingSpaces(*strParam2);
					}

					strParam1->SetLength(pos);
				}

				RemoveTrailingSpaces(*strParam1);
				Found = TRUE;
				break;
			}
		}
		else if (!StrCmpNI(ReadStr, L"@Contents", 9))
			break;
	}

	fseek(SrcFile,OldPos,SEEK_SET);
	return(Found);
}

bool Select(int HelpLanguage,VMenu **MenuPtr)
{
	const wchar_t *Title,*Mask;
	StringOption *strDest;

	if (HelpLanguage)
	{
		Title=MSG(MHelpLangTitle);
		Mask=HelpFileMask;
		strDest=&Opt.strHelpLanguage;
	}
	else
	{
		Title=MSG(MLangTitle);
		Mask=LangFileMask;
		strDest=&Opt.strLanguage;
	}

	MenuItemEx LangMenuItem;
	LangMenuItem.Clear();
	VMenu *LangMenu=new VMenu(Title,nullptr,0,ScrY-4);
	*MenuPtr=LangMenu;
	LangMenu->SetFlags(VMENU_WRAPMODE);
	LangMenu->SetPosition(ScrX/2-8+5*HelpLanguage,ScrY/2-4+2*HelpLanguage,0,0);
	string strFullName;
	FAR_FIND_DATA_EX FindData;
	ScanTree ScTree(FALSE,FALSE);
	ScTree.SetFindPath(g_strFarPath, Mask);

	while (ScTree.GetNextName(&FindData,strFullName))
	{
		FILE *LangFile=_wfopen(strFullName,L"rb");

		if (!LangFile)
			continue;

		UINT nCodePage=CP_OEMCP;
		OldGetFileFormat(LangFile, nCodePage, nullptr, false);
		string strLangName, strLangDescr;

		if (GetLangParam(LangFile,L"Language",&strLangName,&strLangDescr,nCodePage))
		{
			string strEntryName;

			if (!HelpLanguage || (!GetLangParam(LangFile,L"PluginContents",&strEntryName,nullptr,nCodePage) &&
			                      !GetLangParam(LangFile,L"DocumentContents",&strEntryName,nullptr,nCodePage)))
			{
				LangMenuItem.strName.Format(L"%.40s", !strLangDescr.IsEmpty() ? strLangDescr.CPtr():strLangName.CPtr());

				/* $ 01.08.2001 SVS
				   Не допускаем дубликатов!
				   Если в каталог с ФАРом положить еще один HLF с одноименным
				   языком, то... фигня получается при выборе языка.
				*/
				if (LangMenu->FindItem(0,LangMenuItem.strName,LIFIND_EXACTMATCH) == -1)
				{
					LangMenuItem.SetSelect(!StrCmpI(*strDest,strLangName));
					LangMenu->SetUserData(strLangName.CPtr(), (strLangName.GetLength()+1)*sizeof(wchar_t), LangMenu->AddItem(&LangMenuItem));
				}
			}
		}

		fclose(LangFile);
	}

	LangMenu->AssignHighlights(FALSE);
	LangMenu->Process();

	if (LangMenu->Modal::GetExitCode()<0)
		return false;

	*strDest = static_cast<const wchar_t*>(LangMenu->GetUserData(nullptr, 0));
	return true;
}

/* $ 01.09.2000 SVS
  + Новый метод, для получения параметров для .Options
   .Options <KeyName>=<Value>
*/
int GetOptionsParam(FILE *SrcFile,const wchar_t *KeyName,string &strValue, UINT nCodePage)
{
	wchar_t ReadStr[1024]={};
	string strFullParamName;
	int Length=StrLength(L".Options");
	long CurFilePos=ftell(SrcFile);

	while (ReadString(SrcFile, ReadStr, ARRAYSIZE(ReadStr), nCodePage) )
	{
		if (!StrCmpNI(ReadStr,L".Options",Length))
		{
			strFullParamName = RemoveExternalSpaces(ReadStr+Length);
			size_t pos;

			if (strFullParamName.RPos(pos,L'='))
			{
				strValue = strFullParamName;
				strValue.LShift(pos+1);
				RemoveExternalSpaces(strValue);
				strFullParamName.SetLength(pos);
				RemoveExternalSpaces(strFullParamName);

				if (!StrCmpI(strFullParamName,KeyName))
				{
					fseek(SrcFile,CurFilePos,SEEK_SET);
					return TRUE;
				}
			}
		}
	}

	fseek(SrcFile,CurFilePos,SEEK_SET);
	return FALSE;
}

Language::Language():
	MsgAddr(nullptr),
	MsgList(nullptr),
#ifndef NO_WRAPPER
	MsgAddrA(nullptr),
	MsgListA(nullptr),
#endif // NO_WRAPPER
	MsgCount(0),
	LastError(LERROR_SUCCESS),
#ifndef NO_WRAPPER
	m_bUnicode(true),
#endif // NO_WRAPPER
	LanguageLoaded(false)
{
}

void ConvertString(const wchar_t *Src,string &strDest)
{
	wchar_t *Dest = strDest.GetBuffer(wcslen(Src)*2);
	while (*Src)
	{
		switch (*Src)
		{
		case L'\\':
			switch (Src[1])
			{
			case L'\\':
				*(Dest++)=L'\\';
				Src+=2;
				break;
			case L'\"':
				*(Dest++)=L'\"';
				Src+=2;
				break;
			case L'n':
				*(Dest++)=L'\n';
				Src+=2;
				break;
			case L'r':
				*(Dest++)=L'\r';
				Src+=2;
				break;
			case L'b':
				*(Dest++)=L'\b';
				Src+=2;
				break;
			case L't':
				*(Dest++)=L'\t';
				Src+=2;
				break;
			default:
				*(Dest++)=L'\\';
				Src++;
				break;
			}
			break;
		case L'"':
			*(Dest++)=L'"';
			Src+=(Src[1]==L'"') ? 2:1;
			break;
		default:
			*(Dest++)=*(Src++);
			break;
		}

		*Dest=0;
		strDest.ReleaseBuffer();
	}
}

bool Language::Init(const wchar_t *Path, int CountNeed)
{
	if (MsgList
#ifndef NO_WRAPPER
	|| MsgListA
#endif // NO_WRAPPER
	)
		return true;
	GuardLastError gle;
	LastError = LERROR_SUCCESS;
	UINT nCodePage = CP_OEMCP;
	string strLangName=Opt.strLanguage.Get();
	FILE *LangFile=OpenLangFile(Path,LangFileMask,Opt.strLanguage,strMessageFile, nCodePage,FALSE, &strLangName);

	if (!LangFile)
	{
		LastError = LERROR_FILE_NOT_FOUND;
		return false;
	}
	if (this == &Lang && StrCmpI(Opt.strLanguage,strLangName))
		Opt.strLanguage=strLangName;

	long Pos = ftell(LangFile);
	fseek(LangFile, 0, SEEK_END);
	size_t FileSize = ftell(LangFile);
	fseek(LangFile, Pos, SEEK_SET);

#ifndef NO_WRAPPER
	if (!m_bUnicode)
	{
		MsgListA = static_cast<char*>(xf_malloc(FileSize));
	}
	else
#endif // NO_WRAPPER
	{
		MsgList = static_cast<wchar_t*>(xf_malloc(FileSize * sizeof(wchar_t)));
	}

	wchar_t ReadStr[1024]={};

	size_t MsgSize = 0;

	while (ReadString(LangFile, ReadStr, ARRAYSIZE(ReadStr), nCodePage) )
	{
		string strDestStr;
		RemoveExternalSpaces(ReadStr);

		if (*ReadStr != L'\"')
			continue;

		int SrcLength=StrLength(ReadStr);

		if (ReadStr[SrcLength-1]==L'\"')
			ReadStr[SrcLength-1]=0;

		ConvertString(ReadStr+1,strDestStr);
		size_t DestLength=strDestStr.GetLength()+1;

#ifndef NO_WRAPPER
		if (m_bUnicode)
#endif // NO_WRAPPER
		{
			wcscpy(MsgList+MsgSize, strDestStr);
		}
#ifndef NO_WRAPPER
		else
		{
			WideCharToMultiByte(CP_OEMCP, 0, strDestStr, -1, MsgListA+MsgSize, static_cast<int>(DestLength), nullptr, nullptr);
		}
#endif // NO_WRAPPER
		MsgSize+=DestLength;
		MsgCount++;
	}

	//   Проведем проверку на количество строк в LNG-файлах
	if (CountNeed != -1 && CountNeed != MsgCount-1)
	{
		fclose(LangFile);
		LastError = LERROR_BAD_FILE;
		return false;
	}

#ifndef NO_WRAPPER
	if (!m_bUnicode)
	{
		MsgListA = static_cast<char*>(xf_realloc(MsgListA, MsgSize));
	}
	else
#endif // NO_WRAPPER
	{
		MsgList = static_cast<wchar_t*>(xf_realloc(MsgList, MsgSize * sizeof(wchar_t)));
	}

#ifndef NO_WRAPPER
	if (m_bUnicode)
#endif // NO_WRAPPER
	{
		wchar_t *CurAddr = MsgList;
		MsgAddr = new wchar_t*[MsgCount];

		if (!MsgAddr)
		{
			fclose(LangFile);
			return false;
		}

		for (int I=0; I<MsgCount; I++)
		{
			MsgAddr[I]=CurAddr;
			CurAddr+=StrLength(CurAddr)+1;
		}
	}
#ifndef NO_WRAPPER
	else
	{
		char *CurAddrA = MsgListA;
		MsgAddrA = new char*[MsgCount];

		if (!MsgAddrA)
		{
			fclose(LangFile);
			return false;
		}

		for (int I=0; I<MsgCount; I++)
		{
			MsgAddrA[I]=CurAddrA;
			CurAddrA+=strlen(CurAddrA)+1;
		}
	}
#endif // NO_WRAPPER
	fclose(LangFile);

	if (this == &Lang)
		OldLang.Free();

	LanguageLoaded=true;
	return true;
}

#ifndef NO_WRAPPER
bool Language::InitA(const wchar_t *Path, int CountNeed)
{
	m_bUnicode = false;
	return Init(Path, CountNeed);
}
#endif // NO_WRAPPER

Language::~Language()
{
	Free();
}

void Language::Free()
{
	if (MsgList)
	{
		xf_free(MsgList);
		MsgList=nullptr;
	}
	if (MsgAddr)
	{
		delete[] MsgAddr;
		MsgAddr=nullptr;
	}
#ifndef NO_WRAPPER
	if (MsgListA)
	{
		xf_free(MsgListA);
		MsgListA=nullptr;
	}
	if (MsgAddrA)
	{
		delete[] MsgAddrA;
		MsgAddrA=nullptr;
	}
	m_bUnicode = true;
#endif // NO_WRAPPER

	MsgCount=0;
}

void Language::Close()
{
	if (this == &Lang)
	{
		if (OldLang.MsgCount)
			OldLang.Free();

		OldLang.MsgList=MsgList;
		OldLang.MsgAddr=MsgAddr;
#ifndef NO_WRAPPER
		OldLang.MsgListA=MsgListA;
		OldLang.MsgAddrA=MsgAddrA;
		OldLang.m_bUnicode=m_bUnicode;
#endif // NO_WRAPPER
		OldLang.MsgCount=MsgCount;
	}

	MsgList=nullptr;
	MsgAddr=nullptr;
#ifndef NO_WRAPPER
	MsgListA=nullptr;
	MsgAddrA=nullptr;
	m_bUnicode = true;
#endif // NO_WRAPPER
	MsgCount=0;
	LanguageLoaded=false;
}

bool Language::CheckMsgId(LNGID MsgId) const
{
	/* $ 19.03.2002 DJ
	   при отрицательном индексе - также покажем сообщение об ошибке
	   (все лучше, чем трапаться)
	*/
	if (MsgId>=MsgCount || MsgId < 0)
	{
		if (this == &Lang && !LanguageLoaded && this != &OldLang && OldLang.CheckMsgId(MsgId))
			return true;

		/* $ 26.03.2002 DJ
		   если менеджер уже в дауне - сообщение не выводим
		*/
		if (!FrameManager->ManagerIsDown())
		{
			/* $ 03.09.2000 IS
			   ! Нормальное сообщение об отсутствии строки в языковом файле
			     (раньше имя файла обрезалось справа и приходилось иногда гадать - в
			     каком же файле ошибка)
			*/
			string strMsg1(L"Incorrect or damaged ");
			strMsg1+=strMessageFile;
			/* IS $ */
			if (Message(MSG_WARNING, 2,
				L"Error",
				strMsg1,
				FormatString()<<L"Message "<<MsgId<<L" not found",
				L"Ok", L"Quit")==1)
				exit(0);
		}

		return false;
	}

	return true;
}

const wchar_t* Language::GetMsg(LNGID nID) const
{
	if (
#ifndef NO_WRAPPER
	!m_bUnicode ||
#endif // NO_WRAPPER
	!CheckMsgId(nID))
		return L"";

	if (this == &Lang && this != &OldLang && !LanguageLoaded && OldLang.MsgCount > 0)
		return OldLang.MsgAddr[nID];

	return MsgAddr[nID];
}

#ifndef NO_WRAPPER
const char* Language::GetMsgA(LNGID nID) const
{
	if (m_bUnicode || !CheckMsgId(nID))
		return "";

	if (this == &Lang && this != &OldLang && !LanguageLoaded && OldLang.MsgCount > 0)
		return OldLang.MsgAddrA[nID];

	return MsgAddrA[nID];
}
#endif // NO_WRAPPER
