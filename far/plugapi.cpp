/*
plugapi.cpp

API, доступное плагинам (диалоги, меню, ...)

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#ifndef __FARCONST_HPP__
#include "farconst.hpp"
#endif
#ifndef __KEYS_HPP__
#include "keys.hpp"
#endif
#ifndef __FARLANG_HPP__
#include "lang.hpp"
#endif
#ifndef __COLOROS_HPP__
#include "colors.hpp"
#endif
#ifndef __FARSTRUCT_HPP__
#include "struct.hpp"
#endif
#ifndef __PLUGIN_HPP__
#include "plugin.hpp"
#endif
#ifndef __CLASSES_HPP__
#include "classes.hpp"
#endif
#ifndef __FARFUNC_HPP__
#include "fn.hpp"
#endif
#ifndef __FARGLOBAL_HPP__
#include "global.hpp"
#endif


// declare in plugins.cpp
extern int KeepUserScreen;
extern char DirToSet[NM];


void ScanPluginDir();

int WINAPI FarMenuFn(int PluginNumber,int X,int Y,int MaxHeight,
           unsigned int Flags,char *Title,char *Bottom,char *HelpTopic,
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
    unsigned int MenuFlags=0;
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


int WINAPI FarDialogFn(int PluginNumber,int X1,int Y1,int X2,int Y2,
           char *HelpTopic,struct FarDialogItem *Item,int ItemsNumber)
{
  if (DisablePluginsOutput)
    return(-1);
  struct DialogItem *InternalItem=new DialogItem[ItemsNumber];

  int ExitCode;

  for (int I=0;I<ItemsNumber;I++)
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
    Dialog FarDialog(InternalItem,ItemsNumber);
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
    FarDialog.Process();
    ExitCode=FarDialog.GetExitCode();
  }

  for (int I=0;I<ItemsNumber;I++)
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
  delete InternalItem;
//  CheckScreenLock();
  return(ExitCode);
}


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


int WINAPI FarMessageFn(int PluginNumber,unsigned int Flags,char *HelpTopic,
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
  int MsgCode=Message(Flags,ButtonsNumber,Items[0],MsgItems[0],MsgItems[1],
              MsgItems[2],MsgItems[3],MsgItems[4],MsgItems[5],MsgItems[6],
              MsgItems[7],MsgItems[8],MsgItems[9],MsgItems[10],MsgItems[11],
              MsgItems[12],MsgItems[13]);
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
  ConvertNameToFull(Dir,DirName);
  ScTree.SetFindPath(DirName,"*.*");
  *PointToName(DirName)=0;
  int DirLength=strlen(DirName);
  while (ScTree.GetNextName(&FindData,FullName))
  {
    if ((ItemsNumber & 31)==0)
    {
      if (CheckForEsc())
      {
        delete ItemsList;
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
  PluginPanelItem *PanelData=NULL;
  int ItemCount=0;
  if (CheckForEsc())
    StopSearch=TRUE;

  char DirName[NM];
  strncpy(DirName,PluginSearchPath,sizeof(DirName));
  DirName[sizeof(DirName)-1]=0;
  for (int I=0;DirName[I]!=0;I++)
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
  for (int I=0;I<ItemCount && !StopSearch;I++)
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
        DestItem->UserData=(DWORD)new char[Size];
        memcpy((void *)DestItem->UserData,(void *)CurPanelItem->UserData,Size);
      }

      strncpy(DestItem->FindData.cFileName,FullName,sizeof(DestItem->FindData.cFileName)-1);
      DirListItemsNumber++;
    }
  }
  for (int I=0;I<ItemCount && !StopSearch;I++)
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
  for (int I=0;I<sizeof(DirListNumbers)/sizeof(DirListNumbers[0]);I++)
    if (DirListNumbers[I].Addr==PanelItem)
    {
      DirListNumbers[I].Addr=NULL;
      ItemsNumber=DirListNumbers[I].ItemsNumber;
      break;
    }

  for (int I=0;I<ItemsNumber;I++)
  {
    PluginPanelItem *CurPanelItem=PanelItem+I;
    if (CurPanelItem->UserData && (CurPanelItem->Flags & PPIF_USERDATA))
      delete (void *)CurPanelItem->UserData;
  }

  delete PanelItem;
}


#pragma warn -par
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
  FileEditor Editor(FileName,FALSE,FALSE,StartLine,StartChar,Title,X1,Y1,X2,Y2);
  SetConsoleTitle(OldTitle);
  return(Editor.GetExitCode());
}
#pragma warn +par


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
    GotoXY(X,Y);
    SetColor(Color);
    Text(Str);
  }
}


int WINAPI FarEditorControl(int Command,void *Param)
{
  if (CtrlObject->Plugins.CurEditor==NULL)
    return(0);
  return(CtrlObject->Plugins.CurEditor->EditorControl(Command,Param));
}
