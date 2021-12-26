#ifndef TMPCFG_HPP_25F4A642_C5AE_4888_B059_831752D782CF
#define TMPCFG_HPP_25F4A642_C5AE_4888_B059_831752D782CF
#pragma once

struct options_t
{
	bool AddToDisksMenu{};
	bool AddToPluginsMenu{};
	bool CommonPanel{};
	bool SafeModePanel{};
	bool AnyInPanel{};
	int CopyContents{};
	bool Replace{};
	bool MenuForFilelist{};
	bool NewPanelForSearchResults{};
	bool FullScreenPanel{};
	bool ListUTF8{};
	string ColumnTypes;
	string ColumnWidths;
	string StatusColumnTypes;
	string StatusColumnWidths;
	string Mask;
	string Prefix;

	size_t LastSearchResultsPanel{};
	int SelectedCopyContents{};
};

inline options_t Opt;

void GetOptions();
bool Config();

#endif // TMPCFG_HPP_25F4A642_C5AE_4888_B059_831752D782CF
