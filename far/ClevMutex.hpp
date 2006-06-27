#ifndef __CLEVMUTEX_HPP__
#define __CLEVMUTEX_HPP__
/*
ClevMutex.hpp

Мутексы.

*/

/* Revision: 1.01 05.07.2003 $ */

/*
Modify:
  05.07.2003 SVS
    ! DOS -> ANSI (комментарии)
  20.06.2003 SVS
    ! Выделение в качестве самостоятельного модуля
*/

class ClevMutex{
  private:
    HANDLE hMutex;
    BOOL   Locked;

  public:
    ClevMutex(HANDLE hMutex);
    ~ClevMutex();

  public:
    void Lock();
    void Unlock();
};


inline ClevMutex::ClevMutex(HANDLE hMutex0)
{
  InterlockedExchange((LPLONG)&hMutex,(LONG)hMutex0);
  Locked=FALSE;
  Lock();
}

inline ClevMutex::~ClevMutex()
{
  Unlock();
}

inline void ClevMutex::Lock()
{
  if(Locked)
    ReleaseMutex(hMutex);
  WaitForSingleObject(hMutex,INFINITE);
  Locked=TRUE;
}

inline void ClevMutex::Unlock()
{
  if(Locked)
    ReleaseMutex(hMutex);
  Locked=FALSE;
}


#endif	// __CLEVMUTEX_HPP__
