#pragma once

/*
dizlist.hpp

Описания файлов
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

struct DizRecord
{
	string DizText;
	int NameStart;
	int NameLength;
	bool Deleted;
};

class DizList
{
	private:
		string strDizFileName;
		DizRecord **DizData;
		size_t DizCount;
		size_t *IndexData;
		size_t IndexCount;
		bool Modified;
		bool NeedRebuild;
		UINT OrigCodePage;
		char *AnsiBuf;

	private:
		int GetDizPos(const string& Name, int *TextPos);
		int GetDizPosEx(const string& Name, const string& ShortName, int *TextPos);
		bool AddRecord(const string& DizText);
		void BuildIndex();

	public:
		DizList();
		~DizList();

	public:
		void Read(const string& Path, const string* DizName=nullptr);
		void Reset();
		const wchar_t *GetDizTextAddr(const string& Name, const string& ShortName, const __int64 FileSize);
		bool DeleteDiz(const string& Name, const string& ShortName);
		bool Flush(const string& Path, const string *DizName=nullptr);
		bool AddDizText(const string& Name, const string& ShortName, const string& DizText);
		bool CopyDiz(const string& Name, const string& ShortName, const string& DestName, const string& DestShortName,DizList *DestDiz);
		void GetDizName(string &strDizName);
		static void PR_ReadingMsg();
};
