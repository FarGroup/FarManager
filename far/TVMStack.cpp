/*
TVMStack.cpp

Стек (LIFO) для виртуальной машины макросов

*/

#include "headers.hpp"
#pragma hdrstop

#include "TVMStack.hpp"

TVar TVMStack::errorStack;

TVar TVMStack::Pop()
{
  if(Top)
  {
    --Total;
    TVar Destination=Top->Item;
    current=Top->Next;
    delete Top;
    Top=current;
    return Destination;
  }
  return TVMStack::errorStack;
}

TVar TVMStack::Peek()
{
  if(Top)
    return Top->Item;
  return TVMStack::errorStack;
}

TVar TVMStack::Push(const TVar &Source)
{
  current=new OneItem(Source,Top);
  if(current)
  {
    Top=current;
    ++Total;
    return Top->Item;
  }
  return TVMStack::errorStack;
}

void TVMStack::Free()
{
  while(Top)
  {
    current=Top->Next;
    delete Top;
    Top=current;
  }
  Total=0;
}
