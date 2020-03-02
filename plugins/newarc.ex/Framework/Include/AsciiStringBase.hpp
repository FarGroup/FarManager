#pragma once
#include "StringBase.hpp"
#include <stdio.h>

#define SAME_ENCODING -1

class AsciiStringBase
{
protected:

	int m_nEncoding;
	StringData<char>* m_pData;

public:

	AsciiStringBase(int nEncoding)
	{
		m_nEncoding = nEncoding;
		SetEmpty();
	}

	AsciiStringBase(int nEncoding, const char* lpData, int nSrcEncoding = SAME_ENCODING)
	{
		m_nEncoding = nEncoding;
		m_pData = nullptr;
		SetData(lpData, nSrcEncoding);
	}

	AsciiStringBase(int nEncoding, const wchar_t* lpData)
	{
		m_nEncoding = nEncoding;
		m_pData = nullptr;
		SetData(lpData);
	}

	AsciiStringBase(int nEncoding, const AsciiStringBase& strCopy)
	{
		m_nEncoding = nEncoding;
		m_pData = NULL;
		SetData(strCopy);
	}

	AsciiStringBase& SetData(const AsciiStringBase& strCopy)
	{
		if ( strCopy.m_pData != m_pData )
		{
			if ( strCopy.m_nEncoding == m_nEncoding )
			{
				FreeData();

				m_pData = strCopy.m_pData;
				m_pData->AddRef();
			}
			else
				SetData(strCopy.GetString(), strCopy.GetLength(), strCopy.GetEncoding()); //convert
		}

		return *this;
	}

	int GetEncoding() const
	{
		return m_nEncoding;
	}
	////1

	AsciiStringBase& SetData(const wchar_t* lpData, size_t nLength)
	{
		if ( !lpData || !nLength )
		{
			FreeData();
			SetEmpty();
		}
		else
		{
			size_t nSize = WideCharToMultiByte(m_nEncoding, 0, lpData, nLength, 0, 0, 0, 0);

			if ( m_pData && m_pData->Unique() && (nSize < m_pData->GetSize()) )
			{
				WideCharToMultiByte(m_nEncoding, 0, lpData, nLength, m_pData->GetData(), nSize, 0, 0);
				m_pData->SetLength(nSize);
			}
			else
			{
				StringData<char>* pNewData = new StringData<char>(nSize+1);

				WideCharToMultiByte(m_nEncoding, 0, lpData, nLength, pNewData->GetData(), nSize, 0, 0);
				pNewData->SetLength(nSize);

				FreeData();
				m_pData = pNewData;
			}
		}

		return *this;
	}



	//2

	AsciiStringBase& SetData(const char* lpData, size_t nLength, int nSrcEncoding = SAME_ENCODING)
	{
		if ( nSrcEncoding == SAME_ENCODING )
			nSrcEncoding = m_nEncoding;

		if ( !lpData || !nLength )
		{
			FreeData();
			SetEmpty();
		}
		else
		{
			char* pBuffer = nullptr;
			size_t nBufferSize = 0;

			bool bConverted = ConvertEncoding(lpData, nLength, &pBuffer, &nBufferSize, nSrcEncoding, m_nEncoding);

			if ( !bConverted )
			{
				pBuffer = (char*)lpData;
				nBufferSize = nLength;
			}

			if ( m_pData && m_pData->Unique() && (nBufferSize < m_pData->GetSize()) )
			{
				memmove(m_pData->GetData(), pBuffer, nBufferSize);
				m_pData->SetLength(nBufferSize);
			}
			else
			{
				StringData<char>* pNewData = new StringData<char>(nBufferSize+1);

				memmove(pNewData->GetData(), pBuffer, nBufferSize);
				pNewData->SetLength(nBufferSize);

				FreeData();
				m_pData = pNewData;
			}

			if ( bConverted )
				delete [] pBuffer;
		}

		return *this;
	}

	AsciiStringBase& SetData(const char* lpData, int nSrcEncoding = SAME_ENCODING)
	{
		return SetData(lpData, lpData?strlen(lpData):0, nSrcEncoding);
	}

	AsciiStringBase& SetData(const wchar_t* lpData)
	{
		return SetData(lpData, lpData?wcslen(lpData):0);
	}

	AsciiStringBase& SetData(wchar_t Ch)
	{
		return SetData(&Ch, 1);
	}

	AsciiStringBase& SetData(char Ch, int nSrcEncoding = SAME_ENCODING)
	{
		return SetData(&Ch, 1, nSrcEncoding);
	}

