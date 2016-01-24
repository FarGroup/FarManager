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

#include "headers.hpp"
#pragma hdrstop

#include "fnparce.hpp"
#include "panel.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "flink.hpp"
#include "cmdline.hpp"
#include "filepanels.hpp"
#include "dialog.hpp"
#include "DlgGuid.hpp"
#include "message.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "panelmix.hpp"
#include "mix.hpp"
#include "language.hpp"

struct TSubstData
{
	// параметры функции SubstFileName
	const wchar_t *Name;           // Длинное имя
	const wchar_t *ShortName;      // Короткое имя

	string *pListName;
	string *pAnotherListName;

	string *pShortListName;
	string *pAnotherShortListName;

	// локальные переменные
	string strAnotherName;
	string strAnotherShortName;
	string strNameOnly;
	string strShortNameOnly;
	string strAnotherNameOnly;
	string strAnotherShortNameOnly;
	string strCmdDir;
	int  PreserveLFN;
	int  PassivePanel;

	panel_ptr AnotherPanel;
	panel_ptr ActivePanel;
};


static int IsReplaceVariable(const wchar_t *str,int *scr = nullptr,
                             int *end = nullptr,
                             int *beg_scr_break = nullptr,
                             int *end_scr_break = nullptr,
                             int *beg_txt_break = nullptr,
                             int *end_txt_break = nullptr);


static int ReplaceVariables(const wchar_t *DlgTitle,string &strStr, TSubstData& SubstData);

// Str=if exist !#!\!^!.! far:edit < diff -c -p "!#!\!^!.!" !\!.!

