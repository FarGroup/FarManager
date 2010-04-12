/*
vmenu.cpp

Обычное вертикальное меню
  а так же:
    * список в DI_COMBOBOX
    * список в DI_LISTBOX
    * ...
*/

#include "headers.hpp"
#pragma hdrstop

#include "vmenu.hpp"
#include "global.hpp"
#include "lang.hpp"
#include "fn.hpp"
#include "keys.hpp"
#include "macroopcode.hpp"
#include "colors.hpp"
#include "chgprior.hpp"
#include "dialog.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "constitle.hpp"

/* $ 18.07.2000 SVS
   ! изменен вызов конструктора (isListBoxControl) с учетом необходимости
     scrollbar в DI_LISTBOX & DI_COMBOBOX
*/
VMenu::VMenu(const char *Title,       // заголовок меню
             struct MenuData *Data, // пункты меню
             int ItemCount,     // количество пунктов меню
             int MaxHeight,     // максимальная высота
             DWORD Flags,       // нужен ScrollBar?
             FARWINDOWPROC Proc,    // обработчик
             Dialog *ParentDialog)  // родитель для ListBox
{
	int I;
	SetDynamicallyBorn(false);
	VMenu::VMFlags.Set(Flags|VMENU_MOUSEREACTION);
	VMenu::VMFlags.Clear(VMENU_MOUSEDOWN);
	/* SVS $ */
	/*& 28.05.2001 OT Запретить перерисовку фрема во время запуска меню */
//  FrameFromLaunched=FrameManager->GetCurrentFrame();
//  FrameFromLaunched->LockRefresh();
	/* OT &*/
	VMenu::ParentDialog=ParentDialog;
	/* $ 03.06.2001 KM
	   ! Убрана дефолтная установка флага VMENU_WRAPMODE, в противном
	     случае при создании меню прокрутка работала _ВСЕГДА_, что
	     не всегда удобно.
	*/
	VMFlags.Set(VMENU_UPDATEREQUIRED);
	/* KM $ */
	VMFlags.Clear(VMENU_SHOWAMPERSAND);
	TopPos=0;
	SaveScr=NULL;
	OldTitle=NULL;
	GetCursorType(PrevCursorVisible,PrevCursorSize);

	if (!Proc) // функция должна быть всегда!!!
		Proc=(FARWINDOWPROC)VMenu::DefMenuProc;

	VMenuProc=Proc;

	if (Title!=NULL)
		xstrncpy(VMenu::Title,Title, sizeof(VMenu::Title)-1);
	else
		*VMenu::Title=0;

	*BottomTitle=0;
	VMenu::Item=NULL;
	VMenu::ItemCount=0;
	VMenu::LastAddedItem = 0;
	/* $ 01.08.2000 SVS
	 - Bug в конструкторе, если передали NULL для Title
	*/
	/* $ 30.11.2001 DJ
	   инициализируем перед тем, как добавлять айтема
	*/
	MaxLength=(int)strlen(VMenu::Title)+2;
	/* DJ $ */
	/* SVS $ */
	VMenu::ItemHiddenCount=0;
	struct MenuItem NewItem;

	for (I=0; I < ItemCount; I++)
	{
		memset(&NewItem,0,sizeof(NewItem));

		if ((DWORD_PTR)Data[I].Name < MAX_MSG)
			xstrncpy(NewItem.Name,MSG((int)(DWORD_PTR)Data[I].Name),sizeof(NewItem.Name)+1);
		else
			xstrncpy(NewItem.Name,Data[I].Name,sizeof(NewItem.Name)+1);

		//NewItem.AmpPos=-1;
		NewItem.AccelKey=Data[I].AccelKey;
		NewItem.Flags=Data[I].Flags;
		AddItem(&NewItem);
	}

	BoxType=DOUBLE_BOX;

	for (SelectPos=-1,I=0; I<ItemCount; I++)
	{
		int Length=(int)strlen(Item[I].Name);

		if (Length>MaxLength)
			MaxLength=Length;

		if ((Item[I].Flags&LIF_SELECTED) && !(Item[I].Flags&(LIF_DISABLE | LIF_HIDDEN)))
			SelectPos=I;
	}

	VMFlags.Clear(VMENU_SELECTPOSNONE);

	if (SelectPos < 0)
		SelectPos=SetSelectPos(0,1);

	if (SelectPos < 0)
	{
		VMFlags.Set(VMENU_SELECTPOSNONE);
		SelectPos=0;
	}

	SetMaxHeight(MaxHeight);
	/* $ 28.07.2000 SVS
	   Установим цвет по умолчанию
	*/
	SetColors(NULL);

	/* SVS $*/
	if (!VMFlags.Check(VMENU_LISTBOX) && CtrlObject!=NULL)
	{
		PrevMacroMode=CtrlObject->Macro.GetMode();

		if (PrevMacroMode!=MACRO_MAINMENU &&
		        PrevMacroMode!=MACRO_DIALOG &&
		        PrevMacroMode!=MACRO_USERMENU)
			CtrlObject->Macro.SetMode(MACRO_MENU);
	}

	if (!VMFlags.Check(VMENU_LISTBOX))
		FrameManager->ModalizeFrame(this);
}


VMenu::~VMenu()
{
	if (!VMFlags.Check(VMENU_LISTBOX) && CtrlObject!=NULL)
		CtrlObject->Macro.SetMode(PrevMacroMode);

	Hide();
	DeleteItems();
	/*& 28.05.2001 OT Разрешить перерисовку фрейма, в котором создавалось это меню */
//  FrameFromLaunched->UnlockRefresh();
	/* OT &*/
	SetCursorType(PrevCursorVisible,PrevCursorSize);

	if (!VMFlags.Check(VMENU_LISTBOX))
	{
		FrameManager->UnmodalizeFrame(this);
		FrameManager->RefreshFrame();
	}
}

void VMenu::Hide()
{
	CriticalSectionLock Lock(CS);
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

	if (!VMFlags.Check(VMENU_LISTBOX) && SaveScr)
	{
		delete SaveScr;
		SaveScr=NULL;
		ScreenObject::Hide();
	}

	Y2=-1;
//  X2=-1;
	VMFlags.Set(VMENU_UPDATEREQUIRED);

	if (OldTitle)
	{
		delete OldTitle;
		OldTitle=NULL;
	}
}


void VMenu::Show()
{
	CriticalSectionLock Lock(CS);
	int OldX1 = X1, OldY1 = Y1, OldX2 = X2, OldY2 = Y2;

	if (VMFlags.Check(VMENU_LISTBOX))
	{
		if (VMFlags.Check(VMENU_SHOWNOBOX))
			BoxType=NO_BOX;
		else if (VMFlags.Check(VMENU_LISTHASFOCUS))
			BoxType=SHORT_DOUBLE_BOX;
		else
			BoxType=SHORT_SINGLE_BOX;
	}

	if (!VMFlags.Check(VMENU_LISTBOX))
	{
		int AutoCenter=FALSE,AutoHeight=FALSE;

		if (!VMFlags.Check(VMENU_COMBOBOX))
		{
			if (X1==-1)
			{
				X1=(ScrX-MaxLength-4)/2;
				AutoCenter=TRUE;
			}

			if (X1<2)
				X1=2;

			if (X2<=0)
				X2=X1+MaxLength+4;

			if (!AutoCenter && X2 > ScrX-4+2*(BoxType==SHORT_DOUBLE_BOX || BoxType==SHORT_SINGLE_BOX))
			{
				X1+=ScrX-4-X2;
				X2=ScrX-4;

				if (X1<2)
				{
					X1=2;
					X2=ScrX-2;
				}
			}

			if (X2>ScrX-2)
				X2=ScrX-2;

			if (Y1==-1)
			{
				if (MaxHeight!=0 && MaxHeight<GetShowItemCount()) //???
					Y1=(ScrY-MaxHeight-2)/2;
				else if ((Y1=(ScrY-GetShowItemCount()-2)/2)<0) // ???
					Y1=0;

				AutoHeight=TRUE;
			}
		}

		if (Y2<=0)
		{
			if (MaxHeight!=0 && MaxHeight<GetShowItemCount()) //???
				Y2=Y1+MaxHeight+1;
			else
				Y2=Y1+GetShowItemCount()+1; //???
		}

		if (Y2>ScrY)
			Y2=ScrY;

		if (AutoHeight && Y1<3 && Y2>ScrY-3)
		{
			Y1=2;
			Y2=ScrY-2;
		}
	}

	if (X2>X1 && Y2+(VMFlags.Check(VMENU_SHOWNOBOX)?1:0)>Y1)
	{
		/*  if ( (OldX1 != X1) ||
		       (OldY1 != Y1) ||
		       (OldX2 != X2) ||
		       (OldY2 != Y2) )
		     VMFlags.Clear (VMENU_DISABLEDRAWBACKGROUND);*/
		if (SelectPos == -1)
			SelectPos=SetSelectPos(0,1);

//_SVS(SysLog("VMenu::Show()"));
		if (!VMFlags.Check(VMENU_LISTBOX))
			ScreenObject::Show();
//      Show();
		else
		{
			VMFlags.Set(VMENU_UPDATEREQUIRED);
			DisplayObject();
		}
	}
}

/* $ 28.07.2000 SVS
   Переработка функции с учетом VMenu::Colors[] -
      заменены константы на VMenu::Colors[]
*/
void VMenu::DisplayObject()
{
	CriticalSectionLock Lock(CS);
//_SVS(SysLog("VMFlags&VMENU_UPDATEREQUIRED=%d",VMFlags.Check(VMENU_UPDATEREQUIRED)));
//  if (!(VMFlags&VMENU_UPDATEREQUIRED))
//    return;
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	VMFlags.Clear(VMENU_UPDATEREQUIRED);
	Modal::ExitCode=-1;
	SetCursorType(0,10);

	if (!VMFlags.Check(VMENU_LISTBOX) && SaveScr==NULL)
	{
		if (!VMFlags.Check(VMENU_DISABLEDRAWBACKGROUND) && !(BoxType==SHORT_DOUBLE_BOX || BoxType==SHORT_SINGLE_BOX))
			SaveScr=new SaveScreen(X1-2,Y1-1,X2+4,Y2+2);
		else
			SaveScr=new SaveScreen(X1,Y1,X2+2,Y2+1);
	}

	if (!VMFlags.Check(VMENU_DISABLEDRAWBACKGROUND))
	{
		/* $ 23.07.2000 SVS
		   Тень для ListBox ненужна
		*/
		if (BoxType==SHORT_DOUBLE_BOX || BoxType==SHORT_SINGLE_BOX)
		{
			Box(X1,Y1,X2,Y2,VMenu::Colors[VMenuColorBox],BoxType);

			if (!VMFlags.Check(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR))
			{
				MakeShadow(X1+2,Y2+1,X2+1,Y2+1);
				MakeShadow(X2+1,Y1+1,X2+2,Y2+1);
			}
		}
		else
		{
			if (BoxType!=NO_BOX)
				SetScreen(X1-2,Y1-1,X2+2,Y2+1,' ',VMenu::Colors[VMenuColorBody]);
			else
				SetScreen(X1,Y1,X2,Y2,' ',VMenu::Colors[VMenuColorBody]);

			if (!VMFlags.Check(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR))
			{
				MakeShadow(X1,Y2+2,X2+3,Y2+2);
				MakeShadow(X2+3,Y1,X2+4,Y2+2);
			}

			if (BoxType!=NO_BOX)
				Box(X1,Y1,X2,Y2,VMenu::Colors[VMenuColorBox],BoxType);
		}

//    VMFlags.Set (VMENU_DISABLEDRAWBACKGROUND);
	}

	if (!VMFlags.Check(VMENU_LISTBOX))
		DrawTitles();

	ShowMenu(TRUE);
}

