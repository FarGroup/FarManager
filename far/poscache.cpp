/*
poscache.cpp

Кэш позиций в файлах для viewer/editor

*/

/* Revision: 1.11 11.08.2001 $ */

/*
Modify:
  11.08.2001 IS
    ! У UserDefinedList исчез доп. параметр в конструкторе :)
  02.07.2001 IS
    ! У UserDefinedList появился доп. параметр в конструкторе...
  17.06.2001 IS
    ! Сменился формат записи - имя файла теперь берется в кавычки, т.к. оно
      может содержать разделители
    ! Для работы со списком используем не GetCommaWord, а UserDefinedList
  06.06.2001 SVS
    ! От конкретных чисел переходим к макросам + небольшая оптимизация.
  22.05.2001 tran
    ! по результам прогона на CodeGuard
  06.05.2001 DJ
    ! перетрях #include
  06.04.2001 VVM
    - Неправильное позиционирование в открытых файлах
  02.04.2001 VVM
    + Обработка Opt.FlagPosixSemantics и убирание дупов с помощью FindPosition()
  03.11.2000 OT
    ! Введение проверки возвращаемого значения
  02.11.2000 OT
    ! Введение проверки на длину буфера, отведенного под имя файла.
  24.09.2000 SVS
    + Работа по сохранению/восстановлению позиций в файле по RCtrl+<N>
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "poscache.hpp"
#include "global.hpp"
#include "fn.hpp"

/* $ 17.06.2001 IS
   + Имя файла должно быть взято в кавычки, т.к. оно может содержать
     символы-разделители
*/
static char EmptyPos[]="0,0,0,0,0,\"$\"";
/* IS $ */

FilePositionCache::FilePositionCache()
{
  if(!Opt.MaxPositionCache)
  {
    GetRegKey("System","MaxPositionCache",Opt.MaxPositionCache,MAX_POSITIONS);
    if(Opt.MaxPositionCache < 16 || Opt.MaxPositionCache > 128)
      Opt.MaxPositionCache=MAX_POSITIONS;
  }
  Names=(char*)malloc(Opt.MaxPositionCache*3*NM);
  Positions=(unsigned int*)malloc(Opt.MaxPositionCache*5*sizeof(unsigned int));
  ShortPos=(long*)malloc(Opt.MaxPositionCache*(BOOKMARK_COUNT*4)*sizeof(long));

  if(Names  && Positions && ShortPos)
  {
    memset(Names,0,Opt.MaxPositionCache*3*NM);
    memset(Positions,0,sizeof(unsigned int)*Opt.MaxPositionCache*5);
    memset(ShortPos,0xFF,sizeof(long)*Opt.MaxPositionCache*(BOOKMARK_COUNT*4));
    CurPos=0;
    IsMemory=1;
  }
  else
  {
    IsMemory=0;
  }

}

FilePositionCache::~FilePositionCache()
{
  if(IsMemory)
  {
    free(Names);
    free(Positions);
    free(ShortPos);
  }
}

