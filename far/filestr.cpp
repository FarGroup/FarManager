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

OldGetFileString::OldGetFileString(FILE *SrcFile):
	SrcFile(SrcFile),
	wReadBuf(new wchar_t[ReadBufCount]),
	ReadBuf(new char[ReadBufCount]),
	m_nwStrLength(DELTA),
	m_nStrLength(DELTA),
	wStr(static_cast<wchar_t*>(xf_malloc(m_nwStrLength * sizeof(wchar_t)))),
	Str(static_cast<char*>(xf_malloc(m_nStrLength))),
	ReadPos(0),
	ReadSize(0),
	SomeDataLost(false),
	bCrCr(false)
{
}

OldGetFileString::~OldGetFileString()
{
	xf_free(wStr);
	xf_free(Str);
	delete[] wReadBuf;
	delete[] ReadBuf;
}


int OldGetFileString::GetString(wchar_t **DestStr, int nCodePage, int &Length)
{
	int nExitCode;

	if (nCodePage == CP_UNICODE) //utf-16
		nExitCode = GetUnicodeString(DestStr, Length, false);
	else if (nCodePage == CP_REVERSEBOM)
		nExitCode = GetUnicodeString(DestStr, Length, true);
	else
	{
		char *Str;
		nExitCode = GetAnsiString(&Str, Length);

		if (nExitCode == 1)
		{
			DWORD ret = ERROR_SUCCESS;
			int nResultLength = 0;
			bool bGet = false;
			*wStr = L'\0';

			if (!SomeDataLost)
			{
				// при CP_UTF7 dwFlags должен быть 0, см. MSDN
				nResultLength = MultiByteToWideChar(
				                    nCodePage,
				                    (SomeDataLost || nCodePage==CP_UTF7) ? 0 : MB_ERR_INVALID_CHARS,
				                    Str,
				                    Length,
				                    wStr,
				                    m_nwStrLength - 1
				                );

				if (!nResultLength)
				{
					ret = GetLastError();

					if (ERROR_NO_UNICODE_TRANSLATION == ret)
					{
						SomeDataLost = true;
						bGet = true;
					}
				}
			}
			else
				bGet = true;

			if (bGet)
			{
				nResultLength = MultiByteToWideChar(nCodePage, 0, Str, Length, wStr, m_nwStrLength - 1);

				if (!nResultLength)
					ret = GetLastError();
			}

			if (ERROR_INSUFFICIENT_BUFFER == ret)
			{
				nResultLength = MultiByteToWideChar(nCodePage, 0, Str, Length, nullptr, 0);
				wStr = (wchar_t*)xf_realloc_nomove(wStr, (nResultLength + 1) * sizeof(wchar_t));
				*wStr = L'\0';
				m_nwStrLength = nResultLength+1;
				nResultLength = MultiByteToWideChar(nCodePage, 0, Str, Length, wStr, nResultLength);
			}

			if (nResultLength)
				wStr[nResultLength] = L'\0';

			Length = nResultLength;
			*DestStr = wStr;
		}
	}

	return nExitCode;
}

