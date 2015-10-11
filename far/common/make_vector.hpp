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

#if defined _MSC_VER && _MSC_VER < 1800
template<class T, class T1>
std::vector<T> make_vector(T1&& a1)
{
	return std::vector<T>(1, std::forward<T1>(a1));
}

#define MAKE_VECTOR_VTE(TYPENAME_LIST, ARG_LIST, REF_ARG_LIST, FWD_ARG_LIST) \
template<class T, TYPENAME_LIST, VTE_TYPENAME(last)> \
std::vector<T> make_vector(REF_ARG_LIST, VTE_REF_ARG(last)) \
{ \
	auto v = make_vector<T>(FWD_ARG_LIST); \
	v.emplace_back(VTE_FWD_ARG(last)); \
	return v; \
}

#include "variadic_emulation_helpers_begin.hpp"
VTE_GENERATE(MAKE_VECTOR_VTE)
#include "variadic_emulation_helpers_end.hpp"

#undef MAKE_VECTOR_VTE

#else
template<class T, class... Args>
std::vector<T> make_vector(Args&&... args)
{
	return std::vector<T>{ std::forward<Args>(args)... };
}
#endif
