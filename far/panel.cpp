/*
panel.cpp

Parent class для панелей

*/

/* Revision: 1.57 24.07.2001 $ */

/*
Modify:
  24.07.2001 SVS
    + Opt.PgUpChangeDisk
  23.07.2001 SVS
    ! В функции FastFind() код ограничим WaitInFastFind++ и WaitInFastFind--
      ибо здесь ему место (сама переменная как раз за ЭТО и отвечает)
  23.07.2001 SVS
    ! Вернем обратно режим отображения размера в меню выбора диска
      но несколько расширим размеры (+1) числа и сократим "MB" до "M"
  22.07.2001 SVS
    + Повторное нажатие CtrlPgUp в меню выбора дисков гасит это меню.
    + Shift-Enter в меню выбора дисков вызывает проводник для данного диска
  18.07.2001 OT
    ! VFMenu
  21.06.2001 tran
    ! убран глюк AltF1,F1,esc
  17.06.2001 KM
    ! Добавление WRAPMODE в меню.
  14.06.2001 KM
    + Добавлена установка переменных окружения, определяющих
      текущие директории дисков как для активной, так и для
      пассивной панели. Это необходимо программам запускаемым
      из FAR.
  06.06.2001 SVS
    ! Mix/Max
  03.06.2001 SVS
    ! Изменения в связи с переделкой UserData в VMenu
  21.05.2001 DJ
    ! в связи с появлением нового типа немодальных окон переписана отрисовка
      счетчика фоновых экранов
  21.05.2001 SVS
    ! struct MenuData|MenuItem
      Поля Selected, Checked, Separator и Disabled преобразованы в DWORD Flags
  12.05.2001 DJ
    - FCTL_REDRAW[ANOTHER]PANEL обрабатывается только тогда, когда панели
      являются активным фреймом
  10.05.2001 SVS
    - Косметика: при огромных размерах диска в меню "выбора диска" криво
      показываются размеры (сдвиг вправо).
  09.05.2001 OT
    - исправление Panel::Show
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
  30.04.2001 DJ
    - не давало закрыть по Esc менюшку выбора диска, вызванную из quick view
      панели
  28.04.2001 VVM
    + GetSubstName() принимает тип носителя
  27.04.2001 SVS
    + Т.к. нет способа получить состояние "открытости" устройства,
      то добавим обработку Ins для CD - "закрыть диск"
  26.04.2001 VVM
    ! Отмена патча 547
  24.04.2001 SVS
    + Заполнение флагов PanelInfo.Flags
    - !@AFQ! в корне любого диска делал "E:\\file.txt", т.е. 2 слеша.
  24.04.2001 VVM
    - Баг при смене порядка сортировки
  22.04.2001 SVS
    ! Временная отмена куска патча 547
    + Добавка для NT/2000 - вторичный Del на CDROOM задвигает диск.
      Афишировать пока не бум :-)
  19.04.2001 SVS
    - не удалялся SUBST-диск - портилось оригинальное значение DriveType
      в момент проверки "Мы хотим спрятать cd-rom или сменный диск"
  09.04.2001 SVS
    ! проблемы с быстрым поиском.
  02.04.2001 SVS
    ! DRIVE_SUSTITUTE -> DRIVE_SUBSTITUTE
  02.04.2001 VVM
    ! Попытка не будить спящие диски...
  30.03.2001 SVS
    ! GetLogicalDrives заменен на FarGetLogicalDrives() в связи с началом
      компании по поддержке виндовой "полиции".
  28.03.2001 VVM
    + Обработка Opt.RememberLogicalDrives.
  27.03.2001 SVS
    + Shift-F1 на пункте плагина в меню выбора дисков тоже покажет хелп...
  16.03.2001 SVS
    + добавлена информация о пути соединения в диалог подтверждения удаления
      мапленного диска.
    ! Кусок кода, получающий информацию о RemoteName вынесен в функцию
      DriveLocalToRemoteName()
  15.03.2001 SVS
    ! Del в меню дисков
  15.03.2001 IS
    + Используем дополнительные хоткеи, а не просто '#', как раньше, если строк
      от плагинов в меню выбора диска больше 9 штук
  12.03.2001 SVS
    ! Коррекция в связи с изменениями в классе int64
  28.02.2001 IS
    ! "CtrlObject->CmdLine." -> "CtrlObject->CmdLine->"
  27.02.2001 VVM
    ! Символы, зависимые от кодовой страницы
      /[\x01-\x08\x0B-\x0C\x0E-\x1F\xB0-\xDF\xF8-\xFF]/
      переведены в коды.
  26.02.2001 VVM
    - Отмена предыдущего патча
  26.02.2001 VVM
    ! Обработка NULL после OpenPlugin
  14.02.2001 SVS
    ! Дополнительный параметр для MakeListFile - модификаторы
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

#include "panel.hpp"
#include "plugin.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "vmenu.hpp"
#include "filepanels.hpp"
#include "cmdline.hpp"
#include "chgmmode.hpp"
#include "chgprior.hpp"
#include "edit.hpp"
#include "treelist.hpp"
#include "filelist.hpp"
#include "dialog.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"

static int DragX,DragY,DragMove;
static Panel *SrcDragPanel;
static SaveScreen *DragSaveScr=NULL;
static char DragName[NM];

static unsigned char VerticalLine=0x0B3;

static int MessageRemoveConnection(char Letter, int &UpdateProfile);


Panel::Panel()
{
  _OT(SysLog("[%p] Panel::Panel()", this));
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
  _OT(SysLog("[%p] Panel::~Panel()", this));
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


int  Panel::ChangeDiskMenu(int Pos,int FirstCall)
{
  struct MenuItem ChDiskItem;
  memset(&ChDiskItem,0,sizeof(ChDiskItem));
  char DiskType[100],RootDir[10],DiskLetter[50];
  DWORD Mask,DiskMask;
  int DiskCount,Focus,I;
  int ShowSpecial=FALSE;

  *DiskLetter=0;

  _tran(SysLog("Panel::ChangeDiskMenu(), Pos=%i, FirstCall=%i",Pos,FirstCall));
  Mask=FarGetLogicalDrives();

  for (DiskMask=Mask,DiskCount=0;DiskMask!=0;DiskMask>>=1)
    DiskCount+=DiskMask & 1;

  int UserDataSize;
  DWORD UserData;
  {
    _tran(SysLog("create VMenu ChDisk"));
    VFMenu ChDisk(MSG(MChangeDriveTitle),NULL,0,ScrY-Y1-3);
    ChDisk.SetFlags(VMENU_NOTCENTER);
    if ( this==CtrlObject->Cp()->LeftPanel){
      ChDisk.SetFlags(VMENU_LEFTMOST);
    }
    ChDisk.SetHelp("DriveDlg");
    /* $ 17.06.2001 KM
       ! Добавление WRAPMODE в меню.
    */
    ChDisk.SetFlags(VMENU_WRAPMODE);
    /* KM $ */

    char MenuText[NM];
    int DriveType,MenuLine;
    int LabelWidth=Max(11,(int)strlen(MSG(MChangeDriveLabelAbsent)));

    /* $ 02.04.2001 VVM
      ! Попытка не будить спящие диски... */
    for (DiskMask=Mask,MenuLine=I=0;DiskMask!=0;DiskMask>>=1,I++)
    {
      if (DiskMask & 1)
      {
        sprintf(MenuText,"&%c: ",'A'+I);
        sprintf(RootDir,"%c:\\",'A'+I);
        DriveType = GetDriveType(RootDir);
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
            if(GetSubstName(DriveType,LocalName,SubstName,sizeof(SubstName)))
            {
              strcpy(DiskType,MSG(MChangeDriveSUBST));
              DriveType=DRIVE_SUBSTITUTE;
            }
          }
          /* SVS $ */
          strcat(MenuText,DiskType);
        }

        int ShowDisk = (DriveType!=DRIVE_REMOVABLE || (Opt.ChangeDriveMode & DRIVE_SHOW_REMOVABLE)) &&
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
            sprintf(MenuText+strlen(MenuText),"%c%-*s",VerticalLine,LabelWidth,VolumeName);
          }
          if (Opt.ChangeDriveMode & DRIVE_SHOW_FILESYSTEM)
            sprintf(MenuText+strlen(MenuText),"%c%-8.8s",VerticalLine,FileSystemName);
        }

        if (Opt.ChangeDriveMode & DRIVE_SHOW_SIZE)
        {
          char TotalText[NM],FreeText[NM];
          *TotalText=*FreeText=0;
          int64 TotalSize,TotalFree,UserFree;
          if (ShowDisk && GetDiskSize(RootDir,&TotalSize,&TotalFree,&UserFree))
          {
            /* $ 10.05.2001 SVS
               Кривое форматирования вывода при охрененных размерах диска :-(
            */
            sprintf(TotalText,"%6d %1.1s",(TotalSize/(1024*1024)).PLow(),MSG(MChangeDriveMb));
//            FileSizeToStr(TotalText,TotalSize.PHigh(),TotalSize.PLow(),8,0,1);
            sprintf(FreeText,"%6d %1.1s",(UserFree/(1024*1024)).PLow(),MSG(MChangeDriveMb));
//            FileSizeToStr(FreeText,UserFree.PHigh(),UserFree.PLow(),8,0,1);
            /* SVS $ */
          }
          sprintf(MenuText+strlen(MenuText),"%c%-8s%c%-8s",VerticalLine,TotalText,VerticalLine,FreeText);
        }

        if (Opt.ChangeDriveMode & DRIVE_SHOW_NETNAME)
        {
          char RemoteName[NM];
          DriveLocalToRemoteName(DriveType,*RootDir,RemoteName);
          if(*RemoteName)
          {
            strcat(MenuText,"  ");
            strcat(MenuText,RemoteName);
          }
          ShowSpecial=TRUE;
        }
        if (FirstCall)
          ChDiskItem.SetSelect(I==Pos);
        else
          ChDiskItem.SetSelect(MenuLine==Pos);
        strncpy(ChDiskItem.Name,MenuText,sizeof(ChDiskItem.Name));
        if (strlen(MenuText)>4)
          ShowSpecial=TRUE;

        UserData=MAKELONG(MAKEWORD('A'+I,0),DriveType);
        ChDisk.SetUserData((char*)UserData,sizeof(UserData),
                 ChDisk.AddItem(&ChDiskItem));
        MenuLine++;
      } // if (DiskMask & 1)
    } // for
    /* VVM $ */

    int UsedNumbers[10];
    memset(UsedNumbers,0,sizeof(UsedNumbers));

    struct MenuItem PluginMenuItems[20];
    int PluginMenuItemsCount=0;
    memset(PluginMenuItems,0,sizeof(PluginMenuItems));
    /* $ 15.03.2001 IS
         Список дополнительных хоткеев, для случая, когда плагинов, добавляющих
         пункт в меню, больше 9. Всего, btw, обрабатывается не больше 20
         плагинов, поэтому дополнительных хоткеев нужно МИНИМУМ 11(=20-9)
         штук.
    */
    char *AdditionalHotKey=MSG(MAdditionalHotKey);
    int AHKPos=0,                           // индекс в списке хоткеев
        AHKSize=strlen(AdditionalHotKey);   /* для предотвращения выхода за
                                               границу массива */
    /* IS $ */

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
          /* $ 15.03.2001 IS
               Используем дополнительные хоткеи, а не просто '#', как раньше.
          */
          sprintf(MenuText,"&%c: %s",
                  PluginTextNumber>9 ? AdditionalHotKey[(AHKPos++)%AHKSize]:
                  PluginTextNumber+'0', ShowSpecial ? PluginText:"");
          /* IS $ */
          strncpy(ChDiskItem.Name,MenuText,sizeof(ChDiskItem.Name));
          ChDiskItem.UserDataSize=0;
          ChDiskItem.UserData=(char*)MAKELONG(PluginNumber,PluginItem);
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
          /* $ 15.03.2001 IS
               Используем дополнительные хоткеи, а не просто '#', как раньше.
          */
          sprintf(MenuText,"&%c: %s",
                  PluginTextNumber>9 ? AdditionalHotKey[(AHKPos++)%AHKSize]:
                  PluginTextNumber+'0', ShowSpecial ? PluginText:"");
          /* IS $ */
          strncpy(ChDiskItem.Name,MenuText,sizeof(ChDiskItem.Name));
          ChDiskItem.UserDataSize=0;
          ChDiskItem.UserData=(char*)MAKELONG(PluginNumber,PluginItem);
          PluginMenuItems[PluginMenuItemsCount++]=ChDiskItem;
        }
        if (Done)
          break;
      }
      if (PluginMenuItemsCount>0)
      {
        memset(&ChDiskItem,0,sizeof(ChDiskItem));
        ChDiskItem.Flags|=LIF_SEPARATOR;
        ChDiskItem.UserDataSize=0;
        ChDisk.AddItem(&ChDiskItem);
        ChDiskItem.Flags&=~LIF_SEPARATOR;
        for (int I=0;I<PluginMenuItemsCount;I++)
        {
          int MinPos=0;
          for (int J=0;J<PluginMenuItemsCount;J++)
            if (PluginMenuItems[J].Name[1]<PluginMenuItems[MinPos].Name[1])
              MinPos=J;
          PluginMenuItems[MinPos].SetSelect(!FirstCall && (Pos==DiskCount+1+I));
          ChDisk.AddItem(&PluginMenuItems[MinPos]);
          PluginMenuItems[MinPos].Name[1]='9'+1;
        }
      }
    }

    int X=X1+5;
    if (this==CtrlObject->Cp()->RightPanel && IsFullScreen() && X2-X1>40)
      X=(X2-X1+1)/2+5;
    int Y=(ScrY+1-(DiskCount+PluginMenuItemsCount+5))/2;
    if (Y<1) Y=1;
    ChDisk.SetPosition(X,Y,0,0);

    if (Y<3)
      ChDisk.SetBoxType(SHORT_DOUBLE_BOX);

    _tran(SysLog(" call ChDisk.Show"));
    ChDisk.Show();

    while (!ChDisk.Done())
    {
      //_D(SysLog("ExitCode=%i",ChDisk.GetExitCode()));
      int SelPos=ChDisk.GetSelectPos();
      int Key;
      {
        ChangeMacroMode MacroMode(MACRO_DISKS);
        Key=ChDisk.ReadInput();
      }
      switch(Key)
      {
        // Shift-Enter в меню выбора дисков вызывает проводник для данного диска
        case KEY_SHIFTENTER:
          if (SelPos<DiskCount)
          {
            if ((UserData=(DWORD)ChDisk.GetUserData(NULL,0)) != NULL)
            {
              char DosDeviceName[16];
              sprintf(DosDeviceName,"%c:\\",LOBYTE(LOWORD(UserData)));
              Execute(DosDeviceName,FALSE,TRUE,TRUE);
            }
          }
          break;
        case KEY_CTRLPGUP:
          if(Opt.PgUpChangeDisk)
            return -1;
          break;
        /* $ 27.04.2001 SVS
           Т.к. нет способа получить состояние "открытости" устройства,
           то добавим обработку Ins для CD - "закрыть диск"
        */
        case KEY_INS:
          if (SelPos<DiskCount && WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
          {
//            char MsgText[200], LocalName[50];
            if ((UserData=(DWORD)ChDisk.GetUserData(NULL,0)) != NULL)
            {
              DriveType=HIWORD(UserData);
              if(DriveType == DRIVE_CDROM /* || DriveType == DRIVE_REMOVABLE*/)
              {
                EjectVolume(LOBYTE(LOWORD(UserData)),EJECT_LOAD_MEDIA);
                return(SelPos);
              }
            }
          }
          break;
        /* SVS $ */
        case KEY_DEL:
          if (SelPos<DiskCount)
          {
            char MsgText[200], LocalName[50];
            int UpdateProfile=CONNECT_UPDATE_PROFILE;
            if ((UserData=(DWORD)ChDisk.GetUserData(NULL,0)) != NULL)
            {
              DiskLetter[0]=LOBYTE(LOWORD(UserData));
              DiskLetter[1]=0;
              /* $ 14.12.2000 SVS
                 Попробуем сделать Eject :-)
              */
              DriveType=HIWORD(UserData);
              if(DriveType == DRIVE_REMOVABLE || DriveType == DRIVE_CDROM)
              {
                EjectVolume(*DiskLetter,0);
                return(SelPos);
              }
              /* SVS $ */
              /* $ 05.01.2001 SVS
                 Пробуем удалить SUBST-драйв.
              */
              if(DriveType == DRIVE_SUBSTITUTE)
              {
                char DosDeviceName[16];
                sprintf(DosDeviceName,"%c:",*DiskLetter);
                if(!DelSubstDrive(DosDeviceName))
                  return(SelPos);
                else
                {
                  int LastError=GetLastError();
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
                break; // блин. в прошлый раз забыл про это дело...
              }
              /* SVS $ */

              if(DriveType == DRIVE_REMOTE && MessageRemoveConnection(*DiskLetter,UpdateProfile))
              {
                sprintf(LocalName,"%c:",*DiskLetter);

                if (WNetCancelConnection2(LocalName,UpdateProfile,FALSE)==NO_ERROR)
                  return(SelPos);
                else
                {
                  int LastError=GetLastError();
                  sprintf(MsgText,MSG(MChangeDriveCannotDisconnect),LocalName);
                  if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
                    if (Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),MsgText,
                            "\x1",MSG(MChangeDriveOpenFiles),
                            MSG(MChangeDriveAskDisconnect),MSG(MOk),MSG(MCancel))==0)
                    {
                      if (WNetCancelConnection2(LocalName,UpdateProfile,TRUE)==NO_ERROR)
                        return(SelPos);
                    }
                    else
                      break;
                  char RootDir[50];
                  sprintf(RootDir,"%c:\\",*DiskLetter);
                  if (GetDriveType(RootDir)==DRIVE_REMOTE)
                    Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MsgText,MSG(MOk));
                }
                break;
              }
            } // END: if (ChDisk.GetUserData(DiskLetter,...
          } // END: if (SelPos<DiskCount)
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
        /* $ 27.03.2001 SVS
          Shift-F1 на пункте плагина в меню выбора дисков тоже покажет хелп...
        */
        case KEY_SHIFTF1:
          if (SelPos>DiskCount)
          {
            // Вызываем нужный топик, который передали в CommandsMenu()
            if ((UserData=(DWORD)ChDisk.GetUserData(NULL,0)) != NULL)
              FarShowHelp(CtrlObject->Plugins.PluginsData[LOWORD(UserData)].ModuleName,
                    NULL,FHELP_SELFHELP|FHELP_NOSHOWERROR|FHELP_USECONTENTS);
          }
          break;
        /* SVS $ */
        /* $ 21.06.2001 tran
           типа костыль...
           самое простое и быстрое решение проблемы*/
        case KEY_F1:
          {
//            SaveScreen s;
            ChDisk.ProcessInput();
          }
          break;
        /* tran 21.06.2001 $ */
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
      /* $ 30.04.2001 DJ
         и еще одна поправочка: не дает выйти из меню, если оно вызвано
         из quick view panel (в нем CurDir пустая)
      */
      if (ChDisk.Done() && ChDisk.VMenu::GetExitCode()<0 && *CurDir && strncmp(CurDir,"\\\\",2)!=0)
      {
        char RootDir[10];
        strncpy(RootDir,CurDir,3);
        RootDir[3]=0;
        if (GetDriveType(RootDir)==DRIVE_NO_ROOT_DIR)
          ChDisk.ClearDone();
      }
      /* DJ $ */
      /* SVS $ */
      /* tran $ */
      /* SVS $ */
    } // while (!Done)
    if (ChDisk.VMenu::GetExitCode()<0)
      return(-1);
    {
      UserDataSize=ChDisk.VMenu::GetExitCode()>DiskCount?2:3;
      UserData=(DWORD)ChDisk.GetUserData(NULL,0);
    }
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
      int NumDisk=LOBYTE(LOWORD(UserData))-'A';
      char MsgStr[200],NewDir[NM];
      setdisk(NumDisk);
      CtrlObject->CmdLine->GetCurDir(NewDir);
      if (toupper(*NewDir)==LOBYTE(LOWORD(UserData)))
        chdir(NewDir);
      if (getdisk()!=NumDisk)
      {
        char RootDir[NM];
        sprintf(RootDir,"%c:\\",LOBYTE(LOWORD(UserData)));
        chdir(RootDir);
        setdisk(NumDisk);
        if (getdisk()==NumDisk)
          break;
      }
      else
        break;
      sprintf(MsgStr,MSG(MChangeDriveCannotReadDisk),LOBYTE(LOWORD(UserData)));
      if (Message(MSG_WARNING,2,MSG(MError),MsgStr,MSG(MRetry),MSG(MCancel))!=0)
        return(-1);
    }
    char CurDir[NM];
    GetCurrentDirectory(sizeof(CurDir),CurDir);
    Focus=GetFocus();
    Panel *NewPanel=CtrlObject->Cp()->ChangePanel(this,FILE_PANEL,TRUE,FALSE);
    NewPanel->SetCurDir(CurDir,TRUE);
    NewPanel->Show();
    if (Focus || !CtrlObject->Cp()->GetAnotherPanel(this)->IsVisible())
      NewPanel->SetFocus();
  }
  else
    if (UserDataSize==2)
    {
      HANDLE hPlugin=CtrlObject->Plugins.OpenPlugin(LOWORD(UserData),OPEN_DISKMENU,HIWORD(UserData));
      if (hPlugin!=INVALID_HANDLE_VALUE)
      {
        Focus=GetFocus();
        Panel *NewPanel=CtrlObject->Cp()->ChangePanel(this,FILE_PANEL,TRUE,TRUE);
        NewPanel->SetPluginMode(hPlugin,"");
        NewPanel->Update(0);
        NewPanel->Show();
        if (Focus || !CtrlObject->Cp()->GetAnotherPanel(NewPanel)->IsVisible())
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
  WaitInFastFind++;
  {
    int FindX=Min(X1+9,ScrX-22);
    int FindY=Min(Y2,ScrY-2);
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
            /* $ 09.04.2001 SVS
               проблемы с быстрым поиском.
               Подробнее в 00573.ChangeDirCrash.txt
            */
            if(*Name == '"')
            {
              memmove(Name,Name+1,sizeof(Name)-1);
              Name[strlen(Name)-1]=0;
              FindEdit.SetString(Name);
            }
            /* SVS $ */
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
  WaitInFastFind--;
  Show();
  CtrlObject->MainKeyBar->Redraw();
  ScrBuf.Flush();
  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  if (KeyToProcess==KEY_ENTER && ActivePanel->GetType()==TREE_PANEL)
    ((TreeList *)ActivePanel)->ProcessEnter();
  else
    CtrlObject->Cp()->ProcessKey(KeyToProcess);
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


int  Panel::MakeListFile(char *ListFileName,int ShortNames,char *Modifers)
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
  char FileName[NM*2],ShortName[NM];
  int FileAttr;
  GetSelName(NULL,FileAttr);
  while (GetSelName(FileName,FileAttr,ShortName))
  {
    if (ShortNames)
      strcpy(FileName,ShortName);

    if(Modifers && *Modifers)
    {
      if(strchr(Modifers,'F')) // 'F' - использовать полный путь;
      {
        char TempFileName[NM*2];
        strcpy(TempFileName,CurDir);
        sprintf(TempFileName,"%s%s%s",CurDir,(CurDir[strlen(CurDir)-1] != '\\'?"\\":""),FileName);
        if (ShortNames)
          ConvertNameToShort(TempFileName,TempFileName);
        strcpy(FileName,TempFileName);
      }
      if(strchr(Modifers,'Q')) // 'Q' - заключать имена с пробелами в кавычки;
        QuoteSpaceOnly(FileName);
      if(strchr(Modifers,'A')) // 'A' - использовать ANSI кодировку.
        OemToChar(FileName,FileName);

      if(strchr(Modifers,'S')) // 'S' - использовать '/' вместо '\' в путях файлов;
      {
        int I,Len=strlen(FileName);
        for(I=0; I < Len; ++I)
          if(FileName[I] == '\\')
            FileName[I]='/';
      }
    }
//_D(SysLog("%s[%s] %s",__FILE__,Modifers,FileName));
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
  if (CtrlObject->Cp()->ActivePanel!=this)
  {
    CtrlObject->Cp()->ActivePanel->KillFocus();
    CtrlObject->Cp()->ActivePanel=this;
  }
  if (!GetFocus())
  {
    CtrlObject->Cp()->RedrawKeyBar();
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


int  Panel::PanelProcessMouse(MOUSE_EVENT_RECORD *MouseEvent,int &RetCode)
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
          CtrlObject->Cp()->ProcessKey(KEY_CTRLO);
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
        !CtrlObject->Cp()->GetAnotherPanel(SrcDragPanel)->IsVisible())
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


int  Panel::IsDragging()
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


/* $ 14.06.2001 KM
   + Добавлена установка переменных окружения, определяющих
     текущие директории дисков как для активной, так и для
     пассивной панели. Это необходимо программам запускаемым
     из FAR.
*/
int  Panel::SetCurPath()
{
  if (GetMode()==PLUGIN_PANEL)
    return TRUE;

  BOOL RetVal=FALSE;
  char UpDir[NM],Drive[4],*ChPtr;

  strcpy(UpDir,CurDir);
  if ((ChPtr=strrchr(UpDir,'\\'))!=NULL)
    *ChPtr=0;

  if (chdir(CurDir)==-1 || GetFileAttributes(CurDir)==0xFFFFFFFF)
  {
    if (chdir(UpDir)==-1 && chdir("\\")==-1)
      ChangeDisk();
    else
      GetCurrentDirectory(sizeof(CurDir),CurDir);
    return FALSE;
  }

  if (isalpha(CurDir[0]) && CurDir[1]==':')
  {
    sprintf(Drive,"=%c:\x0",CurDir[0]);
    SetEnvironmentVariable(Drive,CurDir);
  }

  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  if (AnotherPanel->GetType()==PLUGIN_PANEL)
    return TRUE;

  if (isalpha(AnotherPanel->CurDir[0]) && AnotherPanel->CurDir[1]==':' &&
      toupper(AnotherPanel->CurDir[0])!=toupper(CurDir[0]))
  {
    sprintf(Drive,"=%c:\x0",AnotherPanel->CurDir[0]);
    SetEnvironmentVariable(Drive,AnotherPanel->CurDir);
  }

  return TRUE;
}
/* KM $ */


void Panel::Hide()
{
  ScreenObject::Hide();
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
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
  /* $ 09.05.2001 OT */
//  SavePrevScreen();
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  if (AnotherPanel->IsVisible() && !GetModalMode())
  {
  /* $ 09.05.2001 OT */
    if (SaveScr) {
      SaveScr->AppendArea(AnotherPanel->SaveScr);
    }
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
    /* $ 19.05.2001 DJ
       переписано для показа диалогов
    */
    int Viewers=FrameManager->GetFrameCountByType (MODALTYPE_VIEWER);
    int Editors=FrameManager->GetFrameCountByType (MODALTYPE_EDITOR);
    int Dialogs=FrameManager->GetFrameCountByType (MODALTYPE_DIALOG);
    if (Viewers>0 || Editors>0 || Dialogs > 0)
    {
      char ScreensText[100];
      sprintf(ScreensText,"[%d", Viewers);
      if (Editors > 0)
        sprintf (ScreensText+strlen (ScreensText), "+%d", Editors);
      if (Dialogs > 0)
        sprintf (ScreensText+strlen (ScreensText), ",%d", Dialogs);
      strcat (ScreensText, "]");
      GotoXY(Opt.ShowColumnTitles ? X1:X1+2,Y1);
      SetColor(COL_PANELSCREENSNUMBER);
      Text(ScreensText);
    }
    /* DJ $ */
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
      CtrlObject->CmdLine->GetCurDir(CmdText);
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
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
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
        /* $ 24.04.2001 VVVM
           Использовать функция ChangeSortOrder() */
          DestPanel->ChangeSortOrder(Order);
        /* VVM $ */
      }
      break;
    case FCTL_CLOSEPLUGIN:
      strcpy((char *)PluginParam,NullToEmpty((char *)Param));
      break;
    case FCTL_GETPANELINFO:
    case FCTL_GETANOTHERPANELINFO:
      if(Param == NULL)
        break;
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
      /* $ 24.04.2001 SVS
         Заполнение флагов PanelInfo.Flags
      */
      {
        static struct {
          int *Opt;
          DWORD Flags;
        } PFLAGS[]={
          {&Opt.ShowHidden,PFLAGS_SHOWHIDDEN},
          {&Opt.Highlight,PFLAGS_HIGHLIGHT},
          {&Opt.AutoChangeFolder,PFLAGS_AUTOCHANGEFOLDER},
          {&Opt.SelectFolders,PFLAGS_SELECTFOLDERS},
          {&Opt.ReverseSort,PFLAGS_ALLOWREVERSESORT},
        };

        DWORD Flags=0;

        for(int I=0; I < sizeof(PFLAGS)/sizeof(PFLAGS[0]); ++I)
          if(*(PFLAGS[I].Opt) != 0)
            Flags|=PFLAGS[I].Flags;

        Flags|=DestPanel->GetSortOrder()<0?PFLAGS_REVERSESORTORDER:0;
        Flags|=DestPanel->GetSortGroups()?PFLAGS_USESORTGROUPS:0;
        Flags|=DestPanel->GetSelectedFirstMode()?PFLAGS_SELECTEDFIRST:0;

        Info->Flags=Flags;
      }
      /* SVS $ */
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
        /* $ 12.05.2001 DJ
           перерисовываемся только в том случае, если мы - текущий фрейм
        */
        if (CtrlObject->Cp()->IsTopFrame())
          Redraw();
        /* DJ */
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
        /* $ 12.05.2001 DJ
           перерисовываемся только в том случае, если мы - текущий фрейм
        */
        if (CtrlObject->Cp()->IsTopFrame())
          AnotherPanel->Redraw();
        /* DJ */
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


int  Panel::GetCurName(char *Name,char *ShortName)
{
  *Name=*ShortName=0;
  return(FALSE);
}

static int MessageRemoveConnection(char Letter, int &UpdateProfile)
{
  int Len1, Len2, Len3,Len4;
  BOOL IsPersistent;
  char MsgText[NM];
/*
  0         1         2         3         4         5         6         7
  0123456789012345678901234567890123456789012345678901234567890123456789012345
0
1   +-------- Отключение сетевого устройства --------+
2   | Вы хотите удалить соединение с устройством C:? |
3   | На устройство %c: отображен каталог            |
4   | \\host\share                                   |
6   +------------------------------------------------+
7   | [ ] Восстанавливать при входе в систему        |
8   +------------------------------------------------+
9   |              [ Да ]   [ Отмена ]               |
10  +------------------------------------------------+
11
*/
  static struct DialogData DCDlgData[]=
  {
/*      Type          X1 Y1 X2  Y2 Focus Flags             DefaultButton
                                      Selected               Data
*/
/* 0 */ DI_DOUBLEBOX, 3, 1, 72, 9, 0, 0, 0,                0,"",
/* 1 */ DI_TEXT,      5, 2,  0, 0, 0, 0, DIF_SHOWAMPERSAND,0,"",
/* 2 */ DI_TEXT,      5, 3,  0, 0, 0, 0, DIF_SHOWAMPERSAND,0,"",
/* 3 */ DI_TEXT,      5, 4,  0, 0, 0, 0, DIF_SHOWAMPERSAND,0,"",
/* 4 */ DI_TEXT,      0, 5,  0, 6, 0, 0, DIF_SEPARATOR,    0,"",
/* 5 */ DI_CHECKBOX,  5, 6, 70, 5, 0, 0, 0,                0,"",
/* 6 */ DI_TEXT,      0, 7,  0, 6, 0, 0, DIF_SEPARATOR,    0,"",
/* 7 */ DI_BUTTON,    0, 8,  0, 0, 1, 0, DIF_CENTERGROUP,  1,"",
/* 8 */ DI_BUTTON,    0, 8,  0, 0, 0, 0, DIF_CENTERGROUP,  0,""
  };
  MakeDialogItems(DCDlgData,DCDlg);

  Len1=strlen(strcpy(DCDlg[0].Data,MSG(MChangeDriveDisconnectTitle)));
  sprintf(MsgText,MSG(MChangeDriveDisconnectQuestion),Letter);
  Len2=strlen(strcpy(DCDlg[1].Data,MsgText));
  sprintf(MsgText,MSG(MChangeDriveDisconnectMapped),Letter);
  Len4=strlen(strcpy(DCDlg[2].Data,MsgText));
  Len3=strlen(strcpy(DCDlg[5].Data,MSG(MChangeDriveDisconnectReconnect)));


  Len1=Max(Len1,Max(Len2,Max(Len3,Len4)));

  strcpy(DCDlg[3].Data,TruncPathStr(DriveLocalToRemoteName(DRIVE_REMOTE,Letter,MsgText),Len1));

  strcpy(DCDlg[7].Data,MSG(MYes));
  strcpy(DCDlg[8].Data,MSG(MCancel));

  // проверяем - это было постоянное соедение или нет?
  // Если ветка в реестре HKCU\Network\БукваДиска есть - это
  //   есть постоянное подключение.
  {
    HKEY hKey;
    IsPersistent=TRUE;
    sprintf(MsgText,"Network\\%c",toupper(Letter));
    if(RegOpenKeyEx(HKEY_CURRENT_USER,MsgText,0,KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
    {
      DCDlg[5].Flags|=DIF_DISABLE;
      DCDlg[5].Selected=0;
      IsPersistent=FALSE;
    }
    else
      DCDlg[5].Selected=Opt.ChangeDriveDisconnetMode;
    RegCloseKey(hKey);
  }

  // скорректируем размеры диалога - для дизайнУ
  DCDlg[0].X2=DCDlg[0].X1+Len1+3;

  int ExitCode=7;
  if(Opt.Confirm.RemoveConnection)
  {
    Dialog Dlg(DCDlg,sizeof(DCDlg)/sizeof(DCDlg[0]));
    Dlg.SetPosition(-1,-1,DCDlg[0].X2+4,11);
    Dlg.SetHelp("DisconnectDrive");
    Dlg.Process();
    ExitCode=Dlg.GetExitCode();
  }
  UpdateProfile=DCDlg[5].Selected?0:CONNECT_UPDATE_PROFILE;
  if(IsPersistent)
    Opt.ChangeDriveDisconnetMode=DCDlg[5].Selected;
  return ExitCode == 7;
}
