#pragma once

#include <compiler.hpp>

#include <windows.h>
#include <shobjidl.h>
#include <winioctl.h>

#include <assert.h>
#include <process.h>
#include <time.h>

#include <memory>
#include <string>
#include <string_view>
#include <list>
#include <vector>
#include <set>
#include <map>
#include <stack>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <iterator>
#include <limits>
#include <numeric>
#include <optional>
#include <cmath>
#include <cstring>

WARNING_PUSH()
WARNING_DISABLE_MSC(5204) // 'type-name': class has virtual functions, but its trivial destructor is not virtual; instances of objects derived from this class may not be destructed correctly
WARNING_DISABLE_GCC("-Wsuggest-override")
WARNING_DISABLE_CLANG("-Weverything")

#include <basetyps.h>

#include "7z/h/CPP/7zip/Archive/IArchive.h"
#include "7z/h/CPP/7zip/IPassword.h"
#include "7z/h/CPP/7zip/ICoder.h"

WARNING_POP()

#include <plugin.hpp>
#include <farcolor.hpp>

inline namespace literals
{
	using namespace std::literals;
}
