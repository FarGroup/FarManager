/*
findfile.cpp

Поиск (Alt-F7)

*/

/* Revision: 1.97 04.03.2002 $ */

/*
Modify:
  04.03.2002 DJ
    ! нажатие Esc в диалоге подтверждения прерывания поиска поиск _не_
	  прерывает
  01.03.2002 SVS
    ! Есть только одна функция создания временного файла - FarMkTempEx
  21.02.2002 VVM
    ! заменим strcpy на strncpy. А то падает при поиске в слишком
      длинных путях.
    ! Отмена патча про ESC.
  21.02.2002 VVM
    ! При запросе на остановку поиска ESC аналогичен ответу NO
  19.02.2002 VVM
    ! Ошибка поиска в плагине. Забыл скобки поставить...
  11.02.2002 SVS
    ! Проинициализируем новые FarListItem в 0 (поелезно при изменении
      размера структуры FarListItem)
  30.01.2002 VVM
    ! Большой фикс - используем hPluginMutex во время работы с панелью
      плагина. Иначе возможен облом при просмотре найденного файла в
      момент поиска на панели этого-же плагина.
  23.01.2002 VVM
    + GetPluginFile() - получить файл для просмотра с панели плагина.
      В отличие от предыдущего подхода - учитывает вложенность папок
      и делает для них SetDirectory()
  18.01.2002 VVM
    ! При вызове просмотра забыли создать временный каталог.
  17.01.2002 VVM
    ! Выделять элементы в списке будем через ListBox, а не в структуре.
      Убираем возможность двойного выделения
    ! Поскольку работу с поиском в папках вынесли в диалог -
      флаг OPIF_FINDFOLDERS в плагине потерял свою актуальность
  16.01.2002 VVM
    ! В функцию AddMenuRecord не передается параметр Path, он там лишний...
      исправление бага № 236
  15.01.2002 VVM
    ! Исправление поиска в подкаталогах архивов
    + Информация о количестве найденных файлов и каталогов разделена.
  28.12.2001 SVS
    ! Правка с учетом изменений структур (про анонимный union)
  18.12.2001 KM
    - > IS "p.s. заметил на 1127 билде: если искать какую-нибудь (длинную?)
      строку, то при повторном поиске (выбрать "Новый поиск" в диалоге
      поиска) строка поиска будет содержать эту строчку, но усеченную
      по ширине.  Например, я искал "FROM_LEFT_2ND_BUTTON_PRESSED", после
      выбора "Новый поиск" получил "...PRESSED".... Кстати, и ищется
      файлах в обоих случаях не "FROM_LEFT_2ND_BUTTON_PRESSED", а
      "...PRESSED" (где-то ты там оригинальную строку поиска портишь)"
  16.12.2001 KM
    - Наконец-то удавлен баг с торможением клавиш курсора во время
      поиска в архивах. Дело оказалось в моей прошлогодней правке
      при поиске в запароленном архиве - IsPluginGetsFile, но так как
      поиск был переделан на Dialog API, то сей флаг теперь стал мешать.
    + Добавлена информативность в "Searching in:". Если ищем текст в файлах,
      то строка выглядит "Searching "text_to_find" in:".
    + Сделана проверка на существование открытого файла в редакторе при
      нажатии F4 во время поиска, при этом выдаётся запрос как в панелях
      (усечённый запрос).
      Артефакт: при нажатии на F4 во время поиска на файле уже открытом в
      редакторе заголовок консоли продолжает изменяться как-будто открыт
      диалог поиска (устал уже просто работать, может завтра найду :).
  11.12.2001 VVM
    - bugz#162 (* вызвав плагин для поиска в архиве и ничего там не найдя, FAR забыл,
         что поиск продолжается всё в том же каталоге и ещё раз вписал имя
         каталога в список результатов поиска. *)
  11.12.2001 VVM
    - Не рисуем диалог при активном скрин-сэйвере
  03.12.2001 DJ
    - корректный показ имен файлов с амперсандами
  01.12.2001 KM
    - [Bug#145] М-да... Такая невнимательность до добра не доведёт :(
      Перепутать Param1 с Param2 это надо суметь...
    + Добавлена в заголовок окна поиска информация о маске поиска:
      "=== Find files: mask*.ext ==="
      Сам много раз на длинных поисках забывал что ищу...
  23.11.2001 KM
    - БЛИН! Правил глюк и глюк добавил :-(
      Перестали работать чекбоксы и радиобатоны.
  23.11.2001 VVM
    ! Немного изменений, что-бы отрисовка диалога не перекрывала сообщение.
  22.11.2001 KM
    - Забыли однако, что DN_BTNCLICK работает со всеми "нажимающимися"
      контролами, только с кнопками немного по-другому, поэтому простое
      нажатие ENTER на кнопках Find или Cancel ни к чему не приводило.
    + Во время поиска нажали Esc - выдаётся стандартный запрос на останов
      операции (если опция включена) или просто поиск останавливается
      (раньше он ещё и выходил из него).
  22.11.2001 VVM
    ! Сбрасыватьсостояние FindFolders при вводе текста.
      Но только если не меняли этот состояние вручную
      Переделка предыдущего патча.
  15.11.2001 IS
    - ищем каталоги только, когда поле "текст" пустое, иначе ерунда получается
    + диалог поиска: не дадим включить поиск папок, если ищем текст
  13.11.2001 VVM
    ! Хм. Добавим папки на панель, если их искали...
  26.10.2001 KM
    + Добавлена опция "Искать каталоги", позволяющая искать не только файлы, но и каталоги.
    - Была попытка просматривать каталоги из архивов по F3.
  20.10.2001 KM
    ! Подчистки по тексту.
    ! Зачем-то при выбросе файлов во временную панель был сделан переход на файл,
      на котором стоял курсор в списке. Убрано.
  19.10.2001 KM
    - Маленький глючок в моём прошлом патче, перепутал параметры местами у memset.
    ! Использование DIF_SEPARATOR2 вмечто MakeSeparator.
    ! Обработка кнопки [ New search ] сделана через enum VVM для однообразности.
  19.10.2001 VVM
    + Вынесли GoTo за пределы диалоговой процедуры.Причины все те же, что и у [ Panel ]
    ! FindFileArcIndex нельзя использовать при обработке выделенных элементов списка.
      Он может быть и другой.
  19.10.2001 VVM
    ! Попытка номер 2. С учетом замечаний и пожеланий от KM и SVS ;)))
    ! Перетрях поиска, зачистка местности, удавливание багов. Далее по тексту...
  19.10.2001 KM
    ! С подачи VVM сделано добавление в список индекса LIST_INDEX_NONE на пустых строках.
    - Поправлен поиск во временной панели, он там не работал и выдавал "странные результаты",
      не позволял просматривать и редактировать файлы, хотя они и OPIF_REALNAMES.
    + Добавлена SortItems для сортировки PluginPanelItem для поиска во временной панели.
  18.10.2001 KM
    - Фар падал при попытке нажатия на кнопку "View", когда ничего не было найдено.
    ! Некоторые уточнения поведения кнопок и мыши при пустом списке.
  15.10.2001 VVM
    ! Отмена предыдущего патча про хост-файл. Факт отсутсвия хост-файла должен
      учитываться по-другому.
  15.10.2001 KM
    - Фар валился при попытке поиска в плагинах типа Temporary panel
      и Network browser. Не учитывался факт отсутствия хостфайла в этих
      плагинах.
    - Не происходил переход на файл при клике на нём мышой в списке.
    - Забыл обработать KEY_NUMPAD5 в списке найденных файлов.
  13.10.2001 VVM
    ! Баг при поиске в темп-панели.
    + Новая функция - очистить списки. Что-бы небыло перерасхода памяти
      после нажатия на [New search]
  12.10.2001 VVM
    ! Устраняем падение при поиске строки. Забыл, что
      INVALID_HANDLE_VALEU <> 0 ;)
  12.10.2001 VVM
    ! Очередной перетрях поиска. Избавляемся от глобальных переменных,
      которые могут конфликтовать. Список архивов теперь хранит хэндл
      архива и флаги для этого архива.
  09.10.2001 VVM
    ! Переделка поиска - ускорение неимеверное :))))
  07.10.2001 SVS
    ! Нафига юзать пвсевдографигу напрямую? ведь договаривались же -
      только коды!
  07.10.2001 SVS
    ! Небольшое ограничение области действия Dialog
  01.10.2001 VVM
    ! Проверять нада переменные на NULL
  01.10.2001 VVM
    ! После Alt-F7 с поиском в архивах не работали меню и диалоги плагинов
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

#define LIST_DELTA  64
static DWORD LIST_INDEX_NONE = (DWORD)-1;

// Список найденных файлов. Индекс из списка хранится в меню.
static LPFINDLIST  FindList;
static DWORD       FindListCapacity;
static DWORD       FindListCount;
// Список архивов. Если файл найден в архиве, то FindList->ArcIndex указывает сюда.
static LPARCLIST   ArcList;
static DWORD       ArcListCapacity;
static DWORD       ArcListCount;

static DWORD FindFileArcIndex;
// Используются для отправки файлов на временную панель.
// индекс текущего элемента в списке и флаг для отправки.
static DWORD FindExitIndex;
enum {FIND_EXIT_NONE, FIND_EXIT_SEARCHAGAIN, FIND_EXIT_GOTO, FIND_EXIT_PANEL};
static int FindExitCode;
//static char FindFileArcName[NM];

static char FindMask[NM],FindStr[SEARCHSTRINGBUFSIZE];
/* $ 30.07.2000 KM
   Добавлена переменная WholeWords для поиска по точному совпадению
*/
static int SearchMode,CmpCase,WholeWords,UseAllTables,SearchInArchives;
/* KM $ */
static int FindFoldersChanged;
static int DlgWidth,DlgHeight;
static volatile int StopSearch,PauseSearch,SearchDone,LastFoundNumber,FindFileCount,FindDirCount,WriteDataUsed;
static char FindMessage[200],LastDirName[2*NM];
static int FindMessageReady,FindCountReady;
static char PluginSearchPath[2*NM];
static HANDLE hDlg;
static int RecurseLevel;
static int BreakMainThread;
static int PluginMode;

