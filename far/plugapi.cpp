/*
plugapi.cpp

API, доступное плагинам (диалоги, меню, ...)

*/

/* Revision: 1.23 02.11.2000 $ */

/*
Modify:
  02.11.2000 OT
   ! Введение проверки на длину буфера, отведенного под имя файла.
  05.10.2000 SVS
   - бага с вызовом хелпа (FHELP_CUSTOMFILE)
  27.09.2000 SVS
   + FarViewerControl
  18.09.2000 SVS
    ! Функция FarDialogEx имеет 2 дополнительных параметра (Future)
  12.09.2000 SVS
    + Реализация флагов FHELP_* для вывода помощи.
  08.09.2000 VVM
    + Обработка команд
      FCTL_SETSORTMODE, FCTL_SETANOTHERSORTMODE
      FCTL_SETSORTORDER, FCTL_SETANOTHERSORTORDER
  30.08.2000 SVS
    ! Пал смертью храбрых флаг FMI_GETFARMSGID
  29.08.2000 SVS
    + Для диалога запомним номер плагина, вызвавшего этот диалог. Сейчас
      это для того, чтобы правильно отреагировать в Dialog API на DN_HELP
  29.08.2000 SVS
    ! Если PluginStartupInfo.GetMsg(?,N|FMI_GETFARMSGID), то подразумеваем, что
      хотим использовать "месаги" из САМОГО far*.lng
  24.08.2000 SVS
    + ACTL_WAITKEY - ожидать определенную (или любую) клавишу
  23.08.2000 SVS
    ! Все Flags приведены к одному виду -> DWORD.
      Модифицированы:
        * функции   FarMenuFn, FarMessageFn, FarShowHelp
        * структуры FarListItem, FarDialogItem
  22.08.2000 SVS
    ! Исключаем ненужные вызовы из FarText.
  18.08.2000 tran 1.12
    + Flags parameter in FarShowHelp
  09.08.2000 tran 1.11
    ! ACTL_GETSYSWORDDIV при Param==NULL просто возвращает длину строки
  03.08.2000 SVS
    + ACTL_GETSYSWORDDIV получить строку с символами разделителями слов
  01.08.2000 SVS
    ! FARDIALOGPROC -> FARWINDOWPROC
  28.07.2000 SVS
    ! В связи с появлением SendDlgMessage в классе Dialog
      вносим некоторые изменения!
  25.07.2000 SVS
    + Программое переключение FulScreen <-> Windowed (ACTL_CONSOLEMODE)
  23.07.2000 SVS
    + Функция FarDefDlgProc обработки диалога по умолчанию
    + Функция FarSendDlgMessage - посылка сообщения диалогу
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  12.07.2000 IS
    + Проверка флагов редактора в FarEditor (раньше они игнорировались) и
      открытие _немодального_ редактора, если есть соответствующий флаг.
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  05.07.2000 IS
    + Функция FarAdvControl
    + Команда ACTL_GETFARVERSION (в FarAdvControl)
  03.07.2000 IS
    + Функция вывода помощи
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

// declare in plugins.cpp
extern int KeepUserScreen;
extern char DirToSet[NM];


void ScanPluginDir();

/* $ 12.09.2000 SVS
  + Реализация флагов для вывода помощи.
*/
/* $ 18.08.2000 tran
   + Flags parameter */
/* $ 03.07.2000 IS
  Функция вывода помощи
*/
BOOL WINAPI FarShowHelp(char *ModuleName, char *HelpTopic,DWORD Flags)
{
  if (!HelpTopic)
    HelpTopic="Contents";

  char Path[2*NM],Topic[512];
  char *Mask=NULL;

  if((Flags&FHELP_FARHELP) || *HelpTopic==':')
    strcpy(Topic,HelpTopic+((Flags&FHELP_FARHELP)?0:1));
  else
  {
    if(ModuleName)
    {
      // FHELP_SELFHELP=0 - трактовать первый пар-р как Info.ModuleName
      //                   и показать топик из хелпа вызвавшего плагина
      if(Flags&(FHELP_SELFHELP|FHELP_CUSTOMFILE|FHELP_CUSTOMPATH))
      {
        strcpy(Path,ModuleName);
        if(Flags&(FHELP_SELFHELP|FHELP_CUSTOMFILE))
        {
          Mask=PointToName(Path);
          if(Flags&FHELP_CUSTOMFILE)
          {
            memmove(Mask+1,Mask,strlen(Mask)+1);
            *Mask++=0;
          }
          else
          {
            *Mask=0;
            Mask=NULL;
          }
        }
      }
      else
        return FALSE;

      sprintf(Topic,"#%s#%s",Path,HelpTopic);
    }
    else
      return FALSE;
  }
  {
    Help Hlp(Topic,Mask);
    if(Hlp.GetError())
      return FALSE;
  }
  return TRUE;
}
/* IS $ */
/* tran 18.08.2000 $ */
/* SVS 12.09.2000 $ */


