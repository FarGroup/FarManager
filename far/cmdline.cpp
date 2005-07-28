/*
cmdline.cpp

Командная строка

*/

/* Revision: 1.81 06.05.2005 $ */

/*
Modify:
  06.05.2005 SVS
    ! ???::GetCurDir() теперь возвращает размер пути, при этом
      его параметр может быть равен NULL. Сделано для того, чтобы
      как то получить этот размер.
  24.04.2005 AY
    ! GCC
  28.02.2005 SVS
    ! В строках ввода (диалоги) и ком.строка - будет конвертироваться
      ближайшее слово или выделение.
    + XLAT_CONVERTALLCMDLINE
  23.12.2004 WARP
    ! Нафиг отрубаем показ дерева, если его создание прервали по Esc (нечего пустое дерево показывать).
      Ставим флаг IsPanel в FALSE. Мы ведь панель не создаем.
  08.12.2004 SVS
    + CmdLine.ItemCount, CmdLine.CurPos
  11.11.2004 SVS
    + Обработка MCODE_V_ITEMCOUNT и MCODE_V_CURPOS
  04.11.2004 SVS
    - Alt+F11 F3/F4: по ф3 ничего не происходит, по ф4 - просмотр
  25.10.2004 SVS
    + Реакция на Ctrl-Shift-Enter в истории папок - перейти в папку на пассивной панели
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  05.08.2004 SVS
    ! MCODE_C_CMDLINE_BOF, MCODE_C_CMDLINE_EOF, MCODE_C_CMDLINE_EMPTY, MCODE_C_CMDLINE_SELECTED
  02.08.2004 SVS
    + обработка MCODE_C_CMDLINE_BOF и MCODE_C_CMDLINE_EOF
  07.07.2004 SVS
    ! Macro II
  06.05.2004 SVS
    + ProcessOSAliases()
  09.03.2004 SVS
    + CorrectRealScreenCoord() - корректировка размеров буфера
  09.01.2004 SVS
    + Opt.ExcludeCmdHistory
  25.09.2003 SVS
    ! KEY_MACROXLAT переименован в KEY_MACRO_XLAT
  09.09.2003 SVS
    - Автодополнение по ^End в пустой строке сразу после старта не работает.
  03.09.2003 SVS
    + Продвинутый вариант промптера, как в XP. Добавил, чтобы не потерялось (трудные времена настают :-(
  21.08.2003 SVS
    ! Сделаем LastCmdStr динамической переменной.
      Отсюда все остальные изменения
    + CommandLine::SetLastCmdStr() - "выставляет" эту самую LastCmdStr.
  28.05.2003 SVS
    ! Opt.EdOpt.PersistentBlocks -> Opt.Dialogs.EditBlock
  22.04.2003 SVS
    ! strcpy -> strNcpy
  20.09.2002 SVS
    - BugZ#619 - ftp: /pub в истории папок
  06.08.2002 SVS
    - после открытия дерева по Alt-F10 и после отмены диалога не по esc,
      а нажатием мыши вне его, не восстанавливается текст линейки клавиш
      модифицированных нажатием Alt, Shift, ...
  30.05.2002 IS
    ! взял управление на себя :-) - тут IsLocalPath применять можно
      безболезненно
  29.05.2002 SVS
    ! "Не справился с управлением" - откат IsLocalPath() до лучших времен.
  28.05.2002 SVS
    ! применим функцию  IsLocalPath()
  24.05.2002 SVS
    + Дублирование Numpad-клавиш
  05.04.2002 SVS
    ! Продолжение для BugZ#239 (Folder shortcuts для несуществующих папок)
      Аналогичное поведение сделаем и для Alt-F12, ибо это правильно!
  25.03.2002 VVM
    + При погашенных панелях колесом крутим историю
  18.03.2002 SVS
    ! Уточнения, в связи с введением Opt.Dialogs
    ! Немного оптимизации и комментариния кода
  06.03.2002 SVS
    ! У функций Истории появились доп.параметры
  28.02.2002 SVS
    ! SetString() имеет доп. параметр - надобность в прорисовке новой строки
  14.12.2001 IS
    + GetStringAddr();
  06.12.2001 SVS
    ! PrepareDiskPath() - имеет доп.параметр - максимальный размер буфера
  26.11.2001 SVS
    ! Заюзаем PrepareDiskPath() для преобразования пути.
  15.11.2001 IS
    - очепятка при предыдущем изменении :-(
  13.11.2001 IS
    - После xlat неправильно работал ctrl-end.
  02.11.2001 SVS
    + GetSelection()
  10.10.2001 SVS
    ! Часть кода, ответственная за "пусковик" внешних прилад вынесена
      в отдельный модуль execute.cpp
  05.10.2001 SVS
    ! Немного оптимизации (с сокращение повторяющегося кода)
  27.09.2001 IS
    - Левый размер при использовании strncpy
  26.09.2001 VVM
    ! Перерисовать панели, если были изменения. В догонку к предыдущему патчу.
  23.09.2001 VVM
    ! CmdExecute: RedrawDesktop должен уничтожиться _до_ вызова UpdateIfChanged.
      Иначе InfoPanel перерисовывается при изменении каталога и портит background
  09.09.2001 IS
    + SetPersistentBlocks - установить/сбросить постоянные блоки
  08.09.2001 VVM
    + Использовать Opt.DialogsEditBlock
  30.08.2001 VVM
    ! В командной строке блоки всегда непостояные.
  23.08.2001 OT
    - исправление far -e file -> AltF9
  13.08.2001 SKV
    + GetSelString, Select
  07.08.2001 SVS
    + Добавим обработку команды OS - CLS
  10.07.2001 SVS
    + Обработка KEY_MACROXLAT
  25.06.2001 SVS
    - неверно работало в "If exist" преобразование toFullName, не учитывался
      факт того, что имя уже может иметь полный путь.
  22.06.2001 SKV
    - Update панелей после исполнения команды.
  18.06.2001 SVS
    - Во время проверки "If exist" не учитывался текущий каталог.
  17.06.2001 IS
    ! Вместо ExpandEnvironmentStrings применяем ExpandEnvironmentStr, т.к. она
      корректно работает с символами, коды которых выше 0x7F.
    + Перекодируем строки перед SetEnvironmentVariable из OEM в ANSI
  07.06.2001 SVS
    + Добавлена обработка операторов "REM" и "::"
  04.06.2001 OT
    - Исправление отрисовки консоли при наличии Qinfo и или других "настандартных" панелей
  26.05.2001 OT
    - Выпрямление логики вызовов в NFZ
  17.05.2001 OT
    - Отрисовка при изменении размеров консоли - ResizeConsole().
  15.05.2001 OT
    ! NWZ -> NFZ
  12.05.2001 DJ
    ! перерисовка командной строки после обработки команды делается только
      если панели остались верхним фреймом
  11.05.2001 OT
    ! Новые методы для отрисовки Background
  10.05.2001 DJ
    * ShowViewEditHistory()
  07.05.2001 SVS
    ! SysLog(); -> _D(SysLog());
  06.05.2001 DJ
    ! перетрях #include
  06.05.2001 ОТ
    ! Переименование Window в Frame :)
  05.05.2001 DJ
    + перетрях NWZ
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  25.04.2001 DJ
    * обработка @ в IF EXIST
    * обработка кавычек внутри имени файла в IF EXIST
  11.04.2001 SVS
    ! Для Alt-F11 и Alt-F12 - теперь будут свои конкретные темы помощи, а не
      абстрактное описание команд командной строки (не нужное для этих
      историй)
  02.04.2001 VVM
    + Обработка Opt.FlagPosixSemantics
  12.03.2001 SVS
    + Alt-Shift-Left, Alt-Shift-Right, Alt-Shift-Home и Alt-Shift-End выделяют
      блок в командной строке независимо от состояния панелей.
  21.02.2001 IS
    ! Opt.EditorPersistentBlocks -> Opt.EdOpt.PersistentBlocks
  19.02.2001 IS
    - баг: не сбрасывалось выделение в командной строке по enter и shift-enter
  14.01.2001 SVS
    + В ProcessOSCommands добавлена обработка
       "IF [NOT] EXIST filename command"
       "IF [NOT] DEFINED variable command"
  18.12.2000 SVS
    - Написано же "Ctrl-D - Символ вправо"!
    + Сбрасываем выделение при редактировании на некоторых клавишах
  13.12.2000 SVS
    ! Для CmdLine - если нет выделения, преобразуем всю строку (XLat)
  04.11.2000 SVS
    + Проверка на альтернативную клавишу при XLat-перекодировке
  24.09.2000 SVS
    + поведение ESC.
    + вызов функции Xlat
  19.09.2000 SVS
    - При выборе из History (по Alt-F8) плагин не получал управление!
  13.09.2000 tran 1.02
    + COL_COMMANDLINEPREFIX
  02.08.2000 tran 1.01
    - мелкий фикс - при выходе по CtrlF10, если файл был открыт на просмотр
      из Alt-F11, был виден keybar в панелях
      как всегда добавил CtrlObject->Cp()->Redraw()
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "cmdline.hpp"
#include "global.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "lang.hpp"
#include "fn.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "history.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "foldtree.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "rdrwdsk.hpp"
#include "savescr.hpp"
#include "scrbuf.hpp"

CommandLine::CommandLine()
{
  *CurDir=0;
  CmdStr.SetEditBeyondEnd(FALSE);
  SetPersistentBlocks(Opt.Dialogs.EditBlock);
  LastCmdStr=NULL;
  LastCmdLength=0;
  LastCmdPartLength=-1;
  BackgroundScreen=NULL;
}

CommandLine::~CommandLine()
{
  if(LastCmdStr)
    xf_free(LastCmdStr);

}

/* $ 09.09.2001 IS установить/сбросить постоянные блоки */
void CommandLine::SetPersistentBlocks(int Mode)
{
  CmdStr.SetPersistentBlocks(Mode);
}
/* IS $ */

