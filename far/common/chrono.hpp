#ifndef CHRONO_HPP_D4A71D62_47D4_45B1_B667_84D6E1E31A14
#define CHRONO_HPP_D4A71D62_47D4_45B1_B667_84D6E1E31A14
#pragma once

/*
chrono.hpp
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

//----------------------------------------------------------------------------

template<typename... tuple_types>
class split_duration: public std::tuple<tuple_types...>
{
public:
	template<typename duration_type>
	explicit split_duration(duration_type Duration)
	{
		(..., (set_and_chop<tuple_types>(Duration)));
	}

	template<typename type>
	[[nodiscard]]
	type& get()
	{
		return std::get<type>(tuple_cast(*this));
	}

	template<typename type>
	[[nodiscard]]
	const type& get() const
	{
		return std::get<type>(tuple_cast(*this));
	}

private:
	template<typename self_type>
	static auto& tuple_cast(self_type& Self)
	{
		// This idiotic cast to std::tuple is for clang
		using tuple_type = std::tuple<tuple_types...>;
		using result_type = std::conditional_t<std::is_const_v<self_type>, const tuple_type, tuple_type>;

		return static_cast<result_type&>(Self);
	}

	template<typename cast_type, typename duration_type>
	void set_and_chop(duration_type& Duration)
	{
		auto& Element = get<cast_type>();
		Element = std::chrono::duration_cast<cast_type>(Duration);
		Duration -= Element;
	}
};

namespace chrono
{
	using days = std::chrono::duration<int, std::ratio_multiply<std::ratio<24>, std::chrono::hours::period>>;
}

inline namespace literals
{
	[[nodiscard]]
	constexpr auto operator"" _d(unsigned long long const Value) noexcept
	{
		return chrono::days(Value);
	}
}

#endif // CHRONO_HPP_D4A71D62_47D4_45B1_B667_84D6E1E31A14
