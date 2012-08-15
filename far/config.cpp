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

Options Opt={};

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

	DialogItemEx *DeleteToRecycleBin = Builder.AddCheckbox(MConfigRecycleBin, Opt.DeleteToRecycleBin);
	DialogItemEx *DeleteLinks = Builder.AddCheckbox(MConfigRecycleBinLink, Opt.DeleteToRecycleBinKillLink);
	DeleteLinks->Indent(4);
	Builder.LinkFlags(DeleteToRecycleBin, DeleteLinks, DIF_DISABLE);

	Builder.AddCheckbox(MConfigSystemCopy, Opt.CMOpt.UseSystemCopy);
	Builder.AddCheckbox(MConfigCopySharing, Opt.CMOpt.CopyOpened);
	Builder.AddCheckbox(MConfigScanJunction, Opt.ScanJunction);
	Builder.AddCheckbox(MConfigCreateUppercaseFolders, Opt.CreateUppercaseFolders);
	Builder.AddCheckbox(MConfigSmartFolderMonitor, Opt.SmartFolderMonitor);

	Builder.AddCheckbox(MConfigSaveHistory, Opt.SaveHistory);
	Builder.AddCheckbox(MConfigSaveFoldersHistory, Opt.SaveFoldersHistory);
	Builder.AddCheckbox(MConfigSaveViewHistory, Opt.SaveViewHistory);
	Builder.AddCheckbox(MConfigRegisteredTypes, Opt.UseRegisteredTypes);
	Builder.AddCheckbox(MConfigCloseCDGate, Opt.CloseCDGate);
	Builder.AddCheckbox(MConfigUpdateEnvironment, Opt.UpdateEnvironment);
	Builder.AddText(MConfigElevation);
	Builder.AddCheckbox(MConfigElevationModify, Opt.StoredElevationMode, ELEVATION_MODIFY_REQUEST)->Indent(4);
	Builder.AddCheckbox(MConfigElevationRead, Opt.StoredElevationMode, ELEVATION_READ_REQUEST)->Indent(4);
	Builder.AddCheckbox(MConfigElevationUsePrivileges, Opt.StoredElevationMode, ELEVATION_USE_PRIVILEGES)->Indent(4);
	Builder.AddCheckbox(MConfigAutoSave, Opt.AutoSaveSetup);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		Opt.ElevationMode = Opt.StoredElevationMode;
	}
}


void PanelSettings()
{
	DialogBuilder Builder(MConfigPanelTitle, L"PanelSettings");
	BOOL AutoUpdate = Opt.AutoUpdateLimit;

	Builder.AddCheckbox(MConfigHidden, Opt.ShowHidden);
	Builder.AddCheckbox(MConfigHighlight, Opt.Highlight);
	Builder.AddCheckbox(MConfigSelectFolders, Opt.SelectFolders);
	Builder.AddCheckbox(MConfigSortFolderExt, Opt.SortFolderExt);
	Builder.AddCheckbox(MConfigReverseSort, Opt.ReverseSort);

	DialogItemEx *AutoUpdateEnabled = Builder.AddCheckbox(MConfigAutoUpdateLimit, &AutoUpdate);
	DialogItemEx *AutoUpdateLimit = Builder.AddIntEditField(Opt.AutoUpdateLimit, 6);
	Builder.LinkFlags(AutoUpdateEnabled, AutoUpdateLimit, DIF_DISABLE, false);
	DialogItemEx *AutoUpdateText = Builder.AddTextBefore(AutoUpdateLimit, MConfigAutoUpdateLimit2);
	AutoUpdateLimit->Indent(4);
	AutoUpdateText->Indent(4);
	Builder.AddCheckbox(MConfigAutoUpdateRemoteDrive, Opt.AutoUpdateRemoteDrive);

	Builder.AddSeparator();
	Builder.AddCheckbox(MConfigShowColumns, Opt.ShowColumnTitles);
	Builder.AddCheckbox(MConfigShowStatus, Opt.ShowPanelStatus);
	Builder.AddCheckbox(MConfigDetailedJunction, Opt.PanelDetailedJunction);
	Builder.AddCheckbox(MConfigShowTotal, Opt.ShowPanelTotals);
	Builder.AddCheckbox(MConfigShowFree, Opt.ShowPanelFree);
	Builder.AddCheckbox(MConfigShowScrollbar, Opt.ShowPanelScrollbar);
	Builder.AddCheckbox(MConfigShowScreensNumber, Opt.ShowScreensNumber);
	Builder.AddCheckbox(MConfigShowSortMode, Opt.ShowSortMode);
	Builder.AddCheckbox(MConfigShowDotsInRoot, Opt.ShowDotsInRoot);
	Builder.AddCheckbox(MConfigHighlightColumnSeparator, Opt.HighlightColumnSeparator);
	Builder.AddCheckbox(MConfigDoubleGlobalColumnSeparator, Opt.DoubleGlobalColumnSeparator);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (!AutoUpdate)
			Opt.AutoUpdateLimit = 0;

	//  FrameManager->RefreshFrame();
		CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
		CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
		CtrlObject->Cp()->Redraw();
	}
}

void TreeSettings()
{
	DialogBuilder Builder(MConfigTreeTitle, L"TreeSettings");

	DialogItemEx *TemplateEdit;

	Builder.AddCheckbox(MConfigTreeAutoChange, Opt.Tree.AutoChangeFolder);

	TemplateEdit = Builder.AddIntEditField(Opt.Tree.MinTreeCount, 3);
	Builder.AddTextBefore(TemplateEdit, MConfigTreeLabelMinFolder);

#if defined(TREEFILE_PROJECT)
	DialogItemEx *Checkbox;

	Builder.AddSeparator(MConfigTreeLabel1);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelLocalDisk, &Opt.Tree.LocalDisk);
	TemplateEdit = Builder.AddEditField(&Opt.Tree.strLocalDisk, 36);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelNetDisk, &Opt.Tree.NetDisk);
	TemplateEdit = Builder.AddEditField(&Opt.Tree.strNetDisk, 36);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelNetPath, &Opt.Tree.NetPath);
	TemplateEdit = Builder.AddEditField(&Opt.Tree.strNetPath, 36);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelRemovableDisk, &Opt.Tree.RemovableDisk);
	TemplateEdit = Builder.AddEditField(&Opt.Tree.strRemovableDisk, 36);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelCDDisk, &Opt.Tree.CDDisk);
	TemplateEdit = Builder.AddEditField(&Opt.Tree.strCDDisk, 36);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Builder.AddText(MConfigTreeLabelSaveLocalPath);
	Builder.AddEditField(&Opt.Tree.strSaveLocalPath, 40);

	Builder.AddText(MConfigTreeLabelSaveNetPath);
	Builder.AddEditField(&Opt.Tree.strSaveNetPath, 40);

	Builder.AddText(MConfigTreeLabelExceptPath);
	Builder.AddEditField(&Opt.Tree.strExceptPath, 40);
#endif

	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
		CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
		CtrlObject->Cp()->Redraw();
	}
}


/* $ 17.12.2001 IS
   Настройка средней кнопки мыши для панелей. Воткнем пока сюда, потом надо
   переехать в специальный диалог по программированию мыши.
*/
void InterfaceSettings()
{
	DialogBuilder Builder(MConfigInterfaceTitle, L"InterfSettings");

	Builder.AddCheckbox(MConfigClock, Opt.Clock);
	Builder.AddCheckbox(MConfigViewerEditorClock, Opt.ViewerEditorClock);
	Builder.AddCheckbox(MConfigMouse, Opt.Mouse);
	Builder.AddCheckbox(MConfigKeyBar, Opt.ShowKeyBar);
	Builder.AddCheckbox(MConfigMenuBar, Opt.ShowMenuBar);
	DialogItemEx *SaverCheckbox = Builder.AddCheckbox(MConfigSaver, Opt.ScreenSaver);

	DialogItemEx *SaverEdit = Builder.AddIntEditField(Opt.ScreenSaverTime, 2);
	SaverEdit->Indent(4);
	Builder.AddTextAfter(SaverEdit, MConfigSaverMinutes);
	Builder.LinkFlags(SaverCheckbox, SaverEdit, DIF_DISABLE);

	Builder.AddCheckbox(MConfigCopyTotal, Opt.CMOpt.CopyShowTotal);
	Builder.AddCheckbox(MConfigCopyTimeRule, Opt.CMOpt.CopyTimeRule);
	Builder.AddCheckbox(MConfigDeleteTotal, Opt.DelOpt.DelShowTotal);
	Builder.AddCheckbox(MConfigPgUpChangeDisk, Opt.PgUpChangeDisk);
	Builder.AddCheckbox(MConfigClearType, Opt.ClearType);
	DialogItemEx* SetIconCheck = Builder.AddCheckbox(MConfigSetConsoleIcon, Opt.SetIcon);
	DialogItemEx* SetAdminIconCheck = Builder.AddCheckbox(MConfigSetAdminConsoleIcon, Opt.SetAdminIcon);
	SetAdminIconCheck->Indent(4);
	Builder.LinkFlags(SetIconCheck, SetAdminIconCheck, DIF_DISABLE);
	Builder.AddText(MConfigTitleAddons);
	Builder.AddEditField(Opt.strTitleAddons, 47);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (Opt.CMOpt.CopyTimeRule)
			Opt.CMOpt.CopyTimeRule = 3;

		SetFarConsoleMode();
		CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
		CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
		CtrlObject->Cp()->SetScreenPosition();
		// $ 10.07.2001 SKV ! надо это делать, иначе если кейбар спрятали, будет полный рамс.
		CtrlObject->Cp()->Redraw();
	}
}

