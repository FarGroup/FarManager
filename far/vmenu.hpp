#ifndef __VMENU_HPP__
#define __VMENU_HPP__
/*
vmenu.hpp

Обычное вертикальное меню
  а так же:
    * список в DI_COMBOBOX
    * ...

*/

/* Revision: 1.03 28.07.2000 $ */

/*
Modify:
  28.07.2000 SVS
   + Добавлены цветовые атрибуты (в переменных) и функции, связанные с
     атрибутами:
  22.07.2000 SVS
   !  AlwaysScrollBar изменен на ListBoxControl
  18.07.2000 SVS
    + Добавлена переменная класса AlwaysScrollBar, предназначенная
      для отображения (всегда, по мере надобности!) в элементах
      DI_LISTBOX & DI_COMBOBOX
    ! В связи с этим изменен вызов конструктора класса.
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

/* $ 28.07.2000 SVS
   Цветовые атрибуты - индексы в массиве цветов
*/
enum{
  VMenuColorBody=0,      // подложка
  VMenuColorBox=1,       // рамка
  VMenuColorTitle=2,     // заголовок - верхний и нижний
  VMenuColorText=3,      // Текст пункта
  VMenuColorHilite=4,    // HotKey
  VMenuColorSeparator=5, // separator
  VMenuColorSelected=6,  // Выбранный
  VMenuColorHSelect=7,   // Выбранный - HotKey
  VMenuColorScrollBar=8  // ScrollBar
};
/* SVS */

class VMenu:public Modal
{
  private:
    SaveScreen *SaveScr;
    char Title[100];
    char BottomTitle[100];
    int SelectPos,TopPos;
    int MaxHeight;
    int MaxLength;
    int DrawBackground,BoxType,WrapMode,ShowAmpersand;
    int UpdateRequired;
    int DialogStyle;
    int AutoHighlight;
    int CallCount;
    int PrevMacroMode;
    /* $ 18.07.2000 SVS
       + переменная, отвечающая за отображение scrollbar в
         DI_LISTBOX & DI_COMBOBOX
    */
    int ListBoxControl;
    /* SVS $ */

  protected:
    struct MenuItem *Item;
    int ItemCount;
    /* $ 28.07.2000 SVS
       Цветовые атрибуты
    */
    short Colors[9];
    /* SVS */

  private:
    void DisplayObject();
    void ShowMenu();

  public:
    /* $ 18.07.2000 SVS
       ! изменен вызов конструктора с учетом необходимости scrollbar в
         DI_LISTBOX & DI_COMBOBOX
         По умолчанию - зависит от настроек показа scrollbar в меню,
         т.е. не требуется. Для данных элементов (DI_LISTBOX & DI_COMBOBOX)
         параметр isListBoxControl должен быть равен TRUE.
    */
    VMenu(char *Title,struct MenuData *Data,int ItemCount,int MaxHeight=0,int isListBoxControl=FALSE);
    /* SVS $ */
    ~VMenu();
    void FastShow() {ShowMenu();}
    void Show();
    void Hide();

    void SetBottomTitle(char *BottomTitle);
    void SetDialogStyle(int Style) {DialogStyle=Style;SetColors(NULL);};
    void SetUpdateRequired(int SetUpdate) {UpdateRequired=SetUpdate;};
    void SetBoxType(int BoxType);
    void SetFlags(unsigned int Flags);

    void AssignHighlights(int Reverse);
    void SetColors(short *Colors=NULL);
    void GetColors(short *Colors);

    int  ProcessKey(int Key);
    int  ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);

    void DeleteItems();
    int  AddItem(struct MenuItem *NewItem);
    int  GetItemCount() {return(ItemCount);};
    int  GetUserData(void *Data,int Size,int Position=-1);
    int  GetSelectPos();
    int  GetSelection(int Position=-1);
    void SetSelection(int Selection,int Position=-1);
};

#endif	// __VMENU_HPP__
