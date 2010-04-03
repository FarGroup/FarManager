/*
mkdir.cpp

Создание каталога
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

#include "mkdir.hpp"
#include "lang.hpp"
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

enum
{
	MKDIR_BORDER,
	MKDIR_TEXT,
	MKDIR_EDIT,
	MKDIR_SEPARATOR0,
	MKDIR_CHECKBOX,
	MKDIR_SEPARATOR2,
	MKDIR_OK,
	MKDIR_CANCEL,
};

LONG_PTR WINAPI MkDirDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
	switch (Msg)
	{
		case DN_CLOSE:
		{
			if (Param1==MKDIR_OK)
			{
				string strDirName=reinterpret_cast<LPCWSTR>(SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,MKDIR_EDIT,0));
				Opt.MultiMakeDir=(SendDlgMessage(hDlg,DM_GETCHECK,MKDIR_CHECKBOX,0)==BSTATE_CHECKED);

				// это по поводу создания одиночного каталога, который
				// начинается с пробела! Чтобы ручками не заключать
				// такой каталог в кавычки
				if (Opt.MultiMakeDir && wcspbrk(strDirName,L";,\"") == nullptr)
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
					Message(MSG_DOWN|MSG_WARNING,1,MSG(MWarning),MSG(MIncorrectDirList),MSG(MOk));
					return FALSE;
				}
			}
		}
		break;
	}

	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

void ShellMakeDir(Panel *SrcPanel)
{
	string strDirName;
	string strOriginalDirName;
	wchar_t *lpwszDirName;
	UserDefinedList DirList(0,0,ULF_UNIQUE);
	DialogDataEx MkDirDlgData[]=
	{
		DI_DOUBLEBOX,3,1,72,8,0,0,0,0,MSG(MMakeFolderTitle),
		DI_TEXT,     5,2, 0,2,0,0,0,0,MSG(MCreateFolder),
		DI_EDIT,     5,3,70,3,1,(DWORD_PTR)L"NewFolder",DIF_EDITEXPAND|DIF_HISTORY|DIF_USELASTHISTORY|DIF_EDITPATH,0,L"",
		DI_TEXT,     0,4, 0,4,0,0,DIF_SEPARATOR,0,L"",
		DI_CHECKBOX, 5,5, 0,5,0,Opt.MultiMakeDir, 0,0,MSG(MMultiMakeDir),
		DI_TEXT,     0,6, 0,6,0,0,DIF_SEPARATOR,0,L"",
		DI_BUTTON,   0,7, 0,7,0,0,DIF_CENTERGROUP,1,MSG(MOk),
		DI_BUTTON,   0,7, 0,7,0,0,DIF_CENTERGROUP,0,MSG(MCancel),
	};
	MakeDialogItemsEx(MkDirDlgData,MkDirDlg);
	Dialog Dlg(MkDirDlg,countof(MkDirDlg),MkDirDlgProc,reinterpret_cast<LONG_PTR>(&DirList));
	Dlg.SetPosition(-1,-1,76,10);
	Dlg.SetHelp(L"MakeFolder");
	Dlg.Process();

	if (Dlg.GetExitCode()==MKDIR_OK)
	{
		strDirName=MkDirDlg[MKDIR_EDIT].strData;
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
			lpwszDirName = strDirName.GetBuffer();
			bool bSuccess = false;

			for (wchar_t *ChPtr=lpwszDirName; *ChPtr!=0; ChPtr++)
			{
				if (IsSlash(*ChPtr))
				{
					*ChPtr=0;

					if (*lpwszDirName && apiCreateDirectory(lpwszDirName,nullptr))
					{
						TreeList::AddTreeName(lpwszDirName);
						bSuccess = true;
					}

					*ChPtr=L'\\';
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
						ret=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotCreateFolder),strOriginalDirName,MSG(MCancel));
					else
						ret=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),MSG(MCannotCreateFolder),strOriginalDirName,MSG(MOk),MSG(MSkip));

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
						ret=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),MSG(MCannotCreateFolder),strOriginalDirName,MSG(MRetry),MSG(MCancel));
					}
					else
					{
						ret=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,MSG(MError),MSG(MCannotCreateFolder),strOriginalDirName,MSG(MRetry),MSG(MSkip),MSG(MCancel));
						bSkip = ret==1;
					}

					if (ret!=0)
					{
						if (bSuccess || bSkip) break;
						else return;
					}
				}
			}

			if (bSuccess2)
				TreeList::AddTreeName(strDirName);
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
