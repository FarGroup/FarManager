/*
config.cpp

Конфигурация
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "config.hpp"
#include "keys.hpp"
#include "colors.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "filelist.hpp"
#include "panel.hpp"
#include "help.hpp"
#include "filefilter.hpp"
#include "poscache.hpp"
#include "findfile.hpp"
#include "hilight.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "palette.hpp"
#include "message.hpp"
#include "stddlg.hpp"
#include "pathmix.hpp"
#include "panelmix.hpp"
#include "strmix.hpp"
#include "udlist.hpp"
#include "FarDlgBuilder.hpp"
#include "elevation.hpp"
#include "configdb.hpp"
#include "FarGuid.hpp"
#include "vmenu2.hpp"

// Стандартный набор разделителей
static const wchar_t *WordDiv0 = L"~!%^&*()+|{}:\"<>?`-=\\[];',./";

// Стандартный набор разделителей для функции Xlat
static const wchar_t *WordDivForXlat0=L" \t!#$%^&*()+|=\\/@?";

const wchar_t *constBatchExt=L".BAT;.CMD;";

const int DefaultTabSize = 8;

wchar_t DefaultLanguage[100] = {};

#if defined(TREEFILE_PROJECT)
const wchar_t *constLocalDiskTemplate=L"%D.%SN.tree";
const wchar_t *constNetDiskTemplate=L"%D.%SN.tree";
const wchar_t *constNetPathTemplate=L"%SR.%SH.tree";
const wchar_t *constRemovableDiskTemplate=L"%SN.tree";
const wchar_t *constCDDiskTemplate=L"CD.%L.%SN.tree";
#endif

static const wchar_t szCtrlDot[]=L"Ctrl.";
static const wchar_t szRCtrlDot[]=L"RCtrl.";
static const wchar_t szCtrlShiftDot[]=L"CtrlShift.";
static const wchar_t szRCtrlShiftDot[]=L"RCtrlShift.";

// KeyName
const wchar_t NKeyColors[]=L"Colors";
const wchar_t NKeyScreen[]=L"Screen";
const wchar_t NKeyCmdline[]=L"Cmdline";
const wchar_t NKeyInterface[]=L"Interface";
const wchar_t NKeyInterfaceCompletion[]=L"Interface.Completion";
const wchar_t NKeyViewer[]=L"Viewer";
const wchar_t NKeyDialog[]=L"Dialog";
const wchar_t NKeyEditor[]=L"Editor";
const wchar_t NKeyXLat[]=L"XLat";
const wchar_t NKeySystem[]=L"System";
const wchar_t NKeySystemException[]=L"System.Exception";
const wchar_t NKeySystemKnownIDs[]=L"System.KnownIDs";
const wchar_t NKeySystemExecutor[]=L"System.Executor";
const wchar_t NKeySystemNowell[]=L"System.Nowell";
const wchar_t NKeyHelp[]=L"Help";
const wchar_t NKeyLanguage[]=L"Language";
const wchar_t NKeyConfirmations[]=L"Confirmations";
const wchar_t NKeyPluginConfirmations[]=L"PluginConfirmations";
const wchar_t NKeyPanel[]=L"Panel";
const wchar_t NKeyPanelLeft[]=L"Panel.Left";
const wchar_t NKeyPanelRight[]=L"Panel.Right";
const wchar_t NKeyPanelLayout[]=L"Panel.Layout";
const wchar_t NKeyPanelTree[]=L"Panel.Tree";
const wchar_t NKeyPanelInfo[]=L"Panel.Info";
const wchar_t NKeyLayout[]=L"Layout";
const wchar_t NKeyDescriptions[]=L"Descriptions";
const wchar_t NKeyKeyMacros[]=L"Macros";
const wchar_t NKeyPolicies[]=L"Policies";
const wchar_t NKeyFileFilter[]=L"OperationsFilter";
const wchar_t NKeyCodePages[]=L"CodePages";
const wchar_t NKeyVMenu[]=L"VMenu";
const wchar_t NKeyCommandHistory[]=L"History.CommandHistory";
const wchar_t NKeyViewEditHistory[]=L"History.ViewEditHistory";
const wchar_t NKeyFolderHistory[]=L"History.FolderHistory";
const wchar_t NKeyDialogHistory[]=L"History.DialogHistory";

const wchar_t NParamHistoryCount[]=L"Count";
const wchar_t NParamHistoryLifetime[]=L"Lifetime";

static const WCHAR _BoxSymbols[48+1] =
{
	0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
	0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
	0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
	0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
	0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
	0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
	0x0000
};

void SystemSettings()
{
	DialogBuilder Builder(MConfigSystemTitle, L"SystemSettings");

	DialogItemEx *DeleteToRecycleBin = Builder.AddCheckbox(MConfigRecycleBin, Global->Opt->DeleteToRecycleBin);
	DialogItemEx *DeleteLinks = Builder.AddCheckbox(MConfigRecycleBinLink, Global->Opt->DeleteToRecycleBinKillLink);
	DeleteLinks->Indent(4);
	Builder.LinkFlags(DeleteToRecycleBin, DeleteLinks, DIF_DISABLE);

	Builder.AddCheckbox(MConfigSystemCopy, Global->Opt->CMOpt.UseSystemCopy);
	Builder.AddCheckbox(MConfigCopySharing, Global->Opt->CMOpt.CopyOpened);
	Builder.AddCheckbox(MConfigScanJunction, Global->Opt->ScanJunction);
	Builder.AddCheckbox(MConfigCreateUppercaseFolders, Global->Opt->CreateUppercaseFolders);
	Builder.AddCheckbox(MConfigSmartFolderMonitor, Global->Opt->SmartFolderMonitor);

	Builder.AddCheckbox(MConfigSaveHistory, Global->Opt->SaveHistory);
	Builder.AddCheckbox(MConfigSaveFoldersHistory, Global->Opt->SaveFoldersHistory);
	Builder.AddCheckbox(MConfigSaveViewHistory, Global->Opt->SaveViewHistory);
	Builder.AddCheckbox(MConfigRegisteredTypes, Global->Opt->UseRegisteredTypes);
	Builder.AddCheckbox(MConfigCloseCDGate, Global->Opt->CloseCDGate);
	Builder.AddCheckbox(MConfigUpdateEnvironment, Global->Opt->UpdateEnvironment);
	Builder.AddText(MConfigElevation);
	Builder.AddCheckbox(MConfigElevationModify, Global->Opt->StoredElevationMode, ELEVATION_MODIFY_REQUEST)->Indent(4);
	Builder.AddCheckbox(MConfigElevationRead, Global->Opt->StoredElevationMode, ELEVATION_READ_REQUEST)->Indent(4);
	Builder.AddCheckbox(MConfigElevationUsePrivileges, Global->Opt->StoredElevationMode, ELEVATION_USE_PRIVILEGES)->Indent(4);
	Builder.AddCheckbox(MConfigAutoSave, Global->Opt->AutoSaveSetup);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		Global->Opt->ElevationMode = Global->Opt->StoredElevationMode;
	}
}


void PanelSettings()
{
	DialogBuilder Builder(MConfigPanelTitle, L"PanelSettings");
	BOOL AutoUpdate = Global->Opt->AutoUpdateLimit;

	Builder.AddCheckbox(MConfigHidden, Global->Opt->ShowHidden);
	Builder.AddCheckbox(MConfigHighlight, Global->Opt->Highlight);
	Builder.AddCheckbox(MConfigSelectFolders, Global->Opt->SelectFolders);
	Builder.AddCheckbox(MConfigRightClickSelect, Global->Opt->RightClickSelect);
	Builder.AddCheckbox(MConfigSortFolderExt, Global->Opt->SortFolderExt);
	Builder.AddCheckbox(MConfigReverseSort, Global->Opt->ReverseSort);

	DialogItemEx *AutoUpdateEnabled = Builder.AddCheckbox(MConfigAutoUpdateLimit, &AutoUpdate);
	DialogItemEx *AutoUpdateLimit = Builder.AddIntEditField(Global->Opt->AutoUpdateLimit, 6);
	Builder.LinkFlags(AutoUpdateEnabled, AutoUpdateLimit, DIF_DISABLE, false);
	DialogItemEx *AutoUpdateText = Builder.AddTextBefore(AutoUpdateLimit, MConfigAutoUpdateLimit2);
	AutoUpdateLimit->Indent(4);
	AutoUpdateText->Indent(4);
	Builder.AddCheckbox(MConfigAutoUpdateRemoteDrive, Global->Opt->AutoUpdateRemoteDrive);

	Builder.AddSeparator();
	Builder.AddCheckbox(MConfigShowColumns, Global->Opt->ShowColumnTitles);
	Builder.AddCheckbox(MConfigShowStatus, Global->Opt->ShowPanelStatus);
	Builder.AddCheckbox(MConfigDetailedJunction, Global->Opt->PanelDetailedJunction);
	Builder.AddCheckbox(MConfigShowTotal, Global->Opt->ShowPanelTotals);
	Builder.AddCheckbox(MConfigShowFree, Global->Opt->ShowPanelFree);
	Builder.AddCheckbox(MConfigShowScrollbar, Global->Opt->ShowPanelScrollbar);
	Builder.AddCheckbox(MConfigShowScreensNumber, Global->Opt->ShowScreensNumber);
	Builder.AddCheckbox(MConfigShowSortMode, Global->Opt->ShowSortMode);
	Builder.AddCheckbox(MConfigShowDotsInRoot, Global->Opt->ShowDotsInRoot);
	Builder.AddCheckbox(MConfigHighlightColumnSeparator, Global->Opt->HighlightColumnSeparator);
	Builder.AddCheckbox(MConfigDoubleGlobalColumnSeparator, Global->Opt->DoubleGlobalColumnSeparator);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (!AutoUpdate)
			Global->Opt->AutoUpdateLimit = 0;

	//  FrameManager->RefreshFrame();
		Global->CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->Redraw();
	}
}

void TreeSettings()
{
	DialogBuilder Builder(MConfigTreeTitle, L"TreeSettings");

	DialogItemEx *TemplateEdit;

	Builder.AddCheckbox(MConfigTreeAutoChange, Global->Opt->Tree.AutoChangeFolder);

	TemplateEdit = Builder.AddIntEditField(Global->Opt->Tree.MinTreeCount, 3);
	Builder.AddTextBefore(TemplateEdit, MConfigTreeLabelMinFolder);

#if defined(TREEFILE_PROJECT)
	DialogItemEx *Checkbox;

	Builder.AddSeparator(MConfigTreeLabel1);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelLocalDisk, &Global->Opt->Tree.LocalDisk);
	TemplateEdit = Builder.AddEditField(&Global->Opt->Tree.strLocalDisk, 36);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelNetDisk, &Global->Opt->Tree.NetDisk);
	TemplateEdit = Builder.AddEditField(&Global->Opt->Tree.strNetDisk, 36);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelNetPath, &Global->Opt->Tree.NetPath);
	TemplateEdit = Builder.AddEditField(&Global->Opt->Tree.strNetPath, 36);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelRemovableDisk, &Global->Opt->Tree.RemovableDisk);
	TemplateEdit = Builder.AddEditField(&Global->Opt->Tree.strRemovableDisk, 36);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelCDDisk, &Global->Opt->Tree.CDDisk);
	TemplateEdit = Builder.AddEditField(&Global->Opt->Tree.strCDDisk, 36);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Builder.AddText(MConfigTreeLabelSaveLocalPath);
	Builder.AddEditField(&Global->Opt->Tree.strSaveLocalPath, 40);

	Builder.AddText(MConfigTreeLabelSaveNetPath);
	Builder.AddEditField(&Global->Opt->Tree.strSaveNetPath, 40);

	Builder.AddText(MConfigTreeLabelExceptPath);
	Builder.AddEditField(&Global->Opt->Tree.strExceptPath, 40);
#endif

	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		Global->CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->Redraw();
	}
}


/* $ 17.12.2001 IS
   Настройка средней кнопки мыши для панелей. Воткнем пока сюда, потом надо
   переехать в специальный диалог по программированию мыши.
*/
void InterfaceSettings()
{
	DialogBuilder Builder(MConfigInterfaceTitle, L"InterfSettings");

	Builder.AddCheckbox(MConfigClock, Global->Opt->Clock);
	Builder.AddCheckbox(MConfigViewerEditorClock, Global->Opt->ViewerEditorClock);
	Builder.AddCheckbox(MConfigMouse, Global->Opt->Mouse);
	Builder.AddCheckbox(MConfigKeyBar, Global->Opt->ShowKeyBar);
	Builder.AddCheckbox(MConfigMenuBar, Global->Opt->ShowMenuBar);
	DialogItemEx *SaverCheckbox = Builder.AddCheckbox(MConfigSaver, Global->Opt->ScreenSaver);

	DialogItemEx *SaverEdit = Builder.AddIntEditField(Global->Opt->ScreenSaverTime, 2);
	SaverEdit->Indent(4);
	Builder.AddTextAfter(SaverEdit, MConfigSaverMinutes);
	Builder.LinkFlags(SaverCheckbox, SaverEdit, DIF_DISABLE);

	Builder.AddCheckbox(MConfigCopyTotal, Global->Opt->CMOpt.CopyShowTotal);
	Builder.AddCheckbox(MConfigCopyTimeRule, Global->Opt->CMOpt.CopyTimeRule);
	Builder.AddCheckbox(MConfigDeleteTotal, Global->Opt->DelOpt.DelShowTotal);
	Builder.AddCheckbox(MConfigPgUpChangeDisk, Global->Opt->PgUpChangeDisk);
	Builder.AddCheckbox(MConfigClearType, Global->Opt->ClearType);
	DialogItemEx* SetIconCheck = Builder.AddCheckbox(MConfigSetConsoleIcon, Global->Opt->SetIcon);
	DialogItemEx* SetAdminIconCheck = Builder.AddCheckbox(MConfigSetAdminConsoleIcon, Global->Opt->SetAdminIcon);
	SetAdminIconCheck->Indent(4);
	Builder.LinkFlags(SetIconCheck, SetAdminIconCheck, DIF_DISABLE);
	Builder.AddText(MConfigTitleAddons);
	Builder.AddEditField(Global->Opt->strTitleAddons, 47);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (Global->Opt->CMOpt.CopyTimeRule)
			Global->Opt->CMOpt.CopyTimeRule = 3;

		SetFarConsoleMode();
		Global->CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->SetScreenPosition();
		// $ 10.07.2001 SKV ! надо это делать, иначе если кейбар спрятали, будет полный рамс.
		Global->CtrlObject->Cp()->Redraw();
	}
}

