#ifndef __MANAGER_HPP__
#define __MANAGER_HPP__
/*
manager.hpp

Переключение между несколькими file panels, viewers, editors

*/

/* Revision: 1.13 21.05.2001 $ */

/*
Modify:
  21.05.2001 OT
    + Добавился RefreshedFrame
  21.05.2001 DJ
    ! чистка внутренностей; в связи с появлением нового типа фреймов
      выкинуто GetFrameTypesCount(); отложенное удаление фрейма
  16.05.2001 DJ
    ! возвращение ExecuteModal()
  15.05.2001 OT
    ! NWZ -> NFZ
  14.05.2001 OT
    ! Изменение порядка вызова параметров ReplaceFrame (для единообразия и удобства)
  12.05.2001 DJ
    ! FrameManager оторван от CtrlObject, выкинут ExecuteModalPtr,
      ReplaceCurrentFrame заменен на ReplaceFrame, GetCurrentFrame()
  10.05.2001 DJ
    + SwitchToPanels(), ModalStack, ModalSaveState(), ExecuteModalPtr()
  06.05.2001 DJ
    ! перетрях #include
    + ReplaceCurrentFrame(), ActivateFrameByPos()
  06.05.2001 ОТ
    ! Переименование Window в Frame :)
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

class Frame;

class Manager
{
  private:
    Frame **FrameList;
    /* $ 16.05.2001 DJ */
    Frame **ModalStack;
    int ModalStackSize, ModalStackCount;
    /* DJ $ */

    /*$ Претенденты на ... */
    Frame *InsertedFrame;
    Frame *DeletedFrame;   // DestroyedFrame
    Frame *ActivatedFrame; // претендент на то, чтобы стать
//    Frame *UpdatedFrame;   // FrameToReplace
//    Frame *UpdatingFrame;  //
    Frame *RefreshedFrame;
    Frame *ModalizedFrame;
    Frame *DeactivatedFrame;
    /* OT $*/
    /* $ 21.05.2001 DJ */
    Frame *FrameToDestruct;  // отложенное удаление для корректной посылки OnChangeFocus(0)
    /* DJ $ */
      
    Frame *CurrentFrame;     // текущий модал

    int DisableDelete;

    int  EndLoop;
    int  FrameCount,
         FrameListSize;
    int  FramePos;

    INPUT_RECORD LastInputRecord;

    void ModalSaveState();

  private:

    void StartupMainloop();
    void FrameMenu(); //    вместо void SelectFrame(); // show window menu (F12)

    // Исполнение приговора
    BOOL Commit();
    void RefreshCommit();
    void ActivateCommit();
    void UpdateCommit();
    void InsertCommit();
    void DeleteCommit();
    /* $ 21.05.2001 DJ */
    void DestructCommit();
    /* DJ $ */

  public:
    Manager();
    ~Manager();

  public:
    // Эти функции вызываются из объектов ядра
    void InsertFrame(Frame *NewFrame, int Index=-1);
    void DeleteFrame(Frame *Deleted=NULL);
    void DeleteFrame(int Index);
    void ModalizeFrame (Frame *Modalized=NULL, int Mode=TRUE); // вместо ExecuteModal
    void DeactivateFrame (Frame *Deactivated,int Direction);
    void ActivateFrame (Frame *Activated);
    void ActivateFrame (int Index);  //вместо ActivateFrameByPos (int NewPos);
    void RefreshFrame(Frame *Refreshed);
    void RefreshFrame(int Index);


    int ExecuteModal (Frame &ModalFrame);

    void CloseAll();
    /* $ 29.12.2000 IS
         Аналог CloseAll, но разрешает продолжение полноценной работы в фаре,
         если пользователь продолжил редактировать файл.
         Возвращает TRUE, если все закрыли и можно выходить из фара.
    */
    BOOL ExitAll();
    /* IS $ */
    BOOL IsAnyFrameModified(int Activate);

    int  GetFrameCount() {return(FrameCount);};
    int  GetFrameCountByType(int Type);

    BOOL IsPanelsActive(); // используется как признак WaitInMainLoop
    void SetFramePos(int NewPos);
    int  FindFrameByFile(int ModalType,char *FileName,char *Dir=NULL);
    void ShowBackground();

    // new methods
    void EnterMainLoop();
    void ProcessMainLoop();
    void ExitMainLoop(int Ask);
    int ProcessKey(int key);
    int ProcessMouse(MOUSE_EVENT_RECORD *me);

    void PluginsMenu(); // вызываем меню по F11
    /* $ 10.05.2001 DJ */
    void SwitchToPanels();
    /* DJ $ */

    INPUT_RECORD *GetLastInputRecord() { return &LastInputRecord; }

    /* $ 12.05.2001 DJ */
    Frame *GetCurrentFrame() { return CurrentFrame; }
    /* DJ $ */

    /*$ 13.05.2001 OT */
    Frame *operator[](int Index);

    /* $ 19.05.2001 DJ
       operator[] (Frame *) -> IndexOf
    */
    int IndexOf(Frame *Frame);
    /* DJ $ */
};

extern Manager *FrameManager;

#endif  // __MANAGER_HPP__
