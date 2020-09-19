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
#include "type_traits.hpp"
#include "string_utils.hpp"

//----------------------------------------------------------------------------

namespace detail
{
	class null_overrider
	{
	public:
		void reset() noexcept {}

		[[nodiscard]]
		bool active(wchar_t) noexcept { return false; }

		void postprocess(std::wstring_view&) noexcept {}
	};

	template<typename... args>
	class composite_overrider: public base<std::tuple<null_overrider, args...>>
	{
		using base_type = typename composite_overrider::base_type;

	public:
		void reset()
		{
			// applied to all args
			return reset_impl(index_sequence{});
		}

		[[nodiscard]]
		bool active(wchar_t i)
		{
			// applied to all args left to right
			return active_impl(i, index_sequence{});
		}

		void postprocess(std::wstring_view& Value)
		{
			// applied to all args right to left
			return postprocess_impl(Value, index_sequence{});
		}

	private:
		using index_sequence = std::index_sequence_for<args...>;

		template<size_t index, template<typename> typename operation>
		[[nodiscard]]
		auto& get_opt()
		{
			using nth_type = std::tuple_element_t<index + 1, base_type>;
			// This idiotic cast to std::tuple is for clang
			return std::get<is_detected_v<operation, nth_type>? index + 1 : 0>(static_cast<base_type&>(*this));
		}

		template<typename T>
		using try_reset = decltype(std::declval<T&>().reset());

		template<size_t... I>
		void reset_impl(std::index_sequence<I...>)
		{
			(..., get_opt<I, try_reset>().reset());
		}

		template<class T>
		using try_active = decltype(std::declval<T&>().active(wchar_t{}));

		template<size_t... I>
		[[nodiscard]]
		bool active_impl(wchar_t i, std::index_sequence<I...>)
		{
			return (... || get_opt<I, try_active>().active(i));
		}

		template<typename T>
		using try_postprocess = decltype(std::declval<T&>().postprocess(std::declval<std::wstring_view&>()));

		template<size_t... I>
		void postprocess_impl(std::wstring_view& Value, std::index_sequence<I...>)
		{
			(..., get_opt<sizeof...(args) - 1 - I, try_postprocess>().postprocess(Value));
		}
	};

	class quotes_overrider
	{
	public:
		void reset() noexcept
		{
			m_InQuotes = false;
			m_MetQuote = false;
		}

		[[nodiscard]]
		bool active(wchar_t i) noexcept
		{
			if (i == L'"')
			{
				m_InQuotes = !m_InQuotes;
				m_MetQuote = true;
				return true;
			}

			return m_InQuotes;
		}

		void postprocess(std::wstring_view& Value)
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
		std::wstring m_Cache;
	};

	class trimmer
	{
	public:
		static void postprocess(std::wstring_view& Value) noexcept
		{
			Value = trim(Value);
		}
	};

	class simple_policy
	{
	public:
		[[nodiscard]]
		auto extract(std::wstring_view::const_iterator const Begin, std::wstring_view::const_iterator const End, const std::wstring_view Separators, std::wstring_view& Value) const
		{
			const auto NewIterator = std::find_first_of(Begin, End, ALL_CONST_RANGE(Separators));
			Value = make_string_view(Begin, NewIterator);
			return NewIterator;
		}
	};

	template<typename... overriders>
	class custom_policy
	{
	public:
		[[nodiscard]]
		auto extract(std::wstring_view::const_iterator const Begin, std::wstring_view::const_iterator const End, const std::wstring_view Separators, std::wstring_view& Value) const
		{
			m_Overrider.reset();

			const auto NewIterator = [&]
			{
				for (auto i = Begin; i != End; ++i)
				{
					if (m_Overrider.active(*i))
						continue;

					if (contains(Separators, *i))
						return i;
				}
				return End;
			}();

			Value = make_string_view(Begin, NewIterator);

			m_Overrider.postprocess(Value);

			return NewIterator;
		}

	private:
		mutable composite_overrider<overriders...> m_Overrider;
	};
}

template<typename policy>
class [[nodiscard]] enum_tokens_t: policy, public enumerator<enum_tokens_t<policy>, std::wstring_view>
{
	IMPLEMENTS_ENUMERATOR(enum_tokens_t);

public:
	enum_tokens_t(std::wstring&& Str, const std::wstring_view Separators):
		m_Storage(std::move(Str)),
		m_View(m_Storage),
		m_Separators(Separators)
	{
	}

	enum_tokens_t(const std::wstring_view Str, const std::wstring_view Separators):
		m_View(Str),
		m_Separators(Separators)
	{
	}

	enum_tokens_t(const wchar_t* const Str, const std::wstring_view Separators):
		enum_tokens_t(std::wstring_view(Str), Separators)
	{
	}

private:
	[[nodiscard]]
	bool get(bool Reset, std::wstring_view& Value) const
	{
		if (Reset)
			m_Iterator = m_View.cbegin();
		else if (m_Iterator != m_View.cend())
			++m_Iterator;

		if (m_Iterator == m_View.cend())
			return false;

		m_Iterator = policy::extract(m_Iterator, m_View.cend(), m_Separators, Value);
		return true;
	}

	std::wstring m_Storage;
	std::wstring_view m_View;
	std::wstring_view m_Separators;
	mutable std::wstring_view::const_iterator m_Iterator{};
};

using enum_tokens = enum_tokens_t<detail::simple_policy>;

using with_quotes = detail::quotes_overrider;
using with_trim = detail::trimmer;

template<typename... args>
using enum_tokens_custom_t = enum_tokens_t<detail::custom_policy<args...>>;

template<typename... args>
using enum_tokens_with_quotes_t = enum_tokens_custom_t<with_quotes, args...>;

using enum_tokens_with_quotes = enum_tokens_with_quotes_t<>;

#endif // ENUM_TOKENS_HPP_0472A061_7BE9_4932_B0C4_26DC64B4AB45
