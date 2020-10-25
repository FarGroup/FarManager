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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "fileedit.hpp"

// Internal:
#include "keyboard.hpp"
#include "encoding.hpp"
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
#include "manager.hpp"
#include "namelist.hpp"
#include "history.hpp"
#include "cmdline.hpp"
#include "scrbuf.hpp"
#include "savescr.hpp"
#include "filestr.hpp"
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "taskbar.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "config.hpp"
#include "delete.hpp"
#include "datetime.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "strmix.hpp"
#include "exitcode.hpp"
#include "constitle.hpp"
#include "wakeful.hpp"
#include "uuids.far.dialogs.hpp"
#include "stddlg.hpp"
#include "plugins.hpp"
#include "lang.hpp"
#include "keybar.hpp"
#include "string_utils.hpp"
#include "cvtname.hpp"
#include "global.hpp"
#include "file_io.hpp"

// Platform:
#include "platform.env.hpp"
#include "platform.fs.hpp"

// Common:
#include "common/view/enumerate.hpp"
// External:
#include "format.hpp"

//----------------------------------------------------------------------------

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

	ID_OE_COUNT
};


static intptr_t hndOpenEditor(Dialog* Dlg, intptr_t msg, intptr_t param1, void* param2)
{
	if (msg == DN_INITDIALOG)
	{
		const auto codepage = *static_cast<uintptr_t*>(param2);
		codepages::instance().FillCodePagesList(Dlg, ID_OE_CODEPAGE, codepage, true, false, true, false, false);
	}

	if (msg == DN_CLOSE)
	{
		if (param1 == ID_OE_OK)
		{
			const auto param = reinterpret_cast<uintptr_t*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
			FarListPos pos={sizeof(FarListPos)};
			Dlg->SendMessage(DM_LISTGETCURPOS, ID_OE_CODEPAGE, &pos);
			*param = Dlg->GetListItemSimpleUserData(ID_OE_CODEPAGE, pos.SelectPos);
			return TRUE;
		}
	}

	return Dlg->DefProc(msg, param1, param2);
}

bool dlgOpenEditor(string &strFileName, uintptr_t &codepage)
{
	auto EditDlg = MakeDialogItems<ID_OE_COUNT>(
	{
		{ DI_DOUBLEBOX, {{3,  1}, {72, 8}}, DIF_NONE, msg(lng::MEditTitle), },
		{ DI_TEXT,      {{5,  2}, {0 , 2}}, DIF_NONE, msg(lng::MEditOpenCreateLabel), },
		{ DI_EDIT,      {{5,  3}, {70, 3}}, DIF_FOCUS | DIF_HISTORY | DIF_USELASTHISTORY | DIF_EDITEXPAND | DIF_EDITPATH, },
		{ DI_TEXT,      {{-1, 4}, {0,  4}}, DIF_SEPARATOR, {} },
		{ DI_TEXT,      {{5,  5}, {0,  5}}, DIF_NONE, msg(lng::MEditCodePage), },
		{ DI_COMBOBOX,  {{25, 5}, {70, 5}}, DIF_DROPDOWNLIST | DIF_LISTWRAPMODE | DIF_LISTAUTOHIGHLIGHT, },
		{ DI_TEXT,      {{-1, 6}, {0,  6}}, DIF_SEPARATOR, {} },
		{ DI_BUTTON,    {{0,  7}, {0,  7}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MOk), },
		{ DI_BUTTON,    {{0,  7}, {0,  7}}, DIF_CENTERGROUP, msg(lng::MCancel), },
	});

	EditDlg[ID_OE_FILENAME].strHistory = L"NewEdit"sv;

	const auto Dlg = Dialog::create(EditDlg, hndOpenEditor, &codepage);
	Dlg->SetPosition({ -1, -1, 76, 10 });
	Dlg->SetHelp(L"FileOpenCreate"sv);
	Dlg->SetId(FileOpenCreateId);
	Dlg->Process();

	if (Dlg->GetExitCode() == ID_OE_OK)
	{
		strFileName = EditDlg[ID_OE_FILENAME].strData;
		return true;
	}

	return false;
}

static bool dlgBadEditorCodepage(uintptr_t& codepage)
{
	DialogBuilder Builder(lng::MWarning);

	Builder.AddText(lng::MEditorLoadCPWarn1)->Flags = DIF_CENTERTEXT;
	Builder.AddText(lng::MEditorLoadCPWarn2)->Flags = DIF_CENTERTEXT;
	Builder.AddText(lng::MEditorSaveNotRecommended)->Flags = DIF_CENTERTEXT;
	Builder.AddSeparator();

	IntOption cp_val;
	cp_val = codepage;

	std::vector<DialogBuilderListItem> Items;
	codepages::instance().FillCodePagesList(Items, true, false, true, false, false);

	Builder.AddComboBox(cp_val, 46, Items);
	Builder.AddOKCancel();
	Builder.SetDialogMode(DMODE_WARNINGSTYLE);
	Builder.SetId(BadEditorCodePageId);

	if (!Builder.ShowDialog())
		return false;

	codepage = cp_val;
	return true;
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
	ID_SF_WINDOWS,
	ID_SF_UNIX,
	ID_SF_MAC,
	ID_SF_SEPARATOR3,
	ID_SF_OK,
	ID_SF_CANCEL,

	ID_SF_COUNT
};

