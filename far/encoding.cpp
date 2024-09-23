/*
encoding.cpp

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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "encoding.hpp"

// Internal:
#include "strmix.hpp"
#include "exception.hpp"
#include "exception_handler.hpp"
#include "plugin.hpp"
#include "codepage_selection.hpp"

// Platform:

// Common:
#include "common/algorithm.hpp"
#include "common/from_string.hpp"
#include "common/function_ref.hpp"
#include "common/io.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static std::optional<size_t> mismatch(std::ranges::random_access_range auto const& Range1, std::ranges::random_access_range auto const& Range2)
{
	const auto [Mismatch1, Mismatch2] = std::ranges::mismatch(Range1, Range2);

	size_t const
		Pos1 = Mismatch1 - std::cbegin(Range1),
		Pos2 = Mismatch2 - std::cbegin(Range2);

	if (Pos1 == std::size(Range1) && Pos2 == std::size(Range2))
		return {};

	return std::min(Pos1, Pos2);
}

static bool is_retarded_error()
{
	const auto Error = GetLastError();
	return Error == ERROR_INVALID_FLAGS || Error == ERROR_INVALID_PARAMETER;
}

// See https://msdn.microsoft.com/en-us/library/windows/desktop/dd319072.aspx
static bool IsNoFlagsCodepage(uintptr_t cp)
{
	return
		cp == CP_UTF8 ||
		cp == 54936 ||
		(cp >= 50220 && cp <= 50222) ||
		cp == 50225 ||
		cp == 50227 ||
		cp == 50229 ||
		(cp >= 57002 && cp <= 57011) ||
		cp == CP_UTF7 ||
		cp == CP_SYMBOL;
}

static size_t widechar_to_multibyte_with_validation(uintptr_t const Codepage, string_view const Str, std::span<char> Buffer, encoding::diagnostics* const Diagnostics)
{
	const auto NoTranslationEnabled = Diagnostics && Diagnostics->EnabledDiagnostics & encoding::diagnostics::no_translation;
	auto IsRetardedCodepage = IsNoFlagsCodepage(Codepage);
	BOOL DefaultCharUsed = FALSE;

	const auto convert = [&](std::span<char> const To)
	{
		for (;;)
		{
			if (const auto Result = WideCharToMultiByte(
				Codepage,
				IsRetardedCodepage || !NoTranslationEnabled? 0 : WC_NO_BEST_FIT_CHARS,
				Str.data(),
				static_cast<int>(Str.size()),
				To.data(),
				static_cast<int>(To.size()),
				{},
				IsRetardedCodepage? nullptr : &DefaultCharUsed
			))
				return Result;

			if (!IsRetardedCodepage && is_retarded_error())
				IsRetardedCodepage = true;
			else
				return 0;
		}
	};

	auto Result = convert(Buffer);
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		// If BufferSize is less than DataSize, this function writes the number of bytes specified by BufferSize to the buffer indicated by Buffer.
		// If the function succeeds and BufferSize is 0, the return value is the required size, in bytes, for the buffer indicated by Buffer.
		Result = convert({});
	}

	// The result is ok and can be trusted
	if (Result && !DefaultCharUsed && !IsRetardedCodepage)
		return Result;

	// They don't care, no point to go deeper
	if (!NoTranslationEnabled)
		return Result;

	std::string LocalBuffer;

	if (Buffer.size() < static_cast<size_t>(Result))
	{
		LocalBuffer.resize(Result);
		Buffer = LocalBuffer;
		Result = convert(Buffer);
	}

	const auto Roundtrip = encoding::get_chars(Codepage, { Buffer.data(), static_cast<size_t>(Result) });

	if (const auto Pos = mismatch(Str, Roundtrip))
	{
		Diagnostics->ErrorPosition = *Pos;
	}

	return Result;
}

static size_t multibyte_to_widechar_with_validation(uintptr_t const Codepage, std::string_view Str, std::span<wchar_t> Buffer, encoding::diagnostics* const Diagnostics)
{
	const auto NoTranslationEnabled = Diagnostics && Diagnostics->EnabledDiagnostics & encoding::diagnostics::no_translation;
	auto IsRetardedCodepage = IsNoFlagsCodepage(Codepage);
	auto Strict = true;

	const auto convert = [&](std::span<wchar_t> const To)
	{
		for (;;)
		{
			if (const auto Result = MultiByteToWideChar(
				Codepage,
				!Strict || IsRetardedCodepage? 0 : MB_ERR_INVALID_CHARS,
				Str.data(),
				static_cast<int>(Str.size()),
				To.data(),
				static_cast<int>(To.size())
			))
				return Result;

			if (!IsRetardedCodepage && is_retarded_error())
				IsRetardedCodepage = true;
			else
				return 0;
		}
	};

	const auto convert_and_get_size = [&]
	{
		auto Result = convert(Buffer);
		if (Buffer.size() <= Str.size() && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			// If BufferSize is less than DataSize, this function writes the number of characters specified by BufferSize to the buffer indicated by Buffer.
			// If the function succeeds and BufferSize is 0, the return value is the required size, in characters, for the buffer indicated by Buffer.
			Result = convert({});
		}
		return Result;
	};

	auto Result = convert_and_get_size();

	// We're still in strict mode and the result is ok and can be trusted
	if (Result && !IsRetardedCodepage)
		return Result;

	// Try to convert at least something
	if (!Result && GetLastError() == ERROR_NO_UNICODE_TRANSLATION)
	{
		Strict = false;
		Result = convert_and_get_size();
	}

	// They don't care, no point to go deeper
	if (!NoTranslationEnabled)
		return Result;

	string LocalBuffer;

	if (Buffer.size() < static_cast<size_t>(Result))
	{
		LocalBuffer.resize(Result);
		Buffer = LocalBuffer;
		Result = convert(Buffer);
		if (!Result)
			return Result;
	}

	const auto Roundtrip = encoding::get_bytes(Codepage, { Buffer.data(), static_cast<size_t>(Result) }, nullptr);

	if (const auto Pos = mismatch(Str, Roundtrip))
	{
		Diagnostics->ErrorPosition = *Pos;
	}

	return Result;
}

static bool IsValid(unsigned cp)
{
	if (cp==CP_ACP || cp==CP_OEMCP || cp==CP_MACCP || cp==CP_THREAD_ACP || cp==CP_SYMBOL)
		return false;

	if (cp == CP_UTF8 || cp == CP_UTF16LE || cp == CP_UTF16BE)
		return false;

	const auto Info = GetCodePageInfo(cp);
	return Info && Info->MaxCharSize == 2;
}

bool MultibyteCodepageDecoder::SetCP(uintptr_t Codepage)
{
	if (Codepage && Codepage == m_Codepage)
		return true;

	if (!IsValid(Codepage))
		return false;

	len_mask.assign(256, 0);
	m1.assign(256, 0);
	m2.assign(256*256, 0);

	union
	{
		char Buffer[2];
		char b1;
		wchar_t b2;
	}
	u{};

	size_t Size = 0;
	for (const auto i: std::views::iota(0, 65536)) // only UCS2 range
	{
		encoding::diagnostics Diagnostics;
		const auto Char = static_cast<wchar_t>(i);
		const auto CharSize = widechar_to_multibyte_with_validation(Codepage, { &Char, 1 }, u.Buffer, &Diagnostics);
		if (!CharSize || Diagnostics.ErrorPosition)
			continue;

		len_mask[u.b1] |= bit(CharSize - 1);
		Size = std::max(Size, CharSize);

		switch (CharSize)
		{
			case 1: m1[u.b1] = Char; break;
			case 2: m2[u.b2] = Char; break;
		}
	}

	m_Codepage = Codepage;
	m_Size = Size;
	return true;
}

size_t MultibyteCodepageDecoder::GetChar(std::string_view const Str, wchar_t& Char, bool* End) const
{
	if (Str.empty())
	{
		if (End)
		{
			*End = true;
		}
		return 0;
	}

	const auto b1 = Str[0];
	const auto lmask = len_mask[b1];
	if (!lmask)
		return 0;

	if (lmask & 0x01)
	{
		Char = m1[b1];
		return 1;
	}

	if (Str.size() < 2)
	{
		if (End)
		{
			*End = true;
		}
		return 0;
	}

	const uint16_t b2 = b1 | (Str[1] << 8);
	if (!m2[b2])
	{
		return 0;
	}
	else
	{
		Char = m2[b2];
		return 2;
	}
}

char raw_eol::to(uintptr_t Codepage, wchar_t WideChar)
{
	char Char;
	return encoding::get_bytes(Codepage, { &WideChar, 1 }, { &Char, 1 })? Char : WideChar;
}

static size_t utf8_get_bytes(string_view Str, std::span<char> Buffer);

static size_t get_bytes_impl(uintptr_t const Codepage, string_view const Str, std::span<char> Buffer, encoding::diagnostics* const Diagnostics)
{
	if (Str.empty())
		return 0;

	if (Diagnostics)
		Diagnostics->clear();

	switch(Codepage)
	{
	case CP_UTF8:
		return utf8_get_bytes(Str, Buffer);

	case CP_UTF16LE:
	case CP_UTF16BE:
		{
			const auto Size = std::min(Str.size() * sizeof(char16_t), Buffer.size());
			if (Codepage == CP_UTF16LE)
			{
				static_assert(std::endian::native == std::endian::little, "No way");
				copy_memory(Str.data(), Buffer.data(), Size);
			}
			else
			{
				const auto EvenSize = Size / sizeof(char16_t) * sizeof(char16_t);
				static_assert(std::endian::native == std::endian::little, "No way");
				swap_bytes(Str.data(), Buffer.data(), EvenSize, sizeof(char16_t));

				if (Size & 1)
					Buffer.back() = extract_integer<char, 1>(Str.back());
			}

			return Str.size() * sizeof(char16_t);
		}

	default:
		return widechar_to_multibyte_with_validation(Codepage, Str, Buffer, Diagnostics);
	}
}

encoding::diagnostics::diagnostics(unsigned Diagnostics):
	EnabledDiagnostics(Diagnostics)
{
}

void encoding::diagnostics::clear()
{
	ErrorPosition.reset();
	PartialInput = {};
	PartialOutput = {};
	m_IsUtf8 = is_utf8::yes_ascii;
}

void encoding::diagnostics::set_is_utf8(is_utf8 const IsUtf8)
{
	if (m_IsUtf8 == is_utf8::yes_ascii)
		m_IsUtf8 = IsUtf8;
}

encoding::is_utf8 encoding::diagnostics::get_is_utf8() const
{
	return m_IsUtf8;
}

size_t encoding::get_bytes(uintptr_t const Codepage, string_view const Str, std::span<char> const Buffer, diagnostics* const Diagnostics)
{
	const auto Result = get_bytes_impl(Codepage, Str, Buffer, Diagnostics);
	if (Result < Buffer.size())
	{
		Buffer[Result] = '\0';
	}
	return Result;
}

void encoding::get_bytes(uintptr_t Codepage, string_view Str, std::string& Buffer, diagnostics* const Diagnostics)
{
	if (Str.empty())
	{
		Buffer.clear();
		return;
	}

	const auto EstimatedCharsCount = [&]
	{
		switch (Codepage)
		{
		case CP_UTF16LE:
		case CP_UTF16BE:
			return Str.size() * sizeof(char16_t);


		default:
			return Str.size();
		}
	};

	// With this approach we can fill the buffer with only one attempt in many cases.
	resize_exp(Buffer, EstimatedCharsCount());

	for (auto Overflow = true; Overflow;)
	{
		const auto Size = get_bytes(Codepage, Str, std::span(Buffer), Diagnostics);
		Overflow = Size > Buffer.size();
		Buffer.resize(Size);
	}
}

std::string encoding::get_bytes(uintptr_t const Codepage, string_view const Str, diagnostics* const Diagnostics)
{
	std::string Result;
	get_bytes(Codepage, Str, Result, Diagnostics);
	return Result;
}

size_t encoding::get_bytes_count(uintptr_t const Codepage, string_view const Str, diagnostics* const Diagnostics)
{
	return get_bytes(Codepage, Str, {}, Diagnostics);
}

static size_t utf8_get_chars(std::string_view Str, std::span<wchar_t> Buffer, encoding::diagnostics* Diagnostics);
static size_t utf7_get_chars(std::string_view Str, std::span<wchar_t> Buffer, encoding::diagnostics* Diagnostics);

static size_t get_chars_impl(uintptr_t const Codepage, std::string_view Str, std::span<wchar_t> const Buffer, encoding::diagnostics* const Diagnostics)
{
	if (Str.empty())
		return 0;

	if (Diagnostics)
		Diagnostics->clear();

	const auto validate_unicode = [&]
	{
		if (Str.size() & 1 && Diagnostics && Diagnostics->EnabledDiagnostics & encoding::diagnostics::not_enough_data)
		{
			Diagnostics->ErrorPosition = Str.size();
			Diagnostics->PartialInput = 1;
			Diagnostics->PartialOutput = 1;
		}
	};

	switch (Codepage)
	{
	case CP_UTF7:
		return utf7_get_chars(Str, Buffer, Diagnostics);

	case CP_UTF8:
		return utf8_get_chars(Str, Buffer, Diagnostics);

	case CP_UTF16LE:
		static_assert(std::endian::native == std::endian::little, "No way");
		copy_memory(Str.data(), Buffer.data(), std::min(Str.size(), Buffer.size() * sizeof(char16_t)));
		validate_unicode();
		return (Str.size() + sizeof(uint16_t) - 1) / sizeof(wchar_t);

	case CP_UTF16BE:
		{
			const auto EvenStrSize = Str.size() / sizeof(char16_t) * sizeof(char16_t);
			const auto BufferSizeInBytes = Buffer.size() * sizeof(char16_t);
			const auto BytesCount = std::min(EvenStrSize, BufferSizeInBytes);
			static_assert(std::endian::native == std::endian::little, "No way");
			swap_bytes(Str.data(), Buffer.data(), BytesCount, sizeof(char16_t));
			if (Str.size() & 1 && Str.size() < BufferSizeInBytes)
				Buffer[BytesCount / sizeof(char16_t)] = make_integer<char16_t>('\0', Str.back());
		}
		validate_unicode();
		return (Str.size() + sizeof(uint16_t) - 1) / sizeof(wchar_t);

	default:
		return multibyte_to_widechar_with_validation(Codepage, Str, Buffer, Diagnostics);
	}
}

size_t encoding::get_chars(uintptr_t const Codepage, std::string_view const Str, std::span<wchar_t> const Buffer, diagnostics* const Diagnostics)
{
	const auto Result = get_chars_impl(Codepage, Str, Buffer, Diagnostics);
	if (Result < Buffer.size())
	{
		Buffer[Result] = {};
	}
	return Result;
}

namespace utf8
{
	static size_t wchars_count(std::string_view Str);
}

void encoding::get_chars(uintptr_t const Codepage, std::string_view const Str, string& Buffer, diagnostics* const Diagnostics)
{
	if (Str.empty())
	{
		Buffer.clear();
		return;
	}

	const auto EstimatedCharsCount = [&]
	{
		switch (Codepage)
		{
		case CP_UTF7:
			// Even though DataSize is always >= BufferSize, we can't use DataSize for estimation - it can be three times larger than necessary.
			return get_chars_count(Codepage, Str, Diagnostics);

		case CP_UTF8:
			// This function assumes correct UTF-8, which is not always the case, but it will do for the size estimation.
			return ::utf8::wchars_count(Str);

		case CP_UTF16LE:
		case CP_UTF16BE:
			return (Str.size() + sizeof(char16_t) - 1) / sizeof(char16_t);

		default:
			return Str.size();
		}
	};

	// With this approach we can fill the buffer with only one attempt in many cases.
	resize_exp(Buffer, EstimatedCharsCount());

	for (auto Overflow = true; Overflow;)
	{
		const auto Size = get_chars(Codepage, Str, std::span(Buffer), Diagnostics);
		Overflow = Size > Buffer.size();
		Buffer.resize(Size);
	}
}

size_t encoding::get_chars(uintptr_t const Codepage, bytes_view const Str, std::span<wchar_t> Buffer, diagnostics* const Diagnostics)
{
	return get_chars(Codepage, to_string_view(Str), Buffer, Diagnostics);
}

void encoding::get_chars(uintptr_t const Codepage, bytes_view const Str, string& Buffer, diagnostics* const Diagnostics)
{
	return get_chars(Codepage, to_string_view(Str), Buffer, Diagnostics);
}

string encoding::get_chars(uintptr_t const Codepage, std::string_view const Str, diagnostics* const Diagnostics)
{
	string Result;
	get_chars(Codepage, Str, Result, Diagnostics);
	return Result;
}

string encoding::get_chars(uintptr_t const Codepage, bytes_view const Str, diagnostics* const Diagnostics)
{
	return get_chars(Codepage, to_string_view(Str), Diagnostics);
}

size_t encoding::get_chars_count(uintptr_t const Codepage, std::string_view const Str, diagnostics* const Diagnostics)
{
	return get_chars(Codepage, Str, {}, Diagnostics);
}

size_t encoding::get_chars_count(uintptr_t const Codepage, bytes_view const Str, diagnostics* const Diagnostics)
{
	return get_chars(Codepage, Str, {}, Diagnostics);
}

void encoding::raise_exception(uintptr_t const Codepage, string_view const Str, size_t const Position)
{
	throw far_known_exception(
		concat(
			codepages::UnsupportedCharacterMessage(Str[Position]),
			L"\n"sv,
			codepages::FormatName(Codepage)
		)
	);
}

string encoding::utf8_or_ansi::get_chars(std::string_view const Str, diagnostics* const Diagnostics)
{
	const auto Utf8 = codepage::utf8();
	const auto Ansi = codepage::ansi();

	const auto Encoding = Utf8 == Ansi || is_valid_utf8(Str, false) == is_utf8::yes?
		Utf8 :
		Ansi;

	return encoding::get_chars(Encoding, Str, Diagnostics);
}

string encoding::ascii::get_chars(std::string_view const Str)
{
	assert(std::ranges::all_of(Str, [](char const Char) { return Char < 128; }));

	return { ALL_CONST_RANGE(Str) };
}

std::string_view encoding::get_signature_bytes(uintptr_t Cp)
{
	switch (Cp)
	{
	case CP_UTF8:    return "\xEF\xBB\xBF"sv;
	case CP_UTF16LE: return "\xFF\xFE"sv;
	case CP_UTF16BE: return "\xFE\xFF"sv;
	default:         return {};
	}
}

encoding::writer::writer(std::ostream& Stream, uintptr_t Codepage, bool AddSignature, bool IgnoreEncodingErrors):
	m_Stream(&Stream),
	m_Codepage(Codepage),
	m_AddSignature(AddSignature),
	m_IgnoreEncodingErrors(IgnoreEncodingErrors)
{
}

void encoding::writer::write_impl(const string_view Str)
{
	if (m_AddSignature)
	{
		io::write(*m_Stream, get_signature_bytes(m_Codepage));
		m_AddSignature = false;
	}

	// Nothing to do here
	if (Str.empty())
		return;

	// No need to encode
	if (m_Codepage == CP_UTF16LE)
		return io::write(*m_Stream, Str);

	diagnostics Diagnostics;
	get_bytes(m_Codepage, Str, m_Buffer, m_IgnoreEncodingErrors? nullptr : &Diagnostics);

	if (Diagnostics.ErrorPosition)
		raise_exception(m_Codepage, Str, *Diagnostics.ErrorPosition);

	io::write(*m_Stream, m_Buffer);
}

//################################################################################################

//                                   2                         5         6
//         0                         6                         2         2
// base64: ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/

static const int ill = 0x0100; // illegal
static const int dir = 0x0200; // direct
static const int opt = 0x0400; // optional direct
static const int b64 = 0x0800; // base64 symbol
static const int pls = 0x1000; // +
static const int mns = 0x2000; // -

static const int ILL = ill + 255;
static const int DIR = dir + 255;
static const int OPT = opt + 255;
static const int PLS = pls + b64 + 62;
static const int MNS = mns + dir + 255;

constexpr short operator""_D(unsigned long long const n)
{
	return static_cast<short>(dir + b64 + n);
}

static const short m7[128] =
{
	//  x00   x01   x02   x03   x04   x05   x06   x07   x08   x09   x0a   x0b   x0c   x0d   x0e   x0f
	    ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  DIR,  DIR,  ILL,  ILL,  DIR,  ILL,  ILL,

	//  x10   x11   x12   x13   x14   x15   x16   x17   x18   x19   x1a   x1b   x1c   x1d   x1e   x1f
	    ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,

	// =x20 !=x21 "=x22 #=x23 $=x24 %=x25 &=x26 '=x27 (=x28 )=x29 *=x2a +=x2b ,x=2c -=x2d .=x2e /=x2f
	    DIR,  OPT,  OPT,  OPT,  OPT,  OPT,  OPT,  DIR,  DIR,  DIR,  OPT,  PLS,  DIR,  MNS,  DIR, 63_D,

	//0=x30 1=x31 2=x32 3=x33 4=x34 5=x35 6=x36 7=x37 8=x38 9=x39 :=x3a ;=x3b <=x3c ==x3d >=x3e ?=x3f
	   52_D, 53_D, 54_D, 55_D, 56_D, 57_D, 58_D, 59_D, 60_D, 61_D,  DIR,  OPT,  OPT,  OPT,  OPT,  DIR,

	//@=x40 A=x41 B=x42 C=x43 D=x44 E=x45 F=x46 G=x47 H=x48 I=x49 J=x4a K=x4b L=x4c M=x4d N=x4e O=x4f
	    OPT,  0_D,  1_D,  2_D,  3_D,  4_D,  5_D,  6_D,  7_D,  8_D,  9_D, 10_D, 11_D, 12_D, 13_D, 14_D,

	//P=x50 Q=x51 R=x52 S=x53 T=x54 U=x55 V=x56 W=x57 X=x58 Y=x59 Z=x5a [=x5b \=x5c ]=x5d ^=x5e _=x5f
	   15_D, 16_D, 17_D, 18_D, 19_D, 20_D, 21_D, 22_D, 23_D, 24_D, 25_D,  OPT,  ILL,  OPT,  OPT,  OPT,

	//`=x60 a=x61 b=x62 c=x63 d=x64 e=x65 f=x66 g=x67 h=x68 i=x69 j=x6a k=x6b l=x6c m=x6d n=x6e o=x6f
	    OPT, 26_D, 27_D, 28_D, 29_D, 30_D, 31_D, 32_D, 33_D, 34_D, 35_D, 36_D, 37_D, 38_D, 39_D, 40_D,

	//p=x70 q=x71 r=x72 s=x73 t=x74 u=x75 v=x76 w=x77 x=x78 y=x79 z=x7a {=x7b |=x7c }=x7d ~=x7e   x7f
	   41_D, 42_D, 43_D, 44_D, 45_D, 46_D, 47_D, 48_D, 49_D, 50_D, 51_D,  OPT,  OPT,  OPT,  ILL,  ILL,
};

// BUGBUG non-BMP range is not supported
// TODO: Rewrite
static size_t Utf7_GetChar(
	std::string_view::const_iterator const Iterator,
	std::string_view::const_iterator const End,
	std::span<wchar_t> const Buffer,
	int& state,
	encoding::diagnostics& Diagnostics
)
{
	const size_t DataSize = End - Iterator;

	if (!DataSize)
		return 0;

	auto StrIterator = Iterator;

	size_t BytesConsumed = 1;
	int m[3];
	BYTE c = *StrIterator++;
	if (c >= 128)
	{
		Buffer[0] = encoding::replace_char;
		Diagnostics.ErrorPosition = BytesConsumed - 1;
		return BytesConsumed;
	}

	union
	{
		int state;
		struct { BYTE carry_bits; BYTE carry_count; bool base64; BYTE unused; } s;
	} u;
	u.state = state;

	m[0] = static_cast<int>(m7[c]);
	if ((m[0] & ill) != 0)
	{
		Buffer[0] = encoding::replace_char;
		Diagnostics.ErrorPosition = BytesConsumed - 1;
		return BytesConsumed;
	}

	if (m[0] == DIR || m[0] == OPT)
	{
		u.s.base64 = false;
	}

	if (!u.s.base64)
	{
		if (c != static_cast<BYTE>('+'))
		{
			Buffer[0] = static_cast<wchar_t>(c);
			return BytesConsumed;
		}
		if (DataSize < 2)
		{
			Buffer[0] = encoding::replace_char;
			Diagnostics.ErrorPosition = BytesConsumed - 1;
			return BytesConsumed;
		}

		c = *StrIterator++;
		BytesConsumed = 2;
		if (c >= 128)
		{
			Buffer[0] = encoding::replace_char;
			Diagnostics.ErrorPosition = BytesConsumed - 1;
			return BytesConsumed;
		}

		if (c == static_cast<BYTE>('-'))
		{
			Buffer[0] = L'+';
			return BytesConsumed;
		}

		m[0] = static_cast<int>(m7[c]);
		if (0 == (m[0] & b64))
		{
			Buffer[0] = encoding::replace_char;
			Diagnostics.ErrorPosition = BytesConsumed - 1;
			return BytesConsumed;
		}

		u.s.base64 = true;
		u.s.carry_count = 0;
	}

	const auto a = 2 - u.s.carry_count / 4;
	if (BytesConsumed + a > DataSize)
	{
		Buffer[0] = encoding::replace_char;
		Diagnostics.ErrorPosition = DataSize - 1;
		return DataSize;
	}

	if ((c = *StrIterator++) >= 128)
	{
		u.s.base64 = false;
		state = u.state;
		Buffer[0] = encoding::replace_char;
		Diagnostics.ErrorPosition = BytesConsumed - 1;
		return BytesConsumed;
	}
	m[1] = static_cast<int>(m7[c]);
	if (0 == (m[1] & b64))
	{
		u.s.base64 = false;
		state = u.state;
		Buffer[0] = encoding::replace_char;
		Diagnostics.ErrorPosition = BytesConsumed - 1;
		return BytesConsumed;
	}
	if (a < 2)
	{
		Buffer[0] = static_cast<wchar_t>((u.s.carry_bits << 12) | (static_cast<BYTE>(m[0]) << 6) | static_cast<BYTE>(m[1]));
		u.s.carry_count = 0;
	}
	else
	{
		++BytesConsumed;
		if ((c = *StrIterator++) >= 128)
		{
			u.s.base64 = false;
			state = u.state;
			Buffer[0] = encoding::replace_char;
			Diagnostics.ErrorPosition = BytesConsumed - 1;
			return BytesConsumed;
		}
		m[2] = static_cast<int>(m7[c]);
		if (0 == (m[2] & b64))
		{
			u.s.base64 = false;
			state = u.state;
			Buffer[0] = encoding::replace_char;
			Diagnostics.ErrorPosition = BytesConsumed - 1;
			return BytesConsumed;
		}
		const unsigned m18 = (static_cast<BYTE>(m[0]) << 12) | (static_cast<BYTE>(m[1]) << 6) | static_cast<BYTE>(m[2]);

		if (u.s.carry_count == 0)
		{
			Buffer[0] = static_cast<wchar_t>(m18 >> 2);
			u.s.carry_bits = static_cast<BYTE>(m18 & 0x03);
			u.s.carry_count = 2;
		}
		else
		{
			Buffer[0] = static_cast<wchar_t>((u.s.carry_bits << 14) | (m18 >> 4));
			u.s.carry_bits = static_cast<BYTE>(m18 & 0x0F);
			u.s.carry_count = 4;
		}
	}
	++BytesConsumed;

	if (DataSize > BytesConsumed && *StrIterator == '-')
	{
		u.s.base64 = false;
		++BytesConsumed;
	}

	state = u.state;
	return BytesConsumed;
}

using get_char_t = function_ref<
	size_t(
		std::string_view::const_iterator It,
		std::string_view::const_iterator End,
		std::span<wchar_t> Decoded,
		int& State, // utf-7 only
		encoding::diagnostics& Diagnostics
	)
>;

static size_t BytesToUnicode(
	std::string_view const Str,
	std::span<wchar_t> const Buffer,
	get_char_t const GetChar,
	encoding::diagnostics* const Diagnostics)
{
	if (Str.empty())
		return 0;

	auto StrIterator = Str.begin();
	const auto StrEnd = Str.end();

	auto BufferIterator = Buffer.begin();
	const auto BufferEnd = Buffer.end();

	int State = 0;
	size_t RequiredSize = 0;

	const auto CanReportNotEnoughData = Diagnostics && Diagnostics->EnabledDiagnostics & encoding::diagnostics::not_enough_data;

	bool PartialOutput = false;

	while (StrIterator != StrEnd)
	{
		wchar_t Decoded[2]{};
		encoding::diagnostics LocalDiagnostics;
		const auto BytesConsumed = GetChar(StrIterator, StrEnd, Decoded, State, LocalDiagnostics);

		if (!BytesConsumed)
			break;

		if (Diagnostics)
		{
			if (LocalDiagnostics.ErrorPosition && !Diagnostics->ErrorPosition)
				Diagnostics->ErrorPosition = StrIterator - Str.begin() + *LocalDiagnostics.ErrorPosition;

			Diagnostics->set_is_utf8(LocalDiagnostics.get_is_utf8());
		}

		const auto StoreChar = [&](wchar_t Char)
		{
			if (BufferIterator != BufferEnd)
			{
				*BufferIterator++ = Char;
			}
			++RequiredSize;
		};

		StoreChar(Decoded[0]);

		if (Decoded[1])
		{
			StoreChar(Decoded[1]);
		}

		StrIterator += BytesConsumed;

		if (!PartialOutput)
		{
			if (LocalDiagnostics.PartialOutput)
			{
				PartialOutput = true;
				if (CanReportNotEnoughData)
				{
					Diagnostics->PartialInput = LocalDiagnostics.PartialInput;
					Diagnostics->PartialOutput = LocalDiagnostics.PartialOutput;
				}
			}
		}
		else if (CanReportNotEnoughData)
		{
			++Diagnostics->PartialInput;
			++Diagnostics->PartialOutput;
		}
	}

	return RequiredSize;
}

static size_t utf7_get_chars(std::string_view const Str, std::span<wchar_t> const Buffer, encoding::diagnostics* const Diagnostics)
{
	return BytesToUnicode(Str, Buffer, Utf7_GetChar, Diagnostics);
}

namespace utf16
{
	const auto
		surrogate_high_first = 0b11011000'00000000u, // D800 55296
		surrogate_high_last  = 0b11011011'11111111u, // DBFF 56319
		surrogate_low_first  = 0b11011100'00000000u, // DC00 56320
		surrogate_low_last   = 0b11011111'11111111u, // DFFF 57343

		surrogate_first      = surrogate_high_first,
		surrogate_last       = surrogate_low_last,

		invalid_first        = 0b11011100'10000000u, // DC80 56448
		invalid_last         = 0b11011100'11111111u; // DCFF 56575
}

namespace utf8
{
	// https://en.wikipedia.org/wiki/UTF-8

	// In WTF-8 (Wobbly Transformation Format, 8-bit) unpaired surrogate halves (U+D800 through U+DFFF) are allowed.
	// This is necessary to store possibly-invalid UTF-16, such as Windows filenames.
	// Many systems that deal with UTF-8 work this way without considering it a different encoding, as it is simpler.
	static constexpr auto support_unpaired_surrogates = true;

	// Version 3 of the Python programming language treats each byte of an invalid UTF-8 bytestream as an error;
	// this gives 128 different possible errors. Extensions have been created to allow any byte sequence that is assumed
	// to be UTF-8 to be lossless transformed to UTF-16 or UTF-32, by translating the 128 possible error bytes to
	// reserved code points, and transforming those code points back to error bytes to output UTF-8.
	// The most common approach is to translate the codes to U+DC80...U+DCFF which are low (trailing) surrogate values
	// and thus "invalid" UTF-16, as used by Python's PEP 383 (or "surrogateescape") approach.
	static constexpr auto support_embedded_raw_bytes = true;

	static_assert(support_unpaired_surrogates && support_embedded_raw_bytes);

	static constexpr bool is_ascii_byte(unsigned int c)
	{
		return c < 0b10000000;
	}

	static constexpr bool is_continuation_byte(unsigned char c)
	{
		return (c & 0b11000000) == 0b10000000;
	}

	namespace detail
	{
		template<size_t continuation_bytes>
		static constexpr unsigned int extract_leading_bits(unsigned char const Char)
		{
			return (Char & (0b11111111 >> (continuation_bytes + 2))) << (6 * continuation_bytes);
		}

		template<size_t... I>
		static constexpr unsigned int extract_continuation_bits_impl(std::index_sequence<I...>, auto... Bytes)
		{
			return (... | ((Bytes & 0b00111111) << (6 * (sizeof...(Bytes) - 1 - I))));
		}

		template<typename... bytes>
		static constexpr unsigned int extract_continuation_bits(bytes... Bytes)
		{
			static_assert(sizeof...(bytes) > 0);
			return extract_continuation_bits_impl(std::index_sequence_for<bytes...>{}, Bytes...);
		}

		template<size_t total>
		static constexpr unsigned char make_leading_byte(unsigned int const Char)
		{
			return ((0b11111111 << (8 - total)) & 0b11111111) | (Char >> (6 * (total - 1)));
		}

		template<size_t index>
		static constexpr unsigned char make_continuation_byte(unsigned int const Char)
		{
			return 0b10000000 | ((Char >> (index * 6)) & 0b00111111);
		}

		template<size_t... I>
		static void write_continuation_bytes(unsigned int const Char, std::output_iterator<char> auto& Iterator, std::index_sequence<I...>)
		{
			(..., (*Iterator++ = make_continuation_byte<sizeof...(I) - 1 - I>(Char)));
		}
	}

	static constexpr unsigned int extract(unsigned char const Byte, auto... Bytes)
	{
		static_assert(sizeof...(Bytes) < 4);
		return detail::extract_leading_bits<sizeof...(Bytes)>(Byte) | detail::extract_continuation_bits(Bytes...);
	}

	template<size_t total>
	static void write(unsigned int const Char, std::output_iterator<char> auto& Iterator)
	{
		if constexpr (total == 1)
		{
			*Iterator++ = Char;
		}
		else
		{
			*Iterator++ = detail::make_leading_byte<total>(Char);
			detail::write_continuation_bytes(Char, Iterator, std::make_index_sequence<total - 1>{});
		}
	}

	static size_t wchars_count(std::string_view const Str)
	{
		size_t Chars = 0;
		size_t Pairs = 0;

		for (const auto Char: Str)
		{
			Chars += static_cast<size_t>(!is_continuation_byte(Char));
			Pairs += static_cast<size_t>((Char & 0b11111000) == 0b11110000);
		}

		return Chars + Pairs;
	}
}

size_t Utf8::get_char(
	std::string_view::const_iterator& StrIterator,
	std::string_view::const_iterator const StrEnd,
	wchar_t& First,
	wchar_t& Second,
	encoding::diagnostics& Diagnostics
)
{
	const auto InvalidChar = [&](unsigned char const Char, size_t const Position)
	{
		First = utf8::support_embedded_raw_bytes?
			utf16::surrogate_low_first | Char :
			encoding::replace_char;

		Diagnostics.ErrorPosition = Position;
		Diagnostics.set_is_utf8(encoding::is_utf8::no);
		return 1;
	};

	const unsigned char c1 = *StrIterator++;

	if (utf8::is_ascii_byte(c1))
	{
		First = c1;
		return 1;
	}

	// illegal 1-st byte
	if (c1 < 0b11000010 || c1 > 0b11110100)
		return InvalidChar(c1, 0);

	const auto Unfinished = [&](size_t const Position)
	{
		Second = 0;
		Diagnostics.PartialInput = 1;
		Diagnostics.PartialOutput = 1;
		return InvalidChar(c1, Position);
	};

	// multibyte (2, 3, 4)
	if (StrIterator == StrEnd)
		return Unfinished(1);

	const unsigned char c2 = *StrIterator;

	if (
		 c2 <  0b10000000 || c2 > 0b10111111  || // illegal 2-nd byte
		(c1 == 0b11100000 && c2 < 0b10100000) || // illegal 3-byte start (overlaps with 2-byte)
		(c1 == 0b11110000 && c2 < 0b10010000) || // illegal 4-byte start (overlaps with 3-byte)
		(c1 == 0b11110100 && c2 > 0b10001111)    // illegal 4-byte (out of unicode range)
	)
		return InvalidChar(c1, 1);

	if (c1 <= 0b11011111)
	{
		// legal 2-byte
		First = utf8::extract(c1, c2);
		++StrIterator;
		Diagnostics.set_is_utf8(encoding::is_utf8::yes);
		return 1;
	}

	// 3 or 4-byte
	if (StrIterator + 1 == StrEnd)
		return Unfinished(2);

	const unsigned char c3 = *(StrIterator + 1);

	// illegal 3-rd byte
	if (!utf8::is_continuation_byte(c3))
		return InvalidChar(c1, 1);

	if (c1 <= 0b11101111)
	{
		// legal 3-byte
		First = utf8::extract(c1, c2, c3);

		// invalid: surrogate area code
		if (in_closed_range(utf16::surrogate_first, First, utf16::surrogate_last))
		{
			Diagnostics.set_is_utf8(encoding::is_utf8::no);

			if constexpr (!utf8::support_unpaired_surrogates)
				return InvalidChar(c1, 2);
		}

		StrIterator += 2;
		Diagnostics.set_is_utf8(encoding::is_utf8::yes);
		return 1;
	}

	// 4-byte
	if (StrIterator + 2 == StrEnd)
		return Unfinished(3);

	const unsigned char c4 = *(StrIterator + 2);

	// illegal 4-th byte
	if (!utf8::is_continuation_byte(c4))
		return InvalidChar(c1, 3);

	// legal 4-byte (produces 2 WCHARs)
	std::tie(First, Second) = encoding::utf16::to_surrogate(utf8::extract(c1, c2, c3, c4));
	StrIterator += 3;
	Diagnostics.set_is_utf8(encoding::is_utf8::yes);
	return 2;
}

size_t Utf8::get_chars(std::string_view const Str, std::span<wchar_t> const Buffer, int& Tail)
{
	auto StrIterator = Str.begin();
	const auto StrEnd = Str.end();

	auto BufferIterator = Buffer.begin();
	const auto BufferEnd = Buffer.end();

	const auto StoreChar = [&](wchar_t Char)
	{
		if (BufferIterator != BufferEnd)
		{
			*BufferIterator++ = Char;
			return true;
		}
		return false;
	};

	while (StrIterator != StrEnd)
	{
		wchar_t First, Second;
		encoding::diagnostics Diagnostics;
		const auto NumberOfChars = get_char(StrIterator, StrEnd, First, Second, Diagnostics);

		if (!StoreChar(NumberOfChars == 1 || BufferIterator + 1 != BufferEnd? First : encoding::replace_char))
			break;

		if (NumberOfChars == 2)
		{
			if (!StoreChar(Second))
				break;
		}
	}

	Tail = StrEnd - StrIterator;
	return BufferIterator - Buffer.begin();
}

static size_t utf8_get_chars(std::string_view const Str, std::span<wchar_t> const Buffer, encoding::diagnostics* const Diagnostics)
{
	return BytesToUnicode(Str, Buffer, [](std::string_view::const_iterator const Iterator, std::string_view::const_iterator const End, std::span<wchar_t> CharBuffer, int&, encoding::diagnostics& Diagnostics)
	{
		auto NextIterator = Iterator;
		(void)Utf8::get_char(NextIterator, End, CharBuffer[0], CharBuffer[1], Diagnostics);
		return static_cast<size_t>(NextIterator - Iterator);
	}, Diagnostics);
}

static size_t utf8_get_bytes(string_view const Str, std::span<char> const Buffer)
{
	auto StrIterator = Str.begin();
	const auto StrEnd = Str.end();

	auto BufferIterator = Buffer.begin();
	size_t RequiredCapacity = 0;
	auto AvailableCapacity = Buffer.size();

	while (StrIterator != StrEnd)
	{
		unsigned int Char = *StrIterator++;

		size_t BytesNumber;

		if (utf8::is_ascii_byte(Char))
		{
			BytesNumber = 1;
		}
		else if (Char < 0b1000'00000000)
		{
			BytesNumber = 2;
		}
		else if (!in_closed_range(utf16::surrogate_first, Char, utf16::surrogate_last))
		{
			// not surrogates
			BytesNumber = 3;
		}
		else if (utf8::support_embedded_raw_bytes && in_closed_range(utf16::invalid_first, Char, utf16::invalid_last))
		{
			// embedded raw byte
			BytesNumber = 1;
			Char &= 0b11111111;
		}
		else if (StrIterator != StrEnd && encoding::utf16::is_valid_surrogate_pair(Char, *StrIterator))
		{
			// valid surrogate pair
			BytesNumber = 4;
			Char = encoding::utf16::extract_codepoint(Char, *StrIterator++);
		}
		else
		{
			BytesNumber = 3;

			if constexpr (!utf8::support_unpaired_surrogates)
			{
				Char = encoding::replace_char;
			}
		}

		RequiredCapacity += BytesNumber;

		if (AvailableCapacity < BytesNumber)
		{
			continue;
		}

		AvailableCapacity -= BytesNumber;

		switch (BytesNumber)
		{
		case 1: utf8::write<1>(Char, BufferIterator); break;
		case 2: utf8::write<2>(Char, BufferIterator); break;
		case 3: utf8::write<3>(Char, BufferIterator); break;
		case 4: utf8::write<4>(Char, BufferIterator); break;
		}
	}

	return RequiredCapacity;
}

bool encoding::utf16::is_high_surrogate(wchar_t const Char)
{
	return in_closed_range(::utf16::surrogate_high_first, Char, ::utf16::surrogate_high_last);
}

bool encoding::utf16::is_low_surrogate(wchar_t const Char)
{
	return in_closed_range(::utf16::surrogate_low_first, Char, ::utf16::surrogate_low_last);
}

bool encoding::utf16::is_valid_surrogate_pair(wchar_t const First, wchar_t const Second)
{
	return is_high_surrogate(First) && is_low_surrogate(Second);
}

char32_t encoding::utf16::extract_codepoint(wchar_t const First, wchar_t const Second)
{
	static_assert(sizeof(wchar_t) == 2);

	return 0b1'00000000'00000000u + ((First - ::utf16::surrogate_high_first) << 10) + (Second - ::utf16::surrogate_low_first);
}

char32_t encoding::utf16::extract_codepoint(string_view const Str)
{
	static_assert(sizeof(wchar_t) == 2);

	return Str.size() > 1 && is_valid_surrogate_pair(Str[0], Str[1])?
		extract_codepoint(Str[0], Str[1]) :
		Str.front();
}

void encoding::utf16::remove_first_codepoint(string_view& Str)
{
	const auto IsSurrogate = Str.size() > 1 && is_valid_surrogate_pair(Str[0], Str[1]);
	Str.remove_prefix(IsSurrogate? 2 : 1);
}

void encoding::utf16::remove_last_codepoint(string_view& Str)
{
	const auto Size = Str.size();
	const auto IsSurrogate = Size > 1 && is_valid_surrogate_pair(Str[Size - 2], Str[Size - 1]);
	Str.remove_suffix(IsSurrogate? 2 : 1);
}

std::pair<wchar_t, wchar_t> encoding::utf16::to_surrogate(char32_t const Codepoint)
{
	if (Codepoint <= std::numeric_limits<char16_t>::max())
		return { static_cast<wchar_t>(Codepoint), 0 };

	const auto TwentyBits = Codepoint - 0b1'00000000'00000000u;
	const auto TenBitsMask = 0b11'11111111;
	return
	{
		static_cast<wchar_t>(::utf16::surrogate_high_first | ((TwentyBits >> 10) & TenBitsMask)),
		static_cast<wchar_t>(::utf16::surrogate_low_first | (TwentyBits & TenBitsMask))
	};
}

void swap_bytes(void const* Src, void* const Dst, size_t const SizeInBytes, size_t const ElementSize)
{
	if (!SizeInBytes)
		return;

	assert(SizeInBytes > 1);
	assert(ElementSize > 1);
	assert(!(ElementSize & 1));
	assert(ElementSize <= SizeInBytes);
	assert(SizeInBytes % ElementSize == 0);

	const auto SrcBytes = static_cast<char const*>(Src);
	const auto DstBytes = static_cast<char*>(Dst);

	for (size_t i = 0; i != SizeInBytes; i += ElementSize)
	{
		for (size_t j = 0; j != ElementSize / 2; ++j)
		{
			const auto
				LeftIndex = i + j,
				RightIndex = i + ElementSize - 1 - j;

			// Src and Dst could overlap
			const auto
				Left = SrcBytes[LeftIndex],
				Right = SrcBytes[RightIndex];

			DstBytes[LeftIndex] = Right;
			DstBytes[RightIndex] = Left;
		}
	}
}

/*
	1 byte:  0xxxxxxx
	2 bytes: 110xxxxx 10xxxxxx
	3 bytes: 1110xxxx 10xxxxxx 10xxxxxx
	4 bytes: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

	1 byte, 7 bits:
	00000000                                                 00000000     00
	01111111                                                 01111111     7F
	 ^^^^^^^
	2 bytes, 5 + 6 = 11 bits:
	11000010 10000000                                    000'10000000    080
	11011111 10111111                                    111'11111111    7FF
	   ^^^^^   ^^^^^^
	3 bytes, 4 + 6 + 6 = 16 bits:
	11100000 10100000 10000000                      00001000'00000000   0800
	11101111 10111111 10111111                      11111111'11111111   FFFF
	    ^^^^   ^^^^^^   ^^^^^^
	4 bytes, 3 + 6 + 6 + 6 = 21 bits:
	11110000 10010000 10000000 10000000       00001'00000000'00000000 010000
	11110100 10001111 10111111 10111111       10000'11111111'11111111 10FFFF
	     ^^^   ^^^^^^   ^^^^^^   ^^^^^^
*/

