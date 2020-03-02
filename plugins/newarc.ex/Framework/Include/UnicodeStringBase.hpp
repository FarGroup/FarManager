#pragma once
#include "StringBase.hpp"
#include <stdio.h>

class UnicodeStringBase
{
protected:

	StringData<wchar_t>* m_pData;

public:

	UnicodeStringBase()
	{
		SetEmpty();
	}

	UnicodeStringBase(const char* lpData, int nSrcEncoding)
	{
		m_pData = nullptr;
		SetData(lpData, nSrcEncoding);
	}

	UnicodeStringBase(const wchar_t* lpData)
	{
		m_pData = nullptr;
		SetData(lpData);
	}

	UnicodeStringBase(const UnicodeStringBase& strCopy)
	{
		m_pData = NULL;
		SetData(strCopy);
	}

	UnicodeStringBase(const AsciiStringBase& strCopy)
	{
		m_pData = NULL;
		SetData(strCopy);
	}

	UnicodeStringBase& SetData(const AsciiStringBase& strCopy)
	{
		m_pData = nullptr;
		SetData(strCopy.GetString(), strCopy.GetLength(), strCopy.GetEncoding());
	
		return *this;
	}

	UnicodeStringBase& SetData(const UnicodeStringBase& strCopy)
	{
		if ( strCopy.m_pData != m_pData )
		{
			FreeData();

			m_pData = strCopy.m_pData;
			m_pData->AddRef();
		}

		return *this;
	}

	UnicodeStringBase& SetData(const wchar_t* lpData, size_t nLength)
	{
		if ( !lpData || !nLength )
		{
			FreeData();
			SetEmpty();
		}
		else
		{
			size_t nSize = nLength;

			if ( m_pData && m_pData->Unique() && (nSize < m_pData->GetSize()) )
			{
				memmove(m_pData->GetData(), lpData, nSize*sizeof(wchar_t));
				m_pData->SetLength(nSize);
			}
			else
			{
				StringData<wchar_t>* pNewData = new StringData<wchar_t>(nSize+1);

				memmove(pNewData->GetData(), lpData, nSize*sizeof(wchar_t));
				pNewData->SetLength(nSize);

				FreeData();
				m_pData = pNewData;
			}
		}

		return *this;
	}

	UnicodeStringBase& SetData(const char* lpData, size_t nLength, int nSrcEncoding)
	{
		if ( !lpData || !nLength )
		{
			FreeData();
			SetEmpty();
		}
		else
		{
			size_t nSize = MultiByteToWideChar(nSrcEncoding, 0, lpData, nLength, 0, 0);

			if ( m_pData && m_pData->Unique() && (nSize < m_pData->GetSize()) )
			{
				MultiByteToWideChar(nSrcEncoding, 0, lpData, nLength, m_pData->GetData(), nSize);
				m_pData->SetLength(nSize);
			}
			else
			{
				StringData<wchar_t>* pNewData = new StringData<wchar_t>(nSize+1);

				MultiByteToWideChar(nSrcEncoding, 0, lpData, nLength, pNewData->GetData(), nSize);
				pNewData->SetLength(nSize);

				FreeData();
				m_pData = pNewData;
			}
		}

		return *this;
	}

	UnicodeStringBase& SetData(const char* lpData, int nSrcEncoding)
	{
		return SetData(lpData, lpData?strlen(lpData):0, nSrcEncoding);
	}

	UnicodeStringBase& SetData(const wchar_t* lpData)
	{
		return SetData(lpData, lpData?wcslen(lpData):0);
	}

	UnicodeStringBase& SetData(wchar_t Ch)
	{
		return SetData(&Ch, 1);
	}

	UnicodeStringBase& SetData(char Ch, int nSrcEncoding)
	{
		return SetData(&Ch, 1, nSrcEncoding);
	}

	UnicodeStringBase& Append(const char* lpData, int nSrcEncoding)
	{
		return Append(lpData, lpData?strlen(lpData):0, nSrcEncoding);
	}

	UnicodeStringBase& Append(const wchar_t* lpData)
	{
		return Append(lpData, lpData?wcslen(lpData):0);
	}

	UnicodeStringBase& Append(const UnicodeStringBase& strAdd)
	{
		return Append(strAdd.GetString(), strAdd.GetLength());
	}

	UnicodeStringBase& Append(const AsciiStringBase& strAdd)
	{
		return Append(strAdd.GetString(), strAdd.GetLength(), strAdd.GetEncoding());
	}


	UnicodeStringBase& Append(wchar_t Ch)
	{
		return Append(&Ch, 1);
	}

	UnicodeStringBase& Append(char Ch, int nSrcEncoding)
	{
		return Append(&Ch, 1, nSrcEncoding);
	}

	UnicodeStringBase& Append(const char* lpData, size_t nLength, int nSrcEncoding)
	{
		if ( !lpData || !nLength ) //*lpData??
			return *this;

		size_t nNewSize = MultiByteToWideChar(nSrcEncoding, 0, lpData, nLength, 0, 0);
		size_t nSize = m_pData->GetLength()+nNewSize;

		if ( m_pData->Unique() && (nSize < m_pData->GetSize()) ) //check!!!
		{
			MultiByteToWideChar(nSrcEncoding, 0, lpData, nLength, m_pData->GetData()+m_pData->GetLength(), nNewSize);
			m_pData->SetLength(nSize);
		}
		else
		{
			StringData<wchar_t>* pNewData = new StringData<wchar_t>(nSize+1);

			memcpy(pNewData->GetData(), m_pData->GetData(), m_pData->GetLength()*sizeof(wchar_t));
			MultiByteToWideChar(nSrcEncoding, 0, lpData, nLength, pNewData->GetData()+m_pData->GetLength(), nNewSize);

			pNewData->SetLength(nSize);
			FreeData();

			m_pData = pNewData;
		}

		return *this;
	}

