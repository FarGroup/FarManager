/*
message.cpp

Вывод MessageBox

*/

/* Revision: 1.03 02.02.2001 $ */

/*
Modify:
  02.02.2001 SVS
    ! Забыли сделать CharToOem в GetErrorString...
  27.01.2001 VVM
    + Если GetErrorString не распознает ошибку - пытается узнать у системы
  29.08.2000 SVS
    + Дополнительный параметр у Message* - номер плагина.
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

static int MessageX1,MessageY1,MessageX2,MessageY2;
static char MsgHelpTopic[80];

int Message(int Flags,int Buttons,char *Title,char *Str1,char *Str2,
            char *Str3,char *Str4,int PluginNumber)
{
  return(Message(Flags,Buttons,Title,Str1,Str2,Str3,Str4,NULL,NULL,NULL,
                 NULL,NULL,NULL,NULL,NULL,NULL,NULL,PluginNumber));
}

int Message(int Flags,int Buttons,char *Title,char *Str1,char *Str2,
            char *Str3,char *Str4,char *Str5,char *Str6,char *Str7,int PluginNumber)
{
  return(Message(Flags,Buttons,Title,Str1,Str2,Str3,Str4,Str5,Str6,Str7,
                 NULL,NULL,NULL,NULL,NULL,NULL,NULL,PluginNumber));
}


int Message(int Flags,int Buttons,char *Title,char *Str1,char *Str2,
            char *Str3,char *Str4,char *Str5,char *Str6,char *Str7,
            char *Str8,char *Str9,char *Str10,int PluginNumber)
{
  return(Message(Flags,Buttons,Title,Str1,Str2,Str3,Str4,Str5,Str6,Str7,Str8,
                 Str9,Str10,NULL,NULL,NULL,NULL,PluginNumber));
}


int Message(int Flags,int Buttons,char *Title,char *Str1,char *Str2,
            char *Str3,char *Str4,char *Str5,char *Str6,char *Str7,
            char *Str8,char *Str9,char *Str10,char *Str11,char *Str12,
            char *Str13,char *Str14,int PluginNumber)
{
  char TmpStr[256],ErrStr[256],HelpTopic[80];
  char *Str[14],*Btn[14];
  int X1,Y1,X2,Y2;
  int Length,MaxLength,BtnLength,StrCount,I;

  strcpy(HelpTopic,MsgHelpTopic);
  *MsgHelpTopic=0;

  Str[0]=Str1;   Str[1]=Str2;   Str[2]=Str3;   Str[3]=Str4;
  Str[4]=Str5;   Str[5]=Str6;   Str[6]=Str7;   Str[7]=Str8;
  Str[8]=Str9;   Str[9]=Str10;  Str[10]=Str11; Str[11]=Str12;
  Str[12]=Str13; Str[13]=Str14;

  Btn[0]=Str1;   Btn[1]=Str2;   Btn[2]=Str3;   Btn[3]=Str4;
  Btn[4]=Str5;   Btn[5]=Str6;   Btn[6]=Str7;   Btn[7]=Str8;
  Btn[8]=Str9;   Btn[9]=Str10;  Btn[10]=Str11; Btn[11]=Str12;
  Btn[12]=Str13; Btn[13]=Str14;

  StrCount=0;
  while (StrCount<sizeof(Str)/sizeof(Str[0]) && Str[StrCount]!=NULL)
    StrCount++;

  for (I=0;I<Buttons;I++)
    Btn[I]=Btn[StrCount-Buttons+I];

  StrCount-=Buttons;

  if (Flags & MSG_ERRORTYPE && GetErrorString(ErrStr, sizeof(ErrStr)))
  {
    for (int I=sizeof(Str)/sizeof(Str[0])-1;I>0;I--)
      Str[I]=Str[I-1];
    Str[0]=ErrStr;
    StrCount++;
  }

  for (BtnLength=0,I=0;I<Buttons;I++)
    BtnLength+=HiStrlen(Btn[I])+2;

  for (MaxLength=BtnLength,I=0;I<StrCount;I++)
  {
    if ((Length=strlen(Str[I]))>MaxLength)
      MaxLength=Length;
  }

  if (MaxLength<strlen(Title)+2)
    MaxLength=strlen(Title)+2;

  if (MaxLength>ScrX-15)
    MaxLength=ScrX-15;

  MessageX1=X1=(ScrX-MaxLength)/2-4;
  MessageX2=X2=X1+MaxLength+9;
  MessageY1=Y1=(ScrY-StrCount)/2-4;
  if (Flags & MSG_DOWN)
  {
    int NewY=ScrY/2-4;
    if (Y1+StrCount+3<ScrY && NewY>Y1+2)
      Y1=NewY;
    else
      Y1+=2;
    MessageY1=Y1;
  }
  MessageY2=Y2=Y1+StrCount+3;

  if (Buttons>0)
  {
    static struct DialogData MsgDlgData[]={
      DI_DOUBLEBOX,0,0,0,0,0,0,0,0,"",

      DI_TEXT,0,0,0,0,0,0,DIF_SHOWAMPERSAND,0,"",
      DI_TEXT,0,0,0,0,0,0,DIF_SHOWAMPERSAND,0,"",
      DI_TEXT,0,0,0,0,0,0,DIF_SHOWAMPERSAND,0,"",
      DI_TEXT,0,0,0,0,0,0,DIF_SHOWAMPERSAND,0,"",
      DI_TEXT,0,0,0,0,0,0,DIF_SHOWAMPERSAND,0,"",
      DI_TEXT,0,0,0,0,0,0,DIF_SHOWAMPERSAND,0,"",
      DI_TEXT,0,0,0,0,0,0,DIF_SHOWAMPERSAND,0,"",
      DI_TEXT,0,0,0,0,0,0,DIF_SHOWAMPERSAND,0,"",
      DI_TEXT,0,0,0,0,0,0,DIF_SHOWAMPERSAND,0,"",
      DI_TEXT,0,0,0,0,0,0,DIF_SHOWAMPERSAND,0,"",
      DI_TEXT,0,0,0,0,0,0,DIF_SHOWAMPERSAND,0,"",
      DI_TEXT,0,0,0,0,0,0,DIF_SHOWAMPERSAND,0,"",

      DI_BUTTON,0,0,0,0,1,0,DIF_CENTERGROUP|DIF_NOBRACKETS,1,"",
      DI_BUTTON,0,0,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,"",
      DI_BUTTON,0,0,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,"",
      DI_BUTTON,0,0,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,"",
      DI_BUTTON,0,0,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,"",
      DI_BUTTON,0,0,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,"",
      DI_BUTTON,0,0,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,"",
      DI_BUTTON,0,0,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,"",
      DI_BUTTON,0,0,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,"",
      DI_BUTTON,0,0,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,"",
      DI_BUTTON,0,0,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,"",
      DI_BUTTON,0,0,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,"",
      DI_BUTTON,0,0,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,"",
      DI_BUTTON,0,0,0,0,0,0,DIF_CENTERGROUP|DIF_NOBRACKETS,0,""
    };
    MakeDialogItems(MsgDlgData,MsgDlg);
    int RetCode;
    MessageY2=++Y2;
    MsgDlg[0].X1=3;
    MsgDlg[0].Y1=1;
    MsgDlg[0].X2=X2-X1-3;
    MsgDlg[0].Y2=Y2-Y1-1;

    for (I=0;I<Buttons;I++)
    {
      MsgDlg[I+13].Y1=Y2-Y1-2;
      sprintf(MsgDlg[I+13].Data," %s ",Btn[I]);
    }

    strcpy(MsgDlg[0].Data,Title);

    for (I=0;I<StrCount;I++)
    {
      int CurItem=I+1;
      if (Flags & MSG_LEFTALIGN)
        MsgDlg[CurItem].X1=5;
      else
        MsgDlg[CurItem].X1=-1;
      MsgDlg[CurItem].Y1=I+2;
      strncpy(MsgDlg[CurItem].Data,Str[I],ScrX-15);
      if (*MsgDlg[CurItem].Data==1)
      {
        MsgDlg[CurItem].Flags|=DIF_BOXCOLOR|DIF_SEPARATOR;
        *MsgDlg[CurItem].Data=0;
      }
      MsgDlg[CurItem].Data[ScrX-15]=0;
    }
    Dialog Dlg(MsgDlg,Buttons+13);
    Dlg.SetPosition(X1,Y1,X2,Y2);
    if (*HelpTopic)
      Dlg.SetHelp(HelpTopic);
    /* $ 29.08.2000 SVS
       Запомним номер плагина
    */
    Dlg.SetPluginNumber(PluginNumber);
    /* SVS $ */
    if (Flags & MSG_WARNING)
      Dlg.SetWarningStyle(TRUE);
    FlushInputBuffer();
    Dlg.Process();
    RetCode=Dlg.GetExitCode();
    return((RetCode<0) ? RetCode:RetCode-13);
  }

  if (!(Flags & MSG_KEEPBACKGROUND))
  {
    SetScreen(X1,Y1,X2,Y2,' ',COL_DIALOGTEXT);
    MakeShadow(X1+2,Y2+1,X2+2,Y2+1);
    MakeShadow(X2+1,Y1+1,X2+2,Y2+1);
    Box(X1+3,Y1+1,X2-3,Y2-1,COL_DIALOGBOX,DOUBLE_BOX);
  }
  SetColor(COL_DIALOGTEXT);
  GotoXY(X1+(X2-X1-1-strlen(Title))/2,Y1+1);
  if (*Title)
    mprintf(" %s ",Title);
  for (I=0;I<StrCount;I++)
  {
    int PosX;
    if (*Str[I]==1)
    {
      GotoXY(X1+3,Y1+I+2);
      int Length=X2-X1-5;
      if (Length>1)
      {
        char Separator[500];
        memset(Separator,'─',Length);
        Separator[0]='╟';
        Separator[Length-1]='╢';
        Separator[Length]=0;
        int TextLength=strlen(Str[I]+1);
        if (TextLength<Length)
          strncpy(&Separator[(Length-TextLength)/2],Str[I]+1,TextLength);
        SetColor(COL_DIALOGBOX);
        BoxText(Separator);
        SetColor(COL_DIALOGTEXT);
      }
      continue;
    }
    if ((Length=strlen(Str[I]))>ScrX-15)
      Length=ScrX-15;
    int Width=X2-X1+1;
    if (Flags & MSG_LEFTALIGN)
    {
      sprintf(TmpStr,"%.*s",Width-10,Str[I]);
      GotoXY(X1+5,Y1+I+2);
    }
    else
    {
      PosX=X1+(Width-Length)/2;
      sprintf(TmpStr,"%*s%.*s%*s",PosX-X1-4,"",Length,Str[I],X2-PosX-Length-3,"");
      GotoXY(X1+4,Y1+I+2);
    }
    Text(TmpStr);
  }
  if (Buttons==0)
    ScrBuf.Flush();
  return(0);
}


