#ifndef __NAMELIST_HPP__
#define __NAMELIST_HPP__
/*
namelist.hpp

Список имен файлов, используется в viewer при нажатии Gray+/Gray-

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/


class NamesList
{
  private:
    char *Names;
    char *CurName;
    int CurNamePos;
    int NamesNumber;
    int NamesSize;

    char CurDir[NM];
  public:
    NamesList();
    ~NamesList();
    void AddName(char *Name);
    bool GetNextName(char *Name);
    bool GetPrevName(char *Name);
    void SetCurName(char *Name);
    void MoveData(NamesList *Dest);
    void GetCurDir(char *Dir);
    void SetCurDir(char *Dir);
};

#endif	// __NAMELIST_HPP__
