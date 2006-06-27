/*
chgprior.cpp

class ChangePriority

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! ���������� Master Copy
    ! ��������� � �������� ���������������� ������
*/

#include "headers.hpp"
#pragma hdrstop

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
