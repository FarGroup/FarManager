/*
infolist.cpp

Информационная панель

*/

/* Revision: 1.41 08.06.2004 $ */

/*
Modify:
  08.06.2004 SVS
    ! Вместо GetDriveType теперь вызываем FAR_GetDriveType().
    ! Вместо "DriveType==DRIVE_CDROM" вызываем IsDriveTypeCDROM()
    + добавка типа CD-привода
  01.03.2004 SVS
    ! Обертки FAR_OemTo* и FAR_CharTo* вокруг одноименных WinAPI-функций
      (задел на будущее + править впоследствии только 1 файл)
  26.02.2003 SVS
    ! вместо ShellUpdatePanels() исполним Redraw()
  25.02.2003 SVS
    - BugZ#805 - Остается прорисовка после поиска в Qview & InfoPanel
  14.01.2003 SVS
    - BugZ#758 - падение FAR'a при запуске ассоциации
  01.10.2002 SVS
    - BugZ#662 - descript.ion
  24.05.2002 SVS
    + Дублирование Numpad-клавиш
  27.04.2002 SVS
    ! 8192 -> MAXSIZE_SHORTCUTDATA
  25.04.2002 IS
    ! внедрение const
    - Баг в PrintInfo: меняли то, что иногда менять нельзя
  26.03.2002 DJ
    ! реализация Update() перенесена из .h; перерисовка делается, только если
      мы активный фрейм
  22.03.2002 SVS
    - strcpy - Fuck!
  20.03.2002 SVS
    ! GetCurrentDirectory -> FarGetCurDir
  16.01.2002 SVS
    ! Косметика в кейбаре
  14.01.2002 IS
    ! chdir -> FarChDir
  02.01.2002 IS
    + GetCurName
  08.12.2001 IS
    - баг: после показа хелпа по панели показывалось непонятно зачем
      еще и Contents
  01.11.2001 SVS
    ! небольшая оптимизация.
  12.10.2001 SKV
    - фикс падения при goto в dizview.
  25.06.2001 IS
    ! Внедрение const
  29.05.2001 tran
    + в processMouseInput при отсутсвующем описании DizView был равен 0
  23.05.2001 OT
    - Выпрямление логики вызовов в NFZ
  16.05.2001 DJ
    ! proof-of-concept
  15.05.2001 OT
    ! NWZ -> NFZ
  06.05.2001 DJ
    ! перетрях #include
  06.05.2001 ОТ
    ! Переименование Window в Frame :)
  05.05.2001 DJ
    + перетрях NWZ
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  30.04.2001 DJ
    + UpdateKeyBar()
    - еще немного всяких фиксов (F1, редактирование описаний)
    ! вместо CloseDizFile() используется виртуальный CloseFile()
  28.04.2001 VVM
    + GetSubstName() принимает тип носителя
  27.04.2001 DJ
    - всякие фиксы про скроллбар в viewer'е для просмотра описаний
  05.04.2001 VVM
    + Переключение макросов в режим MACRO_INFOPANEL
  03.04.2001 VVM
    + Используется Viewer для просмотра описаний.
  02.04.2001 SVS
    ! DRIVE_SUSTITUTE -> DRIVE_SUBSTITUTE
  29.03.2001 IS
    ! Opt.ViewerAutoDetectTable -> Opt.ViOpt.AutoDetectTable
  27.02.2001 VVM
    ! Символы, зависимые от кодовой страницы
      /[\x01-\x08\x0B-\x0C\x0E-\x1F\xB0-\xDF\xF8-\xFF]/
      переведены в коды.
  01.02.2001 SVS
    + В Win2K корректно отображать инфу при заходе в Juction каталог
      Здесь Рут-диск может быть другим
  26.01.2001 SVS
    + Информация о SUBST-дисках - указание пути - на манер ремотных дисков
  05.01.2001 SVS
    + Покажем инфу, что ЭТО - SUBST-диск
  20.07.2000 tran
    - артефакт при f3 (при наличии dirinfo)
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "infolist.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "colors.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "help.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "manager.hpp"

static int LastDizWrapMode = -1;
static int LastDizWrapType = -1;
/* $ 27.04.2001 DJ
   запоминаем, был ли включен скроллбар
*/
static int LastDizShowScrollbar = -1;
/* DJ $ */

