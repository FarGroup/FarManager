#include <cwchar>
#include <shlobj.h>
#include <plugin.hpp>
#include <DlgBuilder.hpp>
#include <PluginSettings.hpp>

#include "FARCmds.hpp"
#include "Lang.hpp"
#include "version.hpp"
#include <initguid.h>
#include "guid.hpp"

struct OptionsName OptName={
	L"ShowCmdOutput",
	L"CatchMode",
	L"ViewZeroFiles",
	L"EditNewFiles",
	L"MaxDataSize",
	L"Separator",
};

struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;
struct Options Opt;

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	Info->StructSize=sizeof(GlobalInfo);
	Info->MinFarVersion=FARMANAGERVERSION;
	Info->Version=PLUGIN_VERSION;
	Info->Guid=MainGuid;
	Info->Title=PLUGIN_NAME;
	Info->Description=PLUGIN_DESC;
	Info->Author=PLUGIN_AUTHOR;
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *psInfo)
{
	Info=*psInfo;
	FSF=*psInfo->FSF;
	Info.FSF=&FSF;

	PluginSettings settings(MainGuid, Info.SettingsControl);
	settings.Get(0,OptName.Separator,Opt.Separator,ARRAYSIZE(Opt.Separator),L" ");
	Opt.ShowCmdOutput=settings.Get(0,OptName.ShowCmdOutput,0);
	Opt.CatchMode=settings.Get(0,OptName.CatchMode,0);
	Opt.ViewZeroFiles=settings.Get(0,OptName.ViewZeroFiles,1);
	Opt.EditNewFiles=settings.Get(0,OptName.EditNewFiles,1);
	Opt.MaxDataSize=settings.Get(0,OptName.MaxDataSize,1048576);
}

HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
	HANDLE DstPanel = PANEL_PASSIVE;
	struct PanelInfo PInfo={};
	PInfo.StructSize = sizeof(PInfo);
	Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELINFO,0,&PInfo);

	wchar_t *pTemp=nullptr;

	if (OInfo->OpenFrom==OPEN_COMMANDLINE) // prefix
	{
		DstPanel = PANEL_ACTIVE;
		pTemp=OpenFromCommandLine(((OpenCommandLineInfo *)OInfo->Data)->CommandLine);
	}

	// set cursor
	if (pTemp && *pTemp)
	{
		static struct PanelRedrawInfo PRI={sizeof(PanelRedrawInfo)};
		int pathlen;
		const wchar_t *pName=FSF.PointToName(pTemp);
		wchar_t *Name=new wchar_t[lstrlen(pName)+1];

		if (!Name)
		{
			delete [] pTemp;
			return nullptr;
		}
		lstrcpy(Name,pName);
		FSF.Trim(Name);
		FSF.Unquote(Name);

		pathlen=(int)(pName-pTemp);

		wchar_t *Dir=nullptr;

		if (pathlen > 0 && *pTemp)
		{
			Dir=new wchar_t[pathlen+1];
			if (!Dir)
			{
				delete[] Name;
				delete [] pTemp;
				return nullptr;
			}
			wmemcpy(Dir,pTemp,pathlen);
			Dir[pathlen]=0;
			FSF.Trim(Dir);
			FSF.Unquote(Dir);

			FarPanelDirectory dirInfo={sizeof(FarPanelDirectory),Dir,NULL,{0},NULL};
			Info.PanelControl(DstPanel,FCTL_SETPANELDIRECTORY,0,&dirInfo);
		}

		Info.PanelControl(DstPanel,FCTL_GETPANELINFO,0,&PInfo);
		PRI.CurrentItem=PInfo.CurrentItem;
		PRI.TopPanelItem=PInfo.TopPanelItem;

		for (size_t J=0; J < PInfo.ItemsNumber; J++)
		{
			bool Equal=false;
			size_t Size = Info.PanelControl(DstPanel,FCTL_GETPANELITEM,J,0);
			PluginPanelItem* PPI=(PluginPanelItem*)malloc(Size);

			if (PPI)
			{
				FarGetPluginPanelItem gpi={sizeof(FarGetPluginPanelItem), Size, PPI};
				Info.PanelControl(DstPanel,FCTL_GETPANELITEM,J,&gpi);
				Equal=!FSF.LStricmp(Name,FSF.PointToName(PPI->FileName));
				free(PPI);
			}

			if (Equal)
			{
				PRI.CurrentItem=J;
				PRI.TopPanelItem=J;
				break;
			}
		}

		Info.PanelControl(DstPanel,FCTL_REDRAWPANEL,0,&PRI);

		if (Dir)
			delete[] Dir;
		delete[] Name;
	}
	else
	{
		Info.PanelControl(DstPanel,FCTL_REDRAWPANEL,0,0);
	}

	if (pTemp)
		delete [] pTemp;

	return nullptr;
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=PF_FULLCMDLINE;

	static const wchar_t *PluginConfigStrings[1];

	PluginConfigStrings[0]=GetMsg(MConfig);
	Info->PluginConfig.Guids=&ConfigMenuGuid;
	Info->PluginConfig.Strings=PluginConfigStrings;
	Info->PluginConfig.Count=ARRAYSIZE(PluginConfigStrings);

	Info->CommandPrefix=L"view:edit:goto:clip:whereis:link:run:load:unload";
}

intptr_t WINAPI ConfigureW(const ConfigureInfo* CfgInfo)
{
	PluginDialogBuilder Builder(Info, MainGuid, DialogGuid, MConfig, L"Config");

	const int CmdOutIDs[] = {MHideCmdOutput, MKeepCmdOutput, MEchoCmdOutput};
	Builder.AddRadioButtons(&Opt.ShowCmdOutput, 3, CmdOutIDs);

	Builder.AddSeparator();

	const int CatchIDs[] = {MCatchAllInOne, MCatchStdOutput, MCatchStdError, MCatchSeparate};
	Builder.AddRadioButtons(&Opt.CatchMode, ARRAYSIZE(CatchIDs), CatchIDs);
	Builder.AddCheckbox(MViewZeroFiles, &Opt.ViewZeroFiles);

	Builder.AddSeparator();

	Builder.AddCheckbox(MEditNewFiles, &Opt.EditNewFiles);

	Builder.AddSeparator();

	FarDialogItem *MaxData = Builder.AddUIntEditField((unsigned int *)&Opt.MaxDataSize, 10);
	Builder.AddTextBefore(MaxData, MMaxDataSize);

	Builder.AddOKCancel(MOk, MCancel);

	if (Builder.ShowDialog())
	{
		PluginSettings settings(MainGuid, Info.SettingsControl);
		settings.Set(0,OptName.ShowCmdOutput,Opt.ShowCmdOutput);
		settings.Set(0,OptName.CatchMode,Opt.CatchMode);
		settings.Set(0,OptName.ViewZeroFiles,Opt.ViewZeroFiles);
		settings.Set(0,OptName.EditNewFiles,Opt.EditNewFiles);
		if (!Opt.MaxDataSize)
			Opt.MaxDataSize=1048576;
		if (Opt.MaxDataSize > 0xFFFFFFFFU)
			Opt.MaxDataSize=0xFFFFFFFFU;
		settings.Set(0,OptName.MaxDataSize,Opt.MaxDataSize);
		return TRUE;
	}

	return FALSE;
}
