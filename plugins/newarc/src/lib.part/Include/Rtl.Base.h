#pragma once
#include <windows.h>

#if defined(_MSC_VER)
#pragma warning(disable:4201) // nameless unions
#pragma warning(disable:4121) // alignment of a member was sensitive to packing (i don't care)
#endif

#include <Rtl.Types.h>
//#include <Rtl.Memory.h>
#include <Rtl.Strings.h>
#include <Rtl.Kernel.h>
#include <Rtl.Hook.h>
#include <Rtl.Options.h>
#include <Rtl.Thunks.h>
#include <Rtl.Misc.h>

#if defined(_MSC_VER)
#pragma comment(linker,"/merge:.CRT=.data")
#endif
