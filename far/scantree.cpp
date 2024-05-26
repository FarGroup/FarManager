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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "scantree.hpp"

// Internal:
#include "config.hpp"
#include "pathmix.hpp"
#include "cvtname.hpp"
#include "global.hpp"
#include "log.hpp"

// Platform:
#include "platform.hpp"
#include "platform.fs.hpp"

// Common:
#include "common/algorithm.hpp"
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

enum tree_flags
{
	// TREE_RETUPDIR causes GetNextName() to return every directory twice:
	// 1. when scanning its parent directory
	// 2. after directory scan is finished
	TREE_RETUPDIR      = 0_bit, // = FRS_RETUPDIR
	TREE_RECUR         = 1_bit, // = FRS_RECUR
	TREE_SCANSYMLINK   = 2_bit, // = FRS_SCANSYMLINK
	TREE_FILESFIRST    = 3_bit, // Сканирование каталога за два прохода. Сначала файлы, затем каталоги
	TREE_SECONDDIRNAME = 4_bit, // set when FSCANTREE_RETUPDIR is enabled and directory scan is finished
};

enum tree_item_flags
{
	TREE_ITEM_SECONDPASS           = 0_bit, // то, что раньше было было SecondPass[]
	TREE_ITEM_INSIDE_REPARSE_POINT = 1_bit, // For Copy: we don't want to delete anything from any reparse points
};


class ScanTree::scantree_item
{
public:
	NONCOPYABLE(scantree_item);
	MOVE_CONSTRUCTIBLE(scantree_item);

	scantree_item() = default;

	BitFlags Flags;
	std::unique_ptr<os::fs::enum_files> Find;
	os::fs::enum_files::iterator Iterator;
	string RealPath;
	unordered_string_set ActiveDirectories;
};

ScanTree::ScanTree(bool RetUpDir, bool Recurse, int ScanJunction, bool FilesFirst)
{
	Flags.Change(TREE_RETUPDIR, RetUpDir);
	Flags.Change(TREE_RECUR, Recurse);
	Flags.Change(TREE_SCANSYMLINK, ScanJunction == -1? Global->Opt->ScanJunction.Get() : ScanJunction != 0);
	Flags.Change(TREE_FILESFIRST, FilesFirst);
}

ScanTree::~ScanTree() = default;

void ScanTree::SetFindPath(string_view const Path, string_view const Mask)
{
	ScanItems.clear();

	strFindMask = Mask;

	strFindPathOriginal = Path;
	AddEndSlash(strFindPathOriginal);

	const auto FullPath = nt_path(ConvertNameToFull(Path));

	strFindPath = nt_path(ConvertNameToReal(FullPath));

	scantree_item Item;
	Item.RealPath = strFindPath;
	Item.ActiveDirectories.emplace(strFindPath);

	path::append(strFindPath, strFindMask);

	if (os::fs::is_directory_reparse_point(os::fs::get_file_attributes(FullPath)))
	{
		Item.Flags.Set(TREE_ITEM_INSIDE_REPARSE_POINT);
	}

	ScanItems.emplace_back(std::move(Item));
}

bool ScanTree::GetNextName(os::fs::find_data& fdata,string &strFullName)
{
	if (ScanItems.empty())
		return false;

	bool Done=false;
	Flags.Clear(TREE_SECONDDIRNAME);

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

			if (Flags.Check(TREE_FILESFIRST))
			{
				if (LastItem.Flags.Check(TREE_ITEM_SECONDPASS))
				{
					if (!Done && !(fdata.Attributes & FILE_ATTRIBUTE_DIRECTORY))
						continue;
				}
				else
				{
					if (!Done && (fdata.Attributes & FILE_ATTRIBUTE_DIRECTORY))
						continue;

					if (Done)
					{
						LastItem.Find.reset();
						LastItem.Flags.Set(TREE_ITEM_SECONDPASS);
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
			CutToSlash(strFindPath,true);
			CutToSlash(strFindPathOriginal,true);

			if (Flags.Check(TREE_RETUPDIR))
			{
				strFullName = strFindPathOriginal;
				// BUGBUG check result
				if (!os::fs::get_find_data(strFindPath, fdata))
				{
					LOGWARNING(L"get_find_data({}): {}"sv, strFindPath, os::last_error());
				}

			}

			CutToSlash(strFindPath);
			strFindPath += strFindMask;

			if (Flags.Check(TREE_RETUPDIR))
			{
				Flags.Set(TREE_SECONDDIRNAME);
				return true;
			}

			return GetNextName(fdata,strFullName);
		}
	}
	else
	{
		const auto is_link = os::fs::is_directory_symbolic_link(fdata);
		if ((fdata.Attributes&FILE_ATTRIBUTE_DIRECTORY) && Flags.Check(TREE_RECUR) && (!is_link || Flags.Check(TREE_SCANSYMLINK)))
		{
			auto RealPath = path::join(ScanItems.back().RealPath, fdata.FileName);

			if (is_link)
				RealPath = nt_path(ConvertNameToReal(RealPath));

			//recursive symlinks guard
			if (!ScanItems.back().ActiveDirectories.contains(RealPath))
			{
				CutToSlash(strFindPath);
				path::append(strFindPath, fdata.FileName, strFindMask);

				CutToSlash(strFindPathOriginal);
				strFindPathOriginal += fdata.FileName;

				strFullName = strFindPathOriginal;
				AddEndSlash(strFindPathOriginal);

				scantree_item Data;
				Data.Flags = ScanItems.back().Flags; // наследуем флаг
				Data.Flags.Clear(TREE_ITEM_SECONDPASS);
				Data.RealPath = RealPath;
				Data.ActiveDirectories = ScanItems.back().ActiveDirectories;
				if (Flags.Check(TREE_SCANSYMLINK))
					Data.ActiveDirectories.emplace(RealPath);

				if (os::fs::is_directory_reparse_point(fdata.Attributes))
				{
					Data.Flags.Set(TREE_ITEM_INSIDE_REPARSE_POINT);
				}
				ScanItems.emplace_back(std::move(Data));

				return true;
			}
		}
	}

	strFullName = strFindPathOriginal;
	CutToSlash(strFullName);
	strFullName += fdata.FileName;

	return true;
}

void ScanTree::SkipDir()
{
	if (ScanItems.empty())
		return;

	{
		const auto& Current = ScanItems.back();
		if (!Flags.Check(TREE_SCANSYMLINK) &&
			Current.Find &&
			Current.Iterator != Current.Find->end() &&
			os::fs::is_directory_symbolic_link(*Current.Iterator)
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

	CutToSlash(strFindPath,true);
	CutToSlash(strFindPathOriginal,true);
	CutToSlash(strFindPath);
	CutToSlash(strFindPathOriginal);
	strFindPath += strFindMask;
}

bool ScanTree::IsDirSearchDone() const
{
	return Flags.Check(TREE_SECONDDIRNAME);
}

bool ScanTree::InsideReparsePoint() const
{
	return std::ranges::any_of(ScanItems, [](scantree_item const& i)
	{
		return i.Flags.Check(TREE_ITEM_INSIDE_REPARSE_POINT);
	});
}
