#define EXPORTLEN 70
#define BUTTONLEN 64
#define DIALOGWID 76
#define DIALOGHGT 24
#define MAXMENULEN 123
#define DATASIZE 32768

//-----------------------------------------------------------------//

LONG_PTR WINAPI MenuDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2)
{
	FarListPos ListPos;
	int Pos,i;
	MenuData *MData;

	if (Macro->HelpActivated)
	{
		Macro->HelpActivated=FALSE;
		Macro->WriteKeyBar(KB_COMMON);
	}

	switch (Msg)
	{
		case DN_MOUSECLICK:
		{
			MOUSE_EVENT_RECORD *MouseEvent=(MOUSE_EVENT_RECORD *)Param2;

			if (Macro->Conf.DblClick)
			{
				if (MouseEvent->dwEventFlags==DOUBLE_CLICK)
					MenuDialogProc(hDlg,DM_KEY,0,KEY_ENTER);
			}
			else
				MenuDialogProc(hDlg,DM_KEY,0,KEY_ENTER);

			// Сообщение обработано, больше не обрабатывать его.
			return TRUE;
		}
		case DN_DRAGGED:

			if (Param1==0)
				return TRUE;
			else if (Param1==1)
			{
				SMALL_RECT rect;

				if (Info.SendDlgMessage(hDlg,DM_GETDLGRECT,0,(LONG_PTR)&rect))
				{
					Macro->MenuX=rect.Left;
					Macro->MenuY=rect.Top;
				}
			}

			return 0;
		case DN_LISTCHANGE:
			Macro->SelectPos=(int)Param2;
			Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,(LONG_PTR)&ListPos);
//      if (ListPos.TopPos<=Macro->SelectPos)
			Macro->TopPos=ListPos.TopPos;
			return TRUE;
		case DN_INITDIALOG:
		{
			Macro->MenuDlg=hDlg;
			FarListTitles ListTitle;
			ListTitle.Title=Macro->MenuTitle;
			ListTitle.Bottom=Macro->MenuBottom;
			Info.SendDlgMessage(hDlg,DM_LISTSETTITLES,0,(LONG_PTR)&ListTitle);
			Macro->FillMenu(hDlg);
			return TRUE;
		}
		case DN_RESIZECONSOLE:
		{
			COORD coord=(*(COORD*)Param2);
			Macro->ConsoleSize=coord;
//      Macro->SaveBar=Info.SaveScreen(0,0,-1,-1);
			Macro->WriteKeyBar(KB_COMMON);
			Macro->FillMenu(hDlg,FALSE);
			return TRUE;
		}
		case DN_KEY:
		{
			Pos=(int)Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0/*(LONG_PTR)&ListPos*/);

			if (Pos == -1)
				return FALSE;

			MData=(MenuData *)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(LONG_PTR)Pos);

			if (*MData->Key=='~' && lstrlen(MData->Key)>1)
			{
				Macro->Deactivated=TRUE;
				lstrcpy(Macro->Key,&MData->Key[1]);
			}
			else
			{
				Macro->Deactivated=FALSE;
				lstrcpy(Macro->Key,MData->Key);
			}

			lstrcpy(Macro->Group,MData->Group);

			switch (Param2)
			{
					// Экспорт макро
				case KEY_F2:
					// Экспорт всех макро
				case KEY_SHIFTF2:
					Macro->ActiveMode=MAC_EXPORTACTIVE;
					Macro->ExportMacro(Param2&KEY_SHIFT?TRUE:FALSE);
					Macro->ActiveMode=MAC_MENUACTIVE;
					return TRUE;
					// Редактирование
				case KEY_F4:
				case KEY_ENTER:
					Macro->ActiveMode=MAC_EDITACTIVE;

					if (Macro->EditMacro())
						Macro->FillMenu(hDlg);

					Macro->ActiveMode=MAC_MENUACTIVE;
					return TRUE;
					// Добавление
				case KEY_INS:
					Macro->ActiveMode=MAC_EDITACTIVE;

					if (Macro->InsertMacro())
						Macro->FillMenu(hDlg);

					Macro->ActiveMode=MAC_MENUACTIVE;
					return TRUE;
					// Копирование - Перемещение
				case KEY_F5:
				case KEY_F6:
					Macro->ActiveMode=MAC_COPYACTIVE;

					if (Macro->CopyMacro((int)Param2))
						Macro->FillMenu(hDlg);

					Macro->ActiveMode=MAC_MENUACTIVE;
					return TRUE;
					// Удаление
				case KEY_DEL:
					Macro->ActiveMode=MAC_DELETEACTIVE;

					if (Macro->DeleteMacro())
						Macro->FillMenu(hDlg);

					Macro->ActiveMode=MAC_MENUACTIVE;
					return TRUE;
					// Группа вверх
				case KEY_CTRLUP:
				{
					int Pos1=(int)Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,(LONG_PTR)&ListPos);

					if (Pos1>=0)
					{
						for (i=Pos1; i>=0; (Macro->Conf.MenuCycle && i-1<0)?i=Macro->MenuItemsNumber-1:i--)
						{
							MenuData *MData1=(MenuData *)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(LONG_PTR)i);

							if (*MData1->Group && *MData1->Key)
							{
								if (CmpStr(MData->Group,MData1->Group)!=0)
								{
									for (; i>=0; i--)
									{
										MenuData *MData2=(MenuData *)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(LONG_PTR)i);

										if (CmpStr(MData1->Group,MData2->Group)!=0)
										{
											i++;
											break;
										}

										if (i==0) break;
									}

									// Запомним позицию и установим её.
									Macro->SelectPos=ListPos.SelectPos=i;
									Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,0,(LONG_PTR)&ListPos);
									Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,(LONG_PTR)&ListPos);
									// Получим текущую верхнюю позицию в списке.
//                  if (ListPos.TopPos<=Macro->SelectPos)
									Macro->TopPos=ListPos.TopPos;
									break;
								}
							}
						}
					}

					return TRUE;
				}
				// Группа вниз
				case KEY_CTRLDOWN:
				{
					int Pos1=(int)Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,(LONG_PTR)&ListPos);

					if (Pos1<Macro->MenuItemsNumber)
					{
						for (i=Pos1; i<Macro->MenuItemsNumber; i++)
						{
							MenuData *MData1=(MenuData *)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(LONG_PTR)i);

							if (*MData1->Group && *MData1->Key)
							{
								if (CmpStr(MData->Group,MData1->Group)!=0)
								{
									// Запомним позицию и установим её.
									Macro->SelectPos=ListPos.SelectPos=i;
									Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,0,(LONG_PTR)&ListPos);
									Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,(LONG_PTR)&ListPos);
									// Получим текущую верхнюю позицию в списке.
//                  if (ListPos.TopPos<=Macro->SelectPos)
									Macro->TopPos=ListPos.TopPos;
									break;
								}
							}
						}

						if (Macro->Conf.MenuCycle && i>=Macro->MenuItemsNumber)
						{
							// Запомним текущию верхнюю позицию и позицию курсора и установим их.
							Macro->SelectPos=Macro->TopPos=ListPos.SelectPos=ListPos.TopPos=0;
							Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,0,(LONG_PTR)&ListPos);
						}
					}

					return TRUE;
				}
			}

			return FALSE;
		}
		case DN_HELP:
		{
			const TCHAR *Topic[]={_T("Contents"),_T("MacroView")};
			Macro->HelpActivated=TRUE;

			if (Macro->HelpInvoked)
				return (LONG_PTR)Topic[1];

			Macro->HelpInvoked=TRUE;
			return (LONG_PTR)Topic[0];
		}
		case DN_CLOSE:
			Macro->MenuDlg=NULL;
			return TRUE;
		case DN_CTLCOLORDIALOG:
			return Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_MENUTEXT);
		case DN_CTLCOLORDLGLIST:
		{
			FarListColors *Colors=(FarListColors *)Param2;
			int ColorIndex[]={COL_MENUBOX,COL_MENUBOX,COL_MENUTITLE,COL_MENUTEXT,COL_MENUHIGHLIGHT,COL_MENUBOX,COL_MENUSELECTEDTEXT,COL_MENUSELECTEDHIGHLIGHT,COL_MENUSCROLLBAR,COL_MENUDISABLEDTEXT,COL_MENUARROWS,COL_MENUARROWSSELECTED,COL_MENUARROWSDISABLED,COL_MENUGRAYTEXT,COL_MENUSELECTEDGRAYTEXT};
			int Count=sizeof(ColorIndex)/sizeof(ColorIndex[0]);

			if (Count>Colors->ColorCount) Count=Colors->ColorCount;

			for (int i=0; i<Count; i++)
				Colors->Colors[i]=(BYTE)Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)(INT_PTR)(ColorIndex[i]));

			return TRUE;
		}
	}

	return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

LONG_PTR WINAPI MacroDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2)
{
	static int InProcess=FALSE;

	if (Macro->WaitForKeyToMacro)
	{
		if (!InProcess)
		{
			InProcess=TRUE;
			Info.SendDlgMessage(hDlg,DM_SETFOCUS,2,0);
			CONSOLE_CURSOR_INFO ci;
			BOOL result=GetConsoleCursorInfo(Macro->hOut,&ci);

			if (result)
			{
				ci.bVisible=FALSE;
				SetConsoleCursorInfo(Macro->hOut,&ci);
			}

#ifndef UNICODE
			Info.DialogEx(Info.ModuleNumber,-1,-1,40,5,NULL,Macro->DefKeyDialog,
			              ARRAYSIZE(Macro->DefKeyDialog),0,0,DefKeyDialogProc,0);
#else
			HANDLE hDefKeyDlg = Info.DialogInit(Info.ModuleNumber,-1,-1,40,5,NULL,Macro->DefKeyDialog,
			                                    ARRAYSIZE(Macro->DefKeyDialog),0,0,DefKeyDialogProc,0);

			if (hDefKeyDlg != INVALID_HANDLE_VALUE)
			{
				Info.DialogRun(hDefKeyDlg);
				Info.DialogFree(hDefKeyDlg);
			}

#endif
			Info.RestoreScreen(NULL);
			Info.RestoreScreen(Macro->SaveScr);
			Macro->SaveScr=NULL;
			Info.SendDlgMessage(hDlg,DM_SETFOCUS,Macro->LastFocus,0);

			if (result)
			{
				ci.bVisible=TRUE;
				SetConsoleCursorInfo(Macro->hOut,&ci);
			}

			Macro->WaitForKeyToMacro=FALSE;
			InProcess=FALSE;
		}
	}

	if (Macro->HelpActivated)
	{
		Macro->HelpActivated=FALSE;
		Macro->WriteKeyBar(KB_DIALOG);
	}

	switch (Msg)
	{
		case DN_INITDIALOG:
		{
			Macro->EditDlg=hDlg;

			switch (Macro->EditMode)
			{
				case EM_INSERT:
					Info.SendDlgMessage(hDlg,DM_SETFOCUS,2,0);
					break;
				case EM_EDIT:
					Info.SendDlgMessage(hDlg,DM_SETFOCUS,8,0);
					break;
			}

			Info.SendDlgMessage(hDlg,DM_LISTSET,4,(LONG_PTR)&Macro->GroupList);
			return TRUE;
		}
		case DN_RESIZECONSOLE:
		{
			Macro->WriteKeyBar(KB_DIALOG);
			return TRUE;
		}
		case DN_MOUSECLICK:

			if (Macro->CtrlDotPressed)
				return TRUE;

			return FALSE;
		case DN_HELP:

			if (Macro->CtrlDotPressed)
				return 0;
			else
				Macro->HelpActivated=TRUE;

			return Param2;
		case DN_CLOSE:

			if (Macro->CtrlDotPressed)
				return FALSE;

			Macro->EditMode=EM_NONE;
			Macro->EditDlg=NULL;
			return TRUE;
		case DN_KEY:
		{
			if (Macro->CtrlDotPressed)
			{
				DWORD Key=((DWORD)Param2 & ~KEY_CTRL & ~KEY_ALT & ~KEY_SHIFT & ~KEY_RCTRL & ~KEY_RALT & ~KEY_SHIFT);

				if (Key>=KEY_END_FKEY)
					return TRUE;

				static int PrevKey=0;

				if (PrevKey==KEY_SHIFTENTER && Param2==KEY_SHIFT)
				{
					PrevKey=(int)Param2;
					return TRUE;
				}

				if ((PrevKey==KEY_DOWN || PrevKey==KEY_UP ||
				        PrevKey==KEY_LEFT || PrevKey==KEY_RIGHT) &&
				        Param2==KEY_SHIFT)
				{
					PrevKey=(int)Param2;
					return TRUE;
				}

				PrevKey=(int)Param2;

				if (FSF.FarKeyToName((int)Param2,Macro->S,ARRAYSIZE(Macro->S)))
				{
					FarDialogItemData ItemData;
					TCHAR StrBuf1[1024],StrBuf2[1024];
					ZeroMemory(StrBuf1,sizeof(StrBuf1));
					ZeroMemory(StrBuf2,sizeof(StrBuf2));

					if (Macro->MacroData)
					{
						if (*Macro->MacroData)
						{
							lstrcat(Macro->MacroData,_T(" "));
							lstrcat(Macro->MacroData,Macro->S);
						}
						else
							lstrcpy(Macro->MacroData,Macro->S);

						ItemData.PtrLength=lstrlen(Macro->MacroData)+1;
						ItemData.PtrData=Macro->MacroData;
					}
					else
					{
						int Len=(int)Info.SendDlgMessage(hDlg,DM_GETTEXT,8,0);

						if (Len)
						{
							ItemData.PtrLength=Len;
							ItemData.PtrData=StrBuf1;
							Info.SendDlgMessage(hDlg,DM_GETTEXT,8,(LONG_PTR)&ItemData);
						}

						if (*StrBuf1)
							wsprintf(StrBuf2,_T("%s %s"),AllTrim(StrBuf1),Macro->S);
						else
							lstrcpy(StrBuf2,Macro->S);

						ItemData.PtrLength=lstrlen(StrBuf2)+1;
						ItemData.PtrData=StrBuf2;
					}

					Info.SendDlgMessage(hDlg,DM_SETTEXT,8,(LONG_PTR)&ItemData);
				}

				return TRUE;
			}

			return FALSE;
		}
		case DN_BTNCLICK:

#ifdef UNICODE
			if (Param1==31) // кнопка [Проверить]
			{
				ActlKeyMacro command;
				command.Command=MCMD_CHECKMACRO;
				command.Param.PlainText.Flags=0;
				command.Param.PlainText.SequenceText=(const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,8,0);

				Info.AdvControl(Info.ModuleNumber,ACTL_KEYMACRO,&command);
				if (command.Param.MacroResult.ErrCode != MPEC_SUCCESS)
				{
					Info.SendDlgMessage(hDlg,DM_SETFOCUS,8,0);
					// TODO:
					//    if (command.Param.MacroResult.ErrCode != MPEC_SUCCESS)
					//       set pos (ErrPos) in Item8
				}
				else
				{
					; // TODO: show "OK" dialog
				}
				return TRUE;
			}
			else
#endif
			if (Param1==16) // Active panel
			{
				// Запретим отрисовку экрана
				Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

				if (Param2==0) // Пометка снята
				{
					//Info.SendDlgMessage(hDlg,DM_SETCHECK,18,BSTATE_3STATE);
					//Info.SendDlgMessage(hDlg,DM_SETCHECK,20,BSTATE_3STATE);
					//Info.SendDlgMessage(hDlg,DM_SETCHECK,22,BSTATE_3STATE);
					Info.SendDlgMessage(hDlg,DM_ENABLE,18,FALSE);
					Info.SendDlgMessage(hDlg,DM_ENABLE,20,FALSE);
					Info.SendDlgMessage(hDlg,DM_ENABLE,22,FALSE);
				}
				else
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,18,TRUE);
					Info.SendDlgMessage(hDlg,DM_ENABLE,20,TRUE);
					Info.SendDlgMessage(hDlg,DM_ENABLE,22,TRUE);
				}

				// Разрешим отрисовку экрана
				Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
				return TRUE;
			}
			else if (Param1==17) // Passive panel
			{
				// Запретим отрисовку экрана
				Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

				if (Param2==0) // Пометка снята
				{
					//Info.SendDlgMessage(hDlg,DM_SETCHECK,19,BSTATE_3STATE);
					//Info.SendDlgMessage(hDlg,DM_SETCHECK,21,BSTATE_3STATE);
					//Info.SendDlgMessage(hDlg,DM_SETCHECK,23,BSTATE_3STATE);
					Info.SendDlgMessage(hDlg,DM_ENABLE,19,FALSE);
					Info.SendDlgMessage(hDlg,DM_ENABLE,21,FALSE);
					Info.SendDlgMessage(hDlg,DM_ENABLE,23,FALSE);
				}
				else
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,19,TRUE);
					Info.SendDlgMessage(hDlg,DM_ENABLE,21,TRUE);
					Info.SendDlgMessage(hDlg,DM_ENABLE,23,TRUE);
				}

				// Разрешим отрисовку экрана
				Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
				return TRUE;
			}
			break; // CHECK!!! ???

		case DN_DRAGGED:

			if (Macro->CtrlDotPressed)
				return FALSE;

			if (Param1==0)
			{
				Macro->EditInMove=TRUE;
				return TRUE;
			}
			else if (Param1==1)
			{
				Macro->EditInMove=FALSE;
				return TRUE;
			}
	}

	return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

LONG_PTR WINAPI DefKeyDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2)
{
	static int EnableClose=FALSE;

	switch (Msg)
	{
		case DN_INITDIALOG:
			Macro->DefDlg=hDlg;
			EnableClose=FALSE;
			return FALSE;
		case DN_MOUSECLICK:
			return TRUE;
		case DN_HELP:
			return 0;
		case DN_CLOSE:

			if (EnableClose)
			{
				EnableClose=FALSE;
				Macro->DefDlg=NULL;
				return TRUE;
			}

			return FALSE;
		case DN_KEY:

			if (Macro->WaitForKeyToMacro)
			{
				DWORD Key=((DWORD)Param2 & ~KEY_CTRL & ~KEY_ALT & ~KEY_SHIFT & ~KEY_RCTRL & ~KEY_RALT & ~KEY_SHIFT);

				if (Key>=KEY_END_FKEY)
					return TRUE;

				if (FSF.FarKeyToName((int)Param2,Macro->S,ARRAYSIZE(Macro->S)))
				{
					FarDialogItemData ItemData;
					TCHAR StrBuf[1024];
					lstrcpy(StrBuf,Macro->S);
					ItemData.PtrLength=lstrlen(StrBuf)+1;
					ItemData.PtrData=StrBuf;
					Info.SendDlgMessage(Macro->EditDlg,DM_SETTEXT,2,(LONG_PTR)&ItemData);
					EnableClose=TRUE;
					Info.SendDlgMessage(hDlg,DM_CLOSE,-1,0);
				}
			}

			return TRUE;
		case DN_DRAGGED:
			return FALSE;
	}

	return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

LONG_PTR WINAPI CopyDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2)
{
	FarListPos ListPos;

	switch (Msg)
	{
		case DN_INITDIALOG:
			ListPos.SelectPos=Macro->UserConfPos;
			ListPos.TopPos=0;
			Info.SendDlgMessage(hDlg,DM_LISTSET,2,(LONG_PTR)&Macro->ConfList);
			Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,2,(LONG_PTR)&ListPos);
			ListPos.SelectPos=0;
			ListPos.TopPos=0;
			Info.SendDlgMessage(hDlg,DM_LISTSET,4,(LONG_PTR)&Macro->GroupList);
			Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,4,(LONG_PTR)&ListPos);
			return FALSE;
		case DN_LISTCHANGE:

			if (Param1==2)
				Macro->UserConfPos=(int)Param2;
			else if (Param1==4)
				Macro->GroupPos=(int)Param2;

			return TRUE;
	}

	return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}


