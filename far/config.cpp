/*
config.cpp

Конфигурация
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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
#include "lang.hpp"
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
#include "registry.hpp"
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
#include "adminmode.hpp"

Options Opt={0};

// Стандартный набор разделителей
static const wchar_t *WordDiv0 = L"~!%^&*()+|{}:\"<>?`-=\\[];',./";

// Стандартный набор разделителей для функции Xlat
static const wchar_t *WordDivForXlat0=L" \t!#$%^&*()+|=\\/@?";

string strKeyNameConsoleDetachKey;
static const wchar_t szCtrlShiftX[]=L"CtrlShiftX";
static const wchar_t szCtrlDot[]=L"Ctrl.";
static const wchar_t szCtrlShiftDot[]=L"CtrlShift.";

// KeyName
const wchar_t NKeyColors[]=L"Colors";
const wchar_t NKeyScreen[]=L"Screen";
const wchar_t NKeyCmdline[]=L"Cmdline";
const wchar_t NKeyInterface[]=L"Interface";
const wchar_t NKeyInterfaceCompletion[]=L"Interface\\Completion";
const wchar_t NKeyViewer[]=L"Viewer";
const wchar_t NKeyDialog[]=L"Dialog";
const wchar_t NKeyEditor[]=L"Editor";
const wchar_t NKeyXLat[]=L"XLat";
const wchar_t NKeySystem[]=L"System";
const wchar_t NKeySystemExecutor[]=L"System\\Executor";
const wchar_t NKeySystemNowell[]=L"System\\Nowell";
const wchar_t NKeyHelp[]=L"Help";
const wchar_t NKeyLanguage[]=L"Language";
const wchar_t NKeyConfirmations[]=L"Confirmations";
const wchar_t NKeyPluginConfirmations[]=L"PluginConfirmations";
const wchar_t NKeyPanel[]=L"Panel";
const wchar_t NKeyPanelLeft[]=L"Panel\\Left";
const wchar_t NKeyPanelRight[]=L"Panel\\Right";
const wchar_t NKeyPanelLayout[]=L"Panel\\Layout";
const wchar_t NKeyPanelTree[]=L"Panel\\Tree";
const wchar_t NKeyPanelInfo[]=L"Panel\\Info";
const wchar_t NKeyLayout[]=L"Layout";
const wchar_t NKeyDescriptions[]=L"Descriptions";
const wchar_t NKeyKeyMacros[]=L"KeyMacros";
const wchar_t NKeyPolicies[]=L"Policies";
const wchar_t NKeyFileFilter[]=L"OperationsFilter";
const wchar_t NKeySavedHistory[]=L"SavedHistory";
const wchar_t NKeySavedViewHistory[]=L"SavedViewHistory";
const wchar_t NKeySavedFolderHistory[]=L"SavedFolderHistory";
const wchar_t NKeySavedDialogHistory[]=L"SavedDialogHistory";
const wchar_t NKeyCodePages[]=L"CodePages";
const wchar_t NParamHistoryCount[]=L"HistoryCount";

const wchar_t *constBatchExt=L".BAT;.CMD;";

void SystemSettings()
{
	DialogBuilder Builder(MConfigSystemTitle, L"SystemSettings");

	Builder.AddCheckbox(MConfigRO, &Opt.ClearReadOnly);

	DialogItemEx *DeleteToRecycleBin = Builder.AddCheckbox(MConfigRecycleBin, &Opt.DeleteToRecycleBin);
	DialogItemEx *DeleteLinks = Builder.AddCheckbox(MConfigRecycleBinLink, &Opt.DeleteToRecycleBinKillLink);
	DeleteLinks->Indent(4);
	Builder.LinkFlags(DeleteToRecycleBin, DeleteLinks, DIF_DISABLE);

	Builder.AddCheckbox(MConfigSystemCopy, &Opt.CMOpt.UseSystemCopy);
	Builder.AddCheckbox(MConfigCopySharing, &Opt.CMOpt.CopyOpened);
	Builder.AddCheckbox(MConfigScanJunction, &Opt.ScanJunction);
	Builder.AddCheckbox(MConfigCreateUppercaseFolders, &Opt.CreateUppercaseFolders);

	DialogItemEx *InactivityExit = Builder.AddCheckbox(MConfigInactivity, &Opt.InactivityExit);
	DialogItemEx *InactivityExitTime = Builder.AddIntEditField(&Opt.InactivityExitTime, 2);
	InactivityExitTime->Indent(4);
	Builder.AddTextAfter(InactivityExitTime, MConfigInactivityMinutes);
	Builder.LinkFlags(InactivityExit, InactivityExitTime, DIF_DISABLE);

	Builder.AddCheckbox(MConfigSaveHistory, &Opt.SaveHistory);
	Builder.AddCheckbox(MConfigSaveFoldersHistory, &Opt.SaveFoldersHistory);
	Builder.AddCheckbox(MConfigSaveViewHistory, &Opt.SaveViewHistory);
	Builder.AddCheckbox(MConfigRegisteredTypes, &Opt.UseRegisteredTypes);
	Builder.AddCheckbox(MConfigCloseCDGate, &Opt.CloseCDGate);
	Builder.AddText(MConfigElevation);
	Builder.AddCheckbox(MConfigElevationModify, &Opt.ElevationMode, ELEVATION_MODIFY_REQUEST)->Indent(2);
	Builder.AddCheckbox(MConfigElevationRead, &Opt.ElevationMode, ELEVATION_READ_REQUEST)->Indent(2);
	Builder.AddCheckbox(MConfigAutoSave, &Opt.AutoSaveSetup);
	Builder.AddOKCancel();

	Builder.ShowDialog();
}


void PanelSettings()
{
	DialogBuilder Builder(MConfigPanelTitle, L"PanelSettings");
	BOOL AutoUpdate = (Opt.AutoUpdateLimit );

	Builder.AddCheckbox(MConfigHidden, &Opt.ShowHidden);
	Builder.AddCheckbox(MConfigHighlight, &Opt.Highlight);
	Builder.AddCheckbox(MConfigAutoChange, &Opt.Tree.AutoChangeFolder);
	Builder.AddCheckbox(MConfigSelectFolders, &Opt.SelectFolders);
	Builder.AddCheckbox(MConfigSortFolderExt, &Opt.SortFolderExt);
	Builder.AddCheckbox(MConfigReverseSort, &Opt.ReverseSort);

	DialogItemEx *AutoUpdateEnabled = Builder.AddCheckbox(MConfigAutoUpdateLimit, &AutoUpdate);
	DialogItemEx *AutoUpdateLimit = Builder.AddIntEditField((int *) &Opt.AutoUpdateLimit, 6);
	Builder.LinkFlags(AutoUpdateEnabled, AutoUpdateLimit, DIF_DISABLE, false);
	DialogItemEx *AutoUpdateText = Builder.AddTextBefore(AutoUpdateLimit, MConfigAutoUpdateLimit2);
	AutoUpdateLimit->Indent(4);
	AutoUpdateText->Indent(4);
	Builder.AddCheckbox(MConfigAutoUpdateRemoteDrive, &Opt.AutoUpdateRemoteDrive);

	Builder.AddSeparator();
	Builder.AddCheckbox(MConfigShowColumns, &Opt.ShowColumnTitles);
	Builder.AddCheckbox(MConfigShowStatus, &Opt.ShowPanelStatus);
	Builder.AddCheckbox(MConfigShowTotal, &Opt.ShowPanelTotals);
	Builder.AddCheckbox(MConfigShowFree, &Opt.ShowPanelFree);
	Builder.AddCheckbox(MConfigShowScrollbar, &Opt.ShowPanelScrollbar);
	Builder.AddCheckbox(MConfigShowScreensNumber, &Opt.ShowScreensNumber);
	Builder.AddCheckbox(MConfigShowSortMode, &Opt.ShowSortMode);
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


/* $ 17.12.2001 IS
   Настройка средней кнопки мыши для панелей. Воткнем пока сюда, потом надо
   переехать в специальный диалог по программированию мыши.
*/
void InterfaceSettings()
{
	DialogBuilder Builder(MConfigInterfaceTitle, L"InterfSettings");

	Builder.AddCheckbox(MConfigClock, &Opt.Clock);
	Builder.AddCheckbox(MConfigViewerEditorClock, &Opt.ViewerEditorClock);
	Builder.AddCheckbox(MConfigMouse, &Opt.Mouse);
	Builder.AddCheckbox(MConfigKeyBar, &Opt.ShowKeyBar);
	Builder.AddCheckbox(MConfigMenuBar, &Opt.ShowMenuBar);
	DialogItemEx *SaverCheckbox = Builder.AddCheckbox(MConfigSaver, &Opt.ScreenSaver);

	DialogItemEx *SaverEdit = Builder.AddIntEditField(&Opt.ScreenSaverTime, 2);
	SaverEdit->Indent(4);
	Builder.AddTextAfter(SaverEdit, MConfigSaverMinutes);
	Builder.LinkFlags(SaverCheckbox, SaverEdit, DIF_DISABLE);

	Builder.AddCheckbox(MConfigCopyTotal, &Opt.CMOpt.CopyShowTotal);
	Builder.AddCheckbox(MConfigCopyTimeRule, &Opt.CMOpt.CopyTimeRule);
	Builder.AddCheckbox(MConfigDeleteTotal, &Opt.DelOpt.DelShowTotal);
	Builder.AddCheckbox(MConfigPgUpChangeDisk, &Opt.PgUpChangeDisk);
	Builder.AddCheckbox(MConfigClearType, &Opt.ClearType);
	Builder.AddText(MConfigTitleAddons);
	Builder.AddEditField(&Opt.strTitleAddons, 47);
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
	DialogItemEx *ListCheck=Builder.AddCheckbox(MConfigAutoCompleteShowList, &Opt.AutoComplete.ShowList);
	DialogItemEx *ModalModeCheck=Builder.AddCheckbox(MConfigAutoCompleteModalList, &Opt.AutoComplete.ModalList);
	ModalModeCheck->Indent(4);
	Builder.AddCheckbox(MConfigAutoCompleteAutoAppend, &Opt.AutoComplete.AppendCompletion);
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
	Builder.AddText(MConfigInfoPanelCNTitle);
	Builder.AddComboBox((int *) &Opt.InfoPanel.ComputerNameFormat, 50, CNListItems, ARRAYSIZE(CNListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddText(MConfigInfoPanelUNTitle);
	Builder.AddComboBox((int *) &Opt.InfoPanel.UserNameFormat, 50, UNListItems, ARRAYSIZE(UNListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddOKCancel();
	Builder.ShowDialog();
}

void DialogSettings()
{
	DialogBuilder Builder(MConfigDlgSetsTitle, L"DialogSettings");

	Builder.AddCheckbox(MConfigDialogsEditHistory, &Opt.Dialogs.EditHistory);
	Builder.AddCheckbox(MConfigDialogsEditBlock, &Opt.Dialogs.EditBlock);
	Builder.AddCheckbox(MConfigDialogsDelRemovesBlocks, &Opt.Dialogs.DelRemovesBlocks);
	Builder.AddCheckbox(MConfigDialogsAutoComplete, &Opt.Dialogs.AutoComplete);
	Builder.AddCheckbox(MConfigDialogsEULBsClear, &Opt.Dialogs.EULBsClear);
	Builder.AddCheckbox(MConfigDialogsMouseButton, &Opt.Dialogs.MouseButton);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (Opt.Dialogs.MouseButton )
			Opt.Dialogs.MouseButton = 0xFFFF;
	}
}

void CmdlineSettings()
{
	DialogBuilder Builder(MConfigCmdlineTitle, L"CmdlineSettings");

	Builder.AddCheckbox(MConfigCmdlineEditBlock, &Opt.CmdLine.EditBlock);
	Builder.AddCheckbox(MConfigCmdlineDelRemovesBlocks, &Opt.CmdLine.DelRemovesBlocks);
	Builder.AddCheckbox(MConfigCmdlineAutoComplete, &Opt.CmdLine.AutoComplete);
	DialogItemEx *UsePromptFormat = Builder.AddCheckbox(MConfigCmdlineUsePromptFormat, &Opt.CmdLine.UsePromptFormat);
	DialogItemEx *PromptFormat = Builder.AddEditField(&Opt.CmdLine.strPromptFormat, 19);
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

	Builder.AddCheckbox(MSetConfirmCopy, &Opt.Confirm.Copy);
	Builder.AddCheckbox(MSetConfirmMove, &Opt.Confirm.Move);
	Builder.AddCheckbox(MSetConfirmRO, &Opt.Confirm.RO);
	Builder.AddCheckbox(MSetConfirmDelete, &Opt.Confirm.Delete);
	Builder.AddCheckbox(MSetConfirmDeleteFolders, &Opt.Confirm.DeleteFolder);
	Builder.AddCheckbox(MSetConfirmEsc, &Opt.Confirm.Esc);
	Builder.AddCheckbox(MSetConfirmRemoveConnection, &Opt.Confirm.RemoveConnection);
	Builder.AddCheckbox(MSetConfirmRemoveSUBST, &Opt.Confirm.RemoveSUBST);
	Builder.AddCheckbox(MSetConfirmDetachVHD, &Opt.Confirm.DetachVHD);
	Builder.AddCheckbox(MSetConfirmRemoveHotPlug, &Opt.Confirm.RemoveHotPlug);
	Builder.AddCheckbox(MSetConfirmAllowReedit, &Opt.Confirm.AllowReedit);
	Builder.AddCheckbox(MSetConfirmHistoryClear, &Opt.Confirm.HistoryClear);
	Builder.AddCheckbox(MSetConfirmExit, &Opt.Confirm.Exit);
	Builder.AddOKCancel();

	Builder.ShowDialog();
}

void PluginsManagerSettings()
{
	DialogBuilder Builder(MPluginsManagerSettingsTitle, L"PluginsManagerSettings");

	Builder.AddCheckbox(MPluginsManagerOEMPluginsSupport, &Opt.LoadPlug.OEMPluginsSupport);
	Builder.AddCheckbox(MPluginsManagerScanSymlinks, &Opt.LoadPlug.ScanSymlinks);
	Builder.AddText(MPluginsManagerPersonalPath);
	Builder.AddEditField(&Opt.LoadPlug.strPersonalPluginsPath, 45, L"PersPath", DIF_EDITPATH);

	Builder.AddSeparator(MPluginConfirmationTitle);
	Builder.AddCheckbox(MPluginsManagerOFPNew, &Opt.PluginConfirm.OpenFilePluginNew);
	DialogItemEx *ConfirmOFP = Builder.AddCheckbox(MPluginsManagerOFP, &Opt.PluginConfirm.OpenFilePlugin);
	DialogItemEx *StandardAssoc = Builder.AddCheckbox(MPluginsManagerStdAssoc, &Opt.PluginConfirm.StandardAssociation);
	DialogItemEx *EvenIfOnlyOne = Builder.AddCheckbox(MPluginsManagerEvenOne, &Opt.PluginConfirm.EvenIfOnlyOnePlugin);
	StandardAssoc->Indent(2);
	EvenIfOnlyOne->Indent(4);
	Builder.LinkFlags(ConfirmOFP, StandardAssoc, DIF_DISABLE);
	Builder.LinkFlags(ConfirmOFP, EvenIfOnlyOne, DIF_DISABLE);
	Builder.LinkFlags(StandardAssoc, EvenIfOnlyOne, DIF_DISABLE);

	Builder.AddCheckbox(MPluginsManagerSFL, &Opt.PluginConfirm.SetFindList);
	Builder.AddCheckbox(MPluginsManagerPF, &Opt.PluginConfirm.Prefix);
	Builder.AddOKCancel();

	Builder.ShowDialog();
}


void SetDizConfig()
{
	DialogBuilder Builder(MCfgDizTitle, L"FileDiz");

	Builder.AddText(MCfgDizListNames);
	Builder.AddEditField(&Opt.Diz.strListNames, 65);
	Builder.AddSeparator();

	Builder.AddCheckbox(MCfgDizSetHidden, &Opt.Diz.SetHidden);
	Builder.AddCheckbox(MCfgDizROUpdate, &Opt.Diz.ROUpdate);
	DialogItemEx *StartPos = Builder.AddIntEditField(&Opt.Diz.StartPos, 2);
	Builder.AddTextAfter(StartPos, MCfgDizStartPos);
	Builder.AddSeparator();

	static int DizOptions[] = { MCfgDizNotUpdate, MCfgDizUpdateIfDisplayed, MCfgDizAlwaysUpdate };
	Builder.AddRadioButtons(&Opt.Diz.UpdateMode, 3, DizOptions);
	Builder.AddSeparator();

	Builder.AddCheckbox(MCfgDizAnsiByDefault, &Opt.Diz.AnsiByDefault);
	Builder.AddCheckbox(MCfgDizSaveInUTF, &Opt.Diz.SaveInUTF);
	Builder.AddOKCancel();
	Builder.ShowDialog();
}

void ViewerConfig(ViewerOptions &ViOpt,bool Local)
{
	DialogBuilder Builder(MViewConfigTitle, L"ViewerSettings");
	if (!Local)
	{
		Builder.AddCheckbox(MViewConfigExternalF3, &Opt.ViOpt.UseExternalViewer);
		Builder.AddText(MViewConfigExternalCommand);
		Builder.AddEditField(&Opt.strExternalViewer, 64, L"ExternalViewer", DIF_EDITPATH);
		Builder.AddSeparator(MViewConfigInternal);
	}

	Builder.StartColumns();
	Builder.AddCheckbox(MViewConfigPersistentSelection, &ViOpt.PersistentBlocks);
	DialogItemEx *SavePos = Builder.AddCheckbox(MViewConfigSavePos, &Opt.ViOpt.SavePos);
	DialogItemEx *TabSize = Builder.AddIntEditField(&ViOpt.TabSize, 3);
	Builder.AddTextAfter(TabSize, MViewConfigTabSize);
	Builder.ColumnBreak();
	Builder.AddCheckbox(MViewConfigArrows, &ViOpt.ShowArrows);
	DialogItemEx *SaveShortPos = Builder.AddCheckbox(MViewConfigSaveShortPos, &Opt.ViOpt.SaveShortPos);
	Builder.LinkFlags(SavePos, SaveShortPos, DIF_DISABLE);
	Builder.AddCheckbox(MViewConfigScrollbar, &ViOpt.ShowScrollbar);
	Builder.EndColumns();

	if (!Local)
	{
		Builder.AddEmptyLine();
		Builder.AddCheckbox(MViewAutoDetectCodePage, &ViOpt.AutoDetectCodePage);
		Builder.AddCheckbox(MViewConfigAnsiCodePageAsDefault, &ViOpt.AnsiCodePageAsDefault);
	}
	Builder.AddOKCancel();
	if (Builder.ShowDialog())
	{
		if (ViOpt.TabSize<1 || ViOpt.TabSize>512)
			ViOpt.TabSize=8;
	}
}

void EditorConfig(EditorOptions &EdOpt,bool Local)
{
	DialogBuilder Builder(MEditConfigTitle, L"EditorSettings");
	if (!Local)
	{
		Builder.AddCheckbox(MEditConfigEditorF4, &Opt.EdOpt.UseExternalEditor);
		Builder.AddText(MEditConfigEditorCommand);
		Builder.AddEditField(&Opt.strExternalEditor, 64, L"ExternalEditor", DIF_EDITPATH);
		Builder.AddSeparator(MEditConfigInternal);
	}

	Builder.AddText(MEditConfigExpandTabsTitle);
	DialogBuilderListItem ExpandTabsItems[] = {
		{ MEditConfigDoNotExpandTabs, EXPAND_NOTABS },
		{ MEditConfigExpandTabs, EXPAND_NEWTABS },
		{ MEditConfigConvertAllTabsToSpaces, EXPAND_ALLTABS }
	};
	Builder.AddComboBox(&EdOpt.ExpandTabs, 64, ExpandTabsItems, 3, DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);

	Builder.StartColumns();
	Builder.AddCheckbox(MEditConfigPersistentBlocks, &EdOpt.PersistentBlocks);
	DialogItemEx *SavePos = Builder.AddCheckbox(MEditConfigSavePos, &EdOpt.SavePos);
	Builder.AddCheckbox(MEditConfigAutoIndent, &EdOpt.AutoIndent);
	DialogItemEx *TabSize = Builder.AddIntEditField(&EdOpt.TabSize, 3);
	Builder.AddTextAfter(TabSize, MEditConfigTabSize);
	Builder.AddCheckbox(MEditShowWhiteSpace, &EdOpt.ShowWhiteSpace);
	Builder.ColumnBreak();
	Builder.AddCheckbox(MEditConfigDelRemovesBlocks, &EdOpt.DelRemovesBlocks);
	DialogItemEx *SaveShortPos = Builder.AddCheckbox(MEditConfigSaveShortPos, &EdOpt.SaveShortPos);
	Builder.LinkFlags(SavePos, SaveShortPos, DIF_DISABLE);
	Builder.AddCheckbox(MEditCursorBeyondEnd, &EdOpt.CursorBeyondEOL);
	Builder.AddCheckbox(MEditConfigScrollbar, &EdOpt.ShowScrollBar);
	Builder.AddCheckbox(MEditConfigPickUpWord, &EdOpt.SearchPickUpWord);
	Builder.EndColumns();

	if (!Local)
	{
		Builder.AddEmptyLine();
		Builder.AddCheckbox(MEditShareWrite, &EdOpt.EditOpenedForWrite);
		Builder.AddCheckbox(MEditLockROFileModification, &EdOpt.ReadOnlyLock, 1);
		Builder.AddCheckbox(MEditWarningBeforeOpenROFile, &EdOpt.ReadOnlyLock, 2);
		Builder.AddCheckbox(MEditAutoDetectCodePage, &EdOpt.AutoDetectCodePage);
		Builder.AddCheckbox(MEditConfigAnsiCodePageAsDefault, &EdOpt.AnsiCodePageAsDefault);
		Builder.AddCheckbox(MEditConfigAnsiCodePageForNewFile, &EdOpt.AnsiCodePageForNewFile);
	}

	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (EdOpt.TabSize<1 || EdOpt.TabSize>512)
			EdOpt.TabSize=8;
	}
}


void SetFolderInfoFiles()
{
	string strFolderInfoFiles;

	if (GetString(MSG(MSetFolderInfoTitle),MSG(MSetFolderInfoNames),L"FolderInfoFiles",
	              Opt.InfoPanel.strFolderInfoFiles,strFolderInfoFiles,L"OptMenu",FIB_ENABLEEMPTY|FIB_BUTTONS))
	{
		Opt.InfoPanel.strFolderInfoFiles = strFolderInfoFiles;

		if (CtrlObject->Cp()->LeftPanel->GetType() == INFO_PANEL)
			CtrlObject->Cp()->LeftPanel->Update(0);

		if (CtrlObject->Cp()->RightPanel->GetType() == INFO_PANEL)
			CtrlObject->Cp()->RightPanel->Update(0);
	}
}


// Структура, описывающая всю конфигурацию(!)
static struct FARConfig
{
	int   IsSave;   // =1 - будет записываться в SaveConfig()
	DWORD ValType;  // REG_DWORD, REG_SZ, REG_BINARY
	const wchar_t *KeyName;
	const wchar_t *ValName;
	void *ValPtr;   // адрес переменной, куда помещаем данные
	DWORD DefDWord; // он же размер данных для REG_SZ и REG_BINARY
	const wchar_t *DefStr;   // строка/данные по умолчанию
} CFG[]=
{
	{1, REG_BINARY, NKeyColors, L"CurrentPalette",(char*)Palette,SizeArrayPalette,(wchar_t*)DefaultPalette},

	{1, REG_DWORD,  NKeyScreen, L"Clock", &Opt.Clock, 1, 0},
	{1, REG_DWORD,  NKeyScreen, L"ViewerEditorClock",&Opt.ViewerEditorClock,0, 0},
	{1, REG_DWORD,  NKeyScreen, L"KeyBar",&Opt.ShowKeyBar,1, 0},
	{1, REG_DWORD,  NKeyScreen, L"ScreenSaver",&Opt.ScreenSaver, 0, 0},
	{1, REG_DWORD,  NKeyScreen, L"ScreenSaverTime",&Opt.ScreenSaverTime,5, 0},
	{0, REG_DWORD,  NKeyScreen, L"DeltaXY", &Opt.ScrSize.DeltaXY, 0, 0},

	{1, REG_DWORD,  NKeyCmdline, L"UsePromptFormat", &Opt.CmdLine.UsePromptFormat,0, 0},
	{1, REG_SZ,     NKeyCmdline, L"PromptFormat",&Opt.CmdLine.strPromptFormat, 0, L"$p$g"},
	{1, REG_DWORD,  NKeyCmdline, L"DelRemovesBlocks", &Opt.CmdLine.DelRemovesBlocks,1, 0},
	{1, REG_DWORD,  NKeyCmdline, L"EditBlock", &Opt.CmdLine.EditBlock,0, 0},
	{1, REG_DWORD,  NKeyCmdline, L"AutoComplete",&Opt.CmdLine.AutoComplete,1, 0},


	{1, REG_DWORD,  NKeyInterface, L"Mouse",&Opt.Mouse,1, 0},
	{0, REG_DWORD,  NKeyInterface, L"UseVk_oem_x",&Opt.UseVk_oem_x,1, 0},
	{1, REG_DWORD,  NKeyInterface, L"ShowMenuBar",&Opt.ShowMenuBar,0, 0},
	{0, REG_DWORD,  NKeyInterface, L"CursorSize1",&Opt.CursorSize[0],15, 0},
	{0, REG_DWORD,  NKeyInterface, L"CursorSize2",&Opt.CursorSize[1],10, 0},
	{0, REG_DWORD,  NKeyInterface, L"CursorSize3",&Opt.CursorSize[2],99, 0},
	{0, REG_DWORD,  NKeyInterface, L"CursorSize4",&Opt.CursorSize[3],99, 0},
	{0, REG_DWORD,  NKeyInterface, L"ShiftsKeyRules",&Opt.ShiftsKeyRules,1, 0},
	{0, REG_DWORD,  NKeyInterface, L"AltF9",&Opt.AltF9, 1, 0},
	{1, REG_DWORD,  NKeyInterface, L"CtrlPgUp",&Opt.PgUpChangeDisk, 1, 0},
	{1, REG_DWORD,  NKeyInterface, L"ClearType",&Opt.ClearType, 0, 0},
	{0, REG_DWORD,  NKeyInterface, L"ShowTimeoutDelFiles",&Opt.ShowTimeoutDelFiles, 50, 0},
	{0, REG_DWORD,  NKeyInterface, L"ShowTimeoutDACLFiles",&Opt.ShowTimeoutDACLFiles, 50, 0},
	{0, REG_DWORD,  NKeyInterface, L"FormatNumberSeparators",&Opt.FormatNumberSeparators, 0, 0},
	{1, REG_DWORD,  NKeyInterface, L"CopyShowTotal",&Opt.CMOpt.CopyShowTotal,1, 0},
	{1, REG_DWORD,  NKeyInterface,L"DelShowTotal",&Opt.DelOpt.DelShowTotal,0, 0},
	{1, REG_SZ,     NKeyInterface,L"TitleAddons",&Opt.strTitleAddons, 0, L"%Ver.%Build %Platform %Admin"},
	{1, REG_DWORD,  NKeyInterfaceCompletion,L"ShowList",&Opt.AutoComplete.ShowList, 1, 0},
	{1, REG_DWORD,  NKeyInterfaceCompletion,L"ModalList",&Opt.AutoComplete.ModalList, 0, 0},
	{1, REG_DWORD,  NKeyInterfaceCompletion,L"Append",&Opt.AutoComplete.AppendCompletion, 0, 0},

	{1, REG_SZ,     NKeyViewer,L"ExternalViewerName",&Opt.strExternalViewer, 0, L""},
	{1, REG_DWORD,  NKeyViewer,L"UseExternalViewer",&Opt.ViOpt.UseExternalViewer,0, 0},
	{1, REG_DWORD,  NKeyViewer,L"SaveViewerPos",&Opt.ViOpt.SavePos,1, 0},
	{1, REG_DWORD,  NKeyViewer,L"SaveViewerShortPos",&Opt.ViOpt.SaveShortPos,1, 0},
	{1, REG_DWORD,  NKeyViewer,L"AutoDetectCodePage",&Opt.ViOpt.AutoDetectCodePage,0, 0},
	{1, REG_DWORD,  NKeyViewer,L"SearchRegexp",&Opt.ViOpt.SearchRegexp,0, 0},

	{1, REG_DWORD,  NKeyViewer,L"TabSize",&Opt.ViOpt.TabSize,8, 0},
	{1, REG_DWORD,  NKeyViewer,L"ShowKeyBar",&Opt.ViOpt.ShowKeyBar,1, 0},
	{0, REG_DWORD,  NKeyViewer,L"ShowTitleBar",&Opt.ViOpt.ShowTitleBar,1, 0},
	{1, REG_DWORD,  NKeyViewer,L"ShowArrows",&Opt.ViOpt.ShowArrows,1, 0},
	{1, REG_DWORD,  NKeyViewer,L"ShowScrollbar",&Opt.ViOpt.ShowScrollbar,0, 0},
	{1, REG_DWORD,  NKeyViewer,L"IsWrap",&Opt.ViOpt.ViewerIsWrap,1, 0},
	{1, REG_DWORD,  NKeyViewer,L"Wrap",&Opt.ViOpt.ViewerWrap,0, 0},
	{1, REG_DWORD,  NKeyViewer,L"PersistentBlocks",&Opt.ViOpt.PersistentBlocks,0, 0},
	{1, REG_DWORD,  NKeyViewer,L"AnsiCodePageAsDefault",&Opt.ViOpt.AnsiCodePageAsDefault,1, 0},

	{1, REG_DWORD,  NKeyDialog, L"EditHistory",&Opt.Dialogs.EditHistory,1, 0},
	{1, REG_DWORD,  NKeyDialog, L"EditBlock",&Opt.Dialogs.EditBlock,0, 0},
	{1, REG_DWORD,  NKeyDialog, L"AutoComplete",&Opt.Dialogs.AutoComplete,1, 0},
	{1, REG_DWORD,  NKeyDialog,L"EULBsClear",&Opt.Dialogs.EULBsClear,0, 0},
	{0, REG_DWORD,  NKeyDialog,L"SelectFromHistory",&Opt.Dialogs.SelectFromHistory,0, 0},
	{0, REG_DWORD,  NKeyDialog,L"EditLine",&Opt.Dialogs.EditLine,0, 0},
	{1, REG_DWORD,  NKeyDialog,L"MouseButton",&Opt.Dialogs.MouseButton,0xFFFF, 0},
	{1, REG_DWORD,  NKeyDialog,L"DelRemovesBlocks",&Opt.Dialogs.DelRemovesBlocks,1, 0},
	{0, REG_DWORD,  NKeyDialog,L"CBoxMaxHeight",&Opt.Dialogs.CBoxMaxHeight,8, 0},

	{1, REG_SZ,     NKeyEditor,L"ExternalEditorName",&Opt.strExternalEditor, 0, L""},
	{1, REG_DWORD,  NKeyEditor,L"UseExternalEditor",&Opt.EdOpt.UseExternalEditor,0, 0},
	{1, REG_DWORD,  NKeyEditor,L"ExpandTabs",&Opt.EdOpt.ExpandTabs,0, 0},
	{1, REG_DWORD,  NKeyEditor,L"TabSize",&Opt.EdOpt.TabSize,8, 0},
	{1, REG_DWORD,  NKeyEditor,L"PersistentBlocks",&Opt.EdOpt.PersistentBlocks,0, 0},
	{1, REG_DWORD,  NKeyEditor,L"DelRemovesBlocks",&Opt.EdOpt.DelRemovesBlocks,1, 0},
	{1, REG_DWORD,  NKeyEditor,L"AutoIndent",&Opt.EdOpt.AutoIndent,0, 0},
	{1, REG_DWORD,  NKeyEditor,L"SaveEditorPos",&Opt.EdOpt.SavePos,1, 0},
	{1, REG_DWORD,  NKeyEditor,L"SaveEditorShortPos",&Opt.EdOpt.SaveShortPos,1, 0},
	{1, REG_DWORD,  NKeyEditor,L"AutoDetectCodePage",&Opt.EdOpt.AutoDetectCodePage,0, 0},
	{1, REG_DWORD,  NKeyEditor,L"EditorCursorBeyondEOL",&Opt.EdOpt.CursorBeyondEOL,1, 0},
	{1, REG_DWORD,  NKeyEditor,L"ReadOnlyLock",&Opt.EdOpt.ReadOnlyLock,0, 0}, // Вернём назад дефолт 1.65 - не предупреждать и не блокировать
	{0, REG_DWORD,  NKeyEditor,L"EditorUndoSize",&Opt.EdOpt.UndoSize,0, 0}, // $ 03.12.2001 IS размер буфера undo в редакторе
	{0, REG_SZ,     NKeyEditor,L"WordDiv",&Opt.strWordDiv, 0, WordDiv0},
	{0, REG_DWORD,  NKeyEditor,L"BSLikeDel",&Opt.EdOpt.BSLikeDel,1, 0},
	{0, REG_DWORD,  NKeyEditor,L"EditorF7Rules",&Opt.EdOpt.F7Rules,1, 0},
	{0, REG_DWORD,  NKeyEditor,L"FileSizeLimit",&Opt.EdOpt.FileSizeLimitLo,(DWORD)0, 0},
	{0, REG_DWORD,  NKeyEditor,L"FileSizeLimitHi",&Opt.EdOpt.FileSizeLimitHi,(DWORD)0, 0},
	{0, REG_DWORD,  NKeyEditor,L"CharCodeBase",&Opt.EdOpt.CharCodeBase,1, 0},
	{0, REG_DWORD,  NKeyEditor,L"AllowEmptySpaceAfterEof", &Opt.EdOpt.AllowEmptySpaceAfterEof,0,0},//skv
	{1, REG_DWORD,  NKeyEditor,L"AnsiCodePageForNewFile",&Opt.EdOpt.AnsiCodePageForNewFile,1, 0},
	{1, REG_DWORD,  NKeyEditor,L"AnsiCodePageAsDefault",&Opt.EdOpt.AnsiCodePageAsDefault,1, 0},
	{1, REG_DWORD,  NKeyEditor,L"ShowKeyBar",&Opt.EdOpt.ShowKeyBar,1, 0},
	{0, REG_DWORD,  NKeyEditor,L"ShowTitleBar",&Opt.EdOpt.ShowTitleBar,1, 0},
	{1, REG_DWORD,  NKeyEditor,L"ShowScrollBar",&Opt.EdOpt.ShowScrollBar,0, 0},
	{1, REG_DWORD,  NKeyEditor,L"EditOpenedForWrite",&Opt.EdOpt.EditOpenedForWrite,1, 0},
	{1, REG_DWORD,  NKeyEditor,L"SearchSelFound",&Opt.EdOpt.SearchSelFound,0, 0},
	{1, REG_DWORD,  NKeyEditor,L"SearchRegexp",&Opt.EdOpt.SearchRegexp,0, 0},
	{1, REG_DWORD,  NKeyEditor,L"SearchPickUpWord",&Opt.EdOpt.SearchPickUpWord,0, 0},
	{1, REG_DWORD,  NKeyEditor,L"ShowWhiteSpace",&Opt.EdOpt.ShowWhiteSpace,0, 0},

	{0, REG_DWORD,  NKeyXLat,L"Flags",&Opt.XLat.Flags,(DWORD)XLAT_SWITCHKEYBLAYOUT|XLAT_CONVERTALLCMDLINE, 0},
	{0, REG_SZ,     NKeyXLat,L"Table1",&Opt.XLat.Table[0],0,L""},
	{0, REG_SZ,     NKeyXLat,L"Table2",&Opt.XLat.Table[1],0,L""},
	{0, REG_SZ,     NKeyXLat,L"Rules1",&Opt.XLat.Rules[0],0,L""},
	{0, REG_SZ,     NKeyXLat,L"Rules2",&Opt.XLat.Rules[1],0,L""},
	{0, REG_SZ,     NKeyXLat,L"Rules3",&Opt.XLat.Rules[2],0,L""},
	{0, REG_SZ,     NKeyXLat,L"WordDivForXlat",&Opt.XLat.strWordDivForXlat, 0,WordDivForXlat0},

	{0, REG_DWORD,  NKeySavedHistory, NParamHistoryCount,&Opt.HistoryCount,512, 0},
	{0, REG_DWORD,  NKeySavedFolderHistory, NParamHistoryCount,&Opt.FoldersHistoryCount,512, 0},
	{0, REG_DWORD,  NKeySavedViewHistory, NParamHistoryCount,&Opt.ViewHistoryCount,512, 0},
	{0, REG_DWORD,  NKeySavedDialogHistory, NParamHistoryCount,&Opt.DialogsHistoryCount,512, 0},

	{1, REG_DWORD,  NKeySystem,L"SaveHistory",&Opt.SaveHistory,1, 0},
	{1, REG_DWORD,  NKeySystem,L"SaveFoldersHistory",&Opt.SaveFoldersHistory,1, 0},
	{0, REG_DWORD,  NKeySystem,L"SavePluginFoldersHistory",&Opt.SavePluginFoldersHistory,0, 0},
	{1, REG_DWORD,  NKeySystem,L"SaveViewHistory",&Opt.SaveViewHistory,1, 0},
	{1, REG_DWORD,  NKeySystem,L"UseRegisteredTypes",&Opt.UseRegisteredTypes,1, 0},
	{1, REG_DWORD,  NKeySystem,L"AutoSaveSetup",&Opt.AutoSaveSetup,0, 0},
	{1, REG_DWORD,  NKeySystem,L"ClearReadOnly",&Opt.ClearReadOnly,0, 0},
	{1, REG_DWORD,  NKeySystem,L"DeleteToRecycleBin",&Opt.DeleteToRecycleBin,1, 0},
	{1, REG_DWORD,  NKeySystem,L"DeleteToRecycleBinKillLink",&Opt.DeleteToRecycleBinKillLink,1, 0},
	{0, REG_DWORD,  NKeySystem,L"WipeSymbol",&Opt.WipeSymbol,0, 0},

	{1, REG_DWORD,  NKeySystem,L"UseSystemCopy",&Opt.CMOpt.UseSystemCopy,1, 0},
	{0, REG_DWORD,  NKeySystem,L"CopySecurityOptions",&Opt.CMOpt.CopySecurityOptions,0, 0},
	{1, REG_DWORD,  NKeySystem,L"CopyOpened",&Opt.CMOpt.CopyOpened,1, 0},
	{1, REG_DWORD,  NKeySystem, L"MultiCopy",&Opt.CMOpt.MultiCopy,0, 0},
	{1, REG_DWORD,  NKeySystem,L"CopyTimeRule",  &Opt.CMOpt.CopyTimeRule, 3, 0},

	{1, REG_DWORD,  NKeySystem,L"CreateUppercaseFolders",&Opt.CreateUppercaseFolders,0, 0},
	{1, REG_DWORD,  NKeySystem,L"InactivityExit",&Opt.InactivityExit,0, 0},
	{1, REG_DWORD,  NKeySystem,L"InactivityExitTime",&Opt.InactivityExitTime,15, 0},
	{1, REG_DWORD,  NKeySystem,L"DriveMenuMode",&Opt.ChangeDriveMode,DRIVE_SHOW_TYPE|DRIVE_SHOW_PLUGINS|DRIVE_SHOW_SIZE_FLOAT|DRIVE_SHOW_CDROM, 0},
	{1, REG_DWORD,  NKeySystem,L"DriveDisconnetMode",&Opt.ChangeDriveDisconnetMode,1, 0},
	{1, REG_DWORD,  NKeySystem,L"AutoUpdateRemoteDrive",&Opt.AutoUpdateRemoteDrive,1, 0},
	{1, REG_DWORD,  NKeySystem,L"FileSearchMode",&Opt.FindOpt.FileSearchMode,FINDAREA_FROM_CURRENT, 0},
	{0, REG_DWORD,  NKeySystem,L"CollectFiles",&Opt.FindOpt.CollectFiles, 1, 0},
	{1, REG_SZ,     NKeySystem,L"SearchInFirstSize",&Opt.FindOpt.strSearchInFirstSize, 0, L""},
	{1, REG_DWORD,  NKeySystem,L"FindAlternateStreams",&Opt.FindOpt.FindAlternateStreams,0,0},
	{1, REG_SZ,     NKeySystem,L"SearchOutFormat",&Opt.FindOpt.strSearchOutFormat, 0, L"D,S,A"},
	{1, REG_SZ,     NKeySystem,L"SearchOutFormatWidth",&Opt.FindOpt.strSearchOutFormatWidth, 0, L"14,13,0"},
	{1, REG_DWORD,  NKeySystem,L"FindFolders",&Opt.FindOpt.FindFolders, 1, 0},
	{1, REG_DWORD,  NKeySystem,L"FindSymLinks",&Opt.FindOpt.FindSymLinks, 1, 0},
	{1, REG_DWORD,  NKeySystem,L"UseFilterInSearch",&Opt.FindOpt.UseFilter,0,0},
	{1, REG_DWORD,  NKeySystem,L"FindCodePage",&Opt.FindCodePage, CP_AUTODETECT, 0},
	{0, REG_DWORD,  NKeySystem,L"SubstPluginPrefix",&Opt.SubstPluginPrefix, 0, 0},
	{0, REG_DWORD,  NKeySystem,L"CmdHistoryRule",&Opt.CmdHistoryRule,0, 0},
	{0, REG_DWORD,  NKeySystem,L"SetAttrFolderRules",&Opt.SetAttrFolderRules,1, 0},
	{0, REG_DWORD,  NKeySystem,L"MaxPositionCache",&Opt.MaxPositionCache,MAX_POSITIONS, 0},
	{0, REG_SZ,     NKeySystem,L"ConsoleDetachKey", &strKeyNameConsoleDetachKey, 0, L"CtrlAltTab"},
	{0, REG_DWORD,  NKeySystem,L"SilentLoadPlugin",  &Opt.LoadPlug.SilentLoadPlugin, 0, 0},
	{1, REG_DWORD,  NKeySystem,L"OEMPluginsSupport",  &Opt.LoadPlug.OEMPluginsSupport, 1, 0},
	{1, REG_DWORD,  NKeySystem,L"ScanSymlinks",  &Opt.LoadPlug.ScanSymlinks, 1, 0},
	{1, REG_DWORD,  NKeySystem,L"MultiMakeDir",&Opt.MultiMakeDir,0, 0},
	{0, REG_DWORD,  NKeySystem,L"FlagPosixSemantics", &Opt.FlagPosixSemantics, 1, 0},
	{0, REG_DWORD,  NKeySystem,L"MsWheelDelta", &Opt.MsWheelDelta, 1, 0},
	{0, REG_DWORD,  NKeySystem,L"MsWheelDeltaView", &Opt.MsWheelDeltaView, 1, 0},
	{0, REG_DWORD,  NKeySystem,L"MsWheelDeltaEdit", &Opt.MsWheelDeltaEdit, 1, 0},
	{0, REG_DWORD,  NKeySystem,L"MsWheelDeltaHelp", &Opt.MsWheelDeltaHelp, 1, 0},
	{0, REG_DWORD,  NKeySystem,L"MsHWheelDelta", &Opt.MsHWheelDelta, 1, 0},
	{0, REG_DWORD,  NKeySystem,L"MsHWheelDeltaView", &Opt.MsHWheelDeltaView, 1, 0},
	{0, REG_DWORD,  NKeySystem,L"MsHWheelDeltaEdit", &Opt.MsHWheelDeltaEdit, 1, 0},
	{0, REG_DWORD,  NKeySystem,L"SubstNameRule", &Opt.SubstNameRule, 2, 0},
	{0, REG_DWORD,  NKeySystem,L"ShowCheckingFile", &Opt.ShowCheckingFile, 0, 0},
	{0, REG_DWORD,  NKeySystem,L"DelThreadPriority", &Opt.DelThreadPriority, THREAD_PRIORITY_NORMAL, 0},
  {0, REG_SZ,     NKeySystem,L"QuotedSymbols",&Opt.strQuotedSymbols, 0, L" &()[]{}^=;!'+,`\xA0"}, //xA0 => 160 =>oem(0xFF)
	{0, REG_DWORD,  NKeySystem,L"QuotedName",&Opt.QuotedName,0xFFFFFFFFU, 0},
	//{0, REG_DWORD,  NKeySystem,L"CPAJHefuayor",&Opt.strCPAJHefuayor,0, 0},
	{0, REG_DWORD,  NKeySystem,L"CloseConsoleRule",&Opt.CloseConsoleRule,1, 0},
	{0, REG_DWORD,  NKeySystem,L"PluginMaxReadData",&Opt.PluginMaxReadData,0x20000, 0},
	{1, REG_DWORD,  NKeySystem,L"CloseCDGate",&Opt.CloseCDGate,1, 0},
	{0, REG_DWORD,  NKeySystem,L"UseNumPad",&Opt.UseNumPad,1, 0},
	{0, REG_DWORD,  NKeySystem,L"CASRule",&Opt.CASRule,0xFFFFFFFFU, 0},
	{0, REG_DWORD,  NKeySystem,L"AllCtrlAltShiftRule",&Opt.AllCtrlAltShiftRule,0x0000FFFF, 0},
	{1, REG_DWORD,  NKeySystem,L"ScanJunction",&Opt.ScanJunction,1, 0},
	{0, REG_DWORD,  NKeySystem,L"UsePrintManager",&Opt.UsePrintManager,1, 0},
	{1, REG_DWORD,  NKeySystem,L"ElevationMode",&Opt.ElevationMode,0xFFFFFFFFU, 0},
	{0, REG_DWORD,  NKeySystem,L"WindowMode",&Opt.WindowMode, 0, 0},

	{0, REG_DWORD,  NKeySystemNowell,L"MoveRO",&Opt.Nowell.MoveRO,1, 0},

	{0, REG_DWORD,  NKeySystemExecutor,L"RestoreCP",&Opt.RestoreCPAfterExecute,1, 0},
	{0, REG_DWORD,  NKeySystemExecutor,L"UseAppPath",&Opt.ExecuteUseAppPath,1, 0},
	{0, REG_DWORD,  NKeySystemExecutor,L"ShowErrorMessage",&Opt.ExecuteShowErrorMessage,1, 0},
	{0, REG_SZ,     NKeySystemExecutor,L"BatchType",&Opt.strExecuteBatchType,0,constBatchExt},
	{0, REG_DWORD,  NKeySystemExecutor,L"FullTitle",&Opt.ExecuteFullTitle,0, 0},

	{0, REG_DWORD,  NKeyPanelTree,L"MinTreeCount",&Opt.Tree.MinTreeCount, 4, 0},
	{0, REG_DWORD,  NKeyPanelTree,L"TreeFileAttr",&Opt.Tree.TreeFileAttr, FILE_ATTRIBUTE_HIDDEN, 0},
	{0, REG_DWORD,  NKeyPanelTree,L"LocalDisk",&Opt.Tree.LocalDisk, 2, 0},
	{0, REG_DWORD,  NKeyPanelTree,L"NetDisk",&Opt.Tree.NetDisk, 2, 0},
	{0, REG_DWORD,  NKeyPanelTree,L"RemovableDisk",&Opt.Tree.RemovableDisk, 2, 0},
	{0, REG_DWORD,  NKeyPanelTree,L"NetPath",&Opt.Tree.NetPath, 2, 0},
	{1, REG_DWORD,  NKeyPanelTree,L"AutoChangeFolder",&Opt.Tree.AutoChangeFolder,0, 0}, // ???

	{0, REG_DWORD,  NKeyHelp,L"ActivateURL",&Opt.HelpURLRules,1, 0},

	{1, REG_SZ,     NKeyLanguage,L"Help",&Opt.strHelpLanguage, 0, L"English"},

	{1, REG_DWORD,  NKeyConfirmations,L"Copy",&Opt.Confirm.Copy,1, 0},
	{1, REG_DWORD,  NKeyConfirmations,L"Move",&Opt.Confirm.Move,1, 0},
	{1, REG_DWORD,  NKeyConfirmations,L"RO",&Opt.Confirm.RO,1, 0},
	{1, REG_DWORD,  NKeyConfirmations,L"Drag",&Opt.Confirm.Drag,1, 0},
	{1, REG_DWORD,  NKeyConfirmations,L"Delete",&Opt.Confirm.Delete,1, 0},
	{1, REG_DWORD,  NKeyConfirmations,L"DeleteFolder",&Opt.Confirm.DeleteFolder,1, 0},
	{1, REG_DWORD,  NKeyConfirmations,L"Esc",&Opt.Confirm.Esc,1, 0},
	{1, REG_DWORD,  NKeyConfirmations,L"RemoveConnection",&Opt.Confirm.RemoveConnection,1, 0},
	{1, REG_DWORD,  NKeyConfirmations,L"RemoveSUBST",&Opt.Confirm.RemoveSUBST,1, 0},
	{1, REG_DWORD,  NKeyConfirmations,L"DetachVHD",&Opt.Confirm.DetachVHD,1, 0},
	{1, REG_DWORD,  NKeyConfirmations,L"RemoveHotPlug",&Opt.Confirm.RemoveHotPlug,1, 0},
	{1, REG_DWORD,  NKeyConfirmations,L"AllowReedit",&Opt.Confirm.AllowReedit,1, 0},
	{1, REG_DWORD,  NKeyConfirmations,L"HistoryClear",&Opt.Confirm.HistoryClear,1, 0},
	{1, REG_DWORD,  NKeyConfirmations,L"Exit",&Opt.Confirm.Exit,1, 0},
	{0, REG_DWORD,  NKeyConfirmations,L"EscTwiceToInterrupt",&Opt.Confirm.EscTwiceToInterrupt,0, 0},

	{1, REG_DWORD,  NKeyPluginConfirmations, L"OpenFilePluginNew", &Opt.PluginConfirm.OpenFilePluginNew, 0, 0},
	{1, REG_DWORD,  NKeyPluginConfirmations, L"OpenFilePlugin", &Opt.PluginConfirm.OpenFilePlugin, 0, 0},
	{1, REG_DWORD,  NKeyPluginConfirmations, L"StandardAssociation", &Opt.PluginConfirm.StandardAssociation, 0, 0},
	{1, REG_DWORD,  NKeyPluginConfirmations, L"EvenIfOnlyOnePlugin", &Opt.PluginConfirm.EvenIfOnlyOnePlugin, 0, 0},
	{1, REG_DWORD,  NKeyPluginConfirmations, L"SetFindList", &Opt.PluginConfirm.SetFindList, 0, 0},
	{1, REG_DWORD,  NKeyPluginConfirmations, L"Prefix", &Opt.PluginConfirm.Prefix, 0, 0},

	{0, REG_DWORD,  NKeyPanel,L"ShellRightLeftArrowsRule",&Opt.ShellRightLeftArrowsRule,0, 0},
	{1, REG_DWORD,  NKeyPanel,L"ShowHidden",&Opt.ShowHidden,1, 0},
	{1, REG_DWORD,  NKeyPanel,L"Highlight",&Opt.Highlight,1, 0},
	{1, REG_DWORD,  NKeyPanel,L"SortFolderExt",&Opt.SortFolderExt,0, 0},
	{1, REG_DWORD,  NKeyPanel,L"SelectFolders",&Opt.SelectFolders,0, 0},
	{1, REG_DWORD,  NKeyPanel,L"ReverseSort",&Opt.ReverseSort,1, 0},
	{0, REG_DWORD,  NKeyPanel,L"RightClickRule",&Opt.PanelRightClickRule,2, 0},
	{0, REG_DWORD,  NKeyPanel,L"CtrlFRule",&Opt.PanelCtrlFRule,1, 0},
	{0, REG_DWORD,  NKeyPanel,L"CtrlAltShiftRule",&Opt.PanelCtrlAltShiftRule,0, 0},
	{0, REG_DWORD,  NKeyPanel,L"RememberLogicalDrives",&Opt.RememberLogicalDrives, 0, 0},
	{1, REG_DWORD,  NKeyPanel,L"AutoUpdateLimit",&Opt.AutoUpdateLimit, 0, 0},

	{1, REG_DWORD,  NKeyPanelLeft,L"Type",&Opt.LeftPanel.Type,0, 0},
	{1, REG_DWORD,  NKeyPanelLeft,L"Visible",&Opt.LeftPanel.Visible,1, 0},
	{1, REG_DWORD,  NKeyPanelLeft,L"Focus",&Opt.LeftPanel.Focus,1, 0},
	{1, REG_DWORD,  NKeyPanelLeft,L"ViewMode",&Opt.LeftPanel.ViewMode,2, 0},
	{1, REG_DWORD,  NKeyPanelLeft,L"SortMode",&Opt.LeftPanel.SortMode,1, 0},
	{1, REG_DWORD,  NKeyPanelLeft,L"SortOrder",&Opt.LeftPanel.SortOrder,1, 0},
	{1, REG_DWORD,  NKeyPanelLeft,L"SortGroups",&Opt.LeftPanel.SortGroups,0, 0},
	{1, REG_DWORD,  NKeyPanelLeft,L"ShortNames",&Opt.LeftPanel.ShowShortNames,0, 0},
	{1, REG_DWORD,  NKeyPanelLeft,L"NumericSort",&Opt.LeftPanel.NumericSort,0, 0},
	{1, REG_SZ,     NKeyPanelLeft,L"Folder",&Opt.strLeftFolder, 0, L""},
	{1, REG_SZ,     NKeyPanelLeft,L"CurFile",&Opt.strLeftCurFile, 0, L""},
	{1, REG_DWORD,  NKeyPanelLeft,L"SelectedFirst",&Opt.LeftSelectedFirst,0,0},
	{1, REG_DWORD,  NKeyPanelLeft,L"DirectoriesFirst",&Opt.LeftPanel.DirectoriesFirst,1,0},

	{1, REG_DWORD,  NKeyPanelRight,L"Type",&Opt.RightPanel.Type,0, 0},
	{1, REG_DWORD,  NKeyPanelRight,L"Visible",&Opt.RightPanel.Visible,1, 0},
	{1, REG_DWORD,  NKeyPanelRight,L"Focus",&Opt.RightPanel.Focus,0, 0},
	{1, REG_DWORD,  NKeyPanelRight,L"ViewMode",&Opt.RightPanel.ViewMode,2, 0},
	{1, REG_DWORD,  NKeyPanelRight,L"SortMode",&Opt.RightPanel.SortMode,1, 0},
	{1, REG_DWORD,  NKeyPanelRight,L"SortOrder",&Opt.RightPanel.SortOrder,1, 0},
	{1, REG_DWORD,  NKeyPanelRight,L"SortGroups",&Opt.RightPanel.SortGroups,0, 0},
	{1, REG_DWORD,  NKeyPanelRight,L"ShortNames",&Opt.RightPanel.ShowShortNames,0, 0},
	{1, REG_DWORD,  NKeyPanelRight,L"NumericSort",&Opt.RightPanel.NumericSort,0, 0},
	{1, REG_SZ,     NKeyPanelRight,L"Folder",&Opt.strRightFolder, 0,L""},
	{1, REG_SZ,     NKeyPanelRight,L"CurFile",&Opt.strRightCurFile, 0,L""},
	{1, REG_DWORD,  NKeyPanelRight,L"SelectedFirst",&Opt.RightSelectedFirst,0, 0},
	{1, REG_DWORD,  NKeyPanelRight,L"DirectoriesFirst",&Opt.RightPanel.DirectoriesFirst,1,0},

	{1, REG_DWORD,  NKeyPanelLayout,L"ColumnTitles",&Opt.ShowColumnTitles,1, 0},
	{1, REG_DWORD,  NKeyPanelLayout,L"StatusLine",&Opt.ShowPanelStatus,1, 0},
	{1, REG_DWORD,  NKeyPanelLayout,L"TotalInfo",&Opt.ShowPanelTotals,1, 0},
	{1, REG_DWORD,  NKeyPanelLayout,L"FreeInfo",&Opt.ShowPanelFree,0, 0},
	{1, REG_DWORD,  NKeyPanelLayout,L"Scrollbar",&Opt.ShowPanelScrollbar,0, 0},
	{0, REG_DWORD,  NKeyPanelLayout,L"ScrollbarMenu",&Opt.ShowMenuScrollbar,1, 0},
	{1, REG_DWORD,  NKeyPanelLayout,L"ScreensNumber",&Opt.ShowScreensNumber,1, 0},
	{1, REG_DWORD,  NKeyPanelLayout,L"SortMode",&Opt.ShowSortMode,1, 0},

	{1, REG_DWORD,  NKeyLayout,L"LeftHeightDecrement",&Opt.LeftHeightDecrement,0, 0},
	{1, REG_DWORD,  NKeyLayout,L"RightHeightDecrement",&Opt.RightHeightDecrement,0, 0},
	{1, REG_DWORD,  NKeyLayout,L"WidthDecrement",&Opt.WidthDecrement,0, 0},
	{1, REG_DWORD,  NKeyLayout,L"FullscreenHelp",&Opt.FullScreenHelp,0, 0},

	{1, REG_SZ,     NKeyDescriptions,L"ListNames",&Opt.Diz.strListNames, 0, L"Descript.ion,Files.bbs"},
	{1, REG_DWORD,  NKeyDescriptions,L"UpdateMode",&Opt.Diz.UpdateMode,DIZ_UPDATE_IF_DISPLAYED, 0},
	{1, REG_DWORD,  NKeyDescriptions,L"ROUpdate",&Opt.Diz.ROUpdate,0, 0},
	{1, REG_DWORD,  NKeyDescriptions,L"SetHidden",&Opt.Diz.SetHidden,1, 0},
	{1, REG_DWORD,  NKeyDescriptions,L"StartPos",&Opt.Diz.StartPos,0, 0},
	{1, REG_DWORD,  NKeyDescriptions,L"AnsiByDefault",&Opt.Diz.AnsiByDefault,0, 0},
	{1, REG_DWORD,  NKeyDescriptions,L"SaveInUTF",&Opt.Diz.SaveInUTF,0, 0},

	{0, REG_DWORD,  NKeyKeyMacros,L"MacroReuseRules",&Opt.Macro.MacroReuseRules,0, 0},
	{0, REG_SZ,     NKeyKeyMacros,L"DateFormat",&Opt.Macro.strDateFormat, 0, L"%a %b %d %H:%M:%S %Z %Y"},
	{0, REG_SZ,     NKeyKeyMacros,L"CONVFMT",&Opt.Macro.strMacroCONVFMT, 0, L"%.6g"},
	{0, REG_DWORD,  NKeyKeyMacros,L"CallPluginRules",&Opt.Macro.CallPluginRules,0, 0},

	{0, REG_DWORD,  NKeyPolicies,L"ShowHiddenDrives",&Opt.Policies.ShowHiddenDrives,1, 0},
	{0, REG_DWORD,  NKeyPolicies,L"DisabledOptions",&Opt.Policies.DisabledOptions,0, 0},


	{0, REG_DWORD,  NKeySystem,L"ExcludeCmdHistory",&Opt.ExcludeCmdHistory,0, 0}, //AN

	{1, REG_DWORD,  NKeyCodePages,L"CPMenuMode",&Opt.CPMenuMode,0,0},

	{1, REG_SZ,     NKeySystem,L"FolderInfo",&Opt.InfoPanel.strFolderInfoFiles, 0, L"DirInfo,File_Id.diz,Descript.ion,ReadMe.*,Read.Me"},
	{1, REG_DWORD,  NKeyPanelInfo,L"InfoComputerNameFormat",&Opt.InfoPanel.ComputerNameFormat, ComputerNamePhysicalNetBIOS, 0},
	{1, REG_DWORD,  NKeyPanelInfo,L"InfoUserNameFormat",&Opt.InfoPanel.UserNameFormat, NameUserPrincipal, 0},
};

void ReadConfig()
{
	DWORD OptPolicies_ShowHiddenDrives,  OptPolicies_DisabledOptions;
	string strKeyNameFromReg;
	string strPersonalPluginsPath;
	size_t I;

	/* <ПРЕПРОЦЕССЫ> *************************************************** */
	// "Вспомним" путь для дополнительного поиска плагинов
	SetRegRootKey(HKEY_LOCAL_MACHINE);
	GetRegKey(NKeySystem,L"TemplatePluginsPath",strPersonalPluginsPath,L"");
	OptPolicies_ShowHiddenDrives=GetRegKey(NKeyPolicies,L"ShowHiddenDrives",1)&1;
	OptPolicies_DisabledOptions=GetRegKey(NKeyPolicies,L"DisabledOptions",0);
	SetRegRootKey(HKEY_CURRENT_USER);
	GetRegKey(NKeySystem,L"PersonalPluginsPath",Opt.LoadPlug.strPersonalPluginsPath, strPersonalPluginsPath);
	bool ExplicitWindowMode=Opt.WindowMode!=FALSE;
	//Opt.LCIDSort=LOCALE_USER_DEFAULT; // проинициализируем на всякий случай
	/* *************************************************** </ПРЕПРОЦЕССЫ> */

	for (I=0; I < ARRAYSIZE(CFG); ++I)
	{
		switch (CFG[I].ValType)
		{
			case REG_DWORD:
				GetRegKey(CFG[I].KeyName, CFG[I].ValName,*(int *)CFG[I].ValPtr,(DWORD)CFG[I].DefDWord);
				break;
			case REG_SZ:
				GetRegKey(CFG[I].KeyName, CFG[I].ValName,*(string *)CFG[I].ValPtr,CFG[I].DefStr);
				break;
			case REG_BINARY:
				int Size=GetRegKey(CFG[I].KeyName, CFG[I].ValName,(BYTE*)CFG[I].ValPtr,(BYTE*)CFG[I].DefStr,CFG[I].DefDWord);

				if (Size && Size < (int)CFG[I].DefDWord)
					memset(((BYTE*)CFG[I].ValPtr)+Size,0,CFG[I].DefDWord-Size);

				break;
		}
	}

	/* <ПОСТПРОЦЕССЫ> *************************************************** */
	if (Opt.ShowMenuBar)
		Opt.ShowMenuBar=1;

	if (Opt.PluginMaxReadData < 0x1000) // || Opt.PluginMaxReadData > 0x80000)
		Opt.PluginMaxReadData=0x20000;

	if(ExplicitWindowMode)
	{
		Opt.WindowMode=TRUE;
	}

	Opt.HelpTabSize=8; // пока жестко пропишем...
	//   Уточняем алгоритм "взятия" палитры.
	for (I=COL_PRIVATEPOSITION_FOR_DIF165ABOVE-COL_FIRSTPALETTECOLOR+1;
	        I < (COL_LASTPALETTECOLOR-COL_FIRSTPALETTECOLOR);
	        ++I)
	{
		if (!Palette[I])
		{
			if (!Palette[COL_PRIVATEPOSITION_FOR_DIF165ABOVE-COL_FIRSTPALETTECOLOR])
				Palette[I]=DefaultPalette[I];
			else if (Palette[COL_PRIVATEPOSITION_FOR_DIF165ABOVE-COL_FIRSTPALETTECOLOR] == 1)
				Palette[I]=BlackPalette[I];

			/*
			else
			  в других случаях нифига ничего не делаем, т.к.
			  есть другие палитры...
			*/
		}
	}

	Opt.ViOpt.ViewerIsWrap&=1;
	Opt.ViOpt.ViewerWrap&=1;

	// Исключаем случайное стирание разделителей ;-)
	if (Opt.strWordDiv.IsEmpty())
		Opt.strWordDiv = WordDiv0;

	// Исключаем случайное стирание разделителей
	if (Opt.XLat.strWordDivForXlat.IsEmpty())
		Opt.XLat.strWordDivForXlat = WordDivForXlat0;

	Opt.PanelRightClickRule%=3;
	Opt.PanelCtrlAltShiftRule%=3;
	Opt.ConsoleDetachKey=KeyNameToKey(strKeyNameConsoleDetachKey);

	if (Opt.EdOpt.TabSize<1 || Opt.EdOpt.TabSize>512)
		Opt.EdOpt.TabSize=8;

	if (Opt.ViOpt.TabSize<1 || Opt.ViOpt.TabSize>512)
		Opt.ViOpt.TabSize=8;

	GetRegKey(NKeyKeyMacros,L"KeyRecordCtrlDot",strKeyNameFromReg,szCtrlDot);

	if ((Opt.Macro.KeyMacroCtrlDot=KeyNameToKey(strKeyNameFromReg)) == (DWORD)-1)
		Opt.Macro.KeyMacroCtrlDot=KEY_CTRLDOT;

	GetRegKey(NKeyKeyMacros,L"KeyRecordCtrlShiftDot",strKeyNameFromReg,szCtrlShiftDot);

	if ((Opt.Macro.KeyMacroCtrlShiftDot=KeyNameToKey(strKeyNameFromReg)) == (DWORD)-1)
		Opt.Macro.KeyMacroCtrlShiftDot=KEY_CTRLSHIFTDOT;

	Opt.EdOpt.strWordDiv = Opt.strWordDiv;
	FileList::ReadPanelModes();
	CtrlObject->EditorPosCache->Read(L"Editor\\LastPositions");
	CtrlObject->ViewerPosCache->Read(L"Viewer\\LastPositions");
	// уточняем системную политику
	// для дисков HKCU может только отменять показ
	Opt.Policies.ShowHiddenDrives&=OptPolicies_ShowHiddenDrives;
	// для опций HKCU может только добавлять блокироку пунктов
	Opt.Policies.DisabledOptions|=OptPolicies_DisabledOptions;

	if (Opt.strExecuteBatchType.IsEmpty()) // предохраняемся
		Opt.strExecuteBatchType=constBatchExt;

	{
		Opt.XLat.CurrentLayout=0;
		memset(Opt.XLat.Layouts,0,sizeof(Opt.XLat.Layouts));
		string strXLatLayouts;
		GetRegKey(NKeyXLat,L"Layouts",strXLatLayouts,L"");

		if (!strXLatLayouts.IsEmpty())
		{
			wchar_t *endptr;
			const wchar_t *ValPtr;
			UserDefinedList DestList;
			DestList.SetParameters(L';',0,ULF_UNIQUE);
			DestList.Set(strXLatLayouts);
			I=0;

			while (nullptr!=(ValPtr=DestList.GetNext()))
			{
				DWORD res=(DWORD)wcstoul(ValPtr, &endptr, 16);
				Opt.XLat.Layouts[I]=(HKL)(LONG_PTR)(HIWORD(res)? res : MAKELONG(res,res));
				++I;

				if (I >= ARRAYSIZE(Opt.XLat.Layouts))
					break;
			}

			if (I <= 1) // если указано меньше двух - "откключаем" эту
				Opt.XLat.Layouts[0]=0;
		}
	}

	memset(Opt.FindOpt.OutColumnTypes,0,sizeof(Opt.FindOpt.OutColumnTypes));
	memset(Opt.FindOpt.OutColumnWidths,0,sizeof(Opt.FindOpt.OutColumnWidths));
	memset(Opt.FindOpt.OutColumnWidthType,0,sizeof(Opt.FindOpt.OutColumnWidthType));
	Opt.FindOpt.OutColumnCount=0;


	if (!Opt.FindOpt.strSearchOutFormat.IsEmpty())
	{
		if (Opt.FindOpt.strSearchOutFormatWidth.IsEmpty())
			Opt.FindOpt.strSearchOutFormatWidth=L"0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0";
		TextToViewSettings(Opt.FindOpt.strSearchOutFormat.CPtr(),Opt.FindOpt.strSearchOutFormatWidth.CPtr(),
                                  Opt.FindOpt.OutColumnTypes,Opt.FindOpt.OutColumnWidths,Opt.FindOpt.OutColumnWidthType,
                                  Opt.FindOpt.OutColumnCount);
	}

	/* *************************************************** </ПОСТПРОЦЕССЫ> */
}


void SaveConfig(int Ask)
{
	if (Opt.Policies.DisabledOptions&0x20000) // Bit 17 - Сохранить параметры
		return;

	if (Ask && Message(0,2,MSG(MSaveSetupTitle),MSG(MSaveSetupAsk1),MSG(MSaveSetupAsk2),MSG(MSaveSetup),MSG(MCancel)))
		return;

	string strTemp;
	/* <ПРЕПРОЦЕССЫ> *************************************************** */
	Panel *LeftPanel=CtrlObject->Cp()->LeftPanel;
	Panel *RightPanel=CtrlObject->Cp()->RightPanel;
	Opt.LeftPanel.Focus=LeftPanel->GetFocus();
	Opt.LeftPanel.Visible=LeftPanel->IsVisible();
	Opt.RightPanel.Focus=RightPanel->GetFocus();
	Opt.RightPanel.Visible=RightPanel->IsVisible();

	if (LeftPanel->GetMode()==NORMAL_PANEL)
	{
		Opt.LeftPanel.Type=LeftPanel->GetType();
		Opt.LeftPanel.ViewMode=LeftPanel->GetViewMode();
		Opt.LeftPanel.SortMode=LeftPanel->GetSortMode();
		Opt.LeftPanel.SortOrder=LeftPanel->GetSortOrder();
		Opt.LeftPanel.SortGroups=LeftPanel->GetSortGroups();
		Opt.LeftPanel.ShowShortNames=LeftPanel->GetShowShortNamesMode();
		Opt.LeftPanel.NumericSort=LeftPanel->GetNumericSort();
		Opt.LeftSelectedFirst=LeftPanel->GetSelectedFirstMode();
		Opt.LeftPanel.DirectoriesFirst=LeftPanel->GetDirectoriesFirst();
	}

	LeftPanel->GetCurDir(Opt.strLeftFolder);
	LeftPanel->GetCurBaseName(Opt.strLeftCurFile, strTemp);

	if (RightPanel->GetMode()==NORMAL_PANEL)
	{
		Opt.RightPanel.Type=RightPanel->GetType();
		Opt.RightPanel.ViewMode=RightPanel->GetViewMode();
		Opt.RightPanel.SortMode=RightPanel->GetSortMode();
		Opt.RightPanel.SortOrder=RightPanel->GetSortOrder();
		Opt.RightPanel.SortGroups=RightPanel->GetSortGroups();
		Opt.RightPanel.ShowShortNames=RightPanel->GetShowShortNamesMode();
		Opt.RightPanel.NumericSort=RightPanel->GetNumericSort();
		Opt.RightSelectedFirst=RightPanel->GetSelectedFirstMode();
		Opt.RightPanel.DirectoriesFirst=RightPanel->GetDirectoriesFirst();
	}

	RightPanel->GetCurDir(Opt.strRightFolder);
	RightPanel->GetCurBaseName(Opt.strRightCurFile,strTemp);
	CtrlObject->HiFiles->SaveHiData();
	/* *************************************************** </ПРЕПРОЦЕССЫ> */
	SetRegKey(NKeySystem,L"PersonalPluginsPath",Opt.LoadPlug.strPersonalPluginsPath);
	SetRegKey(NKeyLanguage,L"Main",Opt.strLanguage);

	for (size_t I=0; I < ARRAYSIZE(CFG); ++I)
	{
		if (CFG[I].IsSave)
			switch (CFG[I].ValType)
			{
				case REG_DWORD:
					SetRegKey(CFG[I].KeyName, CFG[I].ValName,*(int *)CFG[I].ValPtr);
					break;
				case REG_SZ:
					SetRegKey(CFG[I].KeyName, CFG[I].ValName,*(string *)CFG[I].ValPtr);
					break;
				case REG_BINARY:
					SetRegKey(CFG[I].KeyName, CFG[I].ValName,(BYTE*)CFG[I].ValPtr,CFG[I].DefDWord);
					break;
			}
	}

	/* <ПОСТПРОЦЕССЫ> *************************************************** */
	FileFilter::SaveFilters();
	FileList::SavePanelModes();

	if (Ask)
		CtrlObject->Macro.SaveMacros();

	/* *************************************************** </ПОСТПРОЦЕССЫ> */
}
