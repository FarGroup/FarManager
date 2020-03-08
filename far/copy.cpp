﻿/*
copy.cpp

Копирование файлов
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

// Self:
#include "copy.hpp"

// Internal:
#include "keys.hpp"
#include "flink.hpp"
#include "dialog.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "foldtree.hpp"
#include "treelist.hpp"
#include "scantree.hpp"
#include "filefilter.hpp"
#include "fileview.hpp"
#include "syslog.hpp"
#include "cddrv.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "colormix.hpp"
#include "message.hpp"
#include "stddlg.hpp"
#include "fileattr.hpp"
#include "datetime.hpp"
#include "dirinfo.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "strmix.hpp"
#include "panelmix.hpp"
#include "processname.hpp"
#include "DlgGuid.hpp"
#include "console.hpp"
#include "lang.hpp"
#include "manager.hpp"
#include "copy_progress.hpp"
#include "string_utils.hpp"
#include "cvtname.hpp"
#include "exception.hpp"
#include "global.hpp"

// Platform:
#include "platform.env.hpp"
#include "platform.fs.hpp"

// Common:
#include "common.hpp"
#include "common/enum_tokens.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

enum
{
	SDDATA_SIZE = 64*1024,
};

enum
{
	COPY_RULE_NUL    = 0x0001,
	COPY_RULE_FILES  = 0x0002,
};

enum COPY_CODES: int
{
	COPY_CANCEL,
	COPY_SKIPPED,
	COPY_NOFILTER,                              // не считать размеры, т.к. файл не прошел по фильтру
	COPY_FAILURE,
	COPY_SUCCESS,
	COPY_SUCCESS_MOVE,
	COPY_RETRY,
};

enum COPY_FLAGS
{
	FCOPY_NONE                    = 0,
	FCOPY_COPYTONUL               = 0_bit, // Признак копирования в NUL
	FCOPY_CURRENTONLY             = 1_bit, // Только текущий?
	FCOPY_ONLYNEWERFILES          = 2_bit, // Copy only newer files
	FCOPY_LINK                    = 3_bit, // создание линков
	FCOPY_MOVE                    = 4_bit, // перенос/переименование
	FCOPY_DIZREAD                 = 5_bit, //
	FCOPY_VOLMOUNT                = 6_bit, // операция монтирования тома
	FCOPY_STREAMSKIPALL           = 7_bit, // потоки
	FCOPY_COPYSYMLINKCONTENTS     = 8_bit, // Копировать содержимое символических ссылок?
	FCOPY_DECRYPTED_DESTINATION   = 9_bit, // для криптованных файлов - расшифровывать...
	FCOPY_USESYSTEMCOPY           = 10_bit, // использовать системную функцию копирования
	FCOPY_COPYLASTTIME            = 11_bit, // При копировании в несколько каталогов устанавливается для последнего.
	FCOPY_UPDATEPPANEL            = 12_bit, // необходимо обновить пассивную панель
};

template<typename times_type>
static bool set_file_time(const os::fs::file& File, const times_type& Times)
{
	return File.SetTime(&Times.CreationTime, &Times.LastAccessTime, &Times.LastWriteTime, &Times.ChangeTime);
}

static const struct
{
	int CopyFlag;    // по умолчанию выставлять опцию "Copy access rights"?
	int InheritFlag; // по умолчанию выставлять опцию "Inherit access rights"?
	int SaveFlag;    // сохранять состояние "access rights" внутри сессии?
	mutable std::optional<ShellCopy::security> SavedState;
}
SecurityMove
{
	// These static_casts are redundant and only to suppress a spurious warning C4838 in VS2017.
	// See https://github.com/FarGroup/FarManager/issues/166
	// TODO: remove once we drop support for VS2017.
	static_cast<int>(0_bit),
	static_cast<int>(0_bit | 1_bit),
	static_cast<int>(2_bit),
},
SecurityCopy
{
	static_cast<int>(3_bit),
	static_cast<int>(3_bit | 4_bit),
	static_cast<int>(5_bit),
};

static bool ZoomedState, IconicState;

static const size_t default_copy_buffer_size = 32 * 1024;

enum enumShellCopy
{
	ID_SC_TITLE,
	ID_SC_TARGETTITLE,
	ID_SC_TARGETEDIT,
	ID_SC_SEPARATOR1,
	ID_SC_SECURITY_TITLE,
	ID_SC_SECURITY_DEFAULT,
	ID_SC_SECURITY_COPY,
	ID_SC_SECURITY_INHERIT,
	ID_SC_SEPARATOR2,
	ID_SC_COMBOTEXT,
	ID_SC_COMBO,
	IS_SC_PRESERVETIMESTAMPS,
	ID_SC_COPYSYMLINK,
	ID_SC_MULTITARGET,
	ID_SC_SEPARATOR3,
	ID_SC_USEFILTER,
	ID_SC_SEPARATOR4,
	ID_SC_BTNCOPY,
	ID_SC_BTNTREE,
	ID_SC_BTNFILTER,
	ID_SC_BTNCANCEL,

	ID_SC_COUNT
};

enum CopyMode
{
	CM_ASK,
	CM_OVERWRITE,
	CM_SKIP,
	CM_RENAME,
	CM_APPEND,
	CM_ONLYNEWER,
	CM_SEPARATOR,
	CM_ASKRO,
};

/* $ 25.05.2002 IS
 + Всегда работаем с реальными _длинными_ именами, в результате чего
   отлавливается ситуация, когда
   Src="D:\Program Files\filename"
   Dest="D:\PROGRA~1\filename"
   ("D:\PROGRA~1" - короткое имя для "D:\Program Files")
   считается, что имена тоже одинаковые, а раньше считалось,
   что они разные (функция не знала, что и в первом, и во втором случае
   путь один и тот же)
 ! Оптимизация - "велосипед" заменен на DeleteEndSlash
 ! Убираем всю самодеятельность по проверке имен с разным
   регистром из функции прочь, потому что это нужно делать только при
   переименовании, а функция вызывается и при копировании тоже.
   Теперь функция вернет 1, для случая имен src=path\filename,
   dest=path\filename (раньше возвращала 2 - т.е. сигнал об ошибке).
*/

static int CmpFullNames(const string& Src,const string& Dest)
{
	const auto ToFull = [](const string& in)
	{
		// получим полные пути с учетом символических связей
		// (ConvertNameToReal eliminates short names too)
		auto out = ConvertNameToReal(in);
		DeleteEndSlash(out);
		return out;
	};

	return equal_icase(ToFull(Src), ToFull(Dest));
}

static bool CheckNulOrCon(string_view Src)
{
	if (HasPathPrefix(Src))
		Src.remove_prefix(4);

	return (starts_with_icase(Src, L"nul"sv) || starts_with_icase(Src, L"con"sv)) && (Src.size() == 3 || (Src.size() > 3 && IsSlash(Src[3])));
}

static void GenerateName(string &strName, const string& Path)
{
	if (!Path.empty())
		strName = path::join(Path, PointToName(strName));

	// The source string will be altered below so the view must be copied
	const string Ext(PointToExt(strName));
	const auto NameLength = strName.size() - Ext.size();

	// file (2).ext, file (3).ext and so on
	for (int i = 2; os::fs::exists(strName); ++i)
	{
		strName.resize(NameLength);
		append(strName, L" ("sv, str(i), L')', Ext);
	}
}

static void CheckAndUpdateConsole()
{
	const auto hWnd = console.GetWindow();
	if (ZoomedState != (IsZoomed(hWnd) != FALSE) && IconicState == (IsIconic(hWnd) != FALSE))
	{
		ZoomedState = !ZoomedState;
		ChangeVideoMode(ZoomedState != FALSE);
	}
}

enum
{
	DM_CALLTREE = DM_USER+1,
	DM_SWITCHRO = DM_USER+2,
};

intptr_t ShellCopy::CopyDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	switch (Msg)
	{
		case DN_INITDIALOG:
			Dlg->SendMessage(DM_SETCOMBOBOXEVENT,ID_SC_COMBO,ToPtr(CBET_KEY|CBET_MOUSE));
			Dlg->SendMessage(DM_SETINPUTNOTIFY, TRUE, nullptr);
			break;
		case DM_SWITCHRO:
		{
			FarListGetItem LGI={sizeof(FarListGetItem),CM_ASKRO};
			Dlg->SendMessage(DM_LISTGETITEM,ID_SC_COMBO,&LGI);

			if (LGI.Item.Flags&LIF_CHECKED)
				LGI.Item.Flags&=~LIF_CHECKED;
			else
				LGI.Item.Flags|=LIF_CHECKED;

			Dlg->SendMessage(DM_LISTUPDATE,ID_SC_COMBO,&LGI);
			Dlg->SendMessage(DM_REDRAW, 0, nullptr);
			return TRUE;
		}
		case DN_BTNCLICK:
		{
			if (Param1==ID_SC_USEFILTER) // "Use filter"
			{
				m_UseFilter = static_cast<FARCHECKEDSTATE>(reinterpret_cast<intptr_t>(Param2)) == BSTATE_CHECKED;
				return TRUE;
			}

			if (Param1 == ID_SC_BTNTREE) // Tree
			{
				Dlg->SendMessage(DM_CALLTREE, 0, nullptr);
				return FALSE;
			}
			else if (Param1 == ID_SC_BTNCOPY)
			{
				Dlg->SendMessage(DM_CLOSE, ID_SC_BTNCOPY, nullptr);
			}
			else if (Param1==ID_SC_BTNFILTER) // Filter
			{
				m_Filter->FilterEdit();
				return TRUE;
			}

			break;
		}
		case DN_CONTROLINPUT: // по поводу дерева!
		{
			const auto& record = *static_cast<const INPUT_RECORD*>(Param2);
			if (record.EventType==KEY_EVENT)
			{
				const auto key = InputRecordToKey(&record);
				if (!Global->Opt->Tree.TurnOffCompletely)
				{
					if (key == KEY_ALTF10 || key == KEY_RALTF10 || key == KEY_F10 || key == KEY_SHIFTF10)
					{
						AltF10 = (key == KEY_ALTF10 || key == KEY_RALTF10) ? 1 : (key == KEY_SHIFTF10 ? 2 : 0);
						Dlg->SendMessage(DM_CALLTREE, AltF10, nullptr);
						return TRUE;
					}
				}

				if (Param1 == ID_SC_COMBO)
				{
					if (Dlg->SendMessage(DM_LISTGETCURPOS, ID_SC_COMBO, nullptr) == CM_ASKRO)
					{
						if (key==KEY_ENTER || key==KEY_NUMENTER || key==KEY_INS || key==KEY_NUMPAD0 || key==KEY_SPACE)
						{
							return Dlg->SendMessage(DM_SWITCHRO, 0, nullptr);
						}
						else if (key == KEY_TAB)
						{
							Dlg->SendMessage(DM_SETDROPDOWNOPENED, 0, nullptr);
							return TRUE;
						}
					}
				}
			}
		}
		break;

		case DN_LISTHOTKEY:
			if(Param1==ID_SC_COMBO)
			{
				const auto Index = reinterpret_cast<intptr_t>(Param2);
				if (Index == CM_ASKRO)
				{
					Dlg->SendMessage(DM_SWITCHRO, 0, nullptr);
					FarListPos flp = { sizeof(flp), Index, -1 };
					Dlg->SendMessage(DM_LISTSETCURPOS, Param1, &flp);
					return TRUE;
				}
			}
			break;
		case DN_INPUT:
			{
				const auto ir = static_cast<const INPUT_RECORD*>(Param2);
				if (ir->EventType == MOUSE_EVENT && Dlg->SendMessage(DM_GETDROPDOWNOPENED, ID_SC_COMBO, nullptr))
				{
					if (Dlg->SendMessage(DM_LISTGETCURPOS, ID_SC_COMBO, nullptr) == CM_ASKRO && ir->Event.MouseEvent.dwButtonState && !(ir->Event.MouseEvent.dwEventFlags & MOUSE_MOVED))
					{
						Dlg->SendMessage(DM_SWITCHRO, 0, nullptr);
						return FALSE;
					}
				}
			}
			break;
		case DM_CALLTREE:
		{
			/* $ 13.10.2001 IS
			   + При мультикопировании добавляем выбранный в "дереве" каталог к уже
			     существующему списку через точку с запятой.
			   - Баг: при мультикопировании выбранный в "дереве" каталог не
			     заключался в кавычки, если он содержал в своем
			     имени символы-разделители.
			   - Баг: неправильно работало Shift-F10, если строка ввода содержала
			     слеш на конце.
			   - Баг: неправильно работало Shift-F10 при мультикопировании -
			     показывался корневой каталог, теперь показывается самый первый каталог
			     в списке.
			*/
			const auto MultiCopy = Dlg->SendMessage(DM_GETCHECK, ID_SC_MULTITARGET, nullptr) == BSTATE_CHECKED;
			string strOldFolder = reinterpret_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, ID_SC_TARGETEDIT, nullptr));
			string strNewFolder;

			if (AltF10 == 2)
			{
				strNewFolder = strOldFolder;

				if (MultiCopy)
				{
					for (const auto& i: enum_tokens_with_quotes_t<with_trim>(strOldFolder, L",;"sv))
					{
						if (i.empty())
							continue;

						strNewFolder = i;
						break;
					}
				}

				if (strNewFolder.empty())
					AltF10=-1;
				else // убираем лишний слеш
					DeleteEndSlash(strNewFolder);
			}

			if (AltF10 != -1)
			{
				{
					FolderTree::create(strNewFolder,
					                (AltF10==1?MODALTREE_PASSIVE:
					                 (AltF10==2?MODALTREE_FREE:
					                  MODALTREE_ACTIVE)),
					                FALSE, false);
				}

				if (!strNewFolder.empty())
				{
					AddEndSlash(strNewFolder);

					if (MultiCopy) // мультикопирование
					{
						// Добавим кавычки, если имя каталога содержит символы-разделители
						if (strNewFolder.find_first_of(L",;"sv) != string::npos)
							inplace::quote(strNewFolder);

						if (!strOldFolder.empty())
							strOldFolder += L';'; // добавим разделитель к непустому списку

						strNewFolder.insert(0, strOldFolder);
					}

					Dlg->SendMessage(DM_SETTEXTPTR,ID_SC_TARGETEDIT, UNSAFE_CSTR(strNewFolder));
					Dlg->SendMessage(DM_SETFOCUS, ID_SC_TARGETEDIT, nullptr);
				}
			}

			AltF10=0;
			return TRUE;
		}
		case DN_CLOSE:
		{
			if (Param1==ID_SC_BTNCOPY)
			{
				FarListGetItem LGI={sizeof(FarListGetItem),CM_ASKRO};
				Dlg->SendMessage(DM_LISTGETITEM,ID_SC_COMBO,&LGI);

				if (LGI.Item.Flags&LIF_CHECKED)
					AskRO = true;
			}
		}
		break;

		default:
			break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

