#ifndef DIALOG_HPP_7A9BE12B_EE5C_441F_84C9_64E9A63ABEFE
#define DIALOG_HPP_7A9BE12B_EE5C_441F_84C9_64E9A63ABEFE
#pragma once

/*
dialog.hpp

Класс диалога Dialog.

Предназначен для отображения модальных диалогов.
Является производным от класса Modal.
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
#include "vmenu.hpp"
#include "bitflags.hpp"
#include "synchro.hpp"

class History;

// Флаги текущего режима диалога
enum DIALOG_MODES
{
	DMODE_OBJECTS_INITED        =0x00000001, // элементы инициализарованы?
	DMODE_OBJECTS_CREATED       =0x00000002, // объекты (Edit,...) созданы?
	DMODE_WARNINGSTYLE          =0x00000004, // Warning Dialog Style?
	DMODE_DRAGGED               =0x00000008, // диалог двигается?
	DMODE_ISCANMOVE             =0x00000010, // можно ли двигать диалог?
	DMODE_ALTDRAGGED            =0x00000020, // диалог двигается по Alt-Стрелка?
	DMODE_SMALLDIALOG           =0x00000040, // "короткий диалог"
	DMODE_DRAWING               =0x00001000, // диалог рисуется?
	DMODE_KEY                   =0x00002000, // Идет посылка клавиш?
	DMODE_SHOW                  =0x00004000, // Диалог виден?
	DMODE_INPUTEVENT            =0x00008000, // Нужно посылать DN_INPUT в обработчик?
	DMODE_RESIZED               =0x00010000, //
	DMODE_ENDLOOP               =0x00020000, // Конец цикла обработки диалога?
	DMODE_BEGINLOOP             =0x00040000, // Начало цикла обработки диалога?
	DMODE_ISMENU                =0x00080000, // диалог является экземпляром VMenu2
	DMODE_NODRAWSHADOW          =0x00100000, // не рисовать тень?
	DMODE_NODRAWPANEL           =0x00200000, // не рисовать подложку?
	DMODE_FULLSHADOW            =0x00400000,
	DMODE_NOPLUGINS             =0x00800000,
	DMODE_NEEDUPDATE            =0x01000000, // необходимо обновить весь диалог?
	DMODE_VISIBLE               =0x02000000, // отображать диалог на экране (DM_SHOWDIALOG)
	DMODE_KEEPCONSOLETITLE      =0x10000000, // не изменять заголовок консоли
	DMODE_CLICKOUTSIDE          =0x20000000, // было нажатие мыши вне диалога?
	DMODE_MSGINTERNAL           =0x40000000, // Внутренняя Message?
	DMODE_OLDSTYLE              =0x80000000, // Диалог в старом (до 1.70) стиле
};

/*
Описывает один элемент диалога - внутренне представление.
Для плагинов это FarDialogItem
*/
struct DialogItemEx: public FarDialogItem
{
	// Структура, описывающая автоматизацию для DIF_AUTOMATION
	struct DialogItemAutomation;

	int ListPos;
	string strHistory;
	string strMask;
	string strData;
	BitFlags IFlags;
	std::vector<DialogItemAutomation> Auto;
	void *ObjPtr;
	vmenu_ptr ListPtr;
	struct DlgUserControl *UCData;

	DialogItemEx();
	~DialogItemEx();
	DialogItemEx(const DialogItemEx&);
	DialogItemEx& operator=(const DialogItemEx&);
	DialogItemEx(DialogItemEx&&);
	DialogItemEx& operator=(DialogItemEx&&);

	void Indent(int Delta)
	{
		X1 += Delta;
		X2 += Delta;
	}

	bool AddAutomation(DialogItemEx* DlgItem,
		FARDIALOGITEMFLAGS UncheckedSet, FARDIALOGITEMFLAGS UncheckedSkip,
		FARDIALOGITEMFLAGS CheckedSet, FARDIALOGITEMFLAGS CheckedSkip,
		FARDIALOGITEMFLAGS Checked3Set, FARDIALOGITEMFLAGS Checked3Skip);
};

bool IsKeyHighlighted(const string& Str, int Key, int Translate, int AmpPos = -1);
void ItemsToItemsEx(const range<const FarDialogItem*>& Items, const range<DialogItemEx*>& ItemsEx, bool Short = false);

template<size_t N>
auto MakeDialogItemsEx(const FarDialogItem (&InitData)[N])
{
	std::vector<DialogItemEx> Items(N);
	ItemsToItemsEx(make_range(InitData), make_range(Items.data(), Items.size()));
	return Items;
}

class DlgEdit;
class Plugin;
class Dialog;

