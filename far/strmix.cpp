/*
strmix.cpp

Куча разных вспомогательных функций по работе со строками
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

#include "fn.hpp"

#include "lang.hpp"
#include "language.hpp"

string &FormatNumber(const wchar_t *Src, string &strDest, int NumDigits)
{
	static bool first = true;
	static NUMBERFMTW fmt;
	static wchar_t DecimalSep[4];
	static wchar_t ThousandSep[4];
	if (first)
	{
		GetLocaleInfoW(LOCALE_USER_DEFAULT,LOCALE_STHOUSAND,ThousandSep,countof(ThousandSep));
		GetLocaleInfoW(LOCALE_USER_DEFAULT,LOCALE_SDECIMAL,DecimalSep,countof(DecimalSep));
		DecimalSep[1]=0;  //В винде сепараторы цифр могут быть больше одного символа
		ThousandSep[1]=0; //но для нас это будет не очень хорошо

		if(LOWORD(Opt.FormatNumberSeparators))
			*DecimalSep=LOWORD(Opt.FormatNumberSeparators);
		if(HIWORD(Opt.FormatNumberSeparators))
			*ThousandSep=HIWORD(Opt.FormatNumberSeparators);

		fmt.LeadingZero = 1;
		fmt.Grouping = 3;
		fmt.lpDecimalSep = DecimalSep;
		fmt.lpThousandSep = ThousandSep;
		fmt.NegativeOrder = 1;
		first = false;
	}
	fmt.NumDigits = NumDigits;
	string strSrc=Src;
	int Size=GetNumberFormatW(LOCALE_USER_DEFAULT,0,strSrc,&fmt,NULL,0);
	wchar_t* lpwszDest=strDest.GetBuffer(Size);
	GetNumberFormatW(LOCALE_USER_DEFAULT,0,strSrc,&fmt,lpwszDest,Size);
	strDest.ReleaseBuffer();
	return strDest;
}

string &InsertCommas(unsigned __int64 li,string &strDest)
{
   strDest.Format (L"%I64u", li);
   return FormatNumber(strDest,strDest);
/*
   wchar_t *lpwszDest = strDest.GetBuffer(strDest.GetLength() << 1); //BUGBUG

   for (int I=StrLength(lpwszDest)-4;I>=0;I-=3)
   {
      if (lpwszDest[I])
      {
        wmemmove(lpwszDest+I+2,lpwszDest+I+1,StrLength(lpwszDest+I));
        lpwszDest[I+1]=L',';
      }
   }

   strDest.ReleaseBuffer();

   return strDest;
*/
}

const wchar_t* __stdcall PointToName(const wchar_t *lpwszPath)
{
  if ( !lpwszPath )
    return NULL;

  if ( *lpwszPath!=0 && *(lpwszPath+1)==L':' ) lpwszPath+=2;

  const wchar_t *lpwszNamePtr = lpwszPath;
  while ( *lpwszNamePtr ) lpwszNamePtr++;

  while (lpwszNamePtr != lpwszPath)
  {
    if (IsSlash(*lpwszNamePtr))
      return lpwszNamePtr+1;
    lpwszNamePtr--;
  }
  if (IsSlash(*lpwszPath))
    return lpwszPath+1;
  else
    return lpwszPath;
}


//   Аналог PointToName, только для строк типа
//   "name\" (оканчивается на слеш) возвращает указатель на name, а не на пустую
//   строку

const wchar_t* __stdcall PointToFolderNameIfFolder(const wchar_t *Path)
{
  if(!Path)
    return NULL;

  const wchar_t *NamePtr=Path, *prevNamePtr=Path;

  while (*Path)
  {
    if (IsSlash(*Path) ||
        (*Path==L':' && Path==NamePtr+1))
    {
      prevNamePtr=NamePtr;
      NamePtr=Path+1;
    }
    ++Path;
  }
  return ((*NamePtr)?NamePtr:prevNamePtr);
}

const wchar_t* PointToExt(const wchar_t *lpwszPath)
{
  if ( !lpwszPath )
    return NULL;

  const wchar_t *lpwszEndPtr = lpwszPath;
  while ( *lpwszEndPtr ) lpwszEndPtr++;
  const wchar_t *lpwszExtPtr = lpwszEndPtr;

  while (lpwszExtPtr != lpwszPath)
  {
    if ( *lpwszExtPtr==L'.' )
    {
      if (IsSlash(*(lpwszExtPtr-1)) || *(lpwszExtPtr-1)==L':' )
        return lpwszEndPtr;
      else
        return lpwszExtPtr;
    }
    if (IsSlash(*lpwszExtPtr) || *lpwszExtPtr==L':' )
      return lpwszEndPtr;
    lpwszExtPtr--;
  }
  return lpwszEndPtr;
}


/* $ 10.05.2003 IS
   + Облегчим CmpName за счет выноса проверки skippath наружу
   - Ошибка: *Name*.* не находило Name
*/

// IS: это реальное тело функции сравнения с маской, но использовать
// IS: "снаружи" нужно не эту функцию, а CmpName (ее тело расположено
// IS: после CmpName_Body)

