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

// Internal:
#include "window.hpp"
#include "bitflags.hpp"
#include "modal.hpp"

// Platform:

// Common:
#include "common/range.hpp"

// External:

//----------------------------------------------------------------------------

class History;

// Флаги текущего режима диалога
enum DIALOG_MODES
{
	DMODE_NONE                  = 0,
	DMODE_OBJECTS_INITED        = 0_bit,  // элементы инициализарованы?
	DMODE_OBJECTS_CREATED       = 1_bit,  // объекты (Edit,...) созданы?
	DMODE_WARNINGSTYLE          = 2_bit,  // Warning Dialog Style?
	DMODE_KEYDRAGGED            = 3_bit,  // диалог двигается клавиатурой?
	DMODE_ISCANMOVE             = 4_bit,  // можно ли двигать диалог?
	DMODE_MOUSEDRAGGED          = 5_bit,  // диалог двигается мышью?
	DMODE_SMALLDIALOG           = 6_bit,  // "короткий диалог"
	DMODE_DRAWING               = 12_bit, // диалог рисуется?
	DMODE_KEY                   = 13_bit, // Идет посылка клавиш?
	DMODE_SHOW                  = 14_bit, // Диалог виден?
	DMODE_INPUTEVENT            = 15_bit, // Нужно посылать DN_INPUT в обработчик?
	DMODE_RESIZED               = 16_bit, //
	DMODE_ENDLOOP               = 17_bit, // Конец цикла обработки диалога?
	DMODE_BEGINLOOP             = 18_bit, // Начало цикла обработки диалога?
	DMODE_ISMENU                = 19_bit, // диалог является экземпляром VMenu2
	DMODE_NODRAWSHADOW          = 20_bit, // не рисовать тень?
	DMODE_NODRAWPANEL           = 21_bit, // не рисовать подложку?
	DMODE_FULLSHADOW            = 22_bit,
	DMODE_NOPLUGINS             = 23_bit,
	DMODE_NEEDUPDATE            = 24_bit, // необходимо обновить весь диалог?
	DMODE_VISIBLE               = 25_bit, // отображать диалог на экране (DM_SHOWDIALOG)
	DMODE_KEEPCONSOLETITLE      = 28_bit, // не изменять заголовок консоли
	DMODE_CLICKOUTSIDE          = 29_bit, // было нажатие мыши вне диалога?
	DMODE_MSGINTERNAL           = 30_bit, // Внутренняя Message?
	DMODE_OLDSTYLE              = 31_bit, // Диалог в старом (до 1.70) стиле
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
	DialogItemEx(DialogItemEx&&) noexcept;
	DialogItemEx& operator=(DialogItemEx&&) noexcept;

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

bool IsKeyHighlighted(string_view Str, int Key, bool Translate, wchar_t CharKey = {});
void ItemsToItemsEx(span<const FarDialogItem> Items, span<DialogItemEx> ItemsEx, bool Short = false);


struct InitDialogItem
{
	enum FARDIALOGITEMTYPES Type;
	struct
	{
		point TopLeft, BottomRight;
	}
	Position;
	FARDIALOGITEMFLAGS Flags;
	string_view Data;
};

std::vector<DialogItemEx> MakeDialogItems(span<const InitDialogItem> Items);

template<size_t Size, size_t N>
std::vector<DialogItemEx> MakeDialogItems(InitDialogItem const (&Items)[N])
{
	static_assert(Size == N);

	return MakeDialogItems(Items);
}


class DlgEdit;
class Plugin;
class Dialog;

class Dialog: public Modal
{
protected:
	struct private_tag {};

public:
	using dialog_handler = std::function<intptr_t(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)>;

	template<class T, class O>
	static dialog_ptr create(T&& Src, intptr_t(O::*function)(Dialog*, intptr_t, intptr_t, void*), O* object, void* InitParam = nullptr)
	{
		auto Handler = (object && function)? [=](Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2) { return std::invoke(function, object, Dlg, Msg, Param1, Param2); } : dialog_handler();
		return std::make_shared<Dialog>(private_tag(), span(Src), Handler, InitParam);
	}

	template<class T>
	static dialog_ptr create(
		T&& Src, const dialog_handler& handler = nullptr, void* InitParam = nullptr)
	{
		return std::make_shared<Dialog>(private_tag(), span(Src), handler, InitParam);
	}

