#ifndef __EDIT_HPP__
#define __EDIT_HPP__
/*
edit.hpp

Реализация одиночной строки редактирования

*/

/* Revision: 1.17 23.11.2001 $ */

/*
Modify:
  23.11.2001 SVS
    ! IsDialogParent может иметь 3 значения: EDPARENT_*
    + Вместо длинного "if(Mask[...)" введена функция CheckCharMask()
  08.11.2001 SVS
    + IsDialogParent - а родитель у нас кто?
  22.06.2001 SVS
    + ProcessInsDate()
  06.05.2001 DJ
    ! перетрях #include
  13.04.2001 SVS
    ! Обработка Ctrl-Q вынесена в отдельную функцию ProcessCtrlQ(), т.к.
      используется в editor.cpp
  15.02.2001 IS
    + Локальные переменные, в которых запоминается то, что храниться в
      настройках редактора:
      DelRemovesBlocks - "Del удаляет блоки"
      PersistentBlocks - "Постоянные блоки"
    + Функции для управления их состоянием:
      SetDelRemovesBlocks/GetDelRemovesBlocks
      SetPersistentBlocks/GetPersistentBlocks
  14.02.2001 IS
    + Размер табуляции хранится в TabSize, манипулировать им можно при помощи
      GetTabSize, SetTabSize
  13.12.2000 SVS
    + Дополнительный параметр в функции  Xlat()
  16.11.2000 KM & SVS
    + KeyMatchedMask - Проверяет: попадает ли символ в разрешённый
      диапазон символов, пропускаемых маской
    ! Кометика (от SVS :-)
  24.09.2000 SVS
    + Класс Editor - друг
    + Функция Xlat - перекодировка по принципу QWERTY <-> ЙЦУКЕН
  18.09.2000 SVS
    + класс Dialog является "другом" (т.е. полноправным совладельцем)
  12.08.2000 KM 1.06
    + Новые функции SetInputMask и GetInputMask для установки и получения маски ввода.
    + Новая переменная Mask, которая хранит маску ввода для данного объекта Edit.
    + Новая функция GetNextCursorPos, вычисляющая следующее положение курсора
      в строке ввода с учётом Mask.
    + Новая функция RefreshStrByMask.
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

#include "scrobj.hpp"
#include "colors.hpp"

// Константы для Edit::IsDialogParent
enum {
  EDPARENT_NONE=0,       // это явно не в диалоге юзается
  EDPARENT_SINGLELINE=1, // обычная строка ввода в диалоге
  EDPARENT_MULTILINE=2,  // для будущего Memo-Edit (DI_EDITOR или DIF_MULTILINE)
};

class Dialog;
class Editor;

class Edit:public ScreenObject
{
  friend class Dialog;
  friend class Editor;

  private:
    char  *Str;
    /* $ 12.08.2000 KM
       Переменная для хранения маски ввода
    */
    char  *Mask;
    /* KM $ */
    struct CharTableSet *TableSet;
    struct ColorItem *ColorList;
    int    ColorCount;
    int    StrSize;
    /* $ 12.08.2000 KM
       Добавлена переменная для хранения предыдущего положения курсора
    */
    int    CurPos;
    int    PrevCurPos;
    /* KM $ */
    /* $ 28.07.2000 SVS
      + ColorUnChanged (для диалога)
    */
    int    Color;
    int    SelColor;
    int    ColorUnChanged;
    /* SVS $ */
    int    LeftPos;
    int    ConvertTabs;
    int    CursorPos;
    int    EndType;
    int    MaxLength;
    int    SelStart;
    int    SelEnd;
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
    /* $ 14.02.2001 IS
         Размер табуляции - по умолчанию равен Opt.TabSize;
    */
    int    TabSize;
    /* IS $ */
    /* $ 15.02.2001 IS
         Различные опции из настроек редактора теперь запоминаются локально
    */
    int    DelRemovesBlocks; // Del удаляет блоки (Opt.EditorDelRemovesBlocks)
    int    PersistentBlocks; // Постоянные блоки (Opt.EditorPersistentBlocks)
    /* IS $ */

    int    IsDialogParent;   // Признак принадлежности строки к диалогу

  private:
    void   DisplayObject();
    void   ShowString(char *ShowStr,int TabSelStart,int TabSelEnd);
    int    InsertKey(int Key);
    int    RecurseProcessKey(int Key);
    void   DeleteBlock();
    void   ApplyColor();
    /* $ 12.08.2000 KM
       Внутренняя функция которая парсит Mask и возвращает
       следующее возможное положение курсора в строке ввода,
       + функция обновляющая содержимое Str на основании Mask.
    */
    int    GetNextCursorPos(int Position,int Where);
    void   RefreshStrByMask(int InitMode=FALSE);
    /* KM $ */
    /* $ 15.11.2000 KM
       Проверяет: попадает ли символ в разрешённый
       диапазон символов, пропускаемых маской
    */
    int KeyMatchedMask(int Key);
    /* KM $ */

    int ProcessCtrlQ(void);
    int ProcessInsDate(void);
    int CheckCharMask(char Chr);

  public:
    Edit();
    ~Edit();

  public:
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
    /* $ 14.02.2001 IS
         Функции чтения/установления размера табуляции
    */
    void SetTabSize(int NewSize) { TabSize=NewSize; }
    int  GetTabSize(void) {return TabSize; }
    /* IS $ */
    /* $ 15.02.2001 IS
         Функции чтения/установления текущих настроек редактирования
    */
    void SetDelRemovesBlocks(int NewMode) { DelRemovesBlocks=NewMode; }
    int  GetDelRemovesBlocks(void) {return DelRemovesBlocks; }

    void SetPersistentBlocks(int NewMode) { PersistentBlocks=NewMode; }
    int  GetPersistentBlocks(void) {return PersistentBlocks; }
    /* IS $ */
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
    void  SetCurPos(int NewPos) {CurPos=NewPos;PrevCurPos=NewPos;}
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
    /* $ 12.08.2000 KM
       Функции установки и получения маски ввода
    */
    void  SetInputMask(char *InputMask);
    char* GetInputMask() {return Mask;}
    /* KM $ */
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

#ifdef SHITHAPPENS
    void ReplaceSpaces(int i);
#endif
    /* $ 25.07.2000 tran
       + DropDownBox style */
    int DropDownBox;
    /* tran 25.07.2000 $ */
    /* $ 24.09.2000 SVS $
      Функция Xlat - перекодировка по принципу QWERTY <-> ЙЦУКЕН
    */
    /* $ 13.12.2000 SVS
       Дополнительный параметр в функции  Xlat()
    */
    void Xlat(BOOL All=FALSE);
    /* SVS $ */
    /* SVS $ */

    static void DisableEditOut(int Disable);
    static void DisableEncode(int Disable);

    void SetDialogParent(int Sets) {IsDialogParent=Sets;}
    int  GetDialogParent() {return IsDialogParent;}
};

#endif  // __EDIT_HPP__
