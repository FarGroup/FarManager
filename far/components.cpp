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

// Self:
#include "components.hpp"

// Internal:

// Platform:

// Common:
#include "common/enumerator.hpp"
#include "common/function_traits.hpp"
#include "common/preprocessor.hpp"
#include "common/singleton.hpp"

// External:

//----------------------------------------------------------------------------

namespace components
{
	class [[nodiscard]] components_list : public enumerator<components_list, component::get_info>, public singleton<components_list>
	{
		IMPLEMENTS_ENUMERATOR(components_list);
		IMPLEMENTS_SINGLETON;

	public:
		void add(component* item)
		{
			item->m_Next = std::exchange(m_First, item);
		}

	private:
		components_list() = default;
		~components_list() = default;

		[[nodiscard]]
		bool get(bool Reset, value_type& Value) const
		{
			if (Reset)
				m_Iterator = m_First;

			if (!m_Iterator)
				return false;

			Value = m_Iterator->m_GetInfo;
			m_Iterator = m_Iterator->m_Next;
			return true;
		}

		component* m_First{};
		mutable component* m_Iterator{};
	};

	component::component(get_info GetInfo):
		m_GetInfo(GetInfo)
	{
		components_list::instance().add(this);
	}

	const components_map& GetComponentsInfo()
	{
		static const auto sList = []
		{
			FN_RETURN_TYPE(GetComponentsInfo) Result;
			std::ranges::transform(components_list::instance(), std::inserter(Result, Result.end()), [](const auto& i)
			{
				return i();
			});
			return Result;
		}();

		return sList;
	}
}
