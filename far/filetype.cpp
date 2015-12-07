/*
filetype.cpp

Работа с ассоциациями файлов
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

#include "filetype.hpp"
#include "keys.hpp"
#include "dialog.hpp"
#include "vmenu2.hpp"
#include "preservelongname.hpp"
#include "ctrlobj.hpp"
#include "cmdline.hpp"
#include "history.hpp"
#include "filemasks.hpp"
#include "message.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "execute.hpp"
#include "fnparce.hpp"
#include "strmix.hpp"
#include "configdb.hpp"
#include "pathmix.hpp"
#include "language.hpp"
#include "DlgGuid.hpp"

/* $ 25.04.2001 DJ
   обработка @ в IF EXIST: функция, которая извлекает команду из строки
   с IF EXIST с учетом @ и возвращает TRUE, если условие IF EXIST
   выполено, и FALSE в противном случае/
*/

bool ExtractIfExistCommand(string &strCommandText)
{
	bool Result=true;
	const wchar_t *wPtrCmd=PrepareOSIfExist(strCommandText);

	// Во! Условие не выполнено!!!
	// (например, пока рассматривали менюху, в это время)
	// какой-то злобный чебурашка стер файл!
	if (wPtrCmd)
	{
		if (!*wPtrCmd)
		{
			Result=false;
		}
		else
		{
			// прокинем "if exist"
			if (strCommandText.front() == L'@')
			{
				strCommandText.resize(1);
				strCommandText += wPtrCmd;
			}
			else
			{
				strCommandText = wPtrCmd;
			}
		}
	}

	return Result;
}

size_t GetDescriptionWidth()
{
	size_t Width = 0;
	DWORD Index=0;
	unsigned __int64 id;
	string strMask;
	string strDescription;
	filemasks FMask;

	while (ConfigProvider().AssocConfig()->EnumMasks(Index++,&id,strMask))
	{
		if (!FMask.Set(strMask, FMF_SILENT))
			continue;

		ConfigProvider().AssocConfig()->GetDescription(id,strDescription);
		Width = std::max(Width, HiStrlen(strDescription));
	}

	return Width;
}

