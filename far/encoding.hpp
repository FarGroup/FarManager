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
#include "exception.hpp"

// Platform:

// Common:
#include "common/bytes_view.hpp"
#include "common/noncopyable.hpp"
#include "common/range.hpp"

// External:

//----------------------------------------------------------------------------

namespace encoding
{
	class exception: public far_exception
	{
	public:
		template<typename... args>
		explicit exception(uintptr_t Codepage, size_t const Position, args&&... Args):
			far_exception(L""sv, FWD(Args)...),
			m_Codepage(Codepage),
			m_Position(Position)
		{
		}

		auto codepage() const { return m_Codepage; }
		auto position() const { return m_Position; }

	private:
		uintptr_t m_Codepage;
		size_t m_Position;
	};

	namespace codepage
	{
		[[nodiscard]] uintptr_t ansi();
		[[nodiscard]] uintptr_t oem();
	}

	// Throws if the conversion is lossy
	[[nodiscard]] size_t get_bytes_strict(uintptr_t Codepage, string_view Str, span<char> Buffer);
	[[nodiscard]] size_t get_bytes(uintptr_t Codepage, string_view Str, span<char> Buffer, bool* UsedDefaultChar = nullptr);
	[[nodiscard]] std::string get_bytes(uintptr_t Codepage, string_view Str, bool* UsedDefaultChar = nullptr);

	// Throws if the conversion is lossy
	[[nodiscard]] size_t get_bytes_count_strict(uintptr_t Codepage, string_view Str);
	[[nodiscard]] size_t get_bytes_count(uintptr_t Codepage, string_view Str);

	//-------------------------------------------------------------------------

	// Throws if the conversion is lossy
	[[nodiscard]] size_t get_chars_strict(uintptr_t Codepage, std::string_view Str, span<wchar_t> Buffer);
	[[nodiscard]] size_t get_chars(uintptr_t Codepage, std::string_view Str, span<wchar_t> Buffer);
	[[nodiscard]] size_t get_chars(uintptr_t Codepage, bytes_view Str, span<wchar_t> Buffer);
	[[nodiscard]] string get_chars(uintptr_t Codepage, std::string_view Str);
	[[nodiscard]] string get_chars(uintptr_t Codepage, bytes_view Str);

	// Throws if the conversion is lossy
	[[nodiscard]] size_t get_chars_count_strict(uintptr_t Codepage, std::string_view Str);
	[[nodiscard]] size_t get_chars_count(uintptr_t Codepage, std::string_view Str);
	[[nodiscard]] size_t get_chars_count(uintptr_t Codepage, bytes_view Str);

	//-------------------------------------------------------------------------

	namespace detail
	{
		template<uintptr_t Codepage>
		class codepage
		{
		public:
			static auto get_bytes(string_view const Str, span<char> const Buffer, bool* const UsedDefaultChar = nullptr)
			{
				return encoding::get_bytes(Codepage, Str, Buffer, UsedDefaultChar);
			}

			[[nodiscard]] static auto get_bytes(string_view const Str, bool* const UsedDefaultChar = nullptr)
			{
				return encoding::get_bytes(Codepage, Str, UsedDefaultChar);
			}

			[[nodiscard]] static auto get_bytes_count(string_view const Str)
			{
				return encoding::get_bytes_count(Codepage, Str);
			}

			static auto get_chars(std::string_view const Str, span<wchar_t> const Buffer)
			{
				return encoding::get_chars(Codepage, Str, Buffer);
			}

			[[nodiscard]] static auto get_chars(bytes_view const Str, span<wchar_t> const Buffer)
			{
				return encoding::get_chars(Codepage, Str, Buffer);
			}

			[[nodiscard]] static auto get_chars(std::string_view const Str)
			{
				return encoding::get_chars(Codepage, Str);
			}

			[[nodiscard]] static auto get_chars(bytes_view const Str)
			{
				return encoding::get_chars(Codepage, Str);
			}

			[[nodiscard]] static auto get_chars_count(std::string_view const Str)
			{
				return encoding::get_chars_count(Codepage, Str);
			}

			[[nodiscard]] static auto get_chars_count(bytes_view const Str)
			{
				return encoding::get_chars_count(Codepage, Str);
			}
		};
	}