#if defined(__BORLANDC__)
#pragma argsused
#endif
BOOL WINAPI myReadConsoleInputA(HANDLE hConsole,PINPUT_RECORD ir,DWORD nNumber,LPDWORD nNumberOfRead)
{
	BOOL lResult=FALSE;

	if (!p_fnReadConsoleInputOrgA)
		return lResult;

	lResult=(*p_fnReadConsoleInputOrgA)(hConsole,ir,nNumber,nNumberOfRead);
	/*  if (lResult && ir->EventType==KEY_EVENT)
	  {
	    if (!ProcessKey(ir))
	      ZeroMemory(&ir->Event.KeyEvent,sizeof(ir->Event.KeyEvent));
	  }*/
	return lResult;
}

BOOL WINAPI myReadConsoleInputW(HANDLE hConsole,PINPUT_RECORD ir,DWORD nNumber,LPDWORD nNumberOfRead)
{
	BOOL lResult=FALSE;

	if (!p_fnReadConsoleInputOrgW)
		return lResult;

	lResult=(*p_fnReadConsoleInputOrgW)(hConsole,ir,nNumber,nNumberOfRead);
	/*  if (lResult && ir->EventType==KEY_EVENT)
	  {
	    if (!ProcessKey(ir))
	      ZeroMemory(&ir->Event.KeyEvent,sizeof(ir->Event.KeyEvent));
	  }*/
	return lResult;
}

BOOL WINAPI myPeekConsoleInputA(HANDLE hConsole,PINPUT_RECORD ir,DWORD nNumber,LPDWORD nNumberOfRead)
{
	BOOL lResult=FALSE;

	if (!p_fnPeekConsoleInputOrgA)
		return lResult;

	lResult=(*p_fnPeekConsoleInputOrgA)(hConsole,ir,nNumber,nNumberOfRead);

	if (lResult && ir->EventType==KEY_EVENT)
	{
		if (!ProcessPeekKey(ir))
		{
			FlushInputBuffer();
			ZeroMemory(&ir->Event.KeyEvent,sizeof(ir->Event.KeyEvent));
			ir->EventType=0;
		}
	}

	return lResult;
}

BOOL WINAPI myPeekConsoleInputW(HANDLE hConsole,PINPUT_RECORD ir,DWORD nNumber,LPDWORD nNumberOfRead)
{
	BOOL lResult=FALSE;

	if (!p_fnPeekConsoleInputOrgW)
		return lResult;

	lResult=(*p_fnPeekConsoleInputOrgW)(hConsole,ir,nNumber,nNumberOfRead);

	if (lResult && ir->EventType==KEY_EVENT)
	{
		if (!ProcessPeekKey(ir))
		{
			FlushInputBuffer();
			ZeroMemory(&ir->Event.KeyEvent,sizeof(ir->Event.KeyEvent));
			ir->EventType=0;
		}
	}

	return lResult;
}

BOOL WINAPI ProcessKey(PINPUT_RECORD ir)
{
	/*  DWORD KeyState=ir->Event.KeyEvent.dwControlKeyState;
	  WORD ScanCode=ir->Event.KeyEvent.wVirtualScanCode;
	  WORD KeyCode=ir->Event.KeyEvent.wVirtualKeyCode;

	  CtrlPressed=(KeyState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED));
	  AltPressed=(KeyState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
	  ShiftPressed=(KeyState & SHIFT_PRESSED);

	  if (ir->Event.KeyEvent.bKeyDown)
	  {
	    switch(Macro->ActiveMode)
	    {
	      case MAC_MENUACTIVE:
	        if (KeyCode==VK_F2 && !CtrlPressed && !AltPressed && !ShiftPressed) // F2 - Export
	        {
	//          ZeroMemory(&ir->Event.KeyEvent,sizeof(ir->Event.KeyEvent));
	          MenuDialogProc(Macro->MenuDlg,DN_KEY,0,(LONG_PTR)KEY_F2);
	          return FALSE;
	        }
	        else if (KeyCode==VK_F2 && !CtrlPressed && !AltPressed && ShiftPressed) // ShiftF2 - Export all
	        {
	//          ZeroMemory(&ir->Event.KeyEvent,sizeof(ir->Event.KeyEvent));
	          MenuDialogProc(Macro->MenuDlg,DN_KEY,0,(LONG_PTR)KEY_SHIFTF2);
	          return FALSE;
	        }
	        else if (KeyCode==VK_INSERT && !CtrlPressed && !AltPressed && !ShiftPressed) // Ins - New
	        {
	//          ZeroMemory(&ir->Event.KeyEvent,sizeof(ir->Event.KeyEvent));
	          MenuDialogProc(Macro->MenuDlg,DN_KEY,0,(LONG_PTR)KEY_INS);
	          return FALSE;
	        }
	        else if (KeyCode==VK_DELETE && !CtrlPressed && !AltPressed && !ShiftPressed) // Del - Delete
	        {
	//          ZeroMemory(&ir->Event.KeyEvent,sizeof(ir->Event.KeyEvent));
	          MenuDialogProc(Macro->MenuDlg,DN_KEY,0,(LONG_PTR)KEY_DEL);
	          return FALSE;
	        }
	        else if ((KeyCode==VK_F4 || KeyCode==VK_RETURN) && !CtrlPressed && !AltPressed && !ShiftPressed) // F4,Enter - Edit
	        {
	//          ZeroMemory(&ir->Event.KeyEvent,sizeof(ir->Event.KeyEvent));
	          MenuDialogProc(Macro->MenuDlg,DN_KEY,0,(LONG_PTR)KEY_F4);
	          return FALSE;
	        }
	        else if (KeyCode==VK_F5 && !CtrlPressed && !AltPressed && !ShiftPressed) // F5 - Copy
	        {
	//          ZeroMemory(&ir->Event.KeyEvent,sizeof(ir->Event.KeyEvent));
	          MenuDialogProc(Macro->MenuDlg,DN_KEY,0,(LONG_PTR)KEY_F5);
	          return FALSE;
	        }
	        else if (KeyCode==VK_F6 && !CtrlPressed && !AltPressed && !ShiftPressed) // F6 - Move
	        {
	//          ZeroMemory(&ir->Event.KeyEvent,sizeof(ir->Event.KeyEvent));
	          MenuDialogProc(Macro->MenuDlg,DN_KEY,0,(LONG_PTR)KEY_F6);
	          return FALSE;
	        }
	        else*/ /*if (!CtrlPressed && !AltPressed && !ShiftPressed)
          Macro->WriteKeyBar(KB_COMMON);
        else if (ShiftPressed && !CtrlPressed && !AltPressed)
          Macro->WriteKeyBar(KB_SHIFT);
        else if (CtrlPressed && !AltPressed && !ShiftPressed)
          Macro->WriteKeyBar(KB_CTRL);
        else if (AltPressed && !CtrlPressed && !ShiftPressed)
          Macro->WriteKeyBar(KB_ALT);
        break;
      case MAC_EDITACTIVE:
        if (KeyCode==0xbe && CtrlPressed && !AltPressed) // Ctrl-Dot pressed
        {
          if (Macro->HelpActivated || Macro->EditInMove)
          {
//            ZeroMemory(&ir->Event.KeyEvent,sizeof(ir->Event.KeyEvent));
            return FALSE;
          }

          if (Macro->WaitForKeyToMacro)
          {
//            ZeroMemory(&ir->Event.KeyEvent,sizeof(ir->Event.KeyEvent));
            DefKeyDialogProc(Macro->DefDlg,DN_KEY,0,(LONG_PTR)KEY_CTRLDOT);
            return FALSE;
          }

          if (Info.SendDlgMessage(Macro->EditDlg,DM_GETDROPDOWNOPENED,0,0))
            Info.SendDlgMessage(Macro->EditDlg,DM_SETDROPDOWNOPENED,0,FALSE);

          Macro->CtrlDotPressed=(Macro->CtrlDotPressed)?FALSE:TRUE;
          if (!Macro->SaveScr)
          {
            Macro->SaveScr=Info.SaveScreen(0,0,lstrlen(Macro->MacroText),0);
            Info.Text(0,0,0x4F,(TCHAR *)Macro->MacroText);
            Macro->LastFocus=Info.SendDlgMessage(Macro->EditDlg,DM_GETFOCUS,0,0);
            Info.SendDlgMessage(Macro->EditDlg,DM_SETFOCUS,8,0);
          }
          else
            Macro->WaitForKeyToMacro=TRUE;
//          ZeroMemory(&ir->Event.KeyEvent,sizeof(ir->Event.KeyEvent));
          return FALSE;
        }

        if (Macro->WaitForKeyToMacro)
        {
          if (KeyCode==VK_F9 && AltPressed && !CtrlPressed && !ShiftPressed)
            DefKeyDialogProc(Macro->DefDlg,DN_KEY,8,(LONG_PTR)KEY_ALTF9);
          else if (KeyCode==VK_F5 && !AltPressed && CtrlPressed && !ShiftPressed)
            DefKeyDialogProc(Macro->DefDlg,DN_KEY,8,(LONG_PTR)KEY_CTRLF5);
          else if (KeyCode==VK_TAB && !AltPressed && CtrlPressed && !ShiftPressed)
            DefKeyDialogProc(Macro->DefDlg,DN_KEY,8,(LONG_PTR)KEY_CTRLTAB);
          else if (KeyCode==VK_TAB && !AltPressed && CtrlPressed && ShiftPressed)
            DefKeyDialogProc(Macro->DefDlg,DN_KEY,8,(LONG_PTR)KEY_CTRLSHIFTTAB);
          else if (KeyCode==VK_F11 && !AltPressed && !CtrlPressed && !ShiftPressed)
            DefKeyDialogProc(Macro->DefDlg,DN_KEY,8,(LONG_PTR)KEY_F11);
          else if (KeyCode==VK_F12 && !AltPressed && !CtrlPressed && !ShiftPressed)
            DefKeyDialogProc(Macro->DefDlg,DN_KEY,8,(LONG_PTR)KEY_F12);
          else
            break;
//          ZeroMemory(&ir->Event.KeyEvent,sizeof(ir->Event.KeyEvent));
          return FALSE;
        }
        if (Macro->CtrlDotPressed)
        {
          if (KeyCode==VK_F9 && AltPressed && !CtrlPressed && !ShiftPressed)
            MacroDialogProc(Macro->EditDlg,DN_KEY,8,(LONG_PTR)KEY_ALTF9);
          else if (KeyCode==VK_F5 && !AltPressed && CtrlPressed && !ShiftPressed)
            MacroDialogProc(Macro->EditDlg,DN_KEY,8,(LONG_PTR)KEY_CTRLF5);
          else if (KeyCode==VK_TAB && !AltPressed && CtrlPressed && !ShiftPressed)
            MacroDialogProc(Macro->EditDlg,DN_KEY,8,(LONG_PTR)KEY_CTRLTAB);
          else if (KeyCode==VK_TAB && !AltPressed && CtrlPressed && ShiftPressed)
            MacroDialogProc(Macro->EditDlg,DN_KEY,8,(LONG_PTR)KEY_CTRLSHIFTTAB);
          else if (KeyCode==VK_F11 && !AltPressed && !CtrlPressed && !ShiftPressed)
            MacroDialogProc(Macro->EditDlg,DN_KEY,8,(LONG_PTR)KEY_F11);
          else if (KeyCode==VK_F12 && !AltPressed && !CtrlPressed && !ShiftPressed)
            MacroDialogProc(Macro->EditDlg,DN_KEY,8,(LONG_PTR)KEY_F12);
          else
            break;
//          ZeroMemory(&ir->Event.KeyEvent,sizeof(ir->Event.KeyEvent));
          return FALSE;
        }
        else
        {
          if (!CtrlPressed && !AltPressed && !ShiftPressed)
            Macro->WriteKeyBar(KB_DIALOG);
          else if (ShiftPressed && !CtrlPressed && !AltPressed)
            Macro->WriteKeyBar(KB_SHIFTDIALOG);
          else if (CtrlPressed && !AltPressed && !ShiftPressed)
            Macro->WriteKeyBar(KB_CTRL);
          else if (AltPressed && !CtrlPressed && !ShiftPressed)
            Macro->WriteKeyBar(KB_ALT);
        }
        break;
      case MAC_DELETEACTIVE:
      case MAC_EXPORTACTIVE:
      case MAC_COPYACTIVE:
      case MAC_ERRORACTIVE:
        if (!CtrlPressed && !AltPressed && !ShiftPressed)
          Macro->WriteKeyBar(KB_DIALOG);
        else if (ShiftPressed && !CtrlPressed && !AltPressed)
          Macro->WriteKeyBar(KB_SHIFTDIALOG);
        else if (CtrlPressed && !AltPressed && !ShiftPressed)
          Macro->WriteKeyBar(KB_CTRL);
        else if (AltPressed && !CtrlPressed && !ShiftPressed)
          Macro->WriteKeyBar(KB_ALT);
        break;
      default:
        break;
    }
  }
  else
  {
    switch(Macro->ActiveMode)
    {
      case MAC_ERRORACTIVE:
        break;
      case MAC_MENUACTIVE:
        if (!CtrlPressed && !AltPressed && !ShiftPressed)
          Macro->WriteKeyBar(KB_COMMON);
        break;
      case MAC_EDITACTIVE:
      case MAC_DELETEACTIVE:
      case MAC_EXPORTACTIVE:
      case MAC_COPYACTIVE:
        if (!CtrlPressed && !AltPressed && !ShiftPressed)
          Macro->WriteKeyBar(KB_DIALOG);
      default:
        break;
    }
  }*/
	return TRUE;
}

BOOL __fastcall ProcessPeekKey(PINPUT_RECORD ir)
{
	DWORD Key,CKey;
	DWORD KeyState=ir->Event.KeyEvent.dwControlKeyState;
	//WORD ScanCode=ir->Event.KeyEvent.wVirtualScanCode;
	WORD KeyCode=ir->Event.KeyEvent.wVirtualKeyCode;
	CtrlPressed=(KeyState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED));
	AltPressed=(KeyState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
	ShiftPressed=(KeyState & SHIFT_PRESSED);
//  FILE *stream=fopen("log.log","at+");
//  fprintf(stream,"\n\r\n\rScanCode=%x\n\rKeyCode=%x\n\r\tCtrlPressed=%x\n\r\tAltPressed=%x\n\r\tShiftPressed=%x\n\r",ScanCode,KeyCode,CtrlPressed,AltPressed,ShiftPressed);
//  fclose(stream);

	if (KeyCode==0)
		return TRUE;

	if (ir->Event.KeyEvent.bKeyDown)
	{
		switch (Macro->ActiveMode)
		{
			case MAC_MENUACTIVE:

				if (!CtrlPressed && !AltPressed && !ShiftPressed)
					Macro->WriteKeyBar(KB_COMMON);
				else if (ShiftPressed && !CtrlPressed && !AltPressed)
					Macro->WriteKeyBar(KB_SHIFT);
				else if (CtrlPressed && !AltPressed && !ShiftPressed)
					Macro->WriteKeyBar(KB_CTRL);
				else if (AltPressed && !CtrlPressed && !ShiftPressed)
					Macro->WriteKeyBar(KB_ALT);

				break;
			case MAC_EDITACTIVE:

				// Включен режим копирования текста с экрана?
				if (Macro->AltInsPressed)
				{
					// Запретим в режиме копирования текста с экрана работать Ctrl-Dot
					if (KeyCode==0xbe && CtrlPressed && !AltPressed) // Ctrl-Dot pressed
						return FALSE;

					if (KeyCode==VK_ESCAPE && !CtrlPressed && !AltPressed && !ShiftPressed) // Esc pressed
						Macro->AltInsPressed=FALSE;
					else if (KeyCode==VK_RETURN && !CtrlPressed && !AltPressed && !ShiftPressed) // Enter pressed
						Macro->AltInsPressed=FALSE;
					else if (KeyCode==VK_INSERT && CtrlPressed && !AltPressed && !ShiftPressed) // Ctrl-Ins pressed
						Macro->AltInsPressed=FALSE;

					return TRUE;
				}

				// Если не в режиме ввода макроса
				if (!Macro->WaitForKeyToMacro && !Macro->CtrlDotPressed && !Macro->AltInsPressed)
				{
					// Включим режим копирования текста с экрана
					if (KeyCode==VK_INSERT && !CtrlPressed && AltPressed && !ShiftPressed) // Alt-Ins pressed
					{
						Macro->AltInsPressed=TRUE;
						return TRUE;
					}
				}

				// Включим режим ввода макроса
				if (KeyCode==0xbe && CtrlPressed && !AltPressed) // Ctrl-Dot pressed
				{
					// Включена помощь, диалог перемещается или режим копирования текста с экрана
					if (Macro->HelpActivated || Macro->EditInMove || Macro->AltInsPressed)
						return FALSE;

					if (Macro->WaitForKeyToMacro)
					{
						DefKeyDialogProc(Macro->DefDlg,DN_KEY,0,(LONG_PTR)KEY_CTRLDOT);
						return FALSE;
					}

					if (Info.SendDlgMessage(Macro->EditDlg,DM_GETDROPDOWNOPENED,0,0))
						Info.SendDlgMessage(Macro->EditDlg,DM_SETDROPDOWNOPENED,0,FALSE);

					Macro->CtrlDotPressed=(Macro->CtrlDotPressed)?FALSE:TRUE;

					if (!Macro->SaveScr)
					{
						Macro->SaveScr=Info.SaveScreen(0,0,lstrlen(Macro->MacroText),0);
						Info.Text(0,0,0x4F,(TCHAR *)Macro->MacroText);
						Macro->LastFocus=(int)Info.SendDlgMessage(Macro->EditDlg,DM_GETFOCUS,0,0);
						Info.SendDlgMessage(Macro->EditDlg,DM_SETFOCUS,8,0);
					}
					else
						Macro->WaitForKeyToMacro=TRUE;

					return FALSE;
				}

				// Ожидаем клавишу активации макрокоманды.
				if (Macro->WaitForKeyToMacro)
				{
					// Запретим в режиме ввода макроса копировать текст с экрана
					if (KeyCode==VK_INSERT && !CtrlPressed && AltPressed && !ShiftPressed) // Alt-Ins запрещено
						return FALSE;

					Key=FSF.FarInputRecordToKey(ir);

					if (Key!=KEY_NONE && Key!=KEY_IDLE)
					{
						CKey=(Key & ~KEY_CTRL & ~KEY_ALT & ~KEY_SHIFT & ~KEY_RCTRL & ~KEY_RALT);

						if ((CKey<KEY_END_FKEY && CKey>=0x20) || CKey==0x08 || CKey==0x09 || CKey==0x0d || CKey==0x1b)
						{
							if (CKey==0x0d && (KeyState & SHIFT_PRESSED)) // Shift-Enter
								Key|=KEY_SHIFT;

							DefKeyDialogProc(Macro->DefDlg,DN_KEY,0,(LONG_PTR)Key);
							// Завершающая стадия, поэтому очистим событие, но вернём TRUE
							ZeroMemory(&ir->Event.KeyEvent,sizeof(ir->Event.KeyEvent));
							return TRUE;
						}
					}

					return FALSE;
				}

				// Ctrl-. нажата - обрабатываем клавиши.
				if (Macro->CtrlDotPressed)
				{
					// Запретим в режиме ввода макроса копировать текст с экрана
					if (KeyCode==VK_INSERT && !CtrlPressed && AltPressed && !ShiftPressed) // Alt-Ins запрещено
						return FALSE;

					Key=FSF.FarInputRecordToKey(ir);

					if (Key!=KEY_NONE && Key!=KEY_IDLE)
					{
						CKey=(Key & ~KEY_CTRL & ~KEY_ALT & ~KEY_SHIFT & ~KEY_RCTRL & ~KEY_RALT);

						if ((CKey<KEY_END_FKEY && CKey>=0x20) || CKey==0x08 || CKey==0x09 || CKey==0x0d || CKey==0x1b)
						{
							if (CKey==0x0d && (KeyState & SHIFT_PRESSED)) // Shift-Enter
								Key|=KEY_SHIFT;

							MacroDialogProc(Macro->EditDlg,DN_KEY,8,(LONG_PTR)Key);
						}
					}

					return FALSE;
				}
				else
				{
					if (!CtrlPressed && !AltPressed && !ShiftPressed)
						Macro->WriteKeyBar(KB_DIALOG);
					else if (ShiftPressed && !CtrlPressed && !AltPressed)
						Macro->WriteKeyBar(KB_SHIFTDIALOG);
					else if (CtrlPressed && !AltPressed && !ShiftPressed)
						Macro->WriteKeyBar(KB_CTRL);
					else if (AltPressed && !CtrlPressed && !ShiftPressed)
						Macro->WriteKeyBar(KB_ALT);
				}

				break;
			case MAC_DELETEACTIVE:
			case MAC_EXPORTACTIVE:
			case MAC_COPYACTIVE:
			case MAC_ERRORACTIVE:

				if (!CtrlPressed && !AltPressed && !ShiftPressed)
					Macro->WriteKeyBar(KB_DIALOG);
				else if (ShiftPressed && !CtrlPressed && !AltPressed)
					Macro->WriteKeyBar(KB_SHIFTDIALOG);
				else if (CtrlPressed && !AltPressed && !ShiftPressed)
					Macro->WriteKeyBar(KB_CTRL);
				else if (AltPressed && !CtrlPressed && !ShiftPressed)
					Macro->WriteKeyBar(KB_ALT);

				break;
		}
	}
	else
	{
		switch (Macro->ActiveMode)
		{
			case MAC_MENUACTIVE:

				if (!CtrlPressed && !AltPressed && !ShiftPressed)
					Macro->WriteKeyBar(KB_COMMON);

				break;
			case MAC_EDITACTIVE:

				if (Macro->CtrlDotPressed || Macro->WaitForKeyToMacro)
					break;

			case MAC_DELETEACTIVE:
			case MAC_EXPORTACTIVE:
			case MAC_COPYACTIVE:
			case MAC_ERRORACTIVE:

				if (!CtrlPressed && !AltPressed && !ShiftPressed)
					Macro->WriteKeyBar(KB_DIALOG);

				break;
		}
	}

	return TRUE;
}


