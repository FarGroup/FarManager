/*
poscache.cpp

Кэш позиций в файлах для viewer/editor

*/

/* Revision: 1.15 17.12.2002 $ */

/*
Modify:
  17.12.2002 SVS
    ! класс FilePositionCache подвергся значительным переделкам,
     т.к. позиции вьювере измеряются в терминах __int64, а редактора - DWORD
     Т.е. класс теперь понимает 2 типа кэша - FPOSCACHE_32 и FPOSCACHE_64.
  15.08.2002 IS
    ! DirList.Start -> DirList.Reset
  29.10.2001 IS
    ! SaveEditorPos и SaveEditorShortPos переехали в EditorOptions
  20.08.2001 VVM
    ! Ошибка при задании разделителя списка.
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
#include "udlist.hpp"

#define MSIZE_PARAM            (Opt.MaxPositionCache*SizeValue*5)
#define MSIZE_POSITION         (BOOKMARK_COUNT*Opt.MaxPositionCache*SizeValue*4)

#define PARAM_POS(Pos)         ((Pos)*SizeValue*5)
#define POSITION_POS(Pos,Idx)  ((Pos)*BOOKMARK_COUNT*SizeValue*4+(Idx)*BOOKMARK_COUNT*SizeValue)

/* $ 17.06.2001 IS
   + Имя файла должно быть взято в кавычки, т.к. оно может содержать
     символы-разделители
*/
static char EmptyPos[]="0,0,0,0,0,\"$\"";
/* IS $ */

char FilePositionCache::SubKeyItem[16]="Item";
char FilePositionCache::SubKeyShort[16]="Short";
char *FilePositionCache::PtrSubKeyItem;
char *FilePositionCache::PtrSubKeyShort;


FilePositionCache::FilePositionCache(int TypeCache)
{
  PtrSubKeyItem=SubKeyItem+strlen(SubKeyItem);
  PtrSubKeyShort=SubKeyShort+strlen(SubKeyShort);

  if(!Opt.MaxPositionCache)
  {
    GetRegKey("System","MaxPositionCache",Opt.MaxPositionCache,MAX_POSITIONS);
    if(Opt.MaxPositionCache < 16 || Opt.MaxPositionCache > 128)
      Opt.MaxPositionCache=MAX_POSITIONS;
  }

  SizeValue=!TypeCache?sizeof(DWORD):sizeof(__int64);

  Param=NULL;
  Position=NULL;
  IsMemory=0;
  CurPos=0;

  if((Names=(char*)malloc(Opt.MaxPositionCache*3*NM)) != NULL)
  {
    Param=(BYTE*)malloc(MSIZE_PARAM);
    Position=(BYTE*)malloc(MSIZE_POSITION);
    if(Param && Position)
    {
      memset(Names,0,Opt.MaxPositionCache*3*NM);
      memset(Param,0,MSIZE_PARAM);
      memset(Position,0xFF,MSIZE_POSITION);
      IsMemory=1;
    }
    else
    {
      if(Param)       free(Param);       Param=NULL;
      if(Position)    free(Position);    Position=NULL;
    }
  }
}

FilePositionCache::~FilePositionCache()
{
  if(Names)     free(Names);
  if(Param)     free(Param);
  if(Position)  free(Position);
}

