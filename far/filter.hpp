#ifndef __PANELFILTER_HPP__
#define __PANELFILTER_HPP__
/*
filter.hpp

Фильтр (Ctrl-I)

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/


class PanelFilter
{
  private:
    void SaveFilterFile();
    int SaveFilterData();
    int EditRecord(char *Title,char *Masks);
    int ShowFilterMenu(int Pos,int FirstCall,int *NeedUpdate);
    void AddMasks(char *Masks,int Exclude);
    void ProcessSelection(VMenu &FilterList);
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
