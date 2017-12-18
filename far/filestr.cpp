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

#include "headers.hpp"
#pragma hdrstop

#include "filestr.hpp"
#include "nsUniversalDetectorEx.hpp"
#include "config.hpp"
#include "codepage_selection.hpp"
#include "strmix.hpp"

static const size_t DELTA = 1024;
static const size_t ReadBufCount = 0x2000;

bool IsTextUTF8(const char* Buffer, size_t Length, bool& PureAscii)
{
	bool Ascii=true;
	size_t Octets=0;
	size_t LastOctetsPos = 0;
	const size_t MaxCharSize = 4;

	for (size_t i=0; i<Length; i++)
	{
		BYTE c=Buffer[i];

		if (c&0x80)
			Ascii=false;

		if (Octets)
		{
			if ((c&0xC0)!=0x80)
				return false;

			Octets--;
		}
		else
		{
			LastOctetsPos = i;

			if (c&0x80)
			{
				while (c&0x80)
				{
					c <<= 1;
					Octets++;
				}

				Octets--;

				if (!Octets)
					return false;
			}
		}
	}

	PureAscii = Ascii;
	return (!Octets || Length - LastOctetsPos < MaxCharSize) && !Ascii;
}

GetFileString::GetFileString(const os::fs::file& SrcFile, uintptr_t CodePage) :
	SrcFile(SrcFile),
	m_CodePage(CodePage),
	m_Eol(m_CodePage)
{
	if (m_CodePage == CP_UNICODE || m_CodePage == CP_REVERSEBOM)
		m_wReadBuf.resize(ReadBufCount);
	else
		m_ReadBuf.resize(ReadBufCount);

	m_wStr.reserve(DELTA);
}

bool GetFileString::PeekString(string_view& Str, eol::type* Eol)
{
	if(!Peek)
	{
		LastResult = GetString(LastStr);
		Peek = true;
	}

	Str = LastStr;
	return LastResult;
}

bool GetFileString::GetString(string& Str, eol::type* Eol)
{
	string_view Tmp;
	if (!GetString(Tmp))
		return false;

	Str.assign(ALL_CONST_RANGE(Tmp));
	return true;
}

