/*
chgprior.cpp

class ChangePriority

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#define STRICT

#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
#include <windows.h>
#endif

#ifndef __CHANGEPRIORITY_HPP__
#include "chgprior.hpp"
#endif


ChangePriority::ChangePriority(int NewPriority)
{
  SavePriority=GetThreadPriority(GetCurrentThread());
  SetThreadPriority(GetCurrentThread(),NewPriority);
}


ChangePriority::~ChangePriority()
{
  SetThreadPriority(GetCurrentThread(),SavePriority);
}


