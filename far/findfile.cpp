/*
findfile.cpp

Поиск (Alt-F7)

*/

/* Revision: 1.54 27.09.2001 $ */

/*
Modify:
  27.09.2001 IS
    - Левый размер при использовании strncpy
  24.09.2001 OT
    Падение при открытии файла (F3,F4) из поиска
  13.09.2001 KM
    - Не просматривались файлы, найденные в архиве, если поиск начинался
      не с корня архива, а из подкаталогов.
  23.08.2001 VVM
    + В диалоге поиска строка с каталогом расширена на 20 символов - все равно пустовали.
    + Вызвать TruncPathStr() для каталога...
  17.08.2001 KM
    ! При редактировании найденного файла из архива функция "Сохранить"
      заменяется на функцию "Сохранить в", вызывая диалог для ввода имени.
    + Добавлена кнопка "View" в диалог поиска при поиске файла изнутри архива.
    - Поправлена реакция на клавиши серый + и - в просмотре файлов, теперь
      в списке просмотра присутствуют только не архивные файлы.
  15.08.2001 KM
    ! Глобальный перетрях поиска. 5-я серия
    + Добавлены возможности при поиске в архивах:
      1) Просматривать файлы найденные в архиве и находящиеся
         в списке по F3 и F4;
      2) Скидывать список архивов, в которых находятся найденные
         файлы во временную панель.
      3) Производить поиск в архивах не панели плагина, к примеру на
         временной панели, если панель создавалась с флагом OPIF_REALNAMES.
  11.08.2001 KM
    ! Вроде бы удалось нормально синхронизировать нити
      и теперь вся информация о ходе поиска выводится
      корректно.
  10.08.2001 KM
    + Изменение размеров диалога поиска при изменении размеров консоли.
  08.08.2001 KM
    ! Глобальный перетрях поиска. 3-я серия
    ! Кажется удалось избавится ещё от одного потенциально (и не только)
      падучего места при закрытии диалога поиска во время оного: просто
      я забыл перед закрытием диалога дождаться окончания работы второй нити.
    ! Переделан (пока без таблицы ANSI) выбор таблиц поиска.
  07.08.2001 IS
    ! Изменились параметры у FarCharTable
  07.08.2001 SVS
    ! удален FindSaveScr - нафиг, к теропевту. И без него жизнь полна прекрас.
    ! если идет режим поиска (все еще ищем), то отключим вывод помощи,
      ибо... артефакты с прорисовкой. Вот когда закончится поиск, тогде хелп
      будет доступен.
    ! запрещаем во время поиска юзать некоторые манагерные клавиши кроме
      KEY_CTRLALTSHIFTPRESS и KEY_ALTF9
      (2KM: про Alt-F9 - здесь нужно следить DN_RESIZECONSOLE)
    ! во время вызова редактора/вьювера разрешаем юзать клавиши:
      Alt-F9 и F11 :-)
    - ну и напоследок... используя "подвал" :-) корректно восстановим
      экран после F3/F4:
  01.08.2001 KM
    ! Глобальный перетрях поиска. 2-я серия
    ! С подачи OT синхронизацию процессов
      перевёл на Mutex.
    - Кажется удалось избавиться от иксепшенов.
      Артефакты:
        1. Если после F3 или F4 во время поиска
           нажать CAS, то под спрятавшимся
           вьювером/редактором будет видна тень
           от диалога.
        2. После F3 и F4 не восстанавливается
           изображение под диалогом.
        3. Не всегда рисуется количество и место
           поиска.
  31.07.2001 KM
    ! Глобальный перетрях поиска. 1-я серия
      Артефакты:
        1. После F3 и F4 не восстанавливается
           изображение под диалогом.
        2. При выходе из диалога поиска иногда
           выскакивает farexcpt.0xc0000005.
        3. Не всегда корректно синхронизируются нити,
           из-за чего не рисуется количество и место
           поиска.
  27.07.2001 SVS
    ! Опечатка (по наводке IS)
  24.07.2001 IS
    ! Замена проверки на ' ' и '\t' на вызов isspace
    ! Замена проверки на '\n' и '\r' на вызов iseol
  02.07.2001 IS
    ! FileMaskForFindFile.Set(NULL) -> FileMaskForFindFile.Free()
    + Вернул автоматическое добавление '*' к концу маски при определенных
      условиях (это было отменено предыдущим патчем).
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

#define DLG_HEIGHT 21
#define MAX_READ 0x20000

static char FindMask[NM],FindStr[SEARCHSTRINGBUFSIZE];
/* $ 30.07.2000 KM
   Добавлена переменная WholeWords для поиска по точному совпадению
*/
static int SearchMode,CmpCase,WholeWords,UseAllTables,SearchInArchives;
/* KM $ */
static int DlgWidth,DlgHeight;
static volatile int StopSearch,SearchDone,LastFoundNumber,FileCount,WriteDataUsed;
static char FindMessage[200],LastDirName[NM];
static int FindMessageReady,FindCountReady;
static char PluginSearchPath[2*NM];
static char FindFileArcName[NM];
static HANDLE hPlugin;
static HANDLE hDlg;
static struct OpenPluginInfo Info;
static int RecurseLevel;
static int BreakMainThread;
static int ContinueSearch;
static int PluginMode;

static HANDLE hMutex;

/* $ 07.08.2000 KM
   Добавление переменной для борьбы с глюком при поиске в запароленном архиве
*/
static int IsPluginGetsFile;
/* KM $ */

static int UseDecodeTable=FALSE,UseUnicode=FALSE,TableNum=0;
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


