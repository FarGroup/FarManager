/*
treelist.cpp

Tree panel

*/

/* Revision: 1.19 07.08.2001 $ */

/*
Modify:
  07.08.2001 SVS
    ! Численное значение дельты (31) заменено на DELTA_TREECOUNT
    ! сделаем внешний вид дерева на 1 символ уже.
    - бага (со времен 1.64) в функции чтения кэша. Из-за этого не работало
      обновление дерева при создании каталога
  23.07.2001 SVS
    + Для полноты картины: Shift-Enter дереве вызывает проводник
  18.06.2001 SVS
    - Вот же елы палы :-( Вместо MACRO_TREEPANEL макромода стояла
      MACRO_INFOPANEL. К чему бы это?
    + F4 в дереве работает так же как и Ctrl-A
  14.06.2001 SVS
    + KEY_CTRLALTINS - вставляет в клипборд полное имя каталога
  25.05.2001 SVS
    + Добавим возможность создавать линк с дерева
  06.05.2001 DJ
    ! перетрях #include
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  25.04.2001 SVS
    ! Добавка для MODALTREE_FREE
  23.04.2001 SVS
    ! Если файл существует, то позаботимся о сохранении оригинальных атрибутов
      Часть вторая, т.с. - в прошлый раз забыл еще в одном месте выставить...
  06.04.2001 SVS
    ! Корректный вызов удаления папки по F8 и Shift-F8
  05.04.2001 VVM
    + Переключение макросов в режим MACRO_TREEPANEL
  26.03.2001 SVS
    ! Если файл существует, то позаботимся о сохранении оригинальных атрибутов
  28.02.2001 IS
    ! "CtrlObject->CmdLine." -> "CtrlObject->CmdLine->"
  27.02.2001 VVM
    ! Символы, зависимые от кодовой страницы
      /[\x01-\x08\x0B-\x0C\x0E-\x1F\xB0-\xDF\xF8-\xFF]/
      переведены в коды.
  09.01.2001 SVS
    - Для KEY_XXX_BASE нужно прибавить 0x01
  02.11.2000 OT
    ! Введение проверки на длину буфера, отведенного под имя файла.
  16.10.2000 tran
    + MustBeCached(Root) - функция, определяющая необходимость кеширования
      дерева
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "treelist.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "plugin.hpp"
#include "global.hpp"
#include "colors.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "filepanels.hpp"
#include "filelist.hpp"
#include "cmdline.hpp"
#include "chgprior.hpp"
#include "scantree.hpp"
#include "copy.hpp"
#include "qview.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"


#define DELTA_TREECOUNT	31
#define TreeFileName "Tree.Far"
#define TreeCacheFolderName "Tree.Cache"

static int _cdecl SortList(const void *el1,const void *el2);
static int _cdecl SortCacheList(const void *el1,const void *el2);
static int StaticSortCaseSensitive;
static int TreeCmp(char *Str1,char *Str2);

static char TreeLineSymbol[4][3]={
  {0x20,0x20,/*0x20,*/0x00},
  {0xB3,0x20,/*0x20,*/0x00},
  {0xC0,0xC4,/*0xC4,*/0x00},
  {0xC3,0xC4,/*0xC4,*/0x00},
};

static struct TreeListCache
{
  char TreeName[NM];
  char *ListName;
  int TreeCount;
} TreeCache;


TreeList::TreeList()
{
  Type=TREE_PANEL;
  ListData=NULL;
  TreeCount=0;
  WorkDir=CurFile=CurTopFile=0;
  GetSelPosition=0;
  *Root=0;
  UpdateRequired=TRUE;
  CaseSensitiveSort=FALSE;
  PrevMacroMode = -1;
}


TreeList::~TreeList()
{
  /* $ 13.07.2000 SVS
     не надо смешивать new/delete с realloc
  */
  free(ListData);
  /* SVS $ */
  FlushCache();
  SetMacroMode(TRUE);
}

void TreeList::SetRootDir(char *NewRootDir)
{
  strcpy(Root,NewRootDir);
  strcpy(CurDir,NewRootDir);
}

void TreeList::DisplayObject()
{
  if (UpdateRequired)
    Update(0);
  Panel *RootPanel=GetRootPanel();
  if (RootPanel->GetType()==FILE_PANEL)
  {
    int RootCaseSensitive=((FileList *)RootPanel)->IsCaseSensitive();
    if (RootCaseSensitive!=CaseSensitiveSort)
    {
      CaseSensitiveSort=RootCaseSensitive;
      StaticSortCaseSensitive=CaseSensitiveSort;
      qsort(ListData,TreeCount,sizeof(*ListData),SortList);
      FillLastData();
      SyncDir();
    }
  }
  DisplayTree(FALSE);
}


