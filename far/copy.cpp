/*
copy.cpp

Копирование файлов

*/

/* Revision: 1.30 06.05.2001 $ */

/*
Modify:
  06.05.2001 DJ
    ! перетрях #include
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  24.04.2001 VVM
    ! При подсчете времени копирования игнорировать время простоя (в диалогах)
  08.04.2001 SVS
    ! Создание линков только под NT - о чем честно и предупредим.
    + Отладочный код для создания repase point. Просьба в нем пока не
      ковыряться - внес сейчас, т.к. тяжко каждый раз copy.cpp перелапачивать.
      Этот код сейчас отключен!
  05.04.2001 IS
    + Если при копировании/перемещении выделенных элементов больше 1 и среди
      них есть каталог, то всегда делаем так, чтобы на конце был '\\', чтобы
      не было все свалено в кучу :(
  14.03.2001 SVS
    + Зарезервирован кусок кода для создания SymLink для каталогов.
  12.03.2001 SVS
    ! Коррекция в связи с изменениями в классе int64
  06.03.2001 SVS
    ! Немного оптимизации в "индикаторе скорости" + добавка для 'Gb'
  27.02.2001 VVM
    ! Символы, зависимые от кодовой страницы
      /[\x01-\x08\x0B-\x0C\x0E-\x1F\xB0-\xDF\xF8-\xFF]/
      переведены в коды.
  12.02.2001 VVM
    - При включенном TotalCopyIndicator и показе CPS размер показывается 1 раз :)
  06.02.2001 SKV
    - char SelNameShort[NM];, а не char SelNameShort[40];
  30.01.2001 VVM
    + Показывает время копирования,оставшееся время и среднюю скорость.
      Зависит от настроек в реестре CopyTimeRule
  25.01.2001 IS
    - Не очень правильно обрезалось имя в диалоге копирования/перемещения
  17.01.2001 SVS
    ! Не для NT - задисаблим опцию про копирование права.
  01.01.2001 VVM
    + Размер буфера для копирования берется из реестра.
  30.12.2000 SVS
    - При копировании/переносе забыли выставить FILE_ATTRIBUTE_ENCRYPTED
      для каталога, если он есть
    ! Опция "[ ] Только новые/обновленные файлы" теперь не "прячется", а
      просто задисаблена
  14.12.2000 SVS
    + Copy to "NUL"
  03.11.2000 OT
    ! Введение проверки возвращаемого значения
  02.11.2000 OT
    ! Введение проверки на длину буфера, отведенного под имя файла.
  23.10.2000 VVM
    + Динамический буфер копирования
  21.10.2000 SVS
    + Проинициализируем новую переменную Copy_Buffer_Size в стандартное
      значение = 64К
  11.10.2000 SVS
    ! Если Dest-панель плагиновая, то не показываем "[ ] Only never..."
  14.08.2000 SVS
    ! Данные, усеченные до 40 символов... :-(
      А потом используются (CopyDlg[2].Data) по полной программе...
  09.08.2000 KM
    ! Добавлена проверка на режим перемещения диалога
      в ShellCopy::ShellCopy, чтобы избежать в этом режиме
      возможность вызова дерева каталогов.
  04.08.2000 SVS
    + Опция "Only newer file(s)"
  01.08.2000 tran 1.05
    + DIF_USELASTHISTORY в диалогах
  31.07.2000 SVS
    + Расширение переменных среды для строки ввода в диалоге Copy/Move!
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  03.07.2000 IS
    ! Показывать проценты спереди при копировании/переносе
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/


#include "headers.hpp"
#pragma hdrstop

#include "copy.hpp"
#include "global.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "foldtree.hpp"
#include "treelist.hpp"
#include "chgprior.hpp"
#include "scantree.hpp"
#include "savescr.hpp"

#define COPY_BUFFER_SIZE 0x10000

/* $ 30.01.2001 VVM
   + Константы для правил показа
   + Рабочие перменные */
#define COPY_RULE_NUL   0x0001;
#define COPY_RULE_FILES 0x0002;

static int ShowCopyTime;
static clock_t CopyStartTime;
static clock_t CopyTime;
static clock_t LastShowTime;
/* VVM $ */

static DWORD WINAPI CopyProgressRoutine(LARGE_INTEGER TotalFileSize,
       LARGE_INTEGER TotalBytesTransferred,LARGE_INTEGER StreamSize,
       LARGE_INTEGER StreamBytesTransferred,DWORD dwStreamNumber,
       DWORD dwCallbackReason,HANDLE hSourceFile,HANDLE hDestinationFile,
       LPVOID lpData);

static int BarX,BarY,BarLength;

static int64 TotalCopySize,CurCopySize;
static bool ShowTotalCopySize;
static int StaticMove;
static char TotalCopySizeText[32];

static int64 StartCopySizeEx;

#if defined(CREATE_JUNCTION)

long WINAPI CopyDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);

struct CopyDlgParam {
  int    Link;
  Panel *SrcPanel;
};

#endif

/* $ 04.07.2000 SVS
  "Только новые/обновленные файлы"
*/
ShellCopy::ShellCopy(Panel *SrcPanel,int Move,int Link,int CurrentOnly,int Ask,
                     int &ToPlugin,char *PluginDestPath)
{
  // Создание линков только под NT
  if(Link && WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT)
  {
    Message(MSG_DOWN|MSG_WARNING,1,MSG(MWarning),
              MSG(MCopyNotSupportLink1),
              MSG(MCopyNotSupportLink2),
              MSG(MOk));
    return;
  }

  const char *HistoryName="Copy";
  static struct DialogData CopyDlgData[]={
  /* 00 */  DI_DOUBLEBOX,3,1,72,9,0,0,0,0,(char *)MCopyDlgTitle,
  /* 01 */  DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,"",
            /* $ 01.08.2000 tran
             + |DIF_USELASTHISTORY :) */
            /* $ 31.07.2000 SVS
               + Расширение переменных среды!
            */
  /* 02 */  DI_EDIT,5,3,70,3,1,(DWORD)HistoryName,DIF_HISTORY|DIF_EDITEXPAND|DIF_USELASTHISTORY,0,"",
            /* SVS $ */
            /* tran 01.08.2000 $ */
  /* 03 */  DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 04 */  DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)MCopySecurity,
  /* 05 */  DI_CHECKBOX,5,6,0,0,0,0,0,0,(char *)MCopyOnlyNewerFiles,
  /* 06 */  DI_TEXT,3,7,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 07 */  DI_BUTTON,0,8,0,0,0,0,DIF_CENTERGROUP,1,(char *)MCopyDlgCopy,
  /* 08 */  DI_BUTTON,0,8,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCopyDlgTree,
  /* 09 */  DI_BUTTON,0,8,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCopyDlgCancel
  };
  MakeDialogItems(CopyDlgData,CopyDlg);

  char CopyStr[100],SelName[NM],DestDir[NM],InitDestDir[NM];
  /* $ 25.01.2001 IS
      Будет содержать усеченное имя файла, которое покажем в диалоге
      копирования
  */
  /*$ 06.02.2001 SKV
    едрёна вошь!
    почему 40 то???
    А потом туда SelFile strcpy делаем.
  */
  char SelNameShort[NM];
  /* SKV$*/
  /* IS $ */
  int SelCount,DestPlugin;

  *SelName=0;

  ShowTotalCopySize=Opt.CopyShowTotal != 0;


/* $ 06.01.2001 VVM
   Размер буфера берется из реестра */
  GetRegKey("System", "CopyBufferSize", CopyBufferSize, COPY_BUFFER_SIZE);
  if (CopyBufferSize == 0)
    CopyBufferSize = COPY_BUFFER_SIZE;
/* VVM $ */

  DestPlugin=ToPlugin;
  ToPlugin=FALSE;
  ShellCopy::SrcPanel=SrcPanel;
  AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
  PanelMode=DestPlugin ? AnotherPanel->GetMode():NORMAL_PANEL;
  SrcPanelMode=SrcPanel->GetMode();

  strcpy(CopyDlg[5].Data,MSG(MCopyOnlyNewerFiles));
  /* $ 11.10.2000 SVS
     Если противоположная панель - плагин, то не показываем
     "[ ] Только новые/обновленные файлы"
  */
  /* $ 30.12.2000 SVS
     нефига от народа прятать эту опцию - мы ее просто задисаблим.
  */
  if(PanelMode == PLUGIN_PANEL)
    CopyDlg[5].Flags|=DIF_DISABLE;
  /* SVS $ */
  /* SVS $ */

  CopyBuffer=NULL;
  *RenamedName=*CopiedName=0;
  SelectedFolderNameLength=0;

  *SrcDriveRoot=0;
  SrcDriveType=0;

  ShellCopy::Move=Move;
  ShellCopy::Link=Link;
  ShellCopy::CurrentOnly=CurrentOnly;
  TotalFiles=0;
  DizRead=0;

  if ((SelCount=SrcPanel->GetSelCount())==0)
    return;

  StaticMove=Move;

  if (Move)
  {
    strcpy(CopyDlg[0].Data,MSG(MMoveDlgTitle));
    strcpy(CopyDlg[7].Data,MSG(MCopyDlgRename));
  }
  else
    if (Link)
    {
      strcpy(CopyDlg[0].Data,MSG(MLinkDlgTitle));
      strcpy(CopyDlg[7].Data,MSG(MCopyDlgLink));
      // покажем...
      strcpy(CopyDlg[5].Data,MSG(MCopySymLink));
#if defined(CREATE_JUNCTION)
      if(WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT ||
         WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion < 5)
#endif
      {
        CopyDlg[5].Flags|=DIF_DISABLE;
        CopyDlg[5].Selected=0;
      }
    }

  if (SelCount==1)
  {
    int FileAttr;
    SrcPanel->GetSelName(NULL,FileAttr);
    SrcPanel->GetSelName(SelName,FileAttr);

    if (SrcPanel->GetType()==TREE_PANEL)
    {
      char NewDir[NM],*ChPtr;
      strcpy(NewDir,SelName);
      if ((ChPtr=strrchr(NewDir,'\\'))!=0)
      {
        *ChPtr=0;
        if (ChPtr==NewDir || *(ChPtr-1)==':')
          strcat(NewDir,"\\");
        chdir(NewDir);
      }
    }

    if (strcmp(SelName,"..")==NULL)
      return;
    /* $ 25.01.2001 IS
       Урезаем имя до размеров диалога.
    */
    strcpy(SelNameShort,SelName);
    TruncPathStr(SelNameShort,33);
    if (Move)
      sprintf(CopyStr,MSG(MMoveFile),SelNameShort);
    else
      if (Link)
        sprintf(CopyStr,MSG(MLinkFile),SelNameShort);
      else
        sprintf(CopyStr,MSG(MCopyFile),SelNameShort);
    /* IS $ */
  }
  else
    if (Move)
      sprintf(CopyStr,MSG(MMoveFiles),SelCount);
    else
      if (Link)
        sprintf(CopyStr,MSG(MLinkFiles),SelCount);
      else
        sprintf(CopyStr,MSG(MCopyFiles),SelCount);

  AnotherPanel->GetCurDir(DestDir);

  sprintf(CopyDlg[1].Data,"%.65s",CopyStr);
  if (CurrentOnly)
    strcpy(CopyDlg[2].Data,SelName);
  else
    switch(PanelMode)
    {
      case NORMAL_PANEL:
        if ((*DestDir==0 || !AnotherPanel->IsVisible()) && SelCount==1)
          strcpy(CopyDlg[2].Data,SelName);
        else
        {
          strcpy(CopyDlg[2].Data,DestDir);
          AddEndSlash(CopyDlg[2].Data);
        }
        break;
      case PLUGIN_PANEL:
        {
          struct OpenPluginInfo Info;
          AnotherPanel->GetOpenPluginInfo(&Info);
          /* $ 14.08.2000 SVS
             Данные, усеченные до 40 символов... :-(
             А потом используются (CopyDlg[2].Data) по полной программе...
             "%.40s:" -> "%s:"
          */
          sprintf(CopyDlg[2].Data,"%s:",NullToEmpty(Info.Format));
          /* SVS $ */
          while (strlen(CopyDlg[2].Data)<2)
            strcat(CopyDlg[2].Data,":");
        }
        break;
    }
  strcpy(InitDestDir,CopyDlg[2].Data);
  /* $ 17.01.2001 SVS
     Не для NT - задисаблим опцию про копирование права.
  */
  if(Link)
  {
    CopyDlg[4].Selected=1;
    CopyDlg[4].Flags|=DIF_DISABLE;
  }