static const wchar_t *_SubstFileName(const wchar_t *CurStr, TSubstData& SubstData, string &strOut)
{
	// рассмотрим переключатели активности/пассивности панели.
	if (!StrCmpN(CurStr,L"!#",2))
	{
		CurStr+=2;
		SubstData.PassivePanel=TRUE;
		return CurStr;
	}

	if (!StrCmpN(CurStr,L"!^",2))
	{
		CurStr+=2;
		SubstData.PassivePanel=FALSE;
		return CurStr;
	}

	// !! символ '!'
	if (!StrCmpN(CurStr,L"!!",2) && CurStr[2] != L'?')
	{
		strOut += L"!";
		CurStr+=2;
		return CurStr;
	}

	// !.!      Длинное имя файла с расширением
	if (!StrCmpN(CurStr,L"!.!",3) && CurStr[3] != L'?')
	{
		if (SubstData.PassivePanel)
			strOut += SubstData.strAnotherName;
		else
			strOut += SubstData.Name;

		CurStr+=3;
		return CurStr;
	}

	// !~       Короткое имя файла без расширения
	if (!StrCmpN(CurStr,L"!~",2))
	{
		strOut += SubstData.PassivePanel ? SubstData.strAnotherShortNameOnly : SubstData.strShortNameOnly;
		CurStr+=2;
		return CurStr;
	}

	// !`  Длинное расширение файла без имени
	if (!StrCmpN(CurStr,L"!`",2))
	{
		const wchar_t *Ext;

		if (CurStr[2] == L'~')
		{
			Ext=wcsrchr((SubstData.PassivePanel ? SubstData.strAnotherShortName.data():SubstData.ShortName),L'.');
			CurStr+=3;
		}
		else
		{
			Ext=wcsrchr((SubstData.PassivePanel ? SubstData.strAnotherName.data():SubstData.Name),L'.');
			CurStr+=2;
		}

		if (Ext && *Ext)
			strOut += ++Ext;

		return CurStr;
	}

	// !& !&~  список файлов разделенных пробелом.
	if ((!StrCmpN(CurStr,L"!&~",3) && CurStr[3] != L'?') ||
	        (!StrCmpN(CurStr,L"!&",2) && CurStr[2] != L'?'))
	{
		string strFileNameL, strShortNameL;
		const auto WPanel = SubstData.PassivePanel?SubstData.AnotherPanel:SubstData.ActivePanel;
		DWORD FileAttrL;
		int ShortN0=FALSE;
		int CntSkip=2;

		if (CurStr[2] == L'~')
		{
			ShortN0=TRUE;
			CntSkip++;
		}

		WPanel->GetSelName(nullptr,FileAttrL);
		int First = TRUE;

		while (WPanel->GetSelName(&strFileNameL,FileAttrL,&strShortNameL))
		{
			if (ShortN0)
				strFileNameL = strShortNameL;
			else // в список все же должно попасть имя в кавычках.
				QuoteSpaceOnly(strFileNameL);

// Вот здесь фиг его знает - нужно/ненужно...
//   если будет нужно - раскомментируем :-)
//          if(FileAttrL & FILE_ATTRIBUTE_DIRECTORY)
//            AddEndSlash(FileNameL);
			// А нужен ли нам пробел в самом начале?
			if (First)
				First = FALSE;
			else
				strOut += L" ";

			strOut += strFileNameL;
		}

		CurStr+=CntSkip;
		return CurStr;
	}

	// !@  Имя файла, содержащего имена помеченных файлов
	// !$!      Имя файла, содержащего короткие имена помеченных файлов
	// Ниже идет совмещение кода для разбора как !@! так и !$!
	//Вообще-то (по исторической справедливости как бы) - в !$! нужно выбрасывать модификаторы Q и A
	// Но нафиг нада:)
	if (!StrCmpN(CurStr,L"!@",2) || !StrCmpN(CurStr,L"!$",2))
	{
		string *pListName;
		string *pAnotherListName;
		bool ShortN0 = false;

		if (CurStr[1] == L'$')
			ShortN0 = true;

		if (ShortN0)
		{
			pListName = SubstData.pShortListName;
			pAnotherListName = SubstData.pAnotherShortListName;
		}
		else
		{
			pListName = SubstData.pListName;
			pAnotherListName = SubstData.pAnotherListName;
		}

		if (const auto Ptr = wcschr(CurStr + 2, L'!'))
		{
			if (Ptr[1] != L'?')
			{
				const string Modifers(CurStr+2, static_cast<size_t>(Ptr-(CurStr+2)));

				if (pListName)
				{
					if (SubstData.PassivePanel && (!pAnotherListName->empty() || SubstData.AnotherPanel->MakeListFile(*pAnotherListName, ShortN0, Modifers)))
					{
						if (ShortN0)
							ConvertNameToShort(*pAnotherListName, *pAnotherListName);

						strOut += *pAnotherListName;
					}

					if (!SubstData.PassivePanel && (!pListName->empty() || SubstData.ActivePanel->MakeListFile(*pListName, ShortN0, Modifers)))
					{
						if (ShortN0)
							ConvertNameToShort(*pListName,*pListName);

						strOut += *pListName;
					}
				}
				else
				{
					strOut += CurStr;
					strOut += Modifers;
					strOut += L"!";
				}

				CurStr+=Ptr-CurStr+1;
				return CurStr;
			}
		}
	}

	// !-!      Короткое имя файла с расширением
	if (!StrCmpN(CurStr,L"!-!",3) && CurStr[3] != L'?')
	{
		if (SubstData.PassivePanel)
			strOut += SubstData.strAnotherShortName;
		else
			strOut += SubstData.ShortName;

		CurStr+=3;
		return CurStr;
	}

	// !+!      Аналогично !-!, но если длинное имя файла утеряно
	//          после выполнения команды, FAR восстановит его
	if (!StrCmpN(CurStr,L"!+!",3) && CurStr[3] != L'?')
	{
		if (SubstData.PassivePanel)
			strOut += SubstData.strAnotherShortName;
		else
			strOut += SubstData.ShortName;

		CurStr+=3;
		SubstData.PreserveLFN=TRUE;
		return CurStr;
	}

	// !:       Текущий диск
	if (!StrCmpN(CurStr,L"!:",2))
	{
		string strCurDir;
		string strRootDir;

		if (*SubstData.Name && SubstData.Name[1]==L':')
			strCurDir = SubstData.Name;
		else if (SubstData.PassivePanel)
			strCurDir = SubstData.AnotherPanel->GetCurDir();
		else
			strCurDir = SubstData.strCmdDir;

		GetPathRoot(strCurDir,strRootDir);
		DeleteEndSlash(strRootDir);
		strOut += strRootDir;
		CurStr+=2;
		return CurStr;
	}

	// !\       Текущий путь
	// !/       Короткое имя текущего пути
	// Ниже идет совмещение кода для разбора как !\ так и !/
	if (!StrCmpN(CurStr,L"!\\",2) || !StrCmpN(CurStr,L"!=\\",3) || !StrCmpN(CurStr,L"!/",2) || !StrCmpN(CurStr,L"!=/",3))
	{
		string strCurDir;
		bool ShortN0 = false;
		int RealPath= CurStr[1]==L'='?1:0;

		if (CurStr[1] == L'/' || (RealPath && CurStr[2] == L'/'))
		{
			ShortN0 = true;
		}

		if (SubstData.PassivePanel)
			strCurDir = SubstData.AnotherPanel->GetCurDir();
		else
			strCurDir = SubstData.strCmdDir;

		if (RealPath)
		{
			_MakePath1(SubstData.PassivePanel?KEY_ALTSHIFTBACKBRACKET:KEY_ALTSHIFTBRACKET,strCurDir,L"",ShortN0);
			Unquote(strCurDir);
		}

		if (ShortN0)
			ConvertNameToShort(strCurDir,strCurDir);

		AddEndSlash(strCurDir);

		CurStr+=2+RealPath;

		if (*CurStr==L'!')
		{
			if (wcspbrk(SubstData.PassivePanel?SubstData.strAnotherName.data():SubstData.Name,L"\\:"))
				strCurDir.clear();
		}

		strOut +=  strCurDir;
		return CurStr;
	}

	// !?<title>?<init>!
	if (!StrCmpN(CurStr,L"!?",2) && wcschr(CurStr+2,L'!'))
	{
		int j;
		int i = IsReplaceVariable(CurStr);

		if (i == -1)  // if bad format string
		{             // skip 1 char
			j = 1;
		}
		else
		{
			j = i + 1;
		}

		strOut.append(CurStr, j);
		CurStr += j;
		return CurStr;
	}

	// !        Длинное имя файла без расширения
	if (*CurStr==L'!')
	{
		strOut += PointToName(SubstData.PassivePanel ? SubstData.strAnotherNameOnly : SubstData.strNameOnly);
		CurStr++;
	}

	return CurStr;
}


