/*
fileedit.cpp

Редактирование файла - надстройка над editor.cpp
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

#include "fileedit.hpp"
#include "keyboard.hpp"
#include "codepage.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "poscache.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "dialog.hpp"
#include "FarDlgBuilder.hpp"
#include "fileview.hpp"
#include "help.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "namelist.hpp"
#include "history.hpp"
#include "cmdline.hpp"
#include "scrbuf.hpp"
#include "savescr.hpp"
#include "chgprior.hpp"
#include "filestr.hpp"
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "TaskBar.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "config.hpp"
#include "delete.hpp"
#include "datetime.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "strmix.hpp"
#include "exitcode.hpp"
#include "cache.hpp"
#include "constitle.hpp"
#include "wakeful.hpp"
#include "DlgGuid.hpp"
#include "stddlg.hpp"
#include "plugins.hpp"
#include "language.hpp"
#include "keybar.hpp"

enum enumOpenEditor
{
	ID_OE_TITLE,
	ID_OE_OPENFILETITLE,
	ID_OE_FILENAME,
	ID_OE_SEPARATOR1,
	ID_OE_CODEPAGETITLE,
	ID_OE_CODEPAGE,
	ID_OE_SEPARATOR2,
	ID_OE_OK,
	ID_OE_CANCEL,
};


intptr_t hndOpenEditor(Dialog* Dlg, intptr_t msg, intptr_t param1, void* param2)
{
	if (msg == DN_INITDIALOG)
	{
		uintptr_t codepage = *(uintptr_t*)param2;
		Codepages().FillCodePagesList(Dlg, ID_OE_CODEPAGE, codepage, true, false, true, false);
	}

	if (msg == DN_CLOSE)
	{
		if (param1 == ID_OE_OK)
		{
			uintptr_t* param = (uintptr_t*)Dlg->SendMessage(DM_GETDLGDATA, 0, 0);
			FarListPos pos={sizeof(FarListPos)};
			Dlg->SendMessage(DM_LISTGETCURPOS, ID_OE_CODEPAGE, &pos);
			*param = *(uintptr_t*)Dlg->SendMessage(DM_LISTGETDATA, ID_OE_CODEPAGE, ToPtr(pos.SelectPos));
			return TRUE;
		}
	}

	return Dlg->DefProc(msg, param1, param2);
}

bool dlgOpenEditor(string &strFileName, uintptr_t &codepage)
{
	FarDialogItem EditDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,72,8,0,nullptr,nullptr,0,MSG(MEditTitle)},
		{DI_TEXT,     5,2, 0,2,0,nullptr,nullptr,0,MSG(MEditOpenCreateLabel)},
		{DI_EDIT,     5,3,70,3,0,L"NewEdit",nullptr,DIF_FOCUS|DIF_HISTORY|DIF_USELASTHISTORY|DIF_EDITEXPAND|DIF_EDITPATH,L""},
		{DI_TEXT,    -1,4, 0,4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_TEXT,     5,5, 0,5,0,nullptr,nullptr,0,MSG(MEditCodePage)},
		{DI_COMBOBOX,25,5,70,5,0,nullptr,nullptr,DIF_DROPDOWNLIST|DIF_LISTWRAPMODE|DIF_LISTAUTOHIGHLIGHT,L""},
		{DI_TEXT,    -1,6, 0,6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,   0,7, 0,7,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,   0,7, 0,7,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
	};
	auto EditDlg = MakeDialogItemsEx(EditDlgData);
	Dialog Dlg(EditDlg, hndOpenEditor, &codepage);
	Dlg.SetPosition(-1,-1,76,10);
	Dlg.SetHelp(L"FileOpenCreate");
	Dlg.SetId(FileOpenCreateId);
	Dlg.Process();

	if (Dlg.GetExitCode() == ID_OE_OK)
	{
		strFileName = EditDlg[ID_OE_FILENAME].strData;
		return true;
	}

	return false;
}

bool dlgBadEditorCodepage(uintptr_t &codepage)
{
	intptr_t id = 0, id_cp, id_ok;

	DialogBuilder Builder(MWarning, nullptr, [&](Dialog* dlg, intptr_t msg,intptr_t p1,void* p2) -> intptr_t
	{
		if (msg == DN_INITDIALOG)
		{
			Codepages().FillCodePagesList(dlg, id_cp, codepage, true, false, true, false);
		}
		else if (msg == DN_CLOSE && p1 == id_ok)
		{
			FarListPos pos={sizeof(FarListPos)};
			dlg->SendMessage(DM_LISTGETCURPOS, id_cp, &pos);
			codepage = *(uintptr_t*)dlg->SendMessage(DM_LISTGETDATA, id_cp, ToPtr(pos.SelectPos));
			return TRUE;
		}
		return dlg->DefProc(msg, p1, p2);
	});

	++id; Builder.AddText(MEditorLoadCPWarn1)->Flags = DIF_CENTERTEXT;
	++id; Builder.AddText(MEditorLoadCPWarn2)->Flags = DIF_CENTERTEXT;
	++id; Builder.AddText(MEditorSaveNotRecommended)->Flags = DIF_CENTERTEXT;
	++id; Builder.AddSeparator();

	IntOption cp_val;
	std::vector<DialogBuilderListItem2> items;
	id_cp = ++id; Builder.AddComboBox(cp_val, 46, items, DIF_LISTWRAPMODE);
	id_ok = id+2; Builder.AddOKCancel();

   Builder.SetDialogMode(DMODE_WARNINGSTYLE);
	Builder.SetId(BadEditorCodePageId);
	return Builder.ShowDialog();
}

enum enumSaveFileAs
{
	ID_SF_TITLE,
	ID_SF_SAVEASFILETITLE,
	ID_SF_FILENAME,
	ID_SF_SEPARATOR1,
	ID_SF_CODEPAGETITLE,
	ID_SF_CODEPAGE,
	ID_SF_SIGNATURE,
	ID_SF_SEPARATOR2,
	ID_SF_SAVEASFORMATTITLE,
	ID_SF_DONOTCHANGE,
	ID_SF_DOS,
	ID_SF_UNIX,
	ID_SF_MAC,
	ID_SF_SEPARATOR3,
	ID_SF_OK,
	ID_SF_CANCEL,
};

intptr_t hndSaveFileAs(Dialog* Dlg, intptr_t msg, intptr_t param1, void* param2)
{
	static uintptr_t CurrentCodepage = 0;

	switch (msg)
	{
		case DN_INITDIALOG:
		{
			CurrentCodepage = *(uintptr_t *) Dlg->SendMessage(DM_GETDLGDATA, 0, 0);
			Codepages().FillCodePagesList(Dlg, ID_SF_CODEPAGE, CurrentCodepage, false, false, false, false);
			break;
		}
		case DN_CLOSE:
		{
			if (param1 == ID_SF_OK)
			{
				uintptr_t *CodepagePtr = (uintptr_t *)Dlg->SendMessage(DM_GETDLGDATA, 0, 0);
				FarListPos pos={sizeof(FarListPos)};
				Dlg->SendMessage(DM_LISTGETCURPOS, ID_SF_CODEPAGE, &pos);
				*CodepagePtr = *(UINT*)Dlg->SendMessage(DM_LISTGETDATA, ID_SF_CODEPAGE, ToPtr(pos.SelectPos));
				return TRUE;
			}

			break;
		}
		case DN_EDITCHANGE:
		{
			if (param1==ID_SF_CODEPAGE)
			{
				FarListPos pos={sizeof(FarListPos)};
				Dlg->SendMessage(DM_LISTGETCURPOS,ID_SF_CODEPAGE,&pos);
				auto cp = *reinterpret_cast<UINT*>(Dlg->SendMessage(DM_LISTGETDATA,ID_SF_CODEPAGE,ToPtr(pos.SelectPos)));

				if (cp != CurrentCodepage)
				{
					if (IsUnicodeOrUtfCodePage(cp))
					{
						if (!IsUnicodeOrUtfCodePage(CurrentCodepage))
							Dlg->SendMessage(DM_SETCHECK,ID_SF_SIGNATURE,ToPtr(Global->Opt->EdOpt.AddUnicodeBOM));
						Dlg->SendMessage(DM_ENABLE,ID_SF_SIGNATURE,ToPtr(TRUE));
					}
					else
					{
						Dlg->SendMessage(DM_SETCHECK,ID_SF_SIGNATURE,ToPtr(BSTATE_UNCHECKED));
						Dlg->SendMessage(DM_ENABLE, ID_SF_SIGNATURE, ToPtr(FALSE));
					}

					CurrentCodepage = cp;
					return TRUE;
				}
			}

			break;
		}
	default:
		break;
	}

	return Dlg->DefProc(msg, param1, param2);
}



bool dlgSaveFileAs(string &strFileName, int &TextFormat, uintptr_t &codepage,bool &AddSignature)
{
   bool ucp = IsUnicodeOrUtfCodePage(codepage);

	FarDialogItem EditDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,72,15,0,nullptr,nullptr,0,MSG(MEditTitle)},
		{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,MSG(MEditSaveAs)},
		{DI_EDIT,5,3,70,3,0,L"NewEdit",nullptr,DIF_FOCUS|DIF_HISTORY|DIF_EDITEXPAND|DIF_EDITPATH,L""},
		{DI_TEXT,-1,4,0,4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_TEXT,5,5,0,5,0,nullptr,nullptr,0,MSG(MEditCodePage)},
		{DI_COMBOBOX,25,5,70,5,0,nullptr,nullptr,DIF_DROPDOWNLIST|DIF_LISTWRAPMODE|DIF_LISTAUTOHIGHLIGHT,L""},
		{DI_CHECKBOX,5,6,0,6,AddSignature,nullptr,nullptr,ucp ? 0 : DIF_DISABLE,MSG(MEditAddSignature)},
		{DI_TEXT,-1,7,0,7,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_TEXT,5,8,0,8,0,nullptr,nullptr,0,MSG(MEditSaveAsFormatTitle)},
		{DI_RADIOBUTTON,5,9,0,9,0,nullptr,nullptr,DIF_GROUP,MSG(MEditSaveOriginal)},
		{DI_RADIOBUTTON,5,10,0,10,0,nullptr,nullptr,0,MSG(MEditSaveDOS)},
		{DI_RADIOBUTTON,5,11,0,11,0,nullptr,nullptr,0,MSG(MEditSaveUnix)},
		{DI_RADIOBUTTON,5,12,0,12,0,nullptr,nullptr,0,MSG(MEditSaveMac)},
		{DI_TEXT,-1,13,0,13,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,0,14,0,14,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MEditorSave)},
		{DI_BUTTON,0,14,0,14,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
	};
	auto EditDlg = MakeDialogItemsEx(EditDlgData);
	EditDlg[ID_SF_FILENAME].strData = (/*Flags.Check(FFILEEDIT_SAVETOSAVEAS)?strFullFileName:strFileName*/strFileName);
	{
		size_t pos = EditDlg[ID_SF_FILENAME].strData.find(MSG(MNewFileName));
		if (pos != string::npos)
			EditDlg[ID_SF_FILENAME].strData.resize(pos);
	}
	EditDlg[ID_SF_DONOTCHANGE+TextFormat].Selected = TRUE;
	Dialog Dlg(EditDlg, hndSaveFileAs, &codepage);
	Dlg.SetPosition(-1,-1,76,17);
	Dlg.SetHelp(L"FileSaveAs");
	Dlg.SetId(FileSaveAsId);
	Dlg.Process();

	if ((Dlg.GetExitCode() == ID_SF_OK) && !EditDlg[ID_SF_FILENAME].strData.empty())
	{
		strFileName = EditDlg[ID_SF_FILENAME].strData;
		AddSignature=EditDlg[ID_SF_SIGNATURE].Selected!=0;

		if (EditDlg[ID_SF_DONOTCHANGE].Selected)
			TextFormat=0;
		else if (EditDlg[ID_SF_DOS].Selected)
			TextFormat=1;
		else if (EditDlg[ID_SF_UNIX].Selected)
			TextFormat=2;
		else if (EditDlg[ID_SF_MAC].Selected)
			TextFormat=3;

		return true;
	}

	return false;
}


FileEditor::FileEditor(const string& Name, uintptr_t codepage, DWORD InitFlags, int StartLine, int StartChar, const string* PluginData, EDITOR_FLAGS OpenModeExstFile):
	BadConversion(false)
{
	ScreenObjectWithShadow::SetPosition(0,0,ScrX,ScrY);
	m_Flags.Set(InitFlags);
	m_Flags.Set(FFILEEDIT_FULLSCREEN);
	Init(Name,codepage, nullptr, InitFlags, StartLine, StartChar, PluginData, FALSE, nullptr, OpenModeExstFile);
}


FileEditor::FileEditor(const string& Name, uintptr_t codepage, DWORD InitFlags, int StartLine, int StartChar, const string* Title, int X1, int Y1, int X2, int Y2, int DeleteOnClose, window* Update, EDITOR_FLAGS OpenModeExstFile):
	BadConversion(false)
{
	m_Flags.Set(InitFlags);

	if (X1 < 0)
		X1=0;

	if (X2 < 0 || X2 > ScrX)
		X2=ScrX;

	if (Y1 < 0)
		Y1=0;

	if (Y2 < 0 || Y2 > ScrY)
		Y2=ScrY;

	if (X1 >= X2)
	{
		X1=0;
		X2=ScrX;
	}

	if (Y1 >= Y2)
	{
		Y1=0;
		Y2=ScrY;
	}

	ScreenObjectWithShadow::SetPosition(X1,Y1,X2,Y2);
	m_Flags.Change(FFILEEDIT_FULLSCREEN,(!X1 && !Y1 && X2==ScrX && Y2==ScrY));
	string EmptyTitle;
	Init(Name,codepage, Title, InitFlags, StartLine, StartChar, &EmptyTitle, DeleteOnClose, Update, OpenModeExstFile);
}