TMacroView::TMacroView():
		MacroText(_T("MACRO")),
		MacroCmdHistory(_T("MacroCmd")),
		MacroKeyHistory(_T("MacroKey")),
		MacroDescrHistory(_T("MacroDescr")),
		MacroExpHistory(_T("MacroExp")),
		MacroCopyHistory(_T("MacroCopy"))
{
	hOut=GetStdHandle(STD_OUTPUT_HANDLE);
	hIn=GetStdHandle(STD_INPUT_HANDLE);
	MacroData=new TCHAR[DATASIZE];

	if (MacroData)
	{
		TCHAR *tmp=new TCHAR[DATASIZE];

		if (tmp)
			delete[] tmp;
		else
		{
			delete[] MacroData;
			MacroData=NULL;
		}
	}

	InitData();
}


TMacroView::~TMacroView()
{
	if (MacroData)
	{
		delete[] MacroData;
		MacroData=NULL;
	}
}

void __fastcall TMacroView::ReadConsoleSize()
{
	SHORT x=80, y=25;
#ifndef UNICODE
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	if (GetConsoleScreenBufferInfo(hOut,&csbi))
	{
		x=csbi.dwSize.X;
		y=csbi.dwSize.Y;
	}

#else
	SMALL_RECT console_rect;

	if (Info.AdvControl(Info.ModuleNumber,ACTL_GETFARRECT,(void*)&console_rect))
	{
		x=console_rect.Right-console_rect.Left+1;
		y=console_rect.Bottom-console_rect.Top+1;
	}

#endif
	ConsoleSize.X=x;
	ConsoleSize.Y=y;
}

void __fastcall TMacroView::InitMacroAreas()
{
	int i;
	TCHAR *GroupNumbers[]=
	{
		(TCHAR *)MMacroNameDialog,
		(TCHAR *)MMacroNameDisks,
		(TCHAR *)MMacroNameEditor,
		(TCHAR *)MMacroNameHelp,
		(TCHAR *)MMacroNameInfoPanel,
		(TCHAR *)MMacroNameMainMenu,
		(TCHAR *)MMacroNameMenu,
		(TCHAR *)MMacroNameQviewPanel,
		(TCHAR *)MMacroNameSearch,
		(TCHAR *)MMacroNameShell,
		(TCHAR *)MMacroNameTreePanel,
		(TCHAR *)MMacroNameViewer,
		(TCHAR *)MMacroNameOther,
		(TCHAR *)MMacroNameCommon,
		(TCHAR *)MMacroNameFindFolder,
		(TCHAR *)MMacroNameUserMenu,
#ifdef UNICODE
		(TCHAR *)MMacroNameAutoCompletion,
#endif
	};
	MacroGroupsSize=ARRAYSIZE(MacroGroupShort);
//  MacroGroupsSize=ARRAYSIZE(GroupNumbers);

	for (i=0; i<MacroGroupsSize; i++)
	{
		if (Conf.LongGroupNames)
#ifndef UNICODE
			lstrcpyn(GroupItems[i].Text,GetMsg((unsigned)(DWORD_PTR)GroupNumbers[i]),ARRAYSIZE(GroupItems[i].Text));

#else
			GroupItems[i].Text=GetMsg((unsigned)(DWORD_PTR)GroupNumbers[i]);
#endif
		else
#ifndef UNICODE
			lstrcpyn(GroupItems[i].Text,MacroGroupShort[i],ARRAYSIZE(GroupItems[i].Text));

#else
			GroupItems[i].Text=MacroGroupShort[i];
#endif
		GroupItems[i].Flags=0;
	}

	GroupList.ItemsNumber=MacroGroupsSize;
	GroupList.Items=GroupItems;
}

void __fastcall TMacroView::InitDialogs()
{
	InitMacroAreas();
	/*
	  0000000000111111111122222222223333333333444444444455555555556666666666777777
	  0123456789012345678901234567890123456789012345678901234567890123456789012345
	00""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""00
	01"  |========================== Macro settings ==========================|  "01
	02"  | Command of execution            | Work area                        |  "02
	03"  | -------------------------------| ------------------------------- |  "03
	04"  |---------------------------------+----------------------------------|  "04
	05"  | Sequence                                                           |  "05
	06"  | ----------------------------------------------------------------- |  "06
	07"  | Description                                                        |  "07
	08"  | ----------------------------------------------------------------- |  "08
	09"  |---------------------------------+----------------------------------|  "09
	10"  | [ ] Run after FAR start         | [x] Disable screen output        |  "10
	11"  | [?] Command line state          | [?] Block selection presents     |  "11
	12"  | [ ] Active panel                | [ ] Passive panel                |  "12
	13"  |   [?] Plugin/file panel         |   [?] Plugin/file panel          |  "13
	14"  |   [?] Folder/file under cursor  |   [?] Folder/file under cursor   |  "14
	15"  |   [?] Folder/file selected      |   [?] Folder/file selected       |  "15
	16"  |---------------------------------+----------------------------------|  "16
	17"  | [x] Send macro to plugins                                          |  "17
	18"  |--------------------------------------------------------------------|  "18
	19"  | [ ] Deactivate macro                                               |  "19
	20"  |--------------------------------------------------------------------|  "20
	21"  |                         [ Save ]  [ Cancel ]                       |  "21
	22"  |====================================================================|  "22
	23"                                                                          "23
	24""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""24
	  0000000000111111111122222222223333333333444444444455555555556666666666777777
	  0123456789012345678901234567890123456789012345678901234567890123456789012345
	*/
//TODO: Use BoxSymbols?
#ifndef UNICODE
#define _BOX_VTOP "\xB3\xB3\xC1"
#define _BOX_VFUL "\xC2\xB3\xB3\xB3\xB3\xB3\xB3\xC1"
#define _BOX_VERT "\xB3"
#else
#define _BOX_VTOP L"\x2502\x2502\x2534"
#define _BOX_VFUL L"\x252C\x2502\x2502\x2502\x2502\x2502\x2502\x2534"
#define _BOX_VERT L"\x2502"
#endif
	InitDialogItem InitItems[]=
	{

		/* 0*/  {DI_DOUBLEBOX,  3, 1,DIALOGWID-4,DIALOGHGT-2,0,0,0,0,(TCHAR *)MMacroCombination},
		/* 1*/  {DI_TEXT,       5, 2, 0,0,0,0,0,0,CheckLen(GetMsg(MMacroKey),30)},
		/* 2*/  {DI_EDIT,       5, 3,35,3,0,(DWORD_PTR)MacroKeyHistory,DIF_HISTORY,0,_T("")},
		/* 3*/  {DI_TEXT,      39, 2, 0,0,0,0,0,0,CheckLen(GetMsg(MMacroGroup),30)},
		/* 4*/  {DI_COMBOBOX,  39, 3,69,3,0,0,DIF_DROPDOWNLIST,0,_T("")},
		/* 5*/  {DI_TEXT,       5, 4, 0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
		/* 6*/  {DI_VTEXT,     37, 2, 0,0,0,0,0,0,_BOX_VTOP},
		/* 7*/  {DI_TEXT,       5, 5, 0,0,0,0,0,0,CheckLen(GetMsg(MMacroCommand),64)},
		/* 8*/  {DI_EDIT,       5, 6,69,0,1,(DWORD_PTR)MacroCmdHistory,
#ifndef UNICODE
		         (MacroData)?DIF_HISTORY|DIF_VAREDIT :
#endif
		         DIF_HISTORY,0,_T("")},
		/* 9*/  {DI_TEXT,       5, 7, 0,0,0,0,0,0,CheckLen(GetMsg(MMacroDescription),64)},
		/*10*/  {DI_EDIT,       5, 8,69,0,0,(DWORD_PTR)MacroDescrHistory,DIF_HISTORY,0,_T("")},
		/*11*/  {DI_TEXT,       5, 9, 0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},

		/*12*/  {DI_CHECKBOX,   5,10, 0,0,0,0,0,0,CheckLen(GetMsg(MMacroRunAfterStart),27)},
		/*13*/  {DI_CHECKBOX,  39,10, 0,0,0,0,0,0,CheckLen(GetMsg(MMacroDisable),28)},

		/*14*/  {DI_CHECKBOX,   5,11, 0,0,0,0,DIF_3STATE,0,CheckLen(GetMsg(MMacroComState),27)},
		/*15*/  {DI_CHECKBOX,  39,11, 0,0,0,0,DIF_3STATE,0,CheckLen(GetMsg(MMacroSelection),28)},

		/*16*/  {DI_CHECKBOX,   5,12, 0,0,0,0,0,0,CheckLen(GetMsg(MMacroAPanel),27)},
		/*17*/  {DI_CHECKBOX,  39,12, 0,0,0,0,0,0,CheckLen(GetMsg(MMacroPPanel),28)},

		/*18*/  {DI_CHECKBOX,   7,13, 0,0,0,0,DIF_3STATE,0,CheckLen(GetMsg(MMacroFilePlugPanel),26)},
		/*19*/  {DI_CHECKBOX,  41,13, 0,0,0,0,DIF_3STATE,0,CheckLen(GetMsg(MMacroFilePlugPanel),27)},

		/*20*/  {DI_CHECKBOX,   7,14, 0,0,0,0,DIF_3STATE,0,CheckLen(GetMsg(MMacroFolderFileUnder),26)},
		/*21*/  {DI_CHECKBOX,  41,14, 0,0,0,0,DIF_3STATE,0,CheckLen(GetMsg(MMacroFolderFileUnder),27)},

		/*22*/  {DI_CHECKBOX,   7,15, 0,0,0,0,DIF_3STATE,0,CheckLen(GetMsg(MMacroFolderFileSelected),26)},
		/*23*/  {DI_CHECKBOX,  41,15, 0,0,0,0,DIF_3STATE,0,CheckLen(GetMsg(MMacroFolderFileSelected),27)},

		/*24*/  {DI_TEXT,       5,16, 0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
		/*25*/  {DI_VTEXT,     37, 9, 0,0,0,0,0,0,_BOX_VFUL},

		/*26*/  {DI_CHECKBOX,   5,17, 0,0,0,0,0,0,CheckLen(GetMsg(MMacroSendToPlugins),62)},

		/*27*/  {DI_TEXT,       5,18, 0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},

		/*28*/  {DI_CHECKBOX,   5,19, 0,0,0,0,0,0,CheckLen(GetMsg(MMacroSwitchOff),62)},

		/*29*/  {DI_TEXT,       5,20, 0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},

		/*30*/  {DI_BUTTON,     0,21, 0,0,0,0,DIF_CENTERGROUP,1,CheckLen(GetMsg(MMacroSave),64)},
#ifdef UNICODE
		/*31*/  {DI_BUTTON,     0,21, 0,0,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,CheckLen(GetMsg(MMacroCheck),64)},
#endif
		/*32*/  {DI_BUTTON,     0,21, 0,0,0,0,DIF_CENTERGROUP,0,CheckLen(GetMsg(MMacroCancel),64)},
	};
	InitDialogItems(InitItems,EditDialog,ARRAYSIZE(InitItems));

	InitDialogItem DefItems[]=
	{

		/* 0*/  {DI_DOUBLEBOX,  3, 1,40-4,5-2,0,0,0,0,(TCHAR *)MMacroDefineTitle},
		/* 1*/  {DI_TEXT,       5, 2, 0,0,0,0,DIF_CENTERGROUP,0,CheckLen(GetMsg(MMacroDesiredKey),28)},

	};
	InitDialogItems(DefItems,DefKeyDialog,ARRAYSIZE(DefItems));

#ifdef UNICODE
	EditDialog[2].PtrData = _Group;
	EditDialog[4].PtrData = _Button;
	EditDialog[10].PtrData = _Descr;
	EditDialog[8].PtrData = _DataPtr = MacroData?MacroData:_Data;
	_DataPtrSize = MacroData?DATASIZE:ARRAYSIZE(_Data);
#else
#define _Group EditDialog[2].Data.Data
#define _Button EditDialog[4].Data.Data
#define _Descr EditDialog[10].Data.Data
#define _DataPtr EditDialog[8].Data.Data
#endif
}


void __fastcall TMacroView::InitData()
{
	CtrlDotPressed=FALSE;
	WaitForKeyToMacro=FALSE;
	AltInsPressed=FALSE;
	HelpInvoked=FALSE;
	HelpActivated=FALSE;
	EditInMove=FALSE;
	hand=NULL;
	EditDlg=NULL;
	MenuDlg=NULL;
	DefDlg=NULL;
	SaveScr=NULL;
//  SaveBar=NULL;
	p_fnReadConsoleInputOrgA=NULL;
	p_fnReadConsoleInputOrgW=NULL;
	p_fnPeekConsoleInputOrgA=NULL;
	p_fnPeekConsoleInputOrgW=NULL;
	NameList=NULL;
	MacNameList=NULL;
	DescrList=NULL;
	ValueList=NULL;
	MenuList=NULL;
	EditMode=EM_NONE;
	SelectPos=0;
	TopPos=0;
	GroupPos=0;
	UserConfPos=0;
	LastFocus=0;
	MenuX=MenuY=-1;
	MenuH=MenuW=0;
	EditX1=EditY1=EditX2=EditY2=0;
	*Group=*Key=0;
	ReadConsoleSize();
	ReadConfig();

	if (MacroData)
		ZeroMemory(MacroData,DATASIZE);

	InitDialogs();
#ifdef UNICODE
	_Button[0] = _Group[0] = _Descr[0] = _Data[0] = 0;
#endif
}


/*void __fastcall TMacroView::ParseMenuItem(FarListGetItem *List)
{
  TCHAR *ptr,*ptr1;
  *Group=*Key=0;

  if (List->Item.Flags&LIF_CHECKED)
    Deactivated=TRUE;
  else
    Deactivated=FALSE;
  lstrcpy(S,List->Item.Text);
//  while((ptr=_tcschr(S,_T('&')))!=NULL) memmove(ptr,ptr+1,lstrlen(S)-(ptr-S)+1);

  ptr=_tcschr(S,_T(':'));
  if (ptr)
  {
    int len=ptr-S;
    lstrcpyn(Group,S,len+1);
    ptr1=S+GroupKeyLen;
    if (ptr1)
    {
      int len=ptr1-ptr;
      lstrcpyn(Key,ptr+1,len+1);
    }
  }

  AllTrim(Group);
  ConvertGroupName(UnQuoteText(Group),GRP_TOSHORTNAME);
  AllTrim(Key);
  UnQuoteText(Key);
}*/


void __fastcall TMacroView::WriteKeyBar(int kbType)
{
	SMALL_RECT rect;
	ReadConsoleSize();

	if (MenuDlg)
	{
		Info.SendDlgMessage(MenuDlg,DM_GETDLGRECT,0,(LONG_PTR)&rect);

		if (rect.Bottom>=ConsoleSize.Y-1)
			return;

		if (HelpActivated)
			return;
	}

	if (EditDlg)
	{
		Info.SendDlgMessage(EditDlg,DM_GETDLGRECT,0,(LONG_PTR)&rect);

		if (rect.Bottom>=ConsoleSize.Y-1)
			return;

		if (HelpActivated)
			return;
	}

	TCHAR *KeyBarCommon[12]={GetMsg(MMacroKBHelp),
	                         GetMsg(MMacroKBSave),NULL,GetMsg(MMacroKBEdit),
	                         GetMsg(MMacroKBCopy),GetMsg(MMacroKBMove),
	                         NULL,NULL,NULL,
	                         GetMsg(MMacroKBQuit),NULL,NULL,
	                        };
	TCHAR *KeyBarAlt[12]={NULL,
	                      NULL,NULL,NULL,
	                      NULL,NULL,
	                      NULL,NULL,NULL,
	                      NULL,NULL,NULL,
	                     };
	TCHAR *KeyBarCtrl[12]={NULL,
	                       NULL,NULL,NULL,
	                       NULL,NULL,
	                       NULL,NULL,NULL,
	                       NULL,NULL,NULL,
	                      };
	TCHAR *KeyBarShift[12]={NULL,
	                        GetMsg(MMacroKBAltSaveAll),NULL,NULL,
	                        NULL,NULL,
	                        NULL,NULL,NULL,
	                        NULL,NULL,NULL,
	                       };
	TCHAR *KeyBarDialog[12]={GetMsg(MMacroKBHelp),
	                         NULL,NULL,NULL,
	                         NULL,NULL,
	                         NULL,NULL,NULL,
	                         GetMsg(MMacroKBQuit),NULL,NULL,
	                        };
	TCHAR *KeyBarShiftDialog[12]={NULL,
	                              NULL,NULL,NULL,
	                              NULL,NULL,
	                              NULL,NULL,NULL,
	                              NULL,NULL,NULL,
	                             };
	TCHAR *KeyBar[12];
	//Узнаем ширину экрана консоли
	//int ScrWidth=ConsoleSize.X+1;

	switch (kbType)
	{
		case KB_COMMON:
			CopyMemory(KeyBar,KeyBarCommon,sizeof(KeyBarCommon));
			break;
		case KB_ALT:
			CopyMemory(KeyBar,KeyBarAlt,sizeof(KeyBarAlt));
			break;
		case KB_CTRL:
			CopyMemory(KeyBar,KeyBarCtrl,sizeof(KeyBarCtrl));
			break;
		case KB_SHIFT:
			CopyMemory(KeyBar,KeyBarShift,sizeof(KeyBarShift));
			break;
		case KB_DIALOG:
			CopyMemory(KeyBar,KeyBarDialog,sizeof(KeyBarDialog));
			break;
		case KB_SHIFTDIALOG:
			CopyMemory(KeyBar,KeyBarShiftDialog,sizeof(KeyBarShiftDialog));
			break;
	}

	int i,j,k;
	CHAR_INFO *chi=new CHAR_INFO[ConsoleSize.X];
	COORD Size,Coord;
	SMALL_RECT Region;
	Size.X=ConsoleSize.X;
	Size.Y=1;
	Coord.X=0;
	Coord.Y=0;
	Region.Left=0;
	Region.Top=ConsoleSize.Y-1;
	Region.Right=ConsoleSize.X-1;
	Region.Bottom=ConsoleSize.Y-1;
	ReadConsoleOutput(hOut,chi,Size,Coord,&Region);
	int ColDigit=(int)Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_KEYBARNUM);
	int ColText=(int)Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_KEYBARTEXT);
#ifndef UNICODE
#define _CH AsciiChar
#else
#define _CH UnicodeChar
#endif

	if (chi[0].Char._CH==_T('1') && chi[0].Attributes==ColDigit && chi[1].Attributes==ColText)
	{
		for (i=0,j=1; j<ConsoleSize.X; i++)
		{
			if (KeyBar[i] && *KeyBar[i])
			{
				k=0;

				while (chi[j].Attributes==ColText && j<ConsoleSize.X)
				{
					if (KeyBar[i][k]!=0)
						chi[j++].Char._CH=KeyBar[i][k++];
					else
						chi[j++].Char._CH=_T(' ');
				}
			}
			else
			{
				while (chi[j].Attributes==ColText && j<ConsoleSize.X)
				{
					chi[j++].Char._CH=_T(' ');
				}
			}

#undef _CH

			while (chi[j].Attributes!=ColText && j<ConsoleSize.X)
				j++;
		}

		WriteConsoleOutput(hOut,chi,Size,Coord,&Region);
	}

	delete[] chi;
}


