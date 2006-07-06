/*
strmix.cpp

���� ������ ��������������� ������� �� ������ �� ��������

*/

/* Revision: 1.91 07.07.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "fn.hpp"
#include "plugin.hpp"
#include "lang.hpp"

string &InsertCommasW(unsigned __int64 li,string &strDest)
{
   strDest.Format (L"%I64u", li);

   wchar_t *lpwszDest = strDest.GetBuffer();

   for (int I=wcslen(lpwszDest)-4;I>=0;I-=3)
   {
      if (lpwszDest[I])
      {
        wmemmove(lpwszDest+I+2,lpwszDest+I+1,wcslen(lpwszDest+I));
        lpwszDest[I+1]=L',';
      }
   }

   strDest.ReleaseBuffer();

   return strDest;
}


char* __stdcall PointToName (char *lpPath)
{
  if(!lpPath)
    return NULL;

  const char *NamePtr=lpPath;
  while (*lpPath)
  {
    if (*lpPath=='\\' || *lpPath=='/' || *lpPath==':' && lpPath==NamePtr+1)
      NamePtr=lpPath+1;
    lpPath++;
  }
  return (char*)NamePtr;
}


const char* __stdcall PointToName (const char *lpPath)
{
  if(!lpPath)
    return NULL;

  const char *NamePtr=lpPath;
  while (*lpPath)
  {
    if (*lpPath=='\\' || *lpPath=='/' || *lpPath==':' && lpPath==NamePtr+1)
      NamePtr=lpPath+1;
    lpPath++;
  }
  return NamePtr;
}

const wchar_t* __stdcall PointToNameW (const wchar_t *lpwszPath)
{
  if ( !lpwszPath )
    return NULL;

  const wchar_t *lpwszNamePtr = lpwszPath;

  while ( *lpwszPath )
  {
    if ( *lpwszPath==L'\\' || *lpwszPath==L'/' || *lpwszPath==L':' && lpwszPath == lpwszNamePtr+1 )
      lpwszNamePtr = lpwszPath+1;

    lpwszPath++;
  }
  return lpwszNamePtr;
}


//   ������ PointToName, ������ ��� ����� ����
//   "name\" (������������ �� ����) ���������� ��������� �� name, � �� �� ������
//   ������

const wchar_t* __stdcall PointToFolderNameIfFolderW (const wchar_t *Path)
{
  if(!Path)
    return NULL;

  const wchar_t *NamePtr=Path, *prevNamePtr=Path;

  while (*Path)
  {
    if (*Path==L'\\' || *Path==L'/' ||
        *Path==L':' && Path==NamePtr+1)
    {
      prevNamePtr=NamePtr;
      NamePtr=Path+1;
    }
    ++Path;
  }
  return const_cast<const wchar_t*>((*NamePtr)?NamePtr:prevNamePtr);
}


/* $ 10.05.2003 IS
   + �������� CmpName �� ���� ������ �������� skippath ������
   - ������: *Name*.* �� �������� Name
*/

