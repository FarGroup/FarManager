#include <shlobj.h>
#include <plugin.hpp>
#include <DlgBuilder.hpp>
#include <PluginSettings.hpp>

#include "SameFolder.hpp"
#include "Lang.hpp"
#include "version.hpp"
#include <initguid.h>
#include "guid.hpp"

struct OptionsName OptName={
	L"Add2PlugMenu",
	L"Add2DisksMenu",
	L"SetMode",
};

struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;

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
	Opt.Add2PlugMenu=settings.Get(0,OptName.Add2PlugMenu,1);
	Opt.Add2DisksMenu=settings.Get(0,OptName.Add2DisksMenu,0);
	Opt.SetMode=settings.Get(0,OptName.SetMode,0);
}

HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
	HANDLE SrcPanel = PANEL_ACTIVE, DstPanel = PANEL_PASSIVE;
	struct PanelInfo PInfo={};
	PInfo.StructSize = sizeof(PInfo);
	Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELINFO,0,&PInfo);

	if (OInfo->OpenFrom==OPEN_COMMANDLINE) // prefix
	{
		return nullptr;
	}
	else // Same Folder
	{
		if((OInfo->OpenFrom == OPEN_LEFTDISKMENU && (PInfo.Flags&PFLAGS_PANELLEFT)) ||
		   (OInfo->OpenFrom == OPEN_RIGHTDISKMENU && !(PInfo.Flags&PFLAGS_PANELLEFT)))
		{
			SrcPanel = PANEL_PASSIVE;
			DstPanel = PANEL_ACTIVE;
		}


		PInfo.StructSize = sizeof(PInfo);
		Info.PanelControl(SrcPanel,FCTL_GETPANELINFO,0,&PInfo);

		int dirSize=(int)Info.PanelControl(SrcPanel,FCTL_GETPANELDIRECTORY,0,0);

		FarPanelDirectory* dirInfo=(FarPanelDirectory*)malloc(dirSize);
		if (dirInfo)
		{
			// 1. set dir
			dirInfo->StructSize = sizeof(FarPanelDirectory);
			Info.PanelControl(SrcPanel,FCTL_GETPANELDIRECTORY,dirSize,dirInfo);
			Info.PanelControl(DstPanel,FCTL_SETPANELDIRECTORY,0,dirInfo);

			struct PanelRedrawInfo PRI={sizeof(PanelRedrawInfo)};
			PRI.CurrentItem=0;
			PRI.TopPanelItem=0;

			// 2. set position
			size_t Size = Info.PanelControl(SrcPanel,FCTL_GETPANELITEM,PInfo.CurrentItem,0);
			PluginPanelItem* PPISrc=(PluginPanelItem*)malloc(Size);

			if (PPISrc)
			{
				FarGetPluginPanelItem gpiSrc={sizeof(FarGetPluginPanelItem), Size, PPISrc};
				Info.PanelControl(SrcPanel,FCTL_GETPANELITEM,PInfo.CurrentItem,&gpiSrc);

				// find position
				for (size_t J=0; J < PInfo.ItemsNumber; J++)
				{
					bool Equal=false;
					size_t Size = Info.PanelControl(DstPanel,FCTL_GETPANELITEM,J,0);
					PluginPanelItem* PPIDst=(PluginPanelItem*)malloc(Size);
					if (PPIDst)
					{
						FarGetPluginPanelItem gpiDst={sizeof(FarGetPluginPanelItem), Size, PPIDst};
						Info.PanelControl(DstPanel,FCTL_GETPANELITEM,J,&gpiDst);
						Equal=ComparePPI(PPISrc,PPIDst);
						free(PPIDst);
					}

					if (Equal)
					{
						PRI.CurrentItem=J;
						PRI.TopPanelItem=J;
						break;
					}
				}

				free(PPISrc);
			}

			if (Opt.SetMode)
			{
				// TODO: сюда можно добавить установку визуальных настроек панели DstPanel, как у SrcPanel
				;
			}

			Info.PanelControl(DstPanel,FCTL_REDRAWPANEL,0,&PRI);

			free(dirInfo);
		}
	}

	return nullptr;
}


