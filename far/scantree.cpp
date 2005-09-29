/*
scantree.cpp

Сканирование текущего каталога и, опционально, подкаталогов на
предмет имен файлов

*/

/* Revision: 1.11 29.09.2005 $ */

/*
Modify:
  29.09.2005 SVS
    + FSCANTREE_USEDALTFOLDERNAME
    + доп.параметр у конструктора ScanTree()
    ! если выставлен Opt.FolderDeepScan и есть флаг FSCANTREE_USEDALTFOLDERNAME и
      GetFileAttributes вернул ошибку по длинному имени... работаем с коротким (для каталогов)
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  14.06.2003 SVS
    ! Внедрение новых флагов
    ! Вместо SecondPass[] и FindHandle[] вводим структуру ScanTreeData
    + InsideJunction() - при очередном проходе скажет нам - "мы в симлинке?"
    ! FSCANTREE_SCANJUNCTION -> FSCANTREE_SCANSYMLINK
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
      FindClose(Data[I].FindHandle);
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
      Done=((Data[FindHandleCount].FindHandle=FindFirstFile(FindPath,fdata))==INVALID_HANDLE_VALUE);
    else
      Done=!FindNextFile(Data[FindHandleCount].FindHandle,fdata);

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

    char *FileName=fdata->cFileName;
    if (Done || !(*FileName=='.' && (!FileName[1] || FileName[1]=='.' && !FileName[2])))
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
       Если каталог является SymLink (т.н. "Directory Junctions"),
       то в него не ломимся.
    */
    if (Flags.Check(FSCANTREE_RECUR) &&
      ((fdata->dwFileAttributes & (FA_DIREC|FILE_ATTRIBUTE_REPARSE_POINT)) == FA_DIREC ||
          (fdata->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && Flags.Check(FSCANTREE_SCANSYMLINK)))
    /* SVS $ */
    {
      if ((ChPtr=strrchr(FindPath,'\\'))!=NULL)
        *(ChPtr+1)=0;
      if(Opt.FolderDeepScan && Flags.Check(FSCANTREE_USEDALTFOLDERNAME))
        strcat(FindPath,GetFileAttributes(fdata->cFileName)==(DWORD)-1 && *fdata->cAlternateFileName?fdata->cAlternateFileName:fdata->cFileName);
      else
        strcat(FindPath,fdata->cFileName);
      strcpy(FullName,FindPath);
      strcat(FindPath,"\\");
      strcat(FindPath,FindMask);

      _SVS(SysLog("2. FullName='%s'",FullName));
      if (strlen(FindPath)>NM)
        return(FALSE);

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
    FindClose(Handle);

  Data[FindHandleCount--].FindHandle=0;
  if(!Data[FindHandleCount].Flags.Check(FSCANTREE_INSIDEJUNCTION))
    Flags.Clear(FSCANTREE_INSIDEJUNCTION);

  if ((ChPtr=strrchr(FindPath,'\\'))!=NULL)
    *ChPtr=0;

  if ((ChPtr=strrchr(FindPath,'\\'))!=NULL)
    *(ChPtr+1)=0;

  strcat(FindPath,FindMask);
}
