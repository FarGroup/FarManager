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


intptr_t WINAPI hndOpenEditor(HANDLE hDlg, int msg, int param1, void* param2)
{
	if (msg == DN_INITDIALOG)
	{
		int codepage = *(UINT*)param2;
		FillCodePagesList(hDlg, ID_OE_CODEPAGE, codepage, true, false, true, true);
	}

	if (msg == DN_CLOSE)
	{
		if (param1 == ID_OE_OK)
		{
			UINT* param = (UINT*)SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
			FarListPos pos={sizeof(FarListPos)};
			SendDlgMessage(hDlg, DM_LISTGETCURPOS, ID_OE_CODEPAGE, &pos);
			*param = *(UINT*)SendDlgMessage(hDlg, DM_LISTGETDATA, ID_OE_CODEPAGE, ToPtr(pos.SelectPos));
			return TRUE;
		}
	}

	return DefDlgProc(hDlg, msg, param1, param2);
}

bool dlgOpenEditor(string &strFileName, UINT &codepage)
{
	const wchar_t *HistoryName=L"NewEdit";
	FarDialogItem EditDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,72,8,0,nullptr,nullptr,0,MSG(MEditTitle)},
		{DI_TEXT,     5,2, 0,2,0,nullptr,nullptr,0,MSG(MEditOpenCreateLabel)},
		{DI_EDIT,     5,3,70,3,0,HistoryName,nullptr,DIF_FOCUS|DIF_HISTORY|DIF_USELASTHISTORY|DIF_EDITEXPAND|DIF_EDITPATH,L""},
		{DI_TEXT,     3,4, 0,4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_TEXT,     5,5, 0,5,0,nullptr,nullptr,0,MSG(MEditCodePage)},
		{DI_COMBOBOX,25,5,70,5,0,nullptr,nullptr,DIF_DROPDOWNLIST|DIF_LISTWRAPMODE|DIF_LISTAUTOHIGHLIGHT,L""},
		{DI_TEXT,     3,6, 0,6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,   0,7, 0,7,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,   0,7, 0,7,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
	};
	MakeDialogItemsEx(EditDlgData,EditDlg);
	Dialog Dlg(EditDlg, ARRAYSIZE(EditDlg), hndOpenEditor, &codepage);
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

intptr_t WINAPI hndSaveFileAs(HANDLE hDlg, int msg, int param1, void* param2)
{
	static UINT codepage=0;

	switch (msg)
	{
		case DN_INITDIALOG:
		{
			codepage=*(UINT*)SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
			FillCodePagesList(hDlg, ID_SF_CODEPAGE, codepage, false, false, false, true);

			if (IsUnicodeOrUtfCodePage(codepage))
			{
				SendDlgMessage(hDlg,DM_ENABLE,ID_SF_SIGNATURE,ToPtr(TRUE));
			}
			else
			{
				SendDlgMessage(hDlg,DM_SETCHECK,ID_SF_SIGNATURE,ToPtr(BSTATE_UNCHECKED));
				SendDlgMessage(hDlg,DM_ENABLE,ID_SF_SIGNATURE,FALSE);
			}

			break;
		}
		case DN_CLOSE:
		{
			if (param1 == ID_SF_OK)
			{
				UINT *codepage = (UINT*)SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
				FarListPos pos={sizeof(FarListPos)};
				SendDlgMessage(hDlg, DM_LISTGETCURPOS, ID_SF_CODEPAGE, &pos);
				*codepage = *(UINT*)SendDlgMessage(hDlg, DM_LISTGETDATA, ID_SF_CODEPAGE, ToPtr(pos.SelectPos));
				return TRUE;
			}

			break;
		}
		case DN_EDITCHANGE:
		{
			if (param1==ID_SF_CODEPAGE)
			{
				FarListPos pos={sizeof(FarListPos)};
				SendDlgMessage(hDlg,DM_LISTGETCURPOS,ID_SF_CODEPAGE,&pos);
				UINT Cp=*reinterpret_cast<UINT*>(SendDlgMessage(hDlg,DM_LISTGETDATA,ID_SF_CODEPAGE,ToPtr(pos.SelectPos)));

				if (Cp!=codepage)
				{
					codepage=Cp;

					if (IsUnicodeOrUtfCodePage(codepage))
					{
						SendDlgMessage(hDlg,DM_SETCHECK,ID_SF_SIGNATURE,ToPtr(BSTATE_CHECKED));
						SendDlgMessage(hDlg,DM_ENABLE,ID_SF_SIGNATURE,ToPtr(TRUE));
					}
					else
					{
						SendDlgMessage(hDlg,DM_SETCHECK,ID_SF_SIGNATURE,ToPtr(BSTATE_UNCHECKED));
						SendDlgMessage(hDlg,DM_ENABLE,ID_SF_SIGNATURE,FALSE);
					}

					return TRUE;
				}
			}

			break;
		}
	default:
		break;
	}

	return DefDlgProc(hDlg, msg, param1, param2);
}



bool dlgSaveFileAs(string &strFileName, int &TextFormat, UINT &codepage,bool &AddSignature)
{
	const wchar_t *HistoryName=L"NewEdit";
	FarDialogItem EditDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,72,15,0,nullptr,nullptr,0,MSG(MEditTitle)},
		{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,MSG(MEditSaveAs)},
		{DI_EDIT,5,3,70,3,0,HistoryName,nullptr,DIF_FOCUS|DIF_HISTORY|DIF_EDITEXPAND|DIF_EDITPATH,L""},
		{DI_TEXT,3,4,0,4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_TEXT,5,5,0,5,0,nullptr,nullptr,0,MSG(MEditCodePage)},
		{DI_COMBOBOX,25,5,70,5,0,nullptr,nullptr,DIF_DROPDOWNLIST|DIF_LISTWRAPMODE|DIF_LISTAUTOHIGHLIGHT,L""},
		{DI_CHECKBOX,5,6,0,6,AddSignature,nullptr,nullptr,DIF_DISABLE,MSG(MEditAddSignature)},
		{DI_TEXT,3,7,0,7,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_TEXT,5,8,0,8,0,nullptr,nullptr,0,MSG(MEditSaveAsFormatTitle)},
		{DI_RADIOBUTTON,5,9,0,9,0,nullptr,nullptr,DIF_GROUP,MSG(MEditSaveOriginal)},
		{DI_RADIOBUTTON,5,10,0,10,0,nullptr,nullptr,0,MSG(MEditSaveDOS)},
		{DI_RADIOBUTTON,5,11,0,11,0,nullptr,nullptr,0,MSG(MEditSaveUnix)},
		{DI_RADIOBUTTON,5,12,0,12,0,nullptr,nullptr,0,MSG(MEditSaveMac)},
		{DI_TEXT,3,13,0,13,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,0,14,0,14,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,0,14,0,14,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
	};
	MakeDialogItemsEx(EditDlgData,EditDlg);
	EditDlg[ID_SF_FILENAME].strData = (/*Flags.Check(FFILEEDIT_SAVETOSAVEAS)?strFullFileName:strFileName*/strFileName);
	{
		size_t pos=0;
		if (EditDlg[ID_SF_FILENAME].strData.Pos(pos,MSG(MNewFileName)))
			EditDlg[ID_SF_FILENAME].strData.SetLength(pos);
	}
	EditDlg[ID_SF_DONOTCHANGE+TextFormat].Selected = TRUE;
	Dialog Dlg(EditDlg, ARRAYSIZE(EditDlg), hndSaveFileAs, &codepage);
	Dlg.SetPosition(-1,-1,76,17);
	Dlg.SetHelp(L"FileSaveAs");
	Dlg.SetId(FileSaveAsId);
	Dlg.Process();

	if ((Dlg.GetExitCode() == ID_SF_OK) && !EditDlg[ID_SF_FILENAME].strData.IsEmpty())
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


const FileEditor *FileEditor::CurrentEditor = nullptr;

FileEditor::FileEditor(const string& Name, UINT codepage, DWORD InitFlags, int StartLine, int StartChar, const string* PluginData, int OpenModeExstFile):
	BadConversion(false)
{
	ScreenObject::SetPosition(0,0,ScrX,ScrY);
	Flags.Set(InitFlags);
	Flags.Set(FFILEEDIT_FULLSCREEN);
	Init(Name,codepage, nullptr, InitFlags, StartLine, StartChar, PluginData, FALSE, OpenModeExstFile);
}


FileEditor::FileEditor(const string& Name, UINT codepage, DWORD InitFlags, int StartLine, int StartChar, const string* Title, int X1, int Y1, int X2, int Y2, int DeleteOnClose, int OpenModeExstFile):
	BadConversion(false)
{
	Flags.Set(InitFlags);

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

	ScreenObject::SetPosition(X1,Y1,X2,Y2);
	Flags.Change(FFILEEDIT_FULLSCREEN,(!X1 && !Y1 && X2==ScrX && Y2==ScrY));
	string EmptyTitle;
	Init(Name,codepage, Title, InitFlags, StartLine, StartChar, &EmptyTitle, DeleteOnClose, OpenModeExstFile);
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
	//AY: флаг оповещающий закрытие редактора.
	m_bClosing = true;

	if (!Flags.Check(FFILEEDIT_DISABLESAVEPOS) && m_editor->EdOpt.SavePos && CtrlObject)
		SaveToCache();

	BitFlags FEditFlags=m_editor->Flags;
	int FEditEditorID=m_editor->EditorID;

	if (bEE_READ_Sent && CtrlObject)
	{
		FileEditor *save = CtrlObject->Plugins->CurEditor;
		CtrlObject->Plugins->CurEditor=this;
		CtrlObject->Plugins->ProcessEditorEvent(EE_CLOSE,nullptr,FEditEditorID);
		CtrlObject->Plugins->CurEditor = save;
	}

	if (!Flags.Check(FFILEEDIT_OPENFAILED))
	{
		/* $ 11.10.2001 IS
		   Удалим файл вместе с каталогом, если это просится и файла с таким же
		   именем не открыто в других фреймах.
		*/
		/* $ 14.06.2001 IS
		   Если установлен FEDITOR_DELETEONLYFILEONCLOSE и сброшен
		   FEDITOR_DELETEONCLOSE, то удаляем только файл.
		*/
		if (Flags.Check(FFILEEDIT_DELETEONCLOSE|FFILEEDIT_DELETEONLYFILEONCLOSE) &&
		        !FrameManager->CountFramesWithName(strFullFileName))
		{
			if (Flags.Check(FFILEEDIT_DELETEONCLOSE))
				DeleteFileWithFolder(strFullFileName);
			else
			{
				apiSetFileAttributes(strFullFileName,FILE_ATTRIBUTE_NORMAL);
				apiDeleteFile(strFullFileName); //BUGBUG
			}
		}
	}

	if (m_editor)
		delete m_editor;

	m_editor=nullptr;
	CurrentEditor=nullptr;

	if (EditNamesList)
		delete EditNamesList;
}

