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

// Platform:

// Common:
#include "common/algorithm.hpp"
#include "common/function_ref.hpp"
#include "common/io.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

class installed_codepages
{
public:
	installed_codepages()
	{
		Context = this;
		EnumSystemCodePages(callback, CP_INSTALLED);
		Context = {};

		rethrow_if(m_ExceptionPtr);
	}

	const auto& get() const
	{
		return m_InstalledCp;
	}

private:
	static inline installed_codepages* Context;

	static BOOL WINAPI callback(wchar_t* const cpNum)
	{
		return Context->enum_cp_callback(cpNum);
	}

	BOOL enum_cp_callback(wchar_t const* cpNum)
	{
		return cpp_try(
		[&]
		{
			const auto cp = static_cast<unsigned>(std::wcstoul(cpNum, nullptr, 10));

			CPINFOEX cpix;
			if (!GetCPInfoEx(cp, 0, &cpix))
			{
				CPINFO cpi;
				if (!GetCPInfo(cp, &cpi))
					return TRUE;

				cpix.MaxCharSize = cpi.MaxCharSize;
				xwcsncpy(cpix.CodePageName, cpNum, std::size(cpix.CodePageName));
			}

			if (cpix.MaxCharSize < 1)
				return TRUE;

			string_view cp_data = cpix.CodePageName;
			// Windows: "XXXX (Name)", Wine: "Name"
			const auto OpenBracketPos = cp_data.find(L'(');
			if (OpenBracketPos != string::npos)
			{
				const auto CloseBracketPos = cp_data.rfind(L')');
				if (CloseBracketPos != string::npos && CloseBracketPos > OpenBracketPos)
				{
					cp_data = cp_data.substr(OpenBracketPos + 1, CloseBracketPos - OpenBracketPos - 1);
				}
			}

			m_InstalledCp[cp] = { string(cp_data), static_cast<unsigned char>(cpix.MaxCharSize) };
			return TRUE;
		},
		[&]
		{
			SAVE_EXCEPTION_TO(m_ExceptionPtr);
			return FALSE;
		});
	}

	cp_map m_InstalledCp;
	std::exception_ptr m_ExceptionPtr;
};

const cp_map& InstalledCodepages()
{
	static const installed_codepages s_Icp;
	return s_Icp.get();
}

cp_info const* GetCodePageInfo(uintptr_t cp)
{
	// Standard unicode CPs (1200, 1201, 65001) are NOT in the list.
	const auto& InstalledCp = InstalledCodepages();

	if (const auto found = InstalledCp.find(static_cast<unsigned>(cp)); found != InstalledCp.cend())
		return &found->second;

	return {};
}

