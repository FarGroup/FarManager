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
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "help.hpp"
#include "filefilter.hpp"
#include "findfile.hpp"
#include "hilight.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "stddlg.hpp"
#include "pathmix.hpp"
#include "panelmix.hpp"
#include "strmix.hpp"
#include "FarDlgBuilder.hpp"
#include "elevation.hpp"
#include "configdb.hpp"
#include "KnownGuids.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "DlgGuid.hpp"
#include "hmenu.hpp"
#include "usermenu.hpp"
#include "filetype.hpp"
#include "shortcuts.hpp"
#include "plist.hpp"
#include "hotplug.hpp"
#include "setcolor.hpp"
#include "lang.hpp"
#include "language.hpp"
#include "plugins.hpp"
#include "manager.hpp"
#include "xlat.hpp"
#include "panelctype.hpp"
#include "diskmenu.hpp"
#include "string_utils.hpp"
#include "cvtname.hpp"
#include "filemasks.hpp"
#include "RegExp.hpp"

static const size_t predefined_panel_modes_count = 10;

// Стандартный набор разделителей
static const wchar_t WordDiv0[] = L"~!%^&*()+|{}:\"<>?`-=\\[];',./";

// Стандартный набор разделителей для функции Xlat
static const wchar_t WordDivForXlat0[] = L" \t!#$%^&*()+|=\\/@?";

static const int DefaultTabSize = 8;


#if defined(TREEFILE_PROJECT)
static const wchar_t constLocalDiskTemplate[] = L"LD.%D.%SN.tree";
static const wchar_t constNetDiskTemplate[] = L"ND.%D.%SN.tree";
static const wchar_t constNetPathTemplate[] = L"NP.%SR.%SH.tree";
static const wchar_t constRemovableDiskTemplate[] = L"RD.%SN.tree";
static const wchar_t constCDDiskTemplate[] = L"CD.%L.%SN.tree";
#endif

static const wchar_t NKeyScreen[] = L"Screen";
static const wchar_t NKeyCmdline[] = L"Cmdline";
static const wchar_t NKeyInterface[] = L"Interface";
static const wchar_t NKeyInterfaceCompletion[] = L"Interface.Completion";
static const wchar_t NKeyViewer[] = L"Viewer";
static const wchar_t NKeyDialog[] = L"Dialog";
static const wchar_t NKeyEditor[] = L"Editor";
static const wchar_t NKeyXLat[] = L"XLat";
static const wchar_t NKeySystem[] = L"System";
static const wchar_t NKeySystemException[] = L"System.Exception";
static const wchar_t NKeySystemKnownIDs[] = L"System.KnownIDs";
static const wchar_t NKeySystemExecutor[] = L"System.Executor";
static const wchar_t NKeySystemNowell[] = L"System.Nowell";
static const wchar_t NKeyHelp[] = L"Help";
static const wchar_t NKeyLanguage[] = L"Language";
static const wchar_t NKeyConfirmations[] = L"Confirmations";
static const wchar_t NKeyPluginConfirmations[] = L"PluginConfirmations";
static const wchar_t NKeyPanel[] = L"Panel";
static const wchar_t NKeyPanelLeft[] = L"Panel.Left";
static const wchar_t NKeyPanelRight[] = L"Panel.Right";
static const wchar_t NKeyPanelLayout[] = L"Panel.Layout";
static const wchar_t NKeyPanelTree[] = L"Panel.Tree";
static const wchar_t NKeyPanelInfo[] = L"Panel.Info";
static const wchar_t NKeyLayout[] = L"Layout";
static const wchar_t NKeyDescriptions[] = L"Descriptions";
static const wchar_t NKeyKeyMacros[] = L"Macros";
static const wchar_t NKeyPolicies[] = L"Policies";
static const wchar_t NKeyCodePages[] = L"CodePages";
static const wchar_t NKeyVMenu[] = L"VMenu";
static const wchar_t NKeyCommandHistory[] = L"History.CommandHistory";
static const wchar_t NKeyViewEditHistory[] = L"History.ViewEditHistory";
static const wchar_t NKeyFolderHistory[] = L"History.FolderHistory";
static const wchar_t NKeyDialogHistory[] = L"History.DialogHistory";

static size_t DisplayModeToReal(size_t Mode)
{
	return Mode < predefined_panel_modes_count? (Mode == 9? 0 : Mode + 1) : Mode - 1;
};

static size_t RealModeToDisplay(size_t Mode)
{
	return Mode < predefined_panel_modes_count? (Mode == 0? 9 : Mode - 1) : Mode + 1;
};

void Options::SystemSettings()
{
	DialogBuilder Builder(lng::MConfigSystemTitle, L"SystemSettings");

	Builder.AddCheckbox(lng::MConfigRecycleBin, DeleteToRecycleBin);
	Builder.AddCheckbox(lng::MConfigSystemCopy, CMOpt.UseSystemCopy);
	Builder.AddCheckbox(lng::MConfigCopySharing, CMOpt.CopyOpened);
	Builder.AddCheckbox(lng::MConfigScanJunction, ScanJunction);
	Builder.AddCheckbox(lng::MConfigSmartFolderMonitor, SmartFolderMonitor);

	Builder.AddCheckbox(lng::MConfigSaveHistory, SaveHistory);
	Builder.AddCheckbox(lng::MConfigSaveFoldersHistory, SaveFoldersHistory);
	Builder.AddCheckbox(lng::MConfigSaveViewHistory, SaveViewHistory);
	Builder.AddCheckbox(lng::MConfigRegisteredTypes, UseRegisteredTypes);
	Builder.AddCheckbox(lng::MConfigCloseCDGate, CloseCDGate);
	Builder.AddCheckbox(lng::MConfigUpdateEnvironment, UpdateEnvironment);
	Builder.AddText(lng::MConfigElevation);
	Builder.AddCheckbox(lng::MConfigElevationModify, StoredElevationMode, ELEVATION_MODIFY_REQUEST)->Indent(4);
	Builder.AddCheckbox(lng::MConfigElevationRead, StoredElevationMode, ELEVATION_READ_REQUEST)->Indent(4);
	Builder.AddCheckbox(lng::MConfigElevationUsePrivileges, StoredElevationMode, ELEVATION_USE_PRIVILEGES)->Indent(4);
	Builder.AddCheckbox(lng::MConfigAutoSave, AutoSaveSetup);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		ElevationMode = StoredElevationMode;
	}
}

void Options::PanelSettings()
{
	DialogBuilder Builder(lng::MConfigPanelTitle, L"PanelSettings");
	BOOL AutoUpdate = AutoUpdateLimit;

	Builder.AddCheckbox(lng::MConfigHidden, ShowHidden);
	Builder.AddCheckbox(lng::MConfigHighlight, Highlight);
	Builder.AddCheckbox(lng::MConfigSelectFolders, SelectFolders);
	Builder.AddCheckbox(lng::MConfigRightClickSelect, RightClickSelect);
	Builder.AddCheckbox(lng::MConfigSortFolderExt, SortFolderExt);
	Builder.AddCheckbox(lng::MConfigReverseSort, ReverseSort);

	DialogItemEx *AutoUpdateEnabled = Builder.AddCheckbox(lng::MConfigAutoUpdateLimit, &AutoUpdate);
	DialogItemEx *AutoUpdateLimitItem = Builder.AddIntEditField(AutoUpdateLimit, 6);
	Builder.LinkFlags(AutoUpdateEnabled, AutoUpdateLimitItem, DIF_DISABLE, false);
	DialogItemEx *AutoUpdateTextItem = Builder.AddTextBefore(AutoUpdateLimitItem, lng::MConfigAutoUpdateLimit2);
	AutoUpdateLimitItem->Indent(4);
	AutoUpdateTextItem->Indent(4);
	Builder.AddCheckbox(lng::MConfigAutoUpdateRemoteDrive, AutoUpdateRemoteDrive);

	Builder.AddSeparator();
	Builder.AddCheckbox(lng::MConfigShowColumns, ShowColumnTitles);
	Builder.AddCheckbox(lng::MConfigShowStatus, ShowPanelStatus);
	Builder.AddCheckbox(lng::MConfigDetailedJunction, PanelDetailedJunction);
	Builder.AddCheckbox(lng::MConfigShowTotal, ShowPanelTotals);
	Builder.AddCheckbox(lng::MConfigShowFree, ShowPanelFree);
	Builder.AddCheckbox(lng::MConfigShowScrollbar, ShowPanelScrollbar);
	Builder.AddCheckbox(lng::MConfigShowScreensNumber, ShowScreensNumber);
	Builder.AddCheckbox(lng::MConfigShowSortMode, ShowSortMode);
	Builder.AddCheckbox(lng::MConfigShowDotsInRoot, ShowDotsInRoot);
	Builder.AddCheckbox(lng::MConfigHighlightColumnSeparator, HighlightColumnSeparator);
	Builder.AddCheckbox(lng::MConfigDoubleGlobalColumnSeparator, DoubleGlobalColumnSeparator);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (!AutoUpdate)
			AutoUpdateLimit = 0;

		Global->CtrlObject->Cp()->LeftPanel()->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->RightPanel()->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->Redraw();
	}
}

void Options::TreeSettings()
{
	DialogBuilder Builder(lng::MConfigTreeTitle, L"TreeSettings");

	DialogItemEx *TemplateEdit;

	Builder.AddCheckbox(lng::MConfigTreeAutoChange, Tree.AutoChangeFolder);

	TemplateEdit = Builder.AddIntEditField(Tree.MinTreeCount, 3);
	Builder.AddTextBefore(TemplateEdit, lng::MConfigTreeLabelMinFolder);

#if defined(TREEFILE_PROJECT)
	DialogItemEx *Checkbox;

	Builder.AddSeparator(lng::MConfigTreeLabel1);

	Checkbox = Builder.AddCheckbox(lng::MConfigTreeLabelLocalDisk, Tree.LocalDisk);
	TemplateEdit = Builder.AddEditField(Tree.strLocalDisk, 44);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(lng::MConfigTreeLabelNetDisk, Tree.NetDisk);
	TemplateEdit = Builder.AddEditField(Tree.strNetDisk, 44);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(lng::MConfigTreeLabelNetPath, Tree.NetPath);
	TemplateEdit = Builder.AddEditField(Tree.strNetPath, 44);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(lng::MConfigTreeLabelRemovableDisk, Tree.RemovableDisk);
	TemplateEdit = Builder.AddEditField(Tree.strRemovableDisk, 44);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(lng::MConfigTreeLabelCDDisk, Tree.CDDisk);
	TemplateEdit = Builder.AddEditField(Tree.strCDDisk, 44);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Builder.AddText(lng::MConfigTreeLabelSaveLocalPath);
	Builder.AddEditField(Tree.strSaveLocalPath, 48);

	Builder.AddText(lng::MConfigTreeLabelSaveNetPath);
	Builder.AddEditField(Tree.strSaveNetPath, 48);

	Builder.AddText(lng::MConfigTreeLabelExceptPath);
	Builder.AddEditField(Tree.strExceptPath, 48);
#endif

	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		Global->CtrlObject->Cp()->LeftPanel()->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->RightPanel()->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->Redraw();
	}
}

void Options::InterfaceSettings()
{
	DialogBuilder Builder(lng::MConfigInterfaceTitle, L"InterfSettings");

	Builder.AddCheckbox(lng::MConfigClock, Clock);
	Builder.AddCheckbox(lng::MConfigViewerEditorClock, ViewerEditorClock);
	Builder.AddCheckbox(lng::MConfigMouse, Mouse);
	Builder.AddCheckbox(lng::MConfigKeyBar, ShowKeyBar);
	Builder.AddCheckbox(lng::MConfigMenuBar, ShowMenuBar);
	DialogItemEx *SaverCheckbox = Builder.AddCheckbox(lng::MConfigSaver, ScreenSaver);

	DialogItemEx *SaverEdit = Builder.AddIntEditField(ScreenSaverTime, 2);
	SaverEdit->Indent(4);
	Builder.AddTextAfter(SaverEdit, lng::MConfigSaverMinutes);
	Builder.LinkFlags(SaverCheckbox, SaverEdit, DIF_DISABLE);

	Builder.AddCheckbox(lng::MConfigCopyTotal, CMOpt.CopyShowTotal);
	Builder.AddCheckbox(lng::MConfigCopyTimeRule, CMOpt.CopyTimeRule);
	Builder.AddCheckbox(lng::MConfigDeleteTotal, DelOpt.ShowTotal);
	Builder.AddCheckbox(lng::MConfigPgUpChangeDisk, PgUpChangeDisk);
	Builder.AddCheckbox(lng::MConfigClearType, ClearType);
	DialogItemEx* SetIconCheck = Builder.AddCheckbox(lng::MConfigSetConsoleIcon, SetIcon);
	DialogItemEx* SetAdminIconCheck = Builder.AddCheckbox(lng::MConfigSetAdminConsoleIcon, SetAdminIcon);
	SetAdminIconCheck->Indent(4);
	Builder.LinkFlags(SetIconCheck, SetAdminIconCheck, DIF_DISABLE);
	Builder.AddText(lng::MConfigTitleAddons);
	Builder.AddEditField(strTitleAddons, 47);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (CMOpt.CopyTimeRule)
			CMOpt.CopyTimeRule = 3;

		SetFarConsoleMode();
		const auto& Panels = Global->CtrlObject->Cp();
		Panels->LeftPanel()->Update(UPDATE_KEEP_SELECTION);
		Panels->RightPanel()->Update(UPDATE_KEEP_SELECTION);
		Panels->SetScreenPosition();
		// $ 10.07.2001 SKV ! надо это делать, иначе если кейбар спрятали, будет полный рамс.
		Panels->Redraw();
	}
}

void Options::AutoCompleteSettings()
{
	DialogBuilder Builder(lng::MConfigAutoCompleteTitle, L"AutoCompleteSettings");
	DialogItemEx *ListCheck=Builder.AddCheckbox(lng::MConfigAutoCompleteShowList, AutoComplete.ShowList);
	DialogItemEx *ModalModeCheck=Builder.AddCheckbox(lng::MConfigAutoCompleteModalList, AutoComplete.ModalList);
	ModalModeCheck->Indent(4);
	Builder.AddCheckbox(lng::MConfigAutoCompleteAutoAppend, AutoComplete.AppendCompletion);
	Builder.LinkFlags(ListCheck, ModalModeCheck, DIF_DISABLE);
	Builder.AddOKCancel();
	Builder.ShowDialog();
}

