/*
mkdir.cpp

Создание каталога
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
#include "mkdir.hpp"

// Internal:
#include "filepanels.hpp"
#include "panel.hpp"
#include "treelist.hpp"
#include "ctrlobj.hpp"
#include "message.hpp"
#include "config.hpp"
#include "dialog.hpp"
#include "pathmix.hpp"
#include "uuids.far.dialogs.hpp"
#include "flink.hpp"
#include "stddlg.hpp"
#include "lang.hpp"
#include "cvtname.hpp"
#include "global.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common.hpp"
#include "common/enum_tokens.hpp"
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

enum
{
	MKDIR_BORDER,
	MKDIR_TEXT,
	MKDIR_EDIT,
	MKDIR_SEPARATOR0,
	MKDIR_TEXT_LINKTYPE,
	MKDIR_COMBOBOX_LINKTYPE,
	MKDIR_TEXT_LINKPATH,
	MKDIR_EDIT_LINKPATH,
	MKDIR_CHECKBOX,
	MKDIR_SEPARATOR2,
	MKDIR_OK,
	MKDIR_CANCEL,

	MKDIR_COUNT
};

static intptr_t MkDirDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	switch (Msg)
	{
		case DN_EDITCHANGE:
			{
				if (Param1 == MKDIR_COMBOBOX_LINKTYPE)
				{
					Dlg->SendMessage(DM_ENABLE, MKDIR_EDIT_LINKPATH, ToPtr(Param2 != nullptr));
				}
			}
			break;

	default:
		break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

static bool create_directory(string_view const Directory, bool& SkipErrors)
{
	return retryable_ui_operation([&]{ return os::fs::create_directory(Directory); }, Directory, lng::MCannotCreateFolder, SkipErrors);
}

static bool add_reparse_point(string_view const Directory, string_view const Target, ReparsePointTypes const Type, bool& SkipErrors)
{
	return retryable_ui_operation([&]{ return CreateReparsePoint(Target, Directory, Type); }, Directory, lng::MCopyCannotCreateLink, SkipErrors);
}

static void ShellMakeDirImpl(Panel *SrcPanel)
{
	FarList ComboList{ sizeof(ComboList) };
	FarListItem LinkTypeItems[3]{};
	ComboList.ItemsNumber=std::size(LinkTypeItems);
	ComboList.Items=LinkTypeItems;
	ComboList.Items[0].Text=msg(lng::MMakeFolderLinkNone).c_str();
	ComboList.Items[1].Text=msg(lng::MMakeFolderLinkJunction).c_str();
	ComboList.Items[2].Text=msg(lng::MMakeFolderLinkSymlink).c_str();
	ComboList.Items[0].Flags|=LIF_SELECTED;

	auto MkDirDlg = MakeDialogItems<MKDIR_COUNT>(
	{
		{DI_DOUBLEBOX, {{3,  1}, {72, 10}}, DIF_NONE, msg(lng::MMakeFolderTitle), },
		{DI_TEXT,      {{5,  2}, {0,  2 }}, DIF_NONE, msg(lng::MCreateFolder), },
		{DI_EDIT,      {{5,  3}, {70, 3 }}, DIF_FOCUS | DIF_EDITEXPAND | DIF_HISTORY | DIF_USELASTHISTORY | DIF_EDITPATH, },
		{DI_TEXT,      {{-1, 4}, {0,  4 }}, DIF_SEPARATOR, },
		{DI_TEXT,      {{5,  5}, {0,  5 }}, DIF_NONE, msg(lng::MMakeFolderLinkType), },
		{DI_COMBOBOX,  {{20, 5}, {70, 5 }}, DIF_DROPDOWNLIST | DIF_LISTNOAMPERSAND | DIF_LISTWRAPMODE, },
		{DI_TEXT,      {{5,  6}, {0,  6 }}, DIF_NONE, msg(lng::MMakeFolderLinkTarget), },
		{DI_EDIT,      {{20, 6}, {70, 6 }}, DIF_DISABLE | DIF_EDITEXPAND | DIF_HISTORY | DIF_USELASTHISTORY | DIF_EDITPATH, },
		{DI_CHECKBOX,  {{5,  7}, {0,  7 }}, DIF_NONE, msg(lng::MMultiMakeDir), },
		{DI_TEXT,      {{-1, 8}, {0,  8 }}, DIF_SEPARATOR, },
		{DI_BUTTON,    {{0,  9}, {0,  9 }}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MOk), },
		{DI_BUTTON,    {{0,  9}, {0,  9 }}, DIF_CENTERGROUP, msg(lng::MCancel), },
	});

	MkDirDlg[MKDIR_EDIT].strHistory = L"NewFolder"sv;
	MkDirDlg[MKDIR_EDIT_LINKPATH].strHistory = L"NewFolderLinkTarget"sv;
	MkDirDlg[MKDIR_COMBOBOX_LINKTYPE].ListItems=&ComboList;
	MkDirDlg[MKDIR_CHECKBOX].Selected = Global->Opt->MultiMakeDir;

	const auto Dlg = Dialog::create(MkDirDlg, MkDirDlgProc);
	Dlg->SetPosition({ -1, -1, 76, 12 });
	Dlg->SetHelp(L"MakeFolder"sv);
	Dlg->SetId(MakeFolderId);
	Dlg->Process();

	if (Dlg->GetExitCode() != MKDIR_OK)
		return;


	string strDirName=MkDirDlg[MKDIR_EDIT].strData;
	Global->Opt->MultiMakeDir = MkDirDlg[MKDIR_CHECKBOX].Selected == BSTATE_CHECKED;

	// это по поводу создания одиночного каталога, который
	// начинается с пробела! Чтобы ручками не заключать
	// такой каталог в кавычки
	if (Global->Opt->MultiMakeDir && strDirName.find_first_of(L";,\""sv) == string::npos)
	{
		inplace::quote_space(strDirName);
	}

	// нужно создать только ОДИН каталог
	if (!Global->Opt->MultiMakeDir)
	{
		// уберем все лишние кавычки
		// возьмем в кавычки, т.к. могут быть разделители
		inplace::quote_normalise(strDirName);
	}

	string LastCreatedPath;
	bool SkipAll = false;
	auto EmptyList = true;

	for (const auto& Path: enum_tokens_with_quotes_t<with_trim>(std::move(strDirName), L",;"sv))
	{
		if (Path.empty())
			continue;

		EmptyList = false;
		// TODO: almost the same code in dirmix::CreatePath()

		LastCreatedPath = Path;

		strDirName = ConvertNameToFull(Path);
		DeleteEndSlash(strDirName);
		bool Created = false;

		size_t DirOffset = 0;
		ParsePath(strDirName, &DirOffset);

		for (const auto i: std::views::iota(DirOffset, strDirName.size() + 1))
		{
			if (i != strDirName.size() && !path::is_separator(strDirName[i]))
				continue;

			const auto Part = string_view(strDirName).substr(0, i);

			if (os::fs::is_directory(Part) && i != strDirName.size()) // skip all intermediate dirs, but not last.
				continue;

			Created = create_directory(Part, SkipAll);

			if(Created)
				TreeList::AddTreeName(Part);
		}

		if (!Created)
			continue;

		if (!MkDirDlg[MKDIR_COMBOBOX_LINKTYPE].ListPos)
			continue;

		const auto strTarget = unquote(MkDirDlg[MKDIR_EDIT_LINKPATH].strData);

		add_reparse_point(strDirName, strTarget, MkDirDlg[MKDIR_COMBOBOX_LINKTYPE].ListPos == 1? RP_JUNCTION : RP_SYMLINKDIR, SkipAll);
	}

	if (EmptyList)
	{
		Message(MSG_WARNING,
			msg(lng::MWarning),
			{
				msg(lng::MIncorrectDirList)
			},
			{ lng::MOk });
		return;
	}

	SrcPanel->Update(UPDATE_KEEP_SELECTION);

	if (!LastCreatedPath.empty())
	{
		const auto pos = FindSlash(LastCreatedPath);
		if (pos != string::npos)
		{
			LastCreatedPath.resize(pos);
		}

		SrcPanel->GoToFile(LastCreatedPath);
	}

	SrcPanel->Redraw();
	const auto AnotherPanel = Global->CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
	if (AnotherPanel->NeedUpdatePanel(SrcPanel) || AnotherPanel->GetType() == panel_type::QVIEW_PANEL)
	{
		AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
		AnotherPanel->Redraw();
	}
}

void ShellMakeDir(Panel* SrcPanel)
{
	try
	{
		ShellMakeDirImpl(SrcPanel);
	}
	catch (operation_cancelled const&)
	{
		// Nop
	}
}