/* $ 07.05.2001 DJ
   в деструкторе грохаем EditNamesList, если он был создан, а в SetNamesList()
   создаем EditNamesList и копируем туда значения
*/
/*
  Вызов деструкторов идет так:
    FileEditor::~FileEditor()
    Editor::~Editor()
    ...
*/
FileEditor::~FileEditor()
{
	if (!m_Flags.Check(FFILEEDIT_DISABLESAVEPOS) && (m_editor->EdOpt.SavePos || m_editor->EdOpt.SaveShortPos) && Global->CtrlObject)
		SaveToCache();

	if (!m_Flags.Check(FFILEEDIT_OPENFAILED))
	{
		/* $ 11.10.2001 IS
		   Удалим файл вместе с каталогом, если это просится и файла с таким же
		   именем не открыто в других окнах.
		*/
		/* $ 14.06.2001 IS
		   Если установлен FEDITOR_DELETEONLYFILEONCLOSE и сброшен
		   FEDITOR_DELETEONCLOSE, то удаляем только файл.
		*/
		if (m_Flags.Check(FFILEEDIT_DELETEONCLOSE|FFILEEDIT_DELETEONLYFILEONCLOSE) &&
		        !Global->WindowManager->CountWindowsWithName(strFullFileName))
		{
			if (m_Flags.Check(FFILEEDIT_DELETEONCLOSE))
				DeleteFileWithFolder(strFullFileName);
			else
			{
				api::SetFileAttributes(strFullFileName,FILE_ATTRIBUTE_NORMAL);
				api::DeleteFile(strFullFileName); //BUGBUG
			}
		}
	}
}

void FileEditor::Init(
    const string& Name,
    uintptr_t codepage,
    const string* Title,
    DWORD InitFlags,
    int StartLine,
    int StartChar,
    const string* PluginData,
    int DeleteOnClose,
    window* Update,
    EDITOR_FLAGS OpenModeExstFile
)
{
	m_windowKeyBar = std::make_unique<KeyBar>();

	class SmartLock: ::NonCopyable
	{
	public:
		SmartLock(Editor *e): m_editor(e) { m_editor->Lock(); }
		~SmartLock() { m_editor->Unlock(); }

	private:
		Editor *m_editor;
	};

	SysErrorCode=0;
	int BlankFileName = Name == MSG(MNewFileName) || Name.empty();
	//AY: флаг оповещающий закрытие редактора.
	m_bClosing = false;
	bEE_READ_Sent = false;
	bLoaded = false;
	m_bAddSignature = false;
	m_editor = std::make_unique<Editor>();

	SCOPED_ACTION(SmartLock)(m_editor.get());

	m_codepage = codepage;
	m_editor->SetOwner(this);
	m_editor->SetCodePage(m_codepage);
	*AttrStr=0;
	m_FileAttributes=INVALID_FILE_ATTRIBUTES;
	FileAttributesModified=false;
	SetTitle(Title);
	m_KeyBarVisible = Global->Opt->EdOpt.ShowKeyBar;
	m_TitleBarVisible = Global->Opt->EdOpt.ShowTitleBar;
	// $ 17.08.2001 KM - Добавлено для поиска по AltF7. При редактировании найденного файла из архива для клавиши F2 сделать вызов ShiftF2.
	m_Flags.Change(FFILEEDIT_SAVETOSAVEAS, BlankFileName != 0);

	if (BlankFileName && !m_Flags.Check(FFILEEDIT_CANNEWFILE))
	{
		SetExitCode(XC_OPEN_ERROR);
		return;
	}

	SetPluginData(PluginData);
	m_editor->SetHostFileEditor(this);
	SetCanLoseFocus(m_Flags.Check(FFILEEDIT_ENABLEF6));
	api::GetCurrentDirectory(strStartDir);

	if (!SetFileName(Name))
	{
		SetExitCode(XC_OPEN_ERROR);
		return;
	}

	{
		auto EditorWindow = Global->WindowManager->FindWindowByFile(windowtype_editor, strFullFileName);

		if (EditorWindow)
		{
			int SwitchTo=FALSE;

			if (!EditorWindow->GetCanLoseFocus(TRUE) || Global->Opt->Confirm.AllowReedit)
			{
				int MsgCode=0;
				if (OpenModeExstFile == EF_OPENMODE_QUERY)
				{
					if (m_Flags.Check(FFILEEDIT_ENABLEF6))
					{
						const wchar_t* const Items[] = {strFullFileName.data(), MSG(MAskReload), MSG(MCurrent), MSG(MNewOpen), MSG(MReload)};
						MsgCode=Message(0, 3, MSG(MEditTitle),Items, ARRAYSIZE(Items), L"EditorReload", nullptr, &EditorReloadId);
					}
					else
					{
						const wchar_t* const Items[] = {strFullFileName.data(), MSG(MAskReload), MSG(MNewOpen), MSG(MCancel)};
						MsgCode=Message(0, 2, MSG(MEditTitle),Items, ARRAYSIZE(Items), L"EditorReload", nullptr, &EditorReloadModalId);
						if (MsgCode == 0)
							MsgCode=1;
						else
							MsgCode=-200;
					}
				}
				else
				{
					if (m_Flags.Check(FFILEEDIT_ENABLEF6))
						MsgCode=(OpenModeExstFile==EF_OPENMODE_USEEXISTING)?0:
					        (OpenModeExstFile==EF_OPENMODE_NEWIFOPEN?1:
					         (OpenModeExstFile==EF_OPENMODE_RELOADIFOPEN?2:-100)
					        );
					else
						MsgCode=(OpenModeExstFile==EF_OPENMODE_NEWIFOPEN?1:-100);
				}

				switch (MsgCode)
				{
					case 0:         // Current
						SwitchTo=TRUE;
						Global->WindowManager->DeleteWindow(this); //???
						SetExitCode(XC_EXISTS); // ???
						break;
					case 1:         // NewOpen
						SwitchTo=FALSE;
						SetExitCode(XC_OPEN_NEWINSTANCE); // ???
						break;
					case 2:         // Reload
					{
						//файл могли уже закрыть. например макросом в диалоговой процедуре предыдущего Message.
						EditorWindow = Global->WindowManager->FindWindowByFile(windowtype_editor, strFullFileName);
						if (EditorWindow)
						{
							EditorWindow->SetFlags(FFILEEDIT_DISABLESAVEPOS);
							Global->WindowManager->DeleteWindow(EditorWindow);
						}
						SetExitCode(XC_RELOAD); // -2 ???
						break;
					}
					case -200:
						SetExitCode(XC_LOADING_INTERRUPTED);
						return;
					case -100:
						//WindowManager->DeleteWindow(this);  //???
						SetExitCode(XC_EXISTS);
						return;
					default:
						Global->WindowManager->DeleteWindow(this);  //???
						SetExitCode(XC_QUIT);
						return;
				}
			}
			else
			{
				SwitchTo=TRUE;
				SetExitCode((OpenModeExstFile != EF_OPENMODE_QUERY) ? XC_EXISTS : XC_MODIFIED); // TRUE???
			}

			if (SwitchTo)
			{
				//файл могли уже закрыть. например макросом в диалоговой процедуре предыдущего Message.
				EditorWindow = Global->WindowManager->FindWindowByFile(windowtype_editor, strFullFileName);
				if (EditorWindow)
				{
					Global->WindowManager->ActivateWindow(EditorWindow);
				}
				return ;
			}
		}
	}

	/* $ 29.11.2000 SVS
	   Если файл имеет атрибут ReadOnly или System или Hidden,
	   И параметр на запрос выставлен, то сначала спросим.
	*/
	/* $ 03.12.2000 SVS
	   System или Hidden - задаются отдельно
	*/
	/* $ 15.12.2000 SVS
	  - Shift-F4, новый файл. Выдает сообщение :-(
	*/
	DWORD FAttr=api::GetFileAttributes(Name);

	/* $ 05.06.2001 IS
	   + посылаем подальше всех, кто пытается отредактировать каталог
	*/
	if (FAttr!=INVALID_FILE_ATTRIBUTES && FAttr&FILE_ATTRIBUTE_DIRECTORY)
	{
		const wchar_t* const Items[] = {MSG(MEditCanNotEditDirectory),MSG(MOk)};
		Message(MSG_WARNING,1,MSG(MEditTitle),Items, ARRAYSIZE(Items), nullptr, nullptr, &EditorCanNotEditDirectoryId);
		SetExitCode(XC_OPEN_ERROR);
		return;
	}

	if ((m_editor->EdOpt.ReadOnlyLock&2) &&
	        FAttr != INVALID_FILE_ATTRIBUTES &&
	        (FAttr &
	         (FILE_ATTRIBUTE_READONLY|
	          /* Hidden=0x2 System=0x4 - располагаются во 2-м полубайте,
	             поэтому применяем маску 0110.0000 и
	             сдвигаем на свое место => 0000.0110 и получаем
	             те самые нужные атрибуты  */
	          ((m_editor->EdOpt.ReadOnlyLock&0x60)>>4)
	         )
	        )
	   )
	{
		const wchar_t* const Items[] = {Name.data(),MSG(MEditRSH),MSG(MEditROOpen),MSG(MYes),MSG(MNo)};
		if (Message(MSG_WARNING,2,MSG(MEditTitle),Items, ARRAYSIZE(Items), nullptr, nullptr, &EditorOpenRSHId))
		{
			SetExitCode(XC_OPEN_ERROR);
			return;
		}
	}

	m_editor->SetPosition(m_X1,m_Y1+(Global->Opt->EdOpt.ShowTitleBar?1:0),m_X2,m_Y2-(Global->Opt->EdOpt.ShowKeyBar?1:0));
	m_editor->SetStartPos(StartLine,StartChar);
	SetDeleteOnClose(DeleteOnClose);
	int UserBreak;

	/* $ 06.07.2001 IS
	   При создании файла с нуля так же посылаем плагинам событие EE_READ, дабы
	   не нарушать однообразие.
	*/
	if (FAttr == INVALID_FILE_ATTRIBUTES)
		m_Flags.Set(FFILEEDIT_NEW);

	if (BlankFileName && m_Flags.Check(FFILEEDIT_CANNEWFILE))
		m_Flags.Set(FFILEEDIT_NEW);

	if (m_Flags.Check(FFILEEDIT_NEW))
	  m_bAddSignature = Global->Opt->EdOpt.AddUnicodeBOM;

	if (m_Flags.Check(FFILEEDIT_LOCKED))
		m_editor->m_Flags.Set(Editor::FEDITOR_LOCKMODE);

	while (!LoadFile(strFullFileName,UserBreak))
	{
		if (BlankFileName)
		{
			m_Flags.Clear(FFILEEDIT_OPENFAILED); //AY: ну так как редактор мы открываем то видимо надо и сбросить ошибку открытия
			UserBreak=0;
		}

		if (!m_Flags.Check(FFILEEDIT_NEW) || UserBreak)
		{
			if (UserBreak!=1)
			{
				SetLastError(SysErrorCode);
				Global->CatchError();
				if(!OperationFailed(strFullFileName, MEditTitle, MSG(MEditCannotOpen), false))
					continue;
				else
					SetExitCode(XC_OPEN_ERROR);
			}
			else
			{
				SetExitCode(XC_LOADING_INTERRUPTED);
			}

			// Ахтунг. Ниже комментарии оставлены в назидании потомкам (до тех пор, пока не измениться манагер)
			//WindowManager->DeleteWindow(this); // BugZ#546 - Editor валит фар!
			//Global->CtrlObject->Cp()->Redraw(); //AY: вроде как не надо, делает проблемы с проресовкой если в редакторе из истории попытаться выбрать несуществующий файл

			// если прервали загрузку, то фремы нужно проапдейтить, чтобы предыдущие месаги не оставались на экране
			if (!Global->Opt->Confirm.Esc && UserBreak && GetExitCode() == XC_LOADING_INTERRUPTED)
				Global->WindowManager->RefreshWindow();

			return;
		}

		if (m_codepage==CP_DEFAULT || m_codepage == CP_REDETECT)
			m_codepage = GetDefaultCodePage();

		m_editor->SetCodePage(m_codepage);
		m_Flags.Set(FFILEEDIT_CODEPAGECHANGEDBYUSER);
		break;
	}

	if (GetExitCode() == XC_LOADING_INTERRUPTED || GetExitCode() == XC_OPEN_ERROR)
		return;

	ShowConsoleTitle();
	InitKeyBar();
	m_windowKeyBar->SetOwner(this);
	m_windowKeyBar->SetPosition(m_X1, m_Y2, m_X2, m_Y2);

	if (!Global->Opt->EdOpt.ShowKeyBar)
		m_windowKeyBar->Hide();

	SetMacroMode(MACROAREA_EDITOR);

	F4KeyOnly=true;
	bLoaded = true;

	if (m_Flags.Check(FFILEEDIT_ENABLEF6))
	{
		if (Update) Global->WindowManager->ReplaceWindow(Update,this);
		else Global->WindowManager->InsertWindow(this);
	}
	else
	{
		if (Update) Global->WindowManager->DeleteWindow(Update);
		Global->WindowManager->ExecuteWindow(this);
	}
	Global->WindowManager->CallbackWindow([this](){this->ReadEvent();});
}

