#ifndef __PANELFILTER_HPP__
#define __PANELFILTER_HPP__
/*
filter.hpp

Фильтр (Ctrl-I)

*/

/* Revision: 1.06 07.07.2006 $ */

#include "CFileMask.hpp"
#include "UnicodeString.hpp"

class VMenu;
class Panel;

class PanelFilter
{
  private:
    Panel *HostPanel;
    /* $ 01.07.2001 IS
       Добавим классы для работы с масками
    */
    CFileMaskW IncludeMask, ExcludeMask;
    wchar_t *IncludeMaskStr, *ExcludeMaskStr;
    BOOL IncludeMaskIsOK, ExcludeMaskIsOK;
    /* IS $ */

  private:
    void SaveFilterFile();
    int  SaveFilterData();
    int  EditRecord(string &strTitle, string &strMasks);
    int  ShowFilterMenu(int Pos,int FirstCall,int *NeedUpdate);
    void AddMasks(const wchar_t *Masks,int Exclude);
    int  ParseAndAddMasks(wchar_t **ExtPtr,const wchar_t *FileName,DWORD FileAttr,int& ExtCount);
    void ProcessSelection(VMenu *FilterList);
    void SaveFilters();

  public:
    PanelFilter(Panel *HostPanel);
    ~PanelFilter();

  public:
    void FilterEdit();
    int CheckNameW(const wchar_t *Name);
    bool IsEnabled();

  public:
    static void InitFilter();
    static void CloseFilter();
    static void SwapFilter();
    static void SaveSelection();
};
#endif //__PANELFILTER_HPP__