InfoList::InfoList()
{
  Type=INFO_PANEL;
  DizView=NULL;
  PrevMacroMode=-1;
  DizPresent=FALSE;
  *DizFileName=0;
  if (LastDizWrapMode < 0)
  {
    LastDizWrapMode = Opt.ViewerIsWrap;
    LastDizWrapType = Opt.ViewerWrap;
    /* $ 27.04.2001 DJ
       запоминаем, был ли включен скроллбар
    */
    LastDizShowScrollbar = Opt.ViOpt.ShowScrollbar;
    /* DJ $ */
  }
}

InfoList::~InfoList()
{
  /* $ 30.04.2001 DJ
     CloseDizFile() -> CloseFile()
  */
  CloseFile();
  /* DJ $ */
  SetMacroMode(TRUE);
}

/* $ 26.03.2002 DJ
   перерисовка, только если мы текущий фрейм
*/
void InfoList::Update (int Mode)
{
  if (!EnableUpdate)
    return;
  if (CtrlObject->Cp() == FrameManager->GetCurrentFrame())
    Redraw();
}
/* DJ $ */

void InfoList::DisplayObject()
{
  char Title[NM],OutStr[200];
  Panel *AnotherPanel;
  char DriveRoot[NM],VolumeName[NM],FileSystemName[NM];
  DWORD MaxNameLength,FileSystemFlags,VolumeNumber;
  /* $ 30.04.2001 DJ
     CloseDizFile() -> CloseFile()
  */
  CloseFile();
  /* DJ $ */
  Box(X1,Y1,X2,Y2,COL_PANELBOX,DOUBLE_BOX);
  SetScreen(X1+1,Y1+1,X2-1,Y2-1,' ',COL_PANELTEXT);
  SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
  sprintf(Title," %s ",MSG(MInfoTitle));
  TruncStr(Title,X2-X1-3);
  if (*Title)
  {
    GotoXY(X1+(X2-X1+1-strlen(Title))/2,Y1);
    Text(Title);
  }

  DrawSeparator(Y1+3);
  DrawSeparator(Y1+8);

  SetColor(COL_PANELTEXT);
  {
    char ComputerName[40],UserName[40];
    DWORD ComputerNameSize=sizeof(ComputerName),UserNameSize=sizeof(UserName);
    *ComputerName=*UserName=0;
    GetComputerName(ComputerName,&ComputerNameSize);
    GetUserName(UserName,&UserNameSize);
    FAR_CharToOem(ComputerName,ComputerName);
    FAR_CharToOem(UserName,UserName);
    GotoXY(X1+2,Y1+1);
    PrintText(MInfoCompName);
    PrintInfo(ComputerName);
    GotoXY(X1+2,Y1+2);
    PrintText(MInfoUserName);
    PrintInfo(UserName);
  }

  AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  AnotherPanel->GetCurDir(CurDir);
  if (*CurDir==0)
    FarGetCurDir(sizeof(CurDir),CurDir);

  /* $ 01.02.2001 SVS
     В Win2K корректно отображать инфу при заходе в Juction каталог
     Здесь Рут-диск может быть другим
  */
  if((GetFileAttributes(CurDir)&FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
  {
    char JuncName[NM];
    if(GetJunctionPointInfo(CurDir,JuncName,sizeof(JuncName)))
      GetPathRoot(JuncName+4,DriveRoot); //"\??\D:\Junc\Src\"
  }
  else
   GetPathRoot(CurDir,DriveRoot);
  /* SVS $ */

  if (GetVolumeInformation(DriveRoot,VolumeName,sizeof(VolumeName),
                            &VolumeNumber,&MaxNameLength,&FileSystemFlags,
                            FileSystemName,sizeof(FileSystemName)))
  {
    char LocalName[8], DiskType[100], RemoteName[NM], DiskName[NM];
    int ShowRealPath=FALSE;
    int DriveType=FAR_GetDriveType(DriveRoot);
    sprintf(LocalName,"%c:",*DriveRoot);

    if (DriveRoot[0] && DriveRoot[1]==':')
      sprintf(DiskName,"%c:",toupper(*DriveRoot));
    else
      strcpy(DiskName,DriveRoot);

    int IdxMsgID=-1;
    *DiskType=0;
    switch(DriveType)
    {
      case DRIVE_REMOVABLE:
        IdxMsgID=MInfoRemovable;
        break;
      case DRIVE_FIXED:
        IdxMsgID=MInfoFixed;
        break;
      case DRIVE_REMOTE:
        IdxMsgID=MInfoNetwork;
        break;
      case DRIVE_CDROM:
        IdxMsgID=MInfoCDROM;
        break;
      case DRIVE_RAMDISK:
        IdxMsgID=MInfoRAM;
        break;
      default:
        if(IsDriveTypeCDROM(DriveType))
          IdxMsgID=DriveType-DRIVE_CD_RW+MInfoCD_RW;
        else
          *DiskType=0;
        break;
    }
    if(IdxMsgID != -1)
      strcpy(DiskType,MSG(IdxMsgID));
    /* 05.01.2001 SVS
       + Информация про Subst-тип диска
    */
    {
      if(GetSubstName(DriveType,LocalName,RemoteName,sizeof(RemoteName)))
      {
        strcpy(DiskType,MSG(MInfoSUBST));
        DriveType=DRIVE_SUBSTITUTE;
      }
    }
    /* SVS $ */

    sprintf(Title," %s %s %s (%s) ",DiskType,MSG(MInfoDisk),DiskName,FileSystemName);

    if (DriveType==DRIVE_REMOTE)
    {
      DWORD RemoteNameSize=sizeof(RemoteName);
      if (WNetGetConnection(LocalName,RemoteName,&RemoteNameSize)==NO_ERROR)
        ShowRealPath=TRUE;
    }
    else if(DriveType == DRIVE_SUBSTITUTE)
        ShowRealPath=TRUE;

    if(ShowRealPath)
    {
      FAR_CharToOem(RemoteName,RemoteName);
      strcat(Title,RemoteName);
      strcat(Title," ");
    }

    TruncStr(Title,X2-X1-3);
    GotoXY(X1+(X2-X1+1-strlen(Title))/2,Y1+3);
    PrintText(Title);

    int64 TotalSize,TotalFree,UserFree;
    if (GetDiskSize(DriveRoot,&TotalSize,&TotalFree,&UserFree))
    {
      GotoXY(X1+2,Y1+4);
      PrintText(MInfoDiskTotal);
      InsertCommas(TotalSize,OutStr);
      PrintInfo(OutStr);
      GotoXY(X1+2,Y1+5);
      PrintText(MInfoDiskFree);
      InsertCommas(UserFree,OutStr);
      PrintInfo(OutStr);
    }

    GotoXY(X1+2,Y1+6);
    PrintText(MInfoDiskLabel);
    PrintInfo(VolumeName);
    GotoXY(X1+2,Y1+7);
    PrintText(MInfoDiskNumber);
    sprintf(OutStr,"%04X-%04X",VolumeNumber>>16,VolumeNumber & 0xffff);
    PrintInfo(OutStr);
  }

  MEMORYSTATUS ms;
  strcpy(Title,MSG(MInfoMemory));
  GotoXY(X1+(X2-X1-strlen(Title))/2,Y1+8);
  PrintText(Title);
  ms.dwLength=sizeof(ms);
  GlobalMemoryStatus(&ms);
  if (ms.dwMemoryLoad==0)
    ms.dwMemoryLoad=100-ToPercent(ms.dwAvailPhys+ms.dwAvailPageFile,ms.dwTotalPhys+ms.dwTotalPageFile);
  GotoXY(X1+2,Y1+9);
  PrintText(MInfoMemoryLoad);
  sprintf(OutStr,"%d%%",ms.dwMemoryLoad);
  PrintInfo(OutStr);
  GotoXY(X1+2,Y1+10);
  PrintText(MInfoMemoryTotal);
  InsertCommas(ms.dwTotalPhys,OutStr);
  PrintInfo(OutStr);
  GotoXY(X1+2,Y1+11);
  PrintText(MInfoMemoryFree);
  InsertCommas(ms.dwAvailPhys,OutStr);
  PrintInfo(OutStr);
  GotoXY(X1+2,Y1+12);
  PrintText(MInfoVirtualTotal);
  InsertCommas(ms.dwTotalPageFile,OutStr);
  PrintInfo(OutStr);
  GotoXY(X1+2,Y1+13);
  PrintText(MInfoVirtualFree);
  InsertCommas(ms.dwAvailPageFile,OutStr);
  PrintInfo(OutStr);
  ShowDirDescription();
  ShowPluginDescription();
}


int InfoList::ProcessKey(int Key)
{
  if (!IsVisible())
    return(FALSE);

  {
    char ShortcutFolder[NM],PluginModule[NM],PluginFile[NM],PluginData[MAXSIZE_SHORTCUTDATA];
    if (GetShortcutFolder(Key,ShortcutFolder,PluginModule,PluginFile,PluginData))
    {
      Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
      if (AnotherPanel->GetType()==FILE_PANEL && *PluginModule==0)
      {
        AnotherPanel->SetCurDir(ShortcutFolder,TRUE);
        AnotherPanel->Redraw();
      }
      return(TRUE);
    }
  }
  switch(Key)
  {
    /* $ 30.04.2001 DJ
       показываем правильную тему хелпа
    */
    case KEY_F1:
      {
        Help Hlp ("InfoPanel");
      }
      return TRUE;
    /* DJ $ */
    case KEY_F3:
    case KEY_NUMPAD5:  case KEY_SHIFTNUMPAD5:
      if (*DizFileName)
      {
        CtrlObject->Cp()->GetAnotherPanel(this)->GetCurDir(CurDir);
        FarChDir(CurDir);
        new FileViewer(DizFileName,TRUE);//OT
      }
      /* $ 20.07.2000 tran
         после показа перерисовываем панели */
      CtrlObject->Cp()->Redraw();
      /* tran 20.07.2000 $ */
      return(TRUE);
    case KEY_F4:
      /* $ 30.04.2001 DJ
         не показываем редактор, если ничего не задано в именах файлов;
         не редактируем имена описаний со звездочками;
         убираем лишнюю перерисовку панелей
      */
      {
        Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
        AnotherPanel->GetCurDir(CurDir);
        FarChDir(CurDir);
        if (*DizFileName)
          new FileEditor(DizFileName,FALSE,TRUE);
        else if (*Opt.FolderInfoFiles)
        {
          char ArgName[NM];
          const char *p = Opt.FolderInfoFiles;
          while ((p = GetCommaWord(p,ArgName)) != NULL)
          {
            if (!strpbrk (ArgName, "*?"))
            {
              new FileEditor(ArgName,TRUE,TRUE);
              break;
            }
          }
        }
        AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
//        AnotherPanel->Redraw();
        Update(0);
      }
      /* DJ $ */
      /* $ 20.07.2000 tran
         после показа перерисовываем панели */
      CtrlObject->Cp()->Redraw();
      /* tran 20.07.2000 $ */
      return(TRUE);
    case KEY_CTRLR:
      Redraw();
      return(TRUE);
  }
  /* $ 30.04.2001 DJ
     обновляем кейбар после нажатия F8, F2 или Shift-F2
  */
  if (DizView!=NULL && Key>=256)
  {
    int ret = DizView->ProcessKey(Key);
    if (Key == KEY_F8 || Key == KEY_F2 || Key == KEY_SHIFTF2)
    {
      DynamicUpdateKeyBar();
      CtrlObject->MainKeyBar->Redraw();
    }
    if (Key == KEY_F7 || Key == KEY_SHIFTF7)
    {
      __int64 Pos;
      int Length;
      DWORD Flags;
      DizView->GetSelectedParam(Pos,Length,Flags);
//      ShellUpdatePanels(NULL,FALSE);
      Redraw();
      CtrlObject->Cp()->GetAnotherPanel(this)->Redraw();
      DizView->SelectText(Pos,Length,Flags|1);
    }
    return(ret);
  }
  /* DJ $ */
  return(FALSE);
}


int InfoList::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  int RetCode;
  if (Panel::PanelProcessMouse(MouseEvent,RetCode))
    return(RetCode);

  /* $ 29.05.2001 tran
     DizView может быть равен 0 */
  if (MouseEvent->dwMousePosition.Y>=14 && DizView!=NULL)
  {
    /* $ 27.04.2001 DJ
       позволяем использовать скроллбар, если он включен
    */
    _tran(SysLog("InfoList::ProcessMouse() DizView = %p",DizView));
    /* $ 12.10.2001 SKV
      одноко аккуратно посчитаем окошко DizView,
      и оставим 2 символа на скроллинг мышой.
    */
    int DVX1,DVX2,DVY1,DVY2;
    DizView->GetPosition(DVX1,DVY1,DVX2,DVY2);
    if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
        MouseEvent->dwMousePosition.X > DVX1+1 &&
        MouseEvent->dwMousePosition.X < DVX2 - DizView->GetShowScrollbar() - 1 &&
        MouseEvent->dwMousePosition.Y > DVY1+1 &&
        MouseEvent->dwMousePosition.Y < DVY2-1
        )
    {
      ProcessKey(KEY_F3);
      return(TRUE);
    }
    /* SKV$*/
    /* DJ $ */
    if (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED)
    {
      ProcessKey(KEY_F4);
      return(TRUE);
    }
  }

  SetFocus();
  if (DizView!=NULL)
    return(DizView->ProcessMouse(MouseEvent));

  return(TRUE);
}