void FileEditor::Init(
    const string& Name,
    UINT codepage,
    const string* Title,
    DWORD InitFlags,
    int StartLine,
    int StartChar,
    const string* PluginData,
    int DeleteOnClose,
    int OpenModeExstFile
)
{
	class SmartLock
	{
		private:
			Editor *editor;
		public:
			SmartLock() {editor=nullptr;};
			~SmartLock() {if (editor) editor->Unlock();};

			void Set(Editor *e) {editor=e; editor->Lock();};
	};
	SmartLock __smartlock;
	SysErrorCode=0;
	int BlankFileName=!StrCmp(Name,MSG(MNewFileName)) || !*Name;
	//AY: флаг оповещающий закрытие редактора.
	m_bClosing = false;
	bEE_READ_Sent = false;
	m_bAddSignature = false;
	m_editor = new Editor;
	__smartlock.Set(m_editor);

	if (!m_editor)
	{
		ExitCode=XC_OPEN_ERROR;
		return;
	}

	m_codepage = codepage;
	m_editor->SetOwner(this);
	m_editor->SetCodePage(m_codepage);
	*AttrStr=0;
	CurrentEditor=this;
	FileAttributes=INVALID_FILE_ATTRIBUTES;
	FileAttributesModified=false;
	SetTitle(Title);
	EditNamesList = nullptr;
	KeyBarVisible = Opt.EdOpt.ShowKeyBar;
	TitleBarVisible = Opt.EdOpt.ShowTitleBar;
	// $ 17.08.2001 KM - Добавлено для поиска по AltF7. При редактировании найденного файла из архива для клавиши F2 сделать вызов ShiftF2.
	Flags.Change(FFILEEDIT_SAVETOSAVEAS,(BlankFileName?TRUE:FALSE));

	if (BlankFileName && !Flags.Check(FFILEEDIT_CANNEWFILE))
	{
		ExitCode=XC_OPEN_ERROR;
		return;
	}

	SetPluginData(PluginData);
	m_editor->SetHostFileEditor(this);
	SetCanLoseFocus(Flags.Check(FFILEEDIT_ENABLEF6));
	apiGetCurrentDirectory(strStartDir);

	if (!SetFileName(Name))
	{
		ExitCode=XC_OPEN_ERROR;
		return;
	}

	//int FramePos=FrameManager->FindFrameByFile(MODALTYPE_EDITOR,FullFileName);
	//if (FramePos!=-1)
	if (Flags.Check(FFILEEDIT_ENABLEF6))
	{
		//if (Flags.Check(FFILEEDIT_ENABLEF6))
		int FramePos=FrameManager->FindFrameByFile(MODALTYPE_EDITOR, strFullFileName);

		if (FramePos!=-1)
		{
			int SwitchTo=FALSE;

			if (!(*FrameManager)[FramePos]->GetCanLoseFocus(TRUE) ||
			        Opt.Confirm.AllowReedit)
			{
				int MsgCode=0;
				if (OpenModeExstFile == FEOPMODE_QUERY)
				{
					SetMessageHelp(L"EditorReload");
					MsgCode=Message(0,3,MSG(MEditTitle),
					                strFullFileName,
					                MSG(MAskReload),
					                MSG(MCurrent),MSG(MNewOpen),MSG(MReload));
				}
				else
				{
					MsgCode=(OpenModeExstFile==FEOPMODE_USEEXISTING)?0:
					        (OpenModeExstFile==FEOPMODE_NEWIFOPEN?1:
					         (OpenModeExstFile==FEOPMODE_RELOAD?2:-100)
					        );
				}

				switch (MsgCode)
				{
					case 0:         // Current
						SwitchTo=TRUE;
						FrameManager->DeleteFrame(this); //???
						break;
					case 1:         // NewOpen
						SwitchTo=FALSE;
						break;
					case 2:         // Reload
					{
						FrameManager->DeleteFrame(FramePos);
						Frame *deleted_frame = (*FrameManager)[FramePos];
						if ( deleted_frame )
							deleted_frame->SetFlags(FFILEEDIT_DISABLESAVEPOS);
						SetExitCode(-2);
						break;
					}
					case -100:
						//FrameManager->DeleteFrame(this);  //???
						SetExitCode(XC_EXISTS);
						return;
					default:
						FrameManager->DeleteFrame(this);  //???
						SetExitCode(MsgCode == -100?XC_EXISTS:XC_QUIT);
						return;
				}
			}
			else
			{
				SwitchTo=TRUE;
			}

			if (SwitchTo)
			{
				FrameManager->ActivateFrame(FramePos);
				//FrameManager->PluginCommit();
				SetExitCode((OpenModeExstFile != FEOPMODE_QUERY)?XC_EXISTS:TRUE);
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
	DWORD FAttr=apiGetFileAttributes(Name);

	/* $ 05.06.2001 IS
	   + посылаем подальше всех, кто пытается отредактировать каталог
	*/
	if (FAttr!=INVALID_FILE_ATTRIBUTES && FAttr&FILE_ATTRIBUTE_DIRECTORY)
	{
		Message(MSG_WARNING,1,MSG(MEditTitle),MSG(MEditCanNotEditDirectory),MSG(MOk));
		ExitCode=XC_OPEN_ERROR;
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
		if (Message(MSG_WARNING,2,MSG(MEditTitle),Name,MSG(MEditRSH),
		            MSG(MEditROOpen),MSG(MYes),MSG(MNo)))
		{
			ExitCode=XC_OPEN_ERROR;
			return;
		}
	}

	m_editor->SetPosition(X1,Y1+(Opt.EdOpt.ShowTitleBar?1:0),X2,Y2-(Opt.EdOpt.ShowKeyBar?1:0));
	m_editor->SetStartPos(StartLine,StartChar);
	SetDeleteOnClose(DeleteOnClose);
	int UserBreak;

	/* $ 06.07.2001 IS
	   При создании файла с нуля так же посылаем плагинам событие EE_READ, дабы
	   не нарушать однообразие.
	*/
	if (FAttr == INVALID_FILE_ATTRIBUTES)
		Flags.Set(FFILEEDIT_NEW);

	if (BlankFileName && Flags.Check(FFILEEDIT_CANNEWFILE))
		Flags.Set(FFILEEDIT_NEW);

	if (Flags.Check(FFILEEDIT_NEW))
	  m_bAddSignature = true;

	if (Flags.Check(FFILEEDIT_LOCKED))
		m_editor->Flags.Set(FEDITOR_LOCKMODE);

	while (!LoadFile(strFullFileName,UserBreak))
	{
		if (BlankFileName)
		{
			Flags.Clear(FFILEEDIT_OPENFAILED); //AY: ну так как редактор мы открываем то видимо надо и сбросить ошибку открытия
			UserBreak=0;
		}

		if (!Flags.Check(FFILEEDIT_NEW) || UserBreak)
		{
			if (UserBreak!=1)
			{
				SetLastError(SysErrorCode);
				if(!OperationFailed(strFullFileName, MEditTitle, MSG(MEditCannotOpen), false))
					continue;
				else
					ExitCode=XC_OPEN_ERROR;
			}
			else
			{
				ExitCode=XC_LOADING_INTERRUPTED;
			}

			// Ахтунг. Ниже комментарии оставлены в назидании потомкам (до тех пор, пока не измениться манагер)
			//FrameManager->DeleteFrame(this); // BugZ#546 - Editor валит фар!
			//CtrlObject->Cp()->Redraw(); //AY: вроде как не надо, делает проблемы с проресовкой если в редакторе из истории попытаться выбрать несуществующий файл

			// если прервали загрузку, то фремы нужно проапдейтить, чтобы предыдущие месаги не оставались на экране
			if (!Opt.Confirm.Esc && UserBreak && ExitCode==XC_LOADING_INTERRUPTED && FrameManager)
				FrameManager->RefreshFrame();

			return;
		}

		if (m_codepage==CP_DEFAULT || m_codepage == CP_REDETECT)
			m_codepage=Opt.EdOpt.AnsiCodePageForNewFile?GetACP():GetOEMCP();

		m_editor->SetCodePage(m_codepage);
		Flags.Set(FFILEEDIT_CODEPAGECHANGEDBYUSER);
		break;
	}

	CtrlObject->Plugins->CurEditor=this;//&FEdit;
	CtrlObject->Plugins->ProcessEditorEvent(EE_READ,nullptr,m_editor->EditorID);
	bEE_READ_Sent = true;
	ShowConsoleTitle();
	EditKeyBar.SetOwner(this);
	EditKeyBar.SetPosition(X1,Y2,X2,Y2);
	InitKeyBar();

	if (!Opt.EdOpt.ShowKeyBar)
		EditKeyBar.Hide0();

	MacroMode=MACRO_EDITOR;
	CtrlObject->Macro.SetMode(MACRO_EDITOR);

	F4KeyOnly=true;

	if (Flags.Check(FFILEEDIT_ENABLEF6))
		FrameManager->InsertFrame(this);
	else
		FrameManager->ExecuteFrame(this);
}

void FileEditor::InitKeyBar()
{
	EditKeyBar.SetAllGroup(KBL_MAIN,         Opt.OnlyEditorViewerUsed?MSingleEditF1:MEditF1, 12);
	EditKeyBar.SetAllGroup(KBL_SHIFT,        Opt.OnlyEditorViewerUsed?MSingleEditShiftF1:MEditShiftF1, 12);
	EditKeyBar.SetAllGroup(KBL_ALT,          Opt.OnlyEditorViewerUsed?MSingleEditAltF1:MEditAltF1, 12);
	EditKeyBar.SetAllGroup(KBL_CTRL,         Opt.OnlyEditorViewerUsed?MSingleEditCtrlF1:MEditCtrlF1, 12);
	EditKeyBar.SetAllGroup(KBL_CTRLSHIFT,    Opt.OnlyEditorViewerUsed?MSingleEditCtrlShiftF1:MEditCtrlShiftF1, 12);
	EditKeyBar.SetAllGroup(KBL_CTRLALT,      Opt.OnlyEditorViewerUsed?MSingleEditCtrlAltF1:MEditCtrlAltF1, 12);
	EditKeyBar.SetAllGroup(KBL_ALTSHIFT,     Opt.OnlyEditorViewerUsed?MSingleEditAltShiftF1:MEditAltShiftF1, 12);
	EditKeyBar.SetAllGroup(KBL_CTRLALTSHIFT, Opt.OnlyEditorViewerUsed?MSingleEditCtrlAltShiftF1:MEditCtrlAltShiftF1, 12);

	if (!GetCanLoseFocus())
		EditKeyBar.Change(KBL_SHIFT,L"",4-1);

	if (Flags.Check(FFILEEDIT_SAVETOSAVEAS))
		EditKeyBar.Change(KBL_MAIN,MSG(MEditShiftF2),2-1);

	if (!Flags.Check(FFILEEDIT_ENABLEF6))
		EditKeyBar.Change(KBL_MAIN,L"",6-1);

	if (!GetCanLoseFocus())
		EditKeyBar.Change(KBL_MAIN,L"",12-1);

	if (!GetCanLoseFocus())
		EditKeyBar.Change(KBL_ALT,L"",11-1);

	if (m_codepage!=GetACP())
		EditKeyBar.Change(KBL_MAIN,MSG(Opt.OnlyEditorViewerUsed?MSingleEditF8:MEditF8),7);
	else
		EditKeyBar.Change(KBL_MAIN,MSG(Opt.OnlyEditorViewerUsed?MSingleEditF8DOS:MEditF8DOS),7);

	EditKeyBar.ReadRegGroup(L"Editor",Opt.strLanguage);
	EditKeyBar.SetAllRegGroup();
	EditKeyBar.Show();
	if (!Opt.EdOpt.ShowKeyBar)
		EditKeyBar.Hide0();
	m_editor->SetPosition(X1,Y1+(Opt.EdOpt.ShowTitleBar?1:0),X2,Y2-(Opt.EdOpt.ShowKeyBar?1:0));
	SetKeyBar(&EditKeyBar);
}

void FileEditor::SetNamesList(NamesList *Names)
{
	if (!EditNamesList)
		EditNamesList = new NamesList;

	Names->MoveData(*EditNamesList);
}

void FileEditor::Show()
{
	if (Flags.Check(FFILEEDIT_FULLSCREEN))
	{
		if (Opt.EdOpt.ShowKeyBar)
		{
			EditKeyBar.SetPosition(0,ScrY,ScrX,ScrY);
			EditKeyBar.Redraw();
		}

		ScreenObject::SetPosition(0,0,ScrX,ScrY-(Opt.EdOpt.ShowKeyBar?1:0));
		m_editor->SetPosition(0,(Opt.EdOpt.ShowTitleBar?1:0),ScrX,ScrY-(Opt.EdOpt.ShowKeyBar?1:0));
	}

	ScreenObject::Show();
}


void FileEditor::DisplayObject()
{
	if (!m_editor->Locked())
	{
		if (m_editor->Flags.Check(FEDITOR_ISRESIZEDCONSOLE))
		{
			m_editor->Flags.Clear(FEDITOR_ISRESIZEDCONSOLE);
			CtrlObject->Plugins->CurEditor=this;
			CtrlObject->Plugins->ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL,m_editor->EditorID);
		}

		m_editor->Show();
	}
}

__int64 FileEditor::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	if (OpCode == MCODE_V_EDITORSTATE)
	{
		DWORD MacroEditState=0;
		MacroEditState|=Flags.Flags&FFILEEDIT_NEW?0x00000001:0;
		MacroEditState|=Flags.Flags&FFILEEDIT_ENABLEF6?0x00000002:0;
		MacroEditState|=Flags.Flags&FFILEEDIT_DELETEONCLOSE?0x00000004:0;
		MacroEditState|=m_editor->Flags.Flags&FEDITOR_MODIFIED?0x00000008:0;
		MacroEditState|=m_editor->BlockStart?0x00000010:0;
		MacroEditState|=m_editor->VBlockStart?0x00000020:0;
		MacroEditState|=m_editor->Flags.Flags&FEDITOR_WASCHANGED?0x00000040:0;
		MacroEditState|=m_editor->Flags.Flags&FEDITOR_OVERTYPE?0x00000080:0;
		MacroEditState|=m_editor->Flags.Flags&FEDITOR_CURPOSCHANGEDBYPLUGIN?0x00000100:0;
		MacroEditState|=m_editor->Flags.Flags&FEDITOR_LOCKMODE?0x00000200:0;
		MacroEditState|=m_editor->EdOpt.PersistentBlocks?0x00000400:0;
		MacroEditState|=Opt.OnlyEditorViewerUsed?0x08000000|0x00000800:0;
		MacroEditState|=!GetCanLoseFocus()?0x00000800:0;
		return (__int64)MacroEditState;
	}

	if (OpCode == MCODE_V_EDITORCURPOS)
		return (__int64)(m_editor->CurLine->GetTabCurPos()+1);

	if (OpCode == MCODE_V_EDITORCURLINE)
		return (__int64)(m_editor->NumLine+1);

	if (OpCode == MCODE_V_ITEMCOUNT || OpCode == MCODE_V_EDITORLINES)
		return (__int64)(m_editor->NumLastLine);

	if (OpCode == MCODE_F_KEYBAR_SHOW)
	{
		int PrevMode=Opt.EdOpt.ShowKeyBar?2:1;
		switch (iParam)
		{
			case 0:
				break;
			case 1:
				Opt.EdOpt.ShowKeyBar=1;
				EditKeyBar.Show();
				Show();
				KeyBarVisible = Opt.EdOpt.ShowKeyBar;
				break;
			case 2:
				Opt.EdOpt.ShowKeyBar=0;
				EditKeyBar.Hide();
				Show();
				KeyBarVisible = Opt.EdOpt.ShowKeyBar;
				break;
			case 3:
				ProcessKey(KEY_CTRLB);
				break;
			default:
				PrevMode=0;
				break;
		}
		return PrevMode;
	}

	return m_editor->VMProcess(OpCode,vParam,iParam);
}


int FileEditor::ProcessKey(int Key)
{
	return ReProcessKey(Key,FALSE);
}

int FileEditor::ReProcessKey(int Key,int CalledFromControl)
{
	if (Key!=KEY_F4 && Key!=KEY_IDLE)
		F4KeyOnly=false;

	if (Flags.Check(FFILEEDIT_REDRAWTITLE) && (((unsigned int)Key & 0x00ffffff) < KEY_END_FKEY || IsInternalKeyReal((unsigned int)Key & 0x00ffffff)))
		ShowConsoleTitle();

	// BugZ#488 - Shift=enter
	if (IntKeyState.ShiftPressed && (Key == KEY_ENTER || Key == KEY_NUMENTER) && CtrlObject->Macro.IsExecuting() == MACROMODE_NOMACRO)
	{
		Key=Key == KEY_ENTER?KEY_SHIFTENTER:KEY_SHIFTNUMENTER;
	}

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
			if (Flags.Check(FFILEEDIT_ENABLEF6))
			{
				int FirstSave=1;
				UINT cp=m_codepage;

				// проверка на "а может это говно удалили уже?"
				// возможно здесь она и не нужна!
				// хотя, раз уж были изменени, то
				if (m_editor->IsFileChanged() && // в текущем сеансе были изменения?
				        apiGetFileAttributes(strFullFileName) == INVALID_FILE_ATTRIBUTES) // а файл еще существует?
				{
					switch (Message(MSG_WARNING,2,MSG(MEditTitle),
					                MSG(MEditSavedChangedNonFile),
					                MSG(MEditSavedChangedNonFile2),
					                MSG(MHYes),MSG(MHNo)))
					{
						case 0:

							if (ProcessKey(KEY_F2))
							{
								FirstSave=0;
								break;
							}

						default:
							return FALSE;
					}
				}

				if (!FirstSave || m_editor->IsFileChanged() || apiGetFileAttributes(strFullFileName)!=INVALID_FILE_ATTRIBUTES)
				{
					__int64 FilePos=m_editor->GetCurPos(true, m_bAddSignature); // TODO: GetCurPos should return __int64

					/* $ 01.02.2001 IS
					   ! Открываем вьюер с указанием длинного имени файла, а не короткого
					*/
					int NeedQuestion = 1;
					if (ProcessQuitKey(FirstSave,NeedQuestion))
					{
						int delete_on_close = 0;
						if (Flags.Check(FFILEEDIT_DELETEONCLOSE))
							delete_on_close = 1;
						else if (Flags.Check(FFILEEDIT_DELETEONLYFILEONCLOSE))
							delete_on_close = 2;
						SetDeleteOnClose(0);

						//объект будет в конце удалён во FrameManager
						new FileViewer(
							strFullFileName.CPtr(),
							GetCanLoseFocus(), Flags.Check(FFILEEDIT_DISABLEHISTORY), FALSE,
							FilePos, nullptr, EditNamesList, Flags.Check(FFILEEDIT_SAVETOSAVEAS), cp,
							strTitle.IsEmpty() ? nullptr : strTitle.CPtr(),
							delete_on_close);
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
				CtrlObject->CmdLine->ShowViewEditHistory();
				return TRUE;
			}

			break; // отдадим Alt-F11 на растерзание плагинам, если редактор модальный
		}
	}

#if 1
	BOOL ProcessedNext=TRUE;

	_SVS(if (Key=='n' || Key=='m'))
		_SVS(SysLog(L"%d Key='%c'",__LINE__,Key));

	if (!CalledFromControl && (CtrlObject->Macro.IsRecording() == MACROMODE_RECORDING_COMMON || CtrlObject->Macro.IsExecuting() == MACROMODE_EXECUTING_COMMON || CtrlObject->Macro.GetCurRecord(nullptr,nullptr) == MACROMODE_NOMACRO))
	{

		_SVS(if (CtrlObject->Macro.IsRecording() == MACROMODE_RECORDING_COMMON || CtrlObject->Macro.IsExecuting() == MACROMODE_EXECUTING_COMMON))
			_SVS(SysLog(L"%d !!!! CtrlObject->Macro.GetCurRecord(nullptr,nullptr) != MACROMODE_NOMACRO !!!!",__LINE__));

		ProcessedNext=!ProcessEditorInput(FrameManager->GetLastInputRecord());
	}

	if (ProcessedNext)
#else
	if (!CalledFromControl && //CtrlObject->Macro.IsExecuting() || CtrlObject->Macro.IsRecording() || // пусть доходят!
	        !ProcessEditorInput(FrameManager->GetLastInputRecord()))
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
				if (!m_editor->Flags.Check(FEDITOR_LOCKMODE))
				{
					m_editor->Pasting++;
					m_editor->TextChanged(1);
					BOOL IsBlock=m_editor->VBlockStart || m_editor->BlockStart;

					if (!m_editor->EdOpt.PersistentBlocks && IsBlock)
					{
						m_editor->TurnOffMarkingBlock();
						m_editor->DeleteBlock();
					}

					//AddUndoData(CurLine->EditLine.GetStringAddr(),NumLine,
					//                CurLine->EditLine.GetCurPos(),UNDO_EDIT);
					m_editor->Paste(strFullFileName); //???
					//if (!EdOpt.PersistentBlocks)
					m_editor->UnmarkBlock();
					m_editor->Pasting--;
					m_editor->Show(); //???
				}

				return (TRUE);
			}
			/* $ 24.08.2000 SVS
			   + Добавляем реакцию показа бакграунда на клавишу CtrlAltShift
			*/
			case KEY_CTRLO:
			case KEY_RCTRLO:
			{
				if (!Opt.OnlyEditorViewerUsed)
				{
					m_editor->Hide();  // $ 27.09.2000 skv - To prevent redraw in macro with Ctrl-O

					if (FrameManager->ShowBackground())
					{
						SetCursorType(FALSE,0);
						WaitKey();
					}

					Show();
				}

				return TRUE;
			}
			case KEY_F2:
			case KEY_SHIFTF2:
			{
				BOOL Done=FALSE;
				string strOldCurDir;
				apiGetCurrentDirectory(strOldCurDir);
				while (!Done) // бьемся до упора
				{
					size_t pos;
					DWORD FNAttr;
					// проверим путь к файлу, может его уже снесли...
					if (FindLastSlash(pos,strFullFileName))
					{
						wchar_t *lpwszPtr = strFullFileName.GetBuffer();
						wchar_t wChr = lpwszPtr[pos+1];
						lpwszPtr[pos+1]=0;

						// В корне?
						if(IsRootPath(lpwszPtr))
						{
							// а дальше? каталог существует?
							if ((FNAttr=apiGetFileAttributes(lpwszPtr)) == INVALID_FILE_ATTRIBUTES ||
							        !(FNAttr&FILE_ATTRIBUTE_DIRECTORY)
							        //|| LocalStricmp(OldCurDir,FullFileName)  // <- это видимо лишнее.
							   )
								Flags.Set(FFILEEDIT_SAVETOSAVEAS);
						}

						lpwszPtr[pos+1]=wChr;
						//strFullFileName.ReleaseBuffer (); так как ничего не поменялось то это лишнее.
					}

					if (Key == KEY_F2 &&
					        (FNAttr=apiGetFileAttributes(strFullFileName)) != INVALID_FILE_ATTRIBUTES &&
					        !(FNAttr&FILE_ATTRIBUTE_DIRECTORY)
					   )
					{
						Flags.Clear(FFILEEDIT_SAVETOSAVEAS);
					}

					static int TextFormat=0;
					UINT codepage = m_codepage;
					bool SaveAs = Key==KEY_SHIFTF2 || Flags.Check(FFILEEDIT_SAVETOSAVEAS);
					int NameChanged=FALSE;
					string strFullSaveAsName = strFullFileName;

					if (SaveAs)
					{
						string strSaveAsName = Flags.Check(FFILEEDIT_SAVETOSAVEAS)?strFullFileName:strFileName;

						if (!dlgSaveFileAs(strSaveAsName, TextFormat, codepage, m_bAddSignature))
							return FALSE;

						apiExpandEnvironmentStrings(strSaveAsName, strSaveAsName);
						Unquote(strSaveAsName);
						NameChanged=StrCmpI(strSaveAsName, (Flags.Check(FFILEEDIT_SAVETOSAVEAS)?strFullFileName:strFileName));

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
								m_editor->InsertString(nullptr, 0);
								m_codepage = codepage;
							}

							SetFileName(strFullSaveAsName);

							if (!bInPlace)
							{
								//Message(MSG_WARNING, 1, L"WARNING!", L"Editor will be reopened with new file!", MSG(MOk));
								int UserBreak;
								LoadFile(strFullSaveAsName, UserBreak);
								// TODO: возможно подобный ниже код здесь нужен (copy/paste из FileEditor::Init()). оформить его нужно по иному
								//if(!Opt.Confirm.Esc && UserBreak && ExitCode==XC_LOADING_INTERRUPTED && FrameManager)
								//  FrameManager->RefreshFrame();
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
				if (!Opt.OnlyEditorViewerUsed && GetCanLoseFocus())
				{
					if (!Flags.Check(FFILEEDIT_DISABLESAVEPOS) && m_editor->EdOpt.SavePos) // save position/codepage before reload
						SaveToCache();
					CtrlObject->Cp()->ActivePanel->ProcessKey(Key);
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

				if (apiGetFileAttributes(strFullFileName) == INVALID_FILE_ATTRIBUTES) // а сам файл то еще на месте?
				{
					if (!CheckShortcutFolder(&strFullFileNameTemp,FALSE))
						return FALSE;

					strFullFileNameTemp += L"\\."; // для вваливания внутрь :-)
				}

				Panel *ActivePanel = CtrlObject->Cp()->ActivePanel;

				if (Flags.Check(FFILEEDIT_NEW) || (ActivePanel && ActivePanel->FindFile(strFileName) == -1)) // Mantis#279
				{
					UpdateFileList();
					Flags.Clear(FFILEEDIT_NEW);
				}

				{
					SaveScreen Sc;
					CtrlObject->Cp()->GoToFile(strFullFileNameTemp);
					Flags.Set(FFILEEDIT_REDRAWTITLE);
				}

				return (TRUE);
			}
			case KEY_CTRLB:
			case KEY_RCTRLB:
			{
				Opt.EdOpt.ShowKeyBar=!Opt.EdOpt.ShowKeyBar;

				if (Opt.EdOpt.ShowKeyBar)
					EditKeyBar.Show();
				else
					EditKeyBar.Hide0(); // 0 mean - Don't purge saved screen

				Show();
				KeyBarVisible = Opt.EdOpt.ShowKeyBar;
				return (TRUE);
			}
			case KEY_CTRLSHIFTB:
			case KEY_RCTRLSHIFTB:
			{
				Opt.EdOpt.ShowTitleBar=!Opt.EdOpt.ShowTitleBar;
				TitleBarVisible = Opt.EdOpt.ShowTitleBar;
				Show();
				return (TRUE);
			}
			case KEY_SHIFTF10:

				if (!ProcessKey(KEY_F2)) // учтем факт того, что могли отказаться от сохранения
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
					bool FilePlaced=apiGetFileAttributes(strFullFileName) == INVALID_FILE_ATTRIBUTES && !Flags.Check(FFILEEDIT_NEW);

					if (m_editor->IsFileChanged() || // в текущем сеансе были изменения?
					        FilePlaced) // а сам файл то еще на месте?
					{
						int Res;

						if (m_editor->IsFileChanged() && FilePlaced)
							Res=Message(MSG_WARNING,3,MSG(MEditTitle),
							            MSG(MEditSavedChangedNonFile),
							            MSG(MEditSavedChangedNonFile2),
							            MSG(MHYes),MSG(MHNo),MSG(MHCancel));
						else if (!m_editor->IsFileChanged() && FilePlaced)
							Res=Message(MSG_WARNING,3,MSG(MEditTitle),
							            MSG(MEditSavedChangedNonFile1),
							            MSG(MEditSavedChangedNonFile2),
						                MSG(MHYes),MSG(MHNo),MSG(MHCancel));
						else
							Res=100;

						switch (Res)
						{
							case 0:

								if (!ProcessKey(KEY_F2)) // попытка сначала сохранить
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
					else if (!m_editor->Flags.Check(FEDITOR_MODIFIED)) //????
						NeedQuestion=0;
				}

				if (!ProcessQuitKey(FirstSave,NeedQuestion))
					return FALSE;

				return TRUE;
			}
			case KEY_F8:
			{
				if (!IsUnicodeCodePage(m_codepage))
				{
					if (SetCodePage(m_codepage==GetACP()?GetOEMCP():GetACP()))
					{
						Flags.Set(FFILEEDIT_CODEPAGECHANGEDBYUSER);
						ChangeEditKeyBar();
					}
				}
				else
				{
					Message(MSG_WARNING,1,MSG(MEditTitle),MSG(MEditorSwitchUnicodeCPDisabled),MSG(MEditorTryReloadFile),MSG(MOk));
				}

				return TRUE;
			}
			case KEY_SHIFTF8:
			{
				if (!IsUnicodeCodePage(m_codepage))
				{
					UINT codepage = SelectCodePage(m_codepage, false, true, false, true);
					if ( codepage == (CP_DEFAULT & 0xffff) )
					{
						File edit_file;
						bool detect = false, sig_found = false;
						wchar_t ss[256];

						if (edit_file.Open(strFileName, FILE_READ_DATA, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, nullptr, OPEN_EXISTING))
						{
							detect = GetFileFormat(edit_file,codepage,&sig_found,true);
							edit_file.Close();
						}
						if ( !detect )
						{
							Message(MSG_WARNING,1,MSG(MEditTitle),MSG(MEditorCPNotDetected),MSG(MOk));
						}
						else if ( IsUnicodeCodePage(codepage) )
						{
							detect = false;
							_snwprintf(ss, ARRAYSIZE(ss), MSG(MEditorSwitchToUnicodeCPDisabled), static_cast<int>(codepage));
							Message(MSG_WARNING,1,MSG(MEditTitle),ss,MSG(MEditorTryReloadFile),MSG(MOk));
						}
						else if ( !IsCodePageSupported(codepage) )
						{
							detect = false;
							_snwprintf(ss, ARRAYSIZE(ss), MSG(MEditorCPNotSupported), static_cast<int>(codepage));
							Message(MSG_WARNING,1,MSG(MEditTitle),ss,MSG(MOk));
						}
						if ( !detect )
							codepage = static_cast<UINT>(-1);
					}

					if (codepage != static_cast<UINT>(-1))
					{
						SetCodePage(codepage);
						InitKeyBar();
						Flags.Set(FFILEEDIT_CODEPAGECHANGEDBYUSER);
					}
				}
				else
				{
					Message(MSG_WARNING,1,MSG(MEditTitle),MSG(MEditorSwitchUnicodeCPDisabled),MSG(MEditorTryReloadFile),MSG(MOk));
				}

				return TRUE;
			}
			case KEY_ALTSHIFTF9:
			case KEY_RALTSHIFTF9:
			{
				//     Работа с локальной копией EditorOptions
				EditorOptions EdOpt;
				GetEditorOptions(EdOpt);
				EditorConfig(EdOpt,true); // $ 27.11.2001 DJ - Local в EditorConfig
				EditKeyBar.Show(); //???? Нужно ли????
				SetEditorOptions(EdOpt);

				if (Opt.EdOpt.ShowKeyBar)
					EditKeyBar.Show();

				m_editor->Show();
				return TRUE;
			}
			default:
			{
				if (Flags.Check(FFILEEDIT_FULLSCREEN) && CtrlObject->Macro.IsExecuting() == MACROMODE_NOMACRO)
					if (Opt.EdOpt.ShowKeyBar)
						EditKeyBar.Show();

				if (!EditKeyBar.ProcessKey(Key))
					return(m_editor->ProcessKey(Key));
			}
		}
	}
	return TRUE;
}


int FileEditor::ProcessQuitKey(int FirstSave,BOOL NeedQuestion)
{
	string strOldCurDir;
	apiGetCurrentDirectory(strOldCurDir);

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
				if (apiGetFileAttributes(strFullFileName)!=INVALID_FILE_ATTRIBUTES)
				{
					UpdateFileList();
				}
			}

			FrameManager->DeleteFrame();
			SetExitCode(XC_QUIT);
			break;
		}

		if (!StrCmp(strFileName,MSG(MNewFileName)))
		{
			if (!ProcessKey(KEY_SHIFTF2))
			{
				FarChDir(strOldCurDir);
				return FALSE;
			}
			else
				break;
		}

		SetLastError(SysErrorCode);

		if (OperationFailed(strFullFileName, MEditTitle, MSG(MEditCannotSave), false))
			break;

		FirstSave=0;
	}

	FarChDir(strOldCurDir);
	return (unsigned int)GetExitCode() == XC_QUIT;
}


