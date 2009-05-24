/*
fnparce.cpp

Парсер файловых ассоциаций
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
#include "lang.hpp"
#include "message.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "panelmix.hpp"
#include "mix.hpp"

struct TSubstDataW
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

  Panel *AnotherPanel;
  Panel *ActivePanel;
};


static int IsReplaceVariable(const wchar_t *str,int *scr = NULL,
                                int *end = NULL,
                                int *beg_scr_break = NULL,
                                int *end_scr_break = NULL,
                                int *beg_txt_break = NULL,
                                int *end_txt_break = NULL);


static int ReplaceVariables(wchar_t *Str,struct TSubstDataW *PSubstData);
static wchar_t *_SubstFileName(wchar_t *CurStr,struct TSubstDataW *PSubstData,wchar_t *TempStr,int MaxTempStrSize);

// Str=if exist !#!\!^!.! far:edit < diff -c -p "!#!\!^!.!" !\!.!

static wchar_t *_SubstFileName(wchar_t *CurStr,struct TSubstDataW *PSubstData,wchar_t *TmpStr,int MaxTempStrSize)
{
  // рассмотрим переключатели активности/пассивности панели.
  if (StrCmpN(CurStr,L"!#",2)==0)
  {
    CurStr+=2;
    PSubstData->PassivePanel=TRUE;
    //_SVS(SysLog(L"PassivePanel=TRUE '%s'",CurStr));
    return CurStr;
  }

  if (StrCmpN(CurStr,L"!^",2)==0)
  {
    CurStr+=2;
    PSubstData->PassivePanel=FALSE;
    //_SVS(SysLog(L"PassivePanel=FALSE '%s'",CurStr));
    return CurStr;
  }

  // !! символ '!'
  if (StrCmpN(CurStr,L"!!",2)==0 && CurStr[2] != L'?')
  {
    xwcsncat(TmpStr,L"!",MaxTempStrSize-1);
    CurStr+=2;
    //_SVS(SysLog(L"!! TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !.!      Длинное имя файла с расширением
  if (StrCmpN(CurStr,L"!.!",3)==0 && CurStr[3] != L'?')
  {
    xwcsncat(TmpStr,PSubstData->PassivePanel ? (const wchar_t *)PSubstData->strAnotherName:PSubstData->Name, MaxTempStrSize-1);
    CurStr+=3;
    //_SVS(SysLog(L"!.! TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !~       Короткое имя файла без расширения
  if (StrCmpN(CurStr,L"!~",2)==0)
  {
    xwcsncat(TmpStr,PSubstData->PassivePanel ? PSubstData->strAnotherShortNameOnly:PSubstData->strShortNameOnly, MaxTempStrSize-1);
    CurStr+=2;
    //_SVS(SysLog(L"!~ TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !`  Длинное расширение файла без имени
  if (StrCmpN(CurStr,L"!`",2)==0)
  {
    const wchar_t *Ext;
    if(CurStr[2] == L'~')
    {
      Ext=wcsrchr((PSubstData->PassivePanel ? (const wchar_t *)PSubstData->strAnotherShortName:PSubstData->ShortName),L'.');
      CurStr+=3;
    }
    else
    {
      Ext=wcsrchr((PSubstData->PassivePanel ? (const wchar_t *)PSubstData->strAnotherName:PSubstData->Name),L'.');
      CurStr+=2;
    }
    if(Ext && *Ext)
      xwcsncat(TmpStr,++Ext, MaxTempStrSize-1);
    //_SVS(SysLog(L"!` TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !& !&~  список файлов разделенных пробелом.
  if ((!StrCmpN(CurStr,L"!&~",3) && CurStr[3] != L'?') ||
      (!StrCmpN(CurStr,L"!&",2) && CurStr[2] != L'?'))
  {
    string strFileNameL, strShortNameL;
    Panel *WPanel=PSubstData->PassivePanel?PSubstData->AnotherPanel:PSubstData->ActivePanel;
    DWORD FileAttrL;
    int ShortN0=FALSE;
    int CntSkip=2;
    if(CurStr[2] == L'~')
    {
      ShortN0=TRUE;
      CntSkip++;
    }
    WPanel->GetSelName(NULL,FileAttrL);
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
        xwcsncat(TmpStr,L" ", MaxTempStrSize-1);
      xwcsncat(TmpStr,strFileNameL, MaxTempStrSize-1);
      /* $ 05.03.2002 DJ
         если в буфер больше не влезет - выйдем из цикла
      */
      if (wcslen (TmpStr) >= static_cast<size_t>(MaxTempStrSize-1))
        break;
    }
    CurStr+=CntSkip;
    //_SVS(SysLog(L"!& TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !@  Имя файла, содержащего имена помеченных файлов
  //<Skeleton 2003 10 25>
  // !$!      Имя файла, содержащего короткие имена помеченных файлов
  // Ниже идет совмещение кода для разбора как !@! так и !$!
  //Вообще-то (по исторической справедливости как бы) - в !$! нужно выбрасывать модификаторы Q и A
  // Но нафиг нада:)
  //<Skeleton>
  if (StrCmpN(CurStr,L"!@",2)==0
        //<Skeleton 2003 10 25>
        || (StrCmpN(CurStr,L"!$",2)==0))
        //<Skeleton>
  {
    wchar_t Modifers[32]=L"", *Ptr; //???

    string *pListName;
    string *pAnotherListName;

    bool ShortN0 = FALSE;

    if (CurStr[1] == L'$')
      ShortN0 = TRUE;

    if ( ShortN0 )
    {
        pListName = PSubstData->pShortListName;
        pAnotherListName = PSubstData->pAnotherShortListName;
    }
    else
    {
        pListName = PSubstData->pListName;
        pAnotherListName = PSubstData->pAnotherListName;
    }


    if((Ptr=wcschr(CurStr+2,L'!')) != NULL)
    {
      if(Ptr[1] != L'?')
      {
        *Ptr=0;

        xwcsncpy(Modifers,CurStr+2,countof(Modifers)-1);

        if ( pListName)
        {

          if ( PSubstData->PassivePanel && ( !pAnotherListName->IsEmpty() || PSubstData->AnotherPanel->MakeListFile(*pAnotherListName,ShortN0,Modifers)))
          {
            if (ShortN0)
              ConvertNameToShort(*pAnotherListName, *pAnotherListName);

            xwcsncat(TmpStr,*pAnotherListName, MaxTempStrSize-1);

          }

          if ( !PSubstData->PassivePanel && ( !pListName->IsEmpty() || PSubstData->ActivePanel->MakeListFile(*pListName,ShortN0,Modifers)))
          {
            if (ShortN0)
              ConvertNameToShort(*pListName,*pListName);

            xwcsncat(TmpStr,*pListName, MaxTempStrSize-1);
          }
        }
        else
        {
          xwcsncat(TmpStr,CurStr, MaxTempStrSize-1);
          xwcsncat(TmpStr,Modifers, MaxTempStrSize-1);
          xwcsncat(TmpStr,L"!", MaxTempStrSize-1);
        }

        CurStr+=Ptr-CurStr+1;
        return CurStr;
      }
    }
  }

  // !-!      Короткое имя файла с расширением
  if (StrCmpN(CurStr,L"!-!",3)==0 && CurStr[3] != L'?')
  {
    xwcsncat(TmpStr,PSubstData->PassivePanel ? (const wchar_t *)PSubstData->strAnotherShortName:PSubstData->ShortName, MaxTempStrSize-1);
    CurStr+=3;
    //_SVS(SysLog(L"!-! TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !+!      Аналогично !-!, но если длинное имя файла утеряно
  //          после выполнения команды, FAR восстановит его
  if (StrCmpN(CurStr,L"!+!",3)==0 && CurStr[3] != L'?')
  {
    xwcsncat(TmpStr,PSubstData->PassivePanel ? (const wchar_t *)PSubstData->strAnotherShortName:PSubstData->ShortName, MaxTempStrSize-1);
    CurStr+=3;
    PSubstData->PreserveLFN=TRUE;
    //_SVS(SysLog(L"!+! TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !:       Текущий диск
  //<Skeleton 2003 10 26>
  // Надо - добавляю анализ на \\serv\share
  //<Skeleton>
  if (StrCmpN(CurStr,L"!:",2)==0)
  {
    string strCurDir;
    string strRootDir;
    if (*PSubstData->Name && PSubstData->Name[1]==L':')
      strCurDir = PSubstData->Name;
    else
      if (PSubstData->PassivePanel)
        PSubstData->AnotherPanel->GetCurDir(strCurDir);
      else
        strCurDir = PSubstData->strCmdDir;

    GetPathRoot(strCurDir,strRootDir);
    DeleteEndSlash(strRootDir);
    xwcsncat(TmpStr,strRootDir, MaxTempStrSize-1);
    CurStr+=2;
    //_SVS(SysLog(L"!: TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !\       Текущий путь
  //<Skeleton 2003 10 26>
  // !/       Короткое имя текущего пути
  // Ниже идет совмещение кода для разбора как !\ так и !/
  //<Skeleton>
  if (StrCmpN(CurStr,L"!\\",2)==0 || StrCmpN(CurStr,L"!=\\",3)==0
     //<Skeleton 2003 10 26>
     || (StrCmpN(CurStr,L"!/",2)==0) || StrCmpN(CurStr,L"!=/",3)==0)
     //<Skeleton>
  {
    string strCurDir;
    //<Skeleton 2003 10 26>
    bool ShortN0 = FALSE;
    int RealPath= CurStr[1]==L'='?1:0;

    if (CurStr[1] == L'/' || (RealPath && CurStr[2] == L'/'))
    {
      ShortN0 = TRUE;
    }
    //<Skeleton>
    if (PSubstData->PassivePanel)
      PSubstData->AnotherPanel->GetCurDir(strCurDir);
    else
      strCurDir = PSubstData->strCmdDir;

    if(RealPath)
    {
      _MakePath1(PSubstData->PassivePanel?KEY_ALTSHIFTBACKBRACKET:KEY_ALTSHIFTBRACKET,strCurDir,L"",ShortN0);
      Unquote(strCurDir);
    }

    if (ShortN0)
        ConvertNameToShort(strCurDir,strCurDir);

    AddEndSlash(strCurDir);
    if(RealPath)
      QuoteSpaceOnly(strCurDir);

    CurStr+=2+RealPath;

    if (*CurStr==L'!')
    {
//      strcpy(TmpName,PSubstData->Name);
//      strcpy(TmpShortName,PSubstData->ShortName);
      if (wcspbrk(PSubstData->PassivePanel?(const wchar_t *)PSubstData->strAnotherName:PSubstData->Name,L"\\:")!=NULL)
        strCurDir.SetLength(0);
    }

//    if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
    xwcsncat(TmpStr, strCurDir, MaxTempStrSize-1);
    //_SVS(SysLog(L"!\\ TmpStr=[%s] CurDir=[%s]",TmpStr, CurDir));
    return CurStr;
  }

  // !?<title>?<init>!
  if (StrCmpN(CurStr,L"!?",2)==0 && wcschr(CurStr+2,L'!')!=NULL)
  {
    //<Skeleton 2003 11 22>
    //char *NewCurStr=strchr(CurStr+2,'!')+1;
    //strncat(TmpStr,CurStr,NewCurStr-CurStr);
    //CurStr=NewCurStr;

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
    wcsncat(TmpStr,CurStr,j);
    CurStr += j;
    //<Skeleton>
    //_SVS(SysLog(L"!? TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !        Длинное имя файла без расширения
  if (*CurStr==L'!')
  {
//    if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
    xwcsncat(TmpStr,PointToName(PSubstData->PassivePanel ? PSubstData->strAnotherNameOnly:PSubstData->strNameOnly), MaxTempStrSize-1);
    CurStr++;
    //_SVS(SysLog(L"! TmpStr=[%s]",TmpStr));
  }

  return CurStr;
}


/*
  SubstFileName()
  Преобразование метасимволов ассоциации файлов в реальные значения

  Входные ListName и ShortListName обЯзаны иметь размер NM*2 !!!
*/
int SubstFileName(string &strStr,            // результирующая строка
                  const wchar_t *Name,           // Длинное имя
                  const wchar_t *ShortName,      // Короткое имя

                  string *pListName,
                  string *pAnotherListName,
                  string *pShortListName,
                  string *pAnotherShortListName,
                  int   IgnoreInput,    // TRUE - не исполнять "!?<title>?<init>!"
                  const wchar_t *CmdLineDir)     // Каталог исполнения
{
    if ( pListName )
        *pListName = L"";

    if ( pAnotherListName )
        *pAnotherListName = L"";

    if ( pShortListName )
        *pShortListName = L"";

    if ( pAnotherShortListName )
        *pAnotherShortListName = L"";

  /* $ 19.06.2001 SVS
    ВНИМАНИЕ! Для альтернативных метасимволов, не основанных на "!",
    нужно будет либо убрать эту проверку либо изменить условие (последнее
    предпочтительнее!)
  */
  if(!wcschr(strStr,L'!'))
    return FALSE;

  wchar_t TmpStr2[10240]; //BUGBUGBUGBUGBUGBUG!!!!

  struct TSubstDataW SubstData, *PSubstData=&SubstData;

  // Сделаем пока поболее - 10240, но везде нужно проверять размер... (see below)
  wchar_t TmpStr[10240]; //BUGBUGBUG!!!
  wchar_t *CurStr;

  // <PreProcess>
  *TmpStr=0; // пока пусто.

  PSubstData->Name=Name;                    // Длинное имя
  PSubstData->ShortName=ShortName;          // Короткое имя

  PSubstData->pListName=pListName;            // Длинное имя файла-списка
  PSubstData->pAnotherListName=pAnotherListName;            // Длинное имя файла-списка
  PSubstData->pShortListName=pShortListName;  // Короткое имя файла-списка
  PSubstData->pAnotherShortListName=pAnotherShortListName;  // Короткое имя файла-списка

  // Если имя текущего каталога не задано...
  if (CmdLineDir!=NULL)
    PSubstData->strCmdDir = CmdLineDir;
  else // ...спросим у ком.строки
    CtrlObject->CmdLine->GetCurDir(PSubstData->strCmdDir);

  size_t pos;

  // Предварительно получим некоторые "константы" :-)
  PSubstData->strNameOnly = Name;

  if (PSubstData->strNameOnly.RPos(pos,L'.'))
    PSubstData->strNameOnly.SetLength(pos);

  PSubstData->strShortNameOnly = ShortName;

  if (PSubstData->strShortNameOnly.RPos(pos,L'.'))
    PSubstData->strShortNameOnly.SetLength(pos);

  PSubstData->ActivePanel=CtrlObject->Cp()->ActivePanel;
  PSubstData->AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(PSubstData->ActivePanel);

  PSubstData->AnotherPanel->GetCurName(PSubstData->strAnotherName,PSubstData->strAnotherShortName);

  PSubstData->strAnotherNameOnly = PSubstData->strAnotherName;

  if (PSubstData->strAnotherNameOnly.RPos(pos,L'.'))
    PSubstData->strAnotherNameOnly.SetLength(pos);

  PSubstData->strAnotherShortNameOnly = PSubstData->strAnotherShortName;

  if (PSubstData->strAnotherShortNameOnly.RPos(pos,L'.'))
    PSubstData->strAnotherShortNameOnly.SetLength(pos);

  PSubstData->PreserveLFN=FALSE;
  PSubstData->PassivePanel=FALSE; // первоначально речь идет про активную панель!
  // </PreProcess>

  wcscpy (TmpStr2, strStr); //BUGBUG

  if (!IgnoreInput)
    ReplaceVariables(TmpStr2,PSubstData);

  //CurStr=Str;
  CurStr=TmpStr2;

  while (*CurStr)
  {
    //_SVS(SysLog(L"***** Pass=%d",Pass));
    if(*CurStr == L'!')
      CurStr=_SubstFileName(CurStr,PSubstData,TmpStr,(int)(sizeof(TmpStr)-StrLength(TmpStr)-1));
    else                                           //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ как способ проверки!
    {
      wcsncat(TmpStr,CurStr,1);
      CurStr++;
    }
    //_SVS(++Pass);
  }

  strStr = TmpStr;

  //_SVS(SysLog(L"[%s]\n",Str));
  return(PSubstData->PreserveLFN);
}

int ReplaceVariables(wchar_t *Str,struct TSubstDataW *PSubstData)
{
  const int MaxSize=20;

  wchar_t *StartStr=Str;

  if (*Str==L'\"')
    while (*Str && *Str!=L'\"')
      Str++;

  struct DialogItemEx *DlgData = new DialogItemEx[MaxSize+2];
  int DlgSize=0;
  int StrPos[128],StrEndPos[128],StrPosSize=0;

  while (*Str && DlgSize<MaxSize)
  {
    if (*(Str++)!=L'!')
      continue;
    if(!*Str)
      break;
    if (*(Str++)!=L'?')
      continue;
    if(!*Str)
      break;

    //<Skeleton 2003 11 20>
    //if (strchr(Str,'!')==NULL)  //<---------теперича все не просто
    //  return 0;                 // придется сразу определить наличие операторных скобок
                                  // запомнить их позицию
    int scr,end, beg_t,end_t,beg_s,end_s;
    scr = end = beg_t = end_t = beg_s = end_s = 0;
    int ii = IsReplaceVariable(Str-2,&scr,&end,&beg_t,&end_t,&beg_s,&end_s);
    if (ii == -1)
    {
      delete [] DlgData;
      *StartStr=0;
      return 0;
    }

    StrEndPos[StrPosSize] = (int)(Str - StartStr - 2) + ii ; //+1
    StrPos[StrPosSize++]=(int)(Str-StartStr-2);
    //<Skeleton>

    DlgData[DlgSize].Clear();
    DlgData[DlgSize].Type=DI_TEXT;
    DlgData[DlgSize].X1=5;
    DlgData[DlgSize].Y1=DlgSize+2;
    DlgData[DlgSize+1].Clear();
    DlgData[DlgSize+1].Type=DI_EDIT;
    DlgData[DlgSize+1].X1=5;
    DlgData[DlgSize+1].X2=70;
    DlgData[DlgSize+1].Y1=DlgSize+3;
    DlgData[DlgSize+1].Flags|=DIF_HISTORY|DIF_USELASTHISTORY;

    wchar_t HistoryName[MaxSize][20];
    int HistoryNumber=DlgSize/2;
    swprintf(HistoryName[HistoryNumber],L"UserVar%d",HistoryNumber);
    DlgData[DlgSize+1].History=HistoryName[HistoryNumber];

    if (DlgSize==0)
    {
      DlgData[DlgSize+1].Focus=1;
      DlgData[DlgSize+1].DefaultButton=1;
    }

    wchar_t Title[256];
    //<Skeleton 2003 11 22>
    //xstrncpy(Title,Str,sizeof(Title)-1);
    //*strchr(Title,'!')=0;
    //Str+=strlen(Title)+1;
    //char *SrcText=strchr(Title,'?');
    //if (SrcText!=NULL)
    //{
    //  *SrcText=0;
    //  xstrncpy(DlgData[DlgSize+1].Data.Data,SrcText+1,sizeof(DlgData[DlgSize+1].Data.Data)-1);
    //}

    wchar_t Title2[512];
    wchar_t Title3[512];
    wchar_t Txt[512];

    // Заполняем название поля ввода заданным шаблоном - если есть
    *Title = *Title2 = *Title3 = 0;

    if (scr > 2)          // if between !? and ? exist some
      wcsncat(Title,Str,scr-2);

    //strcpy(DlgData[DlgSize].Data.Data,Title);

    wchar_t *t = Title;

    if (StrLength(Title)>0)
    {
      if (Title[0] == L'$')        // begin of history name
      {
        wchar_t *p = &Title[1];
        wchar_t *p1 = wcschr(p,L'$');
        if (p1)
        {
          *p1 = 0;
          t = ++p1;
          wcscpy(HistoryName[HistoryNumber],p);
        }
      }
    }

    wchar_t *Title1 = NULL;
    Title1 = (wchar_t *)xf_realloc(Title1,10240);
    wchar_t *Txt1 = NULL;
    Txt1 = (wchar_t *)xf_realloc(Txt1,10240);
    wchar_t * tmp_t = NULL;
    tmp_t = (wchar_t *)xf_realloc(tmp_t,10240);

    *Title1 = 0;
    *Txt1 = 0;
    *tmp_t = 0;


    if ((beg_t == -1) || (end_t == -1))
    {
      //
    }
    else if ((end_t - beg_t) > 1) //if between ( and ) exist some
    {
      int hist_correct = 0;
      if (t != Title) // Title contain name of history
        hist_correct = (int)(t - Title); //offset for jump over history

      wcsncat(Title1,Title+hist_correct,beg_t-2-hist_correct);   // !?$zz$xxxx(fffff)ddddd
                                                                 //       ^  ^

      wcsncat(Title2,Title+(end_t-2)+1,scr-end_t-1); // !?$zz$xxxx(fffff)ddddd
                                                     //                  ^   ^

      wcsncat(Title3,Title+(beg_t-2)+1,end_t-beg_t-1);  // !?$zz$xxxx(ffffff)ddddd
                                                        //            ^    ^
      wchar_t *CurStr= Title3;
      while (*CurStr)
      {
        if (*CurStr == L'!')
          CurStr=_SubstFileName(CurStr,PSubstData,tmp_t,10240-StrLength(tmp_t)-1);
        else                                           //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ как способ проверки!
        {
          wcsncat(tmp_t,CurStr,1);
          CurStr++;
        }
      }

      wcscat(Title1,tmp_t);
      wcscat(Title1,Title2);
      t = Title1;
    }
    //do it - типа здесь все уже раскрыто и преобразовано
    DlgData[DlgSize].strData = t;

    // Заполняем поле ввода заданным шаблоном - если есть
    wchar_t *s = Txt;
    *Txt = 0;
    *Title2 = 0;
    *Title3 = 0;
    *tmp_t = 0;

    if ((end-scr) > 1)  //if between ? and ! exist some
      wcsncat(Txt,(Str-2)+scr+1,(end-scr)-1);

    if ((beg_s == -1) || (end_s == -1))
    {
      //
    }
    else if ((end_s - beg_s) > 1) //if between ( and ) exist some
    {
      wcsncat(Txt1,Txt,beg_s-scr-1);   // !?$zz$xxxx(fffff)ddddd?rrrr(pppp)qqqqq!
                                       //                        ^  ^

      wcsncat(Title2,Txt+(end_s-scr),end-end_s-1); // !?$zz$xxxx(fffff)ddddd?rrrr(pppp)qqqqq!
                                                   //                                  ^   ^

      wcsncat(Title3,Txt+(beg_s-scr),end_s-beg_s-1);  // !?$zz$xxxx(ffffff)ddddd?rrrr(pppp)qqqqq!
                                                      //                              ^  ^

      wchar_t *CurStr= Title3;
      while (*CurStr)
      {
        if (*CurStr == L'!')
          CurStr=_SubstFileName(CurStr,PSubstData,tmp_t,10240-StrLength(tmp_t)-1);
        else                                           //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ как способ проверки!
        {
          wcsncat(tmp_t,CurStr,1);
          CurStr++;
        }
      }

      wcscat(Txt1,tmp_t);
      wcscat(Txt1,Title2);
      s = Txt1;
    }
    DlgData[DlgSize+1].strData = s;


    xf_free(Title1);
    xf_free(Txt1);
    xf_free(tmp_t);
    //<Skeleton>

    apiExpandEnvironmentStrings (DlgData[DlgSize].strData,DlgData[DlgSize].strData);
    DlgSize+=2;
  }
  if (DlgSize==0)
  {
  	delete [] DlgData;
    return 0;
  }
  DlgData[DlgSize].Clear();
  DlgData[DlgSize].Type=DI_DOUBLEBOX;
  DlgData[DlgSize].X1=3;
  DlgData[DlgSize].Y1=1;
  DlgData[DlgSize].X2=72;
  DlgData[DlgSize].Y2=DlgSize+2;
  DlgSize++;

  int ExitCode;
  {
    Dialog Dlg(DlgData,DlgSize);
    Dlg.SetPosition(-1,-1,76,DlgSize+3);
    Dlg.Process();
    ExitCode=Dlg.GetExitCode();
  }

  if (ExitCode==-1)
  {
    delete [] DlgData;
    *StartStr=0;
    return 0;
  }

  wchar_t TmpStr[4096];
  *TmpStr=0;
  for (Str=StartStr;*Str!=0;Str++)
  {
    int Replace=-1;
    //<Skeleton 2003 11 22>
    int end_pos=0;
    for (int I=0;I<StrPosSize;I++)
      if (Str-StartStr==StrPos[I])
      {
        Replace=I;
        end_pos = StrEndPos[I];
        break;
      }
    //<Skeleton>
    if (Replace!=-1)
    {
      wcscat(TmpStr,DlgData[Replace*2+1].strData);

      //<Skeleton 2003 11 20>
      //Str=strchr(Str+1,'!');
      Str = StartStr+end_pos;
      //<Skeleton>
    }
    else
      wcsncat(TmpStr,Str,1);
  }
  wcscpy(StartStr,TmpStr);

  ////ExpandEnvironmentStr(TmpStr,StartStr,sizeof(DlgData[0].Data)); ///BUGBUG
  delete [] DlgData;

  return 1;
}

bool Panel::MakeListFile(string &strListFileName,bool ShortNames,const wchar_t *Modifers)
{
	bool Ret=false;
	if(FarMkTempEx(strListFileName))
	{
		HANDLE hListFile=apiCreateFile(strListFileName,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,CREATE_ALWAYS,0);
		if(hListFile!=INVALID_HANDLE_VALUE)
		{
			UINT CodePage=CP_OEMCP;
			LPCVOID Eol=DOS_EOL_fmtA;
			DWORD EolSize=2;
			if(Modifers && *Modifers)
			{
				if(wcschr(Modifers,L'A')) // ANSI
				{
					CodePage=CP_ACP;
				}
				else
				{
					DWORD Signature=0;
					int SignatureSize=0;
					if(wcschr(Modifers,L'W')) // UTF16LE
					{
						CodePage=CP_UNICODE;
						Signature=SIGN_UNICODE;
						SignatureSize=2;
						Eol=DOS_EOL_fmt;
						EolSize=2*sizeof(WCHAR);
					}
					else
					{
						if(wcschr(Modifers,L'U')) // UTF8
						{
							CodePage=CP_UTF8;
							Signature=SIGN_UTF8;
							SignatureSize=3;
						}
					}
					if(Signature && SignatureSize)
					{
						DWORD NumberOfBytesWritten;
						WriteFile(hListFile,&Signature,SignatureSize,&NumberOfBytesWritten,NULL);
					}
				}
			}
			string strFileName,strShortName;
			DWORD FileAttr;
			GetSelName(NULL,FileAttr);
			while(GetSelName(&strFileName,FileAttr,&strShortName))
			{
				if(ShortNames)
					strFileName = strShortName;
				if(Modifers && *Modifers)
				{
					if(wcschr(Modifers,L'F') && PointToName(strFileName) == strFileName.CPtr()) // 'F' - использовать полный путь; //BUGBUG ?
					{
						string strTempFileName=strCurDir;
						if(ShortNames)
							ConvertNameToShort(strTempFileName,strTempFileName);
						AddEndSlash(strTempFileName);
						strTempFileName+=strFileName; //BUGBUG ?
						strFileName=strTempFileName;
					}
					if(wcschr(Modifers,L'Q')) // 'Q' - заключать имена с пробелами в кавычки;
						QuoteSpaceOnly(strFileName);
					if(wcschr(Modifers,L'S')) // 'S' - использовать '/' вместо '\' в путях файлов;
					{
						size_t Len=strFileName.GetLength();
						wchar_t *FileName=strFileName.GetBuffer();
						for(size_t i=0;i<Len;i++)
						{
							if(FileName[i]==L'\\')
							{
								FileName[i]=L'/';
							}
						}
						strFileName.ReleaseBuffer ();
					}
				}
				LPCVOID Ptr=NULL;
				LPSTR Buffer=NULL;
				DWORD NumberOfBytesToWrite=0,NumberOfBytesWritten=0;
				if(CodePage==CP_UNICODE)
				{
					Ptr=strFileName.CPtr();
					NumberOfBytesToWrite=static_cast<DWORD>(strFileName.GetLength()*sizeof(WCHAR));
				}
				else
				{
					int Size=WideCharToMultiByte(CodePage,0,strFileName,static_cast<int>(strFileName.GetLength()),NULL,0,NULL,NULL);
					if(Size)
					{
						Buffer=static_cast<LPSTR>(xf_malloc(Size));
						if(Buffer)
						{
							NumberOfBytesToWrite=WideCharToMultiByte(CodePage,0,strFileName,static_cast<int>(strFileName.GetLength()),Buffer,Size,NULL,NULL);
							Ptr=Buffer;
						}
					}
				}
				BOOL Written=WriteFile(hListFile,Ptr,NumberOfBytesToWrite,&NumberOfBytesWritten,NULL);
				if(Buffer)
					xf_free(Buffer);
				if(Written && NumberOfBytesWritten==NumberOfBytesToWrite)
				{
					if(WriteFile(hListFile,Eol,EolSize,&NumberOfBytesWritten,NULL) && NumberOfBytesWritten==EolSize)
					{
						Ret=true;
					}
				}
				else
				{
					Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotCreateListFile),MSG(MCannotCreateListWrite),MSG(MOk));
					apiDeleteFile(strListFileName);
					break;
				}
			}
			CloseHandle(hListFile);
		}
		else
		{
			Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotCreateListFile),MSG(MCannotCreateListTemp),MSG(MOk));
		}
	}
	else
	{
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
// все очень сложно - посл-иe 4 указателя - это смещения от str
// начало скобок в строке описания, конец этих скобок, начало скобок в строке начального заполнения, ну и соотв конец.
// Вообще при простом вызове (который я собираюсь юзать) это выглядит просто:
// i = IsReplaceVariable(str) - ведь нам надо только проверять семантику скобок и всяких ?!
// где  i - тот прыжок, который надо совершить, чтоб прыгнуть на конец ! структуры !??!
{
  const wchar_t *s      = str;
  const wchar_t *scrtxt = str;

  int count_scob = 0;
  int second_count_scob = 0;

  bool was_quest = false;         //  ?
  bool was_asterics = false;      //  !

  bool in_firstpart_was_scob = false;
  const wchar_t *beg_firstpart_scob = NULL;
  const wchar_t *end_firstpart_scob = NULL;

  bool in_secondpart_was_scob = false;
  const wchar_t *beg_secondpart_scob = NULL;
  const wchar_t *end_secondpart_scob = NULL;

  if (!s)
    return -1;

  if (StrCmpN(s,L"!?",2) == 0)
    s = s + 2;
  else
    return -1;
  //
  while (true)    // analize from !? to ?
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
      if (count_scob == 0)
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
  if (count_scob != 0) return -1;
  scrtxt = s - 1; //remember s for return

  while (true)    //analize from ? or !
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
      if (second_count_scob == 0)
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
  if (second_count_scob != 0) return -1;

  //
  if (scr != NULL)
    *scr = (int)(scrtxt - str);
  if (end != NULL)
    *end = (int)(s - str) - 1;

  if (in_firstpart_was_scob)
  {
    if (beg_scr_break != NULL)
      *beg_scr_break = (int)(beg_firstpart_scob - str);
    if (end_scr_break != NULL)
      *end_scr_break = (int)(end_firstpart_scob - str);
  }
  else
  {
    if (beg_scr_break != NULL)
      *beg_scr_break = -1;
    if (end_scr_break != NULL)
      *end_scr_break = -1;
  }

  if (in_secondpart_was_scob)
  {
    if (beg_txt_break != NULL)
      *beg_txt_break = (int)(beg_secondpart_scob - str);
    if (end_txt_break != NULL)
      *end_txt_break = (int)(end_secondpart_scob - str);
  }
  else
  {
    if (beg_txt_break != NULL)
      *beg_txt_break = -1;
    if (end_txt_break != NULL)
      *end_txt_break = -1;
  }

  return (int)((s - str) - 1);
}
