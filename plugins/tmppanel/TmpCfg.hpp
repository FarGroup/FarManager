#ifndef __TMPCFG_HPP__
#define __TMPCFG_HPP__

typedef struct options_s
{
  int AddToDisksMenu;
  int AddToPluginsMenu;
  int CommonPanel;
  int SafeModePanel;
  int AnyInPanel;
  int CopyContents;
  int Mode;
  int MenuForFilelist;
  int NewPanelForSearchResults;
  int FullScreenPanel;
  int LastSearchResultsPanel;
  int SelectedCopyContents;
  TCHAR ColumnTypes[64];
  TCHAR ColumnWidths[64];
  TCHAR StatusColumnTypes[64];
  TCHAR StatusColumnWidths[64];
#ifndef UNICODE
  TCHAR DisksMenuDigit[1];
#endif
  TCHAR Mask[512];
  TCHAR Prefix[16];
} options_t;

extern options_t Opt;

#endif /* __TMPCFG_HPP__ */