ShellCopy::ShellCopy(panel_ptr SrcPanel,     // исходная панель (активная)
                     bool Move,               // =1 - операция Move
                     bool Link,               // =1 - Sym/Hard Link
                     bool CurrentOnly,        // =1 - только текущий файл, под курсором
                     bool Ask,                // =1 - выводить диалог?
                     int& ToPlugin,          // =?
                     string* PluginDestPath,
                     bool ToSubdir):
	m_Filter(std::make_unique<FileFilter>(SrcPanel.get(), FFT_COPY)),
	Flags((Move? FCOPY_MOVE : FCOPY_NONE) | (Link? FCOPY_LINK : FCOPY_NONE) | (CurrentOnly? FCOPY_CURRENTONLY : FCOPY_NONE)),
	SrcPanel(SrcPanel),
	DestPanel(Global->CtrlObject->Cp()->GetAnotherPanel(SrcPanel)),
	SrcPanelMode(SrcPanel->GetMode()),
	DestPanelMode(ToPlugin? DestPanel->GetMode() : panel_mode::NORMAL_PANEL),
	SrcDriveType(),
	DestDriveType(),
	CopyBufferSize(!Global->Opt->CMOpt.BufferSize.Get()? default_copy_buffer_size : Global->Opt->CMOpt.BufferSize.Get()),
	OvrMode(-1),
	ReadOnlyOvrMode(-1),
	ReadOnlyDelMode(-1),
	SkipEncMode(-1),
	SelectedFolderNameLength(),
	RPT(RP_EXACTCOPY),
	AltF10(),
	SelCount(SrcPanel->GetSelCount()),
	FolderPresent(),
	FilesPresent(),
	AskRO(),
	m_UseFilter(),
	m_FileHandleForStreamSizeFix(),
	m_NumberOfTargets()
{
	if (!SelCount)
		return;

	string SingleSelName;
	DWORD SingleSelAttributes = 0;
	unsigned long long SingleSelectedFileSize = 0;

	if (SelCount==1)
	{
		os::fs::find_data Data;
		if (!SrcPanel->get_first_selected(Data))
			return;

		if (IsParentDirectory(Data))
			return;

		SingleSelAttributes = Data.Attributes;
		SingleSelName = Data.FileName;
		SingleSelectedFileSize = Data.FileSize;
	}

	ZoomedState = IsZoomed(console.GetWindow()) != FALSE;
	IconicState = IsIconic(console.GetWindow()) != FALSE;
	bool ShowTotalCopySize = Global->Opt->CMOpt.CopyShowTotal;
	auto DestPlugin = ToPlugin;
	ToPlugin = 0;

	// ***********************************************************************
	// *** Prepare Dialog Controls
	// ***********************************************************************
	int DlgW = 76, DlgH = 17;

	FARDIALOGITEMFLAGS no_tree = Global->Opt->Tree.TurnOffCompletely ? DIF_HIDDEN|DIF_DISABLE : 0;

	auto CopyDlg = MakeDialogItems<ID_SC_COUNT>(
	{
		{ DI_DOUBLEBOX,    {{3,  1 }, {DlgW - 4, DlgH - 2}}, DIF_NONE, msg(lng::MCopyDlgTitle), },
		{ DI_TEXT,         {{5,  2 }, {0,  2 }}, DIF_NONE, },
		{ DI_EDIT,         {{5,  3 }, {70, 3 }}, DIF_FOCUS | DIF_HISTORY | DIF_USELASTHISTORY | DIF_EDITPATH, },
		{ DI_TEXT,         {{-1, 4 }, {0,  4 }}, DIF_SEPARATOR, },
		{ DI_TEXT,         {{5,  5 }, {0,  5 }}, DIF_NONE, msg(lng::MCopySecurity), },
		{ DI_RADIOBUTTON,  {{5,  5 }, {0,  5 }}, DIF_GROUP, msg(lng::MCopySecurityDefault), },
		{ DI_RADIOBUTTON,  {{5,  5 }, {0,  5 }}, DIF_NONE, msg(lng::MCopySecurityCopy), },
		{ DI_RADIOBUTTON,  {{5,  5 }, {0,  5 }}, DIF_NONE, msg(lng::MCopySecurityInherit), },
		{ DI_TEXT,         {{-1, 6 }, {0,  6 }}, DIF_SEPARATOR, },
		{ DI_TEXT,         {{5,  7 }, {0,  7 }}, DIF_NONE, msg(lng::MCopyIfFileExist), },
		{ DI_COMBOBOX,     {{29, 7 }, {70, 7 }}, DIF_DROPDOWNLIST | DIF_LISTNOAMPERSAND | DIF_LISTWRAPMODE, },
		{ DI_CHECKBOX,     {{5,  8 }, {0,  8 }}, DIF_NONE, msg(lng::MCopyPreserveTimestamps), },
		{ DI_CHECKBOX,     {{5,  9 }, {0,  9 }}, DIF_NONE, msg(lng::MCopySymLinkContents), },
		{ DI_CHECKBOX,     {{5,  10}, {0,  10}}, DIF_NONE, msg(lng::MCopyMultiActions), },
		{ DI_TEXT,         {{-1, 11}, {0,  11}}, DIF_SEPARATOR, },
		{ DI_CHECKBOX,     {{5,  12}, {0,  12}}, DIF_AUTOMATION, msg(lng::MCopyUseFilter), },
		{ DI_TEXT,         {{-1, 13}, {0,  13}}, DIF_SEPARATOR, },
		{ DI_BUTTON,       {{0,  14}, {0,  14}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MCopyDlgCopy), },
		{ DI_BUTTON,       {{0,  14}, {0,  14}}, DIF_CENTERGROUP | DIF_BTNNOCLOSE | no_tree, msg(lng::MCopyDlgTree), },
		{ DI_BUTTON,       {{0,  14}, {0,  14}}, DIF_CENTERGROUP | DIF_BTNNOCLOSE | DIF_AUTOMATION | (m_UseFilter ? DIF_NONE : DIF_DISABLE), msg(lng::MCopySetFilter), },
		{ DI_BUTTON,       {{0,  14}, {0,  14}}, DIF_CENTERGROUP, msg(lng::MCopyDlgCancel), },
	});

	CopyDlg[ID_SC_TARGETEDIT].strHistory = L"Copy"sv;
	CopyDlg[IS_SC_PRESERVETIMESTAMPS].Selected = Global->Opt->CMOpt.PreserveTimestamps;
	CopyDlg[ID_SC_MULTITARGET].Selected = Global->Opt->CMOpt.MultiCopy;
	CopyDlg[ID_SC_USEFILTER].Selected = m_UseFilter;

	{
		{
			const auto& Str = msg(lng::MCopySecurity);
			CopyDlg[ID_SC_SECURITY_DEFAULT].X1 = CopyDlg[ID_SC_SECURITY_TITLE].X1 + Str.size() - (contains(Str, L'&')? 1 : 0) + 1;
		}
		{
			const auto& Str = msg(lng::MCopySecurityDefault);
			CopyDlg[ID_SC_SECURITY_COPY].X1 = CopyDlg[ID_SC_SECURITY_DEFAULT].X1 + Str.size() - (contains(Str, L'&')? 1 : 0) + 5;
		}
		{
			const auto& Str = msg(lng::MCopySecurityCopy);
			CopyDlg[ID_SC_SECURITY_INHERIT].X1 = CopyDlg[ID_SC_SECURITY_COPY].X1 + Str.size() - (contains(Str, L'&')? 1 : 0) + 5;
		}
	}

	if (Link)
	{
		CopyDlg[ID_SC_COMBOTEXT].strData=msg(lng::MLinkType);
		CopyDlg[ID_SC_COPYSYMLINK].Selected=0;
		CopyDlg[ID_SC_COPYSYMLINK].Flags|=DIF_DISABLE|DIF_HIDDEN;
	}
	else
	{
		if (Move) // секция про перенос
		{
			CopyDlg[ID_SC_MULTITARGET].Selected = 0;
			CopyDlg[ID_SC_MULTITARGET].Flags |= DIF_DISABLE;
		}

		auto& CurrentState = Move? SecurityMove : SecurityCopy;

		// ставить опцию "Inherit access rights"?
		// двухбитный флаг
		if ((Global->Opt->CMOpt.CopySecurityOptions & CurrentState.InheritFlag) == CurrentState.InheritFlag)
			m_CopySecurity = security::inherit;
		else if (Global->Opt->CMOpt.CopySecurityOptions & CurrentState.CopyFlag)
			m_CopySecurity = security::copy;

		// хотели сессионное запоминание?
		if (CurrentState.SavedState && (Global->Opt->CMOpt.CopySecurityOptions & CurrentState.SaveFlag))
			m_CopySecurity = *CurrentState.SavedState;
		else
			CurrentState.SavedState = m_CopySecurity;
	}

	// вот теперь выставляем
	switch (m_CopySecurity)
	{
	case security::do_nothing:
		CopyDlg[ID_SC_SECURITY_DEFAULT].Selected = 1;
		break;

	case security::copy:
		CopyDlg[ID_SC_SECURITY_COPY].Selected = 1;
		break;

	case security::inherit:
		CopyDlg[ID_SC_SECURITY_INHERIT].Selected = 1;
		break;
	}

	string strCopyStr;

	if (SelCount==1)
	{
		if (SrcPanel->GetType() == panel_type::TREE_PANEL)
		{
			auto strNewDir = SingleSelName;
			const auto pos = FindLastSlash(strNewDir);
			if (pos != string::npos)
			{
				strNewDir.resize(pos);

				if (!pos || strNewDir[pos-1]==L':')
					AddEndSlash(strNewDir);

				FarChDir(strNewDir);
			}
		}

		const auto Format = msg(Move? lng::MMoveFile : Link? lng::MLinkFile : lng::MCopyFile);
		const auto& ToOrIn = msg(Link? lng::MCMLTargetIN : lng::MCMLTargetTO);
		const auto SpaceAvailable = std::max(0, static_cast<int>(CopyDlg[ID_SC_TITLE].X2 - CopyDlg[ID_SC_TITLE].X1 - 1 - 1));
		if (const auto MaxLength = std::max(0, SpaceAvailable - static_cast<int>(HiStrlen(format(Format, L""sv, ToOrIn)))))
		{
			strCopyStr = truncate_right(SingleSelName, MaxLength);
		}
		strCopyStr = format(Format, strCopyStr, ToOrIn);

		// Если копируем одиночный файл, то запрещаем использовать фильтр
		if (!(SingleSelAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			CopyDlg[ID_SC_USEFILTER].Selected=0;
			CopyDlg[ID_SC_USEFILTER].Flags|=DIF_DISABLE;
		}
	}
	else // Объектов несколько!
	{

		// коррекция языка - про окончания
		const auto StrItems = str(SelCount);
		const auto LenItems = StrItems.size();
		auto NItems = lng::MCMLItemsA;

		if ((LenItems >= 2 && StrItems[LenItems-2] == '1') || StrItems[LenItems-1] >= '5' || StrItems[LenItems-1] == '0')
			NItems = lng::MCMLItemsS;
		else if (StrItems[LenItems-1] == '1')
			NItems = lng::MCMLItems0;

		strCopyStr = format(msg(Move? lng::MMoveFiles : Link? lng::MLinkFiles : lng::MCopyFiles),
			SelCount,
			msg(NItems),
			msg(Link? lng::MCMLTargetIN : lng::MCMLTargetTO));
	}

	CopyDlg[ID_SC_TARGETTITLE].strData=strCopyStr;
	CopyDlg[ID_SC_TITLE].strData = msg(Move? lng::MMoveDlgTitle : Link? lng::MLinkDlgTitle : lng::MCopyDlgTitle);
	CopyDlg[ID_SC_BTNCOPY].strData = msg(Move? lng::MCopyDlgRename: Link? lng::MCopyDlgLink : lng::MCopyDlgCopy);

	if (DestPanelMode == panel_mode::PLUGIN_PANEL)
	{
		// Если противоположная панель - плагин, то дисаблим OnlyNewer //?????
/*
		CopySecurity=2;
		CopyDlg[ID_SC_ACCOPY].Selected=0;
		CopyDlg[ID_SC_ACINHERIT].Selected=0;
		CopyDlg[ID_SC_ACLEAVE].Selected=1;
		CopyDlg[ID_SC_ACCOPY].Flags|=DIF_DISABLE;
		CopyDlg[ID_SC_ACINHERIT].Flags|=DIF_DISABLE;
		CopyDlg[ID_SC_ACLEAVE].Flags|=DIF_DISABLE;
*/
	}

	string strDestDir(DestPanel->GetCurDir());
	if(ToSubdir)
	{
		string strSubdir, strShort;
		if (DestPanel->GetCurName(strSubdir, strShort))
			path::append(strDestDir, strSubdir);
	}
	string strSrcDir(SrcPanel->GetCurDir());

	if (CurrentOnly)
	{
		// При копировании только элемента под курсором берем его имя в кавычки, если оно содержит разделители.
		CopyDlg[ID_SC_TARGETEDIT].strData = SingleSelName;

		if (Ask && !Move && CopyDlg[ID_SC_TARGETEDIT].strData.find_first_of(L",;"sv) != string::npos)
		{
			inplace::quote(CopyDlg[ID_SC_TARGETEDIT].strData);
		}
	}
	else
	{
		switch (DestPanelMode)
		{
		case panel_mode::NORMAL_PANEL:
			{
				if ((strDestDir.empty() || !DestPanel->IsVisible() || equal_icase(strSrcDir, strDestDir)) && SelCount==1)
					CopyDlg[ID_SC_TARGETEDIT].strData = SingleSelName;
				else
				{
					CopyDlg[ID_SC_TARGETEDIT].strData = strDestDir;
					AddEndSlash(CopyDlg[ID_SC_TARGETEDIT].strData);
				}

				/* $ 19.07.2003 IS
				   Если цель содержит разделители, то возьмем ее в кавычки, дабы не получить
				   ерунду при F5, Enter в панелях, когда пользователь включит MultiCopy
				*/
				if (Ask &&!Move && CopyDlg[ID_SC_TARGETEDIT].strData.find_first_of(L",;"sv) != string::npos)
				{
					// возьмем в кавычки, т.к. могут быть разделители
					inplace::quote(CopyDlg[ID_SC_TARGETEDIT].strData);
				}

				break;
			}

		case panel_mode::PLUGIN_PANEL:
			{
				OpenPanelInfo Info{};
				DestPanel->GetOpenPanelInfo(&Info);
				CopyDlg[ID_SC_TARGETEDIT].strData = Info.Format? concat(Info.Format, L':') : L"::"sv;
				strPluginFormat = upper(CopyDlg[ID_SC_TARGETEDIT].strData);
				break;
			}
		}
	}

	string strInitDestDir = CopyDlg[ID_SC_TARGETEDIT].strData;
	// Для фильтра

	bool AddSlash=false;

	for (const auto& i: SrcPanel->enum_selected())
	{
		if (m_UseFilter && !m_Filter->FileInFilter(i, nullptr, &i.FileName))
			continue;

		if (i.Attributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			FolderPresent=true;
			AddSlash=true;
		}
		else
		{
			FilesPresent=true;
		}
	}

	if (Link) // рулесы по поводу линков (предварительные!)
	{
		// задисаблим опцию про копирование права.
		CopyDlg[ID_SC_SECURITY_TITLE].Flags   |= DIF_HIDDEN | DIF_DISABLE;
		CopyDlg[ID_SC_SECURITY_DEFAULT].Flags |= DIF_HIDDEN | DIF_DISABLE;
		CopyDlg[ID_SC_SECURITY_COPY].Flags    |= DIF_HIDDEN | DIF_DISABLE;
		CopyDlg[ID_SC_SECURITY_INHERIT].Flags |= DIF_HIDDEN | DIF_DISABLE;
		CopyDlg[ID_SC_SEPARATOR2].Flags       |= DIF_HIDDEN;

		for(int i=ID_SC_SEPARATOR2;i<=ID_SC_COMBO;i++)
		{
			CopyDlg[i].Y1-=2;
			CopyDlg[i].Y2-=2;
		}
		for(int i=ID_SC_MULTITARGET;i<=ID_SC_BTNCANCEL;i++)
		{
			CopyDlg[i].Y1-=3;
			CopyDlg[i].Y2-=3;
		}
		CopyDlg[ID_SC_TITLE].Y2-=3;
		DlgH-=3;
	}

	/* $ 15.06.2002 IS
	   Обработка копирования мышкой - в этом случае диалог не показывается,
	   но переменные все равно инициализируются. Если произойдет неудачная
	   компиляция списка целей, то покажем диалог.
	*/
	string strCopyDlgValue;
	if (!Ask)
	{
		strCopyDlgValue = CopyDlg[ID_SC_TARGETEDIT].strData;
		m_DestList = { strCopyDlgValue };
	}
	else
	{
		// ***********************************************************************
		// *** Вывод и обработка диалога
		// ***********************************************************************
		FarList ComboList={sizeof(FarList)};
		FarListItem LinkTypeItems[5]={},CopyModeItems[8]={};

		if (Link)
		{
			ComboList.ItemsNumber=std::size(LinkTypeItems);
			ComboList.Items=LinkTypeItems;
			ComboList.Items[0].Text = msg(lng::MLinkTypeHardlink).c_str();
			ComboList.Items[1].Text = msg(lng::MLinkTypeJunction).c_str();
			ComboList.Items[2].Text = msg(lng::MLinkTypeSymlink).c_str();
			ComboList.Items[3].Text = msg(lng::MLinkTypeSymlinkFile).c_str();
			ComboList.Items[4].Text = msg(lng::MLinkTypeSymlinkDirectory).c_str();

			if (FilesPresent)
				ComboList.Items[0].Flags|=LIF_SELECTED;
			else
				ComboList.Items[1].Flags|=LIF_SELECTED;
		}
		else
		{
			ComboList.ItemsNumber=std::size(CopyModeItems);
			ComboList.Items=CopyModeItems;
			ComboList.Items[CM_ASK].Text = msg(lng::MCopyAsk).c_str();
			ComboList.Items[CM_OVERWRITE].Text = msg(lng::MCopyOverwrite).c_str();
			ComboList.Items[CM_SKIP].Text = msg(lng::MCopySkip).c_str();
			ComboList.Items[CM_RENAME].Text = msg(lng::MCopyRename).c_str();
			ComboList.Items[CM_APPEND].Text = msg(lng::MCopyAppend).c_str();
			ComboList.Items[CM_ONLYNEWER].Text = msg(lng::MCopyOnlyNewerFiles).c_str();
			ComboList.Items[CM_ASKRO].Text = msg(lng::MCopyAskRO).c_str();
			ComboList.Items[CM_ASK].Flags=LIF_SELECTED;
			ComboList.Items[CM_SEPARATOR].Flags=LIF_SEPARATOR;

			if (Global->Opt->Confirm.RO)
			{
				ComboList.Items[CM_ASKRO].Flags=LIF_CHECKED;
			}
		}

		CopyDlg[ID_SC_COMBO].ListItems=&ComboList;
		const auto Dlg = Dialog::create(CopyDlg, &ShellCopy::CopyDlgProc, this);
		Dlg->SetHelp(Link? L"HardSymLink"sv : L"CopyFiles"sv);
		Dlg->SetId(Link?HardSymLinkId:(Move?(CurrentOnly?MoveCurrentOnlyFileId:MoveFilesId):(CurrentOnly?CopyCurrentOnlyFileId:CopyFilesId)));
		Dlg->SetPosition({ -1, -1, DlgW, DlgH });
		Dlg->SetAutomation(ID_SC_USEFILTER,ID_SC_BTNFILTER,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
//    Dlg->Show();
		// $ 02.06.2001 IS + Проверим список целей и поднимем тревогу, если он содержит ошибки
		int DlgExitCode;

		for (;;)
		{
			Dlg->ClearDone();
			Dlg->Process();
			DlgExitCode=Dlg->GetExitCode();
			//Рефреш текущему времени для фильтра сразу после выхода из диалога
			m_Filter->UpdateCurrentTime();

			if (DlgExitCode == ID_SC_BTNCOPY)
			{
				/* $ 03.08.2001 IS
				   Запомним строчку из диалога и начинаем ее мучить в зависимости от
				   состояния опции мультикопирования
				*/
				auto tmp = strCopyDlgValue = CopyDlg[ID_SC_TARGETEDIT].strData;
				DeleteEndSlash(tmp);
				if (tmp != DestPanel->GetCurDir())
					strCopyDlgValue = os::env::expand(strCopyDlgValue);

				Global->Opt->CMOpt.PreserveTimestamps = CopyDlg[IS_SC_PRESERVETIMESTAMPS].Selected == BSTATE_CHECKED;

				if(!Move)
				{
					Global->Opt->CMOpt.MultiCopy=CopyDlg[ID_SC_MULTITARGET].Selected == BSTATE_CHECKED;
				}

				if (!CopyDlg[ID_SC_MULTITARGET].Selected)
				{
					m_DestList = { unquote(strCopyDlgValue) };
				}
				else
				{
					if (strCopyDlgValue.find_first_of(L",;"sv) == string::npos)
					{
						m_DestList = { unquote(strCopyDlgValue) };
					}
					else
					{
						for (const auto& i: enum_tokens_with_quotes_t<with_trim>(strCopyDlgValue, L",;"sv))
						{
							m_DestList.emplace_back(i);
						}
					}
				}

				if (!m_DestList.empty())
				{
					// Запомнить признак использования фильтра. KM
					m_UseFilter = CopyDlg[ID_SC_USEFILTER].Selected == BSTATE_CHECKED;
					break;
				}
				else
				{
					Message(MSG_WARNING,
						msg(lng::MWarning),
						{
							msg(lng::MCopyIncorrectTargetList)
						},
						{ lng::MOk });
				}
			}
			else
				break;
		}

		if (DlgExitCode == ID_SC_BTNCANCEL || DlgExitCode < 0 || (CopyDlg[ID_SC_BTNCOPY].Flags&DIF_DISABLE))
		{
			if (DestPlugin)
				ToPlugin=-1;

			return;
		}
	}

	// ***********************************************************************
	// *** Стадия подготовки данных после диалога
	// ***********************************************************************
	if (CopyDlg[ID_SC_SECURITY_COPY].Selected)
	{
		m_CopySecurity = security::copy;
	}
	else if (CopyDlg[ID_SC_SECURITY_INHERIT].Selected)
	{
		m_CopySecurity = security::inherit;
	}
	else
	{
		m_CopySecurity = security::do_nothing;
	}

	if (Global->Opt->CMOpt.UseSystemCopy)
		Flags|=FCOPY_USESYSTEMCOPY;
	else
		Flags&=~FCOPY_USESYSTEMCOPY;

	// в любом случае сохраняем сессионное запоминание (не для Link, т.к. для Link временное состояние - "ВСЕГДА!")
	if (!Link)
	{
		(Move? SecurityMove : SecurityCopy).SavedState = m_CopySecurity;
	}

	if (Link)
	{
		switch (CopyDlg[ID_SC_COMBO].ListPos)
		{
			case 0:
				RPT=RP_HARDLINK;
				break;
			case 1:
				RPT=RP_JUNCTION;
				break;
			case 2:
				RPT=RP_SYMLINK;
				break;
			case 3:
				RPT=RP_SYMLINKFILE;
				break;
			case 4:
				RPT=RP_SYMLINKDIR;
				break;
		}
	}
	else
	{
		ReadOnlyOvrMode=AskRO?-1:1;

		switch (CopyDlg[ID_SC_COMBO].ListPos)
		{
			case CM_ASK:
				OvrMode=-1;
				break;
			case CM_OVERWRITE:
				OvrMode=1;
				break;
			case CM_SKIP:
				OvrMode=3;
				ReadOnlyOvrMode=AskRO?-1:3;
				break;
			case CM_RENAME:
				OvrMode=5;
				break;
			case CM_APPEND:
				OvrMode=7;
				break;
			case CM_ONLYNEWER:
				Flags|=FCOPY_ONLYNEWERFILES;
				break;
		}
	}

	Flags|=CopyDlg[ID_SC_COPYSYMLINK].Selected? FCOPY_COPYSYMLINKCONTENTS : FCOPY_NONE;

	if (DestPlugin && CopyDlg[ID_SC_TARGETEDIT].strData == strInitDestDir)
	{
		ToPlugin=1;
		return;
	}

	if (DestPlugin==2)
	{
		if (PluginDestPath)
			*PluginDestPath = strCopyDlgValue;

		return;
	}

	if ((Global->Opt->Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && SrcPanel->IsDizDisplayed()) ||
	        Global->Opt->Diz.UpdateMode==DIZ_UPDATE_ALWAYS)
	{
		Global->CtrlObject->Cp()->LeftPanel()->ReadDiz();
		Global->CtrlObject->Cp()->RightPanel()->ReadDiz();
	}

	CopyBuffer.reset(CopyBufferSize);
	DestPanel->CloseFile();
	strDestDizPath.clear();
	SrcPanel->SaveSelection();
	// TODO: Posix - bugbug
	ReplaceSlashToBackslash(strCopyDlgValue);
	// нужно ли показывать время копирования?
	// ***********************************************************************
	// **** Здесь все подготовительные операции закончены, можно приступать
	// **** к процессу Copy/Move/Link
	// ***********************************************************************
	int NeedDizUpdate=FALSE;
	int NeedUpdateAPanel=FALSE;
	// ПОКА! принудительно выставим обновление.
	// В последствии этот флаг будет выставляться в ShellCopy::CheckUpdatePanel()
	Flags|=FCOPY_UPDATEPPANEL;
	{
		Flags&=~FCOPY_MOVE;
		if (!m_DestList.empty())
		{
			string strNameTmp;

			m_NumberOfTargets=m_DestList.size();

			if (m_NumberOfTargets > 1)
				Move = false;

			FOR_CONST_RANGE(m_DestList, i)
			{
				bool LastIteration = false;
				{
					if (i + 1 == m_DestList.end())
					{
						LastIteration = true;
					}
				}

				strNameTmp = *i;

				if ((strNameTmp.size() == 2) && is_alpha(strNameTmp[0]) && (strNameTmp[1] == L':'))
					PrepareDiskPath(strNameTmp);

				if (CheckNulOrCon(strNameTmp))
				{
					Flags|=FCOPY_COPYTONUL;
					strNameTmp = L"\\\\?\\nul\\"sv;
				}
				else
					Flags&=~FCOPY_COPYTONUL;

				if (Flags&FCOPY_COPYTONUL)
				{
					Flags&=~FCOPY_MOVE;
					Move = false;
				}
				bool ShowCopyTime=(Global->Opt->CMOpt.CopyTimeRule&((Flags&FCOPY_COPYTONUL)?COPY_RULE_NUL:COPY_RULE_FILES))!=0;

				if (SelCount==1 || (Flags&FCOPY_COPYTONUL))
					AddSlash=false; //???


				if (LastIteration) // нужно учесть моменты связанные с операцией Move.
				{
					Flags |= FCOPY_COPYLASTTIME | (Move? FCOPY_MOVE : FCOPY_NONE); // только для последней операции
				}

				// Если выделенных элементов больше 1 и среди них есть каталог, то всегда
				// делаем так, чтобы на конце был '\\'
				// делаем так не всегда, а только когда NameTmp не является маской.
				if (AddSlash && strNameTmp.find_first_of(L"*?"sv) == string::npos)
					AddEndSlash(strNameTmp);

				if (SelCount==1 && !FolderPresent)
				{
					ShowTotalCopySize = false;
				}

				if (Move) // при перемещении "тотал" так же скидывается для "того же диска"
				{
					if (GetPathRoot(strSrcDir) == GetPathRoot(strNameTmp))
						ShowTotalCopySize = false;
					if (SelCount==1 && FolderPresent && CheckUpdateAnotherPanel(SrcPanel, SingleSelName))
					{
						NeedUpdateAPanel=TRUE;
					}
				}

				if (!CP)
					CP = std::make_unique<copy_progress>(Move, ShowTotalCopySize, ShowCopyTime);

				CP->m_Bytes.CurrCopied = 0;

				if (SelCount == 1 && !FolderPresent)
				{
					CP->m_Files.Total = 1;
					CP->m_Bytes.Total = SingleSelectedFileSize;
				}

				// Обнулим инфу про дизы
				strDestDizPath.clear();
				Flags&=~FCOPY_DIZREAD;
				// сохраним выделение
				SrcPanel->SaveSelection();
				strDestFSName.clear();
				int OldCopySymlinkContents=Flags&FCOPY_COPYSYMLINKCONTENTS;
				// собственно - один проход копирования
				// Mantis#45: Необходимо привести копирование ссылок на папки с NTFS на FAT к более логичному виду
				{
					DWORD FilesystemFlags;
					if (os::fs::GetVolumeInformation(GetPathRoot(strNameTmp), nullptr, nullptr, nullptr, &FilesystemFlags, nullptr) && !(FilesystemFlags&FILE_SUPPORTS_REPARSE_POINTS))
						Flags|=FCOPY_COPYSYMLINKCONTENTS;
				}

				const auto I = CopyFileTree(strNameTmp);

				if (OldCopySymlinkContents)
					Flags|=FCOPY_COPYSYMLINKCONTENTS;
				else
					Flags&=~FCOPY_COPYSYMLINKCONTENTS;

				if (I == COPY_CANCEL)
				{
					NeedDizUpdate=TRUE;
					break;
				}

				// если "есть порох в пороховницах" - восстановим выделение
				if (!LastIteration)
					SrcPanel->RestoreSelection();

				// Позаботимся о дизах.
				if (!(Flags&FCOPY_COPYTONUL) && !strDestDizPath.empty())
				{
					// Скидываем только во время последней Op.
					if (LastIteration && Move && !os::fs::file_status(DestDiz.GetDizName()).check(FILE_ATTRIBUTE_READONLY))
						SrcPanel->FlushDiz();

					DestDiz.Flush(strDestDizPath);
				}
			}
		}
		_LOGCOPYR(else SysLog(L"Error: DestList.Set(CopyDlgValue) return FALSE"));
	}
	// ***********************************************************************
	// *** заключительная стадия процесса
	// *** восстанавливаем/дизим/редравим
	// ***********************************************************************

	if (NeedDizUpdate) // при мультикопировании может быть обрыв, но нам все
	{                 // равно нужно апдейтить дизы!
		if (!(Flags&FCOPY_COPYTONUL) && !strDestDizPath.empty())
		{
			if (Move && !os::fs::file_status(DestDiz.GetDizName()).check(FILE_ATTRIBUTE_READONLY))
				SrcPanel->FlushDiz();

			DestDiz.Flush(strDestDizPath);
		}
	}

	if (Global->Opt->CMOpt.PreserveTimestamps)
	{
		for (const auto& CreatedFolder : m_CreatedFolders)
		{
			if (const auto File = os::fs::file(CreatedFolder.FullName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT))
			{
				set_file_time(File, CreatedFolder);
			}
			// TODO: else log
		}
	}


#if 1
	SrcPanel->Update(UPDATE_KEEP_SELECTION);

	if (SelCount==1 && !strRenamedName.empty())
		SrcPanel->GoToFile(strRenamedName);

#if 1

	if (NeedUpdateAPanel && SingleSelAttributes != INVALID_FILE_ATTRIBUTES && (SingleSelAttributes & FILE_ATTRIBUTE_DIRECTORY) && DestPanelMode != panel_mode::PLUGIN_PANEL)
	{
		DestPanel->SetCurDir(SrcPanel->GetCurDir(), false);
	}

#else

	if (SingleSelAttributes != INVALID_FILE_ATTRIBUTES && (SingleSelAttributes & FILE_ATTRIBUTE_DIRECTORY) && DestPanelMode != panel_mode::PLUGIN_PANEL)
	{
		// если SrcDir содержится в DestDir...
		string strTmpDestDir;
		string strTmpSrcDir;
		DestPanel->GetCurDir(strTmpDestDir);
		SrcPanel->GetCurDir(strTmpSrcDir);

		if (CheckUpdateAnotherPanel(SrcPanel,strTmpSrcDir))
			DestPanel->SetCurDir(strTmpDestDir,false);
	}

#endif

	// проверим "нужность" апдейта пассивной панели
	if (Flags&FCOPY_UPDATEPPANEL)
	{
		DestPanel->SortFileList(true);
		DestPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	}

	if (SrcPanelMode == panel_mode::PLUGIN_PANEL)
		SrcPanel->SetPluginModified();

	Global->CtrlObject->Cp()->Redraw();
#else
	SrcPanel->Update(UPDATE_KEEP_SELECTION);

	if (SelCount==1 && strRenamedName.empty())
		SrcPanel->GoToFile(strRenamedName);

	SrcPanel->Redraw();
	DestPanel->SortFileList(true);
	DestPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	DestPanel->Redraw();
#endif
}

ShellCopy::~ShellCopy()
{
	_tran(SysLog(L"[%p] ShellCopy::~ShellCopy(), CopyBufer=%p",this,CopyBuffer));
}

COPY_CODES ShellCopy::CopyFileTree(const string& Dest)
{
	//SaveScreen SaveScr;
	DWORD DestAttr = INVALID_FILE_ATTRIBUTES;
	size_t DestMountLen = 0;

	if (Dest.empty() || IsCurrentDirectory(Dest))
		return COPY_FAILURE; //????

	SetCursorType(false, 0);

	DWORD Flags0 = Flags;

	bool first = true;
	bool UseWildCards = Dest.find_first_of(L"*?"sv) != string::npos;
	bool copy_to_null = (0 != (Flags & FCOPY_COPYTONUL));
	bool move_rename = (0 != (Flags & FCOPY_MOVE));
	bool SameDisk = false;

	if (!CP->m_Bytes.Total)
	{
		//  ! Не сканируем каталоги при создании линков
		if (CP->IsTotalVisible() && !(Flags&FCOPY_LINK) && !CalcTotalSize())
			return COPY_FAILURE;
	}
	else
	{
		CP->m_Bytes.CurrCopied = 0;
	}

	// Основной цикл копирования одной порции.
	//
	for (const auto& i: SrcPanel->enum_selected())
	{
		string strDest(Dest);
		Flags = (Flags0 & ~FCOPY_DIZREAD) | (Flags & FCOPY_DIZREAD);

		bool src_abspath = IsAbsolutePath(i.FileName);

		bool dst_abspath = copy_to_null || IsAbsolutePath(strDest);
		if (!dst_abspath && ((strDest.size() > 2 && strDest[1] == L':') || IsSlash(strDest[0])))
		{
			strDest = ConvertNameToFull(strDest);
			dst_abspath = true;
		}

		SelectedFolderNameLength = (i.Attributes & FILE_ATTRIBUTE_DIRECTORY)? static_cast<int>(i.FileName.size()) : 0;
		if (UseWildCards)
			strDest = ConvertWildcards(i.FileName, strDest, SelectedFolderNameLength);

		bool simple_rename = false;
		if (move_rename && first && SrcPanel->GetSelCount() == 1 && !src_abspath)
			simple_rename = PointToName(strDest).size() == strDest.size();

		if (!simple_rename && !dst_abspath && !IsAbsolutePath(strDest))
		{
			string tpath;
			if (!src_abspath)
			{
				tpath = SrcPanel->GetCurDir();
				AddEndSlash(tpath);
			}
			else
			{
				const auto SlashPos = FindLastSlash(i.FileName);
				if (SlashPos)
				{
					tpath.assign(i.FileName, 0, SlashPos + 1);
				}
			}
			strDest.insert(0, tpath);
		}

		bool check_samedisk = false, dest_changed = false;
		if (first || strSrcDriveRoot.empty() || (src_abspath && !starts_with_icase(i.FileName, strSrcDriveRoot)))
		{
			strSrcDriveRoot = GetPathRoot(src_abspath? i.FileName : SrcPanel->GetCurDir());
			SrcDriveType = FAR_GetDriveType(strSrcDriveRoot);
			check_samedisk = true;
		}
		if (!copy_to_null && (first || strDestDriveRoot.empty() || !starts_with_icase(strDest, strDestDriveRoot)))
		{
			strDestDriveRoot = GetPathRoot(strDest);
			DestDriveType = FAR_GetDriveType(strDestDriveRoot);
			DestMountLen = GetMountPointLen(strDest, strDestDriveRoot);
			check_samedisk = dest_changed = true;
		}
		if (move_rename && !copy_to_null && check_samedisk)
		{
			SameDisk = GetPathRoot(src_abspath? i.FileName : SrcPanel->GetCurDir()) == GetPathRoot(strDest);
		}

		if (first && !copy_to_null && (dst_abspath || !src_abspath) && !UseWildCards
		 && SrcPanel->GetSelCount() > 1
		 && !IsSlash(strDest.back())
		 && !os::fs::exists(strDest))
		{
			switch (Message(MSG_WARNING,
				msg(lng::MWarning),
				{
					strDest,
					msg(lng::MCopyDirectoryOrFile),
				},
				{ lng::MCopyDirectoryOrFileDirectory, lng::MCopyDirectoryOrFileFile, lng::MCancel }))
			{
			case Message::first_button:
				AddEndSlash(strDest);
				[[fallthrough]];
			case Message::second_button:
				break;
			case -2:
			case -1:
			case Message::third_button:
				return COPY_CANCEL;
			}
		}

		if (dest_changed) // check destination drive ready
		{
			DestAttr = os::fs::get_file_attributes(strDest);
			if (INVALID_FILE_ATTRIBUTES == DestAttr && !SameDisk)
			{
				const auto Exists_1 = os::fs::exists(strDestDriveRoot);
				auto Exists_2 = Exists_1;
				while ( !Exists_2 && !SkipErrors)
				{
					const auto ErrorState = error_state::fetch();

					const auto Result = OperationFailed(ErrorState, strDestDriveRoot, lng::MError, {});
					if (Result == operation::retry)
					{
						continue;
					}
					else if (Result == operation::skip)
					{
						return COPY_SKIPPED;
					}
					else if (Result == operation::skip_all)
					{
						SkipErrors = true;
						return COPY_SKIPPED;
					}
					else if (Result == operation::cancel)
					{
						return COPY_CANCEL;
					}

					Exists_2 = os::fs::exists(strDestDriveRoot);
				}
				if (!Exists_1 && Exists_2)
					DestAttr = os::fs::get_file_attributes(strDest);
			}
		}

		const auto pos = FindLastSlash(strDest);
		if (!copy_to_null && pos != string::npos && (!DestMountLen || pos > DestMountLen))
		{
			const auto strNewPath = strDest.substr(0, pos);
			const os::fs::file_status NewPathStatus(strNewPath);
			if (!os::fs::exists(NewPathStatus))
			{
				if (os::fs::create_directory(strNewPath))
					TreeList::AddTreeName(strNewPath);
				else
					CreatePath(strNewPath);

				DestAttr = os::fs::get_file_attributes(strDest);
			}
			else if (os::fs::is_file(NewPathStatus))
			{
				Message(MSG_WARNING,
					msg(lng::MError),
					{
						msg(lng::MCopyCannotCreateFolder),
						strNewPath
					},
					{ lng::MOk });
				return COPY_FAILURE;
			}
		}

		// копируем полный контент, независимо от опции (но не для случая переименования линка по сети)
		if ((DestDriveType == DRIVE_REMOTE || SrcDriveType == DRIVE_REMOTE) && !equal_icase(strSrcDriveRoot, strDestDriveRoot))
			Flags |= FCOPY_COPYSYMLINKCONTENTS;

		first = false;
		string strDestPath = strDest;

		os::fs::find_data SrcData;
		int CopyCode = COPY_SUCCESS;

		auto KeepPathPos = static_cast<int>(i.FileName.size() - PointToName(i.FileName).size());

		if (RPT==RP_JUNCTION || RPT==RP_SYMLINK || RPT==RP_SYMLINKFILE || RPT==RP_SYMLINKDIR)
		{
			switch (MkSymLink(i.FileName, strDest, RPT))
			{
				case 2:
					break;
				case 1:

					// Отметим (Ins) несколько каталогов, ALT-F6 Enter - выделение с папок не снялось.
					if ((!(Flags&FCOPY_CURRENTONLY)) && (Flags&FCOPY_COPYLASTTIME))
						SrcPanel->ClearLastGetSelection();

					continue;
				case 0:
					return COPY_FAILURE;
			}
		}
		else
		{
			// проверка на вшивость ;-)
			if (!os::fs::get_find_data(i.FileName, SrcData))
			{
				strDestPath = i.FileName;
				CP->SetNames(i.FileName,strDestPath);

				if (Message(MSG_WARNING,
					msg(lng::MError),
					{
						msg(lng::MCopyCannotFind),
						i.FileName
					},
					{ lng::MSkip, lng::MCancel }) == Message::second_button)
				{
					return COPY_FAILURE;
				}

				continue;
			}
		}

		if (move_rename)
		{
			if ((m_UseFilter || !SameDisk) || (os::fs::is_directory_symbolic_link(SrcData) && (Flags&FCOPY_COPYSYMLINKCONTENTS)))
			{
				CopyCode=COPY_FAILURE;
			}
			else
			{
				do
				{
					CopyCode=ShellCopyOneFile(i.FileName,SrcData,strDestPath,KeepPathPos,1);
				}
				while (CopyCode==COPY_RETRY);

				if (CopyCode==COPY_SUCCESS_MOVE)
				{
					if (!strDestDizPath.empty())
					{
						if (!strRenamedName.empty())
						{
							DestDiz.Erase(i.FileName, i.AlternateFileName());
							SrcPanel->CopyDiz(i.FileName, i.AlternateFileName(), strRenamedName, strRenamedName, &DestDiz);
						}
						else
						{
							if (strCopiedName.empty())
								strCopiedName = i.FileName;

							SrcPanel->CopyDiz(i.FileName, i.AlternateFileName(), strCopiedName, strCopiedName, &DestDiz);
							SrcPanel->DeleteDiz(i.FileName, i.AlternateFileName());
						}
					}

					continue;
				}

				if (CopyCode==COPY_CANCEL)
					return COPY_CANCEL;

				if (CopyCode==COPY_SKIPPED)
				{
					CP->UpdateAllBytesInfo(SrcData.FileSize);
					continue;
				}
			}
		}

		if (!(Flags&FCOPY_MOVE) || CopyCode==COPY_FAILURE)
		{
			string strCopyDest=strDest;

			do
			{
				CopyCode = ShellCopyOneFile(i.FileName, SrcData, strCopyDest, KeepPathPos, 0);
			}
			while (CopyCode==COPY_RETRY);

			if (CopyCode==COPY_CANCEL)
				return COPY_CANCEL;

			if (CopyCode!=COPY_SUCCESS)
			{
				if (CopyCode == COPY_SKIPPED)
				{
					CP->UpdateAllBytesInfo(SrcData.FileSize);
				}
				continue;
			}
		}

		if (CopyCode==COPY_SUCCESS && !(Flags&FCOPY_COPYTONUL) && !strDestDizPath.empty())
		{
			if (strCopiedName.empty())
				strCopiedName = i.FileName;

			SrcPanel->CopyDiz(i.FileName, i.AlternateFileName(), strCopiedName, strCopiedName, &DestDiz);
		}

		// Mantis#44 - Потеря данных при копировании ссылок на папки
		// если каталог (или нужно копировать симлинк) - придется рекурсивно спускаться...
		if (RPT != RP_SYMLINKFILE && (SrcData.Attributes & FILE_ATTRIBUTE_DIRECTORY) && ((Flags & FCOPY_COPYSYMLINKCONTENTS) || !os::fs::is_directory_symbolic_link(SrcData)))
		{
			int SubCopyCode;
			string strFullName;
			ScanTree ScTree(true, true, Flags & FCOPY_COPYSYMLINKCONTENTS);
			auto strSubName = i.FileName + L'\\';

			if (DestAttr==INVALID_FILE_ATTRIBUTES)
				KeepPathPos = static_cast<int>(strSubName.size());

			int NeedRename = !(os::fs::is_directory_symbolic_link(SrcData) && (Flags&FCOPY_COPYSYMLINKCONTENTS) && (Flags&FCOPY_MOVE));
			ScTree.SetFindPath(strSubName, L"*"sv);

			while (ScTree.GetNextName(SrcData,strFullName))
			{
				if (m_UseFilter && (SrcData.Attributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					// Просто пропустить каталог недостаточно - если каталог помечен в
					// фильтре как некопируемый, то следует пропускать и его и всё его
					// содержимое.
					if (!m_Filter->FileInFilter(SrcData, nullptr, &strFullName))
					{
						ScTree.SkipDir();
						continue;
					}
				}
				{
					int AttemptToMove=FALSE;

					if ((Flags&FCOPY_MOVE) && SameDisk && !(SrcData.Attributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						AttemptToMove=TRUE;
						int Ret;
						string strCopyDest=strDest;

						do
						{
							Ret=ShellCopyOneFile(strFullName,SrcData,strCopyDest,KeepPathPos,NeedRename);
						}
						while (Ret==COPY_RETRY);

						switch (Ret) // 1
						{
							case COPY_CANCEL:
								return COPY_CANCEL;
							case COPY_SKIPPED:
							{
								CP->UpdateAllBytesInfo(SrcData.FileSize);
								continue;
							}
							case COPY_SUCCESS_MOVE:
							{
								continue;
							}
							case COPY_SUCCESS:

								if (!NeedRename) // вариант при перемещении содержимого симлинка с опцией "копировать содержимое сим..."
								{
									CP->UpdateAllBytesInfo(SrcData.FileSize);
									continue;     // ...  т.к. мы ЭТО не мувили, а скопировали, то все, на этом закончим бадаться с этим файлов
								}
						}
					}

					int SaveOvrMode=OvrMode;

					if (AttemptToMove)
						OvrMode=1;

					string strCopyDest=strDest;

					do
					{
						SubCopyCode=ShellCopyOneFile(strFullName,SrcData,strCopyDest,KeepPathPos,0);
					}
					while (SubCopyCode==COPY_RETRY);

					if (AttemptToMove)
						OvrMode=SaveOvrMode;
				}

				if (SubCopyCode==COPY_CANCEL)
					return COPY_CANCEL;

				if (SubCopyCode==COPY_SKIPPED)
				{
					CP->UpdateAllBytesInfo(SrcData.FileSize);
				}

				// здесь нужны проверка на InsideReparsePoint, иначе
				// при мувинге будет удаление файла, что крайне неправильно!
				if (SubCopyCode == COPY_SUCCESS && Flags & FCOPY_MOVE && !ScTree.InsideReparsePoint())
				{
					if (SrcData.Attributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						if (ScTree.IsDirSearchDone() || (os::fs::is_directory_symbolic_link(SrcData) && !(Flags & FCOPY_COPYSYMLINKCONTENTS)))
						{
							if (DeleteAfterMove(strFullName, SrcData.Attributes) == COPY_CANCEL)
								return COPY_CANCEL;

							TreeList::DelTreeName(strFullName);
						}
					}
					else if (DeleteAfterMove(strFullName, SrcData.Attributes) == COPY_CANCEL)
					{
						return COPY_CANCEL;
					}
				}
			}
		}

		if ((Flags & FCOPY_MOVE) && CopyCode == COPY_SUCCESS)
		{
			const auto DeleteCode = DeleteAfterMove(i.FileName, i.Attributes);

			if (DeleteCode == COPY_CANCEL)
				return COPY_CANCEL;

			if (DeleteCode == COPY_SUCCESS)
			{
				if ((SrcData.Attributes & FILE_ATTRIBUTE_DIRECTORY))
					TreeList::DelTreeName(i.FileName);

				if (!strDestDizPath.empty())
					SrcPanel->DeleteDiz(i.FileName, i.AlternateFileName());
			}
		}

		if ((!(Flags&FCOPY_CURRENTONLY)) && (Flags&FCOPY_COPYLASTTIME))
		{
			SrcPanel->ClearLastGetSelection();
		}
	}

	return COPY_SUCCESS; //COPY_SUCCESS_MOVE???
}



// абсолютно невменяемая функция. функция таких размеров вменяема быть не может. переписать ASAP

COPY_CODES ShellCopy::ShellCopyOneFile(
    const string& Src,
    const os::fs::find_data &SrcData,
    string &strDest,
    int KeepPathPos,
    int Rename
)
{
	CP->m_Bytes.CurrCopied = 0; // Сбросить текущий прогресс

	if (CP->IsCancelled())
	{
		return COPY_CANCEL;
	}

	if (m_UseFilter)
	{
		if (!m_Filter->FileInFilter(SrcData, nullptr, &Src))
			return COPY_NOFILTER;
	}

	string strDestPath = strDest;

	DWORD DestAttr=INVALID_FILE_ATTRIBUTES;

	os::fs::find_data DestData;
	if (!(Flags&FCOPY_COPYTONUL))
	{
		if (os::fs::get_find_data(strDestPath,DestData))
			DestAttr=DestData.Attributes;
	}

	int SameName=0, Append=0;

	if (!(Flags&FCOPY_COPYTONUL) && DestAttr!=INVALID_FILE_ATTRIBUTES && (DestAttr & FILE_ATTRIBUTE_DIRECTORY))
	{
		if(SrcData.Attributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			int CmpCode=CmpFullNames(Src,strDestPath);

			if(CmpCode && SrcData.Attributes&FILE_ATTRIBUTE_REPARSE_POINT && RPT==RP_EXACTCOPY && !(Flags&FCOPY_COPYSYMLINKCONTENTS))
			{
				CmpCode = 0;
			}

			if (CmpCode==1) // TODO: error check
			{
				SameName=1;

				if (Rename)
				{
					CmpCode = PointToName(Src) == PointToName(strDestPath);
				}

				if (CmpCode==1)
				{
					Message(MSG_WARNING,
						msg(lng::MError),
						{
							msg(lng::MCannotCopyFolderToItself1),
							Src,
							msg(lng::MCannotCopyFolderToItself2)
						},
						{ lng::MOk },
						L"ErrCopyItSelf"sv);
					return COPY_CANCEL;
				}
			}
		}

		if (!SameName)
		{
			const auto Length = strDestPath.size();

			if (!IsSlash(strDestPath[Length-1]) && strDestPath[Length-1]!=L':')
				strDestPath += L'\\';

			auto Path = string_view(Src).substr(KeepPathPos);

			if (Path.size() > 1 && !KeepPathPos && Path[1] == L':')
				Path.remove_prefix(2);

			if (IsSlash(Path.front()))
				Path.remove_prefix(1);

			append(strDestPath, Path);

			if (!os::fs::get_find_data(strDestPath, DestData))
				DestAttr=INVALID_FILE_ATTRIBUTES;
			else
				DestAttr=DestData.Attributes;
		}
	}

	if (!(Flags&FCOPY_COPYTONUL) && !equal_icase(strDestPath, L"prn"sv))
		SetDestDizPath(strDestPath);

	CP->SetNames(Src, strDestPath);
	CP->SetProgressValue(0,0);

	if (!(Flags&FCOPY_COPYTONUL))
	{
		if constexpr ((false))
		{
			if (!CheckStreams(Src, strDestPath))
				return COPY_CANCEL;
		}

		bool dir = (SrcData.Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		bool rpt = (SrcData.Attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
		bool cpc = (Flags & FCOPY_COPYSYMLINKCONTENTS) != 0;
		if (!dir && rpt && RPT==RP_EXACTCOPY && !cpc)
		{
			bool spf = (SrcData.Attributes & FILE_ATTRIBUTE_SPARSE_FILE) != 0;
			if (spf)
				cpc = true; // ???
			else
			{
				cpc = !(SrcData.ReparseTag == IO_REPARSE_TAG_SYMLINK || SrcData.ReparseTag == IO_REPARSE_TAG_MOUNT_POINT);
			}
			if (cpc)
				Flags |= FCOPY_COPYSYMLINKCONTENTS;
		}

		if (dir || (rpt && RPT==RP_EXACTCOPY && !cpc))
		{
			if (!Rename)
				strCopiedName = PointToName(strDestPath);

			if (DestAttr!=INVALID_FILE_ATTRIBUTES)
			{
				if ((DestAttr & FILE_ATTRIBUTE_DIRECTORY) && !SameName)
				{
					DWORD SetAttr=SrcData.Attributes;

					if (IsDriveTypeCDROM(SrcDriveType) && (SetAttr & FILE_ATTRIBUTE_READONLY))
						SetAttr&=~FILE_ATTRIBUTE_READONLY;

					if (SetAttr!=DestAttr)
						ShellSetAttr(strDestPath,SetAttr);

					return ConvertNameToFull(Src) == strDestPath? COPY_SKIPPED : COPY_SUCCESS;
				}
			}

			if (Rename)
			{
				const auto strSrcFullName = ConvertNameToFull(Src);

				// Пытаемся переименовать, пока не отменят
				for (;;)
				{
					if (os::fs::move_file(Src, strDestPath))
						break;

					const auto ErrorState = error_state::fetch();
					switch (OperationFailed(ErrorState, Src, lng::MError, msg(lng::MCopyCannotRenameFolder), true, false))
					{
					case operation::retry:
						continue;

					case operation::skip:
					{
						os::security::descriptor tmpsd;
						if (m_CopySecurity == security::copy && !GetSecurity(Src, tmpsd))
							return COPY_CANCEL;

						SECURITY_ATTRIBUTES TmpSecAttr{ sizeof(TmpSecAttr), tmpsd? tmpsd.data() : nullptr };

						for (;;)
						{
							if (os::fs::create_directory(strDestPath, tmpsd? &TmpSecAttr : nullptr))
								break;

							const auto CreateDirectoryErrorState = error_state::fetch();
							switch (OperationFailed(CreateDirectoryErrorState, strDestPath, lng::MError, msg(lng::MCopyCannotCreateFolder), true, false))
							{
							case operation::retry:
								continue;

							case operation::skip:
								return COPY_SKIPPED;

							default:
								return COPY_CANCEL;
							}
						}

						const auto NamePart = PointToName(strDestPath);
						if (NamePart.size() == strDestPath.size())
							strRenamedName = strDestPath;
						else
							strCopiedName = NamePart;

						TreeList::AddTreeName(strDestPath);
						return COPY_SUCCESS;
					}

					default:
						return COPY_CANCEL;
					}
				}


				if (m_CopySecurity == security::inherit && !ResetSecurityRecursively(strDestPath))
					return COPY_CANCEL;

				const auto NamePart = PointToName(strDestPath);
				if (NamePart.size() == strDestPath.size())
					strRenamedName = strDestPath;
				else
					strCopiedName = NamePart;

				TreeList::RenTreeName(strSrcFullName, ConvertNameToFull(strDest));
				return SameName? COPY_SKIPPED : COPY_SUCCESS_MOVE;
			}

			os::security::descriptor sd;

			if (m_CopySecurity == security::copy && !GetSecurity(Src, sd))
				return COPY_CANCEL;

			SECURITY_ATTRIBUTES SecAttr = { sizeof(SecAttr), sd? sd.data() : nullptr };
			if (RPT!=RP_SYMLINKFILE && SrcData.Attributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				while (!os::fs::create_directory(
					// CreateDirectoryEx preserves reparse points,
					// so we shouldn't use template when copying with content
					os::fs::is_directory_symbolic_link(SrcData) && (Flags & FCOPY_COPYSYMLINKCONTENTS)? L""s : Src,
					strDestPath,
					sd? &SecAttr : nullptr))
				{
					const auto ErrorState = error_state::fetch();
					const int MsgCode = Message(MSG_WARNING, ErrorState,
						msg(lng::MError),
						{
							msg(lng::MCopyCannotCreateFolder),
							strDestPath
						},
						{ lng::MCopyRetry, lng::MCopySkip, lng::MCopyCancel });

					if (MsgCode == Message::first_button) // Retry
					{
						continue;
					}
					else if (MsgCode == Message::second_button) // Skip
					{
						return COPY_SKIPPED;
					}
					else // Cancel
					{
						return COPY_CANCEL;
					}
				}

				if (Global->Opt->CMOpt.PreserveTimestamps)
					m_CreatedFolders.emplace_back(strDestPath, SrcData);

				DWORD SetAttr=SrcData.Attributes;

				if (IsDriveTypeCDROM(SrcDriveType) && (SetAttr & FILE_ATTRIBUTE_READONLY))
					SetAttr&=~FILE_ATTRIBUTE_READONLY;

				if (!ShellSetAttr(strDestPath, SetAttr))
				{
					// BUGBUG check result
					(void)os::fs::remove_directory(strDestPath);
					return COPY_CANCEL;
				}
			}

			// [ ] Copy contents of symbolic links
			// For file symbolic links only!!!
			// Directory symbolic links and junction points are handled by CreateDirectoryEx.
			if (!dir && rpt && !cpc && RPT==RP_EXACTCOPY)
			{
				switch (MkSymLink(Src, strDestPath, RPT))
				{
					case 2:
						return COPY_CANCEL;
					case 1:
						break;
					case 0:
						return COPY_FAILURE;
				}
			}

			TreeList::AddTreeName(strDestPath);
			return COPY_SUCCESS;
		}

		if (DestAttr!=INVALID_FILE_ATTRIBUTES && !(DestAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			if (SrcData.FileSize==DestData.FileSize)
			{
				int CmpCode=CmpFullNames(Src,strDestPath);

				if(CmpCode && SrcData.Attributes&FILE_ATTRIBUTE_REPARSE_POINT && RPT==RP_EXACTCOPY && !(Flags&FCOPY_COPYSYMLINKCONTENTS))
				{
					CmpCode = 0;
				}

				if (CmpCode==1) // TODO: error check
				{
					SameName=1;

					if (Rename)
					{
						CmpCode = PointToName(Src) == PointToName(strDestPath);
					}

					if (CmpCode==1 && !Rename)
					{
						Message(MSG_WARNING,
							msg(lng::MError),
							{
								msg(lng::MCannotCopyFileToItself1),
								QuoteOuterSpace(Src),
								msg(lng::MCannotCopyFileToItself2)
							},
							{ lng::MOk });
						return COPY_CANCEL;
					}
				}
			}

			int RetCode=COPY_CANCEL;
			string strNewName;

			if (!AskOverwrite(SrcData,Src,strDestPath,DestAttr,SameName,Rename,((Flags&FCOPY_LINK)?0:1),Append,strNewName,RetCode))
			{
				return static_cast<COPY_CODES>(RetCode);
			}

			if (RetCode==COPY_RETRY)
			{
				strDest=strNewName;

				if (CutToSlash(strNewName) && !os::fs::exists(strNewName))
				{
					CreatePath(strNewName);
				}

				return COPY_RETRY;
			}
		}
	}
	else
	{
		if (SrcData.Attributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			return COPY_SUCCESS;
		}
	}

	const auto NWFS_Attr = Global->Opt->Nowell.MoveRO && strDestFSName == L"NWFS"sv;

	{
		for (;;)
		{
			error_state_ex ErrorState;

			if (!(Flags&FCOPY_COPYTONUL) && Rename)
			{
				int AskDelete;

				if (strDestFSName == L"NWFS"sv && !Append &&
				        DestAttr!=INVALID_FILE_ATTRIBUTES && !SameName)
				{
					os::fs::delete_file(strDestPath); //BUGBUG
				}

				bool FileMoved = false;

				if (!Append)
				{
					const auto strSrcFullName = ConvertNameToFull(Src);

					if (NWFS_Attr)
						os::fs::set_file_attributes(strSrcFullName,SrcData.Attributes&(~FILE_ATTRIBUTE_READONLY));

					if (strDestFSName == L"NWFS"sv)
						FileMoved = os::fs::move_file(strSrcFullName, strDestPath);
					else
						FileMoved = os::fs::move_file(strSrcFullName, strDestPath, SameName ? MOVEFILE_COPY_ALLOWED : MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);

					if (!FileMoved)
					{
						ErrorState = error_state::fetch();

						if (NWFS_Attr)
							os::fs::set_file_attributes(strSrcFullName,SrcData.Attributes);

						if (ErrorState.Win32Error == ERROR_NOT_SAME_DEVICE)
							return COPY_FAILURE;
					}
					else
					{
						if (m_CopySecurity == security::inherit && !ResetSecurity(strDestPath))
							return COPY_CANCEL;
					}

					if (NWFS_Attr)
						os::fs::set_file_attributes(strDestPath,SrcData.Attributes);

					if (FileMoved)
					{
						CP->m_Bytes.CurrCopied = SrcData.FileSize;
						CP->UpdateAllBytesInfo(SrcData.FileSize);
					}

					AskDelete=0;
				}
				else
				{
					int CopyCode;
					do
					{
						DWORD Attr=INVALID_FILE_ATTRIBUTES;
						CopyCode = ShellCopyFile(Src, SrcData, strDestPath, Attr, Append, ErrorState);
					}
					while (CopyCode==COPY_RETRY);

					switch (CopyCode)
					{
						case COPY_SUCCESS:
							FileMoved = true;
							break;
						case COPY_FAILURE:
							FileMoved = false;
							break;
						case COPY_CANCEL:
							return COPY_CANCEL;
						case COPY_SKIPPED:
							return COPY_SKIPPED;
					}

					AskDelete=1;
				}

				if (FileMoved)
				{
					if (DestAttr==INVALID_FILE_ATTRIBUTES || !(DestAttr & FILE_ATTRIBUTE_DIRECTORY))
					{
						const auto NamePart = PointToName(strDestPath);
						if (NamePart.size() == strDestPath.size())
							strRenamedName = strDestPath;
						else
							strCopiedName = NamePart;
					}

					if (IsDriveTypeCDROM(SrcDriveType) && (SrcData.Attributes & FILE_ATTRIBUTE_READONLY))
						ShellSetAttr(strDestPath,SrcData.Attributes & (~FILE_ATTRIBUTE_READONLY));

					++CP->m_Files.Copied;

					if (AskDelete && DeleteAfterMove(Src,SrcData.Attributes)==COPY_CANCEL)
						return COPY_CANCEL;

					return COPY_SUCCESS_MOVE;
				}
			}
			else
			{
				int CopyCode;
				do
				{
					CopyCode = ShellCopyFile(Src, SrcData, strDestPath, DestAttr, Append, ErrorState);
				}
				while (CopyCode==COPY_RETRY);

				if (CopyCode==COPY_SUCCESS)
				{
					strCopiedName = PointToName(strDestPath);

					if (!(Flags&FCOPY_COPYTONUL))
					{
						if (IsDriveTypeCDROM(SrcDriveType) && (SrcData.Attributes & FILE_ATTRIBUTE_READONLY))
							ShellSetAttr(strDestPath,SrcData.Attributes & ~FILE_ATTRIBUTE_READONLY);

						if (DestAttr!=INVALID_FILE_ATTRIBUTES && equal_icase(strCopiedName, DestData.FileName) && strCopiedName != DestData.FileName)
							// BUGBUG check result
							(void)os::fs::move_file(strDestPath, strDestPath); //???
					}

					++CP->m_Files.Copied;

					if (DestAttr!=INVALID_FILE_ATTRIBUTES && Append)
						os::fs::set_file_attributes(strDestPath,DestAttr);

					return COPY_SUCCESS;
				}
				else if (CopyCode==COPY_CANCEL || CopyCode==COPY_SKIPPED)
				{
					if (DestAttr!=INVALID_FILE_ATTRIBUTES && Append)
						os::fs::set_file_attributes(strDestPath,DestAttr);

					return static_cast<COPY_CODES>(CopyCode);
				}

				if (DestAttr!=INVALID_FILE_ATTRIBUTES && Append)
					os::fs::set_file_attributes(strDestPath,DestAttr);
			}

			const auto MsgMCannot = Flags & FCOPY_LINK? lng::MCannotLink : Flags & FCOPY_MOVE? lng::MCannotMove : lng::MCannotCopy;
			const auto strMsg1 = quote_unconditional(Src);
			const auto strMsg2 = quote_unconditional(strDestPath);
			int MsgCode;
			if (SrcData.Attributes&FILE_ATTRIBUTE_ENCRYPTED)
			{
				if (SkipEncMode != -1)
				{
					MsgCode = SkipEncMode;

					if (SkipEncMode == 1)
						Flags |= FCOPY_DECRYPTED_DESTINATION;
				}
				else
				{
					// Better to set it always, just in case
					//if (GetLastError() == ERROR_ACCESS_DENIED)
					{
						SetLastError(ERROR_ENCRYPTION_FAILED);
						ErrorState = error_state::fetch();
					}

					MsgCode = Message(MSG_WARNING, ErrorState,
						msg(lng::MError),
						{
							msg(MsgMCannot),
							strMsg1,
							msg(lng::MCannotCopyTo),
							strMsg2
						},
						{ lng::MCopyDecrypt, lng::MCopyDecryptAll, lng::MCopySkip, lng::MCopySkipAll, lng::MCopyCancel });
				}
				switch (MsgCode)
				{
				case 1:
					SkipEncMode = 1;
					[[fallthrough]];
				case 0:
					Flags |= FCOPY_DECRYPTED_DESTINATION;
					break;

				case 3:
					SkipEncMode = 3;
					[[fallthrough]];
				case 2:
					return COPY_SKIPPED;

				case -1:
				case -2:
				case 4:
					return COPY_CANCEL;
				}
			}
			else
			{
				if (!SkipErrors)
				{
					if (!ErrorState)
						ErrorState = error_state::fetch();

					MsgCode = Message(MSG_WARNING, ErrorState,
						msg(lng::MError),
						{
							msg(MsgMCannot),
							strMsg1,
							msg(lng::MCannotCopyTo),
							strMsg2
						},
						{ lng::MCopyRetry, lng::MCopySkip, lng::MCopySkipAll, lng::MCopyCancel });
				}
				else
				{
					MsgCode = Message::second_button;
				}

				switch (MsgCode)
				{
				case Message::second_button:
					return COPY_SKIPPED;

				case Message::third_button:
					SkipErrors = true;
					return COPY_SKIPPED;
				case -1:
				case -2:
				case Message::fourth_button:
					return COPY_CANCEL;
				}
			}

			int RetCode = COPY_CANCEL;
			string strNewName;

			if (!AskOverwrite(SrcData,Src,strDestPath,DestAttr,SameName,Rename,((Flags&FCOPY_LINK)?0:1),Append,strNewName,RetCode))
				return static_cast<COPY_CODES>(RetCode);

			if (RetCode==COPY_RETRY)
			{
				strDest=strNewName;

				if (CutToSlash(strNewName) && !os::fs::exists(strNewName))
				{
					CreatePath(strNewName);
				}

				return COPY_RETRY;
			}
		}
	}
}

// TODO: Copy them?
bool ShellCopy::CheckStreams(const string& Src, const string& DestPath)
{
	if (Flags & FCOPY_STREAMSKIPALL)
		return true;

	if (Flags & FCOPY_USESYSTEMCOPY)
		return true;

	const auto StreamsEnumerator = os::fs::enum_streams(Src);

	if (!std::any_of(ALL_CONST_RANGE(StreamsEnumerator), [](WIN32_FIND_STREAM_DATA const& i){ return !starts_with(i.cStreamName, L"::"sv); }))
		return true;

	switch (Message(MSG_WARNING, msg(lng::MWarning),
		{
			msg(lng::MCopyStream1),
			msg(lng::MCopyStream2), // TODO: lng::MCopyStream3
			msg(lng::MCopyStream4),
		},
		{
			lng::MCopySkip, lng::MCopySkipAll, lng::MCopyCancel
		},
		L"WarnCopyStream"sv))
	{
	case Message::first_button:
		return true;

	case Message::second_button:
		Flags |= FCOPY_STREAMSKIPALL;
		return true;

	default:
		return false;
	}
}

int ShellCopy::DeleteAfterMove(const string& Name,DWORD Attr)
{
	const auto FullName = ConvertNameToFull(Name);
	if (Attr & FILE_ATTRIBUTE_READONLY)
	{
		int MsgCode;

		if (!Global->Opt->Confirm.RO)
			ReadOnlyDelMode=1;

		if (ReadOnlyDelMode!=-1)
			MsgCode=ReadOnlyDelMode;
		else
			MsgCode=Message(MSG_WARNING,
				msg(lng::MWarning),
				{
					msg(lng::MCopyFileRO),
					FullName,
					msg(lng::MCopyAskDelete),
				},
				{ lng::MCopyDeleteRO, lng::MCopyDeleteAllRO, lng::MCopySkip, lng::MCopySkipAll, lng::MCopyCancel });

		switch (MsgCode)
		{
			case 1:
				ReadOnlyDelMode=1;
				break;
			case 2:
				return COPY_SKIPPED;
			case 3:
				ReadOnlyDelMode=3;
				return COPY_SKIPPED;
			case -1:
			case -2:
			case 4:
				return COPY_CANCEL;
		}

		os::fs::set_file_attributes(FullName,FILE_ATTRIBUTE_NORMAL);
	}

	while ((Attr&FILE_ATTRIBUTE_DIRECTORY)?!os::fs::remove_directory(FullName):!os::fs::delete_file(FullName))
	{
		operation MsgCode;

		if (SkipDeleteErrors)
		{
			MsgCode = operation::skip;
		}
		else
		{
			const auto ErrorState = error_state::fetch();

			if (m_UseFilter && (Attr & FILE_ATTRIBUTE_DIRECTORY) && ErrorState.Win32Error == ERROR_DIR_NOT_EMPTY)
				MsgCode = operation::skip;
			else
				MsgCode = OperationFailed(ErrorState, FullName, lng::MError, msg(lng::MCannotDeleteFile));
		}

		switch (MsgCode)
		{
		case operation::retry:
			break;

		case operation::skip:
			return COPY_SKIPPED;

		case operation::skip_all:
			SkipDeleteErrors = true;
			return COPY_SKIPPED;

		case operation::cancel:
			return COPY_CANCEL;
		}
	}

	return COPY_SUCCESS;
}



int ShellCopy::ShellCopyFile(const string& SrcName,const os::fs::find_data &SrcData,
                             string &strDestName,DWORD &DestAttr,int Append, error_state_ex& ErrorState)
{
	if ((Flags&FCOPY_LINK))
	{
		if (RPT==RP_HARDLINK)
		{
			os::fs::delete_file(strDestName); //BUGBUG
			return MkHardLink(SrcName,strDestName)? COPY_SUCCESS : COPY_FAILURE;
		}
		else
		{
			return MkSymLink(SrcName, strDestName, RPT)? COPY_SUCCESS : COPY_FAILURE;
		}
	}

	DWORD FilesystemFlags;
	if ((SrcData.Attributes&FILE_ATTRIBUTE_ENCRYPTED) &&
		os::fs::GetVolumeInformation(GetPathRoot(strDestName), nullptr, nullptr, nullptr, &FilesystemFlags, nullptr) &&
		!(FilesystemFlags & FILE_SUPPORTS_ENCRYPTION))
	{
		int MsgCode;

		if (SkipEncMode!=-1)
		{
			MsgCode=SkipEncMode;

			if (SkipEncMode == 1)
				Flags|=FCOPY_DECRYPTED_DESTINATION;
		}
		else
		{
			MsgCode = Message(MSG_WARNING,
				msg(lng::MWarning),
				{
					msg(lng::MCopyEncryptWarn1),
					quote_unconditional(SrcName),
					msg(lng::MCopyEncryptWarn2),
					msg(lng::MCopyEncryptWarn3)
				},
				{ lng::MCopyIgnore, lng::MCopyIgnoreAll, lng::MCopyCancel },
				L"WarnCopyEncrypt"sv);
		}

		switch (MsgCode)
		{
			case  0:
				_LOGCOPYR(SysLog(L"return COPY_NEXT -> %d",__LINE__));
				Flags|=FCOPY_DECRYPTED_DESTINATION;
				break;//return COPY_NEXT;
			case  1:
				SkipEncMode=1;
				Flags|=FCOPY_DECRYPTED_DESTINATION;
				_LOGCOPYR(SysLog(L"return COPY_NEXT -> %d",__LINE__));
				break;//return COPY_NEXT;
			case -1:
			case -2:
			case  2:
				_LOGCOPYR(SysLog(L"return COPY_CANCEL -> %d",__LINE__));
				return COPY_CANCEL;
		}
	}

	if ((Flags & FCOPY_USESYSTEMCOPY) && !Append)
	{
		if (!(SrcData.Attributes&FILE_ATTRIBUTE_ENCRYPTED) || (IsWindowsXPOrGreater() || !(Flags&(FCOPY_DECRYPTED_DESTINATION))))
		{
			if (!Global->Opt->CMOpt.CopyOpened)
			{
				if (!os::fs::file(SrcName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN))
				{
					_LOGCOPYR(SysLog(L"return COPY_FAILURE -> %d if (SrcHandle==INVALID_HANDLE_VALUE)",__LINE__));
					return COPY_FAILURE;
				}
			}

			//_LOGCOPYR(SysLog(L"call ShellSystemCopy('%s','%s',%p)",SrcName,DestName,SrcData));
			return ShellSystemCopy(SrcName,strDestName,SrcData);
		}
	}

	os::security::descriptor sd;
	if (m_CopySecurity == security::copy && !GetSecurity(SrcName, sd))
		return COPY_CANCEL;

	int OpenMode=FILE_SHARE_READ;

	if (Global->Opt->CMOpt.CopyOpened)
		OpenMode|=FILE_SHARE_WRITE;

	os::fs::file_walker SrcFile;
	bool Opened = SrcFile.Open(SrcName, GENERIC_READ, OpenMode, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);

	if (!Opened && Global->Opt->CMOpt.CopyOpened)
	{
		if (GetLastError() == ERROR_SHARING_VIOLATION)
		{
			Opened = SrcFile.Open(SrcName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
		}
	}

	if (!Opened)
	{
		return COPY_FAILURE;
	}

	os::fs::file DestFile;
	unsigned long long AppendPos=0;

	bool CopySparse=false;

	if (!(Flags&FCOPY_COPYTONUL))
	{
		//if (DestAttr!=INVALID_FILE_ATTRIBUTES && !Append) //вот это портит копирование поверх хардлинков
		//api::DeleteFile(DestName);
		SECURITY_ATTRIBUTES SecAttr = { sizeof(SecAttr), sd? sd.data() : nullptr };

		const auto attrs = SrcData.Attributes & ~(Flags & FCOPY_DECRYPTED_DESTINATION? FILE_ATTRIBUTE_ENCRYPTED : 0);
		const auto IsSystemEncrypted = flags::check_all(attrs, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ENCRYPTED);

		Flags &= ~FCOPY_DECRYPTED_DESTINATION;

		if (!DestFile.Open(
			strDestName,
			GENERIC_WRITE,
			FILE_SHARE_READ,
			sd? &SecAttr : nullptr,
			Append? OPEN_EXISTING : CREATE_ALWAYS,
			(attrs & ~(IsSystemEncrypted? FILE_ATTRIBUTE_SYSTEM : 0)) | FILE_FLAG_SEQUENTIAL_SCAN))
		{
			_LOGCOPYR(DWORD LastError=GetLastError();)
			SrcFile.Close();
			_LOGCOPYR(SysLog(L"return COPY_FAILURE -> %d CreateFile=-1, LastError=%d (0x%08X)",__LINE__,LastError,LastError));
			return COPY_FAILURE;
		}

		if (IsSystemEncrypted)
			os::fs::set_file_attributes(strDestName, attrs);

		const auto strDriveRoot = GetPathRoot(strDestName);

		if (SrcData.Attributes&FILE_ATTRIBUTE_SPARSE_FILE)
		{
			DWORD VolFlags;
			if(os::fs::GetVolumeInformation(strDriveRoot, nullptr, nullptr, nullptr, &VolFlags, nullptr) && VolFlags & FILE_SUPPORTS_SPARSE_FILES)
			{
				if (DestFile.IoControl(FSCTL_SET_SPARSE, nullptr, 0, nullptr, 0))
				{
					CopySparse=true;
				}
			}
		}

		if (Append)
		{
			if (!DestFile.SetPointer(0,&AppendPos,FILE_END))
			{
				SrcFile.Close();
				DestFile.SetEnd();
				DestFile.Close();
				return COPY_FAILURE;
			}
		}

		// если места в приёмнике хватает - займём сразу.
		unsigned long long FreeBytes = 0;
		if (os::fs::get_disk_size(strDriveRoot,nullptr,nullptr,&FreeBytes))
		{
			if (FreeBytes>SrcData.FileSize)
			{
				const auto CurPtr = DestFile.GetPointer();
				DestFile.SetPointer(SrcData.FileSize, nullptr, FILE_CURRENT);
				DestFile.SetEnd();
				DestFile.SetPointer(CurPtr, nullptr, FILE_BEGIN);
			}
		}
	}

	CP->SetProgressValue(0,0);

	if(SrcFile.InitWalk(CopyBufferSize))
	{
		bool AbortOp = false;

		do
		{
			if (CP->IsCancelled())
			{
				AbortOp=true;
			}

			CheckAndUpdateConsole();
			CP->SetProgressValue(CP->m_Bytes.CurrCopied, SrcData.FileSize);

			if (AbortOp)
			{
				SrcFile.Close();

				if (!(Flags&FCOPY_COPYTONUL))
				{
					if (Append)
					{
						DestFile.SetPointer(AppendPos,nullptr,FILE_BEGIN);
					}

					DestFile.SetEnd();
					DestFile.Close();

					if (!Append)
					{
						os::fs::set_file_attributes(strDestName,FILE_ATTRIBUTE_NORMAL);
						os::fs::delete_file(strDestName); //BUGBUG
					}
				}

				return COPY_CANCEL;
			}

			size_t BytesRead;
			while (!SrcFile.Read(CopyBuffer.data(), SrcFile.GetChunkSize(), BytesRead))
			{
				ErrorState = error_state::fetch();

				const int MsgCode = Message(MSG_WARNING, ErrorState,
					msg(lng::MError),
					{
						msg(lng::MCopyReadError),
						SrcName
					},
					{ lng::MRetry, lng::MCancel });
				if (!MsgCode)
					continue;

				SrcFile.Close();

				if (!(Flags&FCOPY_COPYTONUL))
				{
					if (Append)
					{
						DestFile.SetPointer(AppendPos,nullptr,FILE_BEGIN);
					}

					DestFile.SetEnd();
					DestFile.Close();

					if (!Append)
					{
						os::fs::set_file_attributes(strDestName,FILE_ATTRIBUTE_NORMAL);
						os::fs::delete_file(strDestName); //BUGBUG
					}
				}

				CP->SetProgressValue(0,0);
				CP->m_Bytes.CurrCopied = 0; // Сбросить текущий прогресс
				return COPY_FAILURE;
			}

			if (!BytesRead)
			{
				break;
			}

			if (!(Flags&FCOPY_COPYTONUL))
			{
				DestFile.SetPointer(SrcFile.GetChunkOffset() + (Append? AppendPos : 0), nullptr, FILE_BEGIN);
				while (!DestFile.Write(CopyBuffer.data(), BytesRead))
				{
					ErrorState = error_state::fetch();

					int Split=FALSE,SplitCancelled=FALSE,SplitSkipped=FALSE;

					if ((ErrorState.Win32Error == ERROR_DISK_FULL || ErrorState.Win32Error == ERROR_HANDLE_DISK_FULL) &&
						strDestName.size() > 1 && strDestName[1]==L':')
					{
						const auto strDriveRoot = GetPathRoot(strDestName);
						unsigned long long FreeSize = 0;

						if (os::fs::get_disk_size(strDriveRoot, nullptr, nullptr, &FreeSize))
						{
							if (FreeSize<BytesRead &&
								DestFile.Write(CopyBuffer.data(), static_cast<size_t>(FreeSize)) &&
								SrcFile.SetPointer(FreeSize-BytesRead,nullptr,FILE_CURRENT))
							{
								DestFile.Close();
								const int MsgCode=Message(MSG_WARNING, ErrorState,
									msg(lng::MError),
									{
										strDestName
									},
									{ lng::MSplit, lng::MSkip, lng::MRetry, lng::MCancel },
									L"CopyFiles"sv);

								if (MsgCode==2)
								{
									SrcFile.Close();

									if (!Append)
									{
										os::fs::set_file_attributes(strDestName,FILE_ATTRIBUTE_NORMAL);
										os::fs::delete_file(strDestName); //BUGBUG
									}

									return COPY_FAILURE;
								}

								if (!MsgCode)
								{
									Split=TRUE;

									for (;;)
									{
										if (os::fs::get_disk_size(strDriveRoot,nullptr,nullptr,&FreeSize))
										{
											if (FreeSize<BytesRead)
											{
												const int MsgCode2 = Message(MSG_WARNING,
													msg(lng::MWarning),
													{
														msg(lng::MCopyErrorDiskFull),
														strDestName
													},
													{ lng::MRetry, lng::MCancel });

												if (MsgCode2)
												{
													Split=FALSE;
													SplitCancelled=TRUE;
												}
												else
													continue;
											}
										}
										break;
									}
								}

								if (MsgCode==1)
									SplitSkipped=TRUE;

								if (MsgCode==-1 || MsgCode==3)
									SplitCancelled=TRUE;
							}
						}
					}

					if (Split)
					{
						const auto FilePtr = SrcFile.GetPointer();
						os::fs::find_data SplitData=SrcData;
						SplitData.FileSize-=FilePtr;
						int RetCode = COPY_CANCEL;
						string strNewName;

						if (!AskOverwrite(SplitData, SrcName, strDestName, INVALID_FILE_ATTRIBUTES, FALSE, (Flags&FCOPY_MOVE) != 0, (Flags&FCOPY_LINK) == 0, Append, strNewName, RetCode))
						{
							SrcFile.Close();
							return COPY_CANCEL;
						}

						if (RetCode==COPY_RETRY)
						{
							strDestName=strNewName;

							if (CutToSlash(strNewName) && !os::fs::exists(strNewName))
							{
								CreatePath(strNewName);
							}

							return COPY_RETRY;
						}

						string_view DestDir = strDestName;

						if (CutToSlash(DestDir,true))
							CreatePath(DestDir);


						if (!DestFile.Open(strDestName, GENERIC_WRITE, FILE_SHARE_READ, nullptr, (Append ? OPEN_EXISTING:CREATE_ALWAYS), SrcData.Attributes|FILE_FLAG_SEQUENTIAL_SCAN) || (Append && !DestFile.SetPointer(0,nullptr,FILE_END)))
						{
							SrcFile.Close();
							DestFile.Close();
							return COPY_FAILURE;
						}
					}
					else
					{
						if (!SplitCancelled && !SplitSkipped &&
							Message(MSG_WARNING, ErrorState,
								msg(lng::MError),
								{
									msg(lng::MCopyWriteError),
									strDestName
								},
								{ lng::MRetry, lng::MCancel }) == Message::first_button)
						{
							continue;
						}

						SrcFile.Close();

						if (Append)
						{
							DestFile.SetPointer(AppendPos,nullptr,FILE_BEGIN);
						}

						DestFile.SetEnd();
						DestFile.Close();

						if (!Append)
						{
							os::fs::set_file_attributes(strDestName,FILE_ATTRIBUTE_NORMAL);
							os::fs::delete_file(strDestName); //BUGBUG
						}

						CP->SetProgressValue(0,0);

						if (SplitSkipped)
							return COPY_SKIPPED;

						return SplitCancelled? COPY_CANCEL : COPY_FAILURE;
					}

					break;
				}
			}

			CP->UpdateCurrentBytesInfo(SrcFile.GetChunkOffset() + SrcFile.GetChunkSize());
			CP->SetProgressValue(CP->m_Bytes.CurrCopied, SrcData.FileSize);
		}
		while(SrcFile.Step());
	}

	SrcFile.Close();

	if (!(Flags&FCOPY_COPYTONUL))
	{
		if (Global->Opt->CMOpt.PreserveTimestamps)
			set_file_time(DestFile, SrcData);

		if (CopySparse)
		{
			auto Pos = SrcData.FileSize;

			if (Append)
				Pos+=AppendPos;

			DestFile.SetPointer(Pos,nullptr,FILE_BEGIN);
			DestFile.SetEnd();
		}

		DestFile.Close();
		// TODO: ЗДЕСЯ СТАВИТЬ Compressed???
		Flags&=~FCOPY_DECRYPTED_DESTINATION;

		if (!IsWindowsVistaOrGreater() && IsWindowsServer()) // M#1607 WS2003-Share SetFileTime BUG
		{
			if (FAR_GetDriveType(GetPathRoot(strDestName), 0) == DRIVE_REMOTE)
			{
				if (DestFile.Open(strDestName, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT))
				{
					if (Global->Opt->CMOpt.PreserveTimestamps)
						set_file_time(DestFile, SrcData);

					DestFile.Close();
				}
			}
		}
	}

	return COPY_SUCCESS;
}

void ShellCopy::SetDestDizPath(const string& DestPath)
{
	if (!(Flags&FCOPY_DIZREAD))
	{
		strDestDizPath = ConvertNameToFull(DestPath);
		CutToSlash(strDestDizPath);

		if (strDestDizPath.empty())
			strDestDizPath = L'.';

		if ((Global->Opt->Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && !SrcPanel->IsDizDisplayed()) ||
		        Global->Opt->Diz.UpdateMode==DIZ_NOT_UPDATE)
			strDestDizPath.clear();

		if (!strDestDizPath.empty())
			DestDiz.Read(strDestDizPath);

		Flags|=FCOPY_DIZREAD;
	}
}

enum WarnDlgItems
{
	WDLG_BORDER,
	WDLG_TEXT,
	WDLG_FILENAME,
	WDLG_SEPARATOR,
	WDLG_SRCFILEBTN,
	WDLG_DSTFILEBTN,
	WDLG_SEPARATOR2,
	WDLG_CHECKBOX,
	WDLG_SEPARATOR3,
	WDLG_OVERWRITE,
	WDLG_SKIP,
	WDLG_RENAME,
	WDLG_APPEND,
	WDLG_CANCEL,

	WDLG_COUNT
};

enum
{
 DM_OPENVIEWER = DM_USER+33,
};


struct file_names_for_overwrite_dialog
{
	const string* Src;
	string* Dest;
	string* DestPath;
};

intptr_t ShellCopy::WarnDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	switch (Msg)
	{
		case DM_OPENVIEWER:
		{
			const auto WFN = reinterpret_cast<const file_names_for_overwrite_dialog*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));

			if (WFN)
			{
				NamesList List;
				List.AddName(*WFN->Src);
				List.AddName(*WFN->Dest);
				const auto ViewName = *(Param1 == WDLG_SRCFILEBTN ? WFN->Src : WFN->Dest);
				List.SetCurName(ViewName);

				const auto Viewer = FileViewer::create(
					ViewName,
					false,
					false,
					true,
					-1,
					{},
					&List,
					false);

				if (Viewer->GetExitCode()) Global->WindowManager->ExecuteModal(Viewer);
				Global->WindowManager->ResizeAllWindows();
			}
		}
		break;
		case DN_CTLCOLORDLGITEM:
		{
			if (Param1==WDLG_FILENAME)
			{
				const auto Color = colors::PaletteColorToFarColor(COL_WARNDIALOGTEXT);
				const auto Colors = static_cast<FarDialogItemColors*>(Param2);
				Colors->Colors[0] = Color;
				Colors->Colors[2] = Color;
			}
		}
		break;
		case DN_BTNCLICK:
		{
			switch (Param1)
			{
				case WDLG_SRCFILEBTN:
				case WDLG_DSTFILEBTN:
					Dlg->SendMessage(DM_OPENVIEWER, Param1, nullptr);
					break;
				case WDLG_RENAME:
				{
					const auto WFN = reinterpret_cast<const file_names_for_overwrite_dialog*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
					string strDestName = *WFN->Dest;
					GenerateName(strDestName, *WFN->DestPath);

					if (Dlg->SendMessage(DM_GETCHECK, WDLG_CHECKBOX, nullptr) == BSTATE_UNCHECKED)
					{
						int All=BSTATE_UNCHECKED;

						if (GetString(
							msg(lng::MCopyRenameTitle),
							msg(lng::MCopyRenameText),
							{},
							strDestName,
							*WFN->Dest,
							L"CopyAskOverwrite"sv,
							FIB_BUTTONS | FIB_NOAMPERSAND | FIB_EXPANDENV | FIB_CHECKBOX,
							&All,
							msg(lng::MCopyRememberChoice)))
						{
							if (All!=BSTATE_UNCHECKED)
							{
								*WFN->DestPath = *WFN->Dest;
								CutToSlash(*WFN->DestPath);
							}

							Dlg->SendMessage(DM_SETCHECK,WDLG_CHECKBOX,ToPtr(All));
						}
						else
						{
							return TRUE;
						}
					}
					else
					{
						*WFN->Dest=strDestName;
					}
				}
				break;
			}
		}
		break;
		case DN_CONTROLINPUT:
		{
			const auto record = static_cast<const INPUT_RECORD*>(Param2);
			if (record->EventType==KEY_EVENT)
			{
				const auto key = InputRecordToKey(record);
				if ((Param1==WDLG_SRCFILEBTN || Param1==WDLG_DSTFILEBTN) && key==KEY_F3)
				{
					Dlg->SendMessage(DM_OPENVIEWER, Param1, nullptr);
				}
			}
		}
		break;

		default:
			break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

bool ShellCopy::AskOverwrite(const os::fs::find_data &SrcData,
                            const string& SrcName,
                            const string& DestName, DWORD DestAttr,
                            int SameName,int Rename,int AskAppend,
                            int &Append,string &strNewName,int &RetCode)
{
	enum
	{
		DlgH = 13,
		DlgW = 76,
	};

	const auto qDst = QuoteOuterSpace(DestName);

	auto WarnCopyDlg = MakeDialogItems<WDLG_COUNT>(
	{
		{ DI_DOUBLEBOX, {{3,  1 }, {DlgW-4, DlgH-2}}, DIF_NONE, msg(lng::MWarning), },
		{ DI_TEXT,      {{5,  2 }, {DlgW-6, 2     }}, DIF_CENTERTEXT, msg(lng::MCopyFileExist), },
		{ DI_EDIT,      {{5,  3 }, {DlgW-6, 3     }}, DIF_READONLY, qDst, },
		{ DI_TEXT,      {{-1, 4 }, {0,      4     }}, DIF_SEPARATOR, },
		{ DI_BUTTON,    {{5,  5 }, {DlgW-6, 5     }}, DIF_BTNNOCLOSE | DIF_NOBRACKETS, },
		{ DI_BUTTON,    {{5,  6 }, {DlgW-6, 6     }}, DIF_BTNNOCLOSE | DIF_NOBRACKETS, },
		{ DI_TEXT,      {{-1, 7 }, {0,      7     }}, DIF_SEPARATOR, },
		{ DI_CHECKBOX,  {{5,  8 }, {0,      8     }}, DIF_FOCUS, msg(lng::MCopyRememberChoice), },
		{ DI_TEXT,      {{-1, 9 }, {0,      9     }}, DIF_SEPARATOR, },
		{ DI_BUTTON,    {{0,  10}, {0,      10    }}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MCopyOverwrite), },
		{ DI_BUTTON,    {{0,  10}, {0,      10    }}, DIF_CENTERGROUP, msg(lng::MCopySkip), },
		{ DI_BUTTON,    {{0,  10}, {0,      10    }}, DIF_CENTERGROUP, msg(lng::MCopyRename), },
		{ DI_BUTTON,    {{0,  10}, {0,      10    }}, DIF_CENTERGROUP | (AskAppend ? DIF_NONE : (DIF_DISABLE | DIF_HIDDEN)), msg(lng::MCopyAppend), },
		{ DI_BUTTON,    {{0,  10}, {0,      10    }}, DIF_CENTERGROUP, msg(lng::MCopyCancel), },
	});

	os::fs::find_data DestData;
	int DestDataFilled=FALSE;
	Append=FALSE;

	if (Flags&FCOPY_COPYTONUL)
	{
		RetCode=COPY_SKIPPED;
		return true;
	}

	if (DestAttr==INVALID_FILE_ATTRIBUTES)
		if ((DestAttr = os::fs::get_file_attributes(DestName)) == INVALID_FILE_ATTRIBUTES)
			return true;

	if (DestAttr & FILE_ATTRIBUTE_DIRECTORY)
		return true;

	const auto Format = L"{0:26} {1:20} {2} {3}"sv;

	string strDestName = DestName;

	{
		int MsgCode = OvrMode;

		if (OvrMode == -1)
		{
			if (!(Rename? Global->Opt->Confirm.Move : Global->Opt->Confirm.Copy) || SameName)
			{
				MsgCode = 0;
			}
			else
			{
				DestData = {};
				// BUGBUG check result
				(void)os::fs::get_find_data(DestName, DestData);
				DestDataFilled = TRUE;

				if ((Flags&FCOPY_ONLYNEWERFILES))
				{
					// сравним время
					if (DestData.LastWriteTime < SrcData.LastWriteTime)
						MsgCode = 0;
					else
						MsgCode = 2;
				}
				else
				{
					unsigned long long SrcSize = SrcData.FileSize;
					auto SrcLastWriteTime = SrcData.LastWriteTime;
					if ((Flags & FCOPY_COPYSYMLINKCONTENTS) && os::fs::is_directory_symbolic_link(SrcData))
					{
						os::fs::find_data FindData;
						if (os::fs::get_find_data(ConvertNameToReal(SrcName), FindData))
						{
							SrcSize = FindData.FileSize;
							SrcLastWriteTime = FindData.LastWriteTime;
						}
					}

					string strDateText, strTimeText;
					ConvertDate(SrcLastWriteTime, strDateText, strTimeText, 8, 1);
					WarnCopyDlg[WDLG_SRCFILEBTN].strData = format(Format, msg(lng::MCopySource), SrcSize, strDateText, strTimeText);

					ConvertDate(DestData.LastWriteTime, strDateText, strTimeText, 8, 1);
					WarnCopyDlg[WDLG_DSTFILEBTN].strData = format(Format, msg(lng::MCopyDest), DestData.FileSize, strDateText, strTimeText);

					const auto strFullSrcName = ConvertNameToFull(SrcName);
					file_names_for_overwrite_dialog WFN{ &strFullSrcName, &strDestName, &strRenamedFilesPath };
					const auto WarnDlg = Dialog::create(WarnCopyDlg, &ShellCopy::WarnDlgProc, &WFN);
					WarnDlg->SetDialogMode(DMODE_WARNINGSTYLE);
					WarnDlg->SetPosition({ -1, -1, DlgW, DlgH });
					WarnDlg->SetHelp(L"CopyAskOverwrite"sv);
					WarnDlg->SetId(CopyOverwriteId);
					WarnDlg->Process();

					switch (WarnDlg->GetExitCode())
					{
					case WDLG_OVERWRITE:
						MsgCode = WarnCopyDlg[WDLG_CHECKBOX].Selected ? 1 : 0;
						break;
					case WDLG_SKIP:
						MsgCode = WarnCopyDlg[WDLG_CHECKBOX].Selected ? 3 : 2;
						break;
					case WDLG_RENAME:
						MsgCode = WarnCopyDlg[WDLG_CHECKBOX].Selected ? 5 : 4;
						break;
					case WDLG_APPEND:
						MsgCode = WarnCopyDlg[WDLG_CHECKBOX].Selected ? 7 : 6;
						break;
					case -1:
					case -2:
					case WDLG_CANCEL:
						MsgCode = 8;
						break;
					}
				}
			}
		}

		switch (MsgCode)
		{
		case 1:
			OvrMode = 1;
			[[fallthrough]];
		case 0:
			break;
		case 3:
			OvrMode = 2;
			[[fallthrough]];
		case 2:
			RetCode = COPY_SKIPPED;
			return false;
		case 5:
			OvrMode = 5;
			GenerateName(strDestName, strRenamedFilesPath);
			[[fallthrough]];
		case 4:
			RetCode = COPY_RETRY;
			strNewName = strDestName;
			break;
		case 7:
			OvrMode = 6;
			[[fallthrough]];
		case 6:
			Append = TRUE;
			break;
		case -1:
		case -2:
		case 8:
			RetCode = COPY_CANCEL;
			return false;
		}
	}

	if (RetCode!=COPY_RETRY)
	{
		if (DestAttr & FILE_ATTRIBUTE_READONLY)
		{
			int MsgCode=0;

			if (!SameName)
			{
				if (ReadOnlyOvrMode!=-1)
				{
					MsgCode=ReadOnlyOvrMode;
				}
				else
				{
					if (!DestDataFilled)
					{
						DestData = {};
						// BUGBUG check result
						(void)os::fs::get_find_data(DestName, DestData);
					}

					string strDateText, strTimeText;
					ConvertDate(SrcData.LastWriteTime, strDateText, strTimeText, 8, 1);
					WarnCopyDlg[WDLG_SRCFILEBTN].strData = format(Format, msg(lng::MCopySource), SrcData.FileSize, strDateText, strTimeText);

					ConvertDate(DestData.LastWriteTime, strDateText, strTimeText, 8, 1);
					WarnCopyDlg[WDLG_DSTFILEBTN].strData = format(Format, msg(lng::MCopyDest), DestData.FileSize, strDateText, strTimeText);

					WarnCopyDlg[WDLG_TEXT].strData = msg(lng::MCopyFileRO);
					WarnCopyDlg[WDLG_OVERWRITE].strData = msg(Append? lng::MCopyAppend : lng::MCopyOverwrite);
					WarnCopyDlg[WDLG_RENAME].Type = DI_TEXT;
					WarnCopyDlg[WDLG_RENAME].strData.clear();
					WarnCopyDlg[WDLG_APPEND].Type = DI_TEXT;
					WarnCopyDlg[WDLG_APPEND].strData.clear();

					const auto strSrcName = ConvertNameToFull(SrcData.FileName);
					file_names_for_overwrite_dialog WFN{ &strSrcName, &strDestName };
					const auto WarnDlg = Dialog::create(WarnCopyDlg, &ShellCopy::WarnDlgProc, &WFN);
					WarnDlg->SetDialogMode(DMODE_WARNINGSTYLE);
					WarnDlg->SetPosition({ -1, -1, DlgW, DlgH });
					WarnDlg->SetHelp(L"CopyFiles"sv);
					WarnDlg->SetId(CopyReadOnlyId);
					WarnDlg->Process();

					switch (WarnDlg->GetExitCode())
					{
						case WDLG_OVERWRITE:
							MsgCode=WarnCopyDlg[WDLG_CHECKBOX].Selected?1:0;
							break;
						case WDLG_SKIP:
							MsgCode=WarnCopyDlg[WDLG_CHECKBOX].Selected?3:2;
							break;
						case -1:
						case -2:
						case WDLG_CANCEL:
							MsgCode=8;
							break;
					}
				}
			}

			switch (MsgCode)
			{
				case 1:
					ReadOnlyOvrMode=1;
					[[fallthrough]];
				case 0:
					break;
				case 3:
					ReadOnlyOvrMode=2;
					[[fallthrough]];
				case 2:
					RetCode=COPY_SKIPPED;
					return false;
				case -1:
				case -2:
				case 8:
					RetCode=COPY_CANCEL;
					return false;
			}
		}

		if (!SameName && (DestAttr & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)))
			os::fs::set_file_attributes(DestName,FILE_ATTRIBUTE_NORMAL);
	}

	return true;
}



bool ShellCopy::GetSecurity(const string& FileName, os::security::descriptor& sd)
{
	for (;;)
	{
		sd = os::fs::get_file_security(FileName, DACL_SECURITY_INFORMATION);
		if (sd)
			return true;

		if (SkipSecurityErrors)
			return true;

		const auto ErrorState = error_state::fetch();

		switch (OperationFailed(ErrorState, FileName, lng::MError, msg(lng::MCannotGetSecurity)))
		{
		case operation::retry:
			continue;

		case operation::skip:
			return true;

		case operation::skip_all:
			SkipSecurityErrors = true;
			return true;

		default:
			return false;
		}
	}
}

bool ShellCopy::SetSecurity(const string& FileName, const os::security::descriptor& sd)
{
	for (;;)
	{
 		if (os::fs::set_file_security(FileName, DACL_SECURITY_INFORMATION, sd))
			return true;

		if (SkipSecurityErrors)
			return true;

		const auto ErrorState = error_state::fetch();

		switch (OperationFailed(ErrorState, FileName, lng::MError, msg(lng::MCannotSetSecurity)))
		{
		case operation::retry:
			continue;

		case operation::skip:
			return true;

		case operation::skip_all:
			SkipSecurityErrors = true;
			return true;

		default:
			return false;
		}
	}
}

// BUGBUG move to copy_progress
static bool ShellCopySecuryMsg(const copy_progress* CP, const string& Name)
{
	if (Name.empty() || CP->m_SecurityTimeCheck)
	{
		static int Width=30;
		int WidthTemp;
		if (!Name.empty())
		{
			WidthTemp=std::max(static_cast<int>(Name.size()),30);
		}
		else
			Width=WidthTemp=30;

		WidthTemp=std::min(WidthTemp, ScrX/2);
		Width=std::max(Width,WidthTemp);

		auto strOutFileName = truncate_right(Name, Width);
		inplace::fit_to_center(strOutFileName, Width + 4);
		Message(0,
			msg(lng::MMoveDlgTitle),
			{
				msg(lng::MCopyPrepareSecury),
				std::move(strOutFileName)
			},
			{});

		if (CP->IsCancelled())
		{
			return false;
		}
	}

	return true;
}

bool ShellCopy::ResetSecurity(const string& FileName)
{
	for (;;)
	{
		if (os::fs::reset_file_security(FileName))
			return true;

		if (SkipSecurityErrors)
			return true;

		const auto ErrorState = error_state::fetch();

		switch (OperationFailed(ErrorState, FileName, lng::MError, msg(lng::MCannotSetSecurity)))
		{
		case operation::retry:
			continue;

		case operation::skip:
			return true;

		case operation::skip_all:
			SkipSecurityErrors = true;
			return true;

		default:
			return false;
		}
	}
}

bool ShellCopy::ResetSecurityRecursively(const string& FileName)
{
	if (!ResetSecurity(FileName))
		return false;

	if (os::fs::is_directory(FileName))
	{
		ScanTree ScTree(true, true, Flags & FCOPY_COPYSYMLINKCONTENTS);
		ScTree.SetFindPath(FileName, L"*"sv);

		string strFullName;
		os::fs::find_data SrcData;
		while (ScTree.GetNextName(SrcData,strFullName))
		{
			if (!ShellCopySecuryMsg(CP.get(), strFullName))
				break;

			if (!ResetSecurity(strFullName))
			{
				return false;
			}
		}
	}

	return true;
}

int ShellCopy::ShellSystemCopy(const string& SrcName,const string& DestName,const os::fs::find_data &SrcData)
{
	os::security::descriptor sd;

	if (m_CopySecurity == security::copy && !GetSecurity(SrcName, sd))
		return COPY_CANCEL;

	CP->SetNames(SrcName,DestName);
	CP->SetProgressValue(0,0);

	m_FileHandleForStreamSizeFix = nullptr;

	struct callback_data
	{
		ShellCopy* Owner;
		std::exception_ptr ExceptionPtr;
	};

	struct callback_wrapper
	{
		static DWORD CALLBACK callback(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred, DWORD StreamNumber, DWORD CallbackReason, HANDLE SourceFile, HANDLE DestinationFile, LPVOID Data)
		{
			const auto CallbackData = static_cast<callback_data*>(Data);
			try
			{
				return CallbackData->Owner->CopyProgressRoutine(TotalFileSize.QuadPart, TotalBytesTransferred.QuadPart, StreamSize.QuadPart, StreamBytesTransferred.QuadPart, StreamNumber, CallbackReason, SourceFile, DestinationFile);
			}
			CATCH_AND_SAVE_EXCEPTION_TO(CallbackData->ExceptionPtr)

			return PROGRESS_CANCEL;
		}
	};

	callback_data CallbackData{ this };
	if (!os::fs::copy_file(SrcName, DestName, callback_wrapper::callback, &CallbackData, nullptr, Flags&FCOPY_DECRYPTED_DESTINATION ? COPY_FILE_ALLOW_DECRYPTED_DESTINATION : 0))
	{
		rethrow_if(CallbackData.ExceptionPtr);
		Flags&=~FCOPY_DECRYPTED_DESTINATION;
		return (GetLastError() == ERROR_REQUEST_ABORTED)? COPY_CANCEL : COPY_FAILURE;
	}

	Flags&=~FCOPY_DECRYPTED_DESTINATION;

	if (sd && !SetSecurity(DestName, sd))
		return COPY_CANCEL;

	if (Global->Opt->CMOpt.PreserveTimestamps)
	{
		if (const auto DestFile = os::fs::file(DestName, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT))
		{
			set_file_time(DestFile, SrcData);
		}
	}

	return COPY_SUCCESS;
}

DWORD ShellCopy::CopyProgressRoutine(unsigned long long TotalFileSize, unsigned long long TotalBytesTransferred, unsigned long long StreamSize, unsigned long long StreamBytesTransferred, DWORD StreamNumber, DWORD dwCallbackReason, HANDLE hSourceFile, HANDLE hDestinationFile)
{
	// // _LOGCOPYR(CleverSysLog clv(L"CopyProgressRoutine"));
	// // _LOGCOPYR(SysLog(L"dwStreamNumber=%d",dwStreamNumber));
	bool Abort = false;
	if (CP->IsCancelled())
	{
		Abort=true;
	}

	CheckAndUpdateConsole();
	//fix total size
	if (StreamNumber == 1 && hSourceFile != m_FileHandleForStreamSizeFix)
	{
		CP->m_Bytes.Total -= StreamSize;
		CP->m_Bytes.Total += TotalFileSize;
		m_FileHandleForStreamSizeFix = hSourceFile;
	}

	CP->UpdateCurrentBytesInfo(TotalBytesTransferred);
	CP->SetProgressValue(TotalBytesTransferred, TotalFileSize);

	return Abort?PROGRESS_CANCEL:PROGRESS_CONTINUE;
}

bool ShellCopy::CalcTotalSize() const
{
	CP->m_Bytes.Total = 0;
	CP->m_Bytes.CurrCopied = 0;
	CP->m_Files.Total = 0;

	time_check TimeCheck(time_check::mode::delayed, GetRedrawTimeout());

	const auto DirInfoCallback = [&](string_view const Name, unsigned long long const ItemsCount, unsigned long long const Size)
	{
		if (TimeCheck)
			DirInfoMsg(msg(Flags & FCOPY_MOVE? lng::MMoveDlgTitle : lng::MCopyDlgTitle), Name, CP->m_Files.Total + ItemsCount, CP->m_Bytes.Total + Size);
	};

	for (const auto& i: SrcPanel->enum_selected())
	{
		if (!(Flags&FCOPY_COPYSYMLINKCONTENTS) && os::fs::is_directory_symbolic_link(i))
			continue;

		if (i.Attributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			DirInfoData Data{};
			if (GetDirInfo(i.FileName, Data, m_Filter.get(), DirInfoCallback, (Flags&FCOPY_COPYSYMLINKCONTENTS? GETDIRINFO_SCANSYMLINK : 0) | (m_UseFilter? GETDIRINFO_USEFILTER : 0)) <= 0)
				return false;

			if (Data.FileCount > 0) // Not everything filtered out
			{
				CP->m_Bytes.Total += Data.FileSize;
				CP->m_Files.Total += Data.DirCount + Data.FileCount + 1;
			}
		}
		else
		{
			//  Подсчитаем количество файлов
			if (m_UseFilter && !m_Filter->FileInFilter(i, nullptr, &i.FileName))
				continue;

			CP->m_Bytes.Total += i.FileSize;
			++CP->m_Files.Total;
		}
	}

	// INFO: Это для варианта, когда "ВСЕГО = общий размер * количество целей"
	CP->m_Bytes.Total *= m_NumberOfTargets;
	CP->m_Files.Total *= m_NumberOfTargets;
	return true;
}

/*
  Оболочка вокруг SetFileAttributes() для
  корректного выставления атрибутов
*/
bool ShellCopy::ShellSetAttr(const string& Dest, DWORD Attr)
{
	DWORD FileSystemFlagsDst=0;
	if ((Attr & (FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_ENCRYPTED)) && os::fs::GetVolumeInformation(GetPathRoot(Dest), nullptr, nullptr, nullptr, &FileSystemFlagsDst, nullptr))
	{
		if (!(FileSystemFlagsDst & FILE_FILE_COMPRESSION))
		{
			Attr &= ~FILE_ATTRIBUTE_COMPRESSED;
		}
		if (!(FileSystemFlagsDst & FILE_SUPPORTS_ENCRYPTION))
		{
			Attr &= ~FILE_ATTRIBUTE_ENCRYPTED;
		}
	}

	if (ESetFileAttributes(Dest, Attr & ~(FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_ENCRYPTED), SkipErrors) == setattr_result::cancel)
		return false;

	if (Attr & FILE_ATTRIBUTE_COMPRESSED)
	{
		if (ESetFileCompression(Dest, true, Attr& ~FILE_ATTRIBUTE_COMPRESSED, SkipErrors) == setattr_result::cancel)
			return false;
	}

	// При копировании/переносе выставляем FILE_ATTRIBUTE_ENCRYPTED
	// для каталога, если он есть
	if (Attr & FILE_ATTRIBUTE_ENCRYPTED && Attr & FILE_ATTRIBUTE_DIRECTORY)
	{
		if (ESetFileEncryption(Dest, true, Attr & ~FILE_ATTRIBUTE_ENCRYPTED, SkipErrors) == setattr_result::cancel)
			return false;
	}

	return true;
}

ShellCopy::created_folders::created_folders(const string& FullName, const os::fs::find_data& FindData):
	FullName(FullName),
	CreationTime(FindData.CreationTime),
	LastAccessTime(FindData.LastAccessTime),
	LastWriteTime(FindData.LastWriteTime),
	ChangeTime(FindData.ChangeTime)
{
}
