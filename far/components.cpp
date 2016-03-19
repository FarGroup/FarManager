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

#include "headers.hpp"
#pragma hdrstop

#include "components.hpp"

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

	components_list::components_list():
		list(),
		ptr(),
		enum_ptr(),
		m_size()
	{
	}

	void components_list::add(component* item)
	{
		if (!list)
		{
			list = item;
			ptr = list;
		}
		else
		{
			ptr->m_next = item;
		}
		ptr = item;

		++m_size;
	}

	bool components_list::get(size_t index, value_type& value)
	{
		if (!index)
			enum_ptr = list;

		if (enum_ptr)
		{
			value = enum_ptr->m_getInfo;
			enum_ptr = enum_ptr->m_next;
			return true;
		}
		return false;
	}

	std::set<string>& GetComponentsInfo()
	{
		static FN_RETURN_TYPE(GetComponentsInfo) sList;
		if (sList.empty())
		{
			auto& ComponentsList = GetComponentsList();
			std::transform(ALL_RANGE(ComponentsList), std::inserter(sList, sList.end()), [](const auto& i)
			{
				return i();
			});
		}
		return sList;
	}
}
