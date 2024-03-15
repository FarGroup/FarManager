/*
editor.cpp

Редактор
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
#include "editor.hpp"

// Internal:
#include "edit.hpp"
#include "keyboard.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "dialog.hpp"
#include "fileedit.hpp"
#include "scrbuf.hpp"
#include "taskbar.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "clipboard.hpp"
#include "xlat.hpp"
#include "datetime.hpp"
#include "strmix.hpp"
#include "wakeful.hpp"
#include "colormix.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "encoding.hpp"
#include "uuids.far.hpp"
#include "uuids.far.dialogs.hpp"
#include "RegExp.hpp"
#include "plugins.hpp"
#include "lang.hpp"
#include "string_utils.hpp"
#include "global.hpp"
#include "codepage.hpp"

// Platform:

// Common:
#include "common/bytes_view.hpp"
#include "common/scope_exit.hpp"
#include "common/view/enumerate.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static int GlobalEditorCount{};

enum class Editor::undo_type: char
{
	edit,
	insert_string,
	delete_string,
	begin,
	end
};

enum class Editor::SearchReplaceDisposition
{
	Cancel,
	Prev,
	Next,
	All,
	ContinueBackward,
	ContinueForward,
};

/* $ 04.11.2003 SKV
на любом выходе если была нажата кнопка выделения,
и она его "сняла" (сделала 0-й ширины), то его надо убрать.
*/
class EditorBlockGuard
{
public:
	NONCOPYABLE(EditorBlockGuard);

	EditorBlockGuard(Editor& ed, void (Editor::*method)()):
		ed(ed),
		method(method),
		needCheckUnmark(false)
	{}

	~EditorBlockGuard()
	{
		if (needCheckUnmark)
			std::invoke(method, ed);
	}

	void SetNeedCheckUnmark(bool State) { needCheckUnmark = State; }

private:
	Editor& ed;
	void (Editor::*method)();
	bool needCheckUnmark;
};

eol Editor::GetDefaultEOL()
{
	return Global->Opt->EdOpt.NewFileUnixEOL? eol::unix : eol::win;
}

Editor::Editor(window_ptr Owner, bool DialogUsed):
	SimpleScreenObject(std::move(Owner)),
	GlobalEOL(GetDefaultEOL()),
	EdOpt(Global->Opt->EdOpt),
	m_SearchDlgParams
	{
		.SearchStr = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::view_edit).SearchStr,
		.ReplaceStr = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::view_edit).ReplaceStr.value_or(string{}),
		.CaseSensitive = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::view_edit).CaseSensitive.value_or(false),
		.WholeWords = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::view_edit).WholeWords.value_or(false),
		.Regex = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::view_edit).Regex.value_or(false),
		.Fuzzy = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::view_edit).Fuzzy.value_or(false),
		.PreserveStyle = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::view_edit).PreserveStyle.value_or(false)
	},
	EditorID(::GlobalEditorCount++),
	Color(colors::PaletteColorToFarColor(COL_EDITORTEXT)),
	SelColor(colors::PaletteColorToFarColor(COL_EDITORSELECTEDTEXT))
{
	if (DialogUsed)
		m_Flags.Set(FEDITOR_DIALOGMEMOEDIT);

	UnmarkMacroBlock();
	PushString({});
}


Editor::~Editor()
{
	FreeAllocatedData();
	if (!fake_editor)
		KeepInitParameters();
}

void Editor::FreeAllocatedData()
{
	m_AutoDeletedColors.clear();
	m_ColorList.clear();
	Lines.clear();
	UndoData.clear();
	UndoSavePos = UndoPos = UndoData.end();
	UndoSkipLevel=0;
	ClearSessionBookmarks();
	m_it_AnyBlockStart = m_it_LastGetLine = m_it_CurLine = m_it_TopScreen = EndIterator();
}

void Editor::SwapState(Editor& swap_state)
{
	// BUGBUGBUG not all fields swapped
	using std::ranges::swap;
	Lines.swap(swap_state.Lines);
	swap(m_it_TopScreen, swap_state.m_it_TopScreen);
	swap(m_it_CurLine, swap_state.m_it_CurLine);
	swap(m_it_LastGetLine, swap_state.m_it_LastGetLine);
	UndoData.swap(swap_state.UndoData);
	swap(UndoPos, swap_state.UndoPos);
	swap(UndoSavePos, swap_state.UndoSavePos);
	swap(UndoSkipLevel, swap_state.UndoSkipLevel);
	swap(LastChangeStrPos, swap_state.LastChangeStrPos);
	swap(GlobalEOL, swap_state.GlobalEOL);
	swap(m_it_MBlockStart, swap_state.m_it_MBlockStart);
	swap(m_it_AnyBlockStart, swap_state.m_it_AnyBlockStart);
	swap(m_BlockType, swap_state.m_BlockType);
	swap(MBlockStartX, swap_state.MBlockStartX);
	swap(VBlockX, swap_state.VBlockX);
	swap(VBlockSizeX, swap_state.VBlockSizeX);
	swap(VBlockSizeY, swap_state.VBlockSizeY);
	swap(MacroSelectionStart, swap_state.MacroSelectionStart);
	swap(m_StartLine, swap_state.m_StartLine);
	swap(StartChar, swap_state.StartChar);
	m_SavePos.swap(swap_state.m_SavePos);
	SessionBookmarks.swap(swap_state.SessionBookmarks);
	swap(SessionPos, swap_state.SessionPos);
	swap(NewSessionPos, swap_state.NewSessionPos);
	decoded.swap(swap_state.decoded);
	swap(m_FoundLine, swap_state.m_FoundLine);
	swap(m_FoundPos, swap_state.m_FoundPos);
	swap(m_FoundSize, swap_state.m_FoundSize);
	swap(m_ColorList, swap_state.m_ColorList);
	swap(m_AutoDeletedColors, swap_state.m_AutoDeletedColors);
	swap(MaxRightPosState, swap_state.MaxRightPosState);
}

// Consider: Since this function deals with Search/Replace dialog parameters and nothing else,
// should it be called after the dialog was closed (not cancelled) instead of in Edior's destructor?
void Editor::KeepInitParameters() const
{
	m_SearchDlgParams.SaveToShared(SearchReplaceDlgParams::SharedGroup::view_edit);
}

void Editor::DisplayObject()
{
	ShowEditor();
}

void Editor::ShowEditor()
{
	if (Lines.empty())
		return;

	Color = colors::PaletteColorToFarColor(COL_EDITORTEXT);
	SelColor = colors::PaletteColorToFarColor(COL_EDITORSELECTEDTEXT);

	XX2 = m_Where.right - (EdOpt.ShowScrollBar && ScrollBarRequired(ObjHeight(), Lines.size())? 1 : 0);
	/* 17.04.2002 skv
	  Что б курсор не бегал при Alt-F9 в конце длинного файла.
	  Если на экране есть свободное место, и есть текст сверху,
	  перепозиционируем.
	*/

	if (!EdOpt.AllowEmptySpaceAfterEof)
	{
		while (CalcDistance(m_it_TopScreen, EndIterator()) < ObjHeight())
		{
			if (m_it_TopScreen != Lines.begin())
				--m_it_TopScreen;
			else
				break;
		}
	}

	/*
	  если курсор вдруг оказался "за экраном",
	  подвинем экран под курсор, а не
	  курсор загоним в экран.
	*/

	while (CalcDistance(m_it_TopScreen, m_it_CurLine) >= ObjHeight())
	{
		++m_it_TopScreen;
	}

	auto CurPos = m_it_CurLine->GetTabCurPos();

	if (!EdOpt.CursorBeyondEOL)
	{
		if (!MaxRightPosState.m_LastState || MaxRightPosState.m_LastState->first != m_it_CurLine || MaxRightPosState.m_LastState->second != CurPos)
		{
			MaxRightPosState.Position = CurPos;
		}

		const auto RealCurPos = m_it_CurLine->GetCurPos();
		const auto Length = m_it_CurLine->GetLength();

		if (RealCurPos>Length)
		{
			m_it_CurLine->SetCurPos(Length);
			m_it_CurLine->SetLeftPos(0);
			CurPos=m_it_CurLine->GetTabCurPos();
		}

		MaxRightPosState.m_LastState = { m_it_CurLine, CurPos };
	}

	//---
	//для корректной отрисовки текста с табами у CurLine должна быть корректная LeftPos до начала отрисовки
	//так же это позволяет возвращать корректную EditorInfo.LeftPos в EE_REDRAW
	m_it_CurLine->SetHorizontalPosition(m_Where.left, XX2);
	m_it_CurLine->FixLeftPos();
	//---
	if (!Pasting)
	{
		if (!Global->ScrBuf->GetLockCount())
		{
			if (!m_Flags.Check(FEDITOR_DIALOGMEMOEDIT))
			{
				++m_InEERedraw;
				SCOPE_EXIT{ --m_InEERedraw; };
				Global->CtrlObject->Plugins->ProcessEditorEvent(EE_REDRAW, EEREDRAW_ALL, this);
			}
		}
	}

	DrawScrollbar();

	auto LeftPos = m_it_CurLine->GetLeftPos();
	const Edit::ShowInfo info{ LeftPos, CurPos };
	auto Y = m_Where.top;

	for (auto CurPtr = m_it_TopScreen; CurPtr != Lines.end() && Y <= m_Where.bottom; ++CurPtr, ++Y)
	{
		CurPtr->SetEditBeyondEnd(true);
		CurPtr->SetPosition({ m_Where.left, Y, XX2, Y });
		CurPtr->SetLeftPos(LeftPos);
		CurPtr->SetTabCurPos(CurPos);
		CurPtr->SetEditBeyondEnd(EdOpt.CursorBeyondEOL);

		if(CurPtr==m_it_CurLine)
		{
			CurPtr->SetOvertypeMode(m_Flags.Check(FEDITOR_OVERTYPE));
			CurPtr->Show();
		}
		else
		{
			CurPtr->FastShow(&info);
		}
	}

	if (Y != m_Where.bottom + 1)
	{
		SetScreen({ m_Where.left, Y, XX2, m_Where.bottom }, L' ', Color); //Пустые строки после конца текста
	}

	if (IsVerticalSelection() && VBlockSizeX > 0 && VBlockSizeY > 0)
	{
		int CurScreenLine = m_it_CurLine.Number() - CalcDistance(m_it_TopScreen,m_it_CurLine);
		LeftPos=m_it_CurLine->GetLeftPos();

		Y = m_Where.top;
		for (auto CurPtr = m_it_TopScreen; Y <= m_Where.bottom; ++Y)
		{
			if (CurPtr != Lines.end())
			{
				if (CurScreenLine >= m_it_AnyBlockStart.Number() && CurScreenLine < m_it_AnyBlockStart.Number() + VBlockSizeY)
				{
					int BlockX1 = VBlockX - LeftPos + m_Where.left;
					int BlockX2 = VBlockX + VBlockSizeX - 1 - LeftPos + m_Where.left;

					if (BlockX1 < m_Where.left)
						BlockX1 = m_Where.left;

					if (BlockX2>XX2)
						BlockX2=XX2;

					if (BlockX1 <= XX2 && BlockX2 >= m_Where.left)
						Global->ScrBuf->ApplyColor({ BlockX1, Y, BlockX2, Y }, SelColor);
				}

				++CurPtr;
				CurScreenLine++;
			}
		}
	}

	// BUGBUG
	if (const auto HostFileEditor = std::dynamic_pointer_cast<FileEditor>(m_Owner.lock()))
		HostFileEditor->ShowStatus();
}


/*$ 10.08.2000 skv
  Wrapper for Modified.
*/
void Editor::TextChanged(bool State)
{
	m_Flags.Change(FEDITOR_MODIFIED,State);
}


bool Editor::CheckLine(const iterator& line)
{
	FOR_RANGE(Lines, i)
	{
		if (i == line)
			return true;
	}
	return false;
}

int Editor::BlockStart2NumLine(int *Pos)
{
	if (IsAnySelection())
	{
		if (Pos)
		{
			*Pos = m_it_AnyBlockStart->RealPosToVisual(IsVerticalSelection()?
				m_it_AnyBlockStart->VisualPosToReal(VBlockX) :
				m_it_AnyBlockStart->m_SelStart);
		}

		return CalcDistance(FirstLine(), m_it_AnyBlockStart);
	}

	return -1;
}

int Editor::BlockEnd2NumLine(int *Pos)
{
	int iLine=-1, iPos=-1;

	if (IsAnySelection())
	{
		auto eLine = m_it_AnyBlockStart;
		iLine=BlockStart2NumLine(nullptr); // получили строку начала блока

		if (IsVerticalSelection())
		{
			for (int Line=VBlockSizeY; eLine != Lines.end() && Line > 0; --Line, ++eLine)
			{
				iPos=eLine->RealPosToVisual(eLine->VisualPosToReal(VBlockX+VBlockSizeX));
				iLine++;
			}

			iLine--;
		}
		else
		{
			while (eLine != Lines.end())  // поиск строки, содержащую конец блока
			{
				intptr_t StartSel, EndSel;
				eLine->GetSelection(StartSel,EndSel);

				if (EndSel == -1) // это значит, что конец блока "за строкой"
					eLine->GetRealSelection(StartSel,EndSel);

				if (StartSel == -1)
				{
					const auto NextLine = std::next(eLine);
					if (NextLine != Lines.end())
					{
						// Если в текущей строки нет выделения, это еще не значит что мы в конце. Это может быть только начало :)
						NextLine->GetSelection(StartSel, EndSel);

						if (EndSel == -1) // это значит, что конец блока "за строкой"
							NextLine->GetRealSelection(StartSel, EndSel);

						if (StartSel==-1)
						{
							break;
						}
					}
					else
						break;
				}
				else
				{
					iPos=eLine->RealPosToVisual(EndSel);
					iLine++;
				}

				++eLine;
			}

			iLine--;
		}
	}

	if (Pos)
		*Pos=iPos;

	return iLine;
}

struct Editor::InternalEditorBookmark
{
	intptr_t Line;
	intptr_t Cursor;
	intptr_t ScreenLine;
	intptr_t LeftPos;
};

long long Editor::VMProcess(int OpCode, void* vParam, long long iParam)
{
	const auto CurPos = m_it_CurLine->GetCurPos();

	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			// Editor always has at least one line
			return Lines.empty() || (Lines.size() == 1 && Lines.front().m_Str.empty());
		case MCODE_C_EOF:
			return IsLastLine(m_it_CurLine) && CurPos >= m_it_CurLine->GetLength();
		case MCODE_C_BOF:
			return m_it_CurLine == Lines.begin() && !CurPos;
		case MCODE_C_SELECTED:
			return IsAnySelection();
		case MCODE_V_EDITORCURPOS:
			return m_it_CurLine->GetTabCurPos()+1;
		case MCODE_V_EDITORREALPOS:
			return m_it_CurLine->GetCurPos()+1;
		case MCODE_V_EDITORCURLINE:
			return m_it_CurLine.Number() + 1;
		case MCODE_V_ITEMCOUNT:
		case MCODE_V_EDITORLINES:
			return Lines.size();
			// работа со стековыми закладками
		case MCODE_F_BM_ADD:
			AddSessionBookmark();
			return TRUE;
		case MCODE_F_BM_CLEAR:
			ClearSessionBookmarks();
			return TRUE;
		case MCODE_F_BM_NEXT:
			return NextSessionBookmark();
		case MCODE_F_BM_PREV:
			return PrevSessionBookmark();
		case MCODE_F_BM_BACK:
			return BackSessionBookmark();
		case MCODE_F_BM_STAT:
		{
			switch (iParam)
			{
				case 0: // BM.Stat(0) возвращает количество
					return GetSessionBookmarks(nullptr);
				case 1: // индекс текущей закладки (0 если закладок нет)
					return CurrentSessionBookmarkIdx()+1;
			}
			return 0;
		}
		case MCODE_F_BM_PUSH:             // N=BM.push() - сохранить текущую позицию в виде закладки в конце стека
			PushSessionBookMark();
			return TRUE;
		case MCODE_F_BM_POP:              // N=BM.pop() - восстановить текущую позицию из закладки в конце стека и удалить закладку
			return PopSessionBookMark();
		case MCODE_F_BM_GOTO:             // N=BM.goto([n]) - переход на закладку с указанным индексом (0 --> текущую)
			return GotoSessionBookmark(static_cast<int>(iParam - 1));
		case MCODE_F_BM_GET:                   // N=BM.Get(Idx,M) - возвращает координаты строки (M==0) или колонки (M==1) закладки с индексом (Idx=1...)
		{
			long long Ret=-1;
			InternalEditorBookmark ebm{};
			const auto iMode = std::bit_cast<intptr_t>(vParam);

			if (iMode >= 0 && iMode <= 3 && GetSessionBookmark(static_cast<int>(iParam - 1), &ebm))
			{
				switch (iMode)
				{
					case 0: Ret=ebm.Line+1;  break;
					case 1: Ret=ebm.Cursor+1; break;
					case 2: Ret=ebm.LeftPos+1; break;
					case 3: Ret=ebm.ScreenLine+1; break;
				}
			}

			return Ret;
		}
		case MCODE_F_BM_DEL:                   // N=BM.Del(Idx) - удаляет закладку с указанным индексом (x=1...), 0 - удаляет текущую закладку
		{
			return DeleteSessionBookmark(PointerToSessionBookmark(static_cast<int>(iParam - 1)));
		}
		case MCODE_F_EDITOR_SEL:
		{
			int iPos;
			switch (const auto Action = std::bit_cast<intptr_t>(vParam))
			{
				case 0:  // Get Param
				{
					switch (iParam)
					{
						case 0:  // return FirstLine
						{
							return BlockStart2NumLine(nullptr)+1;
						}
						case 1:  // return FirstPos
						{
							if (BlockStart2NumLine(&iPos) != -1)
								return iPos+1;

							return 0;
						}
						case 2:  // return LastLine
						{
							return BlockEnd2NumLine(nullptr)+1;
						}
						case 3:  // return LastPos
						{
							if (BlockEnd2NumLine(&iPos) != -1)
								return iPos+1;

							return 0;
						}
						case 4: // return block type (0=nothing 1=stream, 2=column)
						{
							return IsVerticalSelection()? 2 : IsStreamSelection()? 1 : 0;
						}
					}

					break;
				}
				case 1:  // Set Pos
				{
					switch (iParam)
					{
						case 0: // begin block (FirstLine & FirstPos)
						case 1: // end block (LastLine & LastPos)
						{
							int iLine;
							if (!iParam)
								iLine=BlockStart2NumLine(&iPos);
							else
								iLine=BlockEnd2NumLine(&iPos);

							if (iLine > -1 && iPos > -1)
							{
								const auto NumLine = m_it_CurLine.Number();
								const auto TabCurPos = m_it_CurLine->GetTabCurPos();

								GoToLineAndShow(iLine);
								m_it_CurLine->SetCurPos(m_it_CurLine->VisualPosToReal(iPos));

								if (!EdOpt.CursorBeyondEOL && m_it_CurLine->GetCurPos() > m_it_CurLine->GetLength())
								{
									GoToLineAndShow(NumLine);
									m_it_CurLine->SetCurPos(m_it_CurLine->VisualPosToReal(TabCurPos));
									return 0;
								}
								return 1;
							}

							return 0;
						}
					}

					break;
				}
				case 2: // Set Stream Selection Edge
				case 3: // Set Column Selection Edge
				{
					switch (iParam)
					{
						case 0:  // selection start
						{
							m_it_MBlockStart=m_it_CurLine;
							MBlockStartX=m_it_CurLine->GetCurPos();
							return 1;
						}
						case 1:  // selection finish
						{
							int Ret=0;

							if (m_it_MBlockStart != Lines.end())
							{
								EditorSelect eSel{ sizeof(eSel) };
								eSel.BlockType=(Action == 2)?BTYPE_STREAM:BTYPE_COLUMN;
								eSel.BlockStartPos=MBlockStartX;
								eSel.BlockWidth=m_it_CurLine->GetCurPos()-MBlockStartX;

								if (!eSel.BlockWidth && m_it_MBlockStart == m_it_CurLine)
								{
									UnmarkBlock();
								}
								else
								{
									const auto bl = CalcDistance(FirstLine(), m_it_MBlockStart);
									const auto el = CalcDistance(FirstLine(), m_it_CurLine);

									if (bl > el)
									{
										eSel.BlockStartLine=el;
										eSel.BlockHeight = CalcDistance(m_it_CurLine, m_it_MBlockStart) + 1;
									}
									else
									{
										eSel.BlockStartLine=bl;
										eSel.BlockHeight = CalcDistance(m_it_MBlockStart, m_it_CurLine) + 1;
									}

									if (bl > el || (bl == el && eSel.BlockWidth<0))
									{
										eSel.BlockWidth*=-1;
										eSel.BlockStartPos=m_it_CurLine->GetCurPos();
									}

									Ret=EditorControl(ECTL_SELECT,0,&eSel);
								}
							}

							UnmarkMacroBlock();
							Show();
							return Ret;
						}
					}

					break;
				}
				case 4: // UnMark sel block
				{
					const auto NeedRedraw = IsAnySelection();
					UnmarkBlock();
					UnmarkMacroBlock();

					if (NeedRedraw)
						Show();

					return 1;
				}
			}

			break;
		}
		case MCODE_F_EDITOR_DELLINE:  // N=Editor.DelLine([Line])
		case MCODE_F_EDITOR_INSSTR:   // N=Editor.InsStr([S[,Line]])
		case MCODE_F_EDITOR_SETSTR:   // N=Editor.SetStr([S[,Line]])
		{
			if (m_Flags.Check(FEDITOR_LOCKMODE))
			{
				return 0;
			}

			int DestLine=iParam;

			if (DestLine<0)
				DestLine = m_it_CurLine.Number();

			const auto EditPtr = GetStringByNumber(DestLine);

			if (EditPtr == Lines.end())
			{
				return 0;
			}

			//TurnOffMarkingBlock();

			switch (OpCode)
			{
				case MCODE_F_EDITOR_DELLINE:  // N=Editor.DelLine([Line])
				{
					DeleteString(EditPtr, true);
					return 1;
				}
				case MCODE_F_EDITOR_INSSTR:  // N=Editor.InsStr([S[,Line]])
				{
					const auto NewEditPtr = InsertString(NullToEmpty(static_cast<const wchar_t*>(vParam)), std::next(EditPtr));
					NewEditPtr->SetEOL(EditPtr->GetEOL());
					AddUndoData(undo_type::insert_string, NewEditPtr->GetString(), EditPtr->GetEOL(), DestLine + 1, 0);
					Change(ECTYPE_ADDED,DestLine+1);
					TextChanged(true);
					return 1;
				}
				case MCODE_F_EDITOR_SETSTR:  // N=Editor.SetStr([S[,Line]])
				{
					const auto EOL = EditPtr->GetEOL();
					AddUndoData(undo_type::edit, EditPtr->GetString(), EOL, DestLine, EditPtr->GetCurPos());
					EditPtr->SetString(static_cast<const wchar_t*>(vParam));
					EditPtr->SetEOL(EOL);
					Change(ECTYPE_CHANGED,DestLine);
					TextChanged(true);
					return 1;
				}
			}
		}
		[[fallthrough]];
		case MCODE_V_EDITORSELVALUE: // Editor.SelValue
		{
			*static_cast<string*>(vParam) = IsVerticalSelection()? VBlock2Text() : Block2Text();
			return 1;
		}
	}

	return 0;
}

static bool is_clear_selection_key(unsigned const Key, bool Persistent)
{
	static const unsigned int Keys[]
	{
		KEY_UP,        KEY_NUMPAD8,
		KEY_DOWN,      KEY_NUMPAD2,
		KEY_PGUP,      KEY_NUMPAD9,
		KEY_PGDN,      KEY_NUMPAD3,
		KEY_CTRLPGUP,  KEY_RCTRLPGUP,  KEY_CTRLNUMPAD9,  KEY_RCTRLNUMPAD9,
		KEY_CTRLPGDN,  KEY_RCTRLPGDN,  KEY_CTRLNUMPAD3,  KEY_RCTRLNUMPAD3,
		KEY_CTRLUP,    KEY_RCTRLUP,    KEY_CTRLNUMPAD8,  KEY_RCTRLNUMPAD8,
		KEY_CTRLDOWN,  KEY_RCTRLDOWN,  KEY_CTRLNUMPAD2,  KEY_RCTRLNUMPAD2,
		KEY_CTRLN,     KEY_RCTRLN,
		KEY_CTRLE,     KEY_RCTRLE,
	}, KeysP[]
	{
		KEY_ENTER,     KEY_NUMENTER,
	};

	return Edit::is_clear_selection_key(Key) || contains(Keys, Key) || (Persistent && contains(KeysP, Key));
}

