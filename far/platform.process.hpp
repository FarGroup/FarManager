﻿#ifndef PLATFORM_PROCESS_HPP_234140CB_C857_40CF_901D_A10C5EBEA85B
#define PLATFORM_PROCESS_HPP_234140CB_C857_40CF_901D_A10C5EBEA85B
#pragma once

/*
platform.process.hpp

*/
/*
Copyright © 2020 Far Group
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

// External:

//----------------------------------------------------------------------------

namespace os::process
{
	enum class image_type
	{
		unknown,
		console,
		graphical,
	};

	image_type get_process_subsystem(HANDLE Process);

	size_t enumerate_rm_processes(const string& Filename, DWORD& Reasons, function_ref<bool(string&&)> Handler);
}

#endif // PLATFORM_PROCESS_HPP_234140CB_C857_40CF_901D_A10C5EBEA85B
