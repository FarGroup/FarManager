/*
gettable.cpp

Работа с таблицами символов
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

#include "gettable.hpp"
#include "lang.hpp"
#include "vmenu.hpp"
#include "savefpos.hpp"
#include "keys.hpp"
#include "registry.hpp"
#include "language.hpp"
#include "dialog.hpp"
#include "interf.hpp"
#include "config.hpp"

// Стандартные кодовое страницы
enum StandardCodePages
{
	SearchAll = 1,
	Auto = 2,
	OEM = 4,
	ANSI = 8,
	UTF7 = 16,
	UTF8 = 32,
	UTF16LE = 64,
	UTF16BE = 128,
	AllStandard = OEM | ANSI | UTF7 | UTF8 | UTF16BE | UTF16LE
};

// Диалог
static HANDLE dialog;
// Идентифкатор диалога
static UINT control;
// Меню
static VMenu *tables = NULL;
// Текущая таблица символов
static UINT currentCodePage;
// Количество выбранных и обыкновенных таблиц символов
static int favoriteCodePages, normalCodePages;
// Признак необходимости отображать таблицы символов для поиска
static bool selectedCodePages;

// Добавляем таблицу символов
void AddTable(const wchar_t *codePageName, UINT codePage, int position = -1, bool enabled = true, bool checked = false)
{
	if (!tables)
	{
		// Вычисляем позицию вставляемого элемента
		if (position==-1)
		{
			FarListInfo info;
			Dialog::SendDlgMessage(dialog, DM_LISTINFO, control, (LONG_PTR)&info);
			position = info.ItemsNumber;
		}
		// Вставляем элемент
		FarListInsert item = {position};
		UnicodeString name;
		if (codePage==CP_AUTODETECT)
			name = codePageName;
		else
			name.Format(L"%5u%c %s", codePage, BoxSymbols[BS_V1], codePageName);
		item.Item.Text = name.GetBuffer();
		if (checked)
			item.Item.Flags |= MIF_CHECKED;
		Dialog::SendDlgMessage(dialog, DM_LISTINSERT, control, (LONG_PTR)&item);
		// Устанавливаем данные для элемента
		FarListItemData data;
		data.Index = position;
		data.Data = (void*)(DWORD_PTR)codePage;
		data.DataSize = sizeof(UINT);
		Dialog::SendDlgMessage(dialog, DM_LISTSETDATA, control, (LONG_PTR)&data);
		// Если надо выбираем элемент
		FarListInfo info;
		Dialog::SendDlgMessage(dialog, DM_LISTINFO, control, (LONG_PTR)&info);
		if (info.ItemsNumber==1 || (codePage==currentCodePage && (UINT)Dialog::SendDlgMessage(dialog, DM_LISTGETDATA, control, info.SelectPos)!=codePage))
		{
			Dialog::SendDlgMessage(dialog, DM_LISTSETCURPOS, control, (LONG_PTR)&position);
			Dialog::SendDlgMessage(dialog, DM_SETTEXTPTR, control, (LONG_PTR)item.Item.Text);
		}
	}
	else
	{
		// Создаём новый элемент меню
		MenuItemEx item;
		item.Clear();
		if (!enabled)
			item.Flags |= MIF_GRAYED;
		if (codePage==CP_AUTODETECT)
			item.strName = codePageName;
		else
			item.strName.Format(L"%5u%c %s", codePage, BoxSymbols[BS_V1], codePageName);
		item.UserData = (char *)(UINT_PTR)codePage;
		item.UserDataSize = sizeof(UINT);
		// Добавляем новый элемент в меню
		if (position>=0)
			tables->AddItem(&item, position);
		else
			tables->AddItem(&item);
		// Если надо позиционируем курсор на добавленный элемент
		if (currentCodePage==codePage && (tables->GetSelectPos()==-1 || (UINT)(UINT_PTR)tables->GetItemPtr()->UserData!=codePage))
			tables->SetSelectPos(position>=0?position:tables->GetItemCount()-1, 1);
		// Корректируем позицию выбранного элемента
		else if (position!=-1 && tables->GetSelectPos()>=position)
			tables->SetSelectPos(tables->GetSelectPos()+1, 1);
	}
}

// Добавляем стандартную таблицу символов
void AddStandardTable(const wchar_t *codePageName, UINT codePage, int position = -1, bool enabled = true)
{
	bool checked = false;
	if (selectedCodePages && codePage!=CP_AUTODETECT)
	{
		string strCodePageName;
		strCodePageName.Format(L"%u", codePage);
		int selectType = 0;
		GetRegKey(FavoriteCodePagesKey, strCodePageName, selectType, 0);
		if (selectType & CPST_FIND)
			checked = true;
	}
	AddTable(codePageName, codePage, position, enabled, checked);
}

// Добавляем разделитель
void AddSeparator(int position = -1)
{
	if (!tables)
	{
		if (position==-1)
		{
			FarListInfo info;
			Dialog::SendDlgMessage(dialog, DM_LISTINFO, control, (LONG_PTR)&info);
			position = info.ItemsNumber;
		}
		FarListInsert item = {position};
		item.Item.Flags = LIF_SEPARATOR;
		Dialog::SendDlgMessage(dialog, DM_LISTINSERT, control, (LONG_PTR)&item);
	}
	else
	{
		MenuItemEx item;
		item.Clear();
		item.Flags = MIF_SEPARATOR;
		if (position>=0)
			tables->AddItem(&item, position);
		else
			tables->AddItem(&item);
		// Корректируем позицию выбранного элемента
		if (position!=-1 && tables->GetSelectPos()>=position)
			tables->SetSelectPos(tables->GetSelectPos()+1, 1);
	}
}

// Получаем количество элементов в списке
int GetItemsCount()
{
	if (tables)
		return tables->GetItemCount();
	else
	{
		FarListInfo info;
		Dialog::SendDlgMessage(dialog, DM_LISTINFO, control, (LONG_PTR)&info);
		return info.ItemsNumber;
	}
}

// Получаем позицию для вставки таблицы с учётом сортировки по номеру кодовой страницы
int GetTableInsertPosition(UINT codePage, int start, int length)
{
	if (length==0)
		return start;

	int position = start;
	do
	{
		UINT itemCodePage;
		if (tables)
			itemCodePage = (UINT)(UINT_PTR)tables->GetItemPtr(position)->UserData;
		else
			itemCodePage = (UINT)(UINT_PTR)Dialog::SendDlgMessage(dialog, DM_LISTGETDATA, control, position);
		if (itemCodePage >= codePage)
			return position;
	} while (position++<start+length);

	return position-1;
}

// Callback-функция получения таблиц символов
BOOL __stdcall EnumCodePagesProc(const wchar_t *lpwszCodePage)
{
	UINT codePage = _wtoi(lpwszCodePage);
	// BUBBUG: Существует много кодировок с cpi.MaxCharSize > 1, пока их не поддерживаем
	CPINFOEXW cpi;
	if (GetCPInfoExW(codePage, 0, &cpi) && cpi.MaxCharSize == 1)
	{
		// Формируем имя таблиц символов
		// под виндой на входе "XXXX (Name)" а например под wine просто "Name"
		wchar_t *codePageName = wcschr(cpi.CodePageName, L'(');
		if (codePageName && *(++codePageName))
			codePageName[wcslen(codePageName)-1] = L'\0';
		else
			codePageName = cpi.CodePageName;
		// Получаем признак выбранности таблицы символов
		int selectType = 0;
		GetRegKey(FavoriteCodePagesKey, lpwszCodePage, selectType, 0);
		// Добавляем таблицу символов либо в нормальные, либо в выбранные таблицы симовлов
		if (selectType & CPST_FAVORITE)
		{
			// добавляем разделитель между стандартными и системными таблицами символов
			if (!favoriteCodePages && !normalCodePages)
				AddSeparator();
			// Добавляем таблицу символов в выбранные
			AddTable(
					codePageName,
					codePage,
					GetTableInsertPosition(
							codePage,
							GetItemsCount()-normalCodePages-favoriteCodePages-((favoriteCodePages && normalCodePages)?1:0),
							favoriteCodePages
						),
					true,
					selectType & CPST_FIND ? true : false
				);
			// Если надо добавляем разделитель между выбранными и нормальными таблицами симовлов
			if (!favoriteCodePages && normalCodePages)
				AddSeparator(GetItemsCount()-normalCodePages);
			// Увеличиваем счётчик выбранных таблиц символов
			favoriteCodePages++;
		}
		else if (!tables || !Opt.CPMenuMode)
		{
			// добавляем разделитель между стандартными и системными таблицами символов
			if (!favoriteCodePages && !normalCodePages)
				AddSeparator();
			// Добавляем таблицу символов в нормальные
			AddTable(
					codePageName,
					codePage,
					GetTableInsertPosition(
							codePage,
							GetItemsCount()-normalCodePages,
							normalCodePages
						)
				);
			// Если надо добавляем разделитель между выбранными и нормальными таблицами симовлов
			if (favoriteCodePages && !normalCodePages)
				AddSeparator(GetItemsCount()-normalCodePages);
			// Увеличиваем счётчик выбранных таблиц символов
			normalCodePages++;
		}
	}
	return TRUE;
}

void AddTables(DWORD codePages)
{
	// Добавляем стандартные таблицы символов
	if ((codePages & ::SearchAll) || (codePages & ::Auto))
	{
		AddStandardTable((codePages & ::Auto) ? MSG(MEditOpenAutoDetect) : MSG(MFindFileAllCodePages), CP_AUTODETECT, -1, true);
		AddSeparator();
	}
	AddStandardTable(L"OEM", GetOEMCP(), -1, (codePages & ::OEM)?1:0);
	AddStandardTable(L"ANSI", GetACP(), -1, (codePages & ::ANSI)?1:0);
	AddSeparator();
	AddStandardTable(L"UTF-7", CP_UTF7, -1, (codePages & ::UTF7)?1:0);
	AddStandardTable(L"UTF-8", CP_UTF8, -1, (codePages & ::UTF8)?1:0);
	AddStandardTable(L"UTF-16 (Little endian)", CP_UNICODE, -1, (codePages & ::UTF16LE)?1:0);
	AddStandardTable(L"UTF-16 (Big endian)", CP_REVERSEBOM, -1, (codePages & ::UTF16BE)?1:0);
	// Получаем таблицы символов установленные в системе
	EnumSystemCodePagesW((CODEPAGE_ENUMPROCW)EnumCodePagesProc, CP_INSTALLED);
}

// Обработка добавления/удаления в/из список выбранных таблиц символов
void ProcessSelected(bool select)
{
	if (Opt.CPMenuMode && select)
		return;
	MenuItemEx *curItem = tables->GetItemPtr();
	UINT itemPosition = tables->GetSelectPos();
	UINT itemCount = tables->GetItemCount();
	UINT codePage = (UINT)(UINT_PTR)curItem->UserData;
	if ((select && itemPosition >= itemCount-normalCodePages) || (!select && itemPosition>=itemCount-normalCodePages-favoriteCodePages-(normalCodePages?1:0) && itemPosition < itemCount-normalCodePages))
	{
		// Преобразуем номер таблицы символов в строку
		string strCPName;
		strCPName.Format(L"%u", curItem->UserData);
		// Получаем текущее состояние флага в реестре
		int selectType = 0;
		GetRegKey(FavoriteCodePagesKey, strCPName, selectType, 0);
		// Удаляем/добавляем в ресестре информацию о выбранной кодовой странице
		if (select)
			SetRegKey(FavoriteCodePagesKey, strCPName, CPST_FAVORITE | (selectType & CPST_FIND ? CPST_FIND : 0));
		else if (selectType & CPST_FIND)
			SetRegKey(FavoriteCodePagesKey, strCPName, CPST_FIND);
		else
			DeleteRegValue(FavoriteCodePagesKey, strCPName);

		// Создаём новый элемент меню
		MenuItemEx newItem;
		newItem.Clear();
		newItem.strName = curItem->strName;
		newItem.UserData = (char *)(UINT_PTR)codePage;
		newItem.UserDataSize = sizeof(UINT);

		// Сохраняем позицию курсора
		int position=tables->GetSelectPos();
		// Удаляем старый пункт меню
		tables->DeleteItem(tables->GetSelectPos());
		// Добавляем пункт меню в новое сесто
		if (select)
		{
			// Добавляем разделитель, если выбранных кодовых страниц ещё не было
			// и после добавления останутся нормальные кодовые страницы
			if (!favoriteCodePages && normalCodePages>1)
				AddSeparator(tables->GetItemCount()-normalCodePages);
			// Ищем позицию, куда добавить элемент
			int newPosition = GetTableInsertPosition(
					codePage,
					tables->GetItemCount()-normalCodePages-favoriteCodePages,
					favoriteCodePages
				);
			// Добавляем кодовою страницу в выбранные
			tables->AddItem(&newItem, newPosition);
			// Удаляем разделитель, если нет обыкновынных кодовых страниц
			if (normalCodePages==1)
				tables->DeleteItem(tables->GetItemCount()-1);
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
				tables->DeleteItem(tables->GetItemCount()-normalCodePages-1);
			// Переносим элемент в нормальные таблицы, только если они показываются
			if (!Opt.CPMenuMode)
			{
				// Добавляем разделитель, если не было ни одной нормальной кодовой страницы
				if (!normalCodePages)
					AddSeparator();
				// Добавляем кодовою страницу в нормальные
				tables->AddItem(
						&newItem,
						GetTableInsertPosition(
								codePage,
								tables->GetItemCount()-normalCodePages,
								normalCodePages
							)
					);
				normalCodePages++;
			}
			// Если в режиме скрытия нормальных таблиц мы удалили последнюю выбранную таблицу, то удаляем и разделитель
			else if (favoriteCodePages==1)
				tables->DeleteItem(tables->GetItemCount()-normalCodePages-1);
			favoriteCodePages--;
			if (position==tables->GetItemCount()-normalCodePages-1)
				position--;
		}
		// Устанавливаем позицию в меню
		tables->AdjustSelectPos();
		tables->SetSelectPos(position>=tables->GetItemCount() ? tables->GetItemCount()-1 : position, 1);
		// Показываем меню
		if (Opt.CPMenuMode)
			tables->SetPosition(-1, -1, 0, 0);
		tables->Show();
	}
}

// Заполняем меню выбора таблиц символов
void FillTablesVMenu(bool bShowUnicode, bool bShowUTF)
{
	// Сохраняем выбранную таблицу символов
	UINT codePage = currentCodePage;
	if (tables->GetSelectPos()!=-1 && tables->GetSelectPos()<tables->GetItemCount()-normalCodePages)
		currentCodePage = (UINT)(UINT_PTR)tables->GetItemPtr()->UserData;
	// Очищаем меню
	favoriteCodePages = normalCodePages = 0;
	tables->DeleteItems();
	// Устанавливаем заголовок меню
	UnicodeString title = MSG(MGetTableTitle);
	if (Opt.CPMenuMode)
		title += L"*";
	tables->SetTitle(title);
	// Добавляем таблицы символов
	AddTables(::OEM | ::ANSI | (bShowUTF ? /* BUBUG ::UTF7 | */ ::UTF8 : 0) | (bShowUnicode ? (::UTF16BE | ::UTF16LE) : 0));
	// Восстанавливаем оригинальню таблицу символов
	currentCodePage = codePage;
	// Позиционируем меню
	tables->SetPosition(-1, -1, 0, 0);
	// Показываем меню
	tables->Show();
}