void FilePositionCache::AddPosition(const char *Name,void *PosCache)
{
  if(!IsMemory || !PosCache)
    return;

  char FullName[3*NM];
  if (*Name=='<')
  {
    strcpy(FullName,Name);
  }
  else
  {
    if (ConvertNameToFull(Name,FullName, sizeof(FullName)) >= sizeof(FullName))
    {
      return;
    }
  }
  /* $ 06.04.2001 VVM
    - Неправильное позиционирование в открытых файлах
      Имена копировал до поиска, а не после :) */
  int Pos = FindPosition(FullName);
  if (Pos >= 0)
    CurPos = Pos;
  strcpy(Names+CurPos*3*NM,FullName);
  /* VVM $ */

  int I;

  memset(Position+POSITION_POS(CurPos,0),0xFF,(BOOKMARK_COUNT*4)*SizeValue);
  memcpy(Param+PARAM_POS(CurPos),PosCache,SizeValue*5); // При условии, что в TPosCache?? Param стоит первым :-)

  if(SizeValue == sizeof(DWORD))
  {
    for(I=0; I < 4; ++I)
      if(((struct TPosCache32*)PosCache)->Position[I])
        memcpy(Position+POSITION_POS(CurPos,I),((struct TPosCache32*)PosCache)->Position[I],BOOKMARK_COUNT*SizeValue);
  }
  else
  {
    for(I=0; I < 4; ++I)
      if(((struct TPosCache64*)PosCache)->Position[I])
        memcpy(Position+POSITION_POS(CurPos,I),((struct TPosCache64*)PosCache)->Position[I],BOOKMARK_COUNT*SizeValue);
  }

  if (++CurPos>=Opt.MaxPositionCache)
    CurPos=0;
}



BOOL FilePositionCache::GetPosition(const char *Name,void *PosCache)
{
  if(!IsMemory || !PosCache)
    return FALSE;

  char FullName[3*NM];
  if (*Name=='<')
  {
    strcpy(FullName,Name);
  }
  else
  {
    if (ConvertNameToFull(Name,FullName, sizeof(FullName)) >= sizeof(FullName))
    {
      return FALSE;
    }
  }

  int Pos = FindPosition(FullName);

  //memset(Position+POSITION_POS(CurPos,0),0xFF,(BOOKMARK_COUNT*4)*SizeValue);
  //memcpy(Param+PARAM_POS(CurPos),PosCache,SizeValue*5); // При условии, что в TPosCache?? Param стоит первым :-)

  if (Pos >= 0)
  {
    int I;
    memcpy(PosCache,Param+PARAM_POS(Pos),SizeValue*5); // При условии, что в TPosCache?? Param стоит первым :-)
    if(SizeValue == sizeof(DWORD))
    {
      for(I=0; I < 4; ++I)
        if(((struct TPosCache32*)PosCache)->Position[I])
          memcpy(((struct TPosCache32*)PosCache)->Position[I],Position+POSITION_POS(Pos,I),BOOKMARK_COUNT*SizeValue);
    }
    else
    {
      for(I=0; I < 4; ++I)
        if(((struct TPosCache64*)PosCache)->Position[I])
          memcpy(((struct TPosCache64*)PosCache)->Position[I],Position+POSITION_POS(Pos,I),BOOKMARK_COUNT*SizeValue);
    }
    return TRUE;
  }
  memset(PosCache,0,SizeValue*5); // При условии, что в TPosCache?? Param стоит первым :-)
  return FALSE;
}

int FilePositionCache::FindPosition(const char *FullName)
{
  for (int I=1;I<=Opt.MaxPositionCache;I++)
  {
    int Pos=CurPos-I;
    if (Pos<0)
      Pos+=Opt.MaxPositionCache;

    int CmpRes;
    char *Ptr=Names+Pos*3*NM;

    if (Opt.FlagPosixSemantics)
      CmpRes = strcmp(Ptr,FullName);
    else
      CmpRes = LStricmp(Ptr,FullName);

    if (CmpRes == 0)
      return(Pos);
  }
  return(-1);
}