BOOL __fastcall TMacroView::CreateDirs(TCHAR *Dir)
{
	TCHAR Str[MAX_PATH_LEN];
	const TCHAR *ErrorCreateFolder[]=
	{
		GetMsg(MMacroError),
		GetMsg(MMacroErrorCreateFolder),
		S,
		_T("\x1"),
		GetMsg(MMacroOk),
	};
	TCHAR *Data=Dir;
	TCHAR *ptrend=Data+lstrlen(Data);
	TCHAR *ptr;

	// сетевой путь?
	if ((Data[0]==_T('\\')) && (Data[1]==_T('\\')))
	{
		ptr=_tcschr(&Data[2],_T('\\'));

		if (ptr==NULL
		        || (ptr=_tcschr(ptr+1,_T('\\')))==NULL
		        || ptr+1>=ptrend
		        || _tcschr(ptr+1,_T('\\'))==NULL) return TRUE;

		Data=ptr+1;
	}
	else
	{
		if (_tcschr(Data,_T('\\')) == NULL)
			return TRUE;

		if (Data[0] != _T('\\') && Data[1] == _T(':'))
			Data += 2;

		if (Data[0] == _T('\\'))
			++Data;

		if (Data >= ptrend)
			return TRUE;
	}

	BOOL endstr=FALSE;

	while (1)
	{
		ZeroMemory(Str,sizeof(Str));
		ptr=_tcschr(Data,_T('\\'));

		if (ptr==NULL || ptr>=ptrend)
		{
			lstrcpy(Str,Dir);
			endstr=TRUE;
		}
		else
		{
			lstrcpyn(Str,Dir,(int)(ptr-Dir)+1);
		}

		if (*Str && (!CreateDirectory(Str,NULL)))
		{
			if (GetLastError()!=ERROR_ALREADY_EXISTS)
			{
				lstrcpy(S,Dir);
				CheckRLen(S,EXPORTLEN-8);
				int OldActive=ActiveMode;
				ActiveMode=MAC_ERRORACTIVE;
				Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,ErrorCreateFolder,
				             ARRAYSIZE(ErrorCreateFolder),1);
				ActiveMode=OldActive;
				return FALSE;
			}
		}

		if (endstr)
			break;

		Data=ptr+1;
	}

	return TRUE;
}


TCHAR *TMacroView::ConvertGroupName(TCHAR *Group,int nWhere)
{
	int i,len;

	if (Conf.LongGroupNames)
	{
		len=lstrlen(Group);

		if (len>=3)
			if ((Group[len-1]==_T('.')) && (Group[len-2]==_T('.')) && (Group[len-3]==_T('.')))
				Group[len-3]=0;

		if (len>0)
		{
			switch (nWhere)
			{
				case GRP_TOLONGNAME:

					for (i=0; i<MacroGroupsSize; i++)
					{
						// Предполагаем, что Group короткое имя, поэтому
						// производим поиск в массиве коротких имён
						if (CmpStr(Group,MacroGroupShort[i])==0)
						{
							lstrcpy(Group,GroupItems[i].Text);
							break;
						}
					}

					break;
				case GRP_TOSHORTNAME:

					for (i=0; i<MacroGroupsSize; i++)
					{
						// Предполагаем, что Group длинное имя, поэтому
						// производим поиск в массиве длинных имён
						if (CmpStr(Group,GroupItems[i].Text)==0)
						{
							lstrcpy(Group,MacroGroupShort[i]);
							break;
						}
					}

					break;
			}
		}
	}

	return Group;
}


void TMacroView::InitDialogItems(InitDialogItem *Init,FarDialogItem *Item,
                                 int ItemsNumber)
{
	for (int I=0; I<ItemsNumber; I++)
	{
		Item[I].Type=Init[I].Type;
		Item[I].X1=Init[I].X1;
		Item[I].Y1=Init[I].Y1;
		Item[I].X2=Init[I].X2;
		Item[I].Y2=Init[I].Y2;
		Item[I].Focus=Init[I].Focus;
		Item[I].Param.Reserved=Init[I].Selected;
		Item[I].Flags=Init[I].Flags;
		Item[I].DefaultButton=Init[I].DefaultButton;

		if ((DWORD_PTR)Init[I].Data<300)
#ifndef UNICODE
			lstrcpy(Item[I].Data.Data,GetMsg((unsigned int)(DWORD_PTR)Init[I].Data));
		else
			lstrcpy(Item[I].Data.Data,Init[I].Data);

#else
			Item[I].PtrData = GetMsg((unsigned)(DWORD_PTR)Init[I].Data);
		else
			Item[I].PtrData = Init[I].Data;

		Item[I].MaxLen = 0;
#endif
	}
}


void __fastcall TMacroView::InsertMacroToEditor(BOOL AllMacros)
{
	TCHAR lKey[MAX_KEY_LEN];
	TCHAR lGroup[MAX_KEY_LEN];
	const TCHAR *TmpPrfx=_T("mvu");
	HANDLE fname;
	const TCHAR *ErrorRun[]=
	{
		GetMsg(MMacroError),
		GetMsg(MMacroErrorRun),
		_T("\x1"),
		GetMsg(MMacroOk),
	};
	const TCHAR *ErrorCreateTemp[]=
	{
		GetMsg(MMacroError),
		GetMsg(MMacroErrorCreateTemp),
		_T("\x1"),
		GetMsg(MMacroOk),
	};
	const TCHAR *ErrorReadTemp[]=
	{
		GetMsg(MMacroError),
		GetMsg(MMacroErrorReadTemp),
		_T("\x1"),
		GetMsg(MMacroOk),
	};
	const TCHAR *ErrorInsertStrEditor[]=
	{
		GetMsg(MMacroError),
		GetMsg(MMacroErrorInsertStrEditor),
		S,
		_T("\x1"),
		GetMsg(MMacroOk),
	};
#ifndef UNICODE
	SetFileApisToANSI();
#endif
	/*  if (GetEnvironmentVariable(TempEnvName,TempPath,ARRAYSIZE(TempPath))==0)
	    if (GetCurrentDirectory(ARRAYSIZE(TempPath),TempPath)==0)
	      lstrcpy(TempPath,_T("C:\\"));*/

//  if (GetTempFileName(TempPath,TmpPrfx,0,TempFileName)==0)
	if (FSF.MkTemp(TempFileName,
#ifdef UNICODE
	               sizeof(TempFileName),TmpPrfx)==1)
#else
	               TmpPrfx)==NULL)
#endif
	{
		int OldActive=ActiveMode;
		ActiveMode=MAC_ERRORACTIVE;
		Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,ErrorCreateTemp,
		             ARRAYSIZE(ErrorCreateTemp),1);
		ActiveMode=OldActive;
#ifndef UNICODE
		SetFileApisToOEM();
#endif
		return;
	}
	if (*Group && *Key)
	{
		lstrcpy(lKey,Key);
		lstrcpy(lGroup,Group);

		if (Deactivated)
		{
			lstrcpy(lKey,_T("~"));
			lstrcat(lKey,Key);
		}

		if (AllMacros)
			lstrcpy(S,KeyMacros);
		else
			wsprintf(S,_T("%s\\%s\\%s"),KeyMacros,lGroup,lKey);

		CheckFirstBackSlash(S,FALSE);
		ZeroMemory(&si,sizeof(si));
		si.cb=sizeof(si);
		TCHAR regedit[32];
		BOOL reu = (vi.dwPlatformId==VER_PLATFORM_WIN32_NT && vi.dwMajorVersion>=5);
#ifndef UNICODE

		if (reu)
			lstrcpy(regedit,_T("regedit -ea"));
		else
#endif
			lstrcpy(regedit,_T("regedit -e"));

		wsprintf(Str,_T("%s \"%s\" \"%s\\%s\""),regedit,TempFileName,HKCU,S);
		LPTSTR CurDir=NULL;
#ifdef UNICODE
		DWORD Size=FSF.GetCurrentDirectory(0,NULL);

		if (Size)
		{
			CurDir=new WCHAR[Size];
			FSF.GetCurrentDirectory(Size,CurDir);
		}

#endif
		int Code=CreateProcess(NULL,Str,NULL,NULL,TRUE,0,NULL,CurDir,&si,&pi);

		if (CurDir)
		{
			delete[] CurDir;
		}

		if (Code)
		{
			WaitForSingleObject(pi.hProcess,INFINITE);
			GetExitCodeProcess(pi.hProcess,(LPDWORD)&Code);
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}
		else
		{
			int OldActive=ActiveMode;
			ActiveMode=MAC_ERRORACTIVE;
			Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,ErrorRun,
			             ARRAYSIZE(ErrorRun),1);
			ActiveMode=OldActive;
#ifndef UNICODE
			SetFileApisToOEM();
#endif
			return;
		}

		fname=CreateFile(TempFileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);

		if (fname==INVALID_HANDLE_VALUE)
		{
			int OldActive=ActiveMode;
			ActiveMode=MAC_ERRORACTIVE;
			Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ErrorReadTemp,
			             ARRAYSIZE(ErrorReadTemp),1);
			ActiveMode=OldActive;
#ifndef UNICODE
			SetFileApisToOEM();
#endif
			return;
		}

		int iteration=1;
		int indent=1;
		DWORD t;

		while (ReadFile(fname,S,sizeof(S)-sizeof(S[0]),&t,NULL))
		{
			DWORD i;
#ifdef UNICODE

			if (reu) t /= sizeof(TCHAR);

#endif

			if (!t) break;

#ifdef UNICODE

			if (reu)
			{
#endif

				for (i=0; i<t; i++)
					if (S[i]==_T('\n') || S[i]==_T('\r')) break;

				S[i] = 0;

				if (i < t && ++i < t && S[i-1] == _T('\r') && S[i] == _T('\n')) ++i;

				t = (t-i)*sizeof(TCHAR);
#ifdef UNICODE
			}
			else    // NT4/win9x
			{
				char tmp[ARRAYSIZE(S)], *SA = (char*)S;

				for (i=0; i<t; i++)
					if ((SA[i]=='\n') || (SA[i]=='\r')) break;

				S[i] = 0;

				if (i < t && ++i < t && SA[i-1] == '\r' && SA[i] == '\n') ++i;

				t -= i;
				lstrcpyA(tmp, SA);
				i = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, tmp, -1, S, ARRAYSIZE(S)-1);
				S[i] = 0;
			}

#endif

			if (t) SetFilePointer(fname,-(int)t,NULL,FILE_CURRENT);

			if (!S[0]) continue;

			if (iteration==1)
			{
				if (Info.EditorControl(ECTL_INSERTTEXT,S)==0)
				{
					lstrcpy(S,lGroup);
					ConvertGroupName(S,GRP_TOLONGNAME);
					int OldActive=ActiveMode;
					ActiveMode=MAC_ERRORACTIVE;
					Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ErrorInsertStrEditor,
					             ARRAYSIZE(ErrorInsertStrEditor),1);
					ActiveMode=OldActive;
					break;
				}
			}
			else
			{
				if ((Info.EditorControl(ECTL_INSERTSTRING,&indent)==0) ||
				        (Info.EditorControl(ECTL_INSERTTEXT,S)==0))
				{
					lstrcpy(S,lGroup);
					ConvertGroupName(S,GRP_TOLONGNAME);
					int OldActive=ActiveMode;
					ActiveMode=MAC_ERRORACTIVE;
					Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ErrorInsertStrEditor,
					             ARRAYSIZE(ErrorInsertStrEditor),1);
					ActiveMode=OldActive;
					break;
				}
			}

			iteration++;
		}

		Info.EditorControl(ECTL_REDRAW,NULL);
		CloseHandle(fname);
	}
	DeleteFile(TempFileName);
#ifndef UNICODE
	SetFileApisToOEM();
#endif
}


void __fastcall TMacroView::ExportMacroToFile(BOOL AllMacros)
{
	TCHAR lKey[MAX_KEY_LEN];
	TCHAR lGroup[MAX_KEY_LEN];
	TCHAR sDest[EXPORTLEN];
	TCHAR MacroHelp[64];
#ifdef UNICODE
	wchar_t _group[MAX_PATH_LEN], _fname[MAX_PATH_LEN];
#endif
	const TCHAR *TmpPrfx=_T("mvu");
	const TCHAR *ErrorRun[]=
	{
		GetMsg(MMacroError),
		GetMsg(MMacroErrorRun),
		_T("\x1"),
		GetMsg(MMacroOk),
	};
	const TCHAR *ErrorExists[]=
	{
		GetMsg(MMacroWarning),GetMsg(MMacroWarningFileExists),
		S,_T("\x1"),sDest,_T("\x1"),
		GetMsg(MMacroOverwrite),GetMsg(MMacroRename),GetMsg(MMacroCancel)
	};
	const TCHAR *ErrorCreateFile[]=
	{
		GetMsg(MMacroError),
		GetMsg(MMacroErrorCreateFile),
		Str,
		_T("\x1"),
		GetMsg(MMacroOk),
	};

	if (*Group && *Key)
	{
		lstrcpy(lKey,Key);
		lstrcpy(lGroup,Group);

		if (Deactivated)
		{
			lstrcpy(lKey,_T("~"));
			lstrcat(lKey,Key);
		}

		struct InitDialogItem InitItems[]=
		{
			{DI_DOUBLEBOX,3,1,EXPORTLEN+2,6,0,0,0,0,(TCHAR *)MMacroExport},
			{DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,_T("")},
			{DI_EDIT,5,3,EXPORTLEN,3,1,(DWORD_PTR)MacroExpHistory,DIF_HISTORY,0,_T("")},
			{DI_TEXT,5,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
			{DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)MMacroSave},
			{DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)MMacroCancel}
		};

		FarDialogItem DialogItems[ARRAYSIZE(InitItems)];

		InitDialogItems(InitItems,DialogItems,ARRAYSIZE(InitItems));

		wsprintf(Str,_T("%s: %s"),ConvertGroupName(Group,GRP_TOLONGNAME),Key);

		QuoteText(Str,TRUE);

		if (AllMacros)
		{
#ifndef UNICODE
			lstrcpy(DialogItems[1].Data.Data,GetMsg(MMacroExportAllKey));
			lstrcpy(DialogItems[2].Data.Data,_T("KeyMacros.reg"));
#else
			DialogItems[1].PtrData = GetMsg(MMacroExportAllKey);
			DialogItems[2].PtrData = _T("KeyMacros.reg");
#endif
			lstrcpy(MacroHelp,_T("MacroExportAll"));
		}
		else
		{
#ifdef UNICODE
			DialogItems[1].PtrData = _group;
			DialogItems[2].PtrData = _fname;
#define _N1  _group
#define _N2  _fname
#else
#define _N1  DialogItems[1].Data.Data
#define _N2  DialogItems[2].Data.Data
#endif
			wsprintf(_N1, GetMsg(MMacroExportKey), Str);
			wsprintf(_N2, _T("%s.reg"), Key);
#undef _N2
#undef _N1
			lstrcpy(MacroHelp,_T("MacroExport"));
		}

		WriteKeyBar(KB_DIALOG);
		int ExitCode;
#ifdef UNICODE
		HANDLE hDlg = INVALID_HANDLE_VALUE;
#endif
MACRO_DIALOG:
#ifndef UNICODE
		ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,EXPORTLEN+6,8,MacroHelp,DialogItems,
		                     ARRAYSIZE(DialogItems));
