/*
exception.cpp
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

#include "headers.hpp"
#pragma hdrstop

#include "exception.hpp"
#include "imports.hpp"
#include "encoding.hpp"

error_codes::error_codes():
	Win32Error(GetLastError()),
	NtError(Imports().RtlGetLastNtStatus())
{
}

error_codes::error_codes(ignore):
	Win32Error(ERROR_SUCCESS),
	NtError(STATUS_SUCCESS)
{}


far_exception::far_exception(const string& Message, const char* Function, const char* File, int Line):
	exception_impl(Message, Function, File, Line),
	std::runtime_error(encoding::utf8::get_bytes(get_full_message()))
{
}


std::exception_ptr& GlobalExceptionPtr()
{
	static std::exception_ptr ExceptionPtr;
	return ExceptionPtr;
}

void StoreGlobalException()
{
	GlobalExceptionPtr() = std::current_exception();
}

void RethrowIfNeeded(std::exception_ptr& Ptr)
{
	if (Ptr)
	{
		std::rethrow_exception(std::exchange(Ptr, {}));
	}
}