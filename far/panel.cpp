/*
panel.cpp

Parent class для панелей

*/

/* Revision: 1.14 11.02.2001 $ */

/*
Modify:
  11.02.2001 SVS
    ! Несколько уточнений кода в связи с изменениями в структуре MenuItem
  22.01.2001 VVM
    - Порядок сортировки задается аналогично StartSortOrder
  09.01.2001 SVS
    - Для KEY_XXX_BASE нужно прибавить 0x01
  05.01.2001 SVS
    + Попробуем удалить SUBST диск
    + Покажем инфу, что ЭТО - SUBST-диск
  14.12.2000 SVS
    + Попробуем сделать Eject для съемных дисков в меню выбора дисковода.
  11.11.2000 SVS
    ! FarMkTemp() - убираем (как всегда - то ставим, то тут же убираем :-(((
  11.11.2000 SVS
    ! Используем конструкцию FarMkTemp()
  24.09.2000 SVS
    ! Перерисовка CtrlObject->MainKeyBar (случай, если:
       Ctr-Alt-Shift, потом, Alt-отжать, появится быстрый поиск,
       дальше отпускаем Ctrl и Shift - окно быстрого поиска на месте.
       Теперь, если нажать Esc - KeyBar не перерисуется)
  08.09.2000 VVM
    + Обработка команд
      FCTL_SETSORTMODE, FCTL_SETANOTHERSORTMODE
      FCTL_SETSORTORDER, FCTL_SETANOTHERSORTORDER
  07.09.2000 SVS
    ! Еще одна поправочка (с подачи AT) для Bug#12:
      еще косяк, не дает выйти из меню, если у нас текущий путь - UNC
      "\\server\share\"
  06.09.2000 tran
    - правя баг, внесли пару новых:
       1. strncpy не записывает 0 в конец строки
       2. GetDriveType вызывается постоянно, что грузит комп.
  05.09.2000 SVS
    - Bug#12 -   При удалении сетевого диска по Del и отказе от меню
        фар продолжает показывать удаленный диск. хотя не должен.
        по ctrl-r переходит на ближайший.
  21.07.2000 IG
    - Bug 21 (заголовок после Ctrl-Q, Tab, F3, Esc был кривой)
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
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

static int DragX,DragY,DragMove;
static Panel *SrcDragPanel;
static SaveScreen *DragSaveScr=NULL;
static char DragName[NM];

Panel::Panel()
{
  Focus=0;
  *CurDir=0;
  PanelMode=NORMAL_PANEL;
  PrevViewMode=VIEW_3;
  EnableUpdate=TRUE;
  DragX=DragY=-1;
  SrcDragPanel=NULL;
  ModalMode=0;
  ViewSettings.ColumnCount=0;
  ProcessingPluginCommand=0;
};


Panel::~Panel()
{
  EndDrag();
}


void Panel::SetViewMode(int ViewMode)
{
  PrevViewMode=ViewMode;
  Panel::ViewMode=ViewMode;
};


void Panel::ChangeDirToCurrent()
{
  char NewDir[NM];
  GetCurrentDirectory(sizeof(NewDir),NewDir);
  SetCurDir(NewDir,TRUE);
}


void Panel::ChangeDisk()
{
  int Pos,FirstCall=TRUE;
  if (CurDir[0]!=0 && CurDir[1]==':')
    Pos=toupper(CurDir[0])-'A';
  else
    Pos=getdisk();
  while (Pos!=-1)
  {
    Pos=ChangeDiskMenu(Pos,FirstCall);
    FirstCall=FALSE;
  }
}


int Panel::ChangeDiskMenu(int Pos,int FirstCall)
{
  struct MenuItem ChDiskItem;
  memset(&ChDiskItem,0,sizeof(ChDiskItem));
  char DiskType[100],RootDir[10],DiskLetter[50];
  DWORD Mask,DiskMask;
  int DiskCount,Focus,I;
  int ShowSpecial=FALSE;

  *DiskLetter=0;

  Mask=GetLogicalDrives();

  for (DiskMask=Mask,DiskCount=0;DiskMask!=0;DiskMask>>=1)
    DiskCount+=DiskMask & 1;

  int UserDataSize;
  {
    VMenu ChDisk(MSG(MChangeDriveTitle),NULL,0,ScrY-Y1-3);
    ChDisk.SetHelp("DriveDlg");

    char MenuText[NM];
    int DriveType,MenuLine;
    int LabelWidth=Max(11,strlen(MSG(MChangeDriveLabelAbsent)));

    for (DiskMask=Mask,MenuLine=I=0;DiskMask!=0;DiskMask>>=1,I++)
      if (DiskMask & 1)
      {
        sprintf(MenuText,"&%c: ",'A'+I);
        sprintf(RootDir,"%c:\\",'A'+I);
        DriveType=GetDriveType(RootDir);
        if (Opt.ChangeDriveMode & DRIVE_SHOW_TYPE)
        {
          switch(DriveType)
          {
            case DRIVE_REMOVABLE:
              strcpy(DiskType,MSG(MChangeDriveRemovable));
              break;
            case DRIVE_FIXED:
              strcpy(DiskType,MSG(MChangeDriveFixed));
              break;
            case DRIVE_REMOTE:
              strcpy(DiskType,MSG(MChangeDriveNetwork));
              break;
            case DRIVE_CDROM:
              strcpy(DiskType,MSG(MChangeDriveCDROM));
              break;
            case DRIVE_RAMDISK:
              strcpy(DiskType,MSG(MChangeDriveRAM));
              break;
            default:
              sprintf(DiskType,"%*s",strlen(MSG(MChangeDriveFixed)),"");
              break;
          }
          /* 05.01.2001 SVS
             + Информация про Subst-тип диска
          */
          {
            char LocalName[8],SubstName[NM];
            sprintf(LocalName,"%c:",*RootDir);
            if(GetSubstName(LocalName,SubstName,sizeof(SubstName)))
            {
              strcpy(DiskType,MSG(MChangeDriveSUBST));
              DriveType=DRIVE_SUSTITUTE;
            }
          }
          /* SVS $ */
          strcat(MenuText,DiskType);
        }

        int ShowDisk=(DriveType!=DRIVE_REMOVABLE || (Opt.ChangeDriveMode & DRIVE_SHOW_REMOVABLE)) &&
                     (DriveType!=DRIVE_CDROM || (Opt.ChangeDriveMode & DRIVE_SHOW_CDROM));
        if (Opt.ChangeDriveMode & (DRIVE_SHOW_LABEL|DRIVE_SHOW_FILESYSTEM))
        {
          char VolumeName[NM],FileSystemName[NM];
          *VolumeName=*FileSystemName=0;
          if (ShowDisk && !GetVolumeInformation(RootDir,VolumeName,sizeof(VolumeName),NULL,NULL,NULL,FileSystemName,sizeof(FileSystemName)))
          {
            strcpy(VolumeName,MSG(MChangeDriveLabelAbsent));
            ShowDisk=FALSE;
          }
          if (Opt.ChangeDriveMode & DRIVE_SHOW_LABEL)
          {
            TruncStr(VolumeName,LabelWidth);
            sprintf(MenuText+strlen(MenuText),"│%-*s",LabelWidth,VolumeName);
          }
          if (Opt.ChangeDriveMode & DRIVE_SHOW_FILESYSTEM)
            sprintf(MenuText+strlen(MenuText),"│%-8.8s",FileSystemName);
        }

        if (Opt.ChangeDriveMode & DRIVE_SHOW_SIZE)
        {
          char TotalText[NM],FreeText[NM];
          *TotalText=*FreeText=0;
          int64 TotalSize,TotalFree,UserFree;
          if (ShowDisk && GetDiskSize(RootDir,&TotalSize,&TotalFree,&UserFree))
          {
            sprintf(TotalText,"%5d %.2s",(TotalSize/(1024*1024)).LowPart,MSG(MChangeDriveMb));
            sprintf(FreeText,"%5d %.2s",(UserFree/(1024*1024)).LowPart,MSG(MChangeDriveMb));
          }
          sprintf(MenuText+strlen(MenuText),"│%-8s│%-8s",TotalText,FreeText);
        }

        if (Opt.ChangeDriveMode & DRIVE_SHOW_NETNAME)
        {
          int NetPathShown=FALSE;
          if (DriveType==DRIVE_REMOTE)
          {
            char LocalName[NM],RemoteName[NM];
            DWORD RemoteNameSize=sizeof(RemoteName);
            sprintf(LocalName,"%c:",*RootDir);

            SetFileApisToANSI();
            if (WNetGetConnection(LocalName,RemoteName,&RemoteNameSize)==NO_ERROR)
            {
              NetPathShown=TRUE;
              CharToOem(RemoteName,RemoteName);
              strcat(MenuText,"  ");
              strcat(MenuText,RemoteName);
            }
            SetFileApisToOEM();
          }
          if (!NetPathShown)
          {
            char LocalName[8],SubstName[NM];
            sprintf(LocalName,"%c:",*RootDir);

            if (GetSubstName(LocalName,SubstName,sizeof(SubstName)))
            {
              CharToOem(SubstName,SubstName);
              strcat(MenuText,"  ");
              strcat(MenuText,SubstName);
            }
          }
          ShowSpecial=TRUE;
        }
        if (FirstCall)
          ChDiskItem.Selected=(I==Pos);
        else
          ChDiskItem.Selected=(MenuLine==Pos);
        strncpy(ChDiskItem.Name,MenuText,sizeof(ChDiskItem.Name));
        if (strlen(MenuText)>4)
          ShowSpecial=TRUE;

        /* $ 14.12.2000 SVS
           Дополнительно запомним тип драйва.
        */
        ChDiskItem.UserData[0]='A'+I;
        ChDiskItem.UserData[1]=0;
        ChDiskItem.UserData[2]=(BYTE)DriveType;
        ChDiskItem.UserDataSize=3;
        /* SVS $ */

        ChDisk.AddItem(&ChDiskItem);
        MenuLine++;
      }

    int UsedNumbers[10];
    memset(UsedNumbers,0,sizeof(UsedNumbers));

    struct MenuItem PluginMenuItems[20];
    int PluginMenuItemsCount=0;
    memset(PluginMenuItems,0,sizeof(PluginMenuItems));

    if (Opt.ChangeDriveMode & DRIVE_SHOW_PLUGINS)
    {
      int PluginNumber;
      int PluginItem,Done;
      for (PluginNumber=0;PluginMenuItemsCount<sizeof(PluginMenuItems)/sizeof(PluginMenuItems[0]);PluginNumber++)
      {
        Done=FALSE;
        for (PluginItem=0;;PluginItem++)
        {
          char PluginText[100];
          int PluginTextNumber,ItemPresent;
          if (!CtrlObject->Plugins.GetDiskMenuItem(PluginNumber,PluginItem,
                                   ItemPresent,PluginTextNumber,PluginText))
          {
            Done=TRUE;
            break;
          }
          if (!ItemPresent)
            break;
          if (PluginTextNumber==0)
            continue;
          while (UsedNumbers[PluginTextNumber] && PluginTextNumber<10)
            PluginTextNumber++;
          UsedNumbers[PluginTextNumber%10]=1;
          sprintf(MenuText,"&%c: %s",
                  PluginTextNumber>9 ? '#':PluginTextNumber+'0',
                  ShowSpecial ? PluginText:"");
          strncpy(ChDiskItem.Name,MenuText,sizeof(ChDiskItem.Name));
          ChDiskItem.UserData[0]=PluginNumber;
          ChDiskItem.UserData[1]=PluginItem;
          ChDiskItem.UserDataSize=2;
          PluginMenuItems[PluginMenuItemsCount++]=ChDiskItem;
        }
        if (Done)
          break;
      }
      for (PluginNumber=0;PluginMenuItemsCount<sizeof(PluginMenuItems)/sizeof(PluginMenuItems[0]);PluginNumber++)
      {
        Done=FALSE;
        for (PluginItem=0;;PluginItem++)
        {
          char PluginText[100];
          int PluginTextNumber,ItemPresent;
          if (!CtrlObject->Plugins.GetDiskMenuItem(PluginNumber,PluginItem,
                                   ItemPresent,PluginTextNumber,PluginText))
          {
            Done=TRUE;
            break;
          }
          if (!ItemPresent)
            break;
          if (PluginTextNumber!=0)
            continue;
          PluginTextNumber++;
          while (UsedNumbers[PluginTextNumber] && PluginTextNumber<10)
            PluginTextNumber++;
          UsedNumbers[PluginTextNumber%10]=1;
          sprintf(MenuText,"&%c: %s",
                  PluginTextNumber>9 ? '#':PluginTextNumber+'0',
                  ShowSpecial ? PluginText:"");
          strncpy(ChDiskItem.Name,MenuText,sizeof(ChDiskItem.Name));
          ChDiskItem.UserData[0]=PluginNumber;
          ChDiskItem.UserData[1]=PluginItem;
          ChDiskItem.UserDataSize=2;
          PluginMenuItems[PluginMenuItemsCount++]=ChDiskItem;
        }
        if (Done)
          break;
      }
      if (PluginMenuItemsCount>0)
      {
        memset(&ChDiskItem,0,sizeof(ChDiskItem));
        ChDiskItem.Separator=1;
        ChDiskItem.UserDataSize=0;
        ChDisk.AddItem(&ChDiskItem);
        ChDiskItem.Separator=0;
        for (int I=0;I<PluginMenuItemsCount;I++)
        {
          int MinPos=0;
          for (int J=0;J<PluginMenuItemsCount;J++)
            if (PluginMenuItems[J].Name[1]<PluginMenuItems[MinPos].Name[1])
              MinPos=J;
          PluginMenuItems[MinPos].Selected=!FirstCall && (Pos==DiskCount+1+I);
          ChDisk.AddItem(&PluginMenuItems[MinPos]);
          PluginMenuItems[MinPos].Name[1]='9'+1;
        }
      }
    }

    int X=X1+5;
    if (this==CtrlObject->RightPanel && IsFullScreen() && X2-X1>40)
      X=(X2-X1+1)/2+5;
    int Y=(ScrY+1-(DiskCount+PluginMenuItemsCount+5))/2;
    if (Y<1) Y=1;
    ChDisk.SetPosition(X,Y,0,0);

    if (Y<3)
      ChDisk.SetBoxType(SHORT_DOUBLE_BOX);


    ChDisk.Show();

    while (!ChDisk.Done())
    {
      //SysLog("ExitCode=%i",ChDisk.GetExitCode());
      int SelPos=ChDisk.GetSelectPos();
      int Key;
      {
        ChangeMacroMode MacroMode(MACRO_DISKS);
        Key=ChDisk.ReadInput();
      }
      switch(Key)
      {
        case KEY_DEL:
          if (SelPos<DiskCount)
          {
            char Letter[50],LocalName[50];
            if (ChDisk.GetUserData(Letter,sizeof(Letter)))
            {
              /* $ 14.12.2000 SVS
                 Попробуем сделать Eject :-)
              */
              DriveType=(DWORD)(BYTE)Letter[2];
              if(DriveType == DRIVE_REMOVABLE || DriveType == DRIVE_CDROM)
              {
                EjectVolume(*Letter,0);
                return(SelPos);
              }
              /* SVS $ */
              /* $ 05.01.2001 SVS
                 Пробуем удалить SUBST-драйв.
              */
              if(DriveType == DRIVE_SUSTITUTE)
              {
                char DosDeviceName[16];
                sprintf(DosDeviceName,"%c:",*Letter);
                if(!DelSubstDrive(DosDeviceName))
                  return(SelPos);
                else
                {
                  int LastError=GetLastError();
                  char MsgText[200];
                  sprintf(MsgText,MSG(MChangeDriveCannotDelSubst),DosDeviceName);
                  if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
                    if (Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),MsgText,
                            "\x1",MSG(MChangeDriveOpenFiles),
                            MSG(MChangeDriveAskDisconnect),MSG(MOk),MSG(MCancel))==0)
                    {
                      if(!DelSubstDrive(DosDeviceName))
                        return(SelPos);
                    }
                    else
                      break;
                  Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MsgText,MSG(MOk));
                }
              }
              /* SVS $ */

              sprintf(LocalName,"%c:",*Letter);
              if (WNetCancelConnection2(LocalName,/*CONNECT_UPDATE_PROFILE*/0,FALSE)==NO_ERROR)
                return(SelPos);
              else
              {
                int LastError=GetLastError();
                char MsgText[200];
                sprintf(MsgText,MSG(MChangeDriveCannotDisconnect),LocalName);
                if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
                  if (Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),MsgText,
                          "\x1",MSG(MChangeDriveOpenFiles),
                          MSG(MChangeDriveAskDisconnect),MSG(MOk),MSG(MCancel))==0)
                  {
                    if (WNetCancelConnection2(LocalName,0,TRUE)==NO_ERROR)
                      return(SelPos);
                  }
                  else
                    break;
                char RootDir[50];
                sprintf(RootDir,"%c:\\",*Letter);
                if (GetDriveType(RootDir)==DRIVE_REMOTE)
                  Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MsgText,MSG(MOk));
              }
            }
          }
          break;
        case KEY_CTRL1:
        case KEY_RCTRL1:
          Opt.ChangeDriveMode^=DRIVE_SHOW_TYPE;
          return(SelPos);
        case KEY_CTRL2:
        case KEY_RCTRL2:
          Opt.ChangeDriveMode^=DRIVE_SHOW_NETNAME;
          return(SelPos);
        case KEY_CTRL3:
        case KEY_RCTRL3:
          Opt.ChangeDriveMode^=DRIVE_SHOW_LABEL;
          return(SelPos);
        case KEY_CTRL4:
        case KEY_RCTRL4:
          Opt.ChangeDriveMode^=DRIVE_SHOW_FILESYSTEM;
          return(SelPos);
        case KEY_CTRL5:
        case KEY_RCTRL5:
          Opt.ChangeDriveMode^=DRIVE_SHOW_SIZE;
          return(SelPos);
        case KEY_CTRL6:
        case KEY_RCTRL6:
          Opt.ChangeDriveMode^=DRIVE_SHOW_REMOVABLE;
          return(SelPos);
        case KEY_CTRL7:
        case KEY_RCTRL7:
          Opt.ChangeDriveMode^=DRIVE_SHOW_PLUGINS;
          return(SelPos);
        case KEY_CTRL8:
        case KEY_RCTRL8:
          Opt.ChangeDriveMode^=DRIVE_SHOW_CDROM;
          return(SelPos);
        default:
          ChDisk.ProcessInput();
          break;
      }
      /* $ 05.09.2000 SVS
        Bug#12 -   При удалении сетевого диска по Del и отказе от меню
               фар продолжает показывать удаленный диск. хотя не должен.
               по ctrl-r переходит на ближайший.
               Лучше будет, если он не даст выходить из меню если удален
               текущий диск
      */
      /* $ 06.09.2000 tran
         правя баг, внесли пару новых:
         1. strncpy не записывает 0 в конец строки
         2. GetDriveType вызывается постоянно, что грузит комп.
      */
      /* $ 07.09.2000 SVS
         Еще одна поправочка (с подачи AT):
             еще косяк, не дает выйти из меню, если у нас текущий путь - UNC
             "\\server\share\"
      */
      if (ChDisk.Done() && ChDisk.GetExitCode()<0 && strncmp(CurDir,"\\\\",2)!=0)
      {
        char RootDir[10];
        strncpy(RootDir,CurDir,3);
        RootDir[3]=0;
        if (GetDriveType(RootDir)==DRIVE_NO_ROOT_DIR)
          ChDisk.ClearDone();
      }
      /* SVS $ */
      /* tran $ */
      /* SVS $ */
    }
    if (ChDisk.GetExitCode()<0)
      return(-1);
    UserDataSize=ChDisk.GetUserData(DiskLetter,sizeof(DiskLetter));
  }

  if (ProcessPluginEvent(FE_CLOSE,NULL))
    return(-1);

  ScrBuf.Flush();
  INPUT_RECORD rec;
  PeekInputRecord(&rec);

  if (UserDataSize==3)
  {
    while (1)
    {
      int NumDisk=*DiskLetter-'A';
      char MsgStr[200],NewDir[NM];
      setdisk(NumDisk);
      CtrlObject->CmdLine.GetCurDir(NewDir);
      if (toupper(*NewDir)==*DiskLetter)
        chdir(NewDir);
      if (getdisk()!=NumDisk)
      {
        char RootDir[NM];
        sprintf(RootDir,"%c:\\",*DiskLetter);
        chdir(RootDir);
        setdisk(NumDisk);
        if (getdisk()==NumDisk)
          break;
      }
      else
        break;
      sprintf(MsgStr,MSG(MChangeDriveCannotReadDisk),*DiskLetter);
      if (Message(MSG_WARNING,2,MSG(MError),MsgStr,MSG(MRetry),MSG(MCancel))!=0)
        return(-1);
    }
    char CurDir[NM];
    GetCurrentDirectory(sizeof(CurDir),CurDir);
    Focus=GetFocus();
    Panel *NewPanel=CtrlObject->ChangePanel(this,FILE_PANEL,TRUE,FALSE);
    NewPanel->SetCurDir(CurDir,TRUE);
    NewPanel->Show();
    if (Focus || !CtrlObject->GetAnotherPanel(this)->IsVisible())
      NewPanel->SetFocus();
  }
  else
    if (UserDataSize==2)
    {
      HANDLE hPlugin=CtrlObject->Plugins.OpenPlugin(DiskLetter[0],OPEN_DISKMENU,DiskLetter[1]);
      if (hPlugin!=INVALID_HANDLE_VALUE)
      {
        Focus=GetFocus();
        Panel *NewPanel=CtrlObject->ChangePanel(this,FILE_PANEL,TRUE,TRUE);
        NewPanel->SetPluginMode(hPlugin,"");
        NewPanel->Update(0);
        NewPanel->Show();
        if (Focus || !CtrlObject->GetAnotherPanel(NewPanel)->IsVisible())
          NewPanel->SetFocus();
      }
    }
  return(-1);
}


void Panel::FastFind(int FirstKey)
{
  INPUT_RECORD rec;
  char LastName[NM],Name[NM];
  int Key,KeyToProcess=0;
  *LastName=0;
  {
    int FindX=min(X1+9,ScrX-22);
    int FindY=min(Y2,ScrY-2);
    ChangeMacroMode MacroMode(MACRO_SEARCH);
    SaveScreen SaveScr(FindX,FindY,FindX+21,FindY+2);
    FastFindShow(FindX,FindY);
    Edit FindEdit;
    FindEdit.SetPosition(FindX+2,FindY+1,FindX+19,FindY+1);
    FindEdit.SetEditBeyondEnd(FALSE);
    FindEdit.SetObjectColor(COL_DIALOGEDIT);
    FindEdit.Show();

    while (!KeyToProcess)
    {
      if (FirstKey)
        Key=FirstKey;
      else
      {
        Key=GetInputRecord(&rec);
        if (rec.EventType==MOUSE_EVENT)
          if ((rec.Event.MouseEvent.dwButtonState & 3)==0)
            continue;
          else
            Key=KEY_ESC;
      }
      if (Key==KEY_ESC || Key==KEY_F10)
      {
        KeyToProcess=KEY_NONE;
        break;
      }
      if (Key>=KEY_ALT_BASE+0x01 && Key<=KEY_ALT_BASE+255)
        Key=tolower(Key-KEY_ALT_BASE);
      if (Key>=KEY_ALTSHIFT_BASE+0x01 && Key<=KEY_ALTSHIFT_BASE+255)
        Key=tolower(Key-KEY_ALTSHIFT_BASE);

      if (Key==KEY_MULTIPLY)
        Key='*';

      switch (Key)
      {
        case KEY_CTRLENTER:
          FindPartName(Name,TRUE);
          FindEdit.Show();
          FastFindShow(FindX,FindY);
          break;
        case KEY_NONE:
        case KEY_IDLE:
          break;
        default:
          if ((Key<32 || Key>=256) && Key!=KEY_BS && Key!=KEY_CTRLY &&
              Key!=KEY_CTRLBS && Key!=KEY_ALT && Key!=KEY_SHIFT &&
              Key!=KEY_CTRL && Key!=KEY_RALT && Key!=KEY_RCTRL &&
              Key!=KEY_CTRLINS && Key!=KEY_SHIFTINS)
          {
            KeyToProcess=Key;
            break;
          }
          if (FindEdit.ProcessKey(Key))
          {
            FindEdit.GetString(Name,sizeof(Name));
            if (FindPartName(Name,FALSE))
              strcpy(LastName,Name);
            else
            {
              FindEdit.SetString(LastName);
              strcpy(Name,LastName);
            }
            FindEdit.Show();
            FastFindShow(FindX,FindY);
          }
          break;
      }
      FirstKey=0;
    }
  }
  Show();
  CtrlObject->MainKeyBar.Redraw();
  ScrBuf.Flush();
  Panel *ActivePanel=CtrlObject->ActivePanel;
  if (KeyToProcess==KEY_ENTER && ActivePanel->GetType()==TREE_PANEL)
    ((TreeList *)ActivePanel)->ProcessEnter();
  else
    CtrlObject->ProcessKey(KeyToProcess);
}


