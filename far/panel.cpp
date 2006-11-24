/*
panel.cpp

Parent class для панелей

*/

/* Revision: 1.173 07.07.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "panel.hpp"
#include "plugin.hpp"
#include "macroopcode.hpp"
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
#include "array.hpp"
#include "lockscrn.hpp"
#include "help.hpp"

static int DragX,DragY,DragMove;
static Panel *SrcDragPanel;
static SaveScreen *DragSaveScr=NULL;
static string strDragName;

//static wchar_t VerticalLine=/*0x0B3*/0x2502;

static int MessageRemoveConnection(wchar_t Letter, int &UpdateProfile);

/* $ 21.08.2002 IS
   Класс для хранения пункта плагина в меню выбора дисков
*/
class ChDiskPluginItem
{
  public:
   MenuItemEx Item;
   unsigned int HotKey;
   ChDiskPluginItem()
   {
     Clear ();
   }
   void Clear ()
   {
        HotKey = 0;
        Item.Clear ();
   }
   bool operator==(const ChDiskPluginItem &rhs) const;
   int operator<(const ChDiskPluginItem &rhs) const;
   const ChDiskPluginItem& operator=(const ChDiskPluginItem &rhs);
   ~ChDiskPluginItem()
   {
   }
};

bool ChDiskPluginItem::operator==(const ChDiskPluginItem &rhs) const
{
  return HotKey==rhs.HotKey &&
    !LocalStricmpW(Item.strName,rhs.Item.strName) &&
    Item.UserData==rhs.Item.UserData;
}

int ChDiskPluginItem::operator<(const ChDiskPluginItem &rhs) const
{
  if(HotKey==rhs.HotKey)
    return LocalStricmpW(Item.strName,rhs.Item.strName)<0;
  else if(HotKey && rhs.HotKey)
    return HotKey < rhs.HotKey;
  else
    return HotKey && !rhs.HotKey;
}

const ChDiskPluginItem& ChDiskPluginItem::operator=(const ChDiskPluginItem &rhs)
{
  Item=rhs.Item;
  HotKey=rhs.HotKey;
  return *this;
}
/* IS $ */


Panel::Panel()
{
  _OT(SysLog("[%p] Panel::Panel()", this));
  Focus=0;
  NumericSort=0;
  PanelMode=NORMAL_PANEL;
  PrevViewMode=VIEW_3;
  EnableUpdate=TRUE;
  DragX=DragY=-1;
  SrcDragPanel=NULL;
  ModalMode=0;
  ViewSettings.ColumnCount=0;
  ViewSettings.FullScreen=0;
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
  string strNewDir;
  FarGetCurDirW(strNewDir);
  SetCurDirW(strNewDir,TRUE);
}


