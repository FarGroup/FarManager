/*
edit.cpp

Реализация одиночной строки редактирования

*/

/* Revision: 1.150 25.05.2006 $ */

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

enum {EOL_NONE,EOL_CR,EOL_LF,EOL_CRLF};
static const wchar_t *EOL_TYPE_CHARS_W[]={L"",L"\r",L"\n",L"\r\n"};

#define EDMASK_ANY_W   L'X' // позволяет вводить в строку ввода любой символ;
#define EDMASK_DSS_W   L'#' // позволяет вводить в строку ввода цифры, пробел и знак минуса;
#define EDMASK_DIGIT_W L'9' // позволяет вводить в строку ввода только цифры;
#define EDMASK_ALPHA_W L'A' // позволяет вводить в строку ввода только буквы.
#define EDMASK_HEX_W   L'H' // позволяет вводить в строку ввода шестнадцатиричные символы.


Edit::Edit()
{
  Str=(wchar_t*) xf_malloc(2);
  /* SVS $ */
  StrSize=0;

  WordDiv=Opt.strWordDiv;

  *Str=0;

  Mask=NULL;
  PrevCurPos=0;

  CurPos=0;
  CursorPos=0;
  CursorSize=-1;
  TableSet=NULL;
  LeftPos=0;
  MaxLength=-1;
  SelStart=-1;
  SelEnd=0;
  Flags.Set(FEDITLINE_EDITBEYONDEND);
  Color=F_LIGHTGRAY|B_BLACK;
  SelColor=F_WHITE|B_BLACK;

  ColorUnChanged=COL_DIALOGEDITUNCHANGED;

  EndType=EOL_NONE;
  ColorList=NULL;
  ColorCount=0;

  TabSize=Opt.EdOpt.TabSize;

  TabExpandMode = EXPAND_NOTABS;

  Flags.Change(FEDITLINE_DELREMOVESBLOCKS,Opt.EdOpt.DelRemovesBlocks);
  Flags.Change(FEDITLINE_PERSISTENTBLOCKS,Opt.EdOpt.PersistentBlocks);
}


Edit::~Edit()
{
  if (ColorList)
    xf_free (ColorList);
  if (Mask)
    xf_free(Mask);
  if(Str)
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
      ::SetCursorType(1,CursorSize);
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
    int MaskLen=wcslen(Mask);
    for (i=Position;i<MaskLen && i>=0;i+=Where)
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
      for (i=Position;i>=0;i--)
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
      for (i=Position;i<MaskLen;i++)
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
  int EditLength;
  if (!Flags.Check(FEDITLINE_EDITBEYONDEND) && CurPos>StrSize && StrSize>=0)
    CurPos=StrSize;
  EditLength=ObjWidth;
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
#ifdef SHITHAPPENS
  ReplaceSpaces(0);
#endif

  if ( (TabExpandMode != EXPAND_ALLTABS) && wmemchr(Str,L'\t',StrSize)!=NULL)
  {
    wchar_t *SaveStr;
    /* $ 04.07.2002 SKV
      Выделение тоже нужно запомнить ...
    */
    int SaveSelStart=SelStart;
    int SaveSelEnd=SelEnd;
    /* SKV $ */
    int SaveStrSize=StrSize,SaveCurPos=CurPos;
    if ((SaveStr=new wchar_t[StrSize+1])==NULL)
      return;
    wmemcpy(SaveStr,Str,StrSize);
    ReplaceTabs();
    CursorPos=CurPos;
    if (CurPos-LeftPos>EditLength-1)
      LeftPos=CurPos-EditLength+1;
//    if (!EditOutDisabled)
      ShowString(Str,TabSelStart,TabSelEnd);
    wmemcpy(Str,SaveStr,SaveStrSize);
    Str[SaveStrSize]=0;
    /* $ 13.07.2000 SVS
       раз уж вызывали через new[]...
    */
    delete[] SaveStr;
    /* SVS $*/
    StrSize=SaveStrSize;
    CurPos=SaveCurPos;
    /* $ 04.07.2002 SKV
      ... и восстановать.
    */
    SelStart=SaveSelStart;
    SelEnd=SaveSelEnd;
    /* SKV $ */
    Str=(wchar_t *)xf_realloc(Str,(StrSize+1)*sizeof (wchar_t*));
  }
  else
  {
//    if (!EditOutDisabled)
      ShowString(Str,TabSelStart,TabSelEnd);
    CursorPos=CurPos;
  }
  /* $ 26.07.2000 tran
     при дроп-даун цвета нам не нужны */
  if ( !Flags.Check(FEDITLINE_DROPDOWNBOX) )
      ApplyColor();
  /* tran 26.07.2000 $ */

#ifdef SHITHAPPENS
  ReplaceSpaces(1);
#endif
}


