#ifndef __FRAME_HPP__
#define __FRAME_HPP__

/*
frame.hpp

Немодальное окно (базовый класс для FilePanels, FileEditor, FileViewer)

*/

/* Revision: 1.14 18.07.2001 $ */ 

/*
Modify:
  18.07.2001 OT
    VFMenu
  11.07.2001 OT
    Перенос CtrlAltShift в Manager
  09.07.2001 OT
    Исправление MacroMode для диалогов
  23.06.2001 OT
    - Решение проблемы "старика Мюллера"
  20.06.2001 tran
    * Refresh* внес в cpp из hpp - удобней отлаживать.
  30.05.2001 OT
    - Небольшая корректировка UnlockRefresh()
  26.05.2001 OT
    + Новый атрибут - DynamicallyBorn - показывает, статически или динамически был создан объект
    + SetDynamicallyBorn() и GetDynamicallyBorn()
    + Возможность блокировки перерисовки фрейма: LockRefreshCount, LockRefresh(),UnlockRefresh(),Refreshable()
  18.05.2001 DJ
    ! Функция SetExitCode() теперь виртуальная
  15.05.2001 OT
    ! NWZ -> NFZ
  14.05.2001 OT
    ! метод GetCanLoseFocus() стал виртуальным и введен параметр по умолчанию
  12.05.2001 DJ
    ! отрисовка по OnChangeFocus сделана дефолтным поведением
  12.05.2001 DJ
    + IsTopFrame(), GetMacroMode()
  07.05.2001 DJ
    ! причешем идентификаторы
  06.05.2001 DJ
    ! перетрях #include
  05.05.2001 DJ
    created
*/

#include "scrobj.hpp"

class KeyBar;

enum { MODALTYPE_VIRTUAL,
  MODALTYPE_PANELS,
  MODALTYPE_VIEWER,
  MODALTYPE_EDITOR,
  MODALTYPE_DIALOG,
  MODALTYPE_VMENU,
  MODALTYPE_HELP,
  MODALTYPE_USER
};

class Frame: virtual public ScreenObject
{
  friend class Manager;
  private:
//    Frame **ModalStack;
//    int  ModalStackCount, ModalStackSize;
    Frame *FrameToBack;
    Frame *NextModal,*PrevModal;

  protected:
    int  DynamicallyBorn;
    int  CanLoseFocus;
    int  ExitCode;
    int  KeyBarVisible;
    KeyBar *FrameKeyBar;
    int MacroMode;
    int LockRefreshCount;

  public:
    Frame();
    virtual ~Frame();

//    int ProcessKey(int Key);
//    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);

    virtual int GetCanLoseFocus(int DynamicMode=FALSE) { return(CanLoseFocus); };
    void SetCanLoseFocus(int Mode) { CanLoseFocus=Mode; };
    int  GetExitCode() { return ExitCode; };
    virtual void SetExitCode(int Code) { ExitCode=Code; };

    virtual BOOL IsFileModified() {return(FALSE);};
    virtual char *GetTypeName() {return "Modal";};
    virtual int GetTypeAndName(char *Type,char *Name) {return(MODALTYPE_VIRTUAL);};
    virtual int GetType() { return MODALTYPE_VIRTUAL; }

    virtual void OnDestroy();  // вызывается перед уничтожением окна
    virtual void OnCreate() {};   // вызывается перед созданием окна
    virtual void OnChangeFocus(int focus); // вызывается при смене фокуса
    virtual void Refresh() {OnChangeFocus(1);};  // Просто перерисоваться :)

    void SetKeyBar(KeyBar *FrameKeyBar);
    void UpdateKeyBar();
    virtual void RedrawKeyBar() { Frame::UpdateKeyBar(); };

    /* $ 12.05.2001 DJ */
    int IsTopFrame();
    virtual int GetMacroMode() { return MacroMode; }
    /* DJ $ */
    void Push(Frame* Modalized);
//    bool Pop();
//    Frame *operator[](int Index);
//    int operator[](Frame *ModalFarame);
//    int ModalCount() {return ModalStackCount;}
    void DestroyAllModal();
    void SetDynamicallyBorn(int Born) {DynamicallyBorn=Born;}
    int GetDynamicallyBorn(){return DynamicallyBorn;};
    void LockRefresh();
    void UnlockRefresh();
    int Refreshable();
    virtual int FastHide();
//    int IndexOf(Frame *aFrame);
    bool RemoveModal(Frame *aFrame);
    void ResizeConsole();
    bool HasSaveScreen();
//    bool ifFullConsole();

};

#endif // __FRAME_HPP__
