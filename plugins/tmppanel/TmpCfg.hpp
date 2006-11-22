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
  char ColumnTypes[64];
  char ColumnWidths[64];
  char StatusColumnTypes[64];
  char StatusColumnWidths[64];
  char DisksMenuDigit[1];
  char Mask[512];
  char Prefix[16];
} options_t;

extern options_t Opt;

#endif /* __TMPCFG_HPP__ */