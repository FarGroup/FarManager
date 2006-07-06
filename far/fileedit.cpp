/*
fileedit.cpp

�������������� ����� - ���������� ��� editor.cpp

*/

/* Revision: 1.197 07.07.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "fileedit.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "poscache.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "dialog.hpp"
#include "fileview.hpp"
#include "help.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "namelist.hpp"
#include "history.hpp"
#include "cmdline.hpp"
#include "scrbuf.hpp"
#include "savescr.hpp"


FileEditor::FileEditor(const wchar_t *Name,int CreateNewFile,int EnableSwitch,
                       int StartLine,int StartChar,int DisableHistory,
                       const wchar_t *PluginData,int ToSaveAs, int OpenModeExstFile)
{
  _ECTLLOG(CleverSysLog SL("FileEditor::FileEditor(1)"));
  _KEYMACRO(SysLog("FileEditor::FileEditor(1)"));
  _KEYMACRO(SysLog(1));
  ScreenObject::SetPosition(0,0,ScrX,ScrY);
  Flags.Set(FFILEEDIT_FULLSCREEN);
  Init(Name,NULL,CreateNewFile,EnableSwitch,StartLine,StartChar,
       DisableHistory,PluginData,ToSaveAs,FALSE,OpenModeExstFile);
}


FileEditor::FileEditor(const wchar_t *Name,int CreateNewFile,int EnableSwitch,
            int StartLine,int StartChar,const wchar_t *Title,
            int X1,int Y1,int X2,int Y2,int DisableHistory, int DeleteOnClose,
            int OpenModeExstFile)
{
  _ECTLLOG(CleverSysLog SL("FileEditor::FileEditor(2)"));
  _KEYMACRO(SysLog("FileEditor::FileEditor(2)"));
  _KEYMACRO(SysLog(1));
  /* $ 02.11.2001 IS
       ������������� ���������� ������ �������� ���� ���������� �� �������
  */
  if(X1 < 0)
    X1=0;
  if(X2 < 0 || X2 > ScrX)
    X2=ScrX;
  if(Y1 < 0)
    Y1=0;
  if(Y2 < 0 || Y2 > ScrY)
    Y2=ScrY;
  if(X1 >= X2)
  {
    X1=0;
    X2=ScrX;
  }
  if(Y1 >= Y2)
  {
    Y1=0;
    Y2=ScrY;
  }

  /* IS $ */
  ScreenObject::SetPosition(X1,Y1,X2,Y2);
  Flags.Change(FFILEEDIT_FULLSCREEN,(X1==0 && Y1==0 && X2==ScrX && Y2==ScrY));
  Init(Name,Title,CreateNewFile,EnableSwitch,StartLine,StartChar,DisableHistory,L"",
       FALSE,DeleteOnClose,OpenModeExstFile);
}

/* $ 07.05.2001 DJ
   � ����������� ������� EditNamesList, ���� �� ��� ������, � � SetNamesList()
   ������� EditNamesList � �������� ���� ��������
*/
/*
  ����� ������������ ���� ���:
    FileEditor::~FileEditor()
    Editor::~Editor()
    ...
*/
FileEditor::~FileEditor()
{
  _OT(SysLog("[%p] FileEditor::~FileEditor()",this));

  //AY: ���� ����������� �������� ���������.
  bClosing = true;

  if (FEdit->EdOpt.SavePos && CtrlObject!=NULL)
  {
    int ScreenLinePos=FEdit->CalcDistance(FEdit->TopScreen,FEdit->CurLine,-1);
    int CurPos=FEdit->CurLine->EditLine.GetTabCurPos();
    int LeftPos=FEdit->CurLine->EditLine.GetLeftPos();

    string strCacheName;

    if ( !strPluginData.IsEmpty() )
      strCacheName.Format (L"%s%s",(const wchar_t*)strPluginData,PointToNameW(strFullFileName));
    else
      strCacheName = strFullFileName;

    unsigned int Table=0;
    if (FEdit->Flags.Check(FEDITOR_TABLECHANGEDBYUSER))
    {
      Table=1;
      if (FEdit->AnsiText)
        Table=2;
      else
        if (FEdit->UseDecodeTable)
          Table=FEdit->TableNum+2;
    }

    if (!FEdit->Flags.Check(FEDITOR_OPENFAILED)) // ����� ���� � ��� �������� :-(
    {
      struct TPosCache32 PosCache={0};
      PosCache.Param[0]=FEdit->NumLine;
      PosCache.Param[1]=ScreenLinePos;
      PosCache.Param[2]=CurPos;
      PosCache.Param[3]=LeftPos;
      PosCache.Param[4]=Table;
      if(Opt.EdOpt.SaveShortPos)
      {
        PosCache.Position[0]=FEdit->SavePos.Line;
        PosCache.Position[1]=FEdit->SavePos.Cursor;
        PosCache.Position[2]=FEdit->SavePos.ScreenLine;
        PosCache.Position[3]=FEdit->SavePos.LeftPos;
      }
      CtrlObject->EditorPosCache->AddPosition(strCacheName,&PosCache);
    }
  }

  BitFlags FEditFlags=FEdit->Flags;
  int FEditEditorID=FEdit->EditorID;

  if (!FEditFlags.Check(FEDITOR_OPENFAILED))
  {
    FileEditor *save = CtrlObject->Plugins.CurEditor;
    CtrlObject->Plugins.CurEditor=this;
    CtrlObject->Plugins.ProcessEditorEvent(EE_CLOSE,&FEditEditorID);
    /* $ 11.10.2001 IS
       ������ ���� ������ � ���������, ���� ��� �������� � ����� � ����� ��
       ������ �� ������� � ������ �������.
    */
    /* $ 14.06.2001 IS
       ���� ���������� FEDITOR_DELETEONLYFILEONCLOSE � �������
       FEDITOR_DELETEONCLOSE, �� ������� ������ ����.
    */
    if (FEditFlags.Check(FEDITOR_DELETEONCLOSE|FEDITOR_DELETEONLYFILEONCLOSE) &&
       !FrameManager->CountFramesWithName(strFullFileName))
    {
       if(FEditFlags.Check(FEDITOR_DELETEONCLOSE))
         DeleteFileWithFolderW(strFullFileName);
       else
       {
         SetFileAttributesW(strFullFileName,FILE_ATTRIBUTE_NORMAL);
         DeleteFileW(strFullFileName); //BUGBUG
       }
    }
    /* IS 14.06.2002 $ */
    /* IS 11.10.2001 $ */
    CtrlObject->Plugins.CurEditor = save;
  }

  if(FEdit)
    delete FEdit;
  FEdit=NULL;

  CurrentEditor=NULL;
  if (EditNamesList)
    delete EditNamesList;

  _KEYMACRO(SysLog(-1));
  _KEYMACRO(SysLog("FileEditor::~FileEditor()"));
}

