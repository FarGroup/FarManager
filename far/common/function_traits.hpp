#pragma once

/*
Copyright © 2014 Far Group
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

template <class F> struct return_type;
#define DEFINE_R_TYPE { typedef typename std::remove_const<typename std::remove_reference<R>::type>::type type; };
#if defined _MSC_VER && _MSC_VER < 1800
template <class R> struct return_type<R(*)()> DEFINE_R_TYPE
template <class R, class A0> struct return_type<R(*)(A0)> DEFINE_R_TYPE
template <class R, class A0, class A1> struct return_type<R(*)(A0, A1)> DEFINE_R_TYPE
template <class R, class A0, class A1, class A2> struct return_type<R(*)(A0, A1, A2)> DEFINE_R_TYPE
template <class R, class A0, class A1, class A2, class A3> struct return_type<R(*)(A0, A1, A2, A3)> DEFINE_R_TYPE
template <class R, class A0, class A1, class A2, class A3, class A4> struct return_type<R(*)(A0, A1, A2, A3, A4)> DEFINE_R_TYPE
template <class R, class A0, class A1, class A2, class A3, class A4, class A5> struct return_type<R(*)(A0, A1, A2, A3, A4, A5)> DEFINE_R_TYPE
#else
template <class R, class... A> struct return_type<R(*)(A...)> DEFINE_R_TYPE
#endif
#undef DEFINE_R_TYPE
#define FN_RETURN_TYPE(function_name) return_type<decltype(&function_name)>::type
