#ifndef __PRESERVELONGNAME_HPP__
#define __PRESERVELONGNAME_HPP__
/*
plognmn.hpp

class PreserveLongName

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

#include "farconst.hpp"

class PreserveLongName
{
  private:
    char SaveLongName[NM],SaveShortName[NM];
    int Preserve;
  public:
    PreserveLongName(char *ShortName,int Preserve);
    ~PreserveLongName();
};



#endif	// __PRESERVELONGNAME_HPP__
