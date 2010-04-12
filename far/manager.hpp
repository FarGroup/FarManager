#ifndef __MANAGER_HPP__
#define __MANAGER_HPP__
/*
manager.hpp

Переключение между несколькими file panels, viewers, editors

*/

class Frame;

class Manager
{
#if defined(SYSLOG)
		friend void ManagerClass_Dump(char *Title,const Manager *m,FILE *fp);
#endif
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
		/* $ 15.05.2002 SKV
		  Теперь это список.
		*/
		/*Frame **SemiModalBackFrames;
		int SemiModalBackFramesCount;
		int SemiModalBackFramesSize;*/
		/* SKV $ */

		/* $ 15.05.2002 SKV
		  Так как есть полумодалы, что б не было путаницы,
		  заведём счётчик модальных editor/viewer'ов.
		  Дёргать его  надо ручками перед вызовом ExecuteModal.
		  А автоматом нельзя, так как ExecuteModal вызывается
		  1) не только для настоящих модалов (как это не пародоксально),
		  2) не только для editor/viewer'ов.
		*/
		int ModalEVCount;
		/* SKV $ */

		int  EndLoop;            // Признак выхода из цикла
		int  StartManager;
		INPUT_RECORD LastInputRecord;

		int ModalExitCode;

	private:
		void StartupMainloop();
		Frame *FrameMenu(); //    вместо void SelectFrame(); // show window menu (F12)

		BOOL Commit();         // завершает транзакцию по изменениям в очереди и стеке фреймов
		// Она в цикле вызывает себя, пока хотябы один из указателей отличен от NULL
		// Функции, "подмастерья начальника" - Commit'a
		// Иногда вызываются не только из него и из других мест
		void RefreshCommit();  //
		void DeactivateCommit(); //
		void ActivateCommit(); //
		void UpdateCommit();   // выполняется тогда, когда нужно заменить один фрейм на другой
		void InsertCommit();
		void DeleteCommit();
		void ExecuteCommit();
		void ModalizeCommit();
		void UnmodalizeCommit();

		int GetModalExitCode();

		/*void AddSemiModalBackFrame(Frame* frame);
		BOOL IsSemiModalBackFrame(Frame *frame);
		void RemoveSemiModalBackFrame(Frame* frame);*/

	public:
		Manager();
		~Manager();

	public:
		// Эти функции можно безопасно вызывать практически из любого места кода
		// они как бы накапливают информацию о том, что нужно будет сделать с фреймами при следующем вызове Commit()
		void InsertFrame(Frame *NewFrame, int Index=-1);
		void DeleteFrame(Frame *Deleted=NULL);
		void DeleteFrame(int Index);
		void DeactivateFrame(Frame *Deactivated,int Direction);
		void SwapTwoFrame(int Direction);
		void ActivateFrame(Frame *Activated);
		void ActivateFrame(int Index);
		void RefreshFrame(Frame *Refreshed=NULL);
		void RefreshFrame(int Index);

		//! Функции для запуска модальных фреймов.
		void ExecuteFrame(Frame *Executed);


		//! Входит в новый цикл обработки событий
		void ExecuteModal(Frame *Executed=NULL);
		//! Запускает немодальный фрейм в модальном режиме
		void ExecuteNonModal();
		//! Проверка того, что немодальный фрейм находится еще и на вершине стека.
		BOOL ifDoubleInstance(Frame* frame);

		//!  Функции, которые работают с очередью немодально фрейма.
		//  Сейчас используются только для хранения информаци о наличии запущенных объектов типа VFMenu
		void ModalizeFrame(Frame *Modalized=NULL, int Mode=TRUE);
		void UnmodalizeFrame(Frame *Unmodalized);

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

		/* $ 11.10.2001 IS
		   Подсчитать количество фреймов с указанным именем.
		*/
		int CountFramesWithName(const char *Name, BOOL IgnoreCase=TRUE);
		/* IS $ */

		BOOL IsPanelsActive(); // используется как признак WaitInMainLoop
		void SetFramePos(int NewPos);
		int  FindFrameByFile(int ModalType,char *FileName,char *Dir=NULL);
		BOOL ShowBackground();

		void EnterMainLoop();
		void ProcessMainLoop();
		void ExitMainLoop(int Ask);
		int ProcessKey(DWORD key);
		int ProcessMouse(MOUSE_EVENT_RECORD *me);

		void PluginsMenu(); // вызываем меню по F11
		/* $ 10.05.2001 DJ */
		void SwitchToPanels();
		/* DJ $ */

		INPUT_RECORD *GetLastInputRecord() { return &LastInputRecord; }
		void ResetLastInputRecord() { LastInputRecord.EventType=0; }

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
		/* $ 13.04.2002 KM
		  Для вызова ResizeConsole для всех NextModal у
		  модального фрейма.
		*/
		void ResizeAllModal(Frame *ModalFrame);
		/* KM $ */
		Frame *GetBottomFrame() { return (*this)[FramePos]; }

		BOOL ManagerIsDown() {return EndLoop;}
		BOOL ManagerStarted() {return StartManager;}

		void InitKeyBar(void);

		/* $ 15.05.2002 SKV
		  Так как нужно это в разных местах,
		  а глобальные счётчики не концептуально,
		  то лучше это делать тут.
		*/
		void EnterModalEV() {ModalEVCount++;}
		void ExitModalEV() {ModalEVCount--;}
		BOOL InModalEV() {return ModalEVCount!=0;}
		/* SKV $ */
		void ResizeAllFrame();

		// возвращает top-модал или сам фрейм, если у фрейма нету модалов
		Frame* GetTopModal();
};

extern Manager *FrameManager;

#endif  // __MANAGER_HPP__
