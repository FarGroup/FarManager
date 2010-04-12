/*
dlgedit.cpp

Одиночная строка редактирования для диалога (как наследник класса Edit)
Мультиредактор

*/

#include "headers.hpp"
#pragma hdrstop

#include "dlgedit.hpp"

DlgEdit::DlgEdit(ScreenObject *pOwner,DLGEDITTYPE Type)
{
	DlgEdit::Type=Type;
#if defined(PROJECT_DI_MEMOEDIT)
	multiEdit=NULL;
#endif
	lineEdit=NULL;

	switch (Type)
	{
		case DLGEDIT_MULTILINE:
#if defined(PROJECT_DI_MEMOEDIT)
			multiEdit=new Editor(pOwner,true); // ??? (pOwner) ?
#endif
			break;
		case DLGEDIT_SINGLELINE:
			lineEdit=new Edit; // ??? (pOwner) ?
			break;
	}
}

DlgEdit::~DlgEdit()
{
	if (lineEdit)  delete lineEdit;

#if defined(PROJECT_DI_MEMOEDIT)

	if (multiEdit) delete multiEdit;

#endif
}

int DlgEdit::ProcessKey(int Key)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->ProcessKey(Key);
	else
#endif
		return lineEdit->ProcessKey(Key);
}

int DlgEdit::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
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

void DlgEdit::SetPosition(int X1,int Y1,int X2,int Y2)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetPosition(X1,Y1,X2,Y2);
	else
#endif
		lineEdit->SetPosition(X1,Y1,X2,Y2);
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

void DlgEdit::GetPosition(int& X1,int& Y1,int& X2,int& Y2)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->GetPosition(X1,Y1,X2,Y2);
	else
#endif
		lineEdit->GetPosition(X1,Y1,X2,Y2);
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


void DlgEdit::SetDropDownBox(int NewDropDownBox)
{
	if (Type == DLGEDIT_SINGLELINE)
		lineEdit->SetDropDownBox(NewDropDownBox);
}

int DlgEdit::GetMaxLength()
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

void DlgEdit::SetPasswordMode(int Mode)
{
	if (Type == DLGEDIT_SINGLELINE)
		lineEdit->SetPasswordMode(Mode);
}

void DlgEdit::SetOvertypeMode(int Mode)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetOvertypeMode(Mode);
	else
#endif
		lineEdit->SetOvertypeMode(Mode);
}

int DlgEdit::GetOvertypeMode()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->GetOvertypeMode();
	else
#endif
		return lineEdit->GetOvertypeMode();
}

void DlgEdit::SetInputMask(const char *InputMask)
{
	if (Type == DLGEDIT_SINGLELINE)
		lineEdit->SetInputMask(InputMask);
}

char* DlgEdit::GetInputMask()
{
	if (Type == DLGEDIT_SINGLELINE)
		return lineEdit->GetInputMask();

	return ""; //???
}

void DlgEdit::SetEditBeyondEnd(int Mode)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetEditBeyondEnd(Mode);
	else
#endif
		lineEdit->SetEditBeyondEnd(Mode);
}

void DlgEdit::SetClearFlag(int Flag)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetClearFlag(Flag);
	else
#endif
		lineEdit->SetClearFlag(Flag);
}

int DlgEdit::GetClearFlag(void)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->GetClearFlag();
	else
#endif
		return lineEdit->GetClearFlag();
}

const char* DlgEdit::GetStringAddr()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
	{
		return NULL; //??? //multiEdit;
	}
	else
#endif
		return lineEdit->GetStringAddr();
}

void DlgEdit::SetHiString(const char *Str)
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

void DlgEdit::SetString(const char *Str)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
	{
		; //multiEdit;
	}
	else
#endif
		lineEdit->SetString(Str);
}

void DlgEdit::GetString(char *Str,int MaxSize,int Row)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
	{
		; //multiEdit;
	}
	else
#endif
		lineEdit->GetString(Str,MaxSize);
}

int DlgEdit::GetTabCurPos()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->GetCurPos(); // GetCurCol???
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

void DlgEdit::SetCurPos(int NewCol, int NewRow) // Row==-1 - current line
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetCurPos(NewCol,NewRow);
	else
#endif
		lineEdit->SetCurPos(NewCol);
}

int DlgEdit::GetCurPos()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->GetCurPos(); // GetCurCol???
	else
#endif
		return lineEdit->GetCurPos();
}

int DlgEdit::GetCurRow()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->GetCurRow();
	else
#endif
		return 0;
}

void DlgEdit::SetPersistentBlocks(int Mode)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetPersistentBlocks(Mode);
	else
#endif
		lineEdit->SetPersistentBlocks(Mode);
}

int  DlgEdit::GetPersistentBlocks(void)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->GetPersistentBlocks();
	else
#endif
		return lineEdit->GetPersistentBlocks();
}

void DlgEdit::SetDelRemovesBlocks(int Mode)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetDelRemovesBlocks(Mode);
	else
#endif
		lineEdit->SetDelRemovesBlocks(Mode);
}

int  DlgEdit::GetDelRemovesBlocks(void)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->GetDelRemovesBlocks();
	else
#endif
		return lineEdit->GetDelRemovesBlocks();
}

void DlgEdit::SetObjectColor(int Color,int SelColor,int ColorUnChanged)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetObjectColor(Color,SelColor,ColorUnChanged);
	else
#endif
		lineEdit->SetObjectColor(Color,SelColor,ColorUnChanged);
}

long DlgEdit::GetObjectColor()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return 0;// multiEdit->GetObjectColor();
	else
#endif
		return lineEdit->GetObjectColor();
}

int DlgEdit::GetObjectColorUnChanged()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return 0; // multiEdit->GetObjectColorUnChanged();
	else
#endif
		return lineEdit->GetObjectColorUnChanged();
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

int DlgEdit::GetLeftPos()
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

int DlgEdit::GetLength()
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
		lineEdit->Select(Start,End);
}

void DlgEdit::GetSelection(int &Start,int &End)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		;//multiEdit->GetSelection();
	else
#endif
		lineEdit->GetSelection(Start,End);
}

void DlgEdit::Xlat(BOOL All)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->Xlat();
	else
#endif
		lineEdit->Xlat(All);
}

int  DlgEdit::GetStrSize(int Row)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return 0;//multiEdit->
	else
#endif
		return lineEdit->StrSize;
}

void DlgEdit::SetCursorType(int Visible,int Size)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetCursorType(Visible,Size);
	else
#endif
		lineEdit->SetCursorType(Visible,Size);
}

void DlgEdit::GetCursorType(int &Visible,int &Size)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->GetCursorType(Visible,Size);
	else
#endif
		lineEdit->GetCursorType(Visible,Size);
}

int DlgEdit::GetReadOnly()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->GetReadOnly();
	else
#endif
		return lineEdit->GetReadOnly();
}

void DlgEdit::SetReadOnly(int NewReadOnly)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->SetReadOnly(NewReadOnly);
	else
#endif
		lineEdit->SetReadOnly(NewReadOnly);
}

BitFlags& DlgEdit::Flags()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->Flags;
	else
#endif
		return lineEdit->Flags;
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

void DlgEdit::Hide0()
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		multiEdit->Hide0();
	else
#endif
		lineEdit->Hide0();
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

__int64 DlgEdit::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
#if defined(PROJECT_DI_MEMOEDIT)

	if (Type == DLGEDIT_MULTILINE)
		return multiEdit->VMProcess(OpCode,vParam,iParam);
	else
#endif
		return lineEdit->VMProcess(OpCode,vParam,iParam);
}
