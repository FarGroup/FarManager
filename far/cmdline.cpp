/*
cmdline.cpp

Командная строка

*/

/* Revision: 1.44 02.11.2001 $ */

/*
Modify:
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
#include "fn.hpp"
#include "keys.hpp"
#include "lang.hpp"
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
  SetPersistentBlocks(Opt.DialogsEditBlock);
  LastCmdPartLength=-1;
  *LastCmdStr=0;
  BackgroundScreen=NULL;
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
  char TruncDir[NM];
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


int CommandLine::ProcessKey(int Key)
{
  char Str[2048], *PStr;

  if (Key==KEY_CTRLEND && CmdStr.GetCurPos()==CmdStr.GetLength())
  {
    if (LastCmdPartLength==-1)
      strncpy(LastCmdStr,CmdStr.GetStringAddr(),sizeof(LastCmdStr)-1);
    strcpy(Str,LastCmdStr);
    int CurCmdPartLength=strlen(Str);
    CtrlObject->CmdHistory->GetSimilar(Str,LastCmdPartLength);
    if (LastCmdPartLength==-1)
    {
      LastCmdPartLength=CurCmdPartLength;
      strncpy(LastCmdStr,CmdStr.GetStringAddr(),sizeof(LastCmdStr)-1);
    }
    CmdStr.SetString(Str);
    Show();
    return(TRUE);
  }

  if(Key == KEY_UP)
  {
    if (CtrlObject->Cp()->LeftPanel->IsVisible() || CtrlObject->Cp()->RightPanel->IsVisible())
      return(FALSE);
    Key=KEY_CTRLE;
  }
  else if(Key == KEY_DOWN)
  {
    if (CtrlObject->Cp()->LeftPanel->IsVisible() || CtrlObject->Cp()->RightPanel->IsVisible())
      return(FALSE);
    Key=KEY_CTRLX;
  }

  switch(Key)
  {
    case KEY_CTRLE:
    case KEY_CTRLX:
      if(Key == KEY_CTRLE)
        CtrlObject->CmdHistory->GetPrev(Str);
      else
        CtrlObject->CmdHistory->GetNext(Str);
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
        switch(CtrlObject->CmdHistory->Select(MSG(MHistoryTitle),"History",Str,Type))
        {
          case 1:
            SetString(Str);
            ProcessKey(KEY_ENTER);
            //ExecString(Str,FALSE,FALSE);
            //CtrlObject->CmdHistory->AddToHistory(Str);
            break;
          case 2:
            SetString(Str);
            ProcessKey(KEY_SHIFTENTER);
            //ExecString(Str,FALSE,TRUE);
            //CtrlObject->CmdHistory->AddToHistory(Str);
            break;
          case 3:
            SetString(Str);
            break;
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
          FolderTree Tree(Str,MODALTREE_ACTIVE,4,2,ScrX-4,ScrY-4);
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
        int Type,SelectType;
        if ((SelectType=CtrlObject->FolderHistory->Select(MSG(MFolderHistoryTitle),"HistoryFolders",Str,Type))==1 || SelectType==2)
        {
          if (SelectType==2)
            CtrlObject->FolderHistory->SetAddMode(FALSE,2,TRUE);
          CtrlObject->Cp()->ActivePanel->SetCurDir(Str,Type==0 ? TRUE:FALSE);
          CtrlObject->Cp()->ActivePanel->Redraw();
          CtrlObject->FolderHistory->SetAddMode(TRUE,2,TRUE);
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
        CtrlObject->CmdHistory->AddToHistory(Str);
        if (!ActivePanel->ProcessPluginEvent(FE_COMMAND,(void *)Str))
          CmdExecute(Str,FALSE,Key==KEY_SHIFTENTER,FALSE);
      }
      return(TRUE);

    /* дополнительные клавиши для выделения в ком строке.
       ВНИМАНИЕ!
       Для сокращения кода этот кусок должен стоять перед "default"
    */
    case KEY_ALTSHIFTLEFT:
    case KEY_ALTSHIFTRIGHT:
    case KEY_ALTSHIFTEND:
    case KEY_ALTSHIFTHOME:
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
         Key == KEY_MACROXLAT)
      {
        /* 13.12.2000 SVS
           ! Для CmdLine - если нет выделения, преобразуем всю строку (XLat)
        */
        CmdStr.Xlat(TRUE);
        /* SVS $ */
        return(TRUE);
      }
      /* SVS $ */
      /* SVS $ */

      /* $ 18.12.2000 SVS
         Сбрасываем выделение на некоторых клавишах
      */
      if (!Opt.EdOpt.PersistentBlocks)
      {
        static int UnmarkKeys[]={KEY_LEFT,KEY_CTRLS,KEY_RIGHT,KEY_CTRLD,
                   KEY_CTRLLEFT,KEY_CTRLRIGHT,KEY_CTRLHOME,KEY_CTRLEND,
                   KEY_HOME,KEY_END
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


void CommandLine::SetCurDir(char *CurDir)
{
  strcpy(CommandLine::CurDir,CurDir);
}


void CommandLine::GetCurDir(char *CurDir)
{
  strcpy(CurDir,CommandLine::CurDir);
}

void CommandLine::GetString(char *Str,int MaxSize)
{
  CmdStr.GetString(Str,MaxSize);
}


void CommandLine::SetString(char *Str)
{
  LastCmdPartLength=-1;
  CmdStr.SetString(Str);
  CmdStr.SetLeftPos(0);
  CmdStr.Show();
}


void CommandLine::ExecString(char *Str,int AlwaysWaitFinish,int SeparateWindow,
                             int DirectRun)
{
  SetString(Str);
  CmdExecute(Str,AlwaysWaitFinish,SeparateWindow,DirectRun);
}


void CommandLine::InsertString(char *Str)
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
  char FormatStr[512],ExpandedFormatStr[512];
  strcpy(FormatStr,Opt.UsePromptFormat ? Opt.PromptFormat:"$p$g");
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
          if (isalpha(CurDir[0]) && CurDir[1]==':' && CurDir[2]=='\\')
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
}



/* $ 10.05.2001 DJ
   показ history по Alt-F11 вынесен в отдельную функцию
*/

void CommandLine::ShowViewEditHistory()
{
  char Str[1024],ItemTitle[256];
  int Type,SelectType;
  if ((SelectType=CtrlObject->ViewHistory->Select(MSG(MViewHistoryTitle),"HistoryViews",Str,Type,ItemTitle))==1 || SelectType==2)
  {
    if (SelectType!=2)
      CtrlObject->ViewHistory->AddToHistory(Str,ItemTitle,Type);
    CtrlObject->ViewHistory->SetAddMode(FALSE,Opt.FlagPosixSemantics?1:2,TRUE);

    switch(Type)
    {
    case 0:
      new FileViewer(Str,TRUE);
      break;
    case 1:
      new FileEditor(Str,FALSE,TRUE);
      break;
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
    if (SelectType==3)
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
