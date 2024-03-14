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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "xlat.hpp"

// Internal:
#include "config.hpp"
#include "console.hpp"
#include "configdb.hpp"
#include "global.hpp"
#include "log.hpp"

// Platform:

// Common:
#include "common/enum_tokens.hpp"
#include "common/view/zip.hpp"

// External:

//----------------------------------------------------------------------------

void xlat_initialize()
{
	auto& XLat = Global->Opt->XLat;

	if (XLat.strLayouts.empty())
		return;

	size_t I = 0;
	for (const auto& i: enum_tokens(XLat.strLayouts.Get(), L";"sv))
	{
		if (i.empty())
			continue;

		if (const auto Hkl = os::make_hkl(i); Hkl)
		{
			XLat.Layouts[I] = Hkl;
			++I;

			if (I >= std::size(XLat.Layouts))
				break;
		}
		else
		{
			LOGWARNING(L"Unsupported layout: {}"sv, i);
		}
	}

	if (I < 2) // если указано меньше двух - "отключаем" эту
		XLat.Layouts[0] = nullptr;
}

void Xlat(std::span<wchar_t> const Data, unsigned long long const Flags)
{
	const auto& XLat = Global->Opt->XLat;

	int CurLang=2; // unknown
	size_t LangCount[2]{};

	if (Data.empty())
		return;

	if (XLat.Table[0].empty() || XLat.Table[1].empty())
		return;

	const auto MinLenTable = std::min(XLat.Table[0].size(), XLat.Table[1].size());
	bool ProcessLayoutName=false;
	StringOption RulesNamed;

	if ((Flags & XLAT_USEKEYBLAYOUTNAME))
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

		if (const auto Hkl = console.GetKeyboardLayout())
		{
			RulesNamed = ConfigProvider().GeneralCfg()->GetValue<string>(L"XLat"sv, far::format(L"{:08X}"sv, static_cast<int32_t>(std::bit_cast<intptr_t>(Hkl))));
			if (!RulesNamed.empty())
				ProcessLayoutName = true;
		}
	}

	// цикл по всей строке
	for (auto& Chr: Data)
	{
		const auto ChrOld = Chr;
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
				const auto PreLang=CurLang;

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
			}
		}
	}

	// переключаем раскладку клавиатуры?
	if (Flags & XLAT_SWITCHKEYBLAYOUT)
	{
		if (const auto hWnd = console.GetWindow())
		{
			HKL Next = nullptr;

			if (XLat.Layouts[0])
			{
				if (++XLat.CurrentLayout >= static_cast<int>(std::size(XLat.Layouts)) || !XLat.Layouts[XLat.CurrentLayout])
					XLat.CurrentLayout = 0;

				if (XLat.Layouts[XLat.CurrentLayout])
					Next = XLat.Layouts[XLat.CurrentLayout];
			}

			PostMessage(hWnd, WM_INPUTLANGCHANGEREQUEST, Next? 0 : INPUTLANGCHANGE_FORWARD, std::bit_cast<LPARAM>(Next));

			if (Flags & XLAT_SWITCHKEYBBEEP)
				MessageBeep(0);
		}
	}
}

void xlat_observe_tables(function_ref<void(wchar_t, wchar_t)> const Observer)
{
	const auto& XLat = Global->Opt->XLat;

	for (const auto& [Local, English]: zip(XLat.Table[0].Get(), XLat.Table[1].Get()))
	{
		Observer(Local, English);
	}
}
