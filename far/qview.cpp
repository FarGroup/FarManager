/*
qview.cpp

Quick view panel

*/

/* Revision: 1.36 07.07.2005 $ */

/*
Modify:
  07.07.2005 SVS
    ! Вьюверные настройки собраны в одно место
  26.04.2005 SVS
    ! Вместо кучи кода - вызов одной функции Panel::ProcessShortcutFolder()
  01.03.2004 SVS
    ! Обертки FAR_OemTo* и FAR_CharTo* вокруг одноименных WinAPI-функций
      (задел на будущее + править впоследствии только 1 файл)
  12.09.2003 SVS
    ! Немного увеличим буфер для GetPathRootOne
  06.05.2003 SVS
    + Фича: показ Ratio (задолбался сегодня на калькуляторе считать ;-(
  26.02.2003 SVS
    - BugZ#813 - DM_RESIZEDIALOG в DN_DRAWDIALOG -> проблема
    ! вместо ShellUpdatePanels() исполним Redraw()
  25.02.2003 SVS
    - BugZ#805 - Остается прорисовка после поиска в Qview & InfoPanel
  26.01.2003 IS
    ! FAR_DeleteFile вместо DeleteFile, FAR_RemoveDirectory вместо
      RemoveDirectory, просьба и впредь их использовать для удаления
      соответственно файлов и каталогов.
  24.05.2002 SVS
    + Дублирование Numpad-клавиш
  27.04.2002 SVS
    ! 8192 -> MAXSIZE_SHORTCUTDATA
  22.03.2002 SVS
    - strcpy - Fuck!
  22.03.2002 DJ
    ! отведем побольше места на Title
  14.02.2002 VVM
    ! UpdateIfChanged принимает не булевый Force, а варианты из UIC_*
  16.01.2002 SVS
    ! уточнение кейбара для варианта с каталогом (косметика)
  24.12.2001
    + virtual int GetCurName(char *Name,char *ShortName) - текущий просматриваемый файл
  08.12.2001 IS
    - баг: после показа хелпа по панели показывалась непонятно зачем
      еще и справка по встроенной программе просмотра
  11.09.2001 SVS
    + для "volume mount point" укажем что это именно монтированный том и по
      возможности (если имя диска есть) покажем букву диска. Для прочих
      символических связей оставим "Link"
  16.05.2001 DJ
    ! proof-of-concept
  15.05.2001 OT
    ! NWZ -> NFZ
  06.05.2001 DJ
    ! перетрях #include
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  30.04.2001 DJ
    + UpdateKeyBar()
    - правильный help topic
  05.04.2001 VVM
    + Переключение макросов в режим MACRO_QVIEWPANEL
  12.03.2001 SVS
    ! Коррекция в связи с изменениями в классе int64
  28.02.2001 IS
    ! "CtrlObject->CmdLine." -> "CtrlObject->CmdLine->"
  20.02.2001 VVM
    ! Исправление поведения врапа. (Оторвал зависимость от вьюере)
  12.02.2001 SVS
    ! Выделенный текст отображается COL_PANELINFOTEXT (Highlighted info),
      а не COL_PANELSELECTEDTEXT (Selected text)
  01.02.2001 SVS
    + В панели "Quick view" добавим инфу про Junction
  03.11.2000 OT
    ! Введение проверки возвращаемого значения
  02.11.2000 OT
    ! Введение проверки на длину буфера, отведенного под имя файла.
  04.08.2000 tran 1.06
     Gray+, Gray- передвигают курсор на другой панели
  20.07.2000 tran
    - bug#21, пустой заголовок консоли
      теперь он верный всегда
  12.07.2000 SVS
    ! Для возможности 3-х позиционного Wrap`а статическая переменная
      LastWrapMode имеет не булевое значение, а обычный int
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  04.07.2000 tran
    + не показывать мессаг бакс при невозвожности открыть файл
  28.06.2000 IS
    - Не показывать тип файла для каталогов в "Быстром просмотре"
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "qview.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "lang.hpp"
#include "colors.hpp"
#include "keys.hpp"
#include "global.hpp"
#include "filepanels.hpp"
#include "help.hpp"
#include "viewer.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"

/* $ 12.07.2000 SVS
    ! Для возможности 3-х позиционного Wrap`а статическая переменная
      LastWrapMode имеет не булевое значение, а обычный int
*/
/* $ 20.02.2001 VVM
    ! Врап хранится в 2х переменных. */
static int LastWrapMode = -1;
static int LastWrapType = -1;
/* VVM $ */
/* SVS $ */