UINT GetTableEx(UINT nCurrent, bool bShowUnicode, bool bShowUTF)
{
	currentCodePage = nCurrent;
	// Создаём меню
	tables = new VMenu(L"", NULL, 0, ScrY-4);
	tables->SetBottomTitle(MSG(MGetTableBottomTitle));
	tables->SetFlags(VMENU_WRAPMODE|VMENU_AUTOHIGHLIGHT);
	// Добавляем таблицы символов
	FillTablesVMenu(bShowUnicode, bShowUTF);
	// Показываем меню
	tables->Show();
	// Цикл обработки сообщений меню
	while (!tables->Done())
	{
		switch (tables->ReadInput())
		{
			// Обработка скрытия/показа системных таблиц символов
			case KEY_CTRLH:
				Opt.CPMenuMode = !Opt.CPMenuMode;
				FillTablesVMenu(bShowUnicode, bShowUTF);
				break;
			// Обработка удаления таблицы символов из списка выбранных
			case KEY_DEL:
			case KEY_NUMDEL:
				ProcessSelected(false);
				break;
			// Обработка добавления таблицы символов в список выбранных
			case KEY_INS:
			case KEY_NUMPAD0:
				ProcessSelected(true);
				break;
			default:
				tables->ProcessInput();
				break;
		}
	}

	// Получаем выбранную таблицу символов
	UINT codePage = tables->Modal::GetExitCode() >= 0 ? (UINT)(UINT_PTR)tables->GetUserData(NULL, 0) : (UINT)-1;

	delete tables;
	tables = NULL;

	return codePage;
}

// Заполняем список таблицами символов
UINT AddCodepagesToList(HANDLE dialogHandle, UINT controlId, UINT codePage, bool allowAuto, bool allowAll)
{
	// Устанавливаем переменные для доступа из каллбака
	dialog = dialogHandle;
	control = controlId;
	currentCodePage = codePage;
	favoriteCodePages = normalCodePages = 0;
	selectedCodePages = !allowAuto && allowAll;
	// Добавляем стндартные элементы в список
	AddTables((allowAuto ? ::Auto : 0) | (allowAll ? ::SearchAll : 0) | ::AllStandard);
	// Возвращаем число любимых таблиц символов
	return favoriteCodePages;
}
