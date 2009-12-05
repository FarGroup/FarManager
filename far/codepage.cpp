/*
codepage.cpp

Работа с кодовыми страницами
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

#include "codepage.hpp"
#include "lang.hpp"
#include "vmenu.hpp"
#include "savefpos.hpp"
#include "keys.hpp"
#include "registry.hpp"
#include "language.hpp"
#include "dialog.hpp"
#include "interf.hpp"
#include "config.hpp"

// Ключ где хранятся имена кодовых страниц
const wchar_t *NamesOfCodePagesKey = L"CodePages\\Names";

const wchar_t *FavoriteCodePagesKey = L"CodePages\\Favorites";

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

// Источник вызова каллбака прохода по кодовым страницам
enum CodePagesCallbackCallSource
{
	CodePageSelect,
	CodePagesFill,
	CodePageCheck
};

// Диалог
static HANDLE dialog;
// Идентифкатор диалога
static UINT control;
// Меню
static VMenu *CodePages = NULL;
// Текущая таблица символов
static UINT currentCodePage;
// Количество выбранных и обыкновенных таблиц символов
static int favoriteCodePages, normalCodePages;
// Признак необходимости отображать таблицы символов для поиска
static bool selectedCodePages;
// Источник вызова каллбака для функции EnumSystemCodePages
static CodePagesCallbackCallSource CallbackCallSource;
// Признак того, что кодовая страница поддерживается
static bool CodePageSupported;

// Добавляем таблицу символов
void AddCodePage(const wchar_t *codePageName, UINT codePage, int position = -1, bool enabled = true, bool checked = false)
{
	if (CallbackCallSource == CodePagesFill)
	{
		// Вычисляем позицию вставляемого элемента
		if (position==-1)
		{
			FarListInfo info;
			SendDlgMessage(dialog, DM_LISTINFO, control, (LONG_PTR)&info);
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

		if (selectedCodePages && checked)
		{
			item.Item.Flags |= MIF_CHECKED;
		}

		if (!enabled)
		{
			item.Item.Flags |= MIF_GRAYED;
		}

		SendDlgMessage(dialog, DM_LISTINSERT, control, (LONG_PTR)&item);
		// Устанавливаем данные для элемента
		FarListItemData data;
		data.Index = position;
		data.Data = (void*)(DWORD_PTR)codePage;
		data.DataSize = sizeof(UINT);
		SendDlgMessage(dialog, DM_LISTSETDATA, control, (LONG_PTR)&data);
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
			CodePages->AddItem(&item, position);
		else
			CodePages->AddItem(&item);

		// Если надо позиционируем курсор на добавленный элемент
		if (currentCodePage==codePage && (CodePages->GetSelectPos()==-1 || (UINT)(UINT_PTR)CodePages->GetItemPtr()->UserData!=codePage))
			CodePages->SetSelectPos(position>=0?position:CodePages->GetItemCount()-1, 1);
		// Корректируем позицию выбранного элемента
		else if (position!=-1 && CodePages->GetSelectPos()>=position)
			CodePages->SetSelectPos(CodePages->GetSelectPos()+1, 1);
	}
}

// Добавляем стандартную таблицу символов
void AddStandardCodePage(const wchar_t *codePageName, UINT codePage, int position = -1, bool enabled = true)
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

	AddCodePage(codePageName, codePage, position, enabled, checked);
}

// Добавляем разделитель
void AddSeparator(LPCWSTR Label=NULL,int position = -1)
{
	if (CallbackCallSource == CodePagesFill)
	{
		if (position==-1)
		{
			FarListInfo info;
			SendDlgMessage(dialog, DM_LISTINFO, control, (LONG_PTR)&info);
			position = info.ItemsNumber;
		}

		FarListInsert item = {position};
		item.Item.Text = Label;
		item.Item.Flags = LIF_SEPARATOR;
		SendDlgMessage(dialog, DM_LISTINSERT, control, (LONG_PTR)&item);
	}
	else
	{
		MenuItemEx item;
		item.Clear();
		item.strName = Label;
		item.Flags = MIF_SEPARATOR;

		if (position>=0)
			CodePages->AddItem(&item, position);
		else
			CodePages->AddItem(&item);

		// Корректируем позицию выбранного элемента
		if (position!=-1 && CodePages->GetSelectPos()>=position)
			CodePages->SetSelectPos(CodePages->GetSelectPos()+1, 1);
	}
}

// Получаем количество элементов в списке
int GetItemsCount()
{
	if (CallbackCallSource == CodePageSelect)
	{
		return CodePages->GetItemCount();
	}
	else
	{
		FarListInfo info;
		SendDlgMessage(dialog, DM_LISTINFO, control, (LONG_PTR)&info);
		return info.ItemsNumber;
	}
}

// Получаем позицию для вставки таблицы с учётом сортировки по номеру кодовой страницы
int GetCodePageInsertPosition(UINT codePage, int start, int length)
{
	for (int position=start; position < start+length; position++)
	{
		UINT itemCodePage;

		if (CallbackCallSource == CodePageSelect)
			itemCodePage = (UINT)(UINT_PTR)CodePages->GetItemPtr(position)->UserData;
		else
			itemCodePage = (UINT)(UINT_PTR)SendDlgMessage(dialog, DM_LISTGETDATA, control, position);

		if (itemCodePage >= codePage)
			return position;
	}

	return start+length;
}

// Callback-функция получения таблиц символов
BOOL __stdcall EnumCodePagesProc(const wchar_t *lpwszCodePage)
{
	UINT codePage = _wtoi(lpwszCodePage);

	// Для функции проверки нас не интеерсует информация о кодовых страницах отличных от проверяемой
	if (CallbackCallSource == CodePageCheck && codePage != currentCodePage)
		return TRUE;

	// BUBBUG: Существует много кодировок с cpiex.MaxCharSize > 1, пока их не поддерживаем
	CPINFOEX cpiex;

	if (!GetCPInfoEx(codePage, 0, &cpiex))
	{
		// GetCPInfoEx возвращает ошибку для кодовых страниц без имени (например 1125), которые
		// сами по себе работают. Так что, прежде чем пропускать кодовую страницу из-за ошибки,
		// пробуем получить для неё информауию через GetCPInfo
		CPINFO cpi;

		if (!GetCPInfo(codePage, &cpi))
			return CallbackCallSource == CodePageCheck ? FALSE : TRUE;

		cpiex.MaxCharSize = cpi.MaxCharSize;
		cpiex.CodePageName[0] = L'\0';
	}

	// BUBUG: Пока не поддерживаем многобайтовые кодовые страницы
	if (cpiex.MaxCharSize != 1)
		return CallbackCallSource == CodePageCheck ? FALSE : TRUE;

	// Для функции провепки поддерживаемости кодовый страницы мы прошли все проверки и можем выходить
	if (CallbackCallSource == CodePageCheck)
	{
		CodePageSupported = true;
		return FALSE;
	}

	// Формируем имя таблиц символов
	wchar_t *codePageName = FormatCodePageName(_wtoi(lpwszCodePage), cpiex.CodePageName, sizeof(cpiex.CodePageName)/sizeof(wchar_t));
	// Получаем признак выбранности таблицы символов
	int selectType = 0;
	GetRegKey(FavoriteCodePagesKey, lpwszCodePage, selectType, 0);

	// Добавляем таблицу символов либо в нормальные, либо в выбранные таблицы симовлов
	if (selectType & CPST_FAVORITE)
	{
		// Если надо добавляем разделитель между выбранными и нормальными таблицами симовлов
		if (!favoriteCodePages)
			AddSeparator(MSG(MGetCodePageFavorites),GetItemsCount()-normalCodePages-(normalCodePages?1:0));

		// Добавляем таблицу символов в выбранные
		AddCodePage(
		    codePageName,
		    codePage,
		    GetCodePageInsertPosition(
		        codePage,
		        GetItemsCount()-normalCodePages-favoriteCodePages-(normalCodePages?1:0),
		        favoriteCodePages
		    ),
		    true,
		    selectType & CPST_FIND ? true : false
		);
		// Увеличиваем счётчик выбранных таблиц символов
		favoriteCodePages++;
	}
	else if (CallbackCallSource == CodePagesFill || !Opt.CPMenuMode)
	{
		// добавляем разделитель между стандартными и системными таблицами символов
		if (!favoriteCodePages && !normalCodePages)
			AddSeparator(MSG(MGetCodePageOther));

		// Добавляем таблицу символов в нормальные
		AddCodePage(
		    codePageName,
		    codePage,
		    GetCodePageInsertPosition(
		        codePage,
		        GetItemsCount()-normalCodePages,
		        normalCodePages
		    )
		);
		// Увеличиваем счётчик выбранных таблиц символов
		normalCodePages++;
	}

	return TRUE;
}

void AddCodePages(DWORD codePages)
{
	// Добавляем стандартные таблицы символов
	AddStandardCodePage((codePages & ::SearchAll) ? MSG(MFindFileAllCodePages) : MSG(MEditOpenAutoDetect), CP_AUTODETECT, -1, (codePages & ::SearchAll) || (codePages & ::Auto));
	AddSeparator(MSG(MGetCodePageSystem));
	AddStandardCodePage(L"OEM", GetOEMCP(), -1, (codePages & ::OEM)?1:0);
	AddStandardCodePage(L"ANSI", GetACP(), -1, (codePages & ::ANSI)?1:0);
	AddSeparator(MSG(MGetCodePageUnicode));
	AddStandardCodePage(L"UTF-7", CP_UTF7, -1, (codePages & ::UTF7)?1:0);
	AddStandardCodePage(L"UTF-8", CP_UTF8, -1, (codePages & ::UTF8)?1:0);
	AddStandardCodePage(L"UTF-16 (Little endian)", CP_UNICODE, -1, (codePages & ::UTF16LE)?1:0);
	AddStandardCodePage(L"UTF-16 (Big endian)", CP_REVERSEBOM, -1, (codePages & ::UTF16BE)?1:0);
	// Получаем таблицы символов установленные в системе
	EnumSystemCodePages((CODEPAGE_ENUMPROCW)EnumCodePagesProc, CP_INSTALLED);
}

// Обработка добавления/удаления в/из список выбранных таблиц символов
void ProcessSelected(bool select)
{
	if (Opt.CPMenuMode && select)
		return;

	MenuItemEx *curItem = CodePages->GetItemPtr();
	UINT itemPosition = CodePages->GetSelectPos();
	UINT itemCount = CodePages->GetItemCount();
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
		int position=CodePages->GetSelectPos();
		// Удаляем старый пункт меню
		CodePages->DeleteItem(CodePages->GetSelectPos());

		// Добавляем пункт меню в новое сесто
		if (select)
		{
			// Добавляем разделитель, если выбранных кодовых страниц ещё не было
			// и после добавления останутся нормальные кодовые страницы
			if (!favoriteCodePages && normalCodePages>1)
				AddSeparator(MSG(MGetCodePageFavorites),CodePages->GetItemCount()-normalCodePages);

			// Ищем позицию, куда добавить элемент
			int newPosition = GetCodePageInsertPosition(
			                      codePage,
			                      CodePages->GetItemCount()-normalCodePages-favoriteCodePages,
			                      favoriteCodePages
			                  );
			// Добавляем кодовою страницу в выбранные
			CodePages->AddItem(&newItem, newPosition);

			// Удаляем разделитель, если нет обыкновынных кодовых страниц
			if (normalCodePages==1)
				CodePages->DeleteItem(CodePages->GetItemCount()-1);

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
				CodePages->DeleteItem(CodePages->GetItemCount()-normalCodePages-2);

			// Переносим элемент в нормальные таблицы, только если они показываются
			if (!Opt.CPMenuMode)
			{
				// Добавляем разделитель, если не было ни одной нормальной кодовой страницы
				if (!normalCodePages)
					AddSeparator(MSG(MGetCodePageOther));

				// Добавляем кодовою страницу в нормальные
				CodePages->AddItem(
				    &newItem,
				    GetCodePageInsertPosition(
				        codePage,
				        CodePages->GetItemCount()-normalCodePages,
				        normalCodePages
				    )
				);
				normalCodePages++;
			}
			// Если в режиме скрытия нормальных таблиц мы удалили последнюю выбранную таблицу, то удаляем и разделитель
			else if (favoriteCodePages==1)
				CodePages->DeleteItem(CodePages->GetItemCount()-normalCodePages-1);

			favoriteCodePages--;

			if (position==CodePages->GetItemCount()-normalCodePages-1)
				position--;
		}

		// Устанавливаем позицию в меню
		CodePages->SetSelectPos(position>=CodePages->GetItemCount() ? CodePages->GetItemCount()-1 : position, 1);

		// Показываем меню
		if (Opt.CPMenuMode)
			CodePages->SetPosition(-1, -1, 0, 0);

		CodePages->Show();
	}
}

// Заполняем меню выбора таблиц символов
void FillCodePagesVMenu(bool bShowUnicode, bool bShowUTF)
{
	// Сохраняем выбранную таблицу символов
	UINT codePage = currentCodePage;

	if (CodePages->GetSelectPos()!=-1 && CodePages->GetSelectPos()<CodePages->GetItemCount()-normalCodePages)
		currentCodePage = (UINT)(UINT_PTR)CodePages->GetItemPtr()->UserData;

	// Очищаем меню
	favoriteCodePages = normalCodePages = 0;
	CodePages->DeleteItems();
	// Устанавливаем заголовок меню
	UnicodeString title = MSG(MGetCodePageTitle);

	if (Opt.CPMenuMode)
		title += L" *";

	CodePages->SetTitle(title);
	// Добавляем таблицы символов
	AddCodePages(::OEM | ::ANSI | (bShowUTF ? /* BUBUG ::UTF7 | */ ::UTF8 : 0) | (bShowUnicode ? (::UTF16BE | ::UTF16LE) : 0));
	// Восстанавливаем оригинальню таблицу символов
	currentCodePage = codePage;
	// Позиционируем меню
	CodePages->SetPosition(-1, -1, 0, 0);
	// Показываем меню
	CodePages->Show();
}

