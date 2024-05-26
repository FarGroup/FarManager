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
#include "log.hpp"
#include "elevation.hpp"
#include "codepage.hpp"

// Platform:
#include "platform.hpp"
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

class FileEditor::f4_key_timer
{
public:
	f4_key_timer():
		m_Timer(500ms, {}, [this]{ m_Expired = true; })
	{
	}

	bool expired() const
	{
		return m_Expired;
	}

private:
	os::concurrency::timer m_Timer;
	std::atomic_bool m_Expired{};
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
			auto& param = edit_as<uintptr_t>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
			FarListPos pos{ sizeof(pos) };
			Dlg->SendMessage(DM_LISTGETCURPOS, ID_OE_CODEPAGE, &pos);
			param = Dlg->GetListItemSimpleUserData(ID_OE_CODEPAGE, pos.SelectPos);
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

	Builder.AddText(lng::MEditorLoadCPWarn1).Flags = DIF_CENTERTEXT;
	Builder.AddText(lng::MEditorLoadCPWarn2).Flags = DIF_CENTERTEXT;
	Builder.AddText(lng::MEditorSaveNotRecommended).Flags = DIF_CENTERTEXT;
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
			CurrentCodepage = view_as<uintptr_t>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
			codepages::instance().FillCodePagesList(Dlg, ID_SF_CODEPAGE, CurrentCodepage, false, false, false, false, false);
			break;
		}
		case DN_CLOSE:
		{
			if (param1 == ID_SF_OK)
			{
				auto& Codepage = edit_as<uintptr_t>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
				FarListPos pos{ sizeof(pos) };
				Dlg->SendMessage(DM_LISTGETCURPOS, ID_SF_CODEPAGE, &pos);
				Codepage = Dlg->GetListItemSimpleUserData(ID_SF_CODEPAGE, pos.SelectPos);
				return TRUE;
			}

			break;
		}
		case DN_EDITCHANGE:
		{
			if (param1==ID_SF_CODEPAGE)
			{
				FarListPos pos{ sizeof(pos) };
				Dlg->SendMessage(DM_LISTGETCURPOS,ID_SF_CODEPAGE,&pos);
				const uintptr_t cp = Dlg->GetListItemSimpleUserData(ID_SF_CODEPAGE, pos.SelectPos);
				if (cp != CurrentCodepage)
				{
					if (IsUtfCodePage(cp))
					{
						if (!IsUtfCodePage(CurrentCodepage))
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
	const auto ucp = IsUtfCodePage(codepage);

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
	EditDlg[ID_SF_FILENAME].strData = strFileName;
	EditDlg[ID_SF_SIGNATURE].Selected = AddSignature;

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
	const string EmptyTitle;
	FileEditorPtr->Init(Name, codepage, Title, StartLine, StartChar, &EmptyTitle, DeleteOnClose, Update, OpenModeExstFile);
	return FileEditorPtr;
}

FileEditor::FileEditor(private_tag)
{
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
				if (!os::fs::set_file_attributes(strFullFileName, FILE_ATTRIBUTE_NORMAL)) // BUGBUG
				{
					LOGWARNING(L"set_file_attributes({}): {}"sv, strFullFileName, os::last_error());
				}

				if (!os::fs::delete_file(strFullFileName)) //BUGBUG
				{
					LOGWARNING(L"delete_file({}): {}"sv, strFullFileName, os::last_error());
				}
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

	const auto BlankFileName = Name.empty();

	bEE_READ_Sent = false;
	bLoaded = false;
	m_bAddSignature = false;
	m_editor = std::make_unique<Editor>(shared_from_this());

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
	SetCanLoseFocus(m_Flags.Check(FFILEEDIT_ENABLEF6));
	strStartDir = os::fs::get_current_directory();

	SetFileName(Name);

	{
		if (auto EditorWindow = Global->WindowManager->FindWindowByFile(windowtype_editor, strFullFileName))
		{
			int SwitchTo=FALSE;

			if (!EditorWindow->GetCanLoseFocus(true) || Global->Opt->Confirm.AllowReedit)
			{
				int Result = XC_EXISTS;
				if (OpenModeExstFile == EF_OPENMODE_QUERY)
				{
					message_result MsgCode;
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

						if (MsgCode == message_result::first_button)
							MsgCode = message_result::second_button;
					}

					switch (MsgCode)
					{
					case message_result::first_button:
						Result = XC_EXISTS;
						break;

					case message_result::second_button:
						Result = XC_OPEN_NEWINSTANCE;
						break;

					case message_result::third_button:
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
			{}, &EditorOpenRSHId) != message_result::first_button)
		{
			SetExitCode(XC_OPEN_ERROR);
			return;
		}
	}

	m_editor->SetPosition({ m_Where.left, m_Where.top + (IsTitleBarVisible()? 1 : 0), m_Where.right, m_Where.bottom - (IsKeyBarVisible()? 1 : 0) });
	m_editor->SetStartPos(StartLine,StartChar);
	SetDeleteOnClose(DeleteOnClose);
	int UserBreak = 0;

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
	while (BlankFileName || !LoadFile(strFullFileName, UserBreak, ErrorState))
	{
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

		break;
	}

	if (GetExitCode() == XC_LOADING_INTERRUPTED || GetExitCode() == XC_OPEN_ERROR)
		return;

	if (!m_Flags.Check(FFILEEDIT_DISABLEHISTORY) && !strFileName.empty())
		Global->CtrlObject->ViewHistory->AddToHistory(strFullFileName, m_editor->m_Flags.Check(Editor::FEDITOR_LOCKMODE)? HR_EDITOR_RO : HR_EDITOR);

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

	bLoaded = true;

	m_F4Timer = std::make_unique<f4_key_timer>();

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
		ShowStatus();
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
		MacroEditState |= WasFileSaved()?                                                 6_bit : 0;
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
	elevation::instance().ResetApprove();

	return ReProcessKey(Key, false);
}

bool FileEditor::ReProcessKey(const Manager::Key& Key, bool CalledFromControl)
{
	if (m_F4Timer)
	{
		if (Key() == KEY_F4 && !m_F4Timer->expired())
			return false;

		m_F4Timer.reset();
	}

	const auto LocalKey = Key();

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
			if (!m_Flags.Check(FFILEEDIT_ENABLEF6))
				break; // отдадим F6 плагинам, если есть запрет на переключение

			// If the file is "new", there is nothing to view yet, so we have to save it first.
			const auto NeedSave = m_editor->m_Flags.Check(Editor::FEDITOR_MODIFIED) || m_Flags.Check(FFILEEDIT_NEW);
			bool ConfirmSave = !m_Flags.Check(FFILEEDIT_NEW);

			if (!m_Flags.Check(FFILEEDIT_NEW) && !os::fs::is_file(strFullFileName))
			{
				ConfirmSave = false;

				switch (Message(MSG_WARNING,
					msg(lng::MEditTitle),
					{
						msg(WasFileSaved()? lng::MEditSavedChangedNonFile : lng::MEditSavedChangedNonFile1),
						msg(lng::MEditSavedChangedNonFile2)
					},
					{ lng::MHYes, lng::MHNo },
					{}, &EditorSaveF6DeletedId))
				{
				case message_result::first_button:
					m_editor->m_Flags.Set(Editor::FEDITOR_MODIFIED);
					break;

				default:
					return false;
				}
			}

			if (!ProcessQuitKey(NeedSave, ConfirmSave, false))
				return false;

			const auto delete_on_close =
				m_Flags.Check(FFILEEDIT_DELETEONCLOSE)?
					1 :
					m_Flags.Check(FFILEEDIT_DELETEONLYFILEONCLOSE)?
					2 :
					0;

			SetDeleteOnClose(0);

			FileViewer::create(
				strFullFileName,
				GetCanLoseFocus(),
				m_Flags.Check(FFILEEDIT_DISABLEHISTORY),
				false,
				m_editor->GetCurPos(true, m_bAddSignature),
				{},
				&EditNamesList,
				m_Flags.Check(FFILEEDIT_SAVETOSAVEAS),
				m_codepage,
				strTitle,
				delete_on_close,
				shared_from_this());

			return true;
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
					HideCursor();
					WaitKey();
				}

				Global->WindowManager->RefreshAll();

				return true;
			}
			case KEY_F2:
			case KEY_SHIFTF2:
				return SaveAction(LocalKey == KEY_SHIFTF2);

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

				const auto ActivePanel = Global->CtrlObject->Cp()->ActivePanel();

				if (m_Flags.Check(FFILEEDIT_NEW) || (ActivePanel && ActivePanel->FindFile(strFileName) == -1)) // Mantis#279
				{
					UpdateFileList();
					m_Flags.Clear(FFILEEDIT_NEW);
				}

				{
					SCOPED_ACTION(SaveScreen);
					Global->CtrlObject->Cp()->GoToFile(strFullFileName);
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
				return ProcessQuitKey(true, false, true);

			case KEY_F4:
			case KEY_ESC:
			case KEY_F10:
			{
				bool ConfirmSave = true, NeedSave = m_editor->m_Flags.Check(Editor::FEDITOR_MODIFIED);

				if (!m_Flags.Check(FFILEEDIT_NEW) && !os::fs::is_file(strFullFileName))
				{
					ConfirmSave = false;

					switch (Message(MSG_WARNING,
						msg(lng::MEditTitle),
						{
							msg(WasFileSaved()? lng::MEditSavedChangedNonFile : lng::MEditSavedChangedNonFile1),
							msg(lng::MEditSavedChangedNonFile2)
						},
						{ lng::MHYes, lng::MHNo, lng::MHCancel },
						{}, &EditorSaveExitDeletedId))
					{
					case message_result::first_button:
						m_editor->m_Flags.Set(Editor::FEDITOR_MODIFIED);
						break;

					case message_result::second_button:
						NeedSave = false;
						break;

					default:
						return false;
					}
				}

				return ProcessQuitKey(NeedSave, ConfirmSave, true);
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

			case KEY_CTRLL:
			case KEY_RCTRLL:
				{
					if (m_editor->ProcessKey(Key))
					{
						ShowStatus();
						if (!m_Flags.Check(FFILEEDIT_DISABLEHISTORY))
							Global->CtrlObject->ViewHistory->AddToHistory(strFullFileName, m_editor->m_Flags.Check(Editor::FEDITOR_LOCKMODE)? HR_EDITOR_RO : HR_EDITOR);
					}
				}
				return true;

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
		const os::fs::file EditFile(strFullFileName, FILE_READ_DATA, os::fs::file_share_read, nullptr, OPEN_EXISTING);
		const auto DefaultCodepage = GetDefaultCodePage();
		cp = EditFile? GetFileCodepage(EditFile, DefaultCodepage) : DefaultCodepage;
	}

	if (cp == CP_DEFAULT || !IsCodePageSupported(cp))
	{
		Message(MSG_WARNING,
			msg(lng::MEditTitle),
			{
				far::vformat(msg(lng::MEditorCPNotSupported), cp)
			},
			{ lng::MOk });
		return false;
	}

	if (cp == m_codepage)
		return true;

	const auto CurrentCodepage = m_codepage;

	const auto need_reload = !m_Flags.Check(FFILEEDIT_NEW) // we can't reload non-existing file
		&& (BadConversion
		|| IsUtf16CodePage(m_codepage)
		|| IsUtf16CodePage(cp));

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
				{ lng::MOk, lng::MCancel }) != message_result::first_button)
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

static std::optional<bool> confirm_save()
{
	std::vector Buttons{ lng::MHYes, lng::MHNo };

	if (Global->AllowCancelExit)
		Buttons.emplace_back(lng::MHCancel);

	auto Code = Message(MSG_WARNING,
		msg(lng::MEditTitle),
		{
			msg(lng::MEditAskSave)
		},
		Buttons,
		{}, &EditAskSaveId);

	if (Code == message_result::cancelled && !Global->AllowCancelExit)
		Code = message_result::second_button; // close == not save

	switch (Code)
	{
	case message_result::first_button: // Save
		return true;

	case message_result::second_button: // Not Save
		return false;

	default:
		return {};
	}
}

bool FileEditor::ProcessQuitKey(bool const NeedSave, bool ConfirmSave, bool const DeleteWindow)
{
	if (NeedSave)
	{
		bool TrySave = true;

		if (ConfirmSave)
		{
			if (const auto Save = confirm_save())
				TrySave = *Save;
			else
				return false;
		}

		if (TrySave)
		{
			if (!SaveAction(false))
				return false;

			if (os::fs::exists(strFullFileName))
				UpdateFileList();
		}
		else
		{
			m_editor->TextChanged(false);
		}
	}

	if (DeleteWindow)
		Global->WindowManager->DeleteWindow();

	return true;
}

bool FileEditor::LoadFile(const string_view Name, int& UserBreak, error_state_ex& ErrorState)
{
	try
	{
	// TODO: indentation
	SCOPED_ACTION(taskbar::indeterminate);
	SCOPED_ACTION(wakeful);

	EditorPosCache pc;
	UserBreak = 0;
	os::fs::file EditFile(Name, FILE_READ_DATA, os::fs::file_share_read | (Global->Opt->EdOpt.EditOpenedForWrite? FILE_SHARE_WRITE : 0), nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
	if(!EditFile)
	{
		ErrorState = os::last_error();
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
						string(Name),
						// Ширина = 8 - это будет... в Kb и выше...
						far::vformat(msg(lng::MEditFileLong), trim(FileSizeToStr(FileSize, 8))),
						far::vformat(msg(lng::MEditFileLong2), trim(FileSizeToStr(Global->Opt->EdOpt.FileSizeLimit, 8))),
						msg(lng::MEditROOpen)
					},
					{ lng::MYes, lng::MNo },
					{}, &EditorFileLongId) != message_result::first_button)
				{
					EditFile.Close();
					SetLastError(ERROR_OPEN_FAILED); //????
					ErrorState = os::last_error();
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
					string(Name),
					msg(lng::MEditFileGetSizeError),
					msg(lng::MEditROOpen)
				},
				{ lng::MYes, lng::MNo },
				{}, &EditorFileGetSizeErrorId) != message_result::first_button)
			{
				EditFile.Close();
				SetLastError(ERROR_OPEN_FAILED); //????
				ErrorState = os::last_error();
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

		if (Cached && pc.CodePage && !IsCodePageSupported(pc.CodePage))
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
				if (IsCodePageSupported(Cp))
					m_codepage = Cp;
			}

			if (m_codepage == CP_DEFAULT)
				m_codepage = GetDefaultCodePage();
		}

		m_editor->GlobalEOL = eol::none;

		unsigned long long FileSize = 0;
		// BUGBUG check result
		if (!EditFile.GetSize(FileSize))
		{
			LOGWARNING(L"GetSize({}): {}"sv, EditFile.GetName(), os::last_error());
		}

		std::optional<single_progress> Progress;

		const time_check TimeCheck;

		os::fs::filebuf StreamBuffer(EditFile, std::ios::in);
		std::istream Stream(&StreamBuffer);
		Stream.exceptions(Stream.badbit | Stream.failbit);

		enum_lines EnumFileLines(Stream, m_codepage);
		for (auto Str: EnumFileLines)
		{
			if (testBOM && IsUtfCodePage(m_codepage))
			{
				if (Str.Str.starts_with(encoding::bom_char))
				{
					Str.Str.remove_prefix(1);
					m_bAddSignature = true;
				}
			}
			testBOM = false;

			if (TimeCheck)
			{
				if (CheckForEscAndConfirmAbort())
				{
					UserBreak = 1;
					EditFile.Close();
					return false;
				}

				if (!Progress)
					Progress.emplace(msg(lng::MEditTitle), far::vformat(msg(lng::MEditReading), Name), 0);

				HideCursor();
				const auto CurPos = EditFile.GetPointer();
				auto Percent = ToPercent(CurPos, FileSize);
				// В случае если во время загрузки файл увеличивается размере, то количество
				// процентов может быть больше 100. Обрабатываем эту ситуацию.
				if (Percent > 100)
				{
					// BUGBUG check result
					if (!EditFile.GetSize(FileSize))
					{
						LOGWARNING(L"GetSize({}): {}"sv, EditFile.GetName(), os::last_error());
					}

					Percent = FileSize? std::min(ToPercent(CurPos, FileSize), 100u) : 100;
				}

				Progress->update(Percent);
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
				ErrorState = os::last_error();
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
	ErrorState = os::last_error();
	// BUGBUG check result
	if (os::fs::get_find_data(Name, FileInfo))
		FileInfo.FileName = Name;
	else
		LOGWARNING(L"get_find_data({}): {}"sv, Name, os::last_error());

	EditorGetFileAttributes(Name);
	return true;

	}
	catch (std::bad_alloc const&)
	{
		// TODO: better diagnostics
		m_editor->FreeAllocatedData();
		m_Flags.Set(FFILEEDIT_OPENFAILED);
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		ErrorState = os::last_error();
		return false;
	}
	catch (std::exception const&)
	{
		// A portion of file can be locked

		// TODO: better diagnostics
		m_editor->FreeAllocatedData();
		m_Flags.Set(FFILEEDIT_OPENFAILED);
		ErrorState = os::last_error();
		return false;
	}
}

