/*
treelist.cpp

Tree panel

*/

/* Revision: 1.67 01.04.2005 $ */

/*
Modify:
  01.04.2005 SVS
    + GetItem()
  10.03.2005 SVS
    + У FindFile() и GoToFile() второй параметр - искать только по имени файла
    + CreateTreeFileName() - заготовка для проекта Tree.far
  03.03.2005 SVS
    ! У функции FindPartName() добавлен третий параметр - направление поиска.
    ! Так же в TreeList::AddTreeName() добавлено условие точного совпадения
      добавляемого имени с тем, что в кеше (иначе идет дубляж).
  01.03.2005 SVS
    ! Opt.AutoChangeFolder -> Opt.Tree.AutoChangeFolder
  14.02.2005 SVS
    + В TreeList добавлены функции GetFileName(), FindFile()
  11.11.2004 SVS
    + Обработка MCODE_V_ITEMCOUNT и MCODE_V_CURPOS
  10.11.2004 SVS
    + В HMenu и TreeList добавлена обработка MCODE_*
  01.11.2004 SVS
    - Gray+ и Gray- - не было автосмены папок
  28.10.2004 SVS
    + "Конструкторы" имен - TreeFileName() и TreeCacheFolderName()
      В дальнейшем нужно осуществить TODO по этим функциям!
    ! некоторые переменные вогнаны в константы
  16.10.2004 SVS
    + Gray+ и Gray- используются для быстрого перемещения вверх или вниз по папкам одного уровня.
    + GetNextNavPos() и GetPrevNavPos()
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  08.06.2004 SVS
    ! Вместо GetDriveType теперь вызываем FAR_GetDriveType().
    ! Вместо "DriveType==DRIVE_CDROM" вызываем IsDriveTypeCDROM()
  20.05.2004 SVS
    ! NumericSort - свойство конкретной панели, а не режима отображения
    - Bug#695 - Не работает прерывание по Esc
  19.05.2004 SVS
    ! вместо "SetFileAttributes(Name,0)" выставим "SetFileAttributes(Name,FILE_ATTRIBUTE_NORMAL)"
      пусть баундчекер не блюет.
  11.07.2003 SVS
    + NumericSort
  06.05.2003 SVS
    ! Учтем Opt.UseUnicodeConsole при рисовании дерева
  22.04.2003 SVS
    ! strcpy -> strNcpy
  26.02.2003 SVS
    - BugZ#813 - DM_RESIZEDIALOG в DN_DRAWDIALOG -> проблема
  24.02.2003 SVS
    + для пассивной панели типа QVIEW_PANEL добавим LockScreen, дабы исключить
      лишнюю прорисовку.
  21.01.2003 SVS
    + xf_malloc,xf_realloc,xf_free - обертки вокруг malloc,realloc,free
      Просьба блюсти порядок и прописывать именно xf_* вместо простых.
  04.01.2003 VVM
    - При изменении размера консоли прогресс начинал глючить.
  27.12.2002 VVM
    + Показывать индикатор сканирования раз в секунду.
    + Выделять память блоками по 255 итемов.
    + Сканировать каталоги за один проход.
  04.12.2002 SVS
    - BugZ#695 - Не работает прерывание по Esc
  29.05.2002 SKV
    ! Оптимизация TreeCache
  24.05.2002 SVS
    + Дублирование Numpad-клавиш
  27.04.2002 SVS
    ! 8192 -> MAXSIZE_SHORTCUTDATA
  12.04.2002 IS
    + Учтем то, что Plugin.PutFiles может вернуть 2
  08.04.2002 IS
    ! внедрение const
  26.03.2002 KM
    - Падало в SetTitle на конструкции:
      ===
      sprintf(TitleDir,"{%s - Tree}",NM-1,Ptr);
      ===
      Так как %s был указан без модификатора ".*"
  26.03.2002 DJ
    ! ScanTree::GetNextName() принимает размер буфера для имени файла
  22.03.2002 SVS
    - strcpy - Fuck!
  21.02.2002 SVS
    - BugZ#316 - Нет разделителя.
  12.02.2002 SVS
    - BugZ#303 - Колёсико мыши в дереве
      Сделаем по аналогии с наФигацией в обычной панели.
  16.01.2002 SVS
    - BugZ#249 - Непрорисовка при создании дерева
    - Бага - не отрисовывалась текущий каталог в поз. (Y2-1)
  14.01.2002 IS
    ! chdir -> FarChDir
  28.12.2001 DJ
    - нафиг такую оптимизацию, после которой VC++ билд трапается!
  26.12.2001 SVS
    ! немного оптимизации.
    - Бага - после выделения памяти начинаем рисовать без предварительной
      инициализации нулевого итема в нужное значение. В итоге налицо -
      бага с прорисовкой в заголовке консольного окна.
  11.12.2001 SVS
    + Свой кейбар в деревяхе
    - не обновлась пассивная панель при удалении каталога из деревяхи
  08.12.2001 IS
    - баг: показывали левую справку по F1
    ! небольшая оптимизация по размеру - вместо "define строка"
      используем "const char[]"
  24.10.2001 SVS
    - бага с прорисовкой при вызове дерева из диалога копирования
  24.10.2001 VVM
    ! После сканирования диска перерисовать другую панель.
  24.10.2001 VVM
    ! Рисуем ветки дерева только при установленном TreeIsPrepared.
  23.10.2001 SVS
    ! немного оптимизации - sprintf для "%d" - это жирно.
  22.10.2001 SVS
    - Забыл прибить CALLBACK-функцию при выходе :-(
    ! исправление отрисовки после CALLBACK
    ! ReadTree() возвращает TRUE/FALSE
  21.10.2001 SVS
    + CALLBACK-функция для избавления от BugZ#85
  27.09.2001 IS
    - Левый размер при использовании strncpy
  24.09.2001 VVM
    ! Писать сообщение о чтении дерева только, если это заняло более 500 мсек.
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
#include "help.hpp"
#include "lockscrn.hpp"
#include "macroopcode.hpp"
#include "RefreshFrameManager.hpp"

#define DELTA_TREECOUNT 31

static int _cdecl SortList(const void *el1,const void *el2);
static int _cdecl SortCacheList(const void *el1,const void *el2);
static int StaticSortCaseSensitive;
static int StaticSortNumeric;
static int TreeCmp(char *Str1,char *Str2);
static clock_t TreeStartTime;
static int LastScrX = -1;
static int LastScrY = -1;

static char TreeLineSymbol[4][3]={
  {0x20,0x20,/*0x20,*/0x00},
  {0xB3,0x20,/*0x20,*/0x00},
  {0xC0,0xC4,/*0xC4,*/0x00},
  {0xC3,0xC4,/*0xC4,*/0x00},
};