#if !defined(CREATE_JUNCTION)
  if(WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT)
    CopyDlg[4].Flags|=DIF_DISABLE;
  /* SVS $ */
#endif

  if (Ask)
  {
#if defined(CREATE_JUNCTION)
    struct CopyDlgParam CDP;
    CDP.Link=Link;
    CDP.SrcPanel=SrcPanel;
    Dialog Dlg(CopyDlg,sizeof(CopyDlg)/sizeof(CopyDlg[0]),CopyDlgProc,(long)&CDP);
    Dlg.SetHelp(Link?"HardSymLink":"CopyFiles");
    Dlg.SetPosition(-1,-1,76,11);

    Dlg.Show();
    Dlg.Process();
    if(Dlg.GetExitCode() != 7)
    {
      if (DestPlugin)
        ToPlugin=-1;
      return;
    }
#else
    Dialog Dlg(CopyDlg,sizeof(CopyDlg)/sizeof(CopyDlg[0]));
    Dlg.SetHelp("CopyFiles");
    Dlg.SetPosition(-1,-1,76,11);
    while (1)
    {
      int AltF10=FALSE;
      Dlg.Show();
      while (!Dlg.Done())
      {
        int Key=Dlg.ReadInput();
        /* $ 09.08.2000 KM
           Добавление проверки на режим перемещения
           диалога, чтобы отменить в этом режиме работу дерева
           каталогов, а то как-то некузяво выходит - диалог
           перемещается, а дерево вызывается.
        */
        if (Dlg.IsMoving())
        {
          Dlg.ProcessInput();
          continue;
        }
        /* KM $ */
        switch(Key)
        {
          case KEY_F10:
            Dlg.SetExitCode(8);
            break;
          case KEY_ALTF10:
            AltF10=TRUE;
            Dlg.SetExitCode(8);
            break;
          default:
            Dlg.ProcessInput();
            break;
        }
      }
      int ExitCode=Dlg.GetExitCode();
      if (ExitCode==7)
        break;
      if (ExitCode==8)
      {
        char NewFolder[NM];
        {
          FolderTree Tree(NewFolder,AltF10 ? MODALTREE_PASSIVE:MODALTREE_ACTIVE,25,2,ScrX-7,ScrY-5);
        }
        if (*NewFolder)
        {
          strcpy(CopyDlg[2].Data,NewFolder);
          AddEndSlash(CopyDlg[2].Data);
          CopyDlg[8].Focus=0;
          CopyDlg[2].Focus=1;
          Dlg.InitDialogObjects();
        }
        Dlg.ClearDone();
      }
      if (ExitCode<0 || ExitCode==9)
      {
        if (DestPlugin)
          ToPlugin=-1;
        return;
      }
    }
#endif
  }

  CopySecurity=CopyDlg[4].Selected;
  OnlyNewerFiles=CopyDlg[5].Selected;

  if (DestPlugin && strcmp(CopyDlg[2].Data,InitDestDir)==0)
  {
    ToPlugin=TRUE;
    return;
  }

  Unquote(CopyDlg[2].Data);
  RemoveTrailingSpaces(CopyDlg[2].Data);

  /* $ 14.12.2000 SVS
     Выставляем признак копирования в NUL
  */
  CopyToNUL=stricmp(CopyDlg[2].Data,"nul") == 0;
  /* SVS $ */

  /* $ 05.04.2001 IS
     Если выделенных элементов больше 1 и среди них есть каталог, то всегда
     делаем так, чтобы на конце был '\\'
  */
  if(SelCount>1 && !CopyToNUL)
  {
    int FileAttr, AddSlash=FALSE;
    SrcPanel->GetSelName(NULL,FileAttr);
    while(SrcPanel->GetSelName(SelName,FileAttr))
    {
      if(FileAttr & FA_DIREC)
      {
        AddSlash=TRUE;
        break;
      }
    }

    if(AddSlash) AddEndSlash(CopyDlg[2].Data);
  }
  /* IS $ */
  if (DestPlugin==2)
  {
    if (*PluginDestPath!=NULL)
      strcpy(PluginDestPath,CopyDlg[2].Data);
    return;
  }

  if (Opt.Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && SrcPanel->IsDizDisplayed() ||
      Opt.Diz.UpdateMode==DIZ_UPDATE_ALWAYS)
  {
    CtrlObject->Cp()->LeftPanel->ReadDiz();
    CtrlObject->Cp()->RightPanel->ReadDiz();
  }

  CopyBuffer=new char[CopyBufferSize];
  AnotherPanel->CloseFile();
  *DestDizPath=0;
  SrcPanel->SaveSelection();

  char OldTitle[512];
  GetConsoleTitle(OldTitle,sizeof(OldTitle));

  ShowTitle(TRUE);
  SetFarTitle(Move ? MSG(MCopyMovingTitle):MSG(MCopyCopyingTitle));
  for (int I=0;CopyDlg[2].Data[I]!=0;I++)
    if (CopyDlg[2].Data[I]=='/')
      CopyDlg[2].Data[I]='\\';
  if (SelCount==1 && Move && strpbrk(CopyDlg[2].Data,":\\")==NULL)
    ShowTotalCopySize=FALSE;

/* $ 30.01.2001 VVM
    + Определем, нужно ли показывать время копирования
      Зависит от настроек в реестре CopyTimeRule */
  if (CopyToNUL) {
    ShowCopyTime = Opt.CopyTimeRule & COPY_RULE_NUL; }
  else {
    ShowCopyTime = Opt.CopyTimeRule & COPY_RULE_FILES; }
/* VVM $ */

  CopyFileTree(CopyDlg[2].Data);

  SetConsoleTitle(OldTitle);
  if (!CopyToNUL && *DestDizPath)
  {
    char DestDizName[NM];
    DestDiz.GetDizName(DestDizName);
    DWORD Attr=GetFileAttributes(DestDizName);
    int DestReadOnly=(Attr!=0xffffffff && (Attr & FA_RDONLY));
    if (Move && !DestReadOnly)
      SrcPanel->FlushDiz();
    DestDiz.Flush(DestDizPath);
  }
  SrcPanel->Update(UPDATE_KEEP_SELECTION);
  if (SelCount==1 && *RenamedName)
    SrcPanel->GoToFile(RenamedName);
  SrcPanel->Redraw();
  AnotherPanel->SortFileList(TRUE);
  AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
  AnotherPanel->Redraw();
}
/* SVS $ */

#if defined(CREATE_JUNCTION)
static long WINAPI CopyDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  static struct CopyDlgParam *DlgParam;
  static int AltF10=FALSE;

  switch(Msg)
  {
    case DN_INITDIALOG:
    {
      DlgParam=(struct CopyDlgParam *)Param2;
      break;
    }

    case DN_BTNCLICK:
    {
      if(Param1 == 8)
      {
        char NewFolder[NM];
        {
          FolderTree Tree(NewFolder,AltF10 ? MODALTREE_PASSIVE:MODALTREE_ACTIVE,25,2,ScrX-7,ScrY-5);
        }
        if (*NewFolder)
        {
          AddEndSlash(NewFolder);
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(long)NewFolder);
          Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,2,0);
        }
        AltF10=FALSE;
        return FALSE;
      }
      else if(Param1 == 7)
      {
        Dialog::SendDlgMessage(hDlg,DM_CLOSE,7,0);
      }
      break;
    }

    case DM_KEY:
    {
      if(Param2 == KEY_ALTF10 || Param2 == KEY_F10)
      {
        AltF10=Param2 == KEY_ALTF10;
        Dialog::SendDlgMessage(hDlg,DN_BTNCLICK,8,0);
        return TRUE;
      }
      return FALSE;
    }

    case DN_EDITCHANGE:
    {
      break;
    }
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}
/*
{
  char Root[1024],FSysName[NM];

  DWORD FileSystemFlags;=0;

  GetPathRoot(Path,Root);
  GetVolumeInformation(Root,NULL,0,NULL,NULL,&FileSystemFlags,FSysName,sizeof(FSysName));

  //   Не для NT - задисаблим опцию про копирование права.
  if(!(FileSystemFlags&FS_PERSISTENT_ACLS))
  {
    CopyDlg[4].Flags|=DIF_DISABLE;
    CopyDlg[4].Selected=0;
  }
  // Дисаблим
  CopyDlg[7].Flags|=DIF_DISABLE;
}
*/
#endif