#else

		if (hDlg != INVALID_HANDLE_VALUE)
			Info.DialogFree(hDlg);

		hDlg=Info.DialogInit(Info.ModuleNumber,-1,-1,EXPORTLEN+6,8,MacroHelp,DialogItems,ARRAYSIZE(DialogItems),0,0,NULL,0);

		if (hDlg != INVALID_HANDLE_VALUE)
		{
			ExitCode=Info.DialogRun(hDlg);
#endif

		if (ExitCode==4) // Сохранить
		{
			ZeroMemory(&si,sizeof(si));
			si.cb=sizeof(si);
			const TCHAR *ptr=_tcsrchr(GetDataPtr(2),_T('\\'));

			if (ptr)
			{
				ZeroMemory(Str,sizeof(Str));
				lstrcpyn(Str,GetDataPtr(2),(int)(ptr-GetDataPtr(2))+1);

				if (!CreateDirs(Str))
					goto MACRO_DIALOG;
			}

#ifndef UNICODE
			char AnsiName[MAX_PATH_LEN];
			SetFileApisToANSI();
			OemToChar(DialogItems[2].Data.Data,AnsiName);
#else
#define AnsiName (const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,2,0)
#endif
			ZeroMemory(TempPath,sizeof(TempPath));

			if (ptr)
			{
				lstrcpyn(TempPath,GetDataPtr(2),(int)(ptr-GetDataPtr(2))+1);
				wsprintf(TempFileName,_T("%s\\%s%x.tmp"),TempPath,TmpPrfx,102938);
			}
			else
			{
				lstrcpy(TempPath,_T(""));
				wsprintf(TempFileName,_T("%s%x.tmp"),TmpPrfx,102938);
			}

			HANDLE hTemp;

			if ((hTemp=CreateFile(TempFileName,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
			                      CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,NULL))==INVALID_HANDLE_VALUE)
			{
				lstrcpy(Str,GetDataPtr(2));
				CheckRLen(Str,EXPORTLEN-8);
				int OldActive=ActiveMode;
				ActiveMode=MAC_ERRORACTIVE;
				Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,ErrorCreateFile,
				             ARRAYSIZE(ErrorCreateFile),1);
				ActiveMode=OldActive;
				goto MACRO_DIALOG;
			}

			CloseHandle(hTemp);
			DeleteFile(TempFileName);
			hand=FindFirstFile(AnsiName,&fData);

			if (hand!=INVALID_HANDLE_VALUE)
			{
				FILETIME ft;
				SYSTEMTIME st;
				FileTimeToLocalFileTime(&fData.ftLastWriteTime,&ft);
				FileTimeToSystemTime(&ft,&st);
				//wsprintf(sDest,"  %-24s %10lu %02u.%02u.%02u %02u:%02u:%02u  ",
				FSF.sprintf(sDest,_T("  %-*s %10lu %02u.%02u.%02u %02u:%02u:%02u  "),24,
				            GetMsg(MMacroDestination),
				            (fData.nFileSizeHigh*MAXDWORD)+fData.nFileSizeLow,
				            st.wDay,st.wMonth,st.wYear,
				            st.wHour,st.wMinute,st.wSecond);
				lstrcpy(S,GetDataPtr(2));
				CheckRLen(S,EXPORTLEN-8);
				int OldActive=ActiveMode;
				ActiveMode=MAC_ERRORACTIVE;
				int ExitCode=Info.Message(Info.ModuleNumber,FMSG_WARNING,_T("MacroExportExist"),ErrorExists,
				                          ARRAYSIZE(ErrorExists),3);
				ActiveMode=OldActive;
				FindClose(hand);

				switch (ExitCode)
				{
					case 0:
						break;
					case 1:
#ifndef UNICODE
						SetFileApisToOEM();
#endif
						goto MACRO_DIALOG;
					default:
#ifndef UNICODE
						SetFileApisToOEM();
#endif
						return;
				}
			}

			if (AllMacros)
				lstrcpy(S,KeyMacros);
			else
				wsprintf(S,_T("%s\\%s\\%s"),KeyMacros,lGroup,lKey);

			CheckFirstBackSlash(S,FALSE);
			TCHAR regedit[32];
#ifndef UNICODE

			if ((vi.dwPlatformId==VER_PLATFORM_WIN32_NT) && (vi.dwMajorVersion>=5))
				lstrcpy(regedit,_T("regedit -ea"));
			else
#endif
				lstrcpy(regedit,_T("regedit -e"));

			wsprintf(Str,_T("%s \"%s\" \"%s\\%s\""),regedit,AnsiName,HKCU,S);
#ifdef UNICODE
#undef AnsiName
#endif
			int Code=CreateProcess(NULL,Str,NULL,NULL,TRUE,
			                       0,NULL,NULL,&si,&pi);

			if (Code)
			{
				WaitForSingleObject(pi.hProcess,INFINITE);
				GetExitCodeProcess(pi.hProcess,(LPDWORD)&Code);
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);

				if (Code)
				{
					int OldActive=ActiveMode;
					ActiveMode=MAC_ERRORACTIVE;
					Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,ErrorRun,
					             ARRAYSIZE(ErrorRun),1);
					ActiveMode=OldActive;
				}
			}
			else
			{
				int OldActive=ActiveMode;
				ActiveMode=MAC_ERRORACTIVE;
				Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,ErrorRun,
				             ARRAYSIZE(ErrorRun),1);
				ActiveMode=OldActive;
			}

#ifndef UNICODE
			SetFileApisToOEM();
#endif
		}

#ifdef UNICODE
		Info.DialogFree(hDlg);
	}

#endif
}
}

void TMacroView::SwitchOver(const TCHAR *Group,const TCHAR *Key)
{
	TCHAR lKey[MAX_KEY_LEN];
	TCHAR lGroup[MAX_KEY_LEN];

	if (*Group && *Key)
	{
		lstrcpy(lKey,Key);
		lstrcpy(lGroup,Group);

		if (Deactivated)
		{
			lstrcpy(lKey,_T("~"));
			lstrcat(lKey,Key);
		}

		wsprintf(S,_T("%s\\%s\\%s"),KeyMacros,lGroup,lKey); //полное имя ключа реестра
		CheckFirstBackSlash(S,TRUE);

		if ((EditDialog[28].Param.Selected) && (!Deactivated))  // отключить макрокоманду
		{
			wsprintf(Str,_T("%s\\%s\\~%s"),KeyMacros,lGroup,lKey);
			Reg->MoveKey(S,Str); //(OldName,NewName);
		}
		else if ((!EditDialog[28].Param.Selected) && (Deactivated)) // восстановить макрокоманду
		{
			wsprintf(Str,_T("%s\\%s\\%s"),KeyMacros,lGroup,&lKey[1]);
			Reg->MoveKey(S,Str); //(OldName,NewName);
		}
	}
}


BOOL TMacroView::DeletingMacro(const TCHAR **Items,int ItemsSize,const TCHAR *HelpTopic)
{
	TCHAR lKey[MAX_KEY_LEN];
	int lCode;

	if (*Group && *Key)
	{
		lstrcpy(lKey,Key);

		if (Deactivated)
		{
			lstrcpy(lKey,_T("~"));
			lstrcat(lKey,Key);
		}

		lCode=Info.Message(Info.ModuleNumber,FMSG_WARNING,HelpTopic,Items,ItemsSize,3);
		wsprintf(S,_T("%s\\%s\\%s"),KeyMacros,Group,lKey); //полное имя ключа реестра
		CheckFirstBackSlash(S,TRUE);

		if (lCode==0)
		{
			Reg->DeleteKey(S);
			wsprintf(S,_T("%s\\%s"),KeyMacros,Group);
			Reg->OpenKey(S);

			if (!Reg->HasSubKeys())
				Reg->DeleteKey(S);

			Reg->CloseKey();
			return DM_DELETED;
		}
		else if (lCode==1)
		{
			if (Deactivated)  //TRUE - восстановить
				wsprintf(Str,_T("%s\\%s\\%s"),KeyMacros,Group,&lKey[1]);
			else         //FALSE - отключить
				wsprintf(Str,_T("%s\\%s\\~%s"),KeyMacros,Group,lKey);

			Reg->MoveKey(S,Str); //(OldName,NewName);
			return DM_DEACTIVATED;
		}
	}

	return DM_NONE;
}


BOOL __fastcall TMacroView::CopyMoveMacro(int Op)
{
	BOOL lResult=FALSE;
	TCHAR S1[MAX_PATH_LEN];
	TCHAR LocalKeyMacros[MAX_PATH_LEN];
	TCHAR lKey[MAX_KEY_LEN];
	TCHAR lGroup[MAX_KEY_LEN];
	// Если пользовательских конфигураций в фаре нет, то
	// количество пользователей = 1 (Основная конфигурация)
	int UserCount=1;
	// Индекс текущей конфигурации в списке пользовательских конфигураций.
	UserConfPos=0;
	// Индекс области копирования.
	GroupPos=0;
	const TCHAR *ItemsErrorConf[]=
	{
		GetMsg(MMacroError),
		GetMsg(MMacroErrorSelectConfiguration),
		_T("\x1"),
		GetMsg(MMacroOk),
	};
	const TCHAR *ItemsCopy[]=
	{
		GetMsg(MMacroWarning),
		S,
		_T("\x1"),
		GetMsg(MMacroOk),
	};
	const TCHAR *ItemsExist[]=
	{
		GetMsg(MMacroWarning),GetMsg(MMacroWarningExist),
		S,
		_T("\x1"),
		GetMsg(MMacroOverwrite),GetMsg(MMacroCancel)
	};
	const TCHAR *ItemsKeyEmp[]=
	{
		GetMsg(MMacroWarning),
		S,
		_T("\x1"),
		GetMsg(MMacroOk),
	};
	// Создадим локальную копию адреса расположения макросов в реестре
	// в основной конфигурации пользователя.
	lstrcpy(LocalKeyMacros,KeyMacros);
	// список, в который будем читать из реестра имена пользовательских конфигураций.
	TStrList UserList;
	FarListItem *ConfItems=NULL;

	if (!Reg->OpenKey(FarUsersKey))
	{
		// Создадим первый элемент списка конфигураций - общий.
		ConfItems=new FarListItem[1];

		if (ConfItems==NULL)
		{
			int OldActive=ActiveMode;
			ActiveMode=MAC_ERRORACTIVE;
//      Error(erNotMemory);
			ActiveMode=OldActive;
			return lResult;
		}

		ConfItems[0].Flags=0;
#ifndef UNICODE
		lstrcpyn(ConfItems[0].Text,GetMsg(MMacroDefaultConfig),ARRAYSIZE(ConfItems[0].Text));
#else
		ConfItems[0].Text=GetMsg(MMacroDefaultConfig);
#endif
		ZeroMemory(ConfItems[0].Reserved,sizeof(ConfItems[0].Reserved));
	}
	else
	{
		if (Reg->GetKeyNames(&UserList))
		{
			if (UserList.GetCount()>0)
				UserCount++;

			UserCount+=UserList.GetCount();
			ConfItems=new FarListItem[UserCount];

			if (ConfItems==NULL)
			{
				int OldActive=ActiveMode;
				ActiveMode=MAC_ERRORACTIVE;
//        Error(erNotMemory);
				ActiveMode=OldActive;
				return lResult;
			}

			ConfItems[0].Flags=0;
#ifndef UNICODE
			lstrcpyn(ConfItems[0].Text,GetMsg(MMacroDefaultConfig),ARRAYSIZE(ConfItems[0].Text));
#else
			ConfItems[0].Text=GetMsg(MMacroDefaultConfig);
#endif
			ZeroMemory(ConfItems[0].Reserved,sizeof(ConfItems[0].Reserved));

			if (UserCount>1)
			{
				ConfItems[1].Flags=LIF_SEPARATOR;
#ifndef UNICODE
				ConfItems[1].Text[0]=0;
#else
				ConfItems[1].Text=L"";
#endif
				ZeroMemory(ConfItems[1].Reserved,sizeof(ConfItems[1].Reserved));

				for (int i=0; i<UserList.GetCount(); i++)
				{
					ConfItems[i+2].Flags=0;
#ifndef UNICODE
					lstrcpyn(ConfItems[i+2].Text,UserList.GetText(i),ARRAYSIZE(ConfItems[i+2].Text));
#else
					ConfItems[i+2].Text=UserList.GetText(i);
#endif
					ZeroMemory(ConfItems[i+2].Reserved,sizeof(ConfItems[i+2].Reserved));

					if (*FarUserName && (CmpStr(ConfItems[i+2].Text,FarUserName)==0))
						UserConfPos=i+2;
				}
			}
		}
		// Список пользовательских конфигураций в реестре не найден.
		else
		{
			// Создадим первый элемент списка конфигураций - общий.
			ConfItems=new FarListItem[1];

			if (ConfItems==NULL)
			{
				int OldActive=ActiveMode;
				ActiveMode=MAC_ERRORACTIVE;
//        Error(erNotMemory);
				ActiveMode=OldActive;
				return lResult;
			}

			ConfItems[0].Flags=0;
#ifndef UNICODE
			lstrcpyn(ConfItems[0].Text,GetMsg(MMacroDefaultConfig),ARRAYSIZE(ConfItems[0].Text));
#else
			ConfItems[0].Text=GetMsg(MMacroDefaultConfig);
#endif
			ZeroMemory(ConfItems[0].Reserved,sizeof(ConfItems[0].Reserved));
		}
	}

	ConfList.ItemsNumber=UserCount;
	ConfList.Items=ConfItems;
	// Заполним список значений макрогрупп.
	InitMacroAreas();
	struct InitDialogItem InitItems[]=
	{
		/* 0*/ {DI_DOUBLEBOX,            3,1,EXPORTLEN+2,9,0,0,0,0,_T("")},
		/* 1*/ {DI_TEXT,                 5,2,EXPORTLEN/2,0,0,0,0,0,CheckLen(GetMsg(MMacroSelectConfig),EXPORTLEN/2)},
		/* 2*/ {DI_COMBOBOX,             5,3,EXPORTLEN/2,3,1,0,DIF_DROPDOWNLIST,0,_T("")},
		/* 3*/ {DI_TEXT,     EXPORTLEN/2+4,2,  EXPORTLEN,0,0,0,0,0,CheckLen(GetMsg(MMacroCopyMoveTo),EXPORTLEN/2-2)},
		/* 4*/ {DI_COMBOBOX, EXPORTLEN/2+4,3,  EXPORTLEN,3,0,0,DIF_DROPDOWNLIST,0,_T("")},
		/* 5*/ {DI_TEXT,                 5,4,          0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
		/* 6*/ {DI_VTEXT,    EXPORTLEN/2+2,2,          0,0,0,0,0,0,_BOX_VTOP},
		/* 7*/ {DI_TEXT,                 5,5,  EXPORTLEN,0,0,0,0,0,CheckLen(GetMsg(MMacroNewKey),EXPORTLEN)},
		/* 8*/ {DI_EDIT,                 5,6,  EXPORTLEN,5,0,(DWORD_PTR)MacroCopyHistory,DIF_HISTORY,0,_T("")},
		/* 9*/ {DI_TEXT,                 5,7,          0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
		/*10*/ {DI_BUTTON,               0,8,          0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)MMacroSave},
		/*11*/ {DI_BUTTON,               0,8,          0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)MMacroCancel}
	};
	FarDialogItem DialogItems[ARRAYSIZE(InitItems)];
	InitDialogItems(InitItems,DialogItems,ARRAYSIZE(InitItems));
	// Скопируем в локальный буфер имя группы из которой производим копирование.
	lstrcpy(lGroup,Group);
	// Преобразуем наименование группы из которой копируем в длинное имя.
	ConvertGroupName(lGroup,GRP_TOLONGNAME);
	// Подготовим строку имени копируемого макроса для заголовка диалога.
	wsprintf(S1,_T("\"%s: %s\""),lGroup,Key);
#ifdef UNICODE
	wchar_t _tmps[ARRAYSIZE(lGroup)+ARRAYSIZE(Key)+64];
	DialogItems[0].PtrData = _tmps;
#define _NS _tmps
#else
#define _NS DialogItems[0].Data.Data
#endif
	wsprintf(_NS, _T("%s %s"), GetMsg((Op==KEY_F5) ? MMacroCopy : MMacroMove), S1);
#undef _NS
	// Инициализируем содержимое поля команды выполнения.
#ifndef UNICODE
	lstrcpyn(DialogItems[8].Data.Data,Key,ARRAYSIZE(DialogItems[8].Data.Data));
#else
	wchar_t _key[ARRAYSIZE(Key)], _conf[MAX_KEY_LEN], _area[MAX_KEY_LEN];
	_conf[0] = _area[0] = _key[0] = 0;
	DialogItems[2].PtrData = _conf;
	DialogItems[4].PtrData = _area;
	DialogItems[8].PtrData = _key;
	lstrcpy(_key, Key);
#endif
	int ExitCode;
#ifdef UNICODE
	HANDLE hDlg = INVALID_HANDLE_VALUE;
#endif
COPY_MOVE:
#ifndef UNICODE
	ExitCode = Info.DialogEx(Info.ModuleNumber,-1,-1,EXPORTLEN+6,11,_T("MacroCopy"),
	                         DialogItems,ARRAYSIZE(DialogItems),0,0,
	                         CopyDialogProc,0);
#else

	if (hDlg != INVALID_HANDLE_VALUE)
		Info.DialogFree(hDlg);

	hDlg = Info.DialogInit(Info.ModuleNumber,-1,-1,EXPORTLEN+6,11,_T("MacroCopy"),
	                       DialogItems,ARRAYSIZE(DialogItems),0,0,
	                       CopyDialogProc,0);

	if (hDlg != INVALID_HANDLE_VALUE)
	{
		ExitCode = Info.DialogRun(hDlg);
#endif

	if (ExitCode==10) // Сохранить
	{
		if (GetDataPtr(8)[0]==0) // New key is empty
		{
			wsprintf(S,_T("%s %s"),GetMsg(MMacroWarningInsertEmpty0),GetMsg(MMacroWarningInsertEmpty2));
			int OldActive=ActiveMode;
			ActiveMode=MAC_ERRORACTIVE;
			Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsKeyEmp,
			             ARRAYSIZE(ItemsKeyEmp),1);
			ActiveMode=OldActive;
			goto COPY_MOVE;
		}

		// Создадим локальную копию адреса расположения макросов в реестре,
		// в зависимости от выбранной конфигурации пользователя.
		if (UserConfPos==0)
			wsprintf(LocalKeyMacros,_T("%s\\%s"),Default_KEY,KeyMacros_KEY);
		else if (UserConfPos>1)
			wsprintf(LocalKeyMacros,_T("%s\\%s\\%s"),FarUsersKey,GetDataPtr(2),KeyMacros_KEY);
		else
		{
			int OldActive=ActiveMode;
			ActiveMode=MAC_ERRORACTIVE;
			Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsErrorConf,
			             ARRAYSIZE(ItemsErrorConf),1);
			delete[] ConfItems;
			ActiveMode=OldActive;
			return lResult;
		}

		TCHAR *Str1=new TCHAR[MAX_PATH_LEN];
		lstrcpy(lGroup,GetDataPtr(4)); // New group
		lstrcpy(lKey,GetDataPtr(8)); // New key
		// Сконвертируем имя группы назначения в короткое имя
		ConvertGroupName(lGroup,GRP_TOSHORTNAME);

		if (Deactivated)
		{
			wsprintf(S,_T("%s\\%s\\~%s"),KeyMacros,Group,Key);
			wsprintf(S1,_T("%s\\%s\\~%s"),LocalKeyMacros,lGroup,lKey);
			wsprintf(Str1,_T("%s\\%s\\%s"),LocalKeyMacros,lGroup,lKey);
		}
		else
		{
			wsprintf(S,_T("%s\\%s\\%s"),KeyMacros,Group,Key);
			wsprintf(S1,_T("%s\\%s\\%s"),LocalKeyMacros,lGroup,lKey);
			wsprintf(Str1,_T("%s\\%s\\~%s"),LocalKeyMacros,lGroup,lKey);
		}

		if (CmpStr(S,S1)==0)
		{
			if (Op==KEY_F5)
				lstrcpy(S,GetMsg(MMacroErrorCopy));
			else
				lstrcpy(S,GetMsg(MMacroErrorMove));

			int OldActive=ActiveMode;
			ActiveMode=MAC_ERRORACTIVE;
			Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsCopy,
			             ARRAYSIZE(ItemsCopy),1);
			delete[] Str1;
			ActiveMode=OldActive;
			goto COPY_MOVE;
		}

		if ((Reg->KeyExists(S1)) || (Reg->KeyExists(Str1)))
		{
			lstrcpy(Str,S);
			wsprintf(S,_T("\"%s: %s\""),GetDataPtr(4),lKey);
			int OldActive=ActiveMode;
			ActiveMode=MAC_ERRORACTIVE;

			if (Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsExist,
			                 ARRAYSIZE(ItemsExist),2)!=0)
			{
				delete[] Str1;
				delete[] ConfItems;
				ActiveMode=OldActive;
				return lResult;
			}

			ActiveMode=OldActive;
			lstrcpy(S,Str);
		}

		Reg->DeleteKey(S1);
		Reg->DeleteKey(Str1);

		if (Op==KEY_F5)
			Reg->MoveKey(S,S1,FALSE); //(OldName,NewName);
		else
		{
			Reg->MoveKey(S,S1); //(OldName,NewName);
			wsprintf(S,_T("%s\\%s"),KeyMacros,Group);

			if (Reg->OpenKey(S))
			{
				if (!Reg->HasSubKeys())
					Reg->DeleteKey(S);

				Reg->CloseKey();
			}
		}

		delete[] Str1;
		// Запомним новые имена группы и команды выполнения
		lstrcpy(Group,lGroup);
		lstrcpy(Key,lKey);
		lResult=TRUE;
	}

