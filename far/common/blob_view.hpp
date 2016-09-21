#ifndef BLOB_VIEW_HPP_3707377A_7C4B_4B2E_89EC_6411A1988FB3
#define BLOB_VIEW_HPP_3707377A_7C4B_4B2E_89EC_6411A1988FB3
#pragma once

/*
blob_view.hpp
*/
/*
Copyright © 2016 Far Group
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

using blob_view = range<const char*>;

class writable_blob_view: public range<char*>
{
public:
	NONCOPYABLE(writable_blob_view);
	writable_blob_view() = default;
	writable_blob_view(void* Data, size_t Size): range<char*>(reinterpret_cast<char*>(Data), reinterpret_cast<char*>(Data) + Size) {}
	~writable_blob_view()
	{
		if (m_Allocated)
			delete[] static_cast<const char*>(data());
	}

	writable_blob_view& operator=(const blob_view& rhs)
	{
		if (data())
		{
			if (size() != rhs.size())
				throw MAKE_FAR_EXCEPTION("incorrect blob size");
		}
		else
		{
			static_cast<range<char*>&>(*this) = make_range(new char[rhs.size()], rhs.size());
			m_Allocated = true;
		}
		memcpy(data(), rhs.data(), size());
		return *this;
	}
private:
	bool m_Allocated{};
};

inline auto make_blob_view(const void* Object, size_t Size)
{
	return make_range(reinterpret_cast<const char*>(Object), Size);
}

template<typename T>
auto make_blob_view(const T& Object)
{
	TERSE_STATIC_ASSERT(std::is_pod<T>::value);
	return make_blob_view(&Object, sizeof Object);
}

#endif // BLOB_VIEW_HPP_3707377A_7C4B_4B2E_89EC_6411A1988FB3