void Panel::ChangeDisk()
{
  int Pos,FirstCall=TRUE;
  if ( !strCurDir.IsEmpty() && strCurDir.At(1)==L':')
    Pos=LocalUpperW(strCurDir.At(0))-L'A';
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
  class Guard_Macro_DskShowPosType{
    public:
      Guard_Macro_DskShowPosType(Panel *curPanel){Macro_DskShowPosType=(curPanel==CtrlObject->Cp()->LeftPanel)?1:2;};
     ~Guard_Macro_DskShowPosType(){Macro_DskShowPosType=0;};
  };

  Guard_Macro_DskShowPosType _guard_Macro_DskShowPosType(this);

  MenuItemEx ChDiskItem;
  string strDiskType, strRootDir, strDiskLetter;
  DWORD Mask,DiskMask;
  int DiskCount,Focus,I,J;
  int ShowSpecial=FALSE, SetSelected=FALSE;

  ChDiskItem.Clear ();

  _tran(SysLog("Panel::ChangeDiskMenu(), Pos=%i, FirstCall=%i",Pos,FirstCall));
  Mask=FarGetLogicalDrives();

  for (DiskMask=Mask,DiskCount=0;DiskMask!=0;DiskMask>>=1)
    DiskCount+=DiskMask & 1;

  int UserDataSize=0;
  DWORD UserData=0;
  {
    _tran(SysLog("create VMenu ChDisk"));
    VMenu ChDisk(UMSG(MChangeDriveTitle),NULL,0,TRUE,ScrY-Y1-3);
    ChDisk.SetFlags(VMENU_NOTCENTER);
    if ( this==CtrlObject->Cp()->LeftPanel){
      ChDisk.SetFlags(VMENU_LEFTMOST);
    }
    ChDisk.SetHelp(L"DriveDlg");
    /* $ 17.06.2001 KM
       ! Добавление WRAPMODE в меню.
    */
    ChDisk.SetFlags(VMENU_WRAPMODE);
    /* KM $ */

    string strMenuText;
    int DriveType,MenuLine;
    int LabelWidth=Max(11,(int)wcslen(UMSG(MChangeDriveLabelAbsent)));

    /* $ 02.04.2001 VVM
      ! Попытка не будить спящие диски... */
    for (DiskMask=Mask,MenuLine=I=0;DiskMask!=0;DiskMask>>=1,I++)
    {
      if (DiskMask & 1)
      {
        strMenuText.Format (L"&%c: ",L'A'+I);
        strRootDir.Format (L"%c:\\",L'A'+I);
        DriveType = FAR_GetDriveTypeW(strRootDir,NULL,Opt.ChangeDriveMode & DRIVE_SHOW_CDROM?0x01:0);
        if (Opt.ChangeDriveMode & DRIVE_SHOW_TYPE)
        {
          static struct TypeMessage{
            int DrvType;
            int FarMsg;
          } DrTMsg[]={
            {DRIVE_REMOVABLE,MChangeDriveRemovable},
            {DRIVE_FIXED,MChangeDriveFixed},
            {DRIVE_REMOTE,MChangeDriveNetwork},
            {DRIVE_CDROM,MChangeDriveCDROM},
            {DRIVE_CD_RW,MChangeDriveCD_RW},
            {DRIVE_CD_RWDVD,MChangeDriveCD_RWDVD},
            {DRIVE_DVD_ROM,MChangeDriveDVD_ROM},
            {DRIVE_DVD_RW,MChangeDriveDVD_RW},
            {DRIVE_DVD_RAM,MChangeDriveDVD_RAM},
            {DRIVE_RAMDISK,MChangeDriveRAM},
          };
          for(J=0; J < sizeof(DrTMsg)/sizeof(DrTMsg[1]); ++J)
            if(DrTMsg[J].DrvType == DriveType)
            {
              strDiskType = UMSG(DrTMsg[J].FarMsg);
              _SVS(SysLog("DriveType=%d, DiskType='%S'",DriveType,(const wchar_t*)strDiskType));
              break;
            }

          if(J >= sizeof(DrTMsg)/sizeof(DrTMsg[1]))
            strDiskType.Format (L"%*s",wcslen(UMSG(MChangeDriveFixed)),L"");

          /* 05.01.2001 SVS
             + Информация про Subst-тип диска
          */
          {
            string strLocalName;
            string strSubstName;

            strLocalName.Format (L"%c:",strRootDir.At(0));
            if(GetSubstNameW(DriveType,strLocalName,strSubstName))
            {
              strDiskType = UMSG(MChangeDriveSUBST);
              DriveType=DRIVE_SUBSTITUTE;
            }
          }
          /* SVS $ */

          strMenuText += strDiskType;
        }

        int ShowDisk = (DriveType!=DRIVE_REMOVABLE || (Opt.ChangeDriveMode & DRIVE_SHOW_REMOVABLE)) &&
                       (!IsDriveTypeCDROM(DriveType) || (Opt.ChangeDriveMode & DRIVE_SHOW_CDROM)) &&
                       (DriveType!=DRIVE_REMOTE || (Opt.ChangeDriveMode & DRIVE_SHOW_REMOTE));
        if (Opt.ChangeDriveMode & (DRIVE_SHOW_LABEL|DRIVE_SHOW_FILESYSTEM))
        {
          string strVolumeName, strFileSystemName;

          if (ShowDisk && !apiGetVolumeInformation (strRootDir,&strVolumeName,NULL,NULL,NULL,&strFileSystemName))
          {
            strVolumeName = UMSG(MChangeDriveLabelAbsent);
            ShowDisk=FALSE;
          }

          if (Opt.ChangeDriveMode & DRIVE_SHOW_LABEL)
          {
            /* $ 01.10.2001 IS метку усекаем с конца */
            TruncStrFromEndW(strVolumeName,LabelWidth);
            /* IS $ */
            string strTemp;
            strTemp.Format (L"%c%-*s",(WORD)VerticalLine,LabelWidth,(const wchar_t*)strVolumeName);

            strMenuText += strTemp;
          }
          if (Opt.ChangeDriveMode & DRIVE_SHOW_FILESYSTEM)
          {
              string strTemp;
              strTemp.Format (L"%c%-8.8s",(WORD)VerticalLine,(const wchar_t*)strFileSystemName);

              strMenuText += strTemp;
          }
        }

        if (Opt.ChangeDriveMode & (DRIVE_SHOW_SIZE|DRIVE_SHOW_SIZE_FLOAT))
        {
          string strTotalText, strFreeText;
          unsigned __int64 TotalSize,TotalFree,UserFree;
          if (ShowDisk && GetDiskSizeW(strRootDir,&TotalSize,&TotalFree,&UserFree))
          {
            if (Opt.ChangeDriveMode & DRIVE_SHOW_SIZE)
            {
              //размер как минимум в мегабайтах
              FileSizeToStrW(strTotalText,TotalSize,8,COLUMN_MINSIZEINDEX|1);
              FileSizeToStrW(strFreeText,UserFree,8,COLUMN_MINSIZEINDEX|1);
            }
            else
            {
              //размер с точкой и для 0 добавляем букву размера (B)
              FileSizeToStrW(strTotalText,TotalSize,8,COLUMN_FLOATSIZE|COLUMN_SHOWBYTESINDEX);
              FileSizeToStrW(strFreeText,UserFree,8,COLUMN_FLOATSIZE|COLUMN_SHOWBYTESINDEX);
            }
          }

          string strTemp;
          strTemp.Format(L"%c%-8s%c%-8s",(WORD)VerticalLine,(const wchar_t*)strTotalText,(WORD)VerticalLine,(const wchar_t*)strFreeText);

          strMenuText += strTemp;
        }

        if (Opt.ChangeDriveMode & DRIVE_SHOW_NETNAME)
        {
          string strRemoteName;
          DriveLocalToRemoteNameW(DriveType,strRootDir.At(0),strRemoteName);
          TruncPathStrW(strRemoteName,ScrX-strMenuText.GetLength()-12);
          if( !strRemoteName.IsEmpty() )
          {
            strMenuText += L"  ";
            strMenuText += strRemoteName;
          }
          ShowSpecial=TRUE;
        }

        if (FirstCall)
        {
          ChDiskItem.SetSelect(I==Pos);
          if(!SetSelected)
            SetSelected=(I==Pos);
        }
        else if(Pos < DiskCount)
        {
          ChDiskItem.SetSelect(MenuLine==Pos);
          if(!SetSelected)
            SetSelected=(MenuLine==Pos);
        }

        ChDiskItem.strName = strMenuText;
        if ( strMenuText.GetLength()>4)
          ShowSpecial=TRUE;

        UserData=MAKELONG(MAKEWORD('A'+I,0),DriveType);
        ChDisk.SetUserData((char*)UserData,sizeof(UserData),
                 ChDisk.AddItemW(&ChDiskItem));
        MenuLine++;
      } // if (DiskMask & 1)
    } // for
    /* VVM $ */

    /* $ 21.01.2002 IS
       Снимем ограничение на количество пунктов плагинов в меню вообще(!)
    */
    /* $ 22.01.2002 IS
       Автохоткеи назначаем после основных
    */
    int PluginMenuItemsCount=0;
    if (Opt.ChangeDriveMode & DRIVE_SHOW_PLUGINS)
    {
      int UsedNumbers[10];
      memset(UsedNumbers,0,sizeof(UsedNumbers));
      TArray<ChDiskPluginItem> MPItems, MPItemsNoHotkey;
      ChDiskPluginItem OneItem;
      /* $ 15.03.2001 IS
           Список дополнительных хоткеев, для случая, когда плагинов,
           добавляющих пункт в меню, больше 9.
      */
      char *AdditionalHotKey=MSG(MAdditionalHotKey);
      int AHKPos=0,                           // индекс в списке хоткеев
          AHKSize=strlen(AdditionalHotKey);   /* для предотвращения выхода за
                                                 границу массива */
      /* IS $ */

      int PluginNumber=0, PluginItem; // IS: счетчики - плагинов и пунктов плагина
      int PluginTextNumber, ItemPresent, HotKey, Done=FALSE;
      string strPluginText;

      while(!Done)
      {
        for (PluginItem=0;;++PluginItem)
        {
          if (!CtrlObject->Plugins.GetDiskMenuItem(PluginNumber,PluginItem,
              ItemPresent,PluginTextNumber,strPluginText))
          {
            Done=TRUE;
            break;
          }
          if (!ItemPresent)
            break;

          if(!PluginTextNumber) // IS: автохоткей, назначим потом
            HotKey=-1; // "-1" -  признак автохоткея
          else
          {
            if(PluginTextNumber<10) // IS: хотей указан явно
            {
              // IS: проверим, а не занят ли хоткей
              // IS: если занят, то будем искать его с самого начала - нуля,
              // IS: а не со следующего
              if(UsedNumbers[PluginTextNumber])
              {
                PluginTextNumber=0;
                while (PluginTextNumber<10 && UsedNumbers[PluginTextNumber])
                  PluginTextNumber++;
              }
              UsedNumbers[PluginTextNumber%10]=1;
            }

            if(PluginTextNumber<10)
              HotKey=PluginTextNumber+L'0';
            else if(AHKPos<AHKSize)
              HotKey=AdditionalHotKey[AHKPos];
            else
              HotKey=0;
          }

          /* $ 22.08.2002 IS
               Используем дополнительные хоткеи, а не просто '#', как раньше.
          */
          strMenuText=L"";
          if(HotKey<0)
            strMenuText = ShowSpecial?strPluginText:L"";

          else if(PluginTextNumber<10)
          {
            strMenuText.Format (L"&%c: %s", HotKey, ShowSpecial ? (const wchar_t*)strPluginText:L"");
          }
          else if(AHKPos<AHKSize)
          {
            strMenuText.Format (L"&%c: %s", HotKey, ShowSpecial ? (const wchar_t*)strPluginText:L"");
            ++AHKPos;
          }
          else if(ShowSpecial) // IS: не добавляем пустые строки!
          {
            HotKey=0;
            strMenuText.Format (L"   %s", (const wchar_t*)strPluginText);
          }
          /* IS $ */
          if(HotKey>-1 && !strMenuText.IsEmpty()) // IS: не добавляем пустые строки!
          {
            OneItem.Item.strName = strMenuText;
            OneItem.Item.UserDataSize=0;
            OneItem.Item.UserData=(char*)MAKELONG(PluginNumber,PluginItem);
            OneItem.HotKey=HotKey;
            if(!MPItems.addItem(OneItem))
            {
              Done=TRUE;
              break;
            }
          }
          else if(HotKey<0) // IS: назначение автохоткеей отложим на потом
          {
            OneItem.Item.strName = strMenuText;
            OneItem.Item.UserDataSize=0;
            OneItem.Item.UserData=(char*)MAKELONG(PluginNumber,PluginItem);
            OneItem.HotKey=HotKey;
            if(!MPItemsNoHotkey.addItem(OneItem))
            {
              Done=TRUE;
              break;
            }
          }
        } // END: for (PluginItem=0;;++PluginItem)

        ++PluginNumber;
      }

      // IS: теперь произведем назначение автохоткеев
      PluginTextNumber=0;
      for(int i=0;;++i)
      {
        ChDiskPluginItem *item=MPItemsNoHotkey.getItem(i);
        if(item)
        {
          if(UsedNumbers[PluginTextNumber])
          {
            while (PluginTextNumber<10 && UsedNumbers[PluginTextNumber])
              PluginTextNumber++;
          }
          UsedNumbers[PluginTextNumber%10]=1;

          strMenuText=L"";
          if(PluginTextNumber<10)
          {
            item->HotKey=PluginTextNumber+'0';
            strMenuText.Format (L"&%c: %s", item->HotKey, (const wchar_t*)item->Item.strName);
          }
          else if(AHKPos<AHKSize)
          {
            item->HotKey=AdditionalHotKey[AHKPos];
            strMenuText.Format (L"&%c: %s", item->HotKey, (const wchar_t*)item->Item.strName);
            ++AHKPos;
          }
          else if(ShowSpecial) // IS: не добавляем пустые строки!
          {
            item->HotKey=0;
            strMenuText.Format (L"   %s", (const wchar_t*)item->Item.strName);
          }

          item->Item.strName = strMenuText;
          if( !item->Item.strName.IsEmpty() && !MPItems.addItem(*item))
            break;
        }
        else
          break;
      }

      MPItems.Sort();
      MPItems.Pack(); // выкинем дубли
      PluginMenuItemsCount=MPItems.getSize();
      if(PluginMenuItemsCount)
      {
        ChDiskItem.Clear ();
        ChDiskItem.Flags|=LIF_SEPARATOR;
        ChDiskItem.UserDataSize=0;
        ChDisk.AddItemW(&ChDiskItem);

        ChDiskItem.Flags&=~LIF_SEPARATOR;
        for (int I=0;I<PluginMenuItemsCount;++I)
        {
          if(Pos > DiskCount && !SetSelected)
          {
            MPItems.getItem(I)->Item.SetSelect(DiskCount+I+1==Pos);
            if(!SetSelected)
              SetSelected=DiskCount+I+1==Pos;
          }
          ChDisk.AddItemW(&MPItems.getItem(I)->Item);
        }
      }
    }
    /* IS 22.08.2002 $ */
    /* IS 21.08.2002 $ */

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
            if ((UserData=(DWORD)ChDisk.GetUserData(NULL,0)) != 0)
            {
              string strDosDeviceName;
              strDosDeviceName.Format (L"%c:\\",LOWORD(UserData)); //BUGBUG
              Execute(strDosDeviceName,FALSE,TRUE,TRUE);
            }
          }
          break;
        case KEY_CTRLPGUP:  case KEY_CTRLNUMPAD9:
          if(Opt.PgUpChangeDisk)
            return -1;
          break;
        /* $ 27.04.2001 SVS
           Т.к. нет способа получить состояние "открытости" устройства,
           то добавим обработку Ins для CD - "закрыть диск"
        */
        case KEY_INS:       case KEY_NUMPAD0:
          if (SelPos<DiskCount)// && WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
          {
//            char MsgText[200], LocalName[50];
            if ((UserData=(DWORD)ChDisk.GetUserData(NULL,0)) != 0)
            {
              DriveType=HIWORD(UserData);
              if(IsDriveTypeCDROM(DriveType) /* || DriveType == DRIVE_REMOVABLE*/)
              {
                SaveScreen SvScrn;
                EjectVolume(LOWORD(UserData),EJECT_LOAD_MEDIA);
                return(SelPos);
              }
            }
          }
          break;
        /* SVS $ */
        case KEY_DEL:
          if (SelPos<DiskCount)
          {
            /* $ 28.12.2001 DJ
               обработка Del вынесена в отдельную функцию
            */
            if ((UserData=(DWORD)ChDisk.GetUserData(NULL,0)) != 0)
            {
              if(HIWORD(UserData) == DRIVE_REMOVABLE || IsDriveTypeCDROM(HIWORD(UserData)))
              {
                if(HIWORD(UserData) == DRIVE_REMOVABLE && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT && !IsEjectableMedia(LOWORD(UserData)))
                  break;

                // первая попытка извлеч диск
                if(!EjectVolume(LOWORD(UserData),EJECT_NO_MESSAGE))
                {
                  // запоминаем состояние панелей
                  int CMode=GetMode();
                  int AMode=CtrlObject->Cp()->GetAnotherPanel (this)->GetMode();

                  string strTmpCDir, strTmpADir;

                  GetCurDirW (strTmpCDir);
                  CtrlObject->Cp()->GetAnotherPanel (this)->GetCurDirW (strTmpADir);

                  // отключим меню, иначе бага с прорисовкой этой самой меню
                  // (если меню поболее высоты экрана)
                  ChDisk.Hide();
                  ChDisk.Lock(); // ... и запретим ее перерисовку.

                  // "цикл до умопомрачения"
                  int DoneEject=FALSE;
                  while(!DoneEject)
                  {
                    // "освободим диск" - перейдем при необходимости в домашний каталог
                    // TODO: А если домашний каталог - CD? ;-)
                    IfGoHomeW(LOWORD(UserData));
                    // очередная попытка извлечения без вывода сообщения
                    int ResEject=EjectVolume(LOWORD(UserData),EJECT_NO_MESSAGE);
                    if(!ResEject)
                    {
                      // восстановим пути - это избавит нас от левых данных в панели.
                      if (AMode != PLUGIN_PANEL)
                        CtrlObject->Cp()->GetAnotherPanel (this)->SetCurDirW (strTmpADir, FALSE);
                      if (CMode != PLUGIN_PANEL)
                        SetCurDirW (strTmpCDir, FALSE);

                      // ... и выведем месаг о...
                      string strMsgText;
                      strMsgText.Format (UMSG(MChangeCouldNotEjectMedia), LOWORD(UserData));
                      SetLastError(ERROR_DRIVE_LOCKED); // ...о "The disk is in use or locked by another process."
                      DoneEject=MessageW(MSG_WARNING|MSG_ERRORTYPE,2,UMSG(MError),strMsgText,UMSG(MRetry),UMSG(MCancel))!=0;
                    }
                    else
                      DoneEject=TRUE;
                  }

                  // "отпустим" менюху выбора дисков
                  ChDisk.Unlock();
                  ChDisk.Show();
                }
              }
              else
              {
                int Code = ProcessDelDisk (LOWORD(UserData), HIWORD(UserData), &ChDisk);
                if (Code != DRIVE_DEL_FAIL)
                {
                  // "BugZ#640 - отрисовка" - обновим экран.
                  ScrBuf.Lock(); // отменяем всякую прорисовку
                  FrameManager->ResizeAllFrame();
                  FrameManager->PluginCommit(); // коммитим.
                  ScrBuf.Unlock(); // разрешаем прорисовку

                  // 19.01.2002 VVM  + Если диск был последним - в конце и останемся
                  return (((DiskCount-SelPos)==1) && (SelPos > 0) && (Code != DRIVE_DEL_EJECT))?SelPos-1:SelPos;
                }
              }
            }
            /* DJ $ */
          }
          break;
        case KEY_SHIFTDEL:
          if (SelPos<DiskCount)
          {
            if ((UserData=(DWORD)ChDisk.GetUserData(NULL,0)) != 0 && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
            {
              // первая попытка удалить устройство
              int Code=ProcessRemoveHotplugDevice(LOWORD(UserData),EJECT_NOTIFY_AFTERREMOVE);
              if(Code == 0)
              {
                // запоминаем состояние панелей
                int CMode=GetMode();
                int AMode=CtrlObject->Cp()->GetAnotherPanel (this)->GetMode();

                string strTmpCDir, strTmpADir;

                GetCurDirW (strTmpCDir);
                CtrlObject->Cp()->GetAnotherPanel (this)->GetCurDirW (strTmpADir);

                // отключим меню, иначе бага с прорисовкой этой самой меню
                // (если меню поболее высоты экрана)
                ChDisk.Hide();
                ChDisk.Lock(); // ... и запретим ее перерисовку.

                // "цикл до умопомрачения"
                int DoneEject=FALSE;
                while(!DoneEject)
                {
                  // "освободим диск" - перейдем при необходимости в домашний каталог
                  // TODO: А если домашний каталог - USB? ;-)
                  IfGoHomeW(LOWORD(UserData));
                  // очередная попытка извлечения без вывода сообщения
                  Code=ProcessRemoveHotplugDevice(LOWORD(UserData),EJECT_NO_MESSAGE|EJECT_NOTIFY_AFTERREMOVE);
                  if(Code == 0)
                  {
                    // восстановим пути - это избавит нас от левых данных в панели.
                    if (AMode != PLUGIN_PANEL)
                      CtrlObject->Cp()->GetAnotherPanel (this)->SetCurDirW (strTmpADir, FALSE);
                    if (CMode != PLUGIN_PANEL)
                      SetCurDirW (strTmpCDir, FALSE);

                    // ... и выведем месаг о...
                    string strMsgText;
                    strMsgText.Format (UMSG(MChangeCouldNotEjectHotPlugMedia), LOWORD(UserData));
                    SetLastError(ERROR_DRIVE_LOCKED); // ...о "The disk is in use or locked by another process."
                    DoneEject=MessageW(MSG_WARNING|MSG_ERRORTYPE,2,UMSG(MError),strMsgText,UMSG(MHRetry),UMSG(MHCancel))!=0;
                  }
                  else
                    DoneEject=TRUE;
                }

                // "отпустим" менюху выбора дисков
                ChDisk.Unlock();
                ChDisk.Show();
              }

              return(SelPos);
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
          if (Opt.ChangeDriveMode&DRIVE_SHOW_SIZE)
          {
            Opt.ChangeDriveMode^=DRIVE_SHOW_SIZE;
            Opt.ChangeDriveMode|=DRIVE_SHOW_SIZE_FLOAT;
          }
          else if (Opt.ChangeDriveMode&DRIVE_SHOW_SIZE_FLOAT)
          {
            Opt.ChangeDriveMode^=DRIVE_SHOW_SIZE_FLOAT;
          }
          else
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
        case KEY_CTRL9:
        case KEY_RCTRL9:
          Opt.ChangeDriveMode^=DRIVE_SHOW_REMOTE;
          return(SelPos);
        /* $ 27.03.2001 SVS
          Shift-F1 на пункте плагина в меню выбора дисков тоже покажет хелп...
        */
        case KEY_SHIFTF1:
          if (SelPos>DiskCount)
          {
            // Вызываем нужный топик, который передали в CommandsMenu()
            if ((UserData=(DWORD)ChDisk.GetUserData(NULL,0)) != 0)
            {
              FarShowHelp(CtrlObject->Plugins.PluginsData[LOWORD(UserData)]->strModuleName,
                    NULL,FHELP_SELFHELP|FHELP_NOSHOWERROR|FHELP_USECONTENTS);
            }
          }
          break;
        /* SVS $ */
        case KEY_CTRLR:
          return(SelPos);

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
      if (ChDisk.Done() && ChDisk.Modal::GetExitCode()<0 && !strCurDir.IsEmpty() && wcsncmp(strCurDir,L"\\\\",2)!=0)
      {
        wchar_t RootDir[10]; //BUGBUG
        wcsncpy(RootDir,strCurDir,3);
        RootDir[3]=0;
        if (FAR_GetDriveTypeW(RootDir)==DRIVE_NO_ROOT_DIR)
          ChDisk.ClearDone();
      }
      /* DJ $ */
      /* SVS $ */
      /* tran $ */
      /* SVS $ */
    } // while (!Done)
    if (ChDisk.Modal::GetExitCode()<0)
      return(-1);
    {
      UserDataSize=ChDisk.Modal::GetExitCode()>DiskCount?2:3;
      UserData=(DWORD)ChDisk.GetUserData(NULL,0);
    }
  }

  if(Opt.CloseCDGate && UserData != 0 && IsDriveTypeCDROM(HIWORD(UserData)) && UserDataSize == 3)
  {
    strRootDir.Format (L"%c:",LOWORD(UserData));
    if(!IsDiskInDriveW(strRootDir))
    {
      if(EjectVolume(LOWORD(UserData),EJECT_READY|EJECT_NO_MESSAGE))
      {
        SaveScreen SvScrn;
        MessageW(0,0,L"",UMSG(MChangeWaitingLoadDisk));
        EjectVolume(LOWORD(UserData),EJECT_LOAD_MEDIA|EJECT_NO_MESSAGE);
      }
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
      int NumDisk=LOWORD(UserData)-'A';

      string strMsgStr;
      string strNewDir;

      strNewDir.Format (L"%c:", LOWORD(UserData));

      FarChDirW(strNewDir);
      CtrlObject->CmdLine->GetCurDirW(strNewDir);

      strNewDir.Upper();

      if ( strNewDir.At (0) == LOWORD(UserData))
        FarChDirW(strNewDir);
      if (getdisk()!=NumDisk)
      {
        string strRootDir;
        strRootDir.Format (L"%c:\\", LOWORD(UserData));
        FarChDirW(strRootDir);
        if (getdisk()==NumDisk)
          break;
      }
      else
        break;

      strMsgStr.Format (UMSG(MChangeDriveCannotReadDisk), LOWORD(UserData));

      if (MessageW(MSG_WARNING,2,UMSG(MError),strMsgStr, UMSG(MRetry), UMSG(MCancel))!=0)
        return(-1);
    }

    string strNewCurDir;
    FarGetCurDirW (strNewCurDir);
    // BugZ#208. Если пути совпадают, то ничего не делаем.
    //_tran(SysLog("PanelMode=%i (%i), CurDir=[%s], NewCurDir=[%s]",PanelMode,GetType(),
    //            CurDir,NewCurDir);)

    if(PanelMode == NORMAL_PANEL && GetType()==FILE_PANEL && !LocalStricmpW(strCurDir,strNewCurDir) && IsVisible())
    {
      // А нужно ли делать здесь Update????
      Update(UPDATE_KEEP_SELECTION);
    }
    else
    {
      Focus=GetFocus();
      Panel *NewPanel=CtrlObject->Cp()->ChangePanel(this,FILE_PANEL,TRUE,FALSE);
      NewPanel->SetCurDirW(strNewCurDir,TRUE);
      NewPanel->Show();
      if (Focus || !CtrlObject->Cp()->GetAnotherPanel(this)->IsVisible())
        NewPanel->SetFocus();
    }
  }
  else
    if (UserDataSize==2)
    {
      HANDLE hPlugin=CtrlObject->Plugins.OpenPlugin(LOWORD(UserData),OPEN_DISKMENU,HIWORD(UserData));
      if (hPlugin!=INVALID_HANDLE_VALUE)
      {
        Focus=GetFocus();
        Panel *NewPanel=CtrlObject->Cp()->ChangePanel(this,FILE_PANEL,TRUE,TRUE);
        NewPanel->SetPluginMode(hPlugin,L"");
        NewPanel->Update(0);
        NewPanel->Show();
        if (Focus || !CtrlObject->Cp()->GetAnotherPanel(NewPanel)->IsVisible())
          NewPanel->SetFocus();
      }
    }
  return(-1);
}

/* $ 28.12.2001 DJ
   обработка Del в меню дисков
*/

int Panel::ProcessDelDisk (wchar_t Drive, int DriveType,VMenu *ChDiskMenu)
{
  string strMsgText;
  int UpdateProfile=CONNECT_UPDATE_PROFILE;
  BOOL Processed=FALSE;

  wchar_t DiskLetter [4];
  DiskLetter[0] = Drive;
  DiskLetter[1] = L':';
  DiskLetter[2] = 0;

  if(DriveType == DRIVE_REMOTE && MessageRemoveConnection(Drive,UpdateProfile))
    Processed=TRUE;

  // <КОСТЫЛЬ>
  if(Processed)
  {
    LockScreen LckScr;
    // если мы находимся на удаляемом диске - уходим с него, чтобы не мешать
    // удалению
    IfGoHomeW(Drive);
    FrameManager->ResizeAllFrame();
    FrameManager->GetCurrentFrame()->Show();
    ChDiskMenu->Show();
  }
  // </КОСТЫЛЬ>

  /* $ 05.01.2001 SVS
     Пробуем удалить SUBST-драйв.
  */
  if(DriveType == DRIVE_SUBSTITUTE)
  {
    if(Opt.Confirm.RemoveSUBST)
    {
      string strMsgText;
      strMsgText.Format (UMSG(MChangeSUBSTDisconnectDriveQuestion),Drive);
      if(MessageW(MSG_WARNING,2,UMSG(MChangeSUBSTDisconnectDriveTitle),strMsgText,UMSG(MYes),UMSG(MNo))!=0)
        return DRIVE_DEL_FAIL;
    }

    if(!DelSubstDrive(DiskLetter))
      return DRIVE_DEL_SUCCESS;
    else
    {
      int LastError=GetLastError();
      strMsgText.Format (UMSG(MChangeDriveCannotDelSubst),DiskLetter);
      if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
        if (MessageW(MSG_WARNING|MSG_ERRORTYPE,2,UMSG(MError),strMsgText,
                L"\x1",UMSG(MChangeDriveOpenFiles),
                UMSG(MChangeDriveAskDisconnect),UMSG(MOk),UMSG(MCancel))==0)
        {
          if(!DelSubstDrive(DiskLetter))
            return DRIVE_DEL_SUCCESS;
        }
        else
          return DRIVE_DEL_FAIL;
      MessageW(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MError),strMsgText,UMSG(MOk));
    }
    return DRIVE_DEL_FAIL; // блин. в прошлый раз забыл про это дело...
  }
  /* SVS $ */

  if(Processed)
  {
    if (WNetCancelConnection2W(DiskLetter,UpdateProfile,FALSE)==NO_ERROR)
      return DRIVE_DEL_SUCCESS;
    else
    {
      int LastError=GetLastError();
      strMsgText.Format (UMSG(MChangeDriveCannotDisconnect),DiskLetter);
      if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
        if (MessageW(MSG_WARNING|MSG_ERRORTYPE,2,UMSG(MError),strMsgText,
                L"\x1",UMSG(MChangeDriveOpenFiles),
                UMSG(MChangeDriveAskDisconnect),UMSG(MOk),UMSG(MCancel))==0)
        {
          if (WNetCancelConnection2W(DiskLetter,UpdateProfile,TRUE)==NO_ERROR)
            return DRIVE_DEL_SUCCESS;
        }
        else
          return DRIVE_DEL_FAIL;
      string strRootDir;
      strRootDir.Format (L"%c:\\", *DiskLetter);
      if (FAR_GetDriveTypeW(strRootDir)==DRIVE_REMOTE)
        MessageW(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MError),strMsgText,UMSG(MOk));
    }
    return DRIVE_DEL_FAIL;
  }
  return DRIVE_DEL_FAIL;
}
/* DJ $ */


