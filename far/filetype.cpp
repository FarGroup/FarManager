/*
filetype.cpp

Работа с ассоциациями файлов

*/

/* Revision: 1.08 03.11.2000 $ */

/*
Modify:
  03.11.2000 OT
    ! Введение проверки возвращаемого значения 
  02.11.2000 OT
    ! Введение проверки на длину буфера, отведенного под имя файла.
  01.11.2000 IS
    - Имя файла в случае !$! должно быть коротким
  02.09.2000 tran
    - !@!, !#!@! bug
  01.08.2000 SVS
    ! Изменения, касаемые измений в структурах DialogItem
    ! в usermenu конструкция !?<title>?<init>! с расширением переменных среды!
  21.07.2000 IG
    - Bug 15 (не работала комманда executable.exe !.!?ext:?!)
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop


/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */

static int DeleteTypeRecord(int DeletePos);
static int EditTypeRecord(int EditPos,int TotalRecords,int NewRec);
static int GetDescriptionWidth();
static void ReplaceVariables(char *Str);

int ProcessLocalFileTypes(char *Name,char *ShortName,int Mode,int AlwaysWaitFinish)
{
  char Commands[32][512],Descriptions[32][128],Command[512];
  int CommandCount=0;
  for (int I=0;;I++)
  {
    char RegKey[80],Mask[512];
    sprintf(RegKey,"Associations\\Type%d",I);
    if (!GetRegKey(RegKey,"Mask",Mask,"",sizeof(Mask)))
      break;
    char NewCommand[512],ArgName[NM],*NamePtr=Mask;
    while ((NamePtr=GetCommaWord(NamePtr,ArgName))!=NULL)
      if (CmpName(ArgName,Name))
      {
        switch(Mode)
        {
          case FILETYPE_EXEC:
            GetRegKey(RegKey,"Execute",NewCommand,"",sizeof(NewCommand));
            break;
          case FILETYPE_VIEW:
            GetRegKey(RegKey,"View",NewCommand,"",sizeof(NewCommand));
            break;
          case FILETYPE_EDIT:
            GetRegKey(RegKey,"Edit",NewCommand,"",sizeof(NewCommand));
            break;
        }
        if (*NewCommand==0 || CommandCount>=sizeof(Commands)/sizeof(Commands[0]))
          break;
        if (strcmp(ArgName,"*.*")==0 || strcmp(ArgName,"*")==0)
          if (CommandCount>0)
            continue;
        strcpy(Command,NewCommand);
        strcpy(Commands[CommandCount],Command);
        GetRegKey(RegKey,"Description",Descriptions[CommandCount],"",sizeof(Descriptions[0]));
        CommandCount++;
        break;
      }
  }
  if (CommandCount==0)
    return(FALSE);
  if (CommandCount>1)
  {
    struct MenuItem TypesMenuItem;
    TypesMenuItem.Checked=TypesMenuItem.Separator=*TypesMenuItem.UserData=TypesMenuItem.UserDataSize=0;
    VMenu TypesMenu(MSG(MSelectAssocTitle),NULL,0,ScrY-4);
    TypesMenu.SetHelp("FileAssoc");
    TypesMenu.SetFlags(MENU_WRAPMODE);
    TypesMenu.SetPosition(-1,-1,0,0);

    int DizWidth=GetDescriptionWidth();

    for (int I=0;I<CommandCount;I++)
    {
      char CommandText[512],MenuText[512];
      strcpy(CommandText,Commands[I]);
      SubstFileName(CommandText,Name,ShortName,NULL,NULL,TRUE);
      if (DizWidth==0)
        *MenuText=0;
      else
      {
        char Title[256];
        if (*Descriptions[I])
          strcpy(Title,Descriptions[I]);
        else
          *Title=0;
        int Ampersand=strchr(Title,'&')!=NULL;
        sprintf(MenuText,"%-*.*s │ ",DizWidth+Ampersand,DizWidth+Ampersand,Title);
      }
      TruncStr(CommandText,Min(ScrX,sizeof(TypesMenuItem.Name)-1)-DizWidth-14);
      strcat(MenuText,CommandText);
      TruncStr(MenuText,sizeof(TypesMenuItem.Name)-1);
      strcpy(TypesMenuItem.Name,MenuText);
      TypesMenuItem.Selected=(I==0);
      TypesMenu.AddItem(&TypesMenuItem);
    }
    TypesMenu.Process();
    int ExitCode=TypesMenu.GetExitCode();
    if (ExitCode<0)
      return(TRUE);
    strcpy(Command,Commands[ExitCode]);
  }

  /* $ 02.09.2000 tran
     [NM] -> [NM*2] */
  char ListName[NM*2],ShortListName[NM*2];
  {
    int PreserveLFN=SubstFileName(Command,Name,ShortName,ListName,ShortListName);
    PreserveLongName PreserveName(ShortName,PreserveLFN);
    if (*Command)
      if (*Command!='@')
      {
        CtrlObject->CmdLine.ExecString(Command,AlwaysWaitFinish);
        if (!AlwaysWaitFinish)
          CtrlObject->CmdHistory->AddToHistory(Command);
      }
      else
      {
        SaveScreen SaveScr;
        CtrlObject->LeftPanel->CloseFile();
        CtrlObject->RightPanel->CloseFile();
        Execute(Command+1,AlwaysWaitFinish);
      }
  }
  /* $ 02.09.2000 tran
     remove 4 files, not 2*/
  if (*ListName)
    remove(ListName);
  if (*(ListName+NM))
    remove(ListName+NM);
  if (*ShortListName)
    remove(ShortListName);
  if (*(ShortListName+NM))
    remove(ShortListName+NM);
  /* tran $ */
  return(TRUE);
}