ShellCopy::~ShellCopy()
{
  /* $ 13.07.2000 SVS
       раз уж вызвали new[], то в придачу и delete[] надо... */
  delete[] CopyBuffer;
  /* SVS $ */
}


void ShellCopy::CopyFileTree(char *Dest)
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  SaveScreen SaveScr;
  DWORD DestAttr;
  char SelName[NM],SelShortName[NM];
  int Length,FileAttr;

  CopyBufSize=1024; // Начинаем с 1к
  ReadOnlyDelMode=ReadOnlyOvrMode=OvrMode=-1;
  SetCursorType(FALSE,0);

  *TotalCopySizeText=0;
  if (ShowTotalCopySize && !CalcTotalSize())
    return;

/* $ 30.01.2001 VVM
    + Запомним время начала. */
  if (ShowCopyTime)
    CopyStartTime = clock();
  CopyTime = 0;
  LastShowTime = 0;
/* VVM $ */

  ShellCopyMsg("","",MSG_LEFTALIGN);

  if ((Length=strlen(Dest))==0 || strcmp(Dest,".")==0)
    return;
  if(!CopyToNUL)
  {
    if (Length>1 && Dest[Length-1]=='\\' && Dest[Length-2]!=':')
    {
      Dest[Length-1]=0;
      char NewPath[NM];
      strcpy(NewPath,Dest);
      if (Opt.CreateUppercaseFolders && !IsCaseMixed(NewPath))
        LocalStrupr(NewPath);
      DWORD Attr=GetFileAttributes(NewPath);
      if (Attr==0xFFFFFFFF)
        if (CreateDirectory(NewPath,NULL))
          TreeList::AddTreeName(NewPath);
        else
          CreatePath(NewPath);
      else
        if ((Attr & FA_DIREC)==0)
        {
          Message(MSG_DOWN|MSG_WARNING,1,MSG(MError),MSG(MCopyCannotCreateFolder),NewPath,MSG(MOk));
          return;
        }
    }

    DestAttr=GetFileAttributes(Dest);
  }

  int SameDisk=FALSE;
  if (Move)
  {
    char SrcDir[NM];
    SrcPanel->GetCurDir(SrcDir);
    SameDisk=IsSameDisk(SrcDir,Dest);
  }

  SrcPanel->GetSelName(NULL,FileAttr);
  while (SrcPanel->GetSelName(SelName,FileAttr,SelShortName))
  {
    if (!CopyToNUL && strpbrk(Dest,"*?")!=NULL)
    {
      char FullDest[NM];
      strcpy(FullDest,Dest);
      ShellCopyConvertWildcards(SelName,FullDest);
      DestAttr=GetFileAttributes(FullDest);
    }

    char DestPath[NM];
    strcpy(DestPath,Dest);
    HANDLE FindHandle;
    WIN32_FIND_DATA SrcData;
    int CopyCode,KeepPathPos;

    OverwriteNext=FALSE;

    if (*SrcDriveRoot==0 || LocalStrnicmp(SelName,SrcDriveRoot,strlen(SrcDriveRoot))!=0)
    {
      GetPathRoot(SelName,SrcDriveRoot);
      SrcDriveType=GetDriveType(strchr(SelName,'\\')!=NULL ? SrcDriveRoot:NULL);
    }

    if (FileAttr & FA_DIREC)
      SelectedFolderNameLength=strlen(SelName);
    else
      SelectedFolderNameLength=0;

    if ((FindHandle=FindFirstFile(SelName,&SrcData))==INVALID_HANDLE_VALUE)
    {
      CopyTime+= (clock() - CopyStartTime);
      if (Message(MSG_DOWN|MSG_WARNING,2,MSG(MError),MSG(MCopyCannotFind),
              SelName,MSG(MSkip),MSG(MCancel))==1)
        return;
      CopyStartTime = clock();
      int64 SubSize(SrcData.nFileSizeHigh,SrcData.nFileSizeLow);
      TotalCopySize-=SubSize;
      continue;
    }

    FindClose(FindHandle);

    KeepPathPos=PointToName(SelName)-SelName;

    if (Move)
    {
      if (KeepPathPos!=0 && PointToName(Dest)==Dest)
      {
        strcpy(DestPath,SelName);
        strcpy(DestPath+KeepPathPos,Dest);
        SameDisk=TRUE;
      }
      if (!SameDisk)
        CopyCode=COPY_FAILURE;
      else
      {
        CopyCode=ShellCopyOneFile(SelName,&SrcData,DestPath,KeepPathPos,1);
        if (CopyCode==COPY_SUCCESS_MOVE)
        {
          if (*DestDizPath)
            if (*RenamedName)
            {
              DestDiz.DeleteDiz(SelName,SelShortName);
              SrcPanel->CopyDiz(SelName,SelShortName,RenamedName,RenamedName,&DestDiz);
            }
            else
            {
              if (*CopiedName==0)
                strcpy(CopiedName,SelName);
              SrcPanel->CopyDiz(SelName,SelShortName,CopiedName,CopiedName,&DestDiz);
              SrcPanel->DeleteDiz(SelName,SelShortName);
            }
          continue;
        }
        if (CopyCode==COPY_CANCEL)
          return;
        if (CopyCode==COPY_NEXT)
        {
          int64 SubSize(SrcData.nFileSizeHigh,SrcData.nFileSizeLow);
          TotalCopySize-=SubSize;
          continue;
        }
        if (!Move || CopyCode==COPY_FAILURE)
          OverwriteNext=TRUE;
      }
    }

    if (!Move || CopyCode==COPY_FAILURE)
    {
      CopyCode=ShellCopyOneFile(SelName,&SrcData,Dest,KeepPathPos,0);
      OverwriteNext=FALSE;
      if (CopyCode==COPY_CANCEL)
        return;
      if (CopyCode!=COPY_SUCCESS)
      {
        int64 SubSize(SrcData.nFileSizeHigh,SrcData.nFileSizeLow);
        TotalCopySize-=SubSize;
        continue;
      }
    }

    if (CopyCode==COPY_SUCCESS && !CopyToNUL && *DestDizPath)
    {
      if (*CopiedName==0)
        strcpy(CopiedName,SelName);
      SrcPanel->CopyDiz(SelName,SelShortName,CopiedName,CopiedName,&DestDiz);
    }
#if defined(CREATE_JUNCTION)
    // Кусок для создания SymLink для каталогов.
    if (Link && OnlyNewerFiles && (SrcData.dwFileAttributes & FA_DIREC))
    {
      char SrcFullName[NM], DestFullName[NM];
      if (ConvertNameToFull(SelName,SrcFullName, sizeof(SrcFullName)) >= sizeof(SrcFullName))
        return;
      if (ConvertNameToFull(Dest,DestFullName, sizeof(DestFullName)) >= sizeof(DestFullName))
        return;
      AddEndSlash(DestFullName);
      strcat(DestFullName,SelName);
      if(CreateJunctionPoint(SrcFullName,DestFullName))
        continue;
      else
        return;
    }
    else
#endif
    if (SrcData.dwFileAttributes & FA_DIREC)
    {
      int SubCopyCode;
      char SubName[NM],FullName[NM];
      ScanTree ScTree(TRUE);

      strcpy(SubName,SelName);
      strcat(SubName,"\\");
      if (DestAttr==(DWORD)-1)
        KeepPathPos=strlen(SubName);
      ScTree.SetFindPath(SubName,"*.*");
      while (ScTree.GetNextName(&SrcData,FullName))
      {
        int AttemptToMove=FALSE;
        if (Move && SameDisk && (SrcData.dwFileAttributes & FA_DIREC)==0)
        {
          AttemptToMove=TRUE;
          int SubMoveCode=ShellCopyOneFile(FullName,&SrcData,Dest,KeepPathPos,1);
          if (SubMoveCode==COPY_CANCEL)
            return;
          if (SubMoveCode==COPY_NEXT)
          {
            int64 SubSize(SrcData.nFileSizeHigh,SrcData.nFileSizeLow);
            TotalCopySize-=SubSize;
            continue;
          }
          if (SubMoveCode==COPY_SUCCESS_MOVE)
            continue;
        }
        int SaveOvrMode;
        if (AttemptToMove)
        {
          SaveOvrMode=OvrMode;
          OvrMode=1;
        }
        SubCopyCode=ShellCopyOneFile(FullName,&SrcData,Dest,KeepPathPos,0);
        if (AttemptToMove)
          OvrMode=SaveOvrMode;
        if (SubCopyCode==COPY_CANCEL)
          return;
        if (SubCopyCode==COPY_NEXT)
        {
          int64 SubSize(SrcData.nFileSizeHigh,SrcData.nFileSizeLow);
          TotalCopySize-=SubSize;
        }
        if (Move && SubCopyCode==COPY_SUCCESS)
          if (SrcData.dwFileAttributes & FA_DIREC)
          {
            if (ScTree.IsDirSearchDone())
            {
              if (SrcData.dwFileAttributes & FA_RDONLY)
                SetFileAttributes(FullName,0);
              if (RemoveDirectory(FullName))
                TreeList::DelTreeName(FullName);
            }
          }
          else
            if (DeleteAfterMove(FullName,SrcData.dwFileAttributes)==COPY_CANCEL)
              return;
      }
      if (Move && CopyCode==COPY_SUCCESS)
      {
        if (FileAttr & FA_RDONLY)
          SetFileAttributes(SelName,0);
        if (RemoveDirectory(SelName))
        {
          TreeList::DelTreeName(SelName);
          if (*DestDizPath)
            SrcPanel->DeleteDiz(SelName,SelShortName);
        }
      }
    }
    else
      if (Move && CopyCode==COPY_SUCCESS)
      {
        int DeleteCode;
        if ((DeleteCode=DeleteAfterMove(SelName,FileAttr))==COPY_CANCEL)
          return;
        if (DeleteCode==COPY_SUCCESS && *DestDizPath)
          SrcPanel->DeleteDiz(SelName,SelShortName);
      }
    if (!CurrentOnly)
      SrcPanel->ClearLastGetSelection();
  }
}