int CmpName_Body(const wchar_t *pattern,const wchar_t *str)
{
  wchar_t stringc,patternc,rangec;
  int match;

  for (;; ++str)
  {
    /* $ 01.05.2001 DJ
       используем инлайновые версии
    */
    stringc=Upper(*str);
    patternc=Upper(*pattern++);
    switch (patternc)
    {
      case 0:
        return(stringc==0);
      case L'?':
        if (stringc == 0)
          return(FALSE);

        break;
      case L'*':
        if (!*pattern)
          return(TRUE);

        /* $ 01.05.2001 DJ
           оптимизированная ветка работает и для имен с несколькими
           точками
        */
        if (*pattern==L'.')
        {
          if (pattern[1]==L'*' && pattern[2]==0)
            return(TRUE);
          if (wcspbrk (pattern, L"*?[") == NULL)
          {
            const wchar_t *dot = wcsrchr (str, L'.');
            if (pattern[1] == 0)
              return (dot==NULL || dot[1]==0);
            const wchar_t *patdot = wcschr (pattern+1, L'.');
            if (patdot != NULL && dot == NULL)
              return(FALSE);
            if (patdot == NULL && dot != NULL)
              return(StrCmpI (pattern+1,dot+1) == 0);
          }
        }

        while (*str)
        {
          if (CmpName(pattern,str++,FALSE))
            return(TRUE);
        }
        return(FALSE);
      case L'[':
        if (wcschr(pattern,L']')==NULL)
        {
          if (patternc != stringc)
            return (FALSE);
          break;
        }
        if (*pattern && *(pattern+1)==L']')
        {
          if (*pattern!=*str)
            return(FALSE);
          pattern+=2;
          break;
        }
        match = 0;
        while ((rangec = Upper(*pattern++))!=0)
        {
          if (rangec == L']')
          {
            if (match)
              break;
            else
              return(FALSE);
          }
          if (match)
            continue;
          if (rangec == L'-' && *(pattern - 2) != L'[' && *pattern != L']')
          {
            match = (stringc <= Upper(*pattern) &&
                     Upper(*(pattern - 2)) <= stringc);
            pattern++;
          }
          else
            match = (stringc == rangec);
        }
        if (rangec == 0)
          return(FALSE);
        break;
      default:
        if (patternc != stringc)
        {
          if (patternc==L'.' && stringc==0 && !CmpNameSearchMode)
            return(*pattern!=L'.' && CmpName(pattern,str));
          else
            return(FALSE);
        }
        break;
    }
  }
}


// IS: функция для внешнего мира, использовать ее
int CmpName(const wchar_t *pattern,const wchar_t *str,int skippath)
{
  if (!pattern||!str) return FALSE;
  if (skippath)
    str=PointToName(str);
  return CmpName_Body(pattern,str);
}


int ConvertWildcards(const wchar_t *SrcName, string &strDest, int SelectedFolderNameLength)
{
	string strPartAfterFolderName;
	string strSrc = SrcName;

	wchar_t *DestName = strDest.GetBuffer (strDest.GetLength()+strSrc.GetLength()+1); //???
	wchar_t *DestNamePtr = (wchar_t*)PointToName(DestName);

	string strWildName = DestNamePtr;

	if (wcschr(strWildName, L'*')==NULL && wcschr(strWildName, L'?')==NULL)
	{
		//strDest.ReleaseBuffer (); не надо так как строка не поменялась
		return(FALSE);
	}

	if (SelectedFolderNameLength!=0)
	{
		strPartAfterFolderName = ((const wchar_t *)strSrc+SelectedFolderNameLength);
		strSrc.SetLength(SelectedFolderNameLength);
	}

	const wchar_t *Src = strSrc;
	const wchar_t *SrcNamePtr = PointToName(Src);

	int BeforeNameLength = DestNamePtr==DestName ? (int)(SrcNamePtr-Src) : 0;

	wchar_t *PartBeforeName = (wchar_t*)xf_malloc ((BeforeNameLength+1)*sizeof (wchar_t));

	xwcsncpy(PartBeforeName, Src, BeforeNameLength);

	const wchar_t *SrcNameDot=wcsrchr(SrcNamePtr, L'.');

	const wchar_t *CurWildPtr = strWildName;

	while (*CurWildPtr)
	{
		switch(*CurWildPtr)
		{
			case L'?':
				CurWildPtr++;
				if (*SrcNamePtr)
					*(DestNamePtr++)=*(SrcNamePtr++);
				break;
			case L'*':
				CurWildPtr++;
				while (*SrcNamePtr)
				{
					if (*CurWildPtr==L'.' && SrcNameDot!=NULL && wcschr(CurWildPtr+1,L'.')==NULL)
					{
						if (SrcNamePtr==SrcNameDot)
							break;
					}
					else
						if (*SrcNamePtr==*CurWildPtr)
							break;
					*(DestNamePtr++)=*(SrcNamePtr++);
				}
				break;
			case L'.':
				CurWildPtr++;
				*(DestNamePtr++)=L'.';
				if (wcspbrk(CurWildPtr,L"*?")!=NULL)
					while (*SrcNamePtr)
						if (*(SrcNamePtr++)==L'.')
							break;
				break;
			default:
				*(DestNamePtr++)=*(CurWildPtr++);
				if (*SrcNamePtr && *SrcNamePtr!=L'.')
					SrcNamePtr++;
				break;
		}
	}

	*DestNamePtr=0;
	if (DestNamePtr!=DestName && *(DestNamePtr-1)==L'.')
		*(DestNamePtr-1)=0;

	strDest.ReleaseBuffer ();

	if (*PartBeforeName)
		strDest = PartBeforeName+strDest;
	if (SelectedFolderNameLength!=0)
		strDest += strPartAfterFolderName; //BUGBUG???, was src in 1.7x

	xf_free (PartBeforeName);

	return(TRUE);
}


/* $ 09.10.2000 IS
    Генерация нового имени по маске
    (взял из ShellCopy::ShellCopyConvertWildcards)
*/
// На основе имени файла (Src) и маски (Dest) генерируем новое имя
// SelectedFolderNameLength - длина каталога. Например, есть
// каталог dir1, а в нем файл file1. Нужно сгенерировать имя по маске для dir1.
// Параметры могут быть следующими: Src="dir1", SelectedFolderNameLength=0
// или Src="dir1\\file1", а SelectedFolderNameLength=4 (длина "dir1")

wchar_t * WINAPI InsertQuote(wchar_t *Str)
{
  size_t l = StrLength(Str);
  if (*Str != L'"')
  {
    wmemmove(Str+1,Str,++l);
    *Str=L'"';
  }
  if (Str[l-1] != L'"')
  {
    Str[l++] = L'\"';
    Str[l] = 0;
  }
  return Str;
}

wchar_t* WINAPI QuoteSpace(wchar_t *Str)
{
  if ( wcspbrk(Str, Opt.strQuotedSymbols) != NULL )
    InsertQuote(Str);

  return Str;
}


string& InsertQuote(string &strStr)
{
  size_t l = strStr.GetLength();
  wchar_t *Str = strStr.GetBuffer (strStr.GetLength()+3);

  if (*Str != L'"')
  {
    wmemmove(Str+1,Str,++l);
    *Str=L'"';
  }
  if(Str[l-1] != L'"')
  {
    Str[l++] = L'\"';
  }

  strStr.ReleaseBuffer (l);

  return strStr;
}

