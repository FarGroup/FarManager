#ifndef TRACER_HPP_AD7B9307_ECFD_46FC_B001_E48C9B89DE64
#define TRACER_HPP_AD7B9307_ECFD_46FC_B001_E48C9B89DE64
#pragma once

/*
tracer.hpp
*/
/*
Copyright © 2016 Far Group
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
#include "common/function_ref.hpp"
#include "common/noncopyable.hpp"
#include "common/range.hpp"

// External:

//----------------------------------------------------------------------------

class tracer: noncopyable
{
public:
	static std::vector<DWORD64> get(string_view Module, const EXCEPTION_POINTERS& Pointers, HANDLE ThreadHandle);
	static void get_symbols(string_view Module, span<DWORD64 const> Trace, function_ref<void(string&& Line)> Consumer);
	static void get_symbol(string_view Module, const void* Ptr, string& Address, string& Name, string& Source);

	class with_symbols
	{
	public:
		NONCOPYABLE(with_symbols);

		with_symbols(string_view const Module)
		{
			sym_initialise(Module);
		}

		~with_symbols()
		{
			sym_cleanup();
		}
	};

private:
	static void sym_initialise(string_view Module);
	static void sym_cleanup();
};

#endif // TRACER_HPP_AD7B9307_ECFD_46FC_B001_E48C9B89DE64