void InfoList::PrintText(const char *Str)
{
  if (WhereY()>Y2-1)
    return;
  mprintf("%.*s",X2-WhereX(),Str);
}


void InfoList::PrintText(int MsgID)
{
  PrintText(MSG(MsgID));
}


void InfoList::PrintInfo(const char *str)
{
  if (WhereY()>Y2-1)
    return;
  int SaveColor=GetColor(),MaxLength=X2-WhereX()-2;
  if (MaxLength<0)
    MaxLength=0;
  /* $ 25.04.2002 IS
     Скопируем данные во внутренний буфер, т.к. TruncStr изменяет их, а этого
     делать нельзя, т.к. str может указывать и на r/o-строку
  */
  char Str[NM*2];
  strncpy(Str,str,sizeof(Str));
  /* IS $ */
  TruncStr(Str,MaxLength);
  int Length=strlen(Str);
  int NewX=X2-Length-1;
  if (NewX>X1 && NewX>WhereX())
  {
    GotoXY(NewX,WhereY());
    SetColor(COL_PANELINFOTEXT);
    mprintf("%s ",Str);
    SetColor(SaveColor);
  }
}


void InfoList::PrintInfo(int MsgID)
{
  PrintInfo(MSG(MsgID));
}


void InfoList::ShowDirDescription()
{
  char DizDir[NM];
  int Length;
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  DrawSeparator(Y1+14);
  if (AnotherPanel->GetMode()!=FILE_PANEL)
  {
    SetColor(COL_PANELTEXT);
    GotoXY(X1+2,Y1+15);
    PrintText(MInfoDizAbsent);
    return;
  }
  AnotherPanel->GetCurDir(DizDir);
  if ((Length=strlen(DizDir))>0 && DizDir[Length-1]!='\\')
    strcat(DizDir,"\\");

  char ArgName[NM];
  const char *NamePtr=Opt.FolderInfoFiles;
  while ((NamePtr=GetCommaWord(NamePtr,ArgName))!=NULL)
  {
    char FullDizName[2048];
    strncpy(FullDizName,DizDir,sizeof(FullDizName)-1);
    if(strlen(FullDizName)+strlen(ArgName) < sizeof(FullDizName))
      strcat(FullDizName,ArgName);
    else
      return;

    HANDLE FindHandle;
    WIN32_FIND_DATA FindData;
    FindHandle=FindFirstFile(FullDizName,&FindData);
    if (FindHandle==INVALID_HANDLE_VALUE)
      continue;
    FindClose(FindHandle);
    strcpy(PointToName(FullDizName),FindData.cFileName);
    if (OpenDizFile(FullDizName))
      return;
  }
  /* $ 30.04.2001 DJ
     CloseDizFile() -> CloseFile()
  */
  CloseFile();
  /* DJ $ */
  SetColor(COL_PANELTEXT);
  GotoXY(X1+2,Y1+15);
  PrintText(MInfoDizAbsent);
}