/* $ 14.01.2001 SVS
   Добавим интеллектуальности.
   Если встречается "IF" и оно выполняется, то команда
   помещается в список

   Вызывается для F3, F4 - ассоциации
   Enter в ком строке - ассоциации.
*/
/* $ 06.07.2001
   + Используем filemasks вместо GetCommaWord, этим самым добиваемся того, что
     можно использовать маски исключения
   - Убрал непонятный мне запрет на использование маски файлов типа "*.*"
     (был когда-то, вроде, такой баг-репорт)
*/
bool ProcessLocalFileTypes(const string& Name, const string& ShortName, FILETYPE_MODE Mode, bool AlwaysWaitFinish)
{
	string strCommand, strDescription, strMask;
	{
		const auto TypesMenu = VMenu2::create(MSG(MSelectAssocTitle), nullptr, 0, ScrY - 4);
		TypesMenu->SetHelp(L"FileAssoc");
		TypesMenu->SetMenuFlags(VMENU_WRAPMODE);
		TypesMenu->SetId(SelectAssocMenuId);

		int ActualCmdCount=0; // отображаемых ассоциаций в меню
		filemasks FMask; // для работы с масками файлов

		int CommandCount=0;
		DWORD Index=0;
		unsigned __int64 id;
		string FileName = PointToName(Name);

		while (ConfigProvider().AssocConfig()->EnumMasksForType(Mode,Index++,&id,strMask))
		{
			strCommand.clear();

			if (FMask.Set(strMask,FMF_SILENT))
			{
				if (FMask.Compare(FileName))
				{
					ConfigProvider().AssocConfig()->GetCommand(id,Mode,strCommand);

					if (!strCommand.empty())
					{
						ConfigProvider().AssocConfig()->GetDescription(id,strDescription);
						CommandCount++;
					}
				}

				if (strCommand.empty())
					continue;
			}

			string strCommandText = strCommand;
			SubstFileName(nullptr,strCommandText,Name, ShortName,nullptr,nullptr,nullptr,nullptr,TRUE);

			// все "подставлено", теперь проверим условия "if exist"
			if (!ExtractIfExistCommand(strCommandText))
				continue;

			ActualCmdCount++;

			if (!strDescription.empty())
				SubstFileName(nullptr,strDescription, Name, ShortName, nullptr, nullptr, nullptr, nullptr, TRUE);
			else
				strDescription = strCommandText;

			MenuItemEx TypesMenuItem(strDescription);
			TypesMenuItem.SetSelect(Index==1);
			TypesMenuItem.UserData = strCommand;
			TypesMenu->AddItem(TypesMenuItem);
		}

		if (!CommandCount)
			return false;

		if (!ActualCmdCount)
			return true;

		int ExitCode=0;

		if (ActualCmdCount>1)
		{
			ExitCode=TypesMenu->Run();

			if (ExitCode<0)
				return true;
		}

		strCommand = *TypesMenu->GetUserDataPtr<string>(ExitCode);
	}

	string strListName, strAnotherListName, strShortListName, strAnotherShortListName;

	const string* ListNames[] =
	{
		&strListName,
		&strAnotherListName,
		&strShortListName,
		&strAnotherShortListName
	};

	int PreserveLFN=SubstFileName(nullptr,strCommand, Name, ShortName, &strListName, &strAnotherListName, &strShortListName, &strAnotherShortListName);
	const auto ListFileUsed = !std::all_of(ALL_CONST_RANGE(ListNames), std::mem_fn(&string::empty));

	// Снова все "подставлено", теперь проверим условия "if exist"
	if (ExtractIfExistCommand(strCommand))
	{
		SCOPED_ACTION(PreserveLongName)(ShortName, PreserveLFN);
		RemoveExternalSpaces(strCommand);

		if (!strCommand.empty())
		{
			Global->CtrlObject->CmdLine()->ExecString(strCommand,AlwaysWaitFinish, false, false, ListFileUsed, false,
				Mode == FILETYPE_VIEW || Mode == FILETYPE_ALTVIEW || Mode == FILETYPE_EDIT || Mode == FILETYPE_ALTEDIT);
			if (!(Global->Opt->ExcludeCmdHistory&EXCLUDECMDHISTORY_NOTFARASS) && !AlwaysWaitFinish) //AN
			{
				const auto curDir = Global->CtrlObject->CmdLine()->GetCurDir();
				Global->CtrlObject->CmdHistory->AddToHistory(strCommand, HR_DEFAULT, nullptr, nullptr, curDir.data());
			}
		}
	}

	std::for_each(CONST_RANGE(ListNames, i)
	{
		if (!i->empty())
			os::DeleteFile(*i);
	});

	return true;
}


bool GetFiletypeOpenMode(int keyPressed, FILETYPE_MODE& mode, bool& shouldForceInternal)
{
	bool isModeFound = false; 

	switch (keyPressed)
	{
	case KEY_F3:
	case KEY_NUMPAD5:
	case KEY_SHIFTNUMPAD5:
		mode = Global->Opt->ViOpt.UseExternalViewer? FILETYPE_ALTVIEW : FILETYPE_VIEW;
		shouldForceInternal = false;
		isModeFound = true;
		break;

	case KEY_CTRLSHIFTF3:
	case KEY_RCTRLSHIFTF3:
		mode = FILETYPE_VIEW;
		shouldForceInternal = true;
		isModeFound = true;
		break;

	case KEY_RALTF3:
	case KEY_ALTF3:
		mode = Global->Opt->ViOpt.UseExternalViewer? FILETYPE_VIEW : FILETYPE_ALTVIEW;
		shouldForceInternal = false;
		isModeFound = true;
		break;

	case KEY_F4:
		mode = Global->Opt->EdOpt.UseExternalEditor? FILETYPE_ALTEDIT: FILETYPE_EDIT;
		shouldForceInternal = false;
		isModeFound = true;
		break;

	case KEY_CTRLSHIFTF4:
	case KEY_RCTRLSHIFTF4:
		mode = FILETYPE_EDIT;
		shouldForceInternal = true;
		isModeFound = true;
		break;

	case KEY_ALTF4:
	case KEY_RALTF4:
		mode = Global->Opt->EdOpt.UseExternalEditor? FILETYPE_EDIT : FILETYPE_ALTEDIT;
		shouldForceInternal = false;
		isModeFound = true;
		break;

	case KEY_ENTER:
		mode = FILETYPE_EXEC;
		isModeFound = true;
		break;

	case KEY_CTRLPGDN:
	case KEY_RCTRLPGDN:
		mode = FILETYPE_ALTEXEC;
		isModeFound = true;
		break;

	default:
		// non-default key may be pressed, it's ok. 
		break;
	}

	return isModeFound;
}

