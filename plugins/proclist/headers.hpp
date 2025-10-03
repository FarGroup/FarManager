#ifndef HEADERS_HPP_890CA261_D1CE_46A2_B78E_F3C8106A5AC5
#define HEADERS_HPP_890CA261_D1CE_46A2_B78E_F3C8106A5AC5
#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cwchar>
#include <cwctype>

using string = std::wstring;
using string_view = std::wstring_view;

inline namespace literals
{
	using namespace std::literals;
}


#ifndef _MSC_VER

#include <winsdkver.h>

#undef WINVER

#define WINVER WINVER_MAXVER

#endif

#define WIN32_NO_STATUS //exclude ntstatus.h macros from winnt.h
#include <windows.h>
#undef WIN32_NO_STATUS
#include <lmcons.h>
#include <ntstatus.h>
#include <sddl.h>
#include <unknwn.h>
#include <winternl.h>

#endif // HEADERS_HPP_890CA261_D1CE_46A2_B78E_F3C8106A5AC5
