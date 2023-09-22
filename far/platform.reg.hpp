﻿#ifndef PLATFORM_REG_HPP_A1C836DB_556E_41F8_B04C_AF159E265315
#define PLATFORM_REG_HPP_A1C836DB_556E_41F8_B04C_AF159E265315
#pragma once

/*
platform.reg.hpp

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

// Internal:

// Platform:

// Common:
#include "common/enumerator.hpp"
#include "common/noncopyable.hpp"
#include "common/type_traits.hpp"

// External:

//----------------------------------------------------------------------------

namespace os::reg
{
	class value;
	class enum_key;
	class enum_value;

	class key
	{
		friend enum_key;
		friend enum_value;

	public:
		key() = default;

		static const key classes_root;
		static const key current_user;
		static const key local_machine;

		[[nodiscard]]
		key open(string_view SubKey, DWORD SamDesired) const;

		void close();

		[[nodiscard]]
		HKEY native_handle() const;

		[[nodiscard]]
		enum_key enum_keys() const;

		[[nodiscard]]
		enum_value enum_values() const;

		[[nodiscard]]
		bool exits(string_view Name) const;

		[[nodiscard]]
		bool get(string_view Name, string& Value) const;

		[[nodiscard]]
		bool get(string_view Name, unsigned int& Value) const;

		template<class T>
		[[nodiscard]]
		std::optional<T> get(string_view const SubKeyName, string_view const Name) const
		{
			static_assert(is_one_of_v<T, string, unsigned int>);

			const auto SubKey = open(SubKeyName, KEY_QUERY_VALUE);
			if (!SubKey)
				return {};

			if (T Value; SubKey.get(Name, Value))
				return Value;

			return {};
		}

		[[nodiscard]]
		explicit operator bool() const;

	private:
		explicit key(HKEY Key);

		struct hkey_deleter
		{
			void operator()(HKEY Key) const noexcept;
		};

		[[nodiscard]]
		bool enum_keys_impl(size_t Index, string& Name) const;

		[[nodiscard]]
		bool enum_values_impl(size_t Index, value& Value) const;

		std::unique_ptr<std::remove_pointer_t<HKEY>, hkey_deleter> m_Key;
	};

	class value
	{
	public:
		[[nodiscard]]
		const string& name() const;

		[[nodiscard]]
		DWORD type() const;

		[[nodiscard]]
		string get_string() const;

		[[nodiscard]]
		unsigned int get_unsigned() const;

	private:
		friend class key;

		string m_Name;
		DWORD m_Type{ REG_NONE };
		const key* m_Key{};
	};

	class [[nodiscard]] enum_key: noncopyable, public enumerator<enum_key, string>
	{
		IMPLEMENTS_ENUMERATOR(enum_key);

	public:
		explicit enum_key(const key& Key);

	private:
		[[nodiscard]]
		bool get(bool Reset, value_type& Value) const;

		const key* m_KeyRef{};
		mutable size_t m_Index{};
	};

	class [[nodiscard]] enum_value: noncopyable, public enumerator<enum_value, value>
	{
		IMPLEMENTS_ENUMERATOR(enum_value);

	public:
		explicit enum_value(const key& Key);

	private:
		[[nodiscard]]
		bool get(bool Reset, value_type& Value) const;

		const key* m_KeyRef{};
		mutable size_t m_Index{};
	};
}

#endif // PLATFORM_REG_HPP_A1C836DB_556E_41F8_B04C_AF159E265315
