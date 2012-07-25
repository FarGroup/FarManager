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
	To.Reserved = From.Reserved;
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

		ItemEx->X2 = Max(ItemEx->X1, ItemEx->X2);
		ItemEx->Y2 = Max(ItemEx->Y1, ItemEx->Y2);

		if ((ItemEx->Type == DI_COMBOBOX || ItemEx->Type == DI_LISTBOX) && !IsPtr(Item->ListItems))
		{
			ItemEx->ListItems=nullptr;
		}
	}
}



Dialog::Dialog(DialogItemEx *SrcItem,    // Набор элементов диалога
               size_t SrcItemCount,              // Количество элементов
               FARWINDOWPROC DlgProc,      // Диалоговая процедура
               void* InitParam):             // Ассоцированные с диалогом данные
	bInitOK(false)
{
	Item = (DialogItemEx**)xf_malloc(sizeof(DialogItemEx*)*SrcItemCount);

	for (unsigned i = 0; i < SrcItemCount; i++)
	{
		Item[i] = new DialogItemEx(SrcItem[i]);
	}

	ItemCount = static_cast<int>(SrcItemCount);
	pSaveItemEx = SrcItem;
	Init(DlgProc, InitParam);
}

Dialog::Dialog(const FarDialogItem *SrcItem,    // Набор элементов диалога
               size_t SrcItemCount,              // Количество элементов
               FARWINDOWPROC DlgProc,      // Диалоговая процедура
               void* InitParam)             // Ассоцированные с диалогом данные
{
	bInitOK = false;
	Item = (DialogItemEx**)xf_malloc(sizeof(DialogItemEx*)*SrcItemCount);

	for (unsigned i = 0; i < SrcItemCount; i++)
	{
		Item[i] = new DialogItemEx;
		Item[i]->Clear();
		//BUGBUG add error check
		ConvertItemEx(CVTITEM_FROMPLUGIN,const_cast<FarDialogItem *>(&SrcItem[i]),Item[i],1);
	}

	ItemCount = static_cast<unsigned>(SrcItemCount);
	pSaveItemEx = nullptr;
	Init(DlgProc, InitParam);
}

void Dialog::Init(FARWINDOWPROC DlgProc,      // Диалоговая процедура
                  void* InitParam)         // Ассоцированные с диалогом данные
{
	SetDynamicallyBorn(FALSE); // $OT: По умолчанию все диалоги создаются статически
	CanLoseFocus = FALSE;
	HelpTopic = nullptr;
	//Номер плагина, вызвавшего диалог (-1 = Main)
	PluginOwner = nullptr;
	DataDialog=InitParam;
	DialogMode.Set(DMODE_ISCANMOVE);
	SetDropDownOpened(FALSE);
	IsEnableRedraw=0;
	FocusPos=(unsigned)-1;
	PrevFocusPos=(unsigned)-1;

	if (!DlgProc) // функция должна быть всегда!!!
	{
		DlgProc=DefDlgProc;
		// знать диалог в старом стиле - учтем этот факт!
		DialogMode.Set(DMODE_OLDSTYLE);
	}

	RealDlgProc=DlgProc;

	if (CtrlObject)
	{
		// запомним пред. режим макро.
		PrevMacroMode=CtrlObject->Macro.GetMode();
		// макросить будет в диалогах :-)
		CtrlObject->Macro.SetMode(MACRO_DIALOG);
	}

	//_SVS(SysLog(L"Dialog =%d",CtrlObject->Macro.GetMode()));
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

	if (CtrlObject)
		CtrlObject->Macro.SetMode(PrevMacroMode);

	Hide();
	if (Opt.Clock && FrameManager->IsPanelsActive(true))
		ShowTime(0);
	ScrBuf.Flush();

	if (HelpTopic)
		delete [] HelpTopic;

	for (unsigned i = 0; i < ItemCount; i++)
		delete Item[i];

	xf_free(Item);
	INPUT_RECORD rec;
	PeekInputRecord(&rec);
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

	if(CloseFAR)
	{
		SetDialogMode(DMODE_NOPLUGINS);
	}

	if (!DialogMode.Check(DMODE_INITOBJECTS))      // самодостаточный вариант, когда
	{                      //  элементы инициализируются при первом вызове.
		CheckDialogCoord();
		unsigned InitFocus=InitDialogObjects();
		int Result=(int)DlgProc(this,DN_INITDIALOG,InitFocus,DataDialog);

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

		DlgProc(this,DN_GOTFOCUS,InitFocus,0);
	}
}

//////////////////////////////////////////////////////////////////////////
/* Public, Virtual:
   Расчет значений координат окна диалога и вызов функции
   ScreenObject::Show() для вывода диалога на экран.
*/
void Dialog::Show()
{
	CriticalSectionLock Lock(CS);
	_tran(SysLog(L"[%p] Dialog::Show()",this));

	if (!DialogMode.Check(DMODE_INITOBJECTS))
		return;

	if (!Locked() && DialogMode.Check(DMODE_RESIZED))
	{
		PreRedrawItem preRedrawItem=PreRedraw.Peek();

		if (preRedrawItem.PreRedrawFunc)
			preRedrawItem.PreRedrawFunc();
	}

	DialogMode.Clear(DMODE_RESIZED);

	if (Locked())
		return;

	DialogMode.Set(DMODE_SHOW);
	ScreenObject::Show();
}

