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

#include "headers.hpp"
#pragma hdrstop

#include "mkdir.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "treelist.hpp"
#include "ctrlobj.hpp"
#include "message.hpp"
#include "config.hpp"
#include "dialog.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "DlgGuid.hpp"
#include "flink.hpp"
#include "stddlg.hpp"
#include "language.hpp"

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
};

intptr_t MkDirDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
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

void ShellMakeDir(Panel *SrcPanel)
{
	FarList ComboList={sizeof(FarList)};
	FarListItem LinkTypeItems[3]={};
	ComboList.ItemsNumber=std::size(LinkTypeItems);
	ComboList.Items=LinkTypeItems;
	ComboList.Items[0].Text=MSG(MMakeFolderLinkNone);
	ComboList.Items[1].Text=MSG(MMakeFolderLinkJunction);
	ComboList.Items[2].Text=MSG(MMakeFolderLinkSymlink);
	ComboList.Items[0].Flags|=LIF_SELECTED;

	FarDialogItem MkDirDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,72,10,0,nullptr,nullptr,0,MSG(MMakeFolderTitle)},
		{DI_TEXT,     5,2, 0,2,0,nullptr,nullptr,0,MSG(MCreateFolder)},
		{DI_EDIT,     5,3,70,3,0,L"NewFolder",nullptr,DIF_FOCUS|DIF_EDITEXPAND|DIF_HISTORY|DIF_USELASTHISTORY|DIF_EDITPATH,L""},
		{DI_TEXT,    -1,4, 0,4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_TEXT,     5,5, 0,5,0,nullptr,nullptr,0,MSG(MMakeFolderLinkType)},
		{DI_COMBOBOX,20,5,70,5,0,nullptr,nullptr,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND|DIF_LISTWRAPMODE,L""},
		{DI_TEXT,     5,6, 0,6,0,nullptr,nullptr,0,MSG(MMakeFolderLinkTarget)},
		{DI_EDIT,    20,6,70,6,0,L"NewFolderLinkTarget",nullptr,DIF_DISABLE|DIF_EDITEXPAND|DIF_HISTORY|DIF_USELASTHISTORY|DIF_EDITPATH,L""},
		{DI_CHECKBOX, 5,7, 0,7,Global->Opt->MultiMakeDir,nullptr,nullptr,0,MSG(MMultiMakeDir)},
		{DI_TEXT,    -1,8, 0,8,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,   0,9, 0,9,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,   0,9, 0,9,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
	};
	auto MkDirDlg = MakeDialogItemsEx(MkDirDlgData);
	MkDirDlg[MKDIR_COMBOBOX_LINKTYPE].ListItems=&ComboList;
	const auto Dlg = Dialog::create(MkDirDlg, MkDirDlgProc);
	Dlg->SetPosition(-1,-1,76,12);
	Dlg->SetHelp(L"MakeFolder");
	Dlg->SetId(MakeFolderId);
	Dlg->Process();

	if (Dlg->GetExitCode()==MKDIR_OK)
	{
		string strDirName=MkDirDlg[MKDIR_EDIT].strData;
		Global->Opt->MultiMakeDir = MkDirDlg[MKDIR_CHECKBOX].Selected == BSTATE_CHECKED;

		// это по поводу создания одиночного каталога, который
		// начинается с пробела! Чтобы ручками не заключать
		// такой каталог в кавычки
		if (Global->Opt->MultiMakeDir && strDirName.find_first_of(L";,\"") == string::npos)
		{
			QuoteSpaceOnly(strDirName);
		}

		// нужно создать только ОДИН каталог
		if (!Global->Opt->MultiMakeDir)
		{
			// уберем все лишние кавычки
			// возьмем в кавычки, т.к. могут быть разделители
			InsertQuote(Unquote(strDirName));
		}

		const auto DirList = split<std::vector<string>>(strDirName, STLF_UNIQUE);
		if (DirList.empty())
		{
			Message(MSG_WARNING, 1, MSG(MWarning), MSG(MIncorrectDirList), MSG(MOk));
			return;
		}

		string strOriginalDirName;
		bool SkipAll = false;
		for (const auto& i: DirList)
		{
			// TODO: almost the same code in dirmix::CreatePath()

			strOriginalDirName = i;
			strDirName = ConvertNameToFull(i);
			DeleteEndSlash(strDirName);
			bool bSuccess = false;

			size_t DirOffset = 0;
			ParsePath(strDirName, &DirOffset);
			string Part;
			Part.reserve(strDirName.size());

			for (size_t j = DirOffset; j <= strDirName.size(); ++j)
			{
				if (j == strDirName.size() || IsSlash(strDirName[j]))
				{
					Part = strDirName.substr(0, j);
					if (!os::fs::exists(Part) || j == strDirName.size()) // skip all intermediate dirs, but not last.
					{
						while((bSuccess = os::CreateDirectory(Part, nullptr)) == false && !SkipAll)
						{
							Global->CatchError();
							const auto Result = OperationFailed(strOriginalDirName, MError, MSG(MCannotCreateFolder));
							if (Result == operation::retry)
							{
								continue;
							}
							else if(Result == operation::skip)
							{
								break;
							}
							else if(Result == operation::skip_all)
							{
								SkipAll = true;
								break;
							}
							else if (Result == operation::cancel)
							{
								return;
							}
						}
						if(bSuccess)
						{
							TreeList::AddTreeName(Part);
						}
					}
				}
			}

			if (bSuccess)
			{
				if(MkDirDlg[MKDIR_COMBOBOX_LINKTYPE].ListPos)
				{
					string strTarget=MkDirDlg[MKDIR_EDIT_LINKPATH].strData;
					Unquote(strTarget);
					while(!CreateReparsePoint(strTarget, strDirName, MkDirDlg[MKDIR_COMBOBOX_LINKTYPE].ListPos==1?RP_JUNCTION:RP_SYMLINKDIR) && !SkipAll)
					{
						Global->CatchError();
						const auto Result = OperationFailed(strDirName, MError, MSG(MCopyCannotCreateLink));
						if (Result == operation::retry)
						{
							continue;
						}
						else if (Result == operation::skip)
						{
							break;
						}
						else if(Result == operation::skip_all)
						{
							SkipAll = true;
							break;
						}
						else if (Result == operation::cancel)
						{
							return;
						}
					}
				}
			}
		}

		SrcPanel->Update(UPDATE_KEEP_SELECTION);

		if (!strOriginalDirName.empty())
		{
			const auto pos = FindSlash(strOriginalDirName);
			if (pos != string::npos)
			{
				strOriginalDirName.resize(pos);
			}

			SrcPanel->GoToFile(strOriginalDirName);
		}

		SrcPanel->Redraw();
		const auto AnotherPanel = Global->CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
		if (AnotherPanel->NeedUpdatePanel(SrcPanel) || AnotherPanel->GetType() == panel_type::QVIEW_PANEL)
		{
			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
			AnotherPanel->Redraw();
		}
	}
}
