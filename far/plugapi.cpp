/*
plugapi.cpp

API, ��������� �������� (�������, ����, ...)

*/

/* Revision: 1.212 01.09.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "plugin.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "struct.hpp"
#include "keys.hpp"
#include "lang.hpp"
#include "help.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "scantree.hpp"
#include "rdrwdsk.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "plugins.hpp"
#include "savescr.hpp"
#include "flink.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "frame.hpp"
#include "scrbuf.hpp"
#include "farexcpt.hpp"
#include "lockscrn.hpp"
#include "constitle.hpp"


void ScanPluginDir();


/* $ 07.12.2001 IS
   ������� ������ GetString ��� �������� - � ������� �����������������.
   ������� ��� ����, ����� �� ����������� ��� GetString.
*/
int WINAPI FarInputBox (
        const wchar_t *Title,
        const wchar_t *Prompt,
        const wchar_t *HistoryName,
        const wchar_t *SrcText,
        wchar_t *DestText,
        int DestLength,
        const wchar_t *HelpTopic,
        DWORD Flags
        )
{
  if (FrameManager->ManagerIsDown())
    return FALSE;

  string strDest;

  int nResult = GetStringW(Title,Prompt,HistoryName,SrcText,strDest,DestLength,
     HelpTopic,Flags&~FIB_CHECKBOX,NULL,NULL);

  xwcsncpy (DestText, strDest, DestLength);

  return nResult;
}

/* ������� ������ ������ */
BOOL WINAPI FarShowHelp(const wchar_t *ModuleName,
                        const wchar_t *HelpTopic,
                        DWORD Flags)
{
  if (FrameManager->ManagerIsDown())
    return FALSE;
  if (!HelpTopic)
    HelpTopic=L"Contents";

  DWORD OFlags=Flags;
  Flags&=~(FHELP_NOSHOWERROR|FHELP_USECONTENTS);
  string strPath, strTopic;
  string strMask;

  // ��������� � ������ ������ ���� �� ������������ � � ��� ������,
  // ���� ����� FHELP_FARHELP...
  if((Flags&FHELP_FARHELP) || *HelpTopic==L':')
    strTopic = HelpTopic+((*HelpTopic == L':')?1:0);
  else
  {
    if(ModuleName)
    {
      // FHELP_SELFHELP=0 - ���������� ������ ���-� ��� Info.ModuleName
      //                   � �������� ����� �� ����� ���������� �������
      /* $ 17.11.2000 SVS
         � �������� FHELP_SELFHELP ����� ����? ��������� - 0
         � ����� ����� ��������� ����, ��� ������� �� �������� :-(
      */
      if(Flags == FHELP_SELFHELP || (Flags&(FHELP_CUSTOMFILE|FHELP_CUSTOMPATH)))
      {
        strPath = ModuleName;
        if(Flags == FHELP_SELFHELP || (Flags&(FHELP_CUSTOMFILE)))
        {
          strMask=PointToNameW(strPath);
          if(Flags&FHELP_CUSTOMFILE)
          {
              strPath = PointToNameW(strPath);
          }
          else
          {
              CutToSlashW(strPath);
              strMask = L"";
          }
        }
      }
      else
        return FALSE;
      /* SVS $*/

      strTopic.Format (HelpFormatLink,(const wchar_t*)strPath,HelpTopic);
    }
    else
      return FALSE;
  }
  {
    Help Hlp (strTopic,strMask,OFlags);
    if(Hlp.GetError())
      return FALSE;
  }
  return TRUE;
}
/* IS $ */
/* tran 18.08.2000 $ */
/* SVS 12.09.2000 $ */