/* $ 05.07.2000 IS
  Функция, которая будет действовать и в редакторе, и в панелях, и...
*/
int WINAPI FarAdvControl(int ModuleNumber, int Command, void *Param)
{
 switch(Command)
 {
    case ACTL_GETFARVERSION:
      *(DWORD*)Param=FAR_VERSION;
      return TRUE;
    /* $ 25.07.2000 SVS
       + Программое переключение FulScreen <-> Windowed (ACTL_CONSOLEMODE)
       mode = -2 - получить текущее состояние
              -1 - как тригер
               0 - Windowed
               1 - FulScreen
       Return
               0 - Windowed
               1 - FulScreen
    */
    case ACTL_CONSOLEMODE:
      return FarAltEnter(*(int*)Param);
    /* SVS $ */

    /* $ 03.08.2000 SVS
       получение строки с разделителями слов
       Возвращает размер полученных данных без '\0'
       Максимальный размер приемного буфера = 80 с заключительным '\0'
       Строка выбирается не из реестра, а из Opt.
    */
    case ACTL_GETSYSWORDDIV:
      /* $ 09.08.2000 tran
       + if param==NULL, plugin хочет только узнать длину строки  */
      if ( Param )
          strcpy((char *)Param,Opt.WordDiv);
      /* tran 09.08.2000 $ */
      return strlen(Opt.WordDiv);
    /* SVS $ */

    /* $ 24.08.2000 SVS
       ожидать определенную (или любую) клавишу
       (int)Param - внутренний код клавиши, которую ожидаем, или -1
       если все равно какую клавишу ждать.
       возвращает 0;
    */
    case ACTL_WAITKEY:
      WaitKey((int)Param);
      return 0;
    /* SVS $ */

 }
 return FALSE;
}
/* IS $ */

int WINAPI FarMenuFn(int PluginNumber,int X,int Y,int MaxHeight,
           DWORD Flags,char *Title,char *Bottom,char *HelpTopic,
           int *BreakKeys,int *BreakCode,struct FarMenuItem *Item,
           int ItemsNumber)
{
  if (DisablePluginsOutput)
    return(-1);
  int ExitCode;
  {
    VMenu FarMenu(Title,NULL,0,MaxHeight);
    FarMenu.SetPosition(X,Y,0,0);
    if (BreakCode!=NULL)
      *BreakCode=-1;
    if (HelpTopic!=NULL)
    {
      char Path[NM],Topic[512];
      if (*HelpTopic==':')
        strcpy(Topic,HelpTopic+1);
      else
      {
        strcpy(Path,CtrlObject->Plugins.PluginsData[PluginNumber].ModuleName);
        *PointToName(Path)=0;
        sprintf(Topic,"#%s#%s",Path,HelpTopic);
      }
      FarMenu.SetHelp(Topic);
    }
    if (Bottom!=NULL)
      FarMenu.SetBottomTitle(Bottom);
    for (int I=0;I<ItemsNumber;I++)
    {
      struct MenuItem CurItem;
      CurItem.Selected=Item[I].Selected;
      CurItem.Checked=Item[I].Checked;
      CurItem.Separator=Item[I].Separator;
      *CurItem.UserData=CurItem.UserDataSize=0;
      strncpy(CurItem.Name,Item[I].Text,sizeof(CurItem.Name));
      FarMenu.AddItem(&CurItem);
    }

    DWORD MenuFlags=0;
    if (Flags & FMENU_SHOWAMPERSAND)
      MenuFlags|=MENU_SHOWAMPERSAND;
    if (Flags & FMENU_WRAPMODE)
      MenuFlags|=FMENU_WRAPMODE;
    if (Flags & FMENU_AUTOHIGHLIGHT)
      FarMenu.AssignHighlights(FALSE);
    if (Flags & FMENU_REVERSEAUTOHIGHLIGHT)
      FarMenu.AssignHighlights(TRUE);
    FarMenu.SetFlags(MenuFlags);
    FarMenu.Show();
    while (!FarMenu.Done())
    {
      INPUT_RECORD ReadRec;
      int ReadKey=GetInputRecord(&ReadRec);
      if (ReadRec.EventType==MOUSE_EVENT)
        FarMenu.ProcessMouse(&ReadRec.Event.MouseEvent);
      else
        if (ReadKey!=KEY_NONE)
        {
          if (BreakKeys!=NULL)
            for (int I=0;BreakKeys[I]!=0;I++)
              if (ReadRec.Event.KeyEvent.wVirtualKeyCode==(BreakKeys[I] & 0xffff))
              {
                DWORD Flags=BreakKeys[I]>>16;
                DWORD RealFlags=ReadRec.Event.KeyEvent.dwControlKeyState;
                int Accept=TRUE;
                if ((Flags & PKF_CONTROL) && (RealFlags & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED))==0)
                  Accept=FALSE;
                if ((Flags & PKF_ALT) && (RealFlags & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))==0)
                  Accept=FALSE;
                if ((Flags & PKF_SHIFT) && (RealFlags & SHIFT_PRESSED)==0)
                  Accept=FALSE;
                if (Accept)
                {
                  if (BreakCode!=NULL)
                    *BreakCode=I;
                  FarMenu.Hide();
//                  CheckScreenLock();
                  return(FarMenu.GetSelectPos());
                }
              }
          FarMenu.ProcessKey(ReadKey);
        }
    }
    ExitCode=FarMenu.GetExitCode();
  }