#if defined(USE_WFUNC)
static WCHAR TreeLineSymbolW[4][3]={0};
#endif

static struct TreeListCache
{
  char TreeName[NM];
  char **ListName;
  int TreeCount;
  int TreeSize;

  TreeListCache()
  {
    ListName=NULL;
    TreeCount=0;
    TreeSize=0;
  }
  void Resize()
  {
    if(TreeCount==TreeSize)
    {
      TreeSize+=TreeSize?TreeSize>>2:32;
      char **NewPtr=(char**)xf_realloc(ListName,sizeof(char*)*TreeSize);
      if(!NewPtr)return;
      ListName=NewPtr;
    }
  }
  void Add(const char* name)
  {
    Resize();
    ListName[TreeCount++]=xf_strdup(name);
  }
  void Insert(int idx,const char* name)
  {
    Resize();
    memmove(ListName+idx+1,ListName+idx,sizeof(char*)*(TreeCount-idx));
    ListName[idx]=xf_strdup(name);
    TreeCount++;
  }
  void Delete(int idx)
  {
    xf_free(ListName[idx]);
    memmove(ListName+idx,ListName+idx+1,sizeof(char*)*(TreeCount-idx-1));
    TreeCount--;
  }

  void Clean()
  {
    if(!TreeSize)return;
    for(int i=0;i<TreeCount;i++)
    {
      xf_free(ListName[i]);
    }
    xf_free(ListName);
    ListName=NULL;
    TreeCount=0;
    TreeSize=0;
    TreeName[0]=0;
  }
} TreeCache;


TreeList::TreeList(int IsPanel)
{
  Type=TREE_PANEL;
  ListData=NULL;
  TreeCount=0;
  WorkDir=CurFile=CurTopFile=0;
  GetSelPosition=0;
  *Root=0;
  Flags.Set(FTREELIST_UPDATEREQUIRED);
  CaseSensitiveSort=FALSE;
  NumericSort=FALSE;
  PrevMacroMode = -1;
  Flags.Clear(FTREELIST_TREEISPREPARED);
  ExitCode=1;
  Flags.Change(FTREELIST_ISPANEL,IsPanel);
}


TreeList::~TreeList()
{
  xf_free(ListData);
  FlushCache();
  SetMacroMode(TRUE);
}

void TreeList::SetRootDir(char *NewRootDir)
{
  xstrncpy(Root,NewRootDir,sizeof(Root)-1);
  xstrncpy(CurDir,NewRootDir,sizeof(CurDir)-1);
}

void TreeList::DisplayObject()
{
  if (Flags.Check(FSCROBJ_ISREDRAWING))
    return;
  Flags.Set(FSCROBJ_ISREDRAWING);

  if (Flags.Check(FTREELIST_UPDATEREQUIRED))
    Update(0);
  if(ExitCode)
  {
    Panel *RootPanel=GetRootPanel();
    if (RootPanel->GetType()==FILE_PANEL)
    {
      int RootCaseSensitive=((FileList *)RootPanel)->IsCaseSensitive();
      int RootNumeric=RootPanel->GetNumericSort();
      if (RootCaseSensitive!=CaseSensitiveSort || RootNumeric != NumericSort)
      {
        CaseSensitiveSort=RootCaseSensitive;
        NumericSort=RootNumeric;
        StaticSortCaseSensitive=CaseSensitiveSort;
        StaticSortNumeric=NumericSort;
        far_qsort(ListData,TreeCount,sizeof(*ListData),SortList);
        FillLastData();
        SyncDir();
      }
    }
    DisplayTree(FALSE);
  }
  Flags.Clear(FSCROBJ_ISREDRAWING);
}