void FileEditor::Init(const wchar_t *Name,const wchar_t *Title,int CreateNewFile,int EnableSwitch,
                      int StartLine,int StartChar,int DisableHistory,
                      const wchar_t *PluginData,int ToSaveAs,int DeleteOnClose,
                      int OpenModeExstFile)
{
  _ECTLLOG(CleverSysLog SL("FileEditor::Init()"));
  _ECTLLOG(SysLog("(Name=%s, Title=%s)",Name,Title));
  SysErrorCode=0;
  int BlankFileName=!wcscmp(Name,UMSG(MNewFileName));

  //AY: ���� ����������� �������� ���������.
  bClosing = false;

  FEdit=new Editor;
  FEdit->SetOwner (this);

  if(!FEdit)
  {
    ExitCode=XC_OPEN_ERROR;
    return;
  }

  /* $ 19.02.2001 IS
       � �� ����, ��� ��� ������ ����� GetFileAttributes �� ����������...
  */
  *AttrStr=0;
  /* IS $ */
  CurrentEditor=this;
  FileAttributes=-1;
  FileAttributesModified=false;
  SetTitle(Title);
  /* $ 07.05.2001 DJ */
  EditNamesList = NULL;
  KeyBarVisible = Opt.EdOpt.ShowKeyBar;
  /* DJ $ */
  /* $ 10.05.2001 DJ */
  Flags.Change(FFILEEDIT_DISABLEHISTORY,DisableHistory);
  Flags.Change(FFILEEDIT_ENABLEF6,EnableSwitch);
  /* DJ $ */
  /* $ 17.08.2001 KM
    ��������� ��� ������ �� AltF7. ��� �������������� ���������� ����� ��
    ������ ��� ������� F2 ������� ����� ShiftF2.
  */
  if(BlankFileName)
    CreateNewFile=1;
  Flags.Change(FFILEEDIT_SAVETOSAVEAS,(ToSaveAs||BlankFileName?TRUE:FALSE));
  /* KM $ */

  if (*Name==0)
  {
    ExitCode=XC_OPEN_ERROR;
    return;
  }

  SetPluginData(PluginData);
  FEdit->SetHostFileEditor(this);
  _OT(SysLog("Editor;:Editor(), EnableSwitch=%i",EnableSwitch));
  SetCanLoseFocus(EnableSwitch);

  FarGetCurDirW (strStartDir);

  if(!SetFileName(Name))
  {
    ExitCode=XC_OPEN_ERROR;
    return;
  }

  /*$ 11.05.2001 OT */
  //int FramePos=FrameManager->FindFrameByFile(MODALTYPE_EDITOR,FullFileName);
  //if (FramePos!=-1)
  if (EnableSwitch)
  {
    //if (EnableSwitch)
    int FramePos=FrameManager->FindFrameByFile(MODALTYPE_EDITOR, strFullFileName);
    if (FramePos!=-1)
    {
      int SwitchTo=FALSE;
      int MsgCode=0;
      if (!(*FrameManager)[FramePos]->GetCanLoseFocus(TRUE) ||
          Opt.Confirm.AllowReedit)
      {
        if(OpenModeExstFile == FEOPMODE_QUERY)
        {
          string strMsgFullFileName;
          strMsgFullFileName = strFullFileName;
          SetMessageHelp(L"EditorReload");
          MsgCode=MessageW(0,3,UMSG(MEditTitle),
                TruncPathStrW(strMsgFullFileName,ScrX-16),
                UMSG(MAskReload),
                UMSG(MCurrent),UMSG(MNewOpen),UMSG(MReload));
        }
        else
        {
          MsgCode=(OpenModeExstFile==FEOPMODE_USEEXISTING)?0:
                        (OpenModeExstFile==FEOPMODE_NEWIFOPEN?1:-100);
        }
        switch(MsgCode)
        {
          case 0:         // Current
            SwitchTo=TRUE;
            FrameManager->DeleteFrame(this); //???
            break;
          case 1:         // NewOpen
            SwitchTo=FALSE;
            break;
          case 2:         // Reload
            FrameManager->DeleteFrame(FramePos);
            SetExitCode(-2);
            break;
          case -100:
            //FrameManager->DeleteFrame(this);  //???
            SetExitCode(XC_EXISTS);
            return;
          default:
            FrameManager->DeleteFrame(this);  //???
            SetExitCode(MsgCode == -100?XC_EXISTS:XC_QUIT);
            return;
        }
      }
      else
      {
        SwitchTo=TRUE;
      }
      if (SwitchTo)
      {
        FrameManager->ActivateFrame(FramePos);
        //FrameManager->PluginCommit();
        SetExitCode((OpenModeExstFile != FEOPMODE_QUERY)?XC_EXISTS:TRUE);
        return ;
      }
    }
  }
  /* 11.05.2001 OT $*/

  /* $ 29.11.2000 SVS
     ���� ���� ����� ������� ReadOnly ��� System ��� Hidden,
     � �������� �� ������ ���������, �� ������� �������.
  */
  /* $ 03.12.2000 SVS
     System ��� Hidden - �������� ��������
  */
  /* $ 15.12.2000 SVS
    - Shift-F4, ����� ����. ������ ��������� :-(
  */
  DWORD FAttr=::GetFileAttributesW(Name);
  /* $ 05.06.2001 IS
     + �������� �������� ����, ��� �������� ��������������� �������
  */
  if(FAttr!=-1 && FAttr&FILE_ATTRIBUTE_DIRECTORY)
  {
    MessageW(MSG_WARNING,1,UMSG(MEditTitle),UMSG(MEditCanNotEditDirectory),UMSG(MOk));
    ExitCode=XC_OPEN_ERROR;
    return;
  }
  /* IS $ */
  if((FEdit->EdOpt.ReadOnlyLock&2) &&
     FAttr != -1 &&
     (FAttr &
        (FILE_ATTRIBUTE_READONLY|
           /* Hidden=0x2 System=0x4 - ������������� �� 2-� ���������,
              ������� ��������� ����� 0110.0000 �
              �������� �� ���� ����� => 0000.0110 � ��������
              �� ����� ������ ��������  */
           ((FEdit->EdOpt.ReadOnlyLock&0x60)>>4)
        )
     )
  )
  /* SVS $ */
  {
    _ECTLLOG(SysLog("Message: %s",MSG(MEditROOpen)));

    if(MessageW(MSG_WARNING,2,UMSG(MEditTitle),Name,UMSG(MEditRSH),
                             UMSG(MEditROOpen),UMSG(MYes),UMSG(MNo)))
    {
      //SetLastError(ERROR_ACCESS_DENIED);
      ExitCode=XC_OPEN_ERROR;
      return;
    }
  }
  /* SVS 03.12.2000 $ */
  /* SVS $ */

  FEdit->SetPosition(X1,Y1,X2,Y2-1);
  FEdit->SetStartPos(StartLine,StartChar);
  SetDeleteOnClose(DeleteOnClose);
  int UserBreak;
  /* $ 06.07.2001 IS
     ��� �������� ����� � ���� ��� �� �������� �������� ������� EE_READ, ����
     �� �������� �����������.
  */
  if(FAttr == -1)
    Flags.Set(FFILEEDIT_NEW);

  Flags.Change(FFILEEDIT_ISNEWFILE,CreateNewFile);

  if (!ReadFile(strFullFileName,UserBreak))
  {
    if(BlankFileName)
      UserBreak=0;
    if(!CreateNewFile || UserBreak)
    {
      if (UserBreak!=1)
      {
        SetLastError(SysErrorCode);
        MessageW(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MEditTitle),UMSG(MEditCannotOpen),strFileName,UMSG(MOk));
        ExitCode=XC_OPEN_ERROR;
      }
      else
      {
        ExitCode=XC_LOADING_INTERRUPTED;
      }
      //FrameManager->DeleteFrame(this); // BugZ#546 - Editor ����� ���!
      //CtrlObject->Cp()->Redraw(); //AY: ����� ��� �� ����, ������ ��������
                                    //    � ����������� ���� � ��������� �� �������
                                    //    ���������� ������� �������������� ����

      return;
    }

    if (FEdit->EdOpt.AnsiTableForNewFile)
    {
      int UseUnicode=FALSE;
      FEdit->AnsiText=TRUE;
      FEdit->TableNum=0;
      GetTable(&FEdit->TableSet,TRUE,FEdit->TableNum,UseUnicode);
      FEdit->UseDecodeTable=TRUE;
    }
    else
    {
      FEdit->AnsiText=FALSE;
      FEdit->TableNum=0;
      FEdit->UseDecodeTable=FALSE;
    }

  }

  CtrlObject->Plugins.CurEditor=this;//&FEdit;
  _ECTLLOG(SysLog("call ProcessEditorEvent(EE_READ,NULL) {"));
  CtrlObject->Plugins.ProcessEditorEvent(EE_READ,NULL);
  _ECTLLOG(SysLog("} return From ProcessEditorEvent(EE_READ,NULL)"));

  /* IS $ */
  ShowConsoleTitle();
  EditKeyBar.SetOwner(this);
  EditKeyBar.SetPosition(X1,Y2,X2,Y2);

  /* $ 07.08.2000 SVS
    ! ���, �������� KeyBar ������� � ��������� ������� */
  InitKeyBar();
  /* SVS $*/

  if ( Opt.EdOpt.ShowKeyBar==0 )
    EditKeyBar.Hide0();

  MacroMode=MACRO_EDITOR;
  CtrlObject->Macro.SetMode(MACRO_EDITOR);
/*& OT */
  if (EnableSwitch)
  {
    FrameManager->InsertFrame(this);
    //FrameManager->PluginCommit(); // ����! ����� ������ ������ �� ��������
  }
  else
  {
    FrameManager->ExecuteFrame(this);
  }
/* OT &*/

}

/* $ 07.08.2000 SVS
  ������� ������������� KeyBar Labels
*/
// $ 29.06.2000 tran - ������� �������� ���� �������������� ������
void FileEditor::InitKeyBar(void)
{
  int IKeyLabel[2][7][13]=
  {
    // ������� ��������
    {
      /* (empty)   */ {KBL_MAIN,MEditF1,MEditF2,MEditF3,MEditF4,MEditF5,MEditF6,MEditF7,MEditF8,MEditF9,MEditF10,MEditF11,MEditF12},
      /* Shift     */ {KBL_SHIFT,MEditShiftF1,MEditShiftF2,MEditShiftF3,MEditShiftF4,MEditShiftF5,MEditShiftF6,MEditShiftF7,MEditShiftF8,MEditShiftF9,MEditShiftF10,MEditShiftF11,MEditShiftF12},
      /* Alt       */ {KBL_ALT,MEditAltF1,MEditAltF2,MEditAltF3,MEditAltF4,MEditAltF5,MEditAltF6,MEditAltF7,MEditAltF8,MEditAltF9,MEditAltF10,MEditAltF11,MEditAltF12},
      /* Ctrl      */ {KBL_CTRL,MEditCtrlF1,MEditCtrlF2,MEditCtrlF3,MEditCtrlF4,MEditCtrlF5,MEditCtrlF6,MEditCtrlF7,MEditCtrlF8,MEditCtrlF9,MEditCtrlF10,MEditCtrlF11,MEditCtrlF12},
      /* AltShift  */ {KBL_ALTSHIFT,MEditAltShiftF1,MEditAltShiftF2,MEditAltShiftF3,MEditAltShiftF4,MEditAltShiftF5,MEditAltShiftF6,MEditAltShiftF7,MEditAltShiftF8,MEditAltShiftF9,MEditAltShiftF10,MEditAltShiftF11,MEditAltShiftF12},
      /* CtrlShift */ {KBL_CTRLSHIFT,MEditCtrlShiftF1,MEditCtrlShiftF2,MEditCtrlShiftF3,MEditCtrlShiftF4,MEditCtrlShiftF5,MEditCtrlShiftF6,MEditCtrlShiftF7,MEditCtrlShiftF8,MEditCtrlShiftF9,MEditCtrlShiftF10,MEditCtrlShiftF11,MEditCtrlShiftF12},
      /* CtrlAlt   */ {KBL_CTRLALT,MEditCtrlAltF1,MEditCtrlAltF2,MEditCtrlAltF3,MEditCtrlAltF4,MEditCtrlAltF5,MEditCtrlAltF6,MEditCtrlAltF7,MEditCtrlAltF8,MEditCtrlAltF9,MEditCtrlAltF10,MEditCtrlAltF11,MEditCtrlAltF12},
    },
    // ��������� ��������
    {
      /* (empty)   */ {KBL_MAIN,MSingleEditF1,MSingleEditF2,MSingleEditF3,MSingleEditF4,MSingleEditF5,MSingleEditF6,MSingleEditF7,MSingleEditF8,MSingleEditF9,MSingleEditF10,MSingleEditF11,MSingleEditF12},
      /* Shift     */ {KBL_SHIFT,MSingleEditShiftF1,MSingleEditShiftF2,MSingleEditShiftF3,MSingleEditShiftF4,MSingleEditShiftF5,MSingleEditShiftF6,MSingleEditShiftF7,MSingleEditShiftF8,MSingleEditShiftF9,MSingleEditShiftF10,MSingleEditShiftF11,MSingleEditShiftF12},
      /* Alt       */ {KBL_ALT,MSingleEditAltF1,MSingleEditAltF2,MSingleEditAltF3,MSingleEditAltF4,MSingleEditAltF5,MSingleEditAltF6,MSingleEditAltF7,MSingleEditAltF8,MSingleEditAltF9,MSingleEditAltF10,MSingleEditAltF11,MSingleEditAltF12},
      /* Ctrl      */ {KBL_CTRL,MSingleEditCtrlF1,MSingleEditCtrlF2,MSingleEditCtrlF3,MSingleEditCtrlF4,MSingleEditCtrlF5,MSingleEditCtrlF6,MSingleEditCtrlF7,MSingleEditCtrlF8,MSingleEditCtrlF9,MSingleEditCtrlF10,MSingleEditCtrlF11,MSingleEditCtrlF12},
      /* AltShift  */ {KBL_ALTSHIFT,MSingleEditAltShiftF1,MSingleEditAltShiftF2,MSingleEditAltShiftF3,MSingleEditAltShiftF4,MSingleEditAltShiftF5,MSingleEditAltShiftF6,MSingleEditAltShiftF7,MSingleEditAltShiftF8,MSingleEditAltShiftF9,MSingleEditAltShiftF10,MSingleEditAltShiftF11,MSingleEditAltShiftF12},
      /* CtrlShift */ {KBL_CTRLSHIFT,MSingleEditCtrlShiftF1,MSingleEditCtrlShiftF2,MSingleEditCtrlShiftF3,MSingleEditCtrlShiftF4,MSingleEditCtrlShiftF5,MSingleEditCtrlShiftF6,MSingleEditCtrlShiftF7,MSingleEditCtrlShiftF8,MSingleEditCtrlShiftF9,MSingleEditCtrlShiftF10,MSingleEditCtrlShiftF11,MSingleEditCtrlShiftF12},
      /* CtrlAlt   */ {KBL_CTRLALT,MSingleEditCtrlAltF1,MSingleEditCtrlAltF2,MSingleEditCtrlAltF3,MSingleEditCtrlAltF4,MSingleEditCtrlAltF5,MSingleEditCtrlAltF6,MSingleEditCtrlAltF7,MSingleEditCtrlAltF8,MSingleEditCtrlAltF9,MSingleEditCtrlAltF10,MSingleEditCtrlAltF11,MSingleEditCtrlAltF12},
    }
  };
  wchar_t *FEditKeys[12];
  int I,J;

  for(I=0; I < 7; ++I)
  {
    for(J=1; J <= 12; ++J)
    {
      FEditKeys[J-1]=UMSG(IKeyLabel[Opt.OnlyEditorViewerUsed][I][J]);
    }
    switch(IKeyLabel[Opt.OnlyEditorViewerUsed][I][0])
    {
      case KBL_SHIFT:
        if(!GetCanLoseFocus())
          FEditKeys[4-1]=L"";
        break;
      case KBL_MAIN:
        if(Flags.Check(FFILEEDIT_SAVETOSAVEAS))
          FEditKeys[2-1]=UMSG(MEditShiftF2);
        // $ 10.05.2001 DJ - ������� �� EnableF6 ������ CanLoseFocus
        if(!Flags.Check(FFILEEDIT_ENABLEF6))
          FEditKeys[6-1]=L"";
        if(!GetCanLoseFocus())
          FEditKeys[12-1]=L"";
        break;
      case KBL_ALT:
        // $ 17.12.2001 KM  - ���� !GetCanLoseFocus() ����� �� Alt-F11 ������ ������ ������.
        if(!GetCanLoseFocus())
          FEditKeys[11-1]=L"";
        if(!Opt.UsePrintManager || CtrlObject->Plugins.FindPlugin(SYSID_PRINTMANAGER) == -1)
          FEditKeys[5-1]=L"";
        break;
    }
    EditKeyBar.SetGroup(IKeyLabel[Opt.OnlyEditorViewerUsed][I][0],FEditKeys,sizeof(FEditKeys)/sizeof(FEditKeys[0]));
  }

  if (FEdit->AnsiText)
    EditKeyBar.Change(UMSG(Opt.OnlyEditorViewerUsed?MSingleEditF8DOS:MEditF8DOS),7);
  else
    EditKeyBar.Change(UMSG(Opt.OnlyEditorViewerUsed?MSingleEditF8:MEditF8),7);

  EditKeyBar.Show();
  FEdit->SetPosition(X1,Y1,X2,Y2-(Opt.EdOpt.ShowKeyBar?1:0));
  SetKeyBar(&EditKeyBar);
}
/* SVS $ */