string &QuoteSpace(string &strStr)
{
	if (wcspbrk(strStr, Opt.strQuotedSymbols) != NULL)
		InsertQuote(strStr);

	return strStr;
}


wchar_t*  WINAPI QuoteSpaceOnly(wchar_t *Str)
{
  if (wcschr(Str,L' ')!=NULL)
    InsertQuote(Str);

  return Str;
}


string& WINAPI QuoteSpaceOnly(string &strStr)
{
  if (strStr.Contains(L' '))
    InsertQuote(strStr);

  return(strStr);
}


string& __stdcall TruncStrFromEnd(string &strStr, int MaxLength)
{
	wchar_t *lpwszBuffer = strStr.GetBuffer ();

	TruncStrFromEnd(lpwszBuffer, MaxLength);

	strStr.ReleaseBuffer ();

	return strStr;
}

wchar_t* WINAPI TruncStrFromEnd(wchar_t *Str,int MaxLength)
{
  if ( Str )
  {
    int Length = StrLength(Str);

    if (Length > MaxLength)
    {
      if (MaxLength>3)
        wmemcpy(Str+MaxLength-3, L"...", 3);
      Str[MaxLength]=0;
    }
  }
  return Str;
}


wchar_t* WINAPI TruncStr(wchar_t *Str,int MaxLength)
{
  if (Str)
  {
    int Length=StrLength(Str);
    if (MaxLength<0)
      MaxLength=0;
    if (Length > MaxLength)
    {
      if (MaxLength>3)
      {
        wchar_t *MovePos = Str+Length-MaxLength+3;
        wmemmove(Str+3, MovePos, StrLength(MovePos)+1);
        wmemcpy(Str,L"...",3);
      }

      Str[MaxLength]=0;
    }
  }

  return Str;
}


string& __stdcall TruncStr(string &strStr, int MaxLength)
{
	wchar_t *lpwszBuffer = strStr.GetBuffer ();

	TruncStr(lpwszBuffer, MaxLength);

	strStr.ReleaseBuffer ();

	return strStr;
}


wchar_t* WINAPI TruncPathStr(wchar_t *Str, int MaxLength)
{
  if (Str)
  {
    int nLength = (int)wcslen (Str);

		if((nLength > MaxLength) && (nLength >= 2))
    {
      wchar_t *lpStart = NULL;

			if ( *Str && (Str[1] == L':') && IsSlash(Str[2]) )
         lpStart = Str+3;
      else
      {
        if ( (Str[0] == L'\\') && (Str[1] == L'\\') )
        {
					if ((lpStart = const_cast<wchar_t*>(FirstSlash(Str+2))) != NULL)
					{
						wchar_t *lpStart2=lpStart;
						if ( (lpStart-Str < nLength) && ((lpStart=const_cast<wchar_t*>(FirstSlash(lpStart2+1)))!=NULL))
              lpStart++;
					}
        }
      }

      if ( !lpStart || (lpStart-Str > MaxLength-5) )
        return TruncStr(Str, MaxLength);

      wchar_t *lpInPos = lpStart+3+(nLength-MaxLength);
      wmemmove (lpStart+3, lpInPos, (wcslen (lpInPos)+1));
      wmemcpy (lpStart, L"...", 3);
    }
  }

  return Str;
}


string& __stdcall TruncPathStr(string &strStr, int MaxLength)
{
	wchar_t *lpwszStr= strStr.GetBuffer ();

	TruncPathStr(lpwszStr, MaxLength);

	strStr.ReleaseBuffer ();

	return strStr;
}


wchar_t* WINAPI RemoveLeadingSpaces(wchar_t *Str)
{
  wchar_t *ChPtr;
  if ((ChPtr=Str) == 0)
    return NULL;

  for (; IsSpace(*ChPtr); ChPtr++)
         ;
  if (ChPtr!=Str)
    wmemmove(Str,ChPtr,StrLength(ChPtr)+1);
  return Str;
}


string& WINAPI RemoveLeadingSpaces(string &strStr)
{
  wchar_t *ChPtr = strStr.GetBuffer ();
  wchar_t *Str = ChPtr;

  for (; IsSpace(*ChPtr); ChPtr++)
    ;

  if (ChPtr!=Str)
    wmemmove(Str,ChPtr,(StrLength(ChPtr)+1));

  strStr.ReleaseBuffer ();

  return strStr;
}


// удалить конечные пробелы
wchar_t* WINAPI RemoveTrailingSpaces(wchar_t *Str)
{
  if (!Str)
    return NULL;
  if (*Str == L'\0')
    return Str;

  wchar_t *ChPtr;
  size_t I;

  for (ChPtr=Str+(I=StrLength(Str))-1; I > 0; I--, ChPtr--)
  {
    if (IsSpace(*ChPtr) || IsEol(*ChPtr))
      *ChPtr=0;
    else
      break;
  }

  return Str;
}


string& WINAPI RemoveTrailingSpaces(string &strStr)
{
  if ( strStr.IsEmpty () )
    return strStr;

  RemoveTrailingSpaces(strStr.GetBuffer());

  strStr.ReleaseBuffer ();

  return strStr;
}


wchar_t* WINAPI RemoveExternalSpaces(wchar_t *Str)
{
  return RemoveTrailingSpaces(RemoveLeadingSpaces(Str));
}

string&  WINAPI RemoveExternalSpaces(string &strStr)
{
  return RemoveTrailingSpaces(RemoveLeadingSpaces(strStr));
}


/* $ 02.02.2001 IS
   Заменяет пробелами непечатные символы в строке. В настоящий момент
   обрабатываются только cr и lf.
*/

string& WINAPI RemoveUnprintableCharacters(string &strStr)
{
  wchar_t *Str = strStr.GetBuffer ();
  wchar_t *p = Str;

  while (*p)
  {
     if ( IsEol(*p) )
       *p=L' ';
     p++;
  }

  strStr.ReleaseBuffer();

  return RemoveExternalSpaces(strStr);
}