int ProcessGlobalFileTypes(char *Name,int AlwaysWaitFinish)
{
  char Value[80],*ExtPtr;
  LONG ValueSize;
  if ((ExtPtr=strrchr(Name,'.'))==NULL)
    return(FALSE);
  ValueSize=sizeof(Value);
  if (RegQueryValue(HKEY_CLASSES_ROOT,(LPCTSTR)ExtPtr,(LPTSTR)Value,&ValueSize)!=ERROR_SUCCESS)
    return(FALSE);
  if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion<4)
  {
    char AssocStr[2*NM],ExpAssocStr[2*NM],*ChPtr;
    strcat(Value,"\\shell\\open\\command");
    HKEY hKey;
    if (RegOpenKey(HKEY_CLASSES_ROOT,Value,&hKey)!=ERROR_SUCCESS)
      return(FALSE);
    ValueSize=sizeof(AssocStr);
#if defined(__BORLANDC__) && (__BORLANDC__ <= 0x520)
    if (RegQueryValueEx(hKey,"",NULL,NULL,(LPTSTR)AssocStr,(LPDWORD)&ValueSize)!=ERROR_SUCCESS)
#else
    if (RegQueryValueEx(hKey,"",NULL,NULL,(unsigned char *)AssocStr,(LPDWORD)&ValueSize)!=ERROR_SUCCESS)
#endif
      return(FALSE);
    RegCloseKey(hKey);
    ExpandEnvironmentStrings(AssocStr,ExpAssocStr,sizeof(ExpAssocStr));
    if ((ChPtr=strstr(ExpAssocStr,"%*"))!=NULL)
    {
      char TmpStr[512];
      strcpy(TmpStr,ChPtr+2);
      strcpy(ChPtr,TmpStr);
    }
    if ((ChPtr=strstr(ExpAssocStr,"%1"))!=NULL)
    {
      char TmpStr[512];
      strcpy(TmpStr,ChPtr+2);
      strcpy(ChPtr,Name);
      strcat(ExpAssocStr,TmpStr);
    }
    else
    {
      char ExecStr[NM];
      if (FindExecutable(Name,"",ExecStr)<=(HINSTANCE)32)
        return(FALSE);
      sprintf(ExpAssocStr,"%s %s",QuoteSpace(ExecStr),QuoteSpace(Name));
    }

    if (*ExpAssocStr!='"')
      for (int I=0;ExpAssocStr[I]!=0;I++)
        if (strnicmp(&ExpAssocStr[I],".exe",4)==0 &&
            (ExpAssocStr[I+4]==' ' || ExpAssocStr[I+4]=='/'))
        {
          int SpacePresent=0;
          for (int J=0;J<I;J++)
            if (ExpAssocStr[J]==' ')
            {
              SpacePresent=1;
              break;
            }
          if (SpacePresent)
          {
            char NewStr[2*NM];
            strncpy(NewStr,ExpAssocStr,I+4);
            NewStr[I+4]=0;
            QuoteSpace(NewStr);
            strcat(NewStr,&ExpAssocStr[I+4]);
            strcpy(ExpAssocStr,NewStr);
            QuoteSpace(ExpAssocStr);
          }
          break;
        }
    CtrlObject->CmdLine.ExecString(ExpAssocStr,AlwaysWaitFinish);
  }
  else
  {
/*
    char FullName[2*NM];
    ConvertNameToFull(Name,FullName, sizeof(FullName));
    if (FullName[0]=='\\' && FullName[1]=='\\')
      strcpy(Name,FullName);
    else
      if ((strstr(Name,"ftp")!=NULL || strstr(Name,"www")!=NULL) && PointToName(Name)==Name)
      {
        sprintf(FullName,".\\%s",Name);
        strcpy(Name,FullName);
      }

    QuoteSpace(Name);
*/
    CtrlObject->CmdLine.ExecString(Name,AlwaysWaitFinish,2,FALSE);
    if (!AlwaysWaitFinish)
    {
      char QuotedName[2*NM];
      strcpy(QuotedName,Name);
      QuoteSpace(QuotedName);
      CtrlObject->CmdHistory->AddToHistory(QuotedName);
    }
  }
  return(TRUE);
}


