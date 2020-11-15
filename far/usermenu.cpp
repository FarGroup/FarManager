/*
usermenu.cpp

User menu и есть
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
#include "usermenu.hpp"

// Internal:
#include "keys.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "dialog.hpp"
#include "fileedit.hpp"
#include "preservelongname.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "message.hpp"
#include "fnparce.hpp"
#include "pathmix.hpp"
#include "panelmix.hpp"
#include "filestr.hpp"
#include "interf.hpp"
#include "lang.hpp"
#include "string_utils.hpp"
#include "exception.hpp"
#include "uuids.far.dialogs.hpp"
#include "global.hpp"
#include "file_io.hpp"
#include "keyboard.hpp"

// Platform:
#include "platform.env.hpp"
#include "platform.fs.hpp"

// Common:
#include "common/from_string.hpp"
#include "common/range.hpp"
#include "common/scope_exit.hpp"

// External:

//----------------------------------------------------------------------------

enum
{
	default_menu_file_codepage = CP_UTF8
};


#if defined(PROJECT_DI_MEMOEDIT)
/*
  Идея в следующем.
  1. Строки в реестре хранятся как и раньше, т.к. CommandXXX
  2. Для DI_MEMOEDIT мы из только преобразовываем в один массив
*/
#endif


// Режимы показа меню (Menu mode)
enum class UserMenu::menu_mode: int
{
	local,
	user,
	global,
};

// Коды выхода из меню (Exit codes)
enum
{
	EC_CLOSE_LEVEL      = -1, // Выйти из меню на один уровень вверх
	EC_CLOSE_MENU       = -2, // Выйти из меню по SHIFT+F10
	EC_PARENT_MENU      = -3, // Показать меню родительского каталога
	EC_MAIN_MENU        = -4, // Показать главное меню
	EC_COMMAND_SELECTED = -5, // Выбрана команда - закрыть меню и обновить папку
};

static int PrepareHotKey(string &strHotKey)
{
	int FuncNum=0;

	if (strHotKey.size() > 1 && upper(strHotKey.front()) == L'F')
	{
		if (!from_string(string_view(strHotKey).substr(1), FuncNum) || FuncNum < 1 || FuncNum > 24)
		{
			FuncNum = 0;
			strHotKey.clear();
		}
	}
	else
	{
		// при наличии "&" продублируем
		if (strHotKey == L"&"sv)
			strHotKey.assign(2, L'&');
	}

	return FuncNum;
}

static const auto LocalMenuFileName = L"FarMenu.ini"sv;

struct UserMenu::UserMenuItem
{
	string strHotKey;
	string strLabel;
	std::list<string> Commands;
	bool Submenu{};
	menu_container Menu;
};

static string SerializeMenu(const UserMenu::menu_container& Menu)
{
	string Result;
	const auto Eol = eol::system.str();

	for (const auto& i: Menu)
	{
		auto HotkeyStr = pad_right(i.strHotKey + L':', 5);
		append(Result, HotkeyStr, i.strLabel, Eol);

		if (i.Submenu)
		{
			append(Result, L'{', Eol, SerializeMenu(i.Menu), L'}', Eol);
		}
		else
		{
			const string Padding(HotkeyStr.size(), L' ');
			for (const auto& str: i.Commands)
			{
				append(Result, Padding, str, Eol);
			}
		}
	}
	return Result;
}

static void ParseMenu(UserMenu::menu_container& Menu, range<enum_lines::iterator> const FileStrings, bool OldFormat)
{
	UserMenu::menu_container::value_type *MenuItem = nullptr;

	for (auto i = FileStrings.begin(); i != FileStrings.end(); ++i)
	{
		const auto MenuStr = trim_right(i->Str);

		if (MenuStr.empty())
			continue;

		if (MenuStr.front() == L'{' && MenuItem)
		{
			ParseMenu(MenuItem->Menu, { i, FileStrings.end() }, OldFormat);
			MenuItem->Submenu = true;
			MenuItem = nullptr;
			continue;
		}

		// '}' can be a hotkey as well
		if (MenuStr.front() == L'}' && !(MenuStr.size() > 1 && MenuStr[1] == L':'))
			break;

		if (!std::iswblank(MenuStr.front()))
		{
			auto ChPos = MenuStr.find(L':');

			if (ChPos == string::npos)
				continue;

			// special case: hotkey is ':'
			if (ChPos + 1 != MenuStr.size() && MenuStr[ChPos + 1] == ':')
			{
				++ChPos;
			}

			UserMenu::menu_container::value_type NewItem{};

			NewItem.strHotKey = MenuStr.substr(0, ChPos);
			NewItem.strLabel = trim_left(MenuStr.substr(ChPos + 1));

			// Support for old 1.x separator format
			if (OldFormat && NewItem.strHotKey == L"-"sv && NewItem.strLabel.empty())
			{
				NewItem.strHotKey += L'-';
			}

			Menu.emplace_back(std::move(NewItem));
			MenuItem = &Menu.back();
		}
		else if (MenuItem)
		{
			MenuItem->Commands.emplace_back(trim_left(MenuStr));
		}
	}
}

