/*
strmix.cpp

Куча разных вспомогательных функций по работе со строками

*/

/* Revision: 1.15 25.06.2001 $ */

/*
Modify:
  25.06.2001 IS
    ! Внедрение const
  06.06.2001 VVM
    ! чуть-чуть переделаем Unquote()
  05.06.2001 VVM
    + переделка Unquote()
  07.05.2001 DJ
    * оптимизированная ветка CmpName() работает и для имен с несколькими
      точками
    + CopyMaskStr()
  06.05.2001 DJ
    ! перетрях #include
  22.04.2001 SVS
    + Opt.QuotedSymbols - разделители для QuoteSpace()
    ! немного оптимизации в Unquote
  13.04.2001 SVS
    ! переделаны QuoteSpaceOnly & QuoteSpace - общий код вынесен во
      вспомогательную функцию InsertQuote и добавлена проверка на присутствие
      кавычек в начала и в конце строки.
  08.04.2001 SVS
    ! GetCommaWord() - дополнительный параметр - разделитель, по умолчанию = ','
  20.03.2001 SVS
    + FileSizeToStr - функция преобразования размера файла в... удобочитаемый
      вид - выдрана из FileList::ShowList()
  13.03.2001 SVS
    ! GetPathRoot переехала в flink.cpp :-)
  12.03.2001 SVS
    ! Коррекция в связи с изменениями в классе int64
  06.03.2001 SVS
    ! Немного оптимизации в TruncStr() - избавляемся от лишнего вызова new[]
    ! Немного оптимизации в InsertCommas() - избавляемся от лишнего sprintf()
  05.03.2001 SVS
    ! Немного оптимизации в фунциях QuoteSpace и QuoteSpaceOnly -
      убрал лишний malloc()
  28.02.2001 SVS
    ! CenterStr возвращает указатель на Dest
  22.02.2001 IS
    + RemoveChar - удаляет символ из строки
    ! RemoveHighlights(Str) как макрос (в fn.hpp) - вызывает RemoveChar(Str,'&')
  02.02.2001 IS
    + Функция RemoveUnprintableCharacters - заменяет пробелами непечатные
      символы в строке. В настоящий момент обрабатываются только cr и lf.
  05.01.2001 SVS
    ! Выделение в качестве самостоятельного модуля
    + Функции InsertCommas, PointToName, GetPathRoot, CmpName, ConvertWildcards,
      QuoteSpace, QuoteSpaceOnly, TruncStr, TruncPathStr, Remove???Spaces,
      HiStrlen, AddEndSlash, NullToEmpty, CenterStr, GetCommaWord,
      RemoveHighlights, IsCaseMixed, IsCaseLower, Unquote,
      переехали из mix.cpp
*/

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"
#include "global.hpp"
#include "lang.hpp"

char *InsertCommas(unsigned long Number,char *Dest)
{
  ultoa(Number,Dest,10);
  for (int I=strlen(Dest)-4;I>=0;I-=3)
    if (Dest[I])
    {
      memmove(Dest+I+2,Dest+I+1,strlen(Dest+I));
      Dest[I+1]=',';
    }
  return Dest;
}


char* InsertCommas(int64 li,char *Dest)
{
  if (li<1000000000)
    InsertCommas(li.PLow(),Dest);
  else
  {
    li.itoa(Dest);
    for (int I=strlen(Dest)-4;I>=0;I-=3)
      if (Dest[I])
      {
        memmove(Dest+I+2,Dest+I+1,strlen(Dest+I));
        Dest[I+1]=',';
      }
  }
  return Dest;
}

char* WINAPI PointToName(char *Path)
{
  char *NamePtr=Path;
  while (*Path)
  {
    if (*Path=='\\' || *Path=='/' || *Path==':' && Path==NamePtr+1)
      NamePtr=Path+1;
    Path++;
  }
  return(NamePtr);
}