void FileEditor::ReadEvent(void)
{
	SetBlock();
	Global->CtrlObject->Plugins->ProcessEditorEvent(EE_READ,nullptr,m_editor->EditorID);
	RemoveBlock();
	bEE_READ_Sent = true;
	Show(); //в EE_READ поменялась позиция курсора или размер табуляции.
}

void FileEditor::InitKeyBar()
{
	m_windowKeyBar->SetLabels(Global->OnlyEditorViewerUsed ? MSingleEditF1 : MEditF1);

	if (!GetCanLoseFocus())
	{
		m_windowKeyBar->Change(KBL_MAIN, L"", 12 - 1);
		m_windowKeyBar->Change(KBL_ALT, L"", 11 - 1);
		m_windowKeyBar->Change(KBL_SHIFT, L"", 4 - 1);
	}
	if (m_Flags.Check(FFILEEDIT_SAVETOSAVEAS))
		m_windowKeyBar->Change(KBL_MAIN, MSG(MEditShiftF2), 2 - 1);

	if (!m_Flags.Check(FFILEEDIT_ENABLEF6))
		m_windowKeyBar->Change(KBL_MAIN, L"", 6 - 1);

	if (m_codepage!=GetACP())
		m_windowKeyBar->Change(KBL_MAIN, MSG(Global->OnlyEditorViewerUsed ? MSingleEditF8 : MEditF8), 7);
	else
		m_windowKeyBar->Change(KBL_MAIN, MSG(Global->OnlyEditorViewerUsed ? MSingleEditF8DOS : MEditF8DOS), 7);

	m_windowKeyBar->SetCustomLabels(KBA_EDITOR);

	if (Global->Opt->EdOpt.ShowKeyBar)
		m_windowKeyBar->Show();
	else
		m_windowKeyBar->Hide();

	//m_editor->SetPosition(X1,Y1+(Global->Opt->EdOpt.ShowTitleBar?1:0),X2,Y2-(Global->Opt->EdOpt.ShowKeyBar?1:0));
}

void FileEditor::SetNamesList(NamesList& Names)
{
	EditNamesList = std::move(Names);
}

void FileEditor::Show()
{
	if (m_Flags.Check(FFILEEDIT_FULLSCREEN))
	{
		if (Global->Opt->EdOpt.ShowKeyBar)
		{
			m_windowKeyBar->SetPosition(0,ScrY,ScrX,ScrY);
			m_windowKeyBar->Redraw();
		}

		ScreenObjectWithShadow::SetPosition(0,0,ScrX,ScrY-(Global->Opt->EdOpt.ShowKeyBar?1:0));
		m_editor->SetPosition(0,(Global->Opt->EdOpt.ShowTitleBar?1:0),ScrX,ScrY-(Global->Opt->EdOpt.ShowKeyBar?1:0));
	}

	ScreenObjectWithShadow::Show();
}


void FileEditor::DisplayObject()
{
	if (!m_editor->Locked()&&!m_bClosing)
	{
		if (m_editor->m_Flags.Check(Editor::FEDITOR_ISRESIZEDCONSOLE))
		{
			m_editor->m_Flags.Clear(Editor::FEDITOR_ISRESIZEDCONSOLE);
			Global->CtrlObject->Plugins->ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL,m_editor->EditorID);
		}

		m_editor->Show();
	}
}

__int64 FileEditor::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	if (OpCode == MCODE_V_EDITORSTATE)
	{
		DWORD MacroEditState=0;
		MacroEditState|=m_Flags.Check(FFILEEDIT_NEW)?0x00000001:0;
		MacroEditState|=m_Flags.Check(FFILEEDIT_ENABLEF6)?0x00000002:0;
		MacroEditState|=m_Flags.Check(FFILEEDIT_DELETEONCLOSE)?0x00000004:0;
		MacroEditState|=m_editor->m_Flags.Check(Editor::FEDITOR_MODIFIED)?0x00000008:0;
		MacroEditState|=m_editor->BlockStart != m_editor->Lines.end()? 0x00000010 : 0;
		MacroEditState|=m_editor->VBlockStart != m_editor->Lines.end()? 0x00000020 : 0;
		MacroEditState|=m_editor->m_Flags.Check(Editor::FEDITOR_WASCHANGED)?0x00000040:0;
		MacroEditState|=m_editor->m_Flags.Check(Editor::FEDITOR_OVERTYPE)?0x00000080:0;
		MacroEditState|=m_editor->m_Flags.Check(Editor::FEDITOR_CURPOSCHANGEDBYPLUGIN)?0x00000100:0;
		MacroEditState|=m_editor->m_Flags.Check(Editor::FEDITOR_LOCKMODE)?0x00000200:0;
		MacroEditState|=m_editor->EdOpt.PersistentBlocks?0x00000400:0;
		MacroEditState|=Global->OnlyEditorViewerUsed ? 0x08000000 | 0x00000800 : 0;
		MacroEditState|=!GetCanLoseFocus()?0x00000800:0;
		return MacroEditState;
	}

	if (OpCode == MCODE_V_EDITORCURPOS)
		return m_editor->CurLine->GetTabCurPos()+1;

	if (OpCode == MCODE_V_EDITORCURLINE)
		return m_editor->NumLine+1;

	if (OpCode == MCODE_V_ITEMCOUNT || OpCode == MCODE_V_EDITORLINES)
		return m_editor->NumLastLine;

	if (OpCode == MCODE_F_KEYBAR_SHOW)
	{
		int PrevMode=Global->Opt->EdOpt.ShowKeyBar?2:1;
		switch (iParam)
		{
			case 0:
				break;
			case 1:
				Global->Opt->EdOpt.ShowKeyBar=1;
				m_windowKeyBar->Show();
				Show();
				m_KeyBarVisible = Global->Opt->EdOpt.ShowKeyBar;
				break;
			case 2:
				Global->Opt->EdOpt.ShowKeyBar=0;
				m_windowKeyBar->Hide();
				Show();
				m_KeyBarVisible = Global->Opt->EdOpt.ShowKeyBar;
				break;
			case 3:
				ProcessKey(Manager::Key(KEY_CTRLB));
				break;
			default:
				PrevMode=0;
				break;
		}
		return PrevMode;
	}

	return m_editor->VMProcess(OpCode,vParam,iParam);
}


int FileEditor::ProcessKey(const Manager::Key& Key)
{
	return ReProcessKey(Key.FarKey,FALSE);
}