void GetMessagePosition(int &X1,int &Y1,int &X2,int &Y2)
{
  X1=MessageX1;
  Y1=MessageY1;
  X2=MessageX2;
  Y2=MessageY2;
}


int GetErrorString(char *ErrStr, DWORD StrSize)
{
  int LastError = GetLastError();

  switch(LastError)
  {
    case ERROR_INVALID_FUNCTION:
      strcpy(ErrStr,MSG(MErrorInvalidFunction));
      break;
    case ERROR_BAD_COMMAND:
    case ERROR_CALL_NOT_IMPLEMENTED:
      strcpy(ErrStr,MSG(MErrorBadCommand));
      break;
    case ERROR_FILE_NOT_FOUND:
      strcpy(ErrStr,MSG(MErrorFileNotFound));
      break;
    case ERROR_PATH_NOT_FOUND:
      strcpy(ErrStr,MSG(MErrorPathNotFound));
      break;
    case ERROR_TOO_MANY_OPEN_FILES:
      strcpy(ErrStr,MSG(MErrorTooManyOpenFiles));
      break;
    case ERROR_ACCESS_DENIED:
      strcpy(ErrStr,MSG(MErrorAccessDenied));
      break;
    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY:
      strcpy(ErrStr,MSG(MErrorNotEnoughMemory));
      break;
    case ERROR_WRITE_PROTECT:
      strcpy(ErrStr,MSG(MErrorDiskRO));
      break;
    case ERROR_NOT_READY:
      strcpy(ErrStr,MSG(MErrorDeviceNotReady));
      break;
    case ERROR_NOT_DOS_DISK:
      strcpy(ErrStr,MSG(MErrorCannotAccessDisk));
      break;
    case ERROR_SECTOR_NOT_FOUND:
      strcpy(ErrStr,MSG(MErrorSectorNotFound));
      break;
    case ERROR_OUT_OF_PAPER:
      strcpy(ErrStr,MSG(MErrorOutOfPaper));
      break;
    case ERROR_WRITE_FAULT:
      strcpy(ErrStr,MSG(MErrorWrite));
      break;
    case ERROR_READ_FAULT:
      strcpy(ErrStr,MSG(MErrorRead));
      break;
    case ERROR_GEN_FAILURE:
      strcpy(ErrStr,MSG(MErrorDeviceGeneral));
      break;
    case ERROR_SHARING_VIOLATION:
    case ERROR_LOCK_VIOLATION:
      strcpy(ErrStr,MSG(MErrorFileSharing));
      break;
    case ERROR_BAD_NETPATH:
      strcpy(ErrStr,MSG(MErrorNetworkPathNotFound));
      break;
    case ERROR_NETWORK_BUSY:
      strcpy(ErrStr,MSG(MErrorNetworkBusy));
      break;
    case ERROR_NETWORK_ACCESS_DENIED:
      strcpy(ErrStr,MSG(MErrorNetworkAccessDenied));
      break;
    case ERROR_NET_WRITE_FAULT:
      strcpy(ErrStr,MSG(MErrorNetworkWrite));
      break;
    case ERROR_DRIVE_LOCKED:
      strcpy(ErrStr,MSG(MErrorDiskLocked));
      break;
    case ERROR_ALREADY_EXISTS:
      strcpy(ErrStr,MSG(MErrorFileExists));
      break;
    case ERROR_BAD_PATHNAME:
    case ERROR_INVALID_NAME:
      strcpy(ErrStr,MSG(MErrorInvalidName));
      break;
    case ERROR_DISK_FULL:
    case ERROR_HANDLE_DISK_FULL:
      strcpy(ErrStr,MSG(MErrorInsufficientDiskSpace));
      break;
    case ERROR_DIR_NOT_EMPTY:
      strcpy(ErrStr,MSG(MErrorFolderNotEmpty));
      break;
    case ERROR_INTERNET_INCORRECT_USER_NAME:
      strcpy(ErrStr,MSG(MErrorIncorrectUserName));
      break;
    case ERROR_INTERNET_INCORRECT_PASSWORD:
      strcpy(ErrStr,MSG(MErrorIncorrectPassword));
      break;
    case ERROR_INTERNET_LOGIN_FAILURE:
      strcpy(ErrStr,MSG(MErrorLoginFailure));
      break;
    case ERROR_INTERNET_CONNECTION_ABORTED:
      strcpy(ErrStr,MSG(MErrorConnectionAborted));
      break;
    case ERROR_CANCELLED:
      strcpy(ErrStr,MSG(MErrorCancelled));
      break;
    case ERROR_NO_NETWORK:
      strcpy(ErrStr,MSG(MErrorNetAbsent));
      break;
    case ERROR_DEVICE_IN_USE:
      strcpy(ErrStr,MSG(MErrorNetDeviceInUse));
      break;
    case ERROR_OPEN_FILES:
      strcpy(ErrStr,MSG(MErrorNetOpenFiles));
      break;
    case ERROR_ALREADY_ASSIGNED:
      strcpy(ErrStr,MSG(MErrorAlreadyAssigned));
      break;
    case ERROR_DEVICE_ALREADY_REMEMBERED:
      strcpy(ErrStr,MSG(MErrorAlreadyRemebered));
      break;
    case ERROR_NOT_LOGGED_ON:
      strcpy(ErrStr,MSG(MErrorNotLoggedOn));
      break;
    case ERROR_INVALID_PASSWORD:
      strcpy(ErrStr,MSG(MErrorInvalidPassword));
      break;
    default:
    /* $ 27.01.2001 VVM
         + Если GetErrorString не распознает ошибку - пытается узнать у системы */
      if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        NULL, LastError, 0, ErrStr, StrSize, NULL))
      {
        CharToOem(ErrStr,ErrStr);
        return(TRUE);
      }
    /* VVM $ */
      *ErrStr=0;
      return(FALSE);
  }
  return(TRUE);
}

void SetMessageHelp(char *Topic)
{
  strcpy(MsgHelpTopic,Topic);
}


