#ifndef __DLGEDIT_HPP__
#define __DLGEDIT_HPP__
/*
dlgedit.hpp

Одиночная строка редактирования для диалога (как наследник класса Edit)
Мультиредактор

*/

/*
  Сюда нужно перетащить из edit.hpp и editor.hpp все вещи,
  касаемые масок и.. все что относится только к диалогам
  Это пока только шаблон, заготовка для будущего перехода
*/

#include "scrobj.hpp"
#include "bitflags.hpp"
#include "edit.hpp"
#include "editor.hpp"

enum DLGEDITTYPE{
  DLGEDIT_MULTILINE,
  DLGEDIT_SINGLELINE,
};

class DlgEdit: public ScreenObject
{
  friend class Dialog;

  private: // приватные данные
    DLGEDITTYPE Type;

    Edit   *lineEdit;
#if defined(PROJECT_DI_MEMOEDIT)
    Editor *multiEdit;
#endif

  public:  // публичные данные
    BitFlags& Flags();

  private: // приватные методы
    virtual void DisplayObject();

  public:
    DlgEdit(ScreenObject *pOwner,DLGEDITTYPE Type);
    virtual ~DlgEdit();

  public: // публичные методы
    int   ProcessKey(int Key);
    int   ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);

    virtual void Show();
    virtual void SetPosition(int X1,int Y1,int X2,int Y2);
    virtual void GetPosition(int& X1,int& Y1,int& X2,int& Y2);

    virtual void Hide();
    virtual void Hide0();
    virtual void ShowConsoleTitle();
    virtual void SetScreenPosition();
    virtual void ResizeConsole();
    virtual int  VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);

    void  SetDialogParent(DWORD Sets);
    void  SetDropDownBox(int NewDropDownBox);
    void  SetPasswordMode(int Mode);

    int   GetMaxLength();
    void  SetMaxLength(int Length);
    int   GetLength();
    int   GetStrSize(int Row=-1);

    void  SetInputMaskW(const wchar_t *InputMask);
    const wchar_t* GetInputMaskW();

    void  SetOvertypeMode(int Mode);
    int   GetOvertypeMode();

    void  SetEditBeyondEnd(int Mode);

    void  SetClearFlag(int Flag);
    int   GetClearFlag(void);

    void  SetStringW(const wchar_t *Str);
    void  GetStringW(wchar_t *Str, int MaxSize,int Row=-1); // Row==-1 - current line
    void  GetStringW(string &strStr,int Row=-1);            // Row==-1 - current line
    const wchar_t* GetStringAddrW();

    void  SetCurPos(int NewCol, int NewRow=-1); // Row==-1 - current line
    int   GetCurPos();
    int   GetCurRow();

    void  SetPersistentBlocks(int Mode);
    int   GetPersistentBlocks(void);
    void  SetDelRemovesBlocks(int NewMode);
    int   GetDelRemovesBlocks(void);

    void  SetObjectColor(int Color,int SelColor=0xf,int ColorUnChanged=COL_DIALOGEDITUNCHANGED);
    long  GetObjectColor();
    int   GetObjectColorUnChanged();

    void  FastShow();
    int   GetLeftPos();
    void  SetLeftPos(int NewPos,int Row=-1); // Row==-1 - current line

    void  DeleteBlock();

    void  Select(int Start,int End);           // TODO: не учтено для multiline!
    void  GetSelection(int &Start,int &End);   // TODO: не учтено для multiline!

    void Xlat(BOOL All=FALSE);

    void SetCursorType(int Visible,int Size);
    void GetCursorType(int &Visible,int &Size);

    int  GetReadOnly();
    void SetReadOnly(int NewReadOnly);
};

#endif  // __DLGEDIT_HPP__
