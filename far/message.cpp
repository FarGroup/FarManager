/*
message.cpp

¬ывод MessageBox

*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "ctrlobj.hpp"
#include "fn.hpp"
#include "plugin.hpp"
#include "lang.hpp"
#include "colors.hpp"
#include "dialog.hpp"
#include "farftp.hpp"
#include "scrbuf.hpp"
#include "keys.hpp"
#include "TaskBar.hpp"

static int MessageX1,MessageY1,MessageX2,MessageY2;
static char MsgHelpTopic[80];
static int FirstButtonIndex,LastButtonIndex;
static BOOL IsWarningStyle;


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

LONG_PTR WINAPI MsgDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
	switch (Msg)
	{
		case DN_INITDIALOG:
		{
			FarDialogItem di;

			for (int i=0; Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,i,(LONG_PTR)&di); i++)
			{
				if (di.Type==DI_EDIT)
				{
					COORD pos={0,0};
					Dialog::SendDlgMessage(hDlg,DM_SETCURSORPOS,i,(LONG_PTR)&pos);
				}
			}
		}
		break;
		case DN_CTLCOLORDLGITEM:
		{
			FarDialogItem di;
			Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,Param1,(LONG_PTR)&di);

			if (di.Type==DI_EDIT)
			{
				int Color=FarColorToReal(IsWarningStyle?COL_WARNDIALOGTEXT:COL_DIALOGTEXT)&0xFF;
				return Param2&0xFF00FF00|Color<<16|Color;
			}
		}
		break;
		case DN_KEY:
		{
			if (Param1==FirstButtonIndex && (Param2==KEY_LEFT || Param2 == KEY_NUMPAD4 || Param2==KEY_SHIFTTAB))
			{
				Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,LastButtonIndex,0);
				return TRUE;
			}
			else if (Param1==LastButtonIndex && (Param2==KEY_RIGHT || Param2 == KEY_NUMPAD6 || Param2==KEY_TAB))
			{
				Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,FirstButtonIndex,0);
				return TRUE;
			}
		}
		break;
	}

	return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
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
	*ErrStr=0;

	// *** ѕодготовка данных ***
	if (Flags & MSG_ERRORTYPE)
		ErrorSets=GetErrorString(ErrStr, sizeof(ErrStr));

	// выделим пам€ть под рабочий массив указателей на строки (+запас 16)
	Str=(const char **)xf_malloc((ItemsNumber+ADDSPACEFORPSTRFORMESSAGE) * sizeof(char*));

	if (!Str)
		return -1;

	StrCount=ItemsNumber-Buttons;

	// предварительный обсчет максимального размера.
	for (BtnLength=0,I=0; I<Buttons; I++) //??
		BtnLength+=HiStrlen(Items[I+StrCount])+2;

	for (MaxLength=BtnLength,I=0; I<StrCount; I++)
	{
		if ((Length=(int)strlen(Items[I]))>MaxLength)
			MaxLength=Length;
	}

	// учтем так же размер заголовка
	if (Title && *Title)
	{
		I=(int)strlen(Title)+2;

		if (MaxLength < I)
			MaxLength=I;
	}

#define MAX_WIDTH_MESSAGE (ScrX-13)

	// пева€ коррекци€ максимального размера
	if (MaxLength > MAX_WIDTH_MESSAGE)
		MaxLength=MAX_WIDTH_MESSAGE;

	// теперь обработаем MSG_ERRORTYPE
	int CountErrorLine=0;

	if ((Flags & MSG_ERRORTYPE) && ErrorSets)
	{
		// подсчет количества строк во врапенном сообщениеи
		++CountErrorLine;
		//InsertQuote(ErrStr); // оквочим
		// вычисление "красивого" размера
		int LenErrStr=(int)strlen(ErrStr);

		if (LenErrStr > MAX_WIDTH_MESSAGE)
		{
			// половина меньше?
			if (LenErrStr/2 < MAX_WIDTH_MESSAGE)
			{
				// а половина + 1/3?
				if ((LenErrStr+LenErrStr/3)/2 < MAX_WIDTH_MESSAGE)
					LenErrStr=(LenErrStr+LenErrStr/3)/2;
				else
					LenErrStr/=2;
			}
			else
				LenErrStr=MAX_WIDTH_MESSAGE;
		}
		else if (LenErrStr < MaxLength)
			LenErrStr=MaxLength;

		if (MaxLength > LenErrStr && MaxLength >= MAX_WIDTH_MESSAGE)
			MaxLength=LenErrStr;

		if (MaxLength < LenErrStr && LenErrStr <= MAX_WIDTH_MESSAGE)
			MaxLength=LenErrStr;

		// а теперь проврапим
		//PtrStr=FarFormatText(ErrStr,MaxLength-(MaxLength > MAX_WIDTH_MESSAGE/2?1:0),ErrStr,sizeof(ErrStr),"\n",0); //?? MaxLength ??
		PtrStr=FarFormatText(ErrStr,LenErrStr,ErrStr,sizeof(ErrStr),"\n",0); //?? MaxLength ??

		while ((PtrStr=strchr(PtrStr,'\n')) != NULL)
		{
			*PtrStr++=0;

			if (*PtrStr)
				CountErrorLine++;
		}

		if (CountErrorLine > ADDSPACEFORPSTRFORMESSAGE)
			CountErrorLine=ADDSPACEFORPSTRFORMESSAGE; //??
	}

	// заполн€ем массив...
	CPtrStr=ErrStr;

	for (I=0; I < CountErrorLine; I++)
	{
		Str[I]=CPtrStr;
		CPtrStr+=strlen(CPtrStr)+1;

		if (!*CPtrStr) // два идущих подр€д нул€ - "хандец" всему
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

	if (Y1 < 0)
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
	xstrncpy(HelpTopic,MsgHelpTopic,sizeof(HelpTopic)-1);
	*MsgHelpTopic=0;
	// *** ¬ариант с ƒиалогом ***

	if (Buttons>0)
	{
		int ItemCount;
		struct DialogItem *PtrMsgDlg;
		struct DialogItem *MsgDlg=(struct DialogItem *)
		                          xf_malloc((ItemCount=StrCount+Buttons+1)*
		                                    sizeof(struct DialogItem));

		if (!MsgDlg)
		{
			xf_free(Str);
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

		if (Title && *Title)
			xstrncpy(MsgDlg[0].Data,Title,sizeof(MsgDlg[0].Data)-1);

		int TypeItem=DI_TEXT;
		DWORD FlagsItem=DIF_SHOWAMPERSAND;
		BOOL IsButton=FALSE;
		int CurItem=0;

		for (PtrMsgDlg=MsgDlg+1,I=1; I < ItemCount; ++I, ++PtrMsgDlg, ++CurItem)
		{
			if (I==StrCount+1)
			{
				PtrMsgDlg->Focus=1;
				PtrMsgDlg->DefaultButton=1;
				TypeItem=DI_BUTTON;
				FlagsItem=DIF_CENTERGROUP|DIF_NOBRACKETS;
				IsButton=TRUE;
				FirstButtonIndex=CurItem+1;
				LastButtonIndex=CurItem;
			}

			PtrMsgDlg->Type=TypeItem;
			PtrMsgDlg->Flags=FlagsItem;
			CPtrStr=Str[CurItem];

			if (IsButton)
			{
				PtrMsgDlg->Y1=Y2-Y1-2;
				sprintf(PtrMsgDlg->Data," %s ",CPtrStr);
				LastButtonIndex++;
			}
			else
			{
				PtrMsgDlg->X1=(Flags & MSG_LEFTALIGN)?5:-1;
				PtrMsgDlg->Y1=I+1;
				char Chr=*CPtrStr;

				if (Chr == 1 || Chr == 2)
				{
					CPtrStr++;
					PtrMsgDlg->Flags|=DIF_BOXCOLOR|(Chr==2?DIF_SEPARATOR2:DIF_SEPARATOR);
				}
				else
				{
					size_t TextLength=strlen(CPtrStr);

					if ((int)TextLength>X2-X1-9)
					{
						PtrMsgDlg->Type=DI_EDIT;
						PtrMsgDlg->Flags|=DIF_READONLY|DIF_BTNNOCLOSE|DIF_SELECTONENTRY;
						PtrMsgDlg->X1=5;
						PtrMsgDlg->X2=X2-X1-5;

						if (TextLength>sizeof(PtrMsgDlg->Data)-1)
						{
							PtrMsgDlg->Flags|=DIF_VAREDIT;
							PtrMsgDlg->Ptr.PtrData=(void*)CPtrStr;
							PtrMsgDlg->Ptr.PtrLength=(int)TextLength;
						}
						else
							xstrncpy(PtrMsgDlg->Data,CPtrStr,sizeof(PtrMsgDlg->Data)-1);

						continue;
					}
				}

				xstrncpy(PtrMsgDlg->Data,CPtrStr,Min((int)MAX_WIDTH_MESSAGE,(int)sizeof(PtrMsgDlg->Data))); //?? ScrX-15 ??
			}
		}

		{
			IsWarningStyle=Flags&MSG_WARNING;
			Dialog Dlg(MsgDlg,ItemCount,MsgDlgProc);
			Dlg.SetPosition(X1,Y1,X2,Y2);

			if (*HelpTopic)
				Dlg.SetHelp(HelpTopic);

			Dlg.SetPluginNumber(PluginNumber); // «апомним номер плагина

			if (Flags & MSG_WARNING)
				Dlg.SetDialogMode(DMODE_WARNINGSTYLE);

			Dlg.SetDialogMode(DMODE_MSGINTERNAL);
			FlushInputBuffer();

			if (Flags & MSG_KILLSAVESCREEN)
				Dialog::SendDlgMessage((HANDLE)&Dlg,DM_KILLSAVESCREEN,0,0);

			Dlg.Process();
			RetCode=Dlg.GetExitCode();
		}

		xf_free(MsgDlg);
		xf_free(Str);
		return(RetCode<0?RetCode:RetCode-StrCount-1);
	}

	// *** Ѕез ƒиалога! ***
	SetCursorType(0,0);

	if (!(Flags & MSG_KEEPBACKGROUND))
	{
		SetScreen(X1,Y1,X2,Y2,' ',(Flags & MSG_WARNING)?COL_WARNDIALOGTEXT:COL_DIALOGTEXT);
		MakeShadow(X1+2,Y2+1,X2+2,Y2+1);
		MakeShadow(X2+1,Y1+1,X2+2,Y2+1);
		Box(X1+3,Y1+1,X2-3,Y2-1,(Flags & MSG_WARNING)?COL_WARNDIALOGBOX:COL_DIALOGBOX,DOUBLE_BOX);
	}

	SetColor((Flags & MSG_WARNING)?COL_WARNDIALOGTEXT:COL_DIALOGTEXT);

	if (Title && *Title)
	{
		char TempTitle[2048];
		xstrncpy(TempTitle,Title,Min((int)sizeof(TempTitle)-1,(int)MaxLength));
		GotoXY(X1+(X2-X1-1-(int)strlen(TempTitle))/2,Y1+1);
		mprintf(" %s ",TempTitle);
	}

	for (I=0; I<StrCount; I++)
	{
		int PosX;
		CPtrStr=Str[I];
		char Chr=*CPtrStr;

		if (Chr == 1 || Chr == 2)
		{
			int Length=X2-X1-5;

			if (Length>1)
			{
				SetColor((Flags & MSG_WARNING)?COL_WARNDIALOGBOX:COL_DIALOGBOX);
				GotoXY(X1+3,Y1+I+2);
				DrawLine(Length,(Chr == 2?3:1));
				CPtrStr++;
				int TextLength=(int)strlen(CPtrStr);

				if (TextLength<Length)
				{
					GotoXY(X1+3+(Length-TextLength)/2,Y1+I+2);
					Text(CPtrStr);
				}

				SetColor((Flags & MSG_WARNING)?COL_WARNDIALOGTEXT:COL_DIALOGTEXT);
			}

			continue;
		}

		if ((Length=(int)strlen(CPtrStr))>ScrX-15)
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

	/* $ 13.01.2003 IS
	   - ѕринудительно уберем запрет отрисовки экрана, если количество кнопок
	     в сообщении равно нулю и макрос закончил выполн€тьс€. Ёто необходимо,
	     чтобы заработал прогресс-бар от плагина, который был запущен при помощи
	     макроса запретом отрисовки (bugz#533).
	*/
	xf_free(Str);

	if (Buttons==0)
	{
		if (ScrBuf.GetLockCount()>0 && !CtrlObject->Macro.PeekKey())
			ScrBuf.SetLockCount(0);

		ScrBuf.Flush();
	}

	/* IS $ */
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
	static struct TypeErrMsgs
	{
		DWORD WinMsg;
		int FarMsg;
	} ErrMsgs[]=
	{
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
		{ERROR_DIRECTORY,MErrorInvalidName},
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

		// "новые знани€"
		{ERROR_NO_RECOVERY_POLICY,MErrorNoRecoveryPolicy},
		{ERROR_ENCRYPTION_FAILED,MErrorEncryptionFailed},
		{ERROR_DECRYPTION_FAILED,MErrorDecryptionFailed},
		{ERROR_FILE_NOT_ENCRYPTED,MErrorFileNotEncrypted},
		{ERROR_NO_ASSOCIATION,MErrorNoAssociation},
	};
	DWORD LastError = GetLastError();
	//_SVS(SysLog("LastError=%d (%x)",LastError,LastError));

	for (I=0; I < sizeof(ErrMsgs)/sizeof(ErrMsgs[0]); ++I)
		if (ErrMsgs[I].WinMsg == LastError)
		{
			xstrncpy(ErrStr,MSG(ErrMsgs[I].FarMsg),StrSize-1);
			break;
		}

	//  I = sizeof(ErrMsgs)/sizeof(ErrMsgs[0]);
	if (I >= sizeof(ErrMsgs)/sizeof(ErrMsgs[0]))
	{
		/* $ 27.01.2001 VVM
		   + ≈сли GetErrorString не распознает ошибку - пытаетс€ узнать у системы */
		if (LastError != ERROR_SUCCESS && // нефига показывать лажу...
		        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		                      NULL, LastError, 0, ErrStr, StrSize, NULL))
		{
			// дл€ проверки криков:
			//strcpy(ErrStr,"Ќевозможно найти сетевой путь. ”бедитесь, что сетевой путь указан верно, а конечный компьютер включен и не зан€т. ≈сли система вновь не сможет найти путь, обратитесь к сетевому администратору.");
			FAR_CharToOem(ErrStr,ErrStr);
			/* $ 02.02.2001 IS
			   + «аменим cr и lf на пробелы
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
	xstrncpy(MsgHelpTopic,Topic, sizeof(MsgHelpTopic)-1);
}

/* $ 12.03.2002 VVM
  Ќова€ функци€ - пользователь попыталс€ прервать операцию.
  «ададим вопрос.
  ¬озвращает:
   FALSE - продолжить операцию
   TRUE  - прервать операцию
*/
int AbortMessage()
{
	TaskBarPause TBP;
	int Res = Message(MSG_WARNING|MSG_KILLSAVESCREEN,2,MSG(MKeyESCWasPressed),
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

// ѕроверка на "продолжаемость" экспериментов по... например, удалению файла с разными именами!
BOOL CheckErrorForProcessed(DWORD Err)
{
	switch (Err)
	{
		case ERROR_ACCESS_DENIED:
		case ERROR_WRITE_PROTECT:
		case ERROR_NOT_READY:
		case ERROR_SHARING_VIOLATION:
		case ERROR_LOCK_VIOLATION:
			return FALSE;
	}

	return TRUE;
}
