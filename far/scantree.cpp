/*
scantree.cpp

Сканирование текущего каталога и, опционально, подкаталогов на
предмет имен файлов
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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
#include "syslog.hpp"
#include "config.hpp"
#include "pathmix.hpp"

ScanTree::ScanTree(int RetUpDir,int Recurse, int ScanJunction)
{
	Flags.Change(FSCANTREE_RETUPDIR,RetUpDir);
	Flags.Change(FSCANTREE_RECUR,Recurse);
	Flags.Change(FSCANTREE_SCANSYMLINK,(ScanJunction==-1?(bool)Opt.ScanJunction:ScanJunction!=0));
	ScanItems.setDelta(10);
}

void ScanTree::SetFindPath(const wchar_t *Path,const wchar_t *Mask, const DWORD NewScanFlags)
{
	ScanItems.Free();
	ScanItems.addItem();
	Flags.Clear(FSCANTREE_FILESFIRST);
	strFindMask = Mask;
	strFindPath = Path;
	ConvertNameToReal(strFindPath, ScanItems.lastItem()->RealPath);
	AddEndSlash(strFindPath);
	strFindPath += strFindMask;
	Flags.Flags=(Flags.Flags&0x0000FFFF)|(NewScanFlags&0xFFFF0000);
}

bool ScanTree::GetNextName(FAR_FIND_DATA_EX *fdata,string &strFullName)
{
	if (!ScanItems.getCount())
		return false;

	bool Done=false;
	Flags.Clear(FSCANTREE_SECONDDIRNAME);

	for (;;)
	{
		ScanTreeData* LastItem = ScanItems.lastItem();
		if (!LastItem->Find)
		{
			LastItem->Find = new FindFile(strFindPath);
		}
		Done=!LastItem->Find->Get(*fdata);

		if (Flags.Check(FSCANTREE_FILESFIRST))
		{
			if (LastItem->Flags.Check(FSCANTREE_SECONDPASS))
			{
				if (!Done && !(fdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					continue;
			}
			else
			{
				if (!Done && (fdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					continue;

				if (Done)
				{
					if(LastItem->Find)
					{
						delete LastItem->Find;
						LastItem->Find = nullptr;
					}
					LastItem->Flags.Set(FSCANTREE_SECONDPASS);
					continue;
				}
			}
		}
		break;
	}

	if (Done)
	{
		ScanItems.deleteItem(ScanItems.getCount()-1);

		if (!ScanItems.getCount())
			return false;
		else
		{
			if (ScanItems.lastItem()->Flags.Check(FSCANTREE_INSIDEJUNCTION))
				Flags.Clear(FSCANTREE_INSIDEJUNCTION);

			CutToSlash(strFindPath,true);

			if (Flags.Check(FSCANTREE_RETUPDIR))
			{
				strFullName = strFindPath;
				apiGetFindDataEx(strFullName, *fdata);
			}

			CutToSlash(strFindPath);
			strFindPath += strFindMask;
			_SVS(SysLog(L"1. FullName='%s'",strFullName.CPtr()));

			if (Flags.Check(FSCANTREE_RETUPDIR))
			{
				Flags.Set(FSCANTREE_SECONDDIRNAME);
				return true;
			}

			return GetNextName(fdata,strFullName);
		}
	}
	else
	{
		if ((fdata->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) && Flags.Check(FSCANTREE_RECUR) &&
		        (!(fdata->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) || Flags.Check(FSCANTREE_SCANSYMLINK)))
		{
			string RealPath(ScanItems.lastItem()->RealPath);
			AddEndSlash(RealPath);
			RealPath += fdata->strFileName;

			if (fdata->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
				ConvertNameToReal(RealPath, RealPath);

			//recursive symlinks guard
			bool Recursion = false;

			for (size_t i = 0; i < ScanItems.getCount() && !Recursion; i++)
				Recursion = ScanItems.getItem(i)->RealPath == RealPath;

			if (!Recursion)
			{
				CutToSlash(strFindPath);
				strFindPath += fdata->strFileName;
				strFullName = strFindPath;
				strFindPath += L"\\";
				strFindPath += strFindMask;
				ScanItems.addItem();
				ScanItems.lastItem()->Flags = ScanItems.getItem(ScanItems.getCount()-2)->Flags; // наследуем флаг
				ScanItems.lastItem()->Flags.Clear(FSCANTREE_SECONDPASS);
				ScanItems.lastItem()->RealPath = RealPath;

				if (fdata->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
				{
					ScanItems.lastItem()->Flags.Set(FSCANTREE_INSIDEJUNCTION);
					Flags.Set(FSCANTREE_INSIDEJUNCTION);
				}

				return true;
			}
		}
	}

	strFullName = strFindPath;
	CutToSlash(strFullName);
	strFullName += fdata->strFileName;
	return true;
}

void ScanTree::SkipDir()
{
	if (!ScanItems.getCount())
		return;

	ScanItems.deleteItem(ScanItems.getCount()-1);

	if (!ScanItems.getCount())
		return;

	if (!ScanItems.lastItem()->Flags.Check(FSCANTREE_INSIDEJUNCTION))
		Flags.Clear(FSCANTREE_INSIDEJUNCTION);

	CutToSlash(strFindPath,true);
	CutToSlash(strFindPath);
	strFindPath += strFindMask;
}