COPY_CODES ShellCopy::ShellCopyOneFile(char *Src,WIN32_FIND_DATA *SrcData,
                                       char *Dest,int KeepPathPos,int Rename)
{
  char DestPath[2*NM];
  DWORD DestAttr;
  HANDLE FindHandle;
  WIN32_FIND_DATA DestData;
  int SameName=0,Append=0;

  *RenamedName=*CopiedName=0;

  if (CheckForEsc())
    return(COPY_CANCEL);

  strcpy(DestPath,Dest);

  ShellCopyConvertWildcards(Src,DestPath);

  char *NamePtr=PointToName(DestPath);

  DestAttr=-1;

  if (DestPath[0]=='\\' && DestPath[1]=='\\')
  {
    char Root[NM];
    GetPathRoot(DestPath,Root);
    int RootLength=strlen(Root);
    if (RootLength>0 && Root[RootLength-1]=='\\')
      Root[RootLength-1]=0;
    if (strcmp(DestPath,Root)==0)
      DestAttr=FA_DIREC;
  }

  if (*NamePtr==0 || strcmp(NamePtr,"..")==0)
    DestAttr=FA_DIREC;

  *DestData.cFileName=0;

  if (DestAttr==(DWORD)-1 && (FindHandle=FindFirstFile(DestPath,&DestData))!=INVALID_HANDLE_VALUE)
  {
    FindClose(FindHandle);
    DestAttr=DestData.dwFileAttributes;
  }


  if (DestAttr!=(DWORD)-1 && (DestAttr & FA_DIREC))
  {
    int CmpCode;
    if ((CmpCode=CmpFullNames(Src,DestPath))!=0)
    {
      SameName=1;
      if (CmpCode==2 || !Rename)
      {
        CopyTime+= (clock() - CopyStartTime);
        Message(MSG_DOWN|MSG_WARNING,1,MSG(MError),MSG(MCannotCopyFolderToItself1),
                Src,MSG(MCannotCopyFolderToItself2),MSG(MOk));
        CopyStartTime = clock();
        return(COPY_CANCEL);
      }
    }

    if (!SameName)
    {
      char *PathPtr;
      int Length=strlen(DestPath);
      if (DestPath[Length-1]!='\\' && DestPath[Length-1]!=':')
        strcat(DestPath,"\\");
      PathPtr=Src+KeepPathPos;
      if (*PathPtr && KeepPathPos==0 && PathPtr[1]==':')
        PathPtr+=2;
      if (*PathPtr=='\\')
        PathPtr++;
      strcat(DestPath,PathPtr);
      if ((FindHandle=FindFirstFile(DestPath,&DestData))==INVALID_HANDLE_VALUE)
        DestAttr=-1;
      else
      {
        FindClose(FindHandle);
        DestAttr=DestData.dwFileAttributes;
      }
    }
  }

  if (stricmp(DestPath,"nul")!=0 && stricmp(DestPath,"prn")!=0)
    SetDestDizPath(DestPath);

  ShellCopyMsg(Src,DestPath,MSG_LEFTALIGN|MSG_KEEPBACKGROUND);

  if(!CopyToNUL)
  {
    if (SrcData->dwFileAttributes & FA_DIREC)
    {
      if (!Rename)
        strcpy(CopiedName,PointToName(DestPath));
      if (DestAttr!=(DWORD)-1)
      {
        if ((DestAttr & FA_DIREC) && !SameName)
        {
          DWORD SetAttr=SrcData->dwFileAttributes;
          if (SrcDriveType==DRIVE_CDROM && Opt.ClearReadOnly && (SetAttr & FA_RDONLY))
            SetAttr&=~FA_RDONLY;
          if (SetAttr!=DestAttr)
            SetFileAttributes(DestPath,SetAttr);
          char SrcFullName[NM];
          if (ConvertNameToFull(Src,SrcFullName, sizeof(SrcFullName)) >= sizeof(SrcFullName)){
            return(COPY_NEXT);
          }
          return(strcmp(DestPath,SrcFullName)==0 ? COPY_NEXT:COPY_SUCCESS);
        }
        int Type=GetFileTypeByName(DestPath);
        if (Type==FILE_TYPE_CHAR || Type==FILE_TYPE_PIPE)
          return(Rename ? COPY_NEXT:COPY_SUCCESS);
      }

      if (Rename)
      {
        char SrcFullName[NM],DestFullName[NM];
        if (ConvertNameToFull(Src,SrcFullName, sizeof(SrcFullName)) >= sizeof(SrcFullName)){
          return(COPY_NEXT);
        }
        if (MoveFile(Src,DestPath))
        {
          if (PointToName(DestPath)==DestPath)
            strcpy(RenamedName,DestPath);
          else
            strcpy(CopiedName,PointToName(DestPath));
//          ConvertNameToFull(DestPath,DestFullName, sizeof(DestFullName));
          if (ConvertNameToFull(Dest,DestFullName, sizeof(DestFullName)) >= sizeof(DestFullName)){
            return(COPY_NEXT);
          }
          TreeList::RenTreeName(SrcFullName,DestFullName);
          return(SameName ? COPY_NEXT:COPY_SUCCESS_MOVE);
        }
        else
          return(COPY_FAILURE);
      }

      SECURITY_ATTRIBUTES sa;
      if (CopySecurity && !GetSecurity(Src,&sa))
        return(COPY_CANCEL);
      while (!CreateDirectory(DestPath,CopySecurity ? &sa:NULL))
      {
        int MsgCode;
        CopyTime+= (clock() - CopyStartTime);
        MsgCode=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,MSG(MError),
                        MSG(MCopyCannotCreateFolder),DestPath,MSG(MCopyRetry),
                        MSG(MCopySkip),MSG(MCopyCancel));
        CopyStartTime = clock();
        if (MsgCode!=0)
          return((MsgCode==-2 || MsgCode==2) ? COPY_CANCEL:COPY_NEXT);
      }
      DWORD SetAttr=SrcData->dwFileAttributes;
      if (SrcDriveType==DRIVE_CDROM && Opt.ClearReadOnly && (SetAttr & FA_RDONLY))
        SetAttr&=~FA_RDONLY;
      SetFileAttributes(DestPath,SetAttr);
      /* $ 30.12.2000 SVS
         При копировании/переносе забыли выставить FILE_ATTRIBUTE_ENCRYPTED
         для каталога, если он есть
      */
      if(SetAttr&FILE_ATTRIBUTE_ENCRYPTED)
        if(!ESetFileEncryption(DestPath,1,0))
        {
           RemoveDirectory(DestPath);
           return COPY_CANCEL;
        }
      /* SVS $*/
      TreeList::AddTreeName(DestPath);
      return(COPY_SUCCESS);
    }

    if (DestAttr!=(DWORD)-1 && (DestAttr & FA_DIREC)==0)
    {
      if (SrcData->nFileSizeLow==DestData.nFileSizeLow)
      {
        int CmpCode;
        if ((CmpCode=CmpFullNames(Src,DestPath))!=0)
        {
          SameName=1;
          if (CmpCode==2 || !Rename)
          {
            CopyTime+= (clock() - CopyStartTime);
            Message(MSG_DOWN|MSG_WARNING,1,MSG(MError),MSG(MCannotCopyFileToItself1),
                    Src,MSG(MCannotCopyFileToItself2),MSG(MOk));
            CopyStartTime = clock();
            return(COPY_CANCEL);
          }
        }
      }

      int RetCode, AskCode;
      CopyTime+= (clock() - CopyStartTime);
      AskCode = AskOverwrite(SrcData,DestPath,DestAttr,SameName,Rename,1,Append,RetCode);
      CopyStartTime = clock();
      if (!AskCode)
        return((COPY_CODES)RetCode);
    }
  }
  else
  {
    if (SrcData->dwFileAttributes & FA_DIREC)
      return COPY_SUCCESS;
/*
    {
      char SrcFullName[NM];
      if (ConvertNameToFull(Src,SrcFullName, sizeof(SrcFullName)) >= sizeof(SrcFullName))
      {
        return(COPY_NEXT);
      }
    }
*/
  }

  while (1)
  {
    int64 SaveCopySize=CurCopySize;
    int64 SaveTotalSize=TotalCopySize;
    if (!CopyToNUL && Rename)
    {
      int MoveCode,AskDelete;
      if (WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT)
        if (!Append && DestAttr!=(DWORD)-1 && !SameName)
          remove(DestPath);
      if (!Append)
      {
        if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
          MoveCode=MoveFileEx(Src,DestPath,SameName ? MOVEFILE_COPY_ALLOWED:MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING);
        else
          MoveCode=MoveFile(Src,DestPath);
        if (!MoveCode && GetLastError()==ERROR_NOT_SAME_DEVICE)
          return(COPY_FAILURE);
        if (ShowTotalCopySize && MoveCode)
        {
          int64 AddSize(SrcData->nFileSizeHigh,SrcData->nFileSizeLow);
          CurCopySize+=AddSize;
          ShowBar(CurCopySize,TotalCopySize,true);
          ShowTitle(FALSE);
        }
        AskDelete=0;
      }
      else
      {
        switch(ShellCopyFile(Src,SrcData,DestPath,(DWORD)-1,Append))
        {
          case COPY_SUCCESS:
            MoveCode=TRUE;
            break;
          case COPY_FAILURE:
            MoveCode=FALSE;
            break;
          case COPY_CANCEL:
            return(COPY_CANCEL);
          case COPY_NEXT:
            return(COPY_NEXT);
        }
        AskDelete=1;
      }

      if (MoveCode)
      {
        if (DestAttr==(DWORD)-1 || (DestAttr & FA_DIREC)==0)
          if (PointToName(DestPath)==DestPath)
            strcpy(RenamedName,DestPath);
          else
            strcpy(CopiedName,PointToName(DestPath));
        if (SrcDriveType==DRIVE_CDROM && Opt.ClearReadOnly &&
            (SrcData->dwFileAttributes & FA_RDONLY))
          SetFileAttributes(DestPath,SrcData->dwFileAttributes & ~FA_RDONLY);
        TotalFiles++;
        if (AskDelete && DeleteAfterMove(Src,SrcData->dwFileAttributes)==COPY_CANCEL)
          return(COPY_CANCEL);
        return(COPY_SUCCESS_MOVE);
      }
    }
    else
    {
      int CopyCode;
      if ((CopyCode=ShellCopyFile(Src,SrcData,DestPath,DestAttr,Append))==COPY_SUCCESS)
      {
        strcpy(CopiedName,PointToName(DestPath));
        if(!CopyToNUL)
        {
          if (SrcDriveType==DRIVE_CDROM && Opt.ClearReadOnly &&
              (SrcData->dwFileAttributes & FA_RDONLY))
            SetFileAttributes(DestPath,SrcData->dwFileAttributes & ~FA_RDONLY);
          if (DestAttr!=(DWORD)-1 && LocalStricmp(CopiedName,DestData.cFileName)==0 &&
              strcmp(CopiedName,DestData.cFileName)!=0)
            MoveFile(DestPath,DestPath);
        }
        TotalFiles++;
        return(COPY_SUCCESS);
      }
      else
        if (CopyCode==COPY_CANCEL || CopyCode==COPY_NEXT)
          return((COPY_CODES)CopyCode);
    }
    char Msg1[2*NM],Msg2[2*NM];
    sprintf(Msg1,Link ? MSG(MCannotLink):Move ? MSG(MCannotMove):MSG(MCannotCopy),Src);
    sprintf(Msg2,MSG(MCannotCopyTo),DestPath);
    {
      int MsgCode;
      CopyTime+= (clock() - CopyStartTime);
      MsgCode=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,MSG(MError),
                      Msg1,Msg2,MSG(MCopyRetry),MSG(MCopySkip),
                      MSG(MCopyCancel));
      CopyStartTime = clock();
      if (MsgCode!=0)
        return((MsgCode==-2 || MsgCode==2) ? COPY_CANCEL:COPY_NEXT);
    }
    CurCopySize=SaveCopySize;
    TotalCopySize=SaveTotalSize;
    int RetCode, AskCode;
    CopyTime+= (clock() - CopyStartTime);
    AskCode = AskOverwrite(SrcData,DestPath,DestAttr,SameName,Rename,1,Append,RetCode);
    CopyStartTime = clock();
    if (!AskCode)
      return((COPY_CODES)RetCode);
  }
}


