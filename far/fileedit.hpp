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

#include "window.hpp"
#include "editor.hpp"
#include "plugin.hpp"
#include "namelist.hpp"
#include "codepage_selection.hpp"

// коды возврата Editor::SaveFile()
enum
{
	SAVEFILE_ERROR   = 0,         // пытались сохранять, не получилось
	SAVEFILE_SUCCESS = 1,         // либо успешно сохранили, либо сохранять было не надо
	SAVEFILE_CANCEL  = 2          // сохранение отменено, редактор не закрывать
};

enum FFILEEDIT_FLAGS
{
	FFILEEDIT_NEW                   = 0x00010000,  // Этот файл СОВЕРШЕННО! новый или его успели стереть! Нету такого и все тут.
	FFILEEDIT_REDRAWTITLE           = 0x00020000,  // Нужно редравить заголовок?
	FFILEEDIT_FULLSCREEN            = 0x00040000,  // Полноэкранный режим?
	FFILEEDIT_DISABLEHISTORY        = 0x00080000,  // Запретить запись в историю?
	FFILEEDIT_ENABLEF6              = 0x00100000,  // Переключаться во вьювер можно?
	FFILEEDIT_SAVETOSAVEAS          = 0x00200000,  // $ 17.08.2001 KM  Добавлено для поиска по AltF7.
	//   При редактировании найденного файла из архива для
	//   клавиши F2 сделать вызов ShiftF2.
	FFILEEDIT_SAVEWQUESTIONS        = 0x00400000,  // сохранить без вопросов
	FFILEEDIT_LOCKED                = 0x00800000,  // заблокировать?
	FFILEEDIT_OPENFAILED            = 0x01000000,  // файл открыть не удалось
	FFILEEDIT_DELETEONCLOSE         = 0x02000000,  // удалить в деструкторе файл вместе с каталогом (если тот пуст)
	FFILEEDIT_DELETEONLYFILEONCLOSE = 0x04000000,  // удалить в деструкторе только файл
	FFILEEDIT_DISABLESAVEPOS        = 0x08000000,  // не сохранять позицию для файла
	FFILEEDIT_CANNEWFILE            = 0x10000000,  // допускается новый файл?
	FFILEEDIT_SERVICEREGION         = 0x20000000,  // используется сервисная область
};

class FileEditor: public window,public EditorContainer
{
	struct private_tag {};

public:
	static fileeditor_ptr create(const string&  Name, uintptr_t codepage, DWORD InitFlags, int StartLine = -1, int StartChar = -1, const string* PluginData = nullptr, EDITOR_FLAGS OpenModeExstFile = EF_OPENMODE_QUERY);
	static fileeditor_ptr create(const string&  Name, uintptr_t codepage, DWORD InitFlags, int StartLine, int StartChar, const string* Title, int X1, int Y1, int X2, int Y2, int DeleteOnClose = 0, const window_ptr& Update = nullptr, EDITOR_FLAGS OpenModeExstFile = EF_OPENMODE_QUERY);

	explicit FileEditor(private_tag) {}
	virtual ~FileEditor() override;

	virtual bool IsFileModified() const override { return m_editor->IsFileModified(); }
	virtual int GetTypeAndName(string &strType, string &strName) override;
	virtual long long VMProcess(int OpCode, void* vParam = nullptr, long long iParam = 0) override;
	virtual void Show() override;
	virtual Editor* GetEditor(void) override;