void Panel::FastFindProcessName(Edit *FindEdit,const wchar_t *Src,string &strLastName,string &strName)
{
   wchar_t *Ptr=(wchar_t *)xf_malloc((wcslen(Src)+wcslen(FindEdit->GetStringAddrW())+32)*sizeof (wchar_t));
    if(Ptr)
    {
      wcscpy(Ptr,FindEdit->GetStringAddrW());
      wchar_t *EndPtr=Ptr+wcslen(Ptr);
      wcscat(Ptr,Src);
      UnquoteW(EndPtr);

      EndPtr=Ptr+wcslen(Ptr);
      DWORD Key;
      while(1)
      {
        if(EndPtr == Ptr)
        {
          Key=KEY_NONE;
          break;
        }

        if (FindPartName(Ptr,FALSE))
        {
          Key=*(EndPtr-1);
          *EndPtr=0;
          FindEdit->SetStringW(Ptr);
          strLastName = Ptr;
          strName = Ptr;
          FindEdit->Show();
          break;
        }

        *--EndPtr=0;
      }
      xf_free(Ptr);
    }
}

// корректировка букв
static DWORD _CorrectFastFindKbdLayout(INPUT_RECORD *rec,DWORD Key)
{
  if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && (Key&KEY_ALT))// && Key!=(KEY_ALT|0x3C))
  {
    // // _SVS(SysLog("_CorrectFastFindKbdLayout>>> %s | %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(rec)));
    if(rec->Event.KeyEvent.uChar.AsciiChar && (Key&KEY_MASKF) != rec->Event.KeyEvent.uChar.AsciiChar) //???
      Key=(Key&0xFFFFFF00)|rec->Event.KeyEvent.uChar.AsciiChar;   //???
    // // _SVS(SysLog("_CorrectFastFindKbdLayout<<< %s | %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(rec)));
  }
  return Key;
}

