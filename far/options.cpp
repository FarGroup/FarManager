/*
options.cpp

Фаровское горизонтальное меню (вызов hmenu.cpp с конкретными параметрами)

*/

/* Revision: 1.09 21.05.2001 $ */

/*
Modify:
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

void ShellOptions(int LastCommand,MOUSE_EVENT_RECORD *MouseEvent)
{
  int I;
  struct MenuData LeftMenu[]=
  {
    (char *)MMenuBriefView,LIF_SELECTED,
    (char *)MMenuMediumView,0,
    (char *)MMenuFullView,0,
    (char *)MMenuWideView,0,
    (char *)MMenuDetailedView,0,
    (char *)MMenuDizView,0,
    (char *)MMenuLongDizView,0,
    (char *)MMenuOwnersView,0,
    (char *)MMenuLinksView,0,
    (char *)MMenuAlternativeView,0,
    "",LIF_SEPARATOR,
    (char *)MMenuInfoPanel,0,
    (char *)MMenuTreePanel,0,
    (char *)MMenuQuickView,0,
    "",LIF_SEPARATOR,
    (char *)MMenuSortModes,0,
    (char *)MMenuLongNames,0,
    (char *)MMenuTogglePanel,0,
    (char *)MMenuReread,0,
    (char *)MMenuChangeDrive,0
  };

  struct MenuData FilesMenu[]=
  {
    (char *)MMenuView,LIF_SELECTED,
    (char *)MMenuEdit,0,
    (char *)MMenuCopy,0,
    (char *)MMenuMove,0,
    (char *)MMenuCreateFolder,0,
    (char *)MMenuDelete,0,
    "",LIF_SEPARATOR,
    (char *)MMenuAdd,0,
    (char *)MMenuExtract,0,
    (char *)MMenuArchiveCommands,0,
    "",LIF_SEPARATOR,
    (char *)MMenuAttributes,0,
    (char *)MMenuApplyCommand,0,
    (char *)MMenuDescribe,0,
    "",LIF_SEPARATOR,
    (char *)MMenuSelectGroup,0,
    (char *)MMenuUnselectGroup,0,
    (char *)MMenuInvertSelection,0,
    (char *)MMenuRestoreSelection,0
  };


  struct MenuData CmdMenu[]=
  {
    (char *)MMenuFindFile,LIF_SELECTED,
    (char *)MMenuHistory,0,
    (char *)MMenuVideoMode,0,
    (char *)MMenuFindFolder,0,
    (char *)MMenuViewHistory,0,
    (char *)MMenuFoldersHistory,0,
    "",LIF_SEPARATOR,
    (char *)MMenuSwapPanels,0,
    (char *)MMenuTogglePanels,0,
    (char *)MMenuCompareFolders,0,
    "",LIF_SEPARATOR,
    (char *)MMenuUserMenu,0,
    (char *)MMenuFileAssociations,0,
    (char *)MMenuFolderShortcuts,0,
    (char *)MMenuEditSortGroups,0,
    (char *)MMenuFilter,0,
    "",LIF_SEPARATOR,
    (char *)MMenuPluginCommands,0,
    (char *)MMenuWindowsList,0,
    (char *)MMenuProcessList,0
  };


  struct MenuData OptionsMenu[]=
  {
    (char *)MMenuSystemSettings,LIF_SELECTED,
    (char *)MMenuPanelSettings,0,
    (char *)MMenuInterface,0,
    (char *)MMenuLanguages,0,
    (char *)MMenuPluginsConfig,0,
    "",LIF_SEPARATOR,
    (char *)MMenuConfirmation,0,
    (char *)MMenuFilePanelModes,0,
    (char *)MMenuFileDescriptions,0,
    (char *)MMenuFolderInfoFiles,0,
    "",LIF_SEPARATOR,
    (char *)MMenuViewer,0,
    (char *)MMenuEditor,0,
    "",LIF_SEPARATOR,
    (char *)MMenuColors,0,
    (char *)MMenuFilesHighlighting,0,
    "",LIF_SEPARATOR,
    (char *)MMenuSaveSetup,0
  };


  struct MenuData RightMenu[]=
  {
    (char *)MMenuBriefView,LIF_SELECTED,
    (char *)MMenuMediumView,0,
    (char *)MMenuFullView,0,
    (char *)MMenuWideView,0,
    (char *)MMenuDetailedView,0,
    (char *)MMenuDizView,0,
    (char *)MMenuLongDizView,0,
    (char *)MMenuOwnersView,0,
    (char *)MMenuLinksView,0,
    (char *)MMenuAlternativeView,0,
    "",LIF_SEPARATOR,
    (char *)MMenuInfoPanel,0,
    (char *)MMenuTreePanel,0,
    (char *)MMenuQuickView,0,
    "",LIF_SEPARATOR,
    (char *)MMenuSortModes,0,
    (char *)MMenuLongNames,0,
    (char *)MMenuTogglePanelRight,0,
    (char *)MMenuReread,0,
    (char *)MMenuChangeDriveRight,0
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

  switch(HItem)
  {
    case 0:
      if (VItem>=0 && VItem<=9)
      {
        CtrlObject->Cp()->ChangePanelToFilled(CtrlObject->Cp()->LeftPanel,FILE_PANEL);
        int NewViewMode=VItem==9 ? VIEW_0:VIEW_1+VItem;
        CtrlObject->Cp()->LeftPanel->SetViewMode(NewViewMode);
      }
      else
        switch(VItem)
        {
          case 11:
            CtrlObject->Cp()->ChangePanelToFilled(CtrlObject->Cp()->LeftPanel,INFO_PANEL);
            break;
          case 12:
            CtrlObject->Cp()->ChangePanelToFilled(CtrlObject->Cp()->LeftPanel,TREE_PANEL);
            break;
          case 13:
            CtrlObject->Cp()->ChangePanelToFilled(CtrlObject->Cp()->LeftPanel,QVIEW_PANEL);
            break;
          case 15:
            CtrlObject->Cp()->LeftPanel->ProcessKey(KEY_CTRLF12);
            break;
          case 16:
            CtrlObject->Cp()->LeftPanel->ProcessKey(KEY_CTRLN);
            break;
          case 17:
            CtrlObject->Cp()->ProcessKey(KEY_CTRLF1);
            break;
          case 18:
            CtrlObject->Cp()->LeftPanel->ProcessKey(KEY_CTRLR);
            break;
          case 19:
            CtrlObject->Cp()->LeftPanel->ChangeDisk();
            break;
        }
      break;
    case 1:
      switch(VItem)
      {
        case 0:
          CtrlObject->Cp()->ProcessKey(KEY_F3);
          break;
        case 1:
          CtrlObject->Cp()->ProcessKey(KEY_F4);
          break;
        case 2:
          CtrlObject->Cp()->ProcessKey(KEY_F5);
          break;
        case 3:
          CtrlObject->Cp()->ProcessKey(KEY_F6);
          break;
        case 4:
          CtrlObject->Cp()->ProcessKey(KEY_F7);
          break;
        case 5:
          CtrlObject->Cp()->ProcessKey(KEY_F8);
          break;
        case 7:
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_SHIFTF1);
          break;
        case 8:
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_SHIFTF2);
          break;
        case 9:
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_SHIFTF3);
          break;
        case 11:
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_CTRLA);
          break;
        case 12:
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_CTRLG);
          break;
        case 13:
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_CTRLZ);
          break;
        case 15:
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_ADD);
          break;
        case 16:
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_SUBTRACT);
          break;
        case 17:
          CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_MULTIPLY);
          break;
        case 18:
          CtrlObject->Cp()->ActivePanel->RestoreSelection();
          break;
      }
      break;
    case 2:
      switch(VItem)
      {
        case 0:
          CtrlObject->Cp()->ProcessKey(KEY_ALTF7);
          break;
        case 1:
          CtrlObject->Cp()->ProcessKey(KEY_ALTF8);
          break;
        case 2:
          CtrlObject->Cp()->ProcessKey(KEY_ALTF9);
          break;
        case 3:
          CtrlObject->Cp()->ProcessKey(KEY_ALTF10);
          break;
        case 4:
          CtrlObject->Cp()->ProcessKey(KEY_ALTF11);
          break;
        case 5:
          CtrlObject->Cp()->ProcessKey(KEY_ALTF12);
          break;
        case 7:
          CtrlObject->Cp()->ProcessKey(KEY_CTRLU);
          break;
        case 8:
          CtrlObject->Cp()->ProcessKey(KEY_CTRLO);
          break;
        case 9:
          CtrlObject->Cp()->ActivePanel->CompareDir();
          break;
        case 11:
          ProcessUserMenu(1);
          break;
        case 12:
          EditFileTypes(0);
          break;
        case 13:
          ShowFolderShortcut();
          break;
        case 14:
          CtrlObject->GrpSort->EditGroups();
          break;
        case 15:
          CtrlObject->Cp()->ActivePanel->EditFilter();
          break;
        case 17:
          FrameManager->ProcessKey(KEY_F11);
          break;
        case 18:
          FrameManager->ProcessKey(KEY_F12);
          break;
        case 19:
          ShowProcessList();
          break;
      }
      break;
    case 3:
      switch(VItem)
      {
        case 0:
          SystemSettings();
          break;
        case 1:
          PanelSettings();
          break;
        case 2:
          InterfaceSettings();
          break;
        case 3:
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
              CtrlObject->Cp()->RedrawKeyBar();
              CtrlObject->Cp()->SetScreenPositions();
            }
            delete LangMenu;
          }
          break;
        case 4:
          CtrlObject->Plugins.Configure();
          break;
        case 6:
          SetConfirmations();
          break;
        case 7:
          FileList::SetFilePanelModes();
          break;
        case 8:
          SetDizConfig();
          break;
        case 9:
          SetFolderInfoFiles();
          break;
        case 11:
          ViewerConfig(Opt.ViOpt);
          break;
        case 12:
          EditorConfig(Opt.EdOpt);
          break;
        case 14:
          SetColors();
          break;
        case 15:
          CtrlObject->HiFiles->HiEdit(0);
          break;
        case 17:
          SaveConfig(1);
          break;
      }
      break;
    case 4:
      if (VItem>=0 && VItem<=9)
      {
        CtrlObject->Cp()->ChangePanelToFilled(CtrlObject->Cp()->RightPanel,FILE_PANEL);
        int NewViewMode=VItem==9 ? VIEW_0:VIEW_1+VItem;
        CtrlObject->Cp()->RightPanel->SetViewMode(NewViewMode);
      }
      else
        switch(VItem)
        {
          case 11:
            CtrlObject->Cp()->ChangePanelToFilled(CtrlObject->Cp()->RightPanel,INFO_PANEL);
            break;
          case 12:
            CtrlObject->Cp()->ChangePanelToFilled(CtrlObject->Cp()->RightPanel,TREE_PANEL);
            break;
          case 13:
            CtrlObject->Cp()->ChangePanelToFilled(CtrlObject->Cp()->RightPanel,QVIEW_PANEL);
            break;
          case 15:
            CtrlObject->Cp()->RightPanel->ProcessKey(KEY_CTRLF12);
            break;
          case 16:
            CtrlObject->Cp()->RightPanel->ProcessKey(KEY_CTRLN);
            break;
          case 17:
            CtrlObject->Cp()->ProcessKey(KEY_CTRLF2);
            break;
          case 18:
            CtrlObject->Cp()->RightPanel->ProcessKey(KEY_CTRLR);
            break;
          case 19:
            CtrlObject->Cp()->RightPanel->ChangeDisk();
            break;
        }
      break;
  }
  CtrlObject->CmdLine->Show();
  if (HItem!=-1 && VItem!=-1)
  {
    LastHItem=HItem;
    LastVItem=VItem;
  }
}
