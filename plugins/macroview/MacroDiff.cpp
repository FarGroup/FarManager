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

  switch(Msg)
  {
    case DN_MOUSECLICK:
      if (Param1==0)
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
      return FALSE;
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
      Macro->csbi.dwSize=coord;
//      Macro->SaveBar=Info.SaveScreen(0,0,-1,-1);
      Macro->WriteKeyBar(KB_COMMON);
      Macro->FillMenu(hDlg,FALSE);
      return TRUE;
    }
    case DN_KEY:
    {
      Pos=(int)Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0/*(LONG_PTR)&ListPos*/);
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

      switch(Param2)
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
            for (i=Pos1;(Macro->Conf.MenuCycle && i<0)?i=Macro->MenuItemsNumber-1:i>=0;i--)
            {
              MenuData *MData1=(MenuData *)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(LONG_PTR)i);
              if (*MData1->Group && *MData1->Key)
              {
                if (CmpStr(MData->Group,MData1->Group)!=0)
                {
                  for(;i>=0;i--)
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
            for (i=Pos1;i<Macro->MenuItemsNumber;i++)
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
      char *Topic[]={"Contents","MacroView"};
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
      if (Param1==0)
      {
        FarListColors *ListColors=(FarListColors *)Param2;
        ListColors->Flags=0;
        ListColors->Reserved=0;
        ListColors->ColorCount=10;
        ListColors->Colors[0]=(BYTE)Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_MENUTEXT);
        ListColors->Colors[1]=(BYTE)Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_MENUTEXT);
        ListColors->Colors[2]=(BYTE)Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_MENUTITLE);
        ListColors->Colors[3]=(BYTE)Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_MENUTEXT);
        ListColors->Colors[4]=(BYTE)Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_MENUHIGHLIGHT);
        ListColors->Colors[5]=(BYTE)Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_MENUTEXT);
        ListColors->Colors[6]=(BYTE)Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_MENUSELECTEDTEXT);
        ListColors->Colors[7]=(BYTE)Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_MENUSELECTEDHIGHLIGHT);
        ListColors->Colors[8]=(BYTE)Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_MENUSCROLLBAR);
        ListColors->Colors[9]=(BYTE)Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_MENUDISABLEDTEXT);
        return TRUE;
      }
      return FALSE;
    case DN_CTLCOLORDLGITEM:
      if (Param1==1)
        return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_MENUTITLE))|
               ((Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_MENUHIGHLIGHT))<<8)|
               ((Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_MENUBOX))<<16);
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

      Info.DialogEx(Info.ModuleNumber,-1,-1,40,5,NULL,Macro->DefKeyDialog,
                    sizeof(Macro->DefKeyDialog)/sizeof(Macro->DefKeyDialog[0]),0,0,
                    DefKeyDialogProc,0);

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

  switch(Msg)
  {
    case DN_INITDIALOG:
    {
      Macro->EditDlg=hDlg;
      switch(Macro->EditMode)
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

        if (FSF.FarKeyToName((int)Param2,Macro->S,sizeof(Macro->S)))
        {
          FarDialogItemData ItemData;
          char StrBuf1[1024],StrBuf2[1024];
          ZeroMemory(StrBuf1,sizeof(StrBuf1));
          ZeroMemory(StrBuf2,sizeof(StrBuf2));
          if (Macro->MacroData)
          {
            if (*Macro->MacroData)
            {
              lstrcat(Macro->MacroData," ");
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
              wsprintf(StrBuf2,"%s %s",AllTrim(StrBuf1),Macro->S);
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

  switch(Msg)
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

        if (FSF.FarKeyToName((int)Param2,Macro->S,sizeof(Macro->S)))
        {
          FarDialogItemData ItemData;
          char StrBuf[1024];

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

  switch(Msg)
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
            Info.Text(0,0,0x4F,(char *)Macro->MacroText);
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
    switch(Macro->ActiveMode)
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
            Info.Text(0,0,0x4F,(char *)Macro->MacroText);
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
            if (CKey<KEY_END_FKEY && CKey>=0x20 || CKey==0x08 || CKey==0x09 || CKey==0x0d || CKey==0x1b)
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
            if (CKey<KEY_END_FKEY && CKey>=0x20 || CKey==0x08 || CKey==0x09 || CKey==0x0d || CKey==0x1b)
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
    switch(Macro->ActiveMode)
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
  MacroText("MACRO"),
  MacroCmdHistory("MacroCmd"),
  MacroKeyHistory("MacroKey"),
  MacroDescrHistory("MacroDescr"),
  MacroExpHistory("MacroExp"),
  MacroCopyHistory("MacroCopy")
{
  hOut=GetStdHandle(STD_OUTPUT_HANDLE);
  hIn=GetStdHandle(STD_INPUT_HANDLE);

  MacroData=new char[DATASIZE];
  if (MacroData)
  {
    MacroMulti=new char[DATASIZE];
    if (!MacroMulti)
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


void __fastcall TMacroView::InitMacroAreas()
{
  int i;

  char *GroupNumbers[]=
  {
    (char *)MMacroNameDialog,
    (char *)MMacroNameDisks,
    (char *)MMacroNameEditor,
    (char *)MMacroNameHelp,
    (char *)MMacroNameInfoPanel,
    (char *)MMacroNameMainMenu,
    (char *)MMacroNameMenu,
    (char *)MMacroNameQviewPanel,
    (char *)MMacroNameSearch,
    (char *)MMacroNameShell,
    (char *)MMacroNameTreePanel,
    (char *)MMacroNameViewer,
    (char *)MMacroNameOther,
    (char *)MMacroNameCommon,
  (char *)MMacroNameFindFolder,
  (char *)MMacroNameUserMenu
  };

  MacroGroupsSize=sizeof(MacroGroupShort)/sizeof(MacroGroupShort[0]);
//  MacroGroupsSize=sizeof(GroupNumbers)/sizeof(GroupNumbers[0]);

  for (i=0;i<MacroGroupsSize;i++)
  {
    if (Conf.LongGroupNames)
      lstrcpyn(GroupItems[i].Text,GetMsg((unsigned int)(DWORD_PTR)GroupNumbers[i]),sizeof(GroupItems[i].Text));
    else
      lstrcpyn(GroupItems[i].Text,MacroGroupShort[i],sizeof(GroupItems[i].Text));
    GroupItems[i].Flags=0;
  }

  GroupList.ItemsNumber=MacroGroupsSize;
  GroupList.Items=GroupItems;
}


void __fastcall TMacroView::InitDialogs()
{
  InitMacroAreas();

  //инициализируем диалог редактировния макрокоманды
  char Btn1[32],Btn2[32];
  lstrcpyn(Btn1,GetMsg(MMacroSave),sizeof(Btn1));
  lstrcpyn(Btn2,GetMsg(MMacroCancel),sizeof(Btn2));
  CheckLen(Btn1,sizeof(Btn1)-1,FALSE);
  CheckLen(Btn2,sizeof(Btn2)-1,FALSE);
  int len=lstrlen(Btn1)+4+2+lstrlen(Btn2)+4;
  int x11=(DIALOGWID+2-len)/2;
  int x12=x11+lstrlen(Btn1)+4+2-1;

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

  InitDialogItem InitItems[]=
  {

/* 0*/  {DI_DOUBLEBOX,  3, 1,DIALOGWID-4,DIALOGHGT-2,0,0,0,0,(char *)MMacroCombination},
/* 1*/  {DI_TEXT,       5, 2, 0,0,0,0,0,0,CheckLen(GetMsg(MMacroKey),30)},
/* 2*/  {DI_EDIT,       5, 3,35,3,0,(DWORD_PTR)MacroKeyHistory,DIF_HISTORY,0,""},
/* 3*/  {DI_TEXT,      39, 2, 0,0,0,0,0,0,CheckLen(GetMsg(MMacroGroup),30)},
/* 4*/  {DI_COMBOBOX,  39, 3,69,3,0,0,DIF_DROPDOWNLIST,0,""},
/* 5*/  {DI_TEXT,       5, 4, 0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
/* 6*/  {DI_VTEXT,     37, 2, 0,0,0,0,0,0,"\xB3\xB3\xC1"},
/* 7*/  {DI_TEXT,       5, 5, 0,0,0,0,0,0,CheckLen(GetMsg(MMacroCommand),64)},
/* 8*/  {DI_EDIT,       5, 6,69,0,1,(DWORD_PTR)MacroCmdHistory,(MacroData)?DIF_HISTORY|DIF_VAREDIT:DIF_HISTORY,0,""},
/* 9*/  {DI_TEXT,       5, 7, 0,0,0,0,0,0,CheckLen(GetMsg(MMacroDescription),64)},
/*10*/  {DI_EDIT,       5, 8,69,0,0,(DWORD_PTR)MacroDescrHistory,DIF_HISTORY,0,""},
/*11*/  {DI_TEXT,       5, 9, 0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},

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

/*24*/  {DI_TEXT,       5,16, 0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
/*25*/  {DI_VTEXT,     37, 9, 0,0,0,0,0,0,"\xC2\xB3\xB3\xB3\xB3\xB3\xB3\xC1"},

/*26*/  {DI_CHECKBOX,   5,17, 0,0,0,0,0,0,CheckLen(GetMsg(MMacroSendToPlugins),62)},

/*27*/  {DI_TEXT,       5,18, 0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},

/*28*/  {DI_CHECKBOX,   5,19, 0,0,0,0,0,0,CheckLen(GetMsg(MMacroSwitchOff),62)},
/*29*/  {DI_TEXT,       5,20, 0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
/*30*/  {DI_BUTTON,   x11,21, 0,0,0,0,0,1,Btn1},
/*31*/  {DI_BUTTON,   x12,21, 0,0,0,0,0,0,Btn2},

  };

  InitDialogItems(InitItems,EditDialog,sizeof(InitItems)/sizeof(InitItems[0]));

  InitDialogItem DefItems[]=
  {

/* 0*/  {DI_DOUBLEBOX,  3, 1,40-4,5-2,0,0,0,0,(char *)MMacroDefineTitle},
/* 1*/  {DI_TEXT,       5, 2, 0,0,0,0,DIF_CENTERGROUP,0,CheckLen(GetMsg(MMacroDesiredKey),28)},

  };

  InitDialogItems(DefItems,DefKeyDialog,sizeof(DefItems)/sizeof(DefItems[0]));
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

  GetConsoleScreenBufferInfo(hOut,&csbi);
  ReadConfig();

  if (MacroData)
    ZeroMemory(MacroData,DATASIZE);
  InitDialogs();
}


/*void __fastcall TMacroView::ParseMenuItem(FarListGetItem *List)
{
  char *ptr,*ptr1;
  *Group=*Key=0;

  if (List->Item.Flags&LIF_CHECKED)
    Deactivated=TRUE;
  else
    Deactivated=FALSE;
  lstrcpy(S,List->Item.Text);
//  while((ptr=strchr(S,'&'))!=NULL) memmove(ptr,ptr+1,lstrlen(S)-(ptr-S)+1);

  ptr=strchr(S,':');
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
  GetConsoleScreenBufferInfo(hOut,&csbi);

  if (MenuDlg)
  {
    Info.SendDlgMessage(MenuDlg,DM_GETDLGRECT,0,(LONG_PTR)&rect);
    if (rect.Bottom>=csbi.dwSize.Y-1)
      return;
    if (HelpActivated)
      return;
  }
  if (EditDlg)
  {
    Info.SendDlgMessage(EditDlg,DM_GETDLGRECT,0,(LONG_PTR)&rect);
    if (rect.Bottom>=csbi.dwSize.Y-1)
      return;
    if (HelpActivated)
      return;
  }
  if ((csbi.dwCursorPosition.Y==csbi.dwSize.Y-1) && (OpenFrom==OPEN_PLUGINSMENU))
    return;

  char *KeyBarCommon[12]={GetMsg(MMacroKBHelp),
    GetMsg(MMacroKBSave),NULL,GetMsg(MMacroKBEdit),
    GetMsg(MMacroKBCopy),GetMsg(MMacroKBMove),
    NULL,NULL,NULL,
    GetMsg(MMacroKBQuit),NULL,NULL,
  };
  char *KeyBarAlt[12]={NULL,
    NULL,NULL,NULL,
    NULL,NULL,
    NULL,NULL,NULL,
    NULL,NULL,NULL,
  };
  char *KeyBarCtrl[12]={NULL,
    NULL,NULL,NULL,
    NULL,NULL,
    NULL,NULL,NULL,
    NULL,NULL,NULL,
  };
  char *KeyBarShift[12]={NULL,
    GetMsg(MMacroKBAltSaveAll),NULL,NULL,
    NULL,NULL,
    NULL,NULL,NULL,
    NULL,NULL,NULL,
  };
  char *KeyBarDialog[12]={GetMsg(MMacroKBHelp),
    NULL,NULL,NULL,
    NULL,NULL,
    NULL,NULL,NULL,
    GetMsg(MMacroKBQuit),NULL,NULL,
  };
  char *KeyBarShiftDialog[12]={NULL,
    NULL,NULL,NULL,
    NULL,NULL,
    NULL,NULL,NULL,
    NULL,NULL,NULL,
  };
  char *KeyBar[12];

  //Узнаем ширину экрана консоли
  //int ScrWidth=csbi.dwSize.X+1;

  switch(kbType)
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
  CHAR_INFO *chi=new CHAR_INFO[csbi.dwSize.X];

  COORD Size,Coord;
  SMALL_RECT Region;
  Size.X=csbi.dwSize.X;
  Size.Y=1;
  Coord.X=0;
  Coord.Y=0;
  Region.Left=0;
  Region.Top=csbi.dwSize.Y-1;
  Region.Right=csbi.dwSize.X-1;
  Region.Bottom=csbi.dwSize.Y-1;

  ReadConsoleOutput(hOut,chi,Size,Coord,&Region);

  int ColDigit=(int)Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_KEYBARNUM);
  int ColText=(int)Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_KEYBARTEXT);
  if (chi[0].Char.AsciiChar=='1' && chi[0].Attributes==ColDigit && chi[1].Attributes==ColText)
  {
    for (i=0,j=1;j<csbi.dwSize.X;i++)
    {
      if (KeyBar[i] && *KeyBar[i])
      {
        k=0;
        while (chi[j].Attributes==ColText && j<csbi.dwSize.X)
        {
          if (KeyBar[i][k]!=0)
            chi[j++].Char.AsciiChar=KeyBar[i][k++];
          else
            chi[j++].Char.AsciiChar=' ';
        }
      }
      else
      {
        while (chi[j].Attributes==ColText && j<csbi.dwSize.X)
        {
          chi[j++].Char.AsciiChar=' ';
        }
      }

      while(chi[j].Attributes!=ColText && j<csbi.dwSize.X)
        j++;
    }

    WriteConsoleOutput(hOut,chi,Size,Coord,&Region);
  }

  delete[] chi;
}


BOOL __fastcall TMacroView::CreateDirs(char *Dir)
{
  char Str[MAX_PATH_LEN];
  char *ErrorCreateFolder[]=
  {
    GetMsg(MMacroError),
    GetMsg(MMacroErrorCreateFolder),
    S,
    "\x1",
    GetMsg(MMacroOk),
  };

  char *Data=Dir;
  char *ptrend=Data+lstrlen(Data);
  char *ptr;

  // сетевой путь?
  if ((Data[0]=='\\') && (Data[1]=='\\'))
  {
    ptr=strchr(&Data[2],'\\');
    if (ptr)
    {
      char *ptr1=strchr(ptr+1,'\\');
      if (ptr1)
      {
        char *ptr2=strchr(ptr1+1,'\\');
        if (!ptr2)
          return TRUE;
      }
      else
        return TRUE;
      Data=ptr1+1;
    }
    else
      return TRUE;
  }
  else
  {
    ptr=strchr(Data,'\\');
    if (!ptr)
      return TRUE;
  }

  BOOL endstr=FALSE;
  while(1)
  {
    ZeroMemory(Str,sizeof(Str));
    ptr=strchr(Data,'\\');
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
                     sizeof(ErrorCreateFolder)/sizeof(ErrorCreateFolder[0]),1);
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


char *TMacroView::ConvertGroupName(char *Group,int nWhere)
{
  int i,len;

  if (Conf.LongGroupNames)
  {
    len=lstrlen(Group);
    if (len>=3)
      if ((Group[len-1]=='.') && (Group[len-2]=='.') && (Group[len-3]=='.'))
        Group[len-3]=0;

    if (len>0)
    {
      switch(nWhere)
      {
        case GRP_TOLONGNAME:
          for (i=0;i<MacroGroupsSize;i++)
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
          for (i=0;i<MacroGroupsSize;i++)
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
  for (int I=0;I<ItemsNumber;I++)
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
    if ((unsigned int)(DWORD_PTR)Init[I].Data<300)
      lstrcpy(Item[I].Data.Data,GetMsg((unsigned int)(DWORD_PTR)Init[I].Data));
    else
      lstrcpy(Item[I].Data.Data,Init[I].Data);
  }
}


void __fastcall TMacroView::InsertMacroToEditor(BOOL AllMacros)
{
  char lKey[MAX_KEY_LEN];
  char lGroup[MAX_KEY_LEN];
  char *TmpPrfx="mvu";
  HANDLE fname;

  char *ErrorRun[]=
  {
    GetMsg(MMacroError),
    GetMsg(MMacroErrorRun),
    "\x1",
    GetMsg(MMacroOk),
  };
  char *ErrorCreateTemp[]=
  {
    GetMsg(MMacroError),
    GetMsg(MMacroErrorCreateTemp),
    "\x1",
    GetMsg(MMacroOk),
  };
  char *ErrorReadTemp[]=
  {
    GetMsg(MMacroError),
    GetMsg(MMacroErrorReadTemp),
    "\x1",
    GetMsg(MMacroOk),
  };
  char *ErrorInsertStrEditor[]=
  {
    GetMsg(MMacroError),
    GetMsg(MMacroErrorInsertStrEditor),
    S,
    "\x1",
    GetMsg(MMacroOk),
  };

  SetFileApisToANSI();

/*  if (GetEnvironmentVariable(TempEnvName,TempPath,sizeof(TempPath))==0)
    if (GetCurrentDirectory(sizeof(TempPath),TempPath)==0)
      lstrcpy(TempPath,"C:\\");*/

//  if (GetTempFileName(TempPath,TmpPrfx,0,TempFileName)==0)
  if (FSF.MkTemp(TempFileName,TmpPrfx)==NULL)
  {
    int OldActive=ActiveMode;
    ActiveMode=MAC_ERRORACTIVE;
    Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,ErrorCreateTemp,
                 sizeof(ErrorCreateTemp)/sizeof(ErrorCreateTemp[0]),1);
    ActiveMode=OldActive;
    SetFileApisToOEM();
    return;
  }

  if (*Group && *Key)
  {
    lstrcpy(lKey,Key);
    lstrcpy(lGroup,Group);

    if (Deactivated)
    {
      lstrcpy(lKey,"~");
      lstrcat(lKey,Key);
    }
    if (AllMacros)
      lstrcpy(S,KeyMacros);
    else
      wsprintf(S,"%s\\%s\\%s",KeyMacros,lGroup,lKey);
    CheckFirstBackSlash(S,FALSE);

    ZeroMemory(&si,sizeof(si));
    si.cb=sizeof(si);

    char regedit[32];
    if ((vi.dwPlatformId==VER_PLATFORM_WIN32_NT) && (vi.dwMajorVersion>=5))
      lstrcpy(regedit,"regedit -ea");
    else
      lstrcpy(regedit,"regedit -e");

    wsprintf(Str,"%s \"%s\" \"%s\\%s\"",regedit,TempFileName,HKCU,S);

    int Code=CreateProcess(NULL,Str,NULL,NULL,TRUE,
                           0,NULL,NULL,&si,&pi);
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
                   sizeof(ErrorRun)/sizeof(ErrorRun[0]),1);
      ActiveMode=OldActive;
      SetFileApisToOEM();
      return;
    }

    fname=CreateFile(TempFileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
    if(fname==INVALID_HANDLE_VALUE)
    {
      int OldActive=ActiveMode;
      ActiveMode=MAC_ERRORACTIVE;
      Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ErrorReadTemp,
                   sizeof(ErrorReadTemp)/sizeof(ErrorReadTemp[0]),1);
      ActiveMode=OldActive;
      SetFileApisToOEM();
      return;
    }

    int iteration=1;
    int indent=1;
    DWORD t;
    while (1)
    {
      if (!(ReadFile(fname,S,sizeof(S)-1,&t,NULL) && t))
        break;

      DWORD i;
      for (i=0;i<t;i++)
      {
        if ((S[i]=='\n') || (S[i]=='\r'))
        {
          if (S[i]=='\r' && i+1<t && S[i+1]=='\n')
            S[i++]=0;
          S[i++]=0;
          break;
        }
      }
      S[t]=0;
      SetFilePointer(fname,-((int)(t-i)),NULL,FILE_CURRENT);

      if (iteration==1)
      {
        if (Info.EditorControl(ECTL_INSERTTEXT,S)==0)
        {
          lstrcpy(S,lGroup);
          ConvertGroupName(S,GRP_TOLONGNAME);
          int OldActive=ActiveMode;
          ActiveMode=MAC_ERRORACTIVE;
          Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ErrorInsertStrEditor,
                   sizeof(ErrorInsertStrEditor)/sizeof(ErrorInsertStrEditor[0]),1);
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
                   sizeof(ErrorInsertStrEditor)/sizeof(ErrorInsertStrEditor[0]),1);
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
  SetFileApisToOEM();
}


void __fastcall TMacroView::ExportMacroToFile(BOOL AllMacros)
{
  char lKey[MAX_KEY_LEN];
  char lGroup[MAX_KEY_LEN];
  char sDest[EXPORTLEN];
  char MacroHelp[64];
  char *TmpPrfx="mvu";

  char *ErrorRun[]=
  {
    GetMsg(MMacroError),
    GetMsg(MMacroErrorRun),
    "\x1",
    GetMsg(MMacroOk),
  };
  char *ErrorExists[]=
  {
    GetMsg(MMacroWarning),GetMsg(MMacroWarningFileExists),
    S,"\x1",sDest,"\x1",
    GetMsg(MMacroOverwrite),GetMsg(MMacroRename),GetMsg(MMacroCancel)
  };
  char *ErrorCreateFile[]=
  {
    GetMsg(MMacroError),
    GetMsg(MMacroErrorCreateFile),
    Str,
    "\x1",
    GetMsg(MMacroOk),
  };

  if (*Group && *Key)
  {
    lstrcpy(lKey,Key);
    lstrcpy(lGroup,Group);

    if (Deactivated)
    {
      lstrcpy(lKey,"~");
      lstrcat(lKey,Key);
    }

    struct InitDialogItem InitItems[]=
    {
      {DI_DOUBLEBOX,3,1,EXPORTLEN+2,6,0,0,0,0,(char *)MMacroExport},
      {DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,""},
      {DI_EDIT,5,3,EXPORTLEN,3,1,(DWORD_PTR)MacroExpHistory,DIF_HISTORY,0,""},
      {DI_TEXT,5,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
      {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,1,(char *)MMacroSave},
      {DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,0,(char *)MMacroCancel}
    };

    FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
    InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

    wsprintf(Str,"%s: %s",ConvertGroupName(Group,GRP_TOLONGNAME),Key);
    QuoteText(Str,TRUE);
    if (AllMacros)
    {
      lstrcpy(DialogItems[1].Data.Data,GetMsg(MMacroExportAllKey));
      lstrcpy(DialogItems[2].Data.Data,"KeyMacros.reg");
      lstrcpy(MacroHelp,"MacroExportAll");
    }
    else
    {
      wsprintf(DialogItems[1].Data.Data,GetMsg(MMacroExportKey),Str);
      wsprintf(DialogItems[2].Data.Data,"%s.reg",Key);
      lstrcpy(MacroHelp,"MacroExport");
    }

    WriteKeyBar(KB_DIALOG);

MACRO_DIALOG:

    int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,EXPORTLEN+6,8,MacroHelp,DialogItems,
                             sizeof(DialogItems)/sizeof(DialogItems[0]));
    if (ExitCode==4) // Сохранить
    {
      ZeroMemory(&si,sizeof(si));
      si.cb=sizeof(si);

      char *ptr=strrchr(DialogItems[2].Data.Data,'\\');
      if (ptr)
      {
        ZeroMemory(Str,sizeof(Str));
        lstrcpyn(Str,DialogItems[2].Data.Data,(int)(ptr-DialogItems[2].Data.Data)+1);
        if (!CreateDirs(Str))
          goto MACRO_DIALOG;
      }

      char AnsiName[MAX_PATH_LEN];
      SetFileApisToANSI();
      OemToChar(DialogItems[2].Data.Data,AnsiName);

      ZeroMemory(TempPath,sizeof(TempPath));
      if (ptr)
      {
        lstrcpyn(TempPath,DialogItems[2].Data.Data,(int)(ptr-DialogItems[2].Data.Data)+1);
        wsprintf(TempFileName,"%s\\%s%x.tmp",TempPath,TmpPrfx,102938);
      }
      else
      {
        lstrcpy(TempPath,"");
        wsprintf(TempFileName,"%s%x.tmp",TmpPrfx,102938);
      }

      HANDLE hTemp;
      if ((hTemp=CreateFile(TempFileName,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
                             CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,NULL))==INVALID_HANDLE_VALUE)
      {
        lstrcpy(Str,DialogItems[2].Data.Data);
        CheckRLen(Str,EXPORTLEN-8);
        int OldActive=ActiveMode;
        ActiveMode=MAC_ERRORACTIVE;
        Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,ErrorCreateFile,
                     sizeof(ErrorCreateFile)/sizeof(ErrorCreateFile[0]),1);
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

//        wsprintf(sDest,"  %-24s %10lu %02u.%02u.%02u %02u:%02u:%02u  ",
        FSF.sprintf(sDest,"  %-*s %10lu %02u.%02u.%02u %02u:%02u:%02u  ",24,
                 GetMsg(MMacroDestination),
                (fData.nFileSizeHigh*MAXDWORD)+fData.nFileSizeLow,
                 st.wDay,st.wMonth,st.wYear,
                 st.wHour,st.wMinute,st.wSecond);

        lstrcpy(S,DialogItems[2].Data.Data);
        CheckRLen(S,EXPORTLEN-8);

        int OldActive=ActiveMode;
        ActiveMode=MAC_ERRORACTIVE;
        int ExitCode=Info.Message(Info.ModuleNumber,FMSG_WARNING,"MacroExportExist",ErrorExists,
                     sizeof(ErrorExists)/sizeof(ErrorExists[0]),3);
        ActiveMode=OldActive;

        FindClose(hand);
        switch(ExitCode)
        {
          case 0:
            break;
          case 1:
            SetFileApisToOEM();
            goto MACRO_DIALOG;
          default:
            SetFileApisToOEM();
            return;
        }
      }

      if (AllMacros)
        lstrcpy(S,KeyMacros);
      else
        wsprintf(S,"%s\\%s\\%s",KeyMacros,lGroup,lKey);
      CheckFirstBackSlash(S,FALSE);

      char regedit[32];
      if ((vi.dwPlatformId==VER_PLATFORM_WIN32_NT) && (vi.dwMajorVersion>=5))
        lstrcpy(regedit,"regedit -ea");
      else
        lstrcpy(regedit,"regedit -e");

      wsprintf(Str,"%s \"%s\" \"%s\\%s\"",regedit,AnsiName,HKCU,S);

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
                       sizeof(ErrorRun)/sizeof(ErrorRun[0]),1);
          ActiveMode=OldActive;
        }
      }
      else
      {
        int OldActive=ActiveMode;
        ActiveMode=MAC_ERRORACTIVE;
        Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,ErrorRun,
                     sizeof(ErrorRun)/sizeof(ErrorRun[0]),1);
        ActiveMode=OldActive;
      }
      SetFileApisToOEM();
    }
  }
}


void TMacroView::SwitchOver(char *Group,char *Key)
{
  char lKey[MAX_KEY_LEN];
  char lGroup[MAX_KEY_LEN];

  if (*Group && *Key)
  {
    lstrcpy(lKey,Key);
    lstrcpy(lGroup,Group);

    if (Deactivated)
    {
      lstrcpy(lKey,"~");
      lstrcat(lKey,Key);
    }

    wsprintf(S,"%s\\%s\\%s",KeyMacros,lGroup,lKey); //полное имя ключа реестра
    CheckFirstBackSlash(S,TRUE);

    if ((EditDialog[28].Param.Selected) && (!Deactivated))  // отключить макрокоманду
    {
      wsprintf(Str,"%s\\%s\\~%s",KeyMacros,lGroup,lKey);
      Reg->MoveKey(S,Str); //(OldName,NewName);
    }
    else if ((!EditDialog[28].Param.Selected) && (Deactivated)) // восстановить макрокоманду
    {
      wsprintf(Str,"%s\\%s\\%s",KeyMacros,lGroup,&lKey[1]);
      Reg->MoveKey(S,Str); //(OldName,NewName);
    }
  }
}


BOOL TMacroView::DeletingMacro(char **Items,int ItemsSize,char *HelpTopic)
{
  char lKey[MAX_KEY_LEN];
  int lCode;

  if (*Group && *Key)
  {
    lstrcpy(lKey,Key);

    if (Deactivated)
    {
      lstrcpy(lKey,"~");
      lstrcat(lKey,Key);
    }
    lCode=Info.Message(Info.ModuleNumber,FMSG_WARNING,HelpTopic,Items,ItemsSize,3);

    wsprintf(S,"%s\\%s\\%s",KeyMacros,Group,lKey); //полное имя ключа реестра
    CheckFirstBackSlash(S,TRUE);

    if (lCode==0)
    {
      Reg->DeleteKey(S);
      wsprintf(S,"%s\\%s",KeyMacros,Group);
      Reg->OpenKey(S);
      if (!Reg->HasSubKeys())
        Reg->DeleteKey(S);
      Reg->CloseKey();
      return DM_DELETED;
    }
    else if (lCode==1)
    {
      if (Deactivated)  //TRUE - восстановить
        wsprintf(Str,"%s\\%s\\%s",KeyMacros,Group,&lKey[1]);
      else         //FALSE - отключить
        wsprintf(Str,"%s\\%s\\~%s",KeyMacros,Group,lKey);
      Reg->MoveKey(S,Str); //(OldName,NewName);
      return DM_DEACTIVATED;
    }
  }
  return DM_NONE;
}


BOOL __fastcall TMacroView::CopyMoveMacro(int Op)
{
  BOOL lResult=FALSE;

  char S1[MAX_PATH_LEN];
  char LocalKeyMacros[MAX_PATH_LEN];
  char lKey[MAX_KEY_LEN];
  char lGroup[MAX_KEY_LEN];

  // Если пользовательских конфигураций в фаре нет, то
  // количество пользователей = 1 (Основная конфигурация)
  int UserCount=1;

  // Индекс текущей конфигурации в списке пользовательских конфигураций.
  UserConfPos=0;
  // Индекс области копирования.
  GroupPos=0;

  char *ItemsErrorConf[]=
  {
    GetMsg(MMacroError),
    GetMsg(MMacroErrorSelectConfiguration),
    "\x1",
    GetMsg(MMacroOk),
  };
  char *ItemsCopy[]=
  {
    GetMsg(MMacroWarning),
    S,
    "\x1",
    GetMsg(MMacroOk),
  };
  char *ItemsExist[]=
  {
    GetMsg(MMacroWarning),GetMsg(MMacroWarningExist),
    S,
    "\x1",
    GetMsg(MMacroOverwrite),GetMsg(MMacroCancel)
  };
  char *ItemsKeyEmp[]=
  {
    GetMsg(MMacroWarning),
    S,
    "\x1",
    GetMsg(MMacroOk),
  };

  // Создадим локальную копию адреса расположения макросов в реестре
  // в основной конфигурации пользователя.
  lstrcpy(LocalKeyMacros,KeyMacros);

  // Создадим список, в который будем читать из реестра имена пользовательских конфигураций.
  TStrList *UserList=new TStrList;

  if (!Reg->OpenKey(FarUsersKey))
  {
    // Создадим первый элемент списка конфигураций - общий.
    ConfItems=new FarListItem[1];
    if (ConfItems==NULL)
    {
      delete UserList;
      int OldActive=ActiveMode;
      ActiveMode=MAC_ERRORACTIVE;
//      Error(erNotMemory);
      ActiveMode=OldActive;
      return lResult;
    }

    ConfItems[0].Flags=0;
    lstrcpyn(ConfItems[0].Text,GetMsg(MMacroDefaultConfig),sizeof(ConfItems[0].Text));
    ZeroMemory(ConfItems[0].Reserved,sizeof(ConfItems[0].Reserved));
  }
  else
  {
    if (Reg->GetKeyNames(UserList))
    {
      if (UserList->GetCount()>0)
        UserCount++;
      UserCount+=UserList->GetCount();
      ConfItems=new FarListItem[UserCount];
      if (ConfItems==NULL)
      {
        delete UserList;
        int OldActive=ActiveMode;
        ActiveMode=MAC_ERRORACTIVE;
//        Error(erNotMemory);
        ActiveMode=OldActive;
        return lResult;
      }

      ConfItems[0].Flags=0;
      lstrcpyn(ConfItems[0].Text,GetMsg(MMacroDefaultConfig),sizeof(ConfItems[0].Text));
      ZeroMemory(ConfItems[0].Reserved,sizeof(ConfItems[0].Reserved));

      if (UserCount>1)
      {
        ConfItems[1].Flags=LIF_SEPARATOR;
        ConfItems[1].Text[0]=0;
        ZeroMemory(ConfItems[1].Reserved,sizeof(ConfItems[1].Reserved));

        for (int i=0;i<UserList->GetCount();i++)
        {
          ConfItems[i+2].Flags=0;
          lstrcpyn(ConfItems[i+2].Text,UserList->GetText(i),sizeof(ConfItems[i+2].Text));
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
        delete UserList;
        int OldActive=ActiveMode;
        ActiveMode=MAC_ERRORACTIVE;
//        Error(erNotMemory);
        ActiveMode=OldActive;
        return lResult;
      }

      ConfItems[0].Flags=0;
      lstrcpyn(ConfItems[0].Text,GetMsg(MMacroDefaultConfig),sizeof(ConfItems[0].Text));
      ZeroMemory(ConfItems[0].Reserved,sizeof(ConfItems[0].Reserved));
    }
  }

  delete UserList;

  ConfList.ItemsNumber=UserCount;
  ConfList.Items=ConfItems;

  // Заполним список значений макрогрупп.
  InitMacroAreas();

  struct InitDialogItem InitItems[]=
  {
/* 0*/ {DI_DOUBLEBOX,            3,1,EXPORTLEN+2,9,0,0,0,0,""},
/* 1*/ {DI_TEXT,                 5,2,EXPORTLEN/2,0,0,0,0,0,CheckLen(GetMsg(MMacroSelectConfig),EXPORTLEN/2)},
/* 2*/ {DI_COMBOBOX,             5,3,EXPORTLEN/2,3,1,0,DIF_DROPDOWNLIST,0,""},
/* 3*/ {DI_TEXT,     EXPORTLEN/2+4,2,  EXPORTLEN,0,0,0,0,0,CheckLen(GetMsg(MMacroCopyMoveTo),EXPORTLEN/2-2)},
/* 4*/ {DI_COMBOBOX, EXPORTLEN/2+4,3,  EXPORTLEN,3,0,0,DIF_DROPDOWNLIST,0,""},
/* 5*/ {DI_TEXT,                 5,4,          0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
/* 6*/ {DI_VTEXT,    EXPORTLEN/2+2,2,          0,0,0,0,0,0,"\xB3\xB3\xC1"},
/* 7*/ {DI_TEXT,                 5,5,  EXPORTLEN,0,0,0,0,0,CheckLen(GetMsg(MMacroNewKey),EXPORTLEN)},
/* 8*/ {DI_EDIT,                 5,6,  EXPORTLEN,5,0,(DWORD_PTR)MacroCopyHistory,DIF_HISTORY,0,""},
/* 9*/ {DI_TEXT,                 5,7,          0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
/*10*/ {DI_BUTTON,               0,8,          0,0,0,0,DIF_CENTERGROUP,1,(char *)MMacroSave},
/*11*/ {DI_BUTTON,               0,8,          0,0,0,0,DIF_CENTERGROUP,0,(char *)MMacroCancel}
  };

  FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  // Скопируем в локальный буфер имя группы из которой производим копирование.
  lstrcpy(lGroup,Group);
  // Преобразуем наименование группы из которой копируем в длинное имя.
  ConvertGroupName(lGroup,GRP_TOLONGNAME);

  // Подготовим строку имени копируемого макроса для заголовка диалога.
  wsprintf(S1,"\"%s: %s\"",lGroup,Key);
  if (Op==KEY_F5)
    // Установим заголовок диалога - Копирование
    wsprintf(DialogItems[0].Data.Data,"%s %s",GetMsg(MMacroCopy),S1);
  else
    // Установим заголовок диалога - Перемещение
    wsprintf(DialogItems[0].Data.Data,"%s %s",GetMsg(MMacroMove),S1);

  // Инициализируем содержимое поля команды выполнения.
  lstrcpyn(DialogItems[8].Data.Data,Key,sizeof(DialogItems[8].Data.Data));

COPY_MOVE:

  int ExitCode=Info.DialogEx(Info.ModuleNumber,-1,-1,EXPORTLEN+6,11,"MacroCopy",
           DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]),0,0,
           CopyDialogProc,0);

  if (ExitCode==10) // Сохранить
  {
    if (DialogItems[8].Data.Data[0]==0) // New key is empty
    {
      wsprintf(S,"%s %s",GetMsg(MMacroWarningInsertEmpty0),GetMsg(MMacroWarningInsertEmpty2));

      int OldActive=ActiveMode;
      ActiveMode=MAC_ERRORACTIVE;
      Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsKeyEmp,
        sizeof(ItemsKeyEmp)/sizeof(ItemsKeyEmp[0]),1);
      ActiveMode=OldActive;
      goto COPY_MOVE;
    }

    // Создадим локальную копию адреса расположения макросов в реестре,
    // в зависимости от выбранной конфигурации пользователя.
    if (UserConfPos==0)
      wsprintf(LocalKeyMacros,"%s\\%s",Default_KEY,KeyMacros_KEY);
    else if (UserConfPos>1)
      wsprintf(LocalKeyMacros,"%s\\%s\\%s",FarUsersKey,DialogItems[2].Data.Data,KeyMacros_KEY);
    else
    {
      int OldActive=ActiveMode;
      ActiveMode=MAC_ERRORACTIVE;
      Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsErrorConf,
          sizeof(ItemsErrorConf)/sizeof(ItemsErrorConf[0]),1);
      delete[] ConfItems;
      ActiveMode=OldActive;
      return lResult;
    }

    char *Str1=new char[MAX_PATH_LEN];
    lstrcpy(lGroup,DialogItems[4].Data.Data); // New group
    lstrcpy(lKey,DialogItems[8].Data.Data); // New key

    // Сконвертируем имя группы назначения в короткое имя
    ConvertGroupName(lGroup,GRP_TOSHORTNAME);
    if (Deactivated)
    {
      wsprintf(S,"%s\\%s\\~%s",KeyMacros,Group,Key);
      wsprintf(S1,"%s\\%s\\~%s",LocalKeyMacros,lGroup,lKey);
      wsprintf(Str1,"%s\\%s\\%s",LocalKeyMacros,lGroup,lKey);
    }
    else
    {
      wsprintf(S,"%s\\%s\\%s",KeyMacros,Group,Key);
      wsprintf(S1,"%s\\%s\\%s",LocalKeyMacros,lGroup,lKey);
      wsprintf(Str1,"%s\\%s\\~%s",LocalKeyMacros,lGroup,lKey);
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
        sizeof(ItemsCopy)/sizeof(ItemsCopy[0]),1);
      delete[] Str1;
      ActiveMode=OldActive;
      goto COPY_MOVE;
    }

    if ((Reg->KeyExists(S1)) || (Reg->KeyExists(Str1)))
    {
      lstrcpy(Str,S);
      wsprintf(S,"\"%s: %s\"",DialogItems[4].Data.Data,lKey);
      int OldActive=ActiveMode;
      ActiveMode=MAC_ERRORACTIVE;
      if (Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsExist,
          sizeof(ItemsExist)/sizeof(ItemsExist[0]),2)!=0)
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
      wsprintf(S,"%s\\%s",KeyMacros,Group);
      if (Reg->OpenKey(S))
      {
        if (!Reg->HasSubKeys())
          Reg->DeleteKey(S);
        Reg->CloseKey();
      }
    }

    delete[] Str1;
    delete[] ConfItems;

    // Запомним новые имена группы и команды выполнения
    lstrcpy(Group,lGroup);
    lstrcpy(Key,lKey);

    lResult=TRUE;
  }

  return lResult;
}


void TMacroView::MoveTildeInKey(TStrList *&List,BOOL doit)
{
  int i,len;

  for (i=0;i<List->GetCount();i++)
  {
    List->GetText(Str,i);
    len=lstrlen(Str);
    if (doit)
    {
      if ((*Str=='~') && (len>1))
      {
        MoveMemory(Str,&Str[1],len-1);
        Str[len-1]='\x1';
        List->SetText(Str,i);
      }
    }
    else
    {
      if ((len>1) && (Str[len-1]=='\x1'))
      {
        MoveMemory(&Str[1],Str,len-1);
        *Str='~';
        List->SetText(Str,i);
      }
    }

  }
}


void TMacroView::PrepareDependentSort(TStrList *&List,BOOL doit)
{
  int i;
//  char Group[MAX_KEY_LEN];

  for(i=0;i<List->GetCount();i++)
  {
    // сейчас в Str содержится длинное имя группы макрокоманд
    List->GetText(Str,i);
    // сконвертируем Str в короткое имя группы, хранящееся в реестре
    ConvertGroupName(Str,GRP_TOSHORTNAME);
    if (doit)
    {
      if (((OpenFrom==OPEN_PLUGINSMENU) && (CmpStr(Str,"Shell")==0))  ||
          ((OpenFrom==OPEN_VIEWER)      && (CmpStr(Str,"Viewer")==0)) ||
          ((OpenFrom==OPEN_EDITOR)      && (CmpStr(Str,"Editor")==0)))
      {
        wsprintf(S,"\x1%s",List->GetText(i));
        List->SetText(S,i);
      }
    }
    else
    {
      if (*Str=='\x1')
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
  char Group[MAX_KEY_LEN];
  char OldGroup[MAX_KEY_LEN];
  char Key[MAX_KEY_LEN];
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
      for (i=0;i<NameList->GetCount();i++)
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

    for (i=0;i<NameList->GetCount();i++)
    {
      // сейчас в Group и содержится длинное имя группы макрокоманд
      NameList->GetText(Group,i);
      // сконвертируем Group в короткое имя группы, хранящееся в реестре
      ConvertGroupName(Group,GRP_TOSHORTNAME);

      if (((Conf.ViewShell) && (OpenFrom==OPEN_PLUGINSMENU) &&
          ((CmpStr(Group,"Viewer")==0) || (CmpStr(Group,"Editor")==0))) ||
          ((Conf.ViewViewer) && (OpenFrom==OPEN_VIEWER) &&
           (CmpStr(Group,"Viewer")!=0)) ||
          ((Conf.ViewEditor) && (OpenFrom==OPEN_EDITOR) &&
           (CmpStr(Group,"Editor")!=0)))
        continue;
      wsprintf(S,"%s\\%s",KeyMacros,Group);
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

      for (j=MenuItemsNumber;j<MacNameList->GetCount();j++,k++)
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
          if ((CmpStr(Str,Macro->Group)==0) && (CmpStr((Key[0]=='~' && lstrlen(Key)>1)?&Key[1]:Key,Macro->Key)==0))
            ListPos.SelectPos=k;

        lstrcpy(Str,NameList->GetText(i));  // Str contains group name
        lstrcpy(Key,MacNameList->GetText(j)); // Key contains key name
        if (Conf.GroupDivider)
        {
          // Добавление разделителя групп.
          if (*OldGroup && CmpStr(OldGroup,Str)!=0)
          {
            MenuList->Add("");
            GroupList->Add("");
            KeyList->Add("");
            if (Conf.AddDescription)
              DescrList->Add("");
            AddCount++;
            k++;
          }
          lstrcpy(OldGroup,Str);
        }

        if (Conf.AddDescription)
        {
          wsprintf(S,"%s\\%s\\%s",KeyMacros,Group,Key);
          CheckFirstBackSlash(S,TRUE);
          if (!Reg->OpenKey(S))
            continue;
          Reg->ReadString("Description",S,sizeof(S));
          if (S[0])
          {
            DescrList->Add(S);
            emptyDescr=FALSE;
          }
          else
            DescrList->Add("");
          Reg->CloseKey();
        }

        CheckLen(Str,KeyWidth);
        lstrcat(Str,":");
        FSF.sprintf(S,"%-*s  \"%s\"",KeyWidth+1,Str,(Key[0]=='~' && lstrlen(Key)>1)?&Key[1]:Key);
        CheckLen(S,(csbi.dwSize.X-12>MAXMENULEN) ? MAXMENULEN : csbi.dwSize.X-12);
        if (Key[0]=='~' && lstrlen(Key)>1)
        {
          Str[0]='~';
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
        FSF.sprintf(S,"%*s\xB3%*s",KeyWidth," ",KeyWidth," ");
      else
        FSF.sprintf(S,"%*s",KeyWidth," ");

      FarListItem ListItems; //=new FarListItem[1];
      List.ItemsNumber=1;
      List.Items=&ListItems;

      lstrcpy(ListItems.Text,S);
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

      for (i=0;i<MenuItemsNumber;i++)
      {
        if (Conf.AddDescription && !emptyDescr)
        {
          lstrcpy(Str,MenuList->GetText(i));
          if (*Str)
          {
            FSF.sprintf(S,"%-*s \xB3 %s",GroupKeyLen,(Str[0]=='~')?&Str[1]:Str,DescrList->GetText(i));
            if (Str[0]=='~')
            {
              memmove(&S[1],S,sizeof(S)-1);
              S[0]='~';
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
          ListItems/*[i]*/.Flags=(S[0]=='~')?LIF_CHECKED|'-':0;
          if (S[0]=='~')
            memmove(S,&S[1],sizeof(S)-1);
          CheckLen(S,(csbi.dwSize.X-12>MAXMENULEN)?MAXMENULEN:csbi.dwSize.X-12);
          lstrcpy(ListItems/*[i]*/.Text,S);

          lstrcpyn(MData.Group,GroupList->GetText(i),sizeof(MData.Group)-1);
          MData.Group[sizeof(MData.Group)-1]=0;
          lstrcpyn(MData.Key,KeyList->GetText(i),sizeof(MData.Key)-1);
          MData.Key[sizeof(MData.Key)-1]=0;
        }
        else
        {
          ListItems/*[i]*/.Flags=LIF_SEPARATOR;
          ListItems/*[i]*/.Text[0]=0;
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

  MenuW=(MaxMenuItemLen+8>csbi.dwSize.X)?csbi.dwSize.X-10:MaxMenuItemLen+8;
  MenuH=(MenuItemsNumber+10>csbi.dwSize.Y)?csbi.dwSize.Y-6:MenuItemsNumber+4;
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
    MenuX=(csbi.dwSize.X-MenuW)/2;
    MenuY=(csbi.dwSize.Y-MenuH)/2;
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
  Rect.Left=(MenuW-lstrlen(MenuTitle))/2;
  Rect.Top=1;
  Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,1,(LONG_PTR)&Rect);
  Rect.Left=(MenuW-lstrlen(MenuBottom))/2;
  Rect.Top=MenuH-2;
  Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,2,(LONG_PTR)&Rect);
}


void TMacroView::WriteRegValues(FarDialogItem *DialogItems)
{
  if (MacroData)
  {
    if (MultiLine)
    {
        //BUGBUG Reg->PutData("Sequence",MacroData,,rdMultiString);
    }
    else
    {
        Reg->WriteString("Sequence",MacroData);
    }
  }
  else
  {
    Reg->WriteString("Sequence",DialogItems[8].Data.Data);
  }

  Reg->WriteString("Sequence",(MacroData)?MacroData:DialogItems[8].Data.Data);
  if (DialogItems[10].Data.Data[0])
    Reg->WriteString("Description",DialogItems[10].Data.Data);
  else
    Reg->DeleteValue("Description");

  if (DialogItems[12].Param.Selected)
    Reg->WriteInteger("RunAfterFARStart",DialogItems[12].Param.Selected);
  else
    Reg->DeleteValue("RunAfterFARStart");

  if (DialogItems[13].Param.Selected)
    Reg->WriteInteger("DisableOutput",DialogItems[13].Param.Selected);
  else
    Reg->DeleteValue("DisableOutput");

  switch(DialogItems[14].Param.Selected)
  {
    case 2:
      Reg->DeleteValue("NotEmptyCommandLine");
      Reg->DeleteValue("EmptyCommandLine");
      break;
    case 1:
      Reg->DeleteValue("EmptyCommandLine");
      Reg->WriteInteger("NotEmptyCommandLine",1);
      break;
    case 0:
      Reg->DeleteValue("NotEmptyCommandLine");
      Reg->WriteInteger("EmptyCommandLine",1);
      break;
  }

  switch(DialogItems[15].Param.Selected)
  {
    case 2:
      Reg->DeleteValue("NoEVSelection");
      Reg->DeleteValue("EVSelection");
      break;
    case 1:
      Reg->DeleteValue("NoEVSelection");
      Reg->WriteInteger("EVSelection",1);
      break;
    case 0:
      Reg->DeleteValue("EVSelection");
      Reg->WriteInteger("NoEVSelection",1);
      break;
  }

  // Флаги активной панели
  if (DialogItems[16].Param.Selected)
  {
    switch(DialogItems[18].Param.Selected)
    {
      case 2:
        Reg->DeleteValue("NoPluginPanels");
        Reg->DeleteValue("NoFilePanels");
        break;
      case 1:
        Reg->DeleteValue("NoPluginPanels");
        Reg->WriteInteger("NoFilePanels",1);
        break;
      case 0:
        Reg->DeleteValue("NoFilePanels");
        Reg->WriteInteger("NoPluginPanels",1);
        break;
    }

    switch(DialogItems[20].Param.Selected)
    {
      case 2:
        Reg->DeleteValue("NoFolders");
        Reg->DeleteValue("NoFiles");
        break;
      case 1:
        Reg->DeleteValue("NoFolders");
        Reg->WriteInteger("NoFiles",1);
        break;
      case 0:
        Reg->DeleteValue("NoFiles");
        Reg->WriteInteger("NoFolders",1);
        break;
    }

    switch(DialogItems[22].Param.Selected)
    {
      case 2:
        Reg->DeleteValue("NoSelection");
        Reg->DeleteValue("Selection");
        break;
      case 1:
        Reg->DeleteValue("NoSelection");
        Reg->WriteInteger("Selection",1);
        break;
      case 0:
        Reg->DeleteValue("Selection");
        Reg->WriteInteger("NoSelection",1);
        break;
    }
  }
  else
  {
    Reg->DeleteValue("NoPluginPanels");
    Reg->DeleteValue("NoFilePanels");

    Reg->DeleteValue("NoFolders");
    Reg->DeleteValue("NoFiles");

    Reg->DeleteValue("NoSelection");
    Reg->DeleteValue("Selection");
  }

  // Флаги пассивной панели
  if (DialogItems[17].Param.Selected)
  {
    switch(DialogItems[19].Param.Selected)
    {
      case 2:
        Reg->DeleteValue("NoPluginPPanels");
        Reg->DeleteValue("NoFilePPanels");
        break;
      case 1:
        Reg->DeleteValue("NoPluginPPanels");
        Reg->WriteInteger("NoFilePPanels",1);
        break;
      case 0:
        Reg->DeleteValue("NoFilePPanels");
        Reg->WriteInteger("NoPluginPPanels",1);
        break;
    }

    switch(DialogItems[21].Param.Selected)
    {
      case 2:
        Reg->DeleteValue("NoPFolders");
        Reg->DeleteValue("NoPFiles");
        break;
      case 1:
        Reg->DeleteValue("NoPFolders");
        Reg->WriteInteger("NoPFiles",1);
        break;
      case 0:
        Reg->DeleteValue("NoPFiles");
        Reg->WriteInteger("NoPFolders",1);
        break;
    }

    switch(DialogItems[23].Param.Selected)
    {
      case 2:
        Reg->DeleteValue("NoPSelection");
        Reg->DeleteValue("PSelection");
        break;
      case 1:
        Reg->DeleteValue("NoPSelection");
        Reg->WriteInteger("PSelection",1);
        break;
      case 0:
        Reg->DeleteValue("PSelection");
        Reg->WriteInteger("NoPSelection",1);
        break;
    }
  }
  else
  {
    Reg->DeleteValue("NoPluginPPanels");
    Reg->DeleteValue("NoFilePPanels");

    Reg->DeleteValue("NoPFolders");
    Reg->DeleteValue("NoPFiles");

    Reg->DeleteValue("NoPSelection");
    Reg->DeleteValue("PSelection");
  }

  if (DialogItems[26].Param.Selected)
    Reg->DeleteValue("NoSendKeysToPlugins");
  else
    Reg->WriteInteger("NoSendKeysToPlugins",1);
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
  char lGroup[MAX_KEY_LEN]; //длинное название текущего раздела макроса

  char *ItemsSave1[]=
  {
    GetMsg(MMacroExport),GetMsg(MMacroWhere),S,
    "\x1",
    GetMsg(MMacroInsertEditor),GetMsg(MMacroSaveToFile),GetMsg(MMacroCancel),
  };
  char *ItemsSave2[]=
  {
    GetMsg(MMacroExport),GetMsg(MMacroAllWhere),
    "\x1",
    GetMsg(MMacroInsertEditor),GetMsg(MMacroSaveToFile),GetMsg(MMacroCancel),
  };

  if (MenuItemsNumber!=0)
  {
    ActiveMode=MAC_EXPORTACTIVE;

    switch(OpenFrom)
    {
      case OPEN_EDITOR:
        lstrcpy(lGroup,Group);
        //Из короткого имени группы создадим длинное
        ConvertGroupName(lGroup,GRP_TOLONGNAME);
        wsprintf(S,"%s: %s",lGroup,Key);
        QuoteText(S,TRUE);

        if (AllMacros)
        {
          eCode=Info.Message(Info.ModuleNumber,0,NULL,ItemsSave2,
            sizeof(ItemsSave2)/sizeof(ItemsSave2[0]),3);
        }
        else
        {
          eCode=Info.Message(Info.ModuleNumber,0,NULL,ItemsSave1,
            sizeof(ItemsSave1)/sizeof(ItemsSave1[0]),3);
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

  char Button[BUTTONLEN];
  char lGroup[MAX_KEY_LEN]; //длинное название текущего раздела макроса

  if (Deactivated)
    wsprintf(Str,GetMsg(MMacroWarningDeleteThisKey),GetMsg(MMacroWarningRest));
  else
    wsprintf(Str,GetMsg(MMacroWarningDeleteThisKey),GetMsg(MMacroWarningDel));
  char *ItemsDel[]=
  {
    GetMsg(MMacroWarningDelete),Str,
    S,
    "\x1",
    GetMsg(MMacroDelete),Button,GetMsg(MMacroCancel)
  };

  if (MenuItemsNumber!=0)
  {
    lstrcpy(lGroup,Group);
    //Из короткого имени группы создадим длинное
    ConvertGroupName(lGroup,GRP_TOLONGNAME);
    wsprintf(S,"%s: %s",lGroup,Key);
    QuoteText(S,TRUE);
    if (Deactivated)
      lstrcpy(Button,GetMsg(MMacroTmpRest));
    else
      lstrcpy(Button,GetMsg(MMacroTmpDel));

    WriteKeyBar(KB_DIALOG);
    ActiveMode=MAC_DELETEACTIVE;

    lResult=DeletingMacro(ItemsDel,sizeof(ItemsDel)/sizeof(ItemsDel[0]),"MacroDel")!=DM_NONE;

    *Group=*Key=0;
  }

  return lResult;
}


int __fastcall TMacroView::ShowEdit()
{
  EditX1=(csbi.dwSize.X-DIALOGWID)/2;
  EditY1=(csbi.dwSize.Y-1-DIALOGHGT)/2;
  EditX2=EditX1+DIALOGWID-1;
  EditY2=EditY1+DIALOGHGT-1;

  return Info.DialogEx(Info.ModuleNumber,EditX1,EditY1,EditX2,EditY2,"MacroParams",EditDialog,
                     sizeof(EditDialog)/sizeof(EditDialog[0]),0,0,
                     MacroDialogProc,0);
}


BOOL __fastcall TMacroView::InsertMacro()
{
  BOOL RetVal=FALSE;

  char lGroup[MAX_KEY_LEN]; //длинное название текущего раздела макроса

  char *ItemsInsEmp[]=
  {
    GetMsg(MMacroWarning),GetMsg(MMacroWarningInsertEmpty1),
    GetMsg(MMacroWarningInsertEmpty2),
    "\x1",
    GetMsg(MMacroOk),
  };
  char *ItemsEdit[]=
  {
    GetMsg(MMacroWarningEdit),GetMsg(MMacroWarningEditThisKey),
    "\x1",
    GetMsg(MMacroOk),GetMsg(MMacroCancel)
  };
  char *ItemsExist[]=
  {
    GetMsg(MMacroWarning),GetMsg(MMacroWarningExist),
    S,
    "\x1",
    GetMsg(MMacroOverwrite),GetMsg(MMacroCancel)
  };

  // Проинициализируем диалог!
  InitDialogs();

  // Инициализируем новый макрос как НЕ многострочный
  MultiLine=FALSE;

  EditDialog[2].Data.Data[0]=0;
  EditDialog[2].Focus=TRUE;
  EditDialog[4].Data.Data[0]=0;
  if (MacroData)
  {
    EditDialog[8].Data.Ptr.PtrFlags=0;
    EditDialog[8].Data.Ptr.PtrLength=DATASIZE;
    EditDialog[8].Data.Ptr.PtrData=MacroData;
    *MacroData=0;
  }
  else
    EditDialog[8].Data.Data[0]=0;
  EditDialog[10].Data.Data[0]=0;
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
    lstrcpy(EditDialog[4].Data.Data,GroupItems[GroupIndex[OpenFrom]].Text);
  }
  Deactivated=FALSE;

INSERT_RETRY:

  // сконвертируем из короткого имени группы длинное,
  // для работы в диалоге
  ConvertGroupName(EditDialog[4].Data.Data,GRP_TOLONGNAME);

  WriteKeyBar(KB_DIALOG);
  ActiveMode=MAC_EDITACTIVE;
  EditMode=EM_INSERT;

  int OutCode=ShowEdit();

  if (OutCode==30) // кнопка [Сохранить]
  {
    // конвертируем из длинного имени группы короткое,
    // для записи в реестр
    ConvertGroupName(EditDialog[4].Data.Data,GRP_TOSHORTNAME);

    if ((EditDialog[2].Data.Data[0]!=0) && ((MacroData)?*MacroData!=0:EditDialog[8].Data.Data[0]!=0))
    {
      lstrcpy(lGroup,EditDialog[4].Data.Data);
      //Из короткого имени группы создадим длинное
      ConvertGroupName(lGroup,GRP_TOLONGNAME);
      if ((Conf.AutomaticSave) || ((Info.Message(Info.ModuleNumber,0,NULL,ItemsEdit,
        sizeof(ItemsEdit)/sizeof(ItemsEdit[0]),2))==0))
      {
        char *S1=new char[MAX_KEY_LEN];

        wsprintf(Str,"%s\\%s\\%s",KeyMacros,EditDialog[4].Data.Data,
          EditDialog[2].Data.Data);
        wsprintf(S1,"%s\\%s\\~%s",KeyMacros,EditDialog[4].Data.Data,
          EditDialog[2].Data.Data);

        CheckFirstBackSlash(Str,TRUE);
        CheckFirstBackSlash(S1,TRUE);

        if ((Reg->KeyExists(Str)) || (Reg->KeyExists(S1)))
        {
          int OldActive=ActiveMode;
          ActiveMode=MAC_ERRORACTIVE;
          wsprintf(S,"\"%s: %s\"",lGroup,EditDialog[2].Data.Data);
          if (Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsExist,
              sizeof(ItemsExist)/sizeof(ItemsExist[0]),2)==0)
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
        WriteRegValues(EditDialog);
        Reg->CloseKey();

        SwitchOver(EditDialog[4].Data.Data,EditDialog[2].Data.Data);

        delete[] S1;
      }
    }
    else
    {
      int OldActive=ActiveMode;
      ActiveMode=MAC_ERRORACTIVE;
      Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsInsEmp,
        sizeof(ItemsInsEmp)/sizeof(ItemsInsEmp[0]),1);
      ActiveMode=OldActive;
      goto INSERT_RETRY;
    }

    // Копируем  в переменные Key и Group последние значения
    // для перехода потом на новое значение
    lstrcpy(Key,EditDialog[2].Data.Data);
    lstrcpy(Group,EditDialog[4].Data.Data);
    Deactivated=EditDialog[28].Param.Selected;

    RetVal=TRUE;
  }

  return RetVal;
}


BOOL __fastcall TMacroView::EditMacro()
{
  BOOL RetVal=FALSE;

  char Button[BUTTONLEN];
  char lGroup[MAX_KEY_LEN]; //длинное название текущего раздела макроса
  int i;

  char *ItemsError[]=
  {
    GetMsg(MMacroError),GetMsg(MMacroErrorRead),
    S,
    "\x1",
    GetMsg(MMacroOk),
  };
  char *ItemsErrorData[]=
  {
    GetMsg(MMacroError),GetMsg(MMacroErrorData),
    S,
    "\x1",
    GetMsg(MMacroOk),
  };
  char *ItemsDelEmp[]=
  {
    GetMsg(MMacroWarningDelete),GetMsg(MMacroWarningDeleteKey),
    S,
    "\x1",
    GetMsg(MMacroDelete),Button,GetMsg(MMacroCancel)
  };
  char *ItemsEdit[]=
  {
    GetMsg(MMacroWarningEdit),GetMsg(MMacroWarningEditThisKey),
    "\x1",
    GetMsg(MMacroOk),GetMsg(MMacroCancel)
  };
  char *ItemsExist[]=
  {
    GetMsg(MMacroWarning),GetMsg(MMacroWarningExist),
    S,
    "\x1",
    GetMsg(MMacroOverwrite),GetMsg(MMacroCancel)
  };
  char *ItemsErrorWrite[]=
  {
    GetMsg(MMacroError),GetMsg(MMacroErrorWrite),
    S,
    "\x1",
    GetMsg(MMacroOk),
  };
  char *ItemsEditEmp[]=
  {
    GetMsg(MMacroWarning),GetMsg(MMacroWarningInsertEmpty0),
    GetMsg(MMacroWarningInsertEmpty2),
    "\x1",
    GetMsg(MMacroOk),
  };

  if (MenuItemsNumber==0)
    return RetVal;

  // Проинициализируем диалог!
  InitDialogs();

  // Инициализируем новый макрос как НЕ многострочный
  MultiLine=FALSE;

  lstrcpy(EditDialog[2].Data.Data,Key);
  lstrcpy(EditDialog[4].Data.Data,Group);
  if (MacroData)
  {
    EditDialog[8].Data.Ptr.PtrFlags=0;
    EditDialog[8].Data.Ptr.PtrLength=DATASIZE;
    EditDialog[8].Data.Ptr.PtrData=MacroData;
    *MacroData=0;
  }
  else
    EditDialog[8].Data.Data[0]=0;
  EditDialog[8].Focus=TRUE;
  EditDialog[10].Data.Data[0]=0;
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
    wsprintf(S,"%s\\%s\\~%s",KeyMacros,Group,Key);
  else
    wsprintf(S,"%s\\%s\\%s",KeyMacros,Group,Key);

  CheckFirstBackSlash(S,TRUE);
  if (!Reg->OpenKey(S))
  {
    int OldActive=ActiveMode;
    ActiveMode=MAC_ERRORACTIVE;
    QuoteText(S,TRUE);
    Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsError,
      sizeof(ItemsError)/sizeof(ItemsError[0]),1);
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
      Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsErrorData,
        sizeof(ItemsErrorData)/sizeof(ItemsErrorData[0]),1);
      *Group=*Key=0;
      ActiveMode=OldActive;
    }
    else
    {
      for (i=0;i<ValueList->GetCount();i++)
      {
        lstrcpy(S,ValueList->GetText(i));
        if (CmpStr(S,"Sequence")==0) // Sequence
        {
          if (MacroData)
      {
      // Получим тип данных макропоследовательности
      TRegDataType Type;
      int nSize=Reg->GetData(S,MacroData,DATASIZE,Type);
      if (nSize>0)
      {
        if (Type==rdMultiString)
          // Макрос в реестре с типом многострочный
          MultiLine=TRUE;
      }
      else
        MacroData[0]=0;

      /*if (Type==rdMultiString)
      {
      }
      else
        Reg->ReadString(S,MacroData,DATASIZE);*/
      }
          else
          {
            Reg->ReadString(S,Str,sizeof(Str));
            if (Str[0])
              lstrcpy(EditDialog[8].Data.Data,Str);
          }
        }
        else if (CmpStr(S,"Description")==0) // Description
        {
          Reg->ReadString(S,Str,sizeof(Str));
          if (Str[0])
            lstrcpy(EditDialog[10].Data.Data,Str);
        }
        else if (CmpStr(S,"RunAfterFARStart")==0) // RunAfterFARStart
        {
          int result=Reg->ReadInteger(S);
          if (result>0)
            EditDialog[12].Param.Selected=result;
        }
        else if (CmpStr(S,"DisableOutput")==0) // DisableOutput
        {
          int result=Reg->ReadInteger(S);
          if (result>0)
            EditDialog[13].Param.Selected=result;
        }
        else if (CmpStr(S,"EmptyCommandLine")==0) // EmptyCommandLine
        {
          int result=Reg->ReadInteger(S);
          if (result>0)
            EditDialog[14].Param.Selected=0;
        }
        else if (CmpStr(S,"NotEmptyCommandLine")==0) // NotEmptyCommandLine
        {
          int result=Reg->ReadInteger(S);
          if (result>0)
            EditDialog[14].Param.Selected=1;
        }
        else if (CmpStr(S,"NoEVSelection")==0) // NoEVSelection
        {
          int result=Reg->ReadInteger(S);
          if (result>0)
            EditDialog[15].Param.Selected=0;
        }
        else if (CmpStr(S,"EVSelection")==0) // EVSelection
        {
          int result=Reg->ReadInteger(S);
          if (result>0)
            EditDialog[15].Param.Selected=1;
        }

        // Флаги активной панели
        else if (CmpStr(S,"NoPluginPanels")==0) // NoPluginPanels
        {
          int result=Reg->ReadInteger(S);
          if (result>0)
            EditDialog[18].Param.Selected=0;
        }
        else if (CmpStr(S,"NoFilePanels")==0) // NoFilePanels
        {
          int result=Reg->ReadInteger(S);
          if (result>0)
            EditDialog[18].Param.Selected=1;
        }
        else if (CmpStr(S,"NoFolders")==0) // NoFolders
        {
          int result=Reg->ReadInteger(S);
          if (result>0)
            EditDialog[20].Param.Selected=0;
        }
        else if (CmpStr(S,"NoFiles")==0) // NoFiles
        {
          int result=Reg->ReadInteger(S);
          if (result>0)
            EditDialog[20].Param.Selected=1;
        }
        else if (CmpStr(S,"NoSelection")==0) // NoSelection
        {
          int result=Reg->ReadInteger(S);
          if (result>0)
            EditDialog[22].Param.Selected=0;
        }
        else if (CmpStr(S,"Selection")==0) // Selection
        {
          int result=Reg->ReadInteger(S);
          if (result>0)
            EditDialog[22].Param.Selected=1;
        }

        // Флаги пассивной панели
        else if (CmpStr(S,"NoPluginPPanels")==0) // NoPluginPPanels
        {
          int result=Reg->ReadInteger(S);
          if (result>0)
            EditDialog[19].Param.Selected=0;
        }
        else if (CmpStr(S,"NoFilePPanels")==0) // NoFilePPanels
        {
          int result=Reg->ReadInteger(S);
          if (result>0)
            EditDialog[19].Param.Selected=1;
        }
        else if (CmpStr(S,"NoPFolders")==0) // NoPFolders
        {
          int result=Reg->ReadInteger(S);
          if (result>0)
            EditDialog[21].Param.Selected=0;
        }
        else if (CmpStr(S,"NoPFiles")==0) // NoPFiles
        {
          int result=Reg->ReadInteger(S);
          if (result>0)
            EditDialog[21].Param.Selected=1;
        }
        else if (CmpStr(S,"NoPSelection")==0) // NoPSelection
        {
          int result=Reg->ReadInteger(S);
          if (result>0)
            EditDialog[23].Param.Selected=0;
        }
        else if (CmpStr(S,"PSelection")==0) // PSelection
        {
          int result=Reg->ReadInteger(S);
          if (result>0)
            EditDialog[23].Param.Selected=1;
        }

        else if (CmpStr(S,"NoSendKeysToPlugins")==0) // NoSendKeysToPlugins
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

EDIT_RETRY:

      // сконвертируем из короткого имени группы длинное,
      // для работы в диалоге
      ConvertGroupName(EditDialog[4].Data.Data,GRP_TOLONGNAME);

      WriteKeyBar(KB_DIALOG);
      ActiveMode=MAC_EDITACTIVE;
      EditMode=EM_EDIT;

      int OutCode=ShowEdit();

      if (OutCode==30) // кнопка [Сохранить]
      {
        // конвертируем из длинного имени группы короткое,
        // для записи в реестр
        ConvertGroupName(EditDialog[4].Data.Data,GRP_TOSHORTNAME);

        BOOL deleted=FALSE;
        if ((MacroData)?*MacroData==0:EditDialog[8].Data.Data[0]==0)
        {
          lstrcpy(lGroup,Group);
          //Из короткого имени группы создадим длинное
          ConvertGroupName(lGroup,GRP_TOLONGNAME);
          wsprintf(S,"%s: %s",lGroup,Key);
          QuoteText(S,TRUE);
          if (Deactivated)
            lstrcpy(Button,GetMsg(MMacroTmpRest));
          else
            lstrcpy(Button,GetMsg(MMacroTmpDel));

          if (DeletingMacro(ItemsDelEmp,sizeof(ItemsDelEmp)/sizeof(ItemsDelEmp[0]),"MacroDelEmpty")==DM_DELETED)
            deleted=TRUE;

        }

        if (!deleted)
        {
          if (EditDialog[2].Data.Data[0]!=0)
          {
            lstrcpy(lGroup,EditDialog[4].Data.Data);
            //Из короткого имени группы создадим длинное
            ConvertGroupName(lGroup,GRP_TOLONGNAME);
            if ((Conf.AutomaticSave) || ((Info.Message(Info.ModuleNumber,0,NULL,ItemsEdit,
                 sizeof(ItemsEdit)/sizeof(ItemsEdit[0]),2))==0))
            {
              // Если изменилась группа или команда
              // переместим сначала макрокоманду на новое место
              if ((CmpStr(EditDialog[2].Data.Data,Key)!=0) ||
                  (CmpStr(EditDialog[4].Data.Data,Group)!=0))
              {
                char *S1=new char[MAX_KEY_LEN];

                wsprintf(Str,"%s\\%s\\%s",KeyMacros,EditDialog[4].Data.Data,
                  EditDialog[2].Data.Data);
                wsprintf(S1,"%s\\%s\\~%s",KeyMacros,EditDialog[4].Data.Data,
                  EditDialog[2].Data.Data);

                CheckFirstBackSlash(Str,TRUE);
                CheckFirstBackSlash(S1,TRUE);

                if ((Reg->KeyExists(Str)) || (Reg->KeyExists(S1)))
                {
                  wsprintf(S,"\"%s: %s\"",lGroup,EditDialog[2].Data.Data);
                  int OldActive=ActiveMode;
                  ActiveMode=MAC_ERRORACTIVE;
                  if (Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsExist,
                      sizeof(ItemsExist)/sizeof(ItemsExist[0]),2)==0)
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
                  wsprintf(S1,"%s\\%s\\~%s",KeyMacros,Group,Key); //OldName
                  wsprintf(S,"%s\\%s\\~%s",KeyMacros,EditDialog[4].Data.Data,
                    EditDialog[2].Data.Data); //NewName
                }
                else
                {
                  wsprintf(S1,"%s\\%s\\%s",KeyMacros,Group,Key); //OldName
                  wsprintf(S,"%s\\%s\\%s",KeyMacros,EditDialog[4].Data.Data,
                    EditDialog[2].Data.Data); //NewName
                }

                CheckFirstBackSlash(S,TRUE);
                CheckFirstBackSlash(S1,TRUE);
                Reg->MoveKey(S1,S);
                delete[] S1;
              }
              // Теперь сохраняем параметры макрокоманды
              if (Deactivated)
                wsprintf(S,"%s\\%s\\~%s",KeyMacros,EditDialog[4].Data.Data,
                  EditDialog[2].Data.Data);
              else
                wsprintf(S,"%s\\%s\\%s",KeyMacros,EditDialog[4].Data.Data,
                  EditDialog[2].Data.Data);

              CheckFirstBackSlash(S,TRUE);
              if (!Reg->OpenKey(S))
              {
                int OldActive=ActiveMode;
                ActiveMode=MAC_ERRORACTIVE;
                QuoteText(S,TRUE);
                Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsErrorWrite,
                  sizeof(ItemsErrorWrite)/sizeof(ItemsErrorWrite[0]),1);
                ActiveMode=OldActive;
              }
              else
              {
                WriteRegValues(EditDialog);
                Reg->CloseKey();

                SwitchOver(EditDialog[4].Data.Data,EditDialog[2].Data.Data);

                RetVal=TRUE;
              }
            }
          }
          else
          {
            int OldActive=ActiveMode;
            ActiveMode=MAC_ERRORACTIVE;
            Info.Message(Info.ModuleNumber,FMSG_WARNING,"MacroGroups",ItemsEditEmp,
              sizeof(ItemsEditEmp)/sizeof(ItemsEditEmp[0]),1);
            ActiveMode=OldActive;
            goto EDIT_RETRY;
          }
        }
        // Копируем  в переменные Key и Group последние значения
        // для перехода потом на новое значение
        lstrcpy(Key,EditDialog[2].Data.Data);
        lstrcpy(Group,EditDialog[4].Data.Data);
        Deactivated=EditDialog[28].Param.Selected;
      }
    }
    Reg->CloseKey();
  }

  return RetVal;
}


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
  GetConsoleTitle(OldCaption,sizeof(OldCaption));

  MenuW=60;
  MenuH=14;
//  MenuX=MenuY=-1;

  ActiveMode=MAC_MENUACTIVE;

  InitDialogItem InitItems[]=
  {

/*0*/  {DI_LISTBOX,   2,1,MenuW-3,MenuH-2,1,0,((Conf.UseHighlight)?DIF_LISTAUTOHIGHLIGHT:0)|((Conf.MenuCycle)?DIF_LISTWRAPMODE:0),0,""},
/*1*/  {DI_TEXT,      1000,1000,0,0,0,0,0,0,MenuTitle},

  };

  InitDialogItems(InitItems,MenuDialog,sizeof(InitItems)/sizeof(InitItems[0]));

  Info.DialogEx(Info.ModuleNumber,MenuX,MenuY,MenuW,MenuH,"MacroView",
           MenuDialog,sizeof(MenuDialog)/sizeof(MenuDialog[0]),0,0,
           MenuDialogProc,0);

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

  if (AreFileApisANSI())
    SetFileApisToOEM();

  akm.Command=MCMD_LOADALL;
  ZeroMemory(akm.Param.Reserved,sizeof(akm.Param.Reserved));
  Info.AdvControl(Info.ModuleNumber,ACTL_KEYMACRO,&akm);
  return 0;
}