/* $ 05.07.2000 IS
  �������, ������� ����� ����������� � � ���������, � � �������, �...
*/
#ifndef _MSC_VER
#pragma warn -par
#endif
int WINAPI FarAdvControl(int ModuleNumber, int Command, void *Param)
{
  struct Opt2Flags{
    int *Opt;
    DWORD Flags;
  };
  int I;

  switch(Command)
  {
    case ACTL_GETFARVERSION:
    case ACTL_GETSYSWORDDIV:
    case ACTL_GETCOLOR:
    case ACTL_GETARRAYCOLOR:
    case ACTL_GETFARHWND:
    case ACTL_GETSYSTEMSETTINGS:
    case ACTL_GETPANELSETTINGS:
    case ACTL_GETINTERFACESETTINGS:
    case ACTL_GETCONFIRMATIONS:
    case ACTL_GETDESCSETTINGS:
    case ACTL_GETPOLICIES:
    case ACTL_GETPLUGINMAXREADDATA:
    case ACTL_GETMEDIATYPE:
      break;
    default:
     if (FrameManager && FrameManager->ManagerIsDown())
       return 0;
  }

  switch(Command)
  {
    case ACTL_GETFARVERSION:
    {
      if(Param)
        *(DWORD*)Param=FAR_VERSION;
      return FAR_VERSION;
    }

    /* $ 25.07.2000 SVS
       + ���������� ������������ FulScreen <-> Windowed (ACTL_CONSOLEMODE)
       mode = -2 - �������� ������� ���������
              -1 - ��� ������
               0 - Windowed
               1 - FulScreen
       Return
               0 - Windowed
               1 - FulScreen
    */
    case ACTL_CONSOLEMODE:
    {
      return FarAltEnter((int)Param);
    }
    /* SVS $ */

    case ACTL_GETPLUGINMAXREADDATA:
    {
      return Opt.PluginMaxReadData;
    }

    case ACTL_GETWCHARMODE:
    {
      return Opt.UseUnicodeConsole;
    }

    /* $ 03.08.2000 SVS
       ��������� ������ � ������������� ����
       ���������� ������ ���������� ������ ��� '\0'
       ������������ ������ ��������� ������ = 80 � �������������� '\0'
       ������ ���������� �� �� �������, � �� Opt.
    */
    case ACTL_GETSYSWORDDIV:
    {
      int LenWordDiv=Opt.strWordDiv.GetLength ();
      /* $ 09.08.2000 tran
       + if param==NULL, plugin ����� ������ ������ ����� ������  */
      if (Param && !IsBadWritePtr(Param,LenWordDiv+1))
        wcscpy((wchar_t *)Param,Opt.strWordDiv);
      /* tran 09.08.2000 $ */
      return LenWordDiv;
    }
    /* SVS $ */

    /* $ 24.08.2000 SVS
       ������� ������������ (��� �����) �������
       (int)Param - ���������� ��� �������, ������� �������, ��� -1
       ���� ��� ����� ����� ������� �����.
       ���������� 0;
    */
    case ACTL_WAITKEY:
    {
      return WaitKey(Param?(DWORD)Param:(DWORD)-1);
    }
    /* SVS $ */

    /* $ 04.12.2000 SVS
      ACTL_GETCOLOR - �������� ������������ ���� �� ������, �������������
       � farcolor.hpp
      (int)Param - ������.
      Return - �������� ����� ��� -1 ���� ������ �������.
    */
    case ACTL_GETCOLOR:
    {
      if((int)Param < SizeArrayPalette && (int)Param >= 0)
        return (int)((unsigned int)Palette[(int)Param]);
      return -1;
    }
    /* SVS $ */

    /* $ 04.12.2000 SVS
      ACTL_GETARRAYCOLOR - �������� ���� ������ ������
      Param - ��������� �� ������ ��� NULL - ����� �������� ������ ������
      Return - ������ �������.
    */
    case ACTL_GETARRAYCOLOR:
    {
      if(Param && !IsBadWritePtr(Param,SizeArrayPalette))
        memmove(Param,Palette,SizeArrayPalette);
      return SizeArrayPalette;
    }
    /* SVS $ */

    /*
      Param=struct FARColor{
        DWORD Flags;
        int StartIndex;
        int ColorItem;
        LPBYTE Colors;
      };
    */
    case ACTL_SETARRAYCOLOR:
    {
      if(Param && !IsBadReadPtr(Param,sizeof(struct FarSetColors)))
      {
        struct FarSetColors *Pal=(struct FarSetColors*)Param;
        if(Pal->Colors &&
           Pal->StartIndex >= 0 &&
           Pal->StartIndex+Pal->ColorCount <= SizeArrayPalette &&
           !IsBadReadPtr(Pal->Colors,Pal->ColorCount)
          )
        {
          memmove(Palette+Pal->StartIndex,Pal->Colors,Pal->ColorCount);
          if(Pal->Flags&FCLR_REDRAW)
          {
            ScrBuf.Lock(); // �������� ������ ����������
            FrameManager->ResizeAllFrame();
            FrameManager->PluginCommit(); // ��������.
            ScrBuf.Unlock(); // ��������� ����������
          }
          return TRUE;
        }
      }
      return FALSE;
    }

    /* $ 14.12.2000 SVS
      ACTL_EJECTMEDIA - ������� ���� �� �������� ����������
      Param - ��������� �� ��������� ActlEjectMedia
      Return - TRUE - �������� ����������, FALSE - ������.
    */
    case ACTL_EJECTMEDIA:
    {
      return Param?EjectVolume((char)((ActlEjectMedia*)Param)->Letter,
                               ((ActlEjectMedia*)Param)->Flags):FALSE;
/*
      if(Param)
      {
        struct ActlEjectMedia *aem=(struct ActlEjectMedia *)Param;
        char DiskLetter[4]=" :\\";
        DiskLetter[0]=(char)aem->Letter;
        int DriveType = FAR_GetDriveType(DiskLetter,NULL,FALSE); // ����� �� ���������� ��� CD

        if(DriveType == DRIVE_USBDRIVE && RemoveUSBDrive((char)aem->Letter,aem->Flags))
          return TRUE;
        if(DriveType == DRIVE_SUBSTITUTE && DelSubstDrive(DiskLetter))
          return TRUE;
        if(IsDriveTypeCDROM(DriveType) && EjectVolume((char)aem->Letter,aem->Flags))
          return TRUE;

      }
      return FALSE;
*/
    }
/*
    case ACTL_GETMEDIATYPE:
    {
      struct ActlMediaType *amt=(struct ActlMediaType *)Param;
      char DiskLetter[4]=" :\\";
      DiskLetter[0]=(amt)?(char)amt->Letter:0;
      return FAR_GetDriveType(DiskLetter,NULL,(amt && !(amt->Flags&MEDIATYPE_NODETECTCDROM)?TRUE:FALSE));
    }
*/
    /* $ 21.12.2000 SVS
       Macro API
    */
    case ACTL_KEYMACRO:
    {
      if(CtrlObject && Param) // ��� ������� �� ���� ������.
      {
        KeyMacro& Macro=CtrlObject->Macro; //??
        struct ActlKeyMacro *KeyMacro=(struct ActlKeyMacro*)Param;
        switch(KeyMacro->Command)
        {
          case MCMD_LOADALL: // �� ������� � ������ ��� � ���������� �����������
          {
            if(Macro.IsRecording())
              return FALSE;
            return Macro.LoadMacros(!Macro.IsExecuting());
          }

          case MCMD_SAVEALL: // �� ������ ���� � �������
          {
            if(Macro.IsRecording()) // || Macro.IsExecuting())
              return FALSE;
            Macro.SaveMacros();
            return TRUE;
          }

          case MCMD_POSTMACROSTRING:
          {
            return Macro.PostNewMacro(KeyMacro->Param.PlainText.SequenceText,KeyMacro->Param.PlainText.Flags<<8);
          }

          case MCMD_CHECKMACRO:  // �������� �������
          {
            struct MacroRecord CurMacro={0};
            int Ret=Macro.ParseMacroString(&CurMacro,KeyMacro->Param.PlainText.SequenceText);
            if(Ret)
            {
              if(CurMacro.BufferSize > 1)
                xf_free(CurMacro.Buffer);
            }
            else
            {
              static string ErrMsg[3];
              GetMacroParseError(&ErrMsg[0],&ErrMsg[1],&ErrMsg[2]);
              KeyMacro->Param.MacroResult.ErrMsg1=ErrMsg[0];
              KeyMacro->Param.MacroResult.ErrMsg2=ErrMsg[1];
              KeyMacro->Param.MacroResult.ErrMsg3=ErrMsg[2];
            }
            return Ret;
          }

#if 0
          case MCMD_COMPILEMACRO:
          {
            struct MacroRecord CurMacro={0};
            int Ret=Macro.ParseMacroString(&CurMacro,KeyMacro->Param.PlainText.SequenceText);
            if(Ret)
            {
              //KeyMacro->Params.Compile.Flags=CurMacro.Flags;
              KeyMacro->Param.Compile.Sequence=CurMacro.Buffer;
              KeyMacro->Param.Compile.Count=CurMacro.BufferSize;
            }
            return Ret;
          }
#endif
        }
      }
      return FALSE;
    }

    case ACTL_POSTKEYSEQUENCE:
    {
      if(CtrlObject && Param && ((struct KeySequence*)Param)->Count > 0)
      {
        struct MacroRecord MRec;
        memset(&MRec,0,sizeof(struct MacroRecord));
        MRec.Flags=(((struct KeySequence*)Param)->Flags)<<8;
        MRec.BufferSize=((struct KeySequence*)Param)->Count;
        if(MRec.BufferSize == 1)
          MRec.Buffer=(DWORD *)((struct KeySequence*)Param)->Sequence[0];
        else
          MRec.Buffer=((struct KeySequence*)Param)->Sequence;
        return CtrlObject->Macro.PostNewMacro(&MRec,TRUE);
#if 0
        // ���� ����� - ��� ���������� �������������
        {
          //CtrlObject->Macro.PostNewMacro(&MRec);
          for(int I=0; I < MRec.BufferSize; ++I)
          {
            int Key=MRec.Buffer[I];
            if(CtrlObject->Macro.ProcessKey(Key))
            {
              while((Key=CtrlObject->Macro.GetKey()) != 0)
              {
                FrameManager->ProcessKey(Key);
              }
            }
            else
              FrameManager->ProcessKey(Key);
            FrameManager->PluginCommit();
          }
          return TRUE;
        }
#endif
      }
      return FALSE;
    }

    /* $ 05.06.2001 tran
       ����� ACTL_ ��� ������ � �������� */
    case ACTL_GETWINDOWINFO:
    /* $ 12.04.2005 AY
         thread safe window info */
    case ACTL_GETSHORTWINDOWINFO:
    {
      if(FrameManager && Param && !IsBadWritePtr(Param,sizeof(WindowInfo)))
      {
        string strType, strName;
        WindowInfo *wi=(WindowInfo*)Param;
        Frame *f;
        /* $ 22.12.2001 VVM
          + ���� Pos == -1 �� ����� ������� ����� */
        if (wi->Pos == -1)
          f=FrameManager->GetCurrentFrame();
        else
          f=FrameManager->operator[](wi->Pos);
        /* VVM $ */
        if ( f==NULL )
          return FALSE;
        if (Command==ACTL_GETWINDOWINFO)
        {
          f->GetTypeAndName(strType, strName);

          UnicodeToAnsi (strType, wi->TypeName, sizeof (wi->TypeName)-1); //BUGBUG
          UnicodeToAnsi (strName, wi->Name, sizeof (wi->Name)-1); //BUGBUG
        }
        else
        {
          wi->TypeName[0]=0;
          wi->Name[0]=0;
        }
        wi->Pos=FrameManager->IndexOf(f);
        wi->Type=f->GetType();
        wi->Modified=f->IsFileModified();
        wi->Current=f==FrameManager->GetCurrentFrame();
        return TRUE;
      }
      return FALSE;
    }

    case ACTL_GETWINDOWCOUNT:
    {
      return FrameManager?FrameManager->GetFrameCount():0;
    }

    case ACTL_SETCURRENTWINDOW:
    {
      /* $ 15.05.2002 SKV
        �������� ������������ �������,
        ���� ��������� � ��������� ���������/������.
      */
      if (FrameManager && !FrameManager->InModalEV() &&
          FrameManager->operator[]((int)Param)!=NULL )
      {
        FrameManager->ActivateFrame((int)Param);
        return TRUE;
      }
      return FALSE;
    }
    /* tran 05.06.2001 $ */
    /*$ 26.06.2001 SKV
      ��� ����������� ������ � ACTL_SETCURRENTWINDOW
      (� ����� ��� ��� ���� � �������)
    */
    case ACTL_COMMIT:
    {
      return FrameManager?FrameManager->PluginCommit():FALSE;
    }
    /* SKV$*/
    /* $ 15.09.2001 tran
       ���������� �������� */
    case ACTL_GETFARHWND:
    {
      if(!hFarWnd)
        InitDetectWindowedMode();
      return (int)hFarWnd;
    }
    /* tran $ */

    case ACTL_GETDIALOGSETTINGS:
    {
      DWORD Options=0;
      static struct Opt2Flags ODlg[]={
        {&Opt.Dialogs.EditHistory,FDIS_HISTORYINDIALOGEDITCONTROLS},
        {&Opt.Dialogs.EditBlock,FDIS_PERSISTENTBLOCKSINEDITCONTROLS},
        {&Opt.Dialogs.AutoComplete,FDIS_AUTOCOMPLETEININPUTLINES},
        {&Opt.Dialogs.EULBsClear,FDIS_BSDELETEUNCHANGEDTEXT},
      };
      for(I=0; I < sizeof(ODlg)/sizeof(ODlg[0]); ++I)
        if(*ODlg[I].Opt)
          Options|=ODlg[I].Flags;
      return Options;
    }
    /* $ 24.11.2001 IS
       ��������� � ����������� ����������, ������, ����������, �������������
    */
    case ACTL_GETSYSTEMSETTINGS:
    {
      DWORD Options=0;
      static struct Opt2Flags OSys[]={
        {&Opt.ClearReadOnly,FSS_CLEARROATTRIBUTE},
        {&Opt.DeleteToRecycleBin,FSS_DELETETORECYCLEBIN},
        {&Opt.CMOpt.UseSystemCopy,FSS_USESYSTEMCOPYROUTINE},
        {&Opt.CMOpt.CopyOpened,FSS_COPYFILESOPENEDFORWRITING},
        {&Opt.ScanJunction,FSS_SCANSYMLINK},
        {&Opt.CreateUppercaseFolders,FSS_CREATEFOLDERSINUPPERCASE},
        {&Opt.SaveHistory,FSS_SAVECOMMANDSHISTORY},
        {&Opt.SaveFoldersHistory,FSS_SAVEFOLDERSHISTORY},
        {&Opt.SaveViewHistory,FSS_SAVEVIEWANDEDITHISTORY},
        {&Opt.UseRegisteredTypes,FSS_USEWINDOWSREGISTEREDTYPES},
        {&Opt.AutoSaveSetup,FSS_AUTOSAVESETUP},
      };
      for(I=0; I < sizeof(OSys)/sizeof(OSys[0]); ++I)
        if(*OSys[I].Opt)
          Options|=OSys[I].Flags;
      return Options;
    }

    case ACTL_GETPANELSETTINGS:
    {
      DWORD Options=0;
      static struct Opt2Flags OSys[]={
        {&Opt.ShowHidden,FPS_SHOWHIDDENANDSYSTEMFILES},
        {&Opt.Highlight,FPS_HIGHLIGHTFILES},
        {&Opt.Tree.AutoChangeFolder,FPS_AUTOCHANGEFOLDER},
        {&Opt.SelectFolders,FPS_SELECTFOLDERS},
        {&Opt.ReverseSort,FPS_ALLOWREVERSESORTMODES},
        {&Opt.ShowColumnTitles,FPS_SHOWCOLUMNTITLES},
        {&Opt.ShowPanelStatus,FPS_SHOWSTATUSLINE},
        {&Opt.ShowPanelTotals,FPS_SHOWFILESTOTALINFORMATION},
        {&Opt.ShowPanelFree,FPS_SHOWFREESIZE},
        {&Opt.ShowPanelScrollbar,FPS_SHOWSCROLLBAR},
        {&Opt.ShowScreensNumber,FPS_SHOWBACKGROUNDSCREENSNUMBER},
        {&Opt.ShowSortMode,FPS_SHOWSORTMODELETTER},
      };
      for(I=0; I < sizeof(OSys)/sizeof(OSys[0]); ++I)
        if(*OSys[I].Opt)
          Options|=OSys[I].Flags;
      return Options;
    }

    case ACTL_GETINTERFACESETTINGS:
    {
      DWORD Options=0;
      static struct Opt2Flags OSys[]={
        {&Opt.Clock,FIS_CLOCKINPANELS},
        {&Opt.ViewerEditorClock,FIS_CLOCKINVIEWERANDEDITOR},
        {&Opt.Mouse,FIS_MOUSE},
        {&Opt.ShowKeyBar,FIS_SHOWKEYBAR},
        {&Opt.ShowMenuBar,FIS_ALWAYSSHOWMENUBAR},
        {&Opt.AltGr,FIS_USERIGHTALTASALTGR},
        {&Opt.CMOpt.CopyShowTotal,FIS_SHOWTOTALCOPYPROGRESSINDICATOR},
        {&Opt.CMOpt.CopyTimeRule,FIS_SHOWCOPYINGTIMEINFO},
        {&Opt.PgUpChangeDisk,FIS_USECTRLPGUPTOCHANGEDRIVE},
      };
      for(I=0; I < sizeof(OSys)/sizeof(OSys[0]); ++I)
        if(*OSys[I].Opt)
          Options|=OSys[I].Flags;
      return Options;
    }

    case ACTL_GETCONFIRMATIONS:
    {
      DWORD Options=0;
      static struct Opt2Flags OSys[]={
        {&Opt.Confirm.Copy,FCS_COPYOVERWRITE},
        {&Opt.Confirm.Move,FCS_MOVEOVERWRITE},
        {&Opt.Confirm.Drag,FCS_DRAGANDDROP},
        {&Opt.Confirm.Delete,FCS_DELETE},
        {&Opt.Confirm.DeleteFolder,FCS_DELETENONEMPTYFOLDERS},
        {&Opt.Confirm.Esc,FCS_INTERRUPTOPERATION},
        {&Opt.Confirm.RemoveConnection,FCS_DISCONNECTNETWORKDRIVE},
        {&Opt.Confirm.AllowReedit,FCS_RELOADEDITEDFILE},
        {&Opt.Confirm.HistoryClear,FCS_CLEARHISTORYLIST},
        {&Opt.Confirm.Exit,FCS_EXIT},
      };
      for(I=0; I < sizeof(OSys)/sizeof(OSys[0]); ++I)
        if(*OSys[I].Opt)
          Options|=OSys[I].Flags;
      return Options;
    }
    /* IS $ */

    /* $ 30.01.2002 DJ
       ACTL_GETDESCSETTINGS
    */
    case ACTL_GETDESCSETTINGS:
    {
      // ����� ���� - � �������� �� ��������������
      DWORD Options=0;
      if (Opt.Diz.UpdateMode == DIZ_UPDATE_IF_DISPLAYED)
        Options |= FDS_UPDATEIFDISPLAYED;
      else if (Opt.Diz.UpdateMode == DIZ_UPDATE_ALWAYS)
        Options |= FDS_UPDATEALWAYS;
      if (Opt.Diz.SetHidden)
        Options |= FDS_SETHIDDEN;
      if (Opt.Diz.ROUpdate)
        Options |= FDS_UPDATEREADONLY;
      return Options;
    }
    /* DJ $ */

    case ACTL_GETPOLICIES:
    {
      return Opt.Policies.DisabledOptions|(Opt.Policies.ShowHiddenDrives?FFPOL_SHOWHIDDENDRIVES:0);
    }

  }
  return FALSE;
}
#ifndef _MSC_VER
#pragma warn +par
#endif
/* IS $ */

