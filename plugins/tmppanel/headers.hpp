#ifndef HEADERS_HPP_74234F4A_8C20_4AE2_A532_E93F003489D5
#define HEADERS_HPP_74234F4A_8C20_4AE2_A532_E93F003489D5
#pragma once

#ifdef __GNUC__
// Current implementation of wcschr etc. in gcc removes const from returned pointer. Issue has been opened since 2007.
// These semi-magical defines and appropriate overloads in cpp.hpp are intended to fix this madness.

// Force C version to return const
#undef _CONST_RETURN
#define _CONST_RETURN const
// Disable broken inline overloads
#define __CORRECT_ISO_CPP_WCHAR_H_PROTO
#endif

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
