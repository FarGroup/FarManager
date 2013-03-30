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
#include "configdb.hpp"
#include "codepage.hpp"

const size_t DELTA = 1024;
const size_t ReadBufCount = 0x2000;

enum EolType
{
	FEOL_NONE,
	// \r\n
	FEOL_WINDOWS,
	// \n
	FEOL_UNIX,
	// \r
	FEOL_MAC,
	// \r\r (это не реальное завершение строки, а состояние парсера)
	FEOL_MAC2,
	// \r\r\n (появление таких завершений строк вызвано багом Notepad-а)
	FEOL_NOTEPAD
};

bool IsTextUTF8(const char* Buffer,size_t Length)
{
	bool Ascii=true;
	size_t Octets=0;

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
			if (c&0x80)
			{
				while (c&0x80)
				{
					c<<=1;
					Octets++;
				}

				Octets--;

				if (!Octets)
					return false;
			}
		}
	}

	return (Octets>0||Ascii)?false:true;
}

GetFileString::GetFileString(File& SrcFile):
	SrcFile(SrcFile),
	ReadPos(0),
	ReadSize(0),
	Peek(false),
	LastLength(0),
	LastString(nullptr),
	LastResult(0),
	ReadBuf(ReadBufCount),
	wReadBuf(ReadBufCount),
	m_nStrLength(DELTA),
	Str(static_cast<LPSTR>(xf_malloc(m_nStrLength))),
	m_nwStrLength(DELTA),
	wStr(static_cast<LPWSTR>(xf_malloc(m_nwStrLength * sizeof(wchar_t)))),
	SomeDataLost(false),
	bCrCr(false)
{
}

GetFileString::~GetFileString()
{
	xf_free(wStr);
	xf_free(Str);
}

int GetFileString::PeekString(LPWSTR* DestStr, uintptr_t nCodePage, int& Length)
{
	if(!Peek)
	{
		LastResult = GetString(DestStr, nCodePage, Length);
		Peek = true;
		LastString = DestStr;
		LastLength = Length;
	}
	else
	{
		DestStr = LastString;
		Length = LastLength;
	}
	return LastResult;
}

