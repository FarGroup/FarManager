#ifndef COMPONENTS_HPP_5EB4061D_47B2_4941_8B57_FE405EBD3D83
#define COMPONENTS_HPP_5EB4061D_47B2_4941_8B57_FE405EBD3D83
#pragma once

/*
components.hpp

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

// Internal:
#include "string_sort.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

namespace components
{
	using components_map = std::map<string_view, string, string_sort::less_t>;
	using info = components_map::value_type;

	class component
	{
	public:
		using get_info = info(*)();

		explicit component(get_info GetInfo);

	private:
		friend class components_list;

		get_info m_GetInfo;
		component* m_Next{};
	};

	const components_map& GetComponentsInfo();
}

#endif // COMPONENTS_HPP_5EB4061D_47B2_4941_8B57_FE405EBD3D83
