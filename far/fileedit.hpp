#ifndef FILEEDIT_HPP_4BC43BC9_43BB_4F5B_ADAE_E2C370D65E69
#define FILEEDIT_HPP_4BC43BC9_43BB_4F5B_ADAE_E2C370D65E69
#pragma once

/*
fileedit.hpp

Редактирование файла - надстройка над editor.cpp
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
#include "editor.hpp"
#include "plugin.hpp"
#include "namelist.hpp"
#include "codepage_selection.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:

// External:

//----------------------------------------------------------------------------

struct error_state_ex;

// коды возврата Editor::SaveFile()
enum
{
	SAVEFILE_ERROR   = 0,         // пытались сохранять, не получилось
	SAVEFILE_SUCCESS = 1,         // либо успешно сохранили, либо сохранять было не надо
	SAVEFILE_CANCEL  = 2          // сохранение отменено, редактор не закрывать
};

enum FFILEEDIT_FLAGS
{
	FFILEEDIT_NEW                   = 16_bit,  // Этот файл СОВЕРШЕННО! новый или его успели стереть! Нету такого и все тут.
	FFILEEDIT_REDRAWTITLE           = 17_bit,  // Нужно редравить заголовок?
	FFILEEDIT_FULLSCREEN            = 18_bit,  // Полноэкранный режим?
	FFILEEDIT_DISABLEHISTORY        = 19_bit,  // Запретить запись в историю?
	FFILEEDIT_ENABLEF6              = 20_bit,  // Переключаться во вьювер можно?
	FFILEEDIT_SAVETOSAVEAS          = 21_bit,  // $ 17.08.2001 KM  Добавлено для поиска по AltF7.
	FFILEEDIT_WAS_SAVED             = 22_bit,  // The file was saved by the user at least once. Sometimes we need to know, e.g. if it's from a plugin panel.
	//   При редактировании найденного файла из архива для
	//   клавиши F2 сделать вызов ShiftF2.
	FFILEEDIT_LOCKED                = 23_bit,  // заблокировать?
	FFILEEDIT_OPENFAILED            = 24_bit,  // файл открыть не удалось
	FFILEEDIT_DELETEONCLOSE         = 25_bit,  // удалить в деструкторе файл вместе с каталогом (если тот пуст)
	FFILEEDIT_DELETEONLYFILEONCLOSE = 26_bit,  // удалить в деструкторе только файл
	FFILEEDIT_DISABLESAVEPOS        = 27_bit,  // не сохранять позицию для файла
	FFILEEDIT_CANNEWFILE            = 28_bit,  // допускается новый файл?
	FFILEEDIT_SERVICEREGION         = 29_bit,  // используется сервисная область
};

class FileEditor final: public window,public EditorContainer
{
	struct private_tag { explicit private_tag() = default; };

public:
	static fileeditor_ptr create(string_view Name, uintptr_t codepage, DWORD InitFlags, int StartLine = -1, int StartChar = -1, const string* PluginData = nullptr, EDITOR_FLAGS OpenModeExstFile = EF_OPENMODE_QUERY);
	static fileeditor_ptr create(string_view Name, uintptr_t codepage, DWORD InitFlags, int StartLine, int StartChar, const string* Title, rectangle Position, int DeleteOnClose = 0, const window_ptr& Update = nullptr, EDITOR_FLAGS OpenModeExstFile = EF_OPENMODE_QUERY);

	explicit FileEditor(private_tag);
	~FileEditor() override;

	bool IsFileModified() const override { return m_editor->IsModified(); }
	int GetTypeAndName(string &strType, string &strName) override;
	long long VMProcess(int OpCode, void* vParam = nullptr, long long iParam = 0) override;
	void Show() override;
	Editor* GetEditor() override;

	void ShowStatus() const;
	void SetLockEditor(bool LockMode) const;
	bool IsFullScreen() const { return m_Flags.Check(FFILEEDIT_FULLSCREEN); }
	void SetNamesList(NamesList& Names);
	void SetEnableF6(bool AEnableF6) { m_Flags.Change(FFILEEDIT_ENABLEF6, AEnableF6); InitKeyBar(); }
	// Добавлено для поиска по AltF7. При редактировании найденного файла из
	// архива для клавиши F2 сделать вызов ShiftF2.
	void SetSaveToSaveAs(bool ToSaveAs) { m_Flags.Change(FFILEEDIT_SAVETOSAVEAS, ToSaveAs); InitKeyBar(); }
	intptr_t EditorControl(int Command, intptr_t Param1, void *Param2);
	uintptr_t GetCodePage() const;
	bool SetCodePage(uintptr_t codepage);  //BUGBUG
	bool SetCodePageEx(uintptr_t cp);
	bool WasFileSaved() const { return m_Flags.Check(FFILEEDIT_WAS_SAVED); }
	void GetEditorOptions(Options::EditorOptions& EdOpt) const;
	void SetEditorOptions(const Options::EditorOptions& EdOpt) const;
	void SetPluginTitle(const string* PluginTitle);
	int GetId() const { return m_editor->EditorID; }
	FileEditor* GetById(int ID) { return GetId()==ID?this:nullptr; }
	void AutoDeleteColors() const { m_editor->AutoDeleteColors(); }

private:
	void DisplayObject() override;
	void InitKeyBar() override;
	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	void ShowConsoleTitle() override;
	void OnChangeFocus(bool focus) override;
	void SetScreenPosition() override;
	int GetType() const override { return windowtype_editor; }
	void OnDestroy() override;
	bool GetCanLoseFocus(bool DynamicMode = false) const override;
	bool CanFastHide() const override; // для нужд CtrlAltShift
	string GetTitle() const override;
	bool IsKeyBarVisible() const override;
	bool IsTitleBarVisible() const override;

	/* Ret:
		0 - не удалять ничего
		1 - удалять файл и каталог
		2 - удалять только файл
	*/
	void SetDeleteOnClose(int NewMode);
	bool ReProcessKey(const Manager::Key& Key, bool CalledFromControl = true);
	bool AskOverwrite(string_view FileName);
	void Init(string_view Name, uintptr_t codepage, const string* Title, int StartLine, int StartChar, const string* PluginData, int DeleteOnClose, const window_ptr& Update, EDITOR_FLAGS OpenModeExstFile);
	bool LoadFile(string_view Name, int &UserBreak, error_state_ex& ErrorState);
	bool ReloadFile(uintptr_t codepage);
	//TextFormat, Codepage и AddSignature используются ТОЛЬКО, если bSaveAs = true!
	int SaveFile(string_view Name, bool bSaveAs, error_state_ex& ErrorState, eol Eol = eol::none, uintptr_t Codepage = CP_UTF16LE, bool AddSignature = false);
	bool SaveAction(bool SaveAsIntention);
	void SetTitle(const string* Title);
	void SetFileName(string_view NewFileName);
	int ProcessEditorInput(const INPUT_RECORD& Rec);
	os::fs::attributes EditorGetFileAttributes(string_view Name);
	void SetPluginData(const string* PluginData);
	const string& GetPluginData() const { return strPluginData; }
	bool LoadFromCache(EditorPosCache &pc) const;
	void SaveToCache() const;
	void ReadEvent();
	bool ProcessQuitKey(bool NeedSave, bool ConfirmSave, bool DeleteWindow);
	bool UpdateFileList() const;

	static uintptr_t GetDefaultCodePage();

	std::unique_ptr<Editor> m_editor;
	NamesList EditNamesList;
	string strFileName;
	string strFullFileName;
	string strStartDir;
	string strTitle;
	string strPluginTitle;
	string strPluginData;
	os::fs::find_data FileInfo;
	wchar_t AttrStr[4]{};            // 13.02.2001 IS - Сюда запомним буквы атрибутов, чтобы не вычислять их много раз
	os::fs::attributes m_FileAttributes{};          // 12.02.2001 IS - сюда запомним атрибуты файла при открытии, пригодятся где-нибудь...
	bool m_bClosing{};               // 28.04.2005 AY: true когда редактор закрываеться (т.е. в деструкторе)
	bool bEE_READ_Sent{};
	bool bLoaded{};
	bool m_bAddSignature{};
	bool BadConversion{};
	uintptr_t m_codepage{CP_DEFAULT};
	eol m_SaveEol{ eol::none };

	F8CP f8cps;

	class f4_key_timer;
	std::unique_ptr<f4_key_timer> m_F4Timer;
};

bool dlgOpenEditor(string &strFileName, uintptr_t &codepage);

#endif // FILEEDIT_HPP_4BC43BC9_43BB_4F5B_ADAE_E2C370D65E69