BOOL FilePositionCache::Read(const char *Key)
{
  if(!IsMemory)
    return FALSE;

  char DataStr[8096];
  BYTE DefPos[8096];

  memset(DefPos,0xff,(BOOKMARK_COUNT*4)*SizeValue);

  for (int I=0;I < Opt.MaxPositionCache;I++)
  {
    itoa(I,PtrSubKeyItem,10);
    itoa(I,PtrSubKeyShort,10);

    GetRegKey(Key,SubKeyShort,(LPBYTE)Position+POSITION_POS(I,0),(LPBYTE)DefPos,(BOOKMARK_COUNT*4)*SizeValue);
    GetRegKey(Key,SubKeyItem,DataStr,EmptyPos,sizeof(DataStr));

    if(!strcmp(DataStr,EmptyPos))
    {
      Names[I*3*NM]=0;
      memset(Param+PARAM_POS(I),0,SizeValue*5);
    }
    else
    {
      /* $ 17.06.2001 IS
         ! Применяем интеллектуальный класс, а не GetCommaWord, которая не
           учитывает кавычки
      */
      /* $ 20.08.2001 VVM
         ! Ошибка при задании разделителя списка. */
//      UserDefinedList DataList('\"', 0, 0);
      UserDefinedList DataList(0,0,0);
      /* VVM $ */
      int J=0;
      const char *DataPtr;
      char ArgData[2*NM];

      if(DataList.Set(DataStr))
      {
         DataList.Reset();
         while(NULL!=(DataPtr=DataList.GetNext()))
         {
           if(*DataPtr=='$')
             strcpy(Names+I*3*NM,DataPtr+1);
           else if(J >= 0  && J <= 4)
           {
             if(SizeValue==sizeof(DWORD))
               *(DWORD*)(Param+PARAM_POS(I)+J*SizeValue)=atoi(DataPtr);
             else
               *(__int64*)(Param+PARAM_POS(I)+J*SizeValue)=_atoi64(DataPtr);
           }
           ++J;
         }
      }
      /* IS $ */
    }
  }
  return TRUE;
}


BOOL FilePositionCache::Save(const char *Key)
{
  if(!IsMemory)
    return FALSE;

  char DataStr[2048];
  int J, I, Pos;

  for (I=0;I < Opt.MaxPositionCache;I++)
  {
    itoa(I,PtrSubKeyItem,10);
    itoa(I,PtrSubKeyShort,10);

    if ((Pos=CurPos+I)>=Opt.MaxPositionCache)
      Pos-=Opt.MaxPositionCache;
    /* $ 17.06.2001 IS
       + Имя файла должно быть взято в кавычки, т.к. оно может содержать
         символы-разделители
    */
    DWORD   *Ptr32=(DWORD*)(Param+PARAM_POS(Pos));
    __int64 *Ptr64=(__int64 *)(Param+PARAM_POS(Pos));

    if(SizeValue==sizeof(DWORD))
      sprintf(DataStr,"%d,%d,%d,%d,%d,\"$%s\"",
            Ptr32[0],Ptr32[1],Ptr32[2],Ptr32[3],Ptr32[4],Names+Pos*3*NM);
    else
    {
      sprintf(DataStr,"%I64d,%I64d,%I64d,%I64d,%I64d,\"$%s\"",
            Ptr64[0],Ptr64[1],Ptr64[2],Ptr64[3],Ptr64[4],Names+Pos*3*NM);
    }
    /* IS $ */

    // ????????
    if(!strcmp(DataStr,EmptyPos))
    {
      DeleteRegValue(Key,SubKeyItem);
      continue;
    }
    // ????????

    SetRegKey(Key,SubKeyItem,DataStr);
    if((Opt.SaveViewerShortPos && Opt.SaveViewerPos) ||
       (Opt.EdOpt.SaveShortPos && Opt.EdOpt.SavePos))
    {
      // Если не запоминались позиции по RCtrl+<N>, то и не записываем их
      for(J=0; J < (BOOKMARK_COUNT*4); J++)
      {
        if(SizeValue==sizeof(DWORD))
        {
          if(*(DWORD*)(Position+POSITION_POS(Pos,J)) != -1)
            break;
        }
        else
        {
          if(*(__int64*)(Position+POSITION_POS(Pos,J)) != -1)
            break;
        }
      }

      if(J < (BOOKMARK_COUNT*4))
        SetRegKey(Key,SubKeyShort,Position+POSITION_POS(Pos,0),(BOOKMARK_COUNT*4)*SizeValue);
      else
        DeleteRegValue(Key,SubKeyShort);
    }
  }
  return TRUE;
}
