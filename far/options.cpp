/*
options.cpp

Фаровское горизонтальное меню (вызов hmenu.cpp с конкретными параметрами)

*/

/* Revision: 1.17 18.03.2002 $ */

/*
Modify:
  18.03.2002 SVS
    ! После смены языка интерфейса апдейтим так же титлы истории редактирования
  11.02.2002 SVS
    + Добавка в меню - акселератор - решение BugZ#299
  24.01.2002 SVS
    ! Косметика. Наконец то руки добрались до "красоты" :-)
  19.07.2001 OT
    ! Замена CtrlObject->Cp()->ProcessKey на FrameManager->ProcessKey
  19.07.2001 SVS
    - Не работала смена видеорежима из меню
  11.07.2001 SVS
    + переменные среды: FARLANG
  22.06.2001 SVS
    ! Позаботимся о StrFTime :-)
  14.06.2001 OT
    ! "Бунт" ;-)
  21.05.2001 SVS
    ! struct MenuData|MenuItem
      Поля Selected, Checked, Separator и Disabled преобразованы в DWORD Flags
    ! OptionsDisabled убрана. Пункты дизаблятся перед показом.
  12.05.2001 DJ
    * нажатие Shift-F10 перед тем, как нажималось F9, показывает меню для
      активной панели, а не всегда для левой
  06.05.2001 DJ
    ! перетрях #include
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  30.03.2001 SVS
    ! В OptionsDisabled() задействуем Opt.Policies.DisabledOptions, которая
      централизовано и успешно считывается в config.cpp
  29.03.2001 IS
    + ViewerConfig вызывается с Opt.ViOpt
  28.02.2001 IS
    ! "CtrlObject->CmdLine." -> "CtrlObject->CmdLine->"
  21.02.2001 IS
    + EditorConfig вызывается с Opt.EdOpt
  05.09.2000 tran
    + OptionsEnabled - reg:Policies/DisabledOptions
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
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
#include "grpsort.hpp"
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
    (char *)MMenuFindFile,LIF_SELECTED,KEY_ALTF7,
    (char *)MMenuHistory,0,KEY_ALTF8,
    (char *)MMenuVideoMode,0,KEY_ALTF9,
    (char *)MMenuFindFolder,0,KEY_ALTF10,
    (char *)MMenuViewHistory,0,KEY_ALTF11,
    (char *)MMenuFoldersHistory,0,KEY_ALTF12,
    "",LIF_SEPARATOR,0,
    (char *)MMenuSwapPanels,0,KEY_CTRLU,
    (char *)MMenuTogglePanels,0,KEY_CTRLO,
    (char *)MMenuCompareFolders,0,0,
    "",LIF_SEPARATOR,0,
    (char *)MMenuUserMenu,0,0,
    (char *)MMenuFileAssociations,0,0,
    (char *)MMenuFolderShortcuts,0,0,
    (char *)MMenuEditSortGroups,0,0,
    (char *)MMenuFilter,0,KEY_CTRLI,
    "",LIF_SEPARATOR,0,
    (char *)MMenuPluginCommands,0,KEY_F11,
    (char *)MMenuWindowsList,0,KEY_F12,
    (char *)MMenuProcessList,0,KEY_CTRLW,
  };


  struct MenuData OptionsMenu[]=
  {
    (char *)MMenuSystemSettings,LIF_SELECTED,0,
    (char *)MMenuPanelSettings,0,0,
    (char *)MMenuInterface,0,0,
    (char *)MMenuLanguages,0,0,
    (char *)MMenuPluginsConfig,0,0,
    "",LIF_SEPARATOR,0,
    (char *)MMenuConfirmation,0,0,
    (char *)MMenuFilePanelModes,0,0,
    (char *)MMenuFileDescriptions,0,0,
    (char *)MMenuFolderInfoFiles,0,0,
    "",LIF_SEPARATOR,0,
    (char *)MMenuViewer,0,0,
    (char *)MMenuEditor,0,0,
    "",LIF_SEPARATOR,0,
    (char *)MMenuColors,0,0,
    (char *)MMenuFilesHighlighting,0,0,
    "",LIF_SEPARATOR,0,
    (char *)MMenuSaveSetup,0,KEY_SHIFTF9,
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
  if (Opt.Policies.DisabledOptions)
    for(I=0; I < sizeof(OptionsMenu)/sizeof(OptionsMenu[0]); ++I)
      OptionsMenu[I].SetDisable((Opt.Policies.DisabledOptions >> I) & 1);

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
      ChangeMacroMode MacroMode(MACRO_MAINMENU);
      HOptMenu.ProcessKey(KEY_DOWN);
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
      HOptMenu.Show();
      HOptMenu.ProcessMouse(MouseEvent);
    }
    ChangeMacroMode MacroMode(MACRO_MAINMENU);
    HOptMenu.Process();
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
        case 7:  // Add to archive
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_SHIFTF1);
          break;
        case 8:  // Extract files
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_SHIFTF2);
          break;
        case 9:  // Archive commands
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_SHIFTF3);
          break;
        case 11: // File attributes
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_CTRLA);
          break;
        case 12: // Apply command
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_CTRLG);
          break;
        case 13: // Describe files
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_CTRLZ);
          break;
        case 15: // Select group
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_ADD);
          break;
        case 16: // Unselect group
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_SUBTRACT);
          break;
        case 17: // Invert selection
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_MULTIPLY);
          break;
        case 18: // Restore selection
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
          EditFileTypes(0);
          break;
        case 13: // Folder shortcuts
          ShowFolderShortcut();
          break;
        case 14: // Edit sort groups
          CtrlObject->GrpSort->EditGroups();
          break;
        case 15: // File panel filter
          CtrlObject->Cp()->ActivePanel->EditFilter();
          break;
        case 17: // Plugin commands
          FrameManager->ProcessKey(KEY_F11);
          break;
        case 18: // Screens list
          FrameManager->ProcessKey(KEY_F12);
          break;
        case 19: // Task list
          ShowProcessList();
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
              CtrlObject->ViewHistory->ReloadTitle();
              SetEnvironmentVariable("FARLANG",Opt.Language);
              PrepareStrFTime();
              CtrlObject->Cp()->RedrawKeyBar();
              CtrlObject->Cp()->SetScreenPosition();
            }
            delete LangMenu;
          }
          break;
        case 4:   // Plugins configuration
          CtrlObject->Plugins.Configure();
          break;
        case 6:   // Confirmations
          SetConfirmations();
          break;
        case 7:   // File panel modes
          FileList::SetFilePanelModes();
          break;
        case 8:   // File descriptions
          SetDizConfig();
          break;
        case 9:   // Folder description files
          SetFolderInfoFiles();
          break;
        case 11:  // Viewer settings
          ViewerConfig(Opt.ViOpt);
          break;
        case 12:  // Editor settings
          EditorConfig(Opt.EdOpt);
          break;
        case 14:  // Colors
          SetColors();
          break;
        case 15:  // Files highlighting
          CtrlObject->HiFiles->HiEdit(0);
          break;
        case 17:  // Save setup
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

  CtrlObject->CmdLine->Show();

  if (HItem!=-1 && VItem!=-1)
  {
    LastHItem=HItem;
    LastVItem=VItem;
  }
}