void TreeList::DisplayTree(int Fast)
{
  int I,J,K;
  struct TreeItem *CurPtr;
  char Title[100];
  LockScreen *LckScreen=NULL;

  if(CtrlObject->Cp()->GetAnotherPanel(this)->GetType() == QVIEW_PANEL)
    LckScreen=new LockScreen;

  CorrectPosition();
  if (TreeCount>0)
    xstrncpy(CurDir,ListData[CurFile].Name,sizeof(CurDir)-1);
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
    if (J<TreeCount && Flags.Check(FTREELIST_TREEISPREPARED))
    {
      if (J==0)
        DisplayTreeName("\\",J);
      else
      {
#if defined(USE_WFUNC)
        // первоначальная инициализация
        if(TreeLineSymbolW[0][0] == 0x0000)
        {
          for(int IW=0; IW < 4; ++IW)
          {
            for(int JW=0; JW < 2; ++JW)
              TreeLineSymbolW[IW][JW]=(TreeLineSymbol[IW][JW] == 0x20)?0x20:BoxSymbols[TreeLineSymbol[IW][JW]-0x0B0];
          }
        }
        WCHAR OutStrW[200];
        *OutStrW=0;
#endif
        char  OutStr[200];
        for (*OutStr=0,K=0;K<CurPtr->Depth-1 && WhereX()+3*K<X2-6;K++)
        {
          if (CurPtr->Last[K])
          {
#if defined(USE_WFUNC)
            if(Opt.UseUnicodeConsole)
              wcscat(OutStrW,TreeLineSymbolW[0]);
            else
#endif
              strcat(OutStr,TreeLineSymbol[0]);
          }
          else
          {
#if defined(USE_WFUNC)
            if(Opt.UseUnicodeConsole)
              wcscat(OutStrW,TreeLineSymbolW[1]);
            else
#endif
            strcat(OutStr,TreeLineSymbol[1]);
          }
        }
        if (CurPtr->Last[CurPtr->Depth-1])
        {
#if defined(USE_WFUNC)
          if(Opt.UseUnicodeConsole)
            wcscat(OutStrW,TreeLineSymbolW[2]);
          else
#endif
            strcat(OutStr,TreeLineSymbol[2]);
        }
        else
        {
#if defined(USE_WFUNC)
          if(Opt.UseUnicodeConsole)
            wcscat(OutStrW,TreeLineSymbolW[3]);
          else
#endif
            strcat(OutStr,TreeLineSymbol[3]);
        }
#if defined(USE_WFUNC)
        if(Opt.UseUnicodeConsole)
          BoxTextW(OutStrW,FALSE);
        else
#endif
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

  SetScreen(X1+1,Y2-(ModalMode?2:1),X2-1,Y2-1,' ',COL_PANELTEXT);
  if (TreeCount>0)
  {
    GotoXY(X1+1,Y2-1);
    mprintf("%-*.*s",X2-X1-1,X2-X1-1,ListData[CurFile].Name);
  }

  UpdateViewPanel();
  SetTitle(); // не забудим прорисовать заголовок
  if(LckScreen)
    delete LckScreen;
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
    Flags.Set(FTREELIST_UPDATEREQUIRED);
    return;
  }
  Flags.Clear(FTREELIST_UPDATEREQUIRED);
  GetRoot();
  int LastTreeCount=TreeCount;
  int RetFromReadTree=TRUE;

  Flags.Clear(FTREELIST_TREEISPREPARED);
  int TreeFilePresent=ReadTreeFile();
  if (!TreeFilePresent)
    RetFromReadTree=ReadTree();
  Flags.Set(FTREELIST_TREEISPREPARED);

  if(!RetFromReadTree && !Flags.Check(FTREELIST_ISPANEL))
  {
    ExitCode=0;
    return;
  }

  if (RetFromReadTree && TreeCount>0 && ((Mode & UPDATE_KEEP_SELECTION)==0 || LastTreeCount!=TreeCount))
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
  else if(!RetFromReadTree)
  {
    Show();
    Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
    AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
    AnotherPanel->Redraw();
  }
}


int TreeList::ReadTree()
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  //SaveScreen SaveScr;
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
  xf_free(ListData);
  TreeCount=0;
  if ((ListData=(struct TreeItem*)xf_malloc((TreeCount+256+1)*sizeof(struct TreeItem)))==NULL)
//  if ((ListData=(struct TreeItem*)xf_malloc(sizeof(struct TreeItem)))==NULL)
    return FALSE;
  /* SVS $ */

  memset(&ListData[0], 0, sizeof(ListData[0]));
  strcpy(ListData->Name,Root);

  SaveScreen SaveScrTree;
  UndoGlobalSaveScrPtr UndSaveScr(&SaveScrTree);

  /* Т.к. мы можем вызвать диалог подтверждения (который не перерисовывает панельки,
     а восстанавливает сохраненный образ экрана, то нарисуем чистую панель */
  Redraw();

  if (RootLength>0 && Root[RootLength-1]!=':' && Root[RootLength]=='\\')
    ListData->Name[RootLength]=0;
  TreeCount=1;

  int FirstCall=TRUE, AscAbort=FALSE;
  TreeStartTime = clock();
  SetPreRedrawFunc(TreeList::PR_MsgReadTree);

  RefreshFrameManager frref(ScrX,ScrY,TreeStartTime,FALSE);//DontRedrawFrame);

  ScTree.SetFindPath(Root,"*.*",0);
  LastScrX = ScrX;
  LastScrY = ScrY;
  while (ScTree.GetNextName(&fdata,FullName, sizeof (FullName)-1))
  {
//    if(TreeCount > 3)
    TreeList::MsgReadTree(TreeCount,FirstCall);
    if (CheckForEscSilent())
    {
      AscAbort=ConfirmAbortOp()!=0;
      FirstCall=TRUE;
    }
    if(AscAbort)
      break;
    if ((TreeCount & 255)==0 && (ListData=(struct TreeItem *)xf_realloc(ListData,(TreeCount+256+1)*sizeof(struct TreeItem)))==NULL)
//    if ((ListData=(struct TreeItem *)xf_realloc(ListData,(TreeCount+1)*sizeof(struct TreeItem)))==NULL)
    {
      AscAbort=TRUE;
      break;
    }

    if (!(fdata.dwFileAttributes & FA_DIREC))
      continue;

    memset(&ListData[TreeCount], 0, sizeof(ListData[0]));
    strcpy(ListData[TreeCount++].Name,FullName);
  }

  if(AscAbort)
  {
    xf_free(ListData);
    ListData=NULL;
    TreeCount=0;
    SetPreRedrawFunc(NULL);
    return FALSE;
  }

  SetPreRedrawFunc(NULL);
  StaticSortCaseSensitive=CaseSensitiveSort=StaticSortNumeric=NumericSort=FALSE;
  far_qsort(ListData,TreeCount,sizeof(*ListData),SortList);

  FillLastData();
  SaveTreeFile();
  if (!FirstCall)
  { // Перерисуем другую панель - удалим следы сообщений :)
    Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
    AnotherPanel->Redraw();
  }
  return TRUE;
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
  MkTreeFileName(Root,Name,sizeof(Name)-1);
  // получим и сразу сбросим атрибуты (если получится)
  DWORD FileAttributes=GetFileAttributes(Name);
  if(FileAttributes != -1)
    SetFileAttributes(Name,FILE_ATTRIBUTE_NORMAL);
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
  MkTreeCacheFolderName(FarPath,FolderName,sizeof(FolderName)-1);
  if (CreateDir)
  {
    mkdir(FolderName);
    SetFileAttributes(FolderName,FA_HIDDEN);
  }
  char RemoteName[NM*3];
  *RemoteName=0;
  if (*Root=='\\')
    strcpy(RemoteName,Root);
  else
  {
    char LocalName [8];
    strcpy (LocalName, "A:");
    *LocalName=*Root;
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


void TreeList::PR_MsgReadTree(void)
{
  int FirstCall=1;
  TreeList::MsgReadTree(PreRedrawParam.Flags,FirstCall);
}

int TreeList::MsgReadTree(int TreeCount,int &FirstCall)
{
  /* $ 24.09.2001 VVM
    ! Писать сообщение о чтении дерева только, если это заняло более 500 мсек. */
  BOOL IsChangeConsole = LastScrX != ScrX || LastScrY != ScrY;
  if(IsChangeConsole)
  {
    LastScrX = ScrX;
    LastScrY = ScrY;
  }

  if (IsChangeConsole || (clock() - TreeStartTime) > 1000)
  {
    char NumStr[32];
    itoa(TreeCount,NumStr,10);
    Message((FirstCall ? 0:MSG_KEEPBACKGROUND),0,MSG(MTreeTitle),
            MSG(MReadingTree),NumStr);
    PreRedrawParam.Flags=TreeCount;
    TreeStartTime = clock();
  }
  /* VVM $ */
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
    char ShortcutFolder[NM],PluginModule[NM],PluginFile[NM],PluginData[MAXSIZE_SHORTCUTDATA];
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
/*
    case MCODE_OP_PLAINTEXT:
    {
      const char *str = eStackAsString();
      if (!*str)
        return FALSE;
      Key=*str;
      break;
    }
*/
    case MCODE_C_EMPTY:
      return TreeCount<=0;
    case MCODE_C_EOF:
      return CurFile==TreeCount-1;
    case MCODE_C_BOF:
      return CurFile==0;
    case MCODE_C_SELECTED:
      return FALSE;
    case MCODE_V_ITEMCOUNT:
      return TreeCount;
    case MCODE_V_CURPOS:
      return CurFile+1;
/*
    case MCODE_F_MENU_CHECKHOTKEY:
    {
      const char *str = eStackAsString(1);
      if ( *str )
        return CheckHighlights(*str);
      return FALSE;
    }
*/
  }

  switch(Key)
  {
    /* $ 08.12.2001 IS просят справку для "дерева", ее и покажем
    */
    case KEY_F1:
    {
      {
         Help Hlp ("TreePanel");
      }
      return TRUE;
    }
    /* IS $ */

    case KEY_SHIFTENTER:
    case KEY_CTRLENTER:
    case KEY_CTRLF:
    case KEY_CTRLALTINS:  case KEY_CTRLALTNUMPAD0:
    {
      {
        char QuotedName[NM+2];
        int CAIns=(Key == KEY_CTRLALTINS || Key == KEY_CTRLALTNUMPAD0);

        CurPtr=ListData+CurFile;
        if (strchr(CurPtr->Name,' ')!=NULL)
          sprintf(QuotedName,"\"%s\"%s",CurPtr->Name,(CAIns?"":" "));
        else
          sprintf(QuotedName,"%s%s",CurPtr->Name,(CAIns?"":" "));
        if(CAIns)
          CopyToClipboard(QuotedName);
        else if(Key == KEY_SHIFTENTER)
          Execute(QuotedName,FALSE,TRUE,TRUE);
        else
          CtrlObject->CmdLine->InsertString(QuotedName);
      }
      return(TRUE);
    }

    case KEY_CTRLBACKSLASH:
    {
      CurFile=0;
      ProcessEnter();
      return(TRUE);
    }

    case KEY_ENTER:
    {
      if (!ModalMode && CtrlObject->CmdLine->GetLength()>0)
        break;
      ProcessEnter();
      return(TRUE);
    }

    case KEY_F4:
    case KEY_CTRLA:
    {
      if (SetCurPath())
        ShellSetFileAttributes(this);
      return(TRUE);
    }

    case KEY_CTRLR:
    {
      ReadTree();
      if (TreeCount>0)
        SyncDir();
      Redraw();
      break;
    }

    case KEY_SHIFTF5:
    case KEY_SHIFTF6:
    {
      if (SetCurPath())
      {
        int ToPlugin=0;
        ShellCopy ShCopy(this,Key==KEY_SHIFTF6,FALSE,TRUE,TRUE,ToPlugin,NULL);
      }
      return(TRUE);
    }

    case KEY_F5:
    case KEY_DRAGCOPY:
    case KEY_F6:
    case KEY_ALTF6:
    case KEY_DRAGMOVE:
    {
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
          int PutCode=CtrlObject->Plugins.PutFiles(hAnotherPlugin,ItemList,ItemNumber,Move,0);
          if (PutCode==1 || PutCode==2)
            AnotherPanel->SetPluginModified();
          /* $ 13.07.2000 SVS
             не надо смешивать new/delete с realloc
          */
          xf_free(ItemList);
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
    }

    case KEY_F7:
    {
      if (SetCurPath())
        ShellMakeDir(this);
      return(TRUE);
    }

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
    {
      if (SetCurPath())
      {
        int SaveOpt=Opt.DeleteToRecycleBin;
        if (Key==KEY_SHIFTDEL)
          Opt.DeleteToRecycleBin=0;
        ShellDelete(this,Key==KEY_ALTDEL);
        // Надобно не забыть обновить противоположную панель...
        Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
        AnotherPanel->Update(UPDATE_KEEP_SELECTION);
        AnotherPanel->Redraw();

        Opt.DeleteToRecycleBin=SaveOpt;
        if (Opt.Tree.AutoChangeFolder && !ModalMode)
          ProcessKey(KEY_ENTER);
      }
      return(TRUE);
    }

    case KEY_MSWHEEL_UP:
    case (KEY_MSWHEEL_UP | KEY_ALT):
    {
      Scroll(Key & KEY_ALT?-1:-Opt.MsWheelDelta);
      return(TRUE);
    }

    case KEY_MSWHEEL_DOWN:
    case (KEY_MSWHEEL_DOWN | KEY_ALT):
    {
      Scroll(Key & KEY_ALT?1:Opt.MsWheelDelta);
      return(TRUE);
    }

    case KEY_HOME:        case KEY_SHIFTNUMPAD7:
    {
      Up(0x7fffff);
      if (Opt.Tree.AutoChangeFolder && !ModalMode)
        ProcessKey(KEY_ENTER);
      return(TRUE);
    }

    case KEY_ADD: // OFM: Gray+/Gray- navigation
    {
      CurFile=GetNextNavPos();
      if (Opt.Tree.AutoChangeFolder && !ModalMode)
        ProcessKey(KEY_ENTER);
      else
        DisplayTree(TRUE);
      return TRUE;
    }

    case KEY_SUBTRACT: // OFM: Gray+/Gray- navigation
    {
      CurFile=GetPrevNavPos();
      if (Opt.Tree.AutoChangeFolder && !ModalMode)
        ProcessKey(KEY_ENTER);
      else
        DisplayTree(TRUE);
      return TRUE;
    }

    case KEY_END:         case KEY_SHIFTNUMPAD1:
    {
      Down(0x7fffff);
      if (Opt.Tree.AutoChangeFolder && !ModalMode)
        ProcessKey(KEY_ENTER);
      return(TRUE);
    }

    case KEY_UP:          case KEY_SHIFTNUMPAD8:
    {
      Up(1);
      if (Opt.Tree.AutoChangeFolder && !ModalMode)
        ProcessKey(KEY_ENTER);
      return(TRUE);
    }

    case KEY_DOWN:        case KEY_SHIFTNUMPAD2:
    {
      Down(1);
      if (Opt.Tree.AutoChangeFolder && !ModalMode)
        ProcessKey(KEY_ENTER);
      return(TRUE);
    }

    case KEY_PGUP:        case KEY_SHIFTNUMPAD9:
    {
      CurTopFile-=Y2-Y1-3-ModalMode;
      CurFile-=Y2-Y1-3-ModalMode;
      DisplayTree(TRUE);
      if (Opt.Tree.AutoChangeFolder && !ModalMode)
        ProcessKey(KEY_ENTER);
      return(TRUE);
    }

    case KEY_PGDN:        case KEY_SHIFTNUMPAD3:
    {
      CurTopFile+=Y2-Y1-3-ModalMode;
      CurFile+=Y2-Y1-3-ModalMode;
      DisplayTree(TRUE);
      if (Opt.Tree.AutoChangeFolder && !ModalMode)
        ProcessKey(KEY_ENTER);
      return(TRUE);
    }

    default:
      if (Key>=KEY_ALT_BASE+0x01 && Key<=KEY_ALT_BASE+255 ||
          Key>=KEY_ALTSHIFT_BASE+0x01 && Key<=KEY_ALTSHIFT_BASE+255)
      {
        FastFind(Key);
        if (Opt.Tree.AutoChangeFolder && !ModalMode)
          ProcessKey(KEY_ENTER);
      }
      else
        break;
      return(TRUE);
  }
  return(FALSE);
}


int TreeList::GetNextNavPos()
{
  int NextPos=CurFile;
  if(CurFile+1 < TreeCount)
  {
    int CurDepth=ListData[CurFile].Depth;
    for(int I=CurFile+1; I < TreeCount; ++I)
      if(ListData[I].Depth == CurDepth)
      {
        NextPos=I;
        break;
      }
  }
  return NextPos;
}

int TreeList::GetPrevNavPos()
{
  int PrevPos=CurFile;
  if(CurFile-1 > 0)
  {
    int CurDepth=ListData[CurFile].Depth;
    for(int I=CurFile-1; I > 0; --I)
      if(ListData[I].Depth == CurDepth)
      {
        PrevPos=I;
        break;
      }
  }
  return PrevPos;
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

void TreeList::Scroll(int Count)
{
  CurFile+=Count;
  CurTopFile+=Count;
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
  xstrncpy(SetDir,NewDir,sizeof(SetDir)-1);
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
        OldFile!=CurFile && Opt.Tree.AutoChangeFolder && !ModalMode)
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
    if (Opt.Tree.AutoChangeFolder && !ModalMode)
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
    if (Opt.Tree.AutoChangeFolder && !ModalMode)
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
    if (!ModalMode && FarChDir(CurPtr->Name))
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
  MkTreeFileName(Root,Name,sizeof(Name)-1);
  if (MustBeCached(Root) || (TreeFile=fopen(Name,"rb"))==NULL)
    if (!GetCacheTreeName(Root,Name,FALSE) || (TreeFile=fopen(Name,"rb"))==NULL)
      return(FALSE);
  /* $ 13.07.2000 SVS
     не надо смешивать new/delete с realloc
  */
  xf_free(ListData);
  /* SVS $ */
  ListData=NULL;
  TreeCount=0;
  *LastDirName=0;
  xstrncpy(DirName,Root,sizeof(DirName)-1);
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
    if ((TreeCount & 255)==0 && (ListData=(struct TreeItem *)xf_realloc(ListData,(TreeCount+256+1)*sizeof(struct TreeItem)))==0)
    {
      /* $ 13.07.2000 SVS
         не надо смешивать new/delete с realloc
      */
      xf_free(ListData);
      /* SVS $ */
      ListData=NULL;
      TreeCount=0;
      fclose(TreeFile);
      return(FALSE);
    }
    xstrncpy(ListData[TreeCount++].Name,DirName,sizeof(ListData[0].Name)-1);
  }
  fclose(TreeFile);

  if (TreeCount==0)
    return(FALSE);
  CaseSensitiveSort=FALSE;
  NumericSort=FALSE;

  FillLastData();
  return(TRUE);
}