void Panel::FastFind(int FirstKey)
{
  // // _SVS(CleverSysLog Clev("Panel::FastFind"));
  INPUT_RECORD rec;
  string strLastName, strName;
  int Key,KeyToProcess=0;
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
      {
        FirstKey=_CorrectFastFindKbdLayout(FrameManager->GetLastInputRecord(),FirstKey);
        // // _SVS(SysLog("Panel::FastFind  FirstKey=%s  %s",_FARKEY_ToName(FirstKey),_INPUT_RECORD_Dump(FrameManager->GetLastInputRecord())));
        // // _SVS(SysLog("if (FirstKey)"));
        Key=FirstKey;
      }
      else
      {
        // // _SVS(SysLog("else if (FirstKey)"));
        Key=GetInputRecord(&rec);
        if (rec.EventType==MOUSE_EVENT)
        {
          if ((rec.Event.MouseEvent.dwButtonState & 3)==0)
            continue;
          else
            Key=KEY_ESC;
        }
        else if (!rec.EventType || rec.EventType==KEY_EVENT || rec.EventType==FARMACRO_KEY_EVENT)
        {
          // для вставки воспользуемся макродвижком...
          if(Key==KEY_CTRLV || Key==KEY_SHIFTINS || Key==KEY_SHIFTNUMPAD0)
          {
            wchar_t *ClipText=PasteFromClipboardW();
            if(ClipText && *ClipText)
            {
              FastFindProcessName(&FindEdit,ClipText,strLastName,strName);
              FastFindShow(FindX,FindY);
              delete[] ClipText;
            }
            continue;
          }
          else if((Opt.XLat.XLatFastFindKey && Key == Opt.XLat.XLatFastFindKey ||
                   Opt.XLat.XLatAltFastFindKey && Key == Opt.XLat.XLatAltFastFindKey) ||
                  Key == MCODE_OP_XLAT)
          {
            string strTempName;
            FindEdit.Xlat();
            FindEdit.GetStringW(strTempName);
            FindEdit.SetStringW(L"");
            FastFindProcessName(&FindEdit,strTempName,strLastName,strName);
            FastFindShow(FindX,FindY);
            continue;
          }
          else if(Key == MCODE_OP_DATE || Key == MCODE_OP_PLAINTEXT)
          {
            string strTempName;
            FindEdit.ProcessKey(Key);
            FindEdit.GetStringW(strTempName);
            FindEdit.SetStringW(L"");
            FastFindProcessName(&FindEdit,strTempName,strLastName,strName);
            FastFindShow(FindX,FindY);
            continue;
          }
          else
            Key=_CorrectFastFindKbdLayout(&rec,Key);
        }
      }
      if (Key==KEY_ESC || Key==KEY_F10)
      {
        KeyToProcess=KEY_NONE;
        break;
      }

      // // _SVS(if (!FirstKey) SysLog("Panel::FastFind  Key=%s  %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(&rec)));
      if (Key>=KEY_ALT_BASE+0x01 && Key<=KEY_ALT_BASE+255)
        Key=LocalLowerW (Key-KEY_ALT_BASE);
      if (Key>=KEY_ALTSHIFT_BASE+0x01 && Key<=KEY_ALTSHIFT_BASE+255)
        Key=LocalLowerW (Key-KEY_ALTSHIFT_BASE);

      if (Key==KEY_MULTIPLY)
        Key=L'*';

      switch (Key)
      {
        case KEY_F1:
        {
          FindEdit.Hide();
          SaveScr.RestoreArea();
          {
            Help Hlp (L"FastFind");
          }
          FindEdit.Show();
          FastFindShow(FindX,FindY);
          break;
        }
        case KEY_CTRLENTER:
          FindPartName(strName,TRUE);
          FindEdit.Show();
          FastFindShow(FindX,FindY);
          break;
        case KEY_CTRLSHIFTENTER:
          FindPartName(strName,TRUE,-1);
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
              !(Key==KEY_CTRLINS||Key==KEY_CTRLNUMPAD0) && !(Key==KEY_SHIFTINS||Key==KEY_SHIFTNUMPAD0))
          {
            KeyToProcess=Key;
            break;
          }

          if (FindEdit.ProcessKey(Key))
          {
            FindEdit.GetStringW(strName);

            // уберем двойные '**'

            wchar_t *Name = strName.GetBuffer ();

            int LenName=wcslen(Name);
            if(LenName > 1 && Name[LenName-1] == L'*' && Name[LenName-2] == L'*')
            {
              Name[LenName-1]=0;
              FindEdit.SetStringW(Name);
            }
            /* $ 09.04.2001 SVS
               проблемы с быстрым поиском.
               Подробнее в 00573.ChangeDirCrash.txt
            */
            if(*Name == L'"')
            {
              memmove(Name,Name+1,(sizeof(Name)-1)*sizeof (wchar_t));
              Name[wcslen(Name)-1]=0;
              FindEdit.SetStringW(Name);
            }

            strName.ReleaseBuffer ();
            /* SVS $ */
            if (FindPartName(strName,FALSE))
              strLastName = strName;
            else
            {
              if(CtrlObject->Macro.IsExecuting())// && CtrlObject->Macro.GetLevelState() > 0) // если вставка макросом...
              {
                //CtrlObject->Macro.DropProcess(); // ... то дропнем макропроцесс
//                CtrlObject->Macro.PopState();
;
              }
              FindEdit.SetStringW(strLastName);
              strName = strLastName;
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
  TextW(L" ");
  GotoXY(FindX+20,FindY+1);
  TextW(L" ");
  Box(FindX,FindY,FindX+21,FindY+2,COL_DIALOGBOX,DOUBLE_BOX);
  GotoXY(FindX+7,FindY);
  SetColor(COL_DIALOGBOXTITLE);
  TextW(MSearchFileTitle);
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
    FarChDirW(strCurDir);
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
          SrcDragPanel->GoToFileW(strDragName);
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
    GetSelNameW(NULL,FileAttr);
    if (GetSelNameW(&strDragName,FileAttr) && !TestParentFolderNameW(strDragName))
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
  string strDragMsg, strSelName;
  int SelCount,MsgX,Length;

  if ((SelCount=SrcDragPanel->GetSelCount())==0)
    return;

  if (SelCount==1)
  {
    string strCvtName;
    int FileAttr;
    SrcDragPanel->GetSelNameW(NULL,FileAttr);
    SrcDragPanel->GetSelNameW(&strSelName,FileAttr);
    strCvtName = PointToNameW(strSelName);
    QuoteSpaceW(strCvtName);
    strSelName = strCvtName;
  }
  else
    strSelName.Format (UMSG(MDragFiles), SelCount);

  if (Move)
    strDragMsg.Format (UMSG(MDragMove), (const wchar_t*)strSelName);
  else
    strDragMsg.Format (UMSG(MDragCopy), (const wchar_t*)strSelName);

  if ((Length=strDragMsg.GetLength())+X>ScrX)
  {
    MsgX=ScrX-Length;
    if (MsgX<0)
    {
      MsgX=0;
      TruncStrFromEndW(strDragMsg,ScrX);

      Length=strDragMsg.GetLength ();
    }
  }
  else
    MsgX=X;
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  delete DragSaveScr;
  DragSaveScr=new SaveScreen(MsgX,Y,MsgX+Length-1,Y);
  GotoXY(MsgX,Y);
  SetColor(COL_PANELDRAGTEXT);
  TextW(strDragMsg);
}



int Panel::GetCurDirW(string &strCurDir)
{
  strCurDir = Panel::strCurDir; // TODO: ОПАСНО!!!
  return strCurDir.GetLength();
}



void Panel::SetCurDirW(const wchar_t *CurDir,int ClosePlugin)
{
  strCurDir = CurDir;
  PrepareDiskPathW (strCurDir);
}


void Panel::InitCurDirW(const wchar_t *CurDir)
{
  strCurDir = CurDir;
  PrepareDiskPathW (strCurDir);
}


/* $ 14.06.2001 KM
   + Добавлена установка переменных окружения, определяющих
     текущие директории дисков как для активной, так и для
     пассивной панели. Это необходимо программам запускаемым
     из FAR.
*/
/* $ 05.10.2001 SVS
   ! Давайте для начала выставим нужные значения для пассивной панели,
     а уж потом...
     А то фигня какая-то получается...
*/
/* $ 14.01.2002 IS
   ! Убрал установку переменных окружения, потому что она производится
     в FarChDir, которая теперь используется у нас для установления
     текущего каталога.
*/
int  Panel::SetCurPath()
{
  if (GetMode()==PLUGIN_PANEL)
    return TRUE;

  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  if (AnotherPanel->GetType()!=PLUGIN_PANEL)
  {
    if (LocalIsalphaW(AnotherPanel->strCurDir.At(0)) && AnotherPanel->strCurDir.At(1)==L':' &&
        LocalUpperW(AnotherPanel->strCurDir.At(0))!=LocalUpperW(strCurDir.At(0)))
    {
      // сначала установим переменные окружения для пассивной панели
      // (без реальной смены пути, чтобы лишний раз пассивный каталог
      // не перечитывать)
      FarChDirW(AnotherPanel->strCurDir,FALSE);
    }
  }

  if (!FarChDirW(strCurDir) || GetFileAttributesW(strCurDir)==0xFFFFFFFF)
  {
   // здесь на выбор :-)
#if 1
    while(!FarChDirW(strCurDir))
    {
      string strRoot;
      GetPathRootW (strCurDir, strRoot);
      if(FAR_GetDriveTypeW(strRoot) != DRIVE_REMOVABLE || IsDiskInDriveW(strRoot))
      {
        int Result=CheckFolderW(strCurDir);
        if(Result == CHKFLD_NOTACCESS)
        {
          if(FarChDirW(strRoot))
          {
            SetCurDirW(strRoot,TRUE);
            return TRUE;
          }
        }
        else if(Result == CHKFLD_NOTFOUND)
        {
          if(CheckShortcutFolderW(&strCurDir,FALSE,TRUE) && FarChDirW(strCurDir))
          {
            SetCurDirW(strCurDir,TRUE);
            return TRUE;
          }
        }
        else
          break;
      }
      if(FrameManager && FrameManager->ManagerStarted()) // сначала проверим - а запущен ли менеджер
        ChangeDisk();                                    // если запущен - вызовем меню выбора дисков
      else                                               // оппа...
      {
        string strTemp=strCurDir;
        CutToFolderNameIfFolderW(strCurDir);             // подымаемся вверх, для очередной порции ChDir
        if(strTemp.GetLength()==strCurDir.GetLength())   // здесь проблема - видимо диск недоступен
        {
          SetCurDirW(g_strFarPath,TRUE);                 // тогда просто сваливаем в каталог, откуда стартанул FAR.
          break;
        }
        else
        {
          if(FarChDirW(strCurDir))
          {
            SetCurDirW(strCurDir,TRUE);
            break;
          }
        }
      }
    }
#else
    do{
      BOOL IsChangeDisk=FALSE;
      char Root[1024];
      GetPathRoot(CurDir,Root);
      if(FAR_GetDriveType(Root) == DRIVE_REMOVABLE && !IsDiskInDrive(Root))
        IsChangeDisk=TRUE;
      else if(CheckFolder(CurDir) == CHKFLD_NOTACCESS)
      {
        if(FarChDir(Root))
          SetCurDir(Root,TRUE);
        else
          IsChangeDisk=TRUE;
      }
      if(IsChangeDisk)
        ChangeDisk();
    } while(!FarChDir(CurDir));
#endif
    return FALSE;
  }
  return TRUE;
}
/* IS $ */
/* SVS $ */
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
  if ( Locked () )
    return;

  /* $ 03.10.2001 IS перерисуем строчку меню */
  if (Opt.ShowMenuBar)
      CtrlObject->TopMenuBar->Show();
  /* IS $ */
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
      {
        SetVisible(TRUE);
        return;
      }
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
    ShowSeparator(X2-X1+1,1);
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
      string strScreensText;
      string strAdd;
      strScreensText.Format (L"[%d", Viewers);
      if (Editors > 0)
      {
        strAdd.Format (L"+%d", Editors);
        strScreensText += strAdd;
      }

      if (Dialogs > 0)
      {
        strAdd.Format (L",%d", Dialogs);
        strScreensText += strAdd;
      }
      strScreensText += L"]";
      GotoXY(Opt.ShowColumnTitles ? X1:X1+2,Y1);
      SetColor(COL_PANELSCREENSNUMBER);
      TextW(strScreensText);
    }
    /* DJ $ */
  }
}


