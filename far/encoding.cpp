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

#include "headers.hpp"
#pragma hdrstop

#include "encoding.hpp"

class installed_codepages
{
public:
	installed_codepages();
	const cp_map& get() const { return m_InstalledCp; }

private:
	void insert(UINT Codepage, UINT MaxCharSize, const string& Name)
	{
		m_InstalledCp.emplace(Codepage, std::make_pair(MaxCharSize, Name));
	}
	friend class system_codepages_enumerator;

	cp_map m_InstalledCp;
};

class system_codepages_enumerator
{
public:
	static installed_codepages* context;

	static BOOL CALLBACK enum_cp(wchar_t *cpNum)
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
			wcscpy(cpix.CodePageName, cpNum);
		}
		if (cpix.MaxCharSize > 0)
		{
			string cp_data(cpix.CodePageName);
			// Windows: "XXXX (Name)", Wine: "Name"
			const auto OpenBracketPos = cp_data.find(L"(");
			if (OpenBracketPos != string::npos)
			{
				const auto CloseBracketPos = cp_data.rfind(L")");
				if (CloseBracketPos != string::npos && CloseBracketPos > OpenBracketPos)
				{
					cp_data = cp_data.substr(OpenBracketPos + 1, CloseBracketPos - OpenBracketPos - 1);
				}
			}
			context->insert(cp, cpix.MaxCharSize, cp_data);
		}

		return TRUE;

	}
};

installed_codepages* system_codepages_enumerator::context;

