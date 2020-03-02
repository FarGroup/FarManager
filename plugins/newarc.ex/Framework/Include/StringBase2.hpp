#pragma once
#include <windows.h>
#include <stdio.h>

#define __US_DELTA 20

class BaseTraits
{
public:

	static size_t StrLengthW(const wchar_t* lpData)
	{
		return lpData?wcslen(lpData):0;
	}

	static size_t StrLengthA(const char* lpData)
	{
		return lpData?strlen(lpData):0;
	}

	char* ConvertString(const char* lpData, int nSrcCodePage, int nDestCodePage)
	{
		//BUGBUG
		return nullptr;
	}

	void Free(char* pData)
	{
		if ( pData )
			delete [] pData;
	}
};

class UnicodeTraits : public BaseTraits
{
public:

	static int GetDefaultEncoding()
	{
		return CP_UNICODE;
	}

	static size_t StrLength(const wchar_t* lpData)
	{
		return StrLengthW(lpData);
	}

	static size_t GetAnsiDataLength(const char* lpData, size_t nLength, int nCodePage)
	{
		return MultiByteToWideChar(nCodePage, 0, lpData, nLength, NULL, 0);
	}

	static void CopyAnsiData(wchar_t* lpBuffer, size_t nSize, const char* lpData, size_t nLength, int nCodePage)
	{
		MultiByteToWideChar(nCodePage, 0, lpData, nLength, lpBuffer, nSize);
	}

	static size_t GetUnicodeDataLength(const wchar_t* lpData, size_t nLength, int nCodePage)
	{
		return nLength;
	}

	static void CopyUnicodeData(wchar_t* lpBuffer, size_t nSize, const wchar_t* lpData, size_t nLength, int nCodePage)
	{
		memmove(lpBuffer, lpData, nLength*sizeof(wchar_t));
	}

	static int VPrintf(wchar_t* buffer, size_t count, const wchar_t* format, va_list argptr)
	{
		return _vsnwprintf(buffer, count, format, argptr);
	}

	static BSTR ToBSTR(const wchar_t* lpBuffer, int nCodePage)
	{
		return SysAllocString(lpBuffer);
	}

};

class AnsiTraits : public BaseTraits
{
public:

	static int GetDefaultEncoding()
	{
		return CP_OEMCP;
	}

	static size_t StrLength(const char* lpData)
	{
		return StrLengthA(lpData);
	}

	static size_t GetAnsiDataLength(const char* lpData, size_t nLength, int nCodePage)
	{
		return nLength;
	}

	static void CopyAnsiData(char* lpBuffer, size_t nSize, const char* lpData, size_t nLength, int nCodePage)
	{
		memmove(lpBuffer, lpData, nLength);
	}

	static size_t GetUnicodeDataLength(const wchar_t* lpData, size_t nLength, int nCodePage)
	{
		return WideCharToMultiByte(nCodePage, 0, lpData, nLength, NULL, 0, NULL, NULL);
	}

	static void CopyUnicodeData(char* lpBuffer, size_t nSize, const wchar_t* lpData, size_t nLength, int nCodePage = CP_OEMCP)
	{
		WideCharToMultiByte(nCodePage, 0, lpData, nLength, lpBuffer, nSize, NULL, NULL);
	}

	static int VPrintf(char* buffer, size_t count, const char* format, va_list argptr)
	{
		return _vsnprintf(buffer, count, format, argptr);
	}

	static BSTR ToBSTR(const char* lpBuffer, int nCodePage)
	{
		size_t nSize = MultiByteToWideChar(nCodePage, 0, lpBuffer, -1, NULL, 0);

		wchar_t* pTempBuffer = new wchar_t[nSize+1];

		MultiByteToWideChar(nCodePage, 0, lpBuffer, -1, pTempBuffer, nSize);

		BSTR bstrResult = SysAllocString(pTempBuffer);

		delete [] pTempBuffer;

		return bstrResult;
	}

};

template <typename T>
class StringData {

private:

	size_t m_nLength;
	size_t m_nSize;
	size_t m_nDelta;

	int m_nRefCount;
	T* m_pData;

