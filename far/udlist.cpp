/*
udlist.cpp

Список чего-либо, перечисленного через символ-разделитель. Если нужно, чтобы
элемент списка содержал разделитель, то этот элемент следует заключить в
кавычки. Если кроме разделителя ничего больше в строке нет, то считается, что
это не разделитель, а простой символ.
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

#include "udlist.hpp"

UserDefinedListItem::~UserDefinedListItem()
{
  if(Str)
    xf_free(Str);
}

bool UserDefinedListItem::operator==(const UserDefinedListItem &rhs) const
{
  return (Str && rhs.Str)?(!StrCmpI(Str, rhs.Str)):false;
}

int UserDefinedListItem::operator<(const UserDefinedListItem &rhs) const
{
  if(!Str)
    return 1;
  else if(!rhs.Str)
    return -1;
  else
    return StrCmpI(Str, rhs.Str)<0;
}

const UserDefinedListItem& UserDefinedListItem::operator=(const
  UserDefinedListItem &rhs)
{
  if(this!=&rhs)
  {
    if(Str)
    {
      xf_free(Str);
      Str=NULL;
    }
    if(rhs.Str)
      Str=xf_wcsdup(rhs.Str);
    index=rhs.index;
  }
  return *this;
}

const UserDefinedListItem& UserDefinedListItem::operator=(const wchar_t *rhs)
{
  if(Str!=rhs)
  {
    if(Str)
    {
      xf_free(Str);
      Str=NULL;
    }
    if(rhs)
      Str=xf_wcsdup(rhs);
  }
  return *this;
}

wchar_t *UserDefinedListItem::set(const wchar_t *Src, unsigned int size)
{
  if(Str!=Src)
  {
    if(Str)
    {
      xf_free(Str);
      Str=NULL;
    }
    Str=static_cast<wchar_t*>(xf_malloc((size+1)*sizeof(wchar_t)));
    if(Str)
    {
      wmemcpy(Str,Src,size);
      Str[size]=0;
    }
  }
  return Str;
}

UserDefinedList::UserDefinedList()
{
  Reset();
  SetParameters(0,0,0);
}

UserDefinedList::UserDefinedList(WORD separator1, WORD separator2,
                                 DWORD Flags)
{
  Reset();
  SetParameters(separator1, separator2, Flags);
}

void UserDefinedList::SetDefaultSeparators()
{
  Separator1=L';';
  Separator2=L',';
}

BOOL UserDefinedList::CheckSeparators() const
{
  return !((Separator1==L'\"' || Separator2==L'\"') ||
           (ProcessBrackets &&  (Separator1==L'[' || Separator2==L'[' ||
            Separator1==L']' || Separator2==L']'))
          );
}

BOOL UserDefinedList::SetParameters(WORD separator1, WORD separator2,
                                    DWORD Flags)
{
  Free();
  Separator1=separator1;
  Separator2=separator2;
  ProcessBrackets=(Flags & ULF_PROCESSBRACKETS)?TRUE:FALSE;
  AddAsterisk=(Flags & ULF_ADDASTERISK)?TRUE:FALSE;
  PackAsterisks=(Flags & ULF_PACKASTERISKS)?TRUE:FALSE;
  Unique=(Flags & ULF_UNIQUE)?TRUE:FALSE;
  Sort=(Flags & ULF_SORT)?TRUE:FALSE;

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
  Array.Free();
  Reset();
}

BOOL UserDefinedList::Set(const wchar_t *List, BOOL AddToList)
{
  if(AddToList)
  {
    if(List && !*List) // пусто, нечего добавлять
      return TRUE;
  }
  else
    Free();
  BOOL rc=FALSE;

  if(CheckSeparators() && List && *List)
  {
    int Length, RealLength;
    UserDefinedListItem item;
    item.index=Array.getSize();
    if(*List!=Separator1 && *List!=Separator2)
    {
      Length=StrLength(List);
      BOOL Error=FALSE;
      const wchar_t *CurList=List;
      while(!Error &&
            NULL!=(CurList=Skip(CurList, Length, RealLength, Error)))
      {
        if(Length > 0)
        {
          if(PackAsterisks && 3==Length && 0==memcmp(CurList, L"*.*", 6))
          {
            item=L"*";
            if(!item.Str || !Array.addItem(item))
              Error=TRUE;
          }
          else
          {
            item.set(CurList, Length);
            if(item.Str)
            {
              if(PackAsterisks)
              {
                int i=0, lastAsterisk=FALSE;
                while(i<Length)
                {
                  if(item.Str[i]==L'*')
                  {
                    if(!lastAsterisk)
                      lastAsterisk=TRUE;
                    else
                    {
                      wmemcpy(item.Str+i, item.Str+i+1, StrLength(item.Str+i+1)+1);
                      --i;
                    }
                  }
                  else
                    lastAsterisk=FALSE;
                  ++i;
                }
              }

              if(AddAsterisk && wcspbrk(item.Str,L"?*.")==NULL)
              {
                Length=StrLength(item.Str);
                /* $ 18.09.2002 DJ
                   выделялось на 1 байт меньше, чем надо
                */
                item.Str=static_cast<wchar_t*>(xf_realloc(item.Str, (Length+2)*sizeof(wchar_t)));
                /* DJ $ */
                if(item.Str)
                {
                  item.Str[Length]=L'*';
                  item.Str[Length+1]=0;
                }
                else
                  Error=TRUE;
              }
              if(!Error && !Array.addItem(item))
                Error=TRUE;
            }
            else
              Error=TRUE;
          }

          CurList+=RealLength;
        }
        else
          Error=TRUE;
        ++item.index;
      }

      rc=!Error;
    }
    else
    {
      const wchar_t *End=List+1;
      while(IsSpace(*End)) ++End; // пропустим мусор
      if(!*End) // Если кроме разделителя ничего больше в строке нет,
      {         // то считается, что это не разделитель, а простой символ
        item=L" ";
        if(item.Str)
        {
          *item.Str=*List;
          if(Array.addItem(item))
            rc=TRUE;
        }
      }
    }
  }

  if(rc)
  {
    if(Unique)
    {
      Array.Sort();
      Array.Pack();
    }
    if(!Sort)
      Array.Sort(reinterpret_cast<TARRAYCMPFUNC>(CmpItems));
    else if(!Unique) // чтобы не сортировать уже отсортированное
      Array.Sort();
    unsigned int i=0, maxI=Array.getSize();
    for(;i<maxI;++i)
      Array.getItem(i)->index=i;
    Reset();
  }
  else
    Free();

  return rc;
}

