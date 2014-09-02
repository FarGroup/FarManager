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
#include "colormix.hpp"
#include "message.hpp"
#include "stddlg.hpp"
#include "pathmix.hpp"
#include "panelmix.hpp"
#include "strmix.hpp"
#include "FarDlgBuilder.hpp"
#include "elevation.hpp"
#include "configdb.hpp"
#include "KnownGuids.hpp"
#include "vmenu2.hpp"
#include "codepage.hpp"
#include "DlgGuid.hpp"
#include "hmenu.hpp"
#include "usermenu.hpp"
#include "filetype.hpp"
#include "shortcuts.hpp"
#include "plist.hpp"
#include "hotplug.hpp"
#include "datetime.hpp"
#include "setcolor.hpp"
#include "language.hpp"
#include "plugins.hpp"
#include "manager.hpp"
#include "language.hpp"
#include "locale.hpp"

static const size_t predefined_panel_modes_count = 10;

// Стандартный набор разделителей
static const wchar_t* WordDiv0 = L"~!%^&*()+|{}:\"<>?`-=\\[];',./";

// Стандартный набор разделителей для функции Xlat
static const wchar_t* WordDivForXlat0=L" \t!#$%^&*()+|=\\/@?";

static const wchar_t* constBatchExt=L".BAT;.CMD;";

static const int DefaultTabSize = 8;

static wchar_t DefaultLanguage[100] = {};

static const size_t default_copy_buffer_size = 32 * 1024;


#if defined(TREEFILE_PROJECT)
static const wchar_t* constLocalDiskTemplate=L"LD.%D.%SN.tree";
static const wchar_t* constNetDiskTemplate=L"ND.%D.%SN.tree";
static const wchar_t* constNetPathTemplate=L"NP.%SR.%SH.tree";
static const wchar_t* constRemovableDiskTemplate=L"RD.%SN.tree";
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

void Options::SystemSettings()
{
	DialogBuilder Builder(MConfigSystemTitle, L"SystemSettings");

	DialogItemEx *DeleteToRecycleBinItem = Builder.AddCheckbox(MConfigRecycleBin, DeleteToRecycleBin);
	DialogItemEx *DeleteLinks = Builder.AddCheckbox(MConfigRecycleBinLink, DeleteToRecycleBinKillLink);
	DeleteLinks->Indent(4);
	Builder.LinkFlags(DeleteToRecycleBinItem, DeleteLinks, DIF_DISABLE);

	Builder.AddCheckbox(MConfigSystemCopy, CMOpt.UseSystemCopy);
	Builder.AddCheckbox(MConfigCopySharing, CMOpt.CopyOpened);
	Builder.AddCheckbox(MConfigScanJunction, ScanJunction);
	Builder.AddCheckbox(MConfigCreateUppercaseFolders, CreateUppercaseFolders);
	Builder.AddCheckbox(MConfigSmartFolderMonitor, SmartFolderMonitor);

	Builder.AddCheckbox(MConfigSaveHistory, SaveHistory);
	Builder.AddCheckbox(MConfigSaveFoldersHistory, SaveFoldersHistory);
	Builder.AddCheckbox(MConfigSaveViewHistory, SaveViewHistory);
	Builder.AddCheckbox(MConfigRegisteredTypes, UseRegisteredTypes);
	Builder.AddCheckbox(MConfigCloseCDGate, CloseCDGate);
	Builder.AddCheckbox(MConfigUpdateEnvironment, UpdateEnvironment);
	Builder.AddText(MConfigElevation);
	Builder.AddCheckbox(MConfigElevationModify, StoredElevationMode, ELEVATION_MODIFY_REQUEST)->Indent(4);
	Builder.AddCheckbox(MConfigElevationRead, StoredElevationMode, ELEVATION_READ_REQUEST)->Indent(4);
	Builder.AddCheckbox(MConfigElevationUsePrivileges, StoredElevationMode, ELEVATION_USE_PRIVILEGES)->Indent(4);
	Builder.AddCheckbox(MConfigAutoSave, AutoSaveSetup);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		ElevationMode = StoredElevationMode;
	}
}

void Options::PanelSettings()
{
	DialogBuilder Builder(MConfigPanelTitle, L"PanelSettings");
	BOOL AutoUpdate = AutoUpdateLimit;

	Builder.AddCheckbox(MConfigHidden, ShowHidden);
	Builder.AddCheckbox(MConfigHighlight, Highlight);
	Builder.AddCheckbox(MConfigSelectFolders, SelectFolders);
	Builder.AddCheckbox(MConfigRightClickSelect, RightClickSelect);
	Builder.AddCheckbox(MConfigSortFolderExt, SortFolderExt);
	Builder.AddCheckbox(MConfigReverseSort, ReverseSort);

	DialogItemEx *AutoUpdateEnabled = Builder.AddCheckbox(MConfigAutoUpdateLimit, &AutoUpdate);
	DialogItemEx *AutoUpdateLimitItem = Builder.AddIntEditField(AutoUpdateLimit, 6);
	Builder.LinkFlags(AutoUpdateEnabled, AutoUpdateLimitItem, DIF_DISABLE, false);
	DialogItemEx *AutoUpdateTextItem = Builder.AddTextBefore(AutoUpdateLimitItem, MConfigAutoUpdateLimit2);
	AutoUpdateLimitItem->Indent(4);
	AutoUpdateTextItem->Indent(4);
	Builder.AddCheckbox(MConfigAutoUpdateRemoteDrive, AutoUpdateRemoteDrive);

	Builder.AddSeparator();
	Builder.AddCheckbox(MConfigShowColumns, ShowColumnTitles);
	Builder.AddCheckbox(MConfigShowStatus, ShowPanelStatus);
	Builder.AddCheckbox(MConfigDetailedJunction, PanelDetailedJunction);
	Builder.AddCheckbox(MConfigShowTotal, ShowPanelTotals);
	Builder.AddCheckbox(MConfigShowFree, ShowPanelFree);
	Builder.AddCheckbox(MConfigShowScrollbar, ShowPanelScrollbar);
	Builder.AddCheckbox(MConfigShowScreensNumber, ShowScreensNumber);
	Builder.AddCheckbox(MConfigShowSortMode, ShowSortMode);
	Builder.AddCheckbox(MConfigShowDotsInRoot, ShowDotsInRoot);
	Builder.AddCheckbox(MConfigHighlightColumnSeparator, HighlightColumnSeparator);
	Builder.AddCheckbox(MConfigDoubleGlobalColumnSeparator, DoubleGlobalColumnSeparator);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (!AutoUpdate)
			AutoUpdateLimit = 0;

		Global->CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->Redraw();
	}
}

void Options::TreeSettings()
{
	DialogBuilder Builder(MConfigTreeTitle, L"TreeSettings");

	DialogItemEx *TemplateEdit;

	Builder.AddCheckbox(MConfigTreeAutoChange, Tree.AutoChangeFolder);

	TemplateEdit = Builder.AddIntEditField(Tree.MinTreeCount, 3);
	Builder.AddTextBefore(TemplateEdit, MConfigTreeLabelMinFolder);

#if defined(TREEFILE_PROJECT)
	DialogItemEx *Checkbox;

	Builder.AddSeparator(MConfigTreeLabel1);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelLocalDisk, Tree.LocalDisk);
	TemplateEdit = Builder.AddEditField(Tree.strLocalDisk, 44);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelNetDisk, Tree.NetDisk);
	TemplateEdit = Builder.AddEditField(Tree.strNetDisk, 44);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelNetPath, Tree.NetPath);
	TemplateEdit = Builder.AddEditField(Tree.strNetPath, 44);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelRemovableDisk, Tree.RemovableDisk);
	TemplateEdit = Builder.AddEditField(Tree.strRemovableDisk, 44);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelCDDisk, Tree.CDDisk);
	TemplateEdit = Builder.AddEditField(Tree.strCDDisk, 44);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Builder.AddText(MConfigTreeLabelSaveLocalPath);
	Builder.AddEditField(Tree.strSaveLocalPath, 48);

	Builder.AddText(MConfigTreeLabelSaveNetPath);
	Builder.AddEditField(Tree.strSaveNetPath, 48);

	Builder.AddText(MConfigTreeLabelExceptPath);
	Builder.AddEditField(Tree.strExceptPath, 48);
#endif

	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		Global->CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->Redraw();
	}
}

void Options::InterfaceSettings()
{
	DialogBuilder Builder(MConfigInterfaceTitle, L"InterfSettings");

	Builder.AddCheckbox(MConfigClock, Clock);
	Builder.AddCheckbox(MConfigViewerEditorClock, ViewerEditorClock);
	Builder.AddCheckbox(MConfigMouse, Mouse);
	Builder.AddCheckbox(MConfigKeyBar, ShowKeyBar);
	Builder.AddCheckbox(MConfigMenuBar, ShowMenuBar);
	DialogItemEx *SaverCheckbox = Builder.AddCheckbox(MConfigSaver, ScreenSaver);

	DialogItemEx *SaverEdit = Builder.AddIntEditField(ScreenSaverTime, 2);
	SaverEdit->Indent(4);
	Builder.AddTextAfter(SaverEdit, MConfigSaverMinutes);
	Builder.LinkFlags(SaverCheckbox, SaverEdit, DIF_DISABLE);

	Builder.AddCheckbox(MConfigCopyTotal, CMOpt.CopyShowTotal);
	Builder.AddCheckbox(MConfigCopyTimeRule, CMOpt.CopyTimeRule);
	Builder.AddCheckbox(MConfigDeleteTotal, DelOpt.ShowTotal);
	Builder.AddCheckbox(MConfigPgUpChangeDisk, PgUpChangeDisk);
	Builder.AddCheckbox(MConfigClearType, ClearType);
	DialogItemEx* SetIconCheck = Builder.AddCheckbox(MConfigSetConsoleIcon, SetIcon);
	DialogItemEx* SetAdminIconCheck = Builder.AddCheckbox(MConfigSetAdminConsoleIcon, SetAdminIcon);
	SetAdminIconCheck->Indent(4);
	Builder.LinkFlags(SetIconCheck, SetAdminIconCheck, DIF_DISABLE);
	Builder.AddText(MConfigTitleAddons);
	Builder.AddEditField(strTitleAddons, 47);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (CMOpt.CopyTimeRule)
			CMOpt.CopyTimeRule = 3;

		SetFarConsoleMode();
		Global->CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
		Global->CtrlObject->Cp()->SetScreenPosition();
		// $ 10.07.2001 SKV ! надо это делать, иначе если кейбар спрятали, будет полный рамс.
		Global->CtrlObject->Cp()->Redraw();
	}
}

void Options::AutoCompleteSettings()
{
	DialogBuilder Builder(MConfigAutoCompleteTitle, L"AutoCompleteSettings");
	DialogItemEx *ListCheck=Builder.AddCheckbox(MConfigAutoCompleteShowList, AutoComplete.ShowList);
	DialogItemEx *ModalModeCheck=Builder.AddCheckbox(MConfigAutoCompleteModalList, AutoComplete.ModalList);
	ModalModeCheck->Indent(4);
	Builder.AddCheckbox(MConfigAutoCompleteAutoAppend, AutoComplete.AppendCompletion);
	Builder.LinkFlags(ListCheck, ModalModeCheck, DIF_DISABLE);
	Builder.AddOKCancel();
	Builder.ShowDialog();
}

