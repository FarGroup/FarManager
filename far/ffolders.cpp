/*
ffolders.cpp

Folder shortcuts

*/

/* Revision: 1.04 06.05.2001 $ */

/*
Modify:
  06.05.2001 DJ
    ! перетрях #include
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  28.02.2001 IS
    ! "CtrlObject->CmdLine." -> "CtrlObject->CmdLine->"
  11.02.2001 SVS
    ! Несколько уточнений кода в связи с изменениями в структуре MenuItem
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"
#include "keys.hpp"
#include "global.hpp"
#include "lang.hpp"
#include "vmenu.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"

static int ShowFolderShortcutMenu(int Pos);

int GetShortcutFolder(int Key,char *DestFolder,char *PluginModule,
                      char *PluginFile,char *PluginData)
{
  if (Key<KEY_RCTRL0 || Key>KEY_RCTRL9)
    return(FALSE);
  char ValueName[100],Folder[NM];
  sprintf(ValueName,"Shortcut%d",Key-KEY_RCTRL0);
  GetRegKey("FolderShortcuts",ValueName,Folder,"",sizeof(Folder));
  ExpandEnvironmentStrings(Folder,DestFolder,NM);
  sprintf(ValueName,"PluginModule%d",Key-KEY_RCTRL0);
  GetRegKey("FolderShortcuts",ValueName,PluginModule,"",NM);
  sprintf(ValueName,"PluginFile%d",Key-KEY_RCTRL0);
  GetRegKey("FolderShortcuts",ValueName,PluginFile,"",NM);
  sprintf(ValueName,"PluginData%d",Key-KEY_RCTRL0);
  GetRegKey("FolderShortcuts",ValueName,PluginData,"",NM);

  return(*DestFolder || *PluginModule);
}


int SaveFolderShortcut(int Key,char *SrcFolder,char *PluginModule,
                       char *PluginFile,char *PluginData)
{
  char ValueName[100];
  if (Key<KEY_CTRLSHIFT0 || Key>KEY_CTRLSHIFT9)
    return(FALSE);
  sprintf(ValueName,"Shortcut%d",Key-KEY_CTRLSHIFT0);
  SetRegKey("FolderShortcuts",ValueName,SrcFolder);
  sprintf(ValueName,"PluginModule%d",Key-KEY_CTRLSHIFT0);
  SetRegKey("FolderShortcuts",ValueName,PluginModule);
  sprintf(ValueName,"PluginFile%d",Key-KEY_CTRLSHIFT0);
  SetRegKey("FolderShortcuts",ValueName,PluginFile);
  sprintf(ValueName,"PluginData%d",Key-KEY_CTRLSHIFT0);
  SetRegKey("FolderShortcuts",ValueName,PluginData);
  return(TRUE);
}


void ShowFolderShortcut()
{
  int Pos=0;
  while (Pos!=-1)
    Pos=ShowFolderShortcutMenu(Pos);
}


int ShowFolderShortcutMenu(int Pos)
{
  struct MenuItem ListItem;
  VMenu FolderList(MSG(MFolderShortcutsTitle),NULL,0,ScrY-4);
  FolderList.SetFlags(MENU_SHOWAMPERSAND);
  FolderList.SetHelp("FolderShortcuts");
  FolderList.SetPosition(-1,-1,0,0);
  FolderList.SetBottomTitle(MSG(MFolderShortcutBottom));

  for (int I=0;I<10;I++)
  {
    char ValueName[100],FolderName[NM];
    memset(&ListItem,0,sizeof(ListItem));
    sprintf(ValueName,"Shortcut%d",I);
    GetRegKey("FolderShortcuts",ValueName,FolderName,"",sizeof(FolderName));
    TruncStr(FolderName,60);
    if (*FolderName==0)
      strcpy(FolderName,MSG(MShortcutNone));
    sprintf(ListItem.Name,"%s+%d   %s",MSG(MRightCtrl),I,FolderName);
    ListItem.Selected=(I == Pos);
    FolderList.AddItem(&ListItem);
  }

  FolderList.Show();

  while (!FolderList.Done())
  {
    int SelPos=FolderList.GetSelectPos();
    switch(FolderList.ReadInput())
    {
      case KEY_DEL:
        {
          char ValueName[100];
          sprintf(ValueName,"Shortcut%d",SelPos);
          SetRegKey("FolderShortcuts",ValueName,"");
          sprintf(ValueName,"PluginModule%d",SelPos);
          SetRegKey("FolderShortcuts",ValueName,"");
          sprintf(ValueName,"PluginFile%d",SelPos);
          SetRegKey("FolderShortcuts",ValueName,"");
          sprintf(ValueName,"PluginData%d",SelPos);
          SetRegKey("FolderShortcuts",ValueName,"");
          return(SelPos);
        }
      case KEY_INS:
        {
          char ValueName[100],NewDir[NM];
          CtrlObject->CmdLine->GetCurDir(NewDir);
          sprintf(ValueName,"Shortcut%d",SelPos);
          SetRegKey("FolderShortcuts",ValueName,NewDir);
          sprintf(ValueName,"PluginModule%d",SelPos);
          SetRegKey("FolderShortcuts",ValueName,"");
          sprintf(ValueName,"PluginFile%d",SelPos);
          SetRegKey("FolderShortcuts",ValueName,"");
          sprintf(ValueName,"PluginData%d",SelPos);
          SetRegKey("FolderShortcuts",ValueName,"");
          return(SelPos);
        }
      case KEY_F4:
        {
          char ValueName[100],PluginValueName[100],NewDir[NM],PluginModule[NM];
          char PluginFileValueName[100],PluginFile[NM];
          char PluginDataValueName[100],PluginData[8192];
          sprintf(ValueName,"Shortcut%d",SelPos);
          GetRegKey("FolderShortcuts",ValueName,NewDir,"",sizeof(NewDir));
          sprintf(PluginValueName,"PluginModule%d",SelPos);
          GetRegKey("FolderShortcuts",PluginValueName,PluginModule,"",sizeof(PluginModule));
          sprintf(PluginFileValueName,"PluginFile%d",SelPos);
          GetRegKey("FolderShortcuts",PluginFileValueName,PluginFile,"",sizeof(PluginFile));
          sprintf(PluginDataValueName,"PluginData%d",SelPos);
          GetRegKey("FolderShortcuts",PluginDataValueName,PluginData,"",sizeof(PluginData));
          if (GetString(MSG(MFolderShortcutsTitle),MSG(MEnterShortcut),NULL,NewDir,NewDir,sizeof(NewDir),"FolderShortcuts"))
          {
            SetRegKey("FolderShortcuts",ValueName,NewDir);
            SetRegKey("FolderShortcuts",PluginValueName,PluginModule);
            SetRegKey("FolderShortcuts",PluginFileValueName,PluginFile);
            SetRegKey("FolderShortcuts",PluginDataValueName,PluginData);
            return(SelPos);
          }
          break;
        }
      default:
        FolderList.ProcessInput();
        break;
    }
  }
  int ExitCode=FolderList.GetExitCode();
  if (ExitCode>=0)
  {
    FolderList.Hide();
    CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_RCTRL0+ExitCode);
  }
  return(-1);
}
