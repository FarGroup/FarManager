/*
UnicodeString.hpp

Unicode строки
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

void UnicodeString::SetEUS()
{
	//для оптимизации создания пустых UnicodeString
	static auto EmptyUnicodeStringData = std::make_shared<UnicodeString::UnicodeStringData>(1, 1);
	m_pData = EmptyUnicodeStringData;
}

void UnicodeString::Inflate(size_t nSize)
{
	if (m_pData.unique())
	{
		m_pData->Inflate(nSize);
	}
	else
	{
		auto pNewData = std::make_shared<UnicodeStringData>(nSize);
		size_t nNewLength = std::min(m_pData->GetLength(),nSize-1);
		wmemcpy(pNewData->GetData(),m_pData->GetData(),nNewLength);
		pNewData->SetLength(nNewLength);
		m_pData = pNewData;
	}
}

UnicodeString& UnicodeString::replace(size_t Pos, size_t Len, const wchar_t* Data, size_t DataLen)
{
	// Pos & Len must be valid
	assert(Pos <= m_pData->GetLength());
	assert(Len <= m_pData->GetLength());
	assert(Pos + Len <= m_pData->GetLength());
	// Data and *this must not intersect (but Data can be located entirely within *this)
	assert(!(Data < m_pData->GetData() && Data + DataLen > m_pData->GetData()));
	assert(!(Data < m_pData->GetData() + m_pData->GetLength() && Data + DataLen > m_pData->GetData() + m_pData->GetLength()));

	if (!Len && !DataLen)
		return *this;

	if (DataLen == static_cast<size_t>(-1))
	{
		DataLen = StrLength(Data);
	}
	size_t NewLength = m_pData->GetLength() + DataLen - Len;

	if (m_pData.unique() && NewLength + 1 <= m_pData->GetSize())
	{
		if (NewLength)
		{
			if (Data >= m_pData->GetData() && Data + DataLen <= m_pData->GetData() + m_pData->GetLength())
			{
				// copy data from self
				UnicodeString TmpStr(Data, DataLen);
				wmemmove(m_pData->GetData() + Pos + DataLen, m_pData->GetData() + Pos + Len, m_pData->GetLength() - Pos - Len);
				wmemcpy(m_pData->GetData() + Pos, TmpStr.data(), TmpStr.size());
			}
			else
			{
				wmemmove(m_pData->GetData() + Pos + DataLen, m_pData->GetData() + Pos + Len, m_pData->GetLength() - Pos - Len);
				wmemcpy(m_pData->GetData() + Pos, Data, DataLen);
			}
		}

		m_pData->SetLength(NewLength);
	}
	else
	{
		if (!NewLength)
		{
			SetEUS();
			return *this;
		}

		auto NewData = std::make_shared<UnicodeStringData>(NewLength + 1);
		wmemcpy(NewData->GetData(), m_pData->GetData(), Pos);
		wmemcpy(NewData->GetData() + Pos, Data, DataLen);
		wmemcpy(NewData->GetData() + Pos + DataLen, m_pData->GetData() + Pos + Len, m_pData->GetLength() - Pos - Len);
		NewData->SetLength(NewLength);
		m_pData = NewData;
	}

	return *this;
}

UnicodeString& UnicodeString::assign(const UnicodeString &Str)
{
	if (Str.m_pData != m_pData)
	{
		m_pData = Str.m_pData;
	}

	return *this;
}

UnicodeString UnicodeString::substr(size_t Pos, size_t Len) const
{
	if (size() - Pos < Len)
		Len = size() - Pos;

	return UnicodeString(m_pData->GetData() + Pos, Len);
}

int UnicodeString::compare(size_t Pos, size_t Len, const wchar_t* Data, size_t DataLen) const
{
	if (size() - Pos < Len)
		Len = size() - Pos;

	int ret = wmemcmp(m_pData->GetData() + Pos, Data, Len);
	return ret? ret : Len < DataLen ? -1 : Len == DataLen ? 0 : +1;
}

const UnicodeString operator+(const UnicodeString &strSrc1, const UnicodeString &strSrc2)
{
	return UnicodeString(strSrc1).append(strSrc2);
}

const UnicodeString operator+(const UnicodeString &strSrc1, const wchar_t *lpwszSrc2)
{
	return UnicodeString(strSrc1).append(lpwszSrc2);
}

const UnicodeString operator+(const wchar_t *strSrc1, const UnicodeString &lpwszSrc2)
{
	return UnicodeString(strSrc1).append(lpwszSrc2);
}

wchar_t *UnicodeString::GetBuffer(size_t nSize)
{
	Inflate(nSize == (size_t)-1?m_pData->GetSize():nSize);
	return m_pData->GetData();
}

void UnicodeString::ReleaseBuffer(size_t nLength)
{
	if (nLength == (size_t)-1)
		nLength = StrLength(m_pData->GetData());

	if (nLength >= m_pData->GetSize())
		nLength = m_pData->GetSize() - 1;

	m_pData->SetLength(nLength);
}

void UnicodeString::resize(size_t nLength, wchar_t c)
{
	if (nLength < m_pData->GetLength())
	{
		if (!nLength && !m_pData.unique())
		{
			SetEUS();
		}
		else
		{
			Inflate(nLength+1);
			m_pData->SetLength(nLength);
		}
	}
	else
	{
		append(nLength - m_pData->GetLength(), c);
	}
}

void UnicodeString::clear()
{
	if (!m_pData.unique())
	{
		SetEUS();
	}
	else
	{
		m_pData->SetLength(0);
	}
}