void FilePositionCache::AddPosition(char *Name,unsigned int Position1,
     unsigned int Position2,unsigned int Position3,unsigned int Position4,
     unsigned int Position5,
     long *SPosLine,long *SPosLeftPos,
     long *SPosCursor,long *SPosScreenLine)
{
  if(!IsMemory)
    return;

  char FullName[3*NM];
  if (*Name=='<') {
    strcpy(FullName,Name);
  } else {
//    ConvertNameToFull(Name,FullName, sizeof(FullName));
    if (ConvertNameToFull(Name,FullName, sizeof(FullName)) >= sizeof(FullName)){
      return;
    }
  }
  /* $ 06.04.2001 VVM
    - Неправильное позиционирование в открытых файлах
      Имена копировал до поиска, а не после :) */
  int Pos = FindPosition(FullName);
  if (Pos >= 0)
    CurPos = Pos;
  strcpy(&Names[CurPos*3*NM],FullName);
  /* VVM $ */
  Positions[CurPos*5+0]=Position1;
  Positions[CurPos*5+1]=Position2;
  Positions[CurPos*5+2]=Position3;
  Positions[CurPos*5+3]=Position4;
  Positions[CurPos*5+4]=Position5;

  memset(&ShortPos[CurPos*(BOOKMARK_COUNT*4)],0xFF,(BOOKMARK_COUNT*4)*sizeof(long));
  if(SPosLine)
    memcpy(&ShortPos[CurPos*(BOOKMARK_COUNT*4)+(BOOKMARK_COUNT*0)],SPosLine,BOOKMARK_COUNT*sizeof(long));
  if(SPosLeftPos)
    memcpy(&ShortPos[CurPos*(BOOKMARK_COUNT*4)+(BOOKMARK_COUNT*1)],SPosLeftPos,BOOKMARK_COUNT*sizeof(long));
  if(SPosCursor)
    memcpy(&ShortPos[CurPos*(BOOKMARK_COUNT*4)+(BOOKMARK_COUNT*2)],SPosCursor,BOOKMARK_COUNT*sizeof(long));
  if(SPosScreenLine)
    memcpy(&ShortPos[CurPos*(BOOKMARK_COUNT*4)+(BOOKMARK_COUNT*3)],SPosScreenLine,BOOKMARK_COUNT*sizeof(long));

  if (++CurPos>=Opt.MaxPositionCache)
    CurPos=0;
}


void FilePositionCache::GetPosition(char *Name,unsigned int &Position1,
     unsigned int &Position2,unsigned int &Position3,unsigned int &Position4,
     unsigned int &Position5,
     long *SPosLine,long *SPosLeftPos,
     long *SPosCursor,long *SPosScreenLine)
{
  if(!IsMemory)
    return;

  char FullName[3*NM];
  if (*Name=='<'){
    strcpy(FullName,Name);
  } else {
//    ConvertNameToFull(Name,FullName, sizeof(FullName));
    if (ConvertNameToFull(Name,FullName, sizeof(FullName)) >= sizeof(FullName)){
      return ;
    }
  }
  Position1=Position2=Position3=Position4=Position5=0;
  int Pos = FindPosition(FullName);
  if (Pos >= 0)
  {
    Position1=Positions[Pos*5+0];
    Position2=Positions[Pos*5+1];
    Position3=Positions[Pos*5+2];
    Position4=Positions[Pos*5+3];
    Position5=Positions[Pos*5+4];
    if(SPosLine)
      memcpy(SPosLine,&ShortPos[Pos*(BOOKMARK_COUNT*4)+(BOOKMARK_COUNT*0)],BOOKMARK_COUNT*sizeof(long));
    if(SPosLeftPos)
      memcpy(SPosLeftPos,&ShortPos[Pos*(BOOKMARK_COUNT*4)+(BOOKMARK_COUNT*1)],BOOKMARK_COUNT*sizeof(long));
    if(SPosCursor)
      memcpy(SPosCursor,&ShortPos[Pos*(BOOKMARK_COUNT*4)+(BOOKMARK_COUNT*2)],BOOKMARK_COUNT*sizeof(long));
    if(SPosScreenLine)
      memcpy(SPosScreenLine,&ShortPos[Pos*(BOOKMARK_COUNT*4)+(BOOKMARK_COUNT*3)],BOOKMARK_COUNT*sizeof(long));
  }
}

int FilePositionCache::FindPosition(char *FullName)
{
  for (int I=1;I<=Opt.MaxPositionCache;I++)
  {
    int Pos=CurPos-I;
    if (Pos<0)
      Pos+=Opt.MaxPositionCache;
    int CmpRes;
    if (Opt.FlagPosixSemantics)
      CmpRes = strcmp(&Names[Pos*3*NM],FullName);
    else
      CmpRes = LStricmp(&Names[Pos*3*NM],FullName);
    if (CmpRes == 0)
      return(Pos);
  }
  return(-1);
}

