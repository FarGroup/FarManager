#ifndef __EDIT_HPP__
#define __EDIT_HPP__
/*
edit.hpp

Реализация одиночной строки редактирования

*/

/* Revision: 1.05 03.08.2000 $ */

/*
Modify:
  03.08.2000 KM
    ! Изменены входные параметры функции Search - добавлен параметр int WholeWords.
    ! Добавление в поиск функциональности нахождения целых слов
  28.07.2000 SVS
    + Получение максимального значения строки для потребностей
       Dialod API - GetMaxLength()
    + Функция получения текущих Color,SelColor
    + Переменная класса ColorUnChanged (для диалога)
    + Функция GetObjectColorUnChanged получения текущего ColorUnChanged
    ! SetObjectColor имеет дополнительный параметр для установки ColorUnChanged
  25.07.2000 tran
    + DropDownBox style
  21.07.2000 tran
    + ReplaceSpaces()
  03.07.2000 tran
    + ReadOnly style
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class Edit:public ScreenObject
{
  private:
    void   DisplayObject();
    void   ShowString(char *ShowStr,int TabSelStart,int TabSelEnd);
    int    InsertKey(int Key);
    int    RecurseProcessKey(int Key);
    void   DeleteBlock();
    void   ApplyColor();
    char  *Str;
    struct CharTableSet *TableSet;
    struct ColorItem *ColorList;
    int    ColorCount;
    int    StrSize;
    int    CurPos;
    /* $ 28.07.2000 SVS
      + ColorUnChanged (для диалога)
    */
    int    Color,SelColor,ColorUnChanged;
    /* SVS $ */
    int    LeftPos;
    int    ConvertTabs;
    int    CursorPos;
    int    EndType;
    int    MaxLength;
    int    SelStart,SelEnd;
    char   MarkingBlock;
    char   ClearFlag;
    char   PasswordMode;
    char   EditBeyondEnd;
    char   Overtype;
    char   EditorMode;
    /* $ 03.07.2000 tran
       + ReadOnly style*/
    int    ReadOnly;
    /* tran 03.07.2000 $ */

  public:

    Edit();
    ~Edit();
    void  Show() {DisplayObject();}
    void  FastShow();
    int   ProcessKey(int Key);
    int   ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    /* $ 28.07.2000 SVS
       ! Функция установки текущих Color,SelColor и ColorUnChanged!
    */
    void  SetObjectColor(int Color,int SelColor=0xf,int ColorUnChanged=COL_DIALOGEDITUNCHANGED);
    //   + Функция получения текущих Color,SelColor
    long  GetObjectColor() {return MAKELONG(Color,SelColor);}
    int   GetObjectColorUnChanged() {return ColorUnChanged;}
    /* SVS $*/
    void  GetString(char *Str,int MaxSize);
    char* GetStringAddr();
    void  SetString(char *Str);
    void  SetBinaryString(char *Str,int Length);
    void  GetBinaryString(char **Str,char **EOL,int &Length);
    void  SetEOL(char *EOL);
    int   GetSelString(char *Str,int MaxSize);
    int   GetLength();
    void  InsertString(char *Str);
    void  InsertBinaryString(char *Str,int Length);
    /* $ 03.08.2000 KM
       Добавление параметра WholeWords для поиска целых слов
       в функцию Search.
    */
    int   Search(char *Str,int Position,int Case,int WholeWords,int Reverse);
    /* KM $ */
    void  SetClearFlag(int Flag) {ClearFlag=Flag;}
    void  SetCurPos(int NewPos) {CurPos=NewPos;}
    int   GetCurPos() {return(CurPos);}
    int   GetTabCurPos();
    int   GetLeftPos() {return(LeftPos);}
    void  SetLeftPos(int NewPos) {LeftPos=NewPos;}
    void  SetTabCurPos(int NewPos);
    void  SetPasswordMode(int Mode) {PasswordMode=Mode;};
    void  SetMaxLength(int Length) {MaxLength=Length;};
    /* $ 28.07.2000 SVS
       Получение максимального значения строки для потребностей Dialod API
    */
    int   GetMaxLength() {return MaxLength;};
    /* SVS $ */
    void  SetOvertypeMode(int Mode) {Overtype=Mode;};
    int   GetOvertypeMode() {return(Overtype);};
    void  SetConvertTabs(int Mode) {ConvertTabs=Mode;};
    int   RealPosToTab(int Pos);
    int   TabPosToReal(int Pos);
    void  SetTables(struct CharTableSet *TableSet);
    void  Select(int Start,int End);
    void  AddSelect(int Start,int End);
    void  GetSelection(int &Start,int &End);
    void  GetRealSelection(int &Start,int &End);
    void  SetEditBeyondEnd(int Mode) {EditBeyondEnd=Mode;};
    void  SetEditorMode(int Mode) {EditorMode=Mode;};
    void  ReplaceTabs();
    void  AddColor(struct ColorItem *col);
    int   DeleteColor(int ColorPos);
    int   GetColor(struct ColorItem *col,int Item);

    static void DisableEditOut(int Disable);
    static void DisableEncode(int Disable);
#ifdef SHITHAPPENS
    void ReplaceSpaces(int i);
#endif
    /* $ 25.07.2000 tran
       + DropDownBox style */
    int DropDownBox;
    /* tran 25.07.2000 $ */

};

#endif  // __EDIT_HPP__
