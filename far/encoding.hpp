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

namespace encoding
{
	namespace codepage
	{
		uintptr_t ansi();
		uintptr_t oem();
	}

	size_t get_bytes(uintptr_t Codepage, string_view Str, char* Buffer, size_t BufferSize, bool* UsedDefaultChar = nullptr);

	template<typename T>
	auto get_bytes(uintptr_t const Codepage, string_view const Str, T& Buffer, bool* const UsedDefaultChar = nullptr)
	{
		return get_bytes(Codepage, Str, std::data(Buffer), std::size(Buffer), UsedDefaultChar);
	}

	std::string get_bytes(uintptr_t Codepage, string_view Str, bool* UsedDefaultChar = nullptr);

	inline auto get_bytes_count(uintptr_t const Codepage, string_view const Str)
	{
		return get_bytes(Codepage, Str, nullptr, 0);
	}

	//-------------------------------------------------------------------------

	size_t get_chars(uintptr_t Codepage, std::string_view Str, wchar_t* Buffer, size_t BufferSize);

	template<typename T>
	auto get_chars(uintptr_t const Codepage, std::string_view const Str, T& Buffer)
	{
		return get_chars(Codepage, Str, std::data(Buffer), std::size(Buffer));
	}

	string get_chars(uintptr_t Codepage, std::string_view Str);

	inline auto get_chars_count(uintptr_t const Codepage, std::string_view const Str)
	{
		return get_chars(Codepage, Str, nullptr, 0);
	}

	//-------------------------------------------------------------------------

	namespace detail
	{
		template<uintptr_t Codepage>
		class codepage
		{
		public:
			template<class... args>
			static auto get_bytes_count(string_view Str, args&&... Args) { return encoding::get_bytes_count(Codepage, Str, FWD(Args)...); }

			template<class... args>
			static auto get_bytes(string_view Str, args&&... Args) { return encoding::get_bytes(Codepage, Str, FWD(Args)...); }

			template<class... args>
			static auto get_chars_count(std::string_view Str, args&&... Args) { return encoding::get_chars_count(Codepage, Str, FWD(Args)...); }

			template<class... args>
			static auto get_chars(std::string_view Str, args&&... Args) { return encoding::get_chars(Codepage, Str, FWD(Args)...); }
		};
	}

	using utf8 = detail::codepage<CP_UTF8>;
	using ansi = detail::codepage<CP_ACP>;
	using oem = detail::codepage<CP_OEMCP>;

	std::string_view get_signature_bytes(uintptr_t Cp);

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

bool IsVirtualCodePage(uintptr_t cp);
bool IsUnicodeCodePage(uintptr_t cp);
bool IsStandardCodePage(uintptr_t cp);
bool IsUnicodeOrUtfCodePage(uintptr_t cp);
bool IsNoFlagsCodepage(uintptr_t cp);

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

	size_t get_chars(uintptr_t Codepage, std::string_view Str, wchar_t* Buffer, size_t BufferSize, errors* Errors);
}

namespace Utf8
{
	// returns the number of decoded chars, 1 or 2. Moves the StrIterator forward as required.
	size_t get_char(std::string_view::const_iterator& StrIterator, std::string_view::const_iterator StrEnd, wchar_t& First, wchar_t& Second);
	// returns the number of decoded chars, up to the BufferSize. Stops on buffer overflow. Tail contains the number of unprocessed bytes.
	size_t get_chars(std::string_view Str, wchar_t* Buffer, size_t BufferSize, int& Tail);
	// returns the required buffer size. Fills Buffer up to the BufferSize.
	size_t get_chars(std::string_view Str, wchar_t* Buffer, size_t BufferSize, Utf::errors* Errors);
	// returns the required buffer size. Fills Buffer up to the BufferSize.
	size_t get_bytes(string_view Str, char* Buffer, size_t BufferSize);
}

//#############################################################################

class raw_eol
{
public:
	raw_eol(): m_Cr('\r'), m_Lf('\n') {}
	explicit raw_eol(uintptr_t Codepage): m_Cr(to(Codepage, L'\r')), m_Lf(to(Codepage, L'\n')) {}

	template <class T>
	T cr() const = delete;
	template <class T>
	T lf() const = delete;

private:
	static char to(uintptr_t Codepage, wchar_t WideChar)
	{
		char Char = WideChar;
		encoding::get_bytes(Codepage, { &WideChar, 1 }, &Char, 1);
		return Char;
	}

	const char m_Cr;
	const char m_Lf;
};

template<>
inline char raw_eol::cr<char>() const { return m_Cr; }
template<>
inline char raw_eol::lf<char>() const { return m_Lf; }
template<>
inline wchar_t raw_eol::cr<wchar_t>() const { return L'\r'; }
template<>
inline wchar_t raw_eol::lf<wchar_t>() const { return L'\n'; }

// {Codepage: (MaxCharSize, Name)}
using cp_map = std::unordered_map<UINT, std::pair<UINT, string>>;
const cp_map& InstalledCodepages();
cp_map::value_type::second_type GetCodePageInfo(UINT cp);

#endif // ENCODING_HPP_44AE7032_AF79_4A6F_A2ED_529BC1A38758
