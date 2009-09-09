#ifndef __UNICODESTRING_HPP__
#define __UNICODESTRING_HPP__
/*
UnicodeString.hpp

Unicode строка
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

#define string UnicodeString

#define __US_DELTA 20

class UnicodeStringData {
private:
	size_t m_nLength;
	size_t m_nSize;
	size_t m_nDelta;
	int m_nRefCount;
	wchar_t *m_pData;

	wchar_t *AllocData(size_t nSize, size_t *nNewSize)
	{
		if (nSize <= m_nDelta)
			*nNewSize = m_nDelta;
		else if (nSize%m_nDelta > 0)
			*nNewSize = (nSize/m_nDelta + 1) * m_nDelta;
		else
			*nNewSize = nSize;
		return new wchar_t[*nNewSize];
	}

	void FreeData(wchar_t *pData)
	{
		if (pData)
			delete [] pData;
	}

public:
	UnicodeStringData(size_t nSize=0, size_t nDelta=0)
	{
		m_nDelta = (nDelta == 0 ? __US_DELTA : nDelta);
		m_nLength = 0;
		m_nRefCount = 1;
		m_pData = AllocData(nSize,&m_nSize);
		if (m_pData)
			*m_pData = 0;
		else
			m_nSize=0;
	}

	size_t SetLength(size_t nLength)
	{
		//if (nLength<m_nSize) //Эту проверку делает верхний класс, так что скажем что это оптимизация
		{
			m_nLength = nLength;
			m_pData[m_nLength] = 0;
		}
		return m_nLength;
	}

	void Inflate(size_t nSize)
	{
		if (nSize <= m_nSize)
			return;
		wchar_t *pOldData = m_pData;
		size_t nOldSize = m_nSize;
		if (nSize >= m_nDelta << 3)
			nSize = nSize << 1;
		else
			nSize = (nSize/m_nDelta + 1) * m_nDelta;
		m_pData = AllocData(nSize,&m_nSize);
		if (!m_pData)
		{
			m_pData = pOldData;
			m_nSize = nOldSize;
		}
		else
		{
			if (pOldData)
				wmemcpy(m_pData,pOldData,m_nLength);
			m_pData[m_nLength] = 0;
			FreeData(pOldData);
		}
	}

	wchar_t *GetData()
	{
		return m_pData;
	}

	size_t GetLength() const
	{
		return m_nLength;
	}

	size_t GetSize() const
	{
		return m_nSize;
	}

	int GetRef()
	{
		return m_nRefCount;
	}

	void AddRef()
	{
		m_nRefCount++;
	}

	void DecRef()
	{
		m_nRefCount--;
		if (!m_nRefCount)
			delete this;
	}

	~UnicodeStringData()
	{
		FreeData(m_pData);
	}
};

class UnicodeString {
private:
	UnicodeStringData *m_pData;

	void DeleteData()
	{
		if (m_pData)
			m_pData->DecRef();
	}

public:
	UnicodeString();

	explicit UnicodeString(size_t nSize, size_t nDelta=0)
	{
		m_pData = new UnicodeStringData(nSize, nDelta);
	}

	const UnicodeString& SetData(const UnicodeString &strCopy);
	const UnicodeString& SetData(const wchar_t *lpwszData);
	const UnicodeString& SetData(const wchar_t *lpwszData, size_t nLength);
	const UnicodeString& SetData(const char *lpszData, UINT CodePage=CP_OEMCP);

	UnicodeString(const UnicodeString &strCopy)
	{
		m_pData = NULL;
		SetData(strCopy);
	}

	UnicodeString(const wchar_t *lpwszData)
	{
		m_pData = NULL;
		SetData(lpwszData);
	}

	UnicodeString(const wchar_t *lpwszData, size_t nLength)
	{
		m_pData = NULL;
		SetData(lpwszData,nLength);
	}

	UnicodeString(const char *lpszData, UINT CodePage=CP_OEMCP)
	{
		m_pData = NULL;
		SetData(lpszData,CodePage);
	}

	~UnicodeString()
	{
		DeleteData();
	}

	void Inflate(size_t nSize);

	size_t GetLength() const
	{
		return m_pData->GetLength();
	}

	size_t GetCharString(char *lpszStr, size_t nSize, UINT CodePage=CP_OEMCP) const;

	UnicodeString& Replace(size_t Pos, size_t Len, const wchar_t* Data, size_t DataLen);
	UnicodeString& Replace(size_t Pos, size_t Len, const UnicodeString& Str);
	UnicodeString& Replace(size_t Pos, size_t Len, const wchar_t* Str);
	UnicodeString& Replace(size_t Pos, size_t Len, wchar_t Ch);
	UnicodeString& Append(const wchar_t* Str, size_t StrLen);
	UnicodeString& Append(const UnicodeString& Str);
	UnicodeString& Append(const wchar_t* Str);
	UnicodeString& Append(wchar_t Ch);
	UnicodeString& Append(const char *lpszAdd, UINT CodePage=CP_OEMCP);
	UnicodeString& Insert(size_t Pos, const wchar_t* Str, size_t StrLen);
	UnicodeString& Insert(size_t Pos, const UnicodeString& Str);
	UnicodeString& Insert(size_t Pos, const wchar_t* Str);
	UnicodeString& Insert(size_t Pos, wchar_t Ch);
	UnicodeString& Copy(const wchar_t *Str, size_t StrLen);
	UnicodeString& Copy(const UnicodeString &Str);
	UnicodeString& Copy(const wchar_t *Str);
	UnicodeString& Copy(wchar_t Ch);
	UnicodeString& Remove(size_t Pos, size_t Len = 1);
	UnicodeString& Clear();

	operator const wchar_t *() const
	{
		return m_pData->GetData();
	}

	const wchar_t *CPtr() const
	{
		return m_pData->GetData();
	}

	bool Equal(size_t Pos, size_t Len, const wchar_t* Data, size_t DataLen) const;
	bool Equal(size_t Pos, const wchar_t* Str, size_t StrLen) const;
	bool Equal(size_t Pos, const wchar_t* Str) const;
	bool Equal(size_t Pos, const UnicodeString& Str) const;
	bool Equal(size_t Pos, wchar_t Ch) const;

	const UnicodeString& operator=(const UnicodeString &strCopy)
	{
		return Copy(strCopy);
	}

	const UnicodeString& operator=(const char *lpszData)
	{
		return Copy(lpszData);
	}

	const UnicodeString& operator=(const wchar_t *lpwszData)
	{
		return Copy(lpwszData);
	}

	const UnicodeString& operator=(wchar_t chData)
	{
		return Copy(chData);
	}

	const UnicodeString& operator+=(const UnicodeString &strAdd)
	{
		return Append(strAdd);
	}

	const UnicodeString& operator+=(const char *lpszAdd)
	{
		return Append(lpszAdd);
	}

	const UnicodeString& operator+=(const wchar_t *lpwszAdd)
	{
		return Append(lpwszAdd);
	}

	const UnicodeString& operator+=(wchar_t chAdd)
	{
		return Append(chAdd);
	}

	friend const UnicodeString operator+(const UnicodeString &strSrc1, const UnicodeString &strSrc2);
	friend const UnicodeString operator+(const UnicodeString &strSrc1, const char *lpszSrc2);
	friend const UnicodeString operator+(const UnicodeString &strSrc1, const wchar_t *lpwszSrc2);

	wchar_t *GetBuffer (size_t nSize = (size_t)-1);
	void ReleaseBuffer (size_t nLength = (size_t)-1);

	size_t SetLength(size_t nLength);

	wchar_t At (size_t nIndex) const
	{
		return m_pData->GetData()[nIndex];
	}

	bool IsEmpty () const
	{
		return !(m_pData->GetLength () && *m_pData->GetData ());
	}

	void Lower (size_t nStartPos=0, size_t nLength=(size_t)-1)
	{
		Inflate (m_pData->GetSize());
		CharLowerBuffW (m_pData->GetData()+nStartPos, nLength==(size_t)-1?(DWORD)(m_pData->GetLength()-nStartPos):(DWORD)nLength);
	}

	void Upper (size_t nStartPos=0, size_t nLength=(size_t)-1)
	{
		Inflate (m_pData->GetSize());
		CharUpperBuffW (m_pData->GetData()+nStartPos, nLength==(size_t)-1?(DWORD)(m_pData->GetLength()-nStartPos):(DWORD)nLength);
	}

	void LShift (size_t nShiftCount, size_t nStartPos=0)
	{
		/*if ( nShiftCount > m_pData->GetLength () )
			nShiftCount = m_pData->GetLength ();*/

		Inflate (m_pData->GetSize());
		wmemmove (m_pData->GetData()+nStartPos, m_pData->GetData()+nStartPos+nShiftCount, (m_pData->GetLength()-nStartPos-nShiftCount+1));
		m_pData->SetLength(m_pData->GetLength()-nShiftCount);
	}

	bool Pos(size_t &nPos, wchar_t Ch, size_t nStartPos=0) const
	{
		const wchar_t *lpwszStr = wcschr(m_pData->GetData()+nStartPos,Ch);
		if (lpwszStr)
		{
			nPos = lpwszStr - m_pData->GetData();
			return true;
		}
		return false;
	}

	bool Pos(size_t &nPos, const wchar_t *lpwszFind, size_t nStartPos=0) const
	{
		const wchar_t *lpwszStr = wcsstr(m_pData->GetData()+nStartPos,lpwszFind);
		if (lpwszStr)
		{
			nPos = lpwszStr - m_pData->GetData();
			return true;
		}
		return false;
	}

	bool PosI(size_t &nPos, const wchar_t *lpwszFind, size_t nStartPos=0) const;

	bool RPos(size_t &nPos, wchar_t Ch, size_t nStartPos=0) const
	{
		const wchar_t *lpwszStrStart = m_pData->GetData()+nStartPos;
		const wchar_t *lpwszStrEnd = m_pData->GetData()+m_pData->GetLength();
		do
		{
			if (*lpwszStrEnd == Ch)
			{
				nPos = lpwszStrEnd - m_pData->GetData();
				return true;
			}
			lpwszStrEnd--;
		} while (lpwszStrEnd >= lpwszStrStart);
		return false;
	}

	bool Contains(wchar_t Ch, size_t nStartPos=0) const
	{
		return wcschr(m_pData->GetData()+nStartPos,Ch)==NULL ? false : true;
	}

	bool Contains(const wchar_t *lpwszFind, size_t nStartPos=0) const
	{
		return wcsstr(m_pData->GetData()+nStartPos,lpwszFind)==NULL ? false : true;
	}

	int __cdecl Format (const wchar_t * format, ...);
};

#endif // __UNICODESTRING_HPP__