void TreeList::DisplayTree(int Fast)
{
  int I,J,K;
  struct TreeItem *CurPtr;
  char Title[100];

  CorrectPosition();
  if (TreeCount>0)
    strcpy(CurDir,ListData[CurFile].Name);
  if (!Fast)
  {
    Box(X1,Y1,X2,Y2,COL_PANELBOX,DOUBLE_BOX);
    DrawSeparator(Y2-2-(ModalMode!=0));
    SetColor((Focus || ModalMode) ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
    sprintf(Title," %s ",ModalMode ? MSG(MFindFolderTitle):MSG(MTreeTitle));
    TruncStr(Title,X2-X1-3);
    if (*Title)
    {
      GotoXY(X1+(X2-X1+1-strlen(Title))/2,Y1);
      Text(Title);
    }
  }
  for (I=Y1+1,J=CurTopFile;I<Y2-2-(ModalMode!=0);I++,J++)
  {
    CurPtr=&ListData[J];
    GotoXY(X1+1,I);
    SetColor(COL_PANELTEXT);
    Text(" ");
    if (J<TreeCount)
    {
      if (J==0)
        DisplayTreeName("\\",J);
      else
      {
        char OutStr[200];
        for (*OutStr=0,K=0;K<CurPtr->Depth-1 && WhereX()+3*K<X2-6;K++)
          if (CurPtr->Last[K])
            strcat(OutStr,TreeLineSymbol[0]);
          else
            strcat(OutStr,TreeLineSymbol[1]);
        if (CurPtr->Last[CurPtr->Depth-1])
          strcat(OutStr,TreeLineSymbol[2]);
        else
          strcat(OutStr,TreeLineSymbol[3]);
        Text(OutStr);
        char *ChPtr=strrchr(CurPtr->Name,'\\');
        if (ChPtr!=NULL)
          DisplayTreeName(ChPtr+1,J);
      }
    }
    SetColor(COL_PANELTEXT);
    if ((K=WhereX())<X2)
      mprintf("%*s",X2-WhereX(),"");
  }
  if (Opt.ShowPanelScrollbar)
  {
    SetColor(COL_PANELSCROLLBAR);
    ScrollBar(X2,Y1+1,Y2-Y1-3,CurFile,TreeCount>1 ? TreeCount-1:TreeCount);
  }
  SetColor(COL_PANELTEXT);
  GotoXY(X1+1,Y2-1);
  if (TreeCount>0)
    mprintf("%-*.*s",X2-X1-1,X2-X1-1,ListData[CurFile].Name);
  else
    mprintf("%*s",X2-X1-1,"");
  if (ModalMode)
  {
    GotoXY(X1+1,Y2-2);
    mprintf("%*s",X2-X1-1,"");
  }

  UpdateViewPanel();
}


void TreeList::DisplayTreeName(char *Name,int Pos)
{
  if (WhereX()>X2-4)
    GotoXY(X2-4,WhereY());
  if (Pos==CurFile)
  {
    GotoXY(WhereX()-1,WhereY());
    if (Focus || ModalMode)
    {
      SetColor((Pos==WorkDir) ? COL_PANELSELECTEDCURSOR:COL_PANELCURSOR);
      mprintf(" %.*s ",X2-WhereX()-3,Name);
    }
    else
    {
      SetColor((Pos==WorkDir) ? COL_PANELSELECTEDTEXT:COL_PANELTEXT);
      mprintf("[%.*s]",X2-WhereX()-3,Name);
    }
  }
  else
  {
    SetColor((Pos==WorkDir) ? COL_PANELSELECTEDTEXT:COL_PANELTEXT);
    mprintf("%.*s",X2-WhereX()-1,Name);
  }
}


void TreeList::Update(int Mode)
{
  if (!EnableUpdate)
    return;
  if (!IsVisible())
  {
    UpdateRequired=TRUE;
    return;
  }
  UpdateRequired=FALSE;
  GetRoot();
  int LastTreeCount=TreeCount;
  if (!ReadTreeFile())
    ReadTree();
  if (TreeCount>0 && ((Mode & UPDATE_KEEP_SELECTION)==0 || LastTreeCount!=TreeCount))
  {
    SyncDir();

    struct TreeItem *CurPtr=ListData+CurFile;
    if (GetFileAttributes(CurPtr->Name)==(DWORD)-1)
    {
      DelTreeName(CurPtr->Name);
      Update(UPDATE_KEEP_SELECTION);
      Show();
    }
  }
}


void TreeList::ReadTree()
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  SaveScreen SaveScr;
  ScanTree ScTree(FALSE);
  WIN32_FIND_DATA fdata;
  char FullName[NM];

  FlushCache();
  GetRoot();

  int RootLength=strlen(Root)-1;
  if (RootLength<0)
    RootLength=0;

  /* $ 13.07.2000 SVS
     не надо смешивать new/delete с realloc
  */
  free(ListData);
  TreeCount=0;
  if ((ListData=(struct TreeItem*)malloc(sizeof(struct TreeItem)))==NULL)
    return;
  /* SVS $ */
  strcpy(ListData->Name,Root);
  if (RootLength>0 && Root[RootLength-1]!=':' && Root[RootLength]=='\\')
    ListData->Name[RootLength]=0;
  TreeCount=1;

  int FirstCall=TRUE;
  ScTree.SetFindPath(Root,"*.*");
  while (ScTree.GetNextName(&fdata,FullName))
  {
    if (!(fdata.dwFileAttributes & FA_DIREC))
      continue;
    if (TreeCount>3 && !MsgReadTree(TreeCount,FirstCall) ||
        (ListData=(struct TreeItem *)realloc(ListData,(TreeCount+1)*sizeof(struct TreeItem)))==0)
    {
      /* $ 13.07.2000 SVS
         не надо смешивать new/delete с realloc
      */
      free(ListData);
      /* SVS $ */
      ListData=NULL;
      TreeCount=0;
      return;
    }
    strcpy(ListData[TreeCount++].Name,FullName);
  }
  StaticSortCaseSensitive=CaseSensitiveSort=FALSE;
  qsort(ListData,TreeCount,sizeof(*ListData),SortList);

  FillLastData();
  SaveTreeFile();
}