void Panel::FastFindShow(int FindX,int FindY)
{
  SetColor(COL_DIALOGTEXT);
  GotoXY(FindX+1,FindY+1);
  Text(" ");
  GotoXY(FindX+20,FindY+1);
  Text(" ");
  Box(FindX,FindY,FindX+21,FindY+2,COL_DIALOGBOX,DOUBLE_BOX);
  GotoXY(FindX+7,FindY);
  SetColor(COL_DIALOGBOXTITLE);
  Text(MSearchFileTitle);
}


int Panel::MakeListFile(char *ListFileName,int ShortNames)
{
  FILE *ListFile;
  strcpy(ListFileName,Opt.TempPath);
  strcat(ListFileName,FarTmpXXXXXX);
  if (mktemp(ListFileName)==NULL || (ListFile=fopen(ListFileName,"wb"))==NULL)
  //if (FarMkTemp(ListFileName,"Far")==NULL || (ListFile=fopen(ListFileName,"wb"))==NULL)
  {
    Message(MSG_WARNING,1,MSG(MError),MSG(MCannotCreateListFile),MSG(MOk));
    return(FALSE);
  }
  char FileName[NM],ShortName[NM];
  int FileAttr;
  GetSelName(NULL,FileAttr);
  while (GetSelName(FileName,FileAttr,ShortName))
  {
    if (ShortNames)
      strcpy(FileName,ShortName);
    if (fprintf(ListFile,"%s\r\n",FileName)==EOF)
    {
      fclose(ListFile);
      remove(ListFileName);
      Message(MSG_WARNING,1,MSG(MError),MSG(MCannotCreateListFile),MSG(MOk));
      return(FALSE);
    }
  }
  if (fclose(ListFile)==EOF)
  {
    clearerr(ListFile);
    fclose(ListFile);
    remove(ListFileName);
    Message(MSG_WARNING,1,MSG(MError),MSG(MCannotCreateListFile),MSG(MOk));
    return(FALSE);
  }
  return(TRUE);
}


