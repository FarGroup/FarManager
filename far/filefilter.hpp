#ifndef __FILEFILTER_HPP__
#define __FILEFILTER_HPP__
/*
filefilter.hpp

Файловый фильтр

*/

#include "plugin.hpp"
#include "struct.hpp"
#include "filefilterparams.hpp"

class VMenu;
class Panel;

enum enumFileFilterType
{
  FFT_PANEL = 0,
  FFT_FINDFILE,
  FFT_COPY,
  FFT_SELECT,
};

class FileFilter
{
  private:
    Panel *m_HostPanel;
    enumFileFilterType m_FilterType;
    unsigned __int64 CurrentTime;

    int  ParseAndAddMasks(char **ExtPtr,const char *FileName,DWORD FileAttr,int& ExtCount,int Check);
    void ProcessSelection(VMenu *FilterList);
    enumFileFilterFlagsType GetFFFT();
    int  GetCheck(FileFilterParams *FFP);
    static void SwapPanelFlags(FileFilterParams *CurFilterData);

  public:
    FileFilter(Panel *HostPanel, enumFileFilterType FilterType);
    ~FileFilter();

    bool FilterEdit();
    void UpdateCurrentTime();
    bool FileInFilter(FileListItem *fli);
    bool FileInFilter(WIN32_FIND_DATA *fd);
    bool IsEnabledOnPanel();

    static void InitFilter();
    static void CloseFilter();
    static void SwapFilter();
    static void SaveFilters();
};

#endif  // __FINDFILES_HPP__