#ifdef _MSC_VER
#pragma warning(disable:4018)
#endif
void TreeList::SaveTreeFile()
{
  if (TreeCount<4)
    return;
  char Name[NM];
  FILE *TreeFile;
  long I;
  int RootLength=strlen(Root)-1;
  if (RootLength<0)
    RootLength=0;
  sprintf(Name,"%s%s",Root,TreeFileName);
  // получим и сразу сбросим атрибуты (если получится)
  DWORD FileAttributes=GetFileAttributes(Name);
  if(FileAttributes != -1)
    SetFileAttributes(Name,0);
  if ((TreeFile=fopen(Name,"wb"))==NULL)
  {
    /* $ 16.10.2000 tran
       если диск должен кешироваться, то и пытаться не стоит */
    if (MustBeCached(Root) || (TreeFile=fopen(Name,"wb"))==NULL)
      if (!GetCacheTreeName(Root,Name,TRUE) || (TreeFile=fopen(Name,"wb"))==NULL)
        return;
    /* tran $ */
  }
  for (I=0;I<TreeCount;I++)
    if (RootLength>=strlen(ListData[I].Name))
      fprintf(TreeFile,"\\\n");
    else
      fprintf(TreeFile,"%s\n",ListData[I].Name+RootLength);
  if (fclose(TreeFile)==EOF)
  {
    clearerr(TreeFile);
    fclose(TreeFile);
    remove(Name);
    Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotSaveTree),Name,MSG(MOk));
  }
  else if(FileAttributes != -1) // вернем атрибуты (если получится :-)
    SetFileAttributes(Name,FileAttributes);
}


int TreeList::GetCacheTreeName(char *Root,char *Name,int CreateDir)
{
  char VolumeName[NM],FileSystemName[NM];
  DWORD MaxNameLength,FileSystemFlags,VolumeNumber;
  if (!GetVolumeInformation(Root,VolumeName,sizeof(VolumeName),&VolumeNumber,
                            &MaxNameLength,&FileSystemFlags,
                            FileSystemName,sizeof(FileSystemName)))
    return(FALSE);
  char FolderName[NM];
  sprintf(FolderName,"%s%s",FarPath,TreeCacheFolderName);
  if (CreateDir)
  {
    mkdir(FolderName);
    SetFileAttributes(FolderName,FA_HIDDEN);
  }
  char RemoteName[NM];
  *RemoteName=0;
  if (*Root=='\\')
    strcpy(RemoteName,Root);
  else
  {
    char LocalName[NM];
    sprintf(LocalName,"%c:",*Root);
    DWORD RemoteNameSize=sizeof(RemoteName);
    WNetGetConnection(LocalName,RemoteName,&RemoteNameSize);
    if (*RemoteName)
      AddEndSlash(RemoteName);
  }
  for (int I=0;RemoteName[I]!=0;I++)
    if (RemoteName[I]=='\\')
      RemoteName[I]='_';
  sprintf(Name,"%s\\%s.%x.%s.%s",FolderName,VolumeName,VolumeNumber,
          FileSystemName,RemoteName);
  return(TRUE);
}


void TreeList::GetRoot()
{
  char PanelDir[NM];
  Panel *RootPanel=GetRootPanel();
  RootPanel->GetCurDir(PanelDir);
  GetPathRoot(PanelDir,Root);
}


Panel* TreeList::GetRootPanel()
{
  Panel *RootPanel;
  if (ModalMode)
  {
    if (ModalMode==MODALTREE_ACTIVE)
      RootPanel=CtrlObject->Cp()->ActivePanel;
    else if (ModalMode==MODALTREE_FREE)
      RootPanel=this;
    else
    {
      RootPanel=CtrlObject->Cp()->GetAnotherPanel(CtrlObject->Cp()->ActivePanel);
      if (!RootPanel->IsVisible())
        RootPanel=CtrlObject->Cp()->ActivePanel;
    }
  }
  else
    RootPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  return(RootPanel);
}


void TreeList::SyncDir()
{
  char PanelDir[NM];
  Panel *AnotherPanel=GetRootPanel();
  AnotherPanel->GetCurDir(PanelDir);
  if (*PanelDir)
    if (AnotherPanel->GetType()==FILE_PANEL)
    {
      if (!SetDirPosition(PanelDir))
      {
        ReadSubTree(PanelDir);
        ReadTreeFile();
        SetDirPosition(PanelDir);
      }
    }
    else
      SetDirPosition(PanelDir);
}


int TreeList::MsgReadTree(int TreeCount,int &FirstCall)
{
  char NumStr[100];
  sprintf(NumStr,"%d",TreeCount);
  Message(FirstCall ? 0:MSG_KEEPBACKGROUND,0,MSG(MTreeTitle),
          MSG(MReadingTree),NumStr);
  FirstCall=FALSE;
  if (CheckForEsc())
    return(0);
  return(1);
}


void TreeList::FillLastData()
{
  long Last,Depth,PathLength,SubDirPos,I,J;
  int RootLength=strlen(Root)-1;
  if (RootLength<0)
    RootLength=0;
  for (I=1;I<TreeCount;I++)
  {
    PathLength=strrchr(ListData[I].Name,'\\')-ListData[I].Name+1;
    Depth=ListData[I].Depth=CountSlash(ListData[I].Name+RootLength);
    for (J=I+1,SubDirPos=I,Last=1;J<TreeCount;J++)
      if (CountSlash(ListData[J].Name+RootLength)>Depth)
      {
        SubDirPos=J;
        continue;
      }
      else
      {
        if (LocalStrnicmp(ListData[I].Name,ListData[J].Name,PathLength)==0)
          Last=0;
        break;
      }
    for (J=I;J<=SubDirPos;J++)
      ListData[J].Last[Depth-1]=Last;
  }
}


int TreeList::CountSlash(char *Str)
{
  int Count=0;
  while ((Str=strchr(Str,'\\'))!=NULL)
  {
    Str++;
    Count++;
  }
  return(Count);
}


