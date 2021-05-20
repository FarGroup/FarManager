﻿#ifndef PLACEMENT_HPP_846A43B8_5BFF_4131_87E6_CFC3E6C44956
#define PLACEMENT_HPP_846A43B8_5BFF_4131_87E6_CFC3E6C44956
#pragma once

/*
placement.hpp
*/
/*
Copyright © 2017 Far Group
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

#include "preprocessor.hpp"

//----------------------------------------------------------------------------

namespace placement
{
	template<typename T, typename... args>
	auto& construct(T& Object, args&&... Args)
	{
		return *new(std::addressof(Object)) T{ FWD(Args)... };
	}

	template<typename T>
	void destruct(T& Object) noexcept
	{
		Object.~T();

#ifdef _DEBUG
		// To increase the chance of crash on use-after-delete
		std::memset(static_cast<void*>(&Object), 0xFE, sizeof(Object));
#endif
	}
}

#endif // PLACEMENT_HPP_846A43B8_5BFF_4131_87E6_CFC3E6C44956
