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

// Internal:
#include "encoding.hpp"
#include "eol.hpp"

// Platform:
#include "platform.fwd.hpp"

// Common:
#include "common/enumerator.hpp"
#include "common/smart_ptr.hpp"

// External:

//----------------------------------------------------------------------------

struct file_line
{
	string_view Str;
	eol Eol;
};

// TODO: rename
class [[nodiscard]] enum_lines: public enumerator<enum_lines, file_line>
{
	IMPLEMENTS_ENUMERATOR(enum_lines);

public:
	enum_lines(std::istream& Stream, uintptr_t CodePage);

	bool conversion_error() const { return m_Diagnostics.ErrorPosition.has_value(); }

private:
	[[nodiscard]]
	bool get(bool Reset, file_line& Value) const;

	bool GetString(string_view& Str, eol& Eol) const;

	bool fill() const;

	template<typename T>
	bool GetTString(std::basic_string<T>& To, eol& Eol) const;

	std::istream& m_Stream;
	size_t m_BeginPos;
	uintptr_t m_CodePage;
	raw_eol m_Eol;

	mutable char_ptr m_Buffer;
	mutable std::string_view m_BufferView;

	enum
	{
		default_capacity = 1024,
	};

	struct conversion_data
	{
		mutable std::string m_Bytes;
		mutable wchar_t_ptr_n<default_capacity> m_wBuffer;
	};

	mutable std::variant<conversion_data, string> m_Data;

	mutable size_t m_CrSeen{};
	mutable bool m_EmitExtraCr{};
	mutable encoding::diagnostics m_Diagnostics;
};

// If the file contains a BOM this function will advance the file pointer by the BOM size (either 2 or 3)
uintptr_t GetFileCodepage(const os::fs::file& File, uintptr_t DefaultCodepage, bool* SignatureFound = nullptr, bool UseHeuristics = true);

#endif // FILESTR_HPP_1B6BCA12_AFF9_4C80_A59C_B4B92B21F83F
