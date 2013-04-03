#pragma once

/*
filemasks.hpp

Класс для работы с масками файлов (учитывается наличие масок исключения).
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

#include  "RegExp.hpp"

enum FM_FLAGS
{
	FMF_SILENT = 1,
};

class filemasks
{
public:
	filemasks() {}
	~filemasks() {}

	bool Set(const string& Masks, DWORD Flags);
	bool Compare(const string& Name) const;
	bool IsEmpty() const;
	void ErrorMessage() const;

private:
	void Free();
	wchar_t* FindExcludeChar(wchar_t* masks) const;

	class masks
	{
	public:
		masks():n(0), bRE(false) {}
		~masks() {}

		bool Set(const string& Masks);
		void Free();
		bool Compare(const string& Name) const;
		bool IsEmpty() const;

	private:
		std::list<string> Masks;
		std::unique_ptr<RegExp> re;
		array_ptr<SMatch> m;
		int n;
		bool bRE;
	}
	Include, Exclude;
};
