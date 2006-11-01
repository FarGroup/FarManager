#ifndef __EDIT_HPP__
#define __EDIT_HPP__
/*
edit.hpp

Реализация одиночной строки редактирования

*/

/* Revision: 1.36 25.05.2006 $ */

#include "scrobj.hpp"
#include "colors.hpp"
#include "bitflags.hpp"
#include "UnicodeString.hpp"

//изменить флаги (подвинуть убрав FEDITLINE_CONVERTTABS)

// Младший байт (маска 0xFF) юзается классом ScreenObject!!!
enum FLAGS_CLASS_EDITLINE{
  FEDITLINE_MARKINGBLOCK         = 0x00000100,
  FEDITLINE_DROPDOWNBOX          = 0x00000200,
  FEDITLINE_CLEARFLAG            = 0x00000400,
  FEDITLINE_PASSWORDMODE         = 0x00000800,
  FEDITLINE_EDITBEYONDEND        = 0x00001000,
  FEDITLINE_EDITORMODE           = 0x00002000,
  FEDITLINE_OVERTYPE             = 0x00004000,
  FEDITLINE_DELREMOVESBLOCKS     = 0x00008000,  // Del удаляет блоки (Opt.EditorDelRemovesBlocks)
  FEDITLINE_PERSISTENTBLOCKS     = 0x00010000,  // Постоянные блоки (Opt.EditorPersistentBlocks)
//  FEDITLINE_CONVERTTABS          = 0x00020000,
  FEDITLINE_READONLY             = 0x00040000,
  FEDITLINE_CURSORVISIBLE        = 0x00080000,
  // Если ни один из FEDITLINE_PARENT_ не указан (или указаны оба), то Edit
  // явно не в диалоге юзается.
  FEDITLINE_PARENT_SINGLELINE    = 0x00100000,  // обычная строка ввода в диалоге
  FEDITLINE_PARENT_MULTILINE     = 0x00200000,  // для будущего Memo-Edit (DI_EDITOR или DIF_MULTILINE)
  FEDITLINE_PARENT_EDITOR        = 0x00400000,  // "вверху" обычный редактор
};


class Dialog;
class Editor;

class Edit:public ScreenObject
{
  friend class Dialog;
  friend class Editor;

public:
	Edit  *m_next;
	Edit  *m_prev;

private:	
//    char  *Str;
    wchar_t *Str;

    int    StrSize;
    int    MaxLength;

//    char  *Mask;             // 12.08.2000 KM - Переменная для хранения маски ввода
    wchar_t *Mask;

    struct CharTableSet *TableSet;
    struct ColorItem *ColorList;
    int    ColorCount;

    int    Color;
    int    SelColor;
    int    ColorUnChanged;   // 28.07.2000 SVS - для диалога

    int    LeftPos;
    int    CurPos;
    int    PrevCurPos;       // 12.08.2000 KM - предыдущее положение курсора

    int    TabSize;          // 14.02.2001 IS - Размер табуляции - по умолчанию равен Opt.TabSize;

    int    TabExpandMode;

    int    SelStart;
    int    SelEnd;

    int    EndType;

    int    CursorSize;
    int    CursorPos;
//    const char *WordDiv;
    const wchar_t *WordDiv;

  private:
    void   DisplayObject();
    void   ShowString(const wchar_t *ShowStr,int TabSelStart,int TabSelEnd);
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
    int ProcessInsPlainText(void);
    int CheckCharMask(wchar_t Chr);
    int ProcessInsPath(int Key,int PrevSelStart=-1,int PrevSelEnd=0);

  public:
    Edit(ScreenObject *pOwner = NULL);
    ~Edit();

  public:
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
    void SetDelRemovesBlocks(int Mode) {Flags.Change(FEDITLINE_DELREMOVESBLOCKS,Mode);}
    int  GetDelRemovesBlocks(void) {return Flags.Check(FEDITLINE_DELREMOVESBLOCKS); }

    void SetPersistentBlocks(int Mode) {Flags.Change(FEDITLINE_PERSISTENTBLOCKS,Mode);}
    int  GetPersistentBlocks(void) {return Flags.Check(FEDITLINE_PERSISTENTBLOCKS); }
    /* IS $ */
    void  GetStringW(wchar_t *Str, int MaxSize);
    void  GetStringW(string &strStr);

    const wchar_t* GetStringAddrW();