void CommandLine::DisplayObject()
{
  _OT(SysLog("[%p] CommandLine::DisplayObject()",this));
  char TruncDir[1024];
  GetPrompt(TruncDir);
  TruncPathStr(TruncDir,(X2-X1)/2);
  GotoXY(X1,Y1);
  SetColor(COL_COMMANDLINEPREFIX);
  Text(TruncDir);
  CmdStr.SetObjectColor(COL_COMMANDLINE,COL_COMMANDLINESELECTED);
  CmdStr.SetLeftPos(0);
  CmdStr.SetPosition(X1+strlen(TruncDir),Y1,X2,Y2);
  CmdStr.Show();
}


void CommandLine::SetCurPos(int Pos)
{
  CmdStr.SetLeftPos(0);
  CmdStr.SetCurPos(Pos);
  CmdStr.Redraw();
}

BOOL CommandLine::SetLastCmdStr(const char *Ptr,int LenPtr)
{
  if(LenPtr+1 > LastCmdLength)
    LastCmdStr=(char *)xf_realloc(LastCmdStr, LenPtr+1);
  if(LastCmdStr)
  {
    LastCmdLength=LenPtr;
    strcpy(LastCmdStr,Ptr);
    return TRUE;
  }
  else
    LastCmdLength=0;
  return FALSE;
}