//  CheckScreenLock();
  return(ExitCode);
}

/* $ 23.07.2000 SVS
   Функции для расширенного диалога
*/
// Функция FarDefDlgProc обработки диалога по умолчанию
long WINAPI FarDefDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  if(hDlg)  // исключаем лишний вызов для hDlg=0
    return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
  return 0;
}

/* $ 28.07.2000 SVS
    ! В связи с появлением SendDlgMessage в классе Dialog
      вносим некоторые изменения!
*/
// Посылка сообщения диалогу
long WINAPI FarSendDlgMessage(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  if(hDlg) // исключаем лишний вызов для hDlg=0
    return Dialog::SendDlgMessage(hDlg,Msg,Param1,Param2);
  return 0;
}
/* SVS $ */

int WINAPI FarDialogFn(int PluginNumber,int X1,int Y1,int X2,int Y2,
           char *HelpTopic,struct FarDialogItem *Item,int ItemsNumber)
{
  return FarDialogEx(PluginNumber,X1,Y1,X2,Y2,HelpTopic,Item,ItemsNumber,NULL,NULL,NULL,NULL);
}

int WINAPI FarDialogEx(int PluginNumber,int X1,int Y1,int X2,int Y2,
           char *HelpTopic,struct FarDialogItem *Item,int ItemsNumber,
           DWORD Reserved, DWORD Flags,
           FARWINDOWPROC DlgProc,long Param)

{
  if (DisablePluginsOutput)
    return(-1);
  struct DialogItem *InternalItem=new DialogItem[ItemsNumber];

  int ExitCode,I;

  for (I=0;I<ItemsNumber;I++)
  {
    InternalItem[I].Type=Item[I].Type;
    InternalItem[I].X1=Item[I].X1;
    InternalItem[I].Y1=Item[I].Y1;
    InternalItem[I].X2=Item[I].X2;
    InternalItem[I].Y2=Item[I].Y2;
    InternalItem[I].Focus=Item[I].Focus;
    InternalItem[I].Selected=Item[I].Selected;
    InternalItem[I].Flags=Item[I].Flags;
    InternalItem[I].DefaultButton=Item[I].DefaultButton;
    strcpy(InternalItem[I].Data,Item[I].Data);
    InternalItem[I].ObjPtr=NULL;
  }

  {
    Dialog FarDialog(InternalItem,ItemsNumber,DlgProc,Param);
    FarDialog.SetPosition(X1,Y1,X2,Y2);
    if (HelpTopic!=NULL)
    {
      char Path[NM],Topic[512];
      if (*HelpTopic==':')
        strcpy(Topic,HelpTopic+1);
      else
      {
        strcpy(Path,CtrlObject->Plugins.PluginsData[PluginNumber].ModuleName);
        *PointToName(Path)=0;
        sprintf(Topic,"#%s#%s",Path,HelpTopic);
      }
      FarDialog.SetHelp(Topic);
    }
    /* $ 29.08.2000 SVS
       Запомним номер плагина - сейчас в основном для формирования HelpTopic
    */
    FarDialog.SetPluginNumber(PluginNumber);
    /* SVS $ */
    FarDialog.Process();
    ExitCode=FarDialog.GetExitCode();
  }

  for (I=0;I<ItemsNumber;I++)
  {
    Item[I].Type=InternalItem[I].Type;
    Item[I].X1=InternalItem[I].X1;
    Item[I].Y1=InternalItem[I].Y1;
    Item[I].X2=InternalItem[I].X2;
    Item[I].Y2=InternalItem[I].Y2;
    Item[I].Focus=InternalItem[I].Focus;
    Item[I].Selected=InternalItem[I].Selected;
    Item[I].Flags=InternalItem[I].Flags;
    Item[I].DefaultButton=InternalItem[I].DefaultButton;
    strcpy(Item[I].Data,InternalItem[I].Data);
  }
  /* $ 13.07.2000 SVS
     для new[] нужен delete[]
  */
  delete[] InternalItem;
  /* SVS $*/
//  CheckScreenLock();
  return(ExitCode);
}
/* SVS $ */

