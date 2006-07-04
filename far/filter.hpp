#ifndef __PANELFILTER_HPP__
#define __PANELFILTER_HPP__
/*
filter.hpp

Фильтр (Ctrl-I)

*/

/* Revision: 1.04 04.07.2006 $ */

/*
Modify:
  04.07.2006 IS
    - warnings
  03.04.2002 SVS
    + ParseAndAddMasks() - выявлять и если надо добавлять очередную маску
    ! Немного красоты ;-)
  01.07.2001 IS
    + #include "CFileMask.hpp"
    ! Внедрение const
  06.05.2001 DJ
    ! перетрях #include
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "CFileMask.hpp"

class VMenu;
class Panel;

class PanelFilter
{
  private:
    Panel *HostPanel;
    /* $ 01.07.2001 IS
       Добавим классы для работы с масками
    */
    CFileMask IncludeMask, ExcludeMask;
    char *IncludeMaskStr, *ExcludeMaskStr;
    BOOL IncludeMaskIsOK, ExcludeMaskIsOK;
    /* IS $ */

  private:
    void SaveFilterFile();
    int  SaveFilterData();
    int  EditRecord(char *Title,char *Masks);
    int  ShowFilterMenu(int Pos,int FirstCall,int *NeedUpdate);
    void AddMasks(const char *Masks,int Exclude);
    int  ParseAndAddMasks(char **ExtPtr,const char *FileName,DWORD FileAttr,int& ExtCount);
    void ProcessSelection(VMenu *FilterList);
    void SaveFilters();

  public:
    PanelFilter(Panel *HostPanel);
    ~PanelFilter();

  public:
    void FilterEdit();
    int CheckName(const char *Name);
    bool IsEnabled();

  public:
    static void InitFilter();
    static void CloseFilter();
    static void SwapFilter();
    static void SaveSelection();
};
#endif //__PANELFILTER_HPP__
