/*
findfile.cpp

Поиск (Alt-F7)

*/

/* Revision: 1.37 01.07.2001 $ */

/*
Modify:
  01.07.2001
    + Можно использовать маски исключения при установлении параметров поиска.
  25.06.2001 IS
    ! Внедрение const
  25.06.2001 SVS
    ! Юзаем SEARCHSTRINGBUFSIZE
  23.06.2001 OT
    - косметические исправления, чтобы VC не "предупреждал" :)
  18.06.2001 SVS
    - исправляем последствия предыдущего патча :-)
  18.06.2001 SVS
    - "Невхождение" в архивы - неверное условие на равенство (в 706-м забыл
      этот момент исправить)
  10.06.2001 IS
    + Покажем текущее имя кодовой таблицы в диалоге параметров поиска в файлах.
  09.06.2001 IS
    ! При переходе к найденному не меняем каталог, если мы уже в нем находимся.
      Тем самым добиваемся того, что выделение с элементов панели не
      сбрасывается.
  05.06.2001 SVS
    + Обработчик диалога (без него нажатие на пимпу "[View]" заваливает ФАР)
  04.06.2001 OT
     Подпорка для "естественного" обновления экрана
  03.06.2001 SVS
    ! Изменения в связи с переделкой UserData в VMenu
  30.05.2001 OT
    ! Процессор грузится на 100% после всех найденных файлов, возврат к старому :(... до лучших времен
  26.05.2001 OT
    ! Починка AltF7 в NFZ
  25.05.2001 DJ
    - ставим правильный цвет для disabled строчек
  21.05.2001 SVS
    ! struct MenuData|MenuItem
      Поля Selected, Checked, Separator и Disabled преобразованы в DWORD Flags
    ! Константы MENU_ - в морг
  16.05.2001 DJ
    ! proof-of-concept
  15.05.2001 OT
    ! NWZ -> NFZ
  14.05.2001 DJ
    * дизейблим, а не прячем Search in archives на плагиновой панели
    * колесо чтоб работало :-)
  12.05.2001 DJ
    * курсор не останавливается на пустых строках между каталогами
  10.05.2001 DJ
    + поддержка F6 во вьюере/редакторе, вызванных из Find files
  06.05.2001 DJ
    ! перетрях #include
  05.05.2001 DJ
    + перетрях NWZ
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  30.03.2001 SVS
    ! GetLogicalDrives заменен на FarGetLogicalDrives() в связи с началом
      компании по поддержке виндовой "полиции".
  28.02.2001 IS
    ! "CtrlObject->CmdLine." -> "CtrlObject->CmdLine->"
  27.02.2001 VVM
    ! Символы, зависимые от кодовой страницы
      /[\x01-\x08\x0B-\x0C\x0E-\x1F\xB0-\xDF\xF8-\xFF]/
      переведены в коды.
  11.02.2001 SVS
    ! Несколько уточнений кода в связи с изменениями в структуре MenuItem
  14.12.2000 OT
    -  баг: поиск по Alt-F7 с очень длинной маской поиска/текста
  11.11.2000 SVS
    ! FarMkTemp() - убираем (как всегда - то ставим, то тут же убираем :-(((
  11.11.2000 SVS
    ! Используем конструкцию FarMkTemp()
  21.10.2000 SVS
    ! Добавка для поиска в FFFE-файлах.
  10.09.2000 SVS
    - Запрещаем двигать диалог результатов поиска!
  07.08.2000 KM
    - Глюк в поиске при запароленном архиве, если после получения
      запроса на ввод пароля понажимать долго стрелки вверх или вниз
      или пошевелить мышкой, диалог запроса пароля исчезал.
  05.08.2000 KM
    - Перерисовка каталога поиска в Alt-F7, если пользователь во время
      поиска нажимает стрелки вправо или влево.
  03.08.2000 KM
    + Добавлена возможность поиска по "Целым словам"
  01.08.2000 tran 1.03
    + |DIF_USELASTHISTORY
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

#include "findfile.hpp"
#include "plugin.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "editor.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "filelist.hpp"
#include "cmdline.hpp"
#include "chgprior.hpp"
#include "namelist.hpp"
#include "scantree.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "scrbuf.hpp"
#include "CFileMask.hpp"

static void _cdecl PrepareFilesList(void *Param);
static void _cdecl PreparePluginList(void *Param);
static void ScanPluginTree();
static int LookForString(char *Name);
static int IsFileIncluded(PluginPanelItem *FileItem,char *FullName,DWORD FileAttr);
static void ArchiveSearch(char *ArcName);
static void AddMenuRecord(char *FullName,char *Path,WIN32_FIND_DATA *FindData);
void RereadPlugin(HANDLE hPlugin);

static char FindMask[NM],FindStr[SEARCHSTRINGBUFSIZE];
static VMenu *FilesListMenu;
/* $ 30.07.2000 KM
   Добавлена переменная WholeWords для поиска по точному совпадению
*/
static int SearchMode,CmpCase,WholeWords,UseAllTables,SearchInArchives;
/* KM $ */
static int DlgWidth;
static volatile int StopSearch,SearchDone,LastFoundNumber,FileCount;
static char FindMessage[200],LastDirName[NM];
static int FindMessageReady,FindCountReady;
static char PluginSearchPath[2*NM];
static char FindFileArcName[NM];
static HANDLE hPlugin;
static struct OpenPluginInfo Info;
static int RecurseLevel;
static int BreakMainThread;
/* $ 07.08.2000 KM
   Добавление переменной для борьбы с глюком при поиске в запароленном архиве
*/
static int IsPluginGetsFile;
/* KM $ */

static int UseDecodeTable,TableNum,UseUnicode;
static struct CharTableSet TableSet;

/* $ 01.07.2001 IS
   Объект "маска файлов". Именно его будем использовать для проверки имени
   файла на совпадение с искомым.
*/
static CFileMask FileMaskForFindFile;
/* IS $ */

struct ListItemUserData{
  WIN32_FIND_DATA FileFindData;
  char FindFileArcName[NM];
  BYTE Addons[10];
};


