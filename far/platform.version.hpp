#ifndef PLATFORM_VERSION_HPP_CC7E1536_485F_4A75_862F_E15DEF06C5C5
#define PLATFORM_VERSION_HPP_CC7E1536_485F_4A75_862F_E15DEF06C5C5
#pragma once

/*
platform.version.hpp

*/
/*
Copyright © 2020 Far Group
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

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

namespace os::version
{
	class file_version
	{
	public:
		bool read(string_view Filename);
		wchar_t const* get_string(string_view Value) const;
		VS_FIXEDFILEINFO const* get_fixed_info() const;

	private:
		string m_BlockPath;
		std::vector<std::byte> m_Buffer;
	};

	bool is_win10_build_or_later(DWORD Build);

	// This versioning scheme is mental

	inline bool is_win10_1607_or_later()
	{
		return is_win10_build_or_later(14393);
	}

	inline bool is_win10_1703_or_later()
	{
		return is_win10_build_or_later(15063);
	}
}

#endif // PLATFORM_VERSION_HPP_CC7E1536_485F_4A75_862F_E15DEF06C5C5
