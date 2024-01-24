#ifndef ENUMERATE_HPP_E49903DD_C3ED_4C17_B101_A68582FB7E8C
#define ENUMERATE_HPP_E49903DD_C3ED_4C17_B101_A68582FB7E8C
#pragma once

/*
enumerate.hpp
*/
/*
Copyright © 2019 Far Group
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

#include "zip.hpp"

#include <ranges>

//----------------------------------------------------------------------------

auto enumerate(auto&& Container)
{
	// 0u instead of 0uz below because iota's iterator is considered "input_iterator" only when its difference_type is "integral",
	// and for iota(size_t) on x64 it is __int128, which is not considered "integral" on MSVC.
	// This is as retarded as it gets.
	return zip(FWD(Container), std::views::iota(0u));
}

#endif // ENUMERATE_HPP_E49903DD_C3ED_4C17_B101_A68582FB7E8C
