/*
ffolders.cpp

Folder shortcuts

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
#include "filelist.hpp"

static int ShowFolderShortcutMenu(int Pos);
static const wchar_t HelpFolderShortcuts[]=L"FolderShortcuts";

enum PSCR_CMD{
  PSCR_CMDGET,
  PSCR_CMDSET,
  PSCR_CMDDELALL,
  PSCR_CMDGETFILDERSIZE,
};

enum PSCR_RECTYPE{
  PSCR_RT_SHORTCUT,
  PSCR_RT_PLUGINMODULE,
  PSCR_RT_PLUGINFILE,
  PSCR_RT_PLUGINDATA,
};

static int ProcessShortcutRecord(int Command,int ValType,int RecNumber, string *pValue)
{
  static const wchar_t FolderShortcuts[]=L"FolderShortcuts";
  static const wchar_t *RecTypeName[]={
    L"Shortcut%d",
    L"PluginModule%d",
    L"PluginFile%d",
    L"PluginData%d",
  };

  string strValueName;

  strValueName.Format (RecTypeName[ValType], RecNumber);

  if(Command == PSCR_CMDGET)
    GetRegKey(FolderShortcuts,strValueName,*pValue,L"");
  else if(Command == PSCR_CMDSET)
    SetRegKey(FolderShortcuts,strValueName,NullToEmptyW(*pValue));
  else if(Command == PSCR_CMDDELALL)
  {
    for(int I=0; I < sizeof(RecTypeName)/sizeof(RecTypeName[0]); ++I)
    {
      strValueName.Format (RecTypeName[I],RecNumber);
      SetRegKey(FolderShortcuts,strValueName,L"");
    }
  }
  else if(Command == PSCR_CMDGETFILDERSIZE)
    return GetRegKeySize(FolderShortcuts,strValueName);
  return 0;
}

int GetShortcutFolderSize(int Key)
{
  if (Key<KEY_RCTRL0 || Key>KEY_RCTRL9)
    return 0;
  Key-=KEY_RCTRL0;

  return ProcessShortcutRecord(PSCR_CMDGETFILDERSIZE,PSCR_RT_SHORTCUT,Key,NULL);
}


int GetShortcutFolder(int Key,string *pDestFolder,
                              string *pPluginModule,
                              string *pPluginFile,
                              string *pPluginData)
{
  if (Key<KEY_RCTRL0 || Key>KEY_RCTRL9)
    return(FALSE);

  string strFolder;

  Key-=KEY_RCTRL0;

  ProcessShortcutRecord(PSCR_CMDGET,PSCR_RT_SHORTCUT,Key,&strFolder);
  apiExpandEnvironmentStrings(strFolder, *pDestFolder);

  if(pPluginModule)
    ProcessShortcutRecord(PSCR_CMDGET,PSCR_RT_PLUGINMODULE,Key,pPluginModule);
  if(pPluginFile)
    ProcessShortcutRecord(PSCR_CMDGET,PSCR_RT_PLUGINFILE,Key,pPluginFile);
  if(pPluginData)
    ProcessShortcutRecord(PSCR_CMDGET,PSCR_RT_PLUGINDATA,Key,pPluginData);

  return ( !pDestFolder->IsEmpty() || (pPluginModule && !pPluginModule->IsEmpty()) );
}


int SaveFolderShortcut(int Key,string *pSrcFolder,
                               string *pPluginModule,
                               string *pPluginFile,
                               string *pPluginData)
{
  if (Key<KEY_CTRLSHIFT0 || Key>KEY_CTRLSHIFT9)
    return(FALSE);

  Key-=KEY_CTRLSHIFT0;
  ProcessShortcutRecord(PSCR_CMDSET,PSCR_RT_SHORTCUT,Key,pSrcFolder);
  ProcessShortcutRecord(PSCR_CMDSET,PSCR_RT_PLUGINMODULE,Key,pPluginModule);
  ProcessShortcutRecord(PSCR_CMDSET,PSCR_RT_PLUGINFILE,Key,pPluginFile);
  ProcessShortcutRecord(PSCR_CMDSET,PSCR_RT_PLUGINDATA,Key,pPluginData);

  return(TRUE);
}


void ShowFolderShortcut()
{
  int Pos=0;
  while (Pos!=-1)
    Pos=ShowFolderShortcutMenu(Pos);
}


static int ShowFolderShortcutMenu(int Pos)
{
  int ExitCode=-1;
  {
    MenuItemEx ListItem;
    VMenu FolderList(UMSG(MFolderShortcutsTitle),NULL,0,ScrY-4);
    /* $ 16.06.2001 KM
       ! Добавление WRAPMODE в меню.
    */
    FolderList.SetFlags(VMENU_WRAPMODE); // VMENU_SHOWAMPERSAND|
    /* KM $ */
    FolderList.SetHelp(HelpFolderShortcuts);
    FolderList.SetPosition(-1,-1,0,0);
    FolderList.SetBottomTitle(UMSG(MFolderShortcutBottom));

    for (int I=0;I<10;I++)
    {
      string strFolderName;
      string strValueName;

      ListItem.Clear ();

      ProcessShortcutRecord(PSCR_CMDGET,PSCR_RT_SHORTCUT,I,&strFolderName);

      TruncStr(strFolderName,60);

      if ( strFolderName.IsEmpty() )
      {
        ProcessShortcutRecord(PSCR_CMDGET,PSCR_RT_PLUGINMODULE,I,&strFolderName);
        if( strFolderName.IsEmpty() )
          strFolderName = UMSG(MShortcutNone);
        else
          strFolderName = UMSG(MShortcutPlugin);
      }

      ListItem.strName.Format (L"%s+&%d   %s", UMSG(MRightCtrl),I,(const wchar_t*)strFolderName);
      ListItem.SetSelect(I == Pos);
      FolderList.AddItem(&ListItem);
    }

    FolderList.Show();

    while (!FolderList.Done())
    {
      int SelPos=FolderList.GetSelectPos();
      DWORD Key=FolderList.ReadInput();
      switch(Key)
      {
        case KEY_DEL:
        case KEY_INS:
        {
          ProcessShortcutRecord(PSCR_CMDDELALL,0,SelPos,NULL);
          if(Key == KEY_INS)
          {
            Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;

            string strNewDir;
            CtrlObject->CmdLine->GetCurDir(strNewDir);

            ProcessShortcutRecord(PSCR_CMDSET,PSCR_RT_SHORTCUT,SelPos,&strNewDir);

            if(ActivePanel->GetMode() == PLUGIN_PANEL)
            {
              struct OpenPluginInfo Info;
              ActivePanel->GetOpenPluginInfo(&Info);

              string strTemp;

              PluginHandle *ph = (PluginHandle*)ActivePanel->GetPluginHandle();

              strTemp = ph->pPlugin->m_strModuleName;
              ProcessShortcutRecord(PSCR_CMDSET,PSCR_RT_PLUGINMODULE,SelPos,&strTemp);

              strTemp = Info.HostFile;
              ProcessShortcutRecord(PSCR_CMDSET,PSCR_RT_PLUGINFILE,SelPos,&strTemp);

              strTemp = Info.ShortcutData;
              ProcessShortcutRecord(PSCR_CMDSET,PSCR_RT_PLUGINDATA,SelPos,&strTemp);
            }
          }
          return(SelPos);
        }

        case KEY_F4:
        {
          /* Пока оставим именно так:
             Редактирование по F4 - считаем что ЭТО абс.файловый путь!
             TODO: потом добавим и работу с плагинами (возможно :-)
          */
          string strOldNewDir;
          string strNewDir;

          ProcessShortcutRecord(PSCR_CMDGET,PSCR_RT_SHORTCUT,SelPos,&strNewDir);

          strOldNewDir = strNewDir;

          if (GetString(UMSG(MFolderShortcutsTitle),UMSG(MEnterShortcut),NULL,
                        strNewDir,strNewDir,1024, HelpFolderShortcuts,FIB_BUTTONS/*|FIB_EDITPATH*/) && //BUGBUG 1024
              wcscmp(strNewDir,strOldNewDir) != 0)
          {
            Unquote(strNewDir);
            if(!(strNewDir.At(1) == L':' && strNewDir.At(2) == L'\\' && strNewDir.At(3) == 0))
              DeleteEndSlashW(strNewDir);
            BOOL Saved=TRUE;
            apiExpandEnvironmentStrings(strNewDir,strOldNewDir);
            if(GetFileAttributesW(strOldNewDir) == -1)
            {
              TruncPathStr(strNewDir, ScrX-16);
              SetLastError(ERROR_PATH_NOT_FOUND);
              Saved=(Message(MSG_WARNING | MSG_ERRORTYPE, 2, UMSG (MError), strNewDir, UMSG(MSaveThisShortcut), UMSG(MYes), UMSG(MNo)) == 0);
            }

            if(Saved)
            {
              ProcessShortcutRecord(PSCR_CMDDELALL,0,SelPos,NULL);
              ProcessShortcutRecord(PSCR_CMDSET,PSCR_RT_SHORTCUT,SelPos,&strNewDir);
              return(SelPos);
            }
          }
          break;
        }
        default:
          FolderList.ProcessInput();
          break;
      }
    }
    ExitCode=FolderList.Modal::GetExitCode();
    FolderList.Hide();
  }

  if (ExitCode>=0)
  {
    CtrlObject->Cp()->ActivePanel->ProcessKey(KEY_RCTRL0+ExitCode);
  }

  return(-1);
}
