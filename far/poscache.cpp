/*
poscache.cpp

Кэш позиций в файлах для viewer/editor

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */

FilePositionCache::FilePositionCache()
{
  memset(Names,0,sizeof(Names));
  memset(Positions1,0,sizeof(Positions1));
  memset(Positions2,0,sizeof(Positions2));
  memset(Positions3,0,sizeof(Positions3));
  memset(Positions4,0,sizeof(Positions4));
  memset(Positions5,0,sizeof(Positions5));
  CurPos=0;
}


void FilePositionCache::AddPosition(char *Name,unsigned int Position1,
     unsigned int Position2,unsigned int Position3,unsigned int Position4,
     unsigned int Position5)
{
  char FullName[3*NM];
  if (*Name=='<')
    strcpy(FullName,Name);
  else
    ConvertNameToFull(Name,FullName);
  strcpy(Names[CurPos],FullName);
  Positions1[CurPos]=Position1;
  Positions2[CurPos]=Position2;
  Positions3[CurPos]=Position3;
  Positions4[CurPos]=Position4;
  Positions5[CurPos]=Position5;
  if (++CurPos>=sizeof(Names)/sizeof(Names[0]))
    CurPos=0;
}


void FilePositionCache::GetPosition(char *Name,unsigned int &Position1,
     unsigned int &Position2,unsigned int &Position3,unsigned int &Position4,
     unsigned int &Position5)
{
  char FullName[3*NM];
  if (*Name=='<')
    strcpy(FullName,Name);
  else
    ConvertNameToFull(Name,FullName);
  Position1=Position2=Position3=Position4=Position5=0;
  for (int I=1;I<=sizeof(Names)/sizeof(Names[0]);I++)
  {
    int Pos=CurPos-I;
    if (Pos<0)
      Pos+=sizeof(Names)/sizeof(Names[0]);
    if (LocalStricmp(Names[Pos],FullName)==0)
    {
      Position1=Positions1[Pos];
      Position2=Positions2[Pos];
      Position3=Positions3[Pos];
      Position4=Positions4[Pos];
      Position5=Positions5[Pos];
      break;
    }
  }
}


void FilePositionCache::Read(char *Key)
{
  for (int I=0;I<sizeof(Names)/sizeof(Names[0]);I++)
  {
    char SubKey[100],DataStr[512];
    sprintf(SubKey,"Item%d",I);
    GetRegKey(Key,SubKey,DataStr,"",sizeof(DataStr));

    char ArgData[2*NM],*DataPtr=DataStr;
    for (int J=0;(DataPtr=GetCommaWord(DataPtr,ArgData))!=NULL;J++)
      if (*ArgData=='$')
        strcpy(Names[I],ArgData+1);
      else
        switch(J)
        {
          case 0:
            Positions1[I]=atoi(ArgData);
            break;
          case 1:
            Positions2[I]=atoi(ArgData);
            break;
          case 2:
            Positions3[I]=atoi(ArgData);
            break;
          case 3:
            Positions4[I]=atoi(ArgData);
            break;
          case 4:
            Positions5[I]=atoi(ArgData);
            break;
        }
  }
}


void FilePositionCache::Save(char *Key)
{
  for (int I=0;I<sizeof(Names)/sizeof(Names[0]);I++)
  {
    int Pos=CurPos+I;
    if (Pos>=sizeof(Names)/sizeof(Names[0]))
      Pos-=sizeof(Names)/sizeof(Names[0]);
    char SubKey[100],DataStr[512];
    sprintf(SubKey,"Item%d",I);
    sprintf(DataStr,"%d,%d,%d,%d,%d,$%s",Positions1[Pos],Positions2[Pos],
            Positions3[Pos],Positions4[Pos],Positions5[Pos],Names[Pos]);
    SetRegKey(Key,SubKey,DataStr);
  }
}

