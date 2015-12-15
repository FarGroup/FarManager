#ifndef RANGE_FOR_HPP_ED9508AE_71CC_4F86_B802_47B026B392FD
#define RANGE_FOR_HPP_ED9508AE_71CC_4F86_B802_47B026B392FD
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

// C++11-like range-based for
#if COMPILER == C_CL && VS_OLDER_THAN(VS_2012)
#define DECORATED(name) DETAIL_RANGE_FOR_EMULATION_ ## name ## _
#define f_container DECORATED(container)
#define f_stop DECORATED(stop)
#define f_it DECORATED(it)
#define f_stop_it DECORATED(stop_it)
#define FOR(i, c) \
	if (bool f_stop = false); \
	else for (auto&& f_container = c; !f_stop; f_stop = true) \
	for (auto f_it = std::begin(f_container), e = false? f_it : std::end(f_container); f_it != e && !f_stop; ++f_it) \
	if (bool f_stop_it = (f_stop = true) == false); \
	else for (i = *f_it; !f_stop_it; ((f_stop_it = true) != false), f_stop = false)
// { body }
#undef f_stop_it
#undef f_it
#undef f_stop
#undef f_container
#undef DECORATED
#else
#define FOR(i, c) for(i: c)
#endif

#endif // RANGE_FOR_HPP_ED9508AE_71CC_4F86_B802_47B026B392FD