// Форматируем имя таблицы символов
wchar_t *FormatCodePageName(UINT CodePage, wchar_t *CodePageName, size_t Length)
{
	if (!CodePageName || !Length)
		return CodePageName;

	// Формируем имя таблиц символов
	if (!*CodePageName)
	{
		string strCodePage;
		strCodePage.Format(L"%u", CodePage);
		// Если имя не задано, то пытаемся получить его из Far2\CodePages\Names
		string strCodePageName;
		GetRegKey(NamesOfCodePagesKey, strCodePage, strCodePageName, L"");
		Length = Min(Length-1, strCodePageName.GetLength());
		wmemcpy(CodePageName, strCodePageName, Length);
		CodePageName[Length] = L'\0';
		return CodePageName;
	}
	else
	{
		// Под виндой на входе "XXXX (Name)", а, например, под wine просто "Name"
		wchar_t *Name = wcschr(CodePageName, L'(');

		if (Name && *(++Name))
		{
			Name[wcslen(Name)-1] = L'\0';
			return Name;
		}
		else
		{
			return CodePageName;
		}
	}
}

UINT SelectCodePage(UINT nCurrent, bool bShowUnicode, bool bShowUTF)
{
	CallbackCallSource = CodePageSelect;
	currentCodePage = nCurrent;
	// Создаём меню
	CodePages = new VMenu(L"", NULL, 0, ScrY-4);
	CodePages->SetBottomTitle(MSG(!Opt.CPMenuMode?MGetCodePageBottomTitle:MGetCodePageBottomShortTitle));
	CodePages->SetFlags(VMENU_WRAPMODE|VMENU_AUTOHIGHLIGHT);
	CodePages->SetHelp(L"CodePagesMenu");
	// Добавляем таблицы символов
	FillCodePagesVMenu(bShowUnicode, bShowUTF);
	// Показываем меню
	CodePages->Show();

	// Цикл обработки сообщений меню
	while (!CodePages->Done())
	{
		switch (CodePages->ReadInput())
		{
				// Обработка скрытия/показа системных таблиц символов
			case KEY_CTRLH:
				Opt.CPMenuMode = !Opt.CPMenuMode;
				CodePages->SetBottomTitle(MSG(!Opt.CPMenuMode?MGetCodePageBottomTitle:MGetCodePageBottomShortTitle));
				FillCodePagesVMenu(bShowUnicode, bShowUTF);
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
				CodePages->ProcessInput();
				break;
		}
	}

	// Получаем выбранную таблицу символов
	UINT codePage = CodePages->Modal::GetExitCode() >= 0 ? (UINT)(UINT_PTR)CodePages->GetUserData(NULL, 0) : (UINT)-1;
	delete CodePages;
	CodePages = NULL;
	return codePage;
}

