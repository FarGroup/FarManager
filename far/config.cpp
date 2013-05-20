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
#include "FarDlgBuilder.hpp"
#include "elevation.hpp"
#include "configdb.hpp"
#include "FarGuid.hpp"
#include "vmenu2.hpp"
#include "codepage.hpp"
#include "DlgGuid.hpp"

// Стандартный набор разделителей
static const wchar_t* WordDiv0 = L"~!%^&*()+|{}:\"<>?`-=\\[];',./";

// Стандартный набор разделителей для функции Xlat
static const wchar_t* WordDivForXlat0=L" \t!#$%^&*()+|=\\/@?";

static const wchar_t* constBatchExt=L".BAT;.CMD;";

static const int DefaultTabSize = 8;

static wchar_t DefaultLanguage[100] = {};

#if defined(TREEFILE_PROJECT)
static const wchar_t* constLocalDiskTemplate=L"%D.%SN.tree";
static const wchar_t* constNetDiskTemplate=L"%D.%SN.tree";
static const wchar_t* constNetPathTemplate=L"%SR.%SH.tree";
static const wchar_t* constRemovableDiskTemplate=L"%SN.tree";
static const wchar_t* constCDDiskTemplate=L"CD.%L.%SN.tree";
#endif

static const wchar_t* NKeyScreen=L"Screen";
static const wchar_t* NKeyCmdline=L"Cmdline";
static const wchar_t* NKeyInterface=L"Interface";
static const wchar_t* NKeyInterfaceCompletion=L"Interface.Completion";
static const wchar_t* NKeyViewer=L"Viewer";
static const wchar_t* NKeyDialog=L"Dialog";
static const wchar_t* NKeyEditor=L"Editor";
static const wchar_t* NKeyXLat=L"XLat";
static const wchar_t* NKeySystem=L"System";
static const wchar_t* NKeySystemException=L"System.Exception";
static const wchar_t* NKeySystemKnownIDs=L"System.KnownIDs";
static const wchar_t* NKeySystemExecutor=L"System.Executor";
static const wchar_t* NKeySystemNowell=L"System.Nowell";
static const wchar_t* NKeyHelp=L"Help";
static const wchar_t* NKeyLanguage=L"Language";
static const wchar_t* NKeyConfirmations=L"Confirmations";
static const wchar_t* NKeyPluginConfirmations=L"PluginConfirmations";
static const wchar_t* NKeyPanel=L"Panel";
static const wchar_t* NKeyPanelLeft=L"Panel.Left";
static const wchar_t* NKeyPanelRight=L"Panel.Right";
static const wchar_t* NKeyPanelLayout=L"Panel.Layout";
static const wchar_t* NKeyPanelTree=L"Panel.Tree";
static const wchar_t* NKeyPanelInfo=L"Panel.Info";
static const wchar_t* NKeyLayout=L"Layout";
static const wchar_t* NKeyDescriptions=L"Descriptions";
static const wchar_t* NKeyKeyMacros=L"Macros";
static const wchar_t* NKeyPolicies=L"Policies";
static const wchar_t* NKeyCodePages=L"CodePages";
static const wchar_t* NKeyVMenu=L"VMenu";
static const wchar_t* NKeyCommandHistory=L"History.CommandHistory";
static const wchar_t* NKeyViewEditHistory=L"History.ViewEditHistory";
static const wchar_t* NKeyFolderHistory=L"History.FolderHistory";
static const wchar_t* NKeyDialogHistory=L"History.DialogHistory";

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
	Builder.AddComboBox(Global->Opt->InfoPanel.ComputerNameFormat, 50, CNListItems, ARRAYSIZE(CNListItems), DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddText(MConfigInfoPanelUNTitle);
	Builder.AddComboBox(Global->Opt->InfoPanel.UserNameFormat, 50, UNListItems, ARRAYSIZE(UNListItems), DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
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
		Item.UserData = UNSAFE_CSTR(Name);
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
							filemasks Masks;
							Masks.Set(CurrentMasks);
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
	Builder.AddComboBox(Global->Opt->VMenu.LBtnClick, 40, CAListItems, ARRAYSIZE(CAListItems), DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddText(MConfigVMenuRBtnClick);
	Builder.AddComboBox(Global->Opt->VMenu.RBtnClick, 40, CAListItems, ARRAYSIZE(CAListItems), DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddText(MConfigVMenuMBtnClick);
	Builder.AddComboBox(Global->Opt->VMenu.MBtnClick, 40, CAListItems, ARRAYSIZE(CAListItems), DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
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

void ViewerConfig(Options::ViewerOptions &ViOpt,bool Local)
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

	std::vector<DialogBuilderListItem2> Items; //Must live until Dialog end
	if (!Local)
	{
		Builder.AddEmptyLine();
		DialogItemEx *MaxLineSize = Builder.AddIntEditField(Global->Opt->ViOpt.MaxLineSize, 6);
		Builder.AddTextAfter(MaxLineSize, MViewConfigMaxLineSize);
		Builder.AddCheckbox(MViewAutoDetectCodePage, Global->Opt->ViOpt.AutoDetectCodePage);
		Builder.AddText(MViewConfigDefaultCodePage);
		Global->CodePages->FillCodePagesList(Items, false, false, false, false);
		Builder.AddComboBox(Global->Opt->ViOpt.DefaultCodePage, 64, Items, DIF_LISTWRAPMODE|DIF_LISTAUTOHIGHLIGHT);
	}

	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (Global->Opt->ViOpt.SavePos)
			ViOpt.SaveCodepage = true; // codepage is part of saved position
		if (ViOpt.TabSize<1 || ViOpt.TabSize>512)
			ViOpt.TabSize = DefaultTabSize;
		if (!Global->Opt->ViOpt.MaxLineSize)
			Global->Opt->ViOpt.MaxLineSize = Options::ViewerOptions::eDefLineSize;
		else if (Global->Opt->ViOpt.MaxLineSize < Options::ViewerOptions::eMinLineSize)
			Global->Opt->ViOpt.MaxLineSize = Options::ViewerOptions::eMinLineSize;
		else if (Global->Opt->ViOpt.MaxLineSize > Options::ViewerOptions::eMaxLineSize)
			Global->Opt->ViOpt.MaxLineSize = Options::ViewerOptions::eMaxLineSize;
	}
}

void EditorConfig(Options::EditorOptions &EdOpt,bool Local)
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
	Builder.AddComboBox(EdOpt.ExpandTabs, 64, ExpandTabsItems, 3, DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);

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

	std::vector<DialogBuilderListItem2> Items; //Must live until Dialog end
	if (!Local)
	{
		Builder.AddEmptyLine();
		Builder.AddCheckbox(MEditShareWrite, EdOpt.EditOpenedForWrite);
		Builder.AddCheckbox(MEditLockROFileModification, EdOpt.ReadOnlyLock, 1);
		Builder.AddCheckbox(MEditWarningBeforeOpenROFile, EdOpt.ReadOnlyLock, 2);
		Builder.AddCheckbox(MEditAutoDetectCodePage, EdOpt.AutoDetectCodePage);
		Builder.AddText(MEditConfigDefaultCodePage);
		Global->CodePages->FillCodePagesList(Items, false, false, false, true);
		Builder.AddComboBox(EdOpt.DefaultCodePage, 64, Items, DIF_LISTWRAPMODE|DIF_LISTAUTOHIGHLIGHT);
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
	              Global->Opt->InfoPanel.strFolderInfoFiles.CPtr(),strFolderInfoFiles,L"FolderDiz",FIB_ENABLEEMPTY|FIB_BUTTONS))
	{
		Global->Opt->InfoPanel.strFolderInfoFiles = strFolderInfoFiles;

		if (Global->CtrlObject->Cp()->LeftPanel->GetType() == INFO_PANEL)
			Global->CtrlObject->Cp()->LeftPanel->Update(0);

		if (Global->CtrlObject->Cp()->RightPanel->GetType() == INFO_PANEL)
			Global->CtrlObject->Cp()->RightPanel->Update(0);
	}
}


struct default_value
{
	union
	{
		const wchar_t* sDefault;
		long long iDefault;
		bool bDefault;
	};
	default_value():iDefault(0) {}
	default_value(const wchar_t* sDefault):sDefault(sDefault) {}
	default_value(long long iDefault):iDefault(iDefault) {}
	default_value(unsigned long long iDefault):iDefault(iDefault) {}
	default_value(int iDefault):iDefault(iDefault) {}
	default_value(unsigned int iDefault):iDefault(iDefault) {}
	default_value(bool bDefault):bDefault(bDefault) {}
};

struct FARConfigItem
{
	size_t ApiRoot;
	const wchar_t *KeyName;
	const wchar_t *ValName;
	Option* Value;   // адрес переменной, куда помещаем данные
	default_value Default;

	FormatString ListItemString;

	FarListItem MakeListItem()
	{
		FarListItem Item;
		Item.Flags = 0;
		Item.Reserved[0] = Item.Reserved[1] = 0;
		ListItemString.Clear();
		ListItemString << fmt::ExactWidth(42) << fmt::LeftAlign() << (string(KeyName) + "." + ValName) << BoxSymbols[BS_V1]
		<< fmt::ExactWidth(7) << fmt::LeftAlign() << Value->typeToString() << BoxSymbols[BS_V1]
		<< Value->toString() << Value->ExInfo();
		if(!Value->IsDefault(this))
		{
			Item.Flags = LIF_CHECKED|L'*';
		}
		Item.Text = ListItemString.CPtr();
		return Item;
	}

	bool Edit(bool Hex)
	{
		DialogBuilder Builder;
		Builder.AddText((string(KeyName) + L"." + ValName + L" (" + Value->typeToString() + L"):").CPtr());
		int Result = 0;
		if (!Value->Edit(&Builder, 40, Hex))
		{
			static_cast<DialogBuilderBase<DialogItemEx>*>(&Builder)->AddOKCancel(MOk, MConfigResetValue, MCancel);
			Result = Builder.ShowDialogEx();
		}
		if(Result == 0 || Result == 1)
		{
			if(Result == 1)
			{
				Value->SetDefault(this);
			}
			return true;
		}
		return false;
	}
};


bool BoolOption::IsDefault(const struct FARConfigItem* Holder) const
{
	return Get() == Holder->Default.bDefault;
}

void BoolOption::SetDefault(const struct FARConfigItem* Holder)
{
	Set(Holder->Default.bDefault);
}

bool BoolOption::Edit(DialogBuilder* Builder, int Width, int Param)
{
	Set(!Get());
	return true;
}

bool BoolOption::ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, bool Default)
{
	long long CfgValue = Default;
	bool Result = Storage->GetValue(KeyName, ValueName, &CfgValue, CfgValue);
	Set(CfgValue != 0);
	return Result;
}

