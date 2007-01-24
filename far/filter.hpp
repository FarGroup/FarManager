#ifndef __PANELFILTER_HPP__
#define __PANELFILTER_HPP__
/*
filter.hpp

Фильтр (Ctrl-I)

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