int GetFileString::GetString(LPWSTR* DestStr, uintptr_t nCodePage, int& Length)
{
	if(Peek)
	{
		Peek = false;
		DestStr = LastString;
		Length = LastLength;
		return LastResult;
	}
	int nExitCode;

	if (nCodePage == CP_UNICODE) //utf-16
	{
		nExitCode = GetUnicodeString(DestStr, Length, false);
	}
	else if (nCodePage == CP_REVERSEBOM)
	{
		nExitCode = GetUnicodeString(DestStr, Length, true);
	}
	else
	{
		char *Str;
		nExitCode = GetAnsiString(&Str, Length);

		if (nExitCode == 1)
		{
			DWORD Result = ERROR_SUCCESS;
			int nResultLength = 0;
			bool bGet = false;
			*wStr = L'\0';

			if (!SomeDataLost)
			{
				// при CP_UTF7 dwFlags должен быть 0, см. MSDN
				nResultLength = MultiByteToWideChar(nCodePage, (SomeDataLost || nCodePage==CP_UTF7) ? 0 : MB_ERR_INVALID_CHARS, Str, Length, wStr, m_nwStrLength - 1);

				if (!nResultLength)
				{
					Result = GetLastError();
					if (Result == ERROR_NO_UNICODE_TRANSLATION)
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
				nResultLength = MultiByteToWideChar(nCodePage, 0, Str, Length, wStr, m_nwStrLength - 1);
				if (!nResultLength)
				{
					Result = GetLastError();
				}
			}
			if (Result == ERROR_INSUFFICIENT_BUFFER)
			{
				nResultLength = MultiByteToWideChar(nCodePage, 0, Str, Length, nullptr, 0);
				wStr = static_cast<LPWSTR>(xf_realloc_nomove(wStr, (nResultLength + 1) * sizeof(wchar_t)));
				*wStr = L'\0';
				m_nwStrLength = nResultLength+1;
				nResultLength = MultiByteToWideChar(nCodePage, 0, Str, Length, wStr, nResultLength);
			}
			if (nResultLength)
			{
				wStr[nResultLength] = L'\0';
			}
			Length = nResultLength;
			*DestStr = wStr;
		}
	}
	return nExitCode;
}

int GetFileString::GetAnsiString(LPSTR* DestStr, int& Length)
{
	int CurLength = 0;
	int ExitCode = 1;
	LPSTR ReadBufPtr = ReadPos < ReadSize ? ReadBuf.get() + ReadPos : nullptr;

	// Обработка ситуации, когда у нас пришёл двойной \r\r, а потом не было \n.
	// В этом случаем считаем \r\r двумя MAC окончаниями строк.
	if (bCrCr)
	{
		*Str = '\r';
		CurLength = 1;
		bCrCr = false;
	}
	else
	{
		EolType Eol = FEOL_NONE;
		int x = 0;
		for(;;)
		{
			if (ReadPos >= ReadSize)
			{
				if (!(SrcFile.Read(ReadBuf.get(), ReadBufCount, ReadSize) && ReadSize))
				{
					if (!CurLength)
					{
						ExitCode=0;
					}
					break;
				}
				ReadPos = 0;
				ReadBufPtr = ReadBuf.get();
			}
			if (Eol == FEOL_NONE)
			{
				// UNIX
				if (*ReadBufPtr == '\n')
				{
					Eol = FEOL_UNIX;
				}
				// MAC / Windows? / Notepad?
				else if (*ReadBufPtr == '\r')
				{
					Eol = FEOL_MAC;
				}
			}
			else if (Eol == FEOL_MAC)
			{
				// Windows
				if (*ReadBufPtr == '\n')
				{
					Eol = FEOL_WINDOWS;
				}
				// Notepad?
				else if (*ReadBufPtr == '\r')
				{
					Eol = FEOL_MAC2;
				}
				else
				{
					break;
				}
			}
			else if (Eol == FEOL_WINDOWS || Eol == FEOL_UNIX)
			{
				break;
			}
			else if (Eol == FEOL_MAC2)
			{
				// Notepad
				if (*ReadBufPtr == '\n')
				{
					Eol = FEOL_NOTEPAD;
				}
				else
				{
					// Пришёл \r\r, а \n не пришёл, поэтому считаем \r\r двумя MAC окончаниями строк
					CurLength--;
					bCrCr = true;
					break;
				}
			}
			else
			{
				break;
			}
			ReadPos++;
			if (CurLength >= m_nStrLength - 1)
			{
				LPSTR NewStr = static_cast<LPSTR>(xf_realloc(Str, m_nStrLength + (DELTA << x)));
				if (!NewStr)
				{
					return -1;
				}
				Str = NewStr;
				m_nStrLength += DELTA << x;
				x++;
			}
			Str[CurLength++] = *ReadBufPtr;
			ReadBufPtr++;
		}
	}

	Str[CurLength] = 0;
	*DestStr = Str;
	Length = CurLength;
	return ExitCode;
}

int GetFileString::GetUnicodeString(LPWSTR* DestStr, int& Length, bool bBigEndian)
{
	int CurLength = 0;
	int ExitCode = 1;
	LPWSTR ReadBufPtr = ReadPos < ReadSize ? wReadBuf.get() + ReadPos / sizeof(wchar_t) : nullptr;

	// Обработка ситуации, когда у нас пришёл двойной \r\r, а потом не было \n.
	// В этом случаем считаем \r\r двумя MAC окончаниями строк.
	if (bCrCr)
	{
		*wStr = L'\r';
		CurLength = 1;
		bCrCr = false;
	}
	else
	{
		EolType Eol = FEOL_NONE;
		int x = 0;
		for(;;)
		{
			if (ReadPos >= ReadSize)
			{
				if (!(SrcFile.Read(wReadBuf.get(), ReadBufCount*sizeof(wchar_t), ReadSize) && ReadSize))
				{
					if (!CurLength)
					{
						ExitCode = 0;
					}
					break;
				}

				if (bBigEndian)
				{
					_swab(reinterpret_cast<char*>(wReadBuf.get()), reinterpret_cast<char*>(wReadBuf.get()), ReadSize);
				}
				ReadPos = 0;
				ReadBufPtr = wReadBuf.get();
			}
			if (Eol == FEOL_NONE)
			{
				// UNIX
				if (*ReadBufPtr == L'\n')
				{
					Eol = FEOL_UNIX;
				}
				// MAC / Windows? / Notepad?
				else if (*ReadBufPtr == L'\r')
				{
					Eol = FEOL_MAC;
				}
			}
			else if (Eol == FEOL_MAC)
			{
				// Windows
				if (*ReadBufPtr == L'\n')
				{
					Eol = FEOL_WINDOWS;
				}
				// Notepad?
				else if (*ReadBufPtr == L'\r')
				{
					Eol = FEOL_MAC2;
				}
				else
				{
					break;
				}
			}
			else if (Eol == FEOL_WINDOWS || Eol == FEOL_UNIX)
			{
				break;
			}
			else if (Eol == FEOL_MAC2)
			{
				// Notepad
				if (*ReadBufPtr == L'\n')
				{
					Eol = FEOL_NOTEPAD;
				}
				else
				{
					// Пришёл \r\r, а \n не пришёл, поэтому считаем \r\r двумя MAC окончаниями строк
					CurLength--;
					bCrCr = true;
					break;
				}
			}
			else
			{
				break;
			}
			ReadPos += sizeof(wchar_t);
			if (CurLength >= m_nwStrLength - 1)
			{
				LPWSTR NewStr = static_cast<LPWSTR>(xf_realloc(wStr, (m_nwStrLength + (DELTA << x)) * sizeof(wchar_t)));
				if (!NewStr)
				{
					return -1;
				}
				wStr = NewStr;
				m_nwStrLength += DELTA << x;
				x++;
			}
			wStr[CurLength++] = *ReadBufPtr;
			ReadBufPtr++;
		}
	}
	wStr[CurLength] = 0;
	*DestStr = wStr;
	Length = CurLength;
	return ExitCode;
}

bool GetFileFormat(File& file, uintptr_t& nCodePage, bool* pSignatureFound, bool bUseHeuristics)
{
	DWORD dwTemp=0;
	bool bSignatureFound = false;
	bool bDetect=false;

	DWORD Readed = 0;
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
		DWORD Size=0x8000; // BUGBUG. TODO: configurable
		char_ptr Buffer(Size);
		DWORD ReadSize = 0;
		bool ReadResult = file.Read(Buffer.get(), Size, ReadSize);
		file.SetPointer(0, nullptr, FILE_BEGIN);

		if (ReadResult && ReadSize)
		{
			int test=
				IS_TEXT_UNICODE_STATISTICS|
				IS_TEXT_UNICODE_REVERSE_STATISTICS|
				IS_TEXT_UNICODE_CONTROLS|
				IS_TEXT_UNICODE_REVERSE_CONTROLS|
				IS_TEXT_UNICODE_ILLEGAL_CHARS|
				IS_TEXT_UNICODE_ODD_LENGTH|
				IS_TEXT_UNICODE_NULL_BYTES;

			if (IsTextUnicode(Buffer.get(), ReadSize, &test))
			{
				if (!(test&IS_TEXT_UNICODE_ODD_LENGTH) && !(test&IS_TEXT_UNICODE_ILLEGAL_CHARS))
				{
					if ((test&IS_TEXT_UNICODE_NULL_BYTES) || (test&IS_TEXT_UNICODE_CONTROLS) || (test&IS_TEXT_UNICODE_REVERSE_CONTROLS))
					{
						if ((test&IS_TEXT_UNICODE_CONTROLS) || (test&IS_TEXT_UNICODE_STATISTICS))
						{
							nCodePage=CP_UNICODE;
							bDetect=true;
						}
						else if ((test&IS_TEXT_UNICODE_REVERSE_CONTROLS) || (test&IS_TEXT_UNICODE_REVERSE_STATISTICS))
						{
							nCodePage=CP_REVERSEBOM;
							bDetect=true;
						}
					}
				}
			}
			else if (IsTextUTF8(Buffer.get(), ReadSize))
			{
				nCodePage=CP_UTF8;
				bDetect=true;
			}
			else
			{
				std::unique_ptr<nsUniversalDetectorEx> ns(new nsUniversalDetectorEx());
				ns->HandleData(Buffer.get(), ReadSize);
				ns->DataEnd();
				int cp = ns->getCodePage();
				if ( cp >= 0 )
				{
					const wchar_t *deprecated = Global->Opt->strNoAutoDetectCP;

					if ( 0 == wcscmp(deprecated, L"-1") )
					{
						if ( Global->Opt->CPMenuMode )
						{
							if ( static_cast<UINT>(cp) != GetACP() && static_cast<UINT>(cp) != GetOEMCP() )
							{
								int selectType = 0;
								Global->Db->GeneralCfg()->GetValue(FavoriteCodePagesKey, FormatString() << cp, &selectType, 0);
								if (0 == (selectType & CPST_FAVORITE))
									cp = -1;
							}
						}
					}
					else
					{
						while (*deprecated)
						{
							while (*deprecated && (*deprecated < L'0' || *deprecated > L'9'))
								++deprecated;

							int dp = (int)wcstol(deprecated, (wchar_t **)&deprecated, 0);
							if (cp == dp)
							{
								cp = -1;
								break;
							}
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
	{
		*pSignatureFound = bSignatureFound;
	}
	return bDetect;
}
