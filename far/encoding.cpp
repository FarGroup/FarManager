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

#include "encoding.hpp"

#include "strmix.hpp"
#include "exception.hpp"
#include "plugin.hpp"

#include "common/io.hpp"

class installed_codepages
{
public:
	installed_codepages();
	const auto& get() const { return m_InstalledCp; }

private:
	void insert(UINT Codepage, UINT MaxCharSize, const string& Name)
	{
		m_InstalledCp.emplace(Codepage, std::make_pair(MaxCharSize, Name));
	}
	friend class system_codepages_enumerator;

	cp_map m_InstalledCp;
	std::exception_ptr m_ExceptionPtr;
};

class system_codepages_enumerator
{
public:
	static installed_codepages* context;

	static BOOL CALLBACK enum_cp(wchar_t *cpNum)
	{
		try
		{
			const auto cp = static_cast<UINT>(std::wcstoul(cpNum, nullptr, 10));
			if (cp == CP_UTF8)
				return TRUE; // skip standard unicode

			CPINFOEX cpix;
			if (!GetCPInfoEx(cp, 0, &cpix))
			{
				CPINFO cpi;
				if (!GetCPInfo(cp, &cpi))
					return TRUE;

				cpix.MaxCharSize = cpi.MaxCharSize;
				xwcsncpy(cpix.CodePageName, cpNum, std::size(cpix.CodePageName));
			}
			if (cpix.MaxCharSize > 0)
			{
				string cp_data(cpix.CodePageName);
				// Windows: "XXXX (Name)", Wine: "Name"
				const auto OpenBracketPos = cp_data.find(L'(');
				if (OpenBracketPos != string::npos)
				{
					const auto CloseBracketPos = cp_data.rfind(L')');
					if (CloseBracketPos != string::npos && CloseBracketPos > OpenBracketPos)
					{
						cp_data.resize(CloseBracketPos);
						cp_data.erase(0, OpenBracketPos + 1);
					}
				}
				context->insert(cp, cpix.MaxCharSize, cp_data);
			}

			return TRUE;
		}
		CATCH_AND_SAVE_EXCEPTION_TO(context->m_ExceptionPtr);

		return FALSE;
	}
};

installed_codepages* system_codepages_enumerator::context;

installed_codepages::installed_codepages()
{
	system_codepages_enumerator::context = this;
	EnumSystemCodePages(system_codepages_enumerator::enum_cp, CP_INSTALLED);
	system_codepages_enumerator::context = nullptr;
	RethrowIfNeeded(m_ExceptionPtr);
}

const cp_map& InstalledCodepages()
{
	static const installed_codepages s_Icp;
	return s_Icp.get();
}

cp_map::value_type::second_type GetCodePageInfo(UINT cp)
{
	// Standard unicode CPs (1200, 1201, 65001) are NOT in the list.
	const auto& InstalledCp = InstalledCodepages();
	const auto found = InstalledCp.find(cp);
	if (InstalledCp.end() == found)
		return {};

	return found->second;
}

static bool IsValid(UINT cp)
{
	if (cp==CP_ACP || cp==CP_OEMCP || cp==CP_MACCP || cp==CP_THREAD_ACP || cp==CP_SYMBOL)
		return false;

	if (cp==CP_UTF8 || cp==CP_UNICODE || cp==CP_REVERSEBOM)
		return false;

	return GetCodePageInfo(cp).first == 2;
}