// Удалить символ Target из строки Str (везде!)
string &RemoveChar(string &strStr,wchar_t Target,BOOL Dup)
{
  wchar_t *Ptr = strStr.GetBuffer ();
  wchar_t *Str = Ptr, Chr;
  while ((Chr=*Str++) != 0)
  {
    if (Chr == Target)
    {
      if(Dup && *Str == Target)
      {
        *Ptr++ = Chr;
        ++Str;
      }
      continue;
    }
    *Ptr++ = Chr;
  }
  *Ptr = L'\0';

  strStr.ReleaseBuffer ();

  return strStr;
}


int HiStrlen(const wchar_t *Str)
{
  int Length=0;

  if (Str && *Str)
  {
    while (*Str)
    {
      if (*Str!=L'&')
      {
        Length++;
      }
      else
      {
        if (Str[1] == L'&')
        {
          Length++;
          ++Str;
        }
      }
      Str++;
    }
  }
  return(Length);
}

int HiFindRealPos(const wchar_t *Str, int Pos, BOOL ShowAmp)
{
	/*
			&&      = '&'
			&&&     = '&'
								 ^H
			&&&&    = '&&'
			&&&&&   = '&&'
								 ^H
			&&&&&&  = '&&&'
	*/

	if (ShowAmp)
	{
		return Pos;
	}

	int RealPos = 0;
	int VisPos = 0;

	if (Str)
	{
		while (VisPos < Pos && *Str)
		{
			if (*Str == L'&')
			{
				Str++;
				RealPos++;
				if (*Str == L'&' && *(Str+1) == L'&' && *(Str+2) != L'&')
				{
					Str++;
					RealPos++;
				}
			}

			Str++;
			VisPos++;
			RealPos++;
		}
	}

	return RealPos;
}

BOOL AddEndSlash(wchar_t *Path, wchar_t TypeSlash)
{
  BOOL Result=FALSE;
  if (Path)
  {
    /* $ 06.12.2000 IS
      ! Теперь функция работает с обоими видами слешей, также происходит
        изменение уже существующего конечного слеша на такой, который
        встречается чаще.
    */
    wchar_t *end;
    int Slash=0, BackSlash=0;
    if (!TypeSlash)
    {
      end=Path;
      while (*end)
      {
        Slash+=(*end==L'\\');
        BackSlash+=(*end==L'/');
        end++;
      }
    }
    else
    {
      end=Path+StrLength(Path);
      if (TypeSlash == L'\\')
        Slash=1;
      else
        BackSlash=1;
    }
    int Length=(int)(end-Path);
    char c=(Slash<BackSlash)?L'/':L'\\';
    Result=TRUE;
    if (Length==0)
    {
      *end=c;
      end[1]=0;
    }
    else
    {
      end--;
      if (!IsSlash(*end))
      {
        end[1]=c;
        end[2]=0;
      }
      else
      {
        *end=c;
      }
    }
  }
  return Result;
}


BOOL WINAPI AddEndSlash(wchar_t *Path)
{
	return AddEndSlash(Path, 0);
}


BOOL AddEndSlash(string &strPath)
{
	return AddEndSlash(strPath, 0);
}

BOOL AddEndSlash(
		string &strPath,
		wchar_t TypeSlash
		)
{
	wchar_t *lpwszPath = strPath.GetBuffer (strPath.GetLength()+2);

	BOOL Result = AddEndSlash(lpwszPath, TypeSlash);

	strPath.ReleaseBuffer ();

	return Result;
}


BOOL WINAPI DeleteEndSlash (string &strPath,bool allendslash)
{
  BOOL Ret=FALSE;
  if ( !strPath.IsEmpty() )
  {
    size_t len=strPath.GetLength();
    wchar_t *lpwszPath = strPath.GetBuffer ();
		while ( len && IsSlash(lpwszPath[--len]) )
    {
      Ret=TRUE;
      lpwszPath[len] = L'\0';
      if (!allendslash)
        break;
    }
    strPath.ReleaseBuffer();
  }
  return Ret;
}

string& CenterStr(const wchar_t *Src, string &strDest, int Length)
{
  int SrcLength=StrLength(Src);

  string strTempStr = Src; //если Src == strDest, то надо копировать Src!

  if (SrcLength >= Length)
  {
    /* Здесь не надо отнимать 1 от длины, т.к. strlen не учитывает \0
       и мы получали обрезанные строки */
    strDest = strTempStr;

    strDest.SetLength (Length);
  }
  else
  {
    int Space=(Length-SrcLength)/2;

    strDest.Format (L"%*s%s%*s",Space,L"",(const wchar_t*)strTempStr,Length-Space-SrcLength,L"");
  }

  return strDest;
}


const wchar_t *GetCommaWord(const wchar_t *Src, string &strWord,wchar_t Separator)
{
  int WordPos,SkipBrackets;
  if (*Src==0)
    return(NULL);
  SkipBrackets=FALSE;

  wchar_t *lpwszWord = strWord.GetBuffer (StrLength(Src)+1); //BUGBUG

  for (WordPos=0;*Src!=0;Src++,WordPos++)
  {
    if (*Src==L'[' && wcschr(Src+1,L']')!=NULL)
      SkipBrackets=TRUE;
    if (*Src==L']')
      SkipBrackets=FALSE;
    if (*Src==Separator && !SkipBrackets)
    {
      lpwszWord[WordPos]=0;
      Src++;
      while (IsSpace(*Src))
        Src++;

      strWord.ReleaseBuffer ();
      return(Src);
    }
    else
      lpwszWord[WordPos]=*Src;
  }
  lpwszWord[WordPos]=0;

  strWord.ReleaseBuffer ();

  return(Src);
}


BOOL IsCaseMixed (
	const string &strSrc
	)
{
	const wchar_t *lpwszSrc = (const wchar_t*)strSrc;

	while ( *lpwszSrc && !IsAlpha (*lpwszSrc) )
		lpwszSrc++;

	int Case = IsLower (*lpwszSrc);

	while ( *(lpwszSrc++) )
		if ( IsAlpha(*lpwszSrc) && (IsLower(*lpwszSrc) != Case) )
			return TRUE;

	return FALSE;
}

