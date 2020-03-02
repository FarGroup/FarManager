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
	int ListUTF8;
	wchar_t ColumnTypes[64];
	wchar_t ColumnWidths[64];
	wchar_t StatusColumnTypes[64];
	wchar_t StatusColumnWidths[64];
	wchar_t Mask[512];
	wchar_t Prefix[16];
} options_t;

extern options_t Opt;

#endif /* __TMPCFG_HPP__ */
