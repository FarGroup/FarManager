/*
savefpos.cpp

class SaveFilePos

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

#ifndef __SAVEFILEPOS_HPP__
#include "savefpos.hpp"
#endif

SaveFilePos::SaveFilePos(FILE *SaveFile)
{
  SaveFilePos::SaveFile=SaveFile;
  SavePos=ftell(SaveFile);
}


SaveFilePos::~SaveFilePos()
{
  fseek(SaveFile,SavePos,SEEK_SET);
}

