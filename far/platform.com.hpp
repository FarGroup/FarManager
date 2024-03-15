#ifndef PLATFORM_COM_HPP_4E1C5B1E_3366_45BB_A55B_AD2B1357CA7D
#define PLATFORM_COM_HPP_4E1C5B1E_3366_45BB_A55B_AD2B1357CA7D
#pragma once

/*
platform.com.hpp
*/
/*
Copyright © 2021 Far Group
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
#include "exception.hpp"

// Platform:

// Common:
#include "common/function_ref.hpp"
#include "common/preprocessor.hpp"
#include "common/source_location.hpp"

// External:

//----------------------------------------------------------------------------

namespace os::com
{
	class initialize
	{
	public:
		NONCOPYABLE(initialize);

		initialize();
		~initialize();

	private:
		const bool m_Initialised;
	};

	namespace detail
	{
		template<typename T>
		struct releaser
		{
			void operator()(T* Object) const
			{
				Object->Release();
			}
		};

		struct memory_releaser
		{
			void operator()(const void* Object) const;
		};
	}

	template<typename T>
	using ptr = std::unique_ptr<T, detail::releaser<T>>;

	template<typename T>
	using memory = std::unique_ptr<std::remove_pointer_t<T>, detail::memory_releaser>;

	class exception final: public far_exception
	{
	public:
		explicit exception(HRESULT const ErrorCode, string_view const Message, source_location const& Location = source_location::current()):
			far_exception(Message, false, Location)
		{
			Win32Error = ErrorCode;
		}
	};

	void invoke(function_ref<HRESULT()> Callable, string_view CallableName, source_location const& Location = source_location::current());

#define COM_INVOKE(Function, Args) \
	os::com::invoke([&]{ return Function Args; }, WIDE_SV_LITERAL(Function))

	string get_shell_name(string_view Path);

	string get_shell_filetype_description(string_view FileName);

	ptr<IFileIsInUse> create_file_is_in_use(const string& File);
}

#endif // PLATFORM_COM_HPP_4E1C5B1E_3366_45BB_A55B_AD2B1357CA7D