void VMenu::DrawTitles()
{
	CriticalSectionLock Lock(CS);
	int MaxTitleLength = X2-X1-2;
	int WidthTitle;

	if (*Title)
	{
		if ((WidthTitle=(int)strlen(Title)) > MaxTitleLength)
			WidthTitle=MaxTitleLength-1;

		GotoXY(X1+(X2-X1-1-WidthTitle)/2,Y1);
		SetColor(VMenu::Colors[VMenuColorTitle]);
		mprintf(" %*.*s ",WidthTitle,WidthTitle,Title);
	}

	if (*BottomTitle)
	{
		if ((WidthTitle=(int)strlen(BottomTitle)) > MaxTitleLength)
			WidthTitle=MaxTitleLength-1;

		GotoXY(X1+(X2-X1-1-WidthTitle)/2,Y2);
		SetColor(VMenu::Colors[VMenuColorTitle]);
		mprintf(" %*.*s ",WidthTitle,WidthTitle,BottomTitle);
	}
}

/* $ 28.07.2000 SVS
   Переработка функции с учетом VMenu::Colors[] -
      заменены константы на VMenu::Colors[]
*/
void VMenu::ShowMenu(int IsParent)
{
	CriticalSectionLock Lock(CS);
	char TmpStr[2048];
	unsigned char BoxChar[2],BoxChar2[2];
	int Y,I;

	/* $ 23.02.2002 DJ
	   если в меню нету пунктов - это не значит, что не надо рисовать фон!
	*/
	if (/*ItemCount==0 ||*/ X2<=X1 || Y2<=Y1)  /* DJ $ */
	{
		if (!(VMFlags.Check(VMENU_SHOWNOBOX) && Y2==Y1))
			return;
	}

	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	BoxChar2[1]=BoxChar[1]=0;

	// коррекция Top`а
	if (TopPos+ItemCount >= Y2-Y1 && SelectPos == ItemCount-1)
		TopPos--;

	/* $ 11.01.2003 KM
	   ! Иногда TopPos=-1, что в дальнейшем приводило к отрицательному
	     индексу и, естественно, приводило к exception.
	*/
	if (TopPos<0)
		TopPos=0;

	/* $ 22.07.2001 KM
	 - Не устанавливался тип рамки при первом вызове
	   ShowMenu с параметром TRUE, что давало неверную
	   отрисовку меню.
	*/

	/* $ 21.02.2002 DJ
	   а для списков, не имеющих фокуса, нужна одиночная рамка!
	*/
	if (VMFlags.Check(VMENU_LISTBOX))
	{
		if (VMFlags.Check(VMENU_SHOWNOBOX))
			BoxType = NO_BOX;
		else if (VMFlags.Check(VMENU_LISTHASFOCUS))
			BoxType = SHORT_DOUBLE_BOX;
		else
			BoxType = SHORT_SINGLE_BOX;
	}

	if (VMFlags.Check(VMENU_LISTBOX))
	{
		if ((!IsParent || !GetShowItemCount()))
		{
			if (GetShowItemCount())
				BoxType=VMFlags.Check(VMENU_SHOWNOBOX)?NO_BOX:SHORT_SINGLE_BOX;

			SetScreen(X1,Y1,X2,Y2,' ',VMenu::Colors[VMenuColorBody]);
		}

		if (BoxType!=NO_BOX)
			Box(X1,Y1,X2,Y2,VMenu::Colors[VMenuColorBox],BoxType);

		DrawTitles();
	}

	switch (BoxType)
	{
		case NO_BOX:
			*BoxChar=' ';
			break;
		case SINGLE_BOX:
		case SHORT_SINGLE_BOX:
			*BoxChar=0x0B3; // |
			break;
		case DOUBLE_BOX:
		case SHORT_DOUBLE_BOX:
			*BoxChar=0x0BA; // ||
			break;
	}

	if (GetShowItemCount() <= 0)
		return;

	if (SelectPos < ItemCount && SelectPos >= 0)
	{
		if (Item[SelectPos].Flags&(LIF_DISABLE | LIF_HIDDEN))
			Item[SelectPos].Flags&=~LIF_SELECTED;
		else
			Item[SelectPos].Flags|=LIF_SELECTED;
	}

	/* $ 02.12.2001 KM
	   ! Предварительно, если нужно, настроим "горячие" клавиши.
	*/
	if (VMFlags.Check(VMENU_AUTOHIGHLIGHT|VMENU_REVERSEHIGHLIGHT))
		AssignHighlights(VMFlags.Check(VMENU_REVERSEHIGHLIGHT));

	/* $ 21.07.2001 KM
	 ! Переработка отрисовки меню с флагом VMENU_SHOWNOBOX.
	*/
	if (SelectPos-ItemHiddenCount > TopPos+((BoxType!=NO_BOX)?Y2-Y1-2:Y2-Y1))
		TopPos=SelectPos-((BoxType!=NO_BOX)?Y2-Y1-2:Y2-Y1);

	if (SelectPos < TopPos)
		TopPos=SelectPos;

	if (TopPos<0)
		TopPos=0;

	for (Y=Y1+((BoxType!=NO_BOX)?1:0),I=TopPos; Y < ((BoxType!=NO_BOX)?Y2:Y2+1); Y++,I++)
	{
		GotoXY(X1,Y);

		if (I < ItemCount)
		{
			if (Item[I].Flags&LIF_HIDDEN)
			{
				Y--;
				continue;
			}

			if (Item[I].Flags&LIF_SEPARATOR)
			{
				int SepWidth=X2-X1+1;
				char *Ptr=TmpStr+1;
				MakeSeparator(SepWidth,TmpStr,
				              BoxType==NO_BOX?0:(BoxType==SINGLE_BOX||BoxType==SHORT_SINGLE_BOX?2:1));

				if (I>0 && I<ItemCount-1 && SepWidth>3)
					for (unsigned int J=0; Ptr[J+3]!=0; J++)
					{
						if (Item[I-1][J]==0)
							break;

						if (Item[I-1][J]==0x0B3)
						{
							int Correction=0;

							if (!VMFlags.Check(VMENU_SHOWAMPERSAND) && memchr(Item[I-1].PtrName(),'&',J)!=NULL)
								Correction=1;

							if (strlen(Item[I+1].PtrName())>=J && Item[I+1][J]==0x0B3)
								Ptr[J-Correction+2]=0x0C5;
							else
								Ptr[J-Correction+2]=0x0C1;
						}
					}

				//Text(X1,Y,VMenu::Colors[VMenuColorSeparator],TmpStr); // VMenuColorBox
				SetColor(VMenu::Colors[VMenuColorSeparator]);
#if defined(USE_WFUNC)

				if (Opt.UseUnicodeConsole)
					BoxTextW2(TmpStr,FALSE);
				else
#endif
					BoxText(TmpStr,FALSE);

				if (*Item[I].PtrName())
				{
					int ItemWidth=(int)strlen(Item[I].PtrName());

					if (ItemWidth > X2-X1-3) // 1 ???
						ItemWidth=X2-X1-3;

					GotoXY(X1+(X2-X1-1-ItemWidth)/2,Y);
					mprintf(" %*.*s ",ItemWidth,ItemWidth,Item[I].PtrName());
					//??????
				}
			}
			else
			{
				if (BoxType!=NO_BOX)
				{
					SetColor(VMenu::Colors[VMenuColorBox]);
					BoxText((WORD)(Opt.UseUnicodeConsole?BoxSymbols[*BoxChar-0x0B0]:*BoxChar)); //Text((char*)BoxChar);
					GotoXY(X2,Y);
					BoxText((WORD)(Opt.UseUnicodeConsole?BoxSymbols[*BoxChar-0x0B0]:*BoxChar)); //Text((char*)BoxChar);
				}

				char *Item_I_PtrName=Item[I].PtrName();
				char *_MItemPtr=Item_I_PtrName+Item[I].ShowPos;
//        if ((Item[I].Flags&LIF_SELECTED) && !(Item[I].Flags&LIF_DISABLE))
//          SetColor(VMenu::Colors[VMenuColorSelected]);
//        else
//          SetColor(VMenu::Colors[(Item[I].Flags&LIF_DISABLE?9:3)]);

				if (BoxType!=NO_BOX)
				{
					/*
					          if(Item[I].ShowPos > 0)
					          {
					            //Text(X1,Y,VMenu::Colors[Item[I].Flags&LIF_SELECTED?VMenuColorHSelect:VMenuColorHilite],"{");
					            GotoXY(X1,Y);
					            SetColor((Item[I].Flags&LIF_SELECTED)?VMenu::Colors[VMenuColorHSelect]:VMenu::Colors[VMenuColorHilite]);
					            BoxText(Opt.UseUnicodeConsole?0xab:'<');
					          }
					*/
					GotoXY(X1+1,Y);
				}
				else
					GotoXY(X1,Y);

//
				if ((Item[I].Flags&LIF_SELECTED) && !(Item[I].Flags&LIF_DISABLE))
					SetColor(VMenu::Colors[Item[I].Flags&LIF_GRAYED?VMenuColorSelGrayed:VMenuColorSelected]);
				else
					SetColor(VMenu::Colors[(Item[I].Flags&LIF_DISABLE?VMenuColorDisabled:(Item[I].Flags&LIF_GRAYED?VMenuColorGrayed:VMenuColorText))]);

				char Check=' ';

				if (Item[I].Flags&LIF_CHECKED)
					if (!(Item[I].Flags&0x0000FFFF))
						Check=0x0FB;
					else
						Check=(char)Item[I].Flags&0x0000FFFF;

				int Len_MItemPtr;

				if (VMFlags.Check(VMENU_SHOWAMPERSAND))
					Len_MItemPtr=(int)strlen(_MItemPtr);
				else
					Len_MItemPtr=HiStrlen(_MItemPtr);

#if defined(__BORLANDC__)
#define _snprintf FarSnprintf
#endif
				_snprintf(TmpStr,sizeof(TmpStr)-1,"%c %s",Check,_MItemPtr);

				if (Len_MItemPtr+2 > X2-X1-3)
					TmpStr[HiFindRealPos(TmpStr,X2-X1-1,VMFlags.Check(VMENU_SHOWAMPERSAND))]=0;

				{ // табуляции меняем только при показе!!!
					// для сохранение оригинальной строки!!!
					char *TabPtr;

					while ((TabPtr=strchr(TmpStr,'\t'))!=NULL)
						*TabPtr=' ';
				}
				int Col;

				if (!(Item[I].Flags&LIF_DISABLE))
				{
					if (Item[I].Flags&LIF_SELECTED)
						Col=VMenu::Colors[Item[I].Flags&LIF_GRAYED?VMenuColorSelGrayed:VMenuColorHSelect];
					else
						Col=VMenu::Colors[Item[I].Flags&LIF_GRAYED?VMenuColorGrayed:VMenuColorHilite];
				}
				else
					Col=VMenu::Colors[VMenuColorDisabled];

				if (VMFlags.Check(VMENU_SHOWAMPERSAND))
				{
					Text(TmpStr);
				}
				else
				{
					short AmpPos=Item[I].AmpPos+2;

					if (AmpPos >= 2 && AmpPos < sizeof(TmpStr) && TmpStr[AmpPos] != '&')
					{
						memmove(TmpStr+AmpPos+1,TmpStr+AmpPos,strlen(TmpStr+AmpPos)+1);
						TmpStr[AmpPos]='&';
					}

					HiText(TmpStr,Col);
				}

				// сделаем добавочку для NO_BOX
				mprintf("%*s",X2-WhereX()+(BoxType==NO_BOX?1:0),"");
				SetColor(VMenu::Colors[(Item[I].Flags&LIF_DISABLE)?VMenuColorArrowsDisabled:(Item[I].Flags&LIF_SELECTED?VMenuColorArrowsSelect:VMenuColorArrows)]);

				if (/*BoxType!=NO_BOX && */Item[I].ShowPos > 0)
				{
					GotoXY(X1+1,Y);
					BoxText(Opt.UseUnicodeConsole?0xab:'<');
				}

				if (Len_MItemPtr > X2-X1-3)
				{
					//if ((VMFlags.Check(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR) || Opt.ShowMenuScrollbar) && (((BoxType!=NO_BOX)?Y2-Y1-1:Y2-Y1+1)<ItemCount))
					//  GotoXY(WhereX()-1,Y);
					//else
					GotoXY(X2-1,Y);
					BoxText(Opt.UseUnicodeConsole?0xbb:'>');
				}
			}
		}
		else
		{
			/* $ 21.07.2001 KM
			 ! Переработка отрисовки меню с флагом VMENU_SHOWNOBOX.
			*/
			if (BoxType!=NO_BOX)
			{
				SetColor(VMenu::Colors[VMenuColorBox]);
				BoxText((WORD)(Opt.UseUnicodeConsole?BoxSymbols[*BoxChar-0x0B0]:*BoxChar)); //Text((char*)BoxChar);
				GotoXY(X2,Y);
				BoxText((WORD)(Opt.UseUnicodeConsole?BoxSymbols[*BoxChar-0x0B0]:*BoxChar)); //Text((char*)BoxChar);
				GotoXY(X1+1,Y);
			}
			else
				GotoXY(X1,Y);

			SetColor(VMenu::Colors[VMenuColorText]);
			// сделаем добавочку для NO_BOX
			mprintf("%*s",((BoxType!=NO_BOX)?X2-X1-1:X2-X1)+(BoxType==NO_BOX?1:0),"");
		}
	}

	/* $ 28.06.2000 tran
	     показываем скролбар если пунктов в меню больше чем
	     его высота
	   $ 29.06.2000 SVS
	     Показывать ScrollBar в меню если включена опция Opt.ShowMenuScrollbar
	   $ 18.07.2000 SVS
	     + всегда покажет scrollbar для DI_LISTBOX & DI_COMBOBOX и опционально
	       для вертикального меню
	*/

	/* $ 21.07.2001 KM
	 ! Переработка отрисовки меню с флагом VMENU_SHOWNOBOX.
	*/
	if (VMFlags.Check(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR) || Opt.ShowMenuScrollbar)
	{
		if (((BoxType!=NO_BOX)?Y2-Y1-1:Y2-Y1+1) < GetShowItemCount())
		{
			SetColor(VMenu::Colors[VMenuColorScrollBar]);

			if (BoxType != NO_BOX)
				ScrollBar(X2,Y1+1,Y2-Y1-1,SelectPos,GetShowItemCount()); //SelectPos vs TopPos?
			else
				ScrollBar(X2,Y1,Y2-Y1+1,SelectPos,GetShowItemCount());   //SelectPos vs TopPos?
		}
	}
}