bool Editor::ProcessKeyInternal(const Manager::Key& Key, bool& Refresh)
{
	auto LocalKey = Key;

	if (LocalKey()==KEY_NONE)
		return true;

	switch (LocalKey())
	{
		case KEY_CTRLSHIFTUP:   case KEY_CTRLSHIFTNUMPAD8: LocalKey = KEY_SHIFTUP;   break;
		case KEY_CTRLSHIFTDOWN: case KEY_CTRLSHIFTNUMPAD2: LocalKey = KEY_SHIFTDOWN; break;

		case KEY_CTRLALTUP:     case KEY_RCTRLRALTUP:     case KEY_CTRLRALTUP:        case KEY_RCTRLALTUP:     LocalKey = KEY_ALTUP;   break;
		case KEY_CTRLALTDOWN:   case KEY_RCTRLRALTDOWN:   case KEY_CTRLRALTDOWN:      case KEY_RCTRLALTDOWN:   LocalKey = KEY_ALTDOWN; break;

		case KEY_RCTRLALTLEFT:  case KEY_RCTRLALTNUMPAD4: case KEY_CTRLRALTLEFT:  case KEY_CTRLRALTNUMPAD4: case KEY_RCTRLRALTLEFT:  case KEY_RCTRLRALTNUMPAD4: LocalKey = KEY_CTRLALTLEFT;  break;
		case KEY_RCTRLALTRIGHT: case KEY_RCTRLALTNUMPAD6: case KEY_CTRLRALTRIGHT: case KEY_CTRLRALTNUMPAD6: case KEY_RCTRLRALTRIGHT: case KEY_RCTRLRALTNUMPAD6: LocalKey = KEY_CTRLALTRIGHT; break;
		case KEY_RCTRLALTPGUP:  case KEY_RCTRLALTNUMPAD9: case KEY_CTRLRALTPGUP:  case KEY_CTRLRALTNUMPAD9: case KEY_RCTRLRALTPGUP:  case KEY_RCTRLRALTNUMPAD9: LocalKey = KEY_CTRLALTPGUP;  break;
		case KEY_RCTRLALTPGDN:  case KEY_RCTRLALTNUMPAD3: case KEY_CTRLRALTPGDN:  case KEY_CTRLRALTNUMPAD3: case KEY_RCTRLRALTPGDN:  case KEY_RCTRLRALTNUMPAD3: LocalKey = KEY_CTRLALTPGDN;  break;
		case KEY_RCTRLALTHOME:  case KEY_RCTRLALTNUMPAD7: case KEY_CTRLRALTHOME:  case KEY_CTRLRALTNUMPAD7: case KEY_RCTRLRALTHOME:  case KEY_RCTRLRALTNUMPAD7: LocalKey = KEY_CTRLALTHOME;  break;
		case KEY_RCTRLALTEND:   case KEY_RCTRLALTNUMPAD1: case KEY_CTRLRALTEND:   case KEY_CTRLRALTNUMPAD1: case KEY_RCTRLRALTEND:   case KEY_RCTRLRALTNUMPAD1: LocalKey = KEY_CTRLALTEND;   break;

		case KEY_RCTRLRALTBRACKET:     case KEY_CTRLRALTBRACKET:     case KEY_RCTRLALTBRACKET:     LocalKey = KEY_CTRLALTBRACKET;     break;
		case KEY_RCTRLRALTBACKBRACKET: case KEY_CTRLRALTBACKBRACKET: case KEY_RCTRLALTBACKBRACKET: LocalKey = KEY_CTRLALTBACKBRACKET; break;
	}

	auto CurPos = m_it_CurLine->GetCurPos();
	const auto CurVisPos = GetLineCurPos();

	if (!Pasting && IsAnySelection() && is_clear_selection_key(LocalKey(), EdOpt.PersistentBlocks))
	{
		TurnOffMarkingBlock();

		if (!EdOpt.PersistentBlocks)
			UnmarkBlock();
	}

	if (any_of(LocalKey(), KEY_ALTD, KEY_RALTD))
		LocalKey=KEY_CTRLK;

	// работа с закладками
	if (LocalKey()>=KEY_CTRL0 && LocalKey()<=KEY_CTRL9)
	{
		auto KeyProcessed = GotoBookmark(LocalKey() - KEY_CTRL0);
		if (KeyProcessed)
			Refresh = true;
		return KeyProcessed;
	}

	if (LocalKey()>=KEY_CTRLSHIFT0 && LocalKey()<=KEY_CTRLSHIFT9)
		LocalKey=LocalKey()-KEY_CTRLSHIFT0+KEY_RCTRL0;

	if (LocalKey()>=KEY_RCTRL0 && LocalKey()<=KEY_RCTRL9)
		return SetBookmark(LocalKey()-KEY_RCTRL0);

	intptr_t SelStart=0,SelEnd=0;
	int SelFirst=FALSE;
	int SelAtBeginning=FALSE;
	EditorBlockGuard _bg(*this,&Editor::UnmarkEmptyBlock);

	switch (LocalKey())
	{
		case KEY_SHIFTLEFT:      case KEY_SHIFTRIGHT:
		case KEY_SHIFTUP:        case KEY_SHIFTDOWN:
		case KEY_SHIFTHOME:      case KEY_SHIFTEND:
		case KEY_SHIFTNUMPAD4:   case KEY_SHIFTNUMPAD6:
		case KEY_SHIFTNUMPAD8:   case KEY_SHIFTNUMPAD2:
		case KEY_SHIFTNUMPAD7:   case KEY_SHIFTNUMPAD1:
		case KEY_CTRLSHIFTLEFT:  case KEY_CTRLSHIFTNUMPAD4:
		case KEY_RCTRLSHIFTLEFT: case KEY_RCTRLSHIFTNUMPAD4:
		{
			UnmarkEmptyBlock();
			_bg.SetNeedCheckUnmark(true);
			m_it_CurLine->GetRealSelection(SelStart,SelEnd);

			if (m_Flags.Check(FEDITOR_CURPOSCHANGEDBYPLUGIN))
			{
				bool IsLastSelectionLine=SelStart>=0;
				const auto NextLine = std::next(m_it_CurLine);
				if (NextLine != Lines.end())
				{
					intptr_t NextSelStart=-1,NextSelEnd=0;
					NextLine->GetRealSelection(NextSelStart, NextSelEnd);
					IsLastSelectionLine=IsLastSelectionLine&&(NextSelStart<0);
				}
				bool IsSpecialCase=false;
				if (m_it_CurLine != Lines.begin())
				{
					intptr_t PrevSelStart=-1,PrevSelEnd=0;
					std::prev(m_it_CurLine)->GetRealSelection(PrevSelStart,PrevSelEnd);
					IsSpecialCase=SelStart<0&&PrevSelStart==0&&PrevSelEnd<0;
				}
				if(!((m_it_CurLine==m_it_AnyBlockStart&&CurPos==SelStart)||(IsLastSelectionLine&&CurPos==SelEnd)||(IsSpecialCase&&0==CurPos)))
					TurnOffMarkingBlock();

				m_Flags.Clear(FEDITOR_CURPOSCHANGEDBYPLUGIN);
			}

			if (!m_Flags.Check(FEDITOR_MARKINGBLOCK))
			{
				UnmarkBlock();
				BeginStreamMarking(m_it_CurLine);
				SelFirst=TRUE;
				SelStart=SelEnd=CurPos;
			}
			else
			{
				SelAtBeginning=m_it_CurLine==m_it_AnyBlockStart && CurPos==SelStart;

				if (SelStart==-1)
				{
					SelStart=SelEnd=CurPos;
				}
			}
		}
	}

	switch (LocalKey())
	{
		case KEY_CTRLSHIFTPGUP:   case KEY_CTRLSHIFTNUMPAD9:
		case KEY_RCTRLSHIFTPGUP:  case KEY_RCTRLSHIFTNUMPAD9:
		case KEY_CTRLSHIFTHOME:   case KEY_CTRLSHIFTNUMPAD7:
		case KEY_RCTRLSHIFTHOME:  case KEY_RCTRLSHIFTNUMPAD7:
		{
			Pasting++;

			while (m_it_CurLine != Lines.begin())
			{
				ProcessKeyInternal(Manager::Key(KEY_SHIFTPGUP), Refresh);
			}

			if (any_of(LocalKey(), KEY_CTRLSHIFTHOME, KEY_CTRLSHIFTNUMPAD7, KEY_RCTRLSHIFTHOME, KEY_RCTRLSHIFTNUMPAD7))
				ProcessKeyInternal(Manager::Key(KEY_SHIFTHOME), Refresh);

			Pasting--;
			Refresh = true;
			return true;
		}
		case KEY_CTRLSHIFTPGDN:   case KEY_CTRLSHIFTNUMPAD3:
		case KEY_RCTRLSHIFTPGDN:  case KEY_RCTRLSHIFTNUMPAD3:
		case KEY_CTRLSHIFTEND:    case KEY_CTRLSHIFTNUMPAD1:
		case KEY_RCTRLSHIFTEND:   case KEY_RCTRLSHIFTNUMPAD1:
		{
			Pasting++;

			while (!IsLastLine(m_it_CurLine))
			{
				ProcessKeyInternal(Manager::Key(KEY_SHIFTPGDN), Refresh);
			}

			if (any_of(LocalKey(), KEY_CTRLSHIFTEND, KEY_CTRLSHIFTNUMPAD1, KEY_RCTRLSHIFTEND, KEY_RCTRLSHIFTNUMPAD1))
				ProcessKeyInternal(Manager::Key(KEY_SHIFTEND), Refresh);

			Pasting--;
			Refresh = true;
			return true;
		}
		case KEY_SHIFTPGUP:       case KEY_SHIFTNUMPAD9:
		{
			Pasting++;
			repeat(m_Where.height() - 1, [this, &Refresh]
			{
				ProcessKeyInternal(Manager::Key(KEY_SHIFTUP), Refresh);

				if (!EdOpt.CursorBeyondEOL)
				{
					if (m_it_CurLine->GetCurPos()>m_it_CurLine->GetLength())
					{
						m_it_CurLine->SetCurPos(m_it_CurLine->GetLength());
					}
				}
			});

			Pasting--;
			Refresh = true;
			return true;
		}
		case KEY_SHIFTPGDN:       case KEY_SHIFTNUMPAD3:
		{
			Pasting++;
			repeat(m_Where.height() - 1, [this, &Refresh]
			{
				ProcessKeyInternal(Manager::Key(KEY_SHIFTDOWN), Refresh);

				if (!EdOpt.CursorBeyondEOL)
				{
					if (m_it_CurLine->GetCurPos()>m_it_CurLine->GetLength())
					{
						m_it_CurLine->SetCurPos(m_it_CurLine->GetLength());
					}
				}
			});

			Pasting--;
			Refresh = true;
			return true;
		}
		case KEY_SHIFTHOME:       case KEY_SHIFTNUMPAD7:
		{
			Pasting++;
			m_it_CurLine->Select(0,SelAtBeginning?SelEnd:SelStart);
			ProcessKeyInternal(Manager::Key(KEY_HOME), Refresh);
			Pasting--;
			Refresh = true;
			return true;
		}
		case KEY_SHIFTEND:
		case KEY_SHIFTNUMPAD1:
		{
			{
				Pasting++;
				int CurLength=m_it_CurLine->GetLength();

				if (!SelAtBeginning || SelFirst)
				{
					m_it_CurLine->Select(SelStart,CurLength);
				}
				else
				{
					if (SelEnd!=-1)
						m_it_CurLine->Select(SelEnd,CurLength);
					else
						m_it_CurLine->Select(CurLength,-1);
				}

				m_it_CurLine->SetRightCoord(XX2);
				ProcessKeyInternal(Manager::Key(KEY_END), Refresh);
				Pasting--;
				Refresh = true;
			}
			return true;
		}
		case KEY_SHIFTLEFT:  case KEY_SHIFTNUMPAD4:
		{
			if (!CurPos && m_it_CurLine == Lines.begin())
				return true;

			const auto OldCur = m_it_CurLine;
			Pasting++;
			ProcessKeyInternal(Manager::Key(KEY_LEFT), Refresh);
			Pasting--;

			if (OldCur == m_it_CurLine)
			{
				if (SelAtBeginning || SelFirst)
				{
					m_it_CurLine->Select(m_it_CurLine->GetCurPos(), SelEnd);
				}
				else
				{
					m_it_CurLine->Select(SelStart, m_it_CurLine->GetCurPos());
				}
			}
			else
			{
				if (SelAtBeginning) //курсор в начале блока
				{
					m_it_AnyBlockStart = m_it_CurLine;
					m_it_CurLine->Select(m_it_CurLine->GetLength(), -1);
				}
				else // курсор в конце блока
				{
					OldCur->RemoveSelection();
					m_it_CurLine->GetRealSelection(SelStart, SelEnd);
					m_it_CurLine->Select(SelStart, m_it_CurLine->GetLength());
				}
			}

			Refresh = true;
			return true;
		}
		case KEY_SHIFTRIGHT:  case KEY_SHIFTNUMPAD6:
		{
			if (IsLastLine(m_it_CurLine) && CurPos == m_it_CurLine->GetLength() && !EdOpt.CursorBeyondEOL)
			{
				return true;
			}

			const auto OldCur = m_it_CurLine;
			Pasting++;
			ProcessKeyInternal(Manager::Key(KEY_RIGHT), Refresh);
			Pasting--;

			if (OldCur == m_it_CurLine)
			{
				if (SelAtBeginning)
				{
					m_it_CurLine->Select(m_it_CurLine->GetCurPos(), SelEnd);
				}
				else
				{
					m_it_CurLine->Select(SelStart, m_it_CurLine->GetCurPos());
				}
			}
			else
			{
				if (SelAtBeginning)
				{
					OldCur->RemoveSelection();
					m_it_AnyBlockStart=m_it_CurLine;
				}
				else
				{
					OldCur->Select(SelStart,-1);
				}
			}

			Refresh = true;
			return true;
		}
		case KEY_CTRLSHIFTLEFT:  case KEY_CTRLSHIFTNUMPAD4:
		case KEY_RCTRLSHIFTLEFT: case KEY_RCTRLSHIFTNUMPAD4:
		{
			{
				int SkipSpace=TRUE;
				Pasting++;
				for (;;)
				{
					const auto& Str = m_it_CurLine->GetString();
					/* $ 12.11.2002 DJ
					   обеспечим корректную работу Ctrl-Shift-Left за концом строки
					*/
					size_t LocalCurPos = m_it_CurLine->GetCurPos();

					if (LocalCurPos > Str.size())
					{
						size_t SelStartPos = LocalCurPos;
						m_it_CurLine->ProcessKey(Manager::Key(KEY_END));
						LocalCurPos = m_it_CurLine->GetCurPos();

						if (m_it_CurLine->m_SelStart >= 0)
						{
							if (!SelAtBeginning)
								m_it_CurLine->Select(m_it_CurLine->m_SelStart, static_cast<int>(LocalCurPos));
							else
								m_it_CurLine->Select(static_cast<int>(LocalCurPos), m_it_CurLine->m_SelEnd);
						}
						else
							m_it_CurLine->Select(static_cast<int>(LocalCurPos), static_cast<int>(SelStartPos));
					}

					if (!LocalCurPos)
						break;

					if (std::iswblank(Str[LocalCurPos-1]) || IsWordDiv(EdOpt.strWordDiv,Str[LocalCurPos-1]))
					{
						if (SkipSpace)
						{
							ProcessKeyInternal(Manager::Key(KEY_SHIFTLEFT), Refresh);
							continue;
						}
						else
							break;
					}

					SkipSpace=FALSE;
					ProcessKeyInternal(Manager::Key(KEY_SHIFTLEFT), Refresh);
				}
				Pasting--;
				Refresh = true;
			}
			return true;
		}
		case KEY_CTRLSHIFTRIGHT:  case KEY_CTRLSHIFTNUMPAD6:
		case KEY_RCTRLSHIFTRIGHT: case KEY_RCTRLSHIFTNUMPAD6:
		{
			{
				int SkipSpace=TRUE;
				Pasting++;
				for (;;)
				{
					const auto& Str = m_it_CurLine->GetString();
					const size_t LocalCurPos = m_it_CurLine->GetCurPos();

					if (LocalCurPos >= Str.size())
						break;

					if (std::iswblank(Str[LocalCurPos]) || IsWordDiv(EdOpt.strWordDiv, Str[LocalCurPos]))
					{
						if (SkipSpace)
						{
							ProcessKeyInternal(Manager::Key(KEY_SHIFTRIGHT), Refresh);
							continue;
						}
						else
							break;
					}

					SkipSpace=FALSE;
					ProcessKeyInternal(Manager::Key(KEY_SHIFTRIGHT), Refresh);
				}
				Pasting--;
				Refresh = true;
			}
			return true;
		}
		case KEY_SHIFTDOWN:  case KEY_SHIFTNUMPAD2:
		{
			const auto NextLine = std::next(m_it_CurLine);
			if (NextLine == Lines.end())
				return true;

			CurPos = m_it_CurLine->RealPosToVisual(CurPos);

			if (SelAtBeginning)//Снимаем выделение
			{
				if (SelEnd==-1)
				{
					m_it_CurLine->RemoveSelection();
					m_it_AnyBlockStart = NextLine;
				}
				else
				{
					m_it_CurLine->Select(SelEnd,-1);
				}

				NextLine->GetRealSelection(SelStart,SelEnd);

				if (SelStart!=-1)
					SelStart = NextLine->RealPosToVisual(SelStart);

				if (SelEnd!=-1)
					SelEnd = NextLine->RealPosToVisual(SelEnd);

				if (SelStart==-1)
				{
					SelStart=0;
					SelEnd=CurPos;
				}
				else
				{
					if (SelEnd!=-1 && SelEnd<CurPos)
					{
						SelStart=SelEnd;
						SelEnd=CurPos;
					}
					else
					{
						SelStart=CurPos;
					}
				}

				if (SelStart!=-1)
					SelStart = NextLine->VisualPosToReal(SelStart);

				if (SelEnd!=-1)
					SelEnd = NextLine->VisualPosToReal(SelEnd);
			}
			else //расширяем выделение
			{
				m_it_CurLine->Select(SelStart,-1);
				SelStart = NextLine->VisualPosToReal(0);
				SelEnd = NextLine->VisualPosToReal(CurPos);
			}

			if (!EdOpt.CursorBeyondEOL && SelEnd > NextLine->GetLength())
			{
				SelEnd = NextLine->GetLength();
			}

			if (!EdOpt.CursorBeyondEOL && SelStart > NextLine->GetLength())
			{
				SelStart = NextLine->GetLength();
			}
			NextLine->Select(SelStart,SelEnd);
			Down();
			Refresh = true;
			return true;
		}
		case KEY_SHIFTUP: case KEY_SHIFTNUMPAD8:
		{
			if (m_it_CurLine == Lines.begin())
				return false;

			const auto PrevLine = std::prev(m_it_CurLine);
			if (SelAtBeginning || SelFirst) // расширяем выделение
			{
				m_it_CurLine->Select(0,SelEnd);
				SelStart=m_it_CurLine->RealPosToVisual(CurPos);

				if (!EdOpt.CursorBeyondEOL && PrevLine->VisualPosToReal(SelStart) > PrevLine->GetLength())
				{
					SelStart = PrevLine->RealPosToVisual(PrevLine->GetLength());
				}

				SelStart = PrevLine->VisualPosToReal(SelStart);
				PrevLine->Select(SelStart, -1);
				m_it_AnyBlockStart = PrevLine;
			}
			else // снимаем выделение
			{
				CurPos=m_it_CurLine->RealPosToVisual(CurPos);

				if (!SelStart)
				{
					m_it_CurLine->RemoveSelection();
				}
				else
				{
					m_it_CurLine->Select(0,SelStart);
				}

				PrevLine->GetRealSelection(SelStart, SelEnd);

				if (SelStart != -1)
					SelStart = PrevLine->RealPosToVisual(SelStart);

				if (SelEnd != -1)
					SelEnd = PrevLine->RealPosToVisual(SelEnd);

				if (SelStart==-1)
				{
					m_it_AnyBlockStart = PrevLine;
					SelStart = PrevLine->VisualPosToReal(CurPos);
					SelEnd=-1;
				}
				else
				{
					if (CurPos<SelStart)
					{
						SelEnd=SelStart;
						SelStart=CurPos;
					}
					else
					{
						SelEnd=CurPos;
					}

					SelStart = PrevLine->VisualPosToReal(SelStart);
					SelEnd = PrevLine->VisualPosToReal(SelEnd);

					if (!EdOpt.CursorBeyondEOL && SelEnd > PrevLine->GetLength())
					{
						SelEnd = PrevLine->GetLength();
					}

					if (!EdOpt.CursorBeyondEOL && SelStart > PrevLine->GetLength())
					{
						SelStart = PrevLine->GetLength();
					}
				}

				PrevLine->Select(SelStart, SelEnd);
			}

			Up();
			Refresh = true;
			return true;
		}
		case KEY_CTRLADD:
		case KEY_RCTRLADD:
		{
			Copy(TRUE);
			return true;
		}
		case KEY_CTRLA:
		case KEY_RCTRLA:
		{
			UnmarkBlock();
			SelectAll();
			return true;
		}
		case KEY_CTRLU:
		case KEY_RCTRLU:
		{
			UnmarkMacroBlock();
			UnmarkBlock();
			return true;
		}
		case KEY_CTRLC:
		case KEY_RCTRLC:
		case KEY_CTRLINS:    case KEY_CTRLNUMPAD0:
		case KEY_RCTRLINS:   case KEY_RCTRLNUMPAD0:
		{
			if (/*!EdOpt.PersistentBlocks && */ !IsAnySelection())
			{
				BeginStreamMarking(m_it_CurLine);
				m_it_CurLine->AddSelect(0,-1);
				Refresh = true;
			}

			Copy(FALSE);
			return true;
		}
		case KEY_CTRLP:
		case KEY_RCTRLP:
		case KEY_CTRLM:
		case KEY_RCTRLM:
		{
			if (m_Flags.Check(FEDITOR_LOCKMODE))
				return true;

			if (IsAnySelection())
			{
				intptr_t CurSelStart, CurSelEnd;

				if (IsStreamSelection())
					m_it_CurLine->GetSelection(CurSelStart, CurSelEnd);
				else
				{
					if (m_it_CurLine.Number() < m_it_AnyBlockStart.Number() || m_it_CurLine.Number() >= (m_it_AnyBlockStart.Number() + VBlockSizeY))
					{
						CurSelStart = -1;
						CurSelEnd = -1;
					}
					else
					{
						CurSelStart = m_it_CurLine->VisualPosToReal(VBlockX);
						CurSelEnd = m_it_CurLine->VisualPosToReal(VBlockX + VBlockSizeX);
					}
				}

				Pasting++;

				// TODO: using an internal clipboard to copy/move block is a not a best design choice.
				// Currently we substitute a new local instance of internal_clipboard for this purpose.
				// Consider implementing it without using the clipboard.

				const auto OverriddenClipboard = OverrideClipboard();

				ProcessKeyInternal(Manager::Key(any_of(LocalKey(), KEY_CTRLP, KEY_RCTRLP)? KEY_CTRLINS : KEY_SHIFTDEL), Refresh);

				/* $ 10.04.2001 SVS
				  ^P/^M - некорректно работали: условие для CurPos должно быть ">=",
				   а не "меньше".
				*/
				if (any_of(LocalKey(), KEY_CTRLM, KEY_RCTRLM) && CurSelStart != -1 && CurSelEnd != -1)
				{
					if (CurPos >= CurSelEnd)
						m_it_CurLine->SetCurPos(CurPos - (CurSelEnd - CurSelStart));
					else
						m_it_CurLine->SetCurPos(CurPos);
				}

				ProcessKeyInternal(Manager::Key(KEY_SHIFTINS), Refresh);
				Pasting--;
				ClearInternalClipboard();

				/*$ 08.02.2001 SKV
				  всё делалось с pasting'ом, поэтому redraw плагинам не ушел.
				  сделаем его.
				*/
				Refresh = true;
			}

			return true;
		}
		case KEY_CTRLX:
		case KEY_RCTRLX:
		case KEY_SHIFTDEL:
		case KEY_SHIFTNUMDEL:
		case KEY_SHIFTDECIMAL:
			Copy(FALSE);
			[[fallthrough]];
		case KEY_CTRLD:
		case KEY_RCTRLD:
		{
			if (m_Flags.Check(FEDITOR_LOCKMODE))
				return true;

			TurnOffMarkingBlock();
			DeleteBlock();
			Refresh = true;
			return true;
		}

		case KEY_CTRLV:
		case KEY_RCTRLV:
		case KEY_SHIFTINS: case KEY_SHIFTNUMPAD0:
		{
			if (m_Flags.Check(FEDITOR_LOCKMODE))
				return true;

			Pasting++;

			PasteFromClipboard();
			// MarkingBlock=!VBlockStart;
			m_Flags.Change(FEDITOR_MARKINGBLOCK, IsStreamSelection());
			m_Flags.Clear(FEDITOR_MARKINGVBLOCK);

			if (!EdOpt.PersistentBlocks)
				UnmarkBlock();

			Pasting--;
			Refresh = true;
			return true;
		}

		case KEY_LEFT: case KEY_NUMPAD4:
		{
			m_Flags.Set(FEDITOR_NEWUNDO);

			if (!CurPos && m_it_CurLine != Lines.begin())
			{
				Up();
				m_it_CurLine->ProcessKey(Manager::Key(KEY_END));
			}
			else
			{
				m_it_CurLine->ProcessKey(Manager::Key(KEY_LEFT));
			}
			Refresh = true;

			return true;
		}

		case KEY_INS: case KEY_NUMPAD0:
			m_Flags.Invert(FEDITOR_OVERTYPE);
			Refresh = true;
			return true;

		case KEY_NUMDEL:
		case KEY_DEL:
		{
			if (!m_Flags.Check(FEDITOR_LOCKMODE))
			{
				// Del в самой последней позиции ничего не удаляет, поэтому не модифицируем...
				if (IsLastLine(m_it_CurLine) && CurPos >= m_it_CurLine->GetLength() && !IsAnySelection())
					return true;

				/* $ 07.03.2002 IS
				   Снимем выделение, если блок все равно пустой
				*/
				if (!Pasting)
					UnmarkEmptyBlock();

				if (!Pasting && EdOpt.DelRemovesBlocks && IsAnySelection())
					DeleteBlock();
				else
				{
					if (CurPos>=m_it_CurLine->GetLength())
					{
						AddUndoData(undo_type::begin);
						AddUndoData(undo_type::edit, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), m_it_CurLine.Number(), m_it_CurLine->GetCurPos());

						const auto NextLine = std::next(m_it_CurLine);
						if (NextLine == Lines.end())
						{
							m_it_CurLine->SetEOL(eol::none);
						}
						else
						{
							intptr_t CurSelStart, CurSelEnd, NextSelStart, NextSelEnd;
							int Length=m_it_CurLine->GetLength();
							m_it_CurLine->GetSelection(CurSelStart, CurSelEnd);
							NextLine->GetSelection(NextSelStart, NextSelEnd);
							const auto NextEOL = NextLine->GetEOL();
							const auto& Str = NextLine->GetString();
							m_it_CurLine->InsertString(Str);
							m_it_CurLine->SetEOL(NextLine->GetEOL());
							m_it_CurLine->SetCurPos(CurPos);

							if (m_FoundLine == NextLine)
							{
								m_FoundLine = m_it_CurLine;
								m_FoundPos += m_FoundLine->GetLength();
							}

							DeleteString(NextLine, true);

							m_it_CurLine->SetEOL(NextEOL);

							if (NextSelStart!=-1)
							{
								if (CurSelStart==-1)
								{
									m_it_CurLine->Select(Length+NextSelStart,NextSelEnd==-1 ? -1:Length+NextSelEnd);
									m_it_AnyBlockStart = m_it_CurLine;
								}
								else
									m_it_CurLine->Select(CurSelStart, NextSelEnd == -1 ? -1 : Length + NextSelEnd);
							}
						}

						AddUndoData(undo_type::end);
					}
					else
					{
						AddUndoData(undo_type::edit, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), m_it_CurLine.Number(), m_it_CurLine->GetCurPos());
						m_it_CurLine->ProcessKey(Manager::Key(KEY_DEL));
					}
					Change(ECTYPE_CHANGED, m_it_CurLine.Number());
					TextChanged(true);
				}

				Refresh = true;
			}

			return true;
		}

		case KEY_BS:
		{
			if (!m_Flags.Check(FEDITOR_LOCKMODE))
			{
				// Bs в самом начале нихрена ничего не удаляет, посему не будем выставлять
				if (m_it_CurLine == Lines.begin() && !CurPos && !IsAnySelection())
					return true;

				TextChanged(true);
				bool IsDelBlock = false;

				if (EdOpt.BSLikeDel)
				{
					if (!Pasting && EdOpt.DelRemovesBlocks && IsAnySelection())
						IsDelBlock = true;
				}
				else
				{
					if (!Pasting && !EdOpt.PersistentBlocks && IsStreamSelection())
						IsDelBlock = true;
				}

				if (IsDelBlock)
					DeleteBlock();
				else if (!CurPos && m_it_CurLine != Lines.begin())
				{
					Pasting++;
					Up();
					m_it_CurLine->ProcessKey(Manager::Key(KEY_CTRLEND));
					ProcessKeyInternal(Manager::Key(KEY_DEL), Refresh);
					Pasting--;
				}
				else
				{
					AddUndoData(undo_type::edit, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), m_it_CurLine.Number(), m_it_CurLine->GetCurPos());
					m_it_CurLine->ProcessKey(Manager::Key(KEY_BS));
					Change(ECTYPE_CHANGED, m_it_CurLine.Number());
				}

				Refresh = true;
			}

			return true;
		}

		case KEY_CTRLBS:
		case KEY_RCTRLBS:
		{
			if (!m_Flags.Check(FEDITOR_LOCKMODE))
			{
				TextChanged(true);

				if (!Pasting && !EdOpt.PersistentBlocks && IsStreamSelection())
					DeleteBlock();
				else if (!CurPos && m_it_CurLine != Lines.begin())
					ProcessKeyInternal(Manager::Key(KEY_BS), Refresh);
				else
				{
					AddUndoData(undo_type::edit, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), m_it_CurLine.Number(), m_it_CurLine->GetCurPos());
					m_it_CurLine->ProcessKey(Manager::Key(KEY_CTRLBS));
					Change(ECTYPE_CHANGED, m_it_CurLine.Number());
				}

				Refresh = true;
			}

			return true;
		}

		case KEY_UP: case KEY_NUMPAD8:
		case KEY_DOWN: case KEY_NUMPAD2:
		{
			{
				m_Flags.Set(FEDITOR_NEWUNDO);
				const auto PrevMaxPos = MaxRightPosState.Position;
				const auto LastTopScreen = m_it_TopScreen;
				any_of(LocalKey(), KEY_UP, KEY_NUMPAD8)? Up() : Down();

				if (m_it_TopScreen!=LastTopScreen)
				{
					m_it_CurLine->SetHorizontalPosition(m_Where.left, XX2);
					m_it_CurLine->FixLeftPos();
				}

				if (!EdOpt.CursorBeyondEOL && PrevMaxPos > m_it_CurLine->GetTabCurPos())
				{
					m_it_CurLine->SetTabCurPos(PrevMaxPos);
				}
				Refresh = true;
			}
			return true;
		}

		case KEY_MSWHEEL_UP:
		case KEY_MSWHEEL_UP | KEY_ALT:
		case KEY_MSWHEEL_UP | KEY_RALT:
		{
			const auto Roll = (LocalKey() == KEY_MSWHEEL_UP? get_wheel_scroll_lines(Global->Opt->MsWheelDeltaEdit) : 1) * Key.NumberOfWheelEvents();
			repeat(Roll, [&] { ProcessKeyInternal(Manager::Key(KEY_CTRLUP), Refresh); });
			return true;
		}

		case KEY_MSWHEEL_DOWN:
		case KEY_MSWHEEL_DOWN | KEY_ALT:
		case KEY_MSWHEEL_DOWN | KEY_RALT:
		{
			const auto Roll = (LocalKey() == KEY_MSWHEEL_DOWN? get_wheel_scroll_lines(Global->Opt->MsWheelDeltaEdit) : 1) * Key.NumberOfWheelEvents();
			repeat(Roll, [&] { ProcessKeyInternal(Manager::Key(KEY_CTRLDOWN), Refresh); });
			return true;
		}

		case KEY_MSWHEEL_LEFT:
		case KEY_MSWHEEL_LEFT | KEY_ALT:
		case KEY_MSWHEEL_LEFT | KEY_RALT:
		{
			const auto Roll = (LocalKey() == KEY_MSWHEEL_LEFT? get_wheel_scroll_chars(Global->Opt->MsHWheelDeltaEdit) : 1) * Key.NumberOfWheelEvents();
			repeat(Roll, [&] { ProcessKeyInternal(Manager::Key(KEY_LEFT), Refresh); });
			return true;
		}

		case KEY_MSWHEEL_RIGHT:
		case KEY_MSWHEEL_RIGHT | KEY_ALT:
		case KEY_MSWHEEL_RIGHT | KEY_RALT:
		{
			const auto Roll = (LocalKey() == KEY_MSWHEEL_RIGHT? get_wheel_scroll_chars(Global->Opt->MsHWheelDeltaEdit) : 1) * Key.NumberOfWheelEvents();
			repeat(Roll, [&] { ProcessKeyInternal(Manager::Key(KEY_RIGHT), Refresh); });
			return true;
		}

		case KEY_CTRLUP:  case KEY_CTRLNUMPAD8:
		case KEY_RCTRLUP: case KEY_RCTRLNUMPAD8:
		{
			m_Flags.Set(FEDITOR_NEWUNDO);
			ScrollUp();
			Refresh = true;
			return true;
		}

		case KEY_CTRLDOWN:  case KEY_CTRLNUMPAD2:
		case KEY_RCTRLDOWN: case KEY_RCTRLNUMPAD2:
		{
			m_Flags.Set(FEDITOR_NEWUNDO);
			ScrollDown();
			Refresh = true;
			return true;
		}

		case KEY_PGUP:     case KEY_NUMPAD9:
		{
			m_Flags.Set(FEDITOR_NEWUNDO);

			repeat(m_Where.bottom - m_Where.top, [&]{ ScrollUp(); });

			Refresh = true;
			return true;
		}

		case KEY_PGDN:    case KEY_NUMPAD3:
		{
			m_Flags.Set(FEDITOR_NEWUNDO);

			repeat(m_Where.bottom - m_Where.top, [&] { ScrollDown(); });

			Refresh = true;
			return true;
		}

		case KEY_CTRLHOME:  case KEY_CTRLNUMPAD7:
		case KEY_RCTRLHOME: case KEY_RCTRLNUMPAD7:
		case KEY_CTRLPGUP:  case KEY_CTRLNUMPAD9:
		case KEY_RCTRLPGUP: case KEY_RCTRLNUMPAD9:
		{
			{
				m_Flags.Set(FEDITOR_NEWUNDO);
				int StartPos=m_it_CurLine->GetTabCurPos();
				m_it_TopScreen = m_it_CurLine = FirstLine();

				if (any_of(LocalKey(), KEY_CTRLHOME, KEY_RCTRLHOME, KEY_CTRLNUMPAD7, KEY_RCTRLNUMPAD7))
				{
					m_it_CurLine->SetCurPos(0);
					m_it_CurLine->SetLeftPos(0);
				}
				else
					m_it_CurLine->SetTabCurPos(StartPos);

				Refresh = true;
			}
			return true;
		}

		case KEY_CTRLEND:   case KEY_CTRLNUMPAD1:
		case KEY_RCTRLEND:  case KEY_RCTRLNUMPAD1:
		case KEY_CTRLPGDN:  case KEY_CTRLNUMPAD3:
		case KEY_RCTRLPGDN: case KEY_RCTRLNUMPAD3:
		{
			{
				m_Flags.Set(FEDITOR_NEWUNDO);
				int StartPos=m_it_CurLine->GetTabCurPos();
				m_it_CurLine = LastLine();
				m_it_TopScreen=m_it_CurLine;
				for (int I = m_Where.top; I < m_Where.bottom && m_it_TopScreen != Lines.begin(); ++I)
				{
					m_it_TopScreen->SetPosition({ m_Where.left, I, XX2, I });
					--m_it_TopScreen;
				}

				m_it_CurLine->SetLeftPos(0);

				if (any_of(LocalKey(), KEY_CTRLEND, KEY_RCTRLEND, KEY_CTRLNUMPAD1, KEY_RCTRLNUMPAD1))
				{
					m_it_CurLine->SetCurPos(m_it_CurLine->GetLength());
				}
				else
				{
					m_it_CurLine->SetTabCurPos(StartPos);
				}

				Refresh = true;
			}
			return true;
		}

		case KEY_NUMENTER:
		case KEY_ENTER:
		{
			if (!Pasting && !EdOpt.PersistentBlocks && IsStreamSelection())
				DeleteBlock();

			m_Flags.Set(FEDITOR_NEWUNDO);
			InsertString();
			Refresh = true;

			return true;
		}

		case KEY_CTRLN:
		case KEY_RCTRLN:
		{
			m_Flags.Set(FEDITOR_NEWUNDO);

			while (m_it_CurLine!=m_it_TopScreen)
			{
				--m_it_CurLine;
			}

			m_it_CurLine->SetCurPos(CurPos);
			Refresh = true;
			return true;
		}

		case KEY_CTRLE:
		case KEY_RCTRLE:
		{
			{
				m_Flags.Set(FEDITOR_NEWUNDO);
				auto CurPtr = m_it_TopScreen;
				for (int I = m_Where.top; I < m_Where.bottom; ++I, ++CurPtr)
				{
					if (IsLastLine(CurPtr))
						break;
				}

				m_it_CurLine=CurPtr;
				m_it_CurLine->SetCurPos(CurPos);
				Refresh = true;
			}
			return true;
		}

		case KEY_CTRLL:
		case KEY_RCTRLL:
		{
			m_Flags.Invert(FEDITOR_LOCKMODE);
			return true;
		}

		case KEY_CTRLY:
		case KEY_RCTRLY:
		{
			DeleteString(m_it_CurLine, false);
			Refresh = true;
			return true;
		}

		case KEY_F7:
		{
			DoSearchReplace(ShowSearchReplaceDialog(false));
			return true;
		}

		case KEY_CTRLF7:
		case KEY_RCTRLF7:
		{
			if (!m_Flags.Check(FEDITOR_LOCKMODE))
			{
				DoSearchReplace(ShowSearchReplaceDialog(true));
			}
			return true;
		}

		case KEY_SHIFTF7:
		{
			TurnOffMarkingBlock();
			DoSearchReplace(SearchReplaceDisposition::ContinueForward);
			return true;
		}

		case KEY_ALTF7:
		case KEY_RALTF7:
		{
			TurnOffMarkingBlock();
			DoSearchReplace(SearchReplaceDisposition::ContinueBackward);
			return true;
		}

		case KEY_F11:
		{
			/*
			if(!Flags.Check(FEDITOR_DIALOGMEMOEDIT))
			{
				Global->CtrlObject->Plugins->CurEditor=HostFileEditor; // this;
				if (Global->CtrlObject->Plugins->CommandsMenu(MODALTYPE_EDITOR,0,"Editor"))
					*PluginTitle=0;
				Refresh = true;
			}
			*/
			return true;
		}

		case KEY_CTRLSHIFTZ:
		case KEY_RCTRLSHIFTZ:
		case KEY_ALTBS:
		case KEY_RALTBS:
		case KEY_CTRLZ:
		case KEY_RCTRLZ:
		{
			if (!m_Flags.Check(FEDITOR_LOCKMODE))
			{
				Undo(any_of(LocalKey(), KEY_CTRLSHIFTZ, KEY_RCTRLSHIFTZ));
				m_Flags.Set(FEDITOR_NEWUNDO);
				Refresh = true;
			}

			return true;
		}

		case KEY_ALTF8:
		case KEY_RALTF8:
		{
			GoToPosition();

			// <GOTO_UNMARK:1>
			if (!EdOpt.PersistentBlocks)
				UnmarkBlock();

			// </GOTO_UNMARK>
			Refresh = true;
			return true;
		}

		case KEY_ALTU:
		case KEY_RALTU:
			if (!m_Flags.Check(FEDITOR_LOCKMODE))
			{
				BlockLeft();
				Refresh = true;
			}
			return true;

		case KEY_ALTI:
		case KEY_RALTI:
			if (!m_Flags.Check(FEDITOR_LOCKMODE))
			{
				BlockRight();
				Refresh = true;
			}
			return true;

		case KEY_ALTSHIFTLEFT:  case KEY_ALTSHIFTNUMPAD4:
		case KEY_RALTSHIFTLEFT: case KEY_RALTSHIFTNUMPAD4:
		case KEY_ALTLEFT:
		case KEY_RALTLEFT:
		{
			if (!CurPos)
				return true;

			ProcessVBlockMarking();

			Pasting++;
			{
				int Delta=m_it_CurLine->GetTabCurPos()-m_it_CurLine->RealPosToVisual(CurPos-1);

				if (m_it_CurLine->GetTabCurPos()>VBlockX)
					VBlockSizeX-=Delta;
				else
				{
					VBlockX-=Delta;
					VBlockSizeX+=Delta;
				}

				/* $ 25.07.2000 tran
				   остатки бага 22 - подправка при перебега за границу блока */
				if (VBlockSizeX<0)
				{
					VBlockSizeX=-VBlockSizeX;
					VBlockX-=VBlockSizeX;
				}

				ProcessKeyInternal(Manager::Key(KEY_LEFT), Refresh);
			}
			Pasting--;
			Refresh = true;
			return true;
		}

		case KEY_ALTSHIFTRIGHT:  case KEY_ALTSHIFTNUMPAD6:
		case KEY_RALTSHIFTRIGHT: case KEY_RALTSHIFTNUMPAD6:
		case KEY_ALTRIGHT:
		case KEY_RALTRIGHT:
		{
			/* $ 23.10.2000 tran
			   вместо GetTabCurPos надо вызывать GetCurPos -
			   сравнивать реальную позицию с реальной длиной
			   а было сравнение видимой позицией с реальной длиной*/
			if (!EdOpt.CursorBeyondEOL && m_it_CurLine->GetCurPos()>=m_it_CurLine->GetLength())
				return true;

			ProcessVBlockMarking();

			Pasting++;
			{
				/* $ 18.07.2000 tran
				     встань в начало текста, нажми alt-right, alt-pagedown,
				     выделится блок шириной в 1 колонку, нажми еще alt-right
				     выделение сбросится
				*/
				int VisPos=m_it_CurLine->RealPosToVisual(CurPos),
				           NextVisPos=m_it_CurLine->RealPosToVisual(CurPos+1);

				if (NextVisPos == VisPos)
				{
					// If the next physical pos got resolved into the same visual pos, we're likely standing on a surrogate. Try +2:
					NextVisPos = m_it_CurLine->RealPosToVisual(CurPos + 2);
				}

				const auto Delta=NextVisPos-VisPos;

				if (m_it_CurLine->GetTabCurPos()>=VBlockX+VBlockSizeX)
					VBlockSizeX+=Delta;
				else
				{
					VBlockX+=Delta;
					VBlockSizeX-=Delta;
				}

				/* $ 25.07.2000 tran
				   остатки бага 22 - подправка при перебега за границу блока */
				if (VBlockSizeX<0)
				{
					VBlockSizeX=-VBlockSizeX;
					VBlockX-=VBlockSizeX;
				}

				ProcessKeyInternal(Manager::Key(KEY_RIGHT), Refresh);
			}
			Pasting--;
			Refresh = true;
			return true;
		}

		/* $ 29.06.2000 IG
		  + CtrlAltLeft, CtrlAltRight для вертикальный блоков
		*/
		case KEY_CTRLALTLEFT:   case KEY_CTRLALTNUMPAD4:
		{
			{
				int SkipSpace=TRUE;
				Pasting++;
				for (;;)
				{
					const auto& Str = m_it_CurLine->GetString();
					size_t LocalCurPos = m_it_CurLine->GetCurPos();

					while (LocalCurPos > Str.size())
					{
						ProcessKeyInternal(Manager::Key(KEY_ALTSHIFTLEFT), Refresh);
						LocalCurPos = m_it_CurLine->GetCurPos();
					}

					if (!LocalCurPos)
						break;

					if (std::iswblank(Str[LocalCurPos - 1]) || IsWordDiv(EdOpt.strWordDiv, Str[LocalCurPos - 1]))
					{
						if (SkipSpace)
						{
							ProcessKeyInternal(Manager::Key(KEY_ALTSHIFTLEFT), Refresh);
							continue;
						}
						else
							break;
					}

					SkipSpace=FALSE;
					ProcessKeyInternal(Manager::Key(KEY_ALTSHIFTLEFT), Refresh);
				}

				Pasting--;
				Refresh = true;
			}
			return true;
		}

		case KEY_CTRLALTRIGHT:   case KEY_CTRLALTNUMPAD6:
		{
			{
				bool SkipSpace = true;
				Pasting++;
				for (;;)
				{
					const auto& Str = m_it_CurLine->GetString();
					const size_t LocalCurPos = m_it_CurLine->GetCurPos();

					if (LocalCurPos >= Str.size())
						break;

					if (std::iswblank(Str[LocalCurPos]) || IsWordDiv(EdOpt.strWordDiv, Str[LocalCurPos]))
					{
						if (SkipSpace)
						{
							ProcessKeyInternal(Manager::Key(KEY_ALTSHIFTRIGHT), Refresh);
							continue;
						}
						break;
					}

					SkipSpace = false;
					ProcessKeyInternal(Manager::Key(KEY_ALTSHIFTRIGHT), Refresh);
				}

				Pasting--;
				Refresh = true;
			}
			return true;
		}

		case KEY_ALTSHIFTUP:    case KEY_ALTSHIFTNUMPAD8:
		case KEY_RALTSHIFTUP:   case KEY_RALTSHIFTNUMPAD8:
		case KEY_ALTUP:
		case KEY_RALTUP:
		{
			if (m_it_CurLine == Lines.begin())
				return true;

			ProcessVBlockMarking();

			const auto PrevLine = std::prev(m_it_CurLine);
			if (!EdOpt.CursorBeyondEOL && VBlockX >= PrevLine->RealPosToVisual(PrevLine->GetLength()))
				return true;

			Pasting++;

			if (m_it_CurLine.Number() > m_it_AnyBlockStart.Number())
				VBlockSizeY--;
			else
			{
				++VBlockSizeY;
				--m_it_AnyBlockStart;
			}

			ProcessKeyInternal(Manager::Key(KEY_UP), Refresh);
			AdjustVBlock(CurVisPos);
			Pasting--;
			Refresh = true;
			return true;
		}

		case KEY_ALTSHIFTDOWN:  case KEY_ALTSHIFTNUMPAD2:
		case KEY_RALTSHIFTDOWN: case KEY_RALTSHIFTNUMPAD2:
		case KEY_ALTDOWN:
		case KEY_RALTDOWN:
		{
			const auto NextLine = std::next(m_it_CurLine);
			if (NextLine == Lines.end())
				return true;

			ProcessVBlockMarking();

			if (!EdOpt.CursorBeyondEOL && VBlockX >= NextLine->RealPosToVisual(NextLine->GetLength()))
				return true;

			Pasting++;

			if (m_it_CurLine.Number() >= m_it_AnyBlockStart.Number() + VBlockSizeY - 1)
				++VBlockSizeY;
			else
			{
				--VBlockSizeY;
				++m_it_AnyBlockStart;
			}

			ProcessKeyInternal(Manager::Key(KEY_DOWN), Refresh);
			AdjustVBlock(CurVisPos);
			Pasting--;
			Refresh = true;
			return true;
		}

		case KEY_ALTSHIFTHOME:  case KEY_ALTSHIFTNUMPAD7:
		case KEY_RALTSHIFTHOME: case KEY_RALTSHIFTNUMPAD7:
		case KEY_ALTHOME:
		case KEY_RALTHOME:
		{
			Pasting++;
			while (m_it_CurLine->GetCurPos()>0)
				ProcessKeyInternal(Manager::Key(KEY_ALTSHIFTLEFT), Refresh);
			Pasting--;
			Refresh = true;
			return true;
		}

		case KEY_ALTSHIFTEND:  case KEY_ALTSHIFTNUMPAD1:
		case KEY_RALTSHIFTEND: case KEY_RALTSHIFTNUMPAD1:
		case KEY_ALTEND:
		case KEY_RALTEND:
		{
			Pasting++;
			if (m_it_CurLine->GetCurPos()<m_it_CurLine->GetLength())
				while (m_it_CurLine->GetCurPos()<m_it_CurLine->GetLength())
					ProcessKeyInternal(Manager::Key(KEY_ALTSHIFTRIGHT), Refresh);

			if (m_it_CurLine->GetCurPos()>m_it_CurLine->GetLength())
				while (m_it_CurLine->GetCurPos()>m_it_CurLine->GetLength())
					ProcessKeyInternal(Manager::Key(KEY_ALTSHIFTLEFT), Refresh);
			Pasting--;
			Refresh = true;
			return true;
		}

		case KEY_ALTSHIFTPGUP:  case KEY_ALTSHIFTNUMPAD9:
		case KEY_RALTSHIFTPGUP: case KEY_RALTSHIFTNUMPAD9:
		case KEY_ALTPGUP:
		case KEY_RALTPGUP:
		{
			Pasting++;
			repeat(m_Where.bottom - m_Where.top, [&]{ ProcessKeyInternal(Manager::Key(KEY_ALTSHIFTUP), Refresh); });
			Pasting--;
			Refresh = true;
			return true;
		}

		case KEY_ALTSHIFTPGDN:  case KEY_ALTSHIFTNUMPAD3:
		case KEY_RALTSHIFTPGDN: case KEY_RALTSHIFTNUMPAD3:
		case KEY_ALTPGDN:
		case KEY_RALTPGDN:
		{
			Pasting++;
			repeat(m_Where.bottom - m_Where.top, [&]{ ProcessKeyInternal(Manager::Key(KEY_ALTSHIFTDOWN), Refresh); });
			Pasting--;
			Refresh = true;
			return true;
		}

		case KEY_CTRLALTPGUP:   case KEY_CTRLALTNUMPAD9:
		case KEY_CTRLALTHOME:   case KEY_CTRLALTNUMPAD7:
		{
			Pasting++;
			auto PrevLine = Lines.end();
			while (m_it_CurLine != Lines.begin() && PrevLine != m_it_CurLine)
			{
				PrevLine = m_it_CurLine;
				ProcessKeyInternal(Manager::Key(KEY_ALTUP), Refresh);
			}
			Pasting--;
			Refresh = true;
			return true;
		}

		case KEY_CTRLALTPGDN:   case KEY_CTRLALTNUMPAD3:
		case KEY_CTRLALTEND:    case KEY_CTRLALTNUMPAD1:
		{
			Pasting++;
			auto PrevLine = Lines.end();
			while (!IsLastLine(m_it_CurLine) && PrevLine != m_it_CurLine)
			{
				PrevLine = m_it_CurLine;
				ProcessKeyInternal(Manager::Key(KEY_ALTDOWN), Refresh);
			}

			Pasting--;
			Refresh = true;
			return true;
		}

		case KEY_CTRLALTBRACKET:       // Вставить сетевое (UNC) путь из левой панели
		case KEY_CTRLALTBACKBRACKET:   // Вставить сетевое (UNC) путь из правой панели
		case KEY_ALTSHIFTBRACKET:      // Вставить сетевое (UNC) путь из активной панели
		case KEY_RALTSHIFTBRACKET:
		case KEY_ALTSHIFTBACKBRACKET:  // Вставить сетевое (UNC) путь из пассивной панели
		case KEY_RALTSHIFTBACKBRACKET:
		case KEY_CTRLBRACKET:          // Вставить путь из левой панели
		case KEY_RCTRLBRACKET:
		case KEY_CTRLBACKBRACKET:      // Вставить путь из правой панели
		case KEY_RCTRLBACKBRACKET:
		case KEY_CTRLSHIFTBRACKET:     // Вставить путь из активной панели
		case KEY_RCTRLSHIFTBRACKET:
		case KEY_CTRLSHIFTBACKBRACKET: // Вставить путь из пассивной панели
		case KEY_RCTRLSHIFTBACKBRACKET:
		case KEY_CTRLSHIFTNUMENTER:
		case KEY_RCTRLSHIFTNUMENTER:
		case KEY_SHIFTNUMENTER:
		case KEY_CTRLSHIFTENTER:
		case KEY_RCTRLSHIFTENTER:
		case KEY_SHIFTENTER:
		{
			if (!m_Flags.Check(FEDITOR_LOCKMODE))
			{
				Pasting++;
				TextChanged(true);

				if (!EdOpt.PersistentBlocks && IsStreamSelection())
				{
					TurnOffMarkingBlock();
					DeleteBlock();
				}

				AddUndoData(undo_type::edit, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), m_it_CurLine.Number(), m_it_CurLine->GetCurPos());
				m_it_CurLine->ProcessKey(Key);
				Change(ECTYPE_CHANGED, m_it_CurLine.Number());
				Pasting--;
				Refresh = true;
			}

			return true;
		}

		case KEY_CTRLQ:
		case KEY_RCTRLQ:
		{
			if (!m_Flags.Check(FEDITOR_LOCKMODE))
			{
				m_Flags.Set(FEDITOR_PROCESSCTRLQ);

				// BUGBUG
				if (const auto HostFileEditor = std::dynamic_pointer_cast<FileEditor>(m_Owner.lock()))
					HostFileEditor->ShowStatus();

				Pasting++;
				TextChanged(true);

				if (!EdOpt.PersistentBlocks && IsStreamSelection())
				{
					TurnOffMarkingBlock();
					DeleteBlock();
				}

				AddUndoData(undo_type::edit, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), m_it_CurLine.Number(), m_it_CurLine->GetCurPos());
				m_it_CurLine->ProcessCtrlQ();
				Change(ECTYPE_CHANGED, m_it_CurLine.Number());
				m_Flags.Clear(FEDITOR_PROCESSCTRLQ);
				Pasting--;
				Refresh = true;
			}

			return true;
		}

		case KEY_OP_SELWORD:
		{
			int OldCurPos=CurPos;
			size_t SBegin, SEnd;
			Pasting++;
			UnmarkBlock();

			// CurLine->TableSet ??? => UseDecodeTable?CurLine->TableSet:nullptr !!!
			if (FindWordInString(m_it_CurLine->GetString(), CurPos, SBegin, SEnd, EdOpt.strWordDiv))
			{
				m_it_CurLine->Select(static_cast<int>(SBegin), static_cast<int>(SEnd));

				if (m_it_CurLine->IsSelection())
				{
					BeginStreamMarking(m_it_CurLine);
					//SelFirst=TRUE;
					//BUGBUG, never used
					SelStart = SBegin;
					SelEnd = SEnd - 1;
				}
			}

			CurPos=OldCurPos; // возвращаем обратно
			Pasting--;
			Refresh = true;
			return true;
		}

		case KEY_OP_PLAINTEXT:
		{
			if (!m_Flags.Check(FEDITOR_LOCKMODE))
			{
				Pasting++;
				TextChanged(true);

				if (!EdOpt.PersistentBlocks && IsAnySelection())
				{
					TurnOffMarkingBlock();
					DeleteBlock();
				}

				//AddUndoData(undo_type::edit,CurLine->GetString(),CurLine->GetEOL(),NumLine,CurLine->GetCurPos());
				Paste(Global->CtrlObject->Macro.GetStringToPrint());
				//if (!EdOpt.PersistentBlocks && IsBlock)
				UnmarkBlock();
				Pasting--;
				Refresh = true;
			}

			return true;
		}

		default:
		{
			{
				if (any_of(LocalKey(), KEY_CTRLDEL, KEY_RCTRLDEL, KEY_CTRLNUMDEL, KEY_RCTRLNUMDEL, KEY_CTRLDECIMAL, KEY_RCTRLDECIMAL, KEY_CTRLT, KEY_RCTRLT) &&
					CurPos >= m_it_CurLine->GetLength())
				{
					/*$ 08.12.2000 skv
					  - CTRL-DEL в начале строки при выделенном блоке и
					    включенном EditorDelRemovesBlocks
					*/
					bool save=EdOpt.DelRemovesBlocks;
					EdOpt.DelRemovesBlocks=false;
					auto ret=ProcessKeyInternal(Manager::Key(KEY_DEL), Refresh);
					EdOpt.DelRemovesBlocks=save;
					return ret;
				}

				if (!Pasting && !EdOpt.PersistentBlocks && IsStreamSelection())
					if (IsCharKey(LocalKey()))
					{
						DeleteBlock();
						/* $ 19.09.2002 SKV
						  Однако надо.
						  Иначе если при наличии выделения набирать
						  текст с шифтом флаги не сбросятся и следующий
						  выделенный блок будет глючный.
						*/
						TurnOffMarkingBlock();
						Refresh = true;
					}

				const auto SkipCheckUndo = any_of(LocalKey(),
					KEY_RIGHT, KEY_NUMPAD6,
					KEY_CTRLLEFT, KEY_CTRLNUMPAD4,
					KEY_RCTRLLEFT, KEY_RCTRLNUMPAD4,
					KEY_CTRLRIGHT, KEY_CTRLNUMPAD6,
					KEY_RCTRLRIGHT, KEY_RCTRLNUMPAD6,
					KEY_HOME, KEY_NUMPAD7,
					KEY_END, KEY_NUMPAD1,
					KEY_CTRLS, KEY_RCTRLS);

				if (m_Flags.Check(FEDITOR_LOCKMODE) && !SkipCheckUndo)
					return true;

				if (any_of(LocalKey(), KEY_HOME, KEY_NUMPAD7))
					m_Flags.Set(FEDITOR_NEWUNDO);

				if (any_of(LocalKey(), KEY_CTRLLEFT, KEY_RCTRLLEFT, KEY_CTRLNUMPAD4, KEY_RCTRLNUMPAD4) && !m_it_CurLine->GetCurPos())
				{
					Pasting++;
					ProcessKeyInternal(Manager::Key(KEY_LEFT), Refresh);
					Pasting--;
					/* $ 24.9.2001 SKV
					  fix бага с ctrl-left в начале строки
					  в блоке с переопределённым плагином фоном.
					*/
					Refresh = true;
					return true;
				}

				if (
					(
						(!EdOpt.CursorBeyondEOL && any_of(LocalKey(), KEY_RIGHT, KEY_NUMPAD6)) ||
						any_of(LocalKey(), KEY_CTRLRIGHT, KEY_RCTRLRIGHT, KEY_CTRLNUMPAD6, KEY_RCTRLNUMPAD6)
					) &&
					m_it_CurLine->GetCurPos()>=m_it_CurLine->GetLength() &&
					!IsLastLine(m_it_CurLine))
				{
					Pasting++;
					ProcessKeyInternal(Manager::Key(KEY_HOME), Refresh);
					if (!EdOpt.CursorBeyondEOL)
						MaxRightPosState.Position = m_it_CurLine->GetCurPos();
					ProcessKeyInternal(Manager::Key(KEY_DOWN), Refresh);
					Pasting--;
					Refresh = true;
					return true;
				}

				const auto& Str = m_it_CurLine->GetString();
				string CmpStr;
				if (!SkipCheckUndo)
				{
					CmpStr = Str;
				}

				if (IsCharKey(LocalKey()) && m_it_CurLine->GetCurPos()>0 && Str.empty())
				{
					auto PrevLine = m_it_CurLine == Lines.begin()? Lines.end() : std::prev(m_it_CurLine);

					while (PrevLine != Lines.end() && !PrevLine->GetLength())
					{
						if (PrevLine != Lines.begin())
						{
							--PrevLine;
						}
						else
						{
							PrevLine = Lines.end();
						}
					}

					if (PrevLine != Lines.end())
					{
						int TabPos=m_it_CurLine->GetTabCurPos();
						m_it_CurLine->SetCurPos(0);
						const auto PrevStr = PrevLine->GetString();

						for (const auto& i: PrevStr)
						{
							if (!std::iswblank(i))
								break;

							int NewTabPos=m_it_CurLine->GetTabCurPos();

							if (NewTabPos==TabPos)
								break;

							if (NewTabPos>TabPos)
							{
								m_it_CurLine->ProcessKey(Manager::Key(KEY_BS));

								while (m_it_CurLine->GetTabCurPos()<TabPos)
									m_it_CurLine->ProcessKey(Manager::Key(' '));

								break;
							}

							if (NewTabPos<TabPos)
								m_it_CurLine->ProcessKey(Manager::Key(i));
						}

						m_it_CurLine->SetTabCurPos(TabPos);
					}
				}

				if (LocalKey() == KEY_OP_XLAT)
				{
					Xlat();
					Refresh = true;
					return true;
				}

				// <comment> - это требуется для корректной работы логики блоков для Ctrl-K
				intptr_t PreSelStart,PreSelEnd;
				m_it_CurLine->GetSelection(PreSelStart,PreSelEnd);
				// </comment>

				//AY: Это что бы при FastShow LeftPos не становился в конец строки.
				m_it_CurLine->SetRightCoord(XX2);

				if (m_it_CurLine->ProcessKey(LocalKey))
				{
					intptr_t CurSelStart, CurSelEnd;

					/* $ 17.09.2002 SKV
					  Если находимся в середине блока,
					  в начале строки, и нажимаем tab, который заменяется
					  на пробелы, выделение съедет. Это фикс.
					*/
					if (LocalKey()==KEY_TAB && GetConvertTabs() && IsStreamSelection() && m_it_AnyBlockStart != m_it_CurLine)
					{
						m_it_CurLine->GetSelection(CurSelStart, CurSelEnd);
						m_it_CurLine->Select(CurSelStart==-1?-1:0,CurSelEnd);
					}

					if (!SkipCheckUndo)
					{
						const auto& NewCmpStr = m_it_CurLine->GetString();

						if (CmpStr != NewCmpStr)
						{
							AddUndoData(undo_type::edit, CmpStr, m_it_CurLine->GetEOL(), m_it_CurLine.Number(), std::min(CurPos, m_it_CurLine->GetCurPos())); // EOL? - CurLine->GetEOL()  GlobalEOL   ""
							Change(ECTYPE_CHANGED, m_it_CurLine.Number());
							TextChanged(true);
						}
					}

					// <Bug 794>
					// обработаем только первую и последнюю строку с блоком
					if (any_of(LocalKey(), KEY_CTRLK, KEY_RCTRLK) && EdOpt.PersistentBlocks)
					{
						if (m_it_CurLine == m_it_AnyBlockStart)
						{
							if (CurPos)
							{
								m_it_CurLine->GetSelection(CurSelStart, CurSelEnd);

								// 1. блок за концом строки (CurPos был ближе к началу, чем CurSelStart)
								if ((CurSelEnd == -1 && PreSelStart > CurPos) || CurSelEnd > CurPos)
									CurSelStart = CurSelEnd = -1; // в этом случае снимаем выделение

								// 2. CurPos внутри блока
								else if (CurSelEnd == -1 && PreSelEnd > CurPos && CurSelStart < CurPos)
									CurSelEnd = PreSelEnd;   // в этом случае усекаем блок

								// 3. блок остался слева от CurPos или выделение нужно снять (см. выше)
								if (CurSelEnd >= CurPos || CurSelStart == -1)
									m_it_CurLine->Select(CurSelStart, CurPos);
							}
							else
							{
								m_it_CurLine->RemoveSelection();
								++m_it_AnyBlockStart;
							}
						}
						else // ЗДЕСЬ ЗАСАДА !!! ЕСЛИ ВЫДЕЛЕННЫЙ БЛОК ДОСТАТОЧНО БОЛЬШОЙ (ПО СТРОКАМ), ТО ЦИКЛ ПЕРЕБОРА... МОЖЕТ ЗАТЯНУТЬ...
						{
							// найдем эту последнюю строку (и последняя ли она)
							auto CurPtrBlock = m_it_AnyBlockStart, CurPtrBlock2 = m_it_AnyBlockStart;

							while (CurPtrBlock != Lines.end())
							{
								CurPtrBlock->GetRealSelection(CurSelStart, CurSelEnd);

								if (CurSelStart == -1)
									break;

								CurPtrBlock2=CurPtrBlock;
								++CurPtrBlock;
							}

							if (m_it_CurLine==CurPtrBlock2)
							{
								if (CurPos)
								{
									m_it_CurLine->GetSelection(CurSelStart, CurSelEnd);
									m_it_CurLine->Select(CurSelStart, CurPos);
								}
								else
								{
									m_it_CurLine->RemoveSelection();
									++CurPtrBlock2;
								}
							}
						}
					}

					// </Bug 794>
					Refresh = true;
					return true;
				}

				if (IsVerticalSelection())
					Refresh = true;
			}
			return false;
		}
	}
}

