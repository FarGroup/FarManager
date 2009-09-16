/*
UnicodeString.hpp

Unicode строки
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

UnicodeStringData *eus()
{
	//дл€ оптимизации создани€ пустых UnicodeString
	static UnicodeStringData *EmptyUnicodeStringData = new UnicodeStringData(1,1);
	return EmptyUnicodeStringData;
}

UnicodeString::UnicodeString()
{
	m_pData = eus();
	m_pData->AddRef();
}

void UnicodeString::Inflate(size_t nSize)
{
	if ( m_pData->GetRef() == 1 )
	{
		m_pData->Inflate(nSize);
	}
	else
	{
		UnicodeStringData *pNewData = new UnicodeStringData(nSize);
		size_t nNewLength = Min(m_pData->GetLength(),nSize-1);

		wmemcpy(pNewData->GetData(),m_pData->GetData(),nNewLength);

		pNewData->SetLength(nNewLength);

		m_pData->DecRef();

		m_pData = pNewData;
	}
}

size_t UnicodeString::GetCharString(char *lpszStr, size_t nSize, UINT CodePage) const
{
	if (!lpszStr)
		return 0;
	size_t nCopyLength = (nSize <= m_pData->GetLength()+1 ? nSize-1 : m_pData->GetLength());
	WideCharToMultiByte(CodePage,0,m_pData->GetData(),(int)nCopyLength,lpszStr,(int)nCopyLength+1,NULL,NULL);
	lpszStr[nCopyLength] = 0;
	return nCopyLength+1;
}

const UnicodeString& UnicodeString::SetData(const UnicodeString &strCopy)
{
	if ( strCopy.m_pData != m_pData )
	{
		if (m_pData)
			m_pData->DecRef();
		m_pData = strCopy.m_pData;
		m_pData->AddRef();
	}
	return *this;
}

const UnicodeString& UnicodeString::SetData(const wchar_t *lpwszData)
{
	size_t nLength = StrLength(NullToEmpty(lpwszData));
	return SetData(lpwszData,nLength);
}

const UnicodeString& UnicodeString::SetData(const wchar_t *lpwszData, size_t nLength)
{
	if (m_pData && m_pData->GetRef() == 1 && nLength + 1 <= m_pData->GetSize())
	{
		wmemmove(m_pData->GetData(),lpwszData,nLength);
		m_pData->SetLength(nLength);
	}
	else
	{
		if (!nLength)
		{
			if (m_pData)
				m_pData->DecRef();
			m_pData = eus();
			m_pData->AddRef();
		}
		else
		{
			UnicodeStringData *pNewData = new UnicodeStringData(nLength + 1);
			wmemcpy(pNewData->GetData(),lpwszData,nLength);
			pNewData->SetLength(nLength);
			if (m_pData)
				m_pData->DecRef();
			m_pData = pNewData;
		}
	}
	return *this;
}


const UnicodeString& UnicodeString::SetData(const char *lpszData, UINT CodePage)
{
	if (m_pData)
		m_pData->DecRef();

	if (!lpszData || !*lpszData)
	{
		m_pData = eus();
		m_pData->AddRef();
	}
	else
	{
		size_t nSize = MultiByteToWideChar(CodePage,0,lpszData,-1,NULL,0);
		m_pData = new UnicodeStringData(nSize);
		MultiByteToWideChar(CodePage,0,lpszData,(int)nSize,m_pData->GetData(),(int)m_pData->GetSize());
		m_pData->SetLength(nSize - 1);
	}
	return *this;
}

UnicodeString& UnicodeString::Replace(size_t Pos, size_t Len, const wchar_t* Data, size_t DataLen)
{
	// Pos & Len must be valid
	assert(Pos <= m_pData->GetLength());
	assert(Len <= m_pData->GetLength());
	assert(Pos + Len <= m_pData->GetLength());
	// Data and *this must not intersect (but Data can be located entirely within *this)
	assert(!(Data < m_pData->GetData() && Data + DataLen > m_pData->GetData()));
	assert(!(Data < m_pData->GetData() + m_pData->GetLength() && Data + DataLen > m_pData->GetData() + m_pData->GetLength()));

	if ((Len == 0) && (DataLen == 0))
		return *this;
	size_t NewLength = m_pData->GetLength() + DataLen - Len;
	if (m_pData->GetRef() == 1 && NewLength + 1 <= m_pData->GetSize())
	{
		if (Data >= m_pData->GetData() && Data + DataLen <= m_pData->GetData() + m_pData->GetLength())
		{
			// copy data from self
			UnicodeString TmpStr(Data, DataLen);
			wmemmove(m_pData->GetData() + Pos + DataLen, m_pData->GetData() + Pos + Len, m_pData->GetLength() - Pos - Len);
			wmemcpy(m_pData->GetData() + Pos, TmpStr.CPtr(), TmpStr.GetLength());
		}
		else
		{
			wmemmove(m_pData->GetData() + Pos + DataLen, m_pData->GetData() + Pos + Len, m_pData->GetLength() - Pos - Len);
			wmemcpy(m_pData->GetData() + Pos, Data, DataLen);
		}
		m_pData->SetLength(NewLength);
	}
	else
	{
		UnicodeStringData *NewData = new UnicodeStringData(NewLength + 1);
		wmemcpy(NewData->GetData(), m_pData->GetData(), Pos);
		wmemcpy(NewData->GetData() + Pos, Data, DataLen);
		wmemcpy(NewData->GetData() + Pos + DataLen, m_pData->GetData() + Pos + Len, m_pData->GetLength() - Pos - Len);
		NewData->SetLength(NewLength);
		m_pData->DecRef();
		m_pData = NewData;
	}

	return *this;
}

UnicodeString& UnicodeString::Replace(size_t Pos, size_t Len, const UnicodeString& Str)
{
	return Replace(Pos, Len, Str.CPtr(), Str.GetLength());
}

UnicodeString& UnicodeString::Replace(size_t Pos, size_t Len, const wchar_t* Str)
{
	return Replace(Pos, Len, Str, StrLength(NullToEmpty(Str)));
}

UnicodeString& UnicodeString::Replace(size_t Pos, size_t Len, wchar_t Ch)
{
	return Replace(Pos, Len, &Ch, 1);
}

UnicodeString& UnicodeString::Append(const wchar_t *Str, size_t StrLen)
{
	return Replace(GetLength(), 0, Str, StrLen);
}

UnicodeString& UnicodeString::Append(const UnicodeString &Str)
{
	return Append(Str.CPtr(), Str.GetLength());
}

UnicodeString& UnicodeString::Append(const wchar_t *Str)
{
	return Append(Str, StrLength(NullToEmpty(Str)));
}

UnicodeString& UnicodeString::Append(wchar_t Ch)
{
	return Append(&Ch, 1);
}

UnicodeString& UnicodeString::Append(const char *lpszAdd, UINT CodePage)
{
	if (lpszAdd && *lpszAdd)
	{
		size_t nAddSize = MultiByteToWideChar(CodePage,0,lpszAdd,-1,NULL,0);
		size_t nNewLength = m_pData->GetLength() + nAddSize - 1;
		Inflate(nNewLength + 1);
		MultiByteToWideChar(CodePage,0,lpszAdd,(int)nAddSize,m_pData->GetData() + m_pData->GetLength(),(int)m_pData->GetSize());
		m_pData->SetLength(nNewLength);
	}
	return *this;
}

UnicodeString& UnicodeString::Insert(size_t Pos, const wchar_t* Str, size_t StrLen)
{
	return Replace(Pos, 0, Str, StrLen);
}

UnicodeString& UnicodeString::Insert(size_t Pos, const UnicodeString& Str)
{
	return Insert(Pos, Str.CPtr(), Str.GetLength());
}

UnicodeString& UnicodeString::Insert(size_t Pos, const wchar_t* Str)
{
	return Insert(Pos, Str, StrLength(NullToEmpty(Str)));
}

UnicodeString& UnicodeString::Insert(size_t Pos, wchar_t Ch)
{
	return Insert(Pos, &Ch, 1);
}

UnicodeString& UnicodeString::Copy(const wchar_t *Str, size_t StrLen)
{
	return Replace(0, GetLength(), Str, StrLen);
}

UnicodeString& UnicodeString::Copy(const UnicodeString &Str)
{
	return Copy(Str.CPtr(), Str.GetLength());
}

UnicodeString& UnicodeString::Copy(const wchar_t *Str)
{
	return Copy(Str, StrLength(NullToEmpty(Str)));
}

UnicodeString& UnicodeString::Copy(wchar_t Ch)
{
	return Copy(&Ch, 1);
}

UnicodeString& UnicodeString::Remove(size_t Pos, size_t Len)
{
	return Replace(Pos, Len, NULL, 0);
}

UnicodeString& UnicodeString::Clear()
{
	return Remove(0, GetLength());
}

bool UnicodeString::Equal(size_t Pos, size_t Len, const wchar_t* Data, size_t DataLen) const
{
	if (Pos >= m_pData->GetLength())
		Len = 0;
	else if (Len >= m_pData->GetLength() || Pos + Len >= m_pData->GetLength())
		Len = m_pData->GetLength() - Pos;
	if (Len != DataLen)
		return false;
	return wmemcmp(m_pData->GetData() + Pos, Data, Len) == 0;
}

bool UnicodeString::Equal(size_t Pos, const wchar_t* Str, size_t StrLen) const
{
	return Equal(Pos, StrLen, Str, StrLen);
}

bool UnicodeString::Equal(size_t Pos, const wchar_t* Str) const
{
	return Equal(Pos, StrLength(Str), Str, StrLength(Str));
}

bool UnicodeString::Equal(size_t Pos, const UnicodeString& Str) const
{
	return Equal(Pos, Str.GetLength(), Str.CPtr(), Str.GetLength());
}

bool UnicodeString::Equal(size_t Pos, wchar_t Ch) const
{
	return Equal(Pos, 1, &Ch, 1);
}

bool UnicodeString::operator==(const UnicodeString& Str) const
{
	return Equal(0, GetLength(), Str.CPtr(), Str.GetLength());
}

bool UnicodeString::operator==(const wchar_t* Str) const
{
	return Equal(0, GetLength(), Str, StrLength(Str));
}

bool UnicodeString::operator==(wchar_t Ch) const
{
	return Equal(0, GetLength(), &Ch, 1);
}

const UnicodeString operator+(const UnicodeString &strSrc1, const UnicodeString &strSrc2)
{
	return UnicodeString(strSrc1).Append(strSrc2);
}

const UnicodeString operator+(const UnicodeString &strSrc1, const char *lpszSrc2)
{
	return UnicodeString(strSrc1).Append(lpszSrc2);
}

const UnicodeString operator+(const UnicodeString &strSrc1, const wchar_t *lpwszSrc2)
{
	return UnicodeString(strSrc1).Append(lpwszSrc2);
}

wchar_t *UnicodeString::GetBuffer (size_t nSize)
{
	Inflate (nSize == (size_t)-1?m_pData->GetSize():nSize);

	return m_pData->GetData ();
}

void UnicodeString::ReleaseBuffer (size_t nLength)
{
	if ( nLength == (size_t)-1 )
		nLength = StrLength(m_pData->GetData());
	if (nLength >= m_pData->GetSize())
		nLength = m_pData->GetSize() - 1;
	m_pData->SetLength (nLength);
}

size_t UnicodeString::SetLength(size_t nLength)
{
	if (!nLength && m_pData->GetRef() > 1)
	{
		m_pData->DecRef();
		m_pData = eus();
		m_pData->AddRef();
	}
	else if (nLength < m_pData->GetLength())
	{
		Inflate(nLength+1);
		return m_pData->SetLength(nLength);
	}
	return m_pData->GetLength();
}

int __cdecl UnicodeString::Format (const wchar_t * format, ...)
{
	wchar_t *buffer = NULL;
	size_t Size = MAX_PATH;

	int retValue = -1;
	va_list argptr;

	va_start( argptr, format );

	do
	{
		Size <<= 1;
		wchar_t *tmpbuffer = (wchar_t*)xf_realloc_nomove(buffer, Size*sizeof (wchar_t));

		if (!tmpbuffer)
		{
			va_end( argptr );
			xf_free (buffer);
			return retValue;
		}

		buffer = tmpbuffer;

		//_vsnwprintf не всегда ставит '\0' вконце.
		//ѕоэтому надо обнулить и передать в _vsnwprintf размер-1.
		buffer[Size-1] = 0;
		retValue = _vsnwprintf ( buffer, Size-1, format, argptr );

	} while ( retValue == -1 );

	va_end( argptr );

	SetData (buffer);

	xf_free (buffer);

	return retValue;
}

bool UnicodeString::PosI(size_t &nPos, const wchar_t *lpwszFind, size_t nStartPos) const
{
	const wchar_t *lpwszStr = StrStrI(m_pData->GetData()+nStartPos,lpwszFind);
	if (lpwszStr)
	{
		nPos = lpwszStr - m_pData->GetData();
		return true;
	}
	return false;
}
