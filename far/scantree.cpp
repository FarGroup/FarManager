/*
scantree.cpp

Сканирование текущего каталога и, опционально, подкаталогов на
предмет имен файлов

*/

/* Revision: 1.08 01.06.2003 $ */

/*
Modify:
  01.06.2003 SVS
    ! переходим на BitFlags
  06.03.2003 SVS
    + Opt.ScanJunction - сканировать так же симлинки.
  27.12.2002 VVM
    + Новый параметр ScanFlags. Разные флаги. Пока что только один SF_FILES_FIRST.
      Это параметр по умолчанию устанавливается в функции SetFindPath, если не задано братное.
      Смысл в том, что для сканирования дерева нам не нужны два прохода. Я думаю, что
      потери в скорости были именно здесь.
  23.06.2002 SVS
    ! ннбольшая оптимизация кода
  26.03.2002 DJ
    ! GetNextName() принимает размер буфера для имени файла
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

ScanTree::ScanTree(int RetUpDir,int Recurse, int ScanJunction)
{
  Flags.Change(FSCANTREE_RETUPDIR,RetUpDir);
  Flags.Change(FSCANTREE_RECUR,Recurse);
  Flags.Change(FSCANTREE_SCANJUNCTION,(ScanJunction==-1?Opt.ScanJunction:ScanJunction));
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
  memset(SecondPass,0,sizeof(SecondPass));
  Flags.Clear(FSCANTREE_FILESFIRST);
}


void ScanTree::SetFindPath(const char *Path,const char *Mask, const DWORD NewScanFlags)
{
  Init();
  strncpy(FindMask,Mask,sizeof(FindMask)-1);
  strncpy(FindPath,Path,sizeof(FindPath)-1);
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
    if (FindHandle[FindHandleCount]==0)
      Done=((FindHandle[FindHandleCount]=FindFirstFile(FindPath,fdata))==INVALID_HANDLE_VALUE);
    else
      Done=!FindNextFile(FindHandle[FindHandleCount],fdata);

    if (Flags.Check(FSCANTREE_FILESFIRST))
    {
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
    } /* if */

    char *FileName=fdata->cFileName;
    if (Done || !(*FileName=='.' && (!FileName[1] || FileName[1]=='.' && !FileName[2])))
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

      if (Flags.Check(FSCANTREE_RETUPDIR))
      {
        HANDLE UpHandle;
        strcpy(FullName,FindPath);
        UpHandle=FindFirstFile(FullName,fdata);
        FindClose(UpHandle);
      }

      if ((ChPtr=strrchr(FindPath,'\\'))!=NULL)
        *(ChPtr+1)=0;

      strcat(FindPath,FindMask);

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
       Если каталог является SymLink (т.н. "Directory Junctions"),
       то в него не ломимся.
    */
    if (Flags.Check(FSCANTREE_RECUR) &&
      ((fdata->dwFileAttributes & (FA_DIREC|FILE_ATTRIBUTE_REPARSE_POINT)) == FA_DIREC ||
          (fdata->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && Flags.Check(FSCANTREE_SCANJUNCTION)))
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
  }

  /* $ 26.03.2002 DJ
     если имя слишком длинное - пропустим и вернем следующее
  */
  if (strlen (FindPath) < BufSize)
  {
    strcpy(FullName,FindPath);

    if ((ChPtr=strrchr(FullName,'\\'))!=NULL)
      *(ChPtr+1)=0;

    if (strlen (FullName) + strlen (fdata->cFileName) < BufSize)
    {
      strcat (FullName, fdata->cFileName);
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