bool Editor::ProcessKey(const Manager::Key& Key)
{
	bool RefreshMe = false;
	const auto Result = ProcessKeyInternal(Key, RefreshMe);
	if (RefreshMe) Refresh();
	return Result;
}


bool Editor::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	// $ 28.12.2000 VVM - Щелчок мышкой снимает непостоянный блок всегда
	if ((MouseEvent->dwButtonState & 3))
	{
		TurnOffMarkingBlock();

		if ((!EdOpt.PersistentBlocks) && IsAnySelection())
		{
			UnmarkBlock();
			Show();
		}
	}

	if (EdOpt.ShowScrollBar && ScrollBarRequired(ObjHeight(), Lines.size()) && MouseEvent->dwMousePosition.X == m_Where.right && IsMouseButtonEvent(MouseEvent->dwEventFlags))
	{
		if (MouseEvent->dwMousePosition.Y == m_Where.top)
		{
			// Press and hold the [▲] button
			while_mouse_button_pressed([&](DWORD const Button)
			{
				if (Button != FROM_LEFT_1ST_BUTTON_PRESSED)
					return false;

				ProcessKey(Manager::Key(KEY_CTRLUP));
				return true;
			});
		}
		else if (MouseEvent->dwMousePosition.Y == m_Where.bottom)
		{
			// Press and hold the [▼] button
			while_mouse_button_pressed([&](DWORD const Button)
			{
				if (Button != FROM_LEFT_1ST_BUTTON_PRESSED)
					return false;

				ProcessKey(Manager::Key(KEY_CTRLDOWN));
				return true;
			});
		}
		else
		{
			// Drag the thumb
			while (IsMouseButtonPressed() == FROM_LEFT_1ST_BUTTON_PRESSED)
				GoToLineAndShow((Lines.size() - 1) * (IntKeyState.MousePos.y - m_Where.top) / (m_Where.height() - 1));
		}

		return true;
	}

	if (MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED)
	{
		static std::chrono::steady_clock::time_point EditorPrevDoubleClick;
		static point EditorPrevPosition;

		const auto CurrentTime = std::chrono::steady_clock::now();

		if (
			CurrentTime - EditorPrevDoubleClick <= std::chrono::milliseconds(GetDoubleClickTime()) &&
			IsMouseButtonEvent(MouseEvent->dwEventFlags) &&
			EditorPrevPosition == MouseEvent->dwMousePosition
		)
		{
			m_it_CurLine->Select(0, m_it_CurLine->m_Str.size());

			if (m_it_CurLine->IsSelection())
			{
				BeginStreamMarking(m_it_CurLine);
			}

			EditorPrevDoubleClick = {};
			EditorPrevPosition = {};
			Show();
			return true;
		}

		if (MouseEvent->dwEventFlags==DOUBLE_CLICK)
		{
			TurnOffMarkingBlock();

			if (IsAnySelection())
				UnmarkBlock();

			ProcessKey(Manager::Key(KEY_OP_SELWORD));
			EditorPrevDoubleClick = CurrentTime;
			EditorPrevPosition=MouseEvent->dwMousePosition;
			Show();
			return true;
		}
		else
		{
			EditorPrevDoubleClick = {};
			EditorPrevPosition = {};
		}

		Show();
	}

	if (m_it_CurLine->ProcessMouse(MouseEvent))
	{
		// BUGBUG
		if (const auto HostFileEditor = std::dynamic_pointer_cast<FileEditor>(m_Owner.lock()))
			HostFileEditor->ShowStatus();

		Show();
		return true;
	}

	if (!(MouseEvent->dwButtonState & 3))
		return false;

	if (!m_Where.contains(MouseEvent->dwMousePosition))
		return false;

	// BUGBUG, dead code
	// scroll up
	if (MouseEvent->dwMousePosition.Y == m_Where.top - 1)
	{
		while (IsMouseButtonPressed() && IntKeyState.MousePos.y == m_Where.top - 1)
		{
			ProcessKey(Manager::Key(KEY_UP));
			Global->WindowManager->PluginCommit();
		}

		return true;
	}

	// BUGBUG, dead code
	// scroll down
	if (MouseEvent->dwMousePosition.Y == m_Where.bottom + 1)
	{
		while (IsMouseButtonPressed() && IntKeyState.MousePos.y == m_Where.bottom + 1)
		{
			ProcessKey(Manager::Key(KEY_DOWN));
			Global->WindowManager->PluginCommit();
		}

		return true;
	}

	int NewDist = MouseEvent->dwMousePosition.Y - m_Where.top;
	auto NewPtr=m_it_TopScreen;

	while (NewDist-- && !IsLastLine(NewPtr))
		++NewPtr;

	int Dist = CalcDistance(m_it_TopScreen, NewPtr) - CalcDistance(m_it_TopScreen, m_it_CurLine);

	if (Dist>0)
		while (Dist--)
			Down();
	else
		while (Dist++)
			Up();

	m_it_CurLine->ProcessMouse(MouseEvent);
	Show();
	return true;
}