void ProcessGlobalFileTypes(const string& Name, bool AlwaysWaitFinish, bool RunAs, bool FromPanel)
{
	string strName(Name);
	QuoteSpace(strName);
	Global->CtrlObject->CmdLine()->ExecString(strName, AlwaysWaitFinish, true, true, false, RunAs, false, FromPanel);

	if (!(Global->Opt->ExcludeCmdHistory&EXCLUDECMDHISTORY_NOTWINASS) && !AlwaysWaitFinish)
	{
		const auto curDir = Global->CtrlObject->CmdLine()->GetCurDir();
		Global->CtrlObject->CmdHistory->AddToHistory(strName, HR_DEFAULT, nullptr, nullptr, curDir.data());
	}
}

/*
  Используется для запуска внешнего редактора и вьювера
*/
void ProcessExternal(const string& Command, const string& Name, const string& ShortName, bool AlwaysWaitFinish)
{
	string strListName, strAnotherListName, strShortListName, strAnotherShortListName;

	const string* ListNames[] =
	{
		&strListName,
		&strAnotherListName,
		&strShortListName,
		&strAnotherShortListName
	};

	{
		string strExecStr = Command;
		int PreserveLFN = SubstFileName(nullptr, strExecStr, Name, ShortName, &strListName, &strAnotherListName, &strShortListName, &strAnotherShortListName);
		const auto ListFileUsed = !std::all_of(ALL_CONST_RANGE(ListNames), std::mem_fn(&string::empty));

		// Снова все "подставлено", теперь проверим условия "if exist"
		if (!ExtractIfExistCommand(strExecStr))
			return;

		SCOPED_ACTION(PreserveLongName)(ShortName,PreserveLFN);
		string strFullName;
		ConvertNameToFull(Name, strFullName);
		string strFullShortName;
		ConvertNameToShort(strFullName,strFullShortName);
		//BUGBUGBUGBUGBUGBUG !!! Same ListNames!!!
		string strFullExecStr = Command;
		SubstFileName(nullptr, strFullExecStr, strFullName, strFullShortName, &strListName, &strAnotherListName, &strShortListName, &strAnotherShortListName);

		// Снова все "подставлено", теперь проверим условия "if exist"
		if (!ExtractIfExistCommand(strFullExecStr))
			return;

		Global->CtrlObject->ViewHistory->AddToHistory(strFullExecStr, AlwaysWaitFinish? HR_EXTERNAL_WAIT : HR_EXTERNAL);

		Global->CtrlObject->CmdLine()->ExecString(strExecStr,AlwaysWaitFinish, 0, 0, ListFileUsed, false, true);
	}

	std::for_each(CONST_RANGE(ListNames, i)
	{
		if (!i->empty())
			os::DeleteFile(*i);
	});
}

