/*
message.cpp

Вывод MessageBox

*/

/* Revision: 1.23 03.04.2002 $ */

/*
Modify:
  03.04.2002 SVS
    ! WordWrap -> FarFormatText
  22.03.2002 SVS
    - strcpy - Fuck!
  12.03.2002 VVM
    + Новая функция - пользователь попытался прервать операцию.
      Зададим вопрос.
  27.02.2002 SVS
    ! Косметика для компиляции с debug-инфой под BC
  26.10.2001 SVS
    ! небольшие уточнения размеров
  23.10.2001 SVS
    ! неверное(!) применение strncpy
  22.10.2001 SVS
    + "Новый знания" о системных ошибках - по поводу шифрования в Win2K
  22.10.2001 SVS
    - Не проинициализированная переменная
    ! Заюзаем вместо +16 константу ADDSPACEFORPSTRFORMESSAGE
    - strncpy
  18.10.2001 SVS
    ! У функций Message параметр Flags имеет суть "DWORD"
    + Новый вариант Message - без ограничения на количество строк
    ! "немног" оптимизации кода.
  27.09.2001 IS
    - Левый размер при использовании strncpy
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

#include "global.hpp"
#include "fn.hpp"
#include "plugin.hpp"
#include "lang.hpp"
#include "colors.hpp"
#include "dialog.hpp"
#include "farftp.hpp"
#include "scrbuf.hpp"

static int MessageX1,MessageY1,MessageX2,MessageY2;
static char MsgHelpTopic[80];


int Message(DWORD Flags,int Buttons,const char *Title,const char *Str1,
            const char *Str2,const char *Str3,const char *Str4,
            int PluginNumber)
{
  return(Message(Flags,Buttons,Title,Str1,Str2,Str3,Str4,NULL,NULL,NULL,
                 NULL,NULL,NULL,NULL,NULL,NULL,NULL,PluginNumber));
}

int Message(DWORD Flags,int Buttons,const char *Title,const char *Str1,
            const char *Str2,const char *Str3,const char *Str4,
            const char *Str5,const char *Str6,const char *Str7,
            int PluginNumber)
{
  return(Message(Flags,Buttons,Title,Str1,Str2,Str3,Str4,Str5,Str6,Str7,
                 NULL,NULL,NULL,NULL,NULL,NULL,NULL,PluginNumber));
}


int Message(DWORD Flags,int Buttons,const char *Title,const char *Str1,
            const char *Str2,const char *Str3,const char *Str4,
            const char *Str5,const char *Str6,const char *Str7,
            const char *Str8,const char *Str9,const char *Str10,
            int PluginNumber)
{
  return(Message(Flags,Buttons,Title,Str1,Str2,Str3,Str4,Str5,Str6,Str7,Str8,
                 Str9,Str10,NULL,NULL,NULL,NULL,PluginNumber));
}

int Message(DWORD Flags,int Buttons,const char *Title,const char *Str1,
            const char *Str2,const char *Str3,const char *Str4,
            const char *Str5,const char *Str6,const char *Str7,
            const char *Str8,const char *Str9,const char *Str10,
            const char *Str11,const char *Str12,const char *Str13,
            const char *Str14,int PluginNumber)
{
  int StrCount;
  const char *Str[14];

  Str[0]=Str1;   Str[1]=Str2;   Str[2]=Str3;   Str[3]=Str4;
  Str[4]=Str5;   Str[5]=Str6;   Str[6]=Str7;   Str[7]=Str8;
  Str[8]=Str9;   Str[9]=Str10;  Str[10]=Str11; Str[11]=Str12;
  Str[12]=Str13; Str[13]=Str14;

  StrCount=0;
  while (StrCount<sizeof(Str)/sizeof(Str[0]) && Str[StrCount]!=NULL)
    StrCount++;

  return Message(Flags,Buttons,Title,Str,StrCount,PluginNumber);
}

int Message(DWORD Flags,int Buttons,const char *Title,
            const char * const *Items,int ItemsNumber,
            int PluginNumber)
{
  char TmpStr[256],ErrStr[2048];
  int X1,Y1,X2,Y2;
  int Length,MaxLength,BtnLength,I, J, StrCount;
  BOOL ErrorSets=FALSE;
  const char **Str;
  char *PtrStr;
  const char *CPtrStr;

  // *** Подготовка данных ***
  if (Flags & MSG_ERRORTYPE)
    ErrorSets=GetErrorString(ErrStr, sizeof(ErrStr));

  // выделим память под рабочий массив указателей на строки (+запас 16)
  Str=(const char **)malloc((ItemsNumber+ADDSPACEFORPSTRFORMESSAGE) * sizeof(char*));
  if(!Str)
    return -1;

  StrCount=ItemsNumber-Buttons;

  // предварительный обсчет максимального размера.
  for (BtnLength=0,I=0;I<Buttons;I++) //??
    BtnLength+=HiStrlen(Items[I+StrCount])+2;

  for (MaxLength=BtnLength,I=0;I<StrCount;I++)
  {
    if ((Length=strlen(Items[I]))>MaxLength)
      MaxLength=Length;
  }

  // учтем так же размер заголовка
  if(Title && *Title)
  {
    I=strlen(Title)+2;
    if (MaxLength < I)
      MaxLength=I;
  }

  // певая коррекция максимального размера
  if (MaxLength>ScrX-15)
    MaxLength=ScrX-15;

  // теперь обработаем MSG_ERRORTYPE
  int CountErrorLine=0;

  if ((Flags & MSG_ERRORTYPE) && ErrorSets)
  {
    // подсчет количества строк во врапенном сообщениеи
    ++CountErrorLine;
    //InsertQuote(ErrStr); // оквочим

    // вычисление "красивого" размера
    int LenErrStr=strlen(ErrStr);
    if(LenErrStr > ScrX-15)
    {
      // половина меньше?
      if(LenErrStr/2 < ScrX-15)
      {
        // а половина + 1/3?
        if((LenErrStr+LenErrStr/3)/2 < ScrX-15)
          LenErrStr=(LenErrStr+LenErrStr/3)/2;
        else
          LenErrStr/=2;
      }
      else
        LenErrStr=ScrX-15;
    }
    else if(LenErrStr < MaxLength)
      LenErrStr=MaxLength;
    MaxLength=LenErrStr;

    // а теперь проврапим
    PtrStr=FarFormatText(ErrStr,MaxLength,ErrStr,sizeof(ErrStr),"\n",0); //?? MaxLength ??
    while((PtrStr=strchr(PtrStr,'\n')) != NULL)
    {
      *PtrStr++=0;
      if(*PtrStr)
        CountErrorLine++;
    }
    if(CountErrorLine > ADDSPACEFORPSTRFORMESSAGE)
      CountErrorLine=ADDSPACEFORPSTRFORMESSAGE; //??
  }

  // заполняем массив...
  CPtrStr=ErrStr;
  for (I=0; I < CountErrorLine;I++)
  {
    Str[I]=CPtrStr;
    CPtrStr+=strlen(CPtrStr)+1;
    if(!*CPtrStr) // два идущих подряд нуля - "хандец" всему
    {
      ++I;
      break;
    }
  }

  for (J=0; J < ItemsNumber; ++J, ++I)
  {
    Str[I]=Items[J];
  }


  StrCount+=CountErrorLine;

  MessageX1=X1=(ScrX-MaxLength)/2-4;
  MessageX2=X2=X1+MaxLength+9;
  Y1=(ScrY-StrCount)/2-2;
  if(Y1 < 0)
    Y1=0;
  MessageY1=Y1;

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

  char HelpTopic[80];
  strncpy(HelpTopic,MsgHelpTopic,sizeof(HelpTopic)-1);
  *MsgHelpTopic=0;

  // *** Вариант с Диалогом ***

  if (Buttons>0)
  {
    int ItemCount;
    struct DialogItem *PtrMsgDlg;
    struct DialogItem *MsgDlg=(struct DialogItem *)
                              malloc((ItemCount=StrCount+Buttons+1)*
                                     sizeof(struct DialogItem));
    if(!MsgDlg)
    {
      free(Str);
      return -1;
    }

    memset(MsgDlg,0,ItemCount*sizeof(struct DialogItem));

    int RetCode;
    MessageY2=++Y2;

    MsgDlg[0].Type=DI_DOUBLEBOX;
    MsgDlg[0].X1=3;
    MsgDlg[0].Y1=1;
    MsgDlg[0].X2=X2-X1-3;
    MsgDlg[0].Y2=Y2-Y1-1;

    if(Title && *Title)
      strncpy(MsgDlg[0].Data,Title,sizeof(MsgDlg[0].Data)-1);

    int TypeItem=DI_TEXT;
    DWORD FlagsItem=DIF_SHOWAMPERSAND;
    BOOL IsButton=FALSE;
    int CurItem=0;
    for(PtrMsgDlg=MsgDlg+1,I=1; I < ItemCount; ++I, ++PtrMsgDlg, ++CurItem)
    {
      if(I==StrCount+1)
      {
        PtrMsgDlg->Focus=1;
        PtrMsgDlg->DefaultButton=1;
        TypeItem=DI_BUTTON;
        FlagsItem=DIF_CENTERGROUP|DIF_NOBRACKETS;
        IsButton=TRUE;
      }

      PtrMsgDlg->Type=TypeItem;
      PtrMsgDlg->Flags=FlagsItem;
      CPtrStr=Str[CurItem];
      if(IsButton)
      {
        PtrMsgDlg->Y1=Y2-Y1-2;
        sprintf(PtrMsgDlg->Data," %s ",CPtrStr);
      }
      else
      {
        PtrMsgDlg->X1=(Flags & MSG_LEFTALIGN)?5:-1;
        PtrMsgDlg->Y1=I+1;
        char Chr=*CPtrStr;
        if(Chr == 1 || Chr == 2)
        {
          CPtrStr++;
          PtrMsgDlg->Flags|=DIF_BOXCOLOR|(Chr==2?DIF_SEPARATOR2:DIF_SEPARATOR);
        }
        strncpy(PtrMsgDlg->Data,CPtrStr,Min((int)ScrX-15,(int)sizeof(PtrMsgDlg->Data))); //?? ScrX-15 ??
      }
    }

    {
      Dialog Dlg(MsgDlg,ItemCount);
      Dlg.SetPosition(X1,Y1,X2,Y2);
      if (*HelpTopic)
        Dlg.SetHelp(HelpTopic);
      Dlg.SetPluginNumber(PluginNumber); // Запомним номер плагина
      if (Flags & MSG_WARNING)
        Dlg.SetDialogMode(DMODE_WARNINGSTYLE);
      FlushInputBuffer();
      Dlg.Process();
      RetCode=Dlg.GetExitCode();
    }
    free(MsgDlg);
    free(Str);
    return(RetCode<0?RetCode:RetCode-StrCount-1);
  }


  // *** Без Диалога! ***

  if (!(Flags & MSG_KEEPBACKGROUND))
  {
    SetScreen(X1,Y1,X2,Y2,' ',COL_DIALOGTEXT);
    MakeShadow(X1+2,Y2+1,X2+2,Y2+1);
    MakeShadow(X2+1,Y1+1,X2+2,Y2+1);
    Box(X1+3,Y1+1,X2-3,Y2-1,COL_DIALOGBOX,DOUBLE_BOX);
  }

  SetColor(COL_DIALOGTEXT);
  if(Title && *Title)
  {
    GotoXY(X1+(X2-X1-1-strlen(Title))/2,Y1+1);
    mprintf(" %s ",Title);
  }

  for (I=0;I<StrCount;I++)
  {
    int PosX;
    CPtrStr=Str[I];
    char Chr=*CPtrStr;
    if (Chr == 1 || Chr == 2)
    {
      int Length=X2-X1-5;
      if (Length>1)
      {
        char Separator[1024];
        MakeSeparator(Length,Separator,(Chr == 2?3:1));
        CPtrStr++;
        int TextLength=strlen(CPtrStr);
        if (TextLength<Length)
          memcpy(&Separator[(Length-TextLength)/2],CPtrStr,TextLength);
        SetColor(COL_DIALOGBOX);
        GotoXY(X1+3,Y1+I+2);
        BoxText(Separator);
        SetColor(COL_DIALOGTEXT);
      }
      continue;
    }
    if ((Length=strlen(CPtrStr))>ScrX-15)
      Length=ScrX-15;
    int Width=X2-X1+1;
    if (Flags & MSG_LEFTALIGN)
    {
      sprintf(TmpStr,"%.*s",Width-10,CPtrStr);
      GotoXY(X1+5,Y1+I+2);
    }
    else
    {
      PosX=X1+(Width-Length)/2;
      sprintf(TmpStr,"%*s%.*s%*s",PosX-X1-4,"",Length,CPtrStr,X2-PosX-Length-3,"");
      GotoXY(X1+4,Y1+I+2);
    }
    Text(TmpStr);
  }
  if (Buttons==0)
    ScrBuf.Flush();
  free(Str);
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
    DWORD WinMsg;
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

    // "новые знания"
    {ERROR_NO_RECOVERY_POLICY,MErrorNoRecoveryPolicy},
    {ERROR_ENCRYPTION_FAILED,MErrorEncryptionFailed},
    {ERROR_DECRYPTION_FAILED,MErrorDecryptionFailed},
    {ERROR_FILE_NOT_ENCRYPTED,MErrorFileNotEncrypted},
  };

  DWORD LastError = GetLastError();
  //_SVS(SysLog("LastError=%d (%x)",LastError,LastError));

  for(I=0; I < sizeof(ErrMsgs)/sizeof(ErrMsgs[0]); ++I)
    if(ErrMsgs[I].WinMsg == LastError)
    {
      strncpy(ErrStr,MSG(ErrMsgs[I].FarMsg),StrSize-1);
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
      //_SVS(SysLog("LastErrorMsg=%s",ErrStr));
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
  strncpy(MsgHelpTopic,Topic, sizeof(MsgHelpTopic)-1);
}

/* $ 12.03.2002 VVM
  Новая функция - пользователь попытался прервать операцию.
  Зададим вопрос.
  Возвращает:
   FALSE - продолжить операцию
   TRUE  - прервать операцию
*/
int AbortMessage()
{
  int Res = Message(MSG_WARNING,2,MSG(MKeyESCWasPressed),
            MSG((Opt.Confirm.EscTwiceToInterrupt)?MDoYouWantToStopWork2:MDoYouWantToStopWork),
            MSG(MYes),MSG(MNo));
  if (Res == -1) // Set "ESC" equal to "NO" button
    Res = 1;
  if ((Opt.Confirm.EscTwiceToInterrupt && Res) ||
      (!Opt.Confirm.EscTwiceToInterrupt && !Res))
    return (TRUE);
  else
    return (FALSE);
}
/* VVM $ */
