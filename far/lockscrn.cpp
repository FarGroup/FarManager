/*
lockscrn.cpp

class LockScreen

*/

/* Revision: 1.01 06.05.2001 $ */

/*
Modify:
  06.05.2001 DJ
    ! перетрях #include
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "lockscrn.hpp"
#include "global.hpp"

LockScreen::LockScreen()
{
  ScrBuf.Lock();
}


LockScreen::~LockScreen()
{
  ScrBuf.Unlock();
  ScrBuf.Flush();
}