	void ShowStatus() const;
	void SetLockEditor(bool LockMode) const;
	bool IsFullScreen() const { return m_Flags.Check(FFILEEDIT_FULLSCREEN); }
	void SetNamesList(NamesList& Names);
	void SetEnableF6(bool AEnableF6) { m_Flags.Change(FFILEEDIT_ENABLEF6, AEnableF6); InitKeyBar(); }
	// Добавлено для поиска по AltF7. При редактировании найденного файла из
	// архива для клавиши F2 сделать вызов ShiftF2.
	void SetSaveToSaveAs(bool ToSaveAs) { m_Flags.Change(FFILEEDIT_SAVETOSAVEAS, ToSaveAs); InitKeyBar(); }
	intptr_t EditorControl(int Command, intptr_t Param1, void *Param2);
	bool SetCodePage(uintptr_t codepage);  //BUGBUG
	int  SetCodePage(uintptr_t cp, bool redetect_default, bool ascii2def);
	bool IsFileChanged() const { return m_editor->IsFileChanged(); }
	void GetEditorOptions(Options::EditorOptions& EdOpt) const;
	void SetEditorOptions(const Options::EditorOptions& EdOpt) const;
	void SetPluginTitle(const string* PluginTitle);
	int GetId() const { return m_editor->EditorID; }
	FileEditor* GetById(int ID) { return GetId()==ID?this:nullptr; }
	void AutoDeleteColors() const { m_editor->AutoDeleteColors(); }

private:
	virtual void DisplayObject() override;
	virtual void InitKeyBar() override;
	virtual bool ProcessKey(const Manager::Key& Key) override;
	virtual bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual void ShowConsoleTitle() override;
	virtual void OnChangeFocus(bool focus) override;
	virtual void SetScreenPosition() override;
	virtual int GetType() const override { return windowtype_editor; }
	virtual void OnDestroy() override;
	virtual bool GetCanLoseFocus(bool DynamicMode = false) const override;
	virtual bool CanFastHide() const override; // для нужд CtrlAltShift
	virtual string GetTitle() const override;

	/* Ret:
		0 - не удалять ничего
		1 - удалять файл и каталог
		2 - удалять только файл
	*/
	void SetDeleteOnClose(int NewMode);
	bool ReProcessKey(const Manager::Key& Key, bool CalledFromControl = true);
	bool AskOverwrite(const string& FileName);
	void Init(const string& Name, uintptr_t codepage, const string* Title, int StartLine, int StartChar, const string* PluginData, int DeleteOnClose, const window_ptr& Update, EDITOR_FLAGS OpenModeExstFile);
	bool LoadFile(const string& Name, int &UserBreak);
	bool ReloadFile(uintptr_t codepage);
	//TextFormat, Codepage и AddSignature используются ТОЛЬКО, если bSaveAs = true!
	int SaveFile(const string& Name, int Ask, bool bSaveAs, int TextFormat = 0, uintptr_t Codepage = CP_UNICODE, bool AddSignature = false);
	void SetTitle(const string* Title);
	bool SetFileName(const string& NewFileName);
	int ProcessEditorInput(const INPUT_RECORD& Rec);
	DWORD EditorGetFileAttributes(const string& Name);
	void SetPluginData(const string* PluginData);
	const wchar_t *GetPluginData() const { return strPluginData.data(); }
	bool LoadFromCache(EditorPosCache &pc) const;
	void SaveToCache() const;
	void ReadEvent(void);
	int  ProcessQuitKey(int FirstSave, bool NeedQuestion = true, bool DeleteWindow = true);
	bool UpdateFileList() const;

	static uintptr_t GetDefaultCodePage();

	std::unique_ptr<Editor> m_editor;
	NamesList EditNamesList;
	bool F4KeyOnly{};
	string strFileName;
	string strFullFileName;
	string strStartDir;
	string strTitle;
	string strPluginTitle;
	string strPluginData;
	os::fs::find_data FileInfo;
	wchar_t AttrStr[4]{};            // 13.02.2001 IS - Сюда запомним буквы атрибутов, чтобы не вычислять их много раз
	DWORD m_FileAttributes{};          // 12.02.2001 IS - сюда запомним атрибуты файла при открытии, пригодятся где-нибудь...
	bool FileAttributesModified{};  // 04.11.2003 SKV - надо ли восстанавливать аттрибуты при save
	bool m_bClosing{};               // 28.04.2005 AY: true когда редактор закрываеться (т.е. в деструкторе)
	bool bEE_READ_Sent{};
	bool bLoaded{};
	bool m_bAddSignature{};
	bool BadConversion{};
	uintptr_t m_codepage{CP_DEFAULT}; //BUGBUG

	F8CP f8cps;
};

bool dlgOpenEditor(string &strFileName, uintptr_t &codepage);
bool dlgBadEditorCodepage(uintptr_t &codepage);

#endif // FILEEDIT_HPP_4BC43BC9_43BB_4F5B_ADAE_E2C370D65E69
