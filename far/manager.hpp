#ifndef __MANAGER_HPP__
#define __MANAGER_HPP__
/*
manager.hpp

Переключение между несколькими file panels, viewers, editors

*/

/* Revision: 1.04 04.05.2001 $ */

/*
Modify:
  04.05.2001 DJ
    + доделка и переделка NWZ
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  29.12.2000 IS
    + Метод ExitAll
  28.06.2000 tran
    - NT Console resize bug
      add class member ActiveModal
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class Manager
{
  private:

    void ActivateNextWindow();

    Window **WindowList;
    Window *DestroyedWindow;

    int  EndLoop;

    int  WindowCount,
         WindowListSize;
    int  WindowPos;
    int  UpdateRequired;

    int  NextViewer;
    char NextName[NM];
    int  NextPos;

    INPUT_RECORD LastInputRecord;

    void SetCurrentWindow (Window *NewCurWindow);

  public:

    Manager();
    ~Manager();

    void AddWindow(Window *NewWindow);
    void DestroyWindow(Window *Killed);
    int ExecuteModal(Window &ModalWindow);

    void NextWindow(int Increment);
    void SelectWindow(); // show window menu (F12)

    void CloseAll();
    /* $ 29.12.2000 IS
         Аналог CloseAll, но разрешает продолжение полноценной работы в фаре,
         если пользователь продолжил редактировать файл.
         Возвращает TRUE, если все закрыли и можно выходить из фара.
    */
    BOOL ExitAll();
    /* IS $ */
    BOOL IsAnyWindowModified(int Activate);

    int  GetWindowCount() {return(WindowCount);};
    void GetWindowTypesCount(int &Viewers,int &Editors);
    int  GetWindowCountByType(int Type);

    BOOL IsPanelsActive(); // используется как признак WaitInMainLoop

    void SetWindowPos(int NewPos);

    int  FindWindowByFile(int ModalType,char *FileName);

    void ShowBackground();

    void SetNextWindow(int Viewer,char *Name,long Pos);

    // new methods
    void EnterMainLoop();
    void ProcessMainLoop();
    void ExitMainLoop(int Ask);
    int ProcessKey(int key);
    int ProcessMouse(MOUSE_EVENT_RECORD *me);

    void PluginsMenu(); // вызываем меню по F11
    void CheckExited();

    Window *CurrentWindow;  // текущий модал,
                          // присутсвует в списке, но может быть не активным
    int    EnableSwitch;  // разрешено ли переключение из модала

    INPUT_RECORD *GetLastInputRecord() { return &LastInputRecord; }
};

#endif  // __MANAGER_HPP__