int CmpName(const char *pattern,const char *string,int skippath)
{
  char stringc,patternc,rangec;
  int match;
  static int depth=0;

  if (skippath)
    string=PointToName(const_cast<char*>(string));

  for (;; ++string)
  {
    /* $ 01.05.2001 DJ
       используем инлайновые версии
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
           оптимизированная ветка работает и для имен с несколькими
           точками
        */
        if (*pattern=='.')
        {
          if (pattern[1]=='*' && pattern[2]==0 && depth==0)
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
          depth++;
          int CmpCode=CmpName(pattern,string++,FALSE);
          depth--;
          if (CmpCode)
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

/* $ 09.10.2000 IS
    Генерация нового имени по маске
    (взял из ShellCopy::ShellCopyConvertWildcards)
*/
// На основе имени файла (Src) и маски (Dest) генерируем новое имя
// SelectedFolderNameLength - длина каталога. Например, есть
// каталог dir1, а в нем файл file1. Нужно сгенерировать имя по маске для dir1.
// Параметры могут быть следующими: Src="dir1", SelectedFolderNameLength=0
// или Src="dir1\\file1", а SelectedFolderNameLength=4 (длина "dir1")
int ConvertWildcards(const char *src,char *Dest, int SelectedFolderNameLength)
{
  char WildName[2*NM], Src[NM], *CurWildPtr,*DestNamePtr,*SrcNamePtr;
  char PartBeforeName[NM],PartAfterFolderName[NM];
  DestNamePtr=PointToName(Dest);
  strcpy(WildName,DestNamePtr);
  if (strchr(WildName,'*')==NULL && strchr(WildName,'?')==NULL)
    return(FALSE);

  strncpy(Src, src, sizeof(Src));

  if (SelectedFolderNameLength!=0)
  {
    strcpy(PartAfterFolderName,Src+SelectedFolderNameLength);
    Src[SelectedFolderNameLength]=0;
  }

  SrcNamePtr=PointToName(Src);

  int BeforeNameLength=DestNamePtr==Dest ? SrcNamePtr-Src:0;
  strncpy(PartBeforeName,Src,BeforeNameLength);
  PartBeforeName[BeforeNameLength]=0;

  char *SrcNameDot=strrchr(SrcNamePtr,'.');
  CurWildPtr=WildName;
  while (*CurWildPtr)
    switch(*CurWildPtr)
    {
      case '?':
        CurWildPtr++;
        if (*SrcNamePtr)
          *(DestNamePtr++)=*(SrcNamePtr++);
        break;
      case '*':
        CurWildPtr++;
        while (*SrcNamePtr)
        {
          if (*CurWildPtr=='.' && SrcNameDot!=NULL && strchr(CurWildPtr+1,'.')==NULL)
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
        *(DestNamePtr++)='.';
        if (strpbrk(CurWildPtr,"*?")!=NULL)
          while (*SrcNamePtr)
            if (*(SrcNamePtr++)=='.')
              break;
        break;
      default:
        *(DestNamePtr++)=*(CurWildPtr++);
        if (*SrcNamePtr && *SrcNamePtr!='.')
          SrcNamePtr++;
        break;
    }

  *DestNamePtr=0;
  if (DestNamePtr!=Dest && *(DestNamePtr-1)=='.')
    *(DestNamePtr-1)=0;
  if (*PartBeforeName)
  {
    strcat(PartBeforeName,Dest);
    strcpy(Dest,PartBeforeName);
  }
  if (SelectedFolderNameLength!=0)
    strcat(Src,PartAfterFolderName);
  return(TRUE);
}
/* IS $ */

static char *InsertQuote(char *Str)
{
  unsigned l = strlen(Str);
  if(*Str != '"')
  {
    memmove(Str+1,Str,++l);
    *Str='"';
  }
  if(Str[l-1] != '"')
  {
    Str[l++] = '\"';
    Str[l] = 0;
  }
  return Str;
}

char* QuoteSpace(char *Str)
{
  if (*Str=='-' || *Str=='^' || strpbrk(Str,Opt.QuotedSymbols)!=NULL)
    InsertQuote(Str);
  return(Str);
}

char* WINAPI QuoteSpaceOnly(char *Str)
{
  if (Str && strchr(Str,' ')!=NULL)
    InsertQuote(Str);
  return(Str);
}


char* WINAPI TruncStr(char *Str,int MaxLength)
{
  if(Str)
  {
    int Length;
    if (MaxLength<0)
      MaxLength=0;
    if ((Length=strlen(Str))>MaxLength)
#if 0
      if (MaxLength>3)
      {
        char *TmpStr=new char[MaxLength+5];
        sprintf(TmpStr,"...%s",Str+Length-MaxLength+3);
        strcpy(Str,TmpStr);
        /* $ 13.07.2000 SVS
           ну а здесь раз уж вызвали new[], то в придачу и delete[] надо... */
        delete[] TmpStr;
        /* SVS $ */
      }
      else
        Str[MaxLength]=0;
#else
    {
      if (MaxLength>3)
      {
        memmove(Str+3,Str+Length-MaxLength+3,MaxLength);
        memcpy(Str,"...",3);
      }
      Str[MaxLength]=0;
    }
#endif
  }
  return(Str);
}


char* WINAPI TruncPathStr(char *Str,int MaxLength)
{
  if(Str)
  {
    char *Root=NULL;
    if (Str[0]!=0 && Str[1]==':' && Str[2]=='\\')
      Root=Str+3;
    else
      if (Str[0]=='\\' && Str[1]=='\\' && (Root=strchr(Str+2,'\\'))!=NULL &&
          (Root=strchr(Root+1,'\\'))!=NULL)
        Root++;
    if (Root==NULL || Root-Str+5>MaxLength)
      return(TruncStr(Str,MaxLength));
    int Length=strlen(Str);
    if (Length>MaxLength)
    {
      char *MovePos=Root+Length-MaxLength+3;
      memmove(Root+3,MovePos,strlen(MovePos)+1);
      memcpy(Root,"...",3);
    }
  }
  return(Str);
}

/* $ 07.07.2000 SVS
    + Дополнительная функция обработки строк: RemoveExternalSpaces
    ! Функции Remove*Spaces возвращают char*
*/
// удалить ведущие пробелы
char* WINAPI RemoveLeadingSpaces(char *Str)
{
  char *ChPtr;
  if(Str)
  {
    for (ChPtr=Str;isspace(*ChPtr);ChPtr++)
           ;
    if (ChPtr!=Str)
      memmove(Str,ChPtr,strlen(ChPtr)+1);
  }
  return Str;
}


// удалить конечные пробелы
char* WINAPI RemoveTrailingSpaces(char *Str)
{
  if(Str)
  {
    for (int I=strlen((char *)Str)-1;I>=0;I--)
      if (isspace(Str[I]) || iseol(Str[I]))
        Str[I]=0;
      else
        break;
  }
  return Str;
}

// удалить пробелы снаружи
char* WINAPI RemoveExternalSpaces(char *Str)
{
  return RemoveTrailingSpaces(RemoveLeadingSpaces(Str));
}
/* SVS $ */

/* $ 02.02.2001 IS
   Заменяет пробелами непечатные символы в строке. В настоящий момент
   обрабатываются только cr и lf.
*/
char* WINAPI RemoveUnprintableCharacters(char *Str)
{
 char *p=Str;
 while(*p)
 {
   if('\n'==*p || '\r'==*p) *p=' ';
   p++;
 }
 return RemoveExternalSpaces(Str);
}
/* IS $ */

// Удалить символ Target из строки Str (везде!)
char *RemoveChar(char *Str,char Target)
{
  char *Ptr = Str, *StrBegin = Str, Chr;
  while((Chr=*Str++) != 0)
  {
    if(Chr == Target)
      continue;
    *Ptr++ = Chr;
  }
  *Ptr = '\0';
  return StrBegin;
}

int HiStrlen(const char *Str)
{
  int Length=0;
  while (*Str)
  {
    if (*Str!='&')
      Length++;
    Str++;
  }
  return(Length);
}


int WINAPI AddEndSlash(char *Path)
{
  int Result=0;
  if(Path)
  {
    /* $ 06.12.2000 IS
      ! Теперь функция работает с обоими видами слешей, также происходит
        изменение уже существующего конечного слеша на такой, который
        встречается чаще.
    */
    char *end=Path;
    int Slash=0, BackSlash=0;
    while(*end)
    {
     Slash+=(*end=='\\');
     BackSlash+=(*end=='/');
     end++;
    }
    int Length=end-Path;
    char c=(Slash<BackSlash)?'/':'\\';
    Result = 1;
    if (Length==0)
    {
       *end=c;
       end[1]=0;
    }
    else
    {
     end--;
     if (*end!='\\' && *end!='/')
     {
       end[1]=c;
       end[2]=0;
     }
     else *end=c;
    }
    /* IS $ */
  }
  return Result;
}


char *NullToEmpty(char *Str)
{
  return (Str==NULL) ? "":Str;
}

const char *NullToEmpty(const char *Str)
{
  return (Str==NULL) ? "":Str;
}


char* CenterStr(char *Src,char *Dest,int Length)
{
  char TempSrc[512];
  int SrcLength=strlen(Src);
  strcpy(TempSrc,Src);
  if (SrcLength>=Length)
    strcpy(Dest,TempSrc);
  else
  {
    int Space=(Length-SrcLength)/2;
    sprintf(Dest,"%*s%s%*s",Space,"",TempSrc,Length-Space-SrcLength,"");
  }
  return Dest;
}


/* $ 08.04.2001 SVS
  + дополнительный параметр - разделитель, по умолчанию = ','
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
      while (isspace(*Src))
        Src++;
      return(Src);
    }
    else
      Word[WordPos]=*Src;
  }
  Word[WordPos]=0;
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


/* $ 08.06.2001 VVM
  + Винда убирает _все_ кавычки. А чем мы хуже? */
/* $ 05.06.2001 VVM
  Теперь мы будем жить по новому :)
  И удалять весь квотинг и внутри строки */
/* $ 28.06.2000 IS
  Теперь функция Unquote убирает ВСЕ начальные и заключительные кавычки
*/
/* $ 25.07.2000 SVS
   Вызов WINAPI
*/
void WINAPI Unquote(char *Str)
{
  if (!Str)
    return;
  while (*Str)
  {
    if (*Str=='\"')
      strcpy(Str, Str+1);
    else
      Str++;
  }
}
/* IS $ */
/* VVM $ */
/* VVM $ */

/* FileSizeToStr()
   Форматирование размера файла в удобочитаемый вид.
*/
char* WINAPI FileSizeToStr(char *DestStr,DWORD SizeHigh, DWORD Size, int Width, int ViewFlags)
{
  char OutStr[64];
  char Str[100];

  int Commas=(ViewFlags & COLUMN_COMMAS);

  if (SizeHigh)
  {
    DWORD SizeMb=SizeHigh*4096+Size/(1024*1024);
    if (Commas)
      InsertCommas(SizeMb,Str);
    else
      sprintf(Str,"%u",SizeMb);
    sprintf(OutStr,"%s %1.1s",Str,MSG(MListMb));
    sprintf(DestStr,"%*.*s",Width,Width,OutStr);
    return DestStr;
  }
  if (Commas)
    InsertCommas(Size,Str);
  else
    sprintf(Str,"%u",Size);
  int SizeLength=strlen(Str);
  if (SizeLength<=Width || Width<5)
    sprintf(DestStr,"%*.*s",Width,Width,Str);
  else
  {
    char KbStr[100],MbStr[100];
    int Divider;
    Width-=2;
    strcpy(KbStr,MSG(MListKb));
    strcpy(MbStr,MSG(MListMb));
    if (ViewFlags & COLUMN_THOUSAND)
    {
      Divider=1000;
      LocalStrlwr(KbStr);
      LocalStrlwr(MbStr);
    }
    else
    {
      Divider=1024;
      LocalStrupr(KbStr);
      LocalStrupr(MbStr);
    }
    Size/=Divider;
    if (Commas)
      InsertCommas(Size,Str);
    else
      sprintf(Str,"%u",Size);
    if (strlen(Str)<=Width)
      sprintf(DestStr,"%*.*s %1.1s",Width,Width,Str,KbStr);
    else
    {
      Size/=Divider;
      if (Commas)
        InsertCommas(Size,Str);
      else
        sprintf(Str,"%u",Size);
      sprintf(DestStr,"%*.*s %1.1s",Width,Width,Str,MbStr);
    }
  }
  return DestStr;
}

/* $ 01.05.2001 DJ
   копирование списка масок, разделенного запятыми, в список, разделенный
   NULL (для sort groups и file highlighting)
*/

void CopyMaskStr (char *PDest, const char *PSrc)
{
  BOOL SkipBrackets=FALSE;

  while (*PSrc)
  {
    if (*PSrc == '[' && strchr (PSrc+1, ']') != NULL)
      SkipBrackets = TRUE;
    if (*PSrc == ']')
      SkipBrackets = FALSE;
    if (*PSrc == ',' && !SkipBrackets)
    {
      *PDest++ = '\0';
      PSrc++;
      while (isspace (*PSrc))
        PSrc++;
    }
    else
      *PDest++ = *PSrc++;
  }
  *PDest = '\0';
  *(PDest+1) = '\0';
}