long WINAPI FindFiles::MainDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  Dialog* Dlg=(Dialog*)hDlg;
  char *FindText=MSG(MFindFileText),*FindCode=MSG(MFindFileCodePage);
  char DataStr[NM];

  switch(Msg)
  {
    case DN_INITDIALOG:
    {
      unsigned int W=Dlg->Item[6].X1-Dlg->Item[4].X1-5;
      if (strlen(FindText)>W)
      {
        strncpy(DataStr,FindText,W-3);
        DataStr[W-4]=0;
        strcat(DataStr,"...");
      }
      else
        strcpy(DataStr,FindText);
      Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,4,(long)DataStr);

      W=Dlg->Item[0].X2-Dlg->Item[6].X1-3;
      if (strlen(FindCode)>W)
      {
        strncpy(DataStr,FindCode,W-3);
        DataStr[W-4]=0;
        strcat(DataStr,"...");
      }
      else
        strcpy(DataStr,FindCode);
      Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,6,(long)DataStr);

      if (UseAllTables)
        strcpy(TableSet.TableName,MSG(MFindFileAllTables));
      else if (UseUnicode)
        strcpy(TableSet.TableName,"Unicode");
      else if (!UseDecodeTable)
        strcpy(TableSet.TableName,MSG(MGetTableNormalText));
      else
        PrepareTable(&TableSet,TableNum);
      Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,7,(long)TableSet.TableName);

      return TRUE;
    }
    case DN_LISTCHANGE:
    {
      if (Param1==7)
      {
        UseAllTables=(Param2==0);
        UseUnicode=(Param2==3);
        UseDecodeTable=(Param2>=5);
        if (!UseAllTables)
        {
          strcpy(TableSet.TableName,MSG(MGetTableNormalText));
          if (Param2>=5)
          {
            PrepareTable(&TableSet,Param2-5);
            TableNum=Param2-5;
          }
        }
      }
      return TRUE;
    }
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}


FindFiles::FindFiles()
{
  static char LastFindMask[NM]="*.*",LastFindStr[SEARCHSTRINGBUFSIZE];
  /* $ 30.07.2000 KM
     Добавлена переменная LastWholeWords для поиска по точному совпадению
  */
  static int LastCmpCase=0,LastWholeWords=0,LastUseAllTables=0,LastSearchInArchives=0;
  /* KM $ */
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
  FarList TableList;
  FarListItem *TableItem=(FarListItem *)malloc(sizeof(FarListItem)*4);
  TableList.Items=TableItem;
  TableList.ItemsNumber=4;

  memset(TableItem,0,sizeof(FarListItem)*4);
  strcpy(TableItem[0].Text,MSG(MFindFileAllTables));
  TableItem[1].Flags=LIF_SEPARATOR;
  strcpy(TableItem[2].Text,MSG(MGetTableNormalText));
  strcpy(TableItem[3].Text,"Unicode");

  for (int I=0;;I++)
  {
    CharTableSet cts;
    int RetVal=FarCharTable(I,(char *)&cts,sizeof(cts));
    if (RetVal==-1)
      break;

    if (I==0)
    {
      TableItem=(FarListItem *)realloc(TableItem,sizeof(FarListItem)*5);
      if (TableItem==NULL)
        return;
      TableItem[4].Text[0]=0;
      TableItem[4].Flags=LIF_SEPARATOR;
      TableList.Items=TableItem;
      TableList.ItemsNumber++;
    }

    TableItem=(FarListItem *)realloc(TableItem,sizeof(FarListItem)*(I+6));
    if (TableItem==NULL)
      return;
    strcpy(TableItem[I+5].Text,cts.TableName);
    TableItem[I+5].Flags=0;
    TableList.Items=TableItem;
    TableList.ItemsNumber++;
  }

  do
  {
    const char *MasksHistoryName="Masks",*TextHistoryName="SearchText";
    static char H2Separator[72]={0};
    /* $ 30.07.2000 KM
       Добавлен новый checkbox "Whole words" в диалог поиска
    */
    static struct DialogData FindAskDlgData[]=
    {
      /* 00 */DI_DOUBLEBOX,3,1,72,19,0,0,0,0,(char *)MFindFileTitle,
      /* 01 */DI_TEXT,5,2,0,0,0,0,0,0,(char *)MFindFileMasks,
      /* 02 */DI_EDIT,5,3,70,16,1,(DWORD)MasksHistoryName,DIF_HISTORY|DIF_USELASTHISTORY,0,"",
      /* 03 */DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR,0,"",
      /* 04 */DI_TEXT,5,5,0,0,0,0,0,0,"",
      /* 05 */DI_EDIT,5,6,36,16,0,(DWORD)TextHistoryName,DIF_HISTORY,0,"",
      /* 06 */DI_TEXT,40,5,0,0,0,0,0,0,"",
      /* 07 */DI_COMBOBOX,40,6,70,10,0,(DWORD)&TableList,DIF_DROPDOWNLIST,0,"",
      /* 08 */DI_TEXT,3,7,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
      /* 09 */DI_VTEXT,38,4,0,0,0,0,DIF_BOXCOLOR,0,"СііБ",
      /* 10 */DI_CHECKBOX,5,8,0,0,0,0,0,0,(char *)MFindFileCase,
      /* 11 */DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MFindFileWholeWords,
      /* 12 */DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MFindArchives,
      /* 13 */DI_TEXT,3,11,0,0,0,0,DIF_BOXCOLOR,0,"",
      /* 14 */DI_RADIOBUTTON,5,12,0,0,0,0,DIF_GROUP,0,(char *)MSearchAllDisks,
      /* 15 */DI_RADIOBUTTON,5,13,0,0,0,1,0,0,(char *)MSearchFromRoot,
      /* 16 */DI_RADIOBUTTON,5,14,0,0,0,0,0,0,(char *)MSearchFromCurrent,
      /* 17 */DI_RADIOBUTTON,5,15,0,0,0,0,0,0,(char *)MSearchInCurrent,
      /* 18 */DI_RADIOBUTTON,5,16,0,0,0,0,0,0,(char *)MSearchInSelected,
      /* 19 */DI_TEXT,3,17,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
      /* 20 */DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,1,(char *)MFindFileFind,
      /* 21 */DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
    };
    /* KM $ */
    MakeDialogItems(FindAskDlgData,FindAskDlg);

    /* $ 27.02.2001 SVS
       Динамически сформируем разделительную линию */
    if(!H2Separator[0])
      MakeSeparator(70,H2Separator,3);

    strcpy(FindAskDlg[3].Data,H2Separator);
    strcpy(FindAskDlg[13].Data,H2Separator);
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
          FindAskDlg[12].Flags |= DIF_DISABLE;
        /* DJ $ */
      }
    }

    strcpy(FindAskDlg[2].Data,FindMask);
    strcpy(FindAskDlg[5].Data,FindStr);
    FindAskDlg[10].Selected=CmpCase;
    FindAskDlg[11].Selected=WholeWords;

    /* $ 14.05.2001 DJ
       не селектим чекбокс, если нельзя искать в архивах
    */
    if (!(FindAskDlg[12].Flags & DIF_DISABLE))
      FindAskDlg[12].Selected=SearchInArchives;
    /* DJ $ */
    FindAskDlg[14].Selected=FindAskDlg[15].Selected=0;
    FindAskDlg[16].Selected=FindAskDlg[17].Selected=0;
    FindAskDlg[18].Selected=0;
    FindAskDlg[14+SearchMode].Selected=1;

    while (1)
    {
      Dialog Dlg(FindAskDlg,sizeof(FindAskDlg)/sizeof(FindAskDlg[0]),MainDlgProc);

      Dlg.SetHelp("FindFile");
      Dlg.SetPosition(-1,-1,76,21);
      Dlg.Process();
      int ExitCode=Dlg.GetExitCode();
      if (ExitCode!=20)
      {
        free(TableItem);
        return;
      }
      /* $ 01.07.2001 IS
         Проверим маску на корректность
      */
      if(!*FindAskDlg[2].Data)             // если строка с масками пуста,
         strcpy(FindAskDlg[2].Data, "*"); // то считаем, что маска есть "*"

      if(FileMaskForFindFile.Set(FindAskDlg[2].Data, FMF_ADDASTERISK))
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
    CmpCase=FindAskDlg[10].Selected;
    /* $ 30.07.2000 KM
       Добавлена переменная
    */
    WholeWords=FindAskDlg[11].Selected;
    /* KM $ */
    SearchInArchives=FindAskDlg[12].Selected;
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
    if (FindAskDlg[14].Selected)
      SearchMode=SEARCH_ALL;
    if (FindAskDlg[15].Selected)
      SearchMode=SEARCH_ROOT;
    if (FindAskDlg[16].Selected)
      SearchMode=SEARCH_FROM_CURRENT;
    if (FindAskDlg[17].Selected)
      SearchMode=SEARCH_CURRENT_ONLY;
    if (FindAskDlg[18].Selected)
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
  free(TableItem);
}


