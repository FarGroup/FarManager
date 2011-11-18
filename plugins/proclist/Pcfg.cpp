#include <CRT/crt.hpp>
#include <stdlib.h>
#include <PluginSettings.hpp>
#include <DlgBuilder.hpp>
#include "Proclist.hpp"
#include "Proclng.hpp"
#include "guid.hpp"

int Config()
{
	PluginDialogBuilder Builder(Info, MainGuid, ConfigDialogGuid, MConfigTitle, L"Config");

	Builder.AddCheckbox(MConfigAddToDisksMenu, &Opt.AddToDisksMenu);
	Builder.AddCheckbox(MConfigAddToPluginMenu, &Opt.AddToPluginsMenu);
	Builder.AddSeparator();

	Builder.AddText(MIncludeAdditionalInfo);
	Builder.AddCheckbox(MInclEnvironment, &Opt.ExportEnvironment);
	Builder.AddCheckbox(MInclModuleInfo, &Opt.ExportModuleInfo);
	Builder.AddCheckbox(MInclModuleVersion, &Opt.ExportModuleVersion);
	Builder.AddCheckbox(MInclPerformance, &Opt.ExportPerformance);
	Builder.AddCheckbox(MInclHandles, &Opt.ExportHandles);
	//Builder.AddCheckbox(MInclHandlesUnnamed, &Opt.ExportHandlesUnnamed); // ???

	Builder.AddOKCancel(MOk, MCancel);

	if (!Plist::PanelModesInitialized())
		Plist::InitializePanelModes();

	if (Builder.ShowDialog())
	{
		Opt.Write();
		Plist::SavePanelModes();
		Plist::bInit = false;

		return TRUE;
	}

	return FALSE;
}

void _Opt::Write()
{
	PluginSettings settings(MainGuid, Info.SettingsControl);

	settings.Set(0,L"AddToDisksMenu",Opt.AddToDisksMenu);
	settings.Set(0,L"AddToPluginsMenu",Opt.AddToPluginsMenu);
	settings.Set(0,L"ExportEnvironment",Opt.ExportEnvironment);
	settings.Set(0,L"ExportModuleInfo",Opt.ExportModuleInfo);
	settings.Set(0,L"ExportModuleVersion",Opt.ExportModuleVersion);
	settings.Set(0,L"ExportPerformance",Opt.ExportPerformance);
	settings.Set(0,L"ExportHandles",Opt.ExportHandles);
	settings.Set(0,L"ExportHandlesUnnamed",Opt.ExportHandlesUnnamed);
	//settings.Set(0,L"EnableWMI",Opt.EnableWMI,);
}

void _Opt::Read()
{
	PluginSettings settings(MainGuid, Info.SettingsControl);

	Opt.AddToDisksMenu=settings.Get(0,L"AddToDisksMenu", 1);
	Opt.AddToPluginsMenu=settings.Get(0,L"AddToPluginsMenu", 1);
	Opt.ExportEnvironment=settings.Get(0,L"ExportEnvironment", 1);
	Opt.ExportModuleInfo=settings.Get(0,L"ExportModuleInfo", 1);
	Opt.ExportModuleVersion=settings.Get(0,L"ExportModuleVersion", 0);
	Opt.ExportPerformance=settings.Get(0,L"ExportPerformance", 1);
	Opt.ExportHandles=settings.Get(0,L"ExportHandles", 0);
	Opt.ExportHandlesUnnamed=settings.Get(0,L"ExportHandlesUnnamed",0);
	Opt.EnableWMI=settings.Get(0,L"EnableWMI", 1);
}