void Panel::SetTitle()
{
  if (GetFocus())
  {
    string strTitleDir = L"{";

    if ( !strCurDir.IsEmpty() )
      strTitleDir += strCurDir;
    else
    {
      string strCmdText;
      CtrlObject->CmdLine->GetCurDirW(strCmdText);

      strTitleDir += strCmdText;
    }

    strTitleDir += L"}";

    strLastFarTitle = strTitleDir; //BUGBUG
    SetFarTitleW(strTitleDir);
  }
}

void Panel::GetTitle(string &strTitle,int SubLen,int TruncSize)
{
  string strTitleDir;

  if (PanelMode==PLUGIN_PANEL)
  {
    struct OpenPluginInfoW Info;
    GetOpenPluginInfo(&Info);
    strTitleDir = NullToEmptyW(Info.PanelTitle);
    RemoveExternalSpacesW(strTitleDir);
    TruncStrW(strTitleDir,SubLen-TruncSize);
  }
  else
  {
    if (ShowShortNames)
      ConvertNameToShortW(strCurDir,strTitleDir);
    else
      strTitleDir = strCurDir;
    TruncPathStrW(strTitleDir,SubLen-TruncSize);
  }

  strTitle = L" "+strTitleDir+L" ";
}

int Panel::SetPluginCommand(int Command,void *Param)
{
  _ALGO(CleverSysLog clv("Panel::SetPluginCommand"));
  _ALGO(SysLog("(Command=%s, Param=[%d/0x%08X])",_FCTL_ToName(Command),(int)Param,Param));
  int Result=FALSE;
  ProcessingPluginCommand++;
  FilePanels *FPanels=CtrlObject->Cp();
  Panel *AnotherPanel=FPanels->GetAnotherPanel(this);
  PluginCommand=Command;

  switch(Command)
  {
    case FCTL_SETVIEWMODE:
      Result=FPanels->ChangePanelViewMode(this, (Param?*(int *)Param:0), FPanels->IsTopFrame());
      break;

    case FCTL_SETSORTMODE:
      if (Param!=NULL)
      {
        int Mode=*(int *)Param;
        if ((Mode>SM_DEFAULT) && (Mode<=SM_NUMLINKS))
        {
          SetSortMode(--Mode); // Уменьшим на 1 из-за SM_DEFAULT
          Result=TRUE;
        }
      }
      break;

    case FCTL_SETNUMERICSORT:
      {
        int NumericSortOrder = (Param && (*(int *)Param)) ? 1:0;

        SetNumericSort(NumericSortOrder);
        Result=TRUE;
      }
      break;

    case FCTL_SETSORTORDER:
      {
        /* $ 22.01.2001 VVM
           - Порядок сортировки задается аналогично StartSortOrder */
        int Order = (Param && (*(int *)Param)) ? -1:1;

        ChangeSortOrder(Order);
        Result=TRUE;
      }
      break;

    case FCTL_CLOSEPLUGIN:
      xstrncpy((char *)PluginParam,NullToEmpty((char *)Param),sizeof(PluginParam)-1);
      Result=TRUE;
      //if(Opt.CPAJHefuayor)
      //  CtrlObject->Plugins.ProcessCommandLine((char *)PluginParam);
      break;

    case FCTL_FREEPANELINFO:
    {
        if (Param == NULL || IsBadWritePtr(Param,sizeof(struct PanelInfo)))
            break;

        PanelInfo *Info=(PanelInfo *)Param;

        xf_free (Info->lpwszCurDir);
        xf_free (Info->lpwszColumnTypes);
        xf_free (Info->lpwszColumnWidths);

        for (int i = 0; i < Info->ItemsNumber; i++)
            apiFreeFindData (&Info->PanelItems[i].FindData);

        for (int i = 0; i < Info->SelectedItemsNumber; i++)
            apiFreeFindData (&Info->SelectedItems[i].FindData);

        break;
    }

    case FCTL_GETPANELINFO:
    case FCTL_GETPANELSHORTINFO:
    {
      if(Param == NULL || IsBadWritePtr(Param,sizeof(struct PanelInfo)))
        break;
      struct PanelInfo *Info=(struct PanelInfo *)Param;
      memset(Info,0,sizeof(*Info));

      /* $ 19.03.2002 DJ
         обеспечим наличие данных
      */
      UpdateIfRequired();

      switch( GetType() )
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
      Info->Plugin=(GetMode()==PLUGIN_PANEL);
      int X1,Y1,X2,Y2;
      GetPosition(X1,Y1,X2,Y2);
      Info->PanelRect.left=X1;
      Info->PanelRect.top=Y1;
      Info->PanelRect.right=X2;
      Info->PanelRect.bottom=Y2;
      Info->Visible=IsVisible();
      Info->Focus=GetFocus();
      Info->ViewMode=GetViewMode();
      Info->SortMode=SM_UNSORTED-UNSORTED+GetSortMode();

      string strInfoCurDir;

      GetCurDirW(strInfoCurDir);

      Info->lpwszCurDir = _wcsdup(strInfoCurDir);
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
        };

        DWORD Flags=0;

        for(int I=0; I < sizeof(PFLAGS)/sizeof(PFLAGS[0]); ++I)
          if(*(PFLAGS[I].Opt) != 0)
            Flags|=PFLAGS[I].Flags;

        Flags|=GetSortOrder()<0?PFLAGS_REVERSESORTORDER:0;
        Flags|=GetSortGroups()?PFLAGS_USESORTGROUPS:0;
        Flags|=GetSelectedFirstMode()?PFLAGS_SELECTEDFIRST:0;
        Flags|=GetNumericSort()?PFLAGS_NUMERICSORT:0;


        Info->Flags=Flags;
      }

      if (GetType()==FILE_PANEL)
      {
        FileList *DestFilePanel=(FileList *)this;
        static int Reenter=0;
        if (!Reenter && Info->Plugin)
        {
          Reenter++;
          struct OpenPluginInfoW PInfo;
          DestFilePanel->GetOpenPluginInfo(&PInfo);

          Info->lpwszCurDir = _wcsdup(PInfo.CurDir); //BUGBUG, memleak, see prev _wcsdup
          /* $ 12.12.2001 DJ
             обработаем флаги
          */
          if (PInfo.Flags & OPIF_REALNAMES)
            Info->Flags |= PFLAGS_REALNAMES;
          if (!(PInfo.Flags & OPIF_USEHIGHLIGHTING))
            Info->Flags &= ~PFLAGS_HIGHLIGHT;
          /* DJ $ */
          Reenter--;
        }
        DestFilePanel->PluginGetPanelInfo(Info,(Command == FCTL_GETPANELINFO)?TRUE:FALSE);
      }

      if (!Info->Plugin) // $ 12.12.2001 DJ - на неплагиновой панели - всегда реальные имена
          Info->Flags |= PFLAGS_REALNAMES;
      Result=TRUE;
      break;
    }

    case FCTL_SETSELECTION:
      {
        if (GetType()==FILE_PANEL && !IsBadReadPtr(Param,sizeof(PanelInfo)))
        {
          ((FileList *)this)->PluginSetSelection((PanelInfo *)Param);
          Result=TRUE;
        }
        break;
      }

    case FCTL_UPDATEPANEL:
      Update(Param==NULL ? 0:UPDATE_KEEP_SELECTION);

      if ( GetType() == QVIEW_PANEL )
      	UpdateViewPanel();

      Result=TRUE;
      break;

    case FCTL_REDRAWPANEL:
    {
      PanelRedrawInfo *Info=(PanelRedrawInfo *)Param;
      if (Info && !IsBadReadPtr(Info,sizeof(struct PanelRedrawInfo)))
      {
        CurFile=Info->CurrentItem;
        CurTopFile=Info->TopPanelItem;
      }
        // $ 12.05.2001 DJ перерисовываемся только в том случае, если мы - текущий фрейм
      if (FPanels->IsTopFrame())
      {
        Redraw();
        Result=TRUE;
      }
      break;
    }

    case FCTL_SETPANELDIR:
    {
      if (Param)
      {
        SetCurDirW((const wchar_t *)Param,TRUE);
        Result=TRUE;
      }
      break;
    }

  }
  ProcessingPluginCommand--;
  return Result;
}


