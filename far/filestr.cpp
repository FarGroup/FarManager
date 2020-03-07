﻿/*
filestr.cpp

Класс GetFileString
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

// Self:
#include "filestr.hpp"

// Internal:
#include "nsUniversalDetectorEx.hpp"
#include "config.hpp"
#include "codepage_selection.hpp"
#include "global.hpp"

// Platform:

// Common:
#include "common/algorithm.hpp"
#include "common/bytes_view.hpp"
#include "common/enum_tokens.hpp"
#include "common/io.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

const auto BufferSize = 4096;
static_assert(BufferSize % sizeof(wchar_t) == 0);

enum_lines::enum_lines(std::istream& Stream, uintptr_t CodePage):
	m_Stream(Stream),
	m_StreamExceptions(m_Stream.exceptions()),
	m_BeginPos(m_Stream.tellg()),
	m_CodePage(CodePage),
	m_Eol(m_CodePage),
	m_Buffer(BufferSize)
{
	Stream.exceptions(m_StreamExceptions & ~(Stream.failbit | Stream.eofbit));

	m_wStr.reserve(1024);
	if (!IsUnicodeCodePage(m_CodePage))
		m_Str.reserve(m_wStr.capacity());
}

enum_lines::~enum_lines()
{
	try
	{
		m_Stream.exceptions(m_StreamExceptions);
	}
	catch (const std::exception&)
	{
		// TODO: log
	}
}

bool enum_lines::get(bool Reset, file_line& Value) const
{
	if (Reset)
	{
		m_Data = {};

		if (!m_Stream.bad() && m_Stream.eof())
			m_Stream.clear();

		m_Stream.seekg(m_BeginPos);
	}

	return GetString(Value.Str, Value.Eol);
}

static size_t get_chars(uintptr_t const Codepage, std::string_view const From, span<wchar_t> const To, bool& ConversionError)
{
	if (Codepage == CP_UTF8 || Codepage == CP_UTF7)
	{
		Utf::errors Errors;
		const auto Size = Utf::get_chars(Codepage, From, To, &Errors);
		if (Errors.Conversion.Error)
			ConversionError = true;
		return Size;
	}

	if (!ConversionError)
	{
		if (const auto Size = MultiByteToWideChar(Codepage, MB_ERR_INVALID_CHARS, From.data(), static_cast<int>(From.size()), To.data(), static_cast<int>(To.size())))
			return Size;

		if (const auto Error = GetLastError(); Error == ERROR_NO_UNICODE_TRANSLATION || (Error == ERROR_INVALID_FLAGS && IsNoFlagsCodepage(Codepage)))
		{
			ConversionError = true;
		}
	}

	return encoding::get_chars(Codepage, From, To);
}

bool enum_lines::fill() const
{
	const auto Read = io::read(m_Stream, { m_Buffer.get(), m_Buffer.size() });
	if (!Read)
		return false;

	m_Data = { m_Buffer.get(), Read };

	if (IsUnicodeCodePage(m_CodePage))
	{
		if (const auto MissingBytes = Read % sizeof(wchar_t))
		{
			// EOF in the middle of the character
			// Logically we should return REPLACE_CHAR at this point and call it a day, however:
			// - This class is used in the editor
			// - People often use the editor to edit binary files
			// - If we return REPLACE_CHAR, the incomplete char will be lost
			// - If we pretend that the remaining bytes are \0, the worst thing that could happen is trailing \0 bytes after save.
			std::fill_n(m_Buffer.get() + Read, MissingBytes, '\0');
			m_Data = { m_Buffer.get(), Read + MissingBytes };
			m_ConversionError = true;
		}

		if (m_CodePage == CP_REVERSEBOM)
			swap_bytes(m_Buffer.get(), m_Buffer.get(), m_Data.size());
	}

	return true;
}

template<typename T>
bool enum_lines::GetTString(std::basic_string<T>& To, eol& Eol, bool BigEndian) const
{
	To.clear();

	if (m_CrCr)
	{
		Eol = eol::mac;
		m_CrCr = false;
		return true;
	}

	const auto
		EolCr = m_Eol.cr<T>(),
		EolLf = m_Eol.lf<T>();

	const T AnyEolBuffer[]{ EolCr, EolLf };
	const std::basic_string_view AnyEol(AnyEolBuffer, std::size(AnyEolBuffer));

	const auto ProcessAfterCr = [&](std::basic_string_view<T> const Str)
	{
		if (Str.front() == EolLf)
		{
			Eol = eol::win;
			return 2;
		}

		if (Str.front() != EolCr)
		{
			Eol = eol::mac;
			return 1;
		}

		if (Str.size() == 1)
		{
			m_CrCr = true;
			return 0;
		}

		if (Str[1] == EolLf)
		{
			Eol = eol::bad_win;
			return 3;
		}

		Eol = eol::mac;
		return 1;
	};

	const auto Cast = [&]
	{
		return std::basic_string_view{ static_cast<const T*>(static_cast<const void*>(m_Data.data())), m_Data.size() / sizeof(T) };
	};

	for (;;)
	{
		if (m_Data.empty() && !fill())
		{
			Eol = eol::none;
			return !To.empty();
		}

		const auto StrData = Cast();
		const auto EolPos = StrData.find_first_of(AnyEol);

		To.append(StrData, 0, EolPos);

		if (EolPos == StrData.npos)
		{
			m_Data = {};
			continue;
		}

		m_Data.remove_prefix(EolPos * sizeof(T));

		if (StrData[EolPos] == EolLf)
		{
			Eol = eol::unix;
			m_Data.remove_prefix(sizeof(T));
			return true;
		}

		if (EolPos + 1 != StrData.size())
		{
			if (const auto ToRemove = ProcessAfterCr(StrData.substr(EolPos + 1)))
			{
				m_Data.remove_prefix(ToRemove * sizeof(T));
				return true;
			}
		}

		// (('\r' or '\r\r') and DataEnd): inconclusive, get more
		m_Data = {};

		if (!fill())
		{
			Eol = eol::mac;
			return true;
		}

		m_Data.remove_prefix((ProcessAfterCr(Cast()) - 1) * sizeof(T));
		return true;
	}
}

bool enum_lines::GetString(string_view& Str, eol& Eol) const
{
	if (IsUnicodeCodePage(m_CodePage))
	{
		if (!GetTString(m_wStr, Eol, m_CodePage == CP_REVERSEBOM))
			return false;
	}
	else
	{
		if (!GetTString(m_Str, Eol))
			return false;

		if (!m_Str.empty())
		{
			m_wStr.resize(std::max(m_wStr.size(), m_Str.size()));

			for (auto Overflow = true; Overflow;)
			{
				const auto Size = get_chars(m_CodePage, m_Str, m_wStr, m_ConversionError);
				Overflow = Size > m_wStr.size();
				m_wStr.resize(Size);
			}

			m_Str.clear();
		}
		else
		{
			m_wStr.clear();
		}
	}

	Str = m_wStr;
	return true;
}

// If the file contains a BOM this function will advance the file pointer by the BOM size (either 2 or 3)
static bool GetUnicodeCpUsingBOM(const os::fs::file& File, uintptr_t& Codepage)
{
	char Buffer[3]{};
	size_t BytesRead = 0;
	if (!File.Read(Buffer, std::size(Buffer), BytesRead))
		return false;

	std::string_view const Signature(Buffer, std::size(Buffer));

	if (BytesRead >= 2)
	{
		if (Signature.substr(0, 2) == encoding::get_signature_bytes(CP_UNICODE))
		{
			Codepage = CP_UNICODE;
			File.SetPointer(2, nullptr, FILE_BEGIN);
			return true;
		}

		if (Signature.substr(0, 2) == encoding::get_signature_bytes(CP_REVERSEBOM))
		{
			Codepage = CP_REVERSEBOM;
			File.SetPointer(2, nullptr, FILE_BEGIN);
			return true;
		}
	}

	if (BytesRead >= 3 && Signature == encoding::get_signature_bytes(CP_UTF8))
	{
		Codepage = CP_UTF8;
		File.SetPointer(3, nullptr, FILE_BEGIN);
		return true;
	}

	File.SetPointer(0, nullptr, FILE_BEGIN);
	return false;
}

static bool GetUnicodeCpUsingWindows(const void* Data, size_t Size, uintptr_t& Codepage)
{
	// MSDN documents IS_TEXT_UNICODE_BUFFER_TOO_SMALL but there is no such thing
	if (Size < 2)
		return false;

	int Test = IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_REVERSE_MASK | IS_TEXT_UNICODE_NOT_UNICODE_MASK | IS_TEXT_UNICODE_NOT_ASCII_MASK;

	// return value is ignored - only some tests might pass
	IsTextUnicode(Data, static_cast<int>(Size), &Test);

	if ((Test & IS_TEXT_UNICODE_NOT_UNICODE_MASK) || !(Test & IS_TEXT_UNICODE_NOT_ASCII_MASK))
		return false;

	if (Test & IS_TEXT_UNICODE_UNICODE_MASK)
	{
		Codepage = CP_UNICODE;
		return true;
	}

	if (Test & IS_TEXT_UNICODE_REVERSE_MASK)
	{
		Codepage = CP_REVERSEBOM;
		return true;
	}

	return false;
}

static bool GetCpUsingUniversalDetectorWithExceptions(std::string_view const Str, uintptr_t& Codepage)
{
	if (!GetCpUsingUniversalDetector(Str, Codepage))
		return false;

	// This whole block shouldn't be here
	if (Global->Opt->strNoAutoDetectCP.Get() == L"-1"sv)
	{
		if (Global->Opt->CPMenuMode && static_cast<UINT>(Codepage) != encoding::codepage::ansi() && static_cast<UINT>(Codepage) != encoding::codepage::oem())
		{
			const auto CodepageType = codepages::GetFavorite(Codepage);
			if (!(CodepageType & CPST_FAVORITE))
				return false;
		}
	}
	else
	{
		if (contains(enum_tokens(Global->Opt->strNoAutoDetectCP.Get(), L",;"sv), str(Codepage)))
			return false;
	}

	return true;
}

// If the file contains a BOM this function will advance the file pointer by the BOM size (either 2 or 3)
static bool GetFileCodepage(const os::fs::file& File, uintptr_t DefaultCodepage, uintptr_t& Codepage, bool& SignatureFound, bool& NotUTF8, bool& NotUTF16, bool UseHeuristics)
{
	if (GetUnicodeCpUsingBOM(File, Codepage))
	{
		SignatureFound = true;
		return true;
	}

	if (!UseHeuristics)
		return false;

	// TODO: configurable
	const size_t Size = 32768;
	char_ptr const Buffer(Size);
	size_t ReadSize = 0;

	const auto ReadResult = File.Read(Buffer.get(), Size, ReadSize);
	File.SetPointer(0, nullptr, FILE_BEGIN);

	if (!ReadResult || !ReadSize)
		return false;

	if (GetUnicodeCpUsingWindows(Buffer.get(), ReadSize, Codepage))
		return true;

	NotUTF16 = true;

	unsigned long long FileSize = 0;
	const auto WholeFileRead = File.GetSize(FileSize) && ReadSize == FileSize;
	bool PureAscii = false;

	if (encoding::is_valid_utf8({ Buffer.get(), ReadSize }, !WholeFileRead, PureAscii))
	{
		if (!PureAscii)
			Codepage = CP_UTF8;
		else if (DefaultCodepage == CP_UTF8 || DefaultCodepage == encoding::codepage::ansi() || DefaultCodepage == encoding::codepage::oem())
			Codepage = DefaultCodepage;
		else
			Codepage = encoding::codepage::ansi();

		return true;
	}

	NotUTF8 = true;

	return GetCpUsingUniversalDetectorWithExceptions({ Buffer.get(), ReadSize }, Codepage);
}

uintptr_t GetFileCodepage(const os::fs::file& File, uintptr_t DefaultCodepage, bool* SignatureFound, bool UseHeuristics)
{
	bool SignatureFoundValue = false;
	uintptr_t Codepage;
	bool NotUTF8 = false;
	bool NotUTF16 = false;

	if (!GetFileCodepage(File, DefaultCodepage, Codepage, SignatureFoundValue, NotUTF8, NotUTF16, UseHeuristics))
	{
		Codepage =
			(NotUTF8 && DefaultCodepage == CP_UTF8) || (NotUTF16 && IsUnicodeCodePage(DefaultCodepage))?
				encoding::codepage::ansi() :
				DefaultCodepage;
	}

	if (SignatureFound)
		*SignatureFound = SignatureFoundValue;

	return Codepage;
}

#ifdef ENABLE_TESTS

#include "testing.hpp"
#include "mix.hpp"

TEST_CASE("enum_lines")
{
	static const struct
	{
		string_view Str;
		std::initializer_list<const std::pair<string_view, eol>> Result;
	}
	Tests[]
	{
		{ L""sv, {
		}},
		{ L"dQw4w9WgXcQ"sv, {
			{ L"dQw4w9WgXcQ"sv, eol::none },
		}},
		{ L"\r"sv, {
			{ L""sv, eol::mac },
		}},
		{ L"\n"sv, {
			{ L""sv, eol::unix },
		}},
		{ L"\r\n"sv, {
			{ L""sv, eol::win },
		}},
		{ L"\r\r\n"sv, {
			{ L""sv, eol::bad_win },
		}},
		{ L"Oi!\r"sv, {
			{ L"Oi!"sv, eol::mac },
		}},
		{ L"Oi!\n"sv, {
			{ L"Oi!"sv, eol::unix },
		}},
		{ L"Oi!\r\n"sv, {
			{ L"Oi!"sv, eol::win },
		}},
		{ L"Oi!\r\r\n"sv, {
			{ L"Oi!"sv, eol::bad_win },
		}},
		{ L"\rOi!"sv, {
			{ L""sv,    eol::mac },
			{ L"Oi!"sv, eol::none },
		}},
		{ L"\nOi!"sv, {
			{ L""sv,    eol::unix },
			{ L"Oi!"sv, eol::none },
		}},
		{ L"\r\nOi!"sv, {
			{ L""sv,    eol::win },
			{ L"Oi!"sv, eol::none },
		}},
		{ L"\r\r\nOi!"sv, {
			{ L""sv,    eol::bad_win },
			{ L"Oi!"sv, eol::none },
		}},
		{ L"\r\r"sv, {
			{ L""sv, eol::mac },
			{ L""sv, eol::mac },
		}},
		{ L"\n\n"sv, {
			{ L""sv, eol::unix },
			{ L""sv, eol::unix },
		}},
		{ L"\r\n\r\n"sv, {
			{ L""sv, eol::win },
			{ L""sv, eol::win },
		}},
		{ L"\r\r\n\r\r\n"sv, {
			{ L""sv, eol::bad_win },
			{ L""sv, eol::bad_win },
		}},
		{ L"\n\r\r\n\n\r\r\r\r\n\r\n\r\r"sv, {
			{ L""sv, eol::unix },
			{ L""sv, eol::bad_win },
			{ L""sv, eol::unix },
			{ L""sv, eol::mac },
			{ L""sv, eol::mac },
			{ L""sv, eol::bad_win },
			{ L""sv, eol::win },
			{ L""sv, eol::mac },
			{ L""sv, eol::mac },
		}},
		{ L"Ho\nho\rho!\r\n\r\r\n"sv, {
			{ L"Ho"sv,  eol::unix },
			{ L"ho"sv,  eol::mac },
			{ L"ho!"sv, eol::win },
			{ L""sv,    eol::bad_win },
		}},
	};

	for (const auto& i: Tests)
	{
		for (const auto Codepage: { CP_UNICODE, CP_REVERSEBOM, static_cast<uintptr_t>(CP_UTF8) })
		{
			auto Str = encoding::get_bytes(Codepage, i.Str);
			std::istringstream Stream(Str);
			Stream.exceptions(Stream.badbit | Stream.failbit);

			const auto Enumerator = enum_lines(Stream, Codepage);

			// Twice to make sure that reset works as expected
			for (/*[[maybe_unused]]*/ const auto n: { 0, 1 })
			{
				(void)n; // [[maybe_unused]] causes ICE in VS2017. TODO: Remove after we move to 2019 or later

				auto Iterator = i.Result.begin();

				for (const auto& Line : Enumerator)
				{
					REQUIRE(Iterator != i.Result.end());
					REQUIRE(Iterator->first == Line.Str);
					REQUIRE(Iterator->second == Line.Eol);
					++Iterator;
				}

				REQUIRE(Stream.eof());
				REQUIRE(Iterator == i.Result.end());
			}
		}
	}
}
#endif