void FileEditor::SetNamesList (NamesList *Names)
{
  if (EditNamesList == NULL)
    EditNamesList = new NamesList;
  Names->MoveData (*EditNamesList);
}

/* DJ $ */

void FileEditor::Show()
{
  if (Flags.Check(FFILEEDIT_FULLSCREEN))
  {
    if ( Opt.EdOpt.ShowKeyBar )
    {
       EditKeyBar.SetPosition(0,ScrY,ScrX,ScrY);
       EditKeyBar.Redraw();
    }
    ScreenObject::SetPosition(0,0,ScrX,ScrY-(Opt.EdOpt.ShowKeyBar?1:0));
    FEdit->SetPosition(0,0,ScrX,ScrY-(Opt.EdOpt.ShowKeyBar?1:0));
  }
  ScreenObject::Show();
}


void FileEditor::DisplayObject()
{
  if ( !FEdit->Locked() )
  {
    if(FEdit->Flags.Check(FEDITOR_ISRESIZEDCONSOLE))
    {
      FEdit->Flags.Clear(FEDITOR_ISRESIZEDCONSOLE);
      CtrlObject->Plugins.CurEditor=this;
      CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_CHANGE);//EEREDRAW_ALL);
    }
    FEdit->Show();
  }
}


int FileEditor::ProcessKey(int Key)
{
  return ReProcessKey(Key,FALSE);
}

