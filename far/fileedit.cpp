/*
fileedit.cpp

Редактирование файла - надстройка над editor.cpp

*/

/* Revision: 1.06 27.09.2000 $ */

/*
Modify:
  27.09.2000 SKV
    + Для правильного функционирования макро с Ctrl-O делается FEdit.Hide()
  24.08.2000 SVS
    + Добавляем реакцию показа бакграунда на клавишу CtrlAltShift
  07.08.2000 SVS
    + добавил названия расширенных функциональных клавиш
    + Функция инициализации KeyBar Labels - InitKeyBar()
  21.07.2000 SKV
    + выход с позиционированием на редактируемом файле по CTRLF10
  29.06.2000 tran
    + названия всех функциональных клавиш
  28.06.2000 tran
    - (NT Console resize bug)
      adding FileEditor::SetScreenPosition
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

FileEditor::FileEditor(char *Name,int CreateNewFile,int EnableSwitch,
                       int StartLine,int StartChar,int DisableHistory,
                       char *PluginData)
{
  SetPosition(0,0,ScrX,ScrY);
  FullScreen=TRUE;
  Init(Name,CreateNewFile,EnableSwitch,StartLine,StartChar,DisableHistory,PluginData);
}


FileEditor::FileEditor(char *Name,int CreateNewFile,int EnableSwitch,
            int StartLine,int StartChar,char *Title,
            int X1,int Y1,int X2,int Y2)
{
  SetPosition(X1,Y1,X2,Y2);
  FullScreen=(X1==0 && Y1==0 && X2==ScrX && Y2==ScrY);
  FEdit.SetTitle(Title);
  Init(Name,CreateNewFile,EnableSwitch,StartLine,StartChar,TRUE,"");
}


void FileEditor::Init(char *Name,int CreateNewFile,int EnableSwitch,
                      int StartLine,int StartChar,int DisableHistory,
                      char *PluginData)
{
  if (*Name==0)
    return;
  FEdit.SetPluginData(PluginData);
  FEdit.SetHostFileEditor(this);
  SetEnableSwitch(EnableSwitch);
  GetCurrentDirectory(sizeof(StartDir),StartDir);
  strcpy(FileName,Name);
  ConvertNameToFull(FileName,FullFileName);
  if (EnableSwitch)
  {
    int ModalPos=CtrlObject->ModalManager.FindModalByFile(MODALTYPE_EDITOR,FullFileName);
    if (ModalPos!=-1)
    {
      int MsgCode=Message(0,2,MSG(MEditTitle),FullFileName,MSG(MAskReload),
                          MSG(MCurrent),MSG(MReload));
      switch(MsgCode)
      {
        case 0:
          CtrlObject->ModalManager.SetModalPos(ModalPos);
          CtrlObject->ModalManager.NextModal(0);
          return;
        case 1:
          break;
        default:
          return;
      }
    }
  }
  FEdit.SetPosition(X1,Y1,X2,Y2-1);
  FEdit.SetStartPos(StartLine,StartChar);
  int UserBreak;
  if (!FEdit.ReadFile(FileName,UserBreak) && (!CreateNewFile || UserBreak))
  {
    if (UserBreak!=1)
    {
      Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MEditTitle),MSG(MEditCannotOpen),FileName,MSG(MOk));
      ExitCode=0;
    }
    else
      ExitCode=3;
    return;
  }
  ShowConsoleTitle();
  EditKeyBar.SetOwner(this);
  EditKeyBar.SetPosition(X1,Y2,X2,Y2);

  /* $ 07.08.2000 SVS
    ! Код, касаемый KeyBar вынесен в отдельную функцию */
  InitKeyBar();
  /* SVS $*/

  Process();
  ExitCode=IsFileChanged() ? 1 : 2;
  if (!DisableHistory)
    CtrlObject->ViewHistory->AddToHistory(FullFileName,MSG(MHistoryEdit),1);
}