/*
  SubstFileName()
  Преобразование метасимволов ассоциации файлов в реальные значения

*/
int SubstFileName(const wchar_t *DlgTitle,
                  string &strStr,            // результирующая строка
                  const string& Name,           // Длинное имя
                  const string& ShortName,      // Короткое имя

                  string *pListName,
                  string *pAnotherListName,
                  string *pShortListName,
                  string *pAnotherShortListName,
                  int   IgnoreInput,    // TRUE - не исполнять "!?<title>?<init>!"
                  const wchar_t *CmdLineDir)     // Каталог исполнения
{
	string* Lists[] = { pListName, pAnotherListName, pShortListName, pAnotherShortListName };
	FOR(auto& i, Lists)
	{
		if (i)
			i->clear();
	}

	/* $ 19.06.2001 SVS
	  ВНИМАНИЕ! Для альтернативных метасимволов, не основанных на "!",
	  нужно будет либо убрать эту проверку либо изменить условие (последнее
	  предпочтительнее!)
	*/
	if (strStr.find(L'!') == string::npos)
		return FALSE;

	TSubstData SubstData;
	SubstData.Name=Name.data();                    // Длинное имя
	SubstData.ShortName=ShortName.data();          // Короткое имя
	SubstData.pListName=pListName;            // Длинное имя файла-списка
	SubstData.pAnotherListName=pAnotherListName;            // Длинное имя файла-списка
	SubstData.pShortListName=pShortListName;  // Короткое имя файла-списка
	SubstData.pAnotherShortListName=pAnotherShortListName;  // Короткое имя файла-списка
	// Если имя текущего каталога не задано...
	if (CmdLineDir)
		SubstData.strCmdDir = CmdLineDir;
	else // ...спросим у ком.строки
		SubstData.strCmdDir = Global->CtrlObject->CmdLine()->GetCurDir();

	// Предварительно получим некоторые "константы" :-)
	SubstData.strNameOnly = Name;

	size_t pos = SubstData.strNameOnly.rfind(L'.');
	if (pos != string::npos)
		SubstData.strNameOnly.resize(pos);

	SubstData.strShortNameOnly = ShortName;

	if ((pos = SubstData.strShortNameOnly.rfind(L'.')) != string::npos)
		SubstData.strShortNameOnly.resize(pos);

	SubstData.ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
	SubstData.AnotherPanel=Global->CtrlObject->Cp()->PassivePanel();
	SubstData.AnotherPanel->GetCurName(SubstData.strAnotherName,SubstData.strAnotherShortName);
	SubstData.strAnotherNameOnly = SubstData.strAnotherName;

	if ((pos = SubstData.strAnotherNameOnly.rfind(L'.')) != string::npos)
		SubstData.strAnotherNameOnly.resize(pos);

	SubstData.strAnotherShortNameOnly = SubstData.strAnotherShortName;

	if ((pos = SubstData.strAnotherShortNameOnly.rfind(L'.')) != string::npos)
		SubstData.strAnotherShortNameOnly.resize(pos);

	SubstData.PreserveLFN=FALSE;
	SubstData.PassivePanel=FALSE; // первоначально речь идет про активную панель!

	const wchar_t *CurStr = strStr.data();
	string strOut;

	while (*CurStr)
	{
		if (*CurStr == L'!')
		{
			CurStr=_SubstFileName(CurStr, SubstData, strOut);
		}
		else
		{
			strOut.append(CurStr,1);
			CurStr++;
		}
	}
	strStr = strOut;


	if (!IgnoreInput)
	{
		string title = NullToEmpty(DlgTitle);
		ReplaceVariables(os::env::expand_strings(title).data(), strStr, SubstData);
	}

	return SubstData.PreserveLFN;
}

