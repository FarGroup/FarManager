#include "FarPluginBase.hpp"
#include "SystemApi.hpp"
#include "FarApi.hpp"
#include "FarLng2.hpp"

#define SIGN_UNICODE    0x0000FEFF
#define SIGN_REVERSEBOM 0x0000FFFE
#define SIGN_UTF8       0x00BFBBEF

Language::Language()
{
	m_strLanguage = _T("English");
}

const TCHAR* Language::GetLanguage()
{
	return m_strLanguage;
}

const TCHAR* Language::GetMsg(unsigned int uID)
{
	if ( uID < strings.count() )
		return strings[uID];

	return nullptr;
}

static int GetCodePage(HANDLE hFile, unsigned int& dwOffset)
{
	DWORD dwBuffer = 0;
	DWORD dwRead;

	int nResult = CP_OEMCP;
	dwOffset = 0;

	__int64 nOldPos = 0;

	if ( apiSetFilePointer(hFile, 0, &nOldPos, FILE_CURRENT) )
	{
		if ( ReadFile(hFile, &dwBuffer, 4, &dwRead, NULL) && (dwRead > 3) )
		{
			if ( LOWORD(dwBuffer) == SIGN_UNICODE )
			{
				nResult = CP_UNICODE;
				dwOffset = 4;
			}
			else 
			{
				if ( LOWORD(dwBuffer) == SIGN_REVERSEBOM )
				{
					nResult = CP_REVERSEBOM;
					dwOffset = 4;
				}
				else
				{
					if ( (dwBuffer & 0x00FFFFFF) == SIGN_UTF8 )
					{
						nResult = CP_UTF8;
						dwOffset = 3;
					}
				}
			}
		}

		apiSetFilePointer(hFile, nOldPos, nullptr, FILE_BEGIN);
	}

	return nResult;
}

void Language::AddString(const TCHAR* lpStr)
{
	strings.add(StrDuplicate(lpStr));
}

void Language::ParseString(const TCHAR* lpStr)
{
	string strTemp = lpStr;

	farTrim(strTemp); //FAR ROUTINE, BAD!!!

	if ( strTemp.GetLength() > 1 )
	{
		if ( strTemp.At(0) == _T('.') ) //param
		{
			const TCHAR* lpTemp = strTemp;
			const TCHAR* lpEqual = _tcschr(strTemp, _T('='));

			if ( lpEqual != nullptr )
			{
				string strName;
				string strValue = lpEqual+1;

				strName.SetData(lpTemp, lpEqual-lpTemp);

				if ( strName == _T(".Language") ) //ignore case??
				{
					const TCHAR* lpComma = _tcschr(lpEqual+1, _T(','));

					if ( lpComma != nullptr )
						m_strLanguage.SetData(lpEqual+1, lpComma-lpEqual-1); //mad
					else
						m_strLanguage = strValue;
				}
			}
		}

		if ( strTemp.At(0) == _T('\"') ) //string
		{
			farUnquote(strTemp);
			AddString(strTemp);
		}
	}
}

//buffer will be modified by strtok

bool Language::LoadFromBuffer(char* pBuffer, int nCodePage)
{
	strings.reset();

	if ( nCodePage == CP_UNICODE )
	{
		wchar_t* b = (wchar_t*)pBuffer;
		wchar_t* token = wcstok(b, L"\n\r");

		while ( token )
		{
			string str;

#ifdef UNICODE
			str = token;
#else
			str.SetData(token);
#endif
			ParseString(str);

			token = wcstok(nullptr, L"\n\r");
		}
	}
	else
	{
		char* b = pBuffer;
		char* token = strtok(b, "\n\r");

		while ( token )
		{
			string str;

#ifdef UNICODE
			wchar_t* lpUnicode = nullptr;

			if ( nCodePage == CP_UTF8 )
				lpUnicode = UTF8ToUnicode(token);
			else
				lpUnicode = AnsiToUnicode(token, nCodePage);

			str = lpUnicode;

			free(lpUnicode);
#else
			char* lpAnsi = nullptr;

			if ( nCodePage == CP_UTF8 )
				lpAnsi = UTF8ToAnsi(token, nCodePage);
			else
				lpAnsi = StrDuplicate(token);

			str = lpAnsi;

			free(lpAnsi);
#endif
			ParseString(str);

			token = strtok(nullptr, "\n\r");
		}
	}

	return strings.count() > 0;
}

bool Language::LoadFromFile(const TCHAR* lpFileName)
{
	bool bResult = false;

	HANDLE hFile = CreateFile(
			lpFileName,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL
			);

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		unsigned int uOffset = 0;
		int nCodePage = GetCodePage(hFile, uOffset);

		unsigned __int64 uFileSize = 0;
		
		if ( apiGetFileSize(hFile, &uFileSize) )
		{
			char* pBuffer = new char[uFileSize+1];

			DWORD dwRead;

			if ( ReadFile(hFile, pBuffer, uFileSize, &dwRead, nullptr) )
			{
				pBuffer[dwRead] = 0;
				bResult = LoadFromBuffer(pBuffer+uOffset, nCodePage);
			}

			delete [] pBuffer;
		}

		CloseHandle(hFile);
	}

	return bResult;
}
