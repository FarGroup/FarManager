#ifndef FORMAT_HPP_0FD1BABD_50A_4184_87C2_3C0EB2D444B3
#define FORMAT_HPP_0FD1BABD_50A_4184_87C2_3C0EB2D444B3

#pragma once

#include <fmt/format.h>
#include <fmt/ostream.h>


// TODO: Unify with Far

template<typename F, typename... args>
auto format(F&& Format, args&&... Args)
{
	return fmt::format(Format, Args...);
}

// use FSTR or string_view instead of string literals
template<typename char_type, size_t N, typename... args>
auto format(const char_type(&Format)[N], args&&...) = delete;

#if 1
#define FSTR(str) FMT_STRING(str)
#else
#define FSTR(str) str ## sv
#endif

template<typename T>
auto str(const T& Value)
{
	return fmt::to_wstring(Value);
}

inline auto str(const void* Value)
{
	return format(FSTR(L"0x{0:0{1}X}"), reinterpret_cast<uintptr_t>(Value), sizeof(Value) * 2);
}

std::wstring str(const char*) = delete;
std::wstring str(const wchar_t*) = delete;
std::wstring str(std::string) = delete;
std::wstring str(std::wstring) = delete;
std::wstring str(std::string_view) = delete;
std::wstring str(std::wstring_view) = delete;

#endif // FORMAT_HPP_0FD1BABD_50A_4184_87C2_3C0EB2D444B3