static int FillFileTypesMenu(VMenu2 *TypesMenu,int MenuPos)
{
	const auto DizWidth=GetDescriptionWidth();
	TypesMenu->clear();
	DWORD Index=0;
	string strMask;
	string strTitle;
	unsigned __int64 id;

	while (ConfigProvider().AssocConfig()->EnumMasks(Index++,&id,strMask))
	{
		string strMenuText;

		if (DizWidth)
		{
			ConfigProvider().AssocConfig()->GetDescription(id,strTitle);

			size_t AddLen=strTitle.size() - HiStrlen(strTitle);

			strMenuText = FormatString() << fmt::LeftAlign() << fmt::ExactWidth(DizWidth + AddLen) << strTitle << L' ' << BoxSymbols[BS_V1] << L' ';
		}

		strMenuText += strMask;
		MenuItemEx TypesMenuItem(strMenuText);
		TypesMenuItem.SetSelect((int)(Index-1)==MenuPos);
		TypesMenuItem.UserData = id;
		TypesMenu->AddItem(TypesMenuItem);
	}

	return (int)(Index-1);
}

enum EDITTYPERECORD
{
	ETR_DOUBLEBOX,
	ETR_TEXT_MASKS,
	ETR_EDIT_MASKS,
	ETR_TEXT_DESCR,
	ETR_EDIT_DESCR,
	ETR_SEPARATOR1,
	ETR_COMBO_EXEC,
	ETR_EDIT_EXEC,
	ETR_COMBO_ALTEXEC,
	ETR_EDIT_ALTEXEC,
	ETR_COMBO_VIEW,
	ETR_EDIT_VIEW,
	ETR_COMBO_ALTVIEW,
	ETR_EDIT_ALTVIEW,
	ETR_COMBO_EDIT,
	ETR_EDIT_EDIT,
	ETR_COMBO_ALTEDIT,
	ETR_EDIT_ALTEDIT,
	ETR_SEPARATOR2,
	ETR_BUTTON_OK,
	ETR_BUTTON_CANCEL,
};

intptr_t EditTypeRecordDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	switch (Msg)
	{
		case DN_BTNCLICK:

			switch (Param1)
			{
				case ETR_COMBO_EXEC:
				case ETR_COMBO_ALTEXEC:
				case ETR_COMBO_VIEW:
				case ETR_COMBO_ALTVIEW:
				case ETR_COMBO_EDIT:
				case ETR_COMBO_ALTEDIT:
					Dlg->SendMessage(DM_ENABLE,Param1+1,ToPtr(reinterpret_cast<intptr_t>(Param2)==BSTATE_CHECKED));
					break;
				default:
					break;
			}

			break;
		case DN_CLOSE:

			if (Param1==ETR_BUTTON_OK)
			{
				return filemasks().Set(reinterpret_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, ETR_EDIT_MASKS, nullptr)));
			}
			break;

		default:
			break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

