﻿#ifndef SCANTREE_HPP_43BA8967_D0CF_4462_8AA7_93728B999585
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


#include "bitflags.hpp"

#include "platform.fwd.hpp"

enum
{
	// эту фигню может ставить плагин (младшие 8 бит)
	FSCANTREE_RETUPDIR         = 0x00000001, // = FRS_RETUPDIR
	// FSCANTREE_RETUPDIR causes GetNextName() to return every directory twice:
	// 1. when scanning its parent directory 2. after directory scan is finished
	FSCANTREE_RECUR            = 0x00000002, // = FRS_RECUR
	FSCANTREE_SCANSYMLINK      = 0x00000004, // = FRS_SCANSYMLINK

	// в младшем слове старшие 8 бита служебные!
	FSCANTREE_SECONDPASS       = 0x00002000, // то, что раньше было было SecondPass[]
	FSCANTREE_SECONDDIRNAME    = 0x00004000, // set when FSCANTREE_RETUPDIR is enabled and directory scan is finished
	FSCANTREE_INSIDEJUNCTION   = 0x00008000, // - мы внутри симлинка?

	// здесь те флаги, которые могут выставляться в 3-м параметре SetFindPath()
	FSCANTREE_FILESFIRST       = 0x00010000, // Сканирование каталога за два прохода. Сначала файлы, затем каталоги
};

class ScanTree: noncopyable
{
public:
	explicit ScanTree(bool RetUpDir, bool Recurse=true, int ScanJunction=-1);
	~ScanTree();

	// 3-й параметр - флаги из старшего слова
	void SetFindPath(const string& Path, string_view Mask, DWORD NewScanFlags = FSCANTREE_FILESFIRST);
	bool GetNextName(os::fs::find_data& fdata, string &strFullName);
	void SkipDir();
	int IsDirSearchDone() const {return Flags.Check(FSCANTREE_SECONDDIRNAME);}
	int InsideJunction() const {return Flags.Check(FSCANTREE_INSIDEJUNCTION);}

	struct scantree_item;

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
