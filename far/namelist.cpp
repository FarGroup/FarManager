/*
namelist.cpp

Список имен файлов, используется в viewer при нажатии Gray+/Gray-

*/

/* Revision: 1.08 14.10.2003 $ */

/*
Modify:
  14.10.2003 SVS
    ! Перетрях в NamesList.
    ! NamesList::GetCurDir - имеет доп. параметр - требуемый размер.
    + NamesList::Init()
  21.01.2003 SVS
    + xf_malloc,xf_realloc,xf_free - обертки вокруг malloc,realloc,free
      Просьба блюсти порядок и прописывать именно xf_* вместо простых.
  08.05.2002 SVS
    ! Проверка на NULL перед free()
  06.12.2001 SVS
    ! PrepareDiskPath() - имеет доп.параметр - максимальный размер буфера
  26.11.2001 SVS
    ! Заюзаем PrepareDiskPath() для преобразования пути.
  06.05.2001 DJ
    ! перетрях #include
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

#include "fn.hpp"
#include "namelist.hpp"

NamesList::NamesList(DWORD _MemSize)
{
  Init();
  if((MemSize=_MemSize) > 0)
  {
    Names=(char *)xf_malloc(MemSize);
    if (!Names)
      MemSize=0;
    else
      *Names=0;
  }
}

NamesList::~NamesList()
{
  if(Names)
    xf_free(Names);
}


void NamesList::AddName(char *Name)
{
  int Length=strlen(Name)+1;
  if(NamesSize+Length >= MemSize)
  {
    char *NewNames=(char *)xf_realloc(Names,NamesSize+Length);
    if (NewNames==NULL)
      return;
    Names=NewNames;
  }
  memcpy(Names+NamesSize,Name,Length);
  NamesSize+=Length;
  MemSize+=Length;
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
  DWORD NamePos=0;
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
  if(CurName)
  {
    Dest->CurName=CurName;
    Dest->CurNamePos=CurNamePos;
  }
  else
  {
    Dest->CurName=Names;
    Dest->CurNamePos=0;
  }
  Dest->NamesNumber=NamesNumber;
  Dest->NamesSize=NamesSize;
  strcpy(Dest->CurDir,CurDir);
  Init();
}


void NamesList::GetCurDir(char *Dir,int DestSize)
{
  if(*CurDir)
    strncpy(Dir,CurDir,DestSize);
  else
    *Dir=0;
}


void NamesList::SetCurDir(char *Dir)
{
  PrepareDiskPath(strncpy(CurDir,Dir,sizeof(CurDir)),sizeof(CurDir)-1);
}

void NamesList::Init()
{
  Names=CurName=NULL;
  CurNamePos=NamesNumber=0;
  NamesSize=MemSize=0;
  *CurDir=0;
}
