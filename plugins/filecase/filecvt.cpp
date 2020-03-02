#include "FileCase.hpp"
#include <DlgBuilder.hpp>
#include <PluginSettings.hpp>
#include "FileLng.hpp"
#include "guid.hpp"

int ResetButtonID=0;
int WordDivEditID=0;

INT_PTR WINAPI DlgProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void *Param2)
{
	switch (Msg)
	{
		case DN_BTNCLICK:

			if (Param1==ResetButtonID)
			{
				Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,WordDivEditID,(void *)L" _");
				return TRUE;
			}

			break;
	}

	return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void CaseConvertion()
{
	struct Options Backup;
	memcpy(&Backup,&Opt,sizeof(Backup));

	PluginDialogBuilder Builder(Info, MainGuid, DialogGuid, MFileCase, L"Contents", DlgProc);
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
			struct PanelInfo PInfo = {sizeof(PanelInfo)};
			Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELINFO,0,&PInfo);

			HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);

			const wchar_t *MsgItems[]={GetMsg(MFileCase),GetMsg(MConverting)};
			Info.Message(&MainGuid, nullptr,0,NULL,MsgItems,ARRAYSIZE(MsgItems),0);

			int Size=(int)Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,0,0);
			FarPanelDirectory* dirInfo=(FarPanelDirectory*)malloc(Size);
			if (dirInfo)
			{
				dirInfo->StructSize = sizeof(FarPanelDirectory);
				Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,Size,dirInfo);

				for (size_t I=0; I < PInfo.SelectedItemsNumber; I++)
				{
					size_t Size = Info.PanelControl(PANEL_ACTIVE,FCTL_GETSELECTEDPANELITEM,I,0);
					PluginPanelItem* PPI=(PluginPanelItem*)malloc(Size);

					if (PPI)
					{
						FarGetPluginPanelItem gpi={sizeof(FarGetPluginPanelItem), Size, PPI};
						Info.PanelControl(PANEL_ACTIVE,FCTL_GETSELECTEDPANELITEM,I,&gpi);
						wchar_t *FullName=new wchar_t[lstrlen(dirInfo->Name)+lstrlen(PPI->FileName)+8];
						if (FullName)
						{
							lstrcpy(FullName,dirInfo->Name);
							FSF.AddEndSlash(FullName);
							lstrcat(FullName,PPI->FileName);
							ProcessName(FullName,(DWORD)PPI->FileAttributes);

							delete[] FullName;
						}
						free(PPI);
					}
				}

				free(dirInfo);
			}

			if (!CurRun)
			{
				PluginSettings settings(MainGuid, Info.SettingsControl);
				settings.Set(0,L"WordDiv",Opt.WordDiv);
				settings.Set(0,L"ConvertMode",Opt.ConvertMode);
				settings.Set(0,L"ConvertModeExt",Opt.ConvertModeExt);
				settings.Set(0,L"SkipMixedCase",Opt.SkipMixedCase);
				settings.Set(0,L"ProcessSubDir",Opt.ProcessSubDir);
				settings.Set(0,L"ProcessDir",Opt.ProcessDir);
			}

			Info.RestoreScreen(hScreen);
			Info.PanelControl(PANEL_ACTIVE,FCTL_UPDATEPANEL,0,0);
			Info.PanelControl(PANEL_ACTIVE,FCTL_REDRAWPANEL,0,0);
		}

		if (CurRun)
			memcpy(&Opt,&Backup,sizeof(Opt));
	}
}