#ifdef UNICODE
	Info.DialogFree(hDlg);
}

#endif
delete[] ConfItems;
return lResult;
       }


       void TMacroView::MoveTildeInKey(TStrList *&List,BOOL doit)
{
	int i,len;

	for (i=0; i<List->GetCount(); i++)
	{
		List->GetText(Str,i);
		len=lstrlen(Str);

		if (doit)
		{
			if ((*Str==_T('~')) && (len>1))
			{
				_tmemmove(Str,&Str[1],len-1);
				Str[len-1]=_T('\x1');
				List->SetText(Str,i);
			}
		}
		else
		{
			if ((len>1) && (Str[len-1]==_T('\x1')))
			{
				_tmemmove(&Str[1],Str,len-1);
				*Str=_T('~');
				List->SetText(Str,i);
			}
		}
	}
}


void TMacroView::PrepareDependentSort(TStrList *&List,BOOL doit)
{
	int i;
//  TCHAR Group[MAX_KEY_LEN];

	for (i=0; i<List->GetCount(); i++)
	{
		// сейчас в Str содержится длинное имя группы макрокоманд
		List->GetText(Str,i);
		// сконвертируем Str в короткое имя группы, хранящееся в реестре
		ConvertGroupName(Str,GRP_TOSHORTNAME);

		if (doit)
		{
			if (((OpenFrom==OPEN_PLUGINSMENU) && (CmpStr(Str,_T("Shell"))==0))  ||
			        ((OpenFrom==OPEN_VIEWER)      && (CmpStr(Str,_T("Viewer"))==0)) ||
			        ((OpenFrom==OPEN_EDITOR)      && (CmpStr(Str,_T("Editor"))==0)))
			{
				wsprintf(S,_T("\x1%s"),List->GetText(i));
				List->SetText(S,i);
			}
		}
		else
		{
			if (*Str==_T('\x1'))
			{
				lstrcpy(S,List->GetText(i));
				List->SetText(&S[1],i);
			}
		}
	}
}


//=========================================================
// Заполнение меню данными из реестра
//=========================================================
void __fastcall TMacroView::FillMenu(HANDLE hDlg,int RebuildList)
{
	int i,j,k=0,AddCount=0;
	TCHAR Group[MAX_KEY_LEN];
	TCHAR OldGroup[MAX_KEY_LEN];
	TCHAR Key[MAX_KEY_LEN];
	BOOL emptyDescr=TRUE;
	FarList List;
//  FarListItem *ListItems;
	FarListPos ListPos;
	// Запретим отрисовку экрана
	Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

	if (RebuildList || MenuItemsNumber==0 || MaxMenuItemLen==0)
	{
		MenuItemsNumber=0;
		MaxMenuItemLen=0;
		GroupKeyLen=0;
		EditMode=EM_NONE;
		ZeroMemory(OldGroup,sizeof(OldGroup));
		NameList=new TStrList;        // список коротких имен групп из реестра
		MacNameList=new TStrList;     // список значений в группе
		DescrList=new TStrList;       // список описаний
		MenuList=new TStrList;        // список всех макрокоманд из реестра
		// Эти два списка созданы для синхронизации со списком MenuList
		// и содержат названия групп и ключей как будто они были взяты
		// из MenuList.
		TStrList *GroupList=new TStrList;  // список групп в текущей сортировке
		TStrList *KeyList=new TStrList;    // список ключей в текущей сортировке
		CheckFirstBackSlash(KeyMacros,TRUE);
		Reg->OpenKey(KeyMacros,TRUE);
		Reg->GetKeyNames(NameList);

		if (Conf.LongGroupNames)
		{
			//Сконвертируем короткие имена групп в длинные
			for (i=0; i<NameList->GetCount(); i++)
			{
				NameList->GetText(S,i);
				ConvertGroupName(S,GRP_TOLONGNAME);
				NameList->SetText(S,i);
			}
		}

		//Если сортировка стартозависимая, то сделаем для этого приготовления
		if (Conf.StartDependentSort>0)
			PrepareDependentSort(NameList,TRUE);

		// Сортируем список групп
		NameList->Sort(0,NameList->GetCount()-1);

		//Уберем приготовления стартозависимой сортировки
		if (Conf.StartDependentSort>0)
			PrepareDependentSort(NameList);

		// Номер позиции в готовящемся списке, куда нам следует перейти.
		ListPos.SelectPos=0;
		ListPos.TopPos=TopPos;
		// Очистим весь список
		Info.SendDlgMessage(hDlg,DM_LISTDELETE,0,0);

		for (i=0; i<NameList->GetCount(); i++)
		{
			// сейчас в Group и содержится длинное имя группы макрокоманд
			NameList->GetText(Group,i);
			// сконвертируем Group в короткое имя группы, хранящееся в реестре
			ConvertGroupName(Group,GRP_TOSHORTNAME);

			if (((Conf.ViewShell) && (OpenFrom==OPEN_PLUGINSMENU) &&
			        ((CmpStr(Group,_T("Viewer"))==0) || (CmpStr(Group,_T("Editor"))==0))) ||
			        ((Conf.ViewViewer) && (OpenFrom==OPEN_VIEWER) &&
			         (CmpStr(Group,_T("Viewer"))!=0)) ||
			        ((Conf.ViewEditor) && (OpenFrom==OPEN_EDITOR) &&
			         (CmpStr(Group,_T("Editor"))!=0)))
				continue;

			wsprintf(S,_T("%s\\%s"),KeyMacros,Group);
			CheckFirstBackSlash(S,TRUE);

			if (!Reg->OpenKey(S))
				continue;

			if (!Reg->GetKeyNames(MacNameList))
				continue;

			// Если макрокоманды временно отключены, уберём '~'
			// в ключе для обеспечения корректной сортировки
			MoveTildeInKey(MacNameList,TRUE);
			// Сортируем наименования макрокоманд
			MacNameList->Sort(MenuItemsNumber,MacNameList->GetCount()-1);
			// Восстановим положение тильды в ключе
			MoveTildeInKey(MacNameList);

			for (j=MenuItemsNumber; j<MacNameList->GetCount(); j++,k++)
			{
				MacNameList->GetText(Key,j);
				lstrcpy(Str,Group);
				ConvertGroupName(Str,Conf.LongGroupNames?GRP_TOLONGNAME:GRP_TOSHORTNAME);          //local Group
				ConvertGroupName(Macro->Group,Conf.LongGroupNames?GRP_TOLONGNAME:GRP_TOSHORTNAME); //Macro->Group
				// Проверим, совпадают ли название группы и макроса с Macro->Group и Macro->Key
				// и если да, то запомним номер позиции в списке, мы на него затем перейдём.
				// Если же искомые группа и макрос не найдены, то перейдём потом на строку,
				// на которой были в прошлый раз, а именно SelectPos.
				if (Macro->Key[0] && Macro->Group[0])
					if ((CmpStr(Str,Macro->Group)==0) && (CmpStr((Key[0]==_T('~') && lstrlen(Key)>1)?&Key[1]:Key,Macro->Key)==0))
						ListPos.SelectPos=k;

				lstrcpy(Str,NameList->GetText(i));  // Str contains group name
				lstrcpy(Key,MacNameList->GetText(j)); // Key contains key name

				if (Conf.GroupDivider)
				{
					// Добавление разделителя групп.
					if (*OldGroup && CmpStr(OldGroup,Str)!=0)
					{
						MenuList->Add(_T(""));
						GroupList->Add(_T(""));
						KeyList->Add(_T(""));

						if (Conf.AddDescription)
							DescrList->Add(_T(""));

						AddCount++;
						k++;
					}

					lstrcpy(OldGroup,Str);
				}

				if (Conf.AddDescription)
				{
					wsprintf(S,_T("%s\\%s\\%s"),KeyMacros,Group,Key);
					CheckFirstBackSlash(S,TRUE);

					if (!Reg->OpenKey(S))
						continue;

					Reg->ReadString(_T("Description"),S,sizeof(S));

					if (S[0])
					{
						DescrList->Add(S);
						emptyDescr=FALSE;
					}
					else
						DescrList->Add(_T(""));

					Reg->CloseKey();
				}

				CheckLen(Str,KeyWidth);
				lstrcat(Str,_T(":"));
				FSF.sprintf(S,_T("%-*s  \"%s\""),KeyWidth+1,Str,(Key[0]==_T('~') && lstrlen(Key)>1)?&Key[1]:Key);
				CheckLen(S,(ConsoleSize.X-12>MAXMENULEN) ? MAXMENULEN : ConsoleSize.X-12);

				if (Key[0]==_T('~') && lstrlen(Key)>1)
				{
					Str[0]=_T('~');
					Str[1]=0;
				}
				else
					Str[0]=0;

				lstrcat(Str,S);
				MenuList->Add(Str);
				GroupList->Add(Group);
				KeyList->Add(Key);
				MenuItemsNumber++;
				//вычисляем самое длинное имя макрокоманды
				GroupKeyLen=max(lstrlen(S),GroupKeyLen);
			}
		}

		if (ListPos.SelectPos==0)
			ListPos.SelectPos=SelectPos;

		// Структура для хранения ассоциированных с итемом меню данных.
		MenuData MData;
		FarListItemData ItemData;
		MenuItemsNumber+=AddCount;

		if (MenuItemsNumber==0)
		{
			if (Conf.AddDescription)
				FSF.sprintf(S,_T("%*s") _BOX_VERT _T("%*s"),KeyWidth,_T(" "),KeyWidth,_T(" "));
			else
				FSF.sprintf(S,_T("%*s"),KeyWidth,_T(" "));

			FarListItem ListItems; //=new FarListItem[1];
			List.ItemsNumber=1;
			List.Items=&ListItems;
#ifndef UNICODE
			lstrcpy(ListItems.Text,S);
#else
			ListItems.Text=S;
#endif
			ListItems.Flags=LIF_SELECTED;
			Info.SendDlgMessage(hDlg,DM_LISTADD,0,(LONG_PTR)&List);
			ZeroMemory(&MData,sizeof(MData));
			ItemData.Index=0;
			ItemData.Data=&MData;
			ItemData.DataSize=sizeof(MData);
			Info.SendDlgMessage(hDlg,DM_LISTSETDATA,0,(LONG_PTR)&ItemData);
			MaxMenuItemLen=lstrlen(S);
		}
		else
		{
			FarListItem ListItems; //=new FarListItem[MenuItemsNumber];
			List.ItemsNumber=1; //MenuItemsNumber;
			List.Items=&ListItems;

			for (i=0; i<MenuItemsNumber; i++)
			{
				if (Conf.AddDescription && !emptyDescr)
				{
					lstrcpy(Str,MenuList->GetText(i));

					if (*Str)
					{
						FSF.sprintf(S,_T("%-*s ") _BOX_VERT _T(" %s"),GroupKeyLen,
						            (Str[0]==_T('~'))?&Str[1]:Str,DescrList->GetText(i));

						if (Str[0]==_T('~'))
						{
							memmove(&S[1],S,sizeof(S)-sizeof(S[0]));
							S[0]=_T('~');
						}
					}
					else
						S[0]=0;

//            lstrcpy(S,Str);
				}
				else
					lstrcpy(S,MenuList->GetText(i));

				if (*S)
				{
					ListItems/*[i]*/.Flags=(S[0]==_T('~'))?LIF_CHECKED|_T('-'):0;

					if (S[0]==_T('~'))
						memmove(S,&S[1],sizeof(S)-sizeof(S[0]));

					CheckLen(S,(ConsoleSize.X-12>MAXMENULEN)?MAXMENULEN:ConsoleSize.X-12);
#ifndef UNICODE
					lstrcpy(ListItems/*[i]*/.Text,S);
#else
					ListItems/*[i]*/.Text=S;
#endif
					lstrcpyn(MData.Group,GroupList->GetText(i),ARRAYSIZE(MData.Group)-1);
					MData.Group[ARRAYSIZE(MData.Group)-1]=0;
					lstrcpyn(MData.Key,KeyList->GetText(i),ARRAYSIZE(MData.Key)-1);
					MData.Key[ARRAYSIZE(MData.Key)-1]=0;
				}
				else
				{
					ListItems/*[i]*/.Flags=LIF_SEPARATOR;
#ifndef UNICODE
					ListItems/*[i]*/.Text[0]=0;
#else
					ListItems/*[i]*/.Text=L"";
#endif
					ZeroMemory(&MData,sizeof(MData));
				}

				Info.SendDlgMessage(hDlg,DM_LISTADD,0,(LONG_PTR)&List);
				// Ассоциируем с добавленным итемом меню данные о группе и ключе.
				ItemData.Index=i;
				ItemData.Data=&MData;
				ItemData.DataSize=sizeof(MData);
				Info.SendDlgMessage(hDlg,DM_LISTSETDATA,0,(LONG_PTR)&ItemData);
				MaxMenuItemLen=max(lstrlen(S),MaxMenuItemLen);
			}
		}

//    Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
//    Info.SendDlgMessage(hDlg,DM_LISTSET,0,(LONG_PTR)&List);
//    delete[] ListItems;
		delete GroupList;
		delete KeyList;
		delete NameList;
		delete MacNameList;
		delete DescrList;
		delete MenuList;
		Reg->CloseKey();
		NameList=MacNameList=DescrList=MenuList=NULL;
	}
	else
	{
		// Номер позиции в готовящемся списке, куда нам следует перейти.
		ListPos.SelectPos=SelectPos;
		ListPos.TopPos=TopPos;
	}

	MenuW=(MaxMenuItemLen+8>ConsoleSize.X)?ConsoleSize.X-10:MaxMenuItemLen+8;
	MenuH=(MenuItemsNumber+10>ConsoleSize.Y)?ConsoleSize.Y-6:MenuItemsNumber+4;

	if (MenuH<5)
		MenuH=5;

	if (ListPos.TopPos+(MenuH-4)>MenuItemsNumber)
	{
		ListPos.TopPos=MenuItemsNumber-(MenuH-4);
	}

	// Установим позицию курсора в списке
	Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,0,(LONG_PTR)&ListPos);
	// Разрешим отрисовку экрана
	Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
	COORD Coord;
	Coord.X=MenuW;
	Coord.Y=MenuH;
	Info.SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(LONG_PTR)&Coord);
	/*  if (MenuX==-1 && MenuY==-1) // MenuX и MenuY равны -1, значит первый вызов и диалог центрируется.
	  {
	    MenuX=(ConsoleSize.X-MenuW)/2;
	    MenuY=(ConsoleSize.Y-MenuH)/2;
	  }*/
	Coord.X=MenuX;
	Coord.Y=MenuY;
	Info.SendDlgMessage(hDlg,DM_MOVEDIALOG,TRUE,(LONG_PTR)&Coord);
	SMALL_RECT Rect;
	Rect.Left=2;
	Rect.Top=1;
	Rect.Right=MenuW-3;
	Rect.Bottom=MenuH-2;
	Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,0,(LONG_PTR)&Rect);
}