/*int Panel::GetCurName(char *Name,char *ShortName)
{
  *Name=*ShortName=0;
  return(FALSE);
}*/


int Panel::GetCurNameW(string &strName, string &strShortName)
{
  strName = L"";
  strShortName = L"";
  return(FALSE);
}

/*
int Panel::GetCurBaseName(char *Name,char *ShortName)
{
  *Name=*ShortName=0;
  return(FALSE);
}
*/

int Panel::GetCurBaseNameW(string &strName, string &strShortName)
{
  strName = L"";
  strShortName = L"";
  return(FALSE);
}

static int MessageRemoveConnection(wchar_t Letter, int &UpdateProfile)
{
  int Len1, Len2, Len3,Len4;
  BOOL IsPersistent;
  string strMsgText;
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
  static struct DialogDataEx DCDlgData[]=
  {
/*      Type          X1 Y1 X2  Y2 Focus Flags             DefaultButton
                                      Selected               Data
*/
/* 0 */ DI_DOUBLEBOX, 3, 1, 72, 9, 0, 0, 0,                0,L"",
/* 1 */ DI_TEXT,      5, 2,  0, 0, 0, 0, DIF_SHOWAMPERSAND,0,L"",
/* 2 */ DI_TEXT,      5, 3,  0, 0, 0, 0, DIF_SHOWAMPERSAND,0,L"",
/* 3 */ DI_TEXT,      5, 4,  0, 0, 0, 0, DIF_SHOWAMPERSAND,0,L"",
/* 4 */ DI_TEXT,      0, 5,  0, 6, 0, 0, DIF_SEPARATOR,    0,L"",
/* 5 */ DI_CHECKBOX,  5, 6, 70, 5, 0, 0, 0,                0,L"",
/* 6 */ DI_TEXT,      0, 7,  0, 6, 0, 0, DIF_SEPARATOR,    0,L"",
/* 7 */ DI_BUTTON,    0, 8,  0, 0, 1, 0, DIF_CENTERGROUP,  1,L"",
/* 8 */ DI_BUTTON,    0, 8,  0, 0, 0, 0, DIF_CENTERGROUP,  0,L""
  };
  MakeDialogItemsEx(DCDlgData,DCDlg);


  DCDlg[0].strData = UMSG(MChangeDriveDisconnectTitle);
  Len1 = DCDlg[0].strData.GetLength();

  strMsgText.Format (UMSG(MChangeDriveDisconnectQuestion),Letter);
  DCDlg[1].strData = strMsgText;
  Len2=DCDlg[1].strData.GetLength ();

  strMsgText.Format (UMSG(MChangeDriveDisconnectMapped),Letter);
  DCDlg[2].strData = strMsgText;
  Len4 = DCDlg[2].strData.GetLength();

  DCDlg[5].strData = UMSG(MChangeDriveDisconnectReconnect);
  Len3= DCDlg[5].strData.GetLength ();


  Len1=Max(Len1,Max(Len2,Max(Len3,Len4)));

  DCDlg[3].strData = TruncPathStrW(DriveLocalToRemoteNameW(DRIVE_REMOTE,Letter,strMsgText),Len1);

  DCDlg[7].strData = UMSG(MYes);
  DCDlg[8].strData = UMSG(MCancel);

  // проверяем - это было постоянное соедение или нет?
  // Если ветка в реестре HKCU\Network\БукваДиска есть - это
  //   есть постоянное подключение.
  {
    HKEY hKey;
    IsPersistent=TRUE;
    if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
      strMsgText.Format (L"Network\\%c",LocalUpperW(Letter));
    else
      strMsgText.Format (L"Network\\Persistent\\%c", LocalUpperW(Letter));

    if(RegOpenKeyExW(HKEY_CURRENT_USER,strMsgText,0,KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
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
    Dlg.SetHelp(L"DisconnectDrive");
    Dlg.Process();
    ExitCode=Dlg.GetExitCode();
  }
  UpdateProfile=DCDlg[5].Selected?0:CONNECT_UPDATE_PROFILE;
  if(IsPersistent)
    Opt.ChangeDriveDisconnetMode=DCDlg[5].Selected;
  return ExitCode == 7;
}

BOOL Panel::NeedUpdatePanel(Panel *AnotherPanel)
{
  /* Обновить, если обновление разрешено и пути совпадают */
  if ((!Opt.AutoUpdateLimit || static_cast<DWORD>(GetFileCount()) <= Opt.AutoUpdateLimit) &&
      LocalStricmpW(AnotherPanel->strCurDir,strCurDir)==0)
    return TRUE;
  return FALSE;
}

int Panel::ProcessShortcutFolder(int Key,BOOL ProcTreePanel)
{
  string strShortcutFolder, strPluginModule, strPluginFile, strPluginData;

    if (GetShortcutFolder(Key,&strShortcutFolder,&strPluginModule,&strPluginFile,&strPluginData))
    {
      Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
      if(ProcTreePanel)
      {
        if (AnotherPanel->GetType()==FILE_PANEL)
        {
          AnotherPanel->SetCurDirW(strShortcutFolder,TRUE);
          AnotherPanel->Redraw();
        }
        else
        {
          SetCurDirW(strShortcutFolder,TRUE);
          ProcessKey(KEY_ENTER);
        }
      }
      else
      {
        if (AnotherPanel->GetType()==FILE_PANEL && !strPluginModule.IsEmpty() )
        {
          AnotherPanel->SetCurDirW(strShortcutFolder,TRUE);
          AnotherPanel->Redraw();
        }
      }
      return(TRUE);
    }

  return FALSE;
}