int WINAPI FarMenuFn(int PluginNumber,int X,int Y,int MaxHeight,
           DWORD Flags,const wchar_t *Title,const wchar_t *Bottom,
           const wchar_t *HelpTopic, const int *BreakKeys,int *BreakCode,
           const struct FarMenuItem *Item, int ItemsNumber)
{
  int I;

  if (FrameManager->ManagerIsDown())
    return -1;

  if (DisablePluginsOutput)
    return(-1);

  if((DWORD)PluginNumber >= (DWORD)CtrlObject->Plugins.PluginsCount)
    return(-1); // � ���������.

  int ExitCode;
  {
    VMenu FarMenu(Title,NULL,0,true,MaxHeight);
    CtrlObject->Macro.SetMode(MACRO_MENU);
    FarMenu.SetPosition(X,Y,0,0);
    if (BreakCode!=NULL)
      *BreakCode=-1;

    {
      string strTopic;
      if(Help::MkTopic(PluginNumber,HelpTopic,strTopic))
        FarMenu.SetHelp(strTopic);
    }

    if (Bottom!=NULL)
      FarMenu.SetBottomTitle(Bottom);

    // ����� ����� ����
    DWORD MenuFlags=0;
    if (Flags & FMENU_SHOWAMPERSAND)
      MenuFlags|=VMENU_SHOWAMPERSAND;
    if (Flags & FMENU_WRAPMODE)
      MenuFlags|=VMENU_WRAPMODE;
    if (Flags & FMENU_CHANGECONSOLETITLE)
      MenuFlags|=VMENU_CHANGECONSOLETITLE;
    FarMenu.SetFlags(MenuFlags);

    MenuItemEx CurItem;

    CurItem.Clear ();
    int Selected=0;

    if(Flags&FMENU_USEEXT)
    {
      struct FarMenuItemEx *ItemEx=(struct FarMenuItemEx*)Item;
      for (I=0; I < ItemsNumber; I++, ++ItemEx)
      {
        CurItem.Flags=ItemEx->Flags;
        CurItem.strName=L"";

        // ��������� MultiSelected, �.�. � ��� ������ ������ � ����� �� ������������, ��������� ������ ������
        DWORD SelCurItem=CurItem.Flags&LIF_SELECTED;
        CurItem.Flags&=~LIF_SELECTED;
        if(!Selected && !(CurItem.Flags&LIF_SEPARATOR) && SelCurItem)
        {
          CurItem.Flags|=SelCurItem;
          Selected++;
        }

        CurItem.strName=ItemEx->Text;

        CurItem.AccelKey=(CurItem.Flags&LIF_SEPARATOR)?0:ItemEx->AccelKey;
        FarMenu.AddItemW(&CurItem);
      }
    }
    else
    {
      for (I=0;I<ItemsNumber;I++)
      {
        CurItem.Flags=Item[I].Checked?(LIF_CHECKED|(Item[I].Checked&0xFFFF)):0;
        CurItem.Flags|=Item[I].Selected?LIF_SELECTED:0;
        CurItem.Flags|=Item[I].Separator?LIF_SEPARATOR:0;
        if(Item[I].Separator)
          CurItem.strName=L"";
        else
          CurItem.strName = Item[I].Text;

        DWORD SelCurItem=CurItem.Flags&LIF_SELECTED;
        CurItem.Flags&=~LIF_SELECTED;
        if(!Selected && !(CurItem.Flags&LIF_SEPARATOR) && SelCurItem)
        {
          CurItem.Flags|=SelCurItem;
          Selected++;
        }
        FarMenu.AddItemW(&CurItem);
      }
    }

    if(!Selected)
      FarMenu.SetSelectPos(0,1);

    // ����� ����, � ������� ���������
    if (Flags & FMENU_AUTOHIGHLIGHT)
      FarMenu.AssignHighlights(FALSE);
    if (Flags & FMENU_REVERSEAUTOHIGHLIGHT)
      FarMenu.AssignHighlights(TRUE);


    FarMenu.SetTitle(Title);

    FarMenu.Show();
    while (!FarMenu.Done() && !CloseFARMenu)
    {
      INPUT_RECORD ReadRec;
      int ReadKey=GetInputRecord(&ReadRec);
      if(ReadKey==KEY_CONSOLE_BUFFER_RESIZE)
      {
        LockScreen LckScr;
        FarMenu.Hide();
        FarMenu.Show();
      }
      else if (ReadRec.EventType==MOUSE_EVENT)
        FarMenu.ProcessMouse(&ReadRec.Event.MouseEvent);
      else if (ReadKey!=KEY_NONE)
      {
        if (BreakKeys!=NULL)
        {
          for (int I=0;BreakKeys[I]!=0;I++)
          {
            if(CtrlObject->Macro.IsExecuting())
            {
              int VirtKey,ControlState;
              TranslateKeyToVK(ReadKey,VirtKey,ControlState,&ReadRec);
            }
            if (ReadRec.Event.KeyEvent.wVirtualKeyCode==(BreakKeys[I] & 0xffff))
            {
              DWORD Flags=BreakKeys[I]>>16;
              /* $ 31.07.2001 IS
                 - ���: ���� ������� ����������� � ������ ������� ��
                   ctrl-key, alt-key ��� shift-key, ���� ���� ��� ����������
                   ����� �� ���� ������� � BreakKeys, � ������� ���� �����
                   ��������� ������� �� ������ key. �������: ��������� ����
                   ����� �� �������, �.�. ���������� ��� ������ �����.
              */
              DWORD RealFlags=ReadRec.Event.KeyEvent.dwControlKeyState &
                    (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED|
                    LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED|SHIFT_PRESSED);

              int Accept;
              if(RealFlags) // ������ shift, ctrl ��� alt
              {
                 Accept=FALSE; // �.�. ���� ������ �� ��������
                 if(Flags) // ������ ���� �������� � ������ ctrl|alt|shift
                 {
                   if ((Flags & PKF_CONTROL) &&
                       (RealFlags & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)))
                     Accept=TRUE;
                   if ((Flags & PKF_ALT) &&
                       (RealFlags & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)))
                     Accept=TRUE;
                   if ((Flags & PKF_SHIFT) && (RealFlags & SHIFT_PRESSED))
                     Accept=TRUE;
                 }
              }
              else
                 Accept=!Flags;  // TRUE ������, ���� ��� �� ����� ���������
                                 // ������ � ctrl|alt|shift
              /* IS $ */
              if (Accept)
              {
                if (BreakCode!=NULL)
                  *BreakCode=I;
                FarMenu.Hide();
//                  CheckScreenLock();
                return(FarMenu.GetSelectPos());
              }
            }
          }
        }
        FarMenu.ProcessKey(ReadKey);
      }
    }
    ExitCode=FarMenu.Modal::GetExitCode();
  }