FindFiles::~FindFiles()
{
  /* $ 02.07.2001 IS
     Освободим память.
  */
  FileMaskForFindFile.Free();
  /* IS $ */
  ScrBuf.ResetShadow();
}

long WINAPI FindFiles::FindDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  struct ListItemUserData UserDataItem;
  Dialog* Dlg=(Dialog*)hDlg;
  VMenu *ListBox=Dlg->Item[1].ListPtr;

  switch(Msg)
  {
    case DN_HELP: // в режиме поиска отключим вывод помощи, ибо... артефакты с прорисовкой
      return !SearchDone?NULL:Param2;

    case DN_KEY:
    {
      WaitForSingleObject(hMutex,INFINITE);

      while (ListBox->GetCallCount())
        Sleep(10);
      if (IsPluginGetsFile)
      {
        ReleaseMutex(hMutex);
        return TRUE;
      }

      // некторые спец.клавиши всеже отбработаем.
      if(Param2 == KEY_CTRLALTSHIFTPRESS || Param2 == KEY_ALTF9)
      {
        IsProcessAssignMacroKey--;
        FrameManager->ProcessKey(Param2);
        IsProcessAssignMacroKey++;
        ReleaseMutex(hMutex);
        return TRUE;
      }

      if (Param1==9 && (Param2==KEY_RIGHT || Param2==KEY_TAB)) // [ Stop ] button
      {
        while (ListBox->GetCallCount())
          Sleep(10);
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,5/* [ New search ] */,0);
        ReleaseMutex(hMutex);
        return TRUE;
      }
      else if (Param1==5 && (Param2==KEY_LEFT || Param2==KEY_SHIFTTAB)) // [ New search ] button
      {
        while (ListBox->GetCallCount())
          Sleep(10);
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,9/* [ Stop ] */,0);
        ReleaseMutex(hMutex);
        return TRUE;
      }
      else if (Param2==KEY_UP || Param2==KEY_DOWN || Param2==KEY_PGUP ||
               Param2==KEY_PGDN || Param2==KEY_HOME || Param2==KEY_END ||
               Param2==KEY_MSWHEEL_UP || Param2==KEY_MSWHEEL_DOWN)
      {
        if (ListBox && ListBox->GetItemCount())
        {
          while (ListBox->GetCallCount())
            Sleep(10);
          ListBox->ProcessKey(Param2);
          ReleaseMutex(hMutex);
        }
        return TRUE;
      }
      else if (Param2==KEY_F3 || Param2==KEY_F4)
      {
        if (!ListBox)
        {
          ReleaseMutex(hMutex);
          return TRUE;
        }

        if ((ListBox->GetUserData(&UserDataItem,sizeof(UserDataItem)))!=0 &&
             ListBox->GetUserDataSize()==sizeof(UserDataItem))
        {
          int RemoveTemp=FALSE;
          char SearchFileName[NM];
          char TempDir[NM];
          char *FileName=UserDataItem.FileFindData.cFileName;
          if (*UserDataItem.FindFileArcName && FileName[strlen(FileName)-1]!='\\')
          {
            HANDLE hArc=NULL;
            if (!hPlugin)
            {
              char *Buffer=new char[MAX_READ];
              FILE *ProcessFile=fopen(UserDataItem.FindFileArcName,"rb");
              if (ProcessFile==NULL)
              {
                delete[] Buffer;
                ReleaseMutex(hMutex);
                return TRUE;
              }
              int ReadSize=fread(Buffer,1,MAX_READ,ProcessFile);
              fclose(ProcessFile);

              int SavePluginsOutput=DisablePluginsOutput;
              DisablePluginsOutput=TRUE;
              hArc=CtrlObject->Plugins.OpenFilePlugin(UserDataItem.FindFileArcName,(unsigned char *)Buffer,ReadSize);
              DisablePluginsOutput=SavePluginsOutput;

              delete[] Buffer;

              if (hArc==(HANDLE)-2 || hArc==INVALID_HANDLE_VALUE)
              {
                ReleaseMutex(hMutex);
                return TRUE;
              }
            }

            PluginPanelItem FileItem;
            memset(&FileItem,0,sizeof(FileItem));
            FileItem.FindData=UserDataItem.FileFindData;
            sprintf(TempDir,"%s%s",Opt.TempPath,FarTmpXXXXXX);
            mktemp(TempDir);
            IsPluginGetsFile=TRUE;
            if (!CtrlObject->Plugins.GetFile(hPlugin?hPlugin:hArc,&FileItem,TempDir,SearchFileName,OPM_SILENT|OPM_FIND))
            {
              RemoveDirectory(TempDir);
              IsPluginGetsFile=FALSE;
              ReleaseMutex(hMutex);
              return FALSE;
            }
            RemoveTemp=TRUE;
            IsPluginGetsFile=FALSE;
          }
          else
            strcpy(SearchFileName,UserDataItem.FileFindData.cFileName);

          DWORD FileAttr;
          if ((FileAttr=GetFileAttributes(SearchFileName))!=(DWORD)-1 &&
              (FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0)
          {
            char OldTitle[512];
            GetConsoleTitle(OldTitle,sizeof(OldTitle));

            if (Param2==KEY_F3)
            {
              NamesList ViewList;
              if (!PluginMode || (Info.Flags & OPIF_REALNAMES))
              {
                int ListSize=ListBox->GetItemCount();
                struct ListItemUserData ListFindData;
                for (int I=0;I<ListSize;I++)
                  if ((ListBox->GetUserData(&ListFindData,sizeof(ListFindData),I))!=0 &&
                       ListBox->GetUserDataSize(I)==sizeof(ListFindData))
                  {
                    int Length=strlen(ListFindData.FileFindData.cFileName);
                    if (Length>0 && ListFindData.FileFindData.cFileName[Length-1]!='\\' &&
                        !*ListFindData.FindFileArcName)
                      ViewList.AddName(ListFindData.FileFindData.cFileName);
                  }
                ViewList.SetCurName(UserDataItem.FileFindData.cFileName);
              }
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
              ReleaseMutex(hMutex);
              {
                FileViewer ShellViewer (SearchFileName,FALSE,FALSE,FALSE,-1,NULL,(*UserDataItem.FindFileArcName)?NULL:&ViewList);
                ShellViewer.SetDynamicallyBorn(FALSE);
                ShellViewer.SetEnableF6(TRUE);
                if (*UserDataItem.FindFileArcName)
                  ShellViewer.SetSaveToSaveAs(TRUE);
                IsProcessVE_FindFile++;
                FrameManager->ExecuteModal ();
                IsProcessVE_FindFile--;
                // заставляем рефрешится экран
                FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
              }
              WaitForSingleObject(hMutex,INFINITE);

              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
            }
            else
            {
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
              ReleaseMutex(hMutex);
              {
                FileEditor ShellEditor (SearchFileName,FALSE,FALSE);
                ShellEditor.SetDynamicallyBorn(FALSE);
                ShellEditor.SetEnableF6 (TRUE);
                if (*UserDataItem.FindFileArcName)
                  ShellEditor.SetSaveToSaveAs(TRUE);
                IsProcessVE_FindFile++;
                FrameManager->ExecuteModal ();
                IsProcessVE_FindFile--;
                // заставляем рефрешится экран
                FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
              }
              WaitForSingleObject(hMutex,INFINITE);
              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
            }
            SetConsoleTitle(OldTitle);
          }
          if (RemoveTemp)
            DeleteFileWithFolder(SearchFileName);
        }
        ReleaseMutex(hMutex);
        return TRUE;
      }
      ReleaseMutex(hMutex);
      return FALSE;
    }
    case DN_BTNCLICK:
    {
      if (Param1==5) // [ New search ] button pressed
      {
        ContinueSearch=TRUE;
        return FALSE;
      }
      else if (Param1==9) // [ Stop ] button pressed
      {
        if (StopSearch)
          return FALSE;
        StopSearch=TRUE;
        return TRUE;
      }
      else if (Param1==6) // [ Goto ] button pressed
      {
        WaitForSingleObject(hMutex,INFINITE);

        if (ListBox && ListBox->GetItemCount())
        {
          while (ListBox->GetCallCount())
            Sleep(10);
          ListBox->GetUserData(&UserDataItem,sizeof(UserDataItem));

          char *FileName=UserDataItem.FileFindData.cFileName;
          Panel *FindPanel=CtrlObject->Cp()->ActivePanel;

          if (*UserDataItem.FindFileArcName && !hPlugin)
          {
            char ArcName[NM],ArcPath[NM];
            strncpy(ArcName,UserDataItem.FindFileArcName,sizeof(ArcName)-1);
            if (FindPanel->GetType()!=FILE_PANEL)
              FindPanel=CtrlObject->Cp()->ChangePanel(FindPanel,FILE_PANEL,TRUE,TRUE);
            strcpy(ArcPath,ArcName);
            *PointToName(ArcPath)=0;
            FindPanel->SetCurDir(ArcPath,TRUE);
            hPlugin=((FileList *)FindPanel)->OpenFilePlugin(ArcName,FALSE);
            if (hPlugin==INVALID_HANDLE_VALUE || hPlugin==(HANDLE)-2)
            {
              ReleaseMutex(hMutex);
              return FALSE;
            }
          }
          if (hPlugin)
          {
            SetPluginDirectory(FileName);
            ReleaseMutex(hMutex);
            return FALSE;
          }
          char SetName[NM];
          int Length;
          if ((Length=strlen(FileName))==0)
          {
            ReleaseMutex(hMutex);
            return FALSE;
          }
          if (Length>1 && FileName[Length-1]=='\\' && FileName[Length-2]!=':')
            FileName[Length-1]=0;
          if (GetFileAttributes(FileName)==(DWORD)-1)
          {
            ReleaseMutex(hMutex);
            return FALSE;
          }

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
          {
            ReleaseMutex(hMutex);
            return FALSE;
          }
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
        ReleaseMutex(hMutex);
        return FALSE;
      }
      else if (Param1==7) // [ View ] button pressed
      {
        FindDlgProc(hDlg,DN_KEY,1,KEY_F3);
        return TRUE;
      }
      else if (Param1==8) // [ Panel ] button pressed
      {
        WaitForSingleObject(hMutex,INFINITE);

        if (ListBox && ListBox->GetItemCount() &&
            hPlugin==NULL || (Info.Flags & OPIF_REALNAMES))
        {
          while (ListBox->GetCallCount())
            Sleep(10);
          int ListSize=ListBox->GetItemCount();

          PluginPanelItem *PanelItems=new PluginPanelItem[ListSize];
          if (PanelItems==NULL)
            ListSize=0;
          int ItemsNumber=0;
          for (int i=0;i<ListSize;i++)
          {
            if ((ListBox->GetUserData(&UserDataItem,sizeof(UserDataItem),i))!=0 &&
                ListBox->GetUserDataSize(i)==sizeof(UserDataItem))
            {
              int Length=strlen(UserDataItem.FileFindData.cFileName);
              char *FileName=UserDataItem.FileFindData.cFileName;
              if (Length>0 && (FileName[Length-1]!='\\' &&
                  !*UserDataItem.FindFileArcName) ||
                 (FileName[Length-1]=='\\' &&
                    *UserDataItem.FindFileArcName))
              {
                if (*UserDataItem.FindFileArcName)
                {
                  char *p=strrchr(FileName,':');
                  if (p && p-FileName>1)
                    strcpy(FileName,UserDataItem.FindFileArcName);
                }
                PluginPanelItem *pi=&PanelItems[ItemsNumber++];
                memset(pi,0,sizeof(*pi));
                pi->FindData=UserDataItem.FileFindData;
              }
            }
          }

          HANDLE hNewPlugin=CtrlObject->Plugins.OpenFindListPlugin(PanelItems,ItemsNumber);
          if (hNewPlugin!=INVALID_HANDLE_VALUE)
          {
            Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
            Panel *NewPanel=CtrlObject->Cp()->ChangePanel(ActivePanel,FILE_PANEL,TRUE,TRUE);
            NewPanel->SetPluginMode(hNewPlugin,"");
            NewPanel->Update(0);
            if (ListBox && ListBox->GetItemCount() &&
                ListBox->GetUserData(&UserDataItem,sizeof(UserDataItem)))
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
          ReleaseMutex(hMutex);
          return FALSE;
        }
        ReleaseMutex(hMutex);
        return TRUE;
      }
    }
    case DN_CTLCOLORDLGLIST:
    {
      short ColorArray[10]=
      {
        COL_DIALOGMENUTEXT,
        COL_DIALOGMENUTEXT,
        COL_MENUTITLE,
        COL_DIALOGMENUTEXT,
        COL_DIALOGMENUHIGHLIGHT,
        COL_DIALOGMENUTEXT,
        COL_DIALOGMENUSELECTEDTEXT,
        COL_DIALOGMENUSELECTEDHIGHLIGHT,
        COL_DIALOGMENUSCROLLBAR,
        COL_DIALOGMENUTEXT
      };
      if (Param2)
        for (int I=0;I<Param1;I++)
          ((short *)Param2)[I]=ColorArray[I];
      return TRUE;
    }
    case DN_CLOSE:
    {
      StopSearch=TRUE;
      while (!SearchDone || WriteDataUsed)
        Sleep(10);
      return TRUE;
    }
    /* 10.08.2001 KM
       Изменение размеров диалога поиска при изменении размеров консоли.
    */
    case DN_RESIZECONSOLE:
    {
      WaitForSingleObject(hMutex,INFINITE);

      COORD coord=(*(COORD*)Param2);
      SMALL_RECT rect;
      int IncY=coord.Y-DlgHeight-4;
      int I;

      Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
      for (I=0;I<10;I++)
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,I,FALSE);

      Dialog::SendDlgMessage(hDlg,DM_GETDLGRECT,0,(long)&rect);
      coord.X=rect.Right-rect.Left+1;
      DlgHeight+=IncY;
      coord.Y=DlgHeight;

      if (IncY>0)
        Dialog::SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(long)&coord);

      for (I=0;I<2;I++)
      {
        Dialog::SendDlgMessage(hDlg,DM_GETITEMPOSITION,I,(long)&rect);
        rect.Bottom+=(short)IncY;
        Dialog::SendDlgMessage(hDlg,DM_SETITEMPOSITION,I,(long)&rect);
      }

      for (I=2;I<10;I++)
      {
        Dialog::SendDlgMessage(hDlg,DM_GETITEMPOSITION,I,(long)&rect);
        if (I==2)
          rect.Left=-1;
        rect.Top+=(short)IncY;
        Dialog::SendDlgMessage(hDlg,DM_SETITEMPOSITION,I,(long)&rect);
      }

      if (!(IncY>0))
        Dialog::SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(long)&coord);

      for (I=0;I<10;I++)
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,I,TRUE);
      Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);

      ReleaseMutex(hMutex);
      return TRUE;
    }
    /* KM $ */
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