	T* AllocData(size_t nSize, size_t* nNewSize)
	{
		if ( nSize <= m_nDelta )
			*nNewSize = m_nDelta;
		else

		if ( nSize%m_nDelta > 0 )
			*nNewSize = (nSize/m_nDelta+1)*m_nDelta;
		else
			*nNewSize = nSize;

		return new T[*nNewSize];
	}

	void FreeData(T *pData)
	{
		if ( pData )
			delete [] pData;
	}

public:

	StringData(size_t nSize = 0, size_t nDelta = 0)
	{
		m_nDelta = (nDelta == 0 ? __US_DELTA : nDelta);

		m_nLength = 0;
		m_nRefCount = 1;

		m_pData = AllocData(nSize, &m_nSize);

		if ( m_pData )
			*m_pData = 0;
		else
			m_nSize = 0; //and what?
	}

	~StringData()
	{
		FreeData(m_pData);
	}

	size_t SetLength(size_t nLength)
	{
		m_nLength = nLength;
		m_pData[m_nLength] = 0;

		return m_nLength;
	}

	T* GetData()
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

	bool Unique() const
	{
		return (m_nRefCount == 1);
	}

	void AddRef()
	{
		m_nRefCount++;
	}

	void DecRef()
	{
		m_nRefCount--;

		if ( !m_nRefCount )
			delete this;
	}
};


template <typename T>
StringData<T> *eus()
{
	//для оптимизации создания пустых String
	static StringData<T> *EmptyStringData = new StringData<T>(1,1);
	return EmptyStringData;
}

template <typename T, class Traits>
class StringBase {

protected:

	StringData<T>* m_pData;


public:

	StringBase()
	{
		SetEmpty();
	}

	StringBase(const T* lpData)
	{
		m_pData = NULL;
		SetData(lpData);
	}

	StringBase(const StringBase& strCopy)
	{
		m_pData = NULL;
		SetData(strCopy);
	}

	StringBase(const char* lpData, int nCodePage)
	{
		m_pData = NULL;
		SetData(lpData, nCodePage);
	}

	StringBase(const wchar_t* lpData)
	{
		m_pData = NULL;
		SetData(lpData);
	}

	bool SetEncoding(int nEncoding)
	{
		if ( (m_nEncoding == CP_UNICODE) || (nEncoding == CP_UNICODE) )
			return false;

		if ( m_pData )
		{
			char* pData = Traits::ConvertString(m_pData->GetData(), m_nEncoding, nEncoding);

			//m_nEncoding = nEncoding;

			SetData(pData, nEncoding);

			Traits::Free(pData);
		}
		else
			m_nEncoding = nEncoding;

		return true;
	}

	StringBase& SetData(const StringBase &strCopy)
	{
		if ( strCopy.m_pData != m_pData )
		{
			FreeData();

			m_pData = strCopy.m_pData;
			m_pData->AddRef();
		}

		return *this;
	}

	StringBase& SetData(const wchar_t* lpData, size_t nLength)
	{
		if ( !lpData || !nLength )
		{
			FreeData();
			SetEmpty();
		}
		else
		{
			size_t nSize = Traits::GetUnicodeDataLength(lpData, nLength, m_nEncoding);

			if ( m_pData && m_pData->Unique() && (nSize < m_pData->GetSize()) )
			{
				Traits::CopyUnicodeData(m_pData->GetData(), nSize, lpData, nLength, m_nEncoding);
				m_pData->SetLength(nLength);
			}
			else
			{
				StringData<T>* pNewData = new StringData<T>(nSize+1);

				Traits::CopyUnicodeData(pNewData->GetData(), nSize, lpData, nLength, m_nEncoding);
				pNewData->SetLength(nLength);

				FreeData();
				m_pData = pNewData;
			}
		}

		return *this;
	}


	StringBase& SetData(const char* lpData, size_t nLength, int nCodePage = CP_OEMCP)
	{
		m_nEncoding = nCodePage;

		if ( !lpData || !nLength )
		{
			FreeData();
			SetEmpty();
		}
		else
		{
			size_t nSize = Traits::GetAnsiDataLength(lpData, nLength, m_nEncoding);

			if ( m_pData && m_pData->Unique() && (nSize < m_pData->GetSize()) )
			{
				Traits::CopyAnsiData(m_pData->GetData(), nSize, lpData, nLength, m_nEncoding);
				m_pData->SetLength(nLength);
			}
			else
			{
				StringData<T>* pNewData = new StringData<T>(nSize+1);

				Traits::CopyAnsiData(pNewData->GetData(), nSize, lpData, nLength, m_nEncoding);
				pNewData->SetLength(nLength);

				FreeData();
				m_pData = pNewData;
			}
		}

		return *this;
	}

