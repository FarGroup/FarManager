/*
lockscrn.cpp

class LockScreen

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */

LockScreen::LockScreen()
{
  ScrBuf.Lock();
}


LockScreen::~LockScreen()
{
  ScrBuf.Unlock();
  ScrBuf.Flush();
}