void ProcessExternal(char *Command,char *Name,char *ShortName,int AlwaysWaitFinish)
{
  char ExecStr[512],FullExecStr[512];
  char ListName[NM*2],ShortListName[NM*2];
  char FullName[NM],FullShortName[NM];
  strcpy(ExecStr,Command);
  strcpy(FullExecStr,Command);
  {
    int PreserveLFN=SubstFileName(ExecStr,Name,ShortName,ListName,ShortListName);
    PreserveLongName PreserveName(ShortName,PreserveLFN);

//    ConvertNameToFull(Name,FullName, sizeof(FullName));
    if (ConvertNameToFull(Name,FullName, sizeof(FullName)) >= sizeof(FullName)){
      return;
    }
    ConvertNameToShort(FullName,FullShortName);
    SubstFileName(FullExecStr,FullName,FullShortName,ListName,ShortListName);
    CtrlObject->ViewHistory->AddToHistory(FullExecStr,MSG(MHistoryExt),AlwaysWaitFinish+2);

    if (*ExecStr!='@')
      CtrlObject->CmdLine.ExecString(ExecStr,AlwaysWaitFinish);
    else
    {
      SaveScreen SaveScr;
      CtrlObject->LeftPanel->CloseFile();
      CtrlObject->RightPanel->CloseFile();
      Execute(ExecStr+1,AlwaysWaitFinish);
    }
  }
  if (*ListName)
    remove(ListName);
  if (ListName[NM])
    remove(ListName+NM);
  if (*ShortListName)
    remove(ShortListName);
  if (ShortListName[NM])
    remove(ShortListName+NM);
}


