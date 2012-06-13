#pragma once

/*
frame.hpp

Немодальное окно (базовый класс для FilePanels, FileEditor, FileViewer)
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

#include "scrobj.hpp"


class KeyBar;

enum MODALFRAME_TYPE
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

//    virtual int ProcessKey(int Key);
//    virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);

		virtual int GetCanLoseFocus(int DynamicMode=FALSE) { return(CanLoseFocus); }
		void SetCanLoseFocus(int Mode) { CanLoseFocus=Mode; }
		int  GetExitCode() { return ExitCode; }
		virtual void SetExitCode(int Code) { ExitCode=Code; }

		virtual BOOL IsFileModified() const { return FALSE; }

		virtual const wchar_t *GetTypeName() {return L"[FarModal]";}
		virtual int GetTypeAndName(string &strType, string &strName) {return(MODALTYPE_VIRTUAL);}
		virtual int GetType() { return MODALTYPE_VIRTUAL; }

		virtual void OnDestroy();  // вызывается перед уничтожением окна
		virtual void OnCreate() {}   // вызывается перед созданием окна
		virtual void OnChangeFocus(int focus); // вызывается при смене фокуса
		virtual void Refresh() {OnChangeFocus(1);}  // Просто перерисоваться :)

		virtual void InitKeyBar() {}
		void SetKeyBar(KeyBar *FrameKeyBar);
		void UpdateKeyBar();
		virtual void RedrawKeyBar() { UpdateKeyBar(); }

		int IsTitleBarVisible() const {return TitleBarVisible;}

		/* $ 12.05.2001 DJ */
		int IsTopFrame();
		virtual int GetMacroMode() { return MacroMode; }
		/* DJ $ */
		void Push(Frame* Modalized);
		Frame *GetTopModal() {return NextModal;}
//    bool Pop();
//    Frame *operator[](int Index);
//    int operator[](Frame *ModalFarame);
//    int ModalCount() {return ModalStackCount;}
		void DestroyAllModal();
		void SetDynamicallyBorn(int Born) {DynamicallyBorn=Born;}
		int GetDynamicallyBorn() {return DynamicallyBorn;}
		virtual int FastHide();
//    int IndexOf(Frame *aFrame);
		bool RemoveModal(Frame *aFrame);
		virtual void ResizeConsole();
		bool HasSaveScreen();
//    bool ifFullConsole();
		virtual string &GetTitle(string &Title,int SubLen=-1,int TruncSize=0) { return Title; }
		virtual bool ProcessEvents() {return true;}

		void SetFlags( DWORD flags ) { Flags.Set(flags); }
};
