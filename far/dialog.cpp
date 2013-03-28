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

#include "headers.hpp"
#pragma hdrstop

#include "dialog.hpp"
#include "keyboard.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "chgprior.hpp"
#include "vmenu.hpp"
#include "dlgedit.hpp"
#include "help.hpp"
#include "scrbuf.hpp"
#include "manager.hpp"
#include "savescr.hpp"
#include "constitle.hpp"
#include "lockscrn.hpp"
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "TaskBar.hpp"
#include "interf.hpp"
#include "palette.hpp"
#include "message.hpp"
#include "strmix.hpp"
#include "history.hpp"
#include "FarGuid.hpp"
#include "colormix.hpp"
#include "mix.hpp"
#include "plugins.hpp"

#define VTEXT_ADN_SEPARATORS	1

// Флаги для функции ConvertItem
enum CVTITEMFLAGS
{
	CVTITEM_FROMPLUGIN      = 1,
	CVTITEM_TOPLUGINSHORT   = 2,
	CVTITEM_FROMPLUGINSHORT = 3
};

enum DLGEDITLINEFLAGS
{
	DLGEDITLINE_CLEARSELONKILLFOCUS = 0x00000001, // управляет выделением блока при потере фокуса ввода
	DLGEDITLINE_SELALLGOTFOCUS      = 0x00000002, // управляет выделением блока при получении фокуса ввода
	DLGEDITLINE_NOTSELONGOTFOCUS    = 0x00000004, // не восстанавливать выделение строки редактирования при получении фокуса ввода
	DLGEDITLINE_NEWSELONGOTFOCUS    = 0x00000008, // управляет процессом выделения блока при получении фокуса
	DLGEDITLINE_GOTOEOLGOTFOCUS     = 0x00000010, // при получении фокуса ввода переместить курсор в конец строки
};

enum DLGITEMINTERNALFLAGS
{
	DLGIIF_COMBOBOXNOREDRAWEDIT     = 0x00000008, // не прорисовывать строку редактирования при изменениях в комбо
	DLGIIF_COMBOBOXEVENTKEY         = 0x00000010, // посылать события клавиатуры в диалоговую проц. для открытого комбобокса
	DLGIIF_COMBOBOXEVENTMOUSE       = 0x00000020, // посылать события мыши в диалоговую проц. для открытого комбобокса
};

class DlgUserControl
{
	public:
		COORD CursorPos;
		bool CursorVisible;
		DWORD CursorSize;

	public:
		DlgUserControl():
			CursorVisible(false),
			CursorSize(static_cast<DWORD>(-1))
		{
			CursorPos.X=CursorPos.Y=-1;
		}
		~DlgUserControl() {};
};

//////////////////////////////////////////////////////////////////////////
/*
   Функция, определяющая - "Может ли элемент диалога иметь фокус ввода"
*/
static inline bool CanGetFocus(int Type)
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

bool IsKeyHighlighted(const wchar_t *Str,int Key,int Translate,int AmpPos)
{
	if (AmpPos == -1)
	{
		if (!(Str=wcschr(Str,L'&')))
			return FALSE;

		AmpPos=1;
	}
	else
	{
		if (AmpPos >= StrLength(Str))
			return FALSE;

		Str=Str+AmpPos;
		AmpPos=0;

		if (Str[AmpPos] == L'&')
			AmpPos++;
	}

	int UpperStrKey=Upper((int)Str[AmpPos]);

	if (Key < 0xFFFF)
	{
		return UpperStrKey == (int)Upper(Key) || (Translate && KeyToKeyLayoutCompare(Key,UpperStrKey));
	}

	if (Key&(KEY_ALT|KEY_RALT))
	{
		int AltKey=Key&(~(KEY_ALT|KEY_RALT));

		if (AltKey < 0xFFFF)
		{
			if ((unsigned int)AltKey >= L'0' && (unsigned int)AltKey <= L'9')
				return(AltKey==UpperStrKey);

			if ((unsigned int)AltKey > L' ' && AltKey <= 0xFFFF)
				//         (AltKey=='-'  || AltKey=='/' || AltKey==','  || AltKey=='.' ||
				//          AltKey=='\\' || AltKey=='=' || AltKey=='['  || AltKey==']' ||
				//          AltKey==':'  || AltKey=='"' || AltKey=='~'))
			{
				return(UpperStrKey==(int)Upper(AltKey) || (Translate && KeyToKeyLayoutCompare(AltKey,UpperStrKey)));
			}
		}
	}

	return false;
}

static void ConvertItemSmall(const DialogItemEx& From, FarDialogItem& To)
{
	To = static_cast<FarDialogItem>(From);

	To.Data = nullptr;
	To.History = nullptr;
	To.Mask = nullptr;
	To.Reserved0 = From.Reserved0;
	To.UserData = From.UserData;
}

size_t ItemStringAndSize(const DialogItemEx *Data,string& ItemString)
{
	//TODO: тут видимо надо сделать поумнее
	ItemString=Data->strData;

	if (IsEdit(Data->Type))
	{
		DlgEdit *EditPtr;

		if ((EditPtr = (DlgEdit *)(Data->ObjPtr)) )
			EditPtr->GetString(ItemString);
	}

	size_t sz = ItemString.GetLength();

	if (sz > Data->MaxLength && Data->MaxLength > 0)
		sz = Data->MaxLength;

	return sz;
}

static bool ConvertItemEx(
    CVTITEMFLAGS FromPlugin,
    FarDialogItem *Item,
    DialogItemEx *ItemEx,
    size_t Count
)
{
	if (!Item || !ItemEx)
		return false;

	switch (FromPlugin)
	{
		case CVTITEM_TOPLUGINSHORT:
			for (size_t i = 0; i < Count; ++i, ++Item, ++ItemEx)
			{
				ConvertItemSmall(*ItemEx, *Item);
			}
			break;

		case CVTITEM_FROMPLUGIN:
		case CVTITEM_FROMPLUGINSHORT:
			ItemToItemEx(Item, ItemEx, Count, FromPlugin == CVTITEM_FROMPLUGINSHORT);
			break;
	}

	return true;
}

static size_t ConvertItemEx2(const DialogItemEx *ItemEx, FarGetDialogItem *Item)
{
	size_t size=sizeof(FarDialogItem);
	string str;
	size_t sz = ItemStringAndSize(ItemEx,str);
	size+=(sz+1)*sizeof(wchar_t);
	size+=(ItemEx->strHistory.GetLength()+1)*sizeof(wchar_t);
	size+=(ItemEx->strMask.GetLength()+1)*sizeof(wchar_t);

	if (Item)
	{
		if(Item->Item && Item->Size >= size)
		{
			ConvertItemSmall(*ItemEx, *Item->Item);

			wchar_t* p=(wchar_t*)(Item->Item+1);
			Item->Item->Data = p;
			wmemcpy(p, str.CPtr(), sz+1);
			p+=sz+1;
			Item->Item->History = p;
			wmemcpy(p, ItemEx->strHistory.CPtr(), ItemEx->strHistory.GetLength()+1);
			p+=ItemEx->strHistory.GetLength()+1;
			Item->Item->Mask = p;
			wmemcpy(p, ItemEx->strMask.CPtr(), ItemEx->strMask.GetLength()+1);
		}
	}
	return size;
}

void ItemToItemEx(const FarDialogItem *Item, DialogItemEx *ItemEx, size_t Count, bool Short)
{
	if (!Item || !ItemEx)
		return;

	for (size_t i = 0; i < Count; ++i, ++Item, ++ItemEx)
	{
		*ItemEx = *Item;

		ItemEx->ID = static_cast<int>(i);
		if(!Short)
		{
			ItemEx->strHistory = Item->History;
			ItemEx->strMask = Item->Mask;
			if(Item->Data)
			{
				ItemEx->strData.Copy(Item->Data, Item->MaxLength?Item->MaxLength:StrLength(Item->Data));
			}
		}
		ItemEx->SelStart=-1;

		ItemEx->X2 = std::max(ItemEx->X1, ItemEx->X2);
		ItemEx->Y2 = std::max(ItemEx->Y1, ItemEx->Y2);

		if ((ItemEx->Type == DI_COMBOBOX || ItemEx->Type == DI_LISTBOX) && !Global->IsPtr(Item->ListItems))
		{
			ItemEx->ListItems=nullptr;
		}
	}
}

intptr_t DefProcFunction(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	return Dlg->DefProc(Msg, Param1, Param2);
}

void Dialog::Construct(DialogItemEx* SrcItem, size_t SrcItemCount, DialogOwner* OwnerClass, MemberHandlerFunction HandlerFunction, StaticHandlerFunction DlgProc, void* InitParam)
{
	SavedItems = SrcItem;

	Items.resize(SrcItemCount);
	std::for_each(RANGE(Items, i)
	{
		i.reset(new DialogItemEx(*SrcItem++));
	});
	Init(OwnerClass, HandlerFunction, DlgProc, InitParam);
}

void Dialog::Construct(const FarDialogItem* SrcItem, size_t SrcItemCount, DialogOwner* OwnerClass, MemberHandlerFunction HandlerFunction, StaticHandlerFunction DlgProc, void* InitParam)
{
	SavedItems = nullptr;

	Items.resize(SrcItemCount);
	std::for_each(RANGE(Items, i)
	{
		i.reset(new DialogItemEx);
		//BUGBUG add error check
		ConvertItemEx(CVTITEM_FROMPLUGIN,const_cast<FarDialogItem *>(SrcItem++), i.get(), 1);
	});

	Init(OwnerClass, HandlerFunction, DlgProc, InitParam);
}

Dialog::Dialog(DialogItemEx *SrcItem, size_t SrcItemCount, StaticHandlerFunction DlgProc, void* InitParam):
	bInitOK(false)
{
	Construct(SrcItem, SrcItemCount, nullptr, nullptr, DlgProc, InitParam);
}

Dialog::Dialog(const FarDialogItem *SrcItem, size_t SrcItemCount, StaticHandlerFunction DlgProc, void* InitParam):
	bInitOK(false)
{
	Construct(SrcItem, SrcItemCount, nullptr, nullptr, DlgProc, InitParam);
}

void Dialog::Init(DialogOwner* Owner, MemberHandlerFunction HandlerFunction, StaticHandlerFunction DlgProc, void* InitParam)
{
	SetDynamicallyBorn(FALSE); // $OT: По умолчанию все диалоги создаются статически
	CanLoseFocus = FALSE;
	//Номер плагина, вызвавшего диалог (-1 = Main)
	PluginOwner = nullptr;
	DataDialog=InitParam;
	DialogMode.Set(DMODE_ISCANMOVE);
	SetDropDownOpened(FALSE);
	IsEnableRedraw=0;
	FocusPos=(size_t)-1;
	PrevFocusPos=(size_t)-1;

	OwnerClass = Owner;
	DialogHandler = OwnerClass? HandlerFunction : nullptr;
	if (!DialogHandler)
	{
		if (!DlgProc) // функция должна быть всегда!!!
		{
			DlgProc=&DefProcFunction;
			// знать диалог в старом стиле - учтем этот факт!
			DialogMode.Set(DMODE_OLDSTYLE);
		}
		RealDlgProc=DlgProc;
	}
	if (Global->CtrlObject)
	{
		// запомним пред. режим макро.
		PrevMacroMode=Global->CtrlObject->Macro.GetMode();
		// макросить будет в диалогах :-)
		Global->CtrlObject->Macro.SetMode(MACRO_DIALOG);
	}

	//_SVS(SysLog(L"Dialog =%d",Global->CtrlObject->Macro.GetMode()));
	// запоминаем предыдущий заголовок консоли
	OldTitle=new ConsoleTitle;
	IdExist=false;
	ClearStruct(Id);
	bInitOK = true;
}

//////////////////////////////////////////////////////////////////////////
/* Public, Virtual:
   Деструктор класса Dialog
*/
Dialog::~Dialog()
{
	_tran(SysLog(L"[%p] Dialog::~Dialog()",this));
	DeleteDialogObjects();

	if (Global->CtrlObject)
		Global->CtrlObject->Macro.SetMode(PrevMacroMode);

	Hide();
	if (Global->Opt->Clock && FrameManager->IsPanelsActive(true))
		ShowTime(0);

	if(!CheckDialogMode(DMODE_ISMENU))
		Global->ScrBuf->Flush();

//	INPUT_RECORD rec;
//	PeekInputRecord(&rec);
	delete OldTitle;
	_DIALOG(CleverSysLog CL(L"Destroy Dialog"));
}

void Dialog::CheckDialogCoord()
{
	CriticalSectionLock Lock(CS);

	// задано центрирование диалога по горизонтали?
	// X2 при этом = ширине диалога.
	if (X1 == -1)
	{
		X1 = (ScrX - X2 + 1) / 2;
		X2 += X1 - 1;
	}

	// задано центрирование диалога по вертикали?
	// Y2 при этом = высоте диалога.
	if (Y1 == -1)
	{
		Y1 = (ScrY - Y2 + 1) / 2;
		Y2 += Y1 - 1;
	}
}


void Dialog::InitDialog()
{
	CriticalSectionLock Lock(CS);

	if(Global->CloseFAR)
	{
		SetDialogMode(DMODE_NOPLUGINS);
	}

	if (!DialogMode.Check(DMODE_INITOBJECTS))      // самодостаточный вариант, когда
	{                      //  элементы инициализируются при первом вызове.
		CheckDialogCoord();
		size_t InitFocus=InitDialogObjects();
		int Result=(int)DlgProc(DN_INITDIALOG,InitFocus,DataDialog);

		if (ExitCode == -1)
		{
			if (Result)
			{
				// еще разок, т.к. данные могли быть изменены
				InitFocus=InitDialogObjects(); // InitFocus=????
			}

			if (!DialogMode.Check(DMODE_KEEPCONSOLETITLE))
				ConsoleTitle::SetFarTitle(GetDialogTitle());
		}

		// все объекты проинициализированы!
		DialogMode.Set(DMODE_INITOBJECTS);

		DlgProc(DN_GOTFOCUS,InitFocus,0);
	}
}

//////////////////////////////////////////////////////////////////////////
/* Public, Virtual:
   Расчет значений координат окна диалога и вызов функции
   ScreenObjectWithShadow::Show() для вывода диалога на экран.
*/
void Dialog::Show()
{
	CriticalSectionLock Lock(CS);
	_tran(SysLog(L"[%p] Dialog::Show()",this));

	if (!DialogMode.Check(DMODE_INITOBJECTS))
		return;

	if (!Locked() && DialogMode.Check(DMODE_RESIZED) && !Global->PreRedraw->empty())
	{
		const PreRedrawItem& preRedrawItem(Global->PreRedraw->top());

		if (preRedrawItem.PreRedrawFunc)
			preRedrawItem.PreRedrawFunc();
	}

	DialogMode.Clear(DMODE_RESIZED);

	if (Locked())
		return;

	DialogMode.Set(DMODE_SHOW);
	ScreenObjectWithShadow::Show();
}

//  Цель перехвата данной функции - управление видимостью...
void Dialog::Hide()
{
	CriticalSectionLock Lock(CS);
	_tran(SysLog(L"[%p] Dialog::Hide()",this));

	if (!DialogMode.Check(DMODE_INITOBJECTS))
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
	CriticalSectionLock Lock(CS);

	if (DialogMode.Check(DMODE_SHOW))
	{
		ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
		ShowDialog();          // "нарисуем" диалог.
	}
}