void TMacroView::WriteRegValues(FarDialogItem *DialogItems
#ifdef UNICODE
                                ,HANDLE hDlg
#endif
                               )
{
	const TCHAR *Src=MacroData?MacroData:GetDataPtr(8);

	if (MultiLine)
	{
		int LenMacroData=lstrlen(Src)+2;
		TCHAR *NewMacroData=new TCHAR[LenMacroData];

		if (NewMacroData)
		{
			lstrcpy(NewMacroData,Src);

			for (int J=0; NewMacroData[J]; ++J)
				if (NewMacroData[J] == _T('\n'))
					NewMacroData[J]=0;

			NewMacroData[LenMacroData-1]=0;

			Reg->PutData(_T("Sequence"),(const BYTE*)NewMacroData,LenMacroData*sizeof(TCHAR),rdMultiString);

			delete[] NewMacroData;
    	}
	}
	else
		Reg->WriteString(_T("Sequence"),Src);


	if (GetDataPtr(10)[0])
		Reg->WriteString(_T("Description"),GetDataPtr(10));
	else
		Reg->DeleteValue(_T("Description"));

	if (GetCheck(12))
		Reg->WriteInteger(_T("RunAfterFARStart"),GetCheck(12));
	else
		Reg->DeleteValue(_T("RunAfterFARStart"));

	if (GetCheck(13))
		Reg->WriteInteger(_T("DisableOutput"),GetCheck(13));
	else
		Reg->DeleteValue(_T("DisableOutput"));

	switch (GetCheck(14))
	{
		case 2:
			Reg->DeleteValue(_T("NotEmptyCommandLine"));
			Reg->DeleteValue(_T("EmptyCommandLine"));
			break;
		case 1:
			Reg->DeleteValue(_T("EmptyCommandLine"));
			Reg->WriteInteger(_T("NotEmptyCommandLine"),1);
			break;
		case 0:
			Reg->DeleteValue(_T("NotEmptyCommandLine"));
			Reg->WriteInteger(_T("EmptyCommandLine"),1);
			break;
	}

	switch (GetCheck(15))
	{
		case 2:
			Reg->DeleteValue(_T("NoEVSelection"));
			Reg->DeleteValue(_T("EVSelection"));
			break;
		case 1:
			Reg->DeleteValue(_T("NoEVSelection"));
			Reg->WriteInteger(_T("EVSelection"),1);
			break;
		case 0:
			Reg->DeleteValue(_T("EVSelection"));
			Reg->WriteInteger(_T("NoEVSelection"),1);
			break;
	}

	// Флаги активной панели
	if (GetCheck(16))
	{
		switch (GetCheck(18))
		{
			case 2:
				Reg->DeleteValue(_T("NoPluginPanels"));
				Reg->DeleteValue(_T("NoFilePanels"));
				break;
			case 1:
				Reg->DeleteValue(_T("NoPluginPanels"));
				Reg->WriteInteger(_T("NoFilePanels"),1);
				break;
			case 0:
				Reg->DeleteValue(_T("NoFilePanels"));
				Reg->WriteInteger(_T("NoPluginPanels"),1);
				break;
		}

		switch (GetCheck(20))
		{
			case 2:
				Reg->DeleteValue(_T("NoFolders"));
				Reg->DeleteValue(_T("NoFiles"));
				break;
			case 1:
				Reg->DeleteValue(_T("NoFolders"));
				Reg->WriteInteger(_T("NoFiles"),1);
				break;
			case 0:
				Reg->DeleteValue(_T("NoFiles"));
				Reg->WriteInteger(_T("NoFolders"),1);
				break;
		}

		switch (GetCheck(22))
		{
			case 2:
				Reg->DeleteValue(_T("NoSelection"));
				Reg->DeleteValue(_T("Selection"));
				break;
			case 1:
				Reg->DeleteValue(_T("NoSelection"));
				Reg->WriteInteger(_T("Selection"),1);
				break;
			case 0:
				Reg->DeleteValue(_T("Selection"));
				Reg->WriteInteger(_T("NoSelection"),1);
				break;
		}
	}
	else
	{
		Reg->DeleteValue(_T("NoPluginPanels"));
		Reg->DeleteValue(_T("NoFilePanels"));
		Reg->DeleteValue(_T("NoFolders"));
		Reg->DeleteValue(_T("NoFiles"));
		Reg->DeleteValue(_T("NoSelection"));
		Reg->DeleteValue(_T("Selection"));
	}

	// Флаги пассивной панели
	if (GetCheck(17))
	{
		switch (GetCheck(19))
		{
			case 2:
				Reg->DeleteValue(_T("NoPluginPPanels"));
				Reg->DeleteValue(_T("NoFilePPanels"));
				break;
			case 1:
				Reg->DeleteValue(_T("NoPluginPPanels"));
				Reg->WriteInteger(_T("NoFilePPanels"),1);
				break;
			case 0:
				Reg->DeleteValue(_T("NoFilePPanels"));
				Reg->WriteInteger(_T("NoPluginPPanels"),1);
				break;
		}

		switch (GetCheck(21))
		{
			case 2:
				Reg->DeleteValue(_T("NoPFolders"));
				Reg->DeleteValue(_T("NoPFiles"));
				break;
			case 1:
				Reg->DeleteValue(_T("NoPFolders"));
				Reg->WriteInteger(_T("NoPFiles"),1);
				break;
			case 0:
				Reg->DeleteValue(_T("NoPFiles"));
				Reg->WriteInteger(_T("NoPFolders"),1);
				break;
		}

		switch (GetCheck(23))
		{
			case 2:
				Reg->DeleteValue(_T("NoPSelection"));
				Reg->DeleteValue(_T("PSelection"));
				break;
			case 1:
				Reg->DeleteValue(_T("NoPSelection"));
				Reg->WriteInteger(_T("PSelection"),1);
				break;
			case 0:
				Reg->DeleteValue(_T("PSelection"));
				Reg->WriteInteger(_T("NoPSelection"),1);
				break;
		}
	}
	else
	{
		Reg->DeleteValue(_T("NoPluginPPanels"));
		Reg->DeleteValue(_T("NoFilePPanels"));
		Reg->DeleteValue(_T("NoPFolders"));
		Reg->DeleteValue(_T("NoPFiles"));
		Reg->DeleteValue(_T("NoPSelection"));
		Reg->DeleteValue(_T("PSelection"));
	}

	if (GetCheck(26))
		Reg->DeleteValue(_T("NoSendKeysToPlugins"));
	else
		Reg->WriteInteger(_T("NoSendKeysToPlugins"),1);
}


BOOL __fastcall TMacroView::CopyMacro(int vKey)
{
	BOOL lResult=FALSE;

	if (MenuItemsNumber!=0)
	{
		WriteKeyBar(KB_DIALOG);
		ActiveMode=MAC_COPYACTIVE;
		lResult=CopyMoveMacro(vKey);
	}

	return lResult;
}


void __fastcall TMacroView::ExportMacro(BOOL AllMacros)
{
	int eCode;
	TCHAR lGroup[MAX_KEY_LEN]; //длинное название текущего раздела макроса
	const TCHAR *ItemsSave1[]=
	{
		GetMsg(MMacroExport),GetMsg(MMacroWhere),S,
		_T("\x1"),
		GetMsg(MMacroInsertEditor),GetMsg(MMacroSaveToFile),GetMsg(MMacroCancel),
	};
	const TCHAR *ItemsSave2[]=
	{
		GetMsg(MMacroExport),GetMsg(MMacroAllWhere),
		_T("\x1"),
		GetMsg(MMacroInsertEditor),GetMsg(MMacroSaveToFile),GetMsg(MMacroCancel),
	};

	if (MenuItemsNumber!=0)
	{
		ActiveMode=MAC_EXPORTACTIVE;

		switch (OpenFrom)
		{
			case OPEN_EDITOR:
				lstrcpy(lGroup,Group);
				//Из короткого имени группы создадим длинное
				ConvertGroupName(lGroup,GRP_TOLONGNAME);
				wsprintf(S,_T("%s: %s"),lGroup,Key);
				QuoteText(S,TRUE);

				if (AllMacros)
				{
					eCode=Info.Message(Info.ModuleNumber,0,NULL,ItemsSave2,
					                   ARRAYSIZE(ItemsSave2),3);
				}
				else
				{
					eCode=Info.Message(Info.ModuleNumber,0,NULL,ItemsSave1,
					                   ARRAYSIZE(ItemsSave1),3);
				}

				if (eCode==0)
				{
					InsertMacroToEditor(AllMacros);
					break;
				}
				else if (eCode!=1)
					break;

			default:
				ExportMacroToFile(AllMacros);
				break;
		}

//    Key[0]=Group[0]=0;
	}
}


BOOL __fastcall TMacroView::DeleteMacro()
{
	BOOL lResult=FALSE;
	TCHAR Button[BUTTONLEN];
	TCHAR lGroup[MAX_KEY_LEN]; //длинное название текущего раздела макроса

	if (Deactivated)
		wsprintf(Str,GetMsg(MMacroWarningDeleteThisKey),GetMsg(MMacroWarningRest));
	else
		wsprintf(Str,GetMsg(MMacroWarningDeleteThisKey),GetMsg(MMacroWarningDel));

	const TCHAR *ItemsDel[]=
	{
		GetMsg(MMacroWarningDelete),Str,
		S,
		_T("\x1"),
		GetMsg(MMacroDelete),Button,GetMsg(MMacroCancel)
	};

	if (MenuItemsNumber!=0)
	{
		lstrcpy(lGroup,Group);
		//Из короткого имени группы создадим длинное
		ConvertGroupName(lGroup,GRP_TOLONGNAME);
		wsprintf(S,_T("%s: %s"),lGroup,Key);
		QuoteText(S,TRUE);

		if (Deactivated)
			lstrcpy(Button,GetMsg(MMacroTmpRest));
		else
			lstrcpy(Button,GetMsg(MMacroTmpDel));

		WriteKeyBar(KB_DIALOG);
		ActiveMode=MAC_DELETEACTIVE;
		lResult=DeletingMacro(ItemsDel,ARRAYSIZE(ItemsDel),_T("MacroDel"))!=DM_NONE;
		*Group=*Key=0;
	}

	return lResult;
}

BOOL __fastcall TMacroView::InsertMacro()
{
	BOOL RetVal=FALSE;
	TCHAR lGroup[MAX_KEY_LEN]; //длинное название текущего раздела макроса
	const TCHAR *ItemsInsEmp[]=
	{
		GetMsg(MMacroWarning),GetMsg(MMacroWarningInsertEmpty1),
		GetMsg(MMacroWarningInsertEmpty2),
		_T("\x1"),
		GetMsg(MMacroOk),
	};
	const TCHAR *ItemsEdit[]=
	{
		GetMsg(MMacroWarningEdit),GetMsg(MMacroWarningEditThisKey),
		_T("\x1"),
		GetMsg(MMacroOk),GetMsg(MMacroCancel)
	};
	const TCHAR *ItemsExist[]=
	{
		GetMsg(MMacroWarning),GetMsg(MMacroWarningExist),
		S,
		_T("\x1"),
		GetMsg(MMacroOverwrite),GetMsg(MMacroCancel)
	};
	// Проинициализируем диалог!
	InitDialogs();
	// Инициализируем новый макрос как НЕ многострочный
	MultiLine=FALSE;
	_Group[0]=0;
	EditDialog[2].Focus=TRUE;
	_Button[0]=0;
#ifndef UNICODE

	if (MacroData)
	{
		EditDialog[8].Data.Ptr.PtrFlags=0;
		EditDialog[8].Data.Ptr.PtrLength=DATASIZE;
		EditDialog[8].Data.Ptr.PtrData=MacroData;
		*MacroData=0;
	}
	else
#endif
		_DataPtr[0]=0;

	_Descr[0]=0;
	EditDialog[12].Param.Selected=FALSE;
	EditDialog[13].Param.Selected=TRUE;
	// Command line state
	EditDialog[14].Param.Selected=2;
	// Panel selection
	EditDialog[15].Param.Selected=2;
	// Active panel
	EditDialog[16].Param.Selected=FALSE;
	// Passive panel
	EditDialog[17].Param.Selected=FALSE;
	// Plugin/file active panel
	EditDialog[18].Param.Selected=2;
	EditDialog[18].Flags|=DIF_DISABLE;
	// Plugin/file passive panel
	EditDialog[19].Param.Selected=2;
	EditDialog[19].Flags|=DIF_DISABLE;
	// Folder/file on active panel
	EditDialog[20].Param.Selected=2;
	EditDialog[20].Flags|=DIF_DISABLE;
	// Folder/file on passive panel
	EditDialog[21].Param.Selected=2;
	EditDialog[21].Flags|=DIF_DISABLE;
	// Folder/file selected on active panel
	EditDialog[22].Param.Selected=2;
	EditDialog[22].Flags|=DIF_DISABLE;
	// Folder/file selected on passive panel
	EditDialog[23].Param.Selected=2;
	EditDialog[23].Flags|=DIF_DISABLE;
	// Send macro to plugins
	EditDialog[26].Param.Selected=TRUE;
	// Switch off macro
	EditDialog[28].Param.Selected=FALSE;

	if (GroupIndex[OpenFrom]>=0)
	{
		lstrcpy(_Button,GroupItems[GroupIndex[OpenFrom]].Text);
	}

	Deactivated=FALSE;
	int OutCode;
#ifdef UNICODE
	HANDLE hDlg = INVALID_HANDLE_VALUE;
#endif
INSERT_RETRY:
	// сконвертируем из короткого имени группы длинное,
	// для работы в диалоге
	ConvertGroupName(_Button,GRP_TOLONGNAME);
	WriteKeyBar(KB_DIALOG);
	ActiveMode=MAC_EDITACTIVE;
	EditMode=EM_INSERT;
	EditX1=(ConsoleSize.X-DIALOGWID)/2;
	EditY1=(ConsoleSize.Y-1-DIALOGHGT)/2;
	EditX2=EditX1+DIALOGWID-1;
	EditY2=EditY1+DIALOGHGT-1;
#ifndef UNICODE
	OutCode = Info.DialogEx(Info.ModuleNumber,EditX1,EditY1,EditX2,EditY2,
	                        _T("MacroParams"),EditDialog,ARRAYSIZE(EditDialog),
	                        0,0,MacroDialogProc,0);
#else

	if (hDlg != INVALID_HANDLE_VALUE)
		Info.DialogFree(hDlg);

	hDlg = Info.DialogInit(Info.ModuleNumber,EditX1,EditY1,EditX2,EditY2,
	                       _T("MacroParams"),EditDialog,ARRAYSIZE(EditDialog),
	                       0,0,MacroDialogProc,0);

	if (hDlg != INVALID_HANDLE_VALUE)
	{
		OutCode = Info.DialogRun(hDlg);
#endif

	if (OutCode==30) // кнопка [Сохранить]
	{
#ifdef UNICODE
		lstrcpyn(_Group,(const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,2,0),ARRAYSIZE(_Group));
		lstrcpyn(_Button,(const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,4,0),ARRAYSIZE(_Button));
		lstrcpyn(_Descr,(const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,10,0),ARRAYSIZE(_Descr));
		lstrcpyn(_DataPtr,(const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,8,0),(int)_DataPtrSize);
		EditDialog[28].Param.Selected=static_cast<int>(Info.SendDlgMessage(hDlg,DM_GETCHECK,28,0));
#endif
		// конвертируем из длинного имени группы короткое,
		// для записи в реестр
		ConvertGroupName(_Button,GRP_TOSHORTNAME);

		if ((_Group[0]!=0) && (
#ifndef UNICODE
		            (MacroData)?*MacroData!=0 :
#endif
		            _DataPtr[0]!=0))
		{
			lstrcpy(lGroup,_Button);
			//Из короткого имени группы создадим длинное
			ConvertGroupName(lGroup,GRP_TOLONGNAME);

			if ((Conf.AutomaticSave) || ((Info.Message(Info.ModuleNumber,0,NULL,ItemsEdit,
			                              ARRAYSIZE(ItemsEdit),2))==0))
			{
				TCHAR *S1=new TCHAR[MAX_KEY_LEN];
				wsprintf(Str,_T("%s\\%s\\%s"),KeyMacros,_Button,_Group);
				wsprintf(S1,_T("%s\\%s\\~%s"),KeyMacros,_Button,_Group);
				CheckFirstBackSlash(Str,TRUE);
				CheckFirstBackSlash(S1,TRUE);

				if ((Reg->KeyExists(Str)) || (Reg->KeyExists(S1)))
				{
					int OldActive=ActiveMode;
					ActiveMode=MAC_ERRORACTIVE;
					wsprintf(S,_T("\"%s: %s\""),lGroup,_Group);

					if (Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsExist,
					                 ARRAYSIZE(ItemsExist),2)==0)
					{
						Reg->DeleteKey(S1);
						Reg->DeleteKey(Str);
						ActiveMode=OldActive;
					}
					else
					{
						delete[] S1;
						ActiveMode=OldActive;
						goto INSERT_RETRY;
					}
				}

				Reg->OpenKey(Str,TRUE);
#ifndef UNICODE
				WriteRegValues(EditDialog);
#else
				WriteRegValues(EditDialog,hDlg);
#endif
				Reg->CloseKey();
				SwitchOver(_Button,_Group);
				delete[] S1;
			}
		}
		else
		{
			int OldActive=ActiveMode;
			ActiveMode=MAC_ERRORACTIVE;
			Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsInsEmp,
			             ARRAYSIZE(ItemsInsEmp),1);
			ActiveMode=OldActive;
			goto INSERT_RETRY;
		}

		// Копируем  в переменные Key и Group последние значения
		// для перехода потом на новое значение
		lstrcpy(Key,_Group);
		lstrcpy(Group,_Button);
#ifndef UNICODE
		Deactivated=EditDialog[28].Param.Selected;
#else
		Deactivated=(int)Info.SendDlgMessage(hDlg,DM_GETCHECK,28,0);
#endif
		RetVal=TRUE;
	}

#ifdef UNICODE
	Info.DialogFree(hDlg);
	}

#endif
	return RetVal;
}


