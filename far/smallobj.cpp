ChangePriority::ChangePriority(int NewPriority)
{
  SavePriority=GetThreadPriority(GetCurrentThread());
  SetThreadPriority(GetCurrentThread(),NewPriority);
}


ChangePriority::~ChangePriority()
{
  SetThreadPriority(GetCurrentThread(),SavePriority);
}


SaveFilePos::SaveFilePos(FILE *SaveFile)
{
  SaveFilePos::SaveFile=SaveFile;
  SavePos=ftell(SaveFile);
}


SaveFilePos::~SaveFilePos()
{
  fseek(SaveFile,SavePos,SEEK_SET);
}


RedrawDesktop::RedrawDesktop()
{
  LeftVisible=CtrlObject->LeftPanel->IsVisible();
  RightVisible=CtrlObject->RightPanel->IsVisible();
  CtrlObject->LeftPanel->Hide();
  CtrlObject->RightPanel->Hide();
  CtrlObject->MainKeyBar.Hide();
  CtrlObject->TopMenuBar.Hide();
}


RedrawDesktop::~RedrawDesktop()
{
  if (Opt.ShowKeyBar)
    CtrlObject->MainKeyBar.Show();
  if (Opt.ShowMenuBar)
    CtrlObject->TopMenuBar.Show();
  CtrlObject->CmdLine.Show();
  int RightType=CtrlObject->RightPanel->GetType();
  if (RightVisible && RightType!=QVIEW_PANEL)
    CtrlObject->RightPanel->Show();
  if (LeftVisible)
    CtrlObject->LeftPanel->Show();
  if (RightVisible && RightType==QVIEW_PANEL)
    CtrlObject->RightPanel->Show();
}


PreserveLongName::PreserveLongName(char *ShortName,int Preserve)
{
  PreserveLongName::Preserve=Preserve;
  if (Preserve)
  {
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    FindHandle=FindFirstFile(ShortName,&FindData);
    FindClose(FindHandle);
    if (FindHandle==INVALID_HANDLE_VALUE)
      *SaveLongName=0;
    else
      strcpy(SaveLongName,FindData.cFileName);
    strcpy(SaveShortName,ShortName);
  }
}


PreserveLongName::~PreserveLongName()
{
  if (Preserve && GetFileAttributes(SaveShortName)!=0xFFFFFFFF)
  {
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    FindHandle=FindFirstFile(SaveShortName,&FindData);
    FindClose(FindHandle);
    if (FindHandle==INVALID_HANDLE_VALUE ||
        strcmp(SaveLongName,FindData.cFileName)!=0)
    {
      char NewName[NM];
      strcpy(NewName,SaveShortName);
      strcpy(PointToName(NewName),SaveLongName);
      rename(SaveShortName,NewName);
    }
  }
}


GetFileString::GetFileString(FILE *SrcFile)
{
  Str=new char[1024];
  StrLength=1024;
  GetFileString::SrcFile=SrcFile;
  ReadPos=ReadSize=0;
}


GetFileString::~GetFileString()
{
  delete Str;
}


int GetFileString::GetString(char **DestStr,int &Length)
{
  Length=0;
  int CurLength=0,ExitCode=1;
  while (1)
  {
    if (ReadPos>=ReadSize)
    {
      if ((ReadSize=fread(ReadBuf,1,sizeof(ReadBuf),SrcFile))==0)
      {
        if (CurLength==0)
          ExitCode=0;
        break;
      }
      ReadPos=0;
    }
    int Ch=ReadBuf[ReadPos];
    if (Ch!='\n' && CurLength>0 && Str[CurLength-1]=='\r')
      break;
    ReadPos++;
    if (CurLength>=StrLength-1)
    {
      StrLength+=1024;
      char *NewStr=(char *)realloc(Str,StrLength);
      if (NewStr==NULL)
        return(-1);
      Str=NewStr;
    }
    Str[CurLength++]=Ch;
    if (Ch=='\n')
      break;
  }
  Str[CurLength]=0;
  *DestStr=Str;
  Length=CurLength;
  return(ExitCode);
}


LockScreen::LockScreen()
{
  ScrBuf.Lock();
}


LockScreen::~LockScreen()
{
  ScrBuf.Unlock();
  ScrBuf.Flush();
}


ChangeMacroMode::ChangeMacroMode(int NewMode)
{
  if (CtrlObject!=NULL)
  {
    PrevMacroMode=CtrlObject->Macro.GetMode();
    CtrlObject->Macro.SetMode(NewMode);
  }
  else
    PrevMacroMode=MACRO_SHELL;
}


ChangeMacroMode::~ChangeMacroMode()
{
  if (CtrlObject!=NULL)
    CtrlObject->Macro.SetMode(PrevMacroMode);
}