int Editor::CalcDistance(const numbered_iterator& From, const numbered_iterator& To)
{
	return To.Number() - From.Number();
}

struct Editor::InternalEditorSessionBookMark
{
	DWORD Line;
	DWORD Cursor;
	DWORD ScreenLine;
	DWORD LeftPos;
	size_t hash;
};

Editor::numbered_iterator Editor::DeleteString(numbered_iterator DelPtr, bool DeleteLast)
{
	const auto UpdateIterator = [&DelPtr, this](numbered_iterator& What)
	{
		if (What == DelPtr)
		{
			auto Next = std::next(What);
			Next.DecrementNumber();
			What = Next == Lines.end()? --What : Next;
		}
		else if (What != Lines.end())
		{
			if (What.Number() > DelPtr.Number())
			{
				What.DecrementNumber();
			}
		}
	};

	if (m_Flags.Check(FEDITOR_LOCKMODE))
		return DelPtr;

	/* $ 16.12.2000 OT
	   CtrlY на последней строке с выделенным вертикальным блоком не снимал выделение */
	if (IsVerticalSelection() && m_it_CurLine.Number() >= m_it_AnyBlockStart.Number() && m_it_CurLine.Number() < m_it_AnyBlockStart.Number() + VBlockSizeY)
	{
		if (VBlockSizeY)
		{
			--VBlockSizeY;
		}
		if (!VBlockSizeY)
		{
			Unselect();
		}
	}

	TextChanged(true);

	if (IsLastLine(DelPtr) && (!DeleteLast || DelPtr == Lines.begin()))
	{
		AddUndoData(undo_type::edit, DelPtr->GetString(), DelPtr->GetEOL(), DelPtr.Number(), DelPtr->GetCurPos());
		DelPtr->ClearString();
		Change(ECTYPE_CHANGED, DelPtr.Number());
		return DelPtr;
	}

	const auto CurPos = m_it_CurLine->GetTabCurPos();
	UpdateIterator(m_it_CurLine);
	m_it_CurLine->SetTabCurPos(CurPos);
	UpdateIterator(m_it_AnyBlockStart);
	UpdateIterator(m_it_TopScreen);
	UpdateIterator(m_it_LastGetLine);
	UpdateIterator(m_it_MBlockStart);
	UpdateIterator(m_FoundLine);

	if (IsAnySelection() && !m_it_AnyBlockStart->IsSelection())
	{
		Unselect();
	}

	if (IsLastLine(DelPtr))
	{
		std::prev(DelPtr)->SetEOL(eol::none);
	}

	AddUndoData(undo_type::delete_string, DelPtr->GetString(), DelPtr->GetEOL(), DelPtr.Number(), 0);

	for (auto& i: m_SavePos)
	{
		//FIXME: it would be better to add the bookmarks for deleted line to UndoData
		if (i.Line != POS_NONE && DelPtr.Number() < i.Line)
			--i.Line;
	}

	if (!SessionBookmarks.empty())
	{
		for (auto i = SessionBookmarks.begin(), end = SessionBookmarks.end(); i != end;)
		{
			const auto next = std::next(i);
			if (DelPtr.Number() < static_cast<int>(i->Line))
			{
				i->Line--;
			}
			else
			{
				if (DelPtr.Number() == static_cast<int>(i->Line))
				{
					MoveSessionBookmarkToUndoList(i);
				}
			}
			i = next;
		}
	}

	m_AutoDeletedColors.erase(std::to_address(DelPtr));
	DeleteColor(DelPtr, [](ColorItem const&){ return true; });

	const auto Result = numbered_iterator(Lines.erase(DelPtr), DelPtr.Number());

	Change(ECTYPE_DELETED, DelPtr.Number());

	return Result;
}


void Editor::InsertString()
{
	if (m_Flags.Check(FEDITOR_LOCKMODE))
		return;

	/*$ 10.08.2000 skv
	  There is only one return - if new will fail.
	  In this case things are realy bad.
	  Move TextChanged to the end of functions
	  AFTER all modifications are made.
	*/
//  TextChanged(true);
	auto SrcIndent = Lines.end();
	intptr_t SelStart,SelEnd;
	const auto NextLine = std::next(m_it_CurLine);
	const auto NewString = InsertString({}, NextLine);

	if (NewString == Lines.end())
		return;

	Change(ECTYPE_ADDED, m_it_CurLine.Number() + 1);
	//NewString->SetTables(UseDecodeTable ? &TableSet:nullptr); // ??

	size_t CurPos=m_it_CurLine->GetCurPos();
	m_it_CurLine->GetSelection(SelStart,SelEnd);

	for (auto& i: m_SavePos)
	{
		if (i.Line != POS_NONE && (m_it_CurLine.Number() < i.Line || (m_it_CurLine.Number() == i.Line && !CurPos)))
			++i.Line;
	}

	if (!SessionBookmarks.empty())
	{
		for (auto& i: SessionBookmarks)
		{
			if (m_it_CurLine.Number() < static_cast<int>(i.Line) || (m_it_CurLine.Number() == static_cast<int>(i.Line) && !CurPos))
				i.Line++;
		}
	}

	int IndentPos=0;

	if (EdOpt.AutoIndent && !Pasting)
	{
		auto PrevLine = m_it_CurLine;

		while (PrevLine != Lines.end())
		{
			const auto& Str = PrevLine->GetString();

			const auto It = std::ranges::find_if_not(Str, std::iswblank);
			if (It != Str.cend())
			{
				PrevLine->SetCurPos(static_cast<int>(It - Str.cbegin()));
				IndentPos = PrevLine->GetTabCurPos();
				SrcIndent = PrevLine;
				break;
			}

			if (PrevLine != Lines.begin())
				--PrevLine;
			else
				PrevLine = EndIterator();
		}
	}

	int SpaceOnly=TRUE;

	const auto LineEol = m_it_CurLine->GetEOL();

	if (CurPos < static_cast<size_t>(m_it_CurLine->GetLength()))
	{
		const auto& CurLineStr = m_it_CurLine->GetString();

		if (IndentPos > 0 && !std::all_of(CurLineStr.cbegin(), CurLineStr.cbegin() + CurPos, std::iswblank))
		{
			SpaceOnly = FALSE;
		}

		NewString->SetString(string_view(CurLineStr).substr(CurPos));

		const auto NewLineEmpty = std::all_of(CurLineStr.cbegin() + CurPos, CurLineStr.cend(), std::iswblank);

		AddUndoData(undo_type::begin);
		AddUndoData(undo_type::edit, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), m_it_CurLine.Number(), m_it_CurLine->GetCurPos());
		AddUndoData(undo_type::insert_string, {}, m_it_CurLine->GetEOL(), m_it_CurLine.Number() + 1, 0);
		AddUndoData(undo_type::end);

		string NewCurLineStr(CurLineStr, 0, CurPos);

		if (EdOpt.AutoIndent && NewLineEmpty)
		{
			inplace::trim_right(NewCurLineStr);
		}

		m_it_CurLine->SetString(NewCurLineStr, true);

		if (m_FoundLine == m_it_CurLine && static_cast<size_t>(m_FoundPos) >= CurPos)
		{
			++m_FoundLine;
			m_FoundPos -= static_cast<int>(CurPos);
		}

		Change(ECTYPE_CHANGED, m_it_CurLine.Number());
	}
	else
	{
		NewString->ClearString();
		AddUndoData(undo_type::insert_string, {}, m_it_CurLine->GetEOL(), m_it_CurLine.Number() + 1, 0);
	}

	if (LineEol != eol::none)
	{
		m_it_CurLine->SetEOL(LineEol);
	}
	else
	{
		m_it_CurLine->SetEOL(GlobalEOL != eol::none? GlobalEOL : GetDefaultEOL());
		NewString->SetEOL(LineEol);
	}

	Change(ECTYPE_CHANGED, m_it_CurLine.Number() + 1);

	if (IsVerticalSelection() && m_it_CurLine.Number() >= m_it_AnyBlockStart.Number() && m_it_CurLine.Number() < m_it_AnyBlockStart.Number() + VBlockSizeY)
	{
		VBlockSizeY++;
	}

	if (SelStart!=-1 && (SelEnd==-1 || CurPos < static_cast<size_t>(SelEnd)))
	{
		if (static_cast<intptr_t>(CurPos) >= SelStart)
		{
			m_it_CurLine->Select(SelStart,-1);
			NewString->Select(0, SelEnd == -1? -1 : static_cast<int>(SelEnd - CurPos));
		}
		else
		{
			m_it_CurLine->RemoveSelection();
			NewString->Select(static_cast<int>(SelStart - CurPos), SelEnd == -1? -1 : static_cast<int>(SelEnd - CurPos));
			m_it_AnyBlockStart = numbered_iterator(NewString, m_it_AnyBlockStart.Number() + 1);
		}
	}

	NewString->SetEOL(LineEol);
	m_it_CurLine->SetCurPos(0);

	Down();

	if (IndentPos>0)
	{
		const size_t OrgIndentPos = IndentPos;
		ShowEditor();

		if (SpaceOnly)
		{
			int Decrement=0;
			const auto& Str = m_it_CurLine->GetString();
			const auto MaxI = std::min(static_cast<size_t>(IndentPos), Str.size());
			for (size_t I = 0; I != MaxI && std::iswblank(Str[I]); ++I)
			{
				if (Str[I]==L' ')
					Decrement++;
				else
				{
					const auto TabPos = m_it_CurLine->RealPosToVisual(static_cast<int>(I));
					Decrement+=EdOpt.TabSize - (TabPos % EdOpt.TabSize);
				}
			}

			IndentPos-=Decrement;
		}

		if (IndentPos>0)
		{
			if (m_it_CurLine->GetLength() || !EdOpt.CursorBeyondEOL)
			{
				m_it_CurLine->ProcessKey(Manager::Key(KEY_HOME));
				const auto SaveOvertypeMode = m_it_CurLine->GetOvertypeMode();
				m_it_CurLine->SetOvertypeMode(false);

				if (SrcIndent != Lines.end())
				{
					const auto& PrevStr = SrcIndent->GetString();
					for (size_t I = 0; m_it_CurLine->GetTabCurPos() < IndentPos; ++I)
					{
						if (I < PrevStr.size() && std::iswblank(PrevStr[I]))
						{
							m_it_CurLine->ProcessKey(Manager::Key(PrevStr[I]));
						}
						else
						{
							m_it_CurLine->ProcessKey(Manager::Key(KEY_SPACE));
						}
					}
				}
				else
				{
					while (m_it_CurLine->GetTabCurPos() < IndentPos)
					{
						m_it_CurLine->ProcessKey(Manager::Key(KEY_SPACE));
					}
				}

				while (m_it_CurLine->GetTabCurPos()>IndentPos)
					m_it_CurLine->ProcessKey(Manager::Key(KEY_BS));

				m_it_CurLine->SetOvertypeMode(SaveOvertypeMode);
				Change(ECTYPE_CHANGED, m_it_CurLine.Number());
			}

			m_it_CurLine->SetTabCurPos(IndentPos);
		}

		const auto& Str = m_it_CurLine->GetString();
		CurPos=m_it_CurLine->GetCurPos();

		if (SpaceOnly)
		{
			const auto SpaceIterator = std::ranges::find_if_not(Str, std::iswblank);
			const auto NewPos = std::min<size_t>(SpaceIterator - Str.cbegin(), OrgIndentPos);
			if (NewPos > CurPos)
				m_it_CurLine->SetCurPos(static_cast<int>(NewPos));
		}
	}

	TextChanged(true);
}

void Editor::UpdateIteratorAndKeepPos(numbered_iterator& Iter, const auto& Func)
{
	const auto CurPos = Iter->GetTabCurPos();
	const auto LeftPos = Iter->GetLeftPos();
	Func(Iter);
	Iter->SetLeftPos(LeftPos);
	Iter->SetTabCurPos(CurPos);
}

void Editor::Down()
{
	//TODO: "Свертка" - если учесть "!Flags.Check(FSCROBJ_VISIBLE)", то крутить надо до следующей видимой строки
	if (IsLastLine(m_it_CurLine))
		return;

	const auto Y = std::distance(m_it_TopScreen, m_it_CurLine);

	if (Y >= m_Where.height() - 1)
		++m_it_TopScreen;

	UpdateIteratorAndKeepPos(m_it_CurLine, [](numbered_iterator& Iter) { ++Iter; });
}


void Editor::ScrollDown()
{
	//TODO: "Свертка" - если учесть "!Flags.Check(FSCROBJ_VISIBLE)", то крутить надо до следующей видимой строки
	if (IsLastLine(m_it_CurLine) || IsLastLine(m_it_TopScreen))
		return;

	if (!EdOpt.AllowEmptySpaceAfterEof && CalcDistance(m_it_TopScreen, EndIterator()) < ObjHeight())
	{
		Down();
		return;
	}

	++m_it_TopScreen;

	UpdateIteratorAndKeepPos(m_it_CurLine, [](numbered_iterator& Iter) { ++Iter; });
}

void Editor::Up()
{
	//TODO: "Свертка" - если учесть "!Flags.Check(FSCROBJ_VISIBLE)", то крутить надо до следующей видимой строки
	if (m_it_CurLine == Lines.begin())
		return;

	if (m_it_CurLine==m_it_TopScreen)
		--m_it_TopScreen;

	UpdateIteratorAndKeepPos(m_it_CurLine, [](numbered_iterator& Iter) { --Iter; });
}


void Editor::ScrollUp()
{
	//TODO: "Свертка" - если учесть "!Flags.Check(FSCROBJ_VISIBLE)", то крутить надо до следующей видимой строки
	if (m_it_CurLine == Lines.begin())
		return;

	if (m_it_TopScreen == Lines.begin())
	{
		Up();
		return;
	}

	--m_it_TopScreen;

	UpdateIteratorAndKeepPos(m_it_CurLine, [](numbered_iterator& Iter) { --Iter; });
}

struct FindCoord
{
	int Line;
	int Pos;
	int SearchLen;
};

class undo_block
{
public:
	explicit undo_block(Editor* Owner): m_Owner(Owner) { m_Owner->AddUndoData(Editor::undo_type::begin); }
	~undo_block() { m_Owner->AddUndoData(Editor::undo_type::end); }

private:
	Editor* m_Owner;
};

Editor::SearchReplaceDisposition Editor::ShowSearchReplaceDialog(const bool ReplaceMode)
{
	const auto Picker = [this](bool PickSelection)
	{
		if (PickSelection)
		{
			if (IsAnySelection())
			{
				if (IsStreamSelection())
				{
					intptr_t StartSel, EndSel;
					m_it_AnyBlockStart->GetSelection(StartSel, EndSel);
					if (StartSel != -1)
					{
						return m_it_AnyBlockStart->GetString().substr(StartSel, EndSel == -1 ? string::npos : EndSel - StartSel);
					}
				}
				else
				{
					const size_t TBlockX = m_it_AnyBlockStart->VisualPosToReal(VBlockX);
					const size_t TBlockSizeX = m_it_AnyBlockStart->VisualPosToReal(VBlockX + VBlockSizeX) - TBlockX;
					const auto& Str = m_it_AnyBlockStart->GetString();
					if (TBlockX <= Str.size())
					{
						const auto CopySize = std::min(Str.size() - TBlockX, TBlockSizeX);
						return Str.substr(TBlockX, CopySize);
					}
				}
			}
			else
			{
				return m_it_CurLine->GetString();
			}
		}
		else
		{
			size_t PickBegin, PickEnd;
			const auto& Str = m_it_CurLine->GetString();
			if (FindWordInString(Str, m_it_CurLine->GetCurPos(), PickBegin, PickEnd, EdOpt.strWordDiv))
			{
				return Str.substr(PickBegin, PickEnd - PickBegin);
			}
		}
		return string{};
	};

	switch (GetSearchReplaceString(
		{
			.ReplaceMode = ReplaceMode,
			.ShowButtonsPrevNext = true,
			.ShowButtonAll = !ReplaceMode,
		},
		m_SearchDlgParams,
		L"SearchText"sv,
		L"ReplaceText"sv,
		GetCodePage(),
		L"EditorSearch"sv,
		ReplaceMode? &EditorReplaceId : &EditorSearchId,
		Picker))
	{
		case SearchReplaceDlgResult::Cancel:
			return SearchReplaceDisposition::Cancel;

		case SearchReplaceDlgResult::Prev:
			IsReplaceMode = ReplaceMode;
			return SearchReplaceDisposition::Prev;

		case SearchReplaceDlgResult::Next:
			IsReplaceMode = ReplaceMode;
			return SearchReplaceDisposition::Next;

		case SearchReplaceDlgResult::All:
			IsReplaceMode = ReplaceMode;
			return SearchReplaceDisposition::All;

		case SearchReplaceDlgResult::Ok:
		default:
			std::unreachable();
	}
}

