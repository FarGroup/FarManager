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
#include "lang.hpp"
#include "cvtname.hpp"
#include "exception.hpp"

struct subst_data
{
	struct
	{
		struct
		{
			string Name;
			string NameOnly;
			string* ListName;
		}
		Normal, Short;
		panel_ptr Panel;
	}
	This, Another;

	auto& Default()
	{
		return PassivePanel? Another : This;
	}

	string CmdDir;
	bool PreserveLFN;
	bool PassivePanel;
};

static int IsReplaceVariable(const wchar_t *str,int *scr = nullptr,
                             int *end = nullptr,
                             int *beg_scr_break = nullptr,
                             int *end_scr_break = nullptr,
                             int *beg_txt_break = nullptr,
                             int *end_txt_break = nullptr);


static int ReplaceVariables(const string& DlgTitle,string &strStr, subst_data& SubstData);

// Str=if exist !#!\!^!.! far:edit < diff -c -p "!#!\!^!.!" !\!.!

static const wchar_t *_SubstFileName(const wchar_t *CurStr, subst_data& SubstData, string &strOut)
{
	// рассмотрим переключатели активности/пассивности панели.
	if (starts_with(CurStr, L"!#"_sv))
	{
		CurStr+=2;
		SubstData.PassivePanel = true;
		return CurStr;
	}

	if (starts_with(CurStr, L"!^"_sv))
	{
		CurStr+=2;
		SubstData.PassivePanel = false;
		return CurStr;
	}

	// !! символ '!'
	if (starts_with(CurStr,L"!!"_sv) && CurStr[2] != L'?')
	{
		strOut += L'!';
		CurStr+=2;
		return CurStr;
	}

	// !.!      Длинное имя файла с расширением
	if (starts_with(CurStr, L"!.!"_sv) && CurStr[3] != L'?')
	{
		strOut += SubstData.Default().Normal.Name;
		CurStr+=3;
		return CurStr;
	}

	// !~       Короткое имя файла без расширения
	if (starts_with(CurStr, L"!~"_sv))
	{
		strOut += SubstData.Default().Short.NameOnly;
		CurStr+=2;
		return CurStr;
	}

	// !`  Длинное расширение файла без имени
	if (starts_with(CurStr, L"!`"_sv))
	{
		const wchar_t *Ext;

		if (CurStr[2] == L'~')
		{
			Ext=wcsrchr(SubstData.Default().Short.Name.c_str(), L'.');
			CurStr+=3;
		}
		else
		{
			Ext=wcsrchr(SubstData.Default().Normal.Name.c_str(), L'.');
			CurStr+=2;
		}

		if (Ext && *Ext)
			strOut += ++Ext;

		return CurStr;
	}

	// !& !&~  список файлов разделенных пробелом.
	if ((starts_with(CurStr, L"!&~"_sv) && CurStr[3] != L'?') ||
	        (starts_with(CurStr, L"!&"_sv) && CurStr[2] != L'?'))
	{
		const auto WPanel = SubstData.Default().Panel;
		int ShortN0=FALSE;
		int CntSkip=2;

		if (CurStr[2] == L'~')
		{
			ShortN0=TRUE;
			CntSkip++;
		}

		int First = TRUE;

		for (const auto& i: WPanel->enum_selected())
		{
			string Name;
			if (ShortN0)
			{
				Name = i.AlternateFileName;
			}
			else
			{
				// в список все же должно попасть имя в кавычках.
				Name = i.FileName;
				QuoteSpaceOnly(Name);
			}

			//Вот здесь фиг его знает - нужно/ненужно...
			//если будет нужно - раскомментируем :-)
			//if(i.Attributes & FILE_ATTRIBUTE_DIRECTORY)
			//	AddEndSlash(Name);

			// А нужен ли нам пробел в самом начале?
			if (First)
				First = FALSE;
			else
				strOut += L' ';

			strOut += Name;
		}

		CurStr+=CntSkip;
		return CurStr;
	}

	// !@  Имя файла, содержащего имена помеченных файлов
	// !$!      Имя файла, содержащего короткие имена помеченных файлов
	// Ниже идет совмещение кода для разбора как !@! так и !$!
	//Вообще-то (по исторической справедливости как бы) - в !$! нужно выбрасывать модификаторы Q и A
	// Но нафиг нада:)
	if (starts_with(CurStr, L"!@"_sv) || starts_with(CurStr, L"!$"_sv))
	{
		string *pListName;
		string *pAnotherListName;
		bool ShortN0 = false;

		if (CurStr[1] == L'$')
			ShortN0 = true;

		if (ShortN0)
		{
			pListName = SubstData.This.Short.ListName;
			pAnotherListName = SubstData.Another.Short.ListName;
		}
		else
		{
			pListName = SubstData.This.Normal.ListName;
			pAnotherListName = SubstData.Another.Normal.ListName;
		}

		if (const auto Ptr = wcschr(CurStr + 2, L'!'))
		{
			if (Ptr[1] != L'?')
			{
				const string Modifers(CurStr+2, static_cast<size_t>(Ptr-(CurStr+2)));

				if (pListName)
				{
					if (SubstData.PassivePanel && (!pAnotherListName->empty() || SubstData.Another.Panel->MakeListFile(*pAnotherListName, ShortN0, Modifers)))
					{
						if (ShortN0)
							*pAnotherListName = ConvertNameToShort(*pAnotherListName);

						strOut += *pAnotherListName;
					}

					if (!SubstData.PassivePanel && (!pListName->empty() || SubstData.This.Panel->MakeListFile(*pListName, ShortN0, Modifers)))
					{
						if (ShortN0)
							*pListName = ConvertNameToShort(*pListName);

						strOut += *pListName;
					}
				}
				else
				{
					append(strOut, CurStr, Modifers, L'!');
				}

				CurStr+=Ptr-CurStr+1;
				return CurStr;
			}
		}
	}

	// !-!      Короткое имя файла с расширением
	if (starts_with(CurStr, L"!-!"_sv) && CurStr[3] != L'?')
	{
		strOut += SubstData.Default().Short.Name;

		CurStr+=3;
		return CurStr;
	}

	// !+!      Аналогично !-!, но если длинное имя файла утеряно
	//          после выполнения команды, FAR восстановит его
	if (starts_with(CurStr, L"!+!"_sv) && CurStr[3] != L'?')
	{
		strOut += SubstData.Default().Short.Name;

		CurStr+=3;
		SubstData.PreserveLFN = true;
		return CurStr;
	}

	// !:       Текущий диск
	if (starts_with(CurStr, L"!:"_sv))
	{
		string strCurDir;

		if (SubstData.This.Normal.Name.size() > 1 && SubstData.This.Normal.Name[1]==L':')
			strCurDir = SubstData.This.Normal.Name;
		else if (SubstData.PassivePanel)
			strCurDir = SubstData.Another.Panel->GetCurDir();
		else
			strCurDir = SubstData.CmdDir;

		auto strRootDir = GetPathRoot(strCurDir);
		DeleteEndSlash(strRootDir);
		strOut += strRootDir;
		CurStr+=2;
		return CurStr;
	}

	// !\       Текущий путь
	// !/       Короткое имя текущего пути
	// Ниже идет совмещение кода для разбора как !\ так и !/
	if (
		starts_with(CurStr, L"!\\"_sv) ||
		starts_with(CurStr, L"!=\\"_sv) ||
		starts_with(CurStr, L"!/"_sv) ||
		starts_with(CurStr, L"!=/"_sv)
	)
	{
		string strCurDir;
		bool ShortN0 = false;
		int RealPath= CurStr[1]==L'='?1:0;

		if (CurStr[1] == L'/' || (RealPath && CurStr[2] == L'/'))
		{
			ShortN0 = true;
		}

		if (SubstData.PassivePanel)
			strCurDir = SubstData.Another.Panel->GetCurDir();
		else
			strCurDir = SubstData.CmdDir;

		if (RealPath)
		{
			MakePath(SubstData.PassivePanel? Global->CtrlObject->Cp()->PassivePanel() : Global->CtrlObject->Cp()->ActivePanel(), false, true, ShortN0, strCurDir);
		}

		if (ShortN0)
			strCurDir = ConvertNameToShort(strCurDir);

		AddEndSlash(strCurDir);

		CurStr+=2+RealPath;

		if (*CurStr==L'!')
		{
			if (SubstData.Default().Normal.Name.find_first_of(L"\\:") != string::npos)
				strCurDir.clear();
		}

		strOut +=  strCurDir;
		return CurStr;
	}

	// !?<title>?<init>!
	if (starts_with(CurStr, L"!?"_sv) && wcschr(CurStr + 2, L'!'))
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
		append(strOut, PointToName(SubstData.Default().Normal.NameOnly));
		CurStr++;
	}

	return CurStr;
}


