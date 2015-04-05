/*
xlat.cpp

XLat - перекодировка

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

/*
[HKEY_CURRENT_USER\Software\Far2\XLat]
"Flags"=dword:00000001
"Layouts"="04090409;04190419"
"Table1"="№АВГДЕЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЯавгдезийклмнопрстуфхцчшщъыьэяёЁБЮ"
"Table2"="#FDULTPBQRKVYJGHCNEA{WXIO}SMZfdultpbqrkvyjghcnea[wxio]sm'z`~<>"
"Rules1"=",??&./б,ю.:^Ж:ж;;$\"@Э\""
"Rules2"="?,&?/.,б.ю^::Ж;ж$;@\"\"Э"
"Rules3"="^::ЖЖ^$;;жж$@\"\"ЭЭ@&??,,бб&/..юю/"
*/

#include "headers.hpp"
#pragma hdrstop

#include "config.hpp"
#include "xlat.hpp"
#include "console.hpp"
#include "configdb.hpp"
#include "strmix.hpp"

void xlat_initialize()
{
	auto& XLat = Global->Opt->XLat;
	std::vector<HKL> Layouts;

	// Инициализация XLat для русской раскладки qwerty<->йцукен
	if (std::any_of(ALL_CONST_RANGE(XLat.Table), std::mem_fn(&StringOption::empty)) ||
	    std::any_of(ALL_CONST_RANGE(XLat.Rules), std::mem_fn(&StringOption::empty)))
	{
		if (const auto Count = GetKeyboardLayoutList(0, nullptr))
		{
			Layouts.resize(Count);
			if (GetKeyboardLayoutList(static_cast<int>(Layouts.size()), Layouts.data()))
			{
				const WORD RussianLanguageId = MAKELANGID(LANG_RUSSIAN, SUBLANG_RUSSIAN_RUSSIA);
				if (std::any_of(CONST_RANGE(Layouts, i){ return LOWORD(i) == RussianLanguageId; }))
				{
					static const wchar_t* const Tables[] =
					{
						L"\x2116\x0410\x0412\x0413\x0414\x0415\x0417\x0418\x0419\x041a\x041b\x041c\x041d\x041e\x041f\x0420\x0421\x0422\x0423\x0424\x0425\x0426\x0427\x0428\x0429\x042a\x042b\x042c\x042f\x0430\x0432\x0433\x0434\x0435\x0437\x0438\x0439\x043a\x043b\x043c\x043d\x043e\x043f\x0440\x0441\x0442\x0443\x0444\x0445\x0446\x0447\x0448\x0449\x044a\x044b\x044c\x044d\x044f\x0451\x0401\x0411\x042e",
						L"#FDULTPBQRKVYJGHCNEA{WXIO}SMZfdultpbqrkvyjghcnea[wxio]sm'z`~<>",
					};

					static const wchar_t* const Rules[] =
					{
						L",??&./\x0431,\x044e.:^\x0416:\x0436;;$\"@\x042d\"",
						L"?,&?/.,\x0431.\x044e^::\x0416;\x0436$;@\"\"\x042d",
						L"^::\x0416\x0416^$;;\x0436\x0436$@\"\"\x042d\x042d@&??,,\x0431\x0431&/..\x044e\x044e/",
					};

					const auto SetIfEmpty = [](StringOption& opt, const wchar_t* table) { if (opt.empty()) opt = table; };

					for_each_2(ALL_RANGE(XLat.Table), Tables, SetIfEmpty);
					for_each_2(ALL_RANGE(XLat.Rules), Rules, SetIfEmpty);
				}
			}
		}

	}

	if (!XLat.strLayouts.empty())
	{
		size_t I = 0;
		std::vector<string> Strings;
		split(Strings, XLat.strLayouts, STLF_UNIQUE);
		FOR(const auto& i, Strings)
		{
			DWORD res = std::stoul(i, nullptr, 16);
			XLat.Layouts[I] = (HKL)(intptr_t)(HIWORD(res)? res : MAKELONG(res, res));
			++I;

			if (I >= ARRAYSIZE(XLat.Layouts))
				break;
		}

		if (I <= 1) // если указано меньше двух - "отключаем" эту
			XLat.Layouts[0] = 0;
	}
}