BOOL IsCaseLower (
	const string &strSrc
	)
{
	const wchar_t *lpwszSrc = (const wchar_t*)strSrc;

	while ( *lpwszSrc )
	{
		if ( !IsLower (*lpwszSrc) )
			return FALSE;

		lpwszSrc++;
	}

	return TRUE;
}



void WINAPI Unquote(wchar_t *Str)
{
  if (!Str)
    return;
  wchar_t *Dst=Str;
  while (*Str)
  {
    if (*Str!=L'\"')
      *Dst++=*Str;
    Str++;
  }
  *Dst=0;
}


void WINAPI Unquote(string &strStr)
{
  wchar_t *Dst = strStr.GetBuffer ();
  const wchar_t *Str = Dst;

  while (*Str)
  {
    if (*Str!=L'\"')
      *Dst++=*Str;
    Str++;
  }
  *Dst=0;

  strStr.ReleaseBuffer ();
}


void UnquoteExternal(string &strStr)
{
  if (strStr.At(0) == L'\"' && strStr.At(strStr.GetLength()-1) == L'\"')
  {
    strStr.SetLength(strStr.GetLength()-1);
    strStr.LShift(1);
  }
}


/* FileSizeToStr()
   Форматирование размера файла в удобочитаемый вид.
*/
#define MAX_KMGTBSTR_SIZE 16
static wchar_t KMGTbStrW[5][2][MAX_KMGTBSTR_SIZE]={0};

void __PrepareKMGTbStr(void)
{
  for(int I=0; I < 5; ++I)
  {
    xwcsncpy(KMGTbStrW[I][0],MSG(MListBytes+I),MAX_KMGTBSTR_SIZE-1);
    wcscpy(KMGTbStrW[I][1],KMGTbStrW[I][0]);
    CharLowerW (KMGTbStrW[I][0]);
    CharUpperW (KMGTbStrW[I][1]);
  }
}


string & WINAPI FileSizeToStr(string &strDestStr, unsigned __int64 Size, int Width, int ViewFlags)
{
  string strStr;
  unsigned __int64 Divider;
  int IndexDiv, IndexB;

  // подготовительные мероприятия
  if(KMGTbStrW[0][0][0] == 0)
  {
    __PrepareKMGTbStr();
  }

  int Commas=(ViewFlags & COLUMN_COMMAS);
  int FloatSize=(ViewFlags & COLUMN_FLOATSIZE);
  int Economic=(ViewFlags & COLUMN_ECONOMIC);
  int UseMinSizeIndex=(ViewFlags & COLUMN_MINSIZEINDEX);
  int MinSizeIndex=(ViewFlags & COLUMN_MINSIZEINDEX_MASK)+1;
  int ShowBytesIndex=(ViewFlags & COLUMN_SHOWBYTESINDEX);

  if (ViewFlags & COLUMN_THOUSAND)
  {
    Divider=_ui64(1000);
    IndexDiv=0;
  }
  else
  {
    Divider=_ui64(1024);
    IndexDiv=1;
  }

  unsigned __int64 Sz = Size, Divider2 = Divider/2, Divider64 = Divider, OldSize;

  if (FloatSize)
  {
    unsigned __int64 Divider64F = 1, Divider64F_mul = _ui64(1000), Divider64F2 = _ui64(1), Divider64F2_mul = Divider;
    //выравнивание идёт по 1000 но само деление происходит на Divider
    //например 999 bytes покажутся как 999 а вот 1000 bytes уже покажутся как 0.97 K
    for (IndexB=0; IndexB<4; IndexB++)
    {
      if (Sz < Divider64F*Divider64F_mul)
        break;
      Divider64F = Divider64F*Divider64F_mul;
      Divider64F2  = Divider64F2*Divider64F2_mul;
    }
    if (IndexB==0)
      strStr.Format (L"%d", (DWORD)Sz);
    else
    {
      Sz = (OldSize=Sz) / Divider64F2;
      OldSize = (OldSize % Divider64F2) / (Divider64F2 / Divider64F2_mul);
      DWORD Decimal = (DWORD)(0.5+(double)(DWORD)OldSize/(double)Divider*100.0);
      if (Decimal >= 100)
      {
        Decimal -= 100;
        Sz++;
      }
      strStr.Format (L"%d.%02d", (DWORD)Sz,Decimal);
      FormatNumber(strStr,strStr,2);
    }
    if (IndexB>0 || ShowBytesIndex)
    {
      Width-=(Economic?1:2);
      if (Width<0)
        Width=0;
      if (Economic)
        strDestStr.Format (L"%*.*s%1.1s",Width,Width,(const wchar_t *)strStr,KMGTbStrW[IndexB][IndexDiv]);
      else
        strDestStr.Format (L"%*.*s %1.1s",Width,Width,(const wchar_t *)strStr,KMGTbStrW[IndexB][IndexDiv]);
    }
    else
      strDestStr.Format (L"%*.*s",Width,Width,(const wchar_t *)strStr);

    return strDestStr;
  }

  if (Commas)
    InsertCommas(Sz,strStr);
  else
    strStr.Format (L"%I64u", Sz);

  if ((!UseMinSizeIndex && strStr.GetLength()<=static_cast<size_t>(Width)) || Width<5)
  {
    if (ShowBytesIndex)
    {
      Width-=(Economic?1:2);
      if (Width<0)
        Width=0;
      if (Economic)
        strDestStr.Format (L"%*.*s%1.1s",Width,Width,(const wchar_t *)strStr,KMGTbStrW[0][IndexDiv]);
      else
        strDestStr.Format (L"%*.*s %1.1s",Width,Width,(const wchar_t *)strStr,KMGTbStrW[0][IndexDiv]);
    }
    else
      strDestStr.Format (L"%*.*s",Width,Width,(const wchar_t *)strStr);
  }
  else
  {
    Width-=(Economic?1:2);
    IndexB=0;
    do{
      //Sz=(Sz+Divider2)/Divider64;
      Sz = (OldSize=Sz) / Divider64;
      if ((OldSize % Divider64) > Divider2)
        ++Sz;

      IndexB++;

      if (Commas)
        InsertCommas(Sz,strStr);
      else
        strStr.Format (L"%I64u",Sz);
    } while((UseMinSizeIndex && IndexB<MinSizeIndex) || strStr.GetLength() > static_cast<size_t>(Width));

    if (Economic)
      strDestStr.Format (L"%*.*s%1.1s",Width,Width,(const wchar_t*)strStr,KMGTbStrW[IndexB][IndexDiv]);
    else
      strDestStr.Format (L"%*.*s %1.1s",Width,Width,(const wchar_t*)strStr,KMGTbStrW[IndexB][IndexDiv]);
  }
  return strDestStr;
}



