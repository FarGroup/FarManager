/*
udlist.cpp

Список чего-либо, перечисленного через символ-разделитель. Если нужно, чтобы
элемент списка содержал разделитель, то этот элемент следует заключить в
кавычки.

*/

/* Revision: 1.00 02.06.2001 $ */

/*
Modify:
  02.06.2001 IS
    + Впервые в эфире
*/

#include "headers.hpp"
#pragma hdrstop

#include "udlist.hpp"


UserDefinedList::UserDefinedList()
{
  DataCurrent=Data=DataEnd=NULL;
  Separator=';';
}

UserDefinedList::UserDefinedList(BYTE separator)
{
  DataCurrent=Data=DataEnd=NULL;
  Separator=separator;
}

void UserDefinedList::SetSeparator(BYTE Separator)
{
  Free();
  UserDefinedList::Separator=Separator;
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

  if(List && *List && *List!=Separator)
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

   while(isspace(*Str)) Str++;
   if(*Str==Separator) Str++;
   if(!*Str) return NULL;

   const char *cur=Str;
   while(*cur && *cur!=Separator && *cur!='\"') cur++;
   if(*cur!='\"' || !*cur)
    {
      RealLength=Length=cur-Str;
      return Str;
    }

   cur++;
   const char *QuoteEnd=strchr(cur, '\"');
   if(QuoteEnd==NULL)
    {
      Error=TRUE;
      return NULL;
    }

   const char *End=QuoteEnd+1;
   while(isspace(*End)) End++;
   if(*End==Separator || !*End)
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