// Заполняем список таблицами символов
UINT FillCodePagesList(HANDLE dialogHandle, UINT controlId, UINT codePage, bool allowAuto, bool allowAll)
{
	CallbackCallSource = CodePagesFill;
	// Устанавливаем переменные для доступа из каллбака
	dialog = dialogHandle;
	control = controlId;
	currentCodePage = codePage;
	favoriteCodePages = normalCodePages = 0;
	selectedCodePages = !allowAuto && allowAll;
	// Добавляем стндартные элементы в список
	AddCodePages((allowAuto ? ::Auto : 0) | (allowAll ? ::SearchAll : 0) | ::AllStandard);

	if (CallbackCallSource == CodePagesFill)
	{
		// Если надо выбираем элемент
		FarListInfo info;
		SendDlgMessage(dialogHandle, DM_LISTINFO, control, (LONG_PTR)&info);

		for (int i=0; i<info.ItemsNumber; i++)
		{
			if (static_cast<UINT>(SendDlgMessage(dialogHandle,DM_LISTGETDATA,controlId,i))==codePage)
			{
				FarListGetItem Item={i};
				SendDlgMessage(dialog, DM_LISTGETITEM,controlId,reinterpret_cast<LONG_PTR>(&Item));
				SendDlgMessage(dialog, DM_SETTEXTPTR,controlId,reinterpret_cast<LONG_PTR>(Item.Item.Text));
				FarListPos Pos={i,-1};
				SendDlgMessage(dialogHandle,DM_LISTSETCURPOS,controlId,reinterpret_cast<LONG_PTR>(&Pos));
				break;
			}
		}
	}

	// Возвращаем число любимых таблиц символов
	return favoriteCodePages;
}

bool IsCodePageSupported(UINT CodePage)
{
	// Для стандартных кодовых страниц ничего проверять не надо
	// BUGBUG: мы не везде поддержиаем все стандартные кодовые страницы. Это не проверяется
	if (CodePage == CP_AUTODETECT || IsStandardCodePage(CodePage))
		return true;

	// Проходим по всем кодовым страницам системы и проверяем поддерживаем мы или нет её
	CallbackCallSource = CodePageCheck;
	currentCodePage = CodePage;
	CodePageSupported = false;
	EnumSystemCodePages((CODEPAGE_ENUMPROCW)EnumCodePagesProc, CP_INSTALLED);
	return CodePageSupported;
}
