#ifndef __DIALOG_HPP__
#define __DIALOG_HPP__
/*
dialog.hpp

Класс диалога Dialog.

Предназначен для отображения модальных диалогов.
Является производным от класса Modal.

*/

/* Revision: 1.04 26.07.2000 $ */

/*
Modify:
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
    int lastKey;		// для AutoComplit последняя клавиша
    int InitObjects;            // элементы инициализарованы?
    int CreateObjects;          // объекты (Edit,...) созданы?
    int WarningStyle;           // TRUE - Warning Dialog Style
    int DialogTooLong;          //
    int PrevMacroMode;          // предыдущий режим макро
    FARDIALOGPROC DlgProc;      // функция обработки диалога

  private:
    void DisplayObject();
    void DeleteDialogObjects();
    void ShowDialog();
    int ChangeFocus(int FocusPos,int Step,int SkipGroup);
    int IsEdit(int Type);
    /* $ 26.07.2000 SVS
      + Дополнительный параметр в SelectFromEditHistory для выделения
       нужной позиции в истории (если она соответствует строке ввода)
    */
    void SelectFromEditHistory(Edit *EditLine,char *HistoryName,char *Str);
    /* SVS $ */
    /* $ 18.07.2000 SVS
       + функция SelectFromComboBox для выбора из DI_COMBOBOX
    */
    void SelectFromComboBox(Edit *EditLine,struct FarListItem *HistoryName);
    /* SVS $ */
    /* $ 26.07.2000 SVS
       AutoComplite: Поиск входжение подстроки в истории
    */
    int FindInEditHistory(Edit *EditLine,char *HistoryName,char *FindStr);
    /* SVS $ */
    void AddToEditHistory(char *AddStr,char *HistoryName);
    int ProcessHighlighting(int Key,int FocusPos,int Translate);

  public:
    Dialog(struct DialogItem *Item,int ItemCount,FARDIALOGPROC DlgProc=NULL,long Param=NULL);
    ~Dialog();

  public:
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void Show();
    void FastShow() {ShowDialog();}
    void InitDialogObjects();
    void GetDialogObjectsData();
    void SetWarningStyle(int Style) {WarningStyle=Style;};

    static void DataToItem(struct DialogData *Data,struct DialogItem *Item,
                           int Count);
    static int IsKeyHighlighted(char *Str,int Key,int Translate);
    /* $ 23.07.2000 SVS
       функция обработки диалога (по умолчанию)
    */
    static long WINAPI DefDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);
    /* SVS $ */
};

#endif // __DIALOG_HPP__