int FileEditor::ReProcessKey(int Key,int CalledFromControl)
{
	if (Key!=KEY_F4 && Key!=KEY_IDLE)
		F4KeyOnly=false;

	if (m_Flags.Check(FFILEEDIT_REDRAWTITLE) && (((unsigned int)Key & 0x00ffffff) < KEY_END_FKEY || IsInternalKeyReal((unsigned int)Key & 0x00ffffff)))
		ShowConsoleTitle();

	// Все сотальные необработанные клавиши пустим далее
	/* $ 28.04.2001 DJ
	   не передаем KEY_MACRO* плагину - поскольку ReadRec в этом случае
	   никак не соответствует обрабатываемой клавише, возникают разномастные
	   глюки
	*/
	if (((unsigned int)Key >= KEY_MACRO_BASE && (unsigned int)Key <= KEY_MACRO_ENDBASE) || ((unsigned int)Key>=KEY_OP_BASE && (unsigned int)Key <=KEY_OP_ENDBASE)) // исключаем MACRO
	{
		; //
	}

	switch (Key)
	{
		case KEY_F6:
		{
			if (m_Flags.Check(FFILEEDIT_ENABLEF6))
			{
				int FirstSave=1;
				UINT cp=m_codepage;

				// проверка на "а может это говно удалили уже?"
				// возможно здесь она и не нужна!
				// хотя, раз уж были изменени, то
				if (m_editor->IsFileChanged() && // в текущем сеансе были изменения?
				        api::GetFileAttributes(strFullFileName) == INVALID_FILE_ATTRIBUTES) // а файл еще существует?
				{
					const wchar_t* const Items[] = {MSG(MEditSavedChangedNonFile),MSG(MEditSavedChangedNonFile2),MSG(MHYes),MSG(MHNo)};
					switch (Message(MSG_WARNING,2,MSG(MEditTitle),Items, ARRAYSIZE(Items), nullptr, nullptr, &EditorSaveF6DeletedId))
					{
						case 0:

							if (ProcessKey(Manager::Key(KEY_F2)))
							{
								FirstSave=0;
								break;
							}

						default:
							return FALSE;
					}
				}

				if (!FirstSave || m_editor->IsFileChanged() || api::GetFileAttributes(strFullFileName)!=INVALID_FILE_ATTRIBUTES)
				{
					__int64 FilePos=m_editor->GetCurPos(true, m_bAddSignature); // TODO: GetCurPos should return __int64

					/* $ 01.02.2001 IS
					   ! Открываем вьюер с указанием длинного имени файла, а не короткого
					*/
					int NeedQuestion = 1;
					if (ProcessQuitKey(FirstSave,NeedQuestion,false))
					{
						int delete_on_close = 0;
						if (m_Flags.Check(FFILEEDIT_DELETEONCLOSE))
							delete_on_close = 1;
						else if (m_Flags.Check(FFILEEDIT_DELETEONLYFILEONCLOSE))
							delete_on_close = 2;
						SetDeleteOnClose(0);

						new	FileViewer(
							strFullFileName,
							GetCanLoseFocus(), m_Flags.Check(FFILEEDIT_DISABLEHISTORY), FALSE,
							FilePos, nullptr, &EditNamesList, m_Flags.Check(FFILEEDIT_SAVETOSAVEAS), cp,
							strTitle.empty() ? nullptr : strTitle.data(),
							delete_on_close, this);
					}

					ShowTime(2);
				}

				return TRUE;
			}

			break; // отдадим F6 плагинам, если есть запрет на переключение
		}
		/* $ 10.05.2001 DJ
		   Alt-F11 - показать view/edit history
		*/
		case KEY_ALTF11:
		case KEY_RALTF11:
		{
			if (GetCanLoseFocus())
			{
				Global->CtrlObject->CmdLine->ShowViewEditHistory();
				return TRUE;
			}

			break; // отдадим Alt-F11 на растерзание плагинам, если редактор модальный
		}
	}

#if 1
	BOOL ProcessedNext=TRUE;

	_SVS(if (Key=='n' || Key=='m'))
		_SVS(SysLog(L"%d Key='%c'",__LINE__,Key));

	if (!CalledFromControl && (Global->CtrlObject->Macro.IsRecording() == MACROSTATE_RECORDING_COMMON || Global->CtrlObject->Macro.IsExecuting() == MACROSTATE_EXECUTING_COMMON || Global->CtrlObject->Macro.GetCurRecord() == MACROSTATE_NOMACRO))
	{

		_SVS(if (Global->CtrlObject->Macro.IsRecording() == MACROSTATE_RECORDING_COMMON || Global->CtrlObject->Macro.IsExecuting() == MACROSTATE_EXECUTING_COMMON))
			_SVS(SysLog(L"%d !!!! Global->CtrlObject->Macro.GetCurRecord() != MACROSTATE_NOMACRO !!!!",__LINE__));

		ProcessedNext=!ProcessEditorInput(Global->WindowManager->GetLastInputRecord());
	}

	if (ProcessedNext)
#else
	if (!CalledFromControl && //Global->CtrlObject->Macro.IsExecuting() || Global->CtrlObject->Macro.IsRecording() || // пусть доходят!
	        !ProcessEditorInput(WindowManager->GetLastInputRecord()))
#endif
	{

		switch (Key)
		{
			case KEY_F1:
			{
				Help Hlp(L"Editor");
				return TRUE;
			}
			/* $ 25.04.2001 IS
			     ctrl+f - вставить в строку полное имя редактируемого файла
			*/
			case KEY_CTRLF:
			case KEY_RCTRLF:
			{
				if (!m_editor->m_Flags.Check(Editor::FEDITOR_LOCKMODE))
				{
					m_editor->Pasting++;
					m_editor->TextChanged(true);

					if (!m_editor->EdOpt.PersistentBlocks && (m_editor->VBlockStart != m_editor->Lines.end() || m_editor->BlockStart != m_editor->Lines.end()))
					{
						m_editor->TurnOffMarkingBlock();
						m_editor->DeleteBlock();
					}

					//AddUndoData(CurLine->EditLine.GetStringAddr(),NumLine,
					//                CurLine->EditLine.GetCurPos(),UNDO_EDIT);
					m_editor->Paste(strFullFileName.data()); //???
					//if (!EdOpt.PersistentBlocks)
					m_editor->UnmarkBlock();
					m_editor->Pasting--;
					m_editor->Show(); //???
				}

				return TRUE;
			}
			/* $ 24.08.2000 SVS
			   + Добавляем реакцию показа бакграунда на клавишу CtrlAltShift
			*/
			case KEY_CTRLO:
			case KEY_RCTRLO:
			{
				m_editor->Hide();  // $ 27.09.2000 skv - To prevent redraw in macro with Ctrl-O

				if (Global->WindowManager->ShowBackground())
				{
					SetCursorType(false, 0);
					WaitKey();
				}

				Show();

				return TRUE;
			}
			case KEY_F2:
			case KEY_SHIFTF2:
			{
				BOOL Done=FALSE;
				string strOldCurDir;
				api::GetCurrentDirectory(strOldCurDir);
				while (!Done) // бьемся до упора
				{
					size_t pos;
					DWORD FNAttr;
					// проверим путь к файлу, может его уже снесли...

					// BUGBUG, похоже, не работает

					if (FindLastSlash(pos,strFullFileName))
					{
						string Path = strFullFileName.substr(pos);

						// В корне?
						if(IsRootPath(Path))
						{
							// а дальше? каталог существует?
							if ((FNAttr=api::GetFileAttributes(Path)) == INVALID_FILE_ATTRIBUTES ||
							        !(FNAttr&FILE_ATTRIBUTE_DIRECTORY)
							        //|| LocalStricmp(OldCurDir,FullFileName)  // <- это видимо лишнее.
							   )
								m_Flags.Set(FFILEEDIT_SAVETOSAVEAS);
						}
					}

					if (Key == KEY_F2 &&
					        (FNAttr=api::GetFileAttributes(strFullFileName)) != INVALID_FILE_ATTRIBUTES &&
					        !(FNAttr&FILE_ATTRIBUTE_DIRECTORY)
					   )
					{
						m_Flags.Clear(FFILEEDIT_SAVETOSAVEAS);
					}

					static int TextFormat=0;
					uintptr_t codepage = m_codepage;
					bool SaveAs = Key==KEY_SHIFTF2 || m_Flags.Check(FFILEEDIT_SAVETOSAVEAS);
					int NameChanged=FALSE;
					string strFullSaveAsName = strFullFileName;

					if (SaveAs)
					{
						string strSaveAsName = m_Flags.Check(FFILEEDIT_SAVETOSAVEAS)?strFullFileName:strFileName;

						if (!dlgSaveFileAs(strSaveAsName, TextFormat, codepage, m_bAddSignature))
							return FALSE;

						strSaveAsName = Unquote(api::env::expand_strings(strSaveAsName));
						NameChanged=StrCmpI(strSaveAsName, m_Flags.Check(FFILEEDIT_SAVETOSAVEAS)? strFullFileName : strFileName);

						if (!NameChanged)
							FarChDir(strStartDir); // ПОЧЕМУ? А нужно ли???

						if (NameChanged)
						{
							if (!AskOverwrite(strSaveAsName))
							{
								FarChDir(strOldCurDir);
								return TRUE;
							}
						}

						ConvertNameToFull(strSaveAsName, strFullSaveAsName);  //BUGBUG, не проверяем имя на правильность
						//это не про нас, про нас ниже, все куда страшнее
						/*string strFileNameTemp = strSaveAsName;

						if(!SetFileName(strFileNameTemp))
						{
						  SetLastError(ERROR_INVALID_NAME);
						  Global->CatchError();
						  Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MEditTitle),strFileNameTemp,MSG(MOk));
						  if(!NameChanged)
						    FarChDir(strOldCurDir);
						  continue;
						  //return FALSE;
						} */

						if (!NameChanged)
							FarChDir(strOldCurDir);
					}

					ShowConsoleTitle();
					FarChDir(strStartDir); //???
					int SaveResult=SaveFile(strFullSaveAsName, 0, SaveAs, TextFormat, codepage, m_bAddSignature);

					if (SaveResult==SAVEFILE_ERROR)
					{
						SetLastError(SysErrorCode);
						Global->CatchError();
						if (OperationFailed(strFullFileName, MEditTitle, MSG(MEditCannotSave), false))
						{
							Done=TRUE;
							break;
						}
					}
					else if (SaveResult==SAVEFILE_SUCCESS)
					{
						//здесь идет полная жопа, проверка на ошибки вообще пока отсутствует
						{
							bool bInPlace = /*(!IsUnicodeOrUtfCodePage(m_codepage) && !IsUnicodeOrUtfCodePage(codepage)) || */(m_codepage == codepage);

							if (!bInPlace)
							{
								m_editor->FreeAllocatedData();
								m_editor->InsertString(nullptr, 0, m_editor->LastLine);
								m_codepage = codepage;
							}

							SetFileName(strFullSaveAsName);

							if (!bInPlace)
							{
								//Message(MSG_WARNING, 1, L"WARNING!", L"Editor will be reopened with new file!", MSG(MOk));
								int UserBreak;
								LoadFile(strFullSaveAsName, UserBreak);
								// TODO: возможно подобный ниже код здесь нужен (copy/paste из FileEditor::Init()). оформить его нужно по иному
								//if(!Global->Opt->Confirm.Esc && UserBreak && GetExitCode()==XC_LOADING_INTERRUPTED && WindowManager)
								//  WindowManager->RefreshWindow();
							}

							// перерисовывать надо как минимум когда изменилась кодировка или имя файла
							ShowConsoleTitle();
							Show();//!!! BUGBUG
						}
						Done=TRUE;
					}
					else
					{
						Done=TRUE;
						break;
					}
				}

				return TRUE;
			}
			// $ 30.05.2003 SVS - Shift-F4 в редакторе/вьювере позволяет открывать другой редактор/вьювер (пока только редактор)
			case KEY_SHIFTF4:
			{
				if (!Global->OnlyEditorViewerUsed && GetCanLoseFocus())
				{
					if (!m_Flags.Check(FFILEEDIT_DISABLESAVEPOS) && (m_editor->EdOpt.SavePos || m_editor->EdOpt.SaveShortPos)) // save position/codepage before reload
						SaveToCache();
					Global->CtrlObject->Cp()->ActivePanel()->ProcessKey(Manager::Key(Key));
				}
				return TRUE;
			}
			// $ 21.07.2000 SKV + выход с позиционированием на редактируемом файле по CTRLF10
			case KEY_CTRLF10:
			case KEY_RCTRLF10:
			{
				if (isTemporary())
				{
					return TRUE;
				}

				string strFullFileNameTemp = strFullFileName;

				if (api::GetFileAttributes(strFullFileName) == INVALID_FILE_ATTRIBUTES) // а сам файл то еще на месте?
				{
					if (!CheckShortcutFolder(&strFullFileNameTemp,FALSE))
						return FALSE;

					strFullFileNameTemp += L"\\."; // для вваливания внутрь :-)
				}

				Panel *ActivePanel = Global->CtrlObject->Cp()->ActivePanel();

				if (m_Flags.Check(FFILEEDIT_NEW) || (ActivePanel && ActivePanel->FindFile(strFileName) == -1)) // Mantis#279
				{
					UpdateFileList();
					m_Flags.Clear(FFILEEDIT_NEW);
				}

				{
					SaveScreen Sc;
					Global->CtrlObject->Cp()->GoToFile(strFullFileNameTemp);
					m_Flags.Set(FFILEEDIT_REDRAWTITLE);
				}

				return TRUE;
			}
			case KEY_CTRLB:
			case KEY_RCTRLB:
			{
				Global->Opt->EdOpt.ShowKeyBar=!Global->Opt->EdOpt.ShowKeyBar;

				if (Global->Opt->EdOpt.ShowKeyBar)
					m_windowKeyBar->Show();
				else
					m_windowKeyBar->Hide();

				Show();
				m_KeyBarVisible = Global->Opt->EdOpt.ShowKeyBar;
				return TRUE;
			}
			case KEY_CTRLSHIFTB:
			case KEY_RCTRLSHIFTB:
			{
				Global->Opt->EdOpt.ShowTitleBar=!Global->Opt->EdOpt.ShowTitleBar;
				m_TitleBarVisible = Global->Opt->EdOpt.ShowTitleBar;
				Show();
				return TRUE;
			}
			case KEY_SHIFTF10:

				if (!ProcessKey(Manager::Key(KEY_F2))) // учтем факт того, что могли отказаться от сохранения
					return FALSE;

			case KEY_F4:
				if (F4KeyOnly)
					return TRUE;
			case KEY_ESC:
			case KEY_F10:
			{
				int FirstSave=1, NeedQuestion=1;

				if (Key != KEY_SHIFTF10)   // KEY_SHIFTF10 не учитываем!
				{
					bool FilePlaced=api::GetFileAttributes(strFullFileName) == INVALID_FILE_ATTRIBUTES && !m_Flags.Check(FFILEEDIT_NEW);

					if (m_editor->IsFileChanged() || // в текущем сеансе были изменения?
					        FilePlaced) // а сам файл то еще на месте?
					{
						int Res=100;

						LNGID MsgLine1=MNewFileName;
						if (m_editor->IsFileChanged() && FilePlaced)
							MsgLine1=MEditSavedChangedNonFile;
						else if (!m_editor->IsFileChanged() && FilePlaced)
							MsgLine1=MEditSavedChangedNonFile1;

						if (MsgLine1 != MNewFileName)
						{
							const wchar_t* const Items[] = {MSG(MsgLine1),MSG(MEditSavedChangedNonFile2),MSG(MHYes),MSG(MHNo),MSG(MHCancel)};
							Res=Message(MSG_WARNING,3,MSG(MEditTitle),Items, ARRAYSIZE(Items), nullptr, nullptr, &EditorSaveExitDeletedId);
						}

						switch (Res)
						{
							case 0:

								if (!ProcessKey(Manager::Key(KEY_F2))) // попытка сначала сохранить
									NeedQuestion=0;

								FirstSave=0;
								break;
							case 1:
								NeedQuestion=0;
								FirstSave=0;
								break;
							case 100:
								FirstSave=NeedQuestion=1;
								break;
							case 2:
							default:
								return FALSE;
						}
					}
					else if (!m_editor->m_Flags.Check(Editor::FEDITOR_MODIFIED)) //????
						NeedQuestion=0;
				}

				if (!ProcessQuitKey(FirstSave,NeedQuestion))
					return FALSE;

				return TRUE;
			}

			case KEY_F8:
			case KEY_SHIFTF8:
			{
				uintptr_t codepage = m_codepage;
				if (Key == KEY_F8)
					codepage = m_codepage == GetACP() ? GetOEMCP() : GetACP();
				else
				{
					if (Codepages().SelectCodePage(codepage, true, false, true))
					{
						if (codepage == CP_DEFAULT)
						{
							api::File edit_file;
							bool detect = false, sig_found = false;

							if (edit_file.Open(strFileName, FILE_READ_DATA, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, nullptr, OPEN_EXISTING))
							{
								detect = GetFileFormat(edit_file,codepage,&sig_found,true);
								edit_file.Close();
							}
							if (!detect)
							{
								Message(MSG_WARNING,1,MSG(MEditTitle),MSG(MEditorCPNotDetected),MSG(MOk));
							}
							else if (!Codepages().IsCodePageSupported(codepage))
							{
								detect = false;
								Message(MSG_WARNING, 1, MSG(MEditTitle), (LangString(MEditorCPNotSupported) << codepage).data(), MSG(MOk));
							}
							if ( !detect )
								codepage = m_codepage;
						}
					}
				}

				bool need_reload = !m_Flags.Check(FFILEEDIT_NEW) // we can't reload non-existing file
					&& (BadConversion
					|| IsUnicodeCodePage(m_codepage) || m_codepage == CP_UTF7
					|| IsUnicodeCodePage(codepage));


				if (codepage != m_codepage && need_reload && IsFileModified())
				{
					int res = Message(
						MSG_WARNING, 2, MSG(MEditTitle),
						MSG(MEditorReloadCPWarnLost1), MSG(MEditorReloadCPWarnLost2),
						MSG(MOk), MSG(MCancel)
					);
					if (res != 0)
						codepage = m_codepage;
				}

				if (codepage != m_codepage)
				{
					uintptr_t cp0 = m_codepage;
					if (need_reload)
					{
						ReloadFile(codepage);
					}
					else
						SetCodePage(codepage);

					if (m_codepage != cp0)
					{
						InitKeyBar();
						m_Flags.Set(FFILEEDIT_CODEPAGECHANGEDBYUSER);
					}
				}

				return TRUE;
			}
			case KEY_ALTSHIFTF9:
			case KEY_RALTSHIFTF9:
			{
				// Работа с локальной копией EditorOptions
				Options::EditorOptions EdOpt;
				GetEditorOptions(EdOpt);
				Global->Opt->LocalEditorConfig(EdOpt); // $ 27.11.2001 DJ - Local в EditorConfig
				m_windowKeyBar->Show(); //???? Нужно ли????
				SetEditorOptions(EdOpt);

				if (Global->Opt->EdOpt.ShowKeyBar)
					m_windowKeyBar->Show();

				m_editor->Show();
				return TRUE;
			}
			default:
			{
				if (m_Flags.Check(FFILEEDIT_FULLSCREEN) && Global->CtrlObject->Macro.IsExecuting() == MACROSTATE_NOMACRO)
					if (Global->Opt->EdOpt.ShowKeyBar)
						m_windowKeyBar->Show();

				if (!m_windowKeyBar->ProcessKey(Manager::Key(Key)))
					return m_editor->ProcessKey(Manager::Key(Key));
			}
		}
	}
	return TRUE;
}