void Edit::ShowString(const wchar_t *ShowStr,int TabSelStart,int TabSelEnd)
{
  int EditLength=ObjWidth;
  if (Flags.Check(FEDITLINE_PASSWORDMODE))
  {
    wchar_t *PswStr=new wchar_t[StrSize+1];
    if (PswStr==NULL)
      return;
    wmemset(PswStr,L'*',StrSize);
    PswStr[StrSize]=0;
    Flags.Clear(FEDITLINE_PASSWORDMODE);
    ShowString(PswStr,TabSelStart,TabSelEnd);
    Flags.Set(FEDITLINE_PASSWORDMODE);
    /* $ 13.07.2000 SVS
       раз уж вызывали через new[]...
    */
    delete[] PswStr;
    /* SVS $*/
    return;
  }
  wchar_t *SaveStr=NULL;
  if (TableSet)
  {
    SaveStr=new wchar_t[StrSize+1];
    if (SaveStr==NULL)
      return;
    wmemcpy(SaveStr,Str,StrSize);

    //DecodeString(ShowStr,(unsigned char*)TableSet->DecodeTable,StrSize); BUGBUG
  }
  if (wmemchr(Str,0,StrSize)!=0)
  {
    if (SaveStr==NULL)
    {
      SaveStr=new wchar_t[StrSize+1];
      if (SaveStr==NULL)
        return;
      wmemcpy(SaveStr,Str,StrSize);
    }
    for (int I=0;I<StrSize;I++)
      if (Str[I]==0)
        Str[I]=' ';
  }

  SetColor(Color);

  if (TabSelStart==-1)
  {
    if (Flags.Check(FEDITLINE_CLEARFLAG) && LeftPos<StrSize)
    {
      SetColor(ColorUnChanged);
      /* $ 21.09.2003 KM
         Уточнения работы с маской
      */
      int Len,Size;
      if (Mask && *Mask)
      {
        wchar_t *ShortStr=new wchar_t[StrSize+1];
        if (ShortStr==NULL)
          return;

        wcsncpy(ShortStr,ShowStr,StrSize);
        Len=wcslen(RemoveTrailingSpacesW(ShortStr));
        delete[] ShortStr;
        Size=Len;
      }
      else
      {
        Len=wcslen(&ShowStr[LeftPos]);
        Size=StrSize;
      }
      if(Len > EditLength)
        Len=EditLength;

      mprintfW(L"%-*.*s",Len,Len,&ShowStr[LeftPos]);
      SetColor(Color);

      int BlankLength=EditLength-(Size-LeftPos);
      /* KM $ */

      if (BlankLength > 0)
      {
        mprintfW(L"%*s",BlankLength,L"");
      }
    }
    else
      mprintfW(L"%-*.*s",EditLength,EditLength,LeftPos>StrSize ? L"":&ShowStr[LeftPos]);
  }
  else
  {
    wchar_t *OutStr=new wchar_t[EditLength+1];
    if (OutStr==NULL)
      return;
    if ((TabSelStart-=LeftPos)<0)
      TabSelStart=0;
    int AllString=(TabSelEnd==-1);
    if (AllString)
      TabSelEnd=EditLength;
    else
      if ((TabSelEnd-=LeftPos)<0)
        TabSelEnd=0;
    swprintf(OutStr,L"%-*.*s",EditLength,EditLength,LeftPos>StrSize ? L"":&ShowStr[LeftPos]);
    /* $ 24.08.2000 SVS
       ! У DropDowList`а выделение по полной программе - на всю видимую длину
         ДАЖЕ ЕСЛИ ПУСТАЯ СТРОКА
    */
    if (TabSelStart>=EditLength /*|| !AllString && TabSelStart>=StrSize*/ ||
        TabSelEnd<TabSelStart)
    {
      if(Flags.Check(FEDITLINE_DROPDOWNBOX))
      {
        SetColor(SelColor);
        mprintfW(L"%*s",X2-X1+1,OutStr);
      }
      else
        TextW(OutStr);
    }
    /* SVS $ */
    else
    {
      mprintfW(L"%.*s",TabSelStart,OutStr);
      SetColor(SelColor);
      /* $ 15.08.2000 SVS
         + У DropDowList`а выделение по полной программе - на всю видимую длину
      */
      if(!Flags.Check(FEDITLINE_DROPDOWNBOX))
      {
        mprintfW(L"%.*s",TabSelEnd-TabSelStart,OutStr+TabSelStart);
        if (TabSelEnd<EditLength)
        {
          //SetColor(Flags.Check(FEDITLINE_CLEARFLAG) ? SelColor:Color);
          SetColor(Color);
          TextW(OutStr+TabSelEnd);
        }
      }
      else
      {
        mprintfW(L"%*s",X2-X1+1,OutStr);
      }
      /* SVS $*/
    }
    /* $ 13.07.2000 SVS
       раз уж вызывали через new[]...
    */
    delete[] OutStr;
    /* SVS $*/
  }
  if (SaveStr)
  {
    wmemcpy(Str,SaveStr,StrSize);
    /* $ 13.07.2000 SVS
       раз уж вызывали через new[]...
    */
    delete[] SaveStr;
    /* SVS $*/
  }
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
  string strPathName;

  if (Key>=KEY_RCTRL0 && Key<=KEY_RCTRL9) // шорткаты?
  {
    string strPluginModule, strPluginFile, strPluginData;

    if (GetShortcutFolder(Key,&strPathName,&strPluginModule,&strPluginFile,&strPluginData))
      RetCode=TRUE;
  }
  else // Пути/имена?
  {
    RetCode=_MakePath1W(Key,strPathName,L"");
  }

  // Если что-нить получилось, именно его и вставим (PathName)
  if(RetCode)
  {
    if (Flags.Check(FEDITLINE_CLEARFLAG))
    {
      LeftPos=0;
      SetStringW(L"");
    }

    if (PrevSelStart!=-1)
    {
      SelStart=PrevSelStart;
      SelEnd=PrevSelEnd;
    }

    if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
      DeleteBlock();

//    if(TableSet)
//       EncodeString(PathName,(unsigned char*)TableSet->EncodeTable,strlen(PathName)); //BUGBUG

    InsertStringW (strPathName);

    Flags.Clear(FEDITLINE_CLEARFLAG);
  }

  return RetCode;
}