void ShellCopy::ShellCopyMsg(char *Src,char *Dest,int Flags)
{
  char FilesStr[100],BarStr[100],SrcName[NM],DestName[NM];

  #define BAR_SIZE	40
  static char Bar[BAR_SIZE+2]={0};
  if(!Bar[0])
    memset(Bar,0x0C4,BAR_SIZE);

  strcpy(BarStr,Bar);

  if (ShowTotalCopySize)
  {
    char TotalMsg[100];
    if (*TotalCopySizeText)
      sprintf(TotalMsg," %s: %s ",MSG(MCopyDlgTotal),TotalCopySizeText);
    else
      sprintf(TotalMsg," %s ",MSG(MCopyDlgTotal));
    int TotalLength=strlen(TotalMsg);
    strncpy(BarStr+(strlen(BarStr)-TotalLength+1)/2,TotalMsg,TotalLength);
    *FilesStr=0;
  }
  else
  {
    sprintf(FilesStr,MSG(MCopyProcessed),TotalFiles);
    /* $ 30.01.2001 VVM
        + Запомнить время начала.
          Работает для каждого файла при выключенном ShowTotalIndicator  */
    if ((Src!=NULL) && (ShowCopyTime))
    {
      CopyStartTime = clock();
      CopyTime = 0;
      LastShowTime = 0;
    }
    /* VVM $ */
  }

  if (Src!=NULL)
  {
    sprintf(SrcName,"%-40s",Src);
    TruncPathStr(SrcName,40);
  }
  sprintf(DestName,"%-40s",Dest);
  TruncPathStr(DestName,40);

  SetMessageHelp("CopyFiles");
  if (Src==NULL)
    Message(Flags,0,Move ? MSG(MMoveDlgTitle):MSG(MCopyDlgTitle),"",MSG(MCopyScanning),DestName,"","",BarStr,"");
  else
/* $ 30.01.2001 VVM
    + Показывает время копирования,оставшееся время и среднюю скорость. */
    if (Move)
    {
      if (ShowCopyTime)
        Message(Flags,0,MSG(MMoveDlgTitle),MSG(MCopyMoving),SrcName,MSG(MCopyTo),DestName,"",BarStr,FilesStr,Bar,"");
      else
        Message(Flags,0,MSG(MMoveDlgTitle),MSG(MCopyMoving),SrcName,MSG(MCopyTo),DestName,"",BarStr,FilesStr);
    }
    else
    {
      if (ShowCopyTime)
        Message(Flags,0,MSG(MCopyDlgTitle),MSG(MCopyCopying),SrcName,MSG(MCopyTo),DestName,"",BarStr,FilesStr,Bar,"");
      else
        Message(Flags,0,MSG(MCopyDlgTitle),MSG(MCopyCopying),SrcName,MSG(MCopyTo),DestName,"",BarStr,FilesStr);
    }
/* VVM $ */
  int MessageX1,MessageY1,MessageX2,MessageY2;
  GetMessagePosition(MessageX1,MessageY1,MessageX2,MessageY2);
  BarX=MessageX1+5;
  BarY=MessageY1+6;
  BarLength=MessageX2-MessageX1-9;

  if (Src!=NULL)
  {
    ShowBar(0,0,false);
    if (ShowTotalCopySize)
    {
      ShowBar(CurCopySize,TotalCopySize,true);
      ShowTitle(FALSE);
    }
  }
}


int ShellCopy::ShellCopyConvertWildcards(char *Src,char *Dest)
{
  char WildName[2*NM],*CurWildPtr,*DestNamePtr,*SrcNamePtr;
  char PartBeforeName[NM],PartAfterFolderName[NM];
  DestNamePtr=PointToName(Dest);
  strcpy(WildName,DestNamePtr);
  if (strchr(WildName,'*')==NULL && strchr(WildName,'?')==NULL)
    return(FALSE);

  if (SelectedFolderNameLength!=0)
  {
    strcpy(PartAfterFolderName,Src+SelectedFolderNameLength);
    Src[SelectedFolderNameLength]=0;
  }

  SrcNamePtr=PointToName(Src);

  int BeforeNameLength=DestNamePtr==Dest ? SrcNamePtr-Src:0;
  strncpy(PartBeforeName,Src,BeforeNameLength);
  PartBeforeName[BeforeNameLength]=0;

  char *SrcNameDot=strrchr(SrcNamePtr,'.');
  CurWildPtr=WildName;
  while (*CurWildPtr)
    switch(*CurWildPtr)
    {
      case '?':
        CurWildPtr++;
        if (*SrcNamePtr)
          *(DestNamePtr++)=*(SrcNamePtr++);
        break;
      case '*':
        CurWildPtr++;
        while (*SrcNamePtr)
        {
          if (*CurWildPtr=='.' && SrcNameDot!=NULL && strchr(CurWildPtr+1,'.')==NULL)
          {
            if (SrcNamePtr==SrcNameDot)
              break;
          }
          else
            if (*SrcNamePtr==*CurWildPtr)
              break;
          *(DestNamePtr++)=*(SrcNamePtr++);
        }
        break;
      case '.':
        CurWildPtr++;
        *(DestNamePtr++)='.';
        if (strpbrk(CurWildPtr,"*?")!=NULL)
          while (*SrcNamePtr)
            if (*(SrcNamePtr++)=='.')
              break;
        break;
      default:
        *(DestNamePtr++)=*(CurWildPtr++);
        if (*SrcNamePtr && *SrcNamePtr!='.')
          SrcNamePtr++;
        break;
    }

  *DestNamePtr=0;
  if (DestNamePtr!=Dest && *(DestNamePtr-1)=='.')
    *(DestNamePtr-1)=0;
  if (*PartBeforeName)
  {
    strcat(PartBeforeName,Dest);
    strcpy(Dest,PartBeforeName);
  }
  if (SelectedFolderNameLength!=0)
    strcat(Src,PartAfterFolderName);
  return(TRUE);
}


void ShellCopy::CreatePath(char *Path)
{
  char *ChPtr;
  ChPtr=Path;
  while(*ChPtr)
  {
    if (*ChPtr=='\\')
    {
      *ChPtr=0;

      char DirName[NM];
      strcpy(DirName,Path);
      if (Opt.CreateUppercaseFolders && !IsCaseMixed(DirName))
        LocalStrupr(DirName);

      if (CreateDirectory(DirName,NULL))
        TreeList::AddTreeName(DirName);
      *ChPtr='\\';
    }
    ChPtr++;
  }
  if (CreateDirectory(Path,NULL))
    TreeList::AddTreeName(Path);
}


int ShellCopy::DeleteAfterMove(char *Name,int Attr)
{
  if (Attr & FA_RDONLY)
  {
    int MsgCode;
    CopyTime+= (clock() - CopyStartTime);
    if (ReadOnlyDelMode!=-1)
      MsgCode=ReadOnlyDelMode;
    else
      MsgCode=Message(MSG_DOWN|MSG_WARNING,5,MSG(MWarning),
              MSG(MCopyFileRO),Name,MSG(MCopyAskDelete),
              MSG(MCopyDeleteRO),MSG(MCopyDeleteAllRO),
              MSG(MCopySkipRO),MSG(MCopySkipAllRO),MSG(MCopyCancelRO));
    CopyStartTime = clock();
    switch(MsgCode)
    {
      case 1:
        ReadOnlyDelMode=1;
        break;
      case 2:
        return(COPY_NEXT);
      case 3:
        ReadOnlyDelMode=3;
        return(COPY_NEXT);
      case -1:
      case -2:
      case 4:
        return(COPY_CANCEL);
    }
    SetFileAttributes(Name,0);
  }
  while (remove(Name)!=0)
  {
    int MsgCode;
    CopyTime+= (clock() - CopyStartTime);
    MsgCode=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,MSG(MError),
                    MSG(MCannotDeleteFile),Name,MSG(MDeleteRetry),
                    MSG(MDeleteSkip),MSG(MDeleteCancel));
    CopyStartTime = clock();
    if (MsgCode==1 || MsgCode==-1)
      break;
    if (MsgCode==2 || MsgCode==-2)
      return(COPY_CANCEL);
  }
  return(COPY_SUCCESS);
}


