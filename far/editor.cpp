#if !defined(EDITOR2)
/*
editor.cpp

Редактор

*/

/* Revision: 1.137 24.12.2001 $ */

/*
Modify:
  24.12.2001 SVS
    - Бага компиляции в Editor::ProcessEditorInput - лищняя скобка
  14.12.2001 IS
    ! stricmp -> LocalStricmp
  14.12.2001 IS
    ! внедрение const в соответствии с изменениями класса Edit
  10.12.2001 SVS
    - BugZ#164. При вставке имени файла (Ctrl-F) портилось оригинальное
      имя файла.
  03.12.2001 IS
    ! UndoData - теперь указатель. Размер буфера undo можно изменить
      через реестр.
  03.12.2001 VVM
    ! 1. Имеем 2 строки, втоная длиннее первой.
      2. Встаем на 2 строку впозиции после конца первой, но до второй.
      3. Shift-Up; Shift-End
      4. Можем выделять второй блок, не связанный с первым...
  09.11.2001 IS
    -  проклятое место - EditorF7Rules,
       опять фиксим, т.к. не соответствует заявленному в techinfo.
  05.11.2001
    SVS: ! ESPT_SETTABLE -> ESPT_CHARTABLE: все остальные ESPT_* тоже
           устанавливают, но SET  в  их  названии  нету.
    IS:  ! При неудачной смене таблицы по ESPT_SETTABLE оставим все как есть
           (раньше включали oem).
  01.11.2001 SVS
    ! ECTL_GETBOOKMARK, EditorBookMark -> ECTL_GETBOOKMARKS, EditorBookMarks
    ! С помощью IsBadWritePtr проверим на вшивость адресное пространство
  29.10.2001 IS
    ! SaveEditorPos и SaveEditorShortPos переехали в EditorOptions
      Теперь они локальны для каждой копии редактора, а первая опция может
      быть изменена плагином.
  28.10.2001 SVS
    ! Приведем к одному знаманателю реакцию на вставку путей (то же как и
      в панелях)
  24.10.2001 SVS
    ! обработка вставки имени файла и пути вынесена на уровень редактора, т.к.
      при выделеном блоке в несколько строк получаем лабуду.
    ! Корректно выставим последовательность отмены действий - DeleteBlock()
      сам умеет выставлять ундо
    - Аналогичная ситуация с Ctrl-F - при выделенном не персистентном блоке
      сам блок не удаляется.
  21.10.2001 SVS
    + CALLBACK-функция для избавления от BugZ#85
  19.10.2001 OT
    - Исправление ошибки HyperViewer
  16.10.2001 SKV
    - баг при выделении вертикальных блоков
      после переключения EdOpt.CursorBeyondEOL туда сюда.
  15.10.2001 SVS
    + _KEYMACRO()
  11.10.2001 IS
    + обработка DeleteOnClose
    ! внедрение const
  11.10.2001 VVM
    ! Полное имя файла вставляет CTRL+F (вместо CTRL+SHIFT+ENTER)
  10.10.2001 SVS
    + EditorInfo.CurState
  27.09.2001 IS
    - Левый размер при использовании strncpy
  24.09.2001 SKV
    - ctrl-left fix
  18.08.2001 SVS
    ! параметр у функции Paste - для отработки $Date, у которой есть '%n',
      соответственно изненен код обработки "клавиши" KEY_MACRODATE.
  14.09.2001 SVS
    ! Немного SysLog`ов
  13.09.2001 SKV
    - Shift-End на длииинной строке в блоке с изменённым цветом фона.
  30.08.2001 IS
    - Неопределенное поведение при использовании ECTL_SETPOSITION: режим
      Overtype не менялся, хотя и должен был в этой функции.
  07.08.2001 IS
    + Обработка ESPT_SETTABLE - смена кодировки.
  25.07.2001 IS
    - Баг: в меню по shift-f8 выделялась строка с той кодировкой, которая
      автоматически определилась при открытии файла, даже если текущая
      кодировка была другой. Решение: при открытии файла принудительно
      сбросим номер таблицы символов, если UseDecodeTable==FALSE.
  24.07.2001 IS
    ! Замена проверки на ' ' и '\t' на вызов isspace
  10.07.2001 SVS
    + Обработка KEY_MACROXLAT
  27.06.2001 SVS
    - Stream Block можно двигать в залоченном состоянии :-((
  25.06.2001 IS
    ! Внедрение const
  25.06.2001 SVS
    ! Юзаем SEARCHSTRINGBUFSIZE
  22.06.2001 SVS
    + обработка KEY_MACRODATE
  11.06.2001 SVS
    ! Новые параметры у GetSearchReplaceString() - указывающие размеры буферов
  10.06.2001 IS
    - Баг: зачем-то при продолжении _обратного_ поиска прокручивались на шаг
      _вперед_.
  08.06.2001 IS
    - Баги в некоторых местах из-за использования strcpy, а не strncpy, в
      результате чего гадили в память.
  06.06.2001 SVS
    + EDITOR_UNDO_COUNT - "от чисел к символам"
    + ECTL_GETBOOKMARK - получить инфу о закладках.
    ! Небольшая переделка с учетом InternalEditorBookMark.
  04.06.2001 IS
    - Editor::SaveFile - убраны (с подачи SVS) потенциальные баги - выход из
      функции был до того, как восстановятся атрибуты файла
  03.06.2001 OT
    - Не обновлялся StatusLine после DrawLine в редакторе
  31.05.2001 OT
    ! Исправления в SaveFile() Вместо цифр - поименованные константы типа SAVEFILE_XXX
  25.05.2001 IS
    - При вставке поточного блока из буфера обмена первый символ вставлялся в
      неверной кодировке. Это мой глюк, сорри :)
  24.05.2001 IS
    ! Опять правка последствий 592 (shift-home/end)
  19.05.2001 IS
    - Решение проблемы непрошеной конвертации табуляции (которая должна быть
      добавлена в начало строки при автоотступе) в пробелы при вставке
      чего-либо из буфера обмена.
  16.05.2001 DJ
    ! proof-of-concept
  15.05.2001 OT
    ! NWZ -> NFZ
  07.05.2001 SVS
    ! SysLog(); -> _D(SysLog());
    ! Search теперь возвращает TRUE/FALSE
  07.05.2001 IS
    - Баги: по shift-f7 продолжалась замена, хотя должен был быть поиск
      В Paste устранил потенциальный баг с delete []
  06.05.2001 DJ
    ! перетрях #include
  05.05.2001 IS
    ! shift-home/end - приблизим поведение к тому, какое было до 592
  04.05.2001 OT
    + Неверно формировалось меню плагинов по F11 (NWZ)
      Изменился PluginSet::CommandsMenu()
  27.04.2001 SVS
    - Ctrl-Q: не выставлялся признак модификации
    - Ctrl-Q: не помещались данные в буфер отката
    - XLat: не помещались данные в буфер отката
  27.04.2001 VVM
    + Обработка KEY_MSWHEEL_XXXX
  25.04.2001 SVS
    + KEY_MEDIT_ISSELECTED, в ответ на которую Editor::ProcessKey возвращает
      TRUE - если есть помеченный блок или FALSE - блока нету.
  25.04.2001 IS
    + ctrl-shift-enter - вставить в строку полное имя редактируемого файла
  20.04.2001 IS
    ! _Значительное_ ускорение выделения строки при помощи shift-home или
      shift-end. Просьба сразу сообщать на малейшее отклонение от манеры
      аналогичного поведения до этого патча, я вроде поэкспериментировал, но
      мог что-нибудь и упустить.
  11.04.2001 SVS
    + Добавлена обработка Ctrl-Q - при вставки символа не удалялся блок
      (постоянные блоки выключены)
  10.04.2001 SVS
    - Если файл RO и мы отказались его открывать - все равно плагину
      посылался эвент, что мол файл закрыт! И это при том, что плагин не
      получал эвен про удачное чтение - галиматья, блин :-((((
    ! Избавляемся от варнингов.
    - ^P/^M - некорректно работали: уловие для CurPos должно быть ">=",
      а не "меньше".
    - Забыли Pasting выставить :-( - для Shift-Ins из-за чего тот же колорер
      неверно отрабатывал.
  03.04.2001 IS
    + Обработка ESPT_AUTOINDENT, ESPT_CURSORBEYONDEOL, ESPT_CHARCODEBASE
      (это в ECTL_SETPARAM)
  28.03.2001 SVS
    + дополнительный параметр для SaveFile() - SaveAs
  28.03.2001 VVM
    ! При переходе на строку сдвигалась позиция на символ вправо.
  26.03.2001 SVS
    + При вызове списка плагинов говорим манагеру о том, чтобы он искал
      предопределенный топик "Editor" для Shift-F1 из списка плагинов
  20.03.2001 SVS
    + При выводе сообщения о размере файла сообщается его размер и
      минимально допустимый размер редактирования.
  15.03.2001 OT
    - Если нажать в редакторе <Enter> в конце файла, а потом сделать UNDO...
  12.03.2001 SVS
    ! Коррекция в связи с изменениями в классе int64
  27.02.2001 SVS
    + В статусной строке показываем код символа в зависимости от базы -
      Oct, Dec или Hex
  27.02.2001 IS
    + Проверка нового размера табуляции на допустимые значения в SetTabSize
  26.02.2001 IS
    ! Часть самостоятельных переменных заменено соответствующими из
      EditorOptions. Надо было это сразу сделать, да я что-то стормозил :)
    + SetCursorBeyondEOL
  21.02.2001 IS
    ! Opt.TabSize -> Opt.EdOpt.TabSize
      Opt.EditorPersistentBlocks -> Opt.EdOpt.PersistentBlocks
      Opt.EditorDelRemovesBlocks -> Opt.EdOpt.DelRemovesBlocks
      Opt.EditorExpandTabs -> Opt.EdOpt.ExpandTabs
      Opt.EditorAutoIndent -> Opt.EdOpt.AutoIndent
      Opt.EditorAutoDetectTable -> Opt.EdOpt.AutoDetectTable
      Opt.EditorCursorBeyondEOL -> Opt.EdOpt.CursorBeyondEOL
      Opt.EditorBSLikeDel -> Opt.EdOpt.BSLikeDel
  19.02.2001 IS
    - Забыл проинициализировать AttrStr
  16.02.2001 IS
    + Обработка ECTL_SETPARAM, пока только ESPT_TABSIZE и ESPT_EXPANDTABS
  15.02.2001 IS
    ! Opt.EditorDelRemovesBlocks -> DelRemovesBlocks
      Opt.EditorPersistentBlocks -> PersistentBlocks
    + SetDelRemovesBlocks, SetPersistentBlocks
  15.02.2001 IS
    + Обновим размер табуляции для всех Edit в функции SetTabSize
    ! Opt.EditorExpandTabs -> ConvertTabs
    + SetConvertTabs
    ! SetTabSize вызывается везде перед SetConvertTabs
  14.02.2001 IS
    + Размер табуляции хранится в TabSize, манипулировать им можно при помощи
      GetTabSize, SetTabSize
  14.02.2000 SVS
    ! Динамический размер под количество строк
  14.02.2001 VVM
    + При отсутствии блока AltU/AltI сдвигают текущую строчку
  13.02.2001 IS
    ! Строчка с атрибутами подготавливается при их считывании, а не при каждом
      обновлении экрана. Т.с., оптимизация кода после себя самого ;)
  12.02.2001 IS
    ! Атрибуты считываются только в двух местах, а не при каждом обновлении
      статуса
  08.02.2001 skv
    - EEREDRAW при Ctrl-P, Ctrl-M
  03.02.2001 skv
    - EEREDRAW_ALL теперь соответствует действительности,
      при переходе с одной строки на другую при выключенном
      cursor beyond end of line нажатием вправо.
  21.01.2001 SVS
    ! Диалоги поиска/замены выведен из Editor::Search в отдельную
      функцию GetSearchReplaceString (файл stddlg.cpp)
  14.01.2001 tran
    - убран баг, когда при вызове редактора из плагина
      показывался мусор
  07.01.2001 IS
    - не правильно работала проверка на ошибку при открытии файлов,
      превышающих определенный размер (не всегда срабатывала)
  28.12.2000 VVM
    + Щелчок мышью снимает непостоянный блок всегда.
  23.12.2000 OT
    - Медленно делался CtrlShiftLeft и CtrlAltLeft на ооооочеееень
      длинных словах
  22.12.2000 SVS
    - Вызов из EE_READ команды ECTL_SETKEYBAR приводил к падению ФАРа, т.к.
      объект EditKeyBar еще не существует.
  21.12.2000 SVS
    - В предыдущем исправлении было задано неверное условие для
      правила EditorF7Rules
  16.12.2000 OT
    - CtrlY на последней строке с выделенным вертикальным блоком не снимал
      выделение
  15.12.2000 SVS
    ! Уточнение по поводу того, что вернула GetFileAttributes()
  10.12.2000 IS
    ! Обрабатываем при Xlat только то слово, на котором стоит курсор, или то
      слово, что находится левее позиции курсора на 1 символ
  08.12.2000 SLV
    - Ctrl-Del в начале строки при выделенном блоке и
      включенном Opt.EditorDelRemovesBlocks.
  03.12.2000 IS
    + Показывать в статусной строке буквами RSH соответствующие атрибуты файла,
      если они установлены.
  03.12.2000 SVS
    + "Если файл имеет атрибут ReadOnly..." здесь System и Hidden - задаются
      отдельно.
  29.11.2000 SVS
    + Если файл имеет атрибут ReadOnly или System или Hidden,
      то сразу лочим файл - естественно отключаемо.
    + Opt.EditorFileSizeLimit - минимально допустимый размер файла, после
      которого будет выдан диалог о целесообразности открытия подобного
      файла на редактирование
  28.11.2000 SVS
    + Opt.EditorF7Rules - Правило на счет поиска в редакторе
      "О, это не ощибка - это свойство моей программы" :-)
      Новое поведение стало подконтрольным.
  25.11.2000 IS
    + Если нет выделения, то обработаем текущее слово. Слово определяется на
      основе специальной группы разделителей.
  04.11.2000 SVS
    + Проверка на альтернативную клавишу при XLat-перекодировке
  03.11.2000 OT
    ! Введение проверки возвращаемого значения
  02.11.2000 OT
    ! Введение проверки на длину буфера, отведенного под имя файла.
  23.10.2000 tran 1.40
    ! ВБ, табуляция и CurBeyondEOL
  16.10.2000 tran 1.39
    ! первый поиск идет с текущей позиции, а следующий - со следующей (FGWL#10)
  11.10.2000 SVS
    ! Bs удаляет блок так же, как и Del
    - "Редактировали, залочили, при выходе - потеряли файл :-("
  01.10.2000 IS
    ! Показывать букву диска в статусной строке
  24.09.2000 SVS
    + Работа по сохранению/восстановлению позиций в файле по RCtrl+<N>
    + Перекодировка Xlat
  20.09.2000 SVS
    - Bugs с "наездом" заголовка (от плагина) на всё прочеЯ!
  20.09.2000 SVS
    ! В Replace диалоге для строки replace удален флаг DIF_USELASTHISTORY
    ! Если при замене жмакнули All, то при повторном Shift-F7 снова
      появляется диалог о подтверждении действий.
  13.09.2000 skv
    ! EE_REDRAW вызывается с константами. 1 и 2 поменяны.
  07.09.2000 skv
    + ECTL_PROCESSKEY
  07.09.2000 skv
    - пофиксан быстрый replace при установленной перекодировке
  30.08.2000 tran 1.21
    - bug в автоотступе, внесенный патчем 66
  15.08.2000 skv
    ! Оптимизация Replace.
  10.08.2000 skv
    ! Оптимизация работы EE_REDRAW события редактора.
  07.08.2000 SVS
    + ECTL_SETKEYBAR - Функция установки Keybar Labels
  03.08.2000 KM 1.17
    ! В функцию Search добавлена возможность поиска целых слов.
  03.08.2000 SVS 1.16
    ! WordDiv -> Opt.WordDiv
  01.08.2000 tran 1.15
    + DIF_USELASTHISTORY в диалогах поиска,замены и перехода
  25.07.2000 tran 1.14
    - Bug 22 (остатки)
      подправлены обработки alt-left,alt-right
      на предмет перебега за границу блока
  21.07.2000 tran 1.13
    - Bug 22
      вот теперь это верно решение.
      предыдущие просба считать неверным.
      оно все равно переделано
      ввел три новых метода
        int  GetLineCurPos(); - просто сокращает писанину
        void BeginVBlockMarking(); - начинает вертикальный блок
        void AdjustVBlock(int PrevX); - подравнивает вертикальный блок
                при перебросках курсора при переходе через
                пустое место табуляций
      просьба оставить закоментаренный SysLog
      если что-то в этой части случится, я быстрее разберусь
      если нет - потом сам уберу...
  21.07.2000 tran
    ! Все внутри функции GoToPosition();
  18.07.2000 tran
    - Bug #22
      встань в начало текста, нажми alt-right, alt-pagedown,
      выделится блок шириной в 1 колонку, нажми еще alt-right
      выделение сбросится
  17.07.2000 tran
    - баг с автоотступом при [ ] Expand tabs to spaces
      и когда ентер жался сразу после символа '\t'
      ранее в новую строку вставлялись пробелы (надо \t)
      и символ табуляции на предыдущей строке стирался
      теперь он не стирается и на новой вместо пробелов
      все копируется из старой
      новые {} кое-где, побочный эффект вставки печати отладки,
      пусть их лежат... :)
  17.07.2000 OT
    + Застолбить место под разработку "моего" редактора
  14.07.2000 tran
    + переход на проценты
      вводим 50%, попадаем прямо в середину
      функция GetRowCol стала методом класса
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  11.07.2000 tran
    + строка статуса рисуется с учетом ширины консоли.
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  07.07.2000 SVS
    + Разграничитель слов WordDiv находится теперь в global.cpp и
      берется из реестра (общий для редактирования)
  07.07.2000 tran & SVS
    + in AltF8 - row,col support
  29.06.2000 IG
    + CtrlAltLeft, CtrlAltRight для вертикальный блоков
  28.06.2000 tran
    - trap при размере вертикального блока более 1000 колонок
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "editor.hpp"
#include "edit.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "poscache.hpp"
#include "chgprior.hpp"
#include "filestr.hpp"
#include "help.hpp"
#include "dialog.hpp"
#include "fileedit.hpp"
#include "savescr.hpp"
#include "scrbuf.hpp"

struct EditList
{
  struct EditList *Prev;
  struct EditList *Next;
  Edit EditLine;
};

static struct CharTableSet InitTableSet;
static int InitUseDecodeTable=FALSE,InitTableNum=0,InitAnsiText=FALSE;

static int ReplaceMode,ReplaceAll;

static int EditorID=0;

// struct EditorUndoData
enum {UNDO_NONE=0,UNDO_EDIT,UNDO_INSSTR,UNDO_DELSTR};

Editor::Editor()
{
  DeleteOnClose=FALSE; // ну мы же не самоубийцы, правда? ;-)
  /* $ 19.02.2001 IS
       Я не учел, что для нового файла GetFileAttributes не вызывается...
  */
  *AttrStr=0;
  /* IS $ */
  /* $ 26.02.2001 IS
       Инициализируем переменные одним махом ;)
  */
  memcpy(&EdOpt, &Opt.EdOpt, sizeof(EditorOptions));
  /* IS $ */

  EditKeyBar=NULL;
  strncpy((char *)LastSearchStr,GlobalSearchString,sizeof(LastSearchStr)-1);
  LastSearchCase=GlobalSearchCase;
  /* $ 03.08.2000 KM
     Переменная для поиска "Whole words"
  */
  LastSearchWholeWords=GlobalSearchWholeWords;
  /* KM $ */
  LastSearchReverse=GlobalSearchReverse;
  memcpy(&TableSet,&InitTableSet,sizeof(TableSet));
  UseDecodeTable=InitUseDecodeTable;
  TableNum=InitTableNum;
  AnsiText=InitAnsiText;

  DisableOut=0;
  Pasting=0;
  Modified=0;
  /*$ 10.08.2000 skv
    Initialization
  */
  JustModified=0;
  /* skv$*/
  WasChanged=0;
  NumLine=0;
  NumLastLine=1;
  Overtype=0;
  DisableUndo=0;
  LastChangeStrPos=0;
  *FileName=0;
  MarkingBlock=FALSE;
  MarkingVBlock=FALSE;
  BlockStart=NULL;
  BlockStartLine=0;
  TopList=EndList=TopScreen=CurLine=new struct EditList;
  TopList->EditLine.SetObjectColor(COL_EDITORTEXT,COL_EDITORSELECTEDTEXT);
  /* $ 14.02.2001 IS
       Установим нужный размер табуляции
  */
  TopList->EditLine.SetTabSize(EdOpt.TabSize);
  /* IS $ */
  TopList->EditLine.SetConvertTabs(EdOpt.ExpandTabs);
  TopList->EditLine.SetEditorMode(TRUE);
  TopList->Prev=NULL;
  TopList->Next=NULL;
  /* $ 03.12.2001 IS размер буфера undo теперь может меняться */
  UndoData=static_cast<EditorUndoData*>
    (malloc(Opt.EditorUndoSize*sizeof(EditorUndoData)));
  if(UndoData)
    memset(UndoData,0,Opt.EditorUndoSize*sizeof(EditorUndoData));
  /* IS $ */
  UndoDataPos=0;
  StartLine=StartChar=-1;
  *GlobalEOL=0;
  *Title=0;
  LockMode=FALSE;
  CurrentEditor=this;
  BlockUndo=FALSE;
  *PluginData=0;
  NewUndo=FALSE;
  VBlockStart=NULL;
  memset(&SavePos,0xff,sizeof(SavePos));
  MaxRightPos=0;
  TableChangedByUser=FALSE;
  *PluginTitle=0;
  UndoOverflow=FALSE;
  UndoSavePos=0;
  Editor::EditorID=::EditorID++;
  OpenFailed=true; // Ну, блин. Файл то еще не открыт,
                   // так нефига ставить признак удачного открытия
  HostFileEditor=NULL;
}


Editor::~Editor()
{
  _OT(SysLog("[%p] Editor::~Editor()",this));
  if (EdOpt.SavePos && CtrlObject!=NULL)
  {
    int ScreenLinePos=CalcDistance(TopScreen,CurLine,-1);
    int CurPos=CurLine->EditLine.GetTabCurPos();
    int LeftPos=CurLine->EditLine.GetLeftPos();
    char CacheName[NM*3];
    if (*PluginData)
      sprintf(CacheName,"%s%s",PluginData,PointToName(FileName));
    else
      strcpy(CacheName,FileName);
    unsigned int Table=0;
    if (TableChangedByUser)
    {
      Table=1;
      if (AnsiText)
        Table=2;
      else
        if (UseDecodeTable)
          Table=TableNum+2;
    }

    if (!OpenFailed) // здесь БЯКА в кеш попадала :-(
      CtrlObject->EditorPosCache->AddPosition(CacheName,NumLine,ScreenLinePos,CurPos,LeftPos,Table,
               (EdOpt.SaveShortPos?SavePos.Line:NULL),
               (EdOpt.SaveShortPos?SavePos.Cursor:NULL),
               (EdOpt.SaveShortPos?SavePos.ScreenLine:NULL),
               (EdOpt.SaveShortPos?SavePos.LeftPos:NULL));
  }

  while (EndList!=NULL)
  {
    struct EditList *Prev=EndList->Prev;
    delete EndList;
    EndList=Prev;
  }
  /* $ 03.12.2001 IS
     UndoData - указатель
  */
  if(UndoData)
  {
    for (int I=0;I<Opt.EditorUndoSize;++I)
      if (UndoData[I].Type!=UNDO_NONE && UndoData[I].Str!=NULL)
        delete UndoData[I].Str;
    free(UndoData);
  }
  /* IS $ */
  KeepInitParameters();

  if (!OpenFailed)
  {
    Editor *save = CtrlObject->Plugins.CurEditor;
    CtrlObject->Plugins.CurEditor=this;
//_D(SysLog("%08d EE_CLOSE",__LINE__));
    CtrlObject->Plugins.ProcessEditorEvent(EE_CLOSE,&EditorID);
    /* $ 11.10.2001 IS
       Удалим файл вместе с каталогом, если это просится и файла с таким же
       именем не открыто в других фреймах.
    */
    if (DeleteOnClose && !FrameManager->CountFramesWithName(FileName))
       DeleteFileWithFolder(FileName);
   /* IS $ */
    CtrlObject->Plugins.CurEditor = save;
  }

  CurrentEditor=NULL;
}


void Editor::KeepInitParameters()
{
  strcpy(GlobalSearchString,(char *)LastSearchStr);
  GlobalSearchCase=LastSearchCase;
  /* $ 03.08.2000 KM
    Новая переменная для поиска "Whole words"
  */
  GlobalSearchWholeWords=LastSearchWholeWords;
  /* KM $ */
  GlobalSearchReverse=LastSearchReverse;
  memcpy(&InitTableSet,&TableSet,sizeof(InitTableSet));
  InitUseDecodeTable=UseDecodeTable;
  InitTableNum=TableNum;
  InitAnsiText=AnsiText;
}