void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=PF_NONE; //PF_FULLCMDLINE;

	static const wchar_t *PluginMenuStrings[1];
	static const wchar_t *PluginConfigStrings[1];
	static const wchar_t *DiskMenuStrings[1];

	if (Opt.Add2PlugMenu)
	{
		PluginMenuStrings[0]=GetMsg(MSetSameDir);
        Info->PluginMenu.Guids=&MenuGuid;
        Info->PluginMenu.Strings=PluginMenuStrings;
        Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
	}

	if (Opt.Add2DisksMenu)
	{
		DiskMenuStrings[0]=(wchar_t*)GetMsg(MSetSameDir);
        Info->DiskMenu.Guids=&MenuGuid;
        Info->DiskMenu.Strings=DiskMenuStrings;
        Info->DiskMenu.Count=ARRAYSIZE(DiskMenuStrings);
	}

	PluginConfigStrings[0]=GetMsg(MConfig);
    Info->PluginConfig.Guids=&ConfigMenuGuid;
    Info->PluginConfig.Strings=PluginConfigStrings;
    Info->PluginConfig.Count=ARRAYSIZE(PluginConfigStrings);

	Info->CommandPrefix=L"";
}

intptr_t WINAPI ConfigureW(const ConfigureInfo* CfgInfo)
{
    PluginDialogBuilder Builder(Info, MainGuid, DialogGuid, MConfig, L"Config");

    Builder.AddCheckbox(MAddSetPassiveDir2PlugMenu, &Opt.Add2PlugMenu);
    Builder.AddCheckbox(MAddToDisksMenu, &Opt.Add2DisksMenu);
    //Builder.AddCheckbox(MSetMode, &Opt.SetMode);

    Builder.AddOKCancel(MOk, MCancel);

    if (Builder.ShowDialog())
	{
		PluginSettings settings(MainGuid, Info.SettingsControl);
		settings.Set(0,OptName.Add2PlugMenu,Opt.Add2PlugMenu);
		settings.Set(0,OptName.Add2DisksMenu,Opt.Add2DisksMenu);
		settings.Set(0,OptName.SetMode,Opt.SetMode);
		return TRUE;
	}

	return FALSE;
}

const wchar_t *GetMsg(int MsgId)
{
	return Info.GetMsg(&MainGuid,MsgId);
}

inline uint64_t FileTimeToUI64(const FILETIME& ft)
{
	ULARGE_INTEGER t = {ft.dwLowDateTime, ft.dwHighDateTime};
	return t.QuadPart;
}

bool ComparePPI(const PluginPanelItem* PPISrc,const PluginPanelItem* PPIDst)
{
	if (FileTimeToUI64(PPISrc->CreationTime) == FileTimeToUI64(PPIDst->CreationTime) &&
		FileTimeToUI64(PPISrc->LastAccessTime) == FileTimeToUI64(PPIDst->LastAccessTime) &&
		FileTimeToUI64(PPISrc->LastWriteTime) == FileTimeToUI64(PPIDst->LastWriteTime) &&
		FileTimeToUI64(PPISrc->ChangeTime) == FileTimeToUI64(PPIDst->ChangeTime) &&
		PPISrc->FileSize == PPIDst->FileSize &&
		PPISrc->AllocationSize == PPIDst->AllocationSize &&
		PPISrc->FileAttributes == PPIDst->FileAttributes &&
		PPISrc->NumberOfLinks == PPIDst->NumberOfLinks &&
		PPISrc->CRC32 == PPIDst->CRC32 &&
		!lstrcmp(PPISrc->FileName,PPIDst->FileName) &&
		!lstrcmp(PPISrc->AlternateFileName,PPIDst->AlternateFileName)
	)
		return true;
	return false;
}