int ShellCopy::ShellCopyFile(char *SrcName,WIN32_FIND_DATA *SrcData,
                             char *DestName,DWORD DestAttr,int Append)
{
  if (Link)
    return(MkLink(SrcName,DestName) ? COPY_SUCCESS:COPY_FAILURE);
  if (!CopyToNUL && Opt.UseSystemCopy && !Append)
  {
    if (!Opt.CopyOpened)
    {
      HANDLE SrcHandle=CreateFile(SrcName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
      if (SrcHandle==INVALID_HANDLE_VALUE)
        return(COPY_FAILURE);
      CloseHandle(SrcHandle);
    }
    return(ShellSystemCopy(SrcName,DestName,SrcData));
  }

  SECURITY_ATTRIBUTES sa;
  if (CopySecurity && !GetSecurity(SrcName,&sa))
    return(COPY_CANCEL);
  int OpenMode=FILE_SHARE_READ;
  if (Opt.CopyOpened)
    OpenMode|=FILE_SHARE_WRITE;
  HANDLE SrcHandle=CreateFile(SrcName,GENERIC_READ,OpenMode,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (SrcHandle==INVALID_HANDLE_VALUE)
    return(COPY_FAILURE);

  HANDLE DestHandle;
  DWORD AppendPos=0;

  if(!CopyToNUL)
  {
    if (DestAttr!=(DWORD)-1 && !Append)
      remove(DestName);
    DestHandle=CreateFile(DestName,GENERIC_WRITE,FILE_SHARE_READ,
                          CopySecurity ? &sa:NULL,
                          Append ? OPEN_EXISTING:CREATE_ALWAYS,
                          SrcData->dwFileAttributes|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    if (DestHandle==INVALID_HANDLE_VALUE)
    {
      DWORD LastError=GetLastError();
      CloseHandle(SrcHandle);
      SetLastError(LastError);
      return(COPY_FAILURE);
    }

    if (Append && (AppendPos=SetFilePointer(DestHandle,0,NULL,FILE_END))==0xFFFFFFFF)
    {
      DWORD LastError=GetLastError();
      CloseHandle(SrcHandle);
      CloseHandle(DestHandle);
      SetLastError(LastError);
      return(COPY_FAILURE);
    }
  }

  int64 WrittenSize(0,0);
  while (1)
  {
    if (CheckForEsc())
    {
      CloseHandle(SrcHandle);
      if(!CopyToNUL)
      {
        if (Append)
        {
          SetFilePointer(DestHandle,AppendPos,NULL,FILE_BEGIN);
          SetEndOfFile(DestHandle);
        }
        CloseHandle(DestHandle);
        if (!Append)
        {
          SetFileAttributes(DestName,0);
          DeleteFile(DestName);
        }
      }
      return(COPY_CANCEL);
    }
    DWORD BytesRead,BytesWritten;

    /* $ 23.10.2000 VVM
       + Динамический буфер копирования */

    if (CopyBufSize < CopyBufferSize)
      StartTime=clock();

    while (!ReadFile(SrcHandle,CopyBuffer,CopyBufSize,&BytesRead,NULL))
    {
      CopyTime+= (clock() - CopyStartTime);
      int MsgCode = Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),
                            MSG(MCopyReadError),SrcName,MSG(MRetry),MSG(MCancel));
      CopyStartTime = clock();
      if (MsgCode==0)
        continue;
      DWORD LastError=GetLastError();
      CloseHandle(SrcHandle);
      if(!CopyToNUL)
      {
        if (Append)
        {
          SetFilePointer(DestHandle,AppendPos,NULL,FILE_BEGIN);
          SetEndOfFile(DestHandle);
        }
        CloseHandle(DestHandle);
        if (!Append)
        {
          SetFileAttributes(DestName,0);
          DeleteFile(DestName);
        }
      }
      ShowBar(0,0,false);
      ShowTitle(FALSE);
      SetLastError(LastError);
      return(COPY_FAILURE);
    }
    if (BytesRead==0)
      break;

    if(!CopyToNUL)
    {
      while (!WriteFile(DestHandle,CopyBuffer,BytesRead,&BytesWritten,NULL))
      {
        DWORD LastError=GetLastError();
        int Split=FALSE,SplitCancelled=FALSE,SplitSkipped=FALSE;
        if ((LastError==ERROR_DISK_FULL || LastError==ERROR_HANDLE_DISK_FULL) &&
            DestName[0]!=0 && DestName[1]==':')
        {
          char DriveRoot[NM];
          GetPathRoot(DestName,DriveRoot);

          DWORD SectorsPerCluster,BytesPerSector,FreeClusters,Clusters;
          if (GetDiskFreeSpace(DriveRoot,&SectorsPerCluster,&BytesPerSector,
                               &FreeClusters,&Clusters))
          {
            DWORD FreeSize=SectorsPerCluster*BytesPerSector*FreeClusters;
            if (FreeSize<BytesRead &&
                WriteFile(DestHandle,CopyBuffer,FreeSize,&BytesWritten,NULL) &&
                SetFilePointer(SrcHandle,FreeSize-BytesRead,NULL,FILE_CURRENT)!=0xFFFFFFFF)
            {
              CloseHandle(DestHandle);
              SetMessageHelp("CopyFiles");
              CopyTime+= (clock() - CopyStartTime);
              int MsgCode=Message(MSG_DOWN|MSG_WARNING,4,MSG(MError),
                                  MSG(MErrorInsufficientDiskSpace),DestName,
                                  MSG(MSplit),MSG(MSkip),MSG(MRetry),MSG(MCancel));
              CopyStartTime = clock();
              if (MsgCode==2)
              {
                CloseHandle(SrcHandle);
                if (!Append)
                {
                  SetFileAttributes(DestName,0);
                  DeleteFile(DestName);
                }
                return(COPY_FAILURE);
              }
              if (MsgCode==0)
              {
                Split=TRUE;
                while (1)
                {
                  if (GetDiskFreeSpace(DriveRoot,&SectorsPerCluster,&BytesPerSector,&FreeClusters,&Clusters))
                    if (SectorsPerCluster*BytesPerSector*FreeClusters==0)
                    {
                      CopyTime+= (clock() - CopyStartTime);
                      int MsgCode = Message(MSG_DOWN|MSG_WARNING,2,MSG(MWarning),
                                            MSG(MCopyErrorDiskFull),DestName,
                                            MSG(MRetry),MSG(MCancel));
                      CopyStartTime = clock();
                      if (MsgCode!=0)
                      {
                        Split=FALSE;
                        SplitCancelled=TRUE;
                      }
                      else
                        continue;
                    }
                  break;
                }
              }
              if (MsgCode==1)
                SplitSkipped=TRUE;
              if (MsgCode==-1 || MsgCode==3)
                SplitCancelled=TRUE;
            }
          }
        }
        if (Split)
        {
          int RetCode, AskCode;
          CopyTime+= (clock() - CopyStartTime);
          AskCode = AskOverwrite(SrcData,DestName,0xFFFFFFFF,FALSE,Move,1,Append,RetCode);
          CopyStartTime = clock();
          if (!AskCode)
          {
            CloseHandle(SrcHandle);
            return(COPY_CANCEL);
          }
          char DestDir[NM],*ChPtr;
          strcpy(DestDir,DestName);
          if ((ChPtr=strrchr(DestDir,'\\'))!=NULL)
          {
            *ChPtr=0;
            CreatePath(DestDir);
          }
          DestHandle=CreateFile(DestName,GENERIC_WRITE,FILE_SHARE_READ,NULL,
                                Append ? OPEN_EXISTING:CREATE_ALWAYS,
                                SrcData->dwFileAttributes|FILE_FLAG_SEQUENTIAL_SCAN,NULL);

          if (DestHandle==INVALID_HANDLE_VALUE ||
              Append && SetFilePointer(DestHandle,0,NULL,FILE_END)==0xFFFFFFFF)
          {
            DWORD LastError=GetLastError();
            CloseHandle(SrcHandle);
            CloseHandle(DestHandle);
            SetLastError(LastError);
            return(COPY_FAILURE);
          }
        }
        else
        {
          CopyTime+= (clock() - CopyStartTime);
          if (!SplitCancelled && !SplitSkipped &&
              Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),
              MSG(MCopyWriteError),DestName,MSG(MRetry),MSG(MCancel))==0)
          {
            CopyStartTime = clock();
            continue;
          }
          else
            CopyStartTime = clock();
          CloseHandle(SrcHandle);
          if (Append)
          {
            SetFilePointer(DestHandle,AppendPos,NULL,FILE_BEGIN);
            SetEndOfFile(DestHandle);
          }
          CloseHandle(DestHandle);
          if (!Append)
          {
            SetFileAttributes(DestName,0);
            DeleteFile(DestName);
          }
          ShowBar(0,0,false);
          ShowTitle(FALSE);
          SetLastError(LastError);
          if (SplitSkipped)
            return(COPY_NEXT);
          return(SplitCancelled ? COPY_CANCEL:COPY_FAILURE);
        }
        break;
      }
    }
    else
    {
      BytesWritten=BytesRead; // не забудем приравнять количество записанных байт
    }

    if ((CopyBufSize < CopyBufferSize) && (BytesWritten==CopyBufSize))
    {
      StopTime=clock();
      if ((StopTime - StartTime) < 1000)
      {
        CopyBufSize*=2;
        if (CopyBufSize > CopyBufferSize)
          CopyBufSize=CopyBufferSize;
      }
    } /* if */
    /* VVM $ */

    WrittenSize+=BytesWritten;
    int64 FileSize(SrcData->nFileSizeHigh,SrcData->nFileSizeLow);
    ShowBar(WrittenSize,FileSize,false);
    if (ShowTotalCopySize)
    {
      CurCopySize+=BytesWritten;
      ShowBar(CurCopySize,TotalCopySize,true);
      ShowTitle(FALSE);
    }
  }
  if(!CopyToNUL)
  {
    SetFileTime(DestHandle,NULL,NULL,&SrcData->ftLastWriteTime);
    CloseHandle(SrcHandle);
    CloseHandle(DestHandle);

    if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS &&
        (SrcData->dwFileAttributes & (FA_HIDDEN|FA_SYSTEM|FA_RDONLY)))
      SetFileAttributes(DestName,SrcData->dwFileAttributes);
  }
  else
    CloseHandle(SrcHandle);

  return(COPY_SUCCESS);
}

/* $ 30.01.2001 VVM
    + Перевод секунд в текст */
void static GetTimeText(int Time, char *TimeText)
{
  int Sec = Time;
  int Min = Sec/60;
  Sec-=(Min * 60);
  int Hour = Min/24;
  Min-=(Hour*24);
  sprintf(TimeText,"%02d:%02d:%02d",Hour,Min,Sec);
}
/* VVM $ */