/* $ 07.08.2000 SVS
  Функция инициализации KeyBar Labels
*/
void FileEditor::InitKeyBar(void)
{
  /* $ 29.06.2000 tran
     добавил названия всех функциональных клавиш */
  char *FEditKeys[]={MSG(MEditF1),MSG(MEditF2),MSG(MEditF3),MSG(MEditF4),MSG(MEditF5),EnableSwitch ? MSG(MEditF6):"",MSG(MEditF7),MSG(MEditF8),MSG(MEditF9),MSG(MEditF10),MSG(MEditF11),MSG(MEditF12)};
  char *FEditShiftKeys[]={MSG(MEditShiftF1),MSG(MEditShiftF2),MSG(MEditShiftF3),MSG(MEditShiftF4),MSG(MEditShiftF5),MSG(MEditShiftF6),MSG(MEditShiftF7),MSG(MEditShiftF8),MSG(MEditShiftF9),MSG(MEditShiftF10),MSG(MEditShiftF11),MSG(MEditShiftF12)};
  char *FEditAltKeys[]={MSG(MEditAltF1),MSG(MEditAltF2),MSG(MEditAltF3),MSG(MEditAltF4),MSG(MEditAltF5),MSG(MEditAltF6),MSG(MEditAltF7),MSG(MEditAltF8),MSG(MEditAltF9),MSG(MEditAltF10),MSG(MEditAltF11),MSG(MEditAltF12)};
  char *FEditCtrlKeys[]={MSG(MEditCtrlF1),MSG(MEditCtrlF2),MSG(MEditCtrlF3),MSG(MEditCtrlF4),MSG(MEditCtrlF5),MSG(MEditCtrlF6),MSG(MEditCtrlF7),MSG(MEditCtrlF8),MSG(MEditCtrlF9),MSG(MEditCtrlF10),MSG(MEditCtrlF11),MSG(MEditCtrlF12)};
  /* tran $ */
  /* $ 07.08.2000 SVS
     добавил названия расширенных функциональных клавиш */
  char *FEditAltShiftKeys[]={MSG(MEditAltShiftF1),MSG(MEditAltShiftF2),MSG(MEditAltShiftF3),MSG(MEditAltShiftF4),MSG(MEditAltShiftF5),MSG(MEditAltShiftF6),MSG(MEditAltShiftF7),MSG(MEditAltShiftF8),MSG(MEditAltShiftF9),MSG(MEditAltShiftF10),MSG(MEditAltShiftF11),MSG(MEditAltShiftF12)};
  char *FEditCtrlShiftKeys[]={MSG(MEditCtrlShiftF1),MSG(MEditCtrlShiftF2),MSG(MEditCtrlShiftF3),MSG(MEditCtrlShiftF4),MSG(MEditCtrlShiftF5),MSG(MEditCtrlShiftF6),MSG(MEditCtrlShiftF7),MSG(MEditCtrlShiftF8),MSG(MEditCtrlShiftF9),MSG(MEditCtrlShiftF10),MSG(MEditCtrlShiftF11),MSG(MEditCtrlShiftF12)};
  char *FEditCtrlAltKeys[]={MSG(MEditCtrlAltF1),MSG(MEditCtrlAltF2),MSG(MEditCtrlAltF3),MSG(MEditCtrlAltF4),MSG(MEditCtrlAltF5),MSG(MEditCtrlAltF6),MSG(MEditCtrlAltF7),MSG(MEditCtrlAltF8),MSG(MEditCtrlAltF9),MSG(MEditCtrlAltF10),MSG(MEditCtrlAltF11),MSG(MEditCtrlAltF12)};
  /* SVS $*/

  EditKeyBar.Set(FEditKeys,sizeof(FEditKeys)/sizeof(FEditKeys[0]));
  EditKeyBar.SetShift(FEditShiftKeys,sizeof(FEditShiftKeys)/sizeof(FEditShiftKeys[0]));
  EditKeyBar.SetAlt(FEditAltKeys,sizeof(FEditAltKeys)/sizeof(FEditAltKeys[0]));
  EditKeyBar.SetCtrl(FEditCtrlKeys,sizeof(FEditCtrlKeys)/sizeof(FEditCtrlKeys[0]));

  /* $ 07.08.2000 SVS
     добавил названия расширенных функциональных клавиш */
  EditKeyBar.SetCtrlAlt(FEditCtrlAltKeys,sizeof(FEditCtrlAltKeys)/sizeof(FEditCtrlAltKeys[0]));
  EditKeyBar.SetCtrlShift(FEditCtrlShiftKeys,sizeof(FEditCtrlShiftKeys)/sizeof(FEditCtrlShiftKeys[0]));
  EditKeyBar.SetAltShift(FEditAltShiftKeys,sizeof(FEditAltShiftKeys)/sizeof(FEditAltShiftKeys[0]));
  /* SVS $ */

  EditKeyBar.Show();
  SetKeyBar(&EditKeyBar);
  FEdit.SetEditKeyBar(&EditKeyBar);
}
/* SVS $ */

