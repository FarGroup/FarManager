/*
xlat.cpp

XLat - перекодировка

*/

/* Revision: 1.00 22.12.2000 $ */

/*
Modify:
  22.12.2000 SVS
    + Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop
#include "internalheaders.hpp"


/* $ 05.09.2000 SVS
  XLat-перекодировка!
  На основе плагина EditSwap by SVS :-)))
*/
char* WINAPI Xlat(
   char *Line,                    // исходная строка
   int StartPos,                  // начало переконвертирования
   int EndPos,                    // конец переконвертирования
   struct CharTableSet *TableSet, // перекодировочная таблица (может быть NULL)
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
  if(!Line || *Line == 0)
    return NULL;
  /* SVS $ */

  I=strlen(Line);

  if(EndPos > I)
    EndPos=I;

  if(StartPos < 0)
    StartPos=0;

  if(StartPos > EndPos || StartPos >= I)
    return Line;


//  OemToCharBuff(Opt.QWERTY.Table[0],Opt.QWERTY.Table[0],80);???
  if(!Opt.XLat.Table[0][0] || !Opt.XLat.Table[1][0])
    return Line;


  I=strlen((char *)Opt.XLat.Table[1]),
  J=strlen((char *)Opt.XLat.Table[0]);
  int MinLenTable=(I > J?J:I);

  if (TableSet)
    // из текущей кодировки в OEM
    DecodeString(Line+StartPos,(LPBYTE)TableSet->DecodeTable,EndPos-StartPos+1);

  // цикл по всей строке
  for(J=StartPos; J < EndPos; J++)
  {
    ChrOld=Chr=(BYTE)Line[J];
    // ChrOld - пред символ
    IsChange=0;
    // цикл по просмотру Chr в таблицах
    for(I=0; I < MinLenTable; ++I)
    {
      // символ из латиницы?
      if(Chr == (BYTE)Opt.XLat.Table[1][I])
      {
        Chr=(char)Opt.XLat.Table[0][I];
        IsChange=1;
        CurLang=1; // pred - english
        LangCount[1]++;
        break;
      }
      // символ из русской?
      else if(Chr == (BYTE)Opt.XLat.Table[0][I])
      {
        Chr=(char)Opt.XLat.Table[1][I];
        CurLang=0; // pred - russian
        LangCount[0]++;
        IsChange=1;
        break;
      }
    }

    if(!IsChange) // особые случаи...
    {
      PreLang=CurLang;
      if(LangCount[0] > LangCount[1])
        CurLang=0;
      else
        CurLang=1;
      if(PreLang != CurLang)
        CurLang=PreLang;

        for(I=0; I < sizeof(Opt.XLat.Rules[0]) && Opt.XLat.Rules[0][I]; I+=2)
          if(ChrOld == (BYTE)Opt.XLat.Rules[CurLang][I])
          {
             Chr=(BYTE)Opt.XLat.Rules[CurLang][I+1];
             break;
          }
    }

    Line[J]=(char)Chr;
  }

  if (TableSet)
    // из OEM в текущую кодировку
    EncodeString(Line+StartPos,(LPBYTE)TableSet->EncodeTable,EndPos-StartPos+1);

  // переключаем раскладку клавиатуры?
  //  к сожалению не работает под Win9x - ставьте WinNT и наслаждайтесь :-)
  /* $ 20.09.2000 SVS
     Немного изменим условия и возьмем окно именно FAR.
  */
  if(hFarWnd && (Flags & XLAT_SWITCHKEYBLAYOUT))
  {
    PostMessage(hFarWnd,WM_INPUTLANGCHANGEREQUEST, 1, HKL_NEXT);
    /* $ 04.11.2000 SVS
       Выдаем звуковой сигнал, если надо.
    */
    if(Flags & XLAT_SWITCHKEYBBEEP)
      MessageBeep(0);
    /* SVS $ */
  }
  /* SVS $ */

  return Line;
}
/* SVS $ */
