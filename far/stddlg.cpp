/*
stddlg.cpp

Куча разных стандартных диалогов

*/

/* Revision: 1.13 25.06.2001 $ */

/*
Modify:
  25.06.2001 IS
   ! Внедрение const
  11.06.2001 SVS
    ! Новые параметры у GetSearchReplaceString() - указывающие размеры буферов
  16.05.2001 SVS
    ! DumpExceptionInfo заменен на xfilter
  07.05.2001 SVS
    ! SysLog(); -> _D(SysLog());
  06.05.2001 DJ
    ! перетрях #include
  16.03.2001 SVS
    + GetNameAndPassword();
  13.03.2001 SVS
    - в предыдущем патче неверно работали макросы - не была учтена ситуация
      с макросами.
  12.03.2001 SVS
    ! Грязный Хак в функции GetString :-)
  12.02.2001 SVS
    ! Ops. Баги в GetString :-)
  11.02.2001 SVS
    ! Изменения в GetString с учетом флага DIF_VAREDIT
  28.01.2001 SVS
    ! DumpExeptionInfo -> DumpExceptionInfo ;-)
  23.01.2001 SVS
    + добавим немного эксепшина :-)
  23.01.2001 SVS
    - Ну вот и первая бага в диалоге поиска/замены :-(
  21.01.2001 SVS
    ! Выделение в качестве самостоятельного модуля
    + Функция GetString переехала из mix.cpp
    + GetSearchReplaceString - преобразована из editor.cpp
*/

#include "headers.hpp"
#pragma hdrstop

#include "plugin.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "dialog.hpp"
#include "ctrlobj.hpp"
#include "farexcpt.hpp"