int FileEditor::ProcessQuitKey(int FirstSave,BOOL NeedQuestion,bool DeleteWindow)
{
	string strOldCurDir;
	api::GetCurrentDirectory(strOldCurDir);

	for (;;)
	{
		FarChDir(strStartDir); // ПОЧЕМУ? А нужно ли???
		int SaveCode=SAVEFILE_SUCCESS;

		if (NeedQuestion)
		{
			SaveCode=SaveFile(strFullFileName,FirstSave,0,FALSE);
		}

		if (SaveCode==SAVEFILE_CANCEL)
			break;

		if (SaveCode==SAVEFILE_SUCCESS)
		{
			/* $ 09.02.2002 VVM
			  + Обновить панели, если писали в текущий каталог */
			if (NeedQuestion)
			{
				if (api::GetFileAttributes(strFullFileName)!=INVALID_FILE_ATTRIBUTES)
				{
					UpdateFileList();
				}
			}

			if (DeleteWindow)
			{
				Global->WindowManager->DeleteWindow();
			}
			SetExitCode(XC_QUIT);
			break;
		}

		if (strFileName == MSG(MNewFileName))
		{
			if (!ProcessKey(Manager::Key(KEY_SHIFTF2)))
			{
				FarChDir(strOldCurDir);
				return FALSE;
			}
			else
				break;
		}

		SetLastError(SysErrorCode);
		Global->CatchError();
		if (OperationFailed(strFullFileName, MEditTitle, MSG(MEditCannotSave), false))
			break;

		FirstSave=0;
	}

	FarChDir(strOldCurDir);
	return GetExitCode() == XC_QUIT;
}


// сюды плавно переносить код из Editor::ReadFile()
int FileEditor::LoadFile(const string& Name,int &UserBreak)
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);
	SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<Editor::EditorPreRedrawItem>());
	SCOPED_ACTION(IndeterminateTaskBar);
	SCOPED_ACTION(wakeful);
	int LastLineCR = 0;
	EditorPosCache pc;
	UserBreak = 0;
	api::File EditFile;
	if(!EditFile.Open(Name, FILE_READ_DATA, FILE_SHARE_READ|(Global->Opt->EdOpt.EditOpenedForWrite?FILE_SHARE_WRITE:0), nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN))
	{
		SysErrorCode=GetLastError();

		if ((SysErrorCode != ERROR_FILE_NOT_FOUND) && (SysErrorCode != ERROR_PATH_NOT_FOUND))
		{
			UserBreak = -1;
			m_Flags.Set(FFILEEDIT_OPENFAILED);
		}

		return FALSE;
	}

	/*if (GetFileType(hEdit) != FILE_TYPE_DISK)
	{
		fclose(EditFile);
		SetLastError(ERROR_INVALID_NAME);
		UserBreak=-1;
		Flags.Set(FFILEEDIT_OPENFAILED);
		return FALSE;
	}*/

	if (Global->Opt->EdOpt.FileSizeLimit)
	{
		UINT64 FileSize=0;
		if (EditFile.GetSize(FileSize))
		{
			if (FileSize > static_cast<UINT64>(Global->Opt->EdOpt.FileSizeLimit))
			{
				string strTempStr1, strTempStr2;
				// Ширина = 8 - это будет... в Kb и выше...
				FileSizeToStr(strTempStr1, FileSize, 8);
				FileSizeToStr(strTempStr2, Global->Opt->EdOpt.FileSizeLimit, 8);

				const wchar_t* const Items[] = {Name.data(),
					(LangString(MEditFileLong) << RemoveExternalSpaces(strTempStr1)).data(),
					(LangString(MEditFileLong2) << RemoveExternalSpaces(strTempStr2)).data(),
					MSG(MEditROOpen), MSG(MYes),MSG(MNo)};

				if (Message(MSG_WARNING,2,MSG(MEditTitle),Items, ARRAYSIZE(Items), nullptr, nullptr, &EditorFileLongId))
				{
					EditFile.Close();
					SetLastError(ERROR_OPEN_FAILED); //????
					UserBreak=1;
					m_Flags.Set(FFILEEDIT_OPENFAILED);
					return FALSE;
				}
			}
		}
		else
		{
			const wchar_t* const Items[] = {Name.data(),MSG(MEditFileGetSizeError),MSG(MEditROOpen),MSG(MYes),MSG(MNo)};
			if (Message(MSG_WARNING,2,MSG(MEditTitle),Items, ARRAYSIZE(Items), nullptr, nullptr, &EditorFileGetSizeErrorId))
			{
				EditFile.Close();
				SetLastError(SysErrorCode=ERROR_OPEN_FAILED); //????
				UserBreak=1;
				m_Flags.Set(FFILEEDIT_OPENFAILED);
				return FALSE;
			}
		}
	}

	for (BitFlags f0 = m_editor->m_Flags; true; m_editor->m_Flags = f0)
	{
		m_editor->FreeAllocatedData(false);
		bool bCached = LoadFromCache(pc);

		DWORD FileAttributes=api::GetFileAttributes(Name);
		if((m_editor->EdOpt.ReadOnlyLock&1) && FileAttributes != INVALID_FILE_ATTRIBUTES && (FileAttributes & (FILE_ATTRIBUTE_READONLY|((m_editor->EdOpt.ReadOnlyLock&0x60)>>4))))
		{
			m_editor->m_Flags.Swap(Editor::FEDITOR_LOCKMODE);
		}

		// Проверяем поддерживается или нет загруженная кодовая страница
		if (bCached && pc.CodePage && !Codepages().IsCodePageSupported(pc.CodePage))
			pc.CodePage = 0;

		m_editor->GlobalEOL.clear(); //BUGBUG???
		uintptr_t dwCP=0;
		bool Detect=false;

		bool redetect = (m_codepage == CP_REDETECT);
		if (redetect)
			m_codepage = CP_DEFAULT;

		if (m_codepage == CP_DEFAULT || IsUnicodeOrUtfCodePage(m_codepage))
		{
			Detect=GetFileFormat(EditFile,dwCP,&m_bAddSignature,redetect || Global->Opt->EdOpt.AutoDetectCodePage!=0);

			// Проверяем поддерживается или нет задетектировання кодовая страница
			if (Detect)
				Detect = Codepages().IsCodePageSupported(dwCP);
		}

		if (m_codepage == CP_DEFAULT)
		{
			if (Detect)
			{
				m_codepage=dwCP;
			}

			if (!redetect && bCached)
			{
				if (pc.CodePage)
				{
					m_codepage = pc.CodePage;
					m_Flags.Set(FFILEEDIT_CODEPAGECHANGEDBYUSER);
				}
			}

			if (m_codepage==CP_DEFAULT)
				m_codepage = GetDefaultCodePage();
		}
		else
		{
			m_Flags.Set(FFILEEDIT_CODEPAGECHANGEDBYUSER);
		}

		m_editor->SetCodePage(m_codepage);  //BUGBUG

		if (!IsUnicodeOrUtfCodePage(m_codepage))
		{
			EditFile.SetPointer(0, nullptr, FILE_BEGIN);
		}

		UINT64 FileSize=0;
		EditFile.GetSize(FileSize);
		DWORD StartTime=GetTickCount();

		GetFileString GetStr(EditFile, m_codepage);

		wchar_t *Str;
		size_t StrLength;
		while (GetStr.GetString(&Str, StrLength))
		{
			LastLineCR=0;
			DWORD CurTime=GetTickCount();

			if (CurTime-StartTime>(DWORD)Global->Opt->RedrawTimeout)
			{
				StartTime=CurTime;

				if (CheckForEscSilent())
				{
					if (ConfirmAbortOp())
					{
						UserBreak = 1;
						EditFile.Close();
						return FALSE;
					}
				}

				SetCursorType(false, 0);
				INT64 CurPos = EditFile.GetPointer();
				int Percent=static_cast<int>(CurPos*100/FileSize);
				// В случае если во время загрузки файл увеличивается размере, то количество
				// процентов может быть больше 100. Обрабатываем эту ситуацию.
				if (Percent>100)
				{
					EditFile.GetSize(FileSize);
					Percent=static_cast<int>(CurPos*100/FileSize);
					if (Percent>100)
					{
						Percent=100;
					}
				}
				Editor::EditorShowMsg(MSG(MEditTitle),MSG(MEditReading),Name,Percent);
			}

			const wchar_t *CurEOL;

			size_t Offset = StrLength > 3 ? StrLength - 3 : 0;

			if ((CurEOL = wmemchr(Str+Offset,L'\r',StrLength-Offset))  ||
				(CurEOL = wmemchr(Str+Offset,L'\n',StrLength-Offset)))
			{
				m_editor->GlobalEOL = CurEOL;
				LastLineCR=1;
			}

			m_editor->InsertString(Str, static_cast<int>(StrLength), m_editor->LastLine);
		}

		BadConversion = !GetStr.IsConversionValid();
		if (BadConversion)
		{
			uintptr_t cp = m_codepage;
			if (!dlgBadEditorCodepage(cp)) // cancel
			{
				EditFile.Close();
				SetLastError(SysErrorCode=ERROR_OPEN_FAILED);
				UserBreak=1;
				m_Flags.Set(FFILEEDIT_OPENFAILED);
				return FALSE;
			}
			else if (cp != m_codepage)
			{
				m_codepage = cp;
				EditFile.SetPointer(0, nullptr, FILE_BEGIN);
				continue;
			}
			// else -- codepage accepted
		}
		break;
	}

	if (LastLineCR||!m_editor->NumLastLine)
		m_editor->InsertString(L"", 0, m_editor->LastLine);

	EditFile.Close();
	//if ( bCached )
	m_editor->SetCacheParams(pc, m_bAddSignature);
	SysErrorCode=GetLastError();
	api::GetFindDataEx(Name, FileInfo);
	EditorGetFileAttributes(Name);
	return TRUE;
}

bool FileEditor::ReloadFile(uintptr_t codepage)
{
	m_editor->Lock();

	auto save_codepage(m_codepage), save_codepage1(m_editor->m_codepage);
	auto save_bAddSignature(m_bAddSignature);
	auto save_BadConversiom(BadConversion);
	auto save_Flags(m_Flags), save_Flags1(m_editor->m_Flags);

	Editor saved;
	saved.fake_editor = true;
	m_editor->SwapState(saved);

	int user_break = 0;
	m_codepage = codepage;
	int loaded = LoadFile(strFullFileName, user_break);
	if (!loaded)
	{
		m_codepage = save_codepage;
		m_bAddSignature = save_bAddSignature;
		BadConversion = save_BadConversiom;
		m_Flags = save_Flags;
		m_editor->m_codepage = save_codepage1;
		m_editor->m_Flags = save_Flags1;
		m_editor->SwapState(saved);

		if (user_break != 1)
		{
			SetLastError(SysErrorCode);
			Global->CatchError();
			OperationFailed(strFullFileName, MEditTitle, MSG(MEditCannotOpen), false);
		}
	}
	else
	{
		m_editor->m_Flags.Set(Editor::FEDITOR_WASCHANGED);
		m_editor->m_Flags.Clear(Editor::FEDITOR_MODIFIED);
	}

	m_editor->Unlock();
	Show();
	return loaded != FALSE;
}