int CommandLine::ProcessKey(int Key)
{
  char Str[2048], *PStr;

  if(Key >= MCODE_C_CMDLINE_BOF && Key <= MCODE_C_CMDLINE_SELECTED)
    return CmdStr.ProcessKey(Key-MCODE_C_CMDLINE_BOF+MCODE_C_BOF);
  if(Key == MCODE_V_ITEMCOUNT || Key == MCODE_V_CURPOS)
    return CmdStr.ProcessKey(Key);
  if(Key == MCODE_V_CMDLINE_ITEMCOUNT || Key == MCODE_V_CMDLINE_CURPOS)
    return CmdStr.ProcessKey(Key-MCODE_V_CMDLINE_ITEMCOUNT+MCODE_V_ITEMCOUNT);

  if ((Key==KEY_CTRLEND || Key==KEY_CTRLNUMPAD1) && CmdStr.GetCurPos()==CmdStr.GetLength())
  {
    if (LastCmdPartLength==-1)
      SetLastCmdStr(CmdStr.GetStringAddr(),CmdStr.GetLength());

    if(!LastCmdStr)
      return TRUE;

    xstrncpy(Str,LastCmdStr,sizeof(Str)-1);
    int CurCmdPartLength=strlen(Str);
    CtrlObject->CmdHistory->GetSimilar(Str,LastCmdPartLength);
    if (LastCmdPartLength==-1)
    {
      if(SetLastCmdStr(CmdStr.GetStringAddr(),CmdStr.GetLength()))
        LastCmdPartLength=CurCmdPartLength;
    }
    CmdStr.SetString(Str);
    Show();
    return(TRUE);
  }

  if(Key == KEY_UP || Key == KEY_NUMPAD8)
  {
    if (CtrlObject->Cp()->LeftPanel->IsVisible() || CtrlObject->Cp()->RightPanel->IsVisible())
      return(FALSE);
    Key=KEY_CTRLE;
  }
  else if(Key == KEY_DOWN || Key == KEY_NUMPAD2)
  {
    if (CtrlObject->Cp()->LeftPanel->IsVisible() || CtrlObject->Cp()->RightPanel->IsVisible())
      return(FALSE);
    Key=KEY_CTRLX;
  }
  /* $ 25.03.2002 VVM
    + При погашенных панелях колесом крутим историю */
  if (!CtrlObject->Cp()->LeftPanel->IsVisible() && !CtrlObject->Cp()->RightPanel->IsVisible())
  {
    if (Key == KEY_MSWHEEL_UP)
      Key = KEY_CTRLE;
    else if (Key == KEY_MSWHEEL_DOWN)
      Key = KEY_CTRLX;
  }
  /* VVM $ */

  switch(Key)
  {
    case KEY_TAB: // autocomplete
    {
      //xstrncpy(Str,,sizeof(Str)-1);
      //CmdStr.SetString(Str);
      //Show();
      return(TRUE);
    }

    case KEY_CTRLE:
    case KEY_CTRLX:
      if(Key == KEY_CTRLE)
        CtrlObject->CmdHistory->GetPrev(Str,sizeof(Str));
      else
        CtrlObject->CmdHistory->GetNext(Str,sizeof(Str));
    case KEY_ESC:
      if(Key == KEY_ESC)
      {
        /* $ 24.09.2000 SVS
           Если задано поведение по "Несохранению при Esc", то позицию в
           хистори не меняем и ставим в первое положение.
        */
        if(Opt.CmdHistoryRule)
          CtrlObject->CmdHistory->SetFirst();
        PStr="";
      }
      else
        PStr=Str;
      SetString(PStr);
      return(TRUE);
    case KEY_F2:
      ProcessUserMenu(0);
      return(TRUE);
    case KEY_ALTF8:
      {
        int Type;
        /* $ 19.09.2000 SVS
           - При выборе из History (по Alt-F8) плагин не получал управление!
        */
        int SelectType=CtrlObject->CmdHistory->Select(MSG(MHistoryTitle),"History",Str,sizeof(Str),Type);
        if(SelectType > 0 && SelectType <= 3)
        {
          SetString(Str);
          if(SelectType < 3)
            ProcessKey(SelectType==1?KEY_ENTER:KEY_SHIFTENTER);
        }
        /* SVS $ */
      }
      return(TRUE);
    case KEY_SHIFTF9:
      SaveConfig(1);
      return(TRUE);
    case KEY_F10:
      FrameManager->ExitMainLoop(TRUE);
      return(TRUE);
    case KEY_ALTF10:
      {
        {
          FolderTree Tree(Str,MODALTREE_ACTIVE,4,2,ScrX-4,ScrY-4, FALSE);
          FrameManager->GetCurrentFrame()->RedrawKeyBar(); // Затычка ;-(
        }
        if (*Str)
        {
          Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
          ActivePanel->SetCurDir(Str,TRUE);
          ActivePanel->Show();
          if (ActivePanel->GetType()==TREE_PANEL)
            ActivePanel->ProcessKey(KEY_ENTER);
        }
      }
      return(TRUE);
    case KEY_F11:
      CtrlObject->Plugins.CommandsMenu(FALSE,FALSE,0);
      return(TRUE);
    case KEY_ALTF11:
      /* $ 10.05.2001 DJ
         показ view/edit history вынесен в отдельную процедуру
      */
      ShowViewEditHistory();
      /* DJ $ */
      CtrlObject->Cp()->Redraw();
      return(TRUE);
    case KEY_ALTF12:
      {
        int Type;
        int SelectType=CtrlObject->FolderHistory->Select(MSG(MFolderHistoryTitle),"HistoryFolders",Str,sizeof(Str),Type);
        /*
           SelectType = 0 - Esc
                        1 - Enter
                        2 - Shift-Enter
                        3 - Ctrl-Enter
                        6 - Ctrl-Shift-Enter - на пассивную панель со сменой позиции
        */
        if (SelectType == 1 || SelectType == 2 || SelectType == 6)
        {
          if (SelectType==2)
            CtrlObject->FolderHistory->SetAddMode(FALSE,2,TRUE);
          // пусть плагин сам прыгает... ;-)
          Panel *Panel=CtrlObject->Cp()->ActivePanel;
          if(SelectType == 6)
            Panel=CtrlObject->Cp()->GetAnotherPanel(Panel);

          if(Panel->GetMode() == PLUGIN_PANEL ||
             CheckShortcutFolder(Str,sizeof(Str)-1,FALSE))
          {
            Panel->SetCurDir(Str,Type==0 ? TRUE:FALSE);
            Panel->Redraw();
            CtrlObject->FolderHistory->SetAddMode(TRUE,2,TRUE);
          }
        }
        else
          if (SelectType==3)
            SetString(Str);
      }
      return(TRUE);
    case KEY_ENTER:
    case KEY_SHIFTENTER:
      {
        Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
        /* $ 19.02.2001 IS
             - выделение нам уже не нужно
        */
        CmdStr.Select(-1,0);
        CmdStr.Show();
        /* IS $ */
        CmdStr.GetString(Str,sizeof(Str));
        if (*Str==0)
          break;
        ActivePanel->SetCurPath();
        if(!(Opt.ExcludeCmdHistory&EXCLUDECMDHISTORY_NOTCMDLINE))
          CtrlObject->CmdHistory->AddToHistory(Str);
        ProcessOSAliases(Str,sizeof(Str));
        if (!ActivePanel->ProcessPluginEvent(FE_COMMAND,(void *)Str))
          CmdExecute(Str,FALSE,Key==KEY_SHIFTENTER,FALSE);
      }
      return(TRUE);

    /* дополнительные клавиши для выделения в ком строке.
       ВНИМАНИЕ!
       Для сокращения кода этот кусок должен стоять перед "default"
    */
    case KEY_ALTSHIFTLEFT:  case KEY_ALTSHIFTNUMPAD4:
    case KEY_ALTSHIFTRIGHT: case KEY_ALTSHIFTNUMPAD6:
    case KEY_ALTSHIFTEND:   case KEY_ALTSHIFTNUMPAD1:
    case KEY_ALTSHIFTHOME:  case KEY_ALTSHIFTNUMPAD7:
      Key&=~KEY_ALT;

    default:
      /* $ 24.09.2000 SVS
         Если попалась клавиша вызова функции Xlat, то
         подставим клавишу для редактора, если она != 0
      */
      /* $ 04.11.2000 SVS
         Проверка на альтернативную клавишу
      */
      if((Opt.XLat.XLatCmdLineKey && Key == Opt.XLat.XLatCmdLineKey) ||
         (Opt.XLat.XLatAltCmdLineKey && Key == Opt.XLat.XLatAltCmdLineKey) ||
         Key == MCODE_OP_XLAT)
      {
        /* 13.12.2000 SVS
           ! Для CmdLine - если нет выделения, преобразуем всю строку (XLat)
        */
        CmdStr.Xlat(Opt.XLat.Flags&XLAT_CONVERTALLCMDLINE?TRUE:FALSE);
        /* SVS $ */
        /* $ 13.11.2001 IS иначе неправильно работает ctrl-end */
        if(SetLastCmdStr(CmdStr.GetStringAddr(),CmdStr.GetLength()))
          LastCmdPartLength=strlen(LastCmdStr);
        /* IS $ */
        return(TRUE);
      }
      /* SVS $ */
      /* SVS $ */

      /* $ 18.12.2000 SVS
         Сбрасываем выделение на некоторых клавишах
      */
      if (!Opt.Dialogs.EditBlock)
      {
        static int UnmarkKeys[]={
               KEY_LEFT,       KEY_NUMPAD4,
               KEY_CTRLS,
               KEY_RIGHT,      KEY_NUMPAD6,
               KEY_CTRLD,
               KEY_CTRLLEFT,   KEY_CTRLNUMPAD4,
               KEY_CTRLRIGHT,  KEY_CTRLNUMPAD6,
               KEY_CTRLHOME,   KEY_CTRLNUMPAD7,
               KEY_CTRLEND,    KEY_CTRLNUMPAD1,
               KEY_HOME,       KEY_NUMPAD7,
               KEY_END,        KEY_NUMPAD1
        };

        for (int I=0;I<sizeof(UnmarkKeys)/sizeof(UnmarkKeys[0]);I++)
          if (Key==UnmarkKeys[I])
          {
            CmdStr.Select(-1,0);
            break;
          }
      }
      /* SVS $ */

      /* $ 18.12.2000 SVS
         Написано же "Ctrl-D - Символ вправо"
      */
      if(Key == KEY_CTRLD)
        Key=KEY_RIGHT;
      /* SVS $ */

      if (!CmdStr.ProcessKey(Key))
        break;

      LastCmdPartLength=-1;
      return(TRUE);
  }
  return(FALSE);
}