// вставить с позиции Pos в Str строку InsStr (размером InsSize байт)
// если InsSize = 0, то... вставлять все строку InsStr
// возвращает указатель на Str

wchar_t *InsertString(wchar_t *Str,int Pos,const wchar_t *InsStr,int InsSize)
{
  int InsLen=StrLength(InsStr);
  if(InsSize && InsSize < InsLen)
    InsLen=InsSize;
  wmemmove(Str+Pos+InsLen, Str+Pos, (StrLength(Str+Pos)+1));
  wmemcpy(Str+Pos, InsStr, InsLen);
  return Str;
}


// Заменить в строке Str Count вхождений подстроки FindStr на подстроку ReplStr
// Если Count < 0 - заменять "до полной победы"
// Return - количество замен
int ReplaceStrings(string &strStr,const wchar_t *FindStr,const wchar_t *ReplStr,int Count,BOOL IgnoreCase)
{
  int I=0, J=0, Res;
  int LenReplStr=StrLength(ReplStr);
  int LenFindStr=StrLength(FindStr);
  int L=(int)strStr.GetLength ();

  wchar_t *Str = strStr.GetBuffer (1024); //BUGBUG!!!

  while(I <= L-LenFindStr)
  {
    Res=IgnoreCase?StrCmpNI(Str+I, FindStr, LenFindStr):StrCmpN(Str+I, FindStr, LenFindStr);
    if(Res == 0)
    {
      if(LenReplStr > LenFindStr)
        wmemmove(Str+I+(LenReplStr-LenFindStr),Str+I,(StrLength(Str+I)+1)); // >>
      else if(LenReplStr < LenFindStr)
        wmemmove(Str+I,Str+I+(LenFindStr-LenReplStr),(StrLength(Str+I+(LenFindStr-LenReplStr))+1)); //??
      wmemcpy(Str+I,ReplStr,LenReplStr);
      I += LenReplStr;
      if(++J == Count && Count > 0)
        break;
    }
    else
      I++;
    L=StrLength(Str);
  }

  strStr.ReleaseBuffer ();

  return J;
}

/*
From PHP 4.x.x
Форматирует исходный текст по заданной ширине, используя
разделительную строку. Возвращает строку SrcText свёрнутую
в колонке, заданной параметром Width. Строка рубится при
помощи строки Break.

Разбивает на строки с выравниваением влево.

Если параметр Flahs & FFTM_BREAKLONGWORD, то строка всегда
сворачивается по заданной ширине. Так если у вас есть слово,
которое больше заданной ширины, то оно будет разрезано на части.

Example 1.
FarFormatText("Пример строки, которая будет разбита на несколько строк по ширине в 20 символов.", 20 ,Dest, "\n", 0);
Этот пример вернет:
---
Пример строки,
которая будет
разбита на
несколько строк по
ширине в 20
символов.
---

Example 2.
FarFormatText( "Эта строка содержит оооооооооооооччччччччеееень длиное слово", 9, Dest, NULL, FFTM_BREAKLONGWORD);
Этот пример вернет:

---
Эта
строка
содержит
ооооооооо
ооооччччч
чччеееень
длиное
слово
---

*/

string& WINAPI FarFormatText(const wchar_t *SrcText,     // источник
                           int Width,               // заданная ширина
                           string &strDestText,          // приемник
                           const wchar_t* Break,       // брик, если = NULL, то принимается '\n'
                           DWORD Flags)             // один из FFTM_*
{
  const wchar_t *breakchar;
  breakchar = Break?Break:L"\n";

  if(!SrcText || !*SrcText)
  {
        strDestText = L"";
        return strDestText;
  }

  string strSrc = SrcText; //copy string in case of SrcText == strDestText

  if(!wcspbrk(strSrc,breakchar) && strSrc.GetLength() <= static_cast<size_t>(Width))
  {
    strDestText = strSrc;
    return strDestText;
  }

  long i=0, l=0, pgr=0, last=0;
  wchar_t *newtext;

  const wchar_t *text= strSrc;
  long linelength = Width;
  int breakcharlen = StrLength(breakchar);
  int docut = Flags&FFTM_BREAKLONGWORD?1:0;

  /* Special case for a single-character break as it needs no
     additional storage space */

  if (breakcharlen == 1 && docut == 0)
  {
    newtext = xf_wcsdup (text);
    if(!newtext)
    {
        strDestText = L"";
        return strDestText;
    }

    while (newtext[i] != L'\0')
    {
      /* prescan line to see if it is greater than linelength */
      l = 0;
      while (newtext[i+l] != breakchar[0])
      {
        if (newtext[i+l] == L'\0')
        {
          l--;
          break;
        }
        l++;
      }
      if (l >= linelength)
      {
        pgr = l;
        l = linelength;
        /* needs breaking; work backwards to find previous word */
        while (l >= 0)
        {
          if (newtext[i+l] == L' ')
          {
            newtext[i+l] = breakchar[0];
            break;
          }
          l--;
        }
        if (l == -1)
        {
          /* couldn't break is backwards, try looking forwards */
          l = linelength;
          while (l <= pgr)
          {
            if(newtext[i+l] == L' ')
            {
              newtext[i+l] = breakchar[0];
              break;
            }
            l++;
          }
        }
      }
      i += l+1;
    }
  }
  else
  {
    /* Multiple character line break */
    newtext = (wchar_t*)xf_malloc((strSrc.GetLength() * (breakcharlen+1)+1)*sizeof(wchar_t));
    if(!newtext)
    {
        strDestText = L"";
        return strDestText;
    }

    newtext[0] = L'\0';

    i = 0;
    while (text[i] != L'\0')
    {
      /* prescan line to see if it is greater than linelength */
      l = 0;
      while (text[i+l] != L'\0')
      {
        if (text[i+l] == breakchar[0])
        {
          if (breakcharlen == 1 || StrCmpN(text+i+l, breakchar, breakcharlen)==0)
            break;
        }
        l++;
      }
      if (l >= linelength)
      {
        pgr = l;
        l = linelength;

        /* needs breaking; work backwards to find previous word */
        while (l >= 0)
        {
          if (text[i+l] == L' ')
          {
            wcsncat(newtext, text+last, i+l-last);
            wcscat(newtext, breakchar);
            last = i + l + 1;
            break;
          }
          l--;
        }
        if (l == -1)
        {
          /* couldn't break it backwards, try looking forwards */
          l = linelength - 1;
          while (l <= pgr)
          {
            if (docut == 0)
            {
              if (text[i+l] == L' ')
              {
                wcsncat(newtext, text+last, i+l-last);
                wcscat(newtext, breakchar);
                last = i + l + 1;
                break;
              }
            }
            if (docut == 1)
            {
              if (text[i+l] == L' ' || l > i-last)
              {
                wcsncat(newtext, text+last, i+l-last+1);
                wcscat(newtext, breakchar);
                last = i + l + 1;
                break;
              }
            }
            l++;
          }
        }
        i += l+1;
      }
      else
      {
        i += (l ? l : 1);
      }
    }

    if (i+l > last)
    {
      wcscat(newtext, text+last);
    }
  }

  strDestText = newtext;
  xf_free(newtext);
  return strDestText;
}