int FindFiles::FindFilesProcess()
{
  ContinueSearch=FALSE;

  hMutex=CreateMutex(NULL,FALSE,NULL);

  static struct DialogData FindDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,72,DLG_HEIGHT-2,0,0,0,0,(char *)MFindFileTitle,
  /* 01 */DI_LISTBOX,4,2,71,14,0,0,DIF_LISTNOBOX,0,(char*)0,
  /* 02 */DI_TEXT,-1,15,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 03 */DI_TEXT,5,16,0,0,0,0,0,0,(char *)MFindSearchingIn,
  /* 04 */DI_TEXT,3,17,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 05 */DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MFindNewSearch,
  /* 06 */DI_BUTTON,0,18,0,0,1,0,DIF_CENTERGROUP,1,(char *)MFindGoTo,
  /* 07 */DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MFindView,
  /* 08 */DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MFindPanel,
  /* 09 */DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MFindStop
  };
  MakeDialogItems(FindDlgData,FindDlg);

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  PluginMode=ActivePanel->GetMode()==PLUGIN_PANEL && ActivePanel->IsVisible();

  DlgHeight=DLG_HEIGHT;

  int IncY=ScrY>24 ? ScrY-24:0;
  FindDlg[0].Y2+=IncY;
  FindDlg[1].Y2+=IncY;
  FindDlg[2].Y1+=IncY;
  FindDlg[3].Y1+=IncY;
  FindDlg[4].Y1+=IncY;
  FindDlg[5].Y1+=IncY;
  FindDlg[6].Y1+=IncY;
  FindDlg[7].Y1+=IncY;
  FindDlg[8].Y1+=IncY;
  FindDlg[9].Y1+=IncY;

  DlgHeight+=IncY;

  *FindFileArcName=0;
  if (PluginMode)
  {
    hPlugin=ActivePanel->GetPluginHandle();
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
    if ((Info.Flags & OPIF_REALNAMES)==0)
    {
      strcpy(FindFileArcName,Info.HostFile);
      FindDlg[8].Type=DI_TEXT;
      *FindDlg[8].Data=0;
    }
  }
  else
  {
    hPlugin=NULL;
    memset(&Info,0,sizeof(Info));
  }

  DlgWidth=FindDlg[0].X2-FindDlg[0].X1-4;
  Dialog *pDlg=new Dialog(FindDlg,sizeof(FindDlg)/sizeof(FindDlg[0]),FindDlgProc);
  hDlg=(HANDLE)pDlg;
  pDlg->SetDynamicallyBorn(TRUE);
  pDlg->SetHelp("FindFile");
  pDlg->SetPosition(-1,-1,76,DLG_HEIGHT+IncY);

  LastFoundNumber=0;
  SearchDone=FALSE;
  StopSearch=FALSE;
  WriteDataUsed=FALSE;
  FileCount=0;
  *FindMessage=*LastDirName=FindMessageReady=FindCountReady=0;

  // Нитка для вывода в диалоге информации о ходе поиска
  if (_beginthread(WriteDialogData,0,NULL)==(unsigned long)-1)
    return FALSE;

  if (PluginMode)
  {
    if (_beginthread(PreparePluginList,0,NULL)==(unsigned long)-1)
      return FALSE;
  }
  else
  {
    if (_beginthread(PrepareFilesList,0,NULL)==(unsigned long)-1)
      return FALSE;
  }

  IsProcessAssignMacroKey++; // отключим все спец. клавиши
  pDlg->Process();
  IsProcessAssignMacroKey--;

  CloseHandle(hMutex);

  if (ContinueSearch)
    return TRUE;

  return FALSE;
}