	AsciiStringBase& Append(const char* lpData, int nSrcEncoding = SAME_ENCODING)
	{
		return Append(lpData, lpData?strlen(lpData):0, nSrcEncoding);
	}

	AsciiStringBase& Append(const wchar_t* lpData)
	{
		return Append(lpData, lpData?wcslen(lpData):0);
	}

	AsciiStringBase& Append(const AsciiStringBase& strAdd)
	{
		return Append(strAdd.GetString(), strAdd.GetLength(), strAdd.GetEncoding());
	}

	AsciiStringBase& Append(wchar_t Ch)
	{
		return Append(&Ch, 1);
	}

	AsciiStringBase& Append(char Ch, int nSrcEncoding = SAME_ENCODING)
	{
		return Append(&Ch, 1, nSrcEncoding);
	}

	AsciiStringBase& Append(const char* lpData, size_t nLength, int nSrcEncoding = SAME_ENCODING)
	{
		if ( !lpData || !nLength ) //*lpData??
			return *this;

		if ( nSrcEncoding == SAME_ENCODING )
			nSrcEncoding = m_nEncoding;

		char* pBuffer = nullptr;
		size_t nBufferSize = 0;

		bool bConverted = ConvertEncoding(lpData, nLength, &pBuffer, &nBufferSize, nSrcEncoding, m_nEncoding);

		if ( !bConverted )
		{
			pBuffer = (char*)lpData;
			nBufferSize = nLength;
		}

		size_t nSize = nBufferSize+m_pData->GetLength();

		if ( m_pData->Unique() && (nSize < m_pData->GetSize()) ) //check!!!
		{
			memmove(m_pData->GetData()+m_pData->GetLength(), pBuffer, nBufferSize);
			m_pData->SetLength(nSize);
		}
		else
		{
			StringData<char>* pNewData = new StringData<char>(nSize+1);

			memcpy(pNewData->GetData(), m_pData->GetData(), m_pData->GetLength());
			memmove(pNewData->GetData()+m_pData->GetLength(), pBuffer, nBufferSize);

			pNewData->SetLength(nSize);
			FreeData();

			m_pData = pNewData;
		}

		if ( bConverted )
			delete [] pBuffer;

		return *this;
	}

	AsciiStringBase& Append(const wchar_t* lpData, size_t nLength)
	{
		if ( !lpData || !nLength ) //*lpData??
			return *this;

		size_t nNewLength = WideCharToMultiByte(m_nEncoding, 0, lpData, nLength, 0, 0, 0, 0);
		size_t nSize = nNewLength+m_pData->GetLength();

		if ( m_pData->Unique() && (nSize < m_pData->GetSize()) ) //check!!!
		{
			WideCharToMultiByte(m_nEncoding, 0, lpData, nLength, m_pData->GetData()+m_pData->GetLength(), nNewLength, 0, 0);
			m_pData->SetLength(nSize);
		}
		else
		{
			StringData<char>* pNewData = new StringData<char>(nSize+1);

			memcpy(pNewData->GetData(), m_pData->GetData(), m_pData->GetLength());
			WideCharToMultiByte(m_nEncoding, 0, lpData, nLength, pNewData->GetData()+m_pData->GetLength(), nNewLength, 0, 0);

			pNewData->SetLength(nSize);
			FreeData();

			m_pData = pNewData;
		}

		return *this;
	}

	AsciiStringBase& operator=(const wchar_t* lpData)
	{
		return SetData(lpData);
	}

	AsciiStringBase& operator=(const char* lpData)
	{
		return SetData(lpData);
	}

	AsciiStringBase& operator=(const AsciiStringBase& strCopy)
	{
		return SetData(strCopy);
	}

	AsciiStringBase operator+(const char* lpData)
	{
		AsciiStringBase strResult = *this;
		return strResult.Append(lpData);
	}

	AsciiStringBase operator+(const AsciiStringBase& strAdd)
	{
		AsciiStringBase strResult = *this;
		return strResult.Append(strAdd);
	}

	AsciiStringBase operator+(char Ch)
	{
		AsciiStringBase strResult = *this;
		return strResult.Append(Ch);
	}

	AsciiStringBase& operator+=(const char* lpData)
	{
		return Append(lpData);
	}

	AsciiStringBase& operator+=(char Ch)
	{
		return Append(Ch);
	}

