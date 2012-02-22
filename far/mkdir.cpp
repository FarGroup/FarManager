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
#include "udlist.hpp"
#include "message.hpp"
#include "config.hpp"
#include "dialog.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "dirmix.hpp"
#include "DlgGuid.hpp"
#include "flink.hpp"

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

INT_PTR WINAPI MkDirDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	switch (Msg)
	{
		case DN_LISTCHANGE:
			{
				if (Param1 == MKDIR_COMBOBOX_LINKTYPE)
				{
					SendDlgMessage(hDlg, DM_ENABLE, MKDIR_EDIT_LINKPATH, ToPtr(Param2 != 0));
				}
			}
			break;
		case DN_CLOSE:
		{
			if (Param1==MKDIR_OK)
			{
				string strDirName=reinterpret_cast<LPCWSTR>(SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,MKDIR_EDIT,0));
				Opt.MultiMakeDir=(SendDlgMessage(hDlg,DM_GETCHECK,MKDIR_CHECKBOX,0)==BSTATE_CHECKED);

				// это по поводу создания одиночного каталога, который
				// начинается с пробела! Чтобы ручками не заключать
				// такой каталог в кавычки
				if (Opt.MultiMakeDir && !wcspbrk(strDirName,L";,\""))
				{
					QuoteSpaceOnly(strDirName);
				}

				// нужно создать только ОДИН каталог
				if (!Opt.MultiMakeDir)
				{
					// уберем все лишние кавычки
					Unquote(strDirName);
					// возьмем в кавычки, т.к. могут быть разделители
					InsertQuote(strDirName);
				}

				UserDefinedList* pDirList=reinterpret_cast<UserDefinedList*>(SendDlgMessage(hDlg,DM_GETDLGDATA,0,0));

				if (!pDirList->Set(strDirName))
				{
					Message(MSG_WARNING,1,MSG(MWarning),MSG(MIncorrectDirList),MSG(MOk));
					return FALSE;
				}
			}
		}
		break;
	default:
		break;
	}

	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