void FindFiles::SetPluginDirectory(char *FileName)
{
  char Name[NM],*StartName,*EndName;
  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  if (SearchMode==SEARCH_ROOT || SearchMode==SEARCH_ALL)
    CtrlObject->Plugins.SetDirectory(hPlugin,"\\",OPM_FIND);
  strcpy(Name,FileName);
  StartName=Name;
  while ((EndName=strchr(StartName,'\\'))!=NULL)
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
void _cdecl FindFiles::PrepareFilesList(void *Param)
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
        if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0 || strcmp(SelName,"..")==0 ||
            strcmp(SelName,".")==0)
          continue;
        strcpy(CurRoot,Root);
        AddEndSlash(CurRoot);
        strcat(CurRoot,SelName);
      }
      else
        strcpy(CurRoot,Root);

      ScTree.SetFindPath(CurRoot,"*.*");

      strncpy(FindMessage,CurRoot,sizeof(FindMessage)-1);
      FindMessage[sizeof(FindMessage)-1]=0;
      FindMessageReady=TRUE;

      while (!StopSearch && ScTree.GetNextName(&FindData,FullName))
      {
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
          strncpy(FindMessage,FullName,sizeof(FindMessage)-1);
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
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif


void FindFiles::ArchiveSearch(char *ArcName)
{
  char *Buffer=new char[MAX_READ];
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
  int ReadSize=fread(Buffer,1,MAX_READ,ProcessFile);
  fclose(ProcessFile);

  int SavePluginsOutput=DisablePluginsOutput;
  DisablePluginsOutput=TRUE;
  HANDLE hArc=CtrlObject->Plugins.OpenFilePlugin(ArcName,(unsigned char *)Buffer,ReadSize);
  DisablePluginsOutput=DisablePluginsOutput;

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
  OpenPluginInfo SaveInfo=Info;
  SearchMode=SEARCH_FROM_CURRENT;
  hPlugin=hArc;
  CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
  strcpy(FindFileArcName,ArcName);
  *LastDirName=0;
  PreparePluginList((void *)1);
  *FindFileArcName=0;
  SearchMode=SaveSearchMode;
  Info=SaveInfo;
  CtrlObject->Plugins.ClosePlugin(hPlugin);
  hPlugin=SaveHandle;
}

/* $ 01.07.2001 IS
   Используем FileMaskForFindFile вместо GetCommaWord
*/
int FindFiles::IsFileIncluded(PluginPanelItem *FileItem,char *FullName,DWORD FileAttr)
{
  int FileFound=FileMaskForFindFile.Compare(FullName);
  while(FileFound)
  {
    if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY) &&
        (hPlugin==NULL || (Info.Flags & OPIF_FINDFOLDERS)==0))
      return FALSE;

    if (*FindStr && FileFound)
    {
      FileFound=FALSE;
      if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
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


void FindFiles::AddMenuRecord(char *FullName,char *Path,WIN32_FIND_DATA *FindData)
{
  struct ListItemUserData UserDataItem;
  char MenuText[NM],FileText[NM],SizeText[30];
  char Date[30],DateStr[30],TimeStr[30];
  struct MenuItem ListItem;
  int i;

  Dialog* Dlg=(Dialog*)hDlg;
  VMenu *ListBox=Dlg->Item[1].ListPtr;
  if (!ListBox)
    return;

  WaitForSingleObject(hMutex,INFINITE);

  memset(&ListItem,0,sizeof(ListItem));

  sprintf(SizeText,"%10u",FindData->nFileSizeLow);
  char *DisplayName=FindData->cFileName;
  if (hPlugin && (Info.Flags & OPIF_REALNAMES))
    DisplayName=PointToName(DisplayName);

  sprintf(FileText," %-30.30s %10.10s",DisplayName,SizeText);
  ConvertDate(&FindData->ftLastWriteTime,DateStr,TimeStr,5);
  sprintf(Date,"    %s   %s",DateStr,TimeStr);
  strcat(FileText,Date);
  sprintf(MenuText," %-*.*s",DlgWidth-3,DlgWidth-3,FileText);

  for (i=0;FullName[i]!=0;i++)
  {
    if (FullName[i]=='\x1')
      FullName[i]='\\';
  }

  char PathName[2*NM];
  if (Path)
  {
    for (i=0;Path[i]!=0;i++)
    {
      if (Path[i]=='\x1')
        Path[i]='\\';
    }
    strcpy(PathName,Path);
  }
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
    if (*LastDirName)
    {
      /* $ 12.05.2001 DJ
         курсор не останавливается на пустых строках между каталогами
      */
      ListItem.Flags|=LIF_DISABLE;
      while (ListBox->GetCallCount())
        Sleep(10);
      ListBox->AddItem(&ListItem);
      ListItem.Flags&=~LIF_DISABLE;
      /* DJ $ */
    }
    strcpy(LastDirName,PathName);
    if (*FindFileArcName)
    {
      char ArcPathName[NM*2],DirName[NM];
      sprintf(ArcPathName,"%s:%s",FindFileArcName,*PathName=='.' ? "\\":PathName);
      strcpy(PathName,ArcPathName);
      ScanTree Tree(FALSE,SearchMode!=SEARCH_CURRENT_ONLY);
      strcpy(DirName,FindFileArcName);
      *PointToName(DirName)=0;
      Tree.SetFindPath(DirName,PointToName(FindFileArcName));
      Tree.GetNextName(FindData,FindFileArcName);
    }
    strcpy(SizeText,MSG(MFindFileFolder));
    sprintf(FileText,"%-50.50s     <%6.6s>",TruncPathStr(PathName,50),SizeText);
    sprintf(ListItem.Name,"%-*.*s",DlgWidth-2,DlgWidth-2,FileText);

    memset(&UserDataItem,0,sizeof(UserDataItem));
    UserDataItem.FileFindData=*FindData;
    strcpy(UserDataItem.FileFindData.cFileName,PathName);
    if (*FindFileArcName)
      strcpy(UserDataItem.FindFileArcName,FindFileArcName);

    while (ListBox->GetCallCount())
      Sleep(10);
    ListBox->SetUserData(&UserDataItem,sizeof(UserDataItem),
            ListBox->AddItem(&ListItem));
  }

  memset(&UserDataItem,0,sizeof(UserDataItem));

  UserDataItem.FileFindData=*FindData;
  strncpy(UserDataItem.FileFindData.cFileName,
          FullName,
          sizeof(UserDataItem.FileFindData.cFileName)-1);
  if (*FindFileArcName)
    strcpy(UserDataItem.FindFileArcName,FindFileArcName);
  strcpy(ListItem.Name,MenuText);
  ListItem.SetSelect(!FileCount);

  while (ListBox->GetCallCount())
    Sleep(10);
  ListBox->SetUserData(&UserDataItem,sizeof(UserDataItem),
            ListBox->AddItem(&ListItem));

  LastFoundNumber++;
  FileCount++;
  FindCountReady=TRUE;
  ReleaseMutex(hMutex);
}


int FindFiles::LookForString(char *Name)
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
            if (isspace(Buf[I]) || iseol(Buf[I]))
              locResultLeft=TRUE;
            if (RealReadSize!=sizeof(Buf) && I+1+Length>=RealReadSize)
              locResultRight=TRUE;
            else
              if (isspace(Buf[I+1+Length]) || iseol(Buf[I+1+Length]))
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
              if (isspace(Buf[I+Length]) || iseol(Buf[I+Length]))
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
void _cdecl FindFiles::PreparePluginList(void *Param)
{
  char SaveDir[NM];

  Sleep(100);
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
  }
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif

void FindFiles::ScanPluginTree()
{
  PluginPanelItem *PanelData=NULL;
  int ItemCount=0;

  if (StopSearch || !CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&ItemCount,OPM_FIND))
    return;
  RecurseLevel++;
  if (SearchMode!=SEARCH_SELECTED || RecurseLevel!=1)
  {
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

      if (CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        strncpy(FindMessage,FullName,sizeof(FindMessage)-1);
        FindMessage[sizeof(FindMessage)-1]=0;
        for (int I=0;FindMessage[I]!=0;I++)
          if (FindMessage[I]=='\x1')
            FindMessage[I]='\\';
        FindMessageReady=TRUE;
      }

      if (IsFileIncluded(CurPanelItem,CurName,CurPanelItem->FindData.dwFileAttributes))
        AddMenuRecord(FullName,AddPath,&CurPanelItem->FindData);

      if (SearchInArchives && PluginMode && (Info.Flags & OPIF_REALNAMES))
        ArchiveSearch(FullName);
    }
  }
  if (SearchMode!=SEARCH_CURRENT_ONLY)
  {
    for (int I=0;I<ItemCount && !StopSearch;I++)
    {
      PluginPanelItem *CurPanelItem=PanelData+I;
      char *CurName=CurPanelItem->FindData.cFileName;
      if ((CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
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
  }
  CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,ItemCount);
  RecurseLevel--;
}


void FindFiles::RereadPlugin(HANDLE hPlugin)
{
  int FileCount=0;
  PluginPanelItem *PanelData=NULL;
  if (CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&FileCount,OPM_FIND))
    CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,FileCount);
}