BOOL VMenu::UpdateRequired(void)
{
	CriticalSectionLock Lock(CS);
	return ((LastAddedItem>=TopPos && LastAddedItem<(TopPos+Y2-Y1)+2) || VMFlags.Check(VMENU_UPDATEREQUIRED));
	//+1 for real diff
	//+2 for scrollbar
}

BOOL VMenu::CheckKeyHiOrAcc(DWORD Key,int Type,int Translate)
{
	CriticalSectionLock Lock(CS);
	int I;
	struct MenuItem *CurItem;

	if (VMFlags.Check(VMENU_LISTBOX)) // не забудем сбросить EndLoop для листбокса,
		EndLoop=FALSE;                 // иначе не будут работать хоткеи в активном списке

	for (CurItem=Item,I=0; I < ItemCount; I++, ++CurItem)
	{
		if (!(CurItem->Flags&(LIF_DISABLE|LIF_HIDDEN)) &&
		        (
		            (!Type && CurItem->AccelKey && Key == CurItem->AccelKey) ||
		            (Type && Dialog::IsKeyHighlighted(CurItem->PtrName(),Key,Translate,CurItem->AmpPos))
		        )
		   )
		{
			Item[SelectPos].Flags&=~LIF_SELECTED;
			CurItem->Flags|=LIF_SELECTED;
			SelectPos=I;
			ShowMenu(TRUE);

			if (!VMenu::ParentDialog && !(Item[SelectPos].Flags&LIF_GRAYED))
			{
				Modal::ExitCode=I;
				EndLoop=TRUE;
			}

			break;
		}
	}

	return EndLoop==TRUE;
}

__int64 VMenu::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return (__int64)(ItemCount<=0);
		case MCODE_C_EOF:
			return (__int64)(SelectPos==ItemCount-1);
		case MCODE_C_BOF:
			return (__int64)(SelectPos==0);
		case MCODE_C_SELECTED:
			return (__int64)(ItemCount > 0 && SelectPos >= 0);
		case MCODE_V_ITEMCOUNT:
			return (__int64)GetShowItemCount(); // ????
		case MCODE_V_CURPOS:
			return (__int64)(SelectPos+1);
		case MCODE_F_MENU_CHECKHOTKEY:
		{
			const char *str = (const char *)vParam;
			return (__int64)(CheckHighlights(*str,(int)iParam)+1);
		}
		case MCODE_F_MENU_SELECT:
		{
			const char *str = (const char *)vParam;

			if (*str)
			{
				char Temp[3*NM];
				int Res;

				for (int I=0; I < ItemCount; ++I)
				{
					struct MenuItem *_item=GetItemPtr(I);

					if (_item->Flags&LIF_HIDDEN) //???
						continue;

					Res=0;
					RemoveExternalSpaces(HiText2Str(Temp,sizeof(Temp),((struct MenuItem *)_item)->PtrName()));
					const char *p;

					switch (iParam)
					{
						case 0: // full compare
							Res=LocalStricmp(Temp,str)==0;
							break;
						case 1: // begin compare
							p=LocalStrstri(Temp,str);
							Res= p==Temp;
							break;
						case 2: // end compare
							p=LocalRevStrstri(Temp,str);
							Res= p && p[strlen(str)] == 0;
							break;
						case 3: // in str
							Res=LocalStrstri(Temp,str)!=NULL;
							break;
//            case 4: // pattern
//              Res=CmpName(str,Temp,FALSE);
//              break;
					}

					if (Res)
					{
						SelectPos=SetSelectPos(I,1);

						if (SelectPos != I)
						{
							SelectPos=SetSelectPos(SelectPos,1);
							return _i64(0);
						}

						ShowMenu(TRUE);
						return (__int64)(SelectPos+1);
					}
				}
			}

			return _i64(0);
		}
		case MCODE_F_MENU_GETHOTKEY:
		{
			if (iParam == _i64(-1))
				iParam=(__int64)SelectPos;

			if ((int)iParam < ItemCount) //????
				return (__int64)((DWORD)GetHighlights(GetItemPtr((int)iParam)));

			return _i64(0);
		}
	}

	return _i64(0);
}

int FindNextVisualPos(const char *Str, int Pos, int Direct)
{
	/*
	    &&      = '&'
	    &&&     = '&'
	               ^H
	    &&&&    = '&&'
	    &&&&&   = '&&'
	               ^H
	    &&&&&&  = '&&&'
	*/
	if (Str)
	{
		if (Direct < 0)
		{
			if (!Pos || Pos == 1)
				return 0;

			if (Str[Pos-1] != '&')
			{
				if (Str[Pos-2] == '&')
				{
					if (Pos-3 >= 0 && Str[Pos-3] == '&')
						return Pos-1;

					return Pos-2;
				}

				return Pos-1;
			}
			else
			{
				if (Pos-3 >= 0 && Str[Pos-3] == '&')
				{
					return Pos-3;
				}

				return Pos-2;
			}
		}
		else
		{
			if (!Str[Pos])
				return Pos+1;

			if (Str[Pos] == '&')
			{
				if (Str[Pos+1] == '&' && Str[Pos+2] == '&')
					return Pos+3;

				return Pos+2;
			}
			else
			{
				return Pos+1;
			}
		}
	}

	return 0;
}

BOOL VMenu::ShiftItemShowPos(int Pos,int Direct)
{
	int _len;
	int _OWidth=X2-X1-3;
	int ItemShowPos=Item[Pos].ShowPos;

	if (VMFlags.Check(VMENU_SHOWAMPERSAND))
		_len=(int)strlen(Item[Pos].PtrName());
	else
		_len=HiStrlen(Item[Pos].PtrName());

	if (_len < _OWidth ||
	        (Direct < 0 && ItemShowPos==0) ||
	        (Direct > 0 && ItemShowPos > _len))
		return FALSE;

	if (VMFlags.Check(VMENU_SHOWAMPERSAND))
	{
		if (Direct < 0)
			ItemShowPos--;
		else
			ItemShowPos++;
	}
	else
	{
		ItemShowPos = FindNextVisualPos(Item[Pos].PtrName(),ItemShowPos,Direct);
	}

	if (ItemShowPos < 0)
		ItemShowPos=0;

	if (ItemShowPos > _len-_OWidth)
		ItemShowPos=_len-_OWidth+1;

	if (ItemShowPos != Item[Pos].ShowPos)
	{
		Item[Pos].ShowPos=ItemShowPos;
		VMFlags.Set(VMENU_UPDATEREQUIRED);
		return TRUE;
	}

	return FALSE;
}

