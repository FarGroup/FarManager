#pragma once
#include "Rtl.Base.h"

#ifdef _WIN64
	#define THUNK_MAGIC 0xDEADBEAFDEADBEAF
#else
	#define THUNK_MAGIC 0xDDBBAACC
#endif

#define THUNK_BEGIN(_name, ...)

#define CreateClassThunk(_classname, _func, _retval) \
{ \
	typedef void (__stdcall _classname::*CLASSPROC)(); \
	CLASSPROC _fake_proc = (CLASSPROC)&_classname::_func; \
	_retval = CreateThunk (this, *(void**)&_fake_proc); \
}

#define CreateClassThunkCdecl(_classname, _func, _retval) \
{ \
	typedef void (__cdecl _classname::*CLASSPROC)(); \
	CLASSPROC _fake_proc = (CLASSPROC)&_classname::_func; \
	_retval = CreateThunkCdecl (this, *(void**)&_fake_proc); \
}

#define CreateClassThunkRegister(_classname, _func, _retval) \
{ \
	typedef void (__stdcall _classname::*CLASSPROC)(); \
	CLASSPROC _fake_proc = (CLASSPROC)&_classname::_func; \
	_retval = CreateThunkRegister(this, *(void**)&_fake_proc); \
}

extern byte *CreateThunkFastEx(void *obj, void *start);
extern byte *CreateThunkEx(void *obj, void *start, void *end);
extern byte* CreateThunk (void *pClass, void *pfnClassProc);
extern byte* CreateThunkCdecl (void *pClass, void *pfnClassProc);
extern byte* CreateThunkRegister(void *pClass, void *pClassProc);

extern void ReleaseThunk (byte *pThunk);
extern void ReleaseThunkEx (byte *pThunk);
extern void ReleaseThunkCdecl (byte *pThunk);