void ShellCopy::ShowBar(int64 WrittenSize,int64 TotalSize,bool TotalBar)
{
/* $ 30.01.2001 VVM
    + Запомнить размеры */
  int64 OldWrittenSize = WrittenSize;
  int64 OldTotalSize = TotalSize;
/* VVM $ */
  WrittenSize=WrittenSize>>8;
  TotalSize=TotalSize>>8;

  int Length;
  if (WrittenSize.PLow()>TotalSize.PLow())
    WrittenSize.PLow()=TotalSize.PLow();
  if (TotalSize.PLow()==0)
    Length=BarLength;
  else
    if (TotalSize.PLow()<1000000)
      Length=WrittenSize.PLow()*BarLength/TotalSize.PLow();
    else
      Length=(WrittenSize.PLow()/100)*BarLength/(TotalSize.PLow()/100);
  char ProgressBar[100];
  memset(ProgressBar,0x0B0,BarLength);
  ProgressBar[BarLength]=0;
  if (TotalSize.PLow()!=0)
    memset(ProgressBar,0x0DB,Length);
  SetColor(COL_DIALOGTEXT);
  GotoXY(BarX,BarY+(TotalBar ? 2:0));
  Text(ProgressBar);
/* $ 30.01.2001 VVM
    + Показывает время копирования,оставшееся время и среднюю скорость. */
  if (ShowCopyTime &&
      (!ShowTotalCopySize || TotalBar) &&
      (clock() - LastShowTime > 1000))
  {
    LastShowTime = clock();
    CopyTime+= (clock() - CopyStartTime);
    CopyStartTime = clock();
    int WorkTime = CopyTime/1000;
    int64 SizeLeft = OldTotalSize - OldWrittenSize;

    int TimeLeft;
    char TimeStr[100];
    char c[2];
    c[1]=0;

    if (OldTotalSize == 0 || WorkTime == 0)
      sprintf(TimeStr,MSG(MCopyTimeInfo), " ", " ", 0, " ");
    else
    {
      int CPS = (OldWrittenSize/WorkTime).PLow();
      TimeLeft = (CPS)?(SizeLeft/CPS).PLow():0;
      strcpy(c," ");
      if (CPS > 99999) {
        c[0]='K';
        CPS = CPS/1024;
      }
      if (CPS > 99999) {
        c[0]='M';
        CPS = CPS/1024;
      }
      /* $ 06.03.2001 SVS
         А у меня и такое есть :-)
      */
      if (CPS > 99999) {
        c[0]='G';
        CPS = CPS/1024;
      }
      /* SVS $ */
      char WorkTimeStr[12];
      char TimeLeftStr[12];
      GetTimeText(WorkTime, WorkTimeStr);
      GetTimeText(TimeLeft, TimeLeftStr);
      sprintf(TimeStr,MSG(MCopyTimeInfo), WorkTimeStr, TimeLeftStr, CPS, c);
    }
    GotoXY(BarX,BarY+4);
    Text(TimeStr);
  }
/* VVM $ */
}


void ShellCopy::SetDestDizPath(char *DestPath)
{
  if (!DizRead)
  {
    strcpy(DestDizPath,DestPath);
    char *ChPtr=PointToName(DestDizPath);
    *(ChPtr--)=0;
    if (*DestDizPath==0)
      strcpy(DestDizPath,".");
    if (ChPtr>DestDizPath && *ChPtr!=':' && *(ChPtr-1)!=':')
      *ChPtr=0;
    if (Opt.Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && !SrcPanel->IsDizDisplayed() ||
        Opt.Diz.UpdateMode==DIZ_NOT_UPDATE)
      *DestDizPath=0;
    if (*DestDizPath)
      DestDiz.Read(DestDizPath);
    DizRead=TRUE;
  }
}


int ShellCopy::AskOverwrite(WIN32_FIND_DATA *SrcData,char *DestName,
               DWORD DestAttr,int SameName,int Rename,int AskAppend,
               int &Append,int &RetCode)
{
  HANDLE FindHandle;
  WIN32_FIND_DATA DestData;
  int DestDataFilled=FALSE;

  int MsgCode;

  Append=FALSE;

  if(CopyToNUL)
  {
    RetCode=COPY_NEXT;
    return TRUE;
  }

  if (DestAttr==0xFFFFFFFF)
    if ((DestAttr=GetFileAttributes(DestName))==0xFFFFFFFF)
      return(TRUE);

  if (DestAttr & FA_DIREC)
    return(TRUE);

  char TruncDestName[NM];
  strcpy(TruncDestName,DestName);
  TruncPathStr(TruncDestName,ScrX-16);

  if (OvrMode!=-1)
    MsgCode=OvrMode;
  else
  {
    int Type;
    if (!Opt.Confirm.Copy && !Rename || !Opt.Confirm.Move && Rename ||
        SameName || (Type=GetFileTypeByName(DestName))==FILE_TYPE_CHAR ||
        Type==FILE_TYPE_PIPE || OverwriteNext)
      MsgCode=1;
    else
    {
      memset(&DestData,0,sizeof(DestData));
      if ((FindHandle=FindFirstFile(DestName,&DestData))!=INVALID_HANDLE_VALUE)
        FindClose(FindHandle);
      DestDataFilled=TRUE;
      char SrcFileStr[512],DestFileStr[512],DateText[20],TimeText[20];
      ConvertDate(&SrcData->ftLastWriteTime,DateText,TimeText,8);

      int64 SrcSize(SrcData->nFileSizeHigh,SrcData->nFileSizeLow);
      char SrcSizeText[20];
      SrcSize.itoa(SrcSizeText);
      int64 DestSize(DestData.nFileSizeHigh,DestData.nFileSizeLow);
      char DestSizeText[20];
      DestSize.itoa(DestSizeText);
      /* $ 04.08.2000 SVS
         Опция "Only newer file(s)"
      */
      if(OnlyNewerFiles)
      {
        // сравним время
        int RetCompare=CompareFileTime(&DestData.ftLastWriteTime,&SrcData->ftLastWriteTime);
        if(RetCompare < 0)
          MsgCode=0;
        else
          MsgCode=2;
      }
      else
      {
        sprintf(SrcFileStr,"%-17s %11.11s %s %s",MSG(MCopySource),SrcSizeText,DateText,TimeText);
        ConvertDate(&DestData.ftLastWriteTime,DateText,TimeText,8);
        sprintf(DestFileStr,"%-17s %11.11s %s %s",MSG(MCopyDest),DestSizeText,DateText,TimeText);

        SetMessageHelp("CopyFiles");
        MsgCode=Message(MSG_DOWN|MSG_WARNING,AskAppend ? 6:5,MSG(MWarning),
                MSG(MCopyFileExist),TruncDestName,"\x1",SrcFileStr,DestFileStr,
                "\x1",MSG(MCopyOverwrite),MSG(MCopyOverwriteAll),
                MSG(MCopySkipOvr),MSG(MCopySkipAllOvr),
                AskAppend ? (AskAppend==1 ? MSG(MCopyAppend):MSG(MCopyResume)):MSG(MCopyCancelOvr),
                AskAppend ? MSG(MCopyCancelOvr):NULL);
        if (!AskAppend && MsgCode==4)
          MsgCode=5;
      }
      /* SVS $*/
    }
  }

  switch(MsgCode)
  {
    case 1:
      OvrMode=1;
      break;
    case 2:
      RetCode=COPY_NEXT;
      return(FALSE);
    case 3:
      OvrMode=3;
      RetCode=COPY_NEXT;
      return(FALSE);
    case 4:
      Append=TRUE;
      break;
    case -1:
    case -2:
    case 5:
      RetCode=COPY_CANCEL;
      return(FALSE);
  }
  if ((DestAttr & FA_RDONLY) && !OverwriteNext)
  {
    int MsgCode;
    if (SameName)
      MsgCode=0;
    else
      if (ReadOnlyOvrMode!=-1)
        MsgCode=ReadOnlyOvrMode;
      else
      {
        if (!DestDataFilled)
        {
          memset(&DestData,0,sizeof(DestData));
          if ((FindHandle=FindFirstFile(DestName,&DestData))!=INVALID_HANDLE_VALUE)
            FindClose(FindHandle);
        }
        char SrcFileStr[512],DestFileStr[512],DateText[20],TimeText[20];

        int64 SrcSize(SrcData->nFileSizeHigh,SrcData->nFileSizeLow);
        char SrcSizeText[20];
        SrcSize.itoa(SrcSizeText);
        int64 DestSize(DestData.nFileSizeHigh,DestData.nFileSizeLow);
        char DestSizeText[20];
        DestSize.itoa(DestSizeText);

        ConvertDate(&SrcData->ftLastWriteTime,DateText,TimeText,8);
        sprintf(SrcFileStr,"%-17s %11.11s %s %s",MSG(MCopySource),SrcSizeText,DateText,TimeText);
        ConvertDate(&DestData.ftLastWriteTime,DateText,TimeText,8);
        sprintf(DestFileStr,"%-17s %11.11s %s %s",MSG(MCopyDest),DestSizeText,DateText,TimeText);

        SetMessageHelp("CopyFiles");
        MsgCode=Message(MSG_DOWN|MSG_WARNING,6,MSG(MWarning),
                MSG(MCopyFileRO),TruncDestName,"\x1",SrcFileStr,DestFileStr,
                "\x1",MSG(MCopyOverwrite),MSG(MCopyOverwriteAll),
                MSG(MCopySkipOvr),MSG(MCopySkipAllOvr),
                AskAppend ? MSG(MCopyAppend):MSG(MCopyCancelOvr),
                AskAppend ? MSG(MCopyCancelOvr):NULL);
        if (!AskAppend && MsgCode==4)
          MsgCode=5;
      }
    switch(MsgCode)
    {
      case 1:
        ReadOnlyOvrMode=1;
        break;
      case 2:
        RetCode=COPY_NEXT;
        return(FALSE);
      case 3:
        ReadOnlyOvrMode=3;
        RetCode=COPY_NEXT;
        return(FALSE);
      case 4:
        ReadOnlyOvrMode=1;
        Append=TRUE;
        break;
      case -1:
      case -2:
      case 5:
        RetCode=COPY_CANCEL;
        return(FALSE);
    }
  }
  if (!SameName && (DestAttr & (FA_RDONLY|FA_HIDDEN|FA_SYSTEM)))
    SetFileAttributes(DestName,0);
  return(TRUE);
}