FindFiles::FindFiles()
{
  static char LastFindMask[NM]="*.*",LastFindStr[SEARCHSTRINGBUFSIZE];
  /* $ 30.07.2000 KM
     Добавлена переменная LastWholeWords для поиска по точному совпадению
  */
  static int LastCmpCase=0,LastWholeWords=0,LastUseAllTables=0,LastSearchInArchives=0;
  /* KM $ */
  FindSaveScr=NULL;
  CmpCase=LastCmpCase;
  WholeWords=LastWholeWords;
  UseAllTables=LastUseAllTables;
  SearchInArchives=LastSearchInArchives;
  SearchMode=Opt.FileSearchMode;
  strcpy(FindMask,LastFindMask);
  strcpy(FindStr,LastFindStr);
  BreakMainThread=0;
  /* $ 07.08.2000 KM
     Инициализация переменной для борьбы с глюком при поиске в запароленном архиве
  */
  IsPluginGetsFile=0;
  /* KM $ */
  do
  {
    const char *MasksHistoryName="Masks",*TextHistoryName="SearchText";
    static char H2Separator[72]={0};
    /* $ 30.07.2000 KM
       Добавлен новый checkbox "Whole words" в диалог поиска
    */
    static struct DialogData FindAskDlgData[]=
    {
      /* 00 */DI_DOUBLEBOX,3,1,72,20,0,0,0,0,(char *)MFindFileTitle,
      /* 01 */DI_TEXT,5,2,0,0,0,0,0,0,(char *)MFindFileMasks,
      /* 02 */DI_EDIT,5,3,70,16,1,(DWORD)MasksHistoryName,DIF_HISTORY|DIF_USELASTHISTORY,0,"",
      /* 03 */DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR,0,"",
      /* 04 */DI_TEXT,5,5,0,0,0,0,0,0,(char *)MFindFileText,
      /* 05 */DI_EDIT,5,6,70,16,0,(DWORD)TextHistoryName,DIF_HISTORY,0,"",
      /* 06 */DI_TEXT,3,7,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
      /* 07 */DI_CHECKBOX,5,8,0,0,0,0,0,0,(char *)MFindFileCase,
      /* 08 */DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MFindFileWholeWords,
      /* 09 */DI_CHECKBOX,5,10,0,0,0,0,0,0,"",
      /* 10 */DI_CHECKBOX,5,11,0,0,0,0,0,0,(char *)MFindArchives,
      /* 11 */DI_TEXT,3,12,0,0,0,0,DIF_BOXCOLOR,0,"",
      /* 12 */DI_RADIOBUTTON,5,13,0,0,0,0,DIF_GROUP,0,(char *)MSearchAllDisks,
      /* 13 */DI_RADIOBUTTON,5,14,0,0,0,1,0,0,(char *)MSearchFromRoot,
      /* 14 */DI_RADIOBUTTON,5,15,0,0,0,0,0,0,(char *)MSearchFromCurrent,
      /* 15 */DI_RADIOBUTTON,5,16,0,0,0,0,0,0,(char *)MSearchInCurrent,
      /* 16 */DI_RADIOBUTTON,5,17,0,0,0,0,0,0,(char *)MSearchInSelected,
      /* 17 */DI_TEXT,3,18,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
      /* 18 */DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,1,(char *)MFindFileFind,
      /* 19 */DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,0,(char *)MFindFileTable,
      /* 20 */DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
    };
    /* KM $ */
    MakeDialogItems(FindAskDlgData,FindAskDlg);

    /* $ 27.02.2001 SVS
       Динамически сформируем разделительную линию */
    if(!H2Separator[0])
      MakeSeparator(70,H2Separator,3);

    strcpy(FindAskDlg[3].Data,H2Separator);
    strcpy(FindAskDlg[11].Data,H2Separator);
    /* SVS $ */

    {
      Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
      int PluginMode=ActivePanel->GetMode()==PLUGIN_PANEL && ActivePanel->IsVisible();

      if (PluginMode)
      {
        struct OpenPluginInfo Info;
        HANDLE hPlugin=ActivePanel->GetPluginHandle();
        CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
        /* $ 14.05.2001 DJ
           дизейблим, а не прячем
        */
        if ((Info.Flags & OPIF_REALNAMES)==0)
          FindAskDlg[10].Flags |= DIF_DISABLE;
        /* DJ $ */
      }
    }

    strcpy(FindAskDlg[2].Data,FindMask);
    strcpy(FindAskDlg[5].Data,FindStr);
    FindAskDlg[7].Selected=CmpCase;
    FindAskDlg[8].Selected=WholeWords;
    FindAskDlg[9].Selected=UseAllTables;
    /* $ 14.05.2001 DJ
       не селектим чекбокс, если нельзя искать в архивах
    */
    if (!(FindAskDlg[10].Flags & DIF_DISABLE))
      FindAskDlg[10].Selected=SearchInArchives;
    /* DJ $ */
    FindAskDlg[12].Selected=FindAskDlg[13].Selected=0;
    FindAskDlg[14].Selected=FindAskDlg[15].Selected=0;
    FindAskDlg[16].Selected=0;
    FindAskDlg[12+SearchMode].Selected=1;

    while (1)
    {
      Dialog Dlg(FindAskDlg,sizeof(FindAskDlg)/sizeof(FindAskDlg[0]));

      /* $ 10.06.2001 IS
         + Покажем текущее имя кодовой таблицы
      */
      {
        char *fmt=MSG(MFindFileAllTables), TableName[128];

        if(!UseDecodeTable)
          strcpy(TableSet.TableName, MSG(MGetTableNormalText));
        else if(UseUnicode)
          strcpy(TableSet.TableName, "Unicode");
        else
          PrepareTable(&TableSet,TableNum-2);

        int MsgLength=62-strlen(fmt), TableLength=strlen(TableSet.TableName);
        if (MsgLength+3<TableLength)
        {
          strncpy(TableName, TableSet.TableName, MsgLength);
          strcpy(TableName+MsgLength,"...");
        }
        else
          strcpy(TableName, TableSet.TableName);

        sprintf(FindAskDlg[9].Data, fmt, TableName);
      }
      /* IS $ */

      Dlg.SetHelp("FindFile");
      Dlg.SetPosition(-1,-1,76,22);
      Dlg.Process();
      int ExitCode=Dlg.GetExitCode();
      if (ExitCode==19)
      {
        UseUnicode=TRUE;
        int GetTableCode=GetTable(&TableSet,FALSE,TableNum,UseUnicode);
        if (GetTableCode!=-1)
        {
          UseDecodeTable=GetTableCode;
          FindAskDlg[9].Selected=FALSE;
        }
        FindAskDlg[2].Focus=1;
        FindAskDlg[19].Focus=0;
        continue;
      }

      if (ExitCode!=18)
        return;
      /* $ 01.07.2001 IS
         Проверим маску на корректность
      */
      if(!FindAskDlg[2].Data)             // если строка с масками пуста,
         strcpy(FindAskDlg[2].Data, "*"); // то считаем, что маска есть "*"

      if(FileMaskForFindFile.Set(FindAskDlg[2].Data, 0))
           break;
      /* IS $ */
    }
    /* $ 14.12.2000 OT */
    char Buf1 [24];
    char Buf2 [128];
    if (strlen (FindAskDlg[2].Data) > sizeof(FindMask) ){
      memset (Buf1, 0, sizeof(Buf1));
      memset (Buf2, 0, sizeof(Buf2));
      strncpy (Buf1, MSG(MFindFileMasks), sizeof(Buf1)-1);
      sprintf (Buf2,MSG(MEditInputSize), Buf1, sizeof(FindMask)-1);
      Message(MSG_WARNING,1,MSG(MWarning),
        Buf2,
        MSG(MOk));
    }
    strncpy(FindMask,*FindAskDlg[2].Data ? FindAskDlg[2].Data:"*",sizeof(FindMask)-1);
    /* $ 01.07.2001 IS Кавычки уберутся в другом месте - в CFileMask */
    //Unquote(FindMask);
    /* IS $ */
    if (strlen (FindAskDlg[5].Data) > sizeof(FindStr) ){
      memset (Buf1, 0, sizeof(Buf1));
      memset (Buf2, 0, sizeof(Buf2));
      strncpy (Buf1, MSG(MFindFileText), sizeof(Buf1)-1);
      sprintf (Buf2,MSG(MEditInputSize), Buf1, sizeof(FindStr)-1);
      Message(MSG_WARNING,1,MSG(MWarning),
        Buf2,
        MSG(MOk));
    }
    strncpy(FindStr,FindAskDlg[5].Data,sizeof(FindStr)-1);
    /* OT $ */
    CmpCase=FindAskDlg[7].Selected;
    /* $ 30.07.2000 KM
       Добавлена переменная
    */
    WholeWords=FindAskDlg[8].Selected;
    /* KM $ */
    UseAllTables=FindAskDlg[9].Selected;
    SearchInArchives=FindAskDlg[10].Selected;
    if (*FindStr)
    {
      strcpy(GlobalSearchString,FindStr);
      GlobalSearchCase=CmpCase;
      /* $ 30.07.2000 KM
         Добавлена переменная
      */
      GlobalSearchWholeWords=WholeWords;
      /* KM $ */
    }
    if (FindAskDlg[12].Selected)
      SearchMode=SEARCH_ALL;
    if (FindAskDlg[13].Selected)
      SearchMode=SEARCH_ROOT;
    if (FindAskDlg[14].Selected)
      SearchMode=SEARCH_FROM_CURRENT;
    if (FindAskDlg[15].Selected)
      SearchMode=SEARCH_CURRENT_ONLY;
    if (FindAskDlg[16].Selected)
        SearchMode=SEARCH_SELECTED;
    Opt.FileSearchMode=SearchMode;
    LastCmpCase=CmpCase;
    /* $ 30.07.2000 KM
       Добавлена переменная
    */
    LastWholeWords=WholeWords;
    /* KM $ */
    LastUseAllTables=UseAllTables;
    LastSearchInArchives=SearchInArchives;
    strcpy(LastFindMask,FindMask);
    strcpy(LastFindStr,FindStr);
    if (*FindStr)
      Editor::SetReplaceMode(FALSE);
  } while (FindFilesProcess());
}