void InfoList::ShowPluginDescription()
{
  Panel *AnotherPanel;
  static char VertcalLine[2]={0xBA,0x00};
  AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  if (AnotherPanel->GetMode()!=PLUGIN_PANEL)
    return;
  /* $ 30.04.2001 DJ
     CloseDizFile() -> CloseFile()
  */
  CloseFile();
  /* DJ $ */
  struct OpenPluginInfo Info;
  AnotherPanel->GetOpenPluginInfo(&Info);
  for (int I=0;I<Info.InfoLinesNumber;I++)
  {
    int Y=Y2-Info.InfoLinesNumber+I;
    if (Y<=Y1)
      continue;
    const struct InfoPanelLine *InfoLine=&Info.InfoLines[I];
    GotoXY(X1,Y);
    SetColor(COL_PANELBOX);
    Text(VertcalLine);
    SetColor(COL_PANELTEXT);
    mprintf("%*s",X2-X1-1,"");
    SetColor(COL_PANELBOX);
    Text(VertcalLine);
    GotoXY(X1+2,Y);
    if (InfoLine->Separator)
    {
      char Title[200];
      if (InfoLine->Text!=NULL && *InfoLine->Text)
        sprintf(Title," %s ",InfoLine->Text);
      else
        *Title=0;
      DrawSeparator(Y);
      TruncStr(Title,X2-X1-3);
      GotoXY(X1+(X2-X1-strlen(Title))/2,Y);
      PrintText(Title);
    }
    else
    {
      PrintText(NullToEmpty(InfoLine->Text));
      PrintInfo(NullToEmpty(InfoLine->Data));
    }
  }
}

