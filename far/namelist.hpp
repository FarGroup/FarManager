#ifndef __NAMELIST_HPP__
#define __NAMELIST_HPP__
/*
namelist.hpp

������ ���� ������, ������������ � viewer ��� ������� Gray+/Gray-

*/

/* Revision: 1.06 15.07.2005 $ */

/*
Modify:
  15.07.2005 WARP
    ! ���� ���� ������ �� ������ (Gray+/- �� �������) �������� � ������� (����� NameList).
  13.07.2005 SVS
    ! ������� ����� NamesList. ������ �� ��������� ����� �������.
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  19.11.2003 IS
    + ������ �� ������� (TList) �� ������ ����, ���������� KS, �����
      ���������� ��� ��������.
    ! ��������� const
    + ������ �� ������������ ������ � GetNextName/GetPrevName
    ! MoveData �������� �� �������, � �� ����������
  14.10.2003 SVS
    ! �������� � NamesList.
    ! NamesList::GetCurDir - ����� ���. �������� - ��������� ������.
    + NamesList::Init()
  06.05.2001 DJ
    ! �������� #include
  25.06.2000 SVS
    ! ���������� Master Copy
    ! ��������� � �������� ���������������� ������
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
      // ��� ��������������� �������� ��������� ��� � xstrncpy!
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