int FileEditor::ReProcessKey(int Key,int CalledFromControl)
{
  DWORD FNAttr;

  _KEYMACRO(CleverSysLog SL("FileEditor::ProcessKey()"));
  _KEYMACRO(SysLog("Key=%s Macro.IsExecuting()=%d",_FARKEY_ToName(Key),CtrlObject->Macro.IsExecuting()));

  if (Flags.Check(FFILEEDIT_REDRAWTITLE) && ((Key & 0x00ffffff) < KEY_END_FKEY))
    ShowConsoleTitle();

  // BugZ#488 - Shift=enter
  if(ShiftPressed && Key == KEY_ENTER && CtrlObject->Macro.IsExecuting() == MACROMODE_NOMACRO)
  {
    Key=KEY_SHIFTENTER;
  }

  // ��� ��������� �������������� ������� ������ �����
  /* $ 28.04.2001 DJ
     �� �������� KEY_MACRO* ������� - ��������� ReadRec � ���� ������
     ����� �� ������������� �������������� �������, ��������� ������������
     �����
  */
  if(Key >= KEY_MACRO_BASE && Key <= KEY_MACRO_ENDBASE) // ��������� MACRO
  {
    if(Key == MCODE_V_EDITORSTATE)
    {
      DWORD MacroEditState=0;
      MacroEditState|=Flags.Flags&FFILEEDIT_NEW?0x00000001:0;
      MacroEditState|=Flags.Flags&FFILEEDIT_ENABLEF6?0x00000002:0;
      MacroEditState|=FEdit->Flags.Flags&FEDITOR_DELETEONCLOSE?0x00000004:0;
      MacroEditState|=FEdit->Flags.Flags&FEDITOR_MODIFIED?0x00000008:0;
      MacroEditState|=FEdit->BlockStart?0x00000010:0;
      MacroEditState|=FEdit->VBlockStart?0x00000020:0;
      MacroEditState|=FEdit->Flags.Flags&FEDITOR_WASCHANGED?0x00000040:0;
      MacroEditState|=FEdit->Flags.Flags&FEDITOR_OVERTYPE?0x00000080:0;
      MacroEditState|=FEdit->Flags.Flags&FEDITOR_CURPOSCHANGEDBYPLUGIN?0x00000100:0;
      MacroEditState|=FEdit->Flags.Flags&FEDITOR_LOCKMODE?0x00000200:0;
      MacroEditState|=FEdit->EdOpt.PersistentBlocks?0x00000400:0;
      return MacroEditState;
    }

    if(Key == MCODE_V_EDITORCURPOS)
      return FEdit->CurLine->EditLine.GetTabCurPos()+1;
    if(Key == MCODE_V_EDITORCURLINE)
      return FEdit->NumLine+1;
    if(Key == MCODE_V_ITEMCOUNT || Key == MCODE_V_EDITORLINES)
      return FEdit->NumLastLine;

    return(FEdit->ProcessKey(Key));
  }
  /* DJ $ */

  switch(Key)
  {
    /* $ 27.09.2000 SVS
       ������ �����/����� � �������������� ������� PrintMan
    */
    case KEY_ALTF5:
    {
      if(Opt.UsePrintManager && CtrlObject->Plugins.FindPlugin(SYSID_PRINTMANAGER) != -1)
      {
        CtrlObject->Plugins.CallPlugin(SYSID_PRINTMANAGER,OPEN_EDITOR,0); // printman
        return TRUE;
      }
      break; // ������� Alt-F5 �� ����������� ��������, ���� �� ���������� PrintMan
    }
    /* SVS $*/

    case KEY_F6:
    {
      /* $ 10.05.2001 DJ
         ���������� EnableF6
      */
      if (Flags.Check(FFILEEDIT_ENABLEF6))
      {
        int FirstSave=1, NeedQuestion=1;
        // �������� �� "� ����� ��� ����� ������� ���?"
        // �������� ����� ��� � �� �����!
        // ����, ��� �� ���� ��������, ��
        if(FEdit->IsFileChanged() &&  // � ������� ������ ���� ���������?
           ::GetFileAttributesW(strFullFileName) == -1) // � ���� ��� ����������?
        {
          switch(MessageW(MSG_WARNING,2,UMSG(MEditTitle),
                         UMSG(MEditSavedChangedNonFile),
                         UMSG(MEditSavedChangedNonFile2),
                         UMSG(MEditSave),UMSG(MCancel)))
          {
            case 0:
              if(ProcessKey(KEY_F2))
              {
                FirstSave=0;
                break;
              }
            default:
              return FALSE;
          }
        }

        if(!FirstSave || FEdit->IsFileChanged() || ::GetFileAttributesW (strFullFileName)!=-1)
        {
          long FilePos=FEdit->GetCurPos();
          /* $ 01.02.2001 IS
             ! ��������� ����� � ��������� �������� ����� �����, � �� ���������
          */
          if (ProcessQuitKey(FirstSave,NeedQuestion))
          {
            /* $ 11.10.200 IS
               �� ����� ������� ����, ���� ���� �������� ��������, �� ��� ����
               ������������ ������������ �� �����
            */
            SetDeleteOnClose(0);
            /* IS $ */
            /* $ 06.05.2001 DJ
               ��������� F6 ��� NWZ
            */
            /* $ 07.05.2001 DJ
               ��������� NamesList
            */


            FileViewer *Viewer = new FileViewer (strFullFileName, GetCanLoseFocus(), FALSE,
               FALSE, FilePos, NULL, EditNamesList, Flags.Check(FFILEEDIT_SAVETOSAVEAS));
            /* DJ $ */
  //OT          FrameManager->InsertFrame (Viewer);
            /* DJ $ */
          }
          /* IS $ */
          ShowTime(2);
        }
        return(TRUE);
      }
      break; // ������� F6 ��������, ���� ���� ������ �� ������������
      /* DJ $ */
    }

    /* $ 10.05.2001 DJ
       Alt-F11 - �������� view/edit history
    */
    case KEY_ALTF11:
    {
      if (GetCanLoseFocus())
      {
        CtrlObject->CmdLine->ShowViewEditHistory();
        return TRUE;
      }
      break; // ������� Alt-F11 �� ����������� ��������, ���� �������� ���������
    }
    /* DJ $ */
  }

#if 1
  BOOL ProcessedNext=TRUE;

  _SVS(if(Key=='n' || Key=='m'))
    _SVS(SysLog("%d Key='%c'",__LINE__,Key));

  if(!CalledFromControl && (CtrlObject->Macro.IsRecording() == MACROMODE_RECORDING_COMMON || CtrlObject->Macro.IsExecuting() == MACROMODE_EXECUTING_COMMON || CtrlObject->Macro.GetCurRecord(NULL,NULL) == MACROMODE_NOMACRO))
  {
    _SVS(if(CtrlObject->Macro.IsRecording() == MACROMODE_RECORDING_COMMON || CtrlObject->Macro.IsExecuting() == MACROMODE_EXECUTING_COMMON))
      _SVS(SysLog("%d !!!! CtrlObject->Macro.GetCurRecord(NULL,NULL) != MACROMODE_NOMACRO !!!!",__LINE__));
    ProcessedNext=!ProcessEditorInput(FrameManager->GetLastInputRecord());
  }

  if (ProcessedNext)
#else
  if (!CalledFromControl && //CtrlObject->Macro.IsExecuting() || CtrlObject->Macro.IsRecording() || // ����� �������!
    !ProcessEditorInput(FrameManager->GetLastInputRecord()))
#endif
  {
    _KEYMACRO(SysLog("if (ProcessedNext) => __LINE__=%d",__LINE__));
    switch(Key)
    {
      case KEY_F1:
      {
        Help Hlp (L"Editor");
        return(TRUE);
      }

      /* $ 25.04.2001 IS
           ctrl+f - �������� � ������ ������ ��� �������������� �����
      */
      case KEY_CTRLF:
      {
        if (!FEdit->Flags.Check(FEDITOR_LOCKMODE))
        {
          FEdit->Pasting++;
          FEdit->TextChanged(1);
          BOOL IsBlock=FEdit->VBlockStart || FEdit->BlockStart;
          if (!FEdit->EdOpt.PersistentBlocks && IsBlock)
          {
            FEdit->Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
            FEdit->DeleteBlock();
          }
          //AddUndoData(CurLine->EditLine.GetStringAddr(),NumLine,
          //                CurLine->EditLine.GetCurPos(),UNDO_EDIT);
          FEdit->Paste(strFullFileName); //???
          //if (!EdOpt.PersistentBlocks)
          FEdit->UnmarkBlock();
          FEdit->Pasting--;
          FEdit->Show(); //???
        }
        return (TRUE);
      }
      /* IS $ */
      /* $ 24.08.2000 SVS
         + ��������� ������� ������ ���������� �� ������� CtrlAltShift
      */
      case KEY_CTRLO:
      {
        if(!Opt.OnlyEditorViewerUsed)
        {
          FEdit->Hide();  // $ 27.09.2000 skv - To prevent redraw in macro with Ctrl-O
          if(FrameManager->ShowBackground())
          {
            SetCursorType(FALSE,0);
            WaitKey();
          }
          Show();
        }
        return(TRUE);
      }
  /* $ KEY_CTRLALTSHIFTPRESS ������� � manager OT */

      case KEY_F2:
      case KEY_SHIFTF2:
      {
        BOOL Done=FALSE;

        string strOldCurDir;
        FarGetCurDirW (strOldCurDir);

        wchar_t *lpwszPtr;
        wchar_t wChr;

        while(!Done) // ������ �� �����
        {
          // �������� ���� � �����, ����� ��� ��� ������...
          lpwszPtr = strFullFileName.GetBuffer ();

          lpwszPtr=wcsrchr(lpwszPtr,L'\\');
          if(lpwszPtr)
          {
            wChr=*lpwszPtr;
            *lpwszPtr=0;
            // � �����?
            if (!(LocalIsalphaW(strFullFileName.At(0)) && (strFullFileName.At(1)==L':') && !strFullFileName.At(2)))
            {
              // � ������? ������� ����������?
              if((FNAttr=::GetFileAttributesW(strFullFileName)) == -1 ||
                                !(FNAttr&FILE_ATTRIBUTE_DIRECTORY)
                  //|| LocalStricmp(OldCurDir,FullFileName)  // <- ��� ������ ������.
                )
                Flags.Set(FFILEEDIT_SAVETOSAVEAS);
            }
            *lpwszPtr=wChr;
          }
          strFullFileName.ReleaseBuffer ();


          if(Key == KEY_F2 &&
             (FNAttr=::GetFileAttributesW(strFullFileName)) != -1 &&
             !(FNAttr&FILE_ATTRIBUTE_DIRECTORY))
              Flags.Clear(FFILEEDIT_SAVETOSAVEAS);

          static int TextFormat=0;
          int NameChanged=FALSE;
          if (Key==KEY_SHIFTF2 || Flags.Check(FFILEEDIT_SAVETOSAVEAS))
          {
            const wchar_t *HistoryName=L"NewEdit";
            static struct DialogDataEx EditDlgData[]=
            {
              /* 0 */ DI_DOUBLEBOX,3,1,72,12,0,0,0,0,(const wchar_t *)MEditTitle,
              /* 1 */ DI_TEXT,5,2,0,0,0,0,0,0,(const wchar_t *)MEditSaveAs,
              /* 2 */ DI_EDIT,5,3,70,3,1,(DWORD)HistoryName,DIF_HISTORY/*|DIF_EDITPATH*/,0,L"",
              /* 3 */ DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
              /* 4 */ DI_TEXT,5,5,0,0,0,0,0,0,(const wchar_t *)MEditSaveAsFormatTitle,
              /* 5 */ DI_RADIOBUTTON,5,6,0,0,0,0,DIF_GROUP,0,(const wchar_t *)MEditSaveOriginal,
              /* 6 */ DI_RADIOBUTTON,5,7,0,0,0,0,0,0,(const wchar_t *)MEditSaveDOS,
              /* 7 */ DI_RADIOBUTTON,5,8,0,0,0,0,0,0,(const wchar_t *)MEditSaveUnix,
              /* 8 */ DI_RADIOBUTTON,5,9,0,0,0,0,0,0,(const wchar_t *)MEditSaveMac,
              /* 9 */ DI_TEXT,3,10,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
              /*10 */ DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
              /*11 */ DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel,
            };
            MakeDialogItemsEx(EditDlgData,EditDlg);

            EditDlg[2].strData = (Flags.Check(FFILEEDIT_SAVETOSAVEAS)?strFullFileName:strFileName);

            //xstrncpy(EditDlg[2].Data,(Flags.Check(FFILEEDIT_SAVETOSAVEAS)?FullFileName:FileName),sizeof(EditDlg[2].Data)-1);
            wchar_t *PtrEditDlgData=EditDlg[2].strData.GetBuffer ();

            PtrEditDlgData = wcsstr (PtrEditDlgData, UMSG(MNewFileName));
            if(PtrEditDlgData)
              *PtrEditDlgData=0;

            EditDlg[2].strData.ReleaseBuffer();

            EditDlg[5].Selected=EditDlg[6].Selected=EditDlg[7].Selected=EditDlg[8].Selected=0;
            EditDlg[5+TextFormat].Selected=TRUE;

            {
              Dialog Dlg(EditDlg,sizeof(EditDlg)/sizeof(EditDlg[0]));
              Dlg.SetPosition(-1,-1,76,14);
              Dlg.SetHelp(L"FileSaveAs");
              Dlg.Process();
              if (Dlg.GetExitCode()!=10 || EditDlg[2].strData.IsEmpty() )
                return(FALSE);
            }
            apiExpandEnvironmentStrings (EditDlg[2].strData,EditDlg[2].strData);
            /* $ 07.06.2001 IS
               - ���: ����� ������� ������� �������, � ������ ����� �������
            */
            RemoveTrailingSpacesW(EditDlg[2].strData);
            UnquoteW(EditDlg[2].strData);
            /* IS $ */

            string strData = EditDlg[2].strData;

            NameChanged=LocalStricmpW(strData,(Flags.Check(FFILEEDIT_SAVETOSAVEAS)?strFullFileName:strFileName))!=0;
            /* $ 01.08.2001 tran
               ���� ����� ��������� ������ � ������ FileName
               ����������� EditDlg[2].Data */
            if( !NameChanged )
              FarChDirW (strStartDir); // ������? � ����� ��???

            FNAttr=::GetFileAttributesW(strData);
            if (NameChanged && FNAttr != -1)
            {
              if (MessageW(MSG_WARNING,2,UMSG(MEditTitle),strData,UMSG(MEditExists),
                           UMSG(MEditOvr),UMSG(MYes),UMSG(MNo))!=0)
              {
                FarChDirW(strOldCurDir);
                return(TRUE);
              }
              Flags.Set(FFILEEDIT_SAVEWQUESTIONS);
            }
            /* tran $ */

            string strFileNameTemp = strData;

            if(!SetFileName(strFileNameTemp))
            {
              SetLastError(ERROR_INVALID_NAME);
              MessageW(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MEditTitle),strFileNameTemp,UMSG(MOk));
              if(!NameChanged)
                FarChDirW(strOldCurDir);
              continue;
              //return FALSE;
            }

            if (EditDlg[5].Selected)
              TextFormat=0;
            if (EditDlg[6].Selected)
              TextFormat=1;
            if (EditDlg[7].Selected)
              TextFormat=2;
            if (EditDlg[8].Selected)
              TextFormat=3;
            if(!NameChanged)
              FarChDirW(strOldCurDir);
          }
          ShowConsoleTitle();

          FarChDirW (strStartDir); //???

          if(SaveFile(strFullFileName,0,Key==KEY_SHIFTF2 ? TextFormat:0,Key==KEY_SHIFTF2) == SAVEFILE_ERROR)
          {
            SetLastError(SysErrorCode);
            if (MessageW(MSG_WARNING|MSG_ERRORTYPE,2,UMSG(MEditTitle),UMSG(MEditCannotSave),
                        strFileName,UMSG(MRetry),UMSG(MCancel))!=0)
            {
              Done=TRUE;
              break;
            }
          }
          else
            Done=TRUE;
        }
        return(TRUE);
      }

#if 1
      /* $ 30.05.2003 SVS
         ���� :-) Shift-F4 � ���������/������� ��������� ��������� ������ ��������/������
         ���� �����������
      */
      case KEY_SHIFTF4:
      {
        if(!Opt.OnlyEditorViewerUsed && GetCanLoseFocus())
          CtrlObject->Cp()->ActivePanel->ProcessKey(Key);
        return TRUE;
      }
      /* $ SVS */
#endif

      /*$ 21.07.2000 SKV
          + ����� � ����������������� �� ������������� ����� �� CTRLF10
      */
      case KEY_CTRLF10:
      {
        {
          if (isTemporary())
          {
            return(TRUE);
          }

          string strFullFileNameTemp = strFullFileName;
          /* 26.11.2001 VVM
            ! ������������ ������ ��� ����� */
          /* $ 28.12.2001 DJ
             ������� ��� � ����� �������
          */
          if(::GetFileAttributesW(strFullFileName) == -1) // � ��� ���� �� ��� �� �����?
          {
              if(!CheckShortcutFolderW(&strFullFileNameTemp,-1,FALSE))
              return FALSE;
            strFullFileNameTemp += L"\\."; // ��� ���������� ������ :-)
          }

          if(Flags.Check(FFILEEDIT_NEW))
          {
            UpdateFileList();
            Flags.Clear(FFILEEDIT_NEW);
          }

          {
            SaveScreen Sc;
            CtrlObject->Cp()->GoToFileW (strFullFileNameTemp);
            Flags.Set(FFILEEDIT_REDRAWTITLE);
          }
          /* DJ $ */
          /* VVM $ */
        }
        return (TRUE);
      }
      /* SKV $*/

      case KEY_CTRLB:
      {
        Opt.EdOpt.ShowKeyBar=!Opt.EdOpt.ShowKeyBar;
        if ( Opt.EdOpt.ShowKeyBar )
          EditKeyBar.Show();
        else
          EditKeyBar.Hide0(); // 0 mean - Don't purge saved screen
        Show();
        KeyBarVisible = Opt.EdOpt.ShowKeyBar;
        return (TRUE);
      }

      case KEY_SHIFTF10:
        if(!ProcessKey(KEY_F2)) // ����� ���� ����, ��� ����� ���������� �� ����������
          return FALSE;
      case KEY_ESC:
      case KEY_F10:
      {
        int FirstSave=1, NeedQuestion=1;
        if(Key != KEY_SHIFTF10)    // KEY_SHIFTF10 �� ���������!
        {
          int FilePlased=::GetFileAttributesW (strFullFileName) == -1 && !Flags.Check(FFILEEDIT_ISNEWFILE);
          if(FEdit->IsFileChanged() ||  // � ������� ������ ���� ���������?
             FilePlased) // � ��� ���� �� ��� �� �����?
          {
            int Res;
            if(FEdit->IsFileChanged() && FilePlased)
                Res=MessageW(MSG_WARNING,3,UMSG(MEditTitle),
                           UMSG(MEditSavedChangedNonFile),
                           UMSG(MEditSavedChangedNonFile2),
                           UMSG(MEditSave),UMSG(MEditNotSave),UMSG(MEditContinue));
            else if(!FEdit->IsFileChanged() && FilePlased)
                Res=MessageW(MSG_WARNING,3,UMSG(MEditTitle),
                           UMSG(MEditSavedChangedNonFile1),
                           UMSG(MEditSavedChangedNonFile2),
                           UMSG(MEditSave),UMSG(MEditNotSave),UMSG(MEditContinue));
            else
               Res=100;
            switch(Res)
            {
              case 0:
                if(!ProcessKey(KEY_F2))  // ������� ������� ���������
                  NeedQuestion=0;
                FirstSave=0;
                break;
              case 1:
                NeedQuestion=0;
                FirstSave=0;
                break;
              case 100:
                FirstSave=NeedQuestion=1;
                break;
              case 2:
              default:
                return FALSE;
            }
          }
          else if(!FEdit->Flags.Check(FEDITOR_MODIFIED)) //????
            NeedQuestion=0;

        }
        if(!ProcessQuitKey(FirstSave,NeedQuestion))
          return FALSE;
        return(TRUE);
      }

      /* $ 19.12.2000 SVS
         ����� ������� �������� (� ������ IS)
      */
      case KEY_ALTSHIFTF9:
      {
        /* $ 26.02.2001 IS
             ������ � ��������� ������ EditorOptions
        */
        struct EditorOptions EdOpt;
        GetEditorOptions(EdOpt);

        EditorConfig(EdOpt,1); // $ 27.11.2001 DJ - Local � EditorConfig
        EditKeyBar.Show(); //???? ����� ��????

        SetEditorOptions(EdOpt);

        /* IS $ */
        if ( Opt.EdOpt.ShowKeyBar )
          EditKeyBar.Show();

        FEdit->Show();
        return TRUE;
      }
      /* SVS $ */

      default:
      {
        _KEYMACRO(SysLog("default: __LINE__=%d",__LINE__));
        /* $ 22.03.2001 SVS
           ��� ������� �� ��������� :-)
        */
        if (Flags.Check(FFILEEDIT_FULLSCREEN) && CtrlObject->Macro.IsExecuting() == MACROMODE_NOMACRO)
          if ( Opt.EdOpt.ShowKeyBar )
            EditKeyBar.Show();
        /* SVS $ */
        if (!EditKeyBar.ProcessKey(Key))
          return(FEdit->ProcessKey(Key));
      }
    }
  }
  return(TRUE);
}


