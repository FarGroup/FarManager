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

// Internal:
#include "window.hpp"
#include "viewer.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

class FileViewer final: public window,public ViewerContainer
{
	struct private_tag { explicit private_tag() = default; };

public:
	static fileviewer_ptr create(
		string_view Name,
		bool EnableSwitch = false,
		bool DisableHistory = false,
		bool DisableEdit = false,
		long long ViewStartPos = -1,
		string_view PluginData = {},
		NamesList* ViewNamesList = nullptr,
		bool ToSaveAs = false,
		uintptr_t aCodePage = CP_DEFAULT,
		string_view Title = {},
		int DeleteOnClose = 0,
		window_ptr Update = nullptr);

	static fileviewer_ptr create(
		string_view Name,
		bool EnableSwitch,
		bool DisableHistory,
		string_view Title,
		rectangle Position,
		uintptr_t aCodePage = CP_DEFAULT);

	FileViewer(private_tag, bool DisableEdit, string_view Title);
	~FileViewer() override;

	void InitKeyBar() override;
	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	long long VMProcess(int OpCode,void *vParam=nullptr,long long iParam=0) override;
	void ShowConsoleTitle() override;
	void OnDestroy() override;
	void OnChangeFocus(bool focus) override;
	int GetTypeAndName(string &strType, string &strName) override;
	int GetType() const override { return windowtype_viewer; }
	bool CanFastHide() const override;
	string GetTitle() const override;
	bool IsKeyBarVisible() const override;
	bool IsTitleBarVisible() const override;
	Viewer* GetViewer() override;
	Viewer* GetById(int ID) override;

	/* $ 14.06.2002 IS
	   Параметр DeleteFolder - удалить не только файл, но и каталог, его
	   содержащий (если каталог пуст). По умолчанию - TRUE (получаем
	   поведение SetTempViewName такое же, как и раньше)
	*/
	void SetTempViewName(string_view Name, bool DeleteFolder = true);
	void SetEnableF6(int AEnable) { m_DisableEdit = !AEnable; InitKeyBar(); }
	/* $ 17.08.2001 KM
		Добавлено для поиска по AltF7. При редактировании найденного файла из
		архива для клавиши F2 сделать вызов ShiftF2.
	*/
	void SetSaveToSaveAs(bool ToSaveAs) { m_SaveToSaveAs=ToSaveAs; InitKeyBar(); }
	int  ViewerControl(int Command, intptr_t Param1, void *Param2) const;
	bool IsFullScreen() const {return m_FullScreen;}
	long long GetViewFileSize() const;
	long long GetViewFilePos() const;
	void ShowStatus() const;
	int GetId() const { return m_View->ViewerID; }
	void OnReload();
	void ReadEvent();

private:
	void Show() override;
	void DisplayObject() override;

	void Init(
		string_view Name,
		bool EnableSwitch,
		bool DisableHistory,
		long long ViewStartPos,
		string_view PluginData,
		NamesList *ViewNamesList,
		bool ToSaveAs,
		uintptr_t aCodePage,
		window_ptr Update = nullptr);

	std::unique_ptr<Viewer> m_View;
	bool m_RedrawTitle{};
	bool m_bClosing{};
	bool m_FullScreen{true};
	bool m_DisableEdit;
	bool m_DisableHistory{};
	string m_Name;
	bool m_SaveToSaveAs{};
	int m_DeleteOnClose{};
	string m_StrTitle;

	class f3_key_timer;
	std::unique_ptr<f3_key_timer> m_F3Timer;

	class reload_timer;
	std::unique_ptr<reload_timer> m_ReloadTimer;

};

#endif // FILEVIEW_HPP_BC5E36F0_1E01_45AE_A121_A8D6EED6A14C
