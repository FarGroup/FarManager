/*
edit.cpp

Реализация одиночной строки редактирования

*/

#include "headers.hpp"
#pragma hdrstop

#include "edit.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "plugin.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "editor.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "filelist.hpp"
#include "panel.hpp"
#include "scrbuf.hpp"

static int EditEncodeDisabled=0;
static int Recurse=0;

enum {EOL_NONE,EOL_CR,EOL_LF,EOL_CRLF,EOL_CRCRLF};
static const char *EOL_TYPE_CHARS[]={"","\r","\n","\r\n","\r\r\n"};

// Идентификаторы масок
#define EDMASK_ANY   'X' // позволяет вводить в строку ввода любой символ;
#define EDMASK_DSS   '#' // позволяет вводить в строку ввода цифры, пробел и знак минуса;
#define EDMASK_DIGIT '9' // позволяет вводить в строку ввода только цифры;
#define EDMASK_ALPHA 'A' // позволяет вводить в строку ввода только буквы.
#define EDMASK_HEX   'H' // позволяет вводить в строку ввода шестнадцатиричные символы.

Edit::Edit(ScreenObject *pOwner)
{
	SetOwner(pOwner);
	m_next = NULL;
	m_prev = NULL;
	Str=(char*) xf_malloc(sizeof(char));
	StrSize=0;
	WordDiv=Opt.WordDiv;
	*Str=0;
	// $ 12.08.2000 KM - Установим маску ввода и предыдущее положение курсора
	Mask=NULL;
	PrevCurPos=0;
	CurPos=0;
	CursorPos=0;
	CursorSize=-1;
	TableSet=NULL;
	LeftPos=0;
	MaxLength=-1;
	MSelStart=-1;
	SelStart=-1;
	SelEnd=0;
	Flags.Set(FEDITLINE_EDITBEYONDEND);
	Color=F_LIGHTGRAY|B_BLACK;
	SelColor=F_WHITE|B_BLACK;
	ColorUnChanged=COL_DIALOGEDITUNCHANGED;
	EndType=EOL_NONE;
	ColorList=NULL;
	ColorCount=0;
	TabSize=Opt.EdOpt.TabSize; // $ 21.02.2001 IS - Размер табуляции по умолчанию равен Opt.EdOpt.TabSize;
//  TabExpandMode = Opt.EdOpt.ExpandTabs;
	TabExpandMode = EXPAND_NOTABS;
	Flags.Change(FEDITLINE_DELREMOVESBLOCKS,Opt.EdOpt.DelRemovesBlocks);
	Flags.Change(FEDITLINE_PERSISTENTBLOCKS,Opt.EdOpt.PersistentBlocks);
}


Edit::~Edit()
{
	if (ColorList)
		xf_free(ColorList);

	if (Mask)
		xf_free(Mask);

	if (Str)
		xf_free(Str);
}


void Edit::DisplayObject()
{
	/* $ 26.07.2000 tran
	  + dropdown style */
	if (Flags.Check(FEDITLINE_DROPDOWNBOX))
	{
		Flags.Clear(FEDITLINE_CLEARFLAG);  // при дроп-даун нам не нужно никакого unchanged text
		SelStart=0;
		SelEnd=StrSize; // а также считаем что все выделено -
		//    надо же отличаться от обычных Edit
	}

	/* tran 26.07.2000 $ */
	/* $ 12.08.2000 KM
	   Вычисление нового положения курсора в строке с учётом Mask.
	*/
	int Value=(PrevCurPos>CurPos)?-1:1;
	CurPos=GetNextCursorPos(CurPos,Value);
	/* KM $ */
	FastShow();

	//if (EditOutDisabled)
	//  return;
	/* $ 19.07.2001 KM
	   - Под NT курсор мигал.
	*/
	/* $ 16.07.2001 KM
	   - Борьба через ж*пу с глюком консоли w9x где при запуске
	     некоторых досовых прог курсор приобретал "странный"
	     внешний вид.
	*/
	if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
	{
		::SetCursorType(TRUE,99);
		::SetCursorType(TRUE,CursorSize);
	}

	/* KM $ */
	/* KM $ */

	/* $ 26.07.2000 tran
	   при DropDownBox курсор выключаем
	   не знаю даже - попробовал но не очень красиво вышло */
	if (Flags.Check(FEDITLINE_DROPDOWNBOX))
		::SetCursorType(0,10);
	else
	{
		if (Flags.Check(FEDITLINE_OVERTYPE))
		{
			int NewCursorSize=IsWindowed()?
			                  (Opt.CursorSize[2]?Opt.CursorSize[2]:99):
					                  (Opt.CursorSize[3]?Opt.CursorSize[3]:99);
			::SetCursorType(1,CursorSize==-1?NewCursorSize:CursorSize);
		}
		else
{
			int NewCursorSize=IsWindowed()?
			                  (Opt.CursorSize[0]?Opt.CursorSize[0]:10):
					                  (Opt.CursorSize[1]?Opt.CursorSize[1]:10);
			::SetCursorType(1,CursorSize==-1?NewCursorSize:CursorSize);
		}
	}

	MoveCursor(X1+CursorPos-LeftPos,Y1);
}


void Edit::SetCursorType(int Visible,int Size)
{
	Flags.Change(FEDITLINE_CURSORVISIBLE,Visible);
	CursorSize=Size;
	::SetCursorType(Visible,Size);
}

void Edit::GetCursorType(int &Visible,int &Size)
{
	Visible=Flags.Check(FEDITLINE_CURSORVISIBLE);
	Size=CursorSize;
}

/* $ 12.08.2000 KM
   Вычисление нового положения курсора в строке с учётом Mask.
*/
int Edit::GetNextCursorPos(int Position,int Where)
{
	int Result=Position;

	if (Mask && *Mask && (Where==-1 || Where==1))
	{
		int i;
		int PosChanged=FALSE;
		int MaskLen=(int)strlen(Mask);

		for (i=Position; i<MaskLen && i>=0; i+=Where)
		{
			if (CheckCharMask(Mask[i]))
			{
				Result=i;
				PosChanged=TRUE;
				break;
			}
		}

		if (!PosChanged)
		{
			for (i=Position; i>=0; i--)
			{
				if (CheckCharMask(Mask[i]))
				{
					Result=i;
					PosChanged=TRUE;
					break;
				}
			}
		}

		if (!PosChanged)
		{
			for (i=Position; i<MaskLen; i++)
			{
				if (CheckCharMask(Mask[i]))
				{
					Result=i;
					break;
				}
			}
		}
	}

	return Result;
}
/* KM $ */

void Edit::FastShow()
{
	int EditLength=ObjWidth;

	if (!Flags.Check(FEDITLINE_EDITBEYONDEND) && CurPos>StrSize && StrSize>=0)
		CurPos=StrSize;

	if (MaxLength!=-1)
	{
		if (StrSize>MaxLength)
		{
			Str[MaxLength]=0;
			StrSize=MaxLength;
		}

		if (CurPos>MaxLength-1)
			CurPos=MaxLength>0 ? (MaxLength-1):0;
	}

	int TabCurPos=GetTabCurPos();

	/* $ 31.07.2001 KM
	  ! Для комбобокса сделаем отображение строки
	    с первой позиции.
	*/
	if (!Flags.Check(FEDITLINE_DROPDOWNBOX))
	{
		if (TabCurPos-LeftPos>EditLength-1)
			LeftPos=TabCurPos-EditLength+1;

		if (TabCurPos<LeftPos)
			LeftPos=TabCurPos;
	}

	/* KM $ */
	GotoXY(X1,Y1);
	int TabSelStart=(SelStart==-1) ? -1:RealPosToTab(SelStart);
	int TabSelEnd=(SelEnd<0) ? -1:RealPosToTab(SelEnd);

	/* $ 17.08.2000 KM
	   Если есть маска, сделаем подготовку строки, то есть
	   все "постоянные" символы в маске, не являющиеся шаблонными
	   должны постоянно присутствовать в Str
	*/
	if (Mask && *Mask)
		RefreshStrByMask();

	/* KM $ */
	char *OutStrTmp=new char[EditLength+1];

	if (!OutStrTmp)
		return;

	char *OutStr=new char[EditLength+1];

	if (!OutStr)
	{
		delete[] OutStrTmp;
		return;
	}

	CursorPos=TabCurPos;
	int RealLeftPos=TabPosToReal(LeftPos);
	int OutStrLength=Min(EditLength,StrSize-RealLeftPos);

	if (OutStrLength < 0)
	{
		OutStrLength=0;
	}
	else
		memcpy(OutStrTmp,Str+RealLeftPos,OutStrLength);

	if (TableSet)
		DecodeString(OutStrTmp,(unsigned char*)TableSet->DecodeTable,OutStrLength);

	{
		char *p=OutStrTmp;
		char *e=OutStrTmp+OutStrLength;

		for (OutStrLength=0; OutStrLength<EditLength && p<e; p++)
		{
			if (*p == '\t')
			{
				int S=TabSize-((LeftPos+OutStrLength) % TabSize);

				for (int i=0; i<S && OutStrLength<EditLength; i++,OutStrLength++)
					OutStr[OutStrLength]=' ';
			}
			else
			{
				if (*p == 0)
					OutStr[OutStrLength]=' ';
				else
					OutStr[OutStrLength]=*p;

				OutStrLength++;
			}
		}

		if (Flags.Check(FEDITLINE_PASSWORDMODE))
			memset(OutStr,'*',OutStrLength);
	}
	OutStr[OutStrLength]=0;
	SetColor(Color);

	if (TabSelStart==-1)
	{
		if (Flags.Check(FEDITLINE_CLEARFLAG))
		{
			SetColor(ColorUnChanged);

			if (Mask && *Mask)
				OutStrLength=(int)strlen(RemoveTrailingSpaces(OutStr));

			mprintf("%-*.*s",OutStrLength,OutStrLength,OutStr);
			SetColor(Color);
			int BlankLength=EditLength-OutStrLength;

			if (BlankLength > 0)
				mprintf("%*s",BlankLength,"");
		}
		else
			mprintf("%-*.*s",EditLength,EditLength,OutStr);
	}
	else
	{
		if ((TabSelStart-=LeftPos)<0)
			TabSelStart=0;

		int AllString=(TabSelEnd==-1);

		if (AllString)
			TabSelEnd=EditLength;
		else if ((TabSelEnd-=LeftPos)<0)
			TabSelEnd=0;

		memset(OutStr+OutStrLength,' ',EditLength-OutStrLength);
		OutStr[EditLength]=0;

		/* $ 24.08.2000 SVS
		   ! У DropDowList`а выделение по полной программе - на всю видимую длину
		     ДАЖЕ ЕСЛИ ПУСТАЯ СТРОКА
		*/
		if (TabSelStart>=EditLength /*|| !AllString && TabSelStart>=StrSize*/ ||
		        TabSelEnd<TabSelStart)
		{
			if (Flags.Check(FEDITLINE_DROPDOWNBOX))
			{
				SetColor(SelColor);
				mprintf("%*s",X2-X1+1,OutStr);
			}
			else
				Text(OutStr);
		}
		/* SVS $ */
		else
		{
			mprintf("%.*s",TabSelStart,OutStr);
			SetColor(SelColor);

			/* $ 15.08.2000 SVS
			   + У DropDowList`а выделение по полной программе - на всю видимую длину
			*/
			if (!Flags.Check(FEDITLINE_DROPDOWNBOX))
			{
				mprintf("%.*s",TabSelEnd-TabSelStart,OutStr+TabSelStart);

				if (TabSelEnd<EditLength)
				{
					//SetColor(Flags.Check(FEDITLINE_CLEARFLAG) ? SelColor:Color);
					SetColor(Color);
					Text(OutStr+TabSelEnd);
				}
			}
			else
			{
				mprintf("%*s",X2-X1+1,OutStr);
			}

			/* SVS $*/
		}
	}

	delete[] OutStr;
	delete[] OutStrTmp;

	/* $ 26.07.2000 tran
	   при дроп-даун цвета нам не нужны */
	if (!Flags.Check(FEDITLINE_DROPDOWNBOX))
		ApplyColor();

	/* tran 26.07.2000 $ */
}