// пересчитать координаты для элементов с DIF_CENTERGROUP
void Dialog::ProcessCenterGroup()
{
	CriticalSectionLock Lock(CS);

	FOR_CONST_RANGE(Items, i)
	{
		// Последовательно объявленные элементы с флагом DIF_CENTERGROUP
		// и одинаковой вертикальной позицией будут отцентрированы в диалоге.
		// Их координаты X не важны. Удобно использовать для центрирования
		// групп кнопок.
		if (
			((*i)->Flags & DIF_CENTERGROUP) &&
			(i == Items.begin() || (i != Items.begin() && (!((*(i-1))->Flags & DIF_CENTERGROUP) || (*(i-1))->Y1 != (*i)->Y1)))
		)
		{
			int Length=0;

			for (auto j = i; j != Items.end() && ((*j)->Flags & DIF_CENTERGROUP) && (*j)->Y1 == (*i)->Y1; ++j)
			{
				Length+=LenStrItem(j - Items.begin());

				if (!(*j)->strData.IsEmpty())
					switch ((*j)->Type)
					{
						case DI_BUTTON:
							Length++;
							break;
						case DI_CHECKBOX:
						case DI_RADIOBUTTON:
							Length+=5;
							break;
						default:
							break;
					}
			}

			if (!(*i)->strData.IsEmpty())
			{
				switch ((*i)->Type)
				{
					case DI_BUTTON:
						Length--;
						break;
					case DI_CHECKBOX:
					case DI_RADIOBUTTON:
//            Length-=5;
						break;
					default:
						break;
				} //Бля, це ж ботва какая-то
			}
			int StartX=std::max(0,(X2-X1+1-Length)/2);

			for (auto j = i; j != Items.end() && ((*j)->Flags & DIF_CENTERGROUP) && (*j)->Y1 == (*i)->Y1; ++j)
			{
				(*j)->X1=StartX;
				StartX+=LenStrItem(j - Items.begin());

				if (!(*j)->strData.IsEmpty())
					switch ((*j)->Type)
					{
						case DI_BUTTON:
							StartX++;
							break;
						case DI_CHECKBOX:
						case DI_RADIOBUTTON:
							StartX+=5;
							break;
						default:
							break;
					}

				if (StartX == (*j)->X1)
					(*j)->X2=StartX;
				else
					(*j)->X2=StartX-1;
			}
		}
	}
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
size_t Dialog::InitDialogObjects(size_t ID)
{
	CriticalSectionLock Lock(CS);
	size_t I, J;
	FARDIALOGITEMTYPES Type;
	DialogItemEx *CurItem;
	size_t InitItemCount;
	unsigned __int64 ItemFlags;
	_DIALOG(CleverSysLog CL(L"Init Dialog"));

	if (ID+1 > Items.size())
		return (size_t)-1;

	if (ID == (size_t)-1) // инициализируем все?
	{
		ID=0;
		InitItemCount=Items.size();
	}
	else
	{
		InitItemCount=ID+1;
	}

	//   если FocusPos в пределах и элемент задисаблен, то ищем сначала.
	if (FocusPos!=(size_t)-1 && FocusPos < Items.size() &&
	        (Items[FocusPos]->Flags&(DIF_DISABLE|DIF_NOFOCUS|DIF_HIDDEN)))
		FocusPos = (size_t)-1; // будем искать сначала!

	// предварительный цикл по поводу кнопок
	for (I=ID; I < InitItemCount; I++)
	{
		CurItem = Items[I].get();
		ItemFlags=CurItem->Flags;
		Type=CurItem->Type;

		if (Type==DI_BUTTON && ItemFlags&DIF_SETSHIELD)
		{
			CurItem->strData=string(L"\x2580\x2584 ")+CurItem->strData;
		}

		// для кнопок не имеющи стиля "Показывает заголовок кнопки без скобок"
		//  добавим энти самые скобки
		if (Type==DI_BUTTON && !(ItemFlags & DIF_NOBRACKETS))
		{
			LPCWSTR Brackets[]={L"[ ", L" ]", L"{ ",L" }"};
			int Start=((CurItem->Flags&DIF_DEFAULTBUTTON)?2:0);
			if(CurItem->strData.At(0)!=*Brackets[Start])
			{
				CurItem->strData=Brackets[Start]+CurItem->strData+Brackets[Start+1];
			}
		}
		// предварительный поик фокуса
		if (FocusPos == (size_t)-1 &&
		        CanGetFocus(Type) &&
		        (CurItem->Flags&DIF_FOCUS) &&
		        !(ItemFlags&(DIF_DISABLE|DIF_NOFOCUS|DIF_HIDDEN)))
			FocusPos=I; // запомним первый фокусный элемент

		CurItem->Flags&=~DIF_FOCUS; // сбросим для всех, чтобы не оказалось,
		//   что фокусов - как у дурачка фантиков

		// сбросим флаг DIF_CENTERGROUP для редакторов
		switch (Type)
		{
			case DI_BUTTON:
			case DI_CHECKBOX:
			case DI_RADIOBUTTON:
			case DI_TEXT:
			case DI_VTEXT:  // ????
				break;
			default:

				if (ItemFlags&DIF_CENTERGROUP)
					CurItem->Flags&=~DIF_CENTERGROUP;
		}
	}

	// Опять про фокус ввода - теперь, если "чудо" забыло выставить
	// хотя бы один, то ставим на первый подходящий
	if (FocusPos == (size_t)-1)
	{
		FOR_CONST_RANGE(Items, i) // по всем!!!!
		{
			if (CanGetFocus((*i)->Type) &&
			        !((*i)->Flags&(DIF_DISABLE|DIF_NOFOCUS|DIF_HIDDEN)))
			{
				FocusPos= i - Items.begin();
				break;
			}
		}
	}

	if (FocusPos == (size_t)-1) // ну ни хрена себе - нет ни одного
	{                  //   элемента с возможностью фокуса
		FocusPos=0;     // убится, блин
	}

	// ну вот и добрались до!
	Items[FocusPos]->Flags|=DIF_FOCUS;
	// а теперь все сначала и по полной программе...
	ProcessCenterGroup(); // сначала отцентрируем

	for (I=ID; I < InitItemCount; I++)
	{
		CurItem = Items[I].get();
		Type=CurItem->Type;
		ItemFlags=CurItem->Flags;

		if (Type==DI_LISTBOX)
		{
			if (!DialogMode.Check(DMODE_CREATEOBJECTS))
			{
				CurItem->ListPtr=new VMenu(nullptr,nullptr,0,CurItem->Y2-CurItem->Y1+1,
				                           VMENU_ALWAYSSCROLLBAR|VMENU_LISTBOX,this);
			}

			if (CurItem->ListPtr)
			{
				VMenu *ListPtr=CurItem->ListPtr;
				ListPtr->SetVDialogItemID(I);
				/* $ 13.09.2000 SVS
				   + Флаг DIF_LISTNOAMPERSAND. По умолчанию для DI_LISTBOX &
				     DI_COMBOBOX выставляется флаг MENU_SHOWAMPERSAND. Этот флаг
				     подавляет такое поведение
				*/
				ListPtr->ChangeFlags(VMENU_DISABLED, (ItemFlags&DIF_DISABLE)!=0);
				ListPtr->ChangeFlags(VMENU_SHOWAMPERSAND, (ItemFlags&DIF_LISTNOAMPERSAND)==0);
				ListPtr->ChangeFlags(VMENU_SHOWNOBOX, (ItemFlags&DIF_LISTNOBOX)!=0);
				ListPtr->ChangeFlags(VMENU_WRAPMODE, (ItemFlags&DIF_LISTWRAPMODE)!=0);
				ListPtr->ChangeFlags(VMENU_AUTOHIGHLIGHT, (ItemFlags&DIF_LISTAUTOHIGHLIGHT)!=0);

				if (ItemFlags&DIF_LISTAUTOHIGHLIGHT)
					ListPtr->AssignHighlights(FALSE);

				ListPtr->SetDialogStyle(DialogMode.Check(DMODE_WARNINGSTYLE));
				ListPtr->SetPosition(X1+CurItem->X1,Y1+CurItem->Y1,
				                     X1+CurItem->X2,Y1+CurItem->Y2);
				ListPtr->SetBoxType(SHORT_SINGLE_BOX);

				// поле FarDialogItem.Data для DI_LISTBOX используется как верхний заголовок листа
				if (!(ItemFlags&DIF_LISTNOBOX) && !DialogMode.Check(DMODE_CREATEOBJECTS))
				{
					ListPtr->SetTitle(CurItem->strData);
				}

				// удалим все итемы
				//ListBox->DeleteItems(); //???? А НАДО ЛИ ????
				if (CurItem->ListItems && !DialogMode.Check(DMODE_CREATEOBJECTS))
				{
					ListPtr->AddItem(CurItem->ListItems);
				}

				ListPtr->ChangeFlags(VMENU_LISTHASFOCUS, (CurItem->Flags&DIF_FOCUS)!=0);
			}
		}
		// "редакторы" - разговор особый...
		else if (IsEdit(Type))
		{
			// сбросим флаг DIF_EDITOR для строки ввода, отличной от DI_EDIT,
			// DI_FIXEDIT и DI_PSWEDIT
			if (Type != DI_COMBOBOX)
				if ((ItemFlags&DIF_EDITOR) && Type != DI_EDIT && Type != DI_FIXEDIT && Type != DI_PSWEDIT)
					ItemFlags&=~DIF_EDITOR;

			if (!DialogMode.Check(DMODE_CREATEOBJECTS))
			{
				CurItem->ObjPtr=new DlgEdit(this,I,Type == DI_MEMOEDIT?DLGEDIT_MULTILINE:DLGEDIT_SINGLELINE);

				if (Type == DI_COMBOBOX)
				{
					CurItem->ListPtr=new VMenu(L"",nullptr,0,Global->Opt->Dialogs.CBoxMaxHeight,VMENU_ALWAYSSCROLLBAR|VMENU_NOTCHANGE,this);
					CurItem->ListPtr->SetVDialogItemID(I);
				}

				CurItem->SelStart=-1;
			}

			DlgEdit *DialogEdit=(DlgEdit *)CurItem->ObjPtr;
			// Mantis#58 - символ-маска с кодом 0х0А - пропадает
			//DialogEdit->SetDialogParent((Type != DI_COMBOBOX && (ItemFlags & DIF_EDITOR) || (CurItem->Type==DI_PSWEDIT || CurItem->Type==DI_FIXEDIT))?
			//                            FEDITLINE_PARENT_SINGLELINE:FEDITLINE_PARENT_MULTILINE);
			DialogEdit->SetDialogParent(Type == DI_MEMOEDIT?FEDITLINE_PARENT_MULTILINE:FEDITLINE_PARENT_SINGLELINE);
			DialogEdit->SetReadOnly(0);

			if (Type == DI_COMBOBOX)
			{
				if (CurItem->ListPtr)
				{
					VMenu *ListPtr=CurItem->ListPtr;
					ListPtr->SetBoxType(SHORT_SINGLE_BOX);
					DialogEdit->SetDropDownBox((ItemFlags & DIF_DROPDOWNLIST)!=0);
					ListPtr->ChangeFlags(VMENU_WRAPMODE, (ItemFlags&DIF_LISTWRAPMODE)!=0);
					ListPtr->ChangeFlags(VMENU_DISABLED, (ItemFlags&DIF_DISABLE)!=0);
					ListPtr->ChangeFlags(VMENU_SHOWAMPERSAND, (ItemFlags&DIF_LISTNOAMPERSAND)==0);
					ListPtr->ChangeFlags(VMENU_AUTOHIGHLIGHT, (ItemFlags&DIF_LISTAUTOHIGHLIGHT)!=0);

					if (ItemFlags&DIF_LISTAUTOHIGHLIGHT)
						ListPtr->AssignHighlights(FALSE);

					if (CurItem->ListItems && !DialogMode.Check(DMODE_CREATEOBJECTS))
						ListPtr->AddItem(CurItem->ListItems);

					ListPtr->SetFlags(VMENU_COMBOBOX);
					ListPtr->SetDialogStyle(DialogMode.Check(DMODE_WARNINGSTYLE));
				}
			}

			/* $ 15.10.2000 tran
			  строка редакторирование должна иметь максимум в 511 символов */
			// выставляем максимальный размер в том случае, если он еще не выставлен

			//BUGBUG
			if (DialogEdit->GetMaxLength() == -1)
				DialogEdit->SetMaxLength(CurItem->MaxLength?(int)CurItem->MaxLength:-1);

			DialogEdit->SetPosition(X1+CurItem->X1,Y1+CurItem->Y1,
			                        X1+CurItem->X2,Y1+CurItem->Y2);

//      DialogEdit->SetObjectColor(
//         FarColorToReal(DialogMode.Check(DMODE_WARNINGSTYLE) ?
//             ((ItemFlags&DIF_DISABLE)?COL_WARNDIALOGEDITDISABLED:COL_WARNDIALOGEDIT):
//             ((ItemFlags&DIF_DISABLE)?COL_DIALOGEDITDISABLED:COL_DIALOGEDIT)),
//         FarColorToReal((ItemFlags&DIF_DISABLE)?COL_DIALOGEDITDISABLED:COL_DIALOGEDITSELECTED));
			if (CurItem->Type==DI_PSWEDIT)
			{
				DialogEdit->SetPasswordMode(true);
				// ...Что бы небыло повадно... и для повыщения защиты, т.с.
				ItemFlags&=~DIF_HISTORY;
			}

			if (Type==DI_FIXEDIT)
			{
				//   DIF_HISTORY имеет более высокий приоритет, чем DIF_MASKEDIT
				if (ItemFlags&DIF_HISTORY)
					ItemFlags&=~DIF_MASKEDIT;

				// если DI_FIXEDIT, то курсор сразу ставится на замену...
				//   ай-ай - было недокументированно :-)
				DialogEdit->SetMaxLength(CurItem->X2-CurItem->X1+1+(CurItem->X2==CurItem->X1 || !(ItemFlags&DIF_HISTORY)?0:1));
				DialogEdit->SetOvertypeMode(TRUE);
				/* $ 12.08.2000 KM
				   Если тип строки ввода DI_FIXEDIT и установлен флаг DIF_MASKEDIT
				   и непустой параметр CurItem->Mask, то вызываем новую функцию
				   для установки маски в объект DlgEdit.
				*/

				//  Маска не должна быть пустой (строка из пробелов не учитывается)!
				if ((ItemFlags & DIF_MASKEDIT) && !CurItem->strMask.IsEmpty())
				{
					RemoveExternalSpaces(CurItem->strMask);
					if(!CurItem->strMask.IsEmpty())
					{
						DialogEdit->SetInputMask(CurItem->strMask);
					}
					else
					{
						ItemFlags&=~DIF_MASKEDIT;
					}
				}
			}
			else

				// "мини-редактор"
				// Последовательно определенные поля ввода (edit controls),
				// имеющие этот флаг группируются в редактор с возможностью
				// вставки и удаления строк
				if (!(ItemFlags & DIF_EDITOR) && CurItem->Type != DI_COMBOBOX)
				{
					DialogEdit->SetEditBeyondEnd(FALSE);

					if (!DialogMode.Check(DMODE_INITOBJECTS))
						DialogEdit->SetClearFlag(1);
				}

			if (CurItem->Type == DI_COMBOBOX)
				DialogEdit->SetClearFlag(1);

			/* $ 01.08.2000 SVS
			   Еже ли стоит флаг DIF_USELASTHISTORY и непустая строка ввода,
			   то подстанавливаем первое значение из History
			*/
			if (CurItem->Type==DI_EDIT &&
			        (ItemFlags&(DIF_HISTORY|DIF_USELASTHISTORY)) == (DIF_HISTORY|DIF_USELASTHISTORY))
			{
				ProcessLastHistory(CurItem, -1);
			}

			if ((ItemFlags&DIF_MANUALADDHISTORY) && !(ItemFlags&DIF_HISTORY))
				ItemFlags&=~DIF_MANUALADDHISTORY; // сбросим нафиг.

			/* $ 18.03.2000 SVS
			   Если это ComBoBox и данные не установлены, то берем из списка
			   при условии, что хоть один из пунктов имеет Selected
			*/

			if (Type==DI_COMBOBOX && CurItem->strData.IsEmpty() && CurItem->ListItems)
			{
				FarListItem *ListItems=CurItem->ListItems->Items;
				size_t Length=CurItem->ListItems->ItemsNumber;
				//CurItem->ListPtr->AddItem(CurItem->ListItems);

				for (J=0; J < Length; J++)
				{
					if (ListItems[J].Flags & LIF_SELECTED)
					{
						if (ItemFlags & (DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND))
							HiText2Str(CurItem->strData, ListItems[J].Text);
						else
							CurItem->strData = ListItems[J].Text;

						break;
					}
				}
			}

			DialogEdit->SetCallbackState(false);
			DialogEdit->SetString(CurItem->strData);
			DialogEdit->SetCallbackState(true);

			if (Type==DI_FIXEDIT)
				DialogEdit->SetCurPos(0);

			// Для обычных строк отрубим постоянные блоки
			if (!(ItemFlags&DIF_EDITOR))
				DialogEdit->SetPersistentBlocks(Global->Opt->Dialogs.EditBlock);

			DialogEdit->SetDelRemovesBlocks(Global->Opt->Dialogs.DelRemovesBlocks);

			if (ItemFlags&DIF_READONLY)
				DialogEdit->SetReadOnly(1);
		}
		else if (Type == DI_USERCONTROL)
		{
			if (!DialogMode.Check(DMODE_CREATEOBJECTS))
				CurItem->UCData=new DlgUserControl;
		}
		else if (Type == DI_TEXT)
		{
			if (CurItem->X1 == -1 && (ItemFlags & (DIF_SEPARATOR|DIF_SEPARATOR2)))
				ItemFlags |= DIF_CENTERTEXT;
		}
		else if (Type == DI_VTEXT)
		{
			if (CurItem->Y1 == -1 && (ItemFlags & (DIF_SEPARATOR|DIF_SEPARATOR2)))
				ItemFlags |= DIF_CENTERTEXT;
		}

		CurItem->Flags=ItemFlags;
	}

	// если будет редактор, то обязательно будет выделен.
	SelectOnEntry(FocusPos,TRUE);
	// все объекты созданы!
	DialogMode.Set(DMODE_CREATEOBJECTS);
	return FocusPos;
}


const wchar_t *Dialog::GetDialogTitle()
{
	CriticalSectionLock Lock(CS);
	DialogItemEx *CurItem, *CurItemList=nullptr;

	FOR_CONST_RANGE(Items, i)
	{
		CurItem = i->get();

		// по первому попавшемуся "тексту" установим заголовок консоли!
		if ((CurItem->Type==DI_TEXT ||
		        CurItem->Type==DI_DOUBLEBOX ||
		        CurItem->Type==DI_SINGLEBOX))
		{
			const wchar_t *Ptr = CurItem->strData;

			for (; *Ptr; Ptr++)
				if (IsAlpha(*Ptr) || iswdigit(*Ptr))
					return(Ptr);
		}
		else if (CurItem->Type==DI_LISTBOX && i == Items.begin())
			CurItemList=CurItem;
	}

	if (CurItemList)
	{
		return CurItemList->ListPtr->GetPtrTitle();
	}

	return nullptr; //""
}

void Dialog::ProcessLastHistory(DialogItemEx *CurItem, int MsgIndex)
{
	CriticalSectionLock Lock(CS);
	string &strData = CurItem->strData;

	if (strData.IsEmpty())
	{
		DlgEdit *EditPtr;

		if ((EditPtr = (DlgEdit *)(CurItem->ObjPtr)) )
		{
			History *DlgHistory = EditPtr->GetHistory();
			if(DlgHistory)
			{
				DlgHistory->ReadLastItem(CurItem->strHistory, strData);
			}

			if (MsgIndex != -1)
			{
				// обработка DM_SETHISTORY => надо пропустить изменение текста через
				// диалоговую функцию
				FarDialogItemData IData={sizeof(FarDialogItemData)};
				IData.PtrData=const_cast<wchar_t*>(strData.CPtr());
				IData.PtrLength=strData.GetLength();
				SendMessage(DM_SETTEXT,MsgIndex,&IData);
			}
		}
	}
}


//   Изменение координат и/или размеров итема диалога.
BOOL Dialog::SetItemRect(size_t ID,SMALL_RECT *Rect)
{
	CriticalSectionLock Lock(CS);

	if (ID >= Items.size())
		return FALSE;

	DialogItemEx *CurItem=Items[ID].get();
	FARDIALOGITEMTYPES Type=CurItem->Type;
	CurItem->X1=Rect->Left;
	CurItem->Y1=(Rect->Top<0)?0:Rect->Top;

	if (IsEdit(Type))
	{
		DlgEdit *DialogEdit=(DlgEdit *)CurItem->ObjPtr;
		CurItem->X2=Rect->Right;
		CurItem->Y2=(Type == DI_MEMOEDIT?Rect->Bottom:0);
		DialogEdit->SetPosition(X1+Rect->Left, Y1+Rect->Top,
		                        X1+Rect->Right,Y1+Rect->Top);
	}
	else if (Type==DI_LISTBOX)
	{
		CurItem->X2=Rect->Right;
		CurItem->Y2=Rect->Bottom;
		CurItem->ListPtr->SetPosition(X1+Rect->Left, Y1+Rect->Top,
		                              X1+Rect->Right,Y1+Rect->Bottom);
		CurItem->ListPtr->SetMaxHeight(CurItem->Y2-CurItem->Y1+1);
	}

	switch (Type)
	{
		case DI_TEXT:
			CurItem->X2=Rect->Right;
			CurItem->Y2=(CurItem->Flags & DIF_WORDWRAP)?Rect->Bottom:0;
			break;
		case DI_VTEXT:
			CurItem->X2=0;                    // ???
			CurItem->Y2=Rect->Bottom;
		case DI_DOUBLEBOX:
		case DI_SINGLEBOX:
		case DI_USERCONTROL:
			CurItem->X2=Rect->Right;
			CurItem->Y2=Rect->Bottom;
			break;
		default:
			break;
	}

	if (DialogMode.Check(DMODE_SHOW))
	{
		ShowDialog((size_t)-1);
		Global->ScrBuf->Flush();
	}

	return TRUE;
}

BOOL Dialog::GetItemRect(size_t I,SMALL_RECT& Rect)
{
	CriticalSectionLock Lock(CS);

	if (I >= Items.size())
		return FALSE;

	DialogItemEx *CurItem=Items[I].get();
	unsigned __int64 ItemFlags=CurItem->Flags;
	int Type=CurItem->Type;
	int Len=0;
	Rect.Left=CurItem->X1;
	Rect.Top=CurItem->Y1;
	Rect.Right=CurItem->X2;
	Rect.Bottom=CurItem->Y2;

	switch (Type)
	{
		case DI_COMBOBOX:
		case DI_EDIT:
		case DI_FIXEDIT:
		case DI_PSWEDIT:
		case DI_LISTBOX:
		case DI_MEMOEDIT:
			break;
		default:
			Len=((ItemFlags & DIF_SHOWAMPERSAND)?(int)CurItem->strData.GetLength():HiStrlen(CurItem->strData));
			break;
	}

	switch (Type)
	{
		case DI_TEXT:

			if (CurItem->X1==-1)
				Rect.Left=(X2-X1+1-Len)/2;

			if (Rect.Left < 0)
				Rect.Left=0;

			if (CurItem->Y1==-1)
				Rect.Top=(Y2-Y1+1)/2;

			if (Rect.Top < 0)
				Rect.Top=0;

			if (!(ItemFlags & DIF_WORDWRAP))
				Rect.Bottom=Rect.Top;

			if (!Rect.Right || Rect.Right == Rect.Left)
				Rect.Right=Rect.Left+Len-(Len?1:0);

			if (ItemFlags & (DIF_SEPARATOR|DIF_SEPARATOR2))
			{
				Rect.Bottom=Rect.Top;
				Rect.Left=(!DialogMode.Check(DMODE_SMALLDIALOG)?3:0); //???
				Rect.Right=X2-X1-(!DialogMode.Check(DMODE_SMALLDIALOG)?5:0); //???
			}

			break;
		case DI_VTEXT:

			if (CurItem->X1==-1)
				Rect.Left=(X2-X1+1)/2;

			if (Rect.Left < 0)
				Rect.Left=0;

			if (CurItem->Y1==-1)
				Rect.Top=(Y2-Y1+1-Len)/2;

			if (Rect.Top < 0)
				Rect.Top=0;

			Rect.Right=Rect.Left;

			//Rect.bottom=Rect.top+Len;
			if (!Rect.Bottom || Rect.Bottom == Rect.Top)
				Rect.Bottom=Rect.Top+Len-(Len?1:0);

#if defined(VTEXT_ADN_SEPARATORS)

			if (ItemFlags & (DIF_SEPARATOR|DIF_SEPARATOR2))
			{
				Rect.Right=Rect.Left;
				Rect.Top=(!DialogMode.Check(DMODE_SMALLDIALOG)?1:0); //???
				Rect.Bottom=Y2-Y1-(!DialogMode.Check(DMODE_SMALLDIALOG)?3:0); //???
				break;
			}

#endif
			break;
		case DI_BUTTON:
			Rect.Bottom=Rect.Top;
			Rect.Right=Rect.Left+Len;
			break;
		case DI_CHECKBOX:
		case DI_RADIOBUTTON:
			Rect.Bottom=Rect.Top;
			Rect.Right=Rect.Left+Len+((Type == DI_CHECKBOX)?4:
			                          (ItemFlags & DIF_MOVESELECT?3:4)
			                         );
			break;
		case DI_COMBOBOX:
		case DI_EDIT:
		case DI_FIXEDIT:
		case DI_PSWEDIT:
			Rect.Bottom=Rect.Top;
			break;
	}

	return TRUE;
}

bool Dialog::ItemHasDropDownArrow(const DialogItemEx *Item) const
{
	return ((!Item->strHistory.IsEmpty() && (Item->Flags & DIF_HISTORY) && Global->Opt->Dialogs.EditHistory) ||
		(Item->Type == DI_COMBOBOX && Item->ListPtr && Item->ListPtr->GetItemCount() > 0));
}

//////////////////////////////////////////////////////////////////////////
/* Private:
   Получение данных и удаление "редакторов"
*/
void Dialog::DeleteDialogObjects()
{
	CriticalSectionLock Lock(CS);

	std::for_each(CONST_RANGE(Items, i)
	{
		switch (i->Type)
		{
			case DI_EDIT:
			case DI_FIXEDIT:
			case DI_PSWEDIT:
			case DI_COMBOBOX:
			case DI_MEMOEDIT:

				if (i->ObjPtr)
					delete(DlgEdit *)(i->ObjPtr);

			case DI_LISTBOX:

				if ((i->Type == DI_COMBOBOX || i->Type == DI_LISTBOX) && i->ListPtr)
					delete i->ListPtr;

				break;
			case DI_USERCONTROL:

				if (i->UCData)
					delete i->UCData;

				break;

			default:
				break;
		}

		if (i->Flags&DIF_AUTOMATION)
			i->Auto.clear();
	});
}


//////////////////////////////////////////////////////////////////////////
/* Public:
   Сохраняет значение из полей редактирования.
   При установленном флаге DIF_HISTORY, сохраняет данные в реестре.
*/
void Dialog::GetDialogObjectsData()
{
	CriticalSectionLock Lock(CS);

	std::for_each(CONST_RANGE(Items, i)
	{
		FARDIALOGITEMFLAGS IFlags = i->Flags;

		switch (i->Type)
		{
			case DI_MEMOEDIT:
				break; //????
			case DI_EDIT:
			case DI_FIXEDIT:
			case DI_PSWEDIT:
			case DI_COMBOBOX:
			{
				if (i->ObjPtr)
				{
					string strData;
					DlgEdit *EditPtr=(DlgEdit *)(i->ObjPtr);

					// подготовим данные
					// получим данные
					EditPtr->GetString(strData);

					if (ExitCode >=0 &&
					        (IFlags & DIF_HISTORY) &&
					        !(IFlags & DIF_MANUALADDHISTORY) && // при мануале не добавляем
							!i->strHistory.IsEmpty() &&
					        Global->Opt->Dialogs.EditHistory)
					{
						AddToEditHistory(i.get(), strData);
					}

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

					if ((IFlags&DIF_EDITEXPAND) && i->Type != DI_PSWEDIT && i->Type != DI_FIXEDIT)
					{
						apiExpandEnvironmentStrings(strData, strData);
						//как бы грязный хак, нам нужно обновить строку чтоб отдавалась правильная строка
						//для различных DM_* после закрытия диалога, но ни в коем случае нельзя чтоб
						//высылался DN_EDITCHANGE для этого изменения, ибо диалог уже закрыт.
						EditPtr->SetCallbackState(false);
						EditPtr->SetString(strData);
						EditPtr->SetCallbackState(true);

					}

					i->strData = strData;
				}

				break;
			}
			case DI_LISTBOX:
				/*
				  if(i->ListPtr)
				  {
				    i->ListPos=CurItem->ListPtr->GetSelectPos();
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

		if ((i->Type == DI_COMBOBOX || i->Type == DI_LISTBOX))
		{
			i->ListPos = i->ListPtr? i->ListPtr->GetSelectPos() : 0;
		}

#endif
	});
}

// Функция формирования и запроса цветов.
intptr_t Dialog::CtlColorDlgItem(FarColor Color[4], size_t ItemPos, FARDIALOGITEMTYPES Type, bool Focus, bool Default,FARDIALOGITEMFLAGS Flags)
{
	CriticalSectionLock Lock(CS);
	BOOL DisabledItem=Flags&DIF_DISABLE?TRUE:FALSE;

	switch (Type)
	{
		case DI_SINGLEBOX:
		case DI_DOUBLEBOX:
		{
			// Title
			Color[0] = ColorIndexToColor(DialogMode.Check(DMODE_WARNINGSTYLE)? (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBOXTITLE) : (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGBOXTITLE));
			// HiText
			Color[1] = ColorIndexToColor(DialogMode.Check(DMODE_WARNINGSTYLE)? (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGHIGHLIGHTBOXTITLE) : (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGHIGHLIGHTBOXTITLE));
			// Box
			Color[2] = ColorIndexToColor(DialogMode.Check(DMODE_WARNINGSTYLE)? (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBOX) : (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGBOX));
			break;
		}

#if defined(VTEXT_ADN_SEPARATORS)
		case DI_VTEXT:
#endif
		case DI_TEXT:
		{
			Color[0] = ColorIndexToColor((Flags & DIF_BOXCOLOR)? (DialogMode.Check(DMODE_WARNINGSTYLE)? (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBOX) : (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGBOX)) : (DialogMode.Check(DMODE_WARNINGSTYLE)? (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGTEXT) : (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGTEXT)));
			// HiText
			Color[1] = ColorIndexToColor(DialogMode.Check(DMODE_WARNINGSTYLE)? (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGHIGHLIGHTTEXT) : (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGHIGHLIGHTTEXT));
			if (Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2))
			{
				// Box
				Color[2] = ColorIndexToColor(DialogMode.Check(DMODE_WARNINGSTYLE)? (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBOX) : (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGBOX));
			}
			break;
		}

#if !defined(VTEXT_ADN_SEPARATORS)
		case DI_VTEXT:
		{
			if (Flags & DIF_BOXCOLOR)
				Attr=DialogMode.Check(DMODE_WARNINGSTYLE) ?
				     (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBOX):
						     (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGBOX);
			else if (Flags & DIF_SETCOLOR)
				Attr=(Flags & DIF_COLORMASK);
			else
				Attr=(DialogMode.Check(DMODE_WARNINGSTYLE) ?
				      (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGTEXT):
						      (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGTEXT));

			Attr=MAKEWORD(MAKEWORD(ColorIndexToColor(Attr),0),MAKEWORD(0,0));
			break;
		}
#endif

		case DI_CHECKBOX:
		case DI_RADIOBUTTON:
		{
			Color[0] = ColorIndexToColor(DialogMode.Check(DMODE_WARNINGSTYLE)? (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGTEXT) : (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGTEXT));
			// HiText
			Color[1] = ColorIndexToColor(DialogMode.Check(DMODE_WARNINGSTYLE)? (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGHIGHLIGHTTEXT) : (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGHIGHLIGHTTEXT));
			break;
		}

		case DI_BUTTON:
		{
			if (Focus)
			{
				SetCursorType(0,10);
				// TEXT
				Color[0] = ColorIndexToColor(DialogMode.Check(DMODE_WARNINGSTYLE)? (DisabledItem?COL_WARNDIALOGDISABLED:(Default?COL_WARNDIALOGSELECTEDDEFAULTBUTTON:COL_WARNDIALOGSELECTEDBUTTON)) : (DisabledItem?COL_DIALOGDISABLED:(Default?COL_DIALOGSELECTEDDEFAULTBUTTON:COL_DIALOGSELECTEDBUTTON)));
				// HiText
				Color[1] = ColorIndexToColor(DialogMode.Check(DMODE_WARNINGSTYLE)? (DisabledItem?COL_WARNDIALOGDISABLED:(Default?COL_WARNDIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON:COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON)) : (DisabledItem?COL_DIALOGDISABLED:(Default?COL_DIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON:COL_DIALOGHIGHLIGHTSELECTEDBUTTON)));
			}
			else
			{
				// TEXT
				Color[0] = ColorIndexToColor(DialogMode.Check(DMODE_WARNINGSTYLE)?
						(DisabledItem?COL_WARNDIALOGDISABLED:(Default?COL_WARNDIALOGDEFAULTBUTTON:COL_WARNDIALOGBUTTON)):
						(DisabledItem?COL_DIALOGDISABLED:(Default?COL_DIALOGDEFAULTBUTTON:COL_DIALOGBUTTON)));
				// HiText
				Color[1] = ColorIndexToColor(DialogMode.Check(DMODE_WARNINGSTYLE)? (DisabledItem?COL_WARNDIALOGDISABLED:(Default?COL_WARNDIALOGHIGHLIGHTDEFAULTBUTTON:COL_WARNDIALOGHIGHLIGHTBUTTON)) : (DisabledItem?COL_DIALOGDISABLED:(Default?COL_DIALOGHIGHLIGHTDEFAULTBUTTON:COL_DIALOGHIGHLIGHTBUTTON)));
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
				if (DialogMode.Check(DMODE_WARNINGSTYLE))
				{
					// Text
					Color[0] = ColorIndexToColor(DisabledItem?COL_WARNDIALOGEDITDISABLED:COL_WARNDIALOGEDIT);
					// Select
					Color[1] = ColorIndexToColor(DisabledItem?COL_DIALOGEDITDISABLED:COL_DIALOGEDITSELECTED);
					// Unchanged
					Color[2] = ColorIndexToColor(DisabledItem?COL_WARNDIALOGEDITDISABLED:COL_DIALOGEDITUNCHANGED); //???
					// History
					Color[3] = ColorIndexToColor(DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGTEXT);
				}
				else
				{
					// Text
					Color[0] = ColorIndexToColor(DisabledItem?COL_DIALOGEDITDISABLED:(!Focus?COL_DIALOGEDIT:COL_DIALOGEDITSELECTED));
					// Select
					Color[1] = ColorIndexToColor(DisabledItem?COL_DIALOGEDITDISABLED:(!Focus?COL_DIALOGEDIT:COL_DIALOGEDITSELECTED));
					// Unchanged
					Color[2] = ColorIndexToColor(DisabledItem?COL_DIALOGEDITDISABLED:COL_DIALOGEDITUNCHANGED); //???
					// History
					Color[3] = ColorIndexToColor(DisabledItem?COL_DIALOGDISABLED:COL_DIALOGTEXT);
				}
			}
			else
			{
				if (DialogMode.Check(DMODE_WARNINGSTYLE))
				{
					// Text
					Color[0] = ColorIndexToColor(DisabledItem?COL_WARNDIALOGEDITDISABLED:(Flags&DIF_NOFOCUS?COL_DIALOGEDITUNCHANGED:COL_WARNDIALOGEDIT));
					// Select
					Color[1] = ColorIndexToColor(DisabledItem?COL_DIALOGEDITDISABLED:COL_DIALOGEDITSELECTED);
					// Unchanged
					Color[2] = ColorIndexToColor(DisabledItem?COL_WARNDIALOGEDITDISABLED:COL_DIALOGEDITUNCHANGED);
					// History
					Color[3] = ColorIndexToColor(DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGTEXT);
				}
				else
				{
					// Text
					Color[0] = ColorIndexToColor(DisabledItem?COL_DIALOGEDITDISABLED:(Flags&DIF_NOFOCUS?COL_DIALOGEDITUNCHANGED:COL_DIALOGEDIT));
					// Select
					Color[1] = ColorIndexToColor(DisabledItem?COL_DIALOGEDITDISABLED:COL_DIALOGEDITSELECTED);
					// Unchanged
					Color[2] = ColorIndexToColor(DisabledItem?COL_DIALOGEDITDISABLED:COL_DIALOGEDITUNCHANGED), //???;
					// History
					Color[3] = ColorIndexToColor(DisabledItem?COL_DIALOGDISABLED:COL_DIALOGTEXT);
				}
			}
			break;
		}

		case DI_LISTBOX:
		{
			Items[ItemPos]->ListPtr->SetColors(nullptr);
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
	CriticalSectionLock Lock(CS);

	if (Locked())
		return;

	string strStr;
	wchar_t *lpwszStr;
	DialogItemEx *CurItem;
	int X,Y;
	size_t I,DrawItemCount;
	FarColor ItemColor[4] = {};

	//   Если не разрешена отрисовка, то вываливаем.
	if (IsEnableRedraw ||                // разрешена прорисовка ?
	        (ID+1 > Items.size()) ||             // а номер в рамках дозволенного?
	        DialogMode.Check(DMODE_DRAWING) || // диалог рисуется?
	        !DialogMode.Check(DMODE_SHOW) ||   // если не видим, то и не отрисовываем.
	        !DialogMode.Check(DMODE_INITOBJECTS))
		return;

	DialogMode.Set(DMODE_DRAWING);  // диалог рисуется!!!
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

	if (ID == (size_t)-1) // рисуем все?
	{
		//   Перед прорисовкой диалога посылаем сообщение в обработчик
		if (!DlgProc(DN_DRAWDIALOG,0,0))
		{
			DialogMode.Clear(DMODE_DRAWING);  // конец отрисовки диалога!!!
			return;
		}

		//   перед прорисовкой подложки окна диалога...
		if (!DialogMode.Check(DMODE_NODRAWSHADOW))
			Shadow(DialogMode.Check(DMODE_FULLSHADOW)!=FALSE);              // "наводим" тень

		if (!DialogMode.Check(DMODE_NODRAWPANEL))
		{
			FarColor Color = ColorIndexToColor(DialogMode.Check(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGTEXT:COL_DIALOGTEXT);
			DlgProc(DN_CTLCOLORDIALOG, 0, &Color);
			SetScreen(X1, Y1, X2, Y2, L' ', Color);
		}

		ID=0;
		DrawItemCount = Items.size();
	}
	else
	{
		DrawItemCount=ID+1;
	}

	//IFlags.Set(DIMODE_REDRAW)
	/* TODO:
	   если рисуется контрол и по Z-order`у он пересекается с
	   другим контролом (по координатам), то для "позднего"
	   контрола тоже нужна прорисовка.
	*/
	{
		bool CursorVisible=false;
		DWORD CursorSize=0;

		if (ID != (size_t)-1 && FocusPos != ID)
		{
			if (Items[FocusPos]->Type == DI_USERCONTROL && Items[FocusPos]->UCData->CursorPos.X != -1 && Items[FocusPos]->UCData->CursorPos.Y != -1)
			{
				CursorVisible=Items[FocusPos]->UCData->CursorVisible;
				CursorSize=Items[FocusPos]->UCData->CursorSize;
			}
		}

		SetCursorType(CursorVisible,CursorSize);
	}

	for (I=ID; I < DrawItemCount; I++)
	{
		CurItem = Items[I].get();

		if (CurItem->Flags&DIF_HIDDEN)
			continue;

		/* $ 28.07.2000 SVS
		   Перед прорисовкой каждого элемента посылаем сообщение
		   посредством функции SendDlgMessage - в ней делается все!
		*/
		if (!SendMessage(DN_DRAWDLGITEM,I,0))
			continue;

		int LenText;
		short CX1=CurItem->X1;
		short CY1=CurItem->Y1;
		short CX2=CurItem->X2;
		short CY2=CurItem->Y2;

		if (CX2 > X2-X1)
			CX2 = X2-X1;

		if (CY2 > Y2-Y1)
			CY2 = Y2-Y1;

		short CW=CX2-CX1+1;
		short CH=CY2-CY1+1;
		CtlColorDlgItem(ItemColor, I,CurItem->Type,(CurItem->Flags&DIF_FOCUS)?true:false,(CurItem->Flags&DIF_DEFAULTBUTTON)?true:false,CurItem->Flags);
#if 0

		// TODO: прежде чем эту строку применять... нужно проверить _ВСЕ_ диалоги на предмет X2, Y2. !!!
		if (((CX1 > -1) && (CX2 > 0) && (CX2 > CX1)) &&
		        ((CY1 > -1) && (CY2 > 0) && (CY2 > CY1)))
			SetScreen(X1+CX1,Y1+CY1,X1+CX2,Y1+CY2,' ',Attr&0xFF);

#endif

		switch (CurItem->Type)
		{
				/* ***************************************************************** */
			case DI_SINGLEBOX:
			case DI_DOUBLEBOX:
			{
				BOOL IsDrawTitle=TRUE;
				GotoXY(X1+CX1,Y1+CY1);
				SetColor(ItemColor[2]);

				if (CY1 == CY2)
				{
					DrawLine(CX2-CX1+1,CurItem->Type==DI_SINGLEBOX?8:9); //???
				}
				else if (CX1 == CX2)
				{
					DrawLine(CY2-CY1+1,CurItem->Type==DI_SINGLEBOX?10:11);
					IsDrawTitle=FALSE;
				}
				else
				{
					Box(X1+CX1,Y1+CY1,X1+CX2,Y1+CY2,
					    ItemColor[2],
					    (CurItem->Type==DI_SINGLEBOX) ? SINGLE_BOX:DOUBLE_BOX);
				}

				if (!CurItem->strData.IsEmpty() && IsDrawTitle)
				{
					//  ! Пусть диалог сам заботится о ширине собственного заголовка.
					strStr = CurItem->strData;
					TruncStrFromEnd(strStr,CW-2); // 5 ???
					LenText=LenStrItem(I,strStr);

					if (LenText < CW-2)
					{
						int iLen = (int)strStr.GetLength();
						lpwszStr = strStr.GetBuffer(iLen + 3);
						{
							wmemmove(lpwszStr+1, lpwszStr, iLen);
							*lpwszStr = lpwszStr[++iLen] = L' ';
						}
						strStr.ReleaseBuffer(iLen+1);
						LenText=LenStrItem(I, strStr);
					}

					X=X1+CX1+(CW-LenText)/2;

					if ((CurItem->Flags & DIF_LEFTTEXT) && X1+CX1+1 < X)
						X=X1+CX1+1;
					else if (CurItem->Flags & DIF_RIGHTTEXT)
						X=X1+CX1+(CW-LenText)-1; //2

					SetColor(ItemColor[0]);
					GotoXY(X,Y1+CY1);

					if (CurItem->Flags & DIF_SHOWAMPERSAND)
						Text(strStr);
					else
						HiText(strStr,ItemColor[1]);
				}

				break;
			}
			/* ***************************************************************** */
			case DI_TEXT:
			{
				strStr = CurItem->strData;

				if (!(CurItem->Flags & DIF_WORDWRAP))
				{
					LenText=LenStrItem(I,strStr);

					if (!(CurItem->Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2)) && (CurItem->Flags & DIF_CENTERTEXT) && CX1!=-1)
						LenText=LenStrItem(I,CenterStr(strStr,strStr,CX2-CX1+1));

					if ((CX2 <= 0) || (CX2 < CX1))
						CW = LenText;

					X=(CX1==-1 || (CurItem->Flags & DIF_CENTERTEXT))?(X2-X1+1-LenText)/2:CX1;
					Y=(CY1==-1)?(Y2-Y1+1)/2:CY1;

					if( (CurItem->Flags & DIF_RIGHTTEXT) && CX2 > CX1 )
						X=CX2-LenText+1;

					if (X < 0)
						X=0;

					if (X1+X+LenText > X2)
					{
						int tmpCW=ObjWidth();

						if (CW < ObjWidth())
							tmpCW=CW+1;

						strStr.SetLength(tmpCW-1);
					}

					if (CX1 > -1 && CX2 > CX1 && !(CurItem->Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2))) //половинчатое решение
					{
						SetScreen(X1+CX1,Y1+Y,X1+CX2,Y1+Y,L' ',ItemColor[0]);
						/*
						int CntChr=CX2-CX1+1;
						SetColor(ItemColor[0]);
						GotoXY(X1+X,Y1+Y);

						if (X1+X+CntChr-1 > X2)
							CntChr=X2-(X1+X)+1;

						Global->FS << fmt::MinWidth(CntChr)<<L"";

						if (CntChr < LenText)
							strStr.SetLength(CntChr);
						*/
					}

					if (CurItem->Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2))
					{
						SetColor(ItemColor[2]);
						GotoXY(X1+((CurItem->Flags&DIF_SEPARATORUSER)?X:(!DialogMode.Check(DMODE_SMALLDIALOG)?3:0)),Y1+Y); //????
						ShowUserSeparator((CurItem->Flags&DIF_SEPARATORUSER)?X2-X1+1:RealWidth-(!DialogMode.Check(DMODE_SMALLDIALOG)?6:0/* -1 */),
						                  (CurItem->Flags&DIF_SEPARATORUSER)?12:(CurItem->Flags&DIF_SEPARATOR2?3:1),
					    	              CurItem->strMask
					        	         );
					}

					GotoXY(X1+X,Y1+Y);
					SetColor(ItemColor[0]);

					if (CurItem->Flags & DIF_SHOWAMPERSAND)
						Text(strStr);
					else
						HiText(strStr,ItemColor[1]);
				}
				else
				{
					SetScreen(X1+CX1,Y1+CY1,X1+CX2,Y1+CY2,L' ',ItemColor[0]);

					string strWrap;
					FarFormatText(strStr,CW,strWrap,L"\n",0);
					DWORD CountLine=0;
					bool done=false;

					wchar_t *ptrStr = strWrap.GetBuffer(), *ptrStrPrev=ptrStr;
					while (!done)
					{
						ptrStr=wcschr(ptrStr,L'\n');
						if (!ptrStr)
						{
							ptrStr=ptrStrPrev;
							done=true;
						}
						else
						{
							*ptrStr++=0;
						}

						if (*ptrStr)
						{
							string strResult=ptrStrPrev;
							ptrStrPrev=ptrStr;

							if (CurItem->Flags & DIF_CENTERTEXT)
								CenterStr(strResult,strResult,CW);
							else if (CurItem->Flags & DIF_RIGHTTEXT)
								RightStr(strResult,strResult,CW);

							LenText=LenStrItem(I,strResult);
							X=(CX1==-1 || (CurItem->Flags & DIF_CENTERTEXT))?(CW-LenText)/2:CX1;
							if (X < CX1)
								X=CX1;
							GotoXY(X1+X,Y1+CY1+CountLine);
							SetColor(ItemColor[0]);

							if (CurItem->Flags & DIF_SHOWAMPERSAND)
								Text(strResult);
							else
								HiText(strResult,ItemColor[1]);


							if (++CountLine >= (DWORD)CH)
								break;
						}
					}
					strWrap.ReleaseBuffer();
				}

				break;
			}
			/* ***************************************************************** */
			case DI_VTEXT:
			{
				strStr = CurItem->strData;
				LenText=LenStrItem(I,strStr);

				if (!(CurItem->Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2)) && (CurItem->Flags & DIF_CENTERTEXT) && CY1!=-1)
					LenText = static_cast<int>(CenterStr(strStr,strStr,CY2-CY1+1).GetLength());

				if ((CY2 <= 0) || (CY2 < CY1))
					CH = LenStrItem(I,strStr);

				X=(CX1==-1)?(X2-X1+1)/2:CX1;
				Y=(CY1==-1 || (CurItem->Flags & DIF_CENTERTEXT))?(Y2-Y1+1-LenText)/2:CY1;

				if( (CurItem->Flags & DIF_RIGHTTEXT) && CY2 > CY1 )
					Y=CY2-LenText+1;

				if (Y < 0)
					Y=0;

				if (Y1+Y+LenText > Y2)
				{
					int tmpCH=ObjHeight();

					if (CH < ObjHeight())
						tmpCH=CH+1;

					strStr.SetLength(tmpCH-1);
				}

				// нужно ЭТО
				//SetScreen(X1+CX1,Y1+CY1,X1+CX2,Y1+CY2,' ',Attr&0xFF);
				// вместо этого:
				if (CY1 > -1 && CY2 > CY1 && !(CurItem->Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2))) //половинчатое решение
				{
					SetScreen(X1+X,Y1+CY1,X1+X,Y1+CY2,L' ',ItemColor[0]);
					/*
					int CntChr=CY2-CY1+1;
					SetColor(ItemColor[0]);
					GotoXY(X1+X,Y1+Y);

					if (Y1+Y+CntChr-1 > Y2)
						CntChr=Y2-(Y1+Y)+1;

					vmprintf(L"%*s",CntChr,L"");
					*/
				}

#if defined(VTEXT_ADN_SEPARATORS)

				if (CurItem->Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2))
				{
					SetColor(ItemColor[2]);
					GotoXY(X1+X,Y1+ ((CurItem->Flags&DIF_SEPARATORUSER)?Y:(!DialogMode.Check(DMODE_SMALLDIALOG)?1:0)));  //????
					ShowUserSeparator((CurItem->Flags&DIF_SEPARATORUSER)?Y2-Y1+1:RealHeight-(!DialogMode.Check(DMODE_SMALLDIALOG)?2:0),
					                  (CurItem->Flags&DIF_SEPARATORUSER)?13:(CurItem->Flags&DIF_SEPARATOR2?7:5),
					                  CurItem->strMask
					                 );
				}

#endif
				SetColor(ItemColor[0]);
				GotoXY(X1+X,Y1+Y);

				if (CurItem->Flags & DIF_SHOWAMPERSAND)
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
				GotoXY(X1+CX1,Y1+CY1);

				if (CurItem->Type==DI_CHECKBOX)
				{
					const wchar_t Check[]={L'[',(CurItem->Selected ?(((CurItem->Flags&DIF_3STATE) && CurItem->Selected == 2)?*MSG(MCheckBox2State):L'x'):L' '),L']',L'\0'};
					strStr=Check;

					if (CurItem->strData.GetLength())
						strStr+=L" ";
				}
				else
				{
					wchar_t Dot[]={L' ',CurItem->Selected ? L'\x2022':L' ',L' ',L'\0'};

					if (CurItem->Flags&DIF_MOVESELECT)
					{
						strStr=Dot;
					}
					else
					{
						Dot[0]=L'(';
						Dot[2]=L')';
						strStr=Dot;

						if (CurItem->strData.GetLength())
							strStr+=L" ";
					}
				}

				strStr += CurItem->strData;
				LenText=LenStrItem(I, strStr);

				if (X1+CX1+LenText > X2)
					strStr.SetLength(ObjWidth()-1);

				if (CurItem->Flags & DIF_SHOWAMPERSAND)
					Text(strStr);
				else
					HiText(strStr,ItemColor[1]);

				if (CurItem->Flags&DIF_FOCUS)
				{
					//   Отключение мигающего курсора при перемещении диалога
					if (!DialogMode.Check(DMODE_DRAGGED))
						SetCursorType(1,-1);

					MoveCursor(X1+CX1+1,Y1+CY1);
				}

				break;
			}
			/* ***************************************************************** */
			case DI_BUTTON:
			{
				strStr = CurItem->strData;
				SetColor(ItemColor[0]);
				GotoXY(X1+CX1,Y1+CY1);

				if (CurItem->Flags & DIF_SHOWAMPERSAND)
					Text(strStr);
				else
					HiText(strStr,ItemColor[1]);

				if(CurItem->Flags & DIF_SETSHIELD)
				{
					int startx=X1+CX1+(CurItem->Flags&DIF_NOBRACKETS?0:2);
					FarColor Color;
					Colors::ConsoleColorToFarColor(0xE9, Color);
					Global->ScrBuf->ApplyColor(startx,Y1+CY1,startx+1,Y1+CY1,Color);
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
				DlgEdit *EditPtr=(DlgEdit *)(CurItem->ObjPtr);

				if (!EditPtr)
					break;

				EditPtr->SetObjectColor(ItemColor[0],ItemColor[1],ItemColor[2]);

				if (CurItem->Flags&DIF_FOCUS)
				{
					//   Отключение мигающего курсора при перемещении диалога
					if (!DialogMode.Check(DMODE_DRAGGED))
						SetCursorType(1,-1);

					EditPtr->Show();
				}
				else
				{
					EditPtr->FastShow();
					EditPtr->SetLeftPos(0);
				}

				//   Отключение мигающего курсора при перемещении диалога
				if (DialogMode.Check(DMODE_DRAGGED))
					SetCursorType(0,0);

				if (ItemHasDropDownArrow(CurItem))
				{
					int EditX1,EditY1,EditX2,EditY2;
					EditPtr->GetPosition(EditX1,EditY1,EditX2,EditY2);
					//Text((CurItem->Type == DI_COMBOBOX?"\x1F":"\x19"));
					Text(EditX2+1,EditY1,ItemColor[3],L"\x2193");
				}

				if (CurItem->Type == DI_COMBOBOX && GetDropDownOpened() && CurItem->ListPtr->IsVisible()) // need redraw VMenu?
				{
					CurItem->ListPtr->Hide();
					CurItem->ListPtr->Show();
				}

				break;
			}
			/* ***************************************************************** */
			case DI_LISTBOX:
			{
				if (CurItem->ListPtr)
				{
					//   Перед отрисовкой спросим об изменении цветовых атрибутов
					FarColor RealColors[VMENU_COLOR_COUNT] = {};
					FarDialogItemColors ListColors={sizeof(FarDialogItemColors)};
					ListColors.ColorsCount=VMENU_COLOR_COUNT;
					ListColors.Colors=RealColors;
					CurItem->ListPtr->GetColors(&ListColors);

					if (DlgProc(DN_CTLCOLORDLGLIST,I,&ListColors))
						CurItem->ListPtr->SetColors(&ListColors);

					// Курсор запоминаем...
					bool CursorVisible=false;
					DWORD CursorSize=0;
					GetCursorType(CursorVisible,CursorSize);
					CurItem->ListPtr->Show();

					// .. а теперь восстановим!
					if (FocusPos != I)
						SetCursorType(CursorVisible,CursorSize);
				}

				break;
			}
			/* 01.08.2000 SVS $ */
			/* ***************************************************************** */
			case DI_USERCONTROL:

				if (CurItem->VBuf)
				{
					PutText(X1+CX1,Y1+CY1,X1+CX2,Y1+CY2,CurItem->VBuf);

					// не забудем переместить курсор, если он позиционирован.
					if (FocusPos == I)
					{
						if (CurItem->UCData->CursorPos.X != -1 &&
						        CurItem->UCData->CursorPos.Y != -1)
						{
							MoveCursor(CurItem->UCData->CursorPos.X+CX1+X1,CurItem->UCData->CursorPos.Y+CY1+Y1);
							SetCursorType(CurItem->UCData->CursorVisible,CurItem->UCData->CursorSize);
						}
						else
							SetCursorType(0,-1);
					}
				}

				break; //уже наприсовали :-)))
				/* ***************************************************************** */
				//.........
		} // end switch(...
	} // end for (I=...

	// КОСТЫЛЬ!
	// но работает ;-)
	std::for_each(CONST_RANGE(Items, i)
	{
		if (i->ListPtr && GetDropDownOpened() && i->ListPtr->IsVisible())
		{
			if ((i->Type == DI_COMBOBOX) ||
			        ((i->Type == DI_EDIT || i->Type == DI_FIXEDIT) &&
			         !(i->Flags&DIF_HIDDEN) &&
			         (i->Flags&DIF_HISTORY)))
			{
				i->ListPtr->Show();
			}
		}
	});

	//   Включим индикатор перемещения...
	if (!DialogMode.Check(DMODE_DRAGGED)) // если диалог таскается
	{
		/* $ 03.06.2001 KM
		   + При каждой перерисовке диалога, кроме режима перемещения, устанавливаем
		     заголовок консоли, в противном случае он не всегда восстанавливался.
		*/
		if (!DialogMode.Check(DMODE_KEEPCONSOLETITLE))
			ConsoleTitle::SetFarTitle(GetDialogTitle());
	}

	DialogMode.Clear(DMODE_DRAWING);  // конец отрисовки диалога!!!
	DialogMode.Set(DMODE_SHOW); // диалог на экране!

	if (DialogMode.Check(DMODE_DRAGGED))
	{
		/*
		- BugZ#813 - DM_RESIZEDIALOG в DN_DRAWDIALOG -> проблема: Ctrl-F5 - отрисовка только полозьев.
		  Убираем вызов плагиновго обработчика.
		*/
		//DlgProc(this,DN_DRAWDIALOGDONE,1,0);
		DefProc(DN_DRAWDIALOGDONE,1,0);
	}
	else
		DlgProc(DN_DRAWDIALOGDONE,0,0);
}

