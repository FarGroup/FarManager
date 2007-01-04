#ifndef __NAMELIST_HPP__
#define __NAMELIST_HPP__
/*
namelist.hpp

Список имен файлов, используется в viewer при нажатии Gray+/Gray-

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
