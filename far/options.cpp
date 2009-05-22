/*
options.cpp

Фаровское горизонтальное меню (вызов hmenu.cpp с конкретными параметрами)
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "lang.hpp"
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
#include "imports.hpp"
#include "message.hpp"
#include "hotplug.hpp"
#include "config.hpp"

void ShellOptions(int LastCommand,MOUSE_EVENT_RECORD *MouseEvent)
{
  struct MenuDataEx LeftMenu[]=
  {
    (const wchar_t *)MMenuBriefView,LIF_SELECTED,KEY_CTRL1,
    (const wchar_t *)MMenuMediumView,0,KEY_CTRL2,
    (const wchar_t *)MMenuFullView,0,KEY_CTRL3,
    (const wchar_t *)MMenuWideView,0,KEY_CTRL4,
    (const wchar_t *)MMenuDetailedView,0,KEY_CTRL5,
    (const wchar_t *)MMenuDizView,0,KEY_CTRL6,
    (const wchar_t *)MMenuLongDizView,0,KEY_CTRL7,
    (const wchar_t *)MMenuOwnersView,0,KEY_CTRL8,
    (const wchar_t *)MMenuLinksView,0,KEY_CTRL9,
    (const wchar_t *)MMenuAlternativeView,0,KEY_CTRL0,
    L"",LIF_SEPARATOR,0,
    (const wchar_t *)MMenuInfoPanel,0,KEY_CTRLL,
    (const wchar_t *)MMenuTreePanel,0,KEY_CTRLT,
    (const wchar_t *)MMenuQuickView,0,KEY_CTRLQ,
    L"",LIF_SEPARATOR,0,
    (const wchar_t *)MMenuSortModes,0,KEY_CTRLF12,
    (const wchar_t *)MMenuLongNames,0,KEY_CTRLN,
    (const wchar_t *)MMenuTogglePanel,0,KEY_CTRLF1,
    (const wchar_t *)MMenuReread,0,KEY_CTRLR,
    (const wchar_t *)MMenuChangeDrive,0,KEY_ALTF1,
  };

  struct MenuDataEx FilesMenu[]=
  {
    (const wchar_t *)MMenuView,LIF_SELECTED,KEY_F3,
    (const wchar_t *)MMenuEdit,0,KEY_F4,
    (const wchar_t *)MMenuCopy,0,KEY_F5,
    (const wchar_t *)MMenuMove,0,KEY_F6,
    (const wchar_t *)MMenuCreateFolder,0,KEY_F7,
    (const wchar_t *)MMenuDelete,0,KEY_F8,
    (const wchar_t *)MMenuWipe,0,KEY_ALTDEL,
    L"",LIF_SEPARATOR,0,
    (const wchar_t *)MMenuAdd,0,KEY_SHIFTF1,
    (const wchar_t *)MMenuExtract,0,KEY_SHIFTF2,
    (const wchar_t *)MMenuArchiveCommands,0,KEY_SHIFTF3,
    L"",LIF_SEPARATOR,0,
    (const wchar_t *)MMenuAttributes,0,KEY_CTRLA,
    (const wchar_t *)MMenuApplyCommand,0,KEY_CTRLG,
    (const wchar_t *)MMenuDescribe,0,KEY_CTRLZ,
    L"",LIF_SEPARATOR,0,
    (const wchar_t *)MMenuSelectGroup,0,KEY_ADD,
    (const wchar_t *)MMenuUnselectGroup,0,KEY_SUBTRACT,
    (const wchar_t *)MMenuInvertSelection,0,KEY_MULTIPLY,
    (const wchar_t *)MMenuRestoreSelection,0,KEY_CTRLM,
  };


  struct MenuDataEx CmdMenu[]=
  {
  /* 00 */(const wchar_t *)MMenuFindFile,LIF_SELECTED,KEY_ALTF7,
  /* 01 */(const wchar_t *)MMenuHistory,0,KEY_ALTF8,
  /* 02 */(const wchar_t *)MMenuVideoMode,0,KEY_ALTF9,
  /* 03 */(const wchar_t *)MMenuFindFolder,0,KEY_ALTF10,
  /* 04 */(const wchar_t *)MMenuViewHistory,0,KEY_ALTF11,
  /* 05 */(const wchar_t *)MMenuFoldersHistory,0,KEY_ALTF12,
  /* 06 */L"",LIF_SEPARATOR,0,
  /* 07 */(const wchar_t *)MMenuSwapPanels,0,KEY_CTRLU,
  /* 08 */(const wchar_t *)MMenuTogglePanels,0,KEY_CTRLO,
  /* 09 */(const wchar_t *)MMenuCompareFolders,0,0,
  /* 10 */L"",LIF_SEPARATOR,0,
  /* 11 */(const wchar_t *)MMenuUserMenu,0,0,
  /* 12 */(const wchar_t *)MMenuFileAssociations,0,0,
  /* 13 */(const wchar_t *)MMenuFolderShortcuts,0,0,
  /* 14 */(const wchar_t *)MMenuFilter,0,KEY_CTRLI,
  /* 15 */L"",LIF_SEPARATOR,0,
  /* 16 */(const wchar_t *)MMenuPluginCommands,0,KEY_F11,
  /* 17 */(const wchar_t *)MMenuWindowsList,0,KEY_F12,
  /* 18 */(const wchar_t *)MMenuProcessList,0,KEY_CTRLW,
  /* 19 */(const wchar_t *)MMenuHotPlugList,0,0,
  };


  struct MenuDataEx OptionsMenu[]=
  {
   /* 00 */(const wchar_t *)MMenuSystemSettings,LIF_SELECTED,0,
   /* 01 */(const wchar_t *)MMenuPanelSettings,0,0,
   /* 02 */(const wchar_t *)MMenuInterface,0,0,
   /* 03 */(const wchar_t *)MMenuLanguages,0,0,
   /* 04 */(const wchar_t *)MMenuPluginsConfig,0,0,
   /* 05 */(const wchar_t *)MMenuDialogSettings,0,0,
   /* 06 */L"",LIF_SEPARATOR,0,
   /* 07 */(const wchar_t *)MMenuConfirmation,0,0,
   /* 08 */(const wchar_t *)MMenuFilePanelModes,0,0,
   /* 09 */(const wchar_t *)MMenuFileDescriptions,0,0,
   /* 10 */(const wchar_t *)MMenuFolderInfoFiles,0,0,
   /* 11 */L"",LIF_SEPARATOR,0,
   /* 12 */(const wchar_t *)MMenuViewer,0,0,
   /* 13 */(const wchar_t *)MMenuEditor,0,0,
   /* 14 */L"",LIF_SEPARATOR,0,
   /* 15 */(const wchar_t *)MMenuColors,0,0,
   /* 16 */(const wchar_t *)MMenuFilesHighlighting,0,0,
   /* 17 */L"",LIF_SEPARATOR,0,
   /* 18 */(const wchar_t *)MMenuSaveSetup,0,KEY_SHIFTF9,
  };


  struct MenuDataEx RightMenu[]=
  {
    (const wchar_t *)MMenuBriefView,LIF_SELECTED,KEY_CTRL1,
    (const wchar_t *)MMenuMediumView,0,KEY_CTRL2,
    (const wchar_t *)MMenuFullView,0,KEY_CTRL3,
    (const wchar_t *)MMenuWideView,0,KEY_CTRL4,
    (const wchar_t *)MMenuDetailedView,0,KEY_CTRL5,
    (const wchar_t *)MMenuDizView,0,KEY_CTRL6,
    (const wchar_t *)MMenuLongDizView,0,KEY_CTRL7,
    (const wchar_t *)MMenuOwnersView,0,KEY_CTRL8,
    (const wchar_t *)MMenuLinksView,0,KEY_CTRL9,
    (const wchar_t *)MMenuAlternativeView,0,KEY_CTRL0,
    L"",LIF_SEPARATOR,0,
    (const wchar_t *)MMenuInfoPanel,0,KEY_CTRLL,
    (const wchar_t *)MMenuTreePanel,0,KEY_CTRLT,
    (const wchar_t *)MMenuQuickView,0,KEY_CTRLQ,
    L"",LIF_SEPARATOR,0,
    (const wchar_t *)MMenuSortModes,0,KEY_CTRLF12,
    (const wchar_t *)MMenuLongNames,0,KEY_CTRLN,
    (const wchar_t *)MMenuTogglePanelRight,0,KEY_CTRLF2,
    (const wchar_t *)MMenuReread,0,KEY_CTRLR,
    (const wchar_t *)MMenuChangeDriveRight,0,KEY_ALTF2,
  };


  struct HMenuData MainMenu[]=
  {
		MSG(MMenuLeftTitle),1,LeftMenu,countof(LeftMenu),L"LeftRightMenu",
		MSG(MMenuFilesTitle),0,FilesMenu,countof(FilesMenu),L"FilesMenu",
		MSG(MMenuCommandsTitle),0,CmdMenu,countof(CmdMenu),L"CmdMenu",
		MSG(MMenuOptionsTitle),0,OptionsMenu,countof(OptionsMenu),L"OptMenu",
		MSG(MMenuRightTitle),0,RightMenu,countof(RightMenu),L"LeftRightMenu"
  };

  static int LastHItem=-1,LastVItem=0;
  int HItem,VItem;

  // дисаблим
  CmdMenu[19].SetDisable(!ifn.bSetupAPIFunctions);

  if (Opt.Policies.DisabledOptions)
    for(size_t I=0; I < countof(OptionsMenu); ++I)
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
        {
          if (MenuLine==0)
            LeftMenu[9].SetCheck(1);
          else
            LeftMenu[MenuLine-1].SetCheck(1);
        }
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
        {
          if (MenuLine==0)
            RightMenu[9].SetCheck(1);
          else
            RightMenu[MenuLine-1].SetCheck(1);
        }
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
		HMenu HOptMenu(MainMenu,countof(MainMenu));
    HOptMenu.SetHelp(L"Menus");
    HOptMenu.SetPosition(0,0,ScrX,0);
    if (LastCommand)
    {
      MenuDataEx *VMenuTable[]={LeftMenu,FilesMenu,CmdMenu,OptionsMenu,RightMenu};

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
              if (!Lang.Init(g_strFarPath,true,MNewFileName))
              {
                Message(MSG_WARNING,1,L"Error",L"Cannot load language data",L"Ok");
                exit(0);
              }
              Language::Select(TRUE,&HelpMenu);
              delete HelpMenu;
              LangMenu->Hide();
              CtrlObject->Plugins.ReloadLanguage();
              SetEnvironmentVariableW(L"FARLANG",Opt.strLanguage);
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