int TreeList::ProcessKey(int Key)
{
  struct TreeItem *CurPtr;

  if (!IsVisible())
    return(FALSE);
  if (TreeCount==0 && Key!=KEY_CTRLR)
    return(FALSE);
  {
    char ShortcutFolder[NM],PluginModule[NM],PluginFile[NM],PluginData[8192];
    if (SaveFolderShortcut(Key,CurDir,"","",""))
      return(TRUE);
    if (GetShortcutFolder(Key,ShortcutFolder,PluginModule,PluginFile,PluginData))
    {
      Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
      if (AnotherPanel->GetType()==FILE_PANEL)
      {
        AnotherPanel->SetCurDir(ShortcutFolder,TRUE);
        AnotherPanel->Redraw();
      }
      else
      {
        SetCurDir(ShortcutFolder,TRUE);
        ProcessKey(KEY_ENTER);
      }
      return(TRUE);
    }
  }

  switch(Key)
  {
    case KEY_SHIFTENTER:
    case KEY_CTRLENTER:
    case KEY_CTRLF:
    case KEY_CTRLALTINS:
      {
        char QuotedName[NM+2];
        CurPtr=ListData+CurFile;
        if (strchr(CurPtr->Name,' ')!=NULL)
          sprintf(QuotedName,"\"%s\"%s",CurPtr->Name,(Key == KEY_CTRLALTINS?"":" "));
        else
          sprintf(QuotedName,"%s%s",CurPtr->Name,(Key == KEY_CTRLALTINS?"":" "));
        if(Key == KEY_CTRLALTINS)
          CopyToClipboard(QuotedName);
        else if(Key == KEY_SHIFTENTER)
          Execute(QuotedName,FALSE,TRUE,TRUE);
        else
          CtrlObject->CmdLine->InsertString(QuotedName);
      }
      return(TRUE);
    case KEY_CTRLBACKSLASH:
      CurFile=0;
      ProcessEnter();
      return(TRUE);
    case KEY_ENTER:
      if (!ModalMode && CtrlObject->CmdLine->GetLength()>0)
        break;
      ProcessEnter();
      return(TRUE);
    case KEY_F4:
    case KEY_CTRLA:
      if (SetCurPath())
        ShellSetFileAttributes(this);
      return(TRUE);
    case KEY_CTRLR:
      ReadTree();
      if (TreeCount>0)
        SyncDir();
      Redraw();
      break;
    case KEY_SHIFTF5:
    case KEY_SHIFTF6:
      if (SetCurPath())
      {
        int ToPlugin=0;
        ShellCopy ShCopy(this,Key==KEY_SHIFTF6,FALSE,TRUE,TRUE,ToPlugin,NULL);
      }
      return(TRUE);
    case KEY_F5:
    case KEY_DRAGCOPY:
    case KEY_F6:
    case KEY_ALTF6:
    case KEY_DRAGMOVE:
      if (SetCurPath() && TreeCount>0)
      {
        Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
        int Ask=(Key!=KEY_DRAGCOPY && Key!=KEY_DRAGMOVE || Opt.Confirm.Drag);
        int Move=(Key==KEY_F6 || Key==KEY_DRAGMOVE);
        int ToPlugin=AnotherPanel->GetMode()==PLUGIN_PANEL &&
                     AnotherPanel->IsVisible() &&
                     !CtrlObject->Plugins.UseFarCommand(AnotherPanel->GetPluginHandle(),PLUGIN_FARPUTFILES);
        int Link=(Key==KEY_ALTF6 && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5 && !ToPlugin);
        if(Key==KEY_ALTF6 && !Link) // молча отвалим :-)
          return TRUE;

        ShellCopy ShCopy(this,Move,Link,FALSE,Ask,ToPlugin,NULL);
        if (ToPlugin==1)
        {
          struct PluginPanelItem *ItemList=new PluginPanelItem[1];
          int ItemNumber=1;
          HANDLE hAnotherPlugin=AnotherPanel->GetPluginHandle();
          FileList::FileNameToPluginItem(ListData[CurFile].Name,ItemList);
          if (CtrlObject->Plugins.PutFiles(hAnotherPlugin,ItemList,ItemNumber,Move,0)==1)
            AnotherPanel->SetPluginModified();
          /* $ 13.07.2000 SVS
             не надо смешивать new/delete с realloc
          */
          free(ItemList);
          /* SVS $ */
          if (Move)
            ReadSubTree(ListData[CurFile].Name);
          Update(0);
          Redraw();
          AnotherPanel->Update(UPDATE_KEEP_SELECTION);
          AnotherPanel->Redraw();
        }
      }
      return(TRUE);
    case KEY_F7:
      if (SetCurPath())
        ShellMakeDir(this);
      return(TRUE);
    /*
      Удаление                                   Shift-Del, Shift-F8, F8

      Удаление файлов и папок. F8 и Shift-Del удаляют все выбранные
     файлы, Shift-F8 - только файл под курсором. Shift-Del всегда удаляет
     файлы, не используя Корзину (Recycle Bin). Использование Корзины
     командами F8 и Shift-F8 зависит от конфигурации.

      Уничтожение файлов и папок                                 Alt-Del
    */
    case KEY_F8:
    case KEY_SHIFTDEL:
    case KEY_ALTDEL:
      if (SetCurPath())
      {
        int SaveOpt=Opt.DeleteToRecycleBin;
        if (Key==KEY_SHIFTDEL)
          Opt.DeleteToRecycleBin=0;
        ShellDelete(this,Key==KEY_ALTDEL);
        Opt.DeleteToRecycleBin=SaveOpt;
        if (Opt.AutoChangeFolder && !ModalMode)
          ProcessKey(KEY_ENTER);
      }
      return(TRUE);
    case KEY_HOME:
      Up(0x7fffff);
      if (Opt.AutoChangeFolder && !ModalMode)
        ProcessKey(KEY_ENTER);
      return(TRUE);
    case KEY_END:
      Down(0x7fffff);
      if (Opt.AutoChangeFolder && !ModalMode)
        ProcessKey(KEY_ENTER);
      return(TRUE);
    case KEY_UP:
      Up(1);
      if (Opt.AutoChangeFolder && !ModalMode)
        ProcessKey(KEY_ENTER);
      return(TRUE);
    case KEY_DOWN:
      Down(1);
      if (Opt.AutoChangeFolder && !ModalMode)
        ProcessKey(KEY_ENTER);
      return(TRUE);
    case KEY_PGUP:
      CurTopFile-=Y2-Y1-3-ModalMode;
      CurFile-=Y2-Y1-3-ModalMode;
      DisplayTree(TRUE);
      if (Opt.AutoChangeFolder && !ModalMode)
        ProcessKey(KEY_ENTER);
      return(TRUE);
    case KEY_PGDN:
      CurTopFile+=Y2-Y1-3-ModalMode;
      CurFile+=Y2-Y1-3-ModalMode;
      DisplayTree(TRUE);
      if (Opt.AutoChangeFolder && !ModalMode)
        ProcessKey(KEY_ENTER);
      return(TRUE);
    default:
      if (Key>=KEY_ALT_BASE+0x01 && Key<=KEY_ALT_BASE+255 ||
          Key>=KEY_ALTSHIFT_BASE+0x01 && Key<=KEY_ALTSHIFT_BASE+255)
      {
        FastFind(Key);
        if (Opt.AutoChangeFolder && !ModalMode)
          ProcessKey(KEY_ENTER);
      }
      else
        break;
      return(TRUE);
  }
  return(FALSE);
}