/*
  Функция GetSearchReplaceString выводит диалог поиска или замены, принимает
  от пользователя данные и в случае успешного выполнения диалога возвращает
  TRUE.
  Параметры:
    IsReplaceMode
      TRUE  - если хотим заменять
      FALSE - если хотим искать

    SearchStr
      Указатель на строку поиска.
      Результат отработки диалога заносится в нее же.

    ReplaceStr,
      Указатель на строку замены.
      Результат отработки диалога заносится в нее же.
      Для случая, если IsReplaceMode=FALSE может быть равна NULL

    TextHistoryName
      Имя истории строки поиска.
      Если установлено в NULL, то по умолчанию
      принимается значение "SearchText"
      Если установлено в пустую строку, то история вестись не будет

    ReplaceHistoryName
      Имя истории строки замены.
      Если установлено в NULL, то по умолчанию
      принимается значение "ReplaceText"
      Если установлено в пустую строку, то история вестись не будет

    *Case
      Указатель на переменную, указывающую на значение опции "Case sensitive"
      Если = NULL, то принимается значение 0 (игнорировать регистр)

    *WholeWords
      Указатель на переменную, указывающую на значение опции "Whole words"
      Если = NULL, то принимается значение 0 (в том числе в подстроке)

    *Reverse
      Указатель на переменную, указывающую на значение опции "Reverse search"
      Если = NULL, то принимается значение 0 (прямой поиск)

  Возвращаемое значение:
    TRUE  - пользователь подтвердил свои намериния
    FALSE - пользователь отказался от диалога (Esc)
*/
int WINAPI GetSearchReplaceString(
         int IsReplaceMode,
         unsigned char *SearchStr,
         int LenSearchStr,
         unsigned char *ReplaceStr,
         int LenReplaceStr,
         const char *TextHistoryName,
         const char *ReplaceHistoryName,
         int *Case,
         int *WholeWords,
         int *Reverse)
{
  if(!SearchStr || (IsReplaceMode && !ReplaceStr))
    return FALSE;
  static const char *TextHistoryName0    ="SearchText",
                    *ReplaceHistoryName0 ="ReplaceText";

  if(!TextHistoryName)
    TextHistoryName=TextHistoryName0;
  if(!ReplaceHistoryName)
    ReplaceHistoryName=ReplaceHistoryName0;

  /* $ 03.08.2000 KM
     Добавление checkbox'ов в диалоги для поиска целых слов
  */
  if (IsReplaceMode)
  {
/*
  0         1         2         3         4         5         6         7
  0123456789012345678901234567890123456789012345678901234567890123456789012345
00
01   +----------------------------- Replace ------------------------------+
02   | Search for                                                         |
03   |                                                                   |
04   | Replace with                                                       |
05   |                                                                   |
06   +--------------------------------------------------------------------+
07   | [ ] Case sensitive                                                 |
08   | [ ] Whole words                                                    |
09   | [ ] Reverse search                                                 |
10   +--------------------------------------------------------------------+
11   |                      [ Replace ]  [ Cancel ]                       |
12   +--------------------------------------------------------------------+
13
*/
    static struct DialogData ReplaceDlgData[]={
    /*  0 */DI_DOUBLEBOX,3,1,72,12,0,0,0,0,(char *)MEditReplaceTitle,
    /*  1 */DI_TEXT,5,2,0,0,0,0,0,0,(char *)MEditSearchFor,
    /*  2 */DI_EDIT,5,3,70,3,1,(DWORD)TextHistoryName,DIF_HISTORY|DIF_USELASTHISTORY,0,"",
    /*  3 */DI_TEXT,5,4,0,0,0,0,0,0,(char *)MEditReplaceWith,
    /*  4 */DI_EDIT,5,5,70,3,0,(DWORD)ReplaceHistoryName,DIF_HISTORY/*|DIF_USELASTHISTORY*/,0,"",
    /*  5 */DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    /*  6 */DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)MEditSearchCase,
    /*  7 */DI_CHECKBOX,5,8,0,0,0,0,0,0,(char *)MEditSearchWholeWords,
    /*  8 */DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MEditSearchReverse,
    /*  9 */DI_TEXT,3,10,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    /* 10 */DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP,1,(char *)MEditReplaceReplace,
    /* 11 */DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP,0,(char *)MEditSearchCancel
    };
    /* KM $ */
    MakeDialogItems(ReplaceDlgData,ReplaceDlg);
    if(!*TextHistoryName)
    {
      ReplaceDlg[2].History=0;
      ReplaceDlg[2].Flags&=~DIF_HISTORY;
    }
    if(!*ReplaceHistoryName)
    {
      ReplaceDlg[4].History=0;
      ReplaceDlg[4].Flags&=~DIF_HISTORY;
    }

    strncpy(ReplaceDlg[2].Data,(char *)SearchStr,sizeof(ReplaceDlg[2].Data));
    strncpy(ReplaceDlg[4].Data,(char *)ReplaceStr,sizeof(ReplaceDlg[4].Data));
    ReplaceDlg[6].Selected=Case?*Case:0;
    ReplaceDlg[7].Selected=WholeWords?*WholeWords:0;
    ReplaceDlg[8].Selected=Reverse?*Reverse:0;

    Dialog Dlg(ReplaceDlg,sizeof(ReplaceDlg)/sizeof(ReplaceDlg[0]));
    Dlg.SetPosition(-1,-1,76,14);
    Dlg.Process();
    if (Dlg.GetExitCode()!=10)
      return FALSE;

    strncpy((char *)SearchStr,ReplaceDlg[2].Data,LenSearchStr);
    strncpy((char *)ReplaceStr,ReplaceDlg[4].Data,LenReplaceStr);
    if(Case)       *Case=ReplaceDlg[6].Selected;
    if(WholeWords) *WholeWords=ReplaceDlg[7].Selected;
    if(Reverse)    *Reverse=ReplaceDlg[8].Selected;
  }
  else
  {
/*
  0         1         2         3         4         5         6         7
  0123456789012345678901234567890123456789012345678901234567890123456789012345
00
01   +------------------------------ Search ------------------------------+
02   | Search for                                                         |
03   |                                                                   |
04   +--------------------------------------------------------------------+
05   | [ ] Case sensitive                                                 |
06   | [ ] Whole words                                                    |
07   | [ ] Reverse search                                                 |
08   +--------------------------------------------------------------------+
09   |                       [ Search ]  [ Cancel ]                       |
10   +--------------------------------------------------------------------+
*/
    static struct DialogData SearchDlgData[]={
    /*  0 */DI_DOUBLEBOX,3,1,72,10,0,0,0,0,(char *)MEditSearchTitle,
    /*  1 */DI_TEXT,5,2,0,0,0,0,0,0,(char *)MEditSearchFor,
    /*  2 */DI_EDIT,5,3,70,3,1,(DWORD)TextHistoryName,DIF_HISTORY|DIF_USELASTHISTORY,0,"",
    /*  3 */DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    /*  4 */DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)MEditSearchCase,
    /*  5 */DI_CHECKBOX,5,6,0,0,0,0,0,0,(char *)MEditSearchWholeWords,
    /*  6 */DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)MEditSearchReverse,
    /*  7 */DI_TEXT,3,8,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    /*  8 */DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,1,(char *)MEditSearchSearch,
    /*  9 */DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(char *)MEditSearchCancel
    };
    MakeDialogItems(SearchDlgData,SearchDlg);

    if(!*TextHistoryName)
    {
      SearchDlg[2].History=0;
      SearchDlg[2].Flags&=~DIF_HISTORY;
    }

    strncpy(SearchDlg[2].Data,(char *)SearchStr,sizeof(SearchDlg[2].Data));
    SearchDlg[4].Selected=Case?*Case:0;
    SearchDlg[5].Selected=WholeWords?*WholeWords:0;
    SearchDlg[6].Selected=Reverse?*Reverse:0;

    Dialog Dlg(SearchDlg,sizeof(SearchDlg)/sizeof(SearchDlg[0]));
    Dlg.SetPosition(-1,-1,76,12);
    Dlg.Process();
    if (Dlg.GetExitCode()!=8)
      return FALSE;

    strncpy((char *)SearchStr,SearchDlg[2].Data,LenSearchStr);
    if(ReplaceStr) *ReplaceStr=0;
    if(Case)       *Case=SearchDlg[4].Selected;
    if(WholeWords) *WholeWords=SearchDlg[5].Selected;
    if(Reverse)    *Reverse=SearchDlg[6].Selected;
  }
  return TRUE;
}