//  CheckScreenLock();
  return(ExitCode);
}

/* $ 23.07.2000 SVS
   ������� ��� ������������ �������
*/
// ������� FarDefDlgProc ��������� ������� �� ���������
long WINAPI FarDefDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  if(hDlg)  // ��������� ������ ����� ��� hDlg=0
    return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
  return 0;
}

/* $ 28.07.2000 SVS
    ! � ����� � ���������� SendDlgMessage � ������ Dialog
      ������ ��������� ���������!
*/
// ������� ��������� �������
long WINAPI FarSendDlgMessage(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  if(hDlg) // ��������� ������ ����� ��� hDlg=0
    return Dialog::SendDlgMessage(hDlg,Msg,Param1,Param2);
  return 0;
}
/* SVS $ */

int WINAPI FarDialogFn(int PluginNumber,int X1,int Y1,int X2,int Y2,
           const wchar_t *HelpTopic,struct FarDialogItem *Item,int ItemsNumber)
{
  return FarDialogEx(PluginNumber,X1,Y1,X2,Y2,HelpTopic,Item,ItemsNumber,0,0,NULL,0);
}

/* $   13.12.2000 SVS
   ! FarDialogItem.Data - ����������� strcpy �������� �� memmove
   (�������� ������ ������������)
*/
#ifndef _MSC_VER
#pragma warn -par
#endif
/* ���� ������ ������� - ��������� ���� Flags - ������� ����, ���
   �� ����� ��� �� � �������
*/
static int Except_FarDialogEx(struct DialogItemEx *InternalItem)
{
  if(CtrlObject)
    CtrlObject->Plugins.Flags.Set(PSIF_DIALOG);

  // ���������
  delete[] InternalItem;

  Frame *frame;
  if((frame=FrameManager->GetBottomFrame()) != NULL)
  {
    //while(!frame->Refreshable()) // � ����� ��� ���� �����???
      frame->Unlock(); // ������ ����� :-)
  }
//  CheckScreenLock();
  FrameManager->RefreshFrame(); //??

  return EXCEPTION_CONTINUE_SEARCH; // ��������� ���������� ������� ����������!
}

static int FarDialogExSehed(Dialog& FarDialog, struct FarDialogItem* Item, struct DialogItemEx* InternalItem, int ItemsNumber)
{
  TRY
  {
    FarDialog.Process();
    Dialog::ConvertItemEx(CVTITEM_TOPLUGIN,Item,InternalItem,ItemsNumber);
    int I;
    for(I=0; I < ItemsNumber; ++I)
      InternalItem[I].ID=I;
    return FarDialog.GetExitCode();
  }
  EXCEPT (Except_FarDialogEx(InternalItem))
  {
    return -1;
  }
}

int WINAPI FarDialogEx(int PluginNumber,int X1,int Y1,int X2,int Y2,
           const wchar_t *HelpTopic,struct FarDialogItem *Item,int ItemsNumber,
           DWORD Reserved, DWORD Flags,
           FARWINDOWPROC DlgProc,long Param)

{
  if (FrameManager->ManagerIsDown())
    return -1;

  if (DisablePluginsOutput ||
      ItemsNumber <= 0 ||
      !Item ||
      IsBadReadPtr(Item,sizeof(struct FarDialogItem)*ItemsNumber))
    return(-1);

  if((DWORD)PluginNumber >= (DWORD)CtrlObject->Plugins.PluginsCount)
    return(-1); // � ���������.

  // ����! ������ ��������� ������������� X2 � Y2
  if(X2 < 0 || Y2 < 0)
  {
    return -1;
  }

  struct DialogItemEx *InternalItem=new DialogItemEx[ItemsNumber];

  if(!InternalItem)
    return -1;

  int ExitCode=-1;

  //struct PluginItem *CurPlugin=&CtrlObject->Plugins.PluginsData[PluginNumber];

  memset(InternalItem,0,sizeof(DialogItemEx)*ItemsNumber);

  Dialog::ConvertItemEx(CVTITEM_FROMPLUGIN,Item,InternalItem,ItemsNumber);

  Frame *frame;
  if((frame=FrameManager->GetBottomFrame()) != NULL)
    frame->Lock(); // ������� ���������� ������

  {
    Dialog FarDialog(InternalItem,ItemsNumber,DlgProc,Param);
    FarDialog.SetPosition(X1,Y1,X2,Y2);

    if(Flags & FDLG_WARNING)
      FarDialog.SetDialogMode(DMODE_WARNINGSTYLE);
    if(Flags & FDLG_SMALLDIALOG)
      FarDialog.SetDialogMode(DMODE_SMALLDIALOG);
    if(Flags & FDLG_NODRAWSHADOW)
      FarDialog.SetDialogMode(DMODE_NODRAWSHADOW);
    if(Flags & FDLG_NODRAWPANEL)
      FarDialog.SetDialogMode(DMODE_NODRAWPANEL);
    if(Flags & FDLG_NONMODAL)
      FarDialog.SetCanLoseFocus(TRUE);
    //FarDialog.SetOwnsItems(TRUE);
    FarDialog.SetHelp(HelpTopic);

    /* IS $ */
    /* $ 29.08.2000 SVS
       �������� ����� ������� - ������ � �������� ��� ������������ HelpTopic
    */
    FarDialog.SetPluginNumber(PluginNumber);
    /* SVS $ */

    int I;
    if(Opt.ExceptRules)
    {
      CtrlObject->Plugins.Flags.Clear(PSIF_DIALOG);
      ExitCode=FarDialogExSehed(FarDialog,Item,InternalItem,ItemsNumber);
    }
    else
    {
      FarDialog.Process();
      Dialog::ConvertItemEx(CVTITEM_TOPLUGIN,Item,InternalItem,ItemsNumber);
      for(I=0; I < ItemsNumber; ++I)
        InternalItem[I].ID=I;
      ExitCode=FarDialog.GetExitCode();
    }
  }

  delete[] InternalItem;

  /* $ 15.05.2002 SKV
    ������ ����������� ����� ����� ��, ��� ��������.
  */
  if(frame != NULL)
    frame->Unlock(); // ������ ����� :-)
 /* SKV $ */
//  CheckScreenLock();
  FrameManager->RefreshFrame(); //?? - //AY - ��� ����� ���� ��������� ������ ����� ������ �� �������
  return(ExitCode);
}
#ifndef _MSC_VER
#pragma warn +par
#endif
/* SVS 13.12.2000 $ */
/* SVS $ */

const char* WINAPI FarGetMsgFn(int PluginNumber,int MsgId)
{
  return(CtrlObject?CtrlObject->Plugins.FarGetMsg(PluginNumber,MsgId):"");
}

char* PluginsSet::FarGetMsg(int PluginNumber,int MsgId)
{
  if (PluginNumber<PluginsCount)
  {
    struct PluginItem *CurPlugin=PluginsData[PluginNumber];
    string strPath = CurPlugin->strModuleName;
    CutToSlashW(strPath);
    if (CurPlugin->Lang.Init(strPath))
      return(CurPlugin->Lang.GetMsg(MsgId));
  }
  return("");
}

/* $ 28.01.2001 SVS
   ! ��������� ������� ������� FarMessageFn()
*/

