#pragma once

/*
manager.hpp

Переключение между несколькими file panels, viewers, editors
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

class Frame;

class Manager
{
#if defined(SYSLOG)
		friend void ManagerClass_Dump(const wchar_t *Title,const Manager *m,FILE *fp);
#endif
	private:
		INPUT_RECORD LastInputRecord;
		int  FrameCount;         // Размер немодальной очереди
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

		Frame **ModalStack;     // Стек модальных фреймов
		Frame **FrameList;       // Очередь модальных фреймов
		int ModalStackCount;    // Размер стека модальных фреймов
		int ModalStackSize;     // Буфер стека модальных фреймов

		int  FrameListSize;      // размер буфера под немодальную очередь
		int  FramePos;           // Индекс текущий немодального фрейма. Он не всегда совпадает с CurrentFrame
		// текущий немодальный фрейм можно получить с помощью FrameManager->GetBottomFrame();

		/* $ 15.05.2002 SKV
		  Так как есть полумодалы, что б не было путаницы,
		  заведём счётчик модальных editor/viewer'ов.
		  Дёргать его  надо ручками перед вызовом ExecuteModal.
		  А автоматом нельзя, так как ExecuteModal вызывается
		  1) не только для настоящих модалов (как это не пародоксально),
		  2) не только для editor/viewer'ов.
		*/
		int ModalEVCount;
		int  EndLoop;            // Признак выхода из цикла
		int ModalExitCode;
		int  StartManager;

	private:
		void StartupMainloop();
		Frame *FrameMenu(); //    вместо void SelectFrame(); // show window menu (F12)

		BOOL Commit();         // завершает транзакцию по изменениям в очереди и стеке фреймов
		// Она в цикле вызывает себя, пока хотябы один из указателей отличен от nullptr
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
		void DeleteFrame(Frame *Deleted=nullptr);
		void DeleteFrame(int Index);
		void DeactivateFrame(Frame *Deactivated,int Direction);
		void SwapTwoFrame(int Direction);
		void ActivateFrame(Frame *Activated);
		void ActivateFrame(int Index);
		void RefreshFrame(Frame *Refreshed=nullptr);
		void RefreshFrame(int Index);

		//! Функции для запуска модальных фреймов.
		void ExecuteFrame(Frame *Executed);


		//! Входит в новый цикл обработки событий
		void ExecuteModal(Frame *Executed=nullptr);
		//! Запускает немодальный фрейм в модальном режиме
		void ExecuteNonModal();
		//! Проверка того, что немодальный фрейм находится еще и на вершине стека.
		BOOL ifDoubleInstance(Frame* frame);

		//!  Функции, которые работают с очередью немодально фрейма.
		//  Сейчас используются только для хранения информаци о наличии запущенных объектов типа VFMenu
		void ModalizeFrame(Frame *Modalized=nullptr, int Mode=TRUE);
		void UnmodalizeFrame(Frame *Unmodalized);

		void CloseAll();
		/* $ 29.12.2000 IS
		     Аналог CloseAll, но разрешает продолжение полноценной работы в фаре,
		     если пользователь продолжил редактировать файл.
		     Возвращает TRUE, если все закрыли и можно выходить из фара.
		*/
		BOOL ExitAll();
		BOOL IsAnyFrameModified(int Activate);

		int  GetFrameCount()const {return(FrameCount);};
		int  GetFrameCountByType(int Type);

		/*$ 26.06.2001 SKV
		Для вызова через ACTL_COMMIT
		*/
		BOOL PluginCommit();

		int CountFramesWithName(const wchar_t *Name, BOOL IgnoreCase=TRUE);

		bool IsPanelsActive(); // используется как признак WaitInMainLoop
		void SetFramePos(int NewPos);
		int  FindFrameByFile(int ModalType,const wchar_t *FileName,const wchar_t *Dir=nullptr);
		BOOL ShowBackground();

		void EnterMainLoop();
		void ProcessMainLoop();
		void ExitMainLoop(int Ask);
		int ProcessKey(DWORD key);
		int ProcessMouse(MOUSE_EVENT_RECORD *me);

		void PluginsMenu(); // вызываем меню по F11
		void SwitchToPanels();

		INPUT_RECORD *GetLastInputRecord() { return &LastInputRecord; }
		void SetLastInputRecord(INPUT_RECORD *Rec);
		void ResetLastInputRecord() { LastInputRecord.EventType=0; }

		Frame *GetCurrentFrame() { return CurrentFrame; }

		Frame *operator[](size_t Index)const;

		int IndexOf(Frame *Frame);

		int IndexOfStack(Frame *Frame);
		int HaveAnyFrame();

		void ImmediateHide();
		/* $ 13.04.2002 KM
		  Для вызова ResizeConsole для всех NextModal у
		  модального фрейма.
		*/
		void ResizeAllModal(Frame *ModalFrame);

		Frame *GetBottomFrame() { return (*this)[FramePos]; }

		BOOL ManagerIsDown() {return EndLoop;}
		BOOL ManagerStarted() {return StartManager;}

		void InitKeyBar();

		/* $ 15.05.2002 SKV
		  Так как нужно это в разных местах,
		  а глобальные счётчики не концептуально,
		  то лучше это делать тут.
		*/
		void EnterModalEV() {ModalEVCount++;}
		void ExitModalEV() {ModalEVCount--;}
		BOOL InModalEV() {return ModalEVCount;}

		void ResizeAllFrame();

		// возвращает top-модал или сам фрейм, если у фрейма нету модалов
		Frame* GetTopModal();

		int GetModalStackCount() const {return ModalStackCount;}
		Frame* GetModalFrame(size_t index) const {return ModalStack[index];}
};

extern Manager *FrameManager;
extern long CurrentWindowType;
