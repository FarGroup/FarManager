#ifndef __PANELFILTER_HPP__
#define __PANELFILTER_HPP__
/*
filter.hpp

Фильтр (Ctrl-I)

*/

/* Revision: 1.01 06.05.2001 $ */

/*
Modify:
  06.05.2001 DJ
    ! перетрях #include
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class VMenu;
class Panel;

class PanelFilter
{
  private:
    void SaveFilterFile();
    int SaveFilterData();
    int EditRecord(char *Title,char *Masks);
    int ShowFilterMenu(int Pos,int FirstCall,int *NeedUpdate);
    void AddMasks(char *Masks,int Exclude);
    void ProcessSelection(VMenu *FilterList);
    void SaveFilters();
    Panel *HostPanel;
    char *FilterMask;
    int FilterMaskCount;
    char *ExcludeFilterMask;
    int ExcludeFilterMaskCount;
  public:
    PanelFilter(Panel *HostPanel);
    ~PanelFilter();
    static void InitFilter();
    static void CloseFilter();
    static void SwapFilter();
    static void SaveSelection();
    void FilterEdit();
    int CheckName(char *Name);
    bool IsEnabled();
};
#endif //__PANELFILTER_HPP__
