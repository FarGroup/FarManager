/*
namelist.cpp

Список имен файлов, используется в viewer при нажатии Gray+/Gray-

*/

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"
#include "namelist.hpp"

NamesList::NamesList()
{
  Init();
}

NamesList::~NamesList()
{
}

void NamesList::AddName(const wchar_t *Name,const wchar_t *ShortName)
{
    CurName.Value.strName = Name?Name:L"";
    CurName.Value.strShortName = ShortName?ShortName:L"";
    Names.push_back(CurName);
}


bool NamesList::GetNextName(string &strName, string &strShortName)
{
  if(Names.isEnd())
    return(false);

  const OneName *pName=Names.toNext();

  strName = pName->Value.strName;
  strShortName = pName->Value.strShortName;

  return(true);
}


bool NamesList::GetPrevName (string &strName, string &strShortName)
{
  if (Names.isBegin())
    return(false);

  const OneName *pName=Names.toPrev();

  strName = pName->Value.strName;
  strShortName = pName->Value.strShortName;

  return(true);
}


void NamesList::SetCurName(const wchar_t *Name)
{
    Names.storePosition();

    pCurName=Names.toBegin();

    while ( pCurName )
    {
        if ( !wcscmp (Name, pCurName->Value.strName) )
            return;

        pCurName=Names.toNext();
    }

    Names.restorePosition();
}


void NamesList::MoveData(NamesList &Dest)
{
    Dest.Names.swap(Names);
    Dest.CurName=CurName;

    Dest.strCurrentDir = strCurrentDir;

    Init();
}


void NamesList::GetCurDir (string &strDir)
{
    strDir = strCurrentDir;
}


void NamesList::SetCurDir (const wchar_t *Dir)
{
  strCurrentDir = Dir;
  PrepareDiskPathW(strCurrentDir);
}

void NamesList::Init()
{
    Names.clear();

    CurName.Value.strName = L"";
    CurName.Value.strShortName = L"";

    strCurrentDir = L"";
}
