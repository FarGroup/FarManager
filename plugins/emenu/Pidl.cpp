#include "Pidl.h"
#include <cassert>

CPidl::CPidl()
  : m_ppidl(NULL)
  , m_nSize(0)
  , m_nCount(0)
{
}

CPidl::~CPidl()
{
  for (unsigned i=0; i<m_nCount; i++)
  {
    CoTaskMemFree(m_ppidl[i]);
  }
  delete[] m_ppidl;
}

CPidl::operator LPITEMIDLIST()
{
  if (m_nSize)
  {
    return *m_ppidl;
  }
  else
  {
    return NULL;
  }
}

void CPidl::Realloc(unsigned nNewSize)
{
  LPITEMIDLIST* ppidlNew=new LPITEMIDLIST[nNewSize];
  memcpy(ppidlNew, m_ppidl, sizeof(LPITEMIDLIST)*m_nCount);
  delete[] m_ppidl;
  m_ppidl=ppidlNew;
  m_nSize=nNewSize;
}

void CPidl::EnsureSpace()
{
  assert(m_nCount<=m_nSize);
  if (m_nCount==m_nSize)
  {
    unsigned nNewSize=(m_nSize+1)*2;
    Realloc(nNewSize);
  }
}

LPCITEMIDLIST* CPidl::GetArray()
{
  if (!m_nSize)
  {
    EnsureSpace();
  }
  return const_cast<LPCITEMIDLIST*>(m_ppidl);
}

LPITEMIDLIST* CPidl::operator &()
{
  if (!m_nSize)
  {
    EnsureSpace();
  }
  return m_ppidl;
}

void CPidl::Add(LPITEMIDLIST pidl)
{
  Insert(m_nCount, pidl);
}

void CPidl::Insert(unsigned nIndex, LPITEMIDLIST pidl)
{
  EnsureSpace();
  if (nIndex>=m_nSize|| nIndex>m_nCount)
  {
    assert(0);
    return;
  }
  memmove(m_ppidl+nIndex+1, m_ppidl+nIndex, sizeof(*m_ppidl)*(m_nCount-nIndex));
  m_nCount++;
  m_ppidl[nIndex]=pidl;
}

LPITEMIDLIST CPidl::GetAt(unsigned nIndex)
{
  if (nIndex>=m_nCount)
  {
    return NULL;
  }
  return m_ppidl[nIndex];
}

unsigned CPidl::Count()
{
  return m_nCount;
}