FindFiles::~FindFiles()
{
  /* $ 01.07.2001 IS
     Освободим память.
  */
  FileMaskForFindFile.Set(NULL, FMF_SILENT);
  /* IS $ */
  ScrBuf.ResetShadow();
  delete FindSaveScr;
}

long WINAPI FindFiles::FindDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  if(Msg == DN_BTNCLICK)
  {
    if(Param1 == 6)
    {
      ((Dialog*)hDlg)->SetExitCode(6);
      return TRUE;
    }
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

int FindFiles::FindFilesProcess()
{
  FindSaveScr=new SaveScreen;

  static struct DialogData FindDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,72,19,0,0,0,0,(char *)MFindFileTitle,
  /* 01 */DI_TEXT,3,15,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 02 */DI_TEXT,5,16,0,0,0,0,0,0,(char *)MFindSearchingIn,
  /* 03 */DI_TEXT,3,17,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 04 */DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MFindNewSearch,
  /* 05 */DI_BUTTON,0,18,0,0,1,0,DIF_CENTERGROUP,1,(char *)MFindGoTo,
  /* 06 */DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MFindView,
  /* 07 */DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MFindPanel,
  /* 08 */DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MFindStop
  };
  MakeDialogItems(FindDlgData,FindDlg);

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  struct ListItemUserData UserDataItem;
  int FindListDataSize,DlgExitCode,ListPosChanged=FALSE;

  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  int PluginMode=ActivePanel->GetMode()==PLUGIN_PANEL && ActivePanel->IsVisible();

  int IncY=ScrY>24 ? ScrY-24:0;
  FindDlg[0].Y2+=IncY;
  FindDlg[1].Y1+=IncY;
  FindDlg[2].Y1+=IncY;
  FindDlg[3].Y1+=IncY;
  FindDlg[4].Y1+=IncY;
  FindDlg[5].Y1+=IncY;
  FindDlg[6].Y1+=IncY;
  FindDlg[7].Y1+=IncY;
  FindDlg[8].Y1+=IncY;

  if (PluginMode)
  {
    hPlugin=ActivePanel->GetPluginHandle();
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
    if ((Info.Flags & OPIF_REALNAMES)==0)
    {
      FindDlg[6].Type=FindDlg[7].Type=DI_TEXT;
      *FindDlg[6].Data=*FindDlg[7].Data=0;
    }
  }
  else
    hPlugin=NULL;

  {
    DlgWidth=FindDlg[0].X2-FindDlg[0].X1-4;
    Dialog *pDlg=new Dialog(FindDlg,sizeof(FindDlg)/sizeof(FindDlg[0]),FindDlgProc);
    pDlg->SetDynamicallyBorn(TRUE);
    pDlg->SetHelp("FindFile");
    /* $ 10.09.2000 SVS
       Запрещаем двигать диалог
    */
    pDlg->SetModeMoving(FALSE);
    /* SVS $ */
    pDlg->SetPosition(-1,-1,76,21+IncY);
    pDlg->Show();

    VMenu FindList("",NULL,0,13+IncY);
    FindList.SetDialogStyle(TRUE);
    FindList.SetBoxType(NO_BOX);
    FindList.SetFlags(VMENU_DISABLEDRAWBACKGROUND|VMENU_SHOWAMPERSAND);

    /* $ 25.05.2001 DJ
       ставим правильный цвет для disabled строчек
    */
    FindList.SetOneColor (VMenuColorDisabled, COL_DIALOGTEXT);
    /* DJ $ */

    int DlgX1,DlgY1,DlgX2,DlgY2;
    pDlg->GetPosition(DlgX1,DlgY1,DlgX2,DlgY2);
    FindList.SetPosition(DlgX1+4,DlgY1+1,DlgX2-4,DlgY2-5);
    FindList.Show();
    LastFoundNumber=0;

    SearchDone=FALSE;
    StopSearch=FALSE;
    FileCount=0;
    *FindMessage=*LastDirName=FindMessageReady=FindCountReady=0;
    FilesListMenu=&FindList;
    if (PluginMode)
    {
      if (_beginthread(PreparePluginList,0x10000,NULL)==(unsigned long)-1)
        return(FALSE);
    }
    else
      if (_beginthread(PrepareFilesList,0x10000,NULL)==(unsigned long)-1)
        return(FALSE);

    while (1)
    {
      if (BreakMainThread)
      {
        DlgExitCode=-1;
        break;
      }
      /* $ 07.08.2000 KM
         Добавление кода для борьбы с глюком при поиске в запароленном архиве
      */
      if (IsPluginGetsFile)
      {
        Sleep(100);
        continue;
      }
      /* KM $ */

      INPUT_RECORD rec;
      int Key;

      if (pDlg->Done())
      {
        DlgExitCode=pDlg->GetExitCode();
        switch (DlgExitCode)
        {
          case 4:
          case 7:
            break;
          case 6:
          case 100:
            {
              if ((FindList.GetUserData(&UserDataItem,sizeof(UserDataItem)))!=0 &&
                  FindList.GetUserDataSize()==sizeof(UserDataItem))
              {
                DWORD FileAttr;
                if ((FileAttr=GetFileAttributes(UserDataItem.FileFindData.cFileName))!=(DWORD)-1 &&
                    (FileAttr & FA_DIREC)==0)
                {
                  char OldTitle[512];
                  GetConsoleTitle(OldTitle,sizeof(OldTitle));
                  if (DlgExitCode==6)
                  {
                    NamesList ViewList;
                    if (!PluginMode || (Info.Flags & OPIF_REALNAMES))
                    {
                      int ListSize=FindList.GetItemCount();
                      struct ListItemUserData ListFindData;
                      for (int I=0;I<ListSize;I++)
                        if ((FindList.GetUserData(&ListFindData,sizeof(ListFindData),I))!=0 &&
                             FindList.GetUserDataSize(I)==sizeof(ListFindData))
                        {
                          int Length=strlen(ListFindData.FileFindData.cFileName);
                          if (Length>0 && ListFindData.FileFindData.cFileName[Length-1]!='\\')
                            ViewList.AddName(ListFindData.FileFindData.cFileName);
                        }
                      ViewList.SetCurName(UserDataItem.FileFindData.cFileName);
                    }
                    FindList.Hide();
                    pDlg->Hide();
                    {
                      (*FrameManager)[0]->UnlockRefresh();
                      FileViewer ShellViewer (UserDataItem.FileFindData.cFileName,FALSE,FALSE,FALSE,-1,NULL,&ViewList);//?
                      ShellViewer.SetDynamicallyBorn(FALSE);
                      FrameManager->ExecuteModal ();
                      (*FrameManager)[0]->LockRefresh();
                    }
                    pDlg->Show();
                    FindList.Show();
                  }
                  else
                  {
                    FindList.Hide();
                    pDlg->Hide();
                    {
                      (*FrameManager)[0]->UnlockRefresh();
                      FileEditor ShellEditor (UserDataItem.FileFindData.cFileName,FALSE,FALSE);
                      ShellEditor.SetDynamicallyBorn(FALSE);
                      ShellEditor.SetEnableF6 (TRUE);
                      FrameManager->ExecuteModal ();
                      (*FrameManager)[0]->LockRefresh();
                    }
                    pDlg->Show();
                    FindList.Show();
                  }
                  SetConsoleTitle(OldTitle);
                }
              }
            }
            pDlg->ClearDone();
            pDlg->Show();
            FindList.SetUpdateRequired(TRUE);
            FindList.Show();
            continue;
          case 8:
            if (SearchDone)
              return(FALSE);
            StopSearch=TRUE;
            pDlg->ClearDone();
            continue;
        }
        break;
      }

      if (LastFoundNumber>=0)
      {
        LastFoundNumber=0;
        FindList.Show();
      }

      int MacroKey=CtrlObject->Macro.PeekKey();
      if (MacroKey)
        ScrBuf.Flush();
      if ((!MacroKey || SearchDone) && (MacroKey || PeekInputRecord(&rec)))
      {
        Key=GetInputRecord(&rec);
        if (!MacroKey && rec.EventType==MOUSE_EVENT)
        {
          if (FindList.ProcessMouse(&rec.Event.MouseEvent))
          {
            if (rec.Event.MouseEvent.dwEventFlags==DOUBLE_CLICK)
              pDlg->ProcessKey(KEY_ENTER);
          }
          else
            pDlg->ProcessMouse(&rec.Event.MouseEvent);
        }
        else
        {
          if (!MacroKey && !rec.Event.KeyEvent.bKeyDown)
            continue;
          if (!PluginMode || (Info.Flags & OPIF_REALNAMES))
          {
            if (Key==KEY_F3 || Key==KEY_NUMPAD5)
            {
              pDlg->SetExitCode(6);
              continue;
            }
            if (Key==KEY_F4)
            {
              pDlg->SetExitCode(100);
              continue;
            }
          }
          /* $ 14.05.2001 DJ
             колесо чтоб работало :-)
          */
          if (Key==KEY_UP || Key==KEY_DOWN || Key==KEY_PGUP ||
              Key==KEY_PGDN || Key==KEY_HOME || Key==KEY_END ||
              Key==KEY_MSWHEEL_UP || Key==KEY_MSWHEEL_DOWN)
          {
            ListPosChanged=TRUE;
            FindList.ProcessKey(Key);
          }
          /* DJ $ */
          else
          {
            if (Key!=KEY_ENTER || !FindDlg[5].Focus || FindList.GetUserDataSize())
              if (pDlg->ProcessKey(Key))
              {
                FindList.SetUpdateRequired(TRUE);
                FindList.Show();
                /* $ 05.08.2000 KM
                   При движении курсорных клавиш в диалоге во время поиска
                   диалог перерисовывается и строка, показывающая текущее
                   место поиска исчезает. Инициализация этой переменной
                   заставляет фар перерисовать эту строку заново (она рисуется поверх
                   диалога, вместо внесения этой строки в контрол диалога - для
                   ускорения процедуры поиска).
                */
                if (!SearchDone)
                  FindMessageReady=TRUE;
                /* KM $ */
              }
          }
        }
      }
      else
      {
        if (FindCountReady)
        {
          int DlgX1,DlgY1,DlgX2,DlgY2;
          pDlg->GetPosition(DlgX1,DlgY1,DlgX2,DlgY2);
          char Msg[256];
          sprintf(Msg," %s: %d ",MSG(MFindFound),FileCount);
          GotoXY(DlgX1+(DlgX2-DlgX1-strlen(Msg)+1)/2,DlgY1+FindDlg[1].Y1);
          SetColor(COL_DIALOGTEXT);
          Text(Msg);
          FindCountReady=FALSE;
        }

        if (FindMessageReady)
        {
          if (SearchDone)
          {
            sprintf(FindDlg[2].Data,"%-*.*s",DlgWidth,DlgWidth,FindMessage);
            strcpy(FindDlg[8].Data,MSG(MFindCancel));
            pDlg->InitDialogObjects();
            pDlg->Show();
            FindList.SetUpdateRequired(TRUE);
            FindList.Show();
            SetFarTitle(FindMessage);
          }
          else
          {
            int DlgX1,DlgY1,DlgX2,DlgY2,TextX;
            pDlg->GetPosition(DlgX1,DlgY1,DlgX2,DlgY2);
            TextX=DlgX1+FindDlg[2].X1+strlen(FindDlg[2].Data)+1;
            GotoXY(TextX,DlgY1+FindDlg[2].Y1);
            SetColor(COL_DIALOGTEXT);

            char Msg[512];
            int Width=DlgX2-TextX-5;
            strcpy(Msg,FindMessage);
            TruncStr(Msg,Width);
            mprintf("%-*s",Width,Msg);
          }
          FindMessageReady=FALSE;
        }
                /* $ 30.05.2001 OT возврат к старому стилю */
        Sleep(50);
        /* OT $ */
      }
    }

    FindListDataSize=FindList.GetUserDataSize();
    FindList.GetUserData(&UserDataItem,sizeof(UserDataItem));
    StopSearch=TRUE;
    while (!SearchDone)
      Sleep(10);

    FindList.Hide();
    pDlg->Hide();

    delete FindSaveScr;
    FindSaveScr=NULL;

    if (DlgExitCode==7 && (hPlugin==NULL || (Info.Flags & OPIF_REALNAMES)))
    {
      int ListSize=FindList.GetItemCount();
      PluginPanelItem *PanelItems=new PluginPanelItem[ListSize];
      if (PanelItems==NULL)
        ListSize=0;
      int ItemsNumber=0;
      for (int I=0;I<ListSize;I++)
        if ((FindList.GetUserData(&UserDataItem,sizeof(UserDataItem),I))!=0 &&
            FindList.GetUserDataSize(I)==sizeof(UserDataItem))
        {
          int Length=strlen(UserDataItem.FileFindData.cFileName);
          if (Length>0 && UserDataItem.FileFindData.cFileName[Length-1]!='\\')
          {
            PluginPanelItem *pi=&PanelItems[ItemsNumber++];
            memset(pi,0,sizeof(*pi));
            pi->FindData=UserDataItem.FileFindData;
          }
        }

      HANDLE hNewPlugin=CtrlObject->Plugins.OpenFindListPlugin(PanelItems,ItemsNumber);
      if (hNewPlugin!=INVALID_HANDLE_VALUE)
      {
        Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
        Panel *NewPanel=CtrlObject->Cp()->ChangePanel(ActivePanel,FILE_PANEL,TRUE,TRUE);
        NewPanel->SetPluginMode(hNewPlugin,"");
        NewPanel->Update(0);
        if (ListPosChanged && FindList.GetUserData(&UserDataItem,sizeof(UserDataItem)))
          NewPanel->GoToFile(UserDataItem.FileFindData.cFileName);
        NewPanel->Show();
        NewPanel->SetFocus();
        hPlugin=NULL;
      }
      /* $ 13.07.2000 SVS
         использовали new[]
      */
      delete[] PanelItems;
      /* SVS $ */
    }
  }

  switch (DlgExitCode)
  {
    case 4:
      return(TRUE);
    case 5:
      if (FindListDataSize!=0)
      {
        char *FileName=UserDataItem.FileFindData.cFileName;
        Panel *FindPanel=CtrlObject->Cp()->ActivePanel;
        if (UserDataItem.FindFileArcName[0])
        {
          char ArcName[NM],ArcPath[NM];
          strncpy(ArcName,UserDataItem.FindFileArcName,NM);
          if (FindPanel->GetType()!=FILE_PANEL)
            FindPanel=CtrlObject->Cp()->ChangePanel(FindPanel,FILE_PANEL,TRUE,TRUE);
          strcpy(ArcPath,ArcName);
          *PointToName(ArcPath)=0;
          FindPanel->SetCurDir(ArcPath,TRUE);
          hPlugin=((FileList *)FindPanel)->OpenFilePlugin(ArcName,FALSE);
          if (hPlugin==INVALID_HANDLE_VALUE || hPlugin==(HANDLE)-2)
            return(FALSE);
        }
        if (hPlugin)
        {
          SetPluginDirectory(FileName);
          return(FALSE);
        }
        char SetName[NM];
        int Length;
        if ((Length=strlen(FileName))==0)
          break;
        if (Length>1 && FileName[Length-1]=='\\' && FileName[Length-2]!=':')
          FileName[Length-1]=0;
        if (GetFileAttributes(FileName)==(DWORD)-1)
          break;

        {
          char *NamePtr;
          NamePtr=PointToName(FileName);
          strcpy(SetName,NamePtr);
          *NamePtr=0;
          Length=strlen(FileName);
          if (Length>1 && FileName[Length-1]=='\\' && FileName[Length-2]!=':')
            FileName[Length-1]=0;
        }

        if (*FileName==0)
          break;
        if (FindPanel->GetType()!=FILE_PANEL &&
            CtrlObject->Cp()->GetAnotherPanel(FindPanel)->GetType()==FILE_PANEL)
          FindPanel=CtrlObject->Cp()->GetAnotherPanel(FindPanel);
        /* $ 09.06.2001 IS
           ! Не меняем каталог, если мы уже в нем находимся. Тем самым
             добиваемся того, что выделение с элементов панели не сбрасывается.
        */
        {
          char DirTmp[NM];
          FindPanel->GetCurDir(DirTmp);
          Length=strlen(DirTmp);
          if (Length>1 && DirTmp[Length-1]=='\\' && DirTmp[Length-2]!=':')
            DirTmp[Length-1]=0;
          if(0!=LocalStricmp(FileName, DirTmp))
            FindPanel->SetCurDir(FileName,TRUE);
        }
        /* IS $ */
        if (*SetName)
          FindPanel->GoToFile(SetName);
        FindPanel->Show();
      }
      break;
  }
  return(FALSE);
}


