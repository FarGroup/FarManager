#ifndef __GETFILESTRING_HPP__
#define __GETFILESTRING_HPP__
/*
filestr.hpp

Класс GetFileString

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#ifndef __STDIO_H
#include <stdio.h>
#endif

class GetFileString
{
  private:
    char ReadBuf[8192];
    int ReadPos,ReadSize;
    char *Str;
    int StrLength;
    FILE *SrcFile;
  public:
    GetFileString(FILE *SrcFile);
    ~GetFileString();
    int GetString(char **DestStr,int &Length);
};

#endif	// __GETFILESTRING_HPP__

