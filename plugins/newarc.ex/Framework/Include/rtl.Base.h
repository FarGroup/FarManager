#pragma once

#if (defined(__GNUC__)) || (defined(_MSC_VER) && _MSC_VER < 1600) 
#define nullptr NULL 
#endif

#include <windows.h>
#include <tchar.h>

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#pragma warning(disable:4201) // nameless unions
#pragma warning(disable:4121) // alignment of a member was sensitive to packing (i don't care)

#include <Rtl.Strings.h>
#include <Rtl.Kernel.h>
#include <Rtl.Hook.h>
#include <Rtl.Options.h>
#include <Rtl.Thunks.h>
#include <Rtl.Misc.h>

//#pragma comment(linker,"/merge:.CRT=.data")
