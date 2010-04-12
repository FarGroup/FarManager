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

void NamesList::AddName(const char *Name,const char *ShortName)
{
	xstrncpy(CurName.Value.Name,Name,sizeof(CurName.Value.Name)-1);
	xstrncpy(CurName.Value.ShortName,NullToEmpty(ShortName),sizeof(CurName.Value.ShortName)-1);
	Names.push_back(CurName);
}


bool NamesList::GetNextName(char *Name, const size_t NameSize,char *ShortName, const size_t ShortNameSize)
{
	if (Names.isEnd())
		return(false);

	const OneName *pName=Names.toNext();
	xstrncpy(Name, pName->Value.Name, NameSize-1);

	if (ShortName)
		xstrncpy(ShortName, pName->Value.ShortName, ShortNameSize-1);

	return(true);
}


bool NamesList::GetPrevName(char *Name, const size_t NameSize,char *ShortName, const size_t ShortNameSize)
{
	if (Names.isBegin())
		return(false);

	const OneName *pName=Names.toPrev();
	xstrncpy(Name, pName->Value.Name, NameSize-1);

	if (ShortName)
		xstrncpy(ShortName, pName->Value.ShortName, ShortNameSize-1);

	return(true);
}


void NamesList::SetCurName(const char *Name)
{
	Names.storePosition();
	pCurName=Names.toBegin();

	while (pCurName)
	{
		if (!strcmp(Name,pCurName->Value.Name))
			return;

		pCurName=Names.toNext();
	}

	Names.restorePosition();
}


void NamesList::MoveData(NamesList &Dest)
{
	Dest.Names.swap(Names);
	Dest.CurName=CurName;
	strcpy(Dest.CurDir,CurDir);
	Init();
}


void NamesList::GetCurDir(char *Dir,int DestSize)
{
	if (*CurDir)
		xstrncpy(Dir,CurDir,DestSize);
	else
		*Dir=0;
}


void NamesList::SetCurDir(const char *Dir)
{
	_CHANGEDIR(CleverSysLog clv("NamesList::SetCurDir"));
	_CHANGEDIR(SysLog("(Dir  =\"%s\")",Dir));
	_CHANGEDIR(SysLog("CurDir=\"%s\"",CurDir));

	if (LocalStricmp(CurDir,Dir) || !TestCurrentDirectory(Dir))
		PrepareDiskPath(xstrncpy(CurDir,Dir,sizeof(CurDir)),sizeof(CurDir)-1);
}

void NamesList::Init()
{
	Names.clear();
	CurName.Value.Name[0]=0;
	CurName.Value.ShortName[0]=0;
	*CurDir=0;
}