	StringBase& SetData(const char* lpData, int nCodePage = CP_OEMCP)
	{
		return SetData(lpData, Traits::StrLengthA(lpData), nCodePage);
	}

	StringBase& SetData(const wchar_t* lpData, int nCodePage = CP_OEMCP)
	{
		return SetData(lpData, Traits::StrLengthW(lpData), nCodePage);
	}

	StringBase& SetData(wchar_t Ch, int nCodePage = CP_OEMCP)
	{
		return SetData(&Ch, 1, nCodePage);
	}

	StringBase& SetData(char Ch, int nCodePage = CP_OEMCP)
	{
		return SetData(&Ch, 1, nCodePage);
	}

	StringBase& Append(const char* lpData, int nCodePage = CP_OEMCP)
	{
		return Append(lpData, Traits::StrLengthA(lpData), nCodePage);
	}

	StringBase& Append(const wchar_t* lpData, int nCodePage = CP_OEMCP)
	{
		return Append(lpData, Traits::StrLengthW(lpData), nCodePage);
	}

	StringBase& Append(const StringBase& strAdd)
	{
		return Append(strAdd.GetString(), strAdd.GetLength());
	}

	StringBase& Append(wchar_t Ch, int nCodePage = CP_OEMCP)
	{
		return Append(&Ch, 1, nCodePage);
	}

	StringBase& Append(char Ch, int nCodePage = CP_OEMCP)
	{
		return Append(&Ch, 1, nCodePage);
	}

	StringBase& Append(const char* lpData, size_t nLength, int nCodePage = CP_OEMCP)
	{
		if ( !lpData || !nLength ) //*lpData??
			return *this;

		size_t nNewLength = Traits::GetAnsiDataLength(lpData, nLength, nCodePage);
		size_t nSize = nNewLength+m_pData->GetLength();

		if ( m_pData->Unique() && (nSize < m_pData->GetSize()) ) //check!!!
		{
			Traits::CopyAnsiData(m_pData->GetData()+m_pData->GetLength(), nSize, lpData, nLength, nCodePage);
			m_pData->SetLength(nSize);
		}
		else
		{
			StringData<T>* pNewData = new StringData<T>(nSize+1);

			memcpy(pNewData->GetData(), m_pData->GetData(), m_pData->GetLength()*sizeof(T));
			Traits::CopyAnsiData(pNewData->GetData()+m_pData->GetLength(), nNewLength, lpData, nLength, nCodePage);

			pNewData->SetLength(nSize);
			FreeData();

			m_pData = pNewData;
		}

		return *this;
	}

	StringBase& Append(const wchar_t* lpData, size_t nLength, int nCodePage = CP_OEMCP)
	{
		if ( !lpData || !nLength ) //*lpData??
			return *this;

		size_t nNewLength = Traits::GetUnicodeDataLength(lpData, nLength, nCodePage);
		size_t nSize = nNewLength+m_pData->GetLength();

		if ( m_pData->Unique() && (nSize < m_pData->GetSize()) ) //check!!!
		{
			Traits::CopyUnicodeData(m_pData->GetData()+m_pData->GetLength(), nSize, lpData, nLength, nCodePage);
			m_pData->SetLength(nSize);
		}
		else
		{
			StringData<T>* pNewData = new StringData<T>(nSize+1);

			memcpy(pNewData->GetData(), m_pData->GetData(), m_pData->GetLength()*sizeof(T));
			Traits::CopyUnicodeData(pNewData->GetData()+m_pData->GetLength(), nNewLength, lpData, nLength, nCodePage);

			pNewData->SetLength(nSize);
			FreeData();

			m_pData = pNewData;
		}

		return *this;
	}

	StringBase& operator=(const T* lpData)
	{
		return SetData(lpData);
	}