int Edit::RecurseProcessKey(int Key)
{
	Recurse++;
	int RetCode=ProcessKey(Key);
	Recurse--;
	return(RetCode);
}


// Функция вставки всякой хреновени - от шорткатов до имен файлов
int Edit::ProcessInsPath(int Key,int PrevSelStart,int PrevSelEnd)
{
	int RetCode=FALSE;
	char *PathName;

	if (Key>=KEY_RCTRL0 && Key<=KEY_RCTRL9) // шорткаты?
	{
		char PluginModule[NM],PluginFile[NM],PluginData[MAXSIZE_SHORTCUTDATA];
		int SizeFolderNameShortcut=GetShortcutFolderSize(Key);
		PathName=new char[SizeFolderNameShortcut+NM];

		if (GetShortcutFolder(Key,PathName,SizeFolderNameShortcut+NM,PluginModule,PluginFile,PluginData))
			RetCode=TRUE;
	}
	else // Пути/имена?
	{
		PathName=new char[4096];
		RetCode=_MakePath1(Key,PathName,4096-1,"");
	}

	// Если что-нить получилось, именно его и вставим (PathName)
	if (RetCode)
	{
		if (Flags.Check(FEDITLINE_CLEARFLAG))
		{
			LeftPos=0;
			SetString("");
		}

		if (PrevSelStart!=-1)
		{
			SelStart=PrevSelStart;
			SelEnd=PrevSelEnd;
		}

		if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
			DeleteBlock();

		if (TableSet)
			EncodeString(PathName,(unsigned char*)TableSet->EncodeTable,(int)strlen(PathName));

		char *Ptr=PathName;

		for (; *Ptr; Ptr++)
			InsertKey(*Ptr);

		Flags.Clear(FEDITLINE_CLEARFLAG);
	}

	delete[] PathName;
	return RetCode;
}


__int64 Edit::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return (__int64)(GetLength()==0);
		case MCODE_C_SELECTED:
			return (__int64)(SelStart != -1 && SelStart < SelEnd);
		case MCODE_C_EOF:
			return (__int64)(CurPos >= StrSize);
		case MCODE_C_BOF:
			return (__int64)(CurPos==0);
		case MCODE_V_ITEMCOUNT:
			return (__int64)StrSize;
		case MCODE_V_CURPOS:
			return (__int64)(CursorPos+1);
		case MCODE_F_EDITOR_SEL:
		{
			int Action=(int)((INT_PTR)vParam);

			switch (Action)
			{
				case 0:  // Get Param
				{
					switch (iParam)
					{
						case 0:  // return FirstLine
						case 2:  // return LastLine
							return IsSelection()?1:0;
						case 1:  // return FirstPos
							return IsSelection()?SelStart+1:0;
						case 3:  // return LastPos
							return IsSelection()?SelEnd:0;
						case 4: // return block type (0=nothing 1=stream, 2=column)
							return IsSelection()?1:0;
					}

					break;
				}
				case 1:  // Set Pos
				{
					if (IsSelection())
					{
						switch (iParam)
						{
							case 0: // begin block (FirstLine & FirstPos)
							case 1: // end block (LastLine & LastPos)
							{
								SetTabCurPos(iParam==0?SelStart:SelEnd);
								Show();
								return 1;
							}
						}
					}

					break;
				}
				case 2: // Set Stream Selection Edge
				case 3: // Set Column Selection Edge
				{
					switch (iParam)
					{
						case 0:  // selection start
						{
							MSelStart=GetCurPos();
							return 1;
						}
						case 1:  // selection finish
						{
							if (MSelStart != -1)
							{
								if (MSelStart != GetCurPos())
									Select(MSelStart,GetCurPos());
								else
									Select(-1,0);

								Show();
								MSelStart=-1;
								return 1;
							}

							return 0;
						}
					}

					break;
				}
				case 4: // UnMark sel block
				{
					Select(-1,0);
					MSelStart=-1;
					Show();
					return 1;
				}
			}

			break;
		}
	}

	return _i64(0);
}

