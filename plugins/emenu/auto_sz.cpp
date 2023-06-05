#include "auto_sz.h"
#include <cassert>

auto_sz::auto_sz(LPCWSTR sz)
{
  operator =(sz);
}

auto_sz::auto_sz(LPCWSTR szBuf, size_t nBufLen)
{
  nBufLen++;
  Realloc(nBufLen);
  lstrcpyn(m_sz, szBuf, (int)nBufLen);
}

auto_sz::auto_sz(auto_sz& str)
{
  *this=str;
}

auto_sz::auto_sz(const STRRET& sr, LPCITEMIDLIST piid)
{
  switch (sr.uType)
  {
  case STRRET_CSTR:
    {
      size_t len = lstrlenA(sr.cStr)+1;
      Realloc(len);
      MultiByteToWideChar(CP_ACP, 0, sr.cStr, -1, *this, (int)len);
    }
    break;
  case STRRET_WSTR:
    operator=(sr.pOleStr);
    break;
  case STRRET_OFFSET:
    operator=(reinterpret_cast<LPCWSTR>(reinterpret_cast<char const*>(&piid->mkid) + sr.uOffset));
    break;
  default:
    assert(0);
  }
}

auto_sz::~auto_sz()
{
  Clear();
}

auto_sz::operator LPWSTR()
{
  return m_sz;
}

auto_sz::operator LPCWSTR()
{
  return m_sz;
}

auto_sz::operator LPCWSTR() const
{
  return m_sz;
}

auto_sz& auto_sz::operator =(LPCWSTR sz)
{
  if (m_sz==sz) return *this;
  Clear();
  if (!sz) return *this;
  return operator+=(sz);
}

auto_sz& auto_sz::operator =(const auto_sz& str)
{
  if (this!=&str)
  {
    Realloc(str.m_nSize);
    lstrcpy(m_sz, str.m_sz);
  }
  return *this;
}

wchar_t auto_sz::operator[](int nPos)
{
  assert(nPos>=0);
  return m_sz[nPos];
}

void auto_sz::Alloc(size_t nSize)
{
  Clear();
  if (nSize)
  {
    m_sz=new wchar_t[nSize];
    m_sz[0]=L'\0';
    m_bDelete=true;
    m_nSize=nSize;
  }
}

size_t auto_sz::Size() const
{
  return m_nSize;
}

void auto_sz::Clear()
{
  if (m_bDelete)
  {
    delete[] m_sz;
  }
  m_bDelete=false;
  m_sz={};
  m_nSize=0;
}

void auto_sz::Realloc(size_t nNewSize)
{
  if (!nNewSize)
  {
    Clear();
    return;
  }
  LPWSTR sz=new wchar_t[nNewSize];
  if (m_sz)
  {
    lstrcpyn(sz, m_sz, (int)nNewSize);
    Clear();
  }
  else
  {
    sz[0]=L'\0';
  }
  m_sz=sz;
  m_bDelete=true;
  m_nSize=nNewSize;
}

auto_sz& auto_sz::operator +=(LPCWSTR szAdd)
{
  Realloc(lstrlen(m_sz)+lstrlen(szAdd)+1);
  lstrcat(m_sz, szAdd);
  return *this;
}

size_t auto_sz::Len() const
{
  return lstrlen(m_sz);
}

void auto_sz::Trunc(size_t nNewLen)
{
  if (nNewLen<m_nSize) m_sz[nNewLen]=L'\0';
}

bool operator==(const auto_sz& str, LPCWSTR sz)
{
	return lstrcmp(str, sz) == 0;
}

bool operator==(LPCWSTR sz, const auto_sz& str)
{
	return str == sz;
}

bool operator==(const auto_sz& str1, const auto_sz& str2)
{
	return str1 == str2.operator LPCWSTR();
}

int auto_sz::CompareNoCase(LPCWSTR sz)
{
  return lstrcmpi(m_sz, sz);
}

bool auto_sz::CompareExcluding(LPCWSTR sz, wchar_t chExcl)
{
  LPCWSTR szThis=m_sz;
  while (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE
    , (*szThis==chExcl?++szThis:szThis), 1
    , (*sz==chExcl?++sz:sz), 1)==CSTR_EQUAL)
  {
    if (L'\0'==*szThis) return true;
    szThis++;
    sz++;
  }
  return false;
}

void auto_sz::RemoveTrailing(wchar_t chExcl)
{
  LPWSTR szThis=m_sz+Len();
  while (--szThis>m_sz && *szThis==chExcl) *szThis=L'\0';
}

bool auto_sz::IsEmpty()
{
  return !m_sz || L'\0'==m_sz[0];
}