bool BoolOption::ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, const default_value* Default)
{
	return ReceiveValue(Storage, KeyName, ValueName, Default->bDefault);
}

bool BoolOption::StoreValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName) const
{
	return !Changed() || Storage->SetValue(KeyName, ValueName, Get());
}


bool Bool3Option::IsDefault(const struct FARConfigItem* Holder) const
{
	return Get() == Holder->Default.iDefault;
}

void Bool3Option::SetDefault(const struct FARConfigItem* Holder)
{
	Set(Holder->Default.iDefault);
}

bool Bool3Option::Edit(DialogBuilder* Builder, int Width, int Param)
{
	++*this;
	return true;
}

bool Bool3Option::ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, int Default)
{
	long long CfgValue = Default;
	bool Result = Storage->GetValue(KeyName, ValueName, &CfgValue, CfgValue);
	Set(CfgValue);
	return Result;
}

bool Bool3Option::ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, const default_value* Default)
{
	return ReceiveValue(Storage, KeyName, ValueName, Default->iDefault);
}

bool Bool3Option::StoreValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName) const
{
	return !Changed() || Storage->SetValue(KeyName, ValueName, Get());
}


bool IntOption::IsDefault(const struct FARConfigItem* Holder) const
{
	return Get() == Holder->Default.iDefault;
}

void IntOption::SetDefault(const struct FARConfigItem* Holder)
{
	Set(Holder->Default.iDefault);
}

bool IntOption::Edit(DialogBuilder* Builder, int Width, int Param)
{
	if (Param)
		Builder->AddHexEditField(*this, Width);
	else
		Builder->AddIntEditField(*this, Width);
	return false;
}

bool IntOption::ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, long long Default)
{
	long long CfgValue = Default;
	bool Result = Storage->GetValue(KeyName, ValueName, &CfgValue, CfgValue);
	Set(CfgValue);
	return Result;
}

bool IntOption::ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, const default_value* Default)
{
	return ReceiveValue(Storage, KeyName, ValueName, Default->iDefault);
}

bool IntOption::StoreValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName) const
{
	return !Changed() || Storage->SetValue(KeyName, ValueName, Get());
}

const string IntOption::ExInfo() const
{
	FormatString Result;
	int v = Get();
	wchar_t w1 = static_cast<wchar_t>(v);
	wchar_t w2 = static_cast<wchar_t>(v >> 16);
	Result << L" = 0x" << fmt::MaxWidth(8) << fmt::Radix(16) << v;
	if (w1 > 0x001f && w1 < 0x8000)
	{
		Result << L" = '" << w1;
		if (w2 > 0x001f && w2 < 0x8000)
			Result << w2;
		Result << L"'";
	}
	return Result;
}


bool StringOption::IsDefault(const struct FARConfigItem* Holder) const
{
	return Get() == Holder->Default.sDefault;
}

void StringOption::SetDefault(const struct FARConfigItem* Holder)
{
	Set(Holder->Default.sDefault);
}

bool StringOption::Edit(DialogBuilder* Builder, int Width, int Param)
{
	Builder->AddEditField(*this, Width);
	return false;
}

bool StringOption::ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, const wchar_t* Default)
{
	string CfgValue = Default;
	bool Result = Storage->GetValue(KeyName, ValueName, CfgValue, CfgValue.CPtr());
	Set(CfgValue);
	return Result;
}

bool StringOption::ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, const default_value* Default)
{
	return ReceiveValue(Storage, KeyName, ValueName, Default->sDefault);
}

bool StringOption::StoreValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName) const
{
	return !Changed() || Storage->SetValue(KeyName, ValueName, Get());
}


Options::Options():
	ReadOnlyConfig(-1),
	UseExceptionHandler(0),
	ElevationMode(0),
	WindowMode(-1)
{
	// По умолчанию - брать плагины из основного каталога
	LoadPlug.MainPluginDir = true;
	LoadPlug.PluginsPersonal = true;
	LoadPlug.PluginsCacheOnly = false;

	Macro.DisableMacro=0;
}

FARConfigItem* Options::farconfig::begin() const {return m_items;}

FARConfigItem* Options::farconfig::end() const {return m_items + m_size;}

const FARConfigItem* Options::farconfig::cbegin() const {return m_items;}

const FARConfigItem* Options::farconfig::cend() const {return m_items + m_size;}

FARConfigItem& Options::farconfig::operator[](size_t i) const {return m_items[i];}

