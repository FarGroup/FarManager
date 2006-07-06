#ifndef __UNICODESTRING_HPP__
#define __UNICODESTRING_HPP__
/*
UnicodeString.hpp

Unicode ��ப�

*/

/* Revision: 1.08 07.07.2006 $ */

#define string UnicodeString

#define __US_DELTA 512

class UnicodeStringData {
private:
  size_t m_nLength;
  size_t m_nSize;
  size_t m_nDelta;
  int m_nRefCount;
  wchar_t *m_pData;

  wchar_t *AllocData(size_t nSize, size_t *nNewSize)
  {
    *nNewSize = (nSize/m_nDelta + 1) * m_nDelta;
    if (nSize >= m_nDelta << 3)
      *nNewSize = (*nNewSize) << 1;
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
      m_pData[0] = 0;
    else
      m_nSize=0;
  }

  size_t SetLength(size_t nLength)
  {
    if (nLength<m_nSize)
    {
      m_nLength = nLength;
      m_pData[m_nLength] = 0;
    }
    return m_nLength;
  }

  size_t Inflate(size_t nSize)
  {
    if (nSize <= m_nSize)
      return m_nSize;
    wchar_t *pOldData = m_pData;
    size_t nOldSize = m_nSize;
    m_pData = AllocData(nSize,&m_nSize);
    if (!m_pData)
    {
      m_pData = pOldData;
      m_nSize = nOldSize;
    }
    else
    {
      if (pOldData)
        memcpy(m_pData,pOldData,m_nLength * sizeof(wchar_t));
      m_pData[m_nLength] = 0;
      FreeData(pOldData);
    }
    return m_nSize;
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

  wchar_t *m_lpwszBuffer;

  void DeleteData()
  {
    if (m_pData)
      m_pData->DecRef();
  }

public:
  UnicodeString(size_t nSize=0, size_t nDelta=0)
  {
    m_pData = new UnicodeStringData(nSize, nDelta);
  }

  const UnicodeString& SetData(const UnicodeString &strCopy);
  const UnicodeString& SetData(const wchar_t *lpwszData);
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

  UnicodeString(const char *lpszData, UINT CodePage=CP_OEMCP)
  {
    m_pData = NULL;
    SetData(lpszData,CodePage);
  }

  ~UnicodeString()
  {
    DeleteData();
  }

  size_t Inflate(size_t nSize, bool bForce = false);

  size_t GetLength() const
  {
    return m_pData->GetLength();
  }

  size_t GetCharString(char *lpszStr, size_t nLength, UINT CodePage=CP_OEMCP);

  const UnicodeString& Append(const UnicodeString &strAdd);
  const UnicodeString& Append(const wchar_t *lpwszAdd);
  const UnicodeString& Append(const char *lpszAdd, UINT CodePage=CP_OEMCP);

  operator const wchar_t *() const
  {
    return m_pData->GetData();
  }

  const UnicodeString& operator=(const UnicodeString &strCopy);
  const UnicodeString& operator=(const char *lpszData);
  const UnicodeString& operator=(const wchar_t *lpwszData);

  const UnicodeString& operator+=(const UnicodeString &strAdd);
  const UnicodeString& operator+=(const char *lpszAdd);
  const UnicodeString& operator+=(const wchar_t *lpwszAdd);

  friend const UnicodeString operator+(const UnicodeString &strSrc1, const UnicodeString &strSrc2);
  friend const UnicodeString operator+(const UnicodeString &strSrc1, const char *lpszSrc2);
  friend const UnicodeString operator+(const UnicodeString &strSrc1, const wchar_t *lpwszSrc2);

  wchar_t *GetBuffer (int nLength = -1);
  void ReleaseBuffer (int nLength = -1);

  wchar_t At (int nIndex)
  {
    const wchar_t *lpwszData = m_pData->GetData ();
    return lpwszData[nIndex];
  }

  bool IsEmpty ()
  {
    return !(m_pData->GetLength () && *m_pData->GetData ());
  }

  void Lower ()
  {
    Inflate (m_pData->GetSize());
    //_wcslwr (m_pData->GetData());
    CharLowerW (m_pData->GetData());
  }

  void Upper ()
  {
    Inflate (m_pData->GetSize());
    CharUpperW (m_pData->GetData());
    //_wcsupr (m_pData->GetData());
  }

  void RShift (unsigned int nNewPos)
  {
    if ( nNewPos > m_pData->GetLength () )
        nNewPos = m_pData->GetLength ();

    Inflate (m_pData->GetSize());
    memmove (m_pData->GetData()+nNewPos, m_pData->GetData(), (m_pData->GetLength()-nNewPos+1)*sizeof (wchar_t));
  }

  void LShift (unsigned int nNewPos)
  {
    if ( nNewPos > m_pData->GetLength () )
        nNewPos = m_pData->GetLength ();

    Inflate (m_pData->GetSize());
    memmove (m_pData->GetData(), m_pData->GetData()+nNewPos, (m_pData->GetLength()-nNewPos+1)*sizeof (wchar_t));
  }

  int __cdecl Format (const wchar_t * format, ...)
  {
    wchar_t *buffer = NULL;
    DWORD dwSize = MAX_PATH;

    int retValue;
    va_list argptr;

    va_start( argptr, format );

    do {
        dwSize <<= 1;
        buffer = (wchar_t*)realloc (buffer, dwSize*sizeof (wchar_t));

        retValue = _vsnwprintf ( buffer, dwSize, format, argptr );
    } while ( retValue == -1 );

    va_end( argptr );

    SetData (buffer);

    free (buffer);

    return retValue;
  }

};

#endif // __UNICODESTRING_HPP__
