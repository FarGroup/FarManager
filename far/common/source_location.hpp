#ifndef SOURCE_LOCATION_HPP_BC428459_A5C0_4962_B141_9FF37A80C1BC
#define SOURCE_LOCATION_HPP_BC428459_A5C0_4962_B141_9FF37A80C1BC
#pragma once

/*
source_location.hpp
*/
/*
Copyright © 2024 Far Group
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

//----------------------------------------------------------------------------

// If things are not FUBARed by the committee, vendors will happily come to the rescue. 🤦
// For source_location::function_name() the standard suggests __func__, but does not enforce it.
// All major implementations decided to make it decorated, as in __FUNCSIG__ or __PRETTY_FUNCTION__,
// with return types, parameters, namespaces and other rubbish, bloating your messages, logs, and binaries.
// Only MSVC allows to customize it, so the only way to make it portable is to redefine the whole thing.

struct source_location
{
	[[nodiscard]] static consteval source_location current(
		char const* const File = __builtin_FILE(),
		char const* const Function = __builtin_FUNCTION(),
		unsigned const Line = __builtin_LINE()
		) noexcept
	{
		source_location Result;
		Result.m_File = File;
		Result.m_Function = Function;
		Result.m_Line = Line;
		return Result;
	}

	// If we have our own, we can make it a bit saner as well
	[[nodiscard]] constexpr source_location(
		char const* const File,
		char const* const Function,
		unsigned const Line
	) noexcept:
		m_File(File),
		m_Function(Function),
		m_Line(Line)
	{
	}

	[[nodiscard]] constexpr source_location() noexcept = default;

	[[nodiscard]] constexpr const char* file_name() const noexcept
	{
		return m_File;
	}

	[[nodiscard]] constexpr const char* function_name() const noexcept
	{
		return m_Function;
	}

	[[nodiscard]] constexpr unsigned line() const noexcept
	{
		return m_Line;
	}

	// column() is:
	// - handled differently by MSVC and Clang
	// - not supported at all by GCC
	// - useless in general
	// so not implemented.

private:
	const char* m_File{};
	const char* m_Function{};
	unsigned m_Line{};
};

#endif // SOURCE_LOCATION_HPP_BC428459_A5C0_4962_B141_9FF37A80C1BC