//TextFormat и codepage используются ТОЛЬКО, если bSaveAs = true!
int FileEditor::SaveFile(const string& Name,int Ask, bool bSaveAs, int TextFormat, uintptr_t codepage, bool AddSignature)
{
	if (!bSaveAs)
	{
		TextFormat=0;
		codepage=m_editor->GetCodePage();
	}

	SCOPED_ACTION(IndeterminateTaskBar);
	SCOPED_ACTION(wakeful);

	if (m_editor->m_Flags.Check(Editor::FEDITOR_LOCKMODE) && !m_editor->m_Flags.Check(Editor::FEDITOR_MODIFIED) && !bSaveAs)
		return SAVEFILE_SUCCESS;

	if (Ask)
	{
		if (!m_editor->m_Flags.Check(Editor::FEDITOR_MODIFIED))
			return SAVEFILE_SUCCESS;

		if (Ask)
		{
			const wchar_t* const Items[] = {MSG(MEditAskSave),MSG(MHYes),MSG(MHNo),MSG(MHCancel)};
			int skip = Global->AllowCancelExit ? 0 : 1;
			int Code = Message(MSG_WARNING,3-skip,MSG(MEditTitle),Items, ARRAYSIZE(Items)-skip, nullptr, nullptr, &EditAskSaveId);
			if(Code < 0 && !Global->AllowCancelExit)
			{
				Code = 1; // close == not save
			}
			switch (Code)
			{
				case -1:
				case -2:
				case 2:  // Continue Edit
					return SAVEFILE_CANCEL;
				case 0:  // Save
					break;
				case 1:  // Not Save
					m_editor->TextChanged(false); // 10.08.2000 skv: TextChanged() support;
					return SAVEFILE_SUCCESS;
			}
		}
	}

	int NewFile=TRUE;
	FileAttributesModified=false;

	if ((m_FileAttributes=api::GetFileAttributes(Name))!=INVALID_FILE_ATTRIBUTES)
	{
		// Проверка времени модификации...
		if (!m_Flags.Check(FFILEEDIT_SAVEWQUESTIONS))
		{
			api::FAR_FIND_DATA FInfo;

			if (api::GetFindDataEx(Name, FInfo) && !FileInfo.strFileName.empty())
			{
				if (FileInfo.ftLastWriteTime != FInfo.ftLastWriteTime || FInfo.nFileSize != FileInfo.nFileSize)
				{
					const wchar_t* const Items[] = {MSG(MEditAskSaveExt), MSG(MHYes), MSG(MEditBtnSaveAs), MSG(MHCancel)};
					switch (Message(MSG_WARNING, 3, MSG(MEditTitle), Items, ARRAYSIZE(Items), L"WarnEditorSavedEx", nullptr, &EditAskSaveExtId))
					{
						case -1:
						case -2:
						case 2:  // Continue Edit
							return SAVEFILE_CANCEL;
						case 1:  // Save as

							if (ProcessKey(Manager::Key(KEY_SHIFTF2)))
								return SAVEFILE_SUCCESS;
							else
								return SAVEFILE_CANCEL;

						case 0:  // Save
							break;
					}
				}
			}
		}

		m_Flags.Clear(FFILEEDIT_SAVEWQUESTIONS);
		NewFile=FALSE;

		if (m_FileAttributes & FILE_ATTRIBUTE_READONLY)
		{
			//BUGBUG
			const wchar_t* const Items[] = {Name.data(),MSG(MEditRO),MSG(MEditOvr),MSG(MYes),MSG(MNo)};
			int AskOverwrite=Message(MSG_WARNING,2,MSG(MEditTitle),Items, ARRAYSIZE(Items), nullptr, nullptr, &EditorSavedROId);

			if (AskOverwrite)
				return SAVEFILE_CANCEL;

			api::SetFileAttributes(Name,m_FileAttributes & ~FILE_ATTRIBUTE_READONLY); // сняты атрибуты
			FileAttributesModified=true;
		}

		if (m_FileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))
		{
			api::SetFileAttributes(Name,FILE_ATTRIBUTE_NORMAL);
			FileAttributesModified=true;
		}
	}
	else
	{
		// проверим путь к файлу, может его уже снесли...
		string strCreatedPath = Name;
		const wchar_t *Ptr=LastSlash(strCreatedPath.data());

		if (Ptr)
		{
			CutToSlash(strCreatedPath);
			DWORD FAttr=0;

			if (api::GetFileAttributes(strCreatedPath) == INVALID_FILE_ATTRIBUTES)
			{
				// и попробуем создать.
				// Раз уж
				CreatePath(strCreatedPath);
				FAttr=api::GetFileAttributes(strCreatedPath);
			}

			if (FAttr == INVALID_FILE_ATTRIBUTES)
				return SAVEFILE_ERROR;
		}
	}

	if (BadConversion)
	{
		if(Message(MSG_WARNING,2,MSG(MWarning),MSG(MEditDataLostWarn),MSG(MEditorSaveNotRecommended),MSG(MEditorSave),MSG(MCancel)))
		{
			return SAVEFILE_CANCEL;
		}
		else
		{
			BadConversion = false;
		}
	}

	int RetCode=SAVEFILE_SUCCESS;

	if (TextFormat)
		m_editor->m_Flags.Set(Editor::FEDITOR_WASCHANGED);

	switch (TextFormat)
	{
		case 1:
			m_editor->GlobalEOL = DOS_EOL_fmt;
			break;
		case 2:
			m_editor->GlobalEOL = UNIX_EOL_fmt;
			break;
		case 3:
			m_editor->GlobalEOL = MAC_EOL_fmt;
			break;
		case 4:
			m_editor->GlobalEOL = WIN_EOL_fmt;
			break;
	}

	if (api::GetFileAttributes(Name) == INVALID_FILE_ATTRIBUTES)
		m_Flags.Set(FFILEEDIT_NEW);

	{
		//SaveScreen SaveScr;
		/* $ 11.10.2001 IS
		   Если было произведено сохранение с любым результатом, то не удалять файл
		*/
		m_Flags.Clear(FFILEEDIT_DELETEONCLOSE|FFILEEDIT_DELETEONLYFILEONCLOSE);
//_D(SysLog(L"%08d EE_SAVE",__LINE__));

		if (!IsUnicodeOrUtfCodePage(codepage))
		{
			int LineNumber=-1;
			bool BadSaveConfirmed=false;
			FOR_RANGE(m_editor->Lines, CurPtr)
			{
				++LineNumber;
				const wchar_t *SaveStr, *EndSeq;
				intptr_t Length;
				CurPtr->GetBinaryString(&SaveStr,&EndSeq,Length);
				BOOL UsedDefaultCharStr=FALSE,UsedDefaultCharEOL=FALSE;
				WideCharToMultiByte(codepage,WC_NO_BEST_FIT_CHARS,SaveStr,Length,nullptr,0,nullptr,&UsedDefaultCharStr);

				if (!*EndSeq && CurPtr != m_editor->LastLine)
					EndSeq = m_editor->GlobalEOL.empty() ? DOS_EOL_fmt : m_editor->GlobalEOL.data();

				if (TextFormat&&*EndSeq)
					EndSeq=m_editor->GlobalEOL.data();

				WideCharToMultiByte(codepage,WC_NO_BEST_FIT_CHARS,EndSeq,StrLength(EndSeq),nullptr,0,nullptr,&UsedDefaultCharEOL);

				if (!BadSaveConfirmed && (UsedDefaultCharStr||UsedDefaultCharEOL))
				{
					//SetMessageHelp(L"EditorDataLostWarning")
					int Result=Message(MSG_WARNING,3,MSG(MWarning),MSG(MEditorSaveCPWarn1),MSG(MEditorSaveCPWarn2),MSG(MEditorSaveNotRecommended),MSG(MCancel),MSG(MEditorSaveCPWarnShow),MSG(MEditorSave));
					if (Result==2)
					{
						BadSaveConfirmed=true;
						break;
					}
					else
					{
						if(Result==1)
						{
							m_editor->GoToLine(LineNumber);
							if(UsedDefaultCharStr)
							{
								for(int Pos=0;Pos<Length;Pos++)
								{
									BOOL UseDefChar=0;
									WideCharToMultiByte(codepage,WC_NO_BEST_FIT_CHARS,SaveStr+Pos,1,nullptr,0,nullptr,&UseDefChar);
									if(UseDefChar)
									{
										CurPtr->SetCurPos(Pos);
										break;
									}
								}
							}
							else
							{
								CurPtr->SetCurPos(CurPtr->GetLength());
							}
							Show();
						}
						return SAVEFILE_CANCEL;
					}
				}
			}
		}

		EditorSaveFile esf = {sizeof(esf), Name.data(), m_editor->GlobalEOL.data(), codepage};
		Global->CtrlObject->Plugins->ProcessEditorEvent(EE_SAVE, &esf, m_editor->EditorID);
		api::File EditFile;
		DWORD dwWritten=0;
		// Don't use CreationDisposition=CREATE_ALWAYS here - it's kills alternate streams
		if(!EditFile.Open(Name, m_Flags.Check(FFILEEDIT_NEW)? FILE_WRITE_DATA : GENERIC_WRITE, FILE_SHARE_READ, nullptr, m_Flags.Check(FFILEEDIT_NEW)? CREATE_NEW : TRUNCATE_EXISTING, FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN))
		{
			//_SVS(SysLogLastError();SysLog(L"Name='%s',FileAttributes=%d",Name,FileAttributes));
			RetCode=SAVEFILE_ERROR;
			SysErrorCode=GetLastError();
			goto end;
		}

		m_editor->UndoSavePos=m_editor->UndoPos;
		m_editor->m_Flags.Clear(Editor::FEDITOR_UNDOSAVEPOSLOST);
//    ConvertNameToFull(Name,FileName, sizeof(FileName));
		/*
		    if (ConvertNameToFull(Name,m_editor->FileName, sizeof(m_editor->FileName)) >= sizeof(m_editor->FileName))
		    {
		      m_editor->Flags.Set(FEDITOR_OPENFAILED);
		      RetCode=SAVEFILE_ERROR;
		      goto end;
		    }
		*/
		SetCursorType(false, 0);
		SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<Editor::EditorPreRedrawItem>());

		if (!bSaveAs)
			AddSignature=m_bAddSignature;

		if (AddSignature)
		{
			DWORD dwSignature = 0;
			DWORD SignLength=0;

			switch (codepage)
			{
				case CP_UNICODE:
					dwSignature = SIGN_UNICODE;
					SignLength=2;
					break;
				case CP_REVERSEBOM:
					dwSignature = SIGN_REVERSEBOM;
					SignLength=2;
					break;
				case CP_UTF8:
					dwSignature = SIGN_UTF8;
					SignLength=3;
					break;
			}

			if (!EditFile.Write(&dwSignature,SignLength,dwWritten,nullptr)||dwWritten!=SignLength)
			{
				EditFile.Close();
				api::DeleteFile(Name);
				RetCode=SAVEFILE_ERROR;
				goto end;
			}
		}

		DWORD StartTime=GetTickCount();
		size_t LineNumber = -1;
		CachedWrite Cache(EditFile);

		FOR_RANGE(m_editor->Lines, CurPtr)
		{
			++LineNumber;
			DWORD CurTime=GetTickCount();

			if (CurTime-StartTime>(DWORD)Global->Opt->RedrawTimeout)
			{
				StartTime=CurTime;
				Editor::EditorShowMsg(MSG(MEditTitle),MSG(MEditSaving),Name,(int)(LineNumber*100/m_editor->NumLastLine));
			}

			const wchar_t *SaveStr, *EndSeq;

			intptr_t Length;

			CurPtr->GetBinaryString(&SaveStr,&EndSeq,Length);

			if (!*EndSeq && CurPtr != m_editor->LastLine && (*CurPtr->GetEOL()))
				EndSeq = m_editor->GlobalEOL.empty()? DOS_EOL_fmt : m_editor->GlobalEOL.data();

			if (TextFormat && *EndSeq)
			{
				EndSeq = m_editor->GlobalEOL.data();
				CurPtr->SetEOL(EndSeq);
			}

			int EndLength=StrLength(EndSeq);
			bool bError = false;

			if (codepage == CP_UNICODE)
			{
				if (
				    (Length && !Cache.Write(SaveStr,Length*sizeof(wchar_t))) ||
				    (EndLength && !Cache.Write(EndSeq,EndLength*sizeof(wchar_t)))
						)
				{
					SysErrorCode=GetLastError();
					bError = true;
				}
			}
			else
			{
				auto SwapBytes = [](const wchar_t* src, char* dst, size_t count)
				{
					return _swab(reinterpret_cast<char*>(const_cast<wchar_t*>(src)), dst, static_cast<int>(count));
				};

				if (Length)
				{
					DWORD length = (codepage == CP_REVERSEBOM?static_cast<DWORD>(Length*sizeof(wchar_t)):WideCharToMultiByte(codepage, 0, SaveStr, Length, nullptr, 0, nullptr, nullptr));
					char_ptr SaveStrCopy(length);

					if (SaveStrCopy)
					{
						if (codepage == CP_REVERSEBOM)
							SwapBytes(SaveStr, SaveStrCopy.get(), length);
						else
							WideCharToMultiByte(codepage, 0, SaveStr, Length, SaveStrCopy.get(), length, nullptr, nullptr);

						if (!Cache.Write(SaveStrCopy.get(),length))
						{
							bError = true;
							SysErrorCode=GetLastError();
						}
					}
					else
						bError = true;
				}

				if (!bError)
				{
					if (EndLength)
					{
						DWORD endlength = (codepage == CP_REVERSEBOM?static_cast<DWORD>(EndLength*sizeof(wchar_t)):WideCharToMultiByte(codepage, 0, EndSeq, EndLength, nullptr, 0, nullptr, nullptr));
						char_ptr EndSeqCopy(endlength);

						if (EndSeqCopy)
						{
							if (codepage == CP_REVERSEBOM)
								SwapBytes(EndSeq, EndSeqCopy.get(), endlength);
							else
								WideCharToMultiByte(codepage, 0, EndSeq, EndLength, EndSeqCopy.get(), endlength, nullptr, nullptr);

							if (!Cache.Write(EndSeqCopy.get(),endlength))
							{
								bError = true;
								SysErrorCode=GetLastError();
							}
						}
						else
							bError = true;
					}
				}
			}

			if (bError)
			{
				EditFile.Close();
				api::DeleteFile(Name);
				RetCode=SAVEFILE_ERROR;
				goto end;
			}
		}

		if(Cache.Flush())
		{
			EditFile.SetEnd();
			EditFile.Close();
		}
		else
		{
			SysErrorCode=GetLastError();
			EditFile.Close();
			api::DeleteFile(Name);
			RetCode=SAVEFILE_ERROR;
		}
	}

end:

	if (m_FileAttributes!=INVALID_FILE_ATTRIBUTES && FileAttributesModified)
	{
		api::SetFileAttributes(Name,m_FileAttributes|FILE_ATTRIBUTE_ARCHIVE);
	}

	api::GetFindDataEx(Name, FileInfo);
	EditorGetFileAttributes(Name);

	if (m_editor->m_Flags.Check(Editor::FEDITOR_MODIFIED) || NewFile)
		m_editor->m_Flags.Set(Editor::FEDITOR_WASCHANGED);

	/* Этот кусок раскомметировать в том случае, если народ решит, что
	   для если файл был залочен и мы его переписали под други именем...
	   ...то "лочка" должна быть снята.
	*/