int Dialog::LenStrItem(size_t ID, const wchar_t *lpwszStr)
{
	CriticalSectionLock Lock(CS);

	if (!lpwszStr)
		lpwszStr = Items[ID]->strData;

	return (Items[ID]->Flags & DIF_SHOWAMPERSAND)?StrLength(lpwszStr):HiStrlen(lpwszStr);
}


int Dialog::ProcessMoveDialog(DWORD Key)
{
	CriticalSectionLock Lock(CS);

	if (DialogMode.Check(DMODE_DRAGGED)) // если диалог таскается
	{
		// TODO: Здесь проверить "уже здесь" и не делать лишних движений
		//       Т.е., если нажали End, то при следующем End ненужно ничего делать! - сравнить координаты !!!
		int rr=1;

		//   При перемещении диалога повторяем поведение "бормандовых" сред.
		switch (Key)
		{
			case KEY_CTRLLEFT:  case KEY_CTRLNUMPAD4:
			case KEY_RCTRLLEFT: case KEY_RCTRLNUMPAD4:
			case KEY_CTRLHOME:  case KEY_CTRLNUMPAD7:
			case KEY_RCTRLHOME: case KEY_RCTRLNUMPAD7:
			case KEY_HOME:      case KEY_NUMPAD7:
				rr=(Key == KEY_CTRLLEFT || Key == KEY_RCTRLLEFT || Key == KEY_CTRLNUMPAD4 || Key == KEY_RCTRLNUMPAD4)?10:X1;
			case KEY_LEFT:      case KEY_NUMPAD4:
				Hide();

				for (int i=0; i<rr; i++)
					if (X2>0)
					{
						X1--;
						X2--;
						AdjustEditPos(-1,0);
					}

				if (!DialogMode.Check(DMODE_ALTDRAGGED)) Show();

				break;
			case KEY_CTRLRIGHT:  case KEY_CTRLNUMPAD6:
			case KEY_RCTRLRIGHT: case KEY_RCTRLNUMPAD6:
			case KEY_CTRLEND:    case KEY_CTRLNUMPAD1:
			case KEY_RCTRLEND:   case KEY_RCTRLNUMPAD1:
			case KEY_END:       case KEY_NUMPAD1:
				rr=(Key == KEY_CTRLRIGHT || Key == KEY_RCTRLRIGHT || Key == KEY_CTRLNUMPAD6 || Key == KEY_RCTRLNUMPAD6)?10:std::max(0,ScrX-X2);
			case KEY_RIGHT:     case KEY_NUMPAD6:
				Hide();

				for (int i=0; i<rr; i++)
					if (X1<ScrX)
					{
						X1++;
						X2++;
						AdjustEditPos(1,0);
					}

				if (!DialogMode.Check(DMODE_ALTDRAGGED)) Show();

				break;
			case KEY_PGUP:      case KEY_NUMPAD9:
			case KEY_CTRLPGUP:  case KEY_CTRLNUMPAD9:
			case KEY_RCTRLPGUP: case KEY_RCTRLNUMPAD9:
			case KEY_CTRLUP:    case KEY_CTRLNUMPAD8:
			case KEY_RCTRLUP:   case KEY_RCTRLNUMPAD8:
				rr=(Key == KEY_CTRLUP || Key == KEY_RCTRLUP || Key == KEY_CTRLNUMPAD8 || Key == KEY_RCTRLNUMPAD8)?5:Y1;
			case KEY_UP:        case KEY_NUMPAD8:
				Hide();

				for (int i=0; i<rr; i++)
					if (Y2>0)
					{
						Y1--;
						Y2--;
						AdjustEditPos(0,-1);
					}

				if (!DialogMode.Check(DMODE_ALTDRAGGED)) Show();

				break;
			case KEY_CTRLDOWN:  case KEY_CTRLNUMPAD2:
			case KEY_RCTRLDOWN: case KEY_RCTRLNUMPAD2:
			case KEY_CTRLPGDN:  case KEY_CTRLNUMPAD3:
			case KEY_RCTRLPGDN: case KEY_RCTRLNUMPAD3:
			case KEY_PGDN:      case KEY_NUMPAD3:
				rr=(Key == KEY_CTRLDOWN || Key == KEY_RCTRLDOWN || Key == KEY_CTRLNUMPAD2 || Key == KEY_RCTRLNUMPAD2)? 5:std::max(0,ScrY-Y2);
			case KEY_DOWN:      case KEY_NUMPAD2:
				Hide();

				for (int i=0; i<rr; i++)
					if (Y1<ScrY)
					{
						Y1++;
						Y2++;
						AdjustEditPos(0,1);
					}

				if (!DialogMode.Check(DMODE_ALTDRAGGED)) Show();

				break;
			case KEY_NUMENTER:
			case KEY_ENTER:
			case KEY_CTRLF5:
			case KEY_RCTRLF5:
				DialogMode.Clear(DMODE_DRAGGED); // закончим движение!

				if (!DialogMode.Check(DMODE_ALTDRAGGED))
				{
					DlgProc(DN_DRAGGED,1,0);
					Show();
				}

				break;
			case KEY_ESC:
				Hide();
				AdjustEditPos(OldX1-X1,OldY1-Y1);
				X1=OldX1;
				X2=OldX2;
				Y1=OldY1;
				Y2=OldY2;
				DialogMode.Clear(DMODE_DRAGGED);

				if (!DialogMode.Check(DMODE_ALTDRAGGED))
				{
					DlgProc(DN_DRAGGED,1,ToPtr(TRUE));
					Show();
				}

				break;
		}

		if (DialogMode.Check(DMODE_ALTDRAGGED))
		{
			DialogMode.Clear(DMODE_DRAGGED|DMODE_ALTDRAGGED);
			DlgProc(DN_DRAGGED,1,0);
			Show();
		}

		return (TRUE);
	}

	if ((Key == KEY_CTRLF5 || Key == KEY_RCTRLF5) && DialogMode.Check(DMODE_ISCANMOVE))
	{
		if (DlgProc(DN_DRAGGED,0,0)) // если разрешили перемещать!
		{
			// включаем флаг и запоминаем координаты
			DialogMode.Set(DMODE_DRAGGED);
			OldX1=X1; OldX2=X2; OldY1=Y1; OldY2=Y2;
			//# GetText(0,0,3,0,LV);
			Show();
		}

		return (TRUE);
	}

	return (FALSE);
}

