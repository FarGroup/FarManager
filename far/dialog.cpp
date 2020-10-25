/*
dialog.cpp

Класс диалога
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
#include "dialog.hpp"

// Internal:
#include "keyboard.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "dlgedit.hpp"
#include "help.hpp"
#include "scrbuf.hpp"
#include "manager.hpp"
#include "savescr.hpp"
#include "constitle.hpp"
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "taskbar.hpp"
#include "interf.hpp"
#include "strmix.hpp"
#include "history.hpp"
#include "uuids.far.hpp"
#include "colormix.hpp"
#include "mix.hpp"
#include "plugins.hpp"
#include "lang.hpp"
#include "uuids.far.dialogs.hpp"
#include "string_utils.hpp"
#include "config.hpp"
#include "edit.hpp"
#include "global.hpp"

// Platform:
#include "platform.chrono.hpp"
#include "platform.env.hpp"
#include "platform.memory.hpp"

// Common:
#include "common.hpp"
#include "common/algorithm.hpp"
#include "common/singleton.hpp"
#include "common/uuid.hpp"
#include "common/view/zip.hpp"

// External:

//----------------------------------------------------------------------------

// Флаги для функции ConvertItem
enum CVTITEMFLAGS
{
	CVTITEM_FROMPLUGIN      = 1,
	CVTITEM_TOPLUGINSHORT   = 2,
	CVTITEM_FROMPLUGINSHORT = 3
};

enum DLGITEMINTERNALFLAGS
{
	DLGIIF_COMBOBOXNOREDRAWEDIT     = 3_bit, // не прорисовывать строку редактирования при изменениях в комбо
};

struct DlgUserControl
{
	COORD CursorPos  {-1, -1};
	bool CursorVisible {};
	DWORD CursorSize {DWORD(-1)};
};

//////////////////////////////////////////////////////////////////////////
/*
   Функция, определяющая - "Может ли элемент диалога иметь фокус ввода"
*/
static bool CanGetFocus(int Type)
{
	switch (Type)
	{
		case DI_EDIT:
		case DI_FIXEDIT:
		case DI_PSWEDIT:
		case DI_COMBOBOX:
		case DI_BUTTON:
		case DI_CHECKBOX:
		case DI_RADIOBUTTON:
		case DI_LISTBOX:
		case DI_MEMOEDIT:
		case DI_USERCONTROL:
			return true;
		default:
			return false;
	}
}

static bool IsEmulatedEditorLine(const DialogItemEx& Item)
{
	return Item.Type == DI_EDIT && Item.Flags & DIF_EDITOR;
}

bool IsKeyHighlighted(string_view const Str, int const Key, bool const Translate, wchar_t CharKey)
{
	if (!CharKey)
	{
		if (!HiTextHotkey(Str, CharKey))
			return false;
	}

	const auto UpperStrKey = upper(CharKey);

	if (Key < 0xFFFF)
	{
		return UpperStrKey == static_cast<int>(upper(Key)) || (Translate && KeyToKeyLayoutCompare(Key,UpperStrKey));
	}

	if (Key&(KEY_ALT|KEY_RALT))
	{
		const auto AltKey = Key & ~(KEY_ALT | KEY_RALT);

		if (AltKey < 0xFFFF)
		{
			if (std::iswdigit(AltKey))
				return AltKey==UpperStrKey;

			if (static_cast<unsigned int>(AltKey) > L' ')
				//         (AltKey=='-'  || AltKey=='/' || AltKey==','  || AltKey=='.' ||
				//          AltKey=='\\' || AltKey=='=' || AltKey=='['  || AltKey==']' ||
				//          AltKey==':'  || AltKey=='"' || AltKey=='~'))
			{
				return UpperStrKey == static_cast<int>(upper(AltKey)) || (Translate && KeyToKeyLayoutCompare(AltKey, UpperStrKey));
			}
		}
	}

	return false;
}

static void ConvertItemSmall(const DialogItemEx& From, FarDialogItem& To)
{
	To = static_cast<const FarDialogItem&>(From);

	To.Data = nullptr;
	To.History = nullptr;
	To.Mask = nullptr;
	To.Reserved0 = From.Reserved0;
	To.UserData = From.UserData;
}

static string_view ItemString(const DialogItemEx *Data)
{
	string_view Str = Data->strData;

	if (IsEdit(Data->Type))
	{
		if (const auto EditPtr = static_cast<const DlgEdit*>(Data->ObjPtr))
			Str = EditPtr->GetString();
	}

	const auto sz = Str.size();

	if (sz > Data->MaxLength && Data->MaxLength > 0)
		Str.remove_suffix(sz - Data->MaxLength);

	return Str;
}

static size_t ConvertItemEx2(const DialogItemEx *ItemEx, FarGetDialogItem *Item)
{
	auto size = aligned_sizeof<FarDialogItem>();
	const auto offsetList = size;
	auto offsetListItems = size;
	vmenu_ptr ListBox;
	size_t ListBoxSize = 0;
	if (ItemEx->Type==DI_LISTBOX || ItemEx->Type==DI_COMBOBOX)
	{
		ListBox=ItemEx->ListPtr;
		if (ListBox)
		{
			size += aligned_sizeof<FarList>();
			offsetListItems=size;
			ListBoxSize=ListBox->size();
			size+=ListBoxSize*sizeof(FarListItem);
			for(size_t ii=0;ii != ListBoxSize; ++ii)
			{
				size += (ListBox->at(ii).Name.size() + 1) * sizeof(wchar_t);
			}
		}
	}
	const auto offsetStrings = size;
	const auto str = ItemString(ItemEx);
	size += (str.size() + 1) * sizeof(wchar_t);
	size+=(ItemEx->strHistory.size()+1)*sizeof(wchar_t);
	size+=(ItemEx->strMask.size()+1)*sizeof(wchar_t);

	if (Item)
	{
		if(Item->Item && Item->Size >= size)
		{
			ConvertItemSmall(*ItemEx, *Item->Item);
			if (ListBox)
			{
				const auto list = static_cast<FarList*>(static_cast<void*>(reinterpret_cast<char*>(Item->Item) + offsetList));
				const auto listItems = static_cast<FarListItem*>(static_cast<void*>(reinterpret_cast<char*>(Item->Item) + offsetListItems));
				auto text = static_cast<wchar_t*>(static_cast<void*>(listItems + ListBoxSize));
				for(size_t ii = 0; ii != ListBoxSize; ++ii)
				{
					auto& item = ListBox->at(ii);
					listItems[ii].Flags=item.Flags;
					listItems[ii].Text=text;
					text += item.Name.copy(text, item.Name.npos);
					*text++ = {};
					listItems[ii].UserData = item.SimpleUserData;
					listItems[ii].Reserved = 0;
				}
				list->StructSize=sizeof(*list);
				list->ItemsNumber=ListBoxSize;
				list->Items=listItems;
				Item->Item->ListItems=list;
			}
			auto p = static_cast<wchar_t*>(static_cast<void*>(reinterpret_cast<char*>(Item->Item) + offsetStrings));
			Item->Item->Data = p;
			p += str.copy(p, str.npos);
			*p++ = {};
			Item->Item->History = p;
			p += ItemEx->strHistory.copy(p, ItemEx->strHistory.npos);
			*p++ = {};
			Item->Item->Mask = p;
			p += ItemEx->strMask.copy(p, ItemEx->strMask.npos);
			*p++ = {};
		}
	}
	return size;
}

void ItemsToItemsEx(span<const FarDialogItem> const Items, span<DialogItemEx> const ItemsEx, bool const Short)
{
	for (const auto& [Item, ItemEx]: zip(Items, ItemsEx))
	{
		static_cast<FarDialogItem&>(ItemEx) = Item;

		if(!Short)
		{
			ItemEx.strHistory = NullToEmpty(Item.History);
			ItemEx.strMask = NullToEmpty(Item.Mask);
			if(Item.Data)
			{
				auto Length = wcslen(Item.Data);
				if (Item.MaxLength && Item.MaxLength < Length)
					Length = Item.MaxLength;
				ItemEx.strData.assign(Item.Data, Length);
			}
		}

		ItemEx.X2 = std::max(ItemEx.X1, ItemEx.X2);
		ItemEx.Y2 = std::max(ItemEx.Y1, ItemEx.Y2);

		if ((ItemEx.Type == DI_COMBOBOX || ItemEx.Type == DI_LISTBOX) && !os::memory::is_pointer(Item.ListItems))
		{
			ItemEx.ListItems=nullptr;
		}
	}
}

std::vector<DialogItemEx> MakeDialogItems(span<const InitDialogItem> Items)
{
	std::vector<DialogItemEx> ItemsEx(Items.size());

	for (const auto& [Item, ItemEx]: zip(Items, ItemsEx))
	{
		ItemEx.Type = Item.Type;
		ItemEx.X1 = Item.Position.TopLeft.x;
		ItemEx.X2 = Item.Position.BottomRight.x;
		ItemEx.Y1 = Item.Position.TopLeft.y;
		ItemEx.Y2 = Item.Position.BottomRight.y;
		ItemEx.Flags = Item.Flags;
		ItemEx.strData = Item.Data;
	}

	return ItemsEx;
}

static intptr_t DefProcFunction(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	return Dlg->DefProc(Msg, Param1, Param2);
}

// Структура, описывающая автоматизацию для DIF_AUTOMATION
// на первом этапе - примитивная - выставление флагов у элементов для CheckBox
struct DialogItemEx::DialogItemAutomation
{
	DialogItemEx* Owner;                    // Для этого элемента...
	FARDIALOGITEMFLAGS Flags[3][2];          // ...выставить вот эти флаги
	// [0] - Unchecked, [1] - Checked, [2] - 3Checked
	// [][0] - Set, [][1] - Skip
};

DialogItemEx::DialogItemEx():
	FarDialogItem(),
	ListPos(),
	ObjPtr(),
	UCData()
{}

DialogItemEx::~DialogItemEx() = default;

DialogItemEx::DialogItemEx(const DialogItemEx&) = default;
DialogItemEx& DialogItemEx::operator=(const DialogItemEx&) = default;

DialogItemEx::DialogItemEx(DialogItemEx&&) noexcept = default;
DialogItemEx& DialogItemEx::operator=(DialogItemEx&&) noexcept = default;


bool DialogItemEx::AddAutomation(DialogItemEx* DlgItem,
	FARDIALOGITEMFLAGS UncheckedSet, FARDIALOGITEMFLAGS UncheckedSkip,
	FARDIALOGITEMFLAGS CheckedSet, FARDIALOGITEMFLAGS CheckedSkip,
	FARDIALOGITEMFLAGS Checked3Set, FARDIALOGITEMFLAGS Checked3Skip)
{
	DialogItemAutomation Item;
	Item.Owner = DlgItem;
	Item.Flags[0][0] = UncheckedSet;
	Item.Flags[0][1] = UncheckedSkip;
	Item.Flags[1][0] = CheckedSet;
	Item.Flags[1][1] = CheckedSkip;
	Item.Flags[2][0] = Checked3Set;
	Item.Flags[2][1] = Checked3Skip;
	Auto.emplace_back(Item);
	return true;
}


void Dialog::Construct(span<DialogItemEx> const SrcItems)
{
	_DIALOG(CleverSysLog CL(L"Dialog::Construct() 1"));
	SavedItems = SrcItems.data();

	Items.resize(SrcItems.size());
	Items.assign(ALL_CONST_RANGE(SrcItems));

	// Items[i].Auto.Owner points to SrcItems, we need to update:
	for (const auto& [Item, SrcItem]: zip(Items, SrcItems))
	{
		for (const auto& [ItemAuto, SrcItemAuto]: zip(Item.Auto, SrcItem.Auto))
		{
			// TODO: P1091R3
			const auto SrcItemIterator = std::find_if(ALL_CONST_RANGE(SrcItems), [&SrcItemAuto = SrcItemAuto](const DialogItemEx& i)
			{
				return &i == SrcItemAuto.Owner;
			});
			ItemAuto.Owner = &Items[SrcItemIterator - SrcItems.begin()];
		}
	}

	Init();
}

void Dialog::Construct(span<const FarDialogItem> const SrcItems)
{
	_DIALOG(CleverSysLog CL(L"Dialog::Construct() 2"));
	SavedItems = nullptr;

	Items.resize(SrcItems.size());
	//BUGBUG add error check
	ItemsToItemsEx(SrcItems, Items);
	Init();
}

void Dialog::Init()
{
	m_ConsoleTitle = ConsoleTitle::GetTitle();
	_DIALOG(CleverSysLog CL(L"Dialog::Init()"));
	AddToList();
	SetMacroMode(MACROAREA_DIALOG);
	m_CanLoseFocus = false;
	//Номер плагина, вызвавшего диалог (-1 = Main)
	PluginOwner = nullptr;
	DialogMode.Set(DMODE_ISCANMOVE|DMODE_VISIBLE);
	SetDropDownOpened(FALSE);
	m_DisableRedraw=0;
	m_FocusPos=static_cast<size_t>(-1);
	PrevFocusPos=static_cast<size_t>(-1);

	// функция должна быть всегда!!!
	if (!m_handler)
	{
		m_handler = &DefProcFunction;
		// знать диалог в старом стиле - учтем этот факт!
		DialogMode.Set(DMODE_OLDSTYLE);
	}

	IdExist=false;
	m_Id = {};
	bInitOK = true;
}

//////////////////////////////////////////////////////////////////////////
/* Public, Virtual:
   Деструктор класса Dialog
*/
Dialog::~Dialog()
{
	_DIALOG(CleverSysLog CL(L"Dialog::~Dialog()"));
	_DIALOG(SysLog(L"[%p] Dialog::~Dialog()",this));
	DeleteDialogObjects();

	Dialog::Hide();
	if (Global)
	{
		if (Global->Opt->Clock && Global->WindowManager->IsPanelsActive(true))
			ShowTime();

		if (!CheckDialogMode(DMODE_ISMENU))
			Global->ScrBuf->Flush();
	}

//	INPUT_RECORD rec;
//	PeekInputRecord(&rec);
	RemoveFromList();
	_DIALOG(SysLog(L"Destroy Dialog"));
}

void Dialog::CheckDialogCoord()
{
	// задано центрирование диалога по горизонтали?
	// X2 при этом = ширине диалога.
	if (m_Where.left == -1)
	{
		m_Where.left = (ScrX - m_Where.right + 1) / 2;
		m_Where.right += m_Where.left - 1;
	}

	// задано центрирование диалога по вертикали?
	// Y2 при этом = высоте диалога.
	if (m_Where.top == -1)
	{
		m_Where.top = (ScrY - m_Where.bottom + 1) / 2;
		m_Where.bottom += m_Where.top - 1;
	}
}


void Dialog::InitDialog()
{
	_DIALOG(CleverSysLog CL(L"Dialog::InitDialog()"));

	if(Global->CloseFAR)
	{
		SetDialogMode(DMODE_NOPLUGINS);
	}

	if (!DialogMode.Check(DMODE_OBJECTS_INITED))      // самодостаточный вариант, когда
	{                      //  элементы инициализируются при первом вызове.
		CheckDialogCoord();
		InitDialogObjects();
		const auto Result = static_cast<int>(DlgProc(DN_INITDIALOG, GetDlgFocusPos(), DataDialog));

		if (m_ExitCode == -1)
		{
			if (Result)
			{
				// еще разок, т.к. данные могли быть изменены
				InitDialogObjects();
			}
		}

		// все объекты проинициализированы!
		DialogMode.Set(DMODE_OBJECTS_INITED);

		DlgProc(DN_GOTFOCUS, GetDlgFocusPos(), nullptr);
	}
}

//////////////////////////////////////////////////////////////////////////
/* Public, Virtual:
   Расчет значений координат окна диалога и вызов функции
   ScreenObjectWithShadow::Show() для вывода диалога на экран.
*/
void Dialog::Show()
{
	_DIALOG(CleverSysLog CL(L"Dialog::Show()"));
	_DIALOG(SysLog(L"[%p] Dialog::Show()",this));

	if (!DialogMode.Check(DMODE_OBJECTS_INITED) || !DialogMode.Check(DMODE_VISIBLE))
		return;

	if (DialogMode.Check(DMODE_RESIZED))
	{
		TPreRedrawFunc::instance()([](const PreRedrawItem& Item)
		{
			Item();
		});
	}

	DialogMode.Clear(DMODE_RESIZED);

	DialogMode.Set(DMODE_SHOW);
	ScreenObjectWithShadow::Show();
}

//  Цель перехвата данной функции - управление видимостью...
void Dialog::Hide()
{
	_DIALOG(CleverSysLog CL(L"Dialog::Hide()"));
	_DIALOG(SysLog(L"[%p] Dialog::Hide()",this));

	if (!DialogMode.Check(DMODE_OBJECTS_INITED))
		return;

	DialogMode.Clear(DMODE_SHOW);
	ScreenObjectWithShadow::Hide();
}

//////////////////////////////////////////////////////////////////////////
/* Private, Virtual:
   Инициализация объектов и вывод диалога на экран.
*/
void Dialog::DisplayObject()
{
	_DIALOG(CleverSysLog CL(L"Dialog::DisplayObject()"));

	if (DialogMode.Check(DMODE_SHOW))
	{
		ShowDialog();          // "нарисуем" диалог.
	}
}

// пересчитать координаты для элементов с DIF_CENTERGROUP
void Dialog::ProcessCenterGroup()
{
	FOR_RANGE(Items, i)
	{
		// Последовательно объявленные элементы с флагом DIF_CENTERGROUP
		// и одинаковой вертикальной позицией будут отцентрированы в диалоге.
		// Их координаты X не важны. Удобно использовать для центрирования
		// групп кнопок.

		const auto IsNotSuitableItem = [](const DialogItemEx& Item, int Y) { return !(Item.Flags & DIF_CENTERGROUP && Item.Y1 == Y); };
		const auto IsVisible = [](const DialogItemEx& Item) { return !(Item.Flags & DIF_HIDDEN); };

		if ((i->Flags & DIF_CENTERGROUP) && (i == Items.begin() || IsNotSuitableItem(*(i - 1), i->Y1)))
		{
			const auto ButtonsEnd = std::find_if(i, Items.end(), [&](const DialogItemEx& Item) { return IsNotSuitableItem(Item, i->Y1); });
			const auto FirstVisibleButton = std::find_if(i, ButtonsEnd, IsVisible);

			const auto GetIncrement = [this](const DialogItemEx& Item)
			{
				auto Result = LenStrItem(Item);
				if (!Item.strData.empty())
				{
					switch (Item.Type)
					{
					case DI_BUTTON:
						Result += 1;
						break;
					case DI_CHECKBOX:
					case DI_RADIOBUTTON:
						Result += 5;
						break;
					default:
						break;
					}
				}
				return Result;
			};

			int Length = 0;

			for (auto j = FirstVisibleButton; j != ButtonsEnd; ++j)
			{
				if (IsVisible(*j))
				{
					Length += GetIncrement(*j);
				}
			}

			if (Length && !FirstVisibleButton->strData.empty() && FirstVisibleButton->Type == DI_BUTTON)
				--Length;

			int StartX = std::max(0, (m_Where.width() - Length) / 2);

			for (auto j = FirstVisibleButton; j != ButtonsEnd; ++j)
			{
				if (IsVisible(*j))
				{
					j->X1 = StartX;
					StartX += GetIncrement(*j);
					j->X2 = StartX;
					if (j->X2 != j->X1)
					{
						--j->X2;
					}
				}
			}
		}
	}
}

intptr_t Dialog::GetListItemSimpleUserData(size_t ListId, size_t ItemId) const
{
	return Items[ListId].ListPtr->GetSimpleUserData(static_cast<int>(ItemId));
}

void Dialog::SetListItemComplexUserData(size_t ListId, size_t ItemId, const std::any& Data)
{
	Items[ListId].ListPtr->SetComplexUserData(Data, static_cast<int>(ItemId));
}

std::any* Dialog::GetListItemComplexUserData(size_t ListId, size_t ItemId)
{
	return Items[ListId].ListPtr->GetComplexUserData(static_cast<int>(ItemId));
}

