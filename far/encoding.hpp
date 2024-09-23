#ifndef ENCODING_HPP_44AE7032_AF79_4A6F_A2ED_529BC1A38758
#define ENCODING_HPP_44AE7032_AF79_4A6F_A2ED_529BC1A38758
#pragma once

/*
encoding.hpp

Работа с кодовыми страницами
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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
#include "codepage.hpp"

// Platform:

// Common:
#include "common/bytes_view.hpp"
#include "common/utility.hpp"

// External:

//----------------------------------------------------------------------------

namespace encoding
{
	enum class is_utf8
	{
		no,
		yes,
		yes_ascii
	};

	class diagnostics
	{
	public:
		enum: unsigned
		{
			no_translation = 0_bit,
			not_enough_data = 1_bit,

			all = ~0u
		};

		explicit diagnostics(unsigned Diagnostics = all);

		void clear();

		unsigned EnabledDiagnostics;
		std::optional<size_t> ErrorPosition;
		size_t PartialInput{};
		size_t PartialOutput{};

		void set_is_utf8(is_utf8 IsUtf8);
		is_utf8 get_is_utf8() const;

	private:
		is_utf8 m_IsUtf8{is_utf8::yes_ascii};
	};

	[[nodiscard]] size_t get_bytes(uintptr_t Codepage, string_view Str, std::span<char> Buffer, diagnostics* Diagnostics = {});
	void get_bytes(uintptr_t Codepage, string_view Str, std::string& Buffer, diagnostics* Diagnostics = {});
	[[nodiscard]] std::string get_bytes(uintptr_t Codepage, string_view Str, diagnostics* Diagnostics = {});

	[[nodiscard]] size_t get_bytes_count(uintptr_t Codepage, string_view Str, diagnostics* Diagnostics = {});

	//-------------------------------------------------------------------------

	[[nodiscard]] size_t get_chars(uintptr_t Codepage, std::string_view Str, std::span<wchar_t> Buffer, diagnostics* Diagnostics = {});
	void get_chars(uintptr_t Codepage, std::string_view Str, string& Buffer, diagnostics* Diagnostics = {});
	[[nodiscard]] size_t get_chars(uintptr_t Codepage, bytes_view Str, std::span<wchar_t> Buffer, diagnostics* Diagnostics = {});
	void get_chars(uintptr_t Codepage, bytes_view Str, string& Buffer, diagnostics* Diagnostics = {});
	[[nodiscard]] string get_chars(uintptr_t Codepage, std::string_view Str, diagnostics* Diagnostics = {});
	[[nodiscard]] string get_chars(uintptr_t Codepage, bytes_view Str, diagnostics* Diagnostics = {});

	[[nodiscard]] size_t get_chars_count(uintptr_t Codepage, std::string_view Str, diagnostics* Diagnostics = {});
	[[nodiscard]] size_t get_chars_count(uintptr_t Codepage, bytes_view Str, diagnostics* Diagnostics = {});

	//-------------------------------------------------------------------------

	[[noreturn]]
	void raise_exception(uintptr_t Codepage, string_view Str, size_t Position);

	namespace detail
	{
		template<typename T>
		class codepage
		{
		public:
			[[nodiscard]] static auto get_bytes(string_view const Str, std::span<char> const Buffer, diagnostics* const Diagnostics = {})
			{
				return encoding::get_bytes(T::id(), Str, Buffer, Diagnostics);
			}

			static auto get_bytes(string_view const Str, std::string& Buffer, diagnostics* const Diagnostics = {})
			{
				return encoding::get_bytes(T::id(), Str, Buffer, Diagnostics);
			}

			[[nodiscard]] static auto get_bytes(string_view const Str, diagnostics* const Diagnostics = {})
			{
				return encoding::get_bytes(T::id(), Str, Diagnostics);
			}

			[[nodiscard]] static auto get_bytes_count(string_view const Str, diagnostics* const Diagnostics = {})
			{
				return encoding::get_bytes_count(T::id(), Str, Diagnostics);
			}

			[[nodiscard]] static auto get_chars(std::string_view const Str, std::span<wchar_t> const Buffer, diagnostics* const Diagnostics = {})
			{
				return encoding::get_chars(T::id(), Str, Buffer, Diagnostics);
			}

			static auto get_chars(std::string_view const Str, string& Buffer, diagnostics* const Diagnostics = {})
			{
				return encoding::get_chars(T::id(), Str, Buffer, Diagnostics);
			}

			[[nodiscard]] static auto get_chars(bytes_view const Str, std::span<wchar_t> const Buffer, diagnostics* const Diagnostics = {})
			{
				return encoding::get_chars(T::id(), Str, Buffer, Diagnostics);
			}

			static auto get_chars(bytes_view const Str, string& Buffer, diagnostics* const Diagnostics = {})
			{
				return encoding::get_chars(T::id(), Str, Buffer, Diagnostics);
			}

			[[nodiscard]] static auto get_chars(std::string_view const Str, diagnostics* const Diagnostics = {})
			{
				return encoding::get_chars(T::id(), Str, Diagnostics);
			}

			[[nodiscard]] static auto get_chars(bytes_view const Str, diagnostics* const Diagnostics = {})
			{
				return encoding::get_chars(T::id(), Str, Diagnostics);
			}

			[[nodiscard]] static auto get_chars_count(std::string_view const Str, diagnostics* const Diagnostics = {})
			{
				return encoding::get_chars_count(T::id(), Str, Diagnostics);
			}

			[[nodiscard]] static auto get_chars_count(bytes_view const Str, diagnostics* const Diagnostics = {})
			{
				return encoding::get_chars_count(T::id(), Str, Diagnostics);
			}
		};
	}

	using utf8 = detail::codepage<codepage::detail::utf8>;
	using ansi = detail::codepage<codepage::detail::ansi>;
	using oem = detail::codepage<codepage::detail::oem>;

	struct utf8_or_ansi
	{
		[[nodiscard]] static string get_chars(std::string_view Str, diagnostics* Diagnostics = {});
	};

	struct ascii
	{
		[[nodiscard]] static string get_chars(std::string_view Str);
	};

	[[nodiscard]] std::string_view get_signature_bytes(uintptr_t Cp);

	class writer
	{
	public:
		NONCOPYABLE(writer);
		writer(std::ostream& Stream, uintptr_t Codepage, bool AddSignature = true, bool IgnoreEncodingErrors = false);

		void write(string_view Str, const auto&... Args)
		{
			write_impl(Str);
			(..., write_impl(Args));
		}

	private:
		void write_impl(string_view Str);

		std::string m_Buffer;
		std::ostream* m_Stream;
		uintptr_t m_Codepage;
		bool m_AddSignature;
		bool m_IgnoreEncodingErrors;
	};

	is_utf8 is_valid_utf8(std::string_view Str, bool PartialContent);

	inline constexpr wchar_t bom_char      = L'﻿'; // Zero Length Space
	inline constexpr wchar_t replace_char  = L'�'; // Replacement
	inline constexpr wchar_t continue_char = L'›'; // Single Right-Pointing Angle Quotation Mark

	namespace utf16
	{
		bool is_high_surrogate(wchar_t Char);
		bool is_low_surrogate(wchar_t Char);
		bool is_valid_surrogate_pair(wchar_t First, wchar_t Second);
		char32_t extract_codepoint(wchar_t First, wchar_t Second);
		char32_t extract_codepoint(string_view Str);
		void remove_first_codepoint(string_view& Str);
		void remove_last_codepoint(string_view& Str);
		std::pair<wchar_t, wchar_t> to_surrogate(char32_t Codepoint);
	}
}

void swap_bytes(void const* Src, void* Dst, size_t SizeInBytes, size_t ElementSize);

//#############################################################################

class MultibyteCodepageDecoder
{
public:
	NONCOPYABLE(MultibyteCodepageDecoder);

	MultibyteCodepageDecoder() = default;

	bool SetCP(uintptr_t Codepage);
	uintptr_t GetCP() const { return m_Codepage; }
	size_t GetSize() const { return m_Size; }
	size_t GetChar(std::string_view Str, wchar_t& Char, bool* End = nullptr) const;

private:
	std::vector<char> len_mask; //[256]
	std::vector<wchar_t> m1;    //[256]
	std::vector<wchar_t> m2;  //[65536]

	uintptr_t m_Codepage {};
	size_t m_Size {};
};

//#############################################################################

namespace Utf8
{
	// Returns the number of decoded chars, 1 or 2. Moves StrIterator forward
	// When there is not enough data to decode the whole character:
	// - First will have the the first raw byte embedded
	// - StrIterator will be advanced by one
	// - Diagnostics will be set accordingly
	// You might want to drop the result or continue decoding the tail as embedded raw bytes
	[[nodiscard]] size_t get_char(
		std::string_view::const_iterator& StrIterator,
		std::string_view::const_iterator StrEnd,
		wchar_t& First,
		wchar_t& Second,
		encoding::diagnostics& Diagnostics
	);

	// returns the number of decoded chars, up to Buffer.size(). Stops on buffer overflow. Tail contains the number of unprocessed bytes.
	[[nodiscard]] size_t get_chars(std::string_view Str, std::span<wchar_t> Buffer, int& Tail);
}

//#############################################################################

class [[nodiscard]] raw_eol
{
public:
	explicit raw_eol(uintptr_t const Codepage)
	{
		if (!IsUtfCodePage(Codepage))
		{
			m_Cr = to(Codepage, m_Cr);
			m_Lf = to(Codepage, m_Lf);
		}
	}

	[[nodiscard]] auto cr() const { return m_Cr; }
	[[nodiscard]] auto lf() const { return m_Lf; }

private:
	static char to(uintptr_t Codepage, wchar_t WideChar);

	char
		m_Cr{'\r'},
		m_Lf{'\n'};
};

#endif // ENCODING_HPP_44AE7032_AF79_4A6F_A2ED_529BC1A38758