int TreeList::FindPartName(char *Name,int Next,int Direct)
{
  char Mask[NM*2];
  xstrncpy(Mask,Name,sizeof(Mask)-1);
  strncat(Mask,"*",sizeof(Mask)-1);
  int I;
  for (I=CurFile+(Next?Direct:0); I >= 0 && I < TreeCount; I+=Direct)
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

  for(
      I=(Direct > 0)?0:TreeCount-1;
      (Direct > 0) ? I < CurFile:I > CurFile;
      I+=Direct
     )
  {
    if (CmpName(Mask,ListData[I].Name,TRUE))
    {
      CurFile=I;
      CurTopFile=CurFile-(Y2-Y1-1)/2;
      DisplayTree(TRUE);
      return(TRUE);
    }
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
  char FullName[NM],Root[NM],*ChPtr;
  long CachePos;

  if (*Name==0)
    return;
  ConvertNameToFull(Name,FullName, sizeof(FullName));
  Name=FullName;
  GetPathRoot(Name,Root);
  Name+=strlen(Root)-1;
  if ((ChPtr=strrchr(Name,'\\'))==NULL)
    return;
  ReadCache(Root);
  for(CachePos=0;CachePos<TreeCache.TreeCount;CachePos++)
  {
    int Result=LCStricmp(TreeCache.ListName[CachePos],Name);
    if(!Result)
      break;
    if(Result > 0)
    {
      TreeCache.Insert(CachePos,Name);
      break;
    }
  }
}


void TreeList::DelTreeName(char *Name)
{
  char *ListName,*NewPtr;
  char FullName[NM],*DirName,Root[NM];
  long CachePos,TreeCount;
  int Length,DirLength;
  if (*Name==0)
    return;
  ConvertNameToFull(Name,FullName, sizeof(FullName));
  Name=FullName;
  GetPathRoot(Name,Root);
  Name+=strlen(Root)-1;
  ReadCache(Root);
  for (CachePos=0;CachePos<TreeCache.TreeCount;CachePos++)
  {
    DirName=TreeCache.ListName[CachePos];
    Length=strlen(Name);
    DirLength=strlen(DirName);
    if(DirLength<Length)continue;
    if (LocalStrnicmp(Name,DirName,Length)==0 &&
        (DirName[Length]==0 || DirName[Length]=='\\'))
    {
      TreeCache.Delete(CachePos);
      CachePos--;
    }
  }
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
    char *DirName=TreeCache.ListName[CachePos];
    if (LocalStrnicmp(SrcName,DirName,SrcLength)==0 &&
        (DirName[SrcLength]==0 || DirName[SrcLength]=='\\'))
    {
      char NewName[2*NM];
      strcpy(NewName,DestName);
      strcat(NewName,DirName+SrcLength);
      //xstrncpy(DirName,NewName,NM-1);
      xf_free(TreeCache.ListName[CachePos]);
      TreeCache.ListName[CachePos]=xf_strdup(NewName);
    }
  }
}


