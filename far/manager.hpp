#ifndef __MANAGER_HPP__
#define __MANAGER_HPP__
/*
manager.hpp

Переключение между несколькими file panels, viewers, editors

*/

/* Revision: 1.20 19.07.2001 $ */ 

/*
Modify:
  19.07.2001 OT
    Добавились новые члены и методв типа UnmodalizeХХХ + мини документация
  18.07.2001 OT
    VFMenu
  11.07.2001 OT
    Перенос CtrlAltShift в Manager
  26.06.2001 SKV
    + PluginCommit(); (ACTL_COMMIT)
  06.06.2001 OT
    - Перемудрил зачем-то с ExecuteFrame()...
  28.05.2001 OT
    ! RefreshFrame() по умолчанию обновляет текущий фрейм
  26.05.2001 OT
    + Новые методы ExecuteComit(), ExecuteFrame(), IndexOfStack()
    + Новый член Frame *ExecutedFrameж
    - удаленные методы и члены типа Destruct
    - удаление метода ModalSaveState()
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
    Frame **ModalStack;     // Стек модальных фреймов
    int ModalStackCount;    // Размер стека модальных фреймов
    int ModalStackSize;     // Буфер стека модальных фреймов

    Frame **FrameList;       // Очередь модальных фреймов
    int  FrameCount;         // Размер немодальной очереди
    int  FrameListSize;      // размер буфера под немодальную очередь
    int  FramePos;           // Индекс текущий немодального фрейма. Он не всегда совпадает с CurrentFrame
                             // текущий немодальный фрейм можно получить с помощью FrameManager->GetBottomFrame();

    /*$ Претенденты на ... */
    Frame *InsertedFrame;   // Фрейм, который будет добавлен в конец немодальной очереди
    Frame *DeletedFrame;    // Фрейм, предназначений для удаления из модальной очереди, из модального стека, либо одиночный (которого нет ни там, ни там)
    Frame *ActivatedFrame;  // Фрейм, который необходимо активировать после каких нибудь изменений  
    Frame *RefreshedFrame;  // Фрейм, который нужно просто освежить, т.е. перерисовать
    Frame *ModalizedFrame;  // Фрейм, который становится в "очередь" к текущему немодальному фрейму
    Frame *UnmodalizedFrame;// Фрейм, убираюющийся из "очереди" немодального фрейма
    Frame *DeactivatedFrame;// Фрейм, который указывает на предудущий активный фрейм
    Frame *ExecutedFrame;   // Фрейм, которого вскорости нужно будет поставить на вершину модального сттека

    Frame *CurrentFrame;     // текущий фрейм. Он может нахлодиться как в немодальной очереди, так и в можальном стеке
                             // его можно получить с помощью FrameManager->GetCurrentFrame();


    int  EndLoop;            // Признак выхода из цикла
    INPUT_RECORD LastInputRecord; 
    void StartupMainloop();
    void FrameMenu(); //    вместо void SelectFrame(); // show window menu (F12)

    BOOL Commit();         // завершает транзакцию по изменениям в очереди и стеке фреймов
                           // Она в цикле вызывает себя, пока хотябы один из указателей отличен от NULL
    // Функции, "подмастерья начальника" - Commit'a
    void RefreshCommit();  // 
    void ActivateCommit(); // 
    void UpdateCommit();   // выполняется тогда, когда нужно заменить один фрейм на другой
    void InsertCommit();
    void DeleteCommit();
    void ExecuteCommit();
    void ModalizeCommit();  
    void UnmodalizeCommit();

  public:
    Manager();
    ~Manager();

  public:
    // Эти функции можно безопасно вызывать практически из любого места кода
    // они как бы накапливают информацию о том, что нужно будет сделать с фреймами при следующем вызове Commit()
    void InsertFrame(Frame *NewFrame, int Index=-1); 
    void DeleteFrame(Frame *Deleted=NULL);           
    void DeleteFrame(int Index);
    void DeactivateFrame (Frame *Deactivated,int Direction);
    void ActivateFrame (Frame *Activated);
    void ActivateFrame (int Index);  //вместо ActivateFrameByPos (int NewPos);
    void RefreshFrame(Frame *Refreshed=NULL);
    void RefreshFrame(int Index);

    // Функции для запуска модальных фреймов.
    void ExecuteFrame(Frame *Executed);
    void ExecuteModal (Frame *Executed=NULL); // возвращает то, что возвращает ModalExitCode();

    //  Функции, которые работают с очередью немодально фрейма. 
    //  Сейчас используются только для хранения информаци о наличии запущенных объектов типа VFMenu
    void ModalizeFrame (Frame *Modalized=NULL, int Mode=TRUE);
    void UnmodalizeFrame (Frame *Unmodalized);
  private:
    int GetModalExitCode();
    int ModalExitCode;
  public:
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

    /*$ 26.06.2001 SKV
    Для вызова через ACTL_COMMIT
    */
    BOOL PluginCommit();
    /* SKV$*/

    BOOL IsPanelsActive(); // используется как признак WaitInMainLoop
    void SetFramePos(int NewPos);
    int  FindFrameByFile(int ModalType,char *FileName,char *Dir=NULL);
    void ShowBackground();

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

    Frame *operator[](int Index);

    /* $ 19.05.2001 DJ
       operator[] (Frame *) -> IndexOf
    */
    int IndexOf(Frame *Frame);
    /* DJ $ */

    int IndexOfStack(Frame *Frame);
    int HaveAnyFrame();

/* $ Введена для нужд CtrlAltShift OT */
    void ImmediateHide();
    Frame *GetBottomFrame() { return (*this)[FramePos]; }
};

extern Manager *FrameManager;

#endif  // __MANAGER_HPP__
