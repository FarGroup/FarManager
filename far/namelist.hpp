#ifndef __NAMELIST_HPP__
#define __NAMELIST_HPP__
/*
namelist.hpp

Список имен файлов, используется в viewer при нажатии Gray+/Gray-

*/

/* Revision: 1.05 13.07.2005 $ */

/*
Modify:
  13.07.2005 SVS
    ! Изменен класс NamesList. Теперь он управляет двумя именами.
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
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
#include "plugin.hpp"
#include "fn.hpp"

class NamesList
{
  private:
    struct FileName2{
      char Name[MAX_PATH];
      char ShortName[MAX_PATH];
    };

    struct OneName
    {
      struct FileName2 Value;
      OneName()
      {
        Value.Name[0]=0;
        Value.ShortName[0]=0;
      }
      // для перекрывающихся объектов поведение как у xstrncpy!
      const OneName& operator=(struct FileName2 &rhs)
      {
        xstrncpy(Value.Name,rhs.Name,sizeof(Value.Name)-1);
        xstrncpy(Value.ShortName,rhs.ShortName,sizeof(Value.ShortName)-1);
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
    void AddName(const char *Name,const char *ShortName);
    bool GetNextName(char *Name, const size_t NameSize,char *ShortName, const size_t ShortNameSize);
    bool GetPrevName(char *Name, const size_t NameSize,char *ShortName, const size_t ShortNameSize);
    void SetCurName(const char *Name);
    void MoveData(NamesList &Dest);
    void GetCurDir(char *Dir,int DestSize);
    void SetCurDir(const char *Dir);
};

#endif  // __NAMELIST_HPP__
