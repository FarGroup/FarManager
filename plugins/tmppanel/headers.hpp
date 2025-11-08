#ifndef HEADERS_HPP_74234F4A_8C20_4AE2_A532_E93F003489D5
#define HEADERS_HPP_74234F4A_8C20_4AE2_A532_E93F003489D5
#pragma once

#include <algorithm>
#include <iterator>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include <cstdlib>
#include <cwctype>

#include <plugin.hpp>
#include <PluginSettings.hpp>
#include <DlgBuilder.hpp>

#include <preprocessor.hpp>

using string = std::wstring;
using string_view = std::wstring_view;

inline namespace literals
{
	using namespace std::literals;
}

inline FarStandardFunctions FSF;

struct [[nodiscard]] string_comparer_icase
{
	[[nodiscard]]
	size_t operator()(const string_view Str) const
	{
		string UpStr(Str);
		FSF.LUpperBuf(const_cast<wchar_t*>(UpStr.c_str()), UpStr.size());
		return std::hash<string>{}(UpStr);
	}

	[[nodiscard]]
	bool operator()(const string_view Str1, const string_view Str2) const
	{
		if (Str1 == Str2)
			return true;
		if (Str1.size() != Str2.size())
			return false;
		return std::equal(Str1.begin(), Str1.end(), Str2.begin(),
			[](wchar_t c1, wchar_t c2)
			{
				if (c1 == c2)
					return true;
				FSF.LUpperBuf(&c1, 1);
				FSF.LUpperBuf(&c2, 1);
				return c1 == c2;
			});
	}
};

template<typename T>
using unordered_string_map_icase = std::unordered_map<string, T, string_comparer_icase, string_comparer_icase>;

#endif // HEADERS_HPP_74234F4A_8C20_4AE2_A532_E93F003489D5