installed_codepages::installed_codepages()
{
	system_codepages_enumerator::context = this;
	EnumSystemCodePages(system_codepages_enumerator::enum_cp, CP_INSTALLED);
	system_codepages_enumerator::context = nullptr;
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

	const DWORD flags = (Codepage == CP_UTF8 || Codepage == 54936 || IsNoFlagsCodepage(Codepage))? 0 : WC_NO_BEST_FIT_CHARS;

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

size_t MultibyteCodepageDecoder::GetChar(const char* Buffer, size_t Size, wchar_t& Char, bool* End) const
{
	if (!Buffer || !Size)
	{
		if (End)
		{
			*End = true;
		}
		return 0;
	}

	char b1 = Buffer[0];
	char lmask = len_mask[b1];
	if (!lmask)
		return 0;

	if (lmask & 0x01)
	{
		Char = m1[b1];
		return 1;
	}

	if (Size < 2)
	{
		if (End)
		{
			*End = true;
		}
		return 0;
	}

	UINT16 b2 = b1 | (Buffer[1] << 8);
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

size_t encoding::get_bytes(uintptr_t const Codepage, const wchar_t* const Data, size_t const DataSize, char* const Buffer, size_t const BufferSize, bool* const UsedDefaultChar)
{
	switch(Codepage)
	{
	case CP_UTF8:
		return Utf8::get_bytes(Data, DataSize, Buffer, BufferSize);

	case CP_REVERSEBOM:
			swap_bytes(Data, Buffer, std::min(DataSize * sizeof(wchar_t), BufferSize));
			return DataSize * sizeof(wchar_t);

	default:
		{
			BOOL bUsedDefaultChar = FALSE;
			const auto Result = WideCharToMultiByte(Codepage, 0, Data, static_cast<int>(DataSize), Buffer, static_cast<int>(BufferSize), nullptr, UsedDefaultChar? &bUsedDefaultChar : nullptr);
			if (UsedDefaultChar)
				*UsedDefaultChar = bUsedDefaultChar != FALSE;
			return Result;
		}
	}
}

std::string encoding::get_bytes(uintptr_t const Codepage, const wchar_t* const Data, size_t const DataSize, bool* const UsedDefaultChar)
{
	if (const auto NewSize = encoding::get_bytes_count(Codepage, Data, DataSize))
	{
		std::string Buffer(NewSize, L'\0');
		encoding::get_bytes(Codepage, Data, DataSize, &Buffer[0], Buffer.size(), UsedDefaultChar);
		return Buffer;
	}
	return {};
}

namespace Utf7
{
	size_t get_chars(const char* Str, size_t StrSize, wchar_t* Buffer, size_t BufferSize, Utf::errors *Errors);
}

size_t encoding::get_chars(uintptr_t const Codepage, const char* const Data, size_t const DataSize, wchar_t* const Buffer, size_t const BufferSize)
{
	switch (Codepage)
	{
	case CP_UTF8:
		return Utf8::get_chars(Data, DataSize, Buffer, BufferSize, nullptr);

	case CP_UTF7:
		return Utf7::get_chars(Data, DataSize, Buffer, BufferSize, nullptr);

	case CP_REVERSEBOM:
		swap_bytes(Data, Buffer, std::min(DataSize, BufferSize * sizeof(wchar_t)));
		return DataSize / sizeof(wchar_t);

	default:
		return MultiByteToWideChar(Codepage, 0, Data, static_cast<int>(DataSize), Buffer, static_cast<int>(BufferSize));
	}
}

string encoding::get_chars(uintptr_t const Codepage, const char* const Data, size_t const DataSize)
{
	if (const auto NewSize = encoding::get_chars_count(Codepage, Data, DataSize))
	{
		string Buffer(NewSize, L'\0');
		encoding::get_chars(Codepage, Data, DataSize, &Buffer[0], Buffer.size());
		return Buffer;
	}
	return {};
}

//################################################################################################

size_t Utf::get_chars(uintptr_t const Codepage, const char* const Data, size_t const DataSize, wchar_t* const Buffer, size_t const BufferSize, errors* const Errors)
{
	switch (Codepage)
	{
	case CP_UTF7:
		return Utf7::get_chars(Data, DataSize, Buffer, BufferSize, Errors);
	case CP_UTF8:
		return Utf8::get_chars(Data, DataSize, Buffer, BufferSize, Errors);
	default:
		throw MAKE_FAR_EXCEPTION("not an utf codepage");
	}
}

//################################################################################################

//                                   2                         5         6
//	        0                         6                         2         2
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
static size_t Utf7_GetChar(const char* const Data, size_t const DataSize, wchar_t* const Buffer, bool& ConversionError, int& state)
{
	if (!DataSize)
		return 0;

	auto DataIterator = Data;

	size_t BytesConsumed = 1;
	int m[3];
	BYTE c = *DataIterator++;
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

		c = *DataIterator++;
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

	if ((c = *DataIterator++) >= 128)
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
		if ((c = *DataIterator++) >= 128)
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

	if (DataSize > BytesConsumed && *Data == (BYTE)'-')
	{
		u.s.base64 = false;
		++BytesConsumed;
	}

	state = u.state;
	return BytesConsumed;
}

static size_t BytesToUnicode(
	const char* const Str, size_t const StrSize,
	wchar_t* const Buffer, size_t const BufferSize,
	size_t(*GetChar)(const char*, size_t, wchar_t*, bool&, int&),
	Utf::errors* const Errors)
{
	if (!StrSize)
		return 0;

	if (Errors)
		*Errors = {};

	auto DataIterator = Str;
	const auto DataEnd = Str + StrSize;

	auto BufferIterator = Buffer;
	const auto BufferEnd = Buffer + BufferSize;

	int State = 0;
	size_t RequiredSize = 0;

	while (DataIterator != DataEnd)
	{
		wchar_t TmpBuffer[2]{};
		auto ConversionError = false;
		const auto BytesConsumed = GetChar(DataIterator, DataEnd - DataIterator, TmpBuffer, ConversionError, State);

		if (!BytesConsumed)
			break;

		if (ConversionError)
		{
			TmpBuffer[0] = Utf::REPLACE_CHAR;
			if (Errors && !Errors->Conversion.Error)
			{
				Errors->Conversion.Error = true;
				Errors->Conversion.Position = DataIterator - Str;
			}
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

		DataIterator += BytesConsumed;
	}

	return RequiredSize;
}

size_t Utf7::get_chars(const char* const Data, size_t const DataSize, wchar_t* const Buffer, size_t const BufferSize, Utf::errors* const Errors)
{
	return BytesToUnicode(Data, DataSize, Buffer, BufferSize, Utf7_GetChar, Errors);
}

size_t Utf8::get_char(const char*& DataIterator, const char* const DataEnd, wchar_t& First, wchar_t& Second)
{
	size_t NumberOfChars = 1;

	const auto InvalidChar = [](unsigned char c) { return 0xDC00 + c; };

	const auto c1 = *DataIterator++;

	if (c1 < 0x80)
	{
		// simple ASCII
		First = c1;
	}
	else if (c1 < 0xC2 || c1 >= 0xF5)
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
		if (DataIterator == DataEnd)
		{
			return Unfinished();
		}

		const auto c2 = *DataIterator;

		if ((c2 & 0xC0) != 0x80 ||        // illegal 2-nd byte
			(c1 == 0xE0 && c2 <= 0x9F) || // illegal 3-byte start (overlaps with 2-byte)
			(c1 == 0xF0 && c2 <= 0x8F) || // illegal 4-byte start (overlaps with 3-byte)
			(c1 == 0xF4 && c2 >= 0x90))   // illegal 4-byte (out of unicode range)
		{
			First = InvalidChar(c1);
		}
		else if (c1 < 0xE0)
		{
			// legal 2-byte
			First = ((c1 & 0x1F) << 6) | (c2 & 0x3F);
			++DataIterator;
		}
		else
		{
			// 3 or 4-byte
			if (DataIterator + 1 == DataEnd)
			{
				return Unfinished();
			}

			const auto c3 = *(DataIterator + 1);
			if ((c3 & 0xC0) != 0x80)
			{
				// illegal 3-rd byte
				First = InvalidChar(c1);
			}
			else if (c1 < 0xF0)
			{
				// legal 3-byte
				First = ((c1 & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
				if (First >= 0xD800 && First <= 0xDFFF)
				{
					// invalid: surrogate area code
					First = InvalidChar(c1);
				}
				else
				{
					DataIterator += 2;
				}
			}
			else
			{
				// 4-byte
				if (DataIterator + 2 == DataEnd)
				{
					return Unfinished();
				}

				const auto c4 = *(DataIterator + 2);
				if ((c4 & 0xC0) != 0x80)
				{
					// illegal 4-th byte
					First = InvalidChar(c1);
				}
				else
				{
					// legal 4-byte (produces 2 WCHARs)
					const auto FullChar = ((c1 & 0x07) << 18 | (c2 & 0x3F) << 12 | (c3 & 0x3F) << 6 | (c4 & 0x3F)) - 0x10000;
					First = 0xD800 + (FullChar >> 10);
					Second = 0xDC00 + (FullChar & 0x3FF);
					NumberOfChars = 2;
					DataIterator += 3;
				}
			}
		}
	}

	return NumberOfChars;
}

size_t Utf8::get_chars(const char* const Data, size_t const DataSize, wchar_t* const Buffer, size_t const BufferSize, int& Tail)
{
	auto DataIterator = Data;
	const auto DataEnd = Data + DataSize;

	auto BufferIterator = Buffer;
	const auto BufferEnd = Buffer + BufferSize;

	const auto StoreChar = [&](wchar_t Char)
	{
		if (BufferIterator != BufferEnd)
		{
			*BufferIterator++ = Char;
			return true;
		}
		return false;
	};

	while (DataIterator != DataEnd)
	{
		wchar_t First, Second;
		const auto NumberOfChars = get_char(DataIterator, DataEnd, First, Second);

		if (!StoreChar(NumberOfChars == 1 || BufferIterator + 1 != BufferEnd? First : Utf::REPLACE_CHAR))
			break;

		if (NumberOfChars == 2)
		{
			if (!StoreChar(Second))
				break;
		}
	}

	Tail = DataEnd - DataIterator;
	return BufferIterator - Buffer;
}

size_t Utf8::get_chars(const char* const Data, size_t const DataSize, wchar_t* const Buffer, size_t const BufferSize, Utf::errors* const Errors)
{
	return BytesToUnicode(Data, DataSize, Buffer, BufferSize, [](const char* Data, size_t DataSize, wchar_t* Buffer, bool&, int&)
	{
		auto Iterator = Data;
		Utf8::get_char(Iterator, Data + DataSize, Buffer[0], Buffer[1]);
		return static_cast<size_t>(Iterator - Data);
	}, Errors);
}

size_t Utf8::get_bytes(const wchar_t* const Data, size_t const DataSize, char* const Buffer, size_t const BufferSize)
{
	const auto End = Data + DataSize;
	auto StrPtr = Data;
	auto BufferPtr = Buffer;
	size_t RequiredCapacity = 0;
	auto AvailableCapacity = BufferSize;

	while (StrPtr != End)
	{
		unsigned int Char = *StrPtr++;

		size_t BytesNumber;

		if (Char < 0x80)
		{
			BytesNumber = 1;
		}
		else if (Char < 0x800)
		{
			BytesNumber = 2;
		}
		else if (Char - 0xD800 > 0xDFFF - 0xD800) // not surrogates
		{
			BytesNumber = 3;
		}
		else if (Char - 0xDC80 <= 0xDCFF - 0xDC80) // embedded raw byte
		{
			BytesNumber = 1;
			Char &= 0xFF;
		}
		else if (Char - 0xD800 <= 0xDBFF - 0xD800 && StrPtr != End && *StrPtr - 0xDC00u <= 0xDFFF - 0xDC00) // valid surrogate pair
		{
			BytesNumber = 4;
			Char = 0x3C10000 + ((Char - 0xD800) << 10) + static_cast<unsigned int>(*StrPtr++) - 0xDC00;
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
		case 1:
			*BufferPtr++ = Char;
			break;

		case 2:
			*BufferPtr++ = 0xC0 + (Char >> 6);
			*BufferPtr++ = 0x80 + (Char & 0x3F);
			break;

		case 3:
			*BufferPtr++ = 0xE0 + (Char >> 12);
			*BufferPtr++ = 0x80 + (Char >> 6 & 0x3F);
			*BufferPtr++ = 0x80 + (Char & 0x3F);
			break;

		case 4:
			*BufferPtr++ = Char >> 18;
			*BufferPtr++ = 0x80 + (Char >> 12 & 0x3F);
			*BufferPtr++ = 0x80 + (Char >> 6 & 0x3F);
			*BufferPtr++ = 0x80 + (Char & 0x3F);
			break;
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
