/*
TMPCFG.CPP

Temporary panel configuration

*/

#include "plugin.hpp"
#include <shellapi.h>
#include <PluginSettings.hpp>
#include <DlgBuilder.hpp>

#include "TmpLng.hpp"
#include "TmpCfg.hpp"
#include "TmpClass.hpp"
#include "TmpPanel.hpp"
#include <initguid.h>
#include "guid.hpp"

options_t Opt;

int StartupOptFullScreenPanel,StartupOptCommonPanel,StartupOpenFrom;

void GetOptions(void)
{
	PluginSettings settings(MainGuid, Info.SettingsControl);

	Opt.AddToDisksMenu=settings.Get(0,L"AddToDisksMenu",1);
	Opt.AddToPluginsMenu=settings.Get(0,L"AddToPluginsMenu",1);
	Opt.CommonPanel=settings.Get(0,L"CommonPanel",1);
	Opt.SafeModePanel=settings.Get(0,L"SafeModePanel",0);
	Opt.AnyInPanel=settings.Get(0,L"AnyInPanel",0);
	Opt.CopyContents=settings.Get(0,L"CopyContents",0);
	Opt.Mode=settings.Get(0,L"Mode",1);
	Opt.MenuForFilelist=settings.Get(0,L"MenuForFilelist",0);
	Opt.NewPanelForSearchResults=settings.Get(0,L"NewPanelForSearchResults",0);
	Opt.FullScreenPanel=settings.Get(0,L"FullScreenPanel",0);
	Opt.ListUTF8=settings.Get(0,L"ListUTF8",1);

	settings.Get(0,L"ColumnTypes",Opt.ColumnTypes,ARRAYSIZE(Opt.ColumnTypes),L"N,S");
	settings.Get(0,L"ColumnWidths",Opt.ColumnWidths,ARRAYSIZE(Opt.ColumnWidths),L"0,8");
	settings.Get(0,L"StatusColumnTypes",Opt.StatusColumnTypes,ARRAYSIZE(Opt.StatusColumnTypes),L"NR,SC,D,T");
	settings.Get(0,L"StatusColumnWidths",Opt.StatusColumnWidths,ARRAYSIZE(Opt.StatusColumnWidths),L"0,8,0,5");
	settings.Get(0,L"Mask",Opt.Mask,ARRAYSIZE(Opt.Mask),L"*.temp");
	settings.Get(0,L"Prefix",Opt.Prefix,ARRAYSIZE(Opt.Prefix),L"tmp");
}

int Config()
{
	GetOptions();

	PluginDialogBuilder Builder(Info, MainGuid, ConfigDialogGuid, MConfigTitle, L"Config");
	Builder.StartColumns();
	Builder.AddCheckbox(MConfigAddToDisksMenu, &Opt.AddToDisksMenu);
	Builder.ColumnBreak();
	Builder.AddCheckbox(MConfigAddToPluginsMenu, &Opt.AddToPluginsMenu);
	Builder.EndColumns();
	Builder.AddSeparator();

	Builder.StartColumns();
	Builder.AddCheckbox(MConfigCommonPanel,&Opt.CommonPanel);
	Builder.AddCheckbox(MSafeModePanel,&Opt.SafeModePanel);
	Builder.AddCheckbox(MAnyInPanel,&Opt.AnyInPanel);
	Builder.AddCheckbox(MCopyContens,&Opt.CopyContents,0,true);
	Builder.ColumnBreak();
	Builder.AddCheckbox(MReplaceInFilelist,&Opt.Mode);
	Builder.AddCheckbox(MMenuForFilelist,&Opt.MenuForFilelist);
	Builder.AddCheckbox(MNewPanelForSearchResults,&Opt.NewPanelForSearchResults);
	Builder.AddEmptyLine();
	Builder.EndColumns();
	Builder.AddSeparator();

	Builder.StartColumns();
	Builder.AddText(MColumnTypes);
	Builder.AddEditField(Opt.ColumnTypes,ARRAYSIZE(Opt.ColumnTypes), 30, L"TempPanel.ColumnTypes");
	Builder.AddText(MColumnWidths);
	Builder.AddEditField(Opt.ColumnWidths,ARRAYSIZE(Opt.ColumnWidths), 30, L"TempPanel.ColumnWidths");
	Builder.ColumnBreak();
	Builder.AddText(MStatusColumnTypes);
	Builder.AddEditField(Opt.StatusColumnTypes,ARRAYSIZE(Opt.StatusColumnTypes), 30, L"TempPanel.StatusColumnTypes");
	Builder.AddText(MStatusColumnWidths);
	Builder.AddEditField(Opt.StatusColumnWidths,ARRAYSIZE(Opt.StatusColumnWidths), 30, L"TempPanel.StatusColumnWidths");
	Builder.EndColumns();
	Builder.AddCheckbox(MFullScreenPanel,&Opt.FullScreenPanel);
	Builder.AddSeparator();


	Builder.StartColumns();
	Builder.AddText(MMask);
	Builder.AddEditField(Opt.Mask,ARRAYSIZE(Opt.Mask), 30, L"TempPanel.Mask");
	Builder.ColumnBreak();
	Builder.AddText(MPrefix);
	Builder.AddEditField(Opt.Prefix,ARRAYSIZE(Opt.Prefix), 30, L"TempPanel.Prefix");
	Builder.EndColumns();

	Builder.AddOKCancel(MOk, MCancel);

	if (Builder.ShowDialog())
	{
	    PluginSettings settings(MainGuid, Info.SettingsControl);

		settings.Set(0,L"AddToDisksMenu",Opt.AddToDisksMenu);
		settings.Set(0,L"AddToPluginsMenu",Opt.AddToPluginsMenu);
		settings.Set(0,L"CommonPanel",Opt.CommonPanel);
		settings.Set(0,L"SafeModePanel",Opt.SafeModePanel);
		settings.Set(0,L"AnyInPanel",Opt.AnyInPanel);
		settings.Set(0,L"CopyContents",Opt.CopyContents);
		settings.Set(0,L"Mode",Opt.Mode);
		settings.Set(0,L"MenuForFilelist",Opt.MenuForFilelist);
		settings.Set(0,L"NewPanelForSearchResults",Opt.NewPanelForSearchResults);
		settings.Set(0,L"FullScreenPanel",Opt.FullScreenPanel);
		settings.Set(0,L"ListUTF8",Opt.ListUTF8);

		settings.Set(0,L"ColumnTypes",Opt.ColumnTypes);
		settings.Set(0,L"ColumnWidths",Opt.ColumnWidths);
		settings.Set(0,L"StatusColumnTypes",Opt.StatusColumnTypes);
		settings.Set(0,L"StatusColumnWidths",Opt.StatusColumnWidths);
		settings.Set(0,L"Mask",Opt.Mask);
		settings.Set(0,L"Prefix",Opt.Prefix);

		if (StartupOptFullScreenPanel!=Opt.FullScreenPanel || StartupOptCommonPanel!=Opt.CommonPanel)
		{
			const wchar_t *MsgItems[]={GetMsg(MTempPanel),GetMsg(MConfigNewOption),GetMsg(MOk)};
			Info.Message(&MainGuid, nullptr,0,NULL,MsgItems,ARRAYSIZE(MsgItems),1);
		}
		return TRUE;
	}

	return FALSE;
}