	template<class T>
	Dialog(private_tag, span<T> const Src, const dialog_handler& Handler, void* InitParam):
		DataDialog(InitParam),
		m_handler(Handler)
	{
		Construct(Src);
	}

	~Dialog() override;

	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	long long VMProcess(int OpCode, void *vParam=nullptr, long long iParam = 0) override;
	void Show() override;
	void Hide() override;
	void SetExitCode(int Code) override;
	void OnChangeFocus(bool focus) override;
	int GetTypeAndName(string &strType, string &strName) override;
	int GetType() const override { return windowtype_dialog; }
	bool CanFastHide() const override;
	void ResizeConsole() override;
	void SetPosition(rectangle Where) override;
	void FastShow() {ShowDialog();}
	void SetDeleting() override;
	void ShowConsoleTitle() override;
	bool ProcessEvents() override;

	static bool IsValid(Dialog* Handle);

	bool InitOK() const {return bInitOK;}
	void GetDialogObjectsData();
	void GetDialogObjectsExpandData();
	void SetDialogMode(DWORD Flags) { DialogMode.Set(Flags); }
	bool CheckDialogMode(DWORD Flags) const { return DialogMode.Check(Flags); }
	// метод для перемещения диалога
	void AdjustEditPos(int dx,int dy);
	int IsMoving() const {return DialogMode.Check(DMODE_KEYDRAGGED|DMODE_MOUSEDRAGGED);}
	void SetModeMoving(bool IsMoving) { DialogMode.Change(DMODE_ISCANMOVE,IsMoving);}
	int  GetModeMoving() const {return DialogMode.Check(DMODE_ISCANMOVE);}
	void SetDialogData(void* NewDataDialog);
	void* GetDialogData() const {return DataDialog;}
	void InitDialog();
	void Process();
	void SetPluginOwner(Plugin* NewPluginAddress) {PluginOwner = ((NewPluginAddress == INVALID_HANDLE_VALUE)? nullptr : NewPluginAddress);}
	Plugin* GetPluginOwner() const {return PluginOwner;}
	void SetHelp(string_view Topic);
	void ShowHelp() const;
	int Done() const { return DialogMode.Check(DMODE_ENDLOOP); }
	void ClearDone();
	intptr_t CloseDialog();
	// For MACRO
	const auto& GetAllItem() const { return Items; }
	size_t GetDlgFocusPos() const {return m_FocusPos;}
	int SetAutomation(WORD IDParent,WORD id, FARDIALOGITEMFLAGS UncheckedSet,FARDIALOGITEMFLAGS UncheckedSkip, FARDIALOGITEMFLAGS CheckedSet,FARDIALOGITEMFLAGS CheckedSkip,
		FARDIALOGITEMFLAGS Checked3Set=DIF_NONE,FARDIALOGITEMFLAGS Checked3Skip=DIF_NONE);

	intptr_t DlgProc(intptr_t Msg,intptr_t Param1,void* Param2);
	bool IsInited() const;
	void SetId(const UUID& Id);
	const UUID& GetId() const {return m_Id;}
	virtual intptr_t SendMessage(intptr_t Msg,intptr_t Param1,void* Param2);
	intptr_t DefProc(intptr_t Msg,intptr_t Param1,void* Param2);
	int GetDropDownOpened() const { return DropDownOpened; }
	bool IsRedrawEnabled() const { return m_DisableRedraw == 0; }

	intptr_t GetListItemSimpleUserData(size_t ListId, size_t ItemId) const;

	void SetListItemComplexUserData(size_t ListId, size_t ItemId, const std::any& Data);
	std::any* GetListItemComplexUserData(size_t ListId, size_t ItemId);

	template<class T>
	const T* GetListItemComplexUserDataPtr(size_t ListId, size_t ItemId)
	{
		return std::any_cast<T>(GetListItemComplexUserData(ListId, ItemId));
	}

	class suppress_redraw
	{
	public:
		NONCOPYABLE(suppress_redraw);

		explicit suppress_redraw(Dialog* Dlg);
		~suppress_redraw();

	private:
		Dialog* m_Dlg;
	};

protected:
	void InitDialogObjects(size_t ID = static_cast<size_t>(-1));

private:
	friend class History;
	friend class DlgEdit;

	void DisplayObject() override;
	string GetTitle() const override;

	void AddToList();
	void RemoveFromList();