bool GetFileString::GetString(string_view& Str, eol::type* Eol)
{
	if(Peek)
	{
		Peek = false;
		Str = LastStr;
		return LastResult;
	}

	switch (m_CodePage)
	{
	case CP_UNICODE:
	case CP_REVERSEBOM:
		if (!GetTString(m_wReadBuf, m_wStr, Eol, m_CodePage == CP_REVERSEBOM))
			return false;

		Str = { m_wStr.data(), m_wStr.size() };
		return true;

	case CP_UTF8:
	case CP_UTF7:
		{
			std::vector<char> CharStr;
			CharStr.reserve(DELTA);
			if (!GetTString(m_ReadBuf, CharStr, Eol))
				return false;

			if (!CharStr.empty())
			{
				Utf::errors errs;
				const auto len = Utf::get_chars(m_CodePage, CharStr.data(), CharStr.size(), m_wStr.data(), m_wStr.size(), &errs);

				SomeDataLost = SomeDataLost || errs.Conversion.Error;
				if (len > m_wStr.size())
				{
					resize_nomove(m_wStr, len);
					Utf::get_chars(m_CodePage, CharStr.data(), CharStr.size(), m_wStr.data(), m_wStr.size(), nullptr);
				}
				else
				{
					m_wStr.resize(len);
				}
			}
			else
			{
				m_wStr.clear();
			}

			Str = { m_wStr.data(), m_wStr.size() };
			return true;
		}

	default:
		{
			std::vector<char> CharStr;
			CharStr.reserve(DELTA);
			if (!GetTString(m_ReadBuf, CharStr, Eol))
				return false;

			if (!CharStr.empty())
			{
				DWORD Result = ERROR_SUCCESS;
				bool bGet = false;
				m_wStr.resize(CharStr.size());

				size_t nResultLength = 0;
				if (!SomeDataLost)
				{
					nResultLength = MultiByteToWideChar(m_CodePage, SomeDataLost? 0 : MB_ERR_INVALID_CHARS, CharStr.data(), static_cast<int>(CharStr.size()), m_wStr.data(), static_cast<int>(m_wStr.size()));

					if (!nResultLength)
					{
						Result = GetLastError();
						if (Result == ERROR_NO_UNICODE_TRANSLATION || (Result == ERROR_INVALID_FLAGS && IsNoFlagsCodepage(m_CodePage)))
						{
							SomeDataLost = true;
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
					nResultLength = encoding::get_chars(m_CodePage, CharStr.data(), CharStr.size(), m_wStr);
					if (nResultLength > m_wStr.size())
					{
						Result = ERROR_INSUFFICIENT_BUFFER;
					}
				}

				if (Result == ERROR_INSUFFICIENT_BUFFER)
				{
					nResultLength = encoding::get_chars_count(m_CodePage, CharStr.data(), CharStr.size());
					m_wStr.resize(nResultLength);
					encoding::get_chars(m_CodePage, CharStr.data(), CharStr.size(), m_wStr);
				}

				if (!nResultLength)
					return false;

				m_wStr.resize(nResultLength);
			}
			else
			{
				m_wStr.clear();
			}

			Str = { m_wStr.data(), m_wStr.size() };
			return true;
		}
	}
}

template<typename T>
bool GetFileString::GetTString(std::vector<T>& From, std::vector<T>& To, eol::type* Eol, bool bBigEndian)
{
	To.clear();

	// Обработка ситуации, когда у нас пришёл двойной \r\r, а потом не было \n.
	// В этом случаем считаем \r\r двумя MAC окончаниями строк.
	if (bCrCr)
	{
		bCrCr = false;
		if (Eol)
			*Eol = eol::mac;
		return true;
	}

	auto CurrentEol = eol::none;
	for (const auto* ReadBufPtr = ReadPos < ReadSize? From.data() + ReadPos / sizeof(T) : nullptr; ; ++ReadBufPtr, ReadPos += sizeof(T))
	{
		if (ReadPos >= ReadSize)
		{
			if (!(SrcFile.Read(From.data(), ReadBufCount * sizeof(T), ReadSize) && ReadSize))
			{
				if (Eol)
					*Eol = CurrentEol;
				return !To.empty() || CurrentEol != eol::none;
			}

			if (bBigEndian && sizeof(T) != 1)
			{
				swap_bytes(From.data(), From.data(), ReadSize);
			}

			ReadPos = 0;
			ReadBufPtr = From.data();
		}

		if (!CurrentEol)
		{
			// UNIX
			if (*ReadBufPtr == m_Eol.lf<T>())
			{
				CurrentEol = eol::unix;
				continue;
			}
			// MAC / Windows? / Notepad?
			else if (*ReadBufPtr == m_Eol.cr<T>())
			{
				CurrentEol = eol::mac;
				continue;
			}
		}
		else if (CurrentEol == eol::mac)
		{
			if (m_CrSeen)
			{
				m_CrSeen = false;

				// Notepad
				if (*ReadBufPtr == m_Eol.lf<T>())
				{
					CurrentEol = eol::bad_win;
					continue;
				}
				else
				{
					// Пришёл \r\r, а \n не пришёл, поэтому считаем \r\r двумя MAC окончаниями строк
					bCrCr = true;
					break;
				}
			}
			else
			{
				// Windows
				if (*ReadBufPtr == m_Eol.lf<T>())
				{
					CurrentEol = eol::win;
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

		To.emplace_back(*ReadBufPtr);
		CurrentEol = eol::none;
	}

	if (Eol)
		*Eol = CurrentEol;
	return true;
}

bool GetFileFormat(const os::fs::file& file, uintptr_t& nCodePage, bool* pSignatureFound, bool bUseHeuristics, bool* pPureAscii)
{
	DWORD dwTemp = 0;
	bool bSignatureFound = false;
	bool bDetect = false;
	bool bPureAscii = false;

	size_t Readed = 0;
	if (file.Read(&dwTemp, sizeof(dwTemp), Readed) && Readed > 1 ) // minimum signature size is 2 bytes
	{
		if (LOWORD(dwTemp) == SIGN_UNICODE)
		{
			nCodePage = CP_UNICODE;
			file.SetPointer(2, nullptr, FILE_BEGIN);
			bSignatureFound = true;
		}
		else if (LOWORD(dwTemp) == SIGN_REVERSEBOM)
		{
			nCodePage = CP_REVERSEBOM;
			file.SetPointer(2, nullptr, FILE_BEGIN);
			bSignatureFound = true;
		}
		else if ((dwTemp & 0x00FFFFFF) == SIGN_UTF8)
		{
			nCodePage = CP_UTF8;
			file.SetPointer(3, nullptr, FILE_BEGIN);
			bSignatureFound = true;
		}
		else
		{
			file.SetPointer(0, nullptr, FILE_BEGIN);
		}
	}

	if (bSignatureFound)
	{
		bDetect = true;
	}
	else if (bUseHeuristics)
	{
		file.SetPointer(0, nullptr, FILE_BEGIN);
		size_t Size = 0x8000; // BUGBUG. TODO: configurable
		char_ptr Buffer(Size);
		size_t ReadSize = 0;
		bool ReadResult = file.Read(Buffer.get(), Size, ReadSize);
		file.SetPointer(0, nullptr, FILE_BEGIN);

		bPureAscii = ReadResult && !ReadSize; // empty file == pure ascii

		if (ReadResult && ReadSize)
		{
			// BUGBUG MSDN documents IS_TEXT_UNICODE_BUFFER_TOO_SMALL but there is no such thing
			if (ReadSize > 1)
			{
				int test = IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_REVERSE_MASK | IS_TEXT_UNICODE_NOT_UNICODE_MASK | IS_TEXT_UNICODE_NOT_ASCII_MASK;

				IsTextUnicode(Buffer.get(), static_cast<int>(ReadSize), &test); // return value is ignored, it's ok.

				if (!(test & IS_TEXT_UNICODE_NOT_UNICODE_MASK) && (test & IS_TEXT_UNICODE_NOT_ASCII_MASK))
				{
					if (test & IS_TEXT_UNICODE_UNICODE_MASK)
					{
						nCodePage = CP_UNICODE;
						bDetect = true;
					}
					else if (test & IS_TEXT_UNICODE_REVERSE_MASK)
					{
						nCodePage = CP_REVERSEBOM;
						bDetect = true;
					}
				}

				if (!bDetect && IsTextUTF8(Buffer.get(), ReadSize, bPureAscii))
				{
					nCodePage = CP_UTF8;
					bDetect = true;
				}
			}

			if (!bDetect && !bPureAscii)
			{
				int cp = GetCpUsingUniversalDetector(Buffer.get(), ReadSize);
				// This whole block shouldn't be here
				if ( cp >= 0 )
				{
					if (Global->Opt->strNoAutoDetectCP.Get() == L"-1")
					{
						if ( Global->Opt->CPMenuMode )
						{
							if ( static_cast<UINT>(cp) != GetACP() && static_cast<UINT>(cp) != GetOEMCP() )
							{
								long long selectType = Codepages().GetFavorite(cp);
								if (0 == (selectType & CPST_FAVORITE))
									cp = -1;
							}
						}
					}
					else
					{
						const auto BannedCpList = split<std::vector<string>>(Global->Opt->strNoAutoDetectCP, STLF_UNIQUE);

						if (contains(BannedCpList, str(cp)))
						{
							cp = -1;
						}
					}
				}

				if (cp != -1)
				{
					nCodePage = cp;
					bDetect = true;
				}
			}
		}
	}

	if (pSignatureFound)
		*pSignatureFound = bSignatureFound;

	if (pPureAscii)
		*pPureAscii = bPureAscii;

	return bDetect;
}