/*
  SubstFileName()
  Преобразование метасимволов ассоциации файлов в реальные значения

*/
bool SubstFileName(const wchar_t *DlgTitle,
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
	for (auto& i: { pListName, pAnotherListName, pShortListName, pAnotherShortListName })
	{
		if (i)
			i->clear();
	}

	/* $ 19.06.2001 SVS
	  ВНИМАНИЕ! Для альтернативных метасимволов, не основанных на "!",
	  нужно будет либо убрать эту проверку либо изменить условие (последнее
	  предпочтительнее!)
	*/
	if (!contains(strStr, L'!'))
		return false;

	subst_data SubstData;
	SubstData.This.Normal.Name=Name;
	SubstData.This.Short.Name = ShortName;

	SubstData.This.Normal.ListName = pListName;
	SubstData.This.Short.ListName = pShortListName;

	SubstData.Another.Normal.ListName = pAnotherListName;
	SubstData.Another.Short.ListName = pAnotherShortListName;

	SubstData.CmdDir = CmdLineDir? CmdLineDir : Global->CtrlObject->CmdLine()->GetCurDir();

	const auto& GetNameOnly = [](const string& Str)
	{
		return Str.substr(0, Str.rfind(L'.'));
	};

	// Предварительно получим некоторые "константы" :-)
	SubstData.This.Normal.NameOnly = GetNameOnly(Name);
	SubstData.This.Short.NameOnly = GetNameOnly(ShortName);

	SubstData.This.Panel = Global->CtrlObject->Cp()->ActivePanel();
	SubstData.Another.Panel = Global->CtrlObject->Cp()->PassivePanel();

	SubstData.Another.Panel->GetCurName(SubstData.Another.Normal.Name, SubstData.Another.Short.Name);

	SubstData.Another.Normal.NameOnly = GetNameOnly(SubstData.Another.Normal.Name);
	SubstData.Another.Short.NameOnly = GetNameOnly(SubstData.Another.Short.Name);

	SubstData.PreserveLFN = false;
	SubstData.PassivePanel = false; // первоначально речь идет про активную панель!

	auto CurStr = strStr.c_str();
	string strOut;

	while (*CurStr)
	{
		if (*CurStr == L'!')
		{
			CurStr=_SubstFileName(CurStr, SubstData, strOut);
		}
		else
		{
			strOut += *CurStr;
			CurStr++;
		}
	}
	strStr = strOut;

	if (!IgnoreInput)
	{
		ReplaceVariables(os::env::expand(NullToEmpty(DlgTitle)), strStr, SubstData);
	}

	return SubstData.PreserveLFN;
}