void AutoCompleteSettings()
{
	DialogBuilder Builder(MConfigAutoCompleteTitle, L"AutoCompleteSettings");
	DialogItemEx *ListCheck=Builder.AddCheckbox(MConfigAutoCompleteShowList, Global->Opt->AutoComplete.ShowList);
	DialogItemEx *ModalModeCheck=Builder.AddCheckbox(MConfigAutoCompleteModalList, Global->Opt->AutoComplete.ModalList);
	ModalModeCheck->Indent(4);
	Builder.AddCheckbox(MConfigAutoCompleteAutoAppend, Global->Opt->AutoComplete.AppendCompletion);
	Builder.LinkFlags(ListCheck, ModalModeCheck, DIF_DISABLE);
	Builder.AddOKCancel();
	Builder.ShowDialog();
}

void InfoPanelSettings()
{
	DialogBuilderListItem UNListItems[]=
	{
		{ MConfigInfoPanelUNUnknown, NameUnknown },                            // 0  - unknown name type
		{ MConfigInfoPanelUNFullyQualifiedDN, NameFullyQualifiedDN },          // 1  - CN=John Doe, OU=Software, OU=Engineering, O=Widget, C=US
		{ MConfigInfoPanelUNSamCompatible, NameSamCompatible },                // 2  - Engineering\JohnDoe, If the user account is not in a domain, only NameSamCompatible is supported.
		{ MConfigInfoPanelUNDisplay, NameDisplay },                            // 3  - Probably "John Doe" but could be something else.  I.e. The display name is not necessarily the defining RDN.
		{ MConfigInfoPanelUNUniqueId, NameUniqueId },                          // 6  - String-ized GUID as returned by IIDFromString(). eg: {4fa050f0-f561-11cf-bdd9-00aa003a77b6}
		{ MConfigInfoPanelUNCanonical, NameCanonical },                        // 7  - engineering.widget.com/software/John Doe
		{ MConfigInfoPanelUNUserPrincipal, NameUserPrincipal },                // 8  - someone@example.com
		{ MConfigInfoPanelUNServicePrincipal, NameServicePrincipal },          // 10 - www/srv.engineering.com/engineering.com
		{ MConfigInfoPanelUNDnsDomain, NameDnsDomain },                        // 12 - DNS domain name + SAM username eg: engineering.widget.com\JohnDoe
	};
	DialogBuilderListItem CNListItems[]=
	{
		{ MConfigInfoPanelCNNetBIOS, ComputerNameNetBIOS },                                     // The NetBIOS name of the local computer or the cluster associated with the local computer. This name is limited to MAX_COMPUTERNAME_LENGTH + 1 characters and may be a truncated version of the DNS host name. For example, if the DNS host name is "corporate-mail-server", the NetBIOS name would be "corporate-mail-".
		{ MConfigInfoPanelCNDnsHostname, ComputerNameDnsHostname },                             // The DNS name of the local computer or the cluster associated with the local computer.
		{ MConfigInfoPanelCNDnsDomain, ComputerNameDnsDomain },                                 // The name of the DNS domain assigned to the local computer or the cluster associated with the local computer.
		{ MConfigInfoPanelCNDnsFullyQualified, ComputerNameDnsFullyQualified },                 // The fully-qualified DNS name that uniquely identifies the local computer or the cluster associated with the local computer. This name is a combination of the DNS host name and the DNS domain name, using the form HostName.DomainName. For example, if the DNS host name is "corporate-mail-server" and the DNS domain name is "microsoft.com", the fully qualified DNS name is "corporate-mail-server.microsoft.com".
		{ MConfigInfoPanelCNPhysicalNetBIOS, ComputerNamePhysicalNetBIOS },                     // The NetBIOS name of the local computer. On a cluster, this is the NetBIOS name of the local node on the cluster.
		{ MConfigInfoPanelCNPhysicalDnsHostname, ComputerNamePhysicalDnsHostname },             // The DNS host name of the local computer. On a cluster, this is the DNS host name of the local node on the cluster.
		{ MConfigInfoPanelCNPhysicalDnsDomain, ComputerNamePhysicalDnsDomain },                 // The name of the DNS domain assigned to the local computer. On a cluster, this is the DNS domain of the local node on the cluster.
		{ MConfigInfoPanelCNPhysicalDnsFullyQualified, ComputerNamePhysicalDnsFullyQualified }, // The fully-qualified DNS name that uniquely identifies the computer. On a cluster, this is the fully qualified DNS name of the local node on the cluster. The fully qualified DNS name is a combination of the DNS host name and the DNS domain name, using the form HostName.DomainName.
	};

	DialogBuilder Builder(MConfigInfoPanelTitle, L"InfoPanelSettings");
	Builder.AddCheckbox(MConfigInfoPanelShowPowerStatus, Global->Opt->InfoPanel.ShowPowerStatus);
	Builder.AddCheckbox(MConfigInfoPanelShowCDInfo, Global->Opt->InfoPanel.ShowCDInfo);
	Builder.AddText(MConfigInfoPanelCNTitle);
	Builder.AddComboBox(Global->Opt->InfoPanel.ComputerNameFormat, 50, CNListItems, ARRAYSIZE(CNListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddText(MConfigInfoPanelUNTitle);
	Builder.AddComboBox(Global->Opt->InfoPanel.UserNameFormat, 50, UNListItems, ARRAYSIZE(UNListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		bool needRedraw=false;
		if (Global->CtrlObject->Cp()->LeftPanel->GetType() == INFO_PANEL)
		{
			Global->CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
			needRedraw=true;
		}
		if (Global->CtrlObject->Cp()->RightPanel->GetType() == INFO_PANEL)
		{
			Global->CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
			needRedraw=true;
		}
		if (needRedraw)
		{
			//Global->CtrlObject->Cp()->SetScreenPosition();
			Global->CtrlObject->Cp()->Redraw();
		}
	}
}

void ApplyDefaultMaskGroups()
{
	struct MaskGroups
	{
		const wchar_t* Group;
		const wchar_t* Mask;
	}
	Sets[] =
	{
		{L"arc", L"*.rar,*.zip,*.[zj],*.[bg7x]z,*.[bg]zip,*.tar,*.t[agbx]z,*.ar[cj],*.r[0-9][0-9],*.a[0-9][0-9],*.bz2,*.cab,*.msi,*.jar,*.lha,*.lzh,*.ha,*.ac[bei],*.pa[ck],*.rk,*.cpio,*.rpm,*.zoo,*.hqx,*.sit,*.ice,*.uc2,*.ain,*.imp,*.777,*.ufa,*.boa,*.bs[2a],*.sea,*.hpk,*.ddi,*.x2,*.rkv,*.[lw]sz,*.h[ay]p,*.lim,*.sqz,*.chz"},
		{L"temp", L"*.bak,*.tmp"},
		{L"exec", L"*.exe,*.com,*.bat,*.cmd,%PATHEXT%"},
	};

	for (size_t i = 0; i < ARRAYSIZE(Sets); ++i)
	{
		Global->Db->GeneralCfg()->SetValue(L"Masks", Sets[i].Group, Sets[i].Mask);
	}
}

void FillMasksMenu(VMenu2& MasksMenu, int SelPos = 0)
{
	MasksMenu.DeleteItems();
	string Name, Value;
	for(DWORD i = 0; Global->Db->GeneralCfg()->EnumValues(L"Masks", i, Name, Value); ++i)
	{
		MenuItemEx Item = {};
		string DisplayName(Name);
		const int NameWidth = 10;
		TruncStr(DisplayName, NameWidth);
		Item.strName = FormatString() << fmt::ExactWidth(NameWidth) << fmt::LeftAlign() << DisplayName << L' ' << BoxSymbols[BS_V1] << L' ' << Value;
		Item.UserData = const_cast<wchar_t*>(Name.CPtr());
		Item.UserDataSize = (Name.GetLength()+1)*sizeof(wchar_t);
		MasksMenu.AddItem(&Item);
	}
	MasksMenu.SetSelectPos(SelPos, 0);
}

void MaskGroupsSettings()
{
	VMenu2 MasksMenu(MSG(MMenuMaskGroups), nullptr, 0, 0, VMENU_WRAPMODE|VMENU_SHOWAMPERSAND);
	MasksMenu.SetBottomTitle(MSG(MMaskGroupBottom));
	MasksMenu.SetHelp(L"MaskGroupsSettings");
	FillMasksMenu(MasksMenu);
	MasksMenu.SetPosition(-1, -1, -1, -1);

	bool Changed = false;
	bool Filter = false;
	for(;;)
	{
		MasksMenu.Run([&](int Key)->int
		{
			if(Filter)
			{
				if(Key == KEY_ESC || Key == KEY_F10 || Key == KEY_ENTER || Key == KEY_NUMENTER)
				{
					Filter = false;
					for (int i = 0; i < MasksMenu.GetItemCount(); ++i)
					{
						MasksMenu.UpdateItemFlags(i, MasksMenu.GetItemPtr(i)->Flags&~MIF_HIDDEN);
					}
					MasksMenu.SetPosition(-1, -1, -1, -1);
					MasksMenu.SetTitle(MSG(MMenuMaskGroups));
					MasksMenu.SetBottomTitle(MSG(MMaskGroupBottom));
				}
				return true;
			}
			int ItemPos = MasksMenu.GetSelectPos();
			void* Data = MasksMenu.GetUserData(nullptr, 0, ItemPos);
			const wchar_t* Item = static_cast<const wchar_t*>(Data);
			int KeyProcessed = 1;
			switch (Key)
			{
			case KEY_NUMDEL:
			case KEY_DEL:
				if(Item && !Message(0,2,MSG(MMenuMaskGroups),MSG(MMaskGroupAskDelete), Item, MSG(MDelete), MSG(MCancel)))
				{
					Global->Db->GeneralCfg()->DeleteValue(L"Masks", Item);
					Changed = true;
				}
				break;

			case KEY_NUMPAD0:
			case KEY_INS:
				Item = nullptr;
			case KEY_ENTER:
			case KEY_NUMENTER:
			case KEY_F4:
				{
					string Name(Item), Value;
					if(Item)
					{
						Global->Db->GeneralCfg()->GetValue(L"Masks", Name, Value, L"");
					}
					DialogBuilder Builder(MMenuMaskGroups, nullptr);
					Builder.AddText(MMaskGroupName);
					Builder.AddEditField(&Name, 60);
					Builder.AddText(MMaskGroupMasks);
					Builder.AddEditField(&Value, 60);
					Builder.AddOKCancel();
					if(Builder.ShowDialog())
					{
						if(Item)
						{
							Global->Db->GeneralCfg()->DeleteValue(L"Masks", Item);
						}
						Global->Db->GeneralCfg()->SetValue(L"Masks", Name, Value);
						Changed = true;
					}
				}
				break;

			case KEY_CTRLR:
			case KEY_RCTRLR:
				{
					if (!Message(MSG_WARNING, 2,
						MSG(MMenuMaskGroups),
						MSG(MMaskGroupRestore),
						MSG(MYes),MSG(MCancel)))
					{
						ApplyDefaultMaskGroups();
						Changed = true;
					}
				}
				break;

			case KEY_F7:
				{
					string Value;
					DialogBuilder Builder(MFileFilterTitle, nullptr);
					Builder.AddText(MMaskGroupFindMask);
					Builder.AddEditField(&Value, 60, L"MaskGroupsFindMask");
					Builder.AddOKCancel();
					if(Builder.ShowDialog())
					{
						for (int i=0; i < MasksMenu.GetItemCount(); ++i)
						{
							string CurrentMasks;
							Global->Db->GeneralCfg()->GetValue(L"Masks", static_cast<const wchar_t*>(MasksMenu.GetUserData(nullptr, 0, i)), CurrentMasks, L"");
							CFileMask Masks;
							Masks.Set(CurrentMasks, 0);
							if(!Masks.Compare(Value))
							{
								MasksMenu.UpdateItemFlags(i, MasksMenu.GetItemPtr(i)->Flags|MIF_HIDDEN);
							}
						}
						MasksMenu.SetPosition(-1, -1, -1, -1);
						MasksMenu.SetTitle(Value);
						MasksMenu.SetBottomTitle(LangString(MMaskGroupTotal) << MasksMenu.GetShowItemCount());
						Filter = true;
					}
				}
				break;

			default:
				KeyProcessed = 0;
			}

			if(Changed)
			{
				Changed = false;

				FillMasksMenu(MasksMenu, MasksMenu.GetSelectPos());
				MasksMenu.SetPosition(-1, -1, -1, -1);
				Global->CtrlObject->HiFiles->UpdateHighlighting(true);
			}
			return KeyProcessed;
		});
		if (MasksMenu.GetExitCode()!=-1)
		{
			MasksMenu.Key(KEY_F4);
			continue;
		}
		break;
	}
}

void DialogSettings()
{
	DialogBuilder Builder(MConfigDlgSetsTitle, L"DialogSettings");

	Builder.AddCheckbox(MConfigDialogsEditHistory, Global->Opt->Dialogs.EditHistory);
	Builder.AddCheckbox(MConfigDialogsEditBlock, Global->Opt->Dialogs.EditBlock);
	Builder.AddCheckbox(MConfigDialogsDelRemovesBlocks, Global->Opt->Dialogs.DelRemovesBlocks);
	Builder.AddCheckbox(MConfigDialogsAutoComplete, Global->Opt->Dialogs.AutoComplete);
	Builder.AddCheckbox(MConfigDialogsEULBsClear, Global->Opt->Dialogs.EULBsClear);
	Builder.AddCheckbox(MConfigDialogsMouseButton, Global->Opt->Dialogs.MouseButton);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (Global->Opt->Dialogs.MouseButton )
			Global->Opt->Dialogs.MouseButton = 0xFFFF;
	}
}

void VMenuSettings()
{
	DialogBuilderListItem CAListItems[]=
	{
		{ MConfigVMenuClickCancel, VMENUCLICK_CANCEL },  // Cancel menu
		{ MConfigVMenuClickApply,  VMENUCLICK_APPLY  },  // Execute selected item
		{ MConfigVMenuClickIgnore, VMENUCLICK_IGNORE },  // Do nothing
	};

	DialogBuilder Builder(MConfigVMenuTitle, L"VMenuSettings");

	Builder.AddText(MConfigVMenuLBtnClick);
	Builder.AddComboBox(Global->Opt->VMenu.LBtnClick, 40, CAListItems, ARRAYSIZE(CAListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddText(MConfigVMenuRBtnClick);
	Builder.AddComboBox(Global->Opt->VMenu.RBtnClick, 40, CAListItems, ARRAYSIZE(CAListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddText(MConfigVMenuMBtnClick);
	Builder.AddComboBox(Global->Opt->VMenu.MBtnClick, 40, CAListItems, ARRAYSIZE(CAListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddOKCancel();
	Builder.ShowDialog();
}

void CmdlineSettings()
{
	DialogBuilder Builder(MConfigCmdlineTitle, L"CmdlineSettings");

	Builder.AddCheckbox(MConfigCmdlineEditBlock, Global->Opt->CmdLine.EditBlock);
	Builder.AddCheckbox(MConfigCmdlineDelRemovesBlocks, Global->Opt->CmdLine.DelRemovesBlocks);
	Builder.AddCheckbox(MConfigCmdlineAutoComplete, Global->Opt->CmdLine.AutoComplete);
	DialogItemEx *UsePromptFormat = Builder.AddCheckbox(MConfigCmdlineUsePromptFormat, Global->Opt->CmdLine.UsePromptFormat);
	DialogItemEx *PromptFormat = Builder.AddEditField(Global->Opt->CmdLine.strPromptFormat, 33);
	PromptFormat->Indent(4);
	Builder.LinkFlags(UsePromptFormat, PromptFormat, DIF_DISABLE);

	UsePromptFormat = Builder.AddCheckbox(MConfigCmdlineUseHomeDir, Global->Opt->Exec.UseHomeDir);
	PromptFormat = Builder.AddEditField(Global->Opt->Exec.strHomeDir, 33);
	PromptFormat->Indent(4);
	Builder.LinkFlags(UsePromptFormat, PromptFormat, DIF_DISABLE);

	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		Global->CtrlObject->CmdLine->SetPersistentBlocks(Global->Opt->CmdLine.EditBlock);
		Global->CtrlObject->CmdLine->SetDelRemovesBlocks(Global->Opt->CmdLine.DelRemovesBlocks);
		Global->CtrlObject->CmdLine->SetAutoComplete(Global->Opt->CmdLine.AutoComplete);
	}
}

void SetConfirmations()
{
	DialogBuilder Builder(MSetConfirmTitle, L"ConfirmDlg");

	Builder.AddCheckbox(MSetConfirmCopy, Global->Opt->Confirm.Copy);
	Builder.AddCheckbox(MSetConfirmMove, Global->Opt->Confirm.Move);
	Builder.AddCheckbox(MSetConfirmRO, Global->Opt->Confirm.RO);
	Builder.AddCheckbox(MSetConfirmDrag, Global->Opt->Confirm.Drag);
	Builder.AddCheckbox(MSetConfirmDelete, Global->Opt->Confirm.Delete);
	Builder.AddCheckbox(MSetConfirmDeleteFolders, Global->Opt->Confirm.DeleteFolder);
	Builder.AddCheckbox(MSetConfirmEsc, Global->Opt->Confirm.Esc);
	Builder.AddCheckbox(MSetConfirmRemoveConnection, Global->Opt->Confirm.RemoveConnection);
	Builder.AddCheckbox(MSetConfirmRemoveSUBST, Global->Opt->Confirm.RemoveSUBST);
	Builder.AddCheckbox(MSetConfirmDetachVHD, Global->Opt->Confirm.DetachVHD);
	Builder.AddCheckbox(MSetConfirmRemoveHotPlug, Global->Opt->Confirm.RemoveHotPlug);
	Builder.AddCheckbox(MSetConfirmAllowReedit, Global->Opt->Confirm.AllowReedit);
	Builder.AddCheckbox(MSetConfirmHistoryClear, Global->Opt->Confirm.HistoryClear);
	Builder.AddCheckbox(MSetConfirmExit, Global->Opt->Confirm.Exit);
	Builder.AddOKCancel();

	Builder.ShowDialog();
}

void PluginsManagerSettings()
{
	DialogBuilder Builder(MPluginsManagerSettingsTitle, L"PluginsManagerSettings");
#ifndef NO_WRAPPER
	Builder.AddCheckbox(MPluginsManagerOEMPluginsSupport, Global->Opt->LoadPlug.OEMPluginsSupport);
#endif // NO_WRAPPER
	Builder.AddCheckbox(MPluginsManagerScanSymlinks, Global->Opt->LoadPlug.ScanSymlinks);
	Builder.AddSeparator(MPluginConfirmationTitle);
	Builder.AddCheckbox(MPluginsManagerOFP, Global->Opt->PluginConfirm.OpenFilePlugin);
	DialogItemEx *StandardAssoc = Builder.AddCheckbox(MPluginsManagerStdAssoc, Global->Opt->PluginConfirm.StandardAssociation);
	DialogItemEx *EvenIfOnlyOne = Builder.AddCheckbox(MPluginsManagerEvenOne, Global->Opt->PluginConfirm.EvenIfOnlyOnePlugin);
	StandardAssoc->Indent(2);
	EvenIfOnlyOne->Indent(4);

	Builder.AddCheckbox(MPluginsManagerSFL, Global->Opt->PluginConfirm.SetFindList);
	Builder.AddCheckbox(MPluginsManagerPF, Global->Opt->PluginConfirm.Prefix);
	Builder.AddOKCancel();

	Builder.ShowDialog();
}


void SetDizConfig()
{
	DialogBuilder Builder(MCfgDizTitle, L"FileDiz");

	Builder.AddText(MCfgDizListNames);
	Builder.AddEditField(Global->Opt->Diz.strListNames, 65);
	Builder.AddSeparator();

	Builder.AddCheckbox(MCfgDizSetHidden, Global->Opt->Diz.SetHidden);
	Builder.AddCheckbox(MCfgDizROUpdate, Global->Opt->Diz.ROUpdate);
	DialogItemEx *StartPos = Builder.AddIntEditField(Global->Opt->Diz.StartPos, 2);
	Builder.AddTextAfter(StartPos, MCfgDizStartPos);
	Builder.AddSeparator();

	static int DizOptions[] = { MCfgDizNotUpdate, MCfgDizUpdateIfDisplayed, MCfgDizAlwaysUpdate };
	Builder.AddRadioButtons(Global->Opt->Diz.UpdateMode, 3, DizOptions);
	Builder.AddSeparator();

	Builder.AddCheckbox(MCfgDizAnsiByDefault, Global->Opt->Diz.AnsiByDefault);
	Builder.AddCheckbox(MCfgDizSaveInUTF, Global->Opt->Diz.SaveInUTF);
	Builder.AddOKCancel();
	Builder.ShowDialog();
}

void ViewerConfig(ViewerOptions &ViOpt,bool Local)
{
	DialogBuilder Builder(MViewConfigTitle, L"ViewerSettings");
	if (!Local)
	{
		Builder.AddCheckbox(MViewConfigExternalF3, Global->Opt->ViOpt.UseExternalViewer);
		Builder.AddText(MViewConfigExternalCommand);
		Builder.AddEditField(Global->Opt->strExternalViewer, 64, L"ExternalViewer", DIF_EDITPATH|DIF_EDITPATHEXEC);
		Builder.AddSeparator(MViewConfigInternal);
	}

	Builder.StartColumns();
	Builder.AddCheckbox(MViewConfigPersistentSelection, ViOpt.PersistentBlocks);
	DialogItemEx *SavePos = Builder.AddCheckbox(MViewConfigSavePos, Global->Opt->ViOpt.SavePos); // can't be local
	Builder.AddCheckbox(MViewConfigSaveCodepage, ViOpt.SaveCodepage);
	Builder.AddCheckbox(MViewConfigEditAutofocus, ViOpt.SearchEditFocus);
	DialogItemEx *TabSize = Builder.AddIntEditField(ViOpt.TabSize, 3);
	Builder.AddTextAfter(TabSize, MViewConfigTabSize);
	Builder.ColumnBreak();
	Builder.AddCheckbox(MViewConfigArrows, ViOpt.ShowArrows);
	DialogItemEx *SaveShortPos = Builder.AddCheckbox(MViewConfigSaveShortPos, Global->Opt->ViOpt.SaveShortPos); // can't be local
	Builder.LinkFlags(SavePos, SaveShortPos, DIF_DISABLE);
	Builder.AddCheckbox(MViewConfigSaveWrapMode, ViOpt.SaveWrapMode);
	Builder.AddCheckbox(MViewConfigVisible0x00, ViOpt.Visible0x00);
	Builder.AddCheckbox(MViewConfigScrollbar, ViOpt.ShowScrollbar);
	Builder.EndColumns();

	if (!Local)
	{
		Builder.AddEmptyLine();
		DialogItemEx *MaxLineSize = Builder.AddIntEditField(Global->Opt->ViOpt.MaxLineSize, 6);
		Builder.AddTextAfter(MaxLineSize, MViewConfigMaxLineSize);
		Builder.AddCheckbox(MViewAutoDetectCodePage, Global->Opt->ViOpt.AutoDetectCodePage);
		Builder.AddCheckbox(MViewConfigAnsiCodePageAsDefault, Global->Opt->ViOpt.AnsiCodePageAsDefault);
	}
	Builder.AddOKCancel();
	if (Builder.ShowDialog())
	{
		if (Global->Opt->ViOpt.SavePos)
			ViOpt.SaveCodepage = true; // codepage is part of saved position
		if (ViOpt.TabSize<1 || ViOpt.TabSize>512)
			ViOpt.TabSize = DefaultTabSize;
		if (!Global->Opt->ViOpt.MaxLineSize)
			Global->Opt->ViOpt.MaxLineSize = ViewerOptions::eDefLineSize;
		else if (Global->Opt->ViOpt.MaxLineSize < ViewerOptions::eMinLineSize)
			Global->Opt->ViOpt.MaxLineSize = ViewerOptions::eMinLineSize;
		else if (Global->Opt->ViOpt.MaxLineSize > ViewerOptions::eMaxLineSize)
			Global->Opt->ViOpt.MaxLineSize = ViewerOptions::eMaxLineSize;
	}
}

void EditorConfig(EditorOptions &EdOpt,bool Local)
{
	DialogBuilder Builder(MEditConfigTitle, L"EditorSettings");
	if (!Local)
	{
		Builder.AddCheckbox(MEditConfigEditorF4, Global->Opt->EdOpt.UseExternalEditor);
		Builder.AddText(MEditConfigEditorCommand);
		Builder.AddEditField(Global->Opt->strExternalEditor, 64, L"ExternalEditor", DIF_EDITPATH|DIF_EDITPATHEXEC);
		Builder.AddSeparator(MEditConfigInternal);
	}

	Builder.AddText(MEditConfigExpandTabsTitle);
	DialogBuilderListItem ExpandTabsItems[] = {
		{ MEditConfigDoNotExpandTabs, EXPAND_NOTABS },
		{ MEditConfigExpandTabs, EXPAND_NEWTABS },
		{ MEditConfigConvertAllTabsToSpaces, EXPAND_ALLTABS }
	};
	Builder.AddComboBox(EdOpt.ExpandTabs, 64, ExpandTabsItems, 3, DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);

	Builder.StartColumns();
	Builder.AddCheckbox(MEditConfigPersistentBlocks, EdOpt.PersistentBlocks);
	Builder.AddCheckbox(MEditConfigDelRemovesBlocks, EdOpt.DelRemovesBlocks);
	Builder.AddCheckbox(MEditConfigAutoIndent, EdOpt.AutoIndent);
	DialogItemEx *TabSize = Builder.AddIntEditField(EdOpt.TabSize, 3);
	Builder.AddTextAfter(TabSize, MEditConfigTabSize);
	Builder.AddCheckbox(MEditShowWhiteSpace, EdOpt.ShowWhiteSpace);
	Builder.AddCheckbox(MEditConfigScrollbar, EdOpt.ShowScrollBar);
	Builder.ColumnBreak();
	Builder.AddCheckbox(MEditCursorBeyondEnd, EdOpt.CursorBeyondEOL);
	DialogItemEx *SavePos = Builder.AddCheckbox(MEditConfigSavePos, EdOpt.SavePos);
	DialogItemEx *SaveShortPos = Builder.AddCheckbox(MEditConfigSaveShortPos, EdOpt.SaveShortPos);
	Builder.LinkFlags(SavePos, SaveShortPos, DIF_DISABLE);
	Builder.AddCheckbox(MEditConfigPickUpWord, EdOpt.SearchPickUpWord);
	Builder.AddCheckbox(MEditConfigSelFound, EdOpt.SearchSelFound);
	Builder.AddCheckbox(MEditConfigCursorAtEnd, EdOpt.SearchCursorAtEnd);
	Builder.EndColumns();

	if (!Local)
	{
		Builder.AddEmptyLine();
		Builder.AddCheckbox(MEditShareWrite, EdOpt.EditOpenedForWrite);
		Builder.AddCheckbox(MEditLockROFileModification, EdOpt.ReadOnlyLock, 1);
		Builder.AddCheckbox(MEditWarningBeforeOpenROFile, EdOpt.ReadOnlyLock, 2);
		Builder.AddCheckbox(MEditAutoDetectCodePage, EdOpt.AutoDetectCodePage);
		Builder.AddCheckbox(MEditConfigAnsiCodePageAsDefault, EdOpt.AnsiCodePageAsDefault);
		Builder.AddCheckbox(MEditConfigAnsiCodePageForNewFile, EdOpt.AnsiCodePageForNewFile);
	}

	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (EdOpt.TabSize<1 || EdOpt.TabSize>512)
			EdOpt.TabSize = DefaultTabSize;
	}
}


void SetFolderInfoFiles()
{
	string strFolderInfoFiles;

	if (GetString(MSG(MSetFolderInfoTitle),MSG(MSetFolderInfoNames),L"FolderInfoFiles",
	              Global->Opt->InfoPanel.strFolderInfoFiles,strFolderInfoFiles,L"FolderDiz",FIB_ENABLEEMPTY|FIB_BUTTONS))
	{
		Global->Opt->InfoPanel.strFolderInfoFiles = strFolderInfoFiles;

		if (Global->CtrlObject->Cp()->LeftPanel->GetType() == INFO_PANEL)
			Global->CtrlObject->Cp()->LeftPanel->Update(0);

		if (Global->CtrlObject->Cp()->RightPanel->GetType() == INFO_PANEL)
			Global->CtrlObject->Cp()->RightPanel->Update(0);
	}
}

// Структура, описывающая всю конфигурацию(!)
struct FARConfigItem
{
	size_t ApiRoot;
	const wchar_t *KeyName;
	const wchar_t *ValName;
	Option* Value;   // адрес переменной, куда помещаем данные
	Option::OptionType ValueType;  // TYPE_BOOLEAN, TYPE_BOOLEAN3, TYPE_INTEGER, TYPE_STRING
	union
	{
		const void* Default;
		const wchar_t* sDefault;
		int iDefault;
		bool bDefault;
	};

	bool Edit(bool Hex)
	{
		if(ValueType == Option::TYPE_BOOLEAN)
		{
			*static_cast<BoolOption*>(Value) = !*static_cast<BoolOption*>(Value);
			return true;
		}
		else if(ValueType == Option::TYPE_BOOLEAN3)
		{
			++(*static_cast<Bool3Option*>(Value));
			return true;
		}
		else
		{
			DialogBuilder Builder;
			Builder.AddText(string(KeyName) + L"." + ValName + L" (" + Value->typeToString() + L"):");
			switch(ValueType)
			{
			case Option::TYPE_BOOLEAN:
			case Option::TYPE_BOOLEAN3:
				// only to suppress C4062, TYPE_BOOLEAN is handled above
				break;
			case Option::TYPE_INTEGER:
				if (Hex)
					Builder.AddHexEditField(*static_cast<IntOption*>(Value), 40);
				else
					Builder.AddIntEditField(*static_cast<IntOption*>(Value), 40);
				break;
			case Option::TYPE_STRING:
				Builder.AddEditField(*static_cast<StringOption*>(Value), 40);
				break;
			}
			static_cast<DialogBuilderBase<DialogItemEx>*>(&Builder)->AddOKCancel(MOk, MConfigResetValue, MCancel);
			int Result = Builder.ShowDialogEx();
			if(Result == 0 || Result == 1)
			{
				if(Result == 1)
				{
					// reset to default
					switch(ValueType)
					{
					case Option::TYPE_BOOLEAN:
					case Option::TYPE_BOOLEAN3:
						// only to suppress C4062, TYPE_BOOLEAN is handled above
						break;
					case Option::TYPE_INTEGER:
						*static_cast<IntOption*>(Value) = iDefault;
						break;
					case Option::TYPE_STRING:
						*static_cast<StringOption*>(Value) = sDefault;
						break;
					}
				}
				return true;
			}
		}
		return false;
	}

	bool Changed() const
	{
		switch(ValueType)
		{
		case Option::TYPE_BOOLEAN:
			return static_cast<BoolOption*>(Value)->Get() != bDefault;
		case Option::TYPE_BOOLEAN3:
			return static_cast<Bool3Option*>(Value)->Get() != iDefault;
		case Option::TYPE_INTEGER:
			return static_cast<IntOption*>(Value)->Get() != iDefault;
		case Option::TYPE_STRING:
			return static_cast<StringOption*>(Value)->Get() != sDefault;
		}
		return false;
	}
};

struct farconfig
{
	size_t Size;
	FARConfigItem *Items;
}
FARConfig = {}, FARLocalConfig = {};

#define AddressAndType(x) &x, x.getType()
#define Default(x) reinterpret_cast<const void*>(x)

void InitCFG()
{
	static FARConfigItem _CFG[] =
	{
		{FSSF_PRIVATE,       NKeyCmdline, L"AutoComplete", AddressAndType(Global->Opt->CmdLine.AutoComplete), Default(1)},
		{FSSF_PRIVATE,       NKeyCmdline, L"EditBlock", AddressAndType(Global->Opt->CmdLine.EditBlock), Default(0)},
		{FSSF_PRIVATE,       NKeyCmdline, L"DelRemovesBlocks", AddressAndType(Global->Opt->CmdLine.DelRemovesBlocks), Default(1)},
		{FSSF_PRIVATE,       NKeyCmdline, L"PromptFormat", AddressAndType(Global->Opt->CmdLine.strPromptFormat), Default(L"$p$g")},
		{FSSF_PRIVATE,       NKeyCmdline, L"UsePromptFormat", AddressAndType(Global->Opt->CmdLine.UsePromptFormat), Default(0)},

		{FSSF_PRIVATE,       NKeyCodePages,L"CPMenuMode", AddressAndType(Global->Opt->CPMenuMode), Default(0)},
		{FSSF_PRIVATE,       NKeyCodePages,L"NoAutoDetectCP", AddressAndType(Global->Opt->strNoAutoDetectCP), Default(L"")},

		{FSSF_PRIVATE,       NKeyConfirmations,L"AllowReedit", AddressAndType(Global->Opt->Confirm.AllowReedit), Default(1)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Copy", AddressAndType(Global->Opt->Confirm.Copy), Default(1)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Delete", AddressAndType(Global->Opt->Confirm.Delete), Default(1)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"DeleteFolder", AddressAndType(Global->Opt->Confirm.DeleteFolder), Default(1)},
		{FSSF_PRIVATE,       NKeyConfirmations,L"DetachVHD", AddressAndType(Global->Opt->Confirm.DetachVHD), Default(1)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Drag", AddressAndType(Global->Opt->Confirm.Drag), Default(1)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Esc", AddressAndType(Global->Opt->Confirm.Esc), Default(1)},
		{FSSF_PRIVATE,       NKeyConfirmations,L"EscTwiceToInterrupt", AddressAndType(Global->Opt->Confirm.EscTwiceToInterrupt), Default(0)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Exit", AddressAndType(Global->Opt->Confirm.Exit), Default(1)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"HistoryClear", AddressAndType(Global->Opt->Confirm.HistoryClear), Default(1)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Move", AddressAndType(Global->Opt->Confirm.Move), Default(1)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"RemoveConnection", AddressAndType(Global->Opt->Confirm.RemoveConnection), Default(1)},
		{FSSF_PRIVATE,       NKeyConfirmations,L"RemoveHotPlug", AddressAndType(Global->Opt->Confirm.RemoveHotPlug), Default(1)},
		{FSSF_PRIVATE,       NKeyConfirmations,L"RemoveSUBST", AddressAndType(Global->Opt->Confirm.RemoveSUBST), Default(1)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"RO", AddressAndType(Global->Opt->Confirm.RO), Default(1)},

		{FSSF_PRIVATE,       NKeyDescriptions,L"AnsiByDefault", AddressAndType(Global->Opt->Diz.AnsiByDefault), Default(0)},
		{FSSF_PRIVATE,       NKeyDescriptions,L"ListNames", AddressAndType(Global->Opt->Diz.strListNames), Default(L"Descript.ion,Files.bbs")},
		{FSSF_PRIVATE,       NKeyDescriptions,L"ROUpdate", AddressAndType(Global->Opt->Diz.ROUpdate), Default(0)},
		{FSSF_PRIVATE,       NKeyDescriptions,L"SaveInUTF", AddressAndType(Global->Opt->Diz.SaveInUTF), Default(0)},
		{FSSF_PRIVATE,       NKeyDescriptions,L"SetHidden", AddressAndType(Global->Opt->Diz.SetHidden), Default(1)},
		{FSSF_PRIVATE,       NKeyDescriptions,L"StartPos", AddressAndType(Global->Opt->Diz.StartPos), Default(0)},
		{FSSF_PRIVATE,       NKeyDescriptions,L"UpdateMode", AddressAndType(Global->Opt->Diz.UpdateMode), Default(DIZ_UPDATE_IF_DISPLAYED)},

		{FSSF_PRIVATE,       NKeyDialog,L"AutoComplete", AddressAndType(Global->Opt->Dialogs.AutoComplete), Default(1)},
		{FSSF_PRIVATE,       NKeyDialog,L"CBoxMaxHeight", AddressAndType(Global->Opt->Dialogs.CBoxMaxHeight), Default(8)},
		{FSSF_DIALOG,        NKeyDialog,L"EditBlock", AddressAndType(Global->Opt->Dialogs.EditBlock), Default(0)},
		{FSSF_PRIVATE,       NKeyDialog,L"EditHistory", AddressAndType(Global->Opt->Dialogs.EditHistory), Default(1)},
		{FSSF_PRIVATE,       NKeyDialog,L"EditLine", AddressAndType(Global->Opt->Dialogs.EditLine), Default(0)},
		{FSSF_DIALOG,        NKeyDialog,L"DelRemovesBlocks", AddressAndType(Global->Opt->Dialogs.DelRemovesBlocks), Default(1)},
		{FSSF_DIALOG,        NKeyDialog,L"EULBsClear", AddressAndType(Global->Opt->Dialogs.EULBsClear), Default(0)},
		{FSSF_PRIVATE,       NKeyDialog,L"MouseButton", AddressAndType(Global->Opt->Dialogs.MouseButton), Default(0xFFFF)},

		{FSSF_PRIVATE,       NKeyEditor,L"AllowEmptySpaceAfterEof", AddressAndType(Global->Opt->EdOpt.AllowEmptySpaceAfterEof),Default(0)},
		{FSSF_PRIVATE,       NKeyEditor,L"AnsiCodePageAsDefault", AddressAndType(Global->Opt->EdOpt.AnsiCodePageAsDefault), Default(1)},
		{FSSF_PRIVATE,       NKeyEditor,L"AnsiCodePageForNewFile", AddressAndType(Global->Opt->EdOpt.AnsiCodePageForNewFile), Default(1)},
		{FSSF_PRIVATE,       NKeyEditor,L"AutoDetectCodePage", AddressAndType(Global->Opt->EdOpt.AutoDetectCodePage), Default(1)},
		{FSSF_PRIVATE,       NKeyEditor,L"AutoIndent", AddressAndType(Global->Opt->EdOpt.AutoIndent), Default(0)},
		{FSSF_PRIVATE,       NKeyEditor,L"BSLikeDel", AddressAndType(Global->Opt->EdOpt.BSLikeDel), Default(1)},
		{FSSF_PRIVATE,       NKeyEditor,L"CharCodeBase", AddressAndType(Global->Opt->EdOpt.CharCodeBase), Default(1)},
		{FSSF_PRIVATE,       NKeyEditor,L"DelRemovesBlocks", AddressAndType(Global->Opt->EdOpt.DelRemovesBlocks), Default(1)},
		{FSSF_PRIVATE,       NKeyEditor,L"EditOpenedForWrite", AddressAndType(Global->Opt->EdOpt.EditOpenedForWrite), Default(1)},
		{FSSF_PRIVATE,       NKeyEditor,L"EditorCursorBeyondEOL", AddressAndType(Global->Opt->EdOpt.CursorBeyondEOL), Default(1)},
		{FSSF_PRIVATE,       NKeyEditor,L"EditorF7Rules", AddressAndType(Global->Opt->EdOpt.F7Rules), Default(0)},
		{FSSF_PRIVATE,       NKeyEditor,L"ExpandTabs", AddressAndType(Global->Opt->EdOpt.ExpandTabs), Default(0)},
		{FSSF_PRIVATE,       NKeyEditor,L"ExternalEditorName", AddressAndType(Global->Opt->strExternalEditor), Default(L"")},
		{FSSF_PRIVATE,       NKeyEditor,L"FileSizeLimit", AddressAndType(Global->Opt->EdOpt.FileSizeLimitLo), Default(0)},
		{FSSF_PRIVATE,       NKeyEditor,L"FileSizeLimitHi", AddressAndType(Global->Opt->EdOpt.FileSizeLimitHi), Default(0)},
		{FSSF_PRIVATE,       NKeyEditor,L"KeepEditorEOL", AddressAndType(Global->Opt->EdOpt.KeepEOL), Default(1)},
		{FSSF_PRIVATE,       NKeyEditor,L"PersistentBlocks", AddressAndType(Global->Opt->EdOpt.PersistentBlocks), Default(0)},
		{FSSF_PRIVATE,       NKeyEditor,L"ReadOnlyLock", AddressAndType(Global->Opt->EdOpt.ReadOnlyLock), Default(0)},
		{FSSF_PRIVATE,       NKeyEditor,L"SaveEditorPos", AddressAndType(Global->Opt->EdOpt.SavePos), Default(1)},
		{FSSF_PRIVATE,       NKeyEditor,L"SaveEditorShortPos", AddressAndType(Global->Opt->EdOpt.SaveShortPos), Default(1)},
		{FSSF_PRIVATE,       NKeyEditor,L"SearchPickUpWord", AddressAndType(Global->Opt->EdOpt.SearchPickUpWord), Default(0)},
		{FSSF_PRIVATE,       NKeyEditor,L"SearchRegexp", AddressAndType(Global->Opt->EdOpt.SearchRegexp), Default(0)},
		{FSSF_PRIVATE,       NKeyEditor,L"SearchSelFound", AddressAndType(Global->Opt->EdOpt.SearchSelFound), Default(0)},
		{FSSF_PRIVATE,       NKeyEditor,L"SearchCursorAtEnd", AddressAndType(Global->Opt->EdOpt.SearchCursorAtEnd), Default(0)},
		{FSSF_PRIVATE,       NKeyEditor,L"ShowKeyBar", AddressAndType(Global->Opt->EdOpt.ShowKeyBar), Default(1)},
		{FSSF_PRIVATE,       NKeyEditor,L"ShowScrollBar", AddressAndType(Global->Opt->EdOpt.ShowScrollBar), Default(0)},
		{FSSF_PRIVATE,       NKeyEditor,L"ShowTitleBar", AddressAndType(Global->Opt->EdOpt.ShowTitleBar), Default(1)},
		{FSSF_PRIVATE,       NKeyEditor,L"ShowWhiteSpace", AddressAndType(Global->Opt->EdOpt.ShowWhiteSpace), Default(0)},
		{FSSF_PRIVATE,       NKeyEditor,L"TabSize", AddressAndType(Global->Opt->EdOpt.TabSize), Default(DefaultTabSize)},
		{FSSF_PRIVATE,       NKeyEditor,L"UndoDataSize", AddressAndType(Global->Opt->EdOpt.UndoSize), Default(100*1024*1024)},
		{FSSF_PRIVATE,       NKeyEditor,L"UseExternalEditor", AddressAndType(Global->Opt->EdOpt.UseExternalEditor), Default(0)},
		{FSSF_EDITOR,        NKeyEditor,L"WordDiv", AddressAndType(Global->Opt->strWordDiv), Default(WordDiv0)},

		{FSSF_PRIVATE,       NKeyHelp,L"ActivateURL", AddressAndType(Global->Opt->HelpURLRules), Default(1)},
		{FSSF_PRIVATE,       NKeyHelp,L"HelpSearchRegexp", AddressAndType(Global->Opt->HelpSearchRegexp), Default(0)},

		{FSSF_PRIVATE,       NKeyCommandHistory, NParamHistoryCount, AddressAndType(Global->Opt->HistoryCount), Default(1000)},
		{FSSF_PRIVATE,       NKeyCommandHistory, NParamHistoryLifetime, AddressAndType(Global->Opt->HistoryLifetime), Default(90)},
		{FSSF_PRIVATE,       NKeyDialogHistory, NParamHistoryCount, AddressAndType(Global->Opt->DialogsHistoryCount), Default(1000)},
		{FSSF_PRIVATE,       NKeyDialogHistory, NParamHistoryLifetime, AddressAndType(Global->Opt->DialogsHistoryLifetime), Default(90)},
		{FSSF_PRIVATE,       NKeyFolderHistory, NParamHistoryCount, AddressAndType(Global->Opt->FoldersHistoryCount), Default(1000)},
		{FSSF_PRIVATE,       NKeyFolderHistory, NParamHistoryLifetime, AddressAndType(Global->Opt->FoldersHistoryLifetime), Default(90)},
		{FSSF_PRIVATE,       NKeyViewEditHistory, NParamHistoryCount, AddressAndType(Global->Opt->ViewHistoryCount), Default(1000)},
		{FSSF_PRIVATE,       NKeyViewEditHistory, NParamHistoryLifetime, AddressAndType(Global->Opt->ViewHistoryLifetime), Default(90)},

		{FSSF_PRIVATE,       NKeyInterface,L"DelShowTotal", AddressAndType(Global->Opt->DelOpt.DelShowTotal), Default(0)},

		{FSSF_PRIVATE,       NKeyInterface, L"AltF9", AddressAndType(Global->Opt->AltF9), Default(1)},
		{FSSF_PRIVATE,       NKeyInterface, L"ClearType", AddressAndType(Global->Opt->ClearType), Default(1)},
		{FSSF_PRIVATE,       NKeyInterface, L"CopyShowTotal", AddressAndType(Global->Opt->CMOpt.CopyShowTotal), Default(1)},
		{FSSF_PRIVATE,       NKeyInterface, L"CtrlPgUp", AddressAndType(Global->Opt->PgUpChangeDisk), Default(1)},
		{FSSF_PRIVATE,       NKeyInterface, L"CursorSize1", AddressAndType(Global->Opt->CursorSize[0]), Default(15)},
		{FSSF_PRIVATE,       NKeyInterface, L"CursorSize2", AddressAndType(Global->Opt->CursorSize[1]), Default(10)},
		{FSSF_PRIVATE,       NKeyInterface, L"CursorSize3", AddressAndType(Global->Opt->CursorSize[2]), Default(99)},
		{FSSF_PRIVATE,       NKeyInterface, L"CursorSize4", AddressAndType(Global->Opt->CursorSize[3]), Default(99)},
		{FSSF_PRIVATE,       NKeyInterface, L"EditorTitleFormat", AddressAndType(Global->Opt->strEditorTitleFormat), Default(L"%Lng %File")},
		{FSSF_PRIVATE,       NKeyInterface, L"FormatNumberSeparators", AddressAndType(Global->Opt->FormatNumberSeparators), Default(0)},
		{FSSF_PRIVATE,       NKeyInterface, L"Mouse", AddressAndType(Global->Opt->Mouse), Default(1)},
		{FSSF_PRIVATE,       NKeyInterface, L"SetIcon", AddressAndType(Global->Opt->SetIcon), Default(0)},
		{FSSF_PRIVATE,       NKeyInterface, L"SetAdminIcon", AddressAndType(Global->Opt->SetAdminIcon), Default(1)},
		{FSSF_PRIVATE,       NKeyInterface, L"ShiftsKeyRules", AddressAndType(Global->Opt->ShiftsKeyRules), Default(1)},
		{FSSF_PRIVATE,       NKeyInterface, L"ShowDotsInRoot", AddressAndType(Global->Opt->ShowDotsInRoot), Default(0)},
		{FSSF_INTERFACE,     NKeyInterface, L"ShowMenuBar", AddressAndType(Global->Opt->ShowMenuBar), Default(0)},
		{FSSF_PRIVATE,       NKeyInterface, L"RedrawTimeout", AddressAndType(Global->Opt->RedrawTimeout), Default(200)},
		{FSSF_PRIVATE,       NKeyInterface, L"TitleAddons", AddressAndType(Global->Opt->strTitleAddons), Default(L"%Ver.%Build %Platform %Admin")},
		{FSSF_PRIVATE,       NKeyInterface, L"UseVk_oem_x", AddressAndType(Global->Opt->UseVk_oem_x), Default(1)},
		{FSSF_PRIVATE,       NKeyInterface, L"ViewerTitleFormat", AddressAndType(Global->Opt->strViewerTitleFormat), Default(L"%Lng %File")},

		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"Append", AddressAndType(Global->Opt->AutoComplete.AppendCompletion), Default(0)},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"ModalList", AddressAndType(Global->Opt->AutoComplete.ModalList), Default(0)},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"ShowList", AddressAndType(Global->Opt->AutoComplete.ShowList), Default(1)},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"UseFilesystem", AddressAndType(Global->Opt->AutoComplete.UseFilesystem), Default(1)},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"UseHistory", AddressAndType(Global->Opt->AutoComplete.UseHistory), Default(1)},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"UsePath", AddressAndType(Global->Opt->AutoComplete.UsePath), Default(1)},

		{FSSF_PRIVATE,       NKeyLanguage, L"Main", AddressAndType(Global->Opt->strLanguage), Default(DefaultLanguage)},
		{FSSF_PRIVATE,       NKeyLanguage, L"Help", AddressAndType(Global->Opt->strHelpLanguage), Default(DefaultLanguage)},

		{FSSF_PRIVATE,       NKeyLayout,L"FullscreenHelp", AddressAndType(Global->Opt->FullScreenHelp), Default(0)},
		{FSSF_PRIVATE,       NKeyLayout,L"LeftHeightDecrement", AddressAndType(Global->Opt->LeftHeightDecrement), Default(0)},
		{FSSF_PRIVATE,       NKeyLayout,L"RightHeightDecrement", AddressAndType(Global->Opt->RightHeightDecrement), Default(0)},
		{FSSF_PRIVATE,       NKeyLayout,L"WidthDecrement", AddressAndType(Global->Opt->WidthDecrement), Default(0)},

		{FSSF_PRIVATE,       NKeyKeyMacros,L"CONVFMT", AddressAndType(Global->Opt->Macro.strMacroCONVFMT), Default(L"%.6g")},
		{FSSF_PRIVATE,       NKeyKeyMacros,L"DateFormat", AddressAndType(Global->Opt->Macro.strDateFormat), Default(L"%a %b %d %H:%M:%S %Z %Y")},
		{FSSF_PRIVATE,       NKeyKeyMacros,L"MacroReuseRules", AddressAndType(Global->Opt->Macro.MacroReuseRules), Default(0)},

		{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordCtrlDot", AddressAndType(Global->Opt->Macro.strKeyMacroCtrlDot), Default(L"Ctrl.")},
		{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordRCtrlDot", AddressAndType(Global->Opt->Macro.strKeyMacroRCtrlDot), Default(L"RCtrl.")},
		{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordCtrlShiftDot", AddressAndType(Global->Opt->Macro.strKeyMacroCtrlShiftDot), Default(L"CtrlShift.")},
		{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordRCtrlShiftDot", AddressAndType(Global->Opt->Macro.strKeyMacroRCtrlShiftDot), Default(L"RCtrlShift.")},

		{FSSF_PRIVATE,       NKeyPanel,L"AutoUpdateLimit", AddressAndType(Global->Opt->AutoUpdateLimit), Default(0)},
		{FSSF_PRIVATE,       NKeyPanel,L"CtrlAltShiftRule", AddressAndType(Global->Opt->PanelCtrlAltShiftRule), Default(0)},
		{FSSF_PRIVATE,       NKeyPanel,L"CtrlFRule", AddressAndType(Global->Opt->PanelCtrlFRule), Default(0)},
		{FSSF_PRIVATE,       NKeyPanel,L"Highlight", AddressAndType(Global->Opt->Highlight), Default(1)},
		{FSSF_PRIVATE,       NKeyPanel,L"ReverseSort", AddressAndType(Global->Opt->ReverseSort), Default(1)},
		{FSSF_PRIVATE,       NKeyPanel,L"RememberLogicalDrives", AddressAndType(Global->Opt->RememberLogicalDrives), Default(0)},
		{FSSF_PRIVATE,       NKeyPanel,L"RightClickRule", AddressAndType(Global->Opt->PanelRightClickRule), Default(2)},
		{FSSF_PRIVATE,       NKeyPanel,L"SelectFolders", AddressAndType(Global->Opt->SelectFolders), Default(0)},
		{FSSF_PRIVATE,       NKeyPanel,L"ShellRightLeftArrowsRule", AddressAndType(Global->Opt->ShellRightLeftArrowsRule), Default(0)},
		{FSSF_PANEL,         NKeyPanel,L"ShowHidden", AddressAndType(Global->Opt->ShowHidden), Default(1)},
		{FSSF_PRIVATE,       NKeyPanel,L"SortFolderExt", AddressAndType(Global->Opt->SortFolderExt), Default(0)},
		{FSSF_PRIVATE,       NKeyPanel,L"RightClickSelect", AddressAndType(Global->Opt->RightClickSelect), Default(0)},

		{FSSF_PRIVATE,       NKeyPanelInfo,L"InfoComputerNameFormat", AddressAndType(Global->Opt->InfoPanel.ComputerNameFormat), Default(ComputerNamePhysicalNetBIOS)},
		{FSSF_PRIVATE,       NKeyPanelInfo,L"InfoUserNameFormat", AddressAndType(Global->Opt->InfoPanel.UserNameFormat), Default(NameUserPrincipal)},
		{FSSF_PRIVATE,       NKeyPanelInfo,L"ShowCDInfo", AddressAndType(Global->Opt->InfoPanel.ShowCDInfo), Default(1)},
		{FSSF_PRIVATE,       NKeyPanelInfo,L"ShowPowerStatus", AddressAndType(Global->Opt->InfoPanel.ShowPowerStatus), Default(0)},

		{FSSF_PRIVATE,       NKeyPanelLayout,L"ColoredGlobalColumnSeparator", AddressAndType(Global->Opt->HighlightColumnSeparator), Default(1)},
		{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"ColumnTitles", AddressAndType(Global->Opt->ShowColumnTitles), Default(1)},
		{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"DetailedJunction", AddressAndType(Global->Opt->PanelDetailedJunction), Default(0)},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"DoubleGlobalColumnSeparator", AddressAndType(Global->Opt->DoubleGlobalColumnSeparator), Default(0)},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"FreeInfo", AddressAndType(Global->Opt->ShowPanelFree), Default(0)},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"ScreensNumber", AddressAndType(Global->Opt->ShowScreensNumber), Default(1)},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"Scrollbar", AddressAndType(Global->Opt->ShowPanelScrollbar), Default(0)},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"ScrollbarMenu", AddressAndType(Global->Opt->ShowMenuScrollbar), Default(1)},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"ShowUnknownReparsePoint", AddressAndType(Global->Opt->ShowUnknownReparsePoint), Default(0)},
		{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"SortMode", AddressAndType(Global->Opt->ShowSortMode), Default(1)},
		{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"StatusLine", AddressAndType(Global->Opt->ShowPanelStatus), Default(1)},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"TotalInfo", AddressAndType(Global->Opt->ShowPanelTotals), Default(1)},

		{FSSF_PRIVATE,       NKeyPanelLeft,L"CaseSensitiveSort", AddressAndType(Global->Opt->LeftPanel.CaseSensitiveSort), Default(0)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"DirectoriesFirst", AddressAndType(Global->Opt->LeftPanel.DirectoriesFirst), Default(1)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"NumericSort", AddressAndType(Global->Opt->LeftPanel.NumericSort), Default(0)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"SelectedFirst", AddressAndType(Global->Opt->LeftSelectedFirst), Default(0)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"ShortNames", AddressAndType(Global->Opt->LeftPanel.ShowShortNames), Default(0)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"SortGroups", AddressAndType(Global->Opt->LeftPanel.SortGroups), Default(0)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"SortMode", AddressAndType(Global->Opt->LeftPanel.SortMode), Default(1)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"SortOrder", AddressAndType(Global->Opt->LeftPanel.SortOrder), Default(1)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"Type", AddressAndType(Global->Opt->LeftPanel.Type), Default(0)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"ViewMode", AddressAndType(Global->Opt->LeftPanel.ViewMode), Default(2)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"Visible", AddressAndType(Global->Opt->LeftPanel.Visible), Default(1)},

		{FSSF_PRIVATE,       NKeyPanelRight,L"CaseSensitiveSort", AddressAndType(Global->Opt->RightPanel.CaseSensitiveSort), Default(0)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"DirectoriesFirst", AddressAndType(Global->Opt->RightPanel.DirectoriesFirst), Default(1)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"NumericSort", AddressAndType(Global->Opt->RightPanel.NumericSort), Default(0)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"SelectedFirst", AddressAndType(Global->Opt->RightSelectedFirst), Default(0)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"ShortNames", AddressAndType(Global->Opt->RightPanel.ShowShortNames), Default(0)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"SortGroups", AddressAndType(Global->Opt->RightPanel.SortGroups), Default(0)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"SortMode", AddressAndType(Global->Opt->RightPanel.SortMode), Default(1)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"SortOrder", AddressAndType(Global->Opt->RightPanel.SortOrder), Default(1)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"Type", AddressAndType(Global->Opt->RightPanel.Type), Default(0)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"ViewMode", AddressAndType(Global->Opt->RightPanel.ViewMode), Default(2)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"Visible", AddressAndType(Global->Opt->RightPanel.Visible), Default(1)},

		{FSSF_PRIVATE,       NKeyPanelTree,L"AutoChangeFolder", AddressAndType(Global->Opt->Tree.AutoChangeFolder), Default(0)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"MinTreeCount", AddressAndType(Global->Opt->Tree.MinTreeCount), Default(4)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"TreeFileAttr", AddressAndType(Global->Opt->Tree.TreeFileAttr), Default(FILE_ATTRIBUTE_HIDDEN)},
	#if defined(TREEFILE_PROJECT)
		{FSSF_PRIVATE,       NKeyPanelTree,L"CDDisk", AddressAndType(Global->Opt->Tree.CDDisk), Default(2)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"CDDiskTemplate,0", AddressAndType(Global->Opt->Tree.strCDDisk), Default(constCDDiskTemplate)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"ExceptPath", AddressAndType(Global->Opt->Tree.strExceptPath), Default(L"")},
		{FSSF_PRIVATE,       NKeyPanelTree,L"LocalDisk", AddressAndType(Global->Opt->Tree.LocalDisk), Default(2)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"LocalDiskTemplate", AddressAndType(Global->Opt->Tree.strLocalDisk), Default(constLocalDiskTemplate)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"NetDisk", AddressAndType(Global->Opt->Tree.NetDisk), Default(2)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"NetPath", AddressAndType(Global->Opt->Tree.NetPath), Default(2)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"NetDiskTemplate", AddressAndType(Global->Opt->Tree.strNetDisk), Default(constNetDiskTemplate)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"NetPathTemplate", AddressAndType(Global->Opt->Tree.strNetPath), Default(constNetPathTemplate)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"RemovableDisk", AddressAndType(Global->Opt->Tree.RemovableDisk), Default(2)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"RemovableDiskTemplate,", AddressAndType(Global->Opt->Tree.strRemovableDisk), Default(constRemovableDiskTemplate)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"SaveLocalPath", AddressAndType(Global->Opt->Tree.strSaveLocalPath), Default(L"")},
		{FSSF_PRIVATE,       NKeyPanelTree,L"SaveNetPath", AddressAndType(Global->Opt->Tree.strSaveNetPath), Default(L"")},
	#endif
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"EvenIfOnlyOnePlugin", AddressAndType(Global->Opt->PluginConfirm.EvenIfOnlyOnePlugin), Default(0)},
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"OpenFilePlugin", AddressAndType(Global->Opt->PluginConfirm.OpenFilePlugin), Default(0)},
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"Prefix", AddressAndType(Global->Opt->PluginConfirm.Prefix), Default(0)},
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"SetFindList", AddressAndType(Global->Opt->PluginConfirm.SetFindList), Default(0)},
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"StandardAssociation", AddressAndType(Global->Opt->PluginConfirm.StandardAssociation), Default(0)},

		{FSSF_PRIVATE,       NKeyPolicies,L"DisabledOptions", AddressAndType(Global->Opt->Policies.DisabledOptions), Default(0)},
		{FSSF_PRIVATE,       NKeyPolicies,L"ShowHiddenDrives", AddressAndType(Global->Opt->Policies.ShowHiddenDrives), Default(1)},

		{FSSF_PRIVATE,       NKeyScreen, L"Clock", AddressAndType(Global->Opt->Clock), Default(1)},
		{FSSF_PRIVATE,       NKeyScreen, L"DeltaX", AddressAndType(Global->Opt->ScrSize.DeltaX), Default(0)},
		{FSSF_PRIVATE,       NKeyScreen, L"DeltaY", AddressAndType(Global->Opt->ScrSize.DeltaY), Default(0)},
		{FSSF_SCREEN,        NKeyScreen, L"KeyBar", AddressAndType(Global->Opt->ShowKeyBar), Default(1)},
		{FSSF_PRIVATE,       NKeyScreen, L"ScreenSaver", AddressAndType(Global->Opt->ScreenSaver), Default(0)},
		{FSSF_PRIVATE,       NKeyScreen, L"ScreenSaverTime", AddressAndType(Global->Opt->ScreenSaverTime), Default(5)},
		{FSSF_PRIVATE,       NKeyScreen, L"ViewerEditorClock", AddressAndType(Global->Opt->ViewerEditorClock), Default(1)},

		{FSSF_PRIVATE,       NKeySystem,L"AllCtrlAltShiftRule", AddressAndType(Global->Opt->AllCtrlAltShiftRule), Default(0x0000FFFF)},
		{FSSF_PRIVATE,       NKeySystem,L"AutoSaveSetup", AddressAndType(Global->Opt->AutoSaveSetup), Default(0)},
		{FSSF_PRIVATE,       NKeySystem,L"AutoUpdateRemoteDrive", AddressAndType(Global->Opt->AutoUpdateRemoteDrive), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"BoxSymbols", AddressAndType(Global->Opt->strBoxSymbols), Default(_BoxSymbols)},
		{FSSF_PRIVATE,       NKeySystem,L"CASRule", AddressAndType(Global->Opt->CASRule), Default(0xFFFFFFFFU)},
		{FSSF_PRIVATE,       NKeySystem,L"CloseCDGate", AddressAndType(Global->Opt->CloseCDGate), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"CmdHistoryRule", AddressAndType(Global->Opt->CmdHistoryRule), Default(0)},
		{FSSF_PRIVATE,       NKeySystem,L"CollectFiles", AddressAndType(Global->Opt->FindOpt.CollectFiles), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"ConsoleDetachKey", AddressAndType(Global->Opt->ConsoleDetachKey), Default(L"CtrlShiftTab")},
		{FSSF_PRIVATE,       NKeySystem,L"CopyBufferSize", AddressAndType(Global->Opt->CMOpt.BufferSize), Default(0)},
		{FSSF_SYSTEM,        NKeySystem,L"CopyOpened", AddressAndType(Global->Opt->CMOpt.CopyOpened), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"CopyTimeRule",  AddressAndType(Global->Opt->CMOpt.CopyTimeRule), Default(3)},
		{FSSF_PRIVATE,       NKeySystem,L"CopySecurityOptions", AddressAndType(Global->Opt->CMOpt.CopySecurityOptions), Default(0)},
		{FSSF_PRIVATE,       NKeySystem,L"CreateUppercaseFolders", AddressAndType(Global->Opt->CreateUppercaseFolders), Default(0)},
		{FSSF_SYSTEM,        NKeySystem,L"DeleteToRecycleBin", AddressAndType(Global->Opt->DeleteToRecycleBin), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"DeleteToRecycleBinKillLink", AddressAndType(Global->Opt->DeleteToRecycleBinKillLink), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"DelThreadPriority", AddressAndType(Global->Opt->DelThreadPriority), THREAD_PRIORITY_NORMAL},
		{FSSF_PRIVATE,       NKeySystem,L"DriveDisconnectMode", AddressAndType(Global->Opt->ChangeDriveDisconnectMode), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"DriveMenuMode", AddressAndType(Global->Opt->ChangeDriveMode), Default(DRIVE_SHOW_TYPE|DRIVE_SHOW_PLUGINS|DRIVE_SHOW_SIZE_FLOAT|DRIVE_SHOW_CDROM)},
		{FSSF_PRIVATE,       NKeySystem,L"ElevationMode", AddressAndType(Global->Opt->StoredElevationMode), Default(0x0FFFFFFFU)},
		{FSSF_PRIVATE,       NKeySystem,L"ExceptRules", AddressAndType(Global->Opt->StoredExceptRules), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"ExcludeCmdHistory", AddressAndType(Global->Opt->ExcludeCmdHistory), Default(0)},
		{FSSF_PRIVATE,       NKeySystem,L"FileSearchMode", AddressAndType(Global->Opt->FindOpt.FileSearchMode), Default(FINDAREA_FROM_CURRENT)},
		{FSSF_PRIVATE,       NKeySystem,L"FindAlternateStreams", AddressAndType(Global->Opt->FindOpt.FindAlternateStreams),0,},
		{FSSF_PRIVATE,       NKeySystem,L"FindCodePage", AddressAndType(Global->Opt->FindCodePage), Default(CP_DEFAULT)},
		{FSSF_PRIVATE,       NKeySystem,L"FindFolders", AddressAndType(Global->Opt->FindOpt.FindFolders), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"FindSymLinks", AddressAndType(Global->Opt->FindOpt.FindSymLinks), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"FlagPosixSemantics", AddressAndType(Global->Opt->FlagPosixSemantics), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"FolderInfo", AddressAndType(Global->Opt->InfoPanel.strFolderInfoFiles), Default(L"DirInfo,File_Id.diz,Descript.ion,ReadMe.*,Read.Me")},
		{FSSF_PRIVATE,       NKeySystem,L"MsWheelDelta", AddressAndType(Global->Opt->MsWheelDelta), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"MsWheelDeltaEdit", AddressAndType(Global->Opt->MsWheelDeltaEdit), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"MsWheelDeltaHelp", AddressAndType(Global->Opt->MsWheelDeltaHelp), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"MsWheelDeltaView", AddressAndType(Global->Opt->MsWheelDeltaView), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"MsHWheelDelta", AddressAndType(Global->Opt->MsHWheelDelta), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"MsHWheelDeltaEdit", AddressAndType(Global->Opt->MsHWheelDeltaEdit), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"MsHWheelDeltaView", AddressAndType(Global->Opt->MsHWheelDeltaView), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"MultiCopy", AddressAndType(Global->Opt->CMOpt.MultiCopy), Default(0)},
		{FSSF_PRIVATE,       NKeySystem,L"MultiMakeDir", AddressAndType(Global->Opt->MultiMakeDir), Default(0)},
	#ifndef NO_WRAPPER
		{FSSF_PRIVATE,       NKeySystem,L"OEMPluginsSupport",  AddressAndType(Global->Opt->LoadPlug.OEMPluginsSupport), Default(1)},
	#endif // NO_WRAPPER
		{FSSF_SYSTEM,        NKeySystem,L"PluginMaxReadData", AddressAndType(Global->Opt->PluginMaxReadData), Default(0x20000)},
		{FSSF_PRIVATE,       NKeySystem,L"QuotedName", AddressAndType(Global->Opt->QuotedName), Default(QUOTEDNAME_INSERT)},
		{FSSF_PRIVATE,       NKeySystem,L"QuotedSymbols", AddressAndType(Global->Opt->strQuotedSymbols), Default(L" &()[]{}^=;!'+,`\xA0")},
		{FSSF_PRIVATE,       NKeySystem,L"SaveHistory", AddressAndType(Global->Opt->SaveHistory), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"SaveFoldersHistory", AddressAndType(Global->Opt->SaveFoldersHistory), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"SaveViewHistory", AddressAndType(Global->Opt->SaveViewHistory), Default(1)},
		{FSSF_SYSTEM,        NKeySystem,L"ScanJunction", AddressAndType(Global->Opt->ScanJunction), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"ScanSymlinks",  AddressAndType(Global->Opt->LoadPlug.ScanSymlinks), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"SearchInFirstSize", AddressAndType(Global->Opt->FindOpt.strSearchInFirstSize), Default(L"")},
		{FSSF_PRIVATE,       NKeySystem,L"SearchOutFormat", AddressAndType(Global->Opt->FindOpt.strSearchOutFormat), Default(L"D,S,A")},
		{FSSF_PRIVATE,       NKeySystem,L"SearchOutFormatWidth", AddressAndType(Global->Opt->FindOpt.strSearchOutFormatWidth), Default(L"14,13,0")},
		{FSSF_PRIVATE,       NKeySystem,L"SetAttrFolderRules", AddressAndType(Global->Opt->SetAttrFolderRules), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"ShowCheckingFile", AddressAndType(Global->Opt->ShowCheckingFile), Default(0)},
		{FSSF_PRIVATE,       NKeySystem,L"ShowStatusInfo", AddressAndType(Global->Opt->InfoPanel.strShowStatusInfo), Default(L"")},
		{FSSF_PRIVATE,       NKeySystem,L"SilentLoadPlugin",  AddressAndType(Global->Opt->LoadPlug.SilentLoadPlugin), Default(0)},
		{FSSF_PRIVATE,       NKeySystem,L"SmartFolderMonitor",  AddressAndType(Global->Opt->SmartFolderMonitor), Default(0)},
		{FSSF_PRIVATE,       NKeySystem,L"SubstNameRule", AddressAndType(Global->Opt->SubstNameRule), Default(2)},
		{FSSF_PRIVATE,       NKeySystem,L"SubstPluginPrefix", AddressAndType(Global->Opt->SubstPluginPrefix), Default(0)},
		{FSSF_PRIVATE,       NKeySystem,L"UpdateEnvironment", AddressAndType(Global->Opt->UpdateEnvironment),0,},
		{FSSF_PRIVATE,       NKeySystem,L"UseFilterInSearch", AddressAndType(Global->Opt->FindOpt.UseFilter),0,},
		{FSSF_PRIVATE,       NKeySystem,L"UseRegisteredTypes", AddressAndType(Global->Opt->UseRegisteredTypes), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"UseSystemCopy", AddressAndType(Global->Opt->CMOpt.UseSystemCopy), Default(1)},
		{FSSF_PRIVATE,       NKeySystem,L"WindowMode", AddressAndType(Global->Opt->StoredWindowMode), Default(0)},
		{FSSF_PRIVATE,       NKeySystem,L"WipeSymbol", AddressAndType(Global->Opt->WipeSymbol), Default(0)},

		{FSSF_PRIVATE,       NKeySystemKnownIDs,L"EMenu", AddressAndType(Global->Opt->KnownIDs.EmenuGuidStr), Default(L"742910F1-02ED-4542-851F-DEE37C2E13B2")},
		{FSSF_PRIVATE,       NKeySystemKnownIDs,L"Network", AddressAndType(Global->Opt->KnownIDs.NetworkGuidStr), Default(L"773B5051-7C5F-4920-A201-68051C4176A4")},

		{FSSF_PRIVATE,       NKeySystemNowell,L"MoveRO", AddressAndType(Global->Opt->Nowell.MoveRO), Default(1)},

		{FSSF_PRIVATE,       NKeySystemException,L"FarEventSvc", AddressAndType(Global->Opt->strExceptEventSvc), Default(L"")},
		{FSSF_PRIVATE,       NKeySystemException,L"Used", AddressAndType(Global->Opt->ExceptUsed), Default(0)},

		{FSSF_PRIVATE,       NKeySystemExecutor,L"~", AddressAndType(Global->Opt->Exec.strHomeDir), Default(L"%FARHOME%")},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"BatchType", AddressAndType(Global->Opt->Exec.strExecuteBatchType), Default(constBatchExt)},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"ExcludeCmds", AddressAndType(Global->Opt->Exec.strExcludeCmds), Default(L"")},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"FullTitle", AddressAndType(Global->Opt->Exec.ExecuteFullTitle), Default(0)},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"RestoreCP", AddressAndType(Global->Opt->Exec.RestoreCPAfterExecute), Default(1)},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"SilentExternal", AddressAndType(Global->Opt->Exec.ExecuteSilentExternal), Default(0)},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"UseAppPath", AddressAndType(Global->Opt->Exec.ExecuteUseAppPath), Default(1)},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"UseHomeDir", AddressAndType(Global->Opt->Exec.UseHomeDir), Default(1)},

		{FSSF_PRIVATE,       NKeyViewer,L"AnsiCodePageAsDefault", AddressAndType(Global->Opt->ViOpt.AnsiCodePageAsDefault), Default(1)},
		{FSSF_PRIVATE,       NKeyViewer,L"AutoDetectCodePage", AddressAndType(Global->Opt->ViOpt.AutoDetectCodePage), Default(1)},
		{FSSF_PRIVATE,       NKeyViewer,L"ExternalViewerName", AddressAndType(Global->Opt->strExternalViewer), Default(L"")},
		{FSSF_PRIVATE,       NKeyViewer,L"IsWrap", AddressAndType(Global->Opt->ViOpt.ViewerIsWrap), Default(1)},
		{FSSF_PRIVATE,       NKeyViewer,L"MaxLineSize", AddressAndType(Global->Opt->ViOpt.MaxLineSize), Default(ViewerOptions::eDefLineSize)},
		{FSSF_PRIVATE,       NKeyViewer,L"PersistentBlocks", AddressAndType(Global->Opt->ViOpt.PersistentBlocks), Default(0)},
		{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerPos", AddressAndType(Global->Opt->ViOpt.SavePos), Default(1)},
		{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerShortPos", AddressAndType(Global->Opt->ViOpt.SaveShortPos), Default(1)},
		{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerCodepage", AddressAndType(Global->Opt->ViOpt.SaveCodepage), Default(1)},
		{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerWrapMode", AddressAndType(Global->Opt->ViOpt.SaveWrapMode), Default(0)},
		{FSSF_PRIVATE,       NKeyViewer,L"SearchEditFocus", AddressAndType(Global->Opt->ViOpt.SearchEditFocus), Default(0)},
		{FSSF_PRIVATE,       NKeyViewer,L"SearchRegexp", AddressAndType(Global->Opt->ViOpt.SearchRegexp), Default(0)},
		{FSSF_PRIVATE,       NKeyViewer,L"ShowArrows", AddressAndType(Global->Opt->ViOpt.ShowArrows), Default(1)},
		{FSSF_PRIVATE,       NKeyViewer,L"ShowKeyBar", AddressAndType(Global->Opt->ViOpt.ShowKeyBar), Default(1)},
		{FSSF_PRIVATE,       NKeyViewer,L"ShowTitleBar", AddressAndType(Global->Opt->ViOpt.ShowTitleBar), Default(1)},
		{FSSF_PRIVATE,       NKeyViewer,L"ShowScrollbar", AddressAndType(Global->Opt->ViOpt.ShowScrollbar), Default(0)},
		{FSSF_PRIVATE,       NKeyViewer,L"TabSize", AddressAndType(Global->Opt->ViOpt.TabSize), Default(DefaultTabSize)},
		{FSSF_PRIVATE,       NKeyViewer,L"UseExternalViewer", AddressAndType(Global->Opt->ViOpt.UseExternalViewer), Default(0)},
		{FSSF_PRIVATE,       NKeyViewer,L"Visible0x00", AddressAndType(Global->Opt->ViOpt.Visible0x00), Default(0)},
		{FSSF_PRIVATE,       NKeyViewer,L"Wrap", AddressAndType(Global->Opt->ViOpt.ViewerWrap), Default(0)},
		{FSSF_PRIVATE,       NKeyViewer,L"ZeroChar", AddressAndType(Global->Opt->ViOpt.ZeroChar), Default(0x00B7)}, // middle dot

		{FSSF_PRIVATE,       NKeyVMenu,L"LBtnClick", AddressAndType(Global->Opt->VMenu.LBtnClick), Default(VMENUCLICK_CANCEL)},
		{FSSF_PRIVATE,       NKeyVMenu,L"MBtnClick", AddressAndType(Global->Opt->VMenu.MBtnClick), Default(VMENUCLICK_APPLY)},
		{FSSF_PRIVATE,       NKeyVMenu,L"RBtnClick", AddressAndType(Global->Opt->VMenu.RBtnClick), Default(VMENUCLICK_CANCEL)},

		{FSSF_PRIVATE,       NKeyXLat,L"Flags", AddressAndType(Global->Opt->XLat.Flags), Default(XLAT_SWITCHKEYBLAYOUT|XLAT_CONVERTALLCMDLINE), },
		{FSSF_PRIVATE,       NKeyXLat,L"Layouts", AddressAndType(Global->Opt->XLat.strLayouts), Default(L"")},
		{FSSF_PRIVATE,       NKeyXLat,L"Rules1", AddressAndType(Global->Opt->XLat.Rules[0]), Default(L"")},
		{FSSF_PRIVATE,       NKeyXLat,L"Rules2", AddressAndType(Global->Opt->XLat.Rules[1]), Default(L"")},
		{FSSF_PRIVATE,       NKeyXLat,L"Rules3", AddressAndType(Global->Opt->XLat.Rules[2]), Default(L"")},
		{FSSF_PRIVATE,       NKeyXLat,L"Table1", AddressAndType(Global->Opt->XLat.Table[0]), Default(L"")},
		{FSSF_PRIVATE,       NKeyXLat,L"Table2", AddressAndType(Global->Opt->XLat.Table[1]), Default(L"")},
		{FSSF_PRIVATE,       NKeyXLat,L"WordDivForXlat", AddressAndType(Global->Opt->XLat.strWordDivForXlat), Default(WordDivForXlat0)},
	};
	FARConfig.Items = _CFG;
	FARConfig.Size = ARRAYSIZE(_CFG);
}

void InitLocalCFG()
{
	static FARConfigItem _CFG[] =
	{
		{FSSF_PRIVATE,       NKeyPanelLeft,L"CurFile", AddressAndType(Global->Opt->strLeftCurFile), Default(L"")},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"Focus", AddressAndType(Global->Opt->LeftPanel.Focus), Default(true)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"Folder", AddressAndType(Global->Opt->strLeftFolder), Default(L"")},

		{FSSF_PRIVATE,       NKeyPanelRight,L"CurFile", AddressAndType(Global->Opt->strRightCurFile), Default(L"")},
		{FSSF_PRIVATE,       NKeyPanelRight,L"Focus", AddressAndType(Global->Opt->RightPanel.Focus), Default(false)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"Folder", AddressAndType(Global->Opt->strRightFolder), Default(L"")},
	};
	FARLocalConfig.Items = _CFG;
	FARLocalConfig.Size = ARRAYSIZE(_CFG);
}
#undef AddressAndType
#undef Default


bool GetConfigValue(const wchar_t *Key, const wchar_t *Name, string &strValue)
{
	for (size_t I=0; I < FARConfig.Size; ++I)
	{
		if (!StrCmpI(FARConfig.Items[I].KeyName,Key) && !StrCmpI(FARConfig.Items[I].ValName,Name))
		{
			strValue = FARConfig.Items[I].Value->toString();
			return true;
		}
	}

	return false;
}

bool GetConfigValue(size_t Root, const wchar_t* Name, Option::OptionType& Type, Option*& Data)
{
	if(FSSF_PRIVATE!=Root)
	{
		for(size_t ii=0;ii<FARConfig.Size;++ii)
		{
			if(Root==FARConfig.Items[ii].ApiRoot&&!StrCmpI(FARConfig.Items[ii].ValName,Name))
			{
				Type=FARConfig.Items[ii].ValueType;
				Data=FARConfig.Items[ii].Value;
				return true;
			}
		}
	}
	return false;
}

Options::Options():
	ReadOnlyConfig(-1),
	UseExceptionHandler(0),
	ExceptRules(-1),
	ElevationMode(0),
	WindowMode(-1)
{
#ifndef _DEBUGEXC
	if(IsDebuggerPresent())
	{
		ExceptRules = 0;
	}
#endif

	// По умолчанию - брать плагины из основного каталога
	LoadPlug.MainPluginDir = true;
	LoadPlug.PluginsPersonal = true;
	LoadPlug.PluginsCacheOnly = false;

	Macro.DisableMacro=0;
}

void Options::InitConfig()
{
	if(ConfigList.empty())
	{
		InitCFG();
		InitLocalCFG();
		ConfigList.push_back(VALUE_TYPE(ConfigList)(Global->Db->GeneralCfg(), &FARConfig));
		ConfigList.push_back(VALUE_TYPE(ConfigList)(Global->Db->LocalGeneralCfg(), &FARLocalConfig));
	}
}

void Options::Load()
{
	InitConfig();

	/* <ПРЕПРОЦЕССЫ> *************************************************** */

	/* BUGBUG??
	SetRegRootKey(HKEY_LOCAL_MACHINE);
	DWORD OptPolicies_ShowHiddenDrives=GetRegKey(NKeyPolicies,L"ShowHiddenDrives",1)&1;
	DWORD OptPolicies_DisabledOptions=GetRegKey(NKeyPolicies,L"DisabledOptions",0);
	SetRegRootKey(HKEY_CURRENT_USER);
	*/
	/* *************************************************** </ПРЕПРОЦЕССЫ> */

	GetPrivateProfileString(L"General", L"DefaultLanguage", L"English", DefaultLanguage, ARRAYSIZE(DefaultLanguage), Global->g_strFarINI);

	std::for_each(RANGE(ConfigList, i)
	{
		for (size_t j=0; j < i.second->Size; ++j)
		{
			i.second->Items[j].Value->ReceiveValue(i.first, i.second->Items[j].KeyName, i.second->Items[j].ValName, i.second->Items[j].Default);
		}
	});

	/* <ПОСТПРОЦЕССЫ> *************************************************** */

	Palette.Load();
	GlobalUserMenuDir.ReleaseBuffer(GetPrivateProfileString(L"General", L"GlobalUserMenuDir", Global->g_strFarPath, GlobalUserMenuDir.GetBuffer(NT_MAX_PATH), NT_MAX_PATH, Global->g_strFarINI));
	apiExpandEnvironmentStrings(GlobalUserMenuDir, GlobalUserMenuDir);
	ConvertNameToFull(GlobalUserMenuDir,GlobalUserMenuDir);
	AddEndSlash(GlobalUserMenuDir);

	if (ExceptRules == -1)
	{
		ExceptRules = StoredExceptRules;
	}

	if(WindowMode == -1)
	{
		WindowMode = StoredWindowMode;
	}

	ElevationMode = StoredElevationMode;

	if (PluginMaxReadData < 0x1000)
		PluginMaxReadData=0x20000;

	if (!ViOpt.MaxLineSize)
		ViOpt.MaxLineSize = ViewerOptions::eDefLineSize;
	else if (ViOpt.MaxLineSize < ViewerOptions::eMinLineSize)
		ViOpt.MaxLineSize = ViewerOptions::eMinLineSize;
	else if (ViOpt.MaxLineSize > ViewerOptions::eMaxLineSize)
		ViOpt.MaxLineSize = ViewerOptions::eMaxLineSize;

	// Исключаем случайное стирание разделителей ;-)
	if (strWordDiv.IsEmpty())
		strWordDiv = WordDiv0;

	// Исключаем случайное стирание разделителей
	if (XLat.strWordDivForXlat.IsEmpty())
		XLat.strWordDivForXlat = WordDivForXlat0;

	PanelRightClickRule%=3;
	PanelCtrlAltShiftRule%=3;

	if (EdOpt.TabSize<1 || EdOpt.TabSize>512)
		EdOpt.TabSize = DefaultTabSize;

	if (ViOpt.TabSize<1 || ViOpt.TabSize>512)
		ViOpt.TabSize = DefaultTabSize;

	HelpTabSize = DefaultTabSize; // пока жестко пропишем...


	if ((Macro.KeyMacroCtrlDot=KeyNameToKey(Macro.strKeyMacroCtrlDot)) == static_cast<DWORD>(-1))
		Macro.KeyMacroCtrlDot=KEY_CTRLDOT;

	if ((Macro.KeyMacroRCtrlDot=KeyNameToKey(Macro.strKeyMacroRCtrlDot)) == static_cast<DWORD>(-1))
		Macro.KeyMacroRCtrlDot=KEY_RCTRLDOT;

	if ((Macro.KeyMacroCtrlShiftDot=KeyNameToKey(Macro.strKeyMacroCtrlShiftDot)) == static_cast<DWORD>(-1))
		Macro.KeyMacroCtrlShiftDot=KEY_CTRLSHIFTDOT;

	if ((Macro.KeyMacroRCtrlShiftDot=KeyNameToKey(Macro.strKeyMacroRCtrlShiftDot)) == static_cast<DWORD>(-1))
		Macro.KeyMacroRCtrlShiftDot=KEY_RCTRL|KEY_SHIFT|KEY_DOT;

	EdOpt.strWordDiv = strWordDiv;
	FileList::ReadPanelModes();

	/* BUGBUG??
	// уточняем системную политику
	// для дисков юзер может только отменять показ
	Policies.ShowHiddenDrives&=OptPolicies_ShowHiddenDrives;
	// для опций юзер может только добавлять блокироку пунктов
	Policies.DisabledOptions|=OptPolicies_DisabledOptions;
	*/

	if (Exec.strExecuteBatchType.IsEmpty()) // предохраняемся
		Exec.strExecuteBatchType=constBatchExt;

	// Инициализация XLat для русской раскладки qwerty<->йцукен
	if (XLat.Table[0].IsEmpty())
	{
		bool RussianExists=false;
		HKL Layouts[32];
		UINT Count=GetKeyboardLayoutList(ARRAYSIZE(Layouts), Layouts);
		WORD RussianLanguageId=MAKELANGID(LANG_RUSSIAN, SUBLANG_RUSSIAN_RUSSIA);
		for (UINT I=0; I<Count; I++)
		{
			if (LOWORD(Layouts[I]) == RussianLanguageId)
			{
				RussianExists = true;
				break;
			}
		}

		if (RussianExists)
		{
			XLat.Table[0] = L"\x2116\x0410\x0412\x0413\x0414\x0415\x0417\x0418\x0419\x041a\x041b\x041c\x041d\x041e\x041f\x0420\x0421\x0422\x0423\x0424\x0425\x0426\x0427\x0428\x0429\x042a\x042b\x042c\x042f\x0430\x0432\x0433\x0434\x0435\x0437\x0438\x0439\x043a\x043b\x043c\x043d\x043e\x043f\x0440\x0441\x0442\x0443\x0444\x0445\x0446\x0447\x0448\x0449\x044a\x044b\x044c\x044d\x044f\x0451\x0401\x0411\x042e";
			XLat.Table[1] = L"#FDULTPBQRKVYJGHCNEA{WXIO}SMZfdultpbqrkvyjghcnea[wxio]sm'z`~<>";
			XLat.Rules[0] = L",??&./\x0431,\x044e.:^\x0416:\x0436;;$\"@\x042d\"";
			XLat.Rules[1] = L"?,&?/.,\x0431.\x044e^::\x0416;\x0436$;@\"\"\x042d";
			XLat.Rules[2] = L"^::\x0416\x0416^$;;\x0436\x0436$@\"\"\x042d\x042d@&??,,\x0431\x0431&/..\x044e\x044e/";
		}
	}

	{
		XLat.CurrentLayout=0;
		ClearArray(XLat.Layouts);

		if (!XLat.strLayouts.IsEmpty())
		{
			wchar_t *endptr;
			const wchar_t *ValPtr;
			UserDefinedList DestList(ULF_UNIQUE);
			DestList.Set(XLat.strLayouts);
			size_t I=0;

			while (nullptr!=(ValPtr=DestList.GetNext()))
			{
				DWORD res=(DWORD)wcstoul(ValPtr, &endptr, 16);
				XLat.Layouts[I]=(HKL)(intptr_t)(HIWORD(res)? res : MAKELONG(res,res));
				++I;

				if (I >= ARRAYSIZE(XLat.Layouts))
					break;
			}

			if (I <= 1) // если указано меньше двух - "откключаем" эту
				XLat.Layouts[0]=0;
		}
	}

	ClearArray(FindOpt.OutColumnTypes);
	ClearArray(FindOpt.OutColumnWidths);
	ClearArray(FindOpt.OutColumnWidthType);
	FindOpt.OutColumnCount=0;


	if (!FindOpt.strSearchOutFormat.IsEmpty())
	{
		if (FindOpt.strSearchOutFormatWidth.IsEmpty())
			FindOpt.strSearchOutFormatWidth=L"0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0";
		TextToViewSettings(FindOpt.strSearchOutFormat,FindOpt.strSearchOutFormatWidth,
                                  FindOpt.OutColumnTypes,FindOpt.OutColumnWidths,FindOpt.OutColumnWidthType,
                                  FindOpt.OutColumnCount);
	}

	string tmp[2];
	if (!Global->Db->GeneralCfg()->EnumValues(L"Masks", 0, tmp[0], tmp[1]))
	{
		ApplyDefaultMaskGroups();
	}

	StrToGuid(KnownIDs.EmenuGuidStr, KnownIDs.Emenu);
	StrToGuid(KnownIDs.NetworkGuidStr, KnownIDs.Network);

/* *************************************************** </ПОСТПРОЦЕССЫ> */

	// we assume that any changes after this point will be made by the user
	for(size_t i = 0; i < FARConfig.Size; ++i)
	{
		FARConfig.Items[i].Value->MakeUnchanged();
	}
}


void Options::Save(bool Ask)
{
	InitConfig();

	if (Policies.DisabledOptions&0x20000) // Bit 17 - Сохранить параметры
		return;

	if (Ask && Message(0,2,MSG(MSaveSetupTitle),MSG(MSaveSetupAsk1),MSG(MSaveSetupAsk2),MSG(MSaveSetup),MSG(MCancel)))
		return;

	/* <ПРЕПРОЦЕССЫ> *************************************************** */
	Panel *LeftPanelPtr=Global->CtrlObject->Cp()->LeftPanel;
	Panel *RightPanelPtr=Global->CtrlObject->Cp()->RightPanel;
	LeftPanel.Focus=LeftPanelPtr->GetFocus() != 0;
	LeftPanel.Visible=LeftPanelPtr->IsVisible() != 0;
	RightPanel.Focus=RightPanelPtr->GetFocus() != 0;
	RightPanel.Visible=RightPanelPtr->IsVisible() != 0;

	if (LeftPanelPtr->GetMode()==NORMAL_PANEL)
	{
		LeftPanel.Type=LeftPanelPtr->GetType();
		LeftPanel.ViewMode=LeftPanelPtr->GetViewMode();
		LeftPanel.SortMode=LeftPanelPtr->GetSortMode();
		LeftPanel.SortOrder=LeftPanelPtr->GetSortOrder();
		LeftPanel.SortGroups=LeftPanelPtr->GetSortGroups() != 0;
		LeftPanel.ShowShortNames=LeftPanelPtr->GetShowShortNamesMode() != 0;
		LeftPanel.NumericSort=LeftPanelPtr->GetNumericSort() != 0;
		LeftPanel.CaseSensitiveSort=LeftPanelPtr->GetCaseSensitiveSort() != 0;
		LeftSelectedFirst=LeftPanelPtr->GetSelectedFirstMode() != 0;
		LeftPanel.DirectoriesFirst=LeftPanelPtr->GetDirectoriesFirst() != 0;
	}

	string strTemp1, strTemp2;
	LeftPanelPtr->GetCurDir(strTemp1);
	strLeftFolder = strTemp1;
	LeftPanelPtr->GetCurBaseName(strTemp1, strTemp2);
	strLeftCurFile = strTemp1;
	if (RightPanelPtr->GetMode()==NORMAL_PANEL)
	{
		RightPanel.Type=RightPanelPtr->GetType();
		RightPanel.ViewMode=RightPanelPtr->GetViewMode();
		RightPanel.SortMode=RightPanelPtr->GetSortMode();
		RightPanel.SortOrder=RightPanelPtr->GetSortOrder();
		RightPanel.SortGroups=RightPanelPtr->GetSortGroups() != 0;
		RightPanel.ShowShortNames=RightPanelPtr->GetShowShortNamesMode() != 0;
		RightPanel.NumericSort=RightPanelPtr->GetNumericSort() != 0;
		RightPanel.CaseSensitiveSort=RightPanelPtr->GetCaseSensitiveSort() != 0;
		RightSelectedFirst=RightPanelPtr->GetSelectedFirstMode() != 0;
		RightPanel.DirectoriesFirst=RightPanelPtr->GetDirectoriesFirst() != 0;
	}

	RightPanelPtr->GetCurDir(strTemp1);
	strRightFolder = strTemp1;
	RightPanelPtr->GetCurBaseName(strTemp1, strTemp2);
	strRightCurFile = strTemp1;
	Global->CtrlObject->HiFiles->SaveHiData();
	/* *************************************************** </ПРЕПРОЦЕССЫ> */

	Palette.Save();

	std::for_each(RANGE(ConfigList, i)
	{
		i.first->BeginTransaction();
		for (size_t j=0; j < i.second->Size; ++j)
		{
			i.second->Items[j].Value->StoreValue(i.first, i.second->Items[j].KeyName, i.second->Items[j].ValName);
		}
		i.first->EndTransaction();
	});

	/* <ПОСТПРОЦЕССЫ> *************************************************** */
	FileFilter::SaveFilters();
	FileList::SavePanelModes();

	if (Ask)
		Global->CtrlObject->Macro.SaveMacros();

	/* *************************************************** </ПОСТПРОЦЕССЫ> */
}

void FillListItem(FarListItem& Item, FormatString& fs, FARConfigItem& cfg)
{
	Item.Flags = 0;
	Item.Reserved[0] = Item.Reserved[1] = 0;
	fs.Clear();
	fs << fmt::ExactWidth(42) << fmt::LeftAlign() << (string(cfg.KeyName) + "." + cfg.ValName) << BoxSymbols[BS_V1]
	<< fmt::ExactWidth(7) << fmt::LeftAlign() << cfg.Value->typeToString() << BoxSymbols[BS_V1];
	fs << cfg.Value->toString();
	if (cfg.ValueType == Option::TYPE_INTEGER)
	{
		int v = static_cast<IntOption*>(cfg.Value)->Get();
		wchar_t w1 = static_cast<wchar_t>(v);
		wchar_t w2 = static_cast<wchar_t>(v >> 16);
		fs << L" = 0x" << fmt::MaxWidth(8) << fmt::Radix(16) << v;
		if (w1 > 0x001f && w1 < 0x8000)
		{
			fs << L" = '" << w1;
			if (w2 > 0x001f && w2 < 0x8000) fs << w2;
			fs << L"'";
		}
	}
	if(cfg.Changed())
	{
		Item.Flags = LIF_CHECKED|L'*';
	}
	Item.Text = fs;
}

intptr_t AdvancedConfigDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	static FormatString* fs;
	switch (Msg)
	{
	case DN_INITDIALOG:
		fs = reinterpret_cast<FormatString*>(Param2);
		break;

	case DN_RESIZECONSOLE:
		{
			COORD Size = {(SHORT)std::max(ScrX-4, 60), (SHORT)std::max(ScrY-2, 20)};
			Dlg->SendMessage(DM_RESIZEDIALOG, 0, &Size);
			SMALL_RECT ListPos = {3, 1, (SHORT)(Size.X-4), (SHORT)(Size.Y-2)};
			Dlg->SendMessage(DM_SETITEMPOSITION, 0, &ListPos);
		}
		break;

	case DN_CONTROLINPUT:
		{
			const INPUT_RECORD* record= reinterpret_cast<const INPUT_RECORD*>(Param2);
			if (record->EventType==KEY_EVENT)
			{
				int key = InputRecordToKey(record);
				switch(key)
				{
				case KEY_SHIFTF1:
					{
						FarListInfo ListInfo = {sizeof(ListInfo)};
						Dlg->SendMessage(DM_LISTINFO, Param1, &ListInfo);

						string HelpTopic = string(FARConfig.Items[ListInfo.SelectPos].KeyName) + L"." + FARConfig.Items[ListInfo.SelectPos].ValName;
						Help hlp(HelpTopic.CPtr(), nullptr, FHELP_NOSHOWERROR);
						if (hlp.GetError())
						{
							HelpTopic = string(FARConfig.Items[ListInfo.SelectPos].KeyName) + L"Settings";
							Help hlp1(HelpTopic.CPtr(), nullptr, FHELP_NOSHOWERROR);
						}
					}
					break;

				case KEY_F4:
					Dlg->SendMessage(DM_CLOSE, 0, nullptr);
					break;

				case KEY_SHIFTF4:
					Dlg->SendMessage(DM_CLOSE, 1, nullptr);
					break;

				case KEY_CTRLH:
					{
						static bool HideUnchanged = true;
						Dlg->SendMessage(DM_ENABLEREDRAW, 0 , 0);
						FarListInfo ListInfo = {sizeof(ListInfo)};
						Dlg->SendMessage(DM_LISTINFO, Param1, &ListInfo);
						for(int i = 0; i < static_cast<int>(ListInfo.ItemsNumber); ++i)
						{
							FarListGetItem Item={sizeof(FarListGetItem), i};
							Dlg->SendMessage(DM_LISTGETITEM, 0, &Item);
							bool NeedUpdate = false;
							if(HideUnchanged)
							{
								if(!(Item.Item.Flags&LIF_CHECKED))
								{
									Item.Item.Flags|=LIF_HIDDEN;
									NeedUpdate = true;
								}
							}
							else
							{
								if(Item.Item.Flags&LIF_HIDDEN)
								{
									Item.Item.Flags&=~LIF_HIDDEN;
									NeedUpdate = true;
								}
							}
							if(NeedUpdate)
							{
								FarListUpdate UpdatedItem={sizeof(FarListGetItem), i, Item.Item};
								Dlg->SendMessage(DM_LISTUPDATE, 0, &UpdatedItem);
							}
						}
						HideUnchanged = !HideUnchanged;
						Dlg->SendMessage(DM_ENABLEREDRAW, 1 , 0);
					}
					break;
				}
			}
		}
		break;

	case DN_CLOSE:
		if (Param1 == 0 || Param1 == 1) // BUGBUG, magic
		{
			FarListInfo ListInfo = {sizeof(ListInfo)};
			Dlg->SendMessage(DM_LISTINFO, 0, &ListInfo);

			if (FARConfig.Items[ListInfo.SelectPos].Edit(Param1 != 0))
			{
				Dlg->SendMessage(DM_ENABLEREDRAW, 0 , 0);
				FarListUpdate flu = {sizeof(flu), ListInfo.SelectPos};
				FillListItem(flu.Item, fs[ListInfo.SelectPos], FARConfig.Items[ListInfo.SelectPos]);
				Dlg->SendMessage(DM_LISTUPDATE, 0, &flu);
				FarListPos flp = {sizeof(flp), ListInfo.SelectPos, ListInfo.TopPos};
				Dlg->SendMessage(DM_LISTSETCURPOS, 0, &flp);
				Dlg->SendMessage(DM_ENABLEREDRAW, 1 , 0);
			}
			return FALSE;
		}
		break;
	default:
		break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

bool AdvancedConfig()
{
	int DlgWidth = std::max(ScrX-4, 60), DlgHeight = std::max(ScrY-2, 20);
	FarDialogItem AdvancedConfigDlgData[]=
	{
		{DI_LISTBOX,3,1,DlgWidth-4,DlgHeight-2,0,nullptr,nullptr,DIF_NONE,nullptr},
	};
	MakeDialogItemsEx(AdvancedConfigDlgData,AdvancedConfigDlg);

	FarList Items={sizeof(FarList)};
	Items.ItemsNumber = FARConfig.Size;
	Items.Items = new FarListItem[Items.ItemsNumber];

	FormatString *fs = new FormatString[FARConfig.Size];
	for(size_t i = 0; i < Items.ItemsNumber; ++i)
	{
		FillListItem(Items.Items[i], fs[i], FARConfig.Items[i]);
	}

	AdvancedConfigDlg[0].ListItems = &Items;

	Dialog Dlg(AdvancedConfigDlg,ARRAYSIZE(AdvancedConfigDlg), AdvancedConfigDlgProc, fs);
	Dlg.SetHelp(L"FarConfig");
	Dlg.SetPosition(-1, -1, DlgWidth, DlgHeight);
	Dlg.Process();
	delete[] fs;
	delete[] Items.Items;
	return true;
}


bool BoolOption::ReceiveValue(GeneralConfig* Storage, const wchar_t* KeyName, const wchar_t* ValueName, bool Default)
{
	int CfgValue = Default;
	bool Result = Storage->GetValue(KeyName, ValueName, &CfgValue, CfgValue);
	Set(CfgValue != 0);
	return Result;
}

bool BoolOption::StoreValue(GeneralConfig* Storage, const wchar_t* KeyName, const wchar_t* ValueName)
{
	return !Changed() || Storage->SetValue(KeyName, ValueName, Get());
}

bool Bool3Option::ReceiveValue(GeneralConfig* Storage, const wchar_t* KeyName, const wchar_t* ValueName, int Default)
{
	int CfgValue = Default;
	bool Result = Storage->GetValue(KeyName, ValueName, &CfgValue, CfgValue);
	Set(CfgValue);
	return Result;
}

bool Bool3Option::StoreValue(GeneralConfig* Storage, const wchar_t* KeyName, const wchar_t* ValueName)
{
	return !Changed() || Storage->SetValue(KeyName, ValueName, Get());
}

bool IntOption::ReceiveValue(GeneralConfig* Storage, const wchar_t* KeyName, const wchar_t* ValueName, intptr_t Default)
{
	int CfgValue = Default;
	bool Result = Storage->GetValue(KeyName, ValueName, &CfgValue, CfgValue);
	Set(CfgValue);
	return Result;
}

bool IntOption::StoreValue(GeneralConfig* Storage, const wchar_t* KeyName, const wchar_t* ValueName)
{
	return !Changed() || Storage->SetValue(KeyName, ValueName, Get());
}

bool StringOption::ReceiveValue(GeneralConfig* Storage, const wchar_t* KeyName, const wchar_t* ValueName, const wchar_t* Default)
{
	string CfgValue = Default;
	bool Result = Storage->GetValue(KeyName, ValueName, CfgValue, CfgValue);
	Set(CfgValue);
	return Result;
}

bool StringOption::StoreValue(GeneralConfig* Storage, const wchar_t* KeyName, const wchar_t* ValueName)
{
	return !Changed() || Storage->SetValue(KeyName, ValueName, Get());
}
