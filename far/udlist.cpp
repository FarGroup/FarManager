/*
udlist.cpp

—писок чего-либо, перечисленного через символ-разделитель. ≈сли нужно, чтобы
элемент списка содержал разделитель, то этот элемент следует заключить в
кавычки. ≈сли кроме разделител€ ничего больше в строке нет, то считаетс€, что
это не разделитель, а простой символ.

*/

/* Revision: 1.06 11.08.2001 $ */

/*
Modify:
  11.08.2001 IS
    - Ќекритическа€ опечатка
    ! SetSeparators -> SetParameters
    + ќбработка UDL_FLAGS
  01.08.2001 IS
    + ѕодсчет элементов в списке (Total)
  02.07.2001 IS
    + AddAsterisk
  12.06.2001 IS
    + ѕри добавлении элементов списка игнорируютс€ заключительные и начальные
      пробелы. ≈сли нужно учитывать пробелы, то элемент списка должен быть
      заключен в кавычки
    + ќбрабатываютс€ квадратные скобки. “еперь можно выкинуть GetCommaWord и
      заменить его использованием UserDefinedList
    + CheckSeparators()
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
  Total=0;
  DataCurrent=Data=DataEnd=NULL;
  SetParameters(0, 0, 0);
}

UserDefinedList::UserDefinedList(BYTE separator1, BYTE separator2, DWORD Flags)
{
  Total=0;
  DataCurrent=Data=DataEnd=NULL;
  SetParameters(separator1, separator2, Flags);
}

void UserDefinedList::SetDefaultSeparators()
{
  Separator1=';';
  Separator2=',';
}

BOOL UserDefinedList::CheckSeparators() const
{
  return !((Separator1=='\"' || Separator2=='\"') ||
           (ProcessBrackets &&  (Separator1=='[' || Separator2=='[' ||
            Separator1==']' || Separator2==']'))
          );
}

BOOL UserDefinedList::SetParameters(BYTE separator1, BYTE separator2, DWORD Flags)
{
  Free();
  Separator1=separator1;
  Separator2=separator2;
  ProcessBrackets=(Flags & ULF_PROCESSBRACKETS)?TRUE:FALSE;
  AddAsterisk=(Flags & ULF_ADDASTERISK)?TRUE:FALSE;
  PackAsterisks=(Flags & ULF_PACKASTERISKS)?TRUE:FALSE;

  if(!Separator1 && Separator2)
  {
     Separator1=Separator2;
     Separator2=0;
  }
  if(!Separator1 && !Separator2) SetDefaultSeparators();

  return CheckSeparators();
}

void UserDefinedList::Free()
{
  if(Data)
     free (Data);
  DataCurrent=Data=DataEnd=NULL;
  Total=0;
}

BOOL UserDefinedList::Set(const char *List)
{
  Free();
  BOOL rc=FALSE;

  if(CheckSeparators() && List && *List)
  {
    if(*List!=Separator1 && *List!=Separator2)
      {
        int Length=0, Commas=0, RealLength;
        if(AddAsterisk)
          while(List[Length])
          {
            if(','==List[Length]) ++Commas; // на столько будем увеличивать
                                            // строку дл€ страховки
            ++Length;
          }
        else
          Length=strlen(List);

        {
          Data=(char *)malloc(4+Length+Commas);
          DataEnd=Data;
          if(Data)
          {
            BOOL Error=FALSE;
            const char *CurList=List;
            while(NULL!=(CurList=Skip(CurList, Length, RealLength, Error)))
            {
              if(Length)
              {
                if(PackAsterisks && 3==Length && 0==memcmp(CurList, "*.*", 3))
                {
                   *DataEnd='*';
                   Length=1;
                }
                else
                   strncpy(DataEnd, CurList, Length);

                CurList+=RealLength;
                DataEnd[Length]=0;
                if(AddAsterisk && strpbrk(DataEnd,"?*.")==NULL)
                {
                  DataEnd[Length]='*';
                  ++Length;
                  DataEnd[Length]=0;
                }
                DataEnd+=Length+1;
                ++Total;
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
             ++Total;
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
   while(isspace(*Str)) ++Str;
   if(!*Str) return NULL;

   const char *cur=Str;
   BOOL InBrackets=FALSE, InQoutes = (*cur=='\"');


   if(!InQoutes) // если мы в кавычках, то обработка будет позже и чуть сложнее
     while(*cur) // важно! проверка *cur!=0 должна сто€ть первой
     {
        if(ProcessBrackets)
        {
           if(*cur==']')
             InBrackets=FALSE;

           if(*cur=='[' && NULL!=strchr(cur+1, ']'))
             InBrackets=TRUE;
        }

        if(!InBrackets && (*cur==Separator1 || *cur==Separator2))
          break;

        ++cur;
     }

   if(!InQoutes || !*cur)
    {
      RealLength=Length=cur-Str;
      --cur;
      while(isspace(*cur))
       {
         --Length;
         --cur;
       }
      return Str;
    }

   // мы в кавычках - захватим все отсюда и до следующих кавычек
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