void Options::InfoPanelSettings()
{
	static const FarDialogBuilderListItem UNListItems[]=
	{
		{ lng::MConfigInfoPanelUNUnknown, NameUnknown },                            // 0  - unknown name type
		{ lng::MConfigInfoPanelUNFullyQualifiedDN, NameFullyQualifiedDN },          // 1  - CN=John Doe, OU=Software, OU=Engineering, O=Widget, C=US
		{ lng::MConfigInfoPanelUNSamCompatible, NameSamCompatible },                // 2  - Engineering\JohnDoe, If the user account is not in a domain, only NameSamCompatible is supported.
		{ lng::MConfigInfoPanelUNDisplay, NameDisplay },                            // 3  - Probably "John Doe" but could be something else.  I.e. The display name is not necessarily the defining RDN.
		{ lng::MConfigInfoPanelUNUniqueId, NameUniqueId },                          // 6  - String-ized GUID as returned by IIDFromString(). eg: {4fa050f0-f561-11cf-bdd9-00aa003a77b6}
		{ lng::MConfigInfoPanelUNCanonical, NameCanonical },                        // 7  - engineering.widget.com/software/John Doe
		{ lng::MConfigInfoPanelUNUserPrincipal, NameUserPrincipal },                // 8  - someone@example.com
		{ lng::MConfigInfoPanelUNServicePrincipal, NameServicePrincipal },          // 10 - www/srv.engineering.com/engineering.com
		{ lng::MConfigInfoPanelUNDnsDomain, NameDnsDomain },                        // 12 - DNS domain name + SAM username eg: engineering.widget.com\JohnDoe
	};

	static const FarDialogBuilderListItem CNListItems[] =
	{
		{ lng::MConfigInfoPanelCNNetBIOS, ComputerNameNetBIOS },                                     // The NetBIOS name of the local computer or the cluster associated with the local computer. This name is limited to MAX_COMPUTERNAME_LENGTH + 1 characters and may be a truncated version of the DNS host name. For example, if the DNS host name is "corporate-mail-server", the NetBIOS name would be "corporate-mail-".
		{ lng::MConfigInfoPanelCNDnsHostname, ComputerNameDnsHostname },                             // The DNS name of the local computer or the cluster associated with the local computer.
		{ lng::MConfigInfoPanelCNDnsDomain, ComputerNameDnsDomain },                                 // The name of the DNS domain assigned to the local computer or the cluster associated with the local computer.
		{ lng::MConfigInfoPanelCNDnsFullyQualified, ComputerNameDnsFullyQualified },                 // The fully-qualified DNS name that uniquely identifies the local computer or the cluster associated with the local computer. This name is a combination of the DNS host name and the DNS domain name, using the form HostName.DomainName. For example, if the DNS host name is "corporate-mail-server" and the DNS domain name is "microsoft.com", the fully qualified DNS name is "corporate-mail-server.microsoft.com".
		{ lng::MConfigInfoPanelCNPhysicalNetBIOS, ComputerNamePhysicalNetBIOS },                     // The NetBIOS name of the local computer. On a cluster, this is the NetBIOS name of the local node on the cluster.
		{ lng::MConfigInfoPanelCNPhysicalDnsHostname, ComputerNamePhysicalDnsHostname },             // The DNS host name of the local computer. On a cluster, this is the DNS host name of the local node on the cluster.
		{ lng::MConfigInfoPanelCNPhysicalDnsDomain, ComputerNamePhysicalDnsDomain },                 // The name of the DNS domain assigned to the local computer. On a cluster, this is the DNS domain of the local node on the cluster.
		{ lng::MConfigInfoPanelCNPhysicalDnsFullyQualified, ComputerNamePhysicalDnsFullyQualified }, // The fully-qualified DNS name that uniquely identifies the computer. On a cluster, this is the fully qualified DNS name of the local node on the cluster. The fully qualified DNS name is a combination of the DNS host name and the DNS domain name, using the form HostName.DomainName.
	};

	DialogBuilder Builder(lng::MConfigInfoPanelTitle, L"InfoPanelSettings");
	Builder.AddCheckbox(lng::MConfigInfoPanelShowPowerStatus, InfoPanel.ShowPowerStatus);
	Builder.AddCheckbox(lng::MConfigInfoPanelShowCDInfo, InfoPanel.ShowCDInfo);
	Builder.AddText(lng::MConfigInfoPanelCNTitle);
	Builder.AddComboBox(InfoPanel.ComputerNameFormat, nullptr, 50, CNListItems, std::size(CNListItems), DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddText(lng::MConfigInfoPanelUNTitle);
	Builder.AddComboBox(InfoPanel.UserNameFormat, nullptr, 50, UNListItems, std::size(UNListItems), DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		bool needRedraw=false;
		if (Global->CtrlObject->Cp()->LeftPanel()->GetType() == panel_type::INFO_PANEL)
		{
			Global->CtrlObject->Cp()->LeftPanel()->Update(UPDATE_KEEP_SELECTION);
			needRedraw=true;
		}
		if (Global->CtrlObject->Cp()->RightPanel()->GetType() == panel_type::INFO_PANEL)
		{
			Global->CtrlObject->Cp()->RightPanel()->Update(UPDATE_KEEP_SELECTION);
			needRedraw=true;
		}
		if (needRedraw)
		{
			//Global->CtrlObject->Cp()->SetScreenPosition();
			Global->CtrlObject->Cp()->Redraw();
		}
	}
}

static void ApplyDefaultMaskGroups()
{
	static const std::pair<const wchar_t*, const wchar_t*> Sets[] =
	{
		{L"arc", L"*.rar,*.zip,*.[zj],*.[bg7x]z,*.[bg]zip,*.tar,*.t[agbx]z,*.ar[cj],*.r[0-9][0-9],*.a[0-9][0-9],*.bz2,*.cab,*.msi,*.jar,*.lha,*.lzh,*.ha,*.ac[bei],*.pa[ck],*.rk,*.cpio,*.rpm,*.zoo,*.hqx,*.sit,*.ice,*.uc2,*.ain,*.imp,*.777,*.ufa,*.boa,*.bs[2a],*.sea,*.hpk,*.ddi,*.x2,*.rkv,*.[lw]sz,*.h[ay]p,*.lim,*.sqz,*.chz"},
		{L"temp", L"*.bak,*.tmp"},
		{L"exec", L"*.exe,*.com,*.bat,*.cmd,%PATHEXT%"},
	};

	std::for_each(CONST_RANGE(Sets, i)
	{
		ConfigProvider().GeneralCfg()->SetValue(L"Masks", i.first, i.second);
	});
}

static void FillMasksMenu(VMenu2& MasksMenu, int SelPos = 0)
{
	MasksMenu.clear();

	static const string MasksKeyName = L"Masks"s;
	for(const auto& i: ConfigProvider().GeneralCfg()->ValuesEnumerator<string>(MasksKeyName))
	{
		MenuItemEx Item;
		string DisplayName(i.first);
		const int NameWidth = 10;
		TruncStrFromEnd(DisplayName, NameWidth);
		DisplayName.resize(NameWidth, L' ');
		Item.strName = concat(DisplayName, L' ', BoxSymbols[BS_V1], L' ', i.second);
		Item.UserData = i.first;
		MasksMenu.AddItem(Item);
	}
	MasksMenu.SetSelectPos(SelPos, 0);
}

void Options::MaskGroupsSettings()
{
	const auto MasksMenu = VMenu2::create(msg(lng::MMenuMaskGroups), nullptr, 0, 0, VMENU_WRAPMODE | VMENU_SHOWAMPERSAND);
	MasksMenu->SetBottomTitle(msg(lng::MMaskGroupBottom));
	MasksMenu->SetHelp(L"MaskGroupsSettings");
	FillMasksMenu(*MasksMenu);
	MasksMenu->SetPosition(-1, -1, -1, -1);

	bool Changed = false;
	bool Filter = false;
	for(;;)
	{
		MasksMenu->Run([&](const Manager::Key& RawKey)
		{
			const auto Key=RawKey();
			if(Filter)
			{
				if(Key == KEY_ESC || Key == KEY_F10 || Key == KEY_ENTER || Key == KEY_NUMENTER)
				{
					Filter = false;
					for (size_t i = 0, size = MasksMenu->size(); i != size;  ++i)
					{
						MasksMenu->UpdateItemFlags(static_cast<int>(i), MasksMenu->at(i).Flags & ~MIF_HIDDEN);
					}
					MasksMenu->SetPosition(-1, -1, -1, -1);
					MasksMenu->SetTitle(msg(lng::MMenuMaskGroups));
					MasksMenu->SetBottomTitle(msg(lng::MMaskGroupBottom));
				}
				return 1;
			}
			int ItemPos = MasksMenu->GetSelectPos();
			const auto* Item = MasksMenu->GetUserDataPtr<string>(ItemPos);
			int KeyProcessed = 1;
			static const string EmptyString;
			switch (Key)
			{
			case KEY_NUMDEL:
			case KEY_DEL:
				if(Item && Message(0,
					msg(lng::MMenuMaskGroups),
					{
						msg(lng::MMaskGroupAskDelete),
						*Item
					},
					{ lng::MDelete, lng::MCancel }) == Message::first_button)
				{
					ConfigProvider().GeneralCfg()->DeleteValue(L"Masks", *Item);
					Changed = true;
				}
				break;

			case KEY_NUMPAD0:
			case KEY_INS:
				Item = &EmptyString;
				// fallthrough
			case KEY_ENTER:
			case KEY_NUMENTER:
			case KEY_F4:
				{
					if (Item)
					{
						string Name, Value;

						if (!Item->empty())
						{
							Name = *Item;
							ConfigProvider().GeneralCfg()->GetValue(L"Masks", Name, Value, L"");
						}
						DialogBuilder Builder(lng::MMenuMaskGroups, L"MaskGroupsSettings");
						Builder.AddText(lng::MMaskGroupName);
						Builder.AddEditField(Name, 60);
						Builder.AddText(lng::MMaskGroupMasks);
						Builder.AddEditField(Value, 60);
						Builder.AddOKCancel();
						if(Builder.ShowDialog())
						{
							if(!Item->empty())
							{
								ConfigProvider().GeneralCfg()->DeleteValue(L"Masks", *Item);
							}
							ConfigProvider().GeneralCfg()->SetValue(L"Masks", Name, Value);
							Changed = true;
						}
					}
				}
				break;

			case KEY_CTRLR:
			case KEY_RCTRLR:
				{
					if (Message(MSG_WARNING,
						msg(lng::MMenuMaskGroups),
						{
							msg(lng::MMaskGroupRestore),
						},
						{ lng::MYes, lng::MCancel }) == Message::first_button)
					{
						ApplyDefaultMaskGroups();
						Changed = true;
					}
				}
				break;

			case KEY_F7:
				{
					string Value;
					DialogBuilder Builder(lng::MFileFilterTitle, nullptr);
					Builder.AddText(lng::MMaskGroupFindMask);
					Builder.AddEditField(Value, 60, L"MaskGroupsFindMask");
					Builder.AddOKCancel();
					if(Builder.ShowDialog())
					{
						for (size_t i = 0, size = MasksMenu->size(); i != size; ++i)
						{
							string CurrentMasks;
							ConfigProvider().GeneralCfg()->GetValue(L"Masks", *MasksMenu->GetUserDataPtr<string>(i), CurrentMasks, L"");
							filemasks Masks;
							Masks.Set(CurrentMasks);
							if(!Masks.Compare(Value))
							{
								MasksMenu->UpdateItemFlags(static_cast<int>(i), MasksMenu->at(i).Flags | MIF_HIDDEN);
							}
						}
						MasksMenu->SetPosition(-1, -1, -1, -1);
						MasksMenu->SetTitle(Value);
						MasksMenu->SetBottomTitle(format(lng::MMaskGroupTotal, MasksMenu->GetShowItemCount()));
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

				FillMasksMenu(*MasksMenu, MasksMenu->GetSelectPos());
				Global->CtrlObject->HiFiles->UpdateHighlighting(true);
			}
			return KeyProcessed;
		});
		if (MasksMenu->GetExitCode()!=-1)
		{
			MasksMenu->Key(KEY_F4);
			continue;
		}
		break;
	}
}

void Options::DialogSettings()
{
	DialogBuilder Builder(lng::MConfigDlgSetsTitle, L"DialogSettings");

	Builder.AddCheckbox(lng::MConfigDialogsEditHistory, Dialogs.EditHistory);
	Builder.AddCheckbox(lng::MConfigDialogsEditBlock, Dialogs.EditBlock);
	Builder.AddCheckbox(lng::MConfigDialogsDelRemovesBlocks, Dialogs.DelRemovesBlocks);
	Builder.AddCheckbox(lng::MConfigDialogsAutoComplete, Dialogs.AutoComplete);
	Builder.AddCheckbox(lng::MConfigDialogsEULBsClear, Dialogs.EULBsClear);
	Builder.AddCheckbox(lng::MConfigDialogsMouseButton, Dialogs.MouseButton);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (Dialogs.MouseButton )
			Dialogs.MouseButton = 0xFFFF;
	}
}

void Options::VMenuSettings()
{

	static const FarDialogBuilderListItem CAListItems[]=
	{
		{ lng::MConfigVMenuClickCancel, VMENUCLICK_CANCEL },  // Cancel menu
		{ lng::MConfigVMenuClickApply,  VMENUCLICK_APPLY  },  // Execute selected item
		{ lng::MConfigVMenuClickIgnore, VMENUCLICK_IGNORE },  // Do nothing
	};

	static const std::pair<lng, IntOption VMenuOptions::*> DialogItems[] =
	{
		{ lng::MConfigVMenuLBtnClick, &VMenuOptions::LBtnClick },
		{ lng::MConfigVMenuRBtnClick, &VMenuOptions::RBtnClick },
		{ lng::MConfigVMenuMBtnClick, &VMenuOptions::MBtnClick },
	};

	DialogBuilder Builder(lng::MConfigVMenuTitle, L"VMenuSettings");

	std::for_each(CONST_RANGE(DialogItems, i)
	{
		Builder.AddText(i.first);
		Builder.AddComboBox(std::invoke(i.second, VMenu), nullptr, 40, CAListItems, std::size(CAListItems), DIF_LISTAUTOHIGHLIGHT | DIF_LISTWRAPMODE | DIF_DROPDOWNLIST);
	});

	Builder.AddOKCancel();
	Builder.ShowDialog();
}

void Options::CmdlineSettings()
{
	DialogBuilder Builder(lng::MConfigCmdlineTitle, L"CmdlineSettings");

	Builder.AddCheckbox(lng::MConfigCmdlineEditBlock, CmdLine.EditBlock);
	Builder.AddCheckbox(lng::MConfigCmdlineDelRemovesBlocks, CmdLine.DelRemovesBlocks);
	Builder.AddCheckbox(lng::MConfigCmdlineAutoComplete, CmdLine.AutoComplete);
	DialogItemEx *UsePromptFormat = Builder.AddCheckbox(lng::MConfigCmdlineUsePromptFormat, CmdLine.UsePromptFormat);
	DialogItemEx *PromptFormat = Builder.AddEditField(CmdLine.strPromptFormat, 33);
	PromptFormat->Indent(4);
	Builder.LinkFlags(UsePromptFormat, PromptFormat, DIF_DISABLE);

	UsePromptFormat = Builder.AddCheckbox(lng::MConfigCmdlineUseHomeDir, Exec.UseHomeDir);
	PromptFormat = Builder.AddEditField(Exec.strHomeDir, 33);
	PromptFormat->Indent(4);
	Builder.LinkFlags(UsePromptFormat, PromptFormat, DIF_DISABLE);

	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		Global->CtrlObject->CmdLine()->SetPersistentBlocks(CmdLine.EditBlock);
		Global->CtrlObject->CmdLine()->SetDelRemovesBlocks(CmdLine.DelRemovesBlocks);
		Global->CtrlObject->CmdLine()->SetAutoComplete(CmdLine.AutoComplete);
	}
}

void Options::SetConfirmations()
{
	DialogBuilder Builder(lng::MSetConfirmTitle, L"ConfirmDlg");

	Builder.AddCheckbox(lng::MSetConfirmCopy, Confirm.Copy);
	Builder.AddCheckbox(lng::MSetConfirmMove, Confirm.Move);
	Builder.AddCheckbox(lng::MSetConfirmRO, Confirm.RO);
	Builder.AddCheckbox(lng::MSetConfirmDrag, Confirm.Drag);
	Builder.AddCheckbox(lng::MSetConfirmDelete, Confirm.Delete);
	Builder.AddCheckbox(lng::MSetConfirmDeleteFolders, Confirm.DeleteFolder);
	Builder.AddCheckbox(lng::MSetConfirmEsc, Confirm.Esc);
	Builder.AddCheckbox(lng::MSetConfirmRemoveConnection, Confirm.RemoveConnection);
	Builder.AddCheckbox(lng::MSetConfirmRemoveSUBST, Confirm.RemoveSUBST);
	Builder.AddCheckbox(lng::MSetConfirmDetachVHD, Confirm.DetachVHD);
	Builder.AddCheckbox(lng::MSetConfirmRemoveHotPlug, Confirm.RemoveHotPlug);
	Builder.AddCheckbox(lng::MSetConfirmAllowReedit, Confirm.AllowReedit);
	Builder.AddCheckbox(lng::MSetConfirmHistoryClear, Confirm.HistoryClear);
	Builder.AddCheckbox(lng::MSetConfirmExit, Confirm.Exit);
	Builder.AddOKCancel();

	Builder.ShowDialog();
}

void Options::PluginsManagerSettings()
{
	DialogBuilder Builder(lng::MPluginsManagerSettingsTitle, L"PluginsManagerSettings");
#ifndef NO_WRAPPER
	Builder.AddCheckbox(lng::MPluginsManagerOEMPluginsSupport, LoadPlug.OEMPluginsSupport);
#endif // NO_WRAPPER
	Builder.AddCheckbox(lng::MPluginsManagerScanSymlinks, LoadPlug.ScanSymlinks);
	Builder.AddSeparator(lng::MPluginConfirmationTitle);
	Builder.AddCheckbox(lng::MPluginsManagerOFP, PluginConfirm.OpenFilePlugin);
	DialogItemEx *StandardAssoc = Builder.AddCheckbox(lng::MPluginsManagerStdAssoc, PluginConfirm.StandardAssociation);
	DialogItemEx *EvenIfOnlyOne = Builder.AddCheckbox(lng::MPluginsManagerEvenOne, PluginConfirm.EvenIfOnlyOnePlugin);
	StandardAssoc->Indent(2);
	EvenIfOnlyOne->Indent(4);

	Builder.AddCheckbox(lng::MPluginsManagerSFL, PluginConfirm.SetFindList);
	Builder.AddCheckbox(lng::MPluginsManagerPF, PluginConfirm.Prefix);
	Builder.AddOKCancel();

	Builder.ShowDialog();
}

void Options::SetDizConfig()
{
	DialogBuilder Builder(lng::MCfgDizTitle, L"FileDiz");

	Builder.AddText(lng::MCfgDizListNames);
	Builder.AddEditField(Diz.strListNames, 65);
	Builder.AddSeparator();

	Builder.AddCheckbox(lng::MCfgDizSetHidden, Diz.SetHidden);
	Builder.AddCheckbox(lng::MCfgDizROUpdate, Diz.ROUpdate);
	DialogItemEx *StartPos = Builder.AddIntEditField(Diz.StartPos, 2);
	Builder.AddTextAfter(StartPos, lng::MCfgDizStartPos);
	Builder.AddSeparator();

	static const lng DizOptions[] = { lng::MCfgDizNotUpdate, lng::MCfgDizUpdateIfDisplayed, lng::MCfgDizAlwaysUpdate };
	Builder.AddRadioButtons(Diz.UpdateMode, 3, DizOptions);
	Builder.AddSeparator();

	Builder.AddCheckbox(lng::MCfgDizAnsiByDefault, Diz.AnsiByDefault);
	Builder.AddCheckbox(lng::MCfgDizSaveInUTF, Diz.SaveInUTF);
	Builder.AddOKCancel();
	Builder.ShowDialog();
}

void Options::ViewerConfig(Options::ViewerOptions &ViOptRef, bool Local)
{
	intptr_t save_pos = 0, save_cp = 0, id = 0;
	bool prev_save_cp_value = ViOpt.SaveCodepage, inside = false;

	DialogBuilder Builder(lng::MViewConfigTitle, L"ViewerSettings", [&](Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
	{
		if (Msg == DN_INITDIALOG && save_pos)
		{
			Dlg->SendMessage(DM_ENABLE, save_cp, ToPtr(ViOpt.SavePos ? false : true));
			if (ViOpt.SavePos)
			{
				ViOpt.SaveCodepage = true;
			}
		}
		else if (Msg == DN_BTNCLICK && save_pos)
		{
			if (Param1 == save_pos)
			{
				inside = true;
				Dlg->SendMessage(DM_SETCHECK, save_cp, ToPtr(Param2? true : prev_save_cp_value));
				Dlg->SendMessage(DM_ENABLE, save_cp, ToPtr(Param2 ? false : true));
				inside = false;
			}
			else if (Param1 == save_cp && !inside)
			{
				prev_save_cp_value = (Param2 != nullptr);
			}
		}
		return Dlg->DefProc(Msg, Param1, Param2);
	});

	std::vector<FarDialogBuilderListItem2> Items; //Must live until Dialog end

	if (!Local)
	{
		++id; Builder.AddCheckbox(lng::MViewConfigExternalF3, ViOpt.UseExternalViewer);
		++id; Builder.AddText(lng::MViewConfigExternalCommand);
		++id; Builder.AddEditField(strExternalViewer, 64, L"ExternalViewer", DIF_EDITPATH|DIF_EDITPATHEXEC);
		++id; Builder.AddSeparator(lng::MViewConfigInternal);
	}

	Builder.StartColumns();
	++id; Builder.AddCheckbox(lng::MViewConfigPersistentSelection, ViOptRef.PersistentBlocks);
	++id; Builder.AddCheckbox(lng::MViewConfigEditAutofocus, ViOptRef.SearchEditFocus);
	++id; DialogItemEx *TabSize = Builder.AddIntEditField(ViOptRef.TabSize, 3);
	++id; Builder.AddTextAfter(TabSize, lng::MViewConfigTabSize);
	Builder.ColumnBreak();
	++id; Builder.AddCheckbox(lng::MViewConfigArrows, ViOptRef.ShowArrows);
	++id; Builder.AddCheckbox(lng::MViewConfigVisible0x00, ViOptRef.Visible0x00);
	++id; Builder.AddCheckbox(lng::MViewConfigScrollbar, ViOptRef.ShowScrollbar);
	Builder.EndColumns();

	if (!Local)
	{
		++id; Builder.AddSeparator();
		Builder.StartColumns();
		save_pos = ++id; Builder.AddCheckbox(lng::MViewConfigSavePos, ViOpt.SavePos);
		save_cp = ++id; Builder.AddCheckbox(lng::MViewConfigSaveCodepage, ViOpt.SaveCodepage);
		DialogItemEx *MaxLineSize = Builder.AddIntEditField(ViOpt.MaxLineSize, 6);
		Builder.AddTextAfter(MaxLineSize, lng::MViewConfigMaxLineSize);
		Builder.ColumnBreak();
		Builder.AddCheckbox(lng::MViewConfigSaveShortPos, ViOpt.SaveShortPos);
		Builder.AddCheckbox(lng::MViewConfigSaveWrapMode, ViOpt.SaveWrapMode);
		Builder.AddCheckbox(lng::MViewAutoDetectCodePage, ViOpt.AutoDetectCodePage);
		Builder.EndColumns();
		Builder.AddText(lng::MViewConfigDefaultCodePage);
		Codepages().FillCodePagesList(Items, false, false, false, false, true);
		Builder.AddComboBox(ViOpt.DefaultCodePage, nullptr, 64, Items, DIF_LISTWRAPMODE|DIF_LISTAUTOHIGHLIGHT);
	}

	Builder.AddOKCancel();

	Builder.ShowDialog();
}

void Options::EditorConfig(Options::EditorOptions &EdOptRef, bool Local)
{
	DialogBuilder Builder(lng::MEditConfigTitle, L"EditorSettings");

	std::vector<FarDialogBuilderListItem2> Items; //Must live until Dialog end

	if (!Local)
	{
		Builder.AddCheckbox(lng::MEditConfigEditorF4, EdOpt.UseExternalEditor);
		Builder.AddText(lng::MEditConfigEditorCommand);
		Builder.AddEditField(strExternalEditor, 64, L"ExternalEditor", DIF_EDITPATH|DIF_EDITPATHEXEC);
		Builder.AddSeparator(lng::MEditConfigInternal);
	}

	Builder.AddText(lng::MEditConfigExpandTabsTitle);
	static const FarDialogBuilderListItem ExpandTabsItems[] =
	{
		{ lng::MEditConfigDoNotExpandTabs, EXPAND_NOTABS },
		{ lng::MEditConfigExpandTabs, EXPAND_NEWTABS },
		{ lng::MEditConfigConvertAllTabsToSpaces, EXPAND_ALLTABS }
	};
	Builder.AddComboBox(EdOptRef.ExpandTabs, nullptr, 64, ExpandTabsItems, std::size(ExpandTabsItems), DIF_LISTAUTOHIGHLIGHT | DIF_LISTWRAPMODE | DIF_DROPDOWNLIST);

	Builder.StartColumns();
	Builder.AddCheckbox(lng::MEditConfigPersistentBlocks, EdOptRef.PersistentBlocks);
	Builder.AddCheckbox(lng::MEditConfigDelRemovesBlocks, EdOptRef.DelRemovesBlocks);
	Builder.AddCheckbox(lng::MEditConfigAutoIndent, EdOptRef.AutoIndent);
	DialogItemEx *TabSize = Builder.AddIntEditField(EdOptRef.TabSize, 3);
	Builder.AddTextAfter(TabSize, lng::MEditConfigTabSize);
	Builder.AddCheckbox(lng::MEditShowWhiteSpace, EdOptRef.ShowWhiteSpace);
	Builder.ColumnBreak();
	Builder.AddCheckbox(lng::MEditCursorBeyondEnd, EdOptRef.CursorBeyondEOL);
	Builder.AddCheckbox(lng::MEditConfigSelFound, EdOptRef.SearchSelFound);
	Builder.AddCheckbox(lng::MEditConfigCursorAtEnd, EdOptRef.SearchCursorAtEnd);
	Builder.AddCheckbox(lng::MEditConfigScrollbar, EdOptRef.ShowScrollBar);
	Builder.EndColumns();

	if (!Local)
	{
		Builder.AddSeparator();
		Builder.AddCheckbox(lng::MEditConfigSavePos, EdOptRef.SavePos);
		Builder.AddCheckbox(lng::MEditConfigSaveShortPos, EdOptRef.SaveShortPos);
		Builder.AddCheckbox(lng::MEditShareWrite, EdOpt.EditOpenedForWrite);
		Builder.AddCheckbox(lng::MEditLockROFileModification, EdOpt.ReadOnlyLock, 1);
		Builder.AddCheckbox(lng::MEditWarningBeforeOpenROFile, EdOpt.ReadOnlyLock, 2);
		Builder.AddCheckbox(lng::MEditAutoDetectCodePage, EdOpt.AutoDetectCodePage);
		Builder.AddText(lng::MEditConfigDefaultCodePage);
		Codepages().FillCodePagesList(Items, false, false, false, false, false);
		Builder.AddComboBox(EdOpt.DefaultCodePage, nullptr, 64, Items, DIF_LISTWRAPMODE|DIF_LISTAUTOHIGHLIGHT);
	}

	Builder.AddOKCancel();

	Builder.ShowDialog();
}

void Options::SetFolderInfoFiles()
{
	string strFolderInfoFiles;

	if (GetString(
		msg(lng::MSetFolderInfoTitle).data(),
		msg(lng::MSetFolderInfoNames).data(),
		L"FolderInfoFiles",
		InfoPanel.strFolderInfoFiles.data(),
		strFolderInfoFiles,
		L"FolderDiz",
		FIB_ENABLEEMPTY | FIB_BUTTONS))
	{
		InfoPanel.strFolderInfoFiles = strFolderInfoFiles;

		if (Global->CtrlObject->Cp()->LeftPanel()->GetType() == panel_type::INFO_PANEL)
			Global->CtrlObject->Cp()->LeftPanel()->Update(0);

		if (Global->CtrlObject->Cp()->RightPanel()->GetType() == panel_type::INFO_PANEL)
			Global->CtrlObject->Cp()->RightPanel()->Update(0);
	}
}

static void ResetViewModes(const range<PanelViewSettings*>& Modes, int Index = -1)
{
	static const struct
	{
		std::initializer_list<column> PanelColumns, StatusColumns;
		unsigned long long Flags;
	}
	InitialModes[] =
	{
		// Alternative full
		{
			{
				{NAME_COLUMN | COLUMN_MARK},
				{SIZE_COLUMN | COLUMN_COMMAS, 10},
				{DATE_COLUMN},
			},
			{
				{NAME_COLUMN | COLUMN_RIGHTALIGN},
			},
			PVS_ALIGNEXTENSIONS,
		},

		// Brief
		{
			{
				{NAME_COLUMN},
				{NAME_COLUMN},
				{NAME_COLUMN},
			},
			{
				{NAME_COLUMN | COLUMN_RIGHTALIGN},
				{SIZE_COLUMN, 6},
				{DATE_COLUMN},
				{TIME_COLUMN, 5},
			},
			PVS_ALIGNEXTENSIONS,
		},

		// Medium
		{
			{
				{NAME_COLUMN},
				{NAME_COLUMN},
			},
			{
				{NAME_COLUMN | COLUMN_RIGHTALIGN},
				{SIZE_COLUMN, 6},
				{DATE_COLUMN},
				{TIME_COLUMN, 5},
			},
			PVS_NONE,
		},

		// Full
		{
			{
				{NAME_COLUMN},
				{SIZE_COLUMN, 6},
				{DATE_COLUMN},
				{TIME_COLUMN, 5},
			},
			{
				{NAME_COLUMN | COLUMN_RIGHTALIGN},
			},
			PVS_ALIGNEXTENSIONS,
		},

		// Wide
		{
			{
				{NAME_COLUMN},
				{SIZE_COLUMN, 6},
			},
			{
				{NAME_COLUMN | COLUMN_RIGHTALIGN},
				{SIZE_COLUMN, 6},
				{DATE_COLUMN},
				{TIME_COLUMN, 5},
			},
			PVS_NONE
		},

		// Detailed
		{
			{
				{NAME_COLUMN},
				{SIZE_COLUMN, 6},
				{PACKED_COLUMN, 6},
				{WDATE_COLUMN, 14},
				{CDATE_COLUMN, 14},
				{ADATE_COLUMN, 14},
				{ATTR_COLUMN},
			},
			{
				{NAME_COLUMN | COLUMN_RIGHTALIGN},
			},
			PVS_ALIGNEXTENSIONS|PVS_FULLSCREEN,
		},

		// Descriptions
		{
			{
				{NAME_COLUMN, 40, col_width::percent},
				{DIZ_COLUMN},
			},
			{
				{NAME_COLUMN | COLUMN_RIGHTALIGN},
				{SIZE_COLUMN, 6},
				{DATE_COLUMN},
				{TIME_COLUMN, 5},
			},
			PVS_ALIGNEXTENSIONS,
		},

		// Long descriptions
		{
			{
				{NAME_COLUMN},
				{SIZE_COLUMN, 6},
				{DIZ_COLUMN, 70, col_width::percent},
			},
			{
				{NAME_COLUMN | COLUMN_RIGHTALIGN},
			},
			PVS_ALIGNEXTENSIONS|PVS_FULLSCREEN,
		},

		// File owners
		{
			{
				{NAME_COLUMN},
				{SIZE_COLUMN, 6},
				{OWNER_COLUMN, 15},
			},
			{
				{NAME_COLUMN | COLUMN_RIGHTALIGN},
				{SIZE_COLUMN, 6},
				{DATE_COLUMN},
				{TIME_COLUMN, 15},
			},
			PVS_ALIGNEXTENSIONS,
		},

		// File links
		{
			{
				{NAME_COLUMN},
				{SIZE_COLUMN, 6},
				{NUMLINK_COLUMN, 3},
			},
			{
				{NAME_COLUMN | COLUMN_RIGHTALIGN},
				{SIZE_COLUMN, 6},
				{DATE_COLUMN},
				{TIME_COLUMN, 5},
			},
			PVS_ALIGNEXTENSIONS,
		},
	};
	static_assert(std::size(InitialModes) == predefined_panel_modes_count);

	const auto& InitMode = [](const auto& src, auto& dst)
	{
		dst.PanelColumns = src.PanelColumns;
		dst.StatusColumns = src.StatusColumns;
		dst.Flags = src.Flags;
		dst.Name.clear();
	};

	if (Index < 0)
	{
		for (const auto& i: zip(InitialModes, Modes))
			std::apply(InitMode, i);
	}
	else
	{
		InitMode(InitialModes[Index], Modes.front());
	}
}

void Options::SetFilePanelModes()
{
	size_t CurMode=0;

	if (Global->CtrlObject->Cp()->ActivePanel()->GetType() == panel_type::FILE_PANEL)
	{
		CurMode=Global->CtrlObject->Cp()->ActivePanel()->GetViewMode();
		CurMode = RealModeToDisplay(CurMode);
	}

	for(;;)
	{
		static const lng PredefinedNames[] =
		{
			lng::MMenuBriefView,
			lng::MMenuMediumView,
			lng::MMenuFullView,
			lng::MMenuWideView,
			lng::MMenuDetailedView,
			lng::MMenuDizView,
			lng::MMenuLongDizView,
			lng::MMenuOwnersView,
			lng::MMenuLinksView,
			lng::MMenuAlternativeView,
		};
		static_assert(std::size(PredefinedNames) == predefined_panel_modes_count);

		const auto MenuCount = ViewSettings.size();
		// +1 for separator
		std::vector<MenuDataEx> ModeListMenu(MenuCount > predefined_panel_modes_count? MenuCount + 1: MenuCount);

		for (size_t i = 0; i < ViewSettings.size(); ++i)
		{
			ModeListMenu[RealModeToDisplay(i)].Name = ViewSettings[i].Name.data();
		}

		for (size_t i = 0; i < predefined_panel_modes_count; ++i)
		{
			if (!*ModeListMenu[i].Name)
				ModeListMenu[i].Name = msg(PredefinedNames[i]).data();
		}

		if (MenuCount > predefined_panel_modes_count)
		{
			ModeListMenu[predefined_panel_modes_count].Name = nullptr;
			ModeListMenu[predefined_panel_modes_count].Flags = LIF_SEPARATOR;
		}

		int ModeNumber = -1;

		bool AddNewMode = false;
		bool DeleteMode = false;

		ModeListMenu[CurMode].SetSelect(1);
		{
			const auto ModeList = VMenu2::create(msg(lng::MEditPanelModes), ModeListMenu.data(), ModeListMenu.size(), ScrY - 4);
			ModeList->SetPosition(-1,-1,0,0);
			ModeList->SetHelp(L"PanelViewModes");
			ModeList->SetMenuFlags(VMENU_WRAPMODE);
			ModeList->SetId(PanelViewModesId);
			ModeList->SetBottomTitle(msg(lng::MEditPanelModesBottom));

			ModeNumber=ModeList->Run([&](const Manager::Key& RawKey)
			{
				const auto Key=RawKey();
				switch (Key)
				{
				case KEY_CTRLENTER:
				case KEY_CTRLNUMENTER:
				case KEY_RCTRLENTER:
				case KEY_RCTRLNUMENTER:
				case KEY_CTRLSHIFTENTER:
				case KEY_CTRLSHIFTNUMENTER:
				case KEY_RCTRLSHIFTENTER:
				case KEY_RCTRLSHIFTNUMENTER:
					{
						auto PanelPtr = Global->CtrlObject->Cp()->ActivePanel();
						if (Key & KEY_SHIFT)
						{
							PanelPtr = Global->CtrlObject->Cp()->PassivePanel();
						}
						PanelPtr->SetViewMode(static_cast<int>(DisplayModeToReal(ModeList->GetSelectPos())));
						Global->CtrlObject->Cp()->Redraw();
					}
					return 1;

				case KEY_INS:
				case KEY_NUMPAD0:
					AddNewMode = true;
					ModeList->Close();
					break;

				case KEY_DEL:
				case KEY_NUMDEL:
					if (ModeList->GetSelectPos() >= static_cast<int>(predefined_panel_modes_count))
					{
						DeleteMode = true;
						ModeList->Close();
					}
					break;

				case KEY_F4:
					ModeList->Close(ModeList->GetSelectPos());
					break;

				default:
					break;
				}
				return 0;
			});
		}

		if (ModeNumber<0)
			return;

		CurMode=ModeNumber;

		ModeNumber = static_cast<int>(DisplayModeToReal(ModeNumber));

		if (DeleteMode)
		{
			const auto& SwitchToAnotherMode = [&](panel_ptr p)
			{
				const auto RealMode = static_cast<int>(DisplayModeToReal(CurMode));
				if (p->GetViewMode() == RealMode)
				{
					p->SetViewMode(RealMode - 1);
				}
			};

			SwitchToAnotherMode(Global->CtrlObject->Cp()->ActivePanel());
			SwitchToAnotherMode(Global->CtrlObject->Cp()->PassivePanel());

			Global->CtrlObject->Cp()->Redraw();

			DeleteViewSettings(ModeNumber);
			--CurMode;
			if (CurMode == predefined_panel_modes_count) //separator
				--CurMode;

			continue;
		}


		enum ModeItems
		{
			MD_DOUBLEBOX,
			MD_TEXTNAME,
			MD_EDITNAME,
			MD_TEXTTYPES,
			MD_EDITTYPES,
			MD_TEXTWIDTHS,
			MD_EDITWIDTHS,
			MD_TEXTSTATUSTYPES,
			MD_EDITSTATUSTYPES,
			MD_TEXTSTATUSWIDTHS,
			MD_EDITSTATUSWIDTHS,
			MD_SEPARATOR1,
			MD_CHECKBOX_FULLSCREEN,
			MD_CHECKBOX_ALIGNFILEEXT,
			MD_CHECKBOX_ALIGNFOLDEREXT,
			MD_CHECKBOX_FOLDERUPPERCASE,
			MD_CHECKBOX_FILESLOWERCASE,
			MD_CHECKBOX_UPPERTOLOWERCASE,
			MD_SEPARATOR2,
			MD_BUTTON_OK,
			MD_BUTTON_RESET,
			MD_BUTTON_CANCEL,
		} ;
		FarDialogItem ModeDlgData[]=
		{
			{DI_DOUBLEBOX, 3, 1,72,17,0,nullptr,nullptr,0,AddNewMode? nullptr : ModeListMenu[CurMode].Name},
			{DI_TEXT,      5, 2, 0, 2,0,nullptr,nullptr,0,msg(lng::MEditPanelModeName).data()},
			{DI_EDIT,      5, 3,70, 3,0,nullptr,nullptr,DIF_FOCUS,L""},
			{DI_TEXT,      5, 4, 0, 4,0,nullptr,nullptr,0,msg(lng::MEditPanelModeTypes).data()},
			{DI_EDIT,      5, 5,35, 5,0,nullptr,nullptr,0,L""},
			{DI_TEXT,      5, 6, 0, 6,0,nullptr,nullptr,0,msg(lng::MEditPanelModeWidths).data()},
			{DI_EDIT,      5, 7,35, 7,0,nullptr,nullptr,0,L""},
			{DI_TEXT,     38, 4, 0, 4,0,nullptr,nullptr,0,msg(lng::MEditPanelModeStatusTypes).data()},
			{DI_EDIT,     38, 5,70, 5,0,nullptr,nullptr,0,L""},
			{DI_TEXT,     38, 6, 0, 6,0,nullptr,nullptr,0,msg(lng::MEditPanelModeStatusWidths).data()},
			{DI_EDIT,     38, 7,70, 7,0,nullptr,nullptr,0,L""},
			{DI_TEXT,     -1, 8, 0, 8,0,nullptr,nullptr,DIF_SEPARATOR,L""},
			{DI_CHECKBOX,  5, 9, 0, 9,0,nullptr,nullptr,0,msg(lng::MEditPanelModeFullscreen).data()},
			{DI_CHECKBOX,  5,10, 0,10,0,nullptr,nullptr,0,msg(lng::MEditPanelModeAlignExtensions).data()},
			{DI_CHECKBOX,  5,11, 0,11,0,nullptr,nullptr,0,msg(lng::MEditPanelModeAlignFolderExtensions).data()},
			{DI_CHECKBOX,  5,12, 0,12,0,nullptr,nullptr,0,msg(lng::MEditPanelModeFoldersUpperCase).data()},
			{DI_CHECKBOX,  5,13, 0,13,0,nullptr,nullptr,0,msg(lng::MEditPanelModeFilesLowerCase).data()},
			{DI_CHECKBOX,  5,14, 0,14,0,nullptr,nullptr,0,msg(lng::MEditPanelModeUpperToLowerCase).data()},
			{DI_TEXT,     -1,15, 0,15,0,nullptr,nullptr,DIF_SEPARATOR,L""},
			{DI_BUTTON,    0,16, 0,16,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,msg(lng::MOk).data()},
			{DI_BUTTON,    0,16, 0,16,0,nullptr,nullptr,DIF_CENTERGROUP|(ModeNumber < static_cast<int>(predefined_panel_modes_count)? 0 : DIF_DISABLE),msg(lng::MReset).data()},
			{DI_BUTTON,    0,16, 0,16,0,nullptr,nullptr,DIF_CENTERGROUP,msg(lng::MCancel).data()},
		};
		auto ModeDlg = MakeDialogItemsEx(ModeDlgData);

		RemoveHighlights(ModeDlg[MD_DOUBLEBOX].strData);

		static const std::pair<ModeItems, panel_view_settings_flags> ModesFlagsMapping[] =
		{
			{ MD_CHECKBOX_FULLSCREEN, PVS_FULLSCREEN },
			{ MD_CHECKBOX_ALIGNFILEEXT, PVS_ALIGNEXTENSIONS },
			{ MD_CHECKBOX_ALIGNFOLDEREXT, PVS_FOLDERALIGNEXTENSIONS },
			{ MD_CHECKBOX_FOLDERUPPERCASE, PVS_FOLDERUPPERCASE },
			{ MD_CHECKBOX_FILESLOWERCASE, PVS_FILELOWERCASE },
			{ MD_CHECKBOX_UPPERTOLOWERCASE, PVS_FILEUPPERTOLOWERCASE },
		};

		if (!AddNewMode)
		{
			auto& CurrentSettings = ViewSettings[ModeNumber];

			for (const auto& i : ModesFlagsMapping)
			{
				ModeDlg[i.first].Selected = CurrentSettings.Flags & i.second? BSTATE_CHECKED : BSTATE_UNCHECKED;
			}

			ModeDlg[MD_EDITNAME].strData = CurrentSettings.Name;
			std::tie(ModeDlg[MD_EDITTYPES].strData, ModeDlg[MD_EDITWIDTHS].strData) = SerialiseViewSettings(CurrentSettings.PanelColumns);
			std::tie(ModeDlg[MD_EDITSTATUSTYPES].strData, ModeDlg[MD_EDITSTATUSWIDTHS].strData) = SerialiseViewSettings(CurrentSettings.StatusColumns);
		}

		int ExitCode;

		{
			const auto Dlg = Dialog::create(ModeDlg);
			Dlg->SetPosition(-1,-1,76,19);
			Dlg->SetHelp(L"PanelViewModes");
			Dlg->SetId(PanelViewModesEditId);
			Dlg->Process();
			ExitCode=Dlg->GetExitCode();
		}

		if (ExitCode == MD_BUTTON_OK || ExitCode == MD_BUTTON_RESET)
		{
			PanelViewSettings NewSettings;

			if (ExitCode == MD_BUTTON_OK)
			{
				for(const auto& i: ModesFlagsMapping)
				{
					if (ModeDlg[i.first].Selected == BSTATE_CHECKED)
						NewSettings.Flags |= i.second;
				}

				NewSettings.Name = ModeDlg[MD_EDITNAME].strData;
				NewSettings.PanelColumns = DeserialiseViewSettings(ModeDlg[MD_EDITTYPES].strData, ModeDlg[MD_EDITWIDTHS].strData);
				NewSettings.StatusColumns = DeserialiseViewSettings(ModeDlg[MD_EDITSTATUSTYPES].strData, ModeDlg[MD_EDITSTATUSWIDTHS].strData);
			}
			else
			{
				ResetViewModes(make_range(&NewSettings, 1), ModeNumber);
			}

			if (AddNewMode)
			{
				AddViewSettings(ModeNumber, std::move(NewSettings));
			}
			else
			{
				SetViewSettings(ModeNumber, std::move(NewSettings));
			}

			const auto& Panels = Global->CtrlObject->Cp();
			const auto& LPanel = Panels->LeftPanel();
			const auto& RPanel = Panels->RightPanel();

			LPanel->SortFileList(true);
			RPanel->SortFileList(true);
			Panels->SetScreenPosition();
			// ???
			LPanel->SetViewMode(LPanel->GetViewMode());
			RPanel->SetViewMode(RPanel->GetViewMode());
			LPanel->Redraw();
			RPanel->Redraw();
		}
	}
}

struct FARConfigItem
{
	size_t ApiRoot;
	const wchar_t *KeyName;
	const wchar_t *ValName;
	Option* Value;   // адрес переменной, куда помещаем данные
	any Default;

	FarListItem MakeListItem(string& ListItemString) const
	{
		FarListItem Item{};

		string Type = Value->GetType();
		Type.resize(std::max(Type.size(), size_t(7)), L' ');

		ListItemString = concat(KeyName, L'.', ValName);
		ListItemString.resize(std::max(ListItemString.size(), size_t(42)), L' ');
		append(ListItemString, BoxSymbols[BS_V1], Type, BoxSymbols[BS_V1], Value->toString(), Value->ExInfo());
		if(!Value->IsDefault(Default))
		{
			Item.Flags = LIF_CHECKED|L'*';
		}
		Item.Text = ListItemString.data();
		return Item;
	}

	bool Edit(bool Hex) const
	{
		DialogBuilder Builder;
		Builder.AddText(concat(KeyName, L'.', ValName, L" ("_sv, Value->GetType(), L"):"_sv).data());
		int Result = 0;
		if (!Value->Edit(&Builder, 40, Hex))
		{
			static const lng Buttons[] = { lng::MOk, lng::MReset, lng::MCancel };
			Builder.AddSeparator();
			Builder.AddButtons(make_range(Buttons), 0, 2);
			Result = Builder.ShowDialogEx();
		}
		if(Result == 0 || Result == 1)
		{
			if(Result == 1)
			{
				Value->SetDefault(Default);
			}
			return true;
		}
		return false;
	}
};

static bool ParseIntValue(const string& sValue, long long& iValue)
{
	try
	{
		iValue = std::stoll(sValue);
		return true;
	}
	catch (const std::exception&)
	{
		try
		{
			iValue = std::stoull(sValue);
			return true;
		}
		catch (const std::exception&)
		{
			if (equal_icase(sValue, L"false"_sv))
			{
				iValue = 0;
				return true;
			}

			if (equal_icase(sValue, L"true"_sv))
			{
				iValue = 1;
				return true;
			}

			if (equal_icase(sValue, L"other"_sv))
			{
				iValue = 2;
				return true;
			}
			// TODO: log
		}
	}

	return false;
}


template<class base_type, class derived>
bool detail::OptionImpl<base_type, derived>::ReceiveValue(const GeneralConfig* Storage, const string& KeyName, const string& ValueName, const any& Default)
{
	base_type CfgValue;
	const auto Result = Storage->GetValue(KeyName, ValueName, CfgValue, any_cast<base_type>(Default));
	Set(CfgValue);
	return Result;
}

template<class base_type, class derived>
bool detail::OptionImpl<base_type, derived>::StoreValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, bool always) const
{
	return (!always && !Changed()) || Storage->SetValue(KeyName, ValueName, Get());
}


bool BoolOption::TryParse(const string& value)
{
	long long iValue;
	if (!ParseIntValue(value, iValue))
		return false;

	Set(iValue != 0);
	return true;
}

bool BoolOption::Edit(DialogBuilder* Builder, int Width, int Param)
{
	Set(!Get());
	return true;
}

void BoolOption::Export(FarSettingsItem& To) const
{
	To.Type = FST_QWORD;
	To.Number = Get();
}


bool Bool3Option::TryParse(const string& value)
{
	long long iValue;
	if (!ParseIntValue(value, iValue))
		return false;

	Set(iValue);
	return true;
}

bool Bool3Option::Edit(DialogBuilder* Builder, int Width, int Param)
{
	Set((Get() + 1) % 3);
	return true;
}

void Bool3Option::Export(FarSettingsItem& To) const
{
	To.Type = FST_QWORD;
	To.Number = Get();
}


bool IntOption::TryParse(const string& value)
{
	long long iValue;
	if (!ParseIntValue(value, iValue))
		return false;

	Set(iValue);
	return true;
}

bool IntOption::Edit(DialogBuilder* Builder, int Width, int Param)
{
	if (Param)
		Builder->AddHexEditField(*this, Width);
	else
		Builder->AddIntEditField(*this, Width);
	return false;
}

void IntOption::Export(FarSettingsItem& To) const
{
	To.Type = FST_QWORD;
	To.Number = Get();
}


string IntOption::ExInfo() const
{
	return format(L" = 0x{0:X}", as_unsigned(Get()));
}


bool StringOption::Edit(DialogBuilder* Builder, int Width, int Param)
{
	Builder->AddEditField(*this, Width);
	return false;
}

void StringOption::Export(FarSettingsItem& To) const
{
	To.Type = FST_STRING;
	To.String = Get().data();
}


class Options::farconfig
{
public:
	NONCOPYABLE(farconfig);
	MOVABLE(farconfig);

	using iterator = const FARConfigItem*;
	using const_iterator = iterator;
	using value_type = FARConfigItem;

	farconfig(const FARConfigItem* Items, size_t Size, GeneralConfig* cfg):
		m_items(Items),
		m_size(Size),
		m_cfg(cfg)
	{
	}

	iterator begin() const { return m_items; }
	iterator end() const { return m_items + m_size; }
	const_iterator cbegin() const { return begin(); }
	const_iterator cend() const { return end(); }
	size_t size() const { return m_size; }
	auto& operator[](size_t i) const { return m_items[i]; }

	GeneralConfig* GetConfig() const { return m_cfg; }

private:
	const FARConfigItem* m_items;
	size_t m_size;
	GeneralConfig* m_cfg;
};

Options::Options():
	strWordDiv(EdOpt.strWordDiv),
	KnownIDs(),
	ReadOnlyConfig(-1),
	UseExceptionHandler(0),
	ElevationMode(0),
	WindowMode(-1),
	ViewSettings(m_ViewSettings),
	m_ConfigStrings(),
	m_CurrentConfigType(config_type::roaming),
	m_ViewSettings(predefined_panel_modes_count),
	m_ViewSettingsChanged(false)
{
	const auto& TabSizeValidator = [](long long TabSize)
	{
		return InRange(1, TabSize, 512)? TabSize : DefaultTabSize;
	};

	EdOpt.TabSize.SetValidator(TabSizeValidator);
	ViOpt.TabSize.SetValidator(TabSizeValidator);

	ViOpt.MaxLineSize.SetValidator([](long long Value)
	{
		return Value?
			std::clamp(Value, static_cast<long long>(ViewerOptions::eMinLineSize), static_cast<long long>(ViewerOptions::eMaxLineSize)) :
			static_cast<long long>(ViewerOptions::eDefLineSize);
	});

	PluginMaxReadData.SetValidator([](long long Value) { return std::max(Value, 0x20000ll); });

	// Исключаем случайное стирание разделителей
	EdOpt.strWordDiv.SetValidator([](const string& Value) { return Value.empty()? WordDiv0 : Value; });
	XLat.strWordDivForXlat.SetValidator([](const string& Value) { return Value.empty()? WordDivForXlat0 : Value; });

	PanelRightClickRule.SetValidator([](long long Value) { return Value %= 3; });
	PanelCtrlAltShiftRule.SetValidator([](long long Value) { return Value %= 3; });

	HelpTabSize.SetValidator([](long long Value) { return DefaultTabSize; }); // пока жестко пропишем...

	const auto& MacroKeyValidator = [](const string& Value, DWORD& Key, const string& DefaultValue, DWORD DefaultKey)
	{
		if ((Key = KeyNameToKey(Value)) == static_cast<DWORD>(-1))
		{
			Key = DefaultKey;
			return DefaultValue;
		}
		return Value;
	};

	Macro.strKeyMacroCtrlDot.SetValidator([&](const string& Value)
	{
		return MacroKeyValidator(Value, Macro.KeyMacroCtrlDot, L"Ctrl.", KEY_CTRLDOT);
	});

	Macro.strKeyMacroRCtrlDot.SetValidator([&](const string& Value)
	{
		return MacroKeyValidator(Value, Macro.KeyMacroRCtrlDot, L"RCtrl.", KEY_RCTRLDOT);
	});

	Macro.strKeyMacroCtrlShiftDot.SetValidator([&](const string& Value)
	{
		return MacroKeyValidator(Value, Macro.KeyMacroCtrlShiftDot, L"CtrlShift.", KEY_CTRLSHIFTDOT);
	});

	Macro.strKeyMacroRCtrlShiftDot.SetValidator([&](const string& Value)
	{
		return MacroKeyValidator(Value, Macro.KeyMacroRCtrlShiftDot, L"RCtrlShift.", KEY_RCTRL | KEY_SHIFT | KEY_DOT);
	});

	// По умолчанию - брать плагины из основного каталога
	LoadPlug.MainPluginDir = true;
	LoadPlug.PluginsPersonal = true;
	LoadPlug.PluginsCacheOnly = false;

	Macro.DisableMacro=0;

	ResetViewModes(make_range(m_ViewSettings.data(), m_ViewSettings.size()));
}

Options::~Options() = default;

void Options::InitConfigData()
{
	static const wchar_t DefaultBoxSymbols[] =
	{
		L'\x2591', L'\x2592', L'\x2593', L'\x2502', L'\x2524', L'\x2561', L'\x2562', L'\x2556',
		L'\x2555', L'\x2563', L'\x2551', L'\x2557', L'\x255D', L'\x255C', L'\x255B', L'\x2510',
		L'\x2514', L'\x2534', L'\x252C', L'\x251C', L'\x2500', L'\x253C', L'\x255E', L'\x255F',
		L'\x255A', L'\x2554', L'\x2569', L'\x2566', L'\x2560', L'\x2550', L'\x256C', L'\x2567',
		L'\x2568', L'\x2564', L'\x2565', L'\x2559', L'\x2558', L'\x2552', L'\x2553', L'\x256B',
		L'\x256A', L'\x2518', L'\x250C', L'\x2588', L'\x2584', L'\x258C', L'\x2590', L'\x2580',
		L'\0'
	};
	static_assert(std::size(DefaultBoxSymbols) == BS_COUNT + 1);

	const auto strDefaultLanguage = GetFarIniString(L"General", L"DefaultLanguage", L"English");

	#define OPT_DEF(option, def) &option, std::remove_reference_t<decltype(option)>::underlying_type(def)

	static const FARConfigItem RoamingData[] =
	{
		{FSSF_PRIVATE,       NKeyCmdline, L"AutoComplete", OPT_DEF(CmdLine.AutoComplete, true)},
		{FSSF_PRIVATE,       NKeyCmdline, L"EditBlock", OPT_DEF(CmdLine.EditBlock, false)},
		{FSSF_PRIVATE,       NKeyCmdline, L"DelRemovesBlocks", OPT_DEF(CmdLine.DelRemovesBlocks, true)},
		{FSSF_PRIVATE,       NKeyCmdline, L"PromptFormat", OPT_DEF(CmdLine.strPromptFormat, L"$p$g")},
		{FSSF_PRIVATE,       NKeyCmdline, L"UsePromptFormat", OPT_DEF(CmdLine.UsePromptFormat, false)},

		{FSSF_PRIVATE,       NKeyCodePages,L"CPMenuMode", OPT_DEF(CPMenuMode, false)},
		{FSSF_PRIVATE,       NKeyCodePages,L"NoAutoDetectCP", OPT_DEF(strNoAutoDetectCP, L"")},

		{FSSF_PRIVATE,       NKeyConfirmations,L"AllowReedit", OPT_DEF(Confirm.AllowReedit, true)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Copy", OPT_DEF(Confirm.Copy, true)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Delete", OPT_DEF(Confirm.Delete, true)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"DeleteFolder", OPT_DEF(Confirm.DeleteFolder, true)},
		{FSSF_PRIVATE,       NKeyConfirmations,L"DetachVHD", OPT_DEF(Confirm.DetachVHD, true)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Drag", OPT_DEF(Confirm.Drag, true)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Esc", OPT_DEF(Confirm.Esc, true)},
		{FSSF_PRIVATE,       NKeyConfirmations,L"EscTwiceToInterrupt", OPT_DEF(Confirm.EscTwiceToInterrupt, false)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Exit", OPT_DEF(Confirm.Exit, true)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"HistoryClear", OPT_DEF(Confirm.HistoryClear, true)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Move", OPT_DEF(Confirm.Move, true)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"RemoveConnection", OPT_DEF(Confirm.RemoveConnection, true)},
		{FSSF_PRIVATE,       NKeyConfirmations,L"RemoveHotPlug", OPT_DEF(Confirm.RemoveHotPlug, true)},
		{FSSF_PRIVATE,       NKeyConfirmations,L"RemoveSUBST", OPT_DEF(Confirm.RemoveSUBST, true)},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"RO", OPT_DEF(Confirm.RO, true)},

		{FSSF_PRIVATE,       NKeyDescriptions,L"AnsiByDefault", OPT_DEF(Diz.AnsiByDefault, false)},
		{FSSF_PRIVATE,       NKeyDescriptions,L"ListNames", OPT_DEF(Diz.strListNames, L"Descript.ion,Files.bbs")},
		{FSSF_PRIVATE,       NKeyDescriptions,L"ROUpdate", OPT_DEF(Diz.ROUpdate, false)},
		{FSSF_PRIVATE,       NKeyDescriptions,L"SaveInUTF", OPT_DEF(Diz.SaveInUTF, false)},
		{FSSF_PRIVATE,       NKeyDescriptions,L"SetHidden", OPT_DEF(Diz.SetHidden, true)},
		{FSSF_PRIVATE,       NKeyDescriptions,L"StartPos", OPT_DEF(Diz.StartPos, 0)},
		{FSSF_PRIVATE,       NKeyDescriptions,L"UpdateMode", OPT_DEF(Diz.UpdateMode, DIZ_UPDATE_IF_DISPLAYED)},

		{FSSF_PRIVATE,       NKeyDialog,L"AutoComplete", OPT_DEF(Dialogs.AutoComplete, true)},
		{FSSF_PRIVATE,       NKeyDialog,L"CBoxMaxHeight", OPT_DEF(Dialogs.CBoxMaxHeight, 8)},
		{FSSF_DIALOG,        NKeyDialog,L"EditBlock", OPT_DEF(Dialogs.EditBlock, false)},
		{FSSF_PRIVATE,       NKeyDialog,L"EditHistory", OPT_DEF(Dialogs.EditHistory, true)},
		{FSSF_DIALOG,        NKeyDialog,L"DelRemovesBlocks", OPT_DEF(Dialogs.DelRemovesBlocks, true)},
		{FSSF_DIALOG,        NKeyDialog,L"EULBsClear", OPT_DEF(Dialogs.EULBsClear, false)},
		{FSSF_PRIVATE,       NKeyDialog,L"MouseButton", OPT_DEF(Dialogs.MouseButton, 0xFFFF)},

		{FSSF_PRIVATE,       NKeyEditor,L"AddUnicodeBOM", OPT_DEF(EdOpt.AddUnicodeBOM, true)},
		{FSSF_PRIVATE,       NKeyEditor,L"AllowEmptySpaceAfterEof", OPT_DEF(EdOpt.AllowEmptySpaceAfterEof,false)},
		{FSSF_PRIVATE,       NKeyEditor,L"AutoDetectCodePage", OPT_DEF(EdOpt.AutoDetectCodePage, true)},
		{FSSF_PRIVATE,       NKeyEditor,L"AutoIndent", OPT_DEF(EdOpt.AutoIndent, false)},
		{FSSF_PRIVATE,       NKeyEditor,L"BSLikeDel", OPT_DEF(EdOpt.BSLikeDel, true)},
		{FSSF_PRIVATE,       NKeyEditor,L"CharCodeBase", OPT_DEF(EdOpt.CharCodeBase, 1)},
		{FSSF_PRIVATE,       NKeyEditor,L"DefaultCodePage", OPT_DEF(EdOpt.DefaultCodePage, GetACP())},
		{FSSF_PRIVATE,       NKeyEditor,L"F8CPs", OPT_DEF(EdOpt.strF8CPs, L"")},
		{FSSF_PRIVATE,       NKeyEditor,L"DelRemovesBlocks", OPT_DEF(EdOpt.DelRemovesBlocks, true)},
		{FSSF_PRIVATE,       NKeyEditor,L"EditOpenedForWrite", OPT_DEF(EdOpt.EditOpenedForWrite, true)},
		{FSSF_PRIVATE,       NKeyEditor,L"EditorCursorBeyondEOL", OPT_DEF(EdOpt.CursorBeyondEOL, true)},
		{FSSF_PRIVATE,       NKeyEditor,L"ExpandTabs", OPT_DEF(EdOpt.ExpandTabs, 0)},
		{FSSF_PRIVATE,       NKeyEditor,L"ExternalEditorName", OPT_DEF(strExternalEditor, L"")},
		{FSSF_PRIVATE,       NKeyEditor,L"FileSizeLimit", OPT_DEF(EdOpt.FileSizeLimit, 0)},
		{FSSF_PRIVATE,       NKeyEditor,L"KeepEditorEOL", OPT_DEF(EdOpt.KeepEOL, true)},
		{FSSF_PRIVATE,       NKeyEditor,L"NewFileUnixEOL", OPT_DEF(EdOpt.NewFileUnixEOL, false)},
		{FSSF_PRIVATE,       NKeyEditor,L"PersistentBlocks", OPT_DEF(EdOpt.PersistentBlocks, false)},
		{FSSF_PRIVATE,       NKeyEditor,L"ReadOnlyLock", OPT_DEF(EdOpt.ReadOnlyLock, 0)},
		{FSSF_PRIVATE,       NKeyEditor,L"SaveEditorPos", OPT_DEF(EdOpt.SavePos, true)},
		{FSSF_PRIVATE,       NKeyEditor,L"SaveEditorShortPos", OPT_DEF(EdOpt.SaveShortPos, true)},
		{FSSF_PRIVATE,       NKeyEditor,L"SearchRegexp", OPT_DEF(EdOpt.SearchRegexp, false)},
		{FSSF_PRIVATE,       NKeyEditor,L"SearchSelFound", OPT_DEF(EdOpt.SearchSelFound, false)},
		{FSSF_PRIVATE,       NKeyEditor,L"SearchCursorAtEnd", OPT_DEF(EdOpt.SearchCursorAtEnd, false)},
		{FSSF_PRIVATE,       NKeyEditor,L"ShowKeyBar", OPT_DEF(EdOpt.ShowKeyBar, true)},
		{FSSF_PRIVATE,       NKeyEditor,L"ShowScrollBar", OPT_DEF(EdOpt.ShowScrollBar, false)},
		{FSSF_PRIVATE,       NKeyEditor,L"ShowTitleBar", OPT_DEF(EdOpt.ShowTitleBar, true)},
		{FSSF_PRIVATE,       NKeyEditor,L"ShowWhiteSpace", OPT_DEF(EdOpt.ShowWhiteSpace, 0)},
		{FSSF_PRIVATE,       NKeyEditor,L"TabSize", OPT_DEF(EdOpt.TabSize, DefaultTabSize)},
		{FSSF_PRIVATE,       NKeyEditor,L"UndoDataSize", OPT_DEF(EdOpt.UndoSize, 100*1024*1024)},
		{FSSF_PRIVATE,       NKeyEditor,L"UseExternalEditor", OPT_DEF(EdOpt.UseExternalEditor, false)},
		{FSSF_EDITOR,        NKeyEditor,L"WordDiv", OPT_DEF(EdOpt.strWordDiv, WordDiv0)},

		{FSSF_PRIVATE,       NKeyHelp,L"ActivateURL", OPT_DEF(HelpURLRules, 1)},
		{FSSF_PRIVATE,       NKeyHelp,L"HelpSearchRegexp", OPT_DEF(HelpSearchRegexp, false)},

		{FSSF_PRIVATE,       NKeyCommandHistory, L"Count", OPT_DEF(HistoryCount, 1000)},
		{FSSF_PRIVATE,       NKeyCommandHistory, L"Lifetime", OPT_DEF(HistoryLifetime, 90)},
		{FSSF_PRIVATE,       NKeyDialogHistory, L"Count", OPT_DEF(DialogsHistoryCount, 1000)},
		{FSSF_PRIVATE,       NKeyDialogHistory, L"Lifetime", OPT_DEF(DialogsHistoryLifetime, 90)},
		{FSSF_PRIVATE,       NKeyFolderHistory, L"Count", OPT_DEF(FoldersHistoryCount, 1000)},
		{FSSF_PRIVATE,       NKeyFolderHistory, L"Lifetime", OPT_DEF(FoldersHistoryLifetime, 90)},
		{FSSF_PRIVATE,       NKeyViewEditHistory, L"Count", OPT_DEF(ViewHistoryCount, 1000)},
		{FSSF_PRIVATE,       NKeyViewEditHistory, L"Lifetime", OPT_DEF(ViewHistoryLifetime, 90)},

		{FSSF_PRIVATE,       NKeyInterface,L"DelHighlightSelected", OPT_DEF(DelOpt.HighlightSelected, true)},
		{FSSF_PRIVATE,       NKeyInterface,L"DelShowSelected", OPT_DEF(DelOpt.ShowSelected, 10)},
		{FSSF_PRIVATE,       NKeyInterface,L"DelShowTotal", OPT_DEF(DelOpt.ShowTotal, false)},

		{FSSF_PRIVATE,       NKeyInterface, L"AltF9", OPT_DEF(AltF9, true)},
		{FSSF_PRIVATE,       NKeyInterface, L"ClearType", OPT_DEF(ClearType, true)},
		{FSSF_PRIVATE,       NKeyInterface, L"CopyShowTotal", OPT_DEF(CMOpt.CopyShowTotal, true)},
		{FSSF_PRIVATE,       NKeyInterface, L"CtrlPgUp", OPT_DEF(PgUpChangeDisk, 1)},
		{FSSF_PRIVATE,       NKeyInterface, L"CursorSize1", OPT_DEF(CursorSize[0], 15)},
		{FSSF_PRIVATE,       NKeyInterface, L"CursorSize2", OPT_DEF(CursorSize[1], 10)},
		{FSSF_PRIVATE,       NKeyInterface, L"CursorSize3", OPT_DEF(CursorSize[2], 99)},
		{FSSF_PRIVATE,       NKeyInterface, L"CursorSize4", OPT_DEF(CursorSize[3], 99)},
		{FSSF_PRIVATE,       NKeyInterface, L"EditorTitleFormat", OPT_DEF(strEditorTitleFormat, L"%Lng %File")},
		{FSSF_PRIVATE,       NKeyInterface, L"FormatNumberSeparators", OPT_DEF(FormatNumberSeparators, L"")},
		{FSSF_PRIVATE,       NKeyInterface, L"Mouse", OPT_DEF(Mouse, true)},
		{FSSF_PRIVATE,       NKeyInterface, L"SetIcon", OPT_DEF(SetIcon, false)},
		{FSSF_PRIVATE,       NKeyInterface, L"SetAdminIcon", OPT_DEF(SetAdminIcon, true)},
		{FSSF_PRIVATE,       NKeyInterface, L"ShowDotsInRoot", OPT_DEF(ShowDotsInRoot, false)},
		{FSSF_INTERFACE,     NKeyInterface, L"ShowMenuBar", OPT_DEF(ShowMenuBar, false)},
		{FSSF_PRIVATE,       NKeyInterface, L"RedrawTimeout", OPT_DEF(RedrawTimeout, 200)},
		{FSSF_PRIVATE,       NKeyInterface, L"TitleAddons", OPT_DEF(strTitleAddons, L"%Ver.%Build %Platform %Admin")},
		{FSSF_PRIVATE,       NKeyInterface, L"ViewerTitleFormat", OPT_DEF(strViewerTitleFormat, L"%Lng %File")},

		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"Append", OPT_DEF(AutoComplete.AppendCompletion, false)},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"ModalList", OPT_DEF(AutoComplete.ModalList, false)},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"ShowList", OPT_DEF(AutoComplete.ShowList, true)},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"UseFilesystem", OPT_DEF(AutoComplete.UseFilesystem, 1)},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"UseHistory", OPT_DEF(AutoComplete.UseHistory, 1)},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"UsePath", OPT_DEF(AutoComplete.UsePath, 1)},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"UseEnvironment", OPT_DEF(AutoComplete.UseEnvironment, 1)},

		{FSSF_PRIVATE,       NKeyLanguage, L"Main", OPT_DEF(strLanguage, strDefaultLanguage)},
		{FSSF_PRIVATE,       NKeyLanguage, L"Help", OPT_DEF(strHelpLanguage, strDefaultLanguage)},

		{FSSF_PRIVATE,       NKeyLayout,L"FullscreenHelp", OPT_DEF(FullScreenHelp, false)},
		{FSSF_PRIVATE,       NKeyLayout,L"LeftHeightDecrement", OPT_DEF(LeftHeightDecrement, 0)},
		{FSSF_PRIVATE,       NKeyLayout,L"RightHeightDecrement", OPT_DEF(RightHeightDecrement, 0)},
		{FSSF_PRIVATE,       NKeyLayout,L"WidthDecrement", OPT_DEF(WidthDecrement, 0)},

		{FSSF_PRIVATE,       NKeyKeyMacros,L"DateFormat", OPT_DEF(Macro.strDateFormat, L"%a %b %d %H:%M:%S %Z %Y")},

		{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordCtrlDot", OPT_DEF(Macro.strKeyMacroCtrlDot, L"Ctrl.")},
		{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordRCtrlDot", OPT_DEF(Macro.strKeyMacroRCtrlDot, L"RCtrl.")},
		{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordCtrlShiftDot", OPT_DEF(Macro.strKeyMacroCtrlShiftDot, L"CtrlShift.")},
		{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordRCtrlShiftDot", OPT_DEF(Macro.strKeyMacroRCtrlShiftDot, L"RCtrlShift.")},

		{FSSF_PRIVATE,       NKeyKeyMacros,L"ShowPlayIndicator", OPT_DEF(Macro.ShowPlayIndicator, true)},

		{FSSF_PRIVATE,       NKeyPanel,L"AutoUpdateLimit", OPT_DEF(AutoUpdateLimit, 0)},
		{FSSF_PRIVATE,       NKeyPanel,L"CtrlAltShiftRule", OPT_DEF(PanelCtrlAltShiftRule, 0)},
		{FSSF_PRIVATE,       NKeyPanel,L"CtrlFRule", OPT_DEF(PanelCtrlFRule, false)},
		{FSSF_PRIVATE,       NKeyPanel,L"Highlight", OPT_DEF(Highlight, true)},
		{FSSF_PRIVATE,       NKeyPanel,L"ReverseSort", OPT_DEF(ReverseSort, true)},
		{FSSF_PRIVATE,       NKeyPanel,L"RememberLogicalDrives", OPT_DEF(RememberLogicalDrives, false)},
		{FSSF_PRIVATE,       NKeyPanel,L"RightClickRule", OPT_DEF(PanelRightClickRule, 2)},
		{FSSF_PRIVATE,       NKeyPanel,L"SelectFolders", OPT_DEF(SelectFolders, false)},
		{FSSF_PRIVATE,       NKeyPanel,L"ShellRightLeftArrowsRule", OPT_DEF(ShellRightLeftArrowsRule, false)},
		{FSSF_PRIVATE,       NKeyPanel,L"ShowBytes", OPT_DEF(ShowBytes, false) },
		{FSSF_PANEL,         NKeyPanel,L"ShowHidden", OPT_DEF(ShowHidden, true)},
		{FSSF_PANEL,         NKeyPanel,L"ShortcutAlwaysChdir", OPT_DEF(ShortcutAlwaysChdir, false)},
		{FSSF_PRIVATE,       NKeyPanel,L"SortFolderExt", OPT_DEF(SortFolderExt, false)},
		{FSSF_PRIVATE,       NKeyPanel,L"RightClickSelect", OPT_DEF(RightClickSelect, false)},

		{FSSF_PRIVATE,       NKeyPanelInfo,L"InfoComputerNameFormat", OPT_DEF(InfoPanel.ComputerNameFormat, ComputerNamePhysicalNetBIOS)},
		{FSSF_PRIVATE,       NKeyPanelInfo,L"InfoUserNameFormat", OPT_DEF(InfoPanel.UserNameFormat, NameUserPrincipal)},
		{FSSF_PRIVATE,       NKeyPanelInfo,L"ShowCDInfo", OPT_DEF(InfoPanel.ShowCDInfo, true)},
		{FSSF_PRIVATE,       NKeyPanelInfo,L"ShowPowerStatus", OPT_DEF(InfoPanel.ShowPowerStatus, false)},

		{FSSF_PRIVATE,       NKeyPanelLayout,L"ColoredGlobalColumnSeparator", OPT_DEF(HighlightColumnSeparator, true)},
		{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"ColumnTitles", OPT_DEF(ShowColumnTitles, true)},
		{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"DetailedJunction", OPT_DEF(PanelDetailedJunction, false)},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"DoubleGlobalColumnSeparator", OPT_DEF(DoubleGlobalColumnSeparator, false)},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"FreeInfo", OPT_DEF(ShowPanelFree, false)},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"ScreensNumber", OPT_DEF(ShowScreensNumber, 1)},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"Scrollbar", OPT_DEF(ShowPanelScrollbar, false)},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"ScrollbarMenu", OPT_DEF(ShowMenuScrollbar, true)},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"ShowUnknownReparsePoint", OPT_DEF(ShowUnknownReparsePoint, false)},
		{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"SortMode", OPT_DEF(ShowSortMode, true)},
		{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"StatusLine", OPT_DEF(ShowPanelStatus, true)},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"TotalInfo", OPT_DEF(ShowPanelTotals, true)},

		{FSSF_PRIVATE,       NKeyPanelLeft,L"CaseSensitiveSort", OPT_DEF(LeftPanel.CaseSensitiveSort, false)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"DirectoriesFirst", OPT_DEF(LeftPanel.DirectoriesFirst, true)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"NumericSort", OPT_DEF(LeftPanel.NumericSort, false)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"SelectedFirst", OPT_DEF(LeftPanel.SelectedFirst, false)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"ShortNames", OPT_DEF(LeftPanel.ShowShortNames, false)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"SortGroups", OPT_DEF(LeftPanel.SortGroups, false)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"SortMode", OPT_DEF(LeftPanel.SortMode, 1)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"ReverseSortOrder", OPT_DEF(LeftPanel.ReverseSortOrder, false)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"Type", OPT_DEF(LeftPanel.m_Type, 0)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"ViewMode", OPT_DEF(LeftPanel.ViewMode, 2)},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"Visible", OPT_DEF(LeftPanel.Visible, true)},

		{FSSF_PRIVATE,       NKeyPanelRight,L"CaseSensitiveSort", OPT_DEF(RightPanel.CaseSensitiveSort, false)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"DirectoriesFirst", OPT_DEF(RightPanel.DirectoriesFirst, true)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"NumericSort", OPT_DEF(RightPanel.NumericSort, false)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"SelectedFirst", OPT_DEF(RightPanel.SelectedFirst, false)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"ShortNames", OPT_DEF(RightPanel.ShowShortNames, false)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"SortGroups", OPT_DEF(RightPanel.SortGroups, false)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"SortMode", OPT_DEF(RightPanel.SortMode, 1)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"ReverseSortOrder", OPT_DEF(RightPanel.ReverseSortOrder, false)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"Type", OPT_DEF(RightPanel.m_Type, 0)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"ViewMode", OPT_DEF(RightPanel.ViewMode, 2)},
		{FSSF_PRIVATE,       NKeyPanelRight,L"Visible", OPT_DEF(RightPanel.Visible, true)},

		{FSSF_PRIVATE,       NKeyPanelTree,L"TurnOffCompletely", OPT_DEF(Tree.TurnOffCompletely, true)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"AutoChangeFolder", OPT_DEF(Tree.AutoChangeFolder, false)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"MinTreeCount", OPT_DEF(Tree.MinTreeCount, 4)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"TreeFileAttr", OPT_DEF(Tree.TreeFileAttr, FILE_ATTRIBUTE_HIDDEN)},
	#if defined(TREEFILE_PROJECT)
		{FSSF_PRIVATE,       NKeyPanelTree,L"CDDisk", OPT_DEF(Tree.CDDisk, 2)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"CDDiskTemplate,0", OPT_DEF(Tree.strCDDisk, constCDDiskTemplate)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"ExceptPath", OPT_DEF(Tree.strExceptPath, L"")},
		{FSSF_PRIVATE,       NKeyPanelTree,L"LocalDisk", OPT_DEF(Tree.LocalDisk, 2)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"LocalDiskTemplate", OPT_DEF(Tree.strLocalDisk, constLocalDiskTemplate)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"NetDisk", OPT_DEF(Tree.NetDisk, 2)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"NetPath", OPT_DEF(Tree.NetPath, 2)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"NetDiskTemplate", OPT_DEF(Tree.strNetDisk, constNetDiskTemplate)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"NetPathTemplate", OPT_DEF(Tree.strNetPath, constNetPathTemplate)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"RemovableDisk", OPT_DEF(Tree.RemovableDisk, 2)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"RemovableDiskTemplate,", OPT_DEF(Tree.strRemovableDisk, constRemovableDiskTemplate)},
		{FSSF_PRIVATE,       NKeyPanelTree,L"SaveLocalPath", OPT_DEF(Tree.strSaveLocalPath, L"")},
		{FSSF_PRIVATE,       NKeyPanelTree,L"SaveNetPath", OPT_DEF(Tree.strSaveNetPath, L"")},
	#endif
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"EvenIfOnlyOnePlugin", OPT_DEF(PluginConfirm.EvenIfOnlyOnePlugin, false)},
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"OpenFilePlugin", OPT_DEF(PluginConfirm.OpenFilePlugin, 0)},
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"Prefix", OPT_DEF(PluginConfirm.Prefix, false)},
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"SetFindList", OPT_DEF(PluginConfirm.SetFindList, false)},
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"StandardAssociation", OPT_DEF(PluginConfirm.StandardAssociation, false)},

		{FSSF_PRIVATE,       NKeyPolicies,L"ShowHiddenDrives", OPT_DEF(Policies.ShowHiddenDrives, true)},

		{FSSF_PRIVATE,       NKeyScreen, L"Clock", OPT_DEF(Clock, true)},
		{FSSF_PRIVATE,       NKeyScreen, L"DeltaX", OPT_DEF(ScrSize.DeltaX, 0)},
		{FSSF_PRIVATE,       NKeyScreen, L"DeltaY", OPT_DEF(ScrSize.DeltaY, 0)},
		{FSSF_SCREEN,        NKeyScreen, L"KeyBar", OPT_DEF(ShowKeyBar, true)},
		{FSSF_PRIVATE,       NKeyScreen, L"ScreenSaver", OPT_DEF(ScreenSaver, false)},
		{FSSF_PRIVATE,       NKeyScreen, L"ScreenSaverTime", OPT_DEF(ScreenSaverTime, 5)},
		{FSSF_PRIVATE,       NKeyScreen, L"ViewerEditorClock", OPT_DEF(ViewerEditorClock, true)},

		{FSSF_PRIVATE,       NKeySystem,L"AllCtrlAltShiftRule", OPT_DEF(AllCtrlAltShiftRule, 0x0000FFFF)},
		{FSSF_PRIVATE,       NKeySystem,L"AutoSaveSetup", OPT_DEF(AutoSaveSetup, false)},
		{FSSF_PRIVATE,       NKeySystem,L"AutoUpdateRemoteDrive", OPT_DEF(AutoUpdateRemoteDrive, true)},
		{FSSF_PRIVATE,       NKeySystem,L"BoxSymbols", OPT_DEF(strBoxSymbols, DefaultBoxSymbols)},
		{FSSF_PRIVATE,       NKeySystem,L"CASRule", OPT_DEF(CASRule, -1)},
		{FSSF_PRIVATE,       NKeySystem,L"CloseCDGate", OPT_DEF(CloseCDGate, true)},
		{FSSF_PRIVATE,       NKeySystem,L"CmdHistoryRule", OPT_DEF(CmdHistoryRule, false)},
		{FSSF_PRIVATE,       NKeySystem,L"ConsoleDetachKey", OPT_DEF(ConsoleDetachKey, L"CtrlShiftTab")},
		{FSSF_PRIVATE,       NKeySystem,L"CopyBufferSize", OPT_DEF(CMOpt.BufferSize, 0)},
		{FSSF_SYSTEM,        NKeySystem,L"CopyOpened", OPT_DEF(CMOpt.CopyOpened, true)},
		{FSSF_PRIVATE,       NKeySystem,L"CopyTimeRule",  OPT_DEF(CMOpt.CopyTimeRule, 3)},
		{FSSF_PRIVATE,       NKeySystem,L"CopySecurityOptions", OPT_DEF(CMOpt.CopySecurityOptions, 0)},
		{FSSF_SYSTEM,        NKeySystem,L"DeleteToRecycleBin", OPT_DEF(DeleteToRecycleBin, true)},
		{FSSF_PRIVATE,       NKeySystem,L"DelThreadPriority", OPT_DEF(DelThreadPriority, THREAD_PRIORITY_NORMAL)},
		{FSSF_PRIVATE,       NKeySystem,L"DriveDisconnectMode", OPT_DEF(ChangeDriveDisconnectMode, true)},
		{FSSF_PRIVATE,       NKeySystem,L"DriveMenuMode", OPT_DEF(ChangeDriveMode, DRIVE_SHOW_TYPE|DRIVE_SHOW_PLUGINS|DRIVE_SHOW_SIZE_FLOAT|DRIVE_SHOW_CDROM)},
		{FSSF_PRIVATE,       NKeySystem,L"ElevationMode", OPT_DEF(StoredElevationMode, -1)},
		{FSSF_PRIVATE,       NKeySystem,L"ExcludeCmdHistory", OPT_DEF(ExcludeCmdHistory, 0)},
		{FSSF_PRIVATE,       NKeySystem,L"FileSearchMode", OPT_DEF(FindOpt.FileSearchMode, FINDAREA_FROM_CURRENT)},
		{FSSF_PRIVATE,       NKeySystem,L"FindAlternateStreams", OPT_DEF(FindOpt.FindAlternateStreams, false)},
		{FSSF_PRIVATE,       NKeySystem,L"FindCodePage", OPT_DEF(FindCodePage, CP_DEFAULT)},
		{FSSF_PRIVATE,       NKeySystem,L"FindFolders", OPT_DEF(FindOpt.FindFolders, true)},
		{FSSF_PRIVATE,       NKeySystem,L"FindSymLinks", OPT_DEF(FindOpt.FindSymLinks, true)},
		{FSSF_PRIVATE,       NKeySystem,L"FlagPosixSemantics", OPT_DEF(FlagPosixSemantics, true)},
		{FSSF_PRIVATE,       NKeySystem,L"FolderInfo", OPT_DEF(InfoPanel.strFolderInfoFiles, L"DirInfo,File_Id.diz,Descript.ion,ReadMe.*,Read.Me")},
		{FSSF_PRIVATE,       NKeySystem,L"MsWheelDelta", OPT_DEF(MsWheelDelta, 1)},
		{FSSF_PRIVATE,       NKeySystem,L"MsWheelDeltaEdit", OPT_DEF(MsWheelDeltaEdit, 1)},
		{FSSF_PRIVATE,       NKeySystem,L"MsWheelDeltaHelp", OPT_DEF(MsWheelDeltaHelp, 1)},
		{FSSF_PRIVATE,       NKeySystem,L"MsWheelDeltaView", OPT_DEF(MsWheelDeltaView, 1)},
		{FSSF_PRIVATE,       NKeySystem,L"MsHWheelDelta", OPT_DEF(MsHWheelDelta, 1)},
		{FSSF_PRIVATE,       NKeySystem,L"MsHWheelDeltaEdit", OPT_DEF(MsHWheelDeltaEdit, 1)},
		{FSSF_PRIVATE,       NKeySystem,L"MsHWheelDeltaView", OPT_DEF(MsHWheelDeltaView, 1)},
		{FSSF_PRIVATE,       NKeySystem,L"MultiCopy", OPT_DEF(CMOpt.MultiCopy, false)},
		{FSSF_PRIVATE,       NKeySystem,L"MultiMakeDir", OPT_DEF(MultiMakeDir, false)},
	#ifndef NO_WRAPPER
		{FSSF_PRIVATE,       NKeySystem,L"OEMPluginsSupport", OPT_DEF(LoadPlug.OEMPluginsSupport, true)},
	#endif // NO_WRAPPER
		{FSSF_SYSTEM,        NKeySystem,L"PluginMaxReadData", OPT_DEF(PluginMaxReadData, 0x20000)},
		{FSSF_PRIVATE,       NKeySystem,L"QuotedName", OPT_DEF(QuotedName, QUOTEDNAME_INSERT)},
		{FSSF_PRIVATE,       NKeySystem,L"QuotedSymbols", OPT_DEF(strQuotedSymbols, L" &()[]{}^=;!'+,`\xA0")},
		{FSSF_PRIVATE,       NKeySystem,L"SaveHistory", OPT_DEF(SaveHistory, true)},
		{FSSF_PRIVATE,       NKeySystem,L"SaveFoldersHistory", OPT_DEF(SaveFoldersHistory, true)},
		{FSSF_PRIVATE,       NKeySystem,L"SaveViewHistory", OPT_DEF(SaveViewHistory, true)},
		{FSSF_SYSTEM,        NKeySystem,L"ScanJunction", OPT_DEF(ScanJunction, true)},
		{FSSF_PRIVATE,       NKeySystem,L"ScanSymlinks", OPT_DEF(LoadPlug.ScanSymlinks, true)},
		{FSSF_PRIVATE,       NKeySystem,L"SearchInFirstSize", OPT_DEF(FindOpt.strSearchInFirstSize, L"")},
		{FSSF_PRIVATE,       NKeySystem,L"SearchOutFormat", OPT_DEF(FindOpt.strSearchOutFormat, L"D,S,A")},
		{FSSF_PRIVATE,       NKeySystem,L"SearchOutFormatWidth", OPT_DEF(FindOpt.strSearchOutFormatWidth, L"0,0,0")},
		{FSSF_PRIVATE,       NKeySystem,L"SetAttrFolderRules", OPT_DEF(SetAttrFolderRules, true)},
		{FSSF_PRIVATE,       NKeySystem,L"ShowCheckingFile", OPT_DEF(ShowCheckingFile, false)},
		{FSSF_PRIVATE,       NKeySystem,L"ShowStatusInfo", OPT_DEF(InfoPanel.strShowStatusInfo, L"")},
		{FSSF_PRIVATE,       NKeySystem,L"SmartFolderMonitor", OPT_DEF(SmartFolderMonitor, false)},
		{FSSF_PRIVATE,       NKeySystem,L"SubstNameRule", OPT_DEF(SubstNameRule, 2)},
		{FSSF_PRIVATE,       NKeySystem,L"SubstPluginPrefix", OPT_DEF(SubstPluginPrefix, false)},
		{FSSF_PRIVATE,       NKeySystem,L"UpdateEnvironment", OPT_DEF(UpdateEnvironment, false)},
		{FSSF_PRIVATE,       NKeySystem,L"UseFilterInSearch", OPT_DEF(FindOpt.UseFilter, false)},
		{FSSF_PRIVATE,       NKeySystem,L"UseRegisteredTypes", OPT_DEF(UseRegisteredTypes, true)},
		{FSSF_PRIVATE,       NKeySystem,L"UseSystemCopy", OPT_DEF(CMOpt.UseSystemCopy, true)},
		{FSSF_PRIVATE,       NKeySystem,L"WindowMode", OPT_DEF(StoredWindowMode, true)},
		{FSSF_PRIVATE,       NKeySystem,L"WindowMode.StickyX", OPT_DEF(WindowModeStickyX, true)},
		{FSSF_PRIVATE,       NKeySystem,L"WindowMode.StickyY", OPT_DEF(WindowModeStickyY, false)},
		{FSSF_PRIVATE,       NKeySystem,L"WipeSymbol", OPT_DEF(WipeSymbol, 0)},
		{FSSF_SYSTEM,        NKeySystem,L"WordDiv", OPT_DEF(strWordDiv, WordDiv0)},

		{FSSF_PRIVATE,       NKeySystemKnownIDs, L"EMenu", OPT_DEF(KnownIDs.Emenu.StrId, KnownIDs.Emenu.Default)},
		{FSSF_PRIVATE,       NKeySystemKnownIDs, L"Network", OPT_DEF(KnownIDs.Network.StrId, KnownIDs.Network.Default)},
		{FSSF_PRIVATE,       NKeySystemKnownIDs, L"Arclite", OPT_DEF(KnownIDs.Arclite.StrId, KnownIDs.Arclite.Default)},
		{FSSF_PRIVATE,       NKeySystemKnownIDs, L"Luamacro", OPT_DEF(KnownIDs.Luamacro.StrId, KnownIDs.Luamacro.Default)},
		{FSSF_PRIVATE,       NKeySystemKnownIDs, L"Netbox", OPT_DEF(KnownIDs.Netbox.StrId, KnownIDs.Netbox.Default)},

		{FSSF_PRIVATE,       NKeySystemNowell,L"MoveRO", OPT_DEF(Nowell.MoveRO, true)},

		{FSSF_PRIVATE,       NKeySystemException,L"FarEventSvc", OPT_DEF(strExceptEventSvc, L"")},
		{FSSF_PRIVATE,       NKeySystemException,L"Used", OPT_DEF(ExceptUsed, false)},

		{FSSF_PRIVATE,       NKeySystemExecutor,L"~", OPT_DEF(Exec.strHomeDir, L"%FARHOME%")},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"ExcludeCmds", OPT_DEF(Exec.strExcludeCmds, L"")},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"FullTitle", OPT_DEF(Exec.ExecuteFullTitle, false)},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"RestoreCP", OPT_DEF(Exec.RestoreCPAfterExecute, true)},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"UseAppPath", OPT_DEF(Exec.ExecuteUseAppPath, true)},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"UseHomeDir", OPT_DEF(Exec.UseHomeDir, true)},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"Comspec", OPT_DEF(Exec.Comspec, L"%COMSPEC%")},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"ComspecArguments", OPT_DEF(Exec.ComspecArguments, L"/S /C \"{0}\"")},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"ComspecCondition", OPT_DEF(Exec.ComspecCondition, L"")},

		{FSSF_PRIVATE,       NKeyViewer,L"AutoDetectCodePage", OPT_DEF(ViOpt.AutoDetectCodePage, true)},
		{FSSF_PRIVATE,       NKeyViewer,L"DefaultCodePage", OPT_DEF(ViOpt.DefaultCodePage, GetACP())},
		{FSSF_PRIVATE,       NKeyViewer,L"F8CPs", OPT_DEF(ViOpt.strF8CPs, L"")},
		{FSSF_PRIVATE,       NKeyViewer,L"ExternalViewerName", OPT_DEF(strExternalViewer, L"")},
		{FSSF_PRIVATE,       NKeyViewer,L"IsWrap", OPT_DEF(ViOpt.ViewerIsWrap, true)},
		{FSSF_PRIVATE,       NKeyViewer,L"MaxLineSize", OPT_DEF(ViOpt.MaxLineSize, ViewerOptions::eDefLineSize)},
		{FSSF_PRIVATE,       NKeyViewer,L"PersistentBlocks", OPT_DEF(ViOpt.PersistentBlocks, true)},
		{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerCodepage", OPT_DEF(ViOpt.SaveCodepage, true)},
		{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerPos", OPT_DEF(ViOpt.SavePos, true)},
		{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerShortPos", OPT_DEF(ViOpt.SaveShortPos, true)},
		{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerWrapMode", OPT_DEF(ViOpt.SaveWrapMode, false)},
		{FSSF_PRIVATE,       NKeyViewer,L"SearchEditFocus", OPT_DEF(ViOpt.SearchEditFocus, false)},
		{FSSF_PRIVATE,       NKeyViewer,L"SearchRegexp", OPT_DEF(ViOpt.SearchRegexp, false)},
		{FSSF_PRIVATE,       NKeyViewer,L"SearchWrapStop", OPT_DEF(ViOpt.SearchWrapStop, BSTATE_CHECKED)},
		{FSSF_PRIVATE,       NKeyViewer,L"ShowArrows", OPT_DEF(ViOpt.ShowArrows, true)},
		{FSSF_PRIVATE,       NKeyViewer,L"ShowKeyBar", OPT_DEF(ViOpt.ShowKeyBar, true)},
		{FSSF_PRIVATE,       NKeyViewer,L"ShowScrollbar", OPT_DEF(ViOpt.ShowScrollbar, false)},
		{FSSF_PRIVATE,       NKeyViewer,L"TabSize", OPT_DEF(ViOpt.TabSize, DefaultTabSize)},
		{FSSF_PRIVATE,       NKeyViewer,L"ShowTitleBar", OPT_DEF(ViOpt.ShowTitleBar, true)},
		{FSSF_PRIVATE,       NKeyViewer,L"UseExternalViewer", OPT_DEF(ViOpt.UseExternalViewer, false)},
		{FSSF_PRIVATE,       NKeyViewer,L"Visible0x00", OPT_DEF(ViOpt.Visible0x00, false)},
		{FSSF_PRIVATE,       NKeyViewer,L"Wrap", OPT_DEF(ViOpt.ViewerWrap, false)},
		{FSSF_PRIVATE,       NKeyViewer,L"ZeroChar", OPT_DEF(ViOpt.ZeroChar, L'\xB7')}, // middle dot

		{FSSF_PRIVATE,       NKeyVMenu,L"LBtnClick", OPT_DEF(VMenu.LBtnClick, VMENUCLICK_CANCEL)},
		{FSSF_PRIVATE,       NKeyVMenu,L"MBtnClick", OPT_DEF(VMenu.MBtnClick, VMENUCLICK_APPLY)},
		{FSSF_PRIVATE,       NKeyVMenu,L"RBtnClick", OPT_DEF(VMenu.RBtnClick, VMENUCLICK_CANCEL)},

		{FSSF_PRIVATE,       NKeyXLat,L"Flags", OPT_DEF(XLat.Flags, XLAT_SWITCHKEYBLAYOUT|XLAT_CONVERTALLCMDLINE)},
		{FSSF_PRIVATE,       NKeyXLat,L"Layouts", OPT_DEF(XLat.strLayouts, L"")},
		{FSSF_PRIVATE,       NKeyXLat,L"Rules1", OPT_DEF(XLat.Rules[0], L"")},
		{FSSF_PRIVATE,       NKeyXLat,L"Rules2", OPT_DEF(XLat.Rules[1], L"")},
		{FSSF_PRIVATE,       NKeyXLat,L"Rules3", OPT_DEF(XLat.Rules[2], L"")},
		{FSSF_PRIVATE,       NKeyXLat,L"Table1", OPT_DEF(XLat.Table[0], L"")},
		{FSSF_PRIVATE,       NKeyXLat,L"Table2", OPT_DEF(XLat.Table[1], L"")},
		{FSSF_PRIVATE,       NKeyXLat,L"WordDivForXlat", OPT_DEF(XLat.strWordDivForXlat, WordDivForXlat0)},
	};

	static const FARConfigItem LocalData[] =
	{
		{FSSF_PRIVATE,       NKeyPanelLeft,L"CurFile", OPT_DEF(LeftPanel.CurFile, L"")},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"Folder", OPT_DEF(LeftPanel.Folder, L"")},

		{FSSF_PRIVATE,       NKeyPanelRight,L"CurFile", OPT_DEF(RightPanel.CurFile, L"")},
		{FSSF_PRIVATE,       NKeyPanelRight,L"Folder", OPT_DEF(RightPanel.Folder, L"")},

		{FSSF_PRIVATE,       NKeyPanel,L"LeftFocus", OPT_DEF(LeftFocus, true)},

	};

	m_Config.emplace_back(RoamingData, std::size(RoamingData), ConfigProvider().GeneralCfg().get());
	m_Config.emplace_back(LocalData, std::size(LocalData), ConfigProvider().LocalGeneralCfg().get());
}

Options::farconfig& Options::GetConfig(config_type Type)
{
	return m_Config[static_cast<size_t>(Type)];
}
const Options::farconfig& Options::GetConfig(config_type Type) const
{
	return m_Config[static_cast<size_t>(Type)];
}

template<class container, class pred>
static const Option* GetConfigValuePtr(const container& Config, const pred& Pred)
{
	const auto ItemIterator = std::find_if(ALL_CONST_RANGE(Config), Pred);
	return ItemIterator == Config.cend()? nullptr : ItemIterator->Value;
}

const Option* Options::GetConfigValue(const wchar_t *Key, const wchar_t *Name) const
{
	// TODO Use local too?
	return GetConfigValuePtr(GetConfig(config_type::roaming), [&](const auto& i) { return equal_icase(i.KeyName, Key) && equal_icase(i.ValName, Name); });
}

const Option* Options::GetConfigValue(size_t Root, const wchar_t* Name) const
{
	if (Root == FSSF_PRIVATE)
		return nullptr;

	// TODO Use local too?
	return GetConfigValuePtr(GetConfig(config_type::roaming), [&](const FARConfigItem& i) { return Root == i.ApiRoot && equal_icase(i.ValName, Name); });
}

void Options::InitConfig()
{
	if(m_Config.empty())
	{
		InitConfigData();
	}
}

void Options::SetSearchColumns(const string& Columns, const string& Widths)
{
	if (!Columns.empty())
	{
		auto& FindOpt = Global->Opt->FindOpt;
		FindOpt.OutColumns = DeserialiseViewSettings(Columns, Widths);
		auto Result = SerialiseViewSettings(FindOpt.OutColumns);
		FindOpt.strSearchOutFormat = Result.first;
		FindOpt.strSearchOutFormatWidth = Result.second;
	}
}

void Options::Load(const std::vector<std::pair<string, string>>& Overridden)
{
	// KnownModulesIDs::GuidOption::Default pointer is used in the static config structure, so it MUST be initialized before calling InitConfig()
	static std::pair<GUID, string> DefaultKnownGuids[] =
	{
		{ NetworkGuid, {} },
		{ EMenuGuid, {} },
		{ ArcliteGuid, {} },
		{ LuamacroGuid, {} },
		{ NetBoxGuid, {} },
	};

	static_assert(std::size(DefaultKnownGuids) == sizeof(Options::KnownModulesIDs) / sizeof(Options::KnownModulesIDs::GuidOption));

	KnownModulesIDs::GuidOption* GuidOptions[] =
	{
		&KnownIDs.Network,
		&KnownIDs.Emenu,
		&KnownIDs.Arclite,
		&KnownIDs.Luamacro,
		&KnownIDs.Netbox,
	};
	static_assert(std::size(GuidOptions) == std::size(DefaultKnownGuids));

	for(const auto& i: zip(DefaultKnownGuids, GuidOptions))
	{
		auto& a = std::get<0>(i);
		auto& b = std::get<1>(i);

		a.second = GuidToStr(a.first);
		b->Default = a.second.data();
		b->Id = a.first;
		b->StrId = a.second;
	};

	InitConfig();

	/* <ПРЕПРОЦЕССЫ> *************************************************** */

	/* BUGBUG??
	SetRegRootKey(HKEY_LOCAL_MACHINE);
	DWORD OptPolicies_ShowHiddenDrives=GetRegKey(NKeyPolicies,L"ShowHiddenDrives",1)&1;
	SetRegRootKey(HKEY_CURRENT_USER);
	*/
	/* *************************************************** </ПРЕПРОЦЕССЫ> */

	for (auto& i: m_Config)
	{
		const auto Cfg = i.GetConfig();
		for(const auto& j: i)
		{
			j.Value->ReceiveValue(Cfg, j.KeyName, j.ValName, j.Default);

			for (const auto& ovr: Overridden)
			{
				const auto DotPos = ovr.first.rfind(L'.');
				if (DotPos != string::npos &&
					starts_with_icase(ovr.first, { j.KeyName, DotPos }) &&
					equal_icase(make_string_view(ovr.first, DotPos + 1), j.ValName))
				{
					// TODO: log if failed
					j.Value->TryParse(ovr.second);
				}
			}
		}
	}

	/* <ПОСТПРОЦЕССЫ> *************************************************** */

	Palette.Load();
	GlobalUserMenuDir = ConvertNameToFull(os::env::expand_strings(GetFarIniString(L"General", L"GlobalUserMenuDir", Global->g_strFarPath)));
	AddEndSlash(GlobalUserMenuDir);

	if(WindowMode == -1)
	{
		WindowMode = StoredWindowMode;
	}

	ElevationMode = StoredElevationMode;

	ReadPanelModes();

	/* BUGBUG??
	// уточняем системную политику
	// для дисков юзер может только отменять показ
	Policies.ShowHiddenDrives&=OptPolicies_ShowHiddenDrives;
	*/

	xlat_initialize();

	FindOpt.OutColumns.clear();

	SetSearchColumns(FindOpt.strSearchOutFormat, FindOpt.strSearchOutFormatWidth);

	string tmp[2];
	if (!ConfigProvider().GeneralCfg()->EnumValues(L"Masks", 0, tmp[0], tmp[1]))
	{
		ApplyDefaultMaskGroups();
	}

	for(auto& i: GuidOptions)
	{
		if (i->StrId.empty())
		{
			i->Id = GUID_NULL;
		}
		else
		{
			StrToGuid(i->StrId, i->Id);
		}
	}

/* *************************************************** </ПОСТПРОЦЕССЫ> */

	// we assume that any changes after this point will be made by the user
	std::for_each(RANGE(m_Config, i)
	{
		std::for_each(RANGE(i, j)
		{
			j.Value->MakeUnchanged();
		});
	});
}

void Options::Save(bool Manual)
{
	InitConfig();

	if (Manual && Message(0,
		msg(lng::MSaveSetupTitle),
		{
			msg(lng::MSaveSetupAsk1),
			msg(lng::MSaveSetupAsk2)
		},
		{ lng::MSaveSetup, lng::MCancel }) != Message::first_button)
		return;

	/* <ПРЕПРОЦЕССЫ> *************************************************** */

	const auto& StorePanelOptions = [](panel_ptr PanelPtr, PanelOptions& Panel)
	{
		if (PanelPtr->GetMode() == panel_mode::NORMAL_PANEL)
		{
			Panel.m_Type = static_cast<int>(PanelPtr->GetType());
			Panel.ViewMode = PanelPtr->GetViewMode();
			Panel.SortMode = static_cast<int>(PanelPtr->GetSortMode());
			Panel.ReverseSortOrder = PanelPtr->GetSortOrder();
			Panel.SortGroups = PanelPtr->GetSortGroups();
			Panel.ShowShortNames = PanelPtr->GetShowShortNamesMode();
			Panel.NumericSort = PanelPtr->GetNumericSort();
			Panel.CaseSensitiveSort = PanelPtr->GetCaseSensitiveSort();
			Panel.SelectedFirst = PanelPtr->GetSelectedFirstMode();
			Panel.DirectoriesFirst = PanelPtr->GetDirectoriesFirst();
		}
		Panel.Visible = PanelPtr->IsVisible();
		Panel.Folder = PanelPtr->GetCurDir();
		string strTemp1, strTemp2;
		PanelPtr->GetCurBaseName(strTemp1, strTemp2);
		Panel.CurFile = strTemp1;
	};

	StorePanelOptions(Global->CtrlObject->Cp()->LeftPanel(), LeftPanel);
	StorePanelOptions(Global->CtrlObject->Cp()->RightPanel(), RightPanel);

	LeftFocus = Global->CtrlObject->Cp()->ActivePanel() == Global->CtrlObject->Cp()->LeftPanel();

	/* *************************************************** </ПРЕПРОЦЕССЫ> */

	Global->CtrlObject->HiFiles->Save(Manual);

	Palette.Save(Manual);

	for (const auto& Config: m_Config)
	{
		const auto ConfigStorage = Config.GetConfig();
		SCOPED_ACTION(auto)(ConfigStorage->ScopedTransaction());
		for (const auto& Item: Config)
		{
			Item.Value->StoreValue(ConfigStorage, Item.KeyName, Item.ValName, Manual);
		}
	}

	FileFilter::Save(Manual);
	SavePanelModes(Manual);
	Global->CtrlObject->Macro.SaveMacros(Manual);
}

intptr_t Options::AdvancedConfigDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	auto& CurrentConfig = GetConfig(m_CurrentConfigType);

	switch (Msg)
	{
	case DN_RESIZECONSOLE:
		{
			COORD Size{ static_cast<SHORT>(std::max(ScrX - 4, 60)), static_cast<SHORT>(std::max(ScrY - 2, 20)) };
			Dlg->SendMessage(DM_RESIZEDIALOG, 0, &Size);
			SMALL_RECT ListPos{ 3, 1, static_cast<SHORT>(Size.X - 4), static_cast<SHORT>(Size.Y - 2) };
			Dlg->SendMessage(DM_SETITEMPOSITION, 0, &ListPos);
		}
		break;

	case DN_CONTROLINPUT:
		{
			const auto record = reinterpret_cast<const INPUT_RECORD*>(Param2);
			if (record->EventType==KEY_EVENT)
			{
				int key = InputRecordToKey(record);
				switch(key)
				{
				case KEY_SHIFTF1:
					{
						FarListInfo ListInfo = {sizeof(ListInfo)};
						Dlg->SendMessage(DM_LISTINFO, Param1, &ListInfo);

						auto HelpTopic = concat(CurrentConfig[ListInfo.SelectPos].KeyName, L'.', CurrentConfig[ListInfo.SelectPos].ValName);
						if (Help::create(HelpTopic, nullptr, FHELP_NOSHOWERROR)->GetError())
						{
							HelpTopic = concat(CurrentConfig[ListInfo.SelectPos].KeyName, L"Settings"_sv);
							Help::create(HelpTopic, nullptr, FHELP_NOSHOWERROR);
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
						Dlg->SendMessage(DM_ENABLEREDRAW, 0, nullptr);
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
						Dlg->SendMessage(DM_ENABLEREDRAW, 1, nullptr);
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

			if (CurrentConfig[ListInfo.SelectPos].Edit(Param1 != 0))
			{
				Dlg->SendMessage(DM_ENABLEREDRAW, 0, nullptr);
				FarListUpdate flu = {sizeof(flu), ListInfo.SelectPos};
				flu.Item = CurrentConfig[ListInfo.SelectPos].MakeListItem((*m_ConfigStrings)[ListInfo.SelectPos]);
				Dlg->SendMessage(DM_LISTUPDATE, 0, &flu);
				FarListPos flp = {sizeof(flp), ListInfo.SelectPos, ListInfo.TopPos};
				Dlg->SendMessage(DM_LISTSETCURPOS, 0, &flp);
				Dlg->SendMessage(DM_ENABLEREDRAW, 1, nullptr);
			}
			return FALSE;
		}
		break;
	default:
		break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

bool Options::AdvancedConfig(config_type Mode)
{
	m_CurrentConfigType = Mode;
	auto& CurrentConfig = GetConfig(m_CurrentConfigType);
		
	int DlgWidth = std::max(ScrX-4, 60), DlgHeight = std::max(ScrY-2, 20);
	FarDialogItem AdvancedConfigDlgData[]=
	{
		{DI_LISTBOX,3,1,DlgWidth-4,DlgHeight-2,0,nullptr,nullptr,DIF_NONE,nullptr},
	};
	auto AdvancedConfigDlg = MakeDialogItemsEx(AdvancedConfigDlgData);

	std::vector<FarListItem> items;
	items.reserve(CurrentConfig.size());
	std::vector<string> Strings(CurrentConfig.size());
	m_ConfigStrings = &Strings;
	SCOPE_EXIT{ m_ConfigStrings = nullptr; };
	const auto ConfigData = zip(CurrentConfig, Strings);
	std::transform(ALL_CONST_RANGE(ConfigData), std::back_inserter(items), [](const auto& i) { return std::get<0>(i).MakeListItem(std::get<1>(i)); });

	FarList Items={sizeof(FarList), items.size(), items.data()};

	AdvancedConfigDlg[0].ListItems = &Items;

	const auto Dlg = Dialog::create(AdvancedConfigDlg, &Options::AdvancedConfigDlgProc, this);
	Dlg->SetHelp(L"FarConfig");
	Dlg->SetPosition(-1, -1, DlgWidth, DlgHeight);
	Dlg->SetId(AdvancedConfigId);
	Dlg->Process();
	return true;
}

void Options::SetViewSettings(size_t Index, PanelViewSettings&& Data)
{
	assert(Index < m_ViewSettings.size());

	m_ViewSettings[Index] = std::move(Data);
	m_ViewSettingsChanged = true;
}

void Options::AddViewSettings(size_t Index, PanelViewSettings&& Data)
{
	m_ViewSettings.emplace(m_ViewSettings.begin() + std::max(Index, predefined_panel_modes_count), std::move(Data));
	m_ViewSettingsChanged = true;
}

void Options::DeleteViewSettings(size_t Index)
{
	assert(Index >= predefined_panel_modes_count);

	m_ViewSettings.erase(m_ViewSettings.begin() + Index);
	m_ViewSettingsChanged = true;
}

static const wchar_t CustomModesKeyName[] = L"CustomModes";
static const wchar_t ModesNameName[] = L"Name";
static const wchar_t ModesColumnTitlesName[] = L"ColumnTitles";
static const wchar_t ModesColumnWidthsName[] = L"ColumnWidths";
static const wchar_t ModesStatusColumnTitlesName[] = L"StatusColumnTitles";
static const wchar_t ModesStatusColumnWidthsName[] = L"StatusColumnWidths";
static const wchar_t ModesFlagsName[] = L"Flags";

void Options::ReadPanelModes()
{
	const auto cfg = ConfigProvider().CreatePanelModeConfig();

	auto root = HierarchicalConfig::root_key();

	const auto& ReadMode = [&](auto& i, size_t Index)
	{
		const auto Key = cfg->FindByName(root, str(Index));

		if (!Key)
		{
			return false;
		}
		string strColumnTitles, strColumnWidths;
		cfg->GetValue(Key, ModesColumnTitlesName, strColumnTitles);
		cfg->GetValue(Key, ModesColumnWidthsName, strColumnWidths);

		string strStatusColumnTitles, strStatusColumnWidths;
		cfg->GetValue(Key, ModesStatusColumnTitlesName, strStatusColumnTitles);
		cfg->GetValue(Key, ModesStatusColumnWidthsName, strStatusColumnWidths);

		unsigned long long Flags=0;
		cfg->GetValue(Key, ModesFlagsName, Flags);

		cfg->GetValue(Key, ModesNameName, i.Name);

		if (!strColumnTitles.empty())
			i.PanelColumns = DeserialiseViewSettings(strColumnTitles, strColumnWidths);

		if (!strStatusColumnTitles.empty())
			i.StatusColumns = DeserialiseViewSettings(strStatusColumnTitles, strStatusColumnWidths);

		i.Flags = Flags;

		return true;
	};

	for_each_cnt(m_ViewSettings.begin(), m_ViewSettings.begin() + predefined_panel_modes_count, ReadMode);

	root = cfg->FindByName(cfg->root_key(), CustomModesKeyName);

	if (root)
	{
		for (size_t i = 0; ; ++i)
		{
			PanelViewSettings NewSettings;
			if (ReadMode(NewSettings, i))
				m_ViewSettings.emplace_back(std::move(NewSettings));
			else
				break;
		}
	}

	m_ViewSettingsChanged = false;
}


void Options::SavePanelModes(bool always)
{
	if (!always && !m_ViewSettingsChanged)
		return;

	const auto cfg = ConfigProvider().CreatePanelModeConfig();
	auto root = cfg->root_key();

	const auto& SaveMode = [&](const auto& i, size_t Index)
	{
		const auto PanelResult = SerialiseViewSettings(i.PanelColumns);
		const auto StatusResult = SerialiseViewSettings(i.StatusColumns);

		if(const auto Key = cfg->CreateKey(root, str(Index)))
		{
			cfg->SetValue(Key, ModesNameName, i.Name);
			cfg->SetValue(Key, ModesColumnTitlesName, PanelResult.first);
			cfg->SetValue(Key, ModesColumnWidthsName, PanelResult.second);
			cfg->SetValue(Key, ModesStatusColumnTitlesName, StatusResult.first);
			cfg->SetValue(Key, ModesStatusColumnWidthsName, StatusResult.second);
			cfg->SetValue(Key, ModesFlagsName, i.Flags);
		}
	};

	for_each_cnt(ViewSettings.cbegin(), ViewSettings.cbegin() + predefined_panel_modes_count, SaveMode);

	if ((root = cfg->FindByName(cfg->root_key(), CustomModesKeyName)))
	{
		cfg->DeleteKeyTree(root);
	}

	if ((root = cfg->CreateKey(cfg->root_key(), CustomModesKeyName)))
	{
		for_each_cnt(ViewSettings.cbegin() + predefined_panel_modes_count, ViewSettings.cend(), SaveMode);
	}
	m_ViewSettingsChanged = false;
}

enum enumMenus
{
	MENU_LEFT,
	MENU_FILES,
	MENU_COMMANDS,
	MENU_OPTIONS,
	MENU_RIGHT
};

enum enumLeftMenu
{
	MENU_LEFT_BRIEFVIEW,
	MENU_LEFT_MEDIUMVIEW,
	MENU_LEFT_FULLVIEW,
	MENU_LEFT_WIDEVIEW,
	MENU_LEFT_DETAILEDVIEW,
	MENU_LEFT_DIZVIEW,
	MENU_LEFT_LONGVIEW,
	MENU_LEFT_OWNERSVIEW,
	MENU_LEFT_LINKSVIEW,
	MENU_LEFT_ALTERNATIVEVIEW,
	MENU_LEFT_SEPARATOR1,
	MENU_LEFT_INFOPANEL,
	MENU_LEFT_TREEPANEL,
	MENU_LEFT_QUICKVIEW,
	MENU_LEFT_SEPARATOR2,
	MENU_LEFT_SORTMODES,
	MENU_LEFT_LONGNAMES,
	MENU_LEFT_TOGGLEPANEL,
	MENU_LEFT_REREAD,
	MENU_LEFT_CHANGEDRIVE
};

//currently left == right

enum enumFilesMenu
{
	MENU_FILES_VIEW,
	MENU_FILES_EDIT,
	MENU_FILES_COPY,
	MENU_FILES_MOVE,
	MENU_FILES_LINK,
	MENU_FILES_CREATEFOLDER,
	MENU_FILES_DELETE,
	MENU_FILES_WIPE,
	MENU_FILES_SEPARATOR1,
	MENU_FILES_ADD,
	MENU_FILES_EXTRACT,
	MENU_FILES_ARCHIVECOMMANDS,
	MENU_FILES_SEPARATOR2,
	MENU_FILES_ATTRIBUTES,
	MENU_FILES_APPLYCOMMAND,
	MENU_FILES_DESCRIBE,
	MENU_FILES_SEPARATOR3,
	MENU_FILES_SELECTGROUP,
	MENU_FILES_UNSELECTGROUP,
	MENU_FILES_INVERTSELECTION,
	MENU_FILES_RESTORESELECTION
};

enum enumCommandsMenu
{
	MENU_COMMANDS_FINDFILE,
	MENU_COMMANDS_HISTORY,
	MENU_COMMANDS_VIDEOMODE,
	MENU_COMMANDS_FINDFOLDER,
	MENU_COMMANDS_VIEWHISTORY,
	MENU_COMMANDS_FOLDERHISTORY,
	MENU_COMMANDS_SEPARATOR1,
	MENU_COMMANDS_SWAPPANELS,
	MENU_COMMANDS_TOGGLEPANELS,
	MENU_COMMANDS_COMPAREFOLDERS,
	MENU_COMMANDS_SEPARATOR2,
	MENU_COMMANDS_EDITUSERMENU,
	MENU_COMMANDS_FILEASSOCIATIONS,
	MENU_COMMANDS_FOLDERSHORTCUTS,
	MENU_COMMANDS_FILTER,
	MENU_COMMANDS_SEPARATOR3,
	MENU_COMMANDS_PLUGINCOMMANDS,
	MENU_COMMANDS_WINDOWSLIST,
	MENU_COMMANDS_PROCESSLIST,
	MENU_COMMANDS_HOTPLUGLIST
};

enum enumOptionsMenu
{
	MENU_OPTIONS_SYSTEMSETTINGS,
	MENU_OPTIONS_PANELSETTINGS,
	MENU_OPTIONS_TREESETTINGS,
	MENU_OPTIONS_INTERFACESETTINGS,
	MENU_OPTIONS_LANGUAGES,
	MENU_OPTIONS_PLUGINSCONFIG,
	MENU_OPTIONS_PLUGINSMANAGERSETTINGS,
	MENU_OPTIONS_DIALOGSETTINGS,
	MENU_OPTIONS_VMENUSETTINGS,
	MENU_OPTIONS_CMDLINESETTINGS,
	MENU_OPTIONS_AUTOCOMPLETESETTINGS,
	MENU_OPTIONS_INFOPANELSETTINGS,
	MENU_OPTIONS_MASKGROUPS,
	MENU_OPTIONS_SEPARATOR1,
	MENU_OPTIONS_CONFIRMATIONS,
	MENU_OPTIONS_FILEPANELMODES,
	MENU_OPTIONS_FILEDESCRIPTIONS,
	MENU_OPTIONS_FOLDERINFOFILES,
	MENU_OPTIONS_SEPARATOR2,
	MENU_OPTIONS_VIEWERSETTINGS,
	MENU_OPTIONS_EDITORSETTINGS,
	MENU_OPTIONS_CODEPAGESSETTINGS,
	MENU_OPTIONS_SEPARATOR3,
	MENU_OPTIONS_COLORS,
	MENU_OPTIONS_FILESHIGHLIGHTING,
	MENU_OPTIONS_SEPARATOR4,
	MENU_OPTIONS_SAVESETUP
};

void SetLeftRightMenuChecks(MenuDataEx *pMenu, bool bLeft)
{
	const auto pPanel = bLeft? Global->CtrlObject->Cp()->LeftPanel() : Global->CtrlObject->Cp()->RightPanel();

	switch (pPanel->GetType())
	{
	case panel_type::FILE_PANEL:
		{
			int MenuLine = pPanel->GetViewMode();

			if (MenuLine <= MENU_LEFT_ALTERNATIVEVIEW)
			{
				if (!MenuLine)
					pMenu[MENU_LEFT_ALTERNATIVEVIEW].SetCheck(1);
				else
					pMenu[MenuLine-1].SetCheck(1);
			}
		}
		break;
	case panel_type::INFO_PANEL:
		pMenu[MENU_LEFT_INFOPANEL].SetCheck(1);
		break;
	case panel_type::TREE_PANEL:
		pMenu[MENU_LEFT_TREEPANEL].SetCheck(1);
		break;
	case panel_type::QVIEW_PANEL:
		pMenu[MENU_LEFT_QUICKVIEW].SetCheck(1);
		break;
	}

	pMenu[MENU_LEFT_LONGNAMES].SetCheck(!pPanel->GetShowShortNamesMode());
}

void Options::ShellOptions(bool LastCommand, const MOUSE_EVENT_RECORD *MouseEvent)
{
	const auto& ApplyViewModesNames = [this](MenuDataEx* Menu)
	{
		for (size_t i = 0; i < predefined_panel_modes_count; ++i)
		{
			if (!ViewSettings[i].Name.empty())
				Menu[RealModeToDisplay(i)].Name = ViewSettings[i].Name.data();
		}
	};

	LISTITEMFLAGS no_tree = Global->Opt->Tree.TurnOffCompletely ? LIF_HIDDEN : 0;

	MenuDataEx LeftMenu[]=
	{
		msg(lng::MMenuBriefView).data(), LIF_SELECTED, KEY_CTRL1,
		msg(lng::MMenuMediumView).data(), 0, KEY_CTRL2,
		msg(lng::MMenuFullView).data(), 0, KEY_CTRL3,
		msg(lng::MMenuWideView).data(), 0, KEY_CTRL4,
		msg(lng::MMenuDetailedView).data(), 0, KEY_CTRL5,
		msg(lng::MMenuDizView).data(), 0, KEY_CTRL6,
		msg(lng::MMenuLongDizView).data(), 0, KEY_CTRL7,
		msg(lng::MMenuOwnersView).data(), 0, KEY_CTRL8,
		msg(lng::MMenuLinksView).data(), 0, KEY_CTRL9,
		msg(lng::MMenuAlternativeView).data(), 0, KEY_CTRL0,
		L"", LIF_SEPARATOR, 0,
		msg(lng::MMenuInfoPanel).data(), 0, KEY_CTRLL,
		msg(lng::MMenuTreePanel).data(), no_tree, KEY_CTRLT,
		msg(lng::MMenuQuickView).data(), 0, KEY_CTRLQ,
		L"", LIF_SEPARATOR, 0,
		msg(lng::MMenuSortModes).data(), 0, KEY_CTRLF12,
		msg(lng::MMenuLongNames).data(), 0, KEY_CTRLN,
		msg(lng::MMenuTogglePanel).data(), 0, KEY_CTRLF1,
		msg(lng::MMenuReread).data(), 0, KEY_CTRLR,
		msg(lng::MMenuChangeDrive).data(), 0, KEY_ALTF1,
	};
	ApplyViewModesNames(LeftMenu);
	const auto LeftMenuStrings = VMenu::AddHotkeys(make_range(LeftMenu));

	MenuDataEx FilesMenu[]=
	{
		msg(lng::MMenuView).data(), LIF_SELECTED, KEY_F3,
		msg(lng::MMenuEdit).data(), 0, KEY_F4,
		msg(lng::MMenuCopy).data(), 0, KEY_F5,
		msg(lng::MMenuMove).data(), 0, KEY_F6,
		msg(lng::MMenuLink).data(), 0, KEY_ALTF6,
		msg(lng::MMenuCreateFolder).data(), 0, KEY_F7,
		msg(lng::MMenuDelete).data(), 0, KEY_F8,
		msg(lng::MMenuWipe).data(), 0, KEY_ALTDEL,
		L"", LIF_SEPARATOR, 0,
		msg(lng::MMenuAdd).data(), 0, KEY_SHIFTF1,
		msg(lng::MMenuExtract).data(), 0, KEY_SHIFTF2,
		msg(lng::MMenuArchiveCommands).data(), 0, KEY_SHIFTF3,
		L"", LIF_SEPARATOR, 0,
		msg(lng::MMenuAttributes).data(), 0, KEY_CTRLA,
		msg(lng::MMenuApplyCommand).data(), 0, KEY_CTRLG,
		msg(lng::MMenuDescribe).data(), 0, KEY_CTRLZ,
		L"", LIF_SEPARATOR, 0,
		msg(lng::MMenuSelectGroup).data(), 0, KEY_ADD,
		msg(lng::MMenuUnselectGroup).data(), 0, KEY_SUBTRACT,
		msg(lng::MMenuInvertSelection).data(), 0, KEY_MULTIPLY,
		msg(lng::MMenuRestoreSelection).data(), 0, KEY_CTRLM,
	};
	const auto FilesMenuStrings = VMenu::AddHotkeys(make_range(FilesMenu));

	MenuDataEx CmdMenu[]=
	{
		msg(lng::MMenuFindFile).data(), LIF_SELECTED, KEY_ALTF7,
		msg(lng::MMenuHistory).data(), 0, KEY_ALTF8,
		msg(lng::MMenuVideoMode).data(), 0, KEY_ALTF9,
		msg(lng::MMenuFindFolder).data(), no_tree, KEY_ALTF10,
		msg(lng::MMenuViewHistory).data(), 0, KEY_ALTF11,
		msg(lng::MMenuFoldersHistory).data(), 0, KEY_ALTF12,
		L"", LIF_SEPARATOR, 0,
		msg(lng::MMenuSwapPanels).data(), 0, KEY_CTRLU,
		msg(lng::MMenuTogglePanels).data(), 0, KEY_CTRLO,
		msg(lng::MMenuCompareFolders).data(), 0, 0,
		L"", LIF_SEPARATOR, 0,
		msg(lng::MMenuUserMenu).data(), 0, 0,
		msg(lng::MMenuFileAssociations).data(), 0, 0,
		msg(lng::MMenuFolderShortcuts).data(), 0, 0,
		msg(lng::MMenuFilter).data(), 0, KEY_CTRLI,
		L"", LIF_SEPARATOR, 0,
		msg(lng::MMenuPluginCommands).data(), 0, KEY_F11,
		msg(lng::MMenuWindowsList).data(), 0, KEY_F12,
		msg(lng::MMenuProcessList).data(), 0, KEY_CTRLW,
		msg(lng::MMenuHotPlugList).data(), 0, 0,
	};
	const auto CmdMenuStrings = VMenu::AddHotkeys(make_range(CmdMenu));

	MenuDataEx OptionsMenu[]=
	{
		msg(lng::MMenuSystemSettings).data(), LIF_SELECTED, 0,
		msg(lng::MMenuPanelSettings).data(), 0, 0,
		msg(lng::MMenuTreeSettings).data(), no_tree, 0,
		msg(lng::MMenuInterface).data(), 0, 0,
		msg(lng::MMenuLanguages).data(), 0, 0,
		msg(lng::MMenuPluginsConfig).data(), 0, 0,
		msg(lng::MMenuPluginsManagerSettings).data(), 0, 0,
		msg(lng::MMenuDialogSettings).data(), 0, 0,
		msg(lng::MMenuVMenuSettings).data(), 0, 0,
		msg(lng::MMenuCmdlineSettings).data(), 0, 0,
		msg(lng::MMenuAutoCompleteSettings).data(), 0, 0,
		msg(lng::MMenuInfoPanelSettings).data(), 0, 0,
		msg(lng::MMenuMaskGroups).data(), 0, 0,
		L"", LIF_SEPARATOR, 0,
		msg(lng::MMenuConfirmation).data(), 0, 0,
		msg(lng::MMenuFilePanelModes).data(), 0, 0,
		msg(lng::MMenuFileDescriptions).data(), 0, 0,
		msg(lng::MMenuFolderInfoFiles).data(), 0, 0,
		L"", LIF_SEPARATOR, 0,
		msg(lng::MMenuViewer).data(), 0, 0,
		msg(lng::MMenuEditor).data(), 0, 0,
		msg(lng::MMenuCodePages).data(), 0, 0,
		L"", LIF_SEPARATOR, 0,
		msg(lng::MMenuColors).data(), 0, 0,
		msg(lng::MMenuFilesHighlighting).data(), 0, 0,
		L"", LIF_SEPARATOR, 0,
		msg(lng::MMenuSaveSetup).data(), 0, KEY_SHIFTF9,
	};
	const auto OptionsMenuStrings = VMenu::AddHotkeys(make_range(OptionsMenu));

	MenuDataEx RightMenu[]=
	{
		msg(lng::MMenuBriefView).data(), LIF_SELECTED, KEY_CTRL1,
		msg(lng::MMenuMediumView).data(), 0, KEY_CTRL2,
		msg(lng::MMenuFullView).data(), 0, KEY_CTRL3,
		msg(lng::MMenuWideView).data(), 0, KEY_CTRL4,
		msg(lng::MMenuDetailedView).data(), 0, KEY_CTRL5,
		msg(lng::MMenuDizView).data(), 0, KEY_CTRL6,
		msg(lng::MMenuLongDizView).data(), 0, KEY_CTRL7,
		msg(lng::MMenuOwnersView).data(), 0, KEY_CTRL8,
		msg(lng::MMenuLinksView).data(), 0, KEY_CTRL9,
		msg(lng::MMenuAlternativeView).data(), 0, KEY_CTRL0,
		L"", LIF_SEPARATOR, 0,
		msg(lng::MMenuInfoPanel).data(), 0, KEY_CTRLL,
		msg(lng::MMenuTreePanel).data(), no_tree, KEY_CTRLT,
		msg(lng::MMenuQuickView).data(), 0, KEY_CTRLQ,
		L"", LIF_SEPARATOR, 0,
		msg(lng::MMenuSortModes).data(), 0, KEY_CTRLF12,
		msg(lng::MMenuLongNames).data(), 0, KEY_CTRLN,
		msg(lng::MMenuTogglePanelRight).data(), 0, KEY_CTRLF2,
		msg(lng::MMenuReread).data(), 0, KEY_CTRLR,
		msg(lng::MMenuChangeDriveRight).data(), 0, KEY_ALTF2,
	};
	ApplyViewModesNames(RightMenu);
	const auto RightMenuStrings = VMenu::AddHotkeys(make_range(RightMenu));


	HMenuData MainMenu[]=
	{
		{msg(lng::MMenuLeftTitle).data(), L"LeftRightMenu", LeftMenu, std::size(LeftMenu), 1},
		{msg(lng::MMenuFilesTitle).data(), L"FilesMenu", FilesMenu, std::size(FilesMenu), 0},
		{msg(lng::MMenuCommandsTitle).data(), L"CmdMenu", CmdMenu, std::size(CmdMenu), 0},
		{msg(lng::MMenuOptionsTitle).data(), L"OptMenu", OptionsMenu, std::size(OptionsMenu), 0},
		{msg(lng::MMenuRightTitle).data(), L"LeftRightMenu", RightMenu, std::size(RightMenu), 0},
	};
	static int LastHItem=-1,LastVItem=0;
	int HItem,VItem;

	SetLeftRightMenuChecks(LeftMenu, true);
	SetLeftRightMenuChecks(RightMenu, false);
	// Навигация по меню
	{
		const auto HOptMenu = HMenu::create(MainMenu, std::size(MainMenu));
		HOptMenu->SetHelp(L"Menus");
		HOptMenu->SetPosition(0,0,ScrX,0);
		Global->WindowManager->ExecuteWindow(HOptMenu);

		if (LastCommand)
		{
			MenuDataEx *VMenuTable[] = {LeftMenu, FilesMenu, CmdMenu, OptionsMenu, RightMenu};
			int HItemToShow = LastHItem;

			if (HItemToShow == -1)
			{
				if (Global->CtrlObject->Cp()->ActivePanel() == Global->CtrlObject->Cp()->RightPanel() &&
					Global->CtrlObject->Cp()->ActivePanel()->IsVisible())
					HItemToShow = 4;
				else
					HItemToShow = 0;
			}

			MainMenu[0].Selected = 0;
			MainMenu[HItemToShow].Selected = 1;
			VMenuTable[HItemToShow][0].SetSelect(0);
			VMenuTable[HItemToShow][LastVItem].SetSelect(1);
			Global->WindowManager->CallbackWindow([&HOptMenu](){HOptMenu->ProcessKey(Manager::Key(KEY_DOWN));});
		}
		else
		{
			if (Global->CtrlObject->Cp()->ActivePanel() == Global->CtrlObject->Cp()->RightPanel() &&
				Global->CtrlObject->Cp()->ActivePanel()->IsVisible())
			{
				MainMenu[0].Selected = 0;
				MainMenu[4].Selected = 1;
			}
		}

		if (MouseEvent)
		{
			Global->WindowManager->CallbackWindow([&HOptMenu,MouseEvent](){HOptMenu->ProcessMouse(MouseEvent);});
		}

		Global->WindowManager->ExecuteModal(HOptMenu);
		HOptMenu->GetExitCode(HItem,VItem);
	}

	// "Исполнятор команд меню"
	switch (HItem)
	{
	case MENU_LEFT:
	case MENU_RIGHT:
		{
			auto pPanel = (HItem == MENU_LEFT)? Global->CtrlObject->Cp()->LeftPanel() : Global->CtrlObject->Cp()->RightPanel();

			if (VItem >= MENU_LEFT_BRIEFVIEW && VItem <= MENU_LEFT_ALTERNATIVEVIEW)
			{
				Global->CtrlObject->Cp()->ChangePanelToFilled(pPanel, panel_type::FILE_PANEL);
				pPanel=(HItem == MENU_LEFT)?Global->CtrlObject->Cp()->LeftPanel():Global->CtrlObject->Cp()->RightPanel();
				pPanel->SetViewMode((VItem == MENU_LEFT_ALTERNATIVEVIEW)?VIEW_0:VIEW_1+VItem);
			}
			else
			{
				switch (VItem)
				{
				case MENU_LEFT_INFOPANEL: // Info panel
					Global->CtrlObject->Cp()->ChangePanelToFilled(pPanel, panel_type::INFO_PANEL);
					break;
				case MENU_LEFT_TREEPANEL: // Tree panel
					Global->CtrlObject->Cp()->ChangePanelToFilled(pPanel, panel_type::TREE_PANEL);
					break;
				case MENU_LEFT_QUICKVIEW: // Quick view
					Global->CtrlObject->Cp()->ChangePanelToFilled(pPanel, panel_type::QVIEW_PANEL);
					break;
				case MENU_LEFT_SORTMODES: // Sort modes
					pPanel->ProcessKey(Manager::Key(KEY_CTRLF12));
					break;
				case MENU_LEFT_LONGNAMES: // Show long names
					pPanel->ProcessKey(Manager::Key(KEY_CTRLN));
					break;
				case MENU_LEFT_TOGGLEPANEL: // Panel On/Off
					Global->WindowManager->ProcessKey(Manager::Key((HItem==MENU_LEFT)?KEY_CTRLF1:KEY_CTRLF2));
					break;
				case MENU_LEFT_REREAD: // Re-read
					pPanel->ProcessKey(Manager::Key(KEY_CTRLR));
					break;
				case MENU_LEFT_CHANGEDRIVE: // Change drive
					ChangeDisk(pPanel);
					break;
				}
			}

			break;
		}
	case MENU_FILES:
		{
			switch (VItem)
			{
			case MENU_FILES_VIEW:  // View
				Global->WindowManager->ProcessKey(Manager::Key(KEY_F3));
				break;
			case MENU_FILES_EDIT:  // Edit
				Global->WindowManager->ProcessKey(Manager::Key(KEY_F4));
				break;
			case MENU_FILES_COPY:  // Copy
				Global->WindowManager->ProcessKey(Manager::Key(KEY_F5));
				break;
			case MENU_FILES_MOVE:  // Rename or move
				Global->WindowManager->ProcessKey(Manager::Key(KEY_F6));
				break;
			case MENU_FILES_LINK:  // Create link
				Global->WindowManager->ProcessKey(Manager::Key(KEY_ALTF6));
				break;
			case MENU_FILES_CREATEFOLDER:  // Make folder
				Global->WindowManager->ProcessKey(Manager::Key(KEY_F7));
				break;
			case MENU_FILES_DELETE:  // Delete
				Global->WindowManager->ProcessKey(Manager::Key(KEY_F8));
				break;
			case MENU_FILES_WIPE:  // Wipe
				Global->WindowManager->ProcessKey(Manager::Key(KEY_ALTDEL));
				break;
			case MENU_FILES_ADD:  // Add to archive
				Global->CtrlObject->Cp()->ActivePanel()->ProcessKey(Manager::Key(KEY_SHIFTF1));
				break;
			case MENU_FILES_EXTRACT:  // Extract files
				Global->CtrlObject->Cp()->ActivePanel()->ProcessKey(Manager::Key(KEY_SHIFTF2));
				break;
			case MENU_FILES_ARCHIVECOMMANDS:  // Archive commands
				Global->CtrlObject->Cp()->ActivePanel()->ProcessKey(Manager::Key(KEY_SHIFTF3));
				break;
			case MENU_FILES_ATTRIBUTES: // File attributes
				Global->CtrlObject->Cp()->ActivePanel()->ProcessKey(Manager::Key(KEY_CTRLA));
				break;
			case MENU_FILES_APPLYCOMMAND: // Apply command
				Global->CtrlObject->Cp()->ActivePanel()->ProcessKey(Manager::Key(KEY_CTRLG));
				break;
			case MENU_FILES_DESCRIBE: // Describe files
				Global->CtrlObject->Cp()->ActivePanel()->ProcessKey(Manager::Key(KEY_CTRLZ));
				break;
			case MENU_FILES_SELECTGROUP: // Select group
				Global->CtrlObject->Cp()->ActivePanel()->ProcessKey(Manager::Key(KEY_ADD));
				break;
			case MENU_FILES_UNSELECTGROUP: // Unselect group
				Global->CtrlObject->Cp()->ActivePanel()->ProcessKey(Manager::Key(KEY_SUBTRACT));
				break;
			case MENU_FILES_INVERTSELECTION: // Invert selection
				Global->CtrlObject->Cp()->ActivePanel()->ProcessKey(Manager::Key(KEY_MULTIPLY));
				break;
			case MENU_FILES_RESTORESELECTION: // Restore selection
				Global->CtrlObject->Cp()->ActivePanel()->RestoreSelection();
				break;
			}

			break;
		}
	case MENU_COMMANDS:
		{
			switch (VItem)
			{
			case MENU_COMMANDS_FINDFILE: // Find file
				Global->WindowManager->ProcessKey(Manager::Key(KEY_ALTF7));
				break;
			case MENU_COMMANDS_HISTORY: // History
				Global->WindowManager->ProcessKey(Manager::Key(KEY_ALTF8));
				break;
			case MENU_COMMANDS_VIDEOMODE: // Video mode
				Global->WindowManager->ProcessKey(Manager::Key(KEY_ALTF9));
				break;
			case MENU_COMMANDS_FINDFOLDER: // Find folder
				Global->WindowManager->ProcessKey(Manager::Key(KEY_ALTF10));
				break;
			case MENU_COMMANDS_VIEWHISTORY: // File view history
				Global->WindowManager->ProcessKey(Manager::Key(KEY_ALTF11));
				break;
			case MENU_COMMANDS_FOLDERHISTORY: // Folders history
				Global->WindowManager->ProcessKey(Manager::Key(KEY_ALTF12));
				break;
			case MENU_COMMANDS_SWAPPANELS: // Swap panels
				Global->WindowManager->ProcessKey(Manager::Key(KEY_CTRLU));
				break;
			case MENU_COMMANDS_TOGGLEPANELS: // Panels On/Off
				Global->WindowManager->ProcessKey(Manager::Key(KEY_CTRLO));
				break;
			case MENU_COMMANDS_COMPAREFOLDERS: // Compare folders
				Global->CtrlObject->Cp()->ActivePanel()->CompareDir();
				break;
			case MENU_COMMANDS_EDITUSERMENU: // Edit user menu
				{
					UserMenu(true);
				}
				break;
			case MENU_COMMANDS_FILEASSOCIATIONS: // File associations
				EditFileTypes();
				break;
			case MENU_COMMANDS_FOLDERSHORTCUTS: // Folder shortcuts
				Shortcuts().Configure();
				break;
			case MENU_COMMANDS_FILTER: // File panel filter
				Global->CtrlObject->Cp()->ActivePanel()->EditFilter();
				break;
			case MENU_COMMANDS_PLUGINCOMMANDS: // Plugin commands
				Global->WindowManager->ProcessKey(Manager::Key(KEY_F11));
				break;
			case MENU_COMMANDS_WINDOWSLIST: // Screens list
				Global->WindowManager->ProcessKey(Manager::Key(KEY_F12));
				break;
			case MENU_COMMANDS_PROCESSLIST: // Task list
				ShowProcessList();
				break;
			case MENU_COMMANDS_HOTPLUGLIST: // HotPlug list
				ShowHotplugDevices();
				break;
			}

			break;
		}
	case MENU_OPTIONS:
		{
			switch (VItem)
			{
			case MENU_OPTIONS_SYSTEMSETTINGS:   // System settings
				SystemSettings();
				break;
			case MENU_OPTIONS_PANELSETTINGS:   // Panel settings
				PanelSettings();
				break;
			case MENU_OPTIONS_TREESETTINGS: // Tree settings
				TreeSettings();
				break;
			case MENU_OPTIONS_INTERFACESETTINGS:   // Interface settings
				InterfaceSettings();
				break;
			case MENU_OPTIONS_LANGUAGES:   // Languages
				{
					string SavedLanguage = strLanguage.Get();
					if (SelectInterfaceLanguage())
					{
						try
						{
							far_language::instance().load(Global->g_strFarPath, static_cast<int>(lng::MNewFileName + 1));
						}
						catch (const far_exception& e)
						{
							Message(MSG_WARNING,
								msg(lng::MError),
								{
									e.get_message()
								},
								{ lng::MOk });
							strLanguage = SavedLanguage;
						}

						SelectHelpLanguage();
						Global->CtrlObject->Plugins->ReloadLanguage();
						os::env::set_variable(L"FARLANG", strLanguage);
						PrepareUnitStr();
						Global->WindowManager->InitKeyBar();
						Global->CtrlObject->Cp()->RedrawKeyBar();
						Global->CtrlObject->Cp()->SetScreenPosition();
					}

					break;
				}
			case MENU_OPTIONS_PLUGINSCONFIG:   // Plugins configuration
				Global->CtrlObject->Plugins->Configure();
				break;
			case MENU_OPTIONS_PLUGINSMANAGERSETTINGS: // Plugins manager settings
				PluginsManagerSettings();
				break;
			case MENU_OPTIONS_DIALOGSETTINGS:   // Dialog settings (police=5)
				DialogSettings();
				break;
			case MENU_OPTIONS_VMENUSETTINGS:    // VMenu settings
				VMenuSettings();
				break;
			case MENU_OPTIONS_CMDLINESETTINGS:   // Command line settings
				CmdlineSettings();
				break;
			case MENU_OPTIONS_AUTOCOMPLETESETTINGS: // AutoComplete settings
				AutoCompleteSettings();
				break;
			case MENU_OPTIONS_INFOPANELSETTINGS: // InfoPanel Settings
				InfoPanelSettings();
				break;
			case MENU_OPTIONS_MASKGROUPS:  // Groups of file masks
				MaskGroupsSettings();
				break;
			case MENU_OPTIONS_CONFIRMATIONS:   // Confirmations
				SetConfirmations();
				break;
			case MENU_OPTIONS_FILEPANELMODES:   // File panel modes
				SetFilePanelModes();
				break;
			case MENU_OPTIONS_FILEDESCRIPTIONS:   // File descriptions
				SetDizConfig();
				break;
			case MENU_OPTIONS_FOLDERINFOFILES:   // Folder description files
				SetFolderInfoFiles();
				break;
			case MENU_OPTIONS_VIEWERSETTINGS:  // Viewer settings
				ViewerConfig(ViOpt);
				break;
			case MENU_OPTIONS_EDITORSETTINGS:  // Editor settings
				EditorConfig(EdOpt);
				break;
			case MENU_OPTIONS_CODEPAGESSETTINGS: // Code pages
				{
					uintptr_t CodePage = CP_DEFAULT;
					Codepages().SelectCodePage(CodePage, true, false, true);
				}
				break;
			case MENU_OPTIONS_COLORS:  // Colors
				SetColors();
				break;
			case MENU_OPTIONS_FILESHIGHLIGHTING:  // Files highlighting
				Global->CtrlObject->HiFiles->HiEdit(0);
				break;
			case MENU_OPTIONS_SAVESETUP:  // Save setup
				Save(true);
				break;
			}

			break;
		}
	}

	const auto CurrentWindowType = Global->WindowManager->GetCurrentWindow()->GetType();
	// TODO:Здесь как то нужно изменить, чтобы учесть будущие новые типы полноэкранных окон
	//      или то, что, скажем редактор/вьювер может быть не полноэкранным

	if (!(CurrentWindowType == windowtype_viewer || CurrentWindowType == windowtype_editor))
		Global->CtrlObject->CmdLine()->Show();

	if (HItem != -1 && VItem != -1)
	{
		LastHItem = HItem;
		LastVItem = VItem;
	}
}

string GetFarIniString(const string& AppName, const string& KeyName, const string& Default)
{
	return os::GetPrivateProfileString(AppName, KeyName, Default, Global->g_strFarINI);
}

int GetFarIniInt(const string& AppName, const string& KeyName, int Default)
{
	return GetPrivateProfileInt(AppName.data(), KeyName.data(), Default, Global->g_strFarINI.data());
}

std::chrono::steady_clock::duration GetRedrawTimeout()
{
	return std::chrono::milliseconds(Global->Opt->RedrawTimeout);
}