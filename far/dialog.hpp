#ifndef __DIALOG_HPP__
#define __DIALOG_HPP__
/*
dialog.hpp

Класс диалога Dialog.

Предназначен для отображения модальных диалогов.
Является производным от класса Modal.

*/

/* Revision: 1.10 10.08.2000 $ */

/*
Modify:
  10.08.2000 SVS
   + переменная IsMovedDialog - можно ли двигать диалог :-)
   + функция установки IsMovedDialog
  09.08.2000 KM 1.09
   + Добавление функции проверки на режим перемещения диалога.
     Кстати новер редакции действительно 1.09 - один был пропущен.
  01.08.2000 SVS
   - переменная класса lastKey удалена за ненадобностью :-)
  31.07.2000 tran & SVS
   + переменная класса Dragged - флаг перемещения
     а также OldX*, OldY*,
     метод - AdjustEditPos(int dx,int dy) - подравнивает координаты Edit"ов
   + Сохранение того, что под индикатором перемещения диалога
  28.07.2000 SVS
   + Переменная класса InitParam - хранит параметр, переданный
     в диалог.
   ! Теперь InitDialogObjects возвращает ID элемента с фокусом ввода
   + Функция ChangeFocus2
     Изменяет фокус ввода между двумя элементами.
   ! Переметр Edit *EditLine в функции FindInEditForAC нафиг ненужен!
   ! FindInEditHistory -> FindInEditForAC
     Поиск как в истории, так и в ComboBox`е (чтобы не пладить кода)
   ! SelectFromComboBox имеет дополнительный параметр с тем, чтобы
     позиционировать item в меню со списком в соответсвии со строкой ввода
   + Функция IsFocused, определяющая - "Может ли элемент диалога
     иметь фокус ввода"
   ! IsEdit стал Static-методом!
   + функция посылки сообщений диалогу SendDlgMessage
   + Функция ConvertItem - преобразования из внутреннего представления
     в FarDialogItem и обратно
  26.07.2000 SVS
   + FindInEditHistory: Поиск входжение подстроки в истории
  25.07.2000 SVS
   + Private: lastKey - для AutoComplit последняя клавиша
   + Дополнительный параметр в SelectFromEditHistory для выделения
     нужной позиции в истории (если она соответствует строке ввода)
  25.07.2000 SVS
   ! новый параметр в конструкторе
  23.07.2000 SVS
   + Куча ремарок в исходниках :-)
   + Функция обработки диалога (по умолчанию) - забито место :-)
   ! Изменен вызов конструктора
  18.07.2000 SVS
    + функция SelectFromComboBox для выбора из DI_COMBOBOX
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class Dialog:public Modal
{
  private:
    struct DialogItem *Item;    // массив элементов диалога
    int ItemCount;              // количество элементов диалога

    char OldConsoleTitle[512];  // предыдущий заголовок консоли
    int InitObjects;            // элементы инициализарованы?
    int CreateObjects;          // объекты (Edit,...) созданы?
    int WarningStyle;           // TRUE - Warning Dialog Style
    int DialogTooLong;          //
    int PrevMacroMode;          // предыдущий режим макро
    long InitParam;		// параметр, переданный в конструктор
    FARWINDOWPROC DlgProc;      // функция обработки диалога

    /* $ 31.07.2000 tran
       переменные для перемещения диалога */
    int  Dragged;
    int  OldX1,OldX2,OldY1,OldY2;
    /* tran 31.07.2000 $ */
    /* $ 10.08.2000 SVS
       можно ли двигать диалог :-)
    */
    int IsMovedDialog;
    /* SVS $ */

    /* $ 31.07.2000 SVS
       Сохранение того, что под индикатором перемещения диалога
    */
    CHAR_INFO LV[4];
    /* SVS $ */

  private:
    void DisplayObject();
    void DeleteDialogObjects();
    void ShowDialog();

    /* $ 28.07.2000 SVS
       + Изменяет фокус ввода между двумя элементами.
         Вынесен отдельно для того, чтобы обработать DMSG_KILLFOCUS & DMSG_SETFOCUS
    */
    int ChangeFocus2(int KillFocusPos,int SetFocusPos);
    /* SVS $ */
    int ChangeFocus(int FocusPos,int Step,int SkipGroup);
    static int IsEdit(int Type);
    /* $ 28.07.2000 SVS
       Функция, определяющая - "Может ли элемент диалога иметь фокус ввода"
    */
    static int IsFocused(int Type);
    /* SVS $ */
    /* $ 26.07.2000 SVS
      + Дополнительный параметр в SelectFromEditHistory для выделения
       нужной позиции в истории (если она соответствует строке ввода)
    */
    void SelectFromEditHistory(Edit *EditLine,char *HistoryName,char *Str);
    /* SVS $ */
    /* $ 18.07.2000 SVS
       + функция SelectFromComboBox для выбора из DI_COMBOBOX
    */
    void SelectFromComboBox(Edit *EditLine,struct FarListItems *HistoryName,char *Str=NULL);
    /* SVS $ */
    /* $ 26.07.2000 SVS
       AutoComplite: Поиск входжение подстроки в истории
    */
    int FindInEditForAC(int TypeFind,void *HistoryName,char *FindStr);
    /* SVS $ */
    void AddToEditHistory(char *AddStr,char *HistoryName);
    int ProcessHighlighting(int Key,int FocusPos,int Translate);

  public:
    Dialog(struct DialogItem *Item,int ItemCount,FARWINDOWPROC DlgProc=NULL,long Param=NULL);
    ~Dialog();

  public:
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void Show();
    void FastShow() {ShowDialog();}
    /* $ 28.07.2000 SVS
       Теперь InitDialogObjects возвращает ID элемента
       с фокусом ввода
    */
    int  InitDialogObjects();
    /* SVS $ */
    void GetDialogObjectsData();
    void SetWarningStyle(int Style) {WarningStyle=Style;};

    /* $ 28.07.2000 SVS
       + Функция ConvertItem - преобразования из внутреннего представления
        в FarDialogItem и обратно
    */
    static void ConvertItem(int FromPlugin,struct FarDialogItem *Item,struct DialogItem *Data,
                           int Count);
    /* SVS $ */
    static void DataToItem(struct DialogData *Data,struct DialogItem *Item,
                           int Count);
    static int IsKeyHighlighted(char *Str,int Key,int Translate);
    /* $ 23.07.2000 SVS
       функция обработки диалога (по умолчанию)
    */
    static long WINAPI DefDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);
    /* SVS $ */
    /* $ 28.07.2000 SVS
       функция посылки сообщений диалогу
    */
    static long WINAPI SendDlgMessage(HANDLE hDlg,int Msg,int Param1,long Param2);
    /* SVS $ */

    /* $ 31.07.2000 tran
       метод для перемещения диалога */
    void AdjustEditPos(int dx,int dy);
    /* tran 31.07.2000 $ */

    /* $ 09.08.2000 KM
       Добавление функции, которая позволяет проверить
       находится ли диалог в режиме перемещения.
    */
    int IsMoving() {return Dragged;}
    /* KM $ */
    /* $ 10.08.2000 SVS
       можно ли двигать диалог :-)
    */
    void SetModeMoving(int IsMoving) {IsMovedDialog=IsMoving;};
    int  GetModeMoving(void) {return IsMovedDialog;};
    /* SVS $ */
};

#endif // __DIALOG_HPP__
