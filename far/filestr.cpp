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

// Self:
#include "filestr.hpp"

// Internal:
#include "nsUniversalDetectorEx.hpp"
#include "config.hpp"
#include "codepage_selection.hpp"
#include "global.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/algorithm.hpp"
#include "common/enum_tokens.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static const size_t DELTA = 1024;
static const size_t ReadBufCount = 65536;

enum_file_lines::enum_file_lines(const os::fs::file& SrcFile, uintptr_t CodePage) :
	SrcFile(SrcFile),
	BeginPos(SrcFile.GetPointer()),
	m_CodePage(CodePage),
	m_Eol(m_CodePage)
{
	if (m_CodePage == CP_UNICODE || m_CodePage == CP_REVERSEBOM)
		m_wReadBuf.resize(ReadBufCount);
	else
		m_ReadBuf.resize(ReadBufCount);

	m_wStr.reserve(DELTA);
}

bool enum_file_lines::get(bool Reset, file_line& Value) const
{
	if (Reset)
	{
		SrcFile.SetPointer(BeginPos, nullptr, FILE_BEGIN);
	}

	return GetString(Value.Str, Value.Eol);
}

bool enum_file_lines::GetString(string_view& Str, eol::type& Eol) const
{
	switch (m_CodePage)
	{
	case CP_UNICODE:
	case CP_REVERSEBOM:
		if (!GetTString(m_wReadBuf, m_wStr, Eol, m_CodePage == CP_REVERSEBOM))
			return false;

		Str = m_wStr;
		return true;

	case CP_UTF8:
	case CP_UTF7:
		{
			std::string CharStr;
			CharStr.reserve(DELTA);
			if (!GetTString(m_ReadBuf, CharStr, Eol))
				return false;

			if (!CharStr.empty())
			{
				Utf::errors Errors;
				m_wStr.resize(m_wStr.capacity());

				for (auto Overflow = true; Overflow;)
				{
					const auto Size = Utf::get_chars(m_CodePage, CharStr, m_wStr.data(), m_wStr.size(), &Errors);
					Overflow = Size > m_wStr.size();
					m_wStr.resize(Size);
				}

				m_ConversionError = m_ConversionError || Errors.Conversion.Error;
			}
			else
			{
				m_wStr.clear();
			}

			Str = m_wStr;
			return true;
		}

	default:
		{
			std::string CharStr;
			CharStr.reserve(DELTA);
			if (!GetTString(m_ReadBuf, CharStr, Eol))
				return false;

			if (!CharStr.empty())
			{
				DWORD Result = ERROR_SUCCESS;
				bool bGet = false;
				m_wStr.resize(CharStr.size());

				size_t nResultLength = 0;
				if (!m_ConversionError)
				{
					nResultLength = MultiByteToWideChar(m_CodePage, MB_ERR_INVALID_CHARS, CharStr.data(), static_cast<int>(CharStr.size()), m_wStr.data(), static_cast<int>(m_wStr.size()));

					if (!nResultLength)
					{
						Result = GetLastError();
						if (Result == ERROR_NO_UNICODE_TRANSLATION || (Result == ERROR_INVALID_FLAGS && IsNoFlagsCodepage(m_CodePage)))
						{
							m_ConversionError = true;
							bGet = true;
						}
					}
				}
				else
				{
					bGet = true;
				}

				if (bGet)
				{
					nResultLength = encoding::get_chars(m_CodePage, CharStr, m_wStr);
					if (nResultLength > m_wStr.size())
					{
						Result = ERROR_INSUFFICIENT_BUFFER;
					}
				}

				if (Result == ERROR_INSUFFICIENT_BUFFER)
				{
					nResultLength = encoding::get_chars_count(m_CodePage, CharStr);
					m_wStr.resize(nResultLength);
					encoding::get_chars(m_CodePage, CharStr, m_wStr);
				}

				if (!nResultLength)
					return false;

				m_wStr.resize(nResultLength);
			}
			else
			{
				m_wStr.clear();
			}

			Str = m_wStr;
			return true;
		}
	}
}

template<typename T>
bool enum_file_lines::GetTString(std::vector<T>& From, std::basic_string<T>& To, eol::type& Eol, bool bBigEndian) const
{
	To.clear();

	// Обработка ситуации, когда у нас пришёл двойной \r\r, а потом не было \n.
	// В этом случаем считаем \r\r двумя MAC окончаниями строк.
	if (m_CrCr)
	{
		m_CrCr = false;
		Eol = eol::type::mac;
		return true;
	}

	auto CurrentEol = eol::type::none;
	for (const auto* ReadBufPtr = ReadPos < ReadSize? From.data() + ReadPos / sizeof(T) : nullptr; ; ++ReadBufPtr, ReadPos += sizeof(T))
	{
		if (ReadPos >= ReadSize)
		{
			if (!(SrcFile.Read(From.data(), ReadBufCount * sizeof(T), ReadSize) && ReadSize))
			{
				Eol = CurrentEol;
				return !To.empty() || CurrentEol != eol::type::none;
			}

			if (bBigEndian && sizeof(T) != 1)
			{
				swap_bytes(From.data(), From.data(), ReadSize);
			}

			ReadPos = 0;
			ReadBufPtr = From.data();
		}

		if (CurrentEol == eol::type::none)
		{
			// UNIX
			if (*ReadBufPtr == m_Eol.lf<T>())
			{
				CurrentEol = eol::type::unix;
				continue;
			}
			// MAC / Windows? / Notepad?
			else if (*ReadBufPtr == m_Eol.cr<T>())
			{
				CurrentEol = eol::type::mac;
				continue;
			}
		}
		else if (CurrentEol == eol::type::mac)
		{
			if (m_CrSeen)
			{
				m_CrSeen = false;

				// Notepad
				if (*ReadBufPtr == m_Eol.lf<T>())
				{
					CurrentEol = eol::type::bad_win;
					continue;
				}
				else
				{
					// Пришёл \r\r, а \n не пришёл, поэтому считаем \r\r двумя MAC окончаниями строк
					m_CrCr = true;
					break;
				}
			}
			else
			{
				// Windows
				if (*ReadBufPtr == m_Eol.lf<T>())
				{
					CurrentEol = eol::type::win;
					continue;
				}
				// Notepad or two MACs?
				else if (*ReadBufPtr == m_Eol.cr<T>())
				{
					m_CrSeen = true;
					continue;
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			break;
		}

		To.push_back(*ReadBufPtr);
		CurrentEol = eol::type::none;
	}

	Eol = CurrentEol;
	return true;
}

// If the file contains a BOM this function will advance the file pointer by the BOM size (either 2 or 3)
static bool GetUnicodeCpUsingBOM(const os::fs::file& File, uintptr_t& Codepage)
{
	char Buffer[3]{};
	size_t BytesRead = 0;
	if (!File.Read(Buffer, std::size(Buffer), BytesRead))
		return false;

	std::string_view Signature(Buffer, std::size(Buffer));

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
	char_ptr Buffer(Size);
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
