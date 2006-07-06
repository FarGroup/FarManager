/*
cmdline.cpp

��������� ������

*/

/* Revision: 1.95 07.07.2006 $ */

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
  CmdStr.SetEditBeyondEnd(FALSE);
  SetPersistentBlocks(Opt.Dialogs.EditBlock);
  LastCmdPartLength=-1;
  BackgroundScreen=NULL;
}

CommandLine::~CommandLine()
{
}

/* $ 09.09.2001 IS ����������/�������� ���������� ����� */
void CommandLine::SetPersistentBlocks(int Mode)
{
  CmdStr.SetPersistentBlocks(Mode);
}
/* IS $ */

void CommandLine::DisplayObject()
{
  _OT(SysLog("[%p] CommandLine::DisplayObject()",this));
  string strTruncDir;
  GetPrompt(strTruncDir);
  TruncPathStrW(strTruncDir,(X2-X1)/2);
  GotoXY(X1,Y1);
  SetColor(COL_COMMANDLINEPREFIX);
  TextW(strTruncDir);
  CmdStr.SetObjectColor(COL_COMMANDLINE,COL_COMMANDLINESELECTED);
  CmdStr.SetLeftPos(0);
  CmdStr.SetPosition(X1+strTruncDir.GetLength(),Y1,X2,Y2);
  CmdStr.Show();
}


void CommandLine::SetCurPos(int Pos)
{
  CmdStr.SetLeftPos(0);
  CmdStr.SetCurPos(Pos);
  CmdStr.Redraw();
}

BOOL CommandLine::SetLastCmdStr(const wchar_t *Ptr)
{
  strLastCmdStr = Ptr;
  return TRUE;
}