void TreeList::ReadSubTree(char *Path)
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  //SaveScreen SaveScr;
  ScanTree ScTree(FALSE);
  WIN32_FIND_DATA fdata;
  char FullName[NM],DirName[NM];
  int Count=0,FileAttr;

  if ((FileAttr=GetFileAttributes(Path))==-1 || (FileAttr & FA_DIREC)==0)
    return;
  ConvertNameToFull(Path,DirName, sizeof(DirName));
  AddTreeName(DirName);

  int FirstCall=TRUE, AscAbort=FALSE;
  ScTree.SetFindPath(DirName,"*.*",0);
  SetPreRedrawFunc(TreeList::PR_MsgReadTree);
  LastScrX = ScrX;
  LastScrY = ScrY;
  while (ScTree.GetNextName(&fdata,FullName, sizeof (FullName)-1))
  {
    if (fdata.dwFileAttributes & FA_DIREC)
    {
      TreeList::MsgReadTree(Count+1,FirstCall);
      if (CheckForEscSilent())
      {
        AscAbort=ConfirmAbortOp()!=0;
        FirstCall=TRUE;
      }
      if(AscAbort)
        break;
      AddTreeName(FullName);
      ++Count;
    }
  }
  SetPreRedrawFunc(NULL);
}


void TreeList::ClearCache(int EnableFreeMem)
{
  TreeCache.Clean();
}


