#ifndef DIZLIST_HPP_0115E7F4_A98B_42CE_A43A_275B8A6DFFEF
#define DIZLIST_HPP_0115E7F4_A98B_42CE_A43A_275B8A6DFFEF
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

// Internal:
#include "string_utils.hpp"

// Platform:

// Common:
#include "common/noncopyable.hpp"

// External:

//----------------------------------------------------------------------------

class DizList: noncopyable
{
public:
	DizList();

	void Read(string_view Path, const string* DizName = nullptr);

	void Set(const string& Name, const string& ShortName, const string& DizText);
	bool Erase(const string& Name, const string& ShortName);

	string_view Get(const string& Name, const string& ShortName, long long FileSize) const;

	void Reset();
	bool Flush(string_view Path, const string* DizName = nullptr);
	bool CopyDiz(const string& Name, const string& ShortName, const string& DestName, const string& DestShortName,DizList *DestDiz) const;
	const string& GetDizName() const { return m_DizFileName; }

private:
	using description_data = std::list<string>;
	using desc_map = std::unordered_multimap<string, description_data, hash_icase_t, equal_icase_t>;

	desc_map::iterator Insert(string_view Name);
	desc_map::iterator Find(const string& Name, const string& ShortName);
	desc_map::const_iterator Find(const string& Name, const string& ShortName) const;

	desc_map m_DizData;
	std::list<description_data> m_RemovedEntries;
	std::list<desc_map::value_type*> m_OrderForWrite;
	string m_DizFileName;
	uintptr_t m_CodePage;
	bool m_Modified{};
};

#endif // DIZLIST_HPP_0115E7F4_A98B_42CE_A43A_275B8A6DFFEF