int CommandLine::ProcessKey(int Key)
{
  const wchar_t *PStr;
  string strStr;

  if(Key >= MCODE_C_CMDLINE_BOF && Key <= MCODE_C_CMDLINE_SELECTED)
    return CmdStr.ProcessKey(Key-MCODE_C_CMDLINE_BOF+MCODE_C_BOF);
  if(Key == MCODE_V_ITEMCOUNT || Key == MCODE_V_CURPOS)
    return CmdStr.ProcessKey(Key);
  if(Key == MCODE_V_CMDLINE_ITEMCOUNT || Key == MCODE_V_CMDLINE_CURPOS)
    return CmdStr.ProcessKey(Key-MCODE_V_CMDLINE_ITEMCOUNT+MCODE_V_ITEMCOUNT);

  if ((Key==KEY_CTRLEND || Key==KEY_CTRLNUMPAD1) && CmdStr.GetCurPos()==CmdStr.GetLength())
  {
    if (LastCmdPartLength==-1)
      SetLastCmdStr(CmdStr.GetStringAddrW());

    strStr = strLastCmdStr;
    int CurCmdPartLength=strStr.GetLength ();
    CtrlObject->CmdHistory->GetSimilar(strStr,LastCmdPartLength);
    if (LastCmdPartLength==-1)
    {
      if(SetLastCmdStr(CmdStr.GetStringAddrW()))
        LastCmdPartLength=CurCmdPartLength;
    }
    CmdStr.SetStringW(strStr);
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
    + ��� ���������� ������� ������� ������ ������� */
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
    case KEY_CTRLE:
    case KEY_CTRLX:
      if(Key == KEY_CTRLE)
        CtrlObject->CmdHistory->GetPrev(strStr);
      else
        CtrlObject->CmdHistory->GetNext(strStr);
    case KEY_ESC:
      if(Key == KEY_ESC)
      {
        /* $ 24.09.2000 SVS
           ���� ������ ��������� �� "������������ ��� Esc", �� ������� �
           ������� �� ������ � ������ � ������ ���������.
        */
        if(Opt.CmdHistoryRule)
          CtrlObject->CmdHistory->SetFirst();
        PStr=L"";
      }
      else
        PStr=strStr;
      SetStringW(PStr);
      return(TRUE);
    case KEY_F2:
      ProcessUserMenu(0);
      return(TRUE);
    case KEY_ALTF8:
      {
        int Type;
        /* $ 19.09.2000 SVS
           - ��� ������ �� History (�� Alt-F8) ������ �� ������� ����������!
        */
        int SelectType=CtrlObject->CmdHistory->Select(UMSG(MHistoryTitle),L"History",strStr,Type);
        if(SelectType > 0 && SelectType <= 3)
        {
          SetStringW(strStr);
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
          FolderTree Tree(strStr,MODALTREE_ACTIVE,4,2,ScrX-4,ScrY-4, FALSE);
          FrameManager->GetCurrentFrame()->RedrawKeyBar(); // ������� ;-(
        }
        if ( !strStr.IsEmpty() )
        {
          Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
          ActivePanel->SetCurDirW(strStr,TRUE);
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
         ����� view/edit history ������� � ��������� ���������
      */
      ShowViewEditHistory();
      /* DJ $ */
      CtrlObject->Cp()->Redraw();
      return(TRUE);
    case KEY_ALTF12:
      {
        int Type;
        int SelectType=CtrlObject->FolderHistory->Select(UMSG(MFolderHistoryTitle),L"HistoryFolders",strStr,Type);
        /*
           SelectType = 0 - Esc
                        1 - Enter
                        2 - Shift-Enter
                        3 - Ctrl-Enter
                        6 - Ctrl-Shift-Enter - �� ��������� ������ �� ������ �������
        */
        if (SelectType == 1 || SelectType == 2 || SelectType == 6)
        {
          if (SelectType==2)
            CtrlObject->FolderHistory->SetAddMode(FALSE,2,TRUE);
          // ����� ������ ��� �������... ;-)
          Panel *Panel=CtrlObject->Cp()->ActivePanel;
          if(SelectType == 6)
            Panel=CtrlObject->Cp()->GetAnotherPanel(Panel);

          if(!CtrlObject->Plugins.ProcessCommandLine(strStr,Panel))
          {
            if(Panel->GetMode() == PLUGIN_PANEL || CheckShortcutFolderW(&strStr,FALSE))
            {
              Panel->SetCurDirW(strStr,Type==0 ? TRUE:FALSE);
              Panel->Redraw();
              CtrlObject->FolderHistory->SetAddMode(TRUE,2,TRUE);
            }
          }
        }
        else
          if (SelectType==3)
            SetStringW(strStr);
      }
      return(TRUE);
    case KEY_ENTER:
    case KEY_SHIFTENTER:
      {
        Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
        /* $ 19.02.2001 IS
             - ��������� ��� ��� �� �����
        */
        CmdStr.Select(-1,0);
        CmdStr.Show();
        /* IS $ */
        CmdStr.GetStringW(strStr);
        if ( strStr.IsEmpty() )
          break;

        ActivePanel->SetCurPath();

        if(!(Opt.ExcludeCmdHistory&EXCLUDECMDHISTORY_NOTCMDLINE))
          CtrlObject->CmdHistory->AddToHistory(strStr);

        ProcessOSAliasesW(strStr);

        char *Str = UnicodeToAnsi (strStr);

        if (!ActivePanel->ProcessPluginEvent(FE_COMMAND,(void *)Str))
        {
          wchar_t *lpwszStr = strStr.GetBuffer (); //BUGBUG
          CmdExecute(lpwszStr,FALSE,Key==KEY_SHIFTENTER,FALSE);

          strStr.ReleaseBuffer ();
        }

        xf_free (Str);
      }
      return(TRUE);

    /* �������������� ������� ��� ��������� � ��� ������.
       ��������!
       ��� ���������� ���� ���� ����� ������ ������ ����� "default"
    */
    case KEY_ALTSHIFTLEFT:  case KEY_ALTSHIFTNUMPAD4:
    case KEY_ALTSHIFTRIGHT: case KEY_ALTSHIFTNUMPAD6:
    case KEY_ALTSHIFTEND:   case KEY_ALTSHIFTNUMPAD1:
    case KEY_ALTSHIFTHOME:  case KEY_ALTSHIFTNUMPAD7:
      Key&=~KEY_ALT;

    default:
      /* $ 24.09.2000 SVS
         ���� �������� ������� ������ ������� Xlat, ��
         ��������� ������� ��� ���������, ���� ��� != 0
      */
      /* $ 04.11.2000 SVS
         �������� �� �������������� �������
      */
      if((Opt.XLat.XLatCmdLineKey && Key == Opt.XLat.XLatCmdLineKey) ||
         (Opt.XLat.XLatAltCmdLineKey && Key == Opt.XLat.XLatAltCmdLineKey) ||
         Key == MCODE_OP_XLAT)
      {
        /* 13.12.2000 SVS
           ! ��� CmdLine - ���� ��� ���������, ����������� ��� ������ (XLat)
        */
        CmdStr.Xlat(Opt.XLat.Flags&XLAT_CONVERTALLCMDLINE?TRUE:FALSE);
        /* SVS $ */
        /* $ 13.11.2001 IS ����� ����������� �������� ctrl-end */
        if(SetLastCmdStr(CmdStr.GetStringAddrW()))
          LastCmdPartLength=strLastCmdStr.GetLength ();
        /* IS $ */
        return(TRUE);
      }
      /* SVS $ */
      /* SVS $ */

      /* $ 18.12.2000 SVS
         ���������� ��������� �� ��������� ��������
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
         �������� �� "Ctrl-D - ������ ������"
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


void CommandLine::SetCurDirW(const wchar_t *CurDir)
{
    strCurDir = CurDir;
    PrepareDiskPathW (strCurDir);
}


int CommandLine::GetCurDirW(string &strCurDir)
{
    strCurDir = CommandLine::strCurDir;
    return strCurDir.GetLength();
}


void CommandLine::GetStringW (string &strStr)
{
  CmdStr.GetStringW(strStr);
}


void CommandLine::SetStringW(const wchar_t *Str,BOOL Redraw)
{
  LastCmdPartLength=-1;
  CmdStr.SetStringW(Str);
  CmdStr.SetLeftPos(0);
  if(Redraw)
    CmdStr.Show();
}



void CommandLine::ExecString(const wchar_t *Str,int AlwaysWaitFinish,int SeparateWindow,
                             int DirectRun)
{
  SetStringW(Str);
  CmdExecute(Str,AlwaysWaitFinish,SeparateWindow,DirectRun);
}


void CommandLine::InsertStringW(const wchar_t *Str)
{
  LastCmdPartLength=-1;
  CmdStr.InsertStringW(Str);
  CmdStr.Show();
}


int CommandLine::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  return(CmdStr.ProcessMouse(MouseEvent));
}

void add_char (string &str, wchar_t c) //BUGBUG
{
    wchar_t cc[2];

    cc[0] = c;
    cc[1] = 0;

    str += (const wchar_t*)&cc;
}

void CommandLine::GetPrompt(string &strDestStr)
{
#if 0
  char FormatStr[512],ExpandedFormatStr[512];
  string strFormatStr;
  string strExpandedFormatStr;

  if ( Opt.UsePromptFormat )
    strFormatStr = Opt.strPromptFormat;
  else
    strFormatStr = L"$p$g";

  const wchar_t *Format=strFormatStr;

  if (Opt.UsePromptFormat)
  {
    ExpandEnvironmentStrW(strFormatStr, strExpandedFormatStr);
    Format = strExpandedFormatStr;
  }

  while (*Format)
  {
    if (*Format==L'$')
    {
      Format++;
      switch(*Format)
      {
        case L'$':
          strDestStr += L'$';
          break;
        case L'p':
          strDestStr += strCurDir;
          break;
        case L'n':
          if (IsLocalPathW(strCurDir) && strCurDir.At(2)==L'\\')
            add_char (strDestStr, LocalUpperW(strCurDir.At(0)));
          else
            add_char (strDestStr, L'?');
          break;
        case L'g':
          add_char (strDestStr, L'>');
          break;
      }
      Format++;
    }
    else
      add_char (strDestStr, *(Format++));
  }

#else
  // ����������� ������� ���������, ��� � XP
  if (Opt.UsePromptFormat)
  {
    string strFormatStr, strExpandedFormatStr;
    strFormatStr = Opt.strPromptFormat;
    apiExpandEnvironmentStrings (strFormatStr, strExpandedFormatStr);
    const wchar_t *Format=strExpandedFormatStr;
    wchar_t ChrFmt[][2]={
      {L'A',L'&'},   // $A - & (Ampersand)
      {L'B',L'|'},   // $B - | (pipe)
      {L'C',L'('},   // $C - ( (Left parenthesis)
      {L'F',L')'},   // $F - ) (Right parenthesis)
      {L'G',L'>'},   // $G - > (greater-than sign)
      {L'L',L'<'},   // $L - < (less-than sign)
      {L'Q',L'='},   // $Q - = (equal sign)
      {L'S',L' '},   // $S - (space)
      {L'$',L'$'},   // $$ - $ (dollar sign)
    };
    while (*Format)
    {
      if (*Format==L'$')
      {
        wchar_t Chr=LocalUpperW(*++Format);
        int I;
        for(I=0; I < sizeof(ChrFmt)/sizeof(ChrFmt[0]); ++I)
        {
          if(ChrFmt[I][0] == Chr)
          {
            add_char (strDestStr, ChrFmt[I][1]);
            break;
          }
        }

        if(I == sizeof(ChrFmt)/sizeof(ChrFmt[0]))
        {
          switch(Chr)
          {
            /* ��� �� �����������
            $E - Escape code (ASCII code 27)
            $V - Windows XP version number
            $_ - Carriage return and linefeed
            */
            case L'H': // $H - Backspace (erases previous character)
              strDestStr.GetBuffer (strDestStr.GetLength()-1);
              strDestStr.ReleaseBuffer (); //BUGBUG
              break;
            case L'D': // $D - Current date
            case L'T': // $T - Current time
            {
              string strDateTime;
              MkStrFTimeW(strDateTime,(Chr==L'D'?L"%D":L"%T"));
              strDestStr += strDateTime;
              break;
            }
            case L'N': // $N - Current drive
              if (IsLocalPathW(strCurDir) && strCurDir.At(2)==L'\\')
                add_char (strDestStr, LocalUpperW(strCurDir.At(0)));
              else
                add_char (strDestStr, L'?');
              break;
            case L'P': // $P - Current drive and path
              strDestStr+=strCurDir;
              break;
          }
        }
        Format++;
      }
      else
        add_char (strDestStr, *(Format++));
    }
  }
  else // default prompt = "$p$g"
  {
    strDestStr = strCurDir;
    strDestStr += L">";
  }
