#pragma once
#include <windows.h>

#define __US_DELTA 20

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

#include "AsciiStringBase.hpp"
#include "UnicodeStringBase.hpp"

#ifdef UNICODE
	typedef UnicodeString string;
#else
	typedef AsciiStringBaseEx<CP_OEMCP> string;
#endif