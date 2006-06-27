#ifndef __CRITICALSECTIONS_HPP__
#define __CRITICALSECTIONS_HPP__
/*
CriticalSections.hpp

Критические секции

*/

/* Revision: 1.00 08.12.2004 $ */

/*
Modify:
  08.12.2004 WARP
    ! Выделение в качестве самостоятельного модуля
*/

#pragma once

class CriticalSection
{
  CRITICAL_SECTION _object;
public:
  CriticalSection() { ::InitializeCriticalSection(&_object); }
  ~CriticalSection() { ::DeleteCriticalSection(&_object); }
  void Enter() { ::EnterCriticalSection(&_object); }
  void Leave() { ::LeaveCriticalSection(&_object); }
};

class CriticalSectionLock
{
  CriticalSection &_object;
  void Unlock()  { _object.Leave(); }
public:
  CriticalSectionLock(CriticalSection &object): _object(object)
    {_object.Enter(); }
  ~CriticalSectionLock() { Unlock(); }
};

#endif  // __CRITICALSECTIONS_HPP__