void Options::InfoPanelSettings()
{
	static const DialogBuilderListItem UNListItems[]=
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

	static const DialogBuilderListItem CNListItems[] =
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
	Builder.AddCheckbox(MConfigInfoPanelShowPowerStatus, InfoPanel.ShowPowerStatus);
	Builder.AddCheckbox(MConfigInfoPanelShowCDInfo, InfoPanel.ShowCDInfo);
	Builder.AddText(MConfigInfoPanelCNTitle);
	Builder.AddComboBox(InfoPanel.ComputerNameFormat, 50, CNListItems, ARRAYSIZE(CNListItems), DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddText(MConfigInfoPanelUNTitle);
	Builder.AddComboBox(InfoPanel.UserNameFormat, 50, UNListItems, ARRAYSIZE(UNListItems), DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
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

static void ApplyDefaultMaskGroups()
{
	static const simple_pair<string, string> Sets[] =
	{
		{L"arc", L"*.rar,*.zip,*.[zj],*.[bg7x]z,*.[bg]zip,*.tar,*.t[agbx]z,*.ar[cj],*.r[0-9][0-9],*.a[0-9][0-9],*.bz2,*.cab,*.msi,*.jar,*.lha,*.lzh,*.ha,*.ac[bei],*.pa[ck],*.rk,*.cpio,*.rpm,*.zoo,*.hqx,*.sit,*.ice,*.uc2,*.ain,*.imp,*.777,*.ufa,*.boa,*.bs[2a],*.sea,*.hpk,*.ddi,*.x2,*.rkv,*.[lw]sz,*.h[ay]p,*.lim,*.sqz,*.chz"},
		{L"temp", L"*.bak,*.tmp"},
		{L"exec", L"*.exe,*.com,*.bat,*.cmd,%PATHEXT%"},
	};

	std::for_each(CONST_RANGE(Sets, i)
	{
		Global->Db->GeneralCfg()->SetValue(L"Masks", i.first, i.second);
	});
}

static void FillMasksMenu(VMenu2& MasksMenu, int SelPos = 0)
{
	MasksMenu.DeleteItems();

	const auto MasksEnum = Global->Db->GeneralCfg()->GetStringValuesEnumerator(L"Masks");
	std::for_each(CONST_RANGE(MasksEnum, i)
	{
		MenuItemEx Item;
		string DisplayName(i.first);
		const int NameWidth = 10;
		TruncStrFromEnd(DisplayName, NameWidth);
		DisplayName.resize(NameWidth, L' ');
		Item.strName = DisplayName + L' ' + BoxSymbols[BS_V1] + L' ' + i.second;
		Item.UserData = UNSAFE_CSTR(i.first);
		Item.UserDataSize = (i.first.size()+1)*sizeof(wchar_t);
		MasksMenu.AddItem(Item);
	});
	MasksMenu.SetSelectPos(SelPos, 0);
}

void Options::MaskGroupsSettings()
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
			auto Item = static_cast<const wchar_t*>(Data);
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
				Item = L"";
			case KEY_ENTER:
			case KEY_NUMENTER:
			case KEY_F4:
				{
					if (Item)
					{
						string Name, Value;

						if (*Item)
						{
							Name = Item;
							Global->Db->GeneralCfg()->GetValue(L"Masks", Name, Value, L"");
						}
						DialogBuilder Builder(MMenuMaskGroups, L"MaskGroupsSettings");
						Builder.AddText(MMaskGroupName);
						Builder.AddEditField(&Name, 60);
						Builder.AddText(MMaskGroupMasks);
						Builder.AddEditField(&Value, 60);
						Builder.AddOKCancel();
						if(Builder.ShowDialog())
						{
							if(*Item)
							{
								Global->Db->GeneralCfg()->DeleteValue(L"Masks", Item);
							}
							Global->Db->GeneralCfg()->SetValue(L"Masks", Name, Value);
							Changed = true;
						}
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

void Options::DialogSettings()
{
	DialogBuilder Builder(MConfigDlgSetsTitle, L"DialogSettings");

	Builder.AddCheckbox(MConfigDialogsEditHistory, Dialogs.EditHistory);
	Builder.AddCheckbox(MConfigDialogsEditBlock, Dialogs.EditBlock);
	Builder.AddCheckbox(MConfigDialogsDelRemovesBlocks, Dialogs.DelRemovesBlocks);
	Builder.AddCheckbox(MConfigDialogsAutoComplete, Dialogs.AutoComplete);
	Builder.AddCheckbox(MConfigDialogsEULBsClear, Dialogs.EULBsClear);
	Builder.AddCheckbox(MConfigDialogsMouseButton, Dialogs.MouseButton);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (Dialogs.MouseButton )
			Dialogs.MouseButton = 0xFFFF;
	}
}

void Options::VMenuSettings()
{

	static const DialogBuilderListItem CAListItems[]=
	{
		{ MConfigVMenuClickCancel, VMENUCLICK_CANCEL },  // Cancel menu
		{ MConfigVMenuClickApply,  VMENUCLICK_APPLY  },  // Execute selected item
		{ MConfigVMenuClickIgnore, VMENUCLICK_IGNORE },  // Do nothing
	};

	static const simple_pair<LNGID, IntOption*> DialogItems[] =
	{
		{ MConfigVMenuLBtnClick, &VMenu.LBtnClick },
		{ MConfigVMenuRBtnClick, &VMenu.RBtnClick },
		{ MConfigVMenuMBtnClick, &VMenu.MBtnClick },
	};

	DialogBuilder Builder(MConfigVMenuTitle, L"VMenuSettings");

	std::for_each(CONST_RANGE(DialogItems, i)
	{
		Builder.AddText(i.first);
		Builder.AddComboBox(*i.second, 40, CAListItems, ARRAYSIZE(CAListItems), DIF_LISTAUTOHIGHLIGHT | DIF_LISTWRAPMODE | DIF_DROPDOWNLIST);
	});

	Builder.AddOKCancel();
	Builder.ShowDialog();
}

void Options::CmdlineSettings()
{
	DialogBuilder Builder(MConfigCmdlineTitle, L"CmdlineSettings");

	Builder.AddCheckbox(MConfigCmdlineEditBlock, CmdLine.EditBlock);
	Builder.AddCheckbox(MConfigCmdlineDelRemovesBlocks, CmdLine.DelRemovesBlocks);
	Builder.AddCheckbox(MConfigCmdlineAutoComplete, CmdLine.AutoComplete);
	DialogItemEx *UsePromptFormat = Builder.AddCheckbox(MConfigCmdlineUsePromptFormat, CmdLine.UsePromptFormat);
	DialogItemEx *PromptFormat = Builder.AddEditField(CmdLine.strPromptFormat, 33);
	PromptFormat->Indent(4);
	Builder.LinkFlags(UsePromptFormat, PromptFormat, DIF_DISABLE);

	UsePromptFormat = Builder.AddCheckbox(MConfigCmdlineUseHomeDir, Exec.UseHomeDir);
	PromptFormat = Builder.AddEditField(Exec.strHomeDir, 33);
	PromptFormat->Indent(4);
	Builder.LinkFlags(UsePromptFormat, PromptFormat, DIF_DISABLE);

	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		Global->CtrlObject->CmdLine->SetPersistentBlocks(CmdLine.EditBlock);
		Global->CtrlObject->CmdLine->SetDelRemovesBlocks(CmdLine.DelRemovesBlocks);
		Global->CtrlObject->CmdLine->SetAutoComplete(CmdLine.AutoComplete);
	}
}

void Options::SetConfirmations()
{
	DialogBuilder Builder(MSetConfirmTitle, L"ConfirmDlg");

	Builder.AddCheckbox(MSetConfirmCopy, Confirm.Copy);
	Builder.AddCheckbox(MSetConfirmMove, Confirm.Move);
	Builder.AddCheckbox(MSetConfirmRO, Confirm.RO);
	Builder.AddCheckbox(MSetConfirmDrag, Confirm.Drag);
	Builder.AddCheckbox(MSetConfirmDelete, Confirm.Delete);
	Builder.AddCheckbox(MSetConfirmDeleteFolders, Confirm.DeleteFolder);
	Builder.AddCheckbox(MSetConfirmEsc, Confirm.Esc);
	Builder.AddCheckbox(MSetConfirmRemoveConnection, Confirm.RemoveConnection);
	Builder.AddCheckbox(MSetConfirmRemoveSUBST, Confirm.RemoveSUBST);
	Builder.AddCheckbox(MSetConfirmDetachVHD, Confirm.DetachVHD);
	Builder.AddCheckbox(MSetConfirmRemoveHotPlug, Confirm.RemoveHotPlug);
	Builder.AddCheckbox(MSetConfirmAllowReedit, Confirm.AllowReedit);
	Builder.AddCheckbox(MSetConfirmHistoryClear, Confirm.HistoryClear);
	Builder.AddCheckbox(MSetConfirmExit, Confirm.Exit);
	Builder.AddOKCancel();

	Builder.ShowDialog();
}

void Options::PluginsManagerSettings()
{
	DialogBuilder Builder(MPluginsManagerSettingsTitle, L"PluginsManagerSettings");
#ifndef NO_WRAPPER
	Builder.AddCheckbox(MPluginsManagerOEMPluginsSupport, LoadPlug.OEMPluginsSupport);
#endif // NO_WRAPPER
	Builder.AddCheckbox(MPluginsManagerScanSymlinks, LoadPlug.ScanSymlinks);
	Builder.AddSeparator(MPluginConfirmationTitle);
	Builder.AddCheckbox(MPluginsManagerOFP, PluginConfirm.OpenFilePlugin);
	DialogItemEx *StandardAssoc = Builder.AddCheckbox(MPluginsManagerStdAssoc, PluginConfirm.StandardAssociation);
	DialogItemEx *EvenIfOnlyOne = Builder.AddCheckbox(MPluginsManagerEvenOne, PluginConfirm.EvenIfOnlyOnePlugin);
	StandardAssoc->Indent(2);
	EvenIfOnlyOne->Indent(4);

	Builder.AddCheckbox(MPluginsManagerSFL, PluginConfirm.SetFindList);
	Builder.AddCheckbox(MPluginsManagerPF, PluginConfirm.Prefix);
	Builder.AddOKCancel();

	Builder.ShowDialog();
}

void Options::SetDizConfig()
{
	DialogBuilder Builder(MCfgDizTitle, L"FileDiz");

	Builder.AddText(MCfgDizListNames);
	Builder.AddEditField(Diz.strListNames, 65);
	Builder.AddSeparator();

	Builder.AddCheckbox(MCfgDizSetHidden, Diz.SetHidden);
	Builder.AddCheckbox(MCfgDizROUpdate, Diz.ROUpdate);
	DialogItemEx *StartPos = Builder.AddIntEditField(Diz.StartPos, 2);
	Builder.AddTextAfter(StartPos, MCfgDizStartPos);
	Builder.AddSeparator();

	static const int DizOptions[] = { MCfgDizNotUpdate, MCfgDizUpdateIfDisplayed, MCfgDizAlwaysUpdate };
	Builder.AddRadioButtons(Diz.UpdateMode, 3, DizOptions);
	Builder.AddSeparator();

	Builder.AddCheckbox(MCfgDizAnsiByDefault, Diz.AnsiByDefault);
	Builder.AddCheckbox(MCfgDizSaveInUTF, Diz.SaveInUTF);
	Builder.AddOKCancel();
	Builder.ShowDialog();
}

void Options::ViewerConfig(Options::ViewerOptions &ViOptRef, bool Local)
{
	intptr_t save_pos = 0, save_cp = 0, id = 0;
	bool prev_save_cp_value = ViOpt.SaveCodepage, inside = false;

	DialogBuilder Builder(MViewConfigTitle, L"ViewerSettings", [&](Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2) -> intptr_t
	{
		if (Msg == DN_INITDIALOG && save_pos)
		{
			Dlg->SendMessage(DM_ENABLE, save_cp, reinterpret_cast<void *>(ViOpt.SavePos ? false : true));
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
				Dlg->SendMessage(DM_SETCHECK, save_cp, reinterpret_cast<void *>(Param2 ? true : prev_save_cp_value));
				Dlg->SendMessage(DM_ENABLE, save_cp, reinterpret_cast<void *>(Param2 ? false : true));
				inside = false;
			}
			else if (Param1 == save_cp && !inside)
			{
				prev_save_cp_value = (Param2 != 0);
			}
		}
		return Dlg->DefProc(Msg, Param1, Param2);
	});

	std::vector<DialogBuilderListItem2> Items; //Must live until Dialog end

	if (!Local)
	{
		++id; Builder.AddCheckbox(MViewConfigExternalF3, ViOpt.UseExternalViewer);
		++id; Builder.AddText(MViewConfigExternalCommand);
		++id; Builder.AddEditField(strExternalViewer, 64, L"ExternalViewer", DIF_EDITPATH|DIF_EDITPATHEXEC);
		++id; Builder.AddSeparator(MViewConfigInternal);
	}

	Builder.StartColumns();
	++id; Builder.AddCheckbox(MViewConfigPersistentSelection, ViOptRef.PersistentBlocks);
	++id; Builder.AddCheckbox(MViewConfigEditAutofocus, ViOptRef.SearchEditFocus);
	++id; DialogItemEx *TabSize = Builder.AddIntEditField(ViOptRef.TabSize, 3);
	++id; Builder.AddTextAfter(TabSize, MViewConfigTabSize);
	Builder.ColumnBreak();
	++id; Builder.AddCheckbox(MViewConfigArrows, ViOptRef.ShowArrows);
	++id; Builder.AddCheckbox(MViewConfigVisible0x00, ViOptRef.Visible0x00);
	++id; Builder.AddCheckbox(MViewConfigScrollbar, ViOptRef.ShowScrollbar);
	Builder.EndColumns();

	if (!Local)
	{
		++id; Builder.AddSeparator();
		Builder.StartColumns();
		save_pos = ++id; Builder.AddCheckbox(MViewConfigSavePos, ViOpt.SavePos);
		save_cp = ++id; Builder.AddCheckbox(MViewConfigSaveCodepage, ViOpt.SaveCodepage);
		DialogItemEx *MaxLineSize = Builder.AddIntEditField(ViOpt.MaxLineSize, 6);
		Builder.AddTextAfter(MaxLineSize, MViewConfigMaxLineSize);
		Builder.ColumnBreak();
		Builder.AddCheckbox(MViewConfigSaveShortPos, ViOpt.SaveShortPos);
		Builder.AddCheckbox(MViewConfigSaveWrapMode, ViOpt.SaveWrapMode);
		Builder.AddCheckbox(MViewAutoDetectCodePage, ViOpt.AutoDetectCodePage);
		Builder.EndColumns();
		Builder.AddText(MViewConfigDefaultCodePage);
		Codepages().FillCodePagesList(Items, false, false, false, true);
		Builder.AddComboBox(ViOpt.DefaultCodePage, 64, Items, DIF_LISTWRAPMODE|DIF_LISTAUTOHIGHLIGHT);
	}

	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (!Local)
		{
			if (!ViOpt.MaxLineSize)
				ViOpt.MaxLineSize = Options::ViewerOptions::eDefLineSize;
			else
				ViOpt.MaxLineSize = std::min(std::max((__int64)ViewerOptions::eMinLineSize, ViOpt.MaxLineSize.Get()), (__int64)ViewerOptions::eMaxLineSize);
		}

		if (ViOptRef.TabSize<1 || ViOptRef.TabSize>512)
			ViOptRef.TabSize = DefaultTabSize;
	}
}

void Options::EditorConfig(Options::EditorOptions &EdOptRef, bool Local)
{
	DialogBuilder Builder(MEditConfigTitle, L"EditorSettings");

	std::vector<DialogBuilderListItem2> Items; //Must live until Dialog end

	if (!Local)
	{
		Builder.AddCheckbox(MEditConfigEditorF4, EdOpt.UseExternalEditor);
		Builder.AddText(MEditConfigEditorCommand);
		Builder.AddEditField(strExternalEditor, 64, L"ExternalEditor", DIF_EDITPATH|DIF_EDITPATHEXEC);
		Builder.AddSeparator(MEditConfigInternal);
	}

	Builder.AddText(MEditConfigExpandTabsTitle);
	static const DialogBuilderListItem ExpandTabsItems[] =
	{
		{ MEditConfigDoNotExpandTabs, EXPAND_NOTABS },
		{ MEditConfigExpandTabs, EXPAND_NEWTABS },
		{ MEditConfigConvertAllTabsToSpaces, EXPAND_ALLTABS }
	};
	Builder.AddComboBox(EdOptRef.ExpandTabs, 64, ExpandTabsItems, ARRAYSIZE(ExpandTabsItems), DIF_LISTAUTOHIGHLIGHT | DIF_LISTWRAPMODE | DIF_DROPDOWNLIST);

	Builder.StartColumns();
	Builder.AddCheckbox(MEditConfigPersistentBlocks, EdOptRef.PersistentBlocks);
	Builder.AddCheckbox(MEditConfigDelRemovesBlocks, EdOptRef.DelRemovesBlocks);
	Builder.AddCheckbox(MEditConfigAutoIndent, EdOptRef.AutoIndent);
	DialogItemEx *TabSize = Builder.AddIntEditField(EdOptRef.TabSize, 3);
	Builder.AddTextAfter(TabSize, MEditConfigTabSize);
	Builder.AddCheckbox(MEditShowWhiteSpace, EdOptRef.ShowWhiteSpace);
	Builder.ColumnBreak();
	Builder.AddCheckbox(MEditCursorBeyondEnd, EdOptRef.CursorBeyondEOL);
	Builder.AddCheckbox(MEditConfigPickUpWord, EdOptRef.SearchPickUpWord);
	Builder.AddCheckbox(MEditConfigSelFound, EdOptRef.SearchSelFound);
	Builder.AddCheckbox(MEditConfigCursorAtEnd, EdOptRef.SearchCursorAtEnd);
	Builder.AddCheckbox(MEditConfigScrollbar, EdOptRef.ShowScrollBar);
	Builder.EndColumns();

	if (!Local)
	{
		Builder.AddSeparator();
		Builder.AddCheckbox(MEditConfigSavePos, EdOptRef.SavePos);
		Builder.AddCheckbox(MEditConfigSaveShortPos, EdOptRef.SaveShortPos);
		Builder.AddCheckbox(MEditShareWrite, EdOpt.EditOpenedForWrite);
		Builder.AddCheckbox(MEditLockROFileModification, EdOpt.ReadOnlyLock, 1);
		Builder.AddCheckbox(MEditWarningBeforeOpenROFile, EdOpt.ReadOnlyLock, 2);
		Builder.AddCheckbox(MEditAutoDetectCodePage, EdOpt.AutoDetectCodePage);
		Builder.AddText(MEditConfigDefaultCodePage);
		Codepages().FillCodePagesList(Items, false, false, false, false);
		Builder.AddComboBox(EdOpt.DefaultCodePage, 64, Items, DIF_LISTWRAPMODE|DIF_LISTAUTOHIGHLIGHT);
	}

	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (EdOptRef.TabSize<1 || EdOptRef.TabSize>512)
			EdOptRef.TabSize = DefaultTabSize;
	}
}

void Options::SetFolderInfoFiles()
{
	string strFolderInfoFiles;

	if (GetString(MSG(MSetFolderInfoTitle),MSG(MSetFolderInfoNames),L"FolderInfoFiles",
	              InfoPanel.strFolderInfoFiles.data(),strFolderInfoFiles,L"FolderDiz",FIB_ENABLEEMPTY|FIB_BUTTONS))
	{
		InfoPanel.strFolderInfoFiles = strFolderInfoFiles;

		if (Global->CtrlObject->Cp()->LeftPanel->GetType() == INFO_PANEL)
			Global->CtrlObject->Cp()->LeftPanel->Update(0);

		if (Global->CtrlObject->Cp()->RightPanel->GetType() == INFO_PANEL)
			Global->CtrlObject->Cp()->RightPanel->Update(0);
	}
}

