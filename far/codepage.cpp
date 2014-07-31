/*
codepage.cpp

Работа с кодовыми страницами
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

#include "codepage.hpp"
#include "vmenu2.hpp"
#include "keys.hpp"
#include "language.hpp"
#include "dialog.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "configdb.hpp"
#include "FarDlgBuilder.hpp"
#include "DlgGuid.hpp"

codepages& Codepages()
{
	static codepages cp;
	return cp;
}

// Ключ где хранятся имена кодовых страниц
const wchar_t *NamesOfCodePagesKey = L"CodePages.Names";
const wchar_t *FavoriteCodePagesKey = L"CodePages.Favorites";

// Источник вызова каллбака прохода по кодовым страницам
ENUM(codepages::CodePagesCallbackCallSource)
{
	CodePageSelect,
	CodePagesFill,
	CodePagesFill2,
	CodePageCheck
};

// Стандартные элементы меню кодовых страниц
enum StandardCodePagesMenuItems
{
	SearchAll   = 0x001,  // Find-in-Files dialog
	AutoCP      = 0x002,  // show <Autodetect> item
	OEM         = 0x004,  // show OEM codepage
	ANSI        = 0x008,  // show ANSI codepage
	UTF8        = 0x010,  // show UTF-8 codepage
	UTF16LE     = 0x020,  // show UTF-16 LE codepage
	UTF16BE     = 0x040,  // show UTF-16 BE codepage
	VOnly       = 0x080,  // show only viewer-supported codepages
	DefaultCP   = 0x100, // show <Default> item

	AllStandard = OEM | ANSI | UTF8 | UTF16BE | UTF16LE
};

class system_codepages_enumerator
{
public:
	const static codepages::codepages_data* context;

	static BOOL CALLBACK enum_cp(wchar_t *cpNum)
	{
		auto cp = static_cast<UINT>(_wtoi(cpNum));
		if (cp == CP_UTF8)
			return TRUE; // skip standard unicode

		CPINFOEX cpix;
		if (!GetCPInfoEx(cp, 0, &cpix))
		{
			CPINFO cpi;
			if (!GetCPInfo(cp, &cpi))
				return TRUE;

			cpix.MaxCharSize = cpi.MaxCharSize;
			wcscpy(cpix.CodePageName, cpNum);
		}
		if (cpix.MaxCharSize > 0)
		{
			string cp_data(cpix.CodePageName);
			size_t pos = cp_data.find(L"(");
			// Windows: "XXXX (Name)", Wine: "Name"
			if (pos != string::npos)
			{
				cp_data = cp_data.substr(pos + 1);
				pos = cp_data.find(L")");
				if (pos != string::npos)
					cp_data = cp_data.substr(0, pos);
			}
			context->installed_cp[cp] = std::make_pair(cpix.MaxCharSize, cp_data);
		}

		return TRUE;

	}
};

const codepages::codepages_data* system_codepages_enumerator::context;

const codepages::codepages_data::cp_map& codepages::codepages_data::get() const
{
	if (installed_cp.empty())
	{
		system_codepages_enumerator::context = this;
		EnumSystemCodePages(system_codepages_enumerator::enum_cp, CP_INSTALLED);
		system_codepages_enumerator::context = nullptr;
	}
	return installed_cp;
}


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

codepages::~codepages()
{}

std::pair<UINT, string> codepages::GetCodePageInfo(UINT cp) const
{
	// Standard unicode CPs (1200, 1201, 65001) are NOT in the list.
	auto found = data.get().find(cp);
	if (data.get().end() == found)
		return std::pair<UINT, string>();

	return found->second;
}

// Получаем кодовую страницу для элемента в меню
inline uintptr_t codepages::GetMenuItemCodePage(int Position)
{
	void* Data = CodePagesMenu->GetUserData(nullptr, 0, Position);
	return Data? *static_cast<uintptr_t*>(Data) : 0;
}

inline uintptr_t codepages::GetListItemCodePage(int Position)
{
	intptr_t Data = dialog->SendMessage(DM_LISTGETDATA, control, ToPtr(Position));
	return Data? *reinterpret_cast<uintptr_t*>(Data) : 0;
}

// Проверяем попадает или нет позиция в диапазон стандартных кодовых страниц (правильность работы для разделителей не гарантируется)
inline bool codepages::IsPositionStandard(UINT position)
{
	return position<=(UINT)CodePagesMenu->GetItemCount()-favoriteCodePages-(favoriteCodePages?1:0)-normalCodePages-(normalCodePages?1:0);
}

// Проверяем попадает или нет позиция в диапазон избранных кодовых страниц (правильность работы для разделителей не гарантируется)
inline bool codepages::IsPositionFavorite(UINT position)
{
	return favoriteCodePages && position < (UINT)CodePagesMenu->GetItemCount() - normalCodePages;
}

// Проверяем попадает или нет позиция в диапазон обыкновенных кодовых страниц (правильность работы для разделителей не гарантируется)
inline bool codepages::IsPositionNormal(UINT position)
{
	return position >= CodePagesMenu->GetItemCount() - static_cast<UINT>(normalCodePages);
}

// Формируем строку для визуального представления таблицы символов
string codepages::FormatCodePageString(uintptr_t CodePage, const string& CodePageName, bool IsCodePageNameCustom) const
{
	string result;
	if (static_cast<intptr_t>(CodePage) >= 0)  // CodePage != CP_DEFAULT, CP_REDETECT
	{
		result = std::to_wstring(CodePage);
		result.resize(std::max(result.size(), size_t(5)), L' ');
		result += BoxSymbols[BS_V1];
		result += (!IsCodePageNameCustom || CallbackCallSource == CodePagesFill || CallbackCallSource == CodePagesFill2? L' ' : L'*');
	}
	result += CodePageName;
	return result;
}

// Добавляем таблицу символов
void codepages::AddCodePage(const string& codePageName, uintptr_t codePage, int position, bool enabled, bool checked, bool IsCodePageNameCustom)
{
	if (CallbackCallSource == CodePagesFill)
	{
		// Вычисляем позицию вставляемого элемента
		if (position==-1)
		{
			FarListInfo info={sizeof(FarListInfo)};
			dialog->SendMessage(DM_LISTINFO, control, &info);
			position = static_cast<int>(info.ItemsNumber);
		}

		// Вставляем элемент
		FarListInsert item = {sizeof(FarListInsert),position};

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
		// Устанавливаем данные для элемента
		FarListItemData data={sizeof(FarListItemData)};
		data.Index = position;
		data.Data = &codePage;
		data.DataSize = sizeof(codePage);
		dialog->SendMessage(DM_LISTSETDATA, control, &data);
	}
	else if (CallbackCallSource == CodePagesFill2)
	{
		// Вставляем элемент
		DialogBuilderListItem2 item;

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
		if (position==-1 || position>=static_cast<int>(DialogBuilderList->size()))
		{
			DialogBuilderList->emplace_back(item);
		}
		else
		{
			DialogBuilderList->emplace(DialogBuilderList->begin()+position, item);
		}
	}
	else
	{
		// Создаём новый элемент меню
		MenuItemEx item(FormatCodePageString(codePage, codePageName, IsCodePageNameCustom));
		if (!enabled)
			item.Flags |= MIF_GRAYED;
		item.UserData = &codePage;
		item.UserDataSize = sizeof(codePage);

		// Добавляем новый элемент в меню
		if (position>=0)
			CodePagesMenu->AddItem(item, position);
		else
			CodePagesMenu->AddItem(item);

		// Если надо позиционируем курсор на добавленный элемент
		if (currentCodePage==codePage)
		{
			if ((CodePagesMenu->GetSelectPos()==-1 || GetMenuItemCodePage()!=codePage))
			{
				CodePagesMenu->SetSelectPos(position>=0?position:CodePagesMenu->GetItemCount()-1, 1);
			}
		}
	}
}

// Добавляем стандартную таблицу символов
void codepages::AddStandardCodePage(const wchar_t *codePageName, uintptr_t codePage, int position, bool enabled)
{
	bool checked = false;

	if (selectedCodePages && codePage!=CP_DEFAULT)
	{
		if (GetFavorite(codePage) & CPST_FIND)
			checked = true;
	}

	AddCodePage(codePageName, codePage, position, enabled, checked, false);
}

// Добавляем разделитель
void codepages::AddSeparator(LPCWSTR Label, int position)
{
	if (CallbackCallSource == CodePagesFill)
	{
		if (position==-1)
		{
			FarListInfo info={sizeof(FarListInfo)};
			dialog->SendMessage(DM_LISTINFO, control, &info);
			position = static_cast<int>(info.ItemsNumber);
		}

		FarListInsert item = {sizeof(FarListInsert),position};
		item.Item.Text = Label;
		item.Item.Flags = LIF_SEPARATOR;
		dialog->SendMessage(DM_LISTINSERT, control, &item);
	}
	else if (CallbackCallSource == CodePagesFill2)
	{
		// Вставляем элемент
		DialogBuilderListItem2 item;
		item.Text = Label;
		item.Flags = LIF_SEPARATOR;
		item.ItemValue = 0;

		// Вычисляем позицию вставляемого элемента
		if (position==-1 || position>=static_cast<int>(DialogBuilderList->size()))
		{
			DialogBuilderList->emplace_back(item);
		}
		else
		{
			DialogBuilderList->emplace(DialogBuilderList->begin()+position, item);
		}
	}
	else
	{
		MenuItemEx item;
		item.strName = Label;
		item.Flags = MIF_SEPARATOR;

		if (position>=0)
			CodePagesMenu->AddItem(item, position);
		else
			CodePagesMenu->AddItem(item);
	}
}

// Получаем количество элементов в списке
int codepages::GetItemsCount()
{
	if (CallbackCallSource == CodePageSelect)
	{
		return CodePagesMenu->GetItemCount();
	}
	else if (CallbackCallSource == CodePagesFill2)
	{
		return static_cast<int>(DialogBuilderList->size());
	}
	else
	{
		FarListInfo info={sizeof(FarListInfo)};
		dialog->SendMessage(DM_LISTINFO, control, &info);
		return static_cast<int>(info.ItemsNumber);
	}
}

// Получаем позицию для вставки таблицы с учётом сортировки по номеру кодовой страницы
int codepages::GetCodePageInsertPosition(uintptr_t codePage, int start, int length)
{
	for (int position=start; position < start+length; position++)
	{
		uintptr_t itemCodePage;

		if (CallbackCallSource == CodePageSelect)
			itemCodePage = GetMenuItemCodePage(position);
		else if (CallbackCallSource == CodePagesFill2)
			itemCodePage = (*DialogBuilderList)[position].ItemValue;
		else
			itemCodePage = GetListItemCodePage(position);

		if (itemCodePage >= codePage)
			return position;
	}

	return start+length;
}

// Добавляем все необходимые таблицы символов
void codepages::AddCodePages(DWORD codePages)
{
	// default & re-detect
	//
	uintptr_t cp_auto = CP_DEFAULT;
	if ( 0 != (codePages & ::DefaultCP) )
	{
		AddStandardCodePage(MSG(MDefaultCP), CP_DEFAULT, -1, true);
		cp_auto = CP_REDETECT;
	}

	AddStandardCodePage(MSG(MEditOpenAutoDetect), cp_auto, -1, (codePages & AutoCP) != 0);

	if (codePages & SearchAll)
		AddStandardCodePage(MSG(MFindFileAllCodePages), CP_SET, -1, true);

	// system codepages
	//
	AddSeparator(MSG(MGetCodePageSystem));
	AddStandardCodePage(L"ANSI", GetACP(), -1, (codePages & ::ANSI) != 0);

	if (GetOEMCP() != GetACP())
		AddStandardCodePage(L"OEM", GetOEMCP(), -1, (codePages & ::OEM) != 0);

	// unicode codepages
	//
	AddSeparator(MSG(MGetCodePageUnicode));
	AddStandardCodePage(L"UTF-8", CP_UTF8, -1, (codePages & ::UTF8) != 0);
	AddStandardCodePage(L"UTF-16 (Little endian)", CP_UNICODE, -1, (codePages & ::UTF16LE) != 0);
	AddStandardCodePage(L"UTF-16 (Big endian)", CP_REVERSEBOM, -1, (codePages & ::UTF16BE) != 0);

	// other codepages
	//
	FOR(const auto& i, data.get())
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
				AddSeparator(MSG(MGetCodePageFavorites),GetItemsCount()-normalCodePages-(normalCodePages?1:0));

			// Добавляем таблицу символов в выбранные
			AddCodePage(
				CodepageName, cp,
				GetCodePageInsertPosition(
					cp, GetItemsCount()-normalCodePages-favoriteCodePages-(normalCodePages?1:0), favoriteCodePages
				),
				true,	(selectType & CPST_FIND) != 0, IsCodePageNameCustom
			);
			// Увеличиваем счётчик выбранных таблиц символов
			favoriteCodePages++;
		}
		else if (CallbackCallSource == CodePagesFill || CallbackCallSource == CodePagesFill2 || !Global->Opt->CPMenuMode)
		{
			// добавляем разделитель между стандартными и системными таблицами символов
			if (!normalCodePages)
				AddSeparator(MSG(MGetCodePageOther));

			// Добавляем таблицу символов в нормальные
			AddCodePage(
				CodepageName, cp,
				GetCodePageInsertPosition(cp, GetItemsCount()-normalCodePages, normalCodePages	),
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

		// Удаляем/добавляем в ресестре информацию о выбранной кодовой странице
		if (State)
			SetFavorite(codePage, CPST_FAVORITE | (selectType & CPST_FIND ? CPST_FIND : 0));
		else if (selectType & CPST_FIND)
			SetFavorite(codePage, CPST_FIND);
		else
			DeleteFavorite(codePage);

		// Создаём новый элемент меню
		MenuItemEx newItem(CodePagesMenu->GetItemPtr()->strName);
		newItem.UserData = &codePage;
		newItem.UserDataSize = sizeof(codePage);
		// Сохраняем позицию курсора
		int position=CodePagesMenu->GetSelectPos();
		// Удаляем старый пункт меню
		CodePagesMenu->DeleteItem(CodePagesMenu->GetSelectPos());

		// Добавляем пункт меню в новое место
		if (State)
		{
			// Добавляем разделитель, если выбранных кодовых страниц ещё не было
			// и после добавления останутся нормальные кодовые страницы
			if (!favoriteCodePages && normalCodePages>1)
				AddSeparator(MSG(MGetCodePageFavorites),CodePagesMenu->GetItemCount()-normalCodePages);

			// Ищем позицию, куда добавить элемент
			int newPosition = GetCodePageInsertPosition(
			                      codePage,
			                      CodePagesMenu->GetItemCount()-normalCodePages-favoriteCodePages,
			                      favoriteCodePages
			                  );
			// Добавляем кодовою страницу в выбранные
			CodePagesMenu->AddItem(newItem, newPosition);

			// Удаляем разделитель, если нет обыкновынных кодовых страниц
			if (normalCodePages==1)
				CodePagesMenu->DeleteItem(CodePagesMenu->GetItemCount()-1);

			// Изменяем счётчики нормальных и выбранных кодовых страниц
			favoriteCodePages++;
			normalCodePages--;
			position++;
		}
		else
		{
			// Удаляем разделитеь, если после удаления не останнется ни одной
			// выбранной таблицы символов
			if (favoriteCodePages==1 && normalCodePages>0)
				CodePagesMenu->DeleteItem(CodePagesMenu->GetItemCount()-normalCodePages-2);

			// Переносим элемент в нормальные таблицы, только если они показываются
			if (!Global->Opt->CPMenuMode)
			{
				// Добавляем разделитель, если не было ни одной нормальной кодовой страницы
				if (!normalCodePages)
					AddSeparator(MSG(MGetCodePageOther));

				// Добавляем кодовою страницу в нормальные
				CodePagesMenu->AddItem(
				    newItem,
				    GetCodePageInsertPosition(
				        codePage,
				        CodePagesMenu->GetItemCount()-normalCodePages,
				        normalCodePages
				    )
				);
				normalCodePages++;
			}
			// Если в режиме скрытия нормальных таблиц мы удалили последнюю выбранную таблицу, то удаляем и разделитель
			else if (favoriteCodePages==1)
				CodePagesMenu->DeleteItem(CodePagesMenu->GetItemCount()-normalCodePages-1);

			favoriteCodePages--;

			if (position==CodePagesMenu->GetItemCount()-normalCodePages-1)
				position--;
		}

		// Устанавливаем позицию в меню
		CodePagesMenu->SetSelectPos(position>=CodePagesMenu->GetItemCount() ? CodePagesMenu->GetItemCount()-1 : position, 1);

		// Показываем меню
		if (Global->Opt->CPMenuMode)
			CodePagesMenu->SetPosition(-1, -1, 0, 0);
	}
}

// Заполняем меню выбора таблиц символов
void codepages::FillCodePagesVMenu(bool bShowUnicode, bool bViewOnly, bool bShowAutoDetect)
{
	uintptr_t codePage = currentCodePage;

	if (CodePagesMenu->GetSelectPos()!=-1 && CodePagesMenu->GetSelectPos()<CodePagesMenu->GetItemCount()-normalCodePages)
		currentCodePage = GetMenuItemCodePage();

	// Очищаем меню
	favoriteCodePages = normalCodePages = 0;
	CodePagesMenu->DeleteItems();

	string title = MSG(MGetCodePageTitle);
	if (Global->Opt->CPMenuMode)
		title += L" *";
	CodePagesMenu->SetTitle(title);

	// Добавляем таблицы символов
	AddCodePages(::OEM | ::ANSI | ::UTF8
		| (bShowUnicode ? (::UTF16BE | ::UTF16LE) : 0)
		| (bViewOnly ? ::VOnly : 0)
		| (bShowAutoDetect ? ::AutoCP : 0)
	);
	// Восстанавливаем оригинальню таблицу символов
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
	string strCodePage = std::to_wstring(CodePage);
	string CurrentCodePageName;

	// Пытаемся получить заданное пользователем имя таблицы символов
	if (Global->Db->GeneralCfg()->GetValue(NamesOfCodePagesKey, strCodePage, CurrentCodePageName, L""))
	{
		IsCodePageNameCustom = true;
		if (CurrentCodePageName == CodePageName)
		{
			Global->Db->GeneralCfg()->DeleteValue(NamesOfCodePagesKey, strCodePage);
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
	if (Msg==DN_CLOSE)
	{
		if (Param1==EDITCP_OK || Param1==EDITCP_RESET)
		{
			string strCodePageName;
			uintptr_t CodePage = GetMenuItemCodePage();
			string strCodePage = std::to_wstring(CodePage);

			if (Param1==EDITCP_OK)
			{
				strCodePageName = reinterpret_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, EDITCP_EDIT, nullptr));
			}
			// Если имя кодовой страницы пустое, то считаем, что имя не задано
			if (strCodePageName.empty())
				Global->Db->GeneralCfg()->DeleteValue(NamesOfCodePagesKey, strCodePage);
			else
				Global->Db->GeneralCfg()->SetValue(NamesOfCodePagesKey, strCodePage, strCodePageName);
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
				NewItem.UserData = &CodePage;
				NewItem.UserDataSize = sizeof(CodePage);
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
	string CodePageName = CodePagesMenu->GetItemPtr(Position)->strName;
	size_t BoxPosition = CodePageName.find(BoxSymbols[BS_V1]);
	if (BoxPosition == string::npos)
		return;
	CodePageName.erase(0, BoxPosition+2);
	FarDialogItem EditDialogData[]=
	{
		{DI_DOUBLEBOX, 3, 1, 50, 5, 0, nullptr, nullptr, 0, MSG(MGetCodePageEditCodePageName)},
		{DI_EDIT,      5, 2, 48, 2, 0, L"CodePageName", nullptr, DIF_FOCUS|DIF_HISTORY, CodePageName.data()},
		{DI_TEXT,     -1, 3,  0, 3, 0, nullptr, nullptr, DIF_SEPARATOR, L""},
		{DI_BUTTON,    0, 4,  0, 3, 0, nullptr, nullptr, DIF_DEFAULTBUTTON|DIF_CENTERGROUP, MSG(MOk)},
		{DI_BUTTON,    0, 4,  0, 3, 0, nullptr, nullptr, DIF_CENTERGROUP, MSG(MCancel)},
		{DI_BUTTON,    0, 4,  0, 3, 0, nullptr, nullptr, DIF_CENTERGROUP, MSG(MGetCodePageResetCodePageName)}
	};
	auto EditDialog = MakeDialogItemsEx(EditDialogData);
	Dialog Dlg(EditDialog, this, &codepages::EditDialogProc);
	Dlg.SetPosition(-1, -1, 54, 7);
	Dlg.SetHelp(L"EditCodePageNameDlg");
	Dlg.Process();
}

bool codepages::SelectCodePage(uintptr_t& CodePage, bool bShowUnicode, bool bViewOnly, bool bShowAutoDetect)
{
	bool Result = false;
	CallbackCallSource = CodePageSelect;
	currentCodePage = CodePage;
	// Создаём меню
	CodePagesMenu = std::make_unique<VMenu2>(L"", nullptr, 0, ScrY-4);
	CodePagesMenu->SetBottomTitle(MSG(!Global->Opt->CPMenuMode?MGetCodePageBottomTitle:MGetCodePageBottomShortTitle));
	CodePagesMenu->SetFlags(VMENU_WRAPMODE|VMENU_AUTOHIGHLIGHT);
	CodePagesMenu->SetHelp(L"CodePagesMenu");
	CodePagesMenu->SetId(CodePagesMenuId);
	// Добавляем таблицы символов
	FillCodePagesVMenu(bShowUnicode, bViewOnly, bShowAutoDetect);
	// Показываем меню

	// Цикл обработки сообщений меню
	intptr_t r=CodePagesMenu->Run([&](int ReadKey)->int
	{
		int KeyProcessed = 1;
		switch (ReadKey)
		{
			// Обработка скрытия/показа системных таблиц символов
			case KEY_CTRLH:
			case KEY_RCTRLH:
				Global->Opt->CPMenuMode = !Global->Opt->CPMenuMode;
				CodePagesMenu->SetBottomTitle(MSG(!Global->Opt->CPMenuMode?MGetCodePageBottomTitle:MGetCodePageBottomShortTitle));
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
void codepages::FillCodePagesList(std::vector<DialogBuilderListItem2> &List, bool allowAuto, bool allowAll, bool allowDefault, bool bViewOnly)
{
	CallbackCallSource = CodePagesFill2;
	// Устанавливаем переменные для доступа из каллбака
	DialogBuilderList = &List;
	favoriteCodePages = normalCodePages = 0;
	selectedCodePages = !allowAuto && allowAll;
	// Добавляем стндартные элементы в список
	AddCodePages
	( (allowDefault ? ::DefaultCP : 0)
	| (allowAuto ? ::AutoCP : 0)
	| (allowAll ? ::SearchAll : 0)
	| (bViewOnly ? ::VOnly : 0)
	| ::AllStandard
	);
	DialogBuilderList = nullptr;
}


// Заполняем список таблицами символов
UINT codepages::FillCodePagesList(Dialog* Dlg, UINT controlId, uintptr_t codePage, bool allowAuto, bool allowAll, bool allowDefault, bool bViewOnly)
{
	CallbackCallSource = CodePagesFill;
	// Устанавливаем переменные для доступа из каллбака
	dialog = Dlg;
	control = controlId;
	currentCodePage = codePage;
	favoriteCodePages = normalCodePages = 0;
	selectedCodePages = !allowAuto && allowAll;
	// Добавляем стндартные элементы в список
	AddCodePages
		( (allowDefault ? ::DefaultCP : 0)
		| (allowAuto ? ::AutoCP : 0)
		| (allowAll ? ::SearchAll : 0)
		| (bViewOnly ? ::VOnly : 0)
		| ::AllStandard
	);

	if (CallbackCallSource == CodePagesFill)
	{
		// Если надо выбираем элемент
		FarListInfo info={sizeof(FarListInfo)};
		Dlg->SendMessage(DM_LISTINFO, control, &info);

		for (int i=0; i<static_cast<int>(info.ItemsNumber); i++)
		{
			if (GetListItemCodePage(i)==codePage)
			{
				FarListGetItem Item={sizeof(FarListGetItem),i};
				dialog->SendMessage(DM_LISTGETITEM, control, &Item);
				dialog->SendMessage(DM_SETTEXTPTR, control, const_cast<wchar_t*>(Item.Item.Text));
				FarListPos Pos={sizeof(FarListPos),i,-1};
				dialog->SendMessage(DM_LISTSETCURPOS, control, &Pos);
				break;
			}
		}
	}

	// Возвращаем число избранных таблиц символов
	return favoriteCodePages;
}

bool codepages::IsCodePageSupported(uintptr_t CodePage, size_t MaxCharSize) const
{
	if (CodePage == CP_DEFAULT || IsStandardCodePage(CodePage))
		return true;

	auto CharSize = GetCodePageInfo(static_cast<UINT>(CodePage)).first;
	return CharSize != 0 && CharSize <= MaxCharSize;
}

long long codepages::GetFavorite(uintptr_t cp)
{
	long long value = 0;
	Global->Db->GeneralCfg()->GetValue(FavoriteCodePagesKey, std::to_wstring(cp), &value, 0);
	return value;
}

void codepages::SetFavorite(uintptr_t cp, long long value)
{
	Global->Db->GeneralCfg()->SetValue(FavoriteCodePagesKey, std::to_wstring(cp), value);
}

void codepages::DeleteFavorite(uintptr_t cp)
{
	Global->Db->GeneralCfg()->DeleteValue(FavoriteCodePagesKey, std::to_wstring(cp));
}

GeneralConfig::values_enumerator<DWORD> codepages::GetFavoritesEnumerator()
{
	return Global->Db->GeneralCfg()->GetIntValuesEnumerator(FavoriteCodePagesKey);
}


inline bool IsValid(UINT cp)
{
	if (cp==CP_ACP || cp==CP_OEMCP || cp==CP_MACCP || cp==CP_THREAD_ACP || cp==CP_SYMBOL)
		return false;

	if (cp==CP_UTF8 || cp==CP_UNICODE || cp==CP_REVERSEBOM)
		return false;

	return Codepages().GetCodePageInfo(cp).first == 2;
}


bool MultibyteCodepageDecoder::SetCP(UINT cp)
{
	if (cp && cp == current_cp)
		return true;

	if (!IsValid(cp))
		return false;

	if (len_mask.empty())
		len_mask.resize(256);
	else
		std::fill(ALL_RANGE(len_mask), 0);

	if (m1.empty())
		m1.resize(256);
	else
		std::fill(ALL_RANGE(m1), 0);

	if (m2.empty())
		m2.resize(256*256);
	else
		std::fill(ALL_RANGE(m2), 0);

	BOOL DefUsed, *pDefUsed = (cp==CP_UTF8 || cp==CP_UTF7) ? nullptr : &DefUsed;
	DWORD flags = WC_NO_BEST_FIT_CHARS;
	if (cp==CP_UTF8 || cp==CP_UTF7 || cp==54936 || cp==CP_SYMBOL
	 || (cp>=50220 && cp<=50222) || cp==50225 || cp==50227 || cp==50229 || (cp>=57002 && cp<=57011)
	) flags = 0;

	union {
		BYTE bf[2];
		BYTE    b1;
		UINT16  b2;
	} u;

	int nnn = 0, mb = 0;
	for (unsigned w=0x0000; w <= 0xffff; ++w) // only UCS2 range
	{
		DefUsed = FALSE;
		auto wc = static_cast<wchar_t>(w);
		int len = WideCharToMultiByte(cp, flags, &wc,1, (LPSTR)u.bf,(int)sizeof(u.bf), nullptr, pDefUsed);
		if (len <= 0 || DefUsed)
			continue;

		len_mask[u.b1] |= (BYTE)(1 << (len - 1));
		++nnn;
		if (mb < len) mb = len;

		switch (len)
		{
			case 1: m1[u.b1] = wc; break;
			case 2: m2[u.b2] = wc; break;
		}
	}

	assert(nnn >= 256);
	if (nnn < 256)
		return false;

	current_cp = cp;
	current_mb = mb;
	return true;
}

int MultibyteCodepageDecoder::GetChar(const BYTE *bf, size_t cb, wchar_t& wc) const
{
	if (!bf || cb < 1)
		return -11; // invalid argument

	BYTE b1 = bf[0];
	BYTE lmask = len_mask[b1];
	if (!lmask)
		return -1;
	if (lmask & 0x01)
	{
		wc = m1[b1];
		return +1;
	}

	if (cb < 2)
		return -12;

	UINT16 b2 = b1 | (bf[1] << 8);
	if (!m2[b2])
		return -2;
	else {
		wc = m2[b2];
		return +2;
	}
}

//################################################################################################
//################################################################################################

int Utf::ToWideChar(uintptr_t cp,const char *src, int len, wchar_t* out, int wlen, Errs *errs)
{
	if (cp == CP_UTF8)
		return Utf8::ToWideChar(src, len, out, wlen, errs);

	else if (cp == CP_UTF7)
		return Utf7::ToWideChar(src, len, out, wlen, errs);

	else
		return -1;
}

//################################################################################################

//                                   2                         5         6
//	        0                         6                         2         2
// base64: ABCDEFGHIJKLMNOPQRSTUVWXYZabcdrfghijklmnopqrstuvwxyz0123456789+/

static const int ill = 0x0100; // illegal
static const int dir = 0x0200; // direct
static const int opt = 0x0400; // optional direct
static const int b64 = 0x0800; // base64 symbol
static const int pls = 0x1000; // +
static const int mns = 0x2000; // -

static const int ILL = ill + 255;
static const int DIR = dir + 255;
static const int OPT = opt + 255;
static const int PLS = pls + b64 + 62;
static const int MNS = mns + dir + 255;
#define D(n) dir + b64 + n

static const short m7[128] =
//  x00   x01   x02   x03   x04   x05   x06   x07   x08   x09   x0a   x0b   x0c   x0d   x0e   x0f
{   ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  DIR,  DIR,  ILL,  ILL,  DIR,  ILL,  ILL

//  x10   x11   x12   x13   x14   x15   x16   x17   x18   x19   x1a   x1b   x1c   x1d   x1e   x1f
,   ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL,  ILL

// =x20 !=x21 "=x22 #=x23 $=x24 %=x25 &=x26 '=x27 (=x28 )=x29 *=x2a +=x2b ,x=2c -=x2d .=x2e /=x2f
,   DIR,  OPT,  OPT,  OPT,  OPT,  OPT,  OPT,  DIR,  DIR,  DIR,  OPT,  PLS,  DIR,  MNS, DIR, D(63)

//0=x30 1=x31 2=x32 3=x33 4=x34 5=x35 6=x36 7=x37 8=x38 9=x39 :=x3a ;=x3b <=x3c ==x3d >=x3e ?=x3f
, D(52),D(53),D(54),D(55),D(56),D(57),D(58),D(59),D(60),D(61),  DIR,  OPT,  OPT,  OPT,  OPT,  DIR

//@=x40 A=x41 B=x42 C=x43 D=x44 E=x45 F=x46 G=x47 H=x48 I=x49 J=x4a K=x4b L=x4c M=x4d N=x4e O=x4f
,   OPT, D(0), D(1), D(2), D(3), D(4), D(5), D(6), D(7), D(8), D(9),D(10),D(11),D(12),D(13),D(14)

//P=x50 Q=x51 R=x52 S=x53 T=x54 U=x55 V=x56 W=x57 X=x58 Y=x59 Z=x5a [=x5b \=x5c ]=x5d ^=x5e _=x5f
, D(15),D(16),D(17),D(18),D(19),D(20),D(21),D(22),D(23),D(24),D(25),  OPT,  ILL,  OPT,  OPT,  OPT

//`=x60 a=x61 b=x62 c=x63 d=x64 e=x65 f=x66 g=x67 h=x68 i=x69 j=x6a k=x6b l=x6c m=x6d n=x6e o=x6f
,   OPT,D(26),D(27),D(28),D(29),D(30),D(31),D(32),D(33),D(34),D(35),D(36),D(37),D(38),D(39),D(40)

//p=x70 q=x71 r=x72 s=x73 t=x74 u=x75 v=x76 w=x77 x=x78 y=x79 z=x7a {=x7b |=x7c }=x7d ~=x7e   x7f
, D(41),D(42),D(43),D(44),D(45),D(46),D(47),D(48),D(49),D(50),D(51),  OPT,  OPT,  OPT,  ILL,  ILL
};

static int Utf7_GetChar(const BYTE *bf, size_t cb, wchar_t& wc, int& state)
{
	if (!cb)
		return 0;

	int nc= 1, m[3];
	BYTE c = *bf++;
	if (c >= 128)
		return -nc;

	union
	{
		int state;
		struct { BYTE carry_bits; BYTE carry_count; bool base64; BYTE unused; } s;
	} u;
	u.state = state;

	m[0] = static_cast<int>(m7[c]);
	if ((m[0] & ill) != 0)
		return -nc;

	if (!u.s.base64)
	{
		if (c != (BYTE)'+')
		{
			wc = static_cast<wchar_t>(c);
			return nc;
		}
		if (cb < 2)
			return -nc;

		c = *bf++;
		nc = 2;
		if (c >= 128)
			return -nc;

		if (c == (BYTE)'-')
		{
			wc = L'+';
			return nc;
		}

		m[0] = static_cast<int>(m7[c]);
		if (0 == (m[0] & b64))
			return -nc;

		u.s.base64 = true;
		u.s.carry_count = 0;
	}

	int a = 2 - u.s.carry_count / 4;
	if (cb < static_cast<size_t>(nc) + a)
		return -nc-a;

	if ((c = *bf++) >= 128)
	{
		u.s.base64 = false;
		state = u.state;
		return -nc;
	}
	m[1] = static_cast<int>(m7[c]);
	if (0 == (m[1] & b64))
	{
		u.s.base64 = false;
		state = u.state;
		return -nc;
	}
	if (a < 2)
	{
		wc = static_cast<wchar_t>((u.s.carry_bits << 12) | ((BYTE)m[0] << 6) | (BYTE)m[1]);
		u.s.carry_count = 0;
	}
	else
	{
		++nc;
		if ((c = *bf++) >= 128)
		{
			u.s.base64 = false;
			state = u.state;
			return -nc;
		}
		m[2] = static_cast<int>(m7[c]);
		if (0 == (m[2] & b64))
		{
			u.s.base64 = false;
			state = u.state;
			return -nc;
		}
		unsigned m18 = ((BYTE)m[0] << 12) | ((BYTE)m[1] << 6) | ((BYTE)m[2]);

		if (u.s.carry_count == 0)
		{
			wc = static_cast<wchar_t>(m18 >> 2);
			u.s.carry_bits = (BYTE)(m18 & 0x03);
			u.s.carry_count = 2;
		}
		else
		{
			wc = static_cast<wchar_t>((u.s.carry_bits << 14) | (m18 >> 4));
			u.s.carry_bits = (BYTE)(m18 & 0x07);
			u.s.carry_count = 4;
		}
	}
	++nc;

	if (cb > static_cast<size_t>(nc) && *bf == (BYTE)'-')
	{
		u.s.base64 = false;
		++nc;
	}

	state = u.state;
	return nc;
}

int Utf7::ToWideChar(const char *src, int length, wchar_t* out, int wlen, Utf::Errs *errs)
{
	if (errs)
	{
		errs->first_src = errs->first_out = -1;
		errs->count = 0;
		errs->small_buff = false;
	}

	if (!src || length <= 0)
		return 0;

	int state = 0, no = 0, ns = 0, ne = 0, move = 1;
	wchar_t dummy_out, w1 = Utf::REPLACE_CHAR;
	if (!out)
	{
		out = &dummy_out; move = 0;
	}

	while (length > ns)
	{
		int nc = Utf7_GetChar((const BYTE *)src+ns, length-ns, w1, state);
		if (!nc)
			break;

		if (nc < 0)
		{
			w1 = Utf::REPLACE_CHAR; nc = -nc;
			if (errs && 1 == ++ne)
			{
				errs->first_src = ns; errs->first_out = no;
			}
		}

		if (move && --wlen < 0)
		{
			if (errs)
				errs->small_buff = true;

			out = &dummy_out;
			move = 0;
		}

		*out = w1;
		out += move;
		++no;
		ns += nc;
	}

	if (errs)
		errs->count = ne;

	return no;
}

//################################################################################################

int Utf8::ToWideChar(const char *s, int nc, wchar_t *w1,wchar_t *w2, int wlen, int &tail )
{
	bool need_one = wlen <= 0;
	if (need_one)
		wlen = 2;

	int ic = 0, nw = 0, wc;

	while ( ic < nc )
	{
		unsigned char c1 = ((const unsigned char *)s)[ic++];

		if (c1 < 0x80) // simple ASCII
			wc = (wchar_t)c1;
		else if ( c1 < 0xC2 || c1 >= 0xF5 ) // illegal 1-st byte
			wc = -1;
		else
		{ // multibyte (2, 3, 4)
			if (ic + 0 >= nc )
			{ // unfinished
unfinished:
				if ( nw > 0 )
					tail = nc - ic + 1;
				else
				{
					tail = 0;
					w1[0] = Utf::REPLACE_CHAR;
					if (w2)
						w2[0] = L'?';
					nw = 1;
				}
				return nw;
			}
			unsigned char c2 = ((const unsigned char *)s)[ic];
			if ( 0x80 != (c2 & 0xC0)       // illegal 2-nd byte
				|| (0xE0 == c1 && c2 <= 0x9F) // illegal 3-byte start (overlaps with 2-byte)
				|| (0xF0 == c1 && c2 <= 0x8F) // illegal 4-byte start (overlaps with 3-byte)
				|| (0xF4 == c1 && c2 >= 0x90) // illegal 4-byte (out of unicode range)
				)
			{
				wc = -1;
			}
			else if ( c1 < 0xE0 )
			{ // legal 2-byte
				++ic;
				wc = ((c1 & 0x1F) << 6) | (c2 & 0x3F);
			}
			else
			{ // 3 or 4-byte
				if (ic + 1 >= nc )
					goto unfinished;
				unsigned char c3 = ((const unsigned char *)s)[ic+1];
				if ( 0x80 != (c3 & 0xC0) ) // illegal 3-rd byte
					wc = -1;
				else if ( c1 < 0xF0 )
				{ // legal 3-byte
					ic += 2;
					wc = ((c1 & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
				}
				else
				{ // 4-byte
					if (ic + 2 >= nc )
						goto unfinished;

					unsigned char c4 = ((const unsigned char *)s)[ic+2];
					if ( 0x80 != (c4 & 0xC0) ) // illegal 4-th byte
						wc = -1;
					else
					{ // legal 4-byte (produce 2 WCHARs)
						ic += 3;
						wc = ((c1 & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
						wc -= 0x10000;
						w1[nw] = (wchar_t)(0xD800 + (wc >> 10));
						if (w2)
							w2[nw] = w1[nw];
						++nw;
						wc = 0xDC00 + (wc & 0x3FF);
#if 0
						_ASSERTE(nw < wlen); //??? caller should be fixed to avoid this...
#endif
						if (nw >= wlen)
						{
							--nw;
							wc = Utf::REPLACE_CHAR;
						}
					}
				}
			}
		}

		if ( wc >= 0 )
		{
			w1[nw] = (wchar_t)wc;
			if (w2)
				w2[nw] = (wchar_t)wc;
		}
		else
		{
			w1[nw] = Utf::REPLACE_CHAR;
			if (w2)
				w2[nw] = L'?';
		}
		if (++nw >= wlen || need_one)
			break;
	}

	tail = nc - ic;
	return nw;
}

static inline int Utf8_GetChar(const char *src, int length, wchar_t* wc)
{
	wchar_t w1[2], w2[2];
	int tail;
	int WCharCount = Utf8::ToWideChar(src, length, w1, w2, -2, tail);
	
	if (WCharCount <= 0)
		return 0;

	wc[0] = w1[0];
	if (WCharCount > 1)
	{
		wc[1] = w1[1];
	}

	if (w1[0] == Utf::REPLACE_CHAR && w2[0] == L'?')
		return tail - length; // failed: negative
	else
		return length - tail; // succeed: positive
 }

int Utf8::ToWideChar(const char *src, int length, wchar_t* out, int wlen, Utf::Errs *errs)
{
	if (errs)
	{
		errs->first_src = errs->first_out = -1;
		errs->count = 0;
		errs->small_buff = false;
	}

	if (!src || length <= 0)
		return 0;

	int no = 0, ns = 0, ne = 0, move = 1;
	wchar_t dummy_out[2];
	if (!out)
	{
		out = dummy_out; move = 0;
	}

	while (length > ns)
	{
		wchar_t w1[2] = {};
		int nc = Utf8_GetChar(src+ns, length-ns, w1);
		if (!nc)
			break;

		if (nc < 0)
		{
			w1[0] = Utf::REPLACE_CHAR; nc = -nc;
			if (errs && 1 == ++ne)
			{
				errs->first_src = ns; errs->first_out = no;
			}
		}

		auto Decrement = w1[1]? 2 : 1;
		if (move && (wlen -= Decrement) < 0)
		{
			if (errs)
				errs->small_buff = true;

			out = dummy_out;
			move = 0;
		}

		*out = w1[0];
		out += move;
		++no;

		if (w1[1])
		{
			*out = w1[1];
			out += move;
			++no;
		}

		ns += nc;
	}

	if (errs)
		errs->count = ne;

	return no;
}

//################################################################################################
//################################################################################################
