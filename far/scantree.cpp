/*
scantree.cpp

Сканирование текущего каталога и, опционально, подкаталогов на
предмет имен файлов

*/

/* Revision: 1.03 25.05.2001 $ */

/*
Modify:
  25.06.2001 IS
    ! Внедрение const
  06.05.2001 DJ
    ! перетрях #include
  28.11.2000 SVS
    + Если каталог является SymLink (т.н. "Directory Junctions"),
      то в него не ломимся.
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "scantree.hpp"
#include "fn.hpp"

ScanTree::ScanTree(int RetUpDir,int Recurse)
{
  ScanTree::RetUpDir=RetUpDir;
  ScanTree::Recurse=Recurse;
  Init();
}


ScanTree::~ScanTree()
{
  for (int I=FindHandleCount;I>=0;I--)
    if (FindHandle[I] && FindHandle[I]!=INVALID_HANDLE_VALUE)
      FindClose(FindHandle[I]);
}



void ScanTree::Init()
{
  memset(FindHandle,0,sizeof(FindHandle));
  FindHandleCount=0;
  SecondDirName=0;
  memset(SecondPass,0,sizeof(SecondPass));
}


void ScanTree::SetFindPath(const char *Path,const char *Mask)
{
  Init();
  strcpy(FindPath,Path);
  AddEndSlash(FindPath);
  strcpy(FindMask,Mask);
  strcat(FindPath,FindMask);
}


int ScanTree::GetNextName(WIN32_FIND_DATA *fdata,char *FullName)
{
  int Done;
  char *ChPtr;
  SecondDirName=0;
  while (1)
  {
    if (FindHandle[FindHandleCount]==0)
      Done=((FindHandle[FindHandleCount]=FindFirstFile(FindPath,fdata))==INVALID_HANDLE_VALUE);
    else
      Done=!FindNextFile(FindHandle[FindHandleCount],fdata);
    if (SecondPass[FindHandleCount])
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
        FindClose(FindHandle[FindHandleCount]);
        FindHandle[FindHandleCount]=0;
        SecondPass[FindHandleCount]=TRUE;
        continue;
      }
    }
    if (Done || strcmp(fdata->cFileName,"..")!=0 && strcmp(fdata->cFileName,".")!=0)
      break;
  }

  if (Done)
  {
    if (FindHandle[FindHandleCount]!=INVALID_HANDLE_VALUE)
    {
      FindClose(FindHandle[FindHandleCount]);
      FindHandle[FindHandleCount]=0;
    }
    if (FindHandleCount==0)
      return(FALSE);
    else
    {
      FindHandle[FindHandleCount--]=0;
      if ((ChPtr=strrchr(FindPath,'\\'))!=NULL)
        *ChPtr=0;
      if (RetUpDir)
      {
        HANDLE UpHandle;
        strcpy(FullName,FindPath);
        UpHandle=FindFirstFile(FullName,fdata);
        FindClose(UpHandle);
      }
      if ((ChPtr=strrchr(FindPath,'\\'))!=NULL)
        *(ChPtr+1)=0;
      strcat(FindPath,FindMask);
      if (RetUpDir)
      {
        SecondDirName=1;
        return(TRUE);
      }
      return(GetNextName(fdata,FullName));
    }
  }
  else
    /* $ 28.11.2000 SVS
       Если каталог является SymLink (т.н. "Directory Junctions"),
       то в него не ломимся.
    */
    if ((fdata->dwFileAttributes & FA_DIREC) &&
       !(fdata->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) &&
       Recurse)
    /* SVS $ */
    {
      if ((ChPtr=strrchr(FindPath,'\\'))!=NULL)
        *(ChPtr+1)=0;
      strcat(FindPath,fdata->cFileName);
      strcpy(FullName,FindPath);
      strcat(FindPath,"\\");
      strcat(FindPath,FindMask);
      if (strlen(FindPath)>NM)
        return(FALSE);
      FindHandle[++FindHandleCount]=0;
      SecondPass[FindHandleCount]=0;
      return(TRUE);
    }
  strcpy(FullName,FindPath);
  if ((ChPtr=strrchr(FullName,'\\'))!=NULL)
    *(ChPtr+1)=0;
  strcat(FullName,fdata->cFileName);
  return(TRUE);
}


void ScanTree::SkipDir()
{
  char *ChPtr;
  if (FindHandleCount==0)
    return;
  HANDLE Handle=FindHandle[FindHandleCount];
  if (Handle!=INVALID_HANDLE_VALUE && Handle!=0)
    FindClose(Handle);
  FindHandle[FindHandleCount--]=0;
  if ((ChPtr=strrchr(FindPath,'\\'))!=NULL)
    *ChPtr=0;
  if ((ChPtr=strrchr(FindPath,'\\'))!=NULL)
    *(ChPtr+1)=0;
  strcat(FindPath,FindMask);
}