int VMenu::ProcessKey(int Key)
{
	CriticalSectionLock Lock(CS);
	int I;

	if (Key==KEY_NONE || Key==KEY_IDLE)
		return(FALSE);

	if (Key == KEY_OP_PLAINTEXT)
	{
		const char *str = eStackAsString();

		if (!*str)
			return FALSE;

		Key=*str;
	}

	VMFlags.Set(VMENU_UPDATEREQUIRED);

	if (GetShowItemCount()==0)
		if (Key!=KEY_F1 && Key!=KEY_SHIFTF1 && Key!=KEY_F10 && Key!=KEY_ESC && Key!=KEY_ALTF9)
		{
			Modal::ExitCode=-1;
			return(FALSE);
		}

	if (!(Key >= KEY_MACRO_BASE && Key <= KEY_MACRO_ENDBASE || Key >= KEY_OP_BASE && Key <= KEY_OP_ENDBASE))
	{
		DWORD S=Key&(KEY_CTRL|KEY_ALT|KEY_SHIFT|KEY_RCTRL|KEY_RALT);
		DWORD K=Key&(~(KEY_CTRL|KEY_ALT|KEY_SHIFT|KEY_RCTRL|KEY_RALT));

		if (K==KEY_MULTIPLY)
			Key='*'|S;
		else if (K==KEY_ADD)
			Key='+'|S;
		else if (K==KEY_SUBTRACT)
			Key='-'|S;
		else if (K==KEY_DIVIDE)
			Key='/'|S;
	}

	switch (Key)
	{
		case KEY_ALTF9:
			FrameManager->ProcessKey(KEY_ALTF9);
			break;
		case KEY_NUMENTER:
		case KEY_ENTER:
		{
			if (!VMenu::ParentDialog)
			{
				if (!(Item[SelectPos].Flags&(LIF_DISABLE | LIF_HIDDEN | LIF_GRAYED)))
				{
					EndLoop=TRUE;
					Modal::ExitCode=SelectPos;
				}
			}

			break;
		}
		case KEY_ESC:
		case KEY_F10:
		{
			if (!VMenu::ParentDialog)
			{
				EndLoop=TRUE;
				Modal::ExitCode=-1;
			}

			break;
		}
		case KEY_HOME:         case KEY_NUMPAD7:
		case KEY_CTRLHOME:     case KEY_CTRLNUMPAD7:
		case KEY_CTRLPGUP:     case KEY_CTRLNUMPAD9:
		{
			SelectPos=SetSelectPos(0,1);
			ShowMenu(TRUE);
			break;
		}
		case KEY_END:          case KEY_NUMPAD1:
		case KEY_CTRLEND:      case KEY_CTRLNUMPAD1:
		case KEY_CTRLPGDN:     case KEY_CTRLNUMPAD3:
		{
			SelectPos=SetSelectPos(ItemCount-1,-1);
			ShowMenu(TRUE);
			break;
		}
		case KEY_PGUP:         case KEY_NUMPAD9:
		{
			/* $ 22.07.2001 KM
			 ! Исправление неточности перехода по PgUp/PgDn
			   с установленным флагом VMENU_SHOWNOBOX (NO_BOX)
			*/
			if ((I=SelectPos-((BoxType!=NO_BOX)?Y2-Y1-1:Y2-Y1)) < 0)
				I=0;

			SelectPos=SetSelectPos(I,1);
			ShowMenu(TRUE);
			break;
		}
		case KEY_PGDN:         case KEY_NUMPAD3:
		{
			if ((I=SelectPos+((BoxType!=NO_BOX)?Y2-Y1-1:Y2-Y1)) >= ItemCount)
				I=ItemCount-1;

			SelectPos=SetSelectPos(I,-1);
			ShowMenu(TRUE);
			break;
		}
		case KEY_ALTHOME:           case KEY_NUMPAD7|KEY_ALT:
		case KEY_ALTEND:            case KEY_NUMPAD1|KEY_ALT:
		{
			if (Key == KEY_ALTHOME || Key == (KEY_NUMPAD7|KEY_ALT))
			{
				for (I=0; I < ItemCount; ++I)
					Item[I].ShowPos=0;
			}
			else
			{
				int _len;
				int _OWidth=X2-X1-3;

				for (I=0; I < ItemCount; ++I)
				{
					if (VMFlags.Check(VMENU_SHOWAMPERSAND))
						_len=(int)strlen(Item[I].PtrName());
					else
						_len=HiStrlen(Item[I].PtrName());

					if (_len >= _OWidth)
						Item[I].ShowPos=_len-_OWidth+1;
				}
			}

			ShowMenu(TRUE);
			break;
		}
		case KEY_ALTLEFT:  case KEY_NUMPAD4|KEY_ALT: case KEY_MSWHEEL_LEFT:
		case KEY_ALTRIGHT: case KEY_NUMPAD6|KEY_ALT: case KEY_MSWHEEL_RIGHT:
		{
			BOOL NeedRedraw=FALSE;

			for (I=0; I < ItemCount; ++I)
				if (ShiftItemShowPos(I,(Key == KEY_ALTLEFT || Key == (KEY_NUMPAD4|KEY_ALT) || Key == KEY_MSWHEEL_LEFT)?-1:1))
					NeedRedraw=TRUE;

			if (NeedRedraw)
				ShowMenu(TRUE);

			break;
		}
		case KEY_ALTSHIFTLEFT:      case KEY_NUMPAD4|KEY_ALT|KEY_SHIFT:
		case KEY_ALTSHIFTRIGHT:     case KEY_NUMPAD6|KEY_ALT|KEY_SHIFT:
		{
			if (ShiftItemShowPos(SelectPos,(Key == KEY_ALTSHIFTLEFT || Key == (KEY_NUMPAD4|KEY_ALT|KEY_SHIFT))?-1:1))
				ShowMenu(TRUE);

			break;
		}
		case KEY_MSWHEEL_UP: // $ 27.04.2001 VVM - Обработка KEY_MSWHEEL_XXXX
		case KEY_LEFT:         case KEY_NUMPAD4:
		case KEY_UP:           case KEY_NUMPAD8:
		{
			SelectPos=SetSelectPos(SelectPos-1,-1);
			ShowMenu(TRUE);
			break;
		}
		case KEY_MSWHEEL_DOWN: // $ 27.04.2001 VVM + Обработка KEY_MSWHEEL_XXXX
		case KEY_RIGHT:        case KEY_NUMPAD6:
		case KEY_DOWN:         case KEY_NUMPAD2:
		{
			SelectPos=SetSelectPos(SelectPos+1,1);
			ShowMenu(TRUE);
			break;
		}
		case KEY_TAB:
		case KEY_SHIFTTAB:
		default:
		{
			int OldSelectPos=SelectPos;

			if (!CheckKeyHiOrAcc(Key,0,0))
			{
				if (Key == KEY_SHIFTF1 || Key == KEY_F1)
				{
					if (VMenu::ParentDialog)
						;//VMenu::ParentDialog->ProcessKey(Key);
					else
						ShowHelp();

					break;
				}
				else
				{
					if (!CheckKeyHiOrAcc(Key,1,FALSE))
						CheckKeyHiOrAcc(Key,1,TRUE);
				}
			}

			if (VMenu::ParentDialog && OldSelectPos!=SelectPos && Dialog::SendDlgMessage((HANDLE)ParentDialog,DN_LISTHOTKEY,DialogItemID,SelectPos))
			{
				Item[SelectPos].Flags&=~LIF_SELECTED;
				Item[OldSelectPos].Flags|=LIF_SELECTED;
				SelectPos=OldSelectPos;
				ShowMenu(TRUE);
			}

			return(FALSE);
		}
	}

	return(TRUE);
}

int VMenu::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	CriticalSectionLock Lock(CS);
	int MsPos,MsX,MsY;
	VMFlags.Set(VMENU_UPDATEREQUIRED);

	if (GetShowItemCount()==0)
	{
		if (MouseEvent->dwButtonState && MouseEvent->dwEventFlags==0)
			EndLoop=TRUE;

		Modal::ExitCode=-1;
		return(FALSE);
	}

	MsX=MouseEvent->dwMousePosition.X;
	MsY=MouseEvent->dwMousePosition.Y;

	if (MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED&&(MsX==X1+1||MsX==X2-1))
	{
		while (IsMouseButtonPressed())
			ProcessKey(MsX==X1+1?KEY_ALTLEFT:KEY_ALTRIGHT);

		return TRUE;
	}

	int SbY1=((BoxType!=NO_BOX)?Y1+1:Y1), SbY2=((BoxType!=NO_BOX)?Y2-1:Y2);
	int bShowScrollBar = FALSE;

	if (VMFlags.Check(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR) || Opt.ShowMenuScrollbar)
		bShowScrollBar = TRUE;

	if (bShowScrollBar &&
	        MsX==X2 &&
	        ((BoxType!=NO_BOX)?Y2-Y1-1:Y2-Y1+1) < ItemCount &&
	        (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
	   )
	{
		if (MsY==SbY1)
		{
			while (IsMouseButtonPressed())
			{
				if (SelectPos!=0)
					ProcessKey(KEY_UP);

				ShowMenu(TRUE);
			}

			return(TRUE);
		}

		if (MsY==SbY2)
		{
			while (IsMouseButtonPressed())
			{
				if (SelectPos!=ItemCount-1)
					ProcessKey(KEY_DOWN);

				ShowMenu(TRUE);
			}

			return(TRUE);
		}

		if (MsY>SbY1 && MsY<SbY2)
		{
			int SbHeight;
			int Delta=0;

			while (IsMouseButtonPressed())
			{
				SbHeight=Y2-Y1-2;
				MsPos=(ItemCount-1)*(MouseY-Y1)/(SbHeight);

				if (MsPos >= ItemCount)
				{
					MsPos=ItemCount-1;
					Delta=-1;
				}

				if (MsPos < 0)
				{
					MsPos=0;
					Delta=1;
				}

				if (!(Item[MsPos].Flags&LIF_SEPARATOR) && !(Item[MsPos].Flags&(LIF_DISABLE | LIF_HIDDEN)))
					SelectPos=SetSelectPos(MsPos,Delta); //??

				ShowMenu(TRUE);
			}

			return(TRUE);
		}
	}

	// dwButtonState & 3 - Left & Right button
	if (BoxType!=NO_BOX && (MouseEvent->dwButtonState & 3) && MsX>X1 && MsX<X2)
	{
		if (MsY==Y1)
		{
			while (MsY==Y1 && SelectPos>0 && IsMouseButtonPressed())
				ProcessKey(KEY_UP);

			return(TRUE);
		}

		if (MsY==Y2)
		{
			while (MsY==Y2 && SelectPos<ItemCount-1 && IsMouseButtonPressed())
				ProcessKey(KEY_DOWN);

			return(TRUE);
		}
	}

	if ((BoxType!=NO_BOX)?
	        (MsX>X1 && MsX<X2 && MsY>Y1 && MsY<Y2):
	        (MsX>=X1 && MsX<=X2 && MsY>=Y1 && MsY<=Y2))
	{
		MsPos=TopPos+((BoxType!=NO_BOX)?MsY-Y1-1:MsY-Y1);

		if (MsPos<ItemCount && !(Item[MsPos].Flags&LIF_SEPARATOR) && !(Item[MsPos].Flags&(LIF_DISABLE | LIF_HIDDEN)))
		{
			if (MouseX!=PrevMouseX || MouseY!=PrevMouseY || MouseEvent->dwEventFlags==0)
			{
				/* TODO:

				   Это заготовка для управления поведением листов "не в стиле меню" - когда текущий
				   указатель списка (позиция) следит за мышой...

				        if(!CheckFlags(VMENU_LISTBOX|VMENU_COMBOBOX) && MouseEvent->dwEventFlags==MOUSE_MOVED ||
				            CheckFlags(VMENU_LISTBOX|VMENU_COMBOBOX) && MouseEvent->dwEventFlags!=MOUSE_MOVED)
				*/
				if ((VMenu::VMFlags.Check(VMENU_MOUSEREACTION) && MouseEvent->dwEventFlags==MOUSE_MOVED)
				        ||
				        (!VMenu::VMFlags.Check(VMENU_MOUSEREACTION) && MouseEvent->dwEventFlags!=MOUSE_MOVED)
				        ||
				        (MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED))
				   )
				{
					Item[SelectPos].Flags&=~LIF_SELECTED;
					Item[MsPos].Flags|=LIF_SELECTED;
					SelectPos=MsPos;
				}

				ShowMenu(TRUE);
			}

			if (MouseEvent->dwEventFlags==0 &&
			        (MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED)))
				VMenu::VMFlags.Set(VMENU_MOUSEDOWN);

			if (MouseEvent->dwEventFlags==0 &&
			        (MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED))==0 &&
			        VMenu::VMFlags.Check(VMENU_MOUSEDOWN))
			{
				VMenu::VMFlags.Clear(VMENU_MOUSEDOWN);
				ProcessKey(KEY_ENTER);
			}
		}

		return(TRUE);
	}
	else if (BoxType!=NO_BOX && (MouseEvent->dwButtonState & 3) && MouseEvent->dwEventFlags==0)
	{
		ProcessKey(KEY_ESC);
		return(TRUE);
	}

	return(FALSE);
}