void CommandLine::SetCurDir(const char *CurDir)
{
  PrepareDiskPath(xstrncpy(CommandLine::CurDir,CurDir,sizeof(CommandLine::CurDir)-1),sizeof(CommandLine::CurDir)-1);
}


int CommandLine::GetCurDir(char *CurDir)
{
  if(CurDir)
    strcpy(CurDir,CommandLine::CurDir); // TODO: ОПАСНО!!!
  return strlen(CommandLine::CurDir);
}

void CommandLine::GetString(char *Str,int MaxSize)
{
  CmdStr.GetString(Str,MaxSize);
}

/* $ 14.12.2001 IS получить адрес данных командной строки */
const char *CommandLine::GetStringAddr()
{
  return CmdStr.GetStringAddr();
}
/* IS $ */

void CommandLine::SetString(const char *Str,BOOL Redraw)
{
  LastCmdPartLength=-1;
  CmdStr.SetString(Str);
  CmdStr.SetLeftPos(0);
  if(Redraw)
    CmdStr.Show();
}


void CommandLine::ExecString(char *Str,int AlwaysWaitFinish,int SeparateWindow,
                             int DirectRun)
{
  SetString(Str);
  CmdExecute(Str,AlwaysWaitFinish,SeparateWindow,DirectRun);
}