static void ResetViewModes(PanelViewSettings* Modes, int Index = -1)
{
	static const struct panelmode_init
	{
		struct columns_init
		{
			size_t count;
			column init[10]; // good enough
		}
		Columns, StatusColumns;
		unsigned __int64 Flags;
	}
	Init[] =
	{
		{
			{3, {{COLUMN_MARK|NAME_COLUMN, 0, COUNT_WIDTH}, {SIZE_COLUMN|COLUMN_COMMAS, 10, COUNT_WIDTH}, {DATE_COLUMN, 0, COUNT_WIDTH}}},
			{1, {{COLUMN_RIGHTALIGN|NAME_COLUMN, 0, COUNT_WIDTH}}},
			PVS_ALIGNEXTENSIONS
		},
		{
			{3, {{NAME_COLUMN, 0, COUNT_WIDTH}, {NAME_COLUMN, 0, COUNT_WIDTH}, {NAME_COLUMN, 0, COUNT_WIDTH}}},
			{4, {{COLUMN_RIGHTALIGN|NAME_COLUMN, 0, COUNT_WIDTH}, {SIZE_COLUMN, 6, COUNT_WIDTH}, {DATE_COLUMN, 0, COUNT_WIDTH}, {TIME_COLUMN, 5, COUNT_WIDTH}}},
			PVS_ALIGNEXTENSIONS
		},
		{
			{2, {{NAME_COLUMN, 0, COUNT_WIDTH}, {NAME_COLUMN, 0, COUNT_WIDTH}}},
			{4, {{COLUMN_RIGHTALIGN|NAME_COLUMN, 0, COUNT_WIDTH}, {SIZE_COLUMN, 6, COUNT_WIDTH}, {DATE_COLUMN, 0, COUNT_WIDTH}, {TIME_COLUMN, 5, COUNT_WIDTH}}},
			0
		},
		{
			{4, {{NAME_COLUMN, 0, COUNT_WIDTH}, {SIZE_COLUMN, 6, COUNT_WIDTH}, {DATE_COLUMN, 0, COUNT_WIDTH}, {TIME_COLUMN, 5, COUNT_WIDTH}}},
			{1, {{COLUMN_RIGHTALIGN|NAME_COLUMN, 0, COUNT_WIDTH}}},
			PVS_ALIGNEXTENSIONS
		},
		{
			{2, {{NAME_COLUMN, 0, COUNT_WIDTH}, {SIZE_COLUMN, 6, COUNT_WIDTH}}},
			{4, {{COLUMN_RIGHTALIGN|NAME_COLUMN, 0, COUNT_WIDTH}, {SIZE_COLUMN, 6, COUNT_WIDTH}, {DATE_COLUMN, 0, COUNT_WIDTH}, {TIME_COLUMN, 5, COUNT_WIDTH}}},
			0
		},
		{
			{7, {{NAME_COLUMN, 0, COUNT_WIDTH}, {SIZE_COLUMN, 6, COUNT_WIDTH}, {PACKED_COLUMN, 6, COUNT_WIDTH}, {WDATE_COLUMN, 14, COUNT_WIDTH}, {CDATE_COLUMN, 14, COUNT_WIDTH}, {ADATE_COLUMN, 14, COUNT_WIDTH}, {ATTR_COLUMN, 0, COUNT_WIDTH}}},
			{1, {{COLUMN_RIGHTALIGN|NAME_COLUMN, 0, COUNT_WIDTH}}},
			PVS_ALIGNEXTENSIONS|PVS_FULLSCREEN
		},
		{
			{2, {{NAME_COLUMN, 40, PERCENT_WIDTH}, {DIZ_COLUMN, 0, COUNT_WIDTH}}},
			{4, {{COLUMN_RIGHTALIGN|NAME_COLUMN, 0, COUNT_WIDTH}, {SIZE_COLUMN, 6, COUNT_WIDTH}, {DATE_COLUMN, 0, COUNT_WIDTH}, {TIME_COLUMN, 5, COUNT_WIDTH}}},
			PVS_ALIGNEXTENSIONS
		},
		{
			{3, {{NAME_COLUMN, 0, COUNT_WIDTH}, {SIZE_COLUMN, 6, COUNT_WIDTH}, {DIZ_COLUMN, 70, PERCENT_WIDTH}}},
			{1, {{COLUMN_RIGHTALIGN|NAME_COLUMN, 0, COUNT_WIDTH}}},
			PVS_ALIGNEXTENSIONS|PVS_FULLSCREEN
		},
		{
			{3, {{NAME_COLUMN, 0, COUNT_WIDTH}, {SIZE_COLUMN, 6, COUNT_WIDTH}, {OWNER_COLUMN, 15, COUNT_WIDTH}}},
			{4, {{COLUMN_RIGHTALIGN|NAME_COLUMN, 0, COUNT_WIDTH}, {SIZE_COLUMN, 6, COUNT_WIDTH}, {DATE_COLUMN, 0, COUNT_WIDTH}, {TIME_COLUMN, 15, COUNT_WIDTH}}},
			PVS_ALIGNEXTENSIONS
		},
		{
			{3, {{NAME_COLUMN, 0, COUNT_WIDTH}, {SIZE_COLUMN, 6, COUNT_WIDTH}, {NUMLINK_COLUMN, 3, COUNT_WIDTH}}},
			{4, {{COLUMN_RIGHTALIGN|NAME_COLUMN, 0, COUNT_WIDTH}, {SIZE_COLUMN, 6, COUNT_WIDTH}, {DATE_COLUMN, 0, COUNT_WIDTH}, {TIME_COLUMN, 5, COUNT_WIDTH}}},
			PVS_ALIGNEXTENSIONS
		},
	};
	static_assert(ARRAYSIZE(Init) == predefined_panel_modes_count, "not all initial modes defined");

	auto InitMode = [](const panelmode_init& src, PanelViewSettings& dst)
	{
		dst.PanelColumns.resize(src.Columns.count);
		std::copy(src.Columns.init, src.Columns.init + src.Columns.count, dst.PanelColumns.begin());
		dst.StatusColumns.resize(src.StatusColumns.count);
		std::copy(src.StatusColumns.init, src.StatusColumns.init + src.StatusColumns.count, dst.StatusColumns.begin());
		dst.Flags = src.Flags;
		dst.Name.clear();
	};

	if (Index < 0)
	{
		for_each_2(ALL_CONST_RANGE(Init), Modes, InitMode);
	}
	else
	{
		InitMode(Init[Index], *Modes);
	}
}