static auto GetNoBestFitCharsFlag(uintptr_t Codepage)
{
	// See https://msdn.microsoft.com/en-us/library/windows/desktop/dd374130.aspx
	return Codepage == CP_UTF8 || Codepage == 54936 || IsNoFlagsCodepage(Codepage)? 0 : WC_NO_BEST_FIT_CHARS;
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

	BOOL DefUsed, *pDefUsed = (Codepage == CP_UTF8 || Codepage == CP_UTF7) ? nullptr : &DefUsed;

	const DWORD flags = GetNoBestFitCharsFlag(Codepage);

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
		DefUsed = FALSE;
		const auto Char = static_cast<wchar_t>(i);
		size_t CharSize = WideCharToMultiByte(Codepage, flags, &Char, 1, u.Buffer, static_cast<int>(std::size(u.Buffer)), nullptr, pDefUsed);
		if (!CharSize || DefUsed)
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

	assert(CharsProcessed >= 256);
	if (CharsProcessed < 256)
		return false;

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

	char b1 = Str[0];
	char lmask = len_mask[b1];
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

	UINT16 b2 = b1 | (Str[1] << 8);
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

static size_t get_bytes_impl(uintptr_t const Codepage, string_view const Str, char* const Buffer, size_t const BufferSize, bool* const UsedDefaultChar)
{
	if (Str.empty())
		return 0;

	switch(Codepage)
	{
	case CP_UTF8:
		return Utf8::get_bytes(Str, Buffer, BufferSize);

	case CP_UNICODE:
		if (BufferSize) // paranoid gcc null checks are paranoid
			memcpy(Buffer, Str.data(), std::min(Str.size() * sizeof(wchar_t), BufferSize));
		return Str.size() * sizeof(wchar_t);

	case CP_REVERSEBOM:
		swap_bytes(Str.data(), Buffer, std::min(Str.size() * sizeof(wchar_t), BufferSize));
		return Str.size() * sizeof(wchar_t);

	default:
		{
			BOOL bUsedDefaultChar = FALSE;
			auto Result = WideCharToMultiByte(Codepage, GetNoBestFitCharsFlag(Codepage), Str.data(), static_cast<int>(Str.size()), Buffer, static_cast<int>(BufferSize), nullptr, UsedDefaultChar? &bUsedDefaultChar : nullptr);
			if (!Result && BufferSize < Str.size() && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				// https://msdn.microsoft.com/en-us/library/windows/desktop/dd374130.aspx
				// If BufferSize is less than DataSize, this function writes the number of characters specified by BufferSize to the buffer indicated by Buffer.

				// If the function succeeds and BufferSize is 0, the return value is the required size, in bytes, for the buffer indicated by Buffer.
				Result = WideCharToMultiByte(Codepage, GetNoBestFitCharsFlag(Codepage), Str.data(), static_cast<int>(Str.size()), nullptr, 0, nullptr, UsedDefaultChar ? &bUsedDefaultChar : nullptr);
			}

			if (UsedDefaultChar)
				*UsedDefaultChar = bUsedDefaultChar != FALSE;

			return Result;
		}
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

size_t encoding::get_bytes(uintptr_t const Codepage, string_view const Str, char* const Buffer, size_t const BufferSize, bool* const UsedDefaultChar)
{
	const auto Result = get_bytes_impl(Codepage, Str, Buffer, BufferSize, UsedDefaultChar);
	if (Result < BufferSize)
	{
		Buffer[Result] = '\0';
	}
	return Result;
}

std::string encoding::get_bytes(uintptr_t const Codepage, string_view const Str, bool* const UsedDefaultChar)
{
	if (Str.empty())
		return {};

	// DataSize is a good estimation for the bytes count.
	// With this approach we can fill the buffer with only one attempt in many cases.
	std::string Buffer(Str.size(), '\0');

	for (auto Overflow = true; Overflow;)
	{
		const auto Size = get_bytes(Codepage, Str, Buffer.data(), Buffer.size(), UsedDefaultChar);
		Overflow = Size > Buffer.size();
		Buffer.resize(Size);
	}

	return Buffer;
}

namespace Utf7
{
	size_t get_chars(std::string_view Str, wchar_t* Buffer, size_t BufferSize, Utf::errors *Errors);
}

static size_t get_chars_impl(uintptr_t const Codepage, std::string_view Str, wchar_t* const Buffer, size_t const BufferSize)
{
	switch (Codepage)
	{
	case CP_UTF8:
		return Utf8::get_chars(Str, Buffer, BufferSize, nullptr);

	case CP_UTF7:
		return Utf7::get_chars(Str, Buffer, BufferSize, nullptr);

	case CP_UNICODE:
		if (BufferSize) // paranoid gcc null checks are paranoid
			memcpy(Buffer, Str.data(), std::min(Str.size(), BufferSize * sizeof(wchar_t)));
		return Str.size() / sizeof(wchar_t);

	case CP_REVERSEBOM:
		swap_bytes(Str.data(), Buffer, std::min(Str.size(), BufferSize * sizeof(wchar_t)));
		return Str.size() / sizeof(wchar_t);

	default:
		return MultiByteToWideChar(Codepage, 0, Str.data(), static_cast<int>(Str.size()), Buffer, static_cast<int>(BufferSize));
	}
}

size_t encoding::get_chars(uintptr_t const Codepage, std::string_view const Str, wchar_t* const Buffer, size_t const BufferSize)
{
	const auto Result = get_chars_impl(Codepage, Str, Buffer, BufferSize);
	if (Result < BufferSize)
	{
		Buffer[Result] = L'\0';
	}
	return Result;
}

string encoding::get_chars(uintptr_t const Codepage, std::string_view const Str)
{
	if (Str.empty())
		return {};

	const auto& EstimatedCharsCount = [&]
	{
		switch (Codepage)
		{
		case CP_UNICODE:
		case CP_REVERSEBOM:
			return Str.size() / sizeof(wchar_t);

		case CP_UTF7:
		case CP_UTF8:
			// Even though DataSize is always >= BufferSize for these guys, we can't use DataSize for estimation - it can be three times larger than necessary.
			return get_chars_count(Codepage, Str);

		default:
			return Str.size();
		}
	};

	// With this approach we can fill the buffer with only one attempt in many cases.
	string Buffer(EstimatedCharsCount(), L'\0');
	for (auto Overflow = true; Overflow;)
	{
		const auto Size = get_chars(Codepage, Str, Buffer.data(), Buffer.size());
		Overflow = Size > Buffer.size();
		Buffer.resize(Size);
	}

	return Buffer;
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

size_t Utf::get_chars(uintptr_t const Codepage, std::string_view const Str, wchar_t* const Buffer, size_t const BufferSize, errors* const Errors)
{
	switch (Codepage)
	{
	case CP_UTF7:
		return Utf7::get_chars(Str, Buffer, BufferSize, Errors);
	case CP_UTF8:
		return Utf8::get_chars(Str, Buffer, BufferSize, Errors);
	default:
		throw MAKE_FAR_EXCEPTION(L"Not a utf codepage"sv);
	}
}

//################################################################################################

//                                   2                         5         6
//         0                         6                         2         2
// base64: ABCDEFGHIJKLMNOPQRSTUVWXYZabcdrfghijklmnopqrstuvwxyz0123456789+/

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

constexpr short D(int n) { return dir + b64 + n; }

static const short m7[128] =
//  x00   x01   x02   x03   x04   x05   x06   x07   x08   x09   x0a   x0b   x0c   x0d   x0e   x0f
{   ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  DIR,  DIR,  ILL,  ILL,  DIR,  ILL,  ILL

//  x10   x11   x12   x13   x14   x15   x16   x17   x18   x19   x1a   x1b   x1c   x1d   x1e   x1f
,   ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL

// =x20 !=x21 "=x22 #=x23 $=x24 %=x25 &=x26 '=x27 (=x28 )=x29 *=x2a +=x2b ,x=2c -=x2d .=x2e /=x2f
,   DIR,  OPT,  OPT,  OPT,  OPT,  OPT,  OPT,  DIR,  DIR,  DIR,  OPT,  PLS,  DIR,  MNS, DIR, D(63)

//0=x30 1=x31 2=x32 3=x33 4=x34 5=x35 6=x36 7=x37 8=x38 9=x39 :=x3a ;=x3b <=x3c ==x3d >=x3e ?=x3f
, D(52),D(53),D(54),D(55),D(56),D(57),D(58),D(59),D(60),D(61),  DIR,  OPT,  OPT,  OPT,  OPT,  DIR

//@=x40 A=x41 B=x42 C=x43 D=x44 E=x45 F=x46 G=x47 H=x48 I=x49 J=x4a K=x4b L=x4c M=x4d N=x4e O=x4f
,   OPT, D(0), D(1), D(2), D(3), D(4), D(5), D(6), D(7), D(8), D(9),D(10),D(11),D(12),D(13),D(14)

//P=x50 Q=x51 R=x52 S=x53 T=x54 U=x55 V=x56 W=x57 X=x58 Y=x59 Z=x5a [=x5b \=x5c ]=x5d ^=x5e _=x5f
, D(15),D(16),D(17),D(18),D(19),D(20),D(21),D(22),D(23),D(24),D(25),  OPT,  ILL,  OPT,  OPT,  OPT

//`=x60 a=x61 b=x62 c=x63 d=x64 e=x65 f=x66 g=x67 h=x68 i=x69 j=x6a k=x6b l=x6c m=x6d n=x6e o=x6f
,   OPT,D(26),D(27),D(28),D(29),D(30),D(31),D(32),D(33),D(34),D(35),D(36),D(37),D(38),D(39),D(40)

//p=x70 q=x71 r=x72 s=x73 t=x74 u=x75 v=x76 w=x77 x=x78 y=x79 z=x7a {=x7b |=x7c }=x7d ~=x7e   x7f
, D(41),D(42),D(43),D(44),D(45),D(46),D(47),D(48),D(49),D(50),D(51),  OPT,  OPT,  OPT,  ILL,  ILL
};

// BUGBUG non-BMP range is not supported
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

	if (!u.s.base64)
	{
		if (c != (BYTE)'+')
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

		if (c == (BYTE)'-')
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

	int a = 2 - u.s.carry_count / 4;
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
		*Buffer = static_cast<wchar_t>((u.s.carry_bits << 12) | ((BYTE)m[0] << 6) | (BYTE)m[1]);
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
		unsigned m18 = ((BYTE)m[0] << 12) | ((BYTE)m[1] << 6) | ((BYTE)m[2]);

		if (u.s.carry_count == 0)
		{
			*Buffer = static_cast<wchar_t>(m18 >> 2);
			u.s.carry_bits = (BYTE)(m18 & 0x03);
			u.s.carry_count = 2;
		}
		else
		{
			*Buffer = static_cast<wchar_t>((u.s.carry_bits << 14) | (m18 >> 4));
			u.s.carry_bits = (BYTE)(m18 & 0x07);
			u.s.carry_count = 4;
		}
	}
	++BytesConsumed;

	if (DataSize > BytesConsumed && *Iterator == '-')
	{
		u.s.base64 = false;
		++BytesConsumed;
	}

	state = u.state;
	return BytesConsumed;
}

static size_t BytesToUnicode(
	std::string_view const Str,
	wchar_t* const Buffer, size_t const BufferSize,
	size_t(*GetChar)(std::string_view::const_iterator, std::string_view::const_iterator, wchar_t*, bool&, int&),
	Utf::errors* const Errors)
{
	if (Str.empty())
		return 0;

	if (Errors)
		*Errors = {};

	auto StrIterator = Str.begin();
	const auto StrEnd = Str.end();

	auto BufferIterator = Buffer;
	const auto BufferEnd = Buffer + BufferSize;

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
			TmpBuffer[0] = Utf::REPLACE_CHAR;
			if (Errors && !Errors->Conversion.Error)
			{
				Errors->Conversion.Error = true;
				Errors->Conversion.Position = StrIterator - Str.begin();
			}
		}

		const auto& StoreChar = [&](wchar_t Char)
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

size_t Utf7::get_chars(std::string_view const Str, wchar_t* const Buffer, size_t const BufferSize, Utf::errors* const Errors)
{
	return BytesToUnicode(Str, Buffer, BufferSize, Utf7_GetChar, Errors);
}

namespace utf8
{
	const auto surrogate_low_first  = 0b11011100'00000000u;
	const auto surrogate_low_last   = 0b11011111'11111111u;
	const auto surrogate_high_first = 0b11011000'00000000u;
	const auto surrogate_high_last  = 0b11011011'11111111u;

	const auto invalid_first        = 0b11011100'10000000u;
	const auto invalid_last         = 0b11011100'11111111u;

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

		static unsigned int extract_continuation_bits() { return 0; }

		template<typename... bytes>
		static constexpr unsigned int extract_continuation_bits(unsigned char const Byte, bytes... Bytes)
		{
			return ((Byte & 0b00111111) << (6 * sizeof...(Bytes))) | extract_continuation_bits(Bytes...);
		}

		template<size_t total>
		static constexpr unsigned char make_leading_byte(unsigned int const Char)
		{
			return ((0b11111111 << (8 - total)) & 0b11111111) | (Char >> (6 * (total - 1)));
		}

		template<size_t total, size_t number>
		static constexpr unsigned char make_continuation_byte(unsigned int const Char)
		{
			static_assert(number <= total);
			return 0b10000000 | ((Char >> (6 * (total - number))) & 0b00111111);
		}

		template<size_t total, size_t number, typename iterator, REQUIRES((number > total))>
		static constexpr void write_continuation_bytes(unsigned int, iterator&) {}

		template<size_t total, size_t number, typename iterator, REQUIRES(number <= total)>
		static void write_continuation_bytes(unsigned int const Char, iterator& Iterator)
		{
			*Iterator++ = make_continuation_byte<total, number>(Char);
			write_continuation_bytes<total, number + 1>(Char, Iterator);
		}
	}

	template<typename... bytes>
	static constexpr unsigned int extract(unsigned char const Byte, bytes... Bytes)
	{
		static_assert(sizeof...(Bytes) < 4);
		return detail::extract_leading_bits<sizeof...(Bytes)>(Byte) | detail::extract_continuation_bits(Bytes...);
	}

	template<size_t total, typename iterator, REQUIRES(total == 1)>
	static void write(unsigned int const Char, iterator& Iterator)
	{
		*Iterator++ = Char;
	}

	template<size_t total, typename iterator, REQUIRES((total > 1))>
	static void write(unsigned int const Char, iterator& Iterator)
	{
		*Iterator++ = detail::make_leading_byte<total>(Char);
		detail::write_continuation_bytes<total - 1, 1>(Char, Iterator);
	}
}

size_t Utf8::get_char(std::string_view::const_iterator& StrIterator, std::string_view::const_iterator const StrEnd, wchar_t& First, wchar_t& Second)
{
	size_t NumberOfChars = 1;

	const auto& InvalidChar = [](unsigned char c) { return utf8::surrogate_low_first | c; };

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
		const auto& Unfinished = [&]
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

		if (c2 > 0b10111111 ||                       // illegal 2-nd byte
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

				if (utf8::surrogate_high_first <= First && First <= utf8::surrogate_low_last)
				{
					// invalid: surrogate area code
					First = InvalidChar(c1);
				}
				else
				{
					StrIterator += 2;
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

size_t Utf8::get_chars(std::string_view const Str, wchar_t* const Buffer, size_t const BufferSize, int& Tail)
{
	auto StrIterator = Str.begin();
	const auto StrEnd = Str.end();

	auto BufferIterator = Buffer;
	const auto BufferEnd = Buffer + BufferSize;

	const auto& StoreChar = [&](wchar_t Char)
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

		if (!StoreChar(NumberOfChars == 1 || BufferIterator + 1 != BufferEnd? First : Utf::REPLACE_CHAR))
			break;

		if (NumberOfChars == 2)
		{
			if (!StoreChar(Second))
				break;
		}
	}

	Tail = StrEnd - StrIterator;
	return BufferIterator - Buffer;
}

size_t Utf8::get_chars(std::string_view const Str, wchar_t* const Buffer, size_t const BufferSize, Utf::errors* const Errors)
{
	return BytesToUnicode(Str, Buffer, BufferSize, [](std::string_view::const_iterator const Iterator, std::string_view::const_iterator const End, wchar_t* CharBuffer, bool&, int&)
	{
		auto NextIterator = Iterator;
		get_char(NextIterator, End, CharBuffer[0], CharBuffer[1]);
		return static_cast<size_t>(NextIterator - Iterator);
	}, Errors);
}

size_t Utf8::get_bytes(string_view const Str, char* const Buffer, size_t const BufferSize)
{
	auto StrIterator = Str.begin();
	const auto StrEnd = Str.end();

	auto BufferIterator = Buffer;
	size_t RequiredCapacity = 0;
	auto AvailableCapacity = BufferSize;

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
		else if (!InRange(utf8::surrogate_high_first, Char, utf8::surrogate_low_last))
		{
			// not surrogates
			BytesNumber = 3;
		}
		else if (InRange(utf8::invalid_first, Char, utf8::invalid_last))
		{
			// embedded raw byte
			BytesNumber = 1;
			Char &= 0b11111111;
		}
		else if (StrIterator != StrEnd &&
			InRange(utf8::surrogate_high_first, Char, utf8::surrogate_high_last) &&
			InRange(utf8::surrogate_low_first, *StrIterator, utf8::surrogate_low_last))
		{
			// valid surrogate pair
			BytesNumber = 4;
			Char = 0b1'00000000'00000000u + ((Char - utf8::surrogate_high_first) << 10) + (*StrIterator++ - utf8::surrogate_low_first);
		}
		else
		{
			BytesNumber = 1;
			Char = Utf::REPLACE_CHAR;
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
	_swab(reinterpret_cast<char*>(const_cast<void*>(Src)), reinterpret_cast<char*>(Dst), static_cast<int>(SizeInBytes));
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
		(cp >= 50220 && cp <= 50222) ||
		cp == 50225 ||
		cp == 50227 ||
		cp == 50229 ||
		(cp >= 57002 && cp <= 57011) ||
		cp == CP_UTF7 ||
		cp == CP_SYMBOL;
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
	static const char LookupTable[] =
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

		const auto Bits = (c & 0b01111000) >> 3;
		if (Bits & 1)
			return false;

		ContinuationBytes = LookupTable[Bits >> 1];
		if (!ContinuationBytes)
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

#include "common/test.hpp"

#ifdef _DEBUG
static void TestEncoding()
{
	static const struct
	{
		bool Utf8;
		bool Ascii;
		std::string_view Str;
	}
	Tests[]
	{
		{ true, false, u8R"(
ᚠᛇᚻ᛫ᛒᛦᚦ᛫ᚠᚱᚩᚠᚢᚱ᛫ᚠᛁᚱᚪ᛫ᚷᛖᚻᚹᛦᛚᚳᚢᛗ
ᛋᚳᛖᚪᛚ᛫ᚦᛖᚪᚻ᛫ᛗᚪᚾᚾᚪ᛫ᚷᛖᚻᚹᛦᛚᚳ᛫ᛗᛁᚳᛚᚢᚾ᛫ᚻᛦᛏ᛫ᛞᚫᛚᚪᚾ
ᚷᛁᚠ᛫ᚻᛖ᛫ᚹᛁᛚᛖ᛫ᚠᚩᚱ᛫ᛞᚱᛁᚻᛏᚾᛖ᛫ᛞᚩᛗᛖᛋ᛫ᚻᛚᛇᛏᚪᚾ᛬
)"sv },

		{ true, false, u8R"(
𠜎 𠜱 𠝹 𠱓 𠱸 𠲖 𠳏 𠳕 𠴕 𠵼 𠵿 𠸎
𠸏 𠹷 𠺝 𠺢 𠻗 𠻹 𠻺 𠼭 𠼮 𠽌 𠾴 𠾼
𠿪 𡁜 𡁯 𡁵 𡁶 𡁻 𡃁 𡃉 𡇙 𢃇 𢞵 𢫕
𢭃 𢯊 𢱑 𢱕 𢳂 𢴈 𢵌 𢵧 𢺳 𣲷 𤓓 𤶸
𤷪 𥄫 𦉘 𦟌 𦧲 𦧺 𧨾 𨅝 𨈇 𨋢 𨳊 𨳍
)"sv },

		{ true, true, u8R"(
Lorem ipsum dolor sit amet,
consectetur adipiscing elit,
sed do eiusmod tempor incididunt
ut labore et dolore magna aliqua.
)"sv },

		{ false, false, "\x80"sv },
		{ false, false, "\xFF"sv },
		{ false, false, "\xC0"sv },
		{ false, false, "\xC1"sv },
		{ false, false, "\xC2\x20"sv },
		{ false, false, "\xC2\xC0"sv },
		{ false, false, "\xE0\xC0\xC0"sv },
		{ false, false, "\xF0\xC0\xC0\xC0"sv },
		{ false, false, "\xF4\xBF\xBF\xBF"sv },
	};

	for (const auto i: Tests)
	{
		bool PureAscii = false;
		EXPECT_EQ(i.Utf8, encoding::is_valid_utf8(i.Str, false, PureAscii));
		EXPECT_EQ(i.Ascii, PureAscii);
	}
}
#endif

SELF_TEST(TestEncoding)
