#ifndef __FRAME_HPP__
#define __FRAME_HPP__

/*
frame.hpp

Немодальное окно (базовый класс для FilePanels, FileEditor, FileViewer)

*/

/* Revision: 1.23 29.05.2006 $ */

/*
Modify:
  29.05.2006 SVS
    + GetTitle()
  24.07.2005 WARP
    ! see 02033.LockUnlock.txt
  30.06.2004 SVS
    + GetTopModal() - ответ на вопрос "как добраться до верхнего модала?"
  15.05.2002 SVS
    ! Сделаем виртуальный метод Frame::InitKeyBar и будем его вызывать
      для всех Frame в методе Manager::InitKeyBar.
  28.04.2002 KM
    + MODALTYPE_COMBOBOX
  20.12.2001 IS
    - Баг: MODALTYPE_* не были синхронизированы с WTYPE_*
  02.11.2001 SVS
    ! возвращаемое значение у GetTypeName() - модификатор const
  04.10.2001 OT
    ! Отмена 956 патча
  18.07.2001 OT
    ! VFMenu
  11.07.2001 OT
    ! Перенос CtrlAltShift в Manager
  09.07.2001 OT
    - Исправление MacroMode для диалогов
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

// ПРОСЬБА НЕ ЗАБЫВАТЬ СИНХРОНИЗИРОВАТЬ ИЗМЕНЕНИЯ
// WTYPE_* (plugin.hpp) и MODALTYPE_*!!!
// (и не надо убирать этот комментарий, пока ситуация не изменится ;)
enum { MODALTYPE_VIRTUAL,
  MODALTYPE_PANELS=1,
  MODALTYPE_VIEWER,
  MODALTYPE_EDITOR,
  MODALTYPE_DIALOG,
  MODALTYPE_VMENU,
  MODALTYPE_HELP,
  MODALTYPE_COMBOBOX,
  MODALTYPE_USER,
};

class Frame: public virtual ScreenObject
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

    virtual const char *GetTypeName() {return "[FarModal]";};
    virtual int GetTypeAndName(char *Type,char *Name) {return(MODALTYPE_VIRTUAL);};
    virtual int GetType() { return MODALTYPE_VIRTUAL; }

    virtual void OnDestroy();  // вызывается перед уничтожением окна
    virtual void OnCreate() {};   // вызывается перед созданием окна
    virtual void OnChangeFocus(int focus); // вызывается при смене фокуса
    virtual void Refresh() {OnChangeFocus(1);};  // Просто перерисоваться :)

    virtual void InitKeyBar(void) {}
    void SetKeyBar(KeyBar *FrameKeyBar);
    void UpdateKeyBar();
    virtual void RedrawKeyBar() { Frame::UpdateKeyBar(); };

    /* $ 12.05.2001 DJ */
    int IsTopFrame();
    virtual int GetMacroMode() { return MacroMode; }
    /* DJ $ */
    void Push(Frame* Modalized);
    Frame *GetTopModal(){return NextModal;};
//    bool Pop();
//    Frame *operator[](int Index);
//    int operator[](Frame *ModalFarame);
//    int ModalCount() {return ModalStackCount;}
    void DestroyAllModal();
    void SetDynamicallyBorn(int Born) {DynamicallyBorn=Born;}
    int GetDynamicallyBorn(){return DynamicallyBorn;};
    virtual int FastHide();
//    int IndexOf(Frame *aFrame);
    bool RemoveModal(Frame *aFrame);
    void ResizeConsole();
    bool HasSaveScreen();
//    bool ifFullConsole();
    virtual void GetTitle(char *Title,int LenTitle,int TruncSize=0){};

};

#endif // __FRAME_HPP__
