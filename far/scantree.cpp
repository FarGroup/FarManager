/*
scantree.cpp

Сканирование текущего каталога и, опционально, подкаталогов на
предмет имен файлов

*/

#include "headers.hpp"
#pragma hdrstop

#include "scantree.hpp"
#include "fn.hpp"

ScanTree::ScanTree(int RetUpDir,int Recurse, int ScanJunction)
{
  Flags.Change(FSCANTREE_RETUPDIR,RetUpDir);
  Flags.Change(FSCANTREE_RECUR,Recurse);
  Flags.Change(FSCANTREE_SCANSYMLINK,(ScanJunction==-1?Opt.ScanJunction:ScanJunction));
  Init();
}


ScanTree::~ScanTree()
{
  for (int I=FindHandleCount;I>=0;I--)
    if (Data[I].FindHandle && Data[I].FindHandle!=INVALID_HANDLE_VALUE)
      FindClose(Data[I].FindHandle);
}



void ScanTree::Init()
{
  memset(Data,0,sizeof(Data));
  FindHandleCount=0;
  Flags.Clear(FSCANTREE_FILESFIRST);
}


void ScanTree::SetFindPath(const wchar_t *Path,const wchar_t *Mask, const DWORD NewScanFlags)
{
  Init();

  strFindMask = Mask;
  strFindPath = Path;

  AddEndSlash(strFindPath);

  strFindPath += strFindMask;

  Flags.Flags=(Flags.Flags&0x0000FFFF)|(NewScanFlags&0xFFFF0000);
}


int ScanTree::GetNextName(FAR_FIND_DATA_EX *fdata,string &strFullName)
{
  int Done;
  wchar_t *ChPtr;
  Flags.Clear(FSCANTREE_SECONDDIRNAME);
  while (1)
  {
    if (Data[FindHandleCount].FindHandle==0)
      Done=((Data[FindHandleCount].FindHandle=apiFindFirstFile(strFindPath,fdata))==INVALID_HANDLE_VALUE);
    else
      Done=!apiFindNextFile(Data[FindHandleCount].FindHandle,fdata);

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
          FindClose(Data[FindHandleCount].FindHandle);
          Data[FindHandleCount].FindHandle=0;
          Data[FindHandleCount].Flags.Set(FSCANTREE_SECONDPASS);
          continue;
        }
      }
    } /* if */

    const wchar_t *FileName=fdata->strFileName;
    if (Done || !(*FileName==L'.' && (!FileName[1] || FileName[1]==L'.' && !FileName[2])))
      break;
  }

  if (Done)
  {
    if (Data[FindHandleCount].FindHandle!=INVALID_HANDLE_VALUE)
    {
      FindClose(Data[FindHandleCount].FindHandle);
      Data[FindHandleCount].FindHandle=0;
    }

    if (FindHandleCount==0)
      return(FALSE);
    else
    {
      Data[FindHandleCount--].FindHandle=0;
      if(!Data[FindHandleCount].Flags.Check(FSCANTREE_INSIDEJUNCTION))
        Flags.Clear(FSCANTREE_INSIDEJUNCTION);

      ChPtr = strFindPath.GetBuffer ();

      if ((ChPtr=wcsrchr(ChPtr,L'\\'))!=NULL)
        *ChPtr=0;

      strFindPath.ReleaseBuffer ();

      if (Flags.Check(FSCANTREE_RETUPDIR))
      {
        strFullName = strFindPath;
        apiGetFindDataEx (strFindPath, fdata);
      }

      ChPtr = strFindPath.GetBuffer ();

      if ((ChPtr=wcsrchr(ChPtr,L'\\'))!=NULL)
        *(ChPtr+1)=0;

      strFindPath.ReleaseBuffer ();

      strFindPath += strFindMask;

      _SVS(SysLog(L"1. FullName='%S'",(const wchar_t*)strFullName));
      if (Flags.Check(FSCANTREE_RETUPDIR))
      {
        Flags.Set(FSCANTREE_SECONDDIRNAME);
        return(TRUE);
      }
      return(GetNextName(fdata,strFullName));
    }
  }
  else
  {
    /* $ 28.11.2000 SVS
       Если каталог является SymLink (т.н. "Directory Junctions"),
       то в него не ломимся.
    */
    if (Flags.Check(FSCANTREE_RECUR) &&
      ((fdata->dwFileAttributes & (FA_DIREC|FILE_ATTRIBUTE_REPARSE_POINT)) == FA_DIREC ||
          (fdata->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && Flags.Check(FSCANTREE_SCANSYMLINK)))
    /* SVS $ */
    {

      ChPtr = strFindPath.GetBuffer ();

      if ((ChPtr=wcsrchr(ChPtr,L'\\'))!=NULL)
        *(ChPtr+1)=0;

      strFindPath.ReleaseBuffer ();

      strFindPath += fdata->strFileName;

      strFullName = strFindPath;

      strFindPath += L"\\";
      strFindPath += strFindMask;

      Data[++FindHandleCount].FindHandle=0;
      Data[FindHandleCount].Flags=Data[FindHandleCount-1].Flags; // наследуем флаг
      Data[FindHandleCount].Flags.Clear(FSCANTREE_SECONDPASS);
      if(fdata->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
      {
        Data[FindHandleCount].Flags.Set(FSCANTREE_INSIDEJUNCTION);
        Flags.Set(FSCANTREE_INSIDEJUNCTION);
      }
      return(TRUE);
    }
  }

  /* $ 26.03.2002 DJ
     если имя слишком длинное - пропустим и вернем следующее
  */
  strFullName = strFindPath;

  ChPtr = strFullName.GetBuffer ();

  if ((ChPtr=wcsrchr(ChPtr,L'\\'))!=NULL)
    *(ChPtr+1)=0;

  strFullName.ReleaseBuffer ();

  strFullName += fdata->strFileName;
  return TRUE;
}

void ScanTree::SkipDir()
{
  wchar_t *ChPtr;

  if (FindHandleCount==0)
    return;

  HANDLE Handle=Data[FindHandleCount].FindHandle;
  if (Handle!=INVALID_HANDLE_VALUE && Handle!=0)
    FindClose(Handle);

  Data[FindHandleCount--].FindHandle=0;
  if(!Data[FindHandleCount].Flags.Check(FSCANTREE_INSIDEJUNCTION))
    Flags.Clear(FSCANTREE_INSIDEJUNCTION);

  ChPtr = strFindPath.GetBuffer ();

  if ((ChPtr=wcsrchr(ChPtr,L'\\'))!=NULL)
     *ChPtr=0;

  if ((ChPtr=wcsrchr(ChPtr,L'\\'))!=NULL)
     *(ChPtr+1)=0;

  strFindPath.ReleaseBuffer ();

  strFindPath += strFindMask;
}