int Editor::CalculateSearchStartPosition(const bool Continue, const bool Backward, const bool Regex) const
{
	// We say that a search is a Search-Next, if it is initiated with Shift+F7 or Alt+F7
	// and immediately follows the previous search. In this case, we need to punt the search start
	// position by at least one character to ensure that the search makes progress as the user
	// repeatedly presses Shift+F7 or Alt+F7. In all other cases, start the search from the cursor.
	// To detect Search-Next, we compare the current cursor position with the position
	// where the cursor would be left by the previous search.

	const auto CurLineCurPos{ m_it_CurLine->GetCurPos() };

	// If it is not Shift+F7 or Alt+F7, or the cursor is nowhere near the last found match,
	// or there is no last found match, it is not Search-Next.
	if (!Continue || m_FoundLine != m_it_CurLine || m_FoundPos < 0)
		return CurLineCurPos;

	assert(Regex ? m_FoundSize >= 0 : m_FoundSize > 0);

	// Anchor is the begin or the end of the last found match (depending on cursor-at-the-end mode)
	// where we expect to find cursor if this is Search-Next.
	const auto Anchor{ m_FoundPos + EdOpt.SearchCursorAtEnd * m_FoundSize };

	// If the cursor is not at the anchor, it is not Search-Next.
	if (CurLineCurPos != Anchor)
		return CurLineCurPos;

	// Now we are handling Search-Next. Cursor position does not matter anymore.
	return CalculateSearchNextPositionInTheLine(Backward, Regex);
}

int Editor::CalculateSearchNextPositionInTheLine(const bool Backward, const bool Regex) const
{
	if (Regex)
	{
		// 2023-03-25 MZK: Simulating pre https://github.com/FarGroup/FarManager/pull/651 behavior.
		// See also the discussion in the PR above about unifying plain text and regex search.
		// Also fixed search forward in cursor-at-the-end mode, see https://github.com/FarGroup/FarManager/issues/660.
		// Now search continues from the next character.
		// Note: Regex search backwards steps back one character.
		return m_FoundPos + (Backward ? 0 : 1);
	}
	else
	{
		// To find the next nearest match, include the entire last found match (minus one character)
		// into the search range. Drop one character to ensure search progress.
		return m_FoundPos + (Backward ? m_FoundSize - 1 : 1);
	}
}

void Editor::DoSearchReplace(const SearchReplaceDisposition Disposition)
{
	if (Disposition == SearchReplaceDisposition::Cancel || m_SearchDlgParams.SearchStr.empty())
		return;

	IsReplaceAll = false;

	const auto Backward{ Disposition == SearchReplaceDisposition::Prev || Disposition == SearchReplaceDisposition::ContinueBackward };
	const auto FindAll{ Disposition == SearchReplaceDisposition::All };
	const auto Continue{ Disposition == SearchReplaceDisposition::ContinueBackward || Disposition == SearchReplaceDisposition::ContinueForward };

	bool MatchFound{}, UserBreak{};
	std::optional<undo_block> UndoBlock;
	string QuotedStr;
	const auto FindAllList = VMenu2::create({}, {});
	size_t AllRefLines{};

	{
		HideCursor();

		auto CurPos = FindAll ? 0 : CalculateSearchStartPosition(Continue, Backward, m_SearchDlgParams.Regex.value());

		auto CurPtr = FindAll ? FirstLine() : m_it_CurLine, TmpPtr = CurPtr;

		regex_match Match;
		named_regex_match NamedMatch;
		RegExp re;

		if (m_SearchDlgParams.Regex.value())
		{
			// Q: что важнее: опция диалога или опция RegExp`а?
			try
			{
				re.Compile(
					m_SearchDlgParams.SearchStr,
					(m_SearchDlgParams.SearchStr.starts_with(L'/')? OP_PERLSTYLE : 0) | OP_OPTIMIZE | (m_SearchDlgParams.CaseSensitive.value()? 0 : OP_IGNORECASE));
			}
			catch (regex_exception const& e)
			{
				ReCompileErrorMessage(e, m_SearchDlgParams.SearchStr);
				return; // Broken regex cannot be found. Do as if the search string was not found. Do NOT restore IsReplaceAll.
			}
		}

		QuotedStr = quote_unconditional(m_SearchDlgParams.SearchStr);

		searchers Searchers;
		const auto& Searcher = init_searcher(Searchers, m_SearchDlgParams.CaseSensitive.value(), m_SearchDlgParams.Fuzzy.value(), m_SearchDlgParams.SearchStr);

		const time_check TimeCheck;
		std::optional<single_progress> Progress;
		const auto StartLine = CurPtr.Number();
		SCOPED_ACTION(taskbar::indeterminate);
		SCOPED_ACTION(wakeful);
		int LastCheckedLine = -1;

		while (CurPtr != Lines.end())
		{
			if (TimeCheck)
			{
				if (CheckForEscAndConfirmAbort())
				{
					UserBreak = true;
					break;
				}

				if (!Progress)
					Progress.emplace(msg(lng::MSearchReplaceSearchTitle), far::vformat(msg(lng::MEditSearchingFor), QuotedStr), 0);

				HideCursor();
				const auto Total = FindAll? Lines.size() : Backward ? StartLine : Lines.size() - StartLine;
				const auto Current = std::abs(CurPtr.Number() - StartLine);
				Progress->update(ToPercent(Current, Total));
				taskbar::set_value(Current,Total);
			}

			// In case of regex or preserve style, SearchAndReplaceString returns the actual replacement string
			// in its ReplaceStr parameter. To preserve the user input, we use the local copy for replacement.
			auto strReplaceStrCurrent = IsReplaceMode ? m_SearchDlgParams.ReplaceStr.value() : string{};

			int SearchLength;
			if (SearchAndReplaceString(
				CurPtr->GetString(),
				m_SearchDlgParams.SearchStr,
				Searcher,
				re,
				Match,
				&NamedMatch,
				strReplaceStrCurrent,
				CurPos,
				{
					.CaseSensitive = m_SearchDlgParams.CaseSensitive.value(),
					.WholeWords = m_SearchDlgParams.WholeWords.value(),
					.Reverse = Backward,
					.Regex = m_SearchDlgParams.Regex.value(),
					.PreserveStyle = m_SearchDlgParams.PreserveStyle.value()
				},
				SearchLength,
				GetWordDiv()
			))
			{
				MatchFound = true;

				m_FoundLine = CurPtr;
				m_FoundPos = CurPos;
				m_FoundSize = SearchLength;

				if(FindAll)
				{
					constexpr int service_len{ 12 };
					const auto Location{ far::format(L"{}:{}"sv, m_FoundLine.Number() + 1, m_FoundPos + 1) };
					MenuItemEx Item{ far::format(L"{:{}}{}{}"sv, Location, service_len, BoxSymbols[BS_V1], m_FoundLine->GetString()) };
					Item.Annotations.emplace_back(m_FoundPos + service_len + 1, m_FoundSize);
					Item.ComplexUserData = FindCoord{ m_FoundLine.Number(), m_FoundPos, m_FoundSize };
					FindAllList->AddItem(Item);

					if (m_FoundLine.Number() != LastCheckedLine)
					{
						LastCheckedLine = m_FoundLine.Number();
						++AllRefLines;
					}
					CurPos = CalculateSearchNextPositionInTheLine(/* Backward */ false, m_SearchDlgParams.Regex.value());
				}
				else
				{
					// BUGBUG Too much in editor depends on it
					CurPtr->SetCurPos(CurPos);

					if (!EdOpt.PersistentBlocks)
						UnmarkBlock();

					if (EdOpt.SearchSelFound && !IsReplaceMode)
					{
						Pasting++;
						UnmarkBlock();
						BeginStreamMarking(CurPtr);
						CurPtr->Select(m_FoundPos, m_FoundPos + m_FoundSize);
						Pasting--;
					}

					bool Skip=false, ZeroLength=false;

					// Отступим на четверть и проверим на перекрытие диалогом замены
					int FromTop=(ScrY-2)/4;
					if (FromTop<0 || FromTop>=((ScrY-5)/2-2))
						FromTop=0;

					TmpPtr=m_it_CurLine=CurPtr;

					for ([[maybe_unused]] const auto i: std::views::iota(0, FromTop))
					{
						if (TmpPtr != Lines.begin())
							--TmpPtr;
						else
							break;
					}

					m_it_TopScreen=TmpPtr;

					const auto TabCurPos = CurPtr->GetTabCurPos();

					if (TabCurPos + SearchLength + 8 > CurPtr->GetLeftPos() + ObjWidth())
						CurPtr->SetLeftPos(TabCurPos + SearchLength + 8 - ObjWidth());

					if (!IsReplaceMode)
					{
						CurPtr->SetCurPos(m_FoundPos + (EdOpt.SearchCursorAtEnd? SearchLength : 0));
						break;
					}
					else
					{
						auto MsgCode = message_result::first_button;

						if (!IsReplaceAll)
						{
							ColorItem newcol{};
							newcol.StartPos=m_FoundPos;
							newcol.EndPos=m_FoundPos + m_FoundSize - 1;
							newcol.SetColor(SelColor);
							newcol.SetOwner(FarUuid);
							newcol.Priority=EDITOR_COLOR_SELECTION_PRIORITY;
							AddColor(CurPtr, newcol);

							if (!SearchLength && strReplaceStrCurrent.empty())
								ZeroLength = true;

							Progress.reset();

							MsgCode = Message(0,
								msg(lng::MSearchReplaceReplaceTitle),
								{
									msg(lng::MEditAskReplace),
									quote_unconditional(CurPtr->GetString().substr(CurPos, SearchLength)),
									msg(lng::MEditAskReplaceWith),
									quote_unconditional(strReplaceStrCurrent)
								},
								{ lng::MEditReplace, lng::MEditReplaceAll, lng::MEditSkip, lng::MEditCancel },
								{},
								&EditorConfirmReplaceId);

							DeleteColor(CurPtr, [&](const ColorItem& Item) { return newcol.StartPos == Item.StartPos && newcol.GetOwner() == Item.GetOwner();});

							if (MsgCode == message_result::second_button)
								IsReplaceAll = true;

							if (MsgCode == message_result::third_button)
								Skip = true;

							if (MsgCode == message_result::cancelled || MsgCode == message_result::fourth_button)
							{
								CurPtr->SetCurPos(m_FoundPos + (EdOpt.SearchCursorAtEnd? SearchLength : 0));
								UserBreak = true;
								break;
							}
						}

						if (IsReplaceAll)
							UndoBlock.emplace(this);

						if (MsgCode == message_result::first_button || MsgCode == message_result::second_button)
						{
							Pasting++;
							AddUndoData(undo_type::begin);

							// If Replace string doesn't contain control symbols (tab and return),
							// processed with fast method, otherwise use improved old one.
							//
							if (strReplaceStrCurrent.find_first_of(L"\t\r"sv) != string::npos)
							{
								int SaveOvertypeMode=m_Flags.Check(FEDITOR_OVERTYPE);
								m_Flags.Set(FEDITOR_OVERTYPE);
								m_it_CurLine->SetOvertypeMode(true);

								int I=0;
								auto RefreshMe = false;

								for (; SearchLength && I<static_cast<int>(strReplaceStrCurrent.size()); ++I, --SearchLength)
								{
									const auto Ch = strReplaceStrCurrent[I];

									if (Ch==KEY_TAB)
									{
										m_Flags.Clear(FEDITOR_OVERTYPE);
										m_it_CurLine->SetOvertypeMode(false);
										ProcessKeyInternal(Manager::Key(KEY_DEL), RefreshMe);
										ProcessKeyInternal(Manager::Key(KEY_TAB), RefreshMe);
										m_Flags.Set(FEDITOR_OVERTYPE);
										m_it_CurLine->SetOvertypeMode(true);
										continue;
									}

									/* $ 24.05.2002 SKV
									  Если реплэйсим на Enter, то overtype не спасёт.
									  Нужно сначала удалить то, что заменяем.
									*/
									if (Ch==L'\r')
									{
										ProcessKeyInternal(Manager::Key(KEY_DEL), RefreshMe);

										// ProcessKeyInternal('\r') changes m_it_CurLine!
										m_Flags.Clear(FEDITOR_OVERTYPE);
										m_it_CurLine->SetOvertypeMode(false);
										ProcessKeyInternal(Manager::Key(Ch), RefreshMe);
										m_Flags.Set(FEDITOR_OVERTYPE);
										m_it_CurLine->SetOvertypeMode(true);
										CurPtr = m_it_CurLine;
									}
									else if (Ch!=KEY_BS)
										ProcessKeyInternal(Manager::Key(Ch), RefreshMe);
								}

								bool NeedUpdateCurPtr = false;

								if (!SearchLength)
								{
									m_Flags.Clear(FEDITOR_OVERTYPE);
									m_it_CurLine->SetOvertypeMode(false);

									for (; I<static_cast<int>(strReplaceStrCurrent.size()); I++)
									{
										const auto Ch = strReplaceStrCurrent[I];
										NeedUpdateCurPtr = Ch == KEY_ENTER;
										if (Ch!=KEY_BS)
											ProcessKeyInternal(Manager::Key(Ch), RefreshMe);
									}
								}
								else
								{
									for (; SearchLength; SearchLength--)
									{
										ProcessKeyInternal(Manager::Key(KEY_DEL), RefreshMe);
									}
								}

								if (NeedUpdateCurPtr)
								{
									CurPtr = m_it_CurLine;
								}

								m_Flags.Change(FEDITOR_OVERTYPE,SaveOvertypeMode!=0);
							}
							else
							{
								/* Fast method */
								const auto& Str = CurPtr->GetString();
								int LocalCurPos = CurPtr->GetCurPos();
								const auto NewStr = concat(string_view(Str).substr(0, CurPos), strReplaceStrCurrent, string_view(Str).substr(CurPos + SearchLength), CurPtr->GetEOL().str());
								AddUndoData(undo_type::edit, CurPtr->GetString(), CurPtr->GetEOL(), CurPtr.Number(), CurPos);
								CurPtr->SetString(NewStr, true);
								CurPtr->SetCurPos(CurPos + static_cast<int>(strReplaceStrCurrent.size()));

								if (EdOpt.SearchSelFound && !IsReplaceMode)
								{
									UnmarkBlock();
									BeginStreamMarking(CurPtr);
									CurPtr->Select(LocalCurPos, LocalCurPos + static_cast<int>(strReplaceStrCurrent.size()));
								}

								Change(ECTYPE_CHANGED, CurPtr.Number());
								TextChanged(true);
							}

							AddUndoData(undo_type::end);
							Pasting--;
						}
					}

					CurPos = m_it_CurLine->GetCurPos();
					if ((Skip || ZeroLength) && !Backward)
					{
						CurPos++;
					}
					if (!(Skip || ZeroLength) && Backward)
					{
						(m_it_CurLine = CurPtr = m_FoundLine)->SetCurPos(CurPos = m_FoundPos);
					}
				}
			}
			else
			{
				if (Backward)
				{
					if (CurPtr == Lines.begin())
					{
						CurPtr = EndIterator();
						break;
					}
					else
					{
						--CurPtr;
					}

					CurPos=CurPtr->GetLength();
				}
				else
				{
					CurPos=0;
					++CurPtr;
				}
			}
		}
	}
	Show();

	if(FindAll && MatchFound)
	{
		const auto MenuY1 = std::max(0, ScrY - 20);
		const auto MenuY2 = std::min(ScrY, MenuY1 + std::min(static_cast<int>(FindAllList->size()), 10) + 2);
		FindAllList->SetMenuFlags(VMENU_WRAPMODE | VMENU_SHOWAMPERSAND | VMENU_ENABLEALIGNANNOTATIONS);
		FindAllList->SetPosition({ -1, MenuY1, 0, MenuY2 });
		FindAllList->SetTitle(far::vformat(msg(lng::MEditSearchStatistics), FindAllList->size(), AllRefLines));
		FindAllList->SetBottomTitle(KeysToLocalizedText(KEY_CTRLENTER, KEY_F5, KEY_ADD, KEY_CTRLUP, KEY_CTRLDOWN));
		FindAllList->SetHelp(L"FindAllMenu"sv);
		FindAllList->SetId(EditorFindAllListId);

		bool MenuZoomed = false;

		const auto ExitCode = FindAllList->Run([&](const Manager::Key& RawKey)
		{
			const auto Key=RawKey();
			const auto SelectedPos = FindAllList->GetSelectPos();
			int KeyProcessed = 1;

			switch (Key)
			{
				case KEY_ADD:
					AddSessionBookmark();
					break;

				case KEY_CTRLENTER:
				case KEY_RCTRLENTER:
				// TODO: Need to handle mouse events with ProcessMouse(). This implementation is acceptable,
				// but a click on the border jumps to the currently selected item without changing the item.
				// Normally, mouse click on the border frame does nothing. Mouse click on the border
				// outside of the frame starts dragging the dialog.
				case KEY_CTRL|KEY_MSLCLICK:
				case KEY_RCTRL|KEY_MSLCLICK:
					{
						if (SelectedPos == -1)
							break;

						const auto& coord = *FindAllList->GetComplexUserDataPtr<FindCoord>(SelectedPos);
						GoToLine(coord.Line);
						m_it_CurLine->SetCurPos(coord.Pos);
						if (EdOpt.SearchSelFound)
						{
							Pasting++;
							// if (!EdOpt.PersistentBlocks)
							UnmarkBlock();
							BeginStreamMarking(m_it_CurLine);
							m_it_CurLine->Select(coord.Pos, coord.Pos + coord.SearchLen);
							Pasting--;
						}
						if (EdOpt.SearchCursorAtEnd)
						{
							m_it_CurLine->SetCurPos(coord.Pos + coord.SearchLen);
						}
						Refresh();
					}
					break;

				case KEY_CTRLUP: case KEY_RCTRLUP:
				case KEY_CTRLDOWN: case KEY_RCTRLDOWN:
					{
						bool RefreshMeLocal = false;
						KeyProcessed = ProcessKeyInternal(Manager::Key(Key), RefreshMeLocal);
						if (RefreshMeLocal)
						{
							Refresh();
						}
					}
					break;

				case KEY_F5:
					MenuZoomed=!MenuZoomed;
					if(MenuZoomed)
					{
						FindAllList->SetPosition({ -1, -1, 0, 0 });
					}
					else
					{
						FindAllList->SetPosition({ -1, MenuY1, 0, MenuY2 });
					}
					break;

				default:
					if ((Key>=KEY_CTRL0 && Key<=KEY_CTRL9) || (Key>=KEY_RCTRL0 && Key<=KEY_RCTRL9) ||
					   (Key>=KEY_CTRLSHIFT0 && Key<=KEY_CTRLSHIFT9) || (Key>=KEY_RCTRLSHIFT0 && Key<=KEY_RCTRLSHIFT9))
					{
						bool RefreshMeLocal = false;
						KeyProcessed = ProcessKeyInternal(Manager::Key(Key), RefreshMeLocal);
						if (RefreshMeLocal)
						{
							Refresh();
						}
					}
					else
					{
						KeyProcessed = 0;
					}
					break;
			}
			return KeyProcessed;
		});

		if(ExitCode >= 0)
		{
			const auto& coord = *FindAllList->GetComplexUserDataPtr<FindCoord>(ExitCode);
			GoToLine(coord.Line);
			m_it_CurLine->SetCurPos(coord.Pos);
			if (EdOpt.SearchSelFound)
			{
				Pasting++;
				UnmarkBlock();
				BeginStreamMarking(m_it_CurLine);
				m_it_CurLine->Select(coord.Pos, coord.Pos + coord.SearchLen);
				Pasting--;
			}
			if (EdOpt.SearchCursorAtEnd)
			{
				m_it_CurLine->SetCurPos(coord.Pos + coord.SearchLen);
			}
			Show();
		}
	}

	if (!MatchFound && !UserBreak)
		Message(MSG_WARNING,
			msg(lng::MSearchReplaceSearchTitle),
			{
				msg(lng::MEditNotFound),
				QuotedStr
			},
			{ lng::MOk });
}

void Editor::PasteFromClipboard()
{
	if (m_Flags.Check(FEDITOR_LOCKMODE))
		return;

	const clipboard_accessor Clip;

	if (Clip->Open())
	{
		string data;
		if (Clip->GetVText(data))
		{
			VPaste(data);
		}
		else if (Clip->GetText(data))
		{
			Paste(data);
		}
	}
}

void Editor::Paste(string_view const Data)
{
		if (m_Flags.Check(FEDITOR_LOCKMODE))
			return;

		AddUndoData(undo_type::begin);

		if (!EdOpt.PersistentBlocks && !IsVerticalSelection())
			DeleteBlock();

		m_Flags.Set(FEDITOR_NEWUNDO);
		TextChanged(true);
		const auto SaveOvertype = m_Flags.Check(FEDITOR_OVERTYPE);
		UnmarkBlock();
		Pasting++;
		if (m_Flags.Check(FEDITOR_OVERTYPE))
		{
			m_Flags.Clear(FEDITOR_OVERTYPE);
			m_it_CurLine->SetOvertypeMode(false);
		}

		BeginStreamMarking(m_it_CurLine);

		/* $ 19.05.2001 IS
		   Решение проблемы непрошеной конвертации табуляции (которая должна быть
		   добавлена в начало строки при автоотступе) в пробелы.
		*/
		int StartPos=m_it_CurLine->GetCurPos();
		const auto oldAutoIndent = EdOpt.AutoIndent;

		auto keep_eol = eol::none;
		if (EdOpt.KeepEOL)
		{
			auto line = m_it_CurLine;
			if (line != Lines.begin())
				--line;

			keep_eol = line->GetEOL();
		}

		bool RefreshMe = false;
		for (size_t i = 0, size = Data.size(); i != size; )
		{
			if (IsEol(Data[i]))
			{
				m_it_CurLine->Select(StartPos,-1);
				StartPos=0;
				EdOpt.AutoIndent = false;
				const auto PrevLine = m_it_CurLine;
				ProcessKeyInternal(Manager::Key(KEY_ENTER), RefreshMe);

				auto Eol = eol::unix;
				if (Data[i] == L'\r' && i + 1 != size)
				{
					Eol = eol::mac;
					if (Data[i + 1] == L'\n')
						Eol = eol::win;
					else if (Data[i + 1] == L'\r' && i + 2 != size && Data[i + 2] == L'\n')
						Eol = eol::bad_win;
				}

				if (keep_eol != eol::none)
				{
					PrevLine->SetEOL(keep_eol);
				}
				else
				{
					PrevLine->SetEOL(Eol);
				}

				i += Eol.str().size();
			}
			else
			{
				if (EdOpt.AutoIndent)    // первый символ вставим так, чтобы
				{                        // сработал автоотступ
					ProcessChar(Data[i]); //BUGBUG
					++i;
					StartPos=m_it_CurLine->GetCurPos();

					if (StartPos) StartPos--;
				}

				const size_t Pos = std::find_if(Data.cbegin() + i, Data.cend(), IsEol) - Data.cbegin();
				if (Pos != i)
				{
					const auto& Str = m_it_CurLine->GetString();
					const auto CurPos = m_it_CurLine->GetCurPos();
					AddUndoData(undo_type::edit, Str, m_it_CurLine->GetEOL(), m_it_CurLine.Number(), CurPos); // EOL? - CurLine->GetEOL()  GlobalEOL   ""
					m_it_CurLine->InsertString(string_view(Data).substr(i, Pos - i));
					Change(ECTYPE_CHANGED, m_it_CurLine.Number());
				}

				i = Pos;
			}
		}

		EdOpt.AutoIndent=oldAutoIndent;
		m_it_CurLine->Select(StartPos,m_it_CurLine->GetCurPos());
		/* IS $ */

		if (SaveOvertype)
		{
			m_Flags.Set(FEDITOR_OVERTYPE);
			m_it_CurLine->SetOvertypeMode(true);
		}

		Pasting--;
		AddUndoData(undo_type::end);
}

void Editor::ProcessChar(wchar_t Char)
{
	bool RefreshMe = false;
	if (Char != L'\0')
		ProcessKeyInternal(Manager::Key(Char), RefreshMe);
	else
	{
		const auto cur_pos = m_it_CurLine->GetCurPos();
		ProcessKeyInternal(Manager::Key(L' '), RefreshMe);
		auto& line = m_it_CurLine->m_Str;
		if (line.size() > cur_pos && line[cur_pos] == L' ')
			line[cur_pos] = {};
	}
}

void Editor::Copy(const bool Append)
{
	if (IsVerticalSelection())
	{
		VCopy(Append);
		return;
	}

	const clipboard_accessor Clip;

	if (Clip->Open())
	{
		string CopyData;

		if (Append)
			Clip->GetText(CopyData);

		Clip->SetText(CopyData + Block2Text());
	}
}

string Editor::Block2Text()
{
	size_t TotalChars = 0;

	auto SelEnd = Lines.end();

	for (auto i = m_it_AnyBlockStart.base(), end = Lines.end(); i != end; ++i)
	{
		if (!i->IsSelection())
		{
			SelEnd = i;
			break;
		}

		intptr_t StartSel, EndSel;
		i->GetSelection(StartSel, EndSel);

		if (EndSel == -1)
		{
			TotalChars += i->GetLength() - StartSel;
			TotalChars += i->GetEOL().str().size();
		}
		else
			TotalChars += EndSel - StartSel;
	}

	string CopyData;
	CopyData.reserve(TotalChars);

	for (const auto& i: std::ranges::subrange(m_it_AnyBlockStart.base(), SelEnd))
	{
		CopyData += i.GetSelString();

		intptr_t StartSel, EndSel;
		i.GetSelection(StartSel, EndSel);
		if (EndSel == -1)
		{
			append(CopyData, i.GetEOL().str());
		}
	}

	return CopyData;
}