int ReplaceVariables(const string& DlgTitle, string &strStr, subst_data& SubstData)
{
	auto Str=strStr.c_str();
	const auto StartStr = Str;

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
		Item.strData = DlgTitle;
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
			Item.strHistory = L"UserVar" + str((DlgData.size() - 1) / 2);
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
				const auto p = strTitle.c_str() + 1;
				auto p1 = wcschr(p, L'$');

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
			const auto strTitle2 = strTitle.substr(end_t - 2 + 1 - hist_correct, scr - end_t - 1);

			// !?$zz$xxxx(ffffff)ddddd
			//            ^    ^
			const auto strTitle3 = strTitle.substr(beg_t - 2 + 1 - hist_correct, end_t - beg_t - 1);

			// !?$zz$xxxx(fffff)ddddd
			//       ^  ^
			strTitle.resize(beg_t - 2 - hist_correct);

			string strTmp;
			auto CurStr = strTitle3.c_str();

			while (*CurStr)
			{
				if (*CurStr == L'!')
				{
					CurStr=_SubstFileName(CurStr, SubstData, strTmp);
				}
				else
				{
					strTmp += *CurStr;
					CurStr++;
				}
			}

			append(strTitle, strTmp, strTitle2);
		}

		//do it - типа здесь все уже раскрыто и преобразовано
		DlgData[DlgData.size() - 2].strData = os::env::expand(strTitle);

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
			const auto strTxt2 = strTxt.substr(end_s - scr, end - end_s - 1);

			// !?$zz$xxxx(ffffff)ddddd?rrrr(pppp)qqqqq!
			//                              ^  ^
			const auto strTxt3 = strTxt.substr(beg_s - scr, end_s - beg_s - 1);

			// !?$zz$xxxx(fffff)ddddd?rrrr(pppp)qqqqq!
			//                        ^  ^
			strTxt.resize(beg_s - scr - 1);

			string strTmp;
			auto CurStr = strTxt3.c_str();

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

			append(strTxt, strTmp, strTxt2);
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
		Item.strData = msg(lng::MOk);
		DlgData.emplace_back(Item);

		Item.strData = msg(lng::MCancel);
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
			strTmpStr += *Str;
		}
	}

	strStr = os::env::expand(strTmpStr);
	return 1;
}