int __cdecl UserDefinedList::CmpItems(const UserDefinedListItem **el1,
  const UserDefinedListItem **el2)
{
  if(el1==el2)
    return 0;
  else if((**el1).index==(**el2).index)
    return 0;
  else if((**el1).index<(**el2).index)
    return -1;
  else
    return 1;
}

const wchar_t *UserDefinedList::Skip(const wchar_t *Str, int &Length, int &RealLength, BOOL &Error)
{
   Length=RealLength=0;
   Error=FALSE;

   while(IsSpace(*Str)) ++Str;
   if(*Str==Separator1 || *Str==Separator2) ++Str;
   while(IsSpace(*Str)) ++Str;
   if(!*Str) return NULL;

   const wchar_t *cur=Str;
   BOOL InBrackets=FALSE, InQoutes = (*cur==L'\"');


   if(!InQoutes) // если мы в кавычках, то обработка будет позже и чуть сложнее
     while(*cur) // важно! проверка *cur!=0 должна стоять первой
     {
        if(ProcessBrackets)
        {
           if(*cur==L']')
             InBrackets=FALSE;

           if(*cur==L'[' && NULL!=wcschr(cur+1, L']'))
             InBrackets=TRUE;
        }

        if(!InBrackets && (*cur==Separator1 || *cur==Separator2))
          break;

        ++cur;
     }

   if(!InQoutes || !*cur)
    {
      RealLength=Length=(int)(cur-Str);
      --cur;
      while(IsSpace(*cur))
       {
         --Length;
         --cur;
       }
      return Str;
    }

   // мы в кавычках - захватим все отсюда и до следующих кавычек
   ++cur;
   const wchar_t *QuoteEnd=wcschr(cur, L'\"');
   if(QuoteEnd==NULL)
    {
      Error=TRUE;
      return NULL;
    }

   const wchar_t *End=QuoteEnd+1;
   while(IsSpace(*End)) ++End;
   if(!*End || *End==Separator1 || *End==Separator2)
   {
     Length=(int)(QuoteEnd-cur);
     RealLength=(int)(End-cur);
     return cur;
   }

   Error=TRUE;
   return NULL;
}

void UserDefinedList::Reset(void)
{
  CurrentItem=0;
}

BOOL UserDefinedList::IsEmpty()
{
  unsigned int Size=Array.getSize();
  return !Size || CurrentItem>=Size;
}

const wchar_t *UserDefinedList::GetNext()
{
  UserDefinedListItem *item=Array.getItem(CurrentItem);
  ++CurrentItem;
  return item?item->Str:NULL;
}