int Editor::ReadFile(const char *Name,int &UserBreak)
{
  FILE *EditFile;
  struct EditList *PrevPtr;
  int Count=0,LastLineCR=0,MessageShown=FALSE;
  UserBreak=0;
  OpenFailed=false;
//  ConvertNameToFull(Name,FileName,sizeof(FileName));
  if (ConvertNameToFull(Name,FileName, sizeof(FileName)) >= sizeof(FileName)){
    OpenFailed=true;
    return FALSE;
  }

  DWORD Flags=FILE_FLAG_SEQUENTIAL_SCAN;
  if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
    Flags|=FILE_FLAG_POSIX_SEMANTICS;

  HANDLE hEdit=CreateFile(Name,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,Flags,NULL);

  if (hEdit==INVALID_HANDLE_VALUE && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
    hEdit=CreateFile(Name,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);

  if (hEdit==INVALID_HANDLE_VALUE)
  {
    int LastError=GetLastError();
    if (LastError!=ERROR_FILE_NOT_FOUND && LastError!=ERROR_PATH_NOT_FOUND)
    {
      UserBreak=-1;
      OpenFailed=true;
    }
    return(FALSE);
  }

  int EditHandle=_open_osfhandle((long)hEdit,O_BINARY);
  if (EditHandle==-1)
    return(FALSE);
  if ((EditFile=fdopen(EditHandle,"rb"))==NULL)
    return(FALSE);
  if (GetFileType(hEdit)!=FILE_TYPE_DISK)
  {
    fclose(EditFile);
    SetLastError(ERROR_INVALID_NAME);
    UserBreak=-1;
    OpenFailed=true;
    return(FALSE);
  }

  /* $ 29.11.2000 SVS
   + Проверка на минимально допустимый размер файла, после
     которого будет выдан диалог о целесообразности открытия подобного
     файла на редактирование
  */
  if(Opt.EditorFileSizeLimitLo || Opt.EditorFileSizeLimitHi)
  {
    int64 RealSizeFile;
    /* $ 07.01.2001 IS
        - без этого не правильно работала проверка на ошибку
    */
    SetLastError(NO_ERROR);
    /* IS $ */
    RealSizeFile.PLow()=GetFileSize(hEdit,&RealSizeFile.PHigh());
    if (GetLastError() == NO_ERROR)
    {
      int64 NeedSizeFile(Opt.EditorFileSizeLimitHi,Opt.EditorFileSizeLimitLo);
      if(RealSizeFile > NeedSizeFile)
      {
        char TempBuf[2][128];
        char TempBuf2[2][64];
        // Ширина = 8 - это будет... в Kb и выше...
        FileSizeToStr(TempBuf2[0],RealSizeFile.PHigh(),RealSizeFile.PLow(),8);
        FileSizeToStr(TempBuf2[1],NeedSizeFile.PHigh(),NeedSizeFile.PLow(),8);
        sprintf(TempBuf[0],MSG(MEditFileLong),RemoveExternalSpaces(TempBuf2[0]));
        sprintf(TempBuf[1],MSG(MEditFileLong2),RemoveExternalSpaces(TempBuf2[1]));
        if(Message(MSG_WARNING,2,MSG(MEditTitle),
                    Name,
                    TempBuf[0],
                    TempBuf[1],
                    MSG(MEditROOpen),
                    MSG(MYes),MSG(MNo)))
        {
          fclose(EditFile);
          SetLastError(ERROR_OPEN_FAILED);
          UserBreak=1;
          OpenFailed=true;
          return(FALSE);
        }
      }
    }
  }
  /* SVS $ */
  /* $ 29.11.2000 SVS
     Если файл имеет атрибут ReadOnly или System или Hidden,
     то сразу лочим файл - естественно отключаемо.
  */
  /* $ 03.12.2000 SVS
     System или Hidden - задаются отдельно
  */
  /* $ 15.12.2000 SVS
     Разумнее сначала проверить то, что вернула GetFileAttributes() :-)
  */
  {
    /* $ 12.02.2001 IS
         Запомним атрибуты
    */
    FileAttributes=GetFileAttributes(Name);
    if((Opt.EditorReadOnlyLock&1) &&
       FileAttributes != -1 &&
       (FileAttributes &
          (FILE_ATTRIBUTE_READONLY|
             /* Hidden=0x2 System=0x4 - располагаются во 2-м полубайте,
                поэтому применяем маску 0110.0000 и
                сдвигаем на свое место => 0000.0110 и получаем
                те самые нужные атрибуты  */
             ((Opt.EditorReadOnlyLock&0x60)>>4)
          )
       )
     )
    /* IS $ */
      LockMode=!LockMode;
  }
  /* SVS 15.12.2000 $ */
  /* SVS 03.12.2000 $ */
  /* SVS $ */

  {
    ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
    //SaveScreen SaveScr;
    NumLastLine=0;

    GetFileString GetStr(EditFile);
    char *Str;
    int StrLength,GetCode;

    clock_t StartTime=clock();

    if (EdOpt.AutoDetectTable)
    {
      UseDecodeTable=DetectTable(EditFile,&TableSet,TableNum);
      AnsiText=FALSE;
    }

    while ((GetCode=GetStr.GetString(&Str,StrLength))!=0)
    {
      if (GetCode==-1)
      {
        fclose(EditFile);
        SetPreRedrawFunc(NULL);
        return(FALSE);
      }
      LastLineCR=0;
      if ((++Count & 0xfff)==0 && clock()-StartTime>500)
      {
        if (CheckForEsc())
        {
          UserBreak=1;
          fclose(EditFile);
          SetPreRedrawFunc(NULL);
          return(FALSE);
        }
        if (!MessageShown)
        {
          SetCursorType(FALSE,0);
          SetPreRedrawFunc(Editor::PR_EditorShowMsg);
          EditorShowMsg(MSG(MEditTitle),MSG(MEditReading),Name);
          MessageShown=TRUE;
        }
      }

      char *CurEOL;
      if (!LastLineCR && ((CurEOL=(char *)memchr(Str,'\r',StrLength))!=NULL ||
          (CurEOL=(char *)memchr(Str,'\n',StrLength))!=NULL))
      {
        strncpy(GlobalEOL,CurEOL,sizeof(GlobalEOL)-1);
        GlobalEOL[sizeof(GlobalEOL)-1]=0;
        LastLineCR=1;
      }

      if (NumLastLine!=0)
      {
        EndList->Next=new struct EditList;
        if (EndList->Next==NULL)
        {
          fclose(EditFile);
          SetPreRedrawFunc(NULL);
          return(FALSE);
        }
        PrevPtr=EndList;
        EndList=EndList->Next;
        EndList->Prev=PrevPtr;
        EndList->Next=NULL;
      }

      /* $ 14.02.2001 IS
           Установим нужный размер табуляции
      */
      EndList->EditLine.SetTabSize(EdOpt.TabSize);
      /* IS $ */
      EndList->EditLine.SetConvertTabs(EdOpt.ExpandTabs);
      EndList->EditLine.SetBinaryString(Str,StrLength);
      EndList->EditLine.SetCurPos(0);
      EndList->EditLine.SetObjectColor(COL_EDITORTEXT,COL_EDITORSELECTEDTEXT);
      EndList->EditLine.SetEditorMode(TRUE);

      NumLastLine++;
    }
    SetPreRedrawFunc(NULL);
    if (LastLineCR)
      if ((EndList->Next=new struct EditList)!=NULL)
      {
        PrevPtr=EndList;
        EndList=EndList->Next;
        EndList->Prev=PrevPtr;
        EndList->Next=NULL;
        /* $ 14.02.2001 IS
           Установим нужный размер табуляции
        */
        EndList->EditLine.SetTabSize(EdOpt.TabSize);
        /* IS $ */
        EndList->EditLine.SetConvertTabs(EdOpt.ExpandTabs);
        EndList->EditLine.SetString("");
        EndList->EditLine.SetCurPos(0);
        EndList->EditLine.SetObjectColor(COL_EDITORTEXT,COL_EDITORSELECTEDTEXT);
        EndList->EditLine.SetEditorMode(TRUE);
        NumLastLine++;
      }
  }
  if (NumLine>0)
    NumLastLine--;
  if (NumLastLine==0)
    NumLastLine=1;
  fclose(EditFile);
  if (StartLine==-2)
  {
    struct EditList *CurPtr=TopList;
    long TotalSize=0;
    while (CurPtr!=NULL && CurPtr->Next!=NULL)
    {
      char *SaveStr,*EndSeq;
      int Length;
      CurPtr->EditLine.GetBinaryString(&SaveStr,&EndSeq,Length);
      TotalSize+=Length+strlen(EndSeq);
      if (TotalSize>StartChar)
        break;
      CurPtr=CurPtr->Next;
      NumLine++;
    }
    TopScreen=CurLine=CurPtr;
    if (EdOpt.SavePos && CtrlObject!=NULL)
    {
      /* $ 14.01.2001 tran
         LeftPos надо было инициализировать... */
      unsigned int Line,ScreenLine,LinePos,LeftPos=0;
      char CacheName[NM*3];
      if (*PluginData)
        sprintf(CacheName,"%s%s",PluginData,PointToName(FileName));
      else
        strcpy(CacheName,FileName);
      unsigned int Table;
      CtrlObject->EditorPosCache->GetPosition(CacheName,Line,ScreenLine,LinePos,LeftPos,Table,
               (EdOpt.SaveShortPos?SavePos.Line:NULL),
               (EdOpt.SaveShortPos?SavePos.Cursor:NULL),
               (EdOpt.SaveShortPos?SavePos.ScreenLine:NULL),
               (EdOpt.SaveShortPos?SavePos.LeftPos:NULL));
      //_D(SysLog("after Get cache, LeftPos=%i",LeftPos));
      TableChangedByUser=(Table!=0);
      switch(Table)
      {
        case 0:
          break;
        case 1:
          AnsiText=UseDecodeTable=0;
          break;
        case 2:
          {
            AnsiText=TRUE;
            UseDecodeTable=TRUE;
            TableNum=0;
            int UseUnicode=FALSE;
            GetTable(&TableSet,TRUE,TableNum,UseUnicode);
          }
          break;
        default:
          AnsiText=0;
          UseDecodeTable=1;
          TableNum=Table-2;
          PrepareTable(&TableSet,Table-3);
          break;
      }
      if (NumLine==Line-ScreenLine)
      {
        DisableOut++;
        for (int I=0;I<ScreenLine;I++)
          ProcessKey(KEY_DOWN);
        CurLine->EditLine.SetTabCurPos(LinePos);
        DisableOut--;
      }
      //_D(SysLog("Setleftpos to %i",LeftPos));
      CurLine->EditLine.SetLeftPos(LeftPos);
    }
  }
  else
    if (StartLine!=-1 || EdOpt.SavePos && CtrlObject!=NULL)
    {
      /* $ 14.01.2001 tran
         LeftPos надо было инициализировать... */
      unsigned int Line,ScreenLine,LinePos,LeftPos=0;
      if (StartLine!=-1)
      {
        Line=StartLine-1;
        ScreenLine=ScrY/2;
        if (ScreenLine>Line)
          ScreenLine=Line;
        LinePos=(StartChar>0) ? StartChar-1:0;
      }
      else
      {
        char CacheName[NM*3];
        if (*PluginData)
          sprintf(CacheName,"%s%s",PluginData,PointToName(FileName));
        else
          strcpy(CacheName,FileName);
        unsigned int Table;
        CtrlObject->EditorPosCache->GetPosition(CacheName,Line,ScreenLine,LinePos,LeftPos,Table,
               (EdOpt.SaveShortPos?SavePos.Line:NULL),
               (EdOpt.SaveShortPos?SavePos.Cursor:NULL),
               (EdOpt.SaveShortPos?SavePos.ScreenLine:NULL),
               (EdOpt.SaveShortPos?SavePos.LeftPos:NULL));
        //_D(SysLog("after Get cache 2, LeftPos=%i",LeftPos));
        TableChangedByUser=(Table!=0);
        switch(Table)
        {
          case 0:
            break;
          case 1:
            AnsiText=UseDecodeTable=0;
            break;
          case 2:
            {
              AnsiText=TRUE;
              UseDecodeTable=TRUE;
              TableNum=0;
              int UseUnicode=FALSE;
              GetTable(&TableSet,TRUE,TableNum,UseUnicode);
            }
            break;
          default:
            AnsiText=0;
            UseDecodeTable=1;
            TableNum=Table-2;
            PrepareTable(&TableSet,Table-3);
            break;
        }
      }
      if (ScreenLine>ScrY)
        ScreenLine=ScrY;
      if (Line>=ScreenLine)
      {
        DisableOut++;
        GoToLine(Line-ScreenLine);
        TopScreen=CurLine;
        for (int I=0;I<ScreenLine;I++)
          ProcessKey(KEY_DOWN);
        CurLine->EditLine.SetTabCurPos(LinePos);
        //_D(SysLog("Setleftpos 2 to %i",LeftPos));
        CurLine->EditLine.SetLeftPos(LeftPos);
        DisableOut--;
      }
    }
  if (UseDecodeTable)
    for (struct EditList *CurPtr=TopList;CurPtr!=NULL;CurPtr=CurPtr->Next)
      CurPtr->EditLine.SetTables(&TableSet);
  /* $ 25.07.2001 IS
       Принудительно сбросим номер таблицы символов, т.к. никаких таблиц
       символов не используется (UseDecodeTable==FALSE)
  */
  else
    TableNum=0;
  /* IS $ */

  CtrlObject->Plugins.CurEditor=this;
//_D(SysLog("%08d EE_READ",__LINE__));
  CtrlObject->Plugins.ProcessEditorEvent(EE_READ,NULL);
  return(TRUE);
}

/* $ 12.02.2001 IS
     Заменил локальную FileAttr на FileAttributes
*/
/* $ 04.06.2001 IS
     Убраны (с подачи SVS) потенциальные баги - выход из функции был до того,
     как восстановятся атрибуты файла
*/
int Editor::SaveFile(const char *Name,int Ask,int TextFormat,int SaveAs)
{
  /* $ 11.10.2000 SVS
     Редактировали, залочили, при выходе - потеряли файл :-(
  */
  if (LockMode && !Modified && !SaveAs)
    return(SAVEFILE_SUCCESS);
  /* SVS $ */

  FILE *EditFile;
  struct EditList *CurPtr;
  int AskSave;
  int NewFile=TRUE;
  int RetCode=SAVEFILE_SUCCESS;

  if (TextFormat!=0)
    WasChanged=TRUE;
  switch(TextFormat)
  {
    case 1:
      strcpy(GlobalEOL,"\r\n");
      break;
    case 2:
      strcpy(GlobalEOL,"\n");
      break;
  }

  {
    //SaveScreen SaveScr;
    if (Ask)
    {
      if (!Modified)
        return(SAVEFILE_SUCCESS);
      AskSave=Message(MSG_WARNING,3,MSG(MEditTitle),MSG(MEditAskSave),
                      MSG(MEditSave),MSG(MEditNotSave),MSG(MEditContinue));

      switch (AskSave)
      {
        case -1:
        case -2:
        case 2:
          return(SAVEFILE_CANCEL);
        case 0:
          break;
        case 1:
          /*$ 10.08.2000 skv
            TextChanged() support;
          */
          TextChanged(0);
          /* skv $*/
          return(SAVEFILE_SUCCESS);
      }
    }

    if ((FileAttributes=GetFileAttributes(Name))!=-1)
    {
      NewFile=FALSE;
      if (FileAttributes & FA_RDONLY)
      {
        int AskOverwrite;
        AskOverwrite=Message(MSG_WARNING,2,MSG(MEditTitle),Name,MSG(MEditRO),
                             MSG(MEditOvr),MSG(MYes),MSG(MNo));
        if (AskOverwrite!=0)
          return(SAVEFILE_CANCEL);
        SetFileAttributes(Name,FileAttributes & ~FA_RDONLY); // сняты атрибуты
      }                                                      // после этих строк
      if (FileAttributes & (FA_HIDDEN|FA_SYSTEM))            // return из
        SetFileAttributes(Name,0);                           // середины функции
                                                             // недопустим
    }

    /* $ 11.10.2001 IS
       Если было произведено сохранение с любым результатом, то не удалять файл
    */
    DeleteOnClose=FALSE;
    /* IS $ */
    CtrlObject->Plugins.CurEditor=this;
//_D(SysLog("%08d EE_SAVE",__LINE__));
    CtrlObject->Plugins.ProcessEditorEvent(EE_SAVE,NULL);

    DWORD Flags=FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN;
    if (FileAttributes!=-1 && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
      Flags|=FILE_FLAG_POSIX_SEMANTICS;

    HANDLE hEdit=CreateFile(Name,GENERIC_WRITE,FILE_SHARE_READ,NULL,
                 FileAttributes!=-1 ? TRUNCATE_EXISTING:CREATE_ALWAYS,Flags,NULL);
    if (hEdit==INVALID_HANDLE_VALUE && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT && FileAttributes!=-1)
      hEdit=CreateFile(Name,GENERIC_WRITE,FILE_SHARE_READ,NULL,TRUNCATE_EXISTING,
                       FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    if (hEdit==INVALID_HANDLE_VALUE)
    {
      RetCode=SAVEFILE_ERROR;
      goto end;
    }
    int EditHandle=_open_osfhandle((long)hEdit,O_BINARY);
    if (EditHandle==-1)
    {
      RetCode=SAVEFILE_ERROR;
      goto end;
    }
    if ((EditFile=fdopen(EditHandle,"wb"))==NULL)
    {
      RetCode=SAVEFILE_ERROR;
      goto end;
    }

    UndoSavePos=UndoDataPos;
    UndoOverflow=FALSE;

//    ConvertNameToFull(Name,FileName, sizeof(FileName));
    if (ConvertNameToFull(Name,FileName, sizeof(FileName)) >= sizeof(FileName)){
      OpenFailed=true;
      RetCode=SAVEFILE_ERROR;
      goto end;
    }
    SetCursorType(FALSE,0);
    SetPreRedrawFunc(Editor::PR_EditorShowMsg);
    EditorShowMsg(MSG(MEditTitle),MSG(MEditSaving),Name);
    CurPtr=TopList;

    while (CurPtr!=NULL)
    {
      char *SaveStr,*EndSeq;
      int Length;
      CurPtr->EditLine.GetBinaryString(&SaveStr,&EndSeq,Length);
      if (*EndSeq==0 && CurPtr->Next!=NULL)
        EndSeq=*GlobalEOL ? GlobalEOL:"\r\n";
      if (TextFormat!=0 && *EndSeq!=0)
      {
        if (TextFormat==1)
          EndSeq="\r\n";
        else
          EndSeq="\n";
        CurPtr->EditLine.SetEOL(EndSeq);
      }
      int EndLength=strlen(EndSeq);
      if (fwrite(SaveStr,1,Length,EditFile)!=Length ||
          fwrite(EndSeq,1,EndLength,EditFile)!=EndLength)
      {
        fclose(EditFile);
        remove(Name);
        RetCode=SAVEFILE_ERROR;
        goto end;
      }
      CurPtr=CurPtr->Next;
    }
    if (fflush(EditFile)==EOF)
    {
      fclose(EditFile);
      remove(Name);
      RetCode=SAVEFILE_ERROR;
      goto end;
    }
    SetEndOfFile(hEdit);
    fclose(EditFile);
  }

end:
  SetPreRedrawFunc(NULL);
  if (FileAttributes!=-1)
    SetFileAttributes(Name,FileAttributes|FA_ARCH);
  if (Modified || NewFile)
    WasChanged|=1;

  /* Этот кусок раскомметировать в том случае, если народ решит, что
     для если файл был залочен и мы его переписали под други именем...
     ...то "лочка" должна быть снята.
  */
//  if(SaveAs)
//    LockMode=0;


  /*$ 10.08.2000 skv
    Modified->TextChanged
  */
  TextChanged(0);
  /* skv$*/
  Show();
  return RetCode;
}
/* IS $ */
/* IS $ */

void Editor::DisplayObject()
{
  if (!DisableOut)
    ShowEditor(FALSE);
}


void Editor::ShowEditor(int CurLineOnly)
{
  struct EditList *CurPtr;
  int LeftPos,CurPos,Y;
  if (DisableOut)
    return;

//_SVS(SysLog("Enter to ShowEditor, CurLineOnly=%i",CurLineOnly));
  /*$ 10.08.2000 skv
    To make sure that CurEditor is set to required value.
  */
  CtrlObject->Plugins.CurEditor=this;
  /* skv$*/

  while (CalcDistance(TopScreen,CurLine,-1)>=Y2-Y1)
  {
    DisableOut=TRUE;
    ProcessKey(KEY_UP);
    DisableOut=FALSE;
  }

  CurPos=CurLine->EditLine.GetTabCurPos();
  if (!EdOpt.CursorBeyondEOL)
  {
    MaxRightPos=CurPos;
    int RealCurPos=CurLine->EditLine.GetCurPos();
    int Length=CurLine->EditLine.GetLength();

    if (RealCurPos>Length)
    {
      CurLine->EditLine.SetCurPos(Length);
      CurLine->EditLine.SetLeftPos(0);
      //_D(SysLog("call CurLine->EditLine.FastShow()"));
      CurLine->EditLine.FastShow();
      CurPos=CurLine->EditLine.GetTabCurPos();
    }
  }
  if (!Pasting)
  {
    /*$ 10.08.2000 skv
      Don't send EE_REDRAW while macro is being executed.
      Send EE_REDRAW with param=2 if text was just modified.

    */
    if(!ScrBuf.GetLockCount())
    {
      /*$ 13.09.2000 skv
        EE_REDRAW 1 and 2 replaced.
      */
//_SVS(SysLog("Editor::ShowEditor[%d]: EE_REDRAW (%s)",__LINE__,(JustModified?"EEREDRAW_CHANGE":(CurLineOnly?"EEREDRAW_LINE":"EEREDRAW_ALL"))));
      if(JustModified)
      {
        JustModified=0;
        CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_CHANGE);
      }else
        CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,CurLineOnly?EEREDRAW_LINE:EEREDRAW_ALL);
      /* skv$*/
    }
    /* skv$*/
  }
  if (!CurLineOnly)
  {
    LeftPos=CurLine->EditLine.GetLeftPos();

    for (CurPtr=TopScreen,Y=Y1+1;Y<=Y2;Y++)
      if (CurPtr!=NULL)
      {
        CurPtr->EditLine.SetEditBeyondEnd(TRUE);
        CurPtr->EditLine.SetPosition(X1,Y,X2,Y);
        CurPtr->EditLine.SetTables(UseDecodeTable ? &TableSet:NULL);
        //_D(SysLog("Setleftpos 3 to %i",LeftPos));
        CurPtr->EditLine.SetLeftPos(LeftPos);
        CurPtr->EditLine.SetTabCurPos(CurPos);
        CurPtr->EditLine.FastShow();
        CurPtr->EditLine.SetEditBeyondEnd(EdOpt.CursorBeyondEOL);
        CurPtr=CurPtr->Next;
      }
      else
      {
        GotoXY(X1,Y);
        SetColor(COL_EDITORTEXT);
        mprintf("%*s",ObjWidth,"");
      }
  }

  CurLine->EditLine.SetOvertypeMode(Overtype);
  CurLine->EditLine.Show();

  if (VBlockStart!=NULL && VBlockSizeX>0 && VBlockSizeY>0)
  {
    int CurScreenLine=NumLine-CalcDistance(TopScreen,CurLine,-1);
    LeftPos=CurLine->EditLine.GetLeftPos();
    for (CurPtr=TopScreen,Y=Y1+1;Y<=Y2;Y++)
      if (CurPtr!=NULL)
      {
        if (CurScreenLine>=VBlockY && CurScreenLine<VBlockY+VBlockSizeY)
        {
          /* $ 28.06.2000 tran
             убираем трап при ширине вертикального блока
             более 1000 колонок */

          CHAR_INFO SelBuf[300];
          int BlockX1=VBlockX-LeftPos;
          int BlockX2=VBlockX+VBlockSizeX-1-LeftPos;
          if (BlockX1<X1)
            BlockX1=X1;
          if (BlockX2>X2)
            BlockX2=X2;
          if (BlockX1<=X2 && BlockX2>=X1)
          {
            GetText(BlockX1,Y,BlockX2,Y,SelBuf);
            SetColor(COL_EDITORSELECTEDTEXT);
            int SelColor=GetColor();
            /* tran: было I<VBlockSizeX
               теперь мы заполняем массив ровно столько
               сколько показываем
            */
            for (int I=0;I<=BlockX2;I++)
              SelBuf[I].Attributes=SelColor;
            PutText(BlockX1,Y,BlockX2,Y,SelBuf);
          }
          /* tran $ */
        }
        CurPtr=CurPtr->Next;
        CurScreenLine++;
      }
    }

  ShowStatus();
//_SVS(SysLog("Exit from ShowEditor"));
}


void Editor::ShowStatus()
{
  if (DisableOut)
    return;
  SetColor(COL_EDITORSTATUS);
  GotoXY(X1,Y1);
  char TruncFileName[NM],StatusStr[NM],LineStr[50];
  /* $ 08.06.2001 IS
     - Баг: затирался стек, потому что, например, размер Title больше,
       чем размер TruncFileName
  */
  strncpy(TruncFileName,*PluginTitle ? PluginTitle:(*Title ? Title:FileName),
          sizeof(TruncFileName)-1);
  /* IS $ */
  int NameLength=Opt.ViewerEditorClock ? 19:25;
  /* $ 11.07.2000 tran
     + expand filename if console more when 80 column */
  if (X2>80)
     NameLength+=(X2-80);
  /* tran 11.07.2000 $ */

  if (*PluginTitle || *Title)
    /* $ 20.09.2000 SVS
      - Bugs с "наездом" заголовка (от плагина) на всё прочеЯ!
    */
    /* $ 01.10.2000 IS
      ! Показывать букву диска в статусной строке
    */
    TruncPathStr(TruncFileName,(ObjWidth<NameLength?ObjWidth:NameLength));
    /* IS $ */
    /* SVS $ */
  else
    /* $ 01.10.2000 IS
      ! Показывать букву диска в статусной строке
    */
    TruncPathStr(TruncFileName,NameLength);
    /* IS $ */
  /* $ 14.02.2000 SVS
     Динамический размер под количество строк
  */
  // предварительный расчет.
  sprintf(LineStr,"%d/%d",NumLastLine,NumLastLine);
  int SizeLineStr=strlen(LineStr);
  if(SizeLineStr > 12)
    NameLength-=(SizeLineStr-12);
  else
    SizeLineStr=12;

  sprintf(LineStr,"%d/%d",NumLine+1,NumLastLine);
  /* $ 13.02.2001 IS
    ! Используем уже готовую AttrStr, которая сформирована в
      GetFileAttributes
  */
  sprintf(StatusStr,"%-*s %c%c %10.10s %7s %*.*s %5s %-4d %3s",
          NameLength,TruncFileName,Modified ? '*':' ',LockMode ? '-':' ',
          UseDecodeTable ? TableSet.TableName:AnsiText ? "Win":"DOS",
          MSG(MEditStatusLine),SizeLineStr,SizeLineStr,LineStr,
          MSG(MEditStatusCol),CurLine->EditLine.GetTabCurPos()+1,AttrStr);
  /* IS $ */
  /* SVS $ */
  int StatusWidth=Opt.ViewerEditorClock ? ObjWidth-5:ObjWidth;
  if (StatusWidth<0)
    StatusWidth=0;
  mprintf("%-*.*s",StatusWidth,StatusWidth,StatusStr);

  {
    char *Str;
    int Length;
    CurLine->EditLine.GetBinaryString(&Str,NULL,Length);
    int CurPos=CurLine->EditLine.GetCurPos();
    if (CurPos<Length)
    {
      GotoXY(X2-(Opt.ViewerEditorClock ? 9:2),Y1);
      SetColor(COL_EDITORSTATUS);
      /* $ 27.02.2001 SVS
      Показываем в зависимости от базы */
      static char *FmtCharCode[3]={"%03o","%3d","%02Xh"};
      mprintf(FmtCharCode[EdOpt.CharCodeBase%3],(unsigned char)Str[CurPos]);
      /* SVS $ */
    }
  }
  if (Opt.ViewerEditorClock)
    ShowTime(FALSE);
}
/*$ 10.08.2000 skv
  Wrapper for Modified.
  Set JustModified every call to 1
  to track any text state change.
  Even if state==0, this can be
  last UNDO.
*/
void Editor::TextChanged(int State)
{
  Modified=State;
  JustModified=1;
}
/* skv$*/


int Editor::ProcessKey(int Key)
{
  if (Key==KEY_IDLE)
  {
    if (Opt.ViewerEditorClock)
      ShowTime(FALSE);
    return(TRUE);
  }

  if (Key==KEY_NONE)
    return(TRUE);

  _KEYMACRO(CleverSysLog SL("Editor::ProcessKey()"));
  _KEYMACRO(SysLog("Key=0x%08X",Key));

  int CurPos,CurVisPos,I;
  CurPos=CurLine->EditLine.GetCurPos();
  CurVisPos=GetLineCurPos();

  if ((!ShiftPressed  || CtrlObject->Macro.IsExecuting()) &&
      !IsShiftKey(Key) && !Pasting)
  {
    MarkingBlock=FALSE;
    MarkingVBlock=FALSE;

    if (BlockStart!=NULL || VBlockStart!=NULL && !EdOpt.PersistentBlocks)
      if (!EdOpt.PersistentBlocks)
      {
        static int UnmarkKeys[]={KEY_LEFT,KEY_RIGHT,KEY_HOME,KEY_END,KEY_UP,
                   KEY_DOWN,KEY_PGUP,KEY_PGDN,KEY_CTRLHOME,KEY_CTRLPGUP,
                   KEY_CTRLEND,KEY_CTRLPGDN,KEY_CTRLLEFT,KEY_CTRLRIGHT,
                   KEY_CTRLUP,KEY_CTRLDOWN,KEY_CTRLN,KEY_CTRLE,KEY_CTRLS};
        for (int I=0;I<sizeof(UnmarkKeys)/sizeof(UnmarkKeys[0]);I++)
          if (Key==UnmarkKeys[I])
          {
            UnmarkBlock();
            break;
          }
      }
      else
      {
        int StartSel,EndSel;
        BlockStart->EditLine.GetSelection(StartSel,EndSel);
        if (StartSel==-1 || StartSel==EndSel)
          UnmarkBlock();
      }
  }

  if (Key==KEY_ALTD)
    Key=KEY_CTRLK;

  if (Key>=KEY_CTRL0 && Key<=KEY_CTRL9)
  {
    int Pos=Key-KEY_CTRL0;
    if (SavePos.Line[Pos]!=0xffffffff)
    {
      GoToLine(SavePos.Line[Pos]);
      CurLine->EditLine.SetCurPos(SavePos.Cursor[Pos]);
      CurLine->EditLine.SetLeftPos(SavePos.LeftPos[Pos]);
      TopScreen=CurLine;
      for (int I=0;I<SavePos.ScreenLine[Pos] && TopScreen->Prev!=NULL;I++)
        TopScreen=TopScreen->Prev;
      if (!EdOpt.PersistentBlocks)
        UnmarkBlock();
      Show();
    }
    return(TRUE);
  }
  if (Key>=KEY_CTRLSHIFT0 && Key<=KEY_CTRLSHIFT9)
    Key=Key-KEY_CTRLSHIFT0+KEY_RCTRL0;
  if (Key>=KEY_RCTRL0 && Key<=KEY_RCTRL9)
  {
    int Pos=Key-KEY_RCTRL0;
    SavePos.Line[Pos]=NumLine;
    SavePos.Cursor[Pos]=CurPos;
    SavePos.LeftPos[Pos]=CurLine->EditLine.GetLeftPos();
    SavePos.ScreenLine[Pos]=CalcDistance(TopScreen,CurLine,-1);
    return(TRUE);
  }


  switch(Key)
  {
    case KEY_F1:
      {
        Help Hlp ("Editor");
      }
      return(TRUE);
    case KEY_CTRLSHIFTPGUP:
    case KEY_CTRLSHIFTHOME:
      DisableOut++;
      Pasting++;
      while (CurLine!=TopList)
      {
        Edit::DisableEditOut(TRUE);
        ProcessKey(KEY_SHIFTPGUP);
      }
      ProcessKey(KEY_SHIFTHOME);
      Pasting--;
      DisableOut--;
      Edit::DisableEditOut(FALSE);
      Show();
      return(TRUE);
    case KEY_CTRLSHIFTPGDN:
    case KEY_CTRLSHIFTEND:
      DisableOut++;
      Pasting++;
      while (CurLine!=EndList)
      {
        Edit::DisableEditOut(TRUE);
        ProcessKey(KEY_SHIFTPGDN);
      }
      ProcessKey(KEY_SHIFTEND);
      Pasting--;
      DisableOut--;
      Edit::DisableEditOut(FALSE);
      Show();
      return(TRUE);
    case KEY_SHIFTPGUP:
      Pasting++;
      DisableOut++;
      Edit::DisableEditOut(TRUE);
      for (I=Y1+1;I<Y2;I++)
        ProcessKey(KEY_SHIFTUP);
      Pasting--;
      DisableOut--;
      Edit::DisableEditOut(FALSE);
      Show();
      return(TRUE);
    case KEY_SHIFTPGDN:
      Pasting++;
      DisableOut++;
      Edit::DisableEditOut(TRUE);
      for (I=Y1+1;I<Y2;I++)
        ProcessKey(KEY_SHIFTDOWN);
      Pasting--;
      DisableOut--;
      Edit::DisableEditOut(FALSE);
      Show();
      return(TRUE);
    case KEY_SHIFTHOME:
      Pasting++;
      DisableOut++;
      Edit::DisableEditOut(TRUE);
      /* $ 20.04.2001 IS
           Я не знаю, что двигало Женей (наверное, стремление упростить и
           сократить код), но оно сказалось в худшую сторону на быстродействии.
           Теперь выделяем все за один раз.
      */
      {
        int First=FALSE, // будет TRUE, если выделяем заново, а не продолжаем
            SelStart,SelEnd;
        if (!MarkingBlock)
        {
          UnmarkBlock();
          First=MarkingBlock=TRUE;
          BlockStart=CurLine;
          BlockStartLine=NumLine;
        }

        /* $ 24.05.2001 IS
           ! Приблизим поведение к тому, какое было до 592
        */
        //if (CurPos==0)
        //{ // логика пока не ясна, блок просто скопирован из KEY_SHIFTLEFT
          //CurLine->EditLine.GetSelection(SelStart,SelEnd);
          //if (SelStart==-1 || SelEnd==0)
          //  CurLine->EditLine.Select(-1,0);
        //}
        //else
        if(CurPos)
        {
          int EndPos=CurPos, CurLength=CurLine->EditLine.GetLength();
          if (EndPos>CurLength)
            EndPos=-1; // если курсор за концом строки, то выделим ВСЮ строку

          if (First) // выделяем заново
            CurLine->EditLine.Select(0,EndPos);
          else       // добавляем к уже выделенному
          {
            CurLine->EditLine.GetRealSelection(SelStart,SelEnd);
            if(SelStart<CurPos)
            {
              if(SelStart>CurLength)
                CurLine->EditLine.Select(0,-1);
              else
                CurLine->EditLine.Select(0,SelStart);
            }
            else
              CurLine->EditLine.Select(0,SelEnd);
          }
          CurLine->EditLine.SetCurPos(0);
          CurPos=0;   // надо ли??
        }
        /* IS $ */
      }
      // то, что я накуролесил, раньше делали эти две строки, оставил, чтобы
      // в случае чего далеко не искать
      //while (CurLine->EditLine.GetCurPos()>0)
      //  ProcessKey(KEY_SHIFTLEFT);
      /* IS $ */
      Pasting--;
      DisableOut--;
      Edit::DisableEditOut(FALSE);
      Show();
      return(TRUE);
    case KEY_SHIFTEND:
      {
        int LeftPos=CurLine->EditLine.GetLeftPos();
        Pasting++;
        DisableOut++;
        Edit::DisableEditOut(TRUE);
        int CurLength=CurLine->EditLine.GetLength();
        /* $ 20.04.2001 IS
             Выделяем все за один раз. Попробуйте выделить строку размером в 50
             кил при помощи shift-end до этого патча и после. Почувствуйте
             разницу :)
        */
        {
          int First=FALSE; // будет TRUE, если выделяем заново, а не продолжаем
          if (!MarkingBlock)
          {
            UnmarkBlock();
            First=MarkingBlock=TRUE;
            BlockStart=CurLine;
            BlockStartLine=NumLine;
          }

          /* $ 24.05.2001 IS
             ! Приблизим поведение к тому, какое было до 592
          */
          int SelStart, SelEnd;
          CurLine->EditLine.GetRealSelection(SelStart,SelEnd);
          if(CurPos>CurLength) // мы за пределами строки
          {
            if (First || (SelStart > CurLength)) // выделяем заново или есть блок за пределами строки
              CurLine->EditLine.Select(CurLength, -1);
            else       // убираем выделение от позиции курсора до конца строки
              CurLine->EditLine.Select(SelStart, CurLength);
          }
          else // выделяем вправо
          {
            if(First)
              // выделяем заново
              CurLine->EditLine.Select(CurPos, CurLength);
            else if(SelEnd==-1)
              CurLine->EditLine.Select(CurLength, SelEnd);
            else if(SelEnd<=CurPos)
              CurLine->EditLine.Select(SelStart, CurLength);
            else
              CurLine->EditLine.Select(SelEnd, CurLength);
            /* IS $ */
          }
          CurLine->EditLine.SetCurPos(CurLength);
          CurPos=CurLength;   // надо ли??
        // опять-таки старый код, чтобы был перед глазами
        //if (CurPos>CurLength)
        //  while (CurLine->EditLine.GetCurPos()>CurLength)
        //    ProcessKey(KEY_SHIFTLEFT);
        //else
        //  while (CurLine->EditLine.GetCurPos()<CurLength)
        //    ProcessKey(KEY_SHIFTRIGHT);
        }
        /* IS $ */
        Pasting--;
        DisableOut--;
        Edit::DisableEditOut(FALSE);

        /* $ 13.9.2001 SKV
          Однако LeftPos апдейтится только в FastShow :-\
        */
        CurLine->EditLine.FastShow();
        ShowEditor(LeftPos==CurLine->EditLine.GetLeftPos());
        /* SKV$*/
      }
      return(TRUE);
    case KEY_SHIFTLEFT:
      if (CurPos>0 || CurLine->Prev!=NULL)
      {
        int SelStart,SelEnd;
        if (!MarkingBlock)
        {
          UnmarkBlock();
          MarkingBlock=TRUE;
        }
        if (CurPos==0)
        {
          CurLine->EditLine.GetSelection(SelStart,SelEnd);
          if (SelStart==-1 || SelEnd==0)
            CurLine->EditLine.Select(-1,0);
        }
        int LeftPos=CurLine->EditLine.GetLeftPos();
        Pasting++;
        ProcessKey(KEY_LEFT);
        Pasting--;
        CurLine->EditLine.GetRealSelection(SelStart,SelEnd);
        CurPos=CurLine->EditLine.GetCurPos();
        if (SelStart!=-1 && SelStart<=CurPos)
        {
          if (CurPos==0)
            CurLine->EditLine.Select(-1,0);
          else
            CurLine->EditLine.Select(SelStart,CurPos);
        }
        else
        {
          int EndPos=CurPos+1;
          int Length=CurLine->EditLine.GetLength();
          if (EndPos>Length)
            EndPos=-1;
          CurLine->EditLine.AddSelect(CurPos,EndPos);
          BlockStart=CurLine;
          BlockStartLine=NumLine;
        }
        ShowEditor(LeftPos!=0 && LeftPos==CurLine->EditLine.GetLeftPos());
      }
      return(TRUE);
    case KEY_SHIFTRIGHT:
      {
        int SelStart,SelEnd;
        if (!MarkingBlock)
        {
          UnmarkBlock();
          MarkingBlock=TRUE;
          BlockStart=CurLine;
          BlockStartLine=NumLine;
        }
        CurLine->EditLine.GetSelection(SelStart,SelEnd);
        CurPos=CurLine->EditLine.GetCurPos();
        if (SelStart!=-1 && SelEnd==-1 || SelEnd>CurPos)
        {
          if (CurPos+1==SelEnd)
          {
            CurLine->EditLine.Select(-1,0);
            BlockStart=CurLine->Next;
            BlockStartLine=NumLine+1;
          }
          else
            CurLine->EditLine.Select(CurPos+1,SelEnd);
        }
        else
        {
          CurLine->EditLine.AddSelect(CurPos,CurPos+1);
          if (CurLine->Prev!=NULL)
          {
            CurLine->Prev->EditLine.GetSelection(SelStart,SelEnd);
            if (SelStart==-1)
            {
              BlockStart=CurLine;
              BlockStartLine=NumLine;
            }
          }
          else
          {
            BlockStart=CurLine;
            BlockStartLine=NumLine;
          }
        }
        Pasting++;
        struct EditList *PrevLine=CurLine;
        ProcessKey(KEY_RIGHT);
        if (CurLine!=PrevLine)
        {
          int SelStart,SelEnd;
          PrevLine->EditLine.GetSelection(SelStart,SelEnd);
          if (SelEnd>SelStart)
          {
            PrevLine->EditLine.Select(SelStart,-1);
            CtrlObject->Plugins.CurEditor=this;
//_D(SysLog("%08d EE_REDRAW",__LINE__));
            CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
            PrevLine->EditLine.FastShow();
          }
        }
        Pasting--;
      }
      CtrlObject->Plugins.CurEditor=this;
//_D(SysLog("%08d EE_REDRAW",__LINE__));
      CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_LINE);
      return(TRUE);
    case KEY_CTRLSHIFTLEFT:
      {
        int SkipSpace=TRUE;
        Pasting++;
        /* $ 23.12.2000 OT */
        DisableOut++;
        Edit::DisableEditOut(TRUE);
        /* OT $ */
        while (1)
        {
          char *Str;
          int Length;
          CurLine->EditLine.GetBinaryString(&Str,NULL,Length);
          int CurPos=CurLine->EditLine.GetCurPos();
          if (CurPos>Length)
          {
            CurLine->EditLine.ProcessKey(KEY_END);
            CurPos=CurLine->EditLine.GetCurPos();
          }
          if (CurPos==0)
            break;
          /* $ 03.08.2000 SVS
            ! WordDiv -> Opt.WordDiv
          */
          if (isspace(Str[CurPos-1]) || strchr(Opt.WordDiv,Str[CurPos-1])!=NULL)
          /* SVS $ */
            if (SkipSpace)
            {
              ProcessKey(KEY_SHIFTLEFT);
              continue;
            }
            else
              break;
          SkipSpace=FALSE;
          ProcessKey(KEY_SHIFTLEFT);
        }
        Pasting--;
        /* $ 23.12.2000 OT */
        DisableOut--;
        Edit::DisableEditOut(FALSE);
        Show();
        /* OT $ */
      }
      return(TRUE);
    case KEY_CTRLSHIFTRIGHT:
      {
        int SkipSpace=TRUE;
        Pasting++;
        DisableOut++;
        Edit::DisableEditOut(TRUE);
        while (1)
        {
          char *Str;
          int Length;
          CurLine->EditLine.GetBinaryString(&Str,NULL,Length);
          int CurPos=CurLine->EditLine.GetCurPos();
          if (CurPos>=Length)
            break;
          /* $ 03.08.2000 SVS
            ! WordDiv -> Opt.WordDiv
          */
          if (isspace(Str[CurPos]) || strchr(Opt.WordDiv,Str[CurPos])!=NULL)
          /* SVS $ */
            if (SkipSpace)
            {
              ProcessKey(KEY_SHIFTRIGHT);
              continue;
            }
            else
              break;
          SkipSpace=FALSE;
          ProcessKey(KEY_SHIFTRIGHT);
        }
        Pasting--;
        DisableOut--;
        Edit::DisableEditOut(FALSE);
        Show();
      }
      return(TRUE);
    case KEY_SHIFTDOWN:
      if (CurLine->Next!=NULL)
      {
        int SelStart,SelEnd,NextPos;
        int TabCurPos=CurLine->EditLine.GetTabCurPos();
        if (!MarkingBlock)
        {
          UnmarkBlock();
          MarkingBlock=TRUE;
          BlockStart=CurLine;
          BlockStartLine=NumLine;
        }
        CurLine->Next->EditLine.GetSelection(SelStart,SelEnd);
        CurLine->Next->EditLine.SetTabCurPos(TabCurPos);
        NextPos=CurLine->Next->EditLine.GetCurPos();
        if (SelStart!=-1)
        {
          CurLine->EditLine.Select(-1,0);
          if (SelEnd!=-1 && NextPos>SelEnd)
            CurLine->Next->EditLine.Select(SelEnd,NextPos);
          else
            CurLine->Next->EditLine.Select(NextPos,SelEnd);
          BlockStart=CurLine->Next;
          BlockStartLine=NumLine+1;
        }
        else
        {
          CurLine->EditLine.AddSelect(CurPos,-1);
          CurLine->Next->EditLine.Select(0,NextPos);
        }
        Down();
        Show();
      }
      return(TRUE);
    case KEY_SHIFTUP:
      if (CurLine->Prev!=NULL)
      {
        int SelStart,SelEnd,PrevPos;
        int TabCurPos=CurLine->EditLine.GetTabCurPos();
        if (!MarkingBlock)
        {
          UnmarkBlock();
          MarkingBlock=TRUE;
        }
        CurLine->Prev->EditLine.GetSelection(SelStart,SelEnd);
        CurLine->Prev->EditLine.SetTabCurPos(TabCurPos);
        PrevPos=CurLine->Prev->EditLine.GetCurPos();
        if (SelStart!=-1)
        {
          CurLine->EditLine.Select(-1,0);
          if (PrevPos<SelStart)
            CurLine->Prev->EditLine.Select(PrevPos,SelStart);
          else
            CurLine->Prev->EditLine.Select(SelStart,PrevPos);
        }
        else
        {
          BlockStart=CurLine->Prev;
          BlockStartLine=NumLine-1;
          CurLine->EditLine.AddSelect(0,CurPos);
          CurLine->Prev->EditLine.Select(PrevPos,-1);
        }
        Up();
        Show();
      }
      return(TRUE);
    case KEY_CTRLADD:
      Copy(TRUE);
      return(TRUE);
    case KEY_CTRLA:
      UnmarkBlock();
      SelectAll();
      return(TRUE);
    case KEY_CTRLU:
      UnmarkBlock();
      return(TRUE);
    case KEY_CTRLC:
    case KEY_CTRLINS:
      if (!EdOpt.PersistentBlocks && BlockStart==NULL && VBlockStart==NULL)
      {
        BlockStart=CurLine;
        BlockStartLine=NumLine;
        CurLine->EditLine.AddSelect(0,-1);
        Show();
      }
      Copy(FALSE);
      return(TRUE);
    case KEY_CTRLP:
    case KEY_CTRLM:
      if (BlockStart!=NULL || VBlockStart!=NULL)
      {
        char *OemData=PasteFromClipboard();
        char *VBlockData=PasteFormatFromClipboard("FAR_VerticalBlock");

        int SelStart,SelEnd;
        CurLine->EditLine.GetSelection(SelStart,SelEnd);

        Pasting++;
        ProcessKey(Key==KEY_CTRLP ? KEY_CTRLINS:KEY_SHIFTDEL);

        /* $ 10.04.2001 SVS
          ^P/^M - некорректно работали: уловие для CurPos должно быть ">=",
           а не "меньше".
        */
        if (Key==KEY_CTRLM && SelStart!=-1 && SelEnd!=-1)
          if (CurPos>=SelEnd)
            CurLine->EditLine.SetCurPos(CurPos-(SelEnd-SelStart));
          else
            CurLine->EditLine.SetCurPos(CurPos);
        /* SVS $ */
        ProcessKey(KEY_SHIFTINS);
        Pasting--;

        /*$ 08.02.2001 SKV
          всё делалось с pasting'ом, поэтому redraw плагинам не ушел.
          сделаем его.
        */
        Show();
        /* SKV$*/

        if (OemData!=NULL)
        {
          CopyToClipboard(OemData);
          delete OemData;
        }
        if (VBlockData!=NULL)
        {
          CopyFormatToClipboard("FAR_VerticalBlock",VBlockData);
          delete VBlockData;
        }
      }
      return(TRUE);
    case KEY_CTRLX:
    case KEY_SHIFTDEL:
      Copy(FALSE);
    case KEY_CTRLD:
      MarkingBlock=MarkingVBlock=FALSE;
      DeleteBlock();
      Show();
      return(TRUE);
    case KEY_CTRLV:
    case KEY_SHIFTINS:
      /* $ 10.04.2001 SVS
         Забыли Pasting выставить :-(
      */
      Pasting++;
      if (!EdOpt.PersistentBlocks && VBlockStart==NULL)
        DeleteBlock();
      Paste();
      MarkingBlock=(VBlockStart==NULL);
      MarkingVBlock=FALSE;
      if (!EdOpt.PersistentBlocks)
        UnmarkBlock();
      Pasting--;
      Show();
      return(TRUE);
      /* SVS $ */
    case KEY_LEFT:
      NewUndo=TRUE;
      if (CurPos==0 && CurLine->Prev!=NULL)
      {
        Up();
        Show();
        CurLine->EditLine.ProcessKey(KEY_END);
        Show();
      }
      else
      {
        int LeftPos=CurLine->EditLine.GetLeftPos();
        CurLine->EditLine.ProcessKey(KEY_LEFT);
        ShowEditor(LeftPos==CurLine->EditLine.GetLeftPos());
      }
      return(TRUE);
    case KEY_INS:
      Overtype=!Overtype;
      Show();
      return(TRUE);
    case KEY_DEL:
      if (!LockMode)
      {
        if (!Pasting && EdOpt.DelRemovesBlocks && (BlockStart!=NULL || VBlockStart!=NULL))
          DeleteBlock();
        else
        {
          AddUndoData(CurLine->EditLine.GetStringAddr(),NumLine,
                      CurLine->EditLine.GetCurPos(),UNDO_EDIT);
          if (CurPos>=CurLine->EditLine.GetLength())
          {
            if (CurLine->Next==NULL)
              CurLine->EditLine.SetEOL("");
            else
            {
              int SelStart,SelEnd,NextSelStart,NextSelEnd;
              int Length=CurLine->EditLine.GetLength();
              CurLine->EditLine.GetSelection(SelStart,SelEnd);
              CurLine->Next->EditLine.GetSelection(NextSelStart,NextSelEnd);

              char *Str;
              int NextLength;
              CurLine->Next->EditLine.GetBinaryString(&Str,NULL,NextLength);
              CurLine->EditLine.InsertBinaryString(Str,NextLength);
              CurLine->EditLine.SetCurPos(CurPos);

              BlockUndo++;
              DeleteString(CurLine->Next,TRUE,NumLine+1);
              BlockUndo--;
              if (NextLength==0)
                CurLine->EditLine.SetEOL("");

              if (NextSelStart!=-1)
                if (SelStart==-1)
                {
                  CurLine->EditLine.Select(Length+NextSelStart,NextSelEnd==-1 ? -1:Length+NextSelEnd);
                  BlockStart=CurLine;
                  BlockStartLine=NumLine;
                }
                else
                  CurLine->EditLine.Select(SelStart,NextSelEnd==-1 ? -1:Length+NextSelEnd);

            }
          }
          else
            CurLine->EditLine.ProcessKey(KEY_DEL);
        }
        /*$ 10.08.2000 skv
          Modified->TextChanged
        */
        TextChanged(1);
        /* skv $*/
        Show();
      }
      return(TRUE);
    case KEY_BS:
      if (!LockMode)
      {
        /*$ 10.08.2000 skv
          Modified->TextChanged
        */
        TextChanged(1);
        /* skv $*/
        /* $ 11.10.2000 SVS
           Bs удаляет блок так же, как и Del
        */
        int IsDelBlock=FALSE;
        if(EdOpt.BSLikeDel)
        {
          if (!Pasting && EdOpt.DelRemovesBlocks && (BlockStart!=NULL || VBlockStart!=NULL))
            IsDelBlock=TRUE;
        }
        else
        {
          if (!Pasting && !EdOpt.PersistentBlocks && BlockStart!=NULL)
            IsDelBlock=TRUE;
        }
        if (IsDelBlock)
        /* SVS $ */
          DeleteBlock();
        else
          if (CurPos==0 && CurLine->Prev!=NULL)
          {
            Pasting++;
            Up();
            CurLine->EditLine.ProcessKey(KEY_CTRLEND);
            ProcessKey(KEY_DEL);
            Pasting--;
          }
          else
          {
            AddUndoData(CurLine->EditLine.GetStringAddr(),NumLine,
                        CurLine->EditLine.GetCurPos(),UNDO_EDIT);
            CurLine->EditLine.ProcessKey(KEY_BS);
          }

        Show();
      }
      return(TRUE);
    case KEY_CTRLBS:
      if (!LockMode)
      {
        /*$ 10.08.2000 skv
          Modified->TextChanged
        */
        TextChanged(1);
        /* skv $*/
        if (!Pasting && !EdOpt.PersistentBlocks && BlockStart!=NULL)
          DeleteBlock();
        else
          if (CurPos==0 && CurLine->Prev!=NULL)
            ProcessKey(KEY_BS);
          else
          {
            AddUndoData(CurLine->EditLine.GetStringAddr(),NumLine,
                        CurLine->EditLine.GetCurPos(),UNDO_EDIT);
            CurLine->EditLine.ProcessKey(KEY_CTRLBS);
          }
        Show();
      }
      return(TRUE);
    case KEY_UP:
      {
        NewUndo=TRUE;
        int PrevMaxPos=MaxRightPos;
        struct EditList *LastTopScreen=TopScreen;
        Up();
        if (TopScreen==LastTopScreen)
          ShowEditor(TRUE);
        else
          Show();
        if (PrevMaxPos>CurLine->EditLine.GetTabCurPos())
        {
          CurLine->EditLine.SetTabCurPos(PrevMaxPos);
          CurLine->EditLine.FastShow();
          CurLine->EditLine.SetTabCurPos(PrevMaxPos);
          Show();
        }
      }
      return(TRUE);
    case KEY_DOWN:
      {
        NewUndo=TRUE;
        int PrevMaxPos=MaxRightPos;
        struct EditList *LastTopScreen=TopScreen;
        Down();
        if (TopScreen==LastTopScreen)
          ShowEditor(TRUE);
        else
          Show();
        if (PrevMaxPos>CurLine->EditLine.GetTabCurPos())
        {
          CurLine->EditLine.SetTabCurPos(PrevMaxPos);
          CurLine->EditLine.FastShow();
          CurLine->EditLine.SetTabCurPos(PrevMaxPos);
          Show();
        }
      }
      return(TRUE);
    /* $ 27.04.2001 VVM
      + Обработка колеса мышки */
    case KEY_MSWHEEL_UP:
      {
        for (int i=0; i<Opt.MsWheelDeltaEdit; i++)
          ProcessKey(KEY_CTRLUP);
        return(TRUE);
      }
    case KEY_MSWHEEL_DOWN:
      {
        for (int i=0; i<Opt.MsWheelDeltaEdit; i++)
          ProcessKey(KEY_CTRLDOWN);
        return(TRUE);
      }
    /* VVM $ */
    case KEY_CTRLUP:
      NewUndo=TRUE;
      ScrollUp();
      Show();
      return(TRUE);
    case KEY_CTRLDOWN:
      NewUndo=TRUE;
      ScrollDown();
      Show();
      return(TRUE);
    case KEY_PGUP:
      NewUndo=TRUE;
      for (I=Y1+1;I<Y2;I++)
        ScrollUp();
      Show();
      return(TRUE);
    case KEY_PGDN:
      NewUndo=TRUE;
      for (I=Y1+1;I<Y2;I++)
        ScrollDown();
      Show();
      return(TRUE);
    case KEY_CTRLHOME:
    case KEY_CTRLPGUP:
      {
        NewUndo=TRUE;
        int StartPos=CurLine->EditLine.GetTabCurPos();
        NumLine=0;
        TopScreen=CurLine=TopList;
        if (Key==KEY_CTRLHOME)
          CurLine->EditLine.SetCurPos(0);
        else
          CurLine->EditLine.SetTabCurPos(StartPos);
        Show();
      }
      return(TRUE);
    case KEY_CTRLEND:
    case KEY_CTRLPGDN:
      {
        NewUndo=TRUE;
        int StartPos=CurLine->EditLine.GetTabCurPos();
        NumLine=NumLastLine-1;
        CurLine=EndList;
        for (TopScreen=CurLine,I=Y1+1;I<Y2 && TopScreen->Prev!=NULL;I++)
          TopScreen=TopScreen->Prev;
        CurLine->EditLine.SetLeftPos(0);
        if (Key==KEY_CTRLEND)
          CurLine->EditLine.SetCurPos(CurLine->EditLine.GetLength());
        else
          CurLine->EditLine.SetTabCurPos(StartPos);
        Show();
      }
      return(TRUE);
    case KEY_ENTER:
      if (Pasting || !ShiftPressed || CtrlObject->Macro.IsExecuting())
      {
        if (!Pasting && !EdOpt.PersistentBlocks && BlockStart!=NULL)
          DeleteBlock();
        NewUndo=TRUE;
        InsertString();
        Show();
      }
      return(TRUE);
    case KEY_CTRLN:
      NewUndo=TRUE;
      while (CurLine!=TopScreen)
      {
        CurLine=CurLine->Prev;
        NumLine--;
      }
      CurLine->EditLine.SetCurPos(CurPos);
      Show();
      return(TRUE);
    case KEY_CTRLE:
      {
        NewUndo=TRUE;
        struct EditList *CurPtr=TopScreen;
        int CurLineFound=FALSE;
        for (I=Y1+1;I<Y2;I++)
        {
          if (CurPtr->Next==NULL)
            break;
          if (CurPtr==CurLine)
            CurLineFound=TRUE;
          if (CurLineFound)
            NumLine++;
          CurPtr=CurPtr->Next;
        }
        CurLine=CurPtr;
        CurLine->EditLine.SetCurPos(CurPos);
        Show();
      }
      return(TRUE);
    case KEY_CTRLL:
      LockMode=!LockMode;
      Show();
      return(TRUE);
    case KEY_CTRLY:
      DeleteString(CurLine,FALSE,NumLine);
      Show();
      return(TRUE);
    case KEY_CTRLW:
      ShowProcessList();
      return(TRUE);
    case KEY_F7:
    {
      int ReplaceMode0=ReplaceMode;
      int ReplaceAll0=ReplaceAll;
      ReplaceMode=ReplaceAll=FALSE;
      if(!Search(FALSE))
      {
        ReplaceMode=ReplaceMode0;
        ReplaceAll=ReplaceAll0;
      }
      return(TRUE);
    }
    case KEY_CTRLF7:
      if (!LockMode)
      {
        int ReplaceMode0=ReplaceMode;
        int ReplaceAll0=ReplaceAll;
        ReplaceMode=TRUE;
        ReplaceAll=FALSE;
        if(!Search(FALSE))
        {
          ReplaceMode=ReplaceMode0;
          ReplaceAll=ReplaceAll0;
        }
      }
      return(TRUE);
    case KEY_SHIFTF7:
      /* $ 20.09.2000 SVS
         При All после нажатия Shift-F7 надобно снова спросить...
      */
      //ReplaceAll=FALSE;
      /* SVS $*/
      /* $ 07.05.2001 IS
         Сказано в хелпе "Shift-F7 Продолжить _поиск_"
      */
      //ReplaceMode=FALSE;
      /* IS */
      MarkingBlock=FALSE;
      MarkingVBlock=FALSE;
      Search(TRUE);
      return(TRUE);
    case KEY_F8:
      TableChangedByUser=TRUE;
      if ((AnsiText=!AnsiText)!=0)
      {
        int UseUnicode=FALSE;
        GetTable(&TableSet,TRUE,TableNum,UseUnicode);
      }
      TableNum=0;
      UseDecodeTable=AnsiText;
      SetStringsTable();
      ChangeEditKeyBar();
      Show();
      return(TRUE);
    case KEY_SHIFTF8:
      {
        int UseUnicode=FALSE;
        int GetTableCode=GetTable(&TableSet,FALSE,TableNum,UseUnicode);
        if (GetTableCode!=-1)
        {
          TableChangedByUser=TRUE;
          UseDecodeTable=GetTableCode;
          AnsiText=FALSE;
          SetStringsTable();
          ChangeEditKeyBar();
          Show();
        }
      }
      return(TRUE);
    case KEY_F11:
/*
      CtrlObject->Plugins.CurEditor=this;
      if (CtrlObject->Plugins.CommandsMenu(MODALTYPE_EDITOR,0,"Editor"))
        *PluginTitle=0;
      Show();
*/
      return(TRUE);
    case KEY_ALTBS:
    case KEY_CTRLZ:
      if (!LockMode)
      {
        /*$ 10.08.2000 skv
          Without this group undo, like undo of 'delete block' operation
          will be animated.
        */
        DisableOut++;
        Undo();
        DisableOut--;
        /* skv$*/
        Show();
      }
      return(TRUE);
    case KEY_ALTF8:
      {
        /* $ 05.07.2000 tran
           + возможность переходить не только на строку, но и на колонку */
        /* $ 21.07.2000 tran
           Все внутри функции */
        GoToPosition();
        /* tran 21.07.2000 $ */
        /* tran 05.07.2000 $ */
        Show();
      }
      return(TRUE);
    case KEY_ALTU:
      if (!LockMode)
      {
        BlockLeft();
        Show();
      }
      return(TRUE);
    case KEY_ALTI:
      if (!LockMode)
      {
        BlockRight();
        Show();
      }
      return(TRUE);
    case KEY_ALTSHIFTLEFT:
    case KEY_ALTLEFT:
      if (CurPos==0)
        return(TRUE);
      /* $ 21.07.2000 tran
         код вынес в BeginVBlockMarking */
      if (!MarkingVBlock)
        BeginVBlockMarking();
      /* tran 21.07.2000 $ */
      Pasting++;
      {
        int Delta=CurLine->EditLine.GetTabCurPos()-CurLine->EditLine.RealPosToTab(CurPos-1);
        if (CurLine->EditLine.GetTabCurPos()>VBlockX)
          VBlockSizeX-=Delta;
        else
        {
          VBlockX-=Delta;
          VBlockSizeX+=Delta;
        }
        /* $ 25.07.2000 tran
           остатки бага 22 - подправка при перебега за границу блока */
        if ( VBlockSizeX<0 )
        {
            VBlockSizeX=-VBlockSizeX;
            VBlockX-=VBlockSizeX;
        }
        /* tran 25.07.2000 $ */
        ProcessKey(KEY_LEFT);
      }
      Pasting--;
      Show();
      //_D(SysLog("VBlockX=%i, VBlockSizeX=%i, GetLineCurPos=%i",VBlockX,VBlockSizeX,GetLineCurPos()));
      //_D(SysLog("~~~~~~~~~~~~~~~~ KEY_ALTLEFT END, VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
      return(TRUE);
    case KEY_ALTSHIFTRIGHT:
    case KEY_ALTRIGHT:
      /* $ 23.10.2000 tran
         вместо GetTabCurPos надо вызывать GetCurPos -
         сравнивать реальную позицию с реальной длиной
         а было сравнение видимой позицией с реальной длиной*/
      if (!EdOpt.CursorBeyondEOL && CurLine->EditLine.GetCurPos()>=CurLine->EditLine.GetLength())
        return(TRUE);
      /* tran 23.10.2000 $ */

      /* $ 21.07.2000 tran
         код вынес в BeginVBlockMarking */
      if (!MarkingVBlock)
        BeginVBlockMarking();
      /* tran 21.07.2000 $ */

      /* $ 21.07.2000 tran
         bug22 - продолжение
      */
      //_D(SysLog("---------------- KEY_ALTRIGHT, getLineCurPos=%i",GetLineCurPos()));
      Pasting++;
      {
        int Delta;
        /* $ 18.07.2000 tran
             встань в начало текста, нажми alt-right, alt-pagedown,
             выделится блок шириной в 1 колонку, нажми еще alt-right
             выделение сбросится
        */
        int VisPos=CurLine->EditLine.RealPosToTab(CurPos),
            NextVisPos=CurLine->EditLine.RealPosToTab(CurPos+1);
        //_D(SysLog("CurPos=%i, VisPos=%i, NextVisPos=%i",
        //    CurPos,VisPos, NextVisPos); //,CurLine->EditLine.GetTabCurPos()));

        Delta=NextVisPos-VisPos;
         //_D(SysLog("Delta=%i",Delta));
        /* tran $ */

        if (CurLine->EditLine.GetTabCurPos()>=VBlockX+VBlockSizeX)
          VBlockSizeX+=Delta;
        else
        {
          VBlockX+=Delta;
          VBlockSizeX-=Delta;
        }
        /* $ 25.07.2000 tran
           остатки бага 22 - подправка при перебега за границу блока */
        if ( VBlockSizeX<0 )
        {
            VBlockSizeX=-VBlockSizeX;
            VBlockX-=VBlockSizeX;
        }
        /* tran 25.07.2000 $ */
        ProcessKey(KEY_RIGHT);
        //_D(SysLog("VBlockX=%i, VBlockSizeX=%i, GetLineCurPos=%i",VBlockX,VBlockSizeX,GetLineCurPos()));
      }
      Pasting--;
      Show();
      //_D(SysLog("~~~~~~~~~~~~~~~~ KEY_ALTRIGHT END, VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
      /* tran 21.07.2000 $ */

      return(TRUE);
  /* $ 29.06.2000 IG
      + CtrlAltLeft, CtrlAltRight для вертикальный блоков
  */
    case KEY_CTRLALTLEFT:
      {
        int SkipSpace=TRUE;
        Pasting++;
        /* $ 23.12.2000 OT */
        DisableOut++;
        Edit::DisableEditOut(TRUE);
        /* OT $ */
        while (1)
        {
          char *Str;
          int Length;
          CurLine->EditLine.GetBinaryString(&Str,NULL,Length);
          int CurPos=CurLine->EditLine.GetCurPos();
          if (CurPos>Length)
          {
            CurLine->EditLine.ProcessKey(KEY_END);
            CurPos=CurLine->EditLine.GetCurPos();
          }
          if (CurPos==0)
            break;
          /* $ 03.08.2000 SVS
            ! WordDiv -> Opt.WordDiv
          */
          if (isspace(Str[CurPos-1]) || strchr(Opt.WordDiv,Str[CurPos-1])!=NULL)
          /* SVS $ */
            if (SkipSpace)
            {
              ProcessKey(KEY_ALTSHIFTLEFT);
              continue;
            }
            else
              break;
          SkipSpace=FALSE;
          ProcessKey(KEY_ALTSHIFTLEFT);
        }
        Pasting--;
        /* $ 23.12.2000 OT */
        DisableOut--;
        Edit::DisableEditOut(FALSE);
        Show();
        /* OT $ */
      }
      return(TRUE);
    case KEY_CTRLALTRIGHT:
      {
        int SkipSpace=TRUE;
        Pasting++;
        DisableOut++;
        Edit::DisableEditOut(TRUE);
        while (1)
        {
          char *Str;
          int Length;
          CurLine->EditLine.GetBinaryString(&Str,NULL,Length);
          int CurPos=CurLine->EditLine.GetCurPos();
          if (CurPos>=Length)
            break;
          /* $ 03.08.2000 SVS
            ! WordDiv -> Opt.WordDiv
          */
          if (isspace(Str[CurPos]) || strchr(Opt.WordDiv,Str[CurPos])!=NULL)
          /* SVS $*/
            if (SkipSpace)
            {
              ProcessKey(KEY_ALTSHIFTRIGHT);
              continue;
            }
            else
              break;
          SkipSpace=FALSE;
          ProcessKey(KEY_ALTSHIFTRIGHT);
        }
        Pasting--;
        DisableOut--;
        Edit::DisableEditOut(FALSE);
        Show();
      }
      return(TRUE);
  /* IG $ */

    case KEY_ALTSHIFTUP:
    case KEY_ALTUP:
      if (CurLine->Prev==NULL)
        return(TRUE);
      /* $ 21.07.2000 tran
         код вынес в BeginVBlockMarking */
      if (!MarkingVBlock)
        BeginVBlockMarking();
      /* tran 21.07.2000 $ */

      if (!EdOpt.CursorBeyondEOL && VBlockX>=CurLine->Prev->EditLine.GetLength())
        return(TRUE);
      Pasting++;
      if (NumLine>VBlockY)
        VBlockSizeY--;
      else
      {
        VBlockY--;
        VBlockSizeY++;
        VBlockStart=VBlockStart->Prev;
        BlockStartLine--;
      }
      ProcessKey(KEY_UP);
      /* $ 21.07.2000 tran
         вызываем функцию подгонки блока */
      AdjustVBlock(CurVisPos);
      /* tran 21.07.2000 $ */
      Pasting--;
      Show();
      //_D(SysLog("~~~~~~~~ ALT_PGUP, VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
      return(TRUE);
    case KEY_ALTSHIFTDOWN:
    case KEY_ALTDOWN:
      if (CurLine->Next==NULL)
        return(TRUE);
      /* $ 21.07.2000 tran
         код вынес в BeginVBlockMarking */
      if (!MarkingVBlock)
        BeginVBlockMarking();
      /* tran 21.07.2000 $ */
      if (!EdOpt.CursorBeyondEOL && VBlockX>=CurLine->Next->EditLine.GetLength())
        return(TRUE);
      Pasting++;
      if (NumLine>=VBlockY+VBlockSizeY-1)
        VBlockSizeY++;
      else
      {
        VBlockY++;
        VBlockSizeY--;
        VBlockStart=VBlockStart->Next;
        BlockStartLine++;
      }
      ProcessKey(KEY_DOWN);
      /* $ 21.07.2000 tran
         вызываем функцию подгонки блока */
      AdjustVBlock(CurVisPos);
      /* tran 21.07.2000 $ */
      Pasting--;
      Show();
      //_D(SysLog("~~~~ Key_AltDOWN: VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
      return(TRUE);
    case KEY_ALTSHIFTHOME:
    case KEY_ALTHOME:
      Pasting++;
      DisableOut++;
      while (CurLine->EditLine.GetCurPos()>0)
        ProcessKey(KEY_ALTSHIFTLEFT);
      DisableOut--;
      Pasting--;
      Show();
      return(TRUE);
    case KEY_ALTSHIFTEND:
    case KEY_ALTEND:
      Pasting++;
      DisableOut++;
      if (CurLine->EditLine.GetCurPos()<CurLine->EditLine.GetLength())
        while (CurLine->EditLine.GetCurPos()<CurLine->EditLine.GetLength())
          ProcessKey(KEY_ALTSHIFTRIGHT);
      if (CurLine->EditLine.GetCurPos()>CurLine->EditLine.GetLength())
        while (CurLine->EditLine.GetCurPos()>CurLine->EditLine.GetLength())
          ProcessKey(KEY_ALTSHIFTLEFT);
      DisableOut--;
      Pasting--;
      Show();
      return(TRUE);
    case KEY_ALTSHIFTPGUP:
    case KEY_ALTPGUP:
      Pasting++;
      DisableOut++;
      for (I=Y1+1;I<Y2;I++)
        ProcessKey(KEY_ALTSHIFTUP);
      DisableOut--;
      Pasting--;
      Show();
      return(TRUE);
    case KEY_ALTSHIFTPGDN:
    case KEY_ALTPGDN:
      Pasting++;
      DisableOut++;
      for (I=Y1+1;I<Y2;I++)
        ProcessKey(KEY_ALTSHIFTDOWN);
      DisableOut--;
      Pasting--;
      Show();
      return(TRUE);
    case KEY_CTRLALTPGUP:
    case KEY_CTRLALTHOME:
      DisableOut++;
      Pasting++;
      while (CurLine!=TopList)
      {
        Edit::DisableEditOut(TRUE);
        ProcessKey(KEY_ALTUP);
      }
      Pasting--;
      DisableOut--;
      Edit::DisableEditOut(FALSE);
      Show();
      return(TRUE);
    case KEY_CTRLALTPGDN:
    case KEY_CTRLALTEND:
      DisableOut++;
      Pasting++;
      while (CurLine!=EndList)
      {
        Edit::DisableEditOut(TRUE);
        ProcessKey(KEY_ALTDOWN);
      }
      Pasting--;
      DisableOut--;
      Edit::DisableEditOut(FALSE);
      Show();
      return(TRUE);

    case KEY_CTRLALTBRACKET:       // Вставить сетевое (UNC) путь из левой панели
    case KEY_CTRLALTBACKBRACKET:   // Вставить сетевое (UNC) путь из правой панели
    case KEY_ALTSHIFTBRACKET:      // Вставить сетевое (UNC) путь из активной панели
    case KEY_ALTSHIFTBACKBRACKET:  // Вставить сетевое (UNC) путь из пассивной панели
    case KEY_CTRLBRACKET:          // Вставить путь из левой панели
    case KEY_CTRLBACKBRACKET:      // Вставить путь из правой панели
    case KEY_CTRLSHIFTBRACKET:     // Вставить путь из активной панели
    case KEY_CTRLSHIFTBACKBRACKET: // Вставить путь из пассивной панели

    case KEY_CTRLSHIFTENTER:
    case KEY_SHIFTENTER:
      if (!LockMode)
      {
        Pasting++;
        TextChanged(1);
        if (!EdOpt.PersistentBlocks && BlockStart!=NULL)
        {
          MarkingBlock=MarkingVBlock=FALSE;
          DeleteBlock();
        }
        AddUndoData(CurLine->EditLine.GetStringAddr(),NumLine,
                        CurLine->EditLine.GetCurPos(),UNDO_EDIT);
        CurLine->EditLine.ProcessKey(Key);
        Pasting--;
        Show();
      }
      return(TRUE);
    /* $ 11.04.2001 SVS
       Добавлена обработка Ctrl-Q
    */
    case KEY_CTRLQ:
      if (!LockMode)
      {
        Pasting++;
        TextChanged(1);
        if (!EdOpt.PersistentBlocks && BlockStart!=NULL)
        {
          MarkingBlock=MarkingVBlock=FALSE;
          DeleteBlock();
        }
        AddUndoData(CurLine->EditLine.GetStringAddr(),NumLine,
                        CurLine->EditLine.GetCurPos(),UNDO_EDIT);
        CurLine->EditLine.ProcessCtrlQ();
        Pasting--;
        Show();
      }
      return(TRUE);
    /* SVS $ */
    case KEY_MACRODATE:
      if (!LockMode)
      {
        char TStr[NM],Fmt[NM];
        struct tm *time_now;
        time_t secs_now;
        tzset();
        time(&secs_now);
        time_now = localtime(&secs_now);
        CtrlObject->Macro.GetMacroPlainText(Fmt);
        if(!Fmt[0])
          strcpy(Fmt,Opt.DateFormat);
        if(StrFTime(TStr, sizeof(TStr),Fmt,time_now))
        {
          char *Ptr=TStr;
          while(*Ptr) // заменим 0x0A на 0x0D по правилам Paset ;-)
          {
            if(*Ptr == 10)
              *Ptr=13;
            ++Ptr;
          }
          Pasting++;
          //_SVS(SysLogDump(Fmt,0,TStr,strlen(TStr),NULL));
          TextChanged(1);
          if (!EdOpt.PersistentBlocks && BlockStart!=NULL)
          {
            MarkingBlock=MarkingVBlock=FALSE;
            DeleteBlock();
          }
          //AddUndoData(CurLine->EditLine.GetStringAddr(),NumLine,
          //              CurLine->EditLine.GetCurPos(),UNDO_EDIT);
          Paste(TStr);
          if (!EdOpt.PersistentBlocks)
            UnmarkBlock();
          Pasting--;
          Show();
        }
      }
      return(TRUE);
    /* $ 25.04.2001 IS
         ctrl+f - вставить в строку полное имя редактируемого файла
    */
    case KEY_CTRLF:
      if(!LockMode)
      {
        Pasting++;
        TextChanged(1);
        if (!EdOpt.PersistentBlocks && BlockStart!=NULL)
        {
          MarkingBlock=MarkingVBlock=FALSE;
          DeleteBlock();
        }
        //AddUndoData(CurLine->EditLine.GetStringAddr(),NumLine,
        //                CurLine->EditLine.GetCurPos(),UNDO_EDIT);
        char FileName0[NM];
        strncpy(FileName0,FileName,sizeof(FileName0)-1);
        Paste(FileName0);
        if (!EdOpt.PersistentBlocks)
          UnmarkBlock();
        Pasting--;
        Show();
      }
      return (TRUE);
    /* IS $ */
    /* $ 25.04.2001 SVS
       Для макросов - есть блок или нету
    */
    case KEY_MEDIT_ISSELECTED:
      return BlockStart || VBlockStart?TRUE:FALSE;
    /* SVS $ */
    default:
      {
        if ((Key==KEY_CTRLDEL || Key==KEY_CTRLT) && CurPos>=CurLine->EditLine.GetLength())
        {
         /*$ 08.12.2000 skv
           - CTRL-DEL в начале строки при выделенном блоке и
             включенном EditorDelRemovesBlocks
         */
          int save=EdOpt.DelRemovesBlocks;
          EdOpt.DelRemovesBlocks=0;
          int ret=ProcessKey(KEY_DEL);
          EdOpt.DelRemovesBlocks=save;
          return ret;
          /* skv$*/
        }

        if (!Pasting && !EdOpt.PersistentBlocks && BlockStart!=NULL)
          if (Key>=32 && Key<256 || Key==KEY_ADD || Key==KEY_SUBTRACT ||
              Key==KEY_MULTIPLY || Key==KEY_DIVIDE || Key==KEY_TAB)
          {
            DeleteBlock();
            Show();
          }

        int SkipCheckUndo=(Key==KEY_RIGHT || Key==KEY_CTRLLEFT ||
            Key==KEY_CTRLRIGHT || Key==KEY_HOME || Key==KEY_END ||
            Key==KEY_CTRLS);

        if (LockMode && !SkipCheckUndo)
          return(TRUE);

        if (Key==KEY_CTRLLEFT && CurLine->EditLine.GetCurPos()==0)
        {
          Pasting++;
          ProcessKey(KEY_LEFT);
          Pasting--;
          /* $ 24.9.2001 SKV
            fix бага с ctrl-left в начале строки
            в блоке с переопределённым плагином фоном.
          */
          ShowEditor(FALSE);
          //CtrlObject->Plugins.CurEditor=this;
//_D(SysLog("%08d EE_REDRAW",__LINE__));
          //CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
          /* SKV$*/
          return(TRUE);
        }

        if ((!EdOpt.CursorBeyondEOL && Key==KEY_RIGHT || Key==KEY_CTRLRIGHT) &&
            CurLine->EditLine.GetCurPos()>=CurLine->EditLine.GetLength() &&
            CurLine->Next!=NULL)
        {
          Pasting++;
          ProcessKey(KEY_HOME);
          ProcessKey(KEY_DOWN);
          Pasting--;
          CtrlObject->Plugins.CurEditor=this;
//_D(SysLog("%08d EE_REDRAW",__LINE__));
//_SVS(SysLog("Editor::ProcessKey[%d](!EdOpt.CursorBeyondEOL): EE_REDRAW(EEREDRAW_ALL)",__LINE__));
          CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
          /*$ 03.02.2001 SKV
            А то EEREDRAW_ALL то уходит, а на самом деле
            только текущая линия перерисовывается.
          */
          ShowEditor(0);
          /* SKV$*/
          return(TRUE);
        }

        char *Str,*CmpStr;
        int Length,CurPos;

        CurLine->EditLine.GetBinaryString(&Str,NULL,Length);
        CurPos=CurLine->EditLine.GetCurPos();

        if (Key<256 && CurPos>0 && Length==0)
        {
          struct EditList *PrevLine=CurLine->Prev;
          while (PrevLine!=NULL && PrevLine->EditLine.GetLength()==0)
            PrevLine=PrevLine->Prev;
          if (PrevLine!=NULL)
          {
            int TabPos=CurLine->EditLine.GetTabCurPos();
            CurLine->EditLine.SetCurPos(0);
            char *PrevStr;
            int PrevLength;
            PrevLine->EditLine.GetBinaryString(&PrevStr,NULL,PrevLength);
            for (int I=0;I<PrevLength && isspace(PrevStr[I]);I++)
            {
              int NewTabPos=CurLine->EditLine.GetTabCurPos();
              if (NewTabPos==TabPos)
                break;
              if (NewTabPos>TabPos)
              {
                CurLine->EditLine.ProcessKey(KEY_BS);
                while (CurLine->EditLine.GetTabCurPos()<TabPos)
                  CurLine->EditLine.ProcessKey(' ');
                break;
              }
              if (NewTabPos<TabPos)
                CurLine->EditLine.ProcessKey(PrevStr[I]);
            }
            CurLine->EditLine.SetTabCurPos(TabPos);
          }
        }

        if (!SkipCheckUndo)
        {
          CurLine->EditLine.GetBinaryString(&Str,NULL,Length);
          CurPos=CurLine->EditLine.GetCurPos();
          CmpStr=new char[Length+1];
          memcpy(CmpStr,Str,Length);
          CmpStr[Length]=0;
        }

        int LeftPos=CurLine->EditLine.GetLeftPos();

        /* $ 24.09.2000 SVS
           Вызов функции Xlat
        */
        /* $ 04.11.2000 SVS
           Проверка на альтернативную клавишу
        */
        /* $ 25.11.2000 IS
           Теперь Xlat работает даже при отсутствии выделения
        */
        if((Opt.XLat.XLatEditorKey && Key == Opt.XLat.XLatEditorKey ||
            Opt.XLat.XLatAltEditorKey && Key == Opt.XLat.XLatAltEditorKey) ||
            Key == KEY_MACROXLAT)
        /* IS  $ */
        {
          Xlat();
          Show();
          return TRUE;
        }
        /* SVS $ */
        /* SVS $ */

        if (CurLine->EditLine.ProcessKey(Key))
        {
          if (!SkipCheckUndo)
          {
            char *NewCmpStr;
            int NewLength;
            CurLine->EditLine.GetBinaryString(&NewCmpStr,NULL,NewLength);
            if (NewLength!=Length || memcmp(CmpStr,NewCmpStr,Length)!=0)
            {
              AddUndoData(CmpStr,NumLine,CurPos,UNDO_EDIT);
              /*$ 10.08.2000 skv
                Modified->TextChanged
              */
              TextChanged(1);
              /* skv $*/
            }
            delete[] CmpStr;
          }
          ShowEditor(LeftPos==CurLine->EditLine.GetLeftPos());
          return(TRUE);
        }
        else
          if (!SkipCheckUndo)
            delete[] CmpStr;
        if (VBlockStart!=NULL)
          Show();
      }
      return(FALSE);
  }
}


int Editor::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  struct EditList *NewPtr;
  int NewDist,Dist;
/* $ 28.12.2000 VVM
  + Щелчок мышкой снимает непостоянный блок всегда */
  if ((MouseEvent->dwButtonState & 3)!=0)
  {
    MarkingBlock=MarkingVBlock=FALSE;
    if ((!EdOpt.PersistentBlocks) && (BlockStart!=NULL || VBlockStart!=NULL))
    {
      UnmarkBlock();
      Show();
    } /* if */
  } /* if */
  if (CurLine->EditLine.ProcessMouse(MouseEvent))
  {
    ShowStatus();
    if (VBlockStart!=NULL)
      Show();
    else
    {
      CtrlObject->Plugins.CurEditor=this;
//_D(SysLog("%08d EE_REDRAW",__LINE__));
      CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_LINE);
    }
    return(TRUE);
  }
  if ((MouseEvent->dwButtonState & 3)==0)
    return(FALSE);
/* VVM $ */
  if (MouseEvent->dwMousePosition.Y==Y1)
  {
    while (IsMouseButtonPressed() && MouseY==Y1)
      ProcessKey(KEY_UP);
    return(TRUE);
  }
  if (MouseEvent->dwMousePosition.Y==Y2+1)
  {
    while (IsMouseButtonPressed() && MouseY==Y2+1)
      ProcessKey(KEY_DOWN);
    return(TRUE);
  }
  if (MouseEvent->dwMousePosition.X<X1 || MouseEvent->dwMousePosition.X>X2 ||
      MouseEvent->dwMousePosition.Y<=Y1 || MouseEvent->dwMousePosition.Y>Y2)
    return(FALSE);
  NewDist=MouseEvent->dwMousePosition.Y-Y1-1;
  NewPtr=TopScreen;
  while (NewDist-- && NewPtr->Next)
    NewPtr=NewPtr->Next;

  Dist=CalcDistance(TopScreen,NewPtr,-1)-CalcDistance(TopScreen,CurLine,-1);

  if (Dist>0)
    while (Dist--)
      Down();
  else
    while (Dist++)
      Up();
  CurLine->EditLine.ProcessMouse(MouseEvent);
  Show();
  return(TRUE);
}


int Editor::CalcDistance(struct EditList *From,struct EditList *To,int MaxDist)
{
  int Distance=0;
  while (From!=To && From->Next!=NULL && (MaxDist==-1 || MaxDist-- > 0))
  {
    Distance++;
    From=From->Next;
  }
  return(Distance);
}



void Editor::DeleteString(struct EditList *DelPtr,int DeleteLast,int UndoLine)
{
  if (LockMode)
    return;
  /* $ 16.12.2000 OT
     CtrlY на последней строке с выделенным вертикальным блоком не снимал выделение */
  if (VBlockStart!=NULL && NumLine<VBlockY+VBlockSizeY)
    if (NumLine<VBlockY)
    {
      if (VBlockY>0)
      {
        VBlockY--;
        BlockStartLine--;
      }
    }
    else
      if (--VBlockSizeY<=0)
        VBlockStart=NULL;
  /* OT $ */

  /*$ 10.08.2000 skv
    Modified->TextChanged
  */
  TextChanged(1);
  /* skv $*/
  if (DelPtr->Next==NULL && (!DeleteLast || DelPtr->Prev==NULL))
  {
    AddUndoData(DelPtr->EditLine.GetStringAddr(),UndoLine,
                DelPtr->EditLine.GetCurPos(),UNDO_EDIT);
    DelPtr->EditLine.SetString("");
    return;
  }

  for (int I=0;I<sizeof(SavePos.Line)/sizeof(SavePos.Line[0]);I++)
    if (SavePos.Line[I]!=0xffffffff && UndoLine<SavePos.Line[I])
      SavePos.Line[I]--;

  NumLastLine--;

  if (CurLine==DelPtr)
  {
    int LeftPos,CurPos;
    CurPos=DelPtr->EditLine.GetTabCurPos();
    LeftPos=DelPtr->EditLine.GetLeftPos();
    if (DelPtr->Next!=NULL)
      CurLine=DelPtr->Next;
    else
      CurLine=DelPtr->Prev;
    CurLine->EditLine.SetLeftPos(LeftPos);
    CurLine->EditLine.SetTabCurPos(CurPos);
  }

  if (DelPtr->Prev)
  {
    DelPtr->Prev->Next=DelPtr->Next;
    if (DelPtr==EndList)
      EndList=EndList->Prev;
  }
  if (DelPtr->Next!=NULL)
    DelPtr->Next->Prev=DelPtr->Prev;
  if (DelPtr==TopScreen)
    if (TopScreen->Next!=NULL)
      TopScreen=TopScreen->Next;
    else
      TopScreen=TopScreen->Prev;
  if (DelPtr==TopList)
    TopList=TopList->Next;
  if (DelPtr==BlockStart)
    BlockStart=BlockStart->Next;
  if (DelPtr==VBlockStart)
    VBlockStart=VBlockStart->Next;
  if (UndoLine!=-1)
    AddUndoData(DelPtr->EditLine.GetStringAddr(),UndoLine,0,UNDO_DELSTR);
  delete DelPtr;
}


void Editor::InsertString()
{
  if (LockMode)
    return;
  /*$ 10.08.2000 skv
    There is only one return - if new will fail.
    In this case things are realy bad.
    Move TextChanged to the end of functions
    AFTER all modifications are made.
  */
//  TextChanged(1);
  /* skv $*/
  struct EditList *NewString;
  struct EditList *SrcIndent=NULL;
  int SelStart,SelEnd;
  int CurPos;
  /* $ 17.07.2000 tran
     + новая переменная */
  int NewLineEmpty=TRUE;
  /* tran 17.07.2000 $ */

  if ((NewString=new struct EditList)==NULL)
    return;

  NewString->EditLine.SetObjectColor(COL_EDITORTEXT,COL_EDITORSELECTEDTEXT);
  /* $ 14.02.2001 IS
       Установим нужный размер табуляции
  */
  NewString->EditLine.SetTabSize(EdOpt.TabSize);
  /* IS $ */
  NewString->EditLine.SetConvertTabs(EdOpt.ExpandTabs);
  NewString->EditLine.SetTables(UseDecodeTable ? &TableSet:NULL);
  NewString->EditLine.SetEditBeyondEnd(EdOpt.CursorBeyondEOL);
  NewString->EditLine.SetEditorMode(TRUE);
  NewString->Prev=CurLine;
  NewString->Next=CurLine->Next;
  if (CurLine->Next)
    CurLine->Next->Prev=NewString;
  CurLine->Next=NewString;
  char *CurLineStr,*EndSeq;
  int Length;
  CurLine->EditLine.GetBinaryString(&CurLineStr,&EndSeq,Length);


  CurPos=CurLine->EditLine.GetCurPos();
  CurLine->EditLine.GetSelection(SelStart,SelEnd);

  for (int I=0;I<sizeof(SavePos.Line)/sizeof(SavePos.Line[0]);I++)
    if (SavePos.Line[I]!=0xffffffff &&
        (NumLine<SavePos.Line[I] || NumLine==SavePos.Line[I] && CurPos==0))
      SavePos.Line[I]++;

  int IndentPos=0;

  if (EdOpt.AutoIndent && !Pasting)
  {
    struct EditList *PrevLine=CurLine;
    while (PrevLine!=NULL)
    {
      char *Str;
      int Length,Found=FALSE;
      PrevLine->EditLine.GetBinaryString(&Str,NULL,Length);
      for (int I=0;I<Length;I++)
        /* $ 24.07.2001 IS isspace для этого и придумали */
        if (!isspace(Str[I]))
        /* IS $ */
        {
          PrevLine->EditLine.SetCurPos(I);
          IndentPos=PrevLine->EditLine.GetTabCurPos();
          SrcIndent=PrevLine;
          Found=TRUE;
          break;
        }
      if (Found)
        break;
      PrevLine=PrevLine->Prev;
    }
  }

  int SpaceOnly=TRUE;

  if (CurPos<Length)
  {


    /* $ 30.08.2000 tran
       раскоментировал код, как нужный.
    */
    /* $ 17.07.2000 tran
       - закоментировал код, как не нужный*/
    if (IndentPos>0)
      for (int I=0;I<CurPos;I++)
        /* $ 24.07.2001 IS isspace для этого и придумали */
        if (!isspace(CurLineStr[I]))
        /* IS $ */
        {
          SpaceOnly=FALSE;
          break;
        }
    /* tran 30.08.2000 $ */

    NewString->EditLine.SetBinaryString(&CurLineStr[CurPos],Length-CurPos);
    /* $ 17.07.2000 tran
       тут мы проверяем новую строку, есть ли на ней что нибудь кроме пробелов
    */
    for ( int i0=0; i0<Length-CurPos; i0++ )
    {
        /* $ 24.07.2001 IS isspace для этого и придумали */
        if (!isspace(CurLineStr[i0+CurPos]))
        /* IS $ */
        {
            NewLineEmpty=FALSE;
            break;
        }
    }
    /* tran 17.07.2000 $ */

    AddUndoData(CurLine->EditLine.GetStringAddr(),NumLine,
                CurLine->EditLine.GetCurPos(),UNDO_EDIT);
    BlockUndo++;
    AddUndoData(NULL,NumLine+1,0,UNDO_INSSTR);
    BlockUndo--;
    CurLineStr[CurPos]=0;
    int StrSize=CurPos;
    /* $ 17.07.2000 tran
       а тут в условие добавили проверку на нашу новую переменную */
    if (EdOpt.AutoIndent && NewLineEmpty)
    {
      RemoveTrailingSpaces(CurLineStr);
      StrSize=strlen(CurLineStr);
    }
    /* tran 17.07.2000 $ */

    CurLine->EditLine.SetBinaryString(CurLineStr,StrSize);
  }
  else
  {
    NewString->EditLine.SetString("");
    AddUndoData(NULL,NumLine+1,0,UNDO_INSSTR);
  }

  if (VBlockStart!=NULL && NumLine<VBlockY+VBlockSizeY)
    if (NumLine<VBlockY)
    {
      VBlockY++;
      BlockStartLine++;
    }
    else
      VBlockSizeY++;

  if (SelStart!=-1 && (SelEnd==-1 || CurPos<SelEnd))
  {
    if (CurPos>=SelStart)
    {
      CurLine->EditLine.Select(SelStart,-1);
      NewString->EditLine.Select(0,SelEnd==-1 ? -1:SelEnd-CurPos);
    }
    else
    {
      CurLine->EditLine.Select(-1,0);
      NewString->EditLine.Select(SelStart-CurPos,SelEnd==-1 ? -1:SelEnd-CurPos);
      BlockStart=NewString;
      BlockStartLine++;
    }
  }
  else
    if (BlockStart!=NULL && NumLine<BlockStartLine)
      BlockStartLine++;

  NewString->EditLine.SetEOL(EndSeq);

  CurLine->EditLine.SetCurPos(0);
  if (CurLine==EndList)
    EndList=NewString;
  NumLastLine++;
  Down();

  if (IndentPos>0)
  {
    int OrgIndentPos=IndentPos;
    ShowEditor(FALSE);

    CurLine->EditLine.GetBinaryString(&CurLineStr,NULL,Length);

    if (SpaceOnly)
    {
      int Decrement=0;
      for (int I=0;I<IndentPos && I<Length;I++)
      {
        /* $ 24.07.2001 IS isspace для этого и придумали */
        if (!isspace(CurLineStr[I]))
        /* IS $ */
          break;
        if (CurLineStr[I]==' ')
          Decrement++;
        else
        {
          int TabPos=CurLine->EditLine.RealPosToTab(I);
          Decrement+=EdOpt.TabSize - (TabPos % EdOpt.TabSize);
        }
      }
      IndentPos-=Decrement;
    }

    if (IndentPos>0)
    {
      if (CurLine->EditLine.GetLength()!=0 || !EdOpt.CursorBeyondEOL)
      {
        CurLine->EditLine.ProcessKey(KEY_HOME);

        int SaveOvertypeMode=CurLine->EditLine.GetOvertypeMode();
        CurLine->EditLine.SetOvertypeMode(FALSE);

        char *PrevStr;
        int PrevLength;

        if (SrcIndent)
        {
          SrcIndent->EditLine.GetBinaryString(&PrevStr,NULL,PrevLength);
        }

        for (int I=0;CurLine->EditLine.GetTabCurPos()<IndentPos;I++)
        {
          if (SrcIndent!=NULL && I<PrevLength && isspace(PrevStr[I]))
          {
            CurLine->EditLine.ProcessKey(PrevStr[I]);
          }
          else
          {
            CurLine->EditLine.ProcessKey(KEY_SPACE);
          }
        }
        while (CurLine->EditLine.GetTabCurPos()>IndentPos)
          CurLine->EditLine.ProcessKey(KEY_BS);

        CurLine->EditLine.SetOvertypeMode(SaveOvertypeMode);
      }
      CurLine->EditLine.SetTabCurPos(IndentPos);
    }

    CurLine->EditLine.GetBinaryString(&CurLineStr,NULL,Length);
    CurPos=CurLine->EditLine.GetCurPos();
    if (SpaceOnly)
    {
      int NewPos=0;
      for (int I=0;I<Length;I++)
      {
        NewPos=I;
        /* $ 24.07.2001 IS isspace для этого и придумали */
        if (!isspace(CurLineStr[I]))
        /* IS $ */
          break;
      }
      if (NewPos>OrgIndentPos)
        NewPos=OrgIndentPos;
      if (NewPos>CurPos)
        CurLine->EditLine.SetCurPos(NewPos);
    }
  }
  /*$ 10.08.2000 skv
    Modified->TextChanged
  */
  TextChanged(1);
  /* skv$*/

}



void Editor::Down()
{
  struct EditList *CurPtr;
  int LeftPos,CurPos,Y;
  if (CurLine->Next==NULL)
    return;
  for (Y=0,CurPtr=TopScreen;CurPtr!=CurLine;CurPtr=CurPtr->Next)
    Y++;
  if (Y>=Y2-Y1-1)
    TopScreen=TopScreen->Next;
  CurPos=CurLine->EditLine.GetTabCurPos();
  LeftPos=CurLine->EditLine.GetLeftPos();
  CurLine=CurLine->Next;
  NumLine++;
  CurLine->EditLine.SetLeftPos(LeftPos);
  CurLine->EditLine.SetTabCurPos(CurPos);
}


void Editor::ScrollDown()
{
  int LeftPos,CurPos;
  if (CurLine->Next==NULL || TopScreen->Next==NULL)
    return;
  if (CalcDistance(TopScreen,EndList,Y2-Y1)<Y2-Y1)
  {
    Down();
    return;
  }
  TopScreen=TopScreen->Next;
  CurPos=CurLine->EditLine.GetTabCurPos();
  LeftPos=CurLine->EditLine.GetLeftPos();
  CurLine=CurLine->Next;
  NumLine++;
  CurLine->EditLine.SetLeftPos(LeftPos);
  CurLine->EditLine.SetTabCurPos(CurPos);
}


void Editor::Up()
{
  int LeftPos,CurPos;
  if (CurLine->Prev==NULL)
    return;

  if (CurLine==TopScreen)
    TopScreen=TopScreen->Prev;

  CurPos=CurLine->EditLine.GetTabCurPos();
  LeftPos=CurLine->EditLine.GetLeftPos();
  CurLine=CurLine->Prev;
  NumLine--;
  CurLine->EditLine.SetLeftPos(LeftPos);
  CurLine->EditLine.SetTabCurPos(CurPos);
}


void Editor::ScrollUp()
{
  int LeftPos,CurPos;
  if (CurLine->Prev==NULL)
    return;
  if (TopScreen->Prev==NULL)
  {
    Up();
    return;
  }

  TopScreen=TopScreen->Prev;
  CurPos=CurLine->EditLine.GetTabCurPos();
  LeftPos=CurLine->EditLine.GetLeftPos();
  CurLine=CurLine->Prev;
  NumLine--;
  CurLine->EditLine.SetLeftPos(LeftPos);
  CurLine->EditLine.SetTabCurPos(CurPos);
}

/* $ 21.01.2001 SVS
   Диалоги поиска/замены выведен из Editor::Search
   в отдельную функцию GetSearchReplaceString
   (файл stddlg.cpp)
*/
BOOL Editor::Search(int Next)
{
  struct EditList *CurPtr;
  unsigned char SearchStr[SEARCHSTRINGBUFSIZE],ReplaceStr[SEARCHSTRINGBUFSIZE];
  static char LastReplaceStr[SEARCHSTRINGBUFSIZE];
  static int LastSuccessfulReplaceMode=0;
  char MsgStr[512];
  const char *TextHistoryName="SearchText",*ReplaceHistoryName="ReplaceText";
  /* $ 03.08.2000 KM
     Новая переменная
  */
  int CurPos,Count,Case,WholeWords,ReverseSearch,Match,NewNumLine,UserBreak;
  /* KM $ */
  if (Next && *LastSearchStr==0)
    return TRUE;

  strncpy((char *)SearchStr,(char *)LastSearchStr,sizeof(SearchStr)-1);
  strncpy((char *)ReplaceStr,(char *)LastReplaceStr,sizeof(ReplaceStr)-1);
  Case=LastSearchCase;
  WholeWords=LastSearchWholeWords;
  ReverseSearch=LastSearchReverse;

  if (!Next)
    if(!GetSearchReplaceString(ReplaceMode,SearchStr,sizeof(SearchStr),
                   ReplaceStr,sizeof(ReplaceStr),
                   TextHistoryName,ReplaceHistoryName,
                   &Case,&WholeWords,&ReverseSearch))
      return FALSE;

  strncpy((char *)LastSearchStr,(char *)SearchStr,sizeof(LastSearchStr)-1);
  strncpy((char *)LastReplaceStr,(char *)ReplaceStr,sizeof(LastReplaceStr)-1);
  LastSearchCase=Case;
  LastSearchWholeWords=WholeWords;
  LastSearchReverse=ReverseSearch;

  if (*SearchStr==0)
    return TRUE;

  LastSuccessfulReplaceMode=ReplaceMode;

  if (!EdOpt.PersistentBlocks)
    UnmarkBlock();

  {
    //SaveScreen SaveScr;

    int SearchLength=strlen((char *)SearchStr);

    sprintf(MsgStr,"\"%s\"",SearchStr);
    SetCursorType(FALSE,0);
    SetPreRedrawFunc(Editor::PR_EditorShowMsg);
    EditorShowMsg(MSG(MEditSearchTitle),MSG(MEditSearchingFor),MsgStr);

    Count=0;
    Match=0;
    UserBreak=0;
    CurPos=CurLine->EditLine.GetCurPos();
    /* $ 16.10.2000 tran
       CurPos увеличивается при следующем поиске */
    /* $ 28.11.2000 SVS
       "О, это не ощибка - это свойство моей программы" :-)
       Новое поведение стало подконтрольным
    */
    /* $ 21.12.2000 SVS
       - В предыдущем исправлении было задано неверное условие для
         правила EditorF7Rules
    */
    /* $ 10.06.2001 IS
       - Баг: зачем-то при продолжении _обратного_ поиска прокручивались на шаг
         _вперед_.
    */
    /* $ 09.11.2001 IS
         проклятое место, блин.
         опять фиксим, т.к. не соответствует заявленному
    */
    if( !ReverseSearch && ( Next || (Opt.EditorF7Rules && !ReplaceMode) ) )
        CurPos++;
    /* IS $ */
    /* IS $ */
    /* SVS $ */
    /* SVS $ */
    /* tran $ */

    NewNumLine=NumLine;
    CurPtr=CurLine;

    while (CurPtr!=NULL)
    {
      if ((++Count & 0xfff)==0 && CheckForEsc())
      {
        UserBreak=TRUE;
        break;
      }
      /* $ 03.08.2000 KM
         Добавление нового параметра в функцию поиска
      */
      if (CurPtr->EditLine.Search((char *)SearchStr,CurPos,Case,WholeWords,ReverseSearch))
      /* KM $ */
      {
        int Skip=FALSE;
        TopScreen=CurLine=CurPtr;
        NumLine=NewNumLine;

        int LeftPos=CurPtr->EditLine.GetLeftPos();
        int TabCurPos=CurPtr->EditLine.GetTabCurPos();
        if (ObjWidth>8 && TabCurPos-LeftPos+SearchLength>ObjWidth-8)
          CurPtr->EditLine.SetLeftPos(TabCurPos+SearchLength-ObjWidth+8);

        if (ReplaceMode)
        {
          int MsgCode=0;
          if (!ReplaceAll)
          {
            Show();
            int CurX,CurY;
            GetCursorPos(CurX,CurY);
            GotoXY(CurX,CurY);
            SetColor(COL_EDITORSELECTEDTEXT);
            const char *Str=CurPtr->EditLine.GetStringAddr()+CurPtr->EditLine.GetCurPos();
            char *TmpStr=new char[SearchLength+1];
            strncpy(TmpStr,Str,SearchLength);
            TmpStr[SearchLength]=0;
            if (UseDecodeTable)
              DecodeString(TmpStr,(unsigned char *)TableSet.DecodeTable);
            Text(TmpStr);
            delete[] TmpStr;

            char QSearchStr[SEARCHSTRINGBUFSIZE+4],QReplaceStr[SEARCHSTRINGBUFSIZE+4];
            sprintf(QSearchStr,"\"%s\"",LastSearchStr);
            sprintf(QReplaceStr,"\"%s\"",LastReplaceStr);
            MsgCode=Message(0,4,MSG(MEditReplaceTitle),MSG(MEditAskReplace),
              QSearchStr,MSG(MEditAskReplaceWith),QReplaceStr,
              MSG(MEditReplace),MSG(MEditReplaceAll),MSG(MEditSkip),MSG(MEditCancel));
            if (MsgCode==1)
              ReplaceAll=TRUE;
            if (MsgCode==2)
              Skip=TRUE;
            if (MsgCode<0 || MsgCode==3)
            {
              UserBreak=TRUE;
              break;
            }
          }
          if (MsgCode==0 || MsgCode==1)
          {
            Pasting++;
            /*$ 15.08.2000 skv
              If Replace string doesn't contain control symbols (tab and return),
              processed with fast method, otherwise use improved old one.
            */
            if(strchr((char*)ReplaceStr,'\t') || strchr((char*)ReplaceStr,13))
            {
              int SaveOvertypeMode=Overtype;
              Overtype=TRUE;
              CurLine->EditLine.SetOvertypeMode(TRUE);
              //int CurPos=CurLine->EditLine.GetCurPos();
              int I;
              for (I=0;SearchStr[I]!=0 && ReplaceStr[I]!=0;I++)
              {
                int Ch=ReplaceStr[I];
                if (Ch==KEY_TAB)
                {
                  Overtype=FALSE;
                  CurLine->EditLine.SetOvertypeMode(FALSE);
                  ProcessKey(KEY_DEL);
                  ProcessKey(KEY_TAB);
                  Overtype=TRUE;
                  CurLine->EditLine.SetOvertypeMode(TRUE);
                  continue;
                }
                if (Ch!=KEY_BS && Ch!=KEY_DEL)
                  ProcessKey(Ch);
              }
              if(SearchStr[I]==0)
              {
                Overtype=FALSE;
                CurLine->EditLine.SetOvertypeMode(FALSE);
                for (;ReplaceStr[I]!=0;I++)
                {
                  int Ch=ReplaceStr[I];
                  if (Ch!=KEY_BS && Ch!=KEY_DEL)
                    ProcessKey(Ch);
                }
              }else
              {
                for (;SearchStr[I]!=0;I++)
                {
                  ProcessKey(KEY_DEL);
                }
              }
              int Cnt=0;
              char *Tmp=(char*)ReplaceStr;
              while((Tmp=strchr(Tmp,13)) != NULL)
              {
                Cnt++;
                Tmp++;
              }
              if(Cnt>0)
              {
                CurPtr=CurLine;
                NewNumLine+=Cnt;
              }
              Overtype=SaveOvertypeMode;
            }
            else
            {
              /* Fast method */
              char *Str,*Eol;
              int StrLen,NewStrLen;
              int SStrLen=strlen((char*)SearchStr),
                  RStrLen=strlen((char*)ReplaceStr);
              CurLine->EditLine.GetBinaryString(&Str,&Eol,StrLen);
              int EolLen=strlen((char*)Eol);
              NewStrLen=StrLen;
              NewStrLen-=SStrLen;
              NewStrLen+=RStrLen;
              NewStrLen+=EolLen;
              char *NewStr=new char[NewStrLen+1];
              int CurPos=CurLine->EditLine.GetCurPos();
              memcpy(NewStr,Str,CurPos);
              memcpy(NewStr+CurPos,ReplaceStr,RStrLen);
              /*$ 07.09.2000 skv
                If table set need to encode string.
              */
              if(UseDecodeTable)
              {
                EncodeString(NewStr+CurPos,(unsigned char*)TableSet.EncodeTable,RStrLen);
              }
              /* skv$*/
              memcpy(NewStr+CurPos+RStrLen,Str+CurPos+SStrLen,StrLen-CurPos-SStrLen);
              memcpy(NewStr+NewStrLen-EolLen,Eol,EolLen);
              AddUndoData(CurLine->EditLine.GetStringAddr(),NumLine,
                          CurLine->EditLine.GetCurPos(),UNDO_EDIT);
              CurLine->EditLine.SetBinaryString(NewStr,NewStrLen);
              CurLine->EditLine.SetCurPos(CurPos+RStrLen);
              delete [] NewStr;

              TextChanged(1);
            }
            /* skv$*/

            if (ReverseSearch)
              CurLine->EditLine.SetCurPos(CurPos);
            Pasting--;
          }
        }
        Match=1;
        if (!ReplaceMode)
          break;
        CurPos=CurLine->EditLine.GetCurPos();
        if (Skip)
          if (!ReverseSearch)
            CurPos++;
      }
      else
        if (ReverseSearch)
        {
          CurPtr=CurPtr->Prev;
          if (CurPtr==NULL)
            break;
          CurPos=CurPtr->EditLine.GetLength();
          NewNumLine--;
        }
        else
        {
          CurPos=0;
          CurPtr=CurPtr->Next;
          NewNumLine++;
        }
    }
    SetPreRedrawFunc(NULL);
  }
  Show();
  if (!Match && !UserBreak)
    Message(MSG_DOWN|MSG_WARNING,1,MSG(MEditSearchTitle),MSG(MEditNotFound),
            MsgStr,MSG(MOk));
   return TRUE;
}
/* SVS $ */

void Editor::Paste(char *Src)
{
  if (LockMode)
    return;

  char *ClipText=Src;
  BOOL IsDeleteClipText=FALSE;

  if(!ClipText)
  {
    if ((ClipText=PasteFormatFromClipboard("FAR_VerticalBlock"))!=NULL)
    {
      VPaste(ClipText);
      return;
    }
    if ((ClipText=PasteFromClipboard())==NULL)
      return;
    IsDeleteClipText=TRUE;
  }

  if (*ClipText)
  {
    NewUndo=TRUE;
    if (UseDecodeTable)
      EncodeString(ClipText,(unsigned char *)TableSet.EncodeTable);
    /*$ 10.08.2000 skv
      Modified->TextChanged
    */
    TextChanged(1);
    /* skv $*/
    int SaveOvertype=Overtype;
    UnmarkBlock();
    Pasting++;
    DisableOut++;
    Edit::DisableEditOut(TRUE);
    if (Overtype)
    {
      Overtype=FALSE;
      CurLine->EditLine.SetOvertypeMode(FALSE);
    }
    BlockStart=CurLine;
    BlockStartLine=NumLine;
    /* $ 19.05.2001 IS
       Решение проблемы непрошеной конвертации табуляции (которая должна быть
       добавлена в начало строки при автоотступе) в пробелы.
    */
    int StartPos=CurLine->EditLine.GetCurPos(),
        oldAutoIndent=EdOpt.AutoIndent;

    for (int I=0;ClipText[I]!=0;)
      if (ClipText[I]!=10)
        if (ClipText[I]==13)
        {
          CurLine->EditLine.Select(StartPos,-1);
          StartPos=0;
          EdOpt.AutoIndent=FALSE;
          ProcessKey(KEY_ENTER);
          BlockUndo=TRUE;
          I++;
        }
        else
        {
          if(EdOpt.AutoIndent)       // первый символ вставим так, чтобы
          {                          // сработал автоотступ
            /* $ 25.05.2001 IS
                 Корректно обработаем вставку в файл в кодировке,
                 отличной от oem
            */
            ProcessKey(UseDecodeTable?TableSet.DecodeTable[(unsigned)ClipText[I]]:ClipText[I]);
            /* IS $ */
            I++;
            StartPos=CurLine->EditLine.GetCurPos();
            if(StartPos) StartPos--;
          }

          int Pos=I;
          while (ClipText[Pos]!=0 && ClipText[Pos]!=10 && ClipText[Pos]!=13)
            Pos++;
          if (Pos>I)
          {
            char *Str;
            int Length,CurPos;
            CurLine->EditLine.GetBinaryString(&Str,NULL,Length);
            CurPos=CurLine->EditLine.GetCurPos();
            AddUndoData(Str,NumLine,CurPos,UNDO_EDIT);
            BlockUndo=TRUE;
            CurLine->EditLine.InsertBinaryString(&ClipText[I],Pos-I);
          }
          I=Pos;
        }
      else
        I++;

    EdOpt.AutoIndent=oldAutoIndent;

    CurLine->EditLine.Select(StartPos,CurLine->EditLine.GetCurPos());
    /* IS $ */

    if (SaveOvertype)
    {
      Overtype=TRUE;
      CurLine->EditLine.SetOvertypeMode(TRUE);
    }

    Edit::DisableEditOut(FALSE);
    Pasting--;
    DisableOut--;
  }
  /* $ 07.05.2001 IS выделяли же в PasteFromClipboard как new [] */
  if(IsDeleteClipText)
    delete [] ClipText;
  /* IS $ */
  BlockUndo=FALSE;
}


void Editor::Copy(int Append)
{
  if (VBlockStart!=NULL)
  {
    VCopy(Append);
    return;
  }

  struct EditList *CurPtr=BlockStart;
  char *CopyData=NULL;
  long DataSize=0,PrevSize=0;

  if (Append)
  {
    CopyData=PasteFromClipboard();
    if (CopyData!=NULL)
      PrevSize=DataSize=strlen(CopyData);
  }

  while (CurPtr!=NULL)
  {
    int StartSel,EndSel;
    int Length=CurPtr->EditLine.GetLength()+1;
    CurPtr->EditLine.GetSelection(StartSel,EndSel);
    if (StartSel==-1)
      break;
    char *NewPtr=(char *)realloc(CopyData,DataSize+Length+2);
    if (NewPtr==NULL)
    {
      delete CopyData;
      CopyData=NULL;
      break;
    }
    CopyData=NewPtr;
    CurPtr->EditLine.GetSelString(CopyData+DataSize,Length);
    DataSize+=strlen(CopyData+DataSize);
    if (EndSel==-1)
    {
      strcpy(CopyData+DataSize,"\r\n");
      DataSize+=2;
    }
    CurPtr=CurPtr->Next;
  }

  if (CopyData!=NULL)
  {
    if (UseDecodeTable)
      DecodeString(CopyData+PrevSize,(unsigned char *)TableSet.DecodeTable);
    CopyToClipboard(CopyData);
    delete CopyData;
  }
}


void Editor::DeleteBlock()
{
  if (LockMode)
    return;

  if (VBlockStart!=NULL)
  {
    DeleteVBlock();
    return;
  }

  struct EditList *CurPtr=BlockStart;

  int UndoNext=FALSE;

  while (CurPtr!=NULL)
  {
    /*$ 10.08.2000 skv
      Modified->TextChanged
    */
    TextChanged(1);
    /* skv $*/
    int StartSel,EndSel;
    CurPtr->EditLine.GetSelection(StartSel,EndSel);
    if (StartSel==-1)
      break;
    if (StartSel==0 && EndSel==-1)
    {
      struct EditList *NextLine=CurPtr->Next;
      BlockUndo=UndoNext;
      DeleteString(CurPtr,FALSE,BlockStartLine);
      UndoNext=TRUE;
      if (BlockStartLine<NumLine)
        NumLine--;
      if (NextLine!=NULL)
      {
        CurPtr=NextLine;
        continue;
      }
      else
        break;
    }
    int Length=CurPtr->EditLine.GetLength();
    char *TmpStr=new char[Length+3];
    if (StartSel!=0 || EndSel!=0)
    {
      BlockUndo=UndoNext;
      AddUndoData(CurPtr->EditLine.GetStringAddr(),BlockStartLine,
                  CurPtr->EditLine.GetCurPos(),UNDO_EDIT);
      UndoNext=TRUE;
    }
    char *CurStr,*EndSeq;
    CurPtr->EditLine.GetBinaryString(&CurStr,&EndSeq,Length);
    memcpy(TmpStr,CurStr,Length);
    TmpStr[Length]=0;
    int DeleteNext=FALSE;
    if (EndSel==-1)
    {
      EndSel=Length;
      if (CurPtr->Next!=NULL)
        DeleteNext=TRUE;
    }
    memmove(TmpStr+StartSel,TmpStr+EndSel,strlen(TmpStr+EndSel)+1);
    int CurPos=StartSel;
/*    if (CurPos>=StartSel)
    {
      CurPos-=(EndSel-StartSel);
      if (CurPos<StartSel)
        CurPos=StartSel;
    }
*/
    Length-=EndSel-StartSel;
    if (DeleteNext)
    {
      char *NextStr,*EndSeq;
      int NextLength,NextStartSel,NextEndSel;
      CurPtr->Next->EditLine.GetSelection(NextStartSel,NextEndSel);
      if (NextStartSel==-1)
        NextEndSel=0;
      if (NextEndSel==-1)
        EndSel=-1;
      else
      {
        CurPtr->Next->EditLine.GetBinaryString(&NextStr,&EndSeq,NextLength);
        NextLength-=NextEndSel;
        TmpStr=(char *)realloc(TmpStr,Length+NextLength+3);
        memcpy(TmpStr+Length,NextStr+NextEndSel,NextLength);
        Length+=NextLength;
      }
      if (CurLine==CurPtr->Next)
      {
        CurLine=CurPtr;
        NumLine--;
      }

      BlockUndo=UndoNext;
      if (CurLine==CurPtr && CurPtr->Next!=NULL && CurPtr->Next==TopScreen)
      {
        CurLine=CurPtr->Next;
        NumLine++;
      }
      DeleteString(CurPtr->Next,FALSE,BlockStartLine+1);
      UndoNext=TRUE;
      if (BlockStartLine+1<NumLine)
        NumLine--;
    }
    int EndLength=strlen(EndSeq);
    memcpy(TmpStr+Length,EndSeq,EndLength);
    Length+=EndLength;
    TmpStr[Length]=0;
    CurPtr->EditLine.SetBinaryString(TmpStr,Length);
    delete[] TmpStr;
    CurPtr->EditLine.SetCurPos(CurPos);
    if (DeleteNext && EndSel==-1)
    {
      CurPtr->EditLine.Select(CurPtr->EditLine.GetLength(),-1);
    }
    else
    {
      CurPtr->EditLine.Select(-1,0);
      CurPtr=CurPtr->Next;
      BlockStartLine++;
    }
  }
  BlockStart=NULL;
  BlockUndo=FALSE;
}


void Editor::UnmarkBlock()
{
  if (BlockStart==NULL && VBlockStart==NULL)
    return;
  VBlockStart=NULL;
  MarkingBlock=FALSE;
  MarkingVBlock=FALSE;
  while (BlockStart!=NULL)
  {
    int StartSel,EndSel;
    BlockStart->EditLine.GetSelection(StartSel,EndSel);
    if (StartSel==-1)
      break;
    BlockStart->EditLine.Select(-1,0);
    BlockStart=BlockStart->Next;
  }
  BlockStart=NULL;
  Show();
}


/* $ 07.07.2000 tran & SVS
   + добавлена возможность переходить на колонку
     по формату [!][ROW][,COL]
     вынужден был изменить тип возвращаемого значения с void на int
     не хотелось вводить переменную в класс
     '!' - задает относительное смещение (пока не реализовано ;-)
*/
/* $ 21.07.2000 tran
   GotoLine стала воид и не выводит диалогов */
void Editor::GoToLine(int Line)
{
  int NewLine;

  NewLine=Line;

  int LastNumLine=NumLine;
  int CurScrLine=CalcDistance(TopScreen,CurLine,-1);
  for (NumLine=0,CurLine=TopList;
         NumLine<NewLine && CurLine->Next!=NULL;
         NumLine++)
    CurLine=CurLine->Next;
  CurScrLine+=NumLine-LastNumLine;

  if (CurScrLine<0 || CurScrLine>=Y2-Y1)
    TopScreen=CurLine;

  Show();
  return ;
}
/* tran 21.07.2000 $ */

/* $ 07.07.2000 tran & SVS
   + добавлена возможность переходить на колонку
     по формату [!][ROW][,COL]
     вынужден был изменить тип возвращаемого значения с void на int
     не хотелось вводить переменную в класс
     '!' - задает относительное смещение (пока не реализовано ;-)
*/
/* $ 21.07.2000 tran
   диалог из GotoLine перекочевал сюда */
void Editor::GoToPosition()
{
  int NewLine, NewCol;
  int LeftPos=CurLine->EditLine.GetTabCurPos()+1;
  int CurPos;
  CurPos=CurLine->EditLine.GetCurPos();

  const char *LineHistoryName="LineNumber";
  static struct DialogData GoToDlgData[]=
  {
    DI_DOUBLEBOX,3,1,21,3,0,0,0,0,(char *)MEditGoToLine,
    DI_EDIT,5,2,19,2,1,(DWORD)LineHistoryName,DIF_HISTORY|DIF_USELASTHISTORY,1,"",
  };
  MakeDialogItems(GoToDlgData,GoToDlg);
  /* $ 01.08.2000 tran
    PrevLine теперь не нужно - USELASTHISTORY рулит */
  //  static char PrevLine[40]={0};

  //  strcpy(GoToDlg[1].Data,PrevLine);
  Dialog Dlg(GoToDlg,sizeof(GoToDlg)/sizeof(GoToDlg[0]));
  Dlg.SetPosition(-1,-1,25,5);
  Dlg.SetHelp("EditorGotoPos");
  Dlg.Process();
    // tran: was if (Dlg.GetExitCode()!=1 || !isdigit(*GoToDlg[1].Data))
  if (Dlg.GetExitCode()!=1 )
      return ;
  // Запомним ранее введенное значение в текущем сеансе работы FAR`а
  //  strncpy(PrevLine,GoToDlg[1].Data,sizeof(PrevLine));

  GetRowCol(GoToDlg[1].Data,&NewLine,&NewCol);

  //_D(SysLog("GoToPosition: NewLine=%i, NewCol=%i",NewLine,NewCol));
  GoToLine(NewLine);

  if ( NewCol == -1)
  {
    CurLine->EditLine.SetTabCurPos(CurPos);
    CurLine->EditLine.SetLeftPos(LeftPos);
  }
  else
    CurLine->EditLine.SetTabCurPos(NewCol);

  Show();
  return ;
}
/* tran 07.07.2000 $ */
/* tran 21.07.2000 $ */


/* $ 07.07.2000 tran & SVS
   function for AltF8 user answer parsing
   Возвращает:
      TRUE  - абсолютное смещение
      FALSE - относительное
*/
/* $ 21.07.2000 tran
   теперь ничего не возвращает
   просто сама определяет относительность
   и вычисляет новые координаты */
void Editor::GetRowCol(char *argv,int *row,int *col)
{
  int x=0xffff,y,l;
  char *argvx=0;
  int LeftPos=CurLine->EditLine.GetTabCurPos() + 1;

  // что бы не оставить "врагу" выбора - только то, что мы хотим ;-)
  // "прибьем" все внешние пробелы.
  RemoveExternalSpaces(argv);

  // получаем индекс вхождения любого разделителя
  // в искомой строке
  l=strcspn(argv,",:;. ");
  // если разделителя нету, то l=strlen(argv)

  if(l < strlen(argv)) // Варианты: "row,col" или ",col"?
  {
    argv[l]='\0'; // Вместо разделителя впиндюлим "конец строки" :-)
    argvx=argv+l+1;
    x=atoi(argvx);
  }
  y=atoi(argv);
  /* $ 14.07.2000 tran
    + переход на проценты */
  if ( strchr(argv,'%')!=0 )
    y=NumLastLine * y / 100;
  /* tran $ */

  /* $ 21.07.2000 tran
     вычисляем относительность */
  if ( argv[0]=='-' || argv[0]=='+' )
    y=NumLine+y+1;
  if ( argvx )
  {
    if ( argvx[0]=='-' || argvx[0]=='+' )
    {
        x=LeftPos+x;
    }
  }

  /* tran 21.07.2000 $ */

  // теперь загоним результат назад
  *row=y;
  if ( x!=0xffff )
    *col=x;
  else
    /* $ 28.03.2001 VVM
      ! Зачем-то прибавляли 1. И сдвигались вправо... */
    *col=LeftPos;
    /* VVM $ */


  (*row)--;
  if (*row< 0)   // если ввели ",Col"
     *row=NumLine;  //   то переходим на текущую строку и колонку
  (*col)--;
  if (*col< -1)
     *col=-1;
  return ;
}
/* tran 07.07.2000 $ */

void Editor::SetEditKeyBar(KeyBar *EditKeyBar)
{
  Editor::EditKeyBar=EditKeyBar;
  ChangeEditKeyBar();
}


void Editor::ChangeEditKeyBar()
{
  if (EditKeyBar)
  {
    if (AnsiText)
      EditKeyBar->Change(MSG(MEditF8DOS),7);
    else
      EditKeyBar->Change(MSG(MEditF8),7);

    EditKeyBar->Redraw();
  }
}


/* $ 03.12.2001 IS
   UndoData - теперь указатель
*/
void Editor::AddUndoData(const char *Str,int StrNum,int StrPos,int Type)
{
  int PrevUndoDataPos;
  if (DisableUndo || !UndoData)
    return;
  if (StrNum==-1)
    StrNum=NumLine;
  if ((PrevUndoDataPos=UndoDataPos-1)<0)
    PrevUndoDataPos=Opt.EditorUndoSize-1;
  if (!NewUndo && Type==UNDO_EDIT && UndoData[PrevUndoDataPos].Type==UNDO_EDIT &&
      StrNum==UndoData[PrevUndoDataPos].StrNum &&
      (abs(StrPos-UndoData[PrevUndoDataPos].StrPos)<=1 ||
      abs(StrPos-LastChangeStrPos)<=1))
  {
    LastChangeStrPos=StrPos;
    return;
  }
  NewUndo=FALSE;
  if (UndoData[UndoDataPos].Type!=UNDO_NONE && UndoData[UndoDataPos].Str!=NULL)
    delete[] UndoData[UndoDataPos].Str;
  UndoData[UndoDataPos].Type=Type;
  UndoData[UndoDataPos].UndoNext=BlockUndo;
  UndoData[UndoDataPos].StrPos=StrPos;
  UndoData[UndoDataPos].StrNum=StrNum;
  if (Str!=NULL)
  {
    UndoData[UndoDataPos].Str=new char[strlen(Str)+1];
    if (UndoData[UndoDataPos].Str!=NULL)
      strcpy(UndoData[UndoDataPos].Str,Str);
  }
  else
    UndoData[UndoDataPos].Str=NULL;
  if (++UndoDataPos==Opt.EditorUndoSize)
    UndoDataPos=0;
  if (UndoDataPos==UndoSavePos)
    UndoOverflow=TRUE;
}
/* IS $ */

/* $ 03.12.2001 IS
   UndoData - теперь указатель
*/
void Editor::Undo()
{
  if(!UndoData)
    return;
  int NewPos=UndoDataPos-1;
  if (NewPos<0)
    NewPos=Opt.EditorUndoSize-1;
  if (UndoData[NewPos].Type==UNDO_NONE)
    return;
  UnmarkBlock();
  UndoDataPos=NewPos;
  /*$ 10.08.2000 skv
    Modified->TextChanged
  */
  TextChanged(1);
  /* skv $*/
  WasChanged=TRUE;
  DisableUndo=TRUE;
  GoToLine(UndoData[UndoDataPos].StrNum);
  switch(UndoData[UndoDataPos].Type)
  {
    case UNDO_INSSTR:
      DeleteString(CurLine,TRUE,NumLine>0 ? NumLine-1:NumLine);
      break;
    case UNDO_DELSTR:
      Pasting++;
      if (NumLine<UndoData[UndoDataPos].StrNum)
      {
        ProcessKey(KEY_END);
        ProcessKey(KEY_ENTER);
      }
      else
      {
        ProcessKey(KEY_HOME);
        ProcessKey(KEY_ENTER);
        ProcessKey(KEY_UP);
      }
      Pasting--;
      if (UndoData[UndoDataPos].Str!=NULL)
        CurLine->EditLine.SetString(UndoData[UndoDataPos].Str);
      break;
    case UNDO_EDIT:
      if (UndoData[UndoDataPos].Str!=NULL)
        CurLine->EditLine.SetString(UndoData[UndoDataPos].Str);
      CurLine->EditLine.SetCurPos(UndoData[UndoDataPos].StrPos);
      break;
  }
  if (UndoData[UndoDataPos].Str!=NULL)
    delete[] UndoData[UndoDataPos].Str;
  UndoData[UndoDataPos].Type=UNDO_NONE;
  if (UndoData[UndoDataPos].UndoNext)
    Undo();
  /*$ 10.08.2000 skv
    ! Modified->TextChanged
  */
  if (!UndoOverflow && UndoDataPos==UndoSavePos)
    TextChanged(0);
  /* skv $*/
  DisableUndo=FALSE;
}
/* IS $ */

void Editor::SelectAll()
{
  struct EditList *CurPtr;
  BlockStart=TopList;
  BlockStartLine=0;
  for (CurPtr=TopList;CurPtr!=NULL;CurPtr=CurPtr->Next)
    if (CurPtr->Next!=NULL)
      CurPtr->EditLine.Select(0,-1);
    else
      CurPtr->EditLine.Select(0,CurPtr->EditLine.GetLength());
  Show();
}


void Editor::SetStartPos(int LineNum,int CharNum)
{
  StartLine=LineNum==0 ? 1:LineNum;
  StartChar=CharNum==0 ? 1:CharNum;
}


int Editor::IsFileChanged()
{
  return(Modified || WasChanged);
}


int Editor::IsFileModified()
{
  return(Modified);
}


void Editor::SetTitle(const char *Title)
{
  if (Title==NULL)
    *Editor::Title=0;
  else
  /* $ 08.06.2001
     - Баг: не учитывался размер Title, что приводило к порче памяти и
       к падению Фара.
  */
    strncpy(Editor::Title, Title, sizeof(Editor::Title)-1);
  /* IS $ */
}

// используется в FileEditor
long Editor::GetCurPos()
{
  struct EditList *CurPtr=TopList;
  long TotalSize=0;
  while (CurPtr!=TopScreen)
  {
    char *SaveStr,*EndSeq;
    int Length;
    CurPtr->EditLine.GetBinaryString(&SaveStr,&EndSeq,Length);
    TotalSize+=Length+strlen(EndSeq);
    CurPtr=CurPtr->Next;
  }
  return(TotalSize);
}


void Editor::SetPluginData(char *PluginData)
{
  strcpy(Editor::PluginData,NullToEmpty(PluginData));
}

void Editor::SetPluginTitle(char *PluginTitle)
{
  strcpy(Editor::PluginTitle,NullToEmpty(PluginTitle));
}

void Editor::SetStringsTable()
{
  struct EditList *CurPtr=TopList;
  while (CurPtr!=NULL)
  {
    CurPtr->EditLine.SetTables(UseDecodeTable ? &TableSet:NULL);
    CurPtr=CurPtr->Next;
  }
}


void Editor::BlockLeft()
{
  if (VBlockStart!=NULL)
  {
    VBlockShift(TRUE);
    return;
  }
  struct EditList *CurPtr=BlockStart;
  int LineNum=BlockStartLine;
/* $ 14.02.2001 VVM
    + При отсутствии блока AltU/AltI сдвигают текущую строчку */
  int MoveLine = 0;
  if (CurPtr==NULL)
  {
    MoveLine = 1;
    CurPtr = CurLine;
    LineNum = NumLine;
  }
/* VVM $ */
  while (CurPtr!=NULL)
  {
    int StartSel,EndSel;
    CurPtr->EditLine.GetSelection(StartSel,EndSel);
    /* $ 14.02.2001 VVM
      + Блока нет - сделаем его искусственно */
    if (MoveLine) {
      StartSel = 0; EndSel = -1;
    }
    /* VVM $ */
    if (StartSel==-1)
      break;

    int Length=CurPtr->EditLine.GetLength();
    char *TmpStr=new char[Length+EdOpt.TabSize+5];

    char *CurStr,*EndSeq;
    CurPtr->EditLine.GetBinaryString(&CurStr,&EndSeq,Length);

    Length--;
    if (*CurStr==' ')
      memcpy(TmpStr,CurStr+1,Length);
    else
      if (*CurStr=='\t')
      {
        memset(TmpStr,' ',EdOpt.TabSize-1);
        memcpy(TmpStr+EdOpt.TabSize-1,CurStr+1,Length);
        Length+=EdOpt.TabSize-1;
      }

    /* $ 24.07.2001 IS isspace для этого и придумали */
    if ((EndSel==-1 || EndSel>StartSel) && isspace(*CurStr))
    /* IS $ */
    {
      int EndLength=strlen(EndSeq);
      memcpy(TmpStr+Length,EndSeq,EndLength);
      Length+=EndLength;
      TmpStr[Length]=0;
      AddUndoData(CurStr,LineNum,0,UNDO_EDIT);
      BlockUndo=TRUE;
      int CurPos=CurPtr->EditLine.GetCurPos();
      CurPtr->EditLine.SetBinaryString(TmpStr,Length);
      CurPtr->EditLine.SetCurPos(CurPos>0 ? CurPos-1:CurPos);
      /* $ 14.02.2001 VVM
        + Выделить только если двигаем блок */
      if (!MoveLine)
      /* VVM $ */
        CurPtr->EditLine.Select(StartSel>0 ? StartSel-1:StartSel,EndSel>0 ? EndSel-1:EndSel);
      /*$ 10.08.2000 skv
        Modified->TextChanged
      */
      TextChanged(1);
      /* skv $*/
    }

    delete[] TmpStr;
    CurPtr=CurPtr->Next;
    LineNum++;
    MoveLine = 0;
  }
  BlockUndo=FALSE;
}


void Editor::BlockRight()
{
  if (VBlockStart!=NULL)
  {
    VBlockShift(FALSE);
    return;
  }
  struct EditList *CurPtr=BlockStart;
  int LineNum=BlockStartLine;
/* $ 14.02.2001 VVM
    + При отсутствии блока AltU/AltI сдвигают текущую строчку */
  int MoveLine = 0;
  if (CurPtr==NULL)
  {
    MoveLine = 1;
    CurPtr = CurLine;
    LineNum = NumLine;
  }
/* VVM $ */
  while (CurPtr!=NULL)
  {
    int StartSel,EndSel;
    CurPtr->EditLine.GetSelection(StartSel,EndSel);
    /* $ 14.02.2001 VVM
      + Блока нет - сделаем его искусственно */
    if (MoveLine) {
      StartSel = 0; EndSel = -1;
    }
    /* VVM $ */
    if (StartSel==-1)
      break;

    int Length=CurPtr->EditLine.GetLength();
    char *TmpStr=new char[Length+5];

    char *CurStr,*EndSeq;
    CurPtr->EditLine.GetBinaryString(&CurStr,&EndSeq,Length);
    *TmpStr=' ';
    memcpy(TmpStr+1,CurStr,Length);
    Length++;

    if (EndSel==-1 || EndSel>StartSel)
    {
      int EndLength=strlen(EndSeq);
      memcpy(TmpStr+Length,EndSeq,EndLength);
      TmpStr[Length+EndLength]=0;
      AddUndoData(CurStr,LineNum,0,UNDO_EDIT);
      BlockUndo=TRUE;
      int CurPos=CurPtr->EditLine.GetCurPos();
      if (Length>1)
        CurPtr->EditLine.SetBinaryString(TmpStr,Length+EndLength);
      CurPtr->EditLine.SetCurPos(CurPos+1);
      /* $ 14.02.2001 VVM
        + Выделить только если двигаем блок */
      if (!MoveLine)
      /* VVM $ */
        CurPtr->EditLine.Select(StartSel>0 ? StartSel+1:StartSel,EndSel>0 ? EndSel+1:EndSel);
      /*$ 10.08.2000 skv
        Modified->TextChanged
      */
      TextChanged(1);
      /* skv $*/
    }

    delete[] TmpStr;
    CurPtr=CurPtr->Next;
    LineNum++;
    MoveLine = 0;
  }
  BlockUndo=FALSE;
}


void Editor::DeleteVBlock()
{
  if (LockMode || VBlockSizeX<=0 || VBlockSizeY<=0)
    return;

  int UndoNext=FALSE;

  if (!EdOpt.PersistentBlocks)
  {
    struct EditList *CurPtr=CurLine;
    struct EditList *NewTopScreen=TopScreen;
    while (CurPtr!=NULL)
    {
      if (CurPtr==VBlockStart)
      {
        TopScreen=NewTopScreen;
        CurLine=CurPtr;
        CurPtr->EditLine.SetTabCurPos(VBlockX);
        break;
      }
      NumLine--;
      if (NewTopScreen==CurPtr && CurPtr->Prev!=NULL)
        NewTopScreen=CurPtr->Prev;
      CurPtr=CurPtr->Prev;
    }
  }

  struct EditList *CurPtr=VBlockStart;

  for (int Line=0;CurPtr!=NULL && Line<VBlockSizeY;Line++,CurPtr=CurPtr->Next)
  {
    /*$ 10.08.2000 skv
      Modified->TextChanged
    */
    TextChanged(1);
    /* skv $*/

    int TBlockX=CurPtr->EditLine.TabPosToReal(VBlockX);
    int TBlockSizeX=CurPtr->EditLine.TabPosToReal(VBlockX+VBlockSizeX)-
                    CurPtr->EditLine.TabPosToReal(VBlockX);

    char *CurStr,*EndSeq;
    int Length;
    CurPtr->EditLine.GetBinaryString(&CurStr,&EndSeq,Length);
    if (TBlockX>=Length)
      continue;

    BlockUndo=UndoNext;
    AddUndoData(CurPtr->EditLine.GetStringAddr(),BlockStartLine+Line,
                CurPtr->EditLine.GetCurPos(),UNDO_EDIT);
    UndoNext=TRUE;

    char *TmpStr=new char[Length+3];
    int CurLength=TBlockX;
    memcpy(TmpStr,CurStr,TBlockX);
    if (Length>TBlockX+TBlockSizeX)
    {
      int CopySize=Length-(TBlockX+TBlockSizeX);
      memcpy(TmpStr+CurLength,CurStr+TBlockX+TBlockSizeX,CopySize);
      CurLength+=CopySize;
    }
    int EndLength=strlen(EndSeq);
    memcpy(TmpStr+CurLength,EndSeq,EndLength);
    CurLength+=EndLength;
    TmpStr[CurLength]=0;

    int CurPos=CurPtr->EditLine.GetCurPos();
    CurPtr->EditLine.SetBinaryString(TmpStr,CurLength);
    if (CurPos>TBlockX)
    {
      CurPos-=TBlockSizeX;
      if (CurPos<TBlockX)
        CurPos=TBlockX;
    }
    CurPtr->EditLine.SetCurPos(CurPos);
    delete[] TmpStr;
  }

  VBlockStart=NULL;
  BlockUndo=FALSE;
}

void Editor::VCopy(int Append)
{
  struct EditList *CurPtr=VBlockStart;
  char *CopyData=NULL;
  long DataSize=0,PrevSize=0;

  if (Append)
  {
    CopyData=PasteFormatFromClipboard("FAR_VerticalBlock");
    if (CopyData!=NULL)
      PrevSize=DataSize=strlen(CopyData);
    else
    {
      CopyData=PasteFromClipboard();
      if (CopyData!=NULL)
        PrevSize=DataSize=strlen(CopyData);
    }
  }

  for (int Line=0;CurPtr!=NULL && Line<VBlockSizeY;Line++,CurPtr=CurPtr->Next)
  {
    int TBlockX=CurPtr->EditLine.TabPosToReal(VBlockX);
    int TBlockSizeX=CurPtr->EditLine.TabPosToReal(VBlockX+VBlockSizeX)-
                    CurPtr->EditLine.TabPosToReal(VBlockX);
    char *CurStr,*EndSeq;
    int Length;
    CurPtr->EditLine.GetBinaryString(&CurStr,&EndSeq,Length);

    int AllocSize=Max(DataSize+Length+3,DataSize+TBlockSizeX+3);
    char *NewPtr=(char *)realloc(CopyData,AllocSize);
    if (NewPtr==NULL)
    {
      delete CopyData;
      CopyData=NULL;
      break;
    }
    CopyData=NewPtr;

    if (Length>TBlockX)
    {
      int CopySize=Length-TBlockX;
      if (CopySize>TBlockSizeX)
        CopySize=TBlockSizeX;
      memcpy(CopyData+DataSize,CurStr+TBlockX,CopySize);
      if (CopySize<TBlockSizeX)
        memset(CopyData+DataSize+CopySize,' ',TBlockSizeX-CopySize);
    }
    else
      memset(CopyData+DataSize,' ',TBlockSizeX);

    DataSize+=TBlockSizeX;


    strcpy(CopyData+DataSize,"\r\n");
    DataSize+=2;
  }

  if (CopyData!=NULL)
  {
    if (UseDecodeTable)
      DecodeString(CopyData+PrevSize,(unsigned char *)TableSet.DecodeTable);
    CopyToClipboard(CopyData);
    CopyFormatToClipboard("FAR_VerticalBlock",CopyData);
    delete CopyData;
  }
}

void Editor::VPaste(char *ClipText)
{
  if (LockMode)
    return;

  if (*ClipText)
  {
    NewUndo=TRUE;
    /*$ 10.08.2000 skv
      Modified->TextChanged
    */
    TextChanged(1);
    /* skv $*/
    int SaveOvertype=Overtype;
    UnmarkBlock();
    Pasting++;
    DisableOut++;
    Edit::DisableEditOut(TRUE);
    if (Overtype)
    {
      Overtype=FALSE;
      CurLine->EditLine.SetOvertypeMode(FALSE);
    }

    VBlockStart=CurLine;
    BlockStartLine=NumLine;

    int StartPos=CurLine->EditLine.GetTabCurPos();

    VBlockX=StartPos;
    VBlockSizeX=0;
    VBlockY=NumLine;
    VBlockSizeY=0;

    struct EditList *SavedTopScreen=TopScreen;


    for (int I=0;ClipText[I]!=0;I++)
      if (ClipText[I]!=13 && ClipText[I+1]!=10)
        ProcessKey(ClipText[I]);
      else
      {
        BlockUndo=TRUE;
        int CurWidth=CurLine->EditLine.GetTabCurPos()-StartPos;
        if (CurWidth>VBlockSizeX)
          VBlockSizeX=CurWidth;
        VBlockSizeY++;
        if (CurLine->Next==NULL)
        {
          if (ClipText[I+2]!=0)
          {
            ProcessKey(KEY_END);
            ProcessKey(KEY_ENTER);
            /* $ 19.05.2001 IS
               Не вставляем пробелы тогда, когда нас об этом не просят, а
               именно - при включенном автоотступе ничего вставлять не нужно,
               оно само вставится и в другом месте.
            */
            if(!EdOpt.AutoIndent)
              for (int I=0;I<StartPos;I++)
                ProcessKey(' ');
            /* IS $ */
          }
        }
        else
        {
          ProcessKey(KEY_DOWN);
          CurLine->EditLine.SetTabCurPos(StartPos);
          CurLine->EditLine.SetOvertypeMode(FALSE);
        }
        I++;
        continue;
      }

    int CurWidth=CurLine->EditLine.GetTabCurPos()-StartPos;
    if (CurWidth>VBlockSizeX)
      VBlockSizeX=CurWidth;
    if (VBlockSizeY==0)
      VBlockSizeY++;

    if (SaveOvertype)
    {
      Overtype=TRUE;
      CurLine->EditLine.SetOvertypeMode(TRUE);
    }

    TopScreen=SavedTopScreen;
    CurLine=VBlockStart;
    NumLine=BlockStartLine;
    CurLine->EditLine.SetTabCurPos(StartPos);

    Edit::DisableEditOut(FALSE);
    Pasting--;
    DisableOut--;
  }
  delete ClipText;
  BlockUndo=FALSE;
}


void Editor::VBlockShift(int Left)
{
  if (LockMode || Left && VBlockX==0 || VBlockSizeX<=0 || VBlockSizeY<=0)
    return;

  struct EditList *CurPtr=VBlockStart;

  int UndoNext=FALSE;

  for (int Line=0;CurPtr!=NULL && Line<VBlockSizeY;Line++,CurPtr=CurPtr->Next)
  {
    /*$ 10.08.2000 skv
      Modified->TextChanged
    */
    TextChanged(1);
    /* skv $*/

    int TBlockX=CurPtr->EditLine.TabPosToReal(VBlockX);
    int TBlockSizeX=CurPtr->EditLine.TabPosToReal(VBlockX+VBlockSizeX)-
                    CurPtr->EditLine.TabPosToReal(VBlockX);

    char *CurStr,*EndSeq;
    int Length;
    CurPtr->EditLine.GetBinaryString(&CurStr,&EndSeq,Length);
    if (TBlockX>Length)
      continue;
    if (Left && CurStr[TBlockX-1]=='\t' ||
        !Left && TBlockX+TBlockSizeX<Length && CurStr[TBlockX+TBlockSizeX]=='\t')
    {
      CurPtr->EditLine.ReplaceTabs();
      CurPtr->EditLine.GetBinaryString(&CurStr,&EndSeq,Length);
      TBlockX=CurPtr->EditLine.TabPosToReal(VBlockX);
      TBlockSizeX=CurPtr->EditLine.TabPosToReal(VBlockX+VBlockSizeX)-
                  CurPtr->EditLine.TabPosToReal(VBlockX);
    }


    BlockUndo=UndoNext;
    AddUndoData(CurPtr->EditLine.GetStringAddr(),BlockStartLine+Line,
                CurPtr->EditLine.GetCurPos(),UNDO_EDIT);
    UndoNext=TRUE;

    int StrLength=Max(Length,TBlockX+TBlockSizeX+!Left);
    char *TmpStr=new char[StrLength+3];
    memset(TmpStr,' ',StrLength);
    memcpy(TmpStr,CurStr,Length);

    if (Left)
    {
      int Ch=TmpStr[TBlockX-1];
      for (int I=TBlockX;I<TBlockX+TBlockSizeX;I++)
        TmpStr[I-1]=TmpStr[I];
      TmpStr[TBlockX+TBlockSizeX-1]=Ch;
    }
    else
    {
      int Ch=TmpStr[TBlockX+TBlockSizeX];
      for (int I=TBlockX+TBlockSizeX-1;I>=TBlockX;I--)
        TmpStr[I+1]=TmpStr[I];
      TmpStr[TBlockX]=Ch;
    }

    while (StrLength>0 && TmpStr[StrLength-1]==' ')
      StrLength--;
    int EndLength=strlen(EndSeq);
    memcpy(TmpStr+StrLength,EndSeq,EndLength);
    StrLength+=EndLength;
    TmpStr[StrLength]=0;

    CurPtr->EditLine.SetBinaryString(TmpStr,StrLength);
    delete[] TmpStr;
  }
  VBlockX+=Left ? -1:1;
  CurLine->EditLine.SetTabCurPos(Left ? VBlockX:VBlockX+VBlockSizeX);
}


int Editor::EditorControl(int Command,void *Param)
{
  int I;
  switch(Command)
  {
    case ECTL_GETSTRING:
      {
        struct EditorGetString *GetString=(struct EditorGetString *)Param;
        struct EditList *CurPtr=GetStringByNumber(GetString->StringNumber);
        if (CurPtr==NULL)
          return(FALSE);
        CurPtr->EditLine.GetBinaryString(&GetString->StringText,&GetString->StringEOL,GetString->StringLength);
        GetString->SelStart=-1;
        GetString->SelEnd=0;
        int DestLine=GetString->StringNumber;
        if (DestLine==-1)
          DestLine=NumLine;
        if (BlockStart!=NULL)
          CurPtr->EditLine.GetSelection(GetString->SelStart,GetString->SelEnd);
        else
          if (VBlockStart!=NULL && DestLine>=VBlockY && DestLine<VBlockY+VBlockSizeY)
          {
            GetString->SelStart=CurPtr->EditLine.TabPosToReal(VBlockX);
            GetString->SelEnd=GetString->SelStart+
                       CurPtr->EditLine.TabPosToReal(VBlockX+VBlockSizeX)-
                       CurPtr->EditLine.TabPosToReal(VBlockX);
          }
      }
      return(TRUE);
    case ECTL_INSERTSTRING:
      if (LockMode)
        return(FALSE);
      {
        int Indent=Param!=NULL && *(int *)Param!=FALSE;
        if (!Indent)
          Pasting++;
        NewUndo=TRUE;
        InsertString();
        Show();
        if (!Indent)
          Pasting--;
      }
      return(TRUE);
    case ECTL_INSERTTEXT:
      if (LockMode)
        return(FALSE);
      {
        char *Str=(char *)Param;
//_SVS(SysLog("Editor::EditorControl[%d]: ECTL_INSERTTEXT(%s)",__LINE__,Str));
        Pasting++;
        DisableOut++;
        Edit::DisableEditOut(TRUE);
        while (*Str)
          ProcessKey(*(Str++));
        Edit::DisableEditOut(FALSE);
        DisableOut--;
        Pasting--;
      }
      return(TRUE);
    case ECTL_SETSTRING:
      if (LockMode)
        return(FALSE);
      {
        struct EditorSetString *SetString=(struct EditorSetString *)Param;

        struct EditList *CurPtr=GetStringByNumber(SetString->StringNumber);
        if (CurPtr==NULL)
          return(FALSE);
        const char *EOL=SetString->StringEOL==NULL ? GlobalEOL:SetString->StringEOL;
        int Length=SetString->StringLength;
        int LengthEOL=strlen(EOL);
        char *NewStr=new char[Length+LengthEOL+1];
        if (NewStr==NULL)
          return(FALSE);
        int DestLine=SetString->StringNumber;
        if (DestLine==-1)
          DestLine=NumLine;
        memcpy(NewStr,SetString->StringText,Length);
        memcpy(NewStr+Length,EOL,LengthEOL);
        AddUndoData(CurPtr->EditLine.GetStringAddr(),DestLine,
                    CurPtr->EditLine.GetCurPos(),UNDO_EDIT);
        int CurPos=CurPtr->EditLine.GetCurPos();
        CurPtr->EditLine.SetBinaryString(NewStr,Length+LengthEOL);
        CurPtr->EditLine.SetCurPos(CurPos);
        /*$ 10.08.2000 skv
          Modified->TextChanged
        */
        TextChanged(1);
        /* skv $*/
        delete[] NewStr;
      }
      return(TRUE);
    case ECTL_DELETESTRING:
      if (LockMode)
        return(FALSE);
      DeleteString(CurLine,FALSE,NumLine);
      return(TRUE);
    case ECTL_DELETECHAR:
      if (LockMode)
        return(FALSE);
      Pasting++;
      ProcessKey(KEY_DEL);
      Pasting--;
      return(TRUE);
    case ECTL_GETINFO:
      {
        struct EditorInfo *Info=(struct EditorInfo *)Param;
        memset(Info,0,sizeof(*Info));
        Info->EditorID=Editor::EditorID;
        Info->FileName=FileName;
        Info->WindowSizeX=ObjWidth;
        Info->WindowSizeY=Y2-Y1;
        Info->TotalLines=NumLastLine;
        Info->CurLine=NumLine;
        Info->CurPos=CurLine->EditLine.GetCurPos();
        Info->CurTabPos=CurLine->EditLine.GetTabCurPos();
        Info->TopScreenLine=NumLine-CalcDistance(TopScreen,CurLine,-1);
        Info->LeftPos=CurLine->EditLine.GetLeftPos();
        Info->Overtype=Overtype;
        Info->BlockType=BTYPE_NONE;
        if (BlockStart!=NULL)
          Info->BlockType=BTYPE_STREAM;
        if (VBlockStart!=NULL)
          Info->BlockType=BTYPE_COLUMN;
        Info->BlockStartLine=Info->BlockType==BTYPE_NONE ? 0:BlockStartLine;
        Info->AnsiMode=AnsiText;
        Info->TableNum=UseDecodeTable ? TableNum-1:-1;
        Info->Options=0;
        if (EdOpt.ExpandTabs)
          Info->Options|=EOPT_EXPANDTABS;
        if (EdOpt.PersistentBlocks)
          Info->Options|=EOPT_PERSISTENTBLOCKS;
        if (EdOpt.DelRemovesBlocks)
          Info->Options|=EOPT_DELREMOVESBLOCKS;
        if (EdOpt.AutoIndent)
          Info->Options|=EOPT_AUTOINDENT;
        if (EdOpt.SavePos)
          Info->Options|=EOPT_SAVEFILEPOSITION;
        if (EdOpt.AutoDetectTable)
          Info->Options|=EOPT_AUTODETECTTABLE;
        if (EdOpt.CursorBeyondEOL)
          Info->Options|=EOPT_CURSORBEYONDEOL;
        Info->TabSize=EdOpt.TabSize;
        Info->BookMarkCount=BOOKMARK_COUNT;
        Info->CurState=LockMode?ECSTATE_LOCKED:0;
        Info->CurState|=!Modified?ECSTATE_SAVED:0;
        Info->CurState|=WasChanged || Modified?ECSTATE_MODIFIED:0;
      }
      return(TRUE);
    case ECTL_SETPOSITION:
      {
        struct EditorSetPosition *Pos=(struct EditorSetPosition *)Param;
        DisableOut++;
        if (Pos->CurLine!=-1)
          if (Pos->CurLine==NumLine-1)
            Up();
          else
            if (Pos->CurLine==NumLine+1)
              Down();
            else
              GoToLine(Pos->CurLine);
        if (Pos->TopScreenLine!=-1 && Pos->TopScreenLine<=NumLine)
        {
          TopScreen=CurLine;
          for (int I=NumLine;I>0 && NumLine-I<Y2-Y1+1 && I!=Pos->TopScreenLine;I--)
            TopScreen=TopScreen->Prev;
        }
        if (Pos->CurPos!=-1)
          CurLine->EditLine.SetCurPos(Pos->CurPos);
        if (Pos->CurTabPos!=-1)
          CurLine->EditLine.SetTabCurPos(Pos->CurTabPos);
        if (Pos->LeftPos!=-1)
          CurLine->EditLine.SetLeftPos(Pos->LeftPos);
        /* $ 30.08.2001 IS
           Изменение режима нужно выставлять сразу, в противном случае приходят
           глюки, т.к. плагинописатель думает, что режим изменен, и ведет себя
           соответствующе, в результате чего получает неопределенное поведение.
        */
        if (Pos->Overtype!=-1)
          CurLine->EditLine.SetOvertypeMode(Overtype=Pos->Overtype);
        /* IS $ */
        DisableOut--;
      }
      return(TRUE);
    case ECTL_SELECT:
      {
        struct EditorSelect *Sel=(struct EditorSelect *)Param;
        UnmarkBlock();
        if (Sel->BlockType==BTYPE_NONE)
          return(TRUE);
        struct EditList *CurPtr=GetStringByNumber(Sel->BlockStartLine);
        if (CurPtr==NULL)
          return(FALSE);
        if (Sel->BlockType==BTYPE_STREAM)
        {
          BlockStart=CurPtr;
          BlockStartLine=Sel->BlockStartLine;
          for (I=0;I<Sel->BlockHeight;I++)
          {
            int SelStart=(I==0) ? Sel->BlockStartPos:0;
            int SelEnd=(I<Sel->BlockHeight-1) ? -1:Sel->BlockStartPos+Sel->BlockWidth;
            CurPtr->EditLine.Select(SelStart,SelEnd);
            CurPtr=CurPtr->Next;
            if (CurPtr==NULL)
              return(FALSE);
          }
        }
        if (Sel->BlockType==BTYPE_COLUMN)
        {
          VBlockStart=CurPtr;
          BlockStartLine=Sel->BlockStartLine;
          if (Sel->BlockWidth==-1)
            return(FALSE);
          VBlockX=Sel->BlockStartPos;
          VBlockY=Sel->BlockStartLine;
          VBlockSizeX=Sel->BlockWidth;
          VBlockSizeY=Sel->BlockHeight;
        }
      }
      return(TRUE);
    case ECTL_REDRAW:
//_SVS(SysLog("Editor::EditorControl[%d]: ECTL_REDRAW",__LINE__));
      Show();
      ScrBuf.Flush();
      return(TRUE);
    case ECTL_EDITORTOOEM:
      {
        struct EditorConvertText *ect=(struct EditorConvertText *)Param;
        if (UseDecodeTable)
          DecodeString(ect->Text,(unsigned char *)TableSet.DecodeTable,ect->TextLength);
      }
      return(TRUE);
    case ECTL_OEMTOEDITOR:
      {
        struct EditorConvertText *ect=(struct EditorConvertText *)Param;
        if (UseDecodeTable)
          EncodeString(ect->Text,(unsigned char *)TableSet.EncodeTable,ect->TextLength);
      }
      return(TRUE);
    case ECTL_TABTOREAL:
      {
        struct EditorConvertPos *ecp=(struct EditorConvertPos *)Param;
        struct EditList *CurPtr=GetStringByNumber(ecp->StringNumber);
        if (CurPtr==NULL)
          return(FALSE);
        ecp->DestPos=CurPtr->EditLine.TabPosToReal(ecp->SrcPos);
      }
      return(TRUE);
    case ECTL_REALTOTAB:
      {
        struct EditorConvertPos *ecp=(struct EditorConvertPos *)Param;
        struct EditList *CurPtr=GetStringByNumber(ecp->StringNumber);
        if (CurPtr==NULL)
          return(FALSE);
        ecp->DestPos=CurPtr->EditLine.RealPosToTab(ecp->SrcPos);
      }
      return(TRUE);
    case ECTL_EXPANDTABS:
      if (LockMode)
        return(FALSE);
      {
        int StringNumber=*(int *)Param;
        struct EditList *CurPtr=GetStringByNumber(StringNumber);
        if (CurPtr==NULL)
          return(FALSE);
        AddUndoData(CurPtr->EditLine.GetStringAddr(),StringNumber,
                    CurPtr->EditLine.GetCurPos(),UNDO_EDIT);
        CurPtr->EditLine.ReplaceTabs();
      }
      return(TRUE);
    case ECTL_SETTITLE:
      {
        char *Title=(char *)Param;
        /* $ 08.06.2001 IS
           - Баг: не учитывался размер PluginTitle
        */
        strncpy(PluginTitle,NullToEmpty(Title),sizeof(PluginTitle)-1);
        /* IS $ */
        ShowStatus();
        ScrBuf.Flush();
      }
      return(TRUE);
    case ECTL_READINPUT:
      {
        _KEYMACRO(CleverSysLog SL("Editor::EditorControl(ECTL_READINPUT)"));
        INPUT_RECORD *rec=(INPUT_RECORD *)Param;
        GetInputRecord(rec);
      }
      return(TRUE);
    case ECTL_PROCESSINPUT:
      {
        _KEYMACRO(CleverSysLog SL("Editor::EditorControl(ECTL_PROCESSINPUT)"));
        INPUT_RECORD *rec=(INPUT_RECORD *)Param;
        if (ProcessEditorInput(rec))
          return(TRUE);
        if (rec->EventType==MOUSE_EVENT)
          ProcessMouse(&rec->Event.MouseEvent);
        else
        {
          int Key=CalcKeyCode(rec,FALSE);
          _KEYMACRO(SysLog("Key=CalcKeyCode() = 0x%08X",Key));
          ProcessKey(Key);
        }
      }
      return(TRUE);
    case ECTL_ADDCOLOR:
      {
        struct EditorColor *col=(struct EditorColor *)Param;
        if (!col)
          return(FALSE);
        struct ColorItem newcol;
        newcol.StartPos=col->StartPos;
        newcol.EndPos=col->EndPos;
        newcol.Color=col->Color;
        struct EditList *CurPtr=GetStringByNumber(col->StringNumber);
        if (CurPtr==NULL)
          return(FALSE);
        if (col->Color==0)
          return(CurPtr->EditLine.DeleteColor(newcol.StartPos));
        CurPtr->EditLine.AddColor(&newcol);
      }
      return(TRUE);
    case ECTL_GETCOLOR:
      {
        struct EditorColor *col=(struct EditorColor *)Param;
        struct EditList *CurPtr=GetStringByNumber(col->StringNumber);
        if (!CurPtr || !col || IsBadWritePtr(col,sizeof(struct EditorColor)))
          return(FALSE);
        struct ColorItem curcol;
        if (!CurPtr->EditLine.GetColor(&curcol,col->ColorItem))
          return(FALSE);
        col->StartPos=curcol.StartPos;
        col->EndPos=curcol.EndPos;
        col->Color=curcol.Color;
      }
      return(TRUE);
    case ECTL_SAVEFILE:
      {
        EditorSaveFile *esf=(EditorSaveFile *)Param;
        char *Name=FileName;
        int EOL=0;
        if (esf!=NULL)
        {
          if (*esf->FileName)
            Name=esf->FileName;
          if (esf->FileEOL!=NULL)
          {
            if (strcmp(esf->FileEOL,"\r\n")==0)
              EOL=1;
            if (strcmp(esf->FileEOL,"\n")==0)
              EOL=2;
          }
        }
        return(SaveFile(Name,FALSE,EOL,!LocalStricmp(Name,FileName)));
      }
    case ECTL_QUIT:
      FrameManager->DeleteFrame(HostFileEditor);
      if (HostFileEditor!=NULL)
        HostFileEditor->SetExitCode(SAVEFILE_ERROR); // что-то меня терзают смутные сомнения ...
      return(TRUE);
    /* $ 07.08.2000 SVS
       Функция установки Keybar Labels
         Param = NULL - восстановить, пред. значение
         Param = -1   - обновить полосу (перерисовать)
         Param = KeyBarTitles
    */
    case ECTL_SETKEYBAR:
    {
      /* $ 22.12.2000 SVS
         Вызов из EE_READ команды ECTL_SETKEYBAR приводил к падению ФАРа, т.к.
         объект EditKeyBar еще не существует.
      */
      if(!EditKeyBar)
        return FALSE;
      /* SVS $ */
      struct KeyBarTitles *Kbt=(struct KeyBarTitles*)Param;
      if(!Kbt)
      {        // восстановить пред значение!
        if (HostFileEditor!=NULL)
          HostFileEditor->InitKeyBar();
      }
      else
      {
        if((long)Param != (long)-1) // не только перерисовать?
        {
          for(I=0; I < 12; ++I)
          {
            if(Kbt->Titles[I])
              EditKeyBar->Change(KBL_MAIN,Kbt->Titles[I],I);
            if(Kbt->CtrlTitles[I])
              EditKeyBar->Change(KBL_CTRL,Kbt->CtrlTitles[I],I);
            if(Kbt->AltTitles[I])
              EditKeyBar->Change(KBL_ALT,Kbt->AltTitles[I],I);
            if(Kbt->ShiftTitles[I])
              EditKeyBar->Change(KBL_SHIFT,Kbt->ShiftTitles[I],I);
            if(Kbt->CtrlShiftTitles[I])
              EditKeyBar->Change(KBL_CTRLSHIFT,Kbt->CtrlShiftTitles[I],I);
            if(Kbt->AltShiftTitles[I])
              EditKeyBar->Change(KBL_ALTSHIFT,Kbt->AltShiftTitles[I],I);
            if(Kbt->CtrlAltTitles[I])
              EditKeyBar->Change(KBL_CTRLALT,Kbt->CtrlAltTitles[I],I);
          }
        }
        EditKeyBar->Show();
      }
      return(TRUE);
    }
    /* SVS $ */
    /*$ 07.09.2000 skv
      New ECTL parameter
    */
    case ECTL_PROCESSKEY:
    {
      ProcessKey((int)Param);
      return TRUE;
    }
    /* skv$*/
    /* $ 16.02.2001 IS
         Изменение некоторых внутренних настроек редактора. Param указывает на
         структуру EditorSetParameter
    */
    case ECTL_SETPARAM:
    {
      struct EditorSetParameter *espar=(struct EditorSetParameter *)Param;
      if(espar)
      {
        int rc=TRUE;
        switch(espar->Type)
        {
          case ESPT_TABSIZE:
            SetTabSize(espar->iParam);
            break;
          case ESPT_EXPANDTABS:
            SetConvertTabs(espar->iParam);
            break;
          case ESPT_AUTOINDENT:
            SetAutoIndent(espar->iParam);
            break;
          case ESPT_CURSORBEYONDEOL:
            SetCursorBeyondEOL(espar->iParam);
            break;
          case ESPT_CHARCODEBASE:
            SetCharCodeBase(espar->iParam);
            break;
          /* $ 07.08.2001 IS сменить кодировку из плагина */
          case ESPT_CHARTABLE:
          {
            int UseUnicode=FALSE;
            /*  $ 04.11.2001 IS
                При неудачной смене таблицы оставим все как есть
                (раньше включали oem)
            */
            int oldAnsiText(AnsiText), oldUseDecodeTable(UseDecodeTable),
                oldTableNum(TableNum), oldChangedByUser(TableChangedByUser);

            AnsiText=espar->iParam==2,
            UseDecodeTable=espar->iParam>1,
            TableNum=UseDecodeTable?espar->iParam-3:-1;
            TableChangedByUser=TRUE;

            if(AnsiText)
               rc=GetTable(&TableSet,TRUE,TableNum,UseUnicode);
            else if(UseDecodeTable)
               rc=PrepareTable(&TableSet, TableNum);

            if(!rc)
            {
              TableChangedByUser=oldChangedByUser;
              TableNum=oldTableNum;
              UseDecodeTable=oldUseDecodeTable;
              AnsiText=oldAnsiText;
            }
            /* IS $ */

            SetStringsTable();
            ChangeEditKeyBar();
            Show();
          }
          /* IS $ */
          /* $ 29.10.2001 IS изменение настройки "Сохранять позицию файла" */
          case ESPT_SAVEFILEPOSITION:
            SetSavePosMode(espar->iParam, -1);
            break;
          /* IS $ */
          default:
            return FALSE;
        }
        return rc;
      }
      return  FALSE;
    }
    /* IS $ */
    case ECTL_GETBOOKMARKS:
    {
      if(!OpenFailed && Param)
      {
        struct EditorBookMarks *ebm=(struct EditorBookMarks *)Param;
        if(ebm->Line && !IsBadWritePtr(ebm->Line,BOOKMARK_COUNT*sizeof(long)))
          memcpy(ebm->Line,SavePos.Line,BOOKMARK_COUNT*sizeof(long));
        if(ebm->Cursor && !IsBadWritePtr(ebm->Cursor,BOOKMARK_COUNT*sizeof(long)))
          memcpy(ebm->Cursor,SavePos.Cursor,BOOKMARK_COUNT*sizeof(long));
        if(ebm->ScreenLine && !IsBadWritePtr(ebm->ScreenLine,BOOKMARK_COUNT*sizeof(long)))
          memcpy(ebm->ScreenLine,SavePos.ScreenLine,BOOKMARK_COUNT*sizeof(long));
        if(ebm->LeftPos && !IsBadWritePtr(ebm->LeftPos,BOOKMARK_COUNT*sizeof(long)))
          memcpy(ebm->LeftPos,SavePos.LeftPos,BOOKMARK_COUNT*sizeof(long));
        return TRUE;
      }
      return FALSE;
    }
  }
  return(FALSE);
}


struct EditList * Editor::GetStringByNumber(int DestLine)
{
  if (DestLine==NumLine || DestLine<0)
    return(CurLine);
  if (DestLine>NumLastLine)
    return(NULL);

  if (DestLine>NumLine)
  {
    struct EditList *CurPtr=CurLine;
    for (int Line=NumLine;Line<DestLine;Line++)
    {
      CurPtr=CurPtr->Next;
      if (CurPtr==NULL)
        return(NULL);
    }
    return(CurPtr);
  }

  if (DestLine<NumLine && DestLine>NumLine/2)
  {
    struct EditList *CurPtr=CurLine;
    for (int Line=NumLine;Line>DestLine;Line--)
    {
      CurPtr=CurPtr->Prev;
      if (CurPtr==NULL)
        return(NULL);
    }
    return(CurPtr);
  }

  {
    struct EditList *CurPtr=TopList;
    for (int Line=0;Line<DestLine;Line++)
    {
      CurPtr=CurPtr->Next;
      if (CurPtr==NULL)
        return(NULL);
    }
    return(CurPtr);
  }
}


int Editor::ProcessEditorInput(INPUT_RECORD *Rec)
{
#if defined(SYSLOG_KEYMACRO)
  CleverSysLog SL("Editor::ProcessEditorInput()");
  if(Rec->EventType == KEY_EVENT)
    SysLog("VKey=0x%04X",Rec->Event.KeyEvent.wVirtualKeyCode);
  CtrlObject->Plugins.CurEditor=this;
  int RetCode=CtrlObject->Plugins.ProcessEditorInput(Rec);
  SysLog("RetCode=%d",RetCode);
#else
  CtrlObject->Plugins.CurEditor=this;
  int RetCode=CtrlObject->Plugins.ProcessEditorInput(Rec);
#endif
  return(RetCode);
}


int Editor::IsShiftKey(int Key)
{
  /*
     29.06.2000 IG
     добавлены клавиши, чтобы не сбрасывалось выделение при их нажатии
  */
  static int ShiftKeys[]={KEY_SHIFTLEFT,KEY_SHIFTRIGHT,KEY_SHIFTHOME,
                KEY_SHIFTEND,KEY_SHIFTUP,KEY_SHIFTDOWN,KEY_SHIFTPGUP,
                KEY_SHIFTPGDN,KEY_CTRLSHIFTHOME,KEY_CTRLSHIFTPGUP,
                KEY_CTRLSHIFTEND,KEY_CTRLSHIFTPGDN,
                KEY_CTRLSHIFTLEFT,KEY_CTRLSHIFTRIGHT,KEY_ALTSHIFTDOWN,
                KEY_ALTSHIFTLEFT,KEY_ALTSHIFTRIGHT,KEY_ALTSHIFTUP,
                KEY_ALTSHIFTEND,KEY_ALTSHIFTHOME,KEY_ALTSHIFTPGDN,
                KEY_ALTSHIFTPGUP,KEY_ALTUP,KEY_ALTLEFT,KEY_ALTDOWN,
                KEY_ALTRIGHT,KEY_ALTHOME,KEY_ALTEND,KEY_ALTPGUP,KEY_ALTPGDN,
                KEY_CTRLALTPGUP,KEY_CTRLALTHOME,KEY_CTRLALTPGDN,KEY_CTRLALTEND,
                KEY_CTRLALTLEFT, KEY_CTRLALTRIGHT
  };
  /* IG $ */

  for (int I=0;I<sizeof(ShiftKeys)/sizeof(ShiftKeys[0]);I++)
    if (Key==ShiftKeys[I])
      return(TRUE);
  return(FALSE);
}


void Editor::SetReplaceMode(int Mode)
{
  ::ReplaceMode=Mode;
}

int Editor::GetLineCurPos()
{
    return CurLine->EditLine.GetTabCurPos();
}

void Editor::BeginVBlockMarking()
{
    UnmarkBlock();
    VBlockStart=CurLine;
    VBlockX=CurLine->EditLine.GetTabCurPos();
    VBlockSizeX=0;
    VBlockY=NumLine;
    VBlockSizeY=1;
    MarkingVBlock=TRUE;
    BlockStartLine=NumLine;
    //_D(SysLog("BeginVBlockMarking, set vblock to  VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
}

void Editor::AdjustVBlock(int PrevX)
{
    int x=GetLineCurPos();
    int c2;

    //_D(SysLog("AdjustVBlock, x=%i,   vblock is VBlockY=%i:%i, VBlockX=%i:%i, PrevX=%i",x,VBlockY,VBlockSizeY,VBlockX,VBlockSizeX,PrevX));
    if ( x==VBlockX+VBlockSizeX)  // ничего не случилось, никаких табуляций нет
        return;
    if ( x>VBlockX )  // курсор убежал внутрь блока
    {
        VBlockSizeX=x-VBlockX;
        //_D(SysLog("x>VBlockX");
    }
    else if ( x<VBlockX ) // курсор убежал за начало блока
    {
        c2=VBlockX;
        if ( PrevX>VBlockX )    // сдвигались вправо, а пришли влево
        {
            VBlockX=x;
            VBlockSizeX=c2-x;   // меняем блок
        }
        else      // сдвигались влево и пришли еще больше влево
        {
            VBlockX=x;
            VBlockSizeX+=c2-x;  // расширяем блок
        }
        //_D(SysLog("x<VBlockX"));
    }
    else if (x==VBlockX && x!=PrevX)
    {
        VBlockSizeX=0;  // ширина в 0, потому прыгнули прям на табуляцию
        //_D(SysLog("x==VBlockX && x!=PrevX"));
    }
    // примечание
    //   случай x>VBLockX+VBlockSizeX не может быть
    //   потому что курсор прыгает назад на табуляцию, но не вперед

    //_D(SysLog("AdjustVBlock, changed vblock  VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
}


/* $ 24.09.2000 SVS
  Перекодировка Xlat
*/
void Editor::Xlat()
{
  struct EditList *CurPtr;
  int Line;

  if (VBlockStart!=NULL)
  {
    CurPtr=VBlockStart;

    for (Line=0;CurPtr!=NULL && Line<VBlockSizeY;Line++,CurPtr=CurPtr->Next)
    {
      int TBlockX=CurPtr->EditLine.TabPosToReal(VBlockX);
      int TBlockSizeX=CurPtr->EditLine.TabPosToReal(VBlockX+VBlockSizeX)-
                      CurPtr->EditLine.TabPosToReal(VBlockX);
      char *CurStr,*EndSeq;
      int Length;
      CurPtr->EditLine.GetBinaryString(&CurStr,&EndSeq,Length);
      int CopySize=Length-TBlockX;
      if (CopySize>TBlockSizeX)
         CopySize=TBlockSizeX;
      AddUndoData(CurPtr->EditLine.GetStringAddr(),BlockStartLine+Line,0,UNDO_EDIT);
      BlockUndo=TRUE;
      ::Xlat(CurPtr->EditLine.Str,TBlockX,TBlockX+CopySize,CurPtr->EditLine.TableSet,Opt.XLat.Flags);
    }
  }
  else
  {
    Line=0;
    CurPtr=BlockStart;
    /* $ 25.11.2000 IS
         Если нет выделения, то обработаем текущее слово. Слово определяется на
         основе специальной группы разделителей.
    */
    if(CurPtr!=NULL)
    {
      while (CurPtr!=NULL)
      {
        int StartSel,EndSel;
        CurPtr->EditLine.GetSelection(StartSel,EndSel);
        if (StartSel==-1)
          break;
        if(EndSel == -1)
          EndSel=strlen(CurPtr->EditLine.Str);
        AddUndoData(CurPtr->EditLine.GetStringAddr(),BlockStartLine+Line,0,UNDO_EDIT);
        ::Xlat(CurPtr->EditLine.Str,StartSel,EndSel,CurPtr->EditLine.TableSet,Opt.XLat.Flags);
        BlockUndo=TRUE;
        Line++;
        CurPtr=CurPtr->Next;
      }
    }
    else
    {
      char *Str=CurLine->EditLine.Str;
      int start=CurLine->EditLine.GetCurPos(), end, StrSize=strlen(Str);
      /* $ 10.12.2000 IS
         Обрабатываем только то слово, на котором стоит курсор, или то слово,
         что находится левее позиции курсора на 1 символ
      */
      BOOL DoXlat=TRUE;

      if(strchr(Opt.XLat.WordDivForXlat,Str[start])!=NULL)
      {
         if(start) start--;
         DoXlat=(strchr(Opt.XLat.WordDivForXlat,Str[start])==NULL);
      }

      if(DoXlat)
      {
        while(start>=0 && strchr(Opt.XLat.WordDivForXlat,Str[start])==NULL)
          start--;
        start++;
        end=start+1;
        while(end<StrSize && strchr(Opt.XLat.WordDivForXlat,Str[end])==NULL)
          end++;
        AddUndoData(Str,NumLine,start,UNDO_EDIT);
        ::Xlat(Str,start,end,CurLine->EditLine.TableSet,Opt.XLat.Flags);
      }
     /* IS $ */
    }
    /* IS $ */
  }
  BlockUndo=FALSE;
  TextChanged(1);
}
/* SVS $ */

/* $ 13.02.2001
     Узнаем атрибуты файла и заодно сформируем готовую строку атрибутов для
     статуса.
*/
DWORD Editor::GetFileAttributes(LPCTSTR Name)
{
  DWORD attr=::GetFileAttributes(Name);
  int ind=0;
  if(0xFFFFFFFF!=attr)
  {
     if(attr&FILE_ATTRIBUTE_READONLY) AttrStr[ind++]='R';
     if(attr&FILE_ATTRIBUTE_SYSTEM) AttrStr[ind++]='S';
     if(attr&FILE_ATTRIBUTE_HIDDEN) AttrStr[ind++]='H';
  }
  AttrStr[ind]=0;
  return attr;
}
/* IS $ */

/* $ 15.02.2001 IS
     Манипуляции с табуляцией на уровне всего загруженного файла.
     Может быть длительной во времени операцией, но тут уж, imho,
     ничего не поделать.
*/
//Обновим размер табуляции
void Editor::SetTabSize(int NewSize)
{
  if (NewSize<1 || NewSize>512)
    NewSize=8;
  if(NewSize!=EdOpt.TabSize) /* Меняем размер табуляции только в том случае, если он
                          на самом деле изменился */
  {
    EdOpt.TabSize=NewSize;
    struct EditList *CurPtr=TopList;
    while (CurPtr!=NULL)
    {
      CurPtr->EditLine.SetTabSize(NewSize);
      CurPtr=CurPtr->Next;
    }
  }
}

// обновим режим пробелы вместо табуляции
// операция необратима, кстати, т.е. пробелы на табуляцию обратно не изменятся
void Editor::SetConvertTabs(int NewMode)
{
  if(NewMode!=EdOpt.ExpandTabs) /* Меняем режим только в том случае, если он
                              на самом деле изменился */
  {
    EdOpt.ExpandTabs=NewMode;
    struct EditList *CurPtr=TopList;
    while (CurPtr!=NULL)
    {
      CurPtr->EditLine.SetConvertTabs(NewMode);
      CurPtr->EditLine.ReplaceTabs();
      CurPtr=CurPtr->Next;
    }
  }
}
/* IS $ */

/* $ 15.02.2001 IS
     + Эпопея продолжается :) Обновим установки EdOpt.DelRemovesBlocks и
       EdOpt.PersistentBlocks
*/
void Editor::SetDelRemovesBlocks(int NewMode)
{
  if(NewMode!=EdOpt.DelRemovesBlocks)
  {
    EdOpt.DelRemovesBlocks=NewMode;
    struct EditList *CurPtr=TopList;
    while (CurPtr!=NULL)
    {
      CurPtr->EditLine.SetDelRemovesBlocks(NewMode);
      CurPtr=CurPtr->Next;
    }
  }
}

void Editor::SetPersistentBlocks(int NewMode)
{
  if(NewMode!=EdOpt.PersistentBlocks)
  {
    EdOpt.PersistentBlocks=NewMode;
    struct EditList *CurPtr=TopList;
    while (CurPtr!=NULL)
    {
      CurPtr->EditLine.SetPersistentBlocks(NewMode);
      CurPtr=CurPtr->Next;
    }
  }
}
/* IS $ */

/* $ 26.02.2001 IS
     "Курсор за пределами строки"
*/
void Editor::SetCursorBeyondEOL(int NewMode)
{
  if(NewMode!=EdOpt.CursorBeyondEOL)
  {
    EdOpt.CursorBeyondEOL=NewMode;
    struct EditList *CurPtr=TopList;
    while (CurPtr!=NULL)
    {
      CurPtr->EditLine.SetEditBeyondEnd(NewMode);
      CurPtr=CurPtr->Next;
    }
  }
  /* $ 16.10.2001 SKV
    Если переключились туда сюда этот режим,
    то из-за этой штуки возникают нехилые глюки
    при выделении вертикальных блоков.
  */
  if(EdOpt.CursorBeyondEOL)
  {
    MaxRightPos=0;
  }
  /* SKV$*/
}
/* IS $ */

/* $ 29.10.200 IS
     Работа с настройками "сохранять позицию файла" и
     "сохранять закладки" после смены настроек по alt-shift-f9.
*/
void Editor::GetSavePosMode(int &SavePos, int &SaveShortPos)
{
   SavePos=EdOpt.SavePos;
   SaveShortPos=EdOpt.SaveShortPos;
}

// передавайте в качестве значения параметра "-1" для параметра,
// который не нужно менять
void Editor::SetSavePosMode(int SavePos, int SaveShortPos)
{
   if(SavePos!=-1)
      EdOpt.SavePos=SavePos;
   if(SaveShortPos!=-1)
      EdOpt.SaveShortPos=SaveShortPos;
}
/* IS $ */

void Editor::EditorShowMsg(const char *Title,const char *Msg, const char* Name)
{
  Message(0,0,Title,Msg,Name);
  PreRedrawParam.Param1=(void *)Title;
  PreRedrawParam.Param2=(void *)Msg;
  PreRedrawParam.Param3=(void *)Name;
}

void Editor::PR_EditorShowMsg(void)
{
  Editor::EditorShowMsg((char*)PreRedrawParam.Param1,(char*)PreRedrawParam.Param2,(char*)PreRedrawParam.Param3);
}

#endif //!defined(EDITOR2)
