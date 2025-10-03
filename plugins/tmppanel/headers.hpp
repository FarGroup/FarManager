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


#endif // HEADERS_HPP_74234F4A_8C20_4AE2_A532_E93F003489D5
