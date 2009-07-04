/*
scantree.cpp

—канирование текущего каталога и, опционально, подкаталогов на
предмет имен файлов

*/

#include "headers.hpp"
#pragma hdrstop

#include "scantree.hpp"
#include "fn.hpp"

ScanTree::ScanTree(int RetUpDir,int Recurse, int ScanJunction,int UsedAltFolderName)
{
  Flags.Change(FSCANTREE_RETUPDIR,RetUpDir);
  Flags.Change(FSCANTREE_RECUR,Recurse);
  Flags.Change(FSCANTREE_SCANSYMLINK,(ScanJunction==-1?Opt.ScanJunction:ScanJunction));
  Flags.Change(FSCANTREE_USEDALTFOLDERNAME,UsedAltFolderName);
  Init();
}


ScanTree::~ScanTree()
{
  for (int I=FindHandleCount;I>=0;I--)
    if (Data[I].FindHandle && Data[I].FindHandle!=INVALID_HANDLE_VALUE)
      FAR_FindClose(Data[I].FindHandle);
}



void ScanTree::Init()
{
  memset(Data,0,sizeof(Data));
  FindHandleCount=0;
  Flags.Clear(FSCANTREE_FILESFIRST);
}


void ScanTree::SetFindPath(const char *Path,const char *Mask, const DWORD NewScanFlags)
{
  Init();
  xstrncpy(FindMask,Mask,sizeof(FindMask)-1);
  xstrncpy(FindPath,Path,sizeof(FindPath)-1);
  AddEndSlash(FindPath);
  strcat(FindPath,FindMask);
  Flags.Flags=(Flags.Flags&0x0000FFFF)|(NewScanFlags&0xFFFF0000);
}