/* $ 25.08.2000 SVS
   ! Функция GetString может при соответсвующем флаге (FIB_BUTTONS) отображать
     сепаратор и кнопки <Ok> & <Cancel>
*/
/* $ 01.08.2000 SVS
  ! Функция ввода строки GetString имеет один параметр для всех флагов
*/
/* $ 31.07.2000 SVS
   ! Функция GetString имеет еще один параметр - расширять ли переменные среды!
*/
// Функция для коррекции аля Shift-F4 Shift-Enter без отпускания Shift ;-)
static long WINAPI GetStringDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  if(Msg == DM_KEY)
  {
//    char KeyText[50];
//    KeyToText(Param2,KeyText);
//    _D(SysLog("%s (0x%08X) ShiftPressed=%d",KeyText,Param2,ShiftPressed));
    if(ShiftPressed && Param2 == KEY_ENTER && !CtrlObject->Macro.IsExecuting())
    {
      DWORD Arr[1]={KEY_SHIFTENTER};
      Dialog::SendDlgMessage(hDlg,Msg,Param1,(long)Arr);
      return TRUE;
    }
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

int WINAPI GetString(const char *Title,const char *Prompt,
                     const char *HistoryName,const char *SrcText,
    char *DestText,int DestLength,const char *HelpTopic,DWORD Flags)
{
  int Substract=3; // дополнительная величина :-)
  int ExitCode;
/*
  0         1         2         3         4         5         6         7
  0123456789012345678901234567890123456789012345678901234567890123456789012345
|0                                                                             |
|1   +------------------------------- Title -------------------------------+   |
|2   | Prompt                                                              |   |
|3   | *******************************************************************|   |
|4   +---------------------------------------------------------------------+   |
|5   |                         [ Ok ]   [ Cancel ]                         |   |
|6   +---------------------------------------------------------------------+   |
|7                                                                             |
*/
  static struct DialogData StrDlgData[]=
  {
/*      Type          X1 Y1 X2  Y2 Focus Flags             DefaultButton
                                      Selected               Data
*/
/* 0 */ DI_DOUBLEBOX, 3, 1, 72, 4, 0, 0, 0,                0,"",
/* 1 */ DI_TEXT,      5, 2,  0, 0, 0, 0, DIF_SHOWAMPERSAND,0,"",
/* 2 */ DI_EDIT,      5, 3, 70, 3, 1, 0, 0,                1,"",
/* 3 */ DI_TEXT,      0, 4,  0, 4, 0, 0, DIF_SEPARATOR,    0,"",
/* 4 */ DI_BUTTON,    0, 5,  0, 0, 0, 0, DIF_CENTERGROUP,  0,"",
/* 5 */ DI_BUTTON,    0, 5,  0, 0, 0, 0, DIF_CENTERGROUP,  0,""
  };
  MakeDialogItems(StrDlgData,StrDlg);

  if(Flags&FIB_BUTTONS)
  {
    Substract=0;
    StrDlg[0].Y2+=2;
    StrDlg[2].DefaultButton=0;
    StrDlg[4].DefaultButton=1;
    strcpy(StrDlg[4].Data,FarMSG(MOk));
    strcpy(StrDlg[5].Data,FarMSG(MCancel));
  }

  if(Flags&FIB_EXPANDENV)
  {
    StrDlg[2].Flags|=DIF_EDITEXPAND;
  }

  if (HistoryName!=NULL)
  {
    StrDlg[2].Selected=(int)HistoryName;
    /* $ 09.08.2000 SVS
       флаг для использовании пред значения из истории задается отдельно!!!
    */
    StrDlg[2].Flags|=DIF_HISTORY|(Flags&FIB_NOUSELASTHISTORY?0:DIF_USELASTHISTORY);
    /* SVS $ */
  }

  if (Flags&FIB_PASSWORD)
    StrDlg[2].Type=DI_PSWEDIT;

  if(Title)
    strcpy(StrDlg[0].Data,Title);
  if(Prompt)
    strcpy(StrDlg[1].Data,Prompt);
  if(DestLength > 511 && !(Flags&FIB_PASSWORD))
  {
    StrDlg[2].Flags|=DIF_VAREDIT;
    StrDlg[2].Ptr.PtrTail[0]=StrDlg[2].Ptr.PtrFlags=0;
    if(SrcText)
      memmove(DestText,SrcText,(strlen(SrcText)+1>DestLength?DestLength:strlen(SrcText)+1));
    StrDlg[2].Ptr.PtrData=DestText;
    StrDlg[2].Ptr.PtrLength=DestLength;
  }
  else
  {
    if(SrcText)
      strncpy(StrDlg[2].Data,SrcText,sizeof(StrDlg[2].Data));
    StrDlg[2].Data[sizeof(StrDlg[2].Data)-1]=0;
  }


  TRY{
    Dialog Dlg(StrDlg,sizeof(StrDlg)/sizeof(StrDlg[0])-Substract,GetStringDlgProc);
    Dlg.SetPosition(-1,-1,76,(Flags&FIB_BUTTONS)?8:6);

    if (HelpTopic!=NULL)
      Dlg.SetHelp(HelpTopic);

    Dlg.Process();
    ExitCode=Dlg.GetExitCode();
  }
  __except ( xfilter(EXCEPT_FARDIALOG,
                   GetExceptionInformation(),NULL,1)) // NULL=???
  {
    return FALSE;
  }

  if (DestLength >= 1 && (ExitCode == 2 || ExitCode == 4))
  {
    if(!(Flags&FIB_ENABLEEMPTY) &&
       (!(StrDlg[2].Flags&DIF_VAREDIT) && *StrDlg[2].Data==0 ||
        (StrDlg[2].Flags&DIF_VAREDIT) && *(char *)StrDlg[2].Ptr.PtrData==0
       )
      )
      return(FALSE);
    if(!(StrDlg[2].Flags&DIF_VAREDIT))
    {
      strncpy(DestText,StrDlg[2].Data,DestLength);
      DestText[DestLength-1]=0;
    }
    return(TRUE);
  }
  return(FALSE);
}
/* SVS $*/
/* 01.08.2000 SVS $*/
/* 25.08.2000 SVS $*/


/*
  Стандартный диалог ввода пароля.
  Умеет сам запоминать последнего юзвера и пароль.

  Name      - сюда будет помещен юзвер (max 256 символов!!!)
  Password  - сюда будет помещен пароль (max 256 символов!!!)
  Title     - заголовок диалога (может быть NULL)
  HelpTopic - тема помощи (может быть NULL)
  Flags     - флаги (GNP_*)
*/
int WINAPI GetNameAndPassword(char *Title,char *UserName,char *Password,char *HelpTopic,DWORD Flags)
{
  static char LastName[256],LastPassword[256];
  int ExitCode;
/*
  0         1         2         3         4         5         6         7
  0123456789012345678901234567890123456789012345678901234567890123456789012345
|0                                                                             |
|1   +------------------------------- Title -------------------------------+   |
|2   | User name                                                           |   |
|3   | *******************************************************************|   |
|4   | User password                                                       |   |
|5   | ******************************************************************* |   |
|6   +---------------------------------------------------------------------+   |
|7   |                         [ Ok ]   [ Cancel ]                         |   |
|8   +---------------------------------------------------------------------+   |
|9                                                                             |
*/
  static struct DialogData PassDlgData[]=
  {
/* 0 */ DI_DOUBLEBOX,  3, 1,72, 8,0,0,0,0,"",
/* 1 */ DI_TEXT,       5, 2, 0, 0,0,0,0,0,"",
/* 2 */ DI_EDIT,       5, 3,70, 3,1,0,DIF_USELASTHISTORY|DIF_HISTORY,0,"",
/* 3 */ DI_TEXT,       5, 4, 0, 0,0,0,0,0,"",
/* 4 */ DI_PSWEDIT,    5, 5,70, 3,0,0,0,0,"",
/* 5 */ DI_TEXT,       3, 6, 0, 0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
/* 6 */ DI_BUTTON,     0, 7, 0, 0,0,0,DIF_CENTERGROUP,1,"",
/* 7 */ DI_BUTTON,     0, 7, 0, 0,0,0,DIF_CENTERGROUP,0,""
  };
  MakeDialogItems(PassDlgData,PassDlg);

  strcpy(PassDlg[1].Data,MSG(MNetUserName));
  strcpy(PassDlg[3].Data,MSG(MNetUserPassword));
  strcpy(PassDlg[6].Data,MSG(MOk));
  strcpy(PassDlg[7].Data,MSG(MCancel));
  if (Title!=NULL)
    strncpy(PassDlg[0].Data,Title,sizeof(PassDlg[0].Data));
  strncpy(PassDlg[2].Data,(Flags&GNP_USELAST)?LastName:UserName,sizeof(LastName));
  strncpy(PassDlg[4].Data,(Flags&GNP_USELAST)?LastPassword:Password,sizeof(LastPassword));

  {
    Dialog Dlg(PassDlg,sizeof(PassDlg)/sizeof(PassDlg[0]));
    Dlg.SetPosition(-1,-1,76,10);

    if (HelpTopic!=NULL)
      Dlg.SetHelp(HelpTopic);

    Dlg.Process();
    ExitCode=Dlg.GetExitCode();
  }

  if (ExitCode!=6)
    return(FALSE);

  // запоминаем всегда.
  strcpy(LastName,strncpy(UserName,PassDlg[2].Data,sizeof(LastName)));
  strcpy(LastPassword,strncpy(Password,PassDlg[4].Data,sizeof(LastPassword)));

  // Convert Name and Password to Ansi
  if(!(Flags&GNP_NOOEMTOCHAR))
  {
    OemToChar(UserName,UserName);
    OemToChar(Password,Password);
  }
  return(TRUE);
}