//  if(SaveAs)
//    Flags.Clear(FEDITOR_LOCKMODE);
	/* 28.12.2001 VVM
	  ! Проверить на успешную запись */
	if (RetCode==SAVEFILE_SUCCESS)
	{
		m_editor->TextChanged(false);
		m_editor->m_Flags.Set(Editor::FEDITOR_NEWUNDO);
	}

	if (GetDynamicallyBorn()) // принудительно сбросим Title // Flags.Check(FFILEEDIT_SAVETOSAVEAS) ????????
		strTitle.clear();

	Show();
	// ************************************
	m_Flags.Clear(FFILEEDIT_NEW);
	return RetCode;
}

int FileEditor::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	F4KeyOnly = false;
	if (!m_windowKeyBar->ProcessMouse(MouseEvent))
		if (!ProcessEditorInput(Global->WindowManager->GetLastInputRecord()))
			if (!m_editor->ProcessMouse(MouseEvent))
				return FALSE;

	return TRUE;
}


int FileEditor::GetTypeAndName(string &strType, string &strName)
{
	strType = MSG(MScreensEdit);
	strName = strFullFileName;
	return windowtype_editor;
}


void FileEditor::ShowConsoleTitle()
{
	string strEditorTitleFormat=Global->Opt->strEditorTitleFormat.Get();
	ReplaceStrings(strEditorTitleFormat, L"%Lng", MSG(MInEditor), true);
	ReplaceStrings(strEditorTitleFormat, L"%File", PointToName(strFileName), true);
	ConsoleTitle::SetFarTitle(strEditorTitleFormat);
	m_Flags.Clear(FFILEEDIT_REDRAWTITLE);
}

void FileEditor::SetScreenPosition()
{
	if (m_Flags.Check(FFILEEDIT_FULLSCREEN))
	{
		SetPosition(0,0,ScrX,ScrY);
	}
}

void FileEditor::OnDestroy()
{
	_OT(SysLog(L"[%p] FileEditor::OnDestroy()",this));

	if (Global->CtrlObject && !m_Flags.Check(FFILEEDIT_DISABLEHISTORY) && StrCmpI(strFileName.data(), MSG(MNewFileName)))
		Global->CtrlObject->ViewHistory->AddToHistory(strFullFileName, m_editor->m_Flags.Check(Editor::FEDITOR_LOCKMODE) ? HR_EDITOR_RO : HR_EDITOR);

	//AY: флаг оповещающий закрытие редактора.
	m_bClosing = true;

	int FEditEditorID=m_editor->EditorID;

	if (bEE_READ_Sent && Global->CtrlObject)
	{
		Global->CtrlObject->Plugins->ProcessEditorEvent(EE_CLOSE,nullptr,FEditEditorID);
	}
}

int FileEditor::GetCanLoseFocus(int DynamicMode) const
{
	if (DynamicMode)
	{
		if (m_editor->IsFileModified())
		{
			return FALSE;
		}
	}
	else
	{
		return m_CanLoseFocus;
	}

	return TRUE;
}

void FileEditor::SetLockEditor(BOOL LockMode)
{
	if (LockMode)
		m_editor->m_Flags.Set(Editor::FEDITOR_LOCKMODE);
	else
		m_editor->m_Flags.Clear(Editor::FEDITOR_LOCKMODE);
}

bool FileEditor::CanFastHide() const
{
	return (Global->Opt->AllCtrlAltShiftRule & CASR_EDITOR) != 0;
}

bool FileEditor::isTemporary() const
{
	return !GetDynamicallyBorn();
}

void FileEditor::ResizeConsole()
{
	m_editor->PrepareResizedConsole();
}

int FileEditor::ProcessEditorInput(const INPUT_RECORD& Rec)
{
	int RetCode;
	RetCode=Global->CtrlObject->Plugins->ProcessEditorInput(&Rec);
	return RetCode;
}

void FileEditor::SetPluginTitle(const string* PluginTitle)
{
	if (!PluginTitle)
		strPluginTitle.clear();
	else
		strPluginTitle = *PluginTitle;
}

BOOL FileEditor::SetFileName(const string& NewFileName)
{
	strFileName = NewFileName;

	if (strFileName != MSG(MNewFileName))
	{
		ConvertNameToFull(strFileName, strFullFileName);
		string strFilePath=strFullFileName;

		if (CutToSlash(strFilePath,1))
		{
			string strCurPath;

			if (api::GetCurrentDirectory(strCurPath))
			{
				DeleteEndSlash(strCurPath);

				if (!StrCmpI(strFilePath, strCurPath))
					strFileName=PointToName(strFullFileName);
			}
		}

		//Дабы избежать бардака, развернём слэшики...
		ReplaceSlashToBSlash(strFullFileName);
	}
	else
	{
		strFullFileName = strStartDir;
		AddEndSlash(strFullFileName);
		strFullFileName += strFileName;
	}

	return TRUE;
}

void FileEditor::SetTitle(const string* Title)
{
	strTitle = Title? *Title : L"";
}

void FileEditor::ChangeEditKeyBar()
{
	static const LNGID F8Labels[2][2] =
	{
		{MEditF8, MSingleEditAltF8},
		{MEditF8DOS, MSingleEditF8DOS},
	};
	m_windowKeyBar->Change(MSG(F8Labels[m_codepage == GetACP()][Global->OnlyEditorViewerUsed]), 7);
	m_windowKeyBar->Redraw();
}

string FileEditor::GetTitle() const
{
	string strLocalTitle;
	if (!strPluginTitle.empty())
		strLocalTitle = strPluginTitle;
	else
	{
		if (!strTitle.empty())
			strLocalTitle = strTitle;
		else
			strLocalTitle = strFullFileName;
	}

	return strLocalTitle;
}

void FileEditor::ShowStatus()
{
	if (m_editor->Locked() || !Global->Opt->EdOpt.ShowTitleBar)
		return;

	SetColor(COL_EDITORSTATUS);
	GotoXY(m_X1,m_Y1); //??
	string strLineStr;
	string strLocalTitle = GetTitle();
	int NameLength = (Global->Opt->ViewerEditorClock && m_Flags.Check(FFILEEDIT_FULLSCREEN)) ? 15:21;
	if (m_X2 > 80)
		NameLength += (m_X2-80);

	if (!strPluginTitle.empty() || !strTitle.empty())
		TruncPathStr(strLocalTitle, (ObjWidth()<NameLength?ObjWidth():NameLength));
	else
		TruncPathStr(strLocalTitle, NameLength);

	//предварительный расчет
	strLineStr = std::to_wstring(m_editor->NumLastLine) + L'/' + std::to_wstring(m_editor->NumLastLine);
	int SizeLineStr = (int)strLineStr.size();

	if (SizeLineStr > 12)
		NameLength -= (SizeLineStr-12);
	else
		SizeLineStr = 12;

	strLineStr = std::to_wstring(m_editor->NumLine + 1) + L'/' + std::to_wstring(m_editor->NumLastLine);
	string strAttr(AttrStr);
	FormatString FString;
	FString<<fmt::LeftAlign()<<fmt::MinWidth(NameLength)<<strLocalTitle<<L' '<<
		(m_editor->m_Flags.Check(Editor::FEDITOR_MODIFIED) ? L'*':L' ')<<
		(m_editor->m_Flags.Check(Editor::FEDITOR_LOCKMODE) ? L'-':L' ')<<
		(m_editor->m_Flags.Check(Editor::FEDITOR_PROCESSCTRLQ) ? L'"':L' ')<<
		fmt::MinWidth(5)<<m_codepage<<L' '<<fmt::MinWidth(3)<<MSG(MEditStatusLine)<<L' '<<
		fmt::ExactWidth(SizeLineStr)<<strLineStr<<L' '<<
		fmt::MinWidth(3)<<MSG(MEditStatusCol)<<L' '<<
		fmt::LeftAlign()<<fmt::MinWidth(4)<<m_editor->CurLine->GetTabCurPos()+1<<L' '<<
		fmt::MinWidth(2)<<MSG(MEditStatusChar)<<L' '<<
		fmt::LeftAlign()<<fmt::MinWidth(4)<<m_editor->CurLine->GetCurPos()+1<<L' '<<
		fmt::MinWidth(3)<<strAttr;

	int StatusWidth=ObjWidth() - ((Global->Opt->ViewerEditorClock && m_Flags.Check(FFILEEDIT_FULLSCREEN))?5:0);

	if (StatusWidth<0)
		StatusWidth=0;

	Global->FS << fmt::LeftAlign()<<fmt::ExactWidth(StatusWidth)<<FString;
	{
		const wchar_t *Str;
		intptr_t Length;
		m_editor->CurLine->GetBinaryString(&Str,nullptr,Length);
		int CurPos=m_editor->CurLine->GetCurPos();

		if (CurPos<Length)
		{
			GotoXY(m_X2-((Global->Opt->ViewerEditorClock && m_Flags.Check(FFILEEDIT_FULLSCREEN)) ? 14:8)-(!m_editor->EdOpt.CharCodeBase?3:0),m_Y1);
			SetColor(COL_EDITORSTATUS);
			/* $ 27.02.2001 SVS
			Показываем в зависимости от базы */
			switch(m_editor->EdOpt.CharCodeBase)
			{
			case 0:
				Global->FS << fmt::MinWidth(7) << fmt::FillChar(L'0') << fmt::Radix(8) << static_cast<UINT>(Str[CurPos]);
				break;
			case 2:
				Global->FS << fmt::MinWidth(4) << fmt::FillChar(L'0') << fmt::Radix(16) << static_cast<UINT>(Str[CurPos]) << L'h';
				break;
			case 1:
			default:
				Global->FS << fmt::MinWidth(5) << static_cast<UINT>(Str[CurPos]);
				break;
			}

			if (!IsUnicodeOrUtfCodePage(m_codepage))
			{
				char C=0;
				BOOL UsedDefaultChar=FALSE;
				WideCharToMultiByte(m_codepage,WC_NO_BEST_FIT_CHARS,&Str[CurPos],1,&C,1,0,&UsedDefaultChar);

				if (C && !UsedDefaultChar && static_cast<wchar_t>(C)!=Str[CurPos])
				{
					Global->FS << L"/";
					switch(m_editor->EdOpt.CharCodeBase)
					{
					case 0:
						Global->FS << fmt::MinWidth(4) << fmt::FillChar(L'0') << fmt::Radix(8) << static_cast<UINT>(C);
						break;
					case 2:
						Global->FS << fmt::MinWidth(2) << fmt::FillChar(L'0') << fmt::Radix(16) << static_cast<UINT>(C) << L'h';
						break;
					case 1:
					default:
						Global->FS << fmt::MinWidth(3) << static_cast<UINT>(C);
						break;
					}
				}
			}
		}
	}

	if (Global->Opt->ViewerEditorClock && m_Flags.Check(FFILEEDIT_FULLSCREEN))
		ShowTime(FALSE);
}

/* $ 13.02.2001
     Узнаем атрибуты файла и заодно сформируем готовую строку атрибутов для
     статуса.
*/
DWORD FileEditor::EditorGetFileAttributes(const string& Name)
{
	m_FileAttributes=api::GetFileAttributes(Name);
	int ind=0;

	if (m_FileAttributes!=INVALID_FILE_ATTRIBUTES)
	{
		if (m_FileAttributes&FILE_ATTRIBUTE_READONLY) AttrStr[ind++]=L'R';

		if (m_FileAttributes&FILE_ATTRIBUTE_SYSTEM) AttrStr[ind++]=L'S';

		if (m_FileAttributes&FILE_ATTRIBUTE_HIDDEN) AttrStr[ind++]=L'H';
	}

	AttrStr[ind]=0;
	return m_FileAttributes;
}

/* true - панель обовили
*/
bool FileEditor::UpdateFileList()
{
	Panel *ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
	const wchar_t *FileName = PointToName(strFullFileName);
	string strFilePath(strFullFileName), strPanelPath(ActivePanel->GetCurDir());
	strFilePath.resize(FileName - strFullFileName.data());
	AddEndSlash(strPanelPath);
	AddEndSlash(strFilePath);

	if (strPanelPath == strFilePath)
	{
		ActivePanel->Update(UPDATE_KEEP_SELECTION|UPDATE_DRAW_MESSAGE);
		return true;
	}

	return false;
}

void FileEditor::SetPluginData(const string* PluginData)
{
	FileEditor::strPluginData = PluginData? *PluginData : L"";
}

/* $ 14.06.2002 IS
   DeleteOnClose стал int:
     0 - не удалять ничего
     1 - удалять файл и каталог
     2 - удалять только файл
*/
void FileEditor::SetDeleteOnClose(int NewMode)
{
	m_Flags.Clear(FFILEEDIT_DELETEONCLOSE|FFILEEDIT_DELETEONLYFILEONCLOSE);

	if (NewMode==1)
		m_Flags.Set(FFILEEDIT_DELETEONCLOSE);
	else if (NewMode==2)
		m_Flags.Set(FFILEEDIT_DELETEONLYFILEONCLOSE);
}

void FileEditor::GetEditorOptions(Options::EditorOptions& EdOpt) const
{
	EdOpt = m_editor->EdOpt;
}

void FileEditor::SetEditorOptions(const Options::EditorOptions& EdOpt)
{
	m_editor->SetOptions(EdOpt);
}

void FileEditor::OnChangeFocus(int focus)
{
	window::OnChangeFocus(focus);
	int FEditEditorID=m_editor->EditorID;
	Global->CtrlObject->Plugins->ProcessEditorEvent(focus?EE_GOTFOCUS:EE_KILLFOCUS,nullptr,FEditEditorID);
}