void CommandLine::InsertString(const char *Str)
{
  LastCmdPartLength=-1;
  CmdStr.InsertString(Str);
  CmdStr.Show();
}


int CommandLine::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  return(CmdStr.ProcessMouse(MouseEvent));
}


void CommandLine::GetPrompt(char *DestStr)
{
#if 1
  char FormatStr[512],ExpandedFormatStr[512];
  xstrncpy(FormatStr,Opt.UsePromptFormat ? Opt.PromptFormat:"$p$g",sizeof(FormatStr)-1);
  char *Format=FormatStr;
  if (Opt.UsePromptFormat)
  {
    ExpandEnvironmentStr(FormatStr,ExpandedFormatStr,sizeof(ExpandedFormatStr));
    Format=ExpandedFormatStr;
  }
  while (*Format)
  {
    if (*Format=='$')
    {
      Format++;
      switch(*Format)
      {
        case '$':
          *(DestStr++)='$';
          break;
        case 'p':
          strcpy(DestStr,CurDir);
          DestStr+=strlen(CurDir);
          break;
        case 'n':
          if (IsLocalPath(CurDir) && CurDir[2]=='\\')
            *(DestStr++)=LocalUpper(*CurDir);
          else
            *(DestStr++)='?';
          break;
        case 'g':
          *(DestStr++)='>';
          break;
      }
      Format++;
    }
    else
      *(DestStr++)=*(Format++);
  }
  *DestStr=0;
#else
  // продвинутый вариант промптера, как в XP
  if (Opt.UsePromptFormat)
  {
    char FormatStr[512],ExpandedFormatStr[512];
    strcpy(FormatStr,Opt.PromptFormat);
    char *Format=FormatStr;
    ExpandEnvironmentStr(FormatStr,ExpandedFormatStr,sizeof(ExpandedFormatStr));
    Format=ExpandedFormatStr;
    char ChrFmt[][2]={
      {'A','&'},   // $A - & (Ampersand)
      {'B','|'},   // $B - | (pipe)
      {'C','('},   // $C - ( (Left parenthesis)
      {'F',')'},   // $F - ) (Right parenthesis)
      {'G','>'},   // $G - > (greater-than sign)
      {'L','<'},   // $L - < (less-than sign)
      {'Q','='},   // $Q - = (equal sign)
      {'S',' '},   // $S - (space)
      {'$','$'},   // $$ - $ (dollar sign)
    };
    while (*Format)
    {
      if (*Format=='$')
      {
        char Chr=toupper(*++Format);
        int I;
        for(I=0; I < sizeof(ChrFmt)/sizeof(ChrFmt[0]); ++I)
        {
          if(ChrFmt[I][0] == Chr)
          {
            *(DestStr++)=ChrFmt[I][1];
            break;
          }
        }

        if(I == sizeof(ChrFmt)/sizeof(ChrFmt[0]))
        {
          switch(Chr)
          {
            /* эти не раелизованы
            $E - Escape code (ASCII code 27)
            $V - Windows XP version number
            $_ - Carriage return and linefeed
            */
            case 'H': // $H - Backspace (erases previous character)
              DestStr--;
              break;
            case 'D': // $D - Current date
            case 'T': // $T - Current time
            {
              char DateTime[64];
              MkStrFTime(DateTime,sizeof(DateTime)-1,(Chr=='D'?"%D":"%T"));
              strcpy(DestStr,DateTime);
              DestStr+=strlen(DateTime);
              break;
            }
            case 'N': // $N - Current drive
              if (IsLocalPath(CurDir) && CurDir[2]=='\\')
                *(DestStr++)=LocalUpper(*CurDir);
              else
                *(DestStr++)='?';
              break;
            case 'P': // $P - Current drive and path
              strcpy(DestStr,CurDir);
              DestStr+=strlen(CurDir);
              break;
          }
        }
        Format++;
      }
      else
        *(DestStr++)=*(Format++);
    }
    *DestStr=0;
  }
  else // default prompt = "$p$g"
  {
    strcpy(DestStr,CurDir);
    strcat(DestStr,">");
  }
#endif
}