int SubstFileName(char *Str,char *Name,char *ShortName,
                  char *ListName,char *ShortListName,int IgnoreInput,
                  char *CmdLineDir)
{
  char TmpStr[4096],NameOnly[NM],ShortNameOnly[NM],*CurStr,*ChPtr;
  char QuotedName[NM+2],QuotedShortName[NM+2];
  char AnotherName[NM],AnotherShortName[NM];
  char AnotherNameOnly[NM],AnotherShortNameOnly[NM];
  char AnotherQuotedName[NM+2],AnotherQuotedShortName[NM+2];
  char CmdDir[NM];

  if (CmdLineDir!=NULL)
    strcpy(CmdDir,CmdLineDir);
  else
    CtrlObject->CmdLine.GetCurDir(CmdDir);

  int PreserveLFN=FALSE;
  *TmpStr=0;
  if (ListName!=NULL)
  {
    *ListName=0;
    ListName[NM]=0;
  }
  if (ShortListName!=NULL)
  {
    *ShortListName=0;
    ShortListName[NM]=0;
  }

  strcpy(NameOnly,Name);
  if ((ChPtr=strrchr(NameOnly,'.'))!=NULL)
    *ChPtr=0;
  strcpy(ShortNameOnly,ShortName);
  if ((ChPtr=strrchr(ShortNameOnly,'.'))!=NULL)
    *ChPtr=0;
  QuoteSpace(NameOnly);
  QuoteSpace(ShortNameOnly);

  strcpy(QuotedName,Name);
  strcpy(QuotedShortName,ShortName);
  QuoteSpace(QuotedName);
  QuoteSpace(QuotedShortName);

  Panel *AnotherPanel=CtrlObject->GetAnotherPanel(CtrlObject->ActivePanel);

  AnotherPanel->GetCurName(AnotherName,AnotherShortName);
  strcpy(AnotherNameOnly,AnotherName);
  if ((ChPtr=strrchr(AnotherNameOnly,'.'))!=NULL)
    *ChPtr=0;
  strcpy(AnotherShortNameOnly,AnotherShortName);
  if ((ChPtr=strrchr(AnotherShortNameOnly,'.'))!=NULL)
    *ChPtr=0;
  strcpy(AnotherQuotedName,AnotherName);
  strcpy(AnotherQuotedShortName,AnotherShortName);
  QuoteSpace(AnotherQuotedName);
  QuoteSpace(AnotherQuotedShortName);

  int SkipQuotes=FALSE;
  CurStr=Str;
//  SysLog("SubstrFileName: CurStr=[%s]",CurStr);
  while (*CurStr)
  {
    int PassivePanel=FALSE;
    if (strncmp(CurStr,"!#",2)==0)
    {
      CurStr+=2;
      PassivePanel=TRUE;
//      SysLog("PassivePanel=TRUE");
    }
    if (*CurStr=='\"')
      SkipQuotes=(CurStr==Str);
    if (!SkipQuotes)
    {
      /* $ 21.07.2000 IG
         Bug 15 (не работала комманда executable.exe !.!?ext:?!)
      */
      if (strncmp(CurStr,"!!",2)==0 && CurStr[2] != '?')
      /* IG $ */
      {
        strcat(TmpStr,"!");
        CurStr+=2;
        continue;
      }
      /* $ 21.07.2000 IG
         Bug 15 (не работала комманда executable.exe !.!?ext:?!)
      */
      if (strncmp(CurStr,"!.!",3)==0 && CurStr[3] != '?')
      /* IG $ */
      {
        strcat(TmpStr,PassivePanel ? AnotherQuotedName:QuotedName);
        CurStr+=3;
        continue;
      }
      if (strncmp(CurStr,"!~",2)==0)
      {
        strcat(TmpStr,PassivePanel ? AnotherShortNameOnly:ShortNameOnly);
        CurStr+=2;
        continue;
      }
      if (strncmp(CurStr,"!?",2)==0 && strchr(CurStr+2,'!')!=NULL)
      {
        char *NewCurStr=strchr(CurStr+2,'!')+1;
        strncat(TmpStr,CurStr,NewCurStr-CurStr);
        CurStr=NewCurStr;
        continue;
      }
      /* $ 21.07.2000 IG
         Bug 15 (не работала комманда executable.exe !.!?ext:?!)
      */
      if (strncmp(CurStr,"!@!",3)==0 && ListName!=NULL && CurStr[3] != '?')
      /* IG $ */
      {
        /* $ 02.09.2000 tran
           !@!, !#!@! bug */
        if ( PassivePanel && ( ListName[NM] || AnotherPanel->MakeListFile(ListName+NM,FALSE)))
        {
          strcat(TmpStr,ListName+NM);
        }
        if ( !PassivePanel && (*ListName || CtrlObject->ActivePanel->MakeListFile(ListName,FALSE)))
        {
          strcat(TmpStr,ListName);
        }
        /* tran $ */
        CurStr+=3;
        continue;
      }
      /* $ 21.07.2000 IG
         Bug 15 (не работала комманда executable.exe !.!?ext:?!)
      */
      if (strncmp(CurStr,"!$!",3)==0 && ShortListName!=NULL && CurStr[3] != '?')
      /* IG $ */
      {
        /* $ 02.09.2000 tran
           !@!, !#!@! bug */
        if ( PassivePanel && (ShortListName[NM] || AnotherPanel->MakeListFile(ShortListName+NM,TRUE)))
        {
          /* $ 01.11.2000 IS
             Имя файла в данном случае должно быть коротким
          */
          ConvertNameToShort(ShortListName+NM,ShortListName+NM);
          /* IS $ */
          strcat(TmpStr,ShortListName+NM);
        }
        if ( !PassivePanel && (*ShortListName || CtrlObject->ActivePanel->MakeListFile(ShortListName,TRUE)))
        {
          /* $ 01.11.2000 IS
             Имя файла в данном случае должно быть коротким
          */
          ConvertNameToShort(ShortListName,ShortListName);
          /* IS $ */
          strcat(TmpStr,ShortListName);
        }
        /* tran $ */
        CurStr+=3;
        continue;
      }
      /* $ 21.07.2000 IG
         Bug 15 (не работала комманда executable.exe !.!?ext:?!)
      */
      if (strncmp(CurStr,"!-!",3)==0 && CurStr[3] != '?')
      /* IG $ */
      {
        strcat(TmpStr,PassivePanel ? AnotherQuotedShortName:QuotedShortName);
        CurStr+=3;
        continue;
      }
      /* $ 21.07.2000 IG
         Bug 15 (не работала комманда executable.exe !.!?ext:?!)
      */
      if (strncmp(CurStr,"!+!",3)==0 && CurStr[3] != '?')
      /* IG $ */
      {
        strcat(TmpStr,PassivePanel ? AnotherQuotedShortName:QuotedShortName);
        CurStr+=3;
        PreserveLFN=TRUE;
        continue;
      }
      if (strncmp(CurStr,"!:",2)==0)
      {
        char CurDir[NM];
        if (*Name && Name[1]==':')
          strcpy(CurDir,Name);
        else
          if (PassivePanel)
            AnotherPanel->GetCurDir(CurDir);
          else
            strcpy(CurDir,CmdDir);
        CurDir[2]=0;
        if (*CurDir && CurDir[1]!=':')
          *CurDir=0;
        strcat(TmpStr,CurDir);
        CurStr+=2;
        continue;
      }
      /* $ 21.07.2000 IG
         Bug 15 (не работала комманда executable.exe !.!?ext:?!)
      */
      if (strncmp(CurStr,"!\\!.!",5)==0 && CurStr[5] != '?')
      /* IG $ */
      {
        char CurDir[NM];
        char *FileName=PassivePanel ? AnotherName:Name;
        if (strpbrk(FileName,"\\:")==NULL)
        {
          if (PassivePanel)
            AnotherPanel->GetCurDir(CurDir);
          else
            strcpy(CurDir,CmdDir);
          AddEndSlash(CurDir);
        }
        else
          *CurDir=0;
        strcat(CurDir,FileName);
        QuoteSpace(CurDir);
        CurStr+=5;
        strcat(TmpStr,CurDir);
        continue;
      }
      if (strncmp(CurStr,"!\\",2)==0)
      {
        char CurDir[NM];
        if (PassivePanel)
          AnotherPanel->GetCurDir(CurDir);
        else
          strcpy(CurDir,CmdDir);
        AddEndSlash(CurDir);
        CurStr+=2;
        if (*CurStr=='!')
        {
          strcpy(QuotedName,Name);
          strcpy(QuotedShortName,ShortName);
          if (strpbrk(Name,"\\:")!=NULL)
            *CurDir=0;
        }
        strcat(TmpStr,CurDir);
        continue;
      }
      if (strncmp(CurStr,"!/",2)==0)
      {
        char CurDir[NM];
        if (PassivePanel)
          AnotherPanel->GetCurDir(CurDir);
        else
          strcpy(CurDir,CmdDir);
        ConvertNameToShort(CurDir,CurDir);
        AddEndSlash(CurDir);
        CurStr+=2;
        if (*CurStr=='!')
        {
          strcpy(QuotedName,Name);
          strcpy(QuotedShortName,ShortName);
          if (strpbrk(Name,"\\:")!=NULL)
          {
            if (PointToName(ShortName)==ShortName)
            {
              strcpy(QuotedShortName,CurDir);
              AddEndSlash(QuotedShortName);
              strcat(QuotedShortName,ShortName);
            }
            *CurDir=0;
          }
        }
        strcat(TmpStr,CurDir);
        continue;
      }
      if (*CurStr=='!')
      {
        strcat(TmpStr,PointToName(PassivePanel ? AnotherNameOnly:NameOnly));
        CurStr++;
        continue;
      }
    }
    strncat(TmpStr,CurStr,1);
    CurStr++;
  }
  if (!IgnoreInput)
    ReplaceVariables(TmpStr);
  strcpy(Str,TmpStr);
  return(PreserveLFN);
}