//////////////////////////////////////////////////////////////////////////
/* Public:
   Инициализация элементов диалога.

   InitDialogObjects возвращает ID элемента с фокусом ввода
   Параметр - для выборочной реинициализации элементов. ID = -1 - касаемо всех объектов
*/
/*
  TODO: Необходимо применить ProcessRadioButton для исправления
        кривых рук некоторых плагинописателей (а надо?)
*/
void Dialog::InitDialogObjects(size_t ID)
{
	size_t InitItemCount;
	_DIALOG(CleverSysLog CL(L"Dialog::InitDialogObjects()"));
	bool AllElements = false;

	if (ID+1 > Items.size())
		return;

	if (ID == static_cast<size_t>(-1)) // инициализируем все?
	{
		AllElements = true;
		ID=0;
		InitItemCount=Items.size();
	}
	else
	{
		InitItemCount=ID+1;
	}

	//   если FocusPos в пределах и элемент задисаблен, то ищем сначала.
	if (m_FocusPos!=static_cast<size_t>(-1) && m_FocusPos < Items.size() &&
	        (Items[m_FocusPos].Flags&(DIF_DISABLE|DIF_NOFOCUS|DIF_HIDDEN)))
		m_FocusPos = static_cast<size_t>(-1); // будем искать сначала!

	// предварительный цикл по поводу кнопок
	for (size_t I = ID; I != InitItemCount; ++I)
	{
		auto& Item = Items[I];

		if (Item.Type==DI_BUTTON && Item.Flags&DIF_SETSHIELD)
		{
			static const auto Shield = L"\x2580\x2584 "sv;
			Item.strData.insert(0, Shield);
		}

		// для кнопок не имеющих стиля "Показывает заголовок кнопки без скобок"
		//  добавим энти самые скобки
		if (Item.Type==DI_BUTTON && !(Item.Flags & DIF_NOBRACKETS))
		{
			static const string_view Brackets[] =
			{
				L"[ "sv, L" ]"sv,
				L"{ "sv, L" }"sv,
			};

			const auto Start = Item.Flags&DIF_DEFAULTBUTTON? 2 : 0;
			if (!Item.strData.empty() && Item.strData.front() != Brackets[Start].front())
			{
				Item.strData.insert(0, Brackets[Start]);
				Item.strData.append(Brackets[Start + 1]);
			}
		}
		// предварительный поиск фокуса
		if (m_FocusPos == static_cast<size_t>(-1) &&
		        CanGetFocus(Item.Type) &&
		        (Item.Flags&DIF_FOCUS) &&
		        !(Item.Flags&(DIF_DISABLE|DIF_NOFOCUS|DIF_HIDDEN)))
			m_FocusPos=I; // запомним первый фокусный элемент

		Item.Flags&=~DIF_FOCUS; // сбросим для всех, чтобы не оказалось,
		//   что фокусов - как у дурачка фантиков

		// сбросим флаг DIF_CENTERGROUP для редакторов
		switch (Item.Type)
		{
			case DI_BUTTON:
			case DI_CHECKBOX:
			case DI_RADIOBUTTON:
			case DI_TEXT:
			case DI_VTEXT:  // ????
				break;

			default:
				Item.Flags &= ~DIF_CENTERGROUP;
		}
	}

	// Опять про фокус ввода - теперь, если "чудо" забыло выставить
	// хотя бы один, то ставим на первый подходящий
	if (m_FocusPos == static_cast<size_t>(-1))
	{
		const auto ItemIterator = std::find_if(CONST_RANGE(Items, i)
		{
			return CanGetFocus(i.Type) && !(i.Flags&(DIF_DISABLE|DIF_NOFOCUS|DIF_HIDDEN));
		});
		if (ItemIterator != Items.cend())
		{
			m_FocusPos= ItemIterator - Items.begin();
		}
	}

	if (m_FocusPos == static_cast<size_t>(-1)) // ну ни хрена себе - нет ни одного
	{                  //   элемента с возможностью фокуса
		m_FocusPos=0;     // убиться, блин
	}

	// ну вот и добрались до!
	Items[m_FocusPos].Flags|=DIF_FOCUS;
	// а теперь все сначала и по полной программе...
	ProcessCenterGroup(); // сначала отцентрируем

	for (size_t I = ID; I != InitItemCount; ++I)
	{
		auto& Item = Items[I];

		if (Item.Type==DI_LISTBOX)
		{
			if (!DialogMode.Check(DMODE_OBJECTS_CREATED))
			{
				Item.ListPtr = VMenu::create({}, {}, Item.Y2 - Item.Y1 + 1, VMENU_ALWAYSSCROLLBAR | VMENU_LISTBOX, std::static_pointer_cast<Dialog>(shared_from_this()));
			}

				auto& ListPtr = Item.ListPtr;
				ListPtr->SetVDialogItemID(I);
				/* $ 13.09.2000 SVS
				   + Флаг DIF_LISTNOAMPERSAND. По умолчанию для DI_LISTBOX &
				     DI_COMBOBOX выставляется флаг MENU_SHOWAMPERSAND. Этот флаг
				     подавляет такое поведение
				*/
				ListPtr->ChangeFlags(VMENU_DISABLED, (Item.Flags & DIF_DISABLE) != 0);
				ListPtr->ChangeFlags(VMENU_SHOWAMPERSAND, (Item.Flags & DIF_LISTNOAMPERSAND) == 0);
				ListPtr->ChangeFlags(VMENU_SHOWNOBOX, (Item.Flags & DIF_LISTNOBOX) != 0);
				ListPtr->ChangeFlags(VMENU_WRAPMODE, (Item.Flags & DIF_LISTWRAPMODE) != 0);
				ListPtr->ChangeFlags(VMENU_AUTOHIGHLIGHT, (Item.Flags & DIF_LISTAUTOHIGHLIGHT) != 0);
				ListPtr->ChangeFlags(VMENU_NOMERGEBORDER, (Item.Flags & DIF_LISTNOMERGEBORDER) != 0);

				if (Item.Flags & DIF_LISTAUTOHIGHLIGHT)
					ListPtr->AssignHighlights();

				ListPtr->SetDialogStyle(DialogMode.Check(DMODE_WARNINGSTYLE));
				ListPtr->SetPosition(
					{
						static_cast<int>(m_Where.left + Item.X1),
						static_cast<int>(m_Where.top + Item.Y1),
						static_cast<int>(m_Where.left + Item.X2),
						static_cast<int>(m_Where.top + Item.Y2)
					});
				ListPtr->SetBoxType(SHORT_SINGLE_BOX);

				// поле FarDialogItem.Data для DI_LISTBOX используется как верхний заголовок листа
				if (!(Item.Flags & DIF_LISTNOBOX) && !DialogMode.Check(DMODE_OBJECTS_CREATED))
				{
					ListPtr->SetTitle(Item.strData);
				}

				// удалим все элементы
				//ListBox->DeleteItems(); //???? А НАДО ЛИ ????
				if (Item.ListItems && !DialogMode.Check(DMODE_OBJECTS_CREATED))
				{
					ListPtr->AddItem(Item.ListItems);
				}

				ListPtr->ChangeFlags(VMENU_LISTHASFOCUS, (Item.Flags&DIF_FOCUS)!=0);
		}
		// "редакторы" - разговор особый...
		else if (IsEdit(Item.Type))
		{
			if (!DialogMode.Check(DMODE_OBJECTS_CREATED))
			{
				Item.ObjPtr = new DlgEdit(shared_from_this(), I, Item.Type == DI_MEMOEDIT? DLGEDIT_MULTILINE : DLGEDIT_SINGLELINE);

				if (Item.Type == DI_COMBOBOX)
				{
					Item.ListPtr = VMenu::create({}, {}, Global->Opt->Dialogs.CBoxMaxHeight, VMENU_ALWAYSSCROLLBAR, std::static_pointer_cast<Dialog>(shared_from_this()));
					Item.ListPtr->SetVDialogItemID(I);
				}
			}

			const auto DialogEdit = static_cast<DlgEdit*>(Item.ObjPtr);
			// Mantis#58 - символ-маска с кодом 0х0А - пропадает
			//DialogEdit->SetDialogParent((IsEmulatedEditorLine(Item) || (Item.Type==DI_PSWEDIT || Item.Type==DI_FIXEDIT))?
			//                            FEDITLINE_PARENT_SINGLELINE:FEDITLINE_PARENT_MULTILINE);
			DialogEdit->SetDialogParent(Item.Type == DI_MEMOEDIT?FEDITLINE_PARENT_MULTILINE:FEDITLINE_PARENT_SINGLELINE);
			DialogEdit->SetReadOnly(false);

			if (Item.Type == DI_COMBOBOX)
			{
				if (Item.ListPtr)
				{
					auto& ListPtr = Item.ListPtr;
					ListPtr->SetBoxType(SHORT_SINGLE_BOX);
					DialogEdit->SetDropDownBox((Item.Flags& DIF_DROPDOWNLIST) != 0);
					ListPtr->ChangeFlags(VMENU_WRAPMODE, (Item.Flags& DIF_LISTWRAPMODE) != 0);
					ListPtr->ChangeFlags(VMENU_DISABLED, (Item.Flags& DIF_DISABLE) != 0);
					ListPtr->ChangeFlags(VMENU_SHOWAMPERSAND, (Item.Flags& DIF_LISTNOAMPERSAND) == 0);
					ListPtr->ChangeFlags(VMENU_AUTOHIGHLIGHT, (Item.Flags& DIF_LISTAUTOHIGHLIGHT) != 0);

					if (Item.Flags & DIF_LISTAUTOHIGHLIGHT)
						ListPtr->AssignHighlights();

					if (Item.ListItems && !DialogMode.Check(DMODE_OBJECTS_CREATED))
						ListPtr->AddItem(Item.ListItems);

					ListPtr->SetMenuFlags(VMENU_COMBOBOX);
					ListPtr->SetDialogStyle(DialogMode.Check(DMODE_WARNINGSTYLE));
				}
			}

			/* $ 15.10.2000 tran
			  строка редакторирование должна иметь максимум в 511 символов */
			// выставляем максимальный размер в том случае, если он еще не выставлен

			//BUGBUG
			if (DialogEdit->GetMaxLength() == -1)
				DialogEdit->SetMaxLength(Item.MaxLength? static_cast<int>(Item.MaxLength) : -1);

			DialogEdit->SetPosition(
				{
					static_cast<int>(m_Where.left + Item.X1),
					static_cast<int>(m_Where.top + Item.Y1),
					static_cast<int>(m_Where.left + Item.X2),
					static_cast<int>(m_Where.top + Item.Y2)
				});

//      DialogEdit->SetObjectColor(
//         FarColorToReal(DialogMode.Check(DMODE_WARNINGSTYLE) ?
//             ((ItemFlags&DIF_DISABLE)?COL_WARNDIALOGEDITDISABLED:COL_WARNDIALOGEDIT):
//             ((ItemFlags&DIF_DISABLE)?COL_DIALOGEDITDISABLED:COL_DIALOGEDIT)),
//         FarColorToReal((ItemFlags&DIF_DISABLE)?COL_DIALOGEDITDISABLED:COL_DIALOGEDITSELECTED));
			if (Item.Type==DI_PSWEDIT)
			{
				DialogEdit->SetPasswordMode(true);
				// ...Что бы не было повадно... и для повыщения защиты, т.с.
				Item.Flags &= ~DIF_HISTORY;
			}

			bool SetUnchanged = false;

			if (Item.Type == DI_FIXEDIT)
			{
				//   DIF_HISTORY имеет более высокий приоритет, чем DIF_MASKEDIT
				if (Item.Flags & DIF_HISTORY)
					Item.Flags &= ~DIF_MASKEDIT;

				// если DI_FIXEDIT, то курсор сразу ставится на замену...
				//   ай-ай - было недокументировано :-)
				DialogEdit->SetMaxLength(Item.X2 - Item.X1 + 1);
				DialogEdit->SetOvertypeMode(true);
				/* $ 12.08.2000 KM
				   Если тип строки ввода DI_FIXEDIT и установлен флаг DIF_MASKEDIT
				   и непустой параметр Item.Mask, то вызываем новую функцию
				   для установки маски в объект DlgEdit.
				*/

				//  Маска не должна быть пустой (строка из пробелов не учитывается)!
				if ((Item.Flags & DIF_MASKEDIT) && !Item.strMask.empty())
				{
					inplace::trim(Item.strMask);
					if(!Item.strMask.empty())
					{
						DialogEdit->SetInputMask(Item.strMask);
					}
					else
					{
						Item.Flags &= ~DIF_MASKEDIT;
					}
				}

				if (!DialogMode.Check(DMODE_OBJECTS_INITED))
					SetUnchanged = true;
			}
			else

				// "мини-редактор"
				// Последовательно определенные поля ввода (edit controls),
				// имеющие этот флаг группируются в редактор с возможностью
				// вставки и удаления строк
				if (!IsEmulatedEditorLine(Item))
				{
					DialogEdit->SetEditBeyondEnd(false);

					if (!DialogMode.Check(DMODE_OBJECTS_INITED))
						SetUnchanged = true;
				}

			if (Item.Type == DI_COMBOBOX)
				SetUnchanged = true;

			/* $ 01.08.2000 SVS
			   Еже ли стоит флаг DIF_USELASTHISTORY и непустая строка ввода,
			   то подставляем первое значение из History
			*/
			if (Item.Type==DI_EDIT && (Item.Flags & (DIF_HISTORY | DIF_USELASTHISTORY)) == (DIF_HISTORY | DIF_USELASTHISTORY))
			{
				ProcessLastHistory(&Item, -1);
			}

			if ((Item.Flags & DIF_MANUALADDHISTORY) && !(Item.Flags & DIF_HISTORY))
				Item.Flags &= ~DIF_MANUALADDHISTORY; // сбросим нафиг.

			/* $ 18.03.2000 SVS
			   Если это ComBoBox и данные не установлены, то берем из списка
			   при условии, что хоть один из пунктов имеет Selected
			*/

			if (Item.Type == DI_COMBOBOX && Item.strData.empty() && Item.ListItems)
			{
				FarListItem *ListItems=Item.ListItems->Items;
				const auto Length = Item.ListItems->ItemsNumber;
				//Item.ListPtr->AddItem(Item.ListItems);

				const auto ItemIterator = std::find_if(ListItems, ListItems + Length, [](FarListItem& i) { return (i.Flags & LIF_SELECTED) != 0; });
				if (ItemIterator != ListItems + Length)
				{
					if (Item.Flags & (DIF_DROPDOWNLIST | DIF_LISTNOAMPERSAND))
						Item.strData = HiText2Str(ItemIterator->Text);
					else
						Item.strData = ItemIterator->Text;
				}
			}

			DialogEdit->SetCallbackState(false);
			DialogEdit->SetString(Item.strData);
			DialogEdit->SetCallbackState(true);

			if (Item.Type == DI_FIXEDIT)
				DialogEdit->SetCurPos(0);

			// Для обычных строк отрубим постоянные блоки
			if (!IsEmulatedEditorLine(Item))
				DialogEdit->SetPersistentBlocks(Global->Opt->Dialogs.EditBlock);

			DialogEdit->SetDelRemovesBlocks(Global->Opt->Dialogs.DelRemovesBlocks);

			if (Item.Flags & DIF_READONLY)
				DialogEdit->SetReadOnly(true);

			if (SetUnchanged)
				DialogEdit->SetClearFlag(true);
		}
		else if (Item.Type == DI_USERCONTROL)
		{
			if (!DialogMode.Check(DMODE_OBJECTS_CREATED))
				Item.UCData=new DlgUserControl;
		}
		else if (Item.Type == DI_TEXT)
		{
			if (Item.X1 == -1 && (Item.Flags & (DIF_SEPARATOR|DIF_SEPARATOR2)))
				Item.Flags |= DIF_CENTERTEXT;
		}
		else if (Item.Type == DI_VTEXT)
		{
			if (Item.Y1 == -1 && (Item.Flags & (DIF_SEPARATOR|DIF_SEPARATOR2)))
				Item.Flags |= DIF_CENTERTEXT;
		}
	}

	// если будет редактор, то обязательно будет выделен.
	SelectOnEntry(m_FocusPos, true);
	// все объекты созданы!
	if (AllElements)
		DialogMode.Set(DMODE_OBJECTS_CREATED);
}

string Dialog::GetTitle() const
{
	const DialogItemEx *CurItemList=nullptr;

	FOR_CONST_RANGE(Items, i)
	{
		// по первому попавшемуся "тексту" установим заголовок консоли!
		if ((i->Type==DI_TEXT ||
		        i->Type==DI_DOUBLEBOX ||
		        i->Type==DI_SINGLEBOX))
		{
			return trim(i->strData);
		}

		if (i->Type==DI_LISTBOX && i == Items.begin())
			CurItemList = &*i;
	}

	return CurItemList? CurItemList->ListPtr->GetTitle() : L""s;
}

void Dialog::ProcessLastHistory(DialogItemEx *CurItem, int MsgIndex)
{
	string &strData = CurItem->strData;

	if (strData.empty())
	{
		if (const auto EditPtr = static_cast<const DlgEdit*>(CurItem->ObjPtr))
		{
			const auto& DlgHistory = EditPtr->GetHistory();
			if(DlgHistory)
			{
				DlgHistory->ReadLastItem(CurItem->strHistory, strData);
			}

			if (MsgIndex != -1)
			{
				// обработка DM_SETHISTORY => надо пропустить изменение текста через
				// диалоговую функцию
				FarDialogItemData IData={sizeof(FarDialogItemData)};
				IData.PtrData=UNSAFE_CSTR(strData);
				IData.PtrLength=strData.size();
				SendMessage(DM_SETTEXT,MsgIndex,&IData);
			}
		}
	}
}


//   Изменение координат и/или размеров элемента диалога.
bool Dialog::SetItemRect(size_t ID, const SMALL_RECT& Rect)
{
	return ID >= Items.size()? false : SetItemRect(Items[ID], Rect);
}

bool Dialog::SetItemRect(DialogItemEx& Item, const SMALL_RECT& Rect)
{
	if (Rect.Left > Rect.Right || Rect.Top > Rect.Bottom)
		return false;

	const auto Type = Item.Type;
	Item.X1 = Rect.Left;
	Item.Y1 = (Rect.Top<0)? 0 : Rect.Top;

	if (IsEdit(Type))
	{
		const auto DialogEdit = static_cast<DlgEdit*>(Item.ObjPtr);
		Item.X2 = Rect.Right;
		Item.Y2 = (Type == DI_MEMOEDIT? Rect.Bottom : 0);
		DialogEdit->SetPosition({ m_Where.left + Rect.Left, m_Where.top + Rect.Top, m_Where.left + Rect.Right, m_Where.top + Rect.Top });
	}
	else if (Type==DI_LISTBOX)
	{
		Item.X2 = Rect.Right;
		Item.Y2 = Rect.Bottom;
		Item.ListPtr->SetPosition({ m_Where.left + Rect.Left, m_Where.top + Rect.Top, m_Where.left + Rect.Right, m_Where.top + Rect.Bottom });
		Item.ListPtr->SetMaxHeight(Item.Y2-Item.Y1+1);
	}

	switch (Type)
	{
		case DI_TEXT:
			Item.X2 = Rect.Right;
			Item.Y2 = (Item.Flags & DIF_WORDWRAP)? Rect.Bottom : 0;
			break;
		case DI_VTEXT:
			Item.X2=0;                    // ???
			Item.Y2 = Rect.Bottom;
			break;
		case DI_DOUBLEBOX:
		case DI_SINGLEBOX:
		case DI_USERCONTROL:
			Item.X2 = Rect.Right;
			Item.Y2 = Rect.Bottom;
			break;
		default:
			break;
	}

	if (DialogMode.Check(DMODE_SHOW))
	{
		SendMessage(DM_REDRAW, 0, nullptr);
	}

	return true;
}

bool Dialog::GetItemRect(size_t I, SMALL_RECT& Rect)
{
	if (I >= Items.size())
		return false;

	const auto& Item = Items[I];

	int Len=0;
	Rect.Left=Item.X1;
	Rect.Top=Item.Y1;
	Rect.Right=Item.X2;
	Rect.Bottom=Item.Y2;

	switch (Item.Type)
	{
		case DI_COMBOBOX:
		case DI_EDIT:
		case DI_FIXEDIT:
		case DI_PSWEDIT:
		case DI_LISTBOX:
		case DI_MEMOEDIT:
			break;
		default:
			Len = static_cast<int>((Item.Flags & DIF_SHOWAMPERSAND)? Item.strData.size() : HiStrlen(Item.strData));
			break;
	}

	switch (Item.Type)
	{
		case DI_TEXT:

			if (Item.X1==-1)
				Rect.Left = (m_Where.width() - Len) / 2;

			if (Rect.Left < 0)
				Rect.Left=0;

			if (Item.Y1==-1)
				Rect.Top = m_Where.height() / 2;

			if (Rect.Top < 0)
				Rect.Top=0;

			if (!(Item.Flags & DIF_WORDWRAP))
				Rect.Bottom=Rect.Top;

			if (!Rect.Right || Rect.Right == Rect.Left)
				Rect.Right=Rect.Left+Len-(Len?1:0);

			if (Item.Flags & (DIF_SEPARATOR | DIF_SEPARATOR2))
			{
				Rect.Bottom=Rect.Top;
				Rect.Left=(!DialogMode.Check(DMODE_SMALLDIALOG)?3:0); //???
				Rect.Right = m_Where.width() - 1 - (!DialogMode.Check(DMODE_SMALLDIALOG)? 5 : 0); //???
			}

			break;
		case DI_VTEXT:

			if (Item.X1==-1)
				Rect.Left = m_Where.width() / 2;

			if (Rect.Left < 0)
				Rect.Left=0;

			if (Item.Y1==-1)
				Rect.Top = (m_Where.height() - Len) / 2;

			if (Rect.Top < 0)
				Rect.Top=0;

			Rect.Right=Rect.Left;

			//Rect.bottom=Rect.top+Len;
			if (!Rect.Bottom || Rect.Bottom == Rect.Top)
				Rect.Bottom=Rect.Top+Len-(Len?1:0);

			if (Item.Flags & (DIF_SEPARATOR | DIF_SEPARATOR2))
			{
				Rect.Top=(!DialogMode.Check(DMODE_SMALLDIALOG)?1:0); //???
				Rect.Bottom = m_Where.height() - 1 - (!DialogMode.Check(DMODE_SMALLDIALOG)? 3 : 0); //???
				break;
			}
			break;

		case DI_BUTTON:
			Rect.Bottom=Rect.Top;
			Rect.Right=Rect.Left+Len;
			break;
		case DI_CHECKBOX:
		case DI_RADIOBUTTON:
			Rect.Bottom=Rect.Top;
			Rect.Right = Rect.Left + Len + (Item.Type == DI_CHECKBOX? 4 : Item.Flags & DIF_MOVESELECT? 3 : 4);
			break;
		case DI_COMBOBOX:
		case DI_EDIT:
		case DI_FIXEDIT:
		case DI_PSWEDIT:
			Rect.Bottom=Rect.Top;
			break;

		default:
			break; // ???
	}

	return true;
}

bool Dialog::ItemHasDropDownArrow(const DialogItemEx *Item)
{
	return (!Item->strHistory.empty() && (Item->Flags & DIF_HISTORY) && Global->Opt->Dialogs.EditHistory) ||
		(Item->Type == DI_COMBOBOX && Item->ListPtr && Item->ListPtr->HasVisible());
}

//////////////////////////////////////////////////////////////////////////
/* Private:
   Получение данных и удаление "редакторов"
*/
void Dialog::DeleteDialogObjects()
{
	_DIALOG(CleverSysLog CL(L"Dialog::DeleteDialogObjects()"));

	for (auto& i: Items)
	{
		switch (i.Type)
		{
			case DI_EDIT:
			case DI_FIXEDIT:
			case DI_PSWEDIT:
			case DI_COMBOBOX:
			case DI_MEMOEDIT:
				delete static_cast<DlgEdit*>(i.ObjPtr);
				[[fallthrough]];
			case DI_LISTBOX:
				if ((i.Type == DI_COMBOBOX || i.Type == DI_LISTBOX))
					 i.ListPtr.reset();
				break;

			case DI_USERCONTROL:
				delete i.UCData;
				break;

			default:
				break;
		}

		if (i.Flags&DIF_AUTOMATION)
			i.Auto.clear();
	}
}