int WINAPI FarMessageFn(int PluginNumber,DWORD Flags,const wchar_t *HelpTopic,
                        const wchar_t * const *Items,int ItemsNumber,
                        int ButtonsNumber)
{
  if (FrameManager->ManagerIsDown())
    return -1;

  if (DisablePluginsOutput)
    return(-1);

  if ((!(Flags&(FMSG_ALLINONE|FMSG_ERRORTYPE)) && ItemsNumber<2) || !Items)
    return(-1);

  if(PluginNumber != -1 && (DWORD)PluginNumber >= (DWORD)CtrlObject->Plugins.PluginsCount)
    return(-1); // � ���������.

  wchar_t *SingleItems=NULL;
  wchar_t *Msg;
  int I;

  // ������ ���������� ����� ��� FMSG_ALLINONE
  if(Flags&FMSG_ALLINONE)
  {
    ItemsNumber=0;
    I=wcslen((wchar_t *)Items)+2;
    if((SingleItems=(wchar_t *)xf_malloc(I*sizeof (wchar_t))) == NULL)
      return -1;

    Msg=wcscpy(SingleItems,(wchar_t *)Items);
    while ((Msg = wcschr(Msg, L'\n')) != NULL)
    {
//      *Msg='\0';

      if(*++Msg == L'\0')
        break;
      ++ItemsNumber;
    }
    ItemsNumber++; //??
  }

  const wchar_t **MsgItems=(const wchar_t **)xf_malloc(sizeof(wchar_t*)*(ItemsNumber+ADDSPACEFORPSTRFORMESSAGE));
  if(!MsgItems)
  {
    xf_free(SingleItems);
    return(-1);
  }

  memset(MsgItems,0,sizeof(wchar_t*)*(ItemsNumber+ADDSPACEFORPSTRFORMESSAGE));

  if(Flags&FMSG_ALLINONE)
  {
    I=0;
    Msg=SingleItems;

    // ������ ���������� ����� � �������� �� ������
    wchar_t *MsgTemp;
    while ((MsgTemp = wcschr(Msg, L'\n')) != NULL)
    {
      *MsgTemp=L'\0';
      MsgItems[I]=Msg;
      Msg+=wcslen(Msg)+1;

      if(*Msg == L'\0')
        break;
      ++I;
    }
    if(*Msg)
    {
      MsgItems[I]=Msg;
    }
  }
  else
  {
    for (I=0; I < ItemsNumber; I++)
      MsgItems[I]=Items[I];
  }

  // ����������� �� ������
  if(ItemsNumber > ScrY-2)
  {
    ItemsNumber=ScrY-2-(Flags&0x000F0000?1:0);
  }

  /* $ 22.03.2001 tran
     ItemsNumber++ -> ++ItemsNumber
     �������� ��������� ������� */
  switch(Flags&0x000F0000)
  {
    case FMSG_MB_OK:
      ButtonsNumber=1;
      MsgItems[ItemsNumber++]=UMSG(MOk);
      break;
    case FMSG_MB_OKCANCEL:
      ButtonsNumber=2;
      MsgItems[ItemsNumber++]=UMSG(MOk);
      MsgItems[ItemsNumber++]=UMSG(MCancel);
      break;
    case FMSG_MB_ABORTRETRYIGNORE:
      ButtonsNumber=3;
      MsgItems[ItemsNumber++]=UMSG(MAbort);
      MsgItems[ItemsNumber++]=UMSG(MRetry);
      MsgItems[ItemsNumber++]=UMSG(MIgnore);
      break;
    case FMSG_MB_YESNO:
      ButtonsNumber=2;
      MsgItems[ItemsNumber++]=UMSG(MYes);
      MsgItems[ItemsNumber++]=UMSG(MNo);
      break;
    case FMSG_MB_YESNOCANCEL:
      ButtonsNumber=3;
      MsgItems[ItemsNumber++]=UMSG(MYes);
      MsgItems[ItemsNumber++]=UMSG(MNo);
      MsgItems[ItemsNumber++]=UMSG(MCancel);
      break;
    case FMSG_MB_RETRYCANCEL:
      ButtonsNumber=2;
      MsgItems[ItemsNumber++]=UMSG(MRetry);
      MsgItems[ItemsNumber++]=UMSG(MCancel);
      break;
  }
  /* tran $ */

  // ���������� �����
  if(PluginNumber != -1)
  {
    string strTopic;
    if(Help::MkTopic(PluginNumber,HelpTopic,strTopic))
      SetMessageHelp(strTopic);
  }

  // ���������������... �����
  Frame *frame;
  if((frame=FrameManager->GetBottomFrame()) != NULL)
    frame->Lock(); // ������� ���������� ������
  int MsgCode=MessageW(Flags,ButtonsNumber,MsgItems[0],MsgItems+1,ItemsNumber-1,PluginNumber);
  /* $ 15.05.2002 SKV
    ������ ����������� ���� ����� ��, ��� ��������.
  */
  if(frame != NULL)
    frame->Unlock(); // ������ ����� :-)
  /* SKV $ */
  //CheckScreenLock();

  if(SingleItems)
    xf_free(SingleItems);
  xf_free(MsgItems);
  return(MsgCode);
}

int WINAPI FarControl(HANDLE hPlugin,int Command,void *Param)
{
  _FCTLLOG(CleverSysLog("Control"));
  _FCTLLOG(SysLog("(hPlugin=0x%08X, Command=%s, Param=[%d/0x%08X])",hPlugin,_FCTL_ToName(Command),(int)Param,Param));
  _ALGO(CleverSysLog clv("FarControl"));
  _ALGO(SysLog("(hPlugin=0x%08X, Command=%s, Param=[%d/0x%08X])",hPlugin,_FCTL_ToName(Command),(int)Param,Param));

  if(Command == FCTL_CHECKPANELSEXIST)
    return CmdMode == FALSE?TRUE:FALSE;

  if (CmdMode || !CtrlObject || !FrameManager || FrameManager->ManagerIsDown())
    return 0;

  FilePanels *FPanels=CtrlObject->Cp();
  CommandLine *CmdLine=CtrlObject->CmdLine;

  switch(Command)
  {
    case FCTL_CLOSEPLUGIN:
      g_strDirToSet = NullToEmptyW((wchar_t *)Param);

    case FCTL_GETPANELINFO:
    case FCTL_GETANOTHERPANELINFO:
    case FCTL_GETPANELSHORTINFO:
    case FCTL_GETANOTHERPANELSHORTINFO:
    case FCTL_UPDATEPANEL:
    case FCTL_UPDATEANOTHERPANEL:
    case FCTL_REDRAWPANEL:
    case FCTL_REDRAWANOTHERPANEL:
    case FCTL_SETPANELDIR:
    case FCTL_SETANOTHERPANELDIR:
    case FCTL_SETSELECTION:
    case FCTL_SETANOTHERSELECTION:
    case FCTL_SETVIEWMODE:
    case FCTL_SETANOTHERVIEWMODE:
    case FCTL_SETSORTMODE:                 //  VVM 08.09.2000  + ����� ���������� �� �������
    case FCTL_SETANOTHERSORTMODE:
    case FCTL_SETSORTORDER:
    case FCTL_SETANOTHERSORTORDER:
    case FCTL_SETNUMERICSORT:
    case FCTL_SETANOTHERNUMERICSORT:
    {
      if(!FPanels)
        return FALSE;

      if (hPlugin==INVALID_HANDLE_VALUE)
      {
        if(FPanels->ActivePanel)
        {
          FPanels->ActivePanel->SetPluginCommand(Command,Param);
          return TRUE;
        }
        return FALSE; //??
      }

      HANDLE hInternal;
      Panel *LeftPanel=FPanels->LeftPanel;
      Panel *RightPanel=FPanels->RightPanel;
      int Processed=FALSE;
      struct PluginHandle *PlHandle;

      if (LeftPanel && LeftPanel->GetMode()==PLUGIN_PANEL)
      {
        PlHandle=(struct PluginHandle *)LeftPanel->GetPluginHandle();
        if(PlHandle && !IsBadReadPtr(PlHandle,sizeof(struct PluginHandle)))
        {
          hInternal=PlHandle->InternalHandle;
          if (hPlugin==hInternal)
          {
            LeftPanel->SetPluginCommand(Command,Param);
            Processed=TRUE;
          }
        }
      }

      if (RightPanel && RightPanel->GetMode()==PLUGIN_PANEL)
      {
        PlHandle=(struct PluginHandle *)RightPanel->GetPluginHandle();
        if(PlHandle && !IsBadReadPtr(PlHandle,sizeof(struct PluginHandle)))
        {
          hInternal=PlHandle->InternalHandle;
          if (hPlugin==hInternal)
          {
            RightPanel->SetPluginCommand(Command,Param);
            Processed=TRUE;
          }
        }
      }

      return(Processed);
    }

    case FCTL_SETUSERSCREEN:
    {
      if (!FPanels || !FPanels->LeftPanel || !FPanels->RightPanel)
        return(FALSE);

      KeepUserScreen++;
      FPanels->LeftPanel->ProcessingPluginCommand++;
      FPanels->RightPanel->ProcessingPluginCommand++;
      ScrBuf.FillBuf();
      SaveScreen SaveScr;
      {
        RedrawDesktop Redraw;
        CmdLine->Hide();
        SaveScr.RestoreArea(FALSE);
      }
      KeepUserScreen--;
      FPanels->LeftPanel->ProcessingPluginCommand--;
      FPanels->RightPanel->ProcessingPluginCommand--;
      return(TRUE);
    }

    case FCTL_GETCMDLINE:
    case FCTL_GETCMDLINESELECTEDTEXT:
    {
      if(Param && !IsBadWritePtr(Param,sizeof(char) * 1024))
      {
          string strParam;

        if (Command==FCTL_GETCMDLINE)
          CmdLine->GetStringW(strParam);
        else
          CmdLine->GetSelStringW(strParam);

        UnicodeToAnsi (strParam, (char*)Param, 1024-1);

        return TRUE;
      }
      return FALSE;
    }

    case FCTL_SETCMDLINE:
    case FCTL_INSERTCMDLINE:
    {
      string strParam;

      strParam.SetData (NullToEmpty((char*)Param), CP_OEMCP); //BUGBUG

      if (Command==FCTL_SETCMDLINE)
        CmdLine->SetStringW(strParam);
      else
        CmdLine->InsertStringW(strParam);
      CmdLine->Redraw();
      return(TRUE);
    }

    case FCTL_SETCMDLINEPOS:
    {
      if(Param && !IsBadReadPtr(Param,sizeof(int)))
      {
        CmdLine->SetCurPos(*(int *)Param);
        CmdLine->Redraw();
        return TRUE;
      }
      return FALSE;
    }

    case FCTL_GETCMDLINEPOS:
    {
      if(Param && !IsBadWritePtr(Param,sizeof(int)))
      {
        *(int *)Param=CmdLine->GetCurPos();
        return TRUE;
      }
      return FALSE;
    }

    case FCTL_GETCMDLINESELECTION:
    {
      CmdLineSelect *sel=(CmdLineSelect*)Param;
      if(sel && !IsBadWritePtr(sel,sizeof(struct CmdLineSelect)))
      {
        CmdLine->GetSelection(sel->SelStart,sel->SelEnd);
        return TRUE;
      }
      return FALSE;
    }

    case FCTL_SETCMDLINESELECTION:
    {
      CmdLineSelect *sel=(CmdLineSelect*)Param;
      if(sel && !IsBadReadPtr(sel,sizeof(struct CmdLineSelect)))
      {
        CmdLine->Select(sel->SelStart,sel->SelEnd);
        CmdLine->Redraw();
        return TRUE;
      }
      return FALSE;
    }

  }
  return(FALSE);
}


