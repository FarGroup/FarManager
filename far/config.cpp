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

Options Opt={};

// Стандартный набор разделителей
static const wchar_t *WordDiv0 = L"~!%^&*()+|{}:\"<>?`-=\\[];',./";

// Стандартный набор разделителей для функции Xlat
static const wchar_t *WordDivForXlat0=L" \t!#$%^&*()+|=\\/@?";

const wchar_t *constBatchExt=L".BAT;.CMD;";

string strKeyNameConsoleDetachKey;
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
const wchar_t NKeyKeyMacros[]=L"KeyMacros";
const wchar_t NKeyPolicies[]=L"Policies";
const wchar_t NKeyFileFilter[]=L"OperationsFilter";
const wchar_t NKeyCodePages[]=L"CodePages";
const wchar_t NKeyVMenu[]=L"VMenu";
const wchar_t NKeyCommandHistory[]=L"History.CommandHistory";
const wchar_t NKeyViewEditHistory[]=L"History.ViewEditHistory";
const wchar_t NKeyFolderHistory[]=L"History.FolderHistory";
const wchar_t NKeyDialogHistory[]=L"History.DialogHistory";

const wchar_t NParamHistoryCount[]=L"HistoryCount";

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

	Builder.AddCheckbox(MConfigSaveHistory, &Opt.SaveHistory);
	Builder.AddCheckbox(MConfigSaveFoldersHistory, &Opt.SaveFoldersHistory);
	Builder.AddCheckbox(MConfigSaveViewHistory, &Opt.SaveViewHistory);
	Builder.AddCheckbox(MConfigRegisteredTypes, &Opt.UseRegisteredTypes);
	Builder.AddCheckbox(MConfigCloseCDGate, &Opt.CloseCDGate);
	Builder.AddCheckbox(MConfigUpdateEnvironment, &Opt.UpdateEnvironment);
	Builder.AddText(MConfigElevation);
	Builder.AddCheckbox(MConfigElevationModify, &Opt.ElevationMode, ELEVATION_MODIFY_REQUEST)->Indent(4);
	Builder.AddCheckbox(MConfigElevationRead, &Opt.ElevationMode, ELEVATION_READ_REQUEST)->Indent(4);
	Builder.AddCheckbox(MConfigElevationUsePrivileges, &Opt.ElevationMode, ELEVATION_USE_PRIVILEGES)->Indent(4);
	Builder.AddCheckbox(MConfigAutoSave, &Opt.AutoSaveSetup);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		Opt.CurrentElevationMode = Opt.ElevationMode;
	}
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
	Builder.AddCheckbox(MConfigHighlightColumnSeparator, &Opt.HighlightColumnSeparator);
	Builder.AddCheckbox(MConfigDoubleGlobalColumnSeparator, &Opt.DoubleGlobalColumnSeparator);
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
	Builder.AddComboBox((int *) &Opt.VMenu.LBtnClick, 40, CAListItems, ARRAYSIZE(CAListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddText(MConfigVMenuRBtnClick);
	Builder.AddComboBox((int *) &Opt.VMenu.RBtnClick, 40, CAListItems, ARRAYSIZE(CAListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddText(MConfigVMenuMBtnClick);
	Builder.AddComboBox((int *) &Opt.VMenu.MBtnClick, 40, CAListItems, ARRAYSIZE(CAListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddOKCancel();
	Builder.ShowDialog();
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
#ifndef NO_WRAPPER
	Builder.AddCheckbox(MPluginsManagerOEMPluginsSupport, &Opt.LoadPlug.OEMPluginsSupport);
#endif // NO_WRAPPER
	Builder.AddCheckbox(MPluginsManagerScanSymlinks, &Opt.LoadPlug.ScanSymlinks);
	Builder.AddSeparator(MPluginConfirmationTitle);
	DialogItemEx *ConfirmOFP = Builder.AddCheckbox(MPluginsManagerOFP, &Opt.PluginConfirm.OpenFilePlugin);
	ConfirmOFP->Flags|=DIF_3STATE;
	DialogItemEx *StandardAssoc = Builder.AddCheckbox(MPluginsManagerStdAssoc, &Opt.PluginConfirm.StandardAssociation);
	DialogItemEx *EvenIfOnlyOne = Builder.AddCheckbox(MPluginsManagerEvenOne, &Opt.PluginConfirm.EvenIfOnlyOnePlugin);
	StandardAssoc->Indent(2);
	EvenIfOnlyOne->Indent(4);

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
	DialogItemEx *SavePos = Builder.AddCheckbox(MViewConfigSavePos, &Opt.ViOpt.SavePos); // can't be local
	Builder.AddCheckbox(MViewConfigEditAutofocus, &ViOpt.SearchEditFocus);
	DialogItemEx *TabSize = Builder.AddIntEditField(&ViOpt.TabSize, 3);
	Builder.AddTextAfter(TabSize, MViewConfigTabSize);
	Builder.ColumnBreak();
	Builder.AddCheckbox(MViewConfigArrows, &ViOpt.ShowArrows);
	DialogItemEx *SaveShortPos = Builder.AddCheckbox(MViewConfigSaveShortPos, &Opt.ViOpt.SaveShortPos); // can't be local
	Builder.LinkFlags(SavePos, SaveShortPos, DIF_DISABLE);
	Builder.AddCheckbox(MViewConfigVisible0x00, &ViOpt.Visible0x00);
	Builder.AddCheckbox(MViewConfigScrollbar, &ViOpt.ShowScrollbar);
	Builder.EndColumns();

	if (!Local)
	{
		Builder.AddEmptyLine();
		DialogItemEx *MaxLineSize = Builder.AddIntEditField(&Opt.ViOpt.MaxLineSize, 6);
		Builder.AddTextAfter(MaxLineSize, MViewConfigMaxLineSize);
		Builder.AddCheckbox(MViewAutoDetectCodePage, &Opt.ViOpt.AutoDetectCodePage);
		Builder.AddCheckbox(MViewConfigAnsiCodePageAsDefault, &Opt.ViOpt.AnsiCodePageAsDefault);
	}
	Builder.AddOKCancel();
	if (Builder.ShowDialog())
	{
		if (ViOpt.TabSize<1 || ViOpt.TabSize>512)
			ViOpt.TabSize=8;
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
	DWORD ValType;  // TYPE_INTEGER, TYPE_TEXT, TYPE_BLOB
	const wchar_t *KeyName;
	const wchar_t *ValName;
	void *ValPtr;   // адрес переменной, куда помещаем данные
	DWORD DefDWord; // он же размер данных для TYPE_BLOB
	const wchar_t *DefStr;   // строка/данные по умолчанию
} CFG[]=
{
	{1, GeneralConfig::TYPE_BLOB,    NKeyColors, L"CurrentPalette",(char*)Opt.Palette.CurrentPalette,static_cast<DWORD>(Opt.Palette.SizeArrayPalette*sizeof(FarColor)),(const wchar_t*)Opt.Palette.DefaultPalette},

	{1, GeneralConfig::TYPE_INTEGER, NKeyScreen, L"Clock", &Opt.Clock, 1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyScreen, L"ViewerEditorClock",&Opt.ViewerEditorClock,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyScreen, L"KeyBar",&Opt.ShowKeyBar,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyScreen, L"ScreenSaver",&Opt.ScreenSaver, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyScreen, L"ScreenSaverTime",&Opt.ScreenSaverTime,5, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyScreen, L"DeltaXY", &Opt.ScrSize.DeltaXY, 0, 0},

	{1, GeneralConfig::TYPE_INTEGER, NKeyCmdline, L"UsePromptFormat", &Opt.CmdLine.UsePromptFormat,0, 0},
	{1, GeneralConfig::TYPE_TEXT,    NKeyCmdline, L"PromptFormat",&Opt.CmdLine.strPromptFormat, 0, L"$p$g"},
	{1, GeneralConfig::TYPE_INTEGER, NKeyCmdline, L"DelRemovesBlocks", &Opt.CmdLine.DelRemovesBlocks,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyCmdline, L"EditBlock", &Opt.CmdLine.EditBlock,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyCmdline, L"AutoComplete",&Opt.CmdLine.AutoComplete,1, 0},


	{1, GeneralConfig::TYPE_INTEGER, NKeyInterface, L"Mouse",&Opt.Mouse,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyInterface, L"UseVk_oem_x",&Opt.UseVk_oem_x,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyInterface, L"ShowMenuBar",&Opt.ShowMenuBar,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyInterface, L"CursorSize1",&Opt.CursorSize[0],15, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyInterface, L"CursorSize2",&Opt.CursorSize[1],10, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyInterface, L"CursorSize3",&Opt.CursorSize[2],99, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyInterface, L"CursorSize4",&Opt.CursorSize[3],99, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyInterface, L"ShiftsKeyRules",&Opt.ShiftsKeyRules,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyInterface, L"AltF9",&Opt.AltF9, 1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyInterface, L"CtrlPgUp",&Opt.PgUpChangeDisk, 1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyInterface, L"ClearType",&Opt.ClearType, 0, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyInterface, L"ShowTimeoutDelFiles",&Opt.ShowTimeoutDelFiles, 50, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyInterface, L"ShowTimeoutDACLFiles",&Opt.ShowTimeoutDACLFiles, 50, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyInterface, L"FormatNumberSeparators",&Opt.FormatNumberSeparators, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyInterface, L"CopyShowTotal",&Opt.CMOpt.CopyShowTotal,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyInterface,L"DelShowTotal",&Opt.DelOpt.DelShowTotal,0, 0},
	{1, GeneralConfig::TYPE_TEXT,    NKeyInterface,L"TitleAddons",&Opt.strTitleAddons, 0, L"%Ver.%Build %Platform %Admin"},
	{1, GeneralConfig::TYPE_INTEGER, NKeyInterfaceCompletion,L"ShowList",&Opt.AutoComplete.ShowList, 1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyInterfaceCompletion,L"ModalList",&Opt.AutoComplete.ModalList, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyInterfaceCompletion,L"Append",&Opt.AutoComplete.AppendCompletion, 0, 0},

	{1, GeneralConfig::TYPE_TEXT,    NKeyViewer,L"ExternalViewerName",&Opt.strExternalViewer, 0, L""},
	{1, GeneralConfig::TYPE_INTEGER, NKeyViewer,L"UseExternalViewer",&Opt.ViOpt.UseExternalViewer,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyViewer,L"SaveViewerPos",&Opt.ViOpt.SavePos,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyViewer,L"SaveViewerShortPos",&Opt.ViOpt.SaveShortPos,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyViewer,L"AutoDetectCodePage",&Opt.ViOpt.AutoDetectCodePage,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyViewer,L"SearchRegexp",&Opt.ViOpt.SearchRegexp,0, 0},

	{1, GeneralConfig::TYPE_INTEGER, NKeyViewer,L"TabSize",&Opt.ViOpt.TabSize,8, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyViewer,L"ShowKeyBar",&Opt.ViOpt.ShowKeyBar,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyViewer,L"ShowTitleBar",&Opt.ViOpt.ShowTitleBar,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyViewer,L"ShowArrows",&Opt.ViOpt.ShowArrows,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyViewer,L"ShowScrollbar",&Opt.ViOpt.ShowScrollbar,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyViewer,L"IsWrap",&Opt.ViOpt.ViewerIsWrap,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyViewer,L"Wrap",&Opt.ViOpt.ViewerWrap,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyViewer,L"PersistentBlocks",&Opt.ViOpt.PersistentBlocks,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyViewer,L"AnsiCodePageAsDefault",&Opt.ViOpt.AnsiCodePageAsDefault,1, 0},

	{1, GeneralConfig::TYPE_INTEGER, NKeyViewer,L"MaxLineSize",&Opt.ViOpt.MaxLineSize,ViewerOptions::eDefLineSize, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyViewer,L"SearchEditFocus",&Opt.ViOpt.SearchEditFocus,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyViewer,L"Visible0x00",&Opt.ViOpt.Visible0x00,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyViewer,L"ZeroChar",&Opt.ViOpt.ZeroChar,0x00B7, 0}, // middle dot

	{1, GeneralConfig::TYPE_INTEGER, NKeyDialog, L"EditHistory",&Opt.Dialogs.EditHistory,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyDialog, L"EditBlock",&Opt.Dialogs.EditBlock,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyDialog, L"AutoComplete",&Opt.Dialogs.AutoComplete,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyDialog,L"EULBsClear",&Opt.Dialogs.EULBsClear,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyDialog,L"SelectFromHistory",&Opt.Dialogs.SelectFromHistory,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyDialog,L"EditLine",&Opt.Dialogs.EditLine,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyDialog,L"MouseButton",&Opt.Dialogs.MouseButton,0xFFFF, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyDialog,L"DelRemovesBlocks",&Opt.Dialogs.DelRemovesBlocks,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyDialog,L"CBoxMaxHeight",&Opt.Dialogs.CBoxMaxHeight,8, 0},

	{1, GeneralConfig::TYPE_TEXT,    NKeyEditor,L"ExternalEditorName",&Opt.strExternalEditor, 0, L""},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"UseExternalEditor",&Opt.EdOpt.UseExternalEditor,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"ExpandTabs",&Opt.EdOpt.ExpandTabs,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"TabSize",&Opt.EdOpt.TabSize,8, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"PersistentBlocks",&Opt.EdOpt.PersistentBlocks,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"DelRemovesBlocks",&Opt.EdOpt.DelRemovesBlocks,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"AutoIndent",&Opt.EdOpt.AutoIndent,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"SaveEditorPos",&Opt.EdOpt.SavePos,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"SaveEditorShortPos",&Opt.EdOpt.SaveShortPos,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"AutoDetectCodePage",&Opt.EdOpt.AutoDetectCodePage,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"EditorCursorBeyondEOL",&Opt.EdOpt.CursorBeyondEOL,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"ReadOnlyLock",&Opt.EdOpt.ReadOnlyLock,0, 0}, // Вернём назад дефолт 1.65 - не предупреждать и не блокировать
	{0, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"EditorUndoSize",&Opt.EdOpt.UndoSize,0, 0}, // $ 03.12.2001 IS размер буфера undo в редакторе
	{0, GeneralConfig::TYPE_TEXT,    NKeyEditor,L"WordDiv",&Opt.strWordDiv, 0, WordDiv0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"BSLikeDel",&Opt.EdOpt.BSLikeDel,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"EditorF7Rules",&Opt.EdOpt.F7Rules,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"FileSizeLimit",&Opt.EdOpt.FileSizeLimitLo,(DWORD)0, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"FileSizeLimitHi",&Opt.EdOpt.FileSizeLimitHi,(DWORD)0, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"CharCodeBase",&Opt.EdOpt.CharCodeBase,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"AllowEmptySpaceAfterEof", &Opt.EdOpt.AllowEmptySpaceAfterEof,0,0},//skv
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"AnsiCodePageForNewFile",&Opt.EdOpt.AnsiCodePageForNewFile,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"AnsiCodePageAsDefault",&Opt.EdOpt.AnsiCodePageAsDefault,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"ShowKeyBar",&Opt.EdOpt.ShowKeyBar,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"ShowTitleBar",&Opt.EdOpt.ShowTitleBar,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"ShowScrollBar",&Opt.EdOpt.ShowScrollBar,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"EditOpenedForWrite",&Opt.EdOpt.EditOpenedForWrite,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"SearchSelFound",&Opt.EdOpt.SearchSelFound,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"SearchRegexp",&Opt.EdOpt.SearchRegexp,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"SearchPickUpWord",&Opt.EdOpt.SearchPickUpWord,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyEditor,L"ShowWhiteSpace",&Opt.EdOpt.ShowWhiteSpace,0, 0},

	{1, GeneralConfig::TYPE_INTEGER, NKeyXLat,L"Flags",&Opt.XLat.Flags,(DWORD)XLAT_SWITCHKEYBLAYOUT|XLAT_CONVERTALLCMDLINE, 0},
	{1, GeneralConfig::TYPE_TEXT,    NKeyXLat,L"Table1",&Opt.XLat.Table[0],0,L""},
	{1, GeneralConfig::TYPE_TEXT,    NKeyXLat,L"Table2",&Opt.XLat.Table[1],0,L""},
	{1, GeneralConfig::TYPE_TEXT,    NKeyXLat,L"Rules1",&Opt.XLat.Rules[0],0,L""},
	{1, GeneralConfig::TYPE_TEXT,    NKeyXLat,L"Rules2",&Opt.XLat.Rules[1],0,L""},
	{1, GeneralConfig::TYPE_TEXT,    NKeyXLat,L"Rules3",&Opt.XLat.Rules[2],0,L""},
	{1, GeneralConfig::TYPE_TEXT,    NKeyXLat,L"WordDivForXlat",&Opt.XLat.strWordDivForXlat, 0,WordDivForXlat0},

	{0, GeneralConfig::TYPE_INTEGER, NKeyCommandHistory, NParamHistoryCount,&Opt.HistoryCount,512, 0}, //BUGBUG
	{0, GeneralConfig::TYPE_INTEGER, NKeyFolderHistory, NParamHistoryCount,&Opt.FoldersHistoryCount,512, 0}, //BUGBUG
	{0, GeneralConfig::TYPE_INTEGER, NKeyViewEditHistory, NParamHistoryCount,&Opt.ViewHistoryCount,512, 0}, //BUGBUG
	{0, GeneralConfig::TYPE_INTEGER, NKeyDialogHistory, NParamHistoryCount,&Opt.DialogsHistoryCount,512, 0}, //BUGBUG
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"MaxPositionCache",&Opt.MaxPositionCache, 512/*MAX_POSITIONS*/, 0}, //BUGBUG

	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"SaveHistory",&Opt.SaveHistory,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"SaveFoldersHistory",&Opt.SaveFoldersHistory,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"SaveViewHistory",&Opt.SaveViewHistory,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"UseRegisteredTypes",&Opt.UseRegisteredTypes,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"AutoSaveSetup",&Opt.AutoSaveSetup,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"ClearReadOnly",&Opt.ClearReadOnly,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"DeleteToRecycleBin",&Opt.DeleteToRecycleBin,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"DeleteToRecycleBinKillLink",&Opt.DeleteToRecycleBinKillLink,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"WipeSymbol",&Opt.WipeSymbol,0, 0},

	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"UseSystemCopy",&Opt.CMOpt.UseSystemCopy,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"CopySecurityOptions",&Opt.CMOpt.CopySecurityOptions,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"CopyOpened",&Opt.CMOpt.CopyOpened,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem, L"MultiCopy",&Opt.CMOpt.MultiCopy,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"CopyTimeRule",  &Opt.CMOpt.CopyTimeRule, 3, 0},

	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"CreateUppercaseFolders",&Opt.CreateUppercaseFolders,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"DriveMenuMode",&Opt.ChangeDriveMode,DRIVE_SHOW_TYPE|DRIVE_SHOW_PLUGINS|DRIVE_SHOW_SIZE_FLOAT|DRIVE_SHOW_CDROM, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"DriveDisconnetMode",&Opt.ChangeDriveDisconnetMode,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"AutoUpdateRemoteDrive",&Opt.AutoUpdateRemoteDrive,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"FileSearchMode",&Opt.FindOpt.FileSearchMode,FINDAREA_FROM_CURRENT, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"CollectFiles",&Opt.FindOpt.CollectFiles, 1, 0},
	{1, GeneralConfig::TYPE_TEXT,    NKeySystem,L"SearchInFirstSize",&Opt.FindOpt.strSearchInFirstSize, 0, L""},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"FindAlternateStreams",&Opt.FindOpt.FindAlternateStreams,0,0},
	{1, GeneralConfig::TYPE_TEXT,    NKeySystem,L"SearchOutFormat",&Opt.FindOpt.strSearchOutFormat, 0, L"D,S,A"},
	{1, GeneralConfig::TYPE_TEXT,    NKeySystem,L"SearchOutFormatWidth",&Opt.FindOpt.strSearchOutFormatWidth, 0, L"14,13,0"},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"FindFolders",&Opt.FindOpt.FindFolders, 1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"FindSymLinks",&Opt.FindOpt.FindSymLinks, 1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"UseFilterInSearch",&Opt.FindOpt.UseFilter,0,0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"FindCodePage",&Opt.FindCodePage, CP_AUTODETECT, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"SubstPluginPrefix",&Opt.SubstPluginPrefix, 0, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"CmdHistoryRule",&Opt.CmdHistoryRule,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"SetAttrFolderRules",&Opt.SetAttrFolderRules,1, 0},
	{0, GeneralConfig::TYPE_TEXT,    NKeySystem,L"ConsoleDetachKey", &strKeyNameConsoleDetachKey, 0, L"CtrlAltTab"},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"SilentLoadPlugin",  &Opt.LoadPlug.SilentLoadPlugin, 0, 0},
