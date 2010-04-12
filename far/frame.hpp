#ifndef __FRAME_HPP__
#define __FRAME_HPP__

/*
frame.hpp

Немодальное окно (базовый класс для FilePanels, FileEditor, FileViewer)

*/

#include "scrobj.hpp"

class KeyBar;

// ПРОСЬБА НЕ ЗАБЫВАТЬ СИНХРОНИЗИРОВАТЬ ИЗМЕНЕНИЯ
// WTYPE_* (plugin.hpp) и MODALTYPE_*!!!
// (и не надо убирать этот комментарий, пока ситуация не изменится ;)
enum
{
	MODALTYPE_VIRTUAL,
	MODALTYPE_PANELS=1,
	MODALTYPE_VIEWER,
	MODALTYPE_EDITOR,
	MODALTYPE_DIALOG,
	MODALTYPE_VMENU,
	MODALTYPE_HELP,
	MODALTYPE_COMBOBOX,
	MODALTYPE_FINDFOLDER,
	MODALTYPE_USER,
};

class Frame: public ScreenObject
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
		int  TitleBarVisible;
		KeyBar *FrameKeyBar;
		int MacroMode;

	public:
		Frame();
		virtual ~Frame();

//  virtual int ProcessKey(int Key);
//  virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);

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

		int IsTitleBarVisible() const {return TitleBarVisible;};

		/* $ 12.05.2001 DJ */
		int IsTopFrame();
		virtual int GetMacroMode() { return MacroMode; }
		/* DJ $ */
		void Push(Frame* Modalized);
		Frame *GetTopModal() {return NextModal;};
//    bool Pop();
//    Frame *operator[](int Index);
//    int operator[](Frame *ModalFarame);
//    int ModalCount() {return ModalStackCount;}
		void DestroyAllModal();
		void SetDynamicallyBorn(int Born) {DynamicallyBorn=Born;}
		int GetDynamicallyBorn() {return DynamicallyBorn;};
		virtual int FastHide();
//    int IndexOf(Frame *aFrame);
		bool RemoveModal(Frame *aFrame);
		virtual void ResizeConsole();
		bool HasSaveScreen();
//    bool ifFullConsole();
		virtual const char *GetTitle(char *Title,int LenTitle,int TruncSize=0) { return NULL; };

};

#endif // __FRAME_HPP__
