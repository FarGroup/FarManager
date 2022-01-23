#include <cstdlib>
#include "FileCase.hpp"
#include <DlgBuilder.hpp>
#include <PluginSettings.hpp>
#include "FileLng.hpp"
#include "guid.hpp"

static int ResetButtonID=0;
static int WordDivEditID=0;

static INT_PTR WINAPI DlgProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void *Param2)
{
	switch (Msg)
	{
		case DN_BTNCLICK:

			if (Param1==ResetButtonID)
			{
				PsInfo.SendDlgMessage(hDlg,DM_SETTEXTPTR,WordDivEditID,const_cast<wchar_t*>(L" _"));
				return TRUE;
			}

			break;
	}

	return PsInfo.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void CaseConvertion()
{
	Options Backup;
	memcpy(&Backup,&Opt,sizeof(Backup));

	PluginDialogBuilder Builder(PsInfo, MainGuid, DialogGuid, MFileCase, L"Contents", DlgProc);
	Builder.StartColumns();
	Builder.AddText(MName);
	const int NameIDs[] = {MLower, MUpper, MFirst, MTitle, MNone};
	Builder.AddRadioButtons(&Opt.ConvertMode, ARRAYSIZE(NameIDs), NameIDs, true);
	Builder.ColumnBreak();
	Builder.AddText(MExtension);
	const int ExtIDs[] = {MLowerExt, MUpperExt, MFirstExt, MTitleExt, MNoneExt};
	Builder.AddRadioButtons(&Opt.ConvertModeExt, ARRAYSIZE(ExtIDs), ExtIDs);
	Builder.EndColumns();
	Builder.AddSeparator();
	Builder.AddCheckbox(MSkipMixedCase, &Opt.SkipMixedCase);
	Builder.AddCheckbox(MProcessSubDir, &Opt.ProcessSubDir);
	Builder.AddCheckbox(MProcessDir, &Opt.ProcessDir);
	Builder.AddSeparator();
	int CurRun = 0;
	Builder.AddCheckbox(MCurRun, &CurRun);
	Builder.AddSeparator();
	Builder.AddText(MWordDiv);
	FarDialogItem *Edit = Builder.AddEditField(Opt.WordDiv,ARRAYSIZE(Opt.WordDiv),20,L"FileCase_WordDiv");
	WordDivEditID = Builder.GetLastID();
	Builder.AddButtonAfter(Edit, MReset);
	ResetButtonID = Builder.GetLastID();
	Builder.AddOKCancel(MOk, MCancel);

	if (Builder.ShowDialog())
	{
		if (Opt.ConvertMode!=MODE_NONE || Opt.ConvertModeExt!=MODE_NONE)
		{
			Opt.WordDivLen=lstrlen(Opt.WordDiv);
			PanelInfo PInfo = {sizeof(PanelInfo)};
			PsInfo.PanelControl(PANEL_ACTIVE,FCTL_GETPANELINFO,0,&PInfo);

			HANDLE hScreen=PsInfo.SaveScreen(0,0,-1,-1);

			const wchar_t *MsgItems[]={GetMsg(MFileCase),GetMsg(MConverting)};
			PsInfo.Message(&MainGuid, nullptr,0,{},MsgItems,ARRAYSIZE(MsgItems),{});

			int DirSize=(int)PsInfo.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,0,{});
			FarPanelDirectory* dirInfo=(FarPanelDirectory*)malloc(DirSize);
			if (dirInfo)
			{
				dirInfo->StructSize = sizeof(FarPanelDirectory);
				PsInfo.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY, DirSize,dirInfo);
				const auto NameLength = lstrlen(dirInfo->Name);

				for (size_t I=0; I < PInfo.SelectedItemsNumber; I++)
				{
					size_t Size = PsInfo.PanelControl(PANEL_ACTIVE,FCTL_GETSELECTEDPANELITEM,I,{});
					PluginPanelItem* PPI=(PluginPanelItem*)malloc(Size);

					if (PPI)
					{
						FarGetPluginPanelItem gpi={sizeof(FarGetPluginPanelItem), Size, PPI};
						PsInfo.PanelControl(PANEL_ACTIVE,FCTL_GETSELECTEDPANELITEM,I,&gpi);
						wchar_t *FullName = new wchar_t[NameLength + lstrlen(PPI->FileName) + 8];

						lstrcpy(FullName,dirInfo->Name);
						FSF.AddEndSlash(FullName);
						lstrcat(FullName,PPI->FileName);
						ProcessName(FullName,(DWORD)PPI->FileAttributes);

						delete[] FullName;

						free(PPI);
					}
				}

				free(dirInfo);
			}

			if (!CurRun)
			{
				PluginSettings settings(MainGuid, PsInfo.SettingsControl);
				settings.Set(0,L"WordDiv",Opt.WordDiv);
				settings.Set(0,L"ConvertMode",Opt.ConvertMode);
				settings.Set(0,L"ConvertModeExt",Opt.ConvertModeExt);
				settings.Set(0,L"SkipMixedCase",Opt.SkipMixedCase);
				settings.Set(0,L"ProcessSubDir",Opt.ProcessSubDir);
				settings.Set(0,L"ProcessDir",Opt.ProcessDir);
			}

			PsInfo.RestoreScreen(hScreen);
			PsInfo.PanelControl(PANEL_ACTIVE,FCTL_UPDATEPANEL,0,{});
			PsInfo.PanelControl(PANEL_ACTIVE,FCTL_REDRAWPANEL,0,{});
		}

		if (CurRun)
			memcpy(&Opt,&Backup,sizeof(Opt));
	}
}
