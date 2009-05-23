/*
poscache.cpp

Кэш позиций в файлах для viewer/editor
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "poscache.hpp"
#include "fn.hpp"
#include "udlist.hpp"
#include "registry.hpp"
#include "config.hpp"

#define MSIZE_PARAM            (Opt.MaxPositionCache*SizeValue*5)
#define MSIZE_POSITION         (BOOKMARK_COUNT*Opt.MaxPositionCache*SizeValue*4)

#define PARAM_POS(Pos)         ((Pos)*SizeValue*5)
#define POSITION_POS(Pos,Idx)  ((Pos)*BOOKMARK_COUNT*SizeValue*4+(Idx)*BOOKMARK_COUNT*SizeValue)

static wchar_t EmptyPos[]=L"0,0,0,0,0,\"$\"";

FilePositionCache::FilePositionCache(int TypeCache)
{
  if (!Opt.MaxPositionCache)
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

  Names=new string[Opt.MaxPositionCache];
  if (Names != NULL)
  {
    Param=(BYTE*)xf_malloc(MSIZE_PARAM);
    Position=(BYTE*)xf_malloc(MSIZE_POSITION);
    if (Param && Position)
    {
      memset(Param,0,MSIZE_PARAM);
      memset(Position,0xFF,MSIZE_POSITION);
      IsMemory=1;
    }
    else
    {
      if (Param)       { xf_free(Param);       Param=NULL; }
      if (Position)    { xf_free(Position);    Position=NULL; }
    }
  }
}

FilePositionCache::~FilePositionCache()
{
  if (Names)     delete[] Names;
  if (Param)     xf_free(Param);
  if (Position)  xf_free(Position);
}

void FilePositionCache::AddPosition(const wchar_t *Name,void *PosCache)
{
  if (!IsMemory || !PosCache)
    return;

  string strFullName;

  if (*Name==L'<')
    strFullName = Name;
  else
    ConvertNameToFull(Name,strFullName);

  int FoundPos, Pos;

  Pos = FoundPos = FindPosition(strFullName);
  if (Pos < 0)
    Pos = CurPos;

  Names[Pos] = strFullName;

  int I;

  memset(Position+POSITION_POS(Pos,0),0xFF,(BOOKMARK_COUNT*4)*SizeValue);
  memcpy(Param+PARAM_POS(Pos),PosCache,SizeValue*5); // При условии, что в TPosCache?? Param стоит первым :-)

  if (SizeValue == sizeof(DWORD))
  {
    for (I=0; I < 4; ++I)
      if (((struct TPosCache32*)PosCache)->Position[I])
        memcpy(Position+POSITION_POS(Pos,I),((struct TPosCache32*)PosCache)->Position[I],BOOKMARK_COUNT*SizeValue);
  }
  else
  {
    for (I=0; I < 4; ++I)
      if (((struct TPosCache64*)PosCache)->Position[I])
        memcpy(Position+POSITION_POS(Pos,I),((struct TPosCache64*)PosCache)->Position[I],BOOKMARK_COUNT*SizeValue);
  }

  if (FoundPos < 0)
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
    ConvertNameToFull(Name, strFullName);

  int Pos = FindPosition(strFullName);

  //memset(Position+POSITION_POS(CurPos,0),0xFF,(BOOKMARK_COUNT*4)*SizeValue);
  //memcpy(Param+PARAM_POS(CurPos),PosCache,SizeValue*5); // При условии, что в TPosCache?? Param стоит первым :-)

  if (Pos >= 0)
  {
    int I;
    memcpy(PosCache,Param+PARAM_POS(Pos),SizeValue*5); // При условии, что в TPosCache?? Param стоит первым :-)
    if (SizeValue == sizeof(DWORD))
    {
      for (I=0; I < 4; ++I)
        if (((struct TPosCache32*)PosCache)->Position[I])
          memcpy(((struct TPosCache32*)PosCache)->Position[I],Position+POSITION_POS(Pos,I),BOOKMARK_COUNT*SizeValue);
    }
    else
    {
      for (I=0; I < 4; ++I)
        if (((struct TPosCache64*)PosCache)->Position[I])
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

    if (Opt.FlagPosixSemantics)
      CmpRes = StrCmp(Names[Pos],FullName);
    else
      CmpRes = StrCmpI(Names[Pos],FullName);

    if (CmpRes == 0)
      return(Pos);
  }
  return(-1);
}

BOOL FilePositionCache::Read(const wchar_t *Key)
{
  if (!IsMemory)
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

    if (!StrCmp(strDataStr,EmptyPos))
    {
      Names[I].SetLength(0);
      memset(Param+PARAM_POS(I),0,SizeValue*5);
    }
    else
    {
      UserDefinedList DataList(0,0,0);

      int J=0;
      const wchar_t *DataPtr;
      string strArgData;

      if (DataList.Set(strDataStr))
      {
         DataList.Reset();
         while (NULL!=(DataPtr=DataList.GetNext()))
         {
           if (*DataPtr==L'$')
           {
             Names[I] = (DataPtr+1);
           }
           else if (J >= 0 && J <= 4)
           {
             if (SizeValue==sizeof(DWORD))
               *(DWORD*)(Param+PARAM_POS(I)+J*SizeValue)=_wtoi(DataPtr);
             else
               *(__int64*)(Param+PARAM_POS(I)+J*SizeValue)=_wtoi64(DataPtr);
           }
           ++J;
         }
      }
    }
  }
  return TRUE;
}


BOOL FilePositionCache::Save(const wchar_t *Key)
{
  if (!IsMemory)
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

    DWORD   *Ptr32=(DWORD*)(Param+PARAM_POS(Pos));
    __int64 *Ptr64=(__int64 *)(Param+PARAM_POS(Pos));

    //Имя файла должно быть взято в кавычки, т.к. оно может содержать символы-разделители
    if (SizeValue==sizeof(DWORD))
    {
      strDataStr.Format (L"%d,%d,%d,%d,%d,\"$%s\"",
            Ptr32[0],Ptr32[1],Ptr32[2],Ptr32[3],Ptr32[4],(const wchar_t *)Names[Pos]);
    }
    else
    {
      strDataStr.Format (L"%I64d,%I64d,%I64d,%I64d,%I64d,\"$%s\"",
            Ptr64[0],Ptr64[1],Ptr64[2],Ptr64[3],Ptr64[4],(const wchar_t *)Names[Pos]);
    }

    //Пустая позиция?
    if (!StrCmp(strDataStr,EmptyPos))
    {
      DeleteRegValue(Key,strItem);
      continue;
    }

    SetRegKey(Key,strItem,strDataStr);
    if ((Opt.ViOpt.SaveViewerShortPos && Opt.ViOpt.SaveViewerPos) ||
        (Opt.EdOpt.SaveShortPos && Opt.EdOpt.SavePos))
    {
      // Если не запоминались позиции по RCtrl+<N>, то и не записываем их
      for (J=0; J < 4; J++)
      {
        if (SizeValue==sizeof(DWORD))
        {
          if (*(DWORD*)(Position+POSITION_POS(Pos,J)) != (DWORD)-1)
            break;
        }
        else
        {
          if (*(__int64*)(Position+POSITION_POS(Pos,J)) != -1)
            break;
        }
      }

      if (J < 4)
        SetRegKey(Key,strShort,Position+POSITION_POS(Pos,0),(BOOKMARK_COUNT*4)*SizeValue);
      else
        DeleteRegValue(Key,strShort);
    }
  }
  return TRUE;
}