void Dialog::GetDialogObjectsExpandData()
{
	for (auto& i: Items)
	{
		switch (i.Type)
		{
			case DI_EDIT:
			case DI_COMBOBOX:
			{
				if (i.ObjPtr && (i.Flags&DIF_EDITEXPAND))
				{
					const auto EditPtr = static_cast<DlgEdit*>(i.ObjPtr);

					// подготовим данные
					// получим данные
					auto strData = EditPtr->GetString();

					/* $ 01.08.2000 SVS
					   ! В History должно заносится значение (для DIF_EXPAND...) перед
					    расширением среды!
					*/
					/*$ 05.07.2000 SVS $
					Проверка - этот элемент предполагает расширение переменных среды?
					т.к. функция GetDialogObjectsData() может вызываться самостоятельно
					Но надо проверить!*/
					/* $ 04.12.2000 SVS
					  ! Для DI_PSWEDIT и DI_FIXEDIT обработка DIF_EDITEXPAND не нужна
					   (DI_FIXEDIT допускается для случая если нету маски)
					*/

					strData = os::env::expand(strData);
					//как бы грязный хак, нам нужно обновить строку чтоб отдавалась правильная строка
					//для различных DM_* после закрытия диалога, но ни в коем случае нельзя чтоб
					//высылался DN_EDITCHANGE для этого изменения, ибо диалог уже закрыт.
					EditPtr->SetCallbackState(false);
					EditPtr->SetString(strData);
					EditPtr->SetCallbackState(true);

					i.strData = strData;
				}

				break;
			}
			default:
				break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
/* Public:
   Сохраняет значение из полей редактирования.
   При установленном флаге DIF_HISTORY, сохраняет данные в реестре.
*/
void Dialog::GetDialogObjectsData()
{
	for (auto& i: Items)
	{
		const auto IFlags = i.Flags;

		switch (i.Type)
		{
			case DI_MEMOEDIT:
				break; //????
			case DI_EDIT:
			case DI_FIXEDIT:
			case DI_PSWEDIT:
			case DI_COMBOBOX:
			{
				if (i.ObjPtr)
				{
					const auto EditPtr = static_cast<const DlgEdit*>(i.ObjPtr);

					// подготовим данные
					// получим данные
					const auto& strData = EditPtr->GetString();

					if (m_ExitCode >=0 &&
					        (IFlags & DIF_HISTORY) &&
					        !(IFlags & DIF_MANUALADDHISTORY) && // при мануале не добавляем
					        !i.strHistory.empty() &&
					        Global->Opt->Dialogs.EditHistory)
					{
						AddToEditHistory(&i, strData);
					}
#if 0
					/* $ 01.08.2000 SVS
					   ! В History должно заносится значение (для DIF_EXPAND...) перед
					    расширением среды!
					*/
					/*$ 05.07.2000 SVS $
					Проверка - этот элемент предполагает расширение переменных среды?
					т.к. функция GetDialogObjectsData() может вызываться самостоятельно
					Но надо проверить!*/
					/* $ 04.12.2000 SVS
					  ! Для DI_PSWEDIT и DI_FIXEDIT обработка DIF_EDITEXPAND не нужна
					   (DI_FIXEDIT допускается для случая если нету маски)
					*/

					if ((IFlags&DIF_EDITEXPAND) && i.Type != DI_PSWEDIT && i.Type != DI_FIXEDIT)
					{
						strData = os::env::expand(strData);
						//как бы грязный хак, нам нужно обновить строку чтоб отдавалась правильная строка
						//для различных DM_* после закрытия диалога, но ни в коем случае нельзя чтоб
						//высылался DN_EDITCHANGE для этого изменения, ибо диалог уже закрыт.
						EditPtr->SetCallbackState(false);
						EditPtr->SetString(strData);
						EditPtr->SetCallbackState(true);

					}
#endif
					i.strData = strData;
				}

				break;
			}
			case DI_LISTBOX:
				/*
				  if(i->ListPtr)
				  {
				    i->ListPos=Items[I].ListPtr->GetSelectPos();
				    break;
				  }
				*/
				break;
				/**/
			default:
				break;
		}

#if 0
		if ((i->Type == DI_COMBOBOX || i->Type == DI_LISTBOX) && i->ListPtr && i->ListItems && DlgProc == DefDlgProc)
		{
			int ListPos=i->ListPtr->GetSelectPos();

			if (ListPos < i->ListItems->ItemsNumber)
			{
				for (int J=0; J < i->ListItems->ItemsNumber; ++J)
					i->ListItems->Items[J].Flags&=~LIF_SELECTED;

				i->ListItems->Items[ListPos].Flags|=LIF_SELECTED;
			}
		}

#else

		if ((i.Type == DI_COMBOBOX || i.Type == DI_LISTBOX))
		{
			i.ListPos = i.ListPtr? i.ListPtr->GetSelectPos() : 0;
		}

#endif
	}
}

// Функция формирования и запроса цветов.
intptr_t Dialog::CtlColorDlgItem(FarColor Color[4], size_t ItemPos, FARDIALOGITEMTYPES Type, bool Focus, bool Default,FARDIALOGITEMFLAGS Flags)
{
	const auto IsWarning = DialogMode.Check(DMODE_WARNINGSTYLE);
	const auto DisabledItem = (Flags&DIF_DISABLE) != 0;

	switch (Type)
	{
		case DI_SINGLEBOX:
		case DI_DOUBLEBOX:
		{
			// Title
			Color[0] = colors::PaletteColorToFarColor(IsWarning? (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBOXTITLE) : (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGBOXTITLE));
			// HiText
			Color[1] = colors::PaletteColorToFarColor(IsWarning? (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGHIGHLIGHTBOXTITLE) : (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGHIGHLIGHTBOXTITLE));
			// Box
			Color[2] = colors::PaletteColorToFarColor(IsWarning? (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBOX) : (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGBOX));
			break;
		}

		case DI_VTEXT:
		case DI_TEXT:
		{
			Color[0] = colors::PaletteColorToFarColor((Flags & DIF_BOXCOLOR)? (IsWarning? (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBOX) : (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGBOX)) : (IsWarning? (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGTEXT) : (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGTEXT)));
			// HiText
			Color[1] = colors::PaletteColorToFarColor(IsWarning? (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGHIGHLIGHTTEXT) : (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGHIGHLIGHTTEXT));
			if (Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2))
			{
				// Box
				Color[2] = colors::PaletteColorToFarColor(IsWarning? (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBOX) : (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGBOX));
			}
			break;
		}

		case DI_CHECKBOX:
		case DI_RADIOBUTTON:
		{
			Color[0] = colors::PaletteColorToFarColor(IsWarning? (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGTEXT) : (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGTEXT));
			// HiText
			Color[1] = colors::PaletteColorToFarColor(IsWarning? (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGHIGHLIGHTTEXT) : (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGHIGHLIGHTTEXT));
			break;
		}

		case DI_BUTTON:
		{
			if (Focus)
			{
				SetCursorType(false, 10);
				// TEXT
				Color[0] = colors::PaletteColorToFarColor(IsWarning? (DisabledItem?COL_WARNDIALOGDISABLED:(Default?COL_WARNDIALOGSELECTEDDEFAULTBUTTON:COL_WARNDIALOGSELECTEDBUTTON)) : (DisabledItem?COL_DIALOGDISABLED:(Default?COL_DIALOGSELECTEDDEFAULTBUTTON:COL_DIALOGSELECTEDBUTTON)));
				// HiText
				Color[1] = colors::PaletteColorToFarColor(IsWarning? (DisabledItem?COL_WARNDIALOGDISABLED:(Default?COL_WARNDIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON:COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON)) : (DisabledItem?COL_DIALOGDISABLED:(Default?COL_DIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON:COL_DIALOGHIGHLIGHTSELECTEDBUTTON)));
			}
			else
			{
				// TEXT
				Color[0] = colors::PaletteColorToFarColor(IsWarning?
						(DisabledItem?COL_WARNDIALOGDISABLED:(Default?COL_WARNDIALOGDEFAULTBUTTON:COL_WARNDIALOGBUTTON)):
						(DisabledItem?COL_DIALOGDISABLED:(Default?COL_DIALOGDEFAULTBUTTON:COL_DIALOGBUTTON)));
				// HiText
				Color[1] = colors::PaletteColorToFarColor(IsWarning? (DisabledItem?COL_WARNDIALOGDISABLED:(Default?COL_WARNDIALOGHIGHLIGHTDEFAULTBUTTON:COL_WARNDIALOGHIGHLIGHTBUTTON)) : (DisabledItem?COL_DIALOGDISABLED:(Default?COL_DIALOGHIGHLIGHTDEFAULTBUTTON:COL_DIALOGHIGHLIGHTBUTTON)));
			}
			break;
		}

		case DI_EDIT:
		case DI_FIXEDIT:
		case DI_PSWEDIT:
		case DI_COMBOBOX:
		case DI_MEMOEDIT:
		{
			if (Type == DI_COMBOBOX && (Flags & DIF_DROPDOWNLIST))
			{
				if (IsWarning)
				{
					// Text
					Color[0] = colors::PaletteColorToFarColor(DisabledItem? COL_WARNDIALOGEDITDISABLED: Focus? COL_WARNDIALOGEDITSELECTED : COL_WARNDIALOGEDIT);
					// Select
					Color[1] = colors::PaletteColorToFarColor(DisabledItem? COL_WARNDIALOGEDITDISABLED : Focus? COL_WARNDIALOGEDITSELECTED : COL_WARNDIALOGEDIT);
					// Unchanged
					Color[2] = colors::PaletteColorToFarColor(DisabledItem? COL_WARNDIALOGEDITDISABLED : COL_WARNDIALOGEDITUNCHANGED); //???
					// History
					Color[3] = colors::PaletteColorToFarColor(DisabledItem? COL_WARNDIALOGDISABLED : COL_WARNDIALOGTEXT);
				}
				else
				{
					// Text
					Color[0] = colors::PaletteColorToFarColor(DisabledItem? COL_DIALOGEDITDISABLED : Focus? COL_DIALOGEDITSELECTED : COL_DIALOGEDIT);
					// Select
					Color[1] = colors::PaletteColorToFarColor(DisabledItem? COL_DIALOGEDITDISABLED: Focus? COL_DIALOGEDITSELECTED : COL_DIALOGEDIT);
					// Unchanged
					Color[2] = colors::PaletteColorToFarColor(DisabledItem? COL_DIALOGEDITDISABLED : COL_DIALOGEDITUNCHANGED); //???
					// History
					Color[3] = colors::PaletteColorToFarColor(DisabledItem? COL_DIALOGDISABLED : COL_DIALOGTEXT);
				}
			}
			else
			{
				if (IsWarning)
				{
					// Text
					Color[0] = colors::PaletteColorToFarColor(DisabledItem? COL_WARNDIALOGEDITDISABLED : Flags & DIF_NOFOCUS? COL_WARNDIALOGEDITUNCHANGED : COL_WARNDIALOGEDIT);
					// Select
					Color[1] = colors::PaletteColorToFarColor(DisabledItem? COL_WARNDIALOGEDITDISABLED : COL_WARNDIALOGEDITSELECTED);
					// Unchanged
					Color[2] = colors::PaletteColorToFarColor(DisabledItem? COL_WARNDIALOGEDITDISABLED : COL_WARNDIALOGEDITUNCHANGED);
					// History
					Color[3] = colors::PaletteColorToFarColor(DisabledItem? COL_WARNDIALOGDISABLED : COL_WARNDIALOGTEXT);
				}
				else
				{
					// Text
					Color[0] = colors::PaletteColorToFarColor(DisabledItem? COL_DIALOGEDITDISABLED : Flags & DIF_NOFOCUS? COL_DIALOGEDITUNCHANGED : COL_DIALOGEDIT);
					// Select
					Color[1] = colors::PaletteColorToFarColor(DisabledItem? COL_DIALOGEDITDISABLED : COL_DIALOGEDITSELECTED);
					// Unchanged
					Color[2] = colors::PaletteColorToFarColor(DisabledItem ? COL_DIALOGEDITDISABLED : COL_DIALOGEDITUNCHANGED);
					// History
					Color[3] = colors::PaletteColorToFarColor(DisabledItem? COL_DIALOGDISABLED : COL_DIALOGTEXT);
				}
			}
			break;
		}

		case DI_LISTBOX:
		{
			Items[ItemPos].ListPtr->SetColors(nullptr);
			return 0;
		}
		default:
		{
			break;
		}
	}
	FarDialogItemColors ItemColors = {sizeof(FarDialogItemColors)};
	ItemColors.ColorsCount=4;
	ItemColors.Colors=Color;
	return DlgProc(DN_CTLCOLORDLGITEM, ItemPos, &ItemColors);
}

//////////////////////////////////////////////////////////////////////////
/* Private:
   Отрисовка элементов диалога на экране.
*/
void Dialog::ShowDialog(size_t ID)
{
	_DIALOG(CleverSysLog CL(L"Dialog::ShowDialog()"));
	_DIALOG(SysLog(L"Locked()=%d, DMODE_SHOW=%d DMODE_DRAWING=%d",Locked(),DialogMode.Check(DMODE_SHOW),DialogMode.Check(DMODE_DRAWING)));

	string strStr;
	int X,Y;
	size_t DrawItemCount;
	FarColor ItemColor[4] = {};
	bool DrawFullDialog = false;

	//   Если не разрешена отрисовка, то вываливаем.
	if (m_DisableRedraw ||                // разрешена прорисовка ?
	        (ID+1 > Items.size()) ||             // а номер в рамках дозволенного?
	        DialogMode.Check(DMODE_DRAWING) || // диалог рисуется?
	        !DialogMode.Check(DMODE_SHOW) ||   // если не видим, то и не отрисовываем.
	        !DialogMode.Check(DMODE_OBJECTS_INITED))
		return;

	_DIALOG(SysLog(L"[%d] DialogMode.Set(DMODE_DRAWING)",__LINE__));
	DialogMode.Set(DMODE_DRAWING);  // диалог рисуется!!!

	if (DialogMode.Check(DMODE_NEEDUPDATE))
	{
		DialogMode.Clear(DMODE_NEEDUPDATE);
		ID = static_cast<size_t>(-1);
	}

	if (ID == static_cast<size_t>(-1)) // рисуем все?
	{
		DrawFullDialog = true;

		//   Перед прорисовкой диалога посылаем сообщение в обработчик
		if (!DlgProc(DN_DRAWDIALOG, 0, nullptr))
		{
			_DIALOG(SysLog(L"[%d] DialogMode.Clear(DMODE_DRAWING)",__LINE__));
			DialogMode.Clear(DMODE_DRAWING);  // конец отрисовки диалога!!!
			return;
		}

		//   перед прорисовкой подложки окна диалога...
		if (!DialogMode.Check(DMODE_NODRAWSHADOW))
			Shadow(DialogMode.Check(DMODE_FULLSHADOW));              // "наводим" тень

		if (!DialogMode.Check(DMODE_NODRAWPANEL))
		{
			FarColor Color = colors::PaletteColorToFarColor(DialogMode.Check(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGTEXT:COL_DIALOGTEXT);
			DlgProc(DN_CTLCOLORDIALOG, 0, &Color);
			SetScreen(m_Where, L' ', Color);
		}

		ID=0;
		DrawItemCount = Items.size();
	}
	else
	{
		DrawItemCount=ID+1;
	}

	/* TODO:
	   если рисуется контрол и по Z-order`у он пересекается с
	   другим контролом (по координатам), то для "позднего"
	   контрола тоже нужна прорисовка.
	*/
	{
		bool CursorVisible=false;
		DWORD CursorSize=0;

		if (m_FocusPos != ID)
		{
			const auto& FocusedItem = Items[m_FocusPos];
			if (FocusedItem.Type == DI_USERCONTROL && FocusedItem.UCData->CursorPos.X != -1 && FocusedItem.UCData->CursorPos.Y != -1)
			{
				CursorVisible = FocusedItem.UCData->CursorVisible;
				CursorSize = FocusedItem.UCData->CursorSize;
			}
		}

		SetCursorType(CursorVisible,CursorSize);
	}

	for (size_t I = ID; I < DrawItemCount; I++)
	{
		const auto& Item = Items[I];

		if (Item.Flags&DIF_HIDDEN)
			continue;

		/* $ 28.07.2000 SVS
		   Перед прорисовкой каждого элемента посылаем сообщение
		   посредством функции SendDlgMessage - в ней делается все!
		*/
		if (!SendMessage(DN_DRAWDLGITEM, I, nullptr))
			continue;

		int LenText;
		short CX1=Item.X1;
		short CY1=Item.Y1;
		short CX2=Item.X2;
		short CY2=Item.Y2;

		if (CX2 > m_Where.width() - 1)
			CX2 = m_Where.width() - 1;

		if (CY2 > m_Where.height() - 1)
			CY2 = m_Where.height() - 1;

		short CW=CX2-CX1+1;
		short CH=CY2-CY1+1;
		CtlColorDlgItem(ItemColor, I,Item.Type,(Item.Flags&DIF_FOCUS) != 0, (Item.Flags&DIF_DEFAULTBUTTON) != 0, Item.Flags);
#if 0

		// TODO: прежде чем эту строку применять... нужно проверить _ВСЕ_ диалоги на предмет X2, Y2. !!!
		if (((CX1 > -1) && (CX2 > 0) && (CX2 > CX1)) &&
		        ((CY1 > -1) && (CY2 > 0) && (CY2 > CY1)))
			SetScreen(X1+CX1,Y1+CY1,X1+CX2,Y1+CY2,' ',Attr&0xFF);

#endif

		switch (Item.Type)
		{
				/* ***************************************************************** */
			case DI_SINGLEBOX:
			case DI_DOUBLEBOX:
			{
				bool IsDrawTitle = true;
				GotoXY(m_Where.left + CX1, m_Where.top + CY1);
				SetColor(ItemColor[2]);

				if (CY1 == CY2)
				{
					DrawLine(CX2 - CX1 + 1, Item.Type == DI_SINGLEBOX ? line_type::h1 : line_type::h2);
				}
				else if (CX1 == CX2)
				{
					DrawLine(CY2 - CY1 + 1, Item.Type == DI_SINGLEBOX ? line_type::v1 : line_type::v2);
					IsDrawTitle = false;
				}
				else
				{
					Box(
						{ m_Where.left + CX1, m_Where.top + CY1, m_Where.left + CX2, m_Where.top + CY2 },
						ItemColor[2],
						(Item.Type==DI_SINGLEBOX)? SINGLE_BOX : DOUBLE_BOX
					);
				}

				if (!Item.strData.empty() && IsDrawTitle && CW > 2)
				{
					//  ! Пусть диалог сам заботится о ширине собственного заголовка.
					strStr = truncate_right(Item.strData, CW - 2); // 5 ???
					LenText=LenStrItem(I,strStr);

					if (LenText < CW-2)
					{
						strStr.insert(0, 1, L' ');
						strStr.push_back(L' ');
						LenText=LenStrItem(I, strStr);
					}

					X = m_Where.left + CX1 + (CW - LenText) / 2;

					if ((Item.Flags & DIF_LEFTTEXT) && m_Where.left + CX1 + 1 < X)
						X = m_Where.left + CX1 + 1;
					else if (Item.Flags & DIF_RIGHTTEXT)
						X = m_Where.left + CX1 + (CW - LenText) - 1; //2

					SetColor(ItemColor[0]);
					GotoXY(X, m_Where.top + CY1);

					if (Item.Flags & DIF_SHOWAMPERSAND)
						Text(strStr);
					else
						HiText(strStr,ItemColor[1]);
				}

				break;
			}
			/* ***************************************************************** */
			case DI_TEXT:
			{
				strStr = Item.strData;

				if (!(Item.Flags & DIF_WORDWRAP))
				{
					if (Item.Flags & (DIF_SEPARATORUSER | DIF_SEPARATOR | DIF_SEPARATOR2))
					{
						if (!strStr.empty())
						{
							if (!starts_with(strStr, L" "sv))
								strStr.insert(0, 1, L' ');
							if (!ends_with(strStr, L" "sv))
								strStr.push_back(L' ');
						}
					}
					else if (CX1 != -1 && CX2 > CX1)
					{
						if (Item.Flags & DIF_RIGHTTEXT)
							inplace::fit_to_right(strStr, CX2 - CX1 + 1);
						if (Item.Flags & DIF_CENTERTEXT)
							inplace::fit_to_center(strStr, CX2 - CX1 + 1);
						else
							inplace::fit_to_left(strStr, CX2 - CX1 + 1);
					}

					LenText = LenStrItem(I, strStr);

					if ((CX2 <= 0) || (CX2 < CX1))
						CW = LenText;

					X = (CX1 == -1)? (m_Where.width() - LenText) / 2 : CX1;
					Y = (CY1 == -1)? m_Where.height() / 2 : CY1;
					int XS=(CX1==-1 || !(Item.Flags&DIF_SEPARATORUSER))?X:CX1;

					if( (Item.Flags & DIF_RIGHTTEXT) && CX2 > CX1 )
						X=CX2-LenText+1;

					if (X < 0)
						X=0;

					if (m_Where.left + X + LenText > m_Where.right)
					{
						int tmpCW=ObjWidth();

						if (CW < ObjWidth())
							tmpCW=CW+1;

						strStr.resize(tmpCW-1);
					}

					if (CX1 > -1 && CX2 > CX1 && !(Item.Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2))) //половинчатое решение
					{
						SetScreen({ m_Where.left + CX1, m_Where.top + Y, m_Where.left + CX2, m_Where.top + Y }, L' ', ItemColor[0]);
						/*
						int CntChr=CX2-CX1+1;
						SetColor(ItemColor[0]);
						GotoXY(X1+X,Y1+Y);

						if (X1+X+CntChr-1 > X2)
							CntChr=X2-(X1+X)+1;

						Text(string(CntChr, L' '));

						if (CntChr < LenText)
							strStr.SetLength(CntChr);
						*/
					}

					if (Item.Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2))
					{
						SetColor(ItemColor[2]);
						GotoXY(m_Where.left + ((Item.Flags & DIF_SEPARATORUSER)? XS : (!DialogMode.Check(DMODE_SMALLDIALOG)? 3 : 0)), m_Where.top + Y); //????
						DrawLine(
							(Item.Flags & DIF_SEPARATORUSER)? CX2 - CX1 + 1 : RealWidth - (DialogMode.Check(DMODE_SMALLDIALOG)? 0 : 6),
							(Item.Flags & DIF_SEPARATORUSER)? line_type::h_user : (Item.Flags & DIF_SEPARATOR2? line_type::h2_to_v2 : line_type::h1_to_v2),
							Item.strMask
						);
					}

					GotoXY(m_Where.left + X, m_Where.top + Y);
					SetColor(ItemColor[0]);

					if (Item.Flags & DIF_SHOWAMPERSAND)
						Text(strStr);
					else
						HiText(strStr,ItemColor[1]);
				}
				else
				{
					SetScreen({ m_Where.left + CX1, m_Where.top + CY1, m_Where.left + CX2, m_Where.top + CY2 }, L' ', ItemColor[0]);

					DWORD CountLine=0;

					for (const auto& i: wrapped_text(strStr, CW))
					{
						auto strResult = string(i);

						if (Item.Flags & DIF_CENTERTEXT)
							inplace::fit_to_center(strResult, CW);
						else if (Item.Flags & DIF_RIGHTTEXT)
							inplace::fit_to_right(strResult, CW);
						else
							inplace::fit_to_left(strResult, CW);

						LenText=LenStrItem(I,strResult);
						X=(CX1==-1 || (Item.Flags & DIF_CENTERTEXT))?(CW-LenText)/2:CX1;
						if (X < CX1)
							X=CX1;
						GotoXY(m_Where.left + X, m_Where.top + CY1 + CountLine);
						SetColor(ItemColor[0]);

						if (Item.Flags & DIF_SHOWAMPERSAND)
							Text(strResult);
						else
							HiText(strResult,ItemColor[1]);

						if (++CountLine >= static_cast<DWORD>(CH))
							break;
					}
				}

				break;
			}
			/* ***************************************************************** */
			case DI_VTEXT:
			{
				strStr = Item.strData;
				LenText=LenStrItem(I,strStr);

				if (!(Item.Flags & (DIF_SEPARATORUSER | DIF_SEPARATOR | DIF_SEPARATOR2)) && (Item.Flags & DIF_CENTERTEXT) && CY1 != -1 && CY2 > CY1)
				{
					inplace::fit_to_center(strStr, CY2 - CY1 + 1);
					LenText = static_cast<int>(strStr.size());
				}

				if ((CY2 <= 0) || (CY2 < CY1))
					CH = LenStrItem(I,strStr);

				X = CX1 == -1? m_Where.width() / 2 : CX1;
				Y = CY1 == -1? (m_Where.height() - LenText) / 2 : CY1;
				int YS=(CY1==-1 || !(Item.Flags&DIF_SEPARATORUSER))?Y:CY1;

				if( (Item.Flags & DIF_RIGHTTEXT) && CY2 > CY1 )
					Y=CY2-LenText+1;

				if (Y < 0)
					Y=0;

				if (Y + LenText >= m_Where.height())
				{
					int tmpCH=ObjHeight();

					if (CH < ObjHeight())
						tmpCH=CH+1;

					strStr.resize(tmpCH-1);
				}

				// нужно ЭТО
				//SetScreen(X1+CX1,Y1+CY1,X1+CX2,Y1+CY2,' ',Attr&0xFF);
				// вместо этого:
				if (CY1 > -1 && CY2 > CY1 && !(Item.Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2))) //половинчатое решение
				{
					SetScreen({ m_Where.left + X, m_Where.top + CY1, m_Where.left + X, m_Where.top + CY2 }, L' ', ItemColor[0]);
					/*
					int CntChr=CY2-CY1+1;
					SetColor(ItemColor[0]);
					GotoXY(X1+X,Y1+Y);

					if (Y1+Y+CntChr-1 > Y2)
						CntChr=Y2-(Y1+Y)+1;

					vmprintf(L"%*s",CntChr,L"");
					*/
				}

				if (Item.Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2))
				{
					SetColor(ItemColor[2]);
					GotoXY(m_Where.left + X, m_Where.top + ((Item.Flags & DIF_SEPARATORUSER)? YS : (!DialogMode.Check(DMODE_SMALLDIALOG)? 1 : 0)));  //????
					DrawLine(
						(Item.Flags & DIF_SEPARATORUSER)? CY2 - CY1 + 1 : RealHeight - (DialogMode.Check(DMODE_SMALLDIALOG)? 0 : 2),
						(Item.Flags & DIF_SEPARATORUSER)? line_type::v_user : (Item.Flags & DIF_SEPARATOR2? line_type::v2_to_h2 : line_type::v1_to_h2),
						Item.strMask
					);
				}

				SetColor(ItemColor[0]);
				GotoXY(m_Where.left + X, m_Where.top + Y);

				if (Item.Flags & DIF_SHOWAMPERSAND)
					VText(strStr);
				else
					HiText(strStr,ItemColor[1], TRUE);

				break;
			}
			/* ***************************************************************** */
			case DI_CHECKBOX:
			case DI_RADIOBUTTON:
			{
				SetColor(ItemColor[0]);
				GotoXY(m_Where.left + CX1, m_Where.top + CY1);

				if (Item.Type==DI_CHECKBOX)
				{
					const auto Check = Item.Selected? (Item.Flags & DIF_3STATE) && Item.Selected == 2? L'?' : L'x' : L' ';
					strStr = concat(L'[', Check, L']');

					if (!Item.strData.empty())
						strStr += L' ';
				}
				else
				{
					const auto Dot = Item.Selected? L'\x2022' : L' ';

					if (Item.Flags&DIF_MOVESELECT)
					{
						strStr = concat(L' ', Dot, L' ');
					}
					else
					{
						strStr = concat(L'(', Dot, L')');

						if (!Item.strData.empty())
							strStr += L' ';
					}
				}

				strStr += Item.strData;
				LenText=LenStrItem(I, strStr);

				if (CX1 + LenText >= m_Where.width())
					strStr.resize(ObjWidth()-1);

				if (Item.Flags & DIF_SHOWAMPERSAND)
					Text(strStr);
				else
					HiText(strStr,ItemColor[1]);

				if (Item.Flags&DIF_FOCUS)
				{
					//   Отключение мигающего курсора при перемещении диалога
					if (!IsMoving())
						SetCursorType(true, -1);

					MoveCursor({ m_Where.left + CX1 + 1, m_Where.top + CY1 });
				}

				break;
			}
			/* ***************************************************************** */
			case DI_BUTTON:
			{
				strStr = Item.strData;
				SetColor(ItemColor[0]);
				GotoXY(m_Where.left + CX1, m_Where.top + CY1);

				if (Item.Flags & DIF_SHOWAMPERSAND)
					Text(strStr);
				else
					HiText(strStr,ItemColor[1]);

				if(Item.Flags & DIF_SETSHIELD)
				{
					int startx = m_Where.left + CX1 + (Item.Flags&DIF_NOBRACKETS? 0 : 2);
					Global->ScrBuf->ApplyColor({ startx, m_Where.top + CY1, startx + 1, m_Where.top + CY1 }, colors::ConsoleColorToFarColor(B_YELLOW | F_LIGHTBLUE));
				}
				break;
			}
			/* ***************************************************************** */
			case DI_EDIT:
			case DI_FIXEDIT:
			case DI_PSWEDIT:
			case DI_COMBOBOX:
			case DI_MEMOEDIT:
			{
				const auto EditPtr = static_cast<DlgEdit*>(Item.ObjPtr);

				if (!EditPtr)
					break;

				EditPtr->SetObjectColor(ItemColor[0],ItemColor[1],ItemColor[2]);

				if (Item.Flags&DIF_FOCUS)
				{
					//   Отключение мигающего курсора при перемещении диалога
					if (!IsMoving())
						SetCursorType(true, -1);

					EditPtr->Show();
				}
				else
				{
					EditPtr->FastShow();
				}

				//   Отключение мигающего курсора при перемещении диалога
				if (IsMoving())
					SetCursorType(false, 0);

				if (ItemHasDropDownArrow(&Item))
				{
					const auto EditPos = EditPtr->GetPosition();
					//Text((CurItem->Type == DI_COMBOBOX?"\x1F":"\x19"));
					Text({ EditPos.right + 1, EditPos.top }, ItemColor[3], L"\x2193"sv);
				}

				if (Item.Type == DI_COMBOBOX && GetDropDownOpened() && Item.ListPtr->IsVisible()) // need redraw VMenu?
				{
					Item.ListPtr->Hide();
					Item.ListPtr->Show();
				}

				break;
			}
			/* ***************************************************************** */
			case DI_LISTBOX:
			{
				if (Item.ListPtr)
				{
					//   Перед отрисовкой спросим об изменении цветовых атрибутов
					FarColor RealColors[VMENU_COLOR_COUNT] = {};
					FarDialogItemColors ListColors={sizeof(FarDialogItemColors)};
					ListColors.ColorsCount=VMENU_COLOR_COUNT;
					ListColors.Colors=RealColors;
					Item.ListPtr->GetColors(&ListColors);

					if (DlgProc(DN_CTLCOLORDLGLIST,I,&ListColors))
						Item.ListPtr->SetColors(&ListColors);

					// Курсор запоминаем...
					bool CursorVisible=false;
					size_t CursorSize = 0;
					GetCursorType(CursorVisible,CursorSize);
					Item.ListPtr->Show();

					// .. а теперь восстановим!
					if (m_FocusPos != I)
						SetCursorType(CursorVisible,CursorSize);
				}

				break;
			}
			/* 01.08.2000 SVS $ */
			/* ***************************************************************** */
			case DI_USERCONTROL:

				if (Item.VBuf)
				{
					PutText({ m_Where.left + CX1, m_Where.top + CY1, m_Where.left + CX2, m_Where.top + CY2 }, Item.VBuf);

					// не забудем переместить курсор, если он позиционирован.
					if (m_FocusPos == I)
					{
						const auto& UCData = Item.UCData;
						if (UCData->CursorPos.X != -1 && UCData->CursorPos.Y != -1)
						{
							MoveCursor({ UCData->CursorPos.X + CX1 + m_Where.left, UCData->CursorPos.Y + CY1 + m_Where.top });
							SetCursorType(UCData->CursorVisible, UCData->CursorSize);
						}
						else
							SetCursorType(false, -1);
					}
				}

				break; //уже нарисовали :-)))
				/* ***************************************************************** */
				//.........
		} // end switch(...

		SendMessage(DN_DRAWDLGITEMDONE, I, nullptr);
	} // end for (I=...

	// КОСТЫЛЬ!
	// но работает ;-)
	for (const auto& i: Items)
	{
		if (i.ListPtr && GetDropDownOpened() && i.ListPtr->IsVisible())
		{
			if ((i.Type == DI_COMBOBOX) ||
			        ((i.Type == DI_EDIT || i.Type == DI_FIXEDIT) &&
			         !(i.Flags&DIF_HIDDEN) &&
			         (i.Flags&DIF_HISTORY)))
			{
				i.ListPtr->Show();
			}
		}
	}

	DialogMode.Set(DMODE_SHOW); // диалог на экране!

	if (DrawFullDialog)
		DlgProc(DN_DRAWDIALOGDONE, 0, nullptr);

	_DIALOG(SysLog(L"[%d] DialogMode.Clear(DMODE_DRAWING)",__LINE__));
	DialogMode.Clear(DMODE_DRAWING);  // конец отрисовки диалога!!!
}

int Dialog::LenStrItem(size_t ID)
{
	return LenStrItem(Items[ID]);
}

int Dialog::LenStrItem(size_t ID, string_view const Str) const
{
	return static_cast<int>((Items[ID].Flags & DIF_SHOWAMPERSAND)? Str.size() : HiStrlen(Str));
}

int Dialog::LenStrItem(const DialogItemEx& Item)
{
	return static_cast<int>((Item.Flags & DIF_SHOWAMPERSAND)? Item.strData.size() : HiStrlen(Item.strData));
}