intptr_t FileEditor::EditorControl(int Command, intptr_t Param1, void *Param2)
{
#if defined(SYSLOG_KEYMACRO)
	_KEYMACRO(CleverSysLog SL(L"FileEditor::EditorControl()"));

	if (Command == ECTL_READINPUT || Command == ECTL_PROCESSINPUT)
	{
		_KEYMACRO(SysLog(L"(Command=%s, Param2=[%d/0x%08X]) Macro.IsExecuting()=%d",_ECTL_ToName(Command),(int)((intptr_t)Param2),(int)((intptr_t)Param2),Global->CtrlObject->Macro.IsExecuting()));
	}

#else
	_ECTLLOG(CleverSysLog SL(L"FileEditor::EditorControl()"));
	_ECTLLOG(SysLog(L"(Command=%s, Param2=[%d/0x%08X])",_ECTL_ToName(Command),(int)Param2,Param2));
#endif

	if(m_editor->EditorControlLocked()) return FALSE;
	if (m_bClosing && (Command != ECTL_GETINFO) && (Command != ECTL_GETBOOKMARKS) && (Command!=ECTL_GETFILENAME))
		return FALSE;

	switch (Command)
	{
		case ECTL_GETFILENAME:
		{
			if (Param2&&(size_t)Param1>strFullFileName.size())
			{
				wcscpy(static_cast<LPWSTR>(Param2),strFullFileName.data());
			}

			return strFullFileName.size()+1;
		}
		case ECTL_GETBOOKMARKS:
		{
			auto ebm = static_cast<EditorBookmarks*>(Param2);
			if (!m_Flags.Check(FFILEEDIT_OPENFAILED) && CheckNullOrStructSize(ebm))
			{
				size_t size;
				if(Editor::InitSessionBookmarksForPlugin(ebm, m_editor->m_SavePos.size(), size))
				{
					for_each_cnt(CONST_RANGE(m_editor->m_SavePos, i, size_t index)
					{
						if (ebm->Line)
						{
							ebm->Line[index] = i.Line;
						}
						if (ebm->Cursor)
						{
							ebm->Cursor[index] = i.LinePos;
						}
						if (ebm->ScreenLine)
						{
							ebm->ScreenLine[index] = i.ScreenLine;
						}
						if (ebm->LeftPos)
						{
							ebm->LeftPos[index] = i.LeftPos;
						}
					});
				}
				return size;
			}
			return 0;
		}
		case ECTL_ADDSESSIONBOOKMARK:
		{
			m_editor->AddSessionBookmark();
			return TRUE;
		}
		case ECTL_PREVSESSIONBOOKMARK:
		{
			m_editor->TurnOffMarkingBlock();
			return m_editor->PrevSessionBookmark();
		}
		case ECTL_NEXTSESSIONBOOKMARK:
		{
			m_editor->TurnOffMarkingBlock();
			return m_editor->NextSessionBookmark();
		}
		case ECTL_CLEARSESSIONBOOKMARKS:
		{
			m_editor->ClearSessionBookmarks();
			return TRUE;
		}
		case ECTL_DELETESESSIONBOOKMARK:
		{
			auto i = m_editor->PointerToSessionBookmark((int) (intptr_t) Param2);
			return m_editor->DeleteSessionBookmark(i);
		}
		case ECTL_GETSESSIONBOOKMARKS:
		{
			return CheckNullOrStructSize((EditorBookmarks *)Param2)?m_editor->GetSessionBookmarksForPlugin((EditorBookmarks *)Param2):0;
		}
		case ECTL_GETTITLE:
		{
			string strLocalTitle = GetTitle();
			if (Param2&&(size_t)Param1>strLocalTitle.size())
			{
				wcscpy(static_cast<LPWSTR>(Param2),strLocalTitle.data());
			}

			return strLocalTitle.size()+1;
		}
		case ECTL_SETTITLE:
		{
			strPluginTitle = NullToEmpty(reinterpret_cast<const wchar_t*>(Param2));
			ShowStatus();
			Global->ScrBuf->Flush(); //???
			return TRUE;
		}
		case ECTL_REDRAW:
		{
			FileEditor::DisplayObject();
			Global->ScrBuf->Flush();
			return TRUE;
		}
		/*
			Функция установки Keybar Labels
			Param2 = nullptr - восстановить, пред. значение
			Param2 = -1   - обновить полосу (перерисовать)
			Param2 = KeyBarTitles
		*/
		case ECTL_SETKEYBAR:
		{
			FarSetKeyBarTitles *Kbt = (FarSetKeyBarTitles*)Param2;

			if (!Kbt)   //восстановить изначальное
				InitKeyBar();
			else
			{
				if ((intptr_t)Param2 != (intptr_t)-1) // не только перерисовать?
				{
					if(CheckStructSize(Kbt))
						m_windowKeyBar->Change(Kbt->Titles);
					else
						return FALSE;
				}

				m_windowKeyBar->Show();
			}

			return TRUE;
		}
		case ECTL_SAVEFILE:
		{
			string strName = strFullFileName;
			int EOL=0;
			uintptr_t codepage=m_codepage;

			EditorSaveFile *esf=(EditorSaveFile *)Param2;
			if (CheckStructSize(esf))
			{

				if (esf->FileName)
					strName=esf->FileName;

				if (esf->FileEOL)
				{
					if (!StrCmp(esf->FileEOL,DOS_EOL_fmt))
						EOL=1;
					else if (!StrCmp(esf->FileEOL,UNIX_EOL_fmt))
						EOL=2;
					else if (!StrCmp(esf->FileEOL,MAC_EOL_fmt))
						EOL=3;
					else if (!StrCmp(esf->FileEOL,WIN_EOL_fmt))
						EOL=4;
				}

				if (esf->CodePage != CP_DEFAULT)
					codepage=esf->CodePage;
			}

			{
				string strOldFullFileName = strFullFileName;

				if (SetFileName(strName))
				{
					if (StrCmpI(strFullFileName, strOldFullFileName))
					{
						if (!AskOverwrite(strName))
						{
							SetFileName(strOldFullFileName);
							return FALSE;
						}
					}

					m_Flags.Set(FFILEEDIT_SAVEWQUESTIONS);
					//всегда записываем в режиме save as - иначе не сменить кодировку и концы линий.
					return SaveFile(strName,FALSE,true,EOL,codepage,m_bAddSignature);
				}
			}

			return FALSE;
		}
		case ECTL_QUIT:
		{
			if (!this->bLoaded) // do not delete not created window
			{
				SetExitCode(XC_LOADING_INTERRUPTED);
			}
			else
			{
				Global->WindowManager->DeleteWindow(this);
				SetExitCode(XC_OPEN_ERROR); // что-то меня терзают смутные сомнения ...??? SAVEFILE_ERROR ???
			}
			return TRUE;
		}
		case ECTL_READINPUT:
		{
			if (Global->CtrlObject->Macro.IsRecording() == MACROSTATE_RECORDING || Global->CtrlObject->Macro.IsExecuting() == MACROSTATE_EXECUTING)
			{
//        return FALSE;
			}

			if (Param2)
			{
				INPUT_RECORD *rec=(INPUT_RECORD *)Param2;

				for (;;)
				{
					DWORD Key=GetInputRecord(rec);

					if ((!rec->EventType || rec->EventType == KEY_EVENT || rec->EventType == FARMACRO_KEY_EVENT) &&
					        ((Key >= KEY_MACRO_BASE && Key <= KEY_MACRO_ENDBASE) || (Key>=KEY_OP_BASE && Key <=KEY_OP_ENDBASE))) // исключаем MACRO
						ReProcessKey(Key);
					else
						break;
				}

				//if(Key==KEY_CONSOLE_BUFFER_RESIZE) //????
				//  Show();                          //????
#if defined(SYSLOG_KEYMACRO)

				if (rec->EventType == KEY_EVENT)
				{
					SysLog(L"ECTL_READINPUT={%s,{%d,%d,Vk=0x%04X,0x%08X}}",
					       (rec->EventType == FARMACRO_KEY_EVENT?L"FARMACRO_KEY_EVENT":L"KEY_EVENT"),
					       rec->Event.KeyEvent.bKeyDown,
					       rec->Event.KeyEvent.wRepeatCount,
					       rec->Event.KeyEvent.wVirtualKeyCode,
					       rec->Event.KeyEvent.dwControlKeyState);
				}

#endif
				return TRUE;
			}

			return FALSE;
		}
		case ECTL_PROCESSINPUT:
		{
			if (Param2)
			{
				auto& rec = *reinterpret_cast<const INPUT_RECORD*>(Param2);

				if (ProcessEditorInput(rec))
					return TRUE;

				if (rec.EventType==MOUSE_EVENT)
					ProcessMouse(&rec.Event.MouseEvent);
				else
				{
#if defined(SYSLOG_KEYMACRO)

					if (!rec.EventType || rec.EventType == KEY_EVENT || rec.EventType == FARMACRO_KEY_EVENT)
					{
						SysLog(L"ECTL_PROCESSINPUT={%s,{%d,%d,Vk=0x%04X,0x%08X}}",
						       (rec.EventType == FARMACRO_KEY_EVENT?L"FARMACRO_KEY_EVENT":L"KEY_EVENT"),
						       rec.Event.KeyEvent.bKeyDown,
						       rec.Event.KeyEvent.wRepeatCount,
						       rec.Event.KeyEvent.wVirtualKeyCode,
						       rec.Event.KeyEvent.dwControlKeyState);
					}

#endif
					int Key=ShieldCalcKeyCode(&rec,FALSE);
					ReProcessKey(Key);
				}

				return TRUE;
			}

			return FALSE;
		}
		case ECTL_SETPARAM:
		{
			EditorSetParameter *espar=(EditorSetParameter *)Param2;
			if (CheckStructSize(espar))
			{
				if (ESPT_SETBOM==espar->Type)
				{
					if(IsUnicodeOrUtfCodePage(m_codepage))
					{
						m_bAddSignature=espar->iParam != 0;
						return TRUE;
					}
					return FALSE;
				}
			}
			break;
		}
	}

	int result=m_editor->EditorControl(Command,Param1,Param2);
	if (result&&Param2&&ECTL_GETINFO==Command)
	{
		EditorInfo *Info=(EditorInfo *)Param2;
		if (CheckStructSize(Info)&&m_bAddSignature)
			Info->Options|=EOPT_BOM;
	}
	return result;
}

bool FileEditor::LoadFromCache(EditorPosCache &pc)
{
	string strCacheName;

	if (*GetPluginData())
	{
		strCacheName=GetPluginData();
		strCacheName+=PointToName(strFullFileName);
	}
	else
	{
		strCacheName+=strFullFileName;
		ReplaceSlashToBSlash(strCacheName);
	}

	pc.Clear();

	if (FilePositionCache::GetPosition(strCacheName, pc))
		return true;

	return false;
}

void FileEditor::SaveToCache()
{
	EditorPosCache pc;
	m_editor->GetCacheParams(pc);
	string strCacheName=strPluginData.empty()?strFullFileName:strPluginData+PointToName(strFullFileName);

	if (!m_Flags.Check(FFILEEDIT_OPENFAILED))   //????
	{
		pc.CodePage = (m_Flags.Check(FFILEEDIT_CODEPAGECHANGEDBYUSER) && !BadConversion)?m_codepage:0;

		FilePositionCache::AddPosition(strCacheName, pc);
	}
}

bool FileEditor::SetCodePage(uintptr_t codepage)
{
	if (codepage == m_codepage || !m_editor)
		return false;

	int x, y;
	if (!m_editor->TryCodePage(codepage, x, y))
	{
		int ret = Message(MSG_WARNING, 3, MSG(MWarning),
			MSG(MEditorSwitchCPWarn1), MSG(MEditorSwitchCPWarn2), MSG(MEditorSwitchCPConfirm),
			MSG(MCancel), MSG(MEditorSaveCPWarnShow), MSG(MOk));

		if (ret < 2) // not confirmed
		{
			if (ret == 1)
			{
				m_editor->GoToLine(y);
				m_editor->CurLine->SetCurPos(x);
				Show();
			}
			return false;
		}
	}
	m_codepage = codepage;
	BadConversion = !m_editor->SetCodePage(m_codepage, &m_bAddSignature);
	return true;
}

bool FileEditor::AskOverwrite(const string& FileName)
{
	bool result=true;
	DWORD FNAttr=api::GetFileAttributes(FileName);

	if (FNAttr!=INVALID_FILE_ATTRIBUTES)
	{
		const wchar_t* const Items[] = {FileName.data(),MSG(MEditExists),MSG(MEditOvr),MSG(MYes),MSG(MNo)};
		if (Message(MSG_WARNING,2,MSG(MEditTitle),Items, ARRAYSIZE(Items), nullptr, nullptr, &EditorAskOverwriteId))
		{
			result=false;
		}
		else
		{
			m_Flags.Set(FFILEEDIT_SAVEWQUESTIONS);
		}
	}

	return result;
}

uintptr_t FileEditor::GetDefaultCodePage()
{
	intptr_t cp = Global->Opt->EdOpt.DefaultCodePage;
	if (cp < 0 || !Codepages().IsCodePageSupported(cp))
		cp = GetACP();
	return cp;
}