int Edit::ProcessKey(int Key)
{
	int I;

	switch (Key)
	{
		case KEY_ADD:
			Key='+';
			break;
		case KEY_SUBTRACT:
			Key='-';
			break;
		case KEY_MULTIPLY:
			Key='*';
			break;
		case KEY_DIVIDE:
			Key='/';
			break;
		case KEY_DECIMAL:
			Key='.';
			break;
		case KEY_CTRLC:
			Key=KEY_CTRLINS;
			break;
		case KEY_CTRLV:
			Key=KEY_SHIFTINS;
			break;
		case KEY_CTRLX:
			Key=KEY_SHIFTDEL;
			break;
	}

	int PrevSelStart=-1,PrevSelEnd=0;

	/* $ 25.07.2000 tran
	   при дроп-даун, нам Ctrl-l не нужен */

	/* $ 03.07.2000 tran
	   + обработка Ctrl-L как переключателя состояния ReadOnly  */
	if (!Flags.Check(FEDITLINE_DROPDOWNBOX) && Key==KEY_CTRLL)
	{
		Flags.Swap(FEDITLINE_READONLY);
	}

	/* tran 03.07.2000 $ */
	/* tran 25.07.2000 $ */

	/* $ 26.07.2000 SVS
	   Bugs #??
	     В строках ввода при выделенном блоке нажимаем BS и вместо
	     ожидаемого удаления блока (как в редакторе) получаем:
	       - символ перед курсором удален
	       - выделение блока снято
	*/
	if (((Key==KEY_BS || Key==KEY_DEL || Key==KEY_NUMDEL) && Flags.Check(FEDITLINE_DELREMOVESBLOCKS) || Key==KEY_CTRLD) &&
	        !Flags.Check(FEDITLINE_EDITORMODE) && SelStart!=-1 && SelStart<SelEnd)
	{
		DeleteBlock();
		Show();
		return(TRUE);
	}

	/* SVS $ */
	int _Macro_IsExecuting=CtrlObject->Macro.IsExecuting();

	// $ 04.07.2000 IG - добавлена проврерка на запуск макроса (00025.edit.cpp.txt)
	if (!ShiftPressed && (!_Macro_IsExecuting || IsNavKey(Key) && _Macro_IsExecuting) &&
	        !IsShiftKey(Key) && !Recurse &&
	        Key!=KEY_SHIFT && Key!=KEY_CTRL && Key!=KEY_ALT &&
	        Key!=KEY_RCTRL && Key!=KEY_RALT && Key!=KEY_NONE &&
	        Key!=KEY_INS &&
	        Key!=KEY_KILLFOCUS && Key != KEY_GOTFOCUS &&
	        ((Key&(~0xFF000000)) != KEY_LWIN && (Key&(~0xFF000000)) != KEY_RWIN && (Key&(~0xFF000000)) != KEY_APPS)
	   )
	{
//_SVS(SysLog("Edit::ProcessKey(0x%08X ==> '%s')",Key,_FARKEY_ToName(Key)));
//_SVS(SysLog("_Macro_IsExecuting = %d",_Macro_IsExecuting));
//_SVS(SysLog("ShiftPressed       = %d",ShiftPressed));
//_SVS(SysLog("IsNavKey(Key)      = %d",IsNavKey(Key)));
//_SVS(SysLog("IsShiftKey(Key)    = %d",IsShiftKey(Key)));
//_SVS(SysLog("Recurse            = %d",Recurse));
		Flags.Clear(FEDITLINE_MARKINGBLOCK); // хмм... а это здесь должно быть?

		if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS) && !(Key==KEY_CTRLINS || Key==KEY_CTRLNUMPAD0) &&
		        !(Key==KEY_SHIFTDEL||Key==KEY_SHIFTNUMDEL||Key==KEY_SHIFTDECIMAL) && !Flags.Check(FEDITLINE_EDITORMODE) && Key != KEY_CTRLQ &&
		        !(Key == KEY_SHIFTINS || Key == KEY_SHIFTNUMPAD0)) //Key != KEY_SHIFTINS) //??
		{
			/* $ 12.11.2002 DJ
			   зачем рисоваться, если ничего не изменилось?
			*/
			if (SelStart != -1 || SelEnd != 0)
			{
				PrevSelStart=SelStart;
				PrevSelEnd=SelEnd;
				Select(-1,0);
				Show();
//_SVS(SysLog("Edit::ProcessKey(), Select Kill"));
			}

			/* DJ $ */
		}
	}

	if (!EditEncodeDisabled && Key<256 && TableSet && !ReturnAltValue)
		Key=TableSet->EncodeTable[Key];

	/* $ 11.09.2000 SVS
	   если Opt.DlgEULBsClear = 1, то BS в диалогах для UnChanged строки
	   удаляет такую строку также, как и Del
	*/
	if (((Opt.Dialogs.EULBsClear && Key==KEY_BS) || Key==KEY_DEL || Key==KEY_NUMDEL) &&
	        Flags.Check(FEDITLINE_CLEARFLAG) && CurPos>=StrSize)
		Key=KEY_CTRLY;

	/* SVS $ */

	/* $ 15.09.2000 SVS
	   Bug - Выделяем кусочек строки -> Shift-Del удяляет всю строку
	         Так должно быть только для UnChanged состояния
	*/
	if ((Key == KEY_SHIFTDEL || Key == KEY_SHIFTNUMDEL || Key == KEY_SHIFTDECIMAL) && Flags.Check(FEDITLINE_CLEARFLAG) && CurPos>=StrSize && SelStart==-1)
	{
		SelStart=0;
		SelEnd=StrSize;
	}

	/* SVS $ */

	if (Flags.Check(FEDITLINE_CLEARFLAG) && (Key<256 && Key!=KEY_BS || Key==KEY_CTRLBRACKET ||
	        Key==KEY_CTRLBACKBRACKET || Key==KEY_CTRLSHIFTBRACKET ||
	        Key==KEY_CTRLSHIFTBACKBRACKET || Key==KEY_SHIFTENTER || Key==KEY_SHIFTNUMENTER))
	{
		LeftPos=0;
		SetString("");
		Show();
	}

	// Здесь - вызов функции вставки путей/файлов
	if (ProcessInsPath(Key,PrevSelStart,PrevSelEnd))
	{
		Show();
		return TRUE;
	}

	if (Key!=KEY_NONE && Key!=KEY_IDLE && Key!=KEY_SHIFTINS && Key!=KEY_SHIFTNUMPAD0 && Key!=KEY_CTRLINS &&
	        (Key<KEY_F1 || Key>KEY_F12) && Key!=KEY_ALT && Key!=KEY_SHIFT &&
	        Key!=KEY_CTRL && Key!=KEY_RALT && Key!=KEY_RCTRL &&
	        (Key<KEY_ALT_BASE || Key>=KEY_ALT_BASE+256) &&
	        !(Key>=KEY_MACRO_BASE && Key<=KEY_MACRO_ENDBASE || Key>=KEY_OP_BASE && Key <=KEY_OP_ENDBASE) && Key!=KEY_CTRLQ)
	{
		Flags.Clear(FEDITLINE_CLEARFLAG);
		Show();
	}

	switch (Key)
	{
		case KEY_SHIFTLEFT: case KEY_SHIFTNUMPAD4:
		{
			if (CurPos>0)
			{
				RecurseProcessKey(KEY_LEFT);

				if (!Flags.Check(FEDITLINE_MARKINGBLOCK))
				{
					Select(-1,0);
					Flags.Set(FEDITLINE_MARKINGBLOCK);
				}

				if (SelStart!=-1 && SelStart<=CurPos)
					Select(SelStart,CurPos);
				else
				{
					int EndPos=CurPos+1;
					int NewStartPos=CurPos;

					if (EndPos>StrSize)
						EndPos=StrSize;

					if (NewStartPos>StrSize)
						NewStartPos=StrSize;

					AddSelect(NewStartPos,EndPos);
				}

				Show();
			}

			return(TRUE);
		}
		case KEY_SHIFTRIGHT: case KEY_SHIFTNUMPAD6:
		{
			if (!Flags.Check(FEDITLINE_MARKINGBLOCK))
			{
				Select(-1,0);
				Flags.Set(FEDITLINE_MARKINGBLOCK);
			}

			if (SelStart!=-1 && SelEnd==-1 || SelEnd>CurPos)
			{
				if (CurPos+1==SelEnd)
					Select(-1,0);
				else
					Select(CurPos+1,SelEnd);
			}
			else
				AddSelect(CurPos,CurPos+1);

			RecurseProcessKey(KEY_RIGHT);
			return(TRUE);
		}
		case KEY_CTRLSHIFTLEFT: case KEY_CTRLSHIFTNUMPAD4:
		{
			/* $ 15.08.2000 KM */
			if (CurPos>StrSize)
			{
				PrevCurPos=CurPos;
				CurPos=StrSize;
			}

			/* KM $ */
			if (CurPos>0)
				RecurseProcessKey(KEY_SHIFTLEFT);

			/* $ 12.01.2004 IS
			   Для сравнения с WordDiv используем IsWordDiv, а не strchr, т.к.
			   текущая кодировка может отличаться от кодировки WordDiv (которая OEM)
			*/
			while (CurPos>0 && !(!IsWordDiv(TableSet, WordDiv, Str[CurPos]) &&
			                     IsWordDiv(TableSet, WordDiv,Str[CurPos-1]) && !IsSpace(Str[CurPos])))
			{
				/* $ 18.08.2000 KM
				   Добавим выход из цикла проверив CurPos-1 на присутствие
				   в WordDiv.
				*/
//        if (!IsSpace(Str[CurPos]) && IsSpace(Str[CurPos-1]))
				if (!IsSpace(Str[CurPos]) && (IsSpace(Str[CurPos-1]) ||
				                              IsWordDiv(TableSet, WordDiv, Str[CurPos-1])))
					break;

				/* KM $ */
				RecurseProcessKey(KEY_SHIFTLEFT);
			}

			/* IS $ */
			Show();
			return(TRUE);
		}
		case KEY_CTRLSHIFTRIGHT: case KEY_CTRLSHIFTNUMPAD6:
		{
			if (CurPos>=StrSize)
				return(FALSE);

			RecurseProcessKey(KEY_SHIFTRIGHT);

			/* $ 12.01.2004 IS
			   Для сравнения с WordDiv используем IsWordDiv, а не strchr, т.к.
			   текущая кодировка может отличаться от кодировки WordDiv (которая OEM)
			*/
			while (CurPos<StrSize && !(IsWordDiv(TableSet, WordDiv, Str[CurPos]) &&
			                           !IsWordDiv(TableSet, WordDiv, Str[CurPos-1])))
			{
				/* $ 18.08.2000 KM
				   Добавим выход из цикла проверив CurPos-1 на присутствие
				   в WordDiv.
				*/
//        if (!IsSpace(Str[CurPos]) && IsSpace(Str[CurPos-1]))
				if (!IsSpace(Str[CurPos]) && (IsSpace(Str[CurPos-1]) || IsWordDiv(TableSet, WordDiv, Str[CurPos-1])))
					break;

				/* KM $ */
				RecurseProcessKey(KEY_SHIFTRIGHT);

				if (MaxLength!=-1 && CurPos==MaxLength-1)
					break;
			}

			/* IS $ */
			Show();
			return(TRUE);
		}
		case KEY_SHIFTHOME:  case KEY_SHIFTNUMPAD7:
		{
			Lock();

			while (CurPos>0)
				RecurseProcessKey(KEY_SHIFTLEFT);

			Unlock();
			Show();
			return(TRUE);
		}
		case KEY_SHIFTEND:  case KEY_SHIFTNUMPAD1:
		{
			Lock();
			/* $ 21.09.2003 KM
			   Уточнения работы с маской
			*/
			int Len;

			if (Mask && *Mask)
			{
				char *ShortStr=new char[StrSize+1];

				if (ShortStr==NULL)
					return FALSE;

				xstrncpy(ShortStr,Str,StrSize);
				Len=(int)strlen(RemoveTrailingSpaces(ShortStr));
				delete[] ShortStr;
			}
			else
				Len=StrSize;

			int LastCurPos=CurPos;

			while (CurPos<Len/*StrSize*/)
			{
				RecurseProcessKey(KEY_SHIFTRIGHT);

				if (LastCurPos==CurPos)break;

				LastCurPos=CurPos;
			}

			/* KM $ */
			Unlock();
			Show();
			return(TRUE);
		}
		case KEY_BS:
		{
			if (CurPos<=0)
				return(FALSE);

			/* $ 15.08.2000 KM */
			PrevCurPos=CurPos;
			/* KM $ */
			CurPos--;

			if (CurPos<=LeftPos)
			{
				LeftPos-=15;

				if (LeftPos<0)
					LeftPos=0;
			}

			if (!RecurseProcessKey(KEY_DEL))
				Show();

			return(TRUE);
		}
		/* $ 10.12.2000 tran
		   KEY_SHIFTBS изменен на KEY_CTRLSHIFTBS*/
		/* $ 03.07.2000 tran
		   + KEY_SHIFTBS - удялем от курсора до начала строки */
		case KEY_CTRLSHIFTBS:
		{
			/* tran $ */
			/* $ 19.08.2000 KM
			   Немного изменён алгоритм удаления до начала строки.
			   Теперь удаление работает и с маской ввода.
			*/
			int i;

			for (i=CurPos; i>=0; i--)
				RecurseProcessKey(KEY_BS);

			Show();
			return(TRUE);
		}
		/* KM $ */
		/* tran 03.07.2000 $ */
		case KEY_CTRLBS:
		{
			/* $ 15.08.2000 KM */
			if (CurPos>StrSize)
			{
				PrevCurPos=CurPos;
				CurPos=StrSize;
			}

			/* KM $ */
			Lock();

//      while (CurPos>0 && IsSpace(Str[CurPos-1]))
//        RecurseProcessKey(KEY_BS);
			while (1)
			{
				int StopDelete=FALSE;

				if (CurPos>1 && IsSpace(Str[CurPos-1])!=IsSpace(Str[CurPos-2]))
					StopDelete=TRUE;

				RecurseProcessKey(KEY_BS);

				if (CurPos==0 || StopDelete)
					break;

				/* $ 12.01.2004 IS
				   Для сравнения с WordDiv используем IsWordDiv, а не strchr, т.к.
				   текущая кодировка может отличаться от кодировки WordDiv (которая OEM)
				*/
				if (IsWordDiv(TableSet, WordDiv,Str[CurPos-1]))
					/* IS $ */
					break;
			}

			Unlock();
			Show();
			return(TRUE);
		}
		case KEY_CTRLQ:
		{
			Lock();

			if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS) && (SelStart != -1 || Flags.Check(FEDITLINE_CLEARFLAG)))
				RecurseProcessKey(KEY_DEL);

			ProcessCtrlQ();
			Unlock();
			Show();
			return(TRUE);
		}
		case KEY_OP_SELWORD:
		{
			int OldCurPos=CurPos;
			int SStart, SEnd;
			PrevSelStart=SelStart;
			PrevSelEnd=SelEnd;
#if defined(MOUSEKEY)

			if (SelStart != -1 && CurPos >= SelStart && CurPos <= SelEnd)
			{ // выделяем ВСЮ строку при повторном двойном клике
				Select(0,StrSize);
			}
			else
#endif
			{
				if (CalcWordFromString(Str,CurPos,&SStart,&SEnd,TableSet,WordDiv)) // TableSet --> UseDecodeTable?&TableSet:NULL
					Select(SStart,SEnd+(SEnd < StrSize?1:0));
			}

			CurPos=OldCurPos; // возвращаем обратно
			Show();
			return TRUE;
		}
		case KEY_OP_DATE:
		case KEY_OP_PLAINTEXT:
		{
			if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
			{
				if (SelStart != -1 || Flags.Check(FEDITLINE_CLEARFLAG)) // BugZ#1053 - Неточности в $Text
					RecurseProcessKey(KEY_DEL);
			}

			const char *str = eStackAsString();

			if (Key == KEY_OP_DATE)
				ProcessInsDate(str);
			else
				ProcessInsPlainText(str);

			Show();
			return TRUE;
		}
		case KEY_CTRLT:
		case KEY_CTRLDEL:
		case KEY_CTRLNUMDEL:
		case KEY_CTRLDECIMAL:
		{
			if (CurPos>=StrSize)
				return(FALSE);

			Lock();

//      while (CurPos<StrSize && IsSpace(Str[CurPos]))
//        RecurseProcessKey(KEY_DEL);
			/* $ 19.08.2000 KM
			   Поставим пока заглушку на удаление, если
			   используется маска ввода.
			*/
			if (Mask && *Mask)
			{
				/* $ 12.11.2000 KM
				   Добавим код для удаления части строки
				   с учётом маски.
				*/
				int MaskLen=(int)strlen(Mask);
				int ptr=CurPos;

				while (ptr<MaskLen)
				{
					ptr++;

					/* $ 12.01.2004 IS
					   Для сравнения с WordDiv используем IsWordDiv, а не strchr, т.к.
					   текущая кодировка может отличаться от кодировки WordDiv (которая OEM)
					*/
					if (!CheckCharMask(Mask[ptr]) ||
					        (IsSpace(Str[ptr]) && !IsSpace(Str[ptr+1])) ||
					        (IsWordDiv(TableSet, WordDiv, Str[ptr])))
						/* IS $ */
						break;
				}

				for (int i=0; i<ptr-CurPos; i++)
					RecurseProcessKey(KEY_DEL);

				/* KM $ */
			}
			else
			{
				while (1)
				{
					int StopDelete=FALSE;

					if (CurPos<StrSize-1 && IsSpace(Str[CurPos]) && !IsSpace(Str[CurPos+1]))
						StopDelete=TRUE;

					RecurseProcessKey(KEY_DEL);

					if (CurPos>=StrSize || StopDelete)
						break;

					/* $ 12.01.2004 IS
					   Для сравнения с WordDiv используем IsWordDiv, а не strchr, т.к.
					   текущая кодировка может отличаться от кодировки WordDiv (которая OEM)
					*/
					if (IsWordDiv(TableSet, WordDiv, Str[CurPos]))
						/* IS $ */
						break;
				}
			}

			Unlock();
			Show();
			return(TRUE);
		}
		case KEY_CTRLY:
		{
			/* $ 25.07.2000 tran
			   + DropDown style */
			/* $ 03.07.2000 tran
			   + обработка ReadOnly */
			if (Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
				return (TRUE);

			/* tran 03.07.2000 $ */
			/* tran 25.07.2000 $ */
			/* $ 15.08.2000 KM */
			PrevCurPos=CurPos;
			/* KM $ */
			LeftPos=CurPos=0;
			*Str=0;
			StrSize=0;
			Str=(char *)xf_realloc(Str,1);
			Select(-1,0);
			Show();
			return(TRUE);
		}
		case KEY_CTRLK:
		{
			/* $ 25.07.2000 tran
			   + DropDown style */
			/* $ 03.07.2000 tran
			   + обработка ReadOnly */
			if (Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
				return (TRUE);

			/* tran 03.07.2000 $ */

			/* tran 25.07.2000 $ */
			if (CurPos>=StrSize)
				return(FALSE);

			if (!Flags.Check(FEDITLINE_EDITBEYONDEND))
			{
				if (CurPos<SelEnd)
					SelEnd=CurPos;

				if (SelEnd<SelStart && SelEnd!=-1)
				{
					SelEnd=0;
					SelStart=-1;
				}
			}

			Str[CurPos]=0;
			StrSize=CurPos;
			Str=(char *)xf_realloc(Str,StrSize+1);
			Show();
			return(TRUE);
		}
		case KEY_HOME:        case KEY_NUMPAD7:
		case KEY_CTRLHOME:    case KEY_CTRLNUMPAD7:
		{
			/* $ 15.08.2000 KM */
			PrevCurPos=CurPos;
			/* KM $ */
			CurPos=0;
			Show();
			return(TRUE);
		}
		case KEY_END:         case KEY_NUMPAD1:
		case KEY_CTRLEND:     case KEY_CTRLNUMPAD1:
		{
			/* $ 15.08.2000 KM */
			PrevCurPos=CurPos;
			/* KM $ */

			/* $ 21.09.2003 KM
			   Уточнения работы с маской
			*/
			if (Mask && *Mask)
			{
				char *ShortStr=new char[StrSize+1];

				if (ShortStr==NULL)
					return FALSE;

				xstrncpy(ShortStr,Str,StrSize);
				CurPos=(int)strlen(RemoveTrailingSpaces(ShortStr));
				delete[] ShortStr;
			}
			else
				CurPos=StrSize;

			/* KM $ */
			Show();
			return(TRUE);
		}
		case KEY_LEFT:        case KEY_NUMPAD4:        case KEY_MSWHEEL_LEFT:
		case KEY_CTRLS:
		{
			if (CurPos>0)
			{
				/* $ 15.08.2000 KM */
				PrevCurPos=CurPos;
				/* KM $ */
				CurPos--;
				Show();
			}

			return(TRUE);
		}
		case KEY_RIGHT:       case KEY_NUMPAD6:        case KEY_MSWHEEL_RIGHT:
		case KEY_CTRLD:
		{
			/* $ 15.08.2000 KM */
			PrevCurPos=CurPos;
			/* KM $ */

			/* $ 21.09.2003 KM
			   Уточнения работы с маской
			*/
			if (Mask && *Mask)
			{
				char *ShortStr=new char[StrSize+1];

				if (ShortStr==NULL)
					return FALSE;

				xstrncpy(ShortStr,Str,StrSize);
				int Len=(int)strlen(RemoveTrailingSpaces(ShortStr));
				delete[] ShortStr;

				if (Len>CurPos)
					CurPos++;
			}
			else
				CurPos++;

			/* KM $ */
			Show();
			return(TRUE);
		}
		case KEY_INS:         case KEY_NUMPAD0:
		{
			Flags.Swap(FEDITLINE_OVERTYPE);
			Show();
			return(TRUE);
		}
		case KEY_NUMDEL:
		case KEY_DEL:
		{
			/* $ 25.07.2000 tran
			   + DropDown style */
			/* $ 03.07.2000 tran
			   + обработка ReadOnly */
			if (Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
				return (TRUE);

			/* tran 03.07.2000 $ */

			/* tran 25.07.2000 $ */
			if (CurPos>=StrSize)
				return(FALSE);

			if (SelStart!=-1)
			{
				if (SelEnd!=-1 && CurPos<SelEnd)
					SelEnd--;

				if (CurPos<SelStart)
					SelStart--;

				if (SelEnd!=-1 && SelEnd<=SelStart)
				{
					SelStart=-1;
					SelEnd=0;
				}
			}

			/* $ 16.08.2000 KM
			   Работа с маской.
			*/
			if (Mask && *Mask)
			{
				int MaskLen=(int)strlen(Mask);
				int i,j;

				for (i=CurPos,j=CurPos; i<MaskLen; i++)
				{
					if (CheckCharMask(Mask[i+1]))
					{
						while (!CheckCharMask(Mask[j]) && j<MaskLen)
							j++;

						Str[j]=Str[i+1];
						j++;
					}
				}

				Str[j]=' ';
			}
			else
			{
				memmove(Str+CurPos,Str+CurPos+1,StrSize-CurPos);
				StrSize--;
				Str=(char *)xf_realloc(Str,StrSize+1);
			}

			/* KM $ */
			Show();
			return(TRUE);
		}
		case KEY_CTRLLEFT:  case KEY_CTRLNUMPAD4:
		{
			/* $ 15.08.2000 KM */
			PrevCurPos=CurPos;

			/* KM $ */
			if (CurPos>StrSize)
				CurPos=StrSize;

			if (CurPos>0)
				CurPos--;

			/* $ 12.01.2004 IS
			   Для сравнения с WordDiv используем IsWordDiv, а не strchr, т.к.
			   текущая кодировка может отличаться от кодировки WordDiv (которая OEM)
			*/
			while (CurPos>0 && !(!IsWordDiv(TableSet, WordDiv, Str[CurPos]) &&
			                     IsWordDiv(TableSet, WordDiv, Str[CurPos-1]) && !IsSpace(Str[CurPos])))
				/* IS $ */
			{
				if (!IsSpace(Str[CurPos]) && IsSpace(Str[CurPos-1]))
					break;

				CurPos--;
			}

			Show();
			return(TRUE);
		}
		case KEY_CTRLRIGHT:   case KEY_CTRLNUMPAD6:
		{
			if (CurPos>=StrSize)
				return(FALSE);

			/* $ 15.08.2000 KM */
			PrevCurPos=CurPos;
			/* KM $ */
			/* $ 21.09.2003 KM
			   Уточнения работы с маской
			*/
			int Len;

			if (Mask && *Mask)
			{
				char *ShortStr=new char[StrSize+1];

				if (ShortStr==NULL)
					return FALSE;

				xstrncpy(ShortStr,Str,StrSize);
				Len=(int)strlen(RemoveTrailingSpaces(ShortStr));
				delete[] ShortStr;

				if (Len>CurPos)
					CurPos++;
			}
			else
			{
				Len=StrSize;
				CurPos++;
			}

			/* $ 12.01.2004 IS
			   Для сравнения с WordDiv используем IsWordDiv, а не strchr, т.к.
			   текущая кодировка может отличаться от кодировки WordDiv (которая OEM)
			*/
			while (CurPos<Len/*StrSize*/ && !(IsWordDiv(TableSet, WordDiv,Str[CurPos]) &&
			                                  !IsWordDiv(TableSet, WordDiv, Str[CurPos-1])))
				/* IS $ */
				/* KM $ */
			{
				if (!IsSpace(Str[CurPos]) && IsSpace(Str[CurPos-1]))
					break;

				CurPos++;
			}

			Show();
			return(TRUE);
		}
		case KEY_SHIFTNUMDEL:
		case KEY_SHIFTDECIMAL:
		case KEY_SHIFTDEL:
		{
			if (SelStart==-1 || SelStart>=SelEnd)
				return(FALSE);

			RecurseProcessKey(KEY_CTRLINS);
			DeleteBlock();
			Show();
			return(TRUE);
		}
		case KEY_CTRLINS:     case KEY_CTRLNUMPAD0:
		{
			if (!Flags.Check(FEDITLINE_PASSWORDMODE))
				if (SelStart==-1 || SelStart>=SelEnd)
				{
					/* $ 26.10.2003 KM
					   ! Уточнение копирования маскированной строки в клипборд.
					*/
					if (Mask && *Mask)
					{
						char *ShortStr=new char[StrSize+1];

						if (ShortStr==NULL)
							return FALSE;

						xstrncpy(ShortStr,Str,StrSize);
						RemoveTrailingSpaces(ShortStr);
						CopyToClipboard(ShortStr);
						delete[] ShortStr;
					}
					else
						CopyToClipboard(Str);

					/* KM $ */
				}
				else if (SelEnd<=StrSize) // TODO: если в начало условия добавить "StrSize &&", то пропадет баг "Ctrl-Ins в пустой строке очищает клипборд"
				{
					int Ch=Str[SelEnd];
					Str[SelEnd]=0;
					CopyToClipboard(Str+SelStart);
					Str[SelEnd]=Ch;
				}

			return(TRUE);
		}
		case KEY_SHIFTINS:    case KEY_SHIFTNUMPAD0:
		{
			/* $ 15.10.2000 tran
			   если строка ввода имет максимальную длину
			   то их клипборда грузим не больше ее*/
			char *ClipText;

			if (MaxLength==-1)
				ClipText=PasteFromClipboard();
			else
				ClipText=PasteFromClipboardEx(MaxLength);

			/* tran $ */
			if (ClipText==NULL)
				return(TRUE);

			if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
			{
				DeleteBlock();
			}

			for (I=(int)strlen(Str)-1; I>=0 && IsEol(Str[I]); I--)
				Str[I]=0;

			for (I=0; ClipText[I]; I++)
			{
				if (IsEol(ClipText[I]))
				{
					if (IsEol(ClipText[I+1]))
						memmove(&ClipText[I],&ClipText[I+1],strlen(&ClipText[I+1])+1);

					if (ClipText[I+1]==0)
						ClipText[I]=0;
					else
						ClipText[I]=' ';
				}
			}

			if (Flags.Check(FEDITLINE_CLEARFLAG))
			{
				LeftPos=0;
				SetString(ClipText);
				Flags.Clear(FEDITLINE_CLEARFLAG);
			}
			else
			{
				InsertString(ClipText);
			}

			xf_free(ClipText);
			Show();
			return(TRUE);
		}
		case KEY_SHIFTTAB:
		{
			/* $ 15.08.2000 KM */
			PrevCurPos=CurPos;
			/* KM $ */
			/* $ 12.12.2000 OT KEY_SHIFTTAB Bug Fix*/
			CursorPos-=(CursorPos-1) % TabSize+1;

			if (CursorPos<0) CursorPos=0; //CursorPos=0,TabSize=1 case

			SetTabCurPos(CursorPos);
			/* OT $ */
			Show();
			return(TRUE);
		}
		/* $ 13.02.2001 VVM
		  + Обработка SHIFT+SPACE */
		case KEY_SHIFTSPACE:
			Key = KEY_SPACE;
			/* VVM $ */
		default:
		{
//      _D(SysLog("Key=0x%08X",Key));
			if (Key==KEY_NONE || Key==KEY_IDLE || Key==KEY_ENTER || Key==KEY_NUMENTER || Key>=256)
				break;

			if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
			{
				if (PrevSelStart!=-1)
				{
					SelStart=PrevSelStart;
					SelEnd=PrevSelEnd;
				}

				DeleteBlock();
			}

			if (InsertKey(Key))
				Show();

			return(TRUE);
		}
	}

	return(FALSE);
}

// обработка Ctrl-Q
int Edit::ProcessCtrlQ(void)
{
	INPUT_RECORD rec;
	DWORD Key;

	while (1)
	{
		Key=GetInputRecord(&rec);

		if (Key!=KEY_NONE && Key!=KEY_IDLE && rec.Event.KeyEvent.uChar.AsciiChar)
			break;

		if (Key==KEY_CONSOLE_BUFFER_RESIZE)
		{
			//int Dis=EditOutDisabled;
			//EditOutDisabled=0;
			Show();
			//EditOutDisabled=Dis;
		}
	}

	/*
	  EditOutDisabled++;
	  if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
	  {
	    DeleteBlock();
	  }
	  else
	    Flags.Clear(FEDITLINE_CLEARFLAG);
	  EditOutDisabled--;
	*/
	return InsertKey(rec.Event.KeyEvent.uChar.AsciiChar);
}

int Edit::ProcessInsDate(const char *Fmt)
{
	int SizeMacroText = 16+(Fmt && *Fmt ? (int)strlen(Fmt) : (int)strlen(Opt.DateFormat));
	SizeMacroText*=4+1;
	char *TStr=(char*)alloca(SizeMacroText);

	if (!TStr)
		return FALSE;

	if (MkStrFTime(TStr,SizeMacroText,Fmt))
	{
		InsertString(TStr);
		return TRUE;
	}

	return FALSE;
}

int Edit::ProcessInsPlainText(const char *str)
{
	if (*str)
	{
		InsertString(str);
		return TRUE;
	}

	return FALSE;
}

int Edit::InsertKey(int Key)
{
	char *NewStr;

	/* $ 25.07.2000 tran
	   + drop-down */

	/* $ 03.07.2000 tran
	   + обработка ReadOnly */
	if (Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
		return (TRUE);

	/* tran 03.07.2000 $ */

	/* $ 15.08.2000 KM
	   Работа с маской ввода.
	*/
	if (Key==KEY_TAB && Flags.Check(FEDITLINE_OVERTYPE))
	{
		PrevCurPos=CurPos;
		/* $ 14.12.2000 OT KEY_TAB Bug Fix*/
		CursorPos+=TabSize - (CursorPos % TabSize);
		SetTabCurPos(CursorPos);
		/* OT $ */
		return(TRUE);
	}

	if (Mask && *Mask)
	{
		int MaskLen=(int)strlen(Mask);

		if (CurPos<MaskLen)
		{
			/* $ 15.11.2000 KM
			   Убран кусок кода и сделана функция KeyMatchedMask,
			   проверяющая разрешение символа на ввод по маске.
			*/
			/* KM $*/
			if (KeyMatchedMask(Key))
			{
				if (!Flags.Check(FEDITLINE_OVERTYPE))
				{
					int i=MaskLen-1;

					while (!CheckCharMask(Mask[i]) && i>CurPos)
						i--;

					for (int j=i; i>CurPos; i--)
					{
						if (CheckCharMask(Mask[i]))
						{
							while (!CheckCharMask(Mask[j-1]))
							{
								if (j<=CurPos)
									/* $ 15.11.2000 KM
									   Замена continue на break
									*/
									break;

								/* KM $ */
								j--;
							}

							Str[i]=Str[j-1];
							j--;
						}
					}
				}

				PrevCurPos=CurPos;
				Str[CurPos++]=Key;
			}
			else
			{
				// Здесь вариант для "ввели символ из маски", например для SetAttr - ввесли '.'
				;// char *Ptr=strchr(Mask+CurPos,Key);
			}
		}
		else if (CurPos<StrSize)
		{
			PrevCurPos=CurPos;
			Str[CurPos++]=Key;
		}
	}
	else
	{
		if (MaxLength == -1 || StrSize < MaxLength)
		{
			if (CurPos>=StrSize)
			{
				if ((NewStr=(char *)xf_realloc(Str,CurPos+2))==NULL)
					return(FALSE);

				Str=NewStr;
				sprintf(&Str[StrSize],"%*s",CurPos-StrSize,"");
				//memset(Str+StrSize,' ',CurPos-StrSize);Str[CurPos+1]=0;
				StrSize=CurPos+1;
			}
			else if (!Flags.Check(FEDITLINE_OVERTYPE))
				StrSize++;

			if (Key==KEY_TAB && (TabExpandMode==EXPAND_NEWTABS || TabExpandMode==EXPAND_ALLTABS))
			{
				StrSize--;
				InsertTab();
				return TRUE;
			}

			if ((NewStr=(char *)xf_realloc(Str,StrSize+1))==NULL)
				return(TRUE);

			Str=NewStr;

			if (!Flags.Check(FEDITLINE_OVERTYPE))
			{
				memmove(Str+CurPos+1,Str+CurPos,StrSize-CurPos);

				if (SelStart!=-1)
				{
					if (SelEnd!=-1 && CurPos<SelEnd)
						SelEnd++;

					if (CurPos<SelStart)
						SelStart++;
				}
			}

			PrevCurPos=CurPos;
			Str[CurPos++]=Key;
		}
		else if (Flags.Check(FEDITLINE_OVERTYPE))
		{
			if (CurPos < StrSize)
			{
				PrevCurPos=CurPos;
				Str[CurPos++]=Key;
			}
		}
		else
			MessageBeep(MB_ICONHAND);
	}

	/* KM $ */
	Str[StrSize]=0;
	return(TRUE);
}

/* $ 28.07.2000 SVS
   ! имеет дополнительный параметр для установки ColorUnChanged
*/
void Edit::SetObjectColor(int Color,int SelColor,int ColorUnChanged)
{
	Edit::Color=Color;
	Edit::SelColor=SelColor;
	Edit::ColorUnChanged=ColorUnChanged;
}
/* SVS $ */


void Edit::GetString(char *pStr,int MaxSize)
{
	//xstrncpy(Str,Edit::Str,MaxSize-1);
	memmove(pStr,Str,Min(StrSize,MaxSize-1));
	pStr[Min(StrSize,MaxSize-1)]=0;
	pStr[MaxSize-1]=0;
}


const char* Edit::GetStringAddr()
{
	return(Edit::Str);
}


void  Edit::SetHiString(const char *Str)
{
	if (Flags.Check(FEDITLINE_READONLY))
		return;

	char *NewStr=(char*) xf_malloc((int)strlen(Str)*2+1);

	if (NewStr==NULL)
		return;

	HiText2Str(NewStr, (int)strlen(Str)*2, Str);
	Select(-1,0);
	SetBinaryString(NewStr,(int)strlen(NewStr));
	xf_free(NewStr);
}

/* $ 25.07.2000 tran
   примечание:
   в этом методе DropDownBox не обрабатывается
   ибо именно этот метод вызывается для установки из истории */
void Edit::SetString(const char *Str, int Length)
{
	if (Flags.Check(FEDITLINE_READONLY))
		return;

	Select(-1,0);
	SetBinaryString(Str,Length==-1?(int)strlen(Str):Length);
}


void Edit::SetEOL(const char *EOL)
{
	EndType=EOL_NONE;

	if (EOL && *EOL)
	{
		if (EOL[0]=='\r')
		{
			if (EOL[1]=='\n')
				EndType=EOL_CRLF;
			else if (EOL[1]=='\r' && EOL[2]=='\n')
				EndType=EOL_CRCRLF;
			else
				EndType=EOL_CR;
		}
		else
		{
			if (EOL[0]=='\n')
				EndType=EOL_LF;
		}
	}
}

const char *Edit::GetEOL(void)
{
	return EOL_TYPE_CHARS[EndType];
}

/* $ 25.07.2000 tran
   примечание:
   в этом методе DropDownBox не обрабатывается
   ибо он вызывается только из SetString и из класса Editor
   в Dialog он нигде не вызывается */
void Edit::SetBinaryString(const char *Str,int Length)
{
	if (Flags.Check(FEDITLINE_READONLY))
		return;

	// коррекция вставляемого размера, если определен MaxLength
	if (MaxLength != -1 && Length > MaxLength)
	{
		Length=MaxLength; // ??
	}

	if (Length>0 && !Flags.Check(FEDITLINE_PARENT_SINGLELINE))
	{
		if (Str[Length-1]=='\r')
		{
			EndType=EOL_CR;
			Length--;
		}
		else
		{
			if (Str[Length-1]=='\n')
			{
				Length--;

				if (Length>0 && Str[Length-1]=='\r')
				{
					Length--;

					if (Length>0 && Str[Length-1]=='\r')
					{
						Length--;
						EndType=EOL_CRCRLF;
					}
					else
						EndType=EOL_CRLF;
				}
				else
					EndType=EOL_LF;
			}
			else
				EndType=EOL_NONE;
		}
	}

	/* $ 15.08.2000 KM
	   Работа с маской
	*/
	/* $ 12.11.2000 KM
	   Убран кусок кода от SVS проверяющий конец строки.
	   Он не работал НИКОГДА.
	*/
	CurPos=0;

	if (Mask && *Mask)
	{
		RefreshStrByMask(TRUE);
		/* $ 26.10.2003 KM
		   ! Скорректируем вставку из клипборда с учётом маски
		*/
		int maskLen=(int)strlen(Mask);

		for (int i=0,j=0; j<maskLen && j<Length;)
		{
			if (CheckCharMask(Mask[i]))
			{
				int goLoop=FALSE;

				if (KeyMatchedMask(Str[j]))
					InsertKey(Str[j]);
				else
					goLoop=TRUE;

				j++;

				if (goLoop) continue;
			}
			else
			{
				PrevCurPos=CurPos;
				CurPos++;
			}

			i++;
		}

		/* KM $ */
		/* Здесь необходимо условие (*Str==0), т.к. для очистки строки
		   обычно вводится нечто вроде SetBinaryString("",0)
		   Т.е. таким образом мы добиваемся "инициализации" строки с маской
		*/
		RefreshStrByMask(*Str==0);
	}
	/* KM $ */
	else
	{
		char *NewStr=(char *)xf_realloc(Edit::Str,Length+1);

		if (NewStr==NULL)
			return;

		Edit::Str=NewStr;
		StrSize=Length;
		memcpy(Edit::Str,Str,Length);
		Edit::Str[Length]=0;

		if (TabExpandMode == EXPAND_ALLTABS)
			ReplaceTabs();

		PrevCurPos=CurPos;
		CurPos=StrSize;
	}

	/* KM $ */
}


void Edit::GetBinaryString(const char **Str,const char **EOL,int &Length)
{
	*Str=Edit::Str;

	if (EOL!=NULL)
		*EOL=EOL_TYPE_CHARS[EndType];

	Length=StrSize;
}


int Edit::GetSelString(char *Str,int MaxSize)
{
	if (SelStart==-1 || SelEnd!=-1 && SelEnd<=SelStart ||
	        SelStart>=StrSize)
	{
		*Str=0;
		return(FALSE);
	}

	int CopyLength;

	if (SelEnd==-1)
		CopyLength=MaxSize-1;
	else
		CopyLength=Min(MaxSize-1,SelEnd-SelStart);

	xstrncpy(Str,Edit::Str+SelStart,CopyLength);
	Str[CopyLength]=0;
	return(TRUE);
}


void Edit::InsertString(const char *Str)
{
	if (Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
		return;

	Select(-1,0);
	InsertBinaryString(Str,(int)strlen(Str));
}


void Edit::InsertBinaryString(const char *Str,int Length)
{
	char *NewStr;

	if (Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
		return;

	Flags.Clear(FEDITLINE_CLEARFLAG);

	if (Mask && *Mask)
	{
		int Pos=CurPos;
		int MaskLen=(int)strlen(Mask);

		if (Pos<MaskLen)
		{
			//_SVS(SysLog("InsertBinaryString ==> Str='%s' (Length=%d) Mask='%s'",Str,Length,Mask+Pos));
			int StrLen=(MaskLen-Pos>Length)?Length:MaskLen-Pos;

			/* $ 15.11.2000 KM
			   Внесены исправления для правильной работы PasteFromClipboard
			   в строке с маской
			*/
			for (int i=Pos,j=0; j<StrLen+Pos;)
			{
				if (CheckCharMask(Mask[i]))
				{
					int goLoop=FALSE;

					if (j < Length && KeyMatchedMask(Str[j]))
					{
						InsertKey(Str[j]);
						//_SVS(SysLog("InsertBinaryString ==> InsertKey(Str[%d]='%c');",j,Str[j]));
					}
					else
						goLoop=TRUE;

					j++;

					if (goLoop) continue;
				}
				else
				{
					PrevCurPos=CurPos;
					CurPos++;
				}

				i++;
			}

			/* KM $ */
		}

		RefreshStrByMask();
		//_SVS(SysLog("InsertBinaryString ==> Edit::Str='%s'",Edit::Str));
	}
	else
	{
		if (MaxLength != -1 && StrSize+Length > MaxLength)
		{
			// коррекция вставляемого размера, если определен MaxLength
			if (StrSize < MaxLength)
			{
				Length=MaxLength-StrSize;
			}
		}

		if (MaxLength == -1 || StrSize+Length <= MaxLength)
		{
			if (CurPos>StrSize)
			{
				if ((NewStr=(char *)xf_realloc(Edit::Str,CurPos+1))==NULL)
					return;

				Edit::Str=NewStr;
				sprintf(&Edit::Str[StrSize],"%*s",CurPos-StrSize,"");
				//memset(Edit::Str+StrSize,' ',CurPos-StrSize);Edit::Str[CurPos+1]=0;
				StrSize=CurPos;
			}

			int TmpSize=StrSize-CurPos;
			char *TmpStr=new char[TmpSize+16];

			if (!TmpStr)
				return;

			memcpy(TmpStr,&Edit::Str[CurPos],TmpSize);
			StrSize+=Length;

			if ((NewStr=(char *)xf_realloc(Edit::Str,StrSize+1))==NULL)
			{
				delete[] TmpStr;
				return;
			}

			Edit::Str=NewStr;
			memcpy(&Edit::Str[CurPos],Str,Length);
			/* $ 15.08.2000 KM */
			PrevCurPos=CurPos;
			/* KM $ */
			CurPos+=Length;
			memcpy(Edit::Str+CurPos,TmpStr,TmpSize);
			Edit::Str[StrSize]=0;
			/* $ 13.07.2000 SVS
			   раз уж вызывали через new[]...
			*/
			delete[] TmpStr;
			/* SVS $*/

			if (TabExpandMode == EXPAND_ALLTABS)
				ReplaceTabs();
		}
		else
			MessageBeep(MB_ICONHAND);
	}
}


int Edit::GetLength()
{
	return(StrSize);
}


/* $ 12.08.2000 KM */
// Функция установки маски ввода в объект Edit
void Edit::SetInputMask(const char *InputMask)
{
	if (Mask)
		xf_free(Mask);

	if (InputMask && *InputMask)
	{
		if ((Mask=xf_strdup(InputMask)) == NULL)
			return;

		RefreshStrByMask(TRUE);
	}
	else
		Mask=NULL;
}


// Функция обновления состояния строки ввода по содержимому Mask
void Edit::RefreshStrByMask(int InitMode)
{
	int i;

	if (Mask && *Mask)
	{
		int MaskLen=(int)strlen(Mask);

		/* $12.11.2000 KM
		   Некоторые изменения в работе с маской.
		   Теперь Str не может быть длиннее Mask и
		   MaxLength будет равна длине маски.
		*/
		if (StrSize!=MaskLen)
		{
			char *NewStr=(char *)xf_realloc(Str,MaskLen+1);

			if (NewStr==NULL)
				return;

			Str=NewStr;

			for (i=StrSize; i<MaskLen; i++)
				Str[i]=' ';

			StrSize=MaxLength=MaskLen;
			Str[StrSize]=0;
		}

		/* KM $ */
		for (i=0; i<MaskLen; i++)
		{
			if (InitMode)
				Str[i]=' ';

			if (!CheckCharMask(Mask[i]))
				Str[i]=Mask[i];
		}
	}
}
/* KM $ */


int Edit::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	if ((MouseEvent->dwButtonState & 3)==0)
		return(FALSE);

	if (MouseEvent->dwMousePosition.X<X1 || MouseEvent->dwMousePosition.X>X2 ||
	        MouseEvent->dwMousePosition.Y!=Y1)
		return(FALSE);

	//SetClearFlag(0); // пусть едитор сам заботится о снятии клеар-текста?
	SetTabCurPos(MouseEvent->dwMousePosition.X - X1 + LeftPos);

	/* $ 05.09.2001 SVS
	  Для непостоянных блоков снимаем выделение
	  А ТАК ЛИ Я СДЕЛАЛ?????
	*/
	if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
		Select(-1,0);

	/* SVS $ */
	if (MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED)
	{
		static int PrevDoubleClick=0;
		static COORD PrevPosition={0,0};

		if (GetTickCount()-PrevDoubleClick<=GetDoubleClickTime() && MouseEvent->dwEventFlags!=MOUSE_MOVED &&
		        PrevPosition.X == MouseEvent->dwMousePosition.X && PrevPosition.Y == MouseEvent->dwMousePosition.Y)
		{
			Select(0,StrSize);
			PrevDoubleClick=0;
			PrevPosition.X=0;
			PrevPosition.Y=0;
		}

		if (MouseEvent->dwEventFlags==DOUBLE_CLICK)
		{
			ProcessKey(KEY_OP_SELWORD);
			PrevDoubleClick=GetTickCount();
			PrevPosition=MouseEvent->dwMousePosition;
		}
		else
		{
			PrevDoubleClick=0;
			PrevPosition.X=0;
			PrevPosition.Y=0;
		}
	}

	Show();
	return(TRUE);
}


/* $ 03.08.2000 KM
   Немного изменён алгоритм из-за необходимости
   добавления поиска целых слов.
*/
int Edit::Search(char *Str,int Position,int Case,int WholeWords,int Reverse)
{
	int I,J,Length=(int)strlen(Str);

	if (Reverse)
	{
		Position--;

		if (Position>=StrSize)
			Position=StrSize-1;

		if (Position<0)
			return(FALSE);
	}

	if (Position<StrSize && *Str)
		for (I=Position; Reverse && I>=0 || !Reverse && I<StrSize; Reverse ? I--:I++)
		{
			for (J=0;; J++)
			{
				if (Str[J]==0)
				{
					CurPos=I;
					return(TRUE);
				}

				/* $ 03.08.2000 KM
				   Добавлен кусок кода для работы при поиске целых слов
				*/
				if (WholeWords)
				{
					int ChLeft,ChRight;
					int locResultLeft=FALSE;
					int locResultRight=FALSE;
					ChLeft=(TableSet==NULL) ? Edit::Str[I-1]:TableSet->DecodeTable[Edit::Str[I-1]];

					/* $ 07.09.2000 KM
					   Исправление глюка при поиске по целым словам.
					*/
					if (I>0)
						locResultLeft=(IsSpace(ChLeft) || strchr(WordDiv,ChLeft)!=NULL);
					else
						locResultLeft=TRUE;

					/* KM $ */
					if (I+Length<StrSize)
					{
						ChRight=(TableSet==NULL) ? Edit::Str[I+Length]:TableSet->DecodeTable[Edit::Str[I+Length]];
						locResultRight=(IsSpace(ChRight) || strchr(WordDiv,ChRight)!=NULL);
					}
					else
						locResultRight=TRUE;

					if (!locResultLeft || !locResultRight)
						break;
				}

				/* $ KM */
				int Ch=(TableSet==NULL) ? Edit::Str[I+J]:TableSet->DecodeTable[Edit::Str[I+J]];

				if (Case)
				{
					if (Ch!=Str[J])
						break;
				}
				else
				{
					if (LocalUpper(Ch)!=LocalUpper(Str[J]))
						break;
				}
			}
		}

	return(FALSE);
}
/* KM $ */

void Edit::InsertTab()
{
	char *TabPtr;
	int Pos,S;

	if (Flags.Check(FEDITLINE_READONLY))
		return;

	Pos=CurPos;
	S=TabSize-(Pos % TabSize);

	if (SelStart!=-1)
	{
		if (Pos<=SelStart)
		{
			SelStart+=S-(Pos==SelStart?0:1);
		}

		if (SelEnd!=-1 && Pos<SelEnd)
		{
			SelEnd+=S;
		}
	}

	int PrevStrSize=StrSize;
	StrSize+=S;
	CurPos+=S;
	Str=(char *)xf_realloc(Str,StrSize+1);
	TabPtr=Str+Pos;
	memmove(TabPtr+S,TabPtr,PrevStrSize-Pos);
	memset(TabPtr,' ',S);
	Str[StrSize]=0;
}


void Edit::ReplaceTabs()
{
	char *TabPtr;
	int Pos=0,S;

	if (Flags.Check(FEDITLINE_READONLY))
		return;

	while ((TabPtr=(char *)memchr(Str+Pos,'\t',StrSize-Pos))!=NULL)
	{
		Pos=(int)(TabPtr-Str);
		S=TabSize-((int)(TabPtr-Str) % TabSize);

		if (SelStart!=-1)
		{
			if (Pos<=SelStart)
			{
				SelStart+=S-(Pos==SelStart?0:1);
			}

			if (SelEnd!=-1 && Pos<SelEnd)
			{
				SelEnd+=S-1;
			}
		}

		int PrevStrSize=StrSize;
		StrSize+=S-1;

		if (CurPos>Pos)
			CurPos+=S-1;

		Str=(char *)xf_realloc(Str,StrSize+1);
		TabPtr=Str+Pos;
		memmove(TabPtr+S,TabPtr+1,PrevStrSize-Pos);
		memset(TabPtr,' ',S);
		Str[StrSize]=0;
	}
}


int Edit::GetTabCurPos()
{
	return(RealPosToTab(CurPos));
}


void Edit::SetTabCurPos(int NewPos)
{
	/* $ 21.09.2003 KM
	   Уточнения работы с маской
	*/
	int Pos;

	if (Mask && *Mask)
	{
		char *ShortStr=new char[StrSize+1];

		if (ShortStr==NULL)
			return;

		xstrncpy(ShortStr,Str,StrSize);
		Pos=(int)strlen(RemoveTrailingSpaces(ShortStr));
		delete[] ShortStr;

		if (NewPos>Pos)
			NewPos=Pos;
	}

	/* KM $ */
	CurPos=TabPosToReal(NewPos);
}


int Edit::RealPosToTab(int Pos)
{
	int TabPos,I;

	if ((TabExpandMode == EXPAND_ALLTABS) || memchr(Str,'\t',StrSize)==NULL)
		return(Pos);

	/* $ 10.10.2004 KM
	   После исправления Bug #1122 привнесён баг с невозможностью
	   выйти за пределы строки в редакторе при установленном
	   Cursor beyond end of line.
	*/
	for (TabPos=0,I=0; I<Pos && ((Flags.Check(FEDITLINE_EDITBEYONDEND))?TRUE:Str[I]); I++)
		/* KM $ */
	{
		if (I>=StrSize)
		{
			TabPos+=Pos-I;
			break;
		}

		if (Str[I]=='\t')
			TabPos+=TabSize - (TabPos % TabSize);
		else
			TabPos++;
	}

	return(TabPos);
}


int Edit::TabPosToReal(int Pos)
{
	int TabPos,I;

	if ((TabExpandMode == EXPAND_ALLTABS) || memchr(Str,'\t',StrSize)==NULL)
		return(Pos);

	/* $ 10.10.2004 KM
	   После исправления Bug #1122 привнесён баг с невозможностью
	   выйти за пределы строки в редакторе при установленном
	   Cursor beyond end of line.
	*/
	for (TabPos=0,I=0; TabPos<Pos && ((Flags.Check(FEDITLINE_EDITBEYONDEND))?TRUE:Str[I]); I++)
		/* KM $ */
	{
		if (I>StrSize)
		{
			I+=Pos-TabPos;
			break;
		}

		if (Str[I]=='\t')
		{
			int NewTabPos=TabPos+TabSize - (TabPos % TabSize);

			if (NewTabPos>Pos)
				break;

			TabPos=NewTabPos;
		}
		else
			TabPos++;
	}

	return(I);
}


void Edit::Select(int Start,int End)
{
	SelStart=Start;
	SelEnd=End;

	/* $ 24.06.2002 SKV
	   Если начало выделения за концом строки, надо выделение снять.
	   17.09.2002 возвращаю обратно. Глюкодром.
	*/
	if (SelEnd<SelStart && SelEnd!=-1)
		/* SKV $ */
	{
		SelStart=-1;
		SelEnd=0;
	}

	if (SelStart==-1 && SelEnd==-1)
	{
		SelStart=-1;
		SelEnd=0;
	}

//  if (SelEnd>StrSize)
//    SelEnd=StrSize;
}


void Edit::AddSelect(int Start,int End)
{
	if (Start<SelStart || SelStart==-1)
		SelStart=Start;

	if (End==-1 || End>SelEnd && SelEnd!=-1)
		SelEnd=End;

	if (SelEnd>StrSize)
		SelEnd=StrSize;

	if (SelEnd<SelStart && SelEnd!=-1)
	{
		SelStart=-1;
		SelEnd=0;
	}
}


void Edit::GetSelection(int &Start,int &End)
{
	/* $ 17.09.2002 SKV
	  Мало того, что это нарушение правил OO design'а,
	  так это еще и источние багов.
	*/
	/*  if (SelEnd>StrSize+1)
	    SelEnd=StrSize+1;
	  if (SelStart>StrSize+1)
	    SelStart=StrSize+1;*/
	/* SKV $ */
	Start=SelStart;
	End=SelEnd;

	if (End>StrSize)
		End=-1;//StrSize;

	if (Start>StrSize)
		Start=StrSize;
}


void Edit::GetRealSelection(int &Start,int &End)
{
	Start=SelStart;
	End=SelEnd;
}


void Edit::DisableEncode(int Disable)
{
	EditEncodeDisabled=Disable;
}

int Edit::GetEncodeState()
{
	return EditEncodeDisabled;
}


void Edit::SetTables(struct CharTableSet *TableSet)
{
	Edit::TableSet=TableSet;
};

void Edit::DeleteBlock()
{
	/* $ 25.07.2000 tran
	   + drop-down */
	/* $ 03.07.2000 tran
	   + обработка ReadOnly */
	if (Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
		return;

	/* tran 03.07.2000 $ */
	/* tran 25.07.2000 $ */

	if (SelStart==-1 || SelStart>=SelEnd)
		return;

	/* $ 15.08.2000 KM
	   Учёт Mask
	*/
	PrevCurPos=CurPos;

	if (Mask && *Mask)
	{
		for (int i=SelStart; i<SelEnd; i++)
		{
			if (CheckCharMask(Mask[i]))
				Str[i]=' ';
		}

		/* $ 18.09.2000 SVS
		  Для Mask - забыли скорректировать позицию :-)
		*/
		CurPos=SelStart;
		/* SVS $*/
	}
	else
	{
		int From=SelStart,To=SelEnd;

		if (From>StrSize)From=StrSize;

		if (To>StrSize)To=StrSize;

		memmove(Str+From,Str+To,StrSize-To+1);
		StrSize-=To-From;

		if (CurPos>From)
			if (CurPos<To)
				CurPos=From;
			else
				CurPos-=To-From;

		Str=(char *)xf_realloc(Str,StrSize+1);
	}

	/* KM $ */
	SelStart=-1;
	SelEnd=0;
	Flags.Clear(FEDITLINE_MARKINGBLOCK);

	// OT: Проверка на корректность поведени строки при удалении и вставки
	if (Flags.Check((FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE)))
	{
		if (LeftPos>CurPos)
			LeftPos=CurPos;
	}
}


void Edit::AddColor(struct ColorItem *col)
{
	if ((ColorCount & 15)==0)
		ColorList=(ColorItem *)xf_realloc(ColorList,(ColorCount+16)*sizeof(*ColorList));

	ColorList[ColorCount++]=*col;
}


int Edit::DeleteColor(int ColorPos)
{
	int Src;

	if (ColorCount==0)
		return(FALSE);

	int Dest=0;

	for (Src=0; Src<ColorCount; Src++)
		if (ColorPos!=-1 && ColorList[Src].StartPos!=ColorPos)
		{
			if (Dest!=Src)
				ColorList[Dest]=ColorList[Src];

			Dest++;
		}

	int DelCount=ColorCount-Dest;
	ColorCount=Dest;

	if (ColorCount==0)
	{
		xf_free(ColorList);
		ColorList=NULL;
	}

	return(DelCount!=0);
}


int Edit::GetColor(struct ColorItem *col,int Item)
{
	if (Item >= ColorCount)
		return(FALSE);

	*col=ColorList[Item];
	return(TRUE);
}


void Edit::ApplyColor()
{
	int Col,I,SelColor0;

	for (Col=0; Col<ColorCount; Col++)
	{
		struct ColorItem *CurItem=ColorList+Col;
		int Attr=CurItem->Color;
		int Length=CurItem->EndPos-CurItem->StartPos+1;

		if (CurItem->StartPos+Length >= StrSize)
			Length=StrSize-CurItem->StartPos;

		int Start=RealPosToTab(CurItem->StartPos)-LeftPos;
		int LengthFind=CurItem->StartPos+Length >= StrSize?StrSize-CurItem->StartPos+1:Length;
		int CorrectPos=0;

		if (Attr&ECF_TAB1)
			Attr&=~ECF_TAB1;
		else
			CorrectPos=LengthFind > 0 && CurItem->StartPos < StrSize && memchr(Str+CurItem->StartPos,'\t',LengthFind)?1:0;

		int End=RealPosToTab(CurItem->EndPos+CorrectPos)-LeftPos;
		CHAR_INFO TextData[1024];

		if (Start<=X2 && End>=X1)
		{
			if (Start<X1)
				Start=X1;

			if (End>X2)
				End=X2;

			Length=End-Start+1;

			if (Length < X2)
				Length-=CorrectPos;

			if (Length > 0 && Length < sizeof(TextData)/sizeof(TextData[0]))
			{
				ScrBuf.Read(Start,Y1,End,Y1,TextData,sizeof(TextData));
				SelColor0=SelColor;

				if (SelColor >= COL_FIRSTPALETTECOLOR)
					SelColor0=Palette[SelColor-COL_FIRSTPALETTECOLOR];

				for (I=0; I < Length; I++)
					if (TextData[I].Attributes != SelColor0)
						TextData[I].Attributes=Attr;

				ScrBuf.Write(Start,Y1,TextData,Length);
			}
		}
	}
}

/* $ 24.09.2000 SVS $
  Функция Xlat - перекодировка по принципу QWERTY <-> ЙЦУКЕН
*/
/* $ 13.12.2000 SVS
   Дополнительный параметр в функции  Xlat()
*/
void Edit::Xlat(BOOL All)
{
	/* $ 13.12.2000 SVS
	   Для CmdLine - если нет выделения, преобразуем всю строку
	*/
	if (All && SelStart == -1 && SelEnd == 0)
	{
		::Xlat(Str,0,(int)strlen(Str),TableSet,Opt.XLat.Flags);
		Show();
		return;
	}

	/* SVS $ */

	/* $ 10.10.2000 IS
	   - иногда не работала конвертация из-за того, что было SelStart==SelEnd
	*/
	if (SelStart != -1 && SelStart != SelEnd)
		/* IS $ */
	{
		if (SelEnd == -1)
			SelEnd=(int)strlen(Str);

		::Xlat(Str,SelStart,SelEnd,TableSet,Opt.XLat.Flags);
		Show();
	}
	/* $ 25.11.2000 IS
	   Если нет выделения, то обработаем текущее слово. Слово определяется на
	   основе специальной группы разделителей.
	*/
	else
	{
		/* $ 10.12.2000 IS
		   Обрабатываем только то слово, на котором стоит курсор, или то слово, что
		   находится левее позиции курсора на 1 символ
		*/
		int start=CurPos, end, StrSize=(int)strlen(Str);
		BOOL DoXlat=TRUE;

		/* $ 12.01.2004 IS
		   Для сравнения с WordDiv используем IsWordDiv, а не strchr, т.к.
		   текущая кодировка может отличаться от кодировки WordDiv (которая OEM)
		*/
		if (IsWordDiv(TableSet,Opt.XLat.WordDivForXlat,Str[start]))
		{
			if (start) start--;

			DoXlat=(!IsWordDiv(TableSet,Opt.XLat.WordDivForXlat,Str[start]));
		}

		if (DoXlat)
		{
			while (start>=0 && !IsWordDiv(TableSet,Opt.XLat.WordDivForXlat,Str[start]))
				start--;

			start++;
			end=start+1;

			while (end<StrSize && !IsWordDiv(TableSet,Opt.XLat.WordDivForXlat,Str[end]))
				end++;

			::Xlat(Str,start,end,TableSet,Opt.XLat.Flags);
			Show();
		}
	}
}


/* $ 15.11.2000 KM
   Проверяет: попадает ли символ в разрешённый
   диапазон символов, пропускаемых маской
*/
int Edit::KeyMatchedMask(int Key)
{
	int Inserted=FALSE;

	if (Mask[CurPos]==EDMASK_ANY)
		Inserted=TRUE;
	else if (Mask[CurPos]==EDMASK_DSS && (isdigit(Key) || Key==' ' || Key=='-'))
		Inserted=TRUE;
	/* $ 15.11.2000 KM
	   Убрано разрешение пробелов в цифровой маске.
	*/
	else if (Mask[CurPos]==EDMASK_DIGIT && (isdigit(Key)))
		Inserted=TRUE;
	else if (Mask[CurPos]==EDMASK_ALPHA && LocalIsalpha(Key))
		Inserted=TRUE;
	/* $ 20.09.2003 KM
	   Добавлена поддержка hex-символов.
	*/
	else if (Mask[CurPos]==EDMASK_HEX && (isdigit(Key) || (LocalUpper(Key)>='A' && LocalUpper(Key)<='F') || (LocalUpper(Key)>='a' && LocalUpper(Key)<='f')))
		Inserted=TRUE;

	return Inserted;
}

int Edit::CheckCharMask(char Chr)
{
	return (Chr==EDMASK_ANY || Chr==EDMASK_DIGIT || Chr==EDMASK_DSS || Chr==EDMASK_ALPHA || Chr==EDMASK_HEX)?TRUE:FALSE;
}

void Edit::SetDialogParent(DWORD Sets)
{
	if ((Sets&(FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE)) == (FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE) ||
	        (Sets&(FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE)) == 0)
		Flags.Clear(FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE);
	else if (Sets&FEDITLINE_PARENT_SINGLELINE)
	{
		Flags.Clear(FEDITLINE_PARENT_MULTILINE);
		Flags.Set(FEDITLINE_PARENT_SINGLELINE);
	}
	else if (Sets&FEDITLINE_PARENT_MULTILINE)
	{
		Flags.Clear(FEDITLINE_PARENT_SINGLELINE);
		Flags.Set(FEDITLINE_PARENT_MULTILINE);
	}
}
