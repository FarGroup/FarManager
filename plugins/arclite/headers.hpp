#pragma once

#define NOMINMAX
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
#include <cmath>
using namespace std;

#define INITGUID
#include <basetyps.h>
#include <CPP/7zip/Archive/IArchive.h>
#include <CPP/7zip/IPassword.h>
#include <CPP/7Zip/ICoder.h>

#include <plugin.hpp>
#include <farcolor.hpp>
