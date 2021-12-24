#include <cwchar>
#include <shlobj.h>
#include <plugin.hpp>
#include <DlgBuilder.hpp>
#include <PluginSettings.hpp>

#include "FARCmds.hpp"
#include "Lang.hpp"
#include "version.hpp"

#include "guid.hpp"
#include <initguid.h>
#include "guid.hpp"

static struct OptionsName OptName
{
	L"ShowCmdOutput",
	L"CatchMode",
	L"ViewZeroFiles",
	L"EditNewFiles",
	L"MaxDataSize",
	L"Separator",
};

PluginStartupInfo PsInfo;
FarStandardFunctions FSF;
Options Opt;

void WINAPI GetGlobalInfoW(GlobalInfo *Info)
{
	Info->StructSize=sizeof(GlobalInfo);
	Info->MinFarVersion=FARMANAGERVERSION;
	Info->Version=PLUGIN_VERSION;
	Info->Guid=MainGuid;
	Info->Title=PLUGIN_NAME;
	Info->Description=PLUGIN_DESC;
	Info->Author=PLUGIN_AUTHOR;
}

void WINAPI SetStartupInfoW(const PluginStartupInfo *Info)
{
	PsInfo=*Info;
	FSF=*PsInfo.FSF;
	PsInfo.FSF=&FSF;

	PluginSettings settings(MainGuid, PsInfo.SettingsControl);
	settings.Get(0,OptName.Separator,Opt.Separator,ARRAYSIZE(Opt.Separator),L" ");
	Opt.ShowCmdOutput=settings.Get(0,OptName.ShowCmdOutput,0);
	Opt.CatchMode=settings.Get(0,OptName.CatchMode,0);
	Opt.ViewZeroFiles=settings.Get(0,OptName.ViewZeroFiles,1);
	Opt.EditNewFiles=settings.Get(0,OptName.EditNewFiles,1);
	Opt.MaxDataSize=settings.Get(0,OptName.MaxDataSize,1048576);
}

HANDLE WINAPI OpenW(const OpenInfo *Info)
{
	HANDLE DstPanel = PANEL_PASSIVE;
	PanelInfo PInfo={};
	PInfo.StructSize = sizeof(PInfo);
	PsInfo.PanelControl(PANEL_ACTIVE,FCTL_GETPANELINFO,0,&PInfo);

	wchar_t *pTemp=nullptr;

	if (Info->OpenFrom==OPEN_COMMANDLINE) // prefix
	{
		DstPanel = PANEL_ACTIVE;
		pTemp=OpenFromCommandLine(((OpenCommandLineInfo *)Info->Data)->CommandLine);
	}

	// set cursor
	if (pTemp && *pTemp)
	{
		static PanelRedrawInfo PRI={sizeof(PanelRedrawInfo)};
		int pathlen;
		const wchar_t *pName=FSF.PointToName(pTemp);
		wchar_t *Name=new wchar_t[lstrlen(pName)+1];
		lstrcpy(Name,pName);
		FSF.Trim(Name);
		FSF.Unquote(Name);

		pathlen=(int)(pName-pTemp);

		wchar_t *Dir=nullptr;

		if (pathlen > 0 && *pTemp)
		{
			Dir=new wchar_t[pathlen+1];
			wmemcpy(Dir,pTemp,pathlen);
			Dir[pathlen]=0;
			FSF.Trim(Dir);
			FSF.Unquote(Dir);

			FarPanelDirectory dirInfo={sizeof(FarPanelDirectory),Dir};
			PsInfo.PanelControl(DstPanel,FCTL_SETPANELDIRECTORY,0,&dirInfo);
		}

		PsInfo.PanelControl(DstPanel,FCTL_GETPANELINFO,0,&PInfo);
		PRI.CurrentItem=PInfo.CurrentItem;
		PRI.TopPanelItem=PInfo.TopPanelItem;

		for (size_t J=0; J < PInfo.ItemsNumber; J++)
		{
			bool Equal=false;
			size_t Size = PsInfo.PanelControl(DstPanel,FCTL_GETPANELITEM,J,{});
			PluginPanelItem* PPI=(PluginPanelItem*)malloc(Size);

			if (PPI)
			{
				FarGetPluginPanelItem gpi={sizeof(FarGetPluginPanelItem), Size, PPI};
				PsInfo.PanelControl(DstPanel,FCTL_GETPANELITEM,J,&gpi);
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

		PsInfo.PanelControl(DstPanel,FCTL_REDRAWPANEL,0,&PRI);

		delete[] Dir;
		delete[] Name;
	}
	else
	{
		PsInfo.PanelControl(DstPanel,FCTL_REDRAWPANEL,0,{});
	}

	delete [] pTemp;

	return nullptr;
}

void WINAPI GetPluginInfoW(PluginInfo *Info)
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
	PluginDialogBuilder Builder(PsInfo, MainGuid, DialogGuid, MConfig, L"Config");

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
		PluginSettings settings(MainGuid, PsInfo.SettingsControl);
		settings.Set(0,OptName.ShowCmdOutput,Opt.ShowCmdOutput);
		settings.Set(0,OptName.CatchMode,Opt.CatchMode);
		settings.Set(0,OptName.ViewZeroFiles,Opt.ViewZeroFiles);
		settings.Set(0,OptName.EditNewFiles,Opt.EditNewFiles);
		if (!Opt.MaxDataSize)
			Opt.MaxDataSize=1048576;
		settings.Set(0,OptName.MaxDataSize,Opt.MaxDataSize);
		return TRUE;
	}

	return FALSE;
}