#ifndef NO_WRAPPER
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"OEMPluginsSupport",  &Opt.LoadPlug.OEMPluginsSupport, 1, 0},
#endif // NO_WRAPPER
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"ScanSymlinks",  &Opt.LoadPlug.ScanSymlinks, 1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"MultiMakeDir",&Opt.MultiMakeDir,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"FlagPosixSemantics", &Opt.FlagPosixSemantics, 1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"MsWheelDelta", &Opt.MsWheelDelta, 1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"MsWheelDeltaView", &Opt.MsWheelDeltaView, 1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"MsWheelDeltaEdit", &Opt.MsWheelDeltaEdit, 1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"MsWheelDeltaHelp", &Opt.MsWheelDeltaHelp, 1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"MsHWheelDelta", &Opt.MsHWheelDelta, 1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"MsHWheelDeltaView", &Opt.MsHWheelDeltaView, 1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"MsHWheelDeltaEdit", &Opt.MsHWheelDeltaEdit, 1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"SubstNameRule", &Opt.SubstNameRule, 2, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"ShowCheckingFile", &Opt.ShowCheckingFile, 0, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"DelThreadPriority", &Opt.DelThreadPriority, THREAD_PRIORITY_NORMAL, 0},
	{0, GeneralConfig::TYPE_TEXT,    NKeySystem,L"QuotedSymbols",&Opt.strQuotedSymbols, 0, L" &()[]{}^=;!'+,`\xA0"}, //xA0 => 160 =>oem(0xFF)
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"QuotedName",&Opt.QuotedName,0xFFFFFFFFU, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"CloseConsoleRule",&Opt.CloseConsoleRule,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"PluginMaxReadData",&Opt.PluginMaxReadData,0x20000, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"CloseCDGate",&Opt.CloseCDGate,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"UpdateEnvironment",&Opt.UpdateEnvironment,0,0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"CASRule",&Opt.CASRule,0xFFFFFFFFU, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"AllCtrlAltShiftRule",&Opt.AllCtrlAltShiftRule,0x0000FFFF, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"ScanJunction",&Opt.ScanJunction,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeySystem,L"ElevationMode",&Opt.ElevationMode,0x0FFFFFFFU, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"WindowMode",&Opt.WindowMode, 0, 0},

	{0, GeneralConfig::TYPE_INTEGER, NKeySystemNowell,L"MoveRO",&Opt.Nowell.MoveRO,1, 0},

	{0, GeneralConfig::TYPE_INTEGER, NKeySystemExecutor,L"RestoreCP",&Opt.RestoreCPAfterExecute,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystemExecutor,L"UseAppPath",&Opt.ExecuteUseAppPath,1, 0},
	{0, GeneralConfig::TYPE_TEXT,    NKeySystemExecutor,L"BatchType",&Opt.strExecuteBatchType,0,constBatchExt},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystemExecutor,L"FullTitle",&Opt.ExecuteFullTitle,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeySystemExecutor,L"SilentExternal",&Opt.ExecuteSilentExternal,0, 0},

	{0, GeneralConfig::TYPE_INTEGER, NKeyPanelTree,L"MinTreeCount",&Opt.Tree.MinTreeCount, 4, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyPanelTree,L"TreeFileAttr",&Opt.Tree.TreeFileAttr, FILE_ATTRIBUTE_HIDDEN, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyPanelTree,L"LocalDisk",&Opt.Tree.LocalDisk, 2, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyPanelTree,L"NetDisk",&Opt.Tree.NetDisk, 2, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyPanelTree,L"RemovableDisk",&Opt.Tree.RemovableDisk, 2, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyPanelTree,L"NetPath",&Opt.Tree.NetPath, 2, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelTree,L"AutoChangeFolder",&Opt.Tree.AutoChangeFolder,0, 0}, // ???

	{0, GeneralConfig::TYPE_INTEGER, NKeyHelp,L"ActivateURL",&Opt.HelpURLRules,1, 0},

	{1, GeneralConfig::TYPE_INTEGER, NKeyConfirmations,L"Copy",&Opt.Confirm.Copy,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyConfirmations,L"Move",&Opt.Confirm.Move,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyConfirmations,L"RO",&Opt.Confirm.RO,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyConfirmations,L"Drag",&Opt.Confirm.Drag,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyConfirmations,L"Delete",&Opt.Confirm.Delete,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyConfirmations,L"DeleteFolder",&Opt.Confirm.DeleteFolder,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyConfirmations,L"Esc",&Opt.Confirm.Esc,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyConfirmations,L"RemoveConnection",&Opt.Confirm.RemoveConnection,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyConfirmations,L"RemoveSUBST",&Opt.Confirm.RemoveSUBST,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyConfirmations,L"DetachVHD",&Opt.Confirm.DetachVHD,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyConfirmations,L"RemoveHotPlug",&Opt.Confirm.RemoveHotPlug,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyConfirmations,L"AllowReedit",&Opt.Confirm.AllowReedit,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyConfirmations,L"HistoryClear",&Opt.Confirm.HistoryClear,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyConfirmations,L"Exit",&Opt.Confirm.Exit,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyConfirmations,L"EscTwiceToInterrupt",&Opt.Confirm.EscTwiceToInterrupt,0, 0},

	{1, GeneralConfig::TYPE_INTEGER, NKeyPluginConfirmations, L"OpenFilePlugin", &Opt.PluginConfirm.OpenFilePlugin, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPluginConfirmations, L"StandardAssociation", &Opt.PluginConfirm.StandardAssociation, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPluginConfirmations, L"EvenIfOnlyOnePlugin", &Opt.PluginConfirm.EvenIfOnlyOnePlugin, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPluginConfirmations, L"SetFindList", &Opt.PluginConfirm.SetFindList, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPluginConfirmations, L"Prefix", &Opt.PluginConfirm.Prefix, 0, 0},

	{0, GeneralConfig::TYPE_INTEGER, NKeyPanel,L"ShellRightLeftArrowsRule",&Opt.ShellRightLeftArrowsRule,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanel,L"ShowHidden",&Opt.ShowHidden,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanel,L"Highlight",&Opt.Highlight,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanel,L"SortFolderExt",&Opt.SortFolderExt,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanel,L"SelectFolders",&Opt.SelectFolders,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanel,L"ReverseSort",&Opt.ReverseSort,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyPanel,L"RightClickRule",&Opt.PanelRightClickRule,2, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyPanel,L"CtrlFRule",&Opt.PanelCtrlFRule,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyPanel,L"CtrlAltShiftRule",&Opt.PanelCtrlAltShiftRule,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyPanel,L"RememberLogicalDrives",&Opt.RememberLogicalDrives, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanel,L"AutoUpdateLimit",&Opt.AutoUpdateLimit, 0, 0},

	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLeft,L"Type",&Opt.LeftPanel.Type,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLeft,L"Visible",&Opt.LeftPanel.Visible,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLeft,L"Focus",&Opt.LeftPanel.Focus,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLeft,L"ViewMode",&Opt.LeftPanel.ViewMode,2, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLeft,L"SortMode",&Opt.LeftPanel.SortMode,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLeft,L"SortOrder",&Opt.LeftPanel.SortOrder,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLeft,L"SortGroups",&Opt.LeftPanel.SortGroups,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLeft,L"ShortNames",&Opt.LeftPanel.ShowShortNames,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLeft,L"NumericSort",&Opt.LeftPanel.NumericSort,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLeft,L"CaseSensitiveSort",&Opt.LeftPanel.CaseSensitiveSort,0, 0},
	{1, GeneralConfig::TYPE_TEXT,    NKeyPanelLeft,L"Folder",&Opt.strLeftFolder, 0, L""},
	{1, GeneralConfig::TYPE_TEXT,    NKeyPanelLeft,L"CurFile",&Opt.strLeftCurFile, 0, L""},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLeft,L"SelectedFirst",&Opt.LeftSelectedFirst,0,0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLeft,L"DirectoriesFirst",&Opt.LeftPanel.DirectoriesFirst,1,0},

	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelRight,L"Type",&Opt.RightPanel.Type,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelRight,L"Visible",&Opt.RightPanel.Visible,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelRight,L"Focus",&Opt.RightPanel.Focus,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelRight,L"ViewMode",&Opt.RightPanel.ViewMode,2, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelRight,L"SortMode",&Opt.RightPanel.SortMode,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelRight,L"SortOrder",&Opt.RightPanel.SortOrder,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelRight,L"SortGroups",&Opt.RightPanel.SortGroups,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelRight,L"ShortNames",&Opt.RightPanel.ShowShortNames,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelRight,L"NumericSort",&Opt.RightPanel.NumericSort,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelRight,L"CaseSensitiveSort",&Opt.RightPanel.CaseSensitiveSort,0, 0},
	{1, GeneralConfig::TYPE_TEXT,    NKeyPanelRight,L"Folder",&Opt.strRightFolder, 0,L""},
	{1, GeneralConfig::TYPE_TEXT,    NKeyPanelRight,L"CurFile",&Opt.strRightCurFile, 0,L""},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelRight,L"SelectedFirst",&Opt.RightSelectedFirst,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelRight,L"DirectoriesFirst",&Opt.RightPanel.DirectoriesFirst,1,0},

	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLayout,L"ColumnTitles",&Opt.ShowColumnTitles,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLayout,L"StatusLine",&Opt.ShowPanelStatus,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLayout,L"TotalInfo",&Opt.ShowPanelTotals,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLayout,L"FreeInfo",&Opt.ShowPanelFree,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLayout,L"Scrollbar",&Opt.ShowPanelScrollbar,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyPanelLayout,L"ScrollbarMenu",&Opt.ShowMenuScrollbar,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLayout,L"ScreensNumber",&Opt.ShowScreensNumber,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLayout,L"SortMode",&Opt.ShowSortMode,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLayout,L"ColoredGlobalColumnSeparator",&Opt.HighlightColumnSeparator,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelLayout,L"DoubleGlobalColumnSeparator",&Opt.DoubleGlobalColumnSeparator,0, 0},

	{1, GeneralConfig::TYPE_INTEGER, NKeyLayout,L"LeftHeightDecrement",&Opt.LeftHeightDecrement,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyLayout,L"RightHeightDecrement",&Opt.RightHeightDecrement,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyLayout,L"WidthDecrement",&Opt.WidthDecrement,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyLayout,L"FullscreenHelp",&Opt.FullScreenHelp,0, 0},

	{1, GeneralConfig::TYPE_TEXT,    NKeyDescriptions,L"ListNames",&Opt.Diz.strListNames, 0, L"Descript.ion,Files.bbs"},
	{1, GeneralConfig::TYPE_INTEGER, NKeyDescriptions,L"UpdateMode",&Opt.Diz.UpdateMode,DIZ_UPDATE_IF_DISPLAYED, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyDescriptions,L"ROUpdate",&Opt.Diz.ROUpdate,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyDescriptions,L"SetHidden",&Opt.Diz.SetHidden,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyDescriptions,L"StartPos",&Opt.Diz.StartPos,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyDescriptions,L"AnsiByDefault",&Opt.Diz.AnsiByDefault,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyDescriptions,L"SaveInUTF",&Opt.Diz.SaveInUTF,0, 0},

	{0, GeneralConfig::TYPE_INTEGER, NKeyKeyMacros,L"MacroReuseRules",&Opt.Macro.MacroReuseRules,0, 0},
	{0, GeneralConfig::TYPE_TEXT,    NKeyKeyMacros,L"DateFormat",&Opt.Macro.strDateFormat, 0, L"%a %b %d %H:%M:%S %Z %Y"},
	{0, GeneralConfig::TYPE_TEXT,    NKeyKeyMacros,L"CONVFMT",&Opt.Macro.strMacroCONVFMT, 0, L"%.6g"},
	{0, GeneralConfig::TYPE_INTEGER, NKeyKeyMacros,L"CallPluginRules",&Opt.Macro.CallPluginRules,0, 0},

	{0, GeneralConfig::TYPE_INTEGER, NKeyPolicies,L"ShowHiddenDrives",&Opt.Policies.ShowHiddenDrives,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, NKeyPolicies,L"DisabledOptions",&Opt.Policies.DisabledOptions,0, 0},

	{0, GeneralConfig::TYPE_INTEGER, NKeySystem,L"ExcludeCmdHistory",&Opt.ExcludeCmdHistory,0, 0}, //AN

	{1, GeneralConfig::TYPE_INTEGER, NKeyCodePages,L"CPMenuMode",&Opt.CPMenuMode,0,0},
	{1, GeneralConfig::TYPE_TEXT,    NKeyCodePages,L"NoAutoDetectCP",&Opt.strNoAutoDetectCP,0,L""},

	{1, GeneralConfig::TYPE_TEXT,    NKeySystem,L"FolderInfo",&Opt.InfoPanel.strFolderInfoFiles, 0, L"DirInfo,File_Id.diz,Descript.ion,ReadMe.*,Read.Me"},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelInfo,L"InfoComputerNameFormat",&Opt.InfoPanel.ComputerNameFormat, ComputerNamePhysicalNetBIOS, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyPanelInfo,L"InfoUserNameFormat",&Opt.InfoPanel.UserNameFormat, NameUserPrincipal, 0},

	{1, GeneralConfig::TYPE_INTEGER, NKeyVMenu,L"LBtnClick",&Opt.VMenu.LBtnClick, VMENUCLICK_CANCEL, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyVMenu,L"RBtnClick",&Opt.VMenu.RBtnClick, VMENUCLICK_CANCEL, 0},
	{1, GeneralConfig::TYPE_INTEGER, NKeyVMenu,L"MBtnClick",&Opt.VMenu.MBtnClick, VMENUCLICK_APPLY, 0},
};