int ReplaceVariables(const wchar_t *DlgTitle, string &strStr, TSubstData& SubstData)
{
	const wchar_t *Str=strStr.data();
	const wchar_t * const StartStr=Str;

	// TODO: use DialogBuilder

	std::vector<DialogItemEx> DlgData;
	DlgData.reserve(30);

	struct pos_item
	{
		ptrdiff_t Pos;
		ptrdiff_t EndPos;
	};
	std::vector<pos_item> Positions;
	Positions.reserve(128);

	{
		DialogItemEx Item;
		Item.Type = DI_DOUBLEBOX;
		Item.X1 = 3;
		Item.Y1 = 1;
		Item.X2 = 72;
		Item.strData = NullToEmpty(DlgTitle);
		DlgData.emplace_back(Item);
	}

	while (*Str)
	{
		if (*(Str++)!=L'!')
			continue;

		if (!*Str)
			break;

		if (*(Str++)!=L'?')
			continue;

		if (!*Str)
			break;

		// теперича все не просто
		// придется сразу определить наличие операторных скобок
		// запомнить их позицию
		int scr,end, beg_t,end_t,beg_s,end_s;
		scr = end = beg_t = end_t = beg_s = end_s = 0;
		int ii = IsReplaceVariable(Str-2,&scr,&end,&beg_t,&end_t,&beg_s,&end_s);

		if (ii == -1)
		{
			strStr.clear();
			return 0;
		}

		{
			pos_item Item = {Str - StartStr - 2, Str - StartStr - 2 + ii};
			Positions.emplace_back(Item);
		}

		{
			DialogItemEx Item;
			Item.Type = DI_TEXT;
			Item.X1 = 5;
			Item.Y1 = Item.Y2 = DlgData.size() + 1;
			DlgData.emplace_back(Item);
		}

		{
			DialogItemEx Item;
			Item.Type = DI_EDIT;
			Item.X1 = 5;
			Item.X2 = 70;
			Item.Y1 = Item.Y2 = DlgData.size() + 1;
			Item.Flags = DIF_HISTORY | DIF_USELASTHISTORY;
			Item.strHistory = L"UserVar" + std::to_wstring((DlgData.size() - 1) / 2);
			DlgData.emplace_back(Item);
		}

		string strTitle;

		if (scr > 2)          // if between !? and ? exist some
			strTitle.assign(Str,scr-2);

		size_t hist_correct = 0;

		if (!strTitle.empty())
		{
			if (strTitle[0] == L'$')        // begin of history name
			{
				const wchar_t *p = strTitle.data() + 1;
				const wchar_t *p1 = wcschr(p,L'$');

				if (p1)
				{
					DlgData.back().strHistory.assign(p,p1-p);
					strTitle = ++p1;
					hist_correct = p1 - p + 1;
				}
			}
		}

		if ((beg_t == -1) || (end_t == -1))
		{
			//
		}
		else if ((end_t - beg_t) > 1) //if between ( and ) exist some
		{
			// !?$zz$xxxx(fffff)ddddd
			//                  ^   ^
			string strTitle2 = strTitle.substr(end_t - 2 + 1 - hist_correct, scr - end_t - 1);

			// !?$zz$xxxx(ffffff)ddddd
			//            ^    ^
			string strTitle3 = strTitle.substr(beg_t - 2 + 1 - hist_correct, end_t - beg_t - 1);

			// !?$zz$xxxx(fffff)ddddd
			//       ^  ^
			strTitle.resize(beg_t - 2 - hist_correct);

			string strTmp;
			const wchar_t *CurStr = strTitle3.data();

			while (*CurStr)
			{
				if (*CurStr == L'!')
				{
					CurStr=_SubstFileName(CurStr, SubstData, strTmp);
				}
				else
				{
					strTmp.append(CurStr,1);
					CurStr++;
				}
			}

			strTitle += strTmp;
			strTitle += strTitle2;
		}

		//do it - типа здесь все уже раскрыто и преобразовано
		DlgData[DlgData.size() - 2].strData = os::env::expand_strings(strTitle);

		// Заполняем поле ввода заданным шаблоном - если есть
		string strTxt;

		if ((end-scr) > 1)  //if between ? and ! exist some
			strTxt.append((Str-2)+scr+1,(end-scr)-1);

		if ((beg_s == -1) || (end_s == -1))
		{
			//
		}
		else if ((end_s - beg_s) > 1) //if between ( and ) exist some
		{
			// !?$zz$xxxx(fffff)ddddd?rrrr(pppp)qqqqq!
			//                                  ^   ^
			string strTxt2 = strTxt.substr(end_s - scr, end - end_s - 1);

			// !?$zz$xxxx(ffffff)ddddd?rrrr(pppp)qqqqq!
			//                              ^  ^
			string strTxt3 = strTxt.substr(beg_s - scr, end_s - beg_s - 1);

			// !?$zz$xxxx(fffff)ddddd?rrrr(pppp)qqqqq!
			//                        ^  ^
			strTxt.resize(beg_s - scr - 1);

			string strTmp;
			const wchar_t *CurStr = strTxt3.data();

			while (*CurStr)
			{
				if (*CurStr == L'!')
				{
					CurStr = _SubstFileName(CurStr, SubstData, strTmp);
				}
				else
				{
					strTmp.push_back(*CurStr);
					CurStr++;
				}
			}

			strTxt += strTmp;
			strTxt += strTxt2;
		}

		DlgData.back().strData = strTxt;
	}

	if (DlgData.size() <= 1)
	{
		return 0;
	}

	{
		DialogItemEx Item;
		Item.Type = DI_TEXT;
		Item.Flags = DIF_SEPARATOR;
		Item.Y1 = Item.Y2 = DlgData.size() + 1;
		DlgData.emplace_back(Item);
	}

	{
		DialogItemEx Item;
		Item.Type = DI_BUTTON;
		Item.Flags = DIF_DEFAULTBUTTON|DIF_CENTERGROUP;
		Item.Y1 = Item.Y2 = DlgData.size() + 1;
		Item.strData = MSG(MOk);
		DlgData.emplace_back(Item);

		Item.strData = MSG(MCancel);
		Item.Flags &= ~DIF_DEFAULTBUTTON;
		DlgData.emplace_back(Item);
	}

	// correct Dlg Title
	DlgData[0].Y2 = DlgData.size();

	int ExitCode;
	{
		const auto Dlg = Dialog::create(DlgData);
		Dlg->SetPosition(-1, -1, 76, static_cast<int>(DlgData.size() + 2));
		Dlg->SetId(UserMenuUserInputId);
		Dlg->Process();
		ExitCode=Dlg->GetExitCode();
	}

	if (ExitCode < 0 || ExitCode == static_cast<int>(DlgData.size() - 1))
	{
		strStr.clear();
		return 0;
	}

	string strTmpStr;

	for (Str=StartStr; *Str; Str++)
	{
		const auto ItemIterator = std::find_if(CONST_RANGE(Positions, i) { return i.Pos == Str - StartStr; });
		if (ItemIterator != Positions.cend())
		{
			strTmpStr += DlgData[(ItemIterator - Positions.cbegin()) * 2 + 2].strData;
			Str = StartStr + ItemIterator->EndPos;
		}
		else
		{
			strTmpStr.append(Str,1);
		}
	}

	strStr = os::env::expand_strings(strTmpStr);
	return 1;
}