void AutoCompleteSettings()
{
	DialogBuilder Builder(MConfigAutoCompleteTitle, L"AutoCompleteSettings");
	DialogItemEx *ListCheck=Builder.AddCheckbox(MConfigAutoCompleteShowList, Opt.AutoComplete.ShowList);
	DialogItemEx *ModalModeCheck=Builder.AddCheckbox(MConfigAutoCompleteModalList, Opt.AutoComplete.ModalList);
	ModalModeCheck->Indent(4);
	Builder.AddCheckbox(MConfigAutoCompleteAutoAppend, Opt.AutoComplete.AppendCompletion);
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
	Builder.AddCheckbox(MConfigInfoPanelShowPowerStatus, Opt.InfoPanel.ShowPowerStatus);
	Builder.AddCheckbox(MConfigInfoPanelShowCDInfo, Opt.InfoPanel.ShowCDInfo);
	Builder.AddText(MConfigInfoPanelCNTitle);
	Builder.AddComboBox(Opt.InfoPanel.ComputerNameFormat, 50, CNListItems, ARRAYSIZE(CNListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddText(MConfigInfoPanelUNTitle);
	Builder.AddComboBox(Opt.InfoPanel.UserNameFormat, 50, UNListItems, ARRAYSIZE(UNListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		bool needRedraw=false;
		if (CtrlObject->Cp()->LeftPanel->GetType() == INFO_PANEL)
		{
			CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
			needRedraw=true;
		}
		if (CtrlObject->Cp()->RightPanel->GetType() == INFO_PANEL)
		{
			CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
			needRedraw=true;
		}
		if (needRedraw)
		{
			//CtrlObject->Cp()->SetScreenPosition();
			CtrlObject->Cp()->Redraw();
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
		{L"arc", L"*.rar,*.zip,*.[zj],*.[bg7]z,*.[bg]zip,*.tar,*.t[agbx]z,*.ar[cj],*.r[0-9][0-9],*.a[0-9][0-9],*.bz2,*.cab,*.msi,*.jar,*.lha,*.lzh,*.ha,*.ac[bei],*.pa[ck],*.rk,*.cpio,*.rpm,*.zoo,*.hqx,*.sit,*.ice,*.uc2,*.ain,*.imp,*.777,*.ufa,*.boa,*.bs[2a],*.sea,*.hpk,*.ddi,*.x2,*.rkv,*.[lw]sz,*.h[ay]p,*.lim,*.sqz,*.chz"},
		{L"temp", L"*.bak,*.tmp"},
		{L"exec", L"*.exe,*.com,*.bat,*.cmd,%PATHEXT%"},
	};

	for (size_t i = 0; i < ARRAYSIZE(Sets); ++i)
	{
		GeneralCfg->SetValue(L"Masks", Sets[i].Group, Sets[i].Mask);
	}
}

void FillMasksMenu(VMenu& MasksMenu, int SelPos = 0)
{
	MasksMenu.DeleteItems();
	string Name, Value;
	for(DWORD i = 0; GeneralCfg->EnumValues(L"Masks", i, Name, Value); ++i)
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
	VMenu MasksMenu(MSG(MMenuMaskGroups), nullptr, 0, 0, VMENU_WRAPMODE|VMENU_SHOWAMPERSAND);
	MasksMenu.SetBottomTitle(MSG(MMaskGroupBottom));
	MasksMenu.SetHelp(L"MaskGroupsSettings");
	FillMasksMenu(MasksMenu);
	MasksMenu.SetPosition(-1, -1, -1, -1);
	MasksMenu.Show();

	bool Changed = false;
	while (!MasksMenu.Done())
	{
		DWORD Key=MasksMenu.ReadInput();
		int ItemPos = MasksMenu.GetSelectPos();
		void* Data = MasksMenu.GetUserData(nullptr, 0, ItemPos);
		const wchar_t* Item = static_cast<const wchar_t*>(Data);
		switch (Key)
		{
		case KEY_NUMDEL:
		case KEY_DEL:
			if(Item && !Message(0,2,MSG(MMenuMaskGroups),MSG(MMaskGroupAskDelete), Item, MSG(MDelete), MSG(MCancel)))
			{
				GeneralCfg->DeleteValue(L"Masks", Item);
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
					GeneralCfg->GetValue(L"Masks", Name, Value, L"");
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
						GeneralCfg->DeleteValue(L"Masks", Item);
					}
					GeneralCfg->SetValue(L"Masks", Name, Value);
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
						GeneralCfg->GetValue(L"Masks", static_cast<const wchar_t*>(MasksMenu.GetUserData(nullptr, 0, i)), CurrentMasks, L"");
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
					MasksMenu.Show();
					while (!MasksMenu.Done())
					{
						DWORD Key=MasksMenu.ReadInput();
						if(Key == KEY_ESC || Key == KEY_F10 || Key == KEY_ENTER || Key == KEY_NUMENTER)
							break;
						else
							MasksMenu.ProcessKey(Key);
					}
					for (int i = 0; i < MasksMenu.GetItemCount(); ++i)
					{
						MasksMenu.UpdateItemFlags(i, MasksMenu.GetItemPtr(i)->Flags&~MIF_HIDDEN);
					}
					MasksMenu.SetPosition(-1, -1, -1, -1);
					MasksMenu.SetTitle(MSG(MMenuMaskGroups));
					MasksMenu.SetBottomTitle(MSG(MMaskGroupBottom));
					MasksMenu.Show();
				}
			}
			break;


		default:
			MasksMenu.ProcessInput();
			break;
		}

		if(Changed)
		{
			Changed = false;

			FillMasksMenu(MasksMenu, MasksMenu.GetSelectPos());
			MasksMenu.SetPosition(-1, -1, -1, -1);
			MasksMenu.SetUpdateRequired(true);
			MasksMenu.Show();
		}
	}
}

void DialogSettings()
{
	DialogBuilder Builder(MConfigDlgSetsTitle, L"DialogSettings");

	Builder.AddCheckbox(MConfigDialogsEditHistory, Opt.Dialogs.EditHistory);
	Builder.AddCheckbox(MConfigDialogsEditBlock, Opt.Dialogs.EditBlock);
	Builder.AddCheckbox(MConfigDialogsDelRemovesBlocks, Opt.Dialogs.DelRemovesBlocks);
	Builder.AddCheckbox(MConfigDialogsAutoComplete, Opt.Dialogs.AutoComplete);
	Builder.AddCheckbox(MConfigDialogsEULBsClear, Opt.Dialogs.EULBsClear);
	Builder.AddCheckbox(MConfigDialogsMouseButton, Opt.Dialogs.MouseButton);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (Opt.Dialogs.MouseButton )
			Opt.Dialogs.MouseButton = 0xFFFF;
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
	Builder.AddComboBox(Opt.VMenu.LBtnClick, 40, CAListItems, ARRAYSIZE(CAListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddText(MConfigVMenuRBtnClick);
	Builder.AddComboBox(Opt.VMenu.RBtnClick, 40, CAListItems, ARRAYSIZE(CAListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddText(MConfigVMenuMBtnClick);
	Builder.AddComboBox(Opt.VMenu.MBtnClick, 40, CAListItems, ARRAYSIZE(CAListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddOKCancel();
	Builder.ShowDialog();
}

void CmdlineSettings()
{
	DialogBuilder Builder(MConfigCmdlineTitle, L"CmdlineSettings");

	Builder.AddCheckbox(MConfigCmdlineEditBlock, Opt.CmdLine.EditBlock);
	Builder.AddCheckbox(MConfigCmdlineDelRemovesBlocks, Opt.CmdLine.DelRemovesBlocks);
	Builder.AddCheckbox(MConfigCmdlineAutoComplete, Opt.CmdLine.AutoComplete);
	DialogItemEx *UsePromptFormat = Builder.AddCheckbox(MConfigCmdlineUsePromptFormat, Opt.CmdLine.UsePromptFormat);
	DialogItemEx *PromptFormat = Builder.AddEditField(Opt.CmdLine.strPromptFormat, 33);
	PromptFormat->Indent(4);
	Builder.LinkFlags(UsePromptFormat, PromptFormat, DIF_DISABLE);

	UsePromptFormat = Builder.AddCheckbox(MConfigCmdlineUseHomeDir, Opt.Exec.UseHomeDir);
	PromptFormat = Builder.AddEditField(Opt.Exec.strHomeDir, 33);
	PromptFormat->Indent(4);
	Builder.LinkFlags(UsePromptFormat, PromptFormat, DIF_DISABLE);

	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		CtrlObject->CmdLine->SetPersistentBlocks(Opt.CmdLine.EditBlock);
		CtrlObject->CmdLine->SetDelRemovesBlocks(Opt.CmdLine.DelRemovesBlocks);
		CtrlObject->CmdLine->SetAutoComplete(Opt.CmdLine.AutoComplete);
	}
}

void SetConfirmations()
{
	DialogBuilder Builder(MSetConfirmTitle, L"ConfirmDlg");

	Builder.AddCheckbox(MSetConfirmCopy, Opt.Confirm.Copy);
	Builder.AddCheckbox(MSetConfirmMove, Opt.Confirm.Move);
	Builder.AddCheckbox(MSetConfirmRO, Opt.Confirm.RO);
	Builder.AddCheckbox(MSetConfirmDrag, Opt.Confirm.Drag);
	Builder.AddCheckbox(MSetConfirmDelete, Opt.Confirm.Delete);
	Builder.AddCheckbox(MSetConfirmDeleteFolders, Opt.Confirm.DeleteFolder);
	Builder.AddCheckbox(MSetConfirmEsc, Opt.Confirm.Esc);
	Builder.AddCheckbox(MSetConfirmRemoveConnection, Opt.Confirm.RemoveConnection);
	Builder.AddCheckbox(MSetConfirmRemoveSUBST, Opt.Confirm.RemoveSUBST);
	Builder.AddCheckbox(MSetConfirmDetachVHD, Opt.Confirm.DetachVHD);
	Builder.AddCheckbox(MSetConfirmRemoveHotPlug, Opt.Confirm.RemoveHotPlug);
	Builder.AddCheckbox(MSetConfirmAllowReedit, Opt.Confirm.AllowReedit);
	Builder.AddCheckbox(MSetConfirmHistoryClear, Opt.Confirm.HistoryClear);
	Builder.AddCheckbox(MSetConfirmExit, Opt.Confirm.Exit);
	Builder.AddOKCancel();

	Builder.ShowDialog();
}

void PluginsManagerSettings()
{
	DialogBuilder Builder(MPluginsManagerSettingsTitle, L"PluginsManagerSettings");
#ifndef NO_WRAPPER
	Builder.AddCheckbox(MPluginsManagerOEMPluginsSupport, Opt.LoadPlug.OEMPluginsSupport);
#endif // NO_WRAPPER
	Builder.AddCheckbox(MPluginsManagerScanSymlinks, Opt.LoadPlug.ScanSymlinks);
	Builder.AddSeparator(MPluginConfirmationTitle);
	Builder.AddCheckbox(MPluginsManagerOFP, Opt.PluginConfirm.OpenFilePlugin);
	DialogItemEx *StandardAssoc = Builder.AddCheckbox(MPluginsManagerStdAssoc, Opt.PluginConfirm.StandardAssociation);
	DialogItemEx *EvenIfOnlyOne = Builder.AddCheckbox(MPluginsManagerEvenOne, Opt.PluginConfirm.EvenIfOnlyOnePlugin);
	StandardAssoc->Indent(2);
	EvenIfOnlyOne->Indent(4);

	Builder.AddCheckbox(MPluginsManagerSFL, Opt.PluginConfirm.SetFindList);
	Builder.AddCheckbox(MPluginsManagerPF, Opt.PluginConfirm.Prefix);
	Builder.AddOKCancel();

	Builder.ShowDialog();
}


void SetDizConfig()
{
	DialogBuilder Builder(MCfgDizTitle, L"FileDiz");

	Builder.AddText(MCfgDizListNames);
	Builder.AddEditField(Opt.Diz.strListNames, 65);
	Builder.AddSeparator();

	Builder.AddCheckbox(MCfgDizSetHidden, Opt.Diz.SetHidden);
	Builder.AddCheckbox(MCfgDizROUpdate, Opt.Diz.ROUpdate);
	DialogItemEx *StartPos = Builder.AddIntEditField(Opt.Diz.StartPos, 2);
	Builder.AddTextAfter(StartPos, MCfgDizStartPos);
	Builder.AddSeparator();

	static int DizOptions[] = { MCfgDizNotUpdate, MCfgDizUpdateIfDisplayed, MCfgDizAlwaysUpdate };
	Builder.AddRadioButtons(Opt.Diz.UpdateMode, 3, DizOptions);
	Builder.AddSeparator();

	Builder.AddCheckbox(MCfgDizAnsiByDefault, Opt.Diz.AnsiByDefault);
	Builder.AddCheckbox(MCfgDizSaveInUTF, Opt.Diz.SaveInUTF);
	Builder.AddOKCancel();
	Builder.ShowDialog();
}

void ViewerConfig(ViewerOptions &ViOpt,bool Local)
{
	DialogBuilder Builder(MViewConfigTitle, L"ViewerSettings");
	if (!Local)
	{
		Builder.AddCheckbox(MViewConfigExternalF3, Opt.ViOpt.UseExternalViewer);
		Builder.AddText(MViewConfigExternalCommand);
		Builder.AddEditField(Opt.strExternalViewer, 64, L"ExternalViewer", DIF_EDITPATH|DIF_EDITPATHEXEC);
		Builder.AddSeparator(MViewConfigInternal);
	}

	Builder.StartColumns();
	Builder.AddCheckbox(MViewConfigPersistentSelection, ViOpt.PersistentBlocks);
	DialogItemEx *SavePos = Builder.AddCheckbox(MViewConfigSavePos, Opt.ViOpt.SavePos); // can't be local
	Builder.AddCheckbox(MViewConfigSaveCodepage, ViOpt.SaveCodepage);
	Builder.AddCheckbox(MViewConfigEditAutofocus, ViOpt.SearchEditFocus);
	DialogItemEx *TabSize = Builder.AddIntEditField(ViOpt.TabSize, 3);
	Builder.AddTextAfter(TabSize, MViewConfigTabSize);
	Builder.ColumnBreak();
	Builder.AddCheckbox(MViewConfigArrows, ViOpt.ShowArrows);
	DialogItemEx *SaveShortPos = Builder.AddCheckbox(MViewConfigSaveShortPos, Opt.ViOpt.SaveShortPos); // can't be local
	Builder.LinkFlags(SavePos, SaveShortPos, DIF_DISABLE);
	Builder.AddCheckbox(MViewConfigSaveWrapMode, ViOpt.SaveWrapMode);
	Builder.AddCheckbox(MViewConfigVisible0x00, ViOpt.Visible0x00);
	Builder.AddCheckbox(MViewConfigScrollbar, ViOpt.ShowScrollbar);
	Builder.EndColumns();

	if (!Local)
	{
		Builder.AddEmptyLine();
		DialogItemEx *MaxLineSize = Builder.AddIntEditField(Opt.ViOpt.MaxLineSize, 6);
		Builder.AddTextAfter(MaxLineSize, MViewConfigMaxLineSize);
		Builder.AddCheckbox(MViewAutoDetectCodePage, Opt.ViOpt.AutoDetectCodePage);
		Builder.AddCheckbox(MViewConfigAnsiCodePageAsDefault, Opt.ViOpt.AnsiCodePageAsDefault);
	}
	Builder.AddOKCancel();
	if (Builder.ShowDialog())
	{
		if (Opt.ViOpt.SavePos)
			ViOpt.SaveCodepage = true; // codepage is part of saved position
		if (ViOpt.TabSize<1 || ViOpt.TabSize>512)
			ViOpt.TabSize = DefaultTabSize;
		if (!Opt.ViOpt.MaxLineSize)
			Opt.ViOpt.MaxLineSize = ViewerOptions::eDefLineSize;
		else if (Opt.ViOpt.MaxLineSize < ViewerOptions::eMinLineSize)
			Opt.ViOpt.MaxLineSize = ViewerOptions::eMinLineSize;
		else if (Opt.ViOpt.MaxLineSize > ViewerOptions::eMaxLineSize)
			Opt.ViOpt.MaxLineSize = ViewerOptions::eMaxLineSize;
	}
}

void EditorConfig(EditorOptions &EdOpt,bool Local)
{
	DialogBuilder Builder(MEditConfigTitle, L"EditorSettings");
	if (!Local)
	{
		Builder.AddCheckbox(MEditConfigEditorF4, Opt.EdOpt.UseExternalEditor);
		Builder.AddText(MEditConfigEditorCommand);
		Builder.AddEditField(Opt.strExternalEditor, 64, L"ExternalEditor", DIF_EDITPATH|DIF_EDITPATHEXEC);
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
	DialogItemEx *SavePos = Builder.AddCheckbox(MEditConfigSavePos, EdOpt.SavePos);
	Builder.AddCheckbox(MEditConfigAutoIndent, EdOpt.AutoIndent);
	DialogItemEx *TabSize = Builder.AddIntEditField(EdOpt.TabSize, 3);
	Builder.AddTextAfter(TabSize, MEditConfigTabSize);
	Builder.AddCheckbox(MEditShowWhiteSpace, EdOpt.ShowWhiteSpace);
	Builder.ColumnBreak();
	Builder.AddCheckbox(MEditConfigDelRemovesBlocks, EdOpt.DelRemovesBlocks);
	DialogItemEx *SaveShortPos = Builder.AddCheckbox(MEditConfigSaveShortPos, EdOpt.SaveShortPos);
	Builder.LinkFlags(SavePos, SaveShortPos, DIF_DISABLE);
	Builder.AddCheckbox(MEditCursorBeyondEnd, EdOpt.CursorBeyondEOL);
	Builder.AddCheckbox(MEditConfigScrollbar, EdOpt.ShowScrollBar);
	Builder.AddCheckbox(MEditConfigPickUpWord, EdOpt.SearchPickUpWord);
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
	              Opt.InfoPanel.strFolderInfoFiles,strFolderInfoFiles,L"FolderDiz",FIB_ENABLEEMPTY|FIB_BUTTONS))
	{
		Opt.InfoPanel.strFolderInfoFiles = strFolderInfoFiles;

		if (CtrlObject->Cp()->LeftPanel->GetType() == INFO_PANEL)
			CtrlObject->Cp()->LeftPanel->Update(0);

		if (CtrlObject->Cp()->RightPanel->GetType() == INFO_PANEL)
			CtrlObject->Cp()->RightPanel->Update(0);
	}
}

template<class T>const GeneralConfig::OptionType TypeId(const T& Type);

template<>const GeneralConfig::OptionType TypeId<BoolOption>(const BoolOption& Type){return GeneralConfig::TYPE_BOOLEAN;}
template<>const GeneralConfig::OptionType TypeId<Bool3Option>(const Bool3Option& Type){return GeneralConfig::TYPE_BOOLEAN3;}
template<>const GeneralConfig::OptionType TypeId<IntOption>(const IntOption& Type){return GeneralConfig::TYPE_INTEGER;}
template<>const GeneralConfig::OptionType TypeId<StringOption>(const StringOption& Type){return GeneralConfig::TYPE_STRING;}

// Структура, описывающая всю конфигурацию(!)
static struct FARConfig
{
	size_t ApiRoot;
	const wchar_t *KeyName;
	const wchar_t *ValName;
	Option* Value;   // адрес переменной, куда помещаем данные
	GeneralConfig::OptionType ValueType;  // TYPE_BOOLEAN, TYPE_BOOLEAN3, TYPE_INTEGER, TYPE_STRING
	union
	{
		const void* Default;
		const wchar_t* sDefault;
		int iDefault;
		bool bDefault;
	};
} CFG[]=
{
	#define AddressAndType(x) &x, TypeId(x)
	#define Default(x) reinterpret_cast<const void*>(x)

	{FSSF_PRIVATE,       NKeyCmdline, L"AutoComplete", AddressAndType(Opt.CmdLine.AutoComplete), Default(1)},
	{FSSF_PRIVATE,       NKeyCmdline, L"EditBlock", AddressAndType(Opt.CmdLine.EditBlock), Default(0)},
	{FSSF_PRIVATE,       NKeyCmdline, L"DelRemovesBlocks", AddressAndType(Opt.CmdLine.DelRemovesBlocks), Default(1)},
	{FSSF_PRIVATE,       NKeyCmdline, L"PromptFormat", AddressAndType(Opt.CmdLine.strPromptFormat), Default(L"$p$g")},
	{FSSF_PRIVATE,       NKeyCmdline, L"UsePromptFormat", AddressAndType(Opt.CmdLine.UsePromptFormat), Default(0)},

	{FSSF_PRIVATE,       NKeyCodePages,L"CPMenuMode", AddressAndType(Opt.CPMenuMode), Default(0)},
	{FSSF_PRIVATE,       NKeyCodePages,L"NoAutoDetectCP", AddressAndType(Opt.strNoAutoDetectCP), Default(L"")},

	{FSSF_PRIVATE,       NKeyConfirmations,L"AllowReedit", AddressAndType(Opt.Confirm.AllowReedit), Default(1)},
	{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Copy", AddressAndType(Opt.Confirm.Copy), Default(1)},
	{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Delete", AddressAndType(Opt.Confirm.Delete), Default(1)},
	{FSSF_CONFIRMATIONS, NKeyConfirmations,L"DeleteFolder", AddressAndType(Opt.Confirm.DeleteFolder), Default(1)},
	{FSSF_PRIVATE,       NKeyConfirmations,L"DetachVHD", AddressAndType(Opt.Confirm.DetachVHD), Default(1)},
	{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Drag", AddressAndType(Opt.Confirm.Drag), Default(1)},
	{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Esc", AddressAndType(Opt.Confirm.Esc), Default(1)},
	{FSSF_PRIVATE,       NKeyConfirmations,L"EscTwiceToInterrupt", AddressAndType(Opt.Confirm.EscTwiceToInterrupt), Default(0)},
	{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Exit", AddressAndType(Opt.Confirm.Exit), Default(1)},
	{FSSF_CONFIRMATIONS, NKeyConfirmations,L"HistoryClear", AddressAndType(Opt.Confirm.HistoryClear), Default(1)},
	{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Move", AddressAndType(Opt.Confirm.Move), Default(1)},
	{FSSF_CONFIRMATIONS, NKeyConfirmations,L"RemoveConnection", AddressAndType(Opt.Confirm.RemoveConnection), Default(1)},
	{FSSF_PRIVATE,       NKeyConfirmations,L"RemoveHotPlug", AddressAndType(Opt.Confirm.RemoveHotPlug), Default(1)},
	{FSSF_PRIVATE,       NKeyConfirmations,L"RemoveSUBST", AddressAndType(Opt.Confirm.RemoveSUBST), Default(1)},
	{FSSF_CONFIRMATIONS, NKeyConfirmations,L"RO", AddressAndType(Opt.Confirm.RO), Default(1)},

	{FSSF_PRIVATE,       NKeyDescriptions,L"AnsiByDefault", AddressAndType(Opt.Diz.AnsiByDefault), Default(0)},
	{FSSF_PRIVATE,       NKeyDescriptions,L"ListNames", AddressAndType(Opt.Diz.strListNames), Default(L"Descript.ion,Files.bbs")},
	{FSSF_PRIVATE,       NKeyDescriptions,L"ROUpdate", AddressAndType(Opt.Diz.ROUpdate), Default(0)},
	{FSSF_PRIVATE,       NKeyDescriptions,L"SaveInUTF", AddressAndType(Opt.Diz.SaveInUTF), Default(0)},
	{FSSF_PRIVATE,       NKeyDescriptions,L"SetHidden", AddressAndType(Opt.Diz.SetHidden), Default(1)},
	{FSSF_PRIVATE,       NKeyDescriptions,L"StartPos", AddressAndType(Opt.Diz.StartPos), Default(0)},
	{FSSF_PRIVATE,       NKeyDescriptions,L"UpdateMode", AddressAndType(Opt.Diz.UpdateMode), Default(DIZ_UPDATE_IF_DISPLAYED)},

	{FSSF_PRIVATE,       NKeyDialog,L"AutoComplete", AddressAndType(Opt.Dialogs.AutoComplete), Default(1)},
	{FSSF_PRIVATE,       NKeyDialog,L"CBoxMaxHeight", AddressAndType(Opt.Dialogs.CBoxMaxHeight), Default(8)},
	{FSSF_DIALOG,        NKeyDialog,L"EditBlock", AddressAndType(Opt.Dialogs.EditBlock), Default(0)},
	{FSSF_PRIVATE,       NKeyDialog,L"EditHistory", AddressAndType(Opt.Dialogs.EditHistory), Default(1)},
	{FSSF_PRIVATE,       NKeyDialog,L"EditLine", AddressAndType(Opt.Dialogs.EditLine), Default(0)},
	{FSSF_DIALOG,        NKeyDialog,L"DelRemovesBlocks", AddressAndType(Opt.Dialogs.DelRemovesBlocks), Default(1)},
	{FSSF_DIALOG,        NKeyDialog,L"EULBsClear", AddressAndType(Opt.Dialogs.EULBsClear), Default(0)},
	{FSSF_PRIVATE,       NKeyDialog,L"MouseButton", AddressAndType(Opt.Dialogs.MouseButton), Default(0xFFFF)},
	{FSSF_PRIVATE,       NKeyDialog,L"SelectFromHistory", AddressAndType(Opt.Dialogs.SelectFromHistory), Default(0)},

	{FSSF_PRIVATE,       NKeyEditor,L"AllowEmptySpaceAfterEof", AddressAndType(Opt.EdOpt.AllowEmptySpaceAfterEof),0,},
	{FSSF_PRIVATE,       NKeyEditor,L"AnsiCodePageAsDefault", AddressAndType(Opt.EdOpt.AnsiCodePageAsDefault), Default(1)},
	{FSSF_PRIVATE,       NKeyEditor,L"AnsiCodePageForNewFile", AddressAndType(Opt.EdOpt.AnsiCodePageForNewFile), Default(1)},
	{FSSF_PRIVATE,       NKeyEditor,L"AutoDetectCodePage", AddressAndType(Opt.EdOpt.AutoDetectCodePage), Default(1)},
	{FSSF_PRIVATE,       NKeyEditor,L"AutoIndent", AddressAndType(Opt.EdOpt.AutoIndent), Default(0)},
	{FSSF_PRIVATE,       NKeyEditor,L"BSLikeDel", AddressAndType(Opt.EdOpt.BSLikeDel), Default(1)},
	{FSSF_PRIVATE,       NKeyEditor,L"CharCodeBase", AddressAndType(Opt.EdOpt.CharCodeBase), Default(1)},
	{FSSF_PRIVATE,       NKeyEditor,L"DelRemovesBlocks", AddressAndType(Opt.EdOpt.DelRemovesBlocks), Default(1)},
	{FSSF_PRIVATE,       NKeyEditor,L"EditOpenedForWrite", AddressAndType(Opt.EdOpt.EditOpenedForWrite), Default(1)},
	{FSSF_PRIVATE,       NKeyEditor,L"EditorCursorBeyondEOL", AddressAndType(Opt.EdOpt.CursorBeyondEOL), Default(1)},
	{FSSF_PRIVATE,       NKeyEditor,L"EditorF7Rules", AddressAndType(Opt.EdOpt.F7Rules), Default(0)},
	{FSSF_PRIVATE,       NKeyEditor,L"EditorUndoSize", AddressAndType(Opt.EdOpt.UndoSize), Default(0)},
	{FSSF_PRIVATE,       NKeyEditor,L"ExpandTabs", AddressAndType(Opt.EdOpt.ExpandTabs), Default(0)},
	{FSSF_PRIVATE,       NKeyEditor,L"ExternalEditorName", AddressAndType(Opt.strExternalEditor), Default(L"")},
	{FSSF_PRIVATE,       NKeyEditor,L"FileSizeLimit", AddressAndType(Opt.EdOpt.FileSizeLimitLo), Default(0)},
	{FSSF_PRIVATE,       NKeyEditor,L"FileSizeLimitHi", AddressAndType(Opt.EdOpt.FileSizeLimitHi), Default(0)},
	{FSSF_PRIVATE,       NKeyEditor,L"PersistentBlocks", AddressAndType(Opt.EdOpt.PersistentBlocks), Default(0)},
	{FSSF_PRIVATE,       NKeyEditor,L"ReadOnlyLock", AddressAndType(Opt.EdOpt.ReadOnlyLock), Default(0)},
	{FSSF_PRIVATE,       NKeyEditor,L"SaveEditorPos", AddressAndType(Opt.EdOpt.SavePos), Default(1)},
	{FSSF_PRIVATE,       NKeyEditor,L"SaveEditorShortPos", AddressAndType(Opt.EdOpt.SaveShortPos), Default(1)},
	{FSSF_PRIVATE,       NKeyEditor,L"SearchPickUpWord", AddressAndType(Opt.EdOpt.SearchPickUpWord), Default(0)},
	{FSSF_PRIVATE,       NKeyEditor,L"SearchRegexp", AddressAndType(Opt.EdOpt.SearchRegexp), Default(0)},
	{FSSF_PRIVATE,       NKeyEditor,L"SearchSelFound", AddressAndType(Opt.EdOpt.SearchSelFound), Default(0)},
	{FSSF_PRIVATE,       NKeyEditor,L"ShowKeyBar", AddressAndType(Opt.EdOpt.ShowKeyBar), Default(1)},
	{FSSF_PRIVATE,       NKeyEditor,L"ShowScrollBar", AddressAndType(Opt.EdOpt.ShowScrollBar), Default(0)},
	{FSSF_PRIVATE,       NKeyEditor,L"ShowTitleBar", AddressAndType(Opt.EdOpt.ShowTitleBar), Default(1)},
	{FSSF_PRIVATE,       NKeyEditor,L"ShowWhiteSpace", AddressAndType(Opt.EdOpt.ShowWhiteSpace), Default(0)},
	{FSSF_PRIVATE,       NKeyEditor,L"TabSize", AddressAndType(Opt.EdOpt.TabSize), Default(DefaultTabSize)},
	{FSSF_PRIVATE,       NKeyEditor,L"UseExternalEditor", AddressAndType(Opt.EdOpt.UseExternalEditor), Default(0)},
	{FSSF_EDITOR,        NKeyEditor,L"WordDiv", AddressAndType(Opt.strWordDiv), Default(WordDiv0)},

	{FSSF_PRIVATE,       NKeyHelp,L"ActivateURL", AddressAndType(Opt.HelpURLRules), Default(1)},

	{FSSF_PRIVATE,       NKeyCommandHistory, NParamHistoryCount, AddressAndType(Opt.HistoryCount), Default(1000)},
	{FSSF_PRIVATE,       NKeyCommandHistory, NParamHistoryLifetime, AddressAndType(Opt.HistoryLifetime), Default(90)},
	{FSSF_PRIVATE,       NKeyDialogHistory, NParamHistoryCount, AddressAndType(Opt.DialogsHistoryCount), Default(1000)},
	{FSSF_PRIVATE,       NKeyDialogHistory, NParamHistoryLifetime, AddressAndType(Opt.DialogsHistoryLifetime), Default(90)},
	{FSSF_PRIVATE,       NKeyFolderHistory, NParamHistoryCount, AddressAndType(Opt.FoldersHistoryCount), Default(1000)},
	{FSSF_PRIVATE,       NKeyFolderHistory, NParamHistoryLifetime, AddressAndType(Opt.FoldersHistoryLifetime), Default(90)},
	{FSSF_PRIVATE,       NKeyViewEditHistory, NParamHistoryCount, AddressAndType(Opt.ViewHistoryCount), Default(1000)},
	{FSSF_PRIVATE,       NKeyViewEditHistory, NParamHistoryLifetime, AddressAndType(Opt.ViewHistoryLifetime), Default(90)},

	{FSSF_PRIVATE,       NKeyInterface,L"DelShowTotal", AddressAndType(Opt.DelOpt.DelShowTotal), Default(0)},

	{FSSF_PRIVATE,       NKeyInterface, L"AltF9", AddressAndType(Opt.AltF9), Default(1)},
	{FSSF_PRIVATE,       NKeyInterface, L"ClearType", AddressAndType(Opt.ClearType), Default(1)},
	{FSSF_PRIVATE,       NKeyInterface, L"CopyShowTotal", AddressAndType(Opt.CMOpt.CopyShowTotal), Default(1)},
	{FSSF_PRIVATE,       NKeyInterface, L"CtrlPgUp", AddressAndType(Opt.PgUpChangeDisk), Default(1)},
	{FSSF_PRIVATE,       NKeyInterface, L"CursorSize1", AddressAndType(Opt.CursorSize[0]), Default(15)},
	{FSSF_PRIVATE,       NKeyInterface, L"CursorSize2", AddressAndType(Opt.CursorSize[1]), Default(10)},
	{FSSF_PRIVATE,       NKeyInterface, L"CursorSize3", AddressAndType(Opt.CursorSize[2]), Default(99)},
	{FSSF_PRIVATE,       NKeyInterface, L"CursorSize4", AddressAndType(Opt.CursorSize[3]), Default(99)},
	{FSSF_PRIVATE,       NKeyInterface, L"EditorTitleFormat", AddressAndType(Opt.strEditorTitleFormat), Default(L"%Lng %File")},
	{FSSF_PRIVATE,       NKeyInterface, L"FormatNumberSeparators", AddressAndType(Opt.FormatNumberSeparators), Default(0)},
	{FSSF_PRIVATE,       NKeyInterface, L"Mouse", AddressAndType(Opt.Mouse), Default(1)},
	{FSSF_PRIVATE,       NKeyInterface, L"SetIcon", AddressAndType(Opt.SetIcon), Default(0)},
	{FSSF_PRIVATE,       NKeyInterface, L"SetAdminIcon", AddressAndType(Opt.SetAdminIcon), Default(1)},
	{FSSF_PRIVATE,       NKeyInterface, L"ShiftsKeyRules", AddressAndType(Opt.ShiftsKeyRules), Default(1)},
	{FSSF_PRIVATE,       NKeyInterface, L"ShowDotsInRoot", AddressAndType(Opt.ShowDotsInRoot), Default(0)},
	{FSSF_INTERFACE,     NKeyInterface, L"ShowMenuBar", AddressAndType(Opt.ShowMenuBar), Default(0)},
	{FSSF_PRIVATE,       NKeyInterface, L"ShowTimeoutDACLFiles", AddressAndType(Opt.ShowTimeoutDACLFiles), Default(50)},
	{FSSF_PRIVATE,       NKeyInterface, L"ShowTimeoutDelFiles", AddressAndType(Opt.ShowTimeoutDelFiles), Default(50)},
	{FSSF_PRIVATE,       NKeyInterface, L"TitleAddons", AddressAndType(Opt.strTitleAddons), Default(L"%Ver.%Build %Platform %Admin")},
	{FSSF_PRIVATE,       NKeyInterface, L"UseVk_oem_x", AddressAndType(Opt.UseVk_oem_x), Default(1)},
	{FSSF_PRIVATE,       NKeyInterface, L"ViewerTitleFormat", AddressAndType(Opt.strViewerTitleFormat), Default(L"%Lng %File")},

	{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"Append", AddressAndType(Opt.AutoComplete.AppendCompletion), Default(0)},
	{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"ModalList", AddressAndType(Opt.AutoComplete.ModalList), Default(0)},
	{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"ShowList", AddressAndType(Opt.AutoComplete.ShowList), Default(1)},
	{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"UseFilesystem", AddressAndType(Opt.AutoComplete.UseFilesystem), Default(1)},
	{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"UseHistory", AddressAndType(Opt.AutoComplete.UseHistory), Default(1)},
	{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"UsePath", AddressAndType(Opt.AutoComplete.UsePath), Default(1)},

	{FSSF_PRIVATE,       NKeyLanguage, L"Main", AddressAndType(Opt.strLanguage), Default(DefaultLanguage)},
	{FSSF_PRIVATE,       NKeyLanguage, L"Help", AddressAndType(Opt.strHelpLanguage), Default(DefaultLanguage)},

	{FSSF_PRIVATE,       NKeyLayout,L"FullscreenHelp", AddressAndType(Opt.FullScreenHelp), Default(0)},
	{FSSF_PRIVATE,       NKeyLayout,L"LeftHeightDecrement", AddressAndType(Opt.LeftHeightDecrement), Default(0)},
	{FSSF_PRIVATE,       NKeyLayout,L"RightHeightDecrement", AddressAndType(Opt.RightHeightDecrement), Default(0)},
	{FSSF_PRIVATE,       NKeyLayout,L"WidthDecrement", AddressAndType(Opt.WidthDecrement), Default(0)},

	{FSSF_PRIVATE,       NKeyKeyMacros,L"CONVFMT", AddressAndType(Opt.Macro.strMacroCONVFMT), Default(L"%.6g")},
	{FSSF_PRIVATE,       NKeyKeyMacros,L"DateFormat", AddressAndType(Opt.Macro.strDateFormat), Default(L"%a %b %d %H:%M:%S %Z %Y")},
	{FSSF_PRIVATE,       NKeyKeyMacros,L"MacroReuseRules", AddressAndType(Opt.Macro.MacroReuseRules), Default(0)},

	{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordCtrlDot", AddressAndType(Opt.Macro.strKeyMacroCtrlDot), Default(L"Ctrl.")},
	{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordRCtrlDot", AddressAndType(Opt.Macro.strKeyMacroRCtrlDot), Default(L"RCtrl.")},
	{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordCtrlShiftDot", AddressAndType(Opt.Macro.strKeyMacroCtrlShiftDot), Default(L"CtrlShift.")},
	{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordRCtrlShiftDot", AddressAndType(Opt.Macro.strKeyMacroRCtrlShiftDot), Default(L"RCtrlShift.")},

	{FSSF_PRIVATE,       NKeyPanel,L"AutoUpdateLimit", AddressAndType(Opt.AutoUpdateLimit), Default(0)},
	{FSSF_PRIVATE,       NKeyPanel,L"CtrlAltShiftRule", AddressAndType(Opt.PanelCtrlAltShiftRule), Default(0)},
	{FSSF_PRIVATE,       NKeyPanel,L"CtrlFRule", AddressAndType(Opt.PanelCtrlFRule), Default(0)},
	{FSSF_PRIVATE,       NKeyPanel,L"Highlight", AddressAndType(Opt.Highlight), Default(1)},
	{FSSF_PRIVATE,       NKeyPanel,L"ReverseSort", AddressAndType(Opt.ReverseSort), Default(1)},
	{FSSF_PRIVATE,       NKeyPanel,L"RememberLogicalDrives", AddressAndType(Opt.RememberLogicalDrives), Default(0)},
	{FSSF_PRIVATE,       NKeyPanel,L"RightClickRule", AddressAndType(Opt.PanelRightClickRule), Default(2)},
	{FSSF_PRIVATE,       NKeyPanel,L"SelectFolders", AddressAndType(Opt.SelectFolders), Default(0)},
	{FSSF_PRIVATE,       NKeyPanel,L"ShellRightLeftArrowsRule", AddressAndType(Opt.ShellRightLeftArrowsRule), Default(0)},
	{FSSF_PANEL,         NKeyPanel,L"ShowHidden", AddressAndType(Opt.ShowHidden), Default(1)},
	{FSSF_PRIVATE,       NKeyPanel,L"SortFolderExt", AddressAndType(Opt.SortFolderExt), Default(0)},

	{FSSF_PRIVATE,       NKeyPanelInfo,L"InfoComputerNameFormat", AddressAndType(Opt.InfoPanel.ComputerNameFormat), Default(ComputerNamePhysicalNetBIOS)},
	{FSSF_PRIVATE,       NKeyPanelInfo,L"InfoUserNameFormat", AddressAndType(Opt.InfoPanel.UserNameFormat), Default(NameUserPrincipal)},
	{FSSF_PRIVATE,       NKeyPanelInfo,L"ShowCDInfo", AddressAndType(Opt.InfoPanel.ShowCDInfo), Default(1)},
	{FSSF_PRIVATE,       NKeyPanelInfo,L"ShowPowerStatus", AddressAndType(Opt.InfoPanel.ShowPowerStatus), Default(0)},

	{FSSF_PRIVATE,       NKeyPanelLayout,L"ColoredGlobalColumnSeparator", AddressAndType(Opt.HighlightColumnSeparator), Default(1)},
	{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"ColumnTitles", AddressAndType(Opt.ShowColumnTitles), Default(1)},
	{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"DetailedJunction", AddressAndType(Opt.PanelDetailedJunction), Default(0)},
	{FSSF_PRIVATE,       NKeyPanelLayout,L"DoubleGlobalColumnSeparator", AddressAndType(Opt.DoubleGlobalColumnSeparator), Default(0)},
	{FSSF_PRIVATE,       NKeyPanelLayout,L"FreeInfo", AddressAndType(Opt.ShowPanelFree), Default(0)},
	{FSSF_PRIVATE,       NKeyPanelLayout,L"ScreensNumber", AddressAndType(Opt.ShowScreensNumber), Default(1)},
	{FSSF_PRIVATE,       NKeyPanelLayout,L"Scrollbar", AddressAndType(Opt.ShowPanelScrollbar), Default(0)},
	{FSSF_PRIVATE,       NKeyPanelLayout,L"ScrollbarMenu", AddressAndType(Opt.ShowMenuScrollbar), Default(1)},
	{FSSF_PRIVATE,       NKeyPanelLayout,L"ShowUnknownReparsePoint", AddressAndType(Opt.ShowUnknownReparsePoint), Default(0)},
	{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"SortMode", AddressAndType(Opt.ShowSortMode), Default(1)},
	{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"StatusLine", AddressAndType(Opt.ShowPanelStatus), Default(1)},
	{FSSF_PRIVATE,       NKeyPanelLayout,L"TotalInfo", AddressAndType(Opt.ShowPanelTotals), Default(1)},

	{FSSF_PRIVATE,       NKeyPanelLeft,L"CaseSensitiveSort", AddressAndType(Opt.LeftPanel.CaseSensitiveSort), Default(0)},
	{FSSF_PRIVATE,       NKeyPanelLeft,L"CurFile", AddressAndType(Opt.strLeftCurFile), Default(L"")},
	{FSSF_PRIVATE,       NKeyPanelLeft,L"DirectoriesFirst", AddressAndType(Opt.LeftPanel.DirectoriesFirst), Default(1)},
	{FSSF_PRIVATE,       NKeyPanelLeft,L"Focus", AddressAndType(Opt.LeftPanel.Focus), Default(1)},
	{FSSF_PRIVATE,       NKeyPanelLeft,L"Folder", AddressAndType(Opt.strLeftFolder), Default(L"")},
	{FSSF_PRIVATE,       NKeyPanelLeft,L"NumericSort", AddressAndType(Opt.LeftPanel.NumericSort), Default(0)},
	{FSSF_PRIVATE,       NKeyPanelLeft,L"SelectedFirst", AddressAndType(Opt.LeftSelectedFirst), Default(0)},
	{FSSF_PRIVATE,       NKeyPanelLeft,L"ShortNames", AddressAndType(Opt.LeftPanel.ShowShortNames), Default(0)},
	{FSSF_PRIVATE,       NKeyPanelLeft,L"SortGroups", AddressAndType(Opt.LeftPanel.SortGroups), Default(0)},
	{FSSF_PRIVATE,       NKeyPanelLeft,L"SortMode", AddressAndType(Opt.LeftPanel.SortMode), Default(1)},
	{FSSF_PRIVATE,       NKeyPanelLeft,L"SortOrder", AddressAndType(Opt.LeftPanel.SortOrder), Default(1)},
	{FSSF_PRIVATE,       NKeyPanelLeft,L"Type", AddressAndType(Opt.LeftPanel.Type), Default(0)},
	{FSSF_PRIVATE,       NKeyPanelLeft,L"ViewMode", AddressAndType(Opt.LeftPanel.ViewMode), Default(2)},
	{FSSF_PRIVATE,       NKeyPanelLeft,L"Visible", AddressAndType(Opt.LeftPanel.Visible), Default(1)},

	{FSSF_PRIVATE,       NKeyPanelRight,L"CaseSensitiveSort", AddressAndType(Opt.RightPanel.CaseSensitiveSort), Default(0)},
	{FSSF_PRIVATE,       NKeyPanelRight,L"CurFile", AddressAndType(Opt.strRightCurFile), Default(L"")},
	{FSSF_PRIVATE,       NKeyPanelRight,L"DirectoriesFirst", AddressAndType(Opt.RightPanel.DirectoriesFirst), Default(1)},
	{FSSF_PRIVATE,       NKeyPanelRight,L"Focus", AddressAndType(Opt.RightPanel.Focus), Default(0)},
	{FSSF_PRIVATE,       NKeyPanelRight,L"Folder", AddressAndType(Opt.strRightFolder), Default(L"")},
	{FSSF_PRIVATE,       NKeyPanelRight,L"NumericSort", AddressAndType(Opt.RightPanel.NumericSort), Default(0)},
	{FSSF_PRIVATE,       NKeyPanelRight,L"SelectedFirst", AddressAndType(Opt.RightSelectedFirst), Default(0)},
	{FSSF_PRIVATE,       NKeyPanelRight,L"ShortNames", AddressAndType(Opt.RightPanel.ShowShortNames), Default(0)},
	{FSSF_PRIVATE,       NKeyPanelRight,L"SortGroups", AddressAndType(Opt.RightPanel.SortGroups), Default(0)},
	{FSSF_PRIVATE,       NKeyPanelRight,L"SortMode", AddressAndType(Opt.RightPanel.SortMode), Default(1)},
	{FSSF_PRIVATE,       NKeyPanelRight,L"SortOrder", AddressAndType(Opt.RightPanel.SortOrder), Default(1)},
	{FSSF_PRIVATE,       NKeyPanelRight,L"Type", AddressAndType(Opt.RightPanel.Type), Default(0)},
	{FSSF_PRIVATE,       NKeyPanelRight,L"ViewMode", AddressAndType(Opt.RightPanel.ViewMode), Default(2)},
	{FSSF_PRIVATE,       NKeyPanelRight,L"Visible", AddressAndType(Opt.RightPanel.Visible), Default(1)},

	{FSSF_PRIVATE,       NKeyPanelTree,L"AutoChangeFolder", AddressAndType(Opt.Tree.AutoChangeFolder), Default(0)},
	{FSSF_PRIVATE,       NKeyPanelTree,L"MinTreeCount", AddressAndType(Opt.Tree.MinTreeCount), Default(4)},
	{FSSF_PRIVATE,       NKeyPanelTree,L"TreeFileAttr", AddressAndType(Opt.Tree.TreeFileAttr), Default(FILE_ATTRIBUTE_HIDDEN)},
#if defined(TREEFILE_PROJECT)
	{FSSF_PRIVATE,       NKeyPanelTree,L"CDDisk", AddressAndType(Opt.Tree.CDDisk), Default(2)},
	{FSSF_PRIVATE,       NKeyPanelTree,L"CDDiskTemplate,0", AddressAndType(Opt.Tree.strCDDisk), Default(constCDDiskTemplate)},
	{FSSF_PRIVATE,       NKeyPanelTree,L"ExceptPath", AddressAndType(Opt.Tree.strExceptPath), Default(L"")},
	{FSSF_PRIVATE,       NKeyPanelTree,L"LocalDisk", AddressAndType(Opt.Tree.LocalDisk), Default(2)},
	{FSSF_PRIVATE,       NKeyPanelTree,L"LocalDiskTemplate", AddressAndType(Opt.Tree.strLocalDisk), Default(constLocalDiskTemplate)},
	{FSSF_PRIVATE,       NKeyPanelTree,L"NetDisk", AddressAndType(Opt.Tree.NetDisk), Default(2)},
	{FSSF_PRIVATE,       NKeyPanelTree,L"NetPath", AddressAndType(Opt.Tree.NetPath), Default(2)},
	{FSSF_PRIVATE,       NKeyPanelTree,L"NetDiskTemplate", AddressAndType(Opt.Tree.strNetDisk), Default(constNetDiskTemplate)},
	{FSSF_PRIVATE,       NKeyPanelTree,L"NetPathTemplate", AddressAndType(Opt.Tree.strNetPath), Default(constNetPathTemplate)},
	{FSSF_PRIVATE,       NKeyPanelTree,L"RemovableDisk", AddressAndType(Opt.Tree.RemovableDisk), Default(2)},
	{FSSF_PRIVATE,       NKeyPanelTree,L"RemovableDiskTemplate,", AddressAndType(Opt.Tree.strRemovableDisk), Default(constRemovableDiskTemplate)},
	{FSSF_PRIVATE,       NKeyPanelTree,L"SaveLocalPath", AddressAndType(Opt.Tree.strSaveLocalPath), Default(L"")},
	{FSSF_PRIVATE,       NKeyPanelTree,L"SaveNetPath", AddressAndType(Opt.Tree.strSaveNetPath), Default(L"")},
#endif
	{FSSF_PRIVATE,       NKeyPluginConfirmations, L"EvenIfOnlyOnePlugin", AddressAndType(Opt.PluginConfirm.EvenIfOnlyOnePlugin), Default(0)},
	{FSSF_PRIVATE,       NKeyPluginConfirmations, L"OpenFilePlugin", AddressAndType(Opt.PluginConfirm.OpenFilePlugin), Default(0)},
	{FSSF_PRIVATE,       NKeyPluginConfirmations, L"Prefix", AddressAndType(Opt.PluginConfirm.Prefix), Default(0)},
	{FSSF_PRIVATE,       NKeyPluginConfirmations, L"SetFindList", AddressAndType(Opt.PluginConfirm.SetFindList), Default(0)},
	{FSSF_PRIVATE,       NKeyPluginConfirmations, L"StandardAssociation", AddressAndType(Opt.PluginConfirm.StandardAssociation), Default(0)},

	{FSSF_PRIVATE,       NKeyPolicies,L"DisabledOptions", AddressAndType(Opt.Policies.DisabledOptions), Default(0)},
	{FSSF_PRIVATE,       NKeyPolicies,L"ShowHiddenDrives", AddressAndType(Opt.Policies.ShowHiddenDrives), Default(1)},

	{FSSF_PRIVATE,       NKeyScreen, L"Clock", AddressAndType(Opt.Clock), Default(1)},
	{FSSF_PRIVATE,       NKeyScreen, L"DeltaX", AddressAndType(Opt.ScrSize.DeltaX), Default(0)},
	{FSSF_PRIVATE,       NKeyScreen, L"DeltaY", AddressAndType(Opt.ScrSize.DeltaY), Default(0)},
	{FSSF_SCREEN,        NKeyScreen, L"KeyBar", AddressAndType(Opt.ShowKeyBar), Default(1)},
	{FSSF_PRIVATE,       NKeyScreen, L"ScreenSaver", AddressAndType(Opt.ScreenSaver), Default(0)},
	{FSSF_PRIVATE,       NKeyScreen, L"ScreenSaverTime", AddressAndType(Opt.ScreenSaverTime), Default(5)},
	{FSSF_PRIVATE,       NKeyScreen, L"ViewerEditorClock", AddressAndType(Opt.ViewerEditorClock), Default(1)},

	{FSSF_PRIVATE,       NKeySystem,L"AllCtrlAltShiftRule", AddressAndType(Opt.AllCtrlAltShiftRule), Default(0x0000FFFF)},
	{FSSF_PRIVATE,       NKeySystem,L"AutoSaveSetup", AddressAndType(Opt.AutoSaveSetup), Default(0)},
	{FSSF_PRIVATE,       NKeySystem,L"AutoUpdateRemoteDrive", AddressAndType(Opt.AutoUpdateRemoteDrive), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"BoxSymbols", AddressAndType(Opt.strBoxSymbols), Default(_BoxSymbols)},
	{FSSF_PRIVATE,       NKeySystem,L"CASRule", AddressAndType(Opt.CASRule), Default(0xFFFFFFFFU)},
	{FSSF_PRIVATE,       NKeySystem,L"CloseCDGate", AddressAndType(Opt.CloseCDGate), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"CmdHistoryRule", AddressAndType(Opt.CmdHistoryRule), Default(0)},
	{FSSF_PRIVATE,       NKeySystem,L"CollectFiles", AddressAndType(Opt.FindOpt.CollectFiles), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"ConsoleDetachKey", AddressAndType(Opt.ConsoleDetachKey), Default(L"CtrlShiftTab")},
	{FSSF_PRIVATE,       NKeySystem,L"CopyBufferSize", AddressAndType(Opt.CMOpt.BufferSize), Default(0)},
	{FSSF_SYSTEM,        NKeySystem,L"CopyOpened", AddressAndType(Opt.CMOpt.CopyOpened), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"CopyTimeRule",  AddressAndType(Opt.CMOpt.CopyTimeRule), Default(3)},
	{FSSF_PRIVATE,       NKeySystem,L"CopySecurityOptions", AddressAndType(Opt.CMOpt.CopySecurityOptions), Default(0)},
	{FSSF_PRIVATE,       NKeySystem,L"CreateUppercaseFolders", AddressAndType(Opt.CreateUppercaseFolders), Default(0)},
	{FSSF_SYSTEM,        NKeySystem,L"DeleteToRecycleBin", AddressAndType(Opt.DeleteToRecycleBin), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"DeleteToRecycleBinKillLink", AddressAndType(Opt.DeleteToRecycleBinKillLink), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"DelThreadPriority", AddressAndType(Opt.DelThreadPriority), THREAD_PRIORITY_NORMAL},
	{FSSF_PRIVATE,       NKeySystem,L"DriveDisconnectMode", AddressAndType(Opt.ChangeDriveDisconnectMode), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"DriveMenuMode", AddressAndType(Opt.ChangeDriveMode), Default(DRIVE_SHOW_TYPE|DRIVE_SHOW_PLUGINS|DRIVE_SHOW_SIZE_FLOAT|DRIVE_SHOW_CDROM)},
	{FSSF_PRIVATE,       NKeySystem,L"ElevationMode", AddressAndType(Opt.StoredElevationMode), Default(0x0FFFFFFFU)},
	{FSSF_PRIVATE,       NKeySystem,L"ExceptRules", AddressAndType(Opt.StoredExceptRules), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"ExcludeCmdHistory", AddressAndType(Opt.ExcludeCmdHistory), Default(0)},
	{FSSF_PRIVATE,       NKeySystem,L"FileSearchMode", AddressAndType(Opt.FindOpt.FileSearchMode), Default(FINDAREA_FROM_CURRENT)},
	{FSSF_PRIVATE,       NKeySystem,L"FindAlternateStreams", AddressAndType(Opt.FindOpt.FindAlternateStreams),0,},
	{FSSF_PRIVATE,       NKeySystem,L"FindCodePage", AddressAndType(Opt.FindCodePage), Default(CP_DEFAULT)},
	{FSSF_PRIVATE,       NKeySystem,L"FindFolders", AddressAndType(Opt.FindOpt.FindFolders), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"FindSymLinks", AddressAndType(Opt.FindOpt.FindSymLinks), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"FlagPosixSemantics", AddressAndType(Opt.FlagPosixSemantics), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"FolderInfo", AddressAndType(Opt.InfoPanel.strFolderInfoFiles), Default(L"DirInfo,File_Id.diz,Descript.ion,ReadMe.*,Read.Me")},
	{FSSF_PRIVATE,       NKeySystem,L"MaxPositionCache", AddressAndType(Opt.MaxPositionCache), Default(512)/*MAX_POSITIONS*/}, //BUGBUG
	{FSSF_PRIVATE,       NKeySystem,L"MsWheelDelta", AddressAndType(Opt.MsWheelDelta), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"MsWheelDeltaEdit", AddressAndType(Opt.MsWheelDeltaEdit), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"MsWheelDeltaHelp", AddressAndType(Opt.MsWheelDeltaHelp), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"MsWheelDeltaView", AddressAndType(Opt.MsWheelDeltaView), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"MsHWheelDelta", AddressAndType(Opt.MsHWheelDelta), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"MsHWheelDeltaEdit", AddressAndType(Opt.MsHWheelDeltaEdit), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"MsHWheelDeltaView", AddressAndType(Opt.MsHWheelDeltaView), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"MultiCopy", AddressAndType(Opt.CMOpt.MultiCopy), Default(0)},
	{FSSF_PRIVATE,       NKeySystem,L"MultiMakeDir", AddressAndType(Opt.MultiMakeDir), Default(0)},
#ifndef NO_WRAPPER
	{FSSF_PRIVATE,       NKeySystem,L"OEMPluginsSupport",  AddressAndType(Opt.LoadPlug.OEMPluginsSupport), Default(1)},
#endif // NO_WRAPPER
	{FSSF_SYSTEM,        NKeySystem,L"PluginMaxReadData", AddressAndType(Opt.PluginMaxReadData), Default(0x20000)},
	{FSSF_PRIVATE,       NKeySystem,L"QuotedName", AddressAndType(Opt.QuotedName), Default(0xFFFFFFFFU)},
	{FSSF_PRIVATE,       NKeySystem,L"QuotedSymbols", AddressAndType(Opt.strQuotedSymbols), Default(L" &()[]{}^=;!'+,`\xA0")},
	{FSSF_PRIVATE,       NKeySystem,L"SaveHistory", AddressAndType(Opt.SaveHistory), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"SaveFoldersHistory", AddressAndType(Opt.SaveFoldersHistory), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"SaveViewHistory", AddressAndType(Opt.SaveViewHistory), Default(1)},
	{FSSF_SYSTEM,        NKeySystem,L"ScanJunction", AddressAndType(Opt.ScanJunction), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"ScanSymlinks",  AddressAndType(Opt.LoadPlug.ScanSymlinks), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"SearchInFirstSize", AddressAndType(Opt.FindOpt.strSearchInFirstSize), Default(L"")},
	{FSSF_PRIVATE,       NKeySystem,L"SearchOutFormat", AddressAndType(Opt.FindOpt.strSearchOutFormat), Default(L"D,S,A")},
	{FSSF_PRIVATE,       NKeySystem,L"SearchOutFormatWidth", AddressAndType(Opt.FindOpt.strSearchOutFormatWidth), Default(L"14,13,0")},
	{FSSF_PRIVATE,       NKeySystem,L"SetAttrFolderRules", AddressAndType(Opt.SetAttrFolderRules), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"ShowCheckingFile", AddressAndType(Opt.ShowCheckingFile), Default(0)},
	{FSSF_PRIVATE,       NKeySystem,L"ShowStatusInfo", AddressAndType(Opt.InfoPanel.strShowStatusInfo), Default(L"")},
	{FSSF_PRIVATE,       NKeySystem,L"SilentLoadPlugin",  AddressAndType(Opt.LoadPlug.SilentLoadPlugin), Default(0)},
	{FSSF_PRIVATE,       NKeySystem,L"SmartFolderMonitor",  AddressAndType(Opt.SmartFolderMonitor), Default(0)},
	{FSSF_PRIVATE,       NKeySystem,L"SubstNameRule", AddressAndType(Opt.SubstNameRule), Default(2)},
	{FSSF_PRIVATE,       NKeySystem,L"SubstPluginPrefix", AddressAndType(Opt.SubstPluginPrefix), Default(0)},
	{FSSF_PRIVATE,       NKeySystem,L"UpdateEnvironment", AddressAndType(Opt.UpdateEnvironment),0,},
	{FSSF_PRIVATE,       NKeySystem,L"UseFilterInSearch", AddressAndType(Opt.FindOpt.UseFilter),0,},
	{FSSF_PRIVATE,       NKeySystem,L"UseRegisteredTypes", AddressAndType(Opt.UseRegisteredTypes), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"UseSystemCopy", AddressAndType(Opt.CMOpt.UseSystemCopy), Default(1)},
	{FSSF_PRIVATE,       NKeySystem,L"WindowMode", AddressAndType(Opt.StoredWindowMode), Default(0)},
	{FSSF_PRIVATE,       NKeySystem,L"WipeSymbol", AddressAndType(Opt.WipeSymbol), Default(0)},

	{FSSF_PRIVATE,       NKeySystemKnownIDs,L"EMenu", AddressAndType(Opt.KnownIDs.EmenuGuidStr), Default(L"742910F1-02ED-4542-851F-DEE37C2E13B2")},
	{FSSF_PRIVATE,       NKeySystemKnownIDs,L"Network", AddressAndType(Opt.KnownIDs.NetworkGuidStr), Default(L"773B5051-7C5F-4920-A201-68051C4176A4")},

	{FSSF_PRIVATE,       NKeySystemNowell,L"MoveRO", AddressAndType(Opt.Nowell.MoveRO), Default(1)},

	{FSSF_PRIVATE,       NKeySystemException,L"FarEventSvc", AddressAndType(Opt.strExceptEventSvc), Default(L"")},
	{FSSF_PRIVATE,       NKeySystemException,L"Used", AddressAndType(Opt.ExceptUsed), Default(0)},

	{FSSF_PRIVATE,       NKeySystemExecutor,L"~", AddressAndType(Opt.Exec.strHomeDir), Default(L"%FARHOME%")},
	{FSSF_PRIVATE,       NKeySystemExecutor,L"BatchType", AddressAndType(Opt.Exec.strExecuteBatchType), Default(constBatchExt)},
	{FSSF_PRIVATE,       NKeySystemExecutor,L"ExcludeCmds", AddressAndType(Opt.Exec.strExcludeCmds), Default(L"")},
	{FSSF_PRIVATE,       NKeySystemExecutor,L"FullTitle", AddressAndType(Opt.Exec.ExecuteFullTitle), Default(0)},
	{FSSF_PRIVATE,       NKeySystemExecutor,L"RestoreCP", AddressAndType(Opt.Exec.RestoreCPAfterExecute), Default(1)},
	{FSSF_PRIVATE,       NKeySystemExecutor,L"SilentExternal", AddressAndType(Opt.Exec.ExecuteSilentExternal), Default(0)},
	{FSSF_PRIVATE,       NKeySystemExecutor,L"UseAppPath", AddressAndType(Opt.Exec.ExecuteUseAppPath), Default(1)},
	{FSSF_PRIVATE,       NKeySystemExecutor,L"UseHomeDir", AddressAndType(Opt.Exec.UseHomeDir), Default(1)},

	{FSSF_PRIVATE,       NKeyViewer,L"AnsiCodePageAsDefault", AddressAndType(Opt.ViOpt.AnsiCodePageAsDefault), Default(1)},
	{FSSF_PRIVATE,       NKeyViewer,L"AutoDetectCodePage", AddressAndType(Opt.ViOpt.AutoDetectCodePage), Default(1)},
	{FSSF_PRIVATE,       NKeyViewer,L"ExternalViewerName", AddressAndType(Opt.strExternalViewer), Default(L"")},
	{FSSF_PRIVATE,       NKeyViewer,L"IsWrap", AddressAndType(Opt.ViOpt.ViewerIsWrap), Default(1)},
	{FSSF_PRIVATE,       NKeyViewer,L"MaxLineSize", AddressAndType(Opt.ViOpt.MaxLineSize), Default(ViewerOptions::eDefLineSize)},
	{FSSF_PRIVATE,       NKeyViewer,L"PersistentBlocks", AddressAndType(Opt.ViOpt.PersistentBlocks), Default(0)},
	{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerPos", AddressAndType(Opt.ViOpt.SavePos), Default(1)},
	{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerShortPos", AddressAndType(Opt.ViOpt.SaveShortPos), Default(1)},
	{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerCodepage", AddressAndType(Opt.ViOpt.SaveCodepage), Default(1)},
	{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerWrapMode", AddressAndType(Opt.ViOpt.SaveWrapMode), Default(0)},
	{FSSF_PRIVATE,       NKeyViewer,L"SearchEditFocus", AddressAndType(Opt.ViOpt.SearchEditFocus), Default(0)},
	{FSSF_PRIVATE,       NKeyViewer,L"SearchRegexp", AddressAndType(Opt.ViOpt.SearchRegexp), Default(0)},
	{FSSF_PRIVATE,       NKeyViewer,L"ShowArrows", AddressAndType(Opt.ViOpt.ShowArrows), Default(1)},
	{FSSF_PRIVATE,       NKeyViewer,L"ShowKeyBar", AddressAndType(Opt.ViOpt.ShowKeyBar), Default(1)},
	{FSSF_PRIVATE,       NKeyViewer,L"ShowTitleBar", AddressAndType(Opt.ViOpt.ShowTitleBar), Default(1)},
	{FSSF_PRIVATE,       NKeyViewer,L"ShowScrollbar", AddressAndType(Opt.ViOpt.ShowScrollbar), Default(0)},
	{FSSF_PRIVATE,       NKeyViewer,L"TabSize", AddressAndType(Opt.ViOpt.TabSize), Default(DefaultTabSize)},
	{FSSF_PRIVATE,       NKeyViewer,L"UseExternalViewer", AddressAndType(Opt.ViOpt.UseExternalViewer), Default(0)},
	{FSSF_PRIVATE,       NKeyViewer,L"Visible0x00", AddressAndType(Opt.ViOpt.Visible0x00), Default(0)},
	{FSSF_PRIVATE,       NKeyViewer,L"Wrap", AddressAndType(Opt.ViOpt.ViewerWrap), Default(0)},
	{FSSF_PRIVATE,       NKeyViewer,L"ZeroChar", AddressAndType(Opt.ViOpt.ZeroChar), Default(0x00B7)}, // middle dot

	{FSSF_PRIVATE,       NKeyVMenu,L"LBtnClick", AddressAndType(Opt.VMenu.LBtnClick), Default(VMENUCLICK_CANCEL)},
	{FSSF_PRIVATE,       NKeyVMenu,L"MBtnClick", AddressAndType(Opt.VMenu.MBtnClick), Default(VMENUCLICK_APPLY)},
	{FSSF_PRIVATE,       NKeyVMenu,L"RBtnClick", AddressAndType(Opt.VMenu.RBtnClick), Default(VMENUCLICK_CANCEL)},

	{FSSF_PRIVATE,       NKeyXLat,L"Flags", AddressAndType(Opt.XLat.Flags), Default(XLAT_SWITCHKEYBLAYOUT|XLAT_CONVERTALLCMDLINE), },
	{FSSF_PRIVATE,       NKeyXLat,L"Layouts", AddressAndType(Opt.XLat.strLayouts), Default(L"")},
	{FSSF_PRIVATE,       NKeyXLat,L"Rules1", AddressAndType(Opt.XLat.Rules[0]), Default(L"")},
	{FSSF_PRIVATE,       NKeyXLat,L"Rules2", AddressAndType(Opt.XLat.Rules[1]), Default(L"")},
	{FSSF_PRIVATE,       NKeyXLat,L"Rules3", AddressAndType(Opt.XLat.Rules[2]), Default(L"")},
	{FSSF_PRIVATE,       NKeyXLat,L"Table1", AddressAndType(Opt.XLat.Table[0]), Default(L"")},
	{FSSF_PRIVATE,       NKeyXLat,L"Table2", AddressAndType(Opt.XLat.Table[1]), Default(L"")},
	{FSSF_PRIVATE,       NKeyXLat,L"WordDivForXlat", AddressAndType(Opt.XLat.strWordDivForXlat), Default(WordDivForXlat0)},
};

bool GetConfigValue(const wchar_t *Key, const wchar_t *Name, string &strValue)
{
	for (size_t I=0; I < ARRAYSIZE(CFG); ++I)
	{
		if (!StrCmpI(CFG[I].KeyName,Key) && !StrCmpI(CFG[I].ValName,Name))
		{
			if(FSSF_PRIVATE==CFG[I].ApiRoot) break;
			strValue = CFG[I].Value->toString();
			return true;
		}
	}

	return false;
}

bool GetConfigValue(size_t Root, const wchar_t* Name, GeneralConfig::OptionType& Type, Option*& Data)
{
	if(FSSF_PRIVATE!=Root)
	{
		for(size_t ii=0;ii<ARRAYSIZE(CFG);++ii)
		{
			if(Root==CFG[ii].ApiRoot&&!StrCmpI(CFG[ii].ValName,Name))
			{
				Type=CFG[ii].ValueType;
				Data=CFG[ii].Value;
				return true;
			}
		}
	}
	return false;
}

void ReadConfig()
{
	/* <ПРЕПРОЦЕССЫ> *************************************************** */

	/* BUGBUG??
	SetRegRootKey(HKEY_LOCAL_MACHINE);
	DWORD OptPolicies_ShowHiddenDrives=GetRegKey(NKeyPolicies,L"ShowHiddenDrives",1)&1;
	DWORD OptPolicies_DisabledOptions=GetRegKey(NKeyPolicies,L"DisabledOptions",0);
	SetRegRootKey(HKEY_CURRENT_USER);
	*/
	/* *************************************************** </ПРЕПРОЦЕССЫ> */

	GetPrivateProfileString(L"General", L"DefaultLanguage", L"English", DefaultLanguage, ARRAYSIZE(DefaultLanguage), g_strFarINI);

	for (size_t I=0; I < ARRAYSIZE(CFG); ++I)
	{
		CFG[I].Value->ReceiveValue(CFG[I].KeyName, CFG[I].ValName, CFG[I].Default);
	}

	/* <ПОСТПРОЦЕССЫ> *************************************************** */

	Opt.Palette.Load();
	Opt.GlobalUserMenuDir.ReleaseBuffer(GetPrivateProfileString(L"General", L"GlobalUserMenuDir", g_strFarPath, Opt.GlobalUserMenuDir.GetBuffer(NT_MAX_PATH), NT_MAX_PATH, g_strFarINI));
	apiExpandEnvironmentStrings(Opt.GlobalUserMenuDir, Opt.GlobalUserMenuDir);
	ConvertNameToFull(Opt.GlobalUserMenuDir,Opt.GlobalUserMenuDir);
	AddEndSlash(Opt.GlobalUserMenuDir);

	if (Opt.ExceptRules == -1)
	{
		Opt.ExceptRules = Opt.StoredExceptRules;
	}

	if(Opt.WindowMode == -1)
	{
		Opt.WindowMode = Opt.StoredWindowMode;
	}

	Opt.ElevationMode = Opt.StoredElevationMode;

	if (Opt.PluginMaxReadData < 0x1000)
		Opt.PluginMaxReadData=0x20000;

	if (!Opt.ViOpt.MaxLineSize)
		Opt.ViOpt.MaxLineSize = ViewerOptions::eDefLineSize;
	else if (Opt.ViOpt.MaxLineSize < ViewerOptions::eMinLineSize)
		Opt.ViOpt.MaxLineSize = ViewerOptions::eMinLineSize;
	else if (Opt.ViOpt.MaxLineSize > ViewerOptions::eMaxLineSize)
		Opt.ViOpt.MaxLineSize = ViewerOptions::eMaxLineSize;

	// Исключаем случайное стирание разделителей ;-)
	if (Opt.strWordDiv.IsEmpty())
		Opt.strWordDiv = WordDiv0;

	// Исключаем случайное стирание разделителей
	if (Opt.XLat.strWordDivForXlat.IsEmpty())
		Opt.XLat.strWordDivForXlat = WordDivForXlat0;

	Opt.PanelRightClickRule%=3;
	Opt.PanelCtrlAltShiftRule%=3;

	if (Opt.EdOpt.TabSize<1 || Opt.EdOpt.TabSize>512)
		Opt.EdOpt.TabSize = DefaultTabSize;

	if (Opt.ViOpt.TabSize<1 || Opt.ViOpt.TabSize>512)
		Opt.ViOpt.TabSize = DefaultTabSize;

	Opt.HelpTabSize = DefaultTabSize; // пока жестко пропишем...


	if ((Opt.Macro.KeyMacroCtrlDot=KeyNameToKey(Opt.Macro.strKeyMacroCtrlDot)) == -1)
		Opt.Macro.KeyMacroCtrlDot=KEY_CTRLDOT;

	if ((Opt.Macro.KeyMacroRCtrlDot=KeyNameToKey(Opt.Macro.strKeyMacroRCtrlDot)) == -1)
		Opt.Macro.KeyMacroRCtrlDot=KEY_RCTRLDOT;

	if ((Opt.Macro.KeyMacroCtrlShiftDot=KeyNameToKey(Opt.Macro.strKeyMacroCtrlShiftDot)) == -1)
		Opt.Macro.KeyMacroCtrlShiftDot=KEY_CTRLSHIFTDOT;

	if ((Opt.Macro.KeyMacroRCtrlShiftDot=KeyNameToKey(Opt.Macro.strKeyMacroRCtrlShiftDot)) == -1)
		Opt.Macro.KeyMacroRCtrlShiftDot=KEY_RCTRL|KEY_SHIFT|KEY_DOT;

	Opt.EdOpt.strWordDiv = Opt.strWordDiv;
	FileList::ReadPanelModes();

	/* BUGBUG??
	// уточняем системную политику
	// для дисков юзер может только отменять показ
	Opt.Policies.ShowHiddenDrives&=OptPolicies_ShowHiddenDrives;
	// для опций юзер может только добавлять блокироку пунктов
	Opt.Policies.DisabledOptions|=OptPolicies_DisabledOptions;
	*/

	if (Opt.Exec.strExecuteBatchType.IsEmpty()) // предохраняемся
		Opt.Exec.strExecuteBatchType=constBatchExt;

	// Инициализация XLat для русской раскладки qwerty<->йцукен
	if (Opt.XLat.Table[0].IsEmpty())
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
			Opt.XLat.Table[0] = L"\x2116\x0410\x0412\x0413\x0414\x0415\x0417\x0418\x0419\x041a\x041b\x041c\x041d\x041e\x041f\x0420\x0421\x0422\x0423\x0424\x0425\x0426\x0427\x0428\x0429\x042a\x042b\x042c\x042f\x0430\x0432\x0433\x0434\x0435\x0437\x0438\x0439\x043a\x043b\x043c\x043d\x043e\x043f\x0440\x0441\x0442\x0443\x0444\x0445\x0446\x0447\x0448\x0449\x044a\x044b\x044c\x044d\x044f\x0451\x0401\x0411\x042e";
			Opt.XLat.Table[1] = L"#FDULTPBQRKVYJGHCNEA{WXIO}SMZfdultpbqrkvyjghcnea[wxio]sm'z`~<>";
			Opt.XLat.Rules[0] = L",??&./\x0431,\x044e.:^\x0416:\x0436;;$\"@\x042d\"";
			Opt.XLat.Rules[1] = L"?,&?/.,\x0431.\x044e^::\x0416;\x0436$;@\"\"\x042d";
			Opt.XLat.Rules[2] = L"^::\x0416\x0416^$;;\x0436\x0436$@\"\"\x042d\x042d@&??,,\x0431\x0431&/..\x044e\x044e/";
		}
	}

	{
		Opt.XLat.CurrentLayout=0;
		ClearArray(Opt.XLat.Layouts);

		if (!Opt.XLat.strLayouts.IsEmpty())
		{
			wchar_t *endptr;
			const wchar_t *ValPtr;
			UserDefinedList DestList(ULF_UNIQUE);
			DestList.Set(Opt.XLat.strLayouts);
			size_t I=0;

			while (nullptr!=(ValPtr=DestList.GetNext()))
			{
				DWORD res=(DWORD)wcstoul(ValPtr, &endptr, 16);
				Opt.XLat.Layouts[I]=(HKL)(intptr_t)(HIWORD(res)? res : MAKELONG(res,res));
				++I;

				if (I >= ARRAYSIZE(Opt.XLat.Layouts))
					break;
			}

			if (I <= 1) // если указано меньше двух - "откключаем" эту
				Opt.XLat.Layouts[0]=0;
		}
	}

	ClearArray(Opt.FindOpt.OutColumnTypes);
	ClearArray(Opt.FindOpt.OutColumnWidths);
	ClearArray(Opt.FindOpt.OutColumnWidthType);
	Opt.FindOpt.OutColumnCount=0;


	if (!Opt.FindOpt.strSearchOutFormat.IsEmpty())
	{
		if (Opt.FindOpt.strSearchOutFormatWidth.IsEmpty())
			Opt.FindOpt.strSearchOutFormatWidth=L"0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0";
		TextToViewSettings(Opt.FindOpt.strSearchOutFormat,Opt.FindOpt.strSearchOutFormatWidth,
                                  Opt.FindOpt.OutColumnTypes,Opt.FindOpt.OutColumnWidths,Opt.FindOpt.OutColumnWidthType,
                                  Opt.FindOpt.OutColumnCount);
	}

	string tmp[2];
	if (!GeneralCfg->EnumValues(L"Masks", 0, tmp[0], tmp[1]))
	{
		ApplyDefaultMaskGroups();
	}

	StrToGuid(Opt.KnownIDs.EmenuGuidStr, Opt.KnownIDs.Emenu);
	StrToGuid(Opt.KnownIDs.NetworkGuidStr, Opt.KnownIDs.Network);

/* *************************************************** </ПОСТПРОЦЕССЫ> */

	// we assume that any changes after this point will be made by the user
	for(size_t i = 0; i < ARRAYSIZE(CFG); ++i)
	{
		CFG[i].Value->MakeUnchanged();
	}
}


void SaveConfig(int Ask)
{
	if (Opt.Policies.DisabledOptions&0x20000) // Bit 17 - Сохранить параметры
		return;

	if (Ask && Message(0,2,MSG(MSaveSetupTitle),MSG(MSaveSetupAsk1),MSG(MSaveSetupAsk2),MSG(MSaveSetup),MSG(MCancel)))
		return;

	/* <ПРЕПРОЦЕССЫ> *************************************************** */
	Panel *LeftPanel=CtrlObject->Cp()->LeftPanel;
	Panel *RightPanel=CtrlObject->Cp()->RightPanel;
	Opt.LeftPanel.Focus=LeftPanel->GetFocus() != 0;
	Opt.LeftPanel.Visible=LeftPanel->IsVisible() != 0;
	Opt.RightPanel.Focus=RightPanel->GetFocus() != 0;
	Opt.RightPanel.Visible=RightPanel->IsVisible() != 0;

	if (LeftPanel->GetMode()==NORMAL_PANEL)
	{
		Opt.LeftPanel.Type=LeftPanel->GetType();
		Opt.LeftPanel.ViewMode=LeftPanel->GetViewMode();
		Opt.LeftPanel.SortMode=LeftPanel->GetSortMode();
		Opt.LeftPanel.SortOrder=LeftPanel->GetSortOrder();
		Opt.LeftPanel.SortGroups=LeftPanel->GetSortGroups() != 0;
		Opt.LeftPanel.ShowShortNames=LeftPanel->GetShowShortNamesMode() != 0;
		Opt.LeftPanel.NumericSort=LeftPanel->GetNumericSort() != 0;
		Opt.LeftPanel.CaseSensitiveSort=LeftPanel->GetCaseSensitiveSort() != 0;
		Opt.LeftSelectedFirst=LeftPanel->GetSelectedFirstMode() != 0;
		Opt.LeftPanel.DirectoriesFirst=LeftPanel->GetDirectoriesFirst() != 0;
	}

	string strTemp1, strTemp2;
	LeftPanel->GetCurDir(strTemp1);
	Opt.strLeftFolder = strTemp1;
	LeftPanel->GetCurBaseName(strTemp1, strTemp2);
	Opt.strLeftCurFile = strTemp1;
	if (RightPanel->GetMode()==NORMAL_PANEL)
	{
		Opt.RightPanel.Type=RightPanel->GetType();
		Opt.RightPanel.ViewMode=RightPanel->GetViewMode();
		Opt.RightPanel.SortMode=RightPanel->GetSortMode();
		Opt.RightPanel.SortOrder=RightPanel->GetSortOrder();
		Opt.RightPanel.SortGroups=RightPanel->GetSortGroups() != 0;
		Opt.RightPanel.ShowShortNames=RightPanel->GetShowShortNamesMode() != 0;
		Opt.RightPanel.NumericSort=RightPanel->GetNumericSort() != 0;
		Opt.RightPanel.CaseSensitiveSort=RightPanel->GetCaseSensitiveSort() != 0;
		Opt.RightSelectedFirst=RightPanel->GetSelectedFirstMode() != 0;
		Opt.RightPanel.DirectoriesFirst=RightPanel->GetDirectoriesFirst() != 0;
	}

	RightPanel->GetCurDir(strTemp1);
	Opt.strRightFolder = strTemp1;
	RightPanel->GetCurBaseName(strTemp1, strTemp2);
	Opt.strRightCurFile = strTemp1;
	CtrlObject->HiFiles->SaveHiData();
	/* *************************************************** </ПРЕПРОЦЕССЫ> */

	Opt.Palette.Save();

	GeneralCfg->BeginTransaction();

	for (size_t I=0; I < ARRAYSIZE(CFG); ++I)
	{
		CFG[I].Value->StoreValue(CFG[I].KeyName, CFG[I].ValName);
	}

	GeneralCfg->EndTransaction();

	/* <ПОСТПРОЦЕССЫ> *************************************************** */
	FileFilter::SaveFilters();
	FileList::SavePanelModes();

	if (Ask)
		CtrlObject->Macro.SaveMacros();

	/* *************************************************** </ПОСТПРОЦЕССЫ> */
}

inline const wchar_t* TypeToText(GeneralConfig::OptionType Type)
{
	static const wchar_t* OptionTypeNames[] =
	{
		L"boolean",
		L"3-state",
		L"integer",
		L"string",
	};
	static_assert(ARRAYSIZE(OptionTypeNames)==GeneralConfig::TYPE_LAST+1, "not all GeneralConfig types handled");
	return OptionTypeNames[Type];
}

void FillListItem(FarListItem& Item, FormatString& fs, FARConfig& cfg)
{
	Item.Flags = 0;
	Item.Reserved[0] = Item.Reserved[1] = Item.Reserved[2] = 0;
	fs.Clear();
	fs << fmt::ExactWidth(42) << fmt::LeftAlign() << (string(cfg.KeyName) + "." + cfg.ValName) << BoxSymbols[BS_V1]
	<< fmt::ExactWidth(7) << fmt::LeftAlign() << TypeToText(cfg.ValueType) << BoxSymbols[BS_V1];
	bool Changed = false;
	fs << cfg.Value->toString();
	switch(cfg.ValueType)
	{
	case GeneralConfig::TYPE_BOOLEAN:
		Changed = static_cast<BoolOption*>(cfg.Value)->Get() != cfg.bDefault;
		break;
	case GeneralConfig::TYPE_BOOLEAN3:
		Changed = static_cast<Bool3Option*>(cfg.Value)->Get() != cfg.iDefault;
		break;
	case GeneralConfig::TYPE_INTEGER:
		Changed = static_cast<IntOption*>(cfg.Value)->Get() != cfg.iDefault;
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
		break;
	case GeneralConfig::TYPE_STRING:
		Changed = static_cast<StringOption*>(cfg.Value)->Get() != cfg.sDefault;
		break;
	}
	if(Changed)
	{
		Item.Flags = LIF_CHECKED|L'*';
	}
	Item.Text = fs;
}

intptr_t WINAPI AdvancedConfigDlgProc(HANDLE hDlg, int Msg, int Param1, void* Param2)
{
	static FormatString* fs;
	switch (Msg)
	{
	case DN_INITDIALOG:
		fs = reinterpret_cast<FormatString*>(Param2);
		break;

	case DN_RESIZECONSOLE:
		{
			COORD Size = {(SHORT)Max(ScrX-4, 60), (SHORT)Max(ScrY-2, 20)};
			SendDlgMessage(hDlg, DM_RESIZEDIALOG, 0, &Size);
			SMALL_RECT ListPos = {3, 1, (SHORT)(Size.X-4), (SHORT)(Size.Y-2)};
			SendDlgMessage(hDlg, DM_SETITEMPOSITION, 0, &ListPos);
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
						SendDlgMessage(hDlg, DM_LISTINFO, Param1, &ListInfo);

						string HelpTopic = string(CFG[ListInfo.SelectPos].KeyName) + L"." + CFG[ListInfo.SelectPos].ValName;
						Help hlp(HelpTopic.CPtr(), nullptr, FHELP_NOSHOWERROR);
						if (hlp.GetError())
						{
							HelpTopic = string(CFG[ListInfo.SelectPos].KeyName) + L"Settings";
							Help hlp1(HelpTopic.CPtr(), nullptr, FHELP_NOSHOWERROR);
						}
					}
					break;

				case KEY_F4:
					SendDlgMessage(hDlg, DM_CLOSE, 0, nullptr);
					break;

				case KEY_SHIFTF4:
					SendDlgMessage(hDlg, DM_CLOSE, 1, nullptr);
					break;

				case KEY_CTRLH:
					{
						static bool HideUnchanged = true;
						SendDlgMessage(hDlg, DM_ENABLEREDRAW, 0 , 0);
						FarListInfo ListInfo = {sizeof(ListInfo)};
						SendDlgMessage(hDlg, DM_LISTINFO, Param1, &ListInfo);
						for(int i = 0; i < static_cast<int>(ListInfo.ItemsNumber); ++i)
						{
							FarListGetItem Item={sizeof(FarListGetItem), i};
							SendDlgMessage(hDlg, DM_LISTGETITEM, 0, &Item);
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
								SendDlgMessage(hDlg, DM_LISTUPDATE, 0, &UpdatedItem);
							}
						}
						HideUnchanged = !HideUnchanged;
						SendDlgMessage(hDlg, DM_ENABLEREDRAW, 1 , 0);
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
			SendDlgMessage(hDlg, DM_LISTINFO, 0, &ListInfo);

			bool Changed = false;

			if(CFG[ListInfo.SelectPos].ValueType == GeneralConfig::TYPE_BOOLEAN)
			{
				Changed = true;
				*static_cast<BoolOption*>(CFG[ListInfo.SelectPos].Value) = !*static_cast<BoolOption*>(CFG[ListInfo.SelectPos].Value);
			}
			else if(CFG[ListInfo.SelectPos].ValueType == GeneralConfig::TYPE_BOOLEAN3)
			{
				Changed = true;
				++(*static_cast<Bool3Option*>(CFG[ListInfo.SelectPos].Value));
			}
			else
			{
				DialogBuilder Builder;
				Builder.AddText(string(CFG[ListInfo.SelectPos].KeyName) + "." + CFG[ListInfo.SelectPos].ValName + L" (" + TypeToText(CFG[ListInfo.SelectPos].ValueType) + L"):");
				switch(CFG[ListInfo.SelectPos].ValueType)
				{
				case GeneralConfig::TYPE_BOOLEAN:
				case GeneralConfig::TYPE_BOOLEAN3:
					// only to suppress C4062, TYPE_BOOLEAN is handled above
					break;
				case GeneralConfig::TYPE_INTEGER:
					if (Param1)
						Builder.AddHexEditField(*static_cast<IntOption*>(CFG[ListInfo.SelectPos].Value), 40);
					else
						Builder.AddIntEditField(*static_cast<IntOption*>(CFG[ListInfo.SelectPos].Value), 40);
					break;
				case GeneralConfig::TYPE_STRING:
					Builder.AddEditField(*static_cast<StringOption*>(CFG[ListInfo.SelectPos].Value), 40);
					break;
				}
				static_cast<DialogBuilderBase<DialogItemEx>*>(&Builder)->AddOKCancel(MOk, MConfigResetValue, MCancel);
				int Result = Builder.ShowDialogEx();
				if(Result == 0 || Result == 1)
				{
					Changed = true;
					if(Result == 1)
					{
						// reset to default
						switch(CFG[ListInfo.SelectPos].ValueType)
						{
						case GeneralConfig::TYPE_BOOLEAN:
						case GeneralConfig::TYPE_BOOLEAN3:
							// only to suppress C4062, TYPE_BOOLEAN is handled above
							break;
						case GeneralConfig::TYPE_INTEGER:
							*static_cast<IntOption*>(CFG[ListInfo.SelectPos].Value) = CFG[ListInfo.SelectPos].iDefault;
							break;
						case GeneralConfig::TYPE_STRING:
							*static_cast<StringOption*>(CFG[ListInfo.SelectPos].Value) = CFG[ListInfo.SelectPos].sDefault;
							break;
						}
					}
				}
			}

			if(Changed)
			{
				SendDlgMessage(hDlg, DM_ENABLEREDRAW, 0 , 0);
				FarListUpdate flu = {sizeof(flu), ListInfo.SelectPos};
				FillListItem(flu.Item, fs[ListInfo.SelectPos], CFG[ListInfo.SelectPos]);
				SendDlgMessage(hDlg, DM_LISTUPDATE, 0, &flu);
				FarListPos flp = {sizeof(flp), ListInfo.SelectPos, ListInfo.TopPos};
				SendDlgMessage(hDlg, DM_LISTSETCURPOS, 0, &flp);
				SendDlgMessage(hDlg, DM_ENABLEREDRAW, 1 , 0);
			}
			return FALSE;
		}
		break;
	default:
		break;
	}

	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

bool AdvancedConfig()
{
	int DlgWidth = Max(ScrX-4, 60), DlgHeight = Max(ScrY-2, 20);
	FarDialogItem AdvancedConfigDlgData[]=
	{
		{DI_LISTBOX,3,1,DlgWidth-4,DlgHeight-2,0,nullptr,nullptr,DIF_NONE,nullptr},
	};
	MakeDialogItemsEx(AdvancedConfigDlgData,AdvancedConfigDlg);

	FarList Items;
	Items.ItemsNumber = ARRAYSIZE(CFG);
	Items.Items = new FarListItem[Items.ItemsNumber];

	FormatString fs[ARRAYSIZE(CFG)];
	for(size_t i = 0; i < Items.ItemsNumber; ++i)
	{
		FillListItem(Items.Items[i], fs[i], CFG[i]);
	}

	AdvancedConfigDlg[0].ListItems = &Items;

	Dialog Dlg(AdvancedConfigDlg,ARRAYSIZE(AdvancedConfigDlg), AdvancedConfigDlgProc, &fs);
	Dlg.SetHelp(L"FarConfig");
	Dlg.SetPosition(-1, -1, DlgWidth, DlgHeight);
	Dlg.Process();
	delete[] Items.Items;
	return true;
}


bool BoolOption::ReceiveValue(const wchar_t* KeyName, const wchar_t* ValueName, bool Default)
{
	int CfgValue = Default;
	bool Result = GeneralCfg->GetValue(KeyName, ValueName, &CfgValue, CfgValue);
	Set(CfgValue != 0);
	return Result;
}

bool BoolOption::StoreValue(const wchar_t* KeyName, const wchar_t* ValueName)
{
	return !Changed() || GeneralCfg->SetValue(KeyName, ValueName, Get());
}

bool Bool3Option::ReceiveValue(const wchar_t* KeyName, const wchar_t* ValueName, int Default)
{
	int CfgValue = Default;
	bool Result = GeneralCfg->GetValue(KeyName, ValueName, &CfgValue, CfgValue);
	Set(CfgValue);
	return Result;
}

bool Bool3Option::StoreValue(const wchar_t* KeyName, const wchar_t* ValueName)
{
	return !Changed() || GeneralCfg->SetValue(KeyName, ValueName, Get());
}

bool IntOption::ReceiveValue(const wchar_t* KeyName, const wchar_t* ValueName, int Default)
{
	int CfgValue = Default;
	bool Result = GeneralCfg->GetValue(KeyName, ValueName, &CfgValue, CfgValue);
	Set(CfgValue);
	return Result;
}

bool IntOption::StoreValue(const wchar_t* KeyName, const wchar_t* ValueName)
{
	return !Changed() || GeneralCfg->SetValue(KeyName, ValueName, Get());
}

bool StringOption::ReceiveValue(const wchar_t* KeyName, const wchar_t* ValueName, const wchar_t* Default)
{
	string CfgValue = Default;
	bool Result = GeneralCfg->GetValue(KeyName, ValueName, CfgValue, CfgValue);
	Set(CfgValue);
	return Result;
}

bool StringOption::StoreValue(const wchar_t* KeyName, const wchar_t* ValueName)
{
	return !Changed() || GeneralCfg->SetValue(KeyName, ValueName, Get());
}
