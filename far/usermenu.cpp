#include "common.hpp"

static int ProcessSingleMenu(char *MenuKey,int MenuPos);
static int DeleteMenuRecord(char *MenuKey,int DeletePos);
static int EditMenuRecord(char *MenuKey,int EditPos,int TotalRecords,int NewRec);
static int EditSubMenu(char *MenuKey,int EditPos,int TotalRecords,int NewRec);
static void MenuRegToFile(char *MenuKey,FILE *MenuFile);
static void MenuFileToReg(char *MenuKey,FILE *MenuFile);

static int MainMenu,MainMenuTitle,MenuModified;
static char MenuRootKey[100],LocalMenuName[100],LocalMenuKey[100];

void ProcessUserMenu(int EditMenu)
{
  char CurDir[NM];
  CtrlObject->CmdLine.GetCurDir(CurDir);
  if (chdir(CurDir)==-1)
    chdir(FarPath);

  FILE *MenuFile;

  sprintf(LocalMenuName,"LocalMenu%u",clock());
  sprintf(LocalMenuKey,"UserMenu\\%s",LocalMenuName);

  DeleteKeyTree(LocalMenuKey);

  char MenuFileName[NM];
  strcpy(MenuFileName,LocalMenuFileName);

  MainMenuTitle=MainMenu=TRUE;

  MenuModified=FALSE;

  if (EditMenu)
  {
    int EditChoice=Message(0,3,MSG(MUserMenuTitle),MSG(MChooseMenuType),
                   MSG(MChooseMenuMain),MSG(MChooseMenuLocal),MSG(MCancel));
    if (EditChoice<0 || EditChoice==2)
      return;
    MainMenuTitle=MainMenu=(EditChoice==0);
  }

  if ((!EditMenu || !MainMenu) && (MenuFile=fopen(MenuFileName,"rb"))!=NULL)
  {
    MenuFileToReg(LocalMenuKey,MenuFile);
    fclose(MenuFile);
    MainMenuTitle=MainMenu=FALSE;
  }
  else
    if (!EditMenu || MainMenu)
    {
      sprintf(MenuFileName,"%s%s",FarPath,LocalMenuFileName);
      if ((MenuFile=fopen(MenuFileName,"rb"))!=NULL)
      {
        MenuFileToReg(LocalMenuKey,MenuFile);
        fclose(MenuFile);
        MainMenu=FALSE;
      }
    }

  char MenuKey[512];
  strcpy(MenuKey,MainMenu ? "MainMenu":LocalMenuName);
  sprintf(MenuRootKey,"UserMenu\\%s",MenuKey);
  ProcessSingleMenu(MenuKey,0);

  chdir(CurDir);

  if (!MainMenu && MenuModified)
  {
    int FileAttr=GetFileAttributes(MenuFileName);
    if (FileAttr!=-1)
    {
      if (FileAttr & FA_RDONLY)
      {
        int AskOverwrite;
        AskOverwrite=Message(MSG_WARNING,2,MSG(MUserMenuTitle),MenuFileName,
                     MSG(MEditRO),MSG(MEditOvr),MSG(MYes),MSG(MNo));
        if (AskOverwrite==0)
          SetFileAttributes(MenuFileName,FileAttr & ~FA_RDONLY);
      }
      if (FileAttr & (FA_HIDDEN|FA_SYSTEM))
        SetFileAttributes(MenuFileName,0);
    }
    if ((MenuFile=fopen(MenuFileName,"wb"))!=NULL)
    {
      MenuRegToFile(LocalMenuKey,MenuFile);
      long Length=filelen(MenuFile);
      fclose(MenuFile);
      if (Length==0)
        remove(LocalMenuFileName);
      CtrlObject->ActivePanel->Update(UPDATE_KEEP_SELECTION);
      CtrlObject->ActivePanel->Redraw();
    }
  }
  if (!MainMenu)
    DeleteKeyTree(LocalMenuKey);
}


