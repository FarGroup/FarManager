/*
dlgedit.cpp

Одиночная строка редактирования для диалога (как наследник класса Edit)
Мультиредактор
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "dlgedit.hpp"

// Internal:
#include "dialog.hpp"
#include "history.hpp"
#include "editcontrol.hpp"
#include "config.hpp"
#include "global.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

DlgEdit::DlgEdit(window_ptr Owner,size_t Index,DLGEDITTYPE Type):
	SimpleScreenObject(std::move(Owner)),
	m_Index(Index),
	Type(Type)
{
	Init();
}

void DlgEdit::Init()
{
	switch (Type)
	{
		case DLGEDIT_MULTILINE:
#if defined(PROJECT_DI_MEMOEDIT)
			multiEdit=new Editor(pOwner,true); // ??? (pOwner) ?
#endif
			break;
		case DLGEDIT_SINGLELINE:
		{
			EditControl::Callback callback{ true, EditChange, this };

			VMenu* iList = nullptr;
			DWORD iFlags=0;
			const auto& CurItem = GetDialog()->Items[m_Index];
			if(Global->Opt->Dialogs.AutoComplete && CurItem.Flags&(DIF_HISTORY|DIF_EDITPATH|DIF_EDITPATHEXEC) && !(CurItem.Flags&DIF_DROPDOWNLIST) && !(CurItem.Flags&DIF_NOAUTOCOMPLETE))
			{
				iFlags=EditControl::EC_ENABLEAUTOCOMPLETE;
			}
			if(CurItem.Flags&DIF_HISTORY && !CurItem.strHistory.empty())
			{
				SetHistory(CurItem.strHistory);
			}
			if(CurItem.Type == DI_COMBOBOX)
			{
				iList = CurItem.ListPtr.get();
			}
			if(CurItem.Flags&DIF_HISTORY)
			{
				iFlags|=EditControl::EC_COMPLETE_HISTORY;
			}
			if(CurItem.Flags&DIF_EDITPATH)
			{
				iFlags|=EditControl::EC_COMPLETE_FILESYSTEM;
			}
			if(CurItem.Flags&DIF_EDITPATHEXEC)
			{
				iFlags|=EditControl::EC_COMPLETE_PATH;
			}
			iFlags|=EditControl::EC_COMPLETE_ENVIRONMENT;

			lineEdit = std::make_unique<EditControl>(GetOwner(), GetOwner().get(), nullptr, &callback, iHistory.get(), iList, iFlags);
		}
		break;
	}
}

DlgEdit::~DlgEdit()
{
#if defined(PROJECT_DI_MEMOEDIT)
	delete multiEdit;
#endif
}


void DlgEdit::SetHistory(string_view const Name)
{
	iHistory = std::make_unique<History>(HISTORYTYPE_DIALOG, Name, Global->Opt->Dialogs.EditHistory, false);

	if (lineEdit)
	{
		lineEdit.get()->pHistory = iHistory.get();
	}
}

bool DlgEdit::ProcessKey(const Manager::Key& Key)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->ProcessKey(Key);
	else
#endif
		return lineEdit->ProcessKey(Key);
}

bool DlgEdit::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->ProcessMouse(MouseEvent);
	else
#endif
		return lineEdit->ProcessMouse(MouseEvent);
}

void DlgEdit::DisplayObject()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->DisplayObject();
	else
#endif
		lineEdit->DisplayObject();
}

void DlgEdit::SetPosition(rectangle Where)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetPosition(Where);
	else
#endif
		lineEdit->SetPosition(Where);
}

void DlgEdit::Show()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->Show();
	else
#endif
		lineEdit->Show();
}

rectangle DlgEdit::GetPosition() const
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->GetPosition();
	else
#endif
		return lineEdit->GetPosition();
}

void DlgEdit::SetDialogParent(DWORD Sets)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetDialogParent(Sets);
	else
#endif
		lineEdit->SetDialogParent(Sets);
}


void DlgEdit::SetDropDownBox(bool NewDropDownBox)
{
	if (Type == DLGEDIT_SINGLELINE)
		lineEdit->SetDropDownBox(NewDropDownBox);
}

int DlgEdit::GetMaxLength() const
{
	if (Type == DLGEDIT_SINGLELINE)
		return lineEdit->GetMaxLength();

	return 0;
}

void DlgEdit::SetMaxLength(int Length)
{
	if (Type == DLGEDIT_SINGLELINE)
		lineEdit->SetMaxLength(Length);
}

void DlgEdit::SetPasswordMode(bool Mode)
{
	if (Type == DLGEDIT_SINGLELINE)
		lineEdit->SetPasswordMode(Mode);
}

void DlgEdit::SetOvertypeMode(bool Mode)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetOvertypeMode(Mode);
	else
#endif
		lineEdit->SetOvertypeMode(Mode);
}

bool DlgEdit::GetOvertypeMode() const
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->GetOvertypeMode();
	else
#endif
		return lineEdit->GetOvertypeMode();
}

void DlgEdit::SetInputMask(string_view const InputMask)
{
	if (Type == DLGEDIT_SINGLELINE)
		lineEdit->SetInputMask(InputMask);
}

string DlgEdit::GetInputMask() const
{
	if (Type == DLGEDIT_SINGLELINE)
		return lineEdit->GetInputMask();

	return {}; //???
}

void DlgEdit::SetEditBeyondEnd(bool Mode)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetEditBeyondEnd(Mode);
	else
#endif
		lineEdit->SetEditBeyondEnd(Mode);
}

void DlgEdit::SetClearFlag(bool Flag)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetClearFlag(Flag);
	else
#endif
		lineEdit->SetClearFlag(Flag);
}

bool DlgEdit::GetClearFlag() const
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->GetClearFlag();
	else
#endif
		return lineEdit->GetClearFlag();
}

void DlgEdit::SetHiString(string_view const Str)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
	{
		; //multiEdit;
	}
	else
#endif
		lineEdit->SetHiString(Str);
}

void DlgEdit::Changed()
{
#if defined(PROJECT_DI_MEMOEDIT)
#endif
	{
		lineEdit->Changed();
	}
}

void DlgEdit::SetString(string_view const Str)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
	{
		; //multiEdit;
	}
	else
#endif
	{
		lineEdit->SetString(Str);
	}
}

void DlgEdit::InsertString(string_view const Str)
{
#if defined(PROJECT_DI_MEMOEDIT)
	if (Type == DLGEDIT_MULTILINE)
	{
		; //multiEdit;
	}
	else
#endif
		lineEdit->InsertString(Str);
}

const string& DlgEdit::GetString(int Row) const
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
	{
		; //multiEdit;
		static_assert(false, "no implementation");
	}
	else
#endif
		return lineEdit->GetString();
}

void DlgEdit::SetCurPos(int NewCol, int NewRow) // Row==-1 - current line
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetCurPos(NewCol,NewRow);
	else
#endif
	{
		lineEdit->SetCurPos(NewCol);
		//lineEdit->AdjustMarkBlock();
	}
}

int DlgEdit::GetCurPos() const
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->GetCurPos();
	else
#endif
		return lineEdit->GetCurPos();
}

int DlgEdit::GetCurRow() const
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->GetCurRow();
	else
#endif
		return 0;
}

int DlgEdit::GetTabCurPos() const
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->GetCurPos();
	else
#endif
		return lineEdit->GetTabCurPos();
}

void DlgEdit::SetTabCurPos(int NewPos)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetCurPos(NewPos,multiEdit->GetCurRow()); //???
	else
#endif
		lineEdit->SetTabCurPos(NewPos);
}


void DlgEdit::SetPersistentBlocks(bool Mode)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetPersistentBlocks(Mode);
	else
#endif
		lineEdit->SetPersistentBlocks(Mode);
}

int  DlgEdit::GetPersistentBlocks() const
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->GetPersistentBlocks();
	else
#endif
		return lineEdit->GetPersistentBlocks();
}

void DlgEdit::SetDelRemovesBlocks(bool Mode)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetDelRemovesBlocks(Mode);
	else
#endif
		lineEdit->SetDelRemovesBlocks(Mode);
}

int  DlgEdit::GetDelRemovesBlocks() const
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->GetDelRemovesBlocks();
	else
#endif
		return lineEdit->GetDelRemovesBlocks();
}

void DlgEdit::SetObjectColor(PaletteColors Color,PaletteColors SelColor,PaletteColors ColorUnChanged)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetObjectColor(Color,SelColor,ColorUnChanged);
	else
#endif
		lineEdit->SetObjectColor(Color,SelColor,ColorUnChanged);
}
void DlgEdit::SetObjectColor(const FarColor& Color,const FarColor& SelColor,const FarColor& ColorUnChanged)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetObjectColor(Color,SelColor,ColorUnChanged);
	else
#endif
		lineEdit->SetObjectColor(Color,SelColor,ColorUnChanged);
}

void DlgEdit::GetObjectColor(FarColor& Color, FarColor& SelColor, FarColor& ColorUnChanged) const
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return 0;// multiEdit->GetObjectColor(Color, SelColor, ColorUnChanged);
	else
#endif
		return lineEdit->GetObjectColor(Color, SelColor, ColorUnChanged);
}

void DlgEdit::FastShow()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		;//multiEdit->FastShow();
	else
#endif
		lineEdit->FastShow();
}

int DlgEdit::GetLeftPos() const
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return 0; // multiEdit->GetLeftPos();
	else
#endif
		return lineEdit->GetLeftPos();
}

void DlgEdit::SetLeftPos(int NewPos,int Row) // Row==-1 - current line
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		;//multiEdit->SetLeftPos(NewPos,Row);
	else
#endif
		lineEdit->SetLeftPos(NewPos);
}

void DlgEdit::DeleteBlock()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->DeleteBlock();
	else
#endif
		lineEdit->DeleteBlock();
}

int DlgEdit::GetLength() const
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return 0; // multiEdit->GetLength();
	else
#endif
		return lineEdit->GetLength();
}

void DlgEdit::Select(int Start,int End)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		;//multiEdit->Select(Start,End);
	else
#endif
	{
		lineEdit->Select(Start,End);
		lineEdit->AdjustMarkBlock();
	}
}

void DlgEdit::RemoveSelection()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		;//multiEdit->RemoveSelection();
	else
#endif
	{
		lineEdit->RemoveSelection();
	}
}

void DlgEdit::GetSelection(intptr_t &Start,intptr_t &End) const
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		;//multiEdit->GetSelection();
	else
#endif
		lineEdit->GetSelection(Start,End);
}

void DlgEdit::Xlat(bool All)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->Xlat();
	else
#endif
		lineEdit->Xlat(All);
}

int  DlgEdit::GetStrSize(int Row) const
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return 0;//multiEdit->
	else
#endif
		return lineEdit->m_Str.size();
}

void DlgEdit::SetCursorType(bool const Visible, size_t const Size)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetCursorType(Visible,Size);
	else
#endif
		lineEdit->SetCursorType(Visible,Size);
}

void DlgEdit::GetCursorType(bool& Visible, size_t& Size) const
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->GetCursorType(Visible,Size);
	else
#endif
		lineEdit->GetCursorType(Visible,Size);
}

bool DlgEdit::GetReadOnly() const
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->GetReadOnly();
	else
#endif
		return lineEdit->GetReadOnly();
}

void DlgEdit::SetReadOnly(bool NewReadOnly)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetReadOnly(NewReadOnly);
	else
#endif
		lineEdit->SetReadOnly(NewReadOnly);
}

void DlgEdit::SetCallbackState(bool Enable)
{
	lineEdit->SetCallbackState(Enable);
}

void DlgEdit::AutoComplete(bool Manual, bool DelBlock)
{
	return lineEdit->AutoComplete(Manual, DelBlock);
}

BitFlags& DlgEdit::Flags() const
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->m_Flags;
	else
#endif
		return lineEdit->m_Flags;
}

void DlgEdit::Hide()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->Hide();
	else
#endif
		lineEdit->Hide();
}

void DlgEdit::ShowConsoleTitle()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->ShowConsoleTitle();
	else
#endif
		lineEdit->ShowConsoleTitle();
}

void DlgEdit::SetScreenPosition()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetScreenPosition();
	else
#endif
		lineEdit->SetScreenPosition();
}

void DlgEdit::ResizeConsole()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->ResizeConsole();
	else
#endif
		lineEdit->ResizeConsole();
}

long long DlgEdit::VMProcess(int OpCode, void* vParam, long long iParam)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->VMProcess(OpCode,vParam,iParam);
	else
#endif
		return lineEdit->VMProcess(OpCode,vParam,iParam);
}

void DlgEdit::EditChange(void* aParam)
{
	static_cast<DlgEdit*>(aParam)->DoEditChange();
}

void DlgEdit::DoEditChange() const
{
	const auto dialog = GetDialog();
	if (dialog->IsInited())
	{
		dialog->SendMessage(DN_EDITCHANGE, m_Index, nullptr);
	}
}

bool DlgEdit::HistoryGetSimilar(string &strStr, int LastCmdPartLength, bool bAppend) const
{
	return iHistory?iHistory->GetSimilar(strStr, LastCmdPartLength, bAppend):false;
}

Dialog* DlgEdit::GetDialog() const
{
	return dynamic_cast<Dialog*>(GetOwner().get());
}