void FileEditor::Process()
{
  ChangeMacroMode MacroMode(MACRO_EDITOR);
  Modal::Process();
}


void FileEditor::Show()
{
  if (FullScreen)
  {
    EditKeyBar.SetPosition(0,ScrY,ScrX,ScrY);
    EditKeyBar.Redraw();
    SetPosition(0,0,ScrX,ScrY-1);
    FEdit.SetPosition(0,0,ScrX,ScrY-1);
  }
  ScreenObject::Show();
}


void FileEditor::DisplayObject()
{
  FEdit.Show();
}


int FileEditor::ProcessKey(int Key)
{
  switch(Key)
  {
    /* $ 24.08.2000 SVS
       + Добавляем реакцию показа бакграунда на клавишу CtrlAltShift
    */
    case KEY_CTRLALTSHIFTPRESS:
    case KEY_CTRLO:
    /*$ 27.09.2000 skv
      To prevent redraw in macro with Ctrl-O
    */
      FEdit.Hide();
    /* skv$*/
      Hide();
      if (CtrlObject->LeftPanel!=CtrlObject->RightPanel)
        CtrlObject->ModalManager.ShowBackground();
      else
      {
        EditKeyBar.Hide();
        WaitKey(Key==KEY_CTRLALTSHIFTPRESS?KEY_CTRLALTSHIFTRELEASE:-1);
      }
      EditKeyBar.Show();
      Show();
      return(TRUE);
    /* SVS $ */
    case KEY_CTRLTAB:
    case KEY_CTRLSHIFTTAB:
    case KEY_F12:
      if (GetEnableSwitch())
      {
        FEdit.KeepInitParameters();
        if (Key==KEY_CTRLSHIFTTAB)
          SetExitCode(3);
        else
          SetExitCode(Key==KEY_CTRLTAB ? 1:2);
      }
      return(TRUE);
    case KEY_F2:
    case KEY_SHIFTF2:
      {
        static int TextFormat=0;
        int NameChanged=FALSE;
        if (Key==KEY_SHIFTF2)
        {
          const char *HistoryName="NewEdit";
          static struct DialogData EditDlgData[]=
          {
            DI_DOUBLEBOX,3,1,72,10,0,0,0,0,(char *)MEditTitle,
            DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,(char *)MEditSaveAs,
            DI_EDIT,5,3,70,3,1,(DWORD)HistoryName,DIF_HISTORY,1,"",
            DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
            DI_RADIOBUTTON,5,5,0,0,0,0,DIF_GROUP,0,(char *)MEditSaveOriginal,
            DI_RADIOBUTTON,5,6,0,0,0,0,0,0,(char *)MEditSaveDOS,
            DI_RADIOBUTTON,5,7,0,0,0,0,0,0,(char *)MEditSaveUnix,
            DI_TEXT,3,8,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
            DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
            DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel,
          };
          MakeDialogItems(EditDlgData,EditDlg);
          strcpy(EditDlg[2].Data,FileName);
          EditDlg[4].Selected=EditDlg[5].Selected=EditDlg[6].Selected=0;
          EditDlg[4+TextFormat].Selected=TRUE;
          Dialog Dlg(EditDlg,sizeof(EditDlg)/sizeof(EditDlg[0]));
          Dlg.SetPosition(-1,-1,76,12);
          Dlg.Process();
          if (Dlg.GetExitCode()!=8 || *EditDlg[2].Data==0)
            return(FALSE);
          Unquote(EditDlg[2].Data);
          RemoveTrailingSpaces(EditDlg[2].Data);
          NameChanged=LocalStricmp(EditDlg[2].Data,FileName)!=0;
          strcpy(FileName,EditDlg[2].Data);
          ConvertNameToFull(FileName,FullFileName);
          if (EditDlg[4].Selected)
            TextFormat=0;
          if (EditDlg[5].Selected)
            TextFormat=1;
          if (EditDlg[6].Selected)
            TextFormat=2;
        }
        ShowConsoleTitle();
        chdir(StartDir);

        if (NameChanged && GetFileAttributes(FileName)!=0xFFFFFFFF)
          if (Message(MSG_WARNING,2,MSG(MEditTitle),FileName,MSG(MEditExists),
                      MSG(MEditOvr),MSG(MYes),MSG(MNo))!=0)
            return(TRUE);

        while (FEdit.SaveFile(FileName,0,Key==KEY_SHIFTF2 ? TextFormat:0)==0)
          if (Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MEditTitle),MSG(MEditCannotSave),
                      FileName,MSG(MRetry),MSG(MCancel))!=0)
            break;
      }
      return(TRUE);
    case KEY_F6:
      if (GetEnableSwitch() &&
          (FEdit.IsFileChanged() || GetFileAttributes(FullFileName)!=0xFFFFFFFF))
      {
        long FilePos=FEdit.GetCurPos();
        ProcessKey(KEY_ESC);
        if (Done())
          CtrlObject->ModalManager.SetNextWindow(TRUE,FileName,FilePos);
        ShowTime(2);
      }
      return(TRUE);
    /*$ 21.07.2000 SKV
        + выход с позиционированием на редактируемом файле по CTRLF10
    */
    case KEY_CTRLF10:
      {
        if(GetEnableSwitch())
        {
          ProcessKey(KEY_F10);
          if(strchr(FileName,'\\') || strchr(FileName,'/'))
          {
            char DirTmp[NM],*NameTmp;
            strncpy(DirTmp,FileName,NM);
            NameTmp=PointToName(DirTmp);
            if(NameTmp>DirTmp)NameTmp[-1]=0;
            CtrlObject->ActivePanel->SetCurDir(DirTmp,TRUE);
            CtrlObject->ActivePanel->GoToFile(NameTmp);
          }else
          {
            CtrlObject->ActivePanel->SetCurDir(StartDir,TRUE);
            CtrlObject->ActivePanel->GoToFile(FileName);
          }
        }
        return (TRUE);
      }
    /* SKV $*/

    case KEY_SHIFTF10:
      ProcessKey(KEY_F2);
    case KEY_ESC:
    case KEY_F10:
      {
        int FirstSave=1;
        while (1)
        {
          chdir(StartDir);
          int SaveCode=FEdit.SaveFile(FileName,FirstSave,0);
          if (SaveCode==2)
            break;
          if (SaveCode==1)
          {
            SetExitCode(0);
            break;
          }
          if (Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MEditTitle),MSG(MEditCannotSave),
                      FileName,MSG(MRetry),MSG(MCancel))!=0)
            break;
          FirstSave=0;
        }
      }
      return(TRUE);
    default:
      if (CtrlObject->Macro.IsExecuting() || !FEdit.ProcessEditorInput(&ReadRec))
        return(FEdit.ProcessKey(Key));
      return(TRUE);
  }
}


int FileEditor::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  if (!EditKeyBar.ProcessMouse(MouseEvent))
    if (!FEdit.ProcessEditorInput(&ReadRec))
      if (!FEdit.ProcessMouse(MouseEvent))
        return(FALSE);
  return(TRUE);
}


int FileEditor::GetTypeAndName(char *Type,char *Name)
{
  strcpy(Type,MSG(MScreensEdit));
  strcpy(Name,FullFileName);
  return(MODALTYPE_EDITOR);
}


void FileEditor::ShowConsoleTitle()
{
  char Title[NM+20];
  sprintf(Title,MSG(MInEditor),PointToName(FileName));
  SetFarTitle(Title);
}


int FileEditor::GetExitCode()
{
  return(ExitCode);
}

/* $ 28.06.2000 tran
 (NT Console resize)
 resize editor */
void FileEditor::SetScreenPosition()
{
  if (FullScreen)
  {
    SetPosition(0,0,ScrX,ScrY);
    Show();
  }
}
/* tran $ */
