#pragma once

#include <compiler.hpp>

#include <windows.h>
#include <shobjidl.h>

#include <string>
#include <list>
#include <vector>
#include <map>
#include <iterator>

WARNING_PUSH()
WARNING_DISABLE_MSC(5204) // 'type-name': class has virtual functions, but its trivial destructor is not virtual; instances of objects derived from this class may not be destructed correctly
WARNING_DISABLE_GCC("-Wsuggest-override")
WARNING_DISABLE_CLANG("-Weverything")

#include <basetyps.h>

#include "../7z/h/CPP/7zip/Archive/IArchive.h"

WARNING_POP()

#include <plugin.hpp>

inline namespace literals
{
	using namespace std::literals;
}