int ScanTree::GetNextName(WIN32_FIND_DATA *fdata,char *FullName, size_t BufSize)
{
  int Done;
  char *ChPtr;
  Flags.Clear(FSCANTREE_SECONDDIRNAME);
  while (1)
  {
    if (Data[FindHandleCount].FindHandle==0)
      Done=((Data[FindHandleCount].FindHandle=FAR_FindFirstFile(FindPath,fdata))==INVALID_HANDLE_VALUE);
    else
      Done=!FAR_FindNextFile(Data[FindHandleCount].FindHandle,fdata);

    if (Flags.Check(FSCANTREE_FILESFIRST))
    {
      if (Data[FindHandleCount].Flags.Check(FSCANTREE_SECONDPASS))
      {
        if (!Done && (fdata->dwFileAttributes & FA_DIREC)==0)
          continue;
      }
      else
      {
        if (!Done && (fdata->dwFileAttributes & FA_DIREC))
          continue;
        if (Done)
        {
          if(!(Data[FindHandleCount].FindHandle == INVALID_HANDLE_VALUE || !Data[FindHandleCount].FindHandle))
            FAR_FindClose(Data[FindHandleCount].FindHandle);
          Data[FindHandleCount].FindHandle=0;
          Data[FindHandleCount].Flags.Set(FSCANTREE_SECONDPASS);
          continue;
        }
      }
    } /* if */

    char *FileName=fdata->cFileName;
    if (Done || !(*FileName=='.' && (!FileName[1] || FileName[1]=='.' && !FileName[2])))
      break;
  }

  // предвычисление длины строки - пр€мое использование xstrncpy может приводить
  // к "странным" результатам, а без проверки происходило переполнение при
  // слишком длинных именах (поученных через subst).
  const char *pm = fdata->cFileName;
  {
    int LenTempFindPath=(int)strlen(FindPath)+(int)strlen(pm)+8;
    char *TempFindPath=(char *)alloca(LenTempFindPath);
    strcpy(TempFindPath,FindPath);
    AddEndSlash(TempFindPath);
    strcat(TempFindPath,pm);

    if(Opt.FolderDeepScan && Flags.Check(FSCANTREE_USEDALTFOLDERNAME) &&
       GetFileAttributes(TempFindPath)==(DWORD)-1 && *fdata->cAlternateFileName)
      pm = fdata->cAlternateFileName;
  }

  if(strlen(FindPath)+strlen(pm)-strlen(FindMask)>=NM)
  {
    _SVS(SysLog("2! FullName EXCEED(%s%s\\%s)",FindPath,pm,FindMask));
    Done=TRUE;
  }

  if (Done)
  {
    if (Data[FindHandleCount].FindHandle!=INVALID_HANDLE_VALUE)
    {
      FAR_FindClose(Data[FindHandleCount].FindHandle);
      Data[FindHandleCount].FindHandle=0;
    }

    if (FindHandleCount==0)
      return(FALSE);
    else
    {
      Data[FindHandleCount--].FindHandle=0;
      if(!Data[FindHandleCount].Flags.Check(FSCANTREE_INSIDEJUNCTION))
        Flags.Clear(FSCANTREE_INSIDEJUNCTION);

      if ((ChPtr=strrchr(FindPath,'\\'))!=NULL)
        *ChPtr=0;

      if (Flags.Check(FSCANTREE_RETUPDIR))
      {
        strcpy(FullName,FindPath);
        GetFileWin32FindData(FullName,fdata);
      }

      if ((ChPtr=strrchr(FindPath,'\\'))!=NULL)
        *(ChPtr+1)=0;

      strcat(FindPath,FindMask);

      _SVS(SysLog("1. FullName='%s'",FullName));
      if (Flags.Check(FSCANTREE_RETUPDIR))
      {
        Flags.Set(FSCANTREE_SECONDDIRNAME);
        return(TRUE);
      }
      return(GetNextName(fdata,FullName, BufSize));
    }
  }
  else
  {
    /* $ 28.11.2000 SVS
       ≈сли каталог €вл€етс€ SymLink (т.н. "Directory Junctions"),
       то в него не ломимс€.
    */
    if (Flags.Check(FSCANTREE_RECUR) &&
      ((fdata->dwFileAttributes & (FA_DIREC|FILE_ATTRIBUTE_REPARSE_POINT)) == FA_DIREC ||
          ((fdata->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && fdata->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) && Flags.Check(FSCANTREE_SCANSYMLINK))))
    /* SVS $ */
    {
      if ((ChPtr=strrchr(FindPath,'\\'))!=NULL)
        *(ChPtr+1)=0;

      strcat(FindPath,pm);
      strcpy(FullName,FindPath);
      strcat(FindPath,"\\");
      strcat(FindPath,FindMask);

      _SVS(SysLog("2. FullName='%s'",FullName));

      Data[++FindHandleCount].FindHandle=0;
      Data[FindHandleCount].Flags=Data[FindHandleCount-1].Flags; // наследуем флаг
      Data[FindHandleCount].Flags.Clear(FSCANTREE_SECONDPASS);
      if(fdata->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && fdata->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
      {
        Data[FindHandleCount].Flags.Set(FSCANTREE_INSIDEJUNCTION);
        Flags.Set(FSCANTREE_INSIDEJUNCTION);
      }
      return(TRUE);
    }
  }

  /* $ 26.03.2002 DJ
     если им€ слишком длинное - пропустим и вернем следующее
  */
  if (strlen (FindPath) < BufSize)
  {
    strcpy(FullName,FindPath);

    if ((ChPtr=strrchr(FullName,'\\'))!=NULL)
      *(ChPtr+1)=0;

    if (strlen (FullName) + strlen (fdata->cFileName) < BufSize)
    {
      strcat (FullName, fdata->cFileName);
      _SVS(SysLog("3. FullName='%s'",FullName));
      return TRUE;
    }
  }
  return GetNextName (fdata, FullName, BufSize);
  /* DJ $ */
}


void ScanTree::SkipDir()
{
  char *ChPtr;

  if (FindHandleCount==0)
    return;

  HANDLE Handle=Data[FindHandleCount].FindHandle;
  if (Handle!=INVALID_HANDLE_VALUE && Handle!=0)
    FAR_FindClose(Handle);

  Data[FindHandleCount--].FindHandle=0;
  if(!Data[FindHandleCount].Flags.Check(FSCANTREE_INSIDEJUNCTION))
    Flags.Clear(FSCANTREE_INSIDEJUNCTION);

  if ((ChPtr=strrchr(FindPath,'\\'))!=NULL)
    *ChPtr=0;

  if ((ChPtr=strrchr(FindPath,'\\'))!=NULL)
    *(ChPtr+1)=0;

  strcat(FindPath,FindMask);
}