int ProcessSingleMenu(char *MenuKey,int MenuPos)
{
  while (1)
  {
    struct MenuItem UserMenuItem;
    UserMenuItem.Checked=UserMenuItem.Separator=*UserMenuItem.UserData=UserMenuItem.UserDataSize=0;
    int NumLine,ExitCode,FuncPos[12];
  
    for (int I=0;I<sizeof(FuncPos)/sizeof(FuncPos[0]);I++)
      FuncPos[I]=-1;
  
    char Name[NM],ShortName[NM];
    CtrlObject->ActivePanel->GetCurName(Name,ShortName);
  
    {
      VMenu UserMenu(MainMenuTitle ? MSG(MMainMenuTitle):MSG(MLocalMenuTitle),NULL,0,ScrY-4);
      UserMenu.SetHelp("UserMenu");
      UserMenu.SetPosition(-1,-1,0,0);
  
      NumLine=0;
  
      while (1)
      {
        char ItemKey[512],HotKey[10],Label[512],MenuText[512];
        sprintf(ItemKey,"UserMenu\\%s\\Item%d",MenuKey,NumLine);
        if (!GetRegKey(ItemKey,"HotKey",HotKey,"",sizeof(HotKey)))
          break;
        if (!GetRegKey(ItemKey,"Label",Label,"",sizeof(Label)))
          break;
        SubstFileName(Label,Name,ShortName,NULL,NULL,TRUE);
        int SubMenu;
        GetRegKey(ItemKey,"Submenu",SubMenu,0);
  
        int FuncNum=0;
        if (strlen(HotKey)>1)
        {
          FuncNum=atoi(&HotKey[1]);
          if (FuncNum<1 || FuncNum>12)
            FuncNum=1;
          sprintf(HotKey,"F%d",FuncNum);
        }
        sprintf(MenuText,"%s%-3.3s %-20.*s",FuncNum>0 ? "":"&",HotKey,ScrX-12,Label);
        if (SubMenu)
          strcat(MenuText," ");
        strncpy(UserMenuItem.Name,MenuText,sizeof(UserMenuItem.Name));
        UserMenuItem.Selected=(NumLine==MenuPos);
        int ItemPos=UserMenu.AddItem(&UserMenuItem);
        if (FuncNum>0)
          FuncPos[FuncNum-1]=ItemPos;
        NumLine++;
      }
  
      *UserMenuItem.Name=0;
      UserMenuItem.Selected=(NumLine==MenuPos);
      UserMenu.AddItem(&UserMenuItem);
  
      {
        UserMenu.Show();
        while (!UserMenu.Done())
        {
          int SelectPos=UserMenu.GetSelectPos();
          int Key=UserMenu.ReadInput();
          if (Key>=KEY_F1 && Key<=KEY_F12)
          {
            int FuncItemPos;
            if ((FuncItemPos=FuncPos[Key-KEY_F1])!=-1)
            {
              UserMenu.SetExitCode(FuncItemPos);
              continue;
            }
          }
          switch(Key)
          {
            case KEY_DEL:
              if (SelectPos<NumLine)
                if (DeleteMenuRecord(MenuKey,SelectPos))
                {
                  UserMenu.Hide();
                  return(ProcessSingleMenu(MenuKey,SelectPos));
                }
              break;
            case KEY_INS:
              if (EditMenuRecord(MenuKey,SelectPos,NumLine,1))
              {
                UserMenu.Hide();
                return(ProcessSingleMenu(MenuKey,SelectPos));
              }
              break;
            case KEY_F4:
            case KEY_SHIFTF4:
              if (SelectPos<NumLine)
                if (EditMenuRecord(MenuKey,SelectPos,NumLine,0))
                {
                  UserMenu.Hide();
                  return(ProcessSingleMenu(MenuKey,SelectPos));
                }
              break;
            case KEY_ALTF4:
              if (RegVer)
              {
                FILE *MenuFile;
                char MenuFileName[NM];
                strcpy(MenuFileName,Opt.TempPath);
                strcat(MenuFileName,"FarTmpXXXXXX");
                if (mktemp(MenuFileName)==NULL || (MenuFile=fopen(MenuFileName,"wb"))==NULL)
                  break;
                MenuRegToFile(MenuRootKey,MenuFile);
                fclose(MenuFile);
                {
                  char OldTitle[512];
                  GetConsoleTitle(OldTitle,sizeof(OldTitle));
                  FileEditor ShellEditor(MenuFileName,FALSE,FALSE,-1,-1,FALSE);
                  SetConsoleTitle(OldTitle);
                  if (ShellEditor.GetExitCode()!=1 || (MenuFile=fopen(MenuFileName,"rb"))==NULL)
                  {
                    remove(MenuFileName);
                    break;
                  }
                }
                DeleteKeyTree(MenuRootKey);
                MenuFileToReg(MenuRootKey,MenuFile);
                fclose(MenuFile);
                remove(MenuFileName);
                UserMenu.Hide();
                MenuModified=TRUE;
                return(FALSE);
              }
              else
                Message(MSG_WARNING,1,MSG(MWarning),MSG(MRegOnly),MSG(MOk));
              break;
            default:
              UserMenu.ProcessInput();
              break;
          }
        }
      }
      ExitCode=UserMenu.GetExitCode();
    }
  
    if (ExitCode<0 || ExitCode>=NumLine)
      return(TRUE);
  
    char CurrentKey[512];
    int SubMenu;
    sprintf(CurrentKey,"UserMenu\\%s\\Item%d",MenuKey,ExitCode);
    GetRegKey(CurrentKey,"Submenu",SubMenu,0);
  
    if (SubMenu)
    {
      char SubMenuKey[512];
      sprintf(SubMenuKey,"%s\\Item%d",MenuKey,ExitCode);
      if (!ProcessSingleMenu(SubMenuKey,0))
        return(FALSE);
      MenuPos=ExitCode;
      continue;
    }
  
    int LeftVisible,RightVisible,PanelsHidden=0;
    int CurLine=0;
  
    char CmdLineDir[NM];
    CtrlObject->CmdLine.GetCurDir(CmdLineDir);
  
    while (1)
    {
      char LineName[50],Command[4096];
      sprintf(LineName,"Command%d",CurLine);
      if (!GetRegKey(CurrentKey,LineName,Command,"",sizeof(Command)))
        break;
  
      char ListName[NM],ShortListName[NM];
      {
        int PreserveLFN=SubstFileName(Command,Name,ShortName,ListName,ShortListName,FALSE,CmdLineDir);
        PreserveLongName PreserveName(ShortName,PreserveLFN);
        if (!PanelsHidden)
        {
          LeftVisible=CtrlObject->LeftPanel->IsVisible();
          RightVisible=CtrlObject->RightPanel->IsVisible();
          CtrlObject->LeftPanel->Hide();
          CtrlObject->RightPanel->Hide();
          CtrlObject->LeftPanel->SetUpdateMode(FALSE);
          CtrlObject->RightPanel->SetUpdateMode(FALSE);
          PanelsHidden=TRUE;
        }
        if (*Command)
          CtrlObject->CmdLine.ExecString(Command,FALSE);
      }
      if (*ListName)
        remove(ListName);
      if (*ShortListName)
        remove(ShortListName);
      CurLine++;
    }
    if (PanelsHidden)
    {
      CtrlObject->LeftPanel->SetUpdateMode(TRUE);
      CtrlObject->RightPanel->SetUpdateMode(TRUE);
      CtrlObject->LeftPanel->Update(UPDATE_KEEP_SELECTION);
      CtrlObject->RightPanel->Update(UPDATE_KEEP_SELECTION);
      if (RightVisible)
        CtrlObject->RightPanel->Show();
      if (LeftVisible)
        CtrlObject->LeftPanel->Show();
    }
    return(FALSE);
  }
}