bool FileEditor::ReloadFile(uintptr_t codepage)
{
	const auto save_codepage(m_codepage);
	const auto save_bAddSignature(m_bAddSignature);
	const auto save_BadConversiom(BadConversion);
	const auto save_Flags(m_Flags), save_Flags1(m_editor->m_Flags);

	Editor saved(shared_from_this());
	saved.fake_editor = true;
	m_editor->SwapState(saved);

	int user_break = 0;
	m_codepage = codepage;
	error_state_ex ErrorState;

	for (;;)
	{
		if (LoadFile(strFullFileName, user_break, ErrorState))
		{
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
int FileEditor::SaveFile(const string_view Name, bool bSaveAs, error_state_ex& ErrorState, eol Eol, uintptr_t Codepage, bool AddSignature)
{
	if (!bSaveAs)
	{
		Eol = eol::none;
		Codepage=m_editor->GetCodePage();
	}

	SCOPED_ACTION(taskbar::indeterminate);
	SCOPED_ACTION(wakeful);

	const auto FileAttr = os::fs::get_file_attributes(Name);
	if (FileAttr != INVALID_FILE_ATTRIBUTES)
	{
		if (os::fs::find_data FInfo; equal_icase(Name, FileInfo.FileName) && os::fs::get_find_data(Name, FInfo))
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
					case message_result::first_button: // Save
						break;

					case message_result::second_button: // Save as
						return SaveAction(true)?
							SAVEFILE_SUCCESS :
							SAVEFILE_CANCEL;

					default:
						return SAVEFILE_CANCEL;
					}
				}
		}

		if (FileAttr & FILE_ATTRIBUTE_READONLY)
		{
			//BUGBUG
			if (Message(MSG_WARNING,
				msg(lng::MEditTitle),
				{
					string(Name),
					msg(lng::MEditRO),
					msg(lng::MEditOvr)
				},
				{ lng::MYes, lng::MNo },
				{}, &EditorSavedROId) != message_result::first_button)
				return SAVEFILE_CANCEL;

			if (!os::fs::set_file_attributes(Name, FileAttr & ~FILE_ATTRIBUTE_READONLY)) //BUGBUG
			{
				LOGWARNING(L"set_file_attributes({}): {}"sv, Name, os::last_error());
			}
		}
	}
	else
	{
		// проверим путь к файлу, может его уже снесли...
		auto CreatedPath = Name;
		if (CutToParent(CreatedPath))
		{
			if (!os::fs::exists(CreatedPath))
			{
				// и попробуем создать.
				// Раз уж
				CreatePath(CreatedPath);
				if (!os::fs::exists(CreatedPath))
				{
					ErrorState = os::last_error();
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
			{ lng::MEditorSave, lng::MCancel }) == message_result::first_button)
		{
			BadConversion = false;
		}
		else
		{
			return SAVEFILE_CANCEL;
		}
	}

	if (Eol != eol::none)
	{
		m_editor->GlobalEOL = Eol;
	}

	if (!os::fs::exists(Name))
		m_Flags.Set(FFILEEDIT_NEW);

	//SaveScreen SaveScr;
	/* $ 11.10.2001 IS
		Если было произведено сохранение с любым результатом, то не удалять файл
	*/
	m_Flags.Clear(FFILEEDIT_DELETEONCLOSE|FFILEEDIT_DELETEONLYFILEONCLOSE);

	if (!IsUtfCodePage(Codepage))
	{
		int LineNumber=-1;
		encoding::diagnostics Diagnostics;

		for(auto& Line: m_editor->Lines)
		{
			++LineNumber;
			const auto& SaveStr = Line.GetString();
			auto LineEol = Line.GetEOL();

			(void)encoding::get_bytes_count(Codepage, SaveStr, &Diagnostics);
			const auto ValidStr = !Diagnostics.ErrorPosition;
			if (ValidStr)
				(void)encoding::get_bytes_count(Codepage, LineEol.str(), &Diagnostics);

			if (Diagnostics.ErrorPosition)
			{
				//SetMessageHelp(L"EditorDataLostWarning")
				const auto Result = Message(MSG_WARNING,
					msg(lng::MWarning),
					{
						codepages::UnsupportedCharacterMessage(SaveStr[*Diagnostics.ErrorPosition]),
						codepages::FormatName(Codepage),
						msg(lng::MEditorSaveNotRecommended)
					},
					{ lng::MEditorSaveCPWarnShow, lng::MEditorSave, lng::MCancel });

				if (Result == message_result::second_button)
					break;

				if(Result == message_result::first_button)
				{
					m_editor->GoToLine(LineNumber);
					if(!ValidStr)
					{
						Line.SetCurPos(static_cast<int>(*Diagnostics.ErrorPosition));
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

	const string NameForPlugin(Name);
	EditorSaveFile esf{ sizeof(esf), NameForPlugin.c_str(), m_editor->GlobalEOL.str().data(), Codepage };
	Global->CtrlObject->Plugins->ProcessEditorEvent(EE_SAVE, &esf, m_editor.get());

	try
	{
		save_file_with_replace(Name, FileAttr, 0, Global->Opt->EdOpt.CreateBackups, [&](std::ostream& Stream)
		{
			m_editor->UndoSavePos = m_editor->UndoPos;
			m_editor->m_Flags.Clear(Editor::FEDITOR_UNDOSAVEPOSLOST);

			HideCursor();

			if (!bSaveAs)
				AddSignature = m_bAddSignature;

			std::optional<single_progress> Progress;
			const time_check TimeCheck;

			// We've already validated the codepage above
			encoding::writer Writer(Stream, Codepage, AddSignature, true);

			size_t LineNumber = -1;

			for (auto& Line : m_editor->Lines)
			{
				++LineNumber;

				if (TimeCheck)
				{
					if (!Progress)
						Progress.emplace(msg(lng::MEditTitle), far::vformat(msg(lng::MEditSaving), Name), 0);

					Progress->update(ToPercent(LineNumber, m_editor->Lines.size()));
				}

				const auto& SaveStr = Line.GetString();
				auto LineEol = Line.GetEOL();

				if (Eol != eol::none && LineEol != eol::none)
				{
					LineEol = m_editor->GlobalEOL;
					Line.SetEOL(LineEol);
				}

				Writer.write(SaveStr, LineEol.str());
			}
		});

		m_editor->TextChanged(false);
		m_editor->m_Flags.Set(Editor::FEDITOR_NEWUNDO);
		m_Flags.Set(FFILEEDIT_WAS_SAVED);
		m_Flags.Clear(FFILEEDIT_NEW);

		// BUGBUG check result
		if (os::fs::get_find_data(Name, FileInfo))
			FileInfo.FileName = Name;
		else
			LOGWARNING(L"get_find_data({}): {}"sv, Name, os::last_error());

		EditorGetFileAttributes(Name);
		Show();
		return SAVEFILE_SUCCESS;
	}
	catch (far_exception const& e)
	{
		ErrorState = e;
		Show();
		return SAVEFILE_ERROR;
	}
}

static auto parent_directory(string_view const FileName)
{
	auto Path = FileName;
	CutToParent(Path);
	return Path;
}

bool FileEditor::SaveAction(bool const SaveAsIntention)
{
	const auto ParentDirectory = parent_directory(strFullFileName);

	const auto SaveAs =
		SaveAsIntention ||
		m_Flags.Check(FFILEEDIT_SAVETOSAVEAS) ||
		!os::fs::is_directory(ParentDirectory);

	for (;;)
	{
		auto Eol = m_SaveEol;
		auto Codepage = m_codepage;
		auto AddSignature = m_bAddSignature;
		auto strFullSaveAsName = strFullFileName;

		if (SaveAs)
		{
			auto strSaveAsName = equal_icase(ParentDirectory, os::fs::get_current_directory())?
				strFileName :
				strFullFileName;

			if (strSaveAsName.empty())
				strSaveAsName = PointToName(strFullFileName);

			if (!dlgSaveFileAs(strSaveAsName, Eol, Codepage, AddSignature))
				return false;

			strSaveAsName = unquote(os::env::expand(strSaveAsName));
			strFullSaveAsName = ConvertNameToFull(strSaveAsName);

			if (!equal_icase(strFullSaveAsName, strFullFileName) && os::fs::exists(strFullSaveAsName) && !AskOverwrite(strSaveAsName))
			{
				return false;
			}
		}

		error_state_ex ErrorState;
		switch (SaveFile(strFullSaveAsName, SaveAs, ErrorState, Eol, Codepage, AddSignature))
		{
		case SAVEFILE_SUCCESS:
			m_Flags.Clear(FFILEEDIT_SAVETOSAVEAS);
			m_codepage = Codepage;
			m_SaveEol = Eol;
			m_bAddSignature = AddSignature;
			SetFileName(strFullSaveAsName);

			// перерисовывать надо как минимум когда изменилась кодировка или имя файла
			ShowConsoleTitle();
			Show();//!!! BUGBUG
			return true;

		case SAVEFILE_ERROR:
			if (OperationFailed(ErrorState, strFullFileName, lng::MEditTitle, msg(lng::MEditCannotSave), false) != operation::retry)
				return false;
			continue;

		case SAVEFILE_CANCEL:
			return false;

		default:
			std::unreachable();
		}
	}
}

bool FileEditor::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	elevation::instance().ResetApprove();

	if (!m_windowKeyBar->ProcessMouse(MouseEvent))
	{
		INPUT_RECORD mouse{ MOUSE_EVENT };
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
	if (Global->CtrlObject && !m_Flags.Check(FFILEEDIT_DISABLEHISTORY) && !strFileName.empty())
		Global->CtrlObject->ViewHistory->AddToHistory(strFullFileName, m_editor->m_Flags.Check(Editor::FEDITOR_LOCKMODE)? HR_EDITOR_RO : HR_EDITOR);

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
	if (DynamicMode && m_editor->IsModified())
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

void FileEditor::SetFileName(const string_view NewFileName)
{
	if (!NewFileName.empty())
	{
		strFullFileName = path::normalize_separators(ConvertNameToFull(NewFileName));
		strFileName = PointToName(strFullFileName);
		return;
	}

	assert(strFileName.empty());

	const auto LocalTime = os::chrono::now_local();

	strFullFileName = path::join(
		strStartDir,
		far::vformat(
			msg(lng::MNewFileName),
			LocalTime.wYear,
			LocalTime.wMonth,
			LocalTime.wDay,
			LocalTime.wHour,
			LocalTime.wMinute,
			LocalTime.wSecond,
			LocalTime.wMilliseconds
		)
	);
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

static std::pair<string, size_t> char_code(std::optional<char32_t> const& Char, int const Codebase)
{
	const auto process = [&](const auto Format, string_view const Max)
	{
		auto Result = std::pair{ Char.has_value()? far::format(Format, static_cast<uint32_t>(*Char)) : L""s, Max.size()};
		Result.second = std::max(Result.first.size(), Result.second);
		return Result;
	};

	switch (Codebase)
	{
	case 0:
		return process(FSTR(L"0{:o}"sv), L"0177777"sv);

	case 2:
		return process(FSTR(L"{:X}h"sv), L"FFFFh"sv);

	case 1:
	default:
		return process(FSTR(L"{}"sv), L"65535"sv);
	}
}

static std::pair<string, size_t> ansi_char_code(std::optional<char32_t> const& Char, int const Codebase, uintptr_t const Codepage)
{
	const auto process = [&](const auto Format, string_view const Max)
	{
		std::optional<unsigned> CharCode;

		char Buffer;
		encoding::diagnostics Diagnostics;
		if (Char.has_value() && *Char <= std::numeric_limits<char16_t>::max())
		{
			const auto Ch = static_cast<wchar_t>(*Char);
			if (encoding::get_bytes(Codepage, { &Ch, 1 }, { &Buffer, 1 }, &Diagnostics) == 1 && !Diagnostics.ErrorPosition)
			{
				const unsigned AnsiCode = Buffer;
				if (AnsiCode != *Char)
				{
					CharCode = AnsiCode;
				}
			}
		}

		return std::pair{ CharCode.has_value()? far::format(Format, *CharCode) : L""s, Max.size() };
	};

	switch (Codebase)
	{
	case 0:
		return process(FSTR(L"0{:<3o}"sv), L"0377"sv);

	case 2:
		return process(FSTR(L"{:02X}h"sv), L"FFh"sv);

	case 1:
	default:
		return process(FSTR(L"{:<3}"sv), L"255"sv);
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
		std::optional<char32_t> Char;

		if (CurPos + 1 < Str.size() && is_valid_surrogate_pair(Str[CurPos], Str[CurPos + 1]))
			Char = encoding::utf16::extract_codepoint(Str[CurPos], Str[CurPos + 1]);
		else if (CurPos < Str.size())
			Char = Str[CurPos];

		auto [UnicodeStr, UnicodeSize] = char_code(Char, m_editor->EdOpt.CharCodeBase);
		CharCode = std::move(UnicodeStr);

		if (!IsUtfCodePage(m_codepage))
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
	const auto LinesFormat = FSTR(L"{}/{}"sv);
	const auto SizeLineStr = far::format(LinesFormat, m_editor->Lines.size(), m_editor->Lines.size()).size();
	const auto strLineStr = far::format(LinesFormat, m_editor->m_it_CurLine.Number() + 1, m_editor->Lines.size());
	const auto strAttr = *AttrStr? L"│"s + AttrStr : L""s;
	auto StatusLine = far::format(FSTR(L"│{}{}│{:5.5}│{:.3} {:>{}}│{:.3} {:<3}│{:.3} {:<3}{}│{}"sv),
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
	const int ClockSize = Global->Opt->Clock && m_Flags.Check(FFILEEDIT_FULLSCREEN)? static_cast<int>(Global->CurrentTime.size()) : 0;
	const int AvailableSpace = std::max(0, ObjWidth() - ClockSize - (ClockSize? 1 : 0));
	inplace::cut_right(StatusLine, AvailableSpace);
	const int NameWidth = std::max(0, AvailableSpace - static_cast<int>(StatusLine.size()));

	Text(fit_to_left(truncate_path(GetTitle(), NameWidth), NameWidth));
	Text(StatusLine);

	if (ClockSize)
		Text(L'│'); // Separator before the clock
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
		ActivePanel->Update(UPDATE_KEEP_SELECTION);
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
			return m_editor->DeleteSessionBookmark(m_editor->PointerToSessionBookmark(static_cast<int>(std::bit_cast<intptr_t>(Param2))));
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
				if (std::bit_cast<intptr_t>(Param2) != -1) // не только перерисовать?
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
			auto strName = strFullFileName;
			auto Eol = m_SaveEol;
			auto Codepage = m_codepage;

			const auto esf = static_cast<const EditorSaveFile*>(Param2);
			if (CheckStructSize(esf))
			{
				if (esf->FileName)
					strName = ConvertNameToFull(esf->FileName);

				if (esf->FileEOL)
					Eol = eol::parse(esf->FileEOL);

				if (esf->CodePage != CP_DEFAULT)
					Codepage = esf->CodePage;
			}

			if (!equal_icase(strName, strFullFileName) && os::fs::exists(strName) && !AskOverwrite(PointToName(strName)))
				return FALSE;

			//всегда записываем в режиме save as - иначе не сменить кодировку и концы линий.
			error_state_ex ErrorState;
			if (SaveFile(strName, true, ErrorState, Eol, Codepage, m_bAddSignature) != SAVEFILE_SUCCESS)
				return FALSE;

			SetFileName(strName);
			m_SaveEol = Eol;
			m_codepage = Codepage;
			return TRUE;
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

				return TRUE;
			}

			return FALSE;
		}
		case ECTL_PROCESSINPUT:
		{
			if (Param2)
			{
				const auto& rec = *static_cast<const INPUT_RECORD*>(Param2);

				if (ProcessEditorInput(rec))
					return TRUE;

				if (rec.EventType==MOUSE_EVENT)
					ProcessMouse(&rec.Event.MouseEvent);
				else
				{
					const auto Key = InputRecordToKey(&rec);
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
					if(IsUtfCodePage(m_codepage))
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

uintptr_t FileEditor::GetCodePage() const
{
	return m_codepage;
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
		strCacheName = path::normalize_separators(strFullFileName);
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
	wchar_t ErrorChar;
	if (!m_editor->TryCodePage(m_codepage, codepage, ErrorCodepage, ErrorLine, ErrorPos, ErrorChar))
	{
		switch (Message(MSG_WARNING,
			msg(lng::MWarning),
			{
				codepages::UnsupportedCharacterMessage(ErrorChar),
				codepages::FormatName(ErrorCodepage),
				msg(lng::MEditorSwitchCPConfirm)
			},
			{ lng::MCancel, lng::MEditorSaveCPWarnShow, lng::MOk })
		)
		{
		default:
		case message_result::first_button:
			return false;

		case message_result::second_button:
			m_editor->GoToLine(static_cast<int>(ErrorLine));
			m_editor->m_it_CurLine->SetCurPos(static_cast<int>(ErrorPos));
			Show();
			return false;

		case message_result::third_button:
			break;
		}
	}

	BadConversion = !m_editor->SetCodePage(m_codepage, codepage, &m_bAddSignature);
	m_codepage = codepage;
	ShowStatus();
	return true;
}

bool FileEditor::AskOverwrite(const string_view FileName)
{
	return Message(MSG_WARNING,
		msg(lng::MEditTitle),
		{
			string(FileName),
			msg(lng::MEditExists),
			msg(lng::MEditOvr)
		},
		{ lng::MYes, lng::MNo },
		{}, &EditorAskOverwriteId) == message_result::first_button;
}

uintptr_t FileEditor::GetDefaultCodePage()
{
	const auto cp = encoding::codepage::normalise(Global->Opt->EdOpt.DefaultCodePage);
	return cp == CP_DEFAULT || !IsCodePageSupported(cp)?
		encoding::codepage::ansi() :
		cp;
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
