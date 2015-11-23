/*
codepage.cpp

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

#include "codepage.hpp"

class installed_codepages
{
public:
	installed_codepages();
	const cp_map& get() const { return m_InstalledCp; }

private:
	void insert(UINT Codepage, UINT MaxCharSize, const string& Name)
	{
		m_InstalledCp.insert(std::make_pair(Codepage, std::make_pair(MaxCharSize, Name)));
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
		return cp_map::value_type::second_type();

	return found->second;
}

inline static bool IsValid(UINT cp)
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
		size_t CharSize = WideCharToMultiByte(Codepage, flags, &Char, 1, u.Buffer, ARRAYSIZE(u.Buffer), nullptr, pDefUsed);
		if (!CharSize || DefUsed)
			continue;

		len_mask[u.b1] |= BIT(CharSize - 1);
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

size_t unicode::to(uintptr_t cp, const wchar_t *src, size_t srclen, char *dst, size_t dstlen, bool* UsedDefaultChar)
{
	if (cp == CP_UTF8)
	{
		const size_t len=Utf8::ToMultiByte(src, srclen, dst);
		if (dst) assert(len<=dstlen);
		return len;
	}
	else if (cp == CP_REVERSEBOM)
	{
		if (dst)
			swap_bytes(src, dst, std::min(srclen * sizeof(wchar_t), dstlen));
		return srclen * sizeof(wchar_t);
	}
	else
	{
		BOOL bUsedDefaultChar = FALSE;
		const auto Result = WideCharToMultiByte(cp, 0, src, static_cast<int>(srclen), dst, static_cast<int>(dstlen), nullptr, UsedDefaultChar? &bUsedDefaultChar : nullptr);
		if (UsedDefaultChar)
			*UsedDefaultChar = bUsedDefaultChar != FALSE;
		return Result;
	}
}

std::string unicode::to(uintptr_t Codepage, const wchar_t *Data, size_t Size, bool* UsedDefaultChar)
{
	if (const auto NewSize = unicode::to(Codepage, Data, Size, nullptr, 0))
	{
		std::string Buffer(NewSize, 0);
		unicode::to(Codepage, Data, Size, &Buffer[0], Buffer.size(), UsedDefaultChar);
		return Buffer;
	}
	return std::string();
}

size_t unicode::from(uintptr_t Codepage, const char* Data, size_t Size, wchar_t* Buffer, size_t BufferSize)
{
	if (Codepage == CP_UTF8)
	{
		return Utf8::ToWideChar(Data, Size, Buffer, BufferSize, nullptr);
	}
	else if (Codepage == CP_UTF7)
	{
		return Utf7::ToWideChar(Data, Size, Buffer, BufferSize, nullptr);
	}
	else if (Codepage == CP_REVERSEBOM)
	{
		if (Buffer)
			swap_bytes(Data, Buffer, std::min(Size, BufferSize * sizeof(wchar_t)));
		return Size / sizeof(wchar_t);
	}
	else
	{
		return MultiByteToWideChar(Codepage, 0, Data, static_cast<int>(Size), Buffer, static_cast<int>(BufferSize));
	}
}

string unicode::from(uintptr_t Codepage, const char* Data, size_t Size)
{
	if (const auto NewSize = unicode::from(Codepage, Data, Size, nullptr, 0))
	{
		string Buffer(NewSize, 0);
		unicode::from(Codepage, Data, Size, &Buffer[0], Buffer.size());
		return Buffer;
	}
	return string();
}

//################################################################################################

int Utf::ToWideChar(uintptr_t cp, const char *src, size_t len, wchar_t* out, size_t wlen, Errs *errs)
{
	if (cp == CP_UTF8)
		return Utf8::ToWideChar(src, len, out, wlen, errs);

	else if (cp == CP_UTF7)
		return Utf7::ToWideChar(src, len, out, wlen, errs);

	else
		return -1;
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
#define D(n) dir + b64 + n

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

static int Utf7_GetChar(const BYTE *bf, size_t cb, wchar_t& wc, int& state)
{
	if (!cb)
		return 0;

	int nc= 1, m[3];
	BYTE c = *bf++;
	if (c >= 128)
		return -nc;

	union
	{
		int state;
		struct { BYTE carry_bits; BYTE carry_count; bool base64; BYTE unused; } s;
	} u;
	u.state = state;

	m[0] = static_cast<int>(m7[c]);
	if ((m[0] & ill) != 0)
		return -nc;

	if (!u.s.base64)
	{
		if (c != (BYTE)'+')
		{
			wc = static_cast<wchar_t>(c);
			return nc;
		}
		if (cb < 2)
			return -nc;

		c = *bf++;
		nc = 2;
		if (c >= 128)
			return -nc;

		if (c == (BYTE)'-')
		{
			wc = L'+';
			return nc;
		}

		m[0] = static_cast<int>(m7[c]);
		if (0 == (m[0] & b64))
			return -nc;

		u.s.base64 = true;
		u.s.carry_count = 0;
	}

	int a = 2 - u.s.carry_count / 4;
	if (cb < static_cast<size_t>(nc) + a)
		return -nc-a;

	if ((c = *bf++) >= 128)
	{
		u.s.base64 = false;
		state = u.state;
		return -nc;
	}
	m[1] = static_cast<int>(m7[c]);
	if (0 == (m[1] & b64))
	{
		u.s.base64 = false;
		state = u.state;
		return -nc;
	}
	if (a < 2)
	{
		wc = static_cast<wchar_t>((u.s.carry_bits << 12) | ((BYTE)m[0] << 6) | (BYTE)m[1]);
		u.s.carry_count = 0;
	}
	else
	{
		++nc;
		if ((c = *bf++) >= 128)
		{
			u.s.base64 = false;
			state = u.state;
			return -nc;
		}
		m[2] = static_cast<int>(m7[c]);
		if (0 == (m[2] & b64))
		{
			u.s.base64 = false;
			state = u.state;
			return -nc;
		}
		unsigned m18 = ((BYTE)m[0] << 12) | ((BYTE)m[1] << 6) | ((BYTE)m[2]);

		if (u.s.carry_count == 0)
		{
			wc = static_cast<wchar_t>(m18 >> 2);
			u.s.carry_bits = (BYTE)(m18 & 0x03);
			u.s.carry_count = 2;
		}
		else
		{
			wc = static_cast<wchar_t>((u.s.carry_bits << 14) | (m18 >> 4));
			u.s.carry_bits = (BYTE)(m18 & 0x07);
			u.s.carry_count = 4;
		}
	}
	++nc;

	if (cb > static_cast<size_t>(nc) && *bf == (BYTE)'-')
	{
		u.s.base64 = false;
		++nc;
	}

	state = u.state;
	return nc;
}

int Utf7::ToWideChar(const char *src, size_t U_length, wchar_t* out, size_t U_wlen, Utf::Errs *errs)
{
	// BUGBUG
	int length = static_cast<int>(U_length);
	int wlen = static_cast<int>(U_wlen);

	if (errs)
	{
		errs->first_src = errs->first_out = -1;
		errs->count = 0;
		errs->small_buff = false;
	}

	if (!src || length <= 0)
		return 0;

	int state = 0, no = 0, ns = 0, ne = 0, move = 1;
	wchar_t dummy_out, w1 = Utf::REPLACE_CHAR;
	if (!out)
	{
		out = &dummy_out; move = 0;
	}

	while (length > ns)
	{
		int nc = Utf7_GetChar((const BYTE *)src+ns, length-ns, w1, state);
		if (!nc)
			break;

		if (nc < 0)
		{
			w1 = Utf::REPLACE_CHAR; nc = -nc;
			if (errs && 1 == ++ne)
			{
				errs->first_src = ns; errs->first_out = no;
			}
		}

		if (move && --wlen < 0)
		{
			if (errs)
				errs->small_buff = true;

			out = &dummy_out;
			move = 0;
		}

		*out = w1;
		out += move;
		++no;
		ns += nc;
	}

	if (errs)
		errs->count = ne;

	return no;
}

//################################################################################################

int Utf8::ToWideChar(const char *s, size_t U_nc, wchar_t *w1, wchar_t *w2, size_t U_wlen, int &tail)
{
	// BUGBUG
	int nc = static_cast<int>(U_nc);
	int wlen = static_cast<int>(U_wlen);

	bool need_one = wlen <= 0;
	if (need_one)
		wlen = 2;

	int ic = 0, nw = 0, wc;
	const auto InvalidChar = [](unsigned char c) -> int { return 0xDC00 + c; };

	while ( ic < nc )
	{
		unsigned char c1 = ((const unsigned char *)s)[ic++];

		if (c1 < 0x80) // simple ASCII
			wc = (wchar_t)c1;
		else if ( c1 < 0xC2 || c1 >= 0xF5 ) // illegal 1-st byte
			wc = InvalidChar(c1);
		else
		{ // multibyte (2, 3, 4)
			if (ic + 0 >= nc )
			{ // unfinished
unfinished:
				if ( nw > 0 )
					tail = nc - ic + 1;
				else
				{
					tail = 0;
					wc = InvalidChar(c1);
					w1[0] = wc;
					if (w2)
						w2[0] = wc;
					nw = 1;
				}
				return nw;
			}
			unsigned char c2 = ((const unsigned char *)s)[ic];
			if ( 0x80 != (c2 & 0xC0)       // illegal 2-nd byte
				|| (0xE0 == c1 && c2 <= 0x9F) // illegal 3-byte start (overlaps with 2-byte)
				|| (0xF0 == c1 && c2 <= 0x8F) // illegal 4-byte start (overlaps with 3-byte)
				|| (0xF4 == c1 && c2 >= 0x90) // illegal 4-byte (out of unicode range)
				)
			{
				wc = InvalidChar(c1);
			}
			else if ( c1 < 0xE0 )
			{ // legal 2-byte
				++ic;
				wc = ((c1 & 0x1F) << 6) | (c2 & 0x3F);
			}
			else
			{ // 3 or 4-byte
				if (ic + 1 >= nc )
					goto unfinished;
				unsigned char c3 = ((const unsigned char *)s)[ic+1];
				if ( 0x80 != (c3 & 0xC0) ) // illegal 3-rd byte
					wc = InvalidChar(c1);
				else if ( c1 < 0xF0 )
				{ // legal 3-byte
					wc = ((c1 & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
					if (wc >= 0xD800 && wc <= 0xDFFF) // invalid: surrogate area code
						wc = InvalidChar(c1);
					else
						ic += 2;
				}
				else
				{ // 4-byte
					if (ic + 2 >= nc )
						goto unfinished;

					unsigned char c4 = ((const unsigned char *)s)[ic+2];
					if ( 0x80 != (c4 & 0xC0) ) // illegal 4-th byte
						wc = InvalidChar(c1);
					else
					{ // legal 4-byte (produce 2 WCHARs)
						ic += 3;
						wc = ((c1 & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
						wc -= 0x10000;
						w1[nw] = (wchar_t)(0xD800 + (wc >> 10));
						if (w2)
							w2[nw] = w1[nw];
						++nw;
						wc = 0xDC00 + (wc & 0x3FF);
						assert(nw < wlen); //??? caller should be fixed to avoid this...
						if (nw >= wlen)
						{
							--nw;
							wc = Utf::REPLACE_CHAR;
						}
					}
				}
			}
		}

		if ( wc >= 0 )
		{
			w1[nw] = (wchar_t)wc;
			if (w2)
				w2[nw] = (wchar_t)wc;
		}
		else
		{
			w1[nw] = Utf::REPLACE_CHAR;
			if (w2)
				w2[nw] = L'?';
		}
		if (++nw >= wlen || need_one)
			break;
	}

	tail = nc - ic;
	return nw;
}

static inline int Utf8_GetChar(const char *src, size_t U_length, wchar_t* wc)
{
	// BUGBUG
	int length = static_cast<int>(U_length);

	wchar_t w1[2], w2[2];
	int tail;
	int WCharCount = Utf8::ToWideChar(src, length, w1, w2, -2, tail);

	if (WCharCount <= 0)
		return 0;

	wc[0] = w1[0];
	if (WCharCount > 1)
	{
		wc[1] = w1[1];
	}

	if (w1[0] == Utf::REPLACE_CHAR && w2[0] == L'?')
		return tail - length; // failed: negative
	else
		return length - tail; // succeed: positive
 }

int Utf8::ToWideChar(const char *src, size_t U_length, wchar_t* out, size_t U_wlen, Utf::Errs *errs)
{
	// BUGBUG
	int length = static_cast<int>(U_length);
	int wlen = static_cast<int>(U_wlen);

	if (errs)
	{
		errs->first_src = errs->first_out = -1;
		errs->count = 0;
		errs->small_buff = false;
	}

	if (!src || length <= 0)
		return 0;

	int no = 0, ns = 0, ne = 0, move = 1;
	wchar_t dummy_out[2];
	if (!out)
	{
		out = dummy_out; move = 0;
	}

	while (length > ns)
	{
		wchar_t w1[2] = {};
		int nc = Utf8_GetChar(src+ns, length-ns, w1);
		if (!nc)
			break;

		if (nc < 0)
		{
			w1[0] = Utf::REPLACE_CHAR; nc = -nc;
			if (errs && 1 == ++ne)
			{
				errs->first_src = ns; errs->first_out = no;
			}
		}

		const auto Decrement = w1[1] ? 2 : 1;
		if (move && (wlen -= Decrement) < 0)
		{
			if (errs)
				errs->small_buff = true;

			out = dummy_out;
			move = 0;
		}

		*out = w1[0];
		out += move;
		++no;

		if (w1[1])
		{
			*out = w1[1];
			out += move;
			++no;
		}

		ns += nc;
	}

	if (errs)
		errs->count = ne;

	return no;
}

size_t Utf8::ToMultiByte(const wchar_t *src, size_t len, char *dst)
{
	const wchar_t *end = src + len;
	size_t result=0;
	while (src < end)
	{
		unsigned int c = *src++;
		if (c < 0x80)
		{
			if (dst)
			{
				*dst++ = c;
			}
			result += 1;
		}
		else if (c < 0x800)
		{
			if (dst)
			{
				dst[0] = 0xC0 + (c >> 6);
				dst[1] = 0x80 + (c & 0x3F);
				dst += 2;
			}
			result += 2;
		}
		else if (c - 0xD800 > 0xDFFF - 0xD800) // not surrogates
		{
		l:
			if (dst)
			{
				dst[0] = 0xE0 + (c >> 12);
				dst[1] = 0x80 + (c >> 6 & 0x3F);
				dst[2] = 0x80 + (c & 0x3F);
				dst += 3;
			}
			result += 3;
		}
		else if (c - 0xDC80 <= 0xDCFF - 0xDC80) // embedded raw byte
		{
			if (dst)
			{
				*dst++ = c & 0xFF;
			}
			result += 1;
		}
		else if (c - 0xD800 <= 0xDBFF - 0xD800 && src < end && *src - 0xDC00u <= 0xDFFF - 0xDC00) // valid surrogate pair
		{
			c = 0x3C10000 + ((c - 0xD800) << 10) + (unsigned int)*src++ - 0xDC00;
			if (dst)
			{
				dst[0] = c >> 18;
				dst[1] = 0x80 + (c >> 12 & 0x3F);
				dst[2] = 0x80 + (c >> 6 & 0x3F);
				dst[3] = 0x80 + (c & 0x3F);
				dst += 4;
			}
			result += 4;
		}
		else
		{
			c = 0xFFFD; // invalid: mapped to 'Replacement Character'
			goto l;
		}
	}
	return result;
}

void swap_bytes(const void* Src, void* Dst, size_t SizeInBytes)
{
	_swab(reinterpret_cast<char*>(const_cast<void*>(Src)), reinterpret_cast<char*>(Dst), static_cast<int>(SizeInBytes));
}
