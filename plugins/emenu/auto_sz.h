#ifndef _AUTO_SZ_H_
#define _AUTO_SZ_H_

#include <shlobj.h>

// BUGBUG remove & use string
class auto_sz
{
public:
  auto_sz() = default;
  auto_sz(LPCWSTR sz);
  auto_sz(LPCWSTR szBuf, size_t nBufLen);
  auto_sz(auto_sz& str);
  auto_sz(const STRRET& sr, LPCITEMIDLIST piid);
  ~auto_sz();
  operator LPWSTR();
  operator LPCWSTR();
  operator LPCWSTR() const;
  auto_sz& operator =(LPCWSTR sz);
  auto_sz& operator =(const auto_sz& str);
  void Alloc(size_t nSize);
  size_t Size() const;
  void Clear();
  auto_sz& operator +=(LPCWSTR szAdd);
  void Realloc(size_t nNewSize);
  size_t Len() const;
  wchar_t operator[](int nPos);
  void Trunc(size_t nNewLen);
  friend bool operator==(const auto_sz& str, LPCWSTR sz);
  friend bool operator==(LPCWSTR sz, const auto_sz& str);
  friend bool operator==(const auto_sz& str1, const auto_sz& str2);
  bool CompareExcluding(LPCWSTR sz, wchar_t chExcl);
  void RemoveTrailing(wchar_t chExcl);
  int CompareNoCase(LPCWSTR sz);
  bool IsEmpty();
protected:
  LPWSTR m_sz{};
  bool m_bDelete{};
  size_t m_nSize{};
};

#endif
