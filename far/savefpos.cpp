/*
savefpos.cpp

class SaveFilePos

*/

/* Revision: 1.01 08.08.2000 $ */

/*
Modify:
  08.08.2000 tran 1.01
    ! long -> int64
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "int64.hpp"
#include "savefpos.hpp"

SaveFilePos::SaveFilePos(FILE *SaveFile)
{
  long high,low;
  HANDLE h=(HANDLE)_get_osfhandle(_fileno(SaveFile));
  SaveFilePos::SaveFile=SaveFile;
  high=0;
  low=SetFilePointer(h,0,&high,FILE_CURRENT);
  SavePos.HighPart=high;
  SavePos.LowPart=low;
}


SaveFilePos::~SaveFilePos()
{
  long high=SavePos.HighPart;
  HANDLE h=(HANDLE)_get_osfhandle(_fileno(SaveFile));
  SetFilePointer(h,SavePos.LowPart,&high,FILE_BEGIN);
}