/* $ 30.04.2001 DJ
   CloseDizFile() -> CloseFile()
*/
void InfoList::CloseFile()
{
  if (DizView!=NULL)
  {
    /* $ 12.10.2001 SKV
      Если идёт вызов метода DizView,
      то не надо делать delete...
    */
    if(DizView->InRecursion)return;
    /* SKV$*/
    LastDizWrapMode=DizView->GetWrapMode();
    LastDizWrapType=DizView->GetWrapType();
    /* $ 27.04.2001 DJ
       запоминаем, был ли включен скроллбар
    */
    LastDizShowScrollbar=DizView->GetShowScrollbar();
    /* DJ $ */
    DizView->SetWrapMode(OldWrapMode);
    DizView->SetWrapType(OldWrapType);
    delete DizView;
    DizView=NULL;
  }
  *DizFileName=0;
}
/* DJ $ */

int InfoList::OpenDizFile(char *DizFile)
{
  _tran(SysLog("InfoList::OpenDizFile([%s]",DizFile));
  if (DizView == NULL)
  {
    /* $ 12.10.2001 SKV
      Теперь это не просто Viewer, а DizViewer :)
    */
    DizView=new DizViewer;
    /* SKV$*/
    _tran(SysLog("InfoList::OpenDizFile() create new Viewer = %p",DizView));
    DizView->SetRestoreScreenMode(FALSE);
    DizView->SetPosition(X1+1,Y1+15,X2-1,Y2-1);
    DizView->SetStatusMode(0);
    DizView->EnableHideCursor(0);
    OldWrapMode = DizView->GetWrapMode();
    OldWrapType = DizView->GetWrapType();
    DizView->SetWrapMode(LastDizWrapMode);
    DizView->SetWrapType(LastDizWrapType);
    /* $ 27.04.2001 DJ
       если скроллбар был включен, включаем
    */
    DizView->SetShowScrollbar (LastDizShowScrollbar);
    /* DJ $ */
  }
  if (!DizView->OpenFile(DizFile,FALSE))
  {
    DizPresent=FALSE;
    return(FALSE);
  }
  DizView->Show();
  strcpy(DizFileName,DizFile);
  char Title[NM];
  sprintf(Title," %s ",PointToName(DizFileName));
  TruncStr(Title,X2-X1-3);
  GotoXY(X1+(X2-X1-strlen(Title))/2,Y1+14);
  SetColor(COL_PANELTEXT);
  PrintText(Title);
  DizPresent=TRUE;
  return(TRUE);
}

