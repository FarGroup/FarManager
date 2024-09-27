#ifndef FORMAT_HPP_0FD1BABD_50A_4184_87C2_3C0EB2D444B3
#define FORMAT_HPP_0FD1BABD_50A_4184_87C2_3C0EB2D444B3

#pragma once

#include <compiler.hpp>

WARNING_PUSH(3)

WARNING_DISABLE_MSC(4267) // 'var' : conversion from 'size_t' to 'type', possible loss of data

WARNING_DISABLE_GCC("-Wctor-dtor-privacy")

WARNING_DISABLE_CLANG("-Weverything")

#define FMT_CONSTEVAL consteval
#define FMT_HAS_CONSTEVAL

#include <algorithm>

#include <fmt/format.h>
#include <fmt/xchar.h>

WARNING_POP()

// TODO: Unify with Far

#undef far
#undef FAR
#define FAR

namespace far
{
	template<typename... args>
	using format_string = fmt::wformat_string<args...>;

	template <typename... args>
	auto format(format_string<args...> const Format, args const&... Args)
	{
		return fmt::vformat(fmt::wstring_view(Format), fmt::make_wformat_args(Args...));
	}

	template<typename... args>
	auto vformat(std::wstring_view const Format, args const&... Args)
	{
		return fmt::vformat(fmt::wstring_view(Format), fmt::make_wformat_args(Args...));
	}
}

template<typename T>
auto str(const T& Value)
{
	return fmt::to_wstring(Value);
}

inline auto str(const void* Value)
{
	return far::format(L"0x{:0{}X}", reinterpret_cast<uintptr_t>(Value), sizeof(Value) * 2);
}

std::wstring str(const char*) = delete;
std::wstring str(const wchar_t*) = delete;
std::wstring str(std::string) = delete;
std::wstring str(std::wstring) = delete;
std::wstring str(std::string_view) = delete;
std::wstring str(std::wstring_view) = delete;

#endif // FORMAT_HPP_0FD1BABD_50A_4184_87C2_3C0EB2D444B3
