#ifndef __LASTERROR_HPP__
#define __LASTERROR_HPP__
/*
lasterror.hpp

��������/�������������� LastError

*/

/* Revision: 1.0 17.05.2004 $ */

/*
Modify:
  17.05.2004 SVS
    + �������� � ������
*/

#include "headers.hpp"
#pragma hdrstop

class GuardLastError
{
  private:
    DWORD LastError;
  public:
    GuardLastError(){LastError=GetLastError();}
    ~GuardLastError(){SetLastError(LastError);}
    DWORD Get(){return LastError;}
};

#endif  // __LASTERROR_HPP__