void InfoList::SetFocus()
{
  Panel::SetFocus();
  SetMacroMode(FALSE);
}

void InfoList::KillFocus()
{
  Panel::KillFocus();
  SetMacroMode(TRUE);
}

void InfoList::SetMacroMode(int Restore)
{
  if (CtrlObject == NULL)
    return;
  if (PrevMacroMode == -1)
    PrevMacroMode = CtrlObject->Macro.GetMode();
  CtrlObject->Macro.SetMode(Restore ? PrevMacroMode:MACRO_INFOPANEL);
}

/* $ 02.01.2002 IS дадим информацию об имени просматриваемого diz-файла
   Если возвращается пустая строка, то значит, что в текущий момент
   diz-файл не просматривается.
*/
int InfoList::GetCurName(char *Name,char *ShortName)
{
  if (Name && ShortName)
  {
    strcpy(Name, DizFileName);
    ConvertNameToShort(DizFileName,ShortName);
    return (TRUE);
  }
  return (FALSE);
}
/* IS $ */
/* $ 30.04.2001 DJ
   свой кейбар
*/

BOOL InfoList::UpdateKeyBar()
{
  KeyBar *KB = CtrlObject->MainKeyBar;
  KB->SetAllGroup (KBL_MAIN, MInfoF1, 12);
  KB->SetAllGroup (KBL_SHIFT, MInfoShiftF1, 12);
  KB->SetAllGroup (KBL_ALT, MInfoAltF1, 12);
  KB->SetAllGroup (KBL_CTRL, MInfoCtrlF1, 12);
  KB->ClearGroup (KBL_CTRLSHIFT);
  KB->ClearGroup (KBL_CTRLALT);
  KB->ClearGroup (KBL_ALTSHIFT);
  DynamicUpdateKeyBar();

  return TRUE;
}

