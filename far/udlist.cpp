/*
udlist.cpp

—писок чего-либо, перечисленного через символ-разделитель. ≈сли нужно, чтобы
элемент списка содержал разделитель, то этот элемент следует заключить в
кавычки. ≈сли кроме разделител€ ничего больше в строке нет, то считаетс€, что
это не разделитель, а простой символ.

*/

/* Revision: 1.02 12.06.2001 $ */

/*
Modify:
  12.06.2001 IS
    + ≈сли кроме разделител€ ничего больше в строке нет,
      то считаетс€, что это не разделитель, а простой символ
  09.06.2001 IS
    + ѕереписано с учетом второго разделител€. “еперь разделителей два. ѕо
      умолчанию они равны ';' и ','
  02.06.2001 IS
    + ¬первые в эфире
*/

#include "headers.hpp"
#pragma hdrstop

#include "udlist.hpp"


UserDefinedList::UserDefinedList()
{
  DataCurrent=Data=DataEnd=NULL;
  SetDefaultSeparators();
}

UserDefinedList::UserDefinedList(BYTE separator1, BYTE separator2)
{
  DataCurrent=Data=DataEnd=NULL;
  Separator1=separator1;
  Separator2=separator2;
  if(!Separator2 && !Separator2) SetDefaultSeparators();
}

void UserDefinedList::SetDefaultSeparators()
{
  Separator1=';';
  Separator2=',';
}

void UserDefinedList::SetSeparator(BYTE separator1, BYTE separator2)
{
  Free();
  Separator1=separator1;
  Separator2=separator2;
  if(!Separator2 && !Separator2) SetDefaultSeparators();
}

void UserDefinedList::Free()
{
  if(Data)
     free (Data);
  DataCurrent=Data=DataEnd=NULL;
}

BOOL UserDefinedList::Set(const char *List)
{
  Free();
  BOOL rc=FALSE;

  if(List && *List)
  {
    if(*List!=Separator1 && *List!=Separator2)
      {
        int Length=strlen(List), RealLength;
        {
          Data=(char *)malloc(4+Length);
          DataEnd=Data;
          if(Data)
          {
            BOOL Error=FALSE;
            const char *CurList=List;
            while(NULL!=(CurList=Skip(CurList, Length, RealLength, Error)))
            {
              if(Length)
              {
                strncpy(DataEnd, CurList, Length);
                CurList+=RealLength;
                DataEnd[Length]=0;
                DataEnd+=Length+1;
              }
              else
              {
                Error=TRUE;
                break;
              }
            }

            rc=!Error;
          }
        }
      }
    else
      {
         const char *End=List+1;
         while(isspace(*End)) ++End; // пропустим мусор
         if(!*End) // ≈сли кроме разделител€ ничего больше в строке нет,
                   // то считаетс€, что это не разделитель, а простой символ
         {
           Data=(char *)malloc(4);
           if(Data)
           {
             DataEnd=Data+2;
             *Data=*List;
             Data[1]=0;
             rc=TRUE;
           }
         }
      }
  }

  if(rc)
      Start();
  else
      Free();

  return rc;
}

const char *UserDefinedList::Skip(const char *Str, int &Length, int &RealLength, BOOL &Error)
{
   Length=RealLength=0;
   Error=FALSE;

   while(isspace(*Str)) ++Str;
   if(*Str==Separator1 || *Str==Separator2) ++Str;
   if(!*Str) return NULL;

   const char *cur=Str;
   // важно! проверка *cur!=0 должна сто€ть первой
   while(*cur && *cur!=Separator1 && *cur!=Separator2 && *cur!='\"') ++cur;
   if(*cur!='\"' || !*cur)
    {
      RealLength=Length=cur-Str;
      return Str;
    }

   ++cur;
   const char *QuoteEnd=strchr(cur, '\"');
   if(QuoteEnd==NULL)
    {
      Error=TRUE;
      return NULL;
    }

   const char *End=QuoteEnd+1;
   while(isspace(*End)) ++End;
   if(!*End || *End==Separator1 || *End==Separator2)
   {
     Length=QuoteEnd-cur;
     RealLength=End-cur;
     return cur;
   }

   Error=TRUE;
   return NULL;
}

void UserDefinedList::Start(void)
{
  DataCurrent=Data;
}

BOOL UserDefinedList::IsEmpty()
{
  return DataCurrent==NULL;
}

const char *UserDefinedList::GetNext()
{
  const char *ret=DataCurrent;
  if(DataCurrent)
  {
    DataCurrent+=strlen(DataCurrent)+1;
    if(DataCurrent>=DataEnd)
       DataCurrent=NULL;
  }

  return ret;
}