// IS: ��� �������� ���� ������� ��������� � ������, �� ������������
// IS: "�������" ����� �� ��� �������, � CmpName (�� ���� �����������
// IS: ����� CmpName_Body)
int CmpName_Body(const char *pattern,const char *string)
{
  unsigned char stringc,patternc,rangec;
  int match;

  for (;; ++string)
  {
    /* $ 01.05.2001 DJ
       ���������� ���������� ������
    */
    stringc=LocalUpperFast(*string);
    patternc=LocalUpperFast(*pattern++);
    /* DJ $ */
    switch (patternc)
    {
      case 0:
        return(stringc==0);
      case '?':
        if (stringc == 0)
          return(FALSE);

        break;
      case '*':
        if (!*pattern)
          return(TRUE);

        /* $ 01.05.2001 DJ
           ���������������� ����� �������� � ��� ���� � �����������
           �������
        */
        if (*pattern=='.')
        {
          if (pattern[1]=='*' && pattern[2]==0)
            return(TRUE);
          if (strpbrk (pattern, "*?[") == NULL)
          {
            const char *dot = strrchr (string, '.');
            if (pattern[1] == 0)
              return (dot==NULL || dot[1]==0);
            const char *patdot = strchr (pattern+1, '.');
            if (patdot != NULL && dot == NULL)
              return(FALSE);
            if (patdot == NULL && dot != NULL)
              return(LocalStricmp (pattern+1,dot+1) == 0);
          }
        }
        /* DJ $ */

        while (*string)
        {
          if (CmpName(pattern,string++,FALSE))
            return(TRUE);
        }
        return(FALSE);
      case '[':
        if (strchr(pattern,']')==NULL)
        {
          if (patternc != stringc)
            return (FALSE);
          break;
        }
        if (*pattern && *(pattern+1)==']')
        {
          if (*pattern!=*string)
            return(FALSE);
          pattern+=2;
          break;
        }
        match = 0;
        while ((rangec = LocalUpper(*pattern++))!=0)
        {
          if (rangec == ']')
            if (match)
              break;
            else
              return(FALSE);
          if (match)
            continue;
          if (rangec == '-' && *(pattern - 2) != '[' && *pattern != ']')
          {
            match = (stringc <= LocalUpper(*pattern) &&
                     LocalUpper(*(pattern - 2)) <= stringc);
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
          if (patternc=='.' && stringc==0 && !CmpNameSearchMode)
            return(*pattern!='.' && CmpName(pattern,string));
          else
            return(FALSE);
        break;
    }
  }
}


int CmpName_BodyW(const wchar_t *pattern,const wchar_t *str)
{
  wchar_t stringc,patternc,rangec;
  int match;

  for (;; ++str)
  {
    /* $ 01.05.2001 DJ
       ���������� ���������� ������
    */
    stringc=LocalUpperW(*str);
    patternc=LocalUpperW(*pattern++);
    /* DJ $ */
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
           ���������������� ����� �������� � ��� ���� � �����������
           �������
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
              return(LocalStricmpW (pattern+1,dot+1) == 0);
          }
        }
        /* DJ $ */

        while (*str)
        {
          if (CmpNameW(pattern,str++,FALSE))
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
        while ((rangec = LocalUpperW(*pattern++))!=0)
        {
          if (rangec == L']')
            if (match)
              break;
            else
              return(FALSE);
          if (match)
            continue;
          if (rangec == L'-' && *(pattern - 2) != L'[' && *pattern != L']')
          {
            match = (stringc <= LocalUpperW(*pattern) &&
                     LocalUpperW(*(pattern - 2)) <= stringc);
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
          if (patternc==L'.' && stringc==0 && !CmpNameSearchMode)
            return(*pattern!=L'.' && CmpNameW(pattern,str));
          else
            return(FALSE);
        break;
    }
  }
}


// IS: ������� ��� �������� ����, ������������ ��
int CmpName(const char *pattern,const char *str,int skippath)
{
  if (skippath)
    str=PointToName(const_cast<char*>(str));
  return CmpName_Body(pattern,str);
}

// IS: ������� ��� �������� ����, ������������ ��
int CmpNameW(const wchar_t *pattern,const wchar_t *str,int skippath)
{
  if (skippath)
    str=PointToNameW(const_cast<wchar_t*>(str));
  return CmpName_BodyW(pattern,str);
}



int ConvertWildcardsW (const wchar_t *SrcName,string &strDest, int SelectedFolderNameLength)
{
    string strWildName;
    wchar_t *DestNamePtr, *SrcNamePtr;
    const wchar_t *CurWildPtr;

    wchar_t *PartAfterFolderName;  //BUGBUG string
    wchar_t *PartBeforeName; //BUGBUG string
    wchar_t *Src = _wcsdup (SrcName);
  //char PartBeforeName[NM],PartAfterFolderName[NM];

    DestNamePtr = strDest.GetBuffer (strDest.GetLength()+wcslen(SrcName)+1); //???
    DestNamePtr = (wchar_t*)PointToNameW(DestNamePtr);

    strWildName = DestNamePtr;

    if (wcschr(strWildName, L'*')==NULL && wcschr(strWildName, L'?')==NULL)
    {
        strDest.ReleaseBuffer ();
        return(FALSE);
    }

    if (SelectedFolderNameLength!=0)
    {
        PartAfterFolderName = _wcsdup (Src+SelectedFolderNameLength);
        Src[SelectedFolderNameLength]=0;
    }

    SrcNamePtr = (wchar_t*)PointToNameW (Src);

    int BeforeNameLength=DestNamePtr==(const wchar_t*)strDest ? SrcNamePtr-Src:0; //BUGBUG strDest

    PartBeforeName = (wchar_t*)xf_malloc ((BeforeNameLength+1)*sizeof (wchar_t));

    wcsncpy(PartBeforeName, Src, BeforeNameLength);
    PartBeforeName[BeforeNameLength]=0;

    wchar_t *SrcNameDot=wcsrchr(SrcNamePtr, L'.');

    CurWildPtr = strWildName;

  while (*CurWildPtr)
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
      case '.':
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

  *DestNamePtr=0;
  if (DestNamePtr!=(const wchar_t*)strDest && *(DestNamePtr-1)==L'.') //BUGBUG strDest
    *(DestNamePtr-1)=0;

  strDest.ReleaseBuffer ();

  if (*PartBeforeName)
    strDest = PartBeforeName+strDest; //BUGBUG not sure
  if (SelectedFolderNameLength!=0)
    strDest += PartAfterFolderName; //BUGBUG, was src

  return(TRUE);
}


/* IS 10.05.2003 $ */

/* $ 09.10.2000 IS
    ��������� ������ ����� �� �����
    (���� �� ShellCopy::ShellCopyConvertWildcards)
*/
// �� ������ ����� ����� (Src) � ����� (Dest) ���������� ����� ���
// SelectedFolderNameLength - ����� ��������. ��������, ����
// ������� dir1, � � ��� ���� file1. ����� ������������� ��� �� ����� ��� dir1.
// ��������� ����� ���� ����������: Src="dir1", SelectedFolderNameLength=0
// ��� Src="dir1\\file1", � SelectedFolderNameLength=4 (����� "dir1")
/* IS $ */

wchar_t * WINAPI InsertQuoteW(wchar_t *Str)
{
  int l = wcslen(Str);
  if(*Str != L'"')
  {
    memmove(Str+1,Str,(++l)*sizeof (wchar_t));
    *Str=L'"';
  }
  if(Str[l-1] != L'"')
  {
    Str[l++] = L'\"';
    Str[l] = 0;
  }
  return Str;
}

wchar_t* WINAPI QuoteSpaceW (wchar_t *Str)
{
  if ( wcspbrk(Str, Opt.strQuotedSymbols) != NULL )
    InsertQuoteW(Str);

  return Str;
}


string& InsertQuoteW(string &strStr)
{
  wchar_t *Str = strStr.GetBuffer (strStr.GetLength()+2);

  InsertQuoteW (Str);

  strStr.ReleaseBuffer ();

  return strStr;
}

string &QuoteSpaceW(string &strStr)
{
    if ( wcspbrk(strStr, Opt.strQuotedSymbols) != NULL)
        InsertQuoteW(strStr);

    return strStr;
}


wchar_t*  WINAPI QuoteSpaceOnlyW (wchar_t *Str)
{
  if (wcschr(Str,L' ')!=NULL)
    InsertQuoteW(Str);

  return Str;
}


string& WINAPI QuoteSpaceOnlyW(string &strStr)
{
  if (wcschr(strStr,L' ')!=NULL)
    InsertQuoteW(strStr);
  return(strStr);
}


string& __stdcall TruncStrFromEndW (string &strStr, int MaxLength)
{
  if( !strStr.IsEmpty() )
  {
    int Length = wcslen(strStr);

    if (Length > MaxLength)
    {
      wchar_t *lpwszStr = strStr.GetBuffer ();

      if (MaxLength>3)
        memcpy(lpwszStr+MaxLength-3, L"...", 6);

      lpwszStr[MaxLength]=0;

      strStr.ReleaseBuffer ();
    }
  }
  return strStr;
}


wchar_t* WINAPI TruncStrW (wchar_t *Str,int MaxLength)
{
  if(Str)
  {
    int Length;
    if (MaxLength<0)
      MaxLength=0;
    if ((Length=wcslen(Str))>MaxLength)
    {
      if (MaxLength>3)
      {
        wchar_t *MovePos = Str+Length-MaxLength+3;
        memmove(Str+3, MovePos, (wcslen(MovePos)+1)*sizeof (wchar_t));
        memcpy(Str,L"...",6);
      }

      Str[MaxLength]=0;
    }
  }

  return Str;
}


string& __stdcall TruncStrW (string &strStr, int MaxLength)
{
    wchar_t *lpwszBuffer = strStr.GetBuffer ();

    TruncStrW (lpwszBuffer, MaxLength);

    strStr.ReleaseBuffer ();

    return strStr;
}


wchar_t* WINAPI TruncPathStrW (wchar_t *Str, int MaxLength)
{
  if (Str)
  {
    int nLength = wcslen (Str);

    if (nLength > MaxLength)
    {
      wchar_t *lpStart = NULL;

      if ( *Str && (Str[1] == L':') && (Str[2] == L'\\') )
         lpStart = Str+3;
      else
      {
        if ( (Str[0] == L'\\') && (Str[1] == L'\\') )
        {
          if ( (lpStart = wcschr (Str+2, L'\\')) != NULL )
            if ( (lpStart = wcschr (lpStart+1, L'\\')) != NULL )
              lpStart++;
        }
      }

      if ( !lpStart || (lpStart-Str > MaxLength-5) )
        return TruncStrW (Str, MaxLength);

      wchar_t *lpInPos = lpStart+3+(nLength-MaxLength);
      memmove (lpStart+3, lpInPos, (wcslen (lpInPos)+1)*sizeof (wchar_t));
      memcpy (lpStart, L"...", 6);
    }
  }

  return Str;
}


string& __stdcall TruncPathStrW (string &strStr, int MaxLength)
{
    wchar_t *lpwszStr= strStr.GetBuffer ();

    TruncPathStrW (lpwszStr, MaxLength);

    strStr.ReleaseBuffer ();

    return strStr;
}


wchar_t* WINAPI RemoveLeadingSpacesW(wchar_t *Str)
{
  wchar_t *ChPtr;
  if((ChPtr=Str) == 0)
    return NULL;

  for (; IsSpaceW(*ChPtr); ChPtr++)
         ;
  if (ChPtr!=Str)
    wmemmove(Str,ChPtr,wcslen(ChPtr)+1);
  return Str;
}


string& WINAPI RemoveLeadingSpacesW(string &strStr)
{
  wchar_t *ChPtr = strStr.GetBuffer ();
  wchar_t *Str = ChPtr;

  for (; IsSpaceW(*ChPtr); ChPtr++)
         ;
  if (ChPtr!=Str)
    memmove(Str,ChPtr,(wcslen(ChPtr)+1)*sizeof(wchar_t));

  strStr.ReleaseBuffer ();

  return strStr;
}


// ������� �������� �������
char* WINAPI RemoveTrailingSpaces(char *Str)
{
  if(!Str)
    return NULL;
  if (*Str == '\0')
    return Str;

  char *ChPtr;
  int I;

  for (ChPtr=Str+(I=strlen((char *)Str)-1); I >= 0; I--, ChPtr--)
    if (IsSpace(*ChPtr) || IsEol(*ChPtr))
      *ChPtr=0;
    else
      break;

  return Str;
}

wchar_t* WINAPI RemoveTrailingSpacesW(wchar_t *Str)
{
  if(!Str)
    return NULL;
  if (*Str == L'\0')
    return Str;

  wchar_t *ChPtr;
  int I;

  for (ChPtr=Str+(I=wcslen((wchar_t*)Str)-1); I >= 0; I--, ChPtr--)
    if (IsSpaceW(*ChPtr) || IsEolW(*ChPtr))
      *ChPtr=0;
    else
      break;

  return Str;
}


string& WINAPI RemoveTrailingSpacesW(string &strStr)
{
  if ( strStr.IsEmpty () )
    return strStr;

  wchar_t *ChPtr = strStr.GetBuffer ();
  int I;

  for (ChPtr=ChPtr+(I=wcslen(ChPtr)-1); I >= 0; I--, ChPtr--)
    if (IsSpaceW(*ChPtr) || IsEolW(*ChPtr))
      *ChPtr=0;
    else
      break;

  strStr.ReleaseBuffer ();

  return strStr;
}


wchar_t* WINAPI RemoveExternalSpacesW(wchar_t *Str)
{
  return RemoveTrailingSpacesW(RemoveLeadingSpacesW(Str));
}

string&  WINAPI RemoveExternalSpacesW(string &strStr)
{
  return RemoveTrailingSpacesW(RemoveLeadingSpacesW(strStr));
}


/* $ 02.02.2001 IS
   �������� ��������� ���������� ������� � ������. � ��������� ������
   �������������� ������ cr � lf.
*/

string& WINAPI RemoveUnprintableCharactersW (string &strStr)
{
  wchar_t *Str = strStr.GetBuffer ();
  wchar_t *p = Str;

  while(*p)
  {
     if ( IsEolW(*p) )
       *p=L' ';
     p++;
  }

  strStr.ReleaseBuffer();

  return RemoveExternalSpacesW(strStr);
}

/* IS $ */

// ������� ������ Target �� ������ Str (�����!)
char *RemoveChar(char *Str,char Target,BOOL Dup)
{
  char *Ptr = Str, *StrBegin = Str, Chr;
  while((Chr=*Str++) != 0)
  {
    if(Chr == Target)
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
  *Ptr = '\0';
  return StrBegin;
}

string &RemoveCharW(string &strStr,wchar_t Target,BOOL Dup)
{
  wchar_t *Ptr = strStr.GetBuffer ();
  wchar_t *Str = Ptr, *StrBegin = Ptr, Chr;
  while((Chr=*Str++) != 0)
  {
    if(Chr == Target)
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


int HiStrlenW(const wchar_t *Str,BOOL Dup)
{
  int Length=0;

  if(Str && *Str)
    while (*Str)
    {
      if (*Str!=L'&')
        Length++;
      else
        if(Dup && Str[1] == L'&')
        {
          Length++;
          ++Str;
        }
      Str++;
    }
  return(Length);
}


BOOL AddEndSlashW (wchar_t *Path, wchar_t TypeSlash)
{
  BOOL Result=FALSE;
  if(Path)
  {
    /* $ 06.12.2000 IS
      ! ������ ������� �������� � ������ ������ ������, ����� ����������
        ��������� ��� ������������� ��������� ����� �� �����, �������
        ����������� ����.
    */
    wchar_t *end;
    int Slash=0, BackSlash=0;
    if(!TypeSlash)
    {
      end=Path;
      while(*end)
      {
       Slash+=(*end==L'\\');
       BackSlash+=(*end==L'/');
       end++;
      }
    }
    else
    {
      end=Path+wcslen(Path);
      if(TypeSlash == L'\\')
        Slash=1;
      else
        BackSlash=1;
    }
    int Length=end-Path;
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
     if (*end!=L'\\' && *end!=L'/')
     {
       end[1]=c;
       end[2]=0;
     }
     else
       *end=c;
    }
    /* IS $ */
  }
  return Result;
}


BOOL WINAPI AddEndSlashW (wchar_t *Path)
{
    return AddEndSlashW (Path, 0);
}


BOOL AddEndSlashW (string &strPath)
{
    return AddEndSlashW (strPath, 0);
}

BOOL AddEndSlashW (
        string &strPath,
        wchar_t TypeSlash
        )
{
    BOOL Result = FALSE;

    wchar_t *lpwszPath = strPath.GetBuffer (strPath.GetLength()+1); // +spaceforslash

    Result = AddEndSlashW (lpwszPath, TypeSlash);

    strPath.ReleaseBuffer ();

    return Result;
}


BOOL WINAPI DeleteEndSlash(char *Path)
{
  if(Path)
  {
    int Length=strlen(Path)-1;
    if (Length >= 0)
    {
      if(*(Path+=Length) == '\\')
      {
        *Path=0;
        return TRUE;
      }
    }
  }
  return FALSE;
}

BOOL WINAPI DeleteEndSlashW (string &strPath)
{
  if( !strPath.IsEmpty() )
  {
    int Length=strPath.GetLength()-1;
    if (Length >= 0)
    {
      wchar_t *lpwszPath = strPath.GetBuffer ();
      if(*(lpwszPath+=Length) == L'\\') //LAME!!!
      {
        *lpwszPath=0;
        strPath.ReleaseBuffer();
        return TRUE;
      }
      strPath.ReleaseBuffer();
    }
  }
  return FALSE;
}

/*
char *NullToEmpty(char *Str)
{
  return (Str==NULL) ? "":Str;
}
*/

const char *NullToEmpty(const char *Str)
{
  return (Str==NULL) ? "":Str;
}


string& CenterStrW(const wchar_t *Src, string &strDest, int Length)
{
  int SrcLength=wcslen(Src);

  string strTempStr = Src; //���� Src == strDest, �� ���� ���������� Src!

  if (SrcLength >= Length)
  {
    /* ����� �� ���� �������� 1 �� �����, �.�. strlen �� ��������� \0
       � �� �������� ���������� ������ */
    strDest = strTempStr;

    strDest.GetBuffer (); //BUGBUG
    strDest.ReleaseBuffer (Length);
  }
  else
  {
    int Space=(Length-SrcLength)/2;

    strDest.Format (L"%*s%s%*s",Space,L"",(const wchar_t*)strTempStr,Length-Space-SrcLength,L"");
  }

  return strDest;
}


/* $ 08.04.2001 SVS
  + �������������� �������� - �����������, �� ��������� = ','
*/
const char *GetCommaWord(const char *Src,char *Word,char Separator)
{
  int WordPos,SkipBrackets;
  if (*Src==0)
    return(NULL);
  SkipBrackets=FALSE;
  for (WordPos=0;*Src!=0;Src++,WordPos++)
  {
    if (*Src=='[' && strchr(Src+1,']')!=NULL)
      SkipBrackets=TRUE;
    if (*Src==']')
      SkipBrackets=FALSE;
    if (*Src==Separator && !SkipBrackets)
    {
      Word[WordPos]=0;
      Src++;
      while (IsSpace(*Src))
        Src++;
      return(Src);
    }
    else
      Word[WordPos]=*Src;
  }
  Word[WordPos]=0;
  return(Src);
}

const wchar_t *GetCommaWordW(const wchar_t *Src, string &strWord,wchar_t Separator)
{
  int WordPos,SkipBrackets;
  if (*Src==0)
    return(NULL);
  SkipBrackets=FALSE;

  wchar_t *lpwszWord = strWord.GetBuffer (wcslen (Src));

  for (WordPos=0;*Src!=0;Src++,WordPos++)
  {
    if (*Src=='[' && wcschr(Src+1,']')!=NULL)
      SkipBrackets=TRUE;
    if (*Src==']')
      SkipBrackets=FALSE;
    if (*Src==Separator && !SkipBrackets)
    {
      lpwszWord[WordPos]=0;
      Src++;
      while (IsSpaceW(*Src))
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

/* SVS $ */

int IsCaseMixed(char *Str)
{
  while (*Str && !LocalIsalpha(*Str))
    Str++;
  int Case=LocalIslower(*Str);
  while (*(Str++))
    if (LocalIsalpha(*Str) && LocalIslower(*Str)!=Case)
      return(TRUE);
  return(FALSE);
}


int IsCaseLower(char *Str)
{
  for (;*Str!=0;Str++)
    if (LocalIsalpha(*Str) && !LocalIslower(*Str))
      return(FALSE);
  return(TRUE);
}

BOOL IsCaseMixedW (
        const string &strSrc
        )
{
    const wchar_t *lpwszSrc = (const wchar_t*)strSrc;

    while ( *lpwszSrc && !LocalIsalphaW (*lpwszSrc) )
        lpwszSrc++;

    int Case = LocalIslowerW (*lpwszSrc);

    while ( *(lpwszSrc++) )
        if ( LocalIsalphaW(*lpwszSrc) && (LocalIslowerW(*lpwszSrc) != Case) )
            return TRUE;

    return FALSE;
}

BOOL IsCaseLowerW (
    const string &strSrc
    )
{
    const wchar_t *lpwszSrc = (const wchar_t*)strSrc;

    while ( *lpwszSrc )
    {
        if ( !LocalIslowerW (*lpwszSrc) )
            return FALSE;

        lpwszSrc++;
    }

    return TRUE;
}



void WINAPI UnquoteW(wchar_t *Str)
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


void WINAPI UnquoteW(string &strStr)
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


void UnquoteExternalW(string &strStr)
{
  wchar_t *lpwszStr = strStr.GetBuffer ();

  if (*lpwszStr == L'\"' && lpwszStr[wcslen(lpwszStr)-1] == L'\"')
  {
    lpwszStr[wcslen(lpwszStr)-1]=0;
    wmemmove(lpwszStr,lpwszStr+1,wcslen(lpwszStr));
  }

  strStr.ReleaseBuffer ();
}


/* FileSizeToStr()
   �������������� ������� ����� � ������������� ���.
*/
#define MAX_KMGTBSTR_SIZE 16
static char KMGTbStr[5][2][MAX_KMGTBSTR_SIZE]={0};
static wchar_t KMGTbStrW[5][2][MAX_KMGTBSTR_SIZE]={0};

void __PrepareKMGTbStr(void)
{
  for(int I=0; I < 5; ++I)
  {
    xstrncpy(KMGTbStr[I][0],MSG(MListBytes+I),MAX_KMGTBSTR_SIZE-1);
    strcpy(KMGTbStr[I][1],KMGTbStr[I][0]);
    LocalStrlwr(KMGTbStr[I][0]);
    LocalStrupr(KMGTbStr[I][1]);
  }
}

void __PrepareKMGTbStrW(void)
{
  for(int I=0; I < 5; ++I)
  {
    xwcsncpy(KMGTbStrW[I][0],UMSG(MListBytes+I),MAX_KMGTBSTR_SIZE-1);
    wcscpy(KMGTbStrW[I][1],KMGTbStrW[I][0]);
    CharLowerW (KMGTbStrW[I][0]);
    CharUpperW (KMGTbStrW[I][1]);
  }
}


string & WINAPI FileSizeToStrW(string &strDestStr, unsigned __int64 Size, int Width, int ViewFlags)
{
  string strStr;
  unsigned __int64 Divider;
  int IndexDiv, IndexB;

  // ���������������� �����������
  if(KMGTbStrW[0][0][0] == 0)
  {
    __PrepareKMGTbStrW();
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
    //������������ ��� �� 1000 �� ���� ������� ���������� �� Divider
    //�������� 999 bytes ��������� ��� 999 � ��� 1000 bytes ��� ��������� ��� 0.97 K
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
      DWORD Decimal = (DWORD)((double)(DWORD)OldSize/(double)Divider*100.0);
      strStr.Format (L"%d.%02d", (DWORD)Sz,Decimal);
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
    InsertCommasW(Sz,strStr);
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
        InsertCommasW(Sz,strStr);
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



// �������� � ������� Pos � Str ������ InsStr (�������� InsSize ����)
// ���� InsSize = 0, ��... ��������� ��� ������ InsStr
// ���������� ��������� �� Str

wchar_t *InsertStringW(wchar_t *Str,int Pos,const wchar_t *InsStr,int InsSize)
{
  int InsLen=wcslen(InsStr);
  if(InsSize && InsSize < InsLen)
    InsLen=InsSize;
  memmove(Str+Pos+InsLen, Str+Pos, (wcslen(Str+Pos)+1)*sizeof (wchar_t));
  memcpy(Str+Pos, InsStr, InsLen*sizeof (wchar_t));
  return Str;
}


// �������� � ������ Str Count ��������� ��������� FindStr �� ��������� ReplStr
// ���� Count < 0 - �������� "�� ������ ������"
// Return - ���������� �����
int ReplaceStrings(char *Str,const char *FindStr,const char *ReplStr,int Count,BOOL IgnoreCase)
{
  int I=0, J=0, Res;
  int LenReplStr=strlen(ReplStr);
  int LenFindStr=strlen(FindStr);
  int L=strlen(Str);

  while(I <= L-LenFindStr)
  {
    Res=IgnoreCase?memicmp(Str+I, FindStr, LenFindStr):memcmp(Str+I, FindStr, LenFindStr);
    if(Res == 0)
    {
      if(LenReplStr > LenFindStr)
        memmove(Str+I+(LenReplStr-LenFindStr),Str+I,strlen(Str+I)+1); // >>
      else if(LenReplStr < LenFindStr)
        memmove(Str+I,Str+I+(LenFindStr-LenReplStr),strlen(Str+I+(LenFindStr-LenReplStr))+1); //??
      memcpy(Str+I,ReplStr,LenReplStr);
      I += LenReplStr;
      if(Count > 0 && ++J == Count)
        break;
    }
    else
      I++;
    L=strlen(Str);
  }
  return J;
}

int ReplaceStringsW(string &strStr,const wchar_t *FindStr,const wchar_t *ReplStr,int Count,BOOL IgnoreCase)
{
  int I=0, J=0, Res;
  int LenReplStr=wcslen(ReplStr);
  int LenFindStr=wcslen(FindStr);
  int L=strStr.GetLength ();

  wchar_t *Str = strStr.GetBuffer (1024); //BUGBUG!!!

  while(I <= L-LenFindStr)
  {
    Res=IgnoreCase?LocalStrnicmpW(Str+I, FindStr, LenFindStr):wcsncmp(Str+I, FindStr, LenFindStr);
    if(Res == 0)
    {
      if(LenReplStr > LenFindStr)
        memmove(Str+I+(LenReplStr-LenFindStr),Str+I,(wcslen(Str+I)+1)*sizeof (wchar_t)); // >>
      else if(LenReplStr < LenFindStr)
        memmove(Str+I,Str+I+(LenFindStr-LenReplStr),(wcslen(Str+I+(LenFindStr-LenReplStr))+1)*sizeof (wchar_t)); //??
      memcpy(Str+I,ReplStr,LenReplStr*sizeof (wchar_t));
      I += LenReplStr;
      if(Count > 0 && ++J == Count)
        break;
    }
    else
      I++;
    L=wcslen(Str);
  }

  strStr.ReleaseBuffer ();

  return J;
}

/*
From PHP 4.x.x
����������� �������� ����� �� �������� ������, ���������
�������������� ������. ���������� ������ SrcText ��������
� �������, �������� ���������� Width. ������ ������� ���
������ ������ Break.

��������� �� ������ � �������������� �����.

���� �������� Flahs & FFTM_BREAKLONGWORD, �� ������ ������
������������� �� �������� ������. ��� ���� � ��� ���� �����,
������� ������ �������� ������, �� ��� ����� ��������� �� �����.

Example 1.
FarFormatText("������ ������, ������� ����� ������� �� ��������� ����� �� ������ � 20 ��������.", 20 ,Dest, "\n", 0);
���� ������ ������:
---
������ ������,
������� �����
������� ��
��������� ����� ��
������ � 20
��������.
---

Example 2.
FarFormatText( "��� ������ �������� ��������������������������� ������ �����", 9, Dest, NULL, FFTM_BREAKLONGWORD);
���� ������ ������:

---
���
������
��������
���������
���������
���������
������
�����
---

*/
char *WINAPI FarFormatText(const char *SrcText,     // ��������
                           int Width,               // �������� ������
                           char *DestText,          // ��������
                           int MaxLen,              // ������������ ������� ���������
                           const char* Break,       // ����, ���� = NULL, �� ����������� '\n'
                           DWORD Flags)             // ���� �� FFTM_*
{
  const char *breakchar;
  breakchar = Break?Break:"\n";

  if(!SrcText || !*SrcText)
    return NULL;

  if(!strpbrk(SrcText,breakchar) && strlen(SrcText) <= static_cast<size_t>(Width))
  {
    if(MaxLen > 0 && DestText)
      xstrncpy(DestText,SrcText,MaxLen-1);
    return DestText;
  }

  long i=0, l=0, pgr=0, last=0;
  char *newtext;

  const char *text= SrcText;
  long linelength = Width;
  int breakcharlen = strlen(breakchar);
  int docut = Flags&FFTM_BREAKLONGWORD?1:0;

  /* Special case for a single-character break as it needs no
     additional storage space */

  if (breakcharlen == 1 && docut == 0)
  {
    newtext = xf_strdup (text);
    if(!newtext)
      return NULL;

    while (newtext[i] != '\0')
    {
      /* prescan line to see if it is greater than linelength */
      l = 0;
      while (newtext[i+l] != breakchar[0])
      {
        if (newtext[i+l] == '\0')
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
          if (newtext[i+l] == ' ')
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
            if(newtext[i+l] == ' ')
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
    newtext = (char*)xf_malloc(strlen(SrcText) * (breakcharlen+1)+1);
    if(!newtext)
      return NULL;

    newtext[0] = '\0';

    i = 0;
    while (text[i] != '\0')
    {
      /* prescan line to see if it is greater than linelength */
      l = 0;
      while (text[i+l] != '\0')
      {
        if (text[i+l] == breakchar[0])
        {
          if (breakcharlen == 1 || strncmp(text+i+l, breakchar, breakcharlen)==0)
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
          if (text[i+l] == ' ')
          {
            strncat(newtext, text+last, i+l-last);
            strcat(newtext, breakchar);
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
              if (text[i+l] == ' ')
              {
                strncat(newtext, text+last, i+l-last);
                strcat(newtext, breakchar);
                last = i + l + 1;
                break;
              }
            }
            if (docut == 1)
            {
              if (text[i+l] == ' ' || l > i-last)
              {
                strncat(newtext, text+last, i+l-last+1);
                strcat(newtext, breakchar);
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
      strcat(newtext, text+last);
    }
  }

  if(DestText && MaxLen > 0)
    xstrncpy(DestText,newtext,MaxLen-1);
  xf_free(newtext);
  return DestText;
}


string& WINAPI FarFormatTextW(const wchar_t *SrcText,     // ��������
                           int Width,               // �������� ������
                           string &strDestText,          // ��������
                           const wchar_t* Break,       // ����, ���� = NULL, �� ����������� '\n'
                           DWORD Flags)             // ���� �� FFTM_*
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
  int breakcharlen = wcslen(breakchar);
  int docut = Flags&FFTM_BREAKLONGWORD?1:0;

  /* Special case for a single-character break as it needs no
     additional storage space */

  if (breakcharlen == 1 && docut == 0)
  {
    newtext = _wcsdup (text);
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
          if (breakcharlen == 1 || wcsncmp(text+i+l, breakchar, breakcharlen)==0)
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

/* $ 12.01.2004 IS
   + ������� ��� ������ ������� � ������������� ����� � ������ �������
     ���������
*/
// ��������� - �������� �� ������ ������������ ����� (������ TRUE, ���� ��)
// ���������:
//   TableSet - ��������� �� ������� ������������� (���� �����������,
//              �� ��������� - OEM)
//   WordDiv  - ����� ������������ ����� � ��������� OEM
//   Chr      - ����������� ������
BOOL IsWordDiv(const struct CharTableSet *TableSet, const char *WordDiv, unsigned char Chr)
{
  return NULL!=strchr(WordDiv, TableSet?TableSet->DecodeTable[Chr]:Chr);
}

BOOL IsWordDivW(const struct CharTableSet *TableSet, const wchar_t *WordDiv, wchar_t Chr)
{
    return FALSE; //BUGBUG
//  return NULL!=strchr(WordDiv, TableSet?TableSet->DecodeTable[Chr]:Chr);
}

/* IS $ */

#if defined(MOUSEKEY)
/*
  Ptr=CalcWordFromString(Str,I,&Start,&End);
  xstrncpy(Dest,Ptr,End-Start+1);
  Dest[End-Start+1]=0;

// ���������:
//   TableSet - ��������� �� ������� ������������� (���� �����������,
//              �� ��������� - OEM)
//   WordDiv  - ����� ������������ ����� � ��������� OEM
  ���������� ��������� �� ������ �����
*/
const char * const CalcWordFromString(const char *Str,int CurPos,int *Start,int *End,const struct CharTableSet *TableSet, const char *WordDiv0)
{
  int I, J, StartWPos, EndWPos;
  DWORD DistLeft, DistRight;
  int StrSize=strlen(Str);
  char WordDiv[512];
  xstrncpy(WordDiv,WordDiv0,sizeof(WordDiv)-5);
  strcat(WordDiv," \t\n\r");

  if(IsWordDiv(TableSet,WordDiv,Str[CurPos]))
  {
    // ��������� ��������� - ���� ������, ��� ����� ����� - ����� ��� ������
    I=J=CurPos;

    // ������ �����
    DistLeft=-1;
    while(I >= 0 && IsWordDiv(TableSet,WordDiv,Str[I]))
    {
      DistLeft++;
      I--;
    }
    if(I < 0)
      DistLeft=-1;

    // ������ ������
    DistRight=-1;
    while(J < StrSize && IsWordDiv(TableSet,WordDiv,Str[J]))
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
  else // ����� ��� ���, �.�. ����� �� �������
    EndWPos=StartWPos=CurPos;

  while(StartWPos >= 0)
    if(IsWordDiv(TableSet,WordDiv,Str[StartWPos]))
    {
      StartWPos++;
      break;
    }
    else
      StartWPos--;
  while(EndWPos < StrSize)
    if(IsWordDiv(TableSet,WordDiv,Str[EndWPos]))
    {
      EndWPos--;
      break;
    }
    else
      EndWPos++;

  if(StartWPos < 0)
    StartWPos=0;
  if(EndWPos >= StrSize)
    EndWPos=StrSize;

  *Start=StartWPos;
  *End=EndWPos;

  return Str+StartWPos;
}
#endif

BOOL TestParentFolderNameW(const wchar_t *Name)
{
  return Name[0] == L'.' && Name[1] == L'.' && !Name[2];
}


int __digit_cnt_0(const char* s, const char** beg)
{
  int n = 0;
  while(*s == '0') s++;
  *beg = s;
  while(isdigit(*s)) { s++; n++; }
  return n;
}

int __cdecl NumStrcmp(const char *s1, const char *s2)
{
  const char *ts1 = s1, *ts2 = s2;
  while(*s1 && *s2)
  {
    if(isdigit(*s1) && isdigit(*s2))
    {
       // ����� ����� ����� ��� ������� �����
       int dig_len1 = __digit_cnt_0(s1, &s1);
       int dig_len2 = __digit_cnt_0(s2, &s2);
       // ���� ���� ������� �������, ������ ��� � ������! :)
       if(dig_len1 != dig_len2)
         return dig_len1 - dig_len2;
       // ����� ���������, ������������...
       while(isdigit(*s1) && isdigit(*s2))
       {
          if(*s1 != *s2)
            return *s1 - *s2;
          s1++; s2++;
       }
       if(*s1 == 0)
         break;
    }
    if(*s1 != *s2)
      return *s1 - *s2; // ����� ������ ������
    s1++; s2++;
  }
  if(*s1 == *s2)
    return strlen(ts2)-strlen(ts1);
  return *s1 - *s2;
}

char *UnicodeToAnsi (const wchar_t *lpwszUnicodeString, int nMaxLength)
{
  int nLength = wcslen (lpwszUnicodeString)+1;

  if ( (nMaxLength > 0) && (nMaxLength < nLength) )
    nLength = nMaxLength;

  char *lpResult = (char*)malloc (nLength);

  memset (lpResult, 0, nLength);

  WideCharToMultiByte (
          CP_OEMCP,
          0,
          lpwszUnicodeString,
          -1,
          lpResult,
          nLength,
          NULL,
          NULL
          );

  return lpResult;
}

void UnicodeToAnsi (
        const wchar_t *lpwszUnicodeString,
        char *lpDest,
        int nMaxLength
        ) //BUGBUG
{
  int nLength = wcslen (lpwszUnicodeString)+1;

  if ( (nMaxLength > 0) && (nMaxLength < nLength) )
    nLength = nMaxLength;

  memset (lpDest, 0, nLength);

  WideCharToMultiByte (CP_OEMCP, 0, lpwszUnicodeString, -1, lpDest, nLength, NULL, NULL); //RAVE!!!
}

string& CutToSlashW (string &strStr, bool bInclude)
{
    wchar_t *lpwszStr = strStr.GetBuffer ();

    lpwszStr = wcsrchr (lpwszStr, '\\');

    if ( lpwszStr )
    {
        if ( bInclude )
            *lpwszStr = 0;
        else
            *(lpwszStr+1) = 0;
    }


    strStr.ReleaseBuffer ();

    return strStr;
}

string& CutToNameUNCW (string &strPath)
{
  wchar_t *lpwszPath = strPath.GetBuffer ();

  if ((lpwszPath[0]==L'/' || lpwszPath[0]==L'\\') && (lpwszPath[1]==L'/' || lpwszPath[1]==L'\\'))
  {
    lpwszPath+=2;
    for (int i=0; i<2; i++)
    {
      while (*lpwszPath && *lpwszPath!=L'/' && *lpwszPath!=L'\\')
        lpwszPath++;
      if (*lpwszPath)
        lpwszPath++;
    }
  }

  wchar_t *lpwszNamePtr = lpwszPath;

  while ( *lpwszPath )
  {
    if ( *lpwszPath==L'\\' || *lpwszPath==L'/' || *lpwszPath==L':' && lpwszPath == lpwszNamePtr+1 )
      lpwszNamePtr = lpwszPath+1;

    lpwszPath++;
  }

  *lpwszNamePtr = 0;

  strPath.ReleaseBuffer ();

  return strPath;

}

const wchar_t* PointToNameUNCW (const wchar_t *lpwszPath)
{
  if ( !lpwszPath )
    return NULL;

  if ((lpwszPath[0]==L'/' || lpwszPath[0]==L'\\') && (lpwszPath[1]==L'/' || lpwszPath[1]==L'\\'))
  {
    lpwszPath+=2;
    for (int i=0; i<2; i++)
    {
      while (*lpwszPath && *lpwszPath!=L'/' && *lpwszPath!=L'\\')
        lpwszPath++;
      if (*lpwszPath)
        lpwszPath++;
    }
  }

  const wchar_t *lpwszNamePtr = lpwszPath;

  while ( *lpwszPath )
  {
    if ( *lpwszPath==L'\\' || *lpwszPath==L'/' || *lpwszPath==L':' && lpwszPath == lpwszNamePtr+1 )
      lpwszNamePtr = lpwszPath+1;

    lpwszPath++;
  }
  return lpwszNamePtr;
}