bool EditTypeRecord(unsigned __int64 EditPos,bool NewRec)
{
	const int DlgX=76,DlgY=23;
	FarDialogItem EditDlgData[]=
	{
		{DI_DOUBLEBOX,3, 1,DlgX-4,DlgY-2,0,nullptr,nullptr,0,MSG(MFileAssocTitle)},
		{DI_TEXT,     5, 2, 0, 2,0,nullptr,nullptr,0,MSG(MFileAssocMasks)},
		{DI_EDIT,     5, 3,DlgX-6, 3,0,L"Masks",nullptr,DIF_FOCUS|DIF_HISTORY,L""},
		{DI_TEXT,     5, 4, 0, 4,0,nullptr,nullptr,0,MSG(MFileAssocDescr)},
		{DI_EDIT,     5, 5,DlgX-6, 5,0,nullptr,nullptr,0,L""},
		{DI_TEXT,     -1, 6, 0, 6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX, 5, 7, 0, 7,1,nullptr,nullptr,0,MSG(MFileAssocExec)},
		{DI_EDIT,     9, 8,DlgX-6, 8,0,nullptr,nullptr,DIF_EDITPATH|DIF_EDITPATHEXEC,L""},
		{DI_CHECKBOX, 5, 9, 0, 9,1,nullptr,nullptr,0,MSG(MFileAssocAltExec)},
		{DI_EDIT,     9,10,DlgX-6,10,0,nullptr,nullptr,DIF_EDITPATH|DIF_EDITPATHEXEC,L""},
		{DI_CHECKBOX, 5,11, 0,11,1,nullptr,nullptr,0,MSG(MFileAssocView)},
		{DI_EDIT,     9,12,DlgX-6,12,0,nullptr,nullptr,DIF_EDITPATH|DIF_EDITPATHEXEC,L""},
		{DI_CHECKBOX, 5,13, 0,13,1,nullptr,nullptr,0,MSG(MFileAssocAltView)},
		{DI_EDIT,     9,14,DlgX-6,14,0,nullptr,nullptr,DIF_EDITPATH|DIF_EDITPATHEXEC,L""},
		{DI_CHECKBOX, 5,15, 0,15,1,nullptr,nullptr,0,MSG(MFileAssocEdit)},
		{DI_EDIT,     9,16,DlgX-6,16,0,nullptr,nullptr,DIF_EDITPATH|DIF_EDITPATHEXEC,L""},
		{DI_CHECKBOX, 5,17, 0,17,1,nullptr,nullptr,0,MSG(MFileAssocAltEdit)},
		{DI_EDIT,     9,18,DlgX-6,18,0,nullptr,nullptr,DIF_EDITPATH|DIF_EDITPATHEXEC,L""},
		{DI_TEXT,     -1,DlgY-4, 0,DlgY-4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,   0,DlgY-3, 0,DlgY-3,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,   0,DlgY-3, 0,DlgY-3,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
	};
	auto EditDlg = MakeDialogItemsEx(EditDlgData);

	if (!NewRec)
	{
		ConfigProvider().AssocConfig()->GetMask(EditPos,EditDlg[ETR_EDIT_MASKS].strData);
		ConfigProvider().AssocConfig()->GetDescription(EditPos,EditDlg[ETR_EDIT_DESCR].strData);
		for (int i=FILETYPE_EXEC,Item=ETR_EDIT_EXEC; i<=FILETYPE_ALTEDIT; i++,Item+=2)
		{
			bool on=false;
			if (!ConfigProvider().AssocConfig()->GetCommand(EditPos,i,EditDlg[Item].strData,&on) || !on)
			{
				EditDlg[Item-1].Selected = BSTATE_UNCHECKED;
				EditDlg[Item].Flags |= DIF_DISABLE;
			}
			else if (on)
			{
				EditDlg[Item-1].Selected = BSTATE_CHECKED;
			}
		}
	}

	const auto Dlg = Dialog::create(EditDlg, EditTypeRecordDlgProc);
	Dlg->SetHelp(L"FileAssocModify");
	Dlg->SetId(FileAssocModifyId);
	Dlg->SetPosition(-1,-1,DlgX,DlgY);
	Dlg->Process();

	if (Dlg->GetExitCode()==ETR_BUTTON_OK)
	{
		if (NewRec)
		{
			EditPos = ConfigProvider().AssocConfig()->AddType(EditPos,EditDlg[ETR_EDIT_MASKS].strData,EditDlg[ETR_EDIT_DESCR].strData);
		}
		else
		{
			ConfigProvider().AssocConfig()->UpdateType(EditPos,EditDlg[ETR_EDIT_MASKS].strData,EditDlg[ETR_EDIT_DESCR].strData);
		}

		for (int i=FILETYPE_EXEC,Item=ETR_EDIT_EXEC; i<=FILETYPE_ALTEDIT; i++,Item+=2)
		{
			ConfigProvider().AssocConfig()->SetCommand(EditPos,i,EditDlg[Item].strData,EditDlg[Item-1].Selected==BSTATE_CHECKED);
		}

		return true;
	}

	return false;
}

bool DeleteTypeRecord(unsigned __int64 DeletePos)
{
	string strMask;
	ConfigProvider().AssocConfig()->GetMask(DeletePos,strMask);
	InsertQuote(strMask);

	if (!Message(MSG_WARNING,2,MSG(MAssocTitle),MSG(MAskDelAssoc),strMask.data(),MSG(MDelete),MSG(MCancel)))
	{
		ConfigProvider().AssocConfig()->DelType(DeletePos);
		return true;
	}

	return false;
}

void EditFileTypes()
{
	SCOPED_ACTION(auto)(ConfigProvider().AssocConfig()->ScopedTransaction());

	int MenuPos=0;
	const auto TypesMenu = VMenu2::create(MSG(MAssocTitle), nullptr, 0, ScrY - 4);
	TypesMenu->SetHelp(L"FileAssoc");
	TypesMenu->SetMenuFlags(VMENU_WRAPMODE);
	TypesMenu->SetBottomTitle(MSG(MAssocBottom));
	TypesMenu->SetId(FileAssocMenuId);

	for (;;)
	{
		int NumLine = FillFileTypesMenu(TypesMenu.get(), MenuPos);
		int ExitCode=TypesMenu->Run([&](const Manager::Key& RawKey)->int
		{
			const auto Key=RawKey.FarKey();
			MenuPos=TypesMenu->GetSelectPos();

			int KeyProcessed = 1;

			switch (Key)
			{
				case KEY_NUMDEL:
				case KEY_DEL:

					if (MenuPos<NumLine)
					{
						if (const auto IdPtr = TypesMenu->GetUserDataPtr<unsigned __int64>(MenuPos))
						{
							DeleteTypeRecord(*IdPtr);
						}
						NumLine = FillFileTypesMenu(TypesMenu.get(), MenuPos);
					}

					break;
				case KEY_NUMPAD0:
				case KEY_INS:
					if (MenuPos - 1 >= 0)
					{
						if (const auto IdPtr = TypesMenu->GetUserDataPtr<unsigned __int64>(MenuPos - 1))
						{
							EditTypeRecord(*IdPtr, true);
						}
					}
					else
					{
						EditTypeRecord(0, true);
					}
					NumLine = FillFileTypesMenu(TypesMenu.get(), MenuPos);
					break;
				case KEY_NUMENTER:
				case KEY_ENTER:
				case KEY_F4:

					if (MenuPos<NumLine)
					{
						if (const auto IdPtr = TypesMenu->GetUserDataPtr<unsigned __int64>(MenuPos))
						{
							EditTypeRecord(*IdPtr, false);
						}
						NumLine = FillFileTypesMenu(TypesMenu.get(), MenuPos);
					}
					return 1;

				case KEY_CTRLUP:
				case KEY_RCTRLUP:
				case KEY_CTRLDOWN:
				case KEY_RCTRLDOWN:
				{
					if (!((Key==KEY_CTRLUP || Key==KEY_RCTRLUP) && !MenuPos) &&
						!((Key == KEY_CTRLDOWN || Key == KEY_RCTRLDOWN) && MenuPos == static_cast<int>(TypesMenu->size() - 1)))
					{
						int NewMenuPos=MenuPos+((Key==KEY_CTRLUP || Key==KEY_RCTRLUP)?-1:+1);
						if (const auto IdPtr = TypesMenu->GetUserDataPtr<unsigned __int64>(MenuPos))
						{
							if (const auto IdPtr2 = TypesMenu->GetUserDataPtr<unsigned __int64>(NewMenuPos))
							{
								if (ConfigProvider().AssocConfig()->SwapPositions(*IdPtr, *IdPtr2))
								{
									MenuPos = NewMenuPos;
								}
							}
						}
						NumLine = FillFileTypesMenu(TypesMenu.get(), MenuPos);
					}
				}
				break;

				default:
					KeyProcessed = 0;
			}
			return KeyProcessed;
		});

		if (ExitCode!=-1)
		{
			MenuPos=ExitCode;
			TypesMenu->Key(KEY_F4);
			continue;
		}

		break;
	}
}
