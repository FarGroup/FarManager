#ifndef TINYXML_HPP_268E08DA_CDE4_4ADA_B082_64745EE1D62E
#define TINYXML_HPP_268E08DA_CDE4_4ADA_B082_64745EE1D62E
#pragma once

/*
tinyxml.hpp

tinyxml wrapper

*/
/*
Copyright © 2011 Far Group
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

// Platform:

// Common:
#include "common/enumerator.hpp"
#include "common/preprocessor.hpp"

// External:

namespace tinyxml_impl
{
WARNING_PUSH()

WARNING_DISABLE_GCC("-Wsuggest-override")
WARNING_DISABLE_GCC("-Wtype-limits")
WARNING_DISABLE_GCC("-Wzero-as-null-pointer-constant")

WARNING_DISABLE_CLANG("-Weverything")

#include "thirdparty/tinyxml2/tinyxml2.h"

WARNING_POP()
}

namespace tinyxml = tinyxml_impl::tinyxml2;

namespace xml
{
	class [[nodiscard]] enum_nodes: public enumerator<enum_nodes, const tinyxml::XMLElement*, true>
	{
		IMPLEMENTS_ENUMERATOR(enum_nodes);

	public:
		NONCOPYABLE(enum_nodes);

		enum_nodes(const tinyxml::XMLNode& Base, const char* Name);
		enum_nodes(tinyxml::XMLHandle Base, const char* Name);

	private:
		[[nodiscard, maybe_unused]]
		bool get(bool Reset, value_type& value) const;

		const char* m_name;
		const tinyxml::XMLNode* m_base;
	};
}

#endif // TINYXML_HPP_268E08DA_CDE4_4ADA_B082_64745EE1D62E
