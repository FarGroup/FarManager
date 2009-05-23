/*
xlat.cpp

XLat - перекодировка

*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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
"Table1"="єј¬√ƒ≈«»… ЋћЌќѕ–—“”‘’÷„ЎўЏџ№яавгдезийклмнопрстуфхцчшщъыьэ€Є®Ѕё"
"Table2"="#FDULTPBQRKVYJGHCNEA{WXIO}SMZfdultpbqrkvyjghcnea[wxio]sm'z`~<>"
"Rules1"=",??&./б,ю.:^∆:ж;;$;\"@Ё\""
"Rules2"="?,&?/.,б.ю^::∆;ж$;@\"\"Ё"
"Rules3"="^::∆∆^$;;жж$@\"\"ЁЁ@&??,,бб&/..юю/"
*/

#include "headers.hpp"
#pragma hdrstop

#include "registry.hpp"
#include "iswind.hpp"
#include "config.hpp"
#include "xlat.hpp"

wchar_t* WINAPI Xlat(wchar_t *Line,
                    int StartPos,
                    int EndPos,
                    DWORD Flags)
{
	wchar_t Chr,ChrOld;
	int PreLang=2,CurLang=2; // unknown
	int LangCount[2]={0,0};
	int IsChange=0;

	if (!Line || !*Line)
		return NULL;

	int Length=StrLength(Line);

	EndPos=Min(EndPos,Length);
	StartPos=Max(StartPos,0);

	if (StartPos > EndPos || StartPos >= Length)
		return Line;

	if(!Opt.XLat.Table[0].GetLength() || !Opt.XLat.Table[1].GetLength())
		return Line;

	size_t MinLenTable=Min(Opt.XLat.Table[0].GetLength(),Opt.XLat.Table[1].GetLength());

	string strLayoutName;

	int ProcessLayoutName=FALSE;
	if ((Flags & XLAT_USEKEYBLAYOUTNAME) && apiGetConsoleKeyboardLayoutName(strLayoutName))
	{
		GetRegKey(L"XLat",strLayoutName,Opt.XLat.Rules[2],L"");
		if(!Opt.XLat.Rules[2].IsEmpty())
			ProcessLayoutName=TRUE;
	}

	// цикл по всей строке
	for (int j=StartPos; j < EndPos; j++)
	{
		ChrOld=Chr=Line[j];
		// ChrOld - пред символ
		IsChange=0;
		// цикл по просмотру Chr в таблицах
		// <=MinLenTable так как длина насто€ща€ а начальный индекс 1
		for (size_t i=0; i <= MinLenTable; i++)
		{
			// символ из латиницы?
			if (Chr == Opt.XLat.Table[1].At(i))
			{
				Chr=Opt.XLat.Table[0].At(i);
				IsChange=1;
				CurLang=1; // pred - english
				LangCount[1]++;
				break;
			}
			// символ из русской?
			else if (Chr == Opt.XLat.Table[0].At(i))
			{
				Chr=Opt.XLat.Table[1].At(i);
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
				for (size_t i=0; i < Opt.XLat.Rules[2].GetLength(); i+=2)
				{
					if (Chr == Opt.XLat.Rules[2].At(i))
					{
						Chr=Opt.XLat.Rules[2].At(i+1);
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

				for (size_t i=0; i < Opt.XLat.Rules[CurLang].GetLength(); i+=2)
				{
					if (ChrOld == Opt.XLat.Rules[CurLang].At(i))
					{
						Chr=Opt.XLat.Rules[CurLang].At(i+1);
						break;
					}
				}
#if 0
				// ≈сли в таблице не найдено и таблица была Unknown...
				if(I >= Opt.XLat.Rules[CurLang][0] && CurLang == 2)
				{
					// ...смотрим сначала в первой таблице...
					for(I=1; I < Opt.XLat.Rules[0][0]; I+=2)
						if(ChrOld == (BYTE)Opt.XLat.Rules[0][I])
							break;
					for(J=1; J < Opt.XLat.Rules[1][0]; J+=2)
						if(ChrOld == (BYTE)Opt.XLat.Rules[1][J])
							break;
					if(I >= Opt.XLat.Rules[0][0])
						CurLang=1;
					if(J >= Opt.XLat.Rules[1][0])
						CurLang=0;
					if()//???
					{
						Chr=(BYTE)Opt.XLat.Rules[CurLang][J+1];
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
		if (!hFarWnd)
			InitDetectWindowedMode();
		if (hFarWnd)
		{
			PostMessageW(hFarWnd,WM_INPUTLANGCHANGEREQUEST, INPUTLANGCHANGE_FORWARD, 0);
			if (Flags & XLAT_SWITCHKEYBBEEP)
				MessageBeep(0);
		}
	}

	return Line;
}
