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
#include "cvtname.hpp"

struct ScanTree::scantree_item
{
public:
	NONCOPYABLE(scantree_item);
	MOVABLE(scantree_item);

	scantree_item() = default;

	BitFlags Flags;
	std::unique_ptr<os::fs::enum_files> Find;
	os::fs::enum_files::iterator Iterator;
	string RealPath;
	std::unordered_set<string> ActiveDirectories;
};

static bool LinkToRealPath(const string& Src, string& Real)
{
	auto Test = ConvertNameToReal(Src);
	if (!Test.empty())
	{
		Test = NTPath(Test);
		if (Src != Test)
		{
			Real = Test;
			return true;
		}
	}

	Real = Src;
	return false;
}

ScanTree::ScanTree(bool RetUpDir, bool Recurse, int ScanJunction)
{
	Flags.Change(FSCANTREE_RETUPDIR,RetUpDir);
	Flags.Change(FSCANTREE_RECUR,Recurse);
	Flags.Change(FSCANTREE_SCANSYMLINK, ScanJunction == -1? Global->Opt->ScanJunction.Get() : ScanJunction != 0);
}

ScanTree::~ScanTree() = default;

void ScanTree::SetFindPath(const string& Path,const string& Mask, const DWORD NewScanFlags)
{
	ScanItems.clear();

	Flags.Clear(FSCANTREE_FILESFIRST);

	strFindMask = Mask;

	strFindPathOriginal = Path;
	AddEndSlash(strFindPathOriginal);

	strFindPath = NTPath(ConvertNameToReal(Path));

	scantree_item Item;
	Item.RealPath = strFindPath;
	Item.ActiveDirectories.emplace(strFindPath);

	AddEndSlash(strFindPath);
	strFindPath += strFindMask;

	Flags.Set((Flags.Flags()&0x0000FFFF)|(NewScanFlags&0xFFFF0000));

	ScanItems.emplace_back(std::move(Item));
}

bool ScanTree::GetNextName(os::fs::find_data& fdata,string &strFullName)
{
	if (ScanItems.empty())
		return false;

	bool Done=false;
	Flags.Clear(FSCANTREE_SECONDDIRNAME);

	{
		scantree_item& LastItem = ScanItems.back();
		for (;;)
		{
			if (!LastItem.Find)
			{
				LastItem.Find = std::make_unique<os::fs::enum_files>(strFindPath);
				LastItem.Iterator = LastItem.Find->end();
			}

			if (LastItem.Iterator == LastItem.Find->end())
			{
				LastItem.Iterator = LastItem.Find->begin();
			}
			else
			{
				++LastItem.Iterator;
			}

			Done = LastItem.Iterator == LastItem.Find->end();

			if (!Done)
			{
				fdata = *ScanItems.back().Iterator;
			}

			if (Flags.Check(FSCANTREE_FILESFIRST))
			{
				if (LastItem.Flags.Check(FSCANTREE_SECONDPASS))
				{
					if (!Done && !(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
						continue;
				}
				else
				{
					if (!Done && (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
						continue;

					if (Done)
					{
						LastItem.Find.reset();
						LastItem.Flags.Set(FSCANTREE_SECONDPASS);
						continue;
					}
				}
			}

			break;
		}
	}

	if (Done)
	{
		ScanItems.pop_back();

		if (ScanItems.empty())
			return false;
		else
		{
			if (ScanItems.back().Flags.Check(FSCANTREE_INSIDEJUNCTION))
				Flags.Clear(FSCANTREE_INSIDEJUNCTION);

			CutToSlash(strFindPath,true);
			CutToSlash(strFindPathOriginal,true);

			if (Flags.Check(FSCANTREE_RETUPDIR))
			{
				strFullName = strFindPathOriginal;
				os::fs::get_find_data(strFindPath, fdata);
			}

			CutToSlash(strFindPath);
			strFindPath += strFindMask;
			_SVS(SysLog(L"1. FullName='%s'",strFullName.data()));

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
		auto is_link = (fdata.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) != 0;
		if ((fdata.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) && Flags.Check(FSCANTREE_RECUR) && (!is_link || Flags.Check(FSCANTREE_SCANSYMLINK)))
		{
			string RealPath(ScanItems.back().RealPath);
			AddEndSlash(RealPath);
			RealPath += fdata.strFileName;

			bool real_path = !is_link || LinkToRealPath(RealPath, RealPath);

			//recursive symlinks guard
			if (real_path && !ScanItems.back().ActiveDirectories.count(RealPath))
			{
				CutToSlash(strFindPath);
				append(strFindPath, fdata.strFileName, L'\\', strFindMask);

				CutToSlash(strFindPathOriginal);
				strFindPathOriginal += fdata.strFileName;

				strFullName = strFindPathOriginal;
				AddEndSlash(strFindPathOriginal);

				scantree_item Data;
				Data.Flags = ScanItems.back().Flags; // наследуем флаг
				Data.Flags.Clear(FSCANTREE_SECONDPASS);
				Data.RealPath = RealPath;
				Data.ActiveDirectories = ScanItems.back().ActiveDirectories;
				if (Flags.Check(FSCANTREE_SCANSYMLINK))
					Data.ActiveDirectories.emplace(RealPath);

				if (is_link)
				{
					Data.Flags.Set(FSCANTREE_INSIDEJUNCTION);
					Flags.Set(FSCANTREE_INSIDEJUNCTION);
				}
				ScanItems.emplace_back(std::move(Data));

				return true;
			}
		}
	}

	strFullName = strFindPathOriginal;
	CutToSlash(strFullName);
	strFullName += fdata.strFileName;

	return true;
}

void ScanTree::SkipDir()
{
	if (ScanItems.empty())
		return;

	{
		const auto& Current = ScanItems.back();
		if (!Flags.Check(FSCANTREE_SCANSYMLINK) &&
			Current.Find &&
			Current.Iterator != Current.Find->end() &&
			(Current.Iterator->dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) == (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)
			)
		{
			// The current item is a directory link but we don't treat it as a directory so it's nothing to skip
			// (perhaps it could have been better to add a special flag for this case rather than the quite complex condition above,
			// but these flags is already a mess).
			return;
		}
	}

	ScanItems.pop_back();

	if (ScanItems.empty())
		return;

	if (!ScanItems.back().Flags.Check(FSCANTREE_INSIDEJUNCTION))
		Flags.Clear(FSCANTREE_INSIDEJUNCTION);

	CutToSlash(strFindPath,true);
	CutToSlash(strFindPathOriginal,true);
	CutToSlash(strFindPath);
	CutToSlash(strFindPathOriginal);
	strFindPath += strFindMask;
}
