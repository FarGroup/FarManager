/*
ffolders.cpp

Folder shortcuts
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

#include "keys.hpp"
#include "lang.hpp"
#include "vmenu.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "filelist.hpp"
#include "registry.hpp"
#include "message.hpp"

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
    SetRegKey(FolderShortcuts,strValueName,NullToEmpty(*pValue));
  else if(Command == PSCR_CMDDELALL)
  {
		for(size_t I=0; I < countof(RecTypeName); ++I)
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
    VMenu FolderList(MSG(MFolderShortcutsTitle),NULL,0,ScrY-4);

    FolderList.SetFlags(VMENU_WRAPMODE); // VMENU_SHOWAMPERSAND|
    FolderList.SetHelp(HelpFolderShortcuts);
    FolderList.SetPosition(-1,-1,0,0);
    FolderList.SetBottomTitle(MSG(MFolderShortcutBottom));

    for (int I=0;I<10;I++)
    {
      string strFolderName;
      string strValueName;

      ListItem.Clear ();

      ProcessShortcutRecord(PSCR_CMDGET,PSCR_RT_SHORTCUT,I,&strFolderName);

      //TruncStr(strFolderName,60);

      if ( strFolderName.IsEmpty() )
      {
        ProcessShortcutRecord(PSCR_CMDGET,PSCR_RT_PLUGINMODULE,I,&strFolderName);
        if( strFolderName.IsEmpty() )
          strFolderName = MSG(MShortcutNone);
        else
          strFolderName = MSG(MShortcutPlugin);
      }

      ListItem.strName.Format (L"%s+&%d   %s", MSG(MRightCtrl),I,(const wchar_t*)strFolderName);
      ListItem.SetSelect(I == Pos);
      FolderList.AddItem(&ListItem);
    }

    FolderList.Show();

    while (!FolderList.Done())
    {
      DWORD Key=FolderList.ReadInput();
      int SelPos=FolderList.GetSelectPos();
      switch(Key)
      {
        case KEY_NUMDEL:
        case KEY_DEL:
        case KEY_NUMPAD0:
        case KEY_INS:
        {
          ProcessShortcutRecord(PSCR_CMDDELALL,0,SelPos,NULL);
          if(Key == KEY_INS || Key == KEY_NUMPAD0)
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

              strTemp = ph->pPlugin->GetModuleName();
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

          if (GetString(MSG(MFolderShortcutsTitle),MSG(MEnterShortcut),NULL,
                        strNewDir,strNewDir,HelpFolderShortcuts,FIB_BUTTONS/*|FIB_EDITPATH*/) &&
              StrCmp(strNewDir,strOldNewDir) != 0)
          {
            Unquote(strNewDir);
						if(!IsLocalRootPath(strNewDir))
              DeleteEndSlash(strNewDir);
            BOOL Saved=TRUE;
            apiExpandEnvironmentStrings(strNewDir,strOldNewDir);
						if(apiGetFileAttributes(strOldNewDir) == INVALID_FILE_ATTRIBUTES)
            {
              SetLastError(ERROR_PATH_NOT_FOUND);
              Saved=(Message(MSG_WARNING | MSG_ERRORTYPE, 2, MSG (MError), strNewDir, MSG(MSaveThisShortcut), MSG(MYes), MSG(MNo)) == 0);
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