BOOL IsWordDiv(const wchar_t *WordDiv, wchar_t Chr)
{
  return NULL!=wcschr (WordDiv, Chr);
}


/*
  Ptr=CalcWordFromString(Str,I,&Start,&End);
  xstrncpy(Dest,Ptr,End-Start+1);
  Dest[End-Start+1]=0;

// Параметры:
//   WordDiv  - набор разделителей слова в кодировке OEM
  возвращает указатель на начало слова
*/
const wchar_t * const CalcWordFromString(const wchar_t *Str,int CurPos,int *Start,int *End, const wchar_t *WordDiv0)
{
  int I, J, StartWPos, EndWPos;
  DWORD DistLeft, DistRight;
  int StrSize=StrLength(Str);

  if(CurPos >= StrSize)
    return NULL;

  string strWordDiv(WordDiv0);
  strWordDiv += L" \t\n\r";

  if(IsWordDiv(strWordDiv,Str[CurPos]))
  {
    // вычисляем дистанцию - куда копать, где ближе слово - слева или справа
    I=J=CurPos;

    // копаем влево
    DistLeft=-1;
    while(I >= 0 && IsWordDiv(strWordDiv,Str[I]))
    {
      DistLeft++;
      I--;
    }
    if(I < 0)
      DistLeft=-1;

    // копаем вправо
    DistRight=-1;
    while(J < StrSize && IsWordDiv(strWordDiv,Str[J]))
    {
      DistRight++;
      J++;
    }
    if(J >= StrSize)
      DistRight=-1;

    if(DistLeft > DistRight) // ?? >=
      EndWPos=StartWPos=J;
    else
      EndWPos=StartWPos=I;
  }
  else // здесь все оби, т.е. стоим на буковке
    EndWPos=StartWPos=CurPos;

  if(StartWPos < StrSize)
  {
    while(StartWPos >= 0)
      if(IsWordDiv(strWordDiv,Str[StartWPos]))
      {
        StartWPos++;
        break;
      }
      else
        StartWPos--;
    while(EndWPos < StrSize)
      if(IsWordDiv(strWordDiv,Str[EndWPos]))
      {
        EndWPos--;
        break;
      }
      else
        EndWPos++;
  }

  if(StartWPos < 0)
    StartWPos=0;
  if(EndWPos >= StrSize)
    EndWPos=StrSize;

  *Start=StartWPos;
  *End=EndWPos;

  return Str+StartWPos;
}

BOOL TestParentFolderName(const wchar_t *Name)
{
	return Name[0] == L'.' && Name[1] == L'.' && (!Name[2] || (IsSlash(Name[2]) && !Name[3]));
}

BOOL TestCurrentFolderName(const wchar_t *Name)
{
	return Name[0] == L'.' && (!Name[1] || (IsSlash(Name[1]) && !Name[2]));
}

bool CutToSlash(string &strStr, bool bInclude)
{
  size_t pos;
	bool bFound=LastSlash(strStr,pos);
	if(bFound)
	{
		if ( bInclude )
			strStr.SetLength(pos);
		else
			strStr.SetLength(pos+1);
	}

  return bFound;
}

string& CutToNameUNC(string &strPath)
{
  wchar_t *lpwszPath = strPath.GetBuffer ();

  if (IsSlash(lpwszPath[0]) && IsSlash(lpwszPath[1]))
  {
    lpwszPath+=2;
    for (int i=0; i<2; i++)
    {
      while (*lpwszPath && !IsSlash(*lpwszPath))
        lpwszPath++;
      if (*lpwszPath)
        lpwszPath++;
    }
  }

  wchar_t *lpwszNamePtr = lpwszPath;

  while ( *lpwszPath )
  {
    if (IsSlash(*lpwszPath) || (*lpwszPath==L':' && lpwszPath == lpwszNamePtr+1) )
      lpwszNamePtr = lpwszPath+1;

    lpwszPath++;
  }

  *lpwszNamePtr = 0;

  strPath.ReleaseBuffer ();

  return strPath;

}

string& CutToFolderNameIfFolder(string &strPath)
{
  wchar_t *lpwszPath = strPath.GetBuffer ();

  wchar_t *lpwszNamePtr=lpwszPath, *lpwszprevNamePtr=lpwszPath;

  while (*lpwszPath)
  {
    if (IsSlash(*lpwszPath) || (*lpwszPath==L':' && lpwszPath==lpwszNamePtr+1))
    {
      lpwszprevNamePtr=lpwszNamePtr;
      lpwszNamePtr=lpwszPath+1;
    }
    ++lpwszPath;
  }

  if (*lpwszNamePtr)
    *lpwszNamePtr=0;
  else
    *lpwszprevNamePtr=0;

  strPath.ReleaseBuffer ();

  return strPath;
}