	UnicodeStringBase& Append(const wchar_t* lpData, size_t nLength)
	{
		if ( !lpData || !nLength ) //*lpData??
			return *this;

		size_t nSize = nLength+m_pData->GetLength();

		if ( m_pData->Unique() && (nSize < m_pData->GetSize()) ) //check!!!
		{
			memmove(m_pData->GetData()+m_pData->GetLength(), lpData, nLength*sizeof(wchar_t));
			m_pData->SetLength(nSize);
		}
		else
		{
			StringData<wchar_t>* pNewData = new StringData<wchar_t>(nSize+1);

			memcpy(pNewData->GetData(), m_pData->GetData(), m_pData->GetLength()*sizeof(wchar_t));
			memmove(pNewData->GetData()+m_pData->GetLength(), lpData, nLength*sizeof(wchar_t));

			pNewData->SetLength(nSize);
			FreeData();

			m_pData = pNewData;
		}

		return *this;
	}

	UnicodeStringBase& operator=(const wchar_t* lpData)
	{
		return SetData(lpData);
	}

	UnicodeStringBase& operator=(const UnicodeStringBase& strCopy)
	{
		return SetData(strCopy);
	}

	UnicodeStringBase& operator=(const AsciiStringBase& strCopy)
	{
		return SetData(strCopy);
	}

	UnicodeStringBase operator+(const UnicodeStringBase& strAdd)
	{
		UnicodeStringBase strResult = *this;
		return strResult.Append(strAdd);
	}

	UnicodeStringBase operator+(const AsciiStringBase& strAdd)
	{
		UnicodeStringBase strResult = *this;
		return strResult.Append(strAdd);
	}

	UnicodeStringBase operator+(wchar_t Ch)
	{
		UnicodeStringBase strResult = *this;
		return strResult.Append(Ch);
	}


	UnicodeStringBase& operator+=(wchar_t Ch)
	{
		return Append(Ch);
	}

	UnicodeStringBase& operator+=(const UnicodeStringBase& strAdd)
	{
		return Append(strAdd);
	}

	UnicodeStringBase& operator+=(const AsciiStringBase& strAdd)
	{
		return Append(strAdd);
	}

	bool operator==(const wchar_t* lpCmp)
	{
		return (lpCmp && (wcslen(lpCmp) == GetLength()) && !memcmp(lpCmp, m_pData->GetData(), GetLength()*sizeof(wchar_t)));
	}

	const wchar_t* GetString() const
	{
		return m_pData->GetData();
	}

	size_t GetLength() const
	{
		return m_pData->GetLength();
	}

	operator const wchar_t*() const
	{
		return m_pData->GetData();
	}

	wchar_t At(size_t nIndex) const
	{
		return m_pData->GetData()[nIndex];
	}

	wchar_t operator[](size_t nIndex) const
	{
		return At(nIndex);
	}

	bool operator==(const UnicodeStringBase& strCmp)
	{
		return (strCmp.m_pData == m_pData) ||
				(
					(strCmp.GetLength() == GetLength()) &&
					!memcmp(m_pData->GetData(), strCmp.m_pData->GetData(), GetLength())
				);
	}

	bool operator!=(const UnicodeStringBase& strCmp)
	{
		return !(*this == strCmp);
	}

	int __cdecl Format(const wchar_t* format, ...)
	{
		wchar_t* buffer = NULL;
		size_t Size = MAX_PATH;

		int retValue = -1;
		va_list argptr;

		va_start(argptr, format);

		do
		{
			Size <<= 1;
			wchar_t* tmpbuffer = (wchar_t*)realloc(buffer, Size*sizeof(wchar_t));

			if ( !tmpbuffer )
			{
				va_end(argptr);
				free(buffer);
				return retValue;
			}

			buffer = tmpbuffer;

			//_vsnwprintf не всегда ставит '\0' вконце.
			//Поэтому надо обнулить и передать в _vsnwprintf размер-1.
			buffer[Size-1] = 0;
			retValue = _vsnwprintf(buffer, Size-1, format, argptr);

		} while (retValue == -1);

		va_end(argptr);

		SetData(buffer);

		free(buffer);

		return retValue;
	}

	wchar_t* GetBuffer(size_t nLength = (size_t)-1)
	{
		size_t nNewLength = (nLength == (size_t)-1)?m_pData->GetLength():nLength;

		if ( !m_pData->Unique() || (nNewLength > m_pData->GetSize()) )
		{
			StringData<wchar_t>* pNewData = new StringData<wchar_t>(nNewLength+1);

			memcpy(pNewData->GetData(), m_pData->GetData(), m_pData->GetLength()*sizeof(wchar_t));
			pNewData->SetLength(m_pData->GetLength());

			FreeData();

			m_pData = pNewData;
		}

		return m_pData->GetData();
	}

	void ReleaseBuffer(size_t nLength = (size_t)-1)
	{
		//неправильная логика
		if ( nLength == (size_t)-1 )
			nLength = wcslen(m_pData->GetData());

		if ( nLength >= m_pData->GetSize() )
			nLength = m_pData->GetSize()-1;

		m_pData->SetLength (nLength);
	}

	bool IsEmpty() const
	{
		return !(m_pData->GetLength() && *m_pData->GetData());
	}

	virtual ~UnicodeStringBase()
	{
		FreeData();
	}

private:

	void FreeData()
	{
		if ( m_pData )
			m_pData->DecRef();
	}

	void SetEmpty()
	{
		m_pData = eus<wchar_t>();
		m_pData->AddRef();
	}
};

typedef UnicodeStringBase UnicodeString;