void VMenu::DeleteItems()
{
	CriticalSectionLock Lock(CS);

	if (Item)
	{
		for (int I=0; I < ItemCount; ++I)
		{
			if (Item[I].UserDataSize > sizeof(Item[I].UserData) && Item[I].UserData)
				xf_free(Item[I].UserData);

			if ((Item[I].Flags&LIF_USETEXTPTR) && Item[I].NamePtr)
				xf_free(Item[I].NamePtr);
		}

		xf_free(Item);
	}

	Item=NULL;
	ItemCount=0;
	ItemHiddenCount=0;
	SelectPos=-1;
	TopPos=0;
	MaxLength=Max((int)strlen(VMenu::Title),(int)strlen(VMenu::BottomTitle))+2;

	if (MaxLength > ScrX-8)
		MaxLength=ScrX-8;

	VMFlags.Set(VMENU_UPDATEREQUIRED);
}


int VMenu::DeleteSelectedItems()
{
	CriticalSectionLock Lock(CS);

	if (Item)
	{
		int I, Cnt;

		for (Cnt=I=0; I < ItemCount; ++I)
			if (Item[I].Flags&LIF_CHECKED)
				Cnt++;

		if (Cnt >= ItemCount)
			DeleteItems();
		else
			for (I=0; I < ItemCount; ++I)
			{
				if (Item[I].Flags&LIF_CHECKED)
				{
					DeleteItem(I);
					I=-1;  // начинаем смотреть с начала, т.к. содержимое массива и ItemCount изменились
				}
			}

		VMFlags.Set(VMENU_UPDATEREQUIRED);
		return Cnt;
	}

	return 0;
}

/* $ 01.08.2000 SVS
   функция удаления N пунктов меню
*/
int VMenu::DeleteItem(int ID,int Count)
{
	CriticalSectionLock Lock(CS);
	int I;

	if (ID < 0 || ID >= ItemCount || Count <= 0)
		return ItemCount;

	if (ID+Count > ItemCount)
		Count=ItemCount-ID; //???

	if (Count <= 0)
		return ItemCount;

	int OldItemSelected=-1;

	for (I=0; I < ItemCount; ++I)
	{
		if (Item[I].Flags & LIF_SELECTED)
			OldItemSelected=I;

		Item [I].Flags&=~LIF_SELECTED;
	}

	if (OldItemSelected >= ID)
	{
		if (ID+Count >= ItemCount)
			OldItemSelected=ID-1;
	}

	if (OldItemSelected < 0)
		OldItemSelected=0;

	// Надобно удалить данные, чтоб потери по памяти не были
	for (I=0; I < Count; ++I)
	{
		struct MenuItem *PtrItem=Item+ID+I;

		if (PtrItem->UserDataSize > sizeof(PtrItem->UserData) && PtrItem->UserData)
			xf_free(PtrItem->UserData);
	}

	// а вот теперь перемещения
	if (ItemCount > 1)
		memmove(Item+ID,Item+ID+Count,sizeof(struct MenuItem)*(ItemCount-(ID+Count))); //???

	// коррекция текущей позиции
	if (SelectPos >= ID && SelectPos < ID+Count)
	{
		SelectPos=ID;

		if (ID+Count == ItemCount)
			SelectPos--;

		/* $ 23.02.2002 DJ
		   постараемся не ставить выделение на сепаратор
		*/
		while (SelectPos > 0 &&(Item [SelectPos].Flags & (LIF_SEPARATOR | LIF_DISABLE | LIF_HIDDEN)))
			SelectPos--;
	}

	VMFlags.Clear(VMENU_SELECTPOSNONE);

	if (SelectPos < 0)
	{
		VMFlags.Set(VMENU_SELECTPOSNONE);
		SelectPos=0;
	}

	ItemCount-=Count;

	if (SelectPos < TopPos || SelectPos > TopPos+Y2-Y1 || TopPos >= ItemCount)
	{
		TopPos=0;
		VMFlags.Set(VMENU_UPDATEREQUIRED);
	}

	// Нужно ли обновить экран?
	if ((ID >= TopPos && ID < TopPos+Y2-Y1) ||
	        (ID+Count >= TopPos && ID+Count < TopPos+Y2-Y1)) //???
	{
		VMFlags.Set(VMENU_UPDATEREQUIRED);
	}

	SelectPos=SetSelectPos(OldItemSelected,1);

	if (Item[SelectPos].Flags & (LIF_SEPARATOR | LIF_DISABLE | LIF_HIDDEN))
		VMFlags.Set(VMENU_SELECTPOSNONE);

	if (SelectPos > -1)
		Item[SelectPos].Flags|=LIF_SELECTED;

	RecalcItemHiddenCount();
	return(ItemCount);
}

int VMenu::RecalcItemHiddenCount()
{
	ItemHiddenCount=0;

	for (int I=0; I < ItemCount; I++)
	{
		if (Item[I].Flags&LIF_HIDDEN)
			ItemHiddenCount++;
	}

	return ItemHiddenCount;
}

int VMenu::AddItem(const struct MenuItem *NewItem,int PosAdd)
{
	CriticalSectionLock Lock(CS);

	if (!NewItem)
		return -1; //???

	struct MenuItem *NewPtr;
	int Length;

	if (PosAdd >= ItemCount)
		PosAdd=ItemCount;

	if (UpdateRequired())
		VMFlags.Set(VMENU_UPDATEREQUIRED);

	if ((ItemCount & 255)==0)
	{
		if ((NewPtr=(struct MenuItem *)xf_realloc(Item,sizeof(struct MenuItem)*(ItemCount+256+1)))==NULL)
			return -1;

		Item=NewPtr;
	}

	// Если < 0 - однозначно ставим в нулевую позицию, т.е добавка сверху
	if (PosAdd < 0)
		PosAdd=0;

	if (PosAdd < ItemCount)
		memmove(Item+PosAdd+1,Item+PosAdd,sizeof(struct MenuItem)*(ItemCount-PosAdd)); //??

	Item[PosAdd]=*NewItem;
	Item[PosAdd].ShowPos=0;

	if (VMFlags.Check(VMENU_SHOWAMPERSAND))
		Length=(int)strlen(Item[PosAdd].PtrName());
	else
		Length=HiStrlen(Item[PosAdd].PtrName());

	if (Length>MaxLength)
		MaxLength=Length;

	if (MaxLength > ScrX-8)
		MaxLength=ScrX-8;

	if (Item[PosAdd].Flags&LIF_SELECTED)
		SelectPos=PosAdd;

	if (Item[PosAdd].Flags&0x0000FFFF)
	{
		Item[PosAdd].Flags|=LIF_CHECKED;

		if ((Item[PosAdd].Flags&0x0000FFFF) == 1)
			Item[PosAdd].Flags&=0xFFFF0000;
	}

	Item[PosAdd].AmpPos=-1;
	VMFlags.Clear(VMENU_SELECTPOSNONE);

	if (SelectPos < 0)
	{
		VMFlags.Set(VMENU_SELECTPOSNONE);
		//SelectPos=0;
	}

	LastAddedItem = PosAdd;
	ItemCount++;
	RecalcItemHiddenCount();
	return ItemCount-1;
}

int VMenu::AddItem(const char *NewStrItem)
{
	CriticalSectionLock Lock(CS);
#if 0
	struct FarList FarList0;
	struct FarListItem FarListItem0;
	memset(&FarListItem0,0,sizeof(FarListItem0));

	if (!NewStrItem || NewStrItem[0] == 0x1)
	{
		FarListItem0.Flags=LIF_SEPARATOR;
		xstrncpy(FarListItem0.Text,NewStrItem+1,sizeof(FarListItem0.Text)-2);
	}
	else
	{
		xstrncpy(FarListItem0.Text,NewStrItem,sizeof(FarListItem0.Text)-1);
	}

	FarList0.ItemsNumber=1;
	FarList0.Items=&FarListItem0;
	return VMenu::AddItem(&FarList0)-1; //-1 потому что AddItem(FarList) возвращает количество элементов
#else
	struct MenuItem NewItem;
	memset(&NewItem,0,sizeof(NewItem));
	size_t LenNewStrItem=strlen(NewStrItem);

	if (!NewStrItem || NewStrItem[0] == 0x1)
		NewItem.Flags=LIF_SEPARATOR;

	if (LenNewStrItem >= sizeof(NewItem.Name))
	{
		NewItem.NamePtr=xf_strdup(NewStrItem+(NewItem.Flags&LIF_SEPARATOR?1:0));
		NewItem.Flags|=LIF_USETEXTPTR;
	}
	else
		xstrncpy(NewItem.Name,NewStrItem+(NewItem.Flags&LIF_SEPARATOR?1:0),sizeof(NewItem.Name)-1);

	return VMenu::AddItem(&NewItem);
#endif
}

int VMenu::AddItem(const struct FarList *List)
{
	CriticalSectionLock Lock(CS);

	if (List && List->Items)
	{
		struct MenuItem MItem;
		struct FarListItem *FItem=List->Items;

		for (int J=0; J < List->ItemsNumber; J++, ++FItem)
			AddItem(FarList2MenuItem(FItem,&MItem));
	}

	return ItemCount;
}

int VMenu::UpdateItem(const struct FarListUpdate *NewItem)
{
	CriticalSectionLock Lock(CS);

	if (NewItem && (DWORD)NewItem->Index < (DWORD)ItemCount)
	{
		struct MenuItem MItem;
		// Освободим память... от ранее занятого ;-)
		struct MenuItem *PItem=Item+NewItem->Index;

		if (PItem->UserDataSize > sizeof(PItem->UserData) && PItem->UserData && (NewItem->Item.Flags&LIF_DELETEUSERDATA))
		{
			xf_free(PItem->UserData);
			PItem->UserData=NULL;
			PItem->UserDataSize=0;
		}

		FarList2MenuItem(&NewItem->Item,&MItem);
		PItem->Flags=MItem.Flags;
		memcpy(PItem->Name,MItem.Name,sizeof(PItem->Name));

		/* $ 23.02.2002 DJ
		   если элемент selected - поставим на него выделение
		*/
		if (PItem->Flags & LIF_SELECTED && !(PItem->Flags & (LIF_SEPARATOR | LIF_DISABLE | LIF_HIDDEN)))
			SelectPos = NewItem->Index;

		AdjustSelectPos();
		RecalcItemHiddenCount();
		return TRUE;
	}

	return FALSE;
}

int VMenu::InsertItem(const struct FarListInsert *NewItem)
{
	CriticalSectionLock Lock(CS);

	if (NewItem)
	{
		struct MenuItem MItem;

		if (AddItem(FarList2MenuItem(&NewItem->Item,&MItem),NewItem->Index) >= 0)
		{
			RecalcItemHiddenCount();
			return ItemCount;
		}
	}

	return -1;
}

int VMenu::GetUserDataSize(int Position)
{
	CriticalSectionLock Lock(CS);

	if (ItemCount==0)
		return(0);

	int DataSize=Item[GetItemPosition(Position)].UserDataSize;
	return(DataSize);
}