int Edit::ProcessKey(int Key)
{

  switch(Key)
  {
    case MCODE_C_EMPTY:
      return GetLength()==0;
    case MCODE_C_SELECTED:
      return SelStart != -1 && SelStart < SelEnd;
    case MCODE_C_EOF:
      return CurPos >= StrSize;
    case MCODE_C_BOF:
      return CurPos==0;
    case MCODE_V_ITEMCOUNT:
      return StrSize;
    case MCODE_V_CURPOS:
      return CursorPos+1;
  }

  int I;
  switch(Key)
  {
    case KEY_ADD:
      Key=L'+';
      break;
    case KEY_SUBTRACT:
      Key=L'-';
      break;
    case KEY_MULTIPLY:
      Key=L'*';
      break;
    case KEY_DIVIDE:
      Key=L'/';
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
  if ( !Flags.Check(FEDITLINE_DROPDOWNBOX) && Key==KEY_CTRLL )
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
  if (((Key==KEY_BS || Key==KEY_DEL) && Flags.Check(FEDITLINE_DELREMOVESBLOCKS) || Key==KEY_CTRLD) &&
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
      Key!=KEY_KILLFOCUS && Key != KEY_GOTFOCUS &&
      ((Key&(~0xFF000000)) != KEY_LWIN && (Key&(~0xFF000000)) != KEY_RWIN && (Key&(~0xFF000000)) != KEY_APPS)
     )
  {
    Flags.Clear(FEDITLINE_MARKINGBLOCK); // хмм... а это здесь должно быть?

    if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS) && !(Key==KEY_CTRLINS || Key==KEY_CTRLNUMPAD0) &&
        Key!=KEY_SHIFTDEL && !Flags.Check(FEDITLINE_EDITORMODE) && Key != KEY_CTRLQ &&
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

//  if (!EditEncodeDisabled && Key<256 && TableSet && !ReturnAltValue) BUGBUG
//    Key=TableSet->EncodeTable[Key];

  /* $ 11.09.2000 SVS
     если Opt.DlgEULBsClear = 1, то BS в диалогах для UnChanged строки
     удаляет такую строку также, как и Del
  */
  if (((Opt.Dialogs.EULBsClear && Key==KEY_BS) || Key==KEY_DEL) &&
     Flags.Check(FEDITLINE_CLEARFLAG) && CurPos>=StrSize)
    Key=KEY_CTRLY;
  /* SVS $ */
  /* $ 15.09.2000 SVS
     Bug - Выделяем кусочек строки -> Shift-Del удяляет всю строку
           Так должно быть только для UnChanged состояния
  */
  if(Key == KEY_SHIFTDEL && Flags.Check(FEDITLINE_CLEARFLAG) && CurPos>=StrSize && SelStart==-1)
  {
    SelStart=0;
    SelEnd=StrSize;
  }
  /* SVS $ */

  if (Flags.Check(FEDITLINE_CLEARFLAG) && (Key<256 && Key!=KEY_BS || Key==KEY_CTRLBRACKET ||
      Key==KEY_CTRLBACKBRACKET || Key==KEY_CTRLSHIFTBRACKET ||
      Key==KEY_CTRLSHIFTBACKBRACKET || Key==KEY_SHIFTENTER))
  {
    LeftPos=0;
    SetStringW(L"");
    Show();
  }

  // Здесь - вызов функции вставки путей/файлов
  if(ProcessInsPath(Key,PrevSelStart,PrevSelEnd))
  {
    Show();
    return TRUE;
  }

  if (Key!=KEY_NONE && Key!=KEY_IDLE && Key!=KEY_SHIFTINS && Key!=KEY_SHIFTNUMPAD0 && Key!=KEY_CTRLINS &&
      (Key<KEY_F1 || Key>KEY_F12) && Key!=KEY_ALT && Key!=KEY_SHIFT &&
      Key!=KEY_CTRL && Key!=KEY_RALT && Key!=KEY_RCTRL &&
      (Key<KEY_ALT_BASE || Key>=KEY_ALT_BASE+256) &&
      (Key<KEY_MACRO_BASE || Key>KEY_MACRO_ENDBASE) && Key!=KEY_CTRLQ)
  {
    Flags.Clear(FEDITLINE_CLEARFLAG);
    Show();
  }


  switch(Key)
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
      while (CurPos>0 && !(!IsWordDivW(TableSet, WordDiv, Str[CurPos]) &&
             IsWordDivW(TableSet, WordDiv,Str[CurPos-1]) && !IsSpace(Str[CurPos])))
      {
        /* $ 18.08.2000 KM
           Добавим выход из цикла проверив CurPos-1 на присутствие
           в WordDiv.
        */
//        if (!IsSpace(Str[CurPos]) && IsSpace(Str[CurPos-1]))
        if (!IsSpaceW(Str[CurPos]) && (IsSpaceW(Str[CurPos-1]) ||
             IsWordDivW(TableSet, WordDiv, Str[CurPos-1])))
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
      while (CurPos<StrSize && !(IsWordDivW(TableSet, WordDiv, Str[CurPos]) &&
             !IsWordDivW(TableSet, WordDiv, Str[CurPos-1])))
      {
        /* $ 18.08.2000 KM
           Добавим выход из цикла проверив CurPos-1 на присутствие
           в WordDiv.
        */
//        if (!IsSpace(Str[CurPos]) && IsSpace(Str[CurPos-1]))
        if (!IsSpaceW(Str[CurPos]) && (IsSpaceW(Str[CurPos-1]) || IsWordDivW(TableSet, WordDiv, Str[CurPos-1])))
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
      Lock ();
      while (CurPos>0)
        RecurseProcessKey(KEY_SHIFTLEFT);
      Unlock ();
      Show();
      return(TRUE);
    }

    case KEY_SHIFTEND:  case KEY_SHIFTNUMPAD1:
    {
      Lock ();

      /* $ 21.09.2003 KM
         Уточнения работы с маской
      */
      int Len;

      if (Mask && *Mask)
      {
        wchar_t *ShortStr=new wchar_t[StrSize+1];
        if (ShortStr==NULL)
          return FALSE;

        wcsncpy(ShortStr,Str,StrSize);
        Len=wcslen(RemoveTrailingSpacesW(ShortStr));
        delete[] ShortStr;
      }
      else
        Len=StrSize;

      int LastCurPos=CurPos;
      while (CurPos<Len/*StrSize*/)
      {
        RecurseProcessKey(KEY_SHIFTRIGHT);
        if(LastCurPos==CurPos)break;
        LastCurPos=CurPos;
      }
      /* KM $ */

      Unlock ();
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
      for (i=CurPos;i>=0;i--)
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
      Lock ();
//      while (CurPos>0 && IsSpace(Str[CurPos-1]))
//        RecurseProcessKey(KEY_BS);
      while (1)
      {
        int StopDelete=FALSE;
        if (CurPos>1 && IsSpaceW(Str[CurPos-1])!=IsSpaceW(Str[CurPos-2]))
          StopDelete=TRUE;
        RecurseProcessKey(KEY_BS);
        if (CurPos==0 || StopDelete)
          break;
        /* $ 12.01.2004 IS
           Для сравнения с WordDiv используем IsWordDiv, а не strchr, т.к.
           текущая кодировка может отличаться от кодировки WordDiv (которая OEM)
        */
        if (IsWordDivW(TableSet, WordDiv,Str[CurPos-1]))
        /* IS $ */
          break;
      }
      Unlock ();
      Show();
      return(TRUE);
    }

    case KEY_CTRLQ:
    {
      Lock ();
      if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS) && (SelStart != -1 || Flags.Check(FEDITLINE_CLEARFLAG)))
        RecurseProcessKey(KEY_DEL);
      ProcessCtrlQ();
      Unlock ();
      Show();
      return(TRUE);
    }

#if defined(MOUSEKEY)
    case MCODE_OP_SELWORD:
    {
      int OldCurPos=CurPos;
      int SStart, SEnd;
      PrevSelStart=SelStart;
      PrevSelEnd=SelEnd;

      if(CurPos >= SelStart && CurPos <= SelEnd)
      { // выделяем ВСЮ строку при повторном двойном клике
        Select(0,StrSize);
      }
      else
      {
        int SStart, SEnd;
        CalcWordFromString(Str,CurPos,&SStart,&SEnd,TableSet,WordDiv);
        Select(SStart,++SEnd);
      }
      CurPos=OldCurPos; // возвращаем обратно
      Show();
      return TRUE;
    }
#endif

    case MCODE_OP_DATE:
    case MCODE_OP_PLAINTEXT:
    {
      if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
      {
        if(SelStart != -1 || Flags.Check(FEDITLINE_CLEARFLAG)) // BugZ#1053 - Неточности в $Text
          RecurseProcessKey(KEY_DEL);
      }
      if(Key == MCODE_OP_DATE)
        ProcessInsDate();
      else
        ProcessInsPlainText();
      Show();
      return TRUE;
    }

    case KEY_CTRLT:
    case KEY_CTRLDEL:
    {
      if (CurPos>=StrSize)
        return(FALSE);
      Lock ();
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
        int MaskLen=wcslen(Mask);
        int ptr=CurPos;
        while(ptr<MaskLen)
        {
          ptr++;
          /* $ 12.01.2004 IS
             Для сравнения с WordDiv используем IsWordDiv, а не strchr, т.к.
             текущая кодировка может отличаться от кодировки WordDiv (которая OEM)
          */
          if (!CheckCharMask(Mask[ptr]) ||
             (IsSpaceW(Str[ptr]) && !IsSpaceW(Str[ptr+1])) ||
             (IsWordDivW(TableSet, WordDiv, Str[ptr])))
          /* IS $ */
            break;
        }
        for (int i=0;i<ptr-CurPos;i++)
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
          if (IsWordDivW(TableSet, WordDiv, Str[CurPos]))
          /* IS $ */
            break;
        }
      }
      Unlock ();
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
      Str=(wchar_t *)xf_realloc(Str,1*sizeof(wchar_t));
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
      Str=(wchar_t *)xf_realloc(Str,(StrSize+1)*sizeof (wchar_t));
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
        wchar_t *ShortStr=new wchar_t[StrSize+1];
        if (ShortStr==NULL)
          return FALSE;
        wcsncpy(ShortStr,Str,StrSize);
        CurPos=wcslen(RemoveTrailingSpacesW(ShortStr));
        delete[] ShortStr;
      }
      else
        CurPos=StrSize;
      /* KM $ */

      Show();
      return(TRUE);
    }

    case KEY_LEFT:        case KEY_NUMPAD4:
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

    case KEY_RIGHT:       case KEY_NUMPAD6:
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
        wchar_t *ShortStr=new wchar_t[StrSize+1];
        if (ShortStr==NULL)
          return FALSE;
        wcsncpy(ShortStr,Str,StrSize);
        int Len=wcslen(RemoveTrailingSpacesW(ShortStr));
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
        int MaskLen=wcslen(Mask);
        int i,j;
        for (i=CurPos,j=CurPos;i<MaskLen;i++)
        {
          if (CheckCharMask(Mask[i+1]))
          {
            while(!CheckCharMask(Mask[j]) && j<MaskLen)
              j++;
            Str[j]=Str[i+1];
            j++;
          }
        }
        Str[j]=' ';
      }
      else
      {
        wmemmove(Str+CurPos,Str+CurPos+1,StrSize-CurPos);
        StrSize--;
        Str=(wchar_t *)xf_realloc(Str,(StrSize+1)*sizeof (wchar_t));
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
      while (CurPos>0 && !(!IsWordDivW(TableSet, WordDiv, Str[CurPos]) &&
             IsWordDivW(TableSet, WordDiv, Str[CurPos-1]) && !IsSpaceW(Str[CurPos])))
      /* IS $ */
      {
        if (!IsSpaceW(Str[CurPos]) && IsSpaceW(Str[CurPos-1]))
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
        wchar_t *ShortStr=new wchar_t[StrSize+1];
        if (ShortStr==NULL)
          return FALSE;
        wcsncpy(ShortStr,Str,StrSize);
        Len=wcslen(RemoveTrailingSpacesW(ShortStr));
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
      while (CurPos<Len/*StrSize*/ && !(IsWordDivW(TableSet, WordDiv,Str[CurPos]) &&
             !IsWordDivW(TableSet, WordDiv, Str[CurPos-1])))
      /* IS $ */
      /* KM $ */
      {
        if (!IsSpaceW(Str[CurPos]) && IsSpaceW(Str[CurPos-1]))
          break;
        CurPos++;
      }
      Show();
      return(TRUE);
    }

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
            wchar_t *ShortStr=new wchar_t[StrSize+1];
            if (ShortStr==NULL)
              return FALSE;
            wcsncpy(ShortStr,Str,StrSize);
            RemoveTrailingSpacesW(ShortStr);
            CopyToClipboardW(ShortStr);
            delete[] ShortStr;
          }
          else
            CopyToClipboardW(Str);
          /* KM $ */
        }
        else
          if (SelEnd<=StrSize) // TODO: если в начало условия добавить "StrSize &&", то пропадет баг "Ctrl-Ins в пустой строке очищает клипборд"
          {
            int Ch=Str[SelEnd];
            Str[SelEnd]=0;
            CopyToClipboardW(Str+SelStart);
            Str[SelEnd]=Ch;
          }
      return(TRUE);
    }

    case KEY_SHIFTINS:    case KEY_SHIFTNUMPAD0:
    {
        /* $ 15.10.2000 tran
           если строка ввода имет максимальную длину
           то их клипборда грузим не больше ее*/
        wchar_t *ClipText=NULL;

        if (MaxLength==-1)
            ClipText=PasteFromClipboardW();
        else
            ClipText=PasteFromClipboardExW(MaxLength);
        /* tran $ */
        if (ClipText==NULL)
          return(TRUE);
        if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS)){
          DeleteBlock();
        }

        for (I=wcslen(Str)-1;I>=0 && IsEolW(Str[I]);I--)
          Str[I]=0;
        for (I=0;ClipText[I];I++)
          if (IsEolW(ClipText[I]))
          {
            if (IsEolW(ClipText[I+1]))
              wmemmove(&ClipText[I],&ClipText[I+1],wcslen(&ClipText[I+1])+1);
            if (ClipText[I+1]==0)
              ClipText[I]=0;
            else
              ClipText[I]=L' ';
          }

        if (Flags.Check(FEDITLINE_CLEARFLAG))
        {
          LeftPos=0;
          SetStringW(ClipText);
          Flags.Clear(FEDITLINE_CLEARFLAG);
        }
        else
          InsertStringW(ClipText);
        /* $ 13.07.2000 SVS
           в PasteFromClipboard запрос памятиче через new[]
        */
        if(ClipText)
          delete[] ClipText;
        /* SVS $ */
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

      if (Key==KEY_NONE || Key==KEY_IDLE || Key==KEY_ENTER || Key>=65536 )
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

      if ( Key==KEY_TAB && (TabExpandMode == EXPAND_NEWTABS) )
      {
         InsertTab ();
         Show ();
      }
      else if (InsertKey(Key))
      {
        if (Key==KEY_TAB && (TabExpandMode == EXPAND_ALLTABS) )
           ReplaceTabs();
        Show();
      }
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

    if(Key==KEY_CONSOLE_BUFFER_RESIZE)
    {
//      int Dis=EditOutDisabled;
//      EditOutDisabled=0;
      Show();
//      EditOutDisabled=Dis;
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

int Edit::ProcessInsDate(void)
{
  const wchar_t *Fmt = eStackAsString();

  string strTStr;

  if(MkStrFTimeW(strTStr,Fmt))
  {
    InsertStringW(strTStr);
    return TRUE;
  }
  return FALSE;
}

int Edit::ProcessInsPlainText(void)
{
  const wchar_t *str = eStackAsString();
  if (*str)
  {
    InsertStringW(str);
    return TRUE;
  }

  return FALSE;
}

int Edit::InsertKey(int Key)
{
  wchar_t *NewStr;
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
    int MaskLen=wcslen(Mask);
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
          while(!CheckCharMask(Mask[i]) && i>CurPos)
            i--;

          for (int j=i;i>CurPos;i--)
          {
            if (CheckCharMask(Mask[i]))
            {
              while(!CheckCharMask(Mask[j-1]))
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
    if(MaxLength == -1 || StrSize < MaxLength)
    {
      if (CurPos>=StrSize)
      {
        if ((NewStr=(wchar_t *)xf_realloc(Str,(CurPos+2)*sizeof (wchar_t)))==NULL)
          return(FALSE);
        Str=NewStr;
        swprintf(&Str[StrSize],L"%*s",CurPos-StrSize,L"");
        //memset(Str+StrSize,' ',CurPos-StrSize);Str[CurPos+1]=0;
        StrSize=CurPos+1;
      }
      else
        if (!Flags.Check(FEDITLINE_OVERTYPE))
          StrSize++;
      if ((NewStr=(wchar_t *)xf_realloc(Str,(StrSize+1)*sizeof (wchar_t)))==NULL)
        return(TRUE);
      Str=NewStr;

      if (!Flags.Check(FEDITLINE_OVERTYPE))
      {
        wmemmove(Str+CurPos+1,Str+CurPos,StrSize-CurPos);
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
      if(CurPos < StrSize)
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


void Edit::GetStringW(wchar_t *Str,int MaxSize)
{
    xwcsncpy(Str, Edit::Str,MaxSize-1);
    Str[MaxSize-1]=0;
}

void Edit::GetStringW(string &strStr)
{
    strStr = Edit::Str;
}


const wchar_t* Edit::GetStringAddrW()
{
    return Str;
}



void Edit::SetStringW(const wchar_t *Str)
{
  /* $ 03.07.2000 tran
     + обработка ReadOnly */
  if ( Flags.Check(FEDITLINE_READONLY) )
    return;
  /* tran 03.07.2000 $ */
  Select(-1,0);
  SetBinaryStringW(Str,wcslen(Str));
}

void Edit::SetEOLW(const wchar_t *EOL)
{
  if (EOL[0]==L'\r')
    if (EOL[1]==L'\n')
      EndType=EOL_CRLF;
    else
      EndType=EOL_CR;
  else
    if (EOL[0]==L'\n')
      EndType=EOL_LF;
    else
      EndType=EOL_NONE;
}

/* $ 25.07.2000 tran
   примечание:
   в этом методе DropDownBox не обрабатывается
   ибо он вызывается только из SetString и из класса Editor
   в Dialog он нигде не вызывается */
void Edit::SetBinaryStringW(const wchar_t *Str,int Length)
{
  /* $ 03.07.2000 tran
     + обработка ReadOnly */
  if ( Flags.Check(FEDITLINE_READONLY) )
    return;
  /* tran 03.07.2000 $ */

  // коррекция вставляемого размера, если определен MaxLength
  if(MaxLength != -1 && Length > MaxLength)
  {
    Length=MaxLength; // ??
  }

  if (Length>0 && !Flags.Check(FEDITLINE_PARENT_SINGLELINE))
  {
    if (Str[Length-1]==L'\r')
    {
      EndType=EOL_CR;
      Length--;
    }
    else
    {
      if (Str[Length-1]==L'\n')
      {
        Length--;
        if (Length>0 && Str[Length-1]==L'\r')
        {
          EndType=EOL_CRLF;
          Length--;
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
    /* $ 21.09.2003 KM
       Очистка строки с маской.
    */
    RefreshStrByMask(TRUE);
    /* KM $ */

    /* $ 26.10.2003 KM
       ! Скорректируем вставку из клипборда с учётом маски
    */
    int maskLen=wcslen(Mask);
    for (int i=0,j=0;j<maskLen && j<Length;)
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
    wchar_t *NewStr=(wchar_t *)xf_realloc(Edit::Str,(Length+1)*sizeof (wchar_t));
    if (NewStr==NULL)
      return;
    Edit::Str=NewStr;
    StrSize=Length;
    wmemcpy(Edit::Str,Str,Length);
    Edit::Str[Length]=0;

    if ( TabExpandMode == EXPAND_ALLTABS )
      ReplaceTabs ();
    PrevCurPos=CurPos;
    CurPos=StrSize;
  }
  /* KM $ */
}


void Edit::GetBinaryStringW(wchar_t *&Str,const wchar_t **EOL,int &Length)
{
    Str=Edit::Str;

    if (EOL!=NULL)
        *EOL=EOL_TYPE_CHARS_W[EndType];

    Length=StrSize; //???
}

void Edit::GetBinaryStringW(const wchar_t *&Str,const wchar_t **EOL,int &Length)
{
    Str=Edit::Str;

    if (EOL!=NULL)
        *EOL=EOL_TYPE_CHARS_W[EndType];

    Length=StrSize; //???
}

int Edit::GetSelStringW(wchar_t *Str, int MaxSize)
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

  wcsncpy(Str,Edit::Str+SelStart,CopyLength);
  Str[CopyLength]=0;

  return(TRUE);
}

int Edit::GetSelStringW (string &strStr)
{
  if (SelStart==-1 || SelEnd!=-1 && SelEnd<=SelStart ||
      SelStart>=StrSize)
  {
    strStr = L"";
    return(FALSE);
  }

  int CopyLength;

  CopyLength=SelEnd-SelStart; //??? BUGBUG

  wchar_t *lpwszStr = strStr.GetBuffer (CopyLength);

  wcsncpy(lpwszStr,Edit::Str+SelStart,CopyLength);
  lpwszStr[CopyLength]=0;

  strStr.ReleaseBuffer ();

  return(TRUE);
}



void Edit::InsertStringW(const wchar_t *Str)
{
  /* $ 25.07.2000 tran
     + drop-down */
  /* $ 03.07.2000 tran
     + обработка ReadOnly */
  if (Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
    return;
  /* tran 03.07.2000 $ */
  /* tran 25.07.2000 $ */

  Select(-1,0);
  InsertBinaryStringW(Str,wcslen(Str));
}


void Edit::InsertBinaryStringW(const wchar_t *Str,int Length)
{
  wchar_t *NewStr;

  /* $ 25.07.2000 tran
     + drop-down */
  /* $ 03.07.2000 tran
     + обработка ReadOnly */
  if (Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
    return;
  /* tran 03.07.2000 $ */

  Flags.Clear(FEDITLINE_CLEARFLAG);

  /* $ 18.08.2000 KM
     Обслуживание маски ввода.
  */
  if (Mask && *Mask)
  {
    int Pos=CurPos;
    int MaskLen=wcslen(Mask);
    if (Pos<MaskLen)
    {
      //_SVS(SysLog("InsertBinaryString ==> Str='%s' (Length=%d) Mask='%s'",Str,Length,Mask+Pos));
      int StrLen=(MaskLen-Pos>Length)?Length:MaskLen-Pos;
      /* $ 15.11.2000 KM
         Внесены исправления для правильной работы PasteFromClipboard
         в строке с маской
      */
      for (int i=Pos,j=0;j<StrLen+Pos;)
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
    if(MaxLength != -1 && StrSize+Length > MaxLength)
    {
      // коррекция вставляемого размера, если определен MaxLength
      if(StrSize < MaxLength)
      {
        Length=MaxLength-StrSize;
      }
    }

    if(MaxLength == -1 || StrSize+Length <= MaxLength)
    {
      if (CurPos>StrSize)
      {
        if ((NewStr=(wchar_t *)xf_realloc(Edit::Str,(CurPos+1)*sizeof (wchar_t)))==NULL)
          return;
        Edit::Str=NewStr;
        swprintf(&Edit::Str[StrSize],L"%*s",CurPos-StrSize,L"");
        //memset(Edit::Str+StrSize,' ',CurPos-StrSize);Edit::Str[CurPos+1]=0;
        StrSize=CurPos;
      }

      int TmpSize=StrSize-CurPos;
      wchar_t *TmpStr=new wchar_t[TmpSize+16];
      if(!TmpStr)
        return;

      wmemcpy(TmpStr,&Edit::Str[CurPos],TmpSize);

      StrSize+=Length;
      if ((NewStr=(wchar_t *)xf_realloc(Edit::Str,(StrSize+1)*sizeof (wchar_t)))==NULL)
      {
        delete[] TmpStr;
        return;
      }
      Edit::Str=NewStr;
      wmemcpy(&Edit::Str[CurPos],Str,Length);
      /* $ 15.08.2000 KM */
      PrevCurPos=CurPos;
      /* KM $ */
      CurPos+=Length;
      wmemcpy(Edit::Str+CurPos,TmpStr,TmpSize);
      Edit::Str[StrSize]=0;
      /* $ 13.07.2000 SVS
         раз уж вызывали через new[]...
      */
      delete[] TmpStr;
      /* SVS $*/

      if ( TabExpandMode == EXPAND_ALLTABS )
        ReplaceTabs();
    }
    else
      MessageBeep(MB_ICONHAND);
  }
  /* KM $ */
}


int Edit::GetLength()
{
  return(StrSize);
}


/* $ 12.08.2000 KM */
// Функция установки маски ввода в объект Edit
void Edit::SetInputMaskW(const wchar_t *InputMask)
{
  if (Mask)
    xf_free(Mask);

  if (InputMask && *InputMask)
  {
    if((Mask=_wcsdup(InputMask)) == NULL)
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
    int MaskLen=wcslen(Mask);
    /* $12.11.2000 KM
       Некоторые изменения в работе с маской.
       Теперь Str не может быть длиннее Mask и
       MaxLength будет равна длине маски.
    */
    if (StrSize!=MaskLen)
    {
      wchar_t *NewStr=(wchar_t *)xf_realloc(Str,(MaskLen+1)*sizeof (wchar_t));
      if (NewStr==NULL)
        return;
      Str=NewStr;
      for (i=StrSize;i<MaskLen;i++)
        Str[i]=L' ';
      StrSize=MaxLength=MaskLen;
      Str[StrSize]=0;
    }
    /* KM $ */
    for (i=0;i<MaskLen;i++)
    {
      if (InitMode)
        Str[i]=L' ';
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
  Show();
  return(TRUE);
}


/* $ 03.08.2000 KM
   Немного изменён алгоритм из-за необходимости
   добавления поиска целых слов.
*/
int Edit::Search(const wchar_t *Str,int Position,int Case,int WholeWords,int Reverse)
{
/*  int I,J,Length=strlen(Str);
  if (Reverse)
  {
    Position--;
    if (Position>=StrSize)
      Position=StrSize-1;
    if (Position<0)
      return(FALSE);
  }
  if (Position<StrSize && *Str)
    for (I=Position;Reverse && I>=0 || !Reverse && I<StrSize;Reverse ? I--:I++)
    {
      for (J=0;;J++)
      {
        if (Str[J]==0)
        {
          CurPos=I;
          return(TRUE);
        }
        if (WholeWords)
        {
          int ChLeft,ChRight;
          int locResultLeft=FALSE;
          int locResultRight=FALSE;

          ChLeft=(TableSet==NULL) ? Edit::Str[I-1]:TableSet->DecodeTable[Edit::Str[I-1]];
          if (I>0)
            locResultLeft=(IsSpace(ChLeft) || strchr(WordDiv,ChLeft)!=NULL);
          else
            locResultLeft=TRUE;
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
    }*/ //BUGBUG
  return(FALSE);
}
/* KM $ */

void Edit::InsertTab()
{
  wchar_t *TabPtr;
  int Pos,S;
  /* $ 03.07.2000 tran
     + юсЁрсюЄър ReadOnly */
  if ( Flags.Check(FEDITLINE_READONLY) )
    return;
  /* tran 03.07.2000 $ */

  Pos=CurPos;
  S=TabSize-(Pos % TabSize);

  if(SelStart!=-1)
  {
    if(Pos<=SelStart)
    {
       SelStart+=S-(Pos==SelStart?0:1);
    }
    if(SelEnd!=-1 && Pos<SelEnd)
    {
      SelEnd+=S;
    }
  }

  int PrevStrSize=StrSize;
  StrSize+=S;

//  if (CurPos>Pos)
    CurPos+=S;

  Str=(wchar_t *)xf_realloc(Str,(StrSize+1)*sizeof (wchar_t));

  TabPtr=Str+Pos;

  wmemmove(TabPtr+S,TabPtr,PrevStrSize-Pos);
  wmemset(TabPtr,L' ',S);

  Str[StrSize]=0;
}


void Edit::ReplaceTabs()
{
  wchar_t *TabPtr;
  int Pos,S;
  /* $ 03.07.2000 tran
     + обработка ReadOnly */
  if ( Flags.Check(FEDITLINE_READONLY) )
    return;
  /* tran 03.07.2000 $ */

  while ((TabPtr=(wchar_t *)wmemchr(Str,L'\t',StrSize))!=NULL)
  {
    Pos=TabPtr-Str;
    S=TabSize-((TabPtr-Str) % TabSize);
    if(SelStart!=-1)
    {
      if(Pos<=SelStart)
      {
        SelStart+=S-(Pos==SelStart?0:1);
      }
      if(SelEnd!=-1 && Pos<SelEnd)
      {
        SelEnd+=S-1;
      }
    }
    int PrevStrSize=StrSize;
    StrSize+=S-1;
    if (CurPos>Pos)
      CurPos+=S-1;
    Str=(wchar_t *)xf_realloc(Str,(StrSize+1)*sizeof (wchar_t));
    TabPtr=Str+Pos;
    wmemmove(TabPtr+S,TabPtr+1,PrevStrSize-Pos);
    wmemset(TabPtr,L' ',S);
#ifdef SHITHAPPENS
    wmemset(TabPtr,0x01,S);
    *TabPtr=0x02;
#endif
    Str[StrSize]=0;
  }
}

#ifdef SHITHAPPENS
void Edit::ReplaceSpaces(int i)
{
  char *TabPtr;
  int Pos,S;
  char a,b;
  if ( i==0 )
  {
    a=' '; b=0xFA;
  }
  else
  {
    b=' '; a=0xFA;
  }
  /* $ 03.07.2000 tran
     + обработка ReadOnly */
  if ( Flags.Check(FEDITLINE_READONLY) )
    return;
  /* tran 03.07.2000 $ */

  while ((TabPtr=(char *)memchr(Str,a,StrSize))!=NULL)
  {
    *TabPtr=b;
  }
}
#endif


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
    wchar_t *ShortStr=new wchar_t[StrSize+1];
    if (ShortStr==NULL)
      return;
    wcsncpy(ShortStr,Str,StrSize);
    Pos=wcslen(RemoveTrailingSpacesW(ShortStr));
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

  if ( (TabExpandMode == EXPAND_ALLTABS) || wmemchr(Str,L'\t',StrSize)==NULL)
    return(Pos);


  /* $ 10.10.2004 KM
     После исправления Bug #1122 привнесён баг с невозможностью
     выйти за пределы строки в редакторе при установленном
     Cursor beyond end of line.
  */
  for (TabPos=0,I=0;I<Pos && ((Flags.Check(FEDITLINE_EDITBEYONDEND))?TRUE:Str[I]);I++)
  /* KM $ */
  {
    if (I>=StrSize)
    {
      TabPos+=Pos-I;
      break;
    }
    if (Str[I]==L'\t')
      TabPos+=TabSize - (TabPos % TabSize);
    else
      TabPos++;
  }
  return(TabPos);
}


int Edit::TabPosToReal(int Pos)
{
  int TabPos,I;

  if ( (TabExpandMode == EXPAND_ALLTABS) || wmemchr(Str,L'\t',StrSize)==NULL)
    return(Pos);


  /* $ 10.10.2004 KM
     После исправления Bug #1122 привнесён баг с невозможностью
     выйти за пределы строки в редакторе при установленном
     Cursor beyond end of line.
  */
  for (TabPos=0,I=0;TabPos<Pos && ((Flags.Check(FEDITLINE_EDITBEYONDEND))?TRUE:Str[I]);I++)
  /* KM $ */
  {
    if (I>StrSize)
    {
      I+=Pos-TabPos;
      break;
    }
    if (Str[I]==L'\t')
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
    for (int i=SelStart;i<SelEnd;i++)
    {
      if (CheckCharMask(Mask[i]))
        Str[i]=L' ';
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
    if(From>StrSize)From=StrSize;
    if(To>StrSize)To=StrSize;
    wmemmove(Str+From,Str+To,StrSize-To+1);
    StrSize-=To-From;
    if (CurPos>From)
      if (CurPos<To)
        CurPos=From;
      else
        CurPos-=To-From;
    Str=(wchar_t *)xf_realloc(Str,(StrSize+1)*sizeof (wchar_t));
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
  for (Src=0;Src<ColorCount;Src++)
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
    xf_free (ColorList);
    ColorList=NULL;
  }
  return(DelCount!=0);
}


int Edit::GetColor(struct ColorItem *col,int Item)
{
  if ((DWORD)Item >= ColorCount)
    return(FALSE);
  *col=ColorList[Item];
  return(TRUE);
}


void Edit::ApplyColor()
{
  int Col,I,SelColor0;

  for (Col=0;Col<ColorCount;Col++)
  {
    struct ColorItem *CurItem=ColorList+Col;
    int Attr=CurItem->Color;
    int Length=CurItem->EndPos-CurItem->StartPos+1;
    if(CurItem->StartPos+Length >= StrSize)
      Length=StrSize-CurItem->StartPos;

    int Start=RealPosToTab(CurItem->StartPos)-LeftPos;
    int LengthFind=CurItem->StartPos+Length >= StrSize?StrSize-CurItem->StartPos+1:Length;
    int CorrectPos=0;

    if(Attr&ECF_TAB1)
      Attr&=~ECF_TAB1;
    else
      CorrectPos=LengthFind > 0 && CurItem->StartPos < StrSize && wmemchr(Str+CurItem->StartPos,L'\t',LengthFind)?1:0;

    int End=RealPosToTab(CurItem->EndPos+CorrectPos)-LeftPos;

    CHAR_INFO TextData[1024];
    if (Start<=X2 && End>=X1)
    {
      if (Start<X1)
        Start=X1;

      if (End>X2)
        End=X2;

      Length=End-Start+1;

      if(Length < X2)
        Length-=CorrectPos;

      if (Length > 0 && Length < sizeof(TextData))
      {
        ScrBuf.Read(Start,Y1,End,Y1,TextData,sizeof(TextData));

        SelColor0=SelColor;

        if(SelColor >= COL_FIRSTPALETTECOLOR)
          SelColor0=Palette[SelColor-COL_FIRSTPALETTECOLOR];

        for (I=0;I < Length;I++)
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
  if(All && SelStart == -1 && SelEnd == 0)
  {
    ::XlatW(Str,0,wcslen(Str),TableSet,Opt.XLat.Flags);
    Show();
    return;
  }
  /* SVS $ */
  /* $ 10.10.2000 IS
     - иногда не работала конвертация из-за того, что было SelStart==SelEnd
  */
  if(SelStart != -1 && SelStart != SelEnd)
  /* IS $ */
  {
    if(SelEnd == -1)
      SelEnd=wcslen(Str);
    ::XlatW(Str,SelStart,SelEnd,TableSet,Opt.XLat.Flags);
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
   int start=CurPos, end, StrSize=wcslen(Str);
   BOOL DoXlat=TRUE;

   /* $ 12.01.2004 IS
      Для сравнения с WordDiv используем IsWordDiv, а не strchr, т.к.
      текущая кодировка может отличаться от кодировки WordDiv (которая OEM)
   */
   if(IsWordDivW(TableSet,Opt.XLat.strWordDivForXlat,Str[start]))
   {
      if(start) start--;
      DoXlat=(!IsWordDivW(TableSet,Opt.XLat.strWordDivForXlat,Str[start]));
   }

   if(DoXlat)
   {
    while(start>=0 && !IsWordDivW(TableSet,Opt.XLat.strWordDivForXlat,Str[start]))
      start--;
    start++;
    end=start+1;
    while(end<StrSize && !IsWordDivW(TableSet,Opt.XLat.strWordDivForXlat,Str[end]))
      end++;
    ::XlatW(Str,start,end,TableSet,Opt.XLat.Flags);
    Show();
   }
   /* 12.01.2004 IS $*/
   /* 10.12.2000 IS $ */
  }
/* IS $ */
}
/* SVS $ */


/* $ 15.11.2000 KM
   Проверяет: попадает ли символ в разрешённый
   диапазон символов, пропускаемых маской
*/
int Edit::KeyMatchedMask(int Key)
{
  int Inserted=FALSE;
  if (Mask[CurPos]==EDMASK_ANY_W)
    Inserted=TRUE;
  else if (Mask[CurPos]==EDMASK_DSS_W && (iswdigit(Key) || Key==L' ' || Key==L'-'))
    Inserted=TRUE;
  /* $ 15.11.2000 KM
     Убрано разрешение пробелов в цифровой маске.
  */
  else if (Mask[CurPos]==EDMASK_DIGIT_W && (iswdigit(Key)))
    Inserted=TRUE;
  /* KM $ */
  else if (Mask[CurPos]==EDMASK_ALPHA_W && LocalIsalphaW(Key))
    Inserted=TRUE;
  /* $ 20.09.2003 KM
     Добавлена поддержка hex-символов.
  */
  else if (Mask[CurPos]==EDMASK_HEX_W && (iswdigit(Key) || (LocalUpperW(Key)>=L'A' && LocalUpperW(Key)<=L'F') || (LocalUpperW(Key)>=L'a' && LocalUpperW(Key)<=L'f')))
    Inserted=TRUE;
  /* KM $ */

  return Inserted;
}
/* KM $ */

int Edit::CheckCharMask(wchar_t Chr)
{
  return (Chr==EDMASK_ANY_W || Chr==EDMASK_DIGIT_W || Chr==EDMASK_DSS_W || Chr==EDMASK_ALPHA_W || Chr==EDMASK_HEX_W)?TRUE:FALSE;
}

void Edit::SetDialogParent(DWORD Sets)
{
  if((Sets&(FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE)) == (FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE) ||
     (Sets&(FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE)) == 0)
    Flags.Clear(FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE);
  else if(Sets&FEDITLINE_PARENT_SINGLELINE)
  {
    Flags.Clear(FEDITLINE_PARENT_MULTILINE);
    Flags.Set(FEDITLINE_PARENT_SINGLELINE);
  }
  else if(Sets&FEDITLINE_PARENT_MULTILINE)
  {
    Flags.Clear(FEDITLINE_PARENT_SINGLELINE);
    Flags.Set(FEDITLINE_PARENT_MULTILINE);
  }
}