void Editor::DeleteBlock()
{
	if (m_Flags.Check(FEDITOR_LOCKMODE))
		return;

	if (IsVerticalSelection())
	{
		DeleteVBlock();
		return;
	}


	AddUndoData(undo_type::begin);

	for (auto CurPtr = m_it_AnyBlockStart; CurPtr != EndIterator();)
	{
		TextChanged(true);
		intptr_t StartSel,EndSel;
		/* $ 17.09.2002 SKV
		  меняем на Real что б ловить выделение за концом строки.
		*/
		CurPtr->GetRealSelection(StartSel,EndSel);

		if (EndSel!=-1 && EndSel>CurPtr->GetLength())
			EndSel=-1;

		if (StartSel==-1)
			break;

		if (!StartSel && EndSel==-1)
		{
			CurPtr = DeleteString(CurPtr, false);

			if (CurPtr != Lines.end())
			{
				continue;
			}
			else
				break;
		}

		size_t Length = CurPtr->GetLength();

		if (StartSel || EndSel)
		{
			AddUndoData(undo_type::edit, CurPtr->GetString(), CurPtr->GetEOL(), m_it_AnyBlockStart.Number(), CurPtr->GetCurPos());
			LastChangeStrPos = StartSel;
		}

		/* $ 17.09.2002 SKV
		  опять про выделение за концом строки.
		  InsertString добавит trailing space'ов
		*/
		if (StartSel > static_cast<intptr_t>(Length))
		{
			Length=StartSel;
			CurPtr->SetCurPos(static_cast<int>(Length));
			CurPtr->InsertString({});
		}

		auto TmpStr = CurPtr->GetString();
		const auto Eol = CurPtr->GetEOL();

		int DeleteNext=FALSE;

		if (EndSel==-1)
		{
			EndSel=Length;

			if (!IsLastLine(CurPtr))
				DeleteNext=TRUE;
		}

		TmpStr.erase(StartSel, EndSel - StartSel);

		const auto CurPos = StartSel;
		/*
		if (CurPos>=StartSel)
		{
			CurPos-=(EndSel-StartSel);
			if (CurPos<StartSel)
				CurPos=StartSel;
		}
		*/

		if (DeleteNext)
		{
			intptr_t NextStartSel,NextEndSel;
			const auto NextLine = std::next(CurPtr);
			NextLine->GetSelection(NextStartSel,NextEndSel);

			if (NextStartSel == -1)
			{
				NextEndSel = 0;
			}

			if (NextEndSel == -1)
			{
				EndSel = -1;
			}
			else
			{
				const auto& NextStr = NextLine->GetString();
				if (NextStr.size() > static_cast<size_t>(NextEndSel))
				{
					TmpStr.append(string_view(NextStr).substr(NextEndSel));
				}
			}

			if (m_it_CurLine == NextLine)
			{
				m_it_CurLine=CurPtr;
			}

			if (m_it_CurLine == CurPtr && NextLine == m_it_TopScreen)
			{
				m_it_TopScreen=CurPtr;
			}

			DeleteString(NextLine, false);
		}

		CurPtr->SetString(TmpStr);
		CurPtr->SetEOL(Eol);

		CurPtr->SetCurPos(CurPos);
		if (StartSel || EndSel)
		{
			Change(ECTYPE_CHANGED, m_it_AnyBlockStart.Number());
		}

		if (DeleteNext && EndSel==-1)
		{
			CurPtr->Select(CurPtr->GetLength(),-1);
		}
		else
		{
			++CurPtr;
		}
	}

	AddUndoData(undo_type::end);
	Unselect();
}


void Editor::UnmarkBlock()
{
	m_Flags.Clear(FEDITOR_CURPOSCHANGEDBYPLUGIN);
	if (!IsAnySelection())
		return;

	TurnOffMarkingBlock();

	while (m_it_AnyBlockStart != Lines.end())
	{
		intptr_t StartSel,EndSel;
		m_it_AnyBlockStart->GetSelection(StartSel,EndSel);

		if (StartSel==-1)
		{
			/* $ 24.06.2002 SKV
			  Если в текущей строки нет выделения,
			  это еще не значит что мы в конце.
			  Это может быть только начало :)
			*/
			const auto NextLine = std::next(m_it_AnyBlockStart);
			if (NextLine != Lines.end())
			{
				NextLine->GetSelection(StartSel,EndSel);

				if (StartSel==-1)
				{
					break;
				}
			}
			else
				break;
		}

		m_it_AnyBlockStart->RemoveSelection();
		++m_it_AnyBlockStart;
	}

	Unselect();
	if (!m_InEERedraw) Refresh();
}

/* $ 07.03.2002 IS
   Удалить выделение, если оно пустое (выделено ноль символов в ширину)
*/
void Editor::UnmarkEmptyBlock()
{
	if (IsAnySelection()) // присутствует выделение
	{
		int nLines=0;
		auto Block = m_it_AnyBlockStart;

		if (IsVerticalSelection())
		{
			if (VBlockSizeX)
				nLines=VBlockSizeY;
		}
		else
		{
			while (Block != Lines.end()) // пробегаем по всем выделенным строкам
			{
				intptr_t StartSel,EndSel;
				Block->GetRealSelection(StartSel,EndSel);

				if (StartSel==-1)
					break;

				if (StartSel!=EndSel)// выделено сколько-то символов
				{
					++nLines;           // увеличим счетчик непустых строк
					break;
				}

				++Block;
			}
		}
		if (!nLines)            // если выделено ноль символов в ширину, то
			UnmarkBlock();       // перестанем морочить голову и снимем выделение
	}
}

void Editor::UnmarkMacroBlock()
{
	m_it_MBlockStart = EndIterator();
	MBlockStartX=-1;
}

void Editor::GoToLine(size_t Line)
{
	if (Line != m_it_CurLine.uNumber())
	{
		bool bReverse = false;
		const auto LastNumLine = m_it_CurLine.Number();
		auto CurScrLine = CalcDistance(m_it_TopScreen, m_it_CurLine);
		const auto CurPos = m_it_CurLine->GetTabCurPos();
		const auto LeftPos = m_it_CurLine->GetLeftPos();

		if (Line < m_it_CurLine.uNumber())
		{
			if (Line > m_it_CurLine.uNumber() / 2)
			{
				bReverse = true;
			}
			else
			{
				m_it_CurLine = FirstLine();
			}
		}
		else
		{
			if (Line > (m_it_CurLine.Number() + (Lines.size() - m_it_CurLine.Number()) / 2))
			{
				bReverse = true;
				m_it_CurLine = LastLine();
			}
		}

		if (bReverse)
		{
			while (m_it_CurLine.uNumber() > Line && m_it_CurLine != Lines.begin())
			{
				--m_it_CurLine;
			}
		}
		else
		{
			while (m_it_CurLine.uNumber() < Line && !IsLastLine(m_it_CurLine))
			{
				++m_it_CurLine;
			}
		}

		CurScrLine += m_it_CurLine.Number() - LastNumLine;

		if (CurScrLine < 0 || CurScrLine > m_Where.height() - 1)
			m_it_TopScreen=m_it_CurLine;

		m_it_CurLine->SetLeftPos(LeftPos);
		m_it_CurLine->SetTabCurPos(CurPos);
	}

// <GOTO_UNMARK:2>
//  if (!EdOpt.PersistentBlocks)
//     UnmarkBlock();
// </GOTO_UNMARK>
	return;
}

void Editor::GoToLineAndShow(size_t Line)
{
	GoToLine(Line);
	Show();
}

void Editor::GoToPosition()
{
	goto_coord Row{};
	goto_coord Col{};

	if (!GoToRowCol(Row, Col, m_GotoHex, L"EditorGotoPos"sv))
		return;

	if (Row.exist)
	{
		if (!Row.percent && !Row.relative)
			--Row.value;

		auto NewRow = Row.percent? FromPercent(Row.value, Lines.size()) : Row.value;

		if (Row.relative)
			NewRow = m_it_CurLine.Number() + NewRow * Row.relative;

		GoToLine(NewRow);
	}

	if (Col.exist)
	{
		if (!Col.percent && !Col.relative)
			--Col.value;

		auto NewCol = Col.percent? FromPercent(Col.value, m_it_CurLine->m_Str.size()) : Col.value;

		if (Col.relative)
			NewCol = m_it_CurLine->GetCurPos() + NewCol * Col.relative;

		m_it_CurLine->SetCurPos(static_cast<int>(NewCol));
	}

	Show();
}

struct Editor::EditorUndoData
{
	NONCOPYABLE(EditorUndoData);
	MOVE_CONSTRUCTIBLE(EditorUndoData);

	// KEEP ALIGNED
	string m_Str;
	bookmark_list m_BM; //treat as uni-directional linked list
	int m_StrPos;
	int m_StrNum;
	eol m_EOL;
	undo_type m_Type;

private:
	static size_t UndoDataSize;

public:
	static size_t GetUndoDataSize() { return UndoDataSize; }

	EditorUndoData(undo_type Type, string_view const Str, eol Eol, int StrNum, int StrPos):
		m_Str(Str),
		m_StrPos(StrPos),
		m_StrNum(StrNum),
		m_EOL(Eol),
		m_Type(Type)
	{
		UndoDataSize += m_Str.size();
	}

	~EditorUndoData()
	{
		UndoDataSize -= m_Str.size();
	}

	void SetData(undo_type Type, string_view const Str, eol Eol, int StrNum, int StrPos)
	{
		m_Type = Type;
		m_StrPos = StrPos;
		m_StrNum = StrNum;
		UndoDataSize -= m_Str.size();
		UndoDataSize += Str.size();
		m_EOL = Eol;
		m_Str = Str;
		m_BM.clear();
	}

	static size_t HashBM(bookmark_list::iterator BM)
	{
		// BUGBUG this is some dark magic
		auto x = std::bit_cast<size_t>(std::to_address(BM));
		x ^= (BM->Line << 16) ^ (BM->Cursor);
		return x;
	}
};

size_t Editor::EditorUndoData::UndoDataSize = 0;

void Editor::AddUndoData(undo_type Type, string_view const Str, eol Eol, int StrNum, int StrPos)
{
	if (m_Flags.Check(FEDITOR_DISABLEUNDO))
		return;

	if (StrNum==-1)
		StrNum = m_it_CurLine.Number();

	const auto Begin = UndoPos == UndoData.end()? UndoData.begin() : std::next(UndoPos);
	for (auto Iterator = Begin, End = UndoData.end(); Iterator != End; ++Iterator)
	{
		if (Iterator == UndoSavePos)
		{
			UndoSavePos = UndoData.end();
			m_Flags.Set(FEDITOR_UNDOSAVEPOSLOST);
			break;
		}
	}
	UndoData.erase(Begin, UndoData.end());

	auto PrevUndo=UndoData.end();
	if(!UndoData.empty())
	{
		--PrevUndo;
		switch (Type)
		{
		case undo_type::end:
			{
				if (PrevUndo->m_Type != undo_type::begin)
				{
					if(PrevUndo != UndoData.begin())
						--PrevUndo;
					else
						PrevUndo = UndoData.end();
				}
				if (PrevUndo != UndoData.end() && PrevUndo->m_Type == undo_type::begin)
				{
					const auto eq = PrevUndo == UndoSavePos;
					UndoData.erase(PrevUndo);
					UndoPos=UndoData.end();
					if(!UndoData.empty())
						--UndoPos;
					if (eq)
						UndoSavePos=UndoPos;

					return;
				}
			}
			break;

		case undo_type::edit:
			{
				if (!m_Flags.Check(FEDITOR_NEWUNDO) && PrevUndo->m_Type == undo_type::edit && StrNum == PrevUndo->m_StrNum &&
						(std::abs(StrPos-PrevUndo->m_StrPos)<=1 || abs(StrPos-LastChangeStrPos)<=1))
				{
					LastChangeStrPos=StrPos;
					return;
				}
			}
			break;

		case undo_type::insert_string:
		case undo_type::delete_string:
		case undo_type::begin:
			break;
		}
	}

	m_Flags.Clear(FEDITOR_NEWUNDO);
	UndoData.emplace_back(Type, Str, Eol, StrNum, StrPos);
	UndoPos=UndoData.end();
	--UndoPos;

	if (EdOpt.UndoSize>0)
	{
		while (!UndoData.empty() && (EditorUndoData::GetUndoDataSize()>static_cast<size_t>(EdOpt.UndoSize) || UndoSkipLevel>0))
		{
			const auto ub = UndoData.begin();

			if (ub->m_Type == undo_type::begin)
				++UndoSkipLevel;

			if (ub->m_Type == undo_type::end && UndoSkipLevel > 0)
				--UndoSkipLevel;

			if (UndoSavePos == UndoData.end())
				m_Flags.Set(FEDITOR_UNDOSAVEPOSLOST);

			if (ub==UndoSavePos)
				UndoSavePos = UndoData.end();

			UndoData.pop_front();
		}

		UndoPos=UndoData.end();
		if(!UndoData.empty())
			--UndoPos;
	}
}

void Editor::Undo(int redo)
{
	auto ustart = UndoPos;
	if(redo)
	{
		if (ustart == UndoData.end())
		{
			ustart = UndoData.begin();
		}
		else
		{
			++ustart;
		}
	}
	if (ustart == UndoData.end())
		return;

	TextChanged(true);
	m_Flags.Set(FEDITOR_DISABLEUNDO);
	int level=0;
	auto uend = ustart;

	for (;;)
	{
		if (uend->m_Type == undo_type::begin || uend->m_Type == undo_type::end)
		{
			const auto l = uend->m_Type == undo_type::begin? -1 : 1;
			level+=redo ? -l : l;
		}

		if (level<=0)
			break;

		if(redo)
		{
			++uend;
			if(uend == UndoData.end())
				break;
		}
		else
		{
			if(uend == UndoData.begin())
				break;
			--uend;
		}
	}

	if (level)
		uend=ustart;

	UnmarkBlock();
	auto ud=ustart;
	bool RefreshMe = false;

	for (;;)
	{
		if (ud->m_Type != undo_type::begin && ud->m_Type != undo_type::end)
		{
			GoToLine(ud->m_StrNum);
		}

		switch (ud->m_Type)
		{
			case undo_type::insert_string:
				ud->SetData(undo_type::delete_string,m_it_CurLine->GetString(),m_it_CurLine->GetEOL(),ud->m_StrNum,ud->m_StrPos);
				DeleteString(m_it_CurLine, true);
				break;

			case undo_type::delete_string:
				ud->m_Type = undo_type::insert_string;
				Pasting++;

				if (m_it_CurLine.Number() < ud->m_StrNum)
				{
					ProcessKeyInternal(Manager::Key(KEY_END), RefreshMe);
					ProcessKeyInternal(Manager::Key(KEY_ENTER), RefreshMe);
				}
				else
				{
					ProcessKeyInternal(Manager::Key(KEY_HOME), RefreshMe);
					ProcessKeyInternal(Manager::Key(KEY_ENTER), RefreshMe);
					ProcessKeyInternal(Manager::Key(KEY_UP), RefreshMe);
				}

				Pasting--;

				MoveSavedSessionBookmarksBack(ud->m_BM);

				m_it_CurLine->SetString(ud->m_Str);
				m_it_CurLine->SetEOL(ud->m_EOL); // необходимо дополнительно выставлять, т.к. SetString вызывает Edit::SetString и... дальше по тексту
				Change(ECTYPE_CHANGED, m_it_CurLine.Number());
				break;

			case undo_type::edit:
			{
				EditorUndoData tmp(undo_type::edit, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), ud->m_StrNum, ud->m_StrPos);

				m_it_CurLine->SetString(ud->m_Str);
				m_it_CurLine->SetEOL(ud->m_EOL); // необходимо дополнительно выставлять, т.к. SetString вызывает Edit::SetString и... дальше по тексту
				Change(ECTYPE_CHANGED, m_it_CurLine.Number());

				m_it_CurLine->SetCurPos(ud->m_StrPos);
				// BUGBUG
				ud->SetData(tmp.m_Type, tmp.m_Str, tmp.m_EOL, tmp.m_StrNum, tmp.m_StrPos);
				break;
			}

			case undo_type::begin:
			case undo_type::end:
				break;
		}

		if (ud==uend)
			break;

		redo? ++ud : --ud;
	}

	UndoPos = ud;
	if(!redo)
	{
		if(UndoPos == UndoData.begin())
			UndoPos = UndoData.end();
		else
			--UndoPos;
	}
	if (!m_Flags.Check(FEDITOR_UNDOSAVEPOSLOST) && UndoPos==UndoSavePos)
		TextChanged(false);

	m_Flags.Clear(FEDITOR_DISABLEUNDO);
}

void Editor::SelectAll()
{
	BeginStreamMarking(FirstLine());

	for (auto& i: Lines)
	{
		i.Select(0, -1);
	}
	Lines.back().Select(0, Lines.back().GetLength());

	TurnOffMarkingBlock();

	Show();
}


void Editor::SetStartPos(int LineNum,int CharNum)
{
	m_StartLine=LineNum? LineNum:1;
	StartChar=CharNum? CharNum:1;
}

bool Editor::IsModified() const
{
	return m_Flags.Check(FEDITOR_MODIFIED);
}

// используется в FileEditor
long long Editor::GetCurPos(bool file_pos, bool add_bom) const
{
	enum { UnknownMultiplier = -1 };
	int Multiplier = 1;
	unsigned long long bom = 0;
	const auto Codepage = GetCodePage();

	if (file_pos)
	{
		switch (Codepage)
		{
		case CP_UTF8:
			Multiplier = UnknownMultiplier;
			if (add_bom)
				bom = 3;
			break;

		case CP_UTF16LE:
		case CP_UTF16BE:
			Multiplier = sizeof(char16_t);
			if (add_bom)
				bom = 1;
			break;

		default:
			if (const auto Info = GetCodePageInfo(Codepage); Info && Info->MaxCharSize > 1)
				Multiplier = UnknownMultiplier;
		}
	}

	const auto TotalSize = std::ranges::fold_left(Lines.cbegin(), m_it_TopScreen.cbase(), bom, [&](auto Value, const auto& line)
	{
		const auto& Str = line.GetString();
		return Value + (Multiplier != UnknownMultiplier? Str.size() : encoding::get_bytes_count(Codepage, Str)) + line.GetEOL().str().size();
	});

	return Multiplier != UnknownMultiplier? TotalSize * Multiplier : TotalSize;
}


void Editor::BlockLeft()
{
	if (IsVerticalSelection())
	{
		VBlockShift(TRUE);
		return;
	}

	auto CurPtr = m_it_AnyBlockStart;
	/* $ 14.02.2001 VVM
	  + При отсутствии блока AltU/AltI сдвигают текущую строчку */
	int MoveLine = 0;

	if (CurPtr == Lines.end())
	{
		MoveLine = 1;
		CurPtr = m_it_CurLine;
	}

	AddUndoData(undo_type::begin);

	while (CurPtr != Lines.end())
	{
		intptr_t StartSel,EndSel;
		CurPtr->GetSelection(StartSel,EndSel);

		/* $ 14.02.2001 VVM
		  + Блока нет - сделаем его искусственно */
		if (MoveLine)
		{
			StartSel = 0; EndSel = -1;
		}

		if (StartSel==-1)
			break;

		const auto& CurStr = CurPtr->GetString();
		const auto Eol = CurPtr->GetEOL();

		if (!CurStr.empty())
		{
			string TmpStr;
			if (CurStr.front() == L' ')
			{
				TmpStr = CurStr.substr(1);
			}
			else if (CurStr.front() == L'\t')
			{
				TmpStr.assign(EdOpt.TabSize - 1, L' ');
				TmpStr.append(CurStr, 1);
			}

			if ((EndSel == -1 || EndSel > StartSel) && std::iswblank(CurStr.front()))
			{
				AddUndoData(undo_type::edit, CurStr, CurPtr->GetEOL(), CurPtr.Number(), 0); // EOL? - CurLine->GetEOL()  GlobalEOL   ""
				const auto CurPos = CurPtr->GetCurPos();
				CurPtr->SetString(TmpStr, true);
				CurPtr->SetEOL(Eol);
				CurPtr->SetCurPos(CurPos > 0? CurPos - 1 : CurPos);

				if (!MoveLine)
					CurPtr->Select(StartSel > 0? StartSel - 1 : StartSel, EndSel > 0? EndSel - 1 : EndSel);

				Change(ECTYPE_CHANGED, CurPtr.Number());
				TextChanged(true);
			}
		}
		++CurPtr;
		MoveLine = 0;
	}

	AddUndoData(undo_type::end);
}


void Editor::BlockRight()
{
	if (IsVerticalSelection())
	{
		VBlockShift(FALSE);
		return;
	}

	auto CurPtr = m_it_AnyBlockStart;
	/* $ 14.02.2001 VVM
	  + При отсутствии блока AltU/AltI сдвигают текущую строчку */
	int MoveLine = 0;

	if (CurPtr == Lines.end())
	{
		MoveLine = 1;
		CurPtr = m_it_CurLine;
	}

	AddUndoData(undo_type::begin);

	while (CurPtr != Lines.end())
	{
		intptr_t StartSel,EndSel;
		CurPtr->GetSelection(StartSel,EndSel);

		/* $ 14.02.2001 VVM
		  + Блока нет - сделаем его искусственно */
		if (MoveLine)
		{
			StartSel = 0; EndSel = -1;
		}

		if (StartSel==-1)
			break;

		const size_t Length = CurPtr->GetLength();

		if (Length && (EndSel == -1 || EndSel > StartSel))
		{
			const auto& CurStr = CurPtr->GetString();
			const auto Eol = CurPtr->GetEOL();
			const auto TmpStr = concat(L' ', CurStr);

			AddUndoData(undo_type::edit, CurStr, CurPtr->GetEOL(), CurPtr.Number(), 0); // EOL? - CurLine->GetEOL()  GlobalEOL   ""
			const auto CurPos = CurPtr->GetCurPos();

			CurPtr->SetString(TmpStr, true);
			CurPtr->SetEOL(Eol);

			CurPtr->SetCurPos(CurPos+1);

			if (!MoveLine)
				CurPtr->Select(StartSel>0 ? StartSel+1:StartSel,EndSel>0 ? EndSel+1:EndSel);

			Change(ECTYPE_CHANGED, CurPtr.Number());
			TextChanged(true);
		}
		++CurPtr;
		MoveLine = 0;
	}

	AddUndoData(undo_type::end);
}


void Editor::DeleteVBlock()
{
	if (m_Flags.Check(FEDITOR_LOCKMODE) || VBlockSizeX<=0 || VBlockSizeY<=0)
		return;

	AddUndoData(undo_type::begin);

	if (!EdOpt.PersistentBlocks)
	{
		auto CurPtr=m_it_CurLine;
		auto NewTopScreen=m_it_TopScreen;

		while (CurPtr != Lines.end())
		{
			if (CurPtr == m_it_AnyBlockStart)
			{
				m_it_TopScreen=NewTopScreen;
				m_it_CurLine=CurPtr;
				CurPtr->SetTabCurPos(VBlockX);
				break;
			}

			if (NewTopScreen == CurPtr && CurPtr != Lines.begin())
				--NewTopScreen;

			--CurPtr;
		}
	}

	auto CurPtr = m_it_AnyBlockStart;

	for (int Line = 0; CurPtr != Lines.end() && Line < VBlockSizeY; ++Line, ++CurPtr)
	{
		TextChanged(true);
		const size_t TBlockX = CurPtr->VisualPosToReal(VBlockX);
		const size_t TBlockSizeX = CurPtr->VisualPosToReal(VBlockX + VBlockSizeX) - CurPtr->VisualPosToReal(VBlockX);
		const auto& CurStr = CurPtr->GetString();

		if (TBlockX >= CurStr.size())
			continue;

		AddUndoData(undo_type::edit, CurPtr->GetString(), CurPtr->GetEOL(), CurPtr.Number(), CurPtr->GetCurPos());
		string TmpStr(CurStr, 0, TBlockX);

		if (CurStr.size() > TBlockX + TBlockSizeX)
		{
			TmpStr.append(string_view(CurStr).substr(TBlockX + TBlockSizeX));
		}

		append(TmpStr, CurPtr->GetEOL().str());
		size_t CurPos = CurPtr->GetCurPos();
		CurPtr->SetString(TmpStr, true);

		if (CurPos>TBlockX)
		{
			CurPos-=TBlockSizeX;

			if (CurPos<TBlockX)
				CurPos=TBlockX;
		}

		CurPtr->SetCurPos(static_cast<int>(CurPos));
		Change(ECTYPE_CHANGED, CurPtr.Number());
	}

	AddUndoData(undo_type::end);
	Unselect();
}

void Editor::VCopy(int Append)
{
	const clipboard_accessor Clip;

	if (Clip->Open())
	{
		string CopyData;

		if (Append)
		{
			if (!Clip->GetVText(CopyData))
				Clip->GetText(CopyData);
		}

		Clip->SetVText(CopyData + VBlock2Text());
	}
}

string Editor::VBlock2Text()
{
	//RealPos всегда <= TabPos, поэтому берём максимальный размер буфера
	const size_t TotalChars = (VBlockSizeX + 2) * VBlockSizeY;

	string CopyData;
	CopyData.reserve(TotalChars);

	auto CurPtr = m_it_AnyBlockStart;

	for (int Line = 0; CurPtr != Lines.end() && Line < VBlockSizeY; ++Line, ++CurPtr)
	{
		const size_t TBlockX = CurPtr->VisualPosToReal(VBlockX);
		const size_t TBlockSizeX = CurPtr->VisualPosToReal(VBlockX + VBlockSizeX) - TBlockX;
		const auto& CurStr = CurPtr->GetString();

		if (CurStr.size() > TBlockX)
		{
			size_t CopySize = CurStr.size() - TBlockX;

			if (CopySize>TBlockSizeX)
				CopySize=TBlockSizeX;

			CopyData.append(CurStr, TBlockX, CopySize);

			if (CopySize<TBlockSizeX)
				CopyData.append(TBlockSizeX-CopySize, L' ');
		}
		else
		{
			CopyData.append(TBlockSizeX, L' ');
		}

		append(CopyData, GetDefaultEOL().str());
	}

	return CopyData;
}

void Editor::VPaste(string_view const Data)
{
		if (m_Flags.Check(FEDITOR_LOCKMODE))
			return;

		AddUndoData(undo_type::begin);

		if (!EdOpt.PersistentBlocks && !IsVerticalSelection())
			DeleteBlock();

		m_Flags.Set(FEDITOR_NEWUNDO);
		TextChanged(true);
		const auto SaveOvertype = m_Flags.Check(FEDITOR_OVERTYPE);
		UnmarkBlock();
		Pasting++;
		if (m_Flags.Check(FEDITOR_OVERTYPE))
		{
			m_Flags.Clear(FEDITOR_OVERTYPE);
			m_it_CurLine->SetOvertypeMode(false);
		}

		BeginVBlockMarking();
		const auto StartPos = m_it_CurLine->GetTabCurPos();
		VBlockSizeY=0;
		const auto SavedTopScreen = m_it_TopScreen;

		for (size_t i = 0, size = Data.size(); i != size; )
		{
			auto NewLineLen = 0;
			if (Data[i] == L'\n')
			{
				NewLineLen = 1;
			}
			else
			{
				if (Data[i] == L'\r')
				{
					NewLineLen = (i + 1 < size && Data[i + 1] == L'\n')? 2 : 1;
				}
			}

			if (!NewLineLen)
			{
				ProcessChar(Data[i++]);
			}
			else
			{
				const auto CurWidth = m_it_CurLine->GetTabCurPos() - StartPos;

				if (CurWidth>VBlockSizeX)
					VBlockSizeX=CurWidth;

				VBlockSizeY++;

				bool RefreshMe = false;
				if (IsLastLine(m_it_CurLine))
				{
					if (i + NewLineLen < size)
					{
						ProcessKeyInternal(Manager::Key(KEY_END), RefreshMe);
						ProcessKeyInternal(Manager::Key(KEY_ENTER), RefreshMe);

						// Mantis 0002966: Неправильная вставка вертикального блока в конце файла
							repeat(StartPos, [this, &RefreshMe]{ ProcessKeyInternal(Manager::Key(L' '), RefreshMe); });
					}
				}
				else
				{
					ProcessKeyInternal(Manager::Key(KEY_DOWN), RefreshMe);
					m_it_CurLine->SetTabCurPos(StartPos);
					m_it_CurLine->SetOvertypeMode(false);
				}

				i += NewLineLen;
			}
		}

		const auto CurWidth = m_it_CurLine->GetTabCurPos() - StartPos;

		if (CurWidth>VBlockSizeX)
			VBlockSizeX=CurWidth;

		if (!VBlockSizeY)
			VBlockSizeY++;

		if (SaveOvertype)
		{
			m_Flags.Set(FEDITOR_OVERTYPE);
			m_it_CurLine->SetOvertypeMode(true);
		}

		m_it_TopScreen=SavedTopScreen;
		m_it_CurLine = m_it_AnyBlockStart;
		m_it_CurLine->SetTabCurPos(StartPos);
		Pasting--;
		AddUndoData(undo_type::end);
}


