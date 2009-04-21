/*
cmdline.cpp

Командная строка

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

  if (BackgroundScreen)
    delete BackgroundScreen;
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
  //CmdStr.SetLeftPos(0);
  CmdStr.SetPosition(X1+(int)strlen(TruncDir),Y1,X2,Y2);
  CmdStr.Show();
}


void CommandLine::SetCurPos(int Pos, int LeftPos)
{
  CmdStr.SetLeftPos(LeftPos);
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

__int64 CommandLine::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
  if(OpCode >= MCODE_C_CMDLINE_BOF && OpCode <= MCODE_C_CMDLINE_SELECTED)
    return CmdStr.VMProcess(OpCode-MCODE_C_CMDLINE_BOF+MCODE_C_BOF,vParam,iParam);
  if(OpCode >= MCODE_C_BOF && OpCode <= MCODE_C_SELECTED)
    return CmdStr.VMProcess(OpCode,vParam,iParam);
  if(OpCode == MCODE_V_ITEMCOUNT || OpCode == MCODE_V_CURPOS)
    return CmdStr.VMProcess(OpCode,vParam,iParam);
  if(OpCode == MCODE_V_CMDLINE_ITEMCOUNT || OpCode == MCODE_V_CMDLINE_CURPOS)
    return CmdStr.VMProcess(OpCode-MCODE_V_CMDLINE_ITEMCOUNT+MCODE_V_ITEMCOUNT,vParam,iParam);

  if(OpCode == MCODE_F_EDITOR_SEL)
    return CmdStr.VMProcess(MCODE_F_EDITOR_SEL,vParam,iParam);

  return _i64(0);
}

int CommandLine::ProcessKey(int Key)
{
  char Str[2048], *PStr;

  if ((Key==KEY_CTRLEND || Key==KEY_CTRLNUMPAD1) && CmdStr.GetCurPos()==CmdStr.GetLength())
  {
    if (LastCmdPartLength==-1)
      SetLastCmdStr(CmdStr.GetStringAddr(),CmdStr.GetLength());

    if(!LastCmdStr)
      return TRUE;

    xstrncpy(Str,LastCmdStr,sizeof(Str)-1);
    int CurCmdPartLength=(int)strlen(Str);
    CtrlObject->CmdHistory->GetSimilar(Str,sizeof(Str),LastCmdPartLength);
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
    switch(Key)
    {
      case KEY_MSWHEEL_UP:    Key = KEY_CTRLE; break;
      case KEY_MSWHEEL_DOWN:  Key = KEY_CTRLX; break;
      case KEY_MSWHEEL_LEFT:  Key = KEY_CTRLS; break;
      case KEY_MSWHEEL_RIGHT: Key = KEY_CTRLD; break;
    }
  }
  /* VVM $ */

  switch(Key)
  {
    //case KEY_TAB: // autocomplete
    //{
      //xstrncpy(Str,,sizeof(Str)-1);
      //CmdStr.SetString(Str);
      //Show();
    //  return(TRUE);
    //}

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
          CtrlObject->CmdHistory->ResetPosition();
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
            ProcessKey(SelectType==1?(int)KEY_ENTER:(int)KEY_SHIFTENTER);
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
          FolderTree Tree(Str,MODALTREE_ACTIVE,TRUE,FALSE);
        }
        CtrlObject->Cp()->RedrawKeyBar();
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

          //Type==1 - плагиновый путь
          //Type==0 - обычный путь
          //если путь плагиновый то сначала попробуем запустить его (а вдруг там префикс)
          //ну а если путь не плагиновый то запускать его точно не надо
          if(Type==0 || !CtrlObject->Plugins.ProcessCommandLine(Str,Panel))
          {
            if(Panel->GetMode() == PLUGIN_PANEL ||
               CheckShortcutFolder(Str,sizeof(Str)-1,FALSE))
            {
              Panel->SetCurDir(Str,Type==0 ? TRUE:FALSE);
              Panel->Redraw();
              CtrlObject->FolderHistory->SetAddMode(TRUE,2,TRUE);
            }
          }
        }
        else
          if (SelectType==3)
            SetString(Str);
      }
      return(TRUE);
    case KEY_NUMENTER:
    case KEY_SHIFTNUMENTER:
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
          CmdExecute(Str,FALSE,Key==KEY_SHIFTENTER||Key==KEY_SHIFTNUMENTER,FALSE);
      }
      return(TRUE);


    case KEY_CTRLU:
      CmdStr.Select(-1,0);
      CmdStr.Show();
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
         Key == KEY_OP_XLAT)
      {
        //   ! Для CmdLine - если нет выделения, преобразуем всю строку (XLat)
        CmdStr.Xlat(Opt.XLat.Flags&XLAT_CONVERTALLCMDLINE?TRUE:FALSE);
        /* $ 13.11.2001 IS иначе неправильно работает ctrl-end */
        if(SetLastCmdStr(CmdStr.GetStringAddr(),CmdStr.GetLength()))
          LastCmdPartLength=(int)strlen(LastCmdStr);
        return(TRUE);
      }

      //   Сбрасываем выделение на некоторых клавишах
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

      // Написано же "Ctrl-D - Символ вправо"
      if(Key == KEY_CTRLD)
        Key=KEY_RIGHT;

      if (!CmdStr.ProcessKey(Key))
        break;

      LastCmdPartLength=-1;
      return(TRUE);
  }
  return(FALSE);
}


BOOL CommandLine::SetCurDir(const char *CurDir)
{
  xstrncpy(CommandLine::CurDir,CurDir,sizeof(CommandLine::CurDir)-1);
	if(CtrlObject->Cp()->ActivePanel->GetMode()!=PLUGIN_PANEL)
		PrepareDiskPath(CommandLine::CurDir,sizeof(CommandLine::CurDir)-1);
  return TRUE;
}


int CommandLine::GetCurDir(char *CurDir)
{
  if(CurDir)
    strcpy(CurDir,CommandLine::CurDir); // TODO: ОПАСНО!!!
  return (int)strlen(CommandLine::CurDir);
}

void CommandLine::GetString(char *Str,int MaxSize)
{
  CmdStr.GetString(Str,MaxSize);
}

const char *CommandLine::GetStringAddr()
{
  return CmdStr.GetStringAddr();
}

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
#if 0
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
    char ExpandedFormatStr[512];
    ExpandEnvironmentStr(Opt.PromptFormat,ExpandedFormatStr,sizeof(ExpandedFormatStr));
    char *Format=ExpandedFormatStr;
    static char ChrFmt[][2]={
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
  char Str[1024];
  int Type;

  int SelectType=CtrlObject->ViewHistory->Select(MSG(MViewHistoryTitle),"HistoryViews",Str,sizeof(Str),Type);
  /*
     SelectType = 0 - Esc
                  1 - Enter
                  2 - Shift-Enter
                  3 - Ctrl-Enter
  */

  if (SelectType == 1 || SelectType == 2)
  {
    if (SelectType!=2)
      CtrlObject->ViewHistory->AddToHistory(Str,Type);
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
        // пусть файл создается
        FileEditor *FEdit=new FileEditor(Str,FFILEEDIT_CANNEWFILE|FFILEEDIT_ENABLEF6);
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
