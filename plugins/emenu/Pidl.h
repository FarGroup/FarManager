// CPidl - class to store one or more LPITEMIDLIST

#ifndef _PIDL_H_
#define _PIDL_H_

#include "shlobj.h"

class CPidl
{
public:
  CPidl();
  ~CPidl();
  operator LPITEMIDLIST();
  LPCITEMIDLIST* GetArray();
  LPITEMIDLIST* operator &();
  LPITEMIDLIST GetAt(unsigned nIndex);
  void Realloc(unsigned nNewSize);
  void EnsureSpace();
  void Add(LPITEMIDLIST pidl);
  void Insert(unsigned nIndex, LPITEMIDLIST pidl);
  unsigned Count();
protected:
  CPidl(const CPidl&);
  CPidl& operator =(const CPidl&);
  LPITEMIDLIST* m_ppidl;
  unsigned m_nSize;
  unsigned m_nCount;
};

#endif