class Dialog: public Modal
{
protected:
	struct private_tag {};

public:
	typedef std::function<intptr_t(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)> dialog_handler;

	template<class T, class O>
	static dialog_ptr create(T&& Src, intptr_t(O::*function)(Dialog*, intptr_t, intptr_t, void*), O* object, void* InitParam = nullptr)
	{
		auto Handler = (object && function)? [=](Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2) { return (object->*function)(Dlg, Msg, Param1, Param2); } : dialog_handler();
		return std::make_shared<Dialog>(private_tag(), Src, Handler, InitParam);
	}

	template<class T>
	static dialog_ptr create(T&& Src, const dialog_handler& handler = nullptr, void* InitParam = nullptr)
	{
		return std::make_shared<Dialog>(private_tag(), Src, handler, InitParam);
	}

	template<class T>
	Dialog(private_tag, T&& Src, const dialog_handler& Handler, void* InitParam):
		bInitOK(),
		DataDialog(InitParam),
		m_handler(Handler)
	{
		Construct(make_range(Src.data(), Src.size()));
	}

	virtual ~Dialog();

	virtual int ProcessKey(const Manager::Key& Key) override;
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0) override;
	virtual void Show() override;
	virtual void Hide() override;
	virtual void SetExitCode(int Code) override;
	virtual int GetTypeAndName(string &strType, string &strName) override;
	virtual int GetType() const override { return windowtype_dialog; }
	virtual bool CanFastHide() const override;
	virtual void ResizeConsole() override;
	virtual void SetPosition(int X1,int Y1,int X2,int Y2) override;
	virtual void FastShow() {ShowDialog();}
	virtual void SetDeleting(void) override;
	virtual void ShowConsoleTitle() override;

	bool InitOK() const {return bInitOK;}
	void GetDialogObjectsData();
	void GetDialogObjectsExpandData();
	void SetDialogMode(DWORD Flags) { DialogMode.Set(Flags); }
	bool CheckDialogMode(DWORD Flags) const { return DialogMode.Check(Flags); }
	// метод для перемещения диалога
	void AdjustEditPos(int dx,int dy);
	int IsMoving() const {return DialogMode.Check(DMODE_DRAGGED);}
	void SetModeMoving(bool IsMoving) { DialogMode.Change(DMODE_ISCANMOVE,IsMoving);}
	int  GetModeMoving() const {return DialogMode.Check(DMODE_ISCANMOVE);}
	void SetDialogData(void* NewDataDialog);
	void* GetDialogData() const {return DataDialog;}
	void InitDialog();
	void Process();
	void SetPluginOwner(Plugin* NewPluginAddress) {PluginOwner = ((NewPluginAddress == INVALID_HANDLE_VALUE)? nullptr : NewPluginAddress);}
	Plugin* GetPluginOwner() const {return PluginOwner;}
	void SetHelp(const string& Topic);
	void ShowHelp();
	int Done() const { return DialogMode.Check(DMODE_ENDLOOP); }
	void ClearDone();
	intptr_t CloseDialog();
	// For MACRO
	const std::vector<DialogItemEx>& GetAllItem() const { return Items; }
	size_t GetDlgFocusPos() const {return m_FocusPos;}
	int SetAutomation(WORD IDParent,WORD id, FARDIALOGITEMFLAGS UncheckedSet,FARDIALOGITEMFLAGS UncheckedSkip, FARDIALOGITEMFLAGS CheckedSet,FARDIALOGITEMFLAGS CheckedSkip,
		FARDIALOGITEMFLAGS Checked3Set=DIF_NONE,FARDIALOGITEMFLAGS Checked3Skip=DIF_NONE);

	intptr_t DlgProc(intptr_t Msg,intptr_t Param1,void* Param2);
	BOOL IsInited() const;
	virtual bool ProcessEvents() override;
	void SetId(const GUID& Id);
	const GUID& GetId() const {return m_Id;}
	intptr_t SendMessage(intptr_t Msg,intptr_t Param1,void* Param2);
	intptr_t DefProc(intptr_t Msg,intptr_t Param1,void* Param2);
	static bool IsValid(Dialog* Handle);
	int GetDropDownOpened() const { return DropDownOpened; }

	template<class T>
	void SetListItemData(size_t ListId, size_t ItemId, const T& Data)
	{
		Items[ListId].ListPtr->SetUserData(Data, static_cast<int>(ItemId));
	}

	template<class T>
	T* GetListItemDataPtr(size_t ListId, size_t ItemId)
	{
		return Items[ListId].ListPtr->GetUserDataPtr<T>(static_cast<int>(ItemId));
	}