static HANDLE hDialogMutex, hPluginMutex;

static int UseDecodeTable=FALSE,UseUnicode=FALSE,TableNum=0;
static struct CharTableSet TableSet;

/* $ 01.07.2001 IS
   Объект "маска файлов". Именно его будем использовать для проверки имени
   файла на совпадение с искомым.
*/
static CFileMask FileMaskForFindFile;
/* IS $ */

int _cdecl SortItems(const void *p1,const void *p2)
{
  PluginPanelItem *Item1=(PluginPanelItem *)p1;
  PluginPanelItem *Item2=(PluginPanelItem *)p2;
  char n1[NM*2],n2[NM*2];
  n1[0]=0;n2[0]=0;
  if (*Item1->FindData.cFileName)
    strncpy(n1,Item1->FindData.cFileName, sizeof(n1)-1);
  if (*Item2->FindData.cFileName)
    strncpy(n2,Item2->FindData.cFileName, sizeof(n1)-1);
  *(PointToName(n1))=0;
  *(PointToName(n2))=0;
  return LocalStricmp(n1,n2);
}

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
        strncpy(DataStr,FindText,sizeof(DataStr)-1);
      Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,4,(long)DataStr);

      W=Dlg->Item[0].X2-Dlg->Item[6].X1-3;
      if (strlen(FindCode)>W)
      {
        strncpy(DataStr,FindCode,W-3);
        DataStr[W-4]=0;
        strcat(DataStr,"...");
      }
      else
        strncpy(DataStr,FindCode,sizeof(DataStr)-1);
      Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,6,(long)DataStr);

      if (UseAllTables)
        strncpy(TableSet.TableName,MSG(MFindFileAllTables),sizeof(TableSet.TableName)-1);
      else if (UseUnicode)
        strncpy(TableSet.TableName,"Unicode",sizeof(TableSet.TableName)-1);
      else if (!UseDecodeTable)
        strncpy(TableSet.TableName,MSG(MGetTableNormalText),sizeof(TableSet.TableName)-1);
      else
        PrepareTable(&TableSet,TableNum);
      Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,7,(long)TableSet.TableName);

      FindFoldersChanged = FALSE;

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
          strncpy(TableSet.TableName,MSG(MGetTableNormalText),sizeof(TableSet.TableName)-1);
          if (Param2>=5)
          {
            PrepareTable(&TableSet,Param2-5);
            TableNum=Param2-5;
          }
        }
      }
      return TRUE;
    }
    /* 22.11.2001 VVM
      ! Сбрасыватьсостояние FindFolders при вводе текста.
        Но только если не меняли этот состояние вручную */
    case DN_BTNCLICK:
    {
      /* $ 23.11.2001 KM
         - БЛИН! Правил глюк и глюк добавил :-(
           Перестали работать чекбоксы и радиобатоны.
      */
      /* $ 22.11.2001 KM
         - Забыли однако, что DN_BTNCLICK работает со всеми "нажимающимися"
           контролами, только с кнопками немного по-другому, поэтому простое
           нажатие ENTER на кнопках Find или Cancel ни к чему не приводило.
      */
      if (Param1==22 || Param1==23) // [ Find ] или [ Cancel ]
        return FALSE;
      else if (Param1==13)
        FindFoldersChanged = TRUE;
      return TRUE;
      /* KM $ */
      /* KM $ */
    }
    case DN_EDITCHANGE:
    {
      if ((Param1==5) && (!FindFoldersChanged))
      // Строка "Содержащий текст"
      {
        FarDialogItem &Item=*reinterpret_cast<FarDialogItem*>(Param2);
        BOOL Checked = (*Item.Data.Data)?FALSE:Opt.FindFolders;
        if (Checked)
          Dialog::SendDlgMessage(hDlg, DM_SETCHECK, 13, BSTATE_CHECKED);
        else
          Dialog::SendDlgMessage(hDlg, DM_SETCHECK, 13, BSTATE_UNCHECKED);
      }
      return TRUE;
    }
    /* VVM $ */
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
  strncpy(FindMask,LastFindMask,sizeof(FindMask)-1);
  strncpy(FindStr,LastFindStr,sizeof(FindStr)-1);
  BreakMainThread=0;
  FarList TableList;
  FarListItem *TableItem=(FarListItem *)malloc(sizeof(FarListItem)*4);
  TableList.Items=TableItem;
  TableList.ItemsNumber=4;

  memset(TableItem,0,sizeof(FarListItem)*4);
  strncpy(TableItem[0].Text,MSG(MFindFileAllTables),sizeof(TableItem[0].Text)-1);
  TableItem[1].Flags=LIF_SEPARATOR;
  strncpy(TableItem[2].Text,MSG(MGetTableNormalText),sizeof(TableItem[2].Text)-1);
  strncpy(TableItem[3].Text,"Unicode",sizeof(TableItem[3].Text)-1);

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
      memset(&TableItem[4],0,sizeof(FarListItem));
      TableItem[4].Flags=LIF_SEPARATOR;
      TableList.Items=TableItem;
      TableList.ItemsNumber++;
    }

    TableItem=(FarListItem *)realloc(TableItem,sizeof(FarListItem)*(I+6));
    if (TableItem==NULL)
      return;
    memset(&TableItem[I+5],0,sizeof(FarListItem));
    strncpy(TableItem[I+5].Text,cts.TableName,sizeof(TableItem[I+5].Text)-1);
    TableList.Items=TableItem;
    TableList.ItemsNumber++;
  }

  FindList = NULL;
  ArcList = NULL;
  hPluginMutex=CreateMutex(NULL,FALSE,NULL);

  do
  {
    ClearAllLists();
    const char *MasksHistoryName="Masks",*TextHistoryName="SearchText";
    /* $ 30.07.2000 KM
       Добавлен новый checkbox "Whole words" в диалог поиска
    */
    static struct DialogData FindAskDlgData[]=
    {
      /* 00 */DI_DOUBLEBOX,3,1,72,18,0,0,0,0,(char *)MFindFileTitle,
      /* 01 */DI_TEXT,5,2,0,0,0,0,0,0,(char *)MFindFileMasks,
      /* 02 */DI_EDIT,5,3,70,16,1,(DWORD)MasksHistoryName,DIF_HISTORY|DIF_USELASTHISTORY,0,"",
      /* 03 */DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR2,0,"",
      /* 04 */DI_TEXT,5,5,0,0,0,0,0,0,"",
      /* 05 */DI_EDIT,5,6,36,16,0,(DWORD)TextHistoryName,DIF_HISTORY,0,"",
      /* 06 */DI_TEXT,40,5,0,0,0,0,0,0,"",
      /* 07 */DI_COMBOBOX,40,6,70,10,0,(DWORD)&TableList,DIF_DROPDOWNLIST,0,"",
      /* 08 */DI_TEXT,3,7,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
      /* 09 */DI_VTEXT,38,4,0,0,0,0,DIF_BOXCOLOR,0,"\xD1\xB3\xB3\xC1",
      /* 10 */DI_CHECKBOX,5,8,0,0,0,0,0,0,(char *)MFindFileCase,
      /* 11 */DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MFindFileWholeWords,
      /* 12 */DI_CHECKBOX,40,8,0,0,0,0,0,0,(char *)MFindArchives,
      /* 13 */DI_CHECKBOX,40,9,0,0,0,0,0,0,(char *)MFindFolders,
      /* 14 */DI_TEXT,3,10,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR2,0,"",
      /* 15 */DI_VTEXT,38,7,0,0,0,0,DIF_BOXCOLOR,0,"\xC5\xB3\xB3\xCF",
      /* 16 */DI_RADIOBUTTON,5,11,0,0,0,0,DIF_GROUP,0,(char *)MSearchAllDisks,
      /* 17 */DI_RADIOBUTTON,5,12,0,0,0,1,0,0,(char *)MSearchFromRoot,
      /* 18 */DI_RADIOBUTTON,5,13,0,0,0,0,0,0,(char *)MSearchFromCurrent,
      /* 19 */DI_RADIOBUTTON,5,14,0,0,0,0,0,0,(char *)MSearchInCurrent,
      /* 20 */DI_RADIOBUTTON,5,15,0,0,0,0,0,0,(char *)MSearchInSelected,
      /* 21 */DI_TEXT,3,16,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
      /* 22 */DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP,1,(char *)MFindFileFind,
      /* 23 */DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
    };
    /* KM $ */
    MakeDialogItems(FindAskDlgData,FindAskDlg);

    {
      Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
      PluginMode=ActivePanel->GetMode()==PLUGIN_PANEL && ActivePanel->IsVisible();

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

    strncpy(FindAskDlg[2].Data,FindMask,sizeof(FindAskDlg[2].Data)-1);
    strncpy(FindAskDlg[5].Data,FindStr,sizeof(FindAskDlg[5].Data)-1);
    FindAskDlg[10].Selected=CmpCase;
    FindAskDlg[11].Selected=WholeWords;

    /* $ 14.05.2001 DJ
       не селектим чекбокс, если нельзя искать в архивах
    */
    if (!(FindAskDlg[12].Flags & DIF_DISABLE))
      FindAskDlg[12].Selected=SearchInArchives;
    /* DJ $ */
    if (!*FindStr)
      FindAskDlg[13].Selected=Opt.FindFolders;
    FindAskDlg[16].Selected=FindAskDlg[17].Selected=0;
    FindAskDlg[18].Selected=FindAskDlg[19].Selected=0;
    FindAskDlg[20].Selected=0;
    FindAskDlg[16+SearchMode].Selected=1;

    while (1)
    {
      int ExitCode;
      {
        Dialog Dlg(FindAskDlg,sizeof(FindAskDlg)/sizeof(FindAskDlg[0]),MainDlgProc);

        Dlg.SetHelp("FindFile");
        Dlg.SetPosition(-1,-1,76,20);
        Dlg.Process();
        ExitCode=Dlg.GetExitCode();
      }
      if (ExitCode!=22)
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
    if (strlen (FindAskDlg[5].Data) > sizeof(FindStr) ){
      memset (Buf1, 0, sizeof(Buf1));
      memset (Buf2, 0, sizeof(Buf2));
      strncpy (Buf1, MSG(MFindFileText), sizeof(Buf1)-1);
      RemoveHighlights(Buf1);
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
    if (FindFoldersChanged)
      Opt.FindFolders=FindAskDlg[13].Selected;
    if (*FindStr)
    {
      strncpy(GlobalSearchString,FindStr,sizeof(GlobalSearchString)-1);
      GlobalSearchCase=CmpCase;
      /* $ 30.07.2000 KM
         Добавлена переменная
      */
      GlobalSearchWholeWords=WholeWords;
      /* KM $ */
    }
    if (FindAskDlg[16].Selected)
      SearchMode=SEARCH_ALL;
    if (FindAskDlg[17].Selected)
      SearchMode=SEARCH_ROOT;
    if (FindAskDlg[18].Selected)
      SearchMode=SEARCH_FROM_CURRENT;
    if (FindAskDlg[19].Selected)
      SearchMode=SEARCH_CURRENT_ONLY;
    if (FindAskDlg[20].Selected)
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
    strncpy(LastFindMask,FindMask,sizeof(LastFindMask)-1);
    strncpy(LastFindStr,FindStr,sizeof(LastFindStr)-1);
    if (*FindStr)
      Editor::SetReplaceMode(FALSE);
  } while (FindFilesProcess());
  CloseHandle(hPluginMutex);
  free(TableItem);
}


FindFiles::~FindFiles()
{
  /* $ 02.07.2001 IS
     Освободим память.
  */
  FileMaskForFindFile.Free();
  /* IS $ */
  ClearAllLists();
  ScrBuf.ResetShadow();
}

int FindFiles::GetPluginFile(DWORD ArcIndex, struct PluginPanelItem *PanelItem,
                             char *DestPath, char *ResultName)
{
  HANDLE hPlugin = ArcList[ArcIndex].hPlugin;
  char SaveDir[NM];
  struct OpenPluginInfo Info;
  CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
  strncpy(SaveDir,Info.CurDir,sizeof(SaveDir)-1);
  AddEndSlash(SaveDir);

  CtrlObject->Plugins.SetDirectory(hPlugin,"\\",OPM_SILENT|OPM_FIND);
  SetPluginDirectory(ArcList[ArcIndex].RootPath, hPlugin);
  SetPluginDirectory(PanelItem->FindData.cFileName, hPlugin);

  PluginPanelItem NewItem = *PanelItem;
  char *FileName = PointToName(NewItem.FindData.cFileName);
  if (FileName != NewItem.FindData.cFileName)
    strncpy(NewItem.FindData.cFileName, FileName, sizeof(NewItem.FindData.cFileName));
  int Result = CtrlObject->Plugins.GetFile(hPlugin,&NewItem,DestPath,ResultName,OPM_SILENT|OPM_FIND);

  CtrlObject->Plugins.SetDirectory(hPlugin,"\\",OPM_SILENT|OPM_FIND);
  SetPluginDirectory(ArcList[ArcIndex].RootPath, hPlugin);
  SetPluginDirectory(SaveDir, hPlugin);
  return(Result);
}

long WINAPI FindFiles::FindDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  Dialog* Dlg=(Dialog*)hDlg;
  VMenu *ListBox=Dlg->Item[1].ListPtr;

  switch(Msg)
  {
    case DN_HELP: // в режиме поиска отключим вывод помощи, ибо... артефакты с прорисовкой
      return !SearchDone?NULL:Param2;

    case DN_MOUSECLICK:
    {
      SMALL_RECT drect,rect;
      Dialog::SendDlgMessage(hDlg,DM_GETDLGRECT,0,(long)&drect);
      Dialog::SendDlgMessage(hDlg,DM_GETITEMPOSITION,1,(long)&rect);
      if (Param1==1 && ((MOUSE_EVENT_RECORD *)Param2)->dwMousePosition.X<drect.Left+rect.Right)
      {
        if (ListBox && ListBox->GetItemCount())
          Dialog::SendDlgMessage(hDlg,DM_CLOSE,6/* [ Go to ] */,0);
        FindDlgProc(hDlg,DN_BTNCLICK,6,0); // emulates a [ Go to ] button pressing
        return TRUE;
      }
      return FALSE;
    }

    case DN_KEY:
    {
      WaitForSingleObject(hDialogMutex,INFINITE);

      if (!StopSearch && Param2==KEY_ESC)
      {
        PauseSearch=TRUE;
        IsProcessAssignMacroKey++; // запретим спец клавиши
                                   // т.е. в этом диалоге нельзя нажать Alt-F9!
        int LocalRes=TRUE;
		/* $ 04.03.2002 DJ
		   нажатие Esc в диалоге confirm Esc поиск _не_ прерывает
		*/
        if(Opt.Confirm.Esc && Message(MSG_WARNING,2,MSG(MKeyESCWasPressed),
                      MSG(MDoYouWantToStopWork),MSG(MYes),MSG(MNo))!=0)
          LocalRes=FALSE;
		/* DJ $ */
        IsProcessAssignMacroKey--;
        PauseSearch=FALSE;
        StopSearch=LocalRes;
        ReleaseMutex(hDialogMutex);
        return TRUE;
      }

      if (!ListBox)
      {
        ReleaseMutex(hDialogMutex);
        return TRUE;
      }

      while (ListBox->GetCallCount())
        Sleep(10);

      // некторые спец.клавиши всеже отбработаем.
      if(Param2 == KEY_CTRLALTSHIFTPRESS || Param2 == KEY_ALTF9)
      {
        IsProcessAssignMacroKey--;
        FrameManager->ProcessKey(Param2);
        IsProcessAssignMacroKey++;
        ReleaseMutex(hDialogMutex);
        return TRUE;
      }

      if (Param1==9 && (Param2==KEY_RIGHT || Param2==KEY_TAB)) // [ Stop ] button
      {
//        while (ListBox->GetCallCount())
//          Sleep(10);
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,5/* [ New search ] */,0);
        ReleaseMutex(hDialogMutex);
        return TRUE;
      }
      else if (Param1==5 && (Param2==KEY_LEFT || Param2==KEY_SHIFTTAB)) // [ New search ] button
      {
//        while (ListBox->GetCallCount())
//          Sleep(10);
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,9/* [ Stop ] */,0);
        ReleaseMutex(hDialogMutex);
        return TRUE;
      }
      else if (Param2==KEY_UP || Param2==KEY_DOWN || Param2==KEY_PGUP ||
               Param2==KEY_PGDN || Param2==KEY_HOME || Param2==KEY_END ||
               Param2==KEY_MSWHEEL_UP || Param2==KEY_MSWHEEL_DOWN)
      {
//        while (ListBox->GetCallCount())
//          Sleep(10);
        ListBox->ProcessKey(Param2);
        ReleaseMutex(hDialogMutex);
        return TRUE;
      }
      else if (Param2==KEY_F3 || Param2==KEY_NUMPAD5 || Param2==KEY_F4)
      {
        if (ListBox->GetItemCount()==0)
        {
          ReleaseMutex(hDialogMutex);
          return TRUE;
        }

        DWORD ItemIndex = (DWORD)ListBox->GetUserData(NULL, 0);
        if (ItemIndex != LIST_INDEX_NONE)
        {
          int RemoveTemp=FALSE;
          char SearchFileName[NM];
          char TempDir[NM];

          if (FindList[ItemIndex].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
          {
            ReleaseMutex(hDialogMutex);
            return TRUE;
          }
          char *FileName=FindList[ItemIndex].FindData.cFileName;
          // FindFileArcIndex нельзя здесь использовать
          // Он может быть уже другой.
          if ((FindList[ItemIndex].ArcIndex != LIST_INDEX_NONE) &&
              (!(ArcList[FindList[ItemIndex].ArcIndex].Flags & OPIF_REALNAMES)))
          {
            char *FindArcName = ArcList[FindList[ItemIndex].ArcIndex].ArcName;
            if (ArcList[FindList[ItemIndex].ArcIndex].hPlugin == INVALID_HANDLE_VALUE)
            {
              char *Buffer=new char[MAX_READ];
              FILE *ProcessFile=fopen(FindArcName,"rb");
              if (ProcessFile==NULL)
              {
                delete[] Buffer;
                ReleaseMutex(hDialogMutex);
                return TRUE;
              }
              int ReadSize=fread(Buffer,1,MAX_READ,ProcessFile);
              fclose(ProcessFile);

              int SavePluginsOutput=DisablePluginsOutput;
              DisablePluginsOutput=TRUE;
              WaitForSingleObject(hPluginMutex,INFINITE);
              ArcList[FindList[ItemIndex].ArcIndex].hPlugin = CtrlObject->Plugins.OpenFilePlugin(FindArcName,(unsigned char *)Buffer,ReadSize);
              ReleaseMutex(hPluginMutex);
              DisablePluginsOutput=SavePluginsOutput;

              delete[] Buffer;

              if (ArcList[FindList[ItemIndex].ArcIndex].hPlugin == (HANDLE)-2 ||
                  ArcList[FindList[ItemIndex].ArcIndex].hPlugin == INVALID_HANDLE_VALUE)
              {
                ReleaseMutex(hDialogMutex);
                ArcList[FindList[ItemIndex].ArcIndex].hPlugin = INVALID_HANDLE_VALUE;
                return TRUE;
              }
            }

            PluginPanelItem FileItem;
            memset(&FileItem,0,sizeof(FileItem));
            FileItem.FindData=FindList[ItemIndex].FindData;
            FarMkTempEx(TempDir); // А проверка на NULL???
            CreateDirectory(TempDir, NULL);
//            if (!CtrlObject->Plugins.GetFile(ArcList[FindList[ItemIndex].ArcIndex].hPlugin,&FileItem,TempDir,SearchFileName,OPM_SILENT|OPM_FIND))
            WaitForSingleObject(hPluginMutex,INFINITE);
            if (!GetPluginFile(FindList[ItemIndex].ArcIndex,&FileItem,TempDir,SearchFileName))
            {
              RemoveDirectory(TempDir);
              ReleaseMutex(hPluginMutex);
              ReleaseMutex(hDialogMutex);
              return FALSE;
            }
            else
              ReleaseMutex(hPluginMutex);
            RemoveTemp=TRUE;
          }
          else
            strncpy(SearchFileName,FindList[ItemIndex].FindData.cFileName,sizeof(SearchFileName)-1);

          DWORD FileAttr;
          if ((FileAttr=GetFileAttributes(SearchFileName))!=(DWORD)-1)
          {
            char OldTitle[512];
            GetConsoleTitle(OldTitle,sizeof(OldTitle));

            if (Param2==KEY_F3 || Param2==KEY_NUMPAD5)
            {
              NamesList ViewList;
              // Возьмем все файлы, которые имеют реальные имена...
              {
                int ListSize=ListBox->GetItemCount();
                DWORD Index;
                for (int I=0;I<ListSize;I++)
                {
                  Index = (DWORD)ListBox->GetUserData(NULL, 0, I);
                  if ((Index != LIST_INDEX_NONE) &&
                      ((FindList[Index].ArcIndex == LIST_INDEX_NONE) ||
                       (ArcList[FindList[Index].ArcIndex].Flags & OPIF_REALNAMES)))
                  {
                    int Length=strlen(FindList[Index].FindData.cFileName);
                    // Не учитывали файлы в архивах с OPIF_REALNAMES
                    if (Length>0 && !(FindList[Index].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
                      ViewList.AddName(FindList[Index].FindData.cFileName);
                  } /* if */
                } /* for */
                ViewList.SetCurName(FindList[ItemIndex].FindData.cFileName);
              }
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
              ReleaseMutex(hDialogMutex);
              {
                FileViewer ShellViewer (SearchFileName,FALSE,FALSE,FALSE,-1,NULL,(FindList[ItemIndex].ArcIndex != LIST_INDEX_NONE)?NULL:&ViewList);
                ShellViewer.SetDynamicallyBorn(FALSE);
                ShellViewer.SetEnableF6(TRUE);
                // FindFileArcIndex нельзя здесь использовать
                // Он может быть уже другой.
                if ((FindList[ItemIndex].ArcIndex != LIST_INDEX_NONE) &&
                    (!(ArcList[FindList[ItemIndex].ArcIndex].Flags & OPIF_REALNAMES)))
                  ShellViewer.SetSaveToSaveAs(TRUE);
                IsProcessVE_FindFile++;
                FrameManager->ExecuteModal ();
                IsProcessVE_FindFile--;
                // заставляем рефрешится экран
                FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
              }
              WaitForSingleObject(hDialogMutex,INFINITE);

              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
            }
            else
            {
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
              ReleaseMutex(hDialogMutex);
              {
                int FramePos=FrameManager->FindFrameByFile(MODALTYPE_EDITOR,SearchFileName);
                int SwitchTo=FALSE;
                if (FramePos!=-1)
                {
                  if (!(*FrameManager)[FramePos]->GetCanLoseFocus(TRUE) ||
                      Opt.Confirm.AllowReedit)
                  {
                    char MsgFullFileName[NM];
                    strncpy(MsgFullFileName,SearchFileName,sizeof(MsgFullFileName)-1);
                    int MsgCode=Message(0,2,MSG(MFindFileTitle),
                          TruncPathStr(MsgFullFileName,ScrX-16),
                          MSG(MAskReload),
                          MSG(MCurrent),MSG(MNewOpen));
                    if (MsgCode==0)
                    {
                      SwitchTo=TRUE;
                    }
                    else if (MsgCode==1)
                    {
                      SwitchTo=FALSE;
                    }
                    else
                    {
                      Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
                      Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
                      return TRUE;
                    }
                  }
                  else
                  {
                    SwitchTo=TRUE;
                  }
                }
                if (SwitchTo)
                {
                  (*FrameManager)[FramePos]->SetCanLoseFocus(FALSE);
                  (*FrameManager)[FramePos]->SetDynamicallyBorn(FALSE);
                  FrameManager->ActivateFrame(FramePos);
                  IsProcessVE_FindFile++;
                  FrameManager->ExecuteNonModal();
                  IsProcessVE_FindFile--;
                  // заставляем рефрешится экран
                  FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
                }
                else
                {
                  FileEditor ShellEditor (SearchFileName,FALSE,FALSE);
                  ShellEditor.SetDynamicallyBorn(FALSE);
                  ShellEditor.SetEnableF6 (TRUE);
                  // FindFileArcIndex нельзя здесь использовать
                  // Он может быть уже другой.
                  if ((FindList[ItemIndex].ArcIndex != LIST_INDEX_NONE) &&
                      (!(ArcList[FindList[ItemIndex].ArcIndex].Flags & OPIF_REALNAMES)))
                    ShellEditor.SetSaveToSaveAs(TRUE);
                  IsProcessVE_FindFile++;
                  FrameManager->ExecuteModal ();
                  IsProcessVE_FindFile--;
                  // заставляем рефрешится экран
                  FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
                }
              }
              // А оно тут надо? Мы ведь при заходе в DN_KEY его захватили...
              WaitForSingleObject(hDialogMutex,INFINITE);
              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
            }
            SetConsoleTitle(OldTitle);
          }
          if (RemoveTemp)
            DeleteFileWithFolder(SearchFileName);
        }
        ReleaseMutex(hDialogMutex);
        return TRUE;
      }
      ReleaseMutex(hDialogMutex);
      return FALSE;
    }
    case DN_BTNCLICK:
    {
      if (Param1==5) // [ New search ] button pressed
      {
        FindExitCode=FIND_EXIT_SEARCHAGAIN;
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
        if (!ListBox)
          return FALSE;

        WaitForSingleObject(hDialogMutex,INFINITE);

        // Переход будем делать так же после выхода из диалога.
        // Причину смотри для [ Panel ]
        if (ListBox->GetItemCount()==0)
        {
          ReleaseMutex(hDialogMutex);
          return (TRUE);
        }
        FindExitIndex = (DWORD)ListBox->GetUserData(NULL, 0);
        if (FindExitIndex != LIST_INDEX_NONE)
          FindExitCode = FIND_EXIT_GOTO;
        ReleaseMutex(hDialogMutex);
        return (FALSE);
      }
      else if (Param1==7) // [ View ] button pressed
      {
        FindDlgProc(hDlg,DN_KEY,1,KEY_F3);
        return TRUE;
      }
      else if (Param1==8) // [ Panel ] button pressed
      {
        if (!ListBox)
          return FALSE;

        WaitForSingleObject(hDialogMutex,INFINITE);

        // На панель будем посылать не в диалоге, а после.
        // После окончания поиска. Иначе возможна ситуация, когда мы
        // ищем на панели, потом ее грохаем и создаем новую (а поиск-то
        // идет!) и в результате ФАР трапается.
        if (ListBox->GetItemCount()==0)
        {
          ReleaseMutex(hDialogMutex);
          return (TRUE);
        }
        FindExitCode = FIND_EXIT_PANEL;
        FindExitIndex = (DWORD)ListBox->GetUserData(NULL, 0);
        ReleaseMutex(hDialogMutex);
        return (FALSE);
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
      WaitForSingleObject(hDialogMutex,INFINITE);

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

      ReleaseMutex(hDialogMutex);
      return TRUE;
    }
    /* KM $ */
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

int FindFiles::FindFilesProcess()
{
  char Title[2*NM];
  char SearchStr[NM];
  hDialogMutex=CreateMutex(NULL,FALSE,NULL);

  if (*FindMask)
    sprintf(Title,"%s: %s",MSG(MFindFileTitle),FindMask);
  else
    sprintf(Title,"%s",MSG(MFindFileTitle));
  if (*FindStr)
  {
    char Temp[NM],FStr[NM];
    strncpy(FStr,FindStr,sizeof(FStr)-1);
    sprintf(Temp," \"%s\"",TruncStrFromEnd(FStr,10));
    sprintf(SearchStr,MSG(MFindSearchingIn),Temp);
  }
  else
    sprintf(SearchStr,MSG(MFindSearchingIn),"");

  /* $ 03.12.2001 DJ
     корректный показ имен файлов с амперсандами
  */
  static struct DialogData FindDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,72,DLG_HEIGHT-2,0,0,0,0,Title,
  /* 01 */DI_LISTBOX,4,2,71,14,0,0,DIF_LISTNOBOX,0,(char*)0,
  /* 02 */DI_TEXT,-1,15,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 03 */DI_TEXT,5,16,0,0,0,0,DIF_SHOWAMPERSAND,0,SearchStr,
  /* 04 */DI_TEXT,3,17,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 05 */DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MFindNewSearch,
  /* 06 */DI_BUTTON,0,18,0,0,1,0,DIF_CENTERGROUP,1,(char *)MFindGoTo,
  /* 07 */DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MFindView,
  /* 08 */DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MFindPanel,
  /* 09 */DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MFindStop
  };
  /* DJ $ */
  MakeDialogItems(FindDlgData,FindDlg);

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

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

  if (PluginMode)
  {
    Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
    HANDLE hPlugin=ActivePanel->GetPluginHandle();
    struct OpenPluginInfo Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

    FindFileArcIndex = AddArcListItem(Info.HostFile, hPlugin, Info.Flags, Info.CurDir);
    if (FindFileArcIndex == LIST_INDEX_NONE)
      return(FALSE);

    if ((Info.Flags & OPIF_REALNAMES)==0)
    {
      FindDlg[8].Type=DI_TEXT;
      *FindDlg[8].Data=0;
    }
  }

  DlgWidth=FindDlg[0].X2-FindDlg[0].X1-4;
  Dialog *pDlg=new Dialog(FindDlg,sizeof(FindDlg)/sizeof(FindDlg[0]),FindDlgProc);
  hDlg=(HANDLE)pDlg;
  pDlg->SetDynamicallyBorn(TRUE);
  pDlg->SetHelp("FindFile");
  pDlg->SetPosition(-1,-1,76,DLG_HEIGHT+IncY);
  // Надо бы показать диалог, а то инициализация элементов запаздывает
  // иногда при поиске и первые элементы не добавляются
  pDlg->Show();

  LastFoundNumber=0;
  SearchDone=FALSE;
  StopSearch=FALSE;
  PauseSearch=FALSE;
  WriteDataUsed=FALSE;
  FindFileCount=FindDirCount=0;
  FindExitIndex = LIST_INDEX_NONE;
  FindExitCode = FIND_EXIT_NONE;
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

  CloseHandle(hDialogMutex);

  switch (FindExitCode)
  {
    case FIND_EXIT_SEARCHAGAIN:
    {
      return TRUE;
    }
    case FIND_EXIT_PANEL:
    // Отработаем переброску на временную панель
    {
      int ListSize = FindListCount;
      PluginPanelItem *PanelItems=new PluginPanelItem[ListSize];
      if (PanelItems==NULL)
        ListSize=0;
      int ItemsNumber=0;
      for (int i=0;i<ListSize;i++)
      {
        char *FileName=FindList[i].FindData.cFileName;
        int Length=strlen(FileName);
        if (Length>0)
        // Добавляем всегда, если имя задано
        {
          // Для плагинов с виртуальными именами заменим имя файла на имя архива.
          // панель сама уберет лишние дубли.
          int IsArchive = ((FindList[i].ArcIndex != LIST_INDEX_NONE) &&
                          !(ArcList[FindList[i].ArcIndex].Flags&OPIF_REALNAMES));
          // Добавляем только файлы или имена архивов
          /* $ 13.11.2001 VVM
            ! Хм. Добавим папки, если их искали... */
          if (IsArchive || Opt.FindFolders ||
              !(FindList[i].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
          /* VVM $ */
          {
            if (IsArchive)
              strncpy(FileName,ArcList[FindList[i].ArcIndex].ArcName,sizeof(FileName)-1);
            PluginPanelItem *pi=&PanelItems[ItemsNumber++];
            memset(pi,0,sizeof(*pi));
            pi->FindData=FindList[i].FindData;
            if (IsArchive)
              pi->FindData.dwFileAttributes = 0;
          }
        } /* if */
      } /* for */

      HANDLE hNewPlugin=CtrlObject->Plugins.OpenFindListPlugin(PanelItems,ItemsNumber);
      if (hNewPlugin!=INVALID_HANDLE_VALUE)
      {
        Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
        Panel *NewPanel=CtrlObject->Cp()->ChangePanel(ActivePanel,FILE_PANEL,TRUE,TRUE);
        NewPanel->SetPluginMode(hNewPlugin,"");
        NewPanel->SetVisible(TRUE);
        NewPanel->Update(0);
//        if (FindExitIndex != LIST_INDEX_NONE)
//          NewPanel->GoToFile(FindList[FindExitIndex].FindData.cFileName);
        NewPanel->Show();
        NewPanel->SetFocus();
      }
      /* $ 13.07.2000 SVS
         использовали new[]
      */
      delete[] PanelItems;
      break;
    } /* case FIND_EXIT_PANEL */
    case FIND_EXIT_GOTO:
    {
      char *FileName=FindList[FindExitIndex].FindData.cFileName;
      Panel *FindPanel=CtrlObject->Cp()->ActivePanel;

      if ((FindList[FindExitIndex].ArcIndex != LIST_INDEX_NONE) &&
          (!(ArcList[FindList[FindExitIndex].ArcIndex].Flags & OPIF_REALNAMES)))
      {
        HANDLE hPlugin = ArcList[FindList[FindExitIndex].ArcIndex].hPlugin;
        if (hPlugin == INVALID_HANDLE_VALUE)
        {
          char ArcName[NM],ArcPath[NM];
          strncpy(ArcName,ArcList[FindList[FindExitIndex].ArcIndex].ArcName,sizeof(ArcName)-1);
          if (FindPanel->GetType()!=FILE_PANEL)
            FindPanel=CtrlObject->Cp()->ChangePanel(FindPanel,FILE_PANEL,TRUE,TRUE);
          strncpy(ArcPath,ArcName,sizeof(ArcPath)-1);
          *PointToName(ArcPath)=0;
          FindPanel->SetCurDir(ArcPath,TRUE);
          hPlugin=((FileList *)FindPanel)->OpenFilePlugin(ArcName,FALSE);
          if (hPlugin==(HANDLE)-2)
            hPlugin = INVALID_HANDLE_VALUE;
        } /* if */
        if (hPlugin != INVALID_HANDLE_VALUE)
        {
          char *StartName = PointToName(FileName);
          Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
          if (SearchMode==SEARCH_ROOT || SearchMode==SEARCH_ALL)
            CtrlObject->Plugins.SetDirectory(hPlugin,"\\",OPM_FIND);
          SetPluginDirectory(FileName, hPlugin);
          ActivePanel->Update(UPDATE_KEEP_SELECTION);
          if (!ActivePanel->GoToFile(StartName))
            ActivePanel->GoToFile(FileName);
          ActivePanel->Show();
        }
      } /* if */
      else
      {
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
          strncpy(SetName,NamePtr,sizeof(SetName)-1);
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
        if ((FindPanel->GetType()!=FILE_PANEL) || (FindPanel->GetMode()!=NORMAL_PANEL))
        // Сменим панель на обычную файловую...
        {
          FindPanel=CtrlObject->Cp()->ChangePanel(FindPanel,FILE_PANEL,TRUE,TRUE);
          FindPanel->SetVisible(TRUE);
          FindPanel->Update(0);
        }
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
        FindPanel->SetFocus();
      }
      break;
    } /* case FIND_EXIT_GOTO */
  } /* switch */

  return FALSE;
}


void FindFiles::SetPluginDirectory(char *DirName, HANDLE hPlugin)
{
  char Name[NM],*StartName,*EndName;
  strncpy(Name,DirName,sizeof(Name)-1);
  StartName=Name;
  while ((EndName=strchr(StartName,'\\'))!=NULL)
  {
    *EndName=0;
    // RereadPlugin
    {
      int FileCount=0;
      PluginPanelItem *PanelData=NULL;
      if (CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&FileCount,OPM_FIND))
        CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,FileCount);
    }
    CtrlObject->Plugins.SetDirectory(hPlugin,StartName,OPM_FIND);
    StartName=EndName+1;
  }
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
        strncpy(CurRoot,Root,sizeof(CurRoot)-1);
        AddEndSlash(CurRoot);
        strcat(CurRoot,SelName);
      }
      else
        strncpy(CurRoot,Root,sizeof(CurRoot)-1);

      ScTree.SetFindPath(CurRoot,"*.*");

      strncpy(FindMessage,CurRoot,sizeof(FindMessage)-1);
      FindMessage[sizeof(FindMessage)-1]=0;
      FindMessageReady=TRUE;

      while (!StopSearch && ScTree.GetNextName(&FindData,FullName))
      {
        while (PauseSearch)
          Sleep(10);

        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
          strncpy(FindMessage,FullName,sizeof(FindMessage)-1);
          FindMessage[sizeof(FindMessage)-1]=0;
          FindMessageReady=TRUE;
        }

        if (IsFileIncluded(NULL,FullName,FindData.dwFileAttributes))
          AddMenuRecord(FullName,&FindData);

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
  sprintf(FindMessage,MSG(MFindDone),FindFileCount,FindDirCount);
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
  /* $ 01.10.2001 VVM */
  DisablePluginsOutput=SavePluginsOutput;
  /* VVM $ */


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
  DWORD SaveArcIndex = FindFileArcIndex;
  {
    SearchMode=SEARCH_FROM_CURRENT;
    struct OpenPluginInfo Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hArc,&Info);
    FindFileArcIndex = AddArcListItem(ArcName, hArc, Info.Flags, Info.CurDir);
    /* $ 11.12.2001 VVM
      - Запомним каталог перед поиском в архиве.
        И если ничего не нашли - не рисуем его снова */
    {
      char SaveDirName[NM];
      int SaveListCount = FindListCount;
      strncpy(SaveDirName, LastDirName, NM);
      *LastDirName = 0;
      PreparePluginList((void *)1);
      WaitForSingleObject(hPluginMutex,INFINITE);
      CtrlObject->Plugins.ClosePlugin(ArcList[FindFileArcIndex].hPlugin);
      ArcList[FindFileArcIndex].hPlugin = INVALID_HANDLE_VALUE;
      ReleaseMutex(hPluginMutex);
      if (SaveListCount == FindListCount)
        strncpy(LastDirName, SaveDirName, NM);
    }
    /* VVM $ */
  }
  FindFileArcIndex = SaveArcIndex;
  SearchMode=SaveSearchMode;
}

/* $ 01.07.2001 IS
   Используем FileMaskForFindFile вместо GetCommaWord
*/
int FindFiles::IsFileIncluded(PluginPanelItem *FileItem,char *FullName,DWORD FileAttr)
{
  int FileFound=FileMaskForFindFile.Compare(FullName);
  HANDLE hPlugin=INVALID_HANDLE_VALUE;
  if (FindFileArcIndex != LIST_INDEX_NONE)
    hPlugin = ArcList[FindFileArcIndex].hPlugin;
  while(FileFound)
  {
    /* $ 17.01.2002 VVM
      ! Поскольку работу с поиском в папках вынесли в диалог -
        флаг в плагине потерял свою актуальность */
    if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY) && (Opt.FindFolders==0))
//        ((hPlugin == INVALID_HANDLE_VALUE) ||
//        (ArcList[FindFileArcIndex].Flags & OPIF_FINDFOLDERS)==0))
      return FALSE;
    /* VVM $ */

    if (*FindStr && FileFound)
    {
      FileFound=FALSE;
      if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
        break;
      char SearchFileName[NM];
      int RemoveTemp=FALSE;
      if ((hPlugin != INVALID_HANDLE_VALUE) && (ArcList[FindFileArcIndex].Flags & OPIF_REALNAMES)==0)
      {
        char TempDir[NM];
        FarMkTempEx(TempDir); // А проверка на NULL???
        CreateDirectory(TempDir,NULL);
        WaitForSingleObject(hPluginMutex,INFINITE);
        if (!CtrlObject->Plugins.GetFile(hPlugin,FileItem,TempDir,SearchFileName,OPM_SILENT|OPM_FIND))
        {
          ReleaseMutex(hPluginMutex);
          RemoveDirectory(TempDir);
          break;
        }
        else
          ReleaseMutex(hPluginMutex);
        RemoveTemp=TRUE;
      }
      else
        strncpy(SearchFileName,FullName,sizeof(SearchFileName)-1);
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


//void FindFiles::AddMenuRecord(char *FullName, char *Path, WIN32_FIND_DATA *FindData)
void FindFiles::AddMenuRecord(char *FullName, WIN32_FIND_DATA *FindData)
{
  char MenuText[NM],FileText[NM],SizeText[30];
  char Date[30],DateStr[30],TimeStr[30];
  struct MenuItem ListItem;
  int i;

  Dialog* Dlg=(Dialog*)hDlg;
  VMenu *ListBox=Dlg->Item[1].ListPtr;
  if (!ListBox)
    return;

  WaitForSingleObject(hDialogMutex,INFINITE);

  memset(&ListItem,0,sizeof(ListItem));

  sprintf(SizeText,"%10u",FindData->nFileSizeLow);
  char *DisplayName=FindData->cFileName;
  if ((FindFileArcIndex != LIST_INDEX_NONE) &&
      (ArcList[FindFileArcIndex].Flags & OPIF_REALNAMES))
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
  /* $ 16.01.2002 VVM
    ! Все равно полный путь передается в FullName, так что эта обработка лишняя */
//  if (Path)
//  {
//    for (i=0;Path[i]!=0;i++)
//    {
//      if (Path[i]=='\x1')
//        Path[i]='\\';
//    }
//    strcpy(PathName,Path);
//  }
//  else
//  {
    strncpy(PathName,FullName,sizeof(PathName)-1);
    PathName[sizeof(PathName)-1]=0;
    if ((FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
      *PointToName(PathName)=0;
//  }
  /* VVM $ */
  if (*PathName==0)
    strcpy(PathName,".\\");

  AddEndSlash(PathName);
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
      // С подачи VVM сделано добавление в список индекса LIST_INDEX_NONE на пустых строках
      ListBox->SetUserData((void*)LIST_INDEX_NONE,sizeof(LIST_INDEX_NONE),ListBox->AddItem(&ListItem));
      ListItem.Flags&=~LIF_DISABLE;
      /* DJ $ */
    }
    strncpy(LastDirName,PathName,sizeof(LastDirName)-1);
    if ((FindFileArcIndex != LIST_INDEX_NONE) &&
        (!(ArcList[FindFileArcIndex].Flags & OPIF_REALNAMES)) &&
        (ArcList[FindFileArcIndex].ArcName) &&
        (*ArcList[FindFileArcIndex].ArcName))
    {
      char ArcPathName[NM*2];
      sprintf(ArcPathName,"%s:%s",ArcList[FindFileArcIndex].ArcName,*PathName=='.' ? "\\":PathName);
      strncpy(PathName,ArcPathName,sizeof(PathName)-1);
    }
    strncpy(SizeText,MSG(MFindFileFolder),sizeof(SizeText)-1);
    sprintf(FileText,"%-50.50s     <%6.6s>",TruncPathStr(PathName,50),SizeText);
    sprintf(ListItem.Name,"%-*.*s",DlgWidth-2,DlgWidth-2,FileText);

    DWORD ItemIndex = AddFindListItem(FindData);
    if (ItemIndex != LIST_INDEX_NONE)
    {
      // Сбросим данные в FindData. Они там от файла
      memset(&FindList[ItemIndex].FindData,0,sizeof(FindList[ItemIndex].FindData));
      // Используем LastDirName, т.к. PathName уже может быть искажена
      strncpy(FindList[ItemIndex].FindData.cFileName, LastDirName,
              sizeof(FindList[ItemIndex].FindData.cFileName)-1);
      // Поставим атрибут у каталога, что-бы он не был файлом :)
      FindList[ItemIndex].FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
      if (FindFileArcIndex != LIST_INDEX_NONE)
        FindList[ItemIndex].ArcIndex = FindFileArcIndex;

      while (ListBox->GetCallCount())
        Sleep(10);
      ListBox->SetUserData((void*)ItemIndex,sizeof(ItemIndex),
                           ListBox->AddItem(&ListItem));
    }
  }

  if ((FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
  {
    DWORD ItemIndex = AddFindListItem(FindData);
    if (ItemIndex != LIST_INDEX_NONE)
    {
      strncpy(FindList[ItemIndex].FindData.cFileName, FullName,
              sizeof(FindList[ItemIndex].FindData.cFileName)-1);
      if (FindFileArcIndex != LIST_INDEX_NONE)
        FindList[ItemIndex].ArcIndex = FindFileArcIndex;
    }
    strncpy(ListItem.Name,MenuText,sizeof(ListItem.Name)-1);
    /* $ 17.01.2002 VVM
      ! Выделять будем не в структуре, а в списке. Дабы не двоилось выделение */
//    ListItem.SetSelect(!FindFileCount);

    while (ListBox->GetCallCount())
      Sleep(10);

    int ListPos = ListBox->AddItem(&ListItem);
    ListBox->SetUserData((void*)ItemIndex,sizeof(ItemIndex), ListPos);
    // Выделим как положено - в списке.
    if (!FindFileCount)
      ListBox->SetSelectPos(ListPos, -1);
    /* VVM $ */
    FindFileCount++;
  }
  else
    FindDirCount++;

  LastFoundNumber++;
  FindCountReady=TRUE;
  ReleaseMutex(hDialogMutex);
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
  strncpy(CmpStr,FindStr,sizeof(CmpStr)-1);
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
          strncpy(CmpStr,FindStr,sizeof(CmpStr)-1);
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

  Sleep(200);
  *PluginSearchPath=0;
  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  /* $ 15.10.2001 VVM */
  HANDLE hPlugin=ArcList[FindFileArcIndex].hPlugin;
  struct OpenPluginInfo Info;
  CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
  strncpy(SaveDir,Info.CurDir,sizeof(SaveDir)-1);
  WaitForSingleObject(hPluginMutex,INFINITE);
  if (SearchMode==SEARCH_ROOT || SearchMode==SEARCH_ALL)
    CtrlObject->Plugins.SetDirectory(hPlugin,"\\",OPM_FIND);
  ReleaseMutex(hPluginMutex);
  RecurseLevel=0;
  ScanPluginTree(hPlugin,ArcList[FindFileArcIndex].Flags);
  /* VVM $ */
  WaitForSingleObject(hPluginMutex,INFINITE);
  if (SearchMode==SEARCH_ROOT || SearchMode==SEARCH_ALL)
    CtrlObject->Plugins.SetDirectory(hPlugin,SaveDir,OPM_FIND);
  ReleaseMutex(hPluginMutex);
  while (!StopSearch && FindMessageReady)
    Sleep(10);
  if (Param==NULL)
  {
    sprintf(FindMessage,MSG(MFindDone),FindFileCount,FindDirCount);
    FindMessageReady=TRUE;
    SearchDone=TRUE;
  }
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif

void FindFiles::ScanPluginTree(HANDLE hPlugin, DWORD Flags)
{
  PluginPanelItem *PanelData=NULL;
  int ItemCount=0;

  WaitForSingleObject(hPluginMutex,INFINITE);
  if (StopSearch || !CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&ItemCount,OPM_FIND))
  {
    ReleaseMutex(hPluginMutex);
    return;
  }
  else
    ReleaseMutex(hPluginMutex);
  RecurseLevel++;

  if ((FindFileArcIndex != LIST_INDEX_NONE) &&
      (ArcList[FindFileArcIndex].Flags & OPIF_REALNAMES))
  {
    qsort((void *)PanelData,ItemCount,sizeof(*PanelData),SortItems);
  }

  if (SearchMode!=SEARCH_SELECTED || RecurseLevel!=1)
  {
    for (int I=0;I<ItemCount && !StopSearch;I++)
    {
      while (PauseSearch)
        Sleep(10);

      PluginPanelItem *CurPanelItem=PanelData+I;
      char *CurName=CurPanelItem->FindData.cFileName;
      char FullName[2*NM];
      if (strcmp(CurName,".")==0 || strcmp(CurName,"..")==0)
        continue;
//      char AddPath[2*NM];
      if (Flags & OPIF_REALNAMES)
      {
        strncpy(FullName,CurName,sizeof(FullName)-1);
//        strcpy(AddPath,CurName);
//        *PointToName(AddPath)=0;
      }
      else
      {
        sprintf(FullName,"%s%s",PluginSearchPath,CurName);
//        strcpy(AddPath,PluginSearchPath);
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
        AddMenuRecord(FullName,&CurPanelItem->FindData);

      if (SearchInArchives && (hPlugin != INVALID_HANDLE_VALUE) && (Flags & OPIF_REALNAMES))
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
      {
        WaitForSingleObject(hPluginMutex,INFINITE);
        if (strchr(CurName,'\x1')==NULL && CtrlObject->Plugins.SetDirectory(hPlugin,CurName,OPM_FIND))
        {
          ReleaseMutex(hPluginMutex);
          strcat(PluginSearchPath,CurName);
          if (strlen(PluginSearchPath)<NM-2)
          {
            strcat(PluginSearchPath,"\x1");
            ScanPluginTree(hPlugin, Flags);
            *strrchr(PluginSearchPath,'\x1')=0;
          }
          char *NamePtr=strrchr(PluginSearchPath,'\x1');
          if (NamePtr!=NULL)
            *(NamePtr+1)=0;
          else
            *PluginSearchPath=0;
          WaitForSingleObject(hPluginMutex,INFINITE);
          if (!CtrlObject->Plugins.SetDirectory(hPlugin,"..",OPM_FIND))
            StopSearch=TRUE;
          ReleaseMutex(hPluginMutex);
          if (StopSearch) break;
        }
        else
          ReleaseMutex(hPluginMutex);
      }
    }
  }
  CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,ItemCount);
  RecurseLevel--;
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
    /* $ 11.12.2001 VVM
      - Не рисуем диалог при активном скрин-сэйвере */
    if (ListBox && !PauseSearch && !ScreenSaverActive)
    /* VVM $ */
    {
      WaitForSingleObject(hDialogMutex,INFINITE);

      if (BreakMainThread)
        StopSearch=TRUE;

      if (FindCountReady)
      {
        sprintf(DataStr," %s: %d ",MSG(MFindFound),FindFileCount+FindDirCount);
        ItemData.PtrData=DataStr;
        ItemData.PtrLength=strlen(DataStr);

        while (ListBox->GetCallCount())
          Sleep(10);
        Dialog::SendDlgMessage(hDlg,DM_SETTEXT,2,(long)&ItemData);
        FindCountReady=FALSE;
      }
      if (FindMessageReady)
      {
        char SearchStr[NM];
        if (*FindStr)
        {
          char Temp[NM],FStr[NM];
          strncpy(FStr,FindStr,sizeof(FStr)-1);
          sprintf(Temp," \"%s\"",TruncStrFromEnd(FStr,10));
          sprintf(SearchStr,MSG(MFindSearchingIn),Temp);
        }
        else
          sprintf(SearchStr,MSG(MFindSearchingIn),"");
        int Wid1=strlen(SearchStr);
        int Wid2=DlgWidth-strlen(SearchStr)-1;

        if (SearchDone)
        {
          while (ListBox->GetCallCount())
            Sleep(10);
          Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

          strncpy(DataStr,MSG(MFindCancel),sizeof(DataStr)-1);
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
      ReleaseMutex(hDialogMutex);
    }

    if (StopSearch && SearchDone && !FindMessageReady && !FindCountReady && !LastFoundNumber)
      break;
    Sleep(20);
  }
  WriteDataUsed=FALSE;
}

BOOL FindFiles::FindListGrow()
{
  DWORD Delta = (FindListCapacity < 256)?LIST_DELTA:FindListCapacity/2;
  LPFINDLIST NewList = (LPFINDLIST)realloc(FindList, (FindListCapacity + Delta) * sizeof(FINDLIST));
  if (NewList)
  {
    FindList = NewList;
    FindListCapacity+= Delta;
    return(TRUE);
  }
  return(FALSE);
}

BOOL FindFiles::ArcListGrow()
{
  DWORD Delta = (ArcListCapacity < 256)?LIST_DELTA:ArcListCapacity/2;
  LPARCLIST NewList = (LPARCLIST)realloc(ArcList, (ArcListCapacity + Delta) * sizeof(ARCLIST));
  if (NewList)
  {
    ArcList = NewList;
    ArcListCapacity+= Delta;
    return(TRUE);
  }
  return(FALSE);
}

DWORD FindFiles::AddFindListItem(WIN32_FIND_DATA *FindData)
{
  if ((FindListCount == FindListCapacity) &&
      (!FindListGrow()))
    return(LIST_INDEX_NONE);
  FindList[FindListCount].FindData = *FindData;
  FindList[FindListCount].ArcIndex = LIST_INDEX_NONE;
  return(FindListCount++);
}

DWORD FindFiles::AddArcListItem(char *ArcName, HANDLE hPlugin,
                                DWORD dwFlags, char *RootPath)
{
  if ((ArcListCount == ArcListCapacity) &&
      (!ArcListGrow()))
    return(LIST_INDEX_NONE);
  strncpy(ArcList[ArcListCount].ArcName, NullToEmpty(ArcName),
          sizeof(ArcList[ArcListCount].ArcName)-1);
  ArcList[ArcListCount].hPlugin = hPlugin;
  ArcList[ArcListCount].Flags = dwFlags;
  strncpy(ArcList[ArcListCount].RootPath, NullToEmpty(RootPath),
          sizeof(ArcList[ArcListCount].RootPath)-1);
  AddEndSlash(ArcList[ArcListCount].RootPath);
  return(ArcListCount++);
}

void FindFiles::ClearAllLists()
{
  if (FindList)
    free(FindList);
  if (ArcList)
    free(ArcList);
  FindList = NULL;
  FindListCapacity = FindListCount = 0;
  ArcList = NULL;
  ArcListCapacity = ArcListCount = 0;
  FindFileArcIndex = LIST_INDEX_NONE;
}