void Editor::VBlockShift(int Left)
{
	if (m_Flags.Check(FEDITOR_LOCKMODE) || (Left && !VBlockX) || VBlockSizeX<=0 || VBlockSizeY<=0)
		return;

	auto CurPtr = m_it_AnyBlockStart;
	AddUndoData(undo_type::begin);

	for (int Line = 0; CurPtr != Lines.end() && Line < VBlockSizeY; ++Line, ++CurPtr)
	{
		TextChanged(true);
		size_t TBlockX = CurPtr->VisualPosToReal(VBlockX);
		size_t TBlockSizeX = CurPtr->VisualPosToReal(VBlockX + VBlockSizeX) - CurPtr->VisualPosToReal(VBlockX);
		{
			const auto& CurStr = CurPtr->GetString();

			if (TBlockX > CurStr.size())
				continue;

			if ((Left && CurStr[TBlockX - 1] == L'\t') || (!Left && TBlockX + TBlockSizeX < CurStr.size() && CurStr[TBlockX + TBlockSizeX] == L'\t'))
			{
				CurPtr->ReplaceTabs();
				TBlockX = CurPtr->VisualPosToReal(VBlockX);
				TBlockSizeX = CurPtr->VisualPosToReal(VBlockX + VBlockSizeX) -
					CurPtr->VisualPosToReal(VBlockX);
			}
		}

		AddUndoData(undo_type::edit, CurPtr->GetString(), CurPtr->GetEOL(), CurPtr.Number(), CurPtr->GetCurPos());

		const auto& CurStr = CurPtr->GetString();
		auto TmpStr = CurStr;
		TmpStr.append(std::max(CurStr.size(), TBlockX + TBlockSizeX + !Left) - CurStr.size(), L' ');

		if (Left)
		{
			const auto BlockBegin = TmpStr.begin() + TBlockX;
			std::rotate(BlockBegin - 1, BlockBegin, BlockBegin + TBlockSizeX);
		}
		else
		{
			const auto BlockBegin = TmpStr.rend() - TBlockX - TBlockSizeX;
			std::rotate(BlockBegin - 1, BlockBegin, BlockBegin + TBlockSizeX);
		}

		inplace::trim_right(TmpStr);

		append(TmpStr, CurPtr->GetEOL().str());
		CurPtr->SetString(TmpStr, true);
		Change(ECTYPE_CHANGED, CurPtr.Number());
	}

	VBlockX+=Left ? -1:1;
	m_it_CurLine->SetTabCurPos(Left ? VBlockX:VBlockX+VBlockSizeX);
	AddUndoData(undo_type::end);
}

int Editor::EditorControl(int Command, intptr_t Param1, void *Param2)
{
	if(EditorControlLocked())
		return FALSE;

	switch (Command)
	{
		case ECTL_GETSTRING:
		{
			const auto GetString = static_cast<EditorGetString*>(Param2);

			if (!CheckStructSize(GetString))
				return false;

			const auto CurPtr = GetStringByNumber(GetString->StringNumber);

			if (CurPtr == Lines.end())
				return false;

			const auto& Str = CurPtr->GetString();
			GetString->StringText = Str.data();
			GetString->StringEOL = CurPtr->GetEOL().str().data();
			GetString->StringLength = Str.size();
			GetString->SelStart = -1;
			GetString->SelEnd = 0;

			const auto DestLine = GetString->StringNumber != -1? GetString->StringNumber : m_it_CurLine.Number();
			if (IsStreamSelection())
			{
				CurPtr->GetRealSelection(GetString->SelStart,GetString->SelEnd);
			}
			else if (IsVerticalSelection() && DestLine >= m_it_AnyBlockStart.Number() && DestLine < m_it_AnyBlockStart.Number() + VBlockSizeY)
			{
				GetString->SelStart = CurPtr->VisualPosToReal(VBlockX);
				GetString->SelEnd = GetString->SelStart + CurPtr->VisualPosToReal(VBlockX + VBlockSizeX) - CurPtr->VisualPosToReal(VBlockX);
			}

			return true;
		}

		case ECTL_INSERTSTRING:
		{
			if (m_Flags.Check(FEDITOR_LOCKMODE))
				return false;

			TurnOffMarkingBlock();
			const auto Indent = Param2 && *static_cast<int*>(Param2) != false;

			if (!Indent)
				++Pasting;

			m_Flags.Set(FEDITOR_NEWUNDO);
			InsertString();

			if (!Indent)
				--Pasting;

			return true;
		}

		case ECTL_INSERTTEXT:
		{
			if (!Param2)
				return false;

			if (m_Flags.Check(FEDITOR_LOCKMODE))
				return false;

			TurnOffMarkingBlock();
			bool RefreshMe = false;
			auto Str = static_cast<const wchar_t*>(Param2);
			++Pasting;

			while (*Str)
			{
				if (L'\n' == *Str)
				{
					--Pasting;
					InsertString();
					++Pasting;
				}
				else
				{
					ProcessKeyInternal(Manager::Key(*Str), RefreshMe);
				}
				++Str;
			}
			--Pasting;
			Refresh();

			return true;
		}

		case ECTL_SETSTRING:
		{
			const auto SetString = static_cast<const EditorSetString*>(Param2);
			if (!CheckStructSize(SetString))
				return false;

			if (m_Flags.Check(FEDITOR_LOCKMODE))
				return false;

			const auto CurPtr = GetStringByNumber(SetString->StringNumber);
			if (CurPtr == Lines.end())
				return false;

			m_Flags.Set(FEDITOR_CURPOSCHANGEDBYPLUGIN);
			const auto DestLine = SetString->StringNumber != -1? SetString->StringNumber : m_it_CurLine.Number();

			AddUndoData(undo_type::edit, CurPtr->GetString(), CurPtr->GetEOL(), DestLine, CurPtr->GetCurPos());
			const auto CurPos = CurPtr->GetCurPos();
			CurPtr->SetString({ SetString->StringText, static_cast<size_t>(SetString->StringLength) }, true);

			if (CurPtr->GetEOL() == eol::none)
				CurPtr->SetEOL(SetString->StringEOL? eol::parse(SetString->StringEOL) : GlobalEOL);

			CurPtr->SetCurPos(CurPos);
			Change(ECTYPE_CHANGED, DestLine);
			TextChanged(true);

			return true;
		}

		case ECTL_DELETESTRING:
		{
			if (m_Flags.Check(FEDITOR_LOCKMODE))
				return false;

			TurnOffMarkingBlock();
			DeleteString(m_it_CurLine, false);

			return true;
		}

		case ECTL_DELETECHAR:
		{
			if (m_Flags.Check(FEDITOR_LOCKMODE))
				return false;

			TurnOffMarkingBlock();
			++Pasting;
			ProcessKey(Manager::Key(KEY_DEL));
			--Pasting;
			return true;
		}

		case ECTL_GETINFO:
		{
			const auto Info = static_cast<EditorInfo*>(Param2);
			if (!CheckStructSize(Info))
				return false;

			Info->EditorID = EditorID;
			Info->WindowSizeX=ObjWidth();
			Info->WindowSizeY = ObjHeight();
			Info->TotalLines = Lines.size();
			Info->CurLine = m_it_CurLine.Number();
			Info->CurPos=m_it_CurLine->GetCurPos();
			Info->CurTabPos=m_it_CurLine->GetTabCurPos();
			Info->TopScreenLine = m_it_CurLine.Number() - CalcDistance(m_it_TopScreen, m_it_CurLine);
			Info->LeftPos=m_it_CurLine->GetLeftPos();
			Info->Overtype=m_Flags.Check(FEDITOR_OVERTYPE);
			Info->BlockType = IsVerticalSelection()? BTYPE_COLUMN : IsStreamSelection()? BTYPE_STREAM : BTYPE_NONE;
			Info->BlockStartLine = Info->BlockType == BTYPE_NONE? 0 : m_it_AnyBlockStart.Number();
			Info->Options=0;

			if (EdOpt.ExpandTabs == EXPAND_ALLTABS)
				Info->Options|=EOPT_EXPANDALLTABS;

			if (EdOpt.ExpandTabs == EXPAND_NEWTABS)
				Info->Options|=EOPT_EXPANDONLYNEWTABS;

			if (EdOpt.PersistentBlocks)
				Info->Options|=EOPT_PERSISTENTBLOCKS;

			if (EdOpt.DelRemovesBlocks)
				Info->Options|=EOPT_DELREMOVESBLOCKS;

			if (EdOpt.AutoIndent)
				Info->Options|=EOPT_AUTOINDENT;

			if (EdOpt.SavePos)
				Info->Options|=EOPT_SAVEFILEPOSITION;

			if (EdOpt.AutoDetectCodePage)
				Info->Options|=EOPT_AUTODETECTCODEPAGE;

			if (EdOpt.CursorBeyondEOL)
				Info->Options|=EOPT_CURSORBEYONDEOL;

			if (EdOpt.ShowWhiteSpace)
			{
				Info->Options|=EOPT_SHOWWHITESPACE;

				if (EdOpt.ShowWhiteSpace==1)
					Info->Options|=EOPT_SHOWLINEBREAK;
			}

			if (EdOpt.ShowScrollBar && ScrollBarRequired(ObjHeight(), Lines.size()))
				Info->Options |= EOPT_SHOWSCROLLBAR;

			Info->TabSize=EdOpt.TabSize;
			Info->BookmarkCount=BOOKMARK_COUNT;
			Info->SessionBookmarkCount=GetSessionBookmarks(nullptr);
			Info->CurState=m_Flags.Check(FEDITOR_LOCKMODE)?ECSTATE_LOCKED:0;
			Info->CurState|=!m_Flags.Check(FEDITOR_MODIFIED)?ECSTATE_SAVED:0;
			Info->CurState|=m_Flags.Check(FEDITOR_MODIFIED)?ECSTATE_MODIFIED:0;
			Info->CodePage = GetCodePage();

			return true;
		}

		case ECTL_SETPOSITION:
		{
			const auto Pos = static_cast<const EditorSetPosition*>(Param2);
			if (!CheckStructSize(Pos))
				return false;

			// выставим флаг об изменении поз
			m_Flags.Set(FEDITOR_CURPOSCHANGEDBYPLUGIN);

			if (Pos->CurLine >= 0) // поменяем строку
			{
				if (Pos->CurLine == m_it_CurLine.Number() - 1)
					Up();
				else if (Pos->CurLine == m_it_CurLine.Number() + 1)
					Down();
				else
					GoToLine(Pos->CurLine);
			}

			if (Pos->TopScreenLine >= 0 && Pos->TopScreenLine <= m_it_CurLine.Number())
			{
				m_it_TopScreen=m_it_CurLine;

				for (int I = m_it_CurLine.Number(); I > 0 && m_it_CurLine.Number() - I < m_Where.height() - 1 && I != Pos->TopScreenLine; --I)
					--m_it_TopScreen;
			}

			if (Pos->CurPos >= 0)
				m_it_CurLine->SetCurPos(Pos->CurPos);

			if (Pos->CurTabPos >= 0)
				m_it_CurLine->SetTabCurPos(Pos->CurTabPos);

			if (Pos->LeftPos >= 0)
				m_it_CurLine->SetLeftPos(Pos->LeftPos);

			m_it_CurLine->SetRightCoord(XX2);
			m_it_CurLine->FixLeftPos();

			/* $ 30.08.2001 IS
			   Изменение режима нужно выставлять сразу, в противном случае приходят
			   глюки, т.к. плагинописатель думает, что режим изменен, и ведет себя
			   соответственно, в результате чего получает неопределенное поведение.
			*/
			if (Pos->Overtype >= 0)
			{
				m_Flags.Change(FEDITOR_OVERTYPE,Pos->Overtype!=0);
				m_it_CurLine->SetOvertypeMode(m_Flags.Check(FEDITOR_OVERTYPE));
			}

			return true;
		}

		case ECTL_SELECT:
		{
			const auto Sel = static_cast<const EditorSelect*>(Param2);
			if (!CheckStructSize(Sel))
				return false;

			if (Sel->BlockType==BTYPE_NONE || Sel->BlockStartPos==-1)
			{
				UnmarkBlock();
				return true;
			}

			if (Sel->BlockHeight < 1)
				return false;

			auto CurPtr = GetStringByNumber(Sel->BlockStartLine);
			if (CurPtr == Lines.end())
				return false;

			UnmarkBlock();

			m_Flags.Set(FEDITOR_CURPOSCHANGEDBYPLUGIN);

			if (Sel->BlockType == BTYPE_STREAM)
			{
				BeginStreamMarking(CurPtr);

				for (const auto i: std::views::iota(0, Sel->BlockHeight))
				{
					const auto SelStart = i? 0 : Sel->BlockStartPos;
					const auto SelEnd = (i < Sel->BlockHeight - 1)? -1 : Sel->BlockStartPos + Sel->BlockWidth;
					CurPtr->Select(SelStart, SelEnd);
					++CurPtr;

					if (CurPtr == Lines.end())
						return true; // ранее было FALSE
				}
			}
			else if (Sel->BlockType == BTYPE_COLUMN)
			{
				BeginVBlockMarking(CurPtr);

				VBlockX = Sel->BlockStartPos;

				VBlockSizeX = Sel->BlockWidth;
				VBlockSizeY = Sel->BlockHeight;

				if (VBlockSizeX < 0)
				{
					VBlockSizeX = -VBlockSizeX;
					VBlockX -= VBlockSizeX;

					if (VBlockX < 0)
						VBlockX = 0;
				}
			}

			return true;
		}

		case ECTL_REDRAW:
		{
			Show();
			Global->ScrBuf->Flush();
			return true;
		}

		case ECTL_TABTOREAL:
		{
			const auto ecp = static_cast<EditorConvertPos*>(Param2);
			if (!CheckStructSize(ecp))
				return false;

			const auto CurPtr = GetStringByNumber(ecp->StringNumber);
			if (CurPtr == Lines.end())
				return false;

			ecp->DestPos=CurPtr->VisualPosToReal(ecp->SrcPos);
			return true;
		}

		case ECTL_REALTOTAB:
		{
			const auto ecp = static_cast<EditorConvertPos*>(Param2);
			if (!CheckStructSize(ecp))
				return false;

			const auto CurPtr = GetStringByNumber(ecp->StringNumber);
			if (CurPtr == Lines.end())
				return false;

			ecp->DestPos = CurPtr->RealPosToVisual(ecp->SrcPos);
			return true;
		}

		case ECTL_EXPANDTABS:
		{
			if (m_Flags.Check(FEDITOR_LOCKMODE))
				return false;

			const auto StringNumber = *static_cast<intptr_t*>(Param2);
			const auto CurPtr = GetStringByNumber(StringNumber);

			if (CurPtr == Lines.end())
				return false;

			AddUndoData(undo_type::edit, CurPtr->GetString(), CurPtr->GetEOL(), StringNumber, CurPtr->GetCurPos());
			if(CurPtr->ReplaceTabs())
				Change(ECTYPE_CHANGED, StringNumber);

			return true;
		}

		// TODO: Если DI_MEMOEDIT не будет юзать раскраску, то должно выполняется в FileEditor::EditorControl(), в диалоге - нафиг ненать
		case ECTL_ADDCOLOR:
		{
			const auto col = static_cast<const EditorColor*>(Param2);
			if (!CheckStructSize(col))
				return false;

			const auto CurPtr = GetStringByNumber(col->StringNumber);
			if (CurPtr == Lines.end())
				return false;

			ColorItem newcol;
			newcol.StartPos=col->StartPos;
			newcol.EndPos=col->EndPos;
			newcol.SetColor(col->Color);
			newcol.Flags=col->Flags;
			newcol.SetOwner(col->Owner);
			newcol.Priority=col->Priority;

			AddColor(CurPtr, newcol);
			if (col->Flags & ECF_AUTODELETE)
				m_AutoDeletedColors.emplace(std::to_address(CurPtr));

			return true;
		}

		// TODO: Если DI_MEMOEDIT не будет юзать раскраску, то должно выполняется в FileEditor::EditorControl(), в диалоге - нафиг ненать
		case ECTL_GETCOLOR:
		{
			const auto col = static_cast<EditorColor*>(Param2);
			if (!CheckStructSize(col))
				return false;

			const auto CurPtr = GetStringByNumber(col->StringNumber);
			if (CurPtr == Lines.end())
				return false;

			ColorItem curcol;
			if (!GetColor(CurPtr, curcol, col->ColorItem))
				return false;

			col->StartPos = curcol.StartPos;
			col->EndPos = curcol.EndPos;
			col->Color=curcol.GetColor();
			col->Flags=curcol.Flags;
			col->Owner=curcol.GetOwner();
			col->Priority=curcol.Priority;

			return true;
		}

		case ECTL_DELCOLOR:
		{
			const auto col = static_cast<const EditorDeleteColor*>(Param2);
			if (!CheckStructSize(col))
				return false;

			const auto CurPtr = GetStringByNumber(col->StringNumber);
			if (CurPtr == Lines.end())
				return false;

			DeleteColor(CurPtr, [&](const ColorItem& Item)
			{
				return (col->StartPos == -1 || col->StartPos == Item.StartPos) && col->Owner == Item.GetOwner();
			});

			return true;
		}

		/* $ 16.02.2001 IS
		     Изменение некоторых внутренних настроек редактора. Param2 указывает на
		     структуру EditorSetParameter
		*/
		case ECTL_SETPARAM:
		{
			const auto espar = static_cast<const EditorSetParameter*>(Param2);
			if (!CheckStructSize(espar))
				return false;

			switch (espar->Type)
			{
			case ESPT_GETWORDDIV:
				if (espar->wszParam && espar->Size)
					xwcsncpy(espar->wszParam,EdOpt.strWordDiv.c_str(), espar->Size);
				return static_cast<int>(EdOpt.strWordDiv.Get().size()) + 1;

			case ESPT_SETWORDDIV:
				SetWordDiv(espar->wszParam && *espar->wszParam? string_view(espar->wszParam) : Global->Opt->EdOpt.strWordDiv);
				return true;

			case ESPT_TABSIZE:
				SetTabSize(espar->iParam);
				return true;

			case ESPT_EXPANDTABS:
				SetConvertTabs(espar->iParam);
				return true;

			case ESPT_AUTOINDENT:
				SetAutoIndent(espar->iParam != 0);
				return true;

			case ESPT_CURSORBEYONDEOL:
				SetCursorBeyondEOL(espar->iParam != 0);
				return true;

			case ESPT_CHARCODEBASE:
				SetCharCodeBase(espar->iParam);
				return true;

			case ESPT_CODEPAGE:
			{
				const uintptr_t cp = espar->iParam;
				// BUGBUG
				if (const auto HostFileEditor = std::dynamic_pointer_cast<FileEditor>(m_Owner.lock()))
				{
					if (!HostFileEditor->SetCodePageEx(cp))
						return false;
				}
				else
				{
					if (cp == CP_DEFAULT || !IsCodePageSupported(cp) || !SetCodePage(GetCodePage(), cp))
						return false;
				}
				Show();
				return true;
			}

			/* $ 29.10.2001 IS изменение настройки "Сохранять позицию файла" */
			case ESPT_SAVEFILEPOSITION:
				SetSavePosMode(espar->iParam, -1);
				return true;

				/* $ 23.03.2002 IS запретить/отменить изменение файла */
			case ESPT_LOCKMODE:
				m_Flags.Change(FEDITOR_LOCKMODE, espar->iParam!=0);
				return true;

			case ESPT_SHOWWHITESPACE:
				SetShowWhiteSpace(espar->iParam);
				return true;

			default:
				return false;
			}
		}

		case ECTL_DELETEBLOCK:
		{
			if (m_Flags.Check(FEDITOR_LOCKMODE) || !IsAnySelection())
				return false;


			TurnOffMarkingBlock();
			DeleteBlock();
			Show();
			return true;
		}

		case ECTL_UNDOREDO:
		{
			const auto eur = static_cast<const EditorUndoRedo*>(Param2);
			if (!CheckStructSize(eur))
				return false;

			switch (eur->Command)
			{
			case EUR_BEGIN:
				AddUndoData(undo_type::begin);
				return true;

			case EUR_END:
				AddUndoData(undo_type::end);
				return true;

			case EUR_UNDO:
			case EUR_REDO:
				Undo(eur->Command == EUR_REDO);
				Refresh();
				return true;

			default:
				return false;
			}
		}

		case ECTL_SUBSCRIBECHANGEEVENT:
		case ECTL_UNSUBSCRIBECHANGEEVENT:
		{
			const auto esce = static_cast<const EditorSubscribeChangeEvent*>(Param2);
			if (!CheckStructSize(esce))
				return false;

			if (Command == ECTL_SUBSCRIBECHANGEEVENT)
				ChangeEventSubscribers.emplace(esce->PluginId);
			else
				ChangeEventSubscribers.erase(esce->PluginId);

			return true;
		}
	}

	return false;
}

bool Editor::SetBookmark(int Pos)
{
	if (static_cast<size_t>(Pos) >= m_SavePos.size())
		return false;

	auto& Bookmark = m_SavePos[Pos];
	Bookmark.Line = m_it_CurLine.Number();
	Bookmark.LinePos = m_it_CurLine->GetCurPos();
	Bookmark.LeftPos = m_it_CurLine->GetLeftPos();
	Bookmark.ScreenLine = CalcDistance(m_it_TopScreen, m_it_CurLine);

	return true;
}

bool Editor::GotoBookmark(int Pos)
{
	if (static_cast<size_t>(Pos) >= m_SavePos.size())
		return false;

	const auto& Bookmark = m_SavePos[Pos];
	if (Bookmark.Line == POS_NONE)
		return true;

	GoToLine(Bookmark.Line);
	m_it_CurLine->SetCurPos(Bookmark.LinePos);
	m_it_CurLine->SetLeftPos(Bookmark.LeftPos);
	m_it_TopScreen=m_it_CurLine;

	for (int i = 0; i < Bookmark.ScreenLine && m_it_TopScreen != Lines.begin(); ++i)
		--m_it_TopScreen;

	if (!EdOpt.PersistentBlocks)
		UnmarkBlock();

	Show();

	return true;
}

void Editor::ClearSessionBookmarks()
{
	NewSessionPos = false;
	SessionBookmarks.clear();
	SessionPos = SessionBookmarks.end();
}

void Editor::UpdateCurrentSessionBookmark()
{
	const auto next = std::next(SessionPos);
	if (next != SessionBookmarks.end())
	{
		SessionPos = next;
	}
	else
	{
		if (SessionPos == SessionBookmarks.begin())
		{
			SessionPos = SessionBookmarks.end();
		}
		else
		{
			--SessionPos;
		}
	}
}

bool Editor::DeleteSessionBookmark(bookmark_list::iterator sb_delete)
{
	if (sb_delete == SessionBookmarks.end())
		return false;

	NewSessionPos = false;

	if (SessionPos == sb_delete)
	{
		UpdateCurrentSessionBookmark();
	}

	SessionBookmarks.erase(sb_delete);
	return true;
}

bool Editor::MoveSessionBookmarkToUndoList(bookmark_list::iterator sb_move)
{
	if (m_Flags.Check(FEDITOR_DISABLEUNDO) || UndoPos->m_Type != undo_type::delete_string)
		return DeleteSessionBookmark(sb_move);

	NewSessionPos = false;

	if (SessionPos == sb_move)
	{
		UpdateCurrentSessionBookmark();
	}

	sb_move->hash = sb_move == SessionBookmarks.begin() ? 0 : UndoPos->HashBM(std::prev(sb_move));
	UndoPos->m_BM.splice(UndoPos->m_BM.end(), SessionBookmarks, sb_move);
	return true;
}

bool Editor::RestoreSessionBookmark()
{
	NewSessionPos = false;
	//only if the cursor is elsewhere
	if (SessionBookmarks.empty() || (static_cast<int>(SessionPos->Line) == m_it_CurLine.Number() && static_cast<int>(SessionPos->Cursor) == m_it_CurLine->GetCurPos()))
		return false;

	GoToLine(SessionPos->Line);
	m_it_CurLine->SetCurPos(SessionPos->Cursor);
	m_it_CurLine->SetLeftPos(SessionPos->LeftPos);
	m_it_TopScreen=m_it_CurLine;

	for (DWORD I = 0; I < SessionPos->ScreenLine && m_it_TopScreen != Lines.begin(); ++I, --m_it_TopScreen)
		;

	if (!EdOpt.PersistentBlocks)
		UnmarkBlock();

	Show();
	return true;
}

void Editor::AddSessionBookmark(bool NewPos)
{
	//remove all subsequent bookmarks
	if (!SessionBookmarks.empty())
	{
		SessionBookmarks.erase(std::next(SessionPos), SessionBookmarks.end());
	}
	//append new bookmark
	InternalEditorSessionBookMark sb_new;
	sb_new.Line = m_it_CurLine.Number();
	sb_new.Cursor = m_it_CurLine->GetCurPos();
	sb_new.LeftPos = m_it_CurLine->GetLeftPos();
	sb_new.ScreenLine = CalcDistance(m_it_TopScreen, m_it_CurLine);
	NewSessionPos = NewPos; // We had to save current position, if we will go to previous bookmark (by default)
	SessionBookmarks.emplace_back(sb_new);
	SessionPos = std::prev(SessionBookmarks.end());
}

void Editor::MoveSavedSessionBookmarksBack(bookmark_list& SavedList)
{
	for (auto i = SavedList.begin(), end = SavedList.end(); i != end;)
	{
		const auto next = std::next(i);
		if (i->hash)
		{
			FOR_RANGE(SessionBookmarks, j)
			{
				if (EditorUndoData::HashBM(j) == i->hash)
				{
					SessionBookmarks.splice(std::next(j), SavedList, i);
					break;
				}
			}
		}
		else
		{
			SessionBookmarks.splice(SessionBookmarks.begin(), SavedList, i);
		}
		i = next;
	}

	SavedList.clear();
}

Editor::bookmark_list::iterator Editor::PointerToSessionBookmark(int iIdx)
{
	auto Result = SessionPos;
	if (iIdx!=-1 && !SessionBookmarks.empty()) // -1 == current
	{
		Result = std::next(SessionBookmarks.begin(), iIdx);
	}
	return Result;
}

bool Editor::BackSessionBookmark()
{
	if (!SessionBookmarks.empty())
	{
		if (NewSessionPos) // If we had to save current position ...
		{
			NewSessionPos = false;
			// ... if current bookmark is last and current_position != bookmark_position
			// save current position as new bookmark
			if (std::next(SessionPos) == SessionBookmarks.end() && (static_cast<int>(SessionPos->Line) != m_it_CurLine.Number() || static_cast<int>(SessionPos->Cursor) != m_it_CurLine->GetCurPos()))
				AddSessionBookmark(false);
		}

		return PrevSessionBookmark();
	}

	return false;
}

