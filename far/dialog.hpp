#ifndef __DIALOG_HPP__
#define __DIALOG_HPP__
/*
dialog.hpp

Класс диалога

*/

/* Revision: 1.01 18.07.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
  18.07.2000 SVS
    + функция SelectFromComboBox для выбора из DI_COMBOBOX
*/

class Dialog:public Modal
{
  private:
    void DisplayObject();
    void DeleteDialogObjects();
    void ShowDialog();
    int ChangeFocus(int FocusPos,int Step,int SkipGroup);
    int IsEdit(int Type);
    void SelectFromEditHistory(Edit *EditLine,char *HistoryName);
    /* $ 18.07.2000 SVS
       + функция SelectFromComboBox для выбора из DI_COMBOBOX
    */
    void SelectFromComboBox(Edit *EditLine,struct FarListItem *HistoryName);
    /* SVS $ */
    void AddToEditHistory(char *AddStr,char *HistoryName);
    int ProcessHighlighting(int Key,int FocusPos,int Translate);

    struct DialogItem *Item;

    char OldConsoleTitle[512];
    int ItemCount;
    int InitObjects;
    int CreateObjects;
    int WarningStyle;
    int DialogTooLong;
    int PrevMacroMode;
  public:
    Dialog(struct DialogItem *Item,int ItemCount);
    ~Dialog();
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
};

#endif // __DIALOG_HPP__
