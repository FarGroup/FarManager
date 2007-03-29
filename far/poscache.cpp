/*
poscache.cpp

Кэш позиций в файлах для viewer/editor

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
static wchar_t EmptyPos[]=L"0,0,0,0,0,\"$\"";
/* IS $ */

/*char FilePositionCache::SubKeyItem[16]="Item";
char FilePositionCache::SubKeyShort[16]="Short";
char *FilePositionCache::PtrSubKeyItem;
char *FilePositionCache::PtrSubKeyShort;*/


FilePositionCache::FilePositionCache(int TypeCache)
{
//  PtrSubKeyItem=SubKeyItem+strlen(SubKeyItem);
//  PtrSubKeyShort=SubKeyShort+strlen(SubKeyShort);

  if(!Opt.MaxPositionCache)
  {
    GetRegKey(L"System",L"MaxPositionCache",Opt.MaxPositionCache,MAX_POSITIONS);
    if(Opt.MaxPositionCache < 16 || Opt.MaxPositionCache > 128)
      Opt.MaxPositionCache=MAX_POSITIONS;
  }

  SizeValue=!TypeCache?sizeof(DWORD):sizeof(__int64);

  Param=NULL;
  Position=NULL;
  IsMemory=0;
  CurPos=0;

  if((Names=(wchar_t*)xf_malloc(Opt.MaxPositionCache*3*NM*sizeof(wchar_t))) != NULL) //BUGBUG!!! NM
  {
    Param=(BYTE*)xf_malloc(MSIZE_PARAM);
    Position=(BYTE*)xf_malloc(MSIZE_POSITION);
    if(Param && Position)
    {
      memset(Names,0,Opt.MaxPositionCache*3*NM*sizeof (wchar_t));
      memset(Param,0,MSIZE_PARAM);
      memset(Position,0xFF,MSIZE_POSITION);
      IsMemory=1;
    }
    else
    {
      if(Param)       xf_free(Param);       Param=NULL;
      if(Position)    xf_free(Position);    Position=NULL;
    }
  }
}

FilePositionCache::~FilePositionCache()
{
  if(Names)     xf_free(Names);
  if(Param)     xf_free(Param);
  if(Position)  xf_free(Position);
}

void FilePositionCache::AddPosition(const wchar_t *Name,void *PosCache)
{
  if(!IsMemory || !PosCache)
    return;

  string strFullName;

  if (*Name==L'<')
    strFullName = Name;
  else
    ConvertNameToFullW(Name,strFullName);
  /* $ 06.04.2001 VVM
    - Неправильное позиционирование в открытых файлах
      Имена копировал до поиска, а не после :) */
  int Pos = FindPosition(strFullName);
  if (Pos >= 0)
    CurPos = Pos;

  wcscpy(Names+CurPos*3*NM,strFullName);
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



BOOL FilePositionCache::GetPosition(const wchar_t *Name,void *PosCache)
{
  if(!IsMemory || !PosCache)
    return FALSE;

  string strFullName;

  if (*Name==L'<')
    strFullName = Name;
  else
    ConvertNameToFullW(Name, strFullName);

  int Pos = FindPosition(strFullName);

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

int FilePositionCache::FindPosition(const wchar_t *FullName)
{
  for (int I=1;I<=Opt.MaxPositionCache;I++)
  {
    int Pos=CurPos-I;
    if (Pos<0)
      Pos+=Opt.MaxPositionCache;

    int CmpRes;
    wchar_t *Ptr=Names+Pos*3*NM;

    if (Opt.FlagPosixSemantics)
      CmpRes = wcscmp(Ptr,FullName);
    else
      CmpRes = LocalStricmpW(Ptr,FullName);

    if (CmpRes == 0)
      return(Pos);
  }
  return(-1);
}

BOOL FilePositionCache::Read(const wchar_t *Key)
{
  if(!IsMemory)
    return FALSE;

  string strItem;
  string strShort;

  string strDataStr;
  BYTE DefPos[8096];

  memset(DefPos,0xff,(BOOKMARK_COUNT*4)*SizeValue);

  for (int I=0;I < Opt.MaxPositionCache;I++)
  {
    strItem.Format (L"Item%d", I);
    strShort.Format (L"Short%d", I);

    GetRegKey(Key,strShort,(LPBYTE)Position+POSITION_POS(I,0),(LPBYTE)DefPos,(BOOKMARK_COUNT*4)*SizeValue);
    GetRegKey(Key,strItem,strDataStr,EmptyPos);

    if(!wcscmp(strDataStr,EmptyPos))
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
      const wchar_t *DataPtr;
      string strArgData;

      if(DataList.Set(strDataStr))
      {
         DataList.Reset();
         while(NULL!=(DataPtr=DataList.GetNext()))
         {
           if(*DataPtr==L'$')
             wcscpy(Names+I*3*NM,DataPtr+1);
           else if(J >= 0  && J <= 4)
           {
             if(SizeValue==sizeof(DWORD))
               *(DWORD*)(Param+PARAM_POS(I)+J*SizeValue)=_wtoi(DataPtr);
             else
               *(__int64*)(Param+PARAM_POS(I)+J*SizeValue)=_wtoi64(DataPtr);
           }
           ++J;
         }
      }
      /* IS $ */
    }
  }
  return TRUE;
}


BOOL FilePositionCache::Save(const wchar_t *Key)
{
  if(!IsMemory)
    return FALSE;

  string strDataStr;
  int J, I, Pos;

  string strItem;
  string strShort;

  for (I=0;I < Opt.MaxPositionCache;I++)
  {
    strItem.Format (L"Item%d", I);
    strShort.Format (L"Short%d", I);

    if ((Pos=CurPos+I)>=Opt.MaxPositionCache)
      Pos-=Opt.MaxPositionCache;
    /* $ 17.06.2001 IS
       + Имя файла должно быть взято в кавычки, т.к. оно может содержать
         символы-разделители
    */
    DWORD   *Ptr32=(DWORD*)(Param+PARAM_POS(Pos));
    __int64 *Ptr64=(__int64 *)(Param+PARAM_POS(Pos));

    if(SizeValue==sizeof(DWORD))
      strDataStr.Format (L"%d,%d,%d,%d,%d,\"$%s\"",
            Ptr32[0],Ptr32[1],Ptr32[2],Ptr32[3],Ptr32[4],Names+Pos*3*NM);
    else
    {
      strDataStr.Format (L"%I64d,%I64d,%I64d,%I64d,%I64d,\"$%s\"",
            Ptr64[0],Ptr64[1],Ptr64[2],Ptr64[3],Ptr64[4],Names+Pos*3*NM);
    }
    /* IS $ */

    // ????????
    if(!wcscmp(strDataStr,EmptyPos))
    {
      DeleteRegValue(Key,strItem);
      continue;
    }
    // ????????

    SetRegKey(Key,strItem,strDataStr);
    if((Opt.ViOpt.SaveViewerShortPos && Opt.ViOpt.SaveViewerPos) ||
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
        SetRegKey(Key,strShort,Position+POSITION_POS(Pos,0),(BOOKMARK_COUNT*4)*SizeValue);
      else
        DeleteRegValue(Key,strShort);
    }
  }
  return TRUE;
}