//  Цель перехвата данной функции - управление видимостью...
void Dialog::Hide()
{
	CriticalSectionLock Lock(CS);
	_tran(SysLog(L"[%p] Dialog::Hide()",this));

	if (!DialogMode.Check(DMODE_INITOBJECTS))
		return;

	DialogMode.Clear(DMODE_SHOW);
	ScreenObject::Hide();
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

	for (unsigned I=0; I < ItemCount; I++)
	{
		// Последовательно объявленные элементы с флагом DIF_CENTERGROUP
		// и одинаковой вертикальной позицией будут отцентрированы в диалоге.
		// Их координаты X не важны. Удобно использовать для центрирования
		// групп кнопок.
		if ((Item[I]->Flags & DIF_CENTERGROUP) &&
		        (!I ||
		         (I > 0 &&
		          (!(Item[I-1]->Flags & DIF_CENTERGROUP) ||
		           Item[I-1]->Y1!=Item[I]->Y1)
		         )
		        )
		   )
		{
			int Length=0;

			for (UINT J=I; J < ItemCount && (Item[J]->Flags & DIF_CENTERGROUP) && Item[J]->Y1==Item[I]->Y1; J++)
			{
				Length+=LenStrItem(J);

				if (!Item[J]->strData.IsEmpty())
					switch (Item[J]->Type)
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

			if (!Item[I]->strData.IsEmpty())
				switch (Item[I]->Type)
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

			int StartX=Max(0,(X2-X1+1-Length)/2);

			for (UINT J=I; J < ItemCount && (Item[J]->Flags & DIF_CENTERGROUP) && Item[J]->Y1==Item[I]->Y1; J++)
			{
				Item[J]->X1=StartX;
				StartX+=LenStrItem(J);

				if (!Item[J]->strData.IsEmpty())
					switch (Item[J]->Type)
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

				if (StartX == Item[J]->X1)
					Item[J]->X2=StartX;
				else
					Item[J]->X2=StartX-1;
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
unsigned Dialog::InitDialogObjects(unsigned ID)
{
	CriticalSectionLock Lock(CS);
	unsigned I, J;
	FARDIALOGITEMTYPES Type;
	DialogItemEx *CurItem;
	unsigned InitItemCount;
	unsigned __int64 ItemFlags;
	_DIALOG(CleverSysLog CL(L"Init Dialog"));

	if (ID+1 > ItemCount)
		return (unsigned)-1;

	if (ID == (unsigned)-1) // инициализируем все?
	{
		ID=0;
		InitItemCount=ItemCount;
	}
	else
	{
		InitItemCount=ID+1;
	}

	//   если FocusPos в пределах и элемент задисаблен, то ищем сначала.
	if (FocusPos!=(unsigned)-1 && FocusPos < ItemCount &&
	        (Item[FocusPos]->Flags&(DIF_DISABLE|DIF_NOFOCUS|DIF_HIDDEN)))
		FocusPos = (unsigned)-1; // будем искать сначала!

	// предварительный цикл по поводу кнопок
	for (I=ID; I < InitItemCount; I++)
	{
		CurItem = Item[I];
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
		if (FocusPos == (unsigned)-1 &&
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
	if (FocusPos == (unsigned)-1)
	{
		for (I=0; I < ItemCount; I++) // по всем!!!!
		{
			CurItem = Item[I];

			if (CanGetFocus(CurItem->Type) &&
			        !(CurItem->Flags&(DIF_DISABLE|DIF_NOFOCUS|DIF_HIDDEN)))
			{
				FocusPos=I;
				break;
			}
		}
	}

	if (FocusPos == (unsigned)-1) // ну ни хрена себе - нет ни одного
	{                  //   элемента с возможностью фокуса
		FocusPos=0;     // убится, блин
	}

	// ну вот и добрались до!
	Item[FocusPos]->Flags|=DIF_FOCUS;
	// а теперь все сначала и по полной программе...
	ProcessCenterGroup(); // сначала отцентрируем

	for (I=ID; I < InitItemCount; I++)
	{
		CurItem = Item[I];
		Type=CurItem->Type;
		ItemFlags=CurItem->Flags;

		if (Type==DI_LISTBOX)
		{
			if (!DialogMode.Check(DMODE_CREATEOBJECTS))
			{
				CurItem->ListPtr=new VMenu(nullptr,nullptr,0,CurItem->Y2-CurItem->Y1+1,
				                           VMENU_ALWAYSSCROLLBAR|VMENU_LISTBOX,nullptr,this);
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
				ListPtr->ChangeFlags(VMENU_DISABLED, ItemFlags&DIF_DISABLE);
				ListPtr->ChangeFlags(VMENU_SHOWAMPERSAND, !(ItemFlags&DIF_LISTNOAMPERSAND));
				ListPtr->ChangeFlags(VMENU_SHOWNOBOX, ItemFlags&DIF_LISTNOBOX);
				ListPtr->ChangeFlags(VMENU_WRAPMODE, ItemFlags&DIF_LISTWRAPMODE);
				ListPtr->ChangeFlags(VMENU_AUTOHIGHLIGHT, ItemFlags&DIF_LISTAUTOHIGHLIGHT);

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
					CurItem->ListPtr=new VMenu(L"",nullptr,0,Opt.Dialogs.CBoxMaxHeight,VMENU_ALWAYSSCROLLBAR|VMENU_NOTCHANGE,nullptr,this);
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
					DialogEdit->SetDropDownBox(ItemFlags & DIF_DROPDOWNLIST);
					ListPtr->ChangeFlags(VMENU_WRAPMODE, ItemFlags&DIF_LISTWRAPMODE);
					ListPtr->ChangeFlags(VMENU_DISABLED, ItemFlags&DIF_DISABLE);
					ListPtr->ChangeFlags(VMENU_SHOWAMPERSAND, !(ItemFlags&DIF_LISTNOAMPERSAND));
					ListPtr->ChangeFlags(VMENU_AUTOHIGHLIGHT, ItemFlags&DIF_LISTAUTOHIGHLIGHT);

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
				DialogEdit->SetPasswordMode(TRUE);
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
				DialogEdit->SetPersistentBlocks(Opt.Dialogs.EditBlock);

			DialogEdit->SetDelRemovesBlocks(Opt.Dialogs.DelRemovesBlocks);

			if (ItemFlags&DIF_READONLY)
				DialogEdit->SetReadOnly(1);
		}
		else if (Type == DI_USERCONTROL)
		{
			if (!DialogMode.Check(DMODE_CREATEOBJECTS))
				CurItem->UCData=new DlgUserControl;
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

	for (unsigned I=0; I < ItemCount; I++)
	{
		CurItem = Item[I];

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
		else if (CurItem->Type==DI_LISTBOX && !I)
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
				SendDlgMessage(this,DM_SETTEXT,MsgIndex,&IData);
			}
		}
	}
}


//   Изменение координат и/или размеров итема диалога.
BOOL Dialog::SetItemRect(unsigned ID,SMALL_RECT *Rect)
{
	CriticalSectionLock Lock(CS);

	if (ID >= ItemCount)
		return FALSE;

	DialogItemEx *CurItem=Item[ID];
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
			CurItem->Y2=0;                    // ???
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
		ShowDialog((unsigned)-1);
		ScrBuf.Flush();
	}

	return TRUE;
}

BOOL Dialog::GetItemRect(unsigned I,SMALL_RECT& Rect)
{
	CriticalSectionLock Lock(CS);

	if (I >= ItemCount)
		return FALSE;

	DialogItemEx *CurItem=Item[I];
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

bool Dialog::ItemHasDropDownArrow(const DialogItemEx *Item)
{
	return ((!Item->strHistory.IsEmpty() && (Item->Flags & DIF_HISTORY) && Opt.Dialogs.EditHistory) ||
		(Item->Type == DI_COMBOBOX && Item->ListPtr && Item->ListPtr->GetItemCount() > 0));
}

//////////////////////////////////////////////////////////////////////////
/* Private:
   Получение данных и удаление "редакторов"
*/
void Dialog::DeleteDialogObjects()
{
	CriticalSectionLock Lock(CS);
	DialogItemEx *CurItem;

	for (unsigned I=0; I < ItemCount; I++)
	{
		CurItem = Item[I];

		switch (CurItem->Type)
		{
			case DI_EDIT:
			case DI_FIXEDIT:
			case DI_PSWEDIT:
			case DI_COMBOBOX:
			case DI_MEMOEDIT:

				if (CurItem->ObjPtr)
					delete(DlgEdit *)(CurItem->ObjPtr);

			case DI_LISTBOX:

				if ((CurItem->Type == DI_COMBOBOX || CurItem->Type == DI_LISTBOX) &&
				        CurItem->ListPtr)
					delete CurItem->ListPtr;

				break;
			case DI_USERCONTROL:

				if (CurItem->UCData)
					delete CurItem->UCData;

				break;

			default:
				break;
		}

		if (CurItem->Flags&DIF_AUTOMATION)
			if (CurItem->AutoPtr)
				xf_free(CurItem->AutoPtr);
	}
}


//////////////////////////////////////////////////////////////////////////
/* Public:
   Сохраняет значение из полей редактирования.
   При установленном флаге DIF_HISTORY, сохраняет данные в реестре.
*/
void Dialog::GetDialogObjectsData()
{
	CriticalSectionLock Lock(CS);
	int Type;
	DialogItemEx *CurItem;

	for (unsigned I=0; I < ItemCount; I++)
	{
		CurItem = Item[I];
		FARDIALOGITEMFLAGS IFlags=CurItem->Flags;

		switch (Type=CurItem->Type)
		{
			case DI_MEMOEDIT:
				break; //????
			case DI_EDIT:
			case DI_FIXEDIT:
			case DI_PSWEDIT:
			case DI_COMBOBOX:
			{
				if (CurItem->ObjPtr)
				{
					string strData;
					DlgEdit *EditPtr=(DlgEdit *)(CurItem->ObjPtr);

					// подготовим данные
					// получим данные
					EditPtr->GetString(strData);

					if (ExitCode >=0 &&
					        (IFlags & DIF_HISTORY) &&
					        !(IFlags & DIF_MANUALADDHISTORY) && // при мануале не добавляем
							!CurItem->strHistory.IsEmpty() &&
					        Opt.Dialogs.EditHistory)
					{
						AddToEditHistory(CurItem, strData);
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

					if ((IFlags&DIF_EDITEXPAND) && Type != DI_PSWEDIT && Type != DI_FIXEDIT)
					{
						apiExpandEnvironmentStrings(strData, strData);
						//как бы грязный хак, нам нужно обновить строку чтоб отдавалась правильная строка
						//для различных DM_* после закрытия диалога, но ни в коем случае нельзя чтоб
						//высылался DN_EDITCHANGE для этого изменения, ибо диалог уже закрыт.
						EditPtr->SetCallbackState(false);
						EditPtr->SetString(strData);
						EditPtr->SetCallbackState(true);

					}

					CurItem->strData = strData;
				}

				break;
			}
			case DI_LISTBOX:
				/*
				  if(CurItem->ListPtr)
				  {
				    CurItem->ListPos=CurItem->ListPtr->GetSelectPos();
				    break;
				  }
				*/
				break;
				/**/
		}

#if 0

		if ((Type == DI_COMBOBOX || Type == DI_LISTBOX) && CurItem->ListPtr && CurItem->ListItems && DlgProc == DefDlgProc)
		{
			int ListPos=CurItem->ListPtr->GetSelectPos();

			if (ListPos < CurItem->ListItems->ItemsNumber)
			{
				for (int J=0; J < CurItem->ListItems->ItemsNumber; ++J)
					CurItem->ListItems->Items[J].Flags&=~LIF_SELECTED;

				CurItem->ListItems->Items[ListPos].Flags|=LIF_SELECTED;
			}
		}

#else

		if ((Type == DI_COMBOBOX || Type == DI_LISTBOX))
		{
			CurItem->ListPos=CurItem->ListPtr?CurItem->ListPtr->GetSelectPos():0;
		}

#endif
	}
}

// Функция формирования и запроса цветов.
intptr_t Dialog::CtlColorDlgItem(FarColor Color[4],int ItemPos,int Type,int Focus,int Default,FARDIALOGITEMFLAGS Flags)
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
			Item[ItemPos]->ListPtr->SetColors(nullptr);
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
	return DlgProc(this, DN_CTLCOLORDLGITEM, ItemPos, &ItemColors);
}

//////////////////////////////////////////////////////////////////////////
/* Private:
   Отрисовка элементов диалога на экране.
*/
void Dialog::ShowDialog(unsigned ID)
{
	CriticalSectionLock Lock(CS);

	if (Locked())
		return;

	string strStr;
	wchar_t *lpwszStr;
	DialogItemEx *CurItem;
	int X,Y;
	unsigned I,DrawItemCount;
	FarColor ItemColor[4] = {};

	//   Если не разрешена отрисовка, то вываливаем.
	if (IsEnableRedraw ||                // разрешена прорисовка ?
	        (ID+1 > ItemCount) ||             // а номер в рамках дозволенного?
	        DialogMode.Check(DMODE_DRAWING) || // диалог рисуется?
	        !DialogMode.Check(DMODE_SHOW) ||   // если не видим, то и не отрисовываем.
	        !DialogMode.Check(DMODE_INITOBJECTS))
		return;

	DialogMode.Set(DMODE_DRAWING);  // диалог рисуется!!!
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

	if (ID == (unsigned)-1) // рисуем все?
	{
		//   Перед прорисовкой диалога посылаем сообщение в обработчик
		if (!DlgProc(this,DN_DRAWDIALOG,0,0))
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
			DlgProc(this, DN_CTLCOLORDIALOG, 0, &Color);
			SetScreen(X1, Y1, X2, Y2, L' ', Color);
		}

		ID=0;
		DrawItemCount=ItemCount;
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

		if (ID != (unsigned)-1 && FocusPos != ID)
		{
			if (Item[FocusPos]->Type == DI_USERCONTROL && Item[FocusPos]->UCData->CursorPos.X != -1 && Item[FocusPos]->UCData->CursorPos.Y != -1)
			{
				CursorVisible=Item[FocusPos]->UCData->CursorVisible;
				CursorSize=Item[FocusPos]->UCData->CursorSize;
			}
		}

		SetCursorType(CursorVisible,CursorSize);
	}

	for (I=ID; I < DrawItemCount; I++)
	{
		CurItem = Item[I];

		if (CurItem->Flags&DIF_HIDDEN)
			continue;

		/* $ 28.07.2000 SVS
		   Перед прорисовкой каждого элемента посылаем сообщение
		   посредством функции SendDlgMessage - в ней делается все!
		*/
		if (!SendDlgMessage(this,DN_DRAWDLGITEM,I,0))
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
				LenText=LenStrItem(I,strStr);

				if (!(CurItem->Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2)) && (CurItem->Flags & DIF_CENTERTEXT) && CX1!=-1)
					LenText=LenStrItem(I,CenterStr(strStr,strStr,CX2-CX1+1));

				X=(CX1==-1 || (CurItem->Flags & (DIF_SEPARATOR|DIF_SEPARATOR2)))?(X2-X1+1-LenText)/2:CX1;
				Y=(CY1==-1)?(Y2-Y1+1)/2:CY1;

				if (X < 0)
					X=0;

				if ((CX2 <= 0) || (CX2 < CX1))
					CW = LenText;

				if (X1+X+LenText > X2)
				{
					int tmpCW=ObjWidth;

					if (CW < ObjWidth)
						tmpCW=CW+1;

					strStr.SetLength(tmpCW-1);
				}

				// нужно ЭТО
				//SetScreen(X1+CX1,Y1+CY1,X1+CX2,Y1+CY2,' ',Attr&0xFF);
				// вместо этого:
				if (CX1 > -1 && CX2 > CX1 && !(CurItem->Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2))) //половинчатое решение
				{
					int CntChr=CX2-CX1+1;
					SetColor(ItemColor[0]);
					GotoXY(X1+X,Y1+Y);

					if (X1+X+CntChr-1 > X2)
						CntChr=X2-(X1+X)+1;

					FS<<fmt::MinWidth(CntChr)<<L"";

					if (CntChr < LenText)
						strStr.SetLength(CntChr);
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

				SetColor(ItemColor[0]);
				GotoXY(X1+X,Y1+Y);

				if (CurItem->Flags & DIF_SHOWAMPERSAND)
				{
					//MessageBox(0, strStr, strStr, MB_OK);
					Text(strStr);
				}
				else
				{
					//MessageBox(0, strStr, strStr, MB_OK);
					HiText(strStr,ItemColor[1]);
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

				X=(CX1==-1)?(X2-X1+1)/2:CX1;
				Y=(CY1==-1 || (CurItem->Flags & (DIF_SEPARATOR|DIF_SEPARATOR2)))?(Y2-Y1+1-LenText)/2:CY1;

				if (Y < 0)
					Y=0;

				if ((CY2 <= 0) || (CY2 < CY1))
					CH = LenStrItem(I,strStr);

				if (Y1+Y+LenText > Y2)
				{
					int tmpCH=ObjHeight;

					if (CH < ObjHeight)
						tmpCH=CH+1;

					strStr.SetLength(tmpCH-1);
				}

				// нужно ЭТО
				//SetScreen(X1+CX1,Y1+CY1,X1+CX2,Y1+CY2,' ',Attr&0xFF);
				// вместо этого:
				if (CY1 > -1 && CY2 > 0 && CY2 > CY1 && !(CurItem->Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2))) //половинчатое решение
				{
					int CntChr=CY2-CY1+1;
					SetColor(ItemColor[0]);
					GotoXY(X1+X,Y1+Y);

					if (Y1+Y+CntChr-1 > Y2)
						CntChr=Y2-(Y1+Y)+1;

					vmprintf(L"%*s",CntChr,L"");
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
					strStr.SetLength(ObjWidth-1);

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
					ScrBuf.ApplyColor(startx,Y1+CY1,startx+1,Y1+CY1,Color);
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

					if (DlgProc(this,DN_CTLCOLORDLGLIST,I,&ListColors))
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
	for (I=0; I < ItemCount; I++)
	{
		CurItem=Item[I];

		if (CurItem->ListPtr && GetDropDownOpened() && CurItem->ListPtr->IsVisible())
		{
			if ((CurItem->Type == DI_COMBOBOX) ||
			        ((CurItem->Type == DI_EDIT || CurItem->Type == DI_FIXEDIT) &&
			         !(CurItem->Flags&DIF_HIDDEN) &&
			         (CurItem->Flags&DIF_HISTORY)))
			{
				CurItem->ListPtr->Show();
			}
		}
	}

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
		DefDlgProc(this,DN_DRAWDIALOGDONE,1,0);
	}
	else
		DlgProc(this,DN_DRAWDIALOGDONE,0,0);
}

int Dialog::LenStrItem(int ID, const wchar_t *lpwszStr)
{
	CriticalSectionLock Lock(CS);

	if (!lpwszStr)
		lpwszStr = Item[ID]->strData;

	return (Item[ID]->Flags & DIF_SHOWAMPERSAND)?StrLength(lpwszStr):HiStrlen(lpwszStr);
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
				rr=(Key == KEY_CTRLRIGHT || Key == KEY_RCTRLRIGHT || Key == KEY_CTRLNUMPAD6 || Key == KEY_RCTRLNUMPAD6)?10:Max(0,ScrX-X2);
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
				rr=(Key == KEY_CTRLDOWN || Key == KEY_RCTRLDOWN || Key == KEY_CTRLNUMPAD2 || Key == KEY_RCTRLNUMPAD2)? 5:Max(0,ScrY-Y2);
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
					DlgProc(this,DN_DRAGGED,1,0);
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
					DlgProc(this,DN_DRAGGED,1,ToPtr(TRUE));
					Show();
				}

				break;
		}

		if (DialogMode.Check(DMODE_ALTDRAGGED))
		{
			DialogMode.Clear(DMODE_DRAGGED|DMODE_ALTDRAGGED);
			DlgProc(this,DN_DRAGGED,1,0);
			Show();
		}

		return (TRUE);
	}

	if ((Key == KEY_CTRLF5 || Key == KEY_RCTRLF5) && DialogMode.Check(DMODE_ISCANMOVE))
	{
		if (DlgProc(this,DN_DRAGGED,0,0)) // если разрешили перемещать!
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

#ifdef FAR_LUA
#else
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

			if (GetDropDownOpened() || Item[FocusPos]->Type == DI_LISTBOX)
			{
				if (Item[FocusPos]->ListPtr)
					return Item[FocusPos]->ListPtr->VMProcess(OpCode,vParam,iParam);
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
			if (IsEdit(Item[FocusPos]->Type))
			{
				if (Item[FocusPos]->Type == DI_COMBOBOX && GetDropDownOpened())
					return Item[FocusPos]->ListPtr->VMProcess(OpCode,vParam,iParam);
				else
					return ((DlgEdit *)(Item[FocusPos]->ObjPtr))->VMProcess(OpCode,vParam,iParam);
			}
			else if (Item[FocusPos]->Type == DI_LISTBOX && OpCode != MCODE_C_SELECTED)
				return Item[FocusPos]->ListPtr->VMProcess(OpCode,vParam,iParam);

			return 0;
		}
		case MCODE_V_DLGITEMTYPE:
		{
			switch (Item[FocusPos]->Type)
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
			return ItemCount;
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
			switch (Item[FocusPos]->Type)
			{
				case DI_COMBOBOX:

					if (DropDownOpened || (Item[FocusPos]->Flags & DIF_DROPDOWNLIST))
					{
						Ret=Item[FocusPos]->ListPtr->VMProcess(OpCode,vParam,iParam);
						break;
					}

				case DI_EDIT:
				case DI_PSWEDIT:
				case DI_FIXEDIT:
					return ((DlgEdit *)(Item[FocusPos]->ObjPtr))->VMProcess(OpCode,vParam,iParam);

				case DI_LISTBOX:
					Ret=Item[FocusPos]->ListPtr->VMProcess(OpCode,vParam,iParam);
					break;

				case DI_USERCONTROL:
				{
					if (OpCode == MCODE_V_CURPOS)
						Ret=Item[FocusPos]->UCData->CursorPos.X;
					break;
				}
				case DI_BUTTON:
				case DI_CHECKBOX:
				case DI_RADIOBUTTON:
					return 0;
				default:
					return 0;
			}

			FarGetValue fgv={OpCode==MCODE_V_ITEMCOUNT?11:7,{FMVT_INTEGER}};
			fgv.Value.Integer=Ret;

			if (SendDlgMessage((HANDLE)this,DN_GETVALUE,FocusPos,&fgv))
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
			if (IsEdit(Item[FocusPos]->Type) || (Item[FocusPos]->Type==DI_COMBOBOX && !(DropDownOpened || (Item[FocusPos]->Flags & DIF_DROPDOWNLIST))))
			{
				return ((DlgEdit *)(Item[FocusPos]->ObjPtr))->VMProcess(OpCode,vParam,iParam);
			}

			return 0;
		}

		default:
			break;
	}

	return 0;
}
#endif

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
	unsigned I;
	string strStr;

	if (Key==KEY_NONE || Key==KEY_IDLE)
	{
		DlgProc(this,DN_ENTERIDLE,0,0); // $ 28.07.2000 SVS Передадим этот факт в обработчик :-)
		return FALSE;
	}

	if (Key == KEY_KILLFOCUS || Key == KEY_GOTFOCUS)
	{
		DlgProc(this,DN_ACTIVATEAPP,Key == KEY_KILLFOCUS?FALSE:TRUE,0);
		return FALSE;
	}

	if (ProcessMoveDialog(Key))
		return TRUE;

	// BugZ#488 - Shift=enter
	if (IntKeyState.ShiftPressed && (Key == KEY_ENTER||Key==KEY_NUMENTER) && !CtrlObject->Macro.IsExecuting() && Item[FocusPos]->Type != DI_BUTTON)
	{
		Key=Key == KEY_ENTER?KEY_SHIFTENTER:KEY_SHIFTNUMENTER;
	}

	if (!(/*(Key>=KEY_MACRO_BASE && Key <=KEY_MACRO_ENDBASE) ||*/ ((unsigned int)Key>=KEY_OP_BASE && (unsigned int)Key <=KEY_OP_ENDBASE)) && !DialogMode.Check(DMODE_KEY))
	{
		INPUT_RECORD rec;
		if (KeyToInputRecord(Key,&rec) && DlgProc(this,DN_CONTROLINPUT,FocusPos,&rec))
			return TRUE;
	}

	if (!DialogMode.Check(DMODE_SHOW))
		return TRUE;

	// А ХЗ, может в этот момент изменилось состояние элемента!
	if (Item[FocusPos]->Flags&DIF_HIDDEN)
		return TRUE;

	// небольшая оптимизация
	if (Item[FocusPos]->Type==DI_CHECKBOX)
	{
		if (!(Item[FocusPos]->Flags&DIF_3STATE))
		{
			if (Key == KEY_MULTIPLY) // в CheckBox 2-state Gray* не работает!
				Key = KEY_NONE;

			if ((Key == KEY_ADD      && !Item[FocusPos]->Selected) ||
			        (Key == KEY_SUBTRACT &&  Item[FocusPos]->Selected))
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

	if (Item[FocusPos]->Type==DI_BUTTON && Key == KEY_SPACE)
		Key=KEY_ENTER;

	if (Item[FocusPos]->Type == DI_LISTBOX)
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
				VMenu *List=Item[FocusPos]->ListPtr;
				int CurListPos=List->GetSelectPos();
				int CheckedListItem=List->GetCheck(-1);
				List->ProcessKey(Key);
				int NewListPos=List->GetSelectPos();

				if (NewListPos != CurListPos && !DlgProc(this,DN_LISTCHANGE,FocusPos,ToPtr(NewListPos)))
				{
					if (!DialogMode.Check(DMODE_SHOW))
						return TRUE;

					List->SetCheck(CheckedListItem,CurListPos);

					if (DialogMode.Check(DMODE_SHOW) && !(Item[FocusPos]->Flags&DIF_HIDDEN))
						ShowDialog(FocusPos); // FocusPos
				}

				if (!(Key == KEY_ENTER || Key == KEY_NUMENTER) || (Item[FocusPos]->Flags&DIF_LISTNOCLOSE))
					return TRUE;
		}
	}

	switch (Key)
	{
		case KEY_F1:

			// Перед выводом диалога посылаем сообщение в обработчик
			//   и если вернули что надо, то выводим подсказку
			if (!Help::MkTopic(PluginOwner,
			                   (const wchar_t*)DlgProc(this,DN_HELP,FocusPos,
			                                           (HelpTopic?HelpTopic:nullptr)),
			                   strStr).IsEmpty())
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

			if (Item[FocusPos]->Type == DI_USERCONTROL) // для user-типа вываливаем
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
			for (I=0; I<ItemCount; I++)
				if (Item[I]->Flags&DIF_DEFAULTBUTTON)
				{
					if (Item[I]->Flags&DIF_DISABLE)
					{
						// ProcessKey(KEY_DOWN); // на твой вкус :-)
						return TRUE;
					}

					if (!IsEdit(Item[I]->Type))
						Item[I]->Selected=1;

					ExitCode=I;
					/* $ 18.05.2001 DJ */
					CloseDialog();
					/* DJ $ */
					return TRUE;
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
			if (Item[FocusPos]->Type != DI_COMBOBOX
			        && IsEdit(Item[FocusPos]->Type)
			        && (Item[FocusPos]->Flags & DIF_EDITOR) && !(Item[FocusPos]->Flags & DIF_READONLY))
			{
				unsigned EditorLastPos;

				for (EditorLastPos=I=FocusPos; I<ItemCount; I++)
					if (IsEdit(Item[I]->Type) && (Item[I]->Flags & DIF_EDITOR))
						EditorLastPos=I;
					else
						break;

				if (((DlgEdit *)(Item[EditorLastPos]->ObjPtr))->GetLength())
					return TRUE;

				for (I=EditorLastPos; I>FocusPos; I--)
				{
					int CurPos;

					if (I==FocusPos+1)
						CurPos=((DlgEdit *)(Item[I-1]->ObjPtr))->GetCurPos();
					else
						CurPos=0;

					((DlgEdit *)(Item[I-1]->ObjPtr))->GetString(strStr);
					int Length=(int)strStr.GetLength();
					((DlgEdit *)(Item[I]->ObjPtr))->SetString(CurPos>=Length ? L"":strStr.CPtr()+CurPos);

					if (CurPos<Length)
						strStr.SetLength(CurPos);

					((DlgEdit *)(Item[I]->ObjPtr))->SetCurPos(0);
					((DlgEdit *)(Item[I-1]->ObjPtr))->SetString(strStr);
				}

				if (EditorLastPos > FocusPos)
				{
					((DlgEdit *)(Item[FocusPos]->ObjPtr))->SetCurPos(0);
					Do_ProcessNextCtrl(FALSE,FALSE);
				}

				ShowDialog();
				return TRUE;
			}
			else if (Item[FocusPos]->Type==DI_BUTTON)
			{
				Item[FocusPos]->Selected=1;

				// сообщение - "Кнокна кликнута"
				if (SendDlgMessage(this,DN_BTNCLICK,FocusPos,0))
					return TRUE;

				if (Item[FocusPos]->Flags&DIF_BTNNOCLOSE)
					return TRUE;

				ExitCode=FocusPos;
				CloseDialog();
				return TRUE;
			}
			else
			{
				ExitCode=-1;

				for (I=0; I<ItemCount; I++)
				{
					if ((Item[I]->Flags&DIF_DEFAULTBUTTON) && !(Item[I]->Flags&DIF_BTNNOCLOSE))
					{
						if (Item[I]->Flags&DIF_DISABLE)
						{
							// ProcessKey(KEY_DOWN); // на твой вкус :-)
							return TRUE;
						}

//            if (!(IsEdit(Item[I].Type) || Item[I].Type == DI_CHECKBOX || Item[I].Type == DI_RADIOBUTTON))
//              Item[I].Selected=1;
						ExitCode=I;
						break;
					}
				}
			}

			if (ExitCode==-1)
				ExitCode=FocusPos;

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

			if (Item[FocusPos]->Type==DI_CHECKBOX)
			{
				unsigned int CHKState=
				    (Key == KEY_ADD?1:
				     (Key == KEY_SUBTRACT?0:
				      ((Key == KEY_MULTIPLY)?2:
				       Item[FocusPos]->Selected)));

				if (Item[FocusPos]->Selected != (int)CHKState)
					if (SendDlgMessage(this,DN_BTNCLICK,FocusPos,ToPtr(CHKState)))
					{
						Item[FocusPos]->Selected=CHKState;
						ShowDialog();
					}
			}

			return TRUE;
		case KEY_LEFT:  case KEY_NUMPAD4: case KEY_MSWHEEL_LEFT:
		case KEY_RIGHT: case KEY_NUMPAD6: case KEY_MSWHEEL_RIGHT:
		{
			if (Item[FocusPos]->Type == DI_USERCONTROL) // для user-типа вываливаем
				return TRUE;

			if (IsEdit(Item[FocusPos]->Type))
			{
				((DlgEdit *)(Item[FocusPos]->ObjPtr))->ProcessKey(Key);
				return TRUE;
			}
			else
			{
				int MinDist=1000,MinPos=0;

				for (I=0; I<ItemCount; I++)
				{
					if (I!=FocusPos &&
					        (IsEdit(Item[I]->Type) ||
					         Item[I]->Type==DI_CHECKBOX ||
					         Item[I]->Type==DI_RADIOBUTTON) &&
					        Item[I]->Y1==Item[FocusPos]->Y1)
					{
						int Dist=Item[I]->X1-Item[FocusPos]->X1;

						if (((Key==KEY_LEFT||Key==KEY_SHIFTNUMPAD4) && Dist<0) || ((Key==KEY_RIGHT||Key==KEY_SHIFTNUMPAD6) && Dist>0))
							if (abs(Dist)<MinDist)
							{
								MinDist=abs(Dist);
								MinPos=I;
							}
					}
				}

				if (MinDist<1000)
				{
					ChangeFocus2(MinPos);

					if (Item[MinPos]->Flags & DIF_MOVESELECT)
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

			if (Item[FocusPos]->Type == DI_USERCONTROL) // для user-типа вываливаем
				return TRUE;

			return Do_ProcessNextCtrl(Key==KEY_LEFT || Key==KEY_UP || Key == KEY_NUMPAD4 || Key == KEY_NUMPAD8);
			// $ 27.04.2001 VVM - Обработка колеса мышки
		case KEY_MSWHEEL_UP:
		case KEY_MSWHEEL_DOWN:
		case KEY_CTRLUP:      case KEY_CTRLNUMPAD8:
		case KEY_RCTRLUP:     case KEY_RCTRLNUMPAD8:
		case KEY_CTRLDOWN:    case KEY_CTRLNUMPAD2:
		case KEY_RCTRLDOWN:   case KEY_RCTRLNUMPAD2:
			return ProcessOpenComboBox(Item[FocusPos]->Type,Item[FocusPos],FocusPos);
			// ЭТО перед default предпоследний!!!
		case KEY_END:  case KEY_NUMPAD1:

			if (Item[FocusPos]->Type == DI_USERCONTROL) // для user-типа вываливаем
				return TRUE;

			if (IsEdit(Item[FocusPos]->Type))
			{
				((DlgEdit *)(Item[FocusPos]->ObjPtr))->ProcessKey(Key);
				return TRUE;
			}

			// ???
			// ЭТО перед default последний!!!
		case KEY_PGDN:   case KEY_NUMPAD3:

			if (Item[FocusPos]->Type == DI_USERCONTROL) // для user-типа вываливаем
				return TRUE;

			if (!(Item[FocusPos]->Flags & DIF_EDITOR))
			{
				for (I=0; I<ItemCount; I++)
					if (Item[I]->Flags&DIF_DEFAULTBUTTON)
					{
						ChangeFocus2(I);
						ShowDialog();
						return TRUE;
					}

				return TRUE;
			}
			break;

		case KEY_F11:
			if (!IsProcessAssignMacroKey)
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
			//if(Item[FocusPos].Type == DI_USERCONTROL) // для user-типа вываливаем
			//  return TRUE;
			if (Item[FocusPos]->Type == DI_LISTBOX)
			{
				VMenu *List=Item[FocusPos]->ListPtr;
				int CurListPos=List->GetSelectPos();
				int CheckedListItem=List->GetCheck(-1);
				List->ProcessKey(Key);
				int NewListPos=List->GetSelectPos();

				if (NewListPos != CurListPos && !DlgProc(this,DN_LISTCHANGE,FocusPos,ToPtr(NewListPos)))
				{
					if (!DialogMode.Check(DMODE_SHOW))
						return TRUE;

					List->SetCheck(CheckedListItem,CurListPos);

					if (DialogMode.Check(DMODE_SHOW) && !(Item[FocusPos]->Flags&DIF_HIDDEN))
						ShowDialog(FocusPos); // FocusPos
				}

				return TRUE;
			}

			if (IsEdit(Item[FocusPos]->Type))
			{
				DlgEdit *edt=(DlgEdit *)Item[FocusPos]->ObjPtr;

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
				else if ((Item[FocusPos]->Flags & DIF_EDITOR) && !(Item[FocusPos]->Flags & DIF_READONLY))
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
								if (FocusPos > 0 && (Item[FocusPos-1]->Flags&DIF_EDITOR))
								{
									// добавляем к предыдущему и...
									DlgEdit *edt_1=(DlgEdit *)Item[FocusPos-1]->ObjPtr;
									edt_1->GetString(strStr);
									CurPos=static_cast<int>(strStr.GetLength());
									string strAdd;
									edt->GetString(strAdd);
									strStr+=strAdd;
									edt_1->SetString(strStr);

									for (I=FocusPos+1; I<ItemCount; I++)
									{
										if (Item[I]->Flags & DIF_EDITOR)
										{
											if (I>FocusPos)
											{
												((DlgEdit *)(Item[I]->ObjPtr))->GetString(strStr);
												((DlgEdit *)(Item[I-1]->ObjPtr))->SetString(strStr);
											}

											((DlgEdit *)(Item[I]->ObjPtr))->SetString(L"");
										}
										else // ага, значит  FocusPos это есть последний из DIF_EDITOR
										{
											((DlgEdit *)(Item[I-1]->ObjPtr))->SetString(L"");
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
							for (I=FocusPos; I<ItemCount; I++)
								if (Item[I]->Flags & DIF_EDITOR)
								{
									if (I>FocusPos)
									{
										((DlgEdit *)(Item[I]->ObjPtr))->GetString(strStr);
										((DlgEdit *)(Item[I-1]->ObjPtr))->SetString(strStr);
									}

									((DlgEdit *)(Item[I]->ObjPtr))->SetString(L"");
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
							if (FocusPos<ItemCount+1 && (Item[FocusPos+1]->Flags & DIF_EDITOR))
							{
								int CurPos=edt->GetCurPos();
								int Length=edt->GetLength();
								int SelStart, SelEnd;
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
									DlgEdit *edt_1=(DlgEdit *)Item[FocusPos+1]->ObjPtr;

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
							I=FocusPos;

							while (Item[I]->Flags & DIF_EDITOR)
								I=ChangeFocus(I,(Key == KEY_PGUP || Key == KEY_NUMPAD9)?-1:1,FALSE);

							if (!(Item[I]->Flags & DIF_EDITOR))
								I=ChangeFocus(I,(Key == KEY_PGUP || Key == KEY_NUMPAD9)?1:-1,FALSE);

							ChangeFocus2(I);
							ShowDialog();

							return TRUE;
						}
					}
				}

				if (Key == KEY_OP_XLAT && !(Item[FocusPos]->Flags & DIF_READONLY))
				{
					edt->SetClearFlag(0);
					edt->Xlat();

					// иначе неправильно работает ctrl-end
					edt->strLastStr = edt->GetStringAddr();
					edt->LastPartLength=static_cast<int>(edt->strLastStr.GetLength());

					Redraw(); // Перерисовка должна идти после DN_EDITCHANGE (imho)
					return TRUE;
				}

				if (!(Item[FocusPos]->Flags & DIF_READONLY) ||
				        ((Item[FocusPos]->Flags & DIF_READONLY) && IsNavKey(Key)))
				{
					// "только что ломанулись и начинать выделение с нуля"?
					if ((Opt.Dialogs.EditLine&DLGEDITLINE_NEWSELONGOTFOCUS) && Item[FocusPos]->SelStart != -1 && PrevFocusPos != FocusPos)// && Item[FocusPos].SelEnd)
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
						if (Item[FocusPos]->Flags & DIF_READONLY)
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

void Dialog::ProcessKey(int Key, unsigned ItemPos)
{
	unsigned SavedFocusPos = FocusPos;
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
	unsigned I;
	int MsX,MsY;
	FARDIALOGITEMTYPES Type;
	SMALL_RECT Rect;
	INPUT_RECORD mouse = {};
	mouse.EventType=MOUSE_EVENT;
	mouse.Event.MouseEvent=*MouseEvent;

	if (!DialogMode.Check(DMODE_SHOW))
		return FALSE;

	if (DialogMode.Check(DMODE_MOUSEEVENT))
	{
		if (!DlgProc(this,DN_INPUT,0,&mouse))
			return TRUE;
	}

	if (!DialogMode.Check(DMODE_SHOW))
		return FALSE;

	MsX=mouse.Event.MouseEvent.dwMousePosition.X;
	MsY=mouse.Event.MouseEvent.dwMousePosition.Y;

	//for (I=0;I<ItemCount;I++)
	for (I=ItemCount-1; I!=(unsigned)-1; I--)
	{
		if (Item[I]->Flags&(DIF_DISABLE|DIF_HIDDEN))
			continue;

		Type=Item[I]->Type;

		if (Type == DI_LISTBOX &&
		        MsY >= Y1+Item[I]->Y1 && MsY <= Y1+Item[I]->Y2 &&
		        MsX >= X1+Item[I]->X1 && MsX <= X1+Item[I]->X2)
		{
			VMenu *List=Item[I]->ListPtr;
			int Pos=List->GetSelectPos();
			int CheckedListItem=List->GetCheck(-1);

			if ((mouse.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED))
			{
				if (FocusPos != I)
				{
					ChangeFocus2(I);
					ShowDialog();
				}

				if (mouse.Event.MouseEvent.dwEventFlags!=DOUBLE_CLICK && !(Item[I]->Flags&(DIF_LISTTRACKMOUSE|DIF_LISTTRACKMOUSEINFOCUS)))
				{
					List->ProcessMouse(&mouse.Event.MouseEvent);
					int NewListPos=List->GetSelectPos();

					if (NewListPos != Pos && !SendDlgMessage(this,DN_LISTCHANGE,I,ToPtr(NewListPos)))
					{
						List->SetCheck(CheckedListItem,Pos);

						if (DialogMode.Check(DMODE_SHOW) && !(Item[I]->Flags&DIF_HIDDEN))
							ShowDialog(I); // FocusPos
					}
					else
					{
						Pos=NewListPos;
					}
				}
				else if (!SendDlgMessage(this,DN_CONTROLINPUT,I,&mouse))
				{
#if 1
					List->ProcessMouse(&mouse.Event.MouseEvent);
					int NewListPos=List->GetSelectPos();
					int InScroolBar=(MsX==X1+Item[I]->X2 && MsY >= Y1+Item[I]->Y1 && MsY <= Y1+Item[I]->Y2) &&
					                (List->CheckFlags(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR) || Opt.ShowMenuScrollbar);

					if (!InScroolBar       &&                                                                // вне скроллбара и
					        NewListPos != Pos &&                                                                 // позиция изменилась и
					        !SendDlgMessage(this,DN_LISTCHANGE,I,ToPtr(NewListPos)))                      // и плагин сказал в морг
					{
						List->SetCheck(CheckedListItem,Pos);

						if (DialogMode.Check(DMODE_SHOW) && !(Item[I]->Flags&DIF_HIDDEN))
							ShowDialog(I); // FocusPos
					}
					else
					{
						Pos=NewListPos;

						if (!InScroolBar && !(Item[I]->Flags&DIF_LISTNOCLOSE))
						{
							ExitCode=I;
							CloseDialog();
							return TRUE;
						}
					}

#else

					if (SendDlgMessage(this,DN_LISTCHANGE,I,(intptr_t)Pos))
					{
						if (MsX==X1+Item[I]->X2 && MsY >= Y1+Item[I]->Y1 && MsY <= Y1+Item[I]->Y2)
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
				if (!mouse.Event.MouseEvent.dwButtonState || SendDlgMessage(this,DN_CONTROLINPUT,I,&mouse))
				{
					if ((I == FocusPos && (Item[I]->Flags&DIF_LISTTRACKMOUSEINFOCUS)) || (Item[I]->Flags&DIF_LISTTRACKMOUSE))
					{
						List->ProcessMouse(&mouse.Event.MouseEvent);
						int NewListPos=List->GetSelectPos();

						if (NewListPos != Pos && !SendDlgMessage(this,DN_LISTCHANGE,I,ToPtr(NewListPos)))
						{
							List->SetCheck(CheckedListItem,Pos);

							if (DialogMode.Check(DMODE_SHOW) && !(Item[I]->Flags&DIF_HIDDEN))
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
		if (DialogMode.Check(DMODE_CLICKOUTSIDE) && !DlgProc(this,DN_CONTROLINPUT,-1,&mouse))
		{
			if (!DialogMode.Check(DMODE_SHOW))
				return FALSE;

//      if (!(mouse.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) && PrevLButtonPressed && ScreenObject::CaptureMouseObject)
			if (!(mouse.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) && (IntKeyState.PrevMouseButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) && (Opt.Dialogs.MouseButton&DMOUSEBUTTON_LEFT))
				ProcessKey(KEY_ESC);
//      else if (!(mouse.Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED) && PrevRButtonPressed && ScreenObject::CaptureMouseObject)
			else if (!(mouse.Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED) && (IntKeyState.PrevMouseButtonState&RIGHTMOST_BUTTON_PRESSED) && (Opt.Dialogs.MouseButton&DMOUSEBUTTON_RIGHT))
				ProcessKey(KEY_ENTER);
		}

		if (mouse.Event.MouseEvent.dwButtonState)
			DialogMode.Set(DMODE_CLICKOUTSIDE);

		//ScreenObject::SetCapture(this);
		return TRUE;
	}

	if (!mouse.Event.MouseEvent.dwButtonState)
	{
		DialogMode.Clear(DMODE_CLICKOUTSIDE);
//    ScreenObject::SetCapture(nullptr);
		return FALSE;
	}

	if (!mouse.Event.MouseEvent.dwEventFlags || mouse.Event.MouseEvent.dwEventFlags==DOUBLE_CLICK)
	{
		// первый цикл - все за исключением рамок.
		//for (I=0; I < ItemCount;I++)
		for (I=ItemCount-1; I!=(unsigned)-1; I--)
		{
			if (Item[I]->Flags&(DIF_DISABLE|DIF_HIDDEN))
				continue;

			GetItemRect(I,Rect);
			Rect.Left+=X1;  Rect.Top+=Y1;
			Rect.Right+=X1; Rect.Bottom+=Y1;
//_D(SysLog(L"? %2d) Rect (%2d,%2d) (%2d,%2d) '%s'",I,Rect.left,Rect.top,Rect.right,Rect.bottom,Item[I].Data));

			if (MsX >= Rect.Left && MsY >= Rect.Top && MsX <= Rect.Right && MsY <= Rect.Bottom)
			{
				// для прозрачных :-)
				if (Item[I]->Type == DI_SINGLEBOX || Item[I]->Type == DI_DOUBLEBOX)
				{
					// если на рамке, то...
					if (((MsX == Rect.Left || MsX == Rect.Right) && MsY >= Rect.Top && MsY <= Rect.Bottom) || // vert
					        ((MsY == Rect.Top  || MsY == Rect.Bottom) && MsX >= Rect.Left && MsX <= Rect.Right))    // hor
					{
						if (DlgProc(this,DN_CONTROLINPUT,I,&mouse))
							return TRUE;

						if (!DialogMode.Check(DMODE_SHOW))
							return FALSE;
					}
					else
						continue;
				}

				if (Item[I]->Type == DI_USERCONTROL)
				{
					// для user-типа подготовим координаты мыши
					mouse.Event.MouseEvent.dwMousePosition.X-=Rect.Left;
					mouse.Event.MouseEvent.dwMousePosition.Y-=Rect.Top;
				}

//_SVS(SysLog(L"+ %2d) Rect (%2d,%2d) (%2d,%2d) '%s' Dbl=%d",I,Rect.left,Rect.top,Rect.right,Rect.bottom,Item[I].Data,mouse.Event.MouseEvent.dwEventFlags==DOUBLE_CLICK));
				if (DlgProc(this,DN_CONTROLINPUT,I,&mouse))
					return TRUE;

				if (!DialogMode.Check(DMODE_SHOW))
					return TRUE;

				if (Item[I]->Type == DI_USERCONTROL)
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

			for (I=ItemCount-1; I!=(unsigned)-1; I--)
			{
				//   Исключаем из списка оповещаемых о мыши недоступные элементы
				if (Item[I]->Flags&(DIF_DISABLE|DIF_HIDDEN))
					continue;

				Type=Item[I]->Type;

				GetItemRect(I,Rect);
				Rect.Left+=X1;  Rect.Top+=Y1;
				Rect.Right+=X1; Rect.Bottom+=Y1;
				if (ItemHasDropDownArrow(Item[I]))
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
						DlgEdit *EditLine=(DlgEdit *)(Item[I]->ObjPtr);
						EditLine->GetPosition(EditX1,EditY1,EditX2,EditY2);

						if (MsY==EditY1 && Type == DI_COMBOBOX &&
						        (Item[I]->Flags & DIF_DROPDOWNLIST) &&
						        MsX >= EditX1 && MsX <= EditX2+1)
						{
							EditLine->SetClearFlag(0);

							ChangeFocus2(I);
							ShowDialog();

							ProcessOpenComboBox(Item[I]->Type,Item[I],I);

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
							if (MsX==EditX2+1 && MsY==EditY1 && ItemHasDropDownArrow(Item[I]))
							{
								EditLine->SetClearFlag(0); // раз уж покусились на, то и...

								ChangeFocus2(I);

								if (!(Item[I]->Flags&DIF_HIDDEN))
									ShowDialog(I);

								ProcessOpenComboBox(Item[I]->Type,Item[I],I);

								return TRUE;
							}
						}
					}

					/* ********************************************************** */
					if (Type==DI_BUTTON &&
					        MsY==Y1+Item[I]->Y1 &&
					        MsX < X1+Item[I]->X1+HiStrlen(Item[I]->strData))
					{
						ChangeFocus2(I);
						ShowDialog();

						while (IsMouseButtonPressed());

						if (IntKeyState.MouseX <  X1 ||
						        IntKeyState.MouseX >  X1+Item[I]->X1+HiStrlen(Item[I]->strData)+4 ||
						        IntKeyState.MouseY != Y1+Item[I]->Y1)
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
					        MsY==Y1+Item[I]->Y1 &&
					        MsX < (X1+Item[I]->X1+HiStrlen(Item[I]->strData)+4-((Item[I]->Flags & DIF_MOVESELECT)!=0)))
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

								if (!DlgProc(this,DN_DRAGGED,0,0)) // а может нас обломали?
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
						DlgProc(this,DN_DRAGGED,1,ToPtr(TRUE));

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
							DlgProc(this,DN_DRAGGED,1,0);

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


int Dialog::ProcessOpenComboBox(FARDIALOGITEMTYPES Type,DialogItemEx *CurItem, unsigned CurFocusPos)
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
	        Opt.Dialogs.EditHistory &&
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

unsigned Dialog::ProcessRadioButton(unsigned CurRB)
{
	CriticalSectionLock Lock(CS);
	unsigned PrevRB=CurRB, J;
	unsigned I;

	for (I=CurRB;; I--)
	{
		if (!I)
			break;

		if (Item[I]->Type==DI_RADIOBUTTON && (Item[I]->Flags & DIF_GROUP))
			break;

		if (Item[I-1]->Type!=DI_RADIOBUTTON)
			break;
	}

	do
	{
		/* $ 28.07.2000 SVS
		  При изменении состояния каждого элемента посылаем сообщение
		  посредством функции SendDlgMessage - в ней делается все!
		*/
		J=Item[I]->Selected;
		Item[I]->Selected=0;

		if (J)
		{
			PrevRB=I;
		}

		++I;
	}
	while (I<ItemCount && Item[I]->Type==DI_RADIOBUTTON &&
	        !(Item[I]->Flags & DIF_GROUP));

	Item[CurRB]->Selected=1;

	unsigned ret = CurRB, focus = FocusPos;

	/* $ 28.07.2000 SVS
	  При изменении состояния каждого элемента посылаем сообщение
	  посредством функции SendDlgMessage - в ней делается все!
	*/
	if (!SendDlgMessage(this,DN_BTNCLICK,PrevRB,0) ||
		!SendDlgMessage(this,DN_BTNCLICK,CurRB,ToPtr(1)))
	{
		// вернем назад, если пользователь не захотел...
		Item[CurRB]->Selected=0;
		Item[PrevRB]->Selected=1;
		ret = PrevRB;
	}

	return (focus == FocusPos ? ret : FocusPos); // если фокус изменили - значит так надо!
}


int Dialog::Do_ProcessFirstCtrl()
{
	CriticalSectionLock Lock(CS);

	if (IsEdit(Item[FocusPos]->Type))
	{
		((DlgEdit *)(Item[FocusPos]->ObjPtr))->ProcessKey(KEY_HOME);
		return TRUE;
	}
	else
	{
		for (unsigned I=0; I<ItemCount; I++)
			if (CanGetFocus(Item[I]->Type))
			{
				ChangeFocus2(I);
				ShowDialog();
				break;
			}
	}

	return TRUE;
}

int Dialog::Do_ProcessNextCtrl(int Up,BOOL IsRedraw)
{
	CriticalSectionLock Lock(CS);
	unsigned OldPos=FocusPos;
	unsigned PrevPos=0;

	if (IsEdit(Item[FocusPos]->Type) && (Item[FocusPos]->Flags & DIF_EDITOR))
		PrevPos=((DlgEdit *)(Item[FocusPos]->ObjPtr))->GetCurPos();

	unsigned I=ChangeFocus(FocusPos,Up? -1:1,FALSE);
	Item[FocusPos]->Flags&=~DIF_FOCUS;
	Item[I]->Flags|=DIF_FOCUS;
	ChangeFocus2(I);

	if (IsEdit(Item[I]->Type) && (Item[I]->Flags & DIF_EDITOR))
		((DlgEdit *)(Item[I]->ObjPtr))->SetCurPos(PrevPos);

	if (Item[FocusPos]->Type == DI_RADIOBUTTON && (Item[I]->Flags & DIF_MOVESELECT))
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
	unsigned I;

	if (ItemCount > 1)
	{
		// Здесь с фокусом ОООЧЕНЬ ТУМАННО!!!
		if (Item[FocusPos]->Flags & DIF_EDITOR)
		{
			I=FocusPos;

			while (Item[I]->Flags & DIF_EDITOR)
				I=ChangeFocus(I,Next ? 1:-1,TRUE);
		}
		else
		{
			I=ChangeFocus(FocusPos,Next ? 1:-1,TRUE);

			if (!Next)
				while (I>0 && (Item[I]->Flags & DIF_EDITOR) &&
				        (Item[I-1]->Flags & DIF_EDITOR) &&
				        !((DlgEdit *)Item[I]->ObjPtr)->GetLength())
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

	if (Item[FocusPos]->Type==DI_CHECKBOX)
	{
		int OldSelected=Item[FocusPos]->Selected;

		if (Item[FocusPos]->Flags&DIF_3STATE)
			(++Item[FocusPos]->Selected)%=3;
		else
			Item[FocusPos]->Selected = !Item[FocusPos]->Selected;

		int OldFocusPos=FocusPos;

		if (!SendDlgMessage(this,DN_BTNCLICK,FocusPos,ToPtr(Item[FocusPos]->Selected)))
			Item[OldFocusPos]->Selected = OldSelected;

		ShowDialog();
		return TRUE;
	}
	else if (Item[FocusPos]->Type==DI_RADIOBUTTON)
	{
		FocusPos=ProcessRadioButton(FocusPos);
		ShowDialog();
		return TRUE;
	}
	else if (IsEdit(Item[FocusPos]->Type) && !(Item[FocusPos]->Flags & DIF_READONLY))
	{
		if (((DlgEdit *)(Item[FocusPos]->ObjPtr))->ProcessKey(KEY_SPACE))
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
unsigned Dialog::ChangeFocus(unsigned CurFocusPos,int Step,int SkipGroup)
{
	CriticalSectionLock Lock(CS);
	unsigned OrigFocusPos=CurFocusPos;
//  int FucusPosNeed=-1;
	// В функцию обработки диалога здесь передаем сообщение,
	//   что элемент - LostFocus() - теряет фокус ввода.
//  if(DialogMode.Check(DMODE_INITOBJECTS))
//    FucusPosNeed=DlgProc(this,DN_KILLFOCUS,FocusPos,0);
//  if(FucusPosNeed != -1 && CanGetFocus(Item[FucusPosNeed].Type))
//    FocusPos=FucusPosNeed;
//  else
	{
		for (;;)
		{
			CurFocusPos+=Step;

			if ((int)CurFocusPos<0)
				CurFocusPos=ItemCount-1;

			if (CurFocusPos>=ItemCount)
				CurFocusPos=0;

			FARDIALOGITEMTYPES Type=Item[CurFocusPos]->Type;

			if (!(Item[CurFocusPos]->Flags&(DIF_NOFOCUS|DIF_DISABLE|DIF_HIDDEN)))
			{
				if (Type==DI_LISTBOX || Type==DI_BUTTON || Type==DI_CHECKBOX || IsEdit(Type) || Type==DI_USERCONTROL)
					break;

				if (Type==DI_RADIOBUTTON && (!SkipGroup || Item[CurFocusPos]->Selected))
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
void Dialog::ChangeFocus2(unsigned SetFocusPos)
{
	CriticalSectionLock Lock(CS);

	if (!(Item[SetFocusPos]->Flags&(DIF_NOFOCUS|DIF_DISABLE|DIF_HIDDEN)))
	{
		int FocusPosNeed=-1;
		if (DialogMode.Check(DMODE_INITOBJECTS))
		{
			FocusPosNeed=(int)DlgProc(this,DN_KILLFOCUS,FocusPos,0);

			if (!DialogMode.Check(DMODE_SHOW))
				return;
		}

		if (FocusPosNeed != -1 && CanGetFocus(Item[FocusPosNeed]->Type))
			SetFocusPos=FocusPosNeed;

		Item[FocusPos]->Flags&=~DIF_FOCUS;

		// "снимать выделение при потере фокуса?"
		if (IsEdit(Item[FocusPos]->Type) &&
		        !(Item[FocusPos]->Type == DI_COMBOBOX && (Item[FocusPos]->Flags & DIF_DROPDOWNLIST)))
		{
			DlgEdit *EditPtr=(DlgEdit*)Item[FocusPos]->ObjPtr;
			EditPtr->GetSelection(Item[FocusPos]->SelStart,Item[FocusPos]->SelEnd);

			if ((Opt.Dialogs.EditLine&DLGEDITLINE_CLEARSELONKILLFOCUS))
			{
				EditPtr->Select(-1,0);
			}
		}

		Item[SetFocusPos]->Flags|=DIF_FOCUS;

		// "не восстанавливать выделение при получении фокуса?"
		if (IsEdit(Item[SetFocusPos]->Type) &&
		        !(Item[SetFocusPos]->Type == DI_COMBOBOX && (Item[SetFocusPos]->Flags & DIF_DROPDOWNLIST)))
		{
			DlgEdit *EditPtr=(DlgEdit*)Item[SetFocusPos]->ObjPtr;

			if (!(Opt.Dialogs.EditLine&DLGEDITLINE_NOTSELONGOTFOCUS))
			{
				if (Opt.Dialogs.EditLine&DLGEDITLINE_SELALLGOTFOCUS)
					EditPtr->Select(0,EditPtr->GetStrSize());
				else
					EditPtr->Select(Item[SetFocusPos]->SelStart,Item[SetFocusPos]->SelEnd);
			}
			else
			{
				EditPtr->Select(-1,0);
			}

			// при получении фокуса ввода переместить курсор в конец строки?
			if (Opt.Dialogs.EditLine&DLGEDITLINE_GOTOEOLGOTFOCUS)
			{
				EditPtr->SetCurPos(EditPtr->GetStrSize());
			}
		}

		//   проинформируем листбокс, есть ли у него фокус
		if (Item[FocusPos]->Type == DI_LISTBOX)
			Item[FocusPos]->ListPtr->ClearFlags(VMENU_LISTHASFOCUS);

		if (Item[SetFocusPos]->Type == DI_LISTBOX)
			Item[SetFocusPos]->ListPtr->SetFlags(VMENU_LISTHASFOCUS);

		SelectOnEntry(FocusPos,FALSE);
		SelectOnEntry(SetFocusPos,TRUE);

		PrevFocusPos=FocusPos;
		FocusPos=SetFocusPos;

		if (DialogMode.Check(DMODE_INITOBJECTS))
			DlgProc(this,DN_GOTFOCUS,FocusPos,0);
	}
}

/*
  Функция SelectOnEntry - выделение строки редактирования
  Обработка флага DIF_SELECTONENTRY
*/
void Dialog::SelectOnEntry(unsigned Pos,BOOL Selected)
{
	//if(!DialogMode.Check(DMODE_SHOW))
	//   return;
	if (IsEdit(Item[Pos]->Type) &&
	        (Item[Pos]->Flags&DIF_SELECTONENTRY)
//     && PrevFocusPos != -1 && PrevFocusPos != Pos
	   )
	{
		DlgEdit *edt=(DlgEdit *)Item[Pos]->ObjPtr;

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

	if (IDParent < ItemCount && (Item[IDParent]->Flags&DIF_AUTOMATION) &&
	        id < ItemCount && IDParent != id) // Сами себя не юзаем!
	{
		Ret = Item[IDParent]->AddAutomation(id, UncheckedSet, UncheckedSkip,
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
	//if((Str=(char*)xf_malloc(MaxLen)) )
	{
		CriticalSectionLock Lock(CS);
		//char *Str;
		string strStr;
		int I,Dest, OriginalPos;
		unsigned CurFocusPos=FocusPos;
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

		if (DlgProc(this,DN_CTLCOLORDLGLIST,CurItem->ID,&ListColors))
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
				if (DlgProc(this,DN_CONTROLINPUT,FocusPos,&ReadRec))
					continue;
			}
			else if (CurItem->IFlags.Check(DLGIIF_COMBOBOXEVENTMOUSE) && ReadRec.EventType == MOUSE_EVENT)
				if (!DlgProc(this,DN_INPUT,0,&ReadRec))
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
				if (!DlgProc(this,DN_LISTCHANGE,CurFocusPos,ToPtr(I)))
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
	//return KEY_ESC;
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
		VMenu HistoryMenu(L"",nullptr,0,Opt.Dialogs.CBoxMaxHeight,VMENU_ALWAYSSCROLLBAR|VMENU_COMBOBOX|VMENU_NOTCHANGE);
		HistoryMenu.SetFlags(VMENU_SHOWAMPERSAND);
		HistoryMenu.SetBoxType(SHORT_SINGLE_BOX);
		SetDropDownOpened(TRUE); // Установим флаг "открытия" комбобокса.
		// запомним (для прорисовки)
		CurItem->ListPtr=&HistoryMenu;
		ret = DlgHist->Select(HistoryMenu, Opt.Dialogs.CBoxMaxHeight, this, strStr);
		// забудим (не нужен)
		CurItem->ListPtr=nullptr;
		SetDropDownOpened(FALSE); // Установим флаг "закрытия" комбобокса.
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
	int I;
	FARDIALOGITEMFLAGS Flags;

	if (StartPos < 0)
		StartPos=0;

	for (I=StartPos; I < (int)ItemCount; I++)
	{
		Type=Item[I]->Type;
		Flags=Item[I]->Flags;

		if ((!IsEdit(Type) || (Type == DI_COMBOBOX && (Flags&DIF_DROPDOWNLIST))) && !(Flags & (DIF_SHOWAMPERSAND|DIF_DISABLE|DIF_HIDDEN)))
		{
			const wchar_t *ChPtr=wcschr(Item[I]->strData,L'&');

			if (ChPtr)
			{
				WORD Ch=ChPtr[1];

				if (Ch && Upper(CheckSymbol) == Upper(Ch))
					return I;
			}
			else if (!CheckSymbol)
				return I;
		}
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////////
/* Private:
   Если жмакнули Alt-???
*/
int Dialog::ProcessHighlighting(int Key,unsigned FocusPos,int Translate)
{
	CriticalSectionLock Lock(CS);
	FARDIALOGITEMTYPES Type;
	FARDIALOGITEMFLAGS Flags;

	INPUT_RECORD rec;
	if(!KeyToInputRecord(Key,&rec))
	{
		ClearStruct(rec);
	}

	for (unsigned I=0; I<ItemCount; I++)
	{
		Type=Item[I]->Type;
		Flags=Item[I]->Flags;

		if ((!IsEdit(Type) || (Type == DI_COMBOBOX && (Flags&DIF_DROPDOWNLIST))) &&
		        !(Flags & (DIF_SHOWAMPERSAND|DIF_DISABLE|DIF_HIDDEN)))
			if (IsKeyHighlighted(Item[I]->strData,Key,Translate))
			{
				int DisableSelect=FALSE;

				// Если ЭТО: DlgEdit(пред контрол) и DI_TEXT в одну строку, то...
				if (I>0 &&
				        Type==DI_TEXT &&                              // DI_TEXT
				        IsEdit(Item[I-1]->Type) &&                     // и редактор
				        Item[I]->Y1==Item[I-1]->Y1 &&                   // и оба в одну строку
				        (I+1 < ItemCount && Item[I]->Y1!=Item[I+1]->Y1)) // ...и следующий контрол в другой строке
				{
					// Сначала сообщим о случившемся факте процедуре обработки диалога, а потом...
					if (!DlgProc(this,DN_HOTKEY,I,&rec))
						break; // сказали не продолжать обработку...

					// ... если предыдущий контрол задизаблен или невидим, тогда выходим.
					if ((Item[I-1]->Flags&(DIF_DISABLE|DIF_HIDDEN)) ) // и не задисаблен
						break;

					I=ChangeFocus(I,-1,FALSE);
					DisableSelect=TRUE;
				}
				else if (Item[I]->Type==DI_TEXT      || Item[I]->Type==DI_VTEXT ||
				         Item[I]->Type==DI_SINGLEBOX || Item[I]->Type==DI_DOUBLEBOX)
				{
					if (I+1 < ItemCount) // ...и следующий контрол
					{
						// Сначала сообщим о случившемся факте процедуре обработки диалога, а потом...
						if (!DlgProc(this,DN_HOTKEY,I,&rec))
							break; // сказали не продолжать обработку...

						// ... если следующий контрол задизаблен или невидим, тогда выходим.
						if ((Item[I+1]->Flags&(DIF_DISABLE|DIF_HIDDEN)) ) // и не задисаблен
							break;

						I=ChangeFocus(I,1,FALSE);
						DisableSelect=TRUE;
					}
				}

				// Сообщим о случивщемся факте процедуре обработки диалога
				if (!DlgProc(this,DN_HOTKEY,I,&rec))
					break; // сказали не продолжать обработку...

				ChangeFocus2(I);
				ShowDialog();

				if ((Item[I]->Type==DI_CHECKBOX || Item[I]->Type==DI_RADIOBUTTON) &&
				        (!DisableSelect || (Item[I]->Flags & DIF_MOVESELECT)))
				{
					Do_ProcessSpace();
					return TRUE;
				}
				else if (Item[I]->Type==DI_BUTTON)
				{
					ProcessKey(KEY_ENTER, I);
					return TRUE;
				}
				// при ComboBox`е - "вываливаем" последний //????
				else if (Item[I]->Type==DI_COMBOBOX)
				{
					ProcessOpenComboBox(Item[I]->Type,Item[I],I);
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
	DialogItemEx *CurItem;
	int x1,x2,y1,y2;

	if (!DialogMode.Check(DMODE_CREATEOBJECTS))
		return;

	ScreenObject *DialogScrObject;

	for (unsigned I=0; I < ItemCount; I++)
	{
		CurItem=Item[I];
		FARDIALOGITEMTYPES Type=CurItem->Type;

		if ((CurItem->ObjPtr  && IsEdit(Type)) ||
		        (CurItem->ListPtr && Type == DI_LISTBOX))
		{
			if (Type == DI_LISTBOX)
				DialogScrObject=(ScreenObject *)CurItem->ListPtr;
			else
				DialogScrObject=(ScreenObject *)CurItem->ObjPtr;

			DialogScrObject->GetPosition(x1,y1,x2,y2);
			x1+=dx;
			x2+=dx;
			y1+=dy;
			y2+=dy;
			DialogScrObject->SetPosition(x1,y1,x2,y2);
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

	if (pSaveItemEx)
		for (unsigned i = 0; i < ItemCount; i++)
			pSaveItemEx[i] = *Item[i];

	if (TBE)
	{
		delete TBE;
	}
}

intptr_t Dialog::CloseDialog()
{
	CriticalSectionLock Lock(CS);
	GetDialogObjectsData();

	intptr_t result=DlgProc(this,DN_CLOSE,ExitCode,0);
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

	if (HelpTopic)
		delete[] HelpTopic;

	HelpTopic=nullptr;

	if (Topic && *Topic)
	{
		HelpTopic = new wchar_t [wcslen(Topic)+1];

		if (HelpTopic)
			wcscpy(HelpTopic, Topic);
	}
}

void Dialog::ShowHelp()
{
	CriticalSectionLock Lock(CS);

	if (HelpTopic && *HelpTopic)
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


int Dialog::GetMacroMode()
{
	return MACRO_DIALOG;
}

int Dialog::FastHide()
{
	return Opt.AllCtrlAltShiftRule & CASR_DIALOG;
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
	SendDlgMessage(static_cast<HANDLE>(this), DN_RESIZECONSOLE, 0, &c);

	int x1, y1, x2, y2;
	GetPosition(x1, y1, x2, y2);
	c.X = Min(x1, ScrX-1);
	c.Y = Min(y1, ScrY-1);
	if(c.X!=x1 || c.Y!=y1)
	{
		c.X = x1;
		c.Y = y1;
		SendDlgMessage(static_cast<HANDLE>(this), DM_MOVEDIALOG, TRUE, &c);
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

intptr_t WINAPI Dialog::DlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	if (DialogMode.Check(DMODE_ENDLOOP))
		return 0;

	intptr_t Result;
	FarDialogEvent de={hDlg,Msg,Param1,Param2,0};

	if(!static_cast<Dialog*>(hDlg)->CheckDialogMode(DMODE_NOPLUGINS))
	{
		if (CtrlObject->Plugins->ProcessDialogEvent(DE_DLGPROCINIT,&de))
			return de.Result;
	}
	Result=RealDlgProc(hDlg,Msg,Param1,Param2);
	if(!static_cast<Dialog*>(hDlg)->CheckDialogMode(DMODE_NOPLUGINS))
	{
		de.Result=Result;
		if (CtrlObject->Plugins->ProcessDialogEvent(DE_DLGPROCEND,&de))
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
intptr_t WINAPI DefDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	_DIALOG(CleverSysLog CL(L"Dialog.DefDlgProc()"));
	_DIALOG(SysLog(L"hDlg=%p, Msg=%s, Param1=%d (0x%08X), Param2=%d (0x%08X)",hDlg,_DLGMSG_ToName(Msg),Param1,Param1,Param2,Param2));

	if (!hDlg || hDlg==INVALID_HANDLE_VALUE)
		return 0;

	FarDialogEvent de={hDlg,Msg,Param1,Param2,0};

	if(!static_cast<Dialog*>(hDlg)->CheckDialogMode(DMODE_NOPLUGINS))
	{
		if (CtrlObject->Plugins->ProcessDialogEvent(DE_DEFDLGPROCINIT,&de))
		{
			return de.Result;
		}
	}
	Dialog* Dlg=(Dialog*)hDlg;
	CriticalSectionLock Lock(Dlg->CS);
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
				Text(Dlg->X1,Dlg->Y1,Color,L"\\");
				Text(Dlg->X1,Dlg->Y2,Color,L"/");
				Text(Dlg->X2,Dlg->Y1,Color,L"/");
				Text(Dlg->X2,Dlg->Y2,Color,L"\\");
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
#ifdef FAR_LUA
				{
					DialogInfo *di=reinterpret_cast<DialogInfo*>(Param2);

					if (CheckStructSize(di))
					{
						di->Id = Dlg->IdExist ? Dlg->Id : *(GUID*)("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
						di->Owner=FarGuid;
						Result=true;
						if (Dlg->PluginOwner)
						{
							di->Owner = Dlg->PluginOwner->GetGUID();
						}
						if (di->StructSize >= offsetof(DialogInfo,ItemsNumber) + sizeof(di->ItemsNumber))
							di->ItemsNumber = Dlg->ItemCount;
						if (di->StructSize >= offsetof(DialogInfo,FocusPos) + sizeof(di->FocusPos))
							di->FocusPos = Dlg->FocusPos;
						if (di->StructSize >= offsetof(DialogInfo,PrevFocusPos) + sizeof(di->PrevFocusPos))
							di->PrevFocusPos = Dlg->PrevFocusPos;
					}
				}
#else
				if (Dlg->IdExist)
				{
					DialogInfo *di=reinterpret_cast<DialogInfo*>(Param2);

					if (CheckStructSize(di))
					{
						di->Id=Dlg->Id;
						di->Owner=FarGuid;
						Result=true;
						if (Dlg->PluginOwner)
						{
							di->Owner = Dlg->PluginOwner->GetGUID();
						}
					}
				}
#endif
			}

			return Result;
		}
		default:
			break;
	}

	// предварительно проверим...
	if ((unsigned)Param1 >= Dlg->ItemCount && Dlg->Item)
		return 0;

	if (Param1>=0)
	{
		CurItem=Dlg->Item[Param1];
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

intptr_t Dialog::CallDlgProc(int nMsg, int nParam1, void* nParam2)
{
	CriticalSectionLock Lock(CS);
	return DlgProc(this, nMsg, nParam1, nParam2);
}

//////////////////////////////////////////////////////////////////////////
/* $ 28.07.2000 SVS
   Посылка сообщения диалогу
   Некоторые сообщения эта функция обрабатывает сама, не передавая управление
   обработчику диалога.
*/
intptr_t WINAPI SendDlgMessage(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	if (!hDlg)
		return 0;

	Dialog* Dlg=(Dialog*)hDlg;
	CriticalSectionLock Lock(Dlg->CS);
	_DIALOG(CleverSysLog CL(L"Dialog.SendDlgMessage()"));
	_DIALOG(SysLog(L"hDlg=%p, Msg=%s, Param1=%d (0x%08X), Param2=%d (0x%08X)",hDlg,_DLGMSG_ToName(Msg),Param1,Param1,Param2,Param2));

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
			W1=Dlg->X2-Dlg->X1+1;
			H1=Dlg->Y2-Dlg->Y1+1;
			Dlg->OldX1=Dlg->X1;
			Dlg->OldY1=Dlg->Y1;
			Dlg->OldX2=Dlg->X2;
			Dlg->OldY2=Dlg->Y2;

			// переместили
			if (Param1>0)  // абсолютно?
			{
				Dlg->X1=((COORD*)Param2)->X;
				Dlg->Y1=((COORD*)Param2)->Y;
				Dlg->X2=W1;
				Dlg->Y2=H1;
				Dlg->CheckDialogCoord();
			}
			else if (!Param1)  // значит относительно
			{
				Dlg->X1+=((COORD*)Param2)->X;
				Dlg->Y1+=((COORD*)Param2)->Y;
			}
			else // Resize, Param2=width/height
			{
				int OldW1,OldH1;
				OldW1=W1;
				OldH1=H1;
				W1=((COORD*)Param2)->X;
				H1=((COORD*)Param2)->Y;
				Dlg->RealWidth = W1;
				Dlg->RealHeight = H1;

				if (W1<OldW1 || H1<OldH1)
				{
					Dlg->DialogMode.Set(DMODE_DRAWING);
					DialogItemEx *Item;
					SMALL_RECT Rect;

					for (unsigned int I=0; I<Dlg->ItemCount; I++)
					{
						Item=Dlg->Item[I];

						if (Item->Flags&DIF_HIDDEN)
							continue;

						Rect.Left=Item->X1;
						Rect.Top=Item->Y1;

						if (Item->X2>=W1)
						{
							Rect.Right=Item->X2-(OldW1-W1);
							Rect.Bottom=Item->Y2;
							Dlg->SetItemRect(I,&Rect);
						}

						if (Item->Y2>=H1)
						{
							Rect.Right=Item->X2;
							Rect.Bottom=Item->Y2-(OldH1-H1);
							Dlg->SetItemRect(I,&Rect);
						}
					}

					Dlg->DialogMode.Clear(DMODE_DRAWING);
				}
			}

			// проверили и скорректировали
			if (Dlg->X1+W1<0)
				Dlg->X1=-W1+1;

			if (Dlg->Y1+H1<0)
				Dlg->Y1=-H1+1;

			if (Dlg->X1>ScrX)
				Dlg->X1=ScrX;

			if (Dlg->Y1>ScrY)
				Dlg->Y1=ScrY;

			Dlg->X2=Dlg->X1+W1-1;
			Dlg->Y2=Dlg->Y1+H1-1;

			if (Param1>0)  // абсолютно?
			{
				Dlg->CheckDialogCoord();
			}

			if (Param1 < 0)  // размер?
			{
				((COORD*)Param2)->X=Dlg->X2-Dlg->X1+1;
				((COORD*)Param2)->Y=Dlg->Y2-Dlg->Y1+1;
			}
			else
			{
				((COORD*)Param2)->X=Dlg->X1;
				((COORD*)Param2)->Y=Dlg->Y1;
			}

			int I=Dlg->IsVisible();// && Dlg->DialogMode.Check(DMODE_INITOBJECTS);

			if (I) Dlg->Hide();

			// приняли.
			Dlg->AdjustEditPos(Dlg->X1-Dlg->OldX1,Dlg->Y1-Dlg->OldY1);

			if (I) Dlg->Show(); // только если диалог был виден

			return reinterpret_cast<intptr_t>(Param2);
		}
		/*****************************************************************/
		case DM_REDRAW:
		{
			if (Dlg->DialogMode.Check(DMODE_INITOBJECTS))
				Dlg->Show();

			return 0;
		}
		/*****************************************************************/
		case DM_ENABLEREDRAW:
		{
			int Prev=Dlg->IsEnableRedraw;

			if (Param1 == TRUE)
				Dlg->IsEnableRedraw++;
			else if (Param1 == FALSE)
				Dlg->IsEnableRedraw--;

			//Edit::DisableEditOut(!Dlg->IsEnableRedraw?FALSE:TRUE);

			if (!Dlg->IsEnableRedraw && Prev != Dlg->IsEnableRedraw)
				if (Dlg->DialogMode.Check(DMODE_INITOBJECTS))
				{
					Dlg->ShowDialog();
//          Dlg->Show();
					ScrBuf.Flush();
				}

			return Prev;
		}
		/*
		    case DM_ENABLEREDRAW:
		    {
		      if(Param1)
		        Dlg->IsEnableRedraw++;
		      else
		        Dlg->IsEnableRedraw--;

		      if(!Dlg->IsEnableRedraw)
		        if(Dlg->DialogMode.Check(DMODE_INITOBJECTS))
		        {
		          Dlg->ShowDialog();
		          ScrBuf.Flush();
		//          Dlg->Show();
		        }
		      return 0;
		    }
		*/
		/*****************************************************************/
		case DM_SHOWDIALOG:
		{
//      if(!Dlg->IsEnableRedraw)
			{
				if (Param1)
				{
					/* $ 20.04.2002 KM
					  Залочим прорисовку при прятании диалога, в противном
					  случае ОТКУДА менеджер узнает, что отрисовывать
					  объект нельзя!
					*/
					if (!Dlg->IsVisible())
					{
						Dlg->Unlock();
						Dlg->Show();
					}
				}
				else
				{
					if (Dlg->IsVisible())
					{
						Dlg->Hide();
						Dlg->Lock();
					}
				}
			}
			return 0;
		}
		/*****************************************************************/
		case DM_SETDLGDATA:
		{
			void* PrewDataDialog=Dlg->DataDialog;
			Dlg->DataDialog=Param2;
			return reinterpret_cast<intptr_t>(PrewDataDialog);
		}
		/*****************************************************************/
		case DM_GETDLGDATA:
		{
			return reinterpret_cast<intptr_t>(Dlg->DataDialog);
		}
		/*****************************************************************/
		case DM_KEY:
		{
			const INPUT_RECORD *KeyArray=(const INPUT_RECORD *)Param2;
			Dlg->DialogMode.Set(DMODE_KEY);

			for (unsigned int I=0; I < (unsigned)Param1; ++I)
				Dlg->ProcessKey(InputRecordToKey(KeyArray+I));

			Dlg->DialogMode.Clear(DMODE_KEY);
			return 0;
		}
		/*****************************************************************/
		case DM_CLOSE:
		{
			if (Param1 == -1)
				Dlg->ExitCode=Dlg->FocusPos;
			else
				Dlg->ExitCode=Param1;

			return Dlg->CloseDialog();
		}
		/*****************************************************************/
		case DM_GETDLGRECT:
		{
			if (Param2)
			{
				int x1,y1,x2,y2;
				Dlg->GetPosition(x1,y1,x2,y2);
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
			return Dlg->GetDropDownOpened();
		}
		/*****************************************************************/
		case DM_KILLSAVESCREEN:
		{
			if (Dlg->SaveScr) Dlg->SaveScr->Discard();

			if (Dlg->ShadowSaveScr) Dlg->ShadowSaveScr->Discard();

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
				return IsProcessAssignMacroKey;

			BOOL OldIsProcessAssignMacroKey=IsProcessAssignMacroKey;
			IsProcessAssignMacroKey=Param1;
			return OldIsProcessAssignMacroKey;
		}
		/*****************************************************************/
		case DM_SETMOUSEEVENTNOTIFY: // Param1 = 1 on, 0 off, -1 - get
		{
			int State=Dlg->DialogMode.Check(DMODE_MOUSEEVENT)?TRUE:FALSE;

			if (Param1 != -1)
			{
				if (!Param1)
					Dlg->DialogMode.Clear(DMODE_MOUSEEVENT);
				else
					Dlg->DialogMode.Set(DMODE_MOUSEEVENT);
			}

			return State;
		}
		/*****************************************************************/
		case DN_RESIZECONSOLE:
		{
			return Dlg->CallDlgProc(Msg,Param1,Param2);
		}
		case DM_GETDIALOGINFO:
		{
			return DefDlgProc(hDlg,DM_GETDIALOGINFO,Param1,Param2);
		}
		default:
			break;
	}

	/*****************************************************************/
	if (Msg >= DM_USER)
	{
		return Dlg->CallDlgProc(Msg,Param1,Param2);
	}

	/*****************************************************************/
	DialogItemEx *CurItem=nullptr;
	FARDIALOGITEMTYPES Type=DI_TEXT;
	size_t Len=0;

	// предварительно проверим...
	/* $ 09.12.2001 DJ
	   для DM_USER проверять _не_надо_!
	*/
	if ((unsigned)Param1 >= Dlg->ItemCount || !Dlg->Item)
		return 0;

//  CurItem=&Dlg->Item[Param1];
	CurItem=Dlg->Item[Param1];
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
							ListBox->SortItems(static_cast<int>(reinterpret_cast<intptr_t>(Param2)));
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

							if (!ListItems)
								return FALSE;

							Ret=ListBox->AddItem(ListItems);
							break;
						}
						case DM_LISTDELETE: // Param1=ID Param2=FarListDelete: StartIndex=BeginIndex, Count=количество (<=0 - все!)
						{
							FarListDelete *ListItems=(FarListDelete *)Param2;
							if(nullptr==ListItems || CheckStructSize(ListItems))
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

							if (!ListItems)
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
									if (!Dlg->CallDlgProc(DN_LISTCHANGE,Param1,ToPtr(Ret)))
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
							int OldSets=CurItem->IFlags.Flags;
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

					if (Dlg->DialogMode.Check(DMODE_SHOW) && ListBox->UpdateRequired())
					{
						Dlg->ShowDialog(Param1);
						ScrBuf.Flush();
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
						Dlg->ProcessLastHistory(CurItem, Param1);
					}
				}
				else
				{
					CurItem->Flags&=~DIF_HISTORY;
					CurItem->strHistory.Clear();
				}

				if (Dlg->DialogMode.Check(DMODE_SHOW))
				{
					Dlg->ShowDialog(Param1);
					ScrBuf.Flush();
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
				return Dlg->AddToEditHistory(CurItem, (const wchar_t*)Param2);
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
				Dlg->ShowDialog(Param1);
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
				if (Dlg->DialogMode.Check(DMODE_SHOW) && Dlg->FocusPos == (unsigned)Param1)
				{
					// что-то одно надо убрать :-)
					MoveCursor(Coord.X+Dlg->X1,Coord.Y+Dlg->Y1); // ???
					Dlg->ShowDialog(Param1); //???
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
					DlgEdit *EditPtr=(DlgEdit *)(CurItem->ObjPtr);
					EditPtr->SetCurPos(esp->CurPos);
					EditPtr->SetTabCurPos(esp->CurTabPos);
					EditPtr->SetLeftPos(esp->LeftPos);
					EditPtr->SetOvertypeMode(esp->Overtype);
					Dlg->ShowDialog(Param1);
					ScrBuf.Flush();
					return TRUE;
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

				if (Dlg->DialogMode.Check(DMODE_SHOW) &&
				        Dlg->FocusPos == (unsigned)Param1 &&
				        CCX != -1 && CCY != -1)
					SetCursorType(CurItem->UCData->CursorVisible,CurItem->UCData->CursorSize);
			}

			return MAKELONG(Visible,Size);
		}
		/*****************************************************************/
		case DN_LISTCHANGE:
		{
			return Dlg->CallDlgProc(Msg,Param1,Param2);
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
					I=Dlg->CallDlgProc(DN_EDITCHANGE,Param1,Item.Item);
					if (I)
					{
						if (Type == DI_COMBOBOX && CurItem->ListPtr)
							CurItem->ListPtr->ChangeFlags(VMENU_DISABLED,CurItem->Flags&DIF_DISABLE);
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
			intptr_t Ret=Dlg->CallDlgProc(Msg,Param1,Param2);

			if (Ret && (CurItem->Flags&DIF_AUTOMATION) && CurItem->AutoCount && CurItem->AutoPtr)
			{
				DialogItemAutomation* Auto=CurItem->AutoPtr;
				intptr_t iParam = reinterpret_cast<intptr_t>(Param2);
				iParam%=3;

				for (UINT I=0; I < CurItem->AutoCount; ++I, ++Auto)
				{
					FARDIALOGITEMFLAGS NewFlags=Dlg->Item[Auto->ID]->Flags;
					Dlg->Item[Auto->ID]->Flags=(NewFlags&(~Auto->Flags[iParam][1]))|Auto->Flags[iParam][0];
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

				if (Selected != State && Dlg->DialogMode.Check(DMODE_SHOW))
				{
					// автоматизация
					if ((CurItem->Flags&DIF_AUTOMATION) && CurItem->AutoCount && CurItem->AutoPtr)
					{
						DialogItemAutomation* Auto=CurItem->AutoPtr;
						State%=3;

						for (UINT I=0; I < CurItem->AutoCount; ++I, ++Auto)
						{
							FARDIALOGITEMFLAGS NewFlags=Dlg->Item[Auto->ID]->Flags;
							Dlg->Item[Auto->ID]->Flags=(NewFlags&(~Auto->Flags[State][1]))|Auto->Flags[State][0];
							// здесь намеренно в обработчик не посылаются эвенты об изменении
							// состояния...
						}

						Param1=-1;
					}

					Dlg->ShowDialog(Param1);
					ScrBuf.Flush();
				}

				return Selected;
			}
			else if (Type == DI_RADIOBUTTON)
			{
				Param1=Dlg->ProcessRadioButton(Param1);

				if (Dlg->DialogMode.Check(DMODE_SHOW))
				{
					Dlg->ShowDialog();
					ScrBuf.Flush();
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
				I=Dlg->CallDlgProc(Msg,Param1,Item.Item);

				if ((Type == DI_LISTBOX || Type == DI_COMBOBOX) && CurItem->ListPtr)
					CurItem->ListPtr->ChangeFlags(VMENU_DISABLED,CurItem->Flags&DIF_DISABLE);
			}
			xf_free(Item.Item);
			return I;
		}
		/*****************************************************************/
		case DM_SETFOCUS:
		{
			if (!CanGetFocus(Type))
				return FALSE;

			if (Dlg->FocusPos == (unsigned)Param1) // уже и так установлено все!
				return TRUE;

			Dlg->ChangeFocus2(Param1);

			if (Dlg->FocusPos == (unsigned)Param1)
			{
				Dlg->ShowDialog();
				return TRUE;
			}

			return FALSE;
		}
		/*****************************************************************/
		case DM_GETFOCUS: // Получить ID фокуса
		{
			return Dlg->FocusPos;
		}
		/*****************************************************************/
		case DM_GETCONSTTEXTPTR:
		{
			return (intptr_t)Ptr;
		}
		/*****************************************************************/
		case DM_GETTEXTPTR:

			if (Param2)
			{
				FarDialogItemData IData={sizeof(FarDialogItemData),0,(wchar_t *)Param2};
				return SendDlgMessage(hDlg,DM_GETTEXT,Param1,&IData);
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
						Len=StrLength(Ptr)+1;

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
							Len=did->PtrLength+1; // Прибавим 1, чтобы учесть нулевой байт.

						if (Len > 0 && did->PtrData)
						{
							wmemmove(did->PtrData,Ptr,Len);
							did->PtrData[Len-1]=0;
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

				return Len-(!Len?0:1);
			}

			// здесь умышленно не ставим return, т.к. хотим получить размер
			// следовательно сразу должен идти "case DM_GETTEXTLENGTH"!!!
			/*****************************************************************/
		}
		case DM_GETTEXTLENGTH:
		{
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
			return SendDlgMessage(hDlg,DM_SETTEXT,Param1,&IData);
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

						if (Dlg->DialogMode.Check(DMODE_SHOW))
						{
							if (!Dlg->DialogMode.Check(DMODE_KEEPCONSOLETITLE))
								ConsoleTitle::SetFarTitle(Dlg->GetDialogTitle());
							Dlg->ShowDialog(Param1);
							ScrBuf.Flush();
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
							int ReadOnly=EditLine->GetReadOnly();
							EditLine->SetReadOnly(0);
							{
								SetAutocomplete da(EditLine);
								EditLine->SetString(CurItem->strData);
							}
							EditLine->SetReadOnly(ReadOnly);

							if (Dlg->DialogMode.Check(DMODE_INITOBJECTS)) // не меняем клеар-флаг, пока не проиницализировались
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
								SendDlgMessage(hDlg,DM_LISTUPDATE,Param1,&LUpdate);
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
					Dlg->InitDialogObjects(Param1); // переинициализируем элементы диалога

				if (Dlg->DialogMode.Check(DMODE_SHOW)) // достаточно ли этого????!!!!
				{
					Dlg->ShowDialog(Param1);
					ScrBuf.Flush();
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
				Dlg->InitDialogObjects(Param1); // переинициализируем элементы диалога
				if (!Dlg->DialogMode.Check(DMODE_KEEPCONSOLETITLE))
					ConsoleTitle::SetFarTitle(Dlg->GetDialogTitle());
				return MaxLen;
			}

			return 0;
		}
		/*****************************************************************/
		case DM_GETDLGITEM:
		{
			FarGetDialogItem* Item = (FarGetDialogItem*)Param2;
			return (!Item||CheckStructSize(Item))?(intptr_t)ConvertItemEx2(CurItem, Item):0;
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
				CurItem->ListPtr->ChangeFlags(VMENU_DISABLED,CurItem->Flags&DIF_DISABLE);

			// еще разок, т.к. данные могли быть изменены
			Dlg->InitDialogObjects(Param1);
			if (!Dlg->DialogMode.Check(DMODE_KEEPCONSOLETITLE))
				ConsoleTitle::SetFarTitle(Dlg->GetDialogTitle());

			if (Dlg->DialogMode.Check(DMODE_SHOW))
			{
				Dlg->ShowDialog(Param1);
				ScrBuf.Flush();
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

				if (Dlg->DialogMode.Check(DMODE_SHOW))// && (PrevFlags&DIF_HIDDEN) != (CurItem->Flags&DIF_HIDDEN))//!(CurItem->Flags&DIF_HIDDEN))
				{
					if ((CurItem->Flags&DIF_HIDDEN) && Dlg->FocusPos == (unsigned)Param1)
					{
						Dlg->ChangeFocus2(Dlg->ChangeFocus(Param1,1,TRUE));
					}

					// Либо все,  либо... только 1
					Dlg->ShowDialog(Dlg->GetDropDownOpened()||(CurItem->Flags&DIF_HIDDEN)?-1:Param1);
					ScrBuf.Flush();
				}
			}

			return (PrevFlags&DIF_HIDDEN)?FALSE:TRUE;
		}
		/*****************************************************************/
		case DM_SETDROPDOWNOPENED: // Param1=ID; Param2={TRUE|FALSE}
		{
			if (!Param2) // Закрываем любой открытый комбобокс или историю
			{
				if (Dlg->GetDropDownOpened())
				{
					Dlg->SetDropDownOpened(FALSE);
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
				if (Dlg->GetDropDownOpened())
				{
					Dlg->SetDropDownOpened(FALSE);
					Sleep(10);
				}

				if (SendDlgMessage(hDlg,DM_SETFOCUS,Param1,0))
				{
					Dlg->ProcessOpenComboBox(Type,CurItem,Param1); //?? Param1 ??
					//Dlg->ProcessKey(KEY_CTRLDOWN);
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
			return Dlg->SetItemRect((int)Param1,(SMALL_RECT*)Param2);
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
					CurItem->ListPtr->ChangeFlags(VMENU_DISABLED,CurItem->Flags&DIF_DISABLE);
			}

			if (Dlg->DialogMode.Check(DMODE_SHOW)) //???
			{
				Dlg->ShowDialog(Param1);
				ScrBuf.Flush();
			}

			return (PrevFlags&DIF_DISABLE)?FALSE:TRUE;
		}
		/*****************************************************************/
		// получить позицию и размеры контрола
		case DM_GETITEMPOSITION: // Param1=ID, Param2=*SMALL_RECT

			if (Param2)
			{
				SMALL_RECT Rect;
				if (Dlg->GetItemRect(Param1,Rect))
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
					EditLine->SetClearFlag(static_cast<int>(reinterpret_cast<intptr_t>(Param2)));
					EditLine->Select(-1,0); // снимаем выделение

					if (Dlg->DialogMode.Check(DMODE_SHOW)) //???
					{
						Dlg->ShowDialog(Param1);
						ScrBuf.Flush();
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
			if (IsEdit(Type) && Param2)
			{
				if (Msg == DM_GETSELECTION)
				{
					EditorSelect *EdSel=(EditorSelect *)Param2;
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
					if (Param2)
					{
						EditorSelect *EdSel=(EditorSelect *)Param2;
						DlgEdit *EditLine=(DlgEdit *)(CurItem->ObjPtr);

						//EdSel->BlockType=BTYPE_STREAM;
						//EdSel->BlockStartLine=0;
						//EdSel->BlockHeight=1;
						if (EdSel->BlockType==BTYPE_NONE)
							EditLine->Select(-1,0);
						else
							EditLine->Select(EdSel->BlockStartPos,EdSel->BlockStartPos+EdSel->BlockWidth);

						if (Dlg->DialogMode.Check(DMODE_SHOW)) //???
						{
							Dlg->ShowDialog(Param1);
							ScrBuf.Flush();
						}

						return TRUE;
					}
				}
			}

			break;
		}
		default:
			break;
	}

	// Все, что сами не отрабатываем - посылаем на обработку обработчику.
	return Dlg->CallDlgProc(Msg,Param1,Param2);
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

	ScreenObject::SetPosition(X1, Y1, X2, Y2);
}
//////////////////////////////////////////////////////////////////////////
BOOL Dialog::IsInited()
{
	CriticalSectionLock Lock(CS);
	return DialogMode.Check(DMODE_INITOBJECTS);
}

void Dialog::SetComboBoxPos(DialogItemEx* CurItem)
{
	if (GetDropDownOpened())
	{
		if(!CurItem)
		{
			CurItem=Item[FocusPos];
		}
		int EditX1,EditY1,EditX2,EditY2;
		((DlgEdit*)CurItem->ObjPtr)->GetPosition(EditX1,EditY1,EditX2,EditY2);

		if (EditX2-EditX1<20)
			EditX2=EditX1+20;

		if (ScrY-EditY1<Min(Opt.Dialogs.CBoxMaxHeight.Get(),CurItem->ListPtr->GetItemCount())+2 && EditY1>ScrY/2)
			CurItem->ListPtr->SetPosition(EditX1,Max(0,EditY1-1-Min(Opt.Dialogs.CBoxMaxHeight.Get(),CurItem->ListPtr->GetItemCount())-1),EditX2,EditY1-1);
		else
			CurItem->ListPtr->SetPosition(EditX1,EditY1+1,EditX2,0);
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
