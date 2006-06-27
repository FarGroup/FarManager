#ifndef __SAVEFILEPOS_HPP__
#define __SAVEFILEPOS_HPP__
/*
savefpos.hpp

class SaveFilePos

*/

/* Revision: 1.03 22.02.2002 $ */

/*
Modify:
  22.02.2002 SVS
    ! revert.  long -> int64 :-\
  09.08.2000 SVS
    ! revert.  int64 -> long
  08.08.2000 tran 1.01
    ! long -> int64
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class SaveFilePos
{
  private:
    FILE *SaveFile;
    __int64 SavePos;
  public:
    SaveFilePos(FILE *SaveFile);
    ~SaveFilePos();
};


#endif	// __SAVEFILEPOS_HPP__
