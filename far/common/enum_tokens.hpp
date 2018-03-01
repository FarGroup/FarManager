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

namespace detail
{
	class null_overrider
	{
	public:
		void reset() {}
		bool active(string_view::iterator i) { return false; }
		void postprocess(string_view& Value) {}
	};

	template<typename... args>
	class composite_overrider: public std::tuple<null_overrider, args...>
	{
	public:
		void reset()
		{
			// applied to all args
			return reset_impl();
		}

		bool active(string_view::iterator i)
		{
			// applied to all args left to right
			return active_impl(i);
		}

		void postprocess(string_view& Value)
		{
			// applied to all args right to left
			return postprocess_impl(Value);
		}

	private:
		template<size_t index, template<class> typename operation>
		auto& get_opt()
		{
			using nth_type = std::tuple_element_t<index, std::tuple<null_overrider, args...>>;
			using is_valid = is_valid<nth_type, operation>;
			return std::get<is_valid::value? index : 0>(*this);
		}

		template<typename T>
		using try_reset = decltype(std::declval<T&>().reset());

		template<size_t index, REQUIRES(index >= sizeof...(args) + 1)>
		void reset_impl() {}

		template<size_t index = 1, REQUIRES(index < sizeof...(args) + 1)>
		void reset_impl()
		{
			get_opt<index, try_reset>().reset();
			reset_impl<index + 1>();
		}

		template<class T>
		using try_active = decltype(std::declval<T&>().active(std::declval<string_view::iterator&>()));

		template<size_t index, REQUIRES(index >= sizeof...(args) + 1)>
		bool active_impl(string_view::iterator) { return false; }

		template<size_t index = 1, REQUIRES(index < sizeof...(args) + 1)>
		bool active_impl(string_view::iterator i)
		{
			return get_opt<index, try_active>().active(i) || active_impl<index + 1>(i);
		}

		template<typename T>
		using try_postprocess = decltype(std::declval<T&>().postprocess(std::declval<string_view&>()));

		template<size_t index, REQUIRES(index >= sizeof...(args) + 1)>
		void postprocess_impl(string_view&) {}

		template<size_t index = 1, REQUIRES(index < sizeof...(args) + 1)>
		void postprocess_impl(string_view& Value)
		{
			get_opt<sizeof...(args) + 1 - index, try_postprocess>().postprocess(Value);
			postprocess_impl<index + 1>(Value);
		}
	};

	class quotes_overrider
	{
	public:
		void reset()
		{
			m_InQuotes = false;
			m_MetQuote = false;
		}

		bool active(string_view::iterator i)
		{
			if (*i == L'"')
			{
				m_InQuotes = !m_InQuotes;
				m_MetQuote = true;
				return true;
			}

			return m_InQuotes;
		}

		void postprocess(string_view& Value)
		{
			if (!m_MetQuote)
				return;

			m_Cache.clear();
			reserve_exp_noshrink(m_Cache, Value.size());
			copy::unquote(Value, std::back_inserter(m_Cache));
			Value = m_Cache;
		}

		bool m_InQuotes{};
		bool m_MetQuote{};
		string m_Cache;
	};

	class trimmer
	{
	public:
		static void postprocess(string_view& Value)
		{
			Value = trim(Value);
		}
	};

	class simple_policy
	{
	public:
		string_view::iterator extract(const string_view View, const string_view Separators, string_view& Value) const
		{
			const auto NewIterator = std::find_first_of(ALL_CONST_RANGE(View), ALL_CONST_RANGE(Separators));
			Value = View.substr(0, NewIterator - View.cbegin());
			return NewIterator;
		}
	};

	template<typename... overriders>
	class custom_policy
	{
	public:
		string_view::iterator extract(const string_view View, const string_view Separators, string_view& Value) const
		{
			m_Overrider.reset();

			const auto NewIterator = [&]
			{
				for (auto i = View.cbegin(); i != View.cend(); ++i)
				{
					if (m_Overrider.active(i))
						continue;

					if (contains(Separators, *i))
						return i;
				}
				return View.cend();
			}();

			Value = View.substr(0, NewIterator - View.cbegin());

			m_Overrider.postprocess(Value);

			return NewIterator;
		}

	private:
		mutable composite_overrider<overriders...> m_Overrider;
	};
}

template<typename policy>
class enum_tokens_t: policy, public enumerator<enum_tokens_t<policy>, string_view>
{
	IMPLEMENTS_ENUMERATOR(enum_tokens_t);

public:
	enum_tokens_t(string&& Str, const string_view Separators):
		m_Storage(std::move(Str)),
		m_View(m_Storage),
		m_Separators(Separators)
	{
	}

	enum_tokens_t(const string_view Str, const string_view Separators):
		m_View(Str),
		m_Separators(Separators)
	{
	}

	enum_tokens_t(const string& Str, const string_view Separators) :
		enum_tokens_t(string_view(Str), Separators)
	{
	}

	enum_tokens_t(const wchar_t* const Str, const string_view Separators) :
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

using enum_tokens = enum_tokens_t<detail::simple_policy>;

using with_quotes = detail::quotes_overrider;
using with_trim = detail::trimmer;

template<typename... args>
using enum_tokens_custom_t = enum_tokens_t<detail::custom_policy<args...>>;

template<typename... args>
using enum_tokens_with_quotes_t = enum_tokens_custom_t<detail::quotes_overrider, args...>;

using enum_tokens_with_quotes = enum_tokens_with_quotes_t<>;

#endif // ENUM_TOKENS_HPP_0472A061_7BE9_4932_B0C4_26DC64B4AB45
