#ifndef __VMENU_HPP__
#define __VMENU_HPP__
/*
vmenu.hpp

Обычное вертикальное меню

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class VMenu:public Modal
{
  private:
    void DisplayObject();
    void ShowMenu();
    SaveScreen *SaveScr;
    struct MenuItem *Item;
    int ItemCount;
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
  public:
    VMenu(char *Title,struct MenuData *Data,int ItemCount,int MaxHeight=0);
    ~VMenu();
    void DeleteItems();
    void FastShow() {ShowMenu();}
    void Show();
    void Hide();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    int AddItem(struct MenuItem *NewItem);
    void SetBottomTitle(char *BottomTitle);
    void SetDialogStyle(int Style) {DialogStyle=Style;};
    void SetUpdateRequired(int SetUpdate) {UpdateRequired=SetUpdate;};
    void SetBoxType(int BoxType);
    void SetFlags(unsigned int Flags);
    int GetUserData(void *Data,int Size,int Position=-1);
    int GetSelection(int Position=-1);
    void SetSelection(int Selection,int Position=-1);
    int GetSelectPos();
    int GetItemCount() {return(ItemCount);};
    void AssignHighlights(int Reverse);
};

#endif	// __VMENU_HPP__