char* WINAPI FarGetMsgFn(int PluginNumber,int MsgId)
{
  return(CtrlObject->Plugins.FarGetMsg(PluginNumber,MsgId));
}

char* PluginsSet::FarGetMsg(int PluginNumber,int MsgId)
{
  if (PluginNumber<PluginsCount)
  {
    struct PluginItem *CurPlugin=&PluginsData[PluginNumber];
    char Path[NM];
    strcpy(Path,CurPlugin->ModuleName);
    *PointToName(Path)=0;
    if (CurPlugin->Lang.Init(Path))
      return(CurPlugin->Lang.GetMsg(MsgId));
  }
  return("");
}


int WINAPI FarMessageFn(int PluginNumber,DWORD Flags,char *HelpTopic,
                        char **Items,int ItemsNumber,int ButtonsNumber)
{
  if (DisablePluginsOutput)
    return(-1);
  if (ItemsNumber<2)
    return(-1);
  char *MsgItems[14];
  memset(MsgItems,0,sizeof(MsgItems));
  for (int I=1;I<ItemsNumber;I++)
    MsgItems[I-1]=Items[I];
  if (HelpTopic!=NULL)
  {
    char Path[NM],Topic[512];
    if (*HelpTopic==':')
      strcpy(Topic,HelpTopic+1);
    else
    {
      strcpy(Path,CtrlObject->Plugins.PluginsData[PluginNumber].ModuleName);
      *PointToName(Path)=0;
      sprintf(Topic,"#%s#%s",Path,HelpTopic);
    }
    SetMessageHelp(Topic);
  }
  /* $ 29.08.2000 SVS
     Запомним номер плагина - сейчас в основном для формирования HelpTopic
  */
  int MsgCode=Message(Flags,ButtonsNumber,Items[0],MsgItems[0],MsgItems[1],
              MsgItems[2],MsgItems[3],MsgItems[4],MsgItems[5],MsgItems[6],
              MsgItems[7],MsgItems[8],MsgItems[9],MsgItems[10],MsgItems[11],
              MsgItems[12],MsgItems[13],PluginNumber);
  /* SVS $ */
//  CheckScreenLock();
  return(MsgCode);
}