void Options::SetFilePanelModes()
{
	auto DisplayModeToReal = [](size_t Mode)->size_t
	{
		if (Mode < predefined_panel_modes_count)
		{
			Mode = (Mode == 9)? 0 : (Mode + 1);
		}
		else
		{
			--Mode;
		}
		return Mode;
	};

	auto RealModeToDisplay = [](size_t Mode)->size_t
	{
		if (Mode < predefined_panel_modes_count)
		{
			Mode = (Mode == 0)? 9 : (Mode - 1);
		}
		else
		{
			++Mode;
		}
		return Mode;
	};

	size_t CurMode=0;

	if (Global->CtrlObject->Cp()->ActivePanel()->GetType()==FILE_PANEL)
	{
		CurMode=Global->CtrlObject->Cp()->ActivePanel()->GetViewMode();
		CurMode = RealModeToDisplay(CurMode);
	}

	for(;;)
	{
		static const LNGID PredefinedNames[] =
		{
			MMenuBriefView,
			MMenuMediumView,
			MMenuFullView,
			MMenuWideView,
			MMenuDetailedView,
			MMenuDizView,
			MMenuLongDizView,
			MMenuOwnersView,
			MMenuLinksView,
			MMenuAlternativeView,
		};
		static_assert(ARRAYSIZE(PredefinedNames) == predefined_panel_modes_count, "Not all panel modes defined");

		auto MenuCount = ViewSettings.size();
		// +1 for separator
		std::vector<MenuDataEx> ModeListMenu(MenuCount > predefined_panel_modes_count? MenuCount + 1: MenuCount);

		for (size_t i = 0; i < ViewSettings.size(); ++i)
		{
			ModeListMenu[RealModeToDisplay(i)].Name = ViewSettings[i].Name.data();
		}

		for (size_t i = 0; i < predefined_panel_modes_count; ++i)
		{
			if (!*ModeListMenu[i].Name)
				ModeListMenu[i].Name = MSG(PredefinedNames[i]);
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
			VMenu2 ModeList(MSG(MEditPanelModes),ModeListMenu.data(), ModeListMenu.size(), ScrY-4);
			ModeList.SetPosition(-1,-1,0,0);
			ModeList.SetHelp(L"PanelViewModes");
			ModeList.SetFlags(VMENU_WRAPMODE);
			ModeList.SetId(PanelViewModesId);
			ModeNumber=ModeList.Run([&](int Key)->int
			{
				switch (Key)
				{
				case KEY_CTRLENTER:
				case KEY_RCTRLENTER:
				case KEY_CTRLSHIFTENTER:
				case KEY_RCTRLSHIFTENTER:
					{
						auto PanelPtr = Global->CtrlObject->Cp()->ActivePanel();
						if (Key & KEY_SHIFT)
						{
							PanelPtr = Global->CtrlObject->Cp()->PassivePanel();
						}
						PanelPtr->SetViewMode(static_cast<int>(DisplayModeToReal(ModeList.GetSelectPos())));
						Global->CtrlObject->Cp()->Redraw();
					}
					return 1;

				case KEY_INS:
					AddNewMode = true;
					ModeList.Close();
					break;

				case KEY_DEL:
					if (ModeList.GetSelectPos() >= (int)predefined_panel_modes_count)
					{
						DeleteMode = true;
						ModeList.Close();
					}
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
			auto SwitchToAnotherMode = [&](Panel* p)
			{
				auto RealMode = static_cast<int>(DisplayModeToReal(CurMode));
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
			{DI_TEXT,      5, 2, 0, 2,0,nullptr,nullptr,0,MSG(MEditPanelModeName)},
			{DI_EDIT,      5, 3,70, 3,0,nullptr,nullptr,DIF_FOCUS,L""},
			{DI_TEXT,      5, 4, 0, 4,0,nullptr,nullptr,0,MSG(MEditPanelModeTypes)},
			{DI_EDIT,      5, 5,35, 5,0,nullptr,nullptr,0,L""},
			{DI_TEXT,      5, 6, 0, 6,0,nullptr,nullptr,0,MSG(MEditPanelModeWidths)},
			{DI_EDIT,      5, 7,35, 7,0,nullptr,nullptr,0,L""},
			{DI_TEXT,     38, 4, 0, 4,0,nullptr,nullptr,0,MSG(MEditPanelModeStatusTypes)},
			{DI_EDIT,     38, 5,70, 5,0,nullptr,nullptr,0,L""},
			{DI_TEXT,     38, 6, 0, 6,0,nullptr,nullptr,0,MSG(MEditPanelModeStatusWidths)},
			{DI_EDIT,     38, 7,70, 7,0,nullptr,nullptr,0,L""},
			{DI_TEXT,     -1, 8, 0, 8,0,nullptr,nullptr,DIF_SEPARATOR,MSG(MEditPanelReadHelp)},
			{DI_CHECKBOX,  5, 9, 0, 9,0,nullptr,nullptr,0,MSG(MEditPanelModeFullscreen)},
			{DI_CHECKBOX,  5,10, 0,10,0,nullptr,nullptr,0,MSG(MEditPanelModeAlignExtensions)},
			{DI_CHECKBOX,  5,11, 0,11,0,nullptr,nullptr,0,MSG(MEditPanelModeAlignFolderExtensions)},
			{DI_CHECKBOX,  5,12, 0,12,0,nullptr,nullptr,0,MSG(MEditPanelModeFoldersUpperCase)},
			{DI_CHECKBOX,  5,13, 0,13,0,nullptr,nullptr,0,MSG(MEditPanelModeFilesLowerCase)},
			{DI_CHECKBOX,  5,14, 0,14,0,nullptr,nullptr,0,MSG(MEditPanelModeUpperToLowerCase)},
			{DI_TEXT,     -1,15, 0,15,0,nullptr,nullptr,DIF_SEPARATOR,L""},
			{DI_BUTTON,    0,16, 0,16,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
			{DI_BUTTON,    0,16, 0,16,0,nullptr,nullptr,DIF_CENTERGROUP|(ModeNumber < (int)predefined_panel_modes_count? 0 : DIF_DISABLE),MSG(MReset)},
			{DI_BUTTON,    0,16, 0,16,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
		};
		auto ModeDlg = MakeDialogItemsEx(ModeDlgData);
		int ExitCode;
		RemoveHighlights(ModeDlg[MD_DOUBLEBOX].strData);

		if (!AddNewMode)
		{
			auto& CurrentSettings = ViewSettings[ModeNumber];

			ModeDlg[MD_CHECKBOX_FULLSCREEN].Selected = (CurrentSettings.Flags & PVS_FULLSCREEN) != 0;
			ModeDlg[MD_CHECKBOX_ALIGNFILEEXT].Selected = (CurrentSettings.Flags & PVS_ALIGNEXTENSIONS) != 0;
			ModeDlg[MD_CHECKBOX_ALIGNFOLDEREXT].Selected = (CurrentSettings.Flags & PVS_FOLDERALIGNEXTENSIONS) != 0;
			ModeDlg[MD_CHECKBOX_FOLDERUPPERCASE].Selected = (CurrentSettings.Flags & PVS_FOLDERUPPERCASE) != 0;
			ModeDlg[MD_CHECKBOX_FILESLOWERCASE].Selected = (CurrentSettings.Flags & PVS_FILELOWERCASE) != 0;
			ModeDlg[MD_CHECKBOX_UPPERTOLOWERCASE].Selected = (CurrentSettings.Flags & PVS_FILEUPPERTOLOWERCASE) != 0;
			ModeDlg[MD_EDITNAME].strData = CurrentSettings.Name;
			ViewSettingsToText(CurrentSettings.PanelColumns, ModeDlg[MD_EDITTYPES].strData, ModeDlg[MD_EDITWIDTHS].strData);
			ViewSettingsToText(CurrentSettings.StatusColumns, ModeDlg[MD_EDITSTATUSTYPES].strData, ModeDlg[MD_EDITSTATUSWIDTHS].strData);
		}

		{
			Dialog Dlg(ModeDlg);
			Dlg.SetPosition(-1,-1,76,19);
			Dlg.SetHelp(L"PanelViewModes");
			Dlg.SetId(PanelViewModesEditId);
			Dlg.Process();
			ExitCode=Dlg.GetExitCode();
		}

		if (ExitCode == MD_BUTTON_OK || ExitCode == MD_BUTTON_RESET)
		{
			PanelViewSettings NewSettings;

			if (ExitCode == MD_BUTTON_OK)
			{
				if (ModeDlg[MD_CHECKBOX_FULLSCREEN].Selected)
					NewSettings.Flags|=PVS_FULLSCREEN;
				if (ModeDlg[MD_CHECKBOX_ALIGNFILEEXT].Selected)
					NewSettings.Flags|=PVS_ALIGNEXTENSIONS;
				if (ModeDlg[MD_CHECKBOX_ALIGNFOLDEREXT].Selected)
					NewSettings.Flags|=PVS_FOLDERALIGNEXTENSIONS;
				if (ModeDlg[MD_CHECKBOX_FOLDERUPPERCASE].Selected)
					NewSettings.Flags|=PVS_FOLDERUPPERCASE;
				if (ModeDlg[MD_CHECKBOX_FILESLOWERCASE].Selected)
					NewSettings.Flags|=PVS_FILELOWERCASE;
				if (ModeDlg[MD_CHECKBOX_UPPERTOLOWERCASE].Selected)
					NewSettings.Flags|=PVS_FILEUPPERTOLOWERCASE;
				NewSettings.Name = ModeDlg[MD_EDITNAME].strData;
				TextToViewSettings(ModeDlg[MD_EDITTYPES].strData, ModeDlg[MD_EDITWIDTHS].strData, NewSettings.PanelColumns);
				TextToViewSettings(ModeDlg[MD_EDITSTATUSTYPES].strData, ModeDlg[MD_EDITSTATUSWIDTHS].strData, NewSettings.StatusColumns);
			}
			else
			{
				ResetViewModes(&NewSettings, ModeNumber);
			}

			if (AddNewMode)
			{
				AddViewSettings(ModeNumber, std::move(NewSettings));
			}
			else
			{
				SetViewSettings(ModeNumber, std::move(NewSettings));
			}
			Global->CtrlObject->Cp()->LeftPanel->SortFileList(TRUE);
			Global->CtrlObject->Cp()->RightPanel->SortFileList(TRUE);
			Global->CtrlObject->Cp()->SetScreenPosition();
			int LeftMode=Global->CtrlObject->Cp()->LeftPanel->GetViewMode();
			int RightMode=Global->CtrlObject->Cp()->RightPanel->GetViewMode();
			Global->CtrlObject->Cp()->LeftPanel->SetViewMode(LeftMode);
			Global->CtrlObject->Cp()->RightPanel->SetViewMode(RightMode);
			Global->CtrlObject->Cp()->LeftPanel->Redraw();
			Global->CtrlObject->Cp()->RightPanel->Redraw();
		}
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

	string ListItemString;

	FarListItem MakeListItem()
	{
		FarListItem Item;
		Item.Flags = 0;
		Item.Reserved[0] = Item.Reserved[1] = 0;

		string Type = Value->typeToString();
		Type.resize(std::max(Type.size(), size_t(7)), L' ');

		ListItemString = string(KeyName) + L"." + ValName;
		ListItemString.resize(std::max(ListItemString.size(), size_t(42)), L' ');
		ListItemString += BoxSymbols[BS_V1] + Type + BoxSymbols[BS_V1] + Value->toString() + Value->ExInfo();
		if(!Value->IsDefault(this))
		{
			Item.Flags = LIF_CHECKED|L'*';
		}
		Item.Text = ListItemString.data();
		return Item;
	}

	bool Edit(bool Hex)
	{
		DialogBuilder Builder;
		Builder.AddText((string(KeyName) + L"." + ValName + L" (" + Value->typeToString() + L"):").data());
		int Result = 0;
		if (!Value->Edit(&Builder, 40, Hex))
		{
			static_cast<DialogBuilderBase<DialogItemEx>*>(&Builder)->AddOKCancel(MOk, MReset, MCancel);
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

inline bool ParseIntValue(const string& sValue, long long& iValue)
{
	bool Result = false;

	try
	{
		iValue = std::stoll(sValue);
		Result = true;
	}
	catch (std::exception&)
	{
		try
		{
			iValue = std::stoull(sValue);
			Result = true;
		}
		catch (std::exception&)
		{
			if (!StrCmpI(sValue, L"false"))
			{
				iValue = 0;
				Result = true;
			}
			else if (!StrCmpI(sValue, L"true"))
			{
				iValue = 1;
				Result = true;
			}
			else if (!StrCmpI(sValue, L"other"))
			{
				iValue = 2;
				Result = true;
			}
			// TODO: else log
		}
	}

	return Result;
}

void BoolOption::fromString(const string& value)
{
	long long iValue;
	if (ParseIntValue(value, iValue))
	{
		Set(iValue);
	}
}

bool BoolOption::IsDefault(const FARConfigItem* Holder) const
{
	return Get() == Holder->Default.bDefault;
}

void BoolOption::SetDefault(const FARConfigItem* Holder)
{
	Set(Holder->Default.bDefault);
}

bool BoolOption::Edit(DialogBuilder* Builder, int Width, int Param)
{
	Set(!Get());
	return true;
}

void BoolOption::Export(FarSettingsItem& To)
{
	To.Type = FST_QWORD;
	To.Number = Get();
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

bool BoolOption::StoreValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, bool always) const
{
	return (!always && !Changed()) || Storage->SetValue(KeyName, ValueName, Get());
}


void Bool3Option::fromString(const string& value)
{
	long long iValue;
	if (ParseIntValue(value, iValue))
	{
		Set(iValue);
	}
}

bool Bool3Option::IsDefault(const FARConfigItem* Holder) const
{
	return Get() == Holder->Default.iDefault;
}

void Bool3Option::SetDefault(const FARConfigItem* Holder)
{
	Set(Holder->Default.iDefault);
}

bool Bool3Option::Edit(DialogBuilder* Builder, int Width, int Param)
{
	++*this;
	return true;
}

void Bool3Option::Export(FarSettingsItem& To)
{
	To.Type = FST_QWORD;
	To.Number = Get();
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

bool Bool3Option::StoreValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, bool always) const
{
	return (!always && !Changed()) || Storage->SetValue(KeyName, ValueName, Get());
}


void IntOption::fromString(const string& value)
{
	long long iValue;
	if (ParseIntValue(value, iValue))
	{
		Set(iValue);
	}
}

bool IntOption::IsDefault(const FARConfigItem* Holder) const
{
	return Get() == Holder->Default.iDefault;
}

void IntOption::SetDefault(const FARConfigItem* Holder)
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

void IntOption::Export(FarSettingsItem& To)
{
	To.Type = FST_QWORD;
	To.Number = Get();
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

bool IntOption::StoreValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, bool always) const
{
	return (!always && !Changed()) || Storage->SetValue(KeyName, ValueName, Get());
}

const string IntOption::ExInfo() const
{
	std::wostringstream oss;
	oss << L" = 0x" << std::hex << Get();
	return oss.str();
}


bool StringOption::IsDefault(const FARConfigItem* Holder) const
{
	return Get() == Holder->Default.sDefault;
}

void StringOption::SetDefault(const FARConfigItem* Holder)
{
	Set(Holder->Default.sDefault);
}

bool StringOption::Edit(DialogBuilder* Builder, int Width, int Param)
{
	Builder->AddEditField(*this, Width);
	return false;
}

void StringOption::Export(FarSettingsItem& To)
{
	To.Type = FST_STRING;
	To.String = Get().data();
}

bool StringOption::ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, const wchar_t* Default)
{
	string CfgValue = Default;
	bool Result = Storage->GetValue(KeyName, ValueName, CfgValue, CfgValue.data());
	Set(CfgValue);
	return Result;
}

bool StringOption::ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, const default_value* Default)
{
	return ReceiveValue(Storage, KeyName, ValueName, Default->sDefault);
}

bool StringOption::StoreValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, bool always) const
{
	return (!always && !Changed()) || Storage->SetValue(KeyName, ValueName, Get());
}


class Options::farconfig:NonCopyable
{
public:
	typedef FARConfigItem* iterator;
	typedef const FARConfigItem* const_iterator;
	typedef FARConfigItem value_type;

	farconfig(FARConfigItem* Items, size_t Size, GeneralConfig* cfg):
		m_items(Items),
		m_size(Size),
		m_cfg(cfg)
	{
	}

	farconfig(farconfig&& rhs):
		m_items(),
		m_size(),
		m_cfg()
	{
		*this = std::move(rhs);
	}

	MOVE_OPERATOR_BY_SWAP(farconfig);

	void swap(farconfig& rhs) noexcept
	{
		std::swap(m_items, rhs.m_items);
		std::swap(m_size, rhs.m_size);
		std::swap(m_cfg, rhs.m_cfg);
	}

	iterator begin() const { return m_items; }
	iterator end() const { return m_items + m_size; }
	const_iterator cbegin() const { return m_items; }
	const_iterator cend() const { return m_items + m_size; }
	size_t size() const { return m_size; }
	value_type& operator[](size_t i) const { return m_items[i]; }

	GeneralConfig* GetConfig() const { return m_cfg; }

private:
	value_type* m_items;
	size_t m_size;
	GeneralConfig* m_cfg;
};

STD_SWAP_SPEC(Options::farconfig);


Options::Options():
	KnownIDs(),
	ReadOnlyConfig(-1),
	UseExceptionHandler(0),
	ElevationMode(0),
	WindowMode(-1),
	ViewSettings(m_ViewSettings),
	CurrentConfig(cfg_roaming),
	m_ViewSettings(predefined_panel_modes_count),
	m_ViewSettingsChanged(false)
{
	// По умолчанию - брать плагины из основного каталога
	LoadPlug.MainPluginDir = true;
	LoadPlug.PluginsPersonal = true;
	LoadPlug.PluginsCacheOnly = false;

	Macro.DisableMacro=0;

	ResetViewModes(m_ViewSettings.data());
}

Options::~Options()
{
}

void Options::InitRoamingCFG()
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
	static_assert(ARRAYSIZE(DefaultBoxSymbols) == 48 + 1, "wrong DefaultBoxSymbols array");

	static FARConfigItem _CFG[] =
	{
		{FSSF_PRIVATE,       NKeyCmdline, L"AutoComplete", &CmdLine.AutoComplete, true},
		{FSSF_PRIVATE,       NKeyCmdline, L"EditBlock", &CmdLine.EditBlock, false},
		{FSSF_PRIVATE,       NKeyCmdline, L"DelRemovesBlocks", &CmdLine.DelRemovesBlocks, true},
		{FSSF_PRIVATE,       NKeyCmdline, L"PromptFormat", &CmdLine.strPromptFormat, L"$p$g"},
		{FSSF_PRIVATE,       NKeyCmdline, L"UsePromptFormat", &CmdLine.UsePromptFormat, false},

		{FSSF_PRIVATE,       NKeyCodePages,L"CPMenuMode", &CPMenuMode, false},
		{FSSF_PRIVATE,       NKeyCodePages,L"NoAutoDetectCP", &strNoAutoDetectCP, L""},

		{FSSF_PRIVATE,       NKeyConfirmations,L"AllowReedit", &Confirm.AllowReedit, true},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Copy", &Confirm.Copy, true},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Delete", &Confirm.Delete, true},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"DeleteFolder", &Confirm.DeleteFolder, true},
		{FSSF_PRIVATE,       NKeyConfirmations,L"DetachVHD", &Confirm.DetachVHD, true},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Drag", &Confirm.Drag, true},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Esc", &Confirm.Esc, true},
		{FSSF_PRIVATE,       NKeyConfirmations,L"EscTwiceToInterrupt", &Confirm.EscTwiceToInterrupt, false},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Exit", &Confirm.Exit, true},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"HistoryClear", &Confirm.HistoryClear, true},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"Move", &Confirm.Move, true},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"RemoveConnection", &Confirm.RemoveConnection, true},
		{FSSF_PRIVATE,       NKeyConfirmations,L"RemoveHotPlug", &Confirm.RemoveHotPlug, true},
		{FSSF_PRIVATE,       NKeyConfirmations,L"RemoveSUBST", &Confirm.RemoveSUBST, true},
		{FSSF_CONFIRMATIONS, NKeyConfirmations,L"RO", &Confirm.RO, true},

		{FSSF_PRIVATE,       NKeyDescriptions,L"AnsiByDefault", &Diz.AnsiByDefault, false},
		{FSSF_PRIVATE,       NKeyDescriptions,L"ListNames", &Diz.strListNames, L"Descript.ion,Files.bbs"},
		{FSSF_PRIVATE,       NKeyDescriptions,L"ROUpdate", &Diz.ROUpdate, false},
		{FSSF_PRIVATE,       NKeyDescriptions,L"SaveInUTF", &Diz.SaveInUTF, false},
		{FSSF_PRIVATE,       NKeyDescriptions,L"SetHidden", &Diz.SetHidden, true},
		{FSSF_PRIVATE,       NKeyDescriptions,L"StartPos", &Diz.StartPos, 0},
		{FSSF_PRIVATE,       NKeyDescriptions,L"UpdateMode", &Diz.UpdateMode, DIZ_UPDATE_IF_DISPLAYED},

		{FSSF_PRIVATE,       NKeyDialog,L"AutoComplete", &Dialogs.AutoComplete, true},
		{FSSF_PRIVATE,       NKeyDialog,L"CBoxMaxHeight", &Dialogs.CBoxMaxHeight, 8},
		{FSSF_DIALOG,        NKeyDialog,L"EditBlock", &Dialogs.EditBlock, false},
		{FSSF_PRIVATE,       NKeyDialog,L"EditHistory", &Dialogs.EditHistory, true},
		{FSSF_PRIVATE,       NKeyDialog,L"EditLine", &Dialogs.EditLine, 0},
		{FSSF_DIALOG,        NKeyDialog,L"DelRemovesBlocks", &Dialogs.DelRemovesBlocks, true},
		{FSSF_DIALOG,        NKeyDialog,L"EULBsClear", &Dialogs.EULBsClear, false},
		{FSSF_PRIVATE,       NKeyDialog,L"MouseButton", &Dialogs.MouseButton, 0xFFFF},

		{FSSF_PRIVATE,       NKeyEditor,L"AddUnicodeBOM", &EdOpt.AddUnicodeBOM, true},
		{FSSF_PRIVATE,       NKeyEditor,L"AllowEmptySpaceAfterEof", &EdOpt.AllowEmptySpaceAfterEof,false},
		{FSSF_PRIVATE,       NKeyEditor,L"AutoDetectCodePage", &EdOpt.AutoDetectCodePage, true},
		{FSSF_PRIVATE,       NKeyEditor,L"AutoIndent", &EdOpt.AutoIndent, false},
		{FSSF_PRIVATE,       NKeyEditor,L"BSLikeDel", &EdOpt.BSLikeDel, true},
		{FSSF_PRIVATE,       NKeyEditor,L"CharCodeBase", &EdOpt.CharCodeBase, 1},
		{FSSF_PRIVATE,       NKeyEditor,L"DefaultCodePage", &EdOpt.DefaultCodePage, GetACP()},
		{FSSF_PRIVATE,       NKeyEditor,L"DelRemovesBlocks", &EdOpt.DelRemovesBlocks, true},
		{FSSF_PRIVATE,       NKeyEditor,L"EditOpenedForWrite", &EdOpt.EditOpenedForWrite, true},
		{FSSF_PRIVATE,       NKeyEditor,L"EditorCursorBeyondEOL", &EdOpt.CursorBeyondEOL, true},
		{FSSF_PRIVATE,       NKeyEditor,L"ExpandTabs", &EdOpt.ExpandTabs, 0},
		{FSSF_PRIVATE,       NKeyEditor,L"ExternalEditorName", &strExternalEditor, L""},
		{FSSF_PRIVATE,       NKeyEditor,L"FileSizeLimit", &EdOpt.FileSizeLimit, 0},
		{FSSF_PRIVATE,       NKeyEditor,L"KeepEditorEOL", &EdOpt.KeepEOL, true},
		{FSSF_PRIVATE,       NKeyEditor,L"PersistentBlocks", &EdOpt.PersistentBlocks, false},
		{FSSF_PRIVATE,       NKeyEditor,L"ReadOnlyLock", &EdOpt.ReadOnlyLock, 0},
		{FSSF_PRIVATE,       NKeyEditor,L"SaveEditorPos", &EdOpt.SavePos, true},
		{FSSF_PRIVATE,       NKeyEditor,L"SaveEditorShortPos", &EdOpt.SaveShortPos, true},
		{FSSF_PRIVATE,       NKeyEditor,L"SearchPickUpWord", &EdOpt.SearchPickUpWord, false},
		{FSSF_PRIVATE,       NKeyEditor,L"SearchRegexp", &EdOpt.SearchRegexp, false},
		{FSSF_PRIVATE,       NKeyEditor,L"SearchSelFound", &EdOpt.SearchSelFound, false},
		{FSSF_PRIVATE,       NKeyEditor,L"SearchCursorAtEnd", &EdOpt.SearchCursorAtEnd, false},
		{FSSF_PRIVATE,       NKeyEditor,L"ShowKeyBar", &EdOpt.ShowKeyBar, true},
		{FSSF_PRIVATE,       NKeyEditor,L"ShowScrollBar", &EdOpt.ShowScrollBar, false},
		{FSSF_PRIVATE,       NKeyEditor,L"ShowTitleBar", &EdOpt.ShowTitleBar, true},
		{FSSF_PRIVATE,       NKeyEditor,L"ShowWhiteSpace", &EdOpt.ShowWhiteSpace, 0},
		{FSSF_PRIVATE,       NKeyEditor,L"TabSize", &EdOpt.TabSize, DefaultTabSize},
		{FSSF_PRIVATE,       NKeyEditor,L"UndoDataSize", &EdOpt.UndoSize, 100*1024*1024},
		{FSSF_PRIVATE,       NKeyEditor,L"UseExternalEditor", &EdOpt.UseExternalEditor, false},
		{FSSF_EDITOR,        NKeyEditor,L"WordDiv", &strWordDiv, WordDiv0},

		{FSSF_PRIVATE,       NKeyHelp,L"ActivateURL", &HelpURLRules, 1},
		{FSSF_PRIVATE,       NKeyHelp,L"HelpSearchRegexp", &HelpSearchRegexp, false},

		{FSSF_PRIVATE,       NKeyCommandHistory, L"Count", &HistoryCount, 1000},
		{FSSF_PRIVATE,       NKeyCommandHistory, L"Lifetime", &HistoryLifetime, 90},
		{FSSF_PRIVATE,       NKeyDialogHistory, L"Count", &DialogsHistoryCount, 1000},
		{FSSF_PRIVATE,       NKeyDialogHistory, L"Lifetime", &DialogsHistoryLifetime, 90},
		{FSSF_PRIVATE,       NKeyFolderHistory, L"Count", &FoldersHistoryCount, 1000},
		{FSSF_PRIVATE,       NKeyFolderHistory, L"Lifetime", &FoldersHistoryLifetime, 90},
		{FSSF_PRIVATE,       NKeyViewEditHistory, L"Count", &ViewHistoryCount, 1000},
		{FSSF_PRIVATE,       NKeyViewEditHistory, L"Lifetime", &ViewHistoryLifetime, 90},

		{FSSF_PRIVATE,       NKeyInterface,L"DelHighlightSelected", &DelOpt.HighlightSelected, true},
		{FSSF_PRIVATE,       NKeyInterface,L"DelShowSelected", &DelOpt.ShowSelected, 10},
		{FSSF_PRIVATE,       NKeyInterface,L"DelShowTotal", &DelOpt.ShowTotal, false},

		{FSSF_PRIVATE,       NKeyInterface, L"AltF9", &AltF9, true},
		{FSSF_PRIVATE,       NKeyInterface, L"ClearType", &ClearType, true},
		{FSSF_PRIVATE,       NKeyInterface, L"CopyShowTotal", &CMOpt.CopyShowTotal, true},
		{FSSF_PRIVATE,       NKeyInterface, L"CtrlPgUp", &PgUpChangeDisk, 1},
		{FSSF_PRIVATE,       NKeyInterface, L"CursorSize1", &CursorSize[0], 15},
		{FSSF_PRIVATE,       NKeyInterface, L"CursorSize2", &CursorSize[1], 10},
		{FSSF_PRIVATE,       NKeyInterface, L"CursorSize3", &CursorSize[2], 99},
		{FSSF_PRIVATE,       NKeyInterface, L"CursorSize4", &CursorSize[3], 99},
		{FSSF_PRIVATE,       NKeyInterface, L"EditorTitleFormat", &strEditorTitleFormat, L"%Lng %File"},
		{FSSF_PRIVATE,       NKeyInterface, L"FormatNumberSeparators", &FormatNumberSeparators, L""},
		{FSSF_PRIVATE,       NKeyInterface, L"Mouse", &Mouse, true},
		{FSSF_PRIVATE,       NKeyInterface, L"SetIcon", &SetIcon, false},
		{FSSF_PRIVATE,       NKeyInterface, L"SetAdminIcon", &SetAdminIcon, true},
		{FSSF_PRIVATE,       NKeyInterface, L"ShiftsKeyRules", &ShiftsKeyRules, true},
		{FSSF_PRIVATE,       NKeyInterface, L"ShowDotsInRoot", &ShowDotsInRoot, false},
		{FSSF_INTERFACE,     NKeyInterface, L"ShowMenuBar", &ShowMenuBar, false},
		{FSSF_PRIVATE,       NKeyInterface, L"RedrawTimeout", &RedrawTimeout, 200},
		{FSSF_PRIVATE,       NKeyInterface, L"TitleAddons", &strTitleAddons, L"%Ver.%Build %Platform %Admin"},
		{FSSF_PRIVATE,       NKeyInterface, L"UseVk_oem_x", &UseVk_oem_x, true},
		{FSSF_PRIVATE,       NKeyInterface, L"ViewerTitleFormat", &strViewerTitleFormat, L"%Lng %File"},

		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"Append", &AutoComplete.AppendCompletion, false},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"ModalList", &AutoComplete.ModalList, false},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"ShowList", &AutoComplete.ShowList, true},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"UseFilesystem", &AutoComplete.UseFilesystem, 1},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"UseHistory", &AutoComplete.UseHistory, 1},
		{FSSF_PRIVATE,       NKeyInterfaceCompletion,L"UsePath", &AutoComplete.UsePath, 1},

		{FSSF_PRIVATE,       NKeyLanguage, L"Main", &strLanguage, DefaultLanguage},
		{FSSF_PRIVATE,       NKeyLanguage, L"Help", &strHelpLanguage, DefaultLanguage},

		{FSSF_PRIVATE,       NKeyLayout,L"FullscreenHelp", &FullScreenHelp, false},
		{FSSF_PRIVATE,       NKeyLayout,L"LeftHeightDecrement", &LeftHeightDecrement, 0},
		{FSSF_PRIVATE,       NKeyLayout,L"RightHeightDecrement", &RightHeightDecrement, 0},
		{FSSF_PRIVATE,       NKeyLayout,L"WidthDecrement", &WidthDecrement, 0},

		{FSSF_PRIVATE,       NKeyKeyMacros,L"CONVFMT", &Macro.strMacroCONVFMT, L"%.6g"},
		{FSSF_PRIVATE,       NKeyKeyMacros,L"DateFormat", &Macro.strDateFormat, L"%a %b %d %H:%M:%S %Z %Y"},

		{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordCtrlDot", &Macro.strKeyMacroCtrlDot, L"Ctrl."},
		{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordRCtrlDot", &Macro.strKeyMacroRCtrlDot, L"RCtrl."},
		{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordCtrlShiftDot", &Macro.strKeyMacroCtrlShiftDot, L"CtrlShift."},
		{FSSF_PRIVATE,       NKeyKeyMacros,L"KeyRecordRCtrlShiftDot", &Macro.strKeyMacroRCtrlShiftDot, L"RCtrlShift."},

		{FSSF_PRIVATE,       NKeyKeyMacros,L"ShowPlayIndicator", &Macro.ShowPlayIndicator, true},

		{FSSF_PRIVATE,       NKeyPanel,L"AutoUpdateLimit", &AutoUpdateLimit, 0},
		{FSSF_PRIVATE,       NKeyPanel,L"CtrlAltShiftRule", &PanelCtrlAltShiftRule, 0},
		{FSSF_PRIVATE,       NKeyPanel,L"CtrlFRule", &PanelCtrlFRule, false},
		{FSSF_PRIVATE,       NKeyPanel,L"Highlight", &Highlight, true},
		{FSSF_PRIVATE,       NKeyPanel,L"ReverseSort", &ReverseSort, true},
		{FSSF_PRIVATE,       NKeyPanel,L"RememberLogicalDrives", &RememberLogicalDrives, false},
		{FSSF_PRIVATE,       NKeyPanel,L"RightClickRule", &PanelRightClickRule, 2},
		{FSSF_PRIVATE,       NKeyPanel,L"SelectFolders", &SelectFolders, false},
		{FSSF_PRIVATE,       NKeyPanel,L"ShellRightLeftArrowsRule", &ShellRightLeftArrowsRule, false},
		{FSSF_PANEL,         NKeyPanel,L"ShowHidden", &ShowHidden, true},
		{FSSF_PANEL,         NKeyPanel,L"ShortcutAlwaysChdir", &ShortcutAlwaysChdir, false},
		{FSSF_PRIVATE,       NKeyPanel,L"SortFolderExt", &SortFolderExt, false},
		{FSSF_PRIVATE,       NKeyPanel,L"RightClickSelect", &RightClickSelect, false},

		{FSSF_PRIVATE,       NKeyPanelInfo,L"InfoComputerNameFormat", &InfoPanel.ComputerNameFormat, ComputerNamePhysicalNetBIOS},
		{FSSF_PRIVATE,       NKeyPanelInfo,L"InfoUserNameFormat", &InfoPanel.UserNameFormat, NameUserPrincipal},
		{FSSF_PRIVATE,       NKeyPanelInfo,L"ShowCDInfo", &InfoPanel.ShowCDInfo, true},
		{FSSF_PRIVATE,       NKeyPanelInfo,L"ShowPowerStatus", &InfoPanel.ShowPowerStatus, false},

		{FSSF_PRIVATE,       NKeyPanelLayout,L"ColoredGlobalColumnSeparator", &HighlightColumnSeparator, true},
		{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"ColumnTitles", &ShowColumnTitles, true},
		{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"DetailedJunction", &PanelDetailedJunction, false},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"DoubleGlobalColumnSeparator", &DoubleGlobalColumnSeparator, false},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"FreeInfo", &ShowPanelFree, false},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"ScreensNumber", &ShowScreensNumber, true},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"Scrollbar", &ShowPanelScrollbar, false},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"ScrollbarMenu", &ShowMenuScrollbar, true},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"ShowUnknownReparsePoint", &ShowUnknownReparsePoint, false},
		{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"SortMode", &ShowSortMode, true},
		{FSSF_PANELLAYOUT,   NKeyPanelLayout,L"StatusLine", &ShowPanelStatus, true},
		{FSSF_PRIVATE,       NKeyPanelLayout,L"TotalInfo", &ShowPanelTotals, true},

		{FSSF_PRIVATE,       NKeyPanelLeft,L"CaseSensitiveSort", &LeftPanel.CaseSensitiveSort, false},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"DirectoriesFirst", &LeftPanel.DirectoriesFirst, true},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"NumericSort", &LeftPanel.NumericSort, false},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"SelectedFirst", &LeftPanel.SelectedFirst, false},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"ShortNames", &LeftPanel.ShowShortNames, false},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"SortGroups", &LeftPanel.SortGroups, false},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"SortMode", &LeftPanel.SortMode, 1},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"ReverseSortOrder", &LeftPanel.ReverseSortOrder, false},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"Type", &LeftPanel.m_Type, 0},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"ViewMode", &LeftPanel.ViewMode, 2},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"Visible", &LeftPanel.Visible, true},

		{FSSF_PRIVATE,       NKeyPanelRight,L"CaseSensitiveSort", &RightPanel.CaseSensitiveSort, false},
		{FSSF_PRIVATE,       NKeyPanelRight,L"DirectoriesFirst", &RightPanel.DirectoriesFirst, true},
		{FSSF_PRIVATE,       NKeyPanelRight,L"NumericSort", &RightPanel.NumericSort, false},
		{FSSF_PRIVATE,       NKeyPanelRight,L"SelectedFirst", &RightPanel.SelectedFirst, false},
		{FSSF_PRIVATE,       NKeyPanelRight,L"ShortNames", &RightPanel.ShowShortNames, false},
		{FSSF_PRIVATE,       NKeyPanelRight,L"SortGroups", &RightPanel.SortGroups, false},
		{FSSF_PRIVATE,       NKeyPanelRight,L"SortMode", &RightPanel.SortMode, 1},
		{FSSF_PRIVATE,       NKeyPanelRight,L"ReverseSortOrder", &RightPanel.ReverseSortOrder, false},
		{FSSF_PRIVATE,       NKeyPanelRight,L"Type", &RightPanel.m_Type, 0},
		{FSSF_PRIVATE,       NKeyPanelRight,L"ViewMode", &RightPanel.ViewMode, 2},
		{FSSF_PRIVATE,       NKeyPanelRight,L"Visible", &RightPanel.Visible, true},

		{FSSF_PRIVATE,       NKeyPanelTree,L"AutoChangeFolder", &Tree.AutoChangeFolder, false},
		{FSSF_PRIVATE,       NKeyPanelTree,L"MinTreeCount", &Tree.MinTreeCount, 4},
		{FSSF_PRIVATE,       NKeyPanelTree,L"TreeFileAttr", &Tree.TreeFileAttr, FILE_ATTRIBUTE_HIDDEN},
	#if defined(TREEFILE_PROJECT)
		{FSSF_PRIVATE,       NKeyPanelTree,L"CDDisk", &Tree.CDDisk, 2},
		{FSSF_PRIVATE,       NKeyPanelTree,L"CDDiskTemplate,0", &Tree.strCDDisk, constCDDiskTemplate},
		{FSSF_PRIVATE,       NKeyPanelTree,L"ExceptPath", &Tree.strExceptPath, L""},
		{FSSF_PRIVATE,       NKeyPanelTree,L"LocalDisk", &Tree.LocalDisk, 2},
		{FSSF_PRIVATE,       NKeyPanelTree,L"LocalDiskTemplate", &Tree.strLocalDisk, constLocalDiskTemplate},
		{FSSF_PRIVATE,       NKeyPanelTree,L"NetDisk", &Tree.NetDisk, 2},
		{FSSF_PRIVATE,       NKeyPanelTree,L"NetPath", &Tree.NetPath, 2},
		{FSSF_PRIVATE,       NKeyPanelTree,L"NetDiskTemplate", &Tree.strNetDisk, constNetDiskTemplate},
		{FSSF_PRIVATE,       NKeyPanelTree,L"NetPathTemplate", &Tree.strNetPath, constNetPathTemplate},
		{FSSF_PRIVATE,       NKeyPanelTree,L"RemovableDisk", &Tree.RemovableDisk, 2},
		{FSSF_PRIVATE,       NKeyPanelTree,L"RemovableDiskTemplate,", &Tree.strRemovableDisk, constRemovableDiskTemplate},
		{FSSF_PRIVATE,       NKeyPanelTree,L"SaveLocalPath", &Tree.strSaveLocalPath, L""},
		{FSSF_PRIVATE,       NKeyPanelTree,L"SaveNetPath", &Tree.strSaveNetPath, L""},
	#endif
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"EvenIfOnlyOnePlugin", &PluginConfirm.EvenIfOnlyOnePlugin, false},
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"OpenFilePlugin", &PluginConfirm.OpenFilePlugin, 0},
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"Prefix", &PluginConfirm.Prefix, false},
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"SetFindList", &PluginConfirm.SetFindList, false},
		{FSSF_PRIVATE,       NKeyPluginConfirmations, L"StandardAssociation", &PluginConfirm.StandardAssociation, false},

		{FSSF_PRIVATE,       NKeyPolicies,L"ShowHiddenDrives", &Policies.ShowHiddenDrives, true},

		{FSSF_PRIVATE,       NKeyScreen, L"Clock", &Clock, true},
		{FSSF_PRIVATE,       NKeyScreen, L"DeltaX", &ScrSize.DeltaX, 0},
		{FSSF_PRIVATE,       NKeyScreen, L"DeltaY", &ScrSize.DeltaY, 0},
		{FSSF_SCREEN,        NKeyScreen, L"KeyBar", &ShowKeyBar, true},
		{FSSF_PRIVATE,       NKeyScreen, L"ScreenSaver", &ScreenSaver, false},
		{FSSF_PRIVATE,       NKeyScreen, L"ScreenSaverTime", &ScreenSaverTime, 5},
		{FSSF_PRIVATE,       NKeyScreen, L"ViewerEditorClock", &ViewerEditorClock, true},

		{FSSF_PRIVATE,       NKeySystem,L"AllCtrlAltShiftRule", &AllCtrlAltShiftRule, 0x0000FFFF},
		{FSSF_PRIVATE,       NKeySystem,L"AutoSaveSetup", &AutoSaveSetup, false},
		{FSSF_PRIVATE,       NKeySystem,L"AutoUpdateRemoteDrive", &AutoUpdateRemoteDrive, true},
		{FSSF_PRIVATE,       NKeySystem,L"BoxSymbols", &strBoxSymbols, DefaultBoxSymbols},
		{FSSF_PRIVATE,       NKeySystem,L"CASRule", &CASRule, -1},
		{FSSF_PRIVATE,       NKeySystem,L"CloseCDGate", &CloseCDGate, true},
		{FSSF_PRIVATE,       NKeySystem,L"CmdHistoryRule", &CmdHistoryRule, false},
		{FSSF_PRIVATE,       NKeySystem,L"ConsoleDetachKey", &ConsoleDetachKey, L"CtrlShiftTab"},
		{FSSF_PRIVATE,       NKeySystem,L"CopyBufferSize", &CMOpt.BufferSize, 0},
		{FSSF_SYSTEM,        NKeySystem,L"CopyOpened", &CMOpt.CopyOpened, true},
		{FSSF_PRIVATE,       NKeySystem,L"CopyTimeRule",  &CMOpt.CopyTimeRule, 3},
		{FSSF_PRIVATE,       NKeySystem,L"CopySecurityOptions", &CMOpt.CopySecurityOptions, 0},
		{FSSF_PRIVATE,       NKeySystem,L"CreateUppercaseFolders", &CreateUppercaseFolders, false},
		{FSSF_SYSTEM,        NKeySystem,L"DeleteToRecycleBin", &DeleteToRecycleBin, true},
		{FSSF_PRIVATE,       NKeySystem,L"DeleteToRecycleBinKillLink", &DeleteToRecycleBinKillLink, true},
		{FSSF_PRIVATE,       NKeySystem,L"DelThreadPriority", &DelThreadPriority, THREAD_PRIORITY_NORMAL},
		{FSSF_PRIVATE,       NKeySystem,L"DriveDisconnectMode", &ChangeDriveDisconnectMode, true},
		{FSSF_PRIVATE,       NKeySystem,L"DriveMenuMode", &ChangeDriveMode, DRIVE_SHOW_TYPE|DRIVE_SHOW_PLUGINS|DRIVE_SHOW_SIZE_FLOAT|DRIVE_SHOW_CDROM},
		{FSSF_PRIVATE,       NKeySystem,L"ElevationMode", &StoredElevationMode, -1},
		{FSSF_PRIVATE,       NKeySystem,L"ExcludeCmdHistory", &ExcludeCmdHistory, 0},
		{FSSF_PRIVATE,       NKeySystem,L"FileSearchMode", &FindOpt.FileSearchMode, FINDAREA_FROM_CURRENT},
		{FSSF_PRIVATE,       NKeySystem,L"FindAlternateStreams", &FindOpt.FindAlternateStreams,0,},
		{FSSF_PRIVATE,       NKeySystem,L"FindCodePage", &FindCodePage, CP_DEFAULT},
		{FSSF_PRIVATE,       NKeySystem,L"FindFolders", &FindOpt.FindFolders, true},
		{FSSF_PRIVATE,       NKeySystem,L"FindSymLinks", &FindOpt.FindSymLinks, true},
		{FSSF_PRIVATE,       NKeySystem,L"FlagPosixSemantics", &FlagPosixSemantics, true},
		{FSSF_PRIVATE,       NKeySystem,L"FolderInfo", &InfoPanel.strFolderInfoFiles, L"DirInfo,File_Id.diz,Descript.ion,ReadMe.*,Read.Me"},
		{FSSF_PRIVATE,       NKeySystem,L"MsWheelDelta", &MsWheelDelta, 1},
		{FSSF_PRIVATE,       NKeySystem,L"MsWheelDeltaEdit", &MsWheelDeltaEdit, 1},
		{FSSF_PRIVATE,       NKeySystem,L"MsWheelDeltaHelp", &MsWheelDeltaHelp, 1},
		{FSSF_PRIVATE,       NKeySystem,L"MsWheelDeltaView", &MsWheelDeltaView, 1},
		{FSSF_PRIVATE,       NKeySystem,L"MsHWheelDelta", &MsHWheelDelta, 1},
		{FSSF_PRIVATE,       NKeySystem,L"MsHWheelDeltaEdit", &MsHWheelDeltaEdit, 1},
		{FSSF_PRIVATE,       NKeySystem,L"MsHWheelDeltaView", &MsHWheelDeltaView, 1},
		{FSSF_PRIVATE,       NKeySystem,L"MultiCopy", &CMOpt.MultiCopy, false},
		{FSSF_PRIVATE,       NKeySystem,L"MultiMakeDir", &MultiMakeDir, false},
	#ifndef NO_WRAPPER
		{FSSF_PRIVATE,       NKeySystem,L"OEMPluginsSupport",  &LoadPlug.OEMPluginsSupport, true},
	#endif // NO_WRAPPER
		{FSSF_SYSTEM,        NKeySystem,L"PluginMaxReadData", &PluginMaxReadData, 0x20000},
		{FSSF_PRIVATE,       NKeySystem,L"QuotedName", &QuotedName, QUOTEDNAME_INSERT},
		{FSSF_PRIVATE,       NKeySystem,L"QuotedSymbols", &strQuotedSymbols, L" &()[]{}^=;!'+,`\xA0"},
		{FSSF_PRIVATE,       NKeySystem,L"SaveHistory", &SaveHistory, true},
		{FSSF_PRIVATE,       NKeySystem,L"SaveFoldersHistory", &SaveFoldersHistory, true},
		{FSSF_PRIVATE,       NKeySystem,L"SaveViewHistory", &SaveViewHistory, true},
		{FSSF_SYSTEM,        NKeySystem,L"ScanJunction", &ScanJunction, true},
		{FSSF_PRIVATE,       NKeySystem,L"ScanSymlinks",  &LoadPlug.ScanSymlinks, true},
		{FSSF_PRIVATE,       NKeySystem,L"SearchInFirstSize", &FindOpt.strSearchInFirstSize, L""},
		{FSSF_PRIVATE,       NKeySystem,L"SearchOutFormat", &FindOpt.strSearchOutFormat, L"D,S,A"},
		{FSSF_PRIVATE,       NKeySystem,L"SearchOutFormatWidth", &FindOpt.strSearchOutFormatWidth, L"14,13,0"},
		{FSSF_PRIVATE,       NKeySystem,L"SetAttrFolderRules", &SetAttrFolderRules, true},
		{FSSF_PRIVATE,       NKeySystem,L"ShowCheckingFile", &ShowCheckingFile, false},
		{FSSF_PRIVATE,       NKeySystem,L"ShowStatusInfo", &InfoPanel.strShowStatusInfo, L""},
		{FSSF_PRIVATE,       NKeySystem,L"SilentLoadPlugin",  &LoadPlug.SilentLoadPlugin, false},
		{FSSF_PRIVATE,       NKeySystem,L"SmartFolderMonitor",  &SmartFolderMonitor, false},
		{FSSF_PRIVATE,       NKeySystem,L"SubstNameRule", &SubstNameRule, 2},
		{FSSF_PRIVATE,       NKeySystem,L"SubstPluginPrefix", &SubstPluginPrefix, false},
		{FSSF_PRIVATE,       NKeySystem,L"UpdateEnvironment", &UpdateEnvironment,0,},
		{FSSF_PRIVATE,       NKeySystem,L"UseFilterInSearch", &FindOpt.UseFilter,0,},
		{FSSF_PRIVATE,       NKeySystem,L"UseRegisteredTypes", &UseRegisteredTypes, true},
		{FSSF_PRIVATE,       NKeySystem,L"UseSystemCopy", &CMOpt.UseSystemCopy, true},
		{FSSF_PRIVATE,       NKeySystem,L"WindowMode", &StoredWindowMode, false},
		{FSSF_PRIVATE,       NKeySystem,L"WipeSymbol", &WipeSymbol, 0},

		{FSSF_PRIVATE,       NKeySystemKnownIDs, L"EMenu", &KnownIDs.Emenu.StrId, KnownIDs.Emenu.Default},
		{FSSF_PRIVATE,       NKeySystemKnownIDs, L"Network", &KnownIDs.Network.StrId, KnownIDs.Network.Default},
		{FSSF_PRIVATE,       NKeySystemKnownIDs, L"Arclite", &KnownIDs.Arclite.StrId, KnownIDs.Arclite.Default},
		{FSSF_PRIVATE,       NKeySystemKnownIDs, L"Luamacro", &KnownIDs.Luamacro.StrId, KnownIDs.Luamacro.Default},
		{FSSF_PRIVATE,       NKeySystemKnownIDs, L"Netbox", &KnownIDs.Netbox.StrId, KnownIDs.Netbox.Default},

		{FSSF_PRIVATE,       NKeySystemNowell,L"MoveRO", &Nowell.MoveRO, true},

		{FSSF_PRIVATE,       NKeySystemException,L"FarEventSvc", &strExceptEventSvc, L""},
		{FSSF_PRIVATE,       NKeySystemException,L"Used", &ExceptUsed, false},

		{FSSF_PRIVATE,       NKeySystemExecutor,L"~", &Exec.strHomeDir, L"%FARHOME%"},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"BatchType", &Exec.strExecuteBatchType, constBatchExt},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"ExcludeCmds", &Exec.strExcludeCmds, L""},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"FullTitle", &Exec.ExecuteFullTitle, false},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"RestoreCP", &Exec.RestoreCPAfterExecute, true},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"SilentExternal", &Exec.ExecuteSilentExternal, false},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"UseAppPath", &Exec.ExecuteUseAppPath, true},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"UseHomeDir", &Exec.UseHomeDir, true},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"NotQuotedShell", &Exec.strNotQuotedShell, L"TCC.EXE;TCCLE.EXE"},
		{FSSF_PRIVATE,       NKeySystemExecutor,L"ComSpecParams", &Exec.strComSpecParams, L"/C"},

		{FSSF_PRIVATE,       NKeyViewer,L"AutoDetectCodePage", &ViOpt.AutoDetectCodePage, true},
		{FSSF_PRIVATE,       NKeyViewer,L"DefaultCodePage", &ViOpt.DefaultCodePage, GetACP()},
		{FSSF_PRIVATE,       NKeyViewer,L"ExternalViewerName", &strExternalViewer, L""},
		{FSSF_PRIVATE,       NKeyViewer,L"IsWrap", &ViOpt.ViewerIsWrap, true},
		{FSSF_PRIVATE,       NKeyViewer,L"MaxLineSize", &ViOpt.MaxLineSize, ViewerOptions::eDefLineSize},
		{FSSF_PRIVATE,       NKeyViewer,L"PersistentBlocks", &ViOpt.PersistentBlocks, false},
		{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerCodepage", &ViOpt.SaveCodepage, true},
		{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerPos", &ViOpt.SavePos, true},
		{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerShortPos", &ViOpt.SaveShortPos, true},
		{FSSF_PRIVATE,       NKeyViewer,L"SaveViewerWrapMode", &ViOpt.SaveWrapMode, false},
		{FSSF_PRIVATE,       NKeyViewer,L"SearchEditFocus", &ViOpt.SearchEditFocus, false},
		{FSSF_PRIVATE,       NKeyViewer,L"SearchRegexp", &ViOpt.SearchRegexp, false},
		{FSSF_PRIVATE,       NKeyViewer,L"SearchWrapStop", &ViOpt.SearchWrapStop, 1}, // Bool3 -- True
		{FSSF_PRIVATE,       NKeyViewer,L"ShowArrows", &ViOpt.ShowArrows, true},
		{FSSF_PRIVATE,       NKeyViewer,L"ShowKeyBar", &ViOpt.ShowKeyBar, true},
		{FSSF_PRIVATE,       NKeyViewer,L"ShowScrollbar", &ViOpt.ShowScrollbar, false},
		{FSSF_PRIVATE,       NKeyViewer,L"TabSize", &ViOpt.TabSize, DefaultTabSize},
		{FSSF_PRIVATE,       NKeyViewer,L"ShowTitleBar", &ViOpt.ShowTitleBar, true},
		{FSSF_PRIVATE,       NKeyViewer,L"UseExternalViewer", &ViOpt.UseExternalViewer, false},
		{FSSF_PRIVATE,       NKeyViewer,L"Visible0x00", &ViOpt.Visible0x00, false},
		{FSSF_PRIVATE,       NKeyViewer,L"Wrap", &ViOpt.ViewerWrap, false},
		{FSSF_PRIVATE,       NKeyViewer,L"ZeroChar", &ViOpt.ZeroChar, 0x00B7}, // middle dot

		{FSSF_PRIVATE,       NKeyVMenu,L"LBtnClick", &VMenu.LBtnClick, VMENUCLICK_CANCEL},
		{FSSF_PRIVATE,       NKeyVMenu,L"MBtnClick", &VMenu.MBtnClick, VMENUCLICK_APPLY},
		{FSSF_PRIVATE,       NKeyVMenu,L"RBtnClick", &VMenu.RBtnClick, VMENUCLICK_CANCEL},

		{FSSF_PRIVATE,       NKeyXLat,L"Flags", &XLat.Flags, XLAT_SWITCHKEYBLAYOUT|XLAT_CONVERTALLCMDLINE},
		{FSSF_PRIVATE,       NKeyXLat,L"Layouts", &XLat.strLayouts, L""},
		{FSSF_PRIVATE,       NKeyXLat,L"Rules1", &XLat.Rules[0], L""},
		{FSSF_PRIVATE,       NKeyXLat,L"Rules2", &XLat.Rules[1], L""},
		{FSSF_PRIVATE,       NKeyXLat,L"Rules3", &XLat.Rules[2], L""},
		{FSSF_PRIVATE,       NKeyXLat,L"Table1", &XLat.Table[0], L""},
		{FSSF_PRIVATE,       NKeyXLat,L"Table2", &XLat.Table[1], L""},
		{FSSF_PRIVATE,       NKeyXLat,L"WordDivForXlat", &XLat.strWordDivForXlat, WordDivForXlat0},
	};

	assert(Config.empty());
	Config.emplace_back(farconfig(_CFG, ARRAYSIZE(_CFG), Global->Db->GeneralCfg()));
}

void Options::InitLocalCFG()
{
	static FARConfigItem _CFG[] =
	{
		{FSSF_PRIVATE,       NKeyPanelLeft,L"CurFile", &LeftPanel.CurFile, L""},
		{FSSF_PRIVATE,       NKeyPanelLeft,L"Folder", &LeftPanel.Folder, L""},

		{FSSF_PRIVATE,       NKeyPanelRight,L"CurFile", &RightPanel.CurFile, L""},
		{FSSF_PRIVATE,       NKeyPanelRight,L"Folder", &RightPanel.Folder, L""},

		{FSSF_PRIVATE,       NKeyPanel,L"LeftFocus", &LeftFocus, true},

	};

	assert(Config.size() == 1);
	Config.emplace_back(farconfig(_CFG, ARRAYSIZE(_CFG), Global->Db->LocalGeneralCfg()));
}

bool Options::GetConfigValue(const wchar_t *Key, const wchar_t *Name, Option*& Data)
{
	// TODO Use local too?
	bool Result = false;
	auto ItemIterator = std::find_if(CONST_RANGE(Config[cfg_roaming], i) { return !StrCmpI(i.KeyName, Key) && !StrCmpI(i.ValName, Name); });
	if (ItemIterator != Config[cfg_roaming].cend())
	{
		Data = ItemIterator->Value;
		Result = true;
	}
	return Result;
}

bool Options::GetConfigValue(size_t Root, const wchar_t* Name, Option*& Data)
{
	// TODO Use local too?
	bool Result = false;
	if (Root != FSSF_PRIVATE)
	{
		auto ItemIterator = std::find_if(CONST_RANGE(Config[cfg_roaming], i) { return Root == i.ApiRoot && !StrCmpI(i.ValName, Name); });
		if (ItemIterator != Config[cfg_roaming].cend())
		{
			Data = ItemIterator->Value;
			Result = true;
		}
	}
	return Result;
}

void Options::InitConfig()
{
	if(Config.empty())
	{
		InitRoamingCFG();
		InitLocalCFG();
	}
}

void Options::Load(const std::vector<std::pair<string, string>>& Overridden)
{
	// KnownModulesIDs::GuidOption::Default pointer is used in the static config structure, so it MUST be initialized before calling InitConfig()
	static simple_pair<GUID, string> DefaultKnownGuids[] = { { NetworkGuid }, { EMenuGuid }, { ArcliteGuid }, { LuamacroGuid }, { NetBoxGuid } };
	static_assert(ARRAYSIZE(DefaultKnownGuids) == sizeof(Options::KnownModulesIDs) / sizeof(Options::KnownModulesIDs::GuidOption), "incomplete DefaultKnownGuids array");

	KnownModulesIDs::GuidOption* GuidOptions[] = { &KnownIDs.Network, &KnownIDs.Emenu, &KnownIDs.Arclite, &KnownIDs.Luamacro, &KnownIDs.Netbox };
	static_assert(ARRAYSIZE(GuidOptions) == ARRAYSIZE(DefaultKnownGuids), "incomplete GuidOptions array");

	for_each_2(ALL_RANGE(DefaultKnownGuids), GuidOptions, [](VALUE_TYPE(DefaultKnownGuids)& a, VALUE_TYPE(GuidOptions)& b)
	{
		a.second = GuidToStr(a.first);
		b->Default = a.second.data();
		b->Id = a.first;
		b->StrId = a.second;
	});


	InitConfig();

	/* <ПРЕПРОЦЕССЫ> *************************************************** */

	/* BUGBUG??
	SetRegRootKey(HKEY_LOCAL_MACHINE);
	DWORD OptPolicies_ShowHiddenDrives=GetRegKey(NKeyPolicies,L"ShowHiddenDrives",1)&1;
	SetRegRootKey(HKEY_CURRENT_USER);
	*/
	/* *************************************************** </ПРЕПРОЦЕССЫ> */

	// BUGBUG
	string strDefaultLanguage = GetFarIniString(L"General", L"DefaultLanguage", L"English");
	xwcsncpy(DefaultLanguage, strDefaultLanguage.data(), ARRAYSIZE(DefaultLanguage));

	FOR(auto& i, Config)
	{
		auto Cfg = i.GetConfig();
		FOR(auto& j, i)
		{
			j.Value->ReceiveValue(Cfg, j.KeyName, j.ValName, &j.Default);

			FOR(const auto& ovr, Overridden)
			{
				auto DotPos = ovr.first.rfind(L'.');
				if (DotPos != string::npos && !StrCmpNI(ovr.first.data(), j.KeyName, DotPos) && !StrCmpI(ovr.first.data() + DotPos + 1, j.ValName))
				{
					j.Value->fromString(ovr.second);
				}
			}
		}
	}

	/* <ПОСТПРОЦЕССЫ> *************************************************** */

	Palette.Load();
	GlobalUserMenuDir = GetFarIniString(L"General", L"GlobalUserMenuDir", Global->g_strFarPath);
	GlobalUserMenuDir = api::env::expand_strings(GlobalUserMenuDir);
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
	if (strWordDiv.empty())
		strWordDiv = WordDiv0;

	// Исключаем случайное стирание разделителей
	if (XLat.strWordDivForXlat.empty())
		XLat.strWordDivForXlat = WordDivForXlat0;

	PanelRightClickRule%=3;
	PanelCtrlAltShiftRule%=3;

	if (EdOpt.TabSize<1 || EdOpt.TabSize>512)
		EdOpt.TabSize = DefaultTabSize;

	if (ViOpt.TabSize<1 || ViOpt.TabSize>512)
		ViOpt.TabSize = DefaultTabSize;

	HelpTabSize = DefaultTabSize; // пока жестко пропишем...

	static const struct
	{
		StringOption* ConfigPtr;
		DWORD* RuntimePtr;
		DWORD Default;
	}
	MacroControlKeys[] =
	{
		{ &Macro.strKeyMacroCtrlDot, &Macro.KeyMacroCtrlDot, KEY_CTRLDOT },
		{ &Macro.strKeyMacroRCtrlDot, &Macro.KeyMacroRCtrlDot, KEY_RCTRLDOT },
		{ &Macro.strKeyMacroCtrlShiftDot, &Macro.KeyMacroCtrlShiftDot, KEY_CTRLSHIFTDOT },
		{ &Macro.strKeyMacroRCtrlShiftDot, &Macro.KeyMacroRCtrlShiftDot, KEY_RCTRL | KEY_SHIFT | KEY_DOT },
	};

	FOR(const auto& i, MacroControlKeys)
	{
		if ((*i.RuntimePtr = KeyNameToKey(*i.ConfigPtr)) == static_cast<DWORD>(-1))
			*i.RuntimePtr = i.Default;
	}

	EdOpt.strWordDiv = strWordDiv;
	ReadPanelModes();

	/* BUGBUG??
	// уточняем системную политику
	// для дисков юзер может только отменять показ
	Policies.ShowHiddenDrives&=OptPolicies_ShowHiddenDrives;
	*/

	if (Exec.strExecuteBatchType.empty()) // предохраняемся
		Exec.strExecuteBatchType=constBatchExt;

	// Инициализация XLat для русской раскладки qwerty<->йцукен
	if (std::any_of(ALL_CONST_RANGE(XLat.Table), std::mem_fn(&StringOption::empty)) ||
		std::any_of(ALL_CONST_RANGE(XLat.Rules), std::mem_fn(&StringOption::empty)))
	{
		int Count = GetKeyboardLayoutList(0, nullptr);
		if (Count)
		{
			std::vector<HKL> Layouts(Count);
			if (GetKeyboardLayoutList(static_cast<int>(Layouts.size()), Layouts.data()))
			{
				const WORD RussianLanguageId = MAKELANGID(LANG_RUSSIAN, SUBLANG_RUSSIAN_RUSSIA);
				if (std::any_of(CONST_RANGE(Layouts, i){ return LOWORD(i) == RussianLanguageId; }))
				{
					static const wchar_t* Tables[] =
					{
						L"\x2116\x0410\x0412\x0413\x0414\x0415\x0417\x0418\x0419\x041a\x041b\x041c\x041d\x041e\x041f\x0420\x0421\x0422\x0423\x0424\x0425\x0426\x0427\x0428\x0429\x042a\x042b\x042c\x042f\x0430\x0432\x0433\x0434\x0435\x0437\x0438\x0439\x043a\x043b\x043c\x043d\x043e\x043f\x0440\x0441\x0442\x0443\x0444\x0445\x0446\x0447\x0448\x0449\x044a\x044b\x044c\x044d\x044f\x0451\x0401\x0411\x042e",
						L"#FDULTPBQRKVYJGHCNEA{WXIO}SMZfdultpbqrkvyjghcnea[wxio]sm'z`~<>",
					};

					static const wchar_t* Rules[] =
					{
						L",??&./\x0431,\x044e.:^\x0416:\x0436;;$\"@\x042d\"",
						L"?,&?/.,\x0431.\x044e^::\x0416;\x0436$;@\"\"\x042d",
						L"^::\x0416\x0416^$;;\x0436\x0436$@\"\"\x042d\x042d@&??,,\x0431\x0431&/..\x044e\x044e/",
					};

					auto SetIfEmpty = [](StringOption& opt, const wchar_t* table) { if (opt.empty()) opt = table; };

					for_each_2(ALL_RANGE(XLat.Table), Tables, SetIfEmpty);
					for_each_2(ALL_RANGE(XLat.Rules), Rules, SetIfEmpty);
				}
			}
		}

	}

	{
		if (!XLat.strLayouts.empty())
		{
			size_t I=0;
			FOR(const auto& i, split_to_vector::get(XLat.strLayouts, STLF_UNIQUE))
			{
				DWORD res = std::stoul(i, nullptr, 16);
				XLat.Layouts[I]=(HKL)(intptr_t)(HIWORD(res)? res : MAKELONG(res,res));
				++I;

				if (I >= ARRAYSIZE(XLat.Layouts))
					break;
			}

			if (I <= 1) // если указано меньше двух - "откключаем" эту
				XLat.Layouts[0]=0;
		}
	}

	FindOpt.OutColumns.clear();

	if (!FindOpt.strSearchOutFormat.empty())
	{
		if (FindOpt.strSearchOutFormatWidth.empty())
			FindOpt.strSearchOutFormatWidth=L"0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0";
		TextToViewSettings(FindOpt.strSearchOutFormat, FindOpt.strSearchOutFormatWidth, FindOpt.OutColumns);
	}

	string tmp[2];
	if (!Global->Db->GeneralCfg()->EnumValues(L"Masks", 0, tmp[0], tmp[1]))
	{
		ApplyDefaultMaskGroups();
	}

	FOR(auto& i, GuidOptions)
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

	if (!CMOpt.BufferSize)
	{
		CMOpt.BufferSize = default_copy_buffer_size;
	}

/* *************************************************** </ПОСТПРОЦЕССЫ> */

	// we assume that any changes after this point will be made by the user
	std::for_each(RANGE(Config, i)
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

	if (Manual && Message(0,2,MSG(MSaveSetupTitle),MSG(MSaveSetupAsk1),MSG(MSaveSetupAsk2),MSG(MSaveSetup),MSG(MCancel)))
		return;

	/* <ПРЕПРОЦЕССЫ> *************************************************** */

	auto StorePanelOptions = [](Panel* PanelPtr, PanelOptions& Panel)
	{
		if (PanelPtr->GetMode()==NORMAL_PANEL)
		{
			Panel.m_Type = PanelPtr->GetType();
			Panel.ViewMode = PanelPtr->GetViewMode();
			Panel.SortMode = PanelPtr->GetSortMode();
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

	StorePanelOptions(Global->CtrlObject->Cp()->LeftPanel, LeftPanel);
	StorePanelOptions(Global->CtrlObject->Cp()->RightPanel, RightPanel);

	LeftFocus = Global->CtrlObject->Cp()->ActivePanel() == Global->CtrlObject->Cp()->LeftPanel;

	/* *************************************************** </ПРЕПРОЦЕССЫ> */

	Global->CtrlObject->HiFiles->Save(Manual);

	Palette.Save(Manual);

	std::for_each(CONST_RANGE(Config, i)
	{
		auto Cfg = i.GetConfig();
		SCOPED_ACTION(auto) = Cfg->ScopedTransaction();
		std::for_each(CONST_RANGE(i, j)
		{
			j.Value->StoreValue(Cfg, j.KeyName, j.ValName, Manual);
		});
	});

	FileFilter::Save(Manual);
	SavePanelModes(Manual);
	Global->CtrlObject->Macro.SaveMacros(Manual);
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
			auto record= reinterpret_cast<const INPUT_RECORD*>(Param2);
			if (record->EventType==KEY_EVENT)
			{
				int key = InputRecordToKey(record);
				switch(key)
				{
				case KEY_SHIFTF1:
					{
						FarListInfo ListInfo = {sizeof(ListInfo)};
						Dlg->SendMessage(DM_LISTINFO, Param1, &ListInfo);

						string HelpTopic = string(Config[CurrentConfig][ListInfo.SelectPos].KeyName) + L"." + Config[CurrentConfig][ListInfo.SelectPos].ValName;
						Help hlp(HelpTopic, nullptr, FHELP_NOSHOWERROR);
						if (hlp.GetError())
						{
							HelpTopic = string(Config[CurrentConfig][ListInfo.SelectPos].KeyName) + L"Settings";
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

			if (Config[CurrentConfig][ListInfo.SelectPos].Edit(Param1 != 0))
			{
				Dlg->SendMessage(DM_ENABLEREDRAW, 0 , 0);
				FarListUpdate flu = {sizeof(flu), ListInfo.SelectPos};
				flu.Item = Config[CurrentConfig][ListInfo.SelectPos].MakeListItem();
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
	auto AdvancedConfigDlg = MakeDialogItemsEx(AdvancedConfigDlgData);

	std::vector<FarListItem> items(Config[CurrentConfig].size());
	std::transform(ALL_RANGE(Config[CurrentConfig]), items.begin(), std::mem_fn(&FARConfigItem::MakeListItem));

	FarList Items={sizeof(FarList), items.size(), items.data()};

	AdvancedConfigDlg[0].ListItems = &Items;

	Dialog Dlg(AdvancedConfigDlg, this, &Options::AdvancedConfigDlgProc);
	Dlg.SetHelp(L"FarConfig");
	Dlg.SetPosition(-1, -1, DlgWidth, DlgHeight);
	Dlg.SetId(AdvancedConfigId);
	Dlg.Process();
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

static const wchar_t *CustomModesKeyName = L"CustomModes";
static const wchar_t *ModesNameName = L"Name";
static const wchar_t *ModesColumnTitlesName = L"ColumnTitles";
static const wchar_t *ModesColumnWidthsName = L"ColumnWidths";
static const wchar_t *ModesStatusColumnTitlesName = L"StatusColumnTitles";
static const wchar_t *ModesStatusColumnWidthsName = L"StatusColumnWidths";
static const wchar_t *ModesFlagsName = L"Flags";

void Options::ReadPanelModes()
{
	auto cfg = Global->Db->CreatePanelModeConfig();

	unsigned __int64 root = 0;

	auto ReadMode = [&](VALUE_TYPE(m_ViewSettings)& i, size_t Index) -> bool
	{
		unsigned __int64 id = cfg->GetKeyID(root, std::to_wstring(Index));

		if (!id)
		{
			return false;
		}
		string strColumnTitles, strColumnWidths;
		cfg->GetValue(id, ModesColumnTitlesName, strColumnTitles);
		cfg->GetValue(id, ModesColumnWidthsName, strColumnWidths);

		string strStatusColumnTitles, strStatusColumnWidths;
		cfg->GetValue(id, ModesStatusColumnTitlesName, strStatusColumnTitles);
		cfg->GetValue(id, ModesStatusColumnWidthsName, strStatusColumnWidths);

		unsigned __int64 Flags=0;
		cfg->GetValue(id, ModesFlagsName, &Flags);

		cfg->GetValue(id, ModesNameName, i.Name);

		if (!strColumnTitles.empty())
			TextToViewSettings(strColumnTitles, strColumnWidths, i.PanelColumns);

		if (!strStatusColumnTitles.empty())
			TextToViewSettings(strStatusColumnTitles, strStatusColumnWidths, i.StatusColumns);

			i.Flags = Flags;

		return true;
	};

	for_each_cnt(m_ViewSettings.begin(), m_ViewSettings.begin() + predefined_panel_modes_count, ReadMode);

	root = cfg->GetKeyID(0, CustomModesKeyName);

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

	auto cfg = Global->Db->CreatePanelModeConfig();
	unsigned __int64 root = 0;

	auto SaveMode = [&](const VALUE_TYPE(ViewSettings)& i, size_t Index)
	{
		string strColumnTitles, strColumnWidths;
		string strStatusColumnTitles, strStatusColumnWidths;

		ViewSettingsToText(i.PanelColumns, strColumnTitles, strColumnWidths);
		ViewSettingsToText(i.StatusColumns, strStatusColumnTitles, strStatusColumnWidths);

		unsigned __int64 id = cfg->CreateKey(root, std::to_wstring(Index));
		if (id)
		{
			cfg->SetValue(id, ModesNameName, i.Name);
			cfg->SetValue(id, ModesColumnTitlesName, strColumnTitles);
			cfg->SetValue(id, ModesColumnWidthsName, strColumnWidths);
			cfg->SetValue(id, ModesStatusColumnTitlesName, strStatusColumnTitles);
			cfg->SetValue(id, ModesStatusColumnWidthsName, strStatusColumnWidths);
			cfg->SetValue(id, ModesFlagsName, i.Flags);
		}
	};

	for_each_cnt(ViewSettings.cbegin(), ViewSettings.cbegin() + predefined_panel_modes_count, SaveMode);

	root = cfg->GetKeyID(0, CustomModesKeyName);
	if (root)
	{
		cfg->DeleteKeyTree(root);
	}
	root = cfg->CreateKey(0, CustomModesKeyName);
	if (root)
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
	Panel *pPanel = bLeft?Global->CtrlObject->Cp()->LeftPanel:Global->CtrlObject->Cp()->RightPanel;

	switch (pPanel->GetType())
	{
	case FILE_PANEL:
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
	case INFO_PANEL:
		pMenu[MENU_LEFT_INFOPANEL].SetCheck(1);
		break;
	case TREE_PANEL:
		pMenu[MENU_LEFT_TREEPANEL].SetCheck(1);
		break;
	case QVIEW_PANEL:
		pMenu[MENU_LEFT_QUICKVIEW].SetCheck(1);
		break;
	}

	pMenu[MENU_LEFT_LONGNAMES].SetCheck(!pPanel->GetShowShortNamesMode());
}

void Options::ShellOptions(bool LastCommand, const MOUSE_EVENT_RECORD *MouseEvent)
{
	auto ApplyViewModesNames = [this](MenuDataEx* Menu)
	{
		for (size_t i = 0; i < 10; ++i)
		{
			if (!ViewSettings[i].Name.empty())
				Menu[i? i - 1 : 9].Name = ViewSettings[i].Name.data();
		}
	};

	MenuDataEx LeftMenu[]=
	{
		MSG(MMenuBriefView),LIF_SELECTED,KEY_CTRL1,
		MSG(MMenuMediumView),0,KEY_CTRL2,
		MSG(MMenuFullView),0,KEY_CTRL3,
		MSG(MMenuWideView),0,KEY_CTRL4,
		MSG(MMenuDetailedView),0,KEY_CTRL5,
		MSG(MMenuDizView),0,KEY_CTRL6,
		MSG(MMenuLongDizView),0,KEY_CTRL7,
		MSG(MMenuOwnersView),0,KEY_CTRL8,
		MSG(MMenuLinksView),0,KEY_CTRL9,
		MSG(MMenuAlternativeView),0,KEY_CTRL0,
		L"",LIF_SEPARATOR,0,
		MSG(MMenuInfoPanel),0,KEY_CTRLL,
		MSG(MMenuTreePanel),0,KEY_CTRLT,
		MSG(MMenuQuickView),0,KEY_CTRLQ,
		L"",LIF_SEPARATOR,0,
		MSG(MMenuSortModes),0,KEY_CTRLF12,
		MSG(MMenuLongNames),0,KEY_CTRLN,
		MSG(MMenuTogglePanel),0,KEY_CTRLF1,
		MSG(MMenuReread),0,KEY_CTRLR,
		MSG(MMenuChangeDrive),0,KEY_ALTF1,
	};
	ApplyViewModesNames(LeftMenu);
	std::vector<string> LeftMenuStrings(ARRAYSIZE(LeftMenu));
	VMenu::AddHotkeys(LeftMenuStrings, LeftMenu, ARRAYSIZE(LeftMenu));

	MenuDataEx FilesMenu[]=
	{
		MSG(MMenuView),LIF_SELECTED,KEY_F3,
		MSG(MMenuEdit),0,KEY_F4,
		MSG(MMenuCopy),0,KEY_F5,
		MSG(MMenuMove),0,KEY_F6,
		MSG(MMenuLink),0,KEY_ALTF6,
		MSG(MMenuCreateFolder),0,KEY_F7,
		MSG(MMenuDelete),0,KEY_F8,
		MSG(MMenuWipe),0,KEY_ALTDEL,
		L"",LIF_SEPARATOR,0,
		MSG(MMenuAdd),0,KEY_SHIFTF1,
		MSG(MMenuExtract),0,KEY_SHIFTF2,
		MSG(MMenuArchiveCommands),0,KEY_SHIFTF3,
		L"",LIF_SEPARATOR,0,
		MSG(MMenuAttributes),0,KEY_CTRLA,
		MSG(MMenuApplyCommand),0,KEY_CTRLG,
		MSG(MMenuDescribe),0,KEY_CTRLZ,
		L"",LIF_SEPARATOR,0,
		MSG(MMenuSelectGroup),0,KEY_ADD,
		MSG(MMenuUnselectGroup),0,KEY_SUBTRACT,
		MSG(MMenuInvertSelection),0,KEY_MULTIPLY,
		MSG(MMenuRestoreSelection),0,KEY_CTRLM,
	};
	std::vector<string> FilesMenuStrings(ARRAYSIZE(FilesMenu));
	VMenu::AddHotkeys(FilesMenuStrings, FilesMenu, ARRAYSIZE(FilesMenu));

	MenuDataEx CmdMenu[]=
	{
		MSG(MMenuFindFile),LIF_SELECTED,KEY_ALTF7,
		MSG(MMenuHistory),0,KEY_ALTF8,
		MSG(MMenuVideoMode),0,KEY_ALTF9,
		MSG(MMenuFindFolder),0,KEY_ALTF10,
		MSG(MMenuViewHistory),0,KEY_ALTF11,
		MSG(MMenuFoldersHistory),0,KEY_ALTF12,
		L"",LIF_SEPARATOR,0,
		MSG(MMenuSwapPanels),0,KEY_CTRLU,
		MSG(MMenuTogglePanels),0,KEY_CTRLO,
		MSG(MMenuCompareFolders),0,0,
		L"",LIF_SEPARATOR,0,
		MSG(MMenuUserMenu),0,0,
		MSG(MMenuFileAssociations),0,0,
		MSG(MMenuFolderShortcuts),0,0,
		MSG(MMenuFilter),0,KEY_CTRLI,
		L"",LIF_SEPARATOR,0,
		MSG(MMenuPluginCommands),0,KEY_F11,
		MSG(MMenuWindowsList),0,KEY_F12,
		MSG(MMenuProcessList),0,KEY_CTRLW,
		MSG(MMenuHotPlugList),0,0,
	};
	std::vector<string> CmdMenuStrings(ARRAYSIZE(CmdMenu));
	VMenu::AddHotkeys(CmdMenuStrings, CmdMenu, ARRAYSIZE(CmdMenu));

	MenuDataEx OptionsMenu[]=
	{
		MSG(MMenuSystemSettings),LIF_SELECTED,0,
		MSG(MMenuPanelSettings),0,0,
		MSG(MMenuTreeSettings),0,0,
		MSG(MMenuInterface),0,0,
		MSG(MMenuLanguages),0,0,
		MSG(MMenuPluginsConfig),0,0,
		MSG(MMenuPluginsManagerSettings),0, 0,
		MSG(MMenuDialogSettings),0,0,
		MSG(MMenuVMenuSettings),0,0,
		MSG(MMenuCmdlineSettings),0,0,
		MSG(MMenuAutoCompleteSettings),0,0,
		MSG(MMenuInfoPanelSettings),0,0,
		MSG(MMenuMaskGroups),0,0,
		L"",LIF_SEPARATOR,0,
		MSG(MMenuConfirmation),0,0,
		MSG(MMenuFilePanelModes),0,0,
		MSG(MMenuFileDescriptions),0,0,
		MSG(MMenuFolderInfoFiles),0,0,
		L"",LIF_SEPARATOR,0,
		MSG(MMenuViewer),0,0,
		MSG(MMenuEditor),0,0,
		MSG(MMenuCodePages),0,0,
		L"",LIF_SEPARATOR,0,
		MSG(MMenuColors),0,0,
		MSG(MMenuFilesHighlighting),0,0,
		L"",LIF_SEPARATOR,0,
		MSG(MMenuSaveSetup),0,KEY_SHIFTF9,
	};
	std::vector<string> OptionsMenuStrings(ARRAYSIZE(OptionsMenu));
	VMenu::AddHotkeys(OptionsMenuStrings, OptionsMenu, ARRAYSIZE(OptionsMenu));

	MenuDataEx RightMenu[]=
	{
		MSG(MMenuBriefView),LIF_SELECTED,KEY_CTRL1,
		MSG(MMenuMediumView),0,KEY_CTRL2,
		MSG(MMenuFullView),0,KEY_CTRL3,
		MSG(MMenuWideView),0,KEY_CTRL4,
		MSG(MMenuDetailedView),0,KEY_CTRL5,
		MSG(MMenuDizView),0,KEY_CTRL6,
		MSG(MMenuLongDizView),0,KEY_CTRL7,
		MSG(MMenuOwnersView),0,KEY_CTRL8,
		MSG(MMenuLinksView),0,KEY_CTRL9,
		MSG(MMenuAlternativeView),0,KEY_CTRL0,
		L"",LIF_SEPARATOR,0,
		MSG(MMenuInfoPanel),0,KEY_CTRLL,
		MSG(MMenuTreePanel),0,KEY_CTRLT,
		MSG(MMenuQuickView),0,KEY_CTRLQ,
		L"",LIF_SEPARATOR,0,
		MSG(MMenuSortModes),0,KEY_CTRLF12,
		MSG(MMenuLongNames),0,KEY_CTRLN,
		MSG(MMenuTogglePanelRight),0,KEY_CTRLF2,
		MSG(MMenuReread),0,KEY_CTRLR,
		MSG(MMenuChangeDriveRight),0,KEY_ALTF2,
	};
	ApplyViewModesNames(RightMenu);
	std::vector<string> RightMenuStrings(ARRAYSIZE(RightMenu));
	VMenu::AddHotkeys(RightMenuStrings, RightMenu, ARRAYSIZE(RightMenu));


	HMenuData MainMenu[]=
	{
		{MSG(MMenuLeftTitle), L"LeftRightMenu", LeftMenu, ARRAYSIZE(LeftMenu), 1},
		{MSG(MMenuFilesTitle), L"FilesMenu", FilesMenu, ARRAYSIZE(FilesMenu), 0},
		{MSG(MMenuCommandsTitle), L"CmdMenu", CmdMenu, ARRAYSIZE(CmdMenu), 0},
		{MSG(MMenuOptionsTitle), L"OptMenu", OptionsMenu, ARRAYSIZE(OptionsMenu), 0},
		{MSG(MMenuRightTitle), L"LeftRightMenu", RightMenu, ARRAYSIZE(RightMenu), 0},
	};
	static int LastHItem=-1,LastVItem=0;
	int HItem,VItem;

	SetLeftRightMenuChecks(LeftMenu, true);
	SetLeftRightMenuChecks(RightMenu, false);
	// Навигация по меню
	{
		HMenu HOptMenu(MainMenu,ARRAYSIZE(MainMenu));
		HOptMenu.SetHelp(L"Menus");
		HOptMenu.SetPosition(0,0,ScrX,0);
		Global->WindowManager->ExecuteWindow(&HOptMenu);

		if (LastCommand)
		{
			MenuDataEx *VMenuTable[] = {LeftMenu, FilesMenu, CmdMenu, OptionsMenu, RightMenu};
			int HItemToShow = LastHItem;

			if (HItemToShow == -1)
			{
				if (Global->CtrlObject->Cp()->ActivePanel() == Global->CtrlObject->Cp()->RightPanel &&
					Global->CtrlObject->Cp()->ActivePanel()->IsVisible())
					HItemToShow = 4;
				else
					HItemToShow = 0;
			}

			MainMenu[0].Selected = 0;
			MainMenu[HItemToShow].Selected = 1;
			VMenuTable[HItemToShow][0].SetSelect(0);
			VMenuTable[HItemToShow][LastVItem].SetSelect(1);
			Global->WindowManager->CallbackWindow([&HOptMenu](){HOptMenu.ProcessKey(Manager::Key(KEY_DOWN));});
		}
		else
		{
			if (Global->CtrlObject->Cp()->ActivePanel() == Global->CtrlObject->Cp()->RightPanel &&
				Global->CtrlObject->Cp()->ActivePanel()->IsVisible())
			{
				MainMenu[0].Selected = 0;
				MainMenu[4].Selected = 1;
			}
		}

		if (MouseEvent)
		{
			Global->WindowManager->CallbackWindow([&HOptMenu,MouseEvent](){HOptMenu.ProcessMouse(MouseEvent);});
		}

		Global->WindowManager->ExecuteModal(&HOptMenu);
		HOptMenu.GetExitCode(HItem,VItem);
	}

	// "Исполнятор команд меню"
	switch (HItem)
	{
	case MENU_LEFT:
	case MENU_RIGHT:
		{
			Panel *pPanel = (HItem == MENU_LEFT)?Global->CtrlObject->Cp()->LeftPanel:Global->CtrlObject->Cp()->RightPanel;

			if (VItem >= MENU_LEFT_BRIEFVIEW && VItem <= MENU_LEFT_ALTERNATIVEVIEW)
			{
				Global->CtrlObject->Cp()->ChangePanelToFilled(pPanel, FILE_PANEL);
				pPanel=(HItem == MENU_LEFT)?Global->CtrlObject->Cp()->LeftPanel:Global->CtrlObject->Cp()->RightPanel;
				pPanel->SetViewMode((VItem == MENU_LEFT_ALTERNATIVEVIEW)?VIEW_0:VIEW_1+VItem);
			}
			else
			{
				switch (VItem)
				{
				case MENU_LEFT_INFOPANEL: // Info panel
					Global->CtrlObject->Cp()->ChangePanelToFilled(pPanel, INFO_PANEL);
					break;
				case MENU_LEFT_TREEPANEL: // Tree panel
					Global->CtrlObject->Cp()->ChangePanelToFilled(pPanel, TREE_PANEL);
					break;
				case MENU_LEFT_QUICKVIEW: // Quick view
					Global->CtrlObject->Cp()->ChangePanelToFilled(pPanel, QVIEW_PANEL);
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
					pPanel->ChangeDisk();
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
					UserMenu Menu(true);
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
							Language NewLanguage(Global->g_strFarPath, MNewFileName + 1);
							Global->Lang->swap(NewLanguage);
						}
						catch (const std::exception& e)
						{
							Message(MSG_WARNING, 1, MSG(MError), wide(e.what()).data(), MSG(MOk));
							strLanguage = SavedLanguage;
						}

						SelectHelpLanguage();
						Global->CtrlObject->Plugins->ReloadLanguage();
						api::env::set_variable(L"FARLANG", strLanguage);
						PrepareStrFTime();
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

	int CurrentWindowType = Global->WindowManager->GetCurrentWindow()->GetType();
	// TODO:Здесь как то нужно изменить, чтобы учесть будущие новые типы полноэкранных окон
	//      или то, что, скажем редактор/вьювер может быть не полноэкранным

	if (!(CurrentWindowType == windowtype_viewer || CurrentWindowType == windowtype_editor))
		Global->CtrlObject->CmdLine->Show();

	if (HItem != -1 && VItem != -1)
	{
		LastHItem = HItem;
		LastVItem = VItem;
	}
}

string GetFarIniString(const string& AppName, const string& KeyName, const string& Default)
{
	return api::GetPrivateProfileString(AppName, KeyName, Default, Global->g_strFarINI);
}

int GetFarIniInt(const string& AppName, const string& KeyName, int Default)
{
	return GetPrivateProfileInt(AppName.data(), KeyName.data(), Default, Global->g_strFarINI.data());
}