encoding::is_utf8 encoding::is_valid_utf8(std::string_view const Str, bool const PartialContent)
{
	bool Ascii = true;
	size_t ContinuationBytes = 0;
	const unsigned char Min = 0b10000000, Max = 0b10111111;
	auto NextMin = Min, NextMax = Max;

	for (const unsigned char c: Str)
	{
		if (ContinuationBytes)
		{
			if (!::utf8::is_continuation_byte(c))
				return is_utf8::no;

			if (c < NextMin || c > NextMax)
				return is_utf8::no;

			NextMin = Min;
			NextMax = Max;

			--ContinuationBytes;
			continue;
		}

		const auto BytesCount = std::countl_one(c);
		if (!BytesCount)
			continue;

		ContinuationBytes = BytesCount - 1;

		Ascii = false;

		NextMin = Min;
		NextMax = Max;

		switch (ContinuationBytes)
		{
		default:
			return is_utf8::no;

		case 1:
			if (c < 0b11000010)
				return is_utf8::no;
			break;

		case 2:
			if (c == 0b11100000)
				NextMin = 0b10100000;
			break;

		case 3:
			if (c > 0b11110100)
				return is_utf8::no;
			if (c == 0b11110000)
				NextMin = 0b10010000;
			else if (c == 0b11110100)
				NextMax = 0b10001111;
			break;
		}
	}

	if (Ascii)
		return is_utf8::yes_ascii;

	if (!ContinuationBytes || PartialContent)
		return is_utf8::yes;

	return is_utf8::no;
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("encoding.basic")
{
	static const struct
	{
		std::string_view Str;
		string_view WideStr;
	}
	Tests[]
	{
#define INIT(x) { x, L ## x }
		INIT(""),
		INIT("0123456789"),
		INIT("ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
#undef INIT
	};

	std::array const Codepages
	{
		static_cast<uintptr_t>(CP_UTF8),
		encoding::codepage::ansi(),
		encoding::codepage::oem()
	};

	for (const auto& Codepage: Codepages)
	{
		for (const auto& i: Tests)
		{
			{
				auto WideStr = encoding::get_chars(Codepage, i.Str);
				REQUIRE(i.WideStr == WideStr);

				auto Str = encoding::get_bytes(Codepage, i.WideStr);
				REQUIRE(i.Str == Str);
			}

			{
				string WideStr;
				encoding::get_chars(Codepage, i.Str, WideStr);
				REQUIRE(i.WideStr == WideStr);

				std::string Str;
				encoding::get_bytes(Codepage, i.WideStr, Str);
				REQUIRE(i.Str == Str);
			}
		}
	}
}

TEST_CASE("encoding.utf8")
{
	using encoding::is_utf8;

	static const struct
	{
		is_utf8 IsUtf8;
		std::string_view Str;
	}
	Tests[]
	{
		{ is_utf8::yes, R"(
ᚠᛇᚻ᛫ᛒᛦᚦ᛫ᚠᚱᚩᚠᚢᚱ᛫ᚠᛁᚱᚪ᛫ᚷᛖᚻᚹᛦᛚᚳᚢᛗ
ᛋᚳᛖᚪᛚ᛫ᚦᛖᚪᚻ᛫ᛗᚪᚾᚾᚪ᛫ᚷᛖᚻᚹᛦᛚᚳ᛫ᛗᛁᚳᛚᚢᚾ᛫ᚻᛦᛏ᛫ᛞᚫᛚᚪᚾ
ᚷᛁᚠ᛫ᚻᛖ᛫ᚹᛁᛚᛖ᛫ᚠᚩᚱ᛫ᛞᚱᛁᚻᛏᚾᛖ᛫ᛞᚩᛗᛖᛋ᛫ᚻᛚᛇᛏᚪᚾ᛬
)"sv },

		{ is_utf8::yes, R"(
぀ ぁ あ ぃ い ぅ う ぇ え ぉ お か が き ぎ く
ぐ け げ こ ご さ ざ し じ す ず せ ぜ そ ぞ た
だ ち ぢ っ つ づ て で と ど な に ぬ ね の は
ば ぱ ひ び ぴ ふ ぶ ぷ へ べ ぺ ほ ぼ ぽ ま み
む め も ゃ や ゅ ゆ ょ よ ら り る れ ろ ゎ わ
ゐ ゑ を ん ゔ ゕ ゖ ゗ ゘ ゙ ゚ ゛ ゜ ゝ ゞ ゟ
)"sv },

		{ is_utf8::yes, R"(
゠ ァ ア ィ イ ゥ ウ ェ エ ォ オ カ ガ キ ギ ク
グ ケ ゲ コ ゴ サ ザ シ ジ ス ズ セ ゼ ソ ゾ タ
ダ チ ヂ ッ ツ ヅ テ デ ト ド ナ ニ ヌ ネ ノ ハ
バ パ ヒ ビ ピ フ ブ プ ヘ ベ ペ ホ ボ ポ マ ミ
ム メ モ ャ ヤ ュ ユ ョ ヨ ラ リ ル レ ロ ヮ ワ
ヰ ヱ ヲ ン ヴ ヵ ヶ ヷ ヸ ヹ ヺ ・ ー ヽ ヾ ヿ
)"sv },

		// Surrogate half width
		{ is_utf8::yes, R"(
𑀐 𑀑 𑀒 𑀓 𑀔 𑀕 𑀖 𑀗 𑀘 𑀙 𑀚 𑀛 𑀜 𑀝 𑀞 𑀟
𑀠 𑀡 𑀢 𑀣 𑀤 𑀥 𑀦 𑀧 𑀨 𑀩 𑀪 𑀫 𑀬 𑀭 𑀮 𑀯
𑀰 𑀱 𑀲 𑀳 𑀴 𑀵 𑀶 𑀷 𑀸 𑀹 𑀺 𑀻 𑀼 𑀽 𑀾 𑀿
)"sv },

		// Surrogate full width
		{ is_utf8::yes, R"(
𠜎 𠜱 𠝹 𠱓 𠱸 𠲖 𠳏 𠳕 𠴕 𠵼 𠵿 𠸎
𠸏 𠹷 𠺝 𠺢 𠻗 𠻹 𠻺 𠼭 𠼮 𠽌 𠾴 𠾼
𠿪 𡁜 𡁯 𡁵 𡁶 𡁻 𡃁 𡃉 𡇙 𢃇 𢞵 𢫕
𢭃 𢯊 𢱑 𢱕 𢳂 𢴈 𢵌 𢵧 𢺳 𣲷 𤓓 𤶸
𤷪 𥄫 𦉘 𦟌 𦧲 𦧺 𧨾 𨅝 𨈇 𨋢 𨳊 𨳍
)"sv },

		{ is_utf8::yes_ascii, R"(
Lorem ipsum dolor sit amet,
consectetur adipiscing elit,
sed do eiusmod tempor incididunt
ut labore et dolore magna aliqua.
)"sv },
		{ is_utf8::yes, "φ"sv },
		{ is_utf8::no, "\x80"sv },
		{ is_utf8::no, "\xFF"sv },
		{ is_utf8::no, "\xC0"sv },
		{ is_utf8::no, "\xC1"sv },
		{ is_utf8::no, "\xC2\x20"sv },
		{ is_utf8::no, "\xC2\xC0"sv },
		{ is_utf8::no, "\xE0\xC0\xC0"sv },
		{ is_utf8::no, "\xEB\x20\xA8"sv },
		{ is_utf8::no, "\xEB\xA0\x28"sv },
		{ is_utf8::no, "\xF0\xC0\xC0\xC0"sv },
		{ is_utf8::no, "\xF4\xBF\xBF\xBF"sv },
		{ is_utf8::no, "\xF0\xA0\xA0\x20"sv },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.IsUtf8 == encoding::is_valid_utf8(i.Str, false));

		const auto Str = encoding::utf8::get_chars(i.Str);

		if (i.IsUtf8 == is_utf8::yes)
		{
			REQUIRE(utf8::wchars_count(i.Str) == Str.size());
		}

		if (utf8::support_embedded_raw_bytes)
		{
			// Lossless
			const auto Bytes = encoding::utf8::get_bytes(Str);
			REQUIRE(i.Str == Bytes);
		}
		else
		{
			// Lossy
			if (i.IsUtf8 == is_utf8::no)
				REQUIRE(contains(Str, encoding::replace_char));
		}
	}
}

TEST_CASE("encoding.ucs2-utf8.round-trip")
{
	const auto round_trip = [](wchar_t const Char)
	{
		char Bytes[4];
		const auto Size = encoding::utf8::get_bytes({ &Char, 1 }, Bytes);
		assert(Size);
		assert(Size <= std::size(Bytes));

		wchar_t Result;
		[[maybe_unused]] const auto ResultSize = encoding::utf8::get_chars({ Bytes, Size }, { &Result, 1 });
		assert(ResultSize == 1u);

		return Result;
	};

	const auto AllValid = std::ranges::all_of(std::views::iota(0, std::numeric_limits<wchar_t>::max() + 1), [&](wchar_t const Char)
	{
		const auto Result = round_trip(Char);

		if constexpr (utf8::support_unpaired_surrogates)
		{
			return Result == Char;
		}
		else
		{
			const auto
				IsSurrogate = in_closed_range(utf16::surrogate_first, Char, utf16::surrogate_last),
				IsInvalid = in_closed_range(utf16::invalid_first, Char, utf16::invalid_last);

			return Result == (!IsSurrogate || (utf8::support_embedded_raw_bytes && IsInvalid)? Char : encoding::replace_char);
		}
	});

	REQUIRE(AllValid);
}

TEST_CASE("encoding.utf8-ucs2.round-trip")
{
	const auto there = [](char const Byte)
	{
		wchar_t Char;
		[[maybe_unused]] const auto Size = encoding::utf8::get_chars({ &Byte, 1 }, { &Char, 1 });
		assert(Size == 1u);
		return Char;
	};

	const auto back = [](wchar_t const Char)
	{
		char Byte;
		[[maybe_unused]] const auto Size = encoding::utf8::get_bytes({ &Char, 1 }, { &Byte, 1 });
		assert(Size == 1u);
		return Byte;
	};

	const auto AllValid = std::ranges::all_of(std::views::iota(0, std::numeric_limits<char>::max() + 1), [&](char const Byte)
	{
		if (!(Byte & 0b10000000) || utf8::support_embedded_raw_bytes)
		{
			return back(there(Byte)) == Byte;
		}
		else if (!utf8::support_embedded_raw_bytes)
		{
			return there(Byte) == encoding::replace_char;
		}
	});

	REQUIRE(AllValid);
}

TEST_CASE("encoding.errors")
{
	static const struct
	{
		unsigned Codepage;
		std::string_view Bytes;
		size_t ErrorPosition, PartialInput, PartialOutput;
	}
	Tests[]
	{
		{ 932,   "\xE0"sv, },
		{ 936,   "\xDB"sv, },
		{ 949,   "\x97"sv, },
		{ 950,   "\x81"sv, },
		{ 1361,  "\x84"sv, },
		{ 10001, "\x85"sv, },
		{ 10002, "\x81"sv, },
		{ 20000, "\xED"sv, },
		{ 20001, "\xED"sv, },
		{ 20003, "\xFB"sv, },
		{ 20004, "\xED"sv, },
		{ 57011, "\xA0"sv, },

		{ 65001, "\xF4"sv,         1, 1, 1 },
		{ 65001, "\xF4\x8F"sv,     2, 2, 2 },
		{ 65001, "\xF4\x8F\xBF"sv, 3, 3, 3 },
	};

	const auto Prefix = "0123"sv;
	const auto ExpectedTemplate = L"0123???"sv;

	for (const auto& i: Tests)
	{
		encoding::diagnostics Diagnostics;
		const auto Bytes = Prefix + i.Bytes;
		auto Str = encoding::get_chars(i.Codepage, Bytes, &Diagnostics);
		const auto ReplaceChars = i.PartialOutput? i.PartialOutput : 1;
		std::ranges::fill_n(Str.begin() + Prefix.size(), ReplaceChars, L'?');
		const auto Expected = ExpectedTemplate.substr(0, Prefix.size() + ReplaceChars);

		REQUIRE(Str == Expected);
		REQUIRE(Diagnostics.ErrorPosition == Prefix.size() + i.ErrorPosition);
		REQUIRE(Diagnostics.PartialInput == i.PartialInput);
		REQUIRE(Diagnostics.PartialOutput == i.PartialOutput);
	}
}

TEST_CASE("encoding.utf7.valid")
{
	static const struct
	{
		std::string_view Bytes;
		string_view Chars;
		bool OneWay;
	}
	Tests[]
	{
		{ {},                                     {} },
		{ "."sv,                                  L"."sv, },
		{ " \t\r\n"sv,                            L" \t\r\n"sv, },
		{ "+AKM-1"sv,                             L"£1"sv, },
		{ "A+ImIDkQ-"sv,                          L"A≢Α"sv, },
		{ "A+ImIDkQ."sv,                          L"A≢Α."sv, true, },
		{ "+ADw- and +AD4-"sv,                    L"< and >"sv, },
		{ "+ZeVnLIqe-"sv,                         L"日本語"sv, },
		{ "Hello, World+ACE-"sv,                  L"Hello, World!"sv, },
		{ "INBOX"sv,                              L"INBOX"sv, },
		{ "Bo+AO4-te de r+AOk-ception"sv,         L"Boîte de réception"sv, },
		{ "+U9dP4TDIMOwwpA-"sv,                   L"受信トレイ"sv, },
		{ "+2Dzfttg838HYPN/H-"sv,                 L"🎶🏁🏇"sv, },
		{ "This+-That-"sv,                        L"This+That-"sv, },
		{ "+/v8"sv,                               L"\xFEFF"sv, true, },
		{ "+/v9"sv,                               L"\xFEFF"sv, true, },
		{ "+/v+"sv,                               L"\xFEFF"sv, true, },
		{ "+/v/"sv,                               L"\xFEFF"sv, true, },
		{ "+/v8-"sv,                              L"\xFEFF"sv, },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(encoding::get_chars(CP_UTF7, i.Bytes) == i.Chars);

		if (!i.OneWay)
			REQUIRE(encoding::get_bytes(CP_UTF7, i.Chars) == i.Bytes);
	}
}

TEST_CASE("encoding.raw_eol")
{
	static const struct
	{
		unsigned Codepage;
		char Cr, Lf;
	}
	Tests[]
	{
		{CP_ACP,   '\r', '\n' },
		{CP_OEMCP, '\r', '\n' },
		{37,       '\r', '%'  },
		{500,      '\r', '%'  },
	};

	for (const auto& i: Tests)
	{
		raw_eol Eol(i.Codepage);

		REQUIRE(Eol.cr() == i.Cr);
		REQUIRE(Eol.lf() == i.Lf);
	}
}

TEST_CASE("encoding.utf16.surrogate")
{
	static const struct
	{
		char32_t Codepoint;
		std::array<wchar_t, 2> Pair;
	}
	Tests[]
	{
		{ U'\U00000000', {L'\x0000', L'\x0000'} },
		{ U'\U00010000', {L'\xD800', L'\xDC00'} },
		{ U'\U0002070E', {L'\xD841', L'\xDF0E'} },
		{ U'\U0010FFFF', {L'\xDBFF', L'\xDFFF'} },
	};

	for (const auto& i: Tests)
	{
		const auto Codepoint = encoding::utf16::extract_codepoint({ i.Pair.data(), i.Pair.size() });
		REQUIRE(i.Codepoint == Codepoint);

		const auto Pair = encoding::utf16::to_surrogate(i.Codepoint);
		REQUIRE(i.Pair[0] == Pair.first);
		REQUIRE(i.Pair[1] == Pair.second);
	}
}

TEST_CASE("encoding.utf8_or_ansi")
{
	#define UTF8_SAMPLE "です"
	REQUIRE(WIDE_SV(UTF8_SAMPLE) == encoding::utf8_or_ansi::get_chars(CHAR_SV(UTF8_SAMPLE)));
	#undef UTF8_SAMPLE

	const auto OpaqueSample = "\xC0\xC1\xC2\xC3\xC4"sv;
	REQUIRE(encoding::ansi::get_chars(OpaqueSample) == encoding::utf8_or_ansi::get_chars(OpaqueSample));
}

TEST_CASE("encoding.utf16.incomplete_bytes")
{
	static const struct
	{
		string_view Str;
		size_t ExpectedSize;
		std::string_view ExpectedBytesLe, ExpectedBytesBe;
	}
	Tests[]
	{
		{ L"A"sv,          2, "\x41"sv,         "\x00"sv },
		{ L"⅀"sv,          2, "\x40"sv,         "\x21"sv },
		{ L"\U0010FFFF"sv, 4, "\xFF\xDB\xFF"sv, "\xDB\xFF\xDF"sv },
	};

	std::string Buffer;

	for (const auto& i: Tests)
	{
		Buffer.resize(i.Str.size() * sizeof(char16_t) - 1);

		{
			const auto Size = encoding::get_bytes(CP_UTF16LE, i.Str, std::span(Buffer));
			REQUIRE(Size == i.ExpectedSize);
			REQUIRE(Buffer == i.ExpectedBytesLe);
		}

		{
			const auto Size = encoding::get_bytes(CP_UTF16BE, i.Str, std::span(Buffer));
			REQUIRE(Size == i.ExpectedSize);
			REQUIRE(Buffer == i.ExpectedBytesBe);
		}
	}
}

TEST_CASE("encoding.utf16.incomplete_chars")
{
	static const struct
	{
		std::string_view Bytes;
		size_t ExpectedSize;
		string_view ExpectedStrLe, ExpectedStrBe;
	}
	Tests[]
	{
		{ "\xAB"sv,         2, L"\x00AB"sv,       L"\xAB00"sv },
		{ "\xAB\xCD\xEF"sv, 4, L"\xCDAB\x00EF"sv, L"\xABCD\xEF00"sv },
	};

	for (const auto& i: Tests)
	{
		{
			encoding::diagnostics Diagnostics;
			const auto Str = encoding::get_chars(CP_UTF16LE, i.Bytes, &Diagnostics);
			REQUIRE(Str == i.ExpectedStrLe);
			REQUIRE(Diagnostics.ErrorPosition == i.ExpectedSize - 1);
			REQUIRE(Diagnostics.PartialInput == 1uz);
			REQUIRE(Diagnostics.PartialOutput == 1uz);
		}

		{
			encoding::diagnostics Diagnostics;
			const auto Str = encoding::get_chars(CP_UTF16BE, i.Bytes, &Diagnostics);
			REQUIRE(Str == i.ExpectedStrBe);
			REQUIRE(Diagnostics.ErrorPosition == i.ExpectedSize - 1);
			REQUIRE(Diagnostics.PartialInput == 1uz);
			REQUIRE(Diagnostics.PartialOutput == 1uz);
		}
	}
}

TEST_CASE("encoding.swap_bytes")
{
	const auto Input =
		"\x01\x23\x45\x67\x89\xAB\xCD\xEF"
		"\x00\x11\x22\x33\x44\x55\x66\x77"
		"\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF"
		""sv;

	static const struct
	{
		std::string_view Expected;
		size_t Size;
	}
	Tests[]
	{
		{
			"\x23\x01\x67\x45\xAB\x89\xEF\xCD"
			"\x11\x00\x33\x22\x55\x44\x77\x66"
			"\x99\x88\xBB\xAA\xDD\xCC\xFF\xEE"
			""sv, 2
		},
		{
			"\x67\x45\x23\x01\xEF\xCD\xAB\x89"
			"\x33\x22\x11\x00\x77\x66\x55\x44"
			"\xBB\xAA\x99\x88\xFF\xEE\xDD\xCC"
			""sv, 4
		},
		{
			"\xEF\xCD\xAB\x89\x67\x45\x23\x01"
			"\x77\x66\x55\x44\x33\x22\x11\x00"
			"\xFF\xEE\xDD\xCC\xBB\xAA\x99\x88"
			""sv, 8
		}
	};

	for (const auto& i: Tests)
	{
		std::string Str(Input);
		swap_bytes(Str.data(), Str.data(), Input.size(), i.Size);
		REQUIRE(Str == i.Expected);
	}
}
#endif