bool Panel::MakeListFile(string &strListFileName,bool ShortNames,const string& Modifers)
{
	bool Ret=false;

	if (FarMkTempEx(strListFileName))
	{
		os::fs::file ListFile;
		if (ListFile.Open(strListFileName,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,CREATE_ALWAYS))
		{
			uintptr_t CodePage=CP_OEMCP;
			LPCVOID Eol="\r\n";
			DWORD EolSize=2;

			if (!Modifers.empty())
			{
				if (Modifers.find(L'A') != string::npos) // ANSI
				{
					CodePage=CP_ACP;
				}
				else
				{
					DWORD Signature=0;
					int SignatureSize=0;

					if (Modifers.find(L'W') != string::npos) // UTF16LE
					{
						CodePage=CP_UNICODE;
						Signature=SIGN_UNICODE;
						SignatureSize=2;
						Eol=WIN_EOL_fmt;
						EolSize=2*sizeof(WCHAR);
					}
					else
					{
						if (Modifers.find(L'U') != string::npos) // UTF8
						{
							CodePage=CP_UTF8;
							Signature=SIGN_UTF8;
							SignatureSize=3;
						}
					}

					if (Signature && SignatureSize)
					{
						size_t NumberOfBytesWritten;
						ListFile.Write(&Signature,SignatureSize, NumberOfBytesWritten);
					}
				}
			}

			string strFileName,strShortName;
			DWORD FileAttr;
			GetSelName(nullptr,FileAttr);

			while (GetSelName(&strFileName,FileAttr,&strShortName))
			{
				if (ShortNames)
					strFileName = strShortName;

				if (!Modifers.empty())
				{
					if (Modifers.find(L'F') != string::npos && PointToName(strFileName) == strFileName.data()) // 'F' - использовать полный путь; //BUGBUG ?
					{
						string strTempFileName(m_CurDir);

						if (ShortNames)
							ConvertNameToShort(strTempFileName,strTempFileName);

						AddEndSlash(strTempFileName);
						strTempFileName+=strFileName; //BUGBUG ?
						strFileName=strTempFileName;
					}

					if (Modifers.find(L'Q') != string::npos) // 'Q' - заключать имена с пробелами в кавычки;
						QuoteSpaceOnly(strFileName);

					if (Modifers.find(L'S') != string::npos) // 'S' - использовать '/' вместо '\' в путях файлов;
					{
						ReplaceBackslashToSlash(strFileName);
					}
				}

				blob Blob;
				std::string Buffer;

				if (CodePage==CP_UNICODE)
				{
					Blob = blob(strFileName.data(), strFileName.size() * sizeof(wchar_t));
				}
				else
				{
					Buffer = unicode::to(CodePage, strFileName.data(), strFileName.size());
					Blob = blob(Buffer.data(), Buffer.size());
				}

				size_t NumberOfBytesWritten = 0;
				if (ListFile.Write(Blob.data(), Blob.size(), NumberOfBytesWritten) && NumberOfBytesWritten == Blob.size())
				{
					if (ListFile.Write(Eol,EolSize,NumberOfBytesWritten) && NumberOfBytesWritten==EolSize)
					{
						Ret=true;
					}
				}
				else
				{
					Global->CatchError();
					Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotCreateListFile),MSG(MCannotCreateListWrite),MSG(MOk));
					os::DeleteFile(strListFileName);
					break;
				}
			}

			ListFile.Close();
		}
		else
		{
			Global->CatchError();
			Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotCreateListFile),MSG(MCannotCreateListTemp),MSG(MOk));
		}
	}
	else
	{
		Global->CatchError();
		Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotCreateListFile),MSG(MCannotCreateListTemp),MSG(MOk));
	}

	return Ret;
}

