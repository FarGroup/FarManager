/*
message.cpp

Вывод MessageBox

*/

/* Revision: 1.13 24.09.2001 $ */

/*
Modify:
  24.09.2001 SVS
    ! немного оптимизации (сокращение кода)
  20.09.2001 SVS
    - не компилится под VC, тем более, что I уже определена!
  19.09.2001 VVM
    + Если сообщение об ошибке слишком длинное - перенесем его...
  25.06.2001 IS
    ! Внедрение const
    - Устранение бага (порча стека) в SetMessageHelp
  18.05.2001 DJ
    + #include "colors.hpp"
  10.05.2001 SVS
    ! Вызываем SetDialogMode(DMODE_WARNINGSTYLE) влоб, т.к. функция
      Dialog::SetWarningStyle() удалена.
  06.05.2001 DJ
    ! перетрях #include
  14.03.2001 SVS
    ! Если GetLastError() вернул ERROR_SUCCESS, то нефига показывать
      инфу про успешное выполнение в сообщении об ошибке.
  27.02.2001 VVM
    ! Отрисовка сепаратора - функцией MakeSeparator()
  02.02.2001 IS
    + Заменим cr и lf на пробелы в GetErrorString
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

static int MessageX1,MessageY1,MessageX2,MessageY2;
static char MsgHelpTopic[80];

#include "global.hpp"
#include "fn.hpp"
#include "plugin.hpp"
#include "lang.hpp"
#include "colors.hpp"
#include "dialog.hpp"
#include "farftp.hpp"
#include "scrbuf.hpp"

int Message(int Flags,int Buttons,const char *Title,const char *Str1,
            const char *Str2,const char *Str3,const char *Str4,
            int PluginNumber)
{
  return(Message(Flags,Buttons,Title,Str1,Str2,Str3,Str4,NULL,NULL,NULL,
                 NULL,NULL,NULL,NULL,NULL,NULL,NULL,PluginNumber));
}

int Message(int Flags,int Buttons,const char *Title,const char *Str1,
            const char *Str2,const char *Str3,const char *Str4,
            const char *Str5,const char *Str6,const char *Str7,
            int PluginNumber)
{
  return(Message(Flags,Buttons,Title,Str1,Str2,Str3,Str4,Str5,Str6,Str7,
                 NULL,NULL,NULL,NULL,NULL,NULL,NULL,PluginNumber));
}


int Message(int Flags,int Buttons,const char *Title,const char *Str1,
            const char *Str2,const char *Str3,const char *Str4,
            const char *Str5,const char *Str6,const char *Str7,
            const char *Str8,const char *Str9,const char *Str10,
            int PluginNumber)
{
  return(Message(Flags,Buttons,Title,Str1,Str2,Str3,Str4,Str5,Str6,Str7,Str8,
                 Str9,Str10,NULL,NULL,NULL,NULL,PluginNumber));
}


int Message(int Flags,int Buttons,const char *Title,const char *Str1,
            const char *Str2,const char *Str3,const char *Str4,
            const char *Str5,const char *Str6,const char *Str7,
            const char *Str8,const char *Str9,const char *Str10,
            const char *Str11,const char *Str12,const char *Str13,
            const char *Str14,int PluginNumber)
{
  char TmpStr[256],ErrStr[256],HelpTopic[80];
  const char *Str[14],*Btn[14];
  int X1,Y1,X2,Y2;
  int Length,MaxLength,BtnLength,StrCount,I;
  int ErrStrPresent = FALSE;
  char ErrStr2[256];

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

  if ((Flags & MSG_ERRORTYPE) && GetErrorString(ErrStr, sizeof(ErrStr)))
  {
    for (int I=sizeof(Str)/sizeof(Str[0])-1;I>0;I--)
      Str[I]=Str[I-1];
    Str[0]=ErrStr;
    StrCount++;
    ErrStrPresent = TRUE;
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
  /* $ 19.09.2001 VVM
    + Если сообщение об ошибке слишком длинное - перенесем его... */
  if ((ErrStrPresent) && (strlen(Str[0]) > MaxLength))
  {
    int DotPos = MaxLength - 1;
    for (I=MaxLength;I>=0;I--)
    {
      if (Str[0][I] == '.')
      {
        DotPos = I;
        break;
      } /* if */
    } /* for */
    strcpy(ErrStr2,&ErrStr[DotPos+1]);
    ErrStr[DotPos+1] = 0;
    for (I=sizeof(Str)/sizeof(Str[0])-2;I>1;I--)
      Str[I]=Str[I-1];
    Str[1]=ErrStr2;
  } /* if */
  /* VVM $ */
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
      Dlg.SetDialogMode(DMODE_WARNINGSTYLE);
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
        MakeSeparator(Length,Separator,1);
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
  int I;
  static struct TypeErrMsgs{
    int WinMsg;
    int FarMsg;
  } ErrMsgs[]={
    {ERROR_INVALID_FUNCTION,MErrorInvalidFunction},
    {ERROR_BAD_COMMAND,MErrorBadCommand},
    {ERROR_CALL_NOT_IMPLEMENTED,MErrorBadCommand},
    {ERROR_FILE_NOT_FOUND,MErrorFileNotFound},
    {ERROR_PATH_NOT_FOUND,MErrorPathNotFound},
    {ERROR_TOO_MANY_OPEN_FILES,MErrorTooManyOpenFiles},
    {ERROR_ACCESS_DENIED,MErrorAccessDenied},
    {ERROR_NOT_ENOUGH_MEMORY,MErrorNotEnoughMemory},
    {ERROR_OUTOFMEMORY,MErrorNotEnoughMemory},
    {ERROR_WRITE_PROTECT,MErrorDiskRO},
    {ERROR_NOT_READY,MErrorDeviceNotReady},
    {ERROR_NOT_DOS_DISK,MErrorCannotAccessDisk},
    {ERROR_SECTOR_NOT_FOUND,MErrorSectorNotFound},
    {ERROR_OUT_OF_PAPER,MErrorOutOfPaper},
    {ERROR_WRITE_FAULT,MErrorWrite},
    {ERROR_READ_FAULT,MErrorRead},
    {ERROR_GEN_FAILURE,MErrorDeviceGeneral},
    {ERROR_SHARING_VIOLATION,MErrorFileSharing},
    {ERROR_LOCK_VIOLATION,MErrorFileSharing},
    {ERROR_BAD_NETPATH,MErrorNetworkPathNotFound},
    {ERROR_NETWORK_BUSY,MErrorNetworkBusy},
    {ERROR_NETWORK_ACCESS_DENIED,MErrorNetworkAccessDenied},
    {ERROR_NET_WRITE_FAULT,MErrorNetworkWrite},
    {ERROR_DRIVE_LOCKED,MErrorDiskLocked},
    {ERROR_ALREADY_EXISTS,MErrorFileExists},
    {ERROR_BAD_PATHNAME,MErrorInvalidName},
    {ERROR_INVALID_NAME,MErrorInvalidName},
    {ERROR_DISK_FULL,MErrorInsufficientDiskSpace},
    {ERROR_HANDLE_DISK_FULL,MErrorInsufficientDiskSpace},
    {ERROR_DIR_NOT_EMPTY,MErrorFolderNotEmpty},
    {ERROR_INTERNET_INCORRECT_USER_NAME,MErrorIncorrectUserName},
    {ERROR_INTERNET_INCORRECT_PASSWORD,MErrorIncorrectPassword},
    {ERROR_INTERNET_LOGIN_FAILURE,MErrorLoginFailure},
    {ERROR_INTERNET_CONNECTION_ABORTED,MErrorConnectionAborted},
    {ERROR_CANCELLED,MErrorCancelled},
    {ERROR_NO_NETWORK,MErrorNetAbsent},
    {ERROR_DEVICE_IN_USE,MErrorNetDeviceInUse},
    {ERROR_OPEN_FILES,MErrorNetOpenFiles},
    {ERROR_ALREADY_ASSIGNED,MErrorAlreadyAssigned},
    {ERROR_DEVICE_ALREADY_REMEMBERED,MErrorAlreadyRemebered},
    {ERROR_NOT_LOGGED_ON,MErrorNotLoggedOn},
    {ERROR_INVALID_PASSWORD,MErrorInvalidPassword},
  };

  int LastError = GetLastError();

  for(I=0; I < sizeof(ErrMsgs)/sizeof(ErrMsgs[0]); ++I)
    if(ErrMsgs[I].WinMsg == LastError)
    {
      strcpy(ErrStr,MSG(ErrMsgs[I].FarMsg));
      break;
    }

  if(I >= sizeof(ErrMsgs)/sizeof(ErrMsgs[0]))
  {
    /* $ 27.01.2001 VVM
       + Если GetErrorString не распознает ошибку - пытается узнать у системы */
    if (LastError != ERROR_SUCCESS && // нефига показывать лажу...
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                      NULL, LastError, 0, ErrStr, StrSize, NULL))
    {
      CharToOem(ErrStr,ErrStr);
      /* $ 02.02.2001 IS
         + Заменим cr и lf на пробелы
      */
      RemoveUnprintableCharacters(ErrStr);
      /* IS $ */
      return(TRUE);
    }
    /* VVM $ */
    *ErrStr=0;
    return(FALSE);
  }
  return(TRUE);
}

void SetMessageHelp(const char *Topic)
{
  strncpy(MsgHelpTopic,Topic, sizeof(MsgHelpTopic));
}