void EditFileTypes(int MenuPos)
{
  struct MenuItem TypesMenuItem;
  int NumLine=0;

  TypesMenuItem.Checked=TypesMenuItem.Separator=*TypesMenuItem.UserData=TypesMenuItem.UserDataSize=0;
  VMenu TypesMenu(MSG(MAssocTitle),NULL,0,ScrY-4);
  TypesMenu.SetHelp("FileAssoc");
  TypesMenu.SetFlags(MENU_WRAPMODE);
  TypesMenu.SetPosition(-1,-1,0,0);
  TypesMenu.SetBottomTitle(MSG(MAssocBottom));

  int DizWidth=GetDescriptionWidth();

  while (1)
  {
    char RegKey[80],Mask[512],MenuText[512];
    sprintf(RegKey,"Associations\\Type%d",NumLine);
    if (!GetRegKey(RegKey,"Mask",Mask,"",sizeof(Mask)))
      break;
    if (DizWidth==0)
      *MenuText=0;
    else
    {
      char Title[256],Description[128];
      GetRegKey(RegKey,"Description",Description,"",sizeof(Description));
      if (*Description)
        strcpy(Title,Description);
      else
        *Title=0;
      int Ampersand=strchr(Title,'&')!=NULL;
      sprintf(MenuText,"%-*.*s │ ",DizWidth+Ampersand,DizWidth+Ampersand,Title);
    }
    TruncStr(Mask,Min(ScrX,sizeof(TypesMenuItem.Name)-1)-DizWidth-14);
    strcat(MenuText,Mask);
    TruncStr(MenuText,sizeof(TypesMenuItem.Name)-1);
    strcpy(TypesMenuItem.Name,MenuText);
    TypesMenuItem.Selected=(NumLine==MenuPos);
    TypesMenu.AddItem(&TypesMenuItem);
    NumLine++;
  }
  *TypesMenuItem.Name=0;
  TypesMenuItem.Selected=(NumLine==MenuPos);
  TypesMenu.AddItem(&TypesMenuItem);

  {
    TypesMenu.Show();
    while (1)
    {
      while (!TypesMenu.Done())
      {
        int SelectPos=TypesMenu.GetSelectPos();
        switch(TypesMenu.ReadInput())
        {
          case KEY_DEL:
            if (SelectPos<NumLine)
              if (DeleteTypeRecord(SelectPos))
              {
                TypesMenu.Hide();
                EditFileTypes(SelectPos);
                return;
              }
            break;
          case KEY_INS:
            if (EditTypeRecord(SelectPos,NumLine,1))
            {
              TypesMenu.Hide();
              EditFileTypes(SelectPos);
              return;
            }
            break;
          case KEY_ENTER:
          case KEY_F4:
            if (SelectPos<NumLine)
              if (EditTypeRecord(SelectPos,NumLine,0))
              {
                TypesMenu.Hide();
                EditFileTypes(SelectPos);
                return;
              }
            break;
          default:
            TypesMenu.ProcessInput();
            break;
        }
      }
      if (TypesMenu.GetExitCode()!=-1)
      {
        TypesMenu.ClearDone();
        TypesMenu.WriteInput(KEY_F4);
        continue;
      }
      break;
    }
  }
}


