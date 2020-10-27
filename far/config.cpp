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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "config.hpp"

// Internal:
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
#include "uuids.plugins.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "uuids.far.dialogs.hpp"
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
#include "string_sort.hpp"
#include "global.hpp"
#include "locale.hpp"
#include "console.hpp"
#include "scrbuf.hpp"

// Platform:
#include "platform.env.hpp"

// Common:
#include "common/algorithm.hpp"
#include "common/from_string.hpp"
#include "common/uuid.hpp"
#include "common/view/enumerate.hpp"
#include "common/view/zip.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static const size_t predefined_panel_modes_count = 10;

// Стандартный набор разделителей
static const auto WordDiv0 = L"~!%^&*()+|{}:\"<>?`-=\\[];',./"sv;

// Стандартный набор разделителей для функции Xlat
static const auto WordDivForXlat0 = L" \t!#$%^&*()+|=\\/@?"sv;

static const int DefaultTabSize = 8;

static const auto

#if defined(TREEFILE_PROJECT)
	LocalDiskTemplate = L"LD.%D.%SN.tree"sv,
	tNetDiskTemplate = L"ND.%D.%SN.tree"sv,
	NetPathTemplate = L"NP.%SR.%SH.tree"sv,
	RemovableDiskTemplate = L"RD.%SN.tree"sv,
	CDDiskTemplate = L"CD.%L.%SN.tree"sv,
#endif

	NKeyScreen = L"Screen"sv,
	NKeyCmdline = L"Cmdline"sv,
	NKeyInterface = L"Interface"sv,
	NKeyInterfaceCompletion = L"Interface.Completion"sv,
	NKeyViewer = L"Viewer"sv,
	NKeyDialog = L"Dialog"sv,
	NKeyEditor = L"Editor"sv,
	NKeyXLat = L"XLat"sv,
	NKeySystem = L"System"sv,
	NKeySystemSort = L"System.Sort"sv,
	NKeySystemException = L"System.Exception"sv,
	NKeySystemKnownIDs = L"System.KnownIDs"sv,
	NKeySystemExecutor = L"System.Executor"sv,
	NKeySystemNowell = L"System.Nowell"sv,
	NKeyHelp = L"Help"sv,
	NKeyLanguage = L"Language"sv,
	NKeyConfirmations = L"Confirmations"sv,
	NKeyPluginConfirmations = L"PluginConfirmations"sv,
	NKeyPanel = L"Panel"sv,
	NKeyPanelLeft = L"Panel.Left"sv,
	NKeyPanelRight = L"Panel.Right"sv,
	NKeyPanelLayout = L"Panel.Layout"sv,
	NKeyPanelTree = L"Panel.Tree"sv,
	NKeyPanelInfo = L"Panel.Info"sv,
	NKeyPanelSortLayers = L"Panel.SortLayers"sv,
	NKeyLayout = L"Layout"sv,
	NKeyDescriptions = L"Descriptions"sv,
	NKeyKeyMacros = L"Macros"sv,
	NKeyPolicies = L"Policies"sv,
	NKeyCodePages = L"CodePages"sv,
	NKeyVMenu = L"VMenu"sv,
	NKeyCommandHistory = L"History.CommandHistory"sv,
	NKeyViewEditHistory = L"History.ViewEditHistory"sv,
	NKeyFolderHistory = L"History.FolderHistory"sv,
	NKeyDialogHistory = L"History.DialogHistory"sv;

static size_t DisplayModeToReal(size_t Mode)
{
	return Mode < predefined_panel_modes_count? (Mode == 9? 0 : Mode + 1) : Mode - 1;
}

static size_t RealModeToDisplay(size_t Mode)
{
	return Mode < predefined_panel_modes_count? (Mode == 0? 9 : Mode - 1) : Mode + 1;
}

void Options::SystemSettings()
{
	const auto GetSortingState = [&]
	{
		return std::tuple(Sort.Collation.Get(), Sort.DigitsAsNumbers.Get(), Sort.CaseSensitive.Get());
	};

	const auto CurrentSortingState = GetSortingState();

	DialogBuilder Builder(lng::MConfigSystemTitle, L"SystemSettings"sv);

	Builder.AddCheckbox(lng::MConfigRecycleBin, DeleteToRecycleBin);
	Builder.AddCheckbox(lng::MConfigSystemCopy, CMOpt.UseSystemCopy);
	Builder.AddCheckbox(lng::MConfigCopySharing, CMOpt.CopyOpened);
	Builder.AddCheckbox(lng::MConfigScanJunction, ScanJunction);
	Builder.AddCheckbox(lng::MConfigSmartFolderMonitor, SmartFolderMonitor);

	Builder.AddCheckbox(lng::MConfigSaveHistory, SaveHistory);
	Builder.AddCheckbox(lng::MConfigSaveFoldersHistory, SaveFoldersHistory);
	Builder.AddCheckbox(lng::MConfigSaveViewHistory, SaveViewHistory);
	Builder.AddCheckbox(lng::MConfigRegisteredTypes, UseRegisteredTypes);
	Builder.AddCheckbox(lng::MConfigUpdateEnvironment, UpdateEnvironment);
	Builder.AddText(lng::MConfigElevation);
	Builder.AddCheckbox(lng::MConfigElevationModify, StoredElevationMode, ELEVATION_MODIFY_REQUEST)->Indent(4);
	Builder.AddCheckbox(lng::MConfigElevationRead, StoredElevationMode, ELEVATION_READ_REQUEST)->Indent(4);
	Builder.AddCheckbox(lng::MConfigElevationUsePrivileges, StoredElevationMode, ELEVATION_USE_PRIVILEGES)->Indent(4);

	static const DialogBuilderListItem SortingMethods[] =
	{
		{ lng::MConfigSortingOrdinal, as_underlying_type(SortingOptions::collation::ordinal) },
		{ lng::MConfigSortingInvariant, as_underlying_type(SortingOptions::collation::invariant) },
		{ lng::MConfigSortingLinguistic, as_underlying_type(SortingOptions::collation::linguistic) },
	};

	const auto SortingMethodsComboBox = Builder.AddComboBox(Sort.Collation, 20, SortingMethods);
	Builder.AddTextBefore(SortingMethodsComboBox, lng::MConfigSortingCollation);
	Builder.AddCheckbox(lng::MConfigSortingDigitsAsNumbers, Sort.DigitsAsNumbers)->Indent(4);
	Builder.AddCheckbox(lng::MConfigSortingCase, Sort.CaseSensitive)->Indent(4);

	Builder.AddCheckbox(lng::MConfigAutoSave, AutoSaveSetup);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		ElevationMode = StoredElevationMode;

		if (CurrentSortingState != GetSortingState())
		{
			Global->CtrlObject->Cp()->ActivePanel()->OnSortingChange();
			Global->CtrlObject->Cp()->PassivePanel()->OnSortingChange();
		}
	}
}

void Options::PanelSettings()
{
	DialogBuilder Builder(lng::MConfigPanelTitle, L"PanelSettings"sv);
	BOOL AutoUpdate = AutoUpdateLimit;

	Builder.AddCheckbox(lng::MConfigHidden, ShowHidden);
	Builder.AddCheckbox(lng::MConfigHighlight, Highlight);
	Builder.AddCheckbox(lng::MConfigSelectFolders, SelectFolders);
	Builder.AddCheckbox(lng::MConfigRightClickSelect, RightClickSelect);
	Builder.AddCheckbox(lng::MConfigSortFolderExt, SortFolderExt);
	Builder.AddCheckbox(lng::MConfigAllowReverseSort, AllowReverseSort);

	const auto AutoUpdateEnabled = Builder.AddCheckbox(lng::MConfigAutoUpdateLimit, AutoUpdate);
	const auto AutoUpdateLimitItem = Builder.AddIntEditField(AutoUpdateLimit, 6);
	Builder.LinkFlags(AutoUpdateEnabled, AutoUpdateLimitItem, DIF_DISABLE, false);
	const auto AutoUpdateTextItem = Builder.AddTextBefore(AutoUpdateLimitItem, lng::MConfigAutoUpdateLimit2);
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
	DialogBuilder Builder(lng::MConfigTreeTitle, L"TreeSettings"sv);

	Builder.AddCheckbox(lng::MConfigTreeAutoChange, Tree.AutoChangeFolder);

	const auto TemplateEdit = Builder.AddIntEditField(Tree.MinTreeCount, 3);
	Builder.AddTextBefore(TemplateEdit, lng::MConfigTreeLabelMinFolder);

#if defined(TREEFILE_PROJECT)
	Builder.AddSeparator(lng::MConfigTreeLabel1);

	auto Checkbox = Builder.AddCheckbox(lng::MConfigTreeLabelLocalDisk, Tree.LocalDisk);
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
	DialogBuilder Builder(lng::MConfigInterfaceTitle, L"InterfSettings"sv);

	Builder.AddCheckbox(lng::MConfigClock, Clock);
	Builder.AddCheckbox(lng::MConfigViewerEditorClock, ViewerEditorClock);
	Builder.AddCheckbox(lng::MConfigMouse, Mouse);
	Builder.AddCheckbox(lng::MConfigKeyBar, ShowKeyBar);
	Builder.AddCheckbox(lng::MConfigMenuBar, ShowMenuBar);
	const auto SaverCheckbox = Builder.AddCheckbox(lng::MConfigSaver, ScreenSaver);
	const auto SaverEdit = Builder.AddIntEditField(ScreenSaverTime, 3);
	SaverEdit->Indent(5);
	Builder.AddTextAfter(SaverEdit, lng::MConfigSaverMinutes);
	Builder.LinkFlags(SaverCheckbox, SaverEdit, DIF_DISABLE);

	Builder.AddCheckbox(lng::MConfigCopyTotal, CMOpt.CopyShowTotal);
	Builder.AddCheckbox(lng::MConfigCopyTimeRule, CMOpt.CopyTimeRule);
	Builder.AddCheckbox(lng::MConfigDeleteTotal, DelOpt.ShowTotal);
	Builder.AddCheckbox(lng::MConfigPgUpChangeDisk, PgUpChangeDisk);
	Builder.AddCheckbox(lng::MConfigUseVirtualTerminalForRendering, VirtualTerminalRendering);
	Builder.AddCheckbox(lng::MConfigClearType, ClearType);
	Builder.StartColumns();
	const auto SetIconCheck = Builder.AddCheckbox(lng::MConfigSetConsoleIcon, SetIcon);
	Builder.ColumnBreak();

	std::vector<DialogBuilderListItem> IconIndices;
	IconIndices.reserve(consoleicons::instance().size());

	for (size_t i = 0, size = consoleicons::instance().size(); i != size; ++i)
	{
		IconIndices.emplace_back(str(i), static_cast<int>(i));
	}

	const auto IconIndexEdit = Builder.AddComboBox(IconIndex, 0, IconIndices);
	Builder.EndColumns();
	Builder.LinkFlags(SetIconCheck, IconIndexEdit, DIF_DISABLE);
	const auto SetAdminIconCheck = Builder.AddCheckbox(lng::MConfigSetAdminConsoleIcon, SetAdminIcon);
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
		consoleicons::instance().set_icon();

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
	DialogBuilder Builder(lng::MConfigAutoCompleteTitle, L"AutoCompleteSettings"sv);
	const auto ListCheck=Builder.AddCheckbox(lng::MConfigAutoCompleteShowList, AutoComplete.ShowList);
	const auto ModalModeCheck=Builder.AddCheckbox(lng::MConfigAutoCompleteModalList, AutoComplete.ModalList);
	ModalModeCheck->Indent(4);
	Builder.AddCheckbox(lng::MConfigAutoCompleteAutoAppend, AutoComplete.AppendCompletion);
	Builder.LinkFlags(ListCheck, ModalModeCheck, DIF_DISABLE);
	Builder.AddOKCancel();
	Builder.ShowDialog();
}

void Options::InfoPanelSettings()
{
	static const DialogBuilderListItem UNListItems[]
	{
		{ lng::MConfigInfoPanelUNLogon,             NameUnknown },
		{ lng::MConfigInfoPanelUNFullyQualifiedDN,  NameFullyQualifiedDN },
		{ lng::MConfigInfoPanelUNSamCompatible,     NameSamCompatible },
		{ lng::MConfigInfoPanelUNDisplay,           NameDisplay },
		{ lng::MConfigInfoPanelUNUniqueId,          NameUniqueId },
		{ lng::MConfigInfoPanelUNCanonical,         NameCanonical },
		{ lng::MConfigInfoPanelUNUserPrincipal,     NameUserPrincipal },
		{ lng::MConfigInfoPanelUNServicePrincipal,  NameServicePrincipal },
		{ lng::MConfigInfoPanelUNDnsDomain,         NameDnsDomain },
		{ lng::MConfigInfoPanelUNGivenName,         NameGivenName },
		{ lng::MConfigInfoPanelUNSurname,           NameSurname },
	};

	static const DialogBuilderListItem CNListItems[]
	{
		{ lng::MConfigInfoPanelCNNetBIOS,                    ComputerNameNetBIOS },
		{ lng::MConfigInfoPanelCNDnsHostname,                ComputerNameDnsHostname },
		{ lng::MConfigInfoPanelCNDnsDomain,                  ComputerNameDnsDomain },
		{ lng::MConfigInfoPanelCNDnsFullyQualified,          ComputerNameDnsFullyQualified },
		{ lng::MConfigInfoPanelCNPhysicalNetBIOS,            ComputerNamePhysicalNetBIOS },
		{ lng::MConfigInfoPanelCNPhysicalDnsHostname,        ComputerNamePhysicalDnsHostname },
		{ lng::MConfigInfoPanelCNPhysicalDnsDomain,          ComputerNamePhysicalDnsDomain },
		{ lng::MConfigInfoPanelCNPhysicalDnsFullyQualified,  ComputerNamePhysicalDnsFullyQualified },
	};

	DialogBuilder Builder(lng::MConfigInfoPanelTitle, L"InfoPanelSettings"sv);
	Builder.AddCheckbox(lng::MConfigInfoPanelShowPowerStatus, InfoPanel.ShowPowerStatus);
	Builder.AddCheckbox(lng::MConfigInfoPanelShowCDInfo, InfoPanel.ShowCDInfo);
	Builder.AddText(lng::MConfigInfoPanelCNTitle);
	Builder.AddComboBox(InfoPanel.ComputerNameFormat, 50, CNListItems);
	Builder.AddText(lng::MConfigInfoPanelUNTitle);
	Builder.AddComboBox(InfoPanel.UserNameFormat, 50, UNListItems);
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
	static const std::pair<string_view, string_view> Sets[] =
	{
		{ L"arc"sv,  L"*.zip,*.rar,*.[7bgxl]z,*.[bg]zip,*.tar,*.t[agbxl]z,*.z,*.ar[cj],*.r[0-9][0-9],*.a[0-9][0-9],*.bz2,*.cab,*.jar,*.lha,*.lzh,*.ha,*.ac[bei],*.pa[ck],*.rk,*.cpio,*.rpm,*.zoo,*.hqx,*.sit,*.ice,*.uc2,*.ain,*.imp,*.777,*.ufa,*.boa,*.bs[2a],*.sea,*.[ah]pk,*.ddi,*.x2,*.rkv,*.[lw]sz,*.h[ay]p,*.lim,*.sqz,*.chz,*.aa[br]"sv },
		{ L"temp"sv, L"*.bak,*.tmp"sv },
		{ L"exec"sv, L"*.exe,*.cmd,*.bat,*.com,%PATHEXT%"sv },
	};

	for (const auto& [Name, Value]: Sets)
	{
		ConfigProvider().GeneralCfg()->SetValue(L"Masks"sv, Name, Value);
	}
}

static void FillMasksMenu(VMenu2& MasksMenu, int SelPos = 0)
{
	MasksMenu.clear();

	for(const auto& [Name, Value]: ConfigProvider().GeneralCfg()->ValuesEnumerator<string>(L"Masks"sv))
	{
		MenuItemEx Item;
		const int NameWidth = 10;
		const auto DisplayName = pad_right(truncate_right(Name, NameWidth), NameWidth);
		Item.Name = concat(DisplayName, L' ', BoxSymbols[BS_V1], L' ', Value);
		Item.ComplexUserData = Name;
		MasksMenu.AddItem(Item);
	}
	MasksMenu.SetSelectPos(SelPos, 0);
}