int WINAPI FarControl(HANDLE hPlugin,int Command,void *Param)
{
  if (CtrlObject->LeftPanel==NULL || CtrlObject->RightPanel==NULL)
    return(0);

  switch(Command)
  {
    case FCTL_CLOSEPLUGIN:
      strcpy(DirToSet,NullToEmpty((char *)Param));
    case FCTL_GETPANELINFO:
    case FCTL_GETANOTHERPANELINFO:
    case FCTL_UPDATEPANEL:
    case FCTL_UPDATEANOTHERPANEL:
    case FCTL_REDRAWPANEL:
    case FCTL_REDRAWANOTHERPANEL:
    case FCTL_SETPANELDIR:
    case FCTL_SETANOTHERPANELDIR:
    case FCTL_SETSELECTION:
    case FCTL_SETANOTHERSELECTION:
    case FCTL_SETVIEWMODE:
    case FCTL_SETANOTHERVIEWMODE:
/* $ VVM 08.09.2000
   + Смена сортировки из плагина
*/
    case FCTL_SETSORTMODE:
    case FCTL_SETANOTHERSORTMODE:
    case FCTL_SETSORTORDER:
    case FCTL_SETANOTHERSORTORDER:
/* VVM $ */
      {
        if (hPlugin==INVALID_HANDLE_VALUE)
        {
          CtrlObject->ActivePanel->SetPluginCommand(Command,Param);
          return(TRUE);
        }
        HANDLE hInternal;
        Panel *LeftPanel=CtrlObject->LeftPanel;
        Panel *RightPanel=CtrlObject->RightPanel;
        int Processed=FALSE;
        if (LeftPanel!=NULL && LeftPanel->GetMode()==PLUGIN_PANEL)
        {
          hInternal=((struct PluginHandle *)LeftPanel->GetPluginHandle())->InternalHandle;
          if (hPlugin==hInternal)
          {
            LeftPanel->SetPluginCommand(Command,Param);
            Processed=TRUE;
          }
        }
        if (RightPanel!=NULL && RightPanel->GetMode()==PLUGIN_PANEL)
        {
          hInternal=((struct PluginHandle *)RightPanel->GetPluginHandle())->InternalHandle;
          if (hPlugin==hInternal)
          {
            RightPanel->SetPluginCommand(Command,Param);
            Processed=TRUE;
          }
        }
        return(Processed);
      }
    case FCTL_GETCMDLINE:
      CtrlObject->CmdLine.GetString((char *)Param,1024);
      return(TRUE);
    case FCTL_SETCMDLINE:
    case FCTL_INSERTCMDLINE:
      if (Command==FCTL_SETCMDLINE)
        CtrlObject->CmdLine.SetString((char *)Param);
      else
        CtrlObject->CmdLine.InsertString((char *)Param);
      CtrlObject->CmdLine.Redraw();
      return(TRUE);
    case FCTL_SETCMDLINEPOS:
      CtrlObject->CmdLine.SetCurPos(*(int *)Param);
      CtrlObject->CmdLine.Redraw();
      return(TRUE);
    case FCTL_GETCMDLINEPOS:
      *(int *)Param=CtrlObject->CmdLine.GetCurPos();
      return(TRUE);
    case FCTL_SETUSERSCREEN:
      if (CtrlObject->LeftPanel==NULL || CtrlObject->RightPanel==NULL)
        return(FALSE);
      KeepUserScreen=TRUE;
      CtrlObject->LeftPanel->ProcessingPluginCommand++;
      CtrlObject->RightPanel->ProcessingPluginCommand++;
      ScrBuf.FillBuf();
      SaveScreen SaveScr;
      {
        RedrawDesktop Redraw;
        CtrlObject->CmdLine.Hide();
        SaveScr.RestoreArea(FALSE);
      }
      CtrlObject->LeftPanel->ProcessingPluginCommand--;
      CtrlObject->RightPanel->ProcessingPluginCommand--;
      return(TRUE);
  }
  return(FALSE);
}


HANDLE WINAPI FarSaveScreen(int X1,int Y1,int X2,int Y2)
{
  if (X2==-1)
    X2=ScrX;
  if (Y2==-1)
    Y2=ScrY;
  return((HANDLE)(new SaveScreen(X1,Y1,X2,Y2,FALSE)));
}


void WINAPI FarRestoreScreen(HANDLE hScreen)
{
  if (hScreen==NULL)
    ScrBuf.FillBuf();
  delete (SaveScreen *)hScreen;
}


int WINAPI FarGetDirList(char *Dir,struct PluginPanelItem **pPanelItem,
                  int *pItemsNumber)
{
  PluginPanelItem *ItemsList=NULL;
  int ItemsNumber=0;
  SaveScreen SaveScr;
  clock_t StartTime=clock();
  int MsgOut=0;

  *pItemsNumber=0;
  *pPanelItem=NULL;

  ScanTree ScTree(FALSE);
  WIN32_FIND_DATA FindData;
  char FullName[NM],DirName[NM];
  ConvertNameToFull(Dir,DirName, sizeof(DirName));
  ScTree.SetFindPath(DirName,"*.*");
  *PointToName(DirName)=0;
  int DirLength=strlen(DirName);
  while (ScTree.GetNextName(&FindData,FullName))
  {
    if ((ItemsNumber & 31)==0)
    {
      if (CheckForEsc())
      {
        /* $ 13.07.2000 SVS
           Запросы были через realloc, потому и free
        */
        if(ItemsList) free(ItemsList);
        /* SVS $ */
        return(FALSE);
      }
      if (!MsgOut && clock()-StartTime > 500)
      {
        SetCursorType(FALSE,0);
        Message(MSG_DOWN,0,"",MSG(MPreparingList));
        MsgOut=1;
      }
      ItemsList=(PluginPanelItem *)realloc(ItemsList,sizeof(*ItemsList)*(ItemsNumber+32+1));
      if (ItemsList==NULL)
      {
        *pItemsNumber=0;
        return(FALSE);
      }
    }
    memset(&ItemsList[ItemsNumber],0,sizeof(*ItemsList));
    ItemsList[ItemsNumber].FindData=FindData;
    strcpy(ItemsList[ItemsNumber].FindData.cFileName,FullName+DirLength);
    ItemsNumber++;
  }
  *pPanelItem=ItemsList;
  *pItemsNumber=ItemsNumber;
  return(TRUE);
}


