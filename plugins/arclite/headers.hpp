#pragma once

#include <windows.h>
#include <shobjidl.h>
#include <winioctl.h>
#undef max

#include <assert.h>
#include <process.h>
#include <time.h>

#include <memory>
#include <string>
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
using namespace std;

#define INITGUID
#include <basetyps.h>
#include "CPP/7zip/Archive/IArchive.h"
#include "CPP/7zip/IPassword.h"

#include "plugin.hpp"
#include "farcolor.hpp"
