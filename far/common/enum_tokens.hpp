#ifndef ENUM_TOKENS_HPP_0472A061_7BE9_4932_B0C4_26DC64B4AB45
#define ENUM_TOKENS_HPP_0472A061_7BE9_4932_B0C4_26DC64B4AB45
#pragma once

/*
enum_tokens.hpp
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

#include "enumerator.hpp"

namespace detail::enum_tokens_policy
{
	class simple
	{
	public:
		string_view::iterator extract(const string_view& View, const string_view& Separators, string_view& Value) const
		{
			const auto NewIterator = std::find_first_of(ALL_CONST_RANGE(View), ALL_CONST_RANGE(Separators));
			Value = View.substr(0, NewIterator - View.cbegin());
			return NewIterator;
		}
	};

	class with_quotes
	{
	public:
		string_view::iterator extract(const string_view& View, const string_view& Separators, string_view& Value) const
		{
			auto MetQuote = false;

			const auto NewIterator = [&]
			{
				auto InQuotes = false;
				for (auto i = View.cbegin(); i != View.cend(); ++i)
				{
					if (*i == L'"')
					{
						InQuotes = !InQuotes;
						MetQuote = true;
					}

					if (!InQuotes && contains(Separators, *i))
						return i;
				}
				return View.cend();
			}();

			Value = View.substr(0, NewIterator - View.cbegin());

			if (MetQuote)
			{
				m_Cache.clear();
				m_Cache.reserve(Value.size());
				copy::unquote(Value, std::back_inserter(m_Cache));
				Value = m_Cache;
			}

			return NewIterator;
		}

	private:
		mutable string m_Cache;
	};
}

template<typename policy>
class enum_tokens_t: policy, public enumerator<enum_tokens_t<policy>, string_view>
{
	IMPLEMENTS_ENUMERATOR(enum_tokens_t);

public:
	enum_tokens_t(string&& Str, const string_view& Separators):
		m_Storage(std::move(Str)),
		m_View(m_Storage),
		m_Separators(Separators)
	{
	}

	enum_tokens_t(const string_view& Str, const string_view& Separators):
		m_View(Str),
		m_Separators(Separators)
	{
	}

	enum_tokens_t(const string& Str, const string_view& Separators) :
		enum_tokens_t(string_view(Str), Separators)
	{
	}

	enum_tokens_t(const wchar_t* Str, const string_view& Separators) :
		enum_tokens_t(string_view(Str), Separators)
	{
	}

private:
	bool get(size_t Index, string_view& Value) const
	{
		if (!Index)
			m_Iterator = m_View.cbegin();
		else if (m_Iterator != m_View.cend())
			++m_Iterator;

		if (m_Iterator == m_View.cend())
			return false;

		m_Iterator = policy::extract(m_View.substr(m_Iterator - m_View.cbegin()), m_Separators, Value);
		return true;
	}

	string m_Storage;
	string_view m_View;
	string_view m_Separators;
	mutable string_view::iterator m_Iterator{};
};

using enum_tokens = enum_tokens_t<detail::enum_tokens_policy::simple>;
using enum_tokens_with_quotes = enum_tokens_t<detail::enum_tokens_policy::with_quotes>;

#endif // ENUM_TOKENS_HPP_0472A061_7BE9_4932_B0C4_26DC64B4AB45