void FilePositionCache::Read(char *Key)
{
  if(!IsMemory)
    return;

  for (int I=0;I < Opt.MaxPositionCache;I++)
  {
    char SubKey[100],DataStr[512];
    sprintf(SubKey,"Item%d",I);
    GetRegKey(Key,SubKey,DataStr,"",sizeof(DataStr));

    if(!strcmp(DataStr,EmptyPos))
    {
      Names[I*3*NM]=0;
      memset(Positions+I*5,0,sizeof(unsigned int)*5);
    }
    else
    {
      /* $ 17.06.2001 IS
         ! Применяем интеллектуальный класс, а не GetCommaWord, которая не
           учитывает кавычки
      */
      UserDefinedList DataList('\"', 0, 0);
      int J=0;
      const char *DataPtr;
      char ArgData[2*NM];
      if(DataList.Set(DataStr))
      {
         DataList.Start();
         while(NULL!=(DataPtr=DataList.GetNext()))
         {
           if(*DataPtr=='$')
              strcpy(Names+I*3*NM,DataPtr+1);
           else
             switch(J)
             {
               case 0:
                 Positions[I*5+0]=atoi(DataPtr);
                 break;
               case 1:
                 Positions[I*5+1]=atoi(DataPtr);
                 break;
               case 2:
                 Positions[I*5+2]=atoi(DataPtr);
                 break;
               case 3:
                 Positions[I*5+3]=atoi(DataPtr);
                 break;
               case 4:
                 Positions[I*5+4]=atoi(DataPtr);
                 break;
             }
           ++J;
         }
      }
      /* IS $ */
    }
    sprintf(SubKey,"Short%d",I);
    memset(DataStr,0xff,(BOOKMARK_COUNT*4)*sizeof(long));
    GetRegKey(Key,SubKey,(LPBYTE)&ShortPos[I*(BOOKMARK_COUNT*4)],(LPBYTE)DataStr,(BOOKMARK_COUNT*4)*sizeof(long));
  }
}


void FilePositionCache::Save(char *Key)
{
  int J, I;
  if(!IsMemory)
    return;
  for (I=0;I < Opt.MaxPositionCache;I++)
  {
    int Pos=CurPos+I;
    if (Pos>=Opt.MaxPositionCache)
      Pos-=Opt.MaxPositionCache;
    char SubKey[100],DataStr[512];
    sprintf(SubKey,"Item%d",I);
    /* $ 17.06.2001 IS
       + Имя файла должно быть взято в кавычки, т.к. оно может содержать
         символы-разделители
    */
    sprintf(DataStr,"%d,%d,%d,%d,%d,\"$%s\"",Positions[Pos*5+0],Positions[Pos*5+1],
            Positions[Pos*5+2],Positions[Pos*5+3],Positions[Pos*5+4],&Names[Pos*3*NM]);
    /* IS $ */
    SetRegKey(Key,SubKey,DataStr);
    if((Opt.SaveViewerShortPos && Opt.SaveViewerPos) ||
       (Opt.SaveEditorShortPos && Opt.SaveEditorPos))
    {
      // Если не запоминались позиции по RCtrl+<N>, то и не записываем их
      for(J=0; J < (BOOKMARK_COUNT*4); J++)
        if(ShortPos[Pos*(BOOKMARK_COUNT*4)+J] != -1)
          break;

      sprintf(SubKey,"Short%d",I);
      if(J < (BOOKMARK_COUNT*4))
        SetRegKey(Key,SubKey,(LPBYTE)&ShortPos[Pos*(BOOKMARK_COUNT*4)],(BOOKMARK_COUNT*4)*sizeof(long));
      else
        DeleteRegValue(Key,SubKey);
    }
  }
}