	void Construct(span<DialogItemEx> SrcItems);
	void Construct(span<const FarDialogItem> SrcItems);
	void Init();
	void DeleteDialogObjects();
	int LenStrItem(size_t ID, string_view Str) const;
	int LenStrItem(size_t ID);
	int LenStrItem(const DialogItemEx& Item);
	void ShowDialog(size_t ID=static_cast<size_t>(-1));  //    ID=-1 - отрисовать весь диалог
	intptr_t CtlColorDlgItem(FarColor Color[4], size_t ItemPos, FARDIALOGITEMTYPES Type, bool Focus, bool Default,FARDIALOGITEMFLAGS Flags);
	/* $ 28.07.2000 SVS
		+ Изменяет фокус ввода между двумя элементами.
		    Вынесен отдельно для того, чтобы обработать DMSG_KILLFOCUS & DMSG_SETFOCUS
	*/
	void ChangeFocus2(size_t SetFocusPos);
	size_t ChangeFocus(size_t CurFocusPos, int Step, bool SkipGroup) const;
	bool SelectFromEditHistory(DialogItemEx const* CurItem, DlgEdit* EditLine, string_view HistoryName);
	int SelectFromComboBox(DialogItemEx *CurItem,DlgEdit*EditLine);
	bool AddToEditHistory(DialogItemEx const* CurItem, string_view AddStr) const;
	void ProcessLastHistory(DialogItemEx *CurItem, int MsgIndex);  // обработка DIF_USELASTHISTORY
	bool ProcessHighlighting(int Key, size_t FocusPos, bool Translate);
	int CheckHighlights(WORD CheckSymbol, int StartPos = 0);
	void SelectOnEntry(size_t Pos, bool Selected);
	void CheckDialogCoord();
	bool GetItemRect(size_t I,SMALL_RECT& Rect);
	bool SetItemRect(size_t ID, const SMALL_RECT& Rect);
	bool SetItemRect(DialogItemEx& Item, const SMALL_RECT& Rect);
	void SetDropDownOpened(int Status) { DropDownOpened=Status; }
	void ProcessCenterGroup();
	size_t ProcessRadioButton(size_t CurRB, bool UncheckAll);
	bool ProcessOpenComboBox(FARDIALOGITEMTYPES Type,DialogItemEx *CurItem,size_t CurFocusPos);
	bool ProcessMoveDialog(DWORD Key);
	bool Do_ProcessTab(bool Next);
	bool Do_ProcessNextCtrl(bool Up, bool IsRedraw=true);
	bool Do_ProcessFirstCtrl();
	bool Do_ProcessSpace();
	void SetComboBoxPos(DialogItemEx* Item=nullptr);
	rectangle CalcComboBoxPos(const DialogItemEx* CurItem, intptr_t ItemCount) const;
	void ProcessKey(int Key, size_t ItemPos);
	void ProcessDrag(const MOUSE_EVENT_RECORD *MouseEvent);

	static bool ItemHasDropDownArrow(const DialogItemEx *Item);


	bool bInitOK{};                   // диалог был успешно инициализирован
	class Plugin* PluginOwner{};      // Плагин, для формирования HelpTopic
	size_t m_FocusPos{};              // всегда известно какой элемент в фокусе
	size_t PrevFocusPos{};            // всегда известно какой элемент был в фокусе
	int m_DisableRedraw{};            // Разрешена перерисовка диалога? ( 0 - разрешена)
	BitFlags DialogMode;              // Флаги текущего режима диалога
	void* DataDialog{};               // Данные, специфические для конкретного экземпляра диалога (первоначально здесь параметр, переданный в конструктор)
	std::vector<DialogItemEx> Items;  // массив элементов диалога
	DialogItemEx* SavedItems{};       // пользовательский массив элементов диалога

	dialog_handler m_handler;

	// переменные для перемещения диалога
	struct
	{
		rectangle OldRect;
		int MsX, MsY;
	}
	m_Drag{};
	string HelpTopic;
	int DropDownOpened{}; // Содержит статус комбобокса и хистори: TRUE - открыт, FALSE - закрыт.
	int RealWidth{};
	int RealHeight{};
	UUID m_Id{};
	bool IdExist{};
	MOUSE_EVENT_RECORD PrevMouseRecord{};
	string m_ConsoleTitle;
};

// BUGBUG
extern std::chrono::steady_clock::duration WaitUserTime;

#endif // DIALOG_HPP_7A9BE12B_EE5C_441F_84C9_64E9A63ABEFE