protected:
	size_t InitDialogObjects(size_t ID = (size_t)-1);

private:
	friend class History;
	friend class DlgEdit;

	virtual void DisplayObject() override;
	virtual string GetTitle() const override;
	typedef std::unordered_set<Dialog*> dialogs_set;
	static dialogs_set& DialogsList();
	void AddToList();
	void RemoveFromList();

	void Construct(const range<DialogItemEx*>& SrcItems);
	void Construct(const range<const FarDialogItem*>& SrcItems);
	void Init();
	void DeleteDialogObjects();
	int LenStrItem(size_t ID, const string& Str) const;
	int LenStrItem(size_t ID);
	int LenStrItem(const DialogItemEx& Item);
	void ShowDialog(size_t ID=(size_t)-1);  //    ID=-1 - отрисовать весь диалог
	intptr_t CtlColorDlgItem(FarColor Color[4], size_t ItemPos, FARDIALOGITEMTYPES Type, bool Focus, bool Default,FARDIALOGITEMFLAGS Flags);
	/* $ 28.07.2000 SVS
		+ Изменяет фокус ввода между двумя элементами.
		    Вынесен отдельно для того, чтобы обработать DMSG_KILLFOCUS & DMSG_SETFOCUS
	*/
	void ChangeFocus2(size_t SetFocusPos);
	size_t ChangeFocus(size_t FocusPos,int Step,int SkipGroup) const;
	BOOL SelectFromEditHistory(const DialogItemEx *CurItem, DlgEdit *EditLine, const string& HistoryName);
	int SelectFromComboBox(DialogItemEx *CurItem,DlgEdit*EditLine);
	int AddToEditHistory(const DialogItemEx* CurItem, const string& AddStr) const;
	void ProcessLastHistory(DialogItemEx *CurItem, int MsgIndex);  // обработка DIF_USELASTHISTORY
	int ProcessHighlighting(int Key,size_t FocusPos,int Translate);
	int CheckHighlights(WORD Chr,int StartPos=0);
	void SelectOnEntry(size_t Pos,BOOL Selected);
	void CheckDialogCoord();
	BOOL GetItemRect(size_t I,SMALL_RECT& Rect);
	bool SetItemRect(size_t ID, const SMALL_RECT& Rect);
	bool SetItemRect(DialogItemEx& item, const SMALL_RECT& Rect);
	void SetDropDownOpened(int Status) { DropDownOpened=Status; }
	void ProcessCenterGroup();
	size_t ProcessRadioButton(size_t);
	int ProcessOpenComboBox(FARDIALOGITEMTYPES Type,DialogItemEx *CurItem,size_t CurFocusPos);
	int ProcessMoveDialog(DWORD Key);
	int Do_ProcessTab(bool Next);
	int Do_ProcessNextCtrl(bool Up, bool IsRedraw=true);
	int Do_ProcessFirstCtrl();
	int Do_ProcessSpace();
	void SetComboBoxPos(DialogItemEx* Item=nullptr);
	void CalcComboBoxPos(const DialogItemEx* CurItem, intptr_t ItemCount, int &X1, int &Y1, int &X2, int &Y2) const;
	void ProcessKey(int Key, size_t ItemPos);

	static bool ItemHasDropDownArrow(const DialogItemEx *Item);


	bool bInitOK;               // диалог был успешно инициализирован
	class Plugin* PluginOwner;       // Плагин, для формирования HelpTopic
	size_t m_FocusPos;               // всегда известно какой элемент в фокусе
	size_t PrevFocusPos;           // всегда известно какой элемент был в фокусе
	int IsEnableRedraw;         // Разрешена перерисовка диалога? ( 0 - разрешена)
	BitFlags DialogMode;        // Флаги текущего режима диалога
	void* DataDialog;        // Данные, специфические для конкретного экземпляра диалога (первоначально здесь параметр, переданный в конструктор)
	std::vector<DialogItemEx> Items; // массив элементов диалога
	DialogItemEx* SavedItems; // пользовательский массив элементов диалога

	dialog_handler m_handler;

	// переменные для перемещения диалога
	int OldX1,OldX2,OldY1,OldY2;
	string HelpTopic;
	int DropDownOpened;// Содержит статус комбобокса и хистори: TRUE - открыт, FALSE - закрыт.
	mutable CriticalSection CS;
	int RealWidth, RealHeight;
	GUID m_Id;
	bool IdExist;
	MOUSE_EVENT_RECORD PrevMouseRecord;
	string m_ConsoleTitle;
};

#endif // DIALOG_HPP_7A9BE12B_EE5C_441F_84C9_64E9A63ABEFE
