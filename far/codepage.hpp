#ifndef CODEPAGE_HPP_44AE7032_AF79_4A6F_A2ED_529BC1A38758
#define CODEPAGE_HPP_44AE7032_AF79_4A6F_A2ED_529BC1A38758
#pragma once

/*
codepage.hpp

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

namespace unicode
{
	#define NOT_PTR(T) typename T, ENABLE_IF(!std::is_pointer<T>::value)

	size_t to(uintptr_t Codepage, const wchar_t* Data, size_t Size, char* Buffer, size_t BufferSize, bool* UsedDefaultChar = nullptr);

	template<NOT_PTR(T)>
	auto to(uintptr_t Codepage, const T& Data, char* Buffer, size_t BufferSize, bool* UsedDefaultChar = nullptr)
	{
		return to(Codepage, Data.data(), Data.size(), Buffer, BufferSize, UsedDefaultChar);
	}

	template<NOT_PTR(T)>
	auto to(uintptr_t Codepage, const wchar_t* Data, size_t Size, T& Buffer, bool* UsedDefaultChar = nullptr)
	{
		return to(Codepage, Data, Size, Buffer.data(), Buffer.size(), UsedDefaultChar);
	}

	template<NOT_PTR(T), NOT_PTR(Y)>
	auto to(uintptr_t Codepage, const T& Data, Y& Buffer, bool* UsedDefaultChar = nullptr)
	{
		return to(Codepage, Data.data(), Data.size(), Buffer.data(), Buffer.size(), UsedDefaultChar);
	}

	std::string to(uintptr_t Codepage, const wchar_t* Data, size_t Size, bool* UsedDefaultChar = nullptr);

	inline auto to(uintptr_t Codepage, const wchar_t* Data, bool* UsedDefaultChar = nullptr)
	{
		return to(Codepage, Data, wcslen(Data), UsedDefaultChar);
	}

	template<NOT_PTR(T)>
	auto to(uintptr_t Codepage, const T& Data, bool* UsedDefaultChar = nullptr)
	{
		return to(Codepage, Data.data(), Data.size(), UsedDefaultChar);
	}

	size_t from(uintptr_t Codepage, const char* Data, size_t Size, wchar_t* Buffer, size_t BufferSize);

	template<NOT_PTR(T)>
	auto from(uintptr_t Codepage, const T& Data, wchar_t* Buffer, size_t BufferSize)
	{
		return from(Codepage, Data.data(), Data.size(), Buffer, BufferSize);
	}

	template<NOT_PTR(T)>
	auto from(uintptr_t Codepage, const char* Data, size_t Size, T& Buffer)
	{
		return from(Codepage, Data, Size, Buffer.data(), Buffer.size());
	}

	template<NOT_PTR(T), NOT_PTR(Y)>
	auto from(uintptr_t Codepage, const T& Data, Y& Buffer)
	{
		return from(Codepage, Data.data(), Data.size(), Buffer.data(), Buffer.size());
	}

	string from(uintptr_t Codepage, const char* Data, size_t Size);

	inline auto from(uintptr_t Codepage, const char* Data)
	{
		return from(Codepage, Data, strlen(Data));
	}

	template<NOT_PTR(T)>
	auto from(uintptr_t Codepage, const T& Data)
	{
		return from(Codepage, Data.data(), Data.size());
	}

#undef NOT_PTR
}

void swap_bytes(const void* Src, void* Dst, size_t SizeInBytes);

inline bool IsUnicodeCodePage(uintptr_t cp) { return cp == CP_UNICODE || cp == CP_REVERSEBOM; }
inline bool IsStandardCodePage(uintptr_t cp) { return IsUnicodeCodePage(cp) || cp == CP_UTF8 || cp == GetOEMCP() || cp == GetACP(); }
inline bool IsUnicodeOrUtfCodePage(uintptr_t cp) { return IsUnicodeCodePage(cp) || cp==CP_UTF8 || cp==CP_UTF7; }

// See https://msdn.microsoft.com/en-us/library/windows/desktop/dd319072.aspx
inline bool IsNoFlagsCodepage(uintptr_t cp) { return (cp >= 50220 && cp <= 50222) || cp == 50225 || cp == 50227 || cp == 50229 || (cp >= 57002 && cp <= 57011) || cp == CP_UTF7 || cp == CP_SYMBOL; }


//#############################################################################

class MultibyteCodepageDecoder: noncopyable
{
public:
	bool SetCP(uintptr_t Codepage);
	uintptr_t GetCP() const { return m_Codepage; }
	size_t GetSize() const { return m_Size; }
	size_t GetChar(const char* Buffer, size_t Size, wchar_t& Char, bool* End = nullptr) const;

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


	struct Errs
	{
		int first_src;
		int first_out;
		int count;
		bool small_buff;
	};

	int ToWideChar(uintptr_t cp, const char *src, size_t len, wchar_t* out, size_t wlen, Errs *errs);
}

namespace Utf7 {
	int ToWideChar(const char *src, size_t len, wchar_t* out, size_t wlen, Utf::Errs *errs);
}

namespace Utf8 {
	int ToWideChar(const char *s, size_t nc, wchar_t *w1, wchar_t *w2, size_t wlen, int &tail);
	int ToWideChar(const char *src, size_t len, wchar_t* out, size_t wlen, Utf::Errs *errs);
	size_t ToMultiByte(const wchar_t *src, size_t len, char *dst);
}

//#############################################################################

class raw_eol
{
public:
	raw_eol(): m_Cr('\r'), m_Lf('\n') {}
	raw_eol(uintptr_t Codepage): m_Cr(to(Codepage, L'\r')), m_Lf(to(Codepage, L'\n')) {}

	template <class T>
	T cr() const = delete;
	template <class T>
	T lf() const = delete;

private:
	static char to(uintptr_t Codepage, wchar_t WideChar)
	{
		char Char = WideChar;
		unicode::to(Codepage, &WideChar, 1, &Char, 1);
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
typedef std::unordered_map<UINT, std::pair<UINT, string>> cp_map;
const cp_map& InstalledCodepages();
cp_map::value_type::second_type GetCodePageInfo(UINT cp);


#endif // CODEPAGE_HPP_44AE7032_AF79_4A6F_A2ED_529BC1A38758
