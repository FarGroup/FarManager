#ifndef __NAMELIST_HPP__
#define __NAMELIST_HPP__
/*
namelist.hpp

Список имен файлов, используется в viewer при нажатии Gray+/Gray-

*/

/* Revision: 1.02 14.10.2003 $ */

/*
Modify:
  14.10.2003 SVS
    ! Перетрях в NamesList.
    ! NamesList::GetCurDir - имеет доп. параметр - требуемый размер.
    + NamesList::Init()
  06.05.2001 DJ
    ! перетрях #include
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "farconst.hpp"

class NamesList
{
  private:
    char *Names;
    char *CurName;
    int CurNamePos;
    int NamesNumber;
    DWORD NamesSize;
    DWORD MemSize;

    char CurDir[NM];

  private:
    void Init();

  public:
    NamesList(DWORD MemSize=0);
    ~NamesList();

  public:
    void AddName(char *Name);
    bool GetNextName(char *Name);
    bool GetPrevName(char *Name);
    void SetCurName(char *Name);
    void MoveData(NamesList *Dest);
    void GetCurDir(char *Dir,int DestSize);
    void SetCurDir(char *Dir);
};

#endif	// __NAMELIST_HPP__