static int IsReplaceVariable(const wchar_t *str,
                             int *scr,
                             int *end,
                             int *beg_scr_break,
                             int *end_scr_break,
                             int *beg_txt_break,
                             int *end_txt_break)
// все очень сложно - последниe 4 указателя - это смещения от str
// начало скобок в строке описания, конец этих скобок, начало скобок в строке начального заполнения, ну и соотв конец.
// Вообще при простом вызове (который я собираюсь юзать) это выглядит просто:
// i = IsReplaceVariable(str) - ведь нам надо только проверять семантику скобок и всяких ?!
// где  i - тот прыжок, который надо совершить, чтоб прыгнуть на конец ! структуры !??!
{
	const wchar_t *s      = str;
	int count_scob = 0;
	int second_count_scob = 0;
	bool was_quest = false;         //  ?
	bool was_asterics = false;      //  !
	bool in_firstpart_was_scob = false;
	const wchar_t *beg_firstpart_scob = nullptr;
	const wchar_t *end_firstpart_scob = nullptr;
	bool in_secondpart_was_scob = false;
	const wchar_t *beg_secondpart_scob = nullptr;
	const wchar_t *end_secondpart_scob = nullptr;

	if (!s)
		return -1;

	if (!StrCmpN(s,L"!?",2))
		s = s + 2;
	else
		return -1;

	//
	for (;;)   // analize from !? to ?
	{
		if (!*s)
			return -1;

		if (*s == L'(')
		{
			if (in_firstpart_was_scob)
			{
				//return -1;
			}
			else
			{
				in_firstpart_was_scob = true;
				beg_firstpart_scob = s;     //remember where is first break
			}

			count_scob += 1;
		}
		else if (*s == L')')
		{
			count_scob -= 1;

			if (!count_scob)
			{
				if (!end_firstpart_scob)
					end_firstpart_scob = s;   //remember where is last break
			}
			else if (count_scob < 0)
				return -1;
		}
		else if ((*s == L'?') && ((!beg_firstpart_scob && !end_firstpart_scob) || (beg_firstpart_scob && end_firstpart_scob)))
		{
			was_quest = true;
		}

		s++;

		if (was_quest) break;
	}

	if (count_scob ) return -1;

	const auto scrtxt = s - 1; //remember s for return

	for (;;)   //analize from ? or !
	{
		if (!*s)
			return -1;

		if (*s == L'(')
		{
			if (in_secondpart_was_scob)
			{
				//return -1;
			}
			else
			{
				in_secondpart_was_scob = true;
				beg_secondpart_scob = s;    //remember where is first break
			}

			second_count_scob += 1;
		}
		else if (*s == L')')
		{
			second_count_scob -= 1;

			if (!second_count_scob)
			{
				if (!end_secondpart_scob)
					end_secondpart_scob = s;  //remember where is last break
			}
			else if (second_count_scob < 0)
				return -1;
		}
		else if ((*s == L'!') && ((!beg_secondpart_scob && !end_secondpart_scob) || (beg_secondpart_scob && end_secondpart_scob)))
		{
			was_asterics = true;
		}

		s++;

		if (was_asterics) break;
	}

	if (second_count_scob ) return -1;

	//
	if (scr )
		*scr = (int)(scrtxt - str);

	if (end )
		*end = (int)(s - str) - 1;

	if (in_firstpart_was_scob)
	{
		if (beg_scr_break )
			*beg_scr_break = (int)(beg_firstpart_scob - str);

		if (end_scr_break )
			*end_scr_break = (int)(end_firstpart_scob - str);
	}
	else
	{
		if (beg_scr_break )
			*beg_scr_break = -1;

		if (end_scr_break )
			*end_scr_break = -1;
	}

	if (in_secondpart_was_scob)
	{
		if (beg_txt_break )
			*beg_txt_break = (int)(beg_secondpart_scob - str);

		if (end_txt_break )
			*end_txt_break = (int)(end_secondpart_scob - str);
	}
	else
	{
		if (beg_txt_break )
			*beg_txt_break = -1;

		if (end_txt_break )
			*end_txt_break = -1;
	}

	return (int)((s - str) - 1);
}