int ShellCopy::GetSecurity(char *FileName,SECURITY_ATTRIBUTES *sa)
{
  char AnsiName[NM];
  SECURITY_INFORMATION si=DACL_SECURITY_INFORMATION;
  SECURITY_DESCRIPTOR *sd=(SECURITY_DESCRIPTOR *)sddata;
  DWORD Needed;
  OemToChar(FileName,AnsiName);
  if (!GetFileSecurity(AnsiName,si,sd,sizeof(sddata),&Needed))
  {
    sd=NULL;
    int LastError=GetLastError();
    if (LastError!=ERROR_SUCCESS && LastError!=ERROR_FILE_NOT_FOUND &&
        LastError!=ERROR_CALL_NOT_IMPLEMENTED &&
        Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),
                MSG(MCannotGetSecurity),FileName,MSG(MOk),MSG(MCancel))==1)
      return(FALSE);
  }
  sa->nLength=sizeof(sa);
  sa->lpSecurityDescriptor=sd;
  sa->bInheritHandle=FALSE;
  return(TRUE);
}


int ShellCopy::SetSecurity(char *FileName,SECURITY_ATTRIBUTES *sa)
{
  char AnsiName[NM];
  SECURITY_INFORMATION si=DACL_SECURITY_INFORMATION;
  OemToChar(FileName,AnsiName);
  if (!SetFileSecurity(AnsiName,si,sa->lpSecurityDescriptor))
  {
    int LastError=GetLastError();
    if (LastError!=ERROR_SUCCESS && LastError!=ERROR_FILE_NOT_FOUND &&
        LastError!=ERROR_CALL_NOT_IMPLEMENTED &&
        Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),
                MSG(MCannotSetSecurity),FileName,MSG(MOk),MSG(MCancel))==1)
      return(FALSE);
  }
  return(TRUE);
}


typedef BOOL (WINAPI *COPYFILEEX)(LPCTSTR lpExistingFileName,
            LPCTSTR lpNewFileName,void *lpProgressRoutine,
            LPVOID lpData,LPBOOL pbCancel,DWORD dwCopyFlags);

int ShellCopy::ShellSystemCopy(char *SrcName,char *DestName,WIN32_FIND_DATA *SrcData)
{
  static COPYFILEEX pCopyFileEx=NULL;
  static int LoadAttempt=FALSE;

  if (!LoadAttempt && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
  {
    HMODULE hKernel=GetModuleHandle("KERNEL32.DLL");
    if (hKernel)
      pCopyFileEx=(COPYFILEEX)GetProcAddress(hKernel,"CopyFileExA");
    LoadAttempt=TRUE;
  }


  SECURITY_ATTRIBUTES sa;
  if (CopySecurity && !GetSecurity(SrcName,&sa))
    return(COPY_CANCEL);

  ShellCopyMsg(SrcName,DestName,MSG_LEFTALIGN|MSG_KEEPBACKGROUND);
  if (pCopyFileEx)
  {
    BOOL Cancel=0;
    StartCopySizeEx=CurCopySize;
    if (!pCopyFileEx(SrcName,DestName,CopyProgressRoutine,NULL,&Cancel,0))
      return(GetLastError()==ERROR_REQUEST_ABORTED ? COPY_CANCEL:COPY_FAILURE);
  }
  else
  {
    if (ShowTotalCopySize)
    {
      int64 AddSize(SrcData->nFileSizeHigh,SrcData->nFileSizeLow);
      CurCopySize+=AddSize;
      ShowBar(CurCopySize,TotalCopySize,true);
      ShowTitle(FALSE);
    }
    if (!CopyFile(SrcName,DestName,FALSE))
      return(COPY_FAILURE);
  }

  if (CopySecurity && !SetSecurity(DestName,&sa))
    return(COPY_CANCEL);
  return(COPY_SUCCESS);
}


#define PROGRESS_CONTINUE  0
#define PROGRESS_CANCEL    1
#if defined(__BORLANDC__)
#pragma warn -par
#endif

DWORD WINAPI CopyProgressRoutine(LARGE_INTEGER TotalFileSize,
      LARGE_INTEGER TotalBytesTransferred,LARGE_INTEGER StreamSize,
      LARGE_INTEGER StreamBytesTransferred,DWORD dwStreamNumber,
      DWORD dwCallbackReason,HANDLE hSourceFile,HANDLE hDestinationFile,
      LPVOID lpData)
{
  if (ShowTotalCopySize && dwStreamNumber==1)
  {
    int64 AddSize(TotalBytesTransferred.u.HighPart,TotalBytesTransferred.u.LowPart);
    CurCopySize=StartCopySizeEx+AddSize;
    ShellCopy::ShowBar(CurCopySize,TotalCopySize,true);
    ShellCopy::ShowTitle(FALSE);
  }
  int64 TransferredSize(TotalBytesTransferred.u.HighPart,TotalBytesTransferred.u.LowPart);
  int64 TotalSize(TotalFileSize.u.HighPart,TotalFileSize.u.LowPart);
  ShellCopy::ShowBar(TransferredSize,TotalSize,false);
  return(CheckForEsc() ? PROGRESS_CANCEL:PROGRESS_CONTINUE);
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif


int ShellCopy::IsSameDisk(char *SrcPath,char *DestPath)
{
  char SrcRoot[NM],DestRoot[NM];
  GetPathRoot(SrcPath,SrcRoot);
  GetPathRoot(DestPath,DestRoot);
  if (strpbrk(DestPath,"\\:")==NULL)
    return(TRUE);
  if ((SrcRoot[0]=='\\' && SrcRoot[1]=='\\' || DestRoot[0]=='\\' && DestRoot[1]=='\\') &&
      LocalStricmp(SrcRoot,DestRoot)!=0)
    return(FALSE);
  if (*SrcPath==0 || *DestPath==0 || SrcPath[1]!=':' || DestPath[1]!=':')
    return(TRUE);
  if (toupper(DestPath[0])==toupper(SrcPath[0]))
    return(TRUE);
  DWORD SrcVolumeNumber=0,DestVolumeNumber=0;
  char SrcVolumeName[NM],DestVolumeName[NM];
  int64 SrcTotalSize,SrcTotalFree,SrcUserFree;
  int64 DestTotalSize,DestTotalFree,DestUserFree;

  if (!GetDiskSize(SrcRoot,&SrcTotalSize,&SrcTotalFree,&SrcUserFree))
    return(FALSE);
  if (!GetDiskSize(DestRoot,&DestTotalSize,&DestTotalFree,&DestUserFree))
    return(FALSE);
  if (!GetVolumeInformation(SrcRoot,SrcVolumeName,sizeof(SrcVolumeName),&SrcVolumeNumber,NULL,NULL,NULL,0))
    return(FALSE);
  if (!GetVolumeInformation(DestRoot,DestVolumeName,sizeof(DestVolumeName),&DestVolumeNumber,NULL,NULL,NULL,0))
    return(FALSE);
  if (SrcVolumeNumber!=0 && SrcVolumeNumber==DestVolumeNumber &&
      strcmp(SrcVolumeName,DestVolumeName)==0 &&
      SrcTotalSize==DestTotalSize)
    return(TRUE);
  return(FALSE);
}


bool ShellCopy::CalcTotalSize()
{
  char SelName[NM],SelShortName[NM];
  int FileAttr;

  TotalCopySize=CurCopySize=0;

  ShellCopyMsg(NULL,"",MSG_LEFTALIGN);

  SrcPanel->GetSelName(NULL,FileAttr);
  while (SrcPanel->GetSelName(SelName,FileAttr,SelShortName))
    if (FileAttr & FA_DIREC)
    {
      unsigned long DirCount,FileCount,ClusterSize;
      int64 FileSize,CompressedSize,RealFileSize;
      ShellCopyMsg(NULL,SelName,MSG_LEFTALIGN|MSG_KEEPBACKGROUND);
      if (!GetDirInfo("",SelName,DirCount,FileCount,FileSize,CompressedSize,
                      RealFileSize,ClusterSize,0xffffffff,FALSE))
      {
        ShowTotalCopySize=false;
        return(false);
      }
      TotalCopySize+=FileSize;
    }
    else
    {
      int64 FileSize;
      if (SrcPanel->GetLastSelectedSize(&FileSize)!=-1)
        TotalCopySize+=FileSize;
    }

  InsertCommas(TotalCopySize,TotalCopySizeText);
  return(true);
}


void ShellCopy::ShowTitle(int FirstTime)
{
  static char PrevTitle[200];
  char Title[200];
  if (FirstTime)
    *PrevTitle=0;
  strcpy(Title,StaticMove ? MSG(MCopyMovingTitle):MSG(MCopyCopyingTitle));

  if (ShowTotalCopySize && !FirstTime)
  {
    int64 CopySize=CurCopySize>>8,TotalSize=TotalCopySize>>8;
    /* $ 03.07.2000 IS
      Показывать проценты спереди при копировании/переносе
    было:
    char Percent[50];
    sprintf(Percent," {%d%%}",ToPercent(CopySize.LowPart,TotalSize.LowPart));
    strcat(Title,Percent);
    */
    char Percent[200];
    sprintf(Percent,"{%d%%} %s",ToPercent(CopySize.PLow(),TotalSize.PLow()),Title);
    strcpy(Title,Percent);
    /* IS $ */
  }
  if (strcmp(PrevTitle,Title)!=0)
  {
    SetFarTitle(Title);
    strcpy(PrevTitle,Title);
  }
}


int ShellCopy::CmpFullNames(char *Src,char *Dest)
{
  char SrcFullName[NM],DestFullName[NM];
  if (ConvertNameToFull(Src,SrcFullName, sizeof(SrcFullName)) >= sizeof(SrcFullName)){
    return(2);
  }
  if (ConvertNameToFull(Dest,DestFullName, sizeof(DestFullName)) >= sizeof(DestFullName)){
    return(2);
  }
//  ConvertNameToFull(Dest,DestFullName, sizeof(DestFullName));
  int I;
  for (I=strlen(SrcFullName)-1;I>0 && SrcFullName[I]=='.';I--)
    SrcFullName[I]=0;
  for (I=strlen(DestFullName)-1;I>0 && DestFullName[I]=='.';I--)
    DestFullName[I]=0;
  if (LocalStricmp(SrcFullName,DestFullName)!=0)
    return(0);
  return(strcmp(PointToName(SrcFullName),PointToName(DestFullName))==0 ? 2:1);
}
