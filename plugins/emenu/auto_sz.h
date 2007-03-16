#ifndef _AUTO_SZ_H_
#define _AUTO_SZ_H_

#include "shlobj.h"

class auto_sz
{
public:
  auto_sz();
  auto_sz(LPCTSTR sz);
  auto_sz(LPCTSTR szBuf, size_t nBufLen);
#ifndef UNICODE
  auto_sz(LPCWSTR szw);
#endif
  auto_sz(auto_sz& str);
  auto_sz(const STRRET& sr, LPCITEMIDLIST piid);
  ~auto_sz();
  operator LPTSTR();
  operator LPCTSTR();
  operator LPCTSTR() const;
  auto_sz& operator =(LPCTSTR sz);
#ifndef UNICODE
  auto_sz& operator =(LPCWSTR szw);
#endif
  auto_sz& operator =(const auto_sz& str);
  void Alloc(size_t nSize);
  size_t Size() const;
  void Clear();
  auto_sz& operator +=(LPCTSTR szAdd);
  void Realloc(size_t nNewSize);
  size_t Len() const;
#ifndef UNICODE
  auto_sz& Ansi2Oem();
  auto_sz& Oem2Ansi();
  operator LPOLESTR();
#endif
  operator void*();
  TCHAR operator[](int nPos);
  void Trunc(size_t nNewLen);
#ifndef UNICODE
  static void SetOem() {s_bOem=true;}
#endif
  bool operator ==(LPCTSTR sz);
  bool operator !=(LPCTSTR sz);
  bool CompareExcluding(LPCTSTR sz, TCHAR chExcl);
  void RemoveTrailing(TCHAR chExcl);
  int CompareNoCase(LPCTSTR sz);
  bool IsEmpty();
protected:
  LPTSTR m_sz;
  bool m_bDelete;
  size_t m_nSize;
#ifndef UNICODE
  WCHAR* m_wsz;
  static bool s_bOem;
#endif
};

#endif