wchar_t* Xlat(wchar_t *Line, int StartPos, int EndPos, unsigned __int64 Flags)
{
	const auto& XLat = Global->Opt->XLat;

	int PreLang=2,CurLang=2; // unknown
	size_t LangCount[2]={};

	if (!Line || !*Line)
		return nullptr;

	int Length=StrLength(Line);
	EndPos=std::min(EndPos,Length);
	StartPos=std::max(StartPos,0);

	if (StartPos > EndPos || StartPos >= Length)
		return Line;

	if (XLat.Table[0].empty() || XLat.Table[1].empty())
		return Line;

	size_t MinLenTable=std::min(XLat.Table[0].size(),XLat.Table[1].size());
	string strLayoutName;
	bool ProcessLayoutName=false;
	StringOption RulesNamed;

	if ((Flags & XLAT_USEKEYBLAYOUTNAME) && Console().GetKeyboardLayoutName(strLayoutName))
	{
		/*
			Уточнение по поводу этого куска, чтобы потом не вспоминать ;-)
			Было сделано в 1.7 build 1585

		    Делаем именно то, что заказывали!

		    Т.е. если сейчас раскладка стоит английская, а мы шпарим по русски, то
		    если раньше 'б' и 'ю' (в результате малой статистики) конвертировались
		    как бог на душу положит... то теперь так, как должно быть.

		    Для проверки нужно у HKEY_CURRENT_USER\Software\Far2\XLat\Flags выставить
		    второй бит (считаем от нуля; 0x4) и дописать две переменных:

		    ; набирали по русски в английской раскладке
		    ; `ё~Ё[х{Х]ъ}Ъ;Ж:Ж'э"Э,б<Б.ю>Ю/.?,
		    XLat::"00000409"="`ё~Ё[х{Х]ъ}Ъ;Ж:Ж'э\"Э,б<Б.ю>Ю/.?,"

		    ; набирали по английски в русской раскладке
		    ; ё`Ё~х[Х{ъ]Ъ}Ж;Ж:э'Э"б,Б<ю.Ю>./,?
		    XLat::"00000419"="ё`Ё~х[Х{ъ]Ъ}Ж;Ж:э'Э\"б,Б<ю.Ю>./,?"

		    Здесь есть бага (хотя, багой и не назовешь...) -
		      конвертнули,
		      переключилась раскладка,
		      руками переключили раскладку,
		      снова конвертим и...
		*/
		string XlatRules;
		ConfigProvider().GeneralCfg()->GetValue(L"XLat", strLayoutName, XlatRules, L"");
		RulesNamed = XlatRules;
		if (!RulesNamed.empty())
			ProcessLayoutName=true;
	}

	// цикл по всей строке
	for (int j=StartPos; j < EndPos; j++)
	{
		wchar_t Chr = Line[j];
		wchar_t ChrOld = Line[j];
		// ChrOld - пред символ
		int IsChange=0;

		// цикл по просмотру Chr в таблицах
		// <=MinLenTable так как длина настоящая а начальный индекс 1
		for (size_t i=0; i <= MinLenTable; i++)
		{
			// символ из латиницы?
			if (Chr == XLat.Table[1][i])
			{
				Chr=XLat.Table[0][i];
				IsChange=1;
				CurLang=1; // pred - english
				LangCount[1]++;
				break;
			}
			// символ из русской?
			else if (Chr == XLat.Table[0][i])
			{
				Chr=XLat.Table[1][i];
				CurLang=0; // pred - russian
				LangCount[0]++;
				IsChange=1;
				break;
			}
		}

		if (!IsChange) // особые случаи...
		{
			if (ProcessLayoutName)
			{
				for (size_t i=0; i < RulesNamed.size(); i+=2)
				{
					if (Chr == RulesNamed[i])
					{
						Chr=RulesNamed[i+1];
						break;
					}
				}
			}
			else
			{
				PreLang=CurLang;

				if (LangCount[0] > LangCount[1])
					CurLang=0;
				else if (LangCount[0] < LangCount[1])
					CurLang=1;
				else
					CurLang=2;

				if (PreLang != CurLang)
					CurLang=PreLang;

				for (size_t i=0; i < XLat.Rules[CurLang].size(); i+=2)
				{
					if (ChrOld == XLat.Rules[CurLang][i])
					{
						Chr=XLat.Rules[CurLang][i+1];
						break;
					}
				}

#if 0

				// Если в таблице не найдено и таблица была Unknown...
				if (I >= XLat.Rules[CurLang][0] && CurLang == 2)
				{
					// ...смотрим сначала в первой таблице...
					for (I=1; I < XLat.Rules[0][0]; I+=2)
						if (ChrOld == (BYTE)XLat.Rules[0][I])
							break;

					for (J=1; J < XLat.Rules[1][0]; J+=2)
						if (ChrOld == (BYTE)XLat.Rules[1][J])
							break;

					if (I >= XLat.Rules[0][0])
						CurLang=1;

					if (J >= XLat.Rules[1][0])
						CurLang=0;

					if ()//???
					{
						Chr=(BYTE)XLat.Rules[CurLang][J+1];
					}
				}

#endif
			}
		}

		Line[j]=Chr;
	}

	// переключаем раскладку клавиатуры?
	if (Flags & XLAT_SWITCHKEYBLAYOUT)
	{
		if (const auto hWnd = Console().GetWindow())
		{
			HKL Next = nullptr;

			if (XLat.Layouts[0])
			{
				if (++XLat.CurrentLayout >= (int)ARRAYSIZE(XLat.Layouts) || !XLat.Layouts[XLat.CurrentLayout])
					XLat.CurrentLayout = 0;

				if (XLat.Layouts[XLat.CurrentLayout])
					Next = XLat.Layouts[XLat.CurrentLayout];
			}

			PostMessage(hWnd,WM_INPUTLANGCHANGEREQUEST, Next?0:INPUTLANGCHANGE_FORWARD, (LPARAM)Next);

			if (Flags & XLAT_SWITCHKEYBBEEP)
				MessageBeep(0);
		}
	}

	return Line;
}