void TreeList::ReadCache(char *TreeRoot)
{
  char TreeName[NM],DirName[NM],*ChPtr;
  char *ListName;
  FILE *TreeFile=NULL;
  if (strcmp(MkTreeFileName(TreeRoot,TreeName,sizeof(TreeName)-1),TreeCache.TreeName)==0)
    return;
  if (TreeCache.TreeCount!=0)
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
    TreeCache.Add(DirName);
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
      SetFileAttributes(TreeCache.TreeName,FILE_ATTRIBUTE_NORMAL);
    if ((TreeFile=fopen(TreeCache.TreeName,"wb"))==NULL)
    {
      ClearCache(1);
      return;
    }
    far_qsort(TreeCache.ListName,TreeCache.TreeCount,sizeof(char*),SortCacheList);
    for (I=0;I<TreeCache.TreeCount;I++)
      fprintf(TreeFile,"%s\n",TreeCache.ListName[I]);
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


int TreeList::GoToFile(const char *Name,BOOL OnlyPartName)
{
  long Pos=FindFile(Name,OnlyPartName);
  if (Pos!=-1)
  {
    CurFile=Pos;
    CorrectPosition();
    return(TRUE);
  }
  return(FALSE);

}

int TreeList::FindFile(const char *Name,BOOL OnlyPartName)
{
  long I;
  struct TreeItem *CurPtr;

  for (CurPtr=ListData, I=0; I < TreeCount; I++, CurPtr++)
  {
    char *CurPtrName=CurPtr->Name;
    if(OnlyPartName)
      CurPtrName=PointToName(CurPtr->Name);

    if (strcmp(Name,CurPtrName)==0)
      return I;
    if (LocalStricmp(Name,CurPtrName)==0)
      return I;
  }
  return -1;
}

int TreeList::GetFileName(char *Name,int Pos,int &FileAttr)
{
  if (Pos < 0 || Pos >= TreeCount)
    return FALSE;
  if(Name)
    strcpy(Name,ListData[Pos].Name);
  FileAttr=FA_DIREC|GetFileAttributes(ListData[Pos].Name);
  return TRUE;
}

int _cdecl SortList(const void *el1,const void *el2)
{
  char *NamePtr1=((struct TreeItem *)el1)->Name;
  char *NamePtr2=((struct TreeItem *)el2)->Name;
  if(!StaticSortNumeric)
    return(StaticSortCaseSensitive ? TreeCmp(NamePtr1,NamePtr2):LCStricmp(NamePtr1,NamePtr2));
  else
    return(StaticSortCaseSensitive ? TreeCmp(NamePtr1,NamePtr2):LCNumStricmp(NamePtr1,NamePtr2));
}

int _cdecl SortCacheList(const void *el1,const void *el2)
{
//  if(!StaticSortNumeric)
    return(LCStricmp(*(char **)el1,*(char **)el2));
//  else
//    return(LCNumStricmp(*(char **)el1,*(char **)el2));
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

    type=FAR_GetDriveType(Root);
    if ( type==DRIVE_UNKNOWN ||
         type==DRIVE_NO_ROOT_DIR ||
         type==DRIVE_REMOVABLE ||
         IsDriveTypeCDROM(type)
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
  SetTitle();
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

BOOL TreeList::UpdateKeyBar()
{
  KeyBar *KB = CtrlObject->MainKeyBar;
  KB->SetAllGroup (KBL_MAIN, MKBTreeF1, 12);
  KB->SetAllGroup (KBL_SHIFT, MKBTreeShiftF1, 12);
  KB->SetAllGroup (KBL_ALT, MKBTreeAltF1, 12);
  KB->SetAllGroup (KBL_CTRL, MKBTreeCtrlF1, 12);
  KB->ClearGroup (KBL_CTRLSHIFT);
  KB->ClearGroup (KBL_CTRLALT);
  KB->ClearGroup (KBL_ALTSHIFT);

  DynamicUpdateKeyBar();

  return TRUE;
}

void TreeList::DynamicUpdateKeyBar()
{
  ;//KeyBar *KB = CtrlObject->MainKeyBar;
}

void TreeList::SetTitle()
{
  if (GetFocus())
  {
    char TitleDir[NM+30];
    char *Ptr="";
    if(ListData)
    {
      struct TreeItem *CurPtr=ListData+CurFile;
      Ptr=CurPtr->Name;
    }
    if (*Ptr)
      sprintf(TitleDir,"{%.*s - Tree}",NM-1,Ptr);
    else
    {
      sprintf(TitleDir,"{Tree}");
    }
    strcpy(LastFarTitle,TitleDir);
    SetFarTitle(TitleDir);
  }
}

/*
   "Local AppData" = [HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders]/Local AppData
   "AppData"       = [HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders]/AppData

*/

// TODO: Файлы "Tree.Far" для локальных дисков должны храниться в "Local AppData\Far"
// TODO: Файлы "Tree.Far" для сетевых дисков должны храниться в "%HOMEDRIVE%\%HOMEPATH%",
//                        если эти переменные среды не определены, то "%APPDATA%\Far"
// хpаним "X.tree" (где 'X'  - буква диска, если не сетевой путь)
// хpаним "server.share.tree" - для сетевого диска без буквы
char *TreeList::MkTreeFileName(const char *RootDir,char *Dest,int DestSize)
{
  xstrncpy(Dest,RootDir,DestSize-1);
  AddEndSlash(Dest);
  strncat(Dest,"Tree.Far",DestSize);
  return Dest;
}

// TODO: этому каталогу (Tree.Cache) место не в FarPath, а в "Local AppData\Far\"
char *TreeList::MkTreeCacheFolderName(const char *RootDir,char *Dest,int DestSize)
{
  xstrncpy(Dest,RootDir,DestSize-1);
  AddEndSlash(Dest);
  strncat(Dest,"Tree.Cache",DestSize);
  return Dest;
}


/*
  Opt.Tree.LocalDisk
  Opt.Tree.NetDisk
  Opt.Tree.NetPath
  Opt.Tree.RemovableDisk
  Opt.Tree.CDROM
  Opt.Tree.SavedTreePath

   локальных дисков - "X.nnnnnnnn.tree"
   сетевых дисков - "X.nnnnnnnn.tree"
   сетевых путей - "Server.share.tree"
   сменных дисков(DRIVE_REMOVABLE) - "Far.nnnnnnnn.tree"
   сменных дисков(CD) - "Label.nnnnnnnn.tree"

*/
char * TreeList::CreateTreeFileName(const char *Path,char *Dest,int DestSize)
{
#if 0
  char RootPath[NM];
  GetPathRoot(Path,RootPath);
  UINT DriveType = FAR_GetDriveType(RootPath,NULL,FALSE);

  // получение инфы о томе
  char VolumeName[NM],FileSystemName[NM];
  DWORD MaxNameLength,FileSystemFlags,VolumeNumber;
  if (!GetVolumeInformation(RootDir,VolumeName,sizeof(VolumeName),&VolumeNumber,
                            &MaxNameLength,&FileSystemFlags,
                            FileSystemName,sizeof(FileSystemName)))
  Opt.Tree.SavedTreePath
#endif
  return Dest;
}

BOOL TreeList::GetItem(int Index,void *Dest)
{
  if((DWORD)Index >= TreeCount)
    return FALSE;
  memcpy(Dest,ListData+Index,sizeof(struct TreeItem));
  return TRUE;
}