#endif
}



/* $ 10.05.2001 DJ
   ����� history �� Alt-F11 ������� � ��������� �������
*/

void CommandLine::ShowViewEditHistory()
{
  string strStr;
  string strItemTitle;
  int Type;

  int SelectType=CtrlObject->ViewHistory->Select(UMSG(MViewHistoryTitle),L"HistoryViews",strStr,Type,&strItemTitle);
  /*
     SelectType = 0 - Esc
                  1 - Enter
                  2 - Shift-Enter
                  3 - Ctrl-Enter
  */

  if (SelectType == 1 || SelectType == 2)
  {
    if (SelectType!=2)
      CtrlObject->ViewHistory->AddToHistory(strStr,strItemTitle,Type);
    CtrlObject->ViewHistory->SetAddMode(FALSE,Opt.FlagPosixSemantics?1:2,TRUE);

    switch(Type)
    {
      case 0: // ������
      {
        new FileViewer(strStr,TRUE);
        break;
      }

      case 1: // ������� �������� � ���������
      case 4: // �������� � �����
      {
        // ����� ���� ���������
        FileEditor *FEdit=new FileEditor(strStr,TRUE,TRUE);
        if(Type == 4)
           FEdit->SetLockEditor(TRUE);
        break;
      }

      // 2 � 3 - ����������� � ProcessExternal
      case 2:
      case 3:
      {
        if ( strStr.At(0) !=L'@' )
        {
          ExecString(strStr,Type-2);
        }
        else
        {
          SaveScreen SaveScr;
          CtrlObject->Cp()->LeftPanel->CloseFile();
          CtrlObject->Cp()->RightPanel->CloseFile();

          Execute((const wchar_t*)strStr+1,Type-2);
        }
        break;
      }
    }
    CtrlObject->ViewHistory->SetAddMode(TRUE,Opt.FlagPosixSemantics?1:2,TRUE);
  }
  else
    if (SelectType==3) // ������� �� ������� � ���.������?
      SetStringW(strStr);
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

void CommandLine::GetSelStringW (string &strStr)
{
  CmdStr.GetSelStringW(strStr);
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