// сюды плавно переносить код из Editor::ReadFile()
int FileEditor::LoadFile(const string& Name,int &UserBreak)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	TPreRedrawFuncGuard preRedrawFuncGuard(Editor::PR_EditorShowMsg);
	TaskBar TB;
	wakeful W;
	int LastLineCR = 0;
	EditorPosCache pc;
	UserBreak = 0;
	File EditFile;
	if(!EditFile.Open(Name, GENERIC_READ, FILE_SHARE_READ|(Opt.EdOpt.EditOpenedForWrite?FILE_SHARE_WRITE:0), nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN))
	{
		SysErrorCode=GetLastError();

		if ((SysErrorCode != ERROR_FILE_NOT_FOUND) && (SysErrorCode != ERROR_PATH_NOT_FOUND))
		{
			UserBreak = -1;
			Flags.Set(FFILEEDIT_OPENFAILED);
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

	if (Opt.EdOpt.FileSizeLimitLo || Opt.EdOpt.FileSizeLimitHi)
	{
		UINT64 FileSize=0;
		if (EditFile.GetSize(FileSize))
		{
			UINT64 MaxSize = static_cast<DWORD>(Opt.EdOpt.FileSizeLimitHi) * 0x100000000ull + static_cast<DWORD>(Opt.EdOpt.FileSizeLimitLo);

			if (FileSize > MaxSize)
			{
				string strTempStr1, strTempStr2;
				// Ширина = 8 - это будет... в Kb и выше...
				FileSizeToStr(strTempStr1, FileSize, 8);
				FileSizeToStr(strTempStr2, MaxSize, 8);

				if (Message(MSG_WARNING,2,MSG(MEditTitle),
					Name,
					LangString(MEditFileLong) << RemoveExternalSpaces(strTempStr1),
					LangString(MEditFileLong2) << RemoveExternalSpaces(strTempStr2),
					MSG(MEditROOpen), MSG(MYes),MSG(MNo)))
				{
					EditFile.Close();
					SetLastError(ERROR_OPEN_FAILED); //????
					UserBreak=1;
					Flags.Set(FFILEEDIT_OPENFAILED);
					return FALSE;
				}
			}
		}
		else
		{
			if (Message(MSG_WARNING,2,MSG(MEditTitle),Name,MSG(MEditFileGetSizeError),MSG(MEditROOpen),MSG(MYes),MSG(MNo)))
			{
				EditFile.Close();
				SetLastError(SysErrorCode=ERROR_OPEN_FAILED); //????
				UserBreak=1;
				Flags.Set(FFILEEDIT_OPENFAILED);
				return FALSE;
			}
		}
	}

	m_editor->FreeAllocatedData(false);
	bool bCached = LoadFromCache(pc);

	DWORD FileAttributes=apiGetFileAttributes(Name);
	if((m_editor->EdOpt.ReadOnlyLock&1) && FileAttributes != INVALID_FILE_ATTRIBUTES && (FileAttributes & (FILE_ATTRIBUTE_READONLY|((m_editor->EdOpt.ReadOnlyLock&0x60)>>4))))
	{
		m_editor->Flags.Swap(FEDITOR_LOCKMODE);
	}

	// Проверяем поддерживается или нет загруженная кодовая страница
	if (bCached && pc.CodePage && !IsCodePageSupported(pc.CodePage))
		pc.CodePage = 0;

	GetFileString GetStr(EditFile);
	*m_editor->GlobalEOL=0; //BUGBUG???
	wchar_t *Str;
	int StrLength,GetCode;
	UINT dwCP=0;
	bool Detect=false;

	bool redetect = (m_codepage == CP_REDETECT);
	if (redetect)
		m_codepage = CP_DEFAULT;

	if (m_codepage == CP_DEFAULT || IsUnicodeOrUtfCodePage(m_codepage))
	{
		Detect=GetFileFormat(EditFile,dwCP,&m_bAddSignature,redetect || Opt.EdOpt.AutoDetectCodePage!=0);

		// Проверяем поддерживается или нет задетектировання кодовая страница
		if (Detect)
			Detect = IsCodePageSupported(dwCP);
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
				Flags.Set(FFILEEDIT_CODEPAGECHANGEDBYUSER);
			}
		}

		if (m_codepage==CP_DEFAULT)
			m_codepage=Opt.EdOpt.AnsiCodePageAsDefault?GetACP():GetOEMCP();
	}
	else
	{
		Flags.Set(FFILEEDIT_CODEPAGECHANGEDBYUSER);
	}

	m_editor->SetCodePage(m_codepage);  //BUGBUG

	if (!IsUnicodeOrUtfCodePage(m_codepage))
	{
		EditFile.SetPointer(0, nullptr, FILE_BEGIN);
	}

	UINT64 FileSize=0;
	EditFile.GetSize(FileSize);
	DWORD StartTime=GetTickCount();

	while ((GetCode=GetStr.GetString(&Str, m_codepage, StrLength)))
	{
		if (GetCode == -1)
		{
			EditFile.Close();
			return FALSE;
		}

		LastLineCR=0;
		DWORD CurTime=GetTickCount();

		if (CurTime-StartTime>(DWORD)Opt.RedrawTimeout)
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

			SetCursorType(FALSE,0);
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

		int Offset = StrLength > 3 ? StrLength - 3 : 0;

		if (!LastLineCR &&
		        (
		            (CurEOL = wmemchr(Str+Offset,L'\r',StrLength-Offset))  ||
		            (CurEOL = wmemchr(Str+Offset,L'\n',StrLength-Offset))
		        )
		   )
		{
			xwcsncpy(m_editor->GlobalEOL,CurEOL,ARRAYSIZE(m_editor->GlobalEOL));
			m_editor->GlobalEOL[ARRAYSIZE(m_editor->GlobalEOL)-1]=0;
			LastLineCR=1;
		}

		if (!m_editor->InsertString(Str, StrLength))
		{
			EditFile.Close();
			return FALSE;
		}
	}

	BadConversion = !GetStr.IsConversionValid();
	if (BadConversion)
	{
		Message(MSG_WARNING,1,MSG(MWarning),MSG(MEditorLoadCPWarn1),MSG(MEditorLoadCPWarn2),MSG(MEditorSaveNotRecommended),MSG(MOk));
	}

	if (LastLineCR||!m_editor->NumLastLine)
		m_editor->InsertString(L"", 0);

	EditFile.Close();
	//if ( bCached )
	m_editor->SetCacheParams(pc, m_bAddSignature);
	SysErrorCode=GetLastError();
	apiGetFindDataEx(Name, FileInfo);
	EditorGetFileAttributes(Name);
	return TRUE;
}