void FindFiles::SetPluginDirectory(char *FileName)
{
  char Name[NM],*StartName,*EndName;
  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  if (SearchMode==SEARCH_ROOT || SearchMode==SEARCH_ALL)
    CtrlObject->Plugins.SetDirectory(hPlugin,"\\",OPM_FIND);
  strcpy(Name,FileName);
  StartName=Name;
  while ((EndName=strchr(StartName,'\x1'))!=NULL ||
         (Info.Flags & OPIF_REALNAMES) && (EndName=strchr(StartName,'\\'))!=NULL)

  {
    *EndName=0;
    RereadPlugin(hPlugin);
    CtrlObject->Plugins.SetDirectory(hPlugin,StartName,OPM_FIND);
    StartName=EndName+1;
  }
  ActivePanel->Update(UPDATE_KEEP_SELECTION);
  if (!ActivePanel->GoToFile(StartName))
    ActivePanel->GoToFile(FileName);
  ActivePanel->Show();
}


#if defined(__BORLANDC__)
#pragma warn -par
#endif
void _cdecl PrepareFilesList(void *Param)
{
  WIN32_FIND_DATA FindData;
  char FullName[NM],Root[NM];

  DWORD DiskMask=FarGetLogicalDrives();
  CtrlObject->CmdLine->GetCurDir(Root);

  for (int CurrentDisk=0;DiskMask!=0;CurrentDisk++,DiskMask>>=1)
  {
    if (SearchMode==SEARCH_ALL)
    {
      if ((DiskMask & 1)==0)
        continue;
      sprintf(Root,"%c:\\",'A'+CurrentDisk);
      int DriveType=GetDriveType(Root);
      if (DriveType==DRIVE_REMOVABLE || DriveType==DRIVE_CDROM)
        if (DiskMask==1)
          break;
        else
          continue;
    }
    else
      if (SearchMode==SEARCH_ROOT)
        GetPathRoot(Root,Root);

    ScanTree ScTree(FALSE,SearchMode!=SEARCH_CURRENT_ONLY);

    char SelName[NM];
    int FileAttr;
    if (SearchMode==SEARCH_SELECTED)
      CtrlObject->Cp()->ActivePanel->GetSelName(NULL,FileAttr);

    while (1)
    {
      char CurRoot[2*NM];
      if (SearchMode==SEARCH_SELECTED)
      {
        if (!CtrlObject->Cp()->ActivePanel->GetSelName(SelName,FileAttr))
          break;
        if ((FileAttr & FA_DIREC)==0 || strcmp(SelName,"..")==0 ||
            strcmp(SelName,".")==0)
          continue;
        strcpy(CurRoot,Root);
        AddEndSlash(CurRoot);
        strcat(CurRoot,SelName);
      }
      else
        strcpy(CurRoot,Root);

      ScTree.SetFindPath(CurRoot,"*.*");

      strncpy(FindMessage,CurRoot,sizeof(FindMessage));
      FindMessage[sizeof(FindMessage)-1]=0;
      FindMessageReady=TRUE;

      while (!StopSearch && ScTree.GetNextName(&FindData,FullName))
      {
        if (FindData.dwFileAttributes & FA_DIREC)
        {
          strncpy(FindMessage,FullName,sizeof(FindMessage));
          FindMessage[sizeof(FindMessage)-1]=0;
          FindMessageReady=TRUE;
        }
        if (IsFileIncluded(NULL,FullName,FindData.dwFileAttributes))
          AddMenuRecord(FullName,NULL,&FindData);
        if (SearchInArchives)
          ArchiveSearch(FullName);
      }
      if (SearchMode!=SEARCH_SELECTED)
        break;
    }
    if (SearchMode!=SEARCH_ALL)
      break;
  }

  while (!StopSearch && FindMessageReady)
    Sleep(10);
  sprintf(FindMessage,MSG(MFindDone),FileCount);
  SearchDone=TRUE;
  FindMessageReady=TRUE;
//  _endthread(); излишество... по умолчанию процесс должен закончиться автоматом по выходу из процедуры
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif


void ArchiveSearch(char *ArcName)
{
  const int MaxRead=0x20000;
  char *Buffer=new char[MaxRead];
  FILE *ProcessFile=fopen(ArcName,"rb");
  if (ProcessFile==NULL)
  {
    /* $ 13.07.2000 SVS
       использовали new[]
    */
    delete[] Buffer;
    /* SVS $ */
    return;
  }
  int ReadSize=fread(Buffer,1,MaxRead,ProcessFile);
  fclose(ProcessFile);

  DisablePluginsOutput=TRUE;
  HANDLE hArc=CtrlObject->Plugins.OpenFilePlugin(ArcName,(unsigned char *)Buffer,ReadSize);
  DisablePluginsOutput=FALSE;

  /* $ 13.07.2000 SVS
     использовали new[]
  */
  delete[] Buffer;
  /* SVS $ */

  if (hArc==(HANDLE)-2)
  {
    BreakMainThread=TRUE;
    return;
  }
  if (hArc==INVALID_HANDLE_VALUE)
    return;
  int SaveSearchMode=SearchMode;
  HANDLE SaveHandle=hPlugin;
  SearchMode=SEARCH_FROM_CURRENT;
  hPlugin=hArc;
  CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
  strcpy(FindFileArcName,ArcName);
  *LastDirName=0;
  PreparePluginList((void *)1);
  *FindFileArcName=0;
  SearchMode=SaveSearchMode;
  CtrlObject->Plugins.ClosePlugin(hPlugin);
  hPlugin=SaveHandle;
}

/* $ 01.07.2001 IS
   Используем FileMaskForFindFile вместо GetCommaWord
*/
int IsFileIncluded(PluginPanelItem *FileItem,char *FullName,DWORD FileAttr)
{
  int FileFound=FileMaskForFindFile.Compare(FullName);
  while(FileFound)
  {
    if ((FileAttr & FA_DIREC) &&
        (hPlugin==NULL || (Info.Flags & OPIF_FINDFOLDERS)==0))
      return FALSE;

    if (*FindStr && FileFound)
    {
      FileFound=FALSE;
      if (FileAttr & FA_DIREC)
        break;
      char SearchFileName[NM];
      int RemoveTemp=FALSE;
      if (hPlugin && (Info.Flags & OPIF_REALNAMES)==0)
      {
        char TempDir[NM];
        sprintf(TempDir,"%s%s",Opt.TempPath,FarTmpXXXXXX);
        mktemp(TempDir);
        CreateDirectory(TempDir,NULL);
        /* $ 07.08.2000 KM
           Добавление переменных для борьбы с глюком при поиске в запароленном архиве
        */
        IsPluginGetsFile=TRUE;
        if (!CtrlObject->Plugins.GetFile(hPlugin,FileItem,TempDir,SearchFileName,OPM_SILENT|OPM_FIND))
        {
          RemoveDirectory(TempDir);
          IsPluginGetsFile=FALSE;
          break;
        }
        RemoveTemp=TRUE;
        IsPluginGetsFile=FALSE;
        /* KM $ */
      }
      else
        strcpy(SearchFileName,FullName);
      if (LookForString(SearchFileName))
        FileFound=TRUE;
      if (RemoveTemp)
        DeleteFileWithFolder(SearchFileName);
    }
    break;
  }
  return(FileFound);
}
/* IS $ */


void AddMenuRecord(char *FullName,char *Path,WIN32_FIND_DATA *FindData)
{
  struct ListItemUserData UserDataItem;
  char MenuText[NM],FileText[NM],SizeText[30];
  char Date[30],DateStr[30],TimeStr[30];
  struct MenuItem ListItem;

  memset(&ListItem,0,sizeof(ListItem));

  if (FindData->dwFileAttributes & FA_DIREC)
    strcpy(SizeText,MSG(MFindFileFolder));
  else
    sprintf(SizeText,"%10u",FindData->nFileSizeLow);

  char *DisplayName=FindData->cFileName;
  if (hPlugin && (Info.Flags & OPIF_REALNAMES))
    DisplayName=PointToName(DisplayName);

  sprintf(FileText," %-30.30s %10.10s",DisplayName,SizeText);
  ConvertDate(&FindData->ftLastWriteTime,DateStr,TimeStr,5);
  sprintf(Date,"    %s   %s",DateStr,TimeStr);
  strcat(FileText,Date);
  sprintf(MenuText," %-*.*s",DlgWidth-3,DlgWidth-3,FileText);

  char PathName[2*NM];
  if (Path!=NULL)
    strcpy(PathName,Path);
  else
  {
    strncpy(PathName,FullName,sizeof(PathName)-1);
    PathName[sizeof(PathName)-1]=0;
    *PointToName(PathName)=0;
  }
  if (*PathName==0)
    strcpy(PathName,".\\");

  if (LocalStricmp(PathName,LastDirName)!=0)
  {
    if (*LastDirName!=0)
    {
      /* $ 12.05.2001 DJ
         курсор не останавливается на пустых строках между каталогами
      */
      ListItem.Flags|=LIF_DISABLE;
      FilesListMenu->AddItem(&ListItem);
      ListItem.Flags&=~LIF_DISABLE;
      /* DJ $ */
    }
    strcpy(LastDirName,PathName);
    if (*FindFileArcName)
    {
      char ArcPathName[NM*2];
      sprintf(ArcPathName,"%s:%s",FindFileArcName,*PathName=='.' ? "\\":PathName);
      strcpy(PathName,ArcPathName);
    }
    sprintf(ListItem.Name,"%-*.*s",DlgWidth-2,DlgWidth-2,PathName);
    for (int I=0;ListItem.Name[I]!=0;I++)
      if (ListItem.Name[I]=='\x1')
        ListItem.Name[I]='\\';

    memset(&UserDataItem,0,sizeof(UserDataItem));
    UserDataItem.FileFindData=*FindData;
    strcpy(UserDataItem.FileFindData.cFileName,PathName);
    if (*FindFileArcName)
      strcpy(UserDataItem.FindFileArcName,FindFileArcName);
    FilesListMenu->SetUserData(&UserDataItem,sizeof(UserDataItem),
            FilesListMenu->AddItem(&ListItem));
  }

  memset(&UserDataItem,0,sizeof(UserDataItem));
  UserDataItem.FileFindData=*FindData;
  strncpy(UserDataItem.FileFindData.cFileName,
          FullName,
          sizeof(UserDataItem.FileFindData.cFileName));
  if (*FindFileArcName)
    strcpy(UserDataItem.FindFileArcName,FindFileArcName);
  strcpy(ListItem.Name,MenuText);
  ListItem.SetSelect(!FileCount);
  FilesListMenu->SetUserData(&UserDataItem,sizeof(UserDataItem),
            FilesListMenu->AddItem(&ListItem));

  LastFoundNumber++;
  FileCount++;
  FindCountReady=TRUE;
}


int LookForString(char *Name)
{
  FILE *SrcFile;
  char Buf[32768],SaveBuf[32768],CmpStr[sizeof(FindStr)];
  int Length,ReadSize;
  if ((Length=strlen(FindStr))==0)
    return(TRUE);
  HANDLE FileHandle=CreateFile(Name,GENERIC_READ|GENERIC_WRITE,
         FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  if (FileHandle==INVALID_HANDLE_VALUE)
    FileHandle=CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                          NULL,OPEN_EXISTING,0,NULL);
  if (FileHandle==INVALID_HANDLE_VALUE)
    return(FALSE);
  int Handle=_open_osfhandle((long)FileHandle,O_BINARY);
  if (Handle==-1)
    return(FALSE);
  if ((SrcFile=fdopen(Handle,"rb"))==NULL)
    return(FALSE);

  FILETIME LastAccess;
  int TimeRead=GetFileTime(FileHandle,NULL,&LastAccess,NULL);
  strcpy(CmpStr,FindStr);
  if (!CmpCase)
    LocalStrupr(CmpStr);
  /* $ 30.07.2000 KM
     Добавочные переменные
  */
  int FirstIteration=TRUE;
  /* KM $ */
  int ReverseBOM=FALSE;
  int IsFirst=FALSE;
  while (!StopSearch && (ReadSize=fread(Buf,1,sizeof(Buf),SrcFile))>0)
  {
    int DecodeTableNum=0;
    int UnicodeSearch=UseUnicode;
    int RealReadSize=ReadSize;

    if (UseAllTables || UseUnicode)
    {
      memcpy(SaveBuf,Buf,ReadSize);

      /* $ 21.10.2000 SVS
         Хреново получилось, в лоб так сказать, но ищет в FFFE-файлах
      */
      if(!IsFirst)
      {
        IsFirst=TRUE;
        if(*(WORD*)Buf == 0xFFFE) // The text contains the Unicode
           ReverseBOM=TRUE;       // byte-reversed byte-order mark
                                  // (Reverse BOM) 0xFFFE as its first character.
      }

      if(ReverseBOM)
      {
        BYTE Chr;
        for(int I=0; I < ReadSize; I+=2)
        {
          Chr=SaveBuf[I];
          SaveBuf[I]=SaveBuf[I+1];
          SaveBuf[I+1]=Chr;
        }
      }
      /* SVS $ */
    }

    while (1)
    {
      if (DecodeTableNum>0 && !UnicodeSearch)
        memcpy(Buf,SaveBuf,ReadSize);
      if (UnicodeSearch)
      {
        WideCharToMultiByte(CP_OEMCP,0,(LPCWSTR)SaveBuf,ReadSize/2,Buf,ReadSize,NULL,NULL);
        ReadSize/=2;
      }
      else
        if (UseDecodeTable || DecodeTableNum>0)
          for (int I=0;I<ReadSize;I++)
            Buf[I]=TableSet.DecodeTable[Buf[I]];
      if (!CmpCase)
        LocalUpperBuf(Buf,ReadSize);
      int CheckSize=ReadSize-Length+1;
      /* $ 30.07.2000 KM
         Обработка "Whole words" в поиске
      */
      for (int I=0;I<CheckSize;I++)
      {
        int cmpResult;
        if (WholeWords)
        {
          int locResultLeft=FALSE;
          int locResultRight=FALSE;
          if (!FirstIteration)
          {
            if (Buf[I]==' ' || Buf[I]=='\t' || Buf[I]=='\n' || Buf[I]=='\r')
              locResultLeft=TRUE;
            if (RealReadSize!=sizeof(Buf) && I+1+Length>=RealReadSize)
              locResultRight=TRUE;
            else
              if (Buf[I+1+Length]==' ' || Buf[I+1+Length]=='\t' ||
                  Buf[I+1+Length]=='\n' || Buf[I+1+Length]=='\r')
                locResultRight=TRUE;

            if (!locResultLeft)
              if (strchr(Opt.WordDiv,Buf[I])!=NULL)
                locResultLeft=TRUE;
            if (!locResultRight)
              if (strchr(Opt.WordDiv,Buf[I+1+Length])!=NULL)
                locResultRight=TRUE;

            cmpResult=locResultLeft && locResultRight && CmpStr[0]==Buf[I+1]
              && (Length==1 || CmpStr[1]==Buf[I+2]
              && (Length==2 || memcmp(CmpStr+2,&Buf[I+3],Length-2)==0));
          }
          else
          {
            FirstIteration=FALSE;

            if (RealReadSize!=sizeof(Buf) && I+Length>=RealReadSize)
              locResultRight=TRUE;
            else
              if (Buf[I+Length]==' ' || Buf[I+Length]=='\t' ||
                  Buf[I+Length]=='\n' || Buf[I+Length]=='\r')
                locResultRight=TRUE;

            if (!locResultRight)
              if (strchr(Opt.WordDiv,Buf[I+1+Length])!=NULL)
                locResultRight=TRUE;

            cmpResult=locResultRight && CmpStr[0]==Buf[I]
              && (Length==1 || CmpStr[1]==Buf[I+1]
              && (Length==2 || memcmp(CmpStr+2,&Buf[I+2],Length-2)==0));
          }
        }
        else
        {
          cmpResult=CmpStr[0]==Buf[I] && (Length==1 || CmpStr[1]==Buf[I+1]
            && (Length==2 || memcmp(CmpStr+2,&Buf[I+2],Length-2)==0));
        }
        if (cmpResult)
        {
          if (TimeRead)
            SetFileTime(FileHandle,NULL,&LastAccess,NULL);
          fclose(SrcFile);
          return(TRUE);
        }
      }
      /* KM $ */
      if (UseAllTables)
      {
        if (PrepareTable(&TableSet,DecodeTableNum++))
        {
          strcpy(CmpStr,FindStr);
          if (!CmpCase)
            LocalStrupr(CmpStr);
        }
        else
          if (!UnicodeSearch)
            UnicodeSearch=true;
          else
            break;
      }
      else
        break;
    }

    if (RealReadSize==sizeof(Buf))
    {
      /* $ 30.07.2000 KM
         Изменение offset при чтении нового блока с учётом WordDiv
      */
      int NewPos;
      if (UnicodeSearch)
        NewPos=ftell(SrcFile)-2*(Length+1);
      else
        NewPos=ftell(SrcFile)-(Length+1);
      fseek(SrcFile,Max(NewPos,0),SEEK_SET);
      /* KM $ */
    }
  }
  if (TimeRead)
    SetFileTime(FileHandle,NULL,&LastAccess,NULL);
  fclose(SrcFile);
  return(FALSE);
}


#if defined(__BORLANDC__)
#pragma warn -par
#endif
void _cdecl PreparePluginList(void *Param)
{
  char SaveDir[NM];
  strcpy(SaveDir,Info.CurDir);
  *PluginSearchPath=0;
  if (SearchMode==SEARCH_ROOT || SearchMode==SEARCH_ALL)
    CtrlObject->Plugins.SetDirectory(hPlugin,"\\",OPM_FIND);
  RecurseLevel=0;
  ScanPluginTree();
  if (SearchMode==SEARCH_ROOT || SearchMode==SEARCH_ALL)
    CtrlObject->Plugins.SetDirectory(hPlugin,SaveDir,OPM_FIND);
  while (!StopSearch && FindMessageReady)
    Sleep(10);
  if (Param==NULL)
  {
    sprintf(FindMessage,MSG(MFindDone),FileCount);
    FindMessageReady=TRUE;
    SearchDone=TRUE;
//ОТ  _endthread(); излишество... по умолчанию процесс должен закончиться автоматом по выходу из процедуры
  }
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif

void ScanPluginTree()
{
  PluginPanelItem *PanelData=NULL;
  int ItemCount=0;
  if (StopSearch || !CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&ItemCount,OPM_FIND))
    return;
  RecurseLevel++;
  if (SearchMode!=SEARCH_SELECTED || RecurseLevel!=1)
    for (int I=0;I<ItemCount && !StopSearch;I++)
    {
      PluginPanelItem *CurPanelItem=PanelData+I;
      char *CurName=CurPanelItem->FindData.cFileName;
      char FullName[2*NM];
      if (strcmp(CurName,".")==0 || strcmp(CurName,"..")==0)
        continue;
      char AddPath[2*NM];
      if (Info.Flags & OPIF_REALNAMES)
      {
        strcpy(FullName,CurName);
        strcpy(AddPath,CurName);
        *PointToName(AddPath)=0;
      }
      else
      {
        sprintf(FullName,"%s%s",PluginSearchPath,CurName);
        strcpy(AddPath,PluginSearchPath);
      }
      if (CurPanelItem->FindData.dwFileAttributes & FA_DIREC)
      {
        strncpy(FindMessage,FullName,sizeof(FindMessage));
        FindMessage[sizeof(FindMessage)-1]=0;
        for (int I=0;FindMessage[I]!=0;I++)
          if (FindMessage[I]=='\x1')
            FindMessage[I]='\\';
        FindMessageReady=TRUE;
      }
      if (IsFileIncluded(CurPanelItem,CurName,CurPanelItem->FindData.dwFileAttributes))
        AddMenuRecord(FullName,AddPath,&CurPanelItem->FindData);
    }
  if (SearchMode!=SEARCH_CURRENT_ONLY)
    for (int I=0;I<ItemCount && !StopSearch;I++)
    {
      PluginPanelItem *CurPanelItem=PanelData+I;
      char *CurName=CurPanelItem->FindData.cFileName;
      if ((CurPanelItem->FindData.dwFileAttributes & FA_DIREC) &&
          strcmp(CurName,".")!=0 && strcmp(CurName,"..")!=0 &&
          (SearchMode!=SEARCH_SELECTED || RecurseLevel!=1 ||
          CtrlObject->Cp()->ActivePanel->IsSelected(CurName)))
        if (strchr(CurName,'\x1')==NULL && CtrlObject->Plugins.SetDirectory(hPlugin,CurName,OPM_FIND))
        {
          strcat(PluginSearchPath,CurName);
          if (strlen(PluginSearchPath)<NM-2)
          {
            strcat(PluginSearchPath,"\x1");
            ScanPluginTree();
            *strrchr(PluginSearchPath,'\x1')=0;
          }
          char *NamePtr=strrchr(PluginSearchPath,'\x1');
          if (NamePtr!=NULL)
            *(NamePtr+1)=0;
          else
            *PluginSearchPath=0;
          if (!CtrlObject->Plugins.SetDirectory(hPlugin,"..",OPM_FIND))
          {
            StopSearch=TRUE;
            break;
          }
        }
    }
  CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,ItemCount);
  RecurseLevel--;
}


void RereadPlugin(HANDLE hPlugin)
{
  int FileCount=0;
  PluginPanelItem *PanelData=NULL;
  if (CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&FileCount,OPM_FIND))
    CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,FileCount);
}