    void  SetStringW(const wchar_t *Str);

    void  SetBinaryStringW(const wchar_t *Str,int Length);

    void  GetBinaryStringW (const wchar_t **Str, const wchar_t **EOL,int &Length);

    void  SetEOLW(const wchar_t *EOL);

    int   GetSelStringW(wchar_t *Str,int MaxSize);
    int   GetSelStringW (string &strStr);

    int   GetLength();

    void  InsertStringW(const wchar_t *Str);
    void  InsertBinaryStringW(const wchar_t *Str,int Length);
    /* $ 03.08.2000 KM
       Добавление параметра WholeWords для поиска целых слов
       в функцию Search.
    */
    int   Search(const wchar_t *Str,int Position,int Case,int WholeWords,int Reverse);
    /* KM $ */
    void  SetClearFlag(int Flag) {Flags.Change(FEDITLINE_CLEARFLAG,Flag);}
    int   GetClearFlag(void) {return Flags.Check(FEDITLINE_CLEARFLAG);}
    void  SetCurPos(int NewPos) {CurPos=NewPos;PrevCurPos=NewPos;}
    int   GetCurPos() {return(CurPos);}
    int   GetTabCurPos();
    int   GetLeftPos() {return(LeftPos);}
    void  SetLeftPos(int NewPos) {LeftPos=NewPos;}
    void  SetTabCurPos(int NewPos);
    void  SetPasswordMode(int Mode) {Flags.Change(FEDITLINE_PASSWORDMODE,Mode);};
    void  SetMaxLength(int Length) {MaxLength=Length;};
    /* $ 28.07.2000 SVS
       Получение максимального значения строки для потребностей Dialod API
    */
    int   GetMaxLength() {return MaxLength;};
    /* SVS $ */
    /* $ 12.08.2000 KM
       Функции установки и получения маски ввода
    */
    void  SetInputMaskW(const wchar_t *InputMask);
    const wchar_t* GetInputMaskW() {return Mask;}

    /* KM $ */
    void  SetOvertypeMode(int Mode) {Flags.Change(FEDITLINE_OVERTYPE,Mode);};
    int   GetOvertypeMode() {return Flags.Check(FEDITLINE_OVERTYPE);};

    void  SetConvertTabs(int Mode) { TabExpandMode = Mode;};
    int   GetConvertTabs() {return TabExpandMode;};

    int   RealPosToTab(int Pos);
    int   TabPosToReal(int Pos);
    void  SetTables(struct CharTableSet *TableSet);
    void  Select(int Start,int End);
    void  AddSelect(int Start,int End);
    void  GetSelection(int &Start,int &End);
    void  GetRealSelection(int &Start,int &End);
    void  SetEditBeyondEnd(int Mode) {Flags.Change(FEDITLINE_EDITBEYONDEND,Mode);};
    void  SetEditorMode(int Mode) {Flags.Change(FEDITLINE_EDITORMODE,Mode);};
    void  ReplaceTabs();

    void  InsertTab ();

    void  AddColor(struct ColorItem *col);
    int   DeleteColor(int ColorPos);
    int   GetColor(struct ColorItem *col,int Item);

#ifdef SHITHAPPENS
    void ReplaceSpaces(int i);
#endif
    /* $ 24.09.2000 SVS $
      Функция Xlat - перекодировка по принципу QWERTY <-> ЙЦУКЕН
    */
    /* $ 13.12.2000 SVS
       Дополнительный параметр в функции  Xlat()
    */
    void Xlat(BOOL All=FALSE);
    /* SVS $ */
    /* SVS $ */

    static void DisableEncode(int Disable);

    void SetDialogParent(DWORD Sets);
    void SetCursorType(int Visible,int Size);
    void GetCursorType(int &Visible,int &Size);
    int  GetReadOnly() {return Flags.Check(FEDITLINE_READONLY);}
    void SetReadOnly(int NewReadOnly) {Flags.Change(FEDITLINE_READONLY,NewReadOnly);}
    int  GetDropDownBox() {return Flags.Check(FEDITLINE_DROPDOWNBOX);}
    void SetDropDownBox(int NewDropDownBox) {Flags.Change(FEDITLINE_DROPDOWNBOX,NewDropDownBox);}
    void SetWordDiv(const wchar_t *WordDiv){Edit::WordDiv=WordDiv;}
};

#endif  // __EDIT_HPP__
