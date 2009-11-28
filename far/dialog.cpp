/*
dialog.cpp

Класс диалога
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

#include "dialog.hpp"
#include "lang.hpp"
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
#include "registry.hpp"
#include "TaskBar.hpp"
#include "interf.hpp"
#include "palette.hpp"
#include "message.hpp"
#include "strmix.hpp"
#include "history.hpp"

#define VTEXT_ADN_SEPARATORS	1

// Флаги для функции ConvertItem
enum CVTITEMFLAGS {
	CVTITEM_TOPLUGIN        = 0,
	CVTITEM_FROMPLUGIN      = 1,
	CVTITEM_TOPLUGINSHORT   = 2,
	CVTITEM_FROMPLUGINSHORT = 3
};

enum DLGEDITLINEFLAGS {
  DLGEDITLINE_CLEARSELONKILLFOCUS = 0x00000001, // управляет выделением блока при потере фокуса ввода
  DLGEDITLINE_SELALLGOTFOCUS      = 0x00000002, // управляет выделением блока при получении фокуса ввода
  DLGEDITLINE_NOTSELONGOTFOCUS    = 0x00000004, // не восстанавливать выделение строки редактирования при получении фокуса ввода
  DLGEDITLINE_NEWSELONGOTFOCUS    = 0x00000008, // управляет процессом выделения блока при получении фокуса
  DLGEDITLINE_GOTOEOLGOTFOCUS     = 0x00000010, // при получении фокуса ввода переместить курсор в конец строки
  DLGEDITLINE_PERSISTBLOCK        = 0x00000020, // постоянные блоки в строках ввода
  DLGEDITLINE_AUTOCOMPLETE        = 0x00000040, // автозавершение в строках ввода
  DLGEDITLINE_AUTOCOMPLETECTRLEND = 0x00000040, // при автозавершение подтверждать комбинацией Ctrl-End
  DLGEDITLINE_HISTORY             = 0x00000100, // история в строках ввода диалогов
};

enum DLGITEMINTERNALFLAGS {
  DLGIIF_LISTREACTIONFOCUS        = 0x00000001, // MouseReaction для фокусного элемента
  DLGIIF_LISTREACTIONNOFOCUS      = 0x00000002, // MouseReaction для не фокусного элемента
  DLGIIF_EDITPATH                 = 0x00000004, // здесь Ctrl-End в строке редактирования будет выдавать на гора автодополнение существующих путей в дополнении к выбору из истории
  DLGIIF_COMBOBOXNOREDRAWEDIT     = 0x00000008, // не прорисовывать строку редактирования при изменениях в комбо
  DLGIIF_COMBOBOXEVENTKEY         = 0x00000010, // посылать события клавиатуры в диалоговую проц. для открытого комбобокса
  DLGIIF_COMBOBOXEVENTMOUSE       = 0x00000020, // посылать события мыши в диалоговую проц. для открытого комбобокса
  DLGIIF_EDITCHANGEPROCESSED      = 0x00000040, // элемент обрабатывает событие DN_EDITCHANGE
};

static const wchar_t fmtSavedDialogHistory[]=L"SavedDialogHistory\\";

//////////////////////////////////////////////////////////////////////////
/*
   Функция, определяющая - "Может ли элемент диалога иметь фокус ввода"
*/
static inline bool CanGetFocus(int Type)
{
    switch(Type) {
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
  if(AmpPos == -1)
  {
    if ((Str=wcschr(Str,L'&'))==NULL)
      return(FALSE);
    AmpPos=1;
  }
  else
  {
    if(AmpPos >= StrLength(Str))
      return FALSE;
    Str=Str+AmpPos;
    AmpPos=0;
    if(Str[AmpPos] == L'&')
      AmpPos++;
  }
  int UpperStrKey=Upper ((int)Str[AmpPos]);
  if (Key < 0xFFFF)
  {
    return UpperStrKey == (int)Upper(Key) || (Translate && KeyToKeyLayoutCompare(Key,UpperStrKey));
  }

  if(Key&KEY_ALT)
  {
    int AltKey=Key&(~KEY_ALT);
    if(AltKey < 0xFFFF)
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

void DialogItemExToDialogItemEx (DialogItemEx *pSrc, DialogItemEx *pDest)
{
    pDest->Type = pSrc->Type;
    pDest->X1 = pSrc->X1;
    pDest->Y1 = pSrc->Y1;
    pDest->X2 = pSrc->X2;
    pDest->Y2 = pSrc->Y2;

    pDest->Focus = pSrc->Focus;

    pDest->History = pSrc->History;
    pDest->Flags = pSrc->Flags;
    pDest->DefaultButton = pSrc->DefaultButton;

    pDest->nMaxLength = 0;
    pDest->strData = pSrc->strData;

    pDest->ID = pSrc->ID;
    pDest->IFlags = pSrc->IFlags;
    pDest->AutoCount = pSrc->AutoCount;
    pDest->AutoPtr = pSrc->AutoPtr;
    pDest->UserData = pSrc->UserData;
    pDest->ObjPtr = pSrc->ObjPtr;
    pDest->ListPtr = pSrc->ListPtr;
    pDest->UCData = pSrc->UCData;

    pDest->SelStart = pSrc->SelStart;
    pDest->SelEnd = pSrc->SelEnd;
}

void ConvertItemSmall(FarDialogItem *Item,DialogItemEx *Data)
{
	Item->Type = Data->Type;
	Item->X1 = Data->X1;
	Item->Y1 = Data->Y1;
	Item->X2 = Data->X2;
	Item->Y2 = Data->Y2;
	Item->Focus = Data->Focus;

	Item->Param.History = Data->History;
	Item->Flags = Data->Flags;
	Item->DefaultButton = Data->DefaultButton;
	Item->MaxLen = Data->nMaxLength;
	Item->PtrData = NULL;
}

size_t ItemStringAndSize(DialogItemEx *Data,string& ItemString)
{
	//TODO: тут видимо надо сделать поумнее
	ItemString=Data->strData;
	if (IsEdit(Data->Type))
	{
		DlgEdit *EditPtr;
		if ((EditPtr = (DlgEdit *)(Data->ObjPtr)) != NULL)
			EditPtr->GetString(ItemString);
	}
	size_t sz = ItemString.GetLength();
	if (sz > Data->nMaxLength && Data->nMaxLength > 0)
		sz = Data->nMaxLength;
	return sz;
}

bool ConvertItemEx (
        CVTITEMFLAGS FromPlugin,
				FarDialogItem *Item,
				DialogItemEx *Data,
        unsigned Count
        )
{
	unsigned I;
	if(!Item || !Data)
		return false;

	switch(FromPlugin)
	{
		case CVTITEM_TOPLUGIN:
		case CVTITEM_TOPLUGINSHORT:
			for (I=0; I < Count; I++, ++Item, ++Data)
			{
				ConvertItemSmall(Item,Data);

				if(FromPlugin==CVTITEM_TOPLUGIN)
				{

					string str;
					size_t sz = ItemStringAndSize(Data,str);

					{
						wchar_t *p = (wchar_t*)xf_malloc((sz+1)*sizeof(wchar_t));
						Item->PtrData = p;
						if (!p) // TODO: may be needed message?
							return false;
						wmemcpy(p, (const wchar_t*)str, sz);
						p[sz] = L'\0';
					}
				}
			}
			break;
		case CVTITEM_FROMPLUGIN:
		case CVTITEM_FROMPLUGINSHORT:
			for (I=0; I < Count; I++, ++Item, ++Data)
			{
				Data->X1 = Item->X1;
				Data->Y1 = Item->Y1;
				Data->X2 = Item->X2;
				Data->Y2 = Item->Y2;
				Data->Focus = Item->Focus;

				Data->History = Item->Param.History;
				Data->Flags = Item->Flags;
				Data->DefaultButton = Item->DefaultButton;

				Data->Type = Item->Type;

				if(FromPlugin==CVTITEM_FROMPLUGIN)
				{
					Data->strData = Item->PtrData;
					Data->nMaxLength = Item->MaxLen;
					if (Data->nMaxLength > 0)
						Data->strData.SetLength(Data->nMaxLength);
				}

				Data->ListItems = Item->Param.ListItems;

				if(Data->X2 < Data->X1) Data->X2=Data->X1;
				if(Data->Y2 < Data->Y1) Data->Y2=Data->Y1;
				if((Data->Type == DI_COMBOBOX || Data->Type == DI_LISTBOX) && !IsPtr(Item->Param.ListItems))
					Data->ListItems=NULL;
			}
			break;
	}
	return true;
}

size_t ConvertItemEx2(FarDialogItem *Item,DialogItemEx *Data)
{
	size_t size=sizeof(*Item);
	string str;
	size_t sz = ItemStringAndSize(Data,str);
	size+=(sz+1)*sizeof(wchar_t);
	if(Item)
	{
		ConvertItemSmall(Item,Data);
		wchar_t* p=(wchar_t*)(Item+1);
		Item->PtrData = p;
		wmemcpy(p, (const wchar_t*)str, sz);
		p[sz] = L'\0';
		if(Data->Type==DI_LISTBOX || Data->Type==DI_COMBOBOX)
			Item->Param.ListPos=Data->ListPtr?Data->ListPtr->GetSelectPos():0;
	}
	return size;
}

void DataToItemEx(DialogDataEx *Data,DialogItemEx *Item,int Count)
{
  int I;

  if(!Item || !Data)
    return;

  for (I=0; I < Count; I++, ++Item, ++Data)
  {
  	Item->Clear();
		Item->ID=static_cast<WORD>(I);
    Item->Type=Data->Type;
    Item->X1=Data->X1;
    Item->Y1=Data->Y1;
    Item->X2=Data->X2;
    Item->Y2=Data->Y2;
    if(Item->X2 < Item->X1) Item->X2=Item->X1;
    if(Item->Y2 < Item->Y1) Item->Y2=Item->Y1;
    Item->Focus=Data->Focus;
    Item->History=Data->History;
    Item->Flags=Data->Flags;
    Item->DefaultButton=Data->DefaultButton;
    Item->SelStart=-1;


		if(!IsPtr(Data->Data))
        Item->strData = MSG((int)(DWORD_PTR)Data->Data);
    else
        Item->strData = Data->Data;
  }
}



Dialog::Dialog(DialogItemEx *SrcItem,    // Набор элементов диалога
               unsigned SrcItemCount,              // Количество элементов
               FARWINDOWPROC DlgProc,      // Диалоговая процедура
               LONG_PTR InitParam)             // Ассоцированные с диалогом данные
{
	bInitOK = false;

	Dialog::Item = (DialogItemEx**)xf_malloc (sizeof(DialogItemEx*)*SrcItemCount);

	for (unsigned i = 0; i < SrcItemCount; i++)
	{
		Dialog::Item[i] = new DialogItemEx;
		Dialog::Item[i]->Clear();
		DialogItemExToDialogItemEx (&SrcItem[i], Dialog::Item[i]);
	}

	Dialog::ItemCount = SrcItemCount;

	Dialog::pSaveItemEx = SrcItem;

	Init(DlgProc, InitParam);
}

Dialog::Dialog(FarDialogItem *SrcItem,    // Набор элементов диалога
               unsigned SrcItemCount,              // Количество элементов
               FARWINDOWPROC DlgProc,      // Диалоговая процедура
               LONG_PTR InitParam)             // Ассоцированные с диалогом данные
{
	bInitOK = false;

	Dialog::Item = (DialogItemEx**)xf_malloc (sizeof(DialogItemEx*)*SrcItemCount);

	for (unsigned i = 0; i < SrcItemCount; i++)
	{
		Dialog::Item[i] = new DialogItemEx;
		Dialog::Item[i]->Clear();
		//BUGBUG add error check
		ConvertItemEx(CVTITEM_FROMPLUGIN,&SrcItem[i],Dialog::Item[i],1);
	}

	Dialog::ItemCount = SrcItemCount;

	Dialog::pSaveItemEx = NULL;

	Init(DlgProc, InitParam);
}

void Dialog::Init(FARWINDOWPROC DlgProc,      // Диалоговая процедура
                  LONG_PTR InitParam)         // Ассоцированные с диалогом данные
{
	SetDynamicallyBorn(FALSE); // $OT: По умолчанию все диалоги создаются статически
	CanLoseFocus = FALSE;
	HelpTopic = NULL;
	//Номер плагина, вызвавшего диалог (-1 = Main)
	PluginNumber=-1;
	Dialog::DataDialog=InitParam;
	DialogMode.Set(DMODE_ISCANMOVE);
	SetDropDownOpened(FALSE);

	IsEnableRedraw=0;

	FocusPos=(unsigned)-1;
	PrevFocusPos=(unsigned)-1;

	if(!DlgProc) // функция должна быть всегда!!!
	{
		DlgProc=DefDlgProc;
		// знать диалог в старом стиле - учтем этот факт!
		DialogMode.Set(DMODE_OLDSTYLE);
	}
	Dialog::RealDlgProc=DlgProc;

	if (CtrlObject!=NULL)
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
	memset(&Id,0,sizeof(Id));

	bInitOK = true;
}

//////////////////////////////////////////////////////////////////////////
/* Public, Virtual:
   Деструктор класса Dialog
*/
Dialog::~Dialog()
{
	_tran(SysLog(L"[%p] Dialog::~Dialog()",this));

	GetDialogObjectsData();
	DeleteDialogObjects();

	if (CtrlObject!=NULL)
		CtrlObject->Macro.SetMode(PrevMacroMode);

	Hide();
	ScrBuf.Flush();

	if (HelpTopic)
		delete [] HelpTopic;

	for (unsigned i = 0; i < ItemCount; i++)
			delete Item[i];

	xf_free (Item);

	INPUT_RECORD rec;
	PeekInputRecord(&rec);
	delete OldTitle;

	_DIALOG(CleverSysLog CL(L"Destroy Dialog"));
}

void Dialog::CheckDialogCoord()
{
  CriticalSectionLock Lock(CS);

  if ( X1 >= 0 )
    X2 = X1+RealWidth-1;

  if ( Y1 >= 0 )
    Y2 = Y1+RealHeight-1;

  if(X2 > ScrX)
  {
    if(X1 != -1 && X2-X1+1 < ScrX) // если мы все же вмещаемся в консоль, то
    {                              // произведем обычный сдвиг диалога...
      int D=X2-ScrX;
      X1-=D;
      X2-=D;
    }
  }

  if (X1 < 0) // задано центрирование диалога по горизонтали?
  {             //   X2 при этом = ширине диалога.
    X1=(ScrX - X2 + 1)/2;

    if (X1 <= 0) // ширина диалога больше ширины экрана?
    {
      X1=0;
    }
    else
      X2+=X1-1;
  }

  if (Y1 < 0) // задано центрирование диалога по вертикали?
  {             //   Y2 при этом = высоте диалога.
    Y1=(ScrY-Y2+1)/2;

    if(!DialogMode.Check(DMODE_SMALLDIALOG)) //????
      if (Y1>5)
        Y1--;

    if ( Y1<0 )
    {
       Y1=0;
       Y2=ScrY+1;
    }
    else
      Y2+=Y1-1;
  }
}


void Dialog::InitDialog()
{
  CriticalSectionLock Lock(CS);

  if (!DialogMode.Check(DMODE_INITOBJECTS))      // самодостаточный вариант, когда
  {                      //  элементы инициализируются при первом вызове.
    CheckDialogCoord();

    unsigned InitFocus=InitDialogObjects();

    int Result=(int)DlgProc((HANDLE)this,DN_INITDIALOG,InitFocus,DataDialog);
    if(ExitCode == -1)
    {
      if(Result)
      {
        // еще разок, т.к. данные могли быть изменены
        InitFocus=InitDialogObjects(); // InitFocus=????
      }
      SetFarTitle(GetDialogTitle());
    }
    // все объекты проинициализированы!
    DialogMode.Set(DMODE_INITOBJECTS);

		DialogInfo di={0};
		if(DlgProc(reinterpret_cast<HANDLE>(this),DN_GETDIALOGINFO,0,reinterpret_cast<LONG_PTR>(&di)))
		{
			if(static_cast<size_t>(di.StructSize)>=offsetof(DialogInfo,Id)+sizeof(di.Id))
			{
				Id=di.Id;
				IdExist=true;
			}
		}

    DlgProc((HANDLE)this,DN_GOTFOCUS,InitFocus,0);
  }


  CheckDialogCoord();
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

  if(!DialogMode.Check(DMODE_INITOBJECTS))
    return;

	if(!Locked() && DialogMode.Check(DMODE_RESIZED))
	{
		PreRedrawItem preRedrawItem=PreRedraw.Peek();
		if(preRedrawItem.PreRedrawFunc)
			preRedrawItem.PreRedrawFunc();
	}

  DialogMode.Clear(DMODE_RESIZED);

   if ( Locked() )
     return;

   DialogMode.Set(DMODE_SHOW);
   ScreenObject::Show();
}

//  Цель перехвата данной функции - управление видимостью...
void Dialog::Hide()
{
  CriticalSectionLock Lock(CS);
  _tran(SysLog(L"[%p] Dialog::Hide()",this));
  if(!DialogMode.Check(DMODE_INITOBJECTS))
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

  if(DialogMode.Check(DMODE_SHOW))
  {
    ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
    ShowDialog();          // "нарисуем" диалог.
  }
}

// пересчитать координаты для элементов с DIF_CENTERGROUP
void Dialog::ProcessCenterGroup()
{
  CriticalSectionLock Lock(CS);

  int Length,StartX;
  int Type;
	DialogItemEx *CurItem, *JCurItem;
  DWORD ItemFlags;

  for (unsigned I=0; I < ItemCount; I++)
  {
    CurItem = Item[I];
    Type=CurItem->Type;
    ItemFlags=CurItem->Flags;

    // Последовательно объявленные элементы с флагом DIF_CENTERGROUP
    // и одинаковой вертикальной позицией будут отцентрированы в диалоге.
    // Их координаты X не важны. Удобно использовать для центрирования
    // групп кнопок.
    if ((ItemFlags & DIF_CENTERGROUP) &&
        (I==0 ||
          (I > 0 &&
            ((Item[I-1]->Flags & DIF_CENTERGROUP)==0 ||
             Item[I-1]->Y1!=CurItem->Y1)
          )
        )
       )
    {
      unsigned J;
      Length=0;

      for (J=I;
           J < ItemCount && (Item[J]->Flags & DIF_CENTERGROUP) && Item[J]->Y1==CurItem->Y1;
           J++)
      {
        JCurItem = Item[J];
        Length+=LenStrItem(J);

//        if (JCurItem->Type==DI_BUTTON && *JCurItem->Data!=' ')
//          Length+=2;
        if ( !JCurItem->strData.IsEmpty() && (JCurItem->strData.At(0) != L' ')) //BBUG
          switch(JCurItem->Type)
          {
            case DI_BUTTON:
              Length+=2;
              break;
            case DI_CHECKBOX:
            case DI_RADIOBUTTON:
              Length+=5;
              break;
          }
      }

//      if (Type==DI_BUTTON && *CurItem->Data!=' ')
//        Length-=2;
      if ( !CurItem->strData.IsEmpty() && (CurItem->strData.At(0) != L' ') )
        switch(Type)
        {
          case DI_BUTTON:
            Length-=2;
            break;
          case DI_CHECKBOX:
          case DI_RADIOBUTTON:
//            Length-=5;
            break;
        } //Бля, це ж ботва какая-то

      StartX=(X2-X1+1-Length)/2;

      if (StartX<0)
        StartX=0;

      for (J=I;
           J < ItemCount && (Item[J]->Flags & DIF_CENTERGROUP) && Item[J]->Y1==CurItem->Y1;
           J++)
      {
        JCurItem = Item[J];
        JCurItem->X1=StartX;
        StartX+=LenStrItem(J);

//        if (JCurItem->Type==DI_BUTTON && *JCurItem->Data!=' ')
//          StartX+=2;
        if ( !JCurItem->strData.IsEmpty() && (JCurItem->strData.At(0) !=L' ') )
          switch(JCurItem->Type)
          {
            case DI_BUTTON:
              StartX+=2;
              break;
            case DI_CHECKBOX:
            case DI_RADIOBUTTON:
              StartX+=5;
              break;
          }

        if (StartX == JCurItem->X1)
          JCurItem->X2=StartX;
        else
          JCurItem->X2=StartX-1;
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
  int Type;
	DialogItemEx *CurItem;
  unsigned InitItemCount;
  DWORD ItemFlags;

  _DIALOG(CleverSysLog CL(L"Init Dialog"));

  if(ID+1 > ItemCount)
    return (unsigned)-1;

  if(ID == (unsigned)-1) // инициализируем все?
  {
    ID=0;
    InitItemCount=ItemCount;
  }
  else
  {
    InitItemCount=ID+1;
  }

  //   если FocusPos в пределах и элемент задисаблен, то ищем сначала.
  if(FocusPos!=(unsigned)-1 && FocusPos < ItemCount &&
     (Item[FocusPos]->Flags&(DIF_DISABLE|DIF_NOFOCUS|DIF_HIDDEN)))
    FocusPos = (unsigned)-1; // будем искать сначала!

  // предварительный цикл по поводу кнопок
  for(I=ID; I < InitItemCount; I++)
  {
    CurItem = Item[I];
    ItemFlags=CurItem->Flags;
    Type=CurItem->Type;

    // для кнопок не имеющи стиля "Показывает заголовок кнопки без скобок"
    //  добавим энти самые скобки
    if (Type==DI_BUTTON &&
        (ItemFlags & DIF_NOBRACKETS)==0 &&
        (!CurItem->strData.IsEmpty () && (CurItem->strData.At(0) != L'[')) )
      CurItem->strData.Format (L"[ %s ]", (const wchar_t*)CurItem->strData);

     // предварительный поик фокуса
     if(FocusPos == (unsigned)-1 &&
        CanGetFocus(Type) &&
        CurItem->Focus &&
        !(ItemFlags&(DIF_DISABLE|DIF_NOFOCUS|DIF_HIDDEN)))
       FocusPos=I; // запомним первый фокусный элемент
     CurItem->Focus=0; // сбросим для всех, чтобы не оказалось,
                       //   что фокусов - как у дурочка фантиков

     // сбросим флаг DIF_CENTERGROUP для редакторов
     switch(Type)
     {
       case DI_BUTTON:
       case DI_CHECKBOX:
       case DI_RADIOBUTTON:
       case DI_TEXT:
       case DI_VTEXT:  // ????
         break;
       default:
         if(ItemFlags&DIF_CENTERGROUP)
           CurItem->Flags&=~DIF_CENTERGROUP;
     }
  }

  // Опять про фокус ввода - теперь, если "чудо" забыло выставить
  // хотя бы один, то ставим на первый подходящий
  if(FocusPos == (unsigned)-1)
  {
    for (I=0; I < ItemCount; I++) // по всем!!!!
    {
        CurItem = Item[I];
      if(CanGetFocus(CurItem->Type) &&
         !(CurItem->Flags&(DIF_DISABLE|DIF_NOFOCUS|DIF_HIDDEN)))
      {
        FocusPos=I;
        break;
      }
    }
  }
  if(FocusPos == (unsigned)-1) // ну ни хрена себе - нет ни одного
  {                  //   элемента с возможностью фокуса
     FocusPos=0;     // убится, блин
  }

  // ну вот и добрались до!
  Item[FocusPos]->Focus=1;

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
        CurItem->ListPtr=new VMenu(NULL,NULL,0,CurItem->Y2-CurItem->Y1+1,
                        VMENU_ALWAYSSCROLLBAR|VMENU_LISTBOX,NULL,this);
      }

      if(CurItem->ListPtr)
      {
        VMenu *ListPtr=CurItem->ListPtr;
        ListPtr->SetVDialogItemID(I);
        /* $ 13.09.2000 SVS
           + Флаг DIF_LISTNOAMPERSAND. По умолчанию для DI_LISTBOX &
             DI_COMBOBOX выставляется флаг MENU_SHOWAMPERSAND. Этот флаг
             подавляет такое поведение
        */
        CurItem->IFlags.Set(DLGIIF_LISTREACTIONFOCUS|DLGIIF_LISTREACTIONNOFOCUS); // всегда!

        ListPtr->ChangeFlags(VMENU_DISABLED, ItemFlags&DIF_DISABLE);
        ListPtr->ChangeFlags(VMENU_SHOWAMPERSAND, !(ItemFlags&DIF_LISTNOAMPERSAND));
        ListPtr->ChangeFlags(VMENU_SHOWNOBOX, ItemFlags&DIF_LISTNOBOX);
        ListPtr->ChangeFlags(VMENU_WRAPMODE, ItemFlags&DIF_LISTWRAPMODE);
        ListPtr->ChangeFlags(VMENU_AUTOHIGHLIGHT, ItemFlags&DIF_LISTAUTOHIGHLIGHT);

        if(ItemFlags&DIF_LISTAUTOHIGHLIGHT)
          ListPtr->AssignHighlights(FALSE);

        ListPtr->SetDialogStyle(DialogMode.Check(DMODE_WARNINGSTYLE));
        ListPtr->SetPosition(X1+CurItem->X1,Y1+CurItem->Y1,
                             X1+CurItem->X2,Y1+CurItem->Y2);
        ListPtr->SetBoxType(SHORT_SINGLE_BOX);
        // поле FarDialogItem.Data для DI_LISTBOX используется как верхний заголовок листа
        if(!(ItemFlags&DIF_LISTNOBOX) && !DialogMode.Check(DMODE_CREATEOBJECTS))
        {
            ListPtr->SetTitle(CurItem->strData);
        }
        // удалим все итемы
        //ListBox->DeleteItems(); //???? А НАДО ЛИ ????
        if(CurItem->ListItems && !DialogMode.Check(DMODE_CREATEOBJECTS))
        {
          ListPtr->AddItem(CurItem->ListItems);
        }

        ListPtr->ChangeFlags (VMENU_LISTHASFOCUS, CurItem->Focus);
      }
    }

    // "редакторы" - разговор особый...
    else if (IsEdit(Type))
    {
      // сбросим флаг DIF_EDITOR для строки ввода, отличной от DI_EDIT,
      // DI_FIXEDIT и DI_PSWEDIT
      if(Type != DI_COMBOBOX)
        if((ItemFlags&DIF_EDITOR) && Type != DI_EDIT && Type != DI_FIXEDIT && Type != DI_PSWEDIT)
          ItemFlags&=~DIF_EDITOR;

      if (!DialogMode.Check(DMODE_CREATEOBJECTS))
      {
        CurItem->ObjPtr=new DlgEdit(this,I,Type == DI_MEMOEDIT?DLGEDIT_MULTILINE:DLGEDIT_SINGLELINE);
        if(Type == DI_COMBOBOX)
        {
          CurItem->ListPtr=new VMenu(L"",NULL,0,Opt.Dialogs.CBoxMaxHeight,VMENU_ALWAYSSCROLLBAR|VMENU_NOTCHANGE,NULL/*,Parent*/);
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
        if(CurItem->ListPtr)
        {
          VMenu *ListPtr=CurItem->ListPtr;

          ListPtr->SetBoxType(SHORT_SINGLE_BOX);
          DialogEdit->SetDropDownBox(ItemFlags & DIF_DROPDOWNLIST);
          ListPtr->ChangeFlags(VMENU_WRAPMODE, ItemFlags&DIF_LISTWRAPMODE);
          ListPtr->ChangeFlags(VMENU_DISABLED, ItemFlags&DIF_DISABLE);
          ListPtr->ChangeFlags(VMENU_SHOWAMPERSAND, !(ItemFlags&DIF_LISTNOAMPERSAND));
          ListPtr->ChangeFlags(VMENU_AUTOHIGHLIGHT, ItemFlags&DIF_LISTAUTOHIGHLIGHT);

          if(ItemFlags&DIF_LISTAUTOHIGHLIGHT)
            ListPtr->AssignHighlights(FALSE);

          if(CurItem->ListItems && !DialogMode.Check(DMODE_CREATEOBJECTS))
            ListPtr->AddItem(CurItem->ListItems);

          ListPtr->SetFlags(VMENU_COMBOBOX);
          ListPtr->SetDialogStyle(DialogMode.Check(DMODE_WARNINGSTYLE));
        }
      }

      /* $ 15.10.2000 tran
        строка редакторирование должна иметь максимум в 511 символов */
      // выставляем максимальный размер в том случае, если он еще не выставлен

      //BUGBUG
      if(DialogEdit->GetMaxLength() == -1)
        DialogEdit->SetMaxLength(CurItem->nMaxLength?(int)CurItem->nMaxLength:-1);

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
        if(ItemFlags&DIF_HISTORY)
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
        if ((ItemFlags & DIF_MASKEDIT) && CurItem->Mask)
        {
          const wchar_t*Ptr=CurItem->Mask;
          while(*Ptr && *Ptr == L' ') ++Ptr;
          if(*Ptr)
            DialogEdit->SetInputMask(CurItem->Mask);
          else
          {
            CurItem->Mask=NULL;
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

        if(CurItem->Type == DI_COMBOBOX)
          DialogEdit->SetClearFlag(1);

      /* $ 01.08.2000 SVS
         Еже ли стоит флаг DIF_USELASTHISTORY и непустая строка ввода,
         то подстанавливаем первое значение из History
      */
      if(CurItem->Type==DI_EDIT &&
        (ItemFlags&(DIF_HISTORY|DIF_USELASTHISTORY)) == (DIF_HISTORY|DIF_USELASTHISTORY))
      {
        ProcessLastHistory (CurItem, -1);
      }

      if((ItemFlags&DIF_MANUALADDHISTORY) && !(ItemFlags&DIF_HISTORY))
        ItemFlags&=~DIF_MANUALADDHISTORY; // сбросим нафиг.

      /* $ 18.03.2000 SVS
         Если это ComBoBox и данные не установлены, то берем из списка
         при условии, что хоть один из пунктов имеет Selected != 0
      */

      if (Type==DI_COMBOBOX && CurItem->strData.IsEmpty () && CurItem->ListItems)
      {
				FarListItem *ListItems=CurItem->ListItems->Items;
        unsigned Length=CurItem->ListItems->ItemsNumber;

        //CurItem->ListPtr->AddItem(CurItem->ListItems);

        for (J=0; J < Length; J++)
        {
          if(ListItems[J].Flags & LIF_SELECTED)
          {
            if(ItemFlags & (DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND))
              HiText2Str(CurItem->strData, ListItems[J].Text);
            else
              CurItem->strData = ListItems[J].Text;
            break;
          }
        }
      }

      DialogEdit->SetString(CurItem->strData);

      if (Type==DI_FIXEDIT)
        DialogEdit->SetCurPos(0);

      // Для обычных строк отрубим постоянные блоки
      if (!(ItemFlags&DIF_EDITOR))
        DialogEdit->SetPersistentBlocks(Opt.Dialogs.EditBlock);

      DialogEdit->SetDelRemovesBlocks(Opt.Dialogs.DelRemovesBlocks);
      if(ItemFlags&DIF_READONLY)
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

	DialogItemEx *CurItem, *CurItemList=NULL;

  for(unsigned I=0; I < ItemCount; I++)
  {
      CurItem = Item[I];
    // по первому попавшемуся "тексту" установим заголовок консоли!
    if ((CurItem->Type==DI_TEXT ||
          CurItem->Type==DI_DOUBLEBOX ||
          CurItem->Type==DI_SINGLEBOX))
    {
      const wchar_t *Ptr = CurItem->strData;
      for (; *Ptr; Ptr++)
        if ( IsAlpha (*Ptr) || iswdigit(*Ptr) )
          return(Ptr);
    }
    else if(CurItem->Type==DI_LISTBOX && !I)
      CurItemList=CurItem;
  }

  if(CurItemList)
  {
      return CurItemList->ListPtr->GetPtrTitle();
  }

  return NULL; //""
}

void Dialog::ProcessLastHistory(DialogItemEx *CurItem, int MsgIndex)
{
	CriticalSectionLock Lock(CS);

	string &strData = CurItem->strData;

	if (strData.IsEmpty())
	{
		string strRegKey=fmtSavedDialogHistory;
		strRegKey+=CurItem->History;
		History::ReadLastItem(strRegKey, strData);
		if (MsgIndex != -1)
		{
			// обработка DM_SETHISTORY => надо пропустить изменение текста через
			// диалоговую функцию
			FarDialogItemData IData;
			IData.PtrData=(wchar_t *)(const wchar_t *)strData;
			IData.PtrLength=(int)strData.GetLength();
			SendDlgMessage(this,DM_SETTEXT,MsgIndex,(LONG_PTR)&IData);
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
  int Type=CurItem->Type;

  CurItem->X1=(short)Rect->Left;
  CurItem->Y1=(Rect->Top<0)?0:Rect->Top;

  if (IsEdit(Type))
  {
      DlgEdit *DialogEdit=(DlgEdit *)CurItem->ObjPtr;
      CurItem->X2=(short)Rect->Right;
      CurItem->Y2=(short)(Type == DI_MEMOEDIT?Rect->Bottom:0);
      DialogEdit->SetPosition(X1+Rect->Left, Y1+Rect->Top,
                                   X1+Rect->Right,Y1+Rect->Top);
  }
  else if (Type==DI_LISTBOX)
  {
      CurItem->X2=(short)Rect->Right;
      CurItem->Y2=(short)Rect->Bottom;
      CurItem->ListPtr->SetPosition(X1+Rect->Left, Y1+Rect->Top,
                                    X1+Rect->Right,Y1+Rect->Bottom);
      CurItem->ListPtr->SetMaxHeight(CurItem->Y2-CurItem->Y1+1);
  }
  switch(Type)
  {
    case DI_TEXT:
      CurItem->X2=(short)Rect->Right;
      CurItem->Y2=0;                    // ???
      break;

    case DI_VTEXT:
      CurItem->X2=0;                    // ???
      CurItem->Y2=(short)Rect->Bottom;

    case DI_DOUBLEBOX:
    case DI_SINGLEBOX:
    case DI_USERCONTROL:
      CurItem->X2=(short)Rect->Right;
      CurItem->Y2=(short)Rect->Bottom;
      break;
  }

  if(DialogMode.Check(DMODE_SHOW))
  {
    ShowDialog((unsigned)-1);
    ScrBuf.Flush();
  }
  return TRUE;
}

BOOL Dialog::GetItemRect(unsigned I,RECT& Rect)
{
  CriticalSectionLock Lock(CS);

  if(I >= ItemCount)
    return FALSE;

	DialogItemEx *CurItem=Item[I];
  DWORD ItemFlags=CurItem->Flags;
  int Type=CurItem->Type;
  int Len=0;

  Rect.left=(int)CurItem->X1;
  Rect.top=(int)CurItem->Y1;
  Rect.right=(int)CurItem->X2;
  Rect.bottom=(int)CurItem->Y2;

  switch(Type)
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

  switch(Type)
  {
    case DI_TEXT:
      if (CurItem->X1==(short)-1)
        Rect.left=(X2-X1+1-Len)/2;
      if(Rect.left < 0)
        Rect.left=0;

      if (CurItem->Y1==(short)-1)
        Rect.top=(Y2-Y1+1)/2;
      if(Rect.top < 0)
        Rect.top=0;

      Rect.bottom=Rect.top;

      if ( Rect.right == 0 || Rect.right == Rect.left)
        Rect.right=Rect.left+Len-(Len==0?0:1);

      if (ItemFlags & (DIF_SEPARATOR|DIF_SEPARATOR2))
      {
        Rect.bottom=Rect.top;
        Rect.left=(!DialogMode.Check(DMODE_SMALLDIALOG)?3:0); //???
        Rect.right=X2-X1-(!DialogMode.Check(DMODE_SMALLDIALOG)?5:0); //???
      }
      break;

    case DI_VTEXT:
      if (CurItem->X1==(short)-1)
        Rect.left=(X2-X1+1)/2;
      if(Rect.left < 0)
        Rect.left=0;

      if (CurItem->Y1==(short)-1)
        Rect.top=(Y2-Y1+1-Len)/2;
      if(Rect.top < 0)
        Rect.top=0;

      Rect.right=Rect.left;
      //Rect.bottom=Rect.top+Len;
      if ( Rect.bottom == 0 || Rect.bottom == Rect.top)
        Rect.bottom=Rect.top+Len-(Len==0?0:1);

#if defined(VTEXT_ADN_SEPARATORS)
      if (ItemFlags & (DIF_SEPARATOR|DIF_SEPARATOR2))
      {
        Rect.right=Rect.left;
        Rect.top=(!DialogMode.Check(DMODE_SMALLDIALOG)?1:0); //???
        Rect.bottom=Y2-Y1-(!DialogMode.Check(DMODE_SMALLDIALOG)?3:0); //???
        break;
      }
#endif
      break;

    case DI_BUTTON:
      Rect.bottom=Rect.top;
      Rect.right=Rect.left+Len;
      break;

    case DI_CHECKBOX:
    case DI_RADIOBUTTON:
      Rect.bottom=Rect.top;
      Rect.right=Rect.left+Len+((Type == DI_CHECKBOX)?4:
                                 (ItemFlags & DIF_MOVESELECT?3:4)
                               );
      break;

    case DI_COMBOBOX:
    case DI_EDIT:
    case DI_FIXEDIT:
    case DI_PSWEDIT:
      Rect.bottom=Rect.top;
      break;
  }
  return TRUE;
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
    switch(CurItem->Type)
    {
      case DI_EDIT:
      case DI_FIXEDIT:
      case DI_PSWEDIT:
      case DI_COMBOBOX:
      case DI_MEMOEDIT:
        if(CurItem->ObjPtr)
          delete (DlgEdit *)(CurItem->ObjPtr);
      case DI_LISTBOX:
        if((CurItem->Type == DI_COMBOBOX || CurItem->Type == DI_LISTBOX) &&
            CurItem->ListPtr)
           delete CurItem->ListPtr;
        break;
      case DI_USERCONTROL:
        if(CurItem->UCData)
          delete CurItem->UCData;
        break;
    }
    if(CurItem->Flags&DIF_AUTOMATION)
      if(CurItem->AutoPtr)
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
    DWORD IFlags=CurItem->Flags;
    switch(Type=CurItem->Type)
    {
      case DI_MEMOEDIT:
        break; //????
      case DI_EDIT:
      case DI_FIXEDIT:
      case DI_PSWEDIT:
      case DI_COMBOBOX:
      {
        if(CurItem->ObjPtr)
        {
          string strData;

          DlgEdit *EditPtr=(DlgEdit *)(CurItem->ObjPtr);
          // подготовим данные

          // получим данные
          EditPtr->GetString(strData);

          if (ExitCode >=0 &&
              (IFlags & DIF_HISTORY) &&
              !(IFlags & DIF_MANUALADDHISTORY) && // при мануале не добавляем
              CurItem->History &&
              Opt.Dialogs.EditHistory)
          {
            AddToEditHistory(strData,CurItem->History);
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

          if((IFlags&DIF_EDITEXPAND) && Type != DI_PSWEDIT && Type != DI_FIXEDIT)
             apiExpandEnvironmentStrings(strData, strData);

          CurItem->strData = strData;

          EditPtr->SetString(strData);
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
		if((Type == DI_COMBOBOX || Type == DI_LISTBOX) && CurItem->ListPtr && CurItem->ListItems && DlgProc == DefDlgProc)
    {
      int ListPos=CurItem->ListPtr->GetSelectPos();
      if(ListPos < CurItem->ListItems->ItemsNumber)
      {
        for(int J=0; J < CurItem->ListItems->ItemsNumber; ++J)
          CurItem->ListItems->Items[J].Flags&=~LIF_SELECTED;
        CurItem->ListItems->Items[ListPos].Flags|=LIF_SELECTED;
      }
    }
#else
    if((Type == DI_COMBOBOX || Type == DI_LISTBOX))
    {
      CurItem->ListPos=CurItem->ListPtr?CurItem->ListPtr->GetSelectPos():0;
    }
#endif
  }
}


// Функция формирования и запроса цветов.
LONG_PTR Dialog::CtlColorDlgItem(int ItemPos,int Type,int Focus,DWORD Flags)
{
  CriticalSectionLock Lock(CS);

  BOOL DisabledItem=Flags&DIF_DISABLE?TRUE:FALSE;
  DWORD Attr=0;

  switch(Type)
  {
    case DI_SINGLEBOX:
    case DI_DOUBLEBOX:
    {
      if (Flags & DIF_SETCOLOR)
        Attr=Flags & DIF_COLORMASK;
      else
      {
        Attr=DialogMode.Check(DMODE_WARNINGSTYLE) ?
                    (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBOX):
                    (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGBOX);
      }

      Attr=MAKELONG(
          MAKEWORD(FarColorToReal(DialogMode.Check(DMODE_WARNINGSTYLE) ?
                      (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBOXTITLE):
                      (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGBOXTITLE)
                   ), // Title LOBYTE
                   FarColorToReal(DialogMode.Check(DMODE_WARNINGSTYLE) ?
                      (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGHIGHLIGHTBOXTITLE):
                      (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGHIGHLIGHTBOXTITLE)
                   )
          ),// HiText HIBYTE
          MAKEWORD(FarColorToReal(Attr), // Box LOBYTE
                   0)                     // HIBYTE
      );
      break;
    }

#if defined(VTEXT_ADN_SEPARATORS)
    case DI_VTEXT:
#endif
    case DI_TEXT:
    {
      if (Flags & DIF_SETCOLOR)
        Attr=Flags & DIF_COLORMASK;
      else
      {
        if (Flags & DIF_BOXCOLOR)
          Attr=DialogMode.Check(DMODE_WARNINGSTYLE) ?
                (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBOX):
                (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGBOX);
        else
          Attr=DialogMode.Check(DMODE_WARNINGSTYLE) ?
                (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGTEXT):
                (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGTEXT);
      }

      Attr=MAKELONG(
         MAKEWORD(FarColorToReal(Attr),
                 FarColorToReal(DialogMode.Check(DMODE_WARNINGSTYLE) ?
                    (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGHIGHLIGHTTEXT):
                    (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGHIGHLIGHTTEXT))), // HIBYTE HiText
           ((Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2))?
             (MAKEWORD(FarColorToReal(DialogMode.Check(DMODE_WARNINGSTYLE) ?
                (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBOX):
                (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGBOX)), // Box LOBYTE
               0))
             :
             0));
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
      Attr=MAKEWORD(MAKEWORD(FarColorToReal(Attr),0),MAKEWORD(0,0));
      break;
    }
#endif

    case DI_CHECKBOX:
    case DI_RADIOBUTTON:
    {
      if (Flags & DIF_SETCOLOR)
        Attr=(Flags & DIF_COLORMASK);
      else
        Attr=(DialogMode.Check(DMODE_WARNINGSTYLE) ?
                (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGTEXT):
                (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGTEXT));

      Attr=MAKEWORD(FarColorToReal(Attr),
           FarColorToReal(DialogMode.Check(DMODE_WARNINGSTYLE) ?
                 (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGHIGHLIGHTTEXT):
                 (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGHIGHLIGHTTEXT))); // HiText
      break;
    }

    case DI_BUTTON:
    {
      if (Focus)
      {
        SetCursorType(0,10);
        Attr=MAKEWORD(
           (Flags & DIF_SETCOLOR)?(Flags & DIF_COLORMASK):
             FarColorToReal(DialogMode.Check(DMODE_WARNINGSTYLE) ?
                 (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGSELECTEDBUTTON):
                 (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGSELECTEDBUTTON)), // TEXT
           FarColorToReal(DialogMode.Check(DMODE_WARNINGSTYLE) ?
                 (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON):
                 (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGHIGHLIGHTSELECTEDBUTTON))); // HiText
      }
      else
      {
        Attr=MAKEWORD(
           (Flags & DIF_SETCOLOR)?(Flags & DIF_COLORMASK):
             FarColorToReal(DialogMode.Check(DMODE_WARNINGSTYLE) ?
                    (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBUTTON):
                    (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGBUTTON)), // TEXT
           FarColorToReal(DialogMode.Check(DMODE_WARNINGSTYLE) ?
                    (DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGHIGHLIGHTBUTTON):
                    (DisabledItem?COL_DIALOGDISABLED:COL_DIALOGHIGHLIGHTBUTTON))); // HiText
      }
      break;
    }

    case DI_EDIT:
    case DI_FIXEDIT:
    case DI_PSWEDIT:
    case DI_COMBOBOX:
    case DI_MEMOEDIT:
    {
      if(Type == DI_COMBOBOX && (Flags & DIF_DROPDOWNLIST))
      {
        if(DialogMode.Check(DMODE_WARNINGSTYLE))
          Attr=MAKELONG(
            MAKEWORD( //LOWORD
              // LOLO (Text)
              FarColorToReal(DisabledItem?COL_WARNDIALOGEDITDISABLED:COL_WARNDIALOGEDIT),
              // LOHI (Select)
              FarColorToReal(DisabledItem?COL_DIALOGEDITDISABLED:COL_DIALOGEDITSELECTED)
            ),
            MAKEWORD( //HIWORD
              // HILO (Unchanged)
              FarColorToReal(DisabledItem?COL_WARNDIALOGEDITDISABLED:COL_DIALOGEDITUNCHANGED), //???
              // HIHI (History)
              FarColorToReal(DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGTEXT)
            )
          );
        else
          Attr=MAKELONG(
            MAKEWORD( //LOWORD
              // LOLO (Text)
              FarColorToReal(DisabledItem?COL_DIALOGEDITDISABLED:(!Focus?COL_DIALOGEDIT:COL_DIALOGEDITSELECTED)),
              // LOHI (Select)
              FarColorToReal(DisabledItem?COL_DIALOGEDITDISABLED:(!Focus?COL_DIALOGEDIT:COL_DIALOGEDITSELECTED))
            ),
            MAKEWORD( //HIWORD
              // HILO (Unchanged)
              FarColorToReal(DisabledItem?COL_DIALOGEDITDISABLED:COL_DIALOGEDITUNCHANGED), //???
              // HIHI (History)
              FarColorToReal(DisabledItem?COL_DIALOGDISABLED:COL_DIALOGTEXT)
            )
          );
      }
      else
      {
        if(DialogMode.Check(DMODE_WARNINGSTYLE))
          Attr=MAKELONG(
            MAKEWORD( //LOWORD
              // LOLO (Text)
              FarColorToReal(DisabledItem?COL_WARNDIALOGEDITDISABLED:
                (Flags&DIF_NOFOCUS?COL_DIALOGEDITUNCHANGED:COL_WARNDIALOGEDIT)),
              // LOHI (Select)
              FarColorToReal(DisabledItem?COL_DIALOGEDITDISABLED:COL_DIALOGEDITSELECTED)
            ),
            MAKEWORD( //HIWORD
              // HILO (Unchanged)
              FarColorToReal(DisabledItem?COL_WARNDIALOGEDITDISABLED:COL_DIALOGEDITUNCHANGED), //???
              // HIHI (History)
              FarColorToReal(DisabledItem?COL_WARNDIALOGDISABLED:COL_WARNDIALOGTEXT)
            )
          );
        else
          Attr=MAKELONG(
            MAKEWORD( //LOWORD
              // LOLO (Text)
              FarColorToReal(DisabledItem?COL_DIALOGEDITDISABLED:
                (Flags&DIF_NOFOCUS?COL_DIALOGEDITUNCHANGED:COL_DIALOGEDIT)),
              // LOHI (Select)
              FarColorToReal(DisabledItem?COL_DIALOGEDITDISABLED:COL_DIALOGEDITSELECTED)
            ),
            MAKEWORD( //HIWORD
              // HILO (Unchanged)
              FarColorToReal(DisabledItem?COL_DIALOGEDITDISABLED:COL_DIALOGEDITUNCHANGED), //???
              // HIHI (History)
              FarColorToReal(DisabledItem?COL_DIALOGDISABLED:COL_DIALOGTEXT)
            )
          );
      }
      break;
    }

    case DI_LISTBOX:
    {
      Item[ItemPos]->ListPtr->SetColors(NULL);
      return 0;
    }

    default:
    {
      return 0;
    }
  }
  return DlgProc((HANDLE)this,DN_CTLCOLORDLGITEM,ItemPos,Attr);
}


//////////////////////////////////////////////////////////////////////////
/* Private:
   Отрисовка элементов диалога на экране.
*/
void Dialog::ShowDialog(unsigned ID)
{
  CriticalSectionLock Lock(CS);

  if ( Locked () )
    return;

  string strStr;
  wchar_t *lpwszStr;

	DialogItemEx *CurItem;
  int X,Y;
  unsigned I,DrawItemCount;
  DWORD Attr;

  //   Если не разрешена отрисовка, то вываливаем.
  if(IsEnableRedraw ||                 // разрешена прорисовка ?
     (ID+1 > ItemCount) ||             // а номер в рамках дозволенного?
     DialogMode.Check(DMODE_DRAWING) || // диалог рисуется?
     !DialogMode.Check(DMODE_SHOW) ||   // если не видим, то и не отрисовываем.
     !DialogMode.Check(DMODE_INITOBJECTS))
    return;

  DialogMode.Set(DMODE_DRAWING);  // диалог рисуется!!!

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  if(ID == (unsigned)-1) // рисуем все?
  {
    //   Перед прорисовкой диалога посылаем сообщение в обработчик
    if(!DlgProc((HANDLE)this,DN_DRAWDIALOG,0,0))
    {
      DialogMode.Clear(DMODE_DRAWING);  // конец отрисовки диалога!!!
      return;
    }


    //   перед прорисовкой подложки окна диалога...
    if(!DialogMode.Check(DMODE_NODRAWSHADOW))
      Shadow();              // "наводим" тень

    if(!DialogMode.Check(DMODE_NODRAWPANEL))
    {
      Attr=(DWORD)DlgProc((HANDLE)this,DN_CTLCOLORDIALOG,0,
          DialogMode.Check(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGTEXT:COL_DIALOGTEXT);
      SetScreen(X1,Y1,X2,Y2,L' ',Attr);
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
    int CursorVisible=0,CursorSize=0;
    if(ID != (unsigned)-1 && FocusPos != ID)
    {
      if(Item[FocusPos]->Type == DI_USERCONTROL && Item[FocusPos]->UCData->CursorPos.X != -1 && Item[FocusPos]->UCData->CursorPos.Y != -1)
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
    if(CurItem->Flags&DIF_HIDDEN)
      continue;


    /* $ 28.07.2000 SVS
       Перед прорисовкой каждого элемента посылаем сообщение
       посредством функции SendDlgMessage - в ней делается все!
    */
		if(!SendDlgMessage((HANDLE)this,DN_DRAWDLGITEM,I,0))
       continue;

    int LenText;
    short CX1=CurItem->X1;
    short CY1=CurItem->Y1;
    short CX2=CurItem->X2;
    short CY2=CurItem->Y2;

    if ( CX2 > X2-X1 )
      CX2 = X2-X1;

    if ( CY2 > Y2-Y1 )
      CY2 = Y2-Y1;

    short CW=CX2-CX1+1;
    short CH=CY2-CY1+1;

    Attr=(DWORD)CtlColorDlgItem(I,CurItem->Type,CurItem->Focus,CurItem->Flags);

#if 0
    // TODO: прежде чем эту строку применять... нужно проверить _ВСЕ_ диалоги на предмет X2, Y2. !!!
    if ( ((CX1 > -1) && (CX2 > 0) && (CX2 > CX1)) &&
       ((CY1 > -1) && (CY2 > 0) && (CY2 > CY1)) )
      SetScreen(X1+CX1,Y1+CY1,X1+CX2,Y1+CY2,' ',Attr&0xFF);
#endif

    switch(CurItem->Type)
    {
      /* ***************************************************************** */
      case DI_SINGLEBOX:
      case DI_DOUBLEBOX:
      {
        BOOL IsDrawTitle=TRUE;
        GotoXY(X1+CX1,Y1+CY1);
        SetColor(LOBYTE(HIWORD(Attr)));
        if(CY1 == CY2)
        {
          DrawLine(CX2-CX1+1,CurItem->Type==DI_SINGLEBOX?8:9); //???
        }
        else if(CX1 == CX2)
        {
          DrawLine(CY2-CY1+1,CurItem->Type==DI_SINGLEBOX?10:11);
          IsDrawTitle=FALSE;
        }
        else
        {
          Box(X1+CX1,Y1+CY1,X1+CX2,Y1+CY2,
             LOBYTE(HIWORD(Attr)),
             (CurItem->Type==DI_SINGLEBOX) ? SINGLE_BOX:DOUBLE_BOX);
        }

        if ( !CurItem->strData.IsEmpty() && IsDrawTitle)
        {
          //  ! Пусть диалог сам заботится о ширине собственного заголовка.
          strStr = CurItem->strData;
          TruncStrFromEnd(strStr,CW-2); // 5 ???
          LenText=LenStrItem(I,strStr);
					if(LenText < CW-2)
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

          SetColor(Attr&0xFF);
          GotoXY(X,Y1+CY1);
          if (CurItem->Flags & DIF_SHOWAMPERSAND)
            Text(strStr);
          else
            HiText(strStr,HIBYTE(LOWORD(Attr)));
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

        if(X < 0)
          X=0;

        if ( (CX2 <= 0) || (CX2 < CX1) )
          CW = LenText;

        if(X1+X+LenText > X2)
        {
          int tmpCW=ObjWidth;
          if(CW < ObjWidth)
            tmpCW=CW+1;
          strStr.SetLength(tmpCW-1);
        }

        // нужно ЭТО
        //SetScreen(X1+CX1,Y1+CY1,X1+CX2,Y1+CY2,' ',Attr&0xFF);
        // вместо этого:
        if(CX1 > -1 && CX2 > CX1 && !(CurItem->Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2))) //половинчатое решение
        {
          int CntChr=CX2-CX1+1;
          SetColor(Attr&0xFF);
          GotoXY(X1+X,Y1+Y);
          if(X1+X+CntChr-1 > X2)
            CntChr=X2-(X1+X)+1;
					FS<<fmt::Width(CntChr)<<L"";
          if (CntChr < LenText)
          	strStr.SetLength(CntChr);
        }

        if (CurItem->Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2))
        {
          SetColor(LOBYTE(HIWORD(Attr)));
          GotoXY(X1+((CurItem->Flags&DIF_SEPARATORUSER)?X:(!DialogMode.Check(DMODE_SMALLDIALOG)?3:0)),Y1+Y); //????

          ShowUserSeparator((CurItem->Flags&DIF_SEPARATORUSER)?X2-X1+1:RealWidth-(!DialogMode.Check(DMODE_SMALLDIALOG)?6:0/* -1 */),
                            (CurItem->Flags&DIF_SEPARATORUSER)?12:(CurItem->Flags&DIF_SEPARATOR2?3:1),
                             CurItem->Mask
                           );
        }

        SetColor(Attr&0xFF);
        GotoXY(X1+X,Y1+Y);

        if (CurItem->Flags & DIF_SHOWAMPERSAND)
        {
          //MessageBox(0, strStr, strStr, MB_OK);
          Text(strStr);
        }
        else
        {
          //MessageBox(0, strStr, strStr, MB_OK);
          HiText(strStr,HIBYTE(LOWORD(Attr)));
        }
        break;
      }

      /* ***************************************************************** */
      case DI_VTEXT:
      {
        strStr = CurItem->strData;
        LenText=LenStrItem(I,strStr);
        if (!(CurItem->Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2)) && (CurItem->Flags & DIF_CENTERTEXT) && CY1!=-1)
          LenText=StrLength(CenterStr(strStr,strStr,CY2-CY1+1));
        X=(CX1==-1)?(X2-X1+1)/2:CX1;
        Y=(CY1==-1 || (CurItem->Flags & (DIF_SEPARATOR|DIF_SEPARATOR2)))?(Y2-Y1+1-LenText)/2:CY1;
        if(Y < 0)
          Y=0;

        if ( (CY2 <= 0) || (CY2 < CY1) )
          CH = LenStrItem(I,strStr);

        if(Y1+Y+LenText > Y2)
        {
          int tmpCH=ObjHeight;
          if(CH < ObjHeight)
            tmpCH=CH+1;
          strStr.SetLength(tmpCH-1);
        }

        // нужно ЭТО
        //SetScreen(X1+CX1,Y1+CY1,X1+CX2,Y1+CY2,' ',Attr&0xFF);
        // вместо этого:
        if(CY1 > -1 && CY2 > 0 && CY2 > CY1 && !(CurItem->Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2))) //половинчатое решение
        {
          int CntChr=CY2-CY1+1;
          SetColor(Attr&0xFF);
          GotoXY(X1+X,Y1+Y);
          if(Y1+Y+CntChr-1 > Y2)
            CntChr=Y2-(Y1+Y)+1;
          vmprintf(L"%*s",CntChr,L"");
        }

#if defined(VTEXT_ADN_SEPARATORS)
        if (CurItem->Flags & (DIF_SEPARATORUSER|DIF_SEPARATOR|DIF_SEPARATOR2))
        {
          SetColor(LOBYTE(HIWORD(Attr)));
          GotoXY(X1+X,Y1+ ((CurItem->Flags&DIF_SEPARATORUSER)?Y:(!DialogMode.Check(DMODE_SMALLDIALOG)?1:0)) ); //????

          ShowUserSeparator((CurItem->Flags&DIF_SEPARATORUSER)?Y2-Y1+1:RealHeight-(!DialogMode.Check(DMODE_SMALLDIALOG)?2:0),
                        (CurItem->Flags&DIF_SEPARATORUSER)?13:(CurItem->Flags&DIF_SEPARATOR2?7:5),
                        CurItem->Mask
                       );
        }
#endif

        SetColor(Attr&0xFF);
        GotoXY(X1+X,Y1+Y);
        if (CurItem->Flags & DIF_SHOWAMPERSAND)
          VText(strStr);
        else
          HiVText(strStr,HIBYTE(LOWORD(Attr)));
        break;
      }

      /* ***************************************************************** */
      case DI_CHECKBOX:
      case DI_RADIOBUTTON:
      {
        SetColor(Attr&0xFF);

        GotoXY(X1+CX1,Y1+CY1);

        if (CurItem->Type==DI_CHECKBOX)
        {
					const wchar_t Check[]={L'[',(CurItem->Selected ?(((CurItem->Flags&DIF_3STATE) && CurItem->Selected == 2)?*MSG(MCheckBox2State):L'x'):L' '),L']',L'\0'};
					strStr=Check;
					if(CurItem->strData.GetLength())
						strStr+=L" ";
        }
        else
        {
					wchar_t Dot[]={L' ',CurItem->Selected ? L'\x2022':L' ',L' ',L'\0'};
					if(CurItem->Flags&DIF_MOVESELECT)
					{
						strStr=Dot;
					}
					else
					{
						Dot[0]=L'(';
						Dot[2]=L')';
						strStr=Dot;
						if(CurItem->strData.GetLength())
							strStr+=L" ";
					}
        }

        strStr += CurItem->strData;

        LenText=LenStrItem(I, strStr);

        if(X1+CX1+LenText > X2)
          strStr.SetLength(ObjWidth-1);

        if (CurItem->Flags & DIF_SHOWAMPERSAND)
          Text(strStr);
        else
          HiText(strStr,HIBYTE(LOWORD(Attr)));

        if (CurItem->Focus)
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

        SetColor(Attr&0xFF);
        GotoXY(X1+CX1,Y1+CY1);

        if (CurItem->Flags & DIF_SHOWAMPERSAND)
          Text(strStr);
        else
          HiText(strStr,HIBYTE(LOWORD(Attr)));
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
        if(!EditPtr)
          break;

        EditPtr->SetObjectColor(Attr&0xFF,HIBYTE(LOWORD(Attr)),LOBYTE(HIWORD(Attr)));

        if (CurItem->Focus)
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

        if ((CurItem->History && (CurItem->Flags & DIF_HISTORY) && Opt.Dialogs.EditHistory) ||
            (CurItem->Type == DI_COMBOBOX && CurItem->ListPtr && CurItem->ListPtr->GetItemCount() > 0))
        {
          int EditX1,EditY1,EditX2,EditY2;

          EditPtr->GetPosition(EditX1,EditY1,EditX2,EditY2);
          //Text((CurItem->Type == DI_COMBOBOX?"\x1F":"\x19"));
          Text(EditX2+1,EditY1,HIBYTE(HIWORD(Attr)),L"\x2193");
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
        if(CurItem->ListPtr)
        {
          //   Перед отрисовкой спросим об изменении цветовых атрибутов
          BYTE RealColors[VMENU_COLOR_COUNT];
					FarListColors ListColors={0};
          ListColors.ColorCount=VMENU_COLOR_COUNT;
          ListColors.Colors=RealColors;

          CurItem->ListPtr->GetColors(&ListColors);
          if(DlgProc((HANDLE)this,DN_CTLCOLORDLGLIST,I,(LONG_PTR)&ListColors))
            CurItem->ListPtr->SetColors(&ListColors);

          // Курсор запоминаем...
          int CurSorVisible,CurSorSize;
          GetCursorType(CurSorVisible,CurSorSize);

          CurItem->ListPtr->Show();

          // .. а теперь восстановим!
          if(FocusPos != I)
            SetCursorType(CurSorVisible,CurSorSize);
        }
        break;
      }
      /* 01.08.2000 SVS $ */


      /* ***************************************************************** */
      case DI_USERCONTROL:
        if(CurItem->VBuf)
        {
          PutText(X1+CX1,Y1+CY1,X1+CX2,Y1+CY2,CurItem->VBuf);
          // не забудем переместить курсор, если он позиционирован.
          if(FocusPos == I)
          {
            if(CurItem->UCData->CursorPos.X != -1 &&
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
    if(CurItem->ListPtr && GetDropDownOpened() && CurItem->ListPtr->IsVisible())
    {
      if((CurItem->Type == DI_COMBOBOX) ||
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
    SetFarTitle(GetDialogTitle());
  }

  DialogMode.Clear(DMODE_DRAWING);  // конец отрисовки диалога!!!
  DialogMode.Set(DMODE_SHOW); // диалог на экране!

  if (DialogMode.Check(DMODE_DRAGGED))
  {
    /*
    - BugZ#813 - DM_RESIZEDIALOG в DN_DRAWDIALOG -> проблема: Ctrl-F5 - отрисовка только полозьев.
      Убираем вызов плагиновго обработчика.
    */
    //DlgProc((HANDLE)this,DN_DRAWDIALOGDONE,1,0);
		DefDlgProc((HANDLE)this,DN_DRAWDIALOGDONE,1,0);
  }
  else
    DlgProc((HANDLE)this,DN_DRAWDIALOGDONE,0,0);

}

int Dialog::LenStrItem(int ID, const wchar_t *lpwszStr)
{
  CriticalSectionLock Lock(CS);

  if ( !lpwszStr )
      lpwszStr = Item[ID]->strData;

  return (Item[ID]->Flags & DIF_SHOWAMPERSAND)?StrLength(lpwszStr):HiStrlen(lpwszStr);
}


int Dialog::ProcessMoveDialog(DWORD Key)
{
  CriticalSectionLock Lock(CS);

  int I;

  if (DialogMode.Check(DMODE_DRAGGED)) // если диалог таскается
  {
    // TODO: Здесь проверить "уже здесь" и не делать лишних движений
    //       Т.е., если нажали End, то при следующем End ненужно ничего делать! - сравнить координаты !!!
    int rr=1;

    //   При перемещении диалога повторяем поведение "бормандовых" сред.
    switch (Key)
    {
        case KEY_CTRLLEFT:  case KEY_CTRLNUMPAD4:
        case KEY_CTRLHOME:  case KEY_CTRLNUMPAD7:
        case KEY_HOME:      case KEY_NUMPAD7:
            rr=Key == KEY_CTRLLEFT || Key == KEY_CTRLNUMPAD4?10:X1;
        case KEY_LEFT:      case KEY_NUMPAD4:
            Hide();
            for ( I=0; I<rr; I++ )
                if ( X2>0 )
                {
                    X1--;
                    X2--;
                    AdjustEditPos(-1,0);
                }
            if(!DialogMode.Check(DMODE_ALTDRAGGED)) Show();
            break;
        case KEY_CTRLRIGHT: case KEY_CTRLNUMPAD6:
        case KEY_CTRLEND:   case KEY_CTRLNUMPAD1:
        case KEY_END:       case KEY_NUMPAD1:
            rr=Key == KEY_CTRLRIGHT || Key == KEY_CTRLNUMPAD6?10:Max(0,ScrX-X2);
        case KEY_RIGHT:     case KEY_NUMPAD6:
            Hide();
            for ( I=0; I<rr; I++ )
                if ( X1<ScrX )
                {
                    X1++;
                    X2++;
                    AdjustEditPos(1,0);
                }
            if(!DialogMode.Check(DMODE_ALTDRAGGED)) Show();
            break;
        case KEY_PGUP:      case KEY_NUMPAD9:
        case KEY_CTRLPGUP:  case KEY_CTRLNUMPAD9:
        case KEY_CTRLUP:    case KEY_CTRLNUMPAD8:
            rr=Key == KEY_CTRLUP || Key == KEY_CTRLNUMPAD8?5:Y1;
        case KEY_UP:        case KEY_NUMPAD8:
            Hide();
            for ( I=0; I<rr; I++ )
                if ( Y2>0 )
                {
                    Y1--;
                    Y2--;
                    AdjustEditPos(0,-1);
                }
            if(!DialogMode.Check(DMODE_ALTDRAGGED)) Show();
            break;
        case KEY_CTRLDOWN:  case KEY_CTRLNUMPAD2:
        case KEY_CTRLPGDN:  case KEY_CTRLNUMPAD3:
        case KEY_PGDN:      case KEY_NUMPAD3:
            rr=Key == KEY_CTRLDOWN || Key == KEY_CTRLNUMPAD2? 5:Max(0,ScrY-Y2);
        case KEY_DOWN:      case KEY_NUMPAD2:
            Hide();
            for ( I=0; I<rr; I++ )
                if ( Y1<ScrY )
                {
                    Y1++;
                    Y2++;
                    AdjustEditPos(0,1);
                }
            if(!DialogMode.Check(DMODE_ALTDRAGGED)) Show();
            break;
        case KEY_NUMENTER:
        case KEY_ENTER:
        case KEY_CTRLF5:
            DialogMode.Clear(DMODE_DRAGGED); // закончим движение!
            if(!DialogMode.Check(DMODE_ALTDRAGGED))
            {
              DlgProc((HANDLE)this,DN_DRAGGED,1,0);
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
            if(!DialogMode.Check(DMODE_ALTDRAGGED))
            {
              DlgProc((HANDLE)this,DN_DRAGGED,1,TRUE);
              Show();
            }
            break;
    }

    if(DialogMode.Check(DMODE_ALTDRAGGED))
    {
      DialogMode.Clear(DMODE_DRAGGED|DMODE_ALTDRAGGED);
      DlgProc((HANDLE)this,DN_DRAGGED,1,0);
      Show();
    }
    return (TRUE);
  }

  if (Key == KEY_CTRLF5 && DialogMode.Check(DMODE_ISCANMOVE))
  {
    if(DlgProc((HANDLE)this,DN_DRAGGED,0,0)) // если разрешили перемещать!
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
  switch(OpCode)
  {
    case MCODE_F_MENU_CHECKHOTKEY:
    {
      const wchar_t *str = (const wchar_t *)vParam;

      if ( GetDropDownOpened() || Item[FocusPos]->Type == DI_LISTBOX )
      {
        if(Item[FocusPos]->ListPtr)
           return Item[FocusPos]->ListPtr->VMProcess(OpCode,vParam,iParam);
      }
      else
         return (__int64)(CheckHighlights(*str,(int)iParam) + 1);
      return 0;
    }
  }
  switch(OpCode)
  {
    case MCODE_C_EOF:
    case MCODE_C_BOF:
    case MCODE_C_SELECTED:
    case MCODE_C_EMPTY:
    {
      if (IsEdit(Item[FocusPos]->Type))
        return ((DlgEdit *)(Item[FocusPos]->ObjPtr))->VMProcess(OpCode,vParam,iParam);
      else if(Item[FocusPos]->Type == DI_LISTBOX && OpCode != MCODE_C_SELECTED)
        return Item[FocusPos]->ListPtr->VMProcess(OpCode,vParam,iParam);
      return 0;
    }

    case MCODE_V_DLGITEMTYPE:
    {
      switch(Item[FocusPos]->Type)
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

		case MCODE_V_DLGINFOID:        // Dlg.Info.Id
		{
			static string strId;
			strId.Format(L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",Id.Data1,Id.Data2,Id.Data3,Id.Data4[0],Id.Data4[1],Id.Data4[2],Id.Data4[3],Id.Data4[4],Id.Data4[5],Id.Data4[6],Id.Data4[7]);
			return reinterpret_cast<INT64>(strId.CPtr());
		}

    case MCODE_V_ITEMCOUNT:
    case MCODE_V_CURPOS:
    {
      switch(Item[FocusPos]->Type)
      {
        case DI_COMBOBOX:
           if(DropDownOpened || (Item[FocusPos]->Flags & DIF_DROPDOWNLIST))
             return Item[FocusPos]->ListPtr->VMProcess(OpCode,vParam,iParam);
        case DI_EDIT:
        case DI_PSWEDIT:
        case DI_FIXEDIT:
           return ((DlgEdit *)(Item[FocusPos]->ObjPtr))->VMProcess(OpCode,vParam,iParam);
        case DI_LISTBOX:
          return Item[FocusPos]->ListPtr->VMProcess(OpCode,vParam,iParam);

        case DI_USERCONTROL:
          if(OpCode == MCODE_V_CURPOS)
            return Item[FocusPos]->UCData->CursorPos.X;
        case DI_BUTTON:
        case DI_CHECKBOX:
        case DI_RADIOBUTTON:
          return 0;
      }
      return 0;
    }

    case MCODE_F_EDITOR_SEL:
    {
      if (IsEdit(Item[FocusPos]->Type) || (Item[FocusPos]->Type==DI_COMBOBOX && !(DropDownOpened || (Item[FocusPos]->Flags & DIF_DROPDOWNLIST))))
      {
        return ((DlgEdit *)(Item[FocusPos]->ObjPtr))->VMProcess(OpCode,vParam,iParam);
      }
			return 0;
    }

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
  _DIALOG(CleverSysLog CL("Dialog::ProcessKey"));
  _DIALOG(SysLog("Param: Key=%s",_FARKEY_ToName(Key)));

  unsigned I;
  string strStr;

  if (Key==KEY_NONE || Key==KEY_IDLE)
  {
    DlgProc((HANDLE)this,DN_ENTERIDLE,0,0); // $ 28.07.2000 SVS Передадим этот факт в обработчик :-)
    return(FALSE);
  }

  if(Key == KEY_KILLFOCUS || Key == KEY_GOTFOCUS)
  {
    DlgProc((HANDLE)this,DN_ACTIVATEAPP,Key == KEY_KILLFOCUS?FALSE:TRUE,0);
    return(FALSE);
  }

  if(ProcessMoveDialog(Key))
    return TRUE;

  // BugZ#488 - Shift=enter
  if(ShiftPressed && (Key == KEY_ENTER||Key==KEY_NUMENTER) && !CtrlObject->Macro.IsExecuting() && Item[FocusPos]->Type != DI_BUTTON)
  {
    Key=Key == KEY_ENTER?KEY_SHIFTENTER:KEY_SHIFTNUMENTER;
  }

  if(!(/*(Key>=KEY_MACRO_BASE && Key <=KEY_MACRO_ENDBASE) ||*/ ((unsigned int)Key>=KEY_OP_BASE && (unsigned int)Key <=KEY_OP_ENDBASE)) && !DialogMode.Check(DMODE_KEY))
    if(DlgProc((HANDLE)this,DN_KEY,FocusPos,Key))
      return TRUE;

  if(!DialogMode.Check(DMODE_SHOW))
     return TRUE;

  // А ХЗ, может в этот момент изменилось состояние элемента!
  if(Item[FocusPos]->Flags&DIF_HIDDEN)
    return TRUE;

  // небольшая оптимизация
  if(Item[FocusPos]->Type==DI_CHECKBOX)
  {
    if(!(Item[FocusPos]->Flags&DIF_3STATE))
    {
      if(Key == KEY_MULTIPLY) // в CheckBox 2-state Gray* не работает!
        Key = KEY_NONE;
      if((Key == KEY_ADD      && !Item[FocusPos]->Selected) ||
         (Key == KEY_SUBTRACT &&  Item[FocusPos]->Selected))
       Key=KEY_SPACE;
    }
    /*
      блок else не нужен, т.к. ниже клавиши будут обработаны...
    */
  }
  else if(Key == KEY_ADD)
    Key='+';
  else if(Key == KEY_SUBTRACT)
    Key='-';
  else if(Key == KEY_MULTIPLY)
    Key='*';

  if (Item[FocusPos]->Type==DI_BUTTON && Key == KEY_SPACE)
    Key=KEY_ENTER;

  if(Item[FocusPos]->Type == DI_LISTBOX)
  {
    switch(Key)
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
        int CheckedListItem=List->GetSelection(-1);

        List->ProcessKey(Key);

        int NewListPos=List->GetSelectPos();
        if(NewListPos != CurListPos && !DlgProc((HANDLE)this,DN_LISTCHANGE,FocusPos,NewListPos))
        {
          if(!DialogMode.Check(DMODE_SHOW))
              return TRUE;
          List->SetSelection(CheckedListItem,CurListPos);
          if(DialogMode.Check(DMODE_SHOW) && !(Item[FocusPos]->Flags&DIF_HIDDEN))
            ShowDialog(FocusPos); // FocusPos
        }

        if(!(Key == KEY_ENTER || Key == KEY_NUMENTER) || (Item[FocusPos]->Flags&DIF_LISTNOCLOSE))
          return(TRUE);

    }
  }

  switch(Key)
  {
//    case KEY_CTRLTAB:
//      DLGIIF_EDITPATH
    case KEY_F1:
      // Перед выводом диалога посылаем сообщение в обработчик
      //   и если вернули что надо, то выводим подсказку
      if(!Help::MkTopic(PluginNumber,
                 (const wchar_t*)DlgProc((HANDLE)this,DN_HELP,FocusPos,
                                (HelpTopic?(LONG_PTR)HelpTopic:0)),
                 strStr).IsEmpty())
      {
        Help Hlp (strStr);
      }
      return(TRUE);

    case KEY_ESC:
    case KEY_BREAK:
    case KEY_F10:
      ExitCode=(Key==KEY_BREAK) ? -2:-1;
      CloseDialog();
      return(TRUE);


    case KEY_HOME: case KEY_NUMPAD7:
      if(Item[FocusPos]->Type == DI_USERCONTROL) // для user-типа вываливаем
        return TRUE;
      return Do_ProcessFirstCtrl();

    case KEY_TAB:
    case KEY_SHIFTTAB:
      return Do_ProcessTab(Key==KEY_TAB);

    case KEY_SPACE:
      return Do_ProcessSpace();

    case KEY_CTRLNUMENTER:
    case KEY_CTRLENTER:
    {
      for (I=0;I<ItemCount;I++)
        if (Item[I]->DefaultButton)
        {
          if(Item[I]->Flags&DIF_DISABLE)
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
          return(TRUE);
        }
      if(!DialogMode.Check(DMODE_OLDSTYLE))
      {
        DialogMode.Clear(DMODE_ENDLOOP); // только если есть
        return TRUE; // делать больше не чего
      }
    }

    case KEY_NUMENTER:
    case KEY_ENTER:
    {
      if (   Item[FocusPos]->Type != DI_COMBOBOX
          && IsEdit(Item[FocusPos]->Type)
          && (Item[FocusPos]->Flags & DIF_EDITOR) && !(Item[FocusPos]->Flags & DIF_READONLY))
      {
        unsigned EditorLastPos;
        for (EditorLastPos=I=FocusPos; I<ItemCount; I++)
          if (IsEdit(Item[I]->Type) && (Item[I]->Flags & DIF_EDITOR))
            EditorLastPos=I;
          else
            break;
        if (((DlgEdit *)(Item[EditorLastPos]->ObjPtr))->GetLength()!=0)
          return(TRUE);
        for (I=EditorLastPos;I>FocusPos;I--)
        {
          int CurPos;
          if (I==FocusPos+1)
            CurPos=((DlgEdit *)(Item[I-1]->ObjPtr))->GetCurPos();
          else
            CurPos=0;
          ((DlgEdit *)(Item[I-1]->ObjPtr))->GetString(strStr);
          int Length=(int)strStr.GetLength();
          ((DlgEdit *)(Item[I]->ObjPtr))->SetString(CurPos>=Length ? L"":(const wchar_t*)strStr+CurPos);

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
        return(TRUE);
      }
      else if (Item[FocusPos]->Type==DI_BUTTON)
      {
        Item[FocusPos]->Selected=1;
        // сообщение - "Кнокна кликнута"
				if(SendDlgMessage((HANDLE)this,DN_BTNCLICK,FocusPos,0))
          return TRUE;

        if(Item[FocusPos]->Flags&DIF_BTNNOCLOSE)
          return(TRUE);

        ExitCode=FocusPos;
        CloseDialog();
        return TRUE;
      }
      else
      {
        ExitCode=-1;
        for (I=0;I<ItemCount;I++)
        {
          if (Item[I]->DefaultButton && !(Item[I]->Flags&DIF_BTNNOCLOSE))
          {
            if(Item[I]->Flags&DIF_DISABLE)
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
      return(TRUE);
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
        if(Item[FocusPos]->Selected != (int)CHKState)
					if(SendDlgMessage((HANDLE)this,DN_BTNCLICK,FocusPos,CHKState))
          {
             Item[FocusPos]->Selected=CHKState;
             ShowDialog();
          }
      }
      return(TRUE);

    case KEY_LEFT:  case KEY_NUMPAD4: case KEY_MSWHEEL_LEFT:
    case KEY_RIGHT: case KEY_NUMPAD6: case KEY_MSWHEEL_RIGHT:
    {
      if(Item[FocusPos]->Type == DI_USERCONTROL) // для user-типа вываливаем
        return TRUE;
      if (IsEdit(Item[FocusPos]->Type))
      {
        ((DlgEdit *)(Item[FocusPos]->ObjPtr))->ProcessKey(Key);
        return(TRUE);
      }
      else
      {
        int MinDist=1000,MinPos=0;
        for (I=0;I<ItemCount;I++)
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
          ChangeFocus2(FocusPos,MinPos);
          if (Item[MinPos]->Flags & DIF_MOVESELECT)
          {
            Do_ProcessSpace();
          }
          else
          {
            ShowDialog();
          }
          return(TRUE);
        }
      }
    }

    case KEY_UP:    case KEY_NUMPAD8:
    case KEY_DOWN:  case KEY_NUMPAD2:
      if(Item[FocusPos]->Type == DI_USERCONTROL) // для user-типа вываливаем
        return TRUE;
      return Do_ProcessNextCtrl(Key==KEY_LEFT || Key==KEY_UP || Key == KEY_NUMPAD4 || Key == KEY_NUMPAD8);

    // $ 27.04.2001 VVM - Обработка колеса мышки
    case KEY_MSWHEEL_UP:
    case KEY_MSWHEEL_DOWN:
    case KEY_CTRLUP:      case KEY_CTRLNUMPAD8:
    case KEY_CTRLDOWN:    case KEY_CTRLNUMPAD2:
      return ProcessOpenComboBox(Item[FocusPos]->Type,Item[FocusPos],FocusPos);

    // ЭТО перед default предпоследний!!!
    case KEY_END:  case KEY_NUMPAD1:
      if(Item[FocusPos]->Type == DI_USERCONTROL) // для user-типа вываливаем
        return TRUE;
      if (IsEdit(Item[FocusPos]->Type))
      {
        ((DlgEdit *)(Item[FocusPos]->ObjPtr))->ProcessKey(Key);
        return(TRUE);
      }
    // ???
    // ЭТО перед default последний!!!
    case KEY_PGDN:   case KEY_NUMPAD3:
      if(Item[FocusPos]->Type == DI_USERCONTROL) // для user-типа вываливаем
        return TRUE;
      if (!(Item[FocusPos]->Flags & DIF_EDITOR))
      {
        for (I=0;I<ItemCount;I++)
          if (Item[I]->DefaultButton)
          {
            ChangeFocus2(FocusPos,I);
            ShowDialog();
            return(TRUE);
          }
        return(TRUE);
      }
      // для DIF_EDITOR будет обработано ниже

    default:
    {
      //if(Item[FocusPos].Type == DI_USERCONTROL) // для user-типа вываливаем
      //  return TRUE;

      if(Item[FocusPos]->Type == DI_LISTBOX)
      {
        VMenu *List=Item[FocusPos]->ListPtr;
        int CurListPos=List->GetSelectPos();
        int CheckedListItem=List->GetSelection(-1);

        List->ProcessKey(Key);
        int NewListPos=List->GetSelectPos();
        if(NewListPos != CurListPos && !DlgProc((HANDLE)this,DN_LISTCHANGE,FocusPos,NewListPos))
        {
          if(!DialogMode.Check(DMODE_SHOW))
              return TRUE;
          List->SetSelection(CheckedListItem,CurListPos);
          if(DialogMode.Check(DMODE_SHOW) && !(Item[FocusPos]->Flags&DIF_HIDDEN))
            ShowDialog(FocusPos); // FocusPos
        }
        return(TRUE);
      }

      if (IsEdit(Item[FocusPos]->Type))
      {
        DlgEdit *edt=(DlgEdit *)Item[FocusPos]->ObjPtr;

        if(Key == KEY_CTRLL) // исключим смену режима RO для поля ввода с клавиатуры
        {
          return TRUE;
        }

        else if(Key == KEY_CTRLU)
        {
          edt->SetClearFlag(0);
          edt->Select(-1,0);
          edt->Show();
          return TRUE;
        }

        else if((Item[FocusPos]->Flags & DIF_EDITOR) && !(Item[FocusPos]->Flags & DIF_READONLY))
        {
          switch(Key)
          {
            case KEY_BS:
            {
              int CurPos=edt->GetCurPos();
              // В начале строки????
              if(!edt->GetCurPos())
              {
                // а "выше" тоже DIF_EDITOR?
                if(FocusPos > 0 && (Item[FocusPos-1]->Flags&DIF_EDITOR))
                {
                  // добавляем к предыдущему и...
                  DlgEdit *edt_1=(DlgEdit *)Item[FocusPos-1]->ObjPtr;
									edt_1->GetString(strStr);
									CurPos=static_cast<int>(strStr.GetLength());
									string strAdd;
									edt->GetString(strAdd);
									strStr+=strAdd;
									edt_1->SetString(strStr);

                  for (I=FocusPos+1;I<ItemCount;I++)
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
              return(TRUE);
            }

            case KEY_CTRLY:
            {
              for (I=FocusPos;I<ItemCount;I++)
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
              return(TRUE);
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
                if(SelStart > -1)
                {
									string strEnd=strStr.CPtr()+SelEnd;
									strStr.SetLength(SelStart);
									strStr+=strEnd;
									edt->SetString(strStr);
                  edt->SetCurPos(SelStart);

                  ShowDialog();
                  return(TRUE);
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
                  return(TRUE);
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
              if(!(Item[I]->Flags & DIF_EDITOR))
                I=ChangeFocus(I,(Key == KEY_PGUP || Key == KEY_NUMPAD9)?1:-1,FALSE);

              unsigned oldFocus=FocusPos;
              ChangeFocus2(FocusPos,I);
              if(oldFocus != I)
              {
                ShowDialog(oldFocus);
                ShowDialog(FocusPos); // ?? I ??
              }
              return(TRUE);
            }
          }
        }

				if(Key == KEY_OP_XLAT && !(Item[FocusPos]->Flags & DIF_READONLY))
        {
          edt->SetClearFlag(0);
          edt->Xlat();
          Redraw(); // Перерисовка должна идти после DN_EDITCHANGE (imho)
          return TRUE;
        }


        if(!(Item[FocusPos]->Flags & DIF_READONLY) ||
            ((Item[FocusPos]->Flags & DIF_READONLY) && IsNavKey(Key)))
        {
          // "только что ломанулись и начинать выделение с нуля"?
          if((Opt.Dialogs.EditLine&DLGEDITLINE_NEWSELONGOTFOCUS) && Item[FocusPos]->SelStart != -1 && PrevFocusPos != FocusPos)// && Item[FocusPos].SelEnd)
          {
            edt->Flags().Clear(FEDITLINE_MARKINGBLOCK);
            PrevFocusPos=FocusPos;
          }

          if (edt->ProcessKey(Key))
          {
            if(Item[FocusPos]->Flags & DIF_READONLY)
              return TRUE;

            //int RedrawNeed=FALSE;
            /* $ 26.07.2000 SVS
               AutoComplite: Если установлен DIF_HISTORY
                   и разрешено автозавершение!.
            */
            /* $ 04.12.2000 SVS
              Автодополнение - чтобы не работало во время проигрывания макросов.
              GetCurRecord() вернет 0 для случая, если нет ни записи ни проигрыша.
            */

            if(!(Item[FocusPos]->Flags & DIF_NOAUTOCOMPLETE))
                        if(CtrlObject->Macro.GetCurRecord(NULL,NULL) == MACROMODE_NOMACRO &&
               ((Item[FocusPos]->Flags & DIF_HISTORY) || Item[FocusPos]->Type == DI_COMBOBOX))
            if((Opt.Dialogs.AutoComplete && Key && Key < 0x10000 && Key != KEY_BS && !(Key == KEY_DEL||Key == KEY_NUMDEL)) ||
               (!Opt.Dialogs.AutoComplete && (Key == KEY_CTRLEND || Key == KEY_CTRLNUMPAD1))
              )
            {
							string strStr;
							edt->GetString(strStr);
							int SelStart,SelEnd;
							edt->GetSelection(SelStart,SelEnd);
							if(SelStart < 0 || SelStart==SelEnd)
								SelStart=(int)strStr.GetLength();
							else
								SelStart++;
							int CurPos=edt->GetCurPos();
							bool DoAutoComplete=(CurPos>=SelStart && (SelStart>=SelEnd || SelEnd>=(int)strStr.GetLength()));
							if(Opt.Dialogs.EditBlock)
							{
								if(DoAutoComplete && CurPos <= SelEnd)
								{
									strStr.SetLength(CurPos);
									edt->Select(CurPos,edt->GetLength()); //select the appropriate text
									edt->DeleteBlock();
									edt->FastShow();
								}
							}
							SelEnd=(int)strStr.GetLength();
							if(DoAutoComplete && FindInEditForAC(Item[FocusPos]->Type == DI_COMBOBOX,Item[FocusPos]->History,strStr))
							{
								edt->SetString(strStr);
								edt->Select(SelEnd,edt->GetLength()); //select the appropriate text
								edt->SetCurPos(CurPos); // SelEnd
							}
						}

            Redraw(); // Перерисовка должна идти после DN_EDITCHANGE (imho)
            return(TRUE);
          }
        }
        else if(!(Key&(KEY_ALT|KEY_RALT)))
          return TRUE;

      }

      if (ProcessHighlighting(Key,FocusPos,FALSE))
        return(TRUE);

      return(ProcessHighlighting(Key,FocusPos,TRUE));
    }
  }
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
  int Type;
  RECT Rect;

  if(!DialogMode.Check(DMODE_SHOW))
    return FALSE;

  if(DialogMode.Check(DMODE_MOUSEEVENT))
  {
    if(!DlgProc((HANDLE)this,DN_MOUSEEVENT,0,(LONG_PTR)MouseEvent))
      return TRUE;
  }

  if(!DialogMode.Check(DMODE_SHOW))
    return FALSE;

  MsX=MouseEvent->dwMousePosition.X;
  MsY=MouseEvent->dwMousePosition.Y;

  //for (I=0;I<ItemCount;I++)
  for (I=ItemCount-1;I!=(unsigned)-1;I--)
  {
    if(Item[I]->Flags&(DIF_DISABLE|DIF_HIDDEN))
      continue;
    Type=Item[I]->Type;
    if (Type == DI_LISTBOX &&
        MsY >= Y1+Item[I]->Y1 && MsY <= Y1+Item[I]->Y2 &&
        MsX >= X1+Item[I]->X1 && MsX <= X1+Item[I]->X2)
    {
      VMenu *List=Item[I]->ListPtr;
      int Pos=List->GetSelectPos();
      int CheckedListItem=List->GetSelection(-1);
      if((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED))
      {
        if(FocusPos != I)
        {
          ChangeFocus2(FocusPos,I);
          ShowDialog();
        }

        if(MouseEvent->dwEventFlags!=DOUBLE_CLICK && (Item[I]->IFlags.Flags&(DLGIIF_LISTREACTIONFOCUS|DLGIIF_LISTREACTIONNOFOCUS)) == 0)
        {
          List->ProcessMouse(MouseEvent);
          int NewListPos=List->GetSelectPos();

          if(NewListPos != Pos && !SendDlgMessage((HANDLE)this,DN_LISTCHANGE,I,(LONG_PTR)NewListPos))
          {
            List->SetSelection(CheckedListItem,Pos);
            if(DialogMode.Check(DMODE_SHOW) && !(Item[I]->Flags&DIF_HIDDEN))
              ShowDialog(I); // FocusPos
          }
          else
          {
            Pos=NewListPos;
          }
        }
        else if (!SendDlgMessage((HANDLE)this,DN_MOUSECLICK,I,(LONG_PTR)MouseEvent))
        {
#if 1
          List->ProcessMouse(MouseEvent);
          int NewListPos=List->GetSelectPos();
          int InScroolBar=(MsX==X1+Item[I]->X2 && MsY >= Y1+Item[I]->Y1 && MsY <= Y1+Item[I]->Y2) &&
                          (List->CheckFlags(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR) || Opt.ShowMenuScrollbar);
          if(!InScroolBar       &&                                                                 // вне скроллбара и
              NewListPos != Pos &&                                                                 // позиция изменилась и
              !SendDlgMessage((HANDLE)this,DN_LISTCHANGE,I,(LONG_PTR)NewListPos))                      // и плагин сказал в морг
          {
            List->SetSelection(CheckedListItem,Pos);
            if(DialogMode.Check(DMODE_SHOW) && !(Item[I]->Flags&DIF_HIDDEN))
              ShowDialog(I); // FocusPos
          }
          else
          {
            Pos=NewListPos;
            if(!InScroolBar && !(Item[I]->Flags&DIF_LISTNOCLOSE))
            {
              ExitCode=I;
              CloseDialog();
              return TRUE;
            }
          }
#else
          if (SendDlgMessage((HANDLE)this,DN_LISTCHANGE,I,(LONG_PTR)Pos))
          {
            if (MsX==X1+Item[I]->X2 && MsY >= Y1+Item[I]->Y1 && MsY <= Y1+Item[I]->Y2)
              List->ProcessMouse(MouseEvent); // забыл проверить на клик на скролбар (KM)
            else
              ProcessKey(KEY_ENTER);
          }
#endif
        }
        return TRUE;
      }
      else
      {
        if( !MouseEvent->dwButtonState || SendDlgMessage((HANDLE)this,DN_MOUSECLICK,I,(LONG_PTR)MouseEvent) )
        {
          if((I == FocusPos && (Item[I]->IFlags.Flags&DLGIIF_LISTREACTIONFOCUS))
              ||
             (I != FocusPos && (Item[I]->IFlags.Flags&DLGIIF_LISTREACTIONNOFOCUS))
            )
          {
            List->ProcessMouse(MouseEvent);
            int NewListPos=List->GetSelectPos();
            if(NewListPos != Pos && !SendDlgMessage((HANDLE)this,DN_LISTCHANGE,I,(LONG_PTR)NewListPos))
            {
              List->SetSelection(CheckedListItem,Pos);
              if(DialogMode.Check(DMODE_SHOW) && !(Item[I]->Flags&DIF_HIDDEN))
                ShowDialog(I); // FocusPos
            }
            else
              Pos=NewListPos;
          }
        }
      }
      return(TRUE);
    }
  }

  if ((MsX<X1 || MsY<Y1 || MsX>X2 || MsY>Y2) && MouseEventFlags != MOUSE_MOVED)
  {
    if(DialogMode.Check(DMODE_CLICKOUTSIDE) && !DlgProc((HANDLE)this,DN_MOUSECLICK,-1,(LONG_PTR)MouseEvent))
    {
      if(!DialogMode.Check(DMODE_SHOW))
        return FALSE;
//      if (!(MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) && PrevLButtonPressed && ScreenObject::CaptureMouseObject)
			if (!(MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) && (PrevMouseButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) && (Opt.Dialogs.MouseButton&DMOUSEBUTTON_LEFT))
        ProcessKey(KEY_ESC);
//      else if (!(MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED) && PrevRButtonPressed && ScreenObject::CaptureMouseObject)
			else if (!(MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED) && (PrevMouseButtonState&RIGHTMOST_BUTTON_PRESSED) && (Opt.Dialogs.MouseButton&DMOUSEBUTTON_RIGHT))
        ProcessKey(KEY_ENTER);
    }

    if (MouseEvent->dwButtonState)
      DialogMode.Set(DMODE_CLICKOUTSIDE);
      //ScreenObject::SetCapture(this);

    return(TRUE);
  }

  if (MouseEvent->dwButtonState==0)
  {
    DialogMode.Clear(DMODE_CLICKOUTSIDE);
//    ScreenObject::SetCapture(NULL);
    return(FALSE);
  }

  if (MouseEvent->dwEventFlags==0 || MouseEvent->dwEventFlags==DOUBLE_CLICK)
  {
    // первый цикл - все за исключением рамок.
    //for (I=0; I < ItemCount;I++)
    for (I=ItemCount-1;I!=(unsigned)-1;I--)
    {
      if(Item[I]->Flags&(DIF_DISABLE|DIF_HIDDEN))
        continue;

      GetItemRect(I,Rect);
      Rect.left+=X1;  Rect.top+=Y1;
      Rect.right+=X1; Rect.bottom+=Y1;
//_D(SysLog(L"? %2d) Rect (%2d,%2d) (%2d,%2d) '%s'",I,Rect.left,Rect.top,Rect.right,Rect.bottom,Item[I].Data));

      if(MsX >= Rect.left && MsY >= Rect.top && MsX <= Rect.right && MsY <= Rect.bottom)
      {
        // для прозрачных :-)
        if(Item[I]->Type == DI_SINGLEBOX || Item[I]->Type == DI_DOUBLEBOX)
        {
          // если на рамке, то...
          if(((MsX == Rect.left || MsX == Rect.right) && MsY >= Rect.top && MsY <= Rect.bottom) || // vert
             ((MsY == Rect.top  || MsY == Rect.bottom) && MsX >= Rect.left && MsX <= Rect.right) )   // hor
          {
            if(DlgProc((HANDLE)this,DN_MOUSECLICK,I,(LONG_PTR)MouseEvent))
              return TRUE;
            if(!DialogMode.Check(DMODE_SHOW))
              return FALSE;
          }
          else
            continue;
        }

        if(Item[I]->Type == DI_USERCONTROL)
        {
          // для user-типа подготовим координаты мыши
          MouseEvent->dwMousePosition.X-=(short)Rect.left;
          MouseEvent->dwMousePosition.Y-=(short)Rect.top;
        }

//_SVS(SysLog(L"+ %2d) Rect (%2d,%2d) (%2d,%2d) '%s' Dbl=%d",I,Rect.left,Rect.top,Rect.right,Rect.bottom,Item[I].Data,MouseEvent->dwEventFlags==DOUBLE_CLICK));
        if(DlgProc((HANDLE)this,DN_MOUSECLICK,I,(LONG_PTR)MouseEvent))
          return TRUE;

        if(!DialogMode.Check(DMODE_SHOW))
          return TRUE;

        if(Item[I]->Type == DI_USERCONTROL)
        {
           ChangeFocus2(FocusPos,I);
           ShowDialog();
           return(TRUE);
        }
        break;
      }
    }

    if((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED))
    {
      //for (I=0;I<ItemCount;I++)
      unsigned OldFocusPos=FocusPos;
      for (I=ItemCount-1;I!=(unsigned)-1;I--)
      {
        //   Исключаем из списка оповещаемых о мыши недоступные элементы
        if(Item[I]->Flags&(DIF_DISABLE|DIF_HIDDEN))
          continue;

        Type=Item[I]->Type;
        if (MsX>=X1+Item[I]->X1)
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

            if(MsY==EditY1 && Type == DI_COMBOBOX &&
               (Item[I]->Flags & DIF_DROPDOWNLIST) &&
               MsX >= EditX1 && MsX <= EditX2+1)
            {
              EditLine->SetClearFlag(0);
              if(!(Item[I]->Flags&DIF_NOFOCUS))
                ChangeFocus2(FocusPos,I);
              else
              {
                Item[FocusPos]->Focus=0; //??
                FocusPos=I;
              }
              ShowDialog();
              ProcessOpenComboBox(Item[FocusPos]->Type,Item[FocusPos],FocusPos);
              //ProcessKey(KEY_CTRLDOWN);
              if(Item[I]->Flags&DIF_NOFOCUS) //???
                FocusPos=OldFocusPos;       //???
              return(TRUE);
            }

            if (EditLine->ProcessMouse(MouseEvent))
            {
              EditLine->SetClearFlag(0); // а может это делать в самом edit?
              if(!(Item[I]->Flags&DIF_NOFOCUS)) //??? !!!
                ChangeFocus2(FocusPos,I);      //??? !!!
              else
              {
                Item[FocusPos]->Focus=0; //??
                FocusPos=I;
              }
              /* $ 23.06.2001 KM
                 ! Оказалось нужно перерисовывать весь диалог иначе
                   не снимался признак активности с комбобокса с которго уходим.
              */
              ShowDialog(); // нужен ли только один контрол или весь диалог?
              return(TRUE);
            }
            else
            {
              // Проверка на DI_COMBOBOX здесь лишняя. Убрана (KM).
              if (MsX==EditX2+1 && MsY==EditY1 &&
                  (Item[I]->History ||
                    (Type == DI_COMBOBOX && Item[I]->ListPtr && Item[I]->ListPtr->GetItemCount())
                  ) &&
                  (((Item[I]->Flags & DIF_HISTORY) && Opt.Dialogs.EditHistory)
                   || Type == DI_COMBOBOX))
//                  ((Item[I].Flags & DIF_HISTORY) && Opt.Dialogs.EditHistory))
              {
                EditLine->SetClearFlag(0); // раз уж покусились на, то и...
                if(!(Item[I]->Flags&DIF_NOFOCUS))
                  ChangeFocus2(FocusPos,I);
                else
                {
                  Item[FocusPos]->Focus=0; //??
                  FocusPos=I;
                }
                if(!(Item[I]->Flags&DIF_HIDDEN))
                  ShowDialog(I);
                ProcessOpenComboBox(Item[FocusPos]->Type,Item[FocusPos],FocusPos);
                //ProcessKey(KEY_CTRLDOWN);
                if(Item[I]->Flags&DIF_NOFOCUS) //???
                   FocusPos=OldFocusPos;      //???
                return(TRUE);
              }
            }
          }

          /* ********************************************************** */
          if (Type==DI_BUTTON &&
              MsY==Y1+Item[I]->Y1 &&
              MsX < X1+Item[I]->X1+HiStrlen(Item[I]->strData))
          {
            if(!(Item[I]->Flags&DIF_NOFOCUS))
            {
              ChangeFocus2(FocusPos,I);
              ShowDialog();
            }
            else
            {
              Item[FocusPos]->Focus=0;
              FocusPos=I;
            }
            while (IsMouseButtonPressed())
              ;
            if (MouseX <  X1 ||
                MouseX >  X1+Item[I]->X1+HiStrlen(Item[I]->strData)+4 ||
                MouseY != Y1+Item[I]->Y1)
            {
              if(!(Item[I]->Flags&DIF_NOFOCUS))
              {
                ChangeFocus2(FocusPos,I);
                ShowDialog();
              }
              return(TRUE);
            }
            ProcessKey(KEY_ENTER);
            return(TRUE);
          }

          /* ********************************************************** */
          if ((Type == DI_CHECKBOX ||
               Type == DI_RADIOBUTTON) &&
              MsY==Y1+Item[I]->Y1 &&
              MsX < (X1+Item[I]->X1+HiStrlen(Item[I]->strData)+4-((Item[I]->Flags & DIF_MOVESELECT)!=0)))
          {
            if(!(Item[I]->Flags&DIF_NOFOCUS))
              ChangeFocus2(FocusPos,I);
            else
            {
              Item[FocusPos]->Focus=0; //??
              FocusPos=I;
            }
            ProcessKey(KEY_SPACE);
//            if(Item[I].Flags&DIF_NOFOCUS)
//              FocusPos=OldFocusPos;
            return(TRUE);
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
        MsX=abs(X1-MouseX);
        MsY=abs(Y1-MouseY);

        int NeedSendMsg=0;

				for(;;)
        {
					DWORD Mb=IsMouseButtonPressed();

          int mx,my,X0,Y0;
					if(Mb==FROM_LEFT_1ST_BUTTON_PRESSED) // still dragging
          {
            int AdjX=0,AdjY=0;
            int OX1=X1;
            int OY1=Y1;
            int NX1=X0=X1;
            int NX2=X2;
            int NY1=Y0=Y1;
            int NY2=Y2;

            if(MouseX==PrevMouseX)
              mx=X1;
            else
              mx=MouseX-MsX;
            if(MouseY==PrevMouseY)
              my=Y1;
            else
              my=MouseY-MsY;

            NX2=mx+(X2-X1);
            NX1=mx;
            AdjX=NX1-X0;

            NY2=my+(Y2-Y1);
            NY1=my;
            AdjY=NY1-Y0;

            // "А был ли мальчик?" (про холостой ход)
            if(OX1 != NX1 || OY1 != NY1)
            {
              if(!NeedSendMsg) // тыкс, а уже посылку делали в диалоговую процедуру?
              {
                NeedSendMsg++;
                if(!DlgProc((HANDLE)this,DN_DRAGGED,0,0)) // а может нас обломали?
                  break;  // валим отсель...плагин сказал - в морг перемещения
                if(!DialogMode.Check(DMODE_SHOW))
                  break;
              }

              // Да, мальчик был. Зачнем...
              {
                LockScreen LckScr;
                Hide();
                X1=NX1; X2=NX2; Y1=NY1; Y2=NY2;
                if(AdjX || AdjY)
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
            DlgProc((HANDLE)this,DN_DRAGGED,1,TRUE);
            if(DialogMode.Check(DMODE_SHOW))
              Show();
            break;
          }
          else  // release key, drop dialog
          {
            if(OldX1!=X1 || OldX2!=X2 || OldY1!=Y1 || OldY2!=Y2)
            {
              LockScreen LckScr;
              DialogMode.Clear(DMODE_DRAGGED);
              DlgProc((HANDLE)this,DN_DRAGGED,1,0);
              if(DialogMode.Check(DMODE_SHOW))
                Show();
            }
            break;
          }

        }// while (1)
      }
    }
  }
  return(FALSE);
}


int Dialog::ProcessOpenComboBox(int Type,DialogItemEx *CurItem, unsigned CurFocusPos)
{
  CriticalSectionLock Lock(CS);

  string strStr;
  DlgEdit *CurEditLine;

  // для user-типа вываливаем
  if(Type == DI_USERCONTROL)
    return TRUE;

  CurEditLine=((DlgEdit *)(CurItem->ObjPtr));
  if (IsEdit(Type) &&
       (CurItem->Flags & DIF_HISTORY) &&
       Opt.Dialogs.EditHistory &&
       CurItem->History &&
       !(CurItem->Flags & DIF_READONLY))
  {
    // Передаем то, что в строке ввода в функцию выбора из истории для выделения нужного пункта в истории.
    CurEditLine->GetString(strStr);

		SelectFromEditHistory(CurItem,CurEditLine,CurItem->History,strStr);

  }
  // $ 18.07.2000 SVS:  +обработка DI_COMBOBOX - выбор из списка!
  else if(Type == DI_COMBOBOX && CurItem->ListPtr &&
          !(CurItem->Flags & DIF_READONLY) &&
          CurItem->ListPtr->GetItemCount() > 0) //??
  {
		SelectFromComboBox(CurItem,CurEditLine,CurItem->ListPtr);
  }
  return(TRUE);
}

unsigned Dialog::ProcessRadioButton(unsigned CurRB)
{
  CriticalSectionLock Lock(CS);

  unsigned PrevRB=CurRB, J;
  unsigned I;

  for (I=CurRB;;I--)
  {
    if(I==0)
      break;

    if (Item[I]->Type==DI_RADIOBUTTON && (Item[I]->Flags & DIF_GROUP))
      break;

    if(Item[I-1]->Type!=DI_RADIOBUTTON)
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
    if(J)
    {
      PrevRB=I;
    }
    ++I;

  } while (I<ItemCount && Item[I]->Type==DI_RADIOBUTTON &&
           (Item[I]->Flags & DIF_GROUP)==0);

  Item[CurRB]->Selected=1;
  /* $ 28.07.2000 SVS
    При изменении состояния каждого элемента посылаем сообщение
    посредством функции SendDlgMessage - в ней делается все!
  */
	if(!SendDlgMessage((HANDLE)this,DN_BTNCLICK,PrevRB,0) ||
		!SendDlgMessage((HANDLE)this,DN_BTNCLICK,CurRB,1))
  {
     // вернем назад, если пользователь не захотел...
     Item[CurRB]->Selected=0;
     Item[PrevRB]->Selected=1;
     return PrevRB;
  }

  return CurRB;
}


int Dialog::Do_ProcessFirstCtrl()
{
  CriticalSectionLock Lock(CS);

  if (IsEdit(Item[FocusPos]->Type))
  {
    ((DlgEdit *)(Item[FocusPos]->ObjPtr))->ProcessKey(KEY_HOME);
    return(TRUE);
  }
  else
  {
    for (unsigned I=0;I<ItemCount;I++)
      if (CanGetFocus(Item[I]->Type))
      {
        unsigned OldPos=FocusPos;
        ChangeFocus2(FocusPos,I);
        if(OldPos!=FocusPos)
        {
          ShowDialog(OldPos);
          ShowDialog(FocusPos);
        }
        break;
      }
  }
  return(TRUE);
}

int Dialog::Do_ProcessNextCtrl(int Up,BOOL IsRedraw)
{
  CriticalSectionLock Lock(CS);

  unsigned OldPos=FocusPos;
  unsigned PrevPos=0;

  if (IsEdit(Item[FocusPos]->Type) && (Item[FocusPos]->Flags & DIF_EDITOR))
    PrevPos=((DlgEdit *)(Item[FocusPos]->ObjPtr))->GetCurPos();

  unsigned I=ChangeFocus(FocusPos,Up? -1:1,FALSE);
  Item[FocusPos]->Focus=0;
  Item[I]->Focus=1;
  ChangeFocus2(FocusPos,I);

  if (IsEdit(Item[I]->Type) && (Item[I]->Flags & DIF_EDITOR))
    ((DlgEdit *)(Item[I]->ObjPtr))->SetCurPos(PrevPos);

  if (Item[FocusPos]->Type == DI_RADIOBUTTON && (Item[I]->Flags & DIF_MOVESELECT))
    ProcessKey(KEY_SPACE);
  else if(IsRedraw)
  {
    ShowDialog(OldPos);
    ShowDialog(FocusPos);
  }

  return(TRUE);
}

int Dialog::Do_ProcessTab(int Next)
{
  CriticalSectionLock Lock(CS);

  unsigned I;
  if(ItemCount > 1)
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
        while (I>0 && (Item[I]->Flags & DIF_EDITOR)!=0 &&
               (Item[I-1]->Flags & DIF_EDITOR)!=0 &&
               ((DlgEdit *)Item[I]->ObjPtr)->GetLength()==0)
          I--;
    }
  }
  else
    I=FocusPos;

  unsigned oldFocus=FocusPos;
  ChangeFocus2(FocusPos,I);
  if(oldFocus != I)
  {
    ShowDialog(oldFocus);
    ShowDialog(FocusPos); // ?? I ??
  }
  return(TRUE);
}


int Dialog::Do_ProcessSpace()
{
  CriticalSectionLock Lock(CS);

  int OldFocusPos;
  if (Item[FocusPos]->Type==DI_CHECKBOX)
  {
    int OldSelected=Item[FocusPos]->Selected;

    if(Item[FocusPos]->Flags&DIF_3STATE)
      (++Item[FocusPos]->Selected)%=3;
    else
      Item[FocusPos]->Selected = !Item[FocusPos]->Selected;

    OldFocusPos=FocusPos;
		if(!SendDlgMessage((HANDLE)this,DN_BTNCLICK,FocusPos,Item[FocusPos]->Selected))
      Item[OldFocusPos]->Selected = OldSelected;

    ShowDialog();
    return(TRUE);
  }
  else if (Item[FocusPos]->Type==DI_RADIOBUTTON)
  {
    FocusPos=ProcessRadioButton(FocusPos);
    ShowDialog();
    return(TRUE);
  }
  else if (IsEdit(Item[FocusPos]->Type) && !(Item[FocusPos]->Flags & DIF_READONLY))
  {
    if(((DlgEdit *)(Item[FocusPos]->ObjPtr))->ProcessKey(KEY_SPACE))
    {
      Redraw(); // Перерисовка должна идти после DN_EDITCHANGE (imho)
    }
    return(TRUE);
  }
  return(TRUE);
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

  int Type;
  unsigned OrigFocusPos=CurFocusPos;
//  int FucusPosNeed=-1;
  // В функцию обработки диалога здесь передаем сообщение,
  //   что элемент - LostFocus() - теряет фокус ввода.
//  if(DialogMode.Check(DMODE_INITOBJECTS))
//    FucusPosNeed=DlgProc((HANDLE)this,DN_KILLFOCUS,FocusPos,0);
//  if(FucusPosNeed != -1 && CanGetFocus(Item[FucusPosNeed].Type))
//    FocusPos=FucusPosNeed;
//  else
  {
		for(;;)
    {
      CurFocusPos+=Step;
      if ((int)CurFocusPos<0)
        CurFocusPos=ItemCount-1;
      if (CurFocusPos>=ItemCount)
        CurFocusPos=0;

      Type=Item[CurFocusPos]->Type;

      if(!(Item[CurFocusPos]->Flags&(DIF_NOFOCUS|DIF_DISABLE|DIF_HIDDEN)))
      {
        if (Type==DI_LISTBOX || Type==DI_BUTTON || Type==DI_CHECKBOX || IsEdit(Type) || Type==DI_USERCONTROL)
          break;
        if (Type==DI_RADIOBUTTON && (!SkipGroup || Item[CurFocusPos]->Selected))
          break;
      }
      // убираем зацикливание с последующим подвисанием :-)
      if(OrigFocusPos == CurFocusPos)
        break;
    }
  }

//  Dialog::FocusPos=FocusPos;
  // В функцию обработки диалога здесь передаем сообщение,
  //   что элемент GotFocus() - получил фокус ввода.
  // Игнорируем возвращаемое функцией диалога значение
//  if(DialogMode.Check(DMODE_INITOBJECTS))
//    DlgProc((HANDLE)this,DN_GOTFOCUS,FocusPos,0);
  return(CurFocusPos);
}


//////////////////////////////////////////////////////////////////////////
/*
   Private:
   Изменяет фокус ввода между двумя элементами.
   Вынесен отдельно с тем, чтобы обработать DN_KILLFOCUS & DM_SETFOCUS
*/
unsigned Dialog::ChangeFocus2(unsigned KillFocusPos,unsigned SetFocusPos)
{
  CriticalSectionLock Lock(CS);

  int FucusPosNeed=-1;
  if(!(Item[SetFocusPos]->Flags&(DIF_NOFOCUS|DIF_DISABLE|DIF_HIDDEN)))
  {
    if(DialogMode.Check(DMODE_INITOBJECTS))
    {
      FucusPosNeed=(int)DlgProc((HANDLE)this,DN_KILLFOCUS,KillFocusPos,0);
      if(!DialogMode.Check(DMODE_SHOW))
         return SetFocusPos;
    }

    if(FucusPosNeed != -1 && CanGetFocus(Item[FucusPosNeed]->Type))
      SetFocusPos=FucusPosNeed;

    if(Item[SetFocusPos]->Flags&DIF_NOFOCUS)
       SetFocusPos=KillFocusPos;

    Item[KillFocusPos]->Focus=0;

    // "снимать выделение при потере фокуса?"
    if(IsEdit(Item[KillFocusPos]->Type) &&
       !(Item[KillFocusPos]->Type == DI_COMBOBOX && (Item[KillFocusPos]->Flags & DIF_DROPDOWNLIST)))
    {
      DlgEdit *EditPtr=(DlgEdit*)Item[KillFocusPos]->ObjPtr;
      EditPtr->GetSelection(Item[KillFocusPos]->SelStart,Item[KillFocusPos]->SelEnd);
      if((Opt.Dialogs.EditLine&DLGEDITLINE_CLEARSELONKILLFOCUS))
      {
        EditPtr->Select(-1,0);
      }
    }

    Item[SetFocusPos]->Focus=1;

    // "не восстанавливать выделение при получении фокуса?"
    if(IsEdit(Item[SetFocusPos]->Type) &&
       !(Item[SetFocusPos]->Type == DI_COMBOBOX && (Item[SetFocusPos]->Flags & DIF_DROPDOWNLIST)))
    {
      DlgEdit *EditPtr=(DlgEdit*)Item[SetFocusPos]->ObjPtr;
      if(!(Opt.Dialogs.EditLine&DLGEDITLINE_NOTSELONGOTFOCUS))
      {
        if(Opt.Dialogs.EditLine&DLGEDITLINE_SELALLGOTFOCUS)
          EditPtr->Select(0,EditPtr->GetStrSize());
        else
          EditPtr->Select(Item[SetFocusPos]->SelStart,Item[SetFocusPos]->SelEnd);
      }
      else
      {
        EditPtr->Select(-1,0);
      }

      // при получении фокуса ввода переместить курсор в конец строки?
      if(Opt.Dialogs.EditLine&DLGEDITLINE_GOTOEOLGOTFOCUS)
      {
        EditPtr->SetCurPos(EditPtr->GetStrSize());
      }
    }

    //   проинформируем листбокс, есть ли у него фокус
    if (Item[KillFocusPos]->Type == DI_LISTBOX)
      Item[KillFocusPos]->ListPtr->ClearFlags (VMENU_LISTHASFOCUS);
    if (Item[SetFocusPos]->Type == DI_LISTBOX)
      Item[SetFocusPos]->ListPtr->SetFlags (VMENU_LISTHASFOCUS);

    Dialog::PrevFocusPos=Dialog::FocusPos;
    Dialog::FocusPos=SetFocusPos;
    if(DialogMode.Check(DMODE_INITOBJECTS))
      DlgProc((HANDLE)this,DN_GOTFOCUS,SetFocusPos,0);
  }
  else
    SetFocusPos=KillFocusPos;

  SelectOnEntry(KillFocusPos,FALSE);
  SelectOnEntry(SetFocusPos,TRUE);
  return(SetFocusPos);
}

/*
  Функция SelectOnEntry - выделение строки редактирования
  Обработка флага DIF_SELECTONENTRY
*/
void Dialog::SelectOnEntry(unsigned Pos,BOOL Selected)
{
  //if(!DialogMode.Check(DMODE_SHOW))
  //   return;
  if(IsEdit(Item[Pos]->Type) &&
     (Item[Pos]->Flags&DIF_SELECTONENTRY)
//     && PrevFocusPos != -1 && PrevFocusPos != Pos
    )
  {
    DlgEdit *edt=(DlgEdit *)Item[Pos]->ObjPtr;
    if(edt)
    {
      if(Selected)
        edt->Select(0,edt->GetLength());
      else
        edt->Select(-1,0);
      //_SVS(SysLog(L"Selected=%d edt->GetLength()=%d",Selected,edt->GetLength()));
    }
  }
}

int Dialog::SetAutomation(WORD IDParent,WORD id,
                             FarDialogItemFlags UncheckedSet,FarDialogItemFlags UncheckedSkip,
                             FarDialogItemFlags CheckedSet,FarDialogItemFlags CheckedSkip,
                             FarDialogItemFlags Checked3Set,FarDialogItemFlags Checked3Skip)
{
  CriticalSectionLock Lock(CS);

  int Ret=FALSE;
  if(IDParent < ItemCount && (Item[IDParent]->Flags&DIF_AUTOMATION) &&
     id < ItemCount && IDParent != id) // Сами себя не юзаем!
  {
    DialogItemAutomation *Auto;
    int AutoCount=Item[IDParent]->AutoCount;
    if((Auto=(DialogItemAutomation*)xf_realloc(Item[IDParent]->AutoPtr,sizeof(DialogItemAutomation)*(AutoCount+1))) != NULL)
    {
      Item[IDParent]->AutoPtr=Auto;
      Auto=Item[IDParent]->AutoPtr+AutoCount;
      Auto->ID=id;
      Auto->Flags[0][0]=UncheckedSet;
      Auto->Flags[0][1]=UncheckedSkip;
      Auto->Flags[1][0]=CheckedSet;
      Auto->Flags[1][1]=CheckedSkip;
      Auto->Flags[2][0]=Checked3Set;
      Auto->Flags[2][1]=Checked3Skip;
      Item[IDParent]->AutoCount++;
      Ret=TRUE;
    }
  }
  return Ret;
}

//////////////////////////////////////////////////////////////////////////
/*
   AutoComplite: Поиск входжение подстроки в истории
*/
int Dialog::FindInEditForAC(int TypeFind,const wchar_t *HistoryName, string &strFindStr)
{
	CriticalSectionLock Lock(CS);

	if (HistoryName==NULL)
		return FALSE;

	int I, LenFindStr=(int)strFindStr.GetLength ();

	if (!TypeFind)
	{
		string strRegKey=fmtSavedDialogHistory;
		strRegKey+=HistoryName;

		History DlgHist(HISTORYTYPE_DIALOG, Opt.DialogsHistoryCount, strRegKey, &Opt.Dialogs.EditHistory, false);
		DlgHist.ReadHistory(true);

		if (!DlgHist.GetSimilar(strFindStr,-1,true))
			return FALSE;
	}
	else
	{
		FarListItem *ListItems=((FarList *)HistoryName)->Items;
		int Count=((FarList *)HistoryName)->ItemsNumber;

		for (I=0; I < Count ;I++)
		{
			if (!StrCmpNI (ListItems[I].Text, strFindStr, LenFindStr) && StrCmp(ListItems[I].Text, strFindStr)!=0)
				break;
		}

		if (I  == Count)
			return FALSE;

		strFindStr += &ListItems[I].Text[LenFindStr];
	}

	return TRUE;
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
  int EditX1,EditY1,EditX2,EditY2;
  int I,Dest, OriginalPos;
  unsigned CurFocusPos=FocusPos;

  //if((Str=(char*)xf_malloc(MaxLen)) != NULL)
  {
    EditLine->GetPosition(EditX1,EditY1,EditX2,EditY2);
    if (EditX2-EditX1<20)
      EditX2=EditX1+20;
		SetDropDownOpened(TRUE); // Установим флаг "открытия" комбобокса.
		SetComboBoxPos();
    // Перед отрисовкой спросим об изменении цветовых атрибутов
    BYTE RealColors[VMENU_COLOR_COUNT];
    FarListColors ListColors={0};
    ListColors.ColorCount=VMENU_COLOR_COUNT;
    ListColors.Colors=RealColors;
    ComboBox->SetColors(NULL);
    ComboBox->GetColors(&ListColors);
    if(DlgProc((HANDLE)this,DN_CTLCOLORDLGLIST,CurItem->ID,(LONG_PTR)&ListColors))
      ComboBox->SetColors(&ListColors);

    // Выставим то, что есть в строке ввода!
    // if(EditLine->GetDropDownBox()) //???
    EditLine->GetString(strStr);
    if(CurItem->Flags & (DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND))
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

      if(CurItem->IFlags.Check(DLGIIF_COMBOBOXEVENTKEY) && ReadRec.EventType == KEY_EVENT)
      {
        if(DlgProc((HANDLE)this,DN_KEY,FocusPos,Key))
          continue;
      }
      else if(CurItem->IFlags.Check(DLGIIF_COMBOBOXEVENTMOUSE) && ReadRec.EventType == MOUSE_EVENT)
        if(!DlgProc((HANDLE)this,DN_MOUSEEVENT,0,(LONG_PTR)&ReadRec.Event.MouseEvent))
          continue;

      // здесь можно добавить что-то свое, например,
      I=ComboBox->GetSelectPos();
      if (Key==KEY_TAB) // Tab в списке - аналог Enter
      {
        ComboBox->ProcessKey(KEY_ENTER);
        continue; //??
      }
      if(I != Dest)
      {
        if(!DlgProc((HANDLE)this,DN_LISTCHANGE,CurFocusPos,I))
          ComboBox->SetSelectPos(Dest,Dest<I?-1:1); //????
        else
          Dest=I;

#if 0
        // во время навигации по DropDown листу - отобразим ЭТО дело в
        // связанной строке
        // ВНИМАНИЕ!!!
        //  Очень медленная реакция!
        if(EditLine->GetDropDownBox())
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

    if(Dest == -1)
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
    if(CurItem->Flags & (DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND))
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

  if(!EditLine)
    return FALSE;

	string strStr;
	int ret=0;

	string strRegKey=fmtSavedDialogHistory;
	strRegKey+=HistoryName;

	History DlgHist(HISTORYTYPE_DIALOG, Opt.DialogsHistoryCount, strRegKey, &Opt.Dialogs.EditHistory, false);
	DlgHist.ReadHistory();
	DlgHist.ResetPosition();

  {
    // создание пустого вертикального меню
    VMenu HistoryMenu(L"",NULL,0,Opt.Dialogs.CBoxMaxHeight,VMENU_ALWAYSSCROLLBAR|VMENU_COMBOBOX|VMENU_NOTCHANGE);
    HistoryMenu.SetFlags(VMENU_SHOWAMPERSAND);
    HistoryMenu.SetBoxType(SHORT_SINGLE_BOX);

    SetDropDownOpened(TRUE); // Установим флаг "открытия" комбобокса.

    // запомним (для прорисовки)
    CurItem->ListPtr=&HistoryMenu;

    ret = DlgHist.Select(HistoryMenu, Opt.Dialogs.CBoxMaxHeight, this, strStr);

    // забудим (не нужен)
    CurItem->ListPtr=NULL;

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
int Dialog::AddToEditHistory(const wchar_t *AddStr,const wchar_t *HistoryName)
{
  CriticalSectionLock Lock(CS);

  if (*AddStr==0)
  {
    return FALSE;
  }

	string strRegKey=fmtSavedDialogHistory;
	strRegKey+=HistoryName;

	History DlgHist(HISTORYTYPE_DIALOG, Opt.DialogsHistoryCount, strRegKey, &Opt.Dialogs.EditHistory, false);
	DlgHist.ReadHistory();
	DlgHist.AddToHistory(AddStr);

  return TRUE;
}

int Dialog::CheckHighlights(WORD CheckSymbol,int StartPos)
{
	CriticalSectionLock Lock(CS);

	int Type, I;
	DWORD Flags;

	if (StartPos < 0)
		StartPos=0;

	for ( I=StartPos; I < (int)ItemCount; I++ )
	{
		Type=Item[I]->Type;
		Flags=Item[I]->Flags;

		if ((!IsEdit(Type) || (Type == DI_COMBOBOX && (Flags&DIF_DROPDOWNLIST))) && (Flags & (DIF_SHOWAMPERSAND|DIF_DISABLE|DIF_HIDDEN))==0)
		{
			const wchar_t *ChPtr=wcschr(Item[I]->strData,L'&');
			if (ChPtr)
			{
				WORD Ch=ChPtr[1];
				if(Ch && Upper(CheckSymbol) == Upper(Ch))
					return I;
			}
			else if ( !CheckSymbol )
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

  int Type;
  DWORD Flags;

  for (unsigned I=0;I<ItemCount;I++)
  {
    Type=Item[I]->Type;
    Flags=Item[I]->Flags;

    if ((!IsEdit(Type) || (Type == DI_COMBOBOX && (Flags&DIF_DROPDOWNLIST))) &&
        (Flags & (DIF_SHOWAMPERSAND|DIF_DISABLE|DIF_HIDDEN))==0)
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
          if(!DlgProc((HANDLE)this,DN_HOTKEY,I,Key))
            break; // сказали не продолжать обработку...
          // ... если предыдущий контрол задизаблен или невидим, тогда выходим.
          if ((Item[I-1]->Flags&(DIF_DISABLE|DIF_HIDDEN)) != 0) // и не задисаблен
             break;
          I=ChangeFocus(I,-1,FALSE);
          DisableSelect=TRUE;
        }
        else if (Item[I]->Type==DI_TEXT      || Item[I]->Type==DI_VTEXT ||
                 Item[I]->Type==DI_SINGLEBOX || Item[I]->Type==DI_DOUBLEBOX)
        {
          if(I+1 < ItemCount) // ...и следующий контрол
          {
            // Сначала сообщим о случившемся факте процедуре обработки диалога, а потом...
            if(!DlgProc((HANDLE)this,DN_HOTKEY,I,Key))
              break; // сказали не продолжать обработку...
            // ... если следующий контрол задизаблен или невидим, тогда выходим.
            if ((Item[I+1]->Flags&(DIF_DISABLE|DIF_HIDDEN)) != 0) // и не задисаблен
              break;
            I=ChangeFocus(I,1,FALSE);
            DisableSelect=TRUE;
          }
        }
        // Сообщим о случивщемся факте процедуре обработки диалога
        if(!DlgProc((HANDLE)this,DN_HOTKEY,I,Key))
          break; // сказали не продолжать обработку...
        ChangeFocus2(FocusPos,I); //??
        if(FocusPos != I)
        {
          ShowDialog(FocusPos);
          ShowDialog(I);
        }

        if ((Item[I]->Type==DI_CHECKBOX || Item[I]->Type==DI_RADIOBUTTON) &&
            (!DisableSelect || (Item[I]->Flags & DIF_MOVESELECT)))
        {
          Do_ProcessSpace();
          return(TRUE);
        }
        else if (Item[I]->Type==DI_BUTTON)
        {
          ProcessKey(KEY_ENTER);
          return(TRUE);
        }
        // при ComboBox`е - "вываливаем" последний //????
        else if (Item[I]->Type==DI_COMBOBOX)
        {
          ProcessOpenComboBox(Item[I]->Type,Item[I],I);
          //ProcessKey(KEY_CTRLDOWN);
          return(TRUE);
        }
        return(TRUE);
      }
  }
  return(FALSE);
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

  if(!DialogMode.Check(DMODE_CREATEOBJECTS))
    return;

  ScreenObject *DialogScrObject;
  for (unsigned I=0; I < ItemCount; I++)
  {
    CurItem=Item[I];
    int Type=CurItem->Type;
    if ((CurItem->ObjPtr  && IsEdit(Type)) ||
        (CurItem->ListPtr && Type == DI_LISTBOX))
    {
       if(Type == DI_LISTBOX)
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
void Dialog::SetDialogData(LONG_PTR NewDataDialog)
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

	TaskBarError *TBE=DialogMode.Check(DMODE_WARNINGSTYLE)?new TaskBarError:NULL;

  if(ExitCode == -1)
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
    FrameManager->ExecuteModal (this);
    save += (clock() - btm);
    if (InterlockedDecrement(&in_dialog) == -1)
      WaitUserTime = save;
  }

  if ( pSaveItemEx )
    for (unsigned i = 0; i < ItemCount; i++)
        DialogItemExToDialogItemEx (Item[i], &pSaveItemEx[i]);

	if(TBE)
	{
		delete TBE;
	}
}

void Dialog::CloseDialog()
{
  CriticalSectionLock Lock(CS);

  GetDialogObjectsData();
  if (DlgProc ((HANDLE)this,DN_CLOSE,ExitCode,0))
  {
    DialogMode.Set(DMODE_ENDLOOP);
    Hide();

    if(DialogMode.Check(DMODE_BEGINLOOP) && (DialogMode.Check(DMODE_MSGINTERNAL) || FrameManager->ManagerStarted()))
    {
      DialogMode.Clear(DMODE_BEGINLOOP);
      FrameManager->DeleteFrame (this);
    }

    _DIALOG(CleverSysLog CL(L"Close Dialog"));
  }
}


/* $ 17.05.2001 DJ
   установка help topic'а и прочие радости, временно перетащенные сюда
   из Modal
*/
void Dialog::SetHelp (const wchar_t *Topic)
{
  CriticalSectionLock Lock(CS);

  if (HelpTopic)
    delete[] HelpTopic;
  HelpTopic=NULL;

  if(Topic && *Topic)
  {
    HelpTopic = new wchar_t [wcslen (Topic)+1];
    if(HelpTopic)
      wcscpy (HelpTopic, Topic);
  }
}

void Dialog::ShowHelp()
{
  CriticalSectionLock Lock(CS);

  if (HelpTopic && *HelpTopic)
  {
    Help Hlp (HelpTopic);
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
int Dialog::GetTypeAndName (string &strType, string &strName)
{
  CriticalSectionLock Lock(CS);

  strType = MSG(MDialogType);

  strName = L"";

  const wchar_t *lpwszTitle = GetDialogTitle();

  if ( lpwszTitle )
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

  COORD c;
  DialogMode.Set(DMODE_RESIZED);

  if(IsVisible())
    Hide();

  // коррекция относительного положения диалога (чтобы не центрировать :-)
  c.X=ScrX+1; c.Y=ScrY+1;
	SendDlgMessage((HANDLE)this,DN_RESIZECONSOLE,0,(LONG_PTR)&c);

  // !!!!!!!!!!! здесь нужно правильно вычислить положение !!!!!!!!!!!
  //c.X=((X1*100/PrevScrX)*ScrX)/100;
  //c.Y=((Y1*100/PrevScrY)*ScrY)/100;
  // !!!!!!!!!!! здесь нужно правильно вычислить положение !!!!!!!!!!!

  c.X=c.Y=-1;
	SendDlgMessage((HANDLE)this,DM_MOVEDIALOG,TRUE,(LONG_PTR)&c);
	Dialog::SetComboBoxPos();
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
//		SendDlgMessage((HANDLE)this,DM_KILLSAVESCREEN,0,0);
//  }
//};

LONG_PTR WINAPI Dialog::DlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  if(DialogMode.Check(DMODE_ENDLOOP))
    return 0;

	LONG_PTR Result;
	FarDialogEvent de={hDlg,Msg,Param1,Param2,0};
	if(CtrlObject->Plugins.ProcessDialogEvent(DE_DLGPROCINIT,&de))
			return de.Result;
	Result=RealDlgProc(hDlg,Msg,Param1,Param2);
	de.Result=Result;
	if(CtrlObject->Plugins.ProcessDialogEvent(DE_DLGPROCEND,&de))
		return de.Result;
	return Result;
}

//////////////////////////////////////////////////////////////////////////
/* $ 28.07.2000 SVS
   функция обработки диалога (по умолчанию)
   Вот именно эта функция и является последним рубежом обработки диалога.
   Т.е. здесь должна быть ВСЯ обработка ВСЕХ сообщений!!!
*/
LONG_PTR WINAPI DefDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  _DIALOG(CleverSysLog CL(L"Dialog.DefDlgProc()"));
  _DIALOG(SysLog(L"hDlg=%p, Msg=%s, Param1=%d (0x%08X), Param2=%d (0x%08X)",hDlg,_DLGMSG_ToName(Msg),Param1,Param1,Param2,Param2));

	if(!hDlg || hDlg==INVALID_HANDLE_VALUE)
		return 0;

	FarDialogEvent de={hDlg,Msg,Param1,Param2,0};
	if(CtrlObject->Plugins.ProcessDialogEvent(DE_DEFDLGPROCINIT,&de))
	{
		return de.Result;
	}

  Dialog* Dlg=(Dialog*)hDlg;

  CriticalSectionLock Lock(Dlg->CS);

	DialogItemEx *CurItem=NULL;
  int Type=0;

  switch(Msg)
  {
    case DN_INITDIALOG:
      return FALSE; // изменений не было!

    case DM_CLOSE:
      return TRUE;  // согласен с закрытием

    case DN_KILLFOCUS:
      return -1;    // "Согласен с потерей фокуса"

    case DN_GOTFOCUS:
      return 0;     // always 0

    case DN_HELP:
      return Param2; // что передали, то и...

    case DN_DRAGGED:
      return TRUE; // согласен с перемещалкой.

    case DN_DRAWDIALOGDONE:
    {
      if(Param1 == 1)  // Нужно отрисовать "салазки"?
      {
        /* $ 03.08.2000 tran
           вывод текста в углу может приводить к ошибкам изображения
           1) когда диалог перемещается в угол
           2) когда диалог перемещается из угла
           сделал вывод красных палочек по углам */
        Text(Dlg->X1,Dlg->Y1,0xCE,L"\\");
        Text(Dlg->X1,Dlg->Y2,0xCE,L"/");
        Text(Dlg->X2,Dlg->Y1,0xCE,L"/");
        Text(Dlg->X2,Dlg->Y2,0xCE,L"\\");
      }
      return TRUE;
    }

    case DN_DRAWDIALOG:
    {
      return TRUE;
    }

    case DN_CTLCOLORDIALOG:
      return Param2;

    case DN_CTLCOLORDLGITEM:
      return Param2;

    case DN_CTLCOLORDLGLIST:
      return FALSE;

    case DN_ENTERIDLE:
      return 0;     // always 0

		case DM_GETDIALOGINFO:
			{
				bool Result=false;
				if(Param2)
				{
					if(Dlg->IdExist)
					{
						DialogInfo *di=reinterpret_cast<DialogInfo*>(Param2);
						if(static_cast<size_t>(di->StructSize)>=offsetof(DialogInfo,Id)+sizeof(di->Id))
						{
							di->Id=Dlg->Id;
							Result=true;
						}
					}
				}
				return Result;
			}
  }

  // предварительно проверим...
  if((unsigned)Param1 >= Dlg->ItemCount && Dlg->Item)
    return 0;

  if (Param1>=0)
  {
    CurItem=Dlg->Item[Param1];
    Type=CurItem->Type;
  }

  switch(Msg)
  {
    case DN_MOUSECLICK:
      return FALSE;

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

    case DN_KEY:
      return FALSE;

    case DN_MOUSEEVENT:
      return TRUE;

    case DM_GETSELECTION: // Msg=DM_GETSELECTION, Param1=ID, Param2=*EditorSelect
      return FALSE;

    case DM_SETSELECTION:
      return FALSE;
  }

  return 0;
}

LONG_PTR Dialog::CallDlgProc (int nMsg, int nParam1, LONG_PTR nParam2)
{
    CriticalSectionLock Lock (CS);

    return Dialog::DlgProc ((HANDLE)this, nMsg, nParam1, nParam2);
}

//////////////////////////////////////////////////////////////////////////
/* $ 28.07.2000 SVS
   Посылка сообщения диалогу
   Некоторые сообщения эта функция обрабатывает сама, не передавая управление
   обработчику диалога.
*/
LONG_PTR WINAPI SendDlgMessage(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  Dialog* Dlg=(Dialog*)hDlg;

  CriticalSectionLock Lock (Dlg->CS);

  unsigned I;

  _DIALOG(CleverSysLog CL(L"Dialog.SendDlgMessage()"));
  _DIALOG(SysLog(L"hDlg=%p, Msg=%s, Param1=%d (0x%08X), Param2=%d (0x%08X)",hDlg,_DLGMSG_ToName(Msg),Param1,Param1,Param2,Param2));

  if ( !Dlg )
    return 0;

  // Сообщения, касаемые только диалога и не затрагивающие элементы
  switch(Msg)
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
      if(Param1>0)   // абсолютно?
      {
        Dlg->X1=((COORD*)Param2)->X;
        Dlg->Y1=((COORD*)Param2)->Y;
        Dlg->X2=W1;
        Dlg->Y2=H1;
        Dlg->CheckDialogCoord();
      }
      else if(Param1 == 0)   // значит относительно
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
        if(Dlg->X1+W1>ScrX)
          Dlg->X1=ScrX-W1+1;
        if(Dlg->Y1+H1>ScrY+1)
          Dlg->Y1=ScrY-H1+2;

        if (W1<OldW1 || H1<OldH1)
        {
          Dlg->DialogMode.Set(DMODE_DRAWING);
          DialogItemEx *Item;
          SMALL_RECT Rect;
          for (I=0; I<Dlg->ItemCount; I++)
          {
            Item=Dlg->Item[I];
            if(Item->Flags&DIF_HIDDEN)
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
      if(Dlg->X1<0)
        Dlg->X1=0;
      if(Dlg->Y1<0)
        Dlg->Y1=0;

      /* $ 11.10.2001 KM
        - Ещё одно уточнение при ресайзинге, с учётом предполагаемого
          выхода краёв диалога за границу экрана.
      */
      if(Dlg->X1+W1>ScrX)
        Dlg->X1=ScrX-W1+1;
      if(Dlg->Y1+H1>ScrY+1)
        Dlg->Y1=ScrY-H1+2;

      Dlg->X2=Dlg->X1+W1-1;
      Dlg->Y2=Dlg->Y1+H1-1;

      Dlg->CheckDialogCoord();

      if(Param1 < 0)   // размер?
      {
        ((COORD*)Param2)->X=Dlg->X2-Dlg->X1+1;
        ((COORD*)Param2)->Y=Dlg->Y2-Dlg->Y1+1;
      }
      else
      {
        ((COORD*)Param2)->X=Dlg->X1;
        ((COORD*)Param2)->Y=Dlg->Y1;
      }

      I=Dlg->IsVisible();// && Dlg->DialogMode.Check(DMODE_INITOBJECTS);
      if(I) Dlg->Hide();
      // приняли.
      Dlg->AdjustEditPos(Dlg->X1-Dlg->OldX1,Dlg->Y1-Dlg->OldY1);
      if(I) Dlg->Show(); // только если диалог был виден

      return Param2;
    }


    /*****************************************************************/
    case DM_REDRAW:
    {
      if(Dlg->DialogMode.Check(DMODE_INITOBJECTS))
        Dlg->Show();
      return 0;
    }

    /*****************************************************************/
    case DM_ENABLEREDRAW:
    {
      int Prev=Dlg->IsEnableRedraw;

      if(Param1 == TRUE)
        Dlg->IsEnableRedraw++;
      else if(Param1 == FALSE)
        Dlg->IsEnableRedraw--;

      //Edit::DisableEditOut(!Dlg->IsEnableRedraw?FALSE:TRUE);

      if(!Dlg->IsEnableRedraw && Prev != Dlg->IsEnableRedraw)
        if(Dlg->DialogMode.Check(DMODE_INITOBJECTS))
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
        if(Param1)
        {
          /* $ 20.04.2002 KM
            Залочим прорисовку при прятании диалога, в противном
            случае ОТКУДА менеджер узнает, что отрисовывать
            объект нельзя!
          */
          if(!Dlg->IsVisible())
          {
            Dlg->Unlock();
            Dlg->Show();
          }
        }
        else
        {
          if(Dlg->IsVisible())
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
      LONG_PTR PrewDataDialog=Dlg->DataDialog;
      Dlg->DataDialog=Param2;
      return PrewDataDialog;
    }

    /*****************************************************************/
    case DM_GETDLGDATA:
    {
      return Dlg->DataDialog;
    }


    /*****************************************************************/
    case DM_KEY:
    {
      int *KeyArray=(int*)Param2;
      Dlg->DialogMode.Set(DMODE_KEY);
      for(I=0; I < (unsigned)Param1; ++I)
        Dlg->ProcessKey(KeyArray[I]);
      Dlg->DialogMode.Clear(DMODE_KEY);
      return 0;
    }


    /*****************************************************************/
    case DM_CLOSE:
    {
      if(Param1 == -1)
        Dlg->ExitCode=Dlg->FocusPos;
      else
        Dlg->ExitCode=Param1;
      Dlg->CloseDialog();
      return TRUE;  // согласен с закрытием
    }


    /*****************************************************************/
    case DM_GETDLGRECT:
    {
			if(Param2)
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
      if(Param1 == -1)
        return IsProcessAssignMacroKey;
      BOOL OldIsProcessAssignMacroKey=IsProcessAssignMacroKey;
      IsProcessAssignMacroKey=Param1;
      return OldIsProcessAssignMacroKey;
    }

    /*****************************************************************/
    case DM_SETMOUSEEVENTNOTIFY: // Param1 = 1 on, 0 off, -1 - get
    {
      int State=Dlg->DialogMode.Check(DMODE_MOUSEEVENT)?TRUE:FALSE;
      if(Param1 != -1)
      {
        if(!Param1)
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

  }

  /*****************************************************************/
  if(Msg >= DM_USER)
  {
    return Dlg->CallDlgProc(Msg,Param1,Param2);
  }

  /*****************************************************************/
	DialogItemEx *CurItem=NULL;
  int Type=0;
  size_t Len=0;
  // предварительно проверим...
  /* $ 09.12.2001 DJ
     для DM_USER проверять _не_надо_!
  */
  if((unsigned)Param1 >= Dlg->ItemCount || !Dlg->Item)
    return 0;

//  CurItem=&Dlg->Item[Param1];
  CurItem=Dlg->Item[Param1];
  Type=CurItem->Type;

	const wchar_t *Ptr= CurItem->strData;
	if(IsEdit(Type) && CurItem->ObjPtr)
		Ptr=const_cast <const wchar_t *>(((DlgEdit *)(CurItem->ObjPtr))->GetStringAddr());


  switch(Msg)
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
		case DM_LISTSETTITLES: // Param1=ID Param2=FarListTitles: TitleLen=strlen(Title), BottomLen=strlen(Bottom)
		case DM_LISTGETTITLES: // Param1=ID Param2=FarListTitles: TitleLen=strlen(Title), BottomLen=strlen(Bottom)
    case DM_LISTGETDATASIZE: // Param1=ID Param2=Index
    case DM_LISTSETMOUSEREACTION: // Param1=ID Param2=FARLISTMOUSEREACTIONTYPE Ret=OldSets
    case DM_SETCOMBOBOXEVENT: // Param1=ID Param2=FARCOMBOBOXEVENTTYPE Ret=OldSets
    case DM_GETCOMBOBOXEVENT: // Param1=ID Param2=0 Ret=Sets
    {
      if(Type==DI_LISTBOX || Type==DI_COMBOBOX)
      {
        VMenu *ListBox=CurItem->ListPtr;
        if(ListBox)
        {
          int Ret=TRUE;
          switch(Msg)
          {
            case DM_LISTINFO:// Param1=ID Param2=FarListInfo
            {
							return ListBox->GetVMenuInfo((FarListInfo*)Param2);
            }

            case DM_LISTSORT: // Param1=ID Param=Direct {0|1}
            {
              ListBox->SortItems((int)Param2);
              /* $ 23.02.2002 DJ
                 корректировка позиции нужна, чтобы не было двух выделенных элементов
              */
              ListBox->AdjustSelectPos();
              break;
            }

            case DM_LISTFINDSTRING: // Param1=ID Param2=FarListFind
            {
							FarListFind* lf=reinterpret_cast<FarListFind*>(Param2);
							return ListBox->FindItem(lf->StartIndex,lf->Pattern,lf->Flags);
            }

            case DM_LISTADDSTR: // Param1=ID Param2=String
            {
              Ret=ListBox->AddItem((wchar_t*)Param2);
              ListBox->AdjustSelectPos();
              break;
            }

            case DM_LISTADD: // Param1=ID Param2=FarList: ItemsNumber=Count, Items=Src
            {
							FarList *ListItems=(FarList *)Param2;
              if(!ListItems)
                return FALSE;
              Ret=ListBox->AddItem(ListItems);
              ListBox->AdjustSelectPos();
              break;
            }

            case DM_LISTDELETE: // Param1=ID Param2=FarListDelete: StartIndex=BeginIndex, Count=количество (<=0 - все!)
            {
              int Count;
							FarListDelete *ListItems=(FarListDelete *)Param2;
              if(!ListItems || (Count=ListItems->Count) <= 0)
                ListBox->DeleteItems();
              else
                ListBox->DeleteItem(ListItems->StartIndex,Count);
              break;
            }

            case DM_LISTINSERT: // Param1=ID Param2=FarListInsert
            {
							if((Ret=ListBox->InsertItem((FarListInsert *)Param2)) == -1)
                return -1;
              ListBox->AdjustSelectPos();
              break;
            }

            case DM_LISTUPDATE: // Param1=ID Param2=FarListUpdate: Index=Index, Items=Src
            {
							if(Param2 && ListBox->UpdateItem((FarListUpdate *)Param2))
                break;
              return FALSE;
            }

            case DM_LISTGETITEM: // Param1=ID Param2=FarListGetItem: ItemsNumber=Index, Items=Dest
            {
							FarListGetItem *ListItems=(FarListGetItem *)Param2;
              if(!ListItems)
                return FALSE;
              MenuItemEx *ListMenuItem;
              if((ListMenuItem=ListBox->GetItemPtr(ListItems->ItemIndex)) != NULL)
              {
                //ListItems->ItemIndex=1;
								FarListItem *Item=&ListItems->Item;
								memset(Item,0,sizeof(FarListItem));
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
              if(Param2 < ListBox->GetItemCount())
                return (LONG_PTR)ListBox->GetUserData(NULL,0,(int)Param2);
              return 0;
            }

            case DM_LISTGETDATASIZE: // Param1=ID Param2=Index
            {
              if(Param2 < ListBox->GetItemCount())
                return ListBox->GetUserDataSize((int)Param2);
              return 0;
            }

						case DM_LISTSETDATA: // Param1=ID Param2=FarListItemData
            {
							FarListItemData *ListItems=(FarListItemData *)Param2;
              if(ListItems &&
                 ListItems->Index < ListBox->GetItemCount())
              {
                Ret=ListBox->SetUserData(ListItems->Data,
                                            ListItems->DataSize,
                                            ListItems->Index);
                if(!Ret && ListBox->GetUserData(NULL,0,ListItems->Index))
                  Ret=sizeof(DWORD);
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
              if(!ListItems)
                return FALSE;
              ListBox->DeleteItems();
              Ret=ListBox->AddItem(ListItems);
              ListBox->AdjustSelectPos();
              break;
            }
            //case DM_LISTINS: // Param1=ID Param2=FarList: ItemsNumber=Index, Items=Dest

						case DM_LISTSETTITLES: // Param1=ID Param2=FarListTitles
            {
							FarListTitles *ListTitle=(FarListTitles *)Param2;
							ListBox->SetTitle(ListTitle->Title);
							ListBox->SetBottomTitle(ListTitle->Bottom);
							break;   //return TRUE;
            }

						case DM_LISTGETTITLES: // Param1=ID Param2=FarListTitles
						{
							if(Param2)
							{
								FarListTitles *ListTitle=(FarListTitles *)Param2;
								string strTitle,strBottomTitle;
								ListBox->GetTitle (strTitle);
								ListBox->GetBottomTitle (strBottomTitle);
								if(!strTitle.IsEmpty()||!strBottomTitle.IsEmpty())
								{
									if(ListTitle->Title&&ListTitle->TitleLen)
										xwcsncpy((wchar_t*)ListTitle->Title,strTitle,ListTitle->TitleLen-1);
									else
										ListTitle->TitleLen=(int)strTitle.GetLength()+1;
									if(ListTitle->Bottom&&ListTitle->BottomLen)
										xwcsncpy((wchar_t*)ListTitle->Bottom,strBottomTitle,ListTitle->BottomLen-1);
									else
										ListTitle->BottomLen=(int)strBottomTitle.GetLength()+1;
									return TRUE;
								}
							}
							return FALSE;
						}

            case DM_LISTGETCURPOS: // Param1=ID Param2=FarListPos
            {
							return Param2?ListBox->GetSelectPos((FarListPos *)Param2):ListBox->GetSelectPos();
            }

            case DM_LISTSETCURPOS: // Param1=ID Param2=FarListPos Ret: RealPos
            {
              /* 26.06.2001 KM Подадим перед изменением позиции об этом сообщение */
              int CurListPos=ListBox->GetSelectPos();
							Ret=ListBox->SetSelectPos((FarListPos *)Param2);
              if(Ret!=CurListPos)
                if(!Dlg->CallDlgProc(DN_LISTCHANGE,Param1,Ret))
                  Ret=ListBox->SetSelectPos(CurListPos,1);
              break; // т.к. нужно перерисовать!
            }

            case DM_LISTSETMOUSEREACTION: // Param1=ID Param2=FARLISTMOUSEREACTIONTYPE Ret=OldSets
            {
              int OldSets=CurItem->IFlags.Flags;
              if(Param2 == LMRT_ONLYFOCUS)
              {
                CurItem->IFlags.Clear(DLGIIF_LISTREACTIONNOFOCUS);
                CurItem->IFlags.Set(DLGIIF_LISTREACTIONFOCUS);
              }
              else if(Param2 == LMRT_NEVER)
              {
                CurItem->IFlags.Clear(DLGIIF_LISTREACTIONNOFOCUS|DLGIIF_LISTREACTIONFOCUS);
                //ListBox->ClearFlags(VMENU_MOUSEREACTION);
              }
              else
              {
                CurItem->IFlags.Set(DLGIIF_LISTREACTIONNOFOCUS|DLGIIF_LISTREACTIONFOCUS);
                //ListBox->SetFlags(VMENU_MOUSEREACTION);
              }

              if((OldSets&(DLGIIF_LISTREACTIONNOFOCUS|DLGIIF_LISTREACTIONFOCUS)) == (DLGIIF_LISTREACTIONNOFOCUS|DLGIIF_LISTREACTIONFOCUS))
                OldSets=LMRT_ALWAYS;
              else if((OldSets&(DLGIIF_LISTREACTIONNOFOCUS|DLGIIF_LISTREACTIONFOCUS)) == 0)
                OldSets=LMRT_NEVER;
              else
                OldSets=LMRT_ONLYFOCUS;
              return OldSets;
            }
            case DM_GETCOMBOBOXEVENT: // Param1=ID Param2=0 Ret=Sets
            {
              return (CurItem->IFlags.Check(DLGIIF_COMBOBOXEVENTKEY)?CBET_KEY:0)|(CurItem->IFlags.Check(DLGIIF_COMBOBOXEVENTMOUSE)?CBET_MOUSE:0);
            }

            case DM_SETCOMBOBOXEVENT: // Param1=ID Param2=FARCOMBOBOXEVENTTYPE Ret=OldSets
            {
              int OldSets=CurItem->IFlags.Flags;
              CurItem->IFlags.Clear(DLGIIF_COMBOBOXEVENTKEY|DLGIIF_COMBOBOXEVENTMOUSE);
              if(Param2&CBET_KEY)
                CurItem->IFlags.Set(DLGIIF_COMBOBOXEVENTKEY);
              if(Param2&CBET_MOUSE)
                CurItem->IFlags.Set(DLGIIF_COMBOBOXEVENTMOUSE);
              return OldSets;
            }
          }
          // уточнение для DI_COMBOBOX - здесь еще и DlgEdit нужно корректно заполнить
          if(!CurItem->IFlags.Check(DLGIIF_COMBOBOXNOREDRAWEDIT) && Type==DI_COMBOBOX && CurItem->ObjPtr)
          {
            MenuItemEx *ListMenuItem;
            if((ListMenuItem=ListBox->GetItemPtr(ListBox->GetSelectPos())) != NULL)
            {
              if(CurItem->Flags & (DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND))
                ((DlgEdit *)(CurItem->ObjPtr))->SetHiString(ListMenuItem->strName);
              else
                ((DlgEdit *)(CurItem->ObjPtr))->SetString(ListMenuItem->strName);

              ((DlgEdit *)(CurItem->ObjPtr))->Select(-1,-1); // снимаем выделение
            }
          }

          if(Dlg->DialogMode.Check(DMODE_SHOW) && ListBox->UpdateRequired())
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
      if(Type==DI_EDIT || Type==DI_FIXEDIT)
      {
        if(Param2 && *(const wchar_t *)Param2)
        {
          CurItem->Flags|=DIF_HISTORY;
          CurItem->History=(const wchar_t *)Param2;
          if(Type==DI_EDIT && (CurItem->Flags&DIF_USELASTHISTORY))
          {
            Dlg->ProcessLastHistory (CurItem, Param1);
          }
        }
        else
        {
          CurItem->Flags&=~DIF_HISTORY;
          CurItem->History=NULL;
        }
        if(Dlg->DialogMode.Check(DMODE_SHOW))
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
      if(Param2 &&
         (Type==DI_EDIT || Type==DI_FIXEDIT) &&
         (CurItem->Flags & DIF_HISTORY))
      {
        return Dlg->AddToEditHistory((const wchar_t*)Param2,CurItem->History);
      }
      return FALSE;
    }

    /*****************************************************************/
    case DM_GETCURSORPOS:
    {
      if(!Param2)
        return FALSE;
      if (IsEdit(Type) && CurItem->ObjPtr)
      {
        ((COORD*)Param2)->X=((DlgEdit *)(CurItem->ObjPtr))->GetCurPos();
        ((COORD*)Param2)->Y=0;
        return TRUE;
      }
      else if(Type == DI_USERCONTROL && CurItem->UCData)
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
        Dlg->ShowDialog (Param1);
        return TRUE;
      }
      else if(Type == DI_USERCONTROL && CurItem->UCData)
      {
        // учтем, что координаты для этого элемента всегда относительные!
        //  и начинаются с 0,0
        COORD Coord=*(COORD*)Param2;
        Coord.X+=CurItem->X1;
        if(Coord.X > CurItem->X2)
          Coord.X=CurItem->X2;

        Coord.Y+=CurItem->Y1;
        if(Coord.Y > CurItem->Y2)
          Coord.Y=CurItem->Y2;

        // Запомним
        CurItem->UCData->CursorPos.X=Coord.X-CurItem->X1;
        CurItem->UCData->CursorPos.Y=Coord.Y-CurItem->Y1;
        // переместим если надо
        if(Dlg->DialogMode.Check(DMODE_SHOW) && Dlg->FocusPos == (unsigned)Param1)
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
			if(Param2 && IsEdit(Type))
      {
        if(Type == DI_MEMOEDIT)
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
			if(Param2 && IsEdit(Type))
      {
        if(Type == DI_MEMOEDIT)
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
        int Visible,Size;
        ((DlgEdit *)(CurItem->ObjPtr))->GetCursorType(Visible,Size);
        return MAKELONG(Visible,Size);
      }
      else if(Type == DI_USERCONTROL && CurItem->UCData)
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
      int Visible=0,Size=0;
      if (IsEdit(Type) && CurItem->ObjPtr)
      {
        ((DlgEdit *)(CurItem->ObjPtr))->GetCursorType(Visible,Size);
        ((DlgEdit *)(CurItem->ObjPtr))->SetCursorType(LOWORD(Param2),HIWORD(Param2));
      }
      else if(Type == DI_USERCONTROL && CurItem->UCData)
      {
        Visible=CurItem->UCData->CursorVisible;
        Size=CurItem->UCData->CursorSize;

        CurItem->UCData->CursorVisible=LOWORD(Param2);
        CurItem->UCData->CursorSize=HIWORD(Param2);

        int CCX=CurItem->UCData->CursorPos.X;
        int CCY=CurItem->UCData->CursorPos.Y;
        if(Dlg->DialogMode.Check(DMODE_SHOW) &&
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
    	FarDialogItem Item;
			if (!ConvertItemEx(CVTITEM_TOPLUGIN,&Item,CurItem,1))
				return FALSE; // no memory TODO: may be needed diagnostic

	  CurItem->IFlags.Set(DLGIIF_EDITCHANGEPROCESSED);
      const wchar_t* original_PtrData=Item.PtrData;
      if((I=(int)Dlg->CallDlgProc(DN_EDITCHANGE,Param1,(LONG_PTR)&Item)) == TRUE)
      {
        if((Type == DI_LISTBOX || Type == DI_COMBOBOX) && CurItem->ListPtr)
          CurItem->ListPtr->ChangeFlags(VMENU_DISABLED,CurItem->Flags&DIF_DISABLE);
      }

			if (original_PtrData)
				xf_free((void*)original_PtrData);

	  CurItem->IFlags.Clear(DLGIIF_EDITCHANGEPROCESSED);
      return I;
    }

    /*****************************************************************/
    case DN_BTNCLICK:
    {
      LONG_PTR Ret=Dlg->CallDlgProc(Msg,Param1,Param2);
      if(Ret && (CurItem->Flags&DIF_AUTOMATION) && CurItem->AutoCount && CurItem->AutoPtr)
      {
        DialogItemAutomation* Auto=CurItem->AutoPtr;
        Param2%=3;
        for(I=0; I < CurItem->AutoCount; ++I, ++Auto)
        {
          DWORD NewFlags=Dlg->Item[Auto->ID]->Flags;
          Dlg->Item[Auto->ID]->Flags=(NewFlags&(~Auto->Flags[Param2][1]))|Auto->Flags[Param2][0];
          // здесь намеренно в обработчик не посылаются эвенты об изменении
          // состояния...
        }
      }
      return Ret;
    }

    /*****************************************************************/
    case DM_GETCHECK:
    {
      if(Type==DI_CHECKBOX || Type==DI_RADIOBUTTON)
        return CurItem->Selected;
      return 0;
    }

    /*****************************************************************/
    case DM_SET3STATE:
    {
      if(Type == DI_CHECKBOX)
      {
        int OldState=CurItem->Flags&DIF_3STATE?TRUE:FALSE;
        if(Param2)
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
      if(Type == DI_CHECKBOX)
      {
        int Selected=CurItem->Selected;

        if(Param2 == BSTATE_TOGGLE)
          Param2=++Selected;

        if(CurItem->Flags&DIF_3STATE)
          Param2%=3;
        else
          Param2&=1;
        CurItem->Selected=(int)Param2;

        if(Selected != (int)Param2 && Dlg->DialogMode.Check(DMODE_SHOW))
        {
          // автоматизация
          if((CurItem->Flags&DIF_AUTOMATION) && CurItem->AutoCount && CurItem->AutoPtr)
          {
            DialogItemAutomation* Auto=CurItem->AutoPtr;
            Param2%=3;
            for(I=0; I < CurItem->AutoCount; ++I, ++Auto)
            {
              DWORD NewFlags=Dlg->Item[Auto->ID]->Flags;
              Dlg->Item[Auto->ID]->Flags=(NewFlags&(~Auto->Flags[Param2][1]))|Auto->Flags[Param2][0];
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
      else if(Type == DI_RADIOBUTTON)
      {
        Param1=Dlg->ProcessRadioButton(Param1);
        if(Dlg->DialogMode.Check(DMODE_SHOW))
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
    	FarDialogItem Item;
			if(!ConvertItemEx(CVTITEM_TOPLUGIN,&Item,CurItem,1))
				return FALSE; // no memory TODO: may be needed diagnostic

      I=(int)Dlg->CallDlgProc(Msg,Param1,(LONG_PTR)&Item);

      if((Type == DI_LISTBOX || Type == DI_COMBOBOX) && CurItem->ListPtr)
        CurItem->ListPtr->ChangeFlags(VMENU_DISABLED,CurItem->Flags&DIF_DISABLE);

      if (Item.PtrData)
      	xf_free((wchar_t *)Item.PtrData);

      return I;
    }

    /*****************************************************************/
    case DM_SETFOCUS:
    {
      if(!CanGetFocus(Type))
        return FALSE;
      if(Dlg->FocusPos == (unsigned)Param1) // уже и так установлено все!
        return TRUE;
      if(Dlg->ChangeFocus2(Dlg->FocusPos,Param1) == (unsigned)Param1)
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
			return (LONG_PTR)Ptr;
		}

    /*****************************************************************/
    case DM_GETTEXTPTR:
      if(Param2)
      {
				FarDialogItemData IData={0,(wchar_t *)Param2};
				return SendDlgMessage(hDlg,DM_GETTEXT,Param1,(LONG_PTR)&IData);
      }

    /*****************************************************************/
    case DM_GETTEXT:
      if(Param2) // если здесь NULL, то это еще один способ получить размер
      {
        FarDialogItemData *did=(FarDialogItemData*)Param2;
        Len=0;
        switch(Type)
        {
          case DI_MEMOEDIT:
            break;
          case DI_COMBOBOX:
          case DI_EDIT:
          case DI_PSWEDIT:
          case DI_FIXEDIT:
            if(!CurItem->ObjPtr)
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
            if (!(CurItem->Flags & DIF_NOBRACKETS) && Type == DI_BUTTON)
            {
              Ptr+=2;
              Len-=4;
            }

            if(!did->PtrLength)
              did->PtrLength=Len;
            else if(Len > did->PtrLength)
              Len=did->PtrLength+1; // Прибавим 1, чтобы учесть нулевой байт.

            if(Len > 0 && did->PtrData)
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
    case DM_GETTEXTLENGTH:
    {
      switch(Type)
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
          if(CurItem->ObjPtr)
          {
            Len=((DlgEdit *)(CurItem->ObjPtr))->GetLength()+1;
            break;
          }

        case DI_LISTBOX:
        {
          Len=0;
          MenuItemEx *ListMenuItem;
          if((ListMenuItem=CurItem->ListPtr->GetItemPtr(-1)) != NULL)
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
      if(!Param2)
        return 0;

			FarDialogItemData IData={StrLength((wchar_t *)Param2),(wchar_t *)Param2};
			return SendDlgMessage(hDlg,DM_SETTEXT,Param1,(LONG_PTR)&IData);
    }

    /*****************************************************************/
    case DM_SETTEXT:
    {
      if(Param2)
      {
        int NeedInit=TRUE;
				FarDialogItemData *did=(FarDialogItemData*)Param2;
        switch(Type)
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
             Len = (int)CurItem->strData.GetLength();
            break;
          default:
            Len=0;
            break;
        }

        switch(Type)
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
            if(Dlg->DialogMode.Check(DMODE_SHOW))
            {
              SetFarTitle(Dlg->GetDialogTitle());
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
            if(CurItem->ObjPtr)
            {
              DlgEdit *EditLine=(DlgEdit *)(CurItem->ObjPtr);
              int ReadOnly=EditLine->GetReadOnly();
              EditLine->SetReadOnly(0);

              EditLine->SetString(CurItem->strData);

              EditLine->SetReadOnly(ReadOnly);
              if(Dlg->DialogMode.Check(DMODE_INITOBJECTS)) // не меняем клеар-флаг, пока не проиницализировались
                EditLine->SetClearFlag(0);
              EditLine->Select(-1,0); // снимаем выделение
              // ...оно уже снимается в DlgEdit::SetString()
            }
            break;

          case DI_LISTBOX: // меняет только текущий итем
          {
            VMenu *ListBox=CurItem->ListPtr;
            if(ListBox)
            {
							FarListUpdate LUpdate;
              LUpdate.Index=ListBox->GetSelectPos();
              MenuItemEx *ListMenuItem=ListBox->GetItemPtr(LUpdate.Index);
              if(ListMenuItem)
              {
                LUpdate.Item.Flags=ListMenuItem->Flags;
                LUpdate.Item.Text=Ptr;
								SendDlgMessage(hDlg,DM_LISTUPDATE,Param1,(LONG_PTR)&LUpdate);
              }
              break;
            }
            else
              return 0;
          }

          default:  // подразумеваем, что остались
            return 0;
        }
        if(NeedInit)
          Dlg->InitDialogObjects(Param1); // переинициализируем элементы диалога
        if(Dlg->DialogMode.Check(DMODE_SHOW)) // достаточно ли этого????!!!!
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
      if((Type==DI_EDIT || Type==DI_PSWEDIT ||
          (Type==DI_COMBOBOX && !(CurItem->Flags & DIF_DROPDOWNLIST))) &&
         CurItem->ObjPtr)
      {
        int MaxLen=((DlgEdit *)(CurItem->ObjPtr))->GetMaxLength();

        // BugZ#628 - Неправильная длина редактируемого текста.
        ((DlgEdit *)(CurItem->ObjPtr))->SetMaxLength((int)Param2);

        //if (DialogMode.Check(DMODE_INITOBJECTS)) //???
        Dlg->InitDialogObjects(Param1); // переинициализируем элементы диалога
        SetFarTitle(Dlg->GetDialogTitle());
        return MaxLen;
      }
      return 0;
    }

		/*****************************************************************/
		case DM_GETDLGITEM:
		{
			FarDialogItem* Item = (FarDialogItem*)Param2;
			return (LONG_PTR)ConvertItemEx2(Item,CurItem);
		}

		/*****************************************************************/
		case DM_GETDLGITEMSHORT:
		{
			if(Param2)
			{
				if(ConvertItemEx(CVTITEM_TOPLUGINSHORT,(FarDialogItem *)Param2,CurItem,1))
				{
					if(Type==DI_LISTBOX || Type==DI_COMBOBOX)
						((FarDialogItem *)Param2)->Param.ListPos=CurItem->ListPtr?CurItem->ListPtr->GetSelectPos():0;
					return TRUE;
				}
			}
			return FALSE;
		}

		/*****************************************************************/
		case DM_SETDLGITEM:
		case DM_SETDLGITEMSHORT:
		{
			if(!Param2)
				return FALSE;
			if(Type != ((FarDialogItem *)Param2)->Type) // пока нефига менять тип
				return FALSE;
			// не менять
			if(!ConvertItemEx((Msg==DM_SETDLGITEM)?CVTITEM_FROMPLUGIN:CVTITEM_FROMPLUGINSHORT,(FarDialogItem *)Param2,CurItem,1))
				return FALSE; // invalid parameters
			CurItem->Type=Type;
			if((Type == DI_LISTBOX || Type == DI_COMBOBOX) && CurItem->ListPtr)
			CurItem->ListPtr->ChangeFlags(VMENU_DISABLED,CurItem->Flags&DIF_DISABLE);
			// еще разок, т.к. данные могли быть изменены
			Dlg->InitDialogObjects(Param1);
			SetFarTitle(Dlg->GetDialogTitle());
			if(Dlg->DialogMode.Check(DMODE_SHOW))
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
      DWORD PrevFlags=CurItem->Flags;
      if(Param2 != -1)
      {
         if(Param2)
           CurItem->Flags&=~DIF_HIDDEN;
         else
           CurItem->Flags|=DIF_HIDDEN;
        if(Dlg->DialogMode.Check(DMODE_SHOW))// && (PrevFlags&DIF_HIDDEN) != (CurItem->Flags&DIF_HIDDEN))//!(CurItem->Flags&DIF_HIDDEN))
        {
          if((CurItem->Flags&DIF_HIDDEN) && Dlg->FocusPos == (unsigned)Param1)
          {
            Param2=Dlg->ChangeFocus(Param1,1,TRUE);
            Dlg->ChangeFocus2(Param1,(int)Param2);
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

				if(SendDlgMessage(hDlg,DM_SETFOCUS,Param1,0))
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
      DWORD PrevFlags=CurItem->Flags;
      if(Param2 != -1)
      {
         if(Param2)
           CurItem->Flags&=~DIF_DISABLE;
         else
           CurItem->Flags|=DIF_DISABLE;
         if((Type == DI_LISTBOX || Type == DI_COMBOBOX) && CurItem->ListPtr)
           CurItem->ListPtr->ChangeFlags(VMENU_DISABLED,CurItem->Flags&DIF_DISABLE);
      }
      if(Dlg->DialogMode.Check(DMODE_SHOW)) //???
      {
        Dlg->ShowDialog(Param1);
        ScrBuf.Flush();
      }
      return (PrevFlags&DIF_DISABLE)?FALSE:TRUE;
    }


    /*****************************************************************/
    // получить позицию и размеры контрола
    case DM_GETITEMPOSITION: // Param1=ID, Param2=*SMALL_RECT
			if(Param2)
      {
        RECT Rect;
        if(Dlg->GetItemRect(Param1,Rect))
        {
          ((SMALL_RECT *)Param2)->Left=(short)Rect.left;
          ((SMALL_RECT *)Param2)->Top=(short)Rect.top;
          ((SMALL_RECT *)Param2)->Right=(short)Rect.right;
          ((SMALL_RECT *)Param2)->Bottom=(short)Rect.bottom;
          return TRUE;
        }
      }
      return FALSE;

    /*****************************************************************/
    case DM_SETITEMDATA:
    {
      LONG_PTR PrewDataDialog=CurItem->UserData;
      CurItem->UserData=Param2;
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
      if(IsEdit(Type))
      {
        DlgEdit *EditLine=(DlgEdit *)(CurItem->ObjPtr);
        int ClearFlag=EditLine->GetClearFlag();
        if(Param2 >= 0)
        {
          EditLine->SetClearFlag((int)Param2);
          EditLine->Select(-1,0); // снимаем выделение
          if(Dlg->DialogMode.Check(DMODE_SHOW)) //???
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
      if(IsEdit(Type) && Param2)
      {
        if(Msg == DM_GETSELECTION)
        {
					EditorSelect *EdSel=(EditorSelect *)Param2;
					DlgEdit *EditLine=(DlgEdit *)(CurItem->ObjPtr);
					EdSel->BlockStartLine=0;
					EdSel->BlockHeight=1;
					EditLine->GetSelection(EdSel->BlockStartPos,EdSel->BlockWidth);
					if(EdSel->BlockStartPos == -1 && EdSel->BlockWidth==0)
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
					if(Param2)
          {
						EditorSelect *EdSel=(EditorSelect *)Param2;
            DlgEdit *EditLine=(DlgEdit *)(CurItem->ObjPtr);
            //EdSel->BlockType=BTYPE_STREAM;
            //EdSel->BlockStartLine=0;
            //EdSel->BlockHeight=1;
            if(EdSel->BlockType==BTYPE_NONE)
              EditLine->Select(-1,0);
            else
              EditLine->Select(EdSel->BlockStartPos,EdSel->BlockStartPos+EdSel->BlockWidth);
            if(Dlg->DialogMode.Check(DMODE_SHOW)) //???
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
  }

  // Все, что сами не отрабатываем - посылаем на обработку обработчику.
  return Dlg->CallDlgProc(Msg,Param1,Param2);
}

void Dialog::SetPosition(int X1,int Y1,int X2,int Y2)
{
  CriticalSectionLock Lock(CS);

  if ( X1 >= 0 )
    RealWidth = X2-X1+1;
  else
    RealWidth = X2;

  if ( Y1 >= 0 )
    RealHeight = Y2-Y1+1;
  else
    RealHeight = Y2;

  ScreenObject::SetPosition (X1, Y1, X2, Y2);
}
//////////////////////////////////////////////////////////////////////////
BOOL Dialog::IsInited()
{
	CriticalSectionLock Lock(CS);
	return DialogMode.Check(DMODE_INITOBJECTS);
}

BOOL Dialog::IsEditChanged(unsigned ID)
{
	CriticalSectionLock Lock(CS);
  	if(ID>=ItemCount) return FALSE;
	return Item[ID]->IFlags.Check(DLGIIF_EDITCHANGEPROCESSED);
}

void Dialog::SetComboBoxPos()
{
	if(GetDropDownOpened())
	{
		int EditX1,EditY1,EditX2,EditY2;
		((DlgEdit*)(Item[FocusPos]->ObjPtr))->GetPosition(EditX1,EditY1,EditX2,EditY2);
		if(EditX2-EditX1<20)
			EditX2=EditX1+20;
		if(ScrY-EditY1<Min(Opt.Dialogs.CBoxMaxHeight,Item[FocusPos]->ListPtr->GetItemCount())+2 && EditY1>ScrY/2)
			Item[FocusPos]->ListPtr->SetPosition(EditX1,Max(0,EditY1-1-Min(Opt.Dialogs.CBoxMaxHeight,Item[FocusPos]->ListPtr->GetItemCount())-1),EditX2,EditY1-1);
		else
			Item[FocusPos]->ListPtr->SetPosition(EditX1,EditY1+1,EditX2,0);
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