void InfoList::DynamicUpdateKeyBar()
{
  KeyBar *KB = CtrlObject->MainKeyBar;
  if (DizView && DizPresent)
  {
    KB->Change (MSG(MInfoF3), 3-1);

    if (DizView->GetAnsiMode())
      KB->Change (MSG(MViewF8DOS), 7);
    else
      KB->Change (MSG(MInfoF8), 7);

    if (!DizView->GetWrapMode())
    {
      if (DizView->GetWrapType())
        KB->Change (MSG(MViewShiftF2), 2-1);
      else
        KB->Change (MSG(MViewF2), 2-1);
    }
    else
      KB->Change (MSG(MViewF2Unwrap), 2-1);

    if (DizView->GetWrapType())
      KB->Change (KBL_SHIFT, MSG(MViewF2), 2-1);
    else
      KB->Change (KBL_SHIFT, MSG(MViewShiftF2), 2-1);
  }
  else
  {
    KB->Change (MSG(MF2), 2-1);
    KB->Change (KBL_SHIFT, "", 2-1);
    KB->Change ("", 3-1);
    KB->Change ("", 8-1);
    KB->Change (KBL_SHIFT, "", 8-1);
    KB->Change (KBL_ALT, MSG(MAltF8), 8-1); // стандартный для панели - "хистори"
  }
}
/* DJ $ */