void FindFiles::WriteDialogData(void *Param)
{
  FarDialogItemData ItemData;
  char DataStr[NM];
  Dialog* Dlg=(Dialog*)hDlg;

  WriteDataUsed=TRUE;
  while(1)
  {
    VMenu *ListBox=Dlg->Item[1].ListPtr;
    if (ListBox)
    {
      WaitForSingleObject(hMutex,INFINITE);

      if (BreakMainThread)
        StopSearch=TRUE;

      if (FindCountReady)
      {
        sprintf(DataStr," %s: %d ",MSG(MFindFound),FileCount);
        ItemData.PtrData=DataStr;
        ItemData.PtrLength=strlen(DataStr);

        while (ListBox->GetCallCount())
          Sleep(10);
        Dialog::SendDlgMessage(hDlg,DM_SETTEXT,2,(long)&ItemData);
        FindCountReady=FALSE;
      }
      if (FindMessageReady)
      {
        char *SearchStr=MSG(MFindSearchingIn);
        int Wid1=strlen(SearchStr);
        int Wid2=DlgWidth-strlen(SearchStr)-1;

        if (SearchDone)
        {
          while (ListBox->GetCallCount())
            Sleep(10);
          Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

          strcpy(DataStr,MSG(MFindCancel));
          ItemData.PtrData=DataStr;
          ItemData.PtrLength=strlen(DataStr);
          Dialog::SendDlgMessage(hDlg,DM_SETTEXT,9,(long)&ItemData);

          sprintf(DataStr,"%-*.*s",DlgWidth,DlgWidth,FindMessage);
          ItemData.PtrData=DataStr;
          ItemData.PtrLength=strlen(DataStr);
          Dialog::SendDlgMessage(hDlg,DM_SETTEXT,3,(long)&ItemData);

          ItemData.PtrData="";
          ItemData.PtrLength=strlen(DataStr);
          Dialog::SendDlgMessage(hDlg,DM_SETTEXT,2,(long)&ItemData);

          Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
          SetFarTitle(FindMessage);
          StopSearch=TRUE;
        }
        else
        {
          sprintf(DataStr,"%-*.*s %-*.*s",Wid1,Wid1,SearchStr,Wid2,Wid2,TruncPathStr(FindMessage,Wid2));
          ItemData.PtrData=DataStr;
          ItemData.PtrLength=strlen(DataStr);
          while (ListBox->GetCallCount())
            Sleep(10);
          Dialog::SendDlgMessage(hDlg,DM_SETTEXT,3,(long)&ItemData);
        }
        FindMessageReady=FALSE;
      }

      if (LastFoundNumber && ListBox)
      {
        LastFoundNumber=0;
        while (ListBox->GetCallCount())
          Sleep(10);
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,1,1);
      }
      ReleaseMutex(hMutex);
    }

    if (StopSearch && SearchDone && !FindMessageReady && !FindCountReady && !LastFoundNumber)
      break;
    Sleep(20);
  }
  WriteDataUsed=FALSE;
}
