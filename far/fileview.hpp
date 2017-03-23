#ifndef FILEVIEW_HPP_BC5E36F0_1E01_45AE_A121_A8D6EED6A14C
#define FILEVIEW_HPP_BC5E36F0_1E01_45AE_A121_A8D6EED6A14C
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

#include "window.hpp"
#include "viewer.hpp"

class FileViewer:public window,public ViewerContainer
{
	struct private_tag {};
public:
	static fileviewer_ptr create(const string& Name, bool EnableSwitch = false, bool DisableHistory = false,
		bool DisableEdit = false, long long ViewStartPos = -1, const wchar_t* PluginData = nullptr,
		NamesList* ViewNamesList = nullptr, bool ToSaveAs = false, uintptr_t aCodePage = CP_DEFAULT,
		const wchar_t* Title = nullptr, int DeleteOnClose = 0, window_ptr Update = nullptr);

	static fileviewer_ptr create(const string& Name, bool EnableSwitch, bool DisableHistory,
		const wchar_t *Title, int X1, int Y1, int X2, int Y2, uintptr_t aCodePage = CP_DEFAULT);

	FileViewer(private_tag, int DisableEdit, const wchar_t *Title);
	virtual ~FileViewer() override;

	virtual void InitKeyBar() override;
	virtual int ProcessKey(const Manager::Key& Key) override;
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual long long VMProcess(int OpCode,void *vParam=nullptr,long long iParam=0) override;
	virtual void ShowConsoleTitle() override;
	virtual void OnDestroy() override;
	virtual void OnChangeFocus(bool focus) override;
	virtual int GetTypeAndName(string &strType, string &strName) override;
	virtual int GetType() const override { return windowtype_viewer; }
	/* $ Введена для нужд CtrlAltShift OT */
	virtual bool CanFastHide() const override;
	virtual string GetTitle() const override;
	virtual Viewer* GetViewer(void) override;
	virtual Viewer* GetById(int ID) override;

	/* $ 14.06.2002 IS
	   Параметр DeleteFolder - удалить не только файл, но и каталог, его
	   содержащий (если каталог пуст). По умолчанию - TRUE (получаем
	   поведение SetTempViewName такое же, как и раньше)
	*/
	void SetTempViewName(const string& Name, bool DeleteFolder = true);
	void SetEnableF6(int AEnable) { DisableEdit = !AEnable; InitKeyBar(); }
	/* $ 17.08.2001 KM
		Добавлено для поиска по AltF7. При редактировании найденного файла из
		архива для клавиши F2 сделать вызов ShiftF2.
	*/
	void SetSaveToSaveAs(bool ToSaveAs) { SaveToSaveAs=ToSaveAs; InitKeyBar(); }
	int  ViewerControl(int Command, intptr_t Param1, void *Param2) const;
	bool IsFullScreen() const {return FullScreen;}
	long long GetViewFileSize() const;
	long long GetViewFilePos() const;
	void ShowStatus() const;
	int GetId() const { return m_View->ViewerID; }
	void OnReload(void);
	void ReadEvent(void);


private:
	virtual void Show() override;
	virtual void DisplayObject() override;

	void Init(const string& Name, bool EnableSwitch, int DisableHistory, long long ViewStartPos, const wchar_t *PluginData, NamesList *ViewNamesList, bool ToSaveAs, uintptr_t aCodePage, window_ptr Update = nullptr);

	std::unique_ptr<Viewer> m_View;
	int RedrawTitle;
	bool F3KeyOnly;
	bool m_bClosing;
	bool FullScreen;
	int DisableEdit;
	int DisableHistory;
	string m_Name;
	bool SaveToSaveAs;
	int delete_on_close;
	string str_title;
};

#endif // FILEVIEW_HPP_BC5E36F0_1E01_45AE_A121_A8D6EED6A14C
