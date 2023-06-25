#pragma once

#include <compiler.hpp>

#include <string>
#include <list>
#include <vector>
#include <map>
#include <iterator>

#include <initguid.h>

WARNING_PUSH()
WARNING_DISABLE_MSC(5204) // 'type-name': class has virtual functions, but its trivial destructor is not virtual; instances of objects derived from this class may not be destructed correctly
WARNING_DISABLE_GCC("-Wsuggest-override")
WARNING_DISABLE_CLANG("-Weverything")

#include "../7z/h/CPP/7zip/Archive/IArchive.h"

WARNING_POP()

inline namespace literals
{
	using namespace std::literals;
}