void Options::MaskGroupsSettings()
{
	const auto MasksMenu = VMenu2::create(msg(lng::MMaskGroupTitle), {}, 0, VMENU_WRAPMODE | VMENU_SHOWAMPERSAND);
	const auto BottomTitle = KeysToLocalizedText(KEY_INS, KEY_DEL, KEY_F4, KEY_F7, KEY_CTRLR);
	MasksMenu->SetBottomTitle(BottomTitle);
	MasksMenu->SetHelp(L"MaskGroupsSettings"sv);
	FillMasksMenu(*MasksMenu);
	MasksMenu->SetPosition({ -1, -1, -1, -1 });

	bool Changed = false;
	bool Filter = false;
	for(;;)
	{
		MasksMenu->Run([&](const Manager::Key& RawKey)
		{
			const auto Key=RawKey();
			if(Filter && any_of(Key, KEY_ESC, KEY_F10, KEY_ENTER, KEY_NUMENTER))
			{
				Filter = false;
				for (size_t i = 0, size = MasksMenu->size(); i != size;  ++i)
				{
					MasksMenu->UpdateItemFlags(static_cast<int>(i), MasksMenu->at(i).Flags & ~MIF_HIDDEN);
				}
				MasksMenu->SetPosition({ -1, -1, -1, -1 });
				MasksMenu->SetBottomTitle(BottomTitle);
				return 1;
			}
			int ItemPos = MasksMenu->GetSelectPos();
			const auto* Item = MasksMenu->GetComplexUserDataPtr<string>(ItemPos);
			int KeyProcessed = 1;
			static const string EmptyString;
			switch (Key)
			{
			case KEY_NUMDEL:
			case KEY_DEL:
				if(Item && Message(0,
					msg(lng::MMaskGroupTitle),
					{
						msg(lng::MMaskGroupAskDelete),
						*Item
					},
					{ lng::MDelete, lng::MCancel }) == Message::first_button)
				{
					ConfigProvider().GeneralCfg()->DeleteValue(L"Masks"sv, *Item);
					Changed = true;
				}
				break;

			case KEY_NUMPAD0:
			case KEY_INS:
				Item = &EmptyString;
				[[fallthrough]];
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
							Value = ConfigProvider().GeneralCfg()->GetValue<string>(L"Masks"sv, Name);
						}
						DialogBuilder Builder(lng::MMaskGroupTitle, L"MaskGroupsSettings"sv);
						Builder.AddText(lng::MMaskGroupName);
						Builder.AddEditField(Name, 60);
						Builder.AddText(lng::MMaskGroupMasks);
						Builder.AddEditField(Value, 60);
						Builder.AddOKCancel();
						if(Builder.ShowDialog())
						{
							if(!Item->empty())
							{
								ConfigProvider().GeneralCfg()->DeleteValue(L"Masks"sv, *Item);
							}
							ConfigProvider().GeneralCfg()->SetValue(L"Masks"sv, Name, Value);
							Changed = true;
						}
					}
				}
				break;

			case KEY_CTRLR:
			case KEY_RCTRLR:
				{
					if (Message(MSG_WARNING,
						msg(lng::MMaskGroupTitle),
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
					DialogBuilder Builder(lng::MFileFilterTitle);
					Builder.AddText(lng::MMaskGroupFindMask);
					Builder.AddEditField(Value, 60, L"MaskGroupsFindMask"sv);
					Builder.AddOKCancel();
					if(Builder.ShowDialog())
					{
						for (size_t i = 0, size = MasksMenu->size(); i != size; ++i)
						{
							filemasks Masks;
							Masks.assign(ConfigProvider().GeneralCfg()->GetValue<string>(L"Masks"sv, *MasksMenu->GetComplexUserDataPtr<string>(i)));
							if(!Masks.check(Value))
							{
								MasksMenu->UpdateItemFlags(static_cast<int>(i), MasksMenu->at(i).Flags | MIF_HIDDEN);
							}
						}
						MasksMenu->SetPosition({ -1, -1, -1, -1 });
						MasksMenu->SetTitle(Value);
						MasksMenu->SetBottomTitle(format(msg(lng::MMaskGroupTotal), MasksMenu->GetShowItemCount()));
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
	DialogBuilder Builder(lng::MConfigDlgSetsTitle, L"DialogSettings"sv);

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

	static const DialogBuilderListItem CAListItems[]
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

	DialogBuilder Builder(lng::MConfigVMenuTitle, L"VMenuSettings"sv);

	for (const auto& [LngId, OptPtr]: DialogItems)
	{
		Builder.AddText(LngId);
		Builder.AddComboBox(std::invoke(OptPtr, VMenu), 40, CAListItems);
	}

	Builder.AddOKCancel();
	Builder.ShowDialog();
}

void Options::CmdlineSettings()
{
	DialogBuilder Builder(lng::MConfigCmdlineTitle, L"CmdlineSettings"sv);

	Builder.AddCheckbox(lng::MConfigCmdlineEditBlock, CmdLine.EditBlock);
	Builder.AddCheckbox(lng::MConfigCmdlineDelRemovesBlocks, CmdLine.DelRemovesBlocks);
	Builder.AddCheckbox(lng::MConfigCmdlineAutoComplete, CmdLine.AutoComplete);

	const auto UsePromptFormat = Builder.AddCheckbox(lng::MConfigCmdlineUsePromptFormat, CmdLine.UsePromptFormat);
	const auto PromptFormat = Builder.AddEditField(CmdLine.strPromptFormat, 33);
	PromptFormat->Indent(4);
	Builder.LinkFlags(UsePromptFormat, PromptFormat, DIF_DISABLE);

	const auto UseHomeDir = Builder.AddCheckbox(lng::MConfigCmdlineUseHomeDir, Exec.UseHomeDir);
	const auto HomeDir = Builder.AddEditField(Exec.strHomeDir, 33);
	HomeDir->Indent(4);
	Builder.LinkFlags(UseHomeDir, HomeDir, DIF_DISABLE);

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
	DialogBuilder Builder(lng::MSetConfirmTitle, L"ConfirmDlg"sv);

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
	DialogBuilder Builder(lng::MPluginsManagerSettingsTitle, L"PluginsManagerSettings"sv);
#ifndef NO_WRAPPER
	Builder.AddCheckbox(lng::MPluginsManagerOEMPluginsSupport, LoadPlug.OEMPluginsSupport);
#endif // NO_WRAPPER
	Builder.AddCheckbox(lng::MPluginsManagerScanSymlinks, LoadPlug.ScanSymlinks);
	Builder.AddSeparator(lng::MPluginConfirmationTitle);
	Builder.AddCheckbox(lng::MPluginsManagerOFP, PluginConfirm.OpenFilePlugin);
	const auto StandardAssoc = Builder.AddCheckbox(lng::MPluginsManagerStdAssoc, PluginConfirm.StandardAssociation);
	const auto EvenIfOnlyOne = Builder.AddCheckbox(lng::MPluginsManagerEvenOne, PluginConfirm.EvenIfOnlyOnePlugin);
	StandardAssoc->Indent(2);
	EvenIfOnlyOne->Indent(4);

	Builder.AddCheckbox(lng::MPluginsManagerSFL, PluginConfirm.SetFindList);
	Builder.AddCheckbox(lng::MPluginsManagerPF, PluginConfirm.Prefix);
	Builder.AddOKCancel();

	Builder.ShowDialog();
}

void Options::SetDizConfig()
{
	DialogBuilder Builder(lng::MCfgDizTitle, L"FileDiz"sv);

	Builder.AddText(lng::MCfgDizListNames);
	Builder.AddEditField(Diz.strListNames, 65);
	Builder.AddSeparator();

	Builder.AddCheckbox(lng::MCfgDizSetHidden, Diz.SetHidden);
	Builder.AddCheckbox(lng::MCfgDizROUpdate, Diz.ROUpdate);
	const auto StartPos = Builder.AddIntEditField(Diz.StartPos, 2);
	Builder.AddTextAfter(StartPos, lng::MCfgDizStartPos);
	Builder.AddSeparator();

	static const lng DizOptions[] = { lng::MCfgDizNotUpdate, lng::MCfgDizUpdateIfDisplayed, lng::MCfgDizAlwaysUpdate };
	Builder.AddRadioButtons(Diz.UpdateMode, DizOptions);
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

	DialogBuilder Builder(lng::MViewConfigTitle, L"ViewerSettings"sv, [&](Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
	{
		if (Msg == DN_INITDIALOG && save_pos)
		{
			Dlg->SendMessage(DM_ENABLE, save_cp, ToPtr(!ViOpt.SavePos));
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
				Dlg->SendMessage(DM_ENABLE, save_cp, ToPtr(!Param2));
				inside = false;
			}
			else if (Param1 == save_cp && !inside)
			{
				prev_save_cp_value = (Param2 != nullptr);
			}
		}
		return Dlg->DefProc(Msg, Param1, Param2);
	});

	std::vector<DialogBuilderListItem> Items; //Must live until Dialog end

	if (!Local)
	{
		++id; Builder.AddCheckbox(lng::MViewConfigExternalF3, ViOpt.UseExternalViewer);
		++id; Builder.AddText(lng::MViewConfigExternalCommand);
		++id; Builder.AddEditField(strExternalViewer, 64, L"ExternalViewer"sv, DIF_EDITPATH|DIF_EDITPATHEXEC);
		++id; Builder.AddSeparator(lng::MViewConfigInternal);
	}

	Builder.StartColumns();
	++id; Builder.AddCheckbox(lng::MViewConfigPersistentSelection, ViOptRef.PersistentBlocks);
	++id; Builder.AddCheckbox(lng::MViewConfigEditAutofocus, ViOptRef.SearchEditFocus);
	++id; const auto TabSize = Builder.AddIntEditField(ViOptRef.TabSize, 3);
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
		Builder.AddCheckbox(lng::MViewConfigSaveShortPos, ViOpt.SaveShortPos);
		const auto MaxLineSize = Builder.AddIntEditField(ViOpt.MaxLineSize, 6);
		Builder.AddTextAfter(MaxLineSize, lng::MViewConfigMaxLineSize);
		Builder.ColumnBreak();
		Builder.AddCheckbox(lng::MViewConfigSaveViewMode, ViOpt.SaveViewMode);
		Builder.AddCheckbox(lng::MViewConfigSaveWrapMode, ViOpt.SaveWrapMode);
		Builder.AddCheckbox(lng::MViewConfigDetectDumpMode, ViOpt.DetectDumpMode);
		Builder.AddCheckbox(lng::MViewAutoDetectCodePage, ViOpt.AutoDetectCodePage);
		Builder.EndColumns();
		Builder.AddText(lng::MViewConfigDefaultCodePage);
		codepages::instance().FillCodePagesList(Items, false, false, false, false, true);
		Builder.AddComboBox(ViOpt.DefaultCodePage, 64, Items);
	}

	Builder.AddOKCancel();

	Builder.ShowDialog();
}

void Options::EditorConfig(Options::EditorOptions &EdOptRef, bool Local)
{
	DialogBuilder Builder(lng::MEditConfigTitle, L"EditorSettings"sv);

	std::vector<DialogBuilderListItem> Items; //Must live until Dialog end

	if (!Local)
	{
		Builder.AddCheckbox(lng::MEditConfigEditorF4, EdOpt.UseExternalEditor);
		Builder.AddText(lng::MEditConfigEditorCommand);
		Builder.AddEditField(strExternalEditor, 64, L"ExternalEditor"sv, DIF_EDITPATH|DIF_EDITPATHEXEC);
		Builder.AddSeparator(lng::MEditConfigInternal);
	}

	Builder.AddText(lng::MEditConfigExpandTabsTitle);
	static const DialogBuilderListItem ExpandTabsItems[]
	{
		{ lng::MEditConfigDoNotExpandTabs, EXPAND_NOTABS },
		{ lng::MEditConfigExpandTabs, EXPAND_NEWTABS },
		{ lng::MEditConfigConvertAllTabsToSpaces, EXPAND_ALLTABS }
	};
	Builder.AddComboBox(EdOptRef.ExpandTabs, 64, ExpandTabsItems);

	Builder.StartColumns();
	Builder.AddCheckbox(lng::MEditConfigPersistentBlocks, EdOptRef.PersistentBlocks);
	Builder.AddCheckbox(lng::MEditConfigDelRemovesBlocks, EdOptRef.DelRemovesBlocks);
	Builder.AddCheckbox(lng::MEditConfigAutoIndent, EdOptRef.AutoIndent);
	const auto TabSize = Builder.AddIntEditField(EdOptRef.TabSize, 3);
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
		codepages::instance().FillCodePagesList(Items, false, false, false, false, false);
		Builder.AddComboBox(EdOpt.DefaultCodePage, 64, Items);
	}

	Builder.AddOKCancel();

	Builder.ShowDialog();
}

void Options::SetFolderInfoFiles()
{
	string strFolderInfoFiles;

	if (GetString(
		msg(lng::MSetFolderInfoTitle),
		msg(lng::MSetFolderInfoNames),
		L"FolderInfoFiles"sv,
		InfoPanel.strFolderInfoFiles,
		strFolderInfoFiles,
		L"FolderDiz"sv,
		FIB_ENABLEEMPTY | FIB_BUTTONS))
	{
		InfoPanel.strFolderInfoFiles = strFolderInfoFiles;

		if (Global->CtrlObject->Cp()->LeftPanel()->GetType() == panel_type::INFO_PANEL)
			Global->CtrlObject->Cp()->LeftPanel()->Update(0);

		if (Global->CtrlObject->Cp()->RightPanel()->GetType() == panel_type::INFO_PANEL)
			Global->CtrlObject->Cp()->RightPanel()->Update(0);
	}
}

static void ResetViewModes(span<PanelViewSettings> const Modes, int const Index = -1)
{
	static const struct
	{
		std::initializer_list<column> PanelColumns, StatusColumns;
		unsigned long long Flags;
	}
	InitialModes[]
	{
		// Alternative full
		{
			{
				{ column_type::name,             COLFLAGS_MARK,        0,  },
				{ column_type::size,             COLFLAGS_GROUPDIGITS, 10, },
				{ column_type::date,             COLFLAGS_NONE,        0,  },
			},
			{
				{ column_type::name,             COLFLAGS_RIGHTALIGN,  0,  },
			},
			PVS_ALIGNEXTENSIONS,
		},

		// Brief
		{
			{
				{ column_type::name,             COLFLAGS_NONE,        0,  },
				{ column_type::name,             COLFLAGS_NONE,        0,  },
				{ column_type::name,             COLFLAGS_NONE,        0,  },
			},
			{
				{ column_type::name,             COLFLAGS_RIGHTALIGN,  0,  },
				{ column_type::size,             COLFLAGS_NONE,        6,  },
				{ column_type::date,             COLFLAGS_NONE,        0,  },
				{ column_type::time,             COLFLAGS_NONE,        5,  },
			},
			PVS_ALIGNEXTENSIONS,
		},

		// Medium
		{
			{
				{ column_type::name,             COLFLAGS_NONE,        0,  },
				{ column_type::name,             COLFLAGS_NONE,        0,  },
			},
			{
				{ column_type::name,             COLFLAGS_RIGHTALIGN,  0,  },
				{ column_type::size,             COLFLAGS_NONE,        6,  },
				{ column_type::date,             COLFLAGS_NONE,        0,  },
				{ column_type::time,             COLFLAGS_NONE,        5,  },
			},
			PVS_NONE,
		},

		// Full
		{
			{
				{ column_type::name,             COLFLAGS_NONE,        0,  },
				{ column_type::size,             COLFLAGS_NONE,        6,  },
				{ column_type::date,             COLFLAGS_NONE,        0,  },
				{ column_type::time,             COLFLAGS_NONE,        5,  },
			},
			{
				{ column_type::name,             COLFLAGS_RIGHTALIGN,  0,  },
			},
			PVS_ALIGNEXTENSIONS,
		},

		// Wide
		{
			{
				{ column_type::name,             COLFLAGS_NONE,        0,  },
				{ column_type::size,             COLFLAGS_NONE,        6,  },
			},
			{
				{ column_type::name,             COLFLAGS_RIGHTALIGN,  0,  },
				{ column_type::size,             COLFLAGS_NONE,        6,  },
				{ column_type::date,             COLFLAGS_NONE,        0,  },
				{ column_type::time,             COLFLAGS_NONE,        5,  },
			},
			PVS_NONE
		},

		// Detailed
		{
			{
				{ column_type::name,             COLFLAGS_NONE,        0,  },
				{ column_type::size,             COLFLAGS_NONE,        6,  },
				{ column_type::size_compressed,  COLFLAGS_NONE,        6,  },
				{ column_type::date_write,       COLFLAGS_NONE,        14, },
				{ column_type::date_creation,    COLFLAGS_NONE,        14, },
				{ column_type::date_access,      COLFLAGS_NONE,        14, },
				{ column_type::attributes,       COLFLAGS_NONE,        0,  },
			},
			{
				{ column_type::name,             COLFLAGS_RIGHTALIGN,  0,  },
			},
			PVS_ALIGNEXTENSIONS | PVS_FULLSCREEN,
		},

		// Descriptions
		{
			{
				{ column_type::name,             COLFLAGS_NONE,        40, col_width::percent, },
				{ column_type::description,      COLFLAGS_NONE,        0,  },
			},
			{
				{ column_type::name,             COLFLAGS_RIGHTALIGN,  0,  },
				{ column_type::size,             COLFLAGS_NONE,        6,  },
				{ column_type::date,             COLFLAGS_NONE,        0,  },
				{ column_type::time,             COLFLAGS_NONE,        5,  },
			},
			PVS_ALIGNEXTENSIONS,
		},

		// Long descriptions
		{
			{
				{ column_type::name,             COLFLAGS_NONE,        0,  },
				{ column_type::size,             COLFLAGS_NONE,        6,  },
				{ column_type::description,      COLFLAGS_NONE,        70, col_width::percent, },
			},
			{
				{ column_type::name,             COLFLAGS_RIGHTALIGN,  0,  },
			},
			PVS_ALIGNEXTENSIONS | PVS_FULLSCREEN,
		},

		// File owners
		{
			{
				{ column_type::name,             COLFLAGS_NONE,        0,  },
				{ column_type::size,             COLFLAGS_NONE,        6,  },
				{ column_type::owner,            COLFLAGS_NONE,        15, },
			},
			{
				{ column_type::name,             COLFLAGS_RIGHTALIGN,  0,  },
				{ column_type::size,             COLFLAGS_NONE,        6   },
				{ column_type::date,             COLFLAGS_NONE,        0,  },
				{ column_type::time,             COLFLAGS_NONE,        15, },
			},
			PVS_ALIGNEXTENSIONS,
		},

		// File links
		{
			{
				{ column_type::name,             COLFLAGS_NONE,        0,  },
				{ column_type::size,             COLFLAGS_NONE,        6,  },
				{ column_type::links_number,     COLFLAGS_NONE,        3,  },
			},
			{
				{ column_type::name,             COLFLAGS_RIGHTALIGN,  0,  },
				{ column_type::size,             COLFLAGS_NONE,        6,  },
				{ column_type::date,             COLFLAGS_NONE,        0,  },
				{ column_type::time,             COLFLAGS_NONE,        5,  },
			},
			PVS_ALIGNEXTENSIONS,
		},
	};
	static_assert(std::size(InitialModes) == predefined_panel_modes_count);

	const auto InitMode = [](const auto& src, auto& dst)
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
		std::vector<menu_item> ModeListMenu(MenuCount > predefined_panel_modes_count? MenuCount + 1: MenuCount);

		for (size_t i = 0; i < ViewSettings.size(); ++i)
		{
			ModeListMenu[RealModeToDisplay(i)].Name = ViewSettings[i].Name;
		}

		for (size_t i = 0; i < predefined_panel_modes_count; ++i)
		{
			if (ModeListMenu[i].Name.empty())
				ModeListMenu[i].Name = msg(PredefinedNames[i]);
		}

		if (MenuCount > predefined_panel_modes_count)
		{
			ModeListMenu[predefined_panel_modes_count].Flags = LIF_SEPARATOR;
		}

		int ModeNumber = -1;

		bool AddNewMode = false;
		bool DeleteMode = false;

		ModeListMenu[CurMode].SetSelect(true);
		{
			const auto ModeList = VMenu2::create(msg(lng::MEditPanelModes), ModeListMenu, ScrY - 4);
			ModeList->SetPosition({ -1, -1, 0, 0 });
			ModeList->SetHelp(L"PanelViewModes"sv);
			ModeList->SetMenuFlags(VMENU_WRAPMODE);
			ModeList->SetId(PanelViewModesId);
			ModeList->SetBottomTitle(KeysToLocalizedText(KEY_INS, KEY_DEL, KEY_F4, KEY_CTRLENTER, KEY_CTRLSHIFTENTER));

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
						Global->WindowManager->RefreshWindow(Global->CtrlObject->Panels());
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
			const auto SwitchToAnotherMode = [&](panel_ptr p)
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

			MD_COUNT
		};

		auto ModeDlg = MakeDialogItems<MD_COUNT>(
		{
			{ DI_DOUBLEBOX, {{3,  1 }, {72, 17}}, DIF_NONE, AddNewMode ? L""sv : ModeListMenu[CurMode].Name, },
			{ DI_TEXT,      {{5,  2 }, {0,  2 }}, DIF_NONE, msg(lng::MEditPanelModeName), },
			{ DI_EDIT,      {{5,  3 }, {70, 3 }}, DIF_FOCUS, },
			{ DI_TEXT,      {{5,  4 }, {0,  4 }}, DIF_NONE, msg(lng::MEditPanelModeTypes), },
			{ DI_EDIT,      {{5,  5 }, {35, 5 }}, DIF_NONE, },
			{ DI_TEXT,      {{5,  6 }, {0,  6 }}, DIF_NONE, msg(lng::MEditPanelModeWidths), },
			{ DI_EDIT,      {{5,  7 }, {35, 7 }}, DIF_NONE, },
			{ DI_TEXT,      {{38, 4 }, {0,  4 }}, DIF_NONE, msg(lng::MEditPanelModeStatusTypes), },
			{ DI_EDIT,      {{38, 5 }, {70, 5 }}, DIF_NONE, },
			{ DI_TEXT,      {{38, 6 }, {0,  6 }}, DIF_NONE, msg(lng::MEditPanelModeStatusWidths), },
			{ DI_EDIT,      {{38, 7 }, {70, 7 }}, DIF_NONE, },
			{ DI_TEXT,      {{-1, 8 }, {0,  8 }}, DIF_SEPARATOR, },
			{ DI_CHECKBOX,  {{5,  9 }, {0,  9 }}, DIF_NONE, msg(lng::MEditPanelModeFullscreen), },
			{ DI_CHECKBOX,  {{5,  10}, {0,  10}}, DIF_NONE, msg(lng::MEditPanelModeAlignExtensions), },
			{ DI_CHECKBOX,  {{5,  11}, {0,  11}}, DIF_NONE, msg(lng::MEditPanelModeAlignFolderExtensions), },
			{ DI_CHECKBOX,  {{5,  12}, {0,  12}}, DIF_NONE, msg(lng::MEditPanelModeFoldersUpperCase), },
			{ DI_CHECKBOX,  {{5,  13}, {0,  13}}, DIF_NONE, msg(lng::MEditPanelModeFilesLowerCase), },
			{ DI_CHECKBOX,  {{5,  14}, {0,  14}}, DIF_NONE, msg(lng::MEditPanelModeUpperToLowerCase), },
			{ DI_TEXT,      {{-1, 15}, {0,  15}}, DIF_SEPARATOR, },
			{ DI_BUTTON,    {{0,  16}, {0,  16}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MOk), },
			{ DI_BUTTON,    {{0,  16}, {0,  16}}, DIF_CENTERGROUP | (ModeNumber < static_cast<int>(predefined_panel_modes_count)? DIF_NONE : DIF_DISABLE), msg(lng::MReset), },
			{ DI_BUTTON,    {{0,  16}, {0,  16}}, DIF_CENTERGROUP, msg(lng::MCancel), },
		});

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

			for (const auto& [Mode, Flag]: ModesFlagsMapping)
			{
				ModeDlg[Mode].Selected = CurrentSettings.Flags & Flag? BSTATE_CHECKED : BSTATE_UNCHECKED;
			}

			ModeDlg[MD_EDITNAME].strData = CurrentSettings.Name;
			std::tie(ModeDlg[MD_EDITTYPES].strData, ModeDlg[MD_EDITWIDTHS].strData) = SerialiseViewSettings(CurrentSettings.PanelColumns);
			std::tie(ModeDlg[MD_EDITSTATUSTYPES].strData, ModeDlg[MD_EDITSTATUSWIDTHS].strData) = SerialiseViewSettings(CurrentSettings.StatusColumns);
		}

		int ExitCode;

		{
			const auto Dlg = Dialog::create(ModeDlg);
			Dlg->SetPosition({ -1, -1, 76, 19 });
			Dlg->SetHelp(L"PanelViewModes"sv);
			Dlg->SetId(PanelViewModesEditId);
			Dlg->Process();
			ExitCode=Dlg->GetExitCode();
		}

		if (ExitCode == MD_BUTTON_OK || ExitCode == MD_BUTTON_RESET)
		{
			PanelViewSettings NewSettings;

			if (ExitCode == MD_BUTTON_OK)
			{
				for(const auto& [Mode, Flag]: ModesFlagsMapping)
				{
					if (ModeDlg[Mode].Selected == BSTATE_CHECKED)
						NewSettings.Flags |= Flag;
				}

				NewSettings.Name = ModeDlg[MD_EDITNAME].strData;
				NewSettings.PanelColumns = DeserialiseViewSettings(ModeDlg[MD_EDITTYPES].strData, ModeDlg[MD_EDITWIDTHS].strData);
				NewSettings.StatusColumns = DeserialiseViewSettings(ModeDlg[MD_EDITSTATUSTYPES].strData, ModeDlg[MD_EDITSTATUSWIDTHS].strData);
			}
			else
			{
				ResetViewModes({ &NewSettings, 1 }, ModeNumber);
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
	template<typename option_type, typename default_type>
	FARConfigItem(size_t Root, string_view KeyName, string_view ValueName, option_type& Value, const default_type& Default):
		ApiRoot(Root),
		KeyName(KeyName),
		ValName(ValueName),
		Value(&Value),
		Default(static_cast<typename option_type::underlying_type>(Default))
	{
	}

	size_t ApiRoot;
	string_view KeyName;
	string_view ValName;
	Option* Value;   // адрес переменной, куда помещаем данные
	std::any Default;

	FarListItem MakeListItem(string& ListItemString) const
	{
		FarListItem Item{};

		const auto Type = fit_to_left(string(Value->GetType()), 7);
		ListItemString = pad_right(concat(KeyName, L'.', ValName), 42);

		append(ListItemString, BoxSymbols[BS_V1], Type, BoxSymbols[BS_V1], Value->toString(), Value->ExInfo());
		if(!Value->IsDefault(Default))
		{
			Item.Flags = LIF_CHECKED|L'*';
		}
		Item.Text = ListItemString.c_str();
		Item.UserData = reinterpret_cast<intptr_t>(this);
		return Item;
	}

	bool Edit(bool Hex) const
	{
		DialogBuilder Builder;
		Builder.AddText(concat(KeyName, L'.', ValName, L" ("sv, Value->GetType(), L"):"sv));
		int Result = 0;
		if (!Value->Edit(&Builder, 40, Hex))
		{
			static const lng Buttons[] = { lng::MOk, lng::MReset, lng::MCancel };
			Builder.AddSeparator();
			Builder.AddButtons(Buttons, 0, 2);
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

static bool ParseIntValue(string_view const sValue, long long& iValue)
{
	if (from_string(sValue, iValue))
		return true;

	unsigned long long uValue;
	if (from_string(sValue, uValue))
	{
		iValue = uValue;
		return true;
	}

	if (equal_icase(sValue, L"false"sv))
	{
		iValue = 0;
		return true;
	}

	if (equal_icase(sValue, L"true"sv))
	{
		iValue = 1;
		return true;
	}

	if (equal_icase(sValue, L"other"sv))
	{
		iValue = 2;
		return true;
	}

	// TODO: log
	return false;
}


template<class base_type, class derived>
bool detail::OptionImpl<base_type, derived>::ReceiveValue(const GeneralConfig* Storage, string_view const KeyName, string_view const ValueName, const std::any& Default)
{
	base_type CfgValue;
	const auto Result = Storage->GetValue(KeyName, ValueName, CfgValue);
	Set(Result? CfgValue : std::any_cast<base_type>(Default));
	return Result;
}

template<class base_type, class derived>
void detail::OptionImpl<base_type, derived>::StoreValue(GeneralConfig* Storage, string_view const KeyName, string_view const ValueName, bool always) const
{
	if (always || Changed())
		Storage->SetValue(KeyName, ValueName, Get());
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


string IntOption::toString() const
{
	return str(Get());
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
	return format(FSTR(L" = 0x{0:X}"), as_unsigned(Get()));
}


bool StringOption::Edit(DialogBuilder* Builder, int Width, int Param)
{
	Builder->AddEditField(*this, Width);
	return false;
}

void StringOption::Export(FarSettingsItem& To) const
{
	To.Type = FST_STRING;
	To.String = c_str();
}


class Options::farconfig
{
public:
	NONCOPYABLE(farconfig);
	MOVE_CONSTRUCTIBLE(farconfig);

	using iterator = const FARConfigItem*;
	using const_iterator = iterator;
	using value_type = FARConfigItem;

	farconfig(span<const FARConfigItem> Items, GeneralConfig* cfg):
		m_items(Items),
		m_cfg(cfg)
	{
	}

	[[nodiscard]]
	decltype(auto) begin() const { return m_items.begin(); }
	[[nodiscard]]
	decltype(auto) end() const { return m_items.end(); }
	[[nodiscard]]
	decltype(auto) cbegin() const { return begin(); }
	[[nodiscard]]
	decltype(auto) cend() const { return end(); }
	[[nodiscard]]
	decltype(auto) size() const { return m_items.size(); }
	[[nodiscard]]
	decltype(auto) operator[](size_t i) const { return m_items[i]; }

	[[nodiscard]]
	GeneralConfig* GetConfig() const { return m_cfg; }

private:
	span<const FARConfigItem> m_items;
	GeneralConfig* m_cfg;
};

Options::Options():
	strWordDiv(EdOpt.strWordDiv),
	ViewSettings(m_ViewSettings),
	m_ViewSettings(predefined_panel_modes_count)
{
	const auto& TabSizeValidator = option::validator([](long long TabSize)
	{
		return in_closed_range(1, TabSize, 512)? TabSize : DefaultTabSize;
	});

	EdOpt.TabSize.SetCallback(TabSizeValidator);
	ViOpt.TabSize.SetCallback(TabSizeValidator);

	ViOpt.MaxLineSize.SetCallback(option::validator([](long long Value)
	{
		return Value?
			std::clamp(Value, static_cast<long long>(ViewerOptions::eMinLineSize), static_cast<long long>(ViewerOptions::eMaxLineSize)) :
			static_cast<long long>(ViewerOptions::eDefLineSize);
	}));

	PluginMaxReadData.SetCallback(option::validator([](long long Value) { return std::max(Value, 0x20000ll); }));

	// Исключаем случайное стирание разделителей
	EdOpt.strWordDiv.SetCallback(option::validator([](const string& Value) { return Value.empty()? string(WordDiv0) : Value; }));
	XLat.strWordDivForXlat.SetCallback(option::validator([](const string& Value) { return Value.empty()? string(WordDivForXlat0) : Value; }));

	PanelRightClickRule.SetCallback(option::validator([](long long Value) { return Value %= 3; }));
	PanelCtrlAltShiftRule.SetCallback(option::validator([](long long Value) { return Value %= 3; }));

	HelpTabSize.SetCallback(option::validator([](long long Value) { return DefaultTabSize; })); // пока жестко пропишем...

	const auto MacroKeyValidator = [](const string& Value, unsigned& Key, string_view const DefaultValue, unsigned DefaultKey)
	{
		Key = KeyNameToKey(Value);
		if (!Key)
		{
			Key = DefaultKey;
			return string(DefaultValue);
		}
		return Value;
	};

	Macro.strKeyMacroCtrlDot.SetCallback(option::validator([MacroKeyValidator, this](const string& Value)
	{
		return MacroKeyValidator(Value, Macro.KeyMacroCtrlDot, L"Ctrl."sv, KEY_CTRLDOT);
	}));

	Macro.strKeyMacroRCtrlDot.SetCallback(option::validator([MacroKeyValidator, this](const string& Value)
	{
		return MacroKeyValidator(Value, Macro.KeyMacroRCtrlDot, L"RCtrl."sv, KEY_RCTRLDOT);
	}));

	Macro.strKeyMacroCtrlShiftDot.SetCallback(option::validator([MacroKeyValidator, this](const string& Value)
	{
		return MacroKeyValidator(Value, Macro.KeyMacroCtrlShiftDot, L"CtrlShift."sv, KEY_CTRLSHIFTDOT);
	}));

	Macro.strKeyMacroRCtrlShiftDot.SetCallback(option::validator([MacroKeyValidator, this](const string& Value)
	{
		return MacroKeyValidator(Value, Macro.KeyMacroRCtrlShiftDot, L"RCtrlShift."sv, KEY_RCTRL | KEY_SHIFT | KEY_DOT);
	}));

	ClearType.SetCallback(option::notifier([](int const Value)
	{
		Global->ScrBuf->SetClearTypeFix(Value);
	}));

	Sort.Collation.SetCallback(option::notifier([](auto) { string_sort::adjust_comparer(); }));
	Sort.DigitsAsNumbers.SetCallback(option::notifier([](auto) { string_sort::adjust_comparer(); }));
	Sort.CaseSensitive.SetCallback(option::notifier([](auto) { string_sort::adjust_comparer(); }));

	FormatNumberSeparators.SetCallback(option::notifier([](auto) { locale.invalidate(); }));

	strBoxSymbols.SetCallback(option::notifier([](const string& Value)
	{
		std::copy_n(Value.begin(), std::min(size_t(BS_COUNT), Value.size()), BoxSymbols);
	}));

	VirtualTerminalRendering.SetCallback(option::notifier([](bool const Value)
	{
		console.EnableVirtualTerminal(Value);
	}));

	// По умолчанию - брать плагины из основного каталога
	LoadPlug.MainPluginDir = true;
	LoadPlug.PluginsPersonal = true;
	LoadPlug.PluginsCacheOnly = false;

	Macro.DisableMacro=0;

	ResetViewModes(m_ViewSettings);
}

Options::~Options() = default;

void Options::InitConfigsData()
{
	static constexpr auto DefaultBoxSymbols =
		L"\x2591" L"\x2592" L"\x2593" L"\x2502" L"\x2524" L"\x2561" L"\x2562" L"\x2556"
		L"\x2555" L"\x2563" L"\x2551" L"\x2557" L"\x255D" L"\x255C" L"\x255B" L"\x2510"
		L"\x2514" L"\x2534" L"\x252C" L"\x251C" L"\x2500" L"\x253C" L"\x255E" L"\x255F"
		L"\x255A" L"\x2554" L"\x2569" L"\x2566" L"\x2560" L"\x2550" L"\x256C" L"\x2567"
		L"\x2568" L"\x2564" L"\x2565" L"\x2559" L"\x2558" L"\x2552" L"\x2553" L"\x256B"
		L"\x256A" L"\x2518" L"\x250C" L"\x2588" L"\x2584" L"\x258C" L"\x2590" L"\x2580"
		L" "sv;

	static_assert(DefaultBoxSymbols.size() == BS_COUNT);

	const auto strDefaultLanguage = GetFarIniString(L"General"sv, L"DefaultLanguage"sv, L"English"sv);

	static const FARConfigItem RoamingData[]
	{
		{FSSF_PRIVATE,           NKeyCmdline,                L"AutoComplete"sv,                  CmdLine.AutoComplete, true},
		{FSSF_PRIVATE,           NKeyCmdline,                L"EditBlock"sv,                     CmdLine.EditBlock, false},
		{FSSF_PRIVATE,           NKeyCmdline,                L"DelRemovesBlocks"sv,              CmdLine.DelRemovesBlocks, true},
		{FSSF_PRIVATE,           NKeyCmdline,                L"PromptFormat"sv,                  CmdLine.strPromptFormat, L"$p$g"sv},
		{FSSF_PRIVATE,           NKeyCmdline,                L"UsePromptFormat"sv,               CmdLine.UsePromptFormat, false},
		{FSSF_PRIVATE,           NKeyCodePages,              L"CPMenuMode"sv,                    CPMenuMode, false},
		{FSSF_PRIVATE,           NKeyCodePages,              L"NoAutoDetectCP"sv,                strNoAutoDetectCP, L""sv},
		{FSSF_PRIVATE,           NKeyConfirmations,          L"AllowReedit"sv,                   Confirm.AllowReedit, true},
		{FSSF_CONFIRMATIONS,     NKeyConfirmations,          L"Copy"sv,                          Confirm.Copy, true},
		{FSSF_CONFIRMATIONS,     NKeyConfirmations,          L"Delete"sv,                        Confirm.Delete, true},
		{FSSF_CONFIRMATIONS,     NKeyConfirmations,          L"DeleteFolder"sv,                  Confirm.DeleteFolder, true},
		{FSSF_PRIVATE,           NKeyConfirmations,          L"DetachVHD"sv,                     Confirm.DetachVHD, true},
		{FSSF_CONFIRMATIONS,     NKeyConfirmations,          L"Drag"sv,                          Confirm.Drag, true},
		{FSSF_CONFIRMATIONS,     NKeyConfirmations,          L"Esc"sv,                           Confirm.Esc, true},
		{FSSF_PRIVATE,           NKeyConfirmations,          L"EscTwiceToInterrupt"sv,           Confirm.EscTwiceToInterrupt, false},
		{FSSF_CONFIRMATIONS,     NKeyConfirmations,          L"Exit"sv,                          Confirm.Exit, true},
		{FSSF_CONFIRMATIONS,     NKeyConfirmations,          L"HistoryClear"sv,                  Confirm.HistoryClear, true},
		{FSSF_CONFIRMATIONS,     NKeyConfirmations,          L"Move"sv,                          Confirm.Move, true},
		{FSSF_CONFIRMATIONS,     NKeyConfirmations,          L"RemoveConnection"sv,              Confirm.RemoveConnection, true},
		{FSSF_PRIVATE,           NKeyConfirmations,          L"RemoveHotPlug"sv,                 Confirm.RemoveHotPlug, true},
		{FSSF_PRIVATE,           NKeyConfirmations,          L"RemoveSUBST"sv,                   Confirm.RemoveSUBST, true},
		{FSSF_CONFIRMATIONS,     NKeyConfirmations,          L"RO"sv,                            Confirm.RO, true},
		{FSSF_PRIVATE,           NKeyDescriptions,           L"AnsiByDefault"sv,                 Diz.AnsiByDefault, false},
		{FSSF_PRIVATE,           NKeyDescriptions,           L"ListNames"sv,                     Diz.strListNames, L"Descript.ion,Files.bbs"sv},
		{FSSF_PRIVATE,           NKeyDescriptions,           L"ROUpdate"sv,                      Diz.ROUpdate, false},
		{FSSF_PRIVATE,           NKeyDescriptions,           L"SaveInUTF"sv,                     Diz.SaveInUTF, false},
		{FSSF_PRIVATE,           NKeyDescriptions,           L"SetHidden"sv,                     Diz.SetHidden, true},
		{FSSF_PRIVATE,           NKeyDescriptions,           L"StartPos"sv,                      Diz.StartPos, 0},
		{FSSF_PRIVATE,           NKeyDescriptions,           L"UpdateMode"sv,                    Diz.UpdateMode, DIZ_UPDATE_IF_DISPLAYED},
		{FSSF_PRIVATE,           NKeyDialog,                 L"AutoComplete"sv,                  Dialogs.AutoComplete, true},
		{FSSF_PRIVATE,           NKeyDialog,                 L"CBoxMaxHeight"sv,                 Dialogs.CBoxMaxHeight, 8},
		{FSSF_DIALOG,            NKeyDialog,                 L"EditBlock"sv,                     Dialogs.EditBlock, false},
		{FSSF_PRIVATE,           NKeyDialog,                 L"EditHistory"sv,                   Dialogs.EditHistory, true},
		{FSSF_DIALOG,            NKeyDialog,                 L"DelRemovesBlocks"sv,              Dialogs.DelRemovesBlocks, true},
		{FSSF_DIALOG,            NKeyDialog,                 L"EULBsClear"sv,                    Dialogs.EULBsClear, false},
		{FSSF_PRIVATE,           NKeyDialog,                 L"MouseButton"sv,                   Dialogs.MouseButton, 0xFFFF},
		{FSSF_PRIVATE,           NKeyEditor,                 L"AddUnicodeBOM"sv,                 EdOpt.AddUnicodeBOM, true},
		{FSSF_PRIVATE,           NKeyEditor,                 L"AllowEmptySpaceAfterEof"sv,       EdOpt.AllowEmptySpaceAfterEof,false},
		{FSSF_PRIVATE,           NKeyEditor,                 L"AutoDetectCodePage"sv,            EdOpt.AutoDetectCodePage, true},
		{FSSF_PRIVATE,           NKeyEditor,                 L"AutoIndent"sv,                    EdOpt.AutoIndent, false},
		{FSSF_PRIVATE,           NKeyEditor,                 L"BSLikeDel"sv,                     EdOpt.BSLikeDel, true},
		{FSSF_PRIVATE,           NKeyEditor,                 L"CharCodeBase"sv,                  EdOpt.CharCodeBase, 1},
		{FSSF_PRIVATE,           NKeyEditor,                 L"DefaultCodePage"sv,               EdOpt.DefaultCodePage, encoding::codepage::ansi()},
		{FSSF_PRIVATE,           NKeyEditor,                 L"F8CPs"sv,                         EdOpt.strF8CPs, L""sv},
		{FSSF_PRIVATE,           NKeyEditor,                 L"DelRemovesBlocks"sv,              EdOpt.DelRemovesBlocks, true},
		{FSSF_PRIVATE,           NKeyEditor,                 L"EditOpenedForWrite"sv,            EdOpt.EditOpenedForWrite, true},
		{FSSF_PRIVATE,           NKeyEditor,                 L"EditorCursorBeyondEOL"sv,         EdOpt.CursorBeyondEOL, true},
		{FSSF_PRIVATE,           NKeyEditor,                 L"ExpandTabs"sv,                    EdOpt.ExpandTabs, 0},
		{FSSF_PRIVATE,           NKeyEditor,                 L"ExternalEditorName"sv,            strExternalEditor, L""sv},
		{FSSF_PRIVATE,           NKeyEditor,                 L"FileSizeLimit"sv,                 EdOpt.FileSizeLimit, 0},
		{FSSF_PRIVATE,           NKeyEditor,                 L"KeepEditorEOL"sv,                 EdOpt.KeepEOL, true},
		{FSSF_PRIVATE,           NKeyEditor,                 L"NewFileUnixEOL"sv,                EdOpt.NewFileUnixEOL, false},
		{FSSF_PRIVATE,           NKeyEditor,                 L"SaveSafely"sv,                    EdOpt.SaveSafely, true},
		{FSSF_PRIVATE,           NKeyEditor,                 L"CreateBackups"sv,                 EdOpt.CreateBackups, false},
		{FSSF_PRIVATE,           NKeyEditor,                 L"PersistentBlocks"sv,              EdOpt.PersistentBlocks, false},
		{FSSF_PRIVATE,           NKeyEditor,                 L"ReadOnlyLock"sv,                  EdOpt.ReadOnlyLock, 0},
		{FSSF_PRIVATE,           NKeyEditor,                 L"SaveEditorPos"sv,                 EdOpt.SavePos, true},
		{FSSF_PRIVATE,           NKeyEditor,                 L"SaveEditorShortPos"sv,            EdOpt.SaveShortPos, true},
		{FSSF_PRIVATE,           NKeyEditor,                 L"SearchRegexp"sv,                  EdOpt.SearchRegexp, false},
		{FSSF_PRIVATE,           NKeyEditor,                 L"SearchSelFound"sv,                EdOpt.SearchSelFound, false},
		{FSSF_PRIVATE,           NKeyEditor,                 L"SearchCursorAtEnd"sv,             EdOpt.SearchCursorAtEnd, false},
		{FSSF_PRIVATE,           NKeyEditor,                 L"ShowKeyBar"sv,                    EdOpt.ShowKeyBar, true},
		{FSSF_PRIVATE,           NKeyEditor,                 L"ShowScrollBar"sv,                 EdOpt.ShowScrollBar, false},
		{FSSF_PRIVATE,           NKeyEditor,                 L"ShowTitleBar"sv,                  EdOpt.ShowTitleBar, true},
		{FSSF_PRIVATE,           NKeyEditor,                 L"ShowWhiteSpace"sv,                EdOpt.ShowWhiteSpace, 0},
		{FSSF_PRIVATE,           NKeyEditor,                 L"TabSize"sv,                       EdOpt.TabSize, DefaultTabSize},
		{FSSF_PRIVATE,           NKeyEditor,                 L"UndoDataSize"sv,                  EdOpt.UndoSize, 100*1024*1024},
		{FSSF_PRIVATE,           NKeyEditor,                 L"UseExternalEditor"sv,             EdOpt.UseExternalEditor, false},
		{FSSF_EDITOR,            NKeyEditor,                 L"WordDiv"sv,                       EdOpt.strWordDiv, WordDiv0},
		{FSSF_PRIVATE,           NKeyHelp,                   L"ActivateURL"sv,                   HelpURLRules, 1},
		{FSSF_PRIVATE,           NKeyHelp,                   L"HelpSearchRegexp"sv,              HelpSearchRegexp, false},
		{FSSF_PRIVATE,           NKeyCommandHistory,         L"Count"sv,                         HistoryCount, 1000},
		{FSSF_PRIVATE,           NKeyCommandHistory,         L"Lifetime"sv,                      HistoryLifetime, 90},
		{FSSF_PRIVATE,           NKeyDialogHistory,          L"Count"sv,                         DialogsHistoryCount, 1000},
		{FSSF_PRIVATE,           NKeyDialogHistory,          L"Lifetime"sv,                      DialogsHistoryLifetime, 90},
		{FSSF_PRIVATE,           NKeyFolderHistory,          L"Count"sv,                         FoldersHistoryCount, 1000},
		{FSSF_PRIVATE,           NKeyFolderHistory,          L"Lifetime"sv,                      FoldersHistoryLifetime, 90},
		{FSSF_PRIVATE,           NKeyViewEditHistory,        L"Count"sv,                         ViewHistoryCount, 1000},
		{FSSF_PRIVATE,           NKeyViewEditHistory,        L"Lifetime"sv,                      ViewHistoryLifetime, 90},
		{FSSF_PRIVATE,           NKeyInterface,              L"DelHighlightSelected"sv,          DelOpt.HighlightSelected, true},
		{FSSF_PRIVATE,           NKeyInterface,              L"DelShowSelected"sv,               DelOpt.ShowSelected, 10},
		{FSSF_PRIVATE,           NKeyInterface,              L"DelShowTotal"sv,                  DelOpt.ShowTotal, false},
		{FSSF_PRIVATE,           NKeyInterface,              L"AltF9"sv,                         AltF9, true},
		{FSSF_PRIVATE,           NKeyInterface,              L"VirtualTerminalRendering"sv,      VirtualTerminalRendering, false},
		{FSSF_PRIVATE,           NKeyInterface,              L"ClearType"sv,                     ClearType, true},
		{FSSF_PRIVATE,           NKeyInterface,              L"CopyShowTotal"sv,                 CMOpt.CopyShowTotal, true},
		{FSSF_PRIVATE,           NKeyInterface,              L"CtrlPgUp"sv,                      PgUpChangeDisk, 1},
		{FSSF_PRIVATE,           NKeyInterface,              L"CursorSize1"sv,                   CursorSize[0], 15},
		{FSSF_PRIVATE,           NKeyInterface,              L"CursorSize2"sv,                   CursorSize[1], 10},
		{FSSF_PRIVATE,           NKeyInterface,              L"CursorSize3"sv,                   CursorSize[2], 99},
		{FSSF_PRIVATE,           NKeyInterface,              L"CursorSize4"sv,                   CursorSize[3], 99},
		{FSSF_PRIVATE,           NKeyInterface,              L"EditorTitleFormat"sv,             strEditorTitleFormat, L"%Lng %File"sv},
		{FSSF_PRIVATE,           NKeyInterface,              L"FormatNumberSeparators"sv,        FormatNumberSeparators, L""sv},
		{FSSF_PRIVATE,           NKeyInterface,              L"Mouse"sv,                         Mouse, true},
		{FSSF_PRIVATE,           NKeyInterface,              L"SetIcon"sv,                       SetIcon, false},
		{FSSF_PRIVATE,           NKeyInterface,              L"IconIndex"sv,                     IconIndex, 0},
		{FSSF_PRIVATE,           NKeyInterface,              L"SetAdminIcon"sv,                  SetAdminIcon, true},
		{FSSF_PRIVATE,           NKeyInterface,              L"ShowDotsInRoot"sv,                ShowDotsInRoot, false},
		{FSSF_INTERFACE,         NKeyInterface,              L"ShowMenuBar"sv,                   ShowMenuBar, false},
		{FSSF_PRIVATE,           NKeyInterface,              L"RedrawTimeout"sv,                 RedrawTimeout, 200},
		{FSSF_PRIVATE,           NKeyInterface,              L"TitleAddons"sv,                   strTitleAddons, L"%Ver %Platform %Admin"sv},
		{FSSF_PRIVATE,           NKeyInterface,              L"ViewerTitleFormat"sv,             strViewerTitleFormat, L"%Lng %File"sv},
		{FSSF_PRIVATE,           NKeyInterfaceCompletion,    L"Append"sv,                        AutoComplete.AppendCompletion, false},
		{FSSF_PRIVATE,           NKeyInterfaceCompletion,    L"ModalList"sv,                     AutoComplete.ModalList, false},
		{FSSF_PRIVATE,           NKeyInterfaceCompletion,    L"ShowList"sv,                      AutoComplete.ShowList, true},
		{FSSF_PRIVATE,           NKeyInterfaceCompletion,    L"UseFilesystem"sv,                 AutoComplete.UseFilesystem, 1},
		{FSSF_PRIVATE,           NKeyInterfaceCompletion,    L"UseHistory"sv,                    AutoComplete.UseHistory, 1},
		{FSSF_PRIVATE,           NKeyInterfaceCompletion,    L"UsePath"sv,                       AutoComplete.UsePath, 1},
		{FSSF_PRIVATE,           NKeyInterfaceCompletion,    L"UseEnvironment"sv,                AutoComplete.UseEnvironment, 1},
		{FSSF_PRIVATE,           NKeyLanguage,               L"Main"sv,                          strLanguage, strDefaultLanguage},
		{FSSF_PRIVATE,           NKeyLanguage,               L"Help"sv,                          strHelpLanguage, strDefaultLanguage},
		{FSSF_PRIVATE,           NKeyLayout,                 L"FullscreenHelp"sv,                FullScreenHelp, false},
		{FSSF_PRIVATE,           NKeyLayout,                 L"LeftHeightDecrement"sv,           LeftHeightDecrement, 0},
		{FSSF_PRIVATE,           NKeyLayout,                 L"RightHeightDecrement"sv,          RightHeightDecrement, 0},
		{FSSF_PRIVATE,           NKeyLayout,                 L"WidthDecrement"sv,                WidthDecrement, 0},
		{FSSF_PRIVATE,           NKeyKeyMacros,              L"DateFormat"sv,                    Macro.strDateFormat, L"%a %b %d %H:%M:%S %z %Y"sv},
		{FSSF_PRIVATE,           NKeyKeyMacros,              L"KeyRecordCtrlDot"sv,              Macro.strKeyMacroCtrlDot, L"Ctrl."sv},
		{FSSF_PRIVATE,           NKeyKeyMacros,              L"KeyRecordRCtrlDot"sv,             Macro.strKeyMacroRCtrlDot, L"RCtrl."sv},
		{FSSF_PRIVATE,           NKeyKeyMacros,              L"KeyRecordCtrlShiftDot"sv,         Macro.strKeyMacroCtrlShiftDot, L"CtrlShift."sv},
		{FSSF_PRIVATE,           NKeyKeyMacros,              L"KeyRecordRCtrlShiftDot"sv,        Macro.strKeyMacroRCtrlShiftDot, L"RCtrlShift."sv},
		{FSSF_PRIVATE,           NKeyKeyMacros,              L"ShowPlayIndicator"sv,             Macro.ShowPlayIndicator, true},
		{FSSF_PRIVATE,           NKeyPanel,                  L"AutoUpdateLimit"sv,               AutoUpdateLimit, 0},
		{FSSF_PRIVATE,           NKeyPanel,                  L"CtrlAltShiftRule"sv,              PanelCtrlAltShiftRule, 0},
		{FSSF_PRIVATE,           NKeyPanel,                  L"CtrlFRule"sv,                     PanelCtrlFRule, false},
		{FSSF_PRIVATE,           NKeyPanel,                  L"Highlight"sv,                     Highlight, true},
		{FSSF_PRIVATE,           NKeyPanel,                  L"ReverseSort"sv,                   AllowReverseSort, true},
		{FSSF_PRIVATE,           NKeyPanel,                  L"ReverseSortCharCompat"sv,         ReverseSortCharCompat, false},
		{FSSF_PRIVATE,           NKeyPanel,                  L"RememberLogicalDrives"sv,         RememberLogicalDrives, false},
		{FSSF_PRIVATE,           NKeyPanel,                  L"RightClickRule"sv,                PanelRightClickRule, 2},
		{FSSF_PRIVATE,           NKeyPanel,                  L"SelectFolders"sv,                 SelectFolders, false},
		{FSSF_PRIVATE,           NKeyPanel,                  L"ShellRightLeftArrowsRule"sv,      ShellRightLeftArrowsRule, false},
		{FSSF_PANEL,             NKeyPanel,                  L"ShowBytes"sv,                     ShowBytes, false},
		{FSSF_PANEL,             NKeyPanel,                  L"ShowHidden"sv,                    ShowHidden, true},
		{FSSF_PANEL,             NKeyPanel,                  L"ShortcutAlwaysChdir"sv,           ShortcutAlwaysChdir, false},
		{FSSF_PRIVATE,           NKeyPanel,                  L"SortFolderExt"sv,                 SortFolderExt, false},
		{FSSF_PRIVATE,           NKeyPanel,                  L"RightClickSelect"sv,              RightClickSelect, false},
		{FSSF_PRIVATE,           NKeyPanelInfo,              L"InfoComputerNameFormat"sv,        InfoPanel.ComputerNameFormat, ComputerNamePhysicalNetBIOS},
		{FSSF_PRIVATE,           NKeyPanelInfo,              L"InfoUserNameFormat"sv,            InfoPanel.UserNameFormat, NameUnknown },
		{FSSF_PRIVATE,           NKeyPanelInfo,              L"ShowCDInfo"sv,                    InfoPanel.ShowCDInfo, true},
		{FSSF_PRIVATE,           NKeyPanelInfo,              L"ShowPowerStatus"sv,               InfoPanel.ShowPowerStatus, false},
		{FSSF_PANELLAYOUT,       NKeyPanelLayout,            L"ColumnTitles"sv,                  ShowColumnTitles, true},
		{FSSF_PANELLAYOUT,       NKeyPanelLayout,            L"DetailedJunction"sv,              PanelDetailedJunction, false},
		{FSSF_PRIVATE,           NKeyPanelLayout,            L"FreeInfo"sv,                      ShowPanelFree, false},
		{FSSF_PRIVATE,           NKeyPanelLayout,            L"ScreensNumber"sv,                 ShowScreensNumber, 1},
		{FSSF_PRIVATE,           NKeyPanelLayout,            L"Scrollbar"sv,                     ShowPanelScrollbar, false},
		{FSSF_PRIVATE,           NKeyPanelLayout,            L"ScrollbarMenu"sv,                 ShowMenuScrollbar, true},
		{FSSF_PRIVATE,           NKeyPanelLayout,            L"ShowUnknownReparsePoint"sv,       ShowUnknownReparsePoint, false},
		{FSSF_PANELLAYOUT,       NKeyPanelLayout,            L"SortMode"sv,                      ShowSortMode, true},
		{FSSF_PANELLAYOUT,       NKeyPanelLayout,            L"StatusLine"sv,                    ShowPanelStatus, true},
		{FSSF_PRIVATE,           NKeyPanelLayout,            L"TotalInfo"sv,                     ShowPanelTotals, true},
		{FSSF_PRIVATE,           NKeyPanelLeft,              L"DirectoriesFirst"sv,              LeftPanel.DirectoriesFirst, true},
		{FSSF_PRIVATE,           NKeyPanelLeft,              L"SelectedFirst"sv,                 LeftPanel.SelectedFirst, false},
		{FSSF_PRIVATE,           NKeyPanelLeft,              L"ShortNames"sv,                    LeftPanel.ShowShortNames, false},
		{FSSF_PRIVATE,           NKeyPanelLeft,              L"SortGroups"sv,                    LeftPanel.SortGroups, false},
		{FSSF_PRIVATE,           NKeyPanelLeft,              L"SortMode"sv,                      LeftPanel.SortMode, 1},
		{FSSF_PRIVATE,           NKeyPanelLeft,              L"ReverseSortOrder"sv,              LeftPanel.ReverseSortOrder, false},
		{FSSF_PRIVATE,           NKeyPanelLeft,              L"Type"sv,                          LeftPanel.m_Type, 0},
		{FSSF_PRIVATE,           NKeyPanelLeft,              L"ViewMode"sv,                      LeftPanel.ViewMode, 2},
		{FSSF_PRIVATE,           NKeyPanelLeft,              L"Visible"sv,                       LeftPanel.Visible, true},
		{FSSF_PRIVATE,           NKeyPanelRight,             L"DirectoriesFirst"sv,              RightPanel.DirectoriesFirst, true},
		{FSSF_PRIVATE,           NKeyPanelRight,             L"SelectedFirst"sv,                 RightPanel.SelectedFirst, false},
		{FSSF_PRIVATE,           NKeyPanelRight,             L"ShortNames"sv,                    RightPanel.ShowShortNames, false},
		{FSSF_PRIVATE,           NKeyPanelRight,             L"SortGroups"sv,                    RightPanel.SortGroups, false},
		{FSSF_PRIVATE,           NKeyPanelRight,             L"SortMode"sv,                      RightPanel.SortMode, 1},
		{FSSF_PRIVATE,           NKeyPanelRight,             L"ReverseSortOrder"sv,              RightPanel.ReverseSortOrder, false},
		{FSSF_PRIVATE,           NKeyPanelRight,             L"Type"sv,                          RightPanel.m_Type, 0},
		{FSSF_PRIVATE,           NKeyPanelRight,             L"ViewMode"sv,                      RightPanel.ViewMode, 2},
		{FSSF_PRIVATE,           NKeyPanelRight,             L"Visible"sv,                       RightPanel.Visible, true},
		{FSSF_PRIVATE,           NKeyPanelTree,              L"TurnOffCompletely"sv,             Tree.TurnOffCompletely, true},
		{FSSF_PRIVATE,           NKeyPanelTree,              L"AutoChangeFolder"sv,              Tree.AutoChangeFolder, false},
		{FSSF_PRIVATE,           NKeyPanelTree,              L"MinTreeCount"sv,                  Tree.MinTreeCount, 4},
		{FSSF_PRIVATE,           NKeyPanelTree,              L"TreeFileAttr"sv,                  Tree.TreeFileAttr, FILE_ATTRIBUTE_HIDDEN},
#if defined(TREEFILE_PROJECT)
		{FSSF_PRIVATE,           NKeyPanelTree,              L"CDDisk"sv,                        Tree.CDDisk, 2},
		{FSSF_PRIVATE,           NKeyPanelTree,              L"CDDiskTemplate"sv,                Tree.strCDDisk, CDDiskTemplate},
		{FSSF_PRIVATE,           NKeyPanelTree,              L"ExceptPath"sv,                    Tree.strExceptPath, L""sv},
		{FSSF_PRIVATE,           NKeyPanelTree,              L"LocalDisk"sv,                     Tree.LocalDisk, 2},
		{FSSF_PRIVATE,           NKeyPanelTree,              L"LocalDiskTemplate"sv,             Tree.strLocalDisk, LocalDiskTemplate},
		{FSSF_PRIVATE,           NKeyPanelTree,              L"NetDisk"sv,                       Tree.NetDisk, 2},
		{FSSF_PRIVATE,           NKeyPanelTree,              L"NetPath"sv,                       Tree.NetPath, 2},
		{FSSF_PRIVATE,           NKeyPanelTree,              L"NetDiskTemplate"sv,               Tree.strNetDisk, NetDiskTemplate},
		{FSSF_PRIVATE,           NKeyPanelTree,              L"NetPathTemplate"sv,               Tree.strNetPath, NetPathTemplate},
		{FSSF_PRIVATE,           NKeyPanelTree,              L"RemovableDisk"sv,                 Tree.RemovableDisk, 2},
		{FSSF_PRIVATE,           NKeyPanelTree,              L"RemovableDiskTemplate"sv,         Tree.strRemovableDisk, RemovableDiskTemplate},
		{FSSF_PRIVATE,           NKeyPanelTree,              L"SaveLocalPath"sv,                 Tree.strSaveLocalPath, L""sv},
		{FSSF_PRIVATE,           NKeyPanelTree,              L"SaveNetPath"sv,                   Tree.strSaveNetPath, L""sv},
#endif
		{FSSF_PRIVATE,           NKeyPluginConfirmations,    L"EvenIfOnlyOnePlugin"sv,           PluginConfirm.EvenIfOnlyOnePlugin, false},
		{FSSF_PRIVATE,           NKeyPluginConfirmations,    L"OpenFilePlugin"sv,                PluginConfirm.OpenFilePlugin, 0},
		{FSSF_PRIVATE,           NKeyPluginConfirmations,    L"Prefix"sv,                        PluginConfirm.Prefix, false},
		{FSSF_PRIVATE,           NKeyPluginConfirmations,    L"SetFindList"sv,                   PluginConfirm.SetFindList, false},
		{FSSF_PRIVATE,           NKeyPluginConfirmations,    L"StandardAssociation"sv,           PluginConfirm.StandardAssociation, false},
		{FSSF_PRIVATE,           NKeyPolicies,               L"ShowHiddenDrives"sv,              Policies.ShowHiddenDrives, true},
		{FSSF_PRIVATE,           NKeyScreen,                 L"Clock"sv,                         Clock, true},
		{FSSF_PRIVATE,           NKeyScreen,                 L"DeltaX"sv,                        ScrSize.DeltaX, 0},
		{FSSF_PRIVATE,           NKeyScreen,                 L"DeltaY"sv,                        ScrSize.DeltaY, 0},
		{FSSF_SCREEN,            NKeyScreen,                 L"KeyBar"sv,                        ShowKeyBar, true},
		{FSSF_PRIVATE,           NKeyScreen,                 L"ScreenSaver"sv,                   ScreenSaver, false},
		{FSSF_PRIVATE,           NKeyScreen,                 L"ScreenSaverTime"sv,               ScreenSaverTime, 5},
		{FSSF_PRIVATE,           NKeyScreen,                 L"ViewerEditorClock"sv,             ViewerEditorClock, true},
		{FSSF_PRIVATE,           NKeySystem,                 L"AllCtrlAltShiftRule"sv,           AllCtrlAltShiftRule, 0x0000FFFF},
		{FSSF_PRIVATE,           NKeySystem,                 L"AutoSaveSetup"sv,                 AutoSaveSetup, false},
		{FSSF_PRIVATE,           NKeySystem,                 L"AutoUpdateRemoteDrive"sv,         AutoUpdateRemoteDrive, true},
		{FSSF_PRIVATE,           NKeySystem,                 L"BoxSymbols"sv,                    strBoxSymbols, DefaultBoxSymbols},
		{FSSF_PRIVATE,           NKeySystem,                 L"CASRule"sv,                       CASRule, -1},
		{FSSF_PRIVATE,           NKeySystem,                 L"CmdHistoryRule"sv,                CmdHistoryRule, false},
		{FSSF_PRIVATE,           NKeySystem,                 L"ConsoleDetachKey"sv,              ConsoleDetachKey, L"CtrlShiftTab"sv},
		{FSSF_PRIVATE,           NKeySystem,                 L"CopyBufferSize"sv,                CMOpt.BufferSize, 0},
		{FSSF_SYSTEM,            NKeySystem,                 L"CopyOpened"sv,                    CMOpt.CopyOpened, true},
		{FSSF_PRIVATE,           NKeySystem,                 L"CopyTimeRule"sv,                  CMOpt.CopyTimeRule, 3},
		{FSSF_PRIVATE,           NKeySystem,                 L"CopySecurityOptions"sv,           CMOpt.CopySecurityOptions, 0},
		{FSSF_PRIVATE,           NKeySystem,                 L"CopyPreserveTimestamps"sv,        CMOpt.PreserveTimestamps, false},
		{FSSF_SYSTEM,            NKeySystem,                 L"DeleteToRecycleBin"sv,            DeleteToRecycleBin, true},
		{FSSF_PRIVATE,           NKeySystem,                 L"DriveDisconnectMode"sv,           ChangeDriveDisconnectMode, true},
		{FSSF_PRIVATE,           NKeySystem,                 L"DriveMenuMode"sv,                 ChangeDriveMode, DRIVE_SHOW_TYPE|DRIVE_SHOW_PLUGINS|DRIVE_SHOW_SIZE_FLOAT|DRIVE_SHOW_CDROM},
		{FSSF_PRIVATE,           NKeySystem,                 L"ElevationMode"sv,                 StoredElevationMode, -1},
		{FSSF_PRIVATE,           NKeySystem,                 L"ExcludeCmdHistory"sv,             ExcludeCmdHistory, 0},
		{FSSF_PRIVATE,           NKeySystem,                 L"FileSearchMode"sv,                FindOpt.FileSearchMode, FINDAREA_FROM_CURRENT},
		{FSSF_PRIVATE,           NKeySystem,                 L"FindAlternateStreams"sv,          FindOpt.FindAlternateStreams, false},
		{FSSF_PRIVATE,           NKeySystem,                 L"FindCodePage"sv,                  FindCodePage, CP_DEFAULT},
		{FSSF_PRIVATE,           NKeySystem,                 L"FindFolders"sv,                   FindOpt.FindFolders, true},
		{FSSF_PRIVATE,           NKeySystem,                 L"FindSymLinks"sv,                  FindOpt.FindSymLinks, true},
		{FSSF_PRIVATE,           NKeySystem,                 L"FlagPosixSemantics"sv,            FlagPosixSemantics, true},
		{FSSF_PRIVATE,           NKeySystem,                 L"FolderInfo"sv,                    InfoPanel.strFolderInfoFiles, L"DirInfo,File_Id.diz,Descript.ion,ReadMe.*,Read.Me"sv},
		{FSSF_PRIVATE,           NKeySystem,                 L"MsWheelDelta"sv,                  MsWheelDelta, 1},
		{FSSF_PRIVATE,           NKeySystem,                 L"MsWheelDeltaEdit"sv,              MsWheelDeltaEdit, 1},
		{FSSF_PRIVATE,           NKeySystem,                 L"MsWheelDeltaHelp"sv,              MsWheelDeltaHelp, 1},
		{FSSF_PRIVATE,           NKeySystem,                 L"MsWheelDeltaView"sv,              MsWheelDeltaView, 1},
		{FSSF_PRIVATE,           NKeySystem,                 L"MsHWheelDelta"sv,                 MsHWheelDelta, 1},
		{FSSF_PRIVATE,           NKeySystem,                 L"MsHWheelDeltaEdit"sv,             MsHWheelDeltaEdit, 1},
		{FSSF_PRIVATE,           NKeySystem,                 L"MsHWheelDeltaView"sv,             MsHWheelDeltaView, 1},
		{FSSF_PRIVATE,           NKeySystem,                 L"MultiCopy"sv,                     CMOpt.MultiCopy, false},
		{FSSF_PRIVATE,           NKeySystem,                 L"MultiMakeDir"sv,                  MultiMakeDir, false},
#ifndef NO_WRAPPER
		{FSSF_PRIVATE,           NKeySystem,                 L"OEMPluginsSupport"sv,             LoadPlug.OEMPluginsSupport, true},
#endif // NO_WRAPPER
		{FSSF_SYSTEM,            NKeySystem,                 L"PluginMaxReadData"sv,             PluginMaxReadData, 0x20000},
		{FSSF_PRIVATE,           NKeySystem,                 L"QuotedName"sv,                    QuotedName, QUOTEDNAME_INSERT},
		{FSSF_PRIVATE,           NKeySystem,                 L"QuotedSymbols"sv,                 strQuotedSymbols, L" &()[]{}^=;!'+,`\xA0"sv},
		{FSSF_PRIVATE,           NKeySystem,                 L"SaveHistory"sv,                   SaveHistory, true},
		{FSSF_PRIVATE,           NKeySystem,                 L"SaveFoldersHistory"sv,            SaveFoldersHistory, true},
		{FSSF_PRIVATE,           NKeySystem,                 L"SaveViewHistory"sv,               SaveViewHistory, true},
		{FSSF_SYSTEM,            NKeySystem,                 L"ScanJunction"sv,                  ScanJunction, true},
		{FSSF_PRIVATE,           NKeySystem,                 L"ScanSymlinks"sv,                  LoadPlug.ScanSymlinks, true},
		{FSSF_PRIVATE,           NKeySystem,                 L"SearchInFirstSize"sv,             FindOpt.strSearchInFirstSize, L""sv},
		{FSSF_PRIVATE,           NKeySystem,                 L"SearchOutFormat"sv,               FindOpt.strSearchOutFormat, L"D,S,A"sv},
		{FSSF_PRIVATE,           NKeySystem,                 L"SearchOutFormatWidth"sv,          FindOpt.strSearchOutFormatWidth, L"0,0,0"sv},
		{FSSF_PRIVATE,           NKeySystem,                 L"SetAttrFolderRules"sv,            SetAttrFolderRules, true},
		{FSSF_PRIVATE,           NKeySystem,                 L"ShowCheckingFile"sv,              ShowCheckingFile, false},
		{FSSF_PRIVATE,           NKeySystem,                 L"ShowStatusInfo"sv,                InfoPanel.strShowStatusInfo, L""sv},
		{FSSF_PRIVATE,           NKeySystem,                 L"SmartFolderMonitor"sv,            SmartFolderMonitor, false},
		{FSSF_PRIVATE,           NKeySystem,                 L"SubstNameRule"sv,                 SubstNameRule, 2},
		{FSSF_PRIVATE,           NKeySystem,                 L"SubstPluginPrefix"sv,             SubstPluginPrefix, false},
		{FSSF_PRIVATE,           NKeySystem,                 L"UpdateEnvironment"sv,             UpdateEnvironment, false},
		{FSSF_PRIVATE,           NKeySystem,                 L"UseFilterInSearch"sv,             FindOpt.UseFilter, false},
		{FSSF_PRIVATE,           NKeySystem,                 L"UseRegisteredTypes"sv,            UseRegisteredTypes, true},
		{FSSF_PRIVATE,           NKeySystem,                 L"UseSystemCopy"sv,                 CMOpt.UseSystemCopy, true},
		{FSSF_PRIVATE,           NKeySystem,                 L"WindowMode"sv,                    StoredWindowMode, true},
		{FSSF_PRIVATE,           NKeySystem,                 L"WindowMode.StickyX"sv,            WindowModeStickyX, true},
		{FSSF_PRIVATE,           NKeySystem,                 L"WindowMode.StickyY"sv,            WindowModeStickyY, false},
		{FSSF_PRIVATE,           NKeySystem,                 L"WipeSymbol"sv,                    WipeSymbol, 0},
		{FSSF_SYSTEM,            NKeySystem,                 L"WordDiv"sv,                       strWordDiv, WordDiv0},
		{FSSF_PRIVATE,           NKeySystemSort,             L"Collation"sv,                     Sort.Collation, as_underlying_type(SortingOptions::collation::linguistic)},
		{FSSF_PRIVATE,           NKeySystemSort,             L"DigitsAsNumbers"sv,               Sort.DigitsAsNumbers, IsWindows7OrGreater()},
		{FSSF_PRIVATE,           NKeySystemSort,             L"CaseSensitive"sv,                 Sort.CaseSensitive, false},
		{FSSF_PRIVATE,           NKeySystemKnownIDs,         L"EMenu"sv,                         KnownIDs.Emenu.StrId, KnownIDs.Emenu.Default},
		{FSSF_PRIVATE,           NKeySystemKnownIDs,         L"Network"sv,                       KnownIDs.Network.StrId, KnownIDs.Network.Default},
		{FSSF_PRIVATE,           NKeySystemKnownIDs,         L"Arclite"sv,                       KnownIDs.Arclite.StrId, KnownIDs.Arclite.Default},
		{FSSF_PRIVATE,           NKeySystemKnownIDs,         L"Luamacro"sv,                      KnownIDs.Luamacro.StrId, KnownIDs.Luamacro.Default},
		{FSSF_PRIVATE,           NKeySystemKnownIDs,         L"Netbox"sv,                        KnownIDs.Netbox.StrId, KnownIDs.Netbox.Default},
		{FSSF_PRIVATE,           NKeySystemNowell,           L"MoveRO"sv,                        Nowell.MoveRO, true},
		{FSSF_PRIVATE,           NKeySystemException,        L"FarEventSvc"sv,                   strExceptEventSvc, L""sv},
		{FSSF_PRIVATE,           NKeySystemException,        L"Used"sv,                          ExceptUsed, false},
		{FSSF_PRIVATE,           NKeySystemExecutor,         L"~"sv,                             Exec.strHomeDir, L"%FARHOME%"sv},
		{FSSF_PRIVATE,           NKeySystemExecutor,         L"ExcludeCmds"sv,                   Exec.strExcludeCmds, L""sv},
		{FSSF_PRIVATE,           NKeySystemExecutor,         L"FullTitle"sv,                     Exec.ExecuteFullTitle, false},
		{FSSF_PRIVATE,           NKeySystemExecutor,         L"RestoreCP"sv,                     Exec.RestoreCPAfterExecute, true},
		{FSSF_PRIVATE,           NKeySystemExecutor,         L"UseAppPath"sv,                    Exec.ExecuteUseAppPath, true},
		{FSSF_PRIVATE,           NKeySystemExecutor,         L"UseHomeDir"sv,                    Exec.UseHomeDir, true},
		{FSSF_PRIVATE,           NKeySystemExecutor,         L"Comspec"sv,                       Exec.Comspec, L"%COMSPEC%"sv},
		{FSSF_PRIVATE,           NKeySystemExecutor,         L"ComspecArguments"sv,              Exec.ComspecArguments, L"/S /C \"{0}\""sv},
		{FSSF_PRIVATE,           NKeySystemExecutor,         L"ComspecCondition"sv,              Exec.ComspecCondition, L""sv},
		{FSSF_PRIVATE,           NKeyViewer,                 L"AutoDetectCodePage"sv,            ViOpt.AutoDetectCodePage, true},
		{FSSF_PRIVATE,           NKeyViewer,                 L"DefaultCodePage"sv,               ViOpt.DefaultCodePage, encoding::codepage::ansi()},
		{FSSF_PRIVATE,           NKeyViewer,                 L"DetectDumpMode"sv,                ViOpt.DetectDumpMode, true},
		{FSSF_PRIVATE,           NKeyViewer,                 L"ExternalViewerName"sv,            strExternalViewer, L""sv},
		{FSSF_PRIVATE,           NKeyViewer,                 L"F8CPs"sv,                         ViOpt.strF8CPs, L""sv},
		{FSSF_PRIVATE,           NKeyViewer,                 L"IsWrap"sv,                        ViOpt.ViewerIsWrap, true},
		{FSSF_PRIVATE,           NKeyViewer,                 L"MaxLineSize"sv,                   ViOpt.MaxLineSize, ViewerOptions::eDefLineSize},
		{FSSF_PRIVATE,           NKeyViewer,                 L"PersistentBlocks"sv,              ViOpt.PersistentBlocks, true},
		{FSSF_PRIVATE,           NKeyViewer,                 L"SaveViewerCodepage"sv,            ViOpt.SaveCodepage, true},
		{FSSF_PRIVATE,           NKeyViewer,                 L"SaveViewerPos"sv,                 ViOpt.SavePos, true},
		{FSSF_PRIVATE,           NKeyViewer,                 L"SaveViewerShortPos"sv,            ViOpt.SaveShortPos, true},
		{FSSF_PRIVATE,           NKeyViewer,                 L"SaveViewerWrapMode"sv,            ViOpt.SaveWrapMode, false},
		{FSSF_PRIVATE,           NKeyViewer,                 L"SaveViewMode"sv,                  ViOpt.SaveViewMode, true},
		{FSSF_PRIVATE,           NKeyViewer,                 L"SearchEditFocus"sv,               ViOpt.SearchEditFocus, false},
		{FSSF_PRIVATE,           NKeyViewer,                 L"SearchRegexp"sv,                  ViOpt.SearchRegexp, false},
		{FSSF_PRIVATE,           NKeyViewer,                 L"SearchWrapStop"sv,                ViOpt.SearchWrapStop, BSTATE_CHECKED},
		{FSSF_PRIVATE,           NKeyViewer,                 L"ShowArrows"sv,                    ViOpt.ShowArrows, true},
		{FSSF_PRIVATE,           NKeyViewer,                 L"ShowKeyBar"sv,                    ViOpt.ShowKeyBar, true},
		{FSSF_PRIVATE,           NKeyViewer,                 L"ShowScrollbar"sv,                 ViOpt.ShowScrollbar, false},
		{FSSF_PRIVATE,           NKeyViewer,                 L"TabSize"sv,                       ViOpt.TabSize, DefaultTabSize},
		{FSSF_PRIVATE,           NKeyViewer,                 L"ShowTitleBar"sv,                  ViOpt.ShowTitleBar, true},
		{FSSF_PRIVATE,           NKeyViewer,                 L"UseExternalViewer"sv,             ViOpt.UseExternalViewer, false},
		{FSSF_PRIVATE,           NKeyViewer,                 L"Visible0x00"sv,                   ViOpt.Visible0x00, false},
		{FSSF_PRIVATE,           NKeyViewer,                 L"Wrap"sv,                          ViOpt.ViewerWrap, false},
		{FSSF_PRIVATE,           NKeyViewer,                 L"ZeroChar"sv,                      ViOpt.ZeroChar, L'\xB7'}, // middle dot
		{FSSF_PRIVATE,           NKeyVMenu,                  L"LBtnClick"sv,                     VMenu.LBtnClick, VMENUCLICK_CANCEL},
		{FSSF_PRIVATE,           NKeyVMenu,                  L"MBtnClick"sv,                     VMenu.MBtnClick, VMENUCLICK_APPLY},
		{FSSF_PRIVATE,           NKeyVMenu,                  L"RBtnClick"sv,                     VMenu.RBtnClick, VMENUCLICK_CANCEL},
		{FSSF_PRIVATE,           NKeyXLat,                   L"Flags"sv,                         XLat.Flags, XLAT_SWITCHKEYBLAYOUT|XLAT_CONVERTALLCMDLINE},
		{FSSF_PRIVATE,           NKeyXLat,                   L"Layouts"sv,                       XLat.strLayouts, L""sv},
		{FSSF_PRIVATE,           NKeyXLat,                   L"Rules1"sv,                        XLat.Rules[0], L""sv},
		{FSSF_PRIVATE,           NKeyXLat,                   L"Rules2"sv,                        XLat.Rules[1], L""sv},
		{FSSF_PRIVATE,           NKeyXLat,                   L"Rules3"sv,                        XLat.Rules[2], L""sv},
		{FSSF_PRIVATE,           NKeyXLat,                   L"Table1"sv,                        XLat.Table[0], L""sv},
		{FSSF_PRIVATE,           NKeyXLat,                   L"Table2"sv,                        XLat.Table[1], L""sv},
		{FSSF_PRIVATE,           NKeyXLat,                   L"WordDivForXlat"sv,                XLat.strWordDivForXlat, WordDivForXlat0},
	};

	static const FARConfigItem LocalData[] =
	{
		{FSSF_PRIVATE,           NKeyPanelLeft,              L"CurFile"sv,                       LeftPanel.CurFile, L""sv},
		{FSSF_PRIVATE,           NKeyPanelLeft,              L"Folder"sv,                        LeftPanel.Folder, L""sv},
		{FSSF_PRIVATE,           NKeyPanelRight,             L"CurFile"sv,                       RightPanel.CurFile, L""sv},
		{FSSF_PRIVATE,           NKeyPanelRight,             L"Folder"sv,                        RightPanel.Folder, L""sv},
		{FSSF_PRIVATE,           NKeyPanel,                  L"LeftFocus"sv,                     LeftFocus, true},
	};

	m_Configs.emplace_back(RoamingData, ConfigProvider().GeneralCfg().get());
	m_Configs.emplace_back(LocalData, ConfigProvider().LocalGeneralCfg().get());
}

Options::farconfig& Options::GetConfig(config_type Type)
{
	return m_Configs[static_cast<size_t>(Type)];
}
const Options::farconfig& Options::GetConfig(config_type Type) const
{
	return m_Configs[static_cast<size_t>(Type)];
}

template<class container, class pred>
static const Option* GetConfigValuePtr(const container& Config, const pred& Pred)
{
	const auto ItemIterator = std::find_if(ALL_CONST_RANGE(Config), Pred);
	return ItemIterator == Config.cend()? nullptr : ItemIterator->Value;
}

const Option* Options::GetConfigValue(const string_view Key, const string_view Name) const
{
	// TODO Use local too?
	return GetConfigValuePtr(GetConfig(config_type::roaming), [&](const auto& i) { return equal_icase(i.KeyName, Key) && equal_icase(i.ValName, Name); });
}

const Option* Options::GetConfigValue(size_t Root, const string_view Name) const
{
	if (Root == FSSF_PRIVATE)
		return nullptr;

	// TODO Use local too?
	return GetConfigValuePtr(GetConfig(config_type::roaming), [&](const FARConfigItem& i) { return Root == i.ApiRoot && equal_icase(i.ValName, Name); });
}

void Options::InitConfigs()
{
	if(m_Configs.empty())
	{
		InitConfigsData();
	}
}

void Options::SetSearchColumns(string_view const Columns, string_view const Widths)
{
	if (Columns.empty())
		return;

	FindOpt.OutColumns = DeserialiseViewSettings(Columns, Widths);
	std::tie(FindOpt.strSearchOutFormat, FindOpt.strSearchOutFormatWidth) = SerialiseViewSettings(FindOpt.OutColumns);
}

void Options::SetDriveMenuHotkeys()
{
	if (!ConfigProvider().GeneralCfg()->GetValue<bool>(L"Interface"sv, L"InitDriveMenuHotkeys"sv, true))
		return;

	static constexpr struct
	{
		KnownModulesIDs::UuidOption KnownModulesIDs::* Option;
		UUID MenuId;
		string_view Hotkey;
	}
	DriveMenuHotkeys[]
	{
		{ &KnownModulesIDs::ProcList, "61026851-2643-4C67-BF80-D3C77A3AE830"_uuid, L"0"sv },
		{ &KnownModulesIDs::TmpPanel, "F98C70B3-A1AE-4896-9388-C5C8E05013B7"_uuid, L"1"sv },
		{ &KnownModulesIDs::Netbox,   "C9FB4F53-54B5-48FF-9BA2-E8EB27F012A2"_uuid, L"2"sv },
		{ &KnownModulesIDs::Network,  "24B6DD41-DF12-470A-A47C-8675ED8D2ED4"_uuid, L"3"sv },
	};

	for (const auto& i: DriveMenuHotkeys)
	{
		ConfigProvider().PlHotkeyCfg()->SetHotkey(std::invoke(i.Option, KnownIDs).StrId, i.MenuId, hotkey_type::drive_menu, i.Hotkey);
	}

	ConfigProvider().GeneralCfg()->SetValue(L"Interface"sv, L"InitDriveMenuHotkeys"sv, false);
}

static std::optional<std::pair<panel_sort, sort_order>> deserialise_sort_layer(string_view const LayerStr)
{
	int Sort = -1;
	int Order = -1;

	for (const auto& Str: enum_tokens(LayerStr, L":"sv))
	{
		switch (Str.front())
		{
		case L'S':
			if (!from_string(Str.substr(1), Sort) || !in_closed_range(0, Sort, static_cast<int>(panel_sort::COUNT)))
				return {};
			break;

		case L'O':
			if (!from_string(Str.substr(1), Order) || !in_closed_range(static_cast<int>(sort_order::first), Order, static_cast<int>(sort_order::last)))
				return {};
			break;
		}
	}

	if (Sort == -1 || Order == -1)
		return {};

	return std::pair(panel_sort{ Sort }, sort_order{ Order });
}

static auto deserialise_sort_layers(string_view const LayersStr)
{
	std::vector<std::pair<panel_sort, sort_order>> Layers;

	for (const auto& Str: enum_tokens(LayersStr, L" "sv))
	{
		if (const auto Layer = deserialise_sort_layer(Str); Layer && !contains(Layers, *Layer))
			Layers.emplace_back(*Layer);
	}

	return Layers;
}

static auto serialise_sort_layer(std::pair<panel_sort, sort_order> const& Layer)
{
	return format(FSTR(L"S{0}:O{1}"), Layer.first, Layer.second);
}

static auto serialise_sort_layers(span<std::pair<panel_sort, sort_order> const> const Layers)
{
	return join(select(Layers, serialise_sort_layer), L" "sv);
}

void Options::ReadSortLayers()
{
	PanelSortLayers.resize(static_cast<size_t>(panel_sort::COUNT));

	for (auto& [Layers, i]: enumerate(PanelSortLayers))
	{
		string LayersStr;
		if (ConfigProvider().GeneralCfg()->GetValue(NKeyPanelSortLayers, str(i), LayersStr))
			Layers = deserialise_sort_layers(LayersStr);

		if (Layers.empty())
		{
			const auto DefaultLayers = default_sort_layers(static_cast<panel_sort>(i));
			Layers.assign(ALL_CONST_RANGE(DefaultLayers));
		}
	}
}

void Options::SaveSortLayers(bool const Always)
{
	auto& Cfg = *ConfigProvider().GeneralCfg();

	for (const auto& [Layers, i]: enumerate(PanelSortLayers))
	{
		const auto DefaultLayers = default_sort_layers(static_cast<panel_sort>(i));
		if (std::equal(ALL_CONST_RANGE(Layers), ALL_CONST_RANGE(DefaultLayers)))
		{
			Cfg.DeleteValue(NKeyPanelSortLayers, str(i));
			continue;
		}

		const auto LayersStr = serialise_sort_layers(Layers);
		if ( Always || (LayersStr != Cfg.GetValue<string>(NKeyPanelSortLayers, str(i))))
			Cfg.SetValue(NKeyPanelSortLayers, str(i), LayersStr);
	}
}

void Options::Load(overrides&& Overrides)
{
	// KnownModulesIDs::UuidOption::Default pointer is used in the static config structure, so it MUST be initialized before calling InitConfig()
	static struct
	{
		KnownModulesIDs::UuidOption KnownModulesIDs::* Option;
		const UUID& Id;
		string StrId;
	}
	DefaultKnownIds[]
	{
		{ &KnownModulesIDs::Network,  NetworkId,  },
		{ &KnownModulesIDs::Emenu,    EMenuId,    },
		{ &KnownModulesIDs::Arclite,  ArcliteId,  },
		{ &KnownModulesIDs::Luamacro, LuamacroId, },
		{ &KnownModulesIDs::Netbox,   NetBoxId,   },
		{ &KnownModulesIDs::ProcList, ProcListId, },
		{ &KnownModulesIDs::TmpPanel, TmpPanelId, },
	};

	static_assert(std::size(DefaultKnownIds) == sizeof(KnownModulesIDs) / sizeof(KnownModulesIDs::UuidOption));

	for(auto& i: DefaultKnownIds)
	{
		i.StrId = uuid::str(i.Id);

		auto& UuidOption = std::invoke(i.Option, KnownIDs);
		UuidOption.Id = i.Id;
		UuidOption.StrId = i.StrId;
		UuidOption.Default = i.StrId;
	}

	InitConfigs();

	const auto GetOverride = [&](const FARConfigItem& Item)
	{
		if (Overrides.empty())
			return false;

		const auto ItemIterator = Overrides.find(concat(Item.KeyName, L'.', Item.ValName));
		if (ItemIterator == Overrides.end())
			return false;

		const auto Result = Item.Value->TryParse(ItemIterator->second);
		// We need it only once
		Overrides.erase(ItemIterator);
		return Result;
	};


	for (auto& Config: m_Configs)
	{
		const auto ConfigStorage = Config.GetConfig();
		for(const auto& Item: Config)
		{
			GetOverride(Item) || Item.Value->ReceiveValue(ConfigStorage, Item.KeyName, Item.ValName, Item.Default);
		}
	}

	/* <ПОСТПРОЦЕССЫ> *************************************************** */

	Palette.Load();
	GlobalUserMenuDir = ConvertNameToFull(os::env::expand(GetFarIniString(L"General"sv, L"GlobalUserMenuDir"sv, Global->g_strFarPath)));
	AddEndSlash(GlobalUserMenuDir);

	if(WindowMode == -1)
	{
		WindowMode = StoredWindowMode;
	}

	console.EnableWindowMode(WindowMode != 0);

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

	{
		const auto Enumerator = ConfigProvider().GeneralCfg()->ValuesEnumerator<string>(L"Masks"sv);
		if (Enumerator.cbegin() == Enumerator.cend())
			ApplyDefaultMaskGroups();
	}

	for (auto& [Ptr, Id, Str]: DefaultKnownIds)
	{
		auto& UuidOption = std::invoke(Ptr, KnownIDs);

		if (const auto Result = uuid::try_parse(UuidOption.StrId.Get()))
			UuidOption.Id = *Result;
		else
			UuidOption.Id = {};
	}

	SetDriveMenuHotkeys();

	ReadSortLayers();

/* *************************************************** </ПОСТПРОЦЕССЫ> */

	// we assume that any changes after this point will be made by the user
	for (auto& i: m_Configs)
		for (auto& j: i)
			j.Value->MakeUnchanged();
}

void Options::Save(bool Manual)
{
	InitConfigs();

	if (Manual && Message(0,
		msg(lng::MSaveSetupTitle),
		{
			msg(lng::MSaveSetupAsk1),
			msg(lng::MSaveSetupAsk2)
		},
		{ lng::MSaveSetup, lng::MCancel }) != Message::first_button)
		return;

	/* <ПРЕПРОЦЕССЫ> *************************************************** */

	const auto StorePanelOptions = [](panel_ptr PanelPtr, PanelOptions& Panel)
	{
		if (PanelPtr->GetMode() == panel_mode::NORMAL_PANEL)
		{
			Panel.m_Type = static_cast<int>(PanelPtr->GetType());
			Panel.ViewMode = PanelPtr->GetViewMode();
			Panel.SortMode = static_cast<int>(PanelPtr->GetSortMode());
			Panel.ReverseSortOrder = PanelPtr->GetSortOrder();
			Panel.SortGroups = PanelPtr->GetSortGroups();
			Panel.ShowShortNames = PanelPtr->GetShowShortNamesMode();
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

	for (const auto& Config: m_Configs)
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
	SaveSortLayers(Manual);
	Global->CtrlObject->Macro.SaveMacros(Manual);
}

enum
{
	ac_item_listbox,

	ac_count
};

intptr_t Options::AdvancedConfigDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	const auto GetConfigItem = [Dlg](int Index)
	{
		return Index == -1 ?
			nullptr : // Everything is filtered out
			reinterpret_cast<const FARConfigItem*>(Dlg->GetListItemSimpleUserData(0, Index));
	};

	const auto EditItem = [&](bool Hex = false)
	{
		FarListInfo ListInfo{ sizeof(ListInfo) };
		Dlg->SendMessage(DM_LISTINFO, ac_item_listbox, &ListInfo);

		const auto CurrentItem = GetConfigItem(ListInfo.SelectPos);
		if (!CurrentItem)
			return;

		if (!CurrentItem->Edit(Hex))
			return;

		SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);

		auto& ConfigStrings = *reinterpret_cast<std::vector<string>*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
		FarListUpdate flu{ sizeof(flu), ListInfo.SelectPos, CurrentItem->MakeListItem(ConfigStrings[ListInfo.SelectPos]) };
		Dlg->SendMessage(DM_LISTUPDATE, ac_item_listbox, &flu);

		FarListPos flp{ sizeof(flp), ListInfo.SelectPos, ListInfo.TopPos };
		Dlg->SendMessage(DM_LISTSETCURPOS, ac_item_listbox, &flp);
	};

	switch (Msg)
	{
	case DN_INITDIALOG:
		{
			SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);

			FarListTitles Titles{ sizeof(Titles) };

			const auto BottomTitle = KeysToLocalizedText(KEY_SHIFTF1, KEY_F4, KEY_SHIFTF4, KEY_CTRLH);
			Titles.Title = msg(lng::MConfigEditor).c_str();
			Titles.Bottom = BottomTitle.c_str();
			Dlg->SendMessage(DM_LISTSETTITLES, ac_item_listbox, &Titles);

			Dlg->SendMessage(DM_LISTSORT, ac_item_listbox, nullptr);
		}
		break;

	case DN_RESIZECONSOLE:
		{
			SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);

			COORD Size{ static_cast<short>(std::max(ScrX - 4, 60)), static_cast<short>(std::max(ScrY - 2, 20)) };
			Dlg->SendMessage(DM_RESIZEDIALOG, 0, &Size);
			SMALL_RECT ListPos{ 3, 1, static_cast<short>(Size.X - 4), static_cast<short>(Size.Y - 2) };
			Dlg->SendMessage(DM_SETITEMPOSITION, ac_item_listbox, &ListPos);
		}
		break;

	case DN_CONTROLINPUT:
		{
			const auto record = static_cast<const INPUT_RECORD*>(Param2);
			if (Param1 == ac_item_listbox && record->EventType==KEY_EVENT)
			{
				switch (InputRecordToKey(record))
				{
				case KEY_SHIFTF1:
					{
						FarListInfo ListInfo = {sizeof(ListInfo)};
						Dlg->SendMessage(DM_LISTINFO, Param1, &ListInfo);

						if (const auto CurrentItem = GetConfigItem(ListInfo.SelectPos))
						{
							if (!help::show(concat(CurrentItem->KeyName, L'.', CurrentItem->ValName), {}, FHELP_NOSHOWERROR))
							{
								help::show(concat(CurrentItem->KeyName, L"Settings"sv), {}, FHELP_NOSHOWERROR);
							}
						}
					}
					break;

				case KEY_F4:
					EditItem();
					break;

				case KEY_SHIFTF4:
					EditItem(true);
					break;

				case KEY_CTRLH:
				case KEY_RCTRLH:
					{
						SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);

						static bool HideUnchanged = true;

						FarListInfo ListInfo = {sizeof(ListInfo)};
						Dlg->SendMessage(DM_LISTINFO, Param1, &ListInfo);
						for(int i = 0; i < static_cast<int>(ListInfo.ItemsNumber); ++i)
						{
							FarListGetItem Item={sizeof(FarListGetItem), i};

							// BUGBUG(?) DM_LISTGETITEM will return false if everything is filtered out
							if (!Dlg->SendMessage(DM_LISTGETITEM, Param1, &Item))
								continue;

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
								Dlg->SendMessage(DM_LISTUPDATE, Param1, &UpdatedItem);
							}
						}
						HideUnchanged = !HideUnchanged;
					}
					break;
				}
			}
		}
		break;

	case DN_CLOSE:
		if (Param1 == ac_item_listbox)
		{
			EditItem(false);
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
	auto& CurrentConfig = GetConfig(Mode);

	const int DlgWidth = std::max(ScrX-4, 60), DlgHeight = std::max(ScrY-2, 20);

	auto AdvancedConfigDlg = MakeDialogItems<ac_count>(
	{
		{ DI_LISTBOX, {{3, 1}, {DlgWidth-4, DlgHeight-2}}, DIF_NONE, L"far:config"sv },
	});

	std::vector<FarListItem> items;
	items.reserve(CurrentConfig.size());
	std::vector<string> Strings(CurrentConfig.size());
	const auto ConfigData = zip(CurrentConfig, Strings);
	std::transform(ALL_CONST_RANGE(ConfigData), std::back_inserter(items), [](const auto& i) { return std::get<0>(i).MakeListItem(std::get<1>(i)); });

	FarList Items={sizeof(FarList), items.size(), items.data()};

	AdvancedConfigDlg[ac_item_listbox].ListItems = &Items;

	const auto Dlg = Dialog::create(AdvancedConfigDlg, &Options::AdvancedConfigDlgProc, &Strings);
	Dlg->SetHelp(L"FarConfig"sv);
	Dlg->SetPosition({ -1, -1, DlgWidth, DlgHeight });
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

static const auto CustomModesKeyName = L"CustomModes"sv;
static const auto ModesNameName = L"Name"sv;
static const auto ModesColumnTitlesName = L"ColumnTitles"sv;
static const auto ModesColumnWidthsName = L"ColumnWidths"sv;
static const auto ModesStatusColumnTitlesName = L"StatusColumnTitles"sv;
static const auto ModesStatusColumnWidthsName = L"StatusColumnWidths"sv;
static const auto ModesFlagsName = L"Flags"sv;

void Options::ReadPanelModes()
{
	const auto cfg = ConfigProvider().CreatePanelModesConfig();

	const auto ReadMode = [&](const HierarchicalConfig::key& Key, PanelViewSettings& i)
	{
		i.Name = cfg->GetValue<string>(Key, ModesNameName);
		i.Flags = cfg->GetValue<unsigned long long>(Key, ModesFlagsName);

		const auto ColumnTitles = cfg->GetValue<string>(Key, ModesColumnTitlesName);
		if (!ColumnTitles.empty())
		{
			const auto ColumnWidths = cfg->GetValue<string>(Key, ModesColumnWidthsName);
			i.PanelColumns = DeserialiseViewSettings(ColumnTitles, ColumnWidths);
		}

		const auto StatusColumnTitles = cfg->GetValue<string>(Key, ModesStatusColumnTitlesName);
		if (!StatusColumnTitles.empty())
		{
			const auto StatusColumnWidths = cfg->GetValue<string>(Key, ModesStatusColumnWidthsName);
			i.StatusColumns = DeserialiseViewSettings(StatusColumnTitles, StatusColumnWidths);
		}
	};

	for (auto& [Item, Index]: enumerate(span(m_ViewSettings).subspan(0, predefined_panel_modes_count)))
	{
		if (const auto Key = cfg->FindByName(cfg->root_key, str(Index)))
			ReadMode(Key, Item);
	}

	if (const auto CustomModesRoot = cfg->FindByName(cfg->root_key, CustomModesKeyName))
	{
		for (const auto& Key: cfg->KeysEnumerator(CustomModesRoot))
		{
			PanelViewSettings NewSettings;
			ReadMode(Key, NewSettings);
			m_ViewSettings.emplace_back(std::move(NewSettings));
		}
	}

	m_ViewSettingsChanged = false;
}


void Options::SavePanelModes(bool always)
{
	if (!always && !m_ViewSettingsChanged)
		return;

	const auto cfg = ConfigProvider().CreatePanelModesConfig();

	SCOPED_ACTION(auto)(cfg->ScopedTransaction());

	const auto SaveMode = [&](HierarchicalConfig::key const ModesKey, PanelViewSettings const& Item, size_t Index)
	{
		const auto [PanelTitles, PanelWidths] = SerialiseViewSettings(Item.PanelColumns);
		const auto [StatusTitles, StatusWidths] = SerialiseViewSettings(Item.StatusColumns);

		const auto Key = cfg->CreateKey(ModesKey, str(Index));

		cfg->SetValue(Key, ModesNameName, Item.Name);
		cfg->SetValue(Key, ModesColumnTitlesName, PanelTitles);
		cfg->SetValue(Key, ModesColumnWidthsName, PanelWidths);
		cfg->SetValue(Key, ModesStatusColumnTitlesName, StatusTitles);
		cfg->SetValue(Key, ModesStatusColumnWidthsName, StatusWidths);
		cfg->SetValue(Key, ModesFlagsName, Item.Flags);
	};

	for (const auto& [Value, Index]: enumerate(span(ViewSettings).subspan(0, predefined_panel_modes_count)))
	{
		SaveMode(cfg->root_key, Value, Index);
	}

	if (const auto ModesKey = cfg->FindByName(cfg->root_key, CustomModesKeyName))
	{
		cfg->DeleteKeyTree(ModesKey);
	}

	const auto ModesKey = cfg->CreateKey(cfg->root_key, CustomModesKeyName);

	for (const auto& [Value, Index]: enumerate(span(ViewSettings).subspan(predefined_panel_modes_count)))
	{
		SaveMode(ModesKey, Value, Index);
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

static void SetLeftRightMenuChecks(menu_item* pMenu, bool bLeft)
{
	const auto pPanel = bLeft? Global->CtrlObject->Cp()->LeftPanel() : Global->CtrlObject->Cp()->RightPanel();

	switch (pPanel->GetType())
	{
	case panel_type::FILE_PANEL:
		pMenu[RealModeToDisplay(pPanel->GetViewMode())].SetCheck();
		break;

	case panel_type::INFO_PANEL:
		pMenu[MENU_LEFT_INFOPANEL].SetCheck();
		break;

	case panel_type::TREE_PANEL:
		pMenu[MENU_LEFT_TREEPANEL].SetCheck();
		break;

	case panel_type::QVIEW_PANEL:
		pMenu[MENU_LEFT_QUICKVIEW].SetCheck();
		break;
	}

	pPanel->GetShowShortNamesMode()? pMenu[MENU_LEFT_LONGNAMES].ClearCheck() : pMenu[MENU_LEFT_LONGNAMES].SetCheck();
}

void Options::ShellOptions(bool LastCommand, const MOUSE_EVENT_RECORD *MouseEvent)
{
	const auto ApplyViewModesNames = [this](menu_item* Menu)
	{
		for (size_t i = 0; i < predefined_panel_modes_count; ++i)
		{
			if (!ViewSettings[i].Name.empty())
				Menu[RealModeToDisplay(i)].Name = ViewSettings[i].Name;
		}
	};

	const auto no_tree = Tree.TurnOffCompletely? LIF_HIDDEN : LIF_NONE;

	menu_item LeftMenu[]
	{
		{ msg(lng::MMenuBriefView), LIF_SELECTED, KEY_CTRL1 },
		{ msg(lng::MMenuMediumView), 0, KEY_CTRL2 },
		{ msg(lng::MMenuFullView), 0, KEY_CTRL3 },
		{ msg(lng::MMenuWideView), 0, KEY_CTRL4 },
		{ msg(lng::MMenuDetailedView), 0, KEY_CTRL5 },
		{ msg(lng::MMenuDizView), 0, KEY_CTRL6 },
		{ msg(lng::MMenuLongDizView), 0, KEY_CTRL7 },
		{ msg(lng::MMenuOwnersView), 0, KEY_CTRL8 },
		{ msg(lng::MMenuLinksView), 0, KEY_CTRL9 },
		{ msg(lng::MMenuAlternativeView), 0, KEY_CTRL0 },
		{ {}, LIF_SEPARATOR },
		{ msg(lng::MMenuInfoPanel), 0, KEY_CTRLL },
		{ msg(lng::MMenuTreePanel), no_tree, KEY_CTRLT },
		{ msg(lng::MMenuQuickView), 0, KEY_CTRLQ },
		{ {}, LIF_SEPARATOR },
		{ msg(lng::MMenuSortModes), 0, KEY_CTRLF12 },
		{ msg(lng::MMenuLongNames), 0, KEY_CTRLN },
		{ msg(lng::MMenuTogglePanel), 0, KEY_CTRLF1 },
		{ msg(lng::MMenuReread), 0, KEY_CTRLR },
		{ msg(lng::MMenuChangeDrive), 0, KEY_ALTF1 },
	};
	ApplyViewModesNames(LeftMenu);
	const auto LeftMenuStrings = VMenu::AddHotkeys(LeftMenu);

	menu_item FilesMenu[]
	{
		{ msg(lng::MMenuView), LIF_SELECTED, KEY_F3 },
		{ msg(lng::MMenuEdit), 0, KEY_F4 },
		{ msg(lng::MMenuCopy), 0, KEY_F5 },
		{ msg(lng::MMenuMove), 0, KEY_F6 },
		{ msg(lng::MMenuLink), 0, KEY_ALTF6 },
		{ msg(lng::MMenuCreateFolder), 0, KEY_F7 },
		{ msg(lng::MMenuDelete), 0, KEY_F8 },
		{ msg(lng::MMenuWipe), 0, KEY_ALTDEL },
		{ {}, LIF_SEPARATOR },
		{ msg(lng::MMenuAdd), 0, KEY_SHIFTF1 },
		{ msg(lng::MMenuExtract), 0, KEY_SHIFTF2 },
		{ msg(lng::MMenuArchiveCommands), 0, KEY_SHIFTF3 },
		{ {}, LIF_SEPARATOR },
		{ msg(lng::MMenuAttributes), 0, KEY_CTRLA },
		{ msg(lng::MMenuApplyCommand), 0, KEY_CTRLG },
		{ msg(lng::MMenuDescribe), 0, KEY_CTRLZ },
		{ {}, LIF_SEPARATOR },
		{ msg(lng::MMenuSelectGroup), 0, KEY_ADD },
		{ msg(lng::MMenuUnselectGroup), 0, KEY_SUBTRACT },
		{ msg(lng::MMenuInvertSelection), 0, KEY_MULTIPLY },
		{ msg(lng::MMenuRestoreSelection), 0, KEY_CTRLM },
	};
	const auto FilesMenuStrings = VMenu::AddHotkeys(FilesMenu);

	menu_item CmdMenu[]
	{
		{ msg(lng::MMenuFindFile), LIF_SELECTED, KEY_ALTF7 },
		{ msg(lng::MMenuHistory), 0, KEY_ALTF8 },
		{ msg(lng::MMenuVideoMode), 0, KEY_ALTF9 },
		{ msg(lng::MMenuFindFolder), no_tree, KEY_ALTF10 },
		{ msg(lng::MMenuViewHistory), 0, KEY_ALTF11 },
		{ msg(lng::MMenuFoldersHistory), 0, KEY_ALTF12 },
		{ {}, LIF_SEPARATOR, 0 },
		{ msg(lng::MMenuSwapPanels), 0, KEY_CTRLU },
		{ msg(lng::MMenuTogglePanels), 0, KEY_CTRLO },
		{ msg(lng::MMenuCompareFolders), 0, 0 },
		{ {}, LIF_SEPARATOR, 0 },
		{ msg(lng::MMenuUserMenu), 0, 0 },
		{ msg(lng::MMenuFileAssociations), 0, 0 },
		{ msg(lng::MMenuFolderShortcuts), 0, 0 },
		{ msg(lng::MMenuFilter), 0, KEY_CTRLI },
		{ {}, LIF_SEPARATOR, 0 },
		{ msg(lng::MMenuPluginCommands), 0, KEY_F11 },
		{ msg(lng::MMenuWindowsList), 0, KEY_F12 },
		{ msg(lng::MMenuProcessList), 0, KEY_CTRLW },
		{ msg(lng::MMenuHotPlugList), 0, 0 },
	};
	const auto CmdMenuStrings = VMenu::AddHotkeys(CmdMenu);

	menu_item OptionsMenu[]
	{
		{ msg(lng::MMenuSystemSettings), LIF_SELECTED },
		{ msg(lng::MMenuPanelSettings), 0 },
		{ msg(lng::MMenuTreeSettings), no_tree},
		{ msg(lng::MMenuInterface), 0 },
		{ msg(lng::MMenuLanguages), 0 },
		{ msg(lng::MMenuPluginsConfig), 0 },
		{ msg(lng::MMenuPluginsManagerSettings), 0 },
		{ msg(lng::MMenuDialogSettings), 0 },
		{ msg(lng::MMenuVMenuSettings), 0 },
		{ msg(lng::MMenuCmdlineSettings), 0 },
		{ msg(lng::MMenuAutoCompleteSettings), 0 },
		{ msg(lng::MMenuInfoPanelSettings), 0 },
		{ msg(lng::MMenuMaskGroups), 0 },
		{ {}, LIF_SEPARATOR },
		{ msg(lng::MMenuConfirmation), 0 },
		{ msg(lng::MMenuFilePanelModes), 0 },
		{ msg(lng::MMenuFileDescriptions), 0 },
		{ msg(lng::MMenuFolderInfoFiles), 0 },
		{ {}, LIF_SEPARATOR },
		{ msg(lng::MMenuViewer), 0 },
		{ msg(lng::MMenuEditor), 0 },
		{ msg(lng::MMenuCodePages), 0 },
		{ {}, LIF_SEPARATOR },
		{ msg(lng::MMenuColors), 0 },
		{ msg(lng::MMenuFilesHighlighting), 0 },
		{ {}, LIF_SEPARATOR },
		{ msg(lng::MMenuSaveSetup), 0, KEY_SHIFTF9 },
	};
	const auto OptionsMenuStrings = VMenu::AddHotkeys(OptionsMenu);

	menu_item RightMenu[]=
	{
		{ msg(lng::MMenuBriefView), LIF_SELECTED, KEY_CTRL1 },
		{ msg(lng::MMenuMediumView), 0, KEY_CTRL2 },
		{ msg(lng::MMenuFullView), 0, KEY_CTRL3 },
		{ msg(lng::MMenuWideView), 0, KEY_CTRL4 },
		{ msg(lng::MMenuDetailedView), 0, KEY_CTRL5 },
		{ msg(lng::MMenuDizView), 0, KEY_CTRL6 },
		{ msg(lng::MMenuLongDizView), 0, KEY_CTRL7 },
		{ msg(lng::MMenuOwnersView), 0, KEY_CTRL8 },
		{ msg(lng::MMenuLinksView), 0, KEY_CTRL9 },
		{ msg(lng::MMenuAlternativeView), 0, KEY_CTRL0 },
		{ {}, LIF_SEPARATOR },
		{ msg(lng::MMenuInfoPanel), 0, KEY_CTRLL },
		{ msg(lng::MMenuTreePanel), no_tree, KEY_CTRLT },
		{ msg(lng::MMenuQuickView), 0, KEY_CTRLQ },
		{ {}, LIF_SEPARATOR },
		{ msg(lng::MMenuSortModes), 0, KEY_CTRLF12 },
		{ msg(lng::MMenuLongNames), 0, KEY_CTRLN },
		{ msg(lng::MMenuTogglePanelRight), 0, KEY_CTRLF2 },
		{ msg(lng::MMenuReread), 0, KEY_CTRLR },
		{ msg(lng::MMenuChangeDriveRight), 0, KEY_ALTF2 },
	};
	ApplyViewModesNames(RightMenu);
	const auto RightMenuStrings = VMenu::AddHotkeys(RightMenu);


	HMenuData MainMenu[]
	{
		{ msg(lng::MMenuLeftTitle), L"LeftRightMenu"sv, LeftMenu, true },
		{ msg(lng::MMenuFilesTitle), L"FilesMenu"sv, FilesMenu },
		{ msg(lng::MMenuCommandsTitle), L"CmdMenu"sv, CmdMenu },
		{ msg(lng::MMenuOptionsTitle), L"OptMenu"sv, OptionsMenu },
		{ msg(lng::MMenuRightTitle), L"LeftRightMenu"sv, RightMenu },
	};
	static int LastHItem=-1,LastVItem=0;
	int HItem,VItem;

	SetLeftRightMenuChecks(LeftMenu, true);
	SetLeftRightMenuChecks(RightMenu, false);
	// Навигация по меню
	{
		const auto HOptMenu = HMenu::create(MainMenu, std::size(MainMenu));
		HOptMenu->SetHelp(L"Menus"sv);
		HOptMenu->SetPosition({ 0, 0, ScrX, 0 });
		Global->WindowManager->ExecuteWindow(HOptMenu);

		const auto IsRightPanelActive = []
		{
			return Global->CtrlObject->Cp()->ActivePanel() == Global->CtrlObject->Cp()->RightPanel() &&
				Global->CtrlObject->Cp()->ActivePanel()->IsVisible();
		};

		if (LastCommand)
		{
			const auto HItemToShow = LastHItem != -1? LastHItem : IsRightPanelActive()? static_cast<int>(std::size(MainMenu) - 1) : 0;

			MainMenu[0].Selected = false;
			MainMenu[HItemToShow].Selected = true;
			MainMenu[HItemToShow].SubMenu[0].SetSelect(false);
			MainMenu[HItemToShow].SubMenu[LastVItem].SetSelect(true);
			Global->WindowManager->CallbackWindow([&HOptMenu](){HOptMenu->ProcessKey(Manager::Key(KEY_DOWN));});
		}
		else
		{
			if (IsRightPanelActive())
			{
				MainMenu[0].Selected = false;
				MainMenu[std::size(MainMenu) - 1].Selected = true;
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
				{
					const auto Result = Shortcuts::Configure();
					if (Result != -1)
						Global->CtrlObject->Cp()->ActivePanel()->ExecShortcutFolder(Result);
				}
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
					auto InterfaceLanguage = strLanguage.Get();
					if (SelectInterfaceLanguage(InterfaceLanguage))
					{
						try
						{
							far_language::instance().load(Global->g_strFarPath, InterfaceLanguage, static_cast<int>(lng::MNewFileName + 1));
							strLanguage = InterfaceLanguage;
						}
						catch (const far_known_exception& e)
						{
							Message(MSG_WARNING,
								msg(lng::MError),
								{
									e.message()
								},
								{ lng::MOk });
						}

						auto HelpLanguage = strHelpLanguage.Get();
						if (SelectHelpLanguage(HelpLanguage))
						{
							strHelpLanguage = HelpLanguage;
						}
						Global->CtrlObject->Plugins->ReloadLanguage();
						os::env::set(L"FARLANG"sv, strLanguage);
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
					codepages::instance().SelectCodePage(CodePage, false, true);
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

string GetFarIniString(string_view const AppName, string_view const KeyName, string_view const Default)
{
	return os::GetPrivateProfileString(AppName, KeyName, Default, Global->g_strFarINI);
}

int GetFarIniInt(string_view const AppName, string_view const KeyName, int Default)
{
	return GetPrivateProfileInt(null_terminated(AppName).c_str(), null_terminated(KeyName).c_str(), Default, Global->g_strFarINI.c_str());
}

std::chrono::steady_clock::duration GetRedrawTimeout() noexcept
{
	return std::chrono::milliseconds(Global->Opt->RedrawTimeout);
}