int VMenu::_SetUserData(struct MenuItem *PItem,
                        const void *Data,   // Данные
                        int Size)     // Размер, если =0 то предполагается, что в Data-строка
{
	if (PItem->UserDataSize > sizeof(PItem->UserData) && PItem->UserData)
		xf_free(PItem->UserData);

	PItem->UserDataSize=0;
	PItem->UserData=NULL;

	if (Data)
	{
		int SizeReal=Size;

		// Если Size=0, то подразумевается, что в Data находится ASCIIZ строка
		if (!Size)
			SizeReal=(int)strlen((const char*)Data)+1;

		// если размер данных Size=0 или Size больше 4 байт (sizeof(void*))
		if (!Size ||
		        Size > sizeof(PItem->UserData)) // если в 4 байта не влезаем, то...
		{
			// размер больше 4 байт?
			if (SizeReal > sizeof(PItem->UserData))
			{
				// ...значит выделяем нужную память.
				if ((PItem->UserData=(char*)xf_malloc(SizeReal)) != NULL)
				{
					PItem->UserDataSize=SizeReal;
					memcpy(PItem->UserData,Data,SizeReal);
				}
			}
			else // ЭТА СТРОКА ПОМЕЩАЕТСЯ В 4 БАЙТА!
			{
				PItem->UserDataSize=SizeReal;
				memcpy(PItem->Str4,Data,SizeReal);
			}
		}
		else // Ок. данные помещаются в 4 байта...
		{
			PItem->UserDataSize=0;         // признак того, что данных либо нет, либо
			PItem->UserData=(char*)Data;   // они помещаются в 4 байта
		}
	}

	return(PItem->UserDataSize);
}

void* VMenu::_GetUserData(struct MenuItem *PItem,void *Data,int Size)
{
	int DataSize=PItem->UserDataSize;
	char *PtrData=PItem->UserData; // PtrData содержит: либо указатель на что-то либо
	// 4 байта!
	/* $ 12.06.2001 KM
	   - Некорректно работала функция. Забыли, что данные в меню
	     могут быть в простом MenuItem.Name
	*/
	if (Size > 0 && Data != NULL)
	{
		if (PtrData) // данные есть?
		{
			// размерчик больше 4 байт?
			if (DataSize > sizeof(PItem->UserData))
			{
				memmove(Data,PtrData,Min(Size,DataSize));
			}
			else if (DataSize > 0) // а данные то вообще есть? Т.е. если в UserData
			{                     // есть строка из 4 байт (UserDataSize при этом > 0)
				memmove(Data,PItem->Str4,Min(Size,DataSize));
			}

			// else а иначе... в PtrData уже указатель сидит!
		}
		else // ... данных нет, значит лудим имя пункта!
		{
			PtrData=PItem->PtrName();

			if (PItem->Flags&LIF_USETEXTPTR)
				memmove(Data,PtrData,Min(Size,(int)strlen(PtrData)));
			else
				memmove(Data,PItem->Name,Min(Size,(int)sizeof(PItem->Name)));
		}
	}

	return(PtrData);
}

struct FarListItem *VMenu::MenuItem2FarList(const struct MenuItem *MItem,
        struct FarListItem *FItem)
{
	if (FItem && MItem)
	{
		memset(FItem,0,sizeof(struct FarListItem));
		FItem->Flags=MItem->Flags&(~LIF_USETEXTPTR); //??
		xstrncpy(FItem->Text,((struct MenuItem *)MItem)->PtrName(),sizeof(FItem->Text)-1);
//    FItem->AccelKey=MItem->AccelKey;
		//??????????????????
		//   FItem->UserData=MItem->UserData;
		//   FItem->UserDataSize=MItem->UserDataSize;
		//??????????????????
		return FItem;
	}

	return NULL;
}

struct MenuItem *VMenu::FarList2MenuItem(const struct FarListItem *FItem,
        struct MenuItem *MItem)
{
	if (FItem && MItem)
	{
		memset(MItem,0,sizeof(struct MenuItem));
		MItem->Flags=FItem->Flags;
//    MItem->AccelKey=FItem->AccelKey;
		xstrncpy(MItem->Name,FItem->Text,sizeof(MItem->Name)-1);
		MItem->Flags&=~LIF_USETEXTPTR;
		//MItem->Flags|=LIF_DELETEUSERDATA; //???????????????????
		//VMenu::_SetUserData(MItem,FItem->UserData,FItem->UserDataSize); //???
		// А здесь надо вычислять AmpPos????
		return MItem;
	}

	return NULL;
}

// получить позицию курсора и верхнюю позицию итема
int VMenu::GetSelectPos(struct FarListPos *ListPos)
{
	CriticalSectionLock Lock(CS);
	ListPos->SelectPos=GetSelectPos();

	if (VMFlags.Check(VMENU_SELECTPOSNONE))
		ListPos->SelectPos=-1;

	ListPos->TopPos=TopPos;
	return ListPos->SelectPos;
}

void VMenu::SetMaxHeight(int NewMaxHeight)
{
	CriticalSectionLock Lock(CS);
	VMenu::MaxHeight=NewMaxHeight;

	if (MaxLength > ScrX-8) //
		MaxLength=ScrX-8;
}

// установить курсор и верхний итем
int VMenu::SetSelectPos(struct FarListPos *ListPos)
{
	CriticalSectionLock Lock(CS);
	int Ret=SetSelectPos(ListPos->SelectPos,1);
	int OldTopPos=TopPos;

	if (Ret > -1)
	{
		TopPos=ListPos->TopPos;

		if (ListPos->TopPos == -1)
		{
			if (ItemCount < MaxHeight)
				TopPos=0;
			else
			{
				//TopPos=Ret-MaxHeight/2;               //?????????
				TopPos = (ListPos->SelectPos-ListPos->TopPos+1) > MaxHeight?ListPos->TopPos+1:ListPos->TopPos;

				if (TopPos+MaxHeight > ItemCount)
					TopPos=ItemCount-MaxHeight;
			}
		}

		if (TopPos < 0)
			TopPos = 0;
	}

	return Ret;
}

// переместить курсор с учетом Disabled & Separator
int VMenu::SetSelectPos(int Pos,int Direct)
{
	CriticalSectionLock Lock(CS);

	if (!Item || !ItemCount)
		return -1;

	int OrigPos=Pos, Pass=0, I=0;

	do
	{
		/* $ 21.02.2002 DJ
		   в меню без WRAPMODE условие OrigPos == Pos никогда не выполнится =>
		   нужно использовать немного другую логику для выхода из цикла
		*/
		if (Pos<0)
		{
			if (VMFlags.Check(VMENU_WRAPMODE))
				Pos=ItemCount-1;
			else
			{
				Pos=0;
				Pass++;
			}
		}

		if (Pos>=ItemCount)
		{
			if (VMFlags.Check(VMENU_WRAPMODE))
				Pos=0;
			else
			{
				Pos=ItemCount-1;
				Pass++;
			}
		}

		if (!(Item[Pos].Flags&LIF_SEPARATOR) && !(Item[Pos].Flags&(LIF_DISABLE | LIF_HIDDEN)))
			break;

		Pos+=Direct;

		if (Pass)
			return SelectPos;

		if (I>=ItemCount) // круг пройден - ничего не найдено :-(
			Pass++;

		++I;
	}
	while (1);

	/* $ 30.01.2003 KM
	   - Иногда фар падал. Как выяснилось если SelectPos был равен -1.
	*/
	if (SelectPos!=-1)
		Item[SelectPos].Flags&=~LIF_SELECTED;

	Item[Pos].Flags|=LIF_SELECTED;
	SelectPos=Pos;

	if (SelectPos!=-1)
		VMFlags.Clear(VMENU_SELECTPOSNONE);

	/* $ 01.07.2001 KM
	  Дадим знать, что позиция изменилась для перерисовки (диалог
	  иногда не "замечал", что позиция изменилась).
	*/
	VMFlags.Set(VMENU_UPDATEREQUIRED);
	return Pos;
}

/* $ 21.02.2002 DJ
   корректировка текущей позиции (чтобы не было двух выделенных элементов,
   или чтобы выделенный элемент не был сепаратором)
*/

void VMenu::AdjustSelectPos()
{
	CriticalSectionLock Lock(CS);

	if (!Item || !ItemCount)
		return;

	/* $ 20.07.2004 KM
	   Добавим проверку на -1, в противном случае падает меню
	   из Dialog API.
	*/
//  if (SelectPos!=-1)
//  {
	int OldSelectPos = SelectPos;

	// если selection стоит в некорректном месте - сбросим его
	if (SelectPos >= 0 && Item [SelectPos].Flags & (LIF_SEPARATOR | LIF_DISABLE | LIF_HIDDEN))
		SelectPos = -1;

	for (int i=0; i<ItemCount; i++)
	{
		if (Item [i].Flags & (LIF_SEPARATOR | LIF_DISABLE | LIF_HIDDEN))
			Item [i].SetSelect(FALSE);
		else
		{
			if (SelectPos == -1)
			{
				Item [i].SetSelect(TRUE);
				SelectPos = i;
			}
			else if (SelectPos >= 0 && SelectPos != i)
			{
				// если уже есть выделенный элемент - оставим как было
				Item [i].SetSelect(FALSE);
			}
		}
	}

	// если ничего не нашли - оставим как было
	if (SelectPos == -1)
	{
		SelectPos = OldSelectPos;

		if (SelectPos >= 0)
			Item [SelectPos].SetSelect(TRUE);
	}

//  }

	if (SelectPos == -1)
		VMFlags.Set(VMENU_SELECTPOSNONE); //??
	else
		VMFlags.Clear(VMENU_SELECTPOSNONE);
}


void VMenu::SetTitle(const char *Title)
{
	CriticalSectionLock Lock(CS);
	int Length;
	VMFlags.Set(VMENU_UPDATEREQUIRED);
	Title=NullToEmpty(Title);
	xstrncpy(VMenu::Title,Title,sizeof(VMenu::Title)-1);
	Length=(int)strlen(Title)+2;

	if (Length > MaxLength)
		MaxLength=Length;

	if (MaxLength > ScrX-8)
		MaxLength=ScrX-8;

	if (VMFlags.Check(VMENU_CHANGECONSOLETITLE))
	{
		if (*VMenu::Title)
		{
			if (!OldTitle)
				OldTitle=new ConsoleTitle;

			SetFarTitle(VMenu::Title);
		}
		else
		{
			if (OldTitle)
			{
				delete OldTitle;
				OldTitle=NULL;
			}
		}
	}
}


const char *VMenu::GetTitle(char *Dest,int Size,int /*TruncSize*/)
{
	CriticalSectionLock Lock(CS);

	if (Dest /*&& *VMenu::Title*/)
		return xstrncpy(Dest,VMenu::Title,Size-1);

	return NULL;
}


void VMenu::SetBottomTitle(const char *BottomTitle)
{
	CriticalSectionLock Lock(CS);
	int Length;
	VMFlags.Set(VMENU_UPDATEREQUIRED);
	BottomTitle=NullToEmpty(BottomTitle);
	xstrncpy(VMenu::BottomTitle,BottomTitle,sizeof(VMenu::BottomTitle)-1);
	Length=(int)strlen(BottomTitle)+2;

	if (Length > MaxLength)
		MaxLength=Length;

	if (MaxLength > ScrX-8)
		MaxLength=ScrX-8;
}


char *VMenu::GetBottomTitle(char *Dest,int Size)
{
	CriticalSectionLock Lock(CS);

	if (Dest && *VMenu::BottomTitle)
		return xstrncpy(Dest,VMenu::BottomTitle,Size-1);

	return NULL;
}


