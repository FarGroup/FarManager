/*
lockscrn.cpp

class LockScreen

*/

/* Revision: 1.01 06.05.2001 $ */

/*
Modify:
  06.05.2001 DJ
    ! �������� #include
  25.06.2000 SVS
    ! ���������� Master Copy
    ! ��������� � �������� ���������������� ������
*/

#include "headers.hpp"
#pragma hdrstop

#include "lockscrn.hpp"
#include "scrbuf.hpp"

LockScreen::LockScreen()
{
  ScrBuf.Lock();
}


LockScreen::~LockScreen()
{
  ScrBuf.Unlock();
  ScrBuf.Flush();
}