HANDLE WINAPI FarSaveScreen(int X1,int Y1,int X2,int Y2)
{
  if (FrameManager->ManagerIsDown())
    return NULL;

  if (X2==-1)
    X2=ScrX;
  if (Y2==-1)
    Y2=ScrY;

  return((HANDLE)(new SaveScreen(X1,Y1,X2,Y2,FALSE)));
}


void WINAPI FarRestoreScreen(HANDLE hScreen)
{
  if (FrameManager->ManagerIsDown())
    return;

  if (hScreen==NULL)
    ScrBuf.FillBuf();
  if (hScreen)
    delete (SaveScreen *)hScreen;
}


static void PR_FarGetDirListMsg(void)
{
  MessageW(MSG_DOWN,0,L"",UMSG(MPreparingList));
}

int WINAPI FarGetDirList(const wchar_t *Dir,FAR_FIND_DATA **pPanelItem,int *pItemsNumber)
{
  if (FrameManager->ManagerIsDown() || !Dir || !*Dir || !pItemsNumber || !pPanelItem)
    return FALSE;

  string strDirName;

  ConvertNameToFullW(Dir, strDirName);

  {
    SaveScreen SaveScr;
    clock_t StartTime=clock();
    int MsgOut=0;

    *pItemsNumber=0;
    *pPanelItem=NULL;

    FAR_FIND_DATA_EX FindData;
    string strFullName;
    ScanTree ScTree(FALSE);

    ScTree.SetFindPathW(strDirName,L"*.*");

    CutToSlashW (strDirName); //BUGBUG
    int DirLength=strDirName.GetLength();
    FAR_FIND_DATA *ItemsList=NULL;
    int ItemsNumber=0;
    while (ScTree.GetNextNameW(&FindData,strFullName))
    {
      if ((ItemsNumber & 31)==0)
      {
        if (CheckForEsc())
        {
          if(ItemsList)
            xf_free(ItemsList);
          SetPreRedrawFunc(NULL);
          return FALSE;
        }

        if (!MsgOut && clock()-StartTime > 500)
        {
          SetCursorType(FALSE,0);
          SetPreRedrawFunc(PR_FarGetDirListMsg);
          PR_FarGetDirListMsg();
          MsgOut=1;
        }

        ItemsList=(FAR_FIND_DATA*)xf_realloc(ItemsList,sizeof(*ItemsList)*(ItemsNumber+32+1));
        if (ItemsList==NULL)
        {
          *pItemsNumber=0;
          SetPreRedrawFunc(NULL);
          return FALSE;
        }
      }

      ItemsList[ItemsNumber].dwFileAttributes = FindData.dwFileAttributes;
      ItemsList[ItemsNumber].nFileSize = FindData.nFileSize;
      ItemsList[ItemsNumber].nPackSize = FindData.nPackSize;
      ItemsList[ItemsNumber].ftCreationTime = FindData.ftCreationTime;
      ItemsList[ItemsNumber].ftLastAccessTime = FindData.ftLastAccessTime;
      ItemsList[ItemsNumber].ftLastWriteTime = FindData.ftLastWriteTime;
      ItemsList[ItemsNumber].lpwszFileName = _wcsdup (FindData.strFileName);
      ItemsList[ItemsNumber].lpwszAlternateFileName = _wcsdup (FindData.strAlternateFileName);

      ItemsNumber++;
    }

    SetPreRedrawFunc(NULL);
    *pPanelItem=ItemsList;
    *pItemsNumber=ItemsNumber;
  }
  return TRUE;
}


static struct PluginPanelItemW *PluginDirList;
static int DirListItemsNumber;
static char PluginSearchPath[NM*16];
static int StopSearch;
static HANDLE hDirListPlugin;
static int PluginSearchMsgOut;
/*static struct
{
  PluginPanelItemW *Addr;
  int ItemsNumber;
} DirListNumbers[16];*/

static void FarGetPluginDirListMsg(const wchar_t *Name,DWORD Flags)
{
  MessageW(Flags,0,L"",UMSG(MPreparingList),Name);
  PreRedrawParam.Flags=Flags;
  PreRedrawParam.Param1=(void*)Name;
}

static void PR_FarGetPluginDirListMsg(void)
{
  FarGetPluginDirListMsg((const wchar_t *)PreRedrawParam.Param1,PreRedrawParam.Flags&(~MSG_KEEPBACKGROUND));
}

int WINAPI FarGetPluginDirList(int PluginNumber,
                               HANDLE hPlugin,
                               const wchar_t *Dir,
                               struct PluginPanelItemW **pPanelItem,
                               int *pItemsNumber)
{
  if (FrameManager->ManagerIsDown() || !Dir || !*Dir || !pItemsNumber || !pPanelItem)
    return FALSE;

  {
    if (wcscmp(Dir,L".")==0 || TestParentFolderNameW(Dir))
      return FALSE;

    static struct PluginHandle DirListPlugin;

    // � �� ����� �� ������ ���������� �� ������� ������?
    if (hPlugin==INVALID_HANDLE_VALUE)
    {
      /* $ 30.11.2001 DJ
         � ���������� �� ��� ������?
      */
      DWORD Handle = (DWORD) CtrlObject->Cp()->ActivePanel->GetPluginHandle();
      if (!Handle || Handle == 0xffffffff)
        return FALSE;

      DirListPlugin=*((struct PluginHandle *)Handle);
      /* DJ $ */
    }
    else
    {
      DirListPlugin.PluginNumber=PluginNumber;
      DirListPlugin.InternalHandle=hPlugin;
    }

    {
      SaveScreen SaveScr;

      SetPreRedrawFunc(NULL);
      {
        string strDirName;
        strDirName = Dir;
        TruncStrW(strDirName,30);
        CenterStrW(strDirName,strDirName,30);
        SetCursorType(FALSE,0);

        SetPreRedrawFunc(PR_FarGetPluginDirListMsg);
        FarGetPluginDirListMsg(strDirName,0);
        PluginSearchMsgOut=FALSE;

        hDirListPlugin=(HANDLE)&DirListPlugin;
        StopSearch=FALSE;
        *pItemsNumber=DirListItemsNumber=0;
        *pPanelItem=PluginDirList=NULL;

        struct OpenPluginInfoW Info;
        CtrlObject->Plugins.GetOpenPluginInfo(hDirListPlugin,&Info);

        string strPrevDir = Info.CurDir;

        if (CtrlObject->Plugins.SetDirectory(hDirListPlugin,Dir,OPM_FIND))
        {
            UnicodeToAnsi(Dir, PluginSearchPath, sizeof(PluginSearchPath)-1);
          //xstrncpy(PluginSearchPath,Dir,sizeof(PluginSearchPath)-1);
          strncat(PluginSearchPath,"\x1",sizeof(PluginSearchPath)-1);

          ScanPluginDir();

          *pPanelItem=PluginDirList;
          *pItemsNumber=DirListItemsNumber;
          CtrlObject->Plugins.SetDirectory(hDirListPlugin,L"..",OPM_FIND);
          PluginPanelItemW *PanelData=NULL;

          int ItemCount=0;
          if (CtrlObject->Plugins.GetFindData(hDirListPlugin,&PanelData,&ItemCount,OPM_FIND))
            CtrlObject->Plugins.FreeFindData(hDirListPlugin,PanelData,ItemCount);

          struct OpenPluginInfoW NewInfo;
          CtrlObject->Plugins.GetOpenPluginInfo(hDirListPlugin,&NewInfo);

          if ( LocalStricmpW (strPrevDir, NewInfo.CurDir)!=0)
            CtrlObject->Plugins.SetDirectory(hDirListPlugin,strPrevDir,OPM_FIND);
        }
      }
      SetPreRedrawFunc(NULL);
    }
  }

  /*if (!StopSearch)
    for (int I=0;I<sizeof(DirListNumbers)/sizeof(DirListNumbers[0]);I++)
      if (DirListNumbers[I].Addr==NULL)
      {
        DirListNumbers[I].Addr=*pPanelItem;
        DirListNumbers[I].ItemsNumber=*pItemsNumber;
        break;
      }*/
  return(!StopSearch);
}

