#include <cstdlib>

#define WIN32_NO_STATUS //exclude ntstatus.h macros from winnt.h
#include <windows.h>
#undef WIN32_NO_STATUS

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

	if (!Builder.ShowDialog())
		return FALSE;

	Opt.Write();
	Plist::SavePanelModes();

	return TRUE;
}

void options::Write() const
{
	PluginSettings settings(MainGuid, Info.SettingsControl);

	settings.Set(0, L"AddToDisksMenu", AddToDisksMenu);
	settings.Set(0, L"AddToPluginsMenu", AddToPluginsMenu);
	settings.Set(0, L"ExportEnvironment", ExportEnvironment);
	settings.Set(0, L"ExportModuleInfo", ExportModuleInfo);
	settings.Set(0, L"ExportModuleVersion", ExportModuleVersion);
	settings.Set(0, L"ExportPerformance", ExportPerformance);
	settings.Set(0, L"ExportHandles", ExportHandles);
	settings.Set(0, L"ExportHandlesUnnamed", ExportHandlesUnnamed);
	//settings.Set(0, L"EnableWMI", EnableWMI);
}

void options::Read()
{
	PluginSettings settings(MainGuid, Info.SettingsControl);

	AddToDisksMenu = settings.Get(0, L"AddToDisksMenu", 1);
	AddToPluginsMenu = settings.Get(0, L"AddToPluginsMenu", 1);
	ExportEnvironment = settings.Get(0, L"ExportEnvironment", 1);
	ExportModuleInfo = settings.Get(0, L"ExportModuleInfo", 1);
	ExportModuleVersion = settings.Get(0, L"ExportModuleVersion", 0);
	ExportPerformance = settings.Get(0, L"ExportPerformance", 1);
	ExportHandles = settings.Get(0, L"ExportHandles", 0);
	ExportHandlesUnnamed = settings.Get(0, L"ExportHandlesUnnamed", 0);
	EnableWMI = settings.Get(0, L"EnableWMI", 1);
}
