/*
TMPCFG.CPP

Temporary panel configuration

*/

#include "TmpCfg.hpp"

#include "guid.hpp"
#include "TmpLng.hpp"
#include "TmpPanel.hpp"

void GetOptions()
{
	PluginSettings settings(MainGuid, PsInfo.SettingsControl);

	Opt.AddToDisksMenu = settings.Get(0, L"AddToDisksMenu", true);
	Opt.AddToPluginsMenu = settings.Get(0, L"AddToPluginsMenu", true);
	Opt.CommonPanel = settings.Get(0, L"CommonPanel", true);
	Opt.SafeModePanel = settings.Get(0, L"SafeModePanel", false);
	Opt.AnyInPanel = settings.Get(0, L"AnyInPanel", false);
	Opt.CopyContents = settings.Get(0, L"CopyContents", 0);
	Opt.Replace = settings.Get(0, L"Mode", true);
	Opt.MenuForFilelist = settings.Get(0, L"MenuForFilelist", false);
	Opt.NewPanelForSearchResults = settings.Get(0, L"NewPanelForSearchResults", false);
	Opt.FullScreenPanel = settings.Get(0, L"FullScreenPanel", false);
	Opt.ListUTF8 = settings.Get(0, L"ListUTF8", true);
	Opt.ColumnTypes = settings.Get(0, L"ColumnTypes", L"N,S");
	Opt.ColumnWidths = settings.Get(0, L"ColumnWidths", L"0,8");
	Opt.StatusColumnTypes = settings.Get(0, L"StatusColumnTypes", L"NR,SC,D,T");
	Opt.StatusColumnWidths = settings.Get(0, L"StatusColumnWidths", L"0,8,0,5");
	Opt.Mask = settings.Get(0, L"Mask", L"*.temp");
	Opt.Prefix = settings.Get(0, L"Prefix", L"tmp");
}

bool Config()
{
	GetOptions();

	PluginDialogBuilder Builder(PsInfo, MainGuid, ConfigDialogGuid, MConfigTitle, L"Config");
	Builder.StartColumns();
	Builder.AddCheckbox(MConfigAddToDisksMenu, &Opt.AddToDisksMenu);
	Builder.ColumnBreak();
	Builder.AddCheckbox(MConfigAddToPluginsMenu, &Opt.AddToPluginsMenu);
	Builder.EndColumns();
	Builder.AddSeparator();

	Builder.StartColumns();
	Builder.AddCheckbox(MConfigCommonPanel, &Opt.CommonPanel);
	Builder.AddCheckbox(MSafeModePanel, &Opt.SafeModePanel);
	Builder.AddCheckbox(MAnyInPanel, &Opt.AnyInPanel);
	Builder.AddCheckbox(MCopyContens, &Opt.CopyContents, 0, true);
	Builder.ColumnBreak();
	Builder.AddCheckbox(MReplaceInFilelist, &Opt.Replace);
	Builder.AddCheckbox(MMenuForFilelist, &Opt.MenuForFilelist);
	Builder.AddCheckbox(MNewPanelForSearchResults, &Opt.NewPanelForSearchResults);
	Builder.AddEmptyLine();
	Builder.EndColumns();
	Builder.AddSeparator();

	Builder.StartColumns();
	Builder.AddText(MColumnTypes);
	Builder.AddEditField(Opt.ColumnTypes, 30, L"TempPanel.ColumnTypes");
	Builder.AddText(MColumnWidths);
	Builder.AddEditField(Opt.ColumnWidths, 30, L"TempPanel.ColumnWidths");
	Builder.ColumnBreak();
	Builder.AddText(MStatusColumnTypes);
	Builder.AddEditField(Opt.StatusColumnTypes, 30, L"TempPanel.StatusColumnTypes");
	Builder.AddText(MStatusColumnWidths);
	Builder.AddEditField(Opt.StatusColumnWidths, 30, L"TempPanel.StatusColumnWidths");
	Builder.EndColumns();
	Builder.AddCheckbox(MFullScreenPanel, &Opt.FullScreenPanel);
	Builder.AddSeparator();

	Builder.StartColumns();
	Builder.AddText(MMask);
	Builder.AddEditField(Opt.Mask, 30, L"TempPanel.Mask");
	Builder.ColumnBreak();
	Builder.AddText(MPrefix);
	Builder.AddEditField(Opt.Prefix, 30, L"TempPanel.Prefix");
	Builder.EndColumns();

	Builder.AddOKCancel(MOk, MCancel);

	if (!Builder.ShowDialog())
		return false;

	PluginSettings settings(MainGuid, PsInfo.SettingsControl);

	settings.Set(0, L"AddToDisksMenu", Opt.AddToDisksMenu);
	settings.Set(0, L"AddToPluginsMenu", Opt.AddToPluginsMenu);
	settings.Set(0, L"CommonPanel", Opt.CommonPanel);
	settings.Set(0, L"SafeModePanel", Opt.SafeModePanel);
	settings.Set(0, L"AnyInPanel", Opt.AnyInPanel);
	settings.Set(0, L"CopyContents", Opt.CopyContents);
	settings.Set(0, L"Mode", Opt.Replace);
	settings.Set(0, L"MenuForFilelist", Opt.MenuForFilelist);
	settings.Set(0, L"NewPanelForSearchResults", Opt.NewPanelForSearchResults);
	settings.Set(0, L"FullScreenPanel", Opt.FullScreenPanel);
	settings.Set(0, L"ListUTF8", Opt.ListUTF8);
	settings.Set(0, L"ColumnTypes", Opt.ColumnTypes.c_str());
	settings.Set(0, L"ColumnWidths", Opt.ColumnWidths.c_str());
	settings.Set(0, L"StatusColumnTypes", Opt.StatusColumnTypes.c_str());
	settings.Set(0, L"StatusColumnWidths", Opt.StatusColumnWidths.c_str());
	settings.Set(0, L"Mask", Opt.Mask.c_str());
	settings.Set(0, L"Prefix", Opt.Prefix.c_str());

	return true;
}
