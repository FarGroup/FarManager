/*
platform.reg.cpp

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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "platform.reg.hpp"

// Internal:
#include "exception.hpp"

// Platform:

// Common:
#include "common/algorithm.hpp"
#include "common/bytes_view.hpp"
#include "common/string_utils.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

namespace
{
	constexpr auto succeeded(LSTATUS const Code)
	{
		return Code == ERROR_SUCCESS;
	}

	constexpr auto failed(LSTATUS const Code)
	{
		return !succeeded(Code);
	}

	constexpr bool is_string_type(DWORD Type)
	{
		return Type == REG_SZ || Type == REG_EXPAND_SZ || Type == REG_MULTI_SZ;
	}

	LSTATUS query_value(const HKEY Key, const string_view Name, DWORD* const Type, void* const Data, size_t* const Size)
	{
		DWORD dwSize = Size? static_cast<DWORD>(*Size) : 0;
		const auto Result = RegQueryValueEx(Key, null_terminated(Name).c_str(), nullptr, Type, static_cast<BYTE*>(Data), Size? &dwSize : nullptr);
		if (Size)
		{
			assert(Key != HKEY_PERFORMANCE_DATA);
			*Size = dwSize;
		}
		return Result;
	}

	LSTATUS query_value(const HKEY Key, const string_view Name, DWORD& Type, bytes& Value)
	{
		size_t Size = 0;
		if (const auto Result = query_value(Key, Name, nullptr, nullptr, &Size); failed(Result))
			return Result;

		Value.resize(Size);
		return query_value(Key, Name, &Type, Value.data(), &Size);
	}
}

namespace os::reg
{
	const key key::classes_root = key(HKEY_CLASSES_ROOT);
	const key key::current_user = key(HKEY_CURRENT_USER);
	const key key::local_machine = key(HKEY_LOCAL_MACHINE);

	exception::exception(error const& Error, source_location const& Location):
		far_exception({{ static_cast<DWORD>(Error.Code), STATUS_SUCCESS }, Error.What }, Location)
	{}

	result<key> key::open(string_view const SubKeyName, DWORD const SamDesired) const
	{
		HKEY HKey;
		if (const auto Result = RegOpenKeyEx(native_handle(), null_terminated(SubKeyName).c_str(), 0, SamDesired, &HKey); failed(Result))
			return error{ far::format(L"RegOpenKeyEx({})"sv, SubKeyName), Result };

		key Result;
		Result.m_Key.reset(HKey);
		return Result;
	}

	void key::close()
	{
		m_Key.reset();
	}

	HKEY key::native_handle() const
	{
		return m_Key.get();
	}

	enum_key key::enum_keys() const
	{
		return enum_key(*this);
	}

	enum_value key::enum_values() const
	{
		return enum_value(*this);
	}

	bool key::enum_keys_impl(size_t Index, string& Name) const
	{
		return detail::ApiDynamicStringReceiver(Name, [&](std::span<wchar_t> const Buffer)
		{
			auto RetSize = static_cast<DWORD>(Buffer.size());
			switch (const auto Result = RegEnumKeyEx(native_handle(), static_cast<DWORD>(Index), Buffer.data(), &RetSize, {}, {}, {}, {}))
			{
			case ERROR_SUCCESS:
				return RetSize;

			case ERROR_MORE_DATA:
				// We don't know how much
				return RetSize * 2;

			case ERROR_NO_MORE_ITEMS:
				return DWORD{};

			default:
				throw exception({ L"RegEnumKeyEx"s, Result });
			}
		});
	}

	bool key::enum_values_impl(size_t Index, value& Value) const
	{
		if (!detail::ApiDynamicStringReceiver(Value.m_Name, [&](std::span<wchar_t> const Buffer)
		{
			auto RetSize = static_cast<DWORD>(Buffer.size());
			switch (const auto Result = RegEnumValue(native_handle(), static_cast<DWORD>(Index), Buffer.data(), &RetSize, {}, &Value.m_Type, {}, {}))
			{
			case ERROR_SUCCESS:
				return RetSize;

			case ERROR_MORE_DATA:
				// We don't know how much
				return RetSize * 2;

			case ERROR_NO_MORE_ITEMS:
				return DWORD{};

			default:
				throw exception({ L"RegEnumValue"s, Result });
			}
		}))
			return false;

		Value.m_Key = this;
		return true;
	}

	bool key::exits(const string_view Name) const
	{
		return succeeded(query_value(native_handle(), Name, {}, {}, {}));
	}

	result<string> key::get_string(const string_view Name) const
	{
		bytes Buffer;
		DWORD Type;
		if (const auto Result = query_value(native_handle(), Name, Type, Buffer); failed(Result))
			return error{ far::format(L"query_value({})"sv, Name), Result };

		if (!is_string_type(Type))
			return error{ far::format(L"Bad value type: {}, expected REG[_EXPAND|_MULTI]_SZ"sv, Type) };

		const auto Data = std::bit_cast<const wchar_t*>(Buffer.data());
		const auto Size = Buffer.size() / sizeof(wchar_t);
		const auto IsNullTerminated = Data[Size - 1] == L'\0';
		return string(Data, Size - IsNullTerminated);
	}

	result<string> key::get_string(string_view const SubKeyName, string_view const Name) const
	{
		const auto SubKey = open(SubKeyName, KEY_QUERY_VALUE);
		if (!SubKey)
			return SubKey.error();

		return SubKey->get_string(Name);
	}

	result<uint32_t> key::get_dword(const string_view Name) const
	{
		bytes Buffer;
		DWORD Type;
		if (const auto Result = query_value(native_handle(), Name, Type, Buffer); failed(Result))
			return error{ far::format(L"query_value({})"sv, Name), Result };

		if (Type != REG_DWORD)
			return error{ far::format(L"Bad value type: {}, expected REG_DWORD"sv, Type) };

		unsigned Value = 0;
		const auto ValueBytes = edit_bytes(Value);
		std::copy_n(Buffer.cbegin(), std::min(Buffer.size(), ValueBytes.size()), ValueBytes.begin());
		return Value;
	}

	result<uint32_t> key::get_dword(string_view const SubKeyName, string_view const Name) const
	{
		const auto SubKey = open(SubKeyName, KEY_QUERY_VALUE);
		if (!SubKey)
			return SubKey.error();

		return SubKey->get_dword(Name);
	}

	key::key(HKEY Key):
		m_Key(Key)
	{
	}

	void key::hkey_deleter::operator()(HKEY Key) const noexcept
	{
		if (any_of(Key, HKEY_CLASSES_ROOT, HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE))
			return;

		RegCloseKey(Key);
	}

	//-------------------------------------------------------------------------

	const string& value::name() const
	{
		return m_Name;
	}

	DWORD value::type() const
	{
		return m_Type;
	}

	string value::get_string() const
	{
		return *m_Key->get_string(m_Name);
	}

	uint32_t value::get_dword() const
	{
		return *m_Key->get_dword(m_Name);
	}

	//-------------------------------------------------------------------------

	enum_key::enum_key(const key& Key):
		m_KeyRef(&Key)
	{
	}

	bool enum_key::get(bool Reset, value_type& Value) const
	{
		if (Reset)
			m_Index = 0;

		return m_KeyRef->enum_keys_impl(m_Index++, Value);
	}

	//-------------------------------------------------------------------------

	enum_value::enum_value(const key& Key):
		m_KeyRef(&Key)
	{
	}

	bool enum_value::get(bool Reset, value_type& Value) const
	{
		if (Reset)
			m_Index = 0;

		return m_KeyRef->enum_values_impl(m_Index++, Value);
	}
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("platform.reg")
{
	{
		REQUIRE(!os::reg::key::current_user.enum_keys().empty());
	}

	{
		const auto Key = os::reg::key::current_user.open(L"SOFTWARE"sv, KEY_ENUMERATE_SUB_KEYS);
		REQUIRE(!Key->enum_keys().empty());
	}

	{
		const auto Key = os::reg::key::local_machine.open(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"sv, KEY_QUERY_VALUE);
		REQUIRE(!Key->enum_values().empty());

		const auto Value = Key->get_string(L"Shell"sv);
		REQUIRE(!Value->empty());
	}
}
#endif