/* $ 10.05.2001 DJ
   показ history по Alt-F11 вынесен в отдельную функцию
*/

void CommandLine::ShowViewEditHistory()
{
  char Str[1024], ItemTitle[256];
  int Type;

  int SelectType=CtrlObject->ViewHistory->Select(MSG(MViewHistoryTitle),"HistoryViews",Str,sizeof(Str),Type,ItemTitle);
  /*
     SelectType = 0 - Esc
                  1 - Enter
                  2 - Shift-Enter
                  3 - Ctrl-Enter
  */

  if (SelectType == 1 || SelectType == 2)
  {
    if (SelectType!=2)
      CtrlObject->ViewHistory->AddToHistory(Str,ItemTitle,Type);
    CtrlObject->ViewHistory->SetAddMode(FALSE,Opt.FlagPosixSemantics?1:2,TRUE);

    switch(Type)
    {
      case 0: // вьювер
      {
        new FileViewer(Str,TRUE);
        break;
      }

      case 1: // обычное открытие в редакторе
      case 4: // открытие с локом
      {
        FileEditor *FEdit=new FileEditor(Str,FALSE,TRUE);
        if(Type == 4)
           FEdit->SetLockEditor(TRUE);
        break;
      }

      // 2 и 3 - заполняется в ProcessExternal
      case 2:
      case 3:
      {
        if (*Str!='@')
          ExecString(Str,Type-2);
        else
        {
          SaveScreen SaveScr;
          CtrlObject->Cp()->LeftPanel->CloseFile();
          CtrlObject->Cp()->RightPanel->CloseFile();
          Execute(Str+1,Type-2);
        }
        break;
      }
    }
    CtrlObject->ViewHistory->SetAddMode(TRUE,Opt.FlagPosixSemantics?1:2,TRUE);
  }
  else
    if (SelectType==3) // скинуть из истории в ком.строку?
      SetString(Str);
}

