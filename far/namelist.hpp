#ifndef __NAMELIST_HPP__
#define __NAMELIST_HPP__
/*
namelist.hpp

Список имен файлов, используется в viewer при нажатии Gray+/Gray-

*/

/* Revision: 1.03 19.11.2003 $ */

/*
Modify:
  19.11.2003 IS
    + Работа со списком (TList) на основе кода, созданного KS, чтобы
      собиралось под борманом.
    ! внедрение const
    + защита от переполнения буфера в GetNextName/GetPrevName
    ! MoveData работает со ссылкой, а не указателем
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
#include "TList.hpp"

class NamesList
{
  private:
    struct OneName
    {
      char Value[MAX_PATH];
      OneName()
      {
        Value[0]=0;
      }
      // для перекрывающихся объектов поведение как у strncpy!
      const OneName& operator=(const char *rhs)
      {
        strncpy(Value,rhs,sizeof(Value)-1);
        return *this;
      }
    };

    typedef TList<OneName> StrList;

    StrList Names;
    OneName CurName;
    const OneName *pCurName;


    char CurDir[NM];

  private:
    void Init();

  public:
    NamesList();
    ~NamesList();

  public:
    void AddName(const char *Name);
    bool GetNextName(char *Name, const size_t NameSize);
    bool GetPrevName(char *Name, const size_t NameSize);
    void SetCurName(const char *Name);
    void MoveData(NamesList &Dest);
    void GetCurDir(char *Dir,int DestSize);
    void SetCurDir(const char *Dir);
};

#endif  // __NAMELIST_HPP__