bool Dialog::ProcessMoveDialog(DWORD Key)
{
	if (DialogMode.Check(DMODE_KEYDRAGGED)) // если диалог таскается
	{
		// TODO: Здесь проверить "уже здесь" и не делать лишних движений
		//       Т.е., если нажали End, то при следующем End ненужно ничего делать! - сравнить координаты !!!
		int rr=1;

		//   При перемещении диалога повторяем поведение "борландовых" сред.
		switch (Key)
		{
			case KEY_CTRLLEFT:  case KEY_CTRLNUMPAD4:
			case KEY_RCTRLLEFT: case KEY_RCTRLNUMPAD4:
			case KEY_CTRLHOME:  case KEY_CTRLNUMPAD7:
			case KEY_RCTRLHOME: case KEY_RCTRLNUMPAD7:
			case KEY_HOME:      case KEY_NUMPAD7:
				rr = any_of(Key, KEY_CTRLLEFT, KEY_RCTRLLEFT, KEY_CTRLNUMPAD4, KEY_RCTRLNUMPAD4)? 10 : m_Where.left;
				[[fallthrough]];
			case KEY_LEFT:      case KEY_NUMPAD4:
				Hide();

				for (int i=0; i<rr; i++)
					if (m_Where.right > 0)
					{
						--m_Where.left;
						--m_Where.right;
						AdjustEditPos(-1,0);
					}

				Show();
				break;

			case KEY_CTRLRIGHT:  case KEY_CTRLNUMPAD6:
			case KEY_RCTRLRIGHT: case KEY_RCTRLNUMPAD6:
			case KEY_CTRLEND:    case KEY_CTRLNUMPAD1:
			case KEY_RCTRLEND:   case KEY_RCTRLNUMPAD1:
			case KEY_END:       case KEY_NUMPAD1:
				rr = any_of(Key, KEY_CTRLRIGHT, KEY_RCTRLRIGHT, KEY_CTRLNUMPAD6, KEY_RCTRLNUMPAD6)? 10 : std::max(0, ScrX - m_Where.right);
				[[fallthrough]];
			case KEY_RIGHT:     case KEY_NUMPAD6:
				Hide();

				for (int i=0; i<rr; i++)
					if (m_Where.left < ScrX)
					{
						++m_Where.left;
						++m_Where.right;
						AdjustEditPos(1,0);
					}

				Show();
				break;

			case KEY_PGUP:      case KEY_NUMPAD9:
			case KEY_CTRLPGUP:  case KEY_CTRLNUMPAD9:
			case KEY_RCTRLPGUP: case KEY_RCTRLNUMPAD9:
			case KEY_CTRLUP:    case KEY_CTRLNUMPAD8:
			case KEY_RCTRLUP:   case KEY_RCTRLNUMPAD8:
				rr = any_of(Key, KEY_CTRLUP, KEY_RCTRLUP, KEY_CTRLNUMPAD8, KEY_RCTRLNUMPAD8)? 5 : m_Where.top;
				[[fallthrough]];
			case KEY_UP:        case KEY_NUMPAD8:
				Hide();

				for (int i=0; i<rr; i++)
					if (m_Where.bottom > 0)
					{
						--m_Where.top;
						--m_Where.bottom;
						AdjustEditPos(0,-1);
					}

				Show();
				break;

			case KEY_CTRLDOWN:  case KEY_CTRLNUMPAD2:
			case KEY_RCTRLDOWN: case KEY_RCTRLNUMPAD2:
			case KEY_CTRLPGDN:  case KEY_CTRLNUMPAD3:
			case KEY_RCTRLPGDN: case KEY_RCTRLNUMPAD3:
			case KEY_PGDN:      case KEY_NUMPAD3:
				rr = any_of(Key, KEY_CTRLDOWN, KEY_RCTRLDOWN, KEY_CTRLNUMPAD2, KEY_RCTRLNUMPAD2)? 5 : std::max(0, ScrY - m_Where.bottom);
				[[fallthrough]];
			case KEY_DOWN:      case KEY_NUMPAD2:
				Hide();

				for (int i=0; i<rr; i++)
					if (m_Where.top < ScrY)
					{
						++m_Where.top;
						++m_Where.bottom;
						AdjustEditPos(0,1);
					}

				Show();
				break;

			case KEY_NUMENTER:
			case KEY_ENTER:
			case KEY_CTRLF5:
			case KEY_RCTRLF5:
				DialogMode.Clear(DMODE_KEYDRAGGED); // закончим движение!
				DlgProc(DN_DRAGGED, 1, nullptr);
				Show();
				break;

			case KEY_ESC:
				Hide();
				AdjustEditPos(m_Drag.OldRect.left - m_Where.left, m_Drag.OldRect.top - m_Where.top);
				m_Where = m_Drag.OldRect;
				DialogMode.Clear(DMODE_KEYDRAGGED);

				DlgProc(DN_DRAGGED,1,ToPtr(TRUE));
				Show();
				break;
		}

		return true;
	}

	if (any_of(Key, KEY_CTRLF5, KEY_RCTRLF5) && DialogMode.Check(DMODE_ISCANMOVE) && !DialogMode.Check(DMODE_MOUSEDRAGGED))
	{
		if (DlgProc(DN_DRAGGED, 0, nullptr)) // если разрешили перемещать!
		{
			// включаем флаг и запоминаем координаты
			DialogMode.Set(DMODE_KEYDRAGGED);
			m_Drag.OldRect = m_Where;
			//# GetText(0,0,3,0,LV);
			Show();
			return true;
		}
	}

	return false;
}