	AsciiStringBase& operator+=(const AsciiStringBase& strAdd)
	{
		return Append(strAdd);
	}

	bool operator==(const char* lpCmp)
	{
		return (lpCmp && (strlen(lpCmp) == GetLength()) && !memcmp(lpCmp, m_pData->GetData(), GetLength()));
	}

	const char* GetString() const
	{
		return m_pData->GetData();
	}

	size_t GetLength() const
	{
		return m_pData->GetLength();
	}

	operator const char*() const
	{
		return m_pData->GetData();
	}

	char At(size_t nIndex) const
	{
		return m_pData->GetData()[nIndex];
	}

	char operator[](size_t nIndex) const
	{
		return At(nIndex);
	}

	bool operator==(const AsciiStringBase& strCmp)
	{
		return (strCmp.m_pData == m_pData) ||
				(
					(strCmp.GetLength() == GetLength()) &&
					!memcmp(m_pData->GetData(), strCmp.m_pData->GetData(), GetLength())
				);
	}

	bool operator!=(const AsciiStringBase& strCmp)
	{
		return !(*this == strCmp);
	}

	int __cdecl Format(const char* format, ...)
	{
		char* buffer = NULL;
		size_t Size = MAX_PATH;

		int retValue = -1;
		va_list argptr;

		va_start(argptr, format);

		do
		{
			Size <<= 1;
			char* tmpbuffer = (char*)realloc(buffer, Size*sizeof(char));

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
			retValue = _vsnprintf(buffer, Size-1, format, argptr);

		} while (retValue == -1);

		va_end(argptr);

		SetData(buffer);

		free(buffer);

		return retValue;
	}

	char* GetBuffer(size_t nLength = (size_t)-1)
	{
		size_t nNewLength = (nLength == (size_t)-1)?m_pData->GetLength():nLength;

		if ( !m_pData->Unique() || (nNewLength > m_pData->GetSize()) )
		{
			StringData<char>* pNewData = new StringData<char>(nNewLength+1);

			memcpy(pNewData->GetData(), m_pData->GetData(), m_pData->GetLength());
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
			nLength = strlen(m_pData->GetData());

		if ( nLength >= m_pData->GetSize() )
			nLength = m_pData->GetSize()-1;

		m_pData->SetLength (nLength);
	}

	bool IsEmpty() const
	{
		return !(m_pData->GetLength() && *m_pData->GetData());
	}

	virtual ~AsciiStringBase()
	{
		FreeData();
	}


private:

	//private
	static bool ConvertEncoding(
			const char* lpData,
			size_t nLength,
			char** lpBuffer,
			size_t* pBufferSize,
			int nSrcEncoding,
			int nDestEncoding
			)
	{
		if ( nSrcEncoding != nDestEncoding )
		{
			size_t nUL = MultiByteToWideChar(nSrcEncoding, 0, lpData, nLength, 0, 0);

			wchar_t* lpTemp = new wchar_t[nUL];

			MultiByteToWideChar(nSrcEncoding, 0, lpData, nLength, lpTemp, nUL);

			size_t nSize = WideCharToMultiByte(nDestEncoding, 0, lpTemp, nUL, 0, 0, 0, 0);

			if ( lpBuffer )
			{
				*lpBuffer = new char[nSize];

				WideCharToMultiByte(nDestEncoding, 0, lpTemp, nUL, *lpBuffer, nSize, 0, 0);

				if ( pBufferSize )
					*pBufferSize = nSize;
					
				return true;
			}
		}

		return false;
	}

	void FreeData()
	{
		if ( m_pData )
			m_pData->DecRef();
	}

	void SetEmpty()
	{
		m_pData = eus<char>();
		m_pData->AddRef();
	}
};


template<int E>
class AsciiStringBaseEx : public AsciiStringBase
{
public:
	AsciiStringBaseEx() : AsciiStringBase(E) {};
	AsciiStringBaseEx(const char* lpData, int nSrcEncoding = SAME_ENCODING) : AsciiStringBase(E, lpData, nSrcEncoding) {};
	AsciiStringBaseEx(const wchar_t* lpData) : AsciiStringBase(E, lpData) {};
	AsciiStringBaseEx(const AsciiStringBase& strCopy) : AsciiStringBase(E, strCopy) {};
};

typedef AsciiStringBaseEx<CP_OEMCP> OemString;
typedef AsciiStringBaseEx<CP_ACP> AnsiString;
typedef AsciiStringBaseEx<CP_UTF8> FakeUtf8String;
