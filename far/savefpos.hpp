#ifndef __SAVEFILEPOS_HPP__
#define __SAVEFILEPOS_HPP__
/*
savefpos.hpp

class SaveFilePos

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class SaveFilePos
{
  private:
    FILE *SaveFile;
    long SavePos;
  public:
    SaveFilePos(FILE *SaveFile);
    ~SaveFilePos();
};


#endif	// __SAVEFILEPOS_HPP__

