#ifndef BLOB_BUILDER_HPP_9C0C6EE1_CAEE_4523_BDEE_53AEE67C167C
#define BLOB_BUILDER_HPP_9C0C6EE1_CAEE_4523_BDEE_53AEE67C167C
#pragma once

/*
blob_builder.hpp

*/
/*
Copyright © 2017 Far Group
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

class blob_builder
{
public:
	NONCOPYABLE(blob_builder);

	explicit blob_builder(uintptr_t CodePage):
		m_CodePage(CodePage),
		m_Signature(IsUnicodeOrUtfCodePage(m_CodePage))
	{
		if (m_Signature)
		{
			m_Data.insert(0, 1, SIGN_UNICODE);
		}
	}

	auto& append(const string_view& Str)
	{
		m_Buffer.clear();
		::append(m_Data, Str);
		return *this;
	}

	bytes_view get() const
	{
		if (m_Data.empty() || (m_Signature && m_Data.size() == 1))
		{
			return { nullptr, 0 };
		}

		if (m_CodePage == CP_UNICODE)
		{
			return bytes_view(m_Data.data(), m_Data.size() * sizeof(wchar_t));
		}

		if (m_Buffer.empty())
		{
			m_Buffer = encoding::get_bytes(m_CodePage, m_Data);
		}
		return bytes_view(m_Buffer.data(), m_Buffer.size());
	}

private:
	mutable std::string m_Buffer;
	string m_Data;
	uintptr_t m_CodePage;
	bool m_Signature;
};

#endif // BLOB_BUILDER_HPP_9C0C6EE1_CAEE_4523_BDEE_53AEE67C167C