static void DeserializeMenu(UserMenu::menu_container& Menu, const os::fs::file& File, uintptr_t& Codepage)
{
	Codepage = GetFileCodepage(File, encoding::codepage::oem());

	os::fs::filebuf StreamBuffer(File, std::ios::in);
	std::istream Stream(&StreamBuffer);
	Stream.exceptions(Stream.badbit | Stream.failbit);

	enum_lines EnumFileLines(Stream, Codepage);
	ParseMenu(Menu, EnumFileLines, Codepage == encoding::codepage::oem());

	if (!IsUnicodeOrUtfCodePage(Codepage))
	{
		Codepage = default_menu_file_codepage;
	}
}

UserMenu::UserMenu(bool ChooseMenuType):
	m_MenuMode(menu_mode::local),
	m_MenuModified(false),
	m_ItemChanged(false),
	m_MenuCP(default_menu_file_codepage)
{
	ProcessUserMenu(ChooseMenuType, {});
}

UserMenu::UserMenu(string_view const MenuFileName):
	m_MenuMode(menu_mode::local),
	m_MenuModified(false),
	m_ItemChanged(false),
	m_MenuCP(default_menu_file_codepage)
{
	ProcessUserMenu(false, MenuFileName);
}

UserMenu::~UserMenu() = default;

void UserMenu::SaveMenu(string_view const MenuFileName) const
{
	if (!m_MenuModified)
		return;

	SCOPE_SUCCESS{ m_MenuModified = false; };

	const auto SerialisedMenu = SerializeMenu(m_Menu);

	const auto FileAttr = os::fs::get_file_attributes(MenuFileName);

	if (FileAttr != INVALID_FILE_ATTRIBUTES && FileAttr & FILE_ATTRIBUTE_READONLY)
	{
		if (Message(MSG_WARNING,
			msg(lng::MUserMenuTitle),
			{
				string(MenuFileName),
				msg(lng::MEditRO),
				msg(lng::MEditOvr)
			},
			{ lng::MYes, lng::MNo }) != Message::first_button)
			return;

		(void)os::fs::set_file_attributes(MenuFileName, FileAttr & ~FILE_ATTRIBUTE_READONLY); //BUGBUG
	}

	try
	{
		if (SerialisedMenu.empty())
		{
			if (!os::fs::delete_file(MenuFileName))
				throw MAKE_FAR_EXCEPTION(L"Can't delete the file"sv);

			return;
		}

		save_file_with_replace(MenuFileName, FileAttr, 0, false, [&](std::ostream& Stream)
		{
			encoding::writer Writer(Stream, m_MenuCP);
			Writer.write(SerialisedMenu);
		});
	}
	catch (const far_exception& e)
	{
		Message(MSG_WARNING, e,
			msg(lng::MError),
			{
				msg(lng::MEditMenuError)
			},
			{ lng::MOk });
	}
}

