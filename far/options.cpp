/*
options.cpp

Фаровское горизонтальное меню (вызов hmenu.cpp с конкретными параметрами)

*/

#include "headers.hpp"
#pragma hdrstop

#include "lang.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "keys.hpp"
#include "hmenu.hpp"
#include "vmenu.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "chgmmode.hpp"
#include "filelist.hpp"
#include "hilight.hpp"
#include "cmdline.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "history.hpp"

void ShellOptions(int LastCommand,MOUSE_EVENT_RECORD *MouseEvent)
{
  int I;
  struct MenuData LeftMenu[]=
  {
    (char *)MMenuBriefView,LIF_SELECTED,KEY_CTRL1,
    (char *)MMenuMediumView,0,KEY_CTRL2,
    (char *)MMenuFullView,0,KEY_CTRL3,
    (char *)MMenuWideView,0,KEY_CTRL4,
    (char *)MMenuDetailedView,0,KEY_CTRL5,
    (char *)MMenuDizView,0,KEY_CTRL6,
    (char *)MMenuLongDizView,0,KEY_CTRL7,
    (char *)MMenuOwnersView,0,KEY_CTRL8,
    (char *)MMenuLinksView,0,KEY_CTRL9,
    (char *)MMenuAlternativeView,0,KEY_CTRL0,
    "",LIF_SEPARATOR,0,
    (char *)MMenuInfoPanel,0,KEY_CTRLL,
    (char *)MMenuTreePanel,0,KEY_CTRLT,
    (char *)MMenuQuickView,0,KEY_CTRLQ,
    "",LIF_SEPARATOR,0,
    (char *)MMenuSortModes,0,KEY_CTRLF12,
    (char *)MMenuLongNames,0,KEY_CTRLN,
    (char *)MMenuTogglePanel,0,KEY_CTRLF1,
    (char *)MMenuReread,0,KEY_CTRLR,
    (char *)MMenuChangeDrive,0,KEY_ALTF1,
  };

  struct MenuData FilesMenu[]=
  {
    (char *)MMenuView,LIF_SELECTED,KEY_F3,
    (char *)MMenuEdit,0,KEY_F4,
    (char *)MMenuCopy,0,KEY_F5,
    (char *)MMenuMove,0,KEY_F6,
    (char *)MMenuCreateFolder,0,KEY_F7,
    (char *)MMenuDelete,0,KEY_F8,
    (char *)MMenuWipe,0,KEY_ALTDEL,
    "",LIF_SEPARATOR,0,
    (char *)MMenuAdd,0,KEY_SHIFTF1,
    (char *)MMenuExtract,0,KEY_SHIFTF2,
    (char *)MMenuArchiveCommands,0,KEY_SHIFTF3,
    "",LIF_SEPARATOR,0,
    (char *)MMenuAttributes,0,KEY_CTRLA,
    (char *)MMenuApplyCommand,0,KEY_CTRLG,
    (char *)MMenuDescribe,0,KEY_CTRLZ,
    "",LIF_SEPARATOR,0,
    (char *)MMenuSelectGroup,0,KEY_ADD,
    (char *)MMenuUnselectGroup,0,KEY_SUBTRACT,
    (char *)MMenuInvertSelection,0,KEY_MULTIPLY,
    (char *)MMenuRestoreSelection,0,KEY_CTRLM,
  };


  struct MenuData CmdMenu[]=
  {
  /* 00 */(char *)MMenuFindFile,LIF_SELECTED,KEY_ALTF7,
  /* 01 */(char *)MMenuHistory,0,KEY_ALTF8,
  /* 02 */(char *)MMenuVideoMode,0,KEY_ALTF9,
  /* 03 */(char *)MMenuFindFolder,0,KEY_ALTF10,
  /* 04 */(char *)MMenuViewHistory,0,KEY_ALTF11,
  /* 05 */(char *)MMenuFoldersHistory,0,KEY_ALTF12,
  /* 06 */"",LIF_SEPARATOR,0,
  /* 07 */(char *)MMenuSwapPanels,0,KEY_CTRLU,
  /* 08 */(char *)MMenuTogglePanels,0,KEY_CTRLO,
  /* 09 */(char *)MMenuCompareFolders,0,0,
  /* 10 */"",LIF_SEPARATOR,0,
  /* 11 */(char *)MMenuUserMenu,0,0,
  /* 12 */(char *)MMenuFileAssociations,0,0,
  /* 13 */(char *)MMenuFolderShortcuts,0,0,
  /* 14 */(char *)MMenuFilter,0,KEY_CTRLI,
  /* 15 */"",LIF_SEPARATOR,0,
  /* 16 */(char *)MMenuPluginCommands,0,KEY_F11,
  /* 17 */(char *)MMenuWindowsList,0,KEY_F12,
  /* 18 */(char *)MMenuProcessList,0,KEY_CTRLW,
  /* 19 */(char *)MMenuHotPlugList,0,0,
  };


  struct MenuData OptionsMenu[]=
  {
   /* 00 */(char *)MMenuSystemSettings,LIF_SELECTED,0,
   /* 01 */(char *)MMenuPanelSettings,0,0,
   /* 02 */(char *)MMenuInterface,0,0,
   /* 03 */(char *)MMenuLanguages,0,0,
   /* 04 */(char *)MMenuPluginsConfig,0,0,
   /* 05 */(char *)MMenuDialogSettings,0,0,
   /* 06 */"",LIF_SEPARATOR,0,
   /* 07 */(char *)MMenuConfirmation,0,0,
   /* 08 */(char *)MMenuFilePanelModes,0,0,
   /* 09 */(char *)MMenuFileDescriptions,0,0,
   /* 10 */(char *)MMenuFolderInfoFiles,0,0,
   /* 11 */"",LIF_SEPARATOR,0,
   /* 12 */(char *)MMenuViewer,0,0,
   /* 13 */(char *)MMenuEditor,0,0,
   /* 14 */"",LIF_SEPARATOR,0,
   /* 15 */(char *)MMenuColors,0,0,
   /* 16 */(char *)MMenuFilesHighlighting,0,0,
   /* 17 */"",LIF_SEPARATOR,0,
   /* 18 */(char *)MMenuSaveSetup,0,KEY_SHIFTF9,
  };


  struct MenuData RightMenu[]=
  {
    (char *)MMenuBriefView,LIF_SELECTED,KEY_CTRL1,
    (char *)MMenuMediumView,0,KEY_CTRL2,
    (char *)MMenuFullView,0,KEY_CTRL3,
    (char *)MMenuWideView,0,KEY_CTRL4,
    (char *)MMenuDetailedView,0,KEY_CTRL5,
    (char *)MMenuDizView,0,KEY_CTRL6,
    (char *)MMenuLongDizView,0,KEY_CTRL7,
    (char *)MMenuOwnersView,0,KEY_CTRL8,
    (char *)MMenuLinksView,0,KEY_CTRL9,
    (char *)MMenuAlternativeView,0,KEY_CTRL0,
    "",LIF_SEPARATOR,0,
    (char *)MMenuInfoPanel,0,KEY_CTRLL,
    (char *)MMenuTreePanel,0,KEY_CTRLT,
    (char *)MMenuQuickView,0,KEY_CTRLQ,
    "",LIF_SEPARATOR,0,
    (char *)MMenuSortModes,0,KEY_CTRLF12,
    (char *)MMenuLongNames,0,KEY_CTRLN,
    (char *)MMenuTogglePanelRight,0,KEY_CTRLF2,
    (char *)MMenuReread,0,KEY_CTRLR,
    (char *)MMenuChangeDriveRight,0,KEY_ALTF2,
  };


  struct HMenuData MainMenu[]=
  {
    MSG(MMenuLeftTitle),1,LeftMenu,sizeof(LeftMenu)/sizeof(LeftMenu[0]),"LeftRightMenu",
    MSG(MMenuFilesTitle),0,FilesMenu,sizeof(FilesMenu)/sizeof(FilesMenu[0]),"FilesMenu",
    MSG(MMenuCommandsTitle),0,CmdMenu,sizeof(CmdMenu)/sizeof(CmdMenu[0]),"CmdMenu",
    MSG(MMenuOptionsTitle),0,OptionsMenu,sizeof(OptionsMenu)/sizeof(OptionsMenu[0]),"OptMenu",
    MSG(MMenuRightTitle),0,RightMenu,sizeof(RightMenu)/sizeof(RightMenu[0]),"LeftRightMenu"
  };

  static int LastHItem=-1,LastVItem=0;
  int HItem,VItem;

  // дисаблим
  CmdMenu[19].SetDisable(!CheckInitSetupAPI());

  if (Opt.Policies.DisabledOptions)
    for(I=0; I < sizeof(OptionsMenu)/sizeof(OptionsMenu[0]); ++I)
    {
      if(I > 6)
        OptionsMenu[I].SetDisable((Opt.Policies.DisabledOptions >> (I-1)) & 1);
      else
        OptionsMenu[I].SetDisable((Opt.Policies.DisabledOptions >> I) & 1);
    }

  // расставим "чеки" для левой панели
  switch(CtrlObject->Cp()->LeftPanel->GetType())
  {
    case FILE_PANEL:
      {
        int MenuLine=CtrlObject->Cp()->LeftPanel->GetViewMode()-VIEW_0;
        if (MenuLine<10)
          if (MenuLine==0)
            LeftMenu[9].SetCheck(1);
          else
            LeftMenu[MenuLine-1].SetCheck(1);
      }
      break;
    case INFO_PANEL:
      LeftMenu[11].SetCheck(1);
      break;
    case TREE_PANEL:
      LeftMenu[12].SetCheck(1);
      break;
    case QVIEW_PANEL:
      LeftMenu[13].SetCheck(1);
      break;
  }

  LeftMenu[16].SetCheck(!CtrlObject->Cp()->LeftPanel->GetShowShortNamesMode());

  // расставим "чеки" для правой панели
  switch(CtrlObject->Cp()->RightPanel->GetType())
  {
    case FILE_PANEL:
      {
        int MenuLine=CtrlObject->Cp()->RightPanel->GetViewMode()-VIEW_0;
        if (MenuLine<10)
          if (MenuLine==0)
            RightMenu[9].SetCheck(1);
          else
            RightMenu[MenuLine-1].SetCheck(1);
      }
      break;
    case INFO_PANEL:
      RightMenu[11].SetCheck(1);
      break;
    case TREE_PANEL:
      RightMenu[12].SetCheck(1);
      break;
    case QVIEW_PANEL:
      RightMenu[13].SetCheck(1);
      break;
  }

  RightMenu[16].SetCheck(!CtrlObject->Cp()->RightPanel->GetShowShortNamesMode());

  // Навигация по меню
  {
    HMenu HOptMenu(MainMenu,sizeof(MainMenu)/sizeof(MainMenu[0]));
    HOptMenu.SetHelp("Menus");
    HOptMenu.SetPosition(0,0,ScrX,0);
    if (LastCommand)
    {
      struct MenuData *VMenuTable[]={LeftMenu,FilesMenu,CmdMenu,OptionsMenu,RightMenu};
      /* $ 12.05.2001 DJ
         корректно обрабатываем нажатие Shift-F10 до того, как нажимали F9
      */
      int HItemToShow = LastHItem;
      if (HItemToShow == -1)
      {
        if (CtrlObject->Cp()->ActivePanel==CtrlObject->Cp()->RightPanel &&
          CtrlObject->Cp()->ActivePanel->IsVisible())
          HItemToShow = 4;
        else
          HItemToShow = 0;
      }
      MainMenu[0].Selected=0;
      MainMenu[HItemToShow].Selected=1;
      VMenuTable[HItemToShow][0].SetSelect(0);
      VMenuTable[HItemToShow][LastVItem].SetSelect(1);
      /* DJ $ */
      HOptMenu.Show();

      {
        ChangeMacroMode MacroMode(MACRO_MAINMENU);
        HOptMenu.ProcessKey(KEY_DOWN);
      }

    }
    else
      if (CtrlObject->Cp()->ActivePanel==CtrlObject->Cp()->RightPanel &&
          CtrlObject->Cp()->ActivePanel->IsVisible())
      {
        MainMenu[0].Selected=0;
        MainMenu[4].Selected=1;
      }
    if (MouseEvent!=NULL)
    {
      ChangeMacroMode MacroMode(MACRO_MAINMENU);
      HOptMenu.Show();
      HOptMenu.ProcessMouse(MouseEvent);
    }

    {
      ChangeMacroMode MacroMode(MACRO_MAINMENU);
      HOptMenu.Process();
    }
    HOptMenu.GetExitCode(HItem,VItem);
  }

  // "Исполнятор команд меню"
  switch(HItem)
  {
    // *** Left
    case 0:
    {
      if (VItem>=0 && VItem<=9) // Режимы левой панели
      {
        CtrlObject->Cp()->ChangePanelToFilled(CtrlObject->Cp()->LeftPanel,FILE_PANEL);
        int NewViewMode=VItem==9 ? VIEW_0:VIEW_1+VItem;
        CtrlObject->Cp()->LeftPanel->SetViewMode(NewViewMode);
      }
      else
      {
        switch(VItem)
        {
          case 11: // Info panel
            CtrlObject->Cp()->ChangePanelToFilled(CtrlObject->Cp()->LeftPanel,INFO_PANEL);
            break;
          case 12: // Tree panel
            CtrlObject->Cp()->ChangePanelToFilled(CtrlObject->Cp()->LeftPanel,TREE_PANEL);
            break;
          case 13: // Quick view
            CtrlObject->Cp()->ChangePanelToFilled(CtrlObject->Cp()->LeftPanel,QVIEW_PANEL);
            break;
          case 15: // Sort modes
            CtrlObject->Cp()->LeftPanel->ProcessKey(KEY_CTRLF12);
            break;
          case 16: // Show long names
            CtrlObject->Cp()->LeftPanel->ProcessKey(KEY_CTRLN);
            break;
          case 17: // Panel On/Off
            FrameManager->ProcessKey(KEY_CTRLF1);
            break;
          case 18: // Re-read
            CtrlObject->Cp()->LeftPanel->ProcessKey(KEY_CTRLR);
            break;
          case 19: // Change drive
            CtrlObject->Cp()->LeftPanel->ChangeDisk();
            break;
        }
      }
      break;
    }

    // *** Files
    case 1:
    {
      switch(VItem)
      {
        case 0:  // View
          FrameManager->ProcessKey(KEY_F3);
          break;
        case 1:  // Edit
          FrameManager->ProcessKey(KEY_F4);
          break;
        case 2:  // Copy
          FrameManager->ProcessKey(KEY_F5);
          break;
        case 3:  // Rename or move
          FrameManager->ProcessKey(KEY_F6);
          break;
        case 4:  // Make folder
          FrameManager->ProcessKey(KEY_F7);
          break;
        case 5:  // Delete
          FrameManager->ProcessKey(KEY_F8);
          break;
        case 6:  // Wipe
          FrameManager->ProcessKey(KEY_ALTDEL);
          break;
        case 8:  // Add to archive
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_SHIFTF1);
          break;
        case 9:  // Extract files
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_SHIFTF2);
          break;
        case 10:  // Archive commands
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_SHIFTF3);
          break;
        case 12: // File attributes
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_CTRLA);
          break;
        case 13: // Apply command
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_CTRLG);
          break;
        case 14: // Describe files
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_CTRLZ);
          break;
        case 16: // Select group
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_ADD);
          break;
        case 17: // Unselect group
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_SUBTRACT);
          break;
        case 18: // Invert selection
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_MULTIPLY);
          break;
        case 19: // Restore selection
          CtrlObject->Cp()->ActivePanel->RestoreSelection();
          break;
      }
      break;
    }

    // *** Commands
    case 2:
    {
      switch(VItem)
      {
        case 0: // Find file
          FrameManager->ProcessKey(KEY_ALTF7);
          break;
        case 1: // History
          FrameManager->ProcessKey(KEY_ALTF8);
          break;
        case 2: // Video mode
          FrameManager->ProcessKey(KEY_ALTF9);
          break;
        case 3: // Find folder
          FrameManager->ProcessKey(KEY_ALTF10);
          break;
        case 4: // File view history
          FrameManager->ProcessKey(KEY_ALTF11);
          break;
        case 5: // Folders history
          FrameManager->ProcessKey(KEY_ALTF12);
          break;
        case 7: // Swap panels
          FrameManager->ProcessKey(KEY_CTRLU);
          break;
        case 8: // Panels On/Off
          FrameManager->ProcessKey(KEY_CTRLO);
          break;
        case 9: // Compare folders
          CtrlObject->Cp()->ActivePanel->CompareDir();
          break;
        case 11: // Edit user menu
          ProcessUserMenu(1);
          break;
        case 12: // File associations
          EditFileTypes();
          break;
        case 13: // Folder shortcuts
          ShowFolderShortcut();
          break;
        case 14: // File panel filter
          CtrlObject->Cp()->ActivePanel->EditFilter();
          break;
        case 16: // Plugin commands
          FrameManager->ProcessKey(KEY_F11);
          break;
        case 17: // Screens list
          FrameManager->ProcessKey(KEY_F12);
          break;
        case 18: // Task list
          ShowProcessList();
          break;
        case 19: // HotPlug list
          ShowHotplugDevice();
          break;
      }
      break;
    }

    // *** Options
    case 3:
    {
      switch(VItem)
      {
        case 0:   // System settings
          SystemSettings();
          break;
        case 1:   // Panel settings
          PanelSettings();
          break;
        case 2:   // Interface settings
          InterfaceSettings();
          break;
        case 3:   // Languages
          {
            VMenu *LangMenu,*HelpMenu;
            if (Language::Select(FALSE,&LangMenu))
            {
              Lang.Close();
              if (!Lang.Init(FarPath,MListEval))
              {
                Message(MSG_WARNING,1,"Error","Cannot load language data","Ok");
                exit(0);
              }
              Language::Select(TRUE,&HelpMenu);
              delete HelpMenu;
              LangMenu->Hide();
              CtrlObject->Plugins.ReloadLanguage();
              SetEnvironmentVariable("FARLANG",Opt.Language);
              PrepareStrFTime();
              __PrepareKMGTbStr();
              FrameManager->InitKeyBar();
              CtrlObject->Cp()->RedrawKeyBar();
              CtrlObject->Cp()->SetScreenPosition();
            }
            delete LangMenu;
          }
          break;
        case 4:   // Plugins configuration
          CtrlObject->Plugins.Configure();
          break;
        case 5:   // Dialog settings (police=5)
          DialogSettings();
          break;
        case 7:   // Confirmations
          SetConfirmations();
          break;
        case 8:   // File panel modes
          FileList::SetFilePanelModes();
          break;
        case 9:   // File descriptions
          SetDizConfig();
          break;
        case 10:   // Folder description files
          SetFolderInfoFiles();
          break;
        case 12:  // Viewer settings
          ViewerConfig(Opt.ViOpt);
          break;
        case 13:  // Editor settings
          EditorConfig(Opt.EdOpt);
          break;
        case 15:  // Colors
          SetColors();
          break;
        case 16:  // Files highlighting
          CtrlObject->HiFiles->HiEdit(0);
          break;
        case 18:  // Save setup
          SaveConfig(1);
          break;
      }
      break;
    }

    // *** Right
    case 4:
    {
      if (VItem>=0 && VItem<=9) // Режимы правой панели
      {
        CtrlObject->Cp()->ChangePanelToFilled(CtrlObject->Cp()->RightPanel,FILE_PANEL);
        int NewViewMode=VItem==9 ? VIEW_0:VIEW_1+VItem;
        CtrlObject->Cp()->RightPanel->SetViewMode(NewViewMode);
      }
      else
        switch(VItem)
        {
          case 11: // Info panel
            CtrlObject->Cp()->ChangePanelToFilled(CtrlObject->Cp()->RightPanel,INFO_PANEL);
            break;
          case 12: // Tree panel
            CtrlObject->Cp()->ChangePanelToFilled(CtrlObject->Cp()->RightPanel,TREE_PANEL);
            break;
          case 13: // Quick view
            CtrlObject->Cp()->ChangePanelToFilled(CtrlObject->Cp()->RightPanel,QVIEW_PANEL);
            break;
          case 15: // Sort modes
            CtrlObject->Cp()->RightPanel->ProcessKey(KEY_CTRLF12);
            break;
          case 16: // Show long names
            CtrlObject->Cp()->RightPanel->ProcessKey(KEY_CTRLN);
            break;
          case 17: // Panel On/Off
            FrameManager->ProcessKey(KEY_CTRLF2);
            break;
          case 18: // Re-read
            CtrlObject->Cp()->RightPanel->ProcessKey(KEY_CTRLR);
            break;
          case 19: // Change drive
            CtrlObject->Cp()->RightPanel->ChangeDisk();
            break;
        }
      break;
    }
  }

  int _CurrentFrame=FrameManager->GetCurrentFrame()->GetType();
  // TODO:Здесь как то нужно изменить, чтобы учесть будущие новые типы полноэкранных фреймов
  //      или то, что, скажем редактор/вьювер может быть не полноэкранным
  if(!(_CurrentFrame == MODALTYPE_VIEWER || _CurrentFrame == MODALTYPE_EDITOR))
    CtrlObject->CmdLine->Show();

  if (HItem!=-1 && VItem!=-1)
  {
    LastHItem=HItem;
    LastVItem=VItem;
  }
}
