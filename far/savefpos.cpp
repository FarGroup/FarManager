/*
savefpos.cpp

class SaveFilePos

*/

/* Revision: 1.04 06.05.2001 $ */

/*
Modify:
  06.05.2001 DJ
    ! перетрях #include
  20.02.2001 SVS
    ! Заголовки - к общему виду!
  09.08.2000 svs
    ! revert.  int64 -> long
  08.08.2000 tran 1.01
    ! long -> int64
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "savefpos.hpp"

SaveFilePos::SaveFilePos(FILE *SaveFile)
{
  SaveFilePos::SaveFile=SaveFile;
  SavePos=ftell(SaveFile);
}


SaveFilePos::~SaveFilePos()
{
  fseek(SaveFile,SavePos,SEEK_SET);
}