void UserMenu::ProcessUserMenu(bool ChooseMenuType, string_view const MenuFileName)
{
	// Путь к текущему каталогу с файлом LocalMenuFileName
	auto strMenuFilePath = Global->CtrlObject->Cp()->ActivePanel()->GetCurDir();
	// по умолчанию меню - это FarMenu.ini
	m_MenuMode = menu_mode::local;
	m_MenuModified = false;

	if (ChooseMenuType)
	{
		const int EditChoice = Message(0,
			msg(lng::MUserMenuTitle),
			{
				msg(lng::MChooseMenuType)
			},
			{ lng::MChooseMenuMain, lng::MChooseMenuLocal, lng::MCancel });

		if (EditChoice<0 || EditChoice==2)
			return;

		if (!EditChoice)
		{
			m_MenuMode = menu_mode::global;
			strMenuFilePath = Global->Opt->GlobalUserMenuDir;
		}
	}

	// основной цикл обработки
	bool FirstRun=true;
	int ExitCode = 0;

	while ((ExitCode != EC_CLOSE_LEVEL) && (ExitCode != EC_CLOSE_MENU) && (ExitCode != EC_COMMAND_SELECTED))
	{
		m_Menu.clear();

		const auto strMenuFileFullPath = !MenuFileName.empty()?
			string(MenuFileName) :
			path::join(strMenuFilePath, LocalMenuFileName);

		// Пытаемся открыть файл на локальном диске
		if (os::fs::is_file(strMenuFileFullPath))
		{
			if (const auto MenuFile = os::fs::file(strMenuFileFullPath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING))
				DeserializeMenu(m_Menu, MenuFile, m_MenuCP);
		}
		else if (m_MenuMode != menu_mode::user)
		{
			// Файл не открылся. Смотрим дальше.
			if (m_MenuMode == menu_mode::global) // был в %FARHOME%?
			{
				m_MenuMode = menu_mode::user;
				strMenuFilePath = Global->Opt->ProfilePath;
				continue;
			}
			else if (!ChooseMenuType)
			{
				if (!FirstRun)
				{
					// подымаемся выше...
					if (CutToParent(strMenuFilePath))
					{
						continue;
					}
				}

				FirstRun = false;
				m_MenuMode = menu_mode::global;
				strMenuFilePath = Global->Opt->GlobalUserMenuDir;
				continue;
			}
		}

		string MenuTitle;

		switch (m_MenuMode)
		{
		case menu_mode::local:
			MenuTitle = msg(lng::MLocalMenuTitle);
			break;

		case menu_mode::global:
		case menu_mode::user:
			MenuTitle = concat(msg(lng::MMainMenuTitle), L" ("sv, msg(m_MenuMode == menu_mode::global? lng::MMainMenuGlobal : lng::MMainMenuUser), L')');
			break;
		}

		// вызываем меню
		ExitCode=ProcessSingleMenu(m_Menu, 0, m_Menu, strMenuFileFullPath, MenuTitle);

		// ...запишем изменения обратно в файл
		SaveMenu(strMenuFileFullPath);

		// что было после вызова меню?
		switch (ExitCode)
		{
				// Показать меню родительского каталога
			case EC_PARENT_MENU:
			{
				if (m_MenuMode == menu_mode::local)
				{
					if (CutToParent(strMenuFilePath))
					{
						continue;
					}

					m_MenuMode = menu_mode::global;
					strMenuFilePath = Global->Opt->GlobalUserMenuDir;
				}
				else
				{
					m_MenuMode = menu_mode::user;
					strMenuFilePath = Global->Opt->ProfilePath;
				}

				break;
			}
			// Показать главное меню
			case EC_MAIN_MENU:
			{
				// $ 14.07.2000 VVM: Shift+F2 переключает Главное меню/локальное в цикле
				switch (m_MenuMode)
				{
					case menu_mode::local:
						m_MenuMode = menu_mode::global;
						strMenuFilePath = Global->Opt->GlobalUserMenuDir;
						break;

					case menu_mode::global:
						m_MenuMode = menu_mode::user;
						strMenuFilePath = Global->Opt->ProfilePath;
						break;

					case menu_mode::user:
						strMenuFilePath = Global->CtrlObject->CmdLine()->GetCurDir();
						m_MenuMode = menu_mode::local;
				}

				break;
			}
		}
	}

	if (Global->WindowManager->IsPanelsActive() && (ExitCode == EC_COMMAND_SELECTED || m_MenuModified))
		ShellUpdatePanels(Global->CtrlObject->Cp()->ActivePanel(), false);
}

using fkey_to_pos_map = std::array<int, 24>;

// заполнение меню
static void FillUserMenu(VMenu2& FarUserMenu, UserMenu::menu_container& Menu, int MenuPos, fkey_to_pos_map& FuncPos, const subst_context& SubstContext)
{
	FarUserMenu.clear();
	FuncPos.fill(-1);
	int NumLines = -1;

	FOR_RANGE(Menu, MenuItem)
	{
		++NumLines;
		MenuItemEx FarUserMenuItem;
		int FuncNum=0;

		// сепаратором является случай, когда хоткей == "--"
		if (MenuItem->strHotKey == L"--"sv)
		{
			FarUserMenuItem.Flags|=LIF_SEPARATOR;
			FarUserMenuItem.Name = MenuItem->strLabel;

			if (NumLines==MenuPos)
			{
				MenuPos++;
			}
		}
		else
		{
			string strLabel = MenuItem->strLabel;
			SubstFileName(strLabel, SubstContext, {}, true, {}, true);
			strLabel = os::env::expand(strLabel);
			string strHotKey = MenuItem->strHotKey;
			FuncNum = PrepareHotKey(strHotKey);
			const auto have_hotkey = !strHotKey.empty();
			const auto Offset = have_hotkey && strHotKey.front() == L'&'? 5 : 4;
			strHotKey.resize(Offset, L' ');
			FarUserMenuItem.Name = concat(have_hotkey && !FuncNum? L"&"sv : L""sv, strHotKey, strLabel);

			if (MenuItem->Submenu)
			{
				FarUserMenuItem.Flags|=MIF_SUBMENU;
			}

			FarUserMenuItem.SetSelect(NumLines==MenuPos);
		}

		FarUserMenuItem.ComplexUserData = MenuItem;
		const auto ItemPos = FarUserMenu.AddItem(FarUserMenuItem);

		if (FuncNum>0)
		{
			FuncPos[FuncNum-1]=ItemPos;
		}
	}
}