/* DJ $ */
int CommandLine::GetCurPos()
{
  return(CmdStr.GetCurPos());
}


void CommandLine::SaveBackground(int X1,int Y1,int X2,int Y2)
{
  if (BackgroundScreen) {
    delete BackgroundScreen;
  }
  BackgroundScreen=new SaveScreen(X1,Y1,X2,Y2);
}

void CommandLine::SaveBackground()
{
  if (BackgroundScreen) {
//    BackgroundScreen->Discard();
    BackgroundScreen->SaveArea();
  }
}
void CommandLine::ShowBackground()
{
  if (BackgroundScreen){
    BackgroundScreen->RestoreArea();
  }
}

void CommandLine::CorrectRealScreenCoord()
{
  if (BackgroundScreen) {
    BackgroundScreen->CorrectRealScreenCoord();
  }
}

void CommandLine::ResizeConsole()
{
  BackgroundScreen->Resize(ScrX+1,ScrY+1,2);
//  this->DisplayObject();
}

/*$ 13.08.2001 SKV */
void CommandLine::GetSelString(char* Buffer,int MaxLength)
{
  CmdStr.GetSelString(Buffer,MaxLength);
}

void CommandLine::Select(int Start,int End)
{
  CmdStr.Select(Start,End);
}
/* SKV$*/

void CommandLine::GetSelection(int &Start,int &End)
{
  CmdStr.GetSelection(Start,End);
}