	StringBase& operator=(const StringBase& strCopy)
	{
		return SetData(strCopy);
	}

	StringBase operator+(const T* lpData)
	{
		string strResult = *this;
		return strResult.Append(lpData);
	}

	StringBase operator+(const StringBase& strAdd)
	{
		string strResult = *this;
		return strResult.Append(strAdd);
	}

	StringBase operator+(T Ch)
	{
		string strResult = *this;
		return strResult.Append(Ch);
	}

	StringBase& operator+=(const T* lpData)
	{
		return Append(lpData);
	}

	StringBase& operator+=(T Ch)
	{
		return Append(Ch);
	}

	StringBase& operator+=(const StringBase& strAdd)
	{
		return Append(strAdd);
	}

	T At(size_t nIndex) const
	{
		return m_pData->GetData()[nIndex];
	}

	const T* GetString() const
	{
		return m_pData->GetData();
	}

	size_t GetLength() const
	{
		return m_pData->GetLength();
	}

	operator const T*() const
	{
		return m_pData->GetData();
	}

	BSTR ToBSTR(int nCodePage = CP_OEMCP) const
	{
		return Traits::ToBSTR(m_pData->GetData(), nCodePage);
	}

	T operator[](size_t nIndex) const
	{
		return At(nIndex);
	}

	bool operator==(const StringBase& strCmp)
	{
		return (strCmp.m_pData == m_pData) ||
				(
					(strCmp.GetLength() == GetLength()) &&
					!memcmp(m_pData->GetData(), strCmp.m_pData->GetData(), GetLength()*sizeof(T))
				);
	}

	bool operator==(const T* lpCmp)
	{
		return (StrLength(lpCmp) == GetLength()) && !memcmp(lpCmp, m_pData->GetData(), GetLength()*sizeof(T));
	}

	bool operator!=(const StringBase& strCmp)
	{
		return !(*this == strCmp);
	}

	bool operator!=(const T* lpCmp)
	{
		return !(*this == *this);
	}

	virtual ~StringBase()
	{
		FreeData();
	}
/*
	friend StringBase operator+(const StringBase& strStr1, const StringBase& strStr2)
	{
		StringBase strResult = strStr1;
		return strResult.Append(strStr2);
	}

	friend StringBase operator+(const StringBase& strStr1, const T* lpData)
	{
		StringBase strResult = strStr1;
		return strResult.Append(lpData);
	}
*/
	int __cdecl Format(const T* format, ...)
	{
		T* buffer = NULL;
		size_t Size = MAX_PATH;

		int retValue = -1;
		va_list argptr;

		va_start(argptr, format);

		do
		{
			Size <<= 1;
			T* tmpbuffer = (T*)realloc(buffer, Size*sizeof (T));

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
			retValue = Traits::VPrintf(buffer, Size-1, format, argptr);

		} while (retValue == -1);

		va_end(argptr);

		SetData(buffer);

		free(buffer);

		return retValue;
	}

	T* GetBuffer(size_t nLength = (size_t)-1)
	{
		size_t nNewLength = (nLength == (size_t)-1)?m_pData->GetLength():nLength;

		if ( !m_pData->Unique() || (nNewLength > m_pData->GetSize()) )
		{
			StringData<T>* pNewData = new StringData<T>(nNewLength+1);

			memcpy(pNewData->GetData(), m_pData->GetData(), m_pData->GetLength()*sizeof(T));
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
			nLength = Traits::StrLength(m_pData->GetData());

		if ( nLength >= m_pData->GetSize() )
			nLength = m_pData->GetSize()-1;

		m_pData->SetLength (nLength);
	}

	bool IsEmpty() const
	{
		return !(m_pData->GetLength() && *m_pData->GetData());
	}


private:

	void FreeData()
	{
		if ( m_pData )
			m_pData->DecRef();
	}

	void SetEmpty()
	{
		m_pData = eus<T>();
		m_pData->AddRef();
	}
};

typedef StringBase<wchar_t, UnicodeTraits> UnicodeString;
typedef StringBase<char, AnsiTraits> AnsiString;

#ifdef UNICODE
	typedef UnicodeString string;
#else
	typedef AnsiString string;
#endif
