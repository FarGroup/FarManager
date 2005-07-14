#pragma once
#include "Rtl.Base.h"

#define CreateClassThunk(_classname, _func, _retval) \
{ \
	typedef void (__stdcall _classname::*CLASSPROC)(); \
	CLASSPROC _fake_proc = (CLASSPROC)&_classname::_func; \
	_retval = CreateThunk (this, *(void**)&_fake_proc); \
}


#define CreateClassThunkEx(_classptr, _classname, _func, _retval) \
{ \
	typedef void (__stdcall _classname::*CLASSPROC)(); \
	CLASSPROC _fake_proc = (CLASSPROC)&_classname::_func; \
	_retval = CreateThunk (_classptr, *(void**)&_fake_proc); \
}


#define CreateClassThunkCdecl(_classname, _func, _retval) \
{ \
	typedef void (__cdecl _classname::*CLASSPROC)(); \
	CLASSPROC _fake_proc = (CLASSPROC)&_classname::_func; \
	_retval = CreateThunkCdecl (this, *(void**)&_fake_proc); \
}


#define CreateClassThunkExCdecl (_classptr, _classname, _func, _retval) \
{ \
	typedef void (__cdecl _classname::*CLASSPROC)(); \
	CLASSPROC _fake_proc = (CLASSPROC)&_classname::_func; \
	_retval = CreateThunkCdecl (_classptr, *(void**)&_fake_proc); \
}


extern byte* CreateThunk (void *pClass, void *pfnClassProc);
extern byte* CreateThunkCdecl (void *pClass, void *pfnClassProc);

extern void ReleaseThunk (byte *pThunk);
extern void ReleaseThunkCdecl (byte *pThunk);