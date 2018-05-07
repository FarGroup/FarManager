/*
components.cpp

static list of third-party components
*/
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

#include "components.hpp"

#include "common/function_traits.hpp"

namespace components
{
	components_list& GetComponentsList()
	{
		static components_list sList;
		return sList;
	}

	component::component(get_info getInfo):
		m_getInfo(getInfo),
		m_next()
	{
		GetComponentsList().add(this);
	}

	void components_list::add(component* item)
	{
		if (!list)
		{
			list = item;
		}
		else
		{
			ptr->m_next = item;
		}
		ptr = item;

		++m_size;
	}

	bool components_list::get(bool Reset, value_type& value) const
	{
		if (Reset)
			enum_ptr = list;

		if (!enum_ptr)
			return false;

		value = enum_ptr->m_getInfo;
		enum_ptr = enum_ptr->m_next;
		return true;
	}

	const std::map<string, string>& GetComponentsInfo()
	{
		static const auto sList = []
		{
			FN_RETURN_TYPE(GetComponentsInfo) Result;
			const auto& ComponentsList = GetComponentsList();
			std::transform(ALL_CONST_RANGE(ComponentsList), std::inserter(Result, Result.end()), [](const auto& i)
			{
				return i();
			});
			return Result;
		}();

		return sList;
	}
}