int OldGetFileString::GetAnsiString(char **DestStr, int &Length)
{
	int CurLength = 0;
	int ExitCode = 1;
	char *ReadBufPtr = ReadPos < ReadSize ? ReadBuf + ReadPos : nullptr;

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
		while (1)
		{
			if (ReadPos >= ReadSize)
			{
				if (!(ReadSize = (int)fread(ReadBuf, 1, ReadBufCount, SrcFile)))
				{
					if (!CurLength)
						ExitCode=0;

					break;
				}

				ReadPos = 0;
				ReadBufPtr = ReadBuf;
			}

			if (Eol == FEOL_NONE)
			{
				// UNIX
				if (*ReadBufPtr == '\n')
					Eol = FEOL_UNIX;
				// MAC / Windows? / Notepad?
				else if (*ReadBufPtr == '\r')
					Eol = FEOL_MAC;
			}
			else if (Eol == FEOL_MAC)
			{
				// Windows
				if (*ReadBufPtr == '\n')
					Eol = FEOL_WINDOWS;
				// Notepad?
				else if (*ReadBufPtr == '\r')
					Eol = FEOL_MAC2;
				else
					break;
			}
			else if (Eol == FEOL_WINDOWS || Eol == FEOL_UNIX)
				break;
			else if (Eol == FEOL_MAC2)
			{
				// Notepad
				if (*ReadBufPtr == '\n')
					Eol = FEOL_NOTEPAD;
				else
				{
					// Пришёл \r\r, а \n не пришёл, поэтому считаем \r\r двумя MAC окончаниями строк
					--CurLength;
					bCrCr = true;
					break;
				}
			}
			else
				break;

			ReadPos++;

			if (CurLength >= m_nStrLength - 1)
			{
				char *NewStr = (char *)xf_realloc(Str, m_nStrLength + (DELTA << x));

				if (!NewStr)
					return (-1);

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
	return (ExitCode);
}

int OldGetFileString::GetUnicodeString(wchar_t **DestStr, int &Length, bool bBigEndian)
{
	int CurLength = 0;
	int ExitCode = 1;
	wchar_t *ReadBufPtr = ReadPos < ReadSize ? wReadBuf + ReadPos / sizeof(wchar_t) : nullptr;

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
		while (1)
		{
			if (ReadPos >= ReadSize)
			{
				if (!(ReadSize = (int)fread(wReadBuf, sizeof(wchar_t), ReadBufCount, SrcFile)))
				{
					if (!CurLength)
						ExitCode=0;

					break;
				}
				ReadSize *= sizeof(wchar_t);

				if (bBigEndian)
					_swab(reinterpret_cast<char*>(wReadBuf), reinterpret_cast<char*>(wReadBuf), ReadSize);

				ReadPos = 0;
				ReadBufPtr = wReadBuf;
			}

			if (Eol == FEOL_NONE)
			{
				// UNIX
				if (*ReadBufPtr == L'\n')
					Eol = FEOL_UNIX;
				// MAC / Windows? / Notepad?
				else if (*ReadBufPtr == L'\r')
					Eol = FEOL_MAC;
			}
			else if (Eol == FEOL_MAC)
			{
				// Windows
				if (*ReadBufPtr == L'\n')
					Eol = FEOL_WINDOWS;
				// Notepad?
				else if (*ReadBufPtr == L'\r')
					Eol = FEOL_MAC2;
				else
					break;
			}
			else if (Eol == FEOL_WINDOWS || Eol == FEOL_UNIX)
				break;
			else if (Eol == FEOL_MAC2)
			{
				// Notepad
				if (*ReadBufPtr == L'\n')
					Eol = FEOL_NOTEPAD;
				else
				{
					// Пришёл \r\r, а \n не пришёл, поэтому считаем \r\r двумя MAC окончаниями строк
					--CurLength;
					bCrCr = true;
					break;
				}
			}
			else
				break;

			ReadPos += sizeof(wchar_t);

			if (CurLength >= m_nwStrLength - 1)
			{
				wchar_t *NewStr = (wchar_t *)xf_realloc(wStr, (m_nwStrLength + (DELTA << x)) * sizeof(wchar_t));

				if (!NewStr)
					return (-1);

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
	return (ExitCode);
}

bool IsTextUTF8(const LPBYTE Buffer,size_t Length)
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

bool OldGetFileFormat(FILE *file, UINT &nCodePage, bool *pSignatureFound, bool bUseHeuristics)
{
	DWORD dwTemp=0;
	bool bSignatureFound = false;
	bool bDetect=false;

	if (fread(&dwTemp, 1, 4, file))
	{
		if (LOWORD(dwTemp) == SIGN_UNICODE)
		{
			nCodePage = CP_UNICODE;
			fseek(file, 2, SEEK_SET);
			bSignatureFound = true;
		}
		else if (LOWORD(dwTemp) == SIGN_REVERSEBOM)
		{
			nCodePage = CP_REVERSEBOM;
			fseek(file, 2, SEEK_SET);
			bSignatureFound = true;
		}
		else if ((dwTemp & 0x00FFFFFF) == SIGN_UTF8)
		{
			nCodePage = CP_UTF8;
			fseek(file, 3, SEEK_SET);
			bSignatureFound = true;
		}
		else
			fseek(file, 0, SEEK_SET);
	}

	if (bSignatureFound)
	{
		bDetect = true;
	}
	else if (bUseHeuristics)
	{
		fseek(file, 0, SEEK_SET);
		size_t sz=0x8000; // BUGBUG. TODO: configurable
		LPVOID Buffer=xf_malloc(sz);
		sz=fread(Buffer,1,sz,file);
		fseek(file,0,SEEK_SET);

		if (sz)
		{
			int test=
			    IS_TEXT_UNICODE_STATISTICS|
			    IS_TEXT_UNICODE_REVERSE_STATISTICS|
			    IS_TEXT_UNICODE_CONTROLS|
			    IS_TEXT_UNICODE_REVERSE_CONTROLS|
			    IS_TEXT_UNICODE_ILLEGAL_CHARS|
			    IS_TEXT_UNICODE_ODD_LENGTH|
			    IS_TEXT_UNICODE_NULL_BYTES;

			if (IsTextUnicode(Buffer, (int)sz, &test))
			{
				if (!(test&IS_TEXT_UNICODE_ODD_LENGTH) && !(test&IS_TEXT_UNICODE_ILLEGAL_CHARS))
				{
					if ((test&IS_TEXT_UNICODE_NULL_BYTES) ||
					        (test&IS_TEXT_UNICODE_CONTROLS) ||
					        (test&IS_TEXT_UNICODE_REVERSE_CONTROLS))
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
			else if (IsTextUTF8((const LPBYTE)Buffer, sz))
			{
				nCodePage=CP_UTF8;
				bDetect=true;
			}
			else
			{
				nsUniversalDetectorEx *ns = new nsUniversalDetectorEx();
				ns->HandleData((const char*)Buffer,(PRUint32)sz);
				ns->DataEnd();
				int cp = ns->getCodePage();

				if (cp != -1)
				{
					nCodePage = cp;
					bDetect = true;
				}

				delete ns;
			}
		}

		xf_free(Buffer);
	}

	if (pSignatureFound)
		*pSignatureFound = bSignatureFound;

	return bDetect;
}

wchar_t *ReadString(FILE *file, wchar_t *lpwszDest, int nDestLength, int nCodePage)
{
	char *lpDest = (char*)xf_malloc((nDestLength+1)*3);  //UTF-8, up to 3 bytes per char support
	memset(lpDest, 0, (nDestLength+1)*3);
	memset(lpwszDest, 0, nDestLength*sizeof(wchar_t));

	if ((nCodePage == CP_UNICODE) || (nCodePage == CP_REVERSEBOM))
	{
		if (!fgetws(lpwszDest, nDestLength, file))
		{
			xf_free(lpDest);
			return nullptr;
		}

		if (nCodePage == CP_REVERSEBOM)
		{
			_swab((char*)lpwszDest, (char*)lpwszDest, nDestLength*sizeof(wchar_t));
			wchar_t *Ch = lpwszDest;
			int nLength = Min(static_cast<int>(wcslen(lpwszDest)), nDestLength);

			while (*Ch)
			{
				if (*Ch == L'\n')
				{
					*(Ch+1) = 0;
					break;
				}

				Ch++;
			}

			int nNewLength = Min(static_cast<int>(wcslen(lpwszDest)), nDestLength);
			fseek(file, (nNewLength-nLength)*sizeof(wchar_t), SEEK_CUR);
		}
	}
	else if (nCodePage == CP_UTF8)
	{
		if (fgets(lpDest, nDestLength*3, file))
			MultiByteToWideChar(CP_UTF8, 0, lpDest, -1, lpwszDest, nDestLength);
		else
		{
			xf_free(lpDest);
			return nullptr;
		}
	}
	else if (nCodePage != -1)
	{
		if (fgets(lpDest, nDestLength, file))
			MultiByteToWideChar(nCodePage, 0, lpDest, -1, lpwszDest, nDestLength);
		else
		{
			xf_free(lpDest);
			return nullptr;
		}
	}

	xf_free(lpDest);
	return lpwszDest;
}


//-----------------------------------------------------------------------------

GetFileString::GetFileString(File& SrcFile):
	SrcFile(SrcFile),
	ReadPos(0),
	ReadSize(0),
	Peek(false),
	LastLength(0),
	LastString(nullptr),
	LastResult(0),
	ReadBuf(new char[ReadBufCount]),
	wReadBuf(new wchar_t[ReadBufCount]),
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
	delete[] wReadBuf;
	delete[] ReadBuf;
}

int GetFileString::PeekString(LPWSTR* DestStr, UINT nCodePage, int& Length)
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

int GetFileString::GetString(LPWSTR* DestStr, UINT nCodePage, int& Length)
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
	LPSTR ReadBufPtr = ReadPos < ReadSize ? ReadBuf + ReadPos : nullptr;

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
				if (!(SrcFile.Read(ReadBuf, ReadBufCount, ReadSize) && ReadSize))
				{
					if (!CurLength)
					{
						ExitCode=0;
					}
					break;
				}
				ReadPos = 0;
				ReadBufPtr = ReadBuf;
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
	LPWSTR ReadBufPtr = ReadPos < ReadSize ? wReadBuf + ReadPos / sizeof(wchar_t) : nullptr;

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
				if (!(SrcFile.Read(wReadBuf, ReadBufCount*sizeof(wchar_t), ReadSize) && ReadSize))
				{
					if (!CurLength)
					{
						ExitCode = 0;
					}
					break;
				}

				if (bBigEndian)
				{
					_swab(reinterpret_cast<char*>(wReadBuf), reinterpret_cast<char*>(wReadBuf), ReadSize);
				}
				ReadPos = 0;
				ReadBufPtr = wReadBuf;
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

bool GetFileFormat(File& file, UINT& nCodePage, bool* pSignatureFound, bool bUseHeuristics)
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
		LPVOID Buffer=xf_malloc(Size);
		DWORD ReadSize = 0;
		bool ReadResult = file.Read(Buffer, Size, ReadSize);
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

			if (IsTextUnicode(Buffer, ReadSize, &test))
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
			else if (IsTextUTF8(static_cast<LPBYTE>(Buffer), ReadSize))
			{
				nCodePage=CP_UTF8;
				bDetect=true;
			}
			else
			{
				nsUniversalDetectorEx *ns = new nsUniversalDetectorEx();
				ns->HandleData(static_cast<LPCSTR>(Buffer), ReadSize);
				ns->DataEnd();
				int cp = ns->getCodePage();
				if ( cp >= 0 )
				{
					const wchar_t *deprecated = Opt.strNoAutoDetectCP;

					if ( 0 == wcscmp(deprecated, L"-1") )
					{
						if ( Opt.CPMenuMode )
						{
							if ( static_cast<UINT>(cp) != GetACP() && static_cast<UINT>(cp) != GetOEMCP() )
							{
								int selectType = 0;
								GeneralCfg->GetValue(FavoriteCodePagesKey, FormatString() << cp, &selectType, 0);
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

				delete ns;
			}
		}

		xf_free(Buffer);
	}

	if (pSignatureFound)
	{
		*pSignatureFound = bSignatureFound;
	}
	return bDetect;
}
