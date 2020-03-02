#ifndef _HANDLE_H_
#define _HANDLE_H_

#include <windows.h>
#include <cassert>

class CHandle
{
public:
  CHandle(HANDLE h=NULL) : m_h(h) {}
  ~CHandle()
  {
    if (m_h && !CloseHandle(m_h))
    {
      assert(0);
    }
  }
  bool operator !() {return (NULL==m_h);}
  operator HANDLE() {return m_h;}
  CHandle& operator =(HANDLE h) {m_h=h; return *this;}
protected:
  HANDLE m_h;
};

#endif