void Options::InitRoamingCFG()
{
	static FARConfigItem _CFG[] =
	{
		{FSSF_PRIVATE,       NKeyCmdline, L"AutoComplete", &Global->Opt->CmdLine.AutoComplete, true},
		{FSSF_PRIVATE,       NKeyCmdline, L"EditBlock", &Global->Opt->CmdLine.EditBlock, false},
		{FSSF_PRIVATE,       NKeyCmdline, L"DelRemovesBlocks", &Global->Opt->CmdLine.DelRemovesBlocks, true},
		{FSSF_PRIVATE,       NKeyCmdline, L"PromptFormat", &Global->Opt->CmdLine.strPromptFormat, L"$p$g"},
		{FSSF_PRIVATE,       NKeyCmdline, L"UsePromptFormat", &Global->Opt->CmdLine.UsePromptFormat, false},

		{FSSF_PRIVATE,       NKeyCodePages,L"CPMenuMode", &Global->Opt->CPMenuMode, false},
		{FSSF_PRIVATE,       NKeyCodePages,L"NoAutoDetectCP", &Global->Opt->strNoAutoDetectCP, L""},

		{FSSF_PRIVATE,       NKeyConfirmations,L"AllowReedit", &Global->Opt->Confirm.AllowReedit, true},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Copy", &Global->Opt->Confirm.Copy, true},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Delete", &Global->Opt->Confirm.Delete, true},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"DeleteFolder", &Global->Opt->Confirm.DeleteFolder, true},
		{FSSF_PRIVATE,       NKeyConfirmations,L"DetachVHD", &Global->Opt->Confirm.DetachVHD, true},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Drag", &Global->Opt->Confirm.Drag, true},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Esc", &Global->Opt->Confirm.Esc, true},
		{FSSF_PRIVATE,       NKeyConfirmations,L"EscTwiceToInterrupt", &Global->Opt->Confirm.EscTwiceToInterrupt, false},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Exit", &Global->Opt->Confirm.Exit, true},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"HistoryClear", &Global->Opt->Confirm.HistoryClear, true},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Move", &Global->Opt->Confirm.Move, true},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"RemoveConnection", &Global->Opt->Confirm.RemoveConnection, true},
		{FSSF_PRIVATE,       NKeyConfirmations,L"RemoveHotPlug", &Global->Opt->Confirm.RemoveHotPlug, true},
		{FSSF_PRIVATE,       NKeyConfirmations,L"RemoveSUBST", &Global->Opt->Confirm.RemoveSUBST, true},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"RO", &Global->Opt->Confirm.RO, true},

		{FSSF_PRIVATE,       NKeyDescriptions,L"AnsiByDefault", &Global->Opt->Diz.AnsiByDefault, false},
		{FSSF_PRIVATE,       NKeyDescriptions,L"ListNames", &Global->Opt->Diz.strListNames, L"Descript.ion,Files.bbs"},
		{FSSF_PRIVATE,       NKeyDescriptions,L"ROUpdate", &Global->Opt->Diz.ROUpdate, false},
		{FSSF_PRIVATE,       NKeyDescriptions,L"SaveInUTF", &Global->Opt->Diz.SaveInUTF, false},
		{FSSF_PRIVATE,       NKeyDescriptions,L"SetHidden", &Global->Opt->Diz.SetHidden, true},
		{FSSF_PRIVATE,       NKeyDescriptions,L"StartPos", &Global->Opt->Diz.StartPos, 0},
		{FSSF_PRIVATE,       NKeyDescriptions,L"UpdateMode", &Global->Opt->Diz.UpdateMode, DIZ_UPDATE_IF_DISPLAYED},

		{FSSF_PRIVATE,       NKeyDialog,L"AutoComplete", &Global->Opt->Dialogs.AutoComplete, true},
		{FSSF_PRIVATE,       NKeyDialog,L"CBoxMaxHeight", &Global->Opt->Dialogs.CBoxMaxHeight, 8},
		{FSSF_DIALOG,        NKeyDialog,L"EditBlock", &Global->Opt->Dialogs.EditBlock, false},
		{FSSF_PRIVATE,       NKeyDialog,L"EditHistory", &Global->Opt->Dialogs.EditHistory, true},
		{FSSF_PRIVATE,       NKeyDialog,L"EditLine", &Global->Opt->Dialogs.EditLine, 0},
		{FSSF_DIALOG,        NKeyDialog,L"DelRemovesBlocks", &Global->Opt->Dialogs.DelRemovesBlocks, true},
		{FSSF_DIALOG,        NKeyDialog,L"EULBsClear", &Global->Opt->Dialogs.EULBsClear, false},
		{FSSF_PRIVATE,       NKeyDialog,L"MouseButton", &Global->Opt->Dialogs.MouseButton, 0xFFFF},

		{FSSF_PRIVATE,       NKeyEditor,L"AddUnicodeBOM", &Global->Opt->EdOpt.AddUnicodeBOM, true},
		{FSSF_PRIVATE,       NKeyEditor,L"AllowEmptySpaceAfterEof", &Global->Opt->EdOpt.AllowEmptySpaceAfterEof,false},
		{FSSF_PRIVATE,       NKeyEditor,L"AutoDetectCodePage", &Global->Opt->EdOpt.AutoDetectCodePage, true},
		{FSSF_PRIVATE,       NKeyEditor,L"AutoIndent", &Global->Opt->EdOpt.AutoIndent, false},
		{FSSF_PRIVATE,       NKeyEditor,L"BSLikeDel", &Global->Opt->EdOpt.BSLikeDel, true},
		{FSSF_PRIVATE,       NKeyEditor,L"CharCodeBase", &Global->Opt->EdOpt.CharCodeBase, 1},
		{FSSF_PRIVATE,       NKeyEditor,L"DefaultCodePage", &Global->Opt->EdOpt.DefaultCodePage, GetACP()},
		{FSSF_PRIVATE,       NKeyEditor,L"DelRemovesBlocks", &Global->Opt->EdOpt.DelRemovesBlocks, true},
		{FSSF_PRIVATE,       NKeyEditor,L"EditOpenedForWrite", &Global->Opt->EdOpt.EditOpenedForWrite, true},
		{FSSF_PRIVATE,       NKeyEditor,L"EditorCursorBeyondEOL", &Global->Opt->EdOpt.CursorBeyondEOL, true},
		{FSSF_PRIVATE,       NKeyEditor,L"EditorF7Rules", &Global->Opt->EdOpt.F7Rules, false},
		{FSSF_PRIVATE,       NKeyEditor,L"ExpandTabs", &Global->Opt->EdOpt.ExpandTabs, 0},
		{FSSF_PRIVATE,       NKeyEditor,L"ExternalEditorName", &Global->Opt->strExternalEditor, L""},
		{FSSF_PRIVATE,       NKeyEditor,L"FileSizeLimit", &Global->Opt->EdOpt.FileSizeLimitLo, 0},
		{FSSF_PRIVATE,       NKeyEditor,L"FileSizeLimitHi", &Global->Opt->EdOpt.FileSizeLimitHi, 0},
		{FSSF_PRIVATE,       NKeyEditor,L"KeepEditorEOL", &Global->Opt->EdOpt.KeepEOL, true},
		{FSSF_PRIVATE,       NKeyEditor,L"PersistentBlocks", &Global->Opt->EdOpt.PersistentBlocks, false},
		{FSSF_PRIVATE,       NKeyEditor,L"ReadOnlyLock", &Global->Opt->EdOpt.ReadOnlyLock, 0},
		{FSSF_PRIVATE,       NKeyEditor,L"SaveEditorPos", &Global->Opt->EdOpt.SavePos, true},
		{FSSF_PRIVATE,       NKeyEditor,L"SaveEditorShortPos", &Global->Opt->EdOpt.SaveShortPos, true},
		{FSSF_PRIVATE,       NKeyEditor,L"SearchPickUpWord", &Global->Opt->EdOpt.SearchPickUpWord, false},
		{FSSF_PRIVATE,       NKeyEditor,L"SearchRegexp", &Global->Opt->EdOpt.SearchRegexp, false},
		{FSSF_PRIVATE,       NKeyEditor,L"SearchSelFound", &Global->Opt->EdOpt.SearchSelFound, false},
		{FSSF_PRIVATE,       NKeyEditor,L"SearchCursorAtEnd", &Global->Opt->EdOpt.SearchCursorAtEnd, false},
		{FSSF_PRIVATE,       NKeyEditor,L"ShowKeyBar", &Global->Opt->EdOpt.ShowKeyBar, true},
		{FSSF_PRIVATE,       NKeyEditor,L"ShowScrollBar", &Global->Opt->EdOpt.ShowScrollBar, false},
		{FSSF_PRIVATE,       NKeyEditor,L"ShowTitleBar", &Global->Opt->EdOpt.ShowTitleBar, true},
		{FSSF_PRIVATE,       NKeyEditor,L"ShowWhiteSpace", &Global->Opt->EdOpt.ShowWhiteSpace, 0},
		{FSSF_PRIVATE,       NKeyEditor,L"TabSize", &Global->Opt->EdOpt.TabSize, DefaultTabSize},
		{FSSF_PRIVATE,       NKeyEditor,L"UndoDataSize", &Global->Opt->EdOpt.UndoSize, 100*1024*1024},
		{FSSF_PRIVATE,       NKeyEditor,L"UseExternalEditor", &Global->Opt->EdOpt.UseExternalEditor, false},
		{FSSF_EDITOR,        NKeyEditor,L"WordDiv", &Global->Opt->strWordDiv, WordDiv0},

		{FSSF_PRIVATE,       NKeyHelp,L"ActivateURL", &Global->Opt->HelpURLRules, 1},
		{FSSF_PRIVATE,       NKeyHelp,L"HelpSearchRegexp", &Global->Opt->HelpSearchRegexp, false},

		{FSSF_PRIVATE,       NKeyCommandHistory, L"Count", &Global->Opt->HistoryCount, 1000},
		{FSSF_PRIVATE,       NKeyCommandHistory, L"Lifetime", &Global->Opt->HistoryLifetime, 90},
		{FSSF_PRIVATE,       NKeyDialogHistory, L"Count", &Global->Opt->DialogsHistoryCount, 1000},
		{FSSF_PRIVATE,       NKeyDialogHistory, L"Lifetime", &Global->Opt->DialogsHistoryLifetime, 90},
		{FSSF_PRIVATE,       NKeyFolderHistory, L"Count", &Global->Opt->FoldersHistoryCount, 1000},
		{FSSF_PRIVATE,       NKeyFolderHistory, L"Lifetime", &Global->Opt->FoldersHistoryLifetime, 90},
		{FSSF_PRIVATE,       NKeyViewEditHistory, L"Count", &Global->Opt->ViewHistoryCount, 1000},
		{FSSF_PRIVATE,       NKeyViewEditHistory, L"Lifetime", &Global->Opt->ViewHistoryLifetime, 90},

		{FSSF_PRIVATE,       NKeyInterface,L"DelShowTotal", &Global->Opt->DelOpt.DelShowTotal, false},

		{FSSF_PRIVATE,       NKeyInterface, L"AltF9", &Global->Opt->AltF9, true},
		{FSSF_PRIVATE,       NKeyInterface, L"ClearType", &Global->Opt->ClearType, true},
		{FSSF_PRIVATE,       NKeyInterface, L"CopyShowTotal", &Global->Opt->CMOpt.CopyShowTotal, true},
		{FSSF_PRIVATE,       NKeyInterface, L"CtrlPgUp", &Global->Opt->PgUpChangeDisk, 1},
		{FSSF_PRIVATE,       NKeyInterface, L"CursorSize1", &Global->Opt->CursorSize[0], 15},
		{FSSF_PRIVATE,       NKeyInterface, L"CursorSize2", &Global->Opt->CursorSize[1], 10},
		{FSSF_PRIVATE,       NKeyInterface, L"CursorSize3", &Global->Opt->CursorSize[2], 99},
		{FSSF_PRIVATE,       NKeyInterface, L"CursorSize4", &Global->Opt->CursorSize[3], 99},
		{FSSF_PRIVATE,       NKeyInterface, L"EditorTitleFormat", &Global->Opt->strEditorTitleFormat, L"%Lng %File"},
		{FSSF_PRIVATE,       NKeyInterface, L"FormatNumberSeparators", &Global->Opt->FormatNumberSeparators, 0},
		{FSSF_PRIVATE,       NKeyInterface, L"Mouse", &Global->Opt->Mouse, true},
		{FSSF_PRIVATE,       NKeyInterface, L"SetIcon", &Global->Opt->SetIcon, false},
		{FSSF_PRIVATE,       NKeyInterface, L"SetAdminIcon", &Global->Opt->SetAdminIcon, true},
		{FSSF_PRIVATE,       NKeyInterface, L"ShiftsKeyRules", &Global->Opt->ShiftsKeyRules, true},
		{FSSF_PRIVATE,       NKeyInterface, L"ShowDotsInRoot", &Global->Opt->ShowDotsInRoot, false},
		{FSSF_INTERFACE,     NKeyInterface, L"ShowMenuBar", &Global->Opt->ShowMenuBar, false},
		{FSSF_PRIVATE,       NKeyInterface, L"RedrawTimeout", &Global->Opt->RedrawTimeout, 200},
		{FSSF_PRIVATE,       NKeyInterface, L"TitleAddons", &Global->Opt->strTitleAddons, L"%Ver.%Build %Platform %Admin"},
		{FSSF_PRIVATE,       NKeyInterface, L"UseVk_oem_x", &Global->Opt->UseVk_oem_x, true},
		{FSSF_PRIVATE,       NKeyInterface, L"ViewerTitleFormat", &Global->Opt->strViewerTitleFormat, L"%Lng %File"},

		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"Append", &Global->Opt->AutoComplete.AppendCompletion, false},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"ModalList", &Global->Opt->AutoComplete.ModalList, false},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"ShowList", &Global->Opt->AutoComplete.ShowList, true},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"UseFilesystem", &Global->Opt->AutoComplete.UseFilesystem, 1},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"UseHistory", &Global->Opt->AutoComplete.UseHistory, 1},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"UsePath", &Global->Opt->AutoComplete.UsePath, 1},

		{FSSF_PRIVATE,       NKeyLanguage, L"Main", &Global->Opt->strLanguage, DefaultLanguage},
		{FSSF_PRIVATE,       NKeyLanguage, L"Help", &Global->Opt->strHelpLanguage, DefaultLanguage},

		{FSSF_PRIVATE,       NKeyLayout,L"FullscreenHelp", &Global->Opt->FullScreenHelp, false},
		{FSSF_PRIVATE,       NKeyLayout,L"LeftHeightDecrement", &Global->Opt->LeftHeightDecrement, 0},
		{FSSF_PRIVATE,       NKeyLayout,L"RightHeightDecrement", &Global->Opt->RightHeightDecrement, 0},
		{FSSF_PRIVATE,       NKeyLayout,L"WidthDecrement", &Global->Opt->WidthDecrement, 0},

		{FSSF_PRIVATE,       NKeyKeyMacros,L"CONVFMT", &Global->Opt->Macro.strMacroCONVFMT, L"%.6g"},
		{FSSF_PRIVATE,       NKeyKeyMacros,L"DateFormat", &Global->Opt->Macro.strDateFormat, L"%a %b %d %H:%M:%S %Z %Y"},
		{FSSF_PRIVATE,       NKeyKeyMacros,L"MacroReuseRules", &Global->Opt->Macro.MacroReuseRules, false},

		{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordCtrlDot", &Global->Opt->Macro.strKeyMacroCtrlDot, L"Ctrl."},
		{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordRCtrlDot", &Global->Opt->Macro.strKeyMacroRCtrlDot, L"RCtrl."},
		{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordCtrlShiftDot", &Global->Opt->Macro.strKeyMacroCtrlShiftDot, L"CtrlShift."},
		{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordRCtrlShiftDot", &Global->Opt->Macro.strKeyMacroRCtrlShiftDot, L"RCtrlShift."},

		{FSSF_PRIVATE,       NKeyPanel,L"AutoUpdateLimit", &Global->Opt->AutoUpdateLimit, 0},
		{FSSF_PRIVATE,       NKeyPanel,L"CtrlAltShiftRule", &Global->Opt->PanelCtrlAltShiftRule, 0},
		{FSSF_PRIVATE,       NKeyPanel,L"CtrlFRule", &Global->Opt->PanelCtrlFRule, false},
		{FSSF_PRIVATE,       NKeyPanel,L"Highlight", &Global->Opt->Highlight, true},
		{FSSF_PRIVATE,       NKeyPanel,L"ReverseSort", &Global->Opt->ReverseSort, true},
		{FSSF_PRIVATE,       NKeyPanel,L"RememberLogicalDrives", &Global->Opt->RememberLogicalDrives, false},
		{FSSF_PRIVATE,       NKeyPanel,L"RightClickRule", &Global->Opt->PanelRightClickRule, 2},
		{FSSF_PRIVATE,       NKeyPanel,L"SelectFolders", &Global->Opt->SelectFolders, false},
		{FSSF_PRIVATE,       NKeyPanel,L"ShellRightLeftArrowsRule", &Global->Opt->ShellRightLeftArrowsRule, false},
		{FSSF_PANEL,         NKeyPanel,L"ShowHidden", &Global->Opt->ShowHidden, true},
		{FSSF_PANEL,         NKeyPanel,L"ShortcutAlwaysChdir", &Global->Opt->ShortcutAlwaysChdir, false},
		{FSSF_PRIVATE,       NKeyPanel,L"SortFolderExt", &Global->Opt->SortFolderExt, false},
		{FSSF_PRIVATE,       NKeyPanel,L"RightClickSelect", &Global->Opt->RightClickSelect, false},

		{FSSF_PRIVATE,       NKeyPanelInfo,L"InfoComputerNameFormat", &Global->Opt->InfoPanel.ComputerNameFormat, ComputerNamePhysicalNetBIOS},
		{FSSF_PRIVATE,       NKeyPanelInfo,L"InfoUserNameFormat", &Global->Opt->InfoPanel.UserNameFormat, NameUserPrincipal},
		{FSSF_PRIVATE,       NKeyPanelInfo,L"ShowCDInfo", &Global->Opt->InfoPanel.ShowCDInfo, true},
		{FSSF_PRIVATE,       NKeyPanelInfo,L"ShowPowerStatus", &Global->Opt->InfoPanel.ShowPowerStatus, false},

		{FSSF_PRIVATE,       NKeyPanelLayout,L"ColoredGlobalColumnSeparator", &Global->Opt->HighlightColumnSeparator, true},
		{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"ColumnTitles", &Global->Opt->ShowColumnTitles, true},
		{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"DetailedJunction", &Global->Opt->PanelDetailedJunction, false},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"DoubleGlobalColumnSeparator", &Global->Opt->DoubleGlobalColumnSeparator, false},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"FreeInfo", &Global->Opt->ShowPanelFree, false},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"ScreensNumber", &Global->Opt->ShowScreensNumber, true},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"Scrollbar", &Global->Opt->ShowPanelScrollbar, false},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"ScrollbarMenu", &Global->Opt->ShowMenuScrollbar, true},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"ShowUnknownReparsePoint", &Global->Opt->ShowUnknownReparsePoint, false},
		{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"SortMode", &Global->Opt->ShowSortMode, true},
		{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"StatusLine", &Global->Opt->ShowPanelStatus, true},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"TotalInfo", &Global->Opt->ShowPanelTotals, true},

		{FSSF_PRIVATE,       NKeyPanelLeft,L"CaseSensitiveSort", &Global->Opt->LeftPanel.CaseSensitiveSort, false},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"DirectoriesFirst", &Global->Opt->LeftPanel.DirectoriesFirst, true},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"NumericSort", &Global->Opt->LeftPanel.NumericSort, false},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"SelectedFirst", &Global->Opt->LeftPanel.SelectedFirst, false},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"ShortNames", &Global->Opt->LeftPanel.ShowShortNames, false},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"SortGroups", &Global->Opt->LeftPanel.SortGroups, false},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"SortMode", &Global->Opt->LeftPanel.SortMode, 1},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"SortOrder", &Global->Opt->LeftPanel.SortOrder, 1},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"Type", &Global->Opt->LeftPanel.Type, 0},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"ViewMode", &Global->Opt->LeftPanel.ViewMode, 2},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"Visible", &Global->Opt->LeftPanel.Visible, true},

		{FSSF_PRIVATE,       NKeyPanelRight,L"CaseSensitiveSort", &Global->Opt->RightPanel.CaseSensitiveSort, false},
		{FSSF_PRIVATE,       NKeyPanelRight,L"DirectoriesFirst", &Global->Opt->RightPanel.DirectoriesFirst, true},
		{FSSF_PRIVATE,       NKeyPanelRight,L"NumericSort", &Global->Opt->RightPanel.NumericSort, false},
		{FSSF_PRIVATE,       NKeyPanelRight,L"SelectedFirst", &Global->Opt->RightPanel.SelectedFirst, false},
		{FSSF_PRIVATE,       NKeyPanelRight,L"ShortNames", &Global->Opt->RightPanel.ShowShortNames, false},
		{FSSF_PRIVATE,       NKeyPanelRight,L"SortGroups", &Global->Opt->RightPanel.SortGroups, false},
		{FSSF_PRIVATE,       NKeyPanelRight,L"SortMode", &Global->Opt->RightPanel.SortMode, 1},
		{FSSF_PRIVATE,       NKeyPanelRight,L"SortOrder", &Global->Opt->RightPanel.SortOrder, 1},
		{FSSF_PRIVATE,       NKeyPanelRight,L"Type", &Global->Opt->RightPanel.Type, 0},
		{FSSF_PRIVATE,       NKeyPanelRight,L"ViewMode", &Global->Opt->RightPanel.ViewMode, 2},
		{FSSF_PRIVATE,       NKeyPanelRight,L"Visible", &Global->Opt->RightPanel.Visible, true},

		{FSSF_PRIVATE,       NKeyPanelTree,L"AutoChangeFolder", &Global->Opt->Tree.AutoChangeFolder, false},
		{FSSF_PRIVATE,       NKeyPanelTree,L"MinTreeCount", &Global->Opt->Tree.MinTreeCount, 4},
		{FSSF_PRIVATE,       NKeyPanelTree,L"TreeFileAttr", &Global->Opt->Tree.TreeFileAttr, FILE_ATTRIBUTE_HIDDEN},
	#if defined(TREEFILE_PROJECT)
		{FSSF_PRIVATE,       NKeyPanelTree,L"CDDisk", &Global->Opt->Tree.CDDisk, 2},
		{FSSF_PRIVATE,       NKeyPanelTree,L"CDDiskTemplate,0", &Global->Opt->Tree.strCDDisk, constCDDiskTemplate},
		{FSSF_PRIVATE,       NKeyPanelTree,L"ExceptPath", &Global->Opt->Tree.strExceptPath, L""},
		{FSSF_PRIVATE,       NKeyPanelTree,L"LocalDisk", &Global->Opt->Tree.LocalDisk, 2},
		{FSSF_PRIVATE,       NKeyPanelTree,L"LocalDiskTemplate", &Global->Opt->Tree.strLocalDisk, constLocalDiskTemplate},
		{FSSF_PRIVATE,       NKeyPanelTree,L"NetDisk", &Global->Opt->Tree.NetDisk, 2},
		{FSSF_PRIVATE,       NKeyPanelTree,L"NetPath", &Global->Opt->Tree.NetPath, 2},
		{FSSF_PRIVATE,       NKeyPanelTree,L"NetDiskTemplate", &Global->Opt->Tree.strNetDisk, constNetDiskTemplate},
		{FSSF_PRIVATE,       NKeyPanelTree,L"NetPathTemplate", &Global->Opt->Tree.strNetPath, constNetPathTemplate},
		{FSSF_PRIVATE,       NKeyPanelTree,L"RemovableDisk", &Global->Opt->Tree.RemovableDisk, 2},
		{FSSF_PRIVATE,       NKeyPanelTree,L"RemovableDiskTemplate,", &Global->Opt->Tree.strRemovableDisk, constRemovableDiskTemplate},
		{FSSF_PRIVATE,       NKeyPanelTree,L"SaveLocalPath", &Global->Opt->Tree.strSaveLocalPath, L""},
		{FSSF_PRIVATE,       NKeyPanelTree,L"SaveNetPath", &Global->Opt->Tree.strSaveNetPath, L""},
	#endif
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"EvenIfOnlyOnePlugin", &Global->Opt->PluginConfirm.EvenIfOnlyOnePlugin, false},
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"OpenFilePlugin", &Global->Opt->PluginConfirm.OpenFilePlugin, 0},
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"Prefix", &Global->Opt->PluginConfirm.Prefix, false},
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"SetFindList", &Global->Opt->PluginConfirm.SetFindList, false},
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"StandardAssociation", &Global->Opt->PluginConfirm.StandardAssociation, false},

		{FSSF_PRIVATE,       NKeyPolicies,L"DisabledOptions", &Global->Opt->Policies.DisabledOptions, 0},
		{FSSF_PRIVATE,       NKeyPolicies,L"ShowHiddenDrives", &Global->Opt->Policies.ShowHiddenDrives, true},

		{FSSF_PRIVATE,       NKeyScreen, L"Clock", &Global->Opt->Clock, true},
		{FSSF_PRIVATE,       NKeyScreen, L"DeltaX", &Global->Opt->ScrSize.DeltaX, 0},
		{FSSF_PRIVATE,       NKeyScreen, L"DeltaY", &Global->Opt->ScrSize.DeltaY, 0},
		{FSSF_SCREEN,        NKeyScreen, L"KeyBar", &Global->Opt->ShowKeyBar, true},
		{FSSF_PRIVATE,       NKeyScreen, L"ScreenSaver", &Global->Opt->ScreenSaver, false},
		{FSSF_PRIVATE,       NKeyScreen, L"ScreenSaverTime", &Global->Opt->ScreenSaverTime, 5},
		{FSSF_PRIVATE,       NKeyScreen, L"ViewerEditorClock", &Global->Opt->ViewerEditorClock, true},

		{FSSF_PRIVATE,       NKeySystem,L"AllCtrlAltShiftRule", &Global->Opt->AllCtrlAltShiftRule, 0x0000FFFF},
		{FSSF_PRIVATE,       NKeySystem,L"AutoSaveSetup", &Global->Opt->AutoSaveSetup, false},
		{FSSF_PRIVATE,       NKeySystem,L"AutoUpdateRemoteDrive", &Global->Opt->AutoUpdateRemoteDrive, true},
		{FSSF_PRIVATE,       NKeySystem,L"BoxSymbols", &Global->Opt->strBoxSymbols, _BoxSymbols},
		{FSSF_PRIVATE,       NKeySystem,L"CASRule", &Global->Opt->CASRule, -1},
		{FSSF_PRIVATE,       NKeySystem,L"CloseCDGate", &Global->Opt->CloseCDGate, true},
		{FSSF_PRIVATE,       NKeySystem,L"CmdHistoryRule", &Global->Opt->CmdHistoryRule, false},
		{FSSF_PRIVATE,       NKeySystem,L"CollectFiles", &Global->Opt->FindOpt.CollectFiles, true},
		{FSSF_PRIVATE,       NKeySystem,L"ConsoleDetachKey", &Global->Opt->ConsoleDetachKey, L"CtrlShiftTab"},
		{FSSF_PRIVATE,       NKeySystem,L"CopyBufferSize", &Global->Opt->CMOpt.BufferSize, 0},
		{FSSF_SYSTEM,        NKeySystem,L"CopyOpened", &Global->Opt->CMOpt.CopyOpened, true},
		{FSSF_PRIVATE,       NKeySystem,L"CopyTimeRule",  &Global->Opt->CMOpt.CopyTimeRule, 3},
		{FSSF_PRIVATE,       NKeySystem,L"CopySecurityOptions", &Global->Opt->CMOpt.CopySecurityOptions, 0},
		{FSSF_PRIVATE,       NKeySystem,L"CreateUppercaseFolders", &Global->Opt->CreateUppercaseFolders, false},
		{FSSF_SYSTEM,        NKeySystem,L"DeleteToRecycleBin", &Global->Opt->DeleteToRecycleBin, true},
		{FSSF_PRIVATE,       NKeySystem,L"DeleteToRecycleBinKillLink", &Global->Opt->DeleteToRecycleBinKillLink, true},
		{FSSF_PRIVATE,       NKeySystem,L"DelThreadPriority", &Global->Opt->DelThreadPriority, THREAD_PRIORITY_NORMAL},
		{FSSF_PRIVATE,       NKeySystem,L"DriveDisconnectMode", &Global->Opt->ChangeDriveDisconnectMode, true},
		{FSSF_PRIVATE,       NKeySystem,L"DriveMenuMode", &Global->Opt->ChangeDriveMode, DRIVE_SHOW_TYPE|DRIVE_SHOW_PLUGINS|DRIVE_SHOW_SIZE_FLOAT|DRIVE_SHOW_CDROM},
		{FSSF_PRIVATE,       NKeySystem,L"ElevationMode", &Global->Opt->StoredElevationMode, -1},
		{FSSF_PRIVATE,       NKeySystem,L"ExcludeCmdHistory", &Global->Opt->ExcludeCmdHistory, 0},
		{FSSF_PRIVATE,       NKeySystem,L"FileSearchMode", &Global->Opt->FindOpt.FileSearchMode, FINDAREA_FROM_CURRENT},
		{FSSF_PRIVATE,       NKeySystem,L"FindAlternateStreams", &Global->Opt->FindOpt.FindAlternateStreams,0,},
		{FSSF_PRIVATE,       NKeySystem,L"FindCodePage", &Global->Opt->FindCodePage, CP_DEFAULT},
		{FSSF_PRIVATE,       NKeySystem,L"FindFolders", &Global->Opt->FindOpt.FindFolders, true},
		{FSSF_PRIVATE,       NKeySystem,L"FindSymLinks", &Global->Opt->FindOpt.FindSymLinks, true},
		{FSSF_PRIVATE,       NKeySystem,L"FlagPosixSemantics", &Global->Opt->FlagPosixSemantics, true},
		{FSSF_PRIVATE,       NKeySystem,L"FolderInfo", &Global->Opt->InfoPanel.strFolderInfoFiles, L"DirInfo,File_Id.diz,Descript.ion,ReadMe.*,Read.Me"},
		{FSSF_PRIVATE,       NKeySystem,L"MsWheelDelta", &Global->Opt->MsWheelDelta, 1},
		{FSSF_PRIVATE,       NKeySystem,L"MsWheelDeltaEdit", &Global->Opt->MsWheelDeltaEdit, 1},
		{FSSF_PRIVATE,       NKeySystem,L"MsWheelDeltaHelp", &Global->Opt->MsWheelDeltaHelp, 1},
		{FSSF_PRIVATE,       NKeySystem,L"MsWheelDeltaView", &Global->Opt->MsWheelDeltaView, 1},
		{FSSF_PRIVATE,       NKeySystem,L"MsHWheelDelta", &Global->Opt->MsHWheelDelta, 1},
		{FSSF_PRIVATE,       NKeySystem,L"MsHWheelDeltaEdit", &Global->Opt->MsHWheelDeltaEdit, 1},
		{FSSF_PRIVATE,       NKeySystem,L"MsHWheelDeltaView", &Global->Opt->MsHWheelDeltaView, 1},
		{FSSF_PRIVATE,       NKeySystem,L"MultiCopy", &Global->Opt->CMOpt.MultiCopy, false},
		{FSSF_PRIVATE,       NKeySystem,L"MultiMakeDir", &Global->Opt->MultiMakeDir, false},
	#ifndef NO_WRAPPER
		{FSSF_PRIVATE,       NKeySystem,L"OEMPluginsSupport",  &Global->Opt->LoadPlug.OEMPluginsSupport, true},
	#endif // NO_WRAPPER
		{FSSF_SYSTEM,        NKeySystem,L"PluginMaxReadData", &Global->Opt->PluginMaxReadData, 0x20000},
		{FSSF_PRIVATE,       NKeySystem,L"QuotedName", &Global->Opt->QuotedName, QUOTEDNAME_INSERT},
		{FSSF_PRIVATE,       NKeySystem,L"QuotedSymbols", &Global->Opt->strQuotedSymbols, L" &()[]{}^=;!'+,`\xA0"},
		{FSSF_PRIVATE,       NKeySystem,L"SaveHistory", &Global->Opt->SaveHistory, true},
		{FSSF_PRIVATE,       NKeySystem,L"SaveFoldersHistory", &Global->Opt->SaveFoldersHistory, true},
		{FSSF_PRIVATE,       NKeySystem,L"SaveViewHistory", &Global->Opt->SaveViewHistory, true},
		{FSSF_SYSTEM,        NKeySystem,L"ScanJunction", &Global->Opt->ScanJunction, true},
		{FSSF_PRIVATE,       NKeySystem,L"ScanSymlinks",  &Global->Opt->LoadPlug.ScanSymlinks, true},
		{FSSF_PRIVATE,       NKeySystem,L"SearchInFirstSize", &Global->Opt->FindOpt.strSearchInFirstSize, L""},
		{FSSF_PRIVATE,       NKeySystem,L"SearchOutFormat", &Global->Opt->FindOpt.strSearchOutFormat, L"D,S,A"},
		{FSSF_PRIVATE,       NKeySystem,L"SearchOutFormatWidth", &Global->Opt->FindOpt.strSearchOutFormatWidth, L"14,13,0"},
		{FSSF_PRIVATE,       NKeySystem,L"SetAttrFolderRules", &Global->Opt->SetAttrFolderRules, true},
		{FSSF_PRIVATE,       NKeySystem,L"ShowCheckingFile", &Global->Opt->ShowCheckingFile, false},
		{FSSF_PRIVATE,       NKeySystem,L"ShowStatusInfo", &Global->Opt->InfoPanel.strShowStatusInfo, L""},
		{FSSF_PRIVATE,       NKeySystem,L"SilentLoadPlugin",  &Global->Opt->LoadPlug.SilentLoadPlugin, false},
		{FSSF_PRIVATE,       NKeySystem,L"SmartFolderMonitor",  &Global->Opt->SmartFolderMonitor, false},
		{FSSF_PRIVATE,       NKeySystem,L"SubstNameRule", &Global->Opt->SubstNameRule, 2},
		{FSSF_PRIVATE,       NKeySystem,L"SubstPluginPrefix", &Global->Opt->SubstPluginPrefix, false},
		{FSSF_PRIVATE,       NKeySystem,L"UpdateEnvironment", &Global->Opt->UpdateEnvironment,0,},
		{FSSF_PRIVATE,       NKeySystem,L"UseFilterInSearch", &Global->Opt->FindOpt.UseFilter,0,},
		{FSSF_PRIVATE,       NKeySystem,L"UseRegisteredTypes", &Global->Opt->UseRegisteredTypes, true},
		{FSSF_PRIVATE,       NKeySystem,L"UseSystemCopy", &Global->Opt->CMOpt.UseSystemCopy, true},
		{FSSF_PRIVATE,       NKeySystem,L"WindowMode", &Global->Opt->StoredWindowMode, false},
		{FSSF_PRIVATE,       NKeySystem,L"WipeSymbol", &Global->Opt->WipeSymbol, 0},

		{FSSF_PRIVATE,       NKeySystemKnownIDs,L"EMenu", &Global->Opt->KnownIDs.EmenuGuidStr, L"742910F1-02ED-4542-851F-DEE37C2E13B2"},
		{FSSF_PRIVATE,       NKeySystemKnownIDs,L"Network", &Global->Opt->KnownIDs.NetworkGuidStr, L"773B5051-7C5F-4920-A201-68051C4176A4"},

		{FSSF_PRIVATE,       NKeySystemNowell,L"MoveRO", &Global->Opt->Nowell.MoveRO, true},

		{FSSF_PRIVATE,       NKeySystemException,L"FarEventSvc", &Global->Opt->strExceptEventSvc, L""},
		{FSSF_PRIVATE,       NKeySystemException,L"Used", &Global->Opt->ExceptUsed, false},

		{FSSF_PRIVATE,       NKeySystemExecutor,L"~", &Global->Opt->Exec.strHomeDir, L"%FARHOME%"},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"BatchType", &Global->Opt->Exec.strExecuteBatchType, constBatchExt},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"ExcludeCmds", &Global->Opt->Exec.strExcludeCmds, L""},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"FullTitle", &Global->Opt->Exec.ExecuteFullTitle, false},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"RestoreCP", &Global->Opt->Exec.RestoreCPAfterExecute, true},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"SilentExternal", &Global->Opt->Exec.ExecuteSilentExternal, false},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"UseAppPath", &Global->Opt->Exec.ExecuteUseAppPath, true},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"UseHomeDir", &Global->Opt->Exec.UseHomeDir, true},

		{FSSF_PRIVATE,       NKeyViewer,L"AutoDetectCodePage", &Global->Opt->ViOpt.AutoDetectCodePage, true},
		{FSSF_PRIVATE,       NKeyViewer,L"DefaultCodePage", &Global->Opt->ViOpt.DefaultCodePage, GetACP()},
		{FSSF_PRIVATE,       NKeyViewer,L"ExternalViewerName", &Global->Opt->strExternalViewer, L""},
		{FSSF_PRIVATE,       NKeyViewer,L"IsWrap", &Global->Opt->ViOpt.ViewerIsWrap, true},
		{FSSF_PRIVATE,       NKeyViewer,L"MaxLineSize", &Global->Opt->ViOpt.MaxLineSize, ViewerOptions::eDefLineSize},
		{FSSF_PRIVATE,       NKeyViewer,L"PersistentBlocks", &Global->Opt->ViOpt.PersistentBlocks, false},
		{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerPos", &Global->Opt->ViOpt.SavePos, true},
		{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerShortPos", &Global->Opt->ViOpt.SaveShortPos, true},
		{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerCodepage", &Global->Opt->ViOpt.SaveCodepage, true},
		{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerWrapMode", &Global->Opt->ViOpt.SaveWrapMode, false},
		{FSSF_PRIVATE,       NKeyViewer,L"SearchEditFocus", &Global->Opt->ViOpt.SearchEditFocus, false},
		{FSSF_PRIVATE,       NKeyViewer,L"SearchRegexp", &Global->Opt->ViOpt.SearchRegexp, false},
		{FSSF_PRIVATE,       NKeyViewer,L"ShowArrows", &Global->Opt->ViOpt.ShowArrows, true},
		{FSSF_PRIVATE,       NKeyViewer,L"ShowKeyBar", &Global->Opt->ViOpt.ShowKeyBar, true},
		{FSSF_PRIVATE,       NKeyViewer,L"ShowTitleBar", &Global->Opt->ViOpt.ShowTitleBar, true},
		{FSSF_PRIVATE,       NKeyViewer,L"ShowScrollbar", &Global->Opt->ViOpt.ShowScrollbar, false},
		{FSSF_PRIVATE,       NKeyViewer,L"TabSize", &Global->Opt->ViOpt.TabSize, DefaultTabSize},
		{FSSF_PRIVATE,       NKeyViewer,L"UseExternalViewer", &Global->Opt->ViOpt.UseExternalViewer, false},
		{FSSF_PRIVATE,       NKeyViewer,L"Visible0x00", &Global->Opt->ViOpt.Visible0x00, false},
		{FSSF_PRIVATE,       NKeyViewer,L"Wrap", &Global->Opt->ViOpt.ViewerWrap, false},
		{FSSF_PRIVATE,       NKeyViewer,L"ZeroChar", &Global->Opt->ViOpt.ZeroChar, 0x00B7}, // middle dot

		{FSSF_PRIVATE,       NKeyVMenu,L"LBtnClick", &Global->Opt->VMenu.LBtnClick, VMENUCLICK_CANCEL},
		{FSSF_PRIVATE,       NKeyVMenu,L"MBtnClick", &Global->Opt->VMenu.MBtnClick, VMENUCLICK_APPLY},
		{FSSF_PRIVATE,       NKeyVMenu,L"RBtnClick", &Global->Opt->VMenu.RBtnClick, VMENUCLICK_CANCEL},

		{FSSF_PRIVATE,       NKeyXLat,L"Flags", &Global->Opt->XLat.Flags, XLAT_SWITCHKEYBLAYOUT|XLAT_CONVERTALLCMDLINE},
		{FSSF_PRIVATE,       NKeyXLat,L"Layouts", &Global->Opt->XLat.strLayouts, L""},
		{FSSF_PRIVATE,       NKeyXLat,L"Rules1", &Global->Opt->XLat.Rules[0], L""},
		{FSSF_PRIVATE,       NKeyXLat,L"Rules2", &Global->Opt->XLat.Rules[1], L""},
		{FSSF_PRIVATE,       NKeyXLat,L"Rules3", &Global->Opt->XLat.Rules[2], L""},
		{FSSF_PRIVATE,       NKeyXLat,L"Table1", &Global->Opt->XLat.Table[0], L""},
		{FSSF_PRIVATE,       NKeyXLat,L"Table2", &Global->Opt->XLat.Table[1], L""},
		{FSSF_PRIVATE,       NKeyXLat,L"WordDivForXlat", &Global->Opt->XLat.strWordDivForXlat, WordDivForXlat0},
	};
	Config[cfg_roaming].first = Global->Db->GeneralCfg().get();
	Config[cfg_roaming].second.assign(_CFG, ARRAYSIZE(_CFG));

}

void Options::InitLocalCFG()
{
	static FARConfigItem _CFG[] =
	{
		{FSSF_PRIVATE,       NKeyPanelLeft,L"CurFile", &Global->Opt->LeftPanel.CurFile, L""},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"Folder", &Global->Opt->LeftPanel.Folder, L""},

		{FSSF_PRIVATE,       NKeyPanelRight,L"CurFile", &Global->Opt->RightPanel.CurFile, L""},
		{FSSF_PRIVATE,       NKeyPanelRight,L"Folder", &Global->Opt->RightPanel.Folder, L""},

		{FSSF_PRIVATE,       NKeyPanel,L"LeftFocus", &Global->Opt->LeftFocus, true},

	};
	Config[cfg_local].first = Global->Db->LocalGeneralCfg().get();
	Config[cfg_local].second.assign(_CFG, ARRAYSIZE(_CFG));
}

bool Options::GetConfigValue(const wchar_t *Key, const wchar_t *Name, string &strValue)
{
	// TODO Use local too?
	return std::any_of(CONST_RANGE(Config[cfg_roaming].second, i)->bool
	{
		if (!StrCmpI(i.KeyName, Key) && !StrCmpI(i.ValName, Name))
		{
			strValue = i.Value->toString();
			return true;
		}
		return false;
	});
}

bool Options::GetConfigValue(size_t Root, const wchar_t* Name, Option::OptionType& Type, Option*& Data)
{
	// TODO Use local too?
	return Root != FSSF_PRIVATE? std::any_of(CONST_RANGE(Config[cfg_roaming].second, i)->bool
	{
		if(Root == i.ApiRoot && !StrCmpI(i.ValName, Name))
		{
			Type = i.Value->getType();
			Data = i.Value;
			return true;
		}
		return false;
	}) : false;
}

void Options::InitConfig()
{
	if(Config.empty())
	{
		Config.resize(2);
		InitRoamingCFG();
		InitLocalCFG();
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

	GetPrivateProfileString(L"General", L"DefaultLanguage", L"English", DefaultLanguage, ARRAYSIZE(DefaultLanguage), Global->g_strFarINI.CPtr());

	std::for_each(CONST_RANGE(Config, i)
	{
		std::for_each(RANGE(i.second, j)
		{
			j.Value->ReceiveValue(i.first, j.KeyName, j.ValName, &j.Default);
		});
	});

	/* <ПОСТПРОЦЕССЫ> *************************************************** */

	Palette.Load();
	GlobalUserMenuDir.ReleaseBuffer(GetPrivateProfileString(L"General", L"GlobalUserMenuDir", Global->g_strFarPath.CPtr(), GlobalUserMenuDir.GetBuffer(NT_MAX_PATH), NT_MAX_PATH, Global->g_strFarINI.CPtr()));
	apiExpandEnvironmentStrings(GlobalUserMenuDir, GlobalUserMenuDir);
	ConvertNameToFull(GlobalUserMenuDir,GlobalUserMenuDir);
	AddEndSlash(GlobalUserMenuDir);

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
			auto DestList(StringToList(XLat.strLayouts, STLF_UNIQUE));
			size_t I=0;

			FOR_CONST_RANGE(DestList, i)
			{
				DWORD res=(DWORD)wcstoul(i->CPtr(), &endptr, 16);
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
	std::for_each(CONST_RANGE(Config, i)
	{
		std::for_each(RANGE(i.second, j)
		{
			j.Value->MakeUnchanged();
		});
	});
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
	LeftPanel.Visible=LeftPanelPtr->IsVisible() != 0;
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
		LeftPanel.SelectedFirst=LeftPanelPtr->GetSelectedFirstMode() != 0;
		LeftPanel.DirectoriesFirst=LeftPanelPtr->GetDirectoriesFirst() != 0;
	}

	LeftPanel.Folder = LeftPanelPtr->GetCurDir();
	string strTemp1, strTemp2;
	LeftPanelPtr->GetCurBaseName(strTemp1, strTemp2);
	LeftPanel.CurFile = strTemp1;
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
		RightPanel.SelectedFirst=RightPanelPtr->GetSelectedFirstMode() != 0;
		RightPanel.DirectoriesFirst=RightPanelPtr->GetDirectoriesFirst() != 0;
	}

	RightPanel.Folder = RightPanelPtr->GetCurDir();
	RightPanelPtr->GetCurBaseName(strTemp1, strTemp2);
	RightPanel.CurFile = strTemp1;

	LeftFocus = Global->CtrlObject->Cp()->ActivePanel == LeftPanelPtr;

	Global->CtrlObject->HiFiles->SaveHiData();
	/* *************************************************** </ПРЕПРОЦЕССЫ> */

	Palette.Save();

	std::for_each(CONST_RANGE(Config, i)
	{
		i.first->BeginTransaction();
		std::for_each(CONST_RANGE(i.second, j)
		{
			j.Value->StoreValue(i.first, j.KeyName, j.ValName);
		});
		i.first->EndTransaction();
	});

	/* <ПОСТПРОЦЕССЫ> *************************************************** */
	FileFilter::SaveFilters();
	FileList::SavePanelModes();

	if (Ask)
		Global->CtrlObject->Macro.SaveMacros();

	/* *************************************************** </ПОСТПРОЦЕССЫ> */
}

intptr_t Options::AdvancedConfigDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	switch (Msg)
	{
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

						string HelpTopic = string(Config[CurrentConfig].second[ListInfo.SelectPos].KeyName) + L"." + Config[CurrentConfig].second[ListInfo.SelectPos].ValName;
						Help hlp(HelpTopic, nullptr, FHELP_NOSHOWERROR);
						if (hlp.GetError())
						{
							HelpTopic = string(Config[CurrentConfig].second[ListInfo.SelectPos].KeyName) + L"Settings";
							Help hlp1(HelpTopic, nullptr, FHELP_NOSHOWERROR);
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
				case KEY_RCTRLH:
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
		// 0 == "OK", 1 == "Reset"
		if (Param1 == 0 || Param1 == 1)
		{
			FarListInfo ListInfo = {sizeof(ListInfo)};
			Dlg->SendMessage(DM_LISTINFO, 0, &ListInfo);

			if (Config[CurrentConfig].second[ListInfo.SelectPos].Edit(Param1 != 0))
			{
				Dlg->SendMessage(DM_ENABLEREDRAW, 0 , 0);
				FarListUpdate flu = {sizeof(flu), ListInfo.SelectPos};
				flu.Item = Config[CurrentConfig].second[ListInfo.SelectPos].MakeListItem();
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

bool Options::AdvancedConfig(farconfig_mode Mode)
{
	CurrentConfig = Mode;

	int DlgWidth = std::max(ScrX-4, 60), DlgHeight = std::max(ScrY-2, 20);
	FarDialogItem AdvancedConfigDlgData[]=
	{
		{DI_LISTBOX,3,1,DlgWidth-4,DlgHeight-2,0,nullptr,nullptr,DIF_NONE,nullptr},
	};
	MakeDialogItemsEx(AdvancedConfigDlgData,AdvancedConfigDlg);

	std::vector<FarListItem> items(Config[CurrentConfig].second.size());
	std::transform(Config[CurrentConfig].second.begin(), Config[CurrentConfig].second.end(), items.begin(), [](VALUE_TYPE(Config[CurrentConfig].second)& i)->FarListItem
	{
		return i.MakeListItem();
	});

	FarList Items={sizeof(FarList), items.size(), items.data()};

	AdvancedConfigDlg[0].ListItems = &Items;

	Dialog Dlg(this, &Options::AdvancedConfigDlgProc, nullptr, AdvancedConfigDlg,ARRAYSIZE(AdvancedConfigDlg));
	Dlg.SetHelp(L"FarConfig");
	Dlg.SetPosition(-1, -1, DlgWidth, DlgHeight);
	Dlg.SetId(AdvancedConfigId);
	Dlg.Process();
	return true;
}
