﻿#ifndef SINGLETON_HPP_689EF327_41C5_4AB7_B9A6_CB5361D7B040
#define SINGLETON_HPP_689EF327_41C5_4AB7_B9A6_CB5361D7B040
#pragma once

/*
singleton.hpp
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

#include "noncopyable.hpp"

//----------------------------------------------------------------------------

template<typename type>
class singleton: noncopyable
{
public:
	[[nodiscard]]
	static type& instance()
	{
		static_assert(std::is_base_of_v<singleton, type>);

		static type Instance;

		assert(!Instance.Destroyed);

		return Instance;
	}

protected:
	using singleton_type = singleton<type>;

	singleton() = default;

	~singleton()
	{
		Destroyed = true;
	}

private:
	bool Destroyed{};
};

#define IMPLEMENTS_SINGLETON friend singleton_type

#endif // SINGLETON_HPP_689EF327_41C5_4AB7_B9A6_CB5361D7B040