int DeleteTypeRecord(int DeletePos)
{
  char RecText[200],ItemName[200],RegKey[80];
  sprintf(RegKey,"Associations\\Type%d",DeletePos);
  GetRegKey(RegKey,"Mask",RecText,"",sizeof(RecText));
  sprintf(ItemName,"\"%s\"",RecText);
  if (Message(MSG_WARNING,2,MSG(MAssocTitle),MSG(MAskDelAssoc),
              ItemName,MSG(MDelete),MSG(MCancel))!=0)
    return(FALSE);
  DeleteKeyRecord("Associations\\Type%d",DeletePos);
  return(TRUE);
}


int EditTypeRecord(int EditPos,int TotalRecords,int NewRec)
{
  const char *HistoryName="Masks";
  static struct DialogData EditDlgData[]={
    DI_DOUBLEBOX,3,1,72,15,0,0,0,0,(char *)MFileAssocTitle,
    DI_TEXT,5,2,0,0,0,0,0,0,(char *)MFileAssocMasks,
    DI_EDIT,5,3,70,3,1,(DWORD)HistoryName,DIF_HISTORY,0,"",
    DI_TEXT,5,4,0,0,0,0,0,0,(char *)MFileAssocDescr,
    DI_EDIT,5,5,70,3,0,0,0,0,"",
    DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_TEXT,5,7,0,0,0,0,0,0,(char *)MFileAssocExec,
    DI_EDIT,5,8,70,3,0,0,0,0,"",
    DI_TEXT,5,9,0,0,0,0,0,0,(char *)MFileAssocView,
    DI_EDIT,5,10,70,3,0,0,0,0,"",
    DI_TEXT,5,11,0,0,0,0,0,0,(char *)MFileAssocEdit,
    DI_EDIT,5,12,70,3,0,0,0,0,"",
    DI_TEXT,3,13,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,14,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,14,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(EditDlgData,EditDlg);

  char RegKey[80];
  sprintf(RegKey,"Associations\\Type%d",EditPos);
  if (!NewRec)
  {
    GetRegKey(RegKey,"Mask",EditDlg[2].Data,"",sizeof(EditDlg[2].Data));
    GetRegKey(RegKey,"Description",EditDlg[4].Data,"",sizeof(EditDlg[4].Data));
    GetRegKey(RegKey,"Execute",EditDlg[7].Data,"",sizeof(EditDlg[7].Data));
    GetRegKey(RegKey,"View",EditDlg[9].Data,"",sizeof(EditDlg[9].Data));
    GetRegKey(RegKey,"Edit",EditDlg[11].Data,"",sizeof(EditDlg[11].Data));
  }

  {
    Dialog Dlg(EditDlg,sizeof(EditDlg)/sizeof(EditDlg[0]));
    Dlg.SetHelp("FileAssoc");
    Dlg.SetPosition(-1,-1,76,17);
    Dlg.Process();
    if (Dlg.GetExitCode()!=13 || *EditDlg[2].Data==0)
      return(FALSE);
  }

  if (NewRec)
    InsertKeyRecord("Associations\\Type%d",EditPos,TotalRecords);
  SetRegKey(RegKey,"Mask",EditDlg[2].Data);
  SetRegKey(RegKey,"Description",EditDlg[4].Data);
  SetRegKey(RegKey,"Execute",EditDlg[7].Data);
  SetRegKey(RegKey,"View",EditDlg[9].Data);
  SetRegKey(RegKey,"Edit",EditDlg[11].Data);

  return(TRUE);
}


int GetDescriptionWidth()
{
  int Width=0,NumLine=0;
  while (1)
  {
    char RegKey[80],Mask[512],Description[128];
    sprintf(RegKey,"Associations\\Type%d",NumLine);
    if (!GetRegKey(RegKey,"Mask",Mask,"",sizeof(Mask)))
      break;
    GetRegKey(RegKey,"Description",Description,"",sizeof(Description));
    int CurWidth=HiStrlen(Description);
    if (CurWidth>Width)
      Width=CurWidth;
    NumLine++;
  }
  if (Width>ScrX/2)
    Width=ScrX/2;
  return(Width);
}


void ReplaceVariables(char *Str)
{
  const int MaxSize=20;
  char *StartStr=Str;

  if (*Str=='\"')
    while (*Str && *Str!='\"')
      Str++;

  struct DialogItem *DlgData=NULL;
  int DlgSize=0;
  int StrPos[64],StrPosSize=0;

  while (*Str && DlgSize<MaxSize)
  {
    if (*(Str++)!='!')
      continue;
    if (*(Str++)!='?')
      continue;
    if (strchr(Str,'!')==NULL)
      return;
    StrPos[StrPosSize++]=Str-StartStr-2;
    DlgData=(struct DialogItem *)realloc(DlgData,(DlgSize+2)*sizeof(*DlgData));
    memset(&DlgData[DlgSize],0,2*sizeof(*DlgData));
    DlgData[DlgSize].Type=DI_TEXT;
    DlgData[DlgSize].X1=5;
    DlgData[DlgSize].Y1=DlgSize+2;
    DlgData[DlgSize+1].Type=DI_EDIT;
    DlgData[DlgSize+1].X1=5;
    DlgData[DlgSize+1].X2=70;
    DlgData[DlgSize+1].Y1=DlgSize+3;
    DlgData[DlgSize+1].Flags|=DIF_HISTORY;

    char HistoryName[MaxSize][20];
    int HistoryNumber=DlgSize/2;
    sprintf(HistoryName[HistoryNumber],"UserVar%d",HistoryNumber);
    /* $ 01.08.2000 SVS
       + .History
    */
    DlgData[DlgSize+1].Selected=(int)HistoryName[HistoryNumber];
    /* SVS $*/

    if (DlgSize==0)
    {
      DlgData[DlgSize+1].Focus=1;
      DlgData[DlgSize+1].DefaultButton=1;
    }
    char Title[256];
    strcpy(Title,Str);
    *strchr(Title,'!')=0;
    Str+=strlen(Title)+1;
    char *SrcText=strchr(Title,'?');
    if (SrcText!=NULL)
    {
      *SrcText=0;
      strcpy(DlgData[DlgSize+1].Data,SrcText+1);
    }
    strcpy(DlgData[DlgSize].Data,Title);
    /* $ 01.08.2000 SVS
       "расширяем" заголовок
    */
    ExpandEnvironmentStr(DlgData[DlgSize].Data,DlgData[DlgSize].Data,sizeof(DlgData[DlgSize].Data));
    /* SVS $*/
    DlgSize+=2;
  }
  if (DlgSize==0)
    return;
  DlgData=(struct DialogItem *)realloc(DlgData,(DlgSize+1)*sizeof(*DlgData));
  memset(&DlgData[DlgSize],0,sizeof(*DlgData));
  DlgData[DlgSize].Type=DI_DOUBLEBOX;
  DlgData[DlgSize].X1=3;
  DlgData[DlgSize].Y1=1;
  DlgData[DlgSize].X2=72;
  DlgData[DlgSize].Y2=DlgSize+2;
  DlgSize++;
  Dialog Dlg(DlgData,DlgSize);
  Dlg.SetPosition(-1,-1,76,DlgSize+3);
  Dlg.Process();
  if (Dlg.GetExitCode()==-1)
  {
    /* $ 13.07.2000 SVS
       запрос был по realloc
    */
    free(DlgData);
    /* SVS $ */
    *StartStr=0;
    return;
  }
  char TmpStr[4096];
  *TmpStr=0;
  for (Str=StartStr;*Str!=0;Str++)
  {
    int Replace=-1;
    for (int I=0;I<StrPosSize;I++)
      if (Str-StartStr==StrPos[I])
      {
        Replace=I;
        break;
      }
    if (Replace!=-1)
    {
      strcat(TmpStr,DlgData[Replace*2+1].Data);
      Str=strchr(Str+1,'!');
    }
    else
      strncat(TmpStr,Str,1);
  }
  strcpy(StartStr,TmpStr);
  /* $ 01.08.2000 SVS
     после "жмаканья" Enter "расширяем" данные
  */
  ExpandEnvironmentStr(TmpStr,StartStr,sizeof(DlgData[0].Data));
  /* SVS $ */
  /* $ 13.07.2000 SVS
     запрос был по realloc
  */
  free(DlgData);
  /* SVS $ */
}
