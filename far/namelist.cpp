/*
namelist.cpp

Список имен файлов, используется в viewer при нажатии Gray+/Gray-

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#define STRICT

#ifndef __ALLOC_H
#include <alloc.h>
#endif
#ifndef __STRING_H
#include <string.h>
#endif
#if !defined(__NEW_H)
#pragma option -p-
#include <new.h>
#pragma option -p.
#endif

#ifndef __FARCONST_HPP__
#include "farconst.hpp"
#endif
#ifndef __NAMELIST_HPP__
#include "namelist.hpp"
#endif

NamesList::NamesList()
{
  Names=CurName=NULL;
  CurNamePos=NamesNumber=NamesSize=0;
  *CurDir=0;
}


NamesList::~NamesList()
{
  delete Names;
}


void NamesList::AddName(char *Name)
{
  int Length=strlen(Name)+1;
  char *NewNames=(char *)realloc(Names,NamesSize+Length);
  if (NewNames==NULL)
    return;
  Names=NewNames;
  memcpy(Names+NamesSize,Name,Length);
  NamesSize+=Length;
  NamesNumber++;
}


bool NamesList::GetNextName(char *Name)
{
  if (CurNamePos>=NamesNumber-1)
    return(false);
  CurNamePos++;
  CurName+=strlen(CurName)+1;
  strcpy(Name,CurName);
  return(true);
}


bool NamesList::GetPrevName(char *Name)
{
  if (CurNamePos<=0)
    return(false);
  CurNamePos--;
  CurName-=2;
  while (CurName!=Names && *(CurName-1)!=0)
    CurName--;
  strcpy(Name,CurName);
  return(true);
}


void NamesList::SetCurName(char *Name)
{
  char *CheckName=Names;
  int NamePos=0;
  while (NamePos<NamesNumber)
    if (strcmp(Name,CheckName)==0)
    {
      CurName=CheckName;
      CurNamePos=NamePos;
      break;
    }
    else
    {
      CheckName+=strlen(CheckName)+1;
      NamePos++;
    }
}


void NamesList::MoveData(NamesList *Dest)
{
  Dest->Names=Names;
  Dest->CurName=CurName;
  Dest->CurNamePos=CurNamePos;
  Dest->NamesNumber=NamesNumber;
  Dest->NamesSize=NamesSize;
  strcpy(Dest->CurDir,CurDir);
  Names=CurName=NULL;
}


void NamesList::GetCurDir(char *Dir)
{
  strcpy(Dir,CurDir);
}


void NamesList::SetCurDir(char *Dir)
{
  strcpy(CurDir,Dir);
}

