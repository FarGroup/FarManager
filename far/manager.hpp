#ifndef __MANAGER_HPP__
#define __MANAGER_HPP__
/*
manager.hpp

Переключение между несколькими file panels, viewers, editors

*/

/* Revision: 1.03 29.04.2001 $ */

/*
Modify:
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

    Modal **ModalList,
          **ActiveList;

    int  ModalCount,
         ModalSizeList,
         ActiveListCount;
    int  ModalPos;
    int  NextViewer;
    char NextName[NM];
    int  NextPos;
    int  UpdateRequired;

  public:

    Manager();
    ~Manager();

    void AddModal(Modal *NewModal);
    void ExecuteModal(Modal *ExecModal);
    void DestroyModal(Modal *KilledModal);

    void NextModal(int Increment);
    void SelectModal(); // show window menu (F12)

    void PushActive();
    void PopActive();

    void CloseAll();
    /* $ 29.12.2000 IS
         Аналог CloseAll, но разрешает продолжение полноценной работы в фаре,
         если пользователь продолжил редактировать файл.
         Возвращает TRUE, если все закрыли и можно выходить из фара.
    */
    BOOL ExitAll();
    /* IS $ */
    BOOL IsAnyModalModified(int Activate);

    int  GetModalCount() {return(ModalCount);};
    void GetModalTypesCount(int &Viewers,int &Editors);
    int  GetModalCountByType(int Type);

    BOOL IsPanelsActive(); // используется как признак WaitInMainLoop

    void SetModalPos(int NewPos);

    int  FindModalByFile(int ModalType,char *FileName);

    void ShowBackground();

    void SetNextWindow(int Viewer,char *Name,long Pos);

    // new methods
    int ProcessKey(int key);
    int ProcessMouse(MOUSE_EVENT_RECORD *me);

    void PluginsMenu(); // вызываем меню по F11
    void CheckExited();

    Modal *ActiveModal;   // активный модал, в том числе и диалог
                          // может не быть в списке
    Modal *CurrentModal;  // текущий модал,
                          // присутсвует в списке, но может быть не активным
    int    EnableSwitch;  // разрешено ли переключение из модала
};

#endif  // __MANAGER_HPP__
