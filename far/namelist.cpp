/*
namelist.cpp

Список имен файлов, используется в viewer при нажатии Gray+/Gray-

*/

/* Revision: 1.02 20.02.2001 $ */

/*
Modify:
  20.02.2001 SVS
    ! Заголовки - к общему виду!
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop
#include "internalheaders.hpp"

NamesList::NamesList()
{
  Names=CurName=NULL;
  CurNamePos=NamesNumber=NamesSize=0;
  *CurDir=0;
}


NamesList::~NamesList()
{
  /* $ 13.07.2000 SVS
     распределение памяти было чрезе realloc
  */
  free(Names);
  /* SVS $ */
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
