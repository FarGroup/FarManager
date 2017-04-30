/*
codepage_selection.cpp
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

#include "codepage_selection.hpp"
#include "encoding.hpp"
#include "vmenu2.hpp"
#include "keys.hpp"
#include "lang.hpp"
#include "dialog.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "configdb.hpp"
#include "FarDlgBuilder.hpp"
#include "DlgGuid.hpp"
#include "strmix.hpp"
#include "vmenu.hpp"

codepages& Codepages()
{
	static codepages cp;
	return cp;
}

// Ключ где хранятся имена кодовых страниц
static const wchar_t NamesOfCodePagesKey[] = L"CodePages.Names";

// Источник вызова каллбака прохода по кодовым страницам
enum CodePagesCallbackCallSource: int
{
	CodePageSelect,
	CodePagesFill,
	CodePagesFill2,
	CodePageCheck
};

// Стандартные элементы меню кодовых страниц
enum StandardCodePagesMenuItems
{
	SearchAll = bit(0), // Find-in-Files dialog
	AutoCP    = bit(1), // show <Autodetect> item
	OEM       = bit(2), // show OEM codepage
	ANSI      = bit(3), // show ANSI codepage
	UTF8      = bit(4), // show UTF-8 codepage
	UTF16LE   = bit(5), // show UTF-16 LE codepage
	UTF16BE   = bit(6), // show UTF-16 BE codepage
	VOnly     = bit(7), // show only viewer-supported codepages
	DefaultCP = bit(8), // show <Default> item

	AllStandard = OEM | ANSI | UTF8 | UTF16BE | UTF16LE
};

codepages::codepages():
	dialog(nullptr),
	control(0),
	DialogBuilderList(),
	currentCodePage(0),
	favoriteCodePages(0),
	normalCodePages(0),
	selectedCodePages(false),
	CallbackCallSource(CodePageSelect)
{
}

codepages::~codepages() = default;

// Получаем кодовую страницу для элемента в меню
uintptr_t codepages::GetMenuItemCodePage(size_t Position) const
{
	const auto DataPtr = CodePagesMenu->GetUserDataPtr<uintptr_t>(Position);
	return DataPtr? *DataPtr : 0;
}

const string& codepages::FavoriteCodePagesKey()
{
	static string sFavourites = L"CodePages.Favorites"s;
	return sFavourites;
}

size_t codepages::GetListItemCodePage(size_t Position) const
{
	const auto DataPtr = dialog->GetListItemDataPtr<uintptr_t>(control, Position);
	return DataPtr? *DataPtr : 0;
}

// Проверяем попадает или нет позиция в диапазон стандартных кодовых страниц (правильность работы для разделителей не гарантируется)
bool codepages::IsPositionStandard(UINT position) const
{
	return position <= (UINT)CodePagesMenu->size() - favoriteCodePages - (favoriteCodePages?1:0) - normalCodePages - (normalCodePages?1:0);
}

// Проверяем попадает или нет позиция в диапазон избранных кодовых страниц (правильность работы для разделителей не гарантируется)
bool codepages::IsPositionFavorite(UINT position) const
{
	return !IsPositionStandard(position) && !IsPositionNormal(position);
}

// Проверяем попадает или нет позиция в диапазон обыкновенных кодовых страниц (правильность работы для разделителей не гарантируется)
bool codepages::IsPositionNormal(UINT position) const
{
	return position >= static_cast<UINT>(CodePagesMenu->size() - normalCodePages);
}

// Формируем строку для визуального представления таблицы символов
string codepages::FormatCodePageString(uintptr_t CodePage, const string& CodePageName, bool IsCodePageNameCustom) const
{
	string result;
	if (static_cast<intptr_t>(CodePage) >= 0)  // CodePage != CP_DEFAULT, CP_REDETECT
	{
		result = str(CodePage);
		result.resize(std::max(result.size(), size_t(5)), L' ');
		result += BoxSymbols[BS_V1];
		result += (!IsCodePageNameCustom || CallbackCallSource == CodePagesFill || CallbackCallSource == CodePagesFill2? L' ' : L'*');
	}
	result += CodePageName;
	return result;
}

// Добавляем таблицу символов
void codepages::AddCodePage(const string& codePageName, uintptr_t codePage, size_t position, bool enabled, bool checked, bool IsCodePageNameCustom) const
{
	if (CallbackCallSource == CodePagesFill)
	{
		// Вычисляем позицию вставляемого элемента
		if (position == size_t(-1))
		{
			FarListInfo info = { sizeof(FarListInfo) };
			dialog->SendMessage(DM_LISTINFO, control, &info);
			position = info.ItemsNumber;
		}

		// Вставляем элемент
		FarListInsert item = { sizeof(FarListInsert), static_cast<intptr_t>(position) };

		string name = FormatCodePageString(codePage, codePageName, IsCodePageNameCustom);
		item.Item.Text = name.data();

		if (selectedCodePages && checked)
		{
			item.Item.Flags |= LIF_CHECKED;
		}

		if (!enabled)
		{
			item.Item.Flags |= LIF_GRAYED;
		}

		dialog->SendMessage(DM_LISTINSERT, control, &item);
		dialog->SetListItemData(control, position, codePage);
	}
	else if (CallbackCallSource == CodePagesFill2)
	{
		// Вставляем элемент
		FarDialogBuilderListItem2 item;

		item.Text = FormatCodePageString(codePage, codePageName, IsCodePageNameCustom);
		item.Flags = LIF_NONE;

		if (selectedCodePages && checked)
		{
			item.Flags |= LIF_CHECKED;
		}

		if (!enabled)
		{
			item.Flags |= LIF_GRAYED;
		}

		item.ItemValue = static_cast<int>(codePage);

		// Вычисляем позицию вставляемого элемента
		if (position == size_t(-1) || position >= DialogBuilderList->size())
		{
			DialogBuilderList->emplace_back(item);
		}
		else
		{
			DialogBuilderList->emplace(DialogBuilderList->begin() + position, item);
		}
	}
	else
	{
		// Создаём новый элемент меню
		MenuItemEx item(FormatCodePageString(codePage, codePageName, IsCodePageNameCustom));
		if (!enabled)
			item.Flags |= MIF_GRAYED;
		item.UserData = codePage;

		// Добавляем новый элемент в меню
		if (position == size_t(-1))
			CodePagesMenu->AddItem(item);
		else
			CodePagesMenu->AddItem(item, static_cast<int>(position));

		// Если надо позиционируем курсор на добавленный элемент
		if (currentCodePage == codePage)
		{
			if ((CodePagesMenu->GetSelectPos() == -1 || GetMenuItemCodePage() != codePage))
			{
				CodePagesMenu->SetSelectPos(static_cast<int>(position == size_t(-1)? CodePagesMenu->size() - 1 : position), 1);
			}
		}
	}
}

// Добавляем стандартную таблицу символов
void codepages::AddStandardCodePage(const string& codePageName, uintptr_t codePage, int position, bool enabled) const
{
	bool checked = false;

	if (selectedCodePages && codePage != CP_DEFAULT)
	{
		if (GetFavorite(codePage) & CPST_FIND)
			checked = true;
	}

	AddCodePage(codePageName, codePage, position, enabled, checked, false);
}

// Добавляем разделитель
void codepages::AddSeparator(const string& Label, size_t position) const
{
	if (CallbackCallSource == CodePagesFill)
	{
		if (position == size_t(-1))
		{
			FarListInfo info = { sizeof(FarListInfo) };
			dialog->SendMessage(DM_LISTINFO, control, &info);
			position = info.ItemsNumber;
		}

		FarListInsert item = { sizeof(FarListInsert), static_cast<intptr_t>(position) };
		item.Item.Text = Label.data();
		item.Item.Flags = LIF_SEPARATOR;
		dialog->SendMessage(DM_LISTINSERT, control, &item);
	}
	else if (CallbackCallSource == CodePagesFill2)
	{
		// Вставляем элемент
		FarDialogBuilderListItem2 item;
		item.Text = Label;
		item.Flags = LIF_SEPARATOR;
		item.ItemValue = 0;

		// Вычисляем позицию вставляемого элемента
		if (position == size_t(-1) || position >= DialogBuilderList->size())
		{
			DialogBuilderList->emplace_back(item);
		}
		else
		{
			DialogBuilderList->emplace(DialogBuilderList->begin() + position, item);
		}
	}
	else
	{
		MenuItemEx item;
		item.strName = Label;
		item.Flags = MIF_SEPARATOR;

		if (position == size_t(-1))
			CodePagesMenu->AddItem(item);
		else
			CodePagesMenu->AddItem(item, static_cast<int>(position));
	}
}

// Получаем количество элементов в списке
size_t codepages::size() const
{
	if (CallbackCallSource == CodePageSelect)
	{
		return CodePagesMenu->size();
	}
	else if (CallbackCallSource == CodePagesFill2)
	{
		return DialogBuilderList->size();
	}
	else
	{
		FarListInfo info = { sizeof(FarListInfo) };
		dialog->SendMessage(DM_LISTINFO, control, &info);
		return info.ItemsNumber;
	}
}

// Получаем позицию для вставки таблицы с учётом сортировки по номеру кодовой страницы
size_t codepages::GetCodePageInsertPosition(uintptr_t codePage, size_t start, size_t length)
{
	const auto& GetCodePage = [this](size_t position) -> uintptr_t
	{
		switch (CallbackCallSource)
		{
		case CodePageSelect: return GetMenuItemCodePage(position);
		case CodePagesFill2: return (*DialogBuilderList)[position].ItemValue;
		default: return GetListItemCodePage(position);
		}
	};

	const auto iRange = make_irange(start, start + length);
	return *std::find_if(CONST_RANGE(iRange, i) { return GetCodePage(i) >= codePage; });
}

// Добавляем все необходимые таблицы символов
void codepages::AddCodePages(DWORD codePages)
{
	// default & re-detect
	//
	uintptr_t cp_auto = CP_DEFAULT;
	if (0 != (codePages & ::DefaultCP))
	{
		AddStandardCodePage(msg(lng::MDefaultCP), CP_DEFAULT, -1, true);
		cp_auto = CP_REDETECT;
	}

	AddStandardCodePage(msg(lng::MEditOpenAutoDetect), cp_auto, -1, (codePages & AutoCP) != 0);

	if (codePages & SearchAll)
		AddStandardCodePage(msg(lng::MFindFileAllCodePages), CP_SET, -1, true);

	// system codepages
	//
	AddSeparator(msg(lng::MGetCodePageSystem));
	AddStandardCodePage(L"ANSI", GetACP(), -1, (codePages & ::ANSI) != 0);

	if (GetOEMCP() != GetACP())
		AddStandardCodePage(L"OEM", GetOEMCP(), -1, (codePages & ::OEM) != 0);

	// unicode codepages
	//
	AddSeparator(msg(lng::MGetCodePageUnicode));
	AddStandardCodePage(L"UTF-8", CP_UTF8, -1, (codePages & ::UTF8) != 0);
	AddStandardCodePage(L"UTF-16 (Little endian)", CP_UNICODE, -1, (codePages & ::UTF16LE) != 0);
	AddStandardCodePage(L"UTF-16 (Big endian)", CP_REVERSEBOM, -1, (codePages & ::UTF16BE) != 0);

	// other codepages
	//
	for (const auto& i: InstalledCodepages())
	{
		UINT cp = i.first;
		if (IsStandardCodePage(cp))
			continue;

		string CodepageName;
		UINT len = 0;
		std::tie(len, CodepageName) = GetCodePageInfo(cp);
		if (!len || (len > 2 && (codePages & ::VOnly)))
			continue;

		bool IsCodePageNameCustom = false;
		FormatCodePageName(cp, CodepageName, IsCodePageNameCustom);

		long long selectType = GetFavorite(cp);

		// Добавляем таблицу символов либо в нормальные, либо в выбранные таблицы символов
		if (selectType & CPST_FAVORITE)
		{
			// Если надо добавляем разделитель между выбранными и нормальными таблицами символов
			if (!favoriteCodePages)
				AddSeparator(msg(lng::MGetCodePageFavorites), size() - normalCodePages - (normalCodePages?1:0));

			// Добавляем таблицу символов в выбранные
			AddCodePage(
				CodepageName, cp,
				GetCodePageInsertPosition(
				cp, size() - normalCodePages - favoriteCodePages - (normalCodePages?1:0), favoriteCodePages
				),
				true, (selectType & CPST_FIND) != 0, IsCodePageNameCustom
				);
			// Увеличиваем счётчик выбранных таблиц символов
			favoriteCodePages++;
		}
		else if (CallbackCallSource == CodePagesFill || CallbackCallSource == CodePagesFill2 || !Global->Opt->CPMenuMode)
		{
			// добавляем разделитель между стандартными и системными таблицами символов
			if (!normalCodePages)
				AddSeparator(msg(lng::MGetCodePageOther));

			// Добавляем таблицу символов в нормальные
			AddCodePage(
				CodepageName, cp,
				GetCodePageInsertPosition(cp, size() - normalCodePages, normalCodePages),
				true, (selectType & CPST_FIND) != 0, IsCodePageNameCustom
				);
			// Увеличиваем счётчик выбранных таблиц символов
			normalCodePages++;
		}
	}
}

// Обработка добавления/удаления в/из список выбранных таблиц символов
void codepages::SetFavorite(bool State)
{
	if (Global->Opt->CPMenuMode && State)
		return;

	UINT itemPosition = CodePagesMenu->GetSelectPos();
	uintptr_t codePage = GetMenuItemCodePage();

	if ((State && IsPositionNormal(itemPosition)) || (!State && IsPositionFavorite(itemPosition)))
	{
		// Получаем текущее состояние флага в реестре
		long long selectType = GetFavorite(codePage);

		// Удаляем/добавляем в реестре информацию о выбранной кодовой странице
		if (State)
			SetFavorite(codePage, CPST_FAVORITE | (selectType & CPST_FIND ? CPST_FIND : 0));
		else if (selectType & CPST_FIND)
			SetFavorite(codePage, CPST_FIND);
		else
			DeleteFavorite(codePage);

		// Создаём новый элемент меню
		MenuItemEx newItem(CodePagesMenu->current().strName);
		newItem.UserData = codePage;
		// Сохраняем позицию курсора
		size_t position = CodePagesMenu->GetSelectPos();
		// Удаляем старый пункт меню
		CodePagesMenu->DeleteItem(CodePagesMenu->GetSelectPos());

		// Добавляем пункт меню в новое место
		if (State)
		{
			// Добавляем разделитель, если выбранных кодовых страниц ещё не было
			// и после добавления останутся нормальные кодовые страницы
			if (!favoriteCodePages && normalCodePages>1)
				AddSeparator(msg(lng::MGetCodePageFavorites), CodePagesMenu->size() - normalCodePages);

			// Ищем позицию, куда добавить элемент
			const auto newPosition = GetCodePageInsertPosition(
				codePage,
				CodePagesMenu->size() - normalCodePages - favoriteCodePages,
				favoriteCodePages
				);
			// Добавляем кодовою страницу в выбранные
			CodePagesMenu->AddItem(newItem, static_cast<int>(newPosition));

			// Удаляем разделитель, если нет обыкновенных кодовых страниц
			if (normalCodePages == 1)
				CodePagesMenu->DeleteItem(static_cast<int>(CodePagesMenu->size() - 1));

			// Изменяем счётчики нормальных и выбранных кодовых страниц
			favoriteCodePages++;
			normalCodePages--;
			position++;
		}
		else
		{
			// Удаляем разделитель, если после удаления не останется ни одной
			// выбранной таблицы символов
			if (favoriteCodePages == 1 && normalCodePages>0)
				CodePagesMenu->DeleteItem(static_cast<int>(CodePagesMenu->size() - normalCodePages - 2));

			// Переносим элемент в нормальные таблицы, только если они показываются
			if (!Global->Opt->CPMenuMode)
			{
				// Добавляем разделитель, если не было ни одной нормальной кодовой страницы
				if (!normalCodePages)
					AddSeparator(msg(lng::MGetCodePageOther));

				// Добавляем кодовою страницу в нормальные
				CodePagesMenu->AddItem(
					newItem,
					static_cast<int>(GetCodePageInsertPosition(
					codePage,
					CodePagesMenu->size() - normalCodePages,
					normalCodePages
					))
					);
				normalCodePages++;
			}
			// Если в режиме скрытия нормальных таблиц мы удалили последнюю выбранную таблицу, то удаляем и разделитель
			else if (favoriteCodePages == 1)
				CodePagesMenu->DeleteItem(static_cast<int>(CodePagesMenu->size() - normalCodePages - 1));

			favoriteCodePages--;

			if (position == CodePagesMenu->size() - normalCodePages - 1)
				position--;
		}

		// Устанавливаем позицию в меню
		CodePagesMenu->SetSelectPos(static_cast<int>(position >= CodePagesMenu->size()? CodePagesMenu->size() - 1 : position), 1);

		// Показываем меню
		if (Global->Opt->CPMenuMode)
			CodePagesMenu->SetPosition(-1, -1, 0, 0);
	}
}

// Заполняем меню выбора таблиц символов
void codepages::FillCodePagesVMenu(bool bShowUnicode, bool bViewOnly, bool bShowAutoDetect)
{
	uintptr_t codePage = currentCodePage;

	if (CodePagesMenu->GetSelectPos() != -1 && static_cast<size_t>(CodePagesMenu->GetSelectPos()) < CodePagesMenu->size() - normalCodePages)
		currentCodePage = GetMenuItemCodePage();

	// Очищаем меню
	favoriteCodePages = normalCodePages = 0;
	CodePagesMenu->clear();

	string title = msg(lng::MGetCodePageTitle);
	if (Global->Opt->CPMenuMode)
		title += L" *";
	CodePagesMenu->SetTitle(title);

	// Добавляем таблицы символов
	AddCodePages(::OEM | ::ANSI | ::UTF8
		| (bShowUnicode ? (::UTF16BE | ::UTF16LE) : 0)
		| (bViewOnly ? ::VOnly : 0)
		| (bShowAutoDetect ? ::AutoCP : 0)
		);
	// Восстанавливаем оригинальную таблицу символов
	currentCodePage = codePage;
	// Позиционируем меню
	CodePagesMenu->SetPosition(-1, -1, 0, 0);
	// Показываем меню
}

// Форматируем имя таблицы символов
string& codepages::FormatCodePageName(uintptr_t CodePage, string& CodePageName) const
{
	bool IsCodePageNameCustom;
	return FormatCodePageName(CodePage, CodePageName, IsCodePageNameCustom);
}

// Форматируем имя таблицы символов
string& codepages::FormatCodePageName(uintptr_t CodePage, string& CodePageName, bool &IsCodePageNameCustom) const
{
	string strCodePage = str(CodePage);
	string CurrentCodePageName;

	// Пытаемся получить заданное пользователем имя таблицы символов
	if (ConfigProvider().GeneralCfg()->GetValue(NamesOfCodePagesKey, strCodePage, CurrentCodePageName, L""))
	{
		IsCodePageNameCustom = true;
		if (CurrentCodePageName == CodePageName)
		{
			ConfigProvider().GeneralCfg()->DeleteValue(NamesOfCodePagesKey, strCodePage);
			IsCodePageNameCustom = false;
		}
		else
		{
			CodePageName = CurrentCodePageName;
		}
	}

	return CodePageName;
}

// Номера контролов диалога редактирования имени кодовой страницы
enum EditCodePagesDialogControls
{
	EDITCP_BORDER,
	EDITCP_EDIT,
	EDITCP_SEPARATOR,
	EDITCP_OK,
	EDITCP_CANCEL,
	EDITCP_RESET,
};

// Каллбак для диалога редактирования имени кодовой страницы
intptr_t codepages::EditDialogProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	if (Msg == DN_CLOSE)
	{
		if (Param1 == EDITCP_OK || Param1 == EDITCP_RESET)
		{
			string strCodePageName;
			uintptr_t CodePage = GetMenuItemCodePage();
			string strCodePage = str(CodePage);

			if (Param1 == EDITCP_OK)
			{
				strCodePageName = reinterpret_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, EDITCP_EDIT, nullptr));
			}
			// Если имя кодовой страницы пустое, то считаем, что имя не задано
			if (strCodePageName.empty())
				ConfigProvider().GeneralCfg()->DeleteValue(NamesOfCodePagesKey, strCodePage);
			else
				ConfigProvider().GeneralCfg()->SetValue(NamesOfCodePagesKey, strCodePage, strCodePageName);
			// Получаем информацию о кодовой странице
			string CodepageName;
			UINT len = 0;
			std::tie(len, CodepageName) = GetCodePageInfo(static_cast<UINT>(CodePage));
			if (len)
			{
				// Формируем имя таблиц символов
				bool IsCodePageNameCustom = false;
				FormatCodePageName(CodePage, CodepageName, IsCodePageNameCustom);
				// Обновляем имя кодовой страницы
				int Position = CodePagesMenu->GetSelectPos();
				CodePagesMenu->DeleteItem(Position);
				MenuItemEx NewItem(FormatCodePageString(CodePage, CodepageName, IsCodePageNameCustom));
				NewItem.UserData = CodePage;
				CodePagesMenu->AddItem(NewItem, Position);
				CodePagesMenu->SetSelectPos(Position, 1);
			}
		}
	}
	return Dlg->DefProc(Msg, Param1, Param2);
}

// Вызов редактора имени кодовой страницы
void codepages::EditCodePageName()
{
	UINT Position = CodePagesMenu->GetSelectPos();
	if (IsPositionStandard(Position))
		return;
	string CodePageName = CodePagesMenu->at(Position).strName;
	size_t BoxPosition = CodePageName.find(BoxSymbols[BS_V1]);
	if (BoxPosition == string::npos)
		return;
	CodePageName.erase(0, BoxPosition + 2);
	FarDialogItem EditDialogData[] =
	{
		{ DI_DOUBLEBOX, 3, 1, 50, 5, 0, nullptr, nullptr, 0, msg(lng::MGetCodePageEditCodePageName).data() },
		{ DI_EDIT, 5, 2, 48, 2, 0, L"CodePageName", nullptr, DIF_FOCUS | DIF_HISTORY, CodePageName.data() },
		{ DI_TEXT, -1, 3, 0, 3, 0, nullptr, nullptr, DIF_SEPARATOR, L"" },
		{ DI_BUTTON, 0, 4, 0, 3, 0, nullptr, nullptr, DIF_DEFAULTBUTTON | DIF_CENTERGROUP, msg(lng::MOk).data() },
		{ DI_BUTTON, 0, 4, 0, 3, 0, nullptr, nullptr, DIF_CENTERGROUP, msg(lng::MCancel).data() },
		{ DI_BUTTON, 0, 4, 0, 3, 0, nullptr, nullptr, DIF_CENTERGROUP, msg(lng::MGetCodePageResetCodePageName).data() }
	};
	auto EditDialog = MakeDialogItemsEx(EditDialogData);
	const auto Dlg = Dialog::create(EditDialog, &codepages::EditDialogProc, this);
	Dlg->SetPosition(-1, -1, 54, 7);
	Dlg->SetHelp(L"EditCodePageNameDlg");
	Dlg->Process();
}

bool codepages::SelectCodePage(uintptr_t& CodePage, bool bShowUnicode, bool bViewOnly, bool bShowAutoDetect)
{
	bool Result = false;
	CallbackCallSource = CodePageSelect;
	currentCodePage = CodePage;
	// Создаём меню
	CodePagesMenu = VMenu2::create(L"", nullptr, 0, ScrY - 4);
	CodePagesMenu->SetBottomTitle(msg(!Global->Opt->CPMenuMode? lng::MGetCodePageBottomTitle : lng::MGetCodePageBottomShortTitle));
	CodePagesMenu->SetMenuFlags(VMENU_WRAPMODE | VMENU_AUTOHIGHLIGHT);
	CodePagesMenu->SetHelp(L"CodePagesMenu");
	CodePagesMenu->SetId(CodePagesMenuId);
	// Добавляем таблицы символов
	FillCodePagesVMenu(bShowUnicode, bViewOnly, bShowAutoDetect);
	// Показываем меню

	// Цикл обработки сообщений меню
	intptr_t r = CodePagesMenu->Run([&](const Manager::Key& RawKey)
	{
		const auto ReadKey = RawKey();
		int KeyProcessed = 1;
		switch (ReadKey)
		{
			// Обработка скрытия/показа системных таблиц символов
		case KEY_CTRLH:
		case KEY_RCTRLH:
			Global->Opt->CPMenuMode = !Global->Opt->CPMenuMode;
			CodePagesMenu->SetBottomTitle(msg(Global->Opt->CPMenuMode? lng::MGetCodePageBottomShortTitle : lng::MGetCodePageBottomTitle));
			FillCodePagesVMenu(bShowUnicode, bViewOnly, bShowAutoDetect);
			break;
			// Обработка удаления таблицы символов из списка выбранных
		case KEY_DEL:
		case KEY_NUMDEL:
			SetFavorite(false);
			break;
			// Обработка добавления таблицы символов в список выбранных
		case KEY_INS:
		case KEY_NUMPAD0:
			SetFavorite(true);
			break;
			// Редактируем имя таблицы символов
		case KEY_F4:
			EditCodePageName();
			break;
		default:
			KeyProcessed = 0;
		}
		return KeyProcessed;
	});

	// Получаем выбранную таблицу символов
	if (r >= 0)
	{
		CodePage = GetMenuItemCodePage();
		Result = true;
	}
	CodePagesMenu.reset();
	return Result;
}

// Заполняем список таблицами символов
void codepages::FillCodePagesList(std::vector<FarDialogBuilderListItem2> &List, bool allowAuto, bool allowAll, bool allowDefault, bool allowChecked, bool bViewOnly)
{
	CallbackCallSource = CodePagesFill2;
	// Устанавливаем переменные для доступа из каллбака
	DialogBuilderList = &List;
	favoriteCodePages = normalCodePages = 0;
	selectedCodePages = allowChecked;
	// Добавляем стандартные элементы в список
	AddCodePages
		((allowDefault ? ::DefaultCP : 0)
		| (allowAuto ? ::AutoCP : 0)
		| (allowAll ? ::SearchAll : 0)
		| (bViewOnly ? ::VOnly : 0)
		| ::AllStandard
		);
	DialogBuilderList = nullptr;
}


// Заполняем список таблицами символов
UINT codepages::FillCodePagesList(Dialog* Dlg, UINT controlId, uintptr_t codePage, bool allowAuto, bool allowAll, bool allowDefault, bool allowChecked, bool bViewOnly)
{
	CallbackCallSource = CodePagesFill;
	// Устанавливаем переменные для доступа из каллбака
	dialog = Dlg;
	control = controlId;
	currentCodePage = codePage;
	favoriteCodePages = normalCodePages = 0;
	selectedCodePages = allowChecked;
	// Добавляем стандартные элементы в список
	AddCodePages
		((allowDefault ? ::DefaultCP : 0)
		| (allowAuto ? ::AutoCP : 0)
		| (allowAll ? ::SearchAll : 0)
		| (bViewOnly ? ::VOnly : 0)
		| ::AllStandard
		);

	if (CallbackCallSource == CodePagesFill)
	{
		// Если надо выбираем элемент
		FarListInfo info = { sizeof(FarListInfo) };
		Dlg->SendMessage(DM_LISTINFO, control, &info);

		for (size_t i = 0; i != info.ItemsNumber; ++i)
		{
			if (GetListItemCodePage(i) == codePage)
			{
				FarListGetItem Item = { sizeof(FarListGetItem), static_cast<intptr_t>(i) };
				dialog->SendMessage(DM_LISTGETITEM, control, &Item);
				dialog->SendMessage(DM_SETTEXTPTR, control, const_cast<wchar_t*>(Item.Item.Text));
				FarListPos Pos = { sizeof(FarListPos), static_cast<intptr_t>(i), -1 };
				dialog->SendMessage(DM_LISTSETCURPOS, control, &Pos);
				break;
			}
		}
	}

	// Возвращаем число избранных таблиц символов
	return favoriteCodePages;
}

bool codepages::IsCodePageSupported(uintptr_t CodePage, size_t MaxCharSize)
{
	if (CodePage == CP_DEFAULT || IsStandardCodePage(CodePage))
		return true;

	const auto CharSize = GetCodePageInfo(static_cast<UINT>(CodePage)).first;
	return CharSize != 0 && CharSize <= MaxCharSize;
}

long long codepages::GetFavorite(uintptr_t cp)
{
	long long value = 0;
	ConfigProvider().GeneralCfg()->GetValue(FavoriteCodePagesKey(), str(cp), value, 0);
	return value;
}

void codepages::SetFavorite(uintptr_t cp, long long value)
{
	ConfigProvider().GeneralCfg()->SetValue(FavoriteCodePagesKey(), str(cp), value);
}

void codepages::DeleteFavorite(uintptr_t cp)
{
	ConfigProvider().GeneralCfg()->DeleteValue(FavoriteCodePagesKey(), str(cp));
}


//################################################################################################

F8CP::F8CP(bool viewer):
	m_AcpName(msg(Global->OnlyEditorViewerUsed? (viewer? lng::MSingleViewF8 : lng::MSingleEditF8) : (viewer? lng::MViewF8 : lng::MEditF8))),
	m_OemName(msg(Global->OnlyEditorViewerUsed? (viewer? lng::MSingleViewF8DOS : lng::MSingleEditF8DOS) : (viewer? lng::MViewF8DOS : lng::MEditF8DOS))),
	m_UtfName(L"UTF-8")
{
	uintptr_t defcp = viewer ? Global->Opt->ViOpt.DefaultCodePage : Global->Opt->EdOpt.DefaultCodePage;

	string cps(viewer ? Global->Opt->ViOpt.strF8CPs : Global->Opt->EdOpt.strF8CPs);
	if (cps != L"-1")
	{
		std::unordered_set<uintptr_t> used_cps;
		for(const auto& str_cp: split<std::vector<string>>(cps, 0))
		{
			auto s = upper_copy(str_cp);
			uintptr_t cp = 0;
			if (s == L"ANSI" || s == L"ACP" || s == L"WIN")
				cp = GetACP();
			else if (s == L"OEM" || s == L"OEMCP" || s == L"DOS")
				cp = GetOEMCP();
			else if (s == L"UTF8" || s == L"UTF-8")
				cp = CP_UTF8;
			else if (s == L"DEFAULT")
				cp = defcp;
			else
			{
				try
				{
					cp = std::stoul(s);
				}
				catch (const std::exception&)
				{
					// TODO: log
					cp = 0;
				}
			}
			if (cp && codepages::IsCodePageSupported(cp, viewer ? 2:20) && !contains(used_cps, cp))
			{
				m_F8CpOrder.emplace_back(cp);
				used_cps.emplace(cp);
			}
		}
	}
	if (m_F8CpOrder.empty())
	{
		const uintptr_t acp = GetACP(), oemcp = GetOEMCP();
		if (cps != L"-1")
			defcp = acp;
		m_F8CpOrder.emplace_back(defcp);
		if (acp != defcp)
			m_F8CpOrder.emplace_back(acp);
		if (oemcp != defcp && oemcp != acp)
			m_F8CpOrder.emplace_back(oemcp);
	}
}

uintptr_t F8CP::NextCP(uintptr_t cp) const
{
	auto curr = std::find(ALL_CONST_RANGE(m_F8CpOrder), cp);
	return curr != m_F8CpOrder.cend() && ++curr != m_F8CpOrder.cend()? *curr : *m_F8CpOrder.cbegin();
}

const string& F8CP::NextCPname(uintptr_t cp) const
{
	const auto next_cp = NextCP(cp);
	if (next_cp == GetACP())
		return m_AcpName;

	if (next_cp == GetOEMCP())
		return m_OemName;

	if (next_cp == CP_UTF8)
		return m_UtfName;

	return m_Number = str(next_cp);
}
