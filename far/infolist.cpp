/*
infolist.cpp

Информационная панель

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
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

InfoList::InfoList()
{
  Type=INFO_PANEL;
  *DizFileName=0;
}


void InfoList::DisplayObject()
{
  char Title[NM],OutStr[200];
  Panel *AnotherPanel;
  char DriveRoot[NM],VolumeName[NM],FileSystemName[NM];
  DWORD MaxNameLength,FileSystemFlags,VolumeNumber;
  *DizFileName=0;
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
    CharToOem(ComputerName,ComputerName);
    CharToOem(UserName,UserName);
    GotoXY(X1+2,Y1+1);
    PrintText(MInfoCompName);
    PrintInfo(ComputerName);
    GotoXY(X1+2,Y1+2);
    PrintText(MInfoUserName);
    PrintInfo(UserName);
  }

  AnotherPanel=CtrlObject->GetAnotherPanel(this);
  AnotherPanel->GetCurDir(CurDir);

  if (*CurDir==0)
    GetCurrentDirectory(sizeof(CurDir),CurDir);

  GetPathRoot(CurDir,DriveRoot);

  if (GetVolumeInformation(DriveRoot,VolumeName,sizeof(VolumeName),
                            &VolumeNumber,&MaxNameLength,&FileSystemFlags,
                            FileSystemName,sizeof(FileSystemName)))
  {
    char DiskType[100];
    int DriveType=GetDriveType(DriveRoot);
    switch(DriveType)
    {
      case DRIVE_REMOVABLE:
        strcpy(DiskType,MSG(MInfoRemovable));
        break;
      case DRIVE_FIXED:
        strcpy(DiskType,MSG(MInfoFixed));
        break;
      case DRIVE_REMOTE:
        strcpy(DiskType,MSG(MInfoNetwork));
        break;
      case DRIVE_CDROM:
        strcpy(DiskType,MSG(MInfoCDROM));
        break;
      case DRIVE_RAMDISK:
        strcpy(DiskType,MSG(MInfoRAM));
        break;
      default:
        *DiskType=0;
        break;
    }
    char DiskName[NM];
    if (DriveRoot[0] && DriveRoot[1]==':')
      sprintf(DiskName,"%c:",toupper(*DriveRoot));
    else
      strcpy(DiskName,DriveRoot);
    sprintf(Title," %s %s %s (%s) ",DiskType,MSG(MInfoDisk),DiskName,FileSystemName);
    if (DriveType==DRIVE_REMOTE)
    {
      char LocalName[NM],RemoteName[NM];
      DWORD RemoteNameSize=sizeof(RemoteName);
      sprintf(LocalName,"%c:",*DriveRoot);

      if (WNetGetConnection(LocalName,RemoteName,&RemoteNameSize)==NO_ERROR)
      {
        CharToOem(RemoteName,RemoteName);
        strcat(Title,RemoteName);
        strcat(Title," ");
      }
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
  {
    char ShortcutFolder[NM],PluginModule[NM],PluginFile[NM],PluginData[8192];
    if (GetShortcutFolder(Key,ShortcutFolder,PluginModule,PluginFile,PluginData))
    {
      Panel *AnotherPanel=CtrlObject->GetAnotherPanel(this);
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
    case KEY_F3:
    case KEY_NUMPAD5:
      if (*DizFileName)
      {
        CtrlObject->GetAnotherPanel(this)->GetCurDir(CurDir);
        chdir(CurDir);
        FileViewer *ShellViewer=new FileViewer(DizFileName,TRUE);
        CtrlObject->ModalManager.AddModal(ShellViewer);
      }
      return(TRUE);
    case KEY_F4:
      CtrlObject->GetAnotherPanel(this)->GetCurDir(CurDir);
      chdir(CurDir);
      if (*DizFileName)
      {
        FileEditor *ShellEditor=new FileEditor(DizFileName,FALSE,TRUE);
        CtrlObject->ModalManager.AddModal(ShellEditor);
      }
      else
      {
        char ArgName[NM];
        GetCommaWord(Opt.FolderInfoFiles,ArgName);
        FileEditor *ShellEditor=new FileEditor(ArgName,TRUE,TRUE);
        CtrlObject->ModalManager.AddModal(ShellEditor);
      }
      {
        Panel *AnotherPanel=CtrlObject->GetAnotherPanel(this);
        AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
        AnotherPanel->Redraw();
        Update(0);
      }
      return(TRUE);
    case KEY_CTRLR:
      Redraw();
      return(TRUE);
  }
  return(FALSE);
}


int InfoList::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  int RetCode;
  if (Panel::PanelProcessMouse(MouseEvent,RetCode))
    return(RetCode);

  if (MouseEvent->dwMousePosition.Y>=14)
  {
    if (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
    {
      ProcessKey(KEY_F3);
      return(TRUE);
    }
    if (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED)
    {
      ProcessKey(KEY_F4);
      return(TRUE);
    }
  }

  SetFocus();
  return(TRUE);
}


void InfoList::PrintText(char *Str)
{
  if (WhereY()>Y2-1)
    return;
  mprintf("%.*s",X2-WhereX(),Str);
}


void InfoList::PrintText(int MsgID)
{
  PrintText(MSG(MsgID));
}


void InfoList::PrintInfo(char *Str)
{
  if (WhereY()>Y2-1)
    return;
  int SaveColor=GetColor(),MaxLength=X2-WhereX()-2;
  if (MaxLength<0)
    MaxLength=0;
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
  char Title[NM],DizStr[100],DizDir[NM];
  FILE *DizFile;
  int I,Length;
  Panel *AnotherPanel=CtrlObject->GetAnotherPanel(this);
  DrawSeparator(Y1+14);
  AnotherPanel->GetCurDir(DizDir);
  if ((Length=strlen(DizDir))>0 && DizDir[Length-1]!='\\')
    strcat(DizDir,"\\");

  char ArgName[NM],*NamePtr=Opt.FolderInfoFiles;
  while ((NamePtr=GetCommaWord(NamePtr,ArgName))!=NULL)
  {
    char FullDizName[NM];
    strcpy(FullDizName,DizDir);
    strcat(FullDizName,ArgName);

    HANDLE FindHandle;
    WIN32_FIND_DATA FindData;
    FindHandle=FindFirstFile(FullDizName,&FindData);
    if (FindHandle==INVALID_HANDLE_VALUE)
      continue;
    FindClose(FindHandle);
    strcpy(PointToName(FullDizName),FindData.cFileName);
    if ((DizFile=fopen(FullDizName,"r"))!=NULL)
    {
      struct CharTableSet TableSet;
      int TableNum;
      int UseDecodeTable=FALSE;
      if (Opt.ViewerAutoDetectTable)
        UseDecodeTable=DetectTable(DizFile,&TableSet,TableNum);
      strcpy(DizFileName,FullDizName);
      sprintf(Title," %s ",PointToName(DizFileName));
      TruncStr(Title,X2-X1-3);
      GotoXY(X1+(X2-X1-strlen(Title))/2,Y1+14);
      SetColor(COL_PANELTEXT);
      PrintText(Title);
      for (I=15;I<Y2;I++)
      {
        if (fgets(DizStr,sizeof(DizStr),DizFile)==NULL)
          break;
        RemoveTrailingSpaces(DizStr);
        for (int K=0;DizStr[K]!=0;K++)
          if (DizStr[K]=='\t')
            DizStr[K]=' ';
        if (UseDecodeTable)
          DecodeString(DizStr,TableSet.DecodeTable);
        GotoXY(X1+1,Y1+I);
        PrintText(DizStr);
      }

      fclose(DizFile);
      return;
    }
  }
  SetColor(COL_PANELTEXT);
  GotoXY(X1+2,Y1+15);
  PrintText(MInfoDizAbsent);
}


void InfoList::ShowPluginDescription()
{
  Panel *AnotherPanel;
  AnotherPanel=CtrlObject->GetAnotherPanel(this);
  if (AnotherPanel->GetMode()!=PLUGIN_PANEL)
    return;
  struct OpenPluginInfo Info;
  AnotherPanel->GetOpenPluginInfo(&Info);
  for (int I=0;I<Info.InfoLinesNumber;I++)
  {
    int Y=Y2-Info.InfoLinesNumber+I;
    if (Y<=Y1)
      continue;
    struct InfoPanelLine *InfoLine=&Info.InfoLines[I];
    GotoXY(X1,Y);
    SetColor(COL_PANELBOX);
    Text("║");
    SetColor(COL_PANELTEXT);
    mprintf("%*s",X2-X1-1,"");
    SetColor(COL_PANELBOX);
    Text("║");
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