BOOL __fastcall TMacroView::EditMacro()
{
	BOOL RetVal=FALSE;
	TCHAR Button[BUTTONLEN];
	TCHAR lGroup[MAX_KEY_LEN]; //длинное название текущего раздела макроса
	int i;
	const TCHAR *ItemsError[]=
	{
		GetMsg(MMacroError),GetMsg(MMacroErrorRead),
		S,
		_T("\x1"),
		GetMsg(MMacroOk),
	};
	const TCHAR *ItemsErrorData[]=
	{
		GetMsg(MMacroError),GetMsg(MMacroErrorData),
		S,
		_T("\x1"),
		GetMsg(MMacroOk),
	};
	const TCHAR *ItemsDelEmp[]=
	{
		GetMsg(MMacroWarningDelete),GetMsg(MMacroWarningDeleteKey),
		S,
		_T("\x1"),
		GetMsg(MMacroDelete),Button,GetMsg(MMacroCancel)
	};
	const TCHAR *ItemsEdit[]=
	{
		GetMsg(MMacroWarningEdit),GetMsg(MMacroWarningEditThisKey),
		_T("\x1"),
		GetMsg(MMacroOk),GetMsg(MMacroCancel)
	};
	const TCHAR *ItemsExist[]=
	{
		GetMsg(MMacroWarning),GetMsg(MMacroWarningExist),
		S,
		_T("\x1"),
		GetMsg(MMacroOverwrite),GetMsg(MMacroCancel)
	};
	const TCHAR *ItemsErrorWrite[]=
	{
		GetMsg(MMacroError),GetMsg(MMacroErrorWrite),
		S,
		_T("\x1"),
		GetMsg(MMacroOk),
	};
	const TCHAR *ItemsEditEmp[]=
	{
		GetMsg(MMacroWarning),GetMsg(MMacroWarningInsertEmpty0),
		GetMsg(MMacroWarningInsertEmpty2),
		_T("\x1"),
		GetMsg(MMacroOk),
	};

	if (MenuItemsNumber==0)
		return RetVal;

	// Проинициализируем диалог!
	InitDialogs();
	// Инициализируем новый макрос как НЕ многострочный
	MultiLine=FALSE;
	lstrcpy(_Group,Key);
	lstrcpy(_Button,Group);
#ifndef UNICODE

	if (MacroData)
	{
		EditDialog[8].Data.Ptr.PtrFlags=0;
		EditDialog[8].Data.Ptr.PtrLength=DATASIZE;
		EditDialog[8].Data.Ptr.PtrData=MacroData;
		*MacroData=0;
	}
	else
#endif
		_DataPtr[0]=0;

	EditDialog[8].Focus=TRUE;
	_Descr[0]=0;
	EditDialog[12].Param.Selected=FALSE;
	EditDialog[13].Param.Selected=FALSE;
	// Command line state
	EditDialog[14].Param.Selected=2;
	// Panel selection
	EditDialog[15].Param.Selected=2;
	// Active panel
	EditDialog[16].Param.Selected=FALSE;
	// Passive panel
	EditDialog[17].Param.Selected=FALSE;
	// Plugin/file active panel
	EditDialog[18].Param.Selected=2;
	// Plugin/file passive panel
	EditDialog[19].Param.Selected=2;
	// Folder/file on active panel
	EditDialog[20].Param.Selected=2;
	// Folder/file on passive panel
	EditDialog[21].Param.Selected=2;
	// Folder/file selected on active panel
	EditDialog[22].Param.Selected=2;
	// Folder/file selected on passive panel
	EditDialog[23].Param.Selected=2;
	// Send macro to plugins
	EditDialog[26].Param.Selected=TRUE;
	// Switch off macro
	EditDialog[28].Param.Selected=FALSE;

	if (Deactivated)
		wsprintf(S,_T("%s\\%s\\~%s"),KeyMacros,Group,Key);
	else
		wsprintf(S,_T("%s\\%s\\%s"),KeyMacros,Group,Key);

	CheckFirstBackSlash(S,TRUE);

	if (!Reg->OpenKey(S))
	{
		int OldActive=ActiveMode;
		ActiveMode=MAC_ERRORACTIVE;
		QuoteText(S,TRUE);
		Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsError,ARRAYSIZE(ItemsError),1);
		*Group=*Key=0;
		ActiveMode=OldActive;
	}
	else
	{
		ValueList=new TStrList;

		if (!Reg->GetValueNames(ValueList))
		{
			int OldActive=ActiveMode;
			ActiveMode=MAC_ERRORACTIVE;
			QuoteText(S,TRUE);
			Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsErrorData,ARRAYSIZE(ItemsErrorData),1);
			*Group=*Key=0;
			ActiveMode=OldActive;
		}
		else
		{
			for (i=0; i<ValueList->GetCount(); i++)
			{
				lstrcpy(S,ValueList->GetText(i));

				if (CmpStr(S,_T("Sequence"))==0) // Sequence
				{
					if (MacroData)
					{
						// Получим тип данных макропоследовательности
						TRegDataType Type;
						int nSize=Reg->GetData(S,MacroData,DATASIZE,Type);

						if (nSize > 0)
						{
							if (Type == rdMultiString)
							{
								// Макрос в реестре с типом многострочный
								MultiLine=TRUE;

								int nLen=(nSize/sizeof(TCHAR))-1;
								for (int J=0; J < nLen; J++)
								{
									if (!MacroData[J])
									{
										if (!MacroData[J+1])
											break;
										MacroData[J]=_T('\n');
									}
								}
							}
						}
						else
							MacroData[0]=0;

					}
					else
					{
						Reg->ReadString(S,Str,sizeof(Str));
						lstrcpy(_DataPtr, Str);
					}
				}
				else if (CmpStr(S,_T("Description"))==0) // Description
				{
					Reg->ReadString(S,Str,sizeof(Str));
					lstrcpy(_Descr, Str);
				}
				else if (CmpStr(S,_T("RunAfterFARStart"))==0) // RunAfterFARStart
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[12].Param.Selected=result;
				}
				else if (CmpStr(S,_T("DisableOutput"))==0) // DisableOutput
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[13].Param.Selected=result;
				}
				else if (CmpStr(S,_T("EmptyCommandLine"))==0) // EmptyCommandLine
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[14].Param.Selected=0;
				}
				else if (CmpStr(S,_T("NotEmptyCommandLine"))==0) // NotEmptyCommandLine
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[14].Param.Selected=1;
				}
				else if (CmpStr(S,_T("NoEVSelection"))==0) // NoEVSelection
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[15].Param.Selected=0;
				}
				else if (CmpStr(S,_T("EVSelection"))==0) // EVSelection
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[15].Param.Selected=1;
				}
				// Флаги активной панели
				else if (CmpStr(S,_T("NoPluginPanels"))==0) // NoPluginPanels
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[18].Param.Selected=0;
				}
				else if (CmpStr(S,_T("NoFilePanels"))==0) // NoFilePanels
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[18].Param.Selected=1;
				}
				else if (CmpStr(S,_T("NoFolders"))==0) // NoFolders
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[20].Param.Selected=0;
				}
				else if (CmpStr(S,_T("NoFiles"))==0) // NoFiles
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[20].Param.Selected=1;
				}
				else if (CmpStr(S,_T("NoSelection"))==0) // NoSelection
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[22].Param.Selected=0;
				}
				else if (CmpStr(S,_T("Selection"))==0) // Selection
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[22].Param.Selected=1;
				}
				// Флаги пассивной панели
				else if (CmpStr(S,_T("NoPluginPPanels"))==0) // NoPluginPPanels
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[19].Param.Selected=0;
				}
				else if (CmpStr(S,_T("NoFilePPanels"))==0) // NoFilePPanels
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[19].Param.Selected=1;
				}
				else if (CmpStr(S,_T("NoPFolders"))==0) // NoPFolders
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[21].Param.Selected=0;
				}
				else if (CmpStr(S,_T("NoPFiles"))==0) // NoPFiles
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[21].Param.Selected=1;
				}
				else if (CmpStr(S,_T("NoPSelection"))==0) // NoPSelection
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[23].Param.Selected=0;
				}
				else if (CmpStr(S,_T("PSelection"))==0) // PSelection
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[23].Param.Selected=1;
				}
				else if (CmpStr(S,_T("NoSendKeysToPlugins"))==0) // NoSendKeysToPlugins
				{
					int result=Reg->ReadInteger(S);

					if (result>0)
						EditDialog[26].Param.Selected=0;
				}
			}

			if (EditDialog[18].Param.Selected!=2 || EditDialog[20].Param.Selected!=2 || EditDialog[22].Param.Selected!=2)
			{
				EditDialog[16].Param.Selected=TRUE;
			}
			else
			{
				EditDialog[16].Param.Selected=FALSE;
				EditDialog[18].Flags|=DIF_DISABLE;
				EditDialog[20].Flags|=DIF_DISABLE;
				EditDialog[22].Flags|=DIF_DISABLE;
			}

			if (EditDialog[19].Param.Selected!=2 || EditDialog[21].Param.Selected!=2 || EditDialog[23].Param.Selected!=2)
			{
				EditDialog[17].Param.Selected=TRUE;
			}
			else
			{
				EditDialog[17].Param.Selected=FALSE;
				EditDialog[19].Flags|=DIF_DISABLE;
				EditDialog[21].Flags|=DIF_DISABLE;
				EditDialog[23].Flags|=DIF_DISABLE;
			}

			if (Deactivated)
				EditDialog[28].Param.Selected=TRUE;

			delete ValueList;
			ValueList=NULL;
			int OutCode;
#ifdef UNICODE
			HANDLE hDlg = INVALID_HANDLE_VALUE;
#endif
EDIT_RETRY:
			// сконвертируем из короткого имени группы длинное,
			// для работы в диалоге
			ConvertGroupName(_Button,GRP_TOLONGNAME);
			WriteKeyBar(KB_DIALOG);
			ActiveMode=MAC_EDITACTIVE;
			EditMode=EM_EDIT;
			EditX1=(ConsoleSize.X-DIALOGWID)/2;
			EditY1=(ConsoleSize.Y-1-DIALOGHGT)/2;
			EditX2=EditX1+DIALOGWID-1;
			EditY2=EditY1+DIALOGHGT-1;
#ifndef UNICODE
			OutCode = Info.DialogEx(Info.ModuleNumber,EditX1,EditY1,EditX2,EditY2,
			                        _T("MacroParams"),EditDialog,ARRAYSIZE(EditDialog),
			                        0,0,MacroDialogProc,0);
#else

			if (hDlg != INVALID_HANDLE_VALUE)
				Info.DialogFree(hDlg);

			hDlg = Info.DialogInit(Info.ModuleNumber,EditX1,EditY1,EditX2,EditY2,
			                       _T("MacroParams"),EditDialog,ARRAYSIZE(EditDialog),
			                       0,0,MacroDialogProc,0);

			if (hDlg != INVALID_HANDLE_VALUE)
			{
				OutCode = Info.DialogRun(hDlg);
#endif

			if (OutCode==30) // кнопка [Сохранить]
			{
#ifdef UNICODE
				lstrcpyn(_Group,(const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,2,0),ARRAYSIZE(_Group));
				lstrcpyn(_Button,(const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,4,0),ARRAYSIZE(_Button));
				lstrcpyn(_Descr,(const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,10,0),ARRAYSIZE(_Descr));
				lstrcpyn(_DataPtr,(const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,8,0),(int)_DataPtrSize);
				EditDialog[28].Param.Selected=static_cast<int>(Info.SendDlgMessage(hDlg,DM_GETCHECK,28,0));
#endif
				// конвертируем из длинного имени группы короткое,
				// для записи в реестр
				ConvertGroupName(_Button,GRP_TOSHORTNAME);
				BOOL deleted=FALSE;

				if (
#ifndef UNICODE
				    (MacroData)?*MacroData==0 :
#endif
				    _DataPtr[0]==0)
				{
					lstrcpy(lGroup,Group);
					//Из короткого имени группы создадим длинное
					ConvertGroupName(lGroup,GRP_TOLONGNAME);
					wsprintf(S,_T("%s: %s"),lGroup,Key);
					QuoteText(S,TRUE);

					if (Deactivated)
						lstrcpy(Button,GetMsg(MMacroTmpRest));
					else
						lstrcpy(Button,GetMsg(MMacroTmpDel));

					if (DeletingMacro(ItemsDelEmp,ARRAYSIZE(ItemsDelEmp),_T("MacroDelEmpty"))==DM_DELETED)
						deleted=TRUE;
				}

				if (!deleted)
				{
					if (_Group[0]!=0)
					{
						lstrcpy(lGroup,_Button);
						//Из короткого имени группы создадим длинное
						ConvertGroupName(lGroup,GRP_TOLONGNAME);

						if ((Conf.AutomaticSave) || (Info.Message(Info.ModuleNumber,0,NULL,ItemsEdit,
						                             ARRAYSIZE(ItemsEdit),2)==0))
						{
							// Если изменилась группа или команда
							// переместим сначала макрокоманду на новое место
							if ((CmpStr(_Group,Key)!=0) ||
							        (CmpStr(_Button,Group)!=0))
							{
								TCHAR *S1=new TCHAR[MAX_KEY_LEN];
								wsprintf(Str,_T("%s\\%s\\%s"),KeyMacros,_Button,_Group);
								wsprintf(S1,_T("%s\\%s\\~%s"),KeyMacros,_Button,_Group);
								CheckFirstBackSlash(Str,TRUE);
								CheckFirstBackSlash(S1,TRUE);

								if ((Reg->KeyExists(Str)) || (Reg->KeyExists(S1)))
								{
									wsprintf(S,_T("\"%s: %s\""),lGroup,_Group);
									int OldActive=ActiveMode;
									ActiveMode=MAC_ERRORACTIVE;

									if (Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsExist,
									                 ARRAYSIZE(ItemsExist),2)==0)
									{
										Reg->DeleteKey(S1);
										Reg->DeleteKey(Str);
										ActiveMode=OldActive;
									}
									else
									{
										delete[] S1;
										ActiveMode=OldActive;
										goto EDIT_RETRY;
									}
								}

								if (Deactivated)
								{
									wsprintf(S1,_T("%s\\%s\\~%s"),KeyMacros,Group,Key); //OldName
									wsprintf(S,_T("%s\\%s\\~%s"),KeyMacros,_Button,_Group); //NewName
								}
								else
								{
									wsprintf(S1,_T("%s\\%s\\%s"),KeyMacros,Group,Key); //OldName
									wsprintf(S,_T("%s\\%s\\%s"),KeyMacros,_Button,_Group); //NewName
								}

								CheckFirstBackSlash(S,TRUE);
								CheckFirstBackSlash(S1,TRUE);
								Reg->MoveKey(S1,S);
								delete[] S1;
							}

							// Теперь сохраняем параметры макрокоманды
							if (Deactivated)
								wsprintf(S,_T("%s\\%s\\~%s"),KeyMacros,_Button,_Group);
							else
								wsprintf(S,_T("%s\\%s\\%s"),KeyMacros,_Button,_Group);

							CheckFirstBackSlash(S,TRUE);

							if (!Reg->OpenKey(S))
							{
								int OldActive=ActiveMode;
								ActiveMode=MAC_ERRORACTIVE;
								QuoteText(S,TRUE);
								Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsErrorWrite,
								             ARRAYSIZE(ItemsErrorWrite),1);
								ActiveMode=OldActive;
							}
							else
							{
#ifndef UNICODE
								WriteRegValues(EditDialog);
#else
								WriteRegValues(EditDialog,hDlg);
#endif
								Reg->CloseKey();
								SwitchOver(_Button,_Group);
								RetVal=TRUE;
							}
						}
					}
					else
					{
						int OldActive=ActiveMode;
						ActiveMode=MAC_ERRORACTIVE;
						Info.Message(Info.ModuleNumber,FMSG_WARNING,_T("MacroGroups"),ItemsEditEmp,
						             ARRAYSIZE(ItemsEditEmp),1);
						ActiveMode=OldActive;
						goto EDIT_RETRY;
					}
				}

				// Копируем  в переменные Key и Group последние значения
				// для перехода потом на новое значение
				lstrcpy(Key,_Group);
				lstrcpy(Group,_Button);
#ifndef UNICODE
				Deactivated=EditDialog[28].Param.Selected;
#else
				Deactivated=(int)Info.SendDlgMessage(hDlg,DM_GETCHECK,28,0);
#endif
			}

#ifdef UNICODE
			Info.DialogFree(hDlg);
		}

#endif
		}

		Reg->CloseKey();
	}

	return RetVal;
}

#ifndef UNICODE
#undef _Group
#undef _Button
#undef _Descr
#undef _DataPtr
#endif


       /*************************************************/
       /* Создает список макросов и выводит в виде меню */
       /*************************************************/
int TMacroView::MacroList()
{
	ActlKeyMacro akm;
	InitData();
	//назначим ширину отображения макрокоманд,
	//в зависимости от установленного Conf.LongGroupNames
	KeyWidth=(Conf.LongGroupNames)?24:10;

	if (Conf.SaveOnStart)
	{
		akm.Command=MCMD_SAVEALL;
		ZeroMemory(akm.Param.Reserved,sizeof(akm.Param.Reserved));
		Info.AdvControl(Info.ModuleNumber,ACTL_KEYMACRO,&akm);
	}

//  SaveBar=Info.SaveScreen(0,0,-1,-1);
	InterceptDllCall(hInstance,"kernel32.dll","ReadConsoleInputA",
	                 (PVOID)&myReadConsoleInputA,(PVOID*)&p_fnReadConsoleInputOrgA) /*InterceptDllCall*/;
	InterceptDllCall(hInstance,"kernel32.dll","ReadConsoleInputW",
	                 (PVOID)&myReadConsoleInputW,(PVOID*)&p_fnReadConsoleInputOrgW) /*InterceptDllCall*/;
	InterceptDllCall(hInstance,"kernel32.dll","PeekConsoleInputA",
	                 (PVOID)&myPeekConsoleInputA,(PVOID*)&p_fnPeekConsoleInputOrgA) /*InterceptDllCall*/;
	InterceptDllCall(hInstance,"kernel32.dll","PeekConsoleInputW",
	                 (PVOID)&myPeekConsoleInputW,(PVOID*)&p_fnPeekConsoleInputOrgW) /*InterceptDllCall*/;

	if (Conf.StartDependentSort)
		lstrcpy(MenuTitle,GetMsg(MMacroTitleDS));
	else
		lstrcpy(MenuTitle,GetMsg(MMacroTitle));

	lstrcpy(MenuBottom,GetMsg(MMacroBottom));
	WriteKeyBar(KB_COMMON);
	GetConsoleTitle(OldCaption,ARRAYSIZE(OldCaption));
	MenuW=60;
	MenuH=14;
//  MenuX=MenuY=-1;
	ActiveMode=MAC_MENUACTIVE;
	InitDialogItem InitItems[]=
	{

		/*0*/  {DI_LISTBOX,   2,1,MenuW-3,MenuH-2,1,0,((Conf.UseHighlight)?DIF_LISTAUTOHIGHLIGHT:0)|((Conf.MenuCycle)?DIF_LISTWRAPMODE:0),0,_T("")},

	};
	FarDialogItem MenuDialog[1];
	InitDialogItems(InitItems,MenuDialog,ARRAYSIZE(InitItems));
#ifndef UNICODE
	Info.DialogEx(Info.ModuleNumber,MenuX,MenuY,MenuW,MenuH,_T("MacroView"),
	              MenuDialog,ARRAYSIZE(MenuDialog),0,0,
	              MenuDialogProc,0);
#else
	HANDLE hDlg = Info.DialogInit(Info.ModuleNumber,MenuX,MenuY,MenuW,MenuH,_T("MacroView"),
	                              MenuDialog,ARRAYSIZE(MenuDialog),0,0,
	                              MenuDialogProc,0);

	if (hDlg != INVALID_HANDLE_VALUE)
	{
		Info.DialogRun(hDlg);
		Info.DialogFree(hDlg);
	}

#endif
	InterceptDllCall(hInstance,"kernel32.dll","ReadConsoleInputA",
	                 (PVOID)p_fnReadConsoleInputOrgA,NULL) /*InterceptDllCall*/;
	p_fnReadConsoleInputOrgA=NULL;
	InterceptDllCall(hInstance,"kernel32.dll","ReadConsoleInputW",
	                 (PVOID)p_fnReadConsoleInputOrgW,NULL) /*InterceptDllCall*/;
	p_fnReadConsoleInputOrgW=NULL;
	InterceptDllCall(hInstance,"kernel32.dll","PeekConsoleInputA",
	                 (PVOID)p_fnPeekConsoleInputOrgA,NULL) /*InterceptDllCall*/;
	p_fnPeekConsoleInputOrgA=NULL;
	InterceptDllCall(hInstance,"kernel32.dll","PeekConsoleInputW",
	                 (PVOID)p_fnPeekConsoleInputOrgW,NULL) /*InterceptDllCall*/;
	p_fnPeekConsoleInputOrgW=NULL;
	Info.RestoreScreen(NULL);
//  Info.RestoreScreen(SaveBar);
//  SaveBar=NULL;
#ifndef UNICODE

	if (AreFileApisANSI())
		SetFileApisToOEM();

#endif
	akm.Command=MCMD_LOADALL;
	ZeroMemory(akm.Param.Reserved,sizeof(akm.Param.Reserved));
	Info.AdvControl(Info.ModuleNumber,ACTL_KEYMACRO,&akm);
	return 0;
}
