char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}


void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item,
                    int ItemsNumber)
{
  for (int I=0;I<ItemsNumber;I++)
  {
    Item[I].Type=Init[I].Type;
    Item[I].X1=Init[I].X1;
    Item[I].Y1=Init[I].Y1;
    Item[I].X2=Init[I].X2;
    Item[I].Y2=Init[I].Y2;
    Item[I].Focus=Init[I].Focus;
    Item[I].Selected=Init[I].Selected;
    Item[I].Flags=Init[I].Flags;
    Item[I].DefaultButton=Init[I].DefaultButton;
    if ((unsigned int)Init[I].Data<2000)
      strcpy(Item[I].Data,GetMsg((unsigned int)Init[I].Data));
    else
      strcpy(Item[I].Data,Init[I].Data);
  }
}


void AddEndSlash(char *Path)
{
  int Length=strlen(Path);
  if (Length==0 || Path[Length-1]!='\\' && Path[Length-1]!='/')
    strcat(Path,"\\");
}


char* TruncStr(char *Str,int MaxLength)
{
  int Length;
  if ((Length=strlen(Str))>MaxLength)
    if (MaxLength>3)
    {
      char *TmpStr=new char[MaxLength+5];
      sprintf(TmpStr,"...%s",Str+Length-MaxLength+3);
      strcpy(Str,TmpStr);
      delete TmpStr;
    }
    else
      Str[MaxLength]=0;
  return(Str);
}


void CenterStr(char *Src,char *Dest,int Length)
{
  char TempSrc[512];
  int SrcLength=strlen(Src);
  strcpy(TempSrc,Src);
  if (SrcLength>=Length)
    strcpy(Dest,TempSrc);
  else
  {
    int Space=(Length-SrcLength)/2;
    sprintf(Dest,"%*s%s%*s",Space,"",TempSrc,Length-Space-SrcLength,"");
  }
}


char* PointToName(char *Path)
{
  char *NamePtr=Path;
  while (*Path)
  {
    if (*Path=='\\' || *Path=='/' || *Path==':')
      NamePtr=Path+1;
    Path++;
  }
  return(NamePtr);
}


int CheckForEsc()
{
  int ExitCode=FALSE;
  while (1)
  {
    INPUT_RECORD rec;
    static HANDLE hConInp=GetStdHandle(STD_INPUT_HANDLE);
    DWORD ReadCount;
    PeekConsoleInput(hConInp,&rec,1,&ReadCount);
    if (ReadCount==0)
      break;
    ReadConsoleInput(hConInp,&rec,1,&ReadCount);
    if (rec.EventType==KEY_EVENT && rec.Event.KeyEvent.bKeyDown)
      if (rec.Event.KeyEvent.wVirtualKeyCode==VK_ESCAPE)
        ExitCode=TRUE;
  }
  return(ExitCode);
}


int IsCaseMixed(char *Str)
{
  char AnsiStr[1024];
  OemToChar(Str,AnsiStr);
  while (*Str && !IsCharAlpha(*Str))
    Str++;
  int Case=IsCharLower(*Str);
  while (*(Str++))
    if (IsCharAlpha(*Str) && IsCharLower(*Str)!=Case)
      return(TRUE);
  return(FALSE);
}


void LocalLower(char *Str)
{
  OemToChar(Str,Str);
  CharLower(Str);
  CharToOem(Str,Str);
}


int LocalStricmp(char *Str1,char *Str2)
{
  char AnsiStr1[8192],AnsiStr2[8192];
  OemToChar(Str1,AnsiStr1);
  OemToChar(Str2,AnsiStr2);
  CharLower(AnsiStr1);
  CharLower(AnsiStr2);
  return(strcmp(AnsiStr1,AnsiStr2));
}


int CopyToClipboard(char *Data)
{
  long DataSize;
  if (Data!=NULL && (DataSize=strlen(Data))!=0)
  {
    HGLOBAL hData;
    void *GData;
    if (!OpenClipboard(NULL))
      return(FALSE);
    EmptyClipboard();
    int BufferSize=DataSize+1;
    if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize))!=NULL)
      if ((GData=GlobalLock(hData))!=NULL)
      {
        memcpy(GData,Data,DataSize+1);
        GlobalUnlock(hData);
        SetClipboardData(CF_OEMTEXT,(HANDLE)hData);
      }
    if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize))!=NULL)
      if ((GData=GlobalLock(hData))!=NULL)
      {
        memcpy(GData,Data,DataSize+1);
        OemToChar((LPCSTR)GData,(LPTSTR)GData);
        GlobalUnlock(hData);
        SetClipboardData(CF_TEXT,(HANDLE)hData);
      }
    if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize*2))!=NULL)
      if ((GData=GlobalLock(hData))!=NULL)
      {
        MultiByteToWideChar(CP_OEMCP,0,Data,-1,(LPWSTR)GData,BufferSize);
        GlobalUnlock(hData);
        SetClipboardData(CF_UNICODETEXT,(HANDLE)hData);
      }
    CloseClipboard();
  }
  return(TRUE);
}


int ToPercent(long N1,long N2)
{
  if (N1 > 10000)
  {
    N1/=100;
    N2/=100;
  }
  if (N2==0)
    return(0);
  if (N2<N1)
    return(100);
  return((int)(N1*100/N2));
}


long filelen(FILE *FPtr)
{
  long SavePos=ftell(FPtr);
  fseek(FPtr,0,SEEK_END);
  long FileSize=ftell(FPtr);
  fseek(FPtr,SavePos,SEEK_SET);
  return(FileSize);
}