void TreeList::Up(int Count)
{
  CurFile-=Count;
  DisplayTree(TRUE);
}


void TreeList::Down(int Count)
{
  CurFile+=Count;
  DisplayTree(TRUE);
}


void TreeList::CorrectPosition()
{
  if (TreeCount==0)
  {
    CurFile=CurTopFile=0;
    return;
  }
  int Height=Y2-Y1-3-(ModalMode!=0);
  if (CurTopFile+Height>TreeCount)
    CurTopFile=TreeCount-Height;
  if (CurFile<0)
    CurFile=0;
  if (CurFile > TreeCount-1)
    CurFile=TreeCount-1;
  if (CurTopFile<0)
    CurTopFile=0;
  if (CurTopFile > TreeCount-1)
    CurTopFile=TreeCount-1;
  if (CurFile<CurTopFile)
    CurTopFile=CurFile;
  if (CurFile>CurTopFile+Height-1)
    CurTopFile=CurFile-(Height-1);
}


#if defined(__BORLANDC__)
#pragma warn -par
#endif
void TreeList::SetCurDir(char *NewDir,int ClosePlugin)
{
  char SetDir[NM];
  strcpy(SetDir,NewDir);
  if (TreeCount==0)
    Update(0);
  if (TreeCount>0 && !SetDirPosition(SetDir))
  {
    Update(0);
    SetDirPosition(SetDir);
  }
  if (GetFocus())
  {
    CtrlObject->CmdLine->SetCurDir(SetDir);
    CtrlObject->CmdLine->Show();
  }
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif


int TreeList::SetDirPosition(char *NewDir)
{
  long I;
  for (I=0;I<TreeCount;I++)
    if (LocalStricmp(NewDir,ListData[I].Name)==0)
    {
      WorkDir=CurFile=I;
      CurTopFile=CurFile-(Y2-Y1-1)/2;
      CorrectPosition();
      return(TRUE);
    }
  return(FALSE);
}


void TreeList::GetCurDir(char *CurDir)
{
  if (TreeCount==0)
  {
    if (ModalMode==MODALTREE_FREE)
      strcpy(CurDir,Root);
    else
      *CurDir=0;
  }
  else
    strcpy(CurDir,ListData[CurFile].Name);
}


int TreeList::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  int OldFile=CurFile;
  int RetCode;
  if (Opt.ShowPanelScrollbar && MouseX==X2 &&
      (MouseEvent->dwButtonState & 1) && !IsDragging())
  {
    int ScrollY=Y1+1;
    int Height=Y2-Y1-3;
    if (MouseY==ScrollY)
    {
      while (IsMouseButtonPressed())
        ProcessKey(KEY_UP);
      if (!ModalMode)
        SetFocus();
      return(TRUE);
    }
    if (MouseY==ScrollY+Height-1)
    {
      while (IsMouseButtonPressed())
        ProcessKey(KEY_DOWN);
      if (!ModalMode)
        SetFocus();
      return(TRUE);
    }
    if (MouseY>ScrollY && MouseY<ScrollY+Height-1 && Height>2)
    {
      CurFile=(TreeCount-1)*(MouseY-ScrollY)/(Height-2);
      DisplayTree(TRUE);
      if (!ModalMode)
        SetFocus();
      return(TRUE);
    }
  }
  if (Panel::PanelProcessMouse(MouseEvent,RetCode))
    return(RetCode);

  if (MouseEvent->dwMousePosition.Y>Y1 && MouseEvent->dwMousePosition.Y<Y2-2)
  {
    if (!ModalMode)
      SetFocus();
    MoveToMouse(MouseEvent);
    DisplayTree(TRUE);
    if (TreeCount==0)
      return(TRUE);

    if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
        MouseEvent->dwEventFlags==DOUBLE_CLICK ||
        (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED) &&
        MouseEvent->dwEventFlags==0 ||
        OldFile!=CurFile && Opt.AutoChangeFolder && !ModalMode)
    {
      ProcessEnter();
      return(TRUE);
    }
    return(TRUE);
  }
  if (MouseEvent->dwMousePosition.Y<=Y1+1)
  {
    if (!ModalMode)
      SetFocus();
    if (TreeCount==0)
      return(TRUE);
    while (IsMouseButtonPressed() && MouseY<=Y1+1)
      Up(1);
    if (Opt.AutoChangeFolder && !ModalMode)
      ProcessKey(KEY_ENTER);
    return(TRUE);
  }
  if (MouseEvent->dwMousePosition.Y>=Y2-2)
  {
    if (!ModalMode)
      SetFocus();
    if (TreeCount==0)
      return(TRUE);
    while (IsMouseButtonPressed() && MouseY>=Y2-2)
      Down(1);
    if (Opt.AutoChangeFolder && !ModalMode)
      ProcessKey(KEY_ENTER);
    return(TRUE);
  }
  return(FALSE);
}


