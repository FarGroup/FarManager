/*
cmdline.cpp

Командная строка

*/

/* Revision: 1.13 11.04.2001 $ */

/*
Modify:
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
      как всегда добавил CtrlObject->Redraw()
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */


CommandLine::CommandLine()
{
  *CurDir=0;
  CmdStr.SetEditBeyondEnd(FALSE);
  LastCmdPartLength=-1;
  *LastCmdStr=0;
}


void CommandLine::DisplayObject()
{
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
  char Str[512];

  if (Key==KEY_CTRLEND && CmdStr.GetCurPos()==CmdStr.GetLength())
  {
    char Command[1024];
    if (LastCmdPartLength==-1)
      strncpy(LastCmdStr,CmdStr.GetStringAddr(),sizeof(LastCmdStr));
    strcpy(Command,LastCmdStr);
    int CurCmdPartLength=strlen(Command);
    CtrlObject->CmdHistory->GetSimilar(Command,LastCmdPartLength);
    if (LastCmdPartLength==-1)
    {
      LastCmdPartLength=CurCmdPartLength;
      strncpy(LastCmdStr,CmdStr.GetStringAddr(),sizeof(LastCmdStr));
    }
    CmdStr.SetString(Command);
    Show();
    return(TRUE);
  }

  switch(Key)
  {
    case KEY_UP:
      if (CtrlObject->LeftPanel->IsVisible() || CtrlObject->RightPanel->IsVisible())
        return(FALSE);
    case KEY_CTRLE:
      {
        char Str[1024];
        CtrlObject->CmdHistory->GetPrev(Str);
        CmdStr.SetString(Str);
        CmdStr.SetLeftPos(0);
        CmdStr.Show();
      }
      LastCmdPartLength=-1;
      return(TRUE);
    case KEY_DOWN:
      if (CtrlObject->LeftPanel->IsVisible() || CtrlObject->RightPanel->IsVisible())
        return(FALSE);
    case KEY_CTRLX:
      {
        char Str[1024];
        CtrlObject->CmdHistory->GetNext(Str);
        CmdStr.SetString(Str);
        CmdStr.SetLeftPos(0);
        CmdStr.Show();
      }
      LastCmdPartLength=-1;
      return(TRUE);
    case KEY_F2:
      ProcessUserMenu(0);
      return(TRUE);
    case KEY_ALTF8:
      {
        char Str[1024];
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
      CtrlObject->ExitMainLoop(TRUE);
      return(TRUE);
    case KEY_ALTF10:
      {
        char NewFolder[NM];
        {
          FolderTree Tree(NewFolder,MODALTREE_ACTIVE,4,2,ScrX-4,ScrY-4);
        }
        if (*NewFolder)
        {
          Panel *ActivePanel=CtrlObject->ActivePanel;
          ActivePanel->SetCurDir(NewFolder,TRUE);
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
            {
              FileViewer *ShellViewer=new FileViewer(Str,TRUE);
              CtrlObject->ModalManager.AddModal(ShellViewer);
              break;
            }
            case 1:
            {
              FileEditor *ShellEditor=new FileEditor(Str,FALSE,TRUE);
              CtrlObject->ModalManager.AddModal(ShellEditor);
              break;
            }
            case 2:
            case 3:
            {
              if (*Str!='@')
                ExecString(Str,Type-2);
              else
              {
                SaveScreen SaveScr;
                CtrlObject->LeftPanel->CloseFile();
                CtrlObject->RightPanel->CloseFile();
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
      CtrlObject->Redraw();
      return(TRUE);
    case KEY_F12:
      CtrlObject->ModalManager.SelectModal();
      CtrlObject->ModalManager.NextModal(0);
      return(TRUE);
    case KEY_ALTF12:
      {
        char Str[1024];
        int Type,SelectType;
        if ((SelectType=CtrlObject->FolderHistory->Select(MSG(MFolderHistoryTitle),"HistoryFolders",Str,Type))==1 || SelectType==2)
        {
          if (SelectType==2)
            CtrlObject->FolderHistory->SetAddMode(FALSE,2,TRUE);
          CtrlObject->ActivePanel->SetCurDir(Str,Type==0 ? TRUE:FALSE);
          CtrlObject->ActivePanel->Redraw();
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
        Panel *ActivePanel=CtrlObject->ActivePanel;
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
    case KEY_ESC:
      /* $ 24.09.2000 SVS
         Если задано поведение по "Несохранению при Esc", то позицию в
         хистори не меняем и ставим в первое положение.
      */
      if(Opt.CmdHistoryRule)
        CtrlObject->CmdHistory->SetFirst();
      /* SVS $ */
      CmdStr.SetString("");
      CmdStr.SetLeftPos(0);
      CmdStr.Show();
      LastCmdPartLength=-1;
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
         (Opt.XLat.XLatAltCmdLineKey && Key == Opt.XLat.XLatAltCmdLineKey))
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


int CommandLine::CmdExecute(char *CmdLine,int AlwaysWaitFinish,
                            int SeparateWindow,int DirectRun)
{
  LastCmdPartLength=-1;
  if (!SeparateWindow && CtrlObject->Plugins.ProcessCommandLine(CmdLine))
  {
    CmdStr.SetString("");
    GotoXY(X1,Y1);
    mprintf("%*s",X2-X1+1,"");
    Show();
    ScrBuf.Flush();
    return(-1);
  }
  int Code;
  {
    RedrawDesktop Redraw;
    CtrlObject->LeftPanel->CloseChangeNotification();
    CtrlObject->RightPanel->CloseChangeNotification();
    CtrlObject->LeftPanel->CloseFile();
    CtrlObject->RightPanel->CloseFile();

    ScrollScreen(1);
    MoveCursor(X1,Y1);
    if (CurDir[0] && CurDir[1]==':')
      chdir(CurDir);
    CmdStr.SetString("");
    if (ProcessOSCommands(CmdLine))
      Code=-1;
    else
      Code=Execute(CmdLine,AlwaysWaitFinish,SeparateWindow,DirectRun);
    int CurX,CurY;
    GetCursorPos(CurX,CurY);
    if (CurY>=Y1-1)
      ScrollScreen(Min(CurY-Y1+2,Opt.ShowKeyBar ? 2:1));
    CtrlObject->LeftPanel->Update(UPDATE_KEEP_SELECTION);
    CtrlObject->RightPanel->Update(UPDATE_KEEP_SELECTION);
    GotoXY(X1,Y1);
    mprintf("%*s",X2-X1+1,"");
  }
  ScrBuf.Flush();
  return(Code);
}


/* $ 14.01.2001 SVS
   + В ProcessOSCommands добавлена обработка
     "IF [NOT] EXIST filename command"
     "IF [NOT] DEFINED variable command"

   Эта функция предназначена для обработки вложенного IF`а
   CmdLine - полная строка вида
     if exist file if exist file2 command
   Return - указатель на "command"
            пуская строка - условие не выполнимо
            NULL - не попался "IF" или ошибки в предложении, например
                   не exist, а esist или предложение неполно.

   DEFINED - подобно EXIST, но оперирует с переменными среды

   Исходная строка (CmdLine) не модифицируется!!!
*/
char* WINAPI PrepareOSIfExist(char *CmdLine)
{
  if(!CmdLine || !*CmdLine)
    return NULL;

  char Cmd[1024], *PtrCmd=CmdLine, *CmdStart;
  int Not=FALSE;
  int Exist=0; // признак наличия конструкции "IF [NOT] EXIST filename command"
               // > 0 - эсть такая конструкция

  while(1)
  {
    if (!PtrCmd || !*PtrCmd || memicmp(PtrCmd,"IF ",3))
      break;

    PtrCmd+=3; while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;

    if (memicmp(PtrCmd,"NOT ",4)==0)
    {
      Not=TRUE;
      PtrCmd+=4; while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
    }

    if (*PtrCmd && !memicmp(PtrCmd,"EXIST ",6))
    {
      PtrCmd+=6; while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
      CmdStart=PtrCmd;
      if(*PtrCmd == '"')
        PtrCmd=strchr(PtrCmd+1,'"');

      if(PtrCmd && *PtrCmd)
      {
        PtrCmd=strchr(PtrCmd,' ');
        if(PtrCmd && *PtrCmd && *PtrCmd == ' ')
        {
          char ExpandedStr[8192];
          memmove(Cmd,CmdStart,PtrCmd-CmdStart+1);
          Cmd[PtrCmd-CmdStart]=0;
          Unquote(Cmd);
//SysLog(Cmd);
          if (ExpandEnvironmentStrings(Cmd,ExpandedStr,sizeof(ExpandedStr))!=0)
          {
            DWORD FileAttr=GetFileAttributes(ExpandedStr);
//SysLog("%08X ExpandedStr=%s",FileAttr,ExpandedStr);
            if(FileAttr != (DWORD)-1 && !Not || FileAttr == (DWORD)-1 && Not)
            {
              while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd;
              Exist++;
            }
            else
              return "";
          }
        }
      }
    }
    // "IF [NOT] DEFINED variable command"
    else if (*PtrCmd && !memicmp(PtrCmd,"DEFINED ",8))
    {
      PtrCmd+=8; while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
      CmdStart=PtrCmd;
      if(*PtrCmd == '"')
        PtrCmd=strchr(PtrCmd+1,'"');

      if(PtrCmd && *PtrCmd)
      {
        PtrCmd=strchr(PtrCmd,' ');
        if(PtrCmd && *PtrCmd && *PtrCmd == ' ')
        {
          char ExpandedStr[8192];
          memmove(Cmd,CmdStart,PtrCmd-CmdStart+1);
          Cmd[PtrCmd-CmdStart]=0;
          DWORD ERet=GetEnvironmentVariable(Cmd,ExpandedStr,sizeof(ExpandedStr));
//SysLog(Cmd);
          if(ERet && !Not || !ERet && Not)
          {
            while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd;
            Exist++;
          }
          else
            return "";
        }
      }
    }
  }
  return Exist?PtrCmd:NULL;
}
/* SVS $ */


int CommandLine::ProcessOSCommands(char *CmdLine)
{
  Panel *SetPanel;
  int Length;
  SetPanel=CtrlObject->ActivePanel;
  if (SetPanel->GetType()!=FILE_PANEL && CtrlObject->GetAnotherPanel(SetPanel)->GetType()==FILE_PANEL)
    SetPanel=CtrlObject->GetAnotherPanel(SetPanel);
  RemoveTrailingSpaces(CmdLine);
  if (isalpha(CmdLine[0]) && CmdLine[1]==':' && CmdLine[2]==0)
  {
    int NewDisk=toupper(CmdLine[0])-'A';
    setdisk(NewDisk);
    if (getdisk()!=NewDisk)
    {
      char NewDir[10];
      sprintf(NewDir,"%c:\\",NewDisk+'A');
      chdir(NewDir);
      setdisk(NewDisk);
    }
    SetPanel->ChangeDirToCurrent();
    return(TRUE);
  }
  if (strnicmp(CmdLine,"SET ",4)==0)
  {
    char Cmd[1024];
    strcpy(Cmd,CmdLine+4);
    char *Value=strchr(Cmd,'=');
    if (Value==NULL)
      return(FALSE);
    *Value=0;
    if (Value[1]==0)
      SetEnvironmentVariable(Cmd,NULL);
    else
    {
      char ExpandedStr[8192];
      if (ExpandEnvironmentStrings(Value+1,ExpandedStr,sizeof(ExpandedStr))!=0)
        SetEnvironmentVariable(Cmd,ExpandedStr);
    }
    return(TRUE);
  }
  /* $ 14.01.2001 SVS
     + В ProcessOSCommands добавлена обработка
       "IF [NOT] EXIST filename command"
       "IF [NOT] DEFINED variable command"
  */
  if (memicmp(CmdLine,"IF ",3)==0)
  {
    char *PtrCmd=PrepareOSIfExist(CmdLine);
    // здесь PtrCmd - уже готовая команда, без IF
    if(PtrCmd && *PtrCmd && CtrlObject->Plugins.ProcessCommandLine(PtrCmd))
    {
      CmdStr.SetString("");
      GotoXY(X1,Y1);
      mprintf("%*s",X2-X1+1,"");
      Show();
      return TRUE;
    }
    return FALSE;
  }
  /* SVS $ */

  if ((strnicmp(CmdLine,"CD",Length=2)==0 || strnicmp(CmdLine,"CHDIR",Length=5)==0) &&
      (isspace(CmdLine[Length]) || CmdLine[Length]=='\\' || strcmp(CmdLine+Length,"..")==0))
  {
    int ChDir=(Length==5);
    while (isspace(CmdLine[Length]))
      Length++;
    if (CmdLine[Length]=='\"')
      Length++;
    char NewDir[NM];
    strcpy(NewDir,&CmdLine[Length]);

    if (CtrlObject->Plugins.ProcessCommandLine(NewDir))
    {
      CmdStr.SetString("");
      GotoXY(X1,Y1);
      mprintf("%*s",X2-X1+1,"");
      Show();
      return(TRUE);
    }

    char *ChPtr=strrchr(NewDir,'\"');
    if (ChPtr!=NULL)
      *ChPtr=0;
    if (SetPanel->GetType()==FILE_PANEL && SetPanel->GetMode()==PLUGIN_PANEL)
    {
      SetPanel->SetCurDir(NewDir,ChDir);
      return(TRUE);
    }
    char ExpandedDir[8192];
    if (ExpandEnvironmentStrings(NewDir,ExpandedDir,sizeof(ExpandedDir))!=0)
      if (chdir(ExpandedDir)==-1)
        return(FALSE);
    SetPanel->ChangeDirToCurrent();
    if (!SetPanel->IsVisible())
      SetPanel->SetTitle();
    return(TRUE);
  }
  return(FALSE);
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
    ExpandEnvironmentStrings(FormatStr,ExpandedFormatStr,sizeof(ExpandedFormatStr));
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


int CommandLine::GetCurPos()
{
  return(CmdStr.GetCurPos());
}
