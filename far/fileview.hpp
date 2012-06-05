#pragma once

/*
fileview.hpp

Просмотр файла - надстройка над viewer.cpp
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

#include "frame.hpp"
#include "viewer.hpp"
#include "keybar.hpp"

class FileViewer:public Frame
{
	private:
		virtual void Show();
		virtual void DisplayObject();
		Viewer View;
		int RedrawTitle;
		KeyBar ViewKeyBar;
		bool F3KeyOnly;
		bool FullScreen;
		int DisableEdit;
		int DisableHistory;

		string strName;

		typedef class Frame inherited;
		/* $ 17.08.2001 KM
		  Добавлено для поиска по AltF7. При редактировании найденного файла из
		  архива для клавиши F2 сделать вызов ShiftF2.
		*/
		int SaveToSaveAs;

		int delete_on_close;
		string    str_title;

	public:
		FileViewer(
			const wchar_t *Name,int EnableSwitch=FALSE,int DisableHistory=FALSE,
			int DisableEdit=FALSE,__int64 ViewStartPos=-1,const wchar_t *PluginData=nullptr,
			NamesList *ViewNamesList=nullptr,int ToSaveAs=FALSE,UINT aCodePage=CP_DEFAULT,
			const wchar_t *Title=nullptr, int DeleteOnClose=0);
		FileViewer(const wchar_t *Name,int EnableSwitch,int DisableHistory,
			const wchar_t *Title,int X1,int Y1,int X2,int Y2,UINT aCodePage=CP_DEFAULT);
		virtual ~FileViewer();

	public:
		void Init(const wchar_t *Name,int EnableSwitch,int DisableHistory,
			__int64 ViewStartPos,const wchar_t *PluginData,NamesList *ViewNamesList,int ToSaveAs);
		virtual void InitKeyBar();
		virtual int ProcessKey(int Key);
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
		virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0);
		virtual void ShowConsoleTitle();
		/* $ 14.06.2002 IS
		   Параметр DeleteFolder - удалить не только файл, но и каталог, его
		   содержащий (если каталог пуст). По умолчанию - TRUE (получаем
		   поведение SetTempViewName такое же, как и раньше)
		*/
		void SetTempViewName(const wchar_t *Name,BOOL DeleteFolder=TRUE);
		virtual void OnDestroy();
		virtual void OnChangeFocus(int focus);

		virtual int GetTypeAndName(string &strType, string &strName);
		virtual const wchar_t *GetTypeName() {return L"[FileView]";}; ///
		virtual int GetType() { return MODALTYPE_VIEWER; }

		void SetEnableF6(int AEnable) { DisableEdit = !AEnable; InitKeyBar(); }
		/* $ Введена для нужд CtrlAltShift OT */
		virtual int FastHide();

		/* $ 17.08.2001 KM
		  Добавлено для поиска по AltF7. При редактировании найденного файла из
		  архива для клавиши F2 сделать вызов ShiftF2.
		*/
		void SetSaveToSaveAs(int ToSaveAs) { SaveToSaveAs=ToSaveAs; InitKeyBar(); }
		int  ViewerControl(int Command,void *Param);
		bool IsFullScreen() {return FullScreen;}
		virtual string &GetTitle(string &Title,int SubLen=-1,int TruncSize=0);
		__int64 GetViewFileSize() const;
		__int64 GetViewFilePos() const;
		void ShowStatus();
		int GetId(void) const { return View.ViewerID; };
};