// обработка единичного меню
int UserMenu::ProcessSingleMenu(std::list<UserMenuItem>& Menu, int MenuPos, std::list<UserMenuItem>& MenuRoot, string_view const MenuFileName, const string& Title)
{
	for (;;)
	{
		string Names[2];
		Global->CtrlObject->Cp()->ActivePanel()->GetCurName(Names[0], Names[1]);
		const auto& strName = Names[0];
		const auto& strShortName = Names[1];
		const subst_context Context(strName, strShortName);

		/* $ 24.07.2000 VVM + При показе главного меню в заголовок добавляет тип - FAR/Registry */

		const auto UserMenu = VMenu2::create(Title, {}, ScrY - 4);
		UserMenu->SetMenuFlags(VMENU_WRAPMODE | VMENU_NOMERGEBORDER);
		UserMenu->SetHelp(L"UserMenu"sv);
		UserMenu->SetPosition({ -1, -1, 0, 0 });
		UserMenu->SetBottomTitle(KeysToLocalizedText(KEY_INS, KEY_DEL, KEY_F4, KEY_ALTF4, KEY_CTRLUP, KEY_CTRLDOWN));
		UserMenu->SetMacroMode(MACROAREA_USERMENU);

		int ReturnCode=1;

		fkey_to_pos_map FuncPos;

		FillUserMenu(*UserMenu, Menu, MenuPos, FuncPos, Context);

		const auto ExitCode = UserMenu->Run([&](const Manager::Key& RawKey)
		{
			const auto Key=RawKey();
			MenuPos=UserMenu->GetSelectPos();
			// CurrentMenuItem can be nullptr if:
			// - menu is empty
			// - menu is not empty, but insidiously consists only of separators
			const auto CurrentMenuItem = UserMenu->GetComplexUserDataPtr<ITERATOR(Menu)>(MenuPos);
			if (Key==KEY_SHIFTF1)
			{
				UserMenu->Key(KEY_F1);
				return 1;
			}
			if (Key==KEY_SHIFTF11)
			{
				UserMenu->Key(KEY_F11);
				return 1;
			}
			if (Key>=KEY_F1 && Key<=KEY_F24)
			{
				int FuncItemPos;

				if ((FuncItemPos=FuncPos[Key-KEY_F1])!=-1)
				{
					UserMenu->Close(FuncItemPos);
					return 1;
				}
			}
			else if (Key == L' ') // исключаем пробел из "хоткеев"!
				return 1;

			int KeyProcessed = 1;
			switch (Key)
			{
					/* $ 24.08.2001 VVM + Стрелки вправо/влево открывают/закрывают подменю соответственно */
				case KEY_RIGHT:
				case KEY_NUMPAD6:
				case KEY_MSWHEEL_RIGHT:
					if (CurrentMenuItem && (*CurrentMenuItem)->Submenu)
						UserMenu->Close(MenuPos);
					break;

				case KEY_LEFT:
				case KEY_NUMPAD4:
				case KEY_MSWHEEL_LEFT:
					if (&Menu != &MenuRoot)
						UserMenu->Close(-1);
					break;

				case KEY_NUMDEL:
				case KEY_DEL:
					if (CurrentMenuItem)
					{
						DeleteMenuRecord(Menu, *CurrentMenuItem);
						FillUserMenu(*UserMenu, Menu, MenuPos, FuncPos, Context);
					}
					break;

				case KEY_INS:
				case KEY_F4:
				case KEY_SHIFTF4:
				case KEY_NUMPAD0:
				{
					const auto IsNew = any_of(Key, KEY_INS, KEY_NUMPAD0);
					if (!IsNew && !CurrentMenuItem)
						break;

					EditMenu(Menu, CurrentMenuItem, IsNew);
					FillUserMenu(*UserMenu, Menu, MenuPos, FuncPos, Context);
					break;
				}

				case KEY_CTRLUP:
				case KEY_RCTRLUP:
				case KEY_CTRLDOWN:
				case KEY_RCTRLDOWN:
				{
					if (!CurrentMenuItem)
						break;

					const auto Up = any_of(Key, KEY_CTRLUP, KEY_RCTRLUP);
					const auto Pos = UserMenu->GetSelectPos();

					if ((Up && !Pos) || (!Up && Pos == static_cast<int>(UserMenu->size() - 1)))
						break;

					m_MenuModified = true;
					auto Other = *CurrentMenuItem;

					if (Up)
					{
						--Other;
						--MenuPos;
					}
					else
					{
						++Other;
						++MenuPos;
					}
					node_swap(Menu, *CurrentMenuItem, Other);

					FillUserMenu(*UserMenu, Menu, MenuPos, FuncPos, Context);
				}
				break;

				case KEY_ALTF4:       // редактировать все меню
				case KEY_RALTF4:
				{
					SaveMenu(MenuFileName);
					{
						const auto ShellEditor = FileEditor::create(MenuFileName, m_MenuCP, FFILEEDIT_DISABLEHISTORY, -1, -1, nullptr);
						if (-1 == ShellEditor->GetExitCode()) Global->WindowManager->ExecuteModal(ShellEditor);
						if (!ShellEditor->IsFileChanged())
							break;
					}
					if (const auto MenuFile = os::fs::file(MenuFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING))
					{
						MenuRoot.clear();
						DeserializeMenu(MenuRoot, MenuFile, m_MenuCP);
						ReturnCode = 0;
						UserMenu->Close(-1);
						return 1; // Закрыть меню
					}
				}
				[[fallthrough]];
				/* $ 28.06.2000 tran
				выход из пользовательского меню по ShiftF10 из любого уровня
				вложенности просто задаем ExitCode -1, и возвращаем FALSE -
				по FALSE оно и выйдет откуда угодно */
				case KEY_SHIFTF10:
					//UserMenu->SetExitCode(-1);
					ReturnCode=EC_CLOSE_MENU;
					UserMenu->Close(-1);
					return 1;

				case KEY_SHIFTF2: // Показать главное меню
					ReturnCode=EC_MAIN_MENU;
					UserMenu->Close(-1);
					return 1;

				case KEY_BS: // Показать меню из родительского каталога только в MM_LOCAL режиме
					if (m_MenuMode == menu_mode::local)
					{
						ReturnCode=EC_PARENT_MENU;
						UserMenu->Close(-1);
						return 1;
					}
					[[fallthrough]];
				default:
					KeyProcessed = 0;
					break;
			}
			return KeyProcessed;
		});

		if (ReturnCode!=1)
			return ReturnCode;

		if (ExitCode < 0)
			return EC_CLOSE_LEVEL; //  вверх на один уровень

		// This time CurrentMenuItem shall never be nullptr - for all weird cases ExitCode must be -1
 		const auto CurrentMenuItem = UserMenu->GetComplexUserDataPtr<ITERATOR(Menu)>(UserMenu->GetSelectPos());

		auto CurrentLabel = (*CurrentMenuItem)->strLabel;
		SubstFileName(CurrentLabel, Context, {}, true, {}, true);
		CurrentLabel = os::env::expand(CurrentLabel);

		if ((*CurrentMenuItem)->Submenu)
		{
			/* $ 14.07.2000 VVM ! Если закрыли подменю, то остаться. Иначе передать управление выше */
			MenuPos = ProcessSingleMenu((*CurrentMenuItem)->Menu, 0, MenuRoot, MenuFileName, concat(Title, L" \xbb "sv, CurrentLabel));

			if (MenuPos!=EC_CLOSE_LEVEL)
				return MenuPos;

			MenuPos = ExitCode;
			continue;
		}

		/* $ 01.05.2001 IS Отключим до лучших времен */
		//int LeftVisible,RightVisible,PanelsHidden=0;
		Global->CtrlObject->CmdLine()->LockUpdatePanel(true);

		// Цикл исполнения команд меню (CommandX)
		for (const auto& str: (*CurrentMenuItem)->Commands)
		{
			auto strCommand = str;

			if (!((starts_with_icase(strCommand, L"REM"sv) && (strCommand.size() == 3 || std::iswblank(strCommand[3]))) || starts_with_icase(strCommand, L"::"sv)))
			{
				/*
				  Осталось корректно обработать ситуацию, например:
				  if exist !#!\!^!.! far:edit < diff -c -p !#!\!^!.! !\!.!
				  Т.е. сначала "вычислить" кусок "if exist !#!\!^!.!", ну а если
				  выполнится, то делать дальше.
				  Или еще пример,
				  if exist ..\a.bat D:\FAR\170\DIFF.MY\mkdiff.bat !?&Номер патча?!
				  ЭТО выполняется всегда, т.к. парсинг всей строки идет, а надо
				  проверить фазу "if exist ..\a.bat", а уж потом делать выводы...
				*/
				//if(ExtractIfExistCommand(Command))
				{
					/* $ 01.05.2001 IS Отключим до лучших времен */
					/*
					if (!PanelsHidden)
					{
						LeftVisible=Global->CtrlObject->Cp()->LeftPanel()->IsVisible();
						RightVisible=Global->CtrlObject->Cp()->RightPanel()->IsVisible();
						Global->CtrlObject->Cp()->LeftPanel()->Hide();
						Global->CtrlObject->Cp()->RightPanel()->Hide();
						Global->CtrlObject->Cp()->LeftPanel()->SetUpdateMode(FALSE);
						Global->CtrlObject->Cp()->RightPanel()->SetUpdateMode(FALSE);
						PanelsHidden=TRUE;
					}
					*/

					bool PreserveLFN = false;
					if (SubstFileName(strCommand, Context, &PreserveLFN, false, CurrentLabel) && !strCommand.empty())
					{
						SCOPED_ACTION(PreserveLongName)(strName, PreserveLFN);

						execute_info Info;
						Info.DisplayCommand = strCommand;
						Info.Command = strCommand;

						Global->CtrlObject->CmdLine()->ExecString(Info);
					}
				}
			} // strCommand != "REM"
		}

		Global->CtrlObject->CmdLine()->LockUpdatePanel(false);

		/* $ 01.05.2001 IS Отключим до лучших времен */
		/*
		if (PanelsHidden)
		{
			Global->CtrlObject->Cp()->LeftPanel()->SetUpdateMode(TRUE);
			Global->CtrlObject->Cp()->RightPanel()->SetUpdateMode(TRUE);
			Global->CtrlObject->Cp()->LeftPanel()->Update(UPDATE_KEEP_SELECTION);
			Global->CtrlObject->Cp()->RightPanel()->Update(UPDATE_KEEP_SELECTION);
			if (RightVisible)
				Global->CtrlObject->Cp()->RightPanel()->Show();
			if (LeftVisible)
				Global->CtrlObject->Cp()->LeftPanel()->Show();
		}
		*/
		/* $ 14.07.2000 VVM ! Закрыть меню */
		/* $ 25.04.2001 DJ - сообщаем, что была выполнена команда (нужно перерисовать панели) */
		return EC_COMMAND_SELECTED;
	}
}