void ShellMakeDir(Panel *SrcPanel)
{
	FarList ComboList;
	FarListItem LinkTypeItems[3]={};
	ComboList.ItemsNumber=ARRAYSIZE(LinkTypeItems);
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
		{DI_TEXT,     0,4, 0,4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_TEXT,     5,5, 0,5,0,nullptr,nullptr,0,MSG(MMakeFolderLinkType)},
		{DI_COMBOBOX,20,5,70,5,0,nullptr,nullptr,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND|DIF_LISTWRAPMODE,L""},
		{DI_TEXT,     5,6, 0,6,0,nullptr,nullptr,0,MSG(MMakeFolderLinkTarget)},
		{DI_EDIT,    20,6,70,6,0,L"NewFolderLinkTarget",nullptr,DIF_DISABLE|DIF_EDITEXPAND|DIF_HISTORY|DIF_USELASTHISTORY|DIF_EDITPATH,L""},
		{DI_CHECKBOX, 5,7, 0,7,Opt.MultiMakeDir,nullptr,nullptr,0,MSG(MMultiMakeDir)},
		{DI_TEXT,     0,8, 0,8,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,   0,9, 0,9,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,   0,9, 0,9,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
	};
	MakeDialogItemsEx(MkDirDlgData,MkDirDlg);
	MkDirDlg[MKDIR_COMBOBOX_LINKTYPE].ListItems=&ComboList;
	UserDefinedList DirList(0,0,ULF_UNIQUE);
	Dialog Dlg(MkDirDlg,ARRAYSIZE(MkDirDlg),MkDirDlgProc,&DirList);
	Dlg.SetPosition(-1,-1,76,12);
	Dlg.SetHelp(L"MakeFolder");
	Dlg.SetId(MakeFolderId);
	Dlg.Process();

	if (Dlg.GetExitCode()==MKDIR_OK)
	{
		string strDirName=MkDirDlg[MKDIR_EDIT].strData;
		string strOriginalDirName;
		const wchar_t *OneDir;
		DirList.Reset();

		while (nullptr!=(OneDir=DirList.GetNext()))
		{
			strDirName = OneDir;
			strOriginalDirName = strDirName;

			//Unquote(DirName);
			if (Opt.CreateUppercaseFolders && !IsCaseMixed(strDirName))
				strDirName.Upper();

			DeleteEndSlash(strDirName,true);
			wchar_t* lpwszDirName = strDirName.GetBuffer();
			bool bSuccess = false;

			if(HasPathPrefix(lpwszDirName))
			{
				lpwszDirName += 4;
			}
			for (wchar_t *ChPtr=lpwszDirName; *ChPtr; ChPtr++)
			{
				if (IsSlash(*ChPtr))
				{
					WCHAR Ch = ChPtr[1];
					ChPtr[1] = 0;
					if (*lpwszDirName)
					{
						string _strDirName(lpwszDirName);
						if (apiGetFileAttributes(_strDirName) == INVALID_FILE_ATTRIBUTES && apiCreateDirectory(_strDirName,nullptr))
						{
							TreeList::AddTreeName(_strDirName);
							bSuccess = true;
						}
					}

					ChPtr[1] = Ch;
				}
			}

			strDirName.ReleaseBuffer();
			BOOL bSuccess2;
			bool bSkip=false;

			while (!(bSuccess2=apiCreateDirectory(strDirName,nullptr)))
			{
				int LastError=GetLastError();

				if (LastError==ERROR_ALREADY_EXISTS || LastError==ERROR_BAD_PATHNAME ||
				        LastError==ERROR_INVALID_NAME || LastError == ERROR_DIRECTORY)
				{
					int ret;

					if (DirList.IsEmpty())
						ret=Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotCreateFolder),strOriginalDirName,MSG(MCancel));
					else
						ret=Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),MSG(MCannotCreateFolder),strOriginalDirName,MSG(MOk),MSG(MSkip));

					bSkip = ret==1;

					if (bSuccess || bSkip)
						break;
					else
						return;
				}
				else
				{
					int ret;

					if (DirList.IsEmpty())
					{
						ret=Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),MSG(MCannotCreateFolder),strOriginalDirName,MSG(MRetry),MSG(MCancel));
					}
					else
					{
						ret=Message(MSG_WARNING|MSG_ERRORTYPE,3,MSG(MError),MSG(MCannotCreateFolder),strOriginalDirName,MSG(MRetry),MSG(MSkip),MSG(MCancel));
						bSkip = ret==1;
					}

					if (ret)
					{
						if (bSuccess || bSkip) break;
						else return;
					}
				}
			}

			if (bSuccess2)
			{
				if(MkDirDlg[MKDIR_COMBOBOX_LINKTYPE].ListPos)
				{
					string strTarget=MkDirDlg[MKDIR_EDIT_LINKPATH].strData;
					Unquote(strTarget);
					if(!CreateReparsePoint(strTarget, strDirName, MkDirDlg[MKDIR_COMBOBOX_LINKTYPE].ListPos==1?RP_JUNCTION:RP_SYMLINKDIR))
					{
						Message(FMSG_WARNING|FMSG_ERRORTYPE, 1, MSG(MError), MSG(MCopyCannotCreateLink), strDirName, MSG(MHOk));
					}
				}

				TreeList::AddTreeName(strDirName);
			}
			else if (!bSkip)
				break;
		}

		SrcPanel->Update(UPDATE_KEEP_SELECTION);

		if (!strDirName.IsEmpty())
		{
			size_t pos;

			if (FindSlash(pos,strDirName))
				strDirName.SetLength(pos);

			if (!SrcPanel->GoToFile(strDirName) && strDirName.At(strDirName.GetLength()-1)==L'.')
			{
				strDirName.SetLength(strDirName.GetLength()-1);
				SrcPanel->GoToFile(strDirName);
			}
		}

		SrcPanel->Redraw();
		Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
		int AnotherType=AnotherPanel->GetType();

		if (AnotherPanel->NeedUpdatePanel(SrcPanel) || AnotherType==QVIEW_PANEL)
		{
			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
			AnotherPanel->Redraw();
		}
	}
}
