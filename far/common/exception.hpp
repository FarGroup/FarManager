#ifndef EXCEPTION_HPP_82CEF198_5774_4AA8_A946_0474EA37028E
#define EXCEPTION_HPP_82CEF198_5774_4AA8_A946_0474EA37028E
#pragma once

/*
exception.hpp
*/
/*
Copyright © 2025 Far Group
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
#include "source_location.hpp"

#include <stdexcept>
#include <string>
#include <string_view>

//----------------------------------------------------------------------------

[[noreturn]]
inline void throw_exception(std::string_view const Message, source_location const& Location = source_location::current())
{
#ifdef FAR_USE_INTERNALS
	[[noreturn]]
	void throw_far_exception(std::string_view /*Message*/, source_location const& /*Location*/);

	throw_far_exception(Message, Location);
#else
	(void)Location;
	throw std::runtime_error(std::string(Message));
#endif
}

#endif // EXCEPTION_HPP_82CEF198_5774_4AA8_A946_0474EA37028E