static struct PluginPanelItem *PluginDirList;
static int DirListItemsNumber;
static char PluginSearchPath[NM*16];
static int StopSearch;
static HANDLE hDirListPlugin;
static int PluginSearchMsgOut;
static struct
{
  PluginPanelItem *Addr;
  int ItemsNumber;
} DirListNumbers[16];

int WINAPI FarGetPluginDirList(int PluginNumber,HANDLE hPlugin,
                  char *Dir,struct PluginPanelItem **pPanelItem,
                  int *pItemsNumber)
{
  {
    if (strcmp(Dir,".")==0 || strcmp(Dir,"..")==0)
      return(FALSE);
    SaveScreen SaveScr;

    {
      char DirName[512];
      strcpy(DirName,Dir);
      TruncStr(DirName,30);
      CenterStr(DirName,DirName,30);
      SetCursorType(FALSE,0);
      Message(0,0,"",MSG(MPreparingList),DirName);
      PluginSearchMsgOut=FALSE;

      static struct PluginHandle
      {
        HANDLE InternalHandle;
        int PluginNumber;
      } DirListPlugin;

      DirListPlugin.PluginNumber=PluginNumber;
      DirListPlugin.InternalHandle=hPlugin;
      hDirListPlugin=(HANDLE)&DirListPlugin;
      StopSearch=FALSE;
      *pItemsNumber=DirListItemsNumber=0;
      *pPanelItem=PluginDirList=NULL;
      struct OpenPluginInfo Info;
      CtrlObject->Plugins.GetOpenPluginInfo(hDirListPlugin,&Info);
      char PrevDir[NM];
      strcpy(PrevDir,Info.CurDir);
      if (CtrlObject->Plugins.SetDirectory(hDirListPlugin,Dir,OPM_FIND))
      {
        strcpy(PluginSearchPath,Dir);
        strcat(PluginSearchPath,"\x1");
        ScanPluginDir();
        *pPanelItem=PluginDirList;
        *pItemsNumber=DirListItemsNumber;
        CtrlObject->Plugins.SetDirectory(hDirListPlugin,"..",OPM_FIND);
        PluginPanelItem *PanelData=NULL;
        int ItemCount=0;
        if (CtrlObject->Plugins.GetFindData(hDirListPlugin,&PanelData,&ItemCount,OPM_FIND))
          CtrlObject->Plugins.FreeFindData(hDirListPlugin,PanelData,ItemCount);
        struct OpenPluginInfo NewInfo;
        CtrlObject->Plugins.GetOpenPluginInfo(hDirListPlugin,&NewInfo);
        if (LocalStricmp(PrevDir,NewInfo.CurDir)!=0)
          CtrlObject->Plugins.SetDirectory(hDirListPlugin,PrevDir,OPM_FIND);
      }
    }
  }

  if (!StopSearch)
    for (int I=0;I<sizeof(DirListNumbers)/sizeof(DirListNumbers[0]);I++)
      if (DirListNumbers[I].Addr==NULL)
      {
        DirListNumbers[I].Addr=*pPanelItem;
        DirListNumbers[I].ItemsNumber=*pItemsNumber;
        break;
      }
  return(!StopSearch);
}