long long Dialog::VMProcess(int OpCode,void *vParam,long long iParam)
{
	_DIALOG(CleverSysLog CL(L"Dialog::VMProcess()"));
	switch (OpCode)
	{
		case MCODE_F_MENU_CHECKHOTKEY:
		case MCODE_F_MENU_GETHOTKEY:
		case MCODE_F_MENU_SELECT:
		case MCODE_F_MENU_GETVALUE:
		case MCODE_F_MENU_ITEMSTATUS:
		case MCODE_V_MENU_VALUE:
		case MCODE_F_MENU_FILTER:
		case MCODE_F_MENU_FILTERSTR:
		{
			const auto str = static_cast<const wchar_t*>(vParam);

			if (GetDropDownOpened() || Items[m_FocusPos].Type == DI_LISTBOX)
			{
				if (Items[m_FocusPos].ListPtr)
					return Items[m_FocusPos].ListPtr->VMProcess(OpCode,vParam,iParam);
			}
			else if (OpCode == MCODE_F_MENU_CHECKHOTKEY)
				return CheckHighlights(*str, static_cast<int>(iParam)) + 1;

			return 0;
		}
	}

	switch (OpCode)
	{
		case MCODE_C_EOF:
		case MCODE_C_BOF:
		case MCODE_C_SELECTED:
		case MCODE_C_EMPTY:
		{
			if (IsEdit(Items[m_FocusPos].Type))
			{
				if (Items[m_FocusPos].Type == DI_COMBOBOX && GetDropDownOpened())
					return Items[m_FocusPos].ListPtr->VMProcess(OpCode,vParam,iParam);
				else
					return static_cast<DlgEdit*>(Items[m_FocusPos].ObjPtr)->VMProcess(OpCode,vParam,iParam);
			}
			else if (Items[m_FocusPos].Type == DI_LISTBOX && OpCode != MCODE_C_SELECTED)
				return Items[m_FocusPos].ListPtr->VMProcess(OpCode,vParam,iParam);

			return 0;
		}
		case MCODE_V_DLGITEMTYPE:
		{
			switch (Items[m_FocusPos].Type)
			{
				case DI_BUTTON:      return 7; // Кнопка (Push Button).
				case DI_CHECKBOX:    return 8; // Контрольный переключатель (Check Box).
				case DI_COMBOBOX:    return DropDownOpened? 0x800A : 10; // Комбинированный список.
				case DI_DOUBLEBOX:   return 3; // Двойная рамка.
				case DI_EDIT:        return DropDownOpened? 0x8004 : 4; // Поле ввода.
				case DI_FIXEDIT:     return 6; // Поле ввода фиксированного размера.
				case DI_LISTBOX:     return 11; // Окно списка.
				case DI_PSWEDIT:     return 5; // Поле ввода пароля.
				case DI_RADIOBUTTON: return 9; // Селекторная кнопка (Radio Button).
				case DI_SINGLEBOX:   return 2; // Одиночная рамка.
				case DI_TEXT:        return 0; // Текстовая строка.
				case DI_USERCONTROL: return 255; // Элемент управления, определяемый программистом.
				case DI_VTEXT:       return 1; // Вертикальная текстовая строка.
				default:
					break;
			}

			return -1;
		}
		case MCODE_V_DLGITEMCOUNT: // Dlg->ItemCount
		{
			return Items.size();
		}
		case MCODE_V_DLGCURPOS:    // Dlg->CurPos
		{
			return m_FocusPos+1;
		}
		case MCODE_V_DLGPREVPOS:    // Dlg->PrevPos
		{
			return PrevFocusPos+1;
		}
		case MCODE_V_DLGINFOID:        // Dlg->Info.Id
		{
			static string strId;
			strId = uuid::str(m_Id);
			return reinterpret_cast<intptr_t>(UNSAFE_CSTR(strId));
		}
		case MCODE_V_DLGINFOOWNER:        // Dlg->Info.Owner
		{
			const auto OwnerId = PluginOwner? PluginOwner->Id() : FarUuid;
			static string strOwnerId;
			strOwnerId = uuid::str(OwnerId);
			return reinterpret_cast<intptr_t>(UNSAFE_CSTR(strOwnerId));
		}
		case MCODE_V_ITEMCOUNT:
		case MCODE_V_CURPOS:
		{
			long long Ret=0;
			switch (Items[m_FocusPos].Type)
			{
				case DI_COMBOBOX:

					if (DropDownOpened || (Items[m_FocusPos].Flags & DIF_DROPDOWNLIST))
					{
						Ret=Items[m_FocusPos].ListPtr->VMProcess(OpCode,vParam,iParam);
						break;
					}
					[[fallthrough]];
				case DI_EDIT:
				case DI_PSWEDIT:
				case DI_FIXEDIT:
					return static_cast<DlgEdit*>(Items[m_FocusPos].ObjPtr)->VMProcess(OpCode,vParam,iParam);

				case DI_LISTBOX:
					Ret=Items[m_FocusPos].ListPtr->VMProcess(OpCode,vParam,iParam);
					break;

				case DI_USERCONTROL:
				{
					if (OpCode == MCODE_V_CURPOS)
						Ret=Items[m_FocusPos].UCData->CursorPos.X;
					break;
				}
				case DI_BUTTON:
				case DI_CHECKBOX:
				case DI_RADIOBUTTON:
					return 0;
				default:
					return 0;
			}

			FarGetValue fgv={sizeof(FarGetValue),OpCode==MCODE_V_ITEMCOUNT?11:7,Ret};

			if (SendMessage(DN_GETVALUE,m_FocusPos,&fgv))
			{
				switch (fgv.Value.Type)
				{
					case FMVT_INTEGER:
						Ret=fgv.Value.Integer;
						break;
					default:
						Ret=0;
						break;
				}
			}

			return Ret;
		}
		case MCODE_F_EDITOR_SEL:
		{
			if (IsEdit(Items[m_FocusPos].Type) || (Items[m_FocusPos].Type==DI_COMBOBOX && !(DropDownOpened || (Items[m_FocusPos].Flags & DIF_DROPDOWNLIST))))
			{
				return static_cast<DlgEdit*>(Items[m_FocusPos].ObjPtr)->VMProcess(OpCode,vParam,iParam);
			}

			return 0;
		}

		default:
			break;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
/* Public, Virtual:
   Обработка данных от клавиатуры.
   Перекрывает BaseInput::ProcessKey.
*/
bool Dialog::ProcessKey(const Manager::Key& Key)
{
	auto LocalKey = Key;
	_DIALOG(CleverSysLog CL(L"Dialog::ProcessKey"));
	_DIALOG(SysLog(L"Param: Key=%s",_FARKEY_ToName(LocalKey())));

	assert(Key.IsEvent());
	if (DialogMode.Check(DMODE_INPUTEVENT) && Key.IsReal())
	{
		INPUT_RECORD rec=Key.Event();
		if (!DlgProc(DN_INPUT,0,&rec))
			return true;
	}

	if (any_of(LocalKey(), KEY_NONE, KEY_IDLE))
	{
		DlgProc(DN_ENTERIDLE, 0, nullptr); // $ 28.07.2000 SVS Передадим этот факт в обработчик :-)
		return false;
	}

	if (ProcessMoveDialog(LocalKey()))
		return true;

	if (!in_closed_range(KEY_OP_BASE, LocalKey(), KEY_OP_ENDBASE) && !DialogMode.Check(DMODE_KEY))
	{
		// wrap-stop mode for user lists
		if (any_of(LocalKey(), KEY_UP, KEY_NUMPAD8, KEY_DOWN, KEY_NUMPAD2) && IsRepeatedKey())
		{
			int n = -1;

			FarGetValue fgv = {sizeof(FarGetValue),11, -1}; // Item Count
			if (SendMessage(DN_GETVALUE,m_FocusPos,&fgv) && fgv.Value.Type==FMVT_INTEGER)
				n = static_cast<int>(fgv.Value.Integer);

			if (n > 1)
			{
				fgv.Type = 7; // Current Item
				fgv.Value.Integer = -1;
				int pos = -1;
				if (SendMessage(DN_GETVALUE,m_FocusPos,&fgv) && fgv.Value.Type==FMVT_INTEGER)
					pos = static_cast<int>(fgv.Value.Integer);

				const auto up = any_of(LocalKey(), KEY_UP, KEY_NUMPAD8);

				if ((pos==1 && up) || (pos==n && !up))
					return true;
				else if (pos==2 && up)   // workaround for first not selectable
					LocalKey = KEY_HOME;
				else if (pos==n-1 && !up) // workaround for last not selectable
					LocalKey = KEY_END;
			}
		}
		assert(Key.IsEvent());
		INPUT_RECORD rec=Key.Event();
		if (DlgProc(DN_CONTROLINPUT,m_FocusPos,&rec))
			return true;
	}

	if (!DialogMode.Check(DMODE_SHOW))
		return true;

	// А ХЗ, может в этот момент изменилось состояние элемента!
	if (Items[m_FocusPos].Flags&DIF_HIDDEN)
		return true;

	// небольшая оптимизация
	if (Items[m_FocusPos].Type==DI_CHECKBOX)
	{
		if (!(Items[m_FocusPos].Flags&DIF_3STATE))
		{
			if (LocalKey() == KEY_MULTIPLY) // в CheckBox 2-state Gray* не работает!
				LocalKey = KEY_NONE;

			if ((LocalKey() == KEY_ADD      && !Items[m_FocusPos].Selected) ||
			        (LocalKey() == KEY_SUBTRACT &&  Items[m_FocusPos].Selected))
				LocalKey=KEY_SPACE;
		}

		/*
		  блок else не нужен, т.к. ниже клавиши будут обработаны...
		*/
	}
	else if (LocalKey() == KEY_ADD)
		LocalKey='+';
	else if (LocalKey() == KEY_SUBTRACT)
		LocalKey='-';
	else if (LocalKey() == KEY_MULTIPLY)
		LocalKey='*';

	if (Items[m_FocusPos].Type==DI_BUTTON && LocalKey() == KEY_SPACE)
		LocalKey=KEY_ENTER;

	if (Items[m_FocusPos].Type == DI_LISTBOX)
	{
		switch (LocalKey())
		{
			case KEY_HOME:     case KEY_NUMPAD7:
			case KEY_LEFT:     case KEY_NUMPAD4:
			case KEY_END:      case KEY_NUMPAD1:
			case KEY_RIGHT:    case KEY_NUMPAD6:
			case KEY_UP:       case KEY_NUMPAD8:
			case KEY_DOWN:     case KEY_NUMPAD2:
			case KEY_PGUP:     case KEY_NUMPAD9:
			case KEY_PGDN:     case KEY_NUMPAD3:
			case KEY_MSWHEEL_UP:
			case KEY_MSWHEEL_DOWN:
			case KEY_MSWHEEL_LEFT:
			case KEY_MSWHEEL_RIGHT:
			case KEY_NUMENTER:
			case KEY_ENTER:
				auto& List = Items[m_FocusPos].ListPtr;
				int CurListPos=List->GetSelectPos();
				List->ProcessKey(Key);
				int NewListPos=List->GetSelectPos();

				if (NewListPos != CurListPos)
				{
					if (!DialogMode.Check(DMODE_SHOW))
						return true;

					if (!(Items[m_FocusPos].Flags&DIF_HIDDEN))
						ShowDialog(m_FocusPos); // FocusPos
				}

				if (none_of(LocalKey(), KEY_ENTER, KEY_NUMENTER) || Items[m_FocusPos].Flags & DIF_LISTNOCLOSE)
					return true;
		}
	}

	switch (LocalKey())
	{
		case KEY_F1:

			// Перед выводом диалога посылаем сообщение в обработчик
			//   и если вернули что надо, то выводим подсказку
			{
				const auto Topic = help::make_topic(PluginOwner, NullToEmpty(reinterpret_cast<const wchar_t*>(DlgProc(DN_HELP, m_FocusPos, const_cast<wchar_t*>(EmptyToNull(HelpTopic))))));
				if (!Topic.empty())
				{
					help::show(Topic);
				}
			}
			return true;

		case KEY_ESC:
		case KEY_BREAK:
		case KEY_F10:
			m_ExitCode=(LocalKey()==KEY_BREAK) ? -2:-1;
			CloseDialog();
			return true;

		case KEY_HOME: case KEY_NUMPAD7:

			if (Items[m_FocusPos].Type == DI_USERCONTROL) // для user-типа вываливаем
				return true;

			return Do_ProcessFirstCtrl();
		case KEY_TAB:
		case KEY_SHIFTTAB:
			return Do_ProcessTab(LocalKey()==KEY_TAB);
		case KEY_SPACE:
			return Do_ProcessSpace();
		case KEY_CTRLNUMENTER:
		case KEY_RCTRLNUMENTER:
		case KEY_CTRLENTER:
		case KEY_RCTRLENTER:
		{
			const auto ItemIterator = std::find_if(RANGE(Items, i)
			{
				return i.Flags & DIF_DEFAULTBUTTON;
			});

			if (ItemIterator != Items.end())
			{
				if (ItemIterator->Flags&DIF_DISABLE)
				{
					// ProcessKey(KEY_DOWN); // на твой вкус :-)
					return true;
				}

				if (!IsEdit(ItemIterator->Type))
					ItemIterator->Selected=1;

				m_ExitCode = ItemIterator - Items.begin();
				CloseDialog();
				return true;
			}

			if (!DialogMode.Check(DMODE_OLDSTYLE))
			{
				DialogMode.Clear(DMODE_ENDLOOP); // только если есть
				return true; // делать больше не чего
			}
		}
		[[fallthrough]];
		case KEY_NUMENTER:
		case KEY_ENTER:
		{
			if (IsEmulatedEditorLine(Items[m_FocusPos]) && !(Items[m_FocusPos].Flags & DIF_READONLY))
			{
				size_t I, EditorLastPos;

				for (EditorLastPos=I=m_FocusPos; I < Items.size(); I++)
					if (IsEmulatedEditorLine(Items[I]))
						EditorLastPos=I;
					else
						break;

				if (static_cast<DlgEdit*>(Items[EditorLastPos].ObjPtr)->GetLength())
					return true;

				const auto focus = static_cast<DlgEdit*>(Items[m_FocusPos].ObjPtr);
				auto strStr = focus->GetString();
				int CurPos = focus->GetCurPos();
				SCOPED_ACTION(SetAutocomplete)(focus);
				string strMove;
				if (CurPos < static_cast<int>(strStr.size()))
				{
					strMove = strStr.substr(CurPos);
					strStr.resize(CurPos);
					focus->SetString(strStr);
					focus->SetCurPos(0);
				}
				focus->SetString(strStr);
				focus->SetCurPos(0);

				for (const auto& Item: span(Items).subspan(m_FocusPos + 1, EditorLastPos - m_FocusPos))
				{
					const auto next = static_cast<DlgEdit*>(Item.ObjPtr);
					strStr = next->GetString();
					next->SetString(strMove);
					focus->SetCurPos(0);
					strMove = strStr;
				}
				Do_ProcessNextCtrl(false, true);
				if (m_FocusPos <= EditorLastPos)
					static_cast<DlgEdit*>(Items[m_FocusPos].ObjPtr)->Changed();
				ShowDialog();
				return true;
			}
			else if (Items[m_FocusPos].Type==DI_BUTTON)
			{
				Items[m_FocusPos].Selected=1;

				// сообщение - "Кнопка нажата"
				if (SendMessage(DN_BTNCLICK, m_FocusPos, nullptr))
					return true;

				if (Items[m_FocusPos].Flags&DIF_BTNNOCLOSE)
					return true;

				m_ExitCode=static_cast<int>(m_FocusPos);
				CloseDialog();
				return true;
			}
			else
			{
				m_ExitCode=-1;

				FOR_CONST_RANGE(Items, i)
				{
					if ((i->Flags&DIF_DEFAULTBUTTON) && !(i->Flags&DIF_BTNNOCLOSE))
					{
						if (i->Flags&DIF_DISABLE)
						{
							// ProcessKey(KEY_DOWN); // на твой вкус :-)
							return true;
						}

//            if (!(IsEdit(i->Type) || i->Type == DI_CHECKBOX || i->Type == DI_RADIOBUTTON))
//              i->Selected=1;
						m_ExitCode= i - Items.begin();
						break;
					}
				}
			}

			if (m_ExitCode==-1)
				m_ExitCode=static_cast<int>(m_FocusPos);

			CloseDialog();
			return true;
		}
		/*
		   3-х уровневое состояние
		   Для чекбокса сюда попадем только в случае, если контрол
		   имеет флаг DIF_3STATE
		*/
		case KEY_ADD:
		case KEY_SUBTRACT:
		case KEY_MULTIPLY:

			if (Items[m_FocusPos].Type==DI_CHECKBOX)
			{
				unsigned int CHKState=
				    (LocalKey() == KEY_ADD?1:
				     (LocalKey() == KEY_SUBTRACT?0:
				      ((LocalKey() == KEY_MULTIPLY)?2:
				       Items[m_FocusPos].Selected)));

				if (Items[m_FocusPos].Selected != static_cast<int>(CHKState))
					if (SendMessage(DN_BTNCLICK,m_FocusPos,ToPtr(CHKState)))
					{
						Items[m_FocusPos].Selected=CHKState;
						ShowDialog();
					}
			}

			return true;
		case KEY_LEFT:  case KEY_NUMPAD4: case KEY_MSWHEEL_LEFT:
		case KEY_RIGHT: case KEY_NUMPAD6: case KEY_MSWHEEL_RIGHT:
		{
			if (Items[m_FocusPos].Type == DI_USERCONTROL) // для user-типа вываливаем
				return true;

			if (IsEdit(Items[m_FocusPos].Type))
			{
				static_cast<DlgEdit*>(Items[m_FocusPos].ObjPtr)->ProcessKey(Key);
				return true;
			}
			else
			{
				size_t MinDist=1000, Pos = 0, MinPos=0;
				for (const auto &i :Items)
				{
					if (Pos != m_FocusPos &&
					        (IsEdit(i.Type) ||
					         i.Type==DI_RADIOBUTTON) &&
					         i.Y1==Items[m_FocusPos].Y1)
					{
						const auto Dist = i.X1-Items[m_FocusPos].X1;

						if ((any_of(LocalKey(), KEY_LEFT, KEY_SHIFTNUMPAD4) && Dist < 0) || (any_of(LocalKey(), KEY_RIGHT, KEY_SHIFTNUMPAD6) && Dist > 0))
						{
							if (static_cast<size_t>(abs(Dist))<MinDist)
							{
								MinDist=static_cast<size_t>(abs(Dist));
								MinPos = Pos;
							}
						}
					}
					++Pos;
				}

				if (MinDist<1000)
				{
					ChangeFocus2(MinPos);

					if (Items[MinPos].Flags & DIF_MOVESELECT)
					{
						Do_ProcessSpace();
					}
					else
					{
						ShowDialog();
					}

					return true;
				}
			}
		}
		[[fallthrough]];
		case KEY_UP:    case KEY_NUMPAD8:
		case KEY_DOWN:  case KEY_NUMPAD2:

			if (Items[m_FocusPos].Type == DI_USERCONTROL) // для user-типа вываливаем
				return true;

			return Do_ProcessNextCtrl(any_of(LocalKey(), KEY_LEFT, KEY_UP, KEY_NUMPAD4, KEY_NUMPAD8));
			// $ 27.04.2001 VVM - Обработка колеса мышки
		case KEY_MSWHEEL_UP:
		case KEY_MSWHEEL_DOWN:
		case KEY_CTRLUP:      case KEY_CTRLNUMPAD8:
		case KEY_RCTRLUP:     case KEY_RCTRLNUMPAD8:
		case KEY_CTRLDOWN:    case KEY_CTRLNUMPAD2:
		case KEY_RCTRLDOWN:   case KEY_RCTRLNUMPAD2:
			return ProcessOpenComboBox(Items[m_FocusPos].Type, &Items[m_FocusPos], m_FocusPos);

		case KEY_F11:
			if (!Global->IsProcessAssignMacroKey)
			{
				if (!CheckDialogMode(DMODE_NOPLUGINS))
					return Global->WindowManager->ProcessKey(Key);
			}
			break;

			// ЭТО перед default предпоследний!!!
		case KEY_END:  case KEY_NUMPAD1:

			if (Items[m_FocusPos].Type == DI_USERCONTROL) // для user-типа вываливаем
				return true;

			if (IsEdit(Items[m_FocusPos].Type))
			{
				static_cast<DlgEdit*>(Items[m_FocusPos].ObjPtr)->ProcessKey(Key);
				return true;
			}

			[[fallthrough]];
			// ???
			// ЭТО перед default последний!!!
		case KEY_PGDN:   case KEY_NUMPAD3:

			if (Items[m_FocusPos].Type == DI_USERCONTROL) // для user-типа вываливаем
				return true;

			if (IsEmulatedEditorLine(Items[m_FocusPos]))
				// для DIF_EDITOR будет обработано ниже [[fallthrough]]
				;
			else
			{
				const auto ItemIterator = std::find_if(CONST_RANGE(Items, i)
				{
					return i.Flags&DIF_DEFAULTBUTTON;
				});
				if (ItemIterator != Items.cend())
				{
					ChangeFocus2(ItemIterator - Items.begin());
					ShowDialog();
					return true;
				}
				return true;
			}
			[[fallthrough]];
		default:
		{
			//if(Items[FocusPos].Type == DI_USERCONTROL) // для user-типа вываливаем
			//  return true;
			if (Items[m_FocusPos].Type == DI_LISTBOX)
			{
				auto& List = Items[m_FocusPos].ListPtr;
				int CurListPos=List->GetSelectPos();
				List->ProcessKey(Key);
				int NewListPos=List->GetSelectPos();

				if (NewListPos != CurListPos)
				{
					if (!DialogMode.Check(DMODE_SHOW))
						return true;

					if (!(Items[m_FocusPos].Flags&DIF_HIDDEN))
						ShowDialog(m_FocusPos); // FocusPos
				}

				return true;
			}

			if (IsEdit(Items[m_FocusPos].Type))
			{
				const auto edt = static_cast<DlgEdit*>(Items[m_FocusPos].ObjPtr);

				if (any_of(LocalKey(), KEY_CTRLL, KEY_RCTRLL)) // исключим смену режима RO для поля ввода с клавиатуры
				{
					return true;
				}

				if (any_of(LocalKey(), KEY_CTRLU, KEY_RCTRLU))
				{
					edt->SetClearFlag(false);
					edt->RemoveSelection();
					edt->Show();
					return true;
				}

				if (IsEmulatedEditorLine(Items[m_FocusPos]) && !(Items[m_FocusPos].Flags & DIF_READONLY))
				{
					switch (LocalKey())
					{
						case KEY_BS:
						{	// В начале строки????
							if (!edt->GetCurPos())
							{	// а "выше" тоже DIF_EDITOR?
								if (m_FocusPos > 0 && IsEmulatedEditorLine(Items[m_FocusPos - 1]))
								{	// добавляем к предыдущему и...
									bool last = false;
									const auto prev = static_cast<DlgEdit*>(Items[m_FocusPos - 1].ObjPtr);
									auto strStr = prev->GetString();
									int pos = static_cast<int>(strStr.size());
									for (size_t I = m_FocusPos; !last && I < Items.size(); ++I)
									{
										const auto& Item = Items[I];

										const auto next = static_cast<const DlgEdit*>(Item.ObjPtr);
										last = !IsEmulatedEditorLine(Item);
										if (!last)
										{
											strStr += next->GetString();
										}
										const auto LocalPrev = static_cast<DlgEdit*>(Items[I - 1].ObjPtr);
										SCOPED_ACTION(SetAutocomplete)(LocalPrev);
										LocalPrev->SetString(strStr);
										strStr.clear();
									}
									Do_ProcessNextCtrl(true);
									prev->SetCurPos(pos);
									prev->Changed();
								}
							}
							else
							{
								edt->ProcessKey(Key);
							}

							ShowDialog();
							return true;
						}
						case KEY_CTRLY:
						case KEY_RCTRLY:
						{
							bool empty = true, last = false;
							for (size_t I = m_FocusPos+1; !last && I < Items.size(); ++I)
							{
								const auto& Item = Items[I];

								last = !IsEmulatedEditorLine(Item);
								string strNext;
								if (!last)
									strNext = static_cast<DlgEdit*>(Item.ObjPtr)->GetString();
								const auto prev = static_cast<DlgEdit*>(Items[I - 1].ObjPtr);
								int CurPos = prev->GetCurPos();
								SCOPED_ACTION(SetAutocomplete)(prev);
								prev->SetString(strNext);
								prev->SetCurPos(CurPos);
								empty = empty && strNext.empty();
							}
							if (empty)
								static_cast<DlgEdit*>(Items[m_FocusPos].ObjPtr)->SetCurPos(0);
							static_cast<DlgEdit*>(Items[m_FocusPos].ObjPtr)->Changed();
							ShowDialog();
							return true;
						}
						case KEY_NUMDEL:
						case KEY_DEL:
						{
							if (m_FocusPos<Items.size() + 1 && IsEmulatedEditorLine(Items[m_FocusPos + 1]))
							{
								const int CurPos=edt->GetCurPos();
								const int Length=edt->GetLength();
								intptr_t SelStart, SelEnd;
								edt->GetSelection(SelStart, SelEnd);
								auto strStr = edt->GetString();

								if (SelStart > -1)
								{
									const auto strEnd = strStr.substr(SelEnd);
									strStr.resize(SelStart);
									strStr+=strEnd;
									edt->SetString(strStr);
									edt->SetCurPos(SelStart);
									ShowDialog();
									return true;
								}
								else if (CurPos>=Length)
								{
									const auto edt_1 = static_cast<DlgEdit*>(Items[m_FocusPos + 1].ObjPtr);
									if (CurPos > Length)
									{
										strStr.resize(CurPos, L' ');
									}
									SCOPED_ACTION(SetAutocomplete)(edt_1);
									edt_1->SetString(strStr + edt_1->GetString());
									return ProcessKey(Manager::Key(KEY_CTRLY));
								}
							}

							break;
						}
						case KEY_PGDN:  case KEY_NUMPAD3:
						case KEY_PGUP:  case KEY_NUMPAD9:
						{
							size_t I = m_FocusPos;

							while (IsEmulatedEditorLine(Items[I]))
								I = ChangeFocus(I, any_of(LocalKey(), KEY_PGUP, KEY_NUMPAD9)? -1 : 1, false);

							if (!IsEmulatedEditorLine(Items[I]))
								I = ChangeFocus(I, any_of(LocalKey(), KEY_PGUP, KEY_NUMPAD9)? 1 : -1, false);

							ChangeFocus2(I);
							ShowDialog();

							return true;
						}
					}
				}

				if (LocalKey() == KEY_OP_XLAT && !(Items[m_FocusPos].Flags & DIF_READONLY))
				{
					edt->SetClearFlag(false);
					edt->Xlat();

					// иначе неправильно работает ctrl-end
					edt->strLastStr = edt->GetString();
					edt->LastPartLength=static_cast<int>(edt->strLastStr.size());

					Redraw(); // Перерисовка должна идти после DN_EDITCHANGE (imho)
					return true;
				}

				if (!(Items[m_FocusPos].Flags & DIF_READONLY) || IsNavKey(LocalKey()))
				{
					if (any_of(LocalKey(), KEY_CTRLSPACE, KEY_RCTRLSPACE))
					{
						SCOPED_ACTION(SetAutocomplete)(edt, true);
						edt->AutoComplete(true,false);
						Redraw();
						return true;
					}

					if (edt->ProcessKey(Key))
					{
						if (Items[m_FocusPos].Flags & DIF_READONLY)
							return true;

						if (any_of(LocalKey(), KEY_CTRLEND, KEY_RCTRLEND, KEY_CTRLNUMPAD1, KEY_RCTRLNUMPAD1) && edt->GetCurPos()==edt->GetLength())
						{
							if (edt->LastPartLength ==-1)
								edt->strLastStr = edt->GetString();

							auto strStr = edt->strLastStr;
							int CurCmdPartLength=static_cast<int>(strStr.size());
							edt->HistoryGetSimilar(strStr, edt->LastPartLength);

							if (edt->LastPartLength == -1)
							{
								edt->strLastStr = edt->GetString();
								edt->LastPartLength = CurCmdPartLength;
							}
							{
								SCOPED_ACTION(SetAutocomplete)(edt);
								edt->SetString(strStr);
								edt->Select(edt->LastPartLength, static_cast<int>(strStr.size()));
							}
							Show();
							return true;
						}

						edt->LastPartLength=-1;

						Redraw(); // Перерисовка должна идти после DN_EDITCHANGE (imho)
						return true;
					}
				}
				else if (!(LocalKey()&(KEY_ALT|KEY_RALT)))
					return true;
			}

			if (ProcessHighlighting(LocalKey(),m_FocusPos,FALSE))
				return true;

			return ProcessHighlighting(LocalKey(),m_FocusPos,TRUE);
		}
	}
	return false;
}

void Dialog::ProcessKey(int Key, size_t ItemPos)
{
	const auto SavedFocusPos = m_FocusPos;
	m_FocusPos = ItemPos;
	ProcessKey(Manager::Key(Key));
	if (m_FocusPos == ItemPos)
		m_FocusPos = SavedFocusPos;
}


//////////////////////////////////////////////////////////////////////////
/* Public, Virtual:
   Обработка данных от "мыши".
   Перекрывает BaseInput::ProcessMouse.
*/
/* $ 18.08.2000 SVS
   + DN_MOUSECLICK
*/
bool Dialog::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	SMALL_RECT Rect;
	INPUT_RECORD mouse = {};
	mouse.EventType=MOUSE_EVENT;
	mouse.Event.MouseEvent=*MouseEvent;
	MOUSE_EVENT_RECORD &MouseRecord=mouse.Event.MouseEvent;

	if (!DialogMode.Check(DMODE_SHOW))
		return false;

	if (DialogMode.Check(DMODE_INPUTEVENT))
	{
		if (!DlgProc(DN_INPUT,0,&mouse))
			return true;
	}

	if (!DialogMode.Check(DMODE_SHOW))
		return false;

	if (DialogMode.Check(DMODE_MOUSEDRAGGED))
	{
		ProcessDrag(MouseEvent);
		return true;
	}

	const auto MsX = MouseRecord.dwMousePosition.X;
	const auto MsY = MouseRecord.dwMousePosition.Y;

	//for (I=0;I<ItemCount;I++)
	for (size_t I = Items.size() - 1; I != static_cast<size_t>(-1); I--)
	{
		const auto& Item = Items[I];

		if (Item.Flags&(DIF_DISABLE|DIF_HIDDEN))
			continue;

		if (Item.Type == DI_LISTBOX &&
			MsY >= m_Where.top + Item.Y1 && MsY <= m_Where.top + Item.Y2 &&
			MsX >= m_Where.left + Item.X1 && MsX <= m_Where.left + Item.X2
		)
		{
			auto& List = Item.ListPtr;
			if (!MouseRecord.dwEventFlags && !(MouseRecord.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) && (PrevMouseRecord.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED))
			{
				if (PrevMouseRecord.dwMousePosition.X==MsX && PrevMouseRecord.dwMousePosition.Y==MsY)
				{
					m_ExitCode=static_cast<int>(I);
					CloseDialog();
					return true;
				}
				PrevMouseRecord=MouseRecord;
			}

			if ((MouseRecord.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED))
			{
				if (m_FocusPos != I)
				{
					ChangeFocus2(I);
					ShowDialog();
				}

				if (MouseRecord.dwEventFlags!=DOUBLE_CLICK && !(Item.Flags&(DIF_LISTTRACKMOUSE|DIF_LISTTRACKMOUSEINFOCUS)))
				{
					List->ProcessMouse(&MouseRecord);
				}
				else
				{
					if (SendMessage(DN_CONTROLINPUT,I,&mouse))
					{
						return true;
					}
					List->ProcessMouse(&MouseRecord);
					const auto InScroolBar = (MsX == m_Where.left + Item.X2 && MsY >= m_Where.top + Item.Y1 && MsY <= m_Where.top + Item.Y2) &&
					                (List->CheckFlags(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR) || Global->Opt->ShowMenuScrollbar);
					if (List->GetLastSelectPosResult() >= 0)
					{
						if (List->CheckFlags(VMENU_SHOWNOBOX) || (MsY > m_Where.top + Item.Y1 && MsY < m_Where.top + Item.Y2))
						{
							if (!InScroolBar && !(Item.Flags&DIF_LISTNOCLOSE))
							{
								if (MouseRecord.dwEventFlags==DOUBLE_CLICK)
								{
									m_ExitCode=static_cast<int>(I);
									CloseDialog();
									return true;
								}
								if (!MouseRecord.dwEventFlags && (MouseRecord.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) && !(PrevMouseRecord.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED))
									PrevMouseRecord=MouseRecord;
							}
						}
					}
				}

				return true;
			}
			else
			{
				if (!mouse.Event.MouseEvent.dwButtonState || SendMessage(DN_CONTROLINPUT,I,&mouse))
				{
					if ((I == m_FocusPos && (Item.Flags&DIF_LISTTRACKMOUSEINFOCUS)) || (Item.Flags&DIF_LISTTRACKMOUSE))
					{
						List->ProcessMouse(&mouse.Event.MouseEvent);
					}
				}
			}

			return true;
		}
	}

	if (!m_Where.contains(MouseRecord.dwMousePosition))
	{
		if (DialogMode.Check(DMODE_CLICKOUTSIDE) && !DlgProc(DN_CONTROLINPUT,-1,&mouse))
		{
			if (!DialogMode.Check(DMODE_SHOW))
				return false;

			if (!(mouse.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) && (IntKeyState.PrevMouseButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) && (Global->Opt->Dialogs.MouseButton&DMOUSEBUTTON_LEFT))
				ProcessKey(Manager::Key(KEY_ESC));
			else if (!(mouse.Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED) && (IntKeyState.PrevMouseButtonState&RIGHTMOST_BUTTON_PRESSED) && (Global->Opt->Dialogs.MouseButton&DMOUSEBUTTON_RIGHT))
				ProcessKey(Manager::Key(KEY_ENTER));
		}
		else if (DialogMode.Check(DMODE_CLICKOUTSIDE))
		{
			DialogMode.Clear(DMODE_CLICKOUTSIDE);
			return true;
		}

		if (mouse.Event.MouseEvent.dwButtonState)
			DialogMode.Set(DMODE_CLICKOUTSIDE);

		return true;
	}

	if (!mouse.Event.MouseEvent.dwButtonState)
	{
		DialogMode.Clear(DMODE_CLICKOUTSIDE);
		return false;
	}

	if (!mouse.Event.MouseEvent.dwEventFlags || mouse.Event.MouseEvent.dwEventFlags==DOUBLE_CLICK)
	{
		// первый цикл - все за исключением рамок.
		//for (I=0; I < ItemCount;I++)
		for (size_t I = Items.size() - 1; I != static_cast<size_t>(-1); I--)
		{
			const auto& Item = Items[I];

			if (Item.Flags&(DIF_DISABLE|DIF_HIDDEN))
				continue;

			GetItemRect(I,Rect);
			Rect.Left += m_Where.left;
			Rect.Top += m_Where.top;
			Rect.Right += m_Where.left;
			Rect.Bottom += m_Where.top;

			if (MsX >= Rect.Left && MsY >= Rect.Top && MsX <= Rect.Right && MsY <= Rect.Bottom)
			{
				// для прозрачных :-)
				if (Item.Type == DI_SINGLEBOX || Item.Type == DI_DOUBLEBOX)
				{
					// если на рамке, то...
					if (MsX == Rect.Left || MsX == Rect.Right || MsY == Rect.Top  || MsY == Rect.Bottom)
					{
						if (DlgProc(DN_CONTROLINPUT,I,&mouse))
							return true;

						if (!DialogMode.Check(DMODE_SHOW))
							return false;
					}
					else
						continue;
				}

				if (Item.Type == DI_USERCONTROL)
				{
					// для user-типа подготовим координаты мыши
					mouse.Event.MouseEvent.dwMousePosition.X-=Rect.Left;
					mouse.Event.MouseEvent.dwMousePosition.Y-=Rect.Top;
				}

				if (DlgProc(DN_CONTROLINPUT,I,&mouse))
					return true;

				if (!DialogMode.Check(DMODE_SHOW))
					return true;

				if (Item.Type == DI_USERCONTROL)
				{
					ChangeFocus2(I);
					ShowDialog();
					return true;
				}

				break;
			}
		}

		if ((mouse.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED))
		{
			for (size_t I = Items.size() - 1; I != static_cast<size_t>(-1); I--)
			{
				auto& Item = Items[I];

				//   Исключаем из списка оповещаемых о мыши недоступные элементы
				if (Item.Flags&(DIF_DISABLE|DIF_HIDDEN))
					continue;

				GetItemRect(I,Rect);
				Rect.Left += m_Where.left;
				Rect.Top += m_Where.top;
				Rect.Right += m_Where.left;
				Rect.Bottom += m_Where.top;
				if (ItemHasDropDownArrow(&Item))
					Rect.Right++;

				if (MsX >= Rect.Left && MsY >= Rect.Top && MsX <= Rect.Right && MsY <= Rect.Bottom)
				{
					/* ********************************************************** */
					if (IsEdit(Item.Type))
					{
						/* $ 15.08.2000 SVS
						   + Сделаем так, чтобы ткнув мышкой в DropDownList
						     список раскрывался сам.
						   Есть некоторая глюкавость - когда список раскрыт и мы
						   мышой переваливаем на другой элемент, то список закрывается
						   но перехода реального на указанный элемент диалога не происходит
						*/
						const auto EditLine = static_cast<DlgEdit*>(Item.ObjPtr);
						const auto EditRect = EditLine->GetPosition();

						if (MsY == EditRect.top && Item.Type == DI_COMBOBOX &&
						        (Item.Flags & DIF_DROPDOWNLIST) &&
						        MsX >= EditRect.left && MsX <= EditRect.right + 1)
						{
							EditLine->SetClearFlag(false);

							ChangeFocus2(I);
							ShowDialog();

							ProcessOpenComboBox(Item.Type, &Item, I);

							return true;
						}

						ChangeFocus2(I);

						if (EditLine->ProcessMouse(&mouse.Event.MouseEvent))
						{
							EditLine->SetClearFlag(false); // а может это делать в самом edit?

							/* $ 23.06.2001 KM
							   ! Оказалось нужно перерисовывать весь диалог иначе
							     не снимался признак активности с комбобокса с которго уходим.
							*/
							ShowDialog(); // нужен ли только один контрол или весь диалог?
							return true;
						}
						else
						{
							// Проверка на DI_COMBOBOX здесь лишняя. Убрана (KM).
							if (MsX == EditRect.right + 1 && MsY == EditRect.top && ItemHasDropDownArrow(&Item))
							{
								EditLine->SetClearFlag(false); // раз уж покусились на, то и...

								ChangeFocus2(I);

								if (!(Item.Flags&DIF_HIDDEN))
									ShowDialog(I);

								ProcessOpenComboBox(Item.Type, &Item, I);

								return true;
							}
						}
					}

					/* ********************************************************** */
					if (Item.Type==DI_BUTTON &&
						MsY == m_Where.top + Item.Y1 &&
						MsX < m_Where.left + Item.X1 + static_cast<intptr_t>(HiStrlen(Item.strData)))
					{
						ChangeFocus2(I);
						ShowDialog();

						while (IsMouseButtonPressed())
							;

						if (IntKeyState.MousePos.x < m_Where.left ||
							IntKeyState.MousePos.x > m_Where.left + Item.X1 + static_cast<intptr_t>(HiStrlen(Item.strData)) + 4 ||
							IntKeyState.MousePos.y != m_Where.top + Item.Y1)
						{
							ChangeFocus2(I);
							ShowDialog();

							return true;
						}

						ProcessKey(KEY_ENTER, I);
						return true;
					}

					/* ********************************************************** */
					if ((Item.Type == DI_CHECKBOX || Item.Type == DI_RADIOBUTTON) &&
						MsY == m_Where.top + Item.Y1 &&
						MsX < (m_Where.left + Item.X1 + static_cast<intptr_t>(HiStrlen(Item.strData)) + 4 - ((Item.Flags & DIF_MOVESELECT) != 0)))
					{
						ChangeFocus2(I);
						ProcessKey(KEY_SPACE, I);
						return true;
					}
				}
			} // for (I=0;I<ItemCount;I++)

			// ДЛЯ MOUSE-Перемещалки:
			//   Сюда попадаем в том случае, если мышь не попала на активные элементы
			//

			if (DialogMode.Check(DMODE_ISCANMOVE) && !DialogMode.Check(DMODE_KEYDRAGGED))
			{
				if (DlgProc(DN_DRAGGED, 0, nullptr))
				{
					DialogMode.Set(DMODE_MOUSEDRAGGED);
					m_Drag.OldRect = m_Where;
					// запомним delta места хватания и Left-Top диалогового окна
					m_Drag.MsX = abs(m_Where.left - IntKeyState.MousePos.x);
					m_Drag.MsY = abs(m_Where.top - IntKeyState.MousePos.y);
					Show();
				}
			}
		}
	}

	return false;
}

void Dialog::ProcessDrag(const MOUSE_EVENT_RECORD *MouseEvent)
{
	const auto buttons = MouseEvent->dwButtonState;
	if (buttons&FROM_LEFT_1ST_BUTTON_PRESSED) // still dragging
	{
		int mx,my;
		if (IntKeyState.MousePos.x == IntKeyState.MousePrevPos.x)
			mx = m_Where.left;
		else
			mx = IntKeyState.MousePos.x - m_Drag.MsX;

		if (IntKeyState.MousePos.y == IntKeyState.MousePrevPos.y)
			my = m_Where.top;
		else
			my = IntKeyState.MousePos.y - m_Drag.MsY;

		const auto X0 = m_Where.left, Y0 = m_Where.top;
		const auto OX1 = m_Where.left, OY1 = m_Where.top;
		const auto NX1 = mx, NX2 = mx + m_Where.width() - 1;
		const auto NY1 = my, NY2 = my + m_Where.height() - 1;
		const auto AdjX = NX1 - X0, AdjY = NY1 - Y0;

		if (OX1 != NX1 || OY1 != NY1)
		{
			Hide();

			m_Where.left = NX1;
			m_Where.right = NX2;
			m_Where.top = NY1;
			m_Where.bottom = NY2;

			if (AdjX || AdjY)
				AdjustEditPos(AdjX,AdjY); //?
			Show();
		}
	}
	else if (buttons == RIGHTMOST_BUTTON_PRESSED) // abort
	{
		Hide();
		AdjustEditPos(m_Drag.OldRect.left - m_Where.left, m_Drag.OldRect.top - m_Where.top);
		m_Where = m_Drag.OldRect;
		DialogMode.Clear(DMODE_MOUSEDRAGGED);
		DlgProc(DN_DRAGGED,1,ToPtr(TRUE));
		Show();
	}
	else // release key, drop dialog
	{
		Hide();
		DialogMode.Clear(DMODE_MOUSEDRAGGED);
		DlgProc(DN_DRAGGED, 1, nullptr);
		Show();
	}
}

bool Dialog::ProcessOpenComboBox(FARDIALOGITEMTYPES Type,DialogItemEx *CurItem, size_t CurFocusPos)
{
	// для user-типа вываливаем
	if (Type == DI_USERCONTROL)
		return true;

	const auto CurEditLine = static_cast<DlgEdit*>(CurItem->ObjPtr);

	if (IsEdit(Type) &&
	        (CurItem->Flags & DIF_HISTORY) &&
	        Global->Opt->Dialogs.EditHistory &&
	        !CurItem->strHistory.empty() &&
	        !(CurItem->Flags & DIF_READONLY))
	{
		// Передаем то, что в строке ввода в функцию выбора из истории для выделения нужного пункта в истории.
		SelectFromEditHistory(CurItem, CurEditLine, CurItem->strHistory);
	}
	// $ 18.07.2000 SVS:  +обработка DI_COMBOBOX - выбор из списка!
	else if (Type == DI_COMBOBOX && CurItem->ListPtr &&
	         !(CurItem->Flags & DIF_READONLY) &&
	         CurItem->ListPtr->HasVisible()) //??
	{
		SelectFromComboBox(CurItem, CurEditLine);
	}

	return true;
}

size_t Dialog::ProcessRadioButton(size_t CurRB, bool UncheckAll)
{
	size_t PrevRB=CurRB, I;

	for (I=CurRB;; I--)
	{
		if (!I)
			break;

		if (Items[I].Type==DI_RADIOBUTTON && (Items[I].Flags & DIF_GROUP))
			break;

		if (Items[I-1].Type!=DI_RADIOBUTTON)
			break;
	}

	do
	{
		/* $ 28.07.2000 SVS
		  При изменении состояния каждого элемента посылаем сообщение
		  посредством функции SendDlgMessage - в ней делается все!
		*/
		const auto J = Items[I].Selected;
		Items[I].Selected=0;

		if (J)
		{
			PrevRB=I;
		}

		++I;
	}
	while (I<Items.size() && Items[I].Type==DI_RADIOBUTTON &&
	        !(Items[I].Flags & DIF_GROUP));

	Items[CurRB].Selected = !UncheckAll;

	auto ret = CurRB;
	const auto focus = m_FocusPos;

	/* $ 28.07.2000 SVS
	  При изменении состояния каждого элемента посылаем сообщение
	  посредством функции SendDlgMessage - в ней делается все!
	*/
	if (!SendMessage(DN_BTNCLICK, PrevRB, nullptr) ||
		!SendMessage(DN_BTNCLICK,CurRB,ToPtr(!UncheckAll)))
	{
		// вернем назад, если пользователь не захотел...
		Items[CurRB].Selected=0;
		Items[PrevRB].Selected=1;
		ret = PrevRB;
	}

	return focus == m_FocusPos ? ret : m_FocusPos; // если фокус изменили - значит так надо!
}


bool Dialog::Do_ProcessFirstCtrl()
{
	if (IsEdit(Items[m_FocusPos].Type))
	{
		static_cast<DlgEdit*>(Items[m_FocusPos].ObjPtr)->ProcessKey(Manager::Key(KEY_HOME));
		return true;
	}
	else
	{
		const auto ItemIterator = std::find_if(CONST_RANGE(Items, i)
		{
			return CanGetFocus(i.Type);
		});
		if (ItemIterator != Items.cend())
		{
			ChangeFocus2(ItemIterator - Items.begin());
			ShowDialog();
		}
	}

	return true;
}

bool Dialog::Do_ProcessNextCtrl(bool Up, bool IsRedraw)
{
	const auto OldPos = m_FocusPos;
	unsigned PrevPos=0;

	if (IsEmulatedEditorLine(Items[m_FocusPos]))
		PrevPos = static_cast<DlgEdit*>(Items[m_FocusPos].ObjPtr)->GetCurPos();

	const auto I = ChangeFocus(m_FocusPos, Up? -1 : 1, false);
	Items[m_FocusPos].Flags&=~DIF_FOCUS;
	auto& Item = Items[I];
	Item.Flags|=DIF_FOCUS;
	ChangeFocus2(I);

	if (IsEmulatedEditorLine(Items[m_FocusPos]))
		static_cast<DlgEdit*>(Item.ObjPtr)->SetCurPos(PrevPos);

	if (Items[m_FocusPos].Type == DI_RADIOBUTTON && (Item.Flags & DIF_MOVESELECT))
		ProcessKey(Manager::Key(KEY_SPACE));
	else if (IsRedraw)
	{
		ShowDialog(OldPos);
		ShowDialog(m_FocusPos);
	}

	return true;
}

bool Dialog::Do_ProcessTab(bool Next)
{
	size_t I;

	if (Items.size() > 1)
	{
		// Здесь с фокусом ОООЧЕНЬ ТУМАННО!!!
		if (IsEmulatedEditorLine(Items[m_FocusPos]))
		{
			I=m_FocusPos;

			while (IsEmulatedEditorLine(Items[I]))
				I=ChangeFocus(I, Next ? 1 : -1, true);
		}
		else
		{
			I = ChangeFocus(m_FocusPos, Next ? 1 : -1, true);

			if (!Next)
				while (I>0 && IsEmulatedEditorLine(Items[I]) &&
				        IsEmulatedEditorLine(Items[I-1]) &&
				        !static_cast<DlgEdit*>(Items[I].ObjPtr)->GetLength())
					I--;
		}
	}
	else
		I=m_FocusPos;

	ChangeFocus2(I);
	ShowDialog();

	return true;
}

bool Dialog::Do_ProcessSpace()
{
	if (Items[m_FocusPos].Type==DI_CHECKBOX)
	{
		const auto OldSelected = Items[m_FocusPos].Selected;

		if (Items[m_FocusPos].Flags&DIF_3STATE)
		{
			++Items[m_FocusPos].Selected;
			Items[m_FocusPos].Selected %= 3;
		}
		else
			Items[m_FocusPos].Selected = !Items[m_FocusPos].Selected;

		const auto OldFocusPos = m_FocusPos;

		if (!SendMessage(DN_BTNCLICK,m_FocusPos,ToPtr(Items[m_FocusPos].Selected)))
			Items[OldFocusPos].Selected = OldSelected;

		ShowDialog();
		return true;
	}
	else if (Items[m_FocusPos].Type==DI_RADIOBUTTON)
	{
		m_FocusPos=ProcessRadioButton(m_FocusPos, false);
		ShowDialog();
		return true;
	}
	else if (IsEdit(Items[m_FocusPos].Type) && !(Items[m_FocusPos].Flags & DIF_READONLY))
	{
		if (static_cast<DlgEdit*>(Items[m_FocusPos].ObjPtr)->ProcessKey(Manager::Key(KEY_SPACE)))
		{
			Redraw(); // Перерисовка должна идти после DN_EDITCHANGE (imho)
		}

		return true;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
/* Private:
   Изменяет фокус ввода (воздействие клавишами
     KEY_TAB, KEY_SHIFTTAB, KEY_UP, KEY_DOWN,
   а так же Alt-HotKey)
*/
/* $ 28.07.2000 SVS
   Довесок для сообщений DN_KILLFOCUS & DN_SETFOCUS
*/
/* $ 24.08.2000 SVS
   Добавка для DI_USERCONTROL
*/
size_t Dialog::ChangeFocus(size_t CurFocusPos, int Step, bool SkipGroup) const
{
	const auto OrigFocusPos = CurFocusPos;
//  int FucusPosNeed=-1;
	// В функцию обработки диалога здесь передаем сообщение,
	//   что элемент - LostFocus() - теряет фокус ввода.
//  if(DialogMode.Check(DMODE_INITOBJECTS))
//    FucusPosNeed=DlgProc(this,DN_KILLFOCUS,FocusPos,0);
//  if(FucusPosNeed != -1 && CanGetFocus(Items[FucusPosNeed].Type))
//    FocusPos=FucusPosNeed;
//  else
	{
		for (;;)
		{
			CurFocusPos+=Step;

			if (static_cast<int>(CurFocusPos) < 0)
				CurFocusPos=Items.size()-1;

			if (CurFocusPos>=Items.size())
				CurFocusPos=0;

			const auto Type = Items[CurFocusPos].Type;

			if (!(Items[CurFocusPos].Flags&(DIF_NOFOCUS|DIF_DISABLE|DIF_HIDDEN)))
			{
				if (Type==DI_LISTBOX || Type==DI_BUTTON || Type==DI_CHECKBOX || IsEdit(Type) || Type==DI_USERCONTROL)
					break;

				if (Type==DI_RADIOBUTTON && (!SkipGroup || Items[CurFocusPos].Selected))
					break;
			}

			// убираем зацикливание с последующим подвисанием :-)
			if (OrigFocusPos == CurFocusPos)
				break;
		}
	}
//  Dialog::FocusPos=FocusPos;
	// В функцию обработки диалога здесь передаем сообщение,
	//   что элемент GotFocus() - получил фокус ввода.
	// Игнорируем возвращаемое функцией диалога значение
//  if(DialogMode.Check(DMODE_INITOBJECTS))
//    DlgProc(this,DN_GOTFOCUS,FocusPos,0);
	return CurFocusPos;
}


//////////////////////////////////////////////////////////////////////////
/*
   Private:
   Изменяет фокус ввода между двумя элементами.
   Вынесен отдельно с тем, чтобы обработать DN_KILLFOCUS & DM_SETFOCUS
*/
void Dialog::ChangeFocus2(size_t SetFocusPos)
{
	if (!(Items[SetFocusPos].Flags&(DIF_NOFOCUS|DIF_DISABLE|DIF_HIDDEN)))
	{
		int FocusPosNeed=-1;
		if (DialogMode.Check(DMODE_OBJECTS_INITED))
		{
			FocusPosNeed = static_cast<int>(DlgProc(DN_KILLFOCUS, m_FocusPos, nullptr));

			if (!DialogMode.Check(DMODE_SHOW))
				return;
		}

		if (FocusPosNeed != -1 && CanGetFocus(Items[FocusPosNeed].Type))
			SetFocusPos=FocusPosNeed;

		for (auto& i: Items)
		{
			i.Flags &= ~DIF_FOCUS;
		}

		Items[SetFocusPos].Flags|=DIF_FOCUS;

		//   проинформируем листбокс, есть ли у него фокус
		if (Items[m_FocusPos].Type == DI_LISTBOX)
			Items[m_FocusPos].ListPtr->ClearFlags(VMENU_LISTHASFOCUS);

		if (Items[SetFocusPos].Type == DI_LISTBOX)
			Items[SetFocusPos].ListPtr->SetMenuFlags(VMENU_LISTHASFOCUS);

		SelectOnEntry(m_FocusPos, false);
		SelectOnEntry(SetFocusPos, true);

		PrevFocusPos=m_FocusPos;
		m_FocusPos=SetFocusPos;

		if (DialogMode.Check(DMODE_OBJECTS_INITED))
			DlgProc(DN_GOTFOCUS, m_FocusPos, nullptr);
	}
}

/*
  Функция SelectOnEntry - выделение строки редактирования
  Обработка флага DIF_SELECTONENTRY
*/
void Dialog::SelectOnEntry(size_t Pos, bool Selected)
{
	//if(!DialogMode.Check(DMODE_SHOW))
	//   return;
	if (IsEdit(Items[Pos].Type) &&
	        (Items[Pos].Flags&DIF_SELECTONENTRY)
//     && PrevFocusPos != -1 && PrevFocusPos != Pos
	   )
	{
		if (const auto edt = static_cast<DlgEdit*>(Items[Pos].ObjPtr))
		{
			if (Selected)
				edt->Select(0,edt->GetLength());
			else
				edt->RemoveSelection();

			//_SVS(SysLog(L"Selected=%d edt->GetLength()=%d",Selected,edt->GetLength()));
		}
	}
}

int Dialog::SetAutomation(WORD IDParent,WORD id,
                          FARDIALOGITEMFLAGS UncheckedSet,FARDIALOGITEMFLAGS UncheckedSkip,
                          FARDIALOGITEMFLAGS CheckedSet,FARDIALOGITEMFLAGS CheckedSkip,
                          FARDIALOGITEMFLAGS Checked3Set,FARDIALOGITEMFLAGS Checked3Skip)
{
	int Ret=FALSE;

	if (IDParent < Items.size() && (Items[IDParent].Flags&DIF_AUTOMATION) &&
	        id < Items.size() && IDParent != id) // Сами себя не юзаем!
	{
		Ret = Items[IDParent].AddAutomation(&Items[id], UncheckedSet, UncheckedSkip,
			                                    CheckedSet, CheckedSkip,
				 						        Checked3Set, Checked3Skip);
	}

	return Ret;
}

//////////////////////////////////////////////////////////////////////////
/* Private:
   Заполняем выпадающий список для ComboBox
*/
int Dialog::SelectFromComboBox(
    DialogItemEx *CurItem,
    DlgEdit *EditLine)                   // строка редактирования
{
		_DIALOG(CleverSysLog CL(L"Dialog::SelectFromComboBox()"));
		const auto ComboBox = CurItem->ListPtr;

		SetDropDownOpened(TRUE); // Установим флаг "открытия" комбобокса.
		DlgProc(DN_DROPDOWNOPENED, m_FocusPos, ToPtr(1));
		SetComboBoxPos(CurItem);
		// Перед отрисовкой спросим об изменении цветовых атрибутов
		FarColor RealColors[VMENU_COLOR_COUNT] = {};
		FarDialogItemColors ListColors={sizeof(FarDialogItemColors)};
		ListColors.ColorsCount=VMENU_COLOR_COUNT;
		ListColors.Colors=RealColors;
		ComboBox->SetColors(nullptr);
		ComboBox->GetColors(&ListColors);

		if (DlgProc(DN_CTLCOLORDLGLIST,CurItem - Items.data(), &ListColors))
			ComboBox->SetColors(&ListColors);

		// Выставим то, что есть в строке ввода!
		// if(EditLine->GetDropDownBox()) //???
		auto strStr = EditLine->GetString();

		if (CurItem->Flags & (DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND))
			strStr = HiText2Str(strStr);

		ComboBox->SetSelectPos(ComboBox->FindItem(0,strStr,LIFIND_EXACTMATCH),1);

		const auto OriginalPos = ComboBox->GetSelectPos();
		CurItem->IFlags.Set(DLGIIF_COMBOBOXNOREDRAWEDIT);
		ComboBox->Process();

		CurItem->IFlags.Clear(DLGIIF_COMBOBOXNOREDRAWEDIT);
		ComboBox->ClearDone();

		const auto Dest = GetDropDownOpened()? // Закрылся не программным путём?
			ComboBox->GetExitCode():
			-1;

		if (Dest == -1)
			ComboBox->SetSelectPos(OriginalPos,0); //????

		SetDropDownOpened(FALSE); // Установим флаг "закрытия" комбобокса.
		DlgProc(DN_DROPDOWNOPENED, m_FocusPos, nullptr);

		if (Dest<0)
		{
			Redraw();
			return KEY_ESC;
		}

		const auto& ItemPtr = ComboBox->at(Dest);

		if (CurItem->Flags & (DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND))
		{
			strStr = HiText2Str(ItemPtr.Name);
			EditLine->SetString(strStr);
		}
		else
			EditLine->SetString(ItemPtr.Name);

		EditLine->SetLeftPos(0);
		Redraw();
		return KEY_ENTER;
}

//////////////////////////////////////////////////////////////////////////
/* Private:
   Заполняем выпадающий список из истории
*/
bool Dialog::SelectFromEditHistory(DialogItemEx const* const CurItem, DlgEdit* const EditLine, string_view const HistoryName)
{
	_DIALOG(CleverSysLog CL(L"Dialog::SelectFromEditHistory()"));

	if (!EditLine)
		return false;

	string strStr;
	history_return_type ret = HRT_CANCEL;
	auto& DlgHist = static_cast<DlgEdit*>(CurItem->ObjPtr)->GetHistory();

	if(DlgHist)
	{
		DlgHist->ResetPosition();
		// создание пустого вертикального меню
		const auto HistoryMenu = VMenu2::create({}, {}, Global->Opt->Dialogs.CBoxMaxHeight, VMENU_ALWAYSSCROLLBAR | VMENU_COMBOBOX);
		HistoryMenu->SetDialogMode(DMODE_NODRAWSHADOW);
		HistoryMenu->SetModeMoving(false);
		HistoryMenu->SetMenuFlags(VMENU_SHOWAMPERSAND);
		HistoryMenu->SetBoxType(SHORT_SINGLE_BOX);
		HistoryMenu->SetId(SelectFromEditHistoryId);
//		SetDropDownOpened(TRUE); // Установим флаг "открытия" комбобокса.
		// запомним (для прорисовки)
//		CurItem->ListPtr=&HistoryMenu;
		SetDropDownOpened(TRUE); // Установим флаг "открытия" комбобокса.
		DlgProc(DN_DROPDOWNOPENED, m_FocusPos, ToPtr(1));
		ret = DlgHist->Select(*HistoryMenu, Global->Opt->Dialogs.CBoxMaxHeight, this, strStr);
		SetDropDownOpened(FALSE); // Установим флаг "открытия" комбобокса.
		DlgProc(DN_DROPDOWNOPENED, m_FocusPos, nullptr);
		// забудем (не нужен)
//		CurItem->ListPtr=nullptr;
//		SetDropDownOpened(FALSE); // Установим флаг "закрытия" комбобокса.
	}

	if (ret != HRT_CANCEL)
	{
		EditLine->SetString(strStr);
		EditLine->SetLeftPos(0);
		EditLine->SetClearFlag(false);
		Redraw();
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
/* Private:
   Работа с историей - добавление и reorder списка
*/
bool Dialog::AddToEditHistory(DialogItemEx const* const CurItem, string_view const AddStr) const
{
	if (!CurItem->ObjPtr)
	{
		return false;
	}

	auto& DlgHist = static_cast<DlgEdit*>(CurItem->ObjPtr)->GetHistory();
	if(DlgHist)
	{
		DlgHist->AddToHistory(AddStr);
	}
	return true;
}

int Dialog::CheckHighlights(WORD CheckSymbol,int StartPos)
{
	if (StartPos < 0)
		StartPos=0;

	for (size_t I = StartPos; I < Items.size(); ++I)
	{
		const auto& Item = Items[I];
		const auto Type = Item.Type;
		const auto Flags = Item.Flags;

		if ((!IsEdit(Type) || (Type == DI_COMBOBOX && (Flags&DIF_DROPDOWNLIST))) && !(Flags & (DIF_SHOWAMPERSAND|DIF_DISABLE|DIF_HIDDEN)))
		{
			wchar_t Ch;
			if (HiTextHotkey(Item.strData, Ch))
			{
				if (Ch && upper(CheckSymbol) == upper(Ch))
					return static_cast<int>(I);
			}
			else if (!CheckSymbol)
				return static_cast<int>(I);
		}
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////////
/* Private:
   Если жмакнули Alt-???
*/
bool Dialog::ProcessHighlighting(int Key, size_t FocusPos, bool Translate)
{
	INPUT_RECORD rec;
	if(!KeyToInputRecord(Key,&rec))
	{
		rec = {};
	}

	// Beware - I is modified within the loop, so don't cache Items[I]
	for (size_t I=0; I<Items.size(); I++)
	{
		if (IsEdit(Items[I].Type) && !(Items[I].Type == DI_COMBOBOX && Items[I].Flags & DIF_DROPDOWNLIST))
			continue;

		if (Items[I].Flags & (DIF_SHOWAMPERSAND | DIF_DISABLE | DIF_HIDDEN))
			continue;

		if (!IsKeyHighlighted(Items[I].strData, Key, Translate))
			continue;

		bool DisableSelect = false;

		// Если ЭТО: DlgEdit(пред контрол) и DI_TEXT в одну строку, то...
		if (
			I > 0 &&
			Items[I].Type == DI_TEXT &&                               // DI_TEXT
			IsEdit(Items[I - 1].Type) &&                              // и редактор
			Items[I].Y1 == Items[I - 1].Y1 &&                         // и оба в одну строку
			(I + 1 < Items.size() && Items[I].Y1 != Items[I + 1].Y1)) // ...и следующий контрол в другой строке
		{
			// Сначала сообщим о случившемся факте процедуре обработки диалога, а потом...
			if (!DlgProc(DN_HOTKEY, I, &rec))
				break; // сказали не продолжать обработку...

			// ... если предыдущий контрол задизаблен или невидим, тогда выходим.
			if ((Items[I - 1].Flags & (DIF_DISABLE | DIF_HIDDEN))) // и не задисаблен
				break;

			I = ChangeFocus(I, -1, false);
			DisableSelect = true;
		}
		else if (
			Items[I].Type == DI_TEXT ||
			Items[I].Type == DI_VTEXT ||
			Items[I].Type == DI_SINGLEBOX ||
			Items[I].Type == DI_DOUBLEBOX)
		{
			if (I < Items.size() - 1) // ...и следующий контрол
			{
				// Сначала сообщим о случившемся факте процедуре обработки диалога, а потом...
				if (!DlgProc(DN_HOTKEY, I, &rec))
					break; // сказали не продолжать обработку...

				// ... если следующий контрол задизаблен или невидим, тогда выходим.
				if ((Items[I + 1].Flags & (DIF_DISABLE | DIF_HIDDEN))) // и не задисаблен
					break;

				I = ChangeFocus(I, 1, false);
				DisableSelect = true;
			}
		}

		// Сообщим о случившемся факте процедуре обработки диалога
		if (!DlgProc(DN_HOTKEY, I, &rec))
			break; // сказали не продолжать обработку...

		ChangeFocus2(I);
		ShowDialog();

		if ((Items[I].Type == DI_CHECKBOX || Items[I].Type == DI_RADIOBUTTON) &&
			(!DisableSelect || (Items[I].Flags & DIF_MOVESELECT)))
		{
			Do_ProcessSpace();
		}
		else if (Items[I].Type == DI_BUTTON)
		{
			ProcessKey(KEY_ENTER, I);
		}
		// при ComboBox`е - "вываливаем" последний //????
		else if (Items[I].Type == DI_COMBOBOX)
		{
			ProcessOpenComboBox(Items[I].Type, &Items[I], I);
			//ProcessKey(KEY_CTRLDOWN);
		}

		return true;
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
/*
   функция подравнивания координат edit классов
*/
void Dialog::AdjustEditPos(int dx, int dy)
{
	if (!DialogMode.Check(DMODE_OBJECTS_CREATED))
		return;


	for (auto& i: Items)
	{
		const auto Type = i.Type;

		if ((i.ObjPtr  && IsEdit(Type)) || (i.ListPtr && Type == DI_LISTBOX))
		{
			const auto DialogScrObject = Type == DI_LISTBOX?
				i.ListPtr.get() :
				static_cast<ScreenObject*>(i.ObjPtr);

			auto Rect = DialogScrObject->GetPosition();
			Rect.left += dx;
			Rect.right += dx;
			Rect.top += dy;
			Rect.bottom += dy;
			DialogScrObject->SetPosition(Rect);
		}
	}

	ProcessCenterGroup();
}


//////////////////////////////////////////////////////////////////////////
/*
   Работа с доп. данными экземпляра диалога
   Пока простое копирование (присвоение)
*/
void Dialog::SetDialogData(void* NewDataDialog)
{
	DataDialog=NewDataDialog;
}

//////////////////////////////////////////////////////////////////////////
/* $ 29.06.2007 yjh\
   При расчётах времён копирования проще/надёжнее учитывать время ожидания
   пользовательских ответов в одном месте (здесь).
   Сброс этой переменной должен осуществляться перед общим началом операции
*/
std::chrono::steady_clock::duration WaitUserTime;

/* $ 11.08.2000 SVS
   + Для того, чтобы послать DM_CLOSE нужно переопределить Process
*/
void Dialog::Process()
{
	_DIALOG(CleverSysLog CL(L"Dialog::Process()"));
//  if(DialogMode.Check(DMODE_SMALLDIALOG))
	SetRestoreScreenMode(true);
	PrevMouseRecord = {};
	ClearDone();
	InitDialog();
	std::optional<taskbar::state> TBE;
	if (DialogMode.Check(DMODE_WARNINGSTYLE))
	{
		TBE.emplace(TBPF_ERROR);
	}

	if (m_ExitCode == -1)
	{
		DialogMode.Set(DMODE_BEGINLOOP);

		if(GetCanLoseFocus())
		{
			Global->WindowManager->InsertWindow(shared_from_this());
		}
		else
		{
			static std::atomic_long DialogsCount(0);
			std::chrono::steady_clock::time_point btm;

			if (!DialogsCount)
			{
				btm = std::chrono::steady_clock::now();
			}

			++DialogsCount;
			Global->WindowManager->ExecuteWindow(shared_from_this());
			Global->WindowManager->ExecuteModal(shared_from_this());
			--DialogsCount;

			if (!DialogsCount)
			{
				WaitUserTime += std::chrono::steady_clock::now() - btm;
			}
		}
	}

	if (SavedItems)
		std::copy(ALL_CONST_RANGE(Items), SavedItems);
}

intptr_t Dialog::CloseDialog()
{
	_DIALOG(CleverSysLog CL(L"Dialog::CloseDialog()"));
	GetDialogObjectsData();

	const auto result = DlgProc(DN_CLOSE, m_ExitCode, nullptr);
	if (result)
	{
		GetDialogObjectsExpandData();
		DialogMode.Set(DMODE_ENDLOOP);
		Hide();

		if (DialogMode.Check(DMODE_BEGINLOOP) && (DialogMode.Check(DMODE_MSGINTERNAL) || Global->WindowManager->ManagerStarted()))
		{
			DialogMode.Clear(DMODE_BEGINLOOP);
			Global->WindowManager->DeleteWindow(shared_from_this());
			Global->WindowManager->PluginCommit();
		}

		_DIALOG(CleverSysLog CL(L"Close Dialog"));
	}
	return result;
}


/* $ 17.05.2001 DJ
   установка help topic'а и прочие радости, временно перетащенные сюда
   из SimpleModal
*/
void Dialog::SetHelp(const string_view Topic)
{
	HelpTopic = Topic;
}

void Dialog::ShowHelp() const
{
	if (!HelpTopic.empty())
	{
		help::show(HelpTopic);
	}
}

void Dialog::ClearDone()
{
	_DIALOG(CleverSysLog CL(L"Dialog::ClearDone()"));
	m_ExitCode=-1;
	DialogMode.Clear(DMODE_ENDLOOP);
}

void Dialog::SetExitCode(int Code)
{
	m_ExitCode=Code;
	DialogMode.Set(DMODE_ENDLOOP);
	//CloseDialog();
}

void Dialog::OnChangeFocus(bool focus)
{
	window::OnChangeFocus(focus);
	if (GetCanLoseFocus())
		DlgProc(focus ? DN_GOTFOCUS : DN_KILLFOCUS, -1, nullptr);
}

/* $ 19.05.2001 DJ
   возвращаем наше название для меню по F12
*/
int Dialog::GetTypeAndName(string &strType, string &strName)
{
	strType = msg(lng::MDialogType);
	strName = GetTitle();
	return windowtype_dialog;
}

bool Dialog::CanFastHide() const
{
	return (Global->Opt->AllCtrlAltShiftRule & CASR_DIALOG) != 0;
}

void Dialog::ResizeConsole()
{
	DialogMode.Set(DMODE_RESIZED);

	if (IsVisible())
	{
		Hide();
	}

	COORD c = {static_cast<short>(ScrX+1), static_cast<short>(ScrY+1)};
	SendMessage(DN_RESIZECONSOLE, 0, &c);

	const auto Rect = GetPosition();
	c.X = std::min(Rect.left, ScrX-1);
	c.Y = std::min(Rect.top, ScrY-1);
	if(c.X != Rect.left || c.Y != Rect.top)
	{
		c.X = Rect.left;
		c.Y = Rect.top;
		SendMessage(DM_MOVEDIALOG, TRUE, &c);
		SetComboBoxPos();
	}
}

intptr_t Dialog::DlgProc(intptr_t Msg,intptr_t Param1,void* Param2)
{
	_DIALOG(CleverSysLog CL(L"Dialog.DlgProc()"));
	if (DialogMode.Check(DMODE_ENDLOOP))
		return 0;
	_DIALOG(SysLog(L"hDlg=%p, Msg=%s, Param1=%d (0x%08X), Param2=%d (0x%08X)",this,_DLGMSG_ToName(Msg),Param1,Param1,Param2,Param2));


	FarDialogEvent de={sizeof(FarDialogEvent),this,Msg,Param1,Param2,0};

	if(!CheckDialogMode(DMODE_NOPLUGINS))
	{
		if (Global->CtrlObject->Plugins->ProcessDialogEvent(DE_DLGPROCINIT,&de))
			return de.Result;
	}

	const auto Result = m_handler(this,Msg,Param1,Param2);

	if(!CheckDialogMode(DMODE_NOPLUGINS))
	{
		de.Result=Result;
		if (Global->CtrlObject->Plugins->ProcessDialogEvent(DE_DLGPROCEND,&de))
			return de.Result;
	}
	return Result;
}

//////////////////////////////////////////////////////////////////////////
/* $ 28.07.2000 SVS
   функция обработки диалога (по умолчанию)
   Вот именно эта функция и является последним рубежом обработки диалога.
   Т.е. здесь должна быть ВСЯ обработка ВСЕХ сообщений!!!
*/
intptr_t Dialog::DefProc(intptr_t Msg, intptr_t Param1, void* Param2)
{
	_DIALOG(CleverSysLog CL(L"Dialog.DefDlgProc()"));
	_DIALOG(SysLog(L"hDlg=%p, Msg=%s, Param1=%d (0x%08X), Param2=%d (0x%08X)",this,_DLGMSG_ToName(Msg),Param1,Param1,Param2,Param2));

	FarDialogEvent de={sizeof(FarDialogEvent),this,Msg,Param1,Param2,0};

	if(!CheckDialogMode(DMODE_NOPLUGINS))
	{
		if (Global->CtrlObject->Plugins->ProcessDialogEvent(DE_DEFDLGPROCINIT,&de))
		{
			return de.Result;
		}
	}
	DialogItemEx *CurItem=nullptr;
	int Type=0;

	switch (Msg)
	{
		case DN_INITDIALOG:
			return FALSE; // изменений не было!
		case DN_CLOSE:
			return TRUE;  // согласен с закрытием
		case DN_KILLFOCUS:
			return -1;    // "Согласен с потерей фокуса"
		case DN_GOTFOCUS:
			return 0;     // always 0
		case DN_HELP:
			return reinterpret_cast<intptr_t>(Param2); // что передали, то и...
		case DN_DRAGGED:
			return TRUE; // согласен с перемещалкой.
		case DN_DRAWDLGITEMDONE: // Param1 = ID
			return TRUE;
		case DN_DRAWDIALOGDONE:
		{
			if (DialogMode.Check(DMODE_KEYDRAGGED)) // Нужно отрисовать "салазки"?
			{
				/* $ 03.08.2000 tran
				   вывод текста в углу может приводить к ошибкам изображения
				   1) когда диалог перемещается в угол
				   2) когда диалог перемещается из угла
				   сделал вывод красных палочек по углам */
				const auto Color = colors::ConsoleColorToFarColor(B_LIGHTRED|F_YELLOW);
				Text({ m_Where.left, m_Where.top }, Color, L"\\"sv);
				Text({ m_Where.left, m_Where.bottom }, Color, L"/"sv);
				Text({ m_Where.right, m_Where.top }, Color, L"/"sv);
				Text({ m_Where.right, m_Where.bottom }, Color, L"\\"sv);
			}

			return TRUE;
		}
		case DN_DRAWDIALOG:
		{
			return TRUE;
		}
		case DN_CTLCOLORDIALOG:
			return FALSE;
		case DN_CTLCOLORDLGITEM:
			return FALSE;
		case DN_CTLCOLORDLGLIST:
			return FALSE;
		case DN_ENTERIDLE:
			return 0;     // always 0
		default:
			break;
	}

	// предварительно проверим...
	if (!Items.empty() && static_cast<size_t>(Param1) >= Items.size())
		return 0;

	if (Param1>=0)
	{
		CurItem = &Items[Param1];
		Type=CurItem->Type;
	}

	switch (Msg)
	{
		case DN_CONTROLINPUT:
			return FALSE;
		//BUGBUG!!!
		case DN_INPUT:
			return TRUE;
		case DN_DRAWDLGITEM:
			return TRUE;
		case DN_HOTKEY:
			return TRUE;
		case DN_EDITCHANGE:
			return TRUE;
		case DN_BTNCLICK:
			return Type != DI_BUTTON || CurItem->Flags&DIF_BTNNOCLOSE;
		case DN_LISTCHANGE:
			return TRUE;
		case DM_GETSELECTION: // Msg=DM_GETSELECTION, Param1=ID, Param2=*EditorSelect
			return FALSE;
		case DM_SETSELECTION:
			return FALSE;
		default:
			break;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
/* $ 28.07.2000 SVS
   Посылка сообщения диалогу
   Некоторые сообщения эта функция обрабатывает сама, не передавая управление
   обработчику диалога.
*/
intptr_t Dialog::SendMessage(intptr_t Msg,intptr_t Param1,void* Param2)
{
	_DIALOG(CleverSysLog CL(L"Dialog.SendDlgMessage()"));
	_DIALOG(SysLog(L"hDlg=%p, Msg=%s, Param1=%d (0x%08X), Param2=%d (0x%08X)",this,_DLGMSG_ToName(Msg),Param1,Param1,Param2,Param2));

	const auto redraw = [this](bool Flush)
	{
		if (DialogMode.Check(DMODE_OBJECTS_INITED) && !DialogMode.Check(DMODE_DRAWING) && IsRedrawEnabled())
		{
			Global->WindowManager->RefreshWindow(shared_from_this());
			Global->WindowManager->PluginCommit();
			if (Flush)
				Global->ScrBuf->Flush();
		}
	};
	// Сообщения, касаемые только диалога и не затрагивающие элементы
	switch (Msg)
	{
			/*****************************************************************/
		case DM_RESIZEDIALOG:
			// изменим вызов RESIZE.
			Param1=-1;
			/*****************************************************************/
			[[fallthrough]];
		case DM_MOVEDIALOG:
		{
			auto W1 = m_Where.width();
			auto H1 = m_Where.height();
			m_Drag.OldRect = m_Where;

			// переместили
			if (Param1>0)  // абсолютно?
			{
				m_Where.left = static_cast<COORD*>(Param2)->X;
				m_Where.top = static_cast<COORD*>(Param2)->Y;
				m_Where.right = W1;
				m_Where.bottom = H1;
				CheckDialogCoord();
			}
			else if (!Param1)  // значит относительно
			{
				m_Where.left += static_cast<COORD*>(Param2)->X;
				m_Where.top += static_cast<COORD*>(Param2)->Y;
			}
			else // Resize, Param2=width/height
			{
				const auto OldW1 = W1;
				const auto OldH1 = H1;
				const auto fixSize = [](intptr_t size) { return (size <= 0) ? 1 : size; };
				W1 = fixSize(static_cast<COORD*>(Param2)->X);
				H1 = fixSize(static_cast<COORD*>(Param2)->Y);
				RealWidth = W1;
				RealHeight = H1;

				if (W1<OldW1 || H1<OldH1)
				{
					_DIALOG(SysLog(L"[%d] DialogMode.Set(DMODE_DRAWING)",__LINE__));
					DialogMode.Set(DMODE_DRAWING);

					for (auto& i: Items)
					{
						if (i.Flags&DIF_HIDDEN)
							continue;

						SMALL_RECT Rect;

						Rect.Left = i.X1;
						Rect.Top = i.Y1;

						if (i.X2 >= W1)
						{
							Rect.Right = i.X2 - (OldW1 - W1);
							Rect.Bottom = i.Y2;
							SetItemRect(i, Rect);
						}

						if (i.Y2 >= H1)
						{
							Rect.Right = i.X2;
							Rect.Bottom = i.Y2 - (OldH1 - H1);
							SetItemRect(i, Rect);
						}
					}

					_DIALOG(SysLog(L"[%d] DialogMode.Clear(DMODE_DRAWING)",__LINE__));
					DialogMode.Clear(DMODE_DRAWING);
				}
			}

			// проверили и скорректировали
			if (m_Where.left + W1 < 0)
				m_Where.left = -W1 + 1;

			if (m_Where.top + H1 < 0)
				m_Where.top = -H1 + 1;

			if (m_Where.left > ScrX)
				m_Where.left = ScrX;

			if (m_Where.top > ScrY)
				m_Where.top = ScrY;

			m_Where.right = m_Where.left + W1 - 1;
			m_Where.bottom = m_Where.top + H1 - 1;

			if (Param1>0)  // абсолютно?
			{
				CheckDialogCoord();
			}

			if (Param1 < 0)  // размер?
			{
				static_cast<COORD*>(Param2)->X = m_Where.width();
				static_cast<COORD*>(Param2)->Y = m_Where.height();
			}
			else
			{
				static_cast<COORD*>(Param2)->X = m_Where.left;
				static_cast<COORD*>(Param2)->Y = m_Where.top;
			}

			int I=IsVisible();// && DialogMode.Check(DMODE_INITOBJECTS);

			if (I) Hide();

			// приняли.
			AdjustEditPos(m_Where.left - m_Drag.OldRect.left, m_Where.top - m_Drag.OldRect.top);

			if (I) Show(); // только если диалог был виден

			return reinterpret_cast<intptr_t>(Param2);
		}
		/*****************************************************************/
		case DM_REDRAW:
		{
			redraw(true);
			return 0;
		}
		/*****************************************************************/
		case DM_ENABLEREDRAW:
		{
			int Prev=m_DisableRedraw;

			if (Param1 == TRUE && m_DisableRedraw < 0)
				m_DisableRedraw++;
			else if (Param1 == FALSE)
				m_DisableRedraw--;

			//Edit::DisableEditOut(IsEnableRedraw);

			if (!m_DisableRedraw && Prev != m_DisableRedraw)
				if (DialogMode.Check(DMODE_OBJECTS_INITED))
				{
					SendMessage(DM_REDRAW, 0, nullptr);
				}

			return Prev;
		}
		/*****************************************************************/
		case DM_SHOWDIALOG:
		{
			if (Param1)
			{
				if (!IsVisible())
				{
					DialogMode.Set(DMODE_VISIBLE);
					redraw(false);
				}
			}
			else
			{
				if (IsVisible())
				{
					Hide();
					DialogMode.Clear(DMODE_VISIBLE);
				}
			}
			return 0;
		}
		/*****************************************************************/
		case DM_SETDLGDATA:
		{
			void* PrewDataDialog=DataDialog;
			DataDialog=Param2;
			return reinterpret_cast<intptr_t>(PrewDataDialog);
		}
		/*****************************************************************/
		case DM_GETDLGDATA:
		{
			return reinterpret_cast<intptr_t>(DataDialog);
		}
		/*****************************************************************/
		case DM_KEY:
		{
			const auto KeyArray = static_cast<const INPUT_RECORD*>(Param2);
			DialogMode.Set(DMODE_KEY);

			for (unsigned int I = 0; I < static_cast<size_t>(Param1); ++I)
				ProcessKey(Manager::Key(InputRecordToKey(KeyArray+I)));

			DialogMode.Clear(DMODE_KEY);
			return 0;
		}
		/*****************************************************************/
		case DM_CLOSE:
		{
			if (Param1 == -1)
				m_ExitCode=static_cast<int>(m_FocusPos);
			else
				m_ExitCode=Param1;

			return CloseDialog();
		}
		/*****************************************************************/
		case DM_GETDLGRECT:
		{
			if (Param2)
			{
				const auto Rect = GetPosition();
				static_cast<SMALL_RECT*>(Param2)->Left = Rect.left;
				static_cast<SMALL_RECT*>(Param2)->Top = Rect.top;
				static_cast<SMALL_RECT*>(Param2)->Right = Rect.right;
				static_cast<SMALL_RECT*>(Param2)->Bottom = Rect.bottom;
				return TRUE;
			}

			return FALSE;
		}
		/*****************************************************************/
		case DM_GETDROPDOWNOPENED: // Param1=0; Param2=0
		{
			return GetDropDownOpened();
		}
		/*****************************************************************/
		case DM_KILLSAVESCREEN:
		{
			if (SaveScr) SaveScr->Discard();

			if (ShadowSaveScr) ShadowSaveScr->Discard();

			return TRUE;
		}
		/*****************************************************************/
		/*
		  Msg=DM_ALLKEYMODE
		  Param1 = -1 - получить состояние
		         =  0 - выключить
		         =  1 - включить
		  Ret = состояние
		*/
		case DM_ALLKEYMODE:
		{
			if (Param1 == -1)
				return Global->IsProcessAssignMacroKey;

			return std::exchange(Global->IsProcessAssignMacroKey, Param1 != 0);
		}
		/*****************************************************************/
		case DM_SETINPUTNOTIFY: // Param1 = 1 on, 0 off, -1 - get
		{
			bool State=DialogMode.Check(DMODE_INPUTEVENT);

			if (Param1 != -1)
			{
				if (!Param1)
					DialogMode.Clear(DMODE_INPUTEVENT);
				else
					DialogMode.Set(DMODE_INPUTEVENT);
			}

			return State;
		}
		/*****************************************************************/
		case DN_RESIZECONSOLE:
		{
			return DlgProc(Msg,Param1,Param2);
		}
		/*****************************************************************/
		case DM_GETDIALOGINFO:
		{
			if (IdExist)
			{
				if (const auto di = static_cast<DialogInfo*>(Param2))
				{
					if (CheckStructSize(di))
					{
						di->Id = m_Id;
						di->Owner = PluginOwner? PluginOwner->Id() : FarUuid;
						return true;
					}
				}
			}
			return false;
		}
		/*****************************************************************/
		// Param1=0, Param2=FarDialogItemData, Ret=size (without '\0')
		case DM_GETDIALOGTITLE:
		{
			const auto did = static_cast<FarDialogItemData*>(Param2);
			const auto strTitleDialog = GetTitle();
			auto Len = strTitleDialog.size();
			if (CheckStructSize(did)) // если здесь nullptr, то это еще один способ получить размер
			{
				if (!did->PtrLength)
					did->PtrLength = Len;
				else if (Len > did->PtrLength)
					Len = did->PtrLength;

				if (did->PtrData)
				{
					std::copy_n(strTitleDialog.data(), Len, did->PtrData);
					did->PtrData[Len] = {};
				}
			}

			return Len;
		}
		default:
			break;
	}

	/*****************************************************************/
	if (Msg >= DM_USER)
	{
		return DlgProc(Msg,Param1,Param2);
	}

	/*****************************************************************/
	size_t Len=0;

	// предварительно проверим...
	/* $ 09.12.2001 DJ
	   для DM_USER проверять _не_надо_!
	*/
	if (static_cast<size_t>(Param1) >= Items.size() || Items.empty())
		return 0;

	const auto CurItem=&Items[Param1];
	const auto Type = CurItem->Type;
	const auto* Ptr= CurItem->strData.c_str();

	if (IsEdit(Type) && CurItem->ObjPtr)
		Ptr = static_cast<DlgEdit*>(CurItem->ObjPtr)->GetString().c_str();

	switch (Msg)
	{
			/*****************************************************************/
		case DM_LISTSORT: // Param1=ID Param=Direct {0|1}
		case DM_LISTADD: // Param1=ID Param2=FarList: ItemsNumber=Count, Items=Src
		case DM_LISTADDSTR: // Param1=ID Param2=String
		case DM_LISTDELETE: // Param1=ID Param2=FarListDelete: StartIndex=BeginIndex, Count=количество (<=0 - все!)
		case DM_LISTGETITEM: // Param1=ID Param2=FarListGetItem: ItemsNumber=Index, Items=Dest
		case DM_LISTSET: // Param1=ID Param2=FarList: ItemsNumber=Count, Items=Src
		case DM_LISTGETCURPOS: // Param1=ID Param2=FarListPos
		case DM_LISTSETCURPOS: // Param1=ID Param2=FarListPos Ret: RealPos
		case DM_LISTUPDATE: // Param1=ID Param2=FarList: ItemsNumber=Index, Items=Src
		case DM_LISTINFO:// Param1=ID Param2=FarListInfo
		case DM_LISTFINDSTRING: // Param1=ID Param2=FarListFind
		case DM_LISTINSERT: // Param1=ID Param2=FarListInsert
		case DM_LISTGETDATA: // Param1=ID Param2=Index
		case DM_LISTSETDATA: // Param1=ID Param2=FarListItemData
		case DM_LISTSETTITLES: // Param1=ID Param2=FarListTitles
		case DM_LISTGETTITLES: // Param1=ID Param2=FarListTitles
		case DM_LISTGETDATASIZE: // Param1=ID Param2=Index
		case DM_SETCOMBOBOXEVENT: // Param1=ID Param2=FARCOMBOBOXEVENTTYPE Ret=OldSets
		case DM_GETCOMBOBOXEVENT: // Param1=ID Param2=0 Ret=Sets
		{
			if (Type==DI_LISTBOX || Type==DI_COMBOBOX)
			{
				auto& ListBox = CurItem->ListPtr;

				if (ListBox)
				{
					intptr_t Ret=TRUE;

					switch (Msg)
					{
						case DM_LISTINFO:// Param1=ID Param2=FarListInfo
						{
							const auto li = static_cast<FarListInfo*>(Param2);
							return CheckStructSize(li) && ListBox->GetVMenuInfo(li);
						}
						case DM_LISTSORT: // Param1=ID Param=Direct {0|1}
						{
							ListBox->SortItems(Param2 != nullptr);
							break;
						}
						case DM_LISTFINDSTRING: // Param1=ID Param2=FarListFind
						{
							const auto lf = static_cast<const FarListFind*>(Param2);
							return CheckStructSize(lf)?ListBox->FindItem(lf->StartIndex,lf->Pattern,lf->Flags):-1;
						}
						case DM_LISTADDSTR: // Param1=ID Param2=String
						{
							Ret=ListBox->AddItem(static_cast<const wchar_t*>(Param2));
							break;
						}
						case DM_LISTADD: // Param1=ID Param2=FarList: ItemsNumber=Count, Items=Src
						{
							const auto ListItems = static_cast<const FarList*>(Param2);

							if (!CheckStructSize(ListItems))
								return FALSE;

							Ret=ListBox->AddItem(ListItems);
							break;
						}
						case DM_LISTDELETE: // Param1=ID Param2=FarListDelete: StartIndex=BeginIndex, Count=количество (<=0 - все!)
						{
							const auto ListItems = static_cast<const FarListDelete*>(Param2);
							if(CheckNullOrStructSize(ListItems))
							{
								int Count;
								if (!ListItems || (Count=ListItems->Count) <= 0)
									ListBox->clear();
								else
									ListBox->DeleteItem(ListItems->StartIndex,Count);
							}
							else return FALSE;

							break;
						}
						case DM_LISTINSERT: // Param1=ID Param2=FarListInsert
						{
							const auto li = static_cast<const FarListInsert*>(Param2);
							if (!CheckStructSize(li) || (Ret = ListBox->InsertItem(li)) == -1)
								return -1;

							break;
						}
						case DM_LISTUPDATE: // Param1=ID Param2=FarListUpdate: Index=Index, Items=Src
						{
							const auto lu = static_cast<const FarListUpdate*>(Param2);
							if (CheckStructSize(lu) && ListBox->UpdateItem(lu))
								break;

							return FALSE;
						}
						case DM_LISTGETITEM: // Param1=ID Param2=FarListGetItem: ItemsNumber=Index, Items=Dest
						{
							const auto ListItems = static_cast<FarListGetItem*>(Param2);

							if (!CheckStructSize(ListItems))
								return FALSE;


							if (static_cast<size_t>(ListItems->ItemIndex) < ListBox->size())
							{
								const auto& ListMenuItem = ListBox->at(ListItems->ItemIndex);
								//ListItems->ItemIndex=1;
								auto& Item = ListItems->Item;
								Item = {};
								Item.Flags=ListMenuItem.Flags;
								Item.Text=ListMenuItem.Name.c_str();
								Item.UserData = ListMenuItem.SimpleUserData;
								Item.Reserved = 0;

								return TRUE;
							}

							return FALSE;
						}
						case DM_LISTGETDATA: // Param1=ID Param2=Index
						{
							if (reinterpret_cast<size_t>(Param2) < ListBox->size())
							{
								const auto Data = ListBox->GetComplexUserDataPtr<std::vector<char>>(static_cast<int>(reinterpret_cast<intptr_t>(Param2)));
								return Data? reinterpret_cast<intptr_t>(Data->data()) : 0;
							}
							return 0;
						}
						case DM_LISTGETDATASIZE: // Param1=ID Param2=Index
						{
							if (reinterpret_cast<size_t>(Param2) < ListBox->size())
							{
								const auto Data = ListBox->GetComplexUserDataPtr<std::vector<char>>(static_cast<int>(reinterpret_cast<intptr_t>(Param2)));
								return Data? Data->size() : 0;
							}

							return 0;
						}
						case DM_LISTSETDATA: // Param1=ID Param2=FarListItemData
						{
							const auto ListItems = static_cast<const FarListItemData*>(Param2);

							if (CheckStructSize(ListItems) && static_cast<size_t>(ListItems->Index) < ListBox->size())
							{
								const auto Data = static_cast<const char*>(ListItems->Data);
								const auto Size = ListItems->DataSize? ListItems->DataSize : (wcslen(static_cast<const wchar_t*>(ListItems->Data)) + 1) * sizeof(wchar_t);
								std::vector<char> DataCopy(Data, Data + Size);
								ListBox->SetComplexUserData(DataCopy, ListItems->Index);

								return TRUE;
							}

							return FALSE;
						}
						/* $ 02.12.2001 KM
						   + Сообщение для добавления в список строк, с удалением
						     уже существующих, т.с. "чистая" установка
						*/
						case DM_LISTSET: // Param1=ID Param2=FarList: ItemsNumber=Count, Items=Src
						{
							const auto ListItems = static_cast<const FarList*>(Param2);

							if (!CheckStructSize(ListItems))
								return FALSE;

							ListBox->clear();
							Ret=ListBox->AddItem(ListItems);
							break;
						}
						//case DM_LISTINS: // Param1=ID Param2=FarList: ItemsNumber=Index, Items=Dest
						case DM_LISTSETTITLES: // Param1=ID Param2=FarListTitles
						{
							const auto ListTitle = static_cast<const FarListTitles*>(Param2);
							if(CheckStructSize(ListTitle))
							{
								ListBox->SetTitle(NullToEmpty(ListTitle->Title));
								ListBox->SetBottomTitle(NullToEmpty(ListTitle->Bottom));
								break;   //return TRUE;
							}
							return FALSE;
						}
						case DM_LISTGETTITLES: // Param1=ID Param2=FarListTitles
						{

							const auto ListTitle = static_cast<FarListTitles*>(Param2);
							auto strTitle = ListBox->GetTitle();
							string strBottomTitle;
							ListBox->GetBottomTitle(strBottomTitle);

							if (CheckStructSize(ListTitle)&&(!strTitle.empty()||!strBottomTitle.empty()))
							{
								if (ListTitle->Title&&ListTitle->TitleSize)
									xwcsncpy(const_cast<wchar_t*>(ListTitle->Title), strTitle.c_str(), ListTitle->TitleSize);
								else
									ListTitle->TitleSize=strTitle.size()+1;

								if (ListTitle->Bottom&&ListTitle->BottomSize)
									xwcsncpy(const_cast<wchar_t*>(ListTitle->Bottom), strBottomTitle.c_str(), ListTitle->BottomSize);
								else
									ListTitle->BottomSize=strBottomTitle.size()+1;
								return TRUE;
							}
							return FALSE;
						}
						case DM_LISTGETCURPOS: // Param1=ID Param2=FarListPos
						{
							const auto lp = static_cast<FarListPos*>(Param2);
							return CheckStructSize(lp)? ListBox->GetSelectPos(lp) : ListBox->GetSelectPos();
						}
						case DM_LISTSETCURPOS: // Param1=ID Param2=FarListPos Ret: RealPos
						{
							const auto lp = static_cast<const FarListPos*>(Param2);
							if(CheckStructSize(lp))
							{
								/* 26.06.2001 KM Подадим перед изменением позиции об этом сообщение */
								int CurListPos=ListBox->GetSelectPos();
								Ret=ListBox->SetSelectPos(static_cast<FarListPos*>(Param2));

								if (Ret!=CurListPos)
									if (!DlgProc(DN_LISTCHANGE,Param1,ToPtr(Ret)))
										Ret=ListBox->SetSelectPos(CurListPos,1);
							}
							else return -1;
							break; // т.к. нужно перерисовать!
						}
						case DM_GETCOMBOBOXEVENT: // Param1=ID Param2=0 Ret=Sets
						{
							return (CurItem->ListPtr && CurItem->ListPtr->CheckFlags(VMENU_COMBOBOXEVENTKEY)?CBET_KEY:0)|(CurItem->ListPtr && CurItem->ListPtr->CheckFlags(VMENU_COMBOBOXEVENTMOUSE)?CBET_MOUSE:0);
						}
						case DM_SETCOMBOBOXEVENT: // Param1=ID Param2=FARCOMBOBOXEVENTTYPE Ret=OldSets
						{
							auto OldSets = SendMessage(DM_GETCOMBOBOXEVENT, Param1, nullptr);
							if (CurItem->ListPtr)
							{
								CurItem->ListPtr->ClearFlags(VMENU_COMBOBOXEVENTKEY | VMENU_COMBOBOXEVENTMOUSE);

								if (reinterpret_cast<intptr_t>(Param2)&CBET_KEY)
									CurItem->ListPtr->SetMenuFlags(VMENU_COMBOBOXEVENTKEY);

								if (reinterpret_cast<intptr_t>(Param2)&CBET_MOUSE)
									CurItem->ListPtr->SetMenuFlags(VMENU_COMBOBOXEVENTMOUSE);

							}
							return OldSets;
						}
						default:
							break;
					}

					// уточнение для DI_COMBOBOX - здесь еще и DlgEdit нужно корректно заполнить
					if (!CurItem->IFlags.Check(DLGIIF_COMBOBOXNOREDRAWEDIT) && Type==DI_COMBOBOX && CurItem->ObjPtr)
					{
						if (ListBox->HasVisible())
						{
							const auto& ListMenuItem = ListBox->at(ListBox->GetSelectPos());
							const auto Edit = static_cast<DlgEdit*>(CurItem->ObjPtr);
							if (CurItem->Flags & (DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND))
								Edit->SetHiString(ListMenuItem.Name);
							else
								Edit->SetString(ListMenuItem.Name);
							Edit->RemoveSelection();
						}
					}

					if (DialogMode.Check(DMODE_SHOW) && ListBox->UpdateRequired())
					{
						SendMessage(DM_REDRAW, 0, nullptr);
					}

					return Ret;
				}
			}

			return FALSE;
		}
		/*****************************************************************/
		case DM_SETHISTORY: // Param1 = ID, Param2 = LPSTR HistoryName
		{
			if (Type==DI_EDIT || Type==DI_FIXEDIT)
			{
				if (Param2 && *static_cast<const wchar_t*>(Param2))
				{
					CurItem->Flags|=DIF_HISTORY;
					CurItem->strHistory = static_cast<const wchar_t*>(Param2);
					static_cast<DlgEdit*>(CurItem->ObjPtr)->SetHistory(CurItem->strHistory);
					if (Type==DI_EDIT && (CurItem->Flags&DIF_USELASTHISTORY))
					{
						ProcessLastHistory(CurItem, Param1);
					}
				}
				else
				{
					CurItem->Flags&=~DIF_HISTORY;
					CurItem->strHistory.clear();
				}

				if (DialogMode.Check(DMODE_SHOW))
				{
					SendMessage(DM_REDRAW, 0, nullptr);
				}

				return TRUE;
			}

			return FALSE;
		}
		/*****************************************************************/
		case DM_ADDHISTORY:
		{
			if (Param2 &&
			        (Type==DI_EDIT || Type==DI_FIXEDIT) &&
			        (CurItem->Flags & DIF_HISTORY))
			{
				return AddToEditHistory(CurItem, static_cast<const wchar_t*>(Param2));
			}

			return FALSE;
		}
		/*****************************************************************/
		case DM_GETCURSORPOS:
		{
			if (!Param2)
				return FALSE;

			if (IsEdit(Type) && CurItem->ObjPtr)
			{
				static_cast<COORD*>(Param2)->X = static_cast<DlgEdit*>(CurItem->ObjPtr)->GetCurPos();
				static_cast<COORD*>(Param2)->Y = 0;
				return TRUE;
			}
			else if (Type == DI_USERCONTROL && CurItem->UCData)
			{
				static_cast<COORD*>(Param2)->X = CurItem->UCData->CursorPos.X;
				static_cast<COORD*>(Param2)->Y = CurItem->UCData->CursorPos.Y;
				return TRUE;
			}

			return FALSE;
		}
		/*****************************************************************/
		case DM_SETCURSORPOS:
		{
			if (IsEdit(Type) && CurItem->ObjPtr && static_cast<COORD*>(Param2)->X >= 0)
			{
				const auto EditPtr = static_cast<DlgEdit*>(CurItem->ObjPtr);
				EditPtr->SetCurPos(static_cast<COORD*>(Param2)->X);
				//EditPtr->Show();
				EditPtr->SetClearFlag(false);
				redraw(false);
				return TRUE;
			}
			else if (Type == DI_USERCONTROL && CurItem->UCData)
			{
				// учтем, что координаты для этого элемента всегда относительные!
				//  и начинаются с 0,0
				COORD Coord=*static_cast<COORD*>(Param2);
				Coord.X+=CurItem->X1;

				if (Coord.X > CurItem->X2)
					Coord.X=CurItem->X2;

				Coord.Y+=CurItem->Y1;

				if (Coord.Y > CurItem->Y2)
					Coord.Y=CurItem->Y2;

				// Запомним
				CurItem->UCData->CursorPos.X=Coord.X-CurItem->X1;
				CurItem->UCData->CursorPos.Y=Coord.Y-CurItem->Y1;

				// переместим если надо
				if (DialogMode.Check(DMODE_SHOW) && m_FocusPos == static_cast<size_t>(Param1))
				{
					// что-то одно надо убрать :-)
					MoveCursor({ Coord.X + m_Where.left, Coord.Y + m_Where.top }); // ???
					redraw(false); //???
				}

				return TRUE;
			}

			return FALSE;
		}
		/*****************************************************************/
		case DM_GETEDITPOSITION:
		{
			if (Param2 && IsEdit(Type))
			{
				if (Type == DI_MEMOEDIT)
				{
					//EditorControl(ECTL_GETINFO,(EditorSetPosition *)Param2);
					return TRUE;
				}
				else
				{
					const auto esp = static_cast<EditorSetPosition*>(Param2);
					if (CheckStructSize(esp))
					{
						const auto EditPtr = static_cast<const DlgEdit*>(CurItem->ObjPtr);
						esp->CurLine=0;
						esp->CurPos=EditPtr->GetCurPos();
						esp->CurTabPos=EditPtr->GetTabCurPos();
						esp->TopScreenLine=0;
						esp->LeftPos=EditPtr->GetLeftPos();
						esp->Overtype=EditPtr->GetOvertypeMode();
						return TRUE;
					}
				}
			}

			return FALSE;
		}
		/*****************************************************************/
		case DM_SETEDITPOSITION:
		{
			if (Param2 && IsEdit(Type))
			{
				if (Type == DI_MEMOEDIT)
				{
					//EditorControl(ECTL_SETPOSITION,(EditorSetPosition *)Param2);
					return TRUE;
				}
				else
				{
					const auto esp = static_cast<const EditorSetPosition*>(Param2);
					if (CheckStructSize(esp))
					{
						const auto EditPtr = static_cast<DlgEdit*>(CurItem->ObjPtr);
						if(esp->CurPos>=0)
							EditPtr->SetCurPos(esp->CurPos);
						if(esp->CurTabPos>=0)
							EditPtr->SetTabCurPos(esp->CurTabPos);
						if(esp->LeftPos>=0)
							EditPtr->SetLeftPos(esp->LeftPos);
						if(esp->Overtype>=0)
							EditPtr->SetOvertypeMode(esp->Overtype!=0);
						SendMessage(DM_REDRAW, 0, nullptr);
						return TRUE;
					}
				}
			}

			return FALSE;
		}
		/*****************************************************************/
		/*
		   Param2=0
		   Return MAKELONG(Visible,Size)
		*/
		case DM_GETCURSORSIZE:
		{
			if (IsEdit(Type) && CurItem->ObjPtr)
			{
				bool Visible{};
				size_t Size{};
				static_cast<DlgEdit*>(CurItem->ObjPtr)->GetCursorType(Visible,Size);
				return MAKELONG(Visible,Size);
			}
			else if (Type == DI_USERCONTROL && CurItem->UCData)
			{
				return MAKELONG(CurItem->UCData->CursorVisible,CurItem->UCData->CursorSize);
			}

			return FALSE;
		}
		/*****************************************************************/
		// Param2=MAKELONG(Visible,Size)
		//   Return MAKELONG(OldVisible,OldSize)
		case DM_SETCURSORSIZE:
		{
			bool Visible{};
			size_t Size{};

			if (IsEdit(Type) && CurItem->ObjPtr)
			{
				static_cast<DlgEdit*>(CurItem->ObjPtr)->GetCursorType(Visible,Size);
				static_cast<DlgEdit*>(CurItem->ObjPtr)->SetCursorType(LOWORD(Param2)!=0,HIWORD(Param2));
			}
			else if (Type == DI_USERCONTROL && CurItem->UCData)
			{
				Visible=CurItem->UCData->CursorVisible;
				Size=CurItem->UCData->CursorSize;
				CurItem->UCData->CursorVisible=LOWORD(Param2)!=0;
				CurItem->UCData->CursorSize=HIWORD(Param2);
				int CCX=CurItem->UCData->CursorPos.X;
				int CCY=CurItem->UCData->CursorPos.Y;

				if (DialogMode.Check(DMODE_SHOW) &&
				        m_FocusPos == static_cast<size_t>(Param1) &&
				        CCX != -1 && CCY != -1)
					SetCursorType(CurItem->UCData->CursorVisible,CurItem->UCData->CursorSize);
			}

			return MAKELONG(Visible,Size);
		}
		/*****************************************************************/
		case DN_LISTCHANGE:
		{
			return DlgProc(Msg,Param1,Param2);
		}
		/*****************************************************************/
		case DN_EDITCHANGE:
		{
			FarGetDialogItem Item={sizeof(FarGetDialogItem),0,nullptr};
			Item.Size=ConvertItemEx2(CurItem,nullptr);
			block_ptr<FarDialogItem> Buffer(Item.Size);
			Item.Item = Buffer.data();
			intptr_t I=FALSE;
			if(ConvertItemEx2(CurItem,&Item)<=Item.Size)
			{
				if(CurItem->Type==DI_EDIT||CurItem->Type==DI_COMBOBOX||CurItem->Type==DI_FIXEDIT||CurItem->Type==DI_PSWEDIT)
				{
					static_cast<DlgEdit*>(CurItem->ObjPtr)->SetCallbackState(false);
					I=DlgProc(DN_EDITCHANGE,Param1,Item.Item);
					if (I)
					{
						if (Type == DI_COMBOBOX && CurItem->ListPtr)
							CurItem->ListPtr->ChangeFlags(VMENU_DISABLED, (CurItem->Flags&DIF_DISABLE)!=0);
					}
					static_cast<DlgEdit*>(CurItem->ObjPtr)->SetCallbackState(true);
				}
			}
			return I;
		}
		/*****************************************************************/
		case DN_BTNCLICK:
		{
			intptr_t Ret=DlgProc(Msg,Param1,Param2);

			if (Ret && (CurItem->Flags&DIF_AUTOMATION) && !CurItem->Auto.empty())
			{
				const auto iParam = reinterpret_cast<intptr_t>(Param2) % 3;

				for (const auto& i: CurItem->Auto)
				{
					const auto NewFlags = i.Owner->Flags;
					i.Owner->Flags = (NewFlags & (~i.Flags[iParam][1])) | i.Flags[iParam][0];
					// здесь намеренно в обработчик не посылаются эвенты об изменении
					// состояния...
				}
			}

			return Ret;
		}
		/*****************************************************************/
		case DM_GETCHECK:
		{
			if (Type==DI_CHECKBOX || Type==DI_RADIOBUTTON)
				return CurItem->Selected;

			return 0;
		}
		/*****************************************************************/
		case DM_SET3STATE:
		{
			if (Type == DI_CHECKBOX)
			{
				int OldState = (CurItem->Flags&DIF_3STATE) != 0;

				if (Param2)
					CurItem->Flags|=DIF_3STATE;
				else
					CurItem->Flags&=~DIF_3STATE;

				return OldState;
			}

			return 0;
		}
		/*****************************************************************/
		case DM_SETCHECK:
		{
			if (Type == DI_CHECKBOX)
			{
				int Selected=CurItem->Selected;
				auto State = reinterpret_cast<intptr_t>(Param2);
				if (State == BSTATE_TOGGLE)
					State=++Selected;

				if (CurItem->Flags&DIF_3STATE)
					State%=3;
				else
					State&=1;

				CurItem->Selected=State;

				if (Selected != State && DialogMode.Check(DMODE_SHOW))
				{
					// автоматизация
					if ((CurItem->Flags&DIF_AUTOMATION) && !CurItem->Auto.empty())
					{
						State%=3;
						for (const auto& i: CurItem->Auto)
						{
							const auto NewFlags = i.Owner->Flags;
							i.Owner->Flags = (NewFlags & (~i.Flags[State][1])) | i.Flags[State][0];
							// здесь намеренно в обработчик не посылаются эвенты об изменении
							// состояния...
						}
						Param1=-1;
					}

					SendMessage(DM_REDRAW, 0, nullptr);
				}

				return Selected;
			}
			else if (Type == DI_RADIOBUTTON)
			{
				Param1 = ProcessRadioButton(Param1, Param2 == ToPtr(BSTATE_3STATE));

				if (DialogMode.Check(DMODE_SHOW))
				{
					SendMessage(DM_REDRAW, 0, nullptr);
				}

				return Param1;
			}

			return 0;
		}
		/*****************************************************************/
		case DN_DRAWDLGITEM:
		{
			FarGetDialogItem Item={sizeof(FarGetDialogItem),0,nullptr};
			Item.Size=ConvertItemEx2(CurItem,nullptr);
			block_ptr<FarDialogItem> Buffer(Item.Size);
			Item.Item = Buffer.data();
			intptr_t I=FALSE;
			if(ConvertItemEx2(CurItem,&Item)<=Item.Size)
			{
				I=DlgProc(Msg,Param1,Item.Item);

				if ((Type == DI_LISTBOX || Type == DI_COMBOBOX) && CurItem->ListPtr)
					CurItem->ListPtr->ChangeFlags(VMENU_DISABLED, (CurItem->Flags&DIF_DISABLE)!=0);
			}
			return I;
		}
		/*****************************************************************/
		case DM_SETFOCUS:
		{
			if (!CanGetFocus(Type))
				return FALSE;

			if (m_FocusPos == static_cast<size_t>(Param1)) // уже и так установлено все!
				return TRUE;

			ChangeFocus2(Param1);

			if (m_FocusPos == static_cast<size_t>(Param1))
			{
				if (DialogMode.Check(DMODE_DRAWING))
					DialogMode.Set(DMODE_NEEDUPDATE);
				else
					redraw(false);
				return TRUE;
			}

			return FALSE;
		}
		/*****************************************************************/
		case DM_GETFOCUS: // Получить ID фокуса
		{
			return m_FocusPos;
		}
		/*****************************************************************/
		case DM_GETCONSTTEXTPTR:
		{
			return reinterpret_cast<intptr_t>(Ptr);
		}
		/*****************************************************************/
		// Param1=ID, Param2=FarDialogItemData, Ret=size (without '\0')
		case DM_GETTEXT:
		{
			const auto did = static_cast<FarDialogItemData*>(Param2);
			const auto InitItemData = [did, &Ptr, &Len]
			{
				if (!did->PtrLength)
					did->PtrLength=Len; //BUGBUG: PtrLength размер переданного нам буфера, зачем мы его меняем?
				else if (Len > did->PtrLength)
					Len=did->PtrLength;

				if (did->PtrData)
				{
					std::copy_n(Ptr, Len, did->PtrData);
					did->PtrData[Len] = {};
				}
			};
			if (CheckStructSize(did)) // если здесь nullptr, то это еще один способ получить размер
			{
				Len=0;

				switch (Type)
				{
					case DI_MEMOEDIT:
						break;
					case DI_COMBOBOX:
					case DI_EDIT:
					case DI_PSWEDIT:
					case DI_FIXEDIT:
					{
						const auto edit = static_cast<const DlgEdit*>(CurItem->ObjPtr);
						if (edit)
						{
							Ptr = edit->GetString().data();
							Len = edit->GetLength();
							InitItemData();
						}
						break;
					}
					case DI_TEXT:
					case DI_VTEXT:
					case DI_SINGLEBOX:
					case DI_DOUBLEBOX:
					case DI_CHECKBOX:
					case DI_RADIOBUTTON:
					case DI_BUTTON:
						Ptr = CurItem->strData.data();
						Len = CurItem->strData.size();

						if (Type == DI_BUTTON)
						{
							if(!(CurItem->Flags & DIF_NOBRACKETS))
							{
								Ptr+=2;
								Len-=4;
							}
							if(CurItem->Flags & DIF_SETSHIELD)
							{
								Ptr+=2;
							}
						}
						InitItemData();
						break;
					case DI_USERCONTROL:
						/*did->PtrLength=CurItem->Ptr.PtrLength; BUGBUG
						did->PtrData=(char*)CurItem->Ptr.PtrData;*/
						break;
					case DI_LISTBOX:
					{
						if (CurItem->ListPtr->GetShowItemCount())
						{
							const auto& ListMenuItem = CurItem->ListPtr->current();
							Ptr = ListMenuItem.Name.data();
							Len = ListMenuItem.Name.size();
						}
						InitItemData();
						break;
					}
					default:  // подразумеваем, что остались
						break;
				}

				return Len;
			}

			//получаем размер
			switch (Type)
			{
				case DI_BUTTON:
					Len = CurItem->strData.size();
					if (!(CurItem->Flags & DIF_NOBRACKETS))
						Len-=4;
					break;

				case DI_USERCONTROL:
					//Len=CurItem->Ptr.PtrLength; BUGBUG
					break;

				case DI_TEXT:
				case DI_VTEXT:
				case DI_SINGLEBOX:
				case DI_DOUBLEBOX:
				case DI_CHECKBOX:
				case DI_RADIOBUTTON:
					Len = CurItem->strData.size();
					break;

				case DI_COMBOBOX:
				case DI_EDIT:
				case DI_PSWEDIT:
				case DI_FIXEDIT:
				case DI_MEMOEDIT:
					if (CurItem->ObjPtr)
					{
						Len = static_cast<DlgEdit*>(CurItem->ObjPtr)->GetLength();
						break;
					}
					[[fallthrough]];
				case DI_LISTBOX:
					Len=0;
					if (CurItem->ListPtr->GetShowItemCount())
					{
						Len = CurItem->ListPtr->current().Name.size();
					}
					break;

				default:
					Len=0;
					break;
			}

			return Len;
		}
		/*****************************************************************/
		case DM_SETTEXTPTR:
		{
			wchar_t* Text = Param2?static_cast<wchar_t*>(Param2):const_cast<wchar_t*>(L"");
			FarDialogItemData IData = { sizeof(FarDialogItemData), wcslen(Text), Text };
			return SendMessage(DM_SETTEXT,Param1,&IData);
		}
		/*****************************************************************/
		case DM_SETTEXT:
		{
			const auto did = static_cast<const FarDialogItemData*>(Param2);
			if (CheckStructSize(did))
			{
				int NeedInit=TRUE;

				switch (Type)
				{
					case DI_MEMOEDIT:
						break;
					case DI_COMBOBOX:
					case DI_EDIT:
					case DI_TEXT:
					case DI_VTEXT:
					case DI_SINGLEBOX:
					case DI_DOUBLEBOX:
					case DI_BUTTON:
					case DI_CHECKBOX:
					case DI_RADIOBUTTON:
					case DI_PSWEDIT:
					case DI_FIXEDIT:
					case DI_LISTBOX: // меняет только текущий элемент
						CurItem->strData.assign(did->PtrData, did->PtrLength);
						Len = CurItem->strData.size();
						break;
					default:
						Len=0;
						break;
				}

				switch (Type)
				{
					case DI_USERCONTROL:
						/*CurItem->Ptr.PtrLength=did->PtrLength;
						CurItem->Ptr.PtrData=did->PtrData;
						return CurItem->Ptr.PtrLength;*/
						return 0; //BUGBUG
					case DI_TEXT:
					case DI_VTEXT:
					case DI_SINGLEBOX:
					case DI_DOUBLEBOX:

						if (DialogMode.Check(DMODE_SHOW))
						{
							ShowConsoleTitle();
							SendMessage(DM_REDRAW, 0, nullptr);
						}

						return Len-(!Len?0:1);
					case DI_BUTTON:
					case DI_CHECKBOX:
					case DI_RADIOBUTTON:
						break;
					case DI_MEMOEDIT:
						break;
					case DI_COMBOBOX:
					case DI_EDIT:
					case DI_PSWEDIT:
					case DI_FIXEDIT:
						NeedInit=FALSE;

						if (CurItem->ObjPtr)
						{
							const auto EditLine = static_cast<DlgEdit*>(CurItem->ObjPtr);
							const auto ReadOnly = EditLine->GetReadOnly();
							const auto IsUnchanged = EditLine->GetClearFlag();

							EditLine->SetReadOnly(false);
							{
								SCOPED_ACTION(SetAutocomplete)(EditLine);
								EditLine->SetString(CurItem->strData);
								EditLine->SetLeftPos(0);
							}
							EditLine->SetReadOnly(ReadOnly);

							// не меняем clear-флаг, пока не проиницализировались
							EditLine->SetClearFlag(DialogMode.Check(DMODE_OBJECTS_INITED)? true : IsUnchanged);
						}

						break;
					case DI_LISTBOX: // меняет только текущий элемент
					{
						auto& ListBox = CurItem->ListPtr;

						if (ListBox && !ListBox->empty())
						{
							FarListUpdate LUpdate={sizeof(FarListUpdate)};
							LUpdate.Index=ListBox->GetSelectPos();
							auto& ListMenuItem = ListBox->at(LUpdate.Index);
							LUpdate.Item.Flags = ListMenuItem.Flags;
							LUpdate.Item.Text = CurItem->strData.c_str();
							LUpdate.Item.UserData = ListMenuItem.SimpleUserData;
							SendMessage(DM_LISTUPDATE, Param1, &LUpdate);

							break;
						}
						else
							return 0;
					}
					default:  // подразумеваем, что остались
						return 0;
				}

				if (NeedInit)
					InitDialogObjects(Param1); // переинициализируем элементы диалога

				if (DialogMode.Check(DMODE_SHOW)) // достаточно ли этого????!!!!
				{
					SendMessage(DM_REDRAW, 0, nullptr);
				}

				//CurItem->strData = did->PtrData;
				return CurItem->strData.size(); //???
			}

			return 0;
		}
		/*****************************************************************/
		case DM_SETMAXTEXTLENGTH:
		{
			if ((Type==DI_EDIT || Type==DI_PSWEDIT ||
			        (Type==DI_COMBOBOX && !(CurItem->Flags & DIF_DROPDOWNLIST))) &&
			        CurItem->ObjPtr)
			{
				int MaxLen = static_cast<DlgEdit*>(CurItem->ObjPtr)->GetMaxLength();
				// BugZ#628 - Неправильная длина редактируемого текста.
				static_cast<DlgEdit*>(CurItem->ObjPtr)->SetMaxLength(static_cast<int>(reinterpret_cast<intptr_t>(Param2)));
				//if (DialogMode.Check(DMODE_INITOBJECTS)) //???
				InitDialogObjects(Param1); // переинициализируем элементы диалога
				ShowConsoleTitle();
				return MaxLen;
			}

			return 0;
		}
		/*****************************************************************/
		case DM_GETDLGITEM:
		{
			const auto Item = static_cast<FarGetDialogItem*>(Param2);
			return (CheckNullOrStructSize(Item)) ? static_cast<intptr_t>(ConvertItemEx2(CurItem, Item)) : 0;
		}
		/*****************************************************************/
		case DM_GETDLGITEMSHORT:
		{
			if (Param2)
			{
				ConvertItemSmall(*CurItem, *static_cast<FarDialogItem*>(Param2));
				return TRUE;
			}
			return FALSE;
		}
		/*****************************************************************/
		case DM_SETDLGITEM:
		case DM_SETDLGITEMSHORT:
		{
			if (!Param2)
				return FALSE;

			const auto Item = static_cast<const FarDialogItem*>(Param2);

			if (Type != Item->Type) // пока нефига менять тип
				return FALSE;

			ItemsToItemsEx({ Item, 1 }, { CurItem, 1 }, Msg != DM_SETDLGITEM);

			CurItem->Type=Type;

			if ((Type == DI_LISTBOX || Type == DI_COMBOBOX) && CurItem->ListPtr)
				CurItem->ListPtr->ChangeFlags(VMENU_DISABLED, (CurItem->Flags&DIF_DISABLE)!=0);

			// еще разок, т.к. данные могли быть изменены
			InitDialogObjects(Param1);
			ShowConsoleTitle();
			if (DialogMode.Check(DMODE_SHOW))
			{
				SendMessage(DM_REDRAW, 0, nullptr);
			}

			return TRUE;
		}
		/*****************************************************************/
		/* $ 03.01.2001 SVS
		    + показать/скрыть элемент
		    Param2: -1 - получить состояние
		             0 - погасить
		             1 - показать
		    Return:  предыдущее состояние
		*/
		case DM_SHOWITEM:
		{
			FARDIALOGITEMFLAGS PrevFlags=CurItem->Flags;

			if (reinterpret_cast<intptr_t>(Param2) != -1)
			{
				if (Param2)
					CurItem->Flags&=~DIF_HIDDEN;
				else
					CurItem->Flags|=DIF_HIDDEN;

				if (DialogMode.Check(DMODE_SHOW))// && (PrevFlags&DIF_HIDDEN) != (CurItem->Flags&DIF_HIDDEN))//!(CurItem->Flags&DIF_HIDDEN))
				{
					if ((CurItem->Flags&DIF_HIDDEN) && m_FocusPos == static_cast<size_t>(Param1))
					{
						ChangeFocus2(ChangeFocus(Param1, 1, true));
					}

					SendMessage(DM_REDRAW, 0, nullptr);
				}
			}

			return !(PrevFlags&DIF_HIDDEN);
		}
		/*****************************************************************/
		case DM_SETDROPDOWNOPENED: // Param1=ID; Param2={TRUE|FALSE}
		{
			if (!Param2) // Закрываем любой открытый комбобокс или историю
			{
				if (GetDropDownOpened())
				{
					SetDropDownOpened(FALSE);
					os::chrono::sleep_for(10ms);
				}

				return TRUE;
			}
			/* $ 09.12.2001 DJ
			   у DI_PSWEDIT не бывает хистори!
			*/
			if ((Type==DI_COMBOBOX || ((Type==DI_EDIT || Type==DI_FIXEDIT)
			                    && (CurItem->Flags&DIF_HISTORY)))) /* DJ $ */
			{
				// Открываем заданный в Param1 комбобокс или историю
				if (GetDropDownOpened())
				{
					SetDropDownOpened(FALSE);
					os::chrono::sleep_for(10ms);
				}

				if (SendMessage(DM_SETFOCUS, Param1, nullptr))
				{
					ProcessOpenComboBox(Type,CurItem,Param1); //?? Param1 ??
					//ProcessKey(KEY_CTRLDOWN);
					return TRUE;
				}
				else
					return FALSE;
			}

			return FALSE;
		}
		/* KM $ */
		/*****************************************************************/
		case DM_SETITEMPOSITION: // Param1 = ID; Param2 = SMALL_RECT
		{
			return SetItemRect(Param1, *static_cast<SMALL_RECT*>(Param2));
		}
		/*****************************************************************/
		/* $ 31.08.2000 SVS
		    + переключение/получение состояния Enable/Disable элемента
		*/
		case DM_ENABLE:
		{
			FARDIALOGITEMFLAGS PrevFlags=CurItem->Flags;

			if (reinterpret_cast<intptr_t>(Param2) != -1)
			{
				if (Param2)
					CurItem->Flags&=~DIF_DISABLE;
				else
					CurItem->Flags|=DIF_DISABLE;

				if ((Type == DI_LISTBOX || Type == DI_COMBOBOX) && CurItem->ListPtr)
					CurItem->ListPtr->ChangeFlags(VMENU_DISABLED, (CurItem->Flags&DIF_DISABLE)!=0);
			}

			if (DialogMode.Check(DMODE_SHOW)) //???
			{
				SendMessage(DM_REDRAW, 0, nullptr);
			}

			return !(PrevFlags&DIF_DISABLE);
		}
		/*****************************************************************/
		// получить позицию и размеры контрола
		case DM_GETITEMPOSITION: // Param1=ID, Param2=*SMALL_RECT

			if (Param2)
			{
				SMALL_RECT Rect;
				if (GetItemRect(Param1,Rect))
				{
					*static_cast<PSMALL_RECT>(Param2) = Rect;
					return TRUE;
				}
			}

			return FALSE;
			/*****************************************************************/
		case DM_SETITEMDATA:
		{
			intptr_t PrewDataDialog=CurItem->UserData;
			CurItem->UserData = reinterpret_cast<intptr_t>(Param2);
			return PrewDataDialog;
		}
		/*****************************************************************/
		case DM_GETITEMDATA:
		{
			return CurItem->UserData;
		}
		/*****************************************************************/
		case DM_EDITUNCHANGEDFLAG: // -1 Get, 0 - Skip, 1 - Set; Выделение блока снимается.
		{
			if (IsEdit(Type))
			{
				const auto EditLine = static_cast<DlgEdit*>(CurItem->ObjPtr);
				const auto ClearFlag = EditLine->GetClearFlag();

				if (reinterpret_cast<intptr_t>(Param2) >= 0)
				{
					EditLine->SetClearFlag(Param2 != nullptr);
					EditLine->RemoveSelection();

					if (DialogMode.Check(DMODE_SHOW)) //???
					{
						SendMessage(DM_REDRAW, 0, nullptr);
					}
				}

				return ClearFlag;
			}

			break;
		}
		/*****************************************************************/
		case DM_GETSELECTION: // Msg=DM_GETSELECTION, Param1=ID, Param2=*EditorSelect
		case DM_SETSELECTION: // Msg=DM_SETSELECTION, Param1=ID, Param2=*EditorSelect
		{
			const auto EdSel = static_cast<EditorSelect*>(Param2);
			if (IsEdit(Type) && CheckStructSize(EdSel))
			{
				if (Msg == DM_GETSELECTION)
				{
					const auto EditLine = static_cast<const DlgEdit*>(CurItem->ObjPtr);
					EdSel->BlockStartLine=0;
					EdSel->BlockHeight=1;
					EditLine->GetSelection(EdSel->BlockStartPos,EdSel->BlockWidth);

					if (EdSel->BlockStartPos == -1 && !EdSel->BlockWidth)
						EdSel->BlockType=BTYPE_NONE;
					else
					{
						EdSel->BlockType=BTYPE_STREAM;
						EdSel->BlockWidth-=EdSel->BlockStartPos;
					}

					return TRUE;
				}
				else
				{
					const auto EditLine = static_cast<DlgEdit*>(CurItem->ObjPtr);

					//EdSel->BlockType=BTYPE_STREAM;
					//EdSel->BlockStartLine=0;
					//EdSel->BlockHeight=1;
					if (EdSel->BlockType==BTYPE_NONE)
						EditLine->RemoveSelection();
					else
						EditLine->Select(EdSel->BlockStartPos,EdSel->BlockStartPos+EdSel->BlockWidth);

					EditLine->SetClearFlag(false);

					if (DialogMode.Check(DMODE_SHOW)) //???
					{
						SendMessage(DM_REDRAW, 0, nullptr);
					}

					return TRUE;
				}
			}

			break;
		}
		default:
			break;
	}

	// Все, что сами не отрабатываем - посылаем на обработку обработчику.
	return DlgProc(Msg,Param1,Param2);
}

void Dialog::SetPosition(rectangle Where)
{
	if (Where.left != -1)
		RealWidth = Where.width();
	else
		RealWidth = Where.right;

	if (Where.top != -1)
		RealHeight = Where.height();
	else
		RealHeight = Where.bottom;

	ScreenObjectWithShadow::SetPosition(Where);
}
//////////////////////////////////////////////////////////////////////////
bool Dialog::IsInited() const
{
	return DialogMode.Check(DMODE_OBJECTS_INITED);
}

rectangle Dialog::CalcComboBoxPos(const DialogItemEx* CurItem, intptr_t ItemCount) const
{
	if(!CurItem)
	{
		CurItem = &Items[m_FocusPos];
	}

	auto Rect = static_cast<DlgEdit*>(CurItem->ObjPtr)->GetPosition();

	if (Rect.width() <= 20)
		Rect.right = Rect.left + 20;

	if (ScrY - Rect.top<std::min(Global->Opt->Dialogs.CBoxMaxHeight.Get(), static_cast<long long>(ItemCount)) + 2 && Rect.top > ScrY / 2)
	{
		Rect.bottom = Rect.top - 1;
		Rect.top = std::max(0ll, Rect.top - 1 - std::min(Global->Opt->Dialogs.CBoxMaxHeight.Get(), static_cast<long long>(ItemCount)) - 1);
	}
	else
	{
		++Rect.top;
		Rect.bottom = 0;
	}

	return Rect;
}

void Dialog::SetComboBoxPos(DialogItemEx* Item)
{
	if (GetDropDownOpened())
	{
		if(!Item)
		{
			Item = &Items[m_FocusPos];
		}
		if (Item->ListPtr)
		{
			Item->ListPtr->SetPosition(CalcComboBoxPos(Item, Item->ListPtr->size()));
		}
	}
}

bool Dialog::ProcessEvents()
{
	return !DialogMode.Check(DMODE_ENDLOOP);
}

void Dialog::SetId(const UUID& Id)
{
	m_Id=Id;
	IdExist=true;
}

class dialogs_set: public singleton<dialogs_set>
{
	IMPLEMENTS_SINGLETON;

public:
	std::unordered_set<Dialog*> Set;
};

void Dialog::AddToList()
{
	if (!dialogs_set::instance().Set.emplace(this).second) assert(false);
}

void Dialog::RemoveFromList()
{
	if (!dialogs_set::instance().Set.erase(this)) assert(false);
}

bool Dialog::IsValid(Dialog* Handle)
{
	return contains(dialogs_set::instance().Set, Handle);
}

void Dialog::SetDeleting()
{
}

void Dialog::ShowConsoleTitle()
{
	ConsoleTitle::SetFarTitle(DialogMode.Check(DMODE_KEEPCONSOLETITLE)? m_ConsoleTitle : GetTitle());
}

Dialog::suppress_redraw::suppress_redraw(Dialog* Dlg):
	m_Dlg(Dlg)
{
	m_Dlg->SendMessage(DM_ENABLEREDRAW, 0, nullptr);
}

Dialog::suppress_redraw::~suppress_redraw()
{
	m_Dlg->SendMessage(DM_ENABLEREDRAW, 1, nullptr);
}
