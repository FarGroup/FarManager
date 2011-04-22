/*
message.cpp

¬ывод MessageBox
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

#include "message.hpp"
#include "ctrlobj.hpp"
#include "lang.hpp"
#include "colors.hpp"
#include "dialog.hpp"
#include "scrbuf.hpp"
#include "keys.hpp"
#include "TaskBar.hpp"
#include "interf.hpp"
#include "palette.hpp"
#include "config.hpp"
#include "keyboard.hpp"

static int MessageX1,MessageY1,MessageX2,MessageY2;
static string strMsgHelpTopic;
static int FirstButtonIndex,LastButtonIndex;
static BOOL IsWarningStyle;


int Message(DWORD Flags,size_t Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            INT_PTR PluginNumber)
{
	return(Message(Flags,Buttons,Title,Str1,Str2,Str3,Str4,nullptr,nullptr,nullptr,
	               nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,PluginNumber));
}

int Message(DWORD Flags,size_t Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            const wchar_t *Str5,const wchar_t *Str6,const wchar_t *Str7,
            INT_PTR PluginNumber)
{
	return(Message(Flags,Buttons,Title,Str1,Str2,Str3,Str4,Str5,Str6,Str7,
	               nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,PluginNumber));
}


int Message(DWORD Flags,size_t Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            const wchar_t *Str5,const wchar_t *Str6,const wchar_t *Str7,
            const wchar_t *Str8,const wchar_t *Str9,const wchar_t *Str10,
            INT_PTR PluginNumber)
{
	return(Message(Flags,Buttons,Title,Str1,Str2,Str3,Str4,Str5,Str6,Str7,Str8,
	               Str9,Str10,nullptr,nullptr,nullptr,nullptr,PluginNumber));
}

int Message(DWORD Flags,size_t Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            const wchar_t *Str5,const wchar_t *Str6,const wchar_t *Str7,
            const wchar_t *Str8,const wchar_t *Str9,const wchar_t *Str10,
            const wchar_t *Str11,const wchar_t *Str12,const wchar_t *Str13,
            const wchar_t *Str14,INT_PTR PluginNumber)
{
	const wchar_t *Str[]={Str1,Str2,Str3,Str4,Str5,Str6,Str7,Str8,Str9,Str10,Str11,Str12,Str13,Str14};
	int StrCount=0;

	while (StrCount<(int)ARRAYSIZE(Str) && Str[StrCount])
		StrCount++;

	return Message(Flags,Buttons,Title,Str,StrCount,PluginNumber);
}

INT_PTR WINAPI MsgDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	switch (Msg)
	{
		case DN_INITDIALOG:
		{
			FarDialogItem di;

			for (int i=0; SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,i,&di); i++)
			{
				if (di.Type==DI_EDIT)
				{
					COORD pos={0,0};
					SendDlgMessage(hDlg,DM_SETCURSORPOS,i,&pos);
				}
			}
		}
		break;
		case DN_CTLCOLORDLGITEM:
		{
			FarDialogItem di;
			SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,Param1,&di);

			if (di.Type==DI_EDIT)
			{
				int Color=FarColorToReal(IsWarningStyle?COL_WARNDIALOGTEXT:COL_DIALOGTEXT)&0xFF;
				return ((reinterpret_cast<INT_PTR>(Param2)&0xFF00FF00)|(Color<<16)|Color);
			}
		}
		break;
		case DN_CONTROLINPUT:
		{
			const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
			if (record->EventType==KEY_EVENT)
			{
				int key = InputRecordToKey((const INPUT_RECORD *)Param2);
				if (Param1==FirstButtonIndex && (key==KEY_LEFT || key == KEY_NUMPAD4 || key==KEY_SHIFTTAB))
				{
					SendDlgMessage(hDlg,DM_SETFOCUS,LastButtonIndex,0);
					return TRUE;
				}
				else if (Param1==LastButtonIndex && (key==KEY_RIGHT || key == KEY_NUMPAD6 || key==KEY_TAB))
				{
					SendDlgMessage(hDlg,DM_SETFOCUS,FirstButtonIndex,0);
					return TRUE;
				}
			}
		}
		break;
	default:
		break;
	}

	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

int Message(
    DWORD Flags,
    size_t Buttons,
    const wchar_t *Title,
    const wchar_t * const *Items,
    size_t ItemsNumber,
    INT_PTR PluginNumber
)
{
	string strTempStr;
	int X1,Y1,X2,Y2;
	int Length, BtnLength;
	DWORD I, MaxLength, StrCount;
	BOOL ErrorSets=FALSE;
	const wchar_t **Str;
	wchar_t *PtrStr;
	const wchar_t *CPtrStr;
	string strErrStr;

	if (Flags & MSG_ERRORTYPE)
		ErrorSets = GetErrorString(strErrStr);

	// выделим пам€ть под рабочий массив указателей на строки (+запас 16)
	Str=(const wchar_t **)xf_malloc((ItemsNumber+ADDSPACEFORPSTRFORMESSAGE) * sizeof(wchar_t*));

	if (!Str)
		return -1;

	StrCount=static_cast<DWORD>(ItemsNumber-Buttons);

	// предварительный обсчет максимального размера.
	for (BtnLength=0,I=0; I<static_cast<DWORD>(Buttons); I++) //??
	{
		BtnLength+=HiStrlen(Items[I+StrCount])+2+2+1; // "[ ", " ]", " "
	}
	if(BtnLength)
	{
		BtnLength--;
	}

	for (MaxLength=BtnLength,I=0; I<StrCount; I++)
	{
		if (static_cast<DWORD>(Length=StrLength(Items[I]))>MaxLength)
			MaxLength=Length;
	}

	// учтем так же размер заголовка
	if (Title && *Title)
	{
		I=(DWORD)StrLength(Title)+2;

		if (MaxLength < I)
			MaxLength=I;
	}

	// пева€ коррекци€ максимального размера
	if (MaxLength > MAX_WIDTH_MESSAGE)
		MaxLength=MAX_WIDTH_MESSAGE;

	// теперь обработаем MSG_ERRORTYPE
	DWORD CountErrorLine=0;

	if ((Flags & MSG_ERRORTYPE) && ErrorSets)
	{
		// подсчет количества строк во врапенном сообщениеи
		++CountErrorLine;
		//InsertQuote(ErrStr); // оквочим
		// вычисление "красивого" размера
		DWORD LenErrStr=(DWORD)strErrStr.GetLength();

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
		FarFormatText(strErrStr,LenErrStr,strErrStr,L"\n",0); //?? MaxLength ??
		PtrStr = strErrStr.GetBuffer();

		//BUGBUG: string не преднозначен дл€ хранени€ строк разделЄнных \0
		while ((PtrStr=wcschr(PtrStr,L'\n')) )
		{
			*PtrStr++=0;

			if (*PtrStr)
				CountErrorLine++;
		}

		strErrStr.ReleaseBuffer();

		if (CountErrorLine > ADDSPACEFORPSTRFORMESSAGE)
			CountErrorLine=ADDSPACEFORPSTRFORMESSAGE; //??
	}

	//BUGBUG: string не преднозначен дл€ хранени€ строк разделЄнных \0
	// заполн€ем массив...
	CPtrStr=strErrStr;

	for (I=0; I < CountErrorLine; I++)
	{
		Str[I]=CPtrStr;
		CPtrStr+=StrLength(CPtrStr)+1;

		if (!*CPtrStr) // два идущих подр€д нул€ - "хандец" всему
		{
			++I;
			break;
		}
	}

	bool EmptyText=false;
	if(ItemsNumber==Buttons && !I)
	{
		EmptyText=true;
		Str[I]=L"";
		I++;
		StrCount++;
		ItemsNumber++;
	}

	for (size_t J=0; J < ItemsNumber-(EmptyText?1:0); ++J, ++I)
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
	MessageY2=Y2=Y1+StrCount+3;
	string strHelpTopic(strMsgHelpTopic);
	strMsgHelpTopic.Clear();
	// *** ¬ариант с ƒиалогом ***

	if (Buttons>0)
	{
		size_t ItemCount=StrCount+Buttons+1;
		DialogItemEx *PtrMsgDlg;
		DialogItemEx *MsgDlg = new DialogItemEx[ItemCount+1];

		if (!MsgDlg)
		{
			xf_free(Str);
			return -1;
		}

		for (DWORD i=0; i<ItemCount+1; i++)
			MsgDlg[i].Clear();

		int RetCode;
		MessageY2=++Y2;
		MsgDlg[0].Type=DI_DOUBLEBOX;
		MsgDlg[0].X1=3;
		MsgDlg[0].Y1=1;
		MsgDlg[0].X2=X2-X1-3;
		MsgDlg[0].Y2=Y2-Y1-1;

		if (Title && *Title)
			MsgDlg[0].strData = Title;

		FARDIALOGITEMTYPES TypeItem=DI_TEXT;
		unsigned __int64 FlagsItem=DIF_SHOWAMPERSAND;
		BOOL IsButton=FALSE;
		int CurItem=0;
		bool StrSeparator=false;
		bool Separator=false;
		for (PtrMsgDlg=MsgDlg+1,I=1; I < ItemCount; ++I, ++PtrMsgDlg, ++CurItem)
		{
			if (I==StrCount+1 && !StrSeparator && !Separator)
			{
				PtrMsgDlg->Type=DI_TEXT;
				PtrMsgDlg->Flags=DIF_SEPARATOR;
				PtrMsgDlg->Y1=PtrMsgDlg->Y2=I+1;
				CurItem--;
				I--;
				Separator=true;
				continue;
			}
			if(I==StrCount+1)
			{
				TypeItem=DI_BUTTON;
				FlagsItem=DIF_CENTERGROUP|DIF_DEFAULTBUTTON|DIF_FOCUS;
				IsButton=TRUE;
				FirstButtonIndex=CurItem+1;
				LastButtonIndex=CurItem;
			}
			else
			{
				FlagsItem&=~DIF_DEFAULTBUTTON;
			}

			PtrMsgDlg->Type=TypeItem;
			PtrMsgDlg->Flags|=FlagsItem;
			CPtrStr=Str[CurItem];

			if (IsButton)
			{
				PtrMsgDlg->Y1=Y2-Y1-2+(Separator?1:0);
				PtrMsgDlg->strData+=CPtrStr;
				LastButtonIndex++;
			}
			else
			{
				PtrMsgDlg->X1=(Flags & MSG_LEFTALIGN)?5:-1;
				PtrMsgDlg->Y1=I+1;
				wchar_t Chr=*CPtrStr;

				if (Chr == L'\1' || Chr == L'\2')
				{
					CPtrStr++;
					PtrMsgDlg->Flags|=(Chr==2?DIF_SEPARATOR2:DIF_SEPARATOR);
					if(I==StrCount)
					{
						StrSeparator=true;
					}
				}
				else if (StrLength(CPtrStr)>X2-X1-9)
				{
					PtrMsgDlg->Type=DI_EDIT;
					PtrMsgDlg->Flags|=DIF_READONLY|DIF_BTNNOCLOSE|DIF_SELECTONENTRY;
					PtrMsgDlg->X1=5;
					PtrMsgDlg->X2=X2-X1-5;
					PtrMsgDlg->strData=CPtrStr;
					continue;
				}

				//xstrncpy(PtrMsgDlg->Data,CPtrStr,Min((int)MAX_WIDTH_MESSAGE,(int)sizeof(PtrMsgDlg->Data))); //?? ScrX-15 ??
				PtrMsgDlg->strData = CPtrStr; //BUGBUG, wrong len
			}
		}

		{
			if(Separator)
			{
				FirstButtonIndex++;
				LastButtonIndex++;
				MessageY2++;
				Y2++;
				MsgDlg[0].Y2++;
				ItemCount++;
			}
			IsWarningStyle=Flags&MSG_WARNING;
			Dialog Dlg(MsgDlg,ItemCount,MsgDlgProc);
			Dlg.SetPosition(X1,Y1,X2,Y2);

			if (!strHelpTopic.IsEmpty())
				Dlg.SetHelp(strHelpTopic);

			Dlg.SetPluginNumber(PluginNumber); // «апомним номер плагина

			if (IsWarningStyle)
			{
				Dlg.SetDialogMode(DMODE_WARNINGSTYLE);
			}

			Dlg.SetDialogMode(DMODE_MSGINTERNAL);
			if (Flags & MSG_NOPLUGINS)
				Dlg.SetDialogMode(DMODE_NOPLUGINS);
			FlushInputBuffer();

			if (Flags & MSG_KILLSAVESCREEN)
				SendDlgMessage((HANDLE)&Dlg,DM_KILLSAVESCREEN,0,0);

			Dlg.Process();
			RetCode=Dlg.GetExitCode();
		}

		delete [] MsgDlg;
		xf_free(Str);
		return(RetCode<0?RetCode:RetCode-StrCount-1-(Separator?1:0));
	}

	// *** Ѕез ƒиалога! ***
	SetCursorType(0,0);

	if (!(Flags & MSG_KEEPBACKGROUND))
	{
		SetScreen(X1,Y1,X2,Y2,L' ',(Flags & MSG_WARNING)?COL_WARNDIALOGTEXT:COL_DIALOGTEXT);
		MakeShadow(X1+2,Y2+1,X2+2,Y2+1);
		MakeShadow(X2+1,Y1+1,X2+2,Y2+1);
		Box(X1+3,Y1+1,X2-3,Y2-1,(Flags & MSG_WARNING)?COL_WARNDIALOGBOX:COL_DIALOGBOX,DOUBLE_BOX);
	}

	SetColor((Flags & MSG_WARNING)?COL_WARNDIALOGTEXT:COL_DIALOGTEXT);

	if (Title && *Title)
	{
		string strTempTitle = Title;

		if (strTempTitle.GetLength() > MaxLength)
			strTempTitle.SetLength(MaxLength);

		GotoXY(X1+(X2-X1-1-(int)strTempTitle.GetLength())/2,Y1+1);
		FS<<L" "<<strTempTitle<<L" ";
	}

	for (I=0; I<StrCount; I++)
	{
		int PosX;
		CPtrStr=Str[I];
		wchar_t Chr=*CPtrStr;

		if (Chr == 1 || Chr == 2)
		{
			int Length=X2-X1-5;

			if (Length>1)
			{
				SetColor((Flags & MSG_WARNING)?COL_WARNDIALOGBOX:COL_DIALOGBOX);
				GotoXY(X1+3,Y1+I+2);
				DrawLine(Length,(Chr == 2?3:1));
				CPtrStr++;
				int TextLength=StrLength(CPtrStr);

				if (TextLength<Length)
				{
					GotoXY(X1+3+(Length-TextLength)/2,Y1+I+2);
					Text(CPtrStr);
				}

				SetColor((Flags & MSG_WARNING)?COL_WARNDIALOGBOX:COL_DIALOGTEXT);
			}

			continue;
		}

		if ((Length=StrLength(CPtrStr))>ScrX-15)
			Length=ScrX-15;

		int Width=X2-X1+1;
		wchar_t *lpwszTemp = nullptr;

		if (Flags & MSG_LEFTALIGN)
		{
			lpwszTemp = (wchar_t*)xf_malloc((Width-10+1)*sizeof(wchar_t));
			_snwprintf(lpwszTemp,Width-10+1,L"%.*s",Width-10,CPtrStr);
			GotoXY(X1+5,Y1+I+2);
		}
		else
		{
			PosX=X1+(Width-Length)/2;
			lpwszTemp = (wchar_t*)xf_malloc((PosX-X1-4+Length+X2-PosX-Length-3+1)*sizeof(wchar_t));
			_snwprintf(lpwszTemp,PosX-X1-4+Length+X2-PosX-Length-3+1,L"%*s%.*s%*s",PosX-X1-4,L"",Length,CPtrStr,X2-PosX-Length-3,L"");
			GotoXY(X1+4,Y1+I+2);
		}

		Text(lpwszTemp);
		xf_free(lpwszTemp);
	}

	/* $ 13.01.2003 IS
	   - ѕринудительно уберем запрет отрисовки экрана, если количество кнопок
	     в сообщении равно нулю и макрос закончил выполн€тьс€. Ёто необходимо,
	     чтобы заработал прогресс-бар от плагина, который был запущен при помощи
	     макроса запретом отрисовки (bugz#533).
	*/
	xf_free(Str);

	if (!Buttons)
	{
		if (ScrBuf.GetLockCount()>0 && !CtrlObject->Macro.PeekKey())
			ScrBuf.SetLockCount(0);

		ScrBuf.Flush();
	}

	return 0;
}


