#ifndef __EDIT_HPP__
#define __EDIT_HPP__
/*
edit.hpp

���������� ��������� ������ ��������������

*/

/* Revision: 1.36 25.05.2006 $ */

#include "scrobj.hpp"
#include "colors.hpp"
#include "bitflags.hpp"
#include "UnicodeString.hpp"

//�������� ����� (��������� ����� FEDITLINE_CONVERTTABS)

// ������� ���� (����� 0xFF) ������� ������� ScreenObject!!!
enum FLAGS_CLASS_EDITLINE{
  FEDITLINE_MARKINGBLOCK         = 0x00000100,
  FEDITLINE_DROPDOWNBOX          = 0x00000200,
  FEDITLINE_CLEARFLAG            = 0x00000400,
  FEDITLINE_PASSWORDMODE         = 0x00000800,
  FEDITLINE_EDITBEYONDEND        = 0x00001000,
  FEDITLINE_EDITORMODE           = 0x00002000,
  FEDITLINE_OVERTYPE             = 0x00004000,
  FEDITLINE_DELREMOVESBLOCKS     = 0x00008000,  // Del ������� ����� (Opt.EditorDelRemovesBlocks)
  FEDITLINE_PERSISTENTBLOCKS     = 0x00010000,  // ���������� ����� (Opt.EditorPersistentBlocks)
//  FEDITLINE_CONVERTTABS          = 0x00020000,
  FEDITLINE_READONLY             = 0x00040000,
  FEDITLINE_CURSORVISIBLE        = 0x00080000,
  // ���� �� ���� �� FEDITLINE_PARENT_ �� ������ (��� ������� ���), �� Edit
  // ���� �� � ������� �������.
  FEDITLINE_PARENT_SINGLELINE    = 0x00100000,  // ������� ������ ����� � �������
  FEDITLINE_PARENT_MULTILINE     = 0x00200000,  // ��� �������� Memo-Edit (DI_EDITOR ��� DIF_MULTILINE)
  FEDITLINE_PARENT_EDITOR        = 0x00400000,  // "������" ������� ��������
};


class Dialog;
class Editor;

class Edit:public ScreenObject
{
  friend class Dialog;
  friend class Editor;

  private:
//    char  *Str;
    wchar_t *Str;

    int    StrSize;
    int    MaxLength;

//    char  *Mask;             // 12.08.2000 KM - ���������� ��� �������� ����� �����
    wchar_t *Mask;

    struct CharTableSet *TableSet;
    struct ColorItem *ColorList;
    int    ColorCount;

    int    Color;
    int    SelColor;
    int    ColorUnChanged;   // 28.07.2000 SVS - ��� �������

    int    LeftPos;
    int    CurPos;
    int    PrevCurPos;       // 12.08.2000 KM - ���������� ��������� �������

    int    TabSize;          // 14.02.2001 IS - ������ ��������� - �� ��������� ����� Opt.TabSize;

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
       ���������� ������� ������� ������ Mask � ����������
       ��������� ��������� ��������� ������� � ������ �����,
       + ������� ����������� ���������� Str �� ��������� Mask.
    */
    int    GetNextCursorPos(int Position,int Where);
    void   RefreshStrByMask(int InitMode=FALSE);
    /* KM $ */
    /* $ 15.11.2000 KM
       ���������: �������� �� ������ � �����������
       �������� ��������, ������������ ������
    */
    int KeyMatchedMask(int Key);
    /* KM $ */

    int ProcessCtrlQ(void);
    int ProcessInsDate(void);
    int ProcessInsPlainText(void);
    int CheckCharMask(wchar_t Chr);
    int ProcessInsPath(int Key,int PrevSelStart=-1,int PrevSelEnd=0);

  public:
    Edit();
    ~Edit();

  public:
    void  FastShow();
    int   ProcessKey(int Key);
    int   ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    /* $ 28.07.2000 SVS
       ! ������� ��������� ������� Color,SelColor � ColorUnChanged!
    */
    void  SetObjectColor(int Color,int SelColor=0xf,int ColorUnChanged=COL_DIALOGEDITUNCHANGED);
    //   + ������� ��������� ������� Color,SelColor
    long  GetObjectColor() {return MAKELONG(Color,SelColor);}
    int   GetObjectColorUnChanged() {return ColorUnChanged;}
    /* SVS $*/
    /* $ 14.02.2001 IS
         ������� ������/������������ ������� ���������
    */
    void SetTabSize(int NewSize) { TabSize=NewSize; }
    int  GetTabSize(void) {return TabSize; }
    /* IS $ */
    /* $ 15.02.2001 IS
         ������� ������/������������ ������� �������� ��������������
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

    void  GetBinaryStringW (wchar_t *&Str, const wchar_t **EOL,int &Length);
    void  GetBinaryStringW (const wchar_t *&Str, const wchar_t **EOL,int &Length);

    void  SetEOLW(const wchar_t *EOL);

    int   GetSelStringW(wchar_t *Str,int MaxSize);
    int   GetSelStringW (string &strStr);

    int   GetLength();

    void  InsertStringW(const wchar_t *Str);
    void  InsertBinaryStringW(const wchar_t *Str,int Length);
    /* $ 03.08.2000 KM
       ���������� ��������� WholeWords ��� ������ ����� ����
       � ������� Search.
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
       ��������� ������������� �������� ������ ��� ������������ Dialod API
    */
    int   GetMaxLength() {return MaxLength;};
    /* SVS $ */
    /* $ 12.08.2000 KM
       ������� ��������� � ��������� ����� �����
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
      ������� Xlat - ������������� �� �������� QWERTY <-> ������
    */
    /* $ 13.12.2000 SVS
       �������������� �������� � �������  Xlat()
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


struct EditList
{
  struct EditList *Prev;
  struct EditList *Next;
  Edit EditLine;

  EditList (ScreenObject *pOwner) { EditLine.SetOwner (pOwner); }
};

#endif  // __EDIT_HPP__