void ScanPluginDir()
{
  int I;
  PluginPanelItem *PanelData=NULL;
  int ItemCount=0;
  if (CheckForEsc())
    StopSearch=TRUE;

  char DirName[NM];
  strncpy(DirName,PluginSearchPath,sizeof(DirName));
  DirName[sizeof(DirName)-1]=0;
  for (I=0;DirName[I]!=0;I++)
    if (DirName[I]=='\x1')
      DirName[I]=DirName[I+1]==0 ? 0:'\\';
  TruncStr(DirName,30);
  CenterStr(DirName,DirName,30);

  Message(MSG_KEEPBACKGROUND,0,"",MSG(MPreparingList),DirName);

  if (StopSearch || !CtrlObject->Plugins.GetFindData(hDirListPlugin,&PanelData,&ItemCount,OPM_FIND))
    return;
  struct PluginPanelItem *NewList=(struct PluginPanelItem *)realloc(PluginDirList,1+sizeof(*PluginDirList)*(DirListItemsNumber+ItemCount));
  if (NewList==NULL)
  {
    StopSearch=TRUE;
    return;
  }
  PluginDirList=NewList;
  for (I=0;I<ItemCount && !StopSearch;I++)
  {
    PluginPanelItem *CurPanelItem=PanelData+I;
    if ((CurPanelItem->FindData.dwFileAttributes & FA_DIREC)==0)
    {
      char FullName[2*NM+1];
      sprintf(FullName,"%.*s%.*s",NM,PluginSearchPath,NM,CurPanelItem->FindData.cFileName);
      for (int I=0;FullName[I]!=0;I++)
        if (FullName[I]=='\x1')
          FullName[I]='\\';
      PluginPanelItem *DestItem=PluginDirList+DirListItemsNumber;
      *DestItem=*CurPanelItem;
      if (CurPanelItem->UserData && (CurPanelItem->Flags & PPIF_USERDATA))
      {
        DWORD Size=*(DWORD *)CurPanelItem->UserData;
        /* $ 13.07.2000 SVS
           вместо new будем использовать malloc
        */
        DestItem->UserData=(DWORD)malloc(Size);
        /* SVS $*/
        memcpy((void *)DestItem->UserData,(void *)CurPanelItem->UserData,Size);
      }

      strncpy(DestItem->FindData.cFileName,FullName,sizeof(DestItem->FindData.cFileName)-1);
      DirListItemsNumber++;
    }
  }
  for (I=0;I<ItemCount && !StopSearch;I++)
  {
    PluginPanelItem *CurPanelItem=PanelData+I;
    if ((CurPanelItem->FindData.dwFileAttributes & FA_DIREC) &&
        strcmp(CurPanelItem->FindData.cFileName,".")!=0 &&
        strcmp(CurPanelItem->FindData.cFileName,"..")!=0)

    {
      struct PluginPanelItem *NewList=(struct PluginPanelItem *)realloc(PluginDirList,sizeof(*PluginDirList)*(DirListItemsNumber+1));
      if (NewList==NULL)
      {
        StopSearch=TRUE;
        return;
      }
      PluginDirList=NewList;
      char FullName[2*NM+1];
      sprintf(FullName,"%.*s%.*s",NM,PluginSearchPath,NM,CurPanelItem->FindData.cFileName);
      for (int I=0;FullName[I]!=0;I++)
        if (FullName[I]=='\x1')
          FullName[I]='\\';
      PluginDirList[DirListItemsNumber]=*CurPanelItem;
      strncpy(PluginDirList[DirListItemsNumber].FindData.cFileName,FullName,sizeof(PluginDirList[0].FindData.cFileName)-1);
      DirListItemsNumber++;
      if (CtrlObject->Plugins.SetDirectory(hDirListPlugin,CurPanelItem->FindData.cFileName,OPM_FIND))
      {
        strcat(PluginSearchPath,CurPanelItem->FindData.cFileName);
        strcat(PluginSearchPath,"\x1");
        if (strlen(PluginSearchPath)<sizeof(PluginSearchPath)-NM)
          ScanPluginDir();
        *strrchr(PluginSearchPath,'\x1')=0;
        char *NamePtr=strrchr(PluginSearchPath,'\x1');
        if (NamePtr!=NULL)
          *(NamePtr+1)=0;
        else
          *PluginSearchPath=0;
        if (!CtrlObject->Plugins.SetDirectory(hDirListPlugin,"..",OPM_FIND))
        {
          StopSearch=TRUE;
          break;
        }
      }
    }
  }
  CtrlObject->Plugins.FreeFindData(hDirListPlugin,PanelData,ItemCount);
}


void WINAPI FarFreeDirList(struct PluginPanelItem *PanelItem)
{
  if (PanelItem==NULL)
    return;
  int ItemsNumber=0;
  int I;
  for (I=0;I<sizeof(DirListNumbers)/sizeof(DirListNumbers[0]);I++)
    if (DirListNumbers[I].Addr==PanelItem)
    {
      DirListNumbers[I].Addr=NULL;
      ItemsNumber=DirListNumbers[I].ItemsNumber;
      break;
    }

  for (I=0;I<ItemsNumber;I++)
  {
    PluginPanelItem *CurPanelItem=PanelItem+I;
    if (CurPanelItem->UserData && (CurPanelItem->Flags & PPIF_USERDATA))
      /* $ 13.07.2000 SVS
        для запроса использовали malloc
      */
      free((void *)CurPanelItem->UserData);
      /* SVS $*/
  }
  /* $ 13.07.2000 SVS
    для запроса использовали realloc
  */
  free(PanelItem);
  /* SVS $*/
}


