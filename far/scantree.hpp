#ifndef SCANTREE_HPP_43BA8967_D0CF_4462_8AA7_93728B999585
#define SCANTREE_HPP_43BA8967_D0CF_4462_8AA7_93728B999585
#pragma once

/*
scantree.hpp

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


// Internal:
#include "bitflags.hpp"

// Platform:
#include "platform.fwd.hpp"

// Common:
#include "common/noncopyable.hpp"

// External:

//----------------------------------------------------------------------------

class ScanTree: noncopyable
{
public:
	explicit ScanTree(bool RetUpDir, bool Recurse=true, int ScanJunction=-1, bool FilesFirst = true);
	~ScanTree();

	// 3-й параметр - флаги из старшего слова
	void SetFindPath(string_view Path, string_view Mask);
	bool GetNextName(os::fs::find_data& fdata, string &strFullName);
	void SkipDir();
	bool IsDirSearchDone() const;
	bool InsideReparsePoint() const;

	class scantree_item;

private:
	BitFlags Flags;
	std::list<scantree_item> ScanItems;
	// path in full NT format, used internally to get correct results
	string strFindPath;
	// relative path, it's what caller expects to receive
	string strFindPathOriginal;
	string strFindMask;
};

#endif // SCANTREE_HPP_43BA8967_D0CF_4462_8AA7_93728B999585
