#ifndef __LOCAL_HPP__
#define __LOCAL_HPP__
/*
local.hpp

Сравнение без учета регистра, преобразование регистра
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

inline int __cdecl StrLength(const wchar_t *str) { return (int) wcslen(str); }

inline int IsSpace(wchar_t x) { return x==L' '  || x==L'\t';  }
inline int IsEol(wchar_t x)   { return x==L'\r' || x==L'\n'; }
inline int IsSpaceOrEos(wchar_t x) { return x==0 || x==L' '  || x==L'\t';  }

const wchar_t * __cdecl StrStrI(const wchar_t *str1, const wchar_t *str2);
const wchar_t * __cdecl RevStrStrI(const wchar_t *str1, const wchar_t *str2);

void __cdecl UpperBuf(wchar_t *Buf, int Length);
void __cdecl LowerBuf(wchar_t *Buf, int Length);
void __cdecl StrUpper(wchar_t *s1);
void __cdecl StrLower(wchar_t *s1);

wchar_t __cdecl Upper(wchar_t Ch);
wchar_t __cdecl Lower(wchar_t Ch);
int __cdecl StrCmpNI(const wchar_t *s1, const wchar_t *s2, int n);
int __cdecl StrCmpI(const wchar_t *s1, const wchar_t *s2);
int __cdecl IsLower(wchar_t Ch);
int __cdecl IsUpper(wchar_t Ch);
int __cdecl IsAlpha(wchar_t Ch);
int __cdecl IsAlphaNum(wchar_t Ch);

int __cdecl StrCmp(const wchar_t *s1, const wchar_t *s2);
int __cdecl StrCmpN(const wchar_t *s1, const wchar_t *s2, int n);
int __cdecl NumStrCmp(const wchar_t *s1, const wchar_t *s2);
int __cdecl NumStrCmpI(const wchar_t *s1, const wchar_t *s2);

#endif //__LOCAL_HPP__