/* $ 30.11.2001 DJ
   ������� � ������� ����� ��� ��� ����������� ������ � ScanPluginDir()
*/

static void CopyPluginDirItem (PluginPanelItemW *CurPanelItem)
{
  string strFullName;

  strFullName.SetData(PluginSearchPath, CP_OEMCP); //BUGBUG
  strFullName += CurPanelItem->FindData.lpwszFileName;

  wchar_t *lpwszFullName = strFullName.GetBuffer ();

  for (int I=0;lpwszFullName[I]!=0;I++)
    if (lpwszFullName[I]==L'\x1')
      lpwszFullName[I]=L'\\';

  strFullName.ReleaseBuffer ();

  PluginPanelItemW *DestItem=PluginDirList+DirListItemsNumber;
  *DestItem=*CurPanelItem;
  if (CurPanelItem->UserData && (CurPanelItem->Flags & PPIF_USERDATA))
  {
    DWORD Size=*(DWORD *)CurPanelItem->UserData;
    /* $ 13.07.2000 SVS
       ������ new ����� ������������ malloc
    */
    DestItem->UserData=(DWORD)xf_malloc(Size);
    /* SVS $*/
    memcpy((void *)DestItem->UserData,(void *)CurPanelItem->UserData,Size);
  }

  DestItem->FindData.lpwszFileName = _wcsdup (strFullName);
  DirListItemsNumber++;
}

/* DJ $ */


void ScanPluginDir()
{
  int I;
  PluginPanelItemW *PanelData=NULL;
  int ItemCount=0;
  int AbortOp=FALSE;

  string strDirName;
  strDirName.SetData(PluginSearchPath, CP_OEMCP); //BUGBUG

  wchar_t *lpwszDirName = strDirName.GetBuffer();

  for (I=0;lpwszDirName[I]!=0;I++)
    if (lpwszDirName[I]=='\x1')
      lpwszDirName[I]=lpwszDirName[I+1]==0 ? 0:L'\\';

  strDirName.ReleaseBuffer();

  TruncStrW(strDirName,30);
  CenterStrW(strDirName,strDirName,30);

  if (CheckForEscSilent())
  {
    if (Opt.Confirm.Esc) // ����� ���������� ������?
      AbortOp=TRUE;

    if (ConfirmAbortOp())
      StopSearch=TRUE;
  }

  FarGetPluginDirListMsg(strDirName,AbortOp?0:MSG_KEEPBACKGROUND);

  if (StopSearch || !CtrlObject->Plugins.GetFindData(hDirListPlugin,&PanelData,&ItemCount,OPM_FIND))
    return;
  struct PluginPanelItemW *NewList=(struct PluginPanelItemW *)xf_realloc(PluginDirList,1+sizeof(*PluginDirList)*(DirListItemsNumber+ItemCount));
  if (NewList==NULL)
  {
    StopSearch=TRUE;
    return;
  }
  PluginDirList=NewList;
  for (I=0;I<ItemCount && !StopSearch;I++)
  {
    PluginPanelItemW *CurPanelItem=PanelData+I;
    /* $ 30.11.2001 DJ
       ������� ����������� ������ � �������
    */
    if ((CurPanelItem->FindData.dwFileAttributes & FA_DIREC)==0)
      CopyPluginDirItem (CurPanelItem);
    /* DJ $ */
  }
  for (I=0;I<ItemCount && !StopSearch;I++)
  {
    PluginPanelItemW *CurPanelItem=PanelData+I;
    if ((CurPanelItem->FindData.dwFileAttributes & FA_DIREC) &&
        wcscmp(CurPanelItem->FindData.lpwszFileName,L".")!=0 &&
        !TestParentFolderNameW(CurPanelItem->FindData.lpwszFileName))

    {
      struct PluginPanelItemW *NewList=(struct PluginPanelItemW *)xf_realloc(PluginDirList,sizeof(*PluginDirList)*(DirListItemsNumber+1));
      if (NewList==NULL)
      {
        StopSearch=TRUE;
        return;
      }
      PluginDirList=NewList;
      /* $ 30.11.2001 DJ
         ���������� ����� ������� ��� ����������� FindData (�� ��������
         ���������� PPIF_USERDATA)
      */
      CopyPluginDirItem (CurPanelItem);
      /* DJ $ */

      string strFileName = CurPanelItem->FindData.lpwszFileName;

      if (CtrlObject->Plugins.SetDirectory(hDirListPlugin,strFileName,OPM_FIND))
      {
          char szFileName[NM];

          UnicodeToAnsi(CurPanelItem->FindData.lpwszFileName, szFileName, sizeof (szFileName)-1);

        strcat(PluginSearchPath,szFileName);
        strcat(PluginSearchPath,"\x1");
        if (strlen(PluginSearchPath)<sizeof(PluginSearchPath)-NM)
          ScanPluginDir();
        *strrchr(PluginSearchPath,'\x1')=0;
        char *NamePtr=strrchr(PluginSearchPath,'\x1');
        if (NamePtr!=NULL)
          *(NamePtr+1)=0;
        else
          *PluginSearchPath=0;
        if (!CtrlObject->Plugins.SetDirectory(hDirListPlugin,L"..",OPM_FIND))
        {
          StopSearch=TRUE;
          break;
        }
      }
    }
  }
  CtrlObject->Plugins.FreeFindData(hDirListPlugin,PanelData,ItemCount);
}


void WINAPI FarFreeDirList(FAR_FIND_DATA *PanelItem, int nItemsNumber)
{
  for (int I=0;I<nItemsNumber;I++)
  {
    FAR_FIND_DATA *CurPanelItem=PanelItem+I;

    apiFreeFindData (CurPanelItem);
  }

  xf_free (PanelItem);
}


void WINAPI FarFreePluginDirList(PluginPanelItemW *PanelItem, int ItemsNumber)
{
  if (PanelItem==NULL)
    return;
  int I;

  for (I=0;I<ItemsNumber;I++)
  {
    PluginPanelItemW *CurPanelItem=PanelItem+I;

    if (CurPanelItem->UserData && (CurPanelItem->Flags & PPIF_USERDATA))
      /* $ 13.07.2000 SVS
        ��� ������� ������������ malloc
      */
      xf_free((void *)CurPanelItem->UserData);
      /* SVS $*/

    apiFreeFindData (&CurPanelItem->FindData);
  }
  /* $ 13.07.2000 SVS
    ��� ������� ������������ realloc
  */
  // ��� ��� �� ������??????????
  //xf_free(static_cast<void*>(const_cast<PluginPanelItem *>(PanelItem)));
  xf_free((void*)PanelItem);
  /* SVS $*/
}


#if defined(__BORLANDC__)
#pragma warn -par
#endif
int WINAPI FarViewer(const wchar_t *FileName,const wchar_t *Title,
                     int X1,int Y1,int X2, int Y2,DWORD Flags)
{
  if (FrameManager->ManagerIsDown())
    return FALSE;


  class ConsoleTitle ct;
  /* $ 09.09.2001 IS */
  int DisableHistory=(Flags & VF_DISABLEHISTORY)?TRUE:FALSE;
  /* IS $ */
  /* $ 15.05.2002 SKV
    �������� ����� ������������ ��������� ������ �� ����������.
  */
  if( FrameManager->InModalEV())
  {
    Flags&=~VF_NONMODAL;
  }
  /* SKV $ */

  if (Flags & VF_NONMODAL)
  {
    /* 09.09.2001 IS ! ������� ��� ����� � �������, ���� ����������� */
    FileViewer *Viewer=new FileViewer(FileName,TRUE,DisableHistory,Title,X1,Y1,X2,Y2);
    /* IS $ */
    if(!Viewer)
      return FALSE;
    /* $ 14.06.2002 IS
       ��������� VF_DELETEONLYFILEONCLOSE - ���� ���� ����� ����� ������
       ��������� �� ��������� � VF_DELETEONCLOSE
    */
    if (Flags & (VF_DELETEONCLOSE|VF_DELETEONLYFILEONCLOSE))
      Viewer->SetTempViewName(FileName,(Flags&VF_DELETEONCLOSE)?TRUE:FALSE);
    /* IS $ */
    /* $ 12.05.2001 DJ */
    Viewer->SetEnableF6 ((Flags & VF_ENABLE_F6) != 0);
    /* DJ $ */

    /* $ 21.05.2002 SKV
      ��������� ���� ���� ������ ����
      �� ��� ������ ����.
    */
    if(!(Flags&VF_IMMEDIATERETURN))
    {
      FrameManager->ExecuteNonModal();
    }
    else
    {
      if(GlobalSaveScrPtr)
        GlobalSaveScrPtr->Discard();
      FrameManager->PluginCommit();
    }
    /* SKV $ */
  }
  else
  {
    /* 09.09.2001 IS ! ������� ��� ����� � �������, ���� ����������� */
    FileViewer Viewer (FileName,FALSE,DisableHistory,Title,X1,Y1,X2,Y2);
    /* IS $ */
    /* $ 28.05.2001 �� ��������� �����, ������� ����� ����� ������� ��������� ���� */
    Viewer.SetDynamicallyBorn(false);
    FrameManager->EnterModalEV();
    FrameManager->ExecuteModal();
    FrameManager->ExitModalEV();
    /* $ 14.06.2002 IS
       ��������� VF_DELETEONLYFILEONCLOSE - ���� ���� ����� ����� ������
       ��������� �� ��������� � VF_DELETEONCLOSE
    */
    if (Flags & (VF_DELETEONCLOSE|VF_DELETEONLYFILEONCLOSE))
      Viewer.SetTempViewName(FileName,(Flags&VF_DELETEONCLOSE)?TRUE:FALSE);
    /* IS $ */
    /* $ 12.05.2001 DJ */
    Viewer.SetEnableF6 ((Flags & VF_ENABLE_F6) != 0);
    /* DJ $ */
    if (!Viewer.GetExitCode()){
      return FALSE;
    }
  }
  return(TRUE);
}