int FileEditor::ProcessQuitKey(int FirstSave,BOOL NeedQuestion)
{
  string strOldCurDir;
  FarGetCurDirW (strOldCurDir);
  while (1)
  {
    FarChDirW(strStartDir); // ������? � ����� ��???
    int SaveCode=SAVEFILE_SUCCESS;
    if(NeedQuestion)
    {
      SaveCode=SaveFile(strFullFileName,FirstSave,0,FALSE);
    }
    if (SaveCode==SAVEFILE_CANCEL)
      break;
    if (SaveCode==SAVEFILE_SUCCESS)
    {
      /* $ 09.02.2002 VVM
        + �������� ������, ���� ������ � ������� ������� */
      if (NeedQuestion)
      {
        UpdateFileList();
      }
      /* VVM $ */

      FrameManager->DeleteFrame();
      SetExitCode (XC_QUIT);
      break;
    }
    if(!wcscmp(strFileName,UMSG(MNewFileName)))
      if(!ProcessKey(KEY_SHIFTF2))
      {
        FarChDirW(strOldCurDir);
        return FALSE;
      }
      else
        break;
    SetLastError(SysErrorCode);
    if (MessageW(MSG_WARNING|MSG_ERRORTYPE,2,UMSG(MEditTitle),UMSG(MEditCannotSave),
              strFileName,UMSG(MRetry),UMSG(MCancel))!=0)
        break;
    FirstSave=0;
  }

  FarChDirW(strOldCurDir);
  return GetExitCode() == XC_QUIT;
}


// ���� ������ ���������� ��� �� Editor::ReadFile()
int FileEditor::ReadFile(const wchar_t *Name,int &UserBreak)
{
  int Ret=FEdit->ReadFile(Name,UserBreak);
  SysErrorCode=GetLastError();
  apiGetFindDataEx (Name,&FileInfo);
  return Ret;
}