string& HiText2Str(string& strDest, const wchar_t *Str)
{
  const wchar_t *ChPtr;
  strDest = Str;
  if ((ChPtr=wcschr(Str,L'&')) != NULL)
  {
    /*
       &&      = '&'
       &&&     = '&'
                  ^H
       &&&&    = '&&'
       &&&&&   = '&&'
                  ^H
       &&&&&&  = '&&&'
    */
    int I=0;
    const wchar_t *ChPtr2=ChPtr;
    while (*ChPtr2++ == L'&')
      ++I;

    if (I&1) // нечет?
    {
      strDest.SetLength(ChPtr-Str);

      if (ChPtr[1])
      {
        wchar_t Chr[2];
        Chr[0]=ChPtr[1]; Chr[1]=0;
        strDest+=Chr;

        string strText = (ChPtr+1);

        ReplaceStrings(strText,L"&&",L"&",-1);

        strDest+=(const wchar_t*)strText+1;
      }
    }
    else
    {
      ReplaceStrings(strDest,L"&&",L"&",-1);
    }
  }
  return strDest;
}

const wchar_t* PointToNameUNC(const wchar_t *lpwszPath)
{
  if ( !lpwszPath )
    return NULL;

  if (IsSlash(lpwszPath[0]) && IsSlash(lpwszPath[1]))
  {
    lpwszPath+=2;
    for (int i=0; i<2; i++)
    {
      while (*lpwszPath && !IsSlash(*lpwszPath))
        lpwszPath++;
      if (*lpwszPath)
        lpwszPath++;
    }
  }

  const wchar_t *lpwszNamePtr = lpwszPath;

  while ( *lpwszPath )
  {
    if (IsSlash(*lpwszPath) || (*lpwszPath==L':' && lpwszPath == lpwszNamePtr+1) )
      lpwszNamePtr = lpwszPath+1;

    lpwszPath++;
  }
  return lpwszNamePtr;
}

string& ReplaceSlashToBSlash(string& strStr)
{
	wchar_t *lpwszStr = strStr.GetBuffer ();

	while ( *lpwszStr )
	{
		if ( *lpwszStr == L'/' )
			*lpwszStr = L'\\';
		lpwszStr++;
	}

	strStr.ReleaseBuffer ();

	return strStr;
}

bool CheckFileSizeStringFormat(const wchar_t *FileSizeStr)
{
//проверяет если формат строки такой: [0-9]+[BbKkMmGgTt]?

  const wchar_t *p = FileSizeStr;

  while (iswdigit(*p))
    p++;

  if (p == FileSizeStr)
    return false;

  if (*p)
  {
    if (*(p+1))
      return false;

    if (!StrStrI(L"BKMGT", p))
      return false;
  }

  return true;
}

unsigned __int64 ConvertFileSizeString(const wchar_t *FileSizeStr)
{
  if (!CheckFileSizeStringFormat(FileSizeStr))
    return _ui64(0);

  unsigned __int64 n = _wtoi64(FileSizeStr);

  wchar_t c = Upper(FileSizeStr[StrLength(FileSizeStr)-1]);

  switch (c)
  {
    case L'K':
      n <<= 10;
      break;

    case L'M':
      n <<= 20;
      break;

    case L'G':
      n <<= 30;
      break;

    case L'T':
      n <<= 40;
      break;
  }

  return n;
}

wchar_t *ReadString (FILE *file, wchar_t *lpwszDest, int nDestLength, int nCodePage)
{
    char *lpDest = (char*)xf_malloc ((nDestLength+1)*3); //UTF-8, up to 3 bytes per char support

    memset (lpDest, 0, (nDestLength+1)*3);
    memset (lpwszDest, 0, nDestLength*sizeof (wchar_t));

    if ( (nCodePage == CP_UNICODE) || (nCodePage == CP_REVERSEBOM) )
    {
        if ( !fgetws (lpwszDest, nDestLength, file) )
        {
            xf_free (lpDest);
            return NULL;
        }

        if ( nCodePage == CP_REVERSEBOM )
        {
            swab ((char*)lpwszDest, (char*)lpwszDest, nDestLength*sizeof (wchar_t));

            wchar_t *Ch = lpwszDest;
            int nLength = Min (static_cast<int>(wcslen (lpwszDest)), nDestLength);

            while ( *Ch )
            {
                if ( *Ch == L'\n' )
                {
                    *(Ch+1) = 0;
                    break;
                }

                Ch++;
            }

            int nNewLength = Min (static_cast<int>(wcslen (lpwszDest)), nDestLength);

            fseek (file, (nNewLength-nLength)*sizeof (wchar_t), SEEK_CUR);
        }

    }
    else

    if ( nCodePage == CP_UTF8 )
    {
        if ( fgets (lpDest, nDestLength*3, file) )
            MultiByteToWideChar (CP_UTF8, 0, lpDest, -1, lpwszDest, nDestLength);
        else
        {
            xf_free (lpDest);
            return NULL;
        }

    }
    else

    if ( nCodePage != -1 )
    {
        if ( fgets (lpDest, nDestLength, file) )
            MultiByteToWideChar (nCodePage, 0, lpDest, -1, lpwszDest, nDestLength);
        else
        {
            xf_free (lpDest);
            return NULL;
        }
    }

    xf_free (lpDest);

    return lpwszDest;
}

const wchar_t *FirstSlash(const wchar_t *String)
{
	do
	{
		if(IsSlash(*String))
			return String;
	}
	while (*String++);
	return NULL;
}

bool FirstSlash(const wchar_t *String,size_t &pos)
{
	bool Ret=false;
	const wchar_t *Ptr=FirstSlash(String);
	if(Ptr)
	{
		pos=Ptr-String;
		Ret=true;
	}
	return Ret;
}

const wchar_t *LastSlash(const wchar_t *String)
{
	const wchar_t *Start = String;
	while (*String++)
		;
	while (--String!=Start && !IsSlash(*String))
		;
	return IsSlash(*String)?String:NULL;
}

bool LastSlash(const wchar_t *String,size_t &pos)
{
	bool Ret=false;
	const wchar_t *Ptr=LastSlash(String);
	if(Ptr)
	{
		pos=Ptr-String;
		Ret=true;
	}
	return Ret;
}