void TreeList::MoveToMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  CurFile=CurTopFile+MouseEvent->dwMousePosition.Y-Y1-1;
  CorrectPosition();
}


void TreeList::ProcessEnter()
{
  struct TreeItem *CurPtr;
  DWORD Attr;
  CurPtr=ListData+CurFile;
  if ((Attr=GetFileAttributes(CurPtr->Name))!=(DWORD)-1 && (Attr & FA_DIREC))
  {
    if (!ModalMode && chdir(CurPtr->Name)!=-1)
    {
      Panel *AnotherPanel=GetRootPanel();
      SetCurDir(CurPtr->Name,TRUE);
      Show();
      AnotherPanel->SetCurDir(CurPtr->Name,TRUE);
      AnotherPanel->Redraw();
    }
  }
  else
  {
    DelTreeName(CurPtr->Name);
    Update(UPDATE_KEEP_SELECTION);
    Show();
  }
}


int TreeList::ReadTreeFile()
{
  char Name[NM],DirName[NM],LastDirName[NM],*ChPtr;
  FILE *TreeFile=NULL;
  int RootLength=strlen(Root)-1;
  if (RootLength<0)
    RootLength=0;

  FlushCache();
  sprintf(Name,"%s%s",Root,TreeFileName);
  if (MustBeCached(Root) || (TreeFile=fopen(Name,"rb"))==NULL)
    if (!GetCacheTreeName(Root,Name,FALSE) || (TreeFile=fopen(Name,"rb"))==NULL)
      return(FALSE);
  /* $ 13.07.2000 SVS
     не надо смешивать new/delete с realloc
  */
  free(ListData);
  /* SVS $ */
  ListData=NULL;
  TreeCount=0;
  *LastDirName=0;
  strcpy(DirName,Root);
  while (fgets(DirName+RootLength,sizeof(DirName)-RootLength,TreeFile)!=NULL)
  {
   if (LocalStricmp(DirName,LastDirName)==0)
      continue;
    strcpy(LastDirName,DirName);
    if ((ChPtr=strchr(DirName,'\n'))!=NULL)
      *ChPtr=0;
    if (RootLength>0 && DirName[RootLength-1]!=':' &&
        DirName[RootLength]=='\\' && DirName[RootLength+1]==0)
      DirName[RootLength]=0;
    if ((TreeCount & 255)==0 && (ListData=(struct TreeItem *)realloc(ListData,(TreeCount+256+1)*sizeof(struct TreeItem)))==0)
    {
      /* $ 13.07.2000 SVS
         не надо смешивать new/delete с realloc
      */
      free(ListData);
      /* SVS $ */
      ListData=NULL;
      TreeCount=0;
      fclose(TreeFile);
      return(FALSE);
    }
    strncpy(ListData[TreeCount++].Name,DirName,sizeof(ListData[0].Name));
  }
  fclose(TreeFile);

  if (TreeCount==0)
    return(FALSE);
  CaseSensitiveSort=FALSE;

  FillLastData();
  return(TRUE);
}


int TreeList::FindPartName(char *Name,int Next)
{
  char Mask[NM];
  sprintf(Mask,"%s*",Name);
  int I;
  for (I=(Next) ? CurFile+1:CurFile;I<TreeCount;I++)
  {
    CmpNameSearchMode=(I==CurFile);
    if (CmpName(Mask,ListData[I].Name,TRUE))
    {
      CmpNameSearchMode=FALSE;
      CurFile=I;
      CurTopFile=CurFile-(Y2-Y1-1)/2;
      DisplayTree(TRUE);
      return(TRUE);
    }
  }
  CmpNameSearchMode=FALSE;
  for (I=0;I<CurFile;I++)
    if (CmpName(Mask,ListData[I].Name,TRUE))
    {
      CurFile=I;
      CurTopFile=CurFile-(Y2-Y1-1)/2;
      DisplayTree(TRUE);
      return(TRUE);
    }
  return(FALSE);
}


int TreeList::GetSelCount()
{
  return(1);
}


int TreeList::GetSelName(char *Name,int &FileAttr,char *ShortName)
{
  if (Name==NULL)
  {
    GetSelPosition=0;
    return(TRUE);
  }

  if (GetSelPosition==0)
  {
    GetCurDir(Name);
    if (ShortName!=NULL)
      strcpy(ShortName,Name);
    FileAttr=FA_DIREC;
    GetSelPosition++;
    return(TRUE);
  }
  GetSelPosition=0;
  return(FALSE);
}


int TreeList::GetCurName(char *Name,char *ShortName)
{
  if (TreeCount==0)
  {
    *Name=*ShortName=0;
    return(FALSE);
  }
  strcpy(Name,ListData[CurFile].Name);
  strcpy(ShortName,Name);
  return(TRUE);
}