int WINAPI FarEditor(const wchar_t *FileName,const wchar_t *Title,
                     int X1,int Y1,int X2,
                     int Y2,DWORD Flags,int StartLine,int StartChar)
{
  if (FrameManager->ManagerIsDown())
    return EEC_OPEN_ERROR;

  ConsoleTitle ct;

  /* $ 12.07.2000 IS
   �������� ������ ��������� (������ ��� ��������������) � ��������
   ������������ ���������, ���� ���� ��������������� ����
  */
  /* $ 21.03.2001 VVM
    + ��������� ����� EF_CREATENEW */
  int CreateNew = (Flags & EF_CREATENEW)?TRUE:FALSE;
  /* VVM $ */
  /* $ 09.09.2001 IS */
  int DisableHistory=(Flags & EF_DISABLEHISTORY)?TRUE:FALSE;
  /* $ 14.06.2002 IS
     ��������� EF_DELETEONLYFILEONCLOSE - ���� ���� ����� ����� ������
     ��������� �� ��������� � EF_DELETEONCLOSE
  */
  int DeleteOnClose = 0;
  if(Flags & EF_DELETEONCLOSE)
    DeleteOnClose = 1;
  else if(Flags & EF_DELETEONLYFILEONCLOSE)
    DeleteOnClose = 2;
  /* IS 14.06.2002 $ */
  /* IS 09.09.2001 $ */
  int OpMode=FEOPMODE_QUERY;
#if 0
  if((Flags&(EF_USEEXISTING|EF_BREAKIFOPEN)) != (EF_USEEXISTING|EF_BREAKIFOPEN))
    OpMode=(Flags&EF_USEEXISTING)?FEOPMODE_USEEXISTING:FEOPMODE_BREAKIFOPEN;
#endif

  /*$ 15.05.2002 SKV
    �������� ����� ������������ ���������, ���� ��������� � ���������
    ��������� ��� ������.
  */
  if (FrameManager->InModalEV())
  {
    Flags&=~EF_NONMODAL;
  }
  /* SKV $ */

  int ExitCode=EEC_OPEN_ERROR;
  if (Flags & EF_NONMODAL)
  {
    /* 09.09.2001 IS ! ������� ��� ����� � �������, ���� ����������� */
    FileEditor *Editor=new FileEditor(FileName,CreateNew,TRUE,
                                      StartLine,StartChar,Title,
                                      X1,Y1,X2,Y2,DisableHistory,
                                      DeleteOnClose,OpMode);
    /* IS $ */
    // ��������� - �������� ���� �������� (������ ��������� XC_OPEN_ERROR - ��. ��� FileEditor::Init())
    if (Editor && Editor->GetExitCode() == XC_OPEN_ERROR)
    {
      delete Editor;
      Editor=NULL;
    }

    if (Editor)
    {
      /* $ 12.05.2001 DJ */
      Editor->SetEnableF6 ((Flags & EF_ENABLE_F6) != 0);
      /* DJ $ */
      Editor->SetPluginTitle(Title);
      /* $ 21.05.2002 SKV
        ��������� ���� ����, ������ ����
        �� ��� ������ ����.
      */
      if(!(Flags&EF_IMMEDIATERETURN))
      {
        FrameManager->ExecuteNonModal();
      }
      else
      {
        if(GlobalSaveScrPtr)
          GlobalSaveScrPtr->Discard();
        FrameManager->PluginCommit();
      }
      /* SKV $ */
      ExitCode=XC_MODIFIED;
    }
  }
  else
  {
    /* 09.09.2001 IS ! ������� ��� ����� � �������, ���� ����������� */
    FileEditor Editor(FileName,CreateNew,FALSE,
                      StartLine,StartChar,Title,
                      X1,Y1,X2,Y2,DisableHistory,
                      DeleteOnClose,OpMode);
    /* IS $ */

    // �������� ������������ (������ ������ ����� ����)
    if(Editor.GetExitCode() != XC_OPEN_ERROR)
    {
      Editor.SetDynamicallyBorn(false);
      /* $ 12.05.2001 DJ */
      Editor.SetEnableF6 ((Flags & EF_ENABLE_F6) != 0);
      /* DJ $ */
      Editor.SetPluginTitle(Title);
      /* $ 15.05.2002 SKV
        ����������� ���� � ����� �/�� ���������� ���������.
      */
      FrameManager->EnterModalEV();
      FrameManager->ExecuteModal();
      FrameManager->ExitModalEV();
      /* SKV $ */
      ExitCode = Editor.GetExitCode();
      if (ExitCode && ExitCode != XC_LOADING_INTERRUPTED)
      {
#if 0
         if(OpMode==FEOPMODE_BREAKIFOPEN && ExitCode==XC_QUIT)
           ExitCode = XC_OPEN_ERROR;
         else
#endif
           ExitCode = Editor.IsFileChanged()?XC_MODIFIED:XC_NOT_MODIFIED;
      }
    }
  }
  return ExitCode;
  /* IS $ */
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif


int WINAPI FarCmpName(const wchar_t *pattern,const wchar_t *string,int skippath)
{
  return(CmpNameW(pattern,string,skippath));
}


int WINAPI FarCharTable(int Command,char *Buffer,int BufferSize)
{
  if (FrameManager->ManagerIsDown())
    return -1;

  struct CharTableSet TableSet;

  if (Command==FCT_DETECT)
  {
    string strDataFileName;
    FILE *DataFile;
    /* $ 19.06.2001
       - ���: �� �������� ���������������.
         ��, ����, ����� �� �� return -1 ������������� � 268??
    */
    if (!FarMkTempExW(strDataFileName) || (DataFile=_wfopen(strDataFileName,L"w+b"))==NULL)
      return(-1);
    /* IS $ */
    fwrite(Buffer,1,BufferSize,DataFile);
    fseek(DataFile,0,SEEK_SET);
    int TableNum;
    int DetectCode=DetectTable(DataFile,&TableSet,TableNum);
    fclose(DataFile);
    _wremove(strDataFileName);
    return(DetectCode ? TableNum-1:-1);
  }

  if (BufferSize > sizeof(struct CharTableSet))
    return(-1);

  /* $ 07.08.2001 IS
       ��� ������� �������� ��������� ������� ��� OEM
  */
  memcpy(&TableSet,Buffer,Min((int)sizeof(CharTableSet),BufferSize));
  /* $ 17.03.2002 IS �� ����������� ���������� �������� TableName */
  if (!PrepareTable(&TableSet,Command,TRUE))
  /* IS $ */
  {
    for(unsigned int i=0;i<256;++i)
    {
      TableSet.EncodeTable[i]=TableSet.DecodeTable[i]=i;
      TableSet.UpperTable[i]=LocalUpper(i);
      TableSet.LowerTable[i]=LocalLower(i);
    }
    xstrncpy(TableSet.TableName,MSG(MGetTableNormalText),sizeof(TableSet.TableName));
    // *TableSet.RFCCharset=0; // ���� ���!
    Command=-1;
  }
  memcpy(Buffer,&TableSet,BufferSize);
  /* IS $ */
  return(Command);
}


void WINAPI FarText(int X,int Y,int Color,const wchar_t *Str)
{
  if (FrameManager->ManagerIsDown())
    return;
  if (Str==NULL)
  {
    int PrevLockCount=ScrBuf.GetLockCount();
    ScrBuf.SetLockCount(0);
    ScrBuf.Flush();
    ScrBuf.SetLockCount(PrevLockCount);
  }
  else
  {
    /* $ 22.08.2000 SVS
       ��������� �������� ������ �� FarText.
    */
    TextW(X,Y,Color,Str);
    /* SVS $ */
  }
}


int WINAPI FarEditorControl(int Command,void *Param)
{
  if (FrameManager->ManagerIsDown() || !CtrlObject->Plugins.CurEditor)
    return(0);
  return(CtrlObject->Plugins.CurEditor->EditorControl(Command,Param));
}

/* $ 27.09.2000 SVS
  ���������� ��������
*/
int WINAPI FarViewerControl(int Command,void *Param)
{
  if (FrameManager->ManagerIsDown() || !CtrlObject->Plugins.CurViewer)
    return(0);
  return(CtrlObject->Plugins.CurViewer->ViewerControl(Command,Param));
}
/* SVS $ */
