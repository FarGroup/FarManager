/*
UnicodeString.hpp

Unicode ��ப�

*/

/* Revision: 1.03 14.09.2005 $ */

/*
Modify:
  14.09.2005 WARP
    ! Unicode
  22.07.2005 WARP
    ! see 00029.nail.txt
  16.07.2005 WARP
    ! ��筥��� UnicodeString::ReleaseBuffer()
  04.07.2005 WARP
    ! �뤥����� � ����⢥ ᠬ����⥫쭮�� �����
*/
#include "headers.hpp"
#pragma hdrstop

#include "UnicodeString.hpp"

size_t UnicodeString::Inflate(size_t nSize, bool bForce)
{
  if ( (m_pData->GetRef() == 1) || bForce )
  {
    m_pData->Inflate(nSize);
  }
  else
  {
    UnicodeStringData *pNewData = new UnicodeStringData(nSize);

    memcpy(pNewData->GetData(),m_pData->GetData(),m_pData->GetLength() * sizeof(wchar_t));

    pNewData->SetLength(m_pData->GetLength());

    m_pData->DecRef();

    m_pData = pNewData;
  }
  return m_pData->GetSize();
}

size_t UnicodeString::GetCharString(char *lpszStr, size_t nLength, UINT CodePage)
{
  size_t nCopyLength = (nLength <= m_pData->GetLength() ? nLength : m_pData->GetLength());
  WideCharToMultiByte(CodePage,0,m_pData->GetData(),nCopyLength,lpszStr,nCopyLength+1,NULL,NULL);
  lpszStr[nCopyLength] = 0;
  return nCopyLength;
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
  if (m_pData)
    m_pData->DecRef();
  size_t nLength = wcslen(lpwszData);
  m_pData = new UnicodeStringData(nLength + 1);
  wchar_t *pStr = m_pData->GetData();
  if (pStr)
  {
    memcpy(pStr,lpwszData,(nLength + 1) * sizeof(wchar_t));
    m_pData->SetLength(nLength);
  }
  return *this;
}

const UnicodeString& UnicodeString::SetData(const char *lpszData, UINT CodePage)
{
  if (m_pData)
    m_pData->DecRef();

  size_t nSize = MultiByteToWideChar(CodePage,0,lpszData,-1,NULL,0);
  m_pData = new UnicodeStringData(nSize);
  MultiByteToWideChar(CodePage,0,lpszData,nSize,m_pData->GetData(),m_pData->GetSize()/sizeof(wchar_t));
  m_pData->SetLength(nSize - 1);
  return *this;
}

const UnicodeString& UnicodeString::Append(const UnicodeString &strAdd)
{
  UnicodeStringData *pAddData = strAdd.m_pData;
  size_t nNewLength = m_pData->GetLength() + pAddData->GetLength();
  Inflate(nNewLength + 1);
  memcpy(m_pData->GetData() + m_pData->GetLength(),pAddData->GetData(),pAddData->GetLength() * sizeof(wchar_t));
  m_pData->SetLength(nNewLength);
  return *this;
}

const UnicodeString& UnicodeString::Append(const wchar_t *lpwszAdd)
{
  size_t nAddLength = wcslen(lpwszAdd);
  size_t nNewLength = m_pData->GetLength() + nAddLength;
  Inflate(nNewLength + 1);
  memcpy(m_pData->GetData() + m_pData->GetLength(),lpwszAdd,nAddLength * sizeof(wchar_t));
  m_pData->SetLength(nNewLength);
  return *this;
}

const UnicodeString& UnicodeString::Append(const char *lpszAdd, UINT CodePage)
{
  size_t nAddSize = MultiByteToWideChar(CodePage,0,lpszAdd,-1,NULL,0);
  size_t nNewLength = m_pData->GetLength() + nAddSize - 1;
  Inflate(nNewLength + 1);
  MultiByteToWideChar(CodePage,0,lpszAdd,nAddSize,m_pData->GetData() + m_pData->GetLength(),m_pData->GetSize()/sizeof(wchar_t));
  m_pData->SetLength(nNewLength);
  return *this;
}

const UnicodeString& UnicodeString::operator=(const UnicodeString &strCopy)
{
  return SetData(strCopy);
}

const UnicodeString& UnicodeString::operator=(const char *lpszData)
{
  return SetData(lpszData);
}

const UnicodeString& UnicodeString::operator=(const wchar_t *lpwszData)
{
  return SetData(lpwszData);
}

const UnicodeString& UnicodeString::operator+=(const UnicodeString &strAdd)
{
  return Append(strAdd);
}

const UnicodeString& UnicodeString::operator+=(const char *lpszAdd)
{
  return Append(lpszAdd);
}

const UnicodeString& UnicodeString::operator+=(const wchar_t *lpwszAdd)
{
  return Append(lpwszAdd);
}

const UnicodeString operator+(const UnicodeString &strSrc1, const UnicodeString &strSrc2)
{
  UnicodeString Result(strSrc1);
  Result += strSrc2;
  return Result;
}

const UnicodeString operator+(const UnicodeString &strSrc1, const char *lpszSrc2)
{
  UnicodeString Result(strSrc1);
  Result += lpszSrc2;
  return Result;
}

const UnicodeString operator+(const UnicodeString &strSrc1, const wchar_t *lpwszSrc2)
{
  UnicodeString Result(strSrc1);
  Result += lpwszSrc2;
  return Result;
}

wchar_t *UnicodeString::GetBuffer (int nLength)
{
    Inflate (nLength == -1?m_pData->GetSize():nLength); //??force??

    return m_pData->GetData ();
}

void UnicodeString::ReleaseBuffer (int nLength)
{
    if ( nLength != -1 )
        m_pData->SetLength (nLength);
    else
        m_pData->SetLength (wcslen(m_pData->GetData()));
}
