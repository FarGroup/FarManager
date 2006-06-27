/*
udlist.cpp

������ ����-����, �������������� ����� ������-�����������. ���� �����, �����
������� ������ �������� �����������, �� ���� ������� ������� ��������� �
�������. ���� ����� ����������� ������ ������ � ������ ���, �� ���������, ���
��� �� �����������, � ������� ������.

*/

/* Revision: 1.15 23.05.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "udlist.hpp"
#include "fn.hpp"

//unicode

UserDefinedListItemW::~UserDefinedListItemW()
{
  if(Str)
    xf_free(Str);
}

bool UserDefinedListItemW::operator==(const UserDefinedListItemW &rhs) const
{
  return (Str && rhs.Str)?(!LocalStricmpW(Str, rhs.Str)):false;
}

int UserDefinedListItemW::operator<(const UserDefinedListItemW &rhs) const
{
  if(!Str)
    return 1;
  else if(!rhs.Str)
    return -1;
  else
    return LocalStricmpW(Str, rhs.Str)<0;
}

const UserDefinedListItemW& UserDefinedListItemW::operator=(const
  UserDefinedListItemW &rhs)
{
  if(this!=&rhs)
  {
    if(Str)
    {
      xf_free(Str);
      Str=NULL;
    }
    if(rhs.Str)
      Str=_wcsdup(rhs.Str);
    index=rhs.index;
  }
  return *this;
}

const UserDefinedListItemW& UserDefinedListItemW::operator=(const wchar_t *rhs)
{
  if(Str!=rhs)
  {
    if(Str)
    {
      xf_free(Str);
      Str=NULL;
    }
    if(rhs)
      Str=wcsdup(rhs);
  }
  return *this;
}

wchar_t *UserDefinedListItemW::set(const wchar_t *Src, unsigned int size)
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
      memcpy(Str, Src, size*sizeof(wchar_t));
      Str[size]=0;
    }
  }
  return Str;
}

UserDefinedListW::UserDefinedListW()
{
  Reset();
  SetParameters(0,0,0);
}

UserDefinedListW::UserDefinedListW(WORD separator1, WORD separator2,
                                 DWORD Flags)
{
  Reset();
  SetParameters(separator1, separator2, Flags);
}

void UserDefinedListW::SetDefaultSeparators()
{
  Separator1=L';';
  Separator2=L',';
}

BOOL UserDefinedListW::CheckSeparators() const
{
  return !((Separator1==L'\"' || Separator2==L'\"') ||
           (ProcessBrackets &&  (Separator1==L'[' || Separator2==L'[' ||
            Separator1==L']' || Separator2==L']'))
          );
}

BOOL UserDefinedListW::SetParameters(WORD separator1, WORD separator2,
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

void UserDefinedListW::Free()
{
  Array.Free();
  Reset();
}

BOOL UserDefinedListW::Set(const wchar_t *List, BOOL AddToList)
{
  if(AddToList)
  {
    if(List && !*List) // �����, ������ ���������
      return TRUE;
  }
  else
    Free();
  BOOL rc=FALSE;

  if(CheckSeparators() && List && *List)
  {
    int Length, RealLength;
    UserDefinedListItemW item;
    item.index=Array.getSize();
    if(*List!=Separator1 && *List!=Separator2)
    {
      Length=wcslen(List);
      BOOL Error=FALSE;
      const wchar_t *CurList=List;
      while(!Error &&
            NULL!=(CurList=Skip(CurList, Length, RealLength, Error)))
      {
        if(Length)
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
                      memcpy(item.Str+i, item.Str+i+1,
                        (wcslen(item.Str+i+1)+1)*sizeof(wchar_t));
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
                Length=wcslen(item.Str);
                /* $ 18.09.2002 DJ
                   ���������� �� 1 ���� ������, ��� ����
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
      while(IsSpaceW(*End)) ++End; // ��������� �����
      if(!*End) // ���� ����� ����������� ������ ������ � ������ ���,
      {         // �� ���������, ��� ��� �� �����������, � ������� ������
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
    else if(!Unique) // ����� �� ����������� ��� ���������������
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

int __cdecl UserDefinedListW::CmpItems(const UserDefinedListItemW **el1,
  const UserDefinedListItemW **el2)
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

const wchar_t *UserDefinedListW::Skip(const wchar_t *Str, int &Length, int &RealLength, BOOL &Error)
{
   Length=RealLength=0;
   Error=FALSE;

   while(IsSpaceW(*Str)) ++Str;
   if(*Str==Separator1 || *Str==Separator2) ++Str;
   while(IsSpaceW(*Str)) ++Str;
   if(!*Str) return NULL;

   const wchar_t *cur=Str;
   BOOL InBrackets=FALSE, InQoutes = (*cur==L'\"');


   if(!InQoutes) // ���� �� � ��������, �� ��������� ����� ����� � ���� �������
     while(*cur) // �����! �������� *cur!=0 ������ ������ ������
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
      RealLength=Length=cur-Str;
      --cur;
      while(IsSpaceW(*cur))
       {
         --Length;
         --cur;
       }
      return Str;
    }

   // �� � �������� - �������� ��� ������ � �� ��������� �������
   ++cur;
   const wchar_t *QuoteEnd=wcschr(cur, L'\"');
   if(QuoteEnd==NULL)
    {
      Error=TRUE;
      return NULL;
    }

   const wchar_t *End=QuoteEnd+1;
   while(IsSpaceW(*End)) ++End;
   if(!*End || *End==Separator1 || *End==Separator2)
   {
     Length=QuoteEnd-cur;
     RealLength=End-cur;
     return cur;
   }

   Error=TRUE;
   return NULL;
}

void UserDefinedListW::Reset(void)
{
  CurrentItem=0;
}

BOOL UserDefinedListW::IsEmpty()
{
  unsigned int Size=Array.getSize();
  return !Size || CurrentItem>=Size;
}

const wchar_t *UserDefinedListW::GetNext()
{
  UserDefinedListItemW *item=Array.getItem(CurrentItem);
  ++CurrentItem;
  return item?item->Str:NULL;
}