void VMenu::SetBoxType(int BoxType)
{
	CriticalSectionLock Lock(CS);
	VMenu::BoxType=BoxType;
}

int VMenu::GetItemPosition(int Position)
{
	CriticalSectionLock Lock(CS);
	int DataPos=(Position==-1) ? SelectPos : Position;

	if (DataPos>=ItemCount)
		DataPos=ItemCount-1;

	return DataPos;
}


int VMenu::GetSelection(int Position)
{
	CriticalSectionLock Lock(CS);

	if (ItemCount==0)
		return(0);

	int DataPos=GetItemPosition(Position);

	if (Item[DataPos].Flags&LIF_SEPARATOR)
		return(0);

	int Checked=Item[DataPos].Flags&0xFFFF;
	return((Item[DataPos].Flags&LIF_CHECKED)?(Checked?Checked:1):0);
}


void VMenu::SetSelection(int Selection,int Position)
{
	CriticalSectionLock Lock(CS);

	if (ItemCount==0)
		return;

	Item[GetItemPosition(Position)].SetCheck(Selection);
}

// Функция GetItemPtr - получить указатель на нужный Item.
struct MenuItem *VMenu::GetItemPtr(int Position)
{
	CriticalSectionLock Lock(CS);

	if (ItemCount==0)
		return NULL;

	return Item+GetItemPosition(Position);
}

char VMenu::GetHighlights(const struct MenuItem *_item)
{
	CriticalSectionLock Lock(CS);
	char Ch=0;

	if (_item)
	{
		const char *Name=((struct MenuItem *)_item)->PtrName();
		const char *ChPtr=strchr(Name,'&');

		if (ChPtr || _item->AmpPos > -1)
		{
			if (!ChPtr && _item->AmpPos > -1)
			{
				ChPtr=Name+_item->AmpPos;
				Ch=*ChPtr;
			}
			else
				Ch=ChPtr[1];

			if (VMFlags.Check(VMENU_SHOWAMPERSAND))
			{
				ChPtr=strchr(ChPtr+1,'&');

				if (ChPtr)
					Ch=ChPtr[1];
			}
		}
	}

	return Ch;
}

int VMenu::CheckHighlights(BYTE CheckSymbol,int StartPos)
{
	CriticalSectionLock Lock(CS);

	if (StartPos < 0)
		StartPos=0;

	for (int I=StartPos; I < ItemCount; I++)
	{
		if (Item[I].Flags&LIF_HIDDEN) //???
			continue;

		char Ch=GetHighlights(Item+I);

		if (Ch)
		{
			if (LocalUpper(CheckSymbol) == LocalUpper(Ch))
				return I;
		}
		else if (!CheckSymbol)
			return I;
	}

	return -1;
}

void VMenu::AssignHighlights(int Reverse)
{
	CriticalSectionLock Lock(CS);
	BYTE Used[256];
	memset(Used,0,sizeof(Used));

	/* $ 02.12.2001 KM
	   + Поелику VMENU_SHOWAMPERSAND сбрасывается для корректной
	     работы ShowMenu сделаем сохранение энтого флага, в противном
	     случае если в диалоге использовался DI_LISTBOX без флага
	     DIF_LISTNOAMPERSAND, то амперсанды отображались в списке
	     только один раз до следующего ShowMenu.
	*/
	if (VMFlags.Check(VMENU_SHOWAMPERSAND))
		VMOldFlags.Set(VMENU_SHOWAMPERSAND);

	if (VMOldFlags.Check(VMENU_SHOWAMPERSAND))
		VMFlags.Set(VMENU_SHOWAMPERSAND);

	/* KM $ */
	int I, Delta=Reverse ? -1:1;

	for (I=(Reverse ? ItemCount-1:0); I >= 0 && I < ItemCount; I+=Delta)
	{
		char Ch=0;
		const char *Name=Item[I].PtrName();
		const char *ChPtr=strchr(Name,'&');
		// TODO: проверка на LIF_HIDDEN
		Item[I].AmpPos=-1;

		if (ChPtr)
		{
			Ch=ChPtr[1];

			if (VMFlags.Check(VMENU_SHOWAMPERSAND))
			{
				ChPtr=strchr(ChPtr+1,'&');

				if (ChPtr)
					Ch=ChPtr[1];
			}
		}

		if (Ch && !Used[LocalUpper(Ch)] && !Used[LocalLower(Ch)])
		{
			Used[LocalUpper(Ch)]++;
			Used[LocalLower(Ch)]++;
			Item[I].AmpPos=(int)(ChPtr-Name);
		}
	}

//_SVS(SysLogDump("Used Pre",0,Used,sizeof(Used),NULL));

	// TODO:  ЭТОТ цикл нужно уточнить - возможно вылезут артефакты (хотя не уверен)
	for (I=Reverse ? ItemCount-1:0; I>=0 && I<ItemCount; I+=Reverse ? -1:1)
	{
		const char *Name=Item[I].PtrName();
		const char *ChPtr=strchr(Name,'&');

		if (ChPtr==NULL || VMFlags.Check(VMENU_SHOWAMPERSAND))
		{
			for (int J=0; Name[J]; J++)
			{
				// TODO: проверка на LIF_HIDDEN
				char Ch=Name[J];

				if ((Ch =='&' || LocalIsalpha(Ch) || (Ch >= '0' && Ch <='9')) &&
				        !Used[LocalUpper(Ch)] && !Used[LocalLower(Ch)])
				{
					Used[LocalUpper(Ch)]++;
					Used[LocalLower(Ch)]++;
					Item[I].AmpPos=J;
					break;
				}
			}
		}
	}

//_SVS(SysLogDump("Used Post",0,Used,sizeof(Used),NULL));
	VMFlags.Set(VMENU_AUTOHIGHLIGHT|(Reverse?VMENU_REVERSEHIGHLIGHT:0));
	VMFlags.Clear(VMENU_SHOWAMPERSAND);
}

void VMenu::SetColors(struct FarListColors *Colors)
{
	CriticalSectionLock Lock(CS);

	if (Colors)
		memmove(VMenu::Colors,Colors->Colors,sizeof(VMenu::Colors));
	else
	{
		static short StdColor[2][3][VMENU_COLOR_COUNT]=
		{
			// Not VMENU_WARNDIALOG
			{
				{ // VMENU_LISTBOX
					COL_DIALOGLISTTEXT,                        // подложка
					COL_DIALOGLISTBOX,                         // рамка
					COL_DIALOGLISTTITLE,                       // заголовок - верхний и нижний
					COL_DIALOGLISTTEXT,                        // Текст пункта
					COL_DIALOGLISTHIGHLIGHT,                   // HotKey
					COL_DIALOGLISTBOX,                         // separator
					COL_DIALOGLISTSELECTEDTEXT,                // Выбранный
					COL_DIALOGLISTSELECTEDHIGHLIGHT,           // Выбранный - HotKey
					COL_DIALOGLISTSCROLLBAR,                   // ScrollBar
					COL_DIALOGLISTDISABLED,                    // Disabled
					COL_DIALOGLISTARROWS,                      // Arrow
					COL_DIALOGLISTARROWSSELECTED,              // Выбранный - Arrow
					COL_DIALOGLISTARROWSDISABLED,              // Arrow Disabled
					COL_DIALOGLISTGRAY,                        // "серый"
					COL_DIALOGLISTSELECTEDGRAYTEXT,            // выбранный "серый"
				},
				{ // VMENU_COMBOBOX
					COL_DIALOGCOMBOTEXT,                       // подложка
					COL_DIALOGCOMBOBOX,                        // рамка
					COL_DIALOGCOMBOTITLE,                      // заголовок - верхний и нижний
					COL_DIALOGCOMBOTEXT,                       // Текст пункта
					COL_DIALOGCOMBOHIGHLIGHT,                  // HotKey
					COL_DIALOGCOMBOBOX,                        // separator
					COL_DIALOGCOMBOSELECTEDTEXT,               // Выбранный
					COL_DIALOGCOMBOSELECTEDHIGHLIGHT,          // Выбранный - HotKey
					COL_DIALOGCOMBOSCROLLBAR,                  // ScrollBar
					COL_DIALOGCOMBODISABLED,                   // Disabled
					COL_DIALOGCOMBOARROWS,                     // Arrow
					COL_DIALOGCOMBOARROWSSELECTED,             // Выбранный - Arrow
					COL_DIALOGCOMBOARROWSDISABLED,             // Arrow Disabled
					COL_DIALOGCOMBOGRAY,                       // "серый"
					COL_DIALOGCOMBOSELECTEDGRAYTEXT,           // выбранный "серый"
				},
				{ // VMenu
					COL_MENUBOX,                               // подложка
					COL_MENUBOX,                               // рамка
					COL_MENUTITLE,                             // заголовок - верхний и нижний
					COL_MENUTEXT,                              // Текст пункта
					COL_MENUHIGHLIGHT,                         // HotKey
					COL_MENUBOX,                               // separator
					COL_MENUSELECTEDTEXT,                      // Выбранный
					COL_MENUSELECTEDHIGHLIGHT,                 // Выбранный - HotKey
					COL_MENUSCROLLBAR,                         // ScrollBar
					COL_MENUDISABLEDTEXT,                      // Disabled
					COL_MENUARROWS,                            // Arrow
					COL_MENUARROWSSELECTED,                    // Выбранный - Arrow
					COL_MENUARROWSDISABLED,                    // Arrow Disabled
					COL_MENUGRAYTEXT,                          // "серый"
					COL_MENUSELECTEDGRAYTEXT,                  // выбранный "серый"
				}
			},

			// == VMENU_WARNDIALOG
			{
				{ // VMENU_LISTBOX
					COL_WARNDIALOGLISTTEXT,                    // подложка
					COL_WARNDIALOGLISTBOX,                     // рамка
					COL_WARNDIALOGLISTTITLE,                   // заголовок - верхний и нижний
					COL_WARNDIALOGLISTTEXT,                    // Текст пункта
					COL_WARNDIALOGLISTHIGHLIGHT,               // HotKey
					COL_WARNDIALOGLISTBOX,                     // separator
					COL_WARNDIALOGLISTSELECTEDTEXT,            // Выбранный
					COL_WARNDIALOGLISTSELECTEDHIGHLIGHT,       // Выбранный - HotKey
					COL_WARNDIALOGLISTSCROLLBAR,               // ScrollBar
					COL_WARNDIALOGLISTDISABLED,                // Disabled
					COL_WARNDIALOGLISTARROWS,                  // Arrow
					COL_WARNDIALOGLISTARROWSSELECTED,          // Выбранный - Arrow
					COL_WARNDIALOGLISTARROWSDISABLED,          // Arrow Disabled
					COL_WARNDIALOGLISTGRAY,                    // "серый"
					COL_WARNDIALOGLISTSELECTEDGRAYTEXT,        // выбранный "серый"
				},
				{ // VMENU_COMBOBOX
					COL_WARNDIALOGCOMBOTEXT,                   // подложка
					COL_WARNDIALOGCOMBOBOX,                    // рамка
					COL_WARNDIALOGCOMBOTITLE,                  // заголовок - верхний и нижний
					COL_WARNDIALOGCOMBOTEXT,                   // Текст пункта
					COL_WARNDIALOGCOMBOHIGHLIGHT,              // HotKey
					COL_WARNDIALOGCOMBOBOX,                    // separator
					COL_WARNDIALOGCOMBOSELECTEDTEXT,           // Выбранный
					COL_WARNDIALOGCOMBOSELECTEDHIGHLIGHT,      // Выбранный - HotKey
					COL_WARNDIALOGCOMBOSCROLLBAR,              // ScrollBar
					COL_WARNDIALOGCOMBODISABLED,               // Disabled
					COL_WARNDIALOGCOMBOARROWS,                 // Arrow
					COL_WARNDIALOGCOMBOARROWSSELECTED,         // Выбранный - Arrow
					COL_WARNDIALOGCOMBOARROWSDISABLED,         // Arrow Disabled
					COL_WARNDIALOGCOMBOGRAY,                   // "серый"
					COL_WARNDIALOGCOMBOSELECTEDGRAYTEXT,       // выбранный "серый"
				},
				{ // VMenu
					COL_MENUBOX,                               // подложка
					COL_MENUBOX,                               // рамка
					COL_MENUTITLE,                             // заголовок - верхний и нижний
					COL_MENUTEXT,                              // Текст пункта
					COL_MENUHIGHLIGHT,                         // HotKey
					COL_MENUBOX,                               // separator
					COL_MENUSELECTEDTEXT,                      // Выбранный
					COL_MENUSELECTEDHIGHLIGHT,                 // Выбранный - HotKey
					COL_MENUSCROLLBAR,                         // ScrollBar
					COL_MENUDISABLEDTEXT,                      // Disabled
					COL_MENUARROWS,                            // Arrow
					COL_MENUARROWSSELECTED,                    // Выбранный - Arrow
					COL_MENUARROWSDISABLED,                    // Arrow Disabled
					COL_MENUGRAYTEXT,                          // "серый"
					COL_MENUSELECTEDGRAYTEXT,                  // выбранный "серый"
				}
			}
		};
		int TypeMenu  = CheckFlags(VMENU_LISTBOX)?0:(CheckFlags(VMENU_COMBOBOX)?1:2);
		int StyleMenu = CheckFlags(VMENU_WARNDIALOG)?1:0;

		if (CheckFlags(VMENU_DISABLED))
		{
			VMenu::Colors[0]=FarColorToReal(StyleMenu?COL_WARNDIALOGDISABLED:COL_DIALOGDISABLED);

			for (int I=1; I < VMENU_COLOR_COUNT; ++I)
				VMenu::Colors[I]=VMenu::Colors[0];
		}
		else
		{
			for (int I=0; I < VMENU_COLOR_COUNT; ++I)
				VMenu::Colors[I]=FarColorToReal(StdColor[StyleMenu][TypeMenu][I]);
		}
	}
}