void Panel::SetFocus()
{
  if (CtrlObject->ActivePanel!=this)
  {
    CtrlObject->ActivePanel->KillFocus();
    CtrlObject->ActivePanel=this;
  }
  if (!GetFocus())
  {
    CtrlObject->RedrawKeyBar();
    Focus=TRUE;
    Redraw();
    chdir(CurDir);
  }
}


void Panel::KillFocus()
{
  Focus=FALSE;
  Redraw();
}


int Panel::PanelProcessMouse(MOUSE_EVENT_RECORD *MouseEvent,int &RetCode)
{
  RetCode=TRUE;
  if (!ModalMode && MouseEvent->dwMousePosition.Y==0)
    if (MouseEvent->dwMousePosition.X==ScrX)
    {
      if (Opt.ScreenSaver && (MouseEvent->dwButtonState & 3)==0)
      {
        EndDrag();
        ScreenSaver(TRUE);
        return(TRUE);
      }
    }
    else
      if ((MouseEvent->dwButtonState & 3)!=0 && MouseEvent->dwEventFlags==0)
      {
        EndDrag();
        if (MouseEvent->dwMousePosition.X==0)
          CtrlObject->ProcessKey(KEY_CTRLO);
        else
          ShellOptions(0,MouseEvent);
        return(TRUE);
      }

  if (!IsVisible() ||
      (MouseEvent->dwMousePosition.X<X1 || MouseEvent->dwMousePosition.X>X2 ||
      MouseEvent->dwMousePosition.Y<Y1 || MouseEvent->dwMousePosition.Y>Y2))
  {
    RetCode=FALSE;
    return(TRUE);
  }

  if (DragX!=-1)
  {
    if ((MouseEvent->dwButtonState & 3)==0)
    {
      EndDrag();
      if (MouseEvent->dwEventFlags==0 && SrcDragPanel!=this)
      {
        MoveToMouse(MouseEvent);
        Redraw();
        SrcDragPanel->ProcessKey(DragMove ? KEY_DRAGMOVE:KEY_DRAGCOPY);
      }
      return(TRUE);
    }
    if (MouseEvent->dwMousePosition.Y<=Y1 || MouseEvent->dwMousePosition.Y>=Y2 ||
        !CtrlObject->GetAnotherPanel(SrcDragPanel)->IsVisible())
    {
      EndDrag();
      return(TRUE);
    }
    if ((MouseEvent->dwButtonState & 2) && MouseEvent->dwEventFlags==0)
      DragMove=!DragMove;
    if (MouseEvent->dwButtonState & 1)
      if ((abs(MouseEvent->dwMousePosition.X-DragX)>15 || SrcDragPanel!=this) &&
          !ModalMode)
      {
        if (SrcDragPanel->GetSelCount()==1 && DragSaveScr==NULL)
        {
          SrcDragPanel->GoToFile(DragName);
          SrcDragPanel->Show();
        }
        DragMessage(MouseEvent->dwMousePosition.X,MouseEvent->dwMousePosition.Y,DragMove);
        return(TRUE);
      }
      else
      {
        delete DragSaveScr;
        DragSaveScr=NULL;
      }
  }

  if ((MouseEvent->dwButtonState & 3)==0)
    return(TRUE);
  if ((MouseEvent->dwButtonState & 1) && MouseEvent->dwEventFlags==0 &&
      X2-X1<ScrX)
  {
    int FileAttr;
    MoveToMouse(MouseEvent);
    GetSelName(NULL,FileAttr);
    if (GetSelName(DragName,FileAttr) && strcmp(DragName,"..")!=0)
    {
      SrcDragPanel=this;
      DragX=MouseEvent->dwMousePosition.X;
      DragY=MouseEvent->dwMousePosition.Y;
      DragMove=ShiftPressed;
    }
  }
  return(FALSE);
}


