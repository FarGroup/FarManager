/*
xlat.cpp

XLat - перекодировка

*/

#include "headers.hpp"
#pragma hdrstop

#include "plugin.hpp"
#include "global.hpp"
#include "fn.hpp"

// $ 05.09.2000 SVS -  XLat-перекодировка!
char* WINAPI Xlat(
    char *Line,                    // исходная строка
    int StartPos,                  // начало переконвертирования
    int EndPos,                    // конец переконвертирования
    const struct CharTableSet *TableSet, // перекодировочная таблица (может быть NULL)
    DWORD Flags)                   // флаги (см. enum XLATMODE)
{
	BYTE Chr,ChrOld;
	int J,I;
	int PreLang=2,CurLang=2; // uncnown
	int LangCount[2]={0,0};
	int IsChange=0;

	/* $ 08.09.2000 SVS
	   Ошибочка вкралась :-)))
	*/
	if (!Line || *Line == 0)
		return NULL;

	/* SVS $ */
	I=(int)strlen(Line);

	if (EndPos > I)
		EndPos=I;

	if (StartPos < 0)
		StartPos=0;

	if (StartPos > EndPos || StartPos >= I)
		return Line;

//  FAR_OemToCharBuff(Opt.QWERTY.Table[0],Opt.QWERTY.Table[0],80);???
	if (!Opt.XLat.Table[0][0] || !Opt.XLat.Table[1][0])
		return Line;

	int MinLenTable=(BYTE)Opt.XLat.Table[1][0];

	if ((BYTE)Opt.XLat.Table[1][0] > (BYTE)Opt.XLat.Table[0][0])
		MinLenTable=(BYTE)Opt.XLat.Table[0][0];

	if (TableSet)
		// из текущей кодировки в OEM
		DecodeString(Line+StartPos,(LPBYTE)TableSet->DecodeTable,EndPos-StartPos+1);

	char LayoutName[64];
	int ProcessLayoutName=FALSE;

	if ((Flags & XLAT_USEKEYBLAYOUTNAME) && FARGetKeybLayoutName(LayoutName,sizeof(LayoutName)-1))
	{
		GetRegKey("XLat",LayoutName,(BYTE*)&Opt.XLat.Rules[2][1],(BYTE*)"",sizeof(Opt.XLat.Rules[2]));

		if (Opt.XLat.Rules[2][1])
			ProcessLayoutName=TRUE;
	}

	// цикл по всей строке
	for (J=StartPos; J < EndPos; J++)
	{
		ChrOld=Chr=(BYTE)Line[J];
		// ChrOld - пред символ
		IsChange=0;

		// цикл по просмотру Chr в таблицах
		// <=MinLenTable так как длина настоящая а начальный индекс 1
		for (I=1; I <= MinLenTable; ++I)
		{
			// символ из латиницы?
			if (Chr == (BYTE)Opt.XLat.Table[1][I])
			{
				Chr=(char)Opt.XLat.Table[0][I];
				IsChange=1;
				CurLang=1; // pred - english
				LangCount[1]++;
				break;
			}
			// символ из русской?
			else if (Chr == (BYTE)Opt.XLat.Table[0][I])
			{
				Chr=(char)Opt.XLat.Table[1][I];
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
				for (I=1; I < Opt.XLat.Rules[2][0]; I+=2)
					if (Chr == (BYTE)Opt.XLat.Rules[2][I])
					{
						Chr=(BYTE)Opt.XLat.Rules[2][I+1];
						break;
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

				for (I=1; I < Opt.XLat.Rules[CurLang][0]; I+=2)
					if (ChrOld == (BYTE)Opt.XLat.Rules[CurLang][I])
					{
						Chr=(BYTE)Opt.XLat.Rules[CurLang][I+1];
						break;
					}

#if 0

				// Если в таблице не найдено и таблица была Unknown...
				if (I >= Opt.XLat.Rules[CurLang][0] && CurLang == 2)
				{
					// ...смотрим сначала в первой таблице...
					for (I=1; I < Opt.XLat.Rules[0][0]; I+=2)
						if (ChrOld == (BYTE)Opt.XLat.Rules[0][I])
							break;

					for (J=1; J < Opt.XLat.Rules[1][0]; J+=2)
						if (ChrOld == (BYTE)Opt.XLat.Rules[1][J])
							break;

					if (I >= Opt.XLat.Rules[0][0])
						CurLang=1;

					if (J >= Opt.XLat.Rules[1][0])
						CurLang=0;

					if ()//???
					{
						Chr=(BYTE)Opt.XLat.Rules[CurLang][J+1];
					}
				}

#endif
			}
		}

		Line[J]=(char)Chr;
	}

	if (TableSet)
		// из OEM в текущую кодировку
		EncodeString(Line+StartPos,(LPBYTE)TableSet->EncodeTable,EndPos-StartPos+1);

	// переключаем раскладку клавиатуры?
	//  к сожалению не работает под Win9x - ставьте WinNT и наслаждайтесь :-)
	if ((Flags & XLAT_SWITCHKEYBLAYOUT))
	{
		if (!hFarWnd)
			InitDetectWindowedMode();

		if (hFarWnd)
		{
			HKL Next=(HKL)0;

			if (Opt.XLat.Layouts[0])
			{
				if (++Opt.XLat.CurrentLayout >= (int)sizeof(Opt.XLat.Layouts)/sizeof(Opt.XLat.Layouts[0]) || !Opt.XLat.Layouts[Opt.XLat.CurrentLayout])
					Opt.XLat.CurrentLayout=0;

				if (Opt.XLat.Layouts[Opt.XLat.CurrentLayout])
					Next=Opt.XLat.Layouts[Opt.XLat.CurrentLayout];
			}

			PostMessage(hFarWnd,WM_INPUTLANGCHANGEREQUEST, Next?0:INPUTLANGCHANGE_FORWARD, (LPARAM)Next);

			if (Flags & XLAT_SWITCHKEYBBEEP)
				MessageBeep(0);
		}
	}

	return Line;
}
