#ifndef FILESTR_HPP_1B6BCA12_AFF9_4C80_A59C_B4B92B21F83F
#define FILESTR_HPP_1B6BCA12_AFF9_4C80_A59C_B4B92B21F83F
#pragma once

/*
filestr.hpp

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

#include "encoding.hpp"
#include "eol.hpp"

struct file_line
{
	string_view Str;
	eol::type Eol;
};

// TODO: rename
class enum_file_lines : public enumerator<enum_file_lines, file_line>
{
	IMPLEMENTS_ENUMERATOR(enum_file_lines);

public:
	enum_file_lines(const os::fs::file& SrcFile, uintptr_t CodePage);
	bool conversion_error() const { return m_ConversionError; }

private:
	bool get(bool Reset, file_line& Value) const;

	bool GetString(string_view& Str, eol::type& Eol) const;

	template<typename T>
	bool GetTString(std::vector<T>& From, std::vector<T>& To, eol::type& Eol, bool bBigEndian = false) const;

	const os::fs::file& SrcFile;
	size_t BeginPos;
	uintptr_t m_CodePage;
	raw_eol m_Eol;

	mutable std::vector<char> m_ReadBuf;
	mutable std::vector<wchar_t> m_wReadBuf;
	mutable std::vector<wchar_t> m_wStr;
	mutable size_t ReadPos{};
	mutable size_t ReadSize{};
	mutable bool m_ConversionError{};
	mutable bool m_CrSeen{};
	mutable bool m_CrCr{};
};

bool GetFileFormat(const os::fs::file& file, uintptr_t& nCodePage, bool* pSignatureFound = nullptr, bool bUseHeuristics = true, bool* pPureAscii = nullptr);

#endif // FILESTR_HPP_1B6BCA12_AFF9_4C80_A59C_B4B92B21F83F
