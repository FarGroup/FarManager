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

class Manager: NonCopyable
{
public:
	struct Key
	{
		INPUT_RECORD Event;
		int FarKey;
		bool EventFilled;
		Key(): Event(), FarKey(0), EventFilled(false) {}
		explicit Key(int Key): Event(), FarKey(Key), EventFilled(false) {}
		//Key(INPUT_RECORD Key): EventFilled(true), Event(Key) {FarKey=0; /*FIXME*/ }
	};
public:
	Manager();

	// Эти функции можно безопасно вызывать практически из любого места кода
	// они как бы накапливают информацию о том, что нужно будет сделать с фреймами при следующем вызове Commit()
	void InsertFrame(Frame *NewFrame);
	void DeleteFrame(Frame *Deleted = nullptr);
	void DeleteFrame(int Index);
	void DeactivateFrame(Frame *Deactivated, int Direction);
	void ActivateFrame(Frame *Activated);
	void ActivateFrame(int Index);
	void RefreshFrame(Frame *Refreshed = nullptr);
	void RefreshFrame(int Index);
	void UpdateFrame(Frame* Old,Frame* New);
	void CallbackFrame(const std::function<void(void)>& Callback);
	//! Функции для запуска модальных фреймов.
	void ExecuteFrame(Frame *Executed);
	//! Входит в новый цикл обработки событий
	void ExecuteModal(Frame *Executed = nullptr);
	//! Запускает немодальный фрейм в модальном режиме
	void ExecuteNonModal(Frame *NonModal);
	void CloseAll();
	/* $ 29.12.2000 IS
	Аналог CloseAll, но разрешает продолжение полноценной работы в фаре,
	если пользователь продолжил редактировать файл.
	Возвращает TRUE, если все закрыли и можно выходить из фара.
	*/
	BOOL ExitAll();
	size_t GetFrameCount()const { return Frames.size(); }
	int  GetFrameCountByType(int Type);
	/*$ 26.06.2001 SKV
	Для вызова через ACTL_COMMIT
	*/
	void PluginCommit();
	int CountFramesWithName(const string& Name, BOOL IgnoreCase = TRUE);
	bool IsPanelsActive(bool and_not_qview = false) const; // используется как признак Global->WaitInMainLoop
	int  FindFrameByFile(int ModalType, const string& FileName, const wchar_t *Dir = nullptr);
	void EnterMainLoop();
	void ProcessMainLoop();
	void ExitMainLoop(int Ask);
	int ProcessKey(Key key);
	int ProcessMouse(const MOUSE_EVENT_RECORD *me);
	void PluginsMenu() const; // вызываем меню по F11
	void SwitchToPanels();
	INPUT_RECORD *GetLastInputRecord() { return &LastInputRecord; }
	void SetLastInputRecord(const INPUT_RECORD *Rec);
	void ResetLastInputRecord() { LastInputRecord.EventType = 0; }
	Frame *GetCurrentFrame() { return CurrentFrame; }
	Frame* GetFrame(size_t Index) const;
	int IndexOf(Frame *Frame);
	int IndexOfStack(Frame *Frame);
	void ImmediateHide();
	Frame *GetBottomFrame() { return GetFrame(FramePos); }
	BOOL ManagerIsDown() const { return EndLoop; }
	BOOL ManagerStarted() const { return StartManager; }
	void InitKeyBar();
	/* $ 15.05.2002 SKV
	Так как нужно это в разных местах,
	а глобальные счётчики не концептуально,
	то лучше это делать тут.
	*/
	void EnterModalEV() { ModalEVCount++; }
	void ExitModalEV() { ModalEVCount--; }
	BOOL InModalEV() const { return ModalEVCount; }
	void ResizeAllFrame();
	size_t GetModalStackCount() const { return ModalFrames.size(); }
	Frame* GetModalFrame(size_t index) const { return ModalFrames[index]; }
	/* $ 13.04.2002 KM
	Для вызова ResizeConsole для всех NextModal у
	модального фрейма.
	*/
	static long GetCurrentWindowType() { return CurrentWindowType; }
	static bool ShowBackground();

private:
#if defined(SYSLOG)
	friend void ManagerClass_Dump(const wchar_t *Title, FILE *fp);
#endif

	Frame *FrameMenu(); //    вместо void SelectFrame(); // show window menu (F12)
	bool HaveAnyFrame() const;
	void Commit(void);         // завершает транзакцию по изменениям в очереди и стеке фреймов
	// Она в цикле вызывает себя, пока хотябы один из указателей отличен от nullptr
	// Функции, "подмастерья начальника" - Commit'a
	// Иногда вызываются не только из него и из других мест
	void InsertCommit(Frame* Param);
	void DeleteCommit(Frame* Param);
	void ActivateCommit(Frame* Param);
	void ActivateCommit(int Index);
	void RefreshCommit(Frame* Param);
	void DeactivateCommit(Frame* Param);
	void ExecuteCommit(Frame* Param);
	void UpdateCommit(Frame* Old,Frame* New);
	int GetModalExitCode() const;

	INPUT_RECORD LastInputRecord;
	Frame *CurrentFrame;     // текущий фрейм. Он может нахлодиться как в немодальной очереди, так и в можальном стеке, его можно получить с помощью FrameManager->GetCurrentFrame();
	std::vector<Frame*> ModalFrames;     // Стек модальных фреймов
	std::vector<Frame*> Frames;       // Очередь модальных фреймов
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
	static long CurrentWindowType;
private:
	class MessageAbstract
	{
		public:
			virtual ~MessageAbstract() {}
			virtual bool Process(void)=0;
	};
	class MessageCallback: public MessageAbstract
	{
		private:
			std::function<void(void)> m_Callback;
		public:
			MessageCallback(const std::function<void(void)>& Callback): m_Callback(Callback) {}
			virtual bool Process(void) override {m_Callback();return true;}
	};
	class MessageOneFrame: public MessageAbstract
	{
		private:
			Frame* m_Param;
			std::function<void(Frame*)> m_Callback;
		public:
			MessageOneFrame(Frame* Param,const std::function<void(Frame*)>& Callback): m_Param(Param),m_Callback(Callback) {}
			virtual bool Process(void) override {m_Callback(m_Param);return true;}
	};
	class MessageTwoFrames: public MessageAbstract
	{
		private:
			Frame* m_Param1;
			Frame* m_Param2;
			std::function<void(Frame*,Frame*)> m_Callback;
		public:
			MessageTwoFrames(Frame* Param1,Frame* Param2,const std::function<void(Frame*,Frame*)>& Callback): m_Param1(Param1),m_Param2(Param2),m_Callback(Callback) {}
			virtual bool Process(void) override {m_Callback(m_Param1,m_Param2);return true;}
	};
	class MessageStop: public MessageAbstract
	{
		public:
			MessageStop() {}
			virtual bool Process(void) override {return false;}
	};

	std::list<std::unique_ptr<MessageAbstract>> m_Queue;
	void PushFrame(Frame* Param,void(Manager::*Callback)(Frame*));
	void CheckAndPushFrame(Frame* Param,void(Manager::*Callback)(Frame*));
	void ProcessFrameByPos(int Index,void(Manager::*Callback)(Frame*));
	void RedeleteFrame(Frame *Deleted);
};