	using utf8 = detail::codepage<CP_UTF8>;
	using ansi = detail::codepage<CP_ACP>;
	using oem = detail::codepage<CP_OEMCP>;

	[[nodiscard]] std::string_view get_signature_bytes(uintptr_t Cp);

	class writer
	{
	public:
		NONCOPYABLE(writer);
		writer(std::ostream& Stream, uintptr_t Codepage, bool AddSignature = true);
		void write(string_view Str);

	private:
		std::vector<char> m_Buffer;
		std::ostream* m_Stream;
		uintptr_t m_Codepage;
		bool m_AddSignature;
	};

	bool is_valid_utf8(std::string_view Str, bool PartialContent, bool& PureAscii);
}

void swap_bytes(const void* Src, void* Dst, size_t SizeInBytes);

[[nodiscard]] bool IsVirtualCodePage(uintptr_t cp);
[[nodiscard]] bool IsUnicodeCodePage(uintptr_t cp);
[[nodiscard]] bool IsStandardCodePage(uintptr_t cp);
[[nodiscard]] bool IsUnicodeOrUtfCodePage(uintptr_t cp);
[[nodiscard]] bool IsNoFlagsCodepage(uintptr_t cp);

[[nodiscard]] string ShortReadableCodepageName(uintptr_t cp);

//#############################################################################

class MultibyteCodepageDecoder: noncopyable
{
public:
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

namespace Utf
{
	const wchar_t REPLACE_CHAR  = L'\xFFFD'; // Replacement
	const wchar_t BOM_CHAR      = L'\xFEFF'; // Zero Length Space
	const wchar_t CONTINUE_CHAR = L'\x203A'; // Single Right-Pointing Angle Quotation Mark


	struct errors
	{
		struct
		{
			bool Error{};
			size_t Position{};
		}
		Conversion;
	};

	size_t get_chars(uintptr_t Codepage, std::string_view Str, span<wchar_t> Buffer, errors* Errors);
}

namespace Utf8
{
	// returns the number of decoded chars, 1 or 2. Moves the StrIterator forward as required.
	[[nodiscard]] size_t get_char(std::string_view::const_iterator& StrIterator, std::string_view::const_iterator StrEnd, wchar_t& First, wchar_t& Second);
	// returns the number of decoded chars, up to Buffer.size(). Stops on buffer overflow. Tail contains the number of unprocessed bytes.
	[[nodiscard]] size_t get_chars(std::string_view Str, span<wchar_t> Buffer, int& Tail);
	// returns the required buffer size. Fills Buffer up to its size.
	[[nodiscard]] size_t get_chars(std::string_view Str, span<wchar_t> Buffer, Utf::errors* Errors);
	// returns the required buffer size. Fills Buffer up to its size.
	[[nodiscard]] size_t get_bytes(string_view Str, span<char> Buffer);
}

//#############################################################################

class [[nodiscard]] raw_eol
{
public:
	raw_eol(): m_Cr('\r'), m_Lf('\n') {}
	explicit raw_eol(uintptr_t Codepage): m_Cr(to(Codepage, L'\r')), m_Lf(to(Codepage, L'\n')) {}

	template <class T>
	[[nodiscard]] T cr() const { return value<T>(L'\r', m_Cr); }

	template <class T>
	[[nodiscard]] T lf() const { return value<T>(L'\n', m_Lf); }

private:
	static char to(uintptr_t Codepage, wchar_t WideChar)
	{
		char Char;
		return encoding::get_bytes(Codepage, { &WideChar, 1 }, { &Char, 1 })? Char : WideChar;
	}

	template <typename T>
	static T value(
		[[maybe_unused]] wchar_t const WideChar,
		[[maybe_unused]] char const Char
	)
	{
		if constexpr (std::is_same_v<T, wchar_t>)
			return WideChar;
		else
		{
			static_assert(std::is_same_v<T, char>);
			return Char;
		}
	}

	const char m_Cr;
	const char m_Lf;
};

struct cp_info
{
	string Name;
	unsigned char MaxCharSize;
};

using cp_map = std::unordered_map<unsigned, cp_info>;
[[nodiscard]] const cp_map& InstalledCodepages();
[[nodiscard]] cp_info const* GetCodePageInfo(unsigned cp);

#endif // ENCODING_HPP_44AE7032_AF79_4A6F_A2ED_529BC1A38758