enum EditMenuItems
{
	EM_DOUBLEBOX,
	EM_HOTKEY_TEXT,
	EM_HOTKEY_EDIT,
	EM_LABEL_TEXT,
	EM_LABEL_EDIT,
	EM_SEPARATOR1,
	EM_COMMANDS_TEXT,
#ifdef PROJECT_DI_MEMOEDIT
	EM_MEMOEDIT,
#else
	EM_EDITLINE_0,
	EM_EDITLINE_1,
	EM_EDITLINE_2,
	EM_EDITLINE_3,
	EM_EDITLINE_4,
	EM_EDITLINE_5,
	EM_EDITLINE_6,
	EM_EDITLINE_7,
	EM_EDITLINE_8,
	EM_EDITLINE_9,
#endif
	EM_SEPARATOR2,
	EM_BUTTON_OK,
	EM_BUTTON_CANCEL,

	EM_COUNT
};

intptr_t UserMenu::EditMenuDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	switch (Msg)
	{
		case DN_EDITCHANGE:
#ifdef PROJECT_DI_MEMOEDIT
			if (Param1 == EM_MEMOEDIT)
#else
			if (Param1 >= EM_EDITLINE_0 && Param1 <= EM_EDITLINE_9)
#endif
				m_ItemChanged = true;
			break;

		case DN_CLOSE:

			if (Param1==EM_BUTTON_OK)
			{
				bool Result = true;
				const string_view HotKey = reinterpret_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, EM_HOTKEY_EDIT, nullptr));
				const string_view Label = reinterpret_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, EM_LABEL_EDIT, nullptr));
				int FocusPos=-1;

				if (HotKey != L"--"sv)
				{
					if (Label.empty())
					{
						FocusPos=EM_LABEL_EDIT;
					}
					else if (HotKey.size() > 1)
					{
						FocusPos=EM_HOTKEY_EDIT;

						if (upper(HotKey.front()) == L'F')
						{
							if (in_closed_range(1, from_string<int>(HotKey.substr(1)), 24))
								FocusPos=-1;
						}
					}
				}

				if (FocusPos!=-1)
				{
					Message(MSG_WARNING,
						msg(lng::MUserMenuTitle),
						{
							msg(Label.empty()? lng::MUserMenuInvalidInputLabel : lng::MUserMenuInvalidInputHotKey)
						},
						{ lng::MOk });
					Dlg->SendMessage(DM_SETFOCUS, FocusPos, nullptr);
					Result = false;
				}

				return Result;
			}
			else if (m_ItemChanged)
			{
				switch(Message(MSG_WARNING,
					msg(lng::MUserMenuTitle),
					{
						msg(lng::MEditMenuConfirmation)
					},
					{ lng::MHYes, lng::MHNo, lng::MHCancel }))
				{
				case Message::first_button:
					Dlg->SendMessage( DM_CLOSE, EM_BUTTON_OK, nullptr);
					break;

				case Message::second_button:
					return true;

				default:
					return false;
				}
			}

			break;
		default:
			break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}