void VMenu::GetColors(struct FarListColors *Colors)
{
	CriticalSectionLock Lock(CS);
	memmove(Colors->Colors,VMenu::Colors,sizeof(VMenu::Colors));
}

/* $ 25.05.2001 DJ
   установка одного цвета
*/
void VMenu::SetOneColor(int Index, short Color)
{
	CriticalSectionLock Lock(CS);

	if ((DWORD)Index < sizeof(Colors) / sizeof(Colors [0]))
		Colors [Index]=FarColorToReal(Color);
}

struct SortItemParam
{
	int Direction;
	int Offset;
};

static int __cdecl  SortItem(const struct MenuItem *el1,
                             const struct MenuItem *el2,
                             const struct SortItemParam *Param)
{
	char Name1[2*NM],Name2[2*NM];
	xstrncpy(Name1,((struct MenuItem *)el1)->PtrName(),sizeof(Name1));
	RemoveChar(Name1,'&',TRUE);
	xstrncpy(Name2,((struct MenuItem *)el2)->PtrName(),sizeof(Name2));
	RemoveChar(Name2,'&',TRUE);
	int Res=LocalStricmp(Name1+Param->Offset,Name2+Param->Offset);
	return(Param->Direction==0?Res:(Res<0?1:(Res>0?-1:0)));
}

static int __cdecl  SortItemDataDWORD(const struct MenuItem *el1,
                                      const struct MenuItem *el2,
                                      const struct SortItemParam *Param)
{
	int Res;
	DWORD Dw1=(DWORD)(DWORD_PTR)(((struct MenuItem *)el1)->UserData);
	DWORD Dw2=(DWORD)(DWORD_PTR)(((struct MenuItem *)el2)->UserData);

	if (Dw1 == Dw2)
		Res=0;
	else if (Dw1 > Dw2)
		Res=1;
	else
		Res=-1;

	return(Param->Direction==0?Res:(Res<0?1:(Res>0?-1:0)));
}

// Сортировка элементов списка
// Offset - начало сравнения! по умолчанию =0
void VMenu::SortItems(int Direction,int Offset,BOOL SortForDataDWORD)
{
	CriticalSectionLock Lock(CS);
	typedef int (__cdecl *qsortex_fn)(const void*,const void*,void*);
	struct SortItemParam Param;
	Param.Direction=Direction;
	Param.Offset=Offset;
	int I;

	//_SVS(for(I=0; I < ItemCount; ++I)SysLog("%2d) 0x%08X - '%s'",I,Item[I].Flags,Item[I].Name));
	if (!SortForDataDWORD) // обычная сортировка
		qsortex((char *)Item,
		        ItemCount,
		        sizeof(struct MenuItem),
		        (qsortex_fn)SortItem,
		        &Param);
	else
		qsortex((char *)Item,
		        ItemCount,
		        sizeof(struct MenuItem),
		        (qsortex_fn)SortItemDataDWORD,
		        &Param);

	//_SVS(for(I=0; I < ItemCount; ++I)SysLog("%2d) 0x%08X - '%s'",I,Item[I].Flags,Item[I].Name));

	// скорректируем SelectPos
	for (I=0; I < ItemCount; ++I)
		if (Item[I].Flags & LIF_SELECTED && !(Item[I].Flags & (LIF_SEPARATOR | LIF_DISABLE | LIF_HIDDEN)))
		{
			SelectPos=I;
			break;
		}

	VMFlags.Set(VMENU_UPDATEREQUIRED);
}

// return Pos || -1
int VMenu::FindItem(const struct FarListFind *FItem)
{
	return FindItem(FItem->StartIndex,FItem->Pattern,FItem->Flags);
}

int VMenu::FindItem(int StartIndex,const char *Pattern,DWORD Flags)
{
	CriticalSectionLock Lock(CS);
	char TmpBuf[130];

	if ((DWORD)StartIndex < (DWORD)ItemCount)
	{
		const char *NamePtr;
		int LenPattern=(int)strlen(Pattern);

		for (int I=StartIndex; I < ItemCount; I++)
		{
			NamePtr=Item[I].PtrName();
			int LenNamePtr=(int)strlen(NamePtr);
			memcpy(TmpBuf,NamePtr,Min((int)LenNamePtr+1,(int)sizeof(TmpBuf)));

			if (Flags&LIFIND_EXACTMATCH)
			{
				if (!LocalStrnicmp(RemoveChar(TmpBuf,'&'),Pattern,Max(LenPattern,LenNamePtr)))
					return I;
			}
			else
			{
				if (CmpName(Pattern,RemoveChar(TmpBuf,'&'),1))
					return I;
			}
		}
	}

	return -1;
}

BOOL VMenu::GetVMenuInfo(struct FarListInfo* Info)
{
	CriticalSectionLock Lock(CS);

	if (Info)
	{
		/* $ 23.02.2002 DJ
		   нефиг показывать наши внутренние флаги
		*/
		Info->Flags=VMFlags.Flags & (LINFO_SHOWNOBOX | LINFO_AUTOHIGHLIGHT
		                             | LINFO_REVERSEHIGHLIGHT | LINFO_WRAPMODE | LINFO_SHOWAMPERSAND);
		Info->ItemsNumber=ItemCount;
		Info->SelectPos=SelectPos;
		Info->TopPos=TopPos;
		Info->MaxHeight=MaxHeight;
		Info->MaxLength=MaxLength;
		memset(&Info->Reserved,0,sizeof(Info->Reserved));
		return TRUE;
	}

	return FALSE;
}

// Присовокупить к итему данные.
int VMenu::SetUserData(void *Data,   // Данные
                       int Size,     // Размер, если =0 то предполагается, что в Data-строка
                       int Position) // номер итема
{
	CriticalSectionLock Lock(CS);

	if (ItemCount==0 || Position < 0)
		return(0);

	int DataSize=VMenu::_SetUserData(Item+GetItemPosition(Position),Data,Size);
	return DataSize;
}

// Получить данные
void* VMenu::GetUserData(void *Data,int Size,int Position)
{
	CriticalSectionLock Lock(CS);
	void *PtrData=NULL;

	if (ItemCount || Position < 0)
	{
		if ((Position=GetItemPosition(Position)) >= 0)
			PtrData=VMenu::_GetUserData(Item+Position,Data,Size);
	}

	return(PtrData);
}

void VMenu::Process()
{
	Modal::Process();
}

void VMenu::ResizeConsole()
{
	CriticalSectionLock Lock(CS);

	/* $ 13.04.2002 KM
	  - Добавим проверку на существование буфера сохранения,
	    т.к. ResizeConsole вызывается теперь для всех экземпляров
	    VMenu при AltF9.
	*/
	if (SaveScr)
	{
		SaveScr->Discard();
		delete SaveScr;
		SaveScr=NULL;
	}

	if (this->CheckFlags(VMENU_NOTCHANGE))
	{
		return;
	}

	ObjWidth=ObjHeight=0;

	if (!this->CheckFlags(VMENU_NOTCENTER))
	{
		Y2=X2=Y1=X1=-1;
	}
	else
	{
		X1=5;

		if (!this->CheckFlags(VMENU_LEFTMOST) && ScrX>40)
		{
			X1=(ScrX+1)/2+5;
		}

		Y1=(ScrY+1-(this->ItemCount+5))/2;

		if (Y1<1) Y1=1;

		X2=Y2=0;
	}
}

int VMenu::GetTypeAndName(char *Type,char *Name)
{
	CriticalSectionLock Lock(CS);

	if (Type)
		strcpy(Type,MSG(MVMenuType));

	if (Name)
		strcpy(Name,Title);

	return(CheckFlags(VMENU_COMBOBOX)?MODALTYPE_COMBOBOX:MODALTYPE_VMENU);
}


#ifndef _MSC_VER
#pragma warn -par
#endif
// функция обработки меню (по умолчанию)
LONG_PTR WINAPI VMenu::DefMenuProc(HANDLE hVMenu,int Msg,int Param1,LONG_PTR Param2)
{
	return 0;
}
#ifndef _MSC_VER
#pragma warn +par
#endif

#ifndef _MSC_VER
#pragma warn -par
#endif
// функция посылки сообщений меню
LONG_PTR WINAPI VMenu::SendMenuMessage(HANDLE hVMenu,int Msg,int Param1,LONG_PTR Param2)
{
	CriticalSectionLock Lock(((VMenu*)hVMenu)->CS);

	if (hVMenu)
		return ((VMenu*)hVMenu)->VMenuProc(hVMenu,Msg,Param1,Param2);

	return 0;
}
#ifndef _MSC_VER
#pragma warn +par
#endif

char MenuItem::operator[](int Pos) const
{
	if (Flags&LIF_USETEXTPTR)
		return (!NamePtr || static_cast<size_t>(Pos) > strlen(NamePtr))?0:NamePtr[Pos];

	return (static_cast<size_t>(Pos) > strlen(Name))?0:Name[Pos];
}

char* MenuItem::PtrName()
{
	return ((Flags&LIF_USETEXTPTR)!=0&&NamePtr)?NamePtr:Name;
}