int DeleteMenuRecord(char *MenuKey,int DeletePos)
{
  char RecText[200],ItemName[200],RegKey[512];
  sprintf(RegKey,"UserMenu\\%s\\Item%d",MenuKey,DeletePos);
  GetRegKey(RegKey,"Label",RecText,"",sizeof(RecText));
  sprintf(ItemName,"\"%s\"",RecText);
  if (Message(MSG_WARNING,2,MSG(MUserMenuTitle),MSG(MAskDeleteMenuItem),
              ItemName,MSG(MDelete),MSG(MCancel))!=0)
    return(FALSE);
  MenuModified=TRUE;
  sprintf(RegKey,"UserMenu\\%s\\Item%%d",MenuKey);
  DeleteKeyRecord(RegKey,DeletePos);
  return(TRUE);
}


int EditMenuRecord(char *MenuKey,int EditPos,int TotalRecords,int NewRec)
{
  static struct DialogData EditDlgData[]={
    DI_DOUBLEBOX,3,1,72,20,0,0,0,0,(char *)MEditMenuTitle,
    DI_TEXT,5,2,0,0,0,0,0,0,(char *)MEditMenuHotKey,
    DI_FIXEDIT,5,3,7,3,1,0,0,0,"",
    DI_TEXT,5,4,0,0,0,0,0,0,(char *)MEditMenuLabel,
    DI_EDIT,5,5,70,3,0,0,0,0,"",
    DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_TEXT,5,7,0,0,0,0,0,0,(char *)MEditMenuCommands,
    DI_EDIT,5,8,70,3,0,0,DIF_EDITOR,0,"",
    DI_EDIT,5,9,70,3,0,0,DIF_EDITOR,0,"",
    DI_EDIT,5,10,70,3,0,0,DIF_EDITOR,0,"",
    DI_EDIT,5,11,70,3,0,0,DIF_EDITOR,0,"",
    DI_EDIT,5,12,70,3,0,0,DIF_EDITOR,0,"",
    DI_EDIT,5,13,70,3,0,0,DIF_EDITOR,0,"",
    DI_EDIT,5,14,70,3,0,0,DIF_EDITOR,0,"",
    DI_EDIT,5,15,70,3,0,0,DIF_EDITOR,0,"",
    DI_EDIT,5,16,70,3,0,0,DIF_EDITOR,0,"",
    DI_EDIT,5,17,70,3,0,0,DIF_EDITOR,0,"",
    DI_TEXT,3,18,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(EditDlgData,EditDlg);

  char ItemKey[512];
  sprintf(ItemKey,"UserMenu\\%s\\Item%d",MenuKey,EditPos);

  MenuModified=TRUE;

  if (NewRec)
  {
    switch (Message(0,2,MSG(MUserMenuTitle),MSG(MAskInsertMenuOrCommand),
                    MSG(MMenuInsertCommand),MSG(MMenuInsertMenu)))
    {
      case -1:
      case -2:
        return(FALSE);
      case 1:
        return(EditSubMenu(MenuKey,EditPos,TotalRecords,TRUE));
    }
  }
  else
  {
    int SubMenu;
    GetRegKey(ItemKey,"Submenu",SubMenu,0);
    if (SubMenu)
      return(EditSubMenu(MenuKey,EditPos,TotalRecords,FALSE));
    GetRegKey(ItemKey,"HotKey",EditDlg[2].Data,"",sizeof(EditDlg[2].Data));
    GetRegKey(ItemKey,"Label",EditDlg[4].Data,"",sizeof(EditDlg[4].Data));
    int CommandNumber=0;
    while (CommandNumber<10)
    {
      char CommandName[20],Command[4096];
      sprintf(CommandName,"Command%d",CommandNumber);
      if (!GetRegKey(ItemKey,CommandName,Command,"",sizeof(Command)))
        break;
      strncpy(EditDlg[7+CommandNumber].Data,Command,sizeof(EditDlg[0].Data));
      CommandNumber++;
    }
  }

  {
    Dialog Dlg(EditDlg,sizeof(EditDlg)/sizeof(EditDlg[0]));
    Dlg.SetHelp("UserMenu");
    Dlg.SetPosition(-1,-1,76,22);
    Dlg.Process();
    if (Dlg.GetExitCode()!=18 || *EditDlg[4].Data==0)
      return(FALSE);
  }

  if (NewRec)
  {
    char KeyMask[512];
    sprintf(KeyMask,"UserMenu\\%s\\Item%%d",MenuKey);
    InsertKeyRecord(KeyMask,EditPos,TotalRecords);
  }

  SetRegKey(ItemKey,"HotKey",EditDlg[2].Data);
  SetRegKey(ItemKey,"Label",EditDlg[4].Data);
  SetRegKey(ItemKey,"Submenu",(DWORD)0);

  int CommandNumber=0,I;
  for (I=0;I<10;I++)
    if (*EditDlg[I+7].Data)
      CommandNumber=I+1;
  for (I=0;I<10;I++)
  {
    char CommandName[20];
    sprintf(CommandName,"Command%d",I);
    if (I>=CommandNumber)
      DeleteRegValue(ItemKey,CommandName);
    else
      SetRegKey(ItemKey,CommandName,EditDlg[I+7].Data);
  }
  return(TRUE);
}


int EditSubMenu(char *MenuKey,int EditPos,int TotalRecords,int NewRec)
{
  static struct DialogData EditDlgData[]=
  {
    DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(char *)MEditSubmenuTitle,
    DI_TEXT,5,2,0,0,0,0,0,0,(char *)MEditSubmenuHotKey,
    DI_FIXEDIT,5,3,7,3,1,0,0,0,"",
    DI_TEXT,5,4,0,0,0,0,0,0,(char *)MEditSubmenuLabel,
    DI_EDIT,5,5,70,3,0,0,0,0,"",
    DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(EditDlgData,EditDlg);

  char ItemKey[512];
  sprintf(ItemKey,"UserMenu\\%s\\Item%d",MenuKey,EditPos);
  if (NewRec)
  {
    *EditDlg[2].Data=0;
    *EditDlg[4].Data=0;
  }
  else
  {
    GetRegKey(ItemKey,"HotKey",EditDlg[2].Data,"",sizeof(EditDlg[2].Data));
    GetRegKey(ItemKey,"Label",EditDlg[4].Data,"",sizeof(EditDlg[4].Data));
  }
  {
    Dialog Dlg(EditDlg,sizeof(EditDlg)/sizeof(EditDlg[0]));
    Dlg.SetHelp("UserMenu");
    Dlg.SetPosition(-1,-1,76,10);
    Dlg.Process();
    if (Dlg.GetExitCode()!=6 || *EditDlg[4].Data==0)
      return(FALSE);
  }
  if (NewRec)
  {
    char KeyMask[512];
    sprintf(KeyMask,"UserMenu\\%s\\Item%%d",MenuKey);
    InsertKeyRecord(KeyMask,EditPos,TotalRecords);
  }

  SetRegKey(ItemKey,"HotKey",EditDlg[2].Data);
  SetRegKey(ItemKey,"Label",EditDlg[4].Data);
  SetRegKey(ItemKey,"Submenu",(DWORD)1);
  return(TRUE);
}


void MenuRegToFile(char *MenuKey,FILE *MenuFile)
{
  for (int I=0;;I++)
  {
    char ItemKey[512],HotKey[10],Label[512];
    int SubMenu;
    sprintf(ItemKey,"%s\\Item%d",MenuKey,I);
    if (!GetRegKey(ItemKey,"Label",Label,"",sizeof(Label)))
      break;
    GetRegKey(ItemKey,"Label",Label,"",sizeof(Label));
    GetRegKey(ItemKey,"HotKey",HotKey,"",sizeof(HotKey));
    GetRegKey(ItemKey,"Submenu",SubMenu,0);
    fprintf(MenuFile,"%s:  %s\r\n",HotKey,Label);
    if (SubMenu)
    {
      fprintf(MenuFile,"{\r\n");
      MenuRegToFile(ItemKey,MenuFile);
      fprintf(MenuFile,"}\r\n");
    }
    else
      for (int J=0;;J++)
      {
        char LineName[50],Command[4096];
        sprintf(LineName,"Command%d",J);
        if (!GetRegKey(ItemKey,LineName,Command,"",sizeof(Command)))
          break;
        fprintf(MenuFile,"    %s\r\n",Command);
      }
  }
}


void MenuFileToReg(char *MenuKey,FILE *MenuFile)
{
  char MenuStr[512];
  int KeyNumber=-1,CommandNumber;
  while (fgets(MenuStr,sizeof(MenuStr),MenuFile)!=NULL)
  {
    char ItemKey[512];
    sprintf(ItemKey,"%s\\Item%d",MenuKey,KeyNumber);
    RemoveTrailingSpaces(MenuStr);
    if (*MenuStr==0)
      continue;
    if (*MenuStr=='{' && KeyNumber>=0)
    {
      MenuFileToReg(ItemKey,MenuFile);
      continue;
    }
    if (*MenuStr=='}')
      break;
    if (!isspace(*MenuStr))
    {
      char HotKey[10],Label[512],*ChPtr;
      int SubMenu;
      if ((ChPtr=strchr(MenuStr,':'))==NULL)
        continue;
      sprintf(ItemKey,"%s\\Item%d",MenuKey,++KeyNumber);
      *ChPtr=0;
      strcpy(HotKey,MenuStr);
      strcpy(Label,ChPtr+1);
      RemoveLeadingSpaces(Label);
      SaveFilePos SavePos(MenuFile);
      SubMenu=(fgets(MenuStr,sizeof(MenuStr),MenuFile)!=NULL && *MenuStr=='{');
      UseSameRegKey();
      SetRegKey(ItemKey,"HotKey",HotKey);
      SetRegKey(ItemKey,"Label",Label);
      SetRegKey(ItemKey,"Submenu",SubMenu);
      CloseSameRegKey();
      CommandNumber=0;
    }
    else
      if (KeyNumber>=0)
      {
        char LineName[50];
        sprintf(LineName,"Command%d",CommandNumber++);
        RemoveLeadingSpaces(MenuStr);
        SetRegKey(ItemKey,LineName,MenuStr);
      }
  }
}

