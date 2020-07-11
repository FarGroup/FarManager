#ifndef IO_HPP_F819C4FA_5A8C_4721_AD6C_8688D9A8E53E
#define IO_HPP_F819C4FA_5A8C_4721_AD6C_8688D9A8E53E
#pragma once

/*
io.hpp

*/
/*
Copyright © 2018 Far Group
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

#include "range.hpp"
#include "scope_exit.hpp"

//----------------------------------------------------------------------------

namespace io
{
	template<typename char_type>
	class basic_streambuf_override
	{
	public:
		NONCOPYABLE(basic_streambuf_override);

		basic_streambuf_override(std::basic_ios<char_type>& Ios, std::basic_streambuf<char_type>& Buf) :
			m_Ios(Ios),
			m_OriginalBuffer(*Ios.rdbuf())
		{
			m_Ios.rdbuf(&Buf);
		}

		~basic_streambuf_override()
		{
			m_Ios.rdbuf(&m_OriginalBuffer);
		}

	private:
		std::basic_ios<char_type>& m_Ios;
		std::basic_streambuf<char_type>& m_OriginalBuffer;
	};

	using wstreambuf_override = basic_streambuf_override<wchar_t>;

	[[nodiscard]]
	inline size_t read(std::istream& Stream, span<std::byte> const Buffer)
	{
		{
			const auto Exceptions = Stream.exceptions();
			Stream.exceptions(Exceptions & ~(Stream.failbit | Stream.eofbit));
			SCOPE_SUCCESS{ Stream.exceptions(Exceptions); };

			Stream.read(static_cast<char*>(static_cast<void*>(Buffer.data())), Buffer.size());
			if (!Stream.bad() && Stream.eof())
				Stream.clear(Stream.eofbit);
		}

		return Stream.gcount();
	}

	template<typename container>
	void write(std::ostream& Stream, const container& Container)
	{
		static_assert(std::is_trivially_copyable_v<VALUE_TYPE(Container)>);

		const auto Size = std::size(Container);
		if (!Size)
			return;

		Stream.write(view_as<char const*>(std::data(Container)), Size * sizeof(*std::data(Container)));
	}
}

#endif // IO_HPP_F819C4FA_5A8C_4721_AD6C_8688D9A8E53E