bool Editor::PrevSessionBookmark()
{
	if (SessionBookmarks.empty())
		return false;

	if (SessionPos != SessionBookmarks.begin()) // If not first bookmark - go
		--SessionPos;

	return RestoreSessionBookmark();
}

bool Editor::NextSessionBookmark()
{
	if (SessionBookmarks.empty())
		return false;

	if (const auto Next = std::next(SessionPos); Next != SessionBookmarks.end())
		SessionPos = Next;

	return RestoreSessionBookmark();
}

bool Editor::LastSessionBookmark()
{
	if (SessionBookmarks.empty())
		return false;

	SessionPos = std::prev(SessionBookmarks.end());
	return RestoreSessionBookmark();
}

bool Editor::GotoSessionBookmark(int iIdx)
{
	if (SessionBookmarks.empty())
		return false;

	const auto sb_temp = PointerToSessionBookmark(iIdx);
	if (sb_temp == SessionBookmarks.end())
		return false;

	SessionPos = sb_temp;
	return RestoreSessionBookmark();
}

void Editor::PushSessionBookMark()
{
	if (!SessionBookmarks.empty())
		SessionPos = std::prev(SessionBookmarks.end());

	AddSessionBookmark(false);
}

bool Editor::PopSessionBookMark()
{
	return LastSessionBookmark() && DeleteSessionBookmark(SessionPos);
}

int Editor::CurrentSessionBookmarkIdx()
{
	return SessionBookmarks.empty()? -1 : std::distance(SessionBookmarks.begin(), SessionPos);
}

bool Editor::GetSessionBookmark(int iIdx, InternalEditorBookmark *Param)
{
	if (SessionBookmarks.empty() || !Param)
		return false;

	const auto sb_temp = PointerToSessionBookmark(iIdx);
	Param->Line = sb_temp->Line;
	Param->Cursor = sb_temp->Cursor;
	Param->LeftPos = sb_temp->LeftPos;
	Param->ScreenLine = sb_temp->ScreenLine;

	return true;
}

size_t Editor::GetSessionBookmarks(EditorBookmarks *Param)
{
	if (SessionBookmarks.empty())
		return 0;

	if (!Param)
		return SessionBookmarks.size();

	for (const auto& [i, index]: enumerate(SessionBookmarks))
	{
		if (Param->Line)
			Param->Line[index] = i.Line;

		if (Param->Cursor)
			Param->Cursor[index] = i.Cursor;

		if (Param->LeftPos)
			Param->LeftPos[index] = i.LeftPos;

		if (Param->ScreenLine)
			Param->ScreenLine[index] = i.ScreenLine;
	}

	return SessionBookmarks.size();
}

size_t Editor::GetSessionBookmarksForPlugin(EditorBookmarks *Param)
{
	const auto count = GetSessionBookmarks(nullptr);
	size_t size;
	if(InitSessionBookmarksForPlugin(Param,count,size))
		GetSessionBookmarks(Param);
	return size;
}

bool Editor::InitSessionBookmarksForPlugin(EditorBookmarks* Param, size_t Count, size_t& Size)
{
	Size = sizeof(*Param) + sizeof(intptr_t) * 4 * Count;
	if (!Param || Param->Size < Size)
		return false;

	const auto data = std::bit_cast<intptr_t*>(Param + 1);
	Param->Count=Count;
	Param->Line=data;
	Param->Cursor=data+Count;
	Param->ScreenLine=data+2*Count;
	Param->LeftPos=data+3*Count;

	return true;
}

Editor::numbered_iterator Editor::GetStringByNumber(int DestLine)
{
	if (DestLine == m_it_CurLine.Number() || DestLine < 0)
	{
		return m_it_LastGetLine = m_it_CurLine;
	}

	if (static_cast<size_t>(DestLine) >= Lines.size())
	{
		return EndIterator();
	}

	if (!DestLine)
	{
		return m_it_LastGetLine = FirstLine();
	}

	if (static_cast<size_t>(DestLine) == Lines.size() - 1)
	{
		return m_it_LastGetLine = LastLine();
	}

	auto CurPtr = m_it_CurLine;

	if(m_it_LastGetLine != Lines.end())
	{
		if(DestLine==m_it_LastGetLine.Number())
		{
			return m_it_LastGetLine;
		}
		CurPtr = m_it_LastGetLine;
	}

	const auto Forward = (static_cast<size_t>(DestLine) > CurPtr.uNumber() && static_cast<size_t>(DestLine) < CurPtr.uNumber() + (Lines.size() - CurPtr.uNumber()) / 2) || (static_cast<size_t>(DestLine) < CurPtr.uNumber() / 2);

	if (DestLine>CurPtr.Number())
	{
		if(!Forward)
		{
			CurPtr = LastLine();
		}
	}
	else
	{
		if(Forward)
		{
			CurPtr = FirstLine();
		}
	}

	if(Forward)
	{
		for ([[maybe_unused]] const auto Line: std::views::iota(0, DestLine - CurPtr.Number()))
		{
			++CurPtr;
			if (CurPtr == Lines.end())
			{
				m_it_LastGetLine = FirstLine();
				return EndIterator();
			}
		}
	}
	else
	{
		for ([[maybe_unused]] const auto Line: std::views::iota(0, CurPtr.Number() - DestLine))
		{
			if (CurPtr == Lines.begin())
				CurPtr = EndIterator();
			else
				--CurPtr;

			if (CurPtr == Lines.end())
			{
				m_it_LastGetLine = LastLine();
				return EndIterator();
			}
		}
	}

	m_it_LastGetLine = CurPtr;
	return CurPtr;
}

int Editor::GetLineCurPos() const
{
	return m_it_CurLine->GetTabCurPos();
}

void Editor::ProcessVBlockMarking()
{
	if (m_Flags.Check(FEDITOR_CURPOSCHANGEDBYPLUGIN))
	{
		const auto CurPos = m_it_CurLine->GetTabCurPos();
		if (!((m_it_CurLine.Number() == m_it_AnyBlockStart.Number() || m_it_CurLine.Number() == m_it_AnyBlockStart.Number() + VBlockSizeY - 1) && (CurPos == VBlockX || CurPos == VBlockX + VBlockSizeX)))
			TurnOffMarkingBlock();
		m_Flags.Clear(FEDITOR_CURPOSCHANGEDBYPLUGIN);
	}
	if (!m_Flags.Check(FEDITOR_MARKINGVBLOCK))
		BeginVBlockMarking();
}

void Editor::BeginVBlockMarking(const numbered_iterator& Where)
{
	UnmarkBlock();
	m_it_AnyBlockStart = Where;
	VBlockX = Where->GetTabCurPos();
	VBlockSizeX=0;
	VBlockSizeY=1;
	m_Flags.Set(FEDITOR_MARKINGVBLOCK);
	m_BlockType = BTYPE_COLUMN;
}

void Editor::BeginVBlockMarking()
{
	return BeginVBlockMarking(m_it_CurLine);
}

void Editor::BeginStreamMarking(const numbered_iterator& Where)
{
	m_it_AnyBlockStart = Where;
	m_BlockType = BTYPE_STREAM;
	m_Flags.Set(FEDITOR_MARKINGBLOCK);
}

void Editor::AdjustVBlock(int PrevX)
{
	const auto x = GetLineCurPos();

	if (x==VBlockX+VBlockSizeX)   // ничего не случилось, никаких табуляций нет
		return;

	if (x>VBlockX)    // курсор убежал внутрь блока
	{
		VBlockSizeX=x-VBlockX;
	}
	else if (x<VBlockX)   // курсор убежал за начало блока
	{
		const auto c2 = VBlockX;

		if (PrevX>VBlockX)      // сдвигались вправо, а пришли влево
		{
			VBlockX=x;
			VBlockSizeX=c2-x;   // меняем блок
		}
		else      // сдвигались влево и пришли еще больше влево
		{
			VBlockX=x;
			VBlockSizeX+=c2-x;  // расширяем блок
		}
	}
	else if (x==VBlockX && x!=PrevX)
	{
		VBlockSizeX=0;  // ширина в 0, потому прыгнули прям на табуляцию
	}

	// примечание
	//   случай x>VBLockX+VBlockSizeX не может быть
	//   потому что курсор прыгает назад на табуляцию, но не вперед
}


void Editor::Xlat()
{
	const auto XLatStr = [&](Edit::edit_string& Str, int StartPos, int EndPos)
	{
		::Xlat({ Str.data() + StartPos, Str.data() + EndPos }, Global->Opt->XLat.Flags);
	};

	bool DoXlat = false;
	AddUndoData(undo_type::begin);

	if (IsVerticalSelection())
	{
		auto CurPtr = m_it_AnyBlockStart;

		for (int Line = 0; CurPtr != Lines.end() && Line < VBlockSizeY; ++Line, ++CurPtr)
		{
			const size_t TBlockX = CurPtr->VisualPosToReal(VBlockX);
			const size_t TBlockSizeX = CurPtr->VisualPosToReal(VBlockX + VBlockSizeX) - CurPtr->VisualPosToReal(VBlockX);
			size_t CopySize = CurPtr->GetLength() - TBlockX;

			if (CopySize>TBlockSizeX)
				CopySize=TBlockSizeX;

			AddUndoData(undo_type::edit, CurPtr->GetString(), CurPtr->GetEOL(), CurPtr.Number(), m_it_CurLine->GetCurPos());
			XLatStr(CurPtr->m_Str, static_cast<int>(TBlockX), static_cast<int>(TBlockX + CopySize));
			Change(ECTYPE_CHANGED, CurPtr.Number());
		}

		DoXlat = true;
	}
	else
	{
		auto CurPtr = m_it_AnyBlockStart;

		// $ 25.11.2000 IS
		//     Если нет выделения, то обработаем текущее слово. Слово определяется на
		//     основе специальной группы разделителей.
		if (CurPtr != Lines.end())
		{
			while (CurPtr != Lines.end())
			{
				intptr_t StartSel,EndSel;
				CurPtr->GetSelection(StartSel,EndSel);

				if (StartSel==-1)
					break;

				if (EndSel == -1)
					EndSel=CurPtr->GetLength(); //wcslen(CurPtr->Str);

				AddUndoData(undo_type::edit, CurPtr->GetString(), CurPtr->GetEOL(), CurPtr.Number(), m_it_CurLine->GetCurPos());
				XLatStr(CurPtr->m_Str, StartSel, EndSel);
				Change(ECTYPE_CHANGED, CurPtr.Number());
				++CurPtr;
			}

			DoXlat = true;
		}
		else
		{
			auto& Str = m_it_CurLine->m_Str;
			int start = m_it_CurLine->GetCurPos();
			const auto StrSize = m_it_CurLine->GetLength(); //wcslen(Str);
			// $ 10.12.2000 IS
			//   Обрабатываем только то слово, на котором стоит курсор, или то слово,
			//   что находится левее позиции курсора на 1 символ
			DoXlat = true;

			if (IsWordDiv(Global->Opt->XLat.strWordDivForXlat,Str[start]))
			{
				if (start) start--;

				DoXlat = !IsWordDiv(Global->Opt->XLat.strWordDivForXlat,Str[start]);
			}

			if (DoXlat)
			{
				while (start>=0 && !IsWordDiv(Global->Opt->XLat.strWordDivForXlat,Str[start]))
					start--;

				start++;
				int end=start+1;

				while (end<StrSize && !IsWordDiv(Global->Opt->XLat.strWordDivForXlat,Str[end]))
					end++;

				AddUndoData(undo_type::edit, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), m_it_CurLine.Number(), start);
				XLatStr(Str, start, end);
				Change(ECTYPE_CHANGED, m_it_CurLine.Number());
			}
		}
	}

	AddUndoData(undo_type::end);

	if (DoXlat)
		TextChanged(true);
}
/* SVS $ */

/* $ 15.02.2001 IS
     Манипуляции с табуляцией на уровне всего загруженного файла.
     Может быть длительной во времени операцией, но тут уж, imho,
     ничего не поделать.
*/
//Обновим размер табуляции

void Editor::SetOptions(const Options::EditorOptions& Options)
{
	//? optimize
	SetTabSize(Options.TabSize);
	SetConvertTabs(Options.ExpandTabs);
	SetDelRemovesBlocks(Options.DelRemovesBlocks);
	SetShowWhiteSpace(Options.ShowWhiteSpace);
	SetPersistentBlocks(Options.PersistentBlocks);
	SetCursorBeyondEOL(Options.CursorBeyondEOL);

	EdOpt = Options;
}

void Editor::SetTabSize(int NewSize)
{
	if (NewSize<1 || NewSize>512)
		NewSize=8;

	if (NewSize!=EdOpt.TabSize)
	{
		EdOpt.TabSize=NewSize;
	}
}

// обновим режим пробелы вместо табуляции
// операция необратима, кстати, т.е. пробелы на табуляцию обратно не изменятся
void Editor::SetConvertTabs(int NewMode)
{
	if (NewMode != EdOpt.ExpandTabs)
	{
		EdOpt.ExpandTabs=NewMode;
		int Pos=0;

		for (auto& i: Lines)
		{
			if (NewMode == EXPAND_ALLTABS)
			{
				if(i.ReplaceTabs())
				{
					Change(ECTYPE_CHANGED, Pos);
				}
			}
			++Pos;
		}
	}
}

void Editor::SetDelRemovesBlocks(bool NewMode)
{
	if (NewMode!=EdOpt.DelRemovesBlocks)
	{
		EdOpt.DelRemovesBlocks=NewMode;

		for (auto& i: Lines)
		{
			i.SetDelRemovesBlocks(NewMode);
		}
	}
}

void Editor::SetShowWhiteSpace(int NewMode)
{
	if (NewMode!=EdOpt.ShowWhiteSpace)
	{
		EdOpt.ShowWhiteSpace=NewMode;

		for (auto& i: Lines)
		{
			i.SetShowWhiteSpace(NewMode);
		}
	}
}

void Editor::SetPersistentBlocks(bool NewMode)
{
	if (NewMode!=EdOpt.PersistentBlocks)
	{
		EdOpt.PersistentBlocks=NewMode;

		for (auto& i: Lines)
		{
			i.SetPersistentBlocks(NewMode);
		}
	}
}

//     "Курсор за пределами строки"
void Editor::SetCursorBeyondEOL(bool NewMode)
{
	if (NewMode!=EdOpt.CursorBeyondEOL)
	{
		EdOpt.CursorBeyondEOL=NewMode;

		for (auto& i: Lines)
		{
			i.SetEditBeyondEnd(NewMode);
		}
	}
}

void Editor::GetSavePosMode(int &SavePos, int &SaveShortPos) const
{
	SavePos=EdOpt.SavePos;
	SaveShortPos=EdOpt.SaveShortPos;
}

// передавайте в качестве значения параметра "-1" для параметра,
// который не нужно менять
void Editor::SetSavePosMode(int SavePos, int SaveShortPos)
{
	if (SavePos!=-1)
		EdOpt.SavePos = (0 != SavePos);

	if (SaveShortPos!=-1)
		EdOpt.SaveShortPos = (0 != SaveShortPos);
}

Editor::numbered_iterator Editor::InsertString(const string_view Str, const numbered_iterator& Where)
{
	const auto Empty = Lines.empty();

	const auto NewLine = numbered_iterator(Lines.emplace(Where, GetOwner()), Where.Number());

	const auto UpdateIterator = [&Where](numbered_iterator& What)
	{
		if (What.Number() >= Where.Number())
		{
			What.IncrementNumber();
		}
	};

	NewLine->SetPersistentBlocks(EdOpt.PersistentBlocks);

	if (!Str.empty())
		NewLine->SetString(Str);

	NewLine->SetCurPos(0);
	NewLine->SetEditorMode(true);
	NewLine->SetShowWhiteSpace(EdOpt.ShowWhiteSpace);

	if (Empty)
	{
		m_it_TopScreen = m_it_CurLine = NewLine;
	}
	else
	{
		UpdateIterator(m_it_CurLine);
		UpdateIterator(m_it_TopScreen);
		UpdateIterator(m_it_LastGetLine);
		UpdateIterator(m_it_AnyBlockStart);
		UpdateIterator(m_it_MBlockStart);
		UpdateIterator(m_FoundLine);
	}
	return NewLine;
}


void Editor::SetCacheParams(EditorPosCache &pc, bool count_bom)
{
	m_SavePos=pc.bm;
	//m_codepage = pc.CodePage; //BUGBUG!!!, LoadFile do it itself

	if (m_StartLine == -2)  // from Viewer!
	{
		auto CurPtr = FirstLine();
		size_t TotalSize = 0;
		const auto Codepage = GetCodePage();

		switch (Codepage)
		{
		case CP_UTF8:
			if (count_bom)
				StartChar -= 3;
			break;

		case CP_UTF16LE:
		case CP_UTF16BE:
			StartChar /= sizeof(char16_t);
			if (count_bom)
				--StartChar;
			break;
		}

		while (!IsLastLine(CurPtr))
		{
			if (Codepage == CP_UTF8)
			{
				TotalSize += encoding::utf8::get_bytes_count(CurPtr->GetString());
			}
			else
			{
				TotalSize += CurPtr->GetString().size();
			}

			TotalSize += CurPtr->GetEOL().str().size();

			if (static_cast<int>(TotalSize) > StartChar)
				break;

			++CurPtr;
		}

		m_it_TopScreen=m_it_CurLine=CurPtr;

		if (m_it_CurLine.Number() == pc.cur.Line - pc.cur.ScreenLine)
		{
			repeat(pc.cur.ScreenLine, [this](){ bool RefreshMe = false; ProcessKeyInternal(Manager::Key(KEY_DOWN), RefreshMe); });
			m_it_CurLine->SetTabCurPos(pc.cur.LinePos);
		}

		m_it_CurLine->SetLeftPos(pc.cur.LeftPos);
	}
	else if (m_StartLine != -1 || EdOpt.SavePos)
	{
		bool translateTabs=false;
		if (m_StartLine!=-1)
		{
			pc.cur.Line = m_StartLine-1;
			pc.cur.ScreenLine = std::min(ObjHeight() / 2, pc.cur.Line); //ScrY

			pc.cur.LinePos = 0;
			if (StartChar > 0)
			{
				pc.cur.LinePos = StartChar-1;
				translateTabs = true;
			}
		}

		pc.cur.ScreenLine = std::min(pc.cur.ScreenLine, ObjHeight()); //ScrY //BUGBUG

		if (pc.cur.Line >= pc.cur.ScreenLine)
		{
			GoToLine(pc.cur.Line-pc.cur.ScreenLine);
			m_it_TopScreen = m_it_CurLine;

			repeat(pc.cur.ScreenLine, [this](){ bool RefreshMe = false; ProcessKeyInternal(Manager::Key(KEY_DOWN), RefreshMe); });

			if(translateTabs)
				m_it_CurLine->SetCurPos(pc.cur.LinePos);
			else
				m_it_CurLine->SetTabCurPos(pc.cur.LinePos);
			m_it_CurLine->SetLeftPos(pc.cur.LeftPos);
		}
	}
}

void Editor::GetCacheParams(EditorPosCache &pc) const
{
	pc.cur.Line = m_it_CurLine.Number();
	pc.cur.ScreenLine = CalcDistance(m_it_TopScreen, m_it_CurLine);
	pc.cur.LinePos = m_it_CurLine->GetTabCurPos();
	pc.cur.LeftPos = m_it_CurLine->GetLeftPos();
	pc.CodePage = GetCodePage();
	pc.bm=m_SavePos;
}

static std::string_view GetLineBytes(string_view const Str, std::vector<char>& Buffer, uintptr_t const Codepage, encoding::diagnostics* const Diagnostics)
{
	for (;;)
	{
		auto const Length = encoding::get_bytes(Codepage, Str, Buffer, Diagnostics);

		if (Length <= Buffer.size())
			return { Buffer.data(), Length };

		resize_exp(Buffer, Length);
	}
}

bool Editor::SetLineCodePage(Edit& Line, uintptr_t CurrentCodepage, uintptr_t const NewCodepage, bool const Validate)
{
	if (Line.m_Str.empty())
		return true;

	encoding::diagnostics Diagnostics;
	const auto Bytes = GetLineBytes(Line.m_Str, decoded, CurrentCodepage, Validate? &Diagnostics : nullptr);
	auto Result = !Bytes.empty() && !Diagnostics.ErrorPosition;
	encoding::get_chars(NewCodepage, Bytes, Line.m_Str, &Diagnostics);
	Result = Result && !Line.m_Str.empty() && !Diagnostics.ErrorPosition;
	Line.Changed();

	return Result;
}

bool Editor::TryCodePage(uintptr_t const CurrentCodepage, uintptr_t const NewCodepage, uintptr_t& ErrorCodepage, size_t& ErrorLine, size_t& ErrorPos, wchar_t& ErrorChar)
{
	if (CurrentCodepage == NewCodepage)
		return true;

	int LineNumber = 0;

	for (auto i = Lines.begin(), end = Lines.end(); i != end; ++i, ++LineNumber)
	{
		if (i->m_Str.empty())
			continue;

		encoding::diagnostics Diagnostics;
		const auto Bytes = GetLineBytes(i->m_Str, decoded, CurrentCodepage, &Diagnostics);

		if (Bytes.empty() || Diagnostics.ErrorPosition)
		{
			ErrorCodepage = CurrentCodepage;
			ErrorLine = LineNumber;
			ErrorPos = *Diagnostics.ErrorPosition;
			ErrorChar = i->m_Str[ErrorPos];
			return false;
		}

		if (!encoding::get_chars_count(NewCodepage, Bytes, &Diagnostics) || Diagnostics.ErrorPosition)
		{
			ErrorCodepage = NewCodepage;
			ErrorLine = LineNumber;

			// Position is in bytes, we might need to convert it back to chars
			const auto Info = GetCodePageInfo(CurrentCodepage);
			if (Info && Info->MaxCharSize == 1)
			{
				ErrorPos = *Diagnostics.ErrorPosition;
			}
			else
			{
				const auto BytesCount = encoding::get_bytes(CurrentCodepage, i->m_Str, decoded, &Diagnostics);
				ErrorPos = encoding::get_chars_count(CurrentCodepage, { decoded.data(), std::min(*Diagnostics.ErrorPosition, BytesCount) });
			}

			ErrorChar = i->m_Str[ErrorPos];

			return false;
		}
	}

	return true;
}

bool Editor::SetCodePage(uintptr_t const CurrentCodepage, uintptr_t const NewCodepage, bool *BOM, bool ShowMe)
{
	if (CurrentCodepage == NewCodepage)
		return true;

	auto Result = true;

	for(auto& i: Lines)
	{
		if (!SetLineCodePage(i, CurrentCodepage, NewCodepage, Result))
			Result = false;
	}

	if (BOM)
	{
		*BOM = false;
		if (NewCodepage == CP_UTF8 && !Lines.empty())
		{
			auto& first = *Lines.begin();
			if (first.m_Str.starts_with(encoding::bom_char))
			{
				first.m_Str.erase(0, 1);
				*BOM = true;
			}
		}
	}

	if (ShowMe) Show(); //BUGBUG: костыль для того, чтобы не было перерисовки в FileEditor::Init.

	return Result; // BUGBUG, more details?
}

uintptr_t Editor::GetCodePage() const
{
	if (const auto HostFileEditor = std::dynamic_pointer_cast<FileEditor>(m_Owner.lock()))
		return HostFileEditor->GetCodePage();

	throw far_exception(L"HostFileEditor is nullptr"sv);
}

void Editor::AddColor(numbered_iterator const& It, ColorItem const& Item)
{
	m_ColorList[std::to_address(It)].insert(Item);
}

void Editor::DeleteColor(Edit const* It, delete_color_condition const Condition)
{
	const auto ColorsIterator = m_ColorList.find(It);
	if (ColorsIterator == m_ColorList.end())
		return;

	std::erase_if(ColorsIterator->second, Condition);
	if (ColorsIterator->second.empty())
		m_ColorList.erase(ColorsIterator);
}

void Editor::DeleteColor(numbered_iterator const& It, delete_color_condition Condition)
{
	return DeleteColor(std::to_address(It), Condition);
}

bool Editor::GetColor(numbered_iterator const& It, ColorItem& Item, size_t const Index) const
{
	const auto ColorsIterator = m_ColorList.find(std::to_address(It));
	if (ColorsIterator == m_ColorList.cend())
		return false;

	if (Index >= ColorsIterator->second.size())
		return false;

	auto ColorIterator = ColorsIterator->second.begin();
	std::advance(ColorIterator, Index);
	Item = *ColorIterator;
	return true;
}

std::multiset<ColorItem> const* Editor::GetColors(Edit* It) const
{
	const auto ColorsIterator = m_ColorList.find(It);
	if (ColorsIterator == m_ColorList.cend())
		return {};

	return &ColorsIterator->second;
}


void Editor::SetDialogParent(DWORD Sets)
{
}

void Editor::SetOvertypeMode(int Mode)
{
}

bool Editor::GetOvertypeMode() const
{
	return false;
}

void Editor::SetEditBeyondEnd(int Mode)
{
}

void Editor::SetClearFlag(bool Flag)
{
}

bool Editor::GetClearFlag() const
{
	return false;
}

int Editor::GetCurCol() const
{
	return m_it_CurLine->GetCurPos();
}

void Editor::SetCurPos(int NewCol, int NewRow)
{
	GoToLineAndShow(NewRow);
	m_it_CurLine->SetTabCurPos(NewCol);
}

void Editor::DrawScrollbar()
{
	if (EdOpt.ShowScrollBar)
	{
		SetColor(COL_EDITORSCROLLBAR);
		XX2 = m_Where.right - (ScrollBar(m_Where.right, m_Where.top, ObjHeight(), m_it_CurLine.Number() - CalcDistance(m_it_TopScreen, m_it_CurLine), Lines.size())? 1 : 0);
	}
}

void Editor::Change(EDITOR_CHANGETYPE Type,int StrNum)
{
	if (ChangeEventSubscribers.empty())
		return;
	if (StrNum==-1)
		StrNum = m_it_CurLine.Number();
	EditorChange ec{ sizeof(ec), Type, StrNum };
	++EditorControlLock;
	Global->CtrlObject->Plugins->ProcessSubscribedEditorEvent(EE_CHANGE, &ec, this, ChangeEventSubscribers);
	--EditorControlLock;
}

void Editor::TurnOffMarkingBlock()
{
	m_Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
}

Editor::numbered_iterator Editor::EndIterator()
{
	return numbered_iterator(Lines.end(), Lines.size());
}

Editor::numbered_const_iterator Editor::EndIterator() const
{
	return numbered_const_iterator(Lines.end(), Lines.size());
}

Editor::numbered_iterator Editor::FirstLine()
{
	return numbered_iterator(Lines.begin(), 0);
}

Editor::numbered_const_iterator Editor::FirstLine() const
{
	return numbered_const_iterator(Lines.begin(), 0);
}

Editor::numbered_iterator Editor::LastLine()
{
	return std::prev(EndIterator());
}

Editor::numbered_const_iterator Editor::LastLine() const
{
	return std::prev(EndIterator());
}

bool Editor::IsLastLine(const Edit* Line) const
{
	return Line == std::to_address(LastLine());
}

bool Editor::IsLastLine(const iterator& Line) const
{
	return Line == LastLine();
}

void Editor::AutoDeleteColors()
{
	for (const auto& i: m_AutoDeletedColors)
	{
		DeleteColor(i, [](const ColorItem& Item){ return (Item.Flags & ECF_AUTODELETE) != 0; });
	}

	m_AutoDeletedColors.clear();
}