void GetMessagePosition(int &X1,int &Y1,int &X2,int &Y2)
{
	X1=MessageX1;
	Y1=MessageY1;
	X2=MessageX2;
	Y2=MessageY2;
}

bool FormatErrorString(bool Nt, DWORD Code, string& Str)
{
	bool Result=false;
	LPWSTR lpBuffer=nullptr;
	Result=FormatMessage((Nt?FORMAT_MESSAGE_FROM_HMODULE:FORMAT_MESSAGE_FROM_SYSTEM)|FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS, (Nt?GetModuleHandle(L"ntdll.dll"):nullptr), Code, 0, reinterpret_cast<LPWSTR>(&lpBuffer), 0, nullptr)!=0;
	Str=lpBuffer;
	LocalFree(lpBuffer);
	RemoveUnprintableCharacters(Str);
	return Result;
}

bool GetWin32ErrorString(DWORD LastWin32Error, string& Str)
{
	return FormatErrorString(false, LastWin32Error, Str);
}

bool GetNtErrorString(NTSTATUS LastNtStatus, string& Str)
{
	return FormatErrorString(true, LastNtStatus, Str);
}

bool GetErrorString(string &strErrStr)
{
	bool Result=false;
	static struct TypeErrMsgs
	{
		DWORD WinMsg;
		int FarMsg;
	}
	ErrMsgs[]=
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
		{ERROR_NO_RECOVERY_POLICY,MErrorNoRecoveryPolicy},
		{ERROR_ENCRYPTION_FAILED,MErrorEncryptionFailed},
		{ERROR_DECRYPTION_FAILED,MErrorDecryptionFailed},
		{ERROR_FILE_NOT_ENCRYPTED,MErrorFileNotEncrypted},
		{ERROR_NO_ASSOCIATION,MErrorNoAssociation},
	};

	DWORD LastError = GetLastError();

	for (size_t i=0; i < ARRAYSIZE(ErrMsgs); i++)
	{
		if (ErrMsgs[i].WinMsg == LastError)
		{
			strErrStr = MSG(ErrMsgs[i].FarMsg);
			Result=true;
			break;
		}
	}

	if (!Result)
	{
		Result=GetWin32ErrorString(LastError, strErrStr);
	}

	return Result;
}


void SetMessageHelp(const wchar_t *Topic)
{
	strMsgHelpTopic = Topic;
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
