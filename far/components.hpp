﻿#ifndef COMPONENTS_HPP_5EB4061D_47B2_4941_8B57_FE405EBD3D83
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

#include "string_sort.hpp"

#include "common/enumerator.hpp"

namespace components
{
	class component
	{
	public:
		using info = std::pair<string, string>;
		using get_info = info(*)();

		explicit component(get_info getInfo);

	private:
		friend class components_list;

		get_info m_getInfo;
		component* m_next;
	};

	class components_list:public enumerator<components_list, component::get_info>
	{
		IMPLEMENTS_ENUMERATOR(components_list);

	public:
		void add(component* item);
		bool empty() const { return list != nullptr; }
		size_t size() const { return m_size; }

	private:
		friend components_list& GetComponentsList();

		bool get(bool Reset, value_type& value) const;

		components_list() = default;

		component* list{};
		component* ptr{};
		mutable component* enum_ptr{};
		size_t m_size{};
	};

	const std::map<string, string, string_sort::less_t>& GetComponentsInfo();
}

#endif // COMPONENTS_HPP_5EB4061D_47B2_4941_8B57_FE405EBD3D83