void TreeList::AddTreeName(char *Name)
{
  char *ListName,*NewPtr;
  char FullName[NM],DirName[NM],Root[NM],*ChPtr;
  long CachePos,TreeCount,CmpLength;

  if (*Name==0)
    return;
  ConvertNameToFull(Name,FullName, sizeof(FullName));
  Name=FullName;
  GetPathRoot(Name,Root);
  Name+=strlen(Root)-1;
  if ((ChPtr=strrchr(Name,'\\'))==NULL)
    return;
  CmpLength=ChPtr-Name;
  ReadCache(Root);
  ListName=NULL;
  TreeCount=0;
  *DirName=0;
  CachePos=0;
  while (1)
  {
    if (CmpLength>=0 && *DirName &&
        (CmpLength==0 || LocalStrnicmp(DirName,Name,CmpLength)==0) &&
        (DirName[CmpLength]==0 || DirName[CmpLength]=='\\'))
    {
      strcpy(DirName,Name);
      CmpLength=-1;
    }
    else
      if (CachePos<TreeCache.TreeCount)
        strncpy(DirName,&TreeCache.ListName[NM*CachePos++],sizeof(DirName));
      else
        break;

    if ((TreeCount & DELTA_TREECOUNT)==0 &&
      (NewPtr=(char *)realloc(ListName,(TreeCount+(DELTA_TREECOUNT+1)+1)*NM))==NULL)
    {
      /* $ 13.07.2000 SVS
         не надо смешивать new/delete с realloc
      */
      free(NewPtr);
      /* SVS $ */
      return;
    }
    ListName=NewPtr;
    strncpy(&ListName[NM*TreeCount++],DirName,NM);
  }
  /* $ 13.07.2000 SVS
     не надо смешивать new/delete с realloc
  */
  free(TreeCache.ListName);
  /* SVS $ */
  TreeCache.ListName=ListName;
  TreeCache.TreeCount=TreeCount;
}


void TreeList::DelTreeName(char *Name)
{
  char *ListName,*NewPtr;
  char FullName[NM],DirName[NM],Root[NM];
  long CachePos,TreeCount;
  int Length;
  if (*Name==0)
    return;
  ConvertNameToFull(Name,FullName, sizeof(FullName));
  Name=FullName;
  GetPathRoot(Name,Root);
  Name+=strlen(Root)-1;
  ReadCache(Root);
  ListName=NULL;
  TreeCount=0;
  *DirName=0;
  for (CachePos=0;CachePos<TreeCache.TreeCount;CachePos++)
  {
    strncpy(DirName,&TreeCache.ListName[NM*CachePos],sizeof(DirName));
    if (LocalStrnicmp(Name,DirName,Length=strlen(Name))==0 &&
        (DirName[Length]==0 || DirName[Length]=='\\'))
      continue;
    if ((TreeCount & DELTA_TREECOUNT)==0 &&
      (NewPtr=(char *)realloc(ListName,(TreeCount+(DELTA_TREECOUNT+1)+1)*NM))==NULL)
    {
      /* $ 13.07.2000 SVS
        не надо смешивать new/delete с realloc
      */
      free(NewPtr);
      /* SVS $ */
      return;
    }
    ListName=NewPtr;
    strncpy(&ListName[NM*TreeCount++],DirName,NM);
  }
  /* $ 13.07.2000 SVS
     не надо смешивать new/delete с realloc
  */
  free(TreeCache.ListName);
  /* SVS $ */
  TreeCache.ListName=ListName;
  TreeCache.TreeCount=TreeCount;
}


void TreeList::RenTreeName(char *SrcName,char *DestName)
{
  if (*SrcName==0 || *DestName==0)
    return;
  char SrcRoot[NM],DestRoot[NM];
  GetPathRoot(SrcName,SrcRoot);
  GetPathRoot(DestName,DestRoot);
  if (LocalStricmp(SrcRoot,DestRoot)!=0)
  {
    DelTreeName(SrcName);
    ReadSubTree(SrcName);
  }
  SrcName+=strlen(SrcRoot)-1;
  DestName+=strlen(DestRoot)-1;

  ReadCache(SrcRoot);

  int SrcLength=strlen(SrcName);
  for (int CachePos=0;CachePos<TreeCache.TreeCount;CachePos++)
  {
    char *DirName=TreeCache.ListName+NM*CachePos;
    if (LocalStrnicmp(SrcName,DirName,SrcLength)==0 &&
        (DirName[SrcLength]==0 || DirName[SrcLength]=='\\'))
    {
      char NewName[2*NM];
      strcpy(NewName,DestName);
      strcat(NewName,DirName+SrcLength);
      strncpy(DirName,NewName,NM-1);
    }
  }
}


void TreeList::ReadSubTree(char *Path)
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  SaveScreen SaveScr;
  ScanTree ScTree(FALSE);
  WIN32_FIND_DATA fdata;
  char FullName[NM],DirName[NM];
  int Count=0,FileAttr;

  if ((FileAttr=GetFileAttributes(Path))==-1 || (FileAttr & FA_DIREC)==0)
    return;
  ConvertNameToFull(Path,DirName, sizeof(DirName));
  AddTreeName(DirName);

  int FirstCall=TRUE;
  ScTree.SetFindPath(DirName,"*.*");
  while (ScTree.GetNextName(&fdata,FullName))
    if (fdata.dwFileAttributes & FA_DIREC)
    {
      if (!MsgReadTree(++Count,FirstCall))
        break;
      AddTreeName(FullName);
    }
}


void TreeList::ClearCache(int EnableFreeMem)
{
  if (EnableFreeMem && TreeCache.ListName!=NULL)
    /* $ 13.07.2000 SVS
       не надо смешивать new/delete с realloc
    */
    free(TreeCache.ListName);
    /* SVS $ */
  *TreeCache.TreeName=0;
  TreeCache.ListName=NULL;
  TreeCache.TreeCount=0;
}


