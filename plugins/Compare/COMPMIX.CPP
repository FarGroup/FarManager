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


int LocalStricmp(char *Str1,char *Str2)
{
  char AnsiStr1[8192],AnsiStr2[8192];
  OemToChar(Str1,AnsiStr1);
  OemToChar(Str2,AnsiStr2);
  CharLower(AnsiStr1);
  CharLower(AnsiStr2);
  return(strcmp(AnsiStr1,AnsiStr2));
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
    if (rec.EventType==KEY_EVENT)
      if (rec.Event.KeyEvent.wVirtualKeyCode==VK_ESCAPE)
        ExitCode=TRUE;
  }
  return(ExitCode);
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
  if (MaxLength<0)
    MaxLength=0;
  if ((Length=strlen(Str))>MaxLength)
    if (MaxLength>3)
    {
      char TmpStr[1024];
      strcpy(TmpStr,"...");
      strcpy(TmpStr+3,Str+Length-MaxLength+3);
      strcpy(Str,TmpStr);
    }
    else
      Str[MaxLength]=0;
  return(Str);
}