static intptr_t hndSaveFileAs(Dialog* Dlg, intptr_t msg, intptr_t param1, void* param2)
{
	static uintptr_t CurrentCodepage = 0;

	switch (msg)
	{
		case DN_INITDIALOG:
		{
			CurrentCodepage = *reinterpret_cast<uintptr_t*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
			codepages::instance().FillCodePagesList(Dlg, ID_SF_CODEPAGE, CurrentCodepage, false, false, false, false, false);
			break;
		}
		case DN_CLOSE:
		{
			if (param1 == ID_SF_OK)
			{
				const auto CodepagePtr = reinterpret_cast<uintptr_t*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
				FarListPos pos={sizeof(FarListPos)};
				Dlg->SendMessage(DM_LISTGETCURPOS, ID_SF_CODEPAGE, &pos);
				*CodepagePtr = Dlg->GetListItemSimpleUserData(ID_SF_CODEPAGE, pos.SelectPos);
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
				const uintptr_t cp = Dlg->GetListItemSimpleUserData(ID_SF_CODEPAGE, pos.SelectPos);
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



static bool dlgSaveFileAs(string &strFileName, eol& Eol, uintptr_t &codepage, bool &AddSignature)
{
	const auto ucp = IsUnicodeOrUtfCodePage(codepage);

	auto EditDlg = MakeDialogItems<ID_SF_COUNT>(
	{
		{ DI_DOUBLEBOX,    {{3,  1 }, {72, 15}}, DIF_NONE, msg(lng::MEditTitle), },
		{ DI_TEXT,         {{5,  2 }, {0,  2 }}, DIF_NONE, msg(lng::MEditSaveAs), },
		{ DI_EDIT,         {{5,  3 }, {70, 3 }}, DIF_FOCUS | DIF_HISTORY | DIF_EDITEXPAND | DIF_EDITPATH, },
		{ DI_TEXT,         {{-1, 4 }, {0,  4 }}, DIF_SEPARATOR, },
		{ DI_TEXT,         {{5,  5 }, {0,  5 }}, DIF_NONE, msg(lng::MEditCodePage), },
		{ DI_COMBOBOX,     {{25, 5 }, {70, 5 }}, DIF_DROPDOWNLIST | DIF_LISTWRAPMODE | DIF_LISTAUTOHIGHLIGHT, },
		{ DI_CHECKBOX,     {{5,  6 }, {0,  6 }}, ucp ? DIF_NONE : DIF_DISABLE,msg(lng::MEditAddSignature), },
		{ DI_TEXT,         {{-1, 7 }, {0,  7 }}, DIF_SEPARATOR, },
		{ DI_TEXT,         {{5,  8 }, {0,  8 }}, DIF_NONE, msg(lng::MEditSaveAsFormatTitle), },
		{ DI_RADIOBUTTON,  {{5,  9 }, {0,  9 }}, DIF_GROUP, msg(lng::MEditSaveOriginal), },
		{ DI_RADIOBUTTON,  {{5,  10}, {0,  10}}, DIF_NONE, msg(lng::MEditSaveDOS), },
		{ DI_RADIOBUTTON,  {{5,  11}, {0,  11}}, DIF_NONE, msg(lng::MEditSaveUnix), },
		{ DI_RADIOBUTTON,  {{5,  12}, {0,  12}}, DIF_NONE, msg(lng::MEditSaveMac), },
		{ DI_TEXT,         {{-1, 13}, {0,  13}}, DIF_SEPARATOR, },
		{ DI_BUTTON,       {{0,  14}, {0,  14}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MEditorSave), },
		{ DI_BUTTON,       {{0,  14}, {0,  14}}, DIF_CENTERGROUP, msg(lng::MCancel), },
	});

	EditDlg[ID_SF_FILENAME].strHistory = L"NewEdit"sv;
	EditDlg[ID_SF_FILENAME].strData = (/*Flags.Check(FFILEEDIT_SAVETOSAVEAS)?strFullFileName:strFileName*/strFileName);
	EditDlg[ID_SF_SIGNATURE].Selected = AddSignature;

	if (const auto pos = EditDlg[ID_SF_FILENAME].strData.find(msg(lng::MNewFileName)); pos != string::npos)
		EditDlg[ID_SF_FILENAME].strData.resize(pos);

	const auto EolToIndex = [&]()
	{
		if (Eol == eol::win)
			return 1;
		if (Eol == eol::unix)
			return 2;
		if (Eol == eol::mac)
			return 3;
		return 0;
	};

	EditDlg[ID_SF_DONOTCHANGE + EolToIndex()].Selected = TRUE;
	const auto Dlg = Dialog::create(EditDlg, hndSaveFileAs, &codepage);
	Dlg->SetPosition({ -1, -1, 76, 17 });
	Dlg->SetHelp(L"FileSaveAs"sv);
	Dlg->SetId(FileSaveAsId);
	Dlg->Process();

	if ((Dlg->GetExitCode() == ID_SF_OK) && !EditDlg[ID_SF_FILENAME].strData.empty())
	{
		strFileName = EditDlg[ID_SF_FILENAME].strData;
		AddSignature=EditDlg[ID_SF_SIGNATURE].Selected!=0;

		if (EditDlg[ID_SF_DONOTCHANGE].Selected)
			Eol = eol::none;
		else if (EditDlg[ID_SF_WINDOWS].Selected)
			Eol = eol::win;
		else if (EditDlg[ID_SF_UNIX].Selected)
			Eol = eol::unix;
		else if (EditDlg[ID_SF_MAC].Selected)
			Eol = eol::mac;

		return true;
	}

	return false;
}

fileeditor_ptr FileEditor::create(const string_view Name, uintptr_t codepage, DWORD InitFlags, int StartLine, int StartChar, const string* PluginData, EDITOR_FLAGS OpenModeExstFile)
{
	auto FileEditorPtr = std::make_shared<FileEditor>(private_tag());
	FileEditorPtr->ScreenObjectWithShadow::SetPosition({ 0, 0, ScrX, ScrY });
	FileEditorPtr->m_Flags.Set(InitFlags);
	FileEditorPtr->m_Flags.Set(FFILEEDIT_FULLSCREEN);
	FileEditorPtr->Init(Name, codepage, nullptr, StartLine, StartChar, PluginData, FALSE, nullptr, OpenModeExstFile);
	return FileEditorPtr;
}

fileeditor_ptr FileEditor::create(const string_view Name, uintptr_t codepage, DWORD InitFlags, int StartLine, int StartChar, const string* Title, rectangle Position, int DeleteOnClose, const window_ptr& Update, EDITOR_FLAGS OpenModeExstFile)
{
	auto FileEditorPtr = std::make_shared<FileEditor>(private_tag());
	FileEditorPtr->m_Flags.Set(InitFlags);

	// BUGBUG WHY ALL THIS?
	if (Position.left < 0)
		Position.left = 0;

	if (Position.right < 0 || Position.right > ScrX)
		Position.right = ScrX;

	if (Position.top < 0)
		Position.top = 0;

	if (Position.bottom < 0 || Position.bottom > ScrY)
		Position.bottom = ScrY;

	if (Position.left > Position.right)
	{
		Position.left = 0;
		Position.right = ScrX;
	}

	if (Position.top > Position.bottom)
	{
		Position.top = 0;
		Position.bottom = ScrY;
	}

	FileEditorPtr->SetPosition(Position);
	FileEditorPtr->m_Flags.Change(FFILEEDIT_FULLSCREEN, (!Position.left && !Position.top && Position.right == ScrX && Position.bottom == ScrY));
	string EmptyTitle;
	FileEditorPtr->Init(Name, codepage, Title, StartLine, StartChar, &EmptyTitle, DeleteOnClose, Update, OpenModeExstFile);
	return FileEditorPtr;
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
				(void)os::fs::set_file_attributes(strFullFileName,FILE_ATTRIBUTE_NORMAL); // BUGBUG
				(void)os::fs::delete_file(strFullFileName); //BUGBUG
			}
		}
	}
}

void FileEditor::Init(
    const string_view Name,
    uintptr_t codepage,
    const string* Title,
    int StartLine,
    int StartChar,
    const string* PluginData,
    int DeleteOnClose,
    const window_ptr& Update,
    EDITOR_FLAGS OpenModeExstFile
)
{
	m_windowKeyBar = std::make_unique<KeyBar>(shared_from_this());

	const auto BlankFileName = Name == msg(lng::MNewFileName) || Name.empty();
	bEE_READ_Sent = false;
	bLoaded = false;
	m_bAddSignature = false;
	m_editor = std::make_unique<Editor>(shared_from_this(), codepage);

	m_codepage = codepage;
	*AttrStr=0;
	m_FileAttributes=INVALID_FILE_ATTRIBUTES;
	SetTitle(Title);
	// $ 17.08.2001 KM - Добавлено для поиска по AltF7. При редактировании найденного файла из архива для клавиши F2 сделать вызов ShiftF2.
	m_Flags.Change(FFILEEDIT_SAVETOSAVEAS, BlankFileName);

	if (BlankFileName && !m_Flags.Check(FFILEEDIT_CANNEWFILE))
	{
		SetExitCode(XC_OPEN_ERROR);
		return;
	}

	SetPluginData(PluginData);
	m_editor->SetHostFileEditor(this);
	SetCanLoseFocus(m_Flags.Check(FFILEEDIT_ENABLEF6));
	strStartDir = os::fs::GetCurrentDirectory();

	if (!SetFileName(Name))
	{
		SetExitCode(XC_OPEN_ERROR);
		return;
	}

	{
		if (auto EditorWindow = Global->WindowManager->FindWindowByFile(windowtype_editor, strFullFileName))
		{
			int SwitchTo=FALSE;

			if (!EditorWindow->GetCanLoseFocus(true) || Global->Opt->Confirm.AllowReedit)
			{
				int Result = XC_EXISTS;
				if (OpenModeExstFile == EF_OPENMODE_QUERY)
				{
					int MsgCode;
					if (m_Flags.Check(FFILEEDIT_ENABLEF6))
					{
						MsgCode = Message(0,
							msg(lng::MEditTitle),
							{
								strFullFileName,
								msg(lng::MAskReload)
							},
							{ lng::MCurrent, lng::MNewOpen, lng::MReload, lng::MCancel },
							L"EditorReload"sv, &EditorReloadId);
					}
					else
					{
						MsgCode = Message(0,
							msg(lng::MEditTitle),
							{
								strFullFileName,
								msg(lng::MAskReload)
							},
							{ lng::MNewOpen, lng::MCancel },
							L"EditorReload"sv, &EditorReloadModalId);

						if (MsgCode == Message::first_button)
							MsgCode = Message::second_button;
					}

					switch (MsgCode)
					{
					case Message::first_button:
						Result = XC_EXISTS;
						break;

					case Message::second_button:
						Result = XC_OPEN_NEWINSTANCE;
						break;

					case Message::third_button:
						Result = XC_RELOAD;
						break;

					default:
						SetExitCode(XC_LOADING_INTERRUPTED);
						return;
					}
				}
				else
				{
					if (m_Flags.Check(FFILEEDIT_ENABLEF6))
					{
						switch (OpenModeExstFile)
						{
						case EF_OPENMODE_USEEXISTING:
							Result = XC_EXISTS;
							break;

						case EF_OPENMODE_NEWIFOPEN:
							Result = XC_OPEN_NEWINSTANCE;
							break;

						case EF_OPENMODE_RELOADIFOPEN:
							Result = XC_RELOAD;
							break;

						default:
							SetExitCode(XC_EXISTS);
							return;
						}
					}
					else
					{
						switch (OpenModeExstFile)
						{
						case EF_OPENMODE_NEWIFOPEN:
							Result = XC_OPEN_NEWINSTANCE;
							break;
						}
					}
				}

				switch (Result)
				{
				case XC_EXISTS:
					SwitchTo=TRUE;
					SetExitCode(Result); // ???
					break;

				case XC_OPEN_NEWINSTANCE:
					SetExitCode(Result); // ???
					break;

				case XC_RELOAD:
					{
						//файл могли уже закрыть. например макросом в диалоговой процедуре предыдущего Message.
						EditorWindow = Global->WindowManager->FindWindowByFile(windowtype_editor, strFullFileName);
						if (EditorWindow)
						{
							EditorWindow->SetFlags(FFILEEDIT_DISABLESAVEPOS);
							Global->WindowManager->DeleteWindow(EditorWindow);
						}
						SetExitCode(Result); // -2 ???
					}
					break;
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
	const os::fs::file_status FileStatus(Name);

	/* $ 05.06.2001 IS
	   + посылаем подальше всех, кто пытается отредактировать каталог
	*/
	if (os::fs::is_directory(FileStatus))
	{
		Message(MSG_WARNING,
			msg(lng::MEditTitle),
			{
				msg(lng::MEditCanNotEditDirectory)
			},
			{ lng::MOk },
			{}, &EditorCanNotEditDirectoryId);
		SetExitCode(XC_OPEN_ERROR);
		return;
	}

	if (m_editor->EdOpt.ReadOnlyLock & 1_bit &&
		FileStatus.check(FILE_ATTRIBUTE_READONLY |
		/*  Hidden=0x2 System=0x4 - располагаются во 2-м полубайте,
		    поэтому применяем маску 0110.0000 и
		    сдвигаем на свое место => 0000.0110 и получаем
		    те самые нужные атрибуты */
			((m_editor->EdOpt.ReadOnlyLock & 0b0110'0000) >> 4)
		)
	)
	{
		if (Message(MSG_WARNING,
			msg(lng::MEditTitle),
			{
				string(Name),
				msg(lng::MEditRSH),
				msg(lng::MEditROOpen)
			},
			{ lng::MYes, lng::MNo },
			{}, &EditorOpenRSHId) != Message::first_button)
		{
			SetExitCode(XC_OPEN_ERROR);
			return;
		}
	}

	m_editor->SetPosition({ m_Where.left, m_Where.top + (IsTitleBarVisible()? 1 : 0), m_Where.right, m_Where.bottom - (IsKeyBarVisible()? 1 : 0) });
	m_editor->SetStartPos(StartLine,StartChar);
	SetDeleteOnClose(DeleteOnClose);
	int UserBreak;

	/* $ 06.07.2001 IS
	   При создании файла с нуля так же посылаем плагинам событие EE_READ, дабы
	   не нарушать однообразие.
	*/
	if (!os::fs::exists(FileStatus))
		m_Flags.Set(FFILEEDIT_NEW);

	if (BlankFileName && m_Flags.Check(FFILEEDIT_CANNEWFILE))
		m_Flags.Set(FFILEEDIT_NEW);

	if (m_Flags.Check(FFILEEDIT_NEW))
		m_bAddSignature = Global->Opt->EdOpt.AddUnicodeBOM;

	if (m_Flags.Check(FFILEEDIT_LOCKED))
		m_editor->m_Flags.Set(Editor::FEDITOR_LOCKMODE);

	error_state_ex ErrorState;
	while (!LoadFile(strFullFileName,UserBreak, ErrorState))
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
				if(OperationFailed(ErrorState, strFullFileName, lng::MEditTitle, msg(lng::MEditCannotOpen), false) == operation::retry)
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
			//Global->CtrlObject->Cp()->Redraw(); //AY: вроде как не надо, делает проблемы с прорисовкой если в редакторе из истории попытаться выбрать несуществующий файл

			// если прервали загрузку, то фреймы нужно проапдейтить, чтобы предыдущие месаги не оставались на экране
			if (!Global->Opt->Confirm.Esc && UserBreak && GetExitCode() == XC_LOADING_INTERRUPTED)
				Global->WindowManager->RefreshWindow();

			return;
		}

		if (m_codepage==CP_DEFAULT || m_codepage == CP_REDETECT)
			m_codepage = GetDefaultCodePage();

		m_editor->SetCodePage(m_codepage, nullptr, false);
		break;
	}

	if (GetExitCode() == XC_LOADING_INTERRUPTED || GetExitCode() == XC_OPEN_ERROR)
		return;

	InitKeyBar();
	// Note: bottom - bottom
	m_windowKeyBar->SetPosition({ m_Where.left, m_Where.bottom, m_Where.right, m_Where.bottom });


	if (IsKeyBarVisible())
	{
		m_windowKeyBar->Show();
	}
	else
	{
		m_windowKeyBar->Hide();
	}

	SetMacroMode(MACROAREA_EDITOR);

	F4KeyOnly=true;
	bLoaded = true;

	if (m_Flags.Check(FFILEEDIT_ENABLEF6))
	{
		if (Update) Global->WindowManager->ReplaceWindow(Update, shared_from_this());
		else Global->WindowManager->InsertWindow(shared_from_this());
	}
	else
	{
		if (Update) Global->WindowManager->DeleteWindow(Update);
		Global->WindowManager->ExecuteWindow(shared_from_this());
	}
	Global->WindowManager->CallbackWindow([this](){ ReadEvent(); });
}

void FileEditor::ReadEvent()
{
	Global->CtrlObject->Plugins->ProcessEditorEvent(EE_READ, nullptr, m_editor.get());
	bEE_READ_Sent = true;
	Global->WindowManager->RefreshWindow(); //в EE_READ поменялась позиция курсора или размер табуляции.
}

void FileEditor::InitKeyBar()
{
	auto& Keybar = *m_windowKeyBar;

	Keybar.SetLabels(lng::MEditF1);

	if (Global->OnlyEditorViewerUsed)
	{
		Keybar[KBL_SHIFT][F4].clear();
		Keybar[KBL_CTRL][F10].clear();
	}

	if (!GetCanLoseFocus())
	{
		Keybar[KBL_MAIN][F12].clear();
		Keybar[KBL_ALT][F11].clear();
		Keybar[KBL_SHIFT][F4].clear();
	}

	if (m_Flags.Check(FFILEEDIT_SAVETOSAVEAS))
		Keybar[KBL_MAIN][F2] = msg(lng::MEditShiftF2);

	if (!m_Flags.Check(FFILEEDIT_ENABLEF6))
		Keybar[KBL_MAIN][F6].clear();

	Keybar[KBL_MAIN][F8] = f8cps.NextCPname(m_codepage);

	Keybar.SetCustomLabels(KBA_EDITOR);
}

void FileEditor::SetNamesList(NamesList& Names)
{
	EditNamesList = std::move(Names);
}

void FileEditor::Show()
{
	if (m_Flags.Check(FFILEEDIT_FULLSCREEN))
	{
		if (IsKeyBarVisible())
		{
			m_windowKeyBar->SetPosition({ 0, ScrY, ScrX, ScrY });
		}
		ScreenObjectWithShadow::SetPosition({ 0, 0, ScrX, ScrY });
	}
	if (IsKeyBarVisible())
	{
		m_windowKeyBar->Redraw();
	}
	m_editor->SetPosition({ m_Where.left, m_Where.top + (IsTitleBarVisible()? 1 : 0), m_Where.right, m_Where.bottom - (IsKeyBarVisible()? 1 : 0) });
	ScreenObjectWithShadow::Show();
}


void FileEditor::DisplayObject()
{
	if (!m_bClosing)
	{
		m_editor->Show();
	}
}

long long FileEditor::VMProcess(int OpCode, void* vParam, long long iParam)
{
	if (OpCode == MCODE_V_EDITORSTATE)
	{
		DWORD MacroEditState = 0;
		MacroEditState |= m_Flags.Check(FFILEEDIT_NEW)?                                   0_bit : 0;
		MacroEditState |= m_Flags.Check(FFILEEDIT_ENABLEF6)?                              1_bit : 0;
		MacroEditState |= m_Flags.Check(FFILEEDIT_DELETEONCLOSE)?                         2_bit : 0;
		MacroEditState |= m_editor->m_Flags.Check(Editor::FEDITOR_MODIFIED)?              3_bit : 0;
		MacroEditState |= m_editor->IsStreamSelection()?                                  4_bit : 0;
		MacroEditState |= m_editor->IsVerticalSelection()?                                5_bit : 0;
		MacroEditState |= m_editor->m_Flags.Check(Editor::FEDITOR_WASCHANGED)?            6_bit : 0;
		MacroEditState |= m_editor->m_Flags.Check(Editor::FEDITOR_OVERTYPE)?              7_bit : 0;
		MacroEditState |= m_editor->m_Flags.Check(Editor::FEDITOR_CURPOSCHANGEDBYPLUGIN)? 8_bit : 0;
		MacroEditState |= m_editor->m_Flags.Check(Editor::FEDITOR_LOCKMODE)?              9_bit : 0;
		MacroEditState |= m_editor->EdOpt.PersistentBlocks?                               10_bit : 0;
		MacroEditState |= !GetCanLoseFocus()?                                             11_bit : 0;
		MacroEditState |= Global->OnlyEditorViewerUsed ?                                  27_bit | 11_bit : 0;
		return MacroEditState;
	}

	if (OpCode == MCODE_V_EDITORCURPOS)
		return m_editor->m_it_CurLine->GetTabCurPos()+1;

	if (OpCode == MCODE_V_EDITORCURLINE)
		return m_editor->m_it_CurLine.Number() + 1;

	if (OpCode == MCODE_V_ITEMCOUNT || OpCode == MCODE_V_EDITORLINES)
		return m_editor->Lines.size();

	if (OpCode == MCODE_F_KEYBAR_SHOW)
	{
		int PrevMode=IsKeyBarVisible()?2:1;
		switch (iParam)
		{
			case 0:
				break;
			case 1:
				Global->Opt->EdOpt.ShowKeyBar = true;
				m_windowKeyBar->Show();
				Show();
				break;
			case 2:
				Global->Opt->EdOpt.ShowKeyBar = false;
				m_windowKeyBar->Hide();
				Show();
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


bool FileEditor::ProcessKey(const Manager::Key& Key)
{
	return ReProcessKey(Key, false);
}

bool FileEditor::ReProcessKey(const Manager::Key& Key, bool CalledFromControl)
{
	const auto LocalKey = Key();
	if (none_of(LocalKey, KEY_F4, KEY_IDLE))
		F4KeyOnly=false;

	if (m_Flags.Check(FFILEEDIT_REDRAWTITLE) && ((LocalKey & 0x00ffffff) < KEY_END_FKEY || IsInternalKeyReal(LocalKey & 0x00ffffff)))
		ShowConsoleTitle();

	// Все остальные необработанные клавиши пустим далее
	/* $ 28.04.2001 DJ
	   не передаем KEY_MACRO* плагину - поскольку ReadRec в этом случае
	   никак не соответствует обрабатываемой клавише, возникают разномастные
	   глюки
	*/
	if (in_closed_range(KEY_MACRO_BASE, LocalKey, KEY_MACRO_ENDBASE) || in_closed_range(KEY_OP_BASE, LocalKey, KEY_OP_ENDBASE)) // исключаем MACRO
	{
		// ; //
	}

	switch (LocalKey)
	{
		case KEY_F6:
		{
			if (m_Flags.Check(FFILEEDIT_ENABLEF6))
			{
				int FirstSave=1;
				auto cp = m_codepage;

				// проверка на "а может это говно удалили уже?"
				// возможно здесь она и не нужна!
				// хотя, раз уж были изменения, то
				if (m_editor->IsFileChanged() && // в текущем сеансе были изменения?
				        !os::fs::exists(strFullFileName))
				{
					switch (Message(MSG_WARNING,
						msg(lng::MEditTitle),
						{
							msg(lng::MEditSavedChangedNonFile),
							msg(lng::MEditSavedChangedNonFile2)
						},
						{ lng::MHYes, lng::MHNo },
						{}, &EditorSaveF6DeletedId))
					{
						case 0:

							if (ProcessKey(Manager::Key(KEY_F2)))
							{
								FirstSave=0;
								break;
							}
							[[fallthrough]];

						default:
							return false;
					}
				}

				if (!FirstSave || m_editor->IsFileChanged() || os::fs::exists(strFullFileName))
				{
					const auto FilePos = m_editor->GetCurPos(true, m_bAddSignature);

					/* $ 01.02.2001 IS
					   ! Открываем viewer с указанием длинного имени файла, а не короткого
					*/
					bool NeedQuestion = true;
					if (ProcessQuitKey(FirstSave,NeedQuestion,false))
					{
						int delete_on_close = 0;
						if (m_Flags.Check(FFILEEDIT_DELETEONCLOSE))
							delete_on_close = 1;
						else if (m_Flags.Check(FFILEEDIT_DELETEONLYFILEONCLOSE))
							delete_on_close = 2;
						SetDeleteOnClose(0);

						FileViewer::create(
							strFullFileName,
							GetCanLoseFocus(),
							m_Flags.Check(FFILEEDIT_DISABLEHISTORY),
							false,
							FilePos,
							{},
							&EditNamesList,
							m_Flags.Check(FFILEEDIT_SAVETOSAVEAS),
							cp,
							strTitle,
							delete_on_close,
							shared_from_this());
					}
				}

				return true;
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
				Global->CtrlObject->CmdLine()->ShowViewEditHistory();
				return true;
			}

			break; // отдадим Alt-F11 на растерзание плагинам, если редактор модальный
		}
	}

	bool ProcessedNext = true;

	_SVS(if (LocalKey=='n' || LocalKey=='m'))
		_SVS(SysLog(L"%d Key='%c'",__LINE__,LocalKey));

	const auto MacroState = Global->CtrlObject->Macro.GetState();
	if (!CalledFromControl && (MacroState == MACROSTATE_RECORDING_COMMON || MacroState == MACROSTATE_EXECUTING_COMMON || MacroState == MACROSTATE_NOMACRO))
	{
		assert(Key.IsEvent());
		if (Key.IsReal())
		{
			ProcessedNext=!ProcessEditorInput(Key.Event());
		}
	}

	if (ProcessedNext)
	{
		switch (LocalKey)
		{
			case KEY_F1:
			{
				help::show(L"Editor"sv);
				return true;
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

					if (!m_editor->EdOpt.PersistentBlocks && m_editor->IsAnySelection())
					{
						m_editor->TurnOffMarkingBlock();
						m_editor->DeleteBlock();
					}

					m_editor->Paste(strFullFileName); //???
					//if (!EdOpt.PersistentBlocks)
					m_editor->UnmarkBlock();
					m_editor->Pasting--;
					m_editor->Show(); //???
				}

				return true;
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

				Global->WindowManager->RefreshAll();

				return true;
			}
			case KEY_F2:
			case KEY_SHIFTF2:
			{
				auto Done = false;
				while (!Done) // бьемся до упора
				{
					// проверим путь к файлу, может его уже снесли...
					// BUGBUG, похоже, не работает
					const auto pos = FindLastSlash(strFullFileName);
					if (pos != string::npos)
					{
						const auto Path = string_view(strFullFileName).substr(pos);

						// В корне?
						if(IsRootPath(Path))
						{
							// а дальше? каталог существует?
							if (!os::fs::is_directory(Path)
							        //|| LocalStricmp(OldCurDir,FullFileName)  // <- это видимо лишнее.
							   )
								m_Flags.Set(FFILEEDIT_SAVETOSAVEAS);
						}
					}

					if (LocalKey == KEY_F2 && os::fs::is_file(strFullFileName))
					{
						m_Flags.Clear(FFILEEDIT_SAVETOSAVEAS);
					}

					static eol SavedEol = eol::none; // none here means "do not change"
					uintptr_t codepage = m_codepage;
					const auto SaveAs = LocalKey==KEY_SHIFTF2 || m_Flags.Check(FFILEEDIT_SAVETOSAVEAS);
					string strFullSaveAsName = strFullFileName;

					if (SaveAs)
					{
						string strSaveAsName = m_Flags.Check(FFILEEDIT_SAVETOSAVEAS)?strFullFileName:strFileName;

						if (!dlgSaveFileAs(strSaveAsName, SavedEol, codepage, m_bAddSignature))
							return false;

						strSaveAsName = unquote(os::env::expand(strSaveAsName));
						const auto NameChanged = !equal_icase(strSaveAsName, m_Flags.Check(FFILEEDIT_SAVETOSAVEAS)? strFullFileName : strFileName);

						if (NameChanged)
						{
							if (!AskOverwrite(strSaveAsName))
							{
								return true;
							}
						}

						strFullSaveAsName = ConvertNameToFull(strSaveAsName);  //BUGBUG, не проверяем имя на правильность
					}

					error_state_ex ErrorState;
					int SaveResult=SaveFile(strFullSaveAsName, 0, SaveAs, ErrorState, SavedEol, codepage, m_bAddSignature);

					if (SaveResult==SAVEFILE_ERROR)
					{
						if (OperationFailed(ErrorState, strFullFileName, lng::MEditTitle, msg(lng::MEditCannotSave), false) != operation::retry)
						{
							Done = true;
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
								m_editor->PushString({});
								m_codepage = codepage;
							}

							SetFileName(strFullSaveAsName);

							if (!bInPlace)
							{
								//Message(MSG_WARNING, 1, L"WARNING!", L"Editor will be reopened with new file!", msg(lng::MOk));
								int UserBreak;
								error_state_ex LoadErrorState;
								LoadFile(strFullSaveAsName, UserBreak, LoadErrorState); // BUGBUG check result
								// TODO: возможно подобный ниже код здесь нужен (copy/paste из FileEditor::Init()). оформить его нужно по иному
								//if(!Global->Opt->Confirm.Esc && UserBreak && GetExitCode()==XC_LOADING_INTERRUPTED && WindowManager)
								//  WindowManager->RefreshWindow();
							}

							// перерисовывать надо как минимум когда изменилась кодировка или имя файла
							ShowConsoleTitle();
							Show();//!!! BUGBUG
						}
						Done = true;
					}
					else
					{
						Done = true;
						break;
					}
				}

				return true;
			}
			// $ 30.05.2003 SVS - Shift-F4 в редакторе/вьювере позволяет открывать другой редактор/вьювер (пока только редактор)
			case KEY_SHIFTF4:
			{
				if (!Global->OnlyEditorViewerUsed && GetCanLoseFocus())
				{
					if (!m_Flags.Check(FFILEEDIT_DISABLESAVEPOS) && (m_editor->EdOpt.SavePos || m_editor->EdOpt.SaveShortPos)) // save position/codepage before reload
						SaveToCache();
					Global->CtrlObject->Cp()->ActivePanel()->ProcessKey(Key);
				}
				return true;
			}
			// $ 21.07.2000 SKV + выход с позиционированием на редактируемом файле по CTRLF10
			case KEY_CTRLF10:
			case KEY_RCTRLF10:
			{
				if (Global->WindowManager->InModal())
				{
					return true;
				}

				string strFullFileNameTemp = strFullFileName;

				if (!os::fs::exists(strFullFileName)) // а сам файл то еще на месте?
				{
					if (!CheckShortcutFolder(strFullFileNameTemp, true, false))
						return false;

					path::append(strFullFileNameTemp, L'.'); // для вваливания внутрь :-)
				}

				const auto ActivePanel = Global->CtrlObject->Cp()->ActivePanel();

				if (m_Flags.Check(FFILEEDIT_NEW) || (ActivePanel && ActivePanel->FindFile(strFileName) == -1)) // Mantis#279
				{
					UpdateFileList();
					m_Flags.Clear(FFILEEDIT_NEW);
				}

				{
					SCOPED_ACTION(SaveScreen);
					Global->CtrlObject->Cp()->GoToFile(strFullFileNameTemp);
					m_Flags.Set(FFILEEDIT_REDRAWTITLE);
				}

				return true;
			}
			case KEY_CTRLB:
			case KEY_RCTRLB:
			{
				Global->Opt->EdOpt.ShowKeyBar=!Global->Opt->EdOpt.ShowKeyBar;

				if (IsKeyBarVisible())
					m_windowKeyBar->Show();
				else
					m_windowKeyBar->Hide();

				Show();
				return true;
			}
			case KEY_CTRLSHIFTB:
			case KEY_RCTRLSHIFTB:
			{
				Global->Opt->EdOpt.ShowTitleBar=!Global->Opt->EdOpt.ShowTitleBar;
				Show();
				return true;
			}
			case KEY_SHIFTF10:

				if (!ProcessKey(Manager::Key(KEY_F2))) // учтем факт того, что могли отказаться от сохранения
					return false;
				[[fallthrough]];
			case KEY_F4:
				if (F4KeyOnly)
					return true;
				[[fallthrough]];
			case KEY_ESC:
			case KEY_F10:
			{
				bool FirstSave = true, NeedQuestion = true;

				if (LocalKey != KEY_SHIFTF10)   // KEY_SHIFTF10 не учитываем!
				{
					const auto FilePlaced = !os::fs::exists(strFullFileName) && !m_Flags.Check(FFILEEDIT_NEW);

					if (m_editor->IsFileChanged() || // в текущем сеансе были изменения?
					        FilePlaced) // а сам файл то еще на месте?
					{
						auto MsgLine1 = lng::MNewFileName;
						if (m_editor->IsFileChanged() && FilePlaced)
							MsgLine1 = lng::MEditSavedChangedNonFile;
						else if (!m_editor->IsFileChanged() && FilePlaced)
							MsgLine1 = lng::MEditSavedChangedNonFile1;

						if (MsgLine1 != lng::MNewFileName)
						{
							switch (Message(MSG_WARNING,
								msg(lng::MEditTitle),
								{
									msg(MsgLine1),
									msg(lng::MEditSavedChangedNonFile2)
								},
								{ lng::MHYes, lng::MHNo, lng::MHCancel },
								{}, &EditorSaveExitDeletedId))
							{
							case Message::first_button:

								if (!ProcessKey(Manager::Key(KEY_F2))) // попытка сначала сохранить
									NeedQuestion = false;

								FirstSave = false;
								break;

							case Message::second_button:
								NeedQuestion = false;
								FirstSave = false;
								break;

							default:
								return false;
							}
						}
					}
					else if (!m_editor->m_Flags.Check(Editor::FEDITOR_MODIFIED)) //????
						NeedQuestion = false;
				}

				return ProcessQuitKey(FirstSave, NeedQuestion);
			}

			case KEY_F8:
			{
				SetCodePageEx(f8cps.NextCP(m_codepage));
				return true;
			}
			case KEY_SHIFTF8:
			{
				uintptr_t codepage = m_codepage;
				if (codepages::instance().SelectCodePage(codepage, false, true))
					SetCodePageEx(codepage == CP_DEFAULT? CP_REDETECT : codepage);

				return true;
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

				if (IsKeyBarVisible())
					m_windowKeyBar->Show();

				m_editor->Show();
				return true;
			}
			default:
			{
				if (m_Flags.Check(FFILEEDIT_FULLSCREEN) && !Global->CtrlObject->Macro.IsExecuting())
					if (IsKeyBarVisible())
						m_windowKeyBar->Show();

				if (!m_windowKeyBar->ProcessKey(Key))
					return m_editor->ProcessKey(Key);
			}
		}
	}
	return true;
}

bool FileEditor::SetCodePageEx(uintptr_t cp)
{
	if (cp == CP_DEFAULT)
	{
		EditorPosCache epc;
		if (!LoadFromCache(epc) || epc.CodePage <= 0 || epc.CodePage > 0xffff)
			return false;

		cp = epc.CodePage;
	}
	else if (cp == CP_REDETECT)
	{
		const os::fs::file EditFile(strFileName, FILE_READ_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING);
		const auto DefaultCodepage = GetDefaultCodePage();
		cp = EditFile? GetFileCodepage(EditFile, DefaultCodepage) : DefaultCodepage;
	}

	if (cp == CP_DEFAULT || !codepages::IsCodePageSupported(cp))
	{
		Message(MSG_WARNING,
			msg(lng::MEditTitle),
			{
				format(msg(lng::MEditorCPNotSupported), cp)
			},
			{ lng::MOk });
		return false;
	}

	if (cp == m_codepage)
		return true;

	const auto CurrentCodepage = m_codepage;

	const auto need_reload = !m_Flags.Check(FFILEEDIT_NEW) // we can't reload non-existing file
		&& (BadConversion
		|| IsUnicodeCodePage(m_codepage)
		|| IsUnicodeCodePage(cp));

	if (need_reload)
	{
		if (IsFileModified())
		{
			if (Message(MSG_WARNING,
				msg(lng::MEditTitle),
				{
					msg(lng::MEditorReloadCPWarnLost1),
					msg(lng::MEditorReloadCPWarnLost2)
				},
				{ lng::MOk, lng::MCancel }) != Message::first_button)
			{
				return false;
			}
		}
		// BUGBUG result check???
		ReloadFile(cp);
	}
	else
	{
		SetCodePage(cp);
	}

	if (m_codepage == CurrentCodepage)
		return false;

	InitKeyBar();
	return true;
}

bool FileEditor::ProcessQuitKey(int FirstSave, bool NeedQuestion, bool DeleteWindow)
{
	for (;;)
	{
		int SaveCode=SAVEFILE_SUCCESS;
		error_state_ex ErrorState;

		if (NeedQuestion)
		{
			SaveCode=SaveFile(strFullFileName, FirstSave, false, ErrorState);
		}

		if (SaveCode==SAVEFILE_CANCEL)
			break;

		if (SaveCode==SAVEFILE_SUCCESS)
		{
			/* $ 09.02.2002 VVM
			  + Обновить панели, если писали в текущий каталог */
			if (NeedQuestion)
			{
				if (os::fs::exists(strFullFileName))
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

		if (strFileName == msg(lng::MNewFileName))
		{
			if (!ProcessKey(Manager::Key(KEY_SHIFTF2)))
			{
				return FALSE;
			}
			else
				break;
		}

		if (OperationFailed(ErrorState, strFullFileName, lng::MEditTitle, msg(lng::MEditCannotSave), false) != operation::retry)
			break;

		FirstSave=0;
	}
	return GetExitCode() == XC_QUIT;
}

bool FileEditor::LoadFile(const string& Name,int &UserBreak, error_state_ex& ErrorState)
{
	try
	{
	// TODO: indentation
	SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<Editor::EditorPreRedrawItem>());
	SCOPED_ACTION(taskbar::indeterminate);
	SCOPED_ACTION(wakeful);

	EditorPosCache pc;
	UserBreak = 0;
	os::fs::file EditFile(Name, FILE_READ_DATA, FILE_SHARE_READ | (Global->Opt->EdOpt.EditOpenedForWrite? (FILE_SHARE_WRITE | FILE_SHARE_DELETE) : 0), nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
	if(!EditFile)
	{
		ErrorState = error_state::fetch();
		if ((ErrorState.Win32Error != ERROR_FILE_NOT_FOUND) && (ErrorState.Win32Error != ERROR_PATH_NOT_FOUND))
		{
			UserBreak = -1;
			m_Flags.Set(FFILEEDIT_OPENFAILED);
		}

		return false;
	}

	if (Global->Opt->EdOpt.FileSizeLimit)
	{
		unsigned long long FileSize = 0;
		if (EditFile.GetSize(FileSize))
		{
			if (FileSize > static_cast<unsigned long long>(Global->Opt->EdOpt.FileSizeLimit))
			{
				if (Message(MSG_WARNING,
					msg(lng::MEditTitle),
					{
						Name,
						// Ширина = 8 - это будет... в Kb и выше...
						format(msg(lng::MEditFileLong), trim(FileSizeToStr(FileSize, 8))),
						format(msg(lng::MEditFileLong2), trim(FileSizeToStr(Global->Opt->EdOpt.FileSizeLimit, 8))),
						msg(lng::MEditROOpen)
					},
					{ lng::MYes, lng::MNo },
					{}, &EditorFileLongId) != Message::first_button)
				{
					EditFile.Close();
					SetLastError(ERROR_OPEN_FAILED); //????
					ErrorState = error_state::fetch();
					UserBreak=1;
					m_Flags.Set(FFILEEDIT_OPENFAILED);
					return false;
				}
			}
		}
		else
		{
			if (Message(MSG_WARNING,
				msg(lng::MEditTitle),
				{
					Name, msg(lng::MEditFileGetSizeError),
					msg(lng::MEditROOpen)
				},
				{ lng::MYes, lng::MNo },
				{}, &EditorFileGetSizeErrorId) != Message::first_button)
			{
				EditFile.Close();
				SetLastError(ERROR_OPEN_FAILED); //????
				ErrorState = error_state::fetch();
				UserBreak=1;
				m_Flags.Set(FFILEEDIT_OPENFAILED);
				return false;
			}
		}
	}

	for (BitFlags f0 = m_editor->m_Flags; ; m_editor->m_Flags = f0)
	{
		m_editor->FreeAllocatedData();
		const auto Cached = LoadFromCache(pc);

		const os::fs::file_status FileStatus(Name);
		if ((m_editor->EdOpt.ReadOnlyLock & 0_bit) && FileStatus.check(FILE_ATTRIBUTE_READONLY | (m_editor->EdOpt.ReadOnlyLock & 0b0110'0000) >> 4))
		{
			m_editor->m_Flags.Invert(Editor::FEDITOR_LOCKMODE);
		}

		if (Cached && pc.CodePage && !codepages::IsCodePageSupported(pc.CodePage))
			pc.CodePage = 0;

		bool testBOM = true;

		const auto Redetect = (m_codepage == CP_REDETECT);
		if (Redetect)
			m_codepage = CP_DEFAULT;

		if (m_codepage == CP_DEFAULT)
		{
			if (!Redetect && Cached && pc.CodePage)
			{
				m_codepage = pc.CodePage;
			}
			else
			{
				const auto Cp = GetFileCodepage(EditFile, GetDefaultCodePage(), &m_bAddSignature, Redetect || Global->Opt->EdOpt.AutoDetectCodePage);
				testBOM = false;
				if (codepages::IsCodePageSupported(Cp))
					m_codepage = Cp;
			}

			if (m_codepage == CP_DEFAULT)
				m_codepage = GetDefaultCodePage();
		}
		m_editor->SetCodePage(m_codepage, nullptr, false);  //BUGBUG
		m_editor->GlobalEOL = eol::none;

		unsigned long long FileSize = 0;
		// BUGBUG check result
		(void)EditFile.GetSize(FileSize);
		const time_check TimeCheck;

		os::fs::filebuf StreamBuffer(EditFile, std::ios::in);
		std::istream Stream(&StreamBuffer);
		Stream.exceptions(Stream.badbit | Stream.failbit);

		enum_lines EnumFileLines(Stream, m_codepage);
		for (auto Str: EnumFileLines)
		{
			if (testBOM && IsUnicodeOrUtfCodePage(m_codepage))
			{
				if (starts_with(Str.Str, encoding::bom_char))
				{
					Str.Str.remove_prefix(1);
					m_bAddSignature = true;
				}
			}
			testBOM = false;

			if (TimeCheck)
			{
				if (CheckForEscSilent())
				{
					if (ConfirmAbortOp())
					{
						UserBreak = 1;
						EditFile.Close();
						return false;
					}
				}

				SetCursorType(false, 0);
				const auto CurPos = EditFile.GetPointer();
				auto Percent = CurPos * 100 / FileSize;
				// В случае если во время загрузки файл увеличивается размере, то количество
				// процентов может быть больше 100. Обрабатываем эту ситуацию.
				if (Percent > 100)
				{
					// BUGBUG check result
					(void)EditFile.GetSize(FileSize);
					Percent = std::min(CurPos * 100 / FileSize, 100ull);
				}
				Editor::EditorShowMsg(msg(lng::MEditTitle), msg(lng::MEditReading), Name, Percent);
			}

			if (m_editor->GlobalEOL == eol::none && Str.Eol != eol::none)
			{
				m_editor->GlobalEOL = Str.Eol;
			}

			m_editor->PushString(Str.Str);
			m_editor->LastLine()->SetEOL(Str.Eol);
		}

		BadConversion = EnumFileLines.conversion_error();
		if (BadConversion)
		{
			uintptr_t cp = m_codepage;
			if (!dlgBadEditorCodepage(cp)) // cancel
			{
				EditFile.Close();
				SetLastError(ERROR_OPEN_FAILED); //????
				ErrorState = error_state::fetch();
				UserBreak=1;
				m_Flags.Set(FFILEEDIT_OPENFAILED);
				return false;
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

	if (m_editor->Lines.empty() || m_editor->Lines.back().GetEOL() != eol::none)
		m_editor->PushString({});

	if (m_editor->GlobalEOL == eol::none)
		m_editor->GlobalEOL = Editor::GetDefaultEOL();

	EditFile.Close();
	m_editor->SetCacheParams(pc, m_bAddSignature);
	ErrorState = error_state::fetch();
	// BUGBUG check result
	(void)os::fs::get_find_data(Name, FileInfo);
	EditorGetFileAttributes(Name);
	return true;

	}
	catch (const std::bad_alloc&)
	{
		// TODO: better diagnostics
		m_editor->FreeAllocatedData();
		m_Flags.Set(FFILEEDIT_OPENFAILED);
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		ErrorState = error_state::fetch();
		return false;
	}
	catch (const std::exception&)
	{
		// A portion of file can be locked

		// TODO: better diagnostics
		m_editor->FreeAllocatedData();
		m_Flags.Set(FFILEEDIT_OPENFAILED);
		ErrorState = error_state::fetch();
		return false;
	}
}

bool FileEditor::ReloadFile(uintptr_t codepage)
{
	const auto save_codepage(m_codepage);
	const auto save_bAddSignature(m_bAddSignature);
	const auto save_BadConversiom(BadConversion);
	const auto save_Flags(m_Flags), save_Flags1(m_editor->m_Flags);

	Editor saved(shared_from_this(), CP_DEFAULT);
	saved.fake_editor = true;
	m_editor->SwapState(saved);

	int user_break = 0;
	m_codepage = codepage;
	error_state_ex ErrorState;

	for (;;)
	{
		if (LoadFile(strFullFileName, user_break, ErrorState))
		{
			m_editor->m_Flags.Set(Editor::FEDITOR_WASCHANGED);
			m_editor->m_Flags.Clear(Editor::FEDITOR_MODIFIED);
			Show();
			return true;
		}

		if (user_break != 1)
		{
			if (OperationFailed(ErrorState, strFullFileName, lng::MEditTitle, msg(lng::MEditCannotOpen), false) == operation::retry)
				continue;
		}

		m_codepage = save_codepage;
		m_bAddSignature = save_bAddSignature;
		BadConversion = save_BadConversiom;
		m_Flags = save_Flags;
		m_editor->m_Flags = save_Flags1;
		m_editor->SwapState(saved);

		Show();
		return false;
	}
}

//TextFormat и codepage используются ТОЛЬКО, если bSaveAs = true!
int FileEditor::SaveFile(const string& Name,int Ask, bool bSaveAs, error_state_ex& ErrorState, eol Eol, uintptr_t Codepage, bool AddSignature)
{
	if (!bSaveAs)
	{
		Eol = eol::none;
		Codepage=m_editor->GetCodePage();
	}

	if (m_editor->m_Flags.Check(Editor::FEDITOR_LOCKMODE) && !m_editor->m_Flags.Check(Editor::FEDITOR_MODIFIED) && !bSaveAs)
		return SAVEFILE_SUCCESS;

	SCOPED_ACTION(taskbar::indeterminate);
	SCOPED_ACTION(wakeful);

	if (Ask)
	{
		if (!m_editor->m_Flags.Check(Editor::FEDITOR_MODIFIED))
			return SAVEFILE_SUCCESS;


		std::vector Buttons{ lng::MHYes, lng::MHNo };
		if (Global->AllowCancelExit)
		{
			Buttons.emplace_back(lng::MHCancel);
		}

		int Code = Message(MSG_WARNING,
			msg(lng::MEditTitle),
			{
				msg(lng::MEditAskSave)
			},
			Buttons,
			{}, &EditAskSaveId);
		if(Code < 0 && !Global->AllowCancelExit)
		{
			Code = Message::second_button; // close == not save
		}

		switch (Code)
		{
		case Message::first_button: // Save
			break;

		case Message::second_button: // Not Save
			m_editor->TextChanged(false);
			return SAVEFILE_SUCCESS;

		default:
			return SAVEFILE_CANCEL;
		}
	}

	int NewFile=TRUE;

	const auto FileAttr = os::fs::get_file_attributes(Name);
	if (FileAttr != INVALID_FILE_ATTRIBUTES)
	{
		// Проверка времени модификации...
		if (!m_Flags.Check(FFILEEDIT_SAVEWQUESTIONS))
		{
			os::fs::find_data FInfo;

			if (os::fs::get_find_data(Name, FInfo) && !FileInfo.FileName.empty())
			{
				if (FileInfo.LastWriteTime != FInfo.LastWriteTime || FInfo.FileSize != FileInfo.FileSize)
				{
					switch (Message(MSG_WARNING,
						msg(lng::MEditTitle),
						{
							msg(lng::MEditAskSaveExt)
						},
						{ lng::MHYes, lng::MEditBtnSaveAs, lng::MHCancel },
						L"WarnEditorSavedEx"sv, &EditAskSaveExtId))
					{
					case Message::first_button: // Save
						break;

					case Message::second_button: // Save as
						return ProcessKey(Manager::Key(KEY_SHIFTF2))?
							SAVEFILE_SUCCESS :
							SAVEFILE_CANCEL;

					default:
						return SAVEFILE_CANCEL;
					}
				}
			}
		}

		m_Flags.Clear(FFILEEDIT_SAVEWQUESTIONS);
		NewFile=FALSE;

		if (FileAttr & FILE_ATTRIBUTE_READONLY)
		{
			//BUGBUG
			if (Message(MSG_WARNING,
				msg(lng::MEditTitle),
				{
					Name,
					msg(lng::MEditRO),
					msg(lng::MEditOvr)
				},
				{ lng::MYes, lng::MNo },
				{}, &EditorSavedROId) != Message::first_button)
				return SAVEFILE_CANCEL;

			(void)os::fs::set_file_attributes(Name, FileAttr & ~FILE_ATTRIBUTE_READONLY); //BUGBUG
		}
	}
	else
	{
		// проверим путь к файлу, может его уже снесли...
		string strCreatedPath = Name;
		if (CutToParent(strCreatedPath))
		{
			if (!os::fs::exists(strCreatedPath))
			{
				// и попробуем создать.
				// Раз уж
				CreatePath(strCreatedPath);
				if (!os::fs::exists(strCreatedPath))
				{
					ErrorState = error_state::fetch();
					return SAVEFILE_ERROR;
				}
			}
		}
	}

	if (BadConversion)
	{
		if(Message(MSG_WARNING,
			msg(lng::MWarning),
			{
				msg(lng::MEditDataLostWarn),
				msg(lng::MEditorSaveNotRecommended)
			},
			{ lng::MEditorSave, lng::MCancel }) == Message::first_button)
		{
			BadConversion = false;
		}
		else
		{
			return SAVEFILE_CANCEL;
		}
	}

	int RetCode=SAVEFILE_SUCCESS;

	if (Eol != eol::none)
	{
		m_editor->m_Flags.Set(Editor::FEDITOR_WASCHANGED);
		m_editor->GlobalEOL = Eol;
	}

	if (!os::fs::exists(Name))
		m_Flags.Set(FFILEEDIT_NEW);

	//SaveScreen SaveScr;
	/* $ 11.10.2001 IS
		Если было произведено сохранение с любым результатом, то не удалять файл
	*/
	m_Flags.Clear(FFILEEDIT_DELETEONCLOSE|FFILEEDIT_DELETEONLYFILEONCLOSE);
	//_D(SysLog(L"%08d EE_SAVE",__LINE__));

	if (!IsUnicodeOrUtfCodePage(Codepage))
	{
		int LineNumber=-1;
		encoding::error_position ErrorPosition;

		for(auto& Line: m_editor->Lines)
		{
			++LineNumber;
			const auto& SaveStr = Line.GetString();
			auto LineEol = Line.GetEOL();

			(void)encoding::get_bytes_count(Codepage, SaveStr, &ErrorPosition);
			const auto ValidStr = !ErrorPosition;
			if (ValidStr)
				(void)encoding::get_bytes_count(Codepage, LineEol.str(), &ErrorPosition);

			if (ErrorPosition)
			{
				//SetMessageHelp(L"EditorDataLostWarning")
				const int Result = Message(MSG_WARNING,
					msg(lng::MWarning),
					{
						msg(lng::MEditorSaveCPWarn1),
						msg(lng::MEditorSaveCPWarn2),
						msg(lng::MEditorSaveNotRecommended)
					},
					{ lng::MCancel, lng::MEditorSaveCPWarnShow, lng::MEditorSave });
				if (Result == Message::third_button)
					break;

				if(Result == Message::second_button)
				{
					m_editor->GoToLine(LineNumber);
					if(!ValidStr)
					{
						Line.SetCurPos(static_cast<int>(*ErrorPosition));
					}
					else
					{
						Line.SetCurPos(Line.GetLength());
					}
					Show();
				}
				return SAVEFILE_CANCEL;
			}
		}
	}

	EditorSaveFile esf = {sizeof(esf), Name.c_str(), m_editor->GlobalEOL.str().data(), Codepage};
	Global->CtrlObject->Plugins->ProcessEditorEvent(EE_SAVE, &esf, m_editor.get());

	try
	{
		save_file_with_replace(Name, FileAttr, 0, Global->Opt->EdOpt.CreateBackups, [&](std::ostream& Stream)
		{
			m_editor->UndoSavePos = m_editor->UndoPos;
			m_editor->m_Flags.Clear(Editor::FEDITOR_UNDOSAVEPOSLOST);

			SetCursorType(false, 0);
			SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<Editor::EditorPreRedrawItem>());

			if (!bSaveAs)
				AddSignature = m_bAddSignature;

			const time_check TimeCheck;

			encoding::writer Writer(Stream, Codepage, AddSignature);

			size_t LineNumber = -1;

			for (auto& Line : m_editor->Lines)
			{
				++LineNumber;

				if (TimeCheck)
				{
					Editor::EditorShowMsg(msg(lng::MEditTitle), msg(lng::MEditSaving), Name, LineNumber * 100 / m_editor->Lines.size());
				}

				const auto& SaveStr = Line.GetString();
				auto LineEol = Line.GetEOL();

				if (Eol != eol::none && LineEol != eol::none)
				{
					LineEol = m_editor->GlobalEOL;
					Line.SetEOL(LineEol);
				}

				Writer.write(SaveStr);
				Writer.write(LineEol.str());
			}
		});
	}
	catch (const far_exception& e)
	{
		RetCode = SAVEFILE_ERROR;
		ErrorState = e;
	}

	// BUGBUG check result
	(void)os::fs::get_find_data(Name, FileInfo);
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

	Show();
	// ************************************
	m_Flags.Clear(FFILEEDIT_NEW);
	return RetCode;
}

bool FileEditor::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	F4KeyOnly = false;
	if (!m_windowKeyBar->ProcessMouse(MouseEvent))
	{
		INPUT_RECORD mouse = { MOUSE_EVENT };
		mouse.Event.MouseEvent=*MouseEvent;
		if (!ProcessEditorInput(mouse))
			if (!m_editor->ProcessMouse(MouseEvent))
				return false;
	}

	return true;
}


int FileEditor::GetTypeAndName(string &strType, string &strName)
{
	strType = msg(lng::MScreensEdit);
	strName = strFullFileName;
	return windowtype_editor;
}


void FileEditor::ShowConsoleTitle()
{
	string strEditorTitleFormat=Global->Opt->strEditorTitleFormat.Get();
	replace_icase(strEditorTitleFormat, L"%Lng"sv, msg(lng::MInEditor));
	replace_icase(strEditorTitleFormat, L"%File"sv, PointToName(strFileName));
	ConsoleTitle::SetFarTitle(strEditorTitleFormat);
	m_Flags.Clear(FFILEEDIT_REDRAWTITLE);
}

void FileEditor::SetScreenPosition()
{
	if (m_Flags.Check(FFILEEDIT_FULLSCREEN))
	{
		SetPosition({ 0, 0, ScrX, ScrY });
	}
}

void FileEditor::OnDestroy()
{
	_OT(SysLog(L"[%p] FileEditor::OnDestroy()",this));

	if (Global->CtrlObject && !m_Flags.Check(FFILEEDIT_DISABLEHISTORY) && !equal_icase(strFileName, msg(lng::MNewFileName)))
		Global->CtrlObject->ViewHistory->AddToHistory(strFullFileName, m_editor->m_Flags.Check(Editor::FEDITOR_LOCKMODE) ? HR_EDITOR_RO : HR_EDITOR);

	//AY: флаг оповещающий закрытие редактора.
	m_bClosing = true;

	if (bEE_READ_Sent && Global->CtrlObject)
	{
		Global->CtrlObject->Plugins->ProcessEditorEvent(EE_CLOSE, nullptr, m_editor.get());
	}
	if (!m_Flags.Check(FFILEEDIT_OPENFAILED) && !m_Flags.Check(FFILEEDIT_DISABLESAVEPOS) && (m_editor->EdOpt.SavePos || m_editor->EdOpt.SaveShortPos) && Global->CtrlObject)
		SaveToCache();
}

bool FileEditor::GetCanLoseFocus(bool DynamicMode) const
{
	if (DynamicMode && m_editor->IsFileModified())
	{
		return false;
	}

	return window::GetCanLoseFocus();
}

void FileEditor::SetLockEditor(bool LockMode) const
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

int FileEditor::ProcessEditorInput(const INPUT_RECORD& Rec)
{
	return Global->CtrlObject->Plugins->ProcessEditorInput(&Rec);
}

void FileEditor::SetPluginTitle(const string* PluginTitle)
{
	if (!PluginTitle)
		strPluginTitle.clear();
	else
		strPluginTitle = *PluginTitle;
}

bool FileEditor::SetFileName(const string_view NewFileName)
{
	// BUGBUG This whole MNewFileName thing is madness.
	// TODO: Just support an empty name
	strFileName = NewFileName.empty()? msg(lng::MNewFileName) : NewFileName;

	if (strFileName != msg(lng::MNewFileName))
	{
		strFullFileName = ConvertNameToFull(strFileName);
		string strFilePath=strFullFileName;

		if (CutToParent(strFilePath))
		{
			if (equal_icase(strFilePath, os::fs::GetCurrentDirectory()))
				strFileName = PointToName(strFullFileName);
		}

		//Дабы избежать бардака, развернём слешики...
		ReplaceSlashToBackslash(strFullFileName);
	}
	else
	{
		strFullFileName = path::join(strStartDir, strFileName);
	}

	return true;
}

void FileEditor::SetTitle(const string* Title)
{
	if (Title)
		strTitle = *Title;
	else
		strTitle.clear();
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

static std::pair<string, size_t> char_code(std::optional<wchar_t> const& Char, int const Codebase)
{
	const auto process = [&](string_view const Format, string_view const Max)
	{
		return std::pair{ Char.has_value()? format(Format, unsigned(*Char)) : L""s, Max.size() };
	};

	switch (Codebase)
	{
	case 0:
		return process(L"0{0:o}"sv, L"0177777"sv);

	case 2:
		return process(L"{0:X}h"sv, L"FFFFh"sv);

	case 1:
	default:
		return process(L"{0}"sv, L"65535"sv);
	}
}

static std::pair<string, size_t> ansi_char_code(std::optional<wchar_t> const& Char, int const Codebase, uintptr_t const Codepage)
{
	const auto process = [&](string_view const Format, string_view const Max)
	{
		std::optional<unsigned> CharCode;

		char Buffer;
		encoding::error_position ErrorPosition;
		if (Char.has_value() && encoding::get_bytes(Codepage, { &*Char, 1 }, { &Buffer, 1 }, &ErrorPosition) == 1 && !ErrorPosition)
		{
			const unsigned AnsiCode = Buffer;
			if (AnsiCode != *Char)
			{
				CharCode = AnsiCode;
			}
		}

		return std::pair{ CharCode.has_value()? format(Format, *CharCode) : L""s, Max.size() };
	};

	switch (Codebase)
	{
	case 0:
		return process(L"0{0:<3o}"sv, L"0377"sv);

	case 2:
		return process(L"{0:02X}h"sv, L"FFh"sv);

	case 1:
	default:
		return process(L"{0:<3}"sv, L"255"sv);
	}
}


void FileEditor::ShowStatus() const
{
	if (!IsTitleBarVisible())
		return;

	SetColor(COL_EDITORSTATUS);
	GotoXY(m_Where.left, m_Where.top); //??


	const auto& Str = m_editor->m_it_CurLine->GetString();
	const size_t CurPos = m_editor->m_it_CurLine->GetCurPos();

	string CharCode;

	{
		std::optional<wchar_t> Char;
		if (CurPos < Str.size())
			Char = Str[CurPos];

		auto [UnicodeStr, UnicodeSize] = char_code(Char, m_editor->EdOpt.CharCodeBase);
		CharCode = std::move(UnicodeStr);

		if (!IsUnicodeOrUtfCodePage(m_codepage))
		{
			const auto [AnsiStr, AnsiSize] = ansi_char_code(Char, m_editor->EdOpt.CharCodeBase, m_codepage);
			if (!CharCode.empty() && !AnsiStr.empty())
			{
				append(CharCode, L'/', AnsiStr);
			}
			UnicodeSize += AnsiSize + 1;
		}

		if (Char.has_value())
			inplace::pad_right(CharCode, UnicodeSize);
		else
			CharCode.assign(UnicodeSize, L' ');
	}

	//предварительный расчет
	const auto LinesFormat = FSTR(L"{0}/{1}");
	const auto SizeLineStr = format(LinesFormat, m_editor->Lines.size(), m_editor->Lines.size()).size();
	const auto strLineStr = format(LinesFormat, m_editor->m_it_CurLine.Number() + 1, m_editor->Lines.size());
	const auto strAttr = *AttrStr? L"│"s + AttrStr : L""s;
	auto StatusLine = format(FSTR(L"│{0}{1}│{2:5.5}│{3:.3} {4:>{5}}│{6:.3} {7:<3}│{8:.3} {9:<3}{10}│{11}"),
		m_editor->m_Flags.Check(Editor::FEDITOR_MODIFIED)?L'*':L' ',
		m_editor->m_Flags.Check(Editor::FEDITOR_LOCKMODE)? L'-' : m_editor->m_Flags.Check(Editor::FEDITOR_PROCESSCTRLQ)? L'"' : L' ',
		ShortReadableCodepageName(m_codepage),
		msg(lng::MEditStatusLine),
		strLineStr,
		SizeLineStr,
		msg(lng::MEditStatusCol),
		m_editor->m_it_CurLine->GetTabCurPos() + 1,
		msg(lng::MEditStatusChar),
		m_editor->m_it_CurLine->GetCurPos() + 1,
		strAttr,
		CharCode);

	// Explicitly signed types - it's too easy to screw it up on small console sizes otherwise
	const int ClockSize = Global->Opt->ViewerEditorClock && m_Flags.Check(FFILEEDIT_FULLSCREEN)? static_cast<int>(Global->CurrentTime.size()) : 0;
	const int AvailableSpace = std::max(0, ObjWidth() - ClockSize - (ClockSize? 1 : 0));
	inplace::cut_right(StatusLine, AvailableSpace);
	const int NameWidth = std::max(0, AvailableSpace - static_cast<int>(StatusLine.size()));

	Text(fit_to_left(truncate_path(GetTitle(), NameWidth), NameWidth));
	Text(StatusLine);

	if (ClockSize)
	{
		Text(L'│'); // Separator before the clock
		ShowTime();
	}
}

/* $ 13.02.2001
     Узнаем атрибуты файла и заодно сформируем готовую строку атрибутов для
     статуса.
*/
os::fs::attributes FileEditor::EditorGetFileAttributes(string_view const Name)
{
	m_FileAttributes = os::fs::get_file_attributes(Name);
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

/* true - панель обновили
*/
bool FileEditor::UpdateFileList() const
{
	const auto ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
	const auto FileName = PointToName(strFullFileName);
	string strFilePath(strFullFileName), strPanelPath(ActivePanel->GetCurDir());
	strFilePath.resize(strFullFileName.size() - FileName.size());
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
	if (PluginData)
		strPluginData = *PluginData;
	else
		strPluginData.clear();
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

void FileEditor::SetEditorOptions(const Options::EditorOptions& EdOpt) const
{
	m_editor->SetOptions(EdOpt);
}

void FileEditor::OnChangeFocus(bool focus)
{
	window::OnChangeFocus(focus);
	if (!m_bClosing)
	{
		Global->CtrlObject->Plugins->ProcessEditorEvent(focus? EE_GOTFOCUS : EE_KILLFOCUS, nullptr, m_editor.get());
	}
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
			if (Param2 && static_cast<size_t>(Param1) > strFullFileName.size())
			{
				*copy_string(strFullFileName, static_cast<wchar_t*>(Param2)) = {};
			}

			return strFullFileName.size()+1;
		}
		case ECTL_GETBOOKMARKS:
		{
			const auto ebm = static_cast<EditorBookmarks*>(Param2);
			if (!m_Flags.Check(FFILEEDIT_OPENFAILED) && CheckNullOrStructSize(ebm))
			{
				size_t size;
				if(Editor::InitSessionBookmarksForPlugin(ebm, m_editor->m_SavePos.size(), size))
				{
					for (const auto& [i, index]: enumerate(m_editor->m_SavePos))
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
					}
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
			return m_editor->DeleteSessionBookmark(m_editor->PointerToSessionBookmark(static_cast<int>(reinterpret_cast<intptr_t>(Param2))));
		}
		case ECTL_GETSESSIONBOOKMARKS:
		{
			return CheckNullOrStructSize(static_cast<const EditorBookmarks*>(Param2))?
				m_editor->GetSessionBookmarksForPlugin(static_cast<EditorBookmarks*>(Param2)) : 0;
		}
		case ECTL_GETTITLE:
		{
			const auto strLocalTitle = GetTitle();
			if (Param2 && static_cast<size_t>(Param1) > strLocalTitle.size())
			{
				*copy_string(strLocalTitle, static_cast<wchar_t*>(Param2)) = {};
			}

			return strLocalTitle.size()+1;
		}
		case ECTL_SETTITLE:
		{
			strPluginTitle = NullToEmpty(static_cast<const wchar_t*>(Param2));
			ShowStatus();
			if (!m_editor->m_InEERedraw)
				Global->ScrBuf->Flush(); //???
			return TRUE;
		}
		case ECTL_REDRAW:
		{
			Global->WindowManager->RefreshWindow(shared_from_this());
			Global->WindowManager->PluginCommit();
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
			const auto Kbt = static_cast<const FarSetKeyBarTitles*>(Param2);

			if (!Kbt)   //восстановить изначальное
				InitKeyBar();
			else
			{
				if (reinterpret_cast<intptr_t>(Param2) != -1) // не только перерисовать?
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
			auto Eol = eol::none;
			uintptr_t codepage=m_codepage;

			const auto esf = static_cast<const EditorSaveFile*>(Param2);
			if (CheckStructSize(esf))
			{

				if (esf->FileName)
					strName=esf->FileName;

				if (esf->FileEOL)
					Eol = eol::parse(esf->FileEOL);

				if (esf->CodePage != CP_DEFAULT)
					codepage=esf->CodePage;
			}

			{
				const auto strOldFullFileName = strFullFileName;

				if (SetFileName(strName))
				{
					if (!equal_icase(strFullFileName, strOldFullFileName))
					{
						if (!AskOverwrite(strName))
						{
							SetFileName(strOldFullFileName);
							return FALSE;
						}
					}

					m_Flags.Set(FFILEEDIT_SAVEWQUESTIONS);
					//всегда записываем в режиме save as - иначе не сменить кодировку и концы линий.
					error_state_ex ErrorState;
					return SaveFile(strName, FALSE, true, ErrorState, Eol, codepage, m_bAddSignature);
				}
			}

			return FALSE;
		}
		case ECTL_QUIT:
		{
			if (!bLoaded) // do not delete not created window
			{
				SetExitCode(XC_LOADING_INTERRUPTED);
			}
			else
			{
				Global->WindowManager->DeleteWindow(shared_from_this());
				SetExitCode(XC_OPEN_ERROR); // что-то меня терзают смутные сомнения ...??? SAVEFILE_ERROR ???
				Global->WindowManager->PluginCommit();
			}
			return TRUE;
		}
		case ECTL_READINPUT:
		{
			const auto MacroState = Global->CtrlObject->Macro.GetState();
			if (MacroState == MACROSTATE_RECORDING || MacroState == MACROSTATE_EXECUTING)
			{
				//return FALSE;
			}

			if (Param2)
			{
				const auto rec = static_cast<INPUT_RECORD*>(Param2);

				for (;;)
				{
					const auto Key = GetInputRecord(rec);

					if ((!rec->EventType || rec->EventType == KEY_EVENT) &&
					        ((Key >= KEY_MACRO_BASE && Key <= KEY_MACRO_ENDBASE) || (Key>=KEY_OP_BASE && Key <=KEY_OP_ENDBASE))) // исключаем MACRO
						ReProcessKey(Manager::Key(Key, *rec));
					else
						break;
				}

#if defined(SYSLOG_KEYMACRO)

				if (rec->EventType == KEY_EVENT)
				{
					SysLog(L"ECTL_READINPUT={KEY_EVENT,{%d,%d,Vk=0x%04X,0x%08X}}",
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
				auto& rec = *static_cast<INPUT_RECORD*>(Param2);

				if (ProcessEditorInput(rec))
					return TRUE;

				if (rec.EventType==MOUSE_EVENT)
					ProcessMouse(&rec.Event.MouseEvent);
				else
				{
#if defined(SYSLOG_KEYMACRO)

					if (!rec.EventType || rec.EventType == KEY_EVENT)
					{
						SysLog(L"ECTL_PROCESSINPUT={%s,{%d,%d,Vk=0x%04X,0x%08X}}",
						       (rec.EventType == KEY_EVENT?L"KEY_EVENT":L"(internal, macro)_KEY_EVENT"),
						       rec.Event.KeyEvent.bKeyDown,
						       rec.Event.KeyEvent.wRepeatCount,
						       rec.Event.KeyEvent.wVirtualKeyCode,
						       rec.Event.KeyEvent.dwControlKeyState);
					}

#endif
					const auto Key = ShieldCalcKeyCode(&rec, false);
					ReProcessKey(Manager::Key(Key, rec));
				}

				return TRUE;
			}

			return FALSE;
		}
		case ECTL_SETPARAM:
		{
			const auto espar = static_cast<const EditorSetParameter*>(Param2);
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

	const auto result = m_editor->EditorControl(Command, Param1, Param2);
	if (result&&ECTL_GETINFO==Command)
	{
		const auto Info=static_cast<EditorInfo*>(Param2);
		if (m_bAddSignature)
			Info->Options|=EOPT_BOM;
		if (Global->Opt->EdOpt.ShowTitleBar)
			Info->Options|=EOPT_SHOWTITLEBAR;
		if (Global->Opt->EdOpt.ShowKeyBar)
			Info->Options|=EOPT_SHOWKEYBAR;

	}
	return result;
}

bool FileEditor::LoadFromCache(EditorPosCache &pc) const
{
	string strCacheName;

	const auto PluginData = GetPluginData();
	if (!PluginData.empty())
	{
		strCacheName = concat(PluginData, PointToName(strFullFileName));
	}
	else
	{
		strCacheName = strFullFileName;
		ReplaceSlashToBackslash(strCacheName);
	}

	pc.Clear();

	return FilePositionCache::GetPosition(strCacheName, pc);
}

void FileEditor::SaveToCache() const
{
	EditorPosCache pc;
	m_editor->GetCacheParams(pc);

	if (!m_Flags.Check(FFILEEDIT_OPENFAILED))   //????
	{
		pc.CodePage = BadConversion ? 0 : m_codepage;
		FilePositionCache::AddPosition(strPluginData.empty()? strFullFileName : strPluginData + PointToName(strFullFileName), pc);
	}
}

bool FileEditor::SetCodePage(uintptr_t codepage)
{
	if (codepage == m_codepage || !m_editor)
		return false;

	uintptr_t ErrorCodepage;
	size_t ErrorLine, ErrorPos;
	if (!m_editor->TryCodePage(codepage, ErrorCodepage, ErrorLine, ErrorPos))
	{
		const auto Info = codepages::GetInfo(ErrorCodepage);

		const int Result = Message(MSG_WARNING,
			msg(lng::MWarning),
			{
				msg(lng::MEditorSwitchCPWarn1),
				msg(lng::MEditorSwitchCPWarn2),
				format(FSTR(L"{0} - {1}"), codepage, Info? Info->Name : str(codepage)),
				msg(lng::MEditorSwitchCPConfirm)
			},
			{ lng::MCancel, lng::MEditorSaveCPWarnShow, lng::MOk });

		switch (Result)
		{
		default:
		case Message::first_button:
			return false;

		case Message::second_button:
			m_editor->GoToLine(static_cast<int>(ErrorLine));
			m_editor->m_it_CurLine->SetCurPos(static_cast<int>(ErrorPos));
			Show();
			return false;

		case Message::third_button:
			break;
		}
	}

	m_codepage = codepage;
	BadConversion = !m_editor->SetCodePage(m_codepage, &m_bAddSignature);
	return true;
}

bool FileEditor::AskOverwrite(const string& FileName)
{
	bool result=true;
	if (os::fs::exists(FileName))
	{
		if (Message(MSG_WARNING,
			msg(lng::MEditTitle),
			{
				FileName,
				msg(lng::MEditExists),
				msg(lng::MEditOvr)
			},
			{ lng::MYes, lng::MNo },
			{}, &EditorAskOverwriteId) != Message::first_button)
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
	if (cp < 0 || !codepages::IsCodePageSupported(cp))
		cp = encoding::codepage::ansi();
	return cp;
}

Editor* FileEditor::GetEditor()
{
	return m_editor.get();
}

bool FileEditor::IsKeyBarVisible() const
{
	return Global->Opt->EdOpt.ShowKeyBar && ObjHeight() > 2;
}

bool FileEditor::IsTitleBarVisible() const
{
	return Global->Opt->EdOpt.ShowTitleBar && ObjHeight() > 1;
}