int Panel::IsDragging()
{
  return(DragSaveScr!=NULL);
}


void Panel::EndDrag()
{
  delete DragSaveScr;
  DragSaveScr=NULL;
  DragX=DragY=-1;
}


void Panel::DragMessage(int X,int Y,int Move)
{
  char DragMsg[NM],SelName[NM];
  int SelCount,MsgX,Length;

  if ((SelCount=SrcDragPanel->GetSelCount())==0)
    return;

  if (SelCount==1)
  {
    char CvtName[NM];
    int FileAttr;
    SrcDragPanel->GetSelName(NULL,FileAttr);
    SrcDragPanel->GetSelName(SelName,FileAttr);
    strcpy(CvtName,PointToName(SelName));
    QuoteSpace(CvtName);
    strcpy(SelName,CvtName);
  }
  else
    sprintf(SelName,MSG(MDragFiles),SelCount);

  if (Move)
    sprintf(DragMsg,MSG(MDragMove),SelName);
  else
    sprintf(DragMsg,MSG(MDragCopy),SelName);
  if ((Length=strlen(DragMsg))+X>ScrX)
  {
    MsgX=ScrX-Length;
    if (MsgX<0)
    {
      MsgX=0;
      TruncStr(DragMsg,ScrX);
      Length=strlen(DragMsg);
    }
  }
  else
    MsgX=X;
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  delete DragSaveScr;
  DragSaveScr=new SaveScreen(MsgX,Y,MsgX+Length-1,Y);
  GotoXY(MsgX,Y);
  SetColor(COL_PANELDRAGTEXT);
  Text(DragMsg);
}