// ���� ������ ���������� ��� �� Editor::SaveFile()
int FileEditor::SaveFile(const wchar_t *Name,int Ask,int TextFormat,int SaveAs)
{
  /* $ 11.10.2000 SVS
     �������������, ��������, ��� ������ - �������� ���� :-(
  */
  if (FEdit->Flags.Check(FEDITOR_LOCKMODE) && !FEdit->Flags.Check(FEDITOR_MODIFIED) && !SaveAs)
    return SAVEFILE_SUCCESS;
  /* SVS $ */

  if (Ask)
  {
    if(!FEdit->Flags.Check(FEDITOR_MODIFIED))
      return SAVEFILE_SUCCESS;

    if (Ask)
    {
      switch (MessageW(MSG_WARNING,3,UMSG(MEditTitle),UMSG(MEditAskSave),
              UMSG(MEditSave),UMSG(MEditNotSave),UMSG(MEditContinue)))
      {
        case -1:
        case -2:
        case 2:  // Continue Edit
          return SAVEFILE_CANCEL;
        case 0:  // Save
          break;
        case 1:  // Not Save
          FEdit->TextChanged(0); // 10.08.2000 skv: TextChanged() support;
          return SAVEFILE_SUCCESS;
      }
    }
  }

  int NewFile=TRUE;
  FileAttributesModified=false;
  if ((FileAttributes=::GetFileAttributesW(Name))!=-1)
  {
    // �������� ������� �����������...
    if(!Flags.Check(FFILEEDIT_SAVEWQUESTIONS))
    {
      FAR_FIND_DATA_EX FInfo;
      if( apiGetFindDataEx (Name,&FInfo) && *FileInfo.strFileName)
      {
        __int64 RetCompare=*(__int64*)&FileInfo.ftLastWriteTime - *(__int64*)&FInfo.ftLastWriteTime;
        if(RetCompare || !(FInfo.nFileSize == FileInfo.nFileSize))
        {
          SetMessageHelp(L"WarnEditorSavedEx");
          switch (MessageW(MSG_WARNING,3,UMSG(MEditTitle),UMSG(MEditAskSaveExt),
                  UMSG(MEditSave),UMSG(MEditBtnSaveAs),UMSG(MEditContinue)))
          {
            case -1:
            case -2:
            case 2:  // Continue Edit
              return SAVEFILE_CANCEL;
            case 1:  // Save as
              if(ProcessKey(KEY_SHIFTF2))
                return SAVEFILE_SUCCESS;
              else
                return SAVEFILE_CANCEL;
            case 0:  // Save
              break;
          }
        }
      }
    }
    Flags.Clear(FFILEEDIT_SAVEWQUESTIONS);

    NewFile=FALSE;
    if (FileAttributes & FA_RDONLY)
    {
        //BUGBUG
      int AskOverwrite=MessageW(MSG_WARNING,2,UMSG(MEditTitle),Name,UMSG(MEditRO),
                           UMSG(MEditOvr),UMSG(MYes),UMSG(MNo));
      if (AskOverwrite!=0)
        return SAVEFILE_CANCEL;

      SetFileAttributesW(Name,FileAttributes & ~FA_RDONLY); // ����� ��������
      FileAttributesModified=true;
    }

    if (FileAttributes & (FA_HIDDEN|FA_SYSTEM))
    {
      SetFileAttributesW(Name,FILE_ATTRIBUTE_NORMAL);
      FileAttributesModified=true;
    }
  }
  else
  {
    // �������� ���� � �����, ����� ��� ��� ������...
    string strCreatedPath = Name;

    const wchar_t *Ptr = wcsrchr (strCreatedPath, L'\\');

    if ( Ptr )
    {
      CutToSlashW (strCreatedPath, true);
      DWORD FAttr=0;
      if(::GetFileAttributesW(strCreatedPath) == -1)
      {
        // � ��������� �������.
        // ��� ��
        CreatePathW(strCreatedPath);
        FAttr=::GetFileAttributesW(strCreatedPath);
      }

      if(FAttr == -1)
        return SAVEFILE_ERROR;
    }
  }

  //int Ret=FEdit->SaveFile(Name,Ask,TextFormat,SaveAs);
  // ************************************
  /* $ 12.02.2001 IS
       ������� ��������� FileAttr �� FileAttributes
  */
  /* $ 04.06.2001 IS
       ������ (� ������ SVS) ������������� ���� - ����� �� ������� ��� �� ����,
       ��� ������������� �������� �����
  */
  int RetCode=SAVEFILE_SUCCESS;

  if (TextFormat!=0)
    FEdit->Flags.Set(FEDITOR_WASCHANGED);

  switch(TextFormat)
  {
    case 1:
      wcscpy(FEdit->GlobalEOL,DOS_EOL_fmtW);
      break;
    case 2:
      wcscpy(FEdit->GlobalEOL,UNIX_EOL_fmtW);
      break;
    case 3:
      wcscpy(FEdit->GlobalEOL,MAC_EOL_fmtW);
      break;
  }

  if(::GetFileAttributesW(Name) == -1)
    Flags.Set(FFILEEDIT_NEW);

  {
    FILE *EditFile;
    //SaveScreen SaveScr;
    /* $ 11.10.2001 IS
       ���� ���� ����������� ���������� � ����� �����������, �� �� ������� ����
    */
    FEdit->Flags.Clear(FEDITOR_DELETEONCLOSE|FEDITOR_DELETEONLYFILEONCLOSE);
    /* IS $ */
    CtrlObject->Plugins.CurEditor=this;
//_D(SysLog("%08d EE_SAVE",__LINE__));
    CtrlObject->Plugins.ProcessEditorEvent(EE_SAVE,NULL);

    DWORD FileFlags=FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN;
    if (FileAttributes!=-1 && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
      FileFlags|=FILE_FLAG_POSIX_SEMANTICS;

    HANDLE hEdit=FAR_CreateFileW(Name,GENERIC_WRITE,FILE_SHARE_READ,NULL,
                 FileAttributes!=-1 ? TRUNCATE_EXISTING:CREATE_ALWAYS,FileFlags,NULL);
    if (hEdit==INVALID_HANDLE_VALUE && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT && FileAttributes!=-1)
    {
      //_SVS(SysLogLastError();SysLog("Name='%s',FileAttributes=%d",Name,FileAttributes));
      hEdit=FAR_CreateFileW(Name,GENERIC_WRITE,FILE_SHARE_READ,NULL,TRUNCATE_EXISTING,
                       FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    }
    if (hEdit==INVALID_HANDLE_VALUE)
    {
      //_SVS(SysLogLastError();SysLog("Name='%s',FileAttributes=%d",Name,FileAttributes));
      RetCode=SAVEFILE_ERROR;
      SysErrorCode=GetLastError();
      goto end;
    }
    int EditHandle=_open_osfhandle((long)hEdit,O_BINARY);
    if (EditHandle==-1)
    {
      RetCode=SAVEFILE_ERROR;
      SysErrorCode=GetLastError();
      goto end;
    }
    if ((EditFile=fdopen(EditHandle,"wb"))==NULL)
    {
      RetCode=SAVEFILE_ERROR;
      SysErrorCode=GetLastError();
      goto end;
    }

    FEdit->UndoSavePos=FEdit->UndoDataPos;
    FEdit->Flags.Clear(FEDITOR_UNDOOVERFLOW);

//    ConvertNameToFull(Name,FileName, sizeof(FileName));
/*
    if (ConvertNameToFull(Name,FEdit->FileName, sizeof(FEdit->FileName)) >= sizeof(FEdit->FileName))
    {
      FEdit->Flags.Set(FEDITOR_OPENFAILED);
      RetCode=SAVEFILE_ERROR;
      goto end;
    }
*/
    SetCursorType(FALSE,0);
    SetPreRedrawFunc(Editor::PR_EditorShowMsg);
    Editor::EditorShowMsg(UMSG(MEditTitle),UMSG(MEditSaving),Name);

    struct EditList *CurPtr=FEdit->TopList;

    while (CurPtr!=NULL)
    {
      const wchar_t *SaveStr, *EndSeq;
      int Length;
      CurPtr->EditLine.GetBinaryStringW(SaveStr,&EndSeq,Length);
      if (*EndSeq==0 && CurPtr->Next!=NULL)
        EndSeq=*FEdit->GlobalEOL ? FEdit->GlobalEOL:DOS_EOL_fmtW;
      if (TextFormat!=0 && *EndSeq!=0)
      {
        if (TextFormat==1)
          EndSeq=DOS_EOL_fmtW;
        else if (TextFormat==2)
          EndSeq=UNIX_EOL_fmtW;
        else
          EndSeq=MAC_EOL_fmtW;
        CurPtr->EditLine.SetEOLW(EndSeq);
      }
      int EndLength=wcslen(EndSeq);

      char *SaveStrCopy = new char[Length];
      char *EndSeqCopy = new char[EndLength];

      WideCharToMultiByte(CP_OEMCP, 0, SaveStr, Length, SaveStrCopy, Length, NULL, NULL);
      WideCharToMultiByte(CP_OEMCP, 0, EndSeq, EndLength, EndSeqCopy, EndLength, NULL, NULL);

      if (fwrite(SaveStrCopy,1,Length,EditFile)!=Length ||
          fwrite(EndSeqCopy,1,EndLength,EditFile)!=EndLength)
      {
        delete SaveStrCopy;
        delete EndSeqCopy;

        fclose(EditFile);
        _wremove(Name);
        RetCode=SAVEFILE_ERROR;
        goto end;
      }

      delete SaveStrCopy;
      delete EndSeqCopy;

      CurPtr=CurPtr->Next;
    }
    if (fflush(EditFile)==EOF)
    {
      fclose(EditFile);
      _wremove(Name);
      RetCode=SAVEFILE_ERROR;
      goto end;
    }
    SetEndOfFile(hEdit);
    fclose(EditFile);
  }

end:
  SetPreRedrawFunc(NULL);

  if (FileAttributes!=-1 && FileAttributesModified)
  {
    SetFileAttributesW(Name,FileAttributes|FA_ARCH);
  }

  apiGetFindDataEx (strFullFileName,&FileInfo);

  if (FEdit->Flags.Check(FEDITOR_MODIFIED) || NewFile)
    FEdit->Flags.Set(FEDITOR_WASCHANGED);

  /* ���� ����� ���������������� � ��� ������, ���� ����� �����, ���
     ��� ���� ���� ��� ������� � �� ��� ���������� ��� ����� ������...
     ...�� "�����" ������ ���� �����.
  */
//  if(SaveAs)
//    Flags.Clear(FEDITOR_LOCKMODE);


  /*$ 10.08.2000 skv
    Modified->TextChanged
  */
  /* 28.12.2001 VVM
    ! ��������� �� �������� ������ */
  if (RetCode==SAVEFILE_SUCCESS)
    FEdit->TextChanged(0);
  /* VVM $ */
  /* skv$*/

  if(GetDynamicallyBorn()) // ������������� ������� Title // Flags.Check(FFILEEDIT_SAVETOSAVEAS) ????????
    strTitle = L"";

  Show();
  /* IS $ */
  /* IS $ */

  // ************************************
  Flags.Clear(FFILEEDIT_ISNEWFILE);

  return RetCode;
}

int FileEditor::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  if (!EditKeyBar.ProcessMouse(MouseEvent))
    if (!ProcessEditorInput(FrameManager->GetLastInputRecord()))
      if (!FEdit->ProcessMouse(MouseEvent))
        return(FALSE);
  return(TRUE);
}


int FileEditor::GetTypeAndName(string &strType, string &strName)
{
  strType = UMSG(MScreensEdit);
  strName = strFullFileName;

  return(MODALTYPE_EDITOR);
}


void FileEditor::ShowConsoleTitle()
{
    string strTitle;

    strTitle.Format (UMSG(MInEditor), PointToNameW (strFileName));
    SetFarTitleW (strTitle);

    Flags.Clear(FFILEEDIT_REDRAWTITLE);
}


/* $ 28.06.2000 tran
 (NT Console resize)
 resize editor */
void FileEditor::SetScreenPosition()
{
  if (Flags.Check(FFILEEDIT_FULLSCREEN))
  {
    SetPosition(0,0,ScrX,ScrY);
  }
}
/* tran $ */

/* $ 10.05.2001 DJ
   ���������� � view/edit history
*/

void FileEditor::OnDestroy()
{
  _OT(SysLog("[%p] FileEditor::OnDestroy()",this));
  if (!Flags.Check(FFILEEDIT_DISABLEHISTORY) && _wcsicmp(strFileName,UMSG(MNewFileName)))
    CtrlObject->ViewHistory->AddToHistory(strFullFileName,UMSG(MHistoryEdit),
                  (FEdit->Flags.Check(FEDITOR_LOCKMODE)?4:1));
  /* $ 19.10.2001 OT
  */
  if (CtrlObject->Plugins.CurEditor==this)//&this->FEdit)
  {
    CtrlObject->Plugins.CurEditor=NULL;
  }
}

int FileEditor::GetCanLoseFocus(int DynamicMode)
{
  if (DynamicMode)
  {
    if (FEdit->IsFileModified())
    {
      return FALSE;
    }
  }
  else
  {
    return CanLoseFocus;
  }
  return TRUE;
}

void FileEditor::SetLockEditor(BOOL LockMode)
{
  if(LockMode)
    FEdit->Flags.Set(FEDITOR_LOCKMODE);
  else
    FEdit->Flags.Clear(FEDITOR_LOCKMODE);
}

int FileEditor::FastHide()
{
  return Opt.AllCtrlAltShiftRule & CASR_EDITOR;
}

BOOL FileEditor::isTemporary()
{
  return (!GetDynamicallyBorn());
}

void FileEditor::ResizeConsole()
{
  FEdit->PrepareResizedConsole();
}

int FileEditor::ProcessEditorInput(INPUT_RECORD *Rec)
{
  int RetCode;
  _KEYMACRO(CleverSysLog SL("FileEditor::ProcessEditorInput()"));
  _KEYMACRO(if(Rec->EventType == KEY_EVENT)          SysLog("KEY_EVENT:          %cVKey=%s",(Rec->Event.KeyEvent.bKeyDown?0x19:0x18),_VK_KEY_ToName(Rec->Event.KeyEvent.wVirtualKeyCode)));
  _KEYMACRO(if(Rec->EventType == FARMACRO_KEY_EVENT) SysLog("FARMACRO_KEY_EVENT: %cVKey=%s",(Rec->Event.KeyEvent.bKeyDown?0x19:0x18),_VK_KEY_ToName(Rec->Event.KeyEvent.wVirtualKeyCode)));

  CtrlObject->Plugins.CurEditor=this;
  RetCode=CtrlObject->Plugins.ProcessEditorInput(Rec);

  _KEYMACRO(SysLog("RetCode=%d",RetCode));
  return RetCode;
}

void FileEditor::SetPluginTitle(const wchar_t *PluginTitle)
{
    if ( !PluginTitle )
        strPluginTitle = L"";
    else
        strPluginTitle = PluginTitle;
}

BOOL FileEditor::SetFileName(const wchar_t *NewFileName)
{
  strFileName = NewFileName;

  if( wcscmp (strFileName,UMSG(MNewFileName)))
  {
    if ( wcspbrk (strFileName, ReservedFilenameSymbolsW) )
        return FALSE;

    ConvertNameToFullW (strFileName, strFullFileName);

    //���� �������� �������, �������� �������...

    wchar_t *lpwszChar = strFullFileName.GetBuffer ();

    while ( *lpwszChar )
    {
        if ( *lpwszChar == L'/' )
            *lpwszChar = L'\\';

        lpwszChar++;
    }

    strFullFileName.ReleaseBuffer ();
  }
  else
  {
    strFullFileName = strStartDir;
    AddEndSlashW (strFullFileName);

    strFullFileName += strFileName;
  }

  return TRUE;
}

void FileEditor::SetTitle(const wchar_t *Title)
{
    if ( Title == NULL )
        strTitle = L"";
    else
        strTitle = Title;
}

void FileEditor::ChangeEditKeyBar()
{
  if (FEdit->AnsiText)
    EditKeyBar.Change(UMSG(Opt.OnlyEditorViewerUsed?MSingleEditF8DOS:MEditF8DOS),7);
  else
    EditKeyBar.Change(UMSG(Opt.OnlyEditorViewerUsed?MSingleEditF8:MEditF8),7);

  EditKeyBar.Redraw();
}

void FileEditor::GetTitle(string &strLocalTitle,int SubLen,int TruncSize)
{
  if ( !strPluginTitle.IsEmpty () )
    strLocalTitle = strPluginTitle;
  else
  {
    if ( !strTitle.IsEmpty () )
      strLocalTitle = strTitle;
    else
      strLocalTitle = strFullFileName;
  }
}

void FileEditor::ShowStatus()
{
  if ( FEdit->Locked () )
    return;

  SetColor(COL_EDITORSTATUS);

  GotoXY(X1,Y1); //??

  wchar_t wszStatus[NM];
  wchar_t wszLineStr[NM]; //BUGBUG

  string strLocalTitle;
  GetTitle(strLocalTitle);

  int NameLength = Opt.ViewerEditorClock && Flags.Check(FFILEEDIT_FULLSCREEN) ? 19:25;

  if ( X2 > 80)
     NameLength += (X2-80);

  if ( !strPluginTitle.IsEmpty () || !strTitle.IsEmpty ())
    TruncPathStrW (strLocalTitle, (ObjWidth<NameLength?ObjWidth:NameLength));
  else
    TruncPathStrW (strLocalTitle, NameLength);

  //��������������� ������
  swprintf (wszLineStr, L"%d/%d", FEdit->NumLastLine, FEdit->NumLastLine);

  int SizeLineStr = wcslen(wszLineStr);

  if( SizeLineStr > 12 )
    NameLength -= (SizeLineStr-12);
  else
    SizeLineStr = 12;

  swprintf (wszLineStr, L"%d/%d", FEdit->NumLine+1, FEdit->NumLastLine);

  const char *TableName;
  char TmpTableName[32];
  if(FEdit->UseDecodeTable)
  {
    xstrncpy(TmpTableName,FEdit->TableSet.TableName,sizeof(TmpTableName));
    TableName=RemoveChar(TmpTableName,'&',TRUE);
  }
  else
    TableName=FEdit->AnsiText ? "Win":"DOS";

  string strTableName;
  strTableName.SetData (TableName, CP_OEMCP);

  string strAttr;
  strAttr.SetData (AttrStr, CP_OEMCP);

  swprintf (
        wszStatus,
        L"%-*s %c%c%c%10.10s %7s %*.*s %5s %-4d %3s",
        NameLength,
        (const wchar_t*)strLocalTitle,
        (FEdit->Flags.Check(FEDITOR_MODIFIED) ? L'*':L' '),
        (FEdit->Flags.Check(FEDITOR_LOCKMODE) ? L'-':L' '),
        (FEdit->Flags.Check(FEDITOR_PROCESSCTRLQ) ? L'"':L' '),
        (const wchar_t*)strTableName,
        UMSG(MEditStatusLine),
        SizeLineStr,
        SizeLineStr,
        wszLineStr,
        UMSG(MEditStatusCol),
        FEdit->CurLine->EditLine.GetTabCurPos()+1,
        (const wchar_t*)strAttr
        );

  int StatusWidth=ObjWidth - (Opt.ViewerEditorClock && Flags.Check(FFILEEDIT_FULLSCREEN)?5:0);

  if (StatusWidth<0)
    StatusWidth=0;

  mprintfW (L"%-*.*s", StatusWidth, StatusWidth, wszStatus);

  {
    const wchar_t *Str;
    int Length;
    FEdit->CurLine->EditLine.GetBinaryStringW(Str,NULL,Length);
    int CurPos=FEdit->CurLine->EditLine.GetCurPos();
    if (CurPos<Length)
    {
      GotoXY(X2-(Opt.ViewerEditorClock && Flags.Check(FFILEEDIT_FULLSCREEN) ? 9:2),Y1);
      SetColor(COL_EDITORSTATUS);
      /* $ 27.02.2001 SVS
      ���������� � ����������� �� ���� */
      static wchar_t *FmtCharCode[3]={L"%03o",L"%3d",L"%02Xh"};
      mprintfW(FmtCharCode[FEdit->EdOpt.CharCodeBase%3],(wchar_t)Str[CurPos]);
      /* SVS $ */
    }
  }

  if (Opt.ViewerEditorClock && Flags.Check(FFILEEDIT_FULLSCREEN))
    ShowTime(FALSE);
}

/* $ 13.02.2001
     ������ �������� ����� � ������ ���������� ������� ������ ��������� ���
     �������.
*/
DWORD FileEditor::GetFileAttributes(const wchar_t *Name)
{
  FileAttributes=::GetFileAttributesW(Name);
  int ind=0;
  if(0xFFFFFFFF!=FileAttributes)
  {
     if(FileAttributes&FILE_ATTRIBUTE_READONLY) AttrStr[ind++]='R';
     if(FileAttributes&FILE_ATTRIBUTE_SYSTEM) AttrStr[ind++]='S';
     if(FileAttributes&FILE_ATTRIBUTE_HIDDEN) AttrStr[ind++]='H';
  }
  AttrStr[ind]=0;
  return FileAttributes;
}
/* IS $ */

/* Return TRUE - ������ �������
*/
BOOL FileEditor::UpdateFileList()
{
  Panel *ActivePanel = CtrlObject->Cp()->ActivePanel;
  const wchar_t *FileName = PointToNameW(strFullFileName);
  string strFilePath, strPanelPath;

  wchar_t *lpwszFilePath = strFilePath.GetBuffer();

  xwcsncpy(lpwszFilePath, strFullFileName, (FileName - (const wchar_t*)strFullFileName)/sizeof(wchar_t));

  strFilePath.ReleaseBuffer();

  ActivePanel->GetCurDirW(strPanelPath);
  AddEndSlashW(strPanelPath);
  AddEndSlashW(strFilePath);
  if (!wcscmp(strPanelPath, strFilePath))
  {
    ActivePanel->Update(UPDATE_KEEP_SELECTION|UPDATE_DRAW_MESSAGE);
    return TRUE;
  }
  return FALSE;
}

void FileEditor::SetPluginData(const wchar_t *PluginData)
{
  FileEditor::strPluginData = NullToEmptyW(PluginData);
}

/* $ 14.06.2002 IS
   DeleteOnClose ���� int:
     0 - �� ������� ������
     1 - ������� ���� � �������
     2 - ������� ������ ����
*/
void FileEditor::SetDeleteOnClose(int NewMode)
{
  FEdit->Flags.Clear(FEDITOR_DELETEONCLOSE|FEDITOR_DELETEONLYFILEONCLOSE);
  if(NewMode==1)
    FEdit->Flags.Set(FEDITOR_DELETEONCLOSE);
  else if(NewMode==2)
    FEdit->Flags.Set(FEDITOR_DELETEONLYFILEONCLOSE);
}
/* IS $ */

void FileEditor::GetEditorOptions(struct EditorOptions& EdOpt)
{
  memmove(&EdOpt,&FEdit->EdOpt,sizeof(struct EditorOptions));
}

void FileEditor::SetEditorOptions(struct EditorOptions& EdOpt)
{
  FEdit->SetTabSize(EdOpt.TabSize);
  FEdit->SetConvertTabs(EdOpt.ExpandTabs);
  FEdit->SetPersistentBlocks(EdOpt.PersistentBlocks);
  FEdit->SetDelRemovesBlocks(EdOpt.DelRemovesBlocks);
  FEdit->SetAutoIndent(EdOpt.AutoIndent);
  FEdit->SetAutoDetectTable(EdOpt.AutoDetectTable);
  FEdit->SetCursorBeyondEOL(EdOpt.CursorBeyondEOL);
  FEdit->SetCharCodeBase(EdOpt.CharCodeBase);
  FEdit->SetSavePosMode(EdOpt.SavePos, EdOpt.SaveShortPos);
  //FEdit->SetBSLikeDel(EdOpt.BSLikeDel);
}

int FileEditor::EditorControl(int Command,void *Param)
{
#if defined(SYSLOG_KEYMACRO)
  _KEYMACRO(CleverSysLog SL("FileEditor::EditorControl()"));
  if(Command == ECTL_READINPUT || Command == ECTL_PROCESSINPUT)
  {
    _KEYMACRO(SysLog("(Command=%s, Param=[%d/0x%08X]) Macro.IsExecuting()=%d",_ECTL_ToName(Command),(int)Param,Param,CtrlObject->Macro.IsExecuting()));
  }
#else
  _ECTLLOG(CleverSysLog SL("FileEditor::EditorControl()"));
  _ECTLLOG(SysLog("(Command=%s, Param=[%d/0x%08X])",_ECTL_ToName(Command),(int)Param,Param));
#endif
  if (bClosing && Command != ECTL_GETINFO && Command != ECTL_GETBOOKMARKS && Command != ECTL_FREEINFO)
    return FALSE;
  switch(Command)
  {
    case ECTL_GETINFO:
    {
      if (FEdit->EditorControl(Command,Param))
      {
        struct EditorInfo *Info=(struct EditorInfo *)Param;
        Info->FileName=_wcsdup (strFullFileName); //BUGBUG
        _ECTLLOG(SysLog("struct EditorInfo{"));
        _ECTLLOG(SysLog("  EditorID       = %d",Info->EditorID));
        _ECTLLOG(SysLog("  FileName       = '%s'",Info->FileName));
        _ECTLLOG(SysLog("  WindowSizeX    = %d",Info->WindowSizeX));
        _ECTLLOG(SysLog("  WindowSizeY    = %d",Info->WindowSizeY));
        _ECTLLOG(SysLog("  TotalLines     = %d",Info->TotalLines));
        _ECTLLOG(SysLog("  CurLine        = %d",Info->CurLine));
        _ECTLLOG(SysLog("  CurPos         = %d",Info->CurPos));
        _ECTLLOG(SysLog("  CurTabPos;     = %d",Info->CurTabPos));
        _ECTLLOG(SysLog("  TopScreenLine  = %d",Info->TopScreenLine));
        _ECTLLOG(SysLog("  LeftPos        = %d",Info->LeftPos));
        _ECTLLOG(SysLog("  Overtype       = %d",Info->Overtype));
        _ECTLLOG(SysLog("  BlockType      = %s (%d)",(Info->BlockType==BTYPE_NONE?"BTYPE_NONE":((Info->BlockType==BTYPE_STREAM?"BTYPE_STREAM":((Info->BlockType==BTYPE_COLUMN?"BTYPE_COLUMN":"BTYPE_?????"))))),Info->BlockType));
        _ECTLLOG(SysLog("  BlockStartLine = %d",Info->BlockStartLine));
        _ECTLLOG(SysLog("  AnsiMode       = %d",Info->AnsiMode));
        _ECTLLOG(SysLog("  TableNum       = %d",Info->TableNum));
        _ECTLLOG(SysLog("  Options        = 0x%08X",Info->Options));
        _ECTLLOG(SysLog("  TabSize        = %d",Info->TabSize));
        _ECTLLOG(SysLog("  BookMarkCount  = %d",Info->BookMarkCount));
        _ECTLLOG(SysLog("  CurState       = 0x%08X",Info->CurState));
        _ECTLLOG(SysLog("}"));
        return TRUE;
      }
      return FALSE;
    }

    case ECTL_FREEINFO:
    {
      struct EditorInfo *Info=(struct EditorInfo *)Param;
      xf_free ((void*)Info->FileName);
    }

    case ECTL_GETBOOKMARKS:
    {
      if(!FEdit->Flags.Check(FEDITOR_OPENFAILED) && Param)
      {
        struct EditorBookMarks *ebm=(struct EditorBookMarks *)Param;
        if(ebm->Line && !IsBadWritePtr(ebm->Line,BOOKMARK_COUNT*sizeof(long)))
          memcpy(ebm->Line,FEdit->SavePos.Line,BOOKMARK_COUNT*sizeof(long));
        if(ebm->Cursor && !IsBadWritePtr(ebm->Cursor,BOOKMARK_COUNT*sizeof(long)))
          memcpy(ebm->Cursor,FEdit->SavePos.Cursor,BOOKMARK_COUNT*sizeof(long));
        if(ebm->ScreenLine && !IsBadWritePtr(ebm->ScreenLine,BOOKMARK_COUNT*sizeof(long)))
          memcpy(ebm->ScreenLine,FEdit->SavePos.ScreenLine,BOOKMARK_COUNT*sizeof(long));
        if(ebm->LeftPos && !IsBadWritePtr(ebm->LeftPos,BOOKMARK_COUNT*sizeof(long)))
          memcpy(ebm->LeftPos,FEdit->SavePos.LeftPos,BOOKMARK_COUNT*sizeof(long));
        return TRUE;
      }
      return FALSE;
    }

    case ECTL_SETTITLE:
    {
      // $ 08.06.2001 IS - ���: �� ���������� ������ PluginTitle
      _ECTLLOG(SysLog("Title='%s'",Param));

      if ( Param)
        strPluginTitle.SetData ((const char*)Param, CP_OEMCP);
      else
        strPluginTitle = L"";

      ShowStatus();
      ScrBuf.Flush();
      return TRUE;
    }

    case ECTL_EDITORTOOEM:
    {
      if(!Param)
        return FALSE;
      /*struct EditorConvertText *ect=(struct EditorConvertText *)Param;
      _ECTLLOG(SysLog("struct EditorConvertText{"));
      _ECTLLOG(SysLog("  Text       ='%s'",ect->Text));
      _ECTLLOG(SysLog("  TextLength =%d",ect->TextLength));
      _ECTLLOG(SysLog("}"));
      if (FEdit->UseDecodeTable && ect->Text)
      {
        DecodeString(ect->Text,(unsigned char *)FEdit->TableSet.DecodeTable,ect->TextLength);
        _ECTLLOG(SysLog("DecodeString -> ect->Text='%s'",ect->Text));
      }*/ //BUGBUG
      return TRUE;
    }

    case ECTL_OEMTOEDITOR:
    {
      if(!Param)
        return FALSE;

      /*struct EditorConvertText *ect=(struct EditorConvertText *)Param;
      _ECTLLOG(SysLog("struct EditorConvertText{"));
      _ECTLLOG(SysLog("  Text       ='%s'",ect->Text));
      _ECTLLOG(SysLog("  TextLength =%d",ect->TextLength));
      _ECTLLOG(SysLog("}"));
      if (FEdit->UseDecodeTable && ect->Text)
      {
        EncodeString(ect->Text,(unsigned char *)FEdit->TableSet.EncodeTable,ect->TextLength);
        _ECTLLOG(SysLog("EncodeString -> ect->Text='%s'",ect->Text));
      }*/ //BUGBUG
      return TRUE;
    }

    case ECTL_REDRAW:
    {
      FileEditor::DisplayObject();
      ScrBuf.Flush();
      return(TRUE);
    }

    /* $ 07.08.2000 SVS
       ������� ��������� Keybar Labels
         Param = NULL - ������������, ����. ��������
         Param = -1   - �������� ������ (������������)
         Param = KeyBarTitles
    */
    // ������ ����������� � FileEditor::EditorControl()
    case ECTL_SETKEYBAR:
    {
      struct KeyBarTitles *Kbt=(struct KeyBarTitles*)Param;
      if(!Kbt)
      {        // ������������ ���� ��������!
        InitKeyBar();
      }
      else
      {
        if((long)Param != (long)-1) // �� ������ ������������?
        {
          for(int I=0; I < 12; ++I)
          {
            if(Kbt->Titles[I])
              EditKeyBar.Change(KBL_MAIN,Kbt->Titles[I],I);
            if(Kbt->CtrlTitles[I])
              EditKeyBar.Change(KBL_CTRL,Kbt->CtrlTitles[I],I);
            if(Kbt->AltTitles[I])
              EditKeyBar.Change(KBL_ALT,Kbt->AltTitles[I],I);
            if(Kbt->ShiftTitles[I])
              EditKeyBar.Change(KBL_SHIFT,Kbt->ShiftTitles[I],I);
            if(Kbt->CtrlShiftTitles[I])
              EditKeyBar.Change(KBL_CTRLSHIFT,Kbt->CtrlShiftTitles[I],I);
            if(Kbt->AltShiftTitles[I])
              EditKeyBar.Change(KBL_ALTSHIFT,Kbt->AltShiftTitles[I],I);
            if(Kbt->CtrlAltTitles[I])
              EditKeyBar.Change(KBL_CTRLALT,Kbt->CtrlAltTitles[I],I);
          }
        }
        EditKeyBar.Show();
      }
      return TRUE;
    }
    /* SVS $ */

    case ECTL_SAVEFILE:
    {
      EditorSaveFile *esf=(EditorSaveFile *)Param;
      string strName = strFullFileName;
      int EOL=0;
      if (esf!=NULL)
      {
        _ECTLLOG(char *LinDump=(esf->FileEOL?(char *)_SysLog_LinearDump(esf->FileEOL,strlen(esf->FileEOL)):NULL));
        _ECTLLOG(SysLog("struct EditorSaveFile{"));
        _ECTLLOG(SysLog("  FileName   ='%s'",esf->FileName));
        _ECTLLOG(SysLog("  FileEOL    ='%s'",(esf->FileEOL?LinDump:"(null)")));
        _ECTLLOG(SysLog("}"));
        _ECTLLOG(if(LinDump)xf_free(LinDump));

        if (*esf->FileName)
          strName=esf->FileName;
        if (esf->FileEOL!=NULL)
        {
          if (wcscmp(esf->FileEOL,DOS_EOL_fmtW)==0)
            EOL=1;
          if (wcscmp(esf->FileEOL,UNIX_EOL_fmtW)==0)
            EOL=2;
          if (wcscmp(esf->FileEOL,MAC_EOL_fmtW)==0)
            EOL=3;
        }
        _ECTLLOG(SysLog("EOL=%d",EOL));
      }

      {
        string strOldFullFileName = strFullFileName;

        if(SetFileName(strName))
          return SaveFile(strName,FALSE,EOL,!LocalStricmpW(strName, strOldFullFileName));
      }
      return FALSE;
    }

    case ECTL_QUIT:
    {
      FrameManager->DeleteFrame(this);
      SetExitCode(SAVEFILE_ERROR); // ���-�� ���� ������� ������� �������� ...???
      return(TRUE);
    }

    case ECTL_READINPUT:
    {
      _KEYMACRO(CleverSysLog SL("FileEditor::EditorControl(ECTL_READINPUT)"));

      if(CtrlObject->Macro.IsRecording() == MACROMODE_RECORDING || CtrlObject->Macro.IsExecuting() == MACROMODE_EXECUTING)
      {
        _KEYMACRO(SysLog("%d if(CtrlObject->Macro.IsRecording() == MACROMODE_RECORDING || CtrlObject->Macro.IsExecuting() == MACROMODE_EXECUTING)",__LINE__));
//        return FALSE;
      }

      if(!Param)
      {
        _ECTLLOG(SysLog("Param = NULL"));
        return FALSE;
      }
      else
      {
        INPUT_RECORD *rec=(INPUT_RECORD *)Param;
        DWORD Key;
        while(1)
        {
          Key=GetInputRecord(rec);
          if((!rec->EventType || rec->EventType == KEY_EVENT || rec->EventType == FARMACRO_KEY_EVENT) && Key >= KEY_MACRO_BASE && Key <= KEY_MACRO_ENDBASE) // ��������� MACRO
             ReProcessKey(Key);
          else
            break;
        }
        //if(Key==KEY_CONSOLE_BUFFER_RESIZE) //????
        //  Show();                          //????
#if defined(SYSLOG_KEYMACRO)
        if(rec->EventType == KEY_EVENT)
        {
          SysLog("ECTL_READINPUT={%s,{%d,%d,Vk=0x%04X,0x%08X}}",
                             (rec->EventType == FARMACRO_KEY_EVENT?"FARMACRO_KEY_EVENT":"KEY_EVENT"),
                             rec->Event.KeyEvent.bKeyDown,
                             rec->Event.KeyEvent.wRepeatCount,
                             rec->Event.KeyEvent.wVirtualKeyCode,
                             rec->Event.KeyEvent.dwControlKeyState);
        }
#endif
      }
      return(TRUE);
    }

    case ECTL_PROCESSINPUT:
    {
      _KEYMACRO(CleverSysLog SL("FileEditor::EditorControl(ECTL_PROCESSINPUT)"));

      if(!Param)
      {
        _ECTLLOG(SysLog("Param = NULL"));
        return FALSE;
      }
      else
      {
        INPUT_RECORD *rec=(INPUT_RECORD *)Param;
        if (ProcessEditorInput(rec))
        {
          _ECTLLOG(SysLog("ProcessEditorInput(rec) => return 1 !!!"));
          return(TRUE);
        }
        if (rec->EventType==MOUSE_EVENT)
          ProcessMouse(&rec->Event.MouseEvent);
        else
        {
#if defined(SYSLOG_KEYMACRO)
          if(!rec->EventType || rec->EventType == KEY_EVENT || rec->EventType == FARMACRO_KEY_EVENT)
          {
            SysLog("ECTL_PROCESSINPUT={%s,{%d,%d,Vk=0x%04X,0x%08X}}",
                             (rec->EventType == FARMACRO_KEY_EVENT?"FARMACRO_KEY_EVENT":"KEY_EVENT"),
                             rec->Event.KeyEvent.bKeyDown,
                             rec->Event.KeyEvent.wRepeatCount,
                             rec->Event.KeyEvent.wVirtualKeyCode,
                             rec->Event.KeyEvent.dwControlKeyState);
          }
#endif
          int Key=CalcKeyCode(rec,FALSE);
          _KEYMACRO(SysLog("Key=CalcKeyCode() = 0x%08X",Key));
          ReProcessKey(Key);
        }
      }
      return(TRUE);
    }

    case ECTL_PROCESSKEY:
    {
      _KEYMACRO(CleverSysLog SL("FileEditor::EditorControl(ECTL_PROCESSKEY)"));
      _ECTLLOG(SysLog("Key = %s",_FARKEY_ToName((DWORD)Param)));
      ReProcessKey((int)Param);
      return TRUE;
    }

  }

  return FEdit->EditorControl(Command,Param);
}