QuickView::QuickView()
{
  Type=QVIEW_PANEL;
  QView=NULL;
  *CurFileName=0;
  *CurFileType=0;
  *TempName=0;
  Directory=0;
  PrevMacroMode = -1;
  /* $ 20.02.2001 VVM
    + Проинициализируем режим врап-а */
  if (LastWrapMode < 0) {
    LastWrapMode = Opt.ViOpt.ViewerIsWrap;
    LastWrapType = Opt.ViOpt.ViewerWrap;
  }
  /* VVM $ */
}


QuickView::~QuickView()
{
  CloseFile();
  SetMacroMode(TRUE);
}


void QuickView::DisplayObject()
{
  if (Flags.Check(FSCROBJ_ISREDRAWING))
    return;
  Flags.Set(FSCROBJ_ISREDRAWING);

  char Title[NM];
  if (QView==NULL && !ProcessingPluginCommand)
    CtrlObject->Cp()->GetAnotherPanel(this)->UpdateViewPanel();
  if (QView!=NULL)
    QView->SetPosition(X1+1,Y1+1,X2-1,Y2-3);
  Box(X1,Y1,X2,Y2,COL_PANELBOX,DOUBLE_BOX);
  SetScreen(X1+1,Y1+1,X2-1,Y2-1,' ',COL_PANELTEXT);
  SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
  sprintf(Title," %s ",MSG(MQuickViewTitle));
  TruncStr(Title,X2-X1-3);
  if (*Title)
  {
    GotoXY(X1+(X2-X1+1-strlen(Title))/2,Y1);
    Text(Title);
  }
  DrawSeparator(Y2-2);
  SetColor(COL_PANELTEXT);
  GotoXY(X1+1,Y2-1);
  mprintf("%-*.*s",X2-X1-1,X2-X1-1,PointToName(CurFileName));
  if (*CurFileType)
  {
    char TypeText[sizeof(CurFileType)];
    sprintf(TypeText," %s ",CurFileType);
    TruncStr(TypeText,X2-X1-1);
    SetColor(COL_PANELSELECTEDINFO);
    GotoXY(X1+(X2-X1+1-strlen(TypeText))/2,Y2-2);
    Text(TypeText);
  }
  if (Directory)
  {
    char Msg[NM*2];
    sprintf(Msg,MSG(MQuickViewFolder),CurFileName);
    TruncStr(Msg,X2-X1-4);
    SetColor(COL_PANELTEXT);
    GotoXY(X1+2,Y1+2);
    PrintText(Msg);

    /* $ 01.02.2001 SVS
       В панели "Quick view" добавим инфу про Junction
    */
    if((GetFileAttributes(CurFileName)&FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
    {
      char JuncName[NM*2];
      int ID_Msg, Width;
      if(GetJunctionPointInfo(CurFileName,JuncName,sizeof(JuncName))) //"\??\D:\Junc\Src\"
      {
        if(!strncmp(JuncName+4,"Volume{",7))
        {
          char JuncRoot[NM*2];
          JuncRoot[0]=JuncRoot[1]=0;
          GetPathRootOne(JuncName+4,JuncRoot);
          if(JuncRoot[1] == ':')
            strcpy(JuncName+4,JuncRoot);
          ID_Msg=MQuickViewVolMount;
          Width=20;
        }
        else
        {
          ID_Msg=MQuickViewJunction;
          Width=9;
        }
        sprintf(Msg,MSG(ID_Msg),TruncPathStr(JuncName+4,X2-X1-4-Width));
        //TruncStr(Msg,X2-X1-4);
        SetColor(COL_PANELTEXT);
        GotoXY(X1+2,Y1+3);
        PrintText(Msg);
      }
    }
    /* SVS $ */

    if (Directory==1 || Directory==4)
    {
      char SlackMsg[100];

      GotoXY(X1+2,Y1+4);
      PrintText(MSG(MQuickViewContains));
      GotoXY(X1+2,Y1+6);
      PrintText(MSG(MQuickViewFolders));
      SetColor(COL_PANELINFOTEXT);
      sprintf(Msg,"%d",DirCount);
      PrintText(Msg);
      SetColor(COL_PANELTEXT);
      GotoXY(X1+2,Y1+7);
      PrintText(MSG(MQuickViewFiles));
      SetColor(COL_PANELINFOTEXT);
      sprintf(Msg,"%d",FileCount);
      PrintText(Msg);
      SetColor(COL_PANELTEXT);
      GotoXY(X1+2,Y1+8);
      PrintText(MSG(MQuickViewBytes));
      SetColor(COL_PANELINFOTEXT);
      InsertCommas(FileSize,Msg);
      PrintText(Msg);
      SetColor(COL_PANELTEXT);
      GotoXY(X1+2,Y1+9);
      PrintText(MSG(MQuickViewCompressed));
      SetColor(COL_PANELINFOTEXT);
      InsertCommas(CompressedFileSize,Msg);
      PrintText(Msg);

      SetColor(COL_PANELTEXT);
      GotoXY(X1+2,Y1+10);
      PrintText(MSG(MQuickViewRatio));
      SetColor(COL_PANELINFOTEXT);
      sprintf(SlackMsg,"%d%%",ToPercent64(CompressedFileSize.Number.i64,FileSize.Number.i64));
      PrintText(SlackMsg);

      if (Directory!=4 && RealFileSize>=CompressedFileSize)
      {
        SetColor(COL_PANELTEXT);
        GotoXY(X1+2,Y1+12);
        PrintText(MSG(MQuickViewCluster));
        SetColor(COL_PANELINFOTEXT);
        InsertCommas(ClusterSize,Msg);
        PrintText(Msg);
        SetColor(COL_PANELTEXT);
        GotoXY(X1+2,Y1+13);
        PrintText(MSG(MQuickViewRealSize));
        SetColor(COL_PANELINFOTEXT);
        InsertCommas(RealFileSize,Msg);
        PrintText(Msg);
        SetColor(COL_PANELTEXT);
        GotoXY(X1+2,Y1+14);
        PrintText(MSG(MQuickViewSlack));
        SetColor(COL_PANELINFOTEXT);
        InsertCommas(RealFileSize-CompressedFileSize,Msg);
        int64 Size1=RealFileSize-CompressedFileSize;
        int64 Size2=RealFileSize;
        while (Size2.PHigh()!=0)
        {
          Size1=Size1>>1;
          Size2=Size2>>1;
        }
        sprintf(SlackMsg,"%s (%d%%)",Msg,ToPercent(Size1.PLow(),Size2.PLow()));
        PrintText(SlackMsg);
      }
    }
  }
  else
    if (QView!=NULL)
      QView->Show();
  Flags.Clear(FSCROBJ_ISREDRAWING);
}


int QuickView::ProcessKey(int Key)
{
  if (!IsVisible())
    return(FALSE);

  if(ProcessShortcutFolder(Key,FALSE))
    return(TRUE);

  /* $ 30.04.2001 DJ
     показываем правильный help topic
  */
  if (Key == KEY_F1)
  {
    Help Hlp ("QViewPanel");
    return TRUE;
  }
  /* DJ $ */
  if (Key==KEY_F3 || Key==KEY_NUMPAD5 || Key == KEY_SHIFTNUMPAD5)
  {
    Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
    if (AnotherPanel->GetType()==FILE_PANEL)
      AnotherPanel->ProcessKey(KEY_F3);
    return(TRUE);
  }
  /* $ 04.08.2000 tran
     Gray+, Gray- передвигают курсор на другой панели*/
  if (Key==KEY_ADD || Key==KEY_SUBTRACT)
  {
    Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
    if (AnotherPanel->GetType()==FILE_PANEL)
      AnotherPanel->ProcessKey(Key==KEY_ADD?KEY_DOWN:KEY_UP);
    return(TRUE);
  }
  /* tran 04.08.2000 $ */

  /* $ 30.04.2001 DJ
     обновляем кейбар
  */
  if (QView!=NULL && !Directory && Key>=256)
  {
    int ret = QView->ProcessKey(Key);
    if (Key == KEY_F4 || Key == KEY_F8 || Key == KEY_F2 || Key == KEY_SHIFTF2)
    {
      DynamicUpdateKeyBar();
      CtrlObject->MainKeyBar->Redraw();
    }
    if (Key == KEY_F7 || Key == KEY_SHIFTF7)
    {
      __int64 Pos;
      int Length;
      DWORD Flags;
      QView->GetSelectedParam(Pos,Length,Flags);
      Redraw();
      CtrlObject->Cp()->GetAnotherPanel(this)->Redraw();
      QView->SelectText(Pos,Length,Flags|1);
    }
    return ret;
  }
  return(FALSE);
}


int QuickView::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  int RetCode;
  if (!IsVisible())
    return(FALSE);
  if (Panel::PanelProcessMouse(MouseEvent,RetCode))
    return(RetCode);
  SetFocus();
  if (QView!=NULL && !Directory)
    return(QView->ProcessMouse(MouseEvent));
  return(FALSE);
}

#if defined(__BORLANDC__)
#pragma warn -par
#endif
void QuickView::Update(int Mode)
{
  if (!EnableUpdate)
    return;
  if (*CurFileName==0)
    CtrlObject->Cp()->GetAnotherPanel(this)->UpdateViewPanel();
  Redraw();
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif


void QuickView::ShowFile(char *FileName,int TempFile,HANDLE hDirPlugin)
{
  char *ExtPtr;
  int FileAttr;
  CloseFile();
  QView=NULL;
  *CurFileName=0;
  if (!IsVisible())
    return;
  if (FileName==NULL)
  {
    ProcessingPluginCommand++;
    Show();
    ProcessingPluginCommand--;
    return;
  }
  QView=new Viewer;
  QView->SetRestoreScreenMode(FALSE);
  QView->SetPosition(X1+1,Y1+1,X2-1,Y2-3);
  QView->SetStatusMode(0);
  QView->EnableHideCursor(0);
  /* $ 20.02.2001 VVM
      + Запомнить старое состояние врапа и потом восстановить. */
  OldWrapMode = QView->GetWrapMode();
  OldWrapType = QView->GetWrapType();
  QView->SetWrapMode(LastWrapMode);
  QView->SetWrapType(LastWrapType);
  /* VVM $ */
  strcpy(CurFileName,FileName);

  if ((ExtPtr=strrchr(CurFileName,'.'))!=NULL)
  {
    char Value[80];
    LONG ValueSize=sizeof(Value);
    if (RegQueryValue(HKEY_CLASSES_ROOT,(LPCTSTR)ExtPtr,(LPTSTR)Value,&ValueSize)==ERROR_SUCCESS)
    {
      ValueSize=sizeof(CurFileType);
      if (RegQueryValue(HKEY_CLASSES_ROOT,Value,(LPTSTR)CurFileType,&ValueSize)!=ERROR_SUCCESS)
        *CurFileType=0;
      FAR_CharToOem(CurFileType,CurFileType);
    }
  }
  if (hDirPlugin || (FileAttr=GetFileAttributes(CurFileName))!=-1 && (FileAttr & FA_DIREC))
  {
    /* $ 28.06.2000 IS
     Не показывать тип файла для каталогов в "Быстром просмотре" /
    */
    *CurFileType=0;
    /* IS $ */
    if (hDirPlugin)
    {
      int ExitCode=GetPluginDirInfo(hDirPlugin,CurFileName,DirCount,
                   FileCount,FileSize,CompressedFileSize);
      if (ExitCode)
        Directory=4;
      else
        Directory=3;
    }
    else
    {
      int ExitCode=GetDirInfo(MSG(MQuickViewTitle),CurFileName,DirCount,
                   FileCount,FileSize,CompressedFileSize,RealFileSize,
                   ClusterSize,500,TRUE);
      if (ExitCode==1)
        Directory=1;
      else
        if (ExitCode==-1)
          Directory=2;
        else
          Directory=3;
    }
  }
  else
    if (*CurFileName)
      /* $ 04.07.2000 tran
         + add FALSE as 'warning' parameter*/
      QView->OpenFile(CurFileName,FALSE);
      /* tran 04.07.2000 $ */

  if (TempFile){
//    ConvertNameToFull(CurFileName,TempName, sizeof(TempName));
    if (ConvertNameToFull(CurFileName,TempName, sizeof(TempName)) >= sizeof(TempName)){
      return;
    }
  }
  Redraw();
  /* $ 30.04.2001 DJ
     обновляем кейбар
  */
  if (CtrlObject->Cp()->ActivePanel == this)
  {
    DynamicUpdateKeyBar();
    CtrlObject->MainKeyBar->Redraw();
  }
  /* DJ $ */
}


void QuickView::CloseFile()
{
  if (QView!=NULL)
  {
    /* $ 20.02.2001 VVM
        ! Восстановить старое значение врапа */
    LastWrapMode=QView->GetWrapMode();
    LastWrapType=QView->GetWrapType();
    QView->SetWrapMode(OldWrapMode);
    QView->SetWrapType(OldWrapType);
    /* VVM $ */
    delete QView;
    QView=NULL;
  }
  QViewDelTempName();
  *CurFileType=0;
  Directory=0;
}


void QuickView::QViewDelTempName()
{
  if (*TempName)
  {
    if (QView!=NULL)
    {
      /* $ 20.02.2001 VVM
          ! Восстановить старое значение врапа */
      LastWrapMode=QView->GetWrapMode();
      LastWrapType=QView->GetWrapType();
      QView->SetWrapMode(OldWrapMode);
      QView->SetWrapType(OldWrapType);
      /* VVM $ */
      delete QView;
      QView=NULL;
    }
    chmod(TempName,S_IREAD|S_IWRITE);
    remove(TempName);
    *PointToName(TempName)=0;
    FAR_RemoveDirectory(TempName);
    *TempName=0;
  }
}


void QuickView::PrintText(char *Str)
{
  if (WhereY()>Y2-3 || WhereX()>X2-2)
    return;
  mprintf("%.*s",X2-2-WhereX()+1,Str);
}


int QuickView::UpdateIfChanged(int UpdateMode)
{
  if (IsVisible() && *CurFileName && Directory==2)
  {
    char ViewName[NM+30];
    strcpy(ViewName,CurFileName);
    ShowFile(ViewName,*TempName,NULL);
    return(TRUE);
  }
  return(FALSE);
}

/* $ 20.07.2000 tran
   два метода - установка заголовка*/
void QuickView::SetTitle()
{
  if (GetFocus())
  {
    char TitleDir[NM+30];
    if (*CurFileName)
      sprintf(TitleDir,"{%.*s - QuickView}",NM-1,CurFileName);
    else
    {
      char CmdText[512];
      CtrlObject->CmdLine->GetString(CmdText,sizeof(CmdText));
      sprintf(TitleDir,"{%.*s}",sizeof(TitleDir)-3,CmdText);
    }
    strcpy(LastFarTitle,TitleDir);
    SetFarTitle(TitleDir);
  }
}
// и его показ в случае получения фокуса
void QuickView::SetFocus()
{
  Panel::SetFocus();
  SetTitle();
  SetMacroMode(FALSE);
}
/* tran 20.07.2000 $ */

void QuickView::KillFocus()
{
  Panel::KillFocus();
  SetMacroMode(TRUE);
}

void QuickView::SetMacroMode(int Restore)
{
  if (CtrlObject == NULL)
    return;
  if (PrevMacroMode == -1)
    PrevMacroMode = CtrlObject->Macro.GetMode();
  CtrlObject->Macro.SetMode(Restore ? PrevMacroMode:MACRO_QVIEWPANEL);
}

/* $ 30.04.2001 DJ
   свой кейбар
*/

BOOL QuickView::UpdateKeyBar()
{
  KeyBar *KB = CtrlObject->MainKeyBar;
  KB->SetAllGroup (KBL_MAIN, MQViewF1, 12);
  KB->SetAllGroup (KBL_SHIFT, MQViewShiftF1, 12);
  KB->SetAllGroup (KBL_ALT, MQViewAltF1, 12);
  KB->SetAllGroup (KBL_CTRL, MQViewCtrlF1, 12);
  KB->ClearGroup (KBL_CTRLSHIFT);
  KB->ClearGroup (KBL_CTRLALT);
  KB->ClearGroup (KBL_ALTSHIFT);

  DynamicUpdateKeyBar();

  return TRUE;
}

int QuickView::GetCurName(char *Name,char *ShortName)
{
  if (Name && ShortName && *CurFileName)
  {
    strcpy(Name, CurFileName);
    strcpy(ShortName, Name);
    return (TRUE);
  }
  return (FALSE);
}

void QuickView::DynamicUpdateKeyBar()
{
  KeyBar *KB = CtrlObject->MainKeyBar;
  if (Directory || !QView)
  {
    KB->Change (MSG(MF2), 2-1);
    KB->Change ("", 4-1);
    KB->Change ("", 8-1);
    KB->Change (KBL_SHIFT, "", 2-1);
    KB->Change (KBL_SHIFT, "", 8-1);
    KB->Change (KBL_ALT, MSG(MAltF8), 8-1); // стандартный для панели - "хистори"
  }
  else {
    if (QView->GetHexMode())
      KB->Change (MSG(MViewF4Text), 4-1);
    else
      KB->Change (MSG(MQViewF4), 4-1);

    if (QView->GetAnsiMode())
      KB->Change (MSG(MViewF8DOS), 8-1);
    else
      KB->Change (MSG(MQViewF8), 8-1);

    if (!QView->GetWrapMode())
    {
      if (QView->GetWrapType())
        KB->Change (MSG(MViewShiftF2), 2-1);
      else
        KB->Change (MSG(MViewF2), 2-1);
    }
    else
      KB->Change (MSG(MViewF2Unwrap), 2-1);

    if (QView->GetWrapType())
      KB->Change (KBL_SHIFT, MSG(MViewF2), 2-1);
    else
      KB->Change (KBL_SHIFT, MSG(MViewShiftF2), 2-1);
  }
}

/* DJ $ */
