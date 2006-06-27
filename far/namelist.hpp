#ifndef __NAMELIST_HPP__
#define __NAMELIST_HPP__
/*
namelist.hpp

Список имен файлов, используется в viewer при нажатии Gray+/Gray-

*/

/* Revision: 1.06 15.07.2005 $ */

/*
Modify:
  15.07.2005 WARP
    ! Лист имен файлов на панели (Gray+/- во вьювере) держится в юникоде (класс NameList).
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
        string strName;
        string strShortName;
    };

    struct OneName
    {
      struct FileName2 Value;

      OneName()
      {
      }
      // для перекрывающихся объектов поведение как у xstrncpy!
      const OneName& operator=(struct FileName2 &rhs)
      {
        Value.strName = rhs.strName;
        Value.strShortName = rhs.strShortName;
        return *this;
      }
    };

    typedef TList<OneName> StrList;

    StrList Names;
    OneName CurName;
    const OneName *pCurName;

    string strCurrentDir;

  private:
    void Init();

  public:
    NamesList();
    ~NamesList();

  public:
    void AddName(const wchar_t *Name,const wchar_t *ShortName);
    bool GetNextName(string &strName, string &strShortName);
    bool GetPrevName(string &strName, string &strShortName);
    void SetCurName(const wchar_t *Name);
    void MoveData(NamesList &Dest);
    void GetCurDir(string &strDir);
    void SetCurDir(const wchar_t *Dir);
};

#endif  // __NAMELIST_HPP__