void ReadConfig()
{
	/* <ПРЕПРОЦЕССЫ> *************************************************** */
	string strGlobalUserMenuDir;
	strGlobalUserMenuDir.ReleaseBuffer(GetPrivateProfileString(L"General", L"GlobalUserMenuDir", g_strFarPath, strGlobalUserMenuDir.GetBuffer(NT_MAX_PATH), NT_MAX_PATH, g_strFarINI));
	apiExpandEnvironmentStrings(strGlobalUserMenuDir, Opt.GlobalUserMenuDir);
	ConvertNameToFull(Opt.GlobalUserMenuDir,Opt.GlobalUserMenuDir);
	AddEndSlash(Opt.GlobalUserMenuDir);

	/* BUGBUG??
	SetRegRootKey(HKEY_LOCAL_MACHINE);
	DWORD OptPolicies_ShowHiddenDrives=GetRegKey(NKeyPolicies,L"ShowHiddenDrives",1)&1;
	DWORD OptPolicies_DisabledOptions=GetRegKey(NKeyPolicies,L"DisabledOptions",0);
	SetRegRootKey(HKEY_CURRENT_USER);
	*/

	GeneralCfg->GetValue(NKeyLanguage, L"Help", Opt.strHelpLanguage, Opt.strLanguage);

	bool ExplicitWindowMode=Opt.WindowMode!=FALSE;
	/* *************************************************** </ПРЕПРОЦЕССЫ> */

	for (size_t I=0; I < ARRAYSIZE(CFG); ++I)
	{
		switch (CFG[I].ValType)
		{
			case GeneralConfig::TYPE_INTEGER:
				GeneralCfg->GetValue(CFG[I].KeyName, CFG[I].ValName,(DWORD *)CFG[I].ValPtr,(DWORD)CFG[I].DefDWord);
				break;
			case GeneralConfig::TYPE_TEXT:
				GeneralCfg->GetValue(CFG[I].KeyName, CFG[I].ValName,*(string *)CFG[I].ValPtr,CFG[I].DefStr);
				break;
			case GeneralConfig::TYPE_BLOB:
				int Size=GeneralCfg->GetValue(CFG[I].KeyName, CFG[I].ValName,(char *)CFG[I].ValPtr,(int)CFG[I].DefDWord,(const char *)CFG[I].DefStr);

				if (Size && Size < (int)CFG[I].DefDWord)
					memset(((BYTE*)CFG[I].ValPtr)+Size,0,CFG[I].DefDWord-Size);

				break;
		}
	}

	/* <ПОСТПРОЦЕССЫ> *************************************************** */

	Opt.CurrentElevationMode = Opt.ElevationMode;

	if (Opt.ShowMenuBar)
		Opt.ShowMenuBar=1;

	if (Opt.PluginMaxReadData < 0x1000) // || Opt.PluginMaxReadData > 0x80000)
		Opt.PluginMaxReadData=0x20000;

	if(ExplicitWindowMode)
	{
		Opt.WindowMode=TRUE;
	}

	Opt.HelpTabSize=8; // пока жестко пропишем...

	Opt.ViOpt.ViewerIsWrap &= 1;
	Opt.ViOpt.ViewerWrap &= 1;
	if (!Opt.ViOpt.MaxLineSize)
		Opt.ViOpt.MaxLineSize = ViewerOptions::eDefLineSize;
	else if (Opt.ViOpt.MaxLineSize < ViewerOptions::eMinLineSize)
		Opt.ViOpt.MaxLineSize = ViewerOptions::eMinLineSize;
	else if (Opt.ViOpt.MaxLineSize > ViewerOptions::eMaxLineSize)
		Opt.ViOpt.MaxLineSize = ViewerOptions::eMaxLineSize;
	Opt.ViOpt.SearchEditFocus &= 1;
	Opt.ViOpt.Visible0x00 &= 1;

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

	string strKeyNameFromCfg;

	GeneralCfg->GetValue(NKeyKeyMacros,L"KeyRecordCtrlDot",strKeyNameFromCfg,szCtrlDot);
	if ((Opt.Macro.KeyMacroCtrlDot=KeyNameToKey(strKeyNameFromCfg)) == (DWORD)-1)
		Opt.Macro.KeyMacroCtrlDot=KEY_CTRLDOT;

	GeneralCfg->GetValue(NKeyKeyMacros,L"KeyRecordRCtrlDot",strKeyNameFromCfg,szRCtrlDot);
	if ((Opt.Macro.KeyMacroRCtrlDot=KeyNameToKey(strKeyNameFromCfg)) == (DWORD)-1)
		Opt.Macro.KeyMacroRCtrlDot=KEY_RCTRLDOT;

	GeneralCfg->GetValue(NKeyKeyMacros,L"KeyRecordCtrlShiftDot",strKeyNameFromCfg,szCtrlShiftDot);
	if ((Opt.Macro.KeyMacroCtrlShiftDot=KeyNameToKey(strKeyNameFromCfg)) == (DWORD)-1)
		Opt.Macro.KeyMacroCtrlShiftDot=KEY_CTRLSHIFTDOT;

	GeneralCfg->GetValue(NKeyKeyMacros,L"KeyRecordRCtrlShiftDot",strKeyNameFromCfg,szRCtrlShiftDot);
	if ((Opt.Macro.KeyMacroRCtrlShiftDot=KeyNameToKey(strKeyNameFromCfg)) == (DWORD)-1)
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

	if (Opt.strExecuteBatchType.IsEmpty()) // предохраняемся
		Opt.strExecuteBatchType=constBatchExt;

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
		string strXLatLayouts;
		GeneralCfg->GetValue(NKeyXLat,L"Layouts",strXLatLayouts,L"");

		if (!strXLatLayouts.IsEmpty())
		{
			wchar_t *endptr;
			const wchar_t *ValPtr;
			UserDefinedList DestList;
			DestList.SetParameters(L';',0,ULF_UNIQUE);
			DestList.Set(strXLatLayouts);
			size_t I=0;

			while (nullptr!=(ValPtr=DestList.GetNext()))
			{
				DWORD res=(DWORD)wcstoul(ValPtr, &endptr, 16);
				Opt.XLat.Layouts[I]=(HKL)(INT_PTR)(HIWORD(res)? res : MAKELONG(res,res));
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
		TextToViewSettings(Opt.FindOpt.strSearchOutFormat.CPtr(),Opt.FindOpt.strSearchOutFormatWidth.CPtr(),false,
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
		Opt.LeftPanel.CaseSensitiveSort=LeftPanel->GetCaseSensitiveSort();
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
		Opt.RightPanel.CaseSensitiveSort=RightPanel->GetCaseSensitiveSort();
		Opt.RightSelectedFirst=RightPanel->GetSelectedFirstMode();
		Opt.RightPanel.DirectoriesFirst=RightPanel->GetDirectoriesFirst();
	}

	RightPanel->GetCurDir(Opt.strRightFolder);
	RightPanel->GetCurBaseName(Opt.strRightCurFile,strTemp);
	CtrlObject->HiFiles->SaveHiData();
	/* *************************************************** </ПРЕПРОЦЕССЫ> */

	GeneralCfg->BeginTransaction();

	GeneralCfg->SetValue(NKeyLanguage,L"Main",Opt.strLanguage);
	GeneralCfg->SetValue(NKeyLanguage, L"Help", Opt.strHelpLanguage);

	for (size_t I=0; I < ARRAYSIZE(CFG); ++I)
	{
		if (CFG[I].IsSave)
			switch (CFG[I].ValType)
			{
				case GeneralConfig::TYPE_INTEGER:
					GeneralCfg->SetValue(CFG[I].KeyName, CFG[I].ValName,*(int *)CFG[I].ValPtr);
					break;
				case GeneralConfig::TYPE_TEXT:
					GeneralCfg->SetValue(CFG[I].KeyName, CFG[I].ValName,*(string *)CFG[I].ValPtr);
					break;
				case GeneralConfig::TYPE_BLOB:
					GeneralCfg->SetValue(CFG[I].KeyName, CFG[I].ValName,(char*)CFG[I].ValPtr,CFG[I].DefDWord);
					break;
			}
	}

	GeneralCfg->EndTransaction();

	/* <ПОСТПРОЦЕССЫ> *************************************************** */
	FileFilter::SaveFilters();
	FileList::SavePanelModes();

	if (Ask)
		CtrlObject->Macro.SaveMacros();

	/* *************************************************** </ПОСТПРОЦЕССЫ> */
}
