/*
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "filestr.hpp"

// Internal:
#include "nsUniversalDetectorEx.hpp"
#include "config.hpp"
#include "codepage.hpp"
#include "codepage_selection.hpp"
#include "global.hpp"
#include "log.hpp"

// Platform:
#include "platform.com.hpp"
#include "platform.fs.hpp"

// Common:
#include "common/algorithm.hpp"
#include "common/bytes_view.hpp"
#include "common/enum_tokens.hpp"
#include "common/io.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

const auto BufferSize = 65536;
static_assert(BufferSize % sizeof(wchar_t) == 0);

enum_lines::enum_lines(std::istream& Stream, uintptr_t CodePage):
	m_Stream(Stream),
	m_BeginPos(m_Stream.tellg()),
	m_CodePage(CodePage),
	m_Eol(m_CodePage),
	m_Buffer(BufferSize)
{
	if (IsUtf16CodePage(m_CodePage))
	{
		m_Data.emplace<string>().reserve(default_capacity);
	}
	else
	{
		m_Data.emplace<conversion_data>().m_Bytes.reserve(default_capacity);
	}
}

bool enum_lines::get(bool Reset, file_line& Value) const
{
	if (Reset)
	{
		m_BufferView = {};

		if (!m_Stream.bad() && m_Stream.eof())
			m_Stream.clear();

		m_Stream.seekg(m_BeginPos);
	}

	return GetString(Value.Str, Value.Eol);
}

bool enum_lines::fill() const
{
	const auto Read = io::read(m_Stream, edit_bytes(m_Buffer));
	if (!Read)
		return false;

	m_BufferView = { m_Buffer.data(), Read };

	if (IsUtf16CodePage(m_CodePage))
	{
		const auto CharSize = sizeof(char16_t);

		if (const auto ExtraBytes = Read % CharSize)
		{
			// EOF in the middle of the character
			// Logically we should return REPLACE_CHAR at this point and call it a day, however:
			// - This class is used in the editor
			// - People often use the editor to edit binary files
			// - If we return REPLACE_CHAR, the incomplete char will be lost
			// - If we pretend that the remaining bytes are \0, the worst thing that could happen is trailing \0 bytes after save.
			const auto MissingBytes = CharSize - ExtraBytes;
			std::fill_n(m_Buffer.begin() + Read, MissingBytes, '\0');
			m_BufferView = { m_Buffer.data(), Read + MissingBytes };
			m_Diagnostics.ErrorPosition = 0;
		}

		if (m_CodePage == CP_UTF16BE)
		{
			static_assert(std::endian::native == std::endian::little, "No way");
			swap_bytes(m_Buffer.data(), m_Buffer.data(), m_BufferView.size(), CharSize);
		}
	}

	return true;
}

template<typename T>
bool enum_lines::GetTString(std::basic_string<T>& To, eol& Eol) const
{
	To.clear();

	if (m_EmitExtraCr)
	{
		Eol = eol::mac;
		m_EmitExtraCr = false;
		return true;
	}

	const T
		EolCr = m_Eol.cr(),
		EolLf = m_Eol.lf();

	for (;;)
	{
		const auto Char = !m_BufferView.empty() || fill()?
			std::optional<T>{ std::basic_string_view<T>{ std::bit_cast<T const*>(m_BufferView.data()), m_BufferView.size() / sizeof(T) }.front() } :
			std::optional<T>{};

		if (Char == EolLf)
		{
			m_BufferView.remove_prefix(sizeof(T));

			switch (m_CrSeen)
			{
			case 0:
				Eol = eol::unix;
				return true;

			case 1:
				Eol = eol::win;
				m_CrSeen = 0;
				return true;

			case 2:
				Eol = eol::bad_win;
				m_CrSeen = 0;
				return true;

			default:
				std::unreachable();
			}
		}

		if (Char == EolCr)
		{
			m_BufferView.remove_prefix(sizeof(T));

			switch (m_CrSeen)
			{
			case 0:
			case 1:
				++m_CrSeen;
				continue;

			case 2:
				Eol = eol::mac;
				m_EmitExtraCr = true;
				m_CrSeen = 1;
				return true;

			default:
				std::unreachable();
			}
		}

		if (m_CrSeen)
		{
			Eol = eol::mac;
			if (m_CrSeen == 2)
				m_EmitExtraCr = true;
			m_CrSeen = 0;
			return true;
		}

		if (Char)
		{
			m_BufferView.remove_prefix(sizeof(T));
			To.push_back(*Char);
		}
		else
		{
			Eol = eol::none;
			return !To.empty();
		}
	}
}

bool enum_lines::GetString(string_view& Str, eol& Eol) const
{
	return std::visit(overload
	{
		[&](string& String)
		{
			if (!GetTString(String, Eol))
				return false;

			Str = String;
			return true;

		},
		[&](const conversion_data& Data)
		{
			if (!GetTString(Data.m_Bytes, Eol))
				return false;

			if (Data.m_Bytes.empty())
			{
				Str = {};
				return true;
			}

			if (Data.m_Bytes.size() > Data.m_wBuffer.size())
				Data.m_wBuffer.reset(Data.m_Bytes.size());

			for (;;)
			{
				const auto Size = encoding::get_chars(m_CodePage, Data.m_Bytes, Data.m_wBuffer, &m_Diagnostics);
				if (Size <= Data.m_wBuffer.size())
				{
					Data.m_Bytes.clear();
					Str = { Data.m_wBuffer.data(), Size };
					return true;
				}

				Data.m_wBuffer.reset(Size);
			}
		}
	}, m_Data);
}

// If the file contains a BOM this function will advance the file pointer by the BOM size (either 2 or 3)
static bool GetUnicodeCpUsingBOM(const os::fs::file& File, uintptr_t& Codepage)
{
	char Buffer[3]{};
	size_t BytesRead = 0;
	if (!File.Read(Buffer, std::size(Buffer), BytesRead))
		return false;

	std::string_view const Data(Buffer, std::size(Buffer));

	for (const auto i: { static_cast<uintptr_t>(CP_UTF8), CP_UTF16LE, CP_UTF16BE })
	{
		const auto Signature = encoding::get_signature_bytes(i);
		if (Data.starts_with(Signature))
		{
			Codepage = i;
			File.SetPointer(Signature.size(), {}, FILE_BEGIN);
			return true;
		}
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
		Codepage = CP_UTF16LE;
		return true;
	}

	if (Test & IS_TEXT_UNICODE_REVERSE_MASK)
	{
		Codepage = CP_UTF16BE;
		return true;
	}

	return false;
}

static bool GetCpUsingML(std::string_view Str, uintptr_t& Codepage, function_ref<bool(uintptr_t)> const IsCodepageAcceptable)
{
	SCOPED_ACTION(os::com::initialize);

	os::com::ptr<IMultiLanguage2> ML;
	if (const auto Result = CoCreateInstance(CLSID_CMultiLanguage, {}, CLSCTX_INPROC_SERVER, IID_IMultiLanguage2, IID_PPV_ARGS_Helper(&ptr_setter(ML))); FAILED(Result))
	{
		LOGWARNING(L"CoCreateInstance(CLSID_CMultiLanguage): {}"sv, os::format_error(Result));
		return false;
	}

	// https://bugs.farmanager.com/view.php?id=4000
	// The Windows implementation of this API keeps the string length as unsigned short and does some math with it while calculating a total score or something.
	// It the string size is 32768 (which we use by default), it can sometimes add up to 65536, which does not fit in unsigned short and becomes 0.
	// That 0 later goes to a denominator somewhere with rather predictable results.
	// MS didn't SEH-guard the function, so the exception leaks back into the process and crashes it.
	// All the factors of 65536 are powers of two, so by reducing such sizes we ensure that even if it overflows, it won't add up to 65536, won't become 0 and won't crash.
	if (std::has_single_bit(Str.size()))
		Str.remove_suffix(1);

	int Size = static_cast<int>(Str.size());
	DetectEncodingInfo Info[16];
	int InfoCount = static_cast<int>(std::size(Info));

	if (const auto Result = ML->DetectInputCodepage(MLDETECTCP_NONE, 0, const_cast<char*>(Str.data()), &Size, Info, &InfoCount); FAILED(Result))
		return false;

	const auto Scores = std::span(Info, InfoCount);
	std::ranges::sort(Scores, [](DetectEncodingInfo const& a, DetectEncodingInfo const& b) { return a.nDocPercent > b.nDocPercent; });

	const auto It = std::ranges::find_if(Scores, [&](DetectEncodingInfo const& i) { return i.nLangID != 0xffffffff && IsCodepageAcceptable(i.nCodePage); });
	if (It == Scores.end())
		return false;

	Codepage = It->nCodePage;
	return true;
}

static bool GetCpUsingHeuristicsWithExceptions(std::string_view const Str, uintptr_t& Codepage)
{
	const auto IsCodepageNotBlacklisted = [](uintptr_t const Cp)
	{
		return !contains(enum_tokens(Global->Opt->strNoAutoDetectCP.Get(), L",;"sv), str(Cp));
	};

	const auto IsCodepageWhitelisted = [](uintptr_t const Cp)
	{
		if (!Global->Opt->CPMenuMode)
			return true;

		if (any_of(Cp, encoding::codepage::ansi(), encoding::codepage::oem()))
			return true;

		const auto CodepageType = codepages::GetFavorite(Cp);
		return (CodepageType & CPST_FAVORITE) != 0;
	};

	const auto IsCodepageAcceptable =
		Global->Opt->strNoAutoDetectCP == L"-1"sv?
			function_ref(IsCodepageWhitelisted) :
			function_ref(IsCodepageNotBlacklisted);

	if (GetCpUsingUniversalDetector(Str, Codepage) && IsCodepageAcceptable(Codepage))
		return true;

	return GetCpUsingML(Str, Codepage, IsCodepageAcceptable);
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

	const auto ReadResult = File.Read(Buffer.data(), Size, ReadSize);
	File.SetPointer(0, nullptr, FILE_BEGIN);

	if (!ReadResult || !ReadSize)
		return false;

	if (GetUnicodeCpUsingWindows(Buffer.data(), ReadSize, Codepage))
	{
		return true;
	}

	NotUTF16 = true;

	unsigned long long FileSize = 0;
	const auto WholeFileRead = File.GetSize(FileSize) && ReadSize == FileSize;

	if (const auto IsUtf8 = encoding::is_valid_utf8({ Buffer.data(), ReadSize }, !WholeFileRead); IsUtf8 != encoding::is_utf8::no)
	{
		if (IsUtf8 == encoding::is_utf8::yes)
			Codepage = CP_UTF8;
		else if (DefaultCodepage == CP_UTF8 || DefaultCodepage == encoding::codepage::ansi() || DefaultCodepage == encoding::codepage::oem())
			Codepage = DefaultCodepage;
		else
			Codepage = encoding::codepage::ansi();

		return true;
	}

	NotUTF8 = true;

	return GetCpUsingHeuristicsWithExceptions({ Buffer.data(), ReadSize }, Codepage);
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
			(NotUTF8 && DefaultCodepage == CP_UTF8) || (NotUTF16 && IsUtf16CodePage(DefaultCodepage))?
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
		{ {}, {
		}},
		{ L"dQw4w9WgXcQ"sv, {
			{ L"dQw4w9WgXcQ"sv, eol::none },
		}},
		{ L"\r"sv, {
			{ {}, eol::mac },
		}},
		{ L"\n"sv, {
			{ {}, eol::unix },
		}},
		{ L"\r\n"sv, {
			{ {}, eol::win },
		}},
		{ L"\r\r\n"sv, {
			{ {}, eol::bad_win },
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
			{ {},       eol::mac },
			{ L"Oi!"sv, eol::none },
		}},
		{ L"\nOi!"sv, {
			{ {},       eol::unix },
			{ L"Oi!"sv, eol::none },
		}},
		{ L"\r\nOi!"sv, {
			{ {},       eol::win },
			{ L"Oi!"sv, eol::none },
		}},
		{ L"\r\r\nOi!"sv, {
			{ {},       eol::bad_win },
			{ L"Oi!"sv, eol::none },
		}},
		{ L"\r\r"sv, {
			{ {}, eol::mac },
			{ {}, eol::mac },
		}},
		{ L"\n\n"sv, {
			{ {}, eol::unix },
			{ {}, eol::unix },
		}},
		{ L"\r\n\r\n"sv, {
			{ {}, eol::win },
			{ {}, eol::win },
		}},
		{ L"\r\r\n\r\r\n"sv, {
			{ {}, eol::bad_win },
			{ {}, eol::bad_win },
		}},
		{ L"\n\r\r\n\n\r\r\r\r\n\r\n\r\r"sv, {
			{ {}, eol::unix },
			{ {}, eol::bad_win },
			{ {}, eol::unix },
			{ {}, eol::mac },
			{ {}, eol::mac },
			{ {}, eol::bad_win },
			{ {}, eol::win },
			{ {}, eol::mac },
			{ {}, eol::mac },
		}},
		{ L"Ho\nho\rho!\r\n\r\r\n"sv, {
			{ L"Ho"sv,  eol::unix },
			{ L"ho"sv,  eol::mac },
			{ L"ho!"sv, eol::win },
			{ {},       eol::bad_win },
		}},
	};

	for (const auto& i: Tests)
	{
		for (const auto Codepage: { CP_UTF16LE, CP_UTF16BE, static_cast<uintptr_t>(CP_UTF8) })
		{
			auto Str = encoding::get_bytes(Codepage, i.Str);
			std::istringstream Stream(Str);
			Stream.exceptions(Stream.badbit | Stream.failbit);

			const auto Enumerator = enum_lines(Stream, Codepage);

			// Twice to make sure that reset works as expected
			repeat(2, [&]
			{
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
			});
		}
	}
}

TEST_CASE("GetCpUsingML_M4000")
{
	// https://bugs.farmanager.com/view.php?id=4000

	char c[32768];
	for (size_t i = 0; i != std::size(c); i += 2)
	{
		c[i + 0] = 0x00;
		c[i + 1] = 0xE0;
	}

	uintptr_t Cp;
	GetCpUsingML({ c, std::size(c) }, Cp, [](uintptr_t){ return true; });

	SUCCEED();
}
#endif