bool Panel::MakeListFile(string &strListFileName,bool ShortNames,const string& Modifers)
{
	uintptr_t CodePage = CP_OEMCP;

	if (!Modifers.empty())
	{
		if (contains(Modifers, L'A')) // ANSI
		{
			CodePage = CP_ACP;
		}
		else if (contains(Modifers, L'U')) // UTF8
		{
			CodePage = CP_UTF8;
		}
		else if (contains(Modifers, L'W')) // UTF16LE
		{
			CodePage = CP_UNICODE;
		}
	}

	const auto& transform = [&](string& strFileName)
	{
		if (!Modifers.empty())
		{
			if (contains(Modifers, L'F') && PointToName(strFileName).size() == strFileName.size()) // 'F' - использовать полный путь; //BUGBUG ?
			{
				strFileName = path::join(ShortNames? ConvertNameToShort(m_CurDir) : m_CurDir, strFileName); //BUGBUG ?
			}

			if (contains(Modifers, L'Q')) // 'Q' - заключать имена с пробелами в кавычки;
				QuoteSpaceOnly(strFileName);

			if (contains(Modifers, L'S')) // 'S' - использовать '/' вместо '\' в путях файлов;
			{
				ReplaceBackslashToSlash(strFileName);
			}
		}
	};


	try
	{
		if (!FarMkTempEx(strListFileName))
			throw MAKE_FAR_EXCEPTION(msg(lng::MCannotCreateListTemp));

		if (const auto ListFile = os::fs::file(strListFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, CREATE_ALWAYS))
		{
			os::fs::filebuf StreamBuffer(ListFile, std::ios::out);
			std::ostream Stream(&StreamBuffer);
			Stream.exceptions(Stream.badbit | Stream.failbit);
			encoding::writer Writer(Stream, CodePage);

			for (const auto& i: enum_selected())
			{
				auto Name = ShortNames? i.AlternateFileName : i.FileName;

				transform(Name);

				Writer.write(Name);
				Writer.write(L"\r\n"_sv);
			}
			
			Stream.flush();
		}
		else
		{
			throw MAKE_FAR_EXCEPTION(msg(lng::MCannotCreateListTemp));
		}

		return true;
	}
	catch (const far_exception& e)
	{
		os::fs::delete_file(strListFileName);
		Message(MSG_WARNING, e.get_error_state(),
			msg(lng::MError),
			{
				msg(lng::MCannotCreateListFile),
				e.get_message()
			},
			{ lng::MOk });
		return false;
	}
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
	const wchar_t* s = str;
	int BracketsCount = 0;
	int SecondBracketsCount = 0;
	bool Question = false;      //  ?
	bool Exclamation = false;      //  !
	bool Bracket1 = false;
	const wchar_t* BeginBracket1 = nullptr;
	const wchar_t* EndBracket1 = nullptr;
	bool Bracket2 = false;
	const wchar_t* BeginBracket2 = nullptr;
	const wchar_t* EndBracket2 = nullptr;

	if (!s)
		return -1;

	if (starts_with(s, L"!?"_sv))
		s = s + 2;
	else
		return -1;

	//
	for (;;)   // analyse from !? to ?
	{
		if (!*s)
			return -1;

		if (*s == L'(')
		{
			if (Bracket1)
			{
				//return -1;
			}
			else
			{
				Bracket1 = true;
				BeginBracket1 = s;     //remember where is first break
			}

			BracketsCount += 1;
		}
		else if (*s == L')')
		{
			BracketsCount -= 1;

			if (!BracketsCount)
			{
				if (!EndBracket1)
					EndBracket1 = s;   //remember where is last break
			}
			else if (BracketsCount < 0)
				return -1;
		}
		else if (*s == L'?' && !!BeginBracket1 == !!EndBracket1)
		{
			Question = true;
		}

		s++;

		if (Question) break;
	}

	if (BracketsCount) return -1;

	const auto scrtxt = s - 1; //remember s for return

	for (;;)   //analyse from ? or !
	{
		if (!*s)
			return -1;

		if (*s == L'(')
		{
			if (Bracket2)
			{
				//return -1;
			}
			else
			{
				Bracket2 = true;
				BeginBracket2 = s;    //remember where is first break
			}

			SecondBracketsCount += 1;
		}
		else if (*s == L')')
		{
			SecondBracketsCount -= 1;

			if (!SecondBracketsCount)
			{
				if (!EndBracket2)
					EndBracket2 = s;  //remember where is last break
			}
			else if (SecondBracketsCount < 0)
				return -1;
		}
		else if (*s == L'!' && !!BeginBracket2 == !!EndBracket2)
		{
			Exclamation = true;
		}

		s++;

		if (Exclamation) break;
	}

	if (SecondBracketsCount) return -1;

	//
	if (scr )
		*scr = (int)(scrtxt - str);

	if (end )
		*end = (int)(s - str) - 1;

	if (Bracket1)
	{
		if (beg_scr_break )
			*beg_scr_break = (int)(BeginBracket1 - str);

		if (end_scr_break )
			*end_scr_break = (int)(EndBracket1 - str);
	}
	else
	{
		if (beg_scr_break )
			*beg_scr_break = -1;

		if (end_scr_break )
			*end_scr_break = -1;
	}

	if (Bracket2)
	{
		if (beg_txt_break )
			*beg_txt_break = (int)(BeginBracket2 - str);

		if (end_txt_break )
			*end_txt_break = (int)(EndBracket2 - str);
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