bool UserMenu::EditMenu(std::list<UserMenuItem>& Menu, std::list<UserMenuItem>::iterator* MenuItem, bool Create)
{
	bool Result = false;
	bool SubMenu = false;
	bool Continue = true;
	m_ItemChanged = false;

	if (Create)
	{
		switch (Message(0,
			msg(lng::MUserMenuTitle),
			{
				msg(lng::MAskInsertMenuOrCommand)
			},
			{ lng::MMenuInsertCommand, lng::MMenuInsertMenu },
			{}, &AskInsertMenuOrCommandId))
		{
			case -1:
			case -2:
				Continue = false;
				[[fallthrough]];
			case Message::second_button:
				SubMenu = true;
		}
	}
	else
	{
		SubMenu = (*MenuItem)->Submenu;
	}

	if (Continue)
	{
		const int DLG_X=76, DLG_Y=SubMenu?10:22;
		const auto State = SubMenu? DIF_HIDDEN | DIF_DISABLE : DIF_NONE;

		auto EditDlg = MakeDialogItems<EM_COUNT>(
		{
			{ DI_DOUBLEBOX, {{3,  1      }, {DLG_X-4, DLG_Y-2}}, DIF_NONE, msg(SubMenu? lng::MEditSubmenuTitle : lng::MEditMenuTitle), },
			{ DI_TEXT,      {{5,  2      }, {0,       2      }}, DIF_NONE, msg(lng::MEditMenuHotKey), },
			{ DI_FIXEDIT,   {{5,  3      }, {7,       3      }}, DIF_FOCUS, },
			{ DI_TEXT,      {{5,  4      }, {0,       4      }}, DIF_NONE, msg(lng::MEditMenuLabel), },
			{ DI_EDIT,      {{5,  5      }, {DLG_X-6, 5      }}, DIF_NONE, },
			{ DI_TEXT,      {{-1, 6      }, {0,       6      }}, DIF_SEPARATOR | State, },
			{ DI_TEXT,      {{5,  7      }, {0,       7      }}, State, msg(lng::MEditMenuCommands), },
#ifdef PROJECT_DI_MEMOEDIT
			{ DI_MEMOEDIT,  {{5,  8      }, {DLG_X-6, 17     }}, DIF_EDITPATH, },
#else
			{ DI_EDIT,      {{5,  8      }, {DLG_X-6, 8      }}, DIF_EDITPATH | DIF_EDITPATHEXEC | DIF_EDITOR | State, },
			{ DI_EDIT,      {{5,  9      }, {DLG_X-6, 9      }}, DIF_EDITPATH | DIF_EDITPATHEXEC | DIF_EDITOR | State, },
			{ DI_EDIT,      {{5,  10     }, {DLG_X-6, 10     }}, DIF_EDITPATH | DIF_EDITPATHEXEC | DIF_EDITOR | State, },
			{ DI_EDIT,      {{5,  11     }, {DLG_X-6, 11     }}, DIF_EDITPATH | DIF_EDITPATHEXEC | DIF_EDITOR | State, },
			{ DI_EDIT,      {{5,  12     }, {DLG_X-6, 12     }}, DIF_EDITPATH | DIF_EDITPATHEXEC | DIF_EDITOR | State, },
			{ DI_EDIT,      {{5,  13     }, {DLG_X-6, 13     }}, DIF_EDITPATH | DIF_EDITPATHEXEC | DIF_EDITOR | State, },
			{ DI_EDIT,      {{5,  14     }, {DLG_X-6, 14     }}, DIF_EDITPATH | DIF_EDITPATHEXEC | DIF_EDITOR | State, },
			{ DI_EDIT,      {{5,  15     }, {DLG_X-6, 15     }}, DIF_EDITPATH | DIF_EDITPATHEXEC | DIF_EDITOR | State, },
			{ DI_EDIT,      {{5,  16     }, {DLG_X-6, 16     }}, DIF_EDITPATH | DIF_EDITPATHEXEC | DIF_EDITOR | State, },
			{ DI_EDIT,      {{5,  17     }, {DLG_X-6, 17     }}, DIF_EDITPATH | DIF_EDITPATHEXEC | DIF_EDITOR | State, },
#endif
			{ DI_TEXT,      {{-1, DLG_Y-4}, {0,       DLG_Y-4}}, DIF_SEPARATOR, },
			{ DI_BUTTON,    {{0,  DLG_Y-3}, {0,       DLG_Y-3}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MOk), },
			{ DI_BUTTON,    {{0,  DLG_Y-3}, {0,       DLG_Y-3}}, DIF_CENTERGROUP, msg(lng::MCancel), },
		});

#ifndef PROJECT_DI_MEMOEDIT
		enum {DI_EDIT_COUNT=EM_SEPARATOR2-EM_COMMANDS_TEXT-1};
#endif

		if (!Create)
		{
			EditDlg[EM_HOTKEY_EDIT].strData = (*MenuItem)->strHotKey;
			EditDlg[EM_LABEL_EDIT].strData = (*MenuItem)->strLabel;
#if defined(PROJECT_DI_MEMOEDIT)
			/*
				...
				здесь добавка строк из "Command%d" в EMR_MEMOEDIT
				...
			*/
			string strBuffer;
			for (string *str=MenuItem->Commands.First(); str && CommandNumber < DI_EDIT_COUNT; str=MenuItem->Commands.Next(str))
			{
				strBuffer+=*str;
				strBuffer+=L'\n';    //??? "\n\r"
			}

			EditDlg[EM_MEMOEDIT].strData = strBuffer; //???
#else
			int CommandNumber=0;
			for (const auto& i: (*MenuItem)->Commands)
			{
				EditDlg[EM_EDITLINE_0+CommandNumber].strData = i;
				if (++CommandNumber == DI_EDIT_COUNT)
					break;
			}
#endif
		}

		const auto Dlg = Dialog::create(EditDlg, &UserMenu::EditMenuDlgProc, this);
		Dlg->SetHelp(L"UserMenu"sv);
		Dlg->SetId(EditUserMenuId);
		Dlg->SetPosition({ -1, -1, DLG_X, DLG_Y });
		Dlg->Process();

		if (Dlg->GetExitCode()==EM_BUTTON_OK)
		{
			m_MenuModified=true;
			auto NewItemIterator = Menu.end();

			if (Create)
			{
				NewItemIterator = Menu.emplace(MenuItem? *MenuItem : Menu.begin(), UserMenuItem());
				MenuItem = &NewItemIterator;
			}

			(*MenuItem)->strHotKey = EditDlg[EM_HOTKEY_EDIT].strData;
			(*MenuItem)->strLabel = EditDlg[EM_LABEL_EDIT].strData;
			(*MenuItem)->Submenu = SubMenu;

			if (!SubMenu)
			{
#if defined(PROJECT_DI_MEMOEDIT)
				/*
				...
				здесь преобразование содержимого элемента EMR_MEMOEDIT в "Command%d"
				...
				*/
#else
				int CommandNumber=0;

				for (int i=0 ; i < DI_EDIT_COUNT ; i++)
					if (!EditDlg[i+EM_EDITLINE_0].strData.empty())
						CommandNumber=i+1;

				(*MenuItem)->Commands.clear();
				for (int i=0 ; i < DI_EDIT_COUNT ; i++)
				{
					if (i>=CommandNumber)
						break;
					else
						(*MenuItem)->Commands.emplace_back(EditDlg[i+EM_EDITLINE_0].strData);
				}
#endif
			}

			Result=true;
		}
	}

	return Result;
}

bool UserMenu::DeleteMenuRecord(std::list<UserMenuItem>& Menu, const std::list<UserMenuItem>::iterator& MenuItem) const
{
	if (Message(MSG_WARNING,
		msg(lng::MUserMenuTitle),
		{
			msg(!MenuItem->Submenu ? lng::MAskDeleteMenuItem : lng::MAskDeleteSubMenuItem),
			quote_unconditional(MenuItem->strLabel)
		},
		{ lng::MDelete, lng::MCancel }) != Message::first_button)
		return false;

	m_MenuModified=true;
	Menu.erase(MenuItem);
	return true;
}