template<typename range1, typename range2>
static std::optional<size_t> mismatch(range1 const& Range1, range2 const& Range2)
{
	const auto [Mismatch1, Mismatch2] = std::mismatch(ALL_CONST_RANGE(Range1), ALL_CONST_RANGE(Range2));

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

static size_t widechar_to_multibyte_with_validation(uintptr_t const Codepage, string_view const Str, span<char> Buffer, encoding::error_position* const ErrorPosition)
{
	if (ErrorPosition)
		ErrorPosition->reset();

	auto IsRetardedCodepage = IsNoFlagsCodepage(Codepage);
	BOOL DefaultCharUsed = FALSE;

	const auto convert = [&](span<char> const To)
	{
		for (;;)
		{
			if (const auto Result = WideCharToMultiByte(
				Codepage,
				IsRetardedCodepage? 0 : WC_NO_BEST_FIT_CHARS,
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
	if (Buffer.size() <= Str.size() && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		// If BufferSize is less than DataSize, this function writes the number of bytes specified by BufferSize to the buffer indicated by Buffer.
		// If the function succeeds and BufferSize is 0, the return value is the required size, in bytes, for the buffer indicated by Buffer.
		Result = convert({});
	}

	// The result is ok and can be trusted
	if (Result && !DefaultCharUsed && !IsRetardedCodepage)
		return Result;

	// They don't care, no point to go deeper
	if (!ErrorPosition)
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
		*ErrorPosition = *Pos;
	}

	return Result;
}

static size_t multibyte_to_widechar_with_validation(uintptr_t const Codepage, std::string_view Str, span<wchar_t> Buffer, encoding::error_position* const ErrorPosition)
{
	if (ErrorPosition)
		ErrorPosition->reset();

	auto IsRetardedCodepage = IsNoFlagsCodepage(Codepage);
	auto Strict = true;

	const auto convert = [&](span<wchar_t> const To)
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
	if (!ErrorPosition)
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
		*ErrorPosition = *Pos;
	}

	return Result;
}

static bool IsValid(unsigned cp)
{
	if (cp==CP_ACP || cp==CP_OEMCP || cp==CP_MACCP || cp==CP_THREAD_ACP || cp==CP_SYMBOL)
		return false;

	if (cp==CP_UTF8 || cp==CP_UNICODE || cp==CP_REVERSEBOM)
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
	u;

	int CharsProcessed = 0;
	size_t Size = 0;
	for (size_t i = 0; i != 65536; ++i) // only UCS2 range
	{
		encoding::error_position ErrorPosition;
		const auto Char = static_cast<wchar_t>(i);
		const auto CharSize = widechar_to_multibyte_with_validation(Codepage, { &Char, 1 }, u.Buffer, &ErrorPosition);
		if (!CharSize || ErrorPosition)
			continue;

		len_mask[u.b1] |= bit(CharSize - 1);
		++CharsProcessed;
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

static size_t utf8_get_bytes(string_view Str, span<char> Buffer);

static size_t get_bytes_impl(uintptr_t const Codepage, string_view const Str, span<char> Buffer, encoding::error_position* const ErrorPosition)
{
	if (Str.empty())
		return 0;

	switch(Codepage)
	{
	case CP_UTF8:
		return utf8_get_bytes(Str, Buffer);

	case CP_UNICODE:
	case CP_REVERSEBOM:
		{
			const auto Size = std::min(Str.size() * sizeof(wchar_t), Buffer.size());
			if (Codepage == CP_UNICODE)
			{
				copy_memory(Str.data(), Buffer.data(), Size);
			}
			else
				swap_bytes(Str.data(), Buffer.data(), Size);

			return Str.size() * sizeof(wchar_t);
		}

	default:
		return widechar_to_multibyte_with_validation(Codepage, Str, Buffer, ErrorPosition);
	}
}

uintptr_t encoding::codepage::ansi()
{
	return GetACP();
}

uintptr_t encoding::codepage::oem()
{
	return GetOEMCP();
}

size_t encoding::get_bytes(uintptr_t const Codepage, string_view const Str, span<char> const Buffer, error_position* const ErrorPosition)
{
	const auto Result = get_bytes_impl(Codepage, Str, Buffer, ErrorPosition);
	if (Result < Buffer.size())
	{
		Buffer[Result] = '\0';
	}
	return Result;
}

std::string encoding::get_bytes(uintptr_t const Codepage, string_view const Str, error_position* const ErrorPosition)
{
	if (Str.empty())
		return {};

	// DataSize is a good estimation for the bytes count.
	// With this approach we can fill the buffer with only one attempt in many cases.
	std::string Buffer(Str.size(), '\0');

	for (auto Overflow = true; Overflow;)
	{
		const auto Size = get_bytes(Codepage, Str, Buffer, ErrorPosition);
		Overflow = Size > Buffer.size();
		Buffer.resize(Size);
	}

	return Buffer;
}

size_t encoding::get_bytes_count(uintptr_t const Codepage, string_view const Str, error_position* ErrorPosition)
{
	return get_bytes(Codepage, Str, {}, ErrorPosition);
}

static size_t utf8_get_chars(std::string_view Str, span<wchar_t> Buffer, encoding::error_position* ErrorPosition);
static size_t utf7_get_chars(std::string_view Str, span<wchar_t> Buffer, encoding::error_position* ErrorPosition);

static size_t get_chars_impl(uintptr_t const Codepage, std::string_view Str, span<wchar_t> const Buffer, encoding::error_position* const ErrorPosition)
{
	if (Str.empty())
		return 0;

	switch (Codepage)
	{
	case CP_UTF8:
		return utf8_get_chars(Str, Buffer, ErrorPosition);

	case CP_UTF7:
		return utf7_get_chars(Str, Buffer, ErrorPosition);

	case CP_UNICODE:
		copy_memory(Str.data(), Buffer.data(), std::min(Str.size(), Buffer.size() * sizeof(wchar_t)));
		return Str.size() / sizeof(wchar_t);

	case CP_REVERSEBOM:
		swap_bytes(Str.data(), Buffer.data(), std::min(Str.size(), Buffer.size() * sizeof(wchar_t)));
		return Str.size() / sizeof(wchar_t);

	default:
		return multibyte_to_widechar_with_validation(Codepage, Str, Buffer, ErrorPosition);
	}
}

size_t encoding::get_chars(uintptr_t const Codepage, std::string_view const Str, span<wchar_t> const Buffer, error_position* const ErrorPosition)
{
	const auto Result = get_chars_impl(Codepage, Str, Buffer, ErrorPosition);
	if (Result < Buffer.size())
	{
		Buffer[Result] = {};
	}
	return Result;
}

size_t encoding::get_chars(uintptr_t const Codepage, bytes_view const Str, span<wchar_t> Buffer, error_position* const ErrorPosition)
{
	return get_chars(Codepage, to_string_view(Str), Buffer, ErrorPosition);
}

string encoding::get_chars(uintptr_t const Codepage, std::string_view const Str, error_position* const ErrorPosition)
{
	if (Str.empty())
		return {};

	const auto EstimatedCharsCount = [&]
	{
		switch (Codepage)
		{
		case CP_UNICODE:
		case CP_REVERSEBOM:
			return Str.size() / sizeof(wchar_t);

		case CP_UTF7:
		case CP_UTF8:
			// Even though DataSize is always >= BufferSize for these guys, we can't use DataSize for estimation - it can be three times larger than necessary.
			return get_chars_count(Codepage, Str, ErrorPosition);

		default:
			return Str.size();
		}
	};

	// With this approach we can fill the buffer with only one attempt in many cases.
	string Buffer(EstimatedCharsCount(), {});
	for (auto Overflow = true; Overflow;)
	{
		const auto Size = get_chars(Codepage, Str, Buffer, ErrorPosition);
		Overflow = Size > Buffer.size();
		Buffer.resize(Size);
	}

	return Buffer;
}

string encoding::get_chars(uintptr_t const Codepage, bytes_view const Str, error_position* const ErrorPosition)
{
	return get_chars(Codepage, to_string_view(Str), ErrorPosition);
}

size_t encoding::get_chars_count(uintptr_t const Codepage, std::string_view const Str, error_position* const ErrorPosition)
{
	return get_chars(Codepage, Str, {}, ErrorPosition);
}

size_t encoding::get_chars_count(uintptr_t const Codepage, bytes_view const Str, error_position* const ErrorPosition)
{
	return get_chars(Codepage, Str, {}, ErrorPosition);
}

std::string_view encoding::get_signature_bytes(uintptr_t Cp)
{
	switch (Cp)
	{
	case CP_UNICODE:
		return "\xFF\xFE"sv;
	case CP_REVERSEBOM:
		return "\xFE\xFF"sv;
	case CP_UTF8:
		return "\xEF\xBB\xBF"sv;
	default:
		return {};
	}
}

encoding::writer::writer(std::ostream& Stream, uintptr_t Codepage, bool AddSignature):
	m_Stream(&Stream),
	m_Codepage(Codepage),
	m_AddSignature(AddSignature)
{
}

void encoding::writer::write(const string_view Str)
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
	if (m_Codepage == CP_UNICODE)
		return io::write(*m_Stream, Str);

	// External buffer to minimise allocations
	m_Buffer.resize(m_Buffer.capacity());

	for (auto Overflow = true; Overflow;)
	{
		const auto Size = get_bytes(m_Codepage, Str, m_Buffer);
		Overflow = Size > m_Buffer.size();
		m_Buffer.resize(Size);
	}

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

constexpr short operator ""_D(unsigned long long const n)
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
static size_t Utf7_GetChar(std::string_view::const_iterator const Iterator, std::string_view::const_iterator const End, wchar_t* const Buffer, bool& ConversionError, int& state)
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
		ConversionError = true;
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
		ConversionError = true;
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
			*Buffer = static_cast<wchar_t>(c);
			return BytesConsumed;
		}
		if (DataSize < 2)
		{
			ConversionError = true;
			return BytesConsumed;
		}

		c = *StrIterator++;
		BytesConsumed = 2;
		if (c >= 128)
		{
			ConversionError = true;
			return BytesConsumed;
		}

		if (c == static_cast<BYTE>('-'))
		{
			*Buffer = L'+';
			return BytesConsumed;
		}

		m[0] = static_cast<int>(m7[c]);
		if (0 == (m[0] & b64))
		{
			ConversionError = true;
			return BytesConsumed;
		}

		u.s.base64 = true;
		u.s.carry_count = 0;
	}

	const auto a = 2 - u.s.carry_count / 4;
	if (BytesConsumed + a > DataSize)
	{
		ConversionError = true;
		return DataSize;
	}

	if ((c = *StrIterator++) >= 128)
	{
		u.s.base64 = false;
		state = u.state;
		ConversionError = true;
		return BytesConsumed;
	}
	m[1] = static_cast<int>(m7[c]);
	if (0 == (m[1] & b64))
	{
		u.s.base64 = false;
		state = u.state;
		ConversionError = true;
		return BytesConsumed;
	}
	if (a < 2)
	{
		*Buffer = static_cast<wchar_t>((u.s.carry_bits << 12) | (static_cast<BYTE>(m[0]) << 6) | static_cast<BYTE>(m[1]));
		u.s.carry_count = 0;
	}
	else
	{
		++BytesConsumed;
		if ((c = *StrIterator++) >= 128)
		{
			u.s.base64 = false;
			state = u.state;
			ConversionError = true;
			return BytesConsumed;
		}
		m[2] = static_cast<int>(m7[c]);
		if (0 == (m[2] & b64))
		{
			u.s.base64 = false;
			state = u.state;
			ConversionError = true;
			return BytesConsumed;
		}
		const unsigned m18 = (static_cast<BYTE>(m[0]) << 12) | (static_cast<BYTE>(m[1]) << 6) | static_cast<BYTE>(m[2]);

		if (u.s.carry_count == 0)
		{
			*Buffer = static_cast<wchar_t>(m18 >> 2);
			u.s.carry_bits = static_cast<BYTE>(m18 & 0x03);
			u.s.carry_count = 2;
		}
		else
		{
			*Buffer = static_cast<wchar_t>((u.s.carry_bits << 14) | (m18 >> 4));
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

static size_t BytesToUnicode(
	std::string_view const Str,
	span<wchar_t> const Buffer,
	function_ref<size_t(std::string_view::const_iterator, std::string_view::const_iterator, wchar_t*, bool&, int&)> const GetChar,
	encoding::error_position* const ErrorPosition)
{
	if (Str.empty())
		return 0;

	auto StrIterator = Str.begin();
	const auto StrEnd = Str.end();

	auto BufferIterator = Buffer.begin();
	const auto BufferEnd = Buffer.end();

	int State = 0;
	size_t RequiredSize = 0;

	while (StrIterator != StrEnd)
	{
		wchar_t TmpBuffer[2]{};
		auto ConversionError = false;
		const auto BytesConsumed = GetChar(StrIterator, StrEnd, TmpBuffer, ConversionError, State);

		if (!BytesConsumed)
			break;

		if (ConversionError)
		{
			TmpBuffer[0] = encoding::replace_char;

			if (ErrorPosition && !*ErrorPosition)
				*ErrorPosition = StrIterator - Str.begin();
		}

		const auto StoreChar = [&](wchar_t Char)
		{
			if (BufferIterator != BufferEnd)
			{
				*BufferIterator++ = Char;
			}
			++RequiredSize;
		};

		StoreChar(TmpBuffer[0]);

		if (TmpBuffer[1])
		{
			StoreChar(TmpBuffer[1]);
		}

		StrIterator += BytesConsumed;
	}

	return RequiredSize;
}

static size_t utf7_get_chars(std::string_view const Str, span<wchar_t> const Buffer, encoding::error_position* const ErrorPosition)
{
	return BytesToUnicode(Str, Buffer, Utf7_GetChar, ErrorPosition);
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

	const auto
		surrogate_high_first = 0b11011000'00000000u, // D800 55296
		surrogate_high_last  = 0b11011011'11111111u, // DBFF 56319
		surrogate_low_first  = 0b11011100'00000000u, // DC00 56320
		surrogate_low_last   = 0b11011111'11111111u, // DFFF 57343

		surrogate_first      = surrogate_high_first,
		surrogate_last       = surrogate_low_last,

		invalid_first        = 0b11011100'10000000u, // DC80 56448
		invalid_last         = 0b11011100'11111111u; // DCFF 56575

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

		template<size_t... I, typename... bytes>
		static constexpr unsigned int extract_continuation_bits_impl(std::index_sequence<I...>, bytes... Bytes)
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

		template<size_t... I, typename iterator>
		static void write_continuation_bytes(unsigned int const Char, iterator& Iterator, std::index_sequence<I...>)
		{
			(..., (*Iterator++ = make_continuation_byte<sizeof...(I) - 1 - I>(Char)));
		}
	}

	template<typename... bytes>
	static constexpr unsigned int extract(unsigned char const Byte, bytes... Bytes)
	{
		static_assert(sizeof...(Bytes) < 4);
		return detail::extract_leading_bits<sizeof...(Bytes)>(Byte) | detail::extract_continuation_bits(Bytes...);
	}

	template<size_t total, typename iterator>
	static void write(unsigned int const Char, iterator& Iterator)
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
}

size_t Utf8::get_char(std::string_view::const_iterator& StrIterator, std::string_view::const_iterator const StrEnd, wchar_t& First, wchar_t& Second)
{
	size_t NumberOfChars = 1;

	const auto InvalidChar = [](unsigned char c)
	{
		return utf8::support_embedded_raw_bytes?
			utf8::surrogate_low_first | c :
			encoding::replace_char;
	};

	const unsigned char c1 = *StrIterator++;

	if (utf8::is_ascii_byte(c1))
	{
		First = c1;
	}
	else if (c1 < 0b11000010 || c1 > 0b11110100)
	{
		// illegal 1-st byte
		First = InvalidChar(c1);
	}
	else
	{
		const auto Unfinished = [&]
		{
			First = InvalidChar(c1);
			return NumberOfChars;
		};

		// multibyte (2, 3, 4)
		if (StrIterator == StrEnd)
		{
			return Unfinished();
		}

		const unsigned char c2 = *StrIterator;

		if (
			 c2 <  0b10000000 || c2 > 0b10111111  || // illegal 2-nd byte
			(c1 == 0b11100000 && c2 < 0b10100000) || // illegal 3-byte start (overlaps with 2-byte)
			(c1 == 0b11110000 && c2 < 0b10010000) || // illegal 4-byte start (overlaps with 3-byte)
			(c1 == 0b11110100 && c2 > 0b10001111))   // illegal 4-byte (out of unicode range)
		{
			First = InvalidChar(c1);
		}
		else if (c1 <= 0b11011111)
		{
			// legal 2-byte
			First = utf8::extract(c1, c2);
			++StrIterator;
		}
		else
		{
			// 3 or 4-byte
			if (StrIterator + 1 == StrEnd)
			{
				return Unfinished();
			}

			const unsigned char c3 = *(StrIterator + 1);
			if (!utf8::is_continuation_byte(c3))
			{
				// illegal 3-rd byte
				First = InvalidChar(c1);
			}
			else if (c1 <= 0b11101111)
			{
				// legal 3-byte
				First = utf8::extract(c1, c2, c3);

				if constexpr (utf8::support_unpaired_surrogates)
				{
					StrIterator += 2;
				}
				else
				{
					if (in_closed_range(utf8::surrogate_first, First, utf8::surrogate_last))
					{
						// invalid: surrogate area code
						First = InvalidChar(c1);
					}
					else
					{
						StrIterator += 2;
					}
				}
			}
			else
			{
				// 4-byte
				if (StrIterator + 2 == StrEnd)
				{
					return Unfinished();
				}

				const unsigned char c4 = *(StrIterator + 2);
				if (!utf8::is_continuation_byte(c4))
				{
					// illegal 4-th byte
					First = InvalidChar(c1);
				}
				else
				{
					// legal 4-byte (produces 2 WCHARs)
					const auto FullChar = utf8::extract(c1, c2, c3, c4) - 0b1'00000000'00000000;

					First  = utf8::surrogate_high_first + (FullChar >> 10);
					Second = utf8::surrogate_low_first + (FullChar & 0b00000011'11111111);
					NumberOfChars = 2;
					StrIterator += 3;
				}
			}
		}
	}

	return NumberOfChars;
}

size_t Utf8::get_chars(std::string_view const Str, span<wchar_t> const Buffer, int& Tail)
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
		const auto NumberOfChars = get_char(StrIterator, StrEnd, First, Second);

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

static size_t utf8_get_chars(std::string_view const Str, span<wchar_t> const Buffer, encoding::error_position* const ErrorPosition)
{
	return BytesToUnicode(Str, Buffer, [](std::string_view::const_iterator const Iterator, std::string_view::const_iterator const End, wchar_t* CharBuffer, bool&, int&)
	{
		auto NextIterator = Iterator;
		(void)Utf8::get_char(NextIterator, End, CharBuffer[0], CharBuffer[1]);
		return static_cast<size_t>(NextIterator - Iterator);
	}, ErrorPosition);
}

static size_t utf8_get_bytes(string_view const Str, span<char> const Buffer)
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
		else if (!in_closed_range(utf8::surrogate_first, Char, utf8::surrogate_last))
		{
			// not surrogates
			BytesNumber = 3;
		}
		else if (utf8::support_embedded_raw_bytes && in_closed_range(utf8::invalid_first, Char, utf8::invalid_last))
		{
			// embedded raw byte
			BytesNumber = 1;
			Char &= 0b11111111;
		}
		else if (StrIterator != StrEnd &&
			in_closed_range(utf8::surrogate_high_first, Char, utf8::surrogate_high_last) &&
			in_closed_range(utf8::surrogate_low_first, *StrIterator, utf8::surrogate_low_last))
		{
			// valid surrogate pair
			BytesNumber = 4;
			Char = 0b1'00000000'00000000u + ((Char - utf8::surrogate_high_first) << 10) + (*StrIterator++ - utf8::surrogate_low_first);
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

void swap_bytes(const void* const Src, void* const Dst, const size_t SizeInBytes)
{
	if (!SizeInBytes)
	{
		// Ucrt for unknown reason aggressively validates that 'source' and 'destination' are not nullptr even if 'bytes' is 0.
		// It's safer to not call it.
		return;
	}
	_swab(static_cast<char*>(const_cast<void*>(Src)), static_cast<char*>(Dst), static_cast<int>(SizeInBytes));
}

bool IsVirtualCodePage(uintptr_t cp)
{
	return cp == CP_DEFAULT || cp == CP_REDETECT || cp == CP_ALL;
}

bool IsUnicodeCodePage(uintptr_t cp)
{
	return cp == CP_UNICODE || cp == CP_REVERSEBOM;
}

bool IsStandardCodePage(uintptr_t cp)
{
	return IsUnicodeCodePage(cp) || cp == CP_UTF8 || cp == encoding::codepage::oem() || cp == encoding::codepage::ansi();
}

bool IsUnicodeOrUtfCodePage(uintptr_t cp)
{
	return IsUnicodeCodePage(cp) || cp == CP_UTF8 || cp == CP_UTF7;
}

// See https://msdn.microsoft.com/en-us/library/windows/desktop/dd319072.aspx
bool IsNoFlagsCodepage(uintptr_t cp)
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

string ShortReadableCodepageName(uintptr_t cp)
{
	switch (cp)
	{
	case CP_UTF7:        return L"UTF-7"s;
	case CP_UTF8:        return L"UTF-8"s;
	case CP_UNICODE:     return L"U16LE"s;
	case CP_REVERSEBOM:  return L"U16BE"s;
	default: return
		cp == encoding::codepage::ansi()? L"ANSI"s :
		cp == encoding::codepage::oem()?  L"OEM"s :
		str(cp);
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

// PureAscii makes sense only if the function returned true
bool encoding::is_valid_utf8(std::string_view const Str, bool const PartialContent, bool& PureAscii)
{
	// The number of consecutive 1 bits in 000-111
	static constexpr char LookupTable[]
	{
		0, // 000
		0, // 001
		0, // 010
		0, // 011
		1, // 100
		1, // 101
		2, // 110
		3, // 111
	};

	bool Ascii = true;
	size_t ContinuationBytes = 0;
	const unsigned char Min = 0b10000000, Max = 0b10111111;
	auto NextMin = Min, NextMax = Max;

	for (const unsigned char c: Str)
	{
		if (ContinuationBytes)
		{
			if (!::utf8::is_continuation_byte(c))
				return false;

			if (c < NextMin || c > NextMax)
				return false;

			NextMin = Min;
			NextMax = Max;

			--ContinuationBytes;
			continue;
		}

		if (::utf8::is_ascii_byte(c))
			continue;

		Ascii = false;

		const auto Bits = (c & 0b01110000) >> 4;

		ContinuationBytes = LookupTable[Bits];
		if (!ContinuationBytes)
			return false;

		if (c & bit(7 - 1 - ContinuationBytes))
			return false;

		NextMin = Min;
		NextMax = Max;

		switch (ContinuationBytes)
		{
		case 1:
			if (c < 0b11000010)
				return false;
			break;

		case 2:
			if (c == 0b11100000)
				NextMin = 0b10100000;
			break;

		case 3:
			if (c > 0b11110100)
				return false;
			if (c == 0b11110000)
				NextMin = 0b10010000;
			else if (c == 0b11110100)
				NextMax = 0b10001111;
			break;
		}
	}

	PureAscii = Ascii;
	return !ContinuationBytes || PartialContent;
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("encoding.utf8")
{
	static const struct
	{
		bool Utf8;
		bool Ascii;
		std::string_view Str;
	}
	Tests[]
	{
		{ true, false, R"(
ᚠᛇᚻ᛫ᛒᛦᚦ᛫ᚠᚱᚩᚠᚢᚱ᛫ᚠᛁᚱᚪ᛫ᚷᛖᚻᚹᛦᛚᚳᚢᛗ
ᛋᚳᛖᚪᛚ᛫ᚦᛖᚪᚻ᛫ᛗᚪᚾᚾᚪ᛫ᚷᛖᚻᚹᛦᛚᚳ᛫ᛗᛁᚳᛚᚢᚾ᛫ᚻᛦᛏ᛫ᛞᚫᛚᚪᚾ
ᚷᛁᚠ᛫ᚻᛖ᛫ᚹᛁᛚᛖ᛫ᚠᚩᚱ᛫ᛞᚱᛁᚻᛏᚾᛖ᛫ᛞᚩᛗᛖᛋ᛫ᚻᛚᛇᛏᚪᚾ᛬
)"sv },

		{ true, false, R"(
𠜎 𠜱 𠝹 𠱓 𠱸 𠲖 𠳏 𠳕 𠴕 𠵼 𠵿 𠸎
𠸏 𠹷 𠺝 𠺢 𠻗 𠻹 𠻺 𠼭 𠼮 𠽌 𠾴 𠾼
𠿪 𡁜 𡁯 𡁵 𡁶 𡁻 𡃁 𡃉 𡇙 𢃇 𢞵 𢫕
𢭃 𢯊 𢱑 𢱕 𢳂 𢴈 𢵌 𢵧 𢺳 𣲷 𤓓 𤶸
𤷪 𥄫 𦉘 𦟌 𦧲 𦧺 𧨾 𨅝 𨈇 𨋢 𨳊 𨳍
)"sv },

		{ true, true, R"(
Lorem ipsum dolor sit amet,
consectetur adipiscing elit,
sed do eiusmod tempor incididunt
ut labore et dolore magna aliqua.
)"sv },
		{ true,  false, "φ"sv },
		{ false, false, "\x80"sv },
		{ false, false, "\xFF"sv },
		{ false, false, "\xC0"sv },
		{ false, false, "\xC1"sv },
		{ false, false, "\xC2\x20"sv },
		{ false, false, "\xC2\xC0"sv },
		{ false, false, "\xE0\xC0\xC0"sv },
		{ false, false, "\xEB\x20\xA8"sv },
		{ false, false, "\xEB\xA0\x28"sv },
		{ false, false, "\xF0\xC0\xC0\xC0"sv },
		{ false, false, "\xF4\xBF\xBF\xBF"sv },
		{ false, false, "\xF0\xA0\xA0\x20"sv },
	};

	for (const auto& i: Tests)
	{
		bool PureAscii = false;
		REQUIRE(i.Utf8 == encoding::is_valid_utf8(i.Str, false, PureAscii));
		REQUIRE(i.Ascii == PureAscii);

		const auto Str = encoding::utf8::get_chars(i.Str);

		if (utf8::support_embedded_raw_bytes)
		{
			// Lossless
			const auto Bytes = encoding::utf8::get_bytes(Str);
			REQUIRE(i.Str == Bytes);
		}
		else
		{
			// Lossy
			if (!i.Utf8)
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

	const irange Chars(std::numeric_limits<wchar_t>::max() + 1);

	const auto AllValid = std::all_of(ALL_CONST_RANGE(Chars), [&](wchar_t const Char)
	{
		const auto Result = round_trip(Char);

		if constexpr (utf8::support_unpaired_surrogates)
		{
			return Result == Char;
		}
		else
		{
			const auto
				IsSurrogate = in_closed_range(utf8::surrogate_first, Char, utf8::surrogate_last),
				IsInvalid = in_closed_range(utf8::invalid_first, Char, utf8::invalid_last);

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

	const irange Bytes(std::numeric_limits<char>::max() + 1);

	const auto AllValid = std::all_of(ALL_CONST_RANGE(Bytes), [&](char const Byte)
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
		size_t Position;
	}
	Tests[]
	{
		{ 932,   "0123\xE0"sv,     4, },
		{ 936,   "0\xDB"sv,        1, },
		{ 949,   "012345\x97"sv,   6, },
		{ 950,   "012\x81"sv,      3, },
		{ 1361,  "\x84"sv,         0, },
		{ 10001, "01\x85"sv,       2, },
		{ 10002, "0123\x81"sv,     4, },
		{ 20000, "012\xED"sv,      3, },
		{ 20001, "\xED"sv,         0, },
		{ 20003, "01\xFB"sv,       2, },
		{ 20004, "0123\xED"sv,     4, },
		{ 57011, "0123\xA0"sv,     4, },
	};

	for (const auto& i: Tests)
	{
		encoding::error_position ErrorPosition;
		REQUIRE(encoding::get_chars_count(i.Codepage, i.Bytes, &ErrorPosition));
		REQUIRE(ErrorPosition);
		REQUIRE(*ErrorPosition == i.Position);
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

		REQUIRE(Eol.cr<char>() == i.Cr);
		REQUIRE(Eol.lf<char>() == i.Lf);

		REQUIRE(Eol.cr<wchar_t>() == '\r');
		REQUIRE(Eol.lf<wchar_t>() == '\n');
	}
}
#endif