//TextFormat и Codepage используются ТОЛЬКО, если bSaveAs = true!

int FileEditor::SaveFile(const string& Name,int Ask, bool bSaveAs, int TextFormat, UINT codepage, bool AddSignature)
{
	if (!bSaveAs)
	{
		TextFormat=0;
		codepage=m_editor->GetCodePage();
	}

	TaskBar TB;
	wakeful W;

	if (m_editor->Flags.Check(FEDITOR_LOCKMODE) && !m_editor->Flags.Check(FEDITOR_MODIFIED) && !bSaveAs)
		return SAVEFILE_SUCCESS;

	if (Ask)
	{
		if (!m_editor->Flags.Check(FEDITOR_MODIFIED))
			return SAVEFILE_SUCCESS;

		if (Ask)
		{
			int Code = AllowCancelExit?Message(MSG_WARNING,3,MSG(MEditTitle),MSG(MEditAskSave),MSG(MHYes),MSG(MHNo),MSG(MHCancel)):Message(MSG_WARNING,2,MSG(MEditTitle),MSG(MEditAskSave),MSG(MHYes),MSG(MHNo));
			if(Code < 0 && !AllowCancelExit)
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
					m_editor->TextChanged(0); // 10.08.2000 skv: TextChanged() support;
					return SAVEFILE_SUCCESS;
			}
		}
	}

	int NewFile=TRUE;
	FileAttributesModified=false;

	if ((FileAttributes=apiGetFileAttributes(Name))!=INVALID_FILE_ATTRIBUTES)
	{
		// Проверка времени модификации...
		if (!Flags.Check(FFILEEDIT_SAVEWQUESTIONS))
		{
			FAR_FIND_DATA_EX FInfo;

			if (apiGetFindDataEx(Name, FInfo) && !FileInfo.strFileName.IsEmpty())
			{
				__int64 RetCompare=FileTimeDifference(&FileInfo.ftLastWriteTime,&FInfo.ftLastWriteTime);

				if (RetCompare || !(FInfo.nFileSize == FileInfo.nFileSize))
				{
					SetMessageHelp(L"WarnEditorSavedEx");

					switch (Message(MSG_WARNING,3,MSG(MEditTitle),MSG(MEditAskSaveExt),
					                MSG(MHYes),MSG(MEditBtnSaveAs),MSG(MHCancel)))
					{
						case -1:
						case -2:
						case 2:  // Continue Edit
							return SAVEFILE_CANCEL;
						case 1:  // Save as

							if (ProcessKey(KEY_SHIFTF2))
								return SAVEFILE_SUCCESS;
							else
								return SAVEFILE_CANCEL;

						case 0:  // Save
							break;
					}
				}
			}
		}

		Flags.Clear(FFILEEDIT_SAVEWQUESTIONS);
		NewFile=FALSE;

		if (FileAttributes & FILE_ATTRIBUTE_READONLY)
		{
			//BUGBUG
			int AskOverwrite=Message(MSG_WARNING,2,MSG(MEditTitle),Name,MSG(MEditRO),
			                         MSG(MEditOvr),MSG(MYes),MSG(MNo));

			if (AskOverwrite)
				return SAVEFILE_CANCEL;

			apiSetFileAttributes(Name,FileAttributes & ~FILE_ATTRIBUTE_READONLY); // сняты атрибуты
			FileAttributesModified=true;
		}

		if (FileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))
		{
			apiSetFileAttributes(Name,FILE_ATTRIBUTE_NORMAL);
			FileAttributesModified=true;
		}
	}
	else
	{
		// проверим путь к файлу, может его уже снесли...
		string strCreatedPath = Name;
		const wchar_t *Ptr=LastSlash(strCreatedPath);

		if (Ptr)
		{
			CutToSlash(strCreatedPath);
			DWORD FAttr=0;

			if (apiGetFileAttributes(strCreatedPath) == INVALID_FILE_ATTRIBUTES)
			{
				// и попробуем создать.
				// Раз уж
				CreatePath(strCreatedPath);
				FAttr=apiGetFileAttributes(strCreatedPath);
			}

			if (FAttr == INVALID_FILE_ATTRIBUTES)
				return SAVEFILE_ERROR;
		}
	}

	if (BadConversion)
	{
		if(Message(MSG_WARNING,2,MSG(MWarning),MSG(MEditDataLostWarn),MSG(MEditorSaveNotRecommended),MSG(MOk),MSG(MCancel)))
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
		m_editor->Flags.Set(FEDITOR_WASCHANGED);

	switch (TextFormat)
	{
		case 1:
			wcscpy(m_editor->GlobalEOL,DOS_EOL_fmt);
			break;
		case 2:
			wcscpy(m_editor->GlobalEOL,UNIX_EOL_fmt);
			break;
		case 3:
			wcscpy(m_editor->GlobalEOL,MAC_EOL_fmt);
			break;
		case 4:
			wcscpy(m_editor->GlobalEOL,WIN_EOL_fmt);
			break;
	}

	if (apiGetFileAttributes(Name) == INVALID_FILE_ATTRIBUTES)
		Flags.Set(FFILEEDIT_NEW);

	{
		//SaveScreen SaveScr;
		/* $ 11.10.2001 IS
		   Если было произведено сохранение с любым результатом, то не удалять файл
		*/
		Flags.Clear(FFILEEDIT_DELETEONCLOSE|FFILEEDIT_DELETEONLYFILEONCLOSE);
		CtrlObject->Plugins->CurEditor=this;
//_D(SysLog(L"%08d EE_SAVE",__LINE__));

		if (!IsUnicodeOrUtfCodePage(codepage))
		{
			int LineNumber=0;
			bool BadSaveConfirmed=false;
			for (Edit *CurPtr=m_editor->TopList; CurPtr; CurPtr=CurPtr->m_next,LineNumber++)
			{
				const wchar_t *SaveStr, *EndSeq;
				intptr_t Length;
				CurPtr->GetBinaryString(&SaveStr,&EndSeq,Length);
				BOOL UsedDefaultCharStr=FALSE,UsedDefaultCharEOL=FALSE;
				WideCharToMultiByte(codepage,WC_NO_BEST_FIT_CHARS,SaveStr,Length,nullptr,0,nullptr,&UsedDefaultCharStr);

				if (!*EndSeq && CurPtr->m_next)
					EndSeq=*m_editor->GlobalEOL?m_editor->GlobalEOL:DOS_EOL_fmt;

				if (TextFormat&&*EndSeq)
					EndSeq=m_editor->GlobalEOL;

				WideCharToMultiByte(codepage,WC_NO_BEST_FIT_CHARS,EndSeq,StrLength(EndSeq),nullptr,0,nullptr,&UsedDefaultCharEOL);

				if (!BadSaveConfirmed && (UsedDefaultCharStr||UsedDefaultCharEOL))
				{
					//SetMessageHelp(L"EditorDataLostWarning")
					int Result=Message(MSG_WARNING,3,MSG(MWarning),MSG(MEditorSaveCPWarn1),MSG(MEditorSaveCPWarn2),MSG(MEditorSaveNotRecommended),MSG(MCancel),MSG(MEditorSaveCPWarnShow),MSG(MOk));
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

		CtrlObject->Plugins->ProcessEditorEvent(EE_SAVE,nullptr,m_editor->EditorID);
		File EditFile;
		DWORD dwWritten=0;
		// Don't use CreationDisposition=CREATE_ALWAYS here - it's kills alternate streams
		if(!EditFile.Open(Name, GENERIC_WRITE, FILE_SHARE_READ, nullptr, Flags.Check(FFILEEDIT_NEW)?CREATE_NEW:TRUNCATE_EXISTING, FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN))
		{
			//_SVS(SysLogLastError();SysLog(L"Name='%s',FileAttributes=%d",Name,FileAttributes));
			RetCode=SAVEFILE_ERROR;
			SysErrorCode=GetLastError();
			goto end;
		}

		m_editor->UndoSavePos=m_editor->UndoPos;
		m_editor->Flags.Clear(FEDITOR_UNDOSAVEPOSLOST);
//    ConvertNameToFull(Name,FileName, sizeof(FileName));
		/*
		    if (ConvertNameToFull(Name,m_editor->FileName, sizeof(m_editor->FileName)) >= sizeof(m_editor->FileName))
		    {
		      m_editor->Flags.Set(FEDITOR_OPENFAILED);
		      RetCode=SAVEFILE_ERROR;
		      goto end;
		    }
		*/
		SetCursorType(FALSE,0);
		TPreRedrawFuncGuard preRedrawFuncGuard(Editor::PR_EditorShowMsg);

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
				apiDeleteFile(Name);
				RetCode=SAVEFILE_ERROR;
				goto end;
			}
		}

		DWORD StartTime=GetTickCount();
		size_t LineNumber=0;
		CachedWrite Cache(EditFile);

		for (Edit *CurPtr=m_editor->TopList; CurPtr; CurPtr=CurPtr->m_next,LineNumber++)
		{
			DWORD CurTime=GetTickCount();

			if (CurTime-StartTime>(DWORD)Opt.RedrawTimeout)
			{
				StartTime=CurTime;
				Editor::EditorShowMsg(MSG(MEditTitle),MSG(MEditSaving),Name,(int)(LineNumber*100/m_editor->NumLastLine));
			}

			const wchar_t *SaveStr, *EndSeq;

			intptr_t Length;

			CurPtr->GetBinaryString(&SaveStr,&EndSeq,Length);

			if (!*EndSeq && CurPtr->m_next && (*CurPtr->GetEOL()))
				EndSeq=*m_editor->GlobalEOL ? m_editor->GlobalEOL:DOS_EOL_fmt;

			if (TextFormat && *EndSeq)
			{
				EndSeq=m_editor->GlobalEOL;
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
				if (Length)
				{
					DWORD length = (codepage == CP_REVERSEBOM?static_cast<DWORD>(Length*sizeof(wchar_t)):WideCharToMultiByte(codepage, 0, SaveStr, Length, nullptr, 0, nullptr, nullptr));
					char *SaveStrCopy=(char *)xf_malloc(length);

					if (SaveStrCopy)
					{
						if (codepage == CP_REVERSEBOM)
							_swab((char*)SaveStr,SaveStrCopy,length);
						else
							WideCharToMultiByte(codepage, 0, SaveStr, Length, SaveStrCopy, length, nullptr, nullptr);

						if (!Cache.Write(SaveStrCopy,length))
						{
							bError = true;
							SysErrorCode=GetLastError();
						}

						xf_free(SaveStrCopy);
					}
					else
						bError = true;
				}

				if (!bError)
				{
					if (EndLength)
					{
						DWORD endlength = (codepage == CP_REVERSEBOM?static_cast<DWORD>(EndLength*sizeof(wchar_t)):WideCharToMultiByte(codepage, 0, EndSeq, EndLength, nullptr, 0, nullptr, nullptr));
						char *EndSeqCopy=(char *)xf_malloc(endlength);

						if (EndSeqCopy)
						{
							if (codepage == CP_REVERSEBOM)
								_swab((char*)EndSeq,EndSeqCopy,endlength);
							else
								WideCharToMultiByte(codepage, 0, EndSeq, EndLength, EndSeqCopy, endlength, nullptr, nullptr);

							if (!Cache.Write(EndSeqCopy,endlength))
							{
								bError = true;
								SysErrorCode=GetLastError();
							}

							xf_free(EndSeqCopy);
						}
						else
							bError = true;
					}
				}
			}

			if (bError)
			{
				EditFile.Close();
				apiDeleteFile(Name);
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
			apiDeleteFile(Name);
			RetCode=SAVEFILE_ERROR;
		}
	}

end:

	if (FileAttributes!=INVALID_FILE_ATTRIBUTES && FileAttributesModified)
	{
		apiSetFileAttributes(Name,FileAttributes|FILE_ATTRIBUTE_ARCHIVE);
	}

	apiGetFindDataEx(Name, FileInfo);
	EditorGetFileAttributes(Name);

	if (m_editor->Flags.Check(FEDITOR_MODIFIED) || NewFile)
		m_editor->Flags.Set(FEDITOR_WASCHANGED);

	/* Этот кусок раскомметировать в том случае, если народ решит, что
	   для если файл был залочен и мы его переписали под други именем...
	   ...то "лочка" должна быть снята.
	*/

//  if(SaveAs)
//    Flags.Clear(FEDITOR_LOCKMODE);
	/* 28.12.2001 VVM
	  ! Проверить на успешную запись */
	if (RetCode==SAVEFILE_SUCCESS)
		m_editor->TextChanged(0);

	if (GetDynamicallyBorn()) // принудительно сбросим Title // Flags.Check(FFILEEDIT_SAVETOSAVEAS) ????????
		strTitle.Clear();

	Show();
	// ************************************
	Flags.Clear(FFILEEDIT_NEW);
	return RetCode;
}

int FileEditor::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	F4KeyOnly = false;
	if (!EditKeyBar.ProcessMouse(MouseEvent))
		if (!ProcessEditorInput(FrameManager->GetLastInputRecord()))
			if (!m_editor->ProcessMouse(MouseEvent))
				return FALSE;

	return TRUE;
}


int FileEditor::GetTypeAndName(string &strType, string &strName)
{
	strType = MSG(MScreensEdit);
	strName = strFullFileName;
	return(MODALTYPE_EDITOR);
}


void FileEditor::ShowConsoleTitle()
{
	string strEditorTitleFormat=Opt.strEditorTitleFormat.Get();
	ReplaceStrings(strEditorTitleFormat,L"%Lng",MSG(MInEditor),-1,true);
	ReplaceStrings(strEditorTitleFormat,L"%File",PointToName(strFileName),-1,true);
	ConsoleTitle::SetFarTitle(strEditorTitleFormat);
	Flags.Clear(FFILEEDIT_REDRAWTITLE);
}

void FileEditor::SetScreenPosition()
{
	if (Flags.Check(FFILEEDIT_FULLSCREEN))
	{
		SetPosition(0,0,ScrX,ScrY);
	}
}

void FileEditor::OnDestroy()
{
	_OT(SysLog(L"[%p] FileEditor::OnDestroy()",this));

	if (!Flags.Check(FFILEEDIT_DISABLEHISTORY) && StrCmpI(strFileName,MSG(MNewFileName)))
		CtrlObject->ViewHistory->AddToHistory(strFullFileName,(m_editor->Flags.Check(FEDITOR_LOCKMODE)?4:1));

	if (CtrlObject->Plugins->CurEditor==this)//&this->FEdit)
	{
		CtrlObject->Plugins->CurEditor=nullptr;
	}
}

int FileEditor::GetCanLoseFocus(int DynamicMode)
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
		return CanLoseFocus;
	}

	return TRUE;
}