void Panel::GetCurDir(char *CurDir)
{
  strcpy(CurDir,Panel::CurDir);
}


#if defined(__BORLANDC__)
#pragma warn -par
#endif
void Panel::SetCurDir(char *CurDir,int ClosePlugin)
{
  strcpy(Panel::CurDir,CurDir);
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif


void Panel::InitCurDir(char *CurDir)
{
  strcpy(Panel::CurDir,CurDir);
}


int Panel::SetCurPath()
{
  if (GetMode()==PLUGIN_PANEL)
    return(TRUE);

  char UpDir[NM],*ChPtr;

  strcpy(UpDir,CurDir);
  if ((ChPtr=strrchr(UpDir,'\\'))!=NULL)
    *ChPtr=0;

  if (chdir(CurDir)==-1 || GetFileAttributes(CurDir)==0xFFFFFFFF)
  {
    if (chdir(UpDir)==-1 && chdir("\\")==-1)
      ChangeDisk();
    else
      GetCurrentDirectory(sizeof(CurDir),CurDir);
    return(FALSE);
  }
  return(TRUE);
}


void Panel::Hide()
{
  ScreenObject::Hide();
  Panel *AnotherPanel=CtrlObject->GetAnotherPanel(this);
  if (AnotherPanel->IsVisible())
  {
    if (AnotherPanel->GetFocus())
      if (AnotherPanel->GetType()==FILE_PANEL && AnotherPanel->IsFullScreen() ||
          GetType()==FILE_PANEL && IsFullScreen())
        AnotherPanel->Show();
  }
}


void Panel::Show()
{
  SavePrevScreen();
  Panel *AnotherPanel=CtrlObject->GetAnotherPanel(this);
  if (AnotherPanel->IsVisible() && !GetModalMode())
  {
    SaveScr->AppendArea(AnotherPanel->SaveScr);
    if (AnotherPanel->GetFocus())
    {
      if (AnotherPanel->IsFullScreen())
        return;
      if (GetType()==FILE_PANEL && IsFullScreen())
      {
        ScreenObject::Show();
        AnotherPanel->Show();
        return;
      }
    }
  }
  ScreenObject::Show();
  ShowScreensCount();
}


void Panel::DrawSeparator(int Y)
{
  if (Y<Y2)
  {
    SetColor(COL_PANELBOX);
    GotoXY(X1,Y);
    ShowSeparator(X2-X1+1);
  }
}


void Panel::ShowScreensCount()
{
  if (Opt.ShowScreensNumber && X1==0)
  {
    int Viewers,Editors;
    CtrlObject->ModalManager.GetModalTypesCount(Viewers,Editors);
    if (Viewers>0 || Editors>0)
    {
      char ScreensText[100];
      if (Editors==0)
        sprintf(ScreensText,"[%d]",Viewers);
      else
        sprintf(ScreensText,"[%d+%d]",Viewers,Editors);
      GotoXY(Opt.ShowColumnTitles ? X1:X1+2,Y1);
      SetColor(COL_PANELSCREENSNUMBER);
      Text(ScreensText);
    }
  }
}


void Panel::SetTitle()
{
  if (GetFocus())
  {
    char TitleDir[NM];
    if (*CurDir)
      sprintf(TitleDir,"{%s}",CurDir);
    else
    {
      char CmdText[512];
      /* $ 21.07.2000 IG
         Bug 21 (заголовок после Ctrl-Q, Tab, F3, Esc был кривой)
      */
      CtrlObject->CmdLine.GetCurDir(CmdText);
      /* IG $*/
      sprintf(TitleDir,"{%s}",CmdText);
    }
    strcpy(LastFarTitle,TitleDir);
    SetFarTitle(TitleDir);
  }
}


void Panel::SetPluginCommand(int Command,void *Param)
{
  ProcessingPluginCommand++;
  Panel *AnotherPanel=CtrlObject->GetAnotherPanel(this);
  PluginCommand=Command;
  switch(Command)
  {
    case FCTL_SETVIEWMODE:
    case FCTL_SETANOTHERVIEWMODE:
      if (Param!=NULL)
      {
        int Mode=*(int *)Param;
        if (Mode>=0 && Mode<=9)
        {
          Panel *DestPanel=(Command==FCTL_SETVIEWMODE) ? this:AnotherPanel;
          if (DestPanel!=NULL)
            DestPanel->SetViewMode(Mode);
        }
      }
      break;
    case FCTL_SETSORTMODE:
    case FCTL_SETANOTHERSORTMODE:
      if (Param!=NULL)
      {
        int Mode=*(int *)Param;
        if ((Mode>SM_DEFAULT) && (Mode<=SM_NUMLINKS))
        {
          Panel *DestPanel=(Command==FCTL_SETSORTMODE) ? this:AnotherPanel;
          if (DestPanel!=NULL)
          // Уменьшим на 1 из-за SM_DEFAULT
            DestPanel->SetSortMode(--Mode);
        }
      }
      break;
    case FCTL_SETSORTORDER:
    case FCTL_SETANOTHERSORTORDER:
      if (Param!=NULL)
      {
        /* $ 22.01.2001 VVM
           - Порядок сортировки задается аналогично StartSortOrder */
        int Order = (*(int *)Param) ? -1:1;
        /* VVM $ */
        Panel *DestPanel=(Command==FCTL_SETSORTORDER) ? this:AnotherPanel;
        if (DestPanel!=NULL)
          DestPanel->SetSortOrder(Order);
      }
      break;
    case FCTL_CLOSEPLUGIN:
      strcpy((char *)PluginParam,NullToEmpty((char *)Param));
      break;
    case FCTL_GETPANELINFO:
    case FCTL_GETANOTHERPANELINFO:
    {
      struct PanelInfo *Info=(struct PanelInfo *)Param;
      memset(Info,0,sizeof(*Info));
      Panel *DestPanel=(Command==FCTL_GETPANELINFO) ? this:AnotherPanel;
      if (DestPanel==NULL)
        break;
      switch(DestPanel->GetType())
      {
        case FILE_PANEL:
          Info->PanelType=PTYPE_FILEPANEL;
          break;
        case TREE_PANEL:
          Info->PanelType=PTYPE_TREEPANEL;
          break;
        case QVIEW_PANEL:
          Info->PanelType=PTYPE_QVIEWPANEL;
          break;
        case INFO_PANEL:
          Info->PanelType=PTYPE_INFOPANEL;
          break;
      }
      Info->Plugin=DestPanel->GetMode()==PLUGIN_PANEL;
      int X1,Y1,X2,Y2;
      DestPanel->GetPosition(X1,Y1,X2,Y2);
      Info->PanelRect.left=X1;
      Info->PanelRect.top=Y1;
      Info->PanelRect.right=X2;
      Info->PanelRect.bottom=Y2;
      Info->Visible=DestPanel->IsVisible();
      Info->Focus=DestPanel->GetFocus();
      Info->ViewMode=DestPanel->GetViewMode();
      Info->SortMode=SM_UNSORTED-UNSORTED+DestPanel->GetSortMode();
      DestPanel->GetCurDir(Info->CurDir);
      if (DestPanel->GetType()==FILE_PANEL)
      {
        FileList *DestFilePanel=(FileList *)DestPanel;
        static int Reenter=0;
        if (!Reenter && Info->Plugin)
        {
          Reenter++;
          struct OpenPluginInfo PInfo;
          DestFilePanel->GetOpenPluginInfo(&PInfo);
          strcpy(Info->CurDir,PInfo.CurDir);
          Reenter--;
        }
        DestFilePanel->PluginGetPanelInfo(Info);
      }
      break;
    }
    case FCTL_SETSELECTION:
    case FCTL_SETANOTHERSELECTION:
      {
        Panel *DestPanel=(Command==FCTL_SETSELECTION) ? this:AnotherPanel;
        if (DestPanel!=NULL && DestPanel->GetType()==FILE_PANEL)
          ((FileList *)DestPanel)->PluginSetSelection((struct PanelInfo *)Param);
        break;
      }
    case FCTL_UPDATEPANEL:
      Update(Param==NULL ? 0:UPDATE_KEEP_SELECTION);
      break;
    case FCTL_UPDATEANOTHERPANEL:
      AnotherPanel->Update(Param==NULL ? 0:UPDATE_KEEP_SELECTION);
      if (AnotherPanel!=NULL && AnotherPanel->GetType()==QVIEW_PANEL)
        UpdateViewPanel();
      break;
    case FCTL_REDRAWPANEL:
      {
        struct PanelRedrawInfo *Info=(struct PanelRedrawInfo *)Param;
        if (Info!=NULL)
        {
          CurFile=Info->CurrentItem;
          CurTopFile=Info->TopPanelItem;
        }
        Redraw();
      }
      break;
    case FCTL_REDRAWANOTHERPANEL:
      if (AnotherPanel!=NULL)
      {
        struct PanelRedrawInfo *Info=(struct PanelRedrawInfo *)Param;
        if (Info!=NULL)
        {
          AnotherPanel->CurFile=Info->CurrentItem;
          AnotherPanel->CurTopFile=Info->TopPanelItem;
        }
        AnotherPanel->Redraw();
      }
      break;
    case FCTL_SETANOTHERPANELDIR:
      if (Param!=NULL && AnotherPanel!=NULL)
        AnotherPanel->SetCurDir((char *)Param,TRUE);
      break;
    case FCTL_SETPANELDIR:
      if (Param!=NULL)
        SetCurDir((char *)Param,TRUE);
      break;
  }
  ProcessingPluginCommand--;
}


int Panel::GetCurName(char *Name,char *ShortName)
{
  *Name=*ShortName=0;
  return(FALSE);
}
