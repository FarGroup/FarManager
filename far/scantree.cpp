/*
scantree.cpp

Сканирование текущего каталога и, опционально, подкаталогов на
предмет имен файлов
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

#include "scantree.hpp"
#include "fn.hpp"

#include "syslog.hpp"

ScanTree::ScanTree(int RetUpDir,int Recurse, int ScanJunction)
{
  Flags.Change(FSCANTREE_RETUPDIR,RetUpDir);
  Flags.Change(FSCANTREE_RECUR,Recurse);
  Flags.Change(FSCANTREE_SCANSYMLINK,(ScanJunction==-1?Opt.ScanJunction:ScanJunction));
	DataCount=MAX_PATH/2;
	Data=static_cast<ScanTreeData*>(xf_malloc(sizeof(ScanTreeData)*DataCount));
  Init();
}


ScanTree::~ScanTree()
{
	for(size_t i=0;i<FindHandleCount;i++)
		if(Data[i].FindHandle && Data[i].FindHandle!=INVALID_HANDLE_VALUE)
			apiFindClose(Data[i].FindHandle);
	xf_free(Data);
}



void ScanTree::Init()
{
	memset(Data,0,sizeof(ScanTreeData)*DataCount);
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
        if (!Done && (fdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
          continue;
      }
      else
      {
        if (!Done && (fdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
          continue;
        if (Done)
        {
          if(!(Data[FindHandleCount].FindHandle == INVALID_HANDLE_VALUE || !Data[FindHandleCount].FindHandle))
            apiFindClose(Data[FindHandleCount].FindHandle);
          Data[FindHandleCount].FindHandle=0;
          Data[FindHandleCount].Flags.Set(FSCANTREE_SECONDPASS);
          continue;
        }
      }
    } /* if */

    const wchar_t *FileName=fdata->strFileName;
    if (Done || !(*FileName==L'.' && (!FileName[1] || (FileName[1]==L'.' && !FileName[2]))))
      break;
  }

  if (Done)
  {
    if (Data[FindHandleCount].FindHandle!=INVALID_HANDLE_VALUE)
    {
      apiFindClose(Data[FindHandleCount].FindHandle);
      Data[FindHandleCount].FindHandle=0;
    }

    if (FindHandleCount==0)
      return(FALSE);
    else
    {
      Data[FindHandleCount--].FindHandle=0;
      if(!Data[FindHandleCount].Flags.Check(FSCANTREE_INSIDEJUNCTION))
        Flags.Clear(FSCANTREE_INSIDEJUNCTION);

      CutToSlash(strFindPath,true);

      if (Flags.Check(FSCANTREE_RETUPDIR))
      {
        strFullName = strFindPath;
        apiGetFindDataEx (strFindPath, fdata);
      }

      CutToSlash(strFindPath);

      strFindPath += strFindMask;

      _SVS(SysLog(L"1. FullName='%s'",(const wchar_t*)strFullName));
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
    /*
       Если каталог является SymLink (т.н. "Directory Junctions"),
       то в него не ломимся.
    */
    if (Flags.Check(FSCANTREE_RECUR) &&
      ((fdata->dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT)) == FILE_ATTRIBUTE_DIRECTORY ||
          ((fdata->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && fdata->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) && Flags.Check(FSCANTREE_SCANSYMLINK))))
    {
      CutToSlash(strFindPath);

      strFindPath += fdata->strFileName;

      strFullName = strFindPath;

      strFindPath += L"\\";
      strFindPath += strFindMask;

			FindHandleCount++;

			if(FindHandleCount>=DataCount)
			{
				DataCount<<=1;
				Data=static_cast<ScanTreeData*>(xf_realloc(Data,sizeof(ScanTreeData)*DataCount));
			}

			Data[FindHandleCount].FindHandle=0;
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

  strFullName = strFindPath;

  CutToSlash(strFullName);

  strFullName += fdata->strFileName;
  return TRUE;
}

void ScanTree::SkipDir()
{
  if (FindHandleCount==0)
    return;

  HANDLE Handle=Data[FindHandleCount].FindHandle;
  if (Handle!=INVALID_HANDLE_VALUE && Handle!=0)
    apiFindClose(Handle);

  Data[FindHandleCount--].FindHandle=0;
  if(!Data[FindHandleCount].Flags.Check(FSCANTREE_INSIDEJUNCTION))
    Flags.Clear(FSCANTREE_INSIDEJUNCTION);

  CutToSlash(strFindPath,true);
  CutToSlash(strFindPath);

  strFindPath += strFindMask;
}