void FileEditor::SetLockEditor(BOOL LockMode)
{
	if (LockMode)
		m_editor->Flags.Set(FEDITOR_LOCKMODE);
	else
		m_editor->Flags.Clear(FEDITOR_LOCKMODE);
}

int FileEditor::FastHide()
{
	return Opt.AllCtrlAltShiftRule & CASR_EDITOR;
}

BOOL FileEditor::isTemporary()
{
	return (!GetDynamicallyBorn());
}

void FileEditor::ResizeConsole()
{
	m_editor->PrepareResizedConsole();
}

int FileEditor::ProcessEditorInput(INPUT_RECORD *Rec)
{
	int RetCode;
	CtrlObject->Plugins->CurEditor=this;
	RetCode=CtrlObject->Plugins->ProcessEditorInput(Rec);
	return RetCode;
}

void FileEditor::SetPluginTitle(const string* PluginTitle)
{
	if (!PluginTitle)
		strPluginTitle.Clear();
	else
		strPluginTitle = *PluginTitle;
}

BOOL FileEditor::SetFileName(const string& NewFileName)
{
	strFileName = NewFileName;

	if (StrCmp(strFileName,MSG(MNewFileName)))
	{
		ConvertNameToFull(strFileName, strFullFileName);
		string strFilePath=strFullFileName;

		if (CutToSlash(strFilePath,1))
		{
			string strCurPath;

			if (apiGetCurrentDirectory(strCurPath))
			{
				DeleteEndSlash(strCurPath);

				if (!StrCmpI(strFilePath,strCurPath))
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
	if (m_codepage!=GetACP())
		EditKeyBar.Change(MSG(Opt.OnlyEditorViewerUsed?MSingleEditF8:MEditF8),7);
	else
		EditKeyBar.Change(MSG(Opt.OnlyEditorViewerUsed?MSingleEditF8DOS:MEditF8DOS),7);

	EditKeyBar.Redraw();
}

string &FileEditor::GetTitle(string &strLocalTitle,int SubLen,int TruncSize)
{
	if (!strPluginTitle.IsEmpty())
		strLocalTitle = strPluginTitle;
	else
	{
		if (!strTitle.IsEmpty())
			strLocalTitle = strTitle;
		else
			strLocalTitle = strFullFileName;
	}

	return strLocalTitle;
}

void FileEditor::ShowStatus()
{
	if (m_editor->Locked() || !Opt.EdOpt.ShowTitleBar)
		return;

	SetColor(COL_EDITORSTATUS);
	GotoXY(X1,Y1); //??
	string strLineStr;
	string strLocalTitle;
	GetTitle(strLocalTitle);
	int NameLength = (Opt.ViewerEditorClock && Flags.Check(FFILEEDIT_FULLSCREEN)) ? 15:21;
	if (X2 > 80)
		NameLength += (X2-80);

	if (!strPluginTitle.IsEmpty() || !strTitle.IsEmpty())
		TruncPathStr(strLocalTitle, (ObjWidth<NameLength?ObjWidth:NameLength));
	else
		TruncPathStr(strLocalTitle, NameLength);

	//предварительный расчет
	strLineStr = FormatString() << m_editor->NumLastLine << L'/' << m_editor->NumLastLine;
	int SizeLineStr = (int)strLineStr.GetLength();

	if (SizeLineStr > 12)
		NameLength -= (SizeLineStr-12);
	else
		SizeLineStr = 12;

	strLineStr = FormatString() << m_editor->NumLine+1 << L'/' << m_editor->NumLastLine;
	string strAttr(AttrStr);
	FormatString FString;
	FString<<fmt::LeftAlign()<<fmt::MinWidth(NameLength)<<strLocalTitle<<L' '<<
	(m_editor->Flags.Check(FEDITOR_MODIFIED) ? L'*':L' ')<<
	(m_editor->Flags.Check(FEDITOR_LOCKMODE) ? L'-':L' ')<<
	(m_editor->Flags.Check(FEDITOR_PROCESSCTRLQ) ? L'"':L' ')<<
	fmt::MinWidth(5)<<m_codepage<<L' '<<fmt::MinWidth(3)<<MSG(MEditStatusLine)<<L' '<<
	fmt::ExactWidth(SizeLineStr)<<strLineStr<<L' '<<

	fmt::MinWidth(3)<<MSG(MEditStatusCol)<<L' '<<
	fmt::LeftAlign()<<fmt::MinWidth(4)<<m_editor->CurLine->GetTabCurPos()+1<<L' '<<

	fmt::MinWidth(2)<<MSG(MEditStatusChar)<<L' '<<
	fmt::LeftAlign()<<fmt::MinWidth(4)<<m_editor->CurLine->GetCurPos()+1<<L' '<<


	fmt::MinWidth(3)<<strAttr;
	int StatusWidth=ObjWidth - ((Opt.ViewerEditorClock && Flags.Check(FFILEEDIT_FULLSCREEN))?5:0);

	if (StatusWidth<0)
		StatusWidth=0;

	FS<<fmt::LeftAlign()<<fmt::ExactWidth(StatusWidth)<<FString;
	{
		const wchar_t *Str;
		intptr_t Length;
		m_editor->CurLine->GetBinaryString(&Str,nullptr,Length);
		int CurPos=m_editor->CurLine->GetCurPos();

		if (CurPos<Length)
		{
			GotoXY(X2-((Opt.ViewerEditorClock && Flags.Check(FFILEEDIT_FULLSCREEN)) ? 14:8)-(!m_editor->EdOpt.CharCodeBase?3:0),Y1);
			SetColor(COL_EDITORSTATUS);
			/* $ 27.02.2001 SVS
			Показываем в зависимости от базы */
			switch(m_editor->EdOpt.CharCodeBase)
			{
			case 0:
				FS << fmt::MinWidth(7) << fmt::FillChar(L'0') << fmt::Radix(8) << static_cast<UINT>(Str[CurPos]);
				break;
			case 2:
				FS << fmt::MinWidth(4) << fmt::FillChar(L'0') << fmt::Radix(16) << static_cast<UINT>(Str[CurPos]) << L'h';
				break;
			case 1:
			default:
				FS << fmt::MinWidth(5) << static_cast<UINT>(Str[CurPos]);
				break;
			}

			if (!IsUnicodeOrUtfCodePage(m_codepage))
			{
				char C=0;
				BOOL UsedDefaultChar=FALSE;
				WideCharToMultiByte(m_codepage,WC_NO_BEST_FIT_CHARS,&Str[CurPos],1,&C,1,0,&UsedDefaultChar);

				if (C && !UsedDefaultChar && static_cast<wchar_t>(C)!=Str[CurPos])
				{
					FS << L"/";
					switch(m_editor->EdOpt.CharCodeBase)
					{
					case 0:
						FS << fmt::MinWidth(4) << fmt::FillChar(L'0') << fmt::Radix(8) << static_cast<UINT>(C);
						break;
					case 2:
						FS << fmt::MinWidth(2) << fmt::FillChar(L'0') << fmt::Radix(16) << static_cast<UINT>(C) << L'h';
						break;
					case 1:
					default:
						FS << fmt::MinWidth(3) << static_cast<UINT>(C);
						break;
					}
				}
			}
		}
	}

	if (Opt.ViewerEditorClock && Flags.Check(FFILEEDIT_FULLSCREEN))
		ShowTime(FALSE);
}

/* $ 13.02.2001
     Узнаем атрибуты файла и заодно сформируем готовую строку атрибутов для
     статуса.
*/
DWORD FileEditor::EditorGetFileAttributes(const string& Name)
{
	FileAttributes=apiGetFileAttributes(Name);
	int ind=0;

	if (FileAttributes!=INVALID_FILE_ATTRIBUTES)
	{
		if (FileAttributes&FILE_ATTRIBUTE_READONLY) AttrStr[ind++]=L'R';

		if (FileAttributes&FILE_ATTRIBUTE_SYSTEM) AttrStr[ind++]=L'S';

		if (FileAttributes&FILE_ATTRIBUTE_HIDDEN) AttrStr[ind++]=L'H';
	}

	AttrStr[ind]=0;
	return FileAttributes;
}

/* Return TRUE - панель обовили
*/
BOOL FileEditor::UpdateFileList()
{
	Panel *ActivePanel = CtrlObject->Cp()->ActivePanel;
	const wchar_t *FileName = PointToName(strFullFileName);
	string strFilePath, strPanelPath;
	strFilePath = strFullFileName;
	strFilePath.SetLength(FileName - strFullFileName.CPtr());
	ActivePanel->GetCurDir(strPanelPath);
	AddEndSlash(strPanelPath);
	AddEndSlash(strFilePath);

	if (!StrCmp(strPanelPath, strFilePath))
	{
		ActivePanel->Update(UPDATE_KEEP_SELECTION|UPDATE_DRAW_MESSAGE);
		return TRUE;
	}

	return FALSE;
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
	Flags.Clear(FFILEEDIT_DELETEONCLOSE|FFILEEDIT_DELETEONLYFILEONCLOSE);

	if (NewMode==1)
		Flags.Set(FFILEEDIT_DELETEONCLOSE);
	else if (NewMode==2)
		Flags.Set(FFILEEDIT_DELETEONLYFILEONCLOSE);
}

void FileEditor::GetEditorOptions(EditorOptions& EdOpt)
{
	EdOpt = m_editor->EdOpt;
}

void FileEditor::SetEditorOptions(EditorOptions& EdOpt)
{
	m_editor->SetTabSize(EdOpt.TabSize);
	m_editor->SetConvertTabs(EdOpt.ExpandTabs);
	m_editor->SetPersistentBlocks(EdOpt.PersistentBlocks);
	m_editor->SetDelRemovesBlocks(EdOpt.DelRemovesBlocks);
	m_editor->SetAutoIndent(EdOpt.AutoIndent);
	m_editor->SetAutoDetectCodePage(EdOpt.AutoDetectCodePage);
	m_editor->SetCursorBeyondEOL(EdOpt.CursorBeyondEOL);
	m_editor->SetCharCodeBase(EdOpt.CharCodeBase);
	m_editor->SetSavePosMode(EdOpt.SavePos, EdOpt.SaveShortPos);
	m_editor->SetReadOnlyLock(EdOpt.ReadOnlyLock);
	m_editor->SetShowScrollBar(EdOpt.ShowScrollBar);
	m_editor->SetShowWhiteSpace(EdOpt.ShowWhiteSpace);
	m_editor->SetSearchPickUpWord(EdOpt.SearchPickUpWord);
	m_editor->SetBSLikeDel(EdOpt.BSLikeDel);
	m_editor->SetWordDiv(EdOpt.strWordDiv);
	m_editor->SetF7Rules(EdOpt.F7Rules);
	m_editor->SetAllowEmptySpaceAfterEof(EdOpt.AllowEmptySpaceAfterEof);
	m_editor->SetSearchSelFound(EdOpt.SearchSelFound);
	m_editor->SetSearchRegexp(EdOpt.SearchRegexp);
}

void FileEditor::OnChangeFocus(int focus)
{
	Frame::OnChangeFocus(focus);
	CtrlObject->Plugins->CurEditor=this;
	int FEditEditorID=m_editor->EditorID;
	CtrlObject->Plugins->ProcessEditorEvent(focus?EE_GOTFOCUS:EE_KILLFOCUS,nullptr,FEditEditorID);
}


int FileEditor::EditorControl(int Command, void *Param)
{
#if defined(SYSLOG_KEYMACRO)
	_KEYMACRO(CleverSysLog SL(L"FileEditor::EditorControl()"));

	if (Command == ECTL_READINPUT || Command == ECTL_PROCESSINPUT)
	{
		_KEYMACRO(SysLog(L"(Command=%s, Param=[%d/0x%08X]) Macro.IsExecuting()=%d",_ECTL_ToName(Command),(int)((intptr_t)Param),(int)((intptr_t)Param),CtrlObject->Macro.IsExecuting()));
	}

#else
	_ECTLLOG(CleverSysLog SL(L"FileEditor::EditorControl()"));
	_ECTLLOG(SysLog(L"(Command=%s, Param=[%d/0x%08X])",_ECTL_ToName(Command),(int)Param,Param));
#endif

	if(m_editor->EditorControlLocked()) return FALSE;
	if (m_bClosing && (Command != ECTL_GETINFO) && (Command != ECTL_GETBOOKMARKS) && (Command!=ECTL_GETFILENAME))
		return FALSE;

	switch (Command)
	{
		case ECTL_GETFILENAME:
		{
			if (Param)
			{
				wcscpy(static_cast<LPWSTR>(Param),strFullFileName);
			}

			return static_cast<int>(strFullFileName.GetLength()+1);
		}
		case ECTL_GETBOOKMARKS:
		{
			EditorBookMarks *ebm = static_cast<EditorBookMarks*>(Param);
			if (!Flags.Check(FFILEEDIT_OPENFAILED) && CheckStructSize(ebm))
			{
				for(int i = 0; i < BOOKMARK_COUNT; i++)
				{
					if (ebm->Line)
					{
						ebm->Line[i] = m_editor->SavePos.Line[i];
					}
					if (ebm->Cursor)
					{
						ebm->Cursor[i] = m_editor->SavePos.LinePos[i];
					}
					if (ebm->ScreenLine)
					{
						ebm->ScreenLine[i] = m_editor->SavePos.ScreenLine[i];
					}
					if (ebm->LeftPos)
					{
						ebm->LeftPos[i] = m_editor->SavePos.LeftPos[i];
					}
				}
				return TRUE;
			}

			return FALSE;
		}
		case ECTL_ADDSESSIONBOOKMARK:
		{
			return m_editor->AddSessionBookmark();
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
			return m_editor->ClearSessionBookmarks();
		}
		case ECTL_DELETESESSIONBOOKMARK:
		{
			return m_editor->DeleteSessionBookmark(m_editor->PointerToSessionBookmark((int)(intptr_t)Param));
		}
		case ECTL_GETSESSIONBOOKMARKS:
		{
			return CheckStructSize((EditorBookMarks *)Param)?m_editor->GetSessionBookmarks((EditorBookMarks *)Param):0;
		}
		case ECTL_SETTITLE:
		{
			strPluginTitle = (const wchar_t*)Param;
			ShowStatus();
			ScrBuf.Flush(); //???
			return TRUE;
		}
		case ECTL_REDRAW:
		{
			FileEditor::DisplayObject();
			ScrBuf.Flush();
			return TRUE;
		}
		/*
			Функция установки Keybar Labels
			Param = nullptr - восстановить, пред. значение
			Param = -1   - обновить полосу (перерисовать)
			Param = KeyBarTitles
		*/
		case ECTL_SETKEYBAR:
		{
			KeyBarTitles *Kbt = (KeyBarTitles*)Param;

			if (!Kbt)   //восстановить изначальное
				InitKeyBar();
			else
			{
				if ((intptr_t)Param != (intptr_t)-1) // не только перерисовать?
					EditKeyBar.Change(Kbt);

				EditKeyBar.Show();
			}

			return TRUE;
		}
		case ECTL_SAVEFILE:
		{
			string strName = strFullFileName;
			int EOL=0;
			UINT codepage=m_codepage;

			EditorSaveFile *esf=(EditorSaveFile *)Param;
			if (CheckStructSize(esf))
			{

				if (*esf->FileName) strName=esf->FileName;

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

				codepage=esf->CodePage;
			}

			{
				string strOldFullFileName = strFullFileName;

				if (SetFileName(strName))
				{
					if (StrCmpI(strFullFileName,strOldFullFileName))
					{
						if (!AskOverwrite(strName))
						{
							SetFileName(strOldFullFileName);
							return FALSE;
						}
					}

					Flags.Set(FFILEEDIT_SAVEWQUESTIONS);
					//всегда записываем в режиме save as - иначе не сменить кодировку и концы линий.
					return SaveFile(strName,FALSE,true,EOL,codepage,m_bAddSignature);
				}
			}

			return FALSE;
		}
		case ECTL_QUIT:
		{
			FrameManager->DeleteFrame(this);
			SetExitCode(SAVEFILE_ERROR); // что-то меня терзают смутные сомнения ...???
			return TRUE;
		}
		case ECTL_READINPUT:
		{
			if (CtrlObject->Macro.IsRecording() == MACROMODE_RECORDING || CtrlObject->Macro.IsExecuting() == MACROMODE_EXECUTING)
			{
//        return FALSE;
			}

			if (Param)
			{
				INPUT_RECORD *rec=(INPUT_RECORD *)Param;
				DWORD Key;

				for (;;)
				{
					Key=GetInputRecord(rec);

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
			if (Param)
			{
				INPUT_RECORD *rec=(INPUT_RECORD *)Param;

				if (ProcessEditorInput(rec))
					return TRUE;

				if (rec->EventType==MOUSE_EVENT)
					ProcessMouse(&rec->Event.MouseEvent);
				else
				{
#if defined(SYSLOG_KEYMACRO)

					if (!rec->EventType || rec->EventType == KEY_EVENT || rec->EventType == FARMACRO_KEY_EVENT)
					{
						SysLog(L"ECTL_PROCESSINPUT={%s,{%d,%d,Vk=0x%04X,0x%08X}}",
						       (rec->EventType == FARMACRO_KEY_EVENT?L"FARMACRO_KEY_EVENT":L"KEY_EVENT"),
						       rec->Event.KeyEvent.bKeyDown,
						       rec->Event.KeyEvent.wRepeatCount,
						       rec->Event.KeyEvent.wVirtualKeyCode,
						       rec->Event.KeyEvent.dwControlKeyState);
					}

#endif
					int Key=ShieldCalcKeyCode(rec,FALSE);
					ReProcessKey(Key);
				}

				return TRUE;
			}

			return FALSE;
		}
		case ECTL_SETPARAM:
		{
			EditorSetParameter *espar=(EditorSetParameter *)Param;
			if (CheckStructSize(espar))
			{
				if (ESPT_SETBOM==espar->Type)
				{
				    if(IsUnicodeOrUtfCodePage(m_codepage))
				    {
						m_bAddSignature=espar->iParam?true:false;
						return TRUE;
					}
					return FALSE;
				}
			}
			break;
		}
	}

	int result=m_editor->EditorControl(Command,Param);
	if (result&&Param&&ECTL_GETINFO==Command)
	{
		EditorInfo *Info=(EditorInfo *)Param;
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
	string strCacheName=strPluginData.IsEmpty()?strFullFileName:strPluginData+PointToName(strFullFileName);

	if (!Flags.Check(FFILEEDIT_OPENFAILED))   //????
	{
		pc.CodePage = (Flags.Check(FFILEEDIT_CODEPAGECHANGEDBYUSER) && !BadConversion)?m_codepage:0;

		FilePositionCache::AddPosition(strCacheName, pc);
	}
}

bool FileEditor::SetCodePage(UINT codepage)
{
	if (codepage == m_codepage || !m_editor)
		return false;

   int x, y;
	if (!m_editor->TryCodePage(codepage, x, y))
	{
		int ret = Message(MSG_WARNING, 3, MSG(MWarning),
			MSG(MEditorSwitchCPWarn1), MSG(MEditorSwitchCPWarn2), MSG(MEditorSwitchCPConfirm),
			MSG(MCancel), MSG(MEditorSaveCPWarnShow), MSG(MOk) );

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
	BadConversion = !m_editor->SetCodePage(m_codepage);
	return true;
}

bool FileEditor::AskOverwrite(const string& FileName)
{
	bool result=true;
	DWORD FNAttr=apiGetFileAttributes(FileName);

	if (FNAttr!=INVALID_FILE_ATTRIBUTES)
	{
		if (Message(MSG_WARNING,2,MSG(MEditTitle),FileName,MSG(MEditExists),MSG(MEditOvr),MSG(MYes),MSG(MNo)))
		{
			result=false;
		}
		else
		{
			Flags.Set(FFILEEDIT_SAVEWQUESTIONS);
		}
	}

	return result;
}
