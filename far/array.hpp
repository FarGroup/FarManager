#ifndef __TArray_hpp
#define __TArray_hpp

/*
array.hpp

������ ������ � ��������

 TArray<Object> Array;
 // Object ������ ����� ����������� �� ��������� � ��������� ���������
 //  bool operator==(const Object &) const
 //  bool operator<(const Object &) const
 //  const Object& operator=(const Object &)
*/

/* Revision: 1.04 24.04.2005 $ */

/*
Modify:
  24.04.2005 AY
    ! GCC
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  25.02.2003 SVS
    ! "free/malloc/realloc -> xf_*" - ���-�� � ������� ��� ���������.
  21.08.2002 IS
    ! �� ������� Sort, ���� ������ ����
  15.08.2002 IS
    + ������� � ����� (���������� �� ���������� S&R)
*/
#include "fn.hpp"

#if defined(__BORLANDC__) || defined(__GNUC__)
typedef int __cdecl (*TARRAYCMPFUNC)(const void *el1,const void *el2);
#else
typedef int (*TARRAYCMPFUNC)(const void *el1,const void *el2);
#endif

template <class Object>
class TArray
{
  private:
    unsigned int internalCount, Count, Delta;
    Object **items;

  private:
    static int __cdecl CmpItems(const Object **el1, const Object **el2);
    bool deleteItem(unsigned int index);

  public:
    TArray(unsigned int Delta=8);
    ~TArray() { Free(); }
    TArray<Object>(const TArray<Object> &rhs);
    TArray& operator=(const TArray<Object> &rhs);

  public:
    void Free();
    void setDelta(unsigned int newDelta);
    bool setSize(unsigned int newSize);
    Object *setItem(unsigned int index, const Object &newItem);
    Object *getItem(unsigned int index);
    int getIndex(const Object &item, int start=-1);
    void Sort(TARRAYCMPFUNC user_cmp_func=NULL); // ������������� ������
    bool Pack(); // ��������� ������ - ������ ���������� ���������� ���������,
                 // ������ ������, �������� ������ ����. ����������, false,
                 // ���� ��������� ������� �� �������������.
                 // ����� Pack() ����� Sort(NULL) �������� � ����������
                 // ����������

  public: // inline
    unsigned int getSize() const { return Count; }
    Object *addItem(const Object &newItem) { return setItem(Count,newItem); }
};

template <class Object>
TArray<Object>::TArray(unsigned int delta):
  items(NULL), Count(0), internalCount(0)
{
  setDelta(delta);
}

template <class Object>
void TArray<Object>::Free()
{
  if(items)
  {
    for(unsigned i=0;i<Count;++i)
      delete items[i];
    xf_free(items);
    items=NULL;
  }
  Count=internalCount=0;
}

template <class Object>
Object *TArray<Object>::setItem(unsigned int index, const Object &newItem)
{
  bool set=true;
  if(index<Count)
    deleteItem(index);
  else
    set=setSize(index+(index==Count));

  if(set)
  {
    items[index]=new Object;
    if(items[index])
      *items[index]=newItem;
    return items[index];
  }
  return NULL;
}

template <class Object>
Object *TArray<Object>::getItem(unsigned int index)
{
  return (index<Count)?items[index]:NULL;
}

template <class Object>
void TArray<Object>::Sort(TARRAYCMPFUNC user_cmp_func)
{
  if(Count)
  {
    if(!user_cmp_func)
      user_cmp_func=reinterpret_cast<TARRAYCMPFUNC>(CmpItems);
    far_qsort(reinterpret_cast<char*>(items),Count,sizeof(Object*),user_cmp_func);
  }
}

template <class Object>
bool TArray<Object>::Pack()
{
  bool was_changed=false;
  for (unsigned int index=1; index<Count; ++index)
  {
    if(*items[index-1]==*items[index])
    {
      deleteItem(index);
      was_changed=true;
      --Count;
      if(index<Count)
      {
        memcpy(&items[index], &items[index+1], sizeof(Object*)*(Count-index));
        --index;
      }
    }
  }
  return was_changed;
}

template <class Object>
bool TArray<Object>::deleteItem(unsigned int index)
{
  if(index<Count)
  {
    delete items[index];
    items[index]=NULL;
    return true;
  }
  return false;
}

template <class Object>
bool TArray<Object>::setSize(unsigned int newSize)
{
  bool rc=false;
  unsigned int i;

  if(newSize < Count)               // ���������� �������
  {
    for(i=newSize;i<Count;++i)
    {
      delete items[i];
      items[i]=NULL;
    }
    Count=newSize;
    rc=true;
  }
  else if (newSize < internalCount) // ����������, �� � ������ ����������
  {
    for(i=Count;i<newSize;++i)
      items[i]=NULL;
    Count=newSize;
    rc=true;
  }
  else                              // ���������� �������
  {
     unsigned int Remainder=newSize%Delta;
     unsigned int newCount=Remainder?(newSize+Delta)-Remainder:
       newSize?newSize:Delta;
     Object **newItems=static_cast<Object**>(xf_malloc(newCount*sizeof(Object*)));
     if(newItems)
     {
       if(items)
       {
         memcpy(newItems,items,Count*sizeof(Object*));
         xf_free(items);
       }
       items=newItems;
       internalCount=newCount;
       for(i=Count;i<newSize;++i)
         items[i]=NULL;
       Count=newSize;
       rc=true;
     }
  }
  return rc;
}

template <class Object>
int __cdecl TArray<Object>::CmpItems(const Object **el1, const Object **el2)
{
  if(el1==el2)
    return 0;
  else if(**el1==**el2)
    return 0;
  else if(**el1<**el2)
    return -1;
  else
    return 1;
}

template <class Object>
TArray<Object>::TArray(const TArray<Object> &rhs):
  items(NULL), Count(0), internalCount(0)
{
  *this=rhs;
}

template <class Object>
TArray<Object>& TArray<Object>::operator=(const TArray<Object> &rhs)
{
  setDelta(rhs.Delta);
  if(setSize(rhs.Count))
  {
    for(unsigned i=0;i<Count;++i)
    {
      if(rhs.items[i])
      {
        if(!items[i])
          items[i]=new Object;
        if(items[i])
          *items[i]=*rhs.items[i];
        else
        {
          Free();
          break;
        }
      }
      else
      {
        delete items[i];
        items[i]=NULL;
      }
    }
  }
  return *this;
}

template <class Object>
void TArray<Object>::setDelta(unsigned int newDelta)
{
  if(newDelta<4)
    newDelta=4;
  Delta=newDelta;
}

template <class Object>
int TArray<Object>::getIndex(const Object &item, int start)
{
  int rc=-1;
  if(start==-1)
    start=0;
  for (unsigned int i=start; i<Count; ++i)
  {
    if(items[i] && item==*items[i])
    {
      rc=i;
      break;
    }
  }
  return rc;
}

#endif // __TArray_hpp