#if defined(__BORLANDC__)
#pragma warn -par
#endif
int WINAPI FarViewer(char *FileName,char *Title,int X1,int Y1,int X2,
                            int Y2,DWORD Flags)
{
  if (X2==-1)
    X2=ScrX;
  if (Y2==-1)
    Y2=ScrY;
  char OldTitle[512];
  GetConsoleTitle(OldTitle,sizeof(OldTitle));
  SaveScreen SaveScr;
  if (Flags & VF_NONMODAL)
  {
    FileViewer *Viewer=new FileViewer(FileName,TRUE,Title,X1,Y1,X2,Y2);
    if (Flags & VF_DELETEONCLOSE)
      Viewer->SetTempViewName(FileName);
    CtrlObject->ModalManager.AddModal(Viewer);
  }
  else
  {
    FileViewer Viewer(FileName,FALSE,Title,X1,Y1,X2,Y2);
    if (Flags & VF_DELETEONCLOSE)
      Viewer.SetTempViewName(FileName);
    SetConsoleTitle(OldTitle);
    return(Viewer.GetExitCode());
  }
  return(TRUE);
}


int WINAPI FarEditor(char *FileName,char *Title,int X1,int Y1,int X2,
                            int Y2,DWORD Flags,int StartLine,int StartChar)
{
  if (X2==-1)
    X2=ScrX;
  if (Y2==-1)
    Y2=ScrY;
  char OldTitle[512];
  GetConsoleTitle(OldTitle,sizeof(OldTitle));
  SaveScreen SaveScr;
  /* $ 12.07.2000 IS
   Проверка флагов редактора (раньше они игнорировались) и открытие
   немодального редактора, если есть соответствующий флаг
  */
  int ExitCode;
  if (Flags & EF_NONMODAL)
  {
   ExitCode=FALSE;
   FileEditor *Editor=new FileEditor(FileName,FALSE,TRUE,StartLine,StartChar,Title,X1,Y1,X2,Y2);
   if(Editor)
     {
      CtrlObject->ModalManager.AddModal(Editor);
      ExitCode=TRUE;
     }
  }
  else
  {
   FileEditor Editor(FileName,FALSE,FALSE,StartLine,StartChar,Title,X1,Y1,X2,Y2);
   SetConsoleTitle(OldTitle);
   return(Editor.GetExitCode());
  }
  return ExitCode;
  /* IS $ */
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif


int WINAPI FarCmpName(char *pattern,char *string,int skippath)
{
  return(CmpName(pattern,string,skippath));
}


int WINAPI FarCharTable(int Command,char *Buffer,int BufferSize)
{
  if (Command==FCT_DETECT)
  {
    char DataFileName[NM];
    FILE *DataFile;
    strcpy(DataFileName,Opt.TempPath);
    strcat(DataFileName,"FarTmpXXXXXX");
    if (mktemp(DataFileName)==NULL || (DataFile=fopen(DataFileName,"w+b"))==NULL)
      return(-1);
    fwrite(Buffer,1,BufferSize,DataFile);
    fseek(DataFile,0,SEEK_SET);
    CharTableSet TableSet;
    int TableNum;
    int DetectCode=DetectTable(DataFile,&TableSet,TableNum);
    fclose(DataFile);
    remove(DataFileName);
    return(DetectCode ? TableNum-1:-1);
  }
  if (BufferSize!=sizeof(CharTableSet))
    return(-1);
  if (!PrepareTable((CharTableSet *)Buffer,Command))
    return(-1);
  return(Command);
}


void WINAPI FarText(int X,int Y,int Color,char *Str)
{
  if (Str==NULL)
  {
    int PrevLockCount=ScrBuf.GetLockCount();
    ScrBuf.SetLockCount(0);
    ScrBuf.Flush();
    ScrBuf.SetLockCount(PrevLockCount);
  }
  else
  {
    /* $ 22.08.2000 SVS
       Исключаем ненужные вызовы из FarText.
    */
    Text(X,Y,Color,Str);
    /* SVS $ */
  }
}


int WINAPI FarEditorControl(int Command,void *Param)
{
  if (CtrlObject->Plugins.CurEditor==NULL)
    return(0);
  return(CtrlObject->Plugins.CurEditor->EditorControl(Command,Param));
}

/* $ 27.09.2000 SVS
  Управление вьювером
*/
int WINAPI FarViewerControl(int Command,void *Param)
{
  if (CtrlObject->Plugins.CurViewer==NULL)
    return(0);
  return(CtrlObject->Plugins.CurViewer->ViewerControl(Command,Param));
}
/* SVS $ */