__int64 Dialog::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
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
			const wchar_t *str = (const wchar_t *)vParam;

			if (GetDropDownOpened() || Items[FocusPos]->Type == DI_LISTBOX)
			{
				if (Items[FocusPos]->ListPtr)
					return Items[FocusPos]->ListPtr->VMProcess(OpCode,vParam,iParam);
			}
			else if (OpCode == MCODE_F_MENU_CHECKHOTKEY)
				return (__int64)(CheckHighlights(*str,(int)iParam) + 1);

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
			if (IsEdit(Items[FocusPos]->Type))
			{
				if (Items[FocusPos]->Type == DI_COMBOBOX && GetDropDownOpened())
					return Items[FocusPos]->ListPtr->VMProcess(OpCode,vParam,iParam);
				else
					return ((DlgEdit *)(Items[FocusPos]->ObjPtr))->VMProcess(OpCode,vParam,iParam);
			}
			else if (Items[FocusPos]->Type == DI_LISTBOX && OpCode != MCODE_C_SELECTED)
				return Items[FocusPos]->ListPtr->VMProcess(OpCode,vParam,iParam);

			return 0;
		}
		case MCODE_V_DLGITEMTYPE:
		{
			switch (Items[FocusPos]->Type)
			{
				case DI_BUTTON:      return 7; // Кнопка (Push Button).
				case DI_CHECKBOX:    return 8; // Контрольный переключатель (Check Box).
				case DI_COMBOBOX:    return (DropDownOpened?0x800A:10); // Комбинированный список.
				case DI_DOUBLEBOX:   return 3; // Двойная рамка.
				case DI_EDIT:        return DropDownOpened?0x8004:4; // Поле ввода.
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
		case MCODE_V_DLGITEMCOUNT: // Dlg.ItemCount
		{
			return Items.size();
		}
		case MCODE_V_DLGCURPOS:    // Dlg.CurPos
		{
			return FocusPos+1;
		}
		case MCODE_V_DLGPREVPOS:    // Dlg.PrevPos
		{
			return PrevFocusPos+1;
		}
		case MCODE_V_DLGINFOID:        // Dlg.Info.Id
		{
			static string strId;
			strId = GuidToStr(Id);
			return reinterpret_cast<intptr_t>(strId.CPtr());
		}
		case MCODE_V_DLGINFOOWNER:        // Dlg.Info.Owner
		{
			static string strOwner;
			GUID Owner = FarGuid;
			if (PluginOwner)
			{
				Owner = PluginOwner->GetGUID();
			}
			strOwner = GuidToStr(Owner);
			return reinterpret_cast<intptr_t>(strOwner.CPtr());
		}
		case MCODE_V_ITEMCOUNT:
		case MCODE_V_CURPOS:
		{
			__int64 Ret=0;
			switch (Items[FocusPos]->Type)
			{
				case DI_COMBOBOX:

					if (DropDownOpened || (Items[FocusPos]->Flags & DIF_DROPDOWNLIST))
					{
						Ret=Items[FocusPos]->ListPtr->VMProcess(OpCode,vParam,iParam);
						break;
					}

				case DI_EDIT:
				case DI_PSWEDIT:
				case DI_FIXEDIT:
					return ((DlgEdit *)(Items[FocusPos]->ObjPtr))->VMProcess(OpCode,vParam,iParam);

				case DI_LISTBOX:
					Ret=Items[FocusPos]->ListPtr->VMProcess(OpCode,vParam,iParam);
					break;

				case DI_USERCONTROL:
				{
					if (OpCode == MCODE_V_CURPOS)
						Ret=Items[FocusPos]->UCData->CursorPos.X;
					break;
				}
				case DI_BUTTON:
				case DI_CHECKBOX:
				case DI_RADIOBUTTON:
					return 0;
				default:
					return 0;
			}

			FarGetValue fgv={sizeof(FarGetValue),OpCode==MCODE_V_ITEMCOUNT?11:7,{FMVT_INTEGER}};
			fgv.Value.Integer=Ret;

			if (SendMessage(DN_GETVALUE,FocusPos,&fgv))
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
			if (IsEdit(Items[FocusPos]->Type) || (Items[FocusPos]->Type==DI_COMBOBOX && !(DropDownOpened || (Items[FocusPos]->Flags & DIF_DROPDOWNLIST))))
			{
				return ((DlgEdit *)(Items[FocusPos]->ObjPtr))->VMProcess(OpCode,vParam,iParam);
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
int Dialog::ProcessKey(int Key)
{
	CriticalSectionLock Lock(CS);
	_DIALOG(CleverSysLog CL(L"Dialog::ProcessKey"));
	_DIALOG(SysLog(L"Param: Key=%s",_FARKEY_ToName(Key)));

	string strStr;

	if (Key==KEY_NONE || Key==KEY_IDLE)
	{
		DlgProc(DN_ENTERIDLE,0,0); // $ 28.07.2000 SVS Передадим этот факт в обработчик :-)
		return FALSE;
	}

	if (Key == KEY_KILLFOCUS || Key == KEY_GOTFOCUS)
	{
		DlgProc(DN_ACTIVATEAPP,Key == KEY_KILLFOCUS?FALSE:TRUE,0);
		return FALSE;
	}

	if (ProcessMoveDialog(Key))
		return TRUE;

	// BugZ#488 - Shift=enter
	if (IntKeyState.ShiftPressed && (Key == KEY_ENTER||Key==KEY_NUMENTER) && !Global->CtrlObject->Macro.IsExecuting() && Items[FocusPos]->Type != DI_BUTTON)
	{
		Key=Key == KEY_ENTER?KEY_SHIFTENTER:KEY_SHIFTNUMENTER;
	}

	if (!(/*(Key>=KEY_MACRO_BASE && Key <=KEY_MACRO_ENDBASE) ||*/ ((unsigned int)Key>=KEY_OP_BASE && (unsigned int)Key <=KEY_OP_ENDBASE)) && !DialogMode.Check(DMODE_KEY))
	{
#if 1	// wrap-stop mode for user lists
		if ((Key==KEY_UP || Key==KEY_NUMPAD8 || Key==KEY_DOWN || Key==KEY_NUMPAD2) && IsRepeatedKey())
		{
			int n = -1, pos = -1;

			FarGetValue fgv = {sizeof(FarGetValue),11, {FMVT_INTEGER}}; // Item Count
			fgv.Value.Integer = -1;
			if (SendMessage(DN_GETVALUE,FocusPos,&fgv) && fgv.Value.Type==FMVT_INTEGER)
				n = static_cast<int>(fgv.Value.Integer);

			if (n > 1)
			{
				fgv.Type = 7; // Current Item
				fgv.Value.Integer = -1;
				if (SendMessage(DN_GETVALUE,FocusPos,&fgv) && fgv.Value.Type==FMVT_INTEGER)
					pos = static_cast<int>(fgv.Value.Integer);

				bool up = (Key==KEY_UP || Key==KEY_NUMPAD8);

				if ((pos==1 && up) || (pos==n && !up))
					return TRUE;
				else if (pos==2 && up)   // workaround for first not selectable
					Key = KEY_HOME;
				else if (pos==n-1 && !up) // workaround for last not selectable
					Key = KEY_END;
			}
		}
#endif
		INPUT_RECORD rec;
		if (KeyToInputRecord(Key,&rec) && DlgProc(DN_CONTROLINPUT,FocusPos,&rec))
			return TRUE;
	}

	if (!DialogMode.Check(DMODE_SHOW))
		return TRUE;

	// А ХЗ, может в этот момент изменилось состояние элемента!
	if (Items[FocusPos]->Flags&DIF_HIDDEN)
		return TRUE;

	// небольшая оптимизация
	if (Items[FocusPos]->Type==DI_CHECKBOX)
	{
		if (!(Items[FocusPos]->Flags&DIF_3STATE))
		{
			if (Key == KEY_MULTIPLY) // в CheckBox 2-state Gray* не работает!
				Key = KEY_NONE;

			if ((Key == KEY_ADD      && !Items[FocusPos]->Selected) ||
			        (Key == KEY_SUBTRACT &&  Items[FocusPos]->Selected))
				Key=KEY_SPACE;
		}

		/*
		  блок else не нужен, т.к. ниже клавиши будут обработаны...
		*/
	}
	else if (Key == KEY_ADD)
		Key='+';
	else if (Key == KEY_SUBTRACT)
		Key='-';
	else if (Key == KEY_MULTIPLY)
		Key='*';

	if (Items[FocusPos]->Type==DI_BUTTON && Key == KEY_SPACE)
		Key=KEY_ENTER;

	if (Items[FocusPos]->Type == DI_LISTBOX)
	{
		switch (Key)
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
				VMenu *List=Items[FocusPos]->ListPtr;
				int CurListPos=List->GetSelectPos();
				int CheckedListItem=List->GetCheck(-1);
				List->ProcessKey(Key);
				int NewListPos=List->GetSelectPos();

				if (NewListPos != CurListPos && !DlgProc(DN_LISTCHANGE,FocusPos,ToPtr(NewListPos)))
				{
					if (!DialogMode.Check(DMODE_SHOW))
						return TRUE;

					List->SetCheck(CheckedListItem,CurListPos);

					if (DialogMode.Check(DMODE_SHOW) && !(Items[FocusPos]->Flags&DIF_HIDDEN))
						ShowDialog(FocusPos); // FocusPos
				}

				if (!(Key == KEY_ENTER || Key == KEY_NUMENTER) || (Items[FocusPos]->Flags&DIF_LISTNOCLOSE))
					return TRUE;
		}
	}

	switch (Key)
	{
		case KEY_F1:

			// Перед выводом диалога посылаем сообщение в обработчик
			//   и если вернули что надо, то выводим подсказку
			if (!Help::MkTopic(PluginOwner, (const wchar_t*)DlgProc(DN_HELP,FocusPos, (void*)EmptyToNull(HelpTopic)), strStr).IsEmpty())
			{
				Help Hlp(strStr);
			}

			return TRUE;
		case KEY_ESC:
		case KEY_BREAK:
		case KEY_F10:
			ExitCode=(Key==KEY_BREAK) ? -2:-1;
			CloseDialog();
			return TRUE;
		case KEY_HOME: case KEY_NUMPAD7:

			if (Items[FocusPos]->Type == DI_USERCONTROL) // для user-типа вываливаем
				return TRUE;

			return Do_ProcessFirstCtrl();
		case KEY_TAB:
		case KEY_SHIFTTAB:
			return Do_ProcessTab(Key==KEY_TAB);
		case KEY_SPACE:
			return Do_ProcessSpace();
		case KEY_CTRLNUMENTER:
		case KEY_RCTRLNUMENTER:
		case KEY_CTRLENTER:
		case KEY_RCTRLENTER:
		{
			FOR_CONST_RANGE(Items, i)
			{
				if ((*i)->Flags & DIF_DEFAULTBUTTON)
				{
					if ((*i)->Flags&DIF_DISABLE)
					{
						// ProcessKey(KEY_DOWN); // на твой вкус :-)
						return TRUE;
					}

					if (!IsEdit((*i)->Type))
						(*i)->Selected=1;

					ExitCode = i - Items.begin();
					/* $ 18.05.2001 DJ */
					CloseDialog();
					/* DJ $ */
					return TRUE;
				}
			}

			if (!DialogMode.Check(DMODE_OLDSTYLE))
			{
				DialogMode.Clear(DMODE_ENDLOOP); // только если есть
				return TRUE; // делать больше не чего
			}
		}
		case KEY_NUMENTER:
		case KEY_ENTER:
		{
			if (Items[FocusPos]->Type != DI_COMBOBOX
			        && IsEdit(Items[FocusPos]->Type)
			        && (Items[FocusPos]->Flags & DIF_EDITOR) && !(Items[FocusPos]->Flags & DIF_READONLY))
			{
				size_t I, EditorLastPos;

				for (EditorLastPos=I=FocusPos; I < Items.size(); I++)
					if (IsEdit(Items[I]->Type) && (Items[I]->Flags & DIF_EDITOR))
						EditorLastPos=I;
					else
						break;

				if (((DlgEdit *)(Items[EditorLastPos]->ObjPtr))->GetLength())
					return TRUE;

				for (I=EditorLastPos; I>FocusPos; I--)
				{
					int CurPos;

					if (I==FocusPos+1)
						CurPos=((DlgEdit *)(Items[I-1]->ObjPtr))->GetCurPos();
					else
						CurPos=0;

					((DlgEdit *)(Items[I-1]->ObjPtr))->GetString(strStr);
					int Length=(int)strStr.GetLength();
					((DlgEdit *)(Items[I]->ObjPtr))->SetString(CurPos>=Length ? L"":strStr.CPtr()+CurPos);

					if (CurPos<Length)
						strStr.SetLength(CurPos);

					((DlgEdit *)(Items[I]->ObjPtr))->SetCurPos(0);
					((DlgEdit *)(Items[I-1]->ObjPtr))->SetString(strStr);
				}

				if (EditorLastPos > FocusPos)
				{
					((DlgEdit *)(Items[FocusPos]->ObjPtr))->SetCurPos(0);
					Do_ProcessNextCtrl(FALSE,FALSE);
				}

				ShowDialog();
				return TRUE;
			}
			else if (Items[FocusPos]->Type==DI_BUTTON)
			{
				Items[FocusPos]->Selected=1;

				// сообщение - "Кнокна кликнута"
				if (SendMessage(DN_BTNCLICK,FocusPos,0))
					return TRUE;

				if (Items[FocusPos]->Flags&DIF_BTNNOCLOSE)
					return TRUE;

				ExitCode=static_cast<int>(FocusPos);
				CloseDialog();
				return TRUE;
			}
			else
			{
				ExitCode=-1;

				FOR_CONST_RANGE(Items, i)
				{
					if (((*i)->Flags&DIF_DEFAULTBUTTON) && !((*i)->Flags&DIF_BTNNOCLOSE))
					{
						if ((*i)->Flags&DIF_DISABLE)
						{
							// ProcessKey(KEY_DOWN); // на твой вкус :-)
							return TRUE;
						}

//            if (!(IsEdit((*i).Type) || (*i).Type == DI_CHECKBOX || (*i).Type == DI_RADIOBUTTON))
//              (*i).Selected=1;
						ExitCode= i - Items.begin();
						break;
					}
				}
			}

			if (ExitCode==-1)
				ExitCode=static_cast<int>(FocusPos);

			CloseDialog();
			return TRUE;
		}
		/*
		   3-х уровневое состояние
		   Для чекбокса сюда попадем только в случае, если контрол
		   имеет флаг DIF_3STATE
		*/
		case KEY_ADD:
		case KEY_SUBTRACT:
		case KEY_MULTIPLY:

			if (Items[FocusPos]->Type==DI_CHECKBOX)
			{
				unsigned int CHKState=
				    (Key == KEY_ADD?1:
				     (Key == KEY_SUBTRACT?0:
				      ((Key == KEY_MULTIPLY)?2:
				       Items[FocusPos]->Selected)));

				if (Items[FocusPos]->Selected != (int)CHKState)
					if (SendMessage(DN_BTNCLICK,FocusPos,ToPtr(CHKState)))
					{
						Items[FocusPos]->Selected=CHKState;
						ShowDialog();
					}
			}

			return TRUE;
		case KEY_LEFT:  case KEY_NUMPAD4: case KEY_MSWHEEL_LEFT:
		case KEY_RIGHT: case KEY_NUMPAD6: case KEY_MSWHEEL_RIGHT:
		{
			if (Items[FocusPos]->Type == DI_USERCONTROL) // для user-типа вываливаем
				return TRUE;

			if (IsEdit(Items[FocusPos]->Type))
			{
				((DlgEdit *)(Items[FocusPos]->ObjPtr))->ProcessKey(Key);
				return TRUE;
			}
			else
			{
				size_t MinDist=1000, Pos = 0, MinPos=0;
				std::for_each(CONST_RANGE(Items, i)
				{
					if (Pos != FocusPos &&
					        (IsEdit(i->Type) ||
					         i->Type==DI_CHECKBOX ||
					         i->Type==DI_RADIOBUTTON) &&
					         i->Y1==Items[FocusPos]->Y1)
					{
						int Dist = i->X1-Items[FocusPos]->X1;

						if (((Key==KEY_LEFT||Key==KEY_SHIFTNUMPAD4) && Dist<0) || ((Key==KEY_RIGHT||Key==KEY_SHIFTNUMPAD6) && Dist>0))
						{
							if (static_cast<size_t>(abs(Dist))<MinDist)
							{
								MinDist=static_cast<size_t>(abs(Dist));
								MinPos = Pos;
							}
						}
					}
					++Pos;
				});

				if (MinDist<1000)
				{
					ChangeFocus2(MinPos);

					if (Items[MinPos]->Flags & DIF_MOVESELECT)
					{
						Do_ProcessSpace();
					}
					else
					{
						ShowDialog();
					}

					return TRUE;
				}
			}
		}
		case KEY_UP:    case KEY_NUMPAD8:
		case KEY_DOWN:  case KEY_NUMPAD2:

			if (Items[FocusPos]->Type == DI_USERCONTROL) // для user-типа вываливаем
				return TRUE;

			return Do_ProcessNextCtrl(Key==KEY_LEFT || Key==KEY_UP || Key == KEY_NUMPAD4 || Key == KEY_NUMPAD8);
			// $ 27.04.2001 VVM - Обработка колеса мышки
		case KEY_MSWHEEL_UP:
		case KEY_MSWHEEL_DOWN:
		case KEY_CTRLUP:      case KEY_CTRLNUMPAD8:
		case KEY_RCTRLUP:     case KEY_RCTRLNUMPAD8:
		case KEY_CTRLDOWN:    case KEY_CTRLNUMPAD2:
		case KEY_RCTRLDOWN:   case KEY_RCTRLNUMPAD2:
			return ProcessOpenComboBox(Items[FocusPos]->Type,Items[FocusPos].get(),FocusPos);
			// ЭТО перед default предпоследний!!!
		case KEY_END:  case KEY_NUMPAD1:

			if (Items[FocusPos]->Type == DI_USERCONTROL) // для user-типа вываливаем
				return TRUE;

			if (IsEdit(Items[FocusPos]->Type))
			{
				((DlgEdit *)(Items[FocusPos]->ObjPtr))->ProcessKey(Key);
				return TRUE;
			}

			// ???
			// ЭТО перед default последний!!!
		case KEY_PGDN:   case KEY_NUMPAD3:

			if (Items[FocusPos]->Type == DI_USERCONTROL) // для user-типа вываливаем
				return TRUE;

			if (!(Items[FocusPos]->Flags & DIF_EDITOR))
			{
				FOR_CONST_RANGE(Items, i)
				{
					if ((*i)->Flags&DIF_DEFAULTBUTTON)
					{
						ChangeFocus2(i - Items.begin());
						ShowDialog();
						return TRUE;
					}
				}
				return TRUE;
			}
			break;

		case KEY_F11:
			if (!Global->IsProcessAssignMacroKey)
			{
				if(!CheckDialogMode(DMODE_NOPLUGINS))
				{
					return FrameManager->ProcessKey(Key);
				}
			}
			break;

			// для DIF_EDITOR будет обработано ниже
		default:
		{
			//if(Items[FocusPos].Type == DI_USERCONTROL) // для user-типа вываливаем
			//  return TRUE;
			if (Items[FocusPos]->Type == DI_LISTBOX)
			{
				VMenu *List=Items[FocusPos]->ListPtr;
				int CurListPos=List->GetSelectPos();
				int CheckedListItem=List->GetCheck(-1);
				List->ProcessKey(Key);
				int NewListPos=List->GetSelectPos();

				if (NewListPos != CurListPos && !DlgProc(DN_LISTCHANGE,FocusPos,ToPtr(NewListPos)))
				{
					if (!DialogMode.Check(DMODE_SHOW))
						return TRUE;

					List->SetCheck(CheckedListItem,CurListPos);

					if (DialogMode.Check(DMODE_SHOW) && !(Items[FocusPos]->Flags&DIF_HIDDEN))
						ShowDialog(FocusPos); // FocusPos
				}

				return TRUE;
			}

			if (IsEdit(Items[FocusPos]->Type))
			{
				DlgEdit *edt=(DlgEdit *)Items[FocusPos]->ObjPtr;

				if (Key == KEY_CTRLL || Key == KEY_RCTRLL) // исключим смену режима RO для поля ввода с клавиатуры
				{
					return TRUE;
				}
				else if (Key == KEY_CTRLU || Key == KEY_RCTRLU)
				{
					edt->SetClearFlag(0);
					edt->Select(-1,0);
					edt->Show();
					return TRUE;
				}
				else if ((Items[FocusPos]->Flags & DIF_EDITOR) && !(Items[FocusPos]->Flags & DIF_READONLY))
				{
					switch (Key)
					{
						case KEY_BS:
						{
							int CurPos=edt->GetCurPos();

							// В начале строки????
							if (!edt->GetCurPos())
							{
								// а "выше" тоже DIF_EDITOR?
								if (FocusPos > 0 && (Items[FocusPos-1]->Flags&DIF_EDITOR))
								{
									// добавляем к предыдущему и...
									DlgEdit *edt_1=(DlgEdit *)Items[FocusPos-1]->ObjPtr;
									edt_1->GetString(strStr);
									CurPos=static_cast<int>(strStr.GetLength());
									string strAdd;
									edt->GetString(strAdd);
									strStr+=strAdd;
									edt_1->SetString(strStr);

									for (size_t I = FocusPos + 1; I < Items.size(); ++I)
									{
										if (Items[I]->Flags & DIF_EDITOR)
										{
											if (I>FocusPos)
											{
												((DlgEdit *)(Items[I]->ObjPtr))->GetString(strStr);
												((DlgEdit *)(Items[I-1]->ObjPtr))->SetString(strStr);
											}

											((DlgEdit *)(Items[I]->ObjPtr))->SetString(L"");
										}
										else // ага, значит  FocusPos это есть последний из DIF_EDITOR
										{
											((DlgEdit *)(Items[I-1]->ObjPtr))->SetString(L"");
											break;
										}
									}

									Do_ProcessNextCtrl(TRUE);
									edt_1->SetCurPos(CurPos);
								}
							}
							else
							{
								edt->ProcessKey(Key);
							}

							ShowDialog();
							return TRUE;
						}
						case KEY_CTRLY:
						case KEY_RCTRLY:
						{
							for (size_t I = FocusPos; I < Items.size(); ++I)
								if (Items[I]->Flags & DIF_EDITOR)
								{
									if (I>FocusPos)
									{
										((DlgEdit *)(Items[I]->ObjPtr))->GetString(strStr);
										((DlgEdit *)(Items[I-1]->ObjPtr))->SetString(strStr);
									}

									((DlgEdit *)(Items[I]->ObjPtr))->SetString(L"");
								}
								else
									break;

							ShowDialog();
							return TRUE;
						}
						case KEY_NUMDEL:
						case KEY_DEL:
						{
							/* $ 19.07.2000 SVS
							   ! "...В редакторе команд меню нажмите home shift+end del
							     блок не удаляется..."
							     DEL у итемов, имеющих DIF_EDITOR, работал без учета
							     выделения...
							*/
							if (FocusPos<Items.size()+1 && (Items[FocusPos+1]->Flags & DIF_EDITOR))
							{
								int CurPos=edt->GetCurPos();
								int Length=edt->GetLength();
								intptr_t SelStart, SelEnd;
								edt->GetSelection(SelStart, SelEnd);
								edt->GetString(strStr);

								if (SelStart > -1)
								{
									string strEnd=strStr.CPtr()+SelEnd;
									strStr.SetLength(SelStart);
									strStr+=strEnd;
									edt->SetString(strStr);
									edt->SetCurPos(SelStart);
									ShowDialog();
									return TRUE;
								}
								else if (CurPos>=Length)
								{
									DlgEdit *edt_1=(DlgEdit *)Items[FocusPos+1]->ObjPtr;

									/* $ 12.09.2000 SVS
									   Решаем проблему, если Del нажали в позиции
									   большей, чем длина строки
									*/
									if (CurPos > Length)
									{
										LPWSTR Str=strStr.GetBuffer(CurPos);
										wmemset(Str+Length,L' ',CurPos-Length);
										strStr.ReleaseBuffer(CurPos);
									}

									string strAdd;
									edt_1->GetString(strAdd);
									edt_1->SetString(strStr+strAdd);
									ProcessKey(KEY_CTRLY);
									edt->SetCurPos(CurPos);
									ShowDialog();
									return TRUE;
								}
							}

							break;
						}
						case KEY_PGDN:  case KEY_NUMPAD3:
						case KEY_PGUP:  case KEY_NUMPAD9:
						{
							size_t I = FocusPos;

							while (Items[I]->Flags & DIF_EDITOR)
								I=ChangeFocus(I,(Key == KEY_PGUP || Key == KEY_NUMPAD9)?-1:1,FALSE);

							if (!(Items[I]->Flags & DIF_EDITOR))
								I=ChangeFocus(I,(Key == KEY_PGUP || Key == KEY_NUMPAD9)?1:-1,FALSE);

							ChangeFocus2(I);
							ShowDialog();

							return TRUE;
						}
					}
				}

				if (Key == KEY_OP_XLAT && !(Items[FocusPos]->Flags & DIF_READONLY))
				{
					edt->SetClearFlag(0);
					edt->Xlat();

					// иначе неправильно работает ctrl-end
					edt->strLastStr = edt->GetStringAddr();
					edt->LastPartLength=static_cast<int>(edt->strLastStr.GetLength());

					Redraw(); // Перерисовка должна идти после DN_EDITCHANGE (imho)
					return TRUE;
				}

				if (!(Items[FocusPos]->Flags & DIF_READONLY) ||
				        ((Items[FocusPos]->Flags & DIF_READONLY) && IsNavKey(Key)))
				{
					// "только что ломанулись и начинать выделение с нуля"?
					if ((Global->Opt->Dialogs.EditLine&DLGEDITLINE_NEWSELONGOTFOCUS) && Items[FocusPos]->SelStart != -1 && PrevFocusPos != FocusPos)// && Items[FocusPos].SelEnd)
					{
						edt->Flags().Clear(FEDITLINE_MARKINGBLOCK);
						PrevFocusPos=FocusPos;
					}

					if(Key == KEY_CTRLSPACE || Key == KEY_RCTRLSPACE)
					{
						SetAutocomplete enable(edt, true);
						edt->AutoComplete(true,false);
						Redraw();
						return TRUE;
					}

					if (edt->ProcessKey(Key))
					{
						if (Items[FocusPos]->Flags & DIF_READONLY)
							return TRUE;

						if ((Key==KEY_CTRLEND || Key==KEY_RCTRLEND || Key==KEY_CTRLNUMPAD1 || Key==KEY_RCTRLNUMPAD1) && edt->GetCurPos()==edt->GetLength())
						{
							if (edt->LastPartLength ==-1)
								edt->strLastStr = edt->GetStringAddr();

							strStr = edt->strLastStr;
							int CurCmdPartLength=static_cast<int>(strStr.GetLength());
							edt->HistoryGetSimilar(strStr, edt->LastPartLength);

							if (edt->LastPartLength == -1)
							{
								edt->strLastStr = edt->GetStringAddr();
								edt->LastPartLength = CurCmdPartLength;
							}
							{
								SetAutocomplete disable(edt);
								edt->SetString(strStr);
								edt->Select(edt->LastPartLength, static_cast<int>(strStr.GetLength()));
							}
							Show();
							return TRUE;
						}

						edt->LastPartLength=-1;

						Redraw(); // Перерисовка должна идти после DN_EDITCHANGE (imho)
						return TRUE;
					}
				}
				else if (!(Key&(KEY_ALT|KEY_RALT)))
					return TRUE;
			}

			if (ProcessHighlighting(Key,FocusPos,FALSE))
				return TRUE;

			return(ProcessHighlighting(Key,FocusPos,TRUE));
		}
	}
	return FALSE;
}

void Dialog::ProcessKey(int Key, size_t ItemPos)
{
	size_t SavedFocusPos = FocusPos;
	FocusPos = ItemPos;
	ProcessKey(Key);
	if (FocusPos == ItemPos)
		FocusPos = SavedFocusPos;
}


//////////////////////////////////////////////////////////////////////////
/* Public, Virtual:
   Обработка данных от "мыши".
   Перекрывает BaseInput::ProcessMouse.
*/
/* $ 18.08.2000 SVS
   + DN_MOUSECLICK
*/
int Dialog::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	CriticalSectionLock Lock(CS);
	int MsX,MsY;
	FARDIALOGITEMTYPES Type;
	SMALL_RECT Rect;
	INPUT_RECORD mouse = {};
	mouse.EventType=MOUSE_EVENT;
	mouse.Event.MouseEvent=*MouseEvent;
	MOUSE_EVENT_RECORD &MouseRecord=mouse.Event.MouseEvent;

	if (!DialogMode.Check(DMODE_SHOW))
		return FALSE;

	if (DialogMode.Check(DMODE_MOUSEEVENT))
	{
		if (!DlgProc(DN_INPUT,0,&mouse))
			return TRUE;
	}

	if (!DialogMode.Check(DMODE_SHOW))
		return FALSE;

	MsX=MouseRecord.dwMousePosition.X;
	MsY=MouseRecord.dwMousePosition.Y;

	//for (I=0;I<ItemCount;I++)
	for (size_t I=Items.size()-1; I!=(size_t)-1; I--)
	{
		if (Items[I]->Flags&(DIF_DISABLE|DIF_HIDDEN))
			continue;

		Type=Items[I]->Type;

		if (Type == DI_LISTBOX &&
		        MsY >= Y1+Items[I]->Y1 && MsY <= Y1+Items[I]->Y2 &&
		        MsX >= X1+Items[I]->X1 && MsX <= X1+Items[I]->X2)
		{
			VMenu *List=Items[I]->ListPtr;
			int Pos=List->GetSelectPos();
			int CheckedListItem=List->GetCheck(-1);

			if (!MouseRecord.dwEventFlags && !(MouseRecord.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) && (PrevMouseRecord.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED))
			{
				if (PrevMouseRecord.dwMousePosition.X==MsX && PrevMouseRecord.dwMousePosition.Y==MsY)
				{
					ExitCode=static_cast<int>(I);
					CloseDialog();
					return TRUE;
				}
				PrevMouseRecord=MouseRecord;
			}

			if ((MouseRecord.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED))
			{
				if (FocusPos != I)
				{
					ChangeFocus2(I);
					ShowDialog();
				}

				if (MouseRecord.dwEventFlags!=DOUBLE_CLICK && !(Items[I]->Flags&(DIF_LISTTRACKMOUSE|DIF_LISTTRACKMOUSEINFOCUS)))
				{
					List->ProcessMouse(&MouseRecord);
					int NewListPos=List->GetSelectPos();

					if (NewListPos != Pos && !SendMessage(DN_LISTCHANGE,I,ToPtr(NewListPos)))
					{
						List->SetCheck(CheckedListItem,Pos);

						if (DialogMode.Check(DMODE_SHOW) && !(Items[I]->Flags&DIF_HIDDEN))
							ShowDialog(I); // FocusPos
					}
					else
					{
						Pos=NewListPos;
					}
				}
				else if (!SendMessage(DN_CONTROLINPUT,I,&mouse))
				{
#if 1
					List->ProcessMouse(&MouseRecord);
					int NewListPos=List->GetSelectPos();
					int InScroolBar=(MsX==X1+Items[I]->X2 && MsY >= Y1+Items[I]->Y1 && MsY <= Y1+Items[I]->Y2) &&
					                (List->CheckFlags(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR) || Global->Opt->ShowMenuScrollbar);

					if (!InScroolBar       &&                                                                // вне скроллбара и
					        NewListPos != Pos &&                                                                 // позиция изменилась и
					        !SendMessage(DN_LISTCHANGE,I,ToPtr(NewListPos)))                      // и плагин сказал в морг
					{
						List->SetCheck(CheckedListItem,Pos);

						if (DialogMode.Check(DMODE_SHOW) && !(Items[I]->Flags&DIF_HIDDEN))
							ShowDialog(I); // FocusPos
					}
					else
					{
						Pos=NewListPos;

						if (List->CheckFlags(VMENU_SHOWNOBOX) ||  (MsY > Y1+Items[I]->Y1 && MsY < Y1+Items[I]->Y2))
							if (!InScroolBar && !(Items[I]->Flags&DIF_LISTNOCLOSE))
							{
								if (MouseRecord.dwEventFlags==DOUBLE_CLICK)
								{
									ExitCode=static_cast<int>(I);
									CloseDialog();
									return TRUE;
								}
								if (!MouseRecord.dwEventFlags && (MouseRecord.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) && !(PrevMouseRecord.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED))
									PrevMouseRecord=MouseRecord;
							}
					}

#else

					if (SendDlgMessage(this,DN_LISTCHANGE,I,(intptr_t)Pos))
					{
						if (MsX==X1+Items[I]->X2 && MsY >= Y1+Items[I]->Y1 && MsY <= Y1+Items[I]->Y2)
							List->ProcessMouse(&mouse.Event.MouseEvent); // забыл проверить на клик на скролбар (KM)
						else
							ProcessKey(KEY_ENTER, I);
					}

#endif
				}

				return TRUE;
			}
			else
			{
				if (!mouse.Event.MouseEvent.dwButtonState || SendMessage(DN_CONTROLINPUT,I,&mouse))
				{
					if ((I == FocusPos && (Items[I]->Flags&DIF_LISTTRACKMOUSEINFOCUS)) || (Items[I]->Flags&DIF_LISTTRACKMOUSE))
					{
						List->ProcessMouse(&mouse.Event.MouseEvent);
						int NewListPos=List->GetSelectPos();

						if (NewListPos != Pos && !SendMessage(DN_LISTCHANGE,I,ToPtr(NewListPos)))
						{
							List->SetCheck(CheckedListItem,Pos);

							if (DialogMode.Check(DMODE_SHOW) && !(Items[I]->Flags&DIF_HIDDEN))
								ShowDialog(I); // FocusPos
						}
						else
							Pos=NewListPos;
					}
				}
			}

			return TRUE;
		}
	}

	if (MsX<X1 || MsY<Y1 || MsX>X2 || MsY>Y2)
	{
		if (DialogMode.Check(DMODE_CLICKOUTSIDE) && !DlgProc(DN_CONTROLINPUT,-1,&mouse))
		{
			if (!DialogMode.Check(DMODE_SHOW))
				return FALSE;

			if (!(mouse.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) && (IntKeyState.PrevMouseButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) && (Global->Opt->Dialogs.MouseButton&DMOUSEBUTTON_LEFT))
				ProcessKey(KEY_ESC);
			else if (!(mouse.Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED) && (IntKeyState.PrevMouseButtonState&RIGHTMOST_BUTTON_PRESSED) && (Global->Opt->Dialogs.MouseButton&DMOUSEBUTTON_RIGHT))
				ProcessKey(KEY_ENTER);
		}

		if (mouse.Event.MouseEvent.dwButtonState)
			DialogMode.Set(DMODE_CLICKOUTSIDE);

		return TRUE;
	}

	if (!mouse.Event.MouseEvent.dwButtonState)
	{
		DialogMode.Clear(DMODE_CLICKOUTSIDE);
		return FALSE;
	}

	if (!mouse.Event.MouseEvent.dwEventFlags || mouse.Event.MouseEvent.dwEventFlags==DOUBLE_CLICK)
	{
		// первый цикл - все за исключением рамок.
		//for (I=0; I < ItemCount;I++)
		for (size_t I=Items.size()-1; I!=(size_t)-1; I--)
		{
			if (Items[I]->Flags&(DIF_DISABLE|DIF_HIDDEN))
				continue;

			GetItemRect(I,Rect);
			Rect.Left+=X1;  Rect.Top+=Y1;
			Rect.Right+=X1; Rect.Bottom+=Y1;
//_D(SysLog(L"? %2d) Rect (%2d,%2d) (%2d,%2d) '%s'",I,Rect.left,Rect.top,Rect.right,Rect.bottom,Items[I].Data));

			if (MsX >= Rect.Left && MsY >= Rect.Top && MsX <= Rect.Right && MsY <= Rect.Bottom)
			{
				// для прозрачных :-)
				if (Items[I]->Type == DI_SINGLEBOX || Items[I]->Type == DI_DOUBLEBOX)
				{
					// если на рамке, то...
					if (((MsX == Rect.Left || MsX == Rect.Right) && MsY >= Rect.Top && MsY <= Rect.Bottom) || // vert
					        ((MsY == Rect.Top  || MsY == Rect.Bottom) && MsX >= Rect.Left && MsX <= Rect.Right))    // hor
					{
						if (DlgProc(DN_CONTROLINPUT,I,&mouse))
							return TRUE;

						if (!DialogMode.Check(DMODE_SHOW))
							return FALSE;
					}
					else
						continue;
				}

				if (Items[I]->Type == DI_USERCONTROL)
				{
					// для user-типа подготовим координаты мыши
					mouse.Event.MouseEvent.dwMousePosition.X-=Rect.Left;
					mouse.Event.MouseEvent.dwMousePosition.Y-=Rect.Top;
				}

//_SVS(SysLog(L"+ %2d) Rect (%2d,%2d) (%2d,%2d) '%s' Dbl=%d",I,Rect.left,Rect.top,Rect.right,Rect.bottom,Items[I].Data,mouse.Event.MouseEvent.dwEventFlags==DOUBLE_CLICK));
				if (DlgProc(DN_CONTROLINPUT,I,&mouse))
					return TRUE;

				if (!DialogMode.Check(DMODE_SHOW))
					return TRUE;

				if (Items[I]->Type == DI_USERCONTROL)
				{
					ChangeFocus2(I);
					ShowDialog();
					return TRUE;
				}

				break;
			}
		}

		if ((mouse.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED))
		{
			//for (I=0;I<ItemCount;I++)

			for (size_t I=Items.size()-1; I!=(size_t)-1; I--)
			{
				//   Исключаем из списка оповещаемых о мыши недоступные элементы
				if (Items[I]->Flags&(DIF_DISABLE|DIF_HIDDEN))
					continue;

				Type=Items[I]->Type;

				GetItemRect(I,Rect);
				Rect.Left+=X1;  Rect.Top+=Y1;
				Rect.Right+=X1; Rect.Bottom+=Y1;
				if (ItemHasDropDownArrow(Items[I].get()))
					Rect.Right++;

				if (MsX >= Rect.Left && MsY >= Rect.Top && MsX <= Rect.Right && MsY <= Rect.Bottom)
				{
					/* ********************************************************** */
					if (IsEdit(Type))
					{
						/* $ 15.08.2000 SVS
						   + Сделаем так, чтобы ткнув мышкой в DropDownList
						     список раскрывался сам.
						   Есть некоторая глюкавость - когда список раскрыт и мы
						   мышой переваливаем на другой элемент, то список закрывается
						   но перехода реального на указанный элемент диалога не происходит
						*/
						int EditX1,EditY1,EditX2,EditY2;
						DlgEdit *EditLine=(DlgEdit *)(Items[I]->ObjPtr);
						EditLine->GetPosition(EditX1,EditY1,EditX2,EditY2);

						if (MsY==EditY1 && Type == DI_COMBOBOX &&
						        (Items[I]->Flags & DIF_DROPDOWNLIST) &&
						        MsX >= EditX1 && MsX <= EditX2+1)
						{
							EditLine->SetClearFlag(0);

							ChangeFocus2(I);
							ShowDialog();

							ProcessOpenComboBox(Items[I]->Type,Items[I].get(),I);

							return TRUE;
						}

						ChangeFocus2(I);

						if (EditLine->ProcessMouse(&mouse.Event.MouseEvent))
						{
							EditLine->SetClearFlag(0); // а может это делать в самом edit?

							/* $ 23.06.2001 KM
							   ! Оказалось нужно перерисовывать весь диалог иначе
							     не снимался признак активности с комбобокса с которго уходим.
							*/
							ShowDialog(); // нужен ли только один контрол или весь диалог?
							return TRUE;
						}
						else
						{
							// Проверка на DI_COMBOBOX здесь лишняя. Убрана (KM).
							if (MsX==EditX2+1 && MsY==EditY1 && ItemHasDropDownArrow(Items[I].get()))
							{
								EditLine->SetClearFlag(0); // раз уж покусились на, то и...

								ChangeFocus2(I);

								if (!(Items[I]->Flags&DIF_HIDDEN))
									ShowDialog(I);

								ProcessOpenComboBox(Items[I]->Type,Items[I].get(),I);

								return TRUE;
							}
						}
					}

					/* ********************************************************** */
					if (Type==DI_BUTTON &&
					        MsY==Y1+Items[I]->Y1 &&
					        MsX < X1+Items[I]->X1+HiStrlen(Items[I]->strData))
					{
						ChangeFocus2(I);
						ShowDialog();

						while (IsMouseButtonPressed());

						if (IntKeyState.MouseX <  X1 ||
						        IntKeyState.MouseX >  X1+Items[I]->X1+HiStrlen(Items[I]->strData)+4 ||
						        IntKeyState.MouseY != Y1+Items[I]->Y1)
						{
							ChangeFocus2(I);
							ShowDialog();

							return TRUE;
						}

						ProcessKey(KEY_ENTER, I);
						return TRUE;
					}

					/* ********************************************************** */
					if ((Type == DI_CHECKBOX ||
					        Type == DI_RADIOBUTTON) &&
					        MsY==Y1+Items[I]->Y1 &&
					        MsX < (X1+Items[I]->X1+HiStrlen(Items[I]->strData)+4-((Items[I]->Flags & DIF_MOVESELECT)!=0)))
					{
						ChangeFocus2(I);
						ProcessKey(KEY_SPACE, I);
						return TRUE;
					}
				}
			} // for (I=0;I<ItemCount;I++)

			// ДЛЯ MOUSE-Перемещалки:
			//   Сюда попадаем в том случае, если мышь не попала на активные элементы
			//

			if (DialogMode.Check(DMODE_ISCANMOVE))
			{
				//DialogMode.Set(DMODE_DRAGGED);
				OldX1=X1; OldX2=X2; OldY1=Y1; OldY2=Y2;
				// запомним delta места хватания и Left-Top диалогового окна
				MsX=abs(X1-IntKeyState.MouseX);
				MsY=abs(Y1-IntKeyState.MouseY);
				int NeedSendMsg=0;

				for (;;)
				{
					DWORD Mb=IsMouseButtonPressed();

					if (Mb==FROM_LEFT_1ST_BUTTON_PRESSED) // still dragging
					{
						int mx,my;
						if (IntKeyState.MouseX==IntKeyState.PrevMouseX)
							mx=X1;
						else
							mx=IntKeyState.MouseX-MsX;

						if (IntKeyState.MouseY==IntKeyState.PrevMouseY)
							my=Y1;
						else
							my=IntKeyState.MouseY-MsY;

						int X0=X1, Y0=Y1;
						int OX1=X1 ,OY1=Y1;
						int NX1=mx, NX2=mx+(X2-X1);
						int NY1=my, NY2=my+(Y2-Y1);
						int AdjX=NX1-X0, AdjY=NY1-Y0;

						// "А был ли мальчик?" (про холостой ход)
						if (OX1 != NX1 || OY1 != NY1)
						{
							if (!NeedSendMsg) // тыкс, а уже посылку делали в диалоговую процедуру?
							{
								NeedSendMsg++;

								if (!DlgProc(DN_DRAGGED,0,0)) // а может нас обломали?
									break;  // валим отсель...плагин сказал - в морг перемещения

								if (!DialogMode.Check(DMODE_SHOW))
									break;
							}

							// Да, мальчик был. Зачнем...
							{
								LockScreen LckScr;
								Hide();
								X1=NX1; X2=NX2; Y1=NY1; Y2=NY2;

								if (AdjX || AdjY)
									AdjustEditPos(AdjX,AdjY); //?

								Show();
							}
						}
					}
					else if (Mb==RIGHTMOST_BUTTON_PRESSED) // abort
					{
						LockScreen LckScr;
						Hide();
						AdjustEditPos(OldX1-X1,OldY1-Y1);
						X1=OldX1;
						X2=OldX2;
						Y1=OldY1;
						Y2=OldY2;
						DialogMode.Clear(DMODE_DRAGGED);
						DlgProc(DN_DRAGGED,1,ToPtr(TRUE));

						if (DialogMode.Check(DMODE_SHOW))
							Show();

						break;
					}
					else  // release key, drop dialog
					{
						if (OldX1!=X1 || OldX2!=X2 || OldY1!=Y1 || OldY2!=Y2)
						{
							LockScreen LckScr;
							DialogMode.Clear(DMODE_DRAGGED);
							DlgProc(DN_DRAGGED,1,0);

							if (DialogMode.Check(DMODE_SHOW))
								Show();
						}

						break;
					}
				}// while (1)
			}
		}
	}

	return FALSE;
}


int Dialog::ProcessOpenComboBox(FARDIALOGITEMTYPES Type,DialogItemEx *CurItem, size_t CurFocusPos)
{
	CriticalSectionLock Lock(CS);
	string strStr;
	DlgEdit *CurEditLine;

	// для user-типа вываливаем
	if (Type == DI_USERCONTROL)
		return TRUE;

	CurEditLine=((DlgEdit *)(CurItem->ObjPtr));

	if (IsEdit(Type) &&
	        (CurItem->Flags & DIF_HISTORY) &&
	        Global->Opt->Dialogs.EditHistory &&
	        !CurItem->strHistory.IsEmpty() &&
	        !(CurItem->Flags & DIF_READONLY))
	{
		// Передаем то, что в строке ввода в функцию выбора из истории для выделения нужного пункта в истории.
		CurEditLine->GetString(strStr);
		SelectFromEditHistory(CurItem,CurEditLine,CurItem->strHistory,strStr);
	}
	// $ 18.07.2000 SVS:  +обработка DI_COMBOBOX - выбор из списка!
	else if (Type == DI_COMBOBOX && CurItem->ListPtr &&
	         !(CurItem->Flags & DIF_READONLY) &&
	         CurItem->ListPtr->GetItemCount() > 0) //??
	{
		SelectFromComboBox(CurItem,CurEditLine,CurItem->ListPtr);
	}

	return TRUE;
}

size_t Dialog::ProcessRadioButton(size_t CurRB)
{
	CriticalSectionLock Lock(CS);
	size_t PrevRB=CurRB, I, J;

	for (I=CurRB;; I--)
	{
		if (!I)
			break;

		if (Items[I]->Type==DI_RADIOBUTTON && (Items[I]->Flags & DIF_GROUP))
			break;

		if (Items[I-1]->Type!=DI_RADIOBUTTON)
			break;
	}

	do
	{
		/* $ 28.07.2000 SVS
		  При изменении состояния каждого элемента посылаем сообщение
		  посредством функции SendDlgMessage - в ней делается все!
		*/
		J=Items[I]->Selected;
		Items[I]->Selected=0;

		if (J)
		{
			PrevRB=I;
		}

		++I;
	}
	while (I<Items.size() && Items[I]->Type==DI_RADIOBUTTON &&
	        !(Items[I]->Flags & DIF_GROUP));

	Items[CurRB]->Selected=1;

	size_t ret = CurRB, focus = FocusPos;

	/* $ 28.07.2000 SVS
	  При изменении состояния каждого элемента посылаем сообщение
	  посредством функции SendDlgMessage - в ней делается все!
	*/
	if (!SendMessage(DN_BTNCLICK,PrevRB,0) ||
		!SendMessage(DN_BTNCLICK,CurRB,ToPtr(1)))
	{
		// вернем назад, если пользователь не захотел...
		Items[CurRB]->Selected=0;
		Items[PrevRB]->Selected=1;
		ret = PrevRB;
	}

	return (focus == FocusPos ? ret : FocusPos); // если фокус изменили - значит так надо!
}


int Dialog::Do_ProcessFirstCtrl()
{
	CriticalSectionLock Lock(CS);

	if (IsEdit(Items[FocusPos]->Type))
	{
		((DlgEdit *)(Items[FocusPos]->ObjPtr))->ProcessKey(KEY_HOME);
		return TRUE;
	}
	else
	{
		FOR_CONST_RANGE(Items, i)
		{
			if (CanGetFocus((*i)->Type))
			{
				ChangeFocus2(i - Items.begin());
				ShowDialog();
				break;
			}
		}
	}

	return TRUE;
}

int Dialog::Do_ProcessNextCtrl(int Up,BOOL IsRedraw)
{
	CriticalSectionLock Lock(CS);
	size_t OldPos=FocusPos;
	unsigned PrevPos=0;

	if (IsEdit(Items[FocusPos]->Type) && (Items[FocusPos]->Flags & DIF_EDITOR))
		PrevPos=((DlgEdit *)(Items[FocusPos]->ObjPtr))->GetCurPos();

	size_t I=ChangeFocus(FocusPos,Up? -1:1,FALSE);
	Items[FocusPos]->Flags&=~DIF_FOCUS;
	Items[I]->Flags|=DIF_FOCUS;
	ChangeFocus2(I);

	if (IsEdit(Items[I]->Type) && (Items[I]->Flags & DIF_EDITOR))
		((DlgEdit *)(Items[I]->ObjPtr))->SetCurPos(PrevPos);

	if (Items[FocusPos]->Type == DI_RADIOBUTTON && (Items[I]->Flags & DIF_MOVESELECT))
		ProcessKey(KEY_SPACE);
	else if (IsRedraw)
	{
		ShowDialog(OldPos);
		ShowDialog(FocusPos);
	}

	return TRUE;
}

int Dialog::Do_ProcessTab(int Next)
{
	CriticalSectionLock Lock(CS);
	size_t I;

	if (Items.size() > 1)
	{
		// Здесь с фокусом ОООЧЕНЬ ТУМАННО!!!
		if (Items[FocusPos]->Flags & DIF_EDITOR)
		{
			I=FocusPos;

			while (Items[I]->Flags & DIF_EDITOR)
				I=ChangeFocus(I,Next ? 1:-1,TRUE);
		}
		else
		{
			I=ChangeFocus(FocusPos,Next ? 1:-1,TRUE);

			if (!Next)
				while (I>0 && (Items[I]->Flags & DIF_EDITOR) &&
				        (Items[I-1]->Flags & DIF_EDITOR) &&
				        !((DlgEdit *)Items[I]->ObjPtr)->GetLength())
					I--;
		}
	}
	else
		I=FocusPos;

	ChangeFocus2(I);
	ShowDialog();

	return TRUE;
}


int Dialog::Do_ProcessSpace()
{
	CriticalSectionLock Lock(CS);

	if (Items[FocusPos]->Type==DI_CHECKBOX)
	{
		int OldSelected=Items[FocusPos]->Selected;

		if (Items[FocusPos]->Flags&DIF_3STATE)
			(++Items[FocusPos]->Selected)%=3;
		else
			Items[FocusPos]->Selected = !Items[FocusPos]->Selected;

		size_t OldFocusPos=FocusPos;

		if (!SendMessage(DN_BTNCLICK,FocusPos,ToPtr(Items[FocusPos]->Selected)))
			Items[OldFocusPos]->Selected = OldSelected;

		ShowDialog();
		return TRUE;
	}
	else if (Items[FocusPos]->Type==DI_RADIOBUTTON)
	{
		FocusPos=ProcessRadioButton(FocusPos);
		ShowDialog();
		return TRUE;
	}
	else if (IsEdit(Items[FocusPos]->Type) && !(Items[FocusPos]->Flags & DIF_READONLY))
	{
		if (((DlgEdit *)(Items[FocusPos]->ObjPtr))->ProcessKey(KEY_SPACE))
		{
			Redraw(); // Перерисовка должна идти после DN_EDITCHANGE (imho)
		}

		return TRUE;
	}

	return TRUE;
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
size_t Dialog::ChangeFocus(size_t CurFocusPos,int Step,int SkipGroup)
{
	CriticalSectionLock Lock(CS);
	size_t OrigFocusPos=CurFocusPos;
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

			if ((int)CurFocusPos<0)
				CurFocusPos=Items.size()-1;

			if (CurFocusPos>=Items.size())
				CurFocusPos=0;

			FARDIALOGITEMTYPES Type=Items[CurFocusPos]->Type;

			if (!(Items[CurFocusPos]->Flags&(DIF_NOFOCUS|DIF_DISABLE|DIF_HIDDEN)))
			{
				if (Type==DI_LISTBOX || Type==DI_BUTTON || Type==DI_CHECKBOX || IsEdit(Type) || Type==DI_USERCONTROL)
					break;

				if (Type==DI_RADIOBUTTON && (!SkipGroup || Items[CurFocusPos]->Selected))
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
	return(CurFocusPos);
}


//////////////////////////////////////////////////////////////////////////
/*
   Private:
   Изменяет фокус ввода между двумя элементами.
   Вынесен отдельно с тем, чтобы обработать DN_KILLFOCUS & DM_SETFOCUS
*/
void Dialog::ChangeFocus2(size_t SetFocusPos)
{
	CriticalSectionLock Lock(CS);

	if (!(Items[SetFocusPos]->Flags&(DIF_NOFOCUS|DIF_DISABLE|DIF_HIDDEN)))
	{
		int FocusPosNeed=-1;
		if (DialogMode.Check(DMODE_INITOBJECTS))
		{
			FocusPosNeed=(int)DlgProc(DN_KILLFOCUS,FocusPos,0);

			if (!DialogMode.Check(DMODE_SHOW))
				return;
		}

		if (FocusPosNeed != -1 && CanGetFocus(Items[FocusPosNeed]->Type))
			SetFocusPos=FocusPosNeed;

		Items[FocusPos]->Flags&=~DIF_FOCUS;

		// "снимать выделение при потере фокуса?"
		if (IsEdit(Items[FocusPos]->Type) &&
		        !(Items[FocusPos]->Type == DI_COMBOBOX && (Items[FocusPos]->Flags & DIF_DROPDOWNLIST)))
		{
			DlgEdit *EditPtr=(DlgEdit*)Items[FocusPos]->ObjPtr;
			EditPtr->GetSelection(Items[FocusPos]->SelStart,Items[FocusPos]->SelEnd);

			if ((Global->Opt->Dialogs.EditLine&DLGEDITLINE_CLEARSELONKILLFOCUS))
			{
				EditPtr->Select(-1,0);
			}
		}

		Items[SetFocusPos]->Flags|=DIF_FOCUS;

		// "не восстанавливать выделение при получении фокуса?"
		if (IsEdit(Items[SetFocusPos]->Type) &&
		        !(Items[SetFocusPos]->Type == DI_COMBOBOX && (Items[SetFocusPos]->Flags & DIF_DROPDOWNLIST)))
		{
			DlgEdit *EditPtr=(DlgEdit*)Items[SetFocusPos]->ObjPtr;

			if (!(Global->Opt->Dialogs.EditLine&DLGEDITLINE_NOTSELONGOTFOCUS))
			{
				if (Global->Opt->Dialogs.EditLine&DLGEDITLINE_SELALLGOTFOCUS)
					EditPtr->Select(0,EditPtr->GetStrSize());
				else
					EditPtr->Select(Items[SetFocusPos]->SelStart,Items[SetFocusPos]->SelEnd);
			}
			else
			{
				EditPtr->Select(-1,0);
			}

			// при получении фокуса ввода переместить курсор в конец строки?
			if (Global->Opt->Dialogs.EditLine&DLGEDITLINE_GOTOEOLGOTFOCUS)
			{
				EditPtr->SetCurPos(EditPtr->GetStrSize());
			}
		}

		//   проинформируем листбокс, есть ли у него фокус
		if (Items[FocusPos]->Type == DI_LISTBOX)
			Items[FocusPos]->ListPtr->ClearFlags(VMENU_LISTHASFOCUS);

		if (Items[SetFocusPos]->Type == DI_LISTBOX)
			Items[SetFocusPos]->ListPtr->SetFlags(VMENU_LISTHASFOCUS);

		SelectOnEntry(FocusPos,FALSE);
		SelectOnEntry(SetFocusPos,TRUE);

		PrevFocusPos=FocusPos;
		FocusPos=SetFocusPos;

		if (DialogMode.Check(DMODE_INITOBJECTS))
			DlgProc(DN_GOTFOCUS,FocusPos,0);
	}
}

/*
  Функция SelectOnEntry - выделение строки редактирования
  Обработка флага DIF_SELECTONENTRY
*/
void Dialog::SelectOnEntry(size_t Pos,BOOL Selected)
{
	//if(!DialogMode.Check(DMODE_SHOW))
	//   return;
	if (IsEdit(Items[Pos]->Type) &&
	        (Items[Pos]->Flags&DIF_SELECTONENTRY)
//     && PrevFocusPos != -1 && PrevFocusPos != Pos
	   )
	{
		DlgEdit *edt=(DlgEdit *)Items[Pos]->ObjPtr;

		if (edt)
		{
			if (Selected)
				edt->Select(0,edt->GetLength());
			else
				edt->Select(-1,0);

			//_SVS(SysLog(L"Selected=%d edt->GetLength()=%d",Selected,edt->GetLength()));
		}
	}
}

int Dialog::SetAutomation(WORD IDParent,WORD id,
                          FARDIALOGITEMFLAGS UncheckedSet,FARDIALOGITEMFLAGS UncheckedSkip,
                          FARDIALOGITEMFLAGS CheckedSet,FARDIALOGITEMFLAGS CheckedSkip,
                          FARDIALOGITEMFLAGS Checked3Set,FARDIALOGITEMFLAGS Checked3Skip)
{
	CriticalSectionLock Lock(CS);
	int Ret=FALSE;

	if (IDParent < Items.size() && (Items[IDParent]->Flags&DIF_AUTOMATION) &&
	        id < Items.size() && IDParent != id) // Сами себя не юзаем!
	{
		Ret = Items[IDParent]->AddAutomation(id, UncheckedSet, UncheckedSkip,
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
    DlgEdit *EditLine,                   // строка редактирования
    VMenu *ComboBox)    // список строк
{
		CriticalSectionLock Lock(CS);
		//char *Str;
		string strStr;
		int I,Dest, OriginalPos;
		size_t CurFocusPos=FocusPos;
		int EditX1,EditY1,EditX2,EditY2;
		EditLine->GetPosition(EditX1,EditY1,EditX2,EditY2);

		if (EditX2-EditX1<20)
			EditX2=EditX1+20;

		SetDropDownOpened(TRUE); // Установим флаг "открытия" комбобокса.
		SetComboBoxPos(CurItem);
		// Перед отрисовкой спросим об изменении цветовых атрибутов
		FarColor RealColors[VMENU_COLOR_COUNT] = {};
		FarDialogItemColors ListColors={sizeof(FarDialogItemColors)};
		ListColors.ColorsCount=VMENU_COLOR_COUNT;
		ListColors.Colors=RealColors;
		ComboBox->SetColors(nullptr);
		ComboBox->GetColors(&ListColors);

		if (DlgProc(DN_CTLCOLORDLGLIST,CurItem->ID,&ListColors))
			ComboBox->SetColors(&ListColors);

		// Выставим то, что есть в строке ввода!
		// if(EditLine->GetDropDownBox()) //???
		EditLine->GetString(strStr);

		if (CurItem->Flags & (DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND))
			HiText2Str(strStr, strStr);

		ComboBox->SetSelectPos(ComboBox->FindItem(0,strStr,LIFIND_EXACTMATCH),1);
		ComboBox->Show();
		OriginalPos=Dest=ComboBox->GetSelectPos();
		CurItem->IFlags.Set(DLGIIF_COMBOBOXNOREDRAWEDIT);

		while (!ComboBox->Done())
		{
			if (!GetDropDownOpened())
			{
				ComboBox->ProcessKey(KEY_ESC);
				continue;
			}

			INPUT_RECORD ReadRec;
			int Key=ComboBox->ReadInput(&ReadRec);

			if (CurItem->IFlags.Check(DLGIIF_COMBOBOXEVENTKEY) && ReadRec.EventType == KEY_EVENT)
			{
				if (DlgProc(DN_CONTROLINPUT,FocusPos,&ReadRec))
					continue;
			}
			else if (CurItem->IFlags.Check(DLGIIF_COMBOBOXEVENTMOUSE) && ReadRec.EventType == MOUSE_EVENT)
				if (!DlgProc(DN_INPUT,0,&ReadRec))
					continue;

			// здесь можно добавить что-то свое, например,
			I=ComboBox->GetSelectPos();

			if (Key==KEY_TAB) // Tab в списке - аналог Enter
			{
				ComboBox->ProcessKey(KEY_ENTER);
				continue; //??
			}

			if (I != Dest)
			{
				if (!DlgProc(DN_LISTCHANGE,CurFocusPos,ToPtr(I)))
					ComboBox->SetSelectPos(Dest,Dest<I?-1:1); //????
				else
					Dest=I;

#if 0

				// во время навигации по DropDown листу - отобразим ЭТО дело в
				// связанной строке
				// ВНИМАНИЕ!!!
				//  Очень медленная реакция!
				if (EditLine->GetDropDownBox())
				{
					MenuItem *CurCBItem=ComboBox->GetItemPtr();
					EditLine->SetString(CurCBItem->Name);
					EditLine->Show();
					//EditLine->FastShow();
				}

#endif
			}

			// обработку multiselect ComboBox
			// ...
			ComboBox->ProcessInput();
		}

		CurItem->IFlags.Clear(DLGIIF_COMBOBOXNOREDRAWEDIT);
		ComboBox->ClearDone();
		ComboBox->Hide();

		if (GetDropDownOpened()) // Закрылся не программным путём?
			Dest=ComboBox->Modal::GetExitCode();
		else
			Dest=-1;

		if (Dest == -1)
			ComboBox->SetSelectPos(OriginalPos,0); //????

		SetDropDownOpened(FALSE); // Установим флаг "закрытия" комбобокса.

		if (Dest<0)
		{
			Redraw();
			//xf_free(Str);
			return KEY_ESC;
		}

		//ComboBox->GetUserData(Str,MaxLen,Dest);
		MenuItemEx *ItemPtr=ComboBox->GetItemPtr(Dest);

		if (CurItem->Flags & (DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND))
		{
			HiText2Str(strStr, ItemPtr->strName);
			EditLine->SetString(strStr);
		}
		else
			EditLine->SetString(ItemPtr->strName);

		EditLine->SetLeftPos(0);
		Redraw();
		//xf_free(Str);
		return KEY_ENTER;
}

//////////////////////////////////////////////////////////////////////////
/* Private:
   Заполняем выпадающий список из истории
*/
BOOL Dialog::SelectFromEditHistory(DialogItemEx *CurItem,
                                   DlgEdit *EditLine,
                                   const wchar_t *HistoryName,
                                   string &strIStr)
{
	CriticalSectionLock Lock(CS);

	if (!EditLine)
		return FALSE;

	string strStr;
	int ret=0;
	History *DlgHist = static_cast<DlgEdit*>(CurItem->ObjPtr)->GetHistory();

	if(DlgHist)
	{
		DlgHist->ResetPosition();
		// создание пустого вертикального меню
		VMenu2 HistoryMenu(L"",nullptr,0,Global->Opt->Dialogs.CBoxMaxHeight,VMENU_ALWAYSSCROLLBAR|VMENU_COMBOBOX|VMENU_NOTCHANGE);
		HistoryMenu.SetDialogMode(DMODE_NODRAWSHADOW);
		HistoryMenu.SetModeMoving(false);
		HistoryMenu.SetFlags(VMENU_SHOWAMPERSAND);
		HistoryMenu.SetBoxType(SHORT_SINGLE_BOX);
//		SetDropDownOpened(TRUE); // Установим флаг "открытия" комбобокса.
		// запомним (для прорисовки)
//		CurItem->ListPtr=&HistoryMenu;
		ret = DlgHist->Select(HistoryMenu, Global->Opt->Dialogs.CBoxMaxHeight, this, strStr);
		// забудим (не нужен)
//		CurItem->ListPtr=nullptr;
//		SetDropDownOpened(FALSE); // Установим флаг "закрытия" комбобокса.
	}

	if (ret > 0)
	{
		EditLine->SetString(strStr);
		EditLine->SetLeftPos(0);
		EditLine->SetClearFlag(0);
		Redraw();
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
/* Private:
   Работа с историей - добавление и reorder списка
*/
int Dialog::AddToEditHistory(DialogItemEx* CurItem, const wchar_t *AddStr)
{
	CriticalSectionLock Lock(CS);

	if (!CurItem->ObjPtr)
	{
		return FALSE;
	}

	History *DlgHist = static_cast<DlgEdit*>(CurItem->ObjPtr)->GetHistory();
	if(DlgHist)
	{
		DlgHist->AddToHistory(AddStr);
	}
	return TRUE;
}

int Dialog::CheckHighlights(WORD CheckSymbol,int StartPos)
{
	CriticalSectionLock Lock(CS);
	FARDIALOGITEMTYPES Type;
	FARDIALOGITEMFLAGS Flags;

	if (StartPos < 0)
		StartPos=0;

	for (size_t I = StartPos; I < Items.size(); ++I)
	{
		Type=Items[I]->Type;
		Flags=Items[I]->Flags;

		if ((!IsEdit(Type) || (Type == DI_COMBOBOX && (Flags&DIF_DROPDOWNLIST))) && !(Flags & (DIF_SHOWAMPERSAND|DIF_DISABLE|DIF_HIDDEN)))
		{
			const wchar_t *ChPtr=wcschr(Items[I]->strData,L'&');

			if (ChPtr)
			{
				WORD Ch=ChPtr[1];

				if (Ch && Upper(CheckSymbol) == Upper(Ch))
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
int Dialog::ProcessHighlighting(int Key,size_t FocusPos,int Translate)
{
	CriticalSectionLock Lock(CS);
	FARDIALOGITEMTYPES Type;
	FARDIALOGITEMFLAGS Flags;

	INPUT_RECORD rec;
	if(!KeyToInputRecord(Key,&rec))
	{
		ClearStruct(rec);
	}

	for (size_t I=0; I<Items.size(); I++)
	{
		Type=Items[I]->Type;
		Flags=Items[I]->Flags;

		if ((!IsEdit(Type) || (Type == DI_COMBOBOX && (Flags&DIF_DROPDOWNLIST))) &&
		        !(Flags & (DIF_SHOWAMPERSAND|DIF_DISABLE|DIF_HIDDEN)))
			if (IsKeyHighlighted(Items[I]->strData,Key,Translate))
			{
				int DisableSelect=FALSE;

				// Если ЭТО: DlgEdit(пред контрол) и DI_TEXT в одну строку, то...
				if (I>0 &&
				        Type==DI_TEXT &&                              // DI_TEXT
				        IsEdit(Items[I-1]->Type) &&                     // и редактор
				        Items[I]->Y1==Items[I-1]->Y1 &&                   // и оба в одну строку
				        (I+1 < Items.size() && Items[I]->Y1!=Items[I+1]->Y1)) // ...и следующий контрол в другой строке
				{
					// Сначала сообщим о случившемся факте процедуре обработки диалога, а потом...
					if (!DlgProc(DN_HOTKEY,I,&rec))
						break; // сказали не продолжать обработку...

					// ... если предыдущий контрол задизаблен или невидим, тогда выходим.
					if ((Items[I-1]->Flags&(DIF_DISABLE|DIF_HIDDEN)) ) // и не задисаблен
						break;

					I=ChangeFocus(I,-1,FALSE);
					DisableSelect=TRUE;
				}
				else if (Items[I]->Type==DI_TEXT      || Items[I]->Type==DI_VTEXT ||
				         Items[I]->Type==DI_SINGLEBOX || Items[I]->Type==DI_DOUBLEBOX)
				{
					if (I < Items.size() - 1) // ...и следующий контрол
					{
						// Сначала сообщим о случившемся факте процедуре обработки диалога, а потом...
						if (!DlgProc(DN_HOTKEY,I,&rec))
							break; // сказали не продолжать обработку...

						// ... если следующий контрол задизаблен или невидим, тогда выходим.
						if ((Items[I+1]->Flags&(DIF_DISABLE|DIF_HIDDEN)) ) // и не задисаблен
							break;

						I=ChangeFocus(I,1,FALSE);
						DisableSelect=TRUE;
					}
				}

				// Сообщим о случивщемся факте процедуре обработки диалога
				if (!DlgProc(DN_HOTKEY,I,&rec))
					break; // сказали не продолжать обработку...

				ChangeFocus2(I);
				ShowDialog();

				if ((Items[I]->Type==DI_CHECKBOX || Items[I]->Type==DI_RADIOBUTTON) &&
				        (!DisableSelect || (Items[I]->Flags & DIF_MOVESELECT)))
				{
					Do_ProcessSpace();
					return TRUE;
				}
				else if (Items[I]->Type==DI_BUTTON)
				{
					ProcessKey(KEY_ENTER, I);
					return TRUE;
				}
				// при ComboBox`е - "вываливаем" последний //????
				else if (Items[I]->Type==DI_COMBOBOX)
				{
					ProcessOpenComboBox(Items[I]->Type,Items[I].get(),I);
					//ProcessKey(KEY_CTRLDOWN);
					return TRUE;
				}

				return TRUE;
			}
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
/*
   функция подравнивания координат edit классов
*/
void Dialog::AdjustEditPos(int dx, int dy)
{
	CriticalSectionLock Lock(CS);
	int x1,x2,y1,y2;

	if (!DialogMode.Check(DMODE_CREATEOBJECTS))
		return;

	ScreenObject *DialogScrObject;

	std::for_each(CONST_RANGE(Items, i)
	{
		FARDIALOGITEMTYPES Type = i->Type;

		if ((i->ObjPtr  && IsEdit(Type)) ||
		        (i->ListPtr && Type == DI_LISTBOX))
		{
			if (Type == DI_LISTBOX)
				DialogScrObject = i->ListPtr;
			else
				DialogScrObject = static_cast<ScreenObject*>(i->ObjPtr);

			DialogScrObject->GetPosition(x1,y1,x2,y2);
			x1+=dx;
			x2+=dx;
			y1+=dy;
			y2+=dy;
			DialogScrObject->SetPosition(x1,y1,x2,y2);
		}
	});

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
   При рассчётах времён копирования проще/надёжнее учитывать время ожидания
   пользовательских ответов в одном месте (здесь).
   Сброс этой переменной должен осуществляться перед общим началом операции
*/
long WaitUserTime;

/* $ 11.08.2000 SVS
   + Для того, чтобы послать DM_CLOSE нужно переопределить Process
*/
void Dialog::Process()
{
//  if(DialogMode.Check(DMODE_SMALLDIALOG))
	SetRestoreScreenMode(TRUE);
	ClearStruct(PrevMouseRecord);
	ClearDone();
	InitDialog();
	TaskBarError *TBE=DialogMode.Check(DMODE_WARNINGSTYLE)?new TaskBarError:nullptr;

	if (ExitCode == -1)
	{
		static LONG in_dialog = -1;
		clock_t btm = 0;
		long    save = 0;
		DialogMode.Set(DMODE_BEGINLOOP);

		if (!InterlockedIncrement(&in_dialog))
		{
			btm = clock();
			save = WaitUserTime;
			WaitUserTime = -1;
		}

		FrameManager->ExecuteModal(this);
		save += (clock() - btm);

		if (InterlockedDecrement(&in_dialog) == -1)
			WaitUserTime = save;
	}

	if (SavedItems)
		for (unsigned i = 0; i < Items.size(); i++)
			SavedItems[i] = *Items[i];

	if (TBE)
	{
		delete TBE;
	}
}

intptr_t Dialog::CloseDialog()
{
	CriticalSectionLock Lock(CS);
	GetDialogObjectsData();

	intptr_t result=DlgProc(DN_CLOSE,ExitCode,0);
	if (result)
	{
		DialogMode.Set(DMODE_ENDLOOP);
		Hide();

		if (DialogMode.Check(DMODE_BEGINLOOP) && (DialogMode.Check(DMODE_MSGINTERNAL) || FrameManager->ManagerStarted()))
		{
			DialogMode.Clear(DMODE_BEGINLOOP);
			FrameManager->DeleteFrame(this);
			FrameManager->PluginCommit();
		}

		_DIALOG(CleverSysLog CL(L"Close Dialog"));
	}
	return result;
}


/* $ 17.05.2001 DJ
   установка help topic'а и прочие радости, временно перетащенные сюда
   из Modal
*/
void Dialog::SetHelp(const wchar_t *Topic)
{
	CriticalSectionLock Lock(CS);

	HelpTopic.Clear();

	if (Topic && *Topic)
	{
		HelpTopic = Topic;
	}
}

void Dialog::ShowHelp()
{
	CriticalSectionLock Lock(CS);

	if (!HelpTopic.IsEmpty())
	{
		Help Hlp(HelpTopic);
	}
}

void Dialog::ClearDone()
{
	CriticalSectionLock Lock(CS);
	ExitCode=-1;
	DialogMode.Clear(DMODE_ENDLOOP);
}

void Dialog::SetExitCode(int Code)
{
	CriticalSectionLock Lock(CS);
	ExitCode=Code;
	DialogMode.Set(DMODE_ENDLOOP);
	//CloseDialog();
}


/* $ 19.05.2001 DJ
   возвращаем наше название для меню по F12
*/
int Dialog::GetTypeAndName(string &strType, string &strName)
{
	CriticalSectionLock Lock(CS);
	strType = MSG(MDialogType);
	strName.Clear();
	const wchar_t *lpwszTitle = GetDialogTitle();

	if (lpwszTitle)
		strName = lpwszTitle;

	return MODALTYPE_DIALOG;
}


MACROMODEAREA Dialog::GetMacroMode()
{
	return MACRO_DIALOG;
}

int Dialog::FastHide()
{
	return Global->Opt->AllCtrlAltShiftRule & CASR_DIALOG;
}

void Dialog::ResizeConsole()
{
	CriticalSectionLock Lock(CS);

	DialogMode.Set(DMODE_RESIZED);

	if (IsVisible())
	{
		Hide();
	}

	COORD c = {static_cast<SHORT>(ScrX+1), static_cast<SHORT>(ScrY+1)};
	SendMessage(DN_RESIZECONSOLE, 0, &c);

	int x1, y1, x2, y2;
	GetPosition(x1, y1, x2, y2);
	c.X = std::min(x1, ScrX-1);
	c.Y = std::min(y1, ScrY-1);
	if(c.X!=x1 || c.Y!=y1)
	{
		c.X = x1;
		c.Y = y1;
		SendMessage(DM_MOVEDIALOG, TRUE, &c);
		SetComboBoxPos();
	}
};

//void Dialog::OnDestroy()
//{
//  /* $ 21.04.2002 KM
//  //  Эта функция потеряла своё значение при текущем менеджере
//  //  и системе создания и уничтожения фреймов.
//  if(DialogMode.Check(DMODE_RESIZED))
//  {
//    Frame *BFrame=FrameManager->GetBottomFrame();
//    if(BFrame)
//      BFrame->UnlockRefresh();
//    /* $ 21.04.2002 KM
//        А вот этот DM_KILLSAVESCREEN здесь только вредит. Удаление
//        диалога происходит без восстановления ShadowSaveScr и вот
//        они: "артефакты" непрорисовки.
//    */
//		SendDlgMessage(this,DM_KILLSAVESCREEN,0,0);
//  }
//};

intptr_t Dialog::DlgProc(intptr_t Msg,intptr_t Param1,void* Param2)
{
	if (DialogMode.Check(DMODE_ENDLOOP))
		return 0;

	intptr_t Result;
	FarDialogEvent de={sizeof(FarDialogEvent),this,Msg,Param1,Param2,0};

	if(!CheckDialogMode(DMODE_NOPLUGINS))
	{
		if (Global->CtrlObject->Plugins->ProcessDialogEvent(DE_DLGPROCINIT,&de))
			return de.Result;
	}
	if (OwnerClass)
		Result=(OwnerClass->*DialogHandler)(this,Msg,Param1,Param2);
	else
		Result = RealDlgProc(this,Msg,Param1,Param2);
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
	CriticalSectionLock Lock(CS);
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
		case DN_DRAWDIALOGDONE:
		{
			if (Param1 == 1) // Нужно отрисовать "салазки"?
			{
				/* $ 03.08.2000 tran
				   вывод текста в углу может приводить к ошибкам изображения
				   1) когда диалог перемещается в угол
				   2) когда диалог перемещается из угла
				   сделал вывод красных палочек по углам */
				FarColor Color;
				Colors::ConsoleColorToFarColor(0xCE, Color);
				Text(X1, Y1, Color, L"\\");
				Text(X1, Y2, Color, L"/");
				Text(X2, Y1, Color, L"/");
				Text(X2, Y2, Color, L"\\");
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
		case DM_GETDIALOGINFO:
		{
			bool Result=false;

			if (Param2)
			{
				if (IdExist)
				{
					DialogInfo *di=reinterpret_cast<DialogInfo*>(Param2);

					if (CheckStructSize(di))
					{
						di->Id=Id;
						di->Owner=FarGuid;
						Result=true;
						if (PluginOwner)
						{
							di->Owner = PluginOwner->GetGUID();
						}
					}
				}
			}

			return Result;
		}
		default:
			break;
	}

	// предварительно проверим...
	if (!Items.empty() && static_cast<size_t>(Param1) >= Items.size())
		return 0;

	if (Param1>=0)
	{
		CurItem=Items[Param1].get();
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
			return ((Type==DI_BUTTON && !(CurItem->Flags&DIF_BTNNOCLOSE))?FALSE:TRUE);
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
	CriticalSectionLock Lock(CS);
	_DIALOG(CleverSysLog CL(L"Dialog.SendDlgMessage()"));
	_DIALOG(SysLog(L"hDlg=%p, Msg=%s, Param1=%d (0x%08X), Param2=%d (0x%08X)",this,_DLGMSG_ToName(Msg),Param1,Param1,Param2,Param2));

	// Сообщения, касаемые только диалога и не затрагивающие элементы
	switch (Msg)
	{
			/*****************************************************************/
		case DM_RESIZEDIALOG:
			// изменим вызов RESIZE.
			Param1=-1;
			/*****************************************************************/
		case DM_MOVEDIALOG:
		{
			int W1,H1;
			W1=X2-X1+1;
			H1=Y2-Y1+1;
			OldX1=X1;
			OldY1=Y1;
			OldX2=X2;
			OldY2=Y2;

			// переместили
			if (Param1>0)  // абсолютно?
			{
				X1=((COORD*)Param2)->X;
				Y1=((COORD*)Param2)->Y;
				X2=W1;
				Y2=H1;
				CheckDialogCoord();
			}
			else if (!Param1)  // значит относительно
			{
				X1+=((COORD*)Param2)->X;
				Y1+=((COORD*)Param2)->Y;
			}
			else // Resize, Param2=width/height
			{
				int OldW1,OldH1;
				OldW1=W1;
				OldH1=H1;
				W1=((COORD*)Param2)->X;
				H1=((COORD*)Param2)->Y;
				RealWidth = W1;
				RealHeight = H1;

				if (W1<OldW1 || H1<OldH1)
				{
					DialogMode.Set(DMODE_DRAWING);
					DialogItemEx *Item;
					SMALL_RECT Rect;

					for (size_t I=0; I < Items.size(); I++)
					{
						Item=this->Items[I].get();

						if (Item->Flags&DIF_HIDDEN)
							continue;

						Rect.Left=Item->X1;
						Rect.Top=Item->Y1;

						if (Item->X2>=W1)
						{
							Rect.Right=Item->X2-(OldW1-W1);
							Rect.Bottom=Item->Y2;
							SetItemRect(I,&Rect);
						}

						if (Item->Y2>=H1)
						{
							Rect.Right=Item->X2;
							Rect.Bottom=Item->Y2-(OldH1-H1);
							SetItemRect(I,&Rect);
						}
					}

					DialogMode.Clear(DMODE_DRAWING);
				}
			}

			// проверили и скорректировали
			if (X1+W1<0)
				X1=-W1+1;

			if (Y1+H1<0)
				Y1=-H1+1;

			if (X1>ScrX)
				X1=ScrX;

			if (Y1>ScrY)
				Y1=ScrY;

			X2=X1+W1-1;
			Y2=Y1+H1-1;

			if (Param1>0)  // абсолютно?
			{
				CheckDialogCoord();
			}

			if (Param1 < 0)  // размер?
			{
				((COORD*)Param2)->X=X2-X1+1;
				((COORD*)Param2)->Y=Y2-Y1+1;
			}
			else
			{
				((COORD*)Param2)->X=X1;
				((COORD*)Param2)->Y=Y1;
			}

			int I=IsVisible();// && DialogMode.Check(DMODE_INITOBJECTS);

			if (I) Hide();

			// приняли.
			AdjustEditPos(X1-OldX1,Y1-OldY1);

			if (I) Show(); // только если диалог был виден

			return reinterpret_cast<intptr_t>(Param2);
		}
		/*****************************************************************/
		case DM_REDRAW:
		{
			if (DialogMode.Check(DMODE_INITOBJECTS))
				Show();

			return 0;
		}
		/*****************************************************************/
		case DM_ENABLEREDRAW:
		{
			int Prev=IsEnableRedraw;

			if (Param1 == TRUE)
				IsEnableRedraw++;
			else if (Param1 == FALSE)
				IsEnableRedraw--;

			//Edit::DisableEditOut(!IsEnableRedraw?FALSE:TRUE);

			if (!IsEnableRedraw && Prev != IsEnableRedraw)
				if (DialogMode.Check(DMODE_INITOBJECTS))
				{
					ShowDialog();
//          Show();
					Global->ScrBuf->Flush();
				}

			return Prev;
		}
		/*
		    case DM_ENABLEREDRAW:
		    {
		      if(Param1)
		        IsEnableRedraw++;
		      else
		        IsEnableRedraw--;

		      if(!IsEnableRedraw)
		        if(DialogMode.Check(DMODE_INITOBJECTS))
		        {
		          ShowDialog();
		          Global->ScrBuf->Flush();
		//          Show();
		        }
		      return 0;
		    }
		*/
		/*****************************************************************/
		case DM_SHOWDIALOG:
		{
//      if(!IsEnableRedraw)
			{
				if (Param1)
				{
					/* $ 20.04.2002 KM
					  Залочим прорисовку при прятании диалога, в противном
					  случае ОТКУДА менеджер узнает, что отрисовывать
					  объект нельзя!
					*/
					if (!IsVisible())
					{
						Unlock();
						Show();
					}
				}
				else
				{
					if (IsVisible())
					{
						Hide();
						this->Lock();
					}
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
			const INPUT_RECORD *KeyArray=(const INPUT_RECORD *)Param2;
			DialogMode.Set(DMODE_KEY);

			for (unsigned int I=0; I < (size_t)Param1; ++I)
				ProcessKey(InputRecordToKey(KeyArray+I));

			DialogMode.Clear(DMODE_KEY);
			return 0;
		}
		/*****************************************************************/
		case DM_CLOSE:
		{
			if (Param1 == -1)
				ExitCode=static_cast<int>(FocusPos);
			else
				ExitCode=Param1;

			return CloseDialog();
		}
		/*****************************************************************/
		case DM_GETDLGRECT:
		{
			if (Param2)
			{
				int x1,y1,x2,y2;
				GetPosition(x1,y1,x2,y2);
				((SMALL_RECT*)Param2)->Left=x1;
				((SMALL_RECT*)Param2)->Top=y1;
				((SMALL_RECT*)Param2)->Right=x2;
				((SMALL_RECT*)Param2)->Bottom=y2;
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

			BOOL OldIsProcessAssignMacroKey=Global->IsProcessAssignMacroKey;
			Global->IsProcessAssignMacroKey=Param1;
			return OldIsProcessAssignMacroKey;
		}
		/*****************************************************************/
		case DM_SETMOUSEEVENTNOTIFY: // Param1 = 1 on, 0 off, -1 - get
		{
			int State=DialogMode.Check(DMODE_MOUSEEVENT)?TRUE:FALSE;

			if (Param1 != -1)
			{
				if (!Param1)
					DialogMode.Clear(DMODE_MOUSEEVENT);
				else
					DialogMode.Set(DMODE_MOUSEEVENT);
			}

			return State;
		}
		/*****************************************************************/
		case DN_RESIZECONSOLE:
		{
			return DlgProc(Msg,Param1,Param2);
		}
		case DM_GETDIALOGINFO:
		{
			return DlgProc(DM_GETDIALOGINFO,Param1,Param2);
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
	DialogItemEx *CurItem=nullptr;
	FARDIALOGITEMTYPES Type=DI_TEXT;
	size_t Len=0;

	// предварительно проверим...
	/* $ 09.12.2001 DJ
	   для DM_USER проверять _не_надо_!
	*/
	if (static_cast<size_t>(Param1) >= Items.size() || Items.empty())
		return 0;

//  CurItem=&Items[Param1];
	CurItem=Items[Param1].get();
	Type=CurItem->Type;
	const wchar_t *Ptr= CurItem->strData;

	if (IsEdit(Type) && CurItem->ObjPtr)
		Ptr=const_cast <const wchar_t *>(((DlgEdit *)(CurItem->ObjPtr))->GetStringAddr());

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
				VMenu *ListBox=CurItem->ListPtr;

				if (ListBox)
				{
					intptr_t Ret=TRUE;

					switch (Msg)
					{
						case DM_LISTINFO:// Param1=ID Param2=FarListInfo
						{
							FarListInfo* li=static_cast<FarListInfo*>(Param2);
							return CheckStructSize(li)&&ListBox->GetVMenuInfo(li);
						}
						case DM_LISTSORT: // Param1=ID Param=Direct {0|1}
						{
							ListBox->SortItems(Param2 != nullptr);
							break;
						}
						case DM_LISTFINDSTRING: // Param1=ID Param2=FarListFind
						{
							FarListFind* lf=reinterpret_cast<FarListFind*>(Param2);
							return CheckStructSize(lf)?ListBox->FindItem(lf->StartIndex,lf->Pattern,lf->Flags):-1;
						}
						case DM_LISTADDSTR: // Param1=ID Param2=String
						{
							Ret=ListBox->AddItem((wchar_t*)Param2);
							break;
						}
						case DM_LISTADD: // Param1=ID Param2=FarList: ItemsNumber=Count, Items=Src
						{
							FarList *ListItems=(FarList *)Param2;

							if (!CheckStructSize(ListItems))
								return FALSE;

							Ret=ListBox->AddItem(ListItems);
							break;
						}
						case DM_LISTDELETE: // Param1=ID Param2=FarListDelete: StartIndex=BeginIndex, Count=количество (<=0 - все!)
						{
							FarListDelete *ListItems=(FarListDelete *)Param2;
							if(CheckNullOrStructSize(ListItems))
							{
								int Count;
								if (!ListItems || (Count=ListItems->Count) <= 0)
									ListBox->DeleteItems();
								else
									ListBox->DeleteItem(ListItems->StartIndex,Count);
							}
							else return FALSE;

							break;
						}
						case DM_LISTINSERT: // Param1=ID Param2=FarListInsert
						{
							FarListInsert* li=static_cast<FarListInsert *>(Param2);
							if (!CheckStructSize(li) || (Ret=ListBox->InsertItem((FarListInsert *)Param2)) == -1)
								return -1;

							break;
						}
						case DM_LISTUPDATE: // Param1=ID Param2=FarListUpdate: Index=Index, Items=Src
						{
							FarListUpdate* lu=static_cast<FarListUpdate *>(Param2);
							if (CheckStructSize(lu) && ListBox->UpdateItem(lu))
								break;

							return FALSE;
						}
						case DM_LISTGETITEM: // Param1=ID Param2=FarListGetItem: ItemsNumber=Index, Items=Dest
						{
							FarListGetItem *ListItems=(FarListGetItem *)Param2;

							if (!CheckStructSize(ListItems))
								return FALSE;

							MenuItemEx *ListMenuItem;

							if ((ListMenuItem=ListBox->GetItemPtr(ListItems->ItemIndex)) )
							{
								//ListItems->ItemIndex=1;
								FarListItem *Item=&ListItems->Item;
								ClearStruct(*Item);
								Item->Flags=ListMenuItem->Flags;
								Item->Text=ListMenuItem->strName;
								/*
								if(ListMenuItem->UserDataSize <= sizeof(DWORD)) //???
								   Item->UserData=ListMenuItem->UserData;
								*/
								return TRUE;
							}

							return FALSE;
						}
						case DM_LISTGETDATA: // Param1=ID Param2=Index
						{
							if (reinterpret_cast<intptr_t>(Param2) < ListBox->GetItemCount())
								return (intptr_t)ListBox->GetUserData(nullptr,0,static_cast<int>(reinterpret_cast<intptr_t>(Param2)));

							return 0;
						}
						case DM_LISTGETDATASIZE: // Param1=ID Param2=Index
						{
							if (reinterpret_cast<intptr_t>(Param2) < ListBox->GetItemCount())
								return ListBox->GetUserDataSize(static_cast<int>(reinterpret_cast<intptr_t>(Param2)));

							return 0;
						}
						case DM_LISTSETDATA: // Param1=ID Param2=FarListItemData
						{
							FarListItemData *ListItems=(FarListItemData *)Param2;

							if (CheckStructSize(ListItems) &&
							        ListItems->Index < ListBox->GetItemCount())
							{
								Ret=ListBox->SetUserData(ListItems->Data,
								                         ListItems->DataSize,
								                         ListItems->Index);
								return Ret;
							}

							return 0;
						}
						/* $ 02.12.2001 KM
						   + Сообщение для добавления в список строк, с удалением
						     уже существующих, т.с. "чистая" установка
						*/
						case DM_LISTSET: // Param1=ID Param2=FarList: ItemsNumber=Count, Items=Src
						{
							FarList *ListItems=(FarList *)Param2;

							if (!CheckStructSize(ListItems))
								return FALSE;

							ListBox->DeleteItems();
							Ret=ListBox->AddItem(ListItems);
							break;
						}
						//case DM_LISTINS: // Param1=ID Param2=FarList: ItemsNumber=Index, Items=Dest
						case DM_LISTSETTITLES: // Param1=ID Param2=FarListTitles
						{
							FarListTitles *ListTitle=(FarListTitles *)Param2;
							if(CheckStructSize(ListTitle))
							{
								ListBox->SetTitle(ListTitle->Title);
								ListBox->SetBottomTitle(ListTitle->Bottom);
								break;   //return TRUE;
							}
							return FALSE;
						}
						case DM_LISTGETTITLES: // Param1=ID Param2=FarListTitles
						{

							FarListTitles *ListTitle=(FarListTitles *)Param2;
							string strTitle,strBottomTitle;
							ListBox->GetTitle(strTitle);
							ListBox->GetBottomTitle(strBottomTitle);

							if (CheckStructSize(ListTitle)&&(!strTitle.IsEmpty()||!strBottomTitle.IsEmpty()))
							{
								if (ListTitle->Title&&ListTitle->TitleSize)
									xwcsncpy((wchar_t*)ListTitle->Title,strTitle,ListTitle->TitleSize);
								else
									ListTitle->TitleSize=strTitle.GetLength()+1;

								if (ListTitle->Bottom&&ListTitle->BottomSize)
									xwcsncpy((wchar_t*)ListTitle->Bottom,strBottomTitle,ListTitle->BottomSize);
								else
									ListTitle->BottomSize=strBottomTitle.GetLength()+1;
								return TRUE;
							}
							return FALSE;
						}
						case DM_LISTGETCURPOS: // Param1=ID Param2=FarListPos
						{
							FarListPos* lp=static_cast<FarListPos *>(Param2);
							return CheckStructSize(lp)?ListBox->GetSelectPos(lp):ListBox->GetSelectPos();
						}
						case DM_LISTSETCURPOS: // Param1=ID Param2=FarListPos Ret: RealPos
						{
							FarListPos* lp=static_cast<FarListPos *>(Param2);
							if(CheckStructSize(lp))
							{
								/* 26.06.2001 KM Подадим перед изменением позиции об этом сообщение */
								int CurListPos=ListBox->GetSelectPos();
								Ret=ListBox->SetSelectPos((FarListPos *)Param2);

								if (Ret!=CurListPos)
									if (!DlgProc(DN_LISTCHANGE,Param1,ToPtr(Ret)))
										Ret=ListBox->SetSelectPos(CurListPos,1);
							}
							else return -1;
							break; // т.к. нужно перерисовать!
						}
						case DM_GETCOMBOBOXEVENT: // Param1=ID Param2=0 Ret=Sets
						{
							return (CurItem->IFlags.Check(DLGIIF_COMBOBOXEVENTKEY)?CBET_KEY:0)|(CurItem->IFlags.Check(DLGIIF_COMBOBOXEVENTMOUSE)?CBET_MOUSE:0);
						}
						case DM_SETCOMBOBOXEVENT: // Param1=ID Param2=FARCOMBOBOXEVENTTYPE Ret=OldSets
						{
							int OldSets=CurItem->IFlags.Flags();
							CurItem->IFlags.Clear(DLGIIF_COMBOBOXEVENTKEY|DLGIIF_COMBOBOXEVENTMOUSE);

							if (reinterpret_cast<intptr_t>(Param2)&CBET_KEY)
								CurItem->IFlags.Set(DLGIIF_COMBOBOXEVENTKEY);

							if (reinterpret_cast<intptr_t>(Param2)&CBET_MOUSE)
								CurItem->IFlags.Set(DLGIIF_COMBOBOXEVENTMOUSE);

							return OldSets;
						}
						default:
							break;
					}

					// уточнение для DI_COMBOBOX - здесь еще и DlgEdit нужно корректно заполнить
					if (!CurItem->IFlags.Check(DLGIIF_COMBOBOXNOREDRAWEDIT) && Type==DI_COMBOBOX && CurItem->ObjPtr)
					{
						MenuItemEx *ListMenuItem;

						if ((ListMenuItem=ListBox->GetItemPtr(ListBox->GetSelectPos())) )
						{
							if (CurItem->Flags & (DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND))
								((DlgEdit *)(CurItem->ObjPtr))->SetHiString(ListMenuItem->strName);
							else
								((DlgEdit *)(CurItem->ObjPtr))->SetString(ListMenuItem->strName);

							((DlgEdit *)(CurItem->ObjPtr))->Select(-1,-1); // снимаем выделение
						}
					}

					if (DialogMode.Check(DMODE_SHOW) && ListBox->UpdateRequired())
					{
						ShowDialog(Param1);
						Global->ScrBuf->Flush();
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
				if (Param2 && *(const wchar_t *)Param2)
				{
					CurItem->Flags|=DIF_HISTORY;
					CurItem->strHistory=(const wchar_t *)Param2;
					static_cast<DlgEdit*>(CurItem->ObjPtr)->SetHistory(CurItem->strHistory);
					if (Type==DI_EDIT && (CurItem->Flags&DIF_USELASTHISTORY))
					{
						ProcessLastHistory(CurItem, Param1);
					}
				}
				else
				{
					CurItem->Flags&=~DIF_HISTORY;
					CurItem->strHistory.Clear();
				}

				if (DialogMode.Check(DMODE_SHOW))
				{
					ShowDialog(Param1);
					Global->ScrBuf->Flush();
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
				return AddToEditHistory(CurItem, (const wchar_t*)Param2);
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
				((COORD*)Param2)->X=((DlgEdit *)(CurItem->ObjPtr))->GetCurPos();
				((COORD*)Param2)->Y=0;
				return TRUE;
			}
			else if (Type == DI_USERCONTROL && CurItem->UCData)
			{
				((COORD*)Param2)->X=CurItem->UCData->CursorPos.X;
				((COORD*)Param2)->Y=CurItem->UCData->CursorPos.Y;
				return TRUE;
			}

			return FALSE;
		}
		/*****************************************************************/
		case DM_SETCURSORPOS:
		{
			if (IsEdit(Type) && CurItem->ObjPtr && ((COORD*)Param2)->X >= 0)
			{
				DlgEdit *EditPtr=(DlgEdit *)(CurItem->ObjPtr);
				EditPtr->SetCurPos(((COORD*)Param2)->X);
				//EditPtr->Show();
				ShowDialog(Param1);
				return TRUE;
			}
			else if (Type == DI_USERCONTROL && CurItem->UCData)
			{
				// учтем, что координаты для этого элемента всегда относительные!
				//  и начинаются с 0,0
				COORD Coord=*(COORD*)Param2;
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
				if (DialogMode.Check(DMODE_SHOW) && FocusPos == (size_t)Param1)
				{
					// что-то одно надо убрать :-)
					MoveCursor(Coord.X+X1,Coord.Y+Y1); // ???
					ShowDialog(Param1); //???
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
					EditorSetPosition *esp=(EditorSetPosition *)Param2;
					if (CheckStructSize(esp))
					{
						DlgEdit *EditPtr=(DlgEdit *)(CurItem->ObjPtr);
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
					EditorSetPosition *esp=(EditorSetPosition *)Param2;
					if (CheckStructSize(esp))
					{
						DlgEdit *EditPtr=(DlgEdit *)(CurItem->ObjPtr);
						if(esp->CurPos>=0)
							EditPtr->SetCurPos(esp->CurPos);
						if(esp->CurTabPos>=0)
							EditPtr->SetTabCurPos(esp->CurTabPos);
						if(esp->LeftPos>=0)
							EditPtr->SetLeftPos(esp->LeftPos);
						if(esp->Overtype>=0)
							EditPtr->SetOvertypeMode(esp->Overtype!=0);
						ShowDialog(Param1);
						Global->ScrBuf->Flush();
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
				bool Visible;
				DWORD Size;
				((DlgEdit *)(CurItem->ObjPtr))->GetCursorType(Visible,Size);
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
			bool Visible=0;
			DWORD Size=0;

			if (IsEdit(Type) && CurItem->ObjPtr)
			{
				((DlgEdit *)(CurItem->ObjPtr))->GetCursorType(Visible,Size);
				((DlgEdit *)(CurItem->ObjPtr))->SetCursorType(LOWORD(Param2)!=0,HIWORD(Param2));
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
				        FocusPos == (size_t)Param1 &&
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
			Item.Item=(FarDialogItem*)xf_malloc(Item.Size);
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
			xf_free(Item.Item);
			return I;
		}
		/*****************************************************************/
		case DN_BTNCLICK:
		{
			intptr_t Ret=DlgProc(Msg,Param1,Param2);

			if (Ret && (CurItem->Flags&DIF_AUTOMATION) && !CurItem->Auto.empty())
			{
				intptr_t iParam = reinterpret_cast<intptr_t>(Param2);
				iParam%=3;

				std::for_each(RANGE(CurItem->Auto, i)
				{
					FARDIALOGITEMFLAGS NewFlags=Items[i.ID]->Flags;
					Items[i.ID]->Flags=(NewFlags&(~i.Flags[iParam][1]))|i.Flags[iParam][0];
					// здесь намеренно в обработчик не посылаются эвенты об изменении
					// состояния...
				});
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
				int OldState=CurItem->Flags&DIF_3STATE?TRUE:FALSE;

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
				intptr_t State = reinterpret_cast<intptr_t>(Param2);
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
						std::for_each(RANGE(CurItem->Auto, i)
						{
							FARDIALOGITEMFLAGS NewFlags = Items[i.ID]->Flags;
							Items[i.ID]->Flags=(NewFlags&(~i.Flags[State][1]))|i.Flags[State][0];
							// здесь намеренно в обработчик не посылаются эвенты об изменении
							// состояния...
						});
						Param1=-1;
					}

					ShowDialog(Param1);
					Global->ScrBuf->Flush();
				}

				return Selected;
			}
			else if (Type == DI_RADIOBUTTON)
			{
				Param1=ProcessRadioButton(Param1);

				if (DialogMode.Check(DMODE_SHOW))
				{
					ShowDialog();
					Global->ScrBuf->Flush();
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
			Item.Item=(FarDialogItem*)xf_malloc(Item.Size);
			intptr_t I=FALSE;
			if(ConvertItemEx2(CurItem,&Item)<=Item.Size)
			{
				I=DlgProc(Msg,Param1,Item.Item);

				if ((Type == DI_LISTBOX || Type == DI_COMBOBOX) && CurItem->ListPtr)
					CurItem->ListPtr->ChangeFlags(VMENU_DISABLED, (CurItem->Flags&DIF_DISABLE)!=0);
			}
			xf_free(Item.Item);
			return I;
		}
		/*****************************************************************/
		case DM_SETFOCUS:
		{
			if (!CanGetFocus(Type))
				return FALSE;

			if (FocusPos == (size_t)Param1) // уже и так установлено все!
				return TRUE;

			ChangeFocus2(Param1);

			if (FocusPos == (size_t)Param1)
			{
				ShowDialog();
				return TRUE;
			}

			return FALSE;
		}
		/*****************************************************************/
		case DM_GETFOCUS: // Получить ID фокуса
		{
			return FocusPos;
		}
		/*****************************************************************/
		case DM_GETCONSTTEXTPTR:
		{
			return (intptr_t)Ptr;
		}
		/*****************************************************************/
		case DM_GETTEXT:
		{
			FarDialogItemData *did=(FarDialogItemData*)Param2;
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

						if (!CurItem->ObjPtr)
							break;

						Ptr=const_cast <const wchar_t *>(((DlgEdit *)(CurItem->ObjPtr))->GetStringAddr());
					case DI_TEXT:
					case DI_VTEXT:
					case DI_SINGLEBOX:
					case DI_DOUBLEBOX:
					case DI_CHECKBOX:
					case DI_RADIOBUTTON:
					case DI_BUTTON:
						Len=StrLength(Ptr);

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

						if (!did->PtrLength)
							did->PtrLength=Len;
						else if (Len > did->PtrLength)
							Len=did->PtrLength;

						if (did->PtrData)
						{
							wmemmove(did->PtrData,Ptr,Len);
							did->PtrData[Len]=L'\0';
						}

						break;
					case DI_USERCONTROL:
						/*did->PtrLength=CurItem->Ptr.PtrLength; BUGBUG
						did->PtrData=(char*)CurItem->Ptr.PtrData;*/
						break;
					case DI_LISTBOX:
					{
//            if(!CurItem->ListPtr)
//              break;
//            did->PtrLength=CurItem->ListPtr->GetUserData(did->PtrData,did->PtrLength,-1);
						break;
					}
					default:  // подразумеваем, что остались
						did->PtrLength=0;
						break;
				}

				return Len;
			}

			//получаем размер
			switch (Type)
			{
				case DI_BUTTON:
					Len=StrLength(Ptr)+1;

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
					Len=StrLength(Ptr)+1;
					break;
				case DI_COMBOBOX:
				case DI_EDIT:
				case DI_PSWEDIT:
				case DI_FIXEDIT:
				case DI_MEMOEDIT:

					if (CurItem->ObjPtr)
					{
						Len=((DlgEdit *)(CurItem->ObjPtr))->GetLength()+1;
						break;
					}

				case DI_LISTBOX:
				{
					Len=0;
					MenuItemEx *ListMenuItem;

					if ((ListMenuItem=CurItem->ListPtr->GetItemPtr(-1)) )
					{
						Len=(int)ListMenuItem->strName.GetLength()+1;
					}

					break;
				}
				default:
					Len=0;
					break;
			}

			return Len-(!Len?0:1);
		}
		/*****************************************************************/
		case DM_SETTEXTPTR:
		{
			wchar_t* Text = Param2?static_cast<wchar_t*>(Param2):const_cast<wchar_t*>(L"");
			FarDialogItemData IData={sizeof(FarDialogItemData),(size_t)StrLength(Text),Text};
			return SendMessage(DM_SETTEXT,Param1,&IData);
		}
		/*****************************************************************/
		case DM_SETTEXT:
		{
			FarDialogItemData *did=(FarDialogItemData*)Param2;
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
					case DI_LISTBOX: // меняет только текущий итем
						CurItem->strData = did->PtrData;
						Len = CurItem->strData.GetLength();
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
							if (!DialogMode.Check(DMODE_KEEPCONSOLETITLE))
								ConsoleTitle::SetFarTitle(GetDialogTitle());
							ShowDialog(Param1);
							Global->ScrBuf->Flush();
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
							DlgEdit *EditLine=(DlgEdit *)(CurItem->ObjPtr);
							bool ReadOnly=EditLine->GetReadOnly();
							EditLine->SetReadOnly(0);
							{
								SetAutocomplete da(EditLine);
								EditLine->SetString(CurItem->strData);
							}
							EditLine->SetReadOnly(ReadOnly);

							if (DialogMode.Check(DMODE_INITOBJECTS)) // не меняем клеар-флаг, пока не проиницализировались
								EditLine->SetClearFlag(0);

							EditLine->Select(-1,0); // снимаем выделение
							// ...оно уже снимается в DlgEdit::SetString()
						}

						break;
					case DI_LISTBOX: // меняет только текущий итем
					{
						VMenu *ListBox=CurItem->ListPtr;

						if (ListBox)
						{
							FarListUpdate LUpdate={sizeof(FarListUpdate)};
							LUpdate.Index=ListBox->GetSelectPos();
							MenuItemEx *ListMenuItem=ListBox->GetItemPtr(LUpdate.Index);

							if (ListMenuItem)
							{
								LUpdate.Item.Flags=ListMenuItem->Flags;
								LUpdate.Item.Text=Ptr;
								SendMessage(DM_LISTUPDATE,Param1,&LUpdate);
							}

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
					ShowDialog(Param1);
					Global->ScrBuf->Flush();
				}

				//CurItem->strData = did->PtrData;
				return CurItem->strData.GetLength(); //???
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
				int MaxLen=((DlgEdit *)(CurItem->ObjPtr))->GetMaxLength();
				// BugZ#628 - Неправильная длина редактируемого текста.
				((DlgEdit *)(CurItem->ObjPtr))->SetMaxLength(static_cast<int>(reinterpret_cast<intptr_t>(Param2)));
				//if (DialogMode.Check(DMODE_INITOBJECTS)) //???
				InitDialogObjects(Param1); // переинициализируем элементы диалога
				if (!DialogMode.Check(DMODE_KEEPCONSOLETITLE))
					ConsoleTitle::SetFarTitle(GetDialogTitle());
				return MaxLen;
			}

			return 0;
		}
		/*****************************************************************/
		case DM_GETDLGITEM:
		{
			FarGetDialogItem* Item = (FarGetDialogItem*)Param2;
			return (CheckNullOrStructSize(Item))?(intptr_t)ConvertItemEx2(CurItem, Item):0;
		}
		/*****************************************************************/
		case DM_GETDLGITEMSHORT:
		{
			if (Param2 && ConvertItemEx(CVTITEM_TOPLUGINSHORT,(FarDialogItem *)Param2,CurItem,1))
				return TRUE;
			return FALSE;
		}
		/*****************************************************************/
		case DM_SETDLGITEM:
		case DM_SETDLGITEMSHORT:
		{
			if (!Param2)
				return FALSE;

			if (Type != ((FarDialogItem *)Param2)->Type) // пока нефига менять тип
				return FALSE;

			// не менять
			if (!ConvertItemEx((Msg==DM_SETDLGITEM)?CVTITEM_FROMPLUGIN:CVTITEM_FROMPLUGINSHORT,(FarDialogItem *)Param2,CurItem,1))
				return FALSE; // invalid parameters

			CurItem->Type=Type;

			if ((Type == DI_LISTBOX || Type == DI_COMBOBOX) && CurItem->ListPtr)
				CurItem->ListPtr->ChangeFlags(VMENU_DISABLED, (CurItem->Flags&DIF_DISABLE)!=0);

			// еще разок, т.к. данные могли быть изменены
			InitDialogObjects(Param1);
			if (!DialogMode.Check(DMODE_KEEPCONSOLETITLE))
				ConsoleTitle::SetFarTitle(GetDialogTitle());

			if (DialogMode.Check(DMODE_SHOW))
			{
				ShowDialog(Param1);
				Global->ScrBuf->Flush();
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
					if ((CurItem->Flags&DIF_HIDDEN) && FocusPos == (size_t)Param1)
					{
						ChangeFocus2(ChangeFocus(Param1,1,TRUE));
					}

					// Либо все,  либо... только 1
					ShowDialog(GetDropDownOpened()||(CurItem->Flags&DIF_HIDDEN)?-1:Param1);
					Global->ScrBuf->Flush();
				}
			}

			return (PrevFlags&DIF_HIDDEN)?FALSE:TRUE;
		}
		/*****************************************************************/
		case DM_SETDROPDOWNOPENED: // Param1=ID; Param2={TRUE|FALSE}
		{
			if (!Param2) // Закрываем любой открытый комбобокс или историю
			{
				if (GetDropDownOpened())
				{
					SetDropDownOpened(FALSE);
					Sleep(10);
				}

				return TRUE;
			}
			/* $ 09.12.2001 DJ
			   у DI_PSWEDIT не бывает хистори!
			*/
			else if (Param2 && (Type==DI_COMBOBOX || ((Type==DI_EDIT || Type==DI_FIXEDIT)
			                    && (CurItem->Flags&DIF_HISTORY)))) /* DJ $ */
			{
				// Открываем заданный в Param1 комбобокс или историю
				if (GetDropDownOpened())
				{
					SetDropDownOpened(FALSE);
					Sleep(10);
				}

				if (SendMessage(DM_SETFOCUS,Param1,0))
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
			return SetItemRect((int)Param1,(SMALL_RECT*)Param2);
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
				ShowDialog(Param1);
				Global->ScrBuf->Flush();
			}

			return (PrevFlags&DIF_DISABLE)?FALSE:TRUE;
		}
		/*****************************************************************/
		// получить позицию и размеры контрола
		case DM_GETITEMPOSITION: // Param1=ID, Param2=*SMALL_RECT

			if (Param2)
			{
				SMALL_RECT Rect;
				if (GetItemRect(Param1,Rect))
				{
					*reinterpret_cast<PSMALL_RECT>(Param2)=Rect;
					return TRUE;
				}
			}

			return FALSE;
			/*****************************************************************/
		case DM_SETITEMDATA:
		{
			intptr_t PrewDataDialog=CurItem->UserData;
			CurItem->UserData=(intptr_t)Param2;
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
				DlgEdit *EditLine=(DlgEdit *)(CurItem->ObjPtr);
				int ClearFlag=EditLine->GetClearFlag();

				if (reinterpret_cast<intptr_t>(Param2) >= 0)
				{
					EditLine->SetClearFlag(Param2!=0);
					EditLine->Select(-1,0); // снимаем выделение

					if (DialogMode.Check(DMODE_SHOW)) //???
					{
						ShowDialog(Param1);
						Global->ScrBuf->Flush();
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
			EditorSelect *EdSel=(EditorSelect *)Param2;
			if (IsEdit(Type) && CheckStructSize(EdSel))
			{
				if (Msg == DM_GETSELECTION)
				{
					DlgEdit *EditLine=(DlgEdit *)(CurItem->ObjPtr);
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
					DlgEdit *EditLine=(DlgEdit *)(CurItem->ObjPtr);

					//EdSel->BlockType=BTYPE_STREAM;
					//EdSel->BlockStartLine=0;
					//EdSel->BlockHeight=1;
					if (EdSel->BlockType==BTYPE_NONE)
						EditLine->Select(-1,0);
					else
						EditLine->Select(EdSel->BlockStartPos,EdSel->BlockStartPos+EdSel->BlockWidth);

					if (DialogMode.Check(DMODE_SHOW)) //???
					{
						ShowDialog(Param1);
						Global->ScrBuf->Flush();
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

void Dialog::SetPosition(int X1,int Y1,int X2,int Y2)
{
	CriticalSectionLock Lock(CS);

	if (X1 != -1)
		RealWidth = X2-X1+1;
	else
		RealWidth = X2;

	if (Y1 != -1)
		RealHeight = Y2-Y1+1;
	else
		RealHeight = Y2;

	ScreenObjectWithShadow::SetPosition(X1, Y1, X2, Y2);
}
//////////////////////////////////////////////////////////////////////////
BOOL Dialog::IsInited()
{
	CriticalSectionLock Lock(CS);
	return DialogMode.Check(DMODE_INITOBJECTS);
}

void Dialog::CalcComboBoxPos(DialogItemEx* CurItem, intptr_t ItemCount, int &X1, int &Y1, int &X2, int &Y2)
{
	if(!CurItem)
	{
		CurItem=Items[FocusPos].get();
	}

	((DlgEdit*)CurItem->ObjPtr)->GetPosition(X1,Y1,X2,Y2);

	if (X2-X1<20)
		X2=X1+20;

	if (ScrY-Y1<std::min(Global->Opt->Dialogs.CBoxMaxHeight.Get(),ItemCount)+2 && Y1>ScrY/2)
	{
		Y2=Y1-1;
		Y1=std::max((intptr_t)0,Y1-1-std::min(Global->Opt->Dialogs.CBoxMaxHeight.Get(),ItemCount)-1);
	}
	else
	{
		++Y1;
		Y2=0;
	}
}

void Dialog::SetComboBoxPos(DialogItemEx* CurItem)
{
	if (GetDropDownOpened())
	{
		if(!CurItem)
		{
			CurItem=Items[FocusPos].get();
		}
		int X1,Y1,X2,Y2;
		CalcComboBoxPos(CurItem, CurItem->ListPtr->GetItemCount(), X1, Y1, X2, Y2);
		CurItem->ListPtr->SetPosition(X1, Y1, X2, Y2);
	}
}

bool Dialog::ProcessEvents()
{
	return !DialogMode.Check(DMODE_ENDLOOP);
}

void Dialog::SetId(const GUID& Id)
{
	this->Id=Id;
	IdExist=true;
}

intptr_t PluginDialogProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	// TODO: SEH
	return static_cast<PluginDialog*>(Dlg)->Proc()(Dlg, Msg, Param1, Param2);
}