void TreeList::ReadCache(char *TreeRoot)
{
  char TreeName[NM],DirName[NM],*ChPtr;
  char *ListName;
  FILE *TreeFile=NULL;
  /* $ 07.08.2001 SVS
     Ну просто ОХРЕНЕТЬ!!!!
     т.е. сначала сравниваем, а потом формируем имя? ню-ню.
     Камень в огород ER.
  */
  sprintf(TreeName,"%s%s",TreeRoot,TreeFileName);

  if (strcmp(TreeName,TreeCache.TreeName)==0)
    return;
  /* SVS $ */
  if (TreeCache.ListName!=NULL)
    FlushCache();

  if (MustBeCached(TreeRoot) || (TreeFile=fopen(TreeName,"rb"))==NULL)
    if (!GetCacheTreeName(TreeRoot,TreeName,FALSE) || (TreeFile=fopen(TreeName,"rb"))==NULL)
    {
      ClearCache(1);
      return;
    }
  strcpy(TreeCache.TreeName,TreeName);
  while (fgets(DirName,sizeof(DirName),TreeFile)!=NULL)
  {
    if ((ChPtr=strchr(DirName,'\n'))!=NULL)
      *ChPtr=0;
    if ((TreeCache.TreeCount & DELTA_TREECOUNT)==0 &&
      (ListName=(char *)realloc(TreeCache.ListName,(TreeCache.TreeCount+(DELTA_TREECOUNT+1)+1)*NM))==NULL)
    {
      fclose(TreeFile);
      return;
    }
    TreeCache.ListName=ListName;
    strncpy(&TreeCache.ListName[NM*TreeCache.TreeCount++],DirName,NM);
  }
  fclose(TreeFile);
}


void TreeList::FlushCache()
{
  FILE *TreeFile;
  int I;
  if (*TreeCache.TreeName)
  {
    DWORD FileAttributes=GetFileAttributes(TreeCache.TreeName);
    if(FileAttributes != -1)
      SetFileAttributes(TreeCache.TreeName,0);
    if ((TreeFile=fopen(TreeCache.TreeName,"wb"))==NULL)
    {
      ClearCache(1);
      return;
    }
    qsort(TreeCache.ListName,TreeCache.TreeCount,NM,SortCacheList);
    for (I=0;I<TreeCache.TreeCount;I++)
      fprintf(TreeFile,"%s\n",&TreeCache.ListName[NM*I]);
    if (fclose(TreeFile)==EOF)
    {
      clearerr(TreeFile);
      fclose(TreeFile);
      remove(TreeCache.TreeName);
      Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotSaveTree),
              TreeCache.TreeName,MSG(MOk));
    }
    else if(FileAttributes != -1) // вернем атрибуты (если получится :-)
      SetFileAttributes(TreeCache.TreeName,FileAttributes);
  }
  ClearCache(1);
}


void TreeList::UpdateViewPanel()
{
  if (!ModalMode)
  {
    Panel *AnotherPanel=GetRootPanel();
    char CurName[NM];
    GetCurDir(CurName);
    if (AnotherPanel->GetType()==QVIEW_PANEL && SetCurPath())
      ((QuickView *)AnotherPanel)->ShowFile(CurName,FALSE,NULL);
  }
}


int TreeList::GoToFile(char *Name)
{
  for (long I=0;I<TreeCount;I++)
    if (LocalStricmp(Name,ListData[I].Name)==0)
    {
      CurFile=I;
      CorrectPosition();
      return(TRUE);
    }
  return(FALSE);
}

int _cdecl SortList(const void *el1,const void *el2)
{
  char *NamePtr1=((struct TreeItem *)el1)->Name;
  char *NamePtr2=((struct TreeItem *)el2)->Name;
  return(StaticSortCaseSensitive ? TreeCmp(NamePtr1,NamePtr2):LCStricmp(NamePtr1,NamePtr2));
}

int _cdecl SortCacheList(const void *el1,const void *el2)
{
  return(LCStricmp((char *)el1,(char *)el2));
}


int TreeCmp(char *Str1,char *Str2)
{
  while (1)
  {
    if (*Str1 != *Str2)
    {
      if (*Str1==0)
        return(-1);
      if (*Str2==0)
        return(1);
      if (*Str1=='\\')
        return(-1);
      if (*Str2=='\\')
        return(1);
      return(*Str1<*Str2 ? -1:1);
    }
    if (*(Str1++) == 0)
      break;
    Str2++;
  }
  return(0);
}

/* $ 16.10.2000 tran
 функция, определяющаяя необходимость кеширования
 файла */
int TreeList::MustBeCached(char *Root)
{
    UINT type;

    type=GetDriveType(Root);
    if ( type==DRIVE_UNKNOWN ||
         type==DRIVE_NO_ROOT_DIR ||
         type==DRIVE_REMOVABLE ||
         type==DRIVE_CDROM
         )
    {
        if ( type==DRIVE_REMOVABLE )
        {
            if ( toupper(Root[0])=='A' || toupper(Root[0])=='B')
            {
                return FALSE; // это дискеты
            }
        }
        return TRUE;
        // кешируются CD, removable и неизвестно что :)
    }
    /* остались
        DRIVE_REMOTE
        DRIVE_RAMDISK
        DRIVE_FIXED
    */
    return FALSE;
}

void TreeList::SetFocus()
{
  Panel::SetFocus();
  SetMacroMode(FALSE);
}

void TreeList::KillFocus()
{
  if (CurFile<TreeCount)
  {
    struct TreeItem *CurPtr=ListData+CurFile;
    if (GetFileAttributes(CurPtr->Name)==(DWORD)-1)
    {
      DelTreeName(CurPtr->Name);
      Update(UPDATE_KEEP_SELECTION);
    }
  }
  Panel::KillFocus();
  SetMacroMode(TRUE);
}

void TreeList::SetMacroMode(int Restore)
{
  if (CtrlObject == NULL)
    return;
  if (PrevMacroMode == -1)
    PrevMacroMode = CtrlObject->Macro.GetMode();
  CtrlObject->Macro.SetMode(Restore ? PrevMacroMode:MACRO_TREEPANEL);
}
