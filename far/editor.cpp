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

#include "headers.hpp"
#pragma hdrstop

#include "editor.hpp"
#include "edit.hpp"
#include "keyboard.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "dialog.hpp"
#include "fileedit.hpp"
#include "scrbuf.hpp"
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "TaskBar.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "clipboard.hpp"
#include "xlat.hpp"
#include "datetime.hpp"
#include "stddlg.hpp"
#include "strmix.hpp"
#include "FarDlgBuilder.hpp"
#include "wakeful.hpp"
#include "colormix.hpp"
#include "vmenu2.hpp"
#include "codepage.hpp"
#include "DlgGuid.hpp"
#include "RegExp.hpp"
#include "plugins.hpp"
#include "language.hpp"
#include "regex_helpers.hpp"

static bool ReplaceMode, ReplaceAll;

static int EditorID=0;

// EditorUndoData
enum {UNDO_EDIT=1,UNDO_INSSTR,UNDO_DELSTR,UNDO_BEGIN,UNDO_END};

/* $ 04.11.2003 SKV
на любом выходе если была нажата кнопка выделения,
и она его "сняла" (сделала 0-й ширины), то его надо убрать.
*/
class EditorBlockGuard: noncopyable
{
public:
	EditorBlockGuard(Editor& ed, void (Editor::*method)()):
		ed(ed),
		method(method),
		needCheckUnmark(false)
	{}

	~EditorBlockGuard()
	{
		if (needCheckUnmark)(ed.*method)();
	}

	void SetNeedCheckUnmark(bool State) { needCheckUnmark = State; }

private:
	Editor& ed;
	void (Editor::*method)();
	bool needCheckUnmark;
};

const wchar_t* Editor::GetDefaultEOL()
{
	return Global->Opt->EdOpt.NewFileUnixEOL ? UNIX_EOL_fmt : WIN_EOL_fmt;
}

Editor::Editor(window_ptr Owner, bool DialogUsed):
	SimpleScreenObject(Owner),
	m_it_TopScreen(EndIterator()),
	m_it_CurLine(EndIterator()),
	m_it_LastGetLine(EndIterator()),
	UndoPos(UndoData.end()),
	UndoSavePos(UndoData.end()),
	UndoSkipLevel(0),
	LastChangeStrPos(0),
	m_LinesCount(),
	EdOpt(Global->Opt->EdOpt),
	Pasting(0),
	m_it_MBlockStart(EndIterator()),
	m_it_AnyBlockStart(EndIterator()),
	m_BlockType(BTYPE_NONE),
	MBlockStartX(),
	MaxRightPos(0),
	LastSearchCase(Global->GlobalSearchCase),
	LastSearchWholeWords(Global->GlobalSearchWholeWords),
	LastSearchReverse(Global->GlobalSearchReverse),
	LastSearchRegexp(Global->Opt->EdOpt.SearchRegexp),
	LastSearchPreserveStyle(false),
	m_codepage(CP_DEFAULT),
	m_StartLine(-1),
	StartChar(-1),
	SessionPos(SessionBookmarks.end()),
	NewSessionPos(),
	EditorID(::EditorID++),
	HostFileEditor(nullptr),
	SortColorLockCount(0),
	SortColorUpdate(false),
	EditorControlLock(0),
	Color(colors::PaletteColorToFarColor(COL_EDITORTEXT)),
	SelColor(colors::PaletteColorToFarColor(COL_EDITORSELECTEDTEXT)),
	MacroSelectionStart(-1),
	CursorPos(0),
	fake_editor(false),
	m_FoundLine(EndIterator()),
	m_FoundPos(),
	m_FoundSize()
{
	_KEYMACRO(SysLog(L"Editor::Editor()"));
	_KEYMACRO(SysLog(1));

	if (DialogUsed)
		m_Flags.Set(FEDITOR_DIALOGMEMOEDIT);

	if (Global->GetSearchHex())
	{
		const auto Blob = HexStringToBlob(Global->GetSearchString().data(), 0);
		strLastSearchStr.assign(ALL_CONST_RANGE(Blob));
	}
	else
	{
		strLastSearchStr = Global->GetSearchString();
	}
	UnmarkMacroBlock();

	GlobalEOL = GetDefaultEOL();
	PushString(nullptr, 0);
}


Editor::~Editor()
{
	//_SVS(SysLog(L"[%p] Editor::~Editor()",this));
	FreeAllocatedData();
	if (!fake_editor)
		KeepInitParameters();
	_KEYMACRO(SysLog(-1));
	_KEYMACRO(SysLog(L"Editor::~Editor()"));
}

void Editor::FreeAllocatedData()
{
	m_AutoDeletedColors.clear();
	Lines.clear();
	UndoData.clear();
	UndoSavePos = UndoPos = UndoData.end();
	UndoSkipLevel=0;
	ClearSessionBookmarks();
	m_it_AnyBlockStart = m_it_LastGetLine = m_it_CurLine = m_it_TopScreen = EndIterator();
	m_LinesCount = 0;
}

void Editor::SwapState(Editor& swap_state)
{
	// BUGBUGBUG not all fields swapped
	using std::swap;
	Lines.swap(swap_state.Lines);
	swap(m_it_AnyBlockStart, swap_state.m_it_AnyBlockStart);
	swap(m_BlockType, swap_state.m_BlockType);
	swap(m_it_LastGetLine, swap_state.m_it_LastGetLine);
	swap(m_it_TopScreen, swap_state.m_it_TopScreen);
	swap(m_it_CurLine, swap_state.m_it_CurLine);
	swap(m_LinesCount, swap_state.m_LinesCount);

	UndoData.swap(swap_state.UndoData);
	swap(UndoPos, swap_state.UndoPos);
	swap(UndoSavePos, swap_state.UndoSavePos);
	swap(UndoSkipLevel, swap_state.UndoSkipLevel);
	SessionBookmarks.swap(swap_state.SessionBookmarks);
	m_SavePos.swap(swap_state.m_SavePos);
	swap(NewSessionPos, swap_state.NewSessionPos);
	GlobalEOL.swap(swap_state.GlobalEOL);
}

void Editor::KeepInitParameters()
{
	// Установлен глобальный режим поиска 16-ричных данных?
	if (Global->GetSearchHex())
	{
		// BUGBUG, it's unclear how to represent unicode in hex
		const auto AnsiStr = narrow(strLastSearchStr);
		Global->StoreSearchString(BlobToHexWString(AnsiStr.data(), AnsiStr.size(), 0), true);
	}
	else
	{
		Global->StoreSearchString(strLastSearchStr, false);
	}
	Global->GlobalSearchCase=LastSearchCase;
	Global->GlobalSearchWholeWords=LastSearchWholeWords;
	Global->GlobalSearchReverse=LastSearchReverse;
	Global->Opt->EdOpt.SearchRegexp=LastSearchRegexp;
}

void Editor::DisplayObject()
{
	ShowEditor();
}

void Editor::ShowEditor()
{
	if (Locked() || Lines.empty())
		return;

	Color = colors::PaletteColorToFarColor(COL_EDITORTEXT);
	SelColor = colors::PaletteColorToFarColor(COL_EDITORSELECTEDTEXT);

	int LeftPos,CurPos,Y;

	XX2=m_X2 - (EdOpt.ShowScrollBar && ScrollBarRequired(ObjHeight(), m_LinesCount) ? 1 : 0);
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
		//DisableOut=TRUE;
		//ProcessKey(KEY_UP);
		//DisableOut=FALSE;
	}

	CurPos = m_it_CurLine->GetTabCurPos();

	if (!EdOpt.CursorBeyondEOL)
	{
		MaxRightPos=CurPos;
		int RealCurPos=m_it_CurLine->GetCurPos();
		int Length=m_it_CurLine->GetLength();

		if (RealCurPos>Length)
		{
			m_it_CurLine->SetCurPos(Length);
			m_it_CurLine->SetLeftPos(0);
			CurPos=m_it_CurLine->GetTabCurPos();
		}
	}

	//---
	//для корректной отрисовки текста с табами у CurLine должна быть корректная LeftPos до начала отрисовки
	//так же это позволяет возвращать корректную EditorInfo.LeftPos в EE_REDRAW
	m_it_CurLine->SetHorizontalPosition(m_X1, XX2);
	m_it_CurLine->FixLeftPos();
	//---
	if (!Pasting)
	{
		/*$ 10.08.2000 skv
		  Don't send EE_REDRAW while macro is being executed.

		*/
		_SYS_EE_REDRAW(CleverSysLog Clev(L"Editor::ShowEditor()"));

		if (!Global->ScrBuf->GetLockCount())
		{
			if (!m_Flags.Check(FEDITOR_DIALOGMEMOEDIT))
			{
				_SYS_EE_REDRAW(SysLog(L"Call ProcessEditorEvent(EE_REDRAW)"));
				SortColorLock();
				Global->CtrlObject->Plugins->ProcessEditorEvent(EE_REDRAW, EEREDRAW_ALL, this);
				SortColorUnlock();
			}
		}
		_SYS_EE_REDRAW(else SysLog(L"ScrBuf Locked !!!"));
	}

	DrawScrollbar();

	LeftPos=m_it_CurLine->GetLeftPos();
#if 0

	// крайне экспериментальный кусок!
	if (CurPos+LeftPos < XX2)
		LeftPos=0;
	else if (CurLine->X2 < XX2)
		LeftPos=CurLine->GetLength()-CurPos;

	if (LeftPos < 0)
		LeftPos=0;

#endif
	Edit::ShowInfo info={LeftPos,CurPos};
	Y = m_Y1;
	for (auto CurPtr=m_it_TopScreen; Y<=m_Y2; Y++)
	{
		if (CurPtr != Lines.end())
		{
			CurPtr->SetEditBeyondEnd(true);
			CurPtr->SetPosition(m_X1,Y,XX2,Y);
			//CurPtr->SetTables(UseDecodeTable ? &TableSet:nullptr);
			//_D(SysLog(L"Setleftpos 3 to %i",LeftPos));
			CurPtr->SetLeftPos(LeftPos);
			CurPtr->SetTabCurPos(CurPos);
			CurPtr->SetEditBeyondEnd(EdOpt.CursorBeyondEOL);
			if(CurPtr==m_it_CurLine)
			{
				CurPtr->SetOvertypeMode(m_Flags.Check(FEDITOR_OVERTYPE));
				CurPtr->Show();
			}
			else
				CurPtr->FastShow(&info);
			++CurPtr;
		}
		else
		{
			SetScreen(m_X1,Y,XX2,Y,L' ',colors::PaletteColorToFarColor(COL_EDITORTEXT)); //Пустые строки после конца текста
		}
	}

	if (IsVerticalSelection() && VBlockSizeX > 0 && VBlockSizeY > 0)
	{
		int CurScreenLine = static_cast<int>(m_it_CurLine.Number() - CalcDistance(m_it_TopScreen,m_it_CurLine));
		LeftPos=m_it_CurLine->GetLeftPos();

		Y = m_Y1;
		for (auto CurPtr=m_it_TopScreen; Y<=m_Y2; Y++)
		{
			if (CurPtr != Lines.end())
			{
				if (CurScreenLine >= m_it_AnyBlockStart.Number() && CurScreenLine < m_it_AnyBlockStart.Number() + VBlockSizeY)
				{
					int BlockX1=VBlockX-LeftPos+m_X1;
					int BlockX2=VBlockX+VBlockSizeX-1-LeftPos+m_X1;

					if (BlockX1<m_X1)
						BlockX1=m_X1;

					if (BlockX2>XX2)
						BlockX2=XX2;

					if (BlockX1<=XX2 && BlockX2>=m_X1)
						ChangeBlockColor(BlockX1,Y,BlockX2,Y,colors::PaletteColorToFarColor(COL_EDITORSELECTEDTEXT));
				}

				++CurPtr;
				CurScreenLine++;
			}
		}
	}

	if (HostFileEditor) HostFileEditor->ShowStatus();

//_SVS(SysLog(L"Exit from ShowEditor"));
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
			if (IsVerticalSelection())
				*Pos = m_it_AnyBlockStart->RealPosToTab(m_it_AnyBlockStart->TabPosToReal(VBlockX));
			else
				*Pos = m_it_AnyBlockStart->RealPosToTab(m_it_AnyBlockStart->m_SelStart);
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
				iPos=eLine->RealPosToTab(eLine->TabPosToReal(VBlockX+VBlockSizeX));
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
					iPos=eLine->RealPosToTab(EndSel);
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

__int64 Editor::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	int CurPos=m_it_CurLine->GetCurPos();

	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			// Editor always has at least one line
			return Lines.empty() || (m_LinesCount == 1 && Lines.front().m_Str.empty());
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
			return m_LinesCount;
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
			return GotoSessionBookmark((int)iParam-1);
		case MCODE_F_BM_GET:                   // N=BM.Get(Idx,M) - возвращает координаты строки (M==0) или колонки (M==1) закладки с индексом (Idx=1...)
		{
			__int64 Ret=-1;
			InternalEditorBookmark ebm = {};
			const auto iMode = reinterpret_cast<intptr_t>(vParam);

			if (iMode >= 0 && iMode <= 3 && GetSessionBookmark((int)iParam-1,&ebm))
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
			return DeleteSessionBookmark(PointerToSessionBookmark((int)iParam - 1));
		}
		case MCODE_F_EDITOR_SEL:
		{
			int iPos;
			const auto Action = reinterpret_cast<intptr_t>(vParam);

			switch (Action)
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
								const auto _NumLine = m_it_CurLine.Number();
								int _CurPos=m_it_CurLine->GetTabCurPos();

								GoToLine(iLine);
								m_it_CurLine->SetCurPos(m_it_CurLine->TabPosToReal(iPos));

								if (!EdOpt.CursorBeyondEOL && m_it_CurLine->GetCurPos() > m_it_CurLine->GetLength())
								{
									GoToLine(_NumLine);
									m_it_CurLine->SetCurPos(m_it_CurLine->TabPosToReal(_CurPos));
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
								EditorSelect eSel={sizeof(EditorSelect)};
								eSel.BlockType=(Action == 2)?BTYPE_STREAM:BTYPE_COLUMN;
								eSel.BlockStartPos=MBlockStartX;
								eSel.BlockWidth=m_it_CurLine->GetCurPos()-MBlockStartX;

								if (!eSel.BlockWidth && m_it_MBlockStart == m_it_CurLine)
								{
									UnmarkBlock();
								}
								else
								{
									int bl = CalcDistance(FirstLine(), m_it_MBlockStart);
									int el = CalcDistance(FirstLine(), m_it_CurLine);

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
					bool NeedRedraw = IsAnySelection();
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
				_ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
				return 0;
			}

			int DestLine=iParam;

			if (DestLine<0)
				DestLine = m_it_CurLine.Number();

			const auto EditPtr = GetStringByNumber(DestLine);

			if (EditPtr == Lines.end())
			{
				_ECTLLOG(SysLog(L"VMProcess(MCODE_F_EDITOR_*,...) => GetStringByNumber(%d) return nullptr",DestLine));
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
					const auto NewEditPtr = InsertString((const wchar_t *)vParam, StrLength((const wchar_t *)vParam), std::next(EditPtr));
					NewEditPtr->SetEOL(EditPtr->GetEOL());
					AddUndoData(UNDO_INSSTR, NewEditPtr->GetString(), EditPtr->GetEOL(), DestLine,0);
					Change(ECTYPE_ADDED,DestLine+1);
					TextChanged(1);
					return 1;
				}
				case MCODE_F_EDITOR_SETSTR:  // N=Editor.SetStr([S[,Line]])
				{
					string strEOL=EditPtr->GetEOL();
					AddUndoData(UNDO_EDIT, EditPtr->GetString(), strEOL.data(), DestLine, EditPtr->GetCurPos());
					const auto Str = static_cast<const wchar_t*>(vParam);
					EditPtr->SetString(Str, wcslen(Str));
					EditPtr->SetEOL(strEOL);
					Change(ECTYPE_CHANGED,DestLine);
					TextChanged(1);
					return 1;
				}
			}
		}
		case MCODE_V_EDITORSELVALUE: // Editor.SelValue
		{
			*reinterpret_cast<string*>(vParam) = IsVerticalSelection()? VBlock2Text() : Block2Text();
			return 1;
		}
	}

	return 0;
}


int Editor::ProcessKey(const Manager::Key& Key)
{
	auto LocalKey = Key;
	if (LocalKey()==KEY_IDLE)
	{
		if (Global->Opt->ViewerEditorClock && HostFileEditor && HostFileEditor->IsFullScreen() && Global->Opt->EdOpt.ShowTitleBar)
			ShowTime(FALSE);

		return TRUE;
	}

	if (LocalKey()==KEY_NONE)
		return TRUE;

	switch (LocalKey())
	{
		case KEY_CTRLSHIFTUP:   case KEY_CTRLSHIFTNUMPAD8: LocalKey = KEY_SHIFTUP;   break;
		case KEY_CTRLSHIFTDOWN:	case KEY_CTRLSHIFTNUMPAD2: LocalKey = KEY_SHIFTDOWN; break;

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

	_KEYMACRO(CleverSysLog SL(L"Editor::ProcessKey()"));
	_KEYMACRO(SysLog(L"Key=%s",_FARKEY_ToName(LocalKey())));
	int CurPos=m_it_CurLine->GetCurPos();
	int CurVisPos=GetLineCurPos();
	int isk = IsShiftKey(LocalKey());
	int ick = (LocalKey()==KEY_CTRLC || LocalKey()==KEY_RCTRLC || LocalKey()==KEY_CTRLINS || LocalKey()==KEY_CTRLNUMPAD0 || LocalKey()==KEY_RCTRLINS || LocalKey()==KEY_RCTRLNUMPAD0);
	int imk = ((unsigned int)LocalKey() >= KEY_MACRO_BASE && (unsigned int)LocalKey() <= KEY_MACRO_ENDBASE);
	int ipk = ((unsigned int)LocalKey() >= KEY_OP_BASE && (unsigned int)LocalKey() <= KEY_OP_ENDBASE);

	_SVS(SysLog(L"[%d] isk=%d",__LINE__,isk));

	//if ((!isk || Global->CtrlObject->Macro.IsExecuting()) && !isk && !Pasting)
	if (!isk && !Pasting && !ick && !imk && !ipk )
	{
		_SVS(SysLog(L"[%d] BlockStart=(%d,%d)",__LINE__,m_it_BlockStart,m_it_VBlockStart));

		if (IsAnySelection())
		{
			TurnOffMarkingBlock();
		}

		if (IsAnySelection() && !EdOpt.PersistentBlocks)
//    if (BlockStart || VBlockStart && !EdOpt.PersistentBlocks)
		{
			TurnOffMarkingBlock();

			if (!EdOpt.PersistentBlocks)
			{
				static int UnmarkKeys[]=
				{
					KEY_LEFT,      KEY_NUMPAD4,
					KEY_RIGHT,     KEY_NUMPAD6,
					KEY_HOME,      KEY_NUMPAD7,
					KEY_END,       KEY_NUMPAD1,
					KEY_UP,        KEY_NUMPAD8,
					KEY_DOWN,      KEY_NUMPAD2,
					KEY_PGUP,      KEY_NUMPAD9,
					KEY_PGDN,      KEY_NUMPAD3,
					KEY_CTRLHOME,  KEY_RCTRLHOME,  KEY_CTRLNUMPAD7,  KEY_RCTRLNUMPAD7,
					KEY_CTRLPGUP,  KEY_RCTRLPGUP,  KEY_CTRLNUMPAD9,  KEY_RCTRLNUMPAD9,
					KEY_CTRLEND,   KEY_RCTRLEND,   KEY_CTRLNUMPAD1,  KEY_RCTRLNUMPAD1,
					KEY_CTRLPGDN,  KEY_RCTRLPGDN,  KEY_CTRLNUMPAD3,  KEY_RCTRLNUMPAD3,
					KEY_CTRLLEFT,  KEY_RCTRLLEFT,  KEY_CTRLNUMPAD4,  KEY_RCTRLNUMPAD4,
					KEY_CTRLRIGHT, KEY_RCTRLRIGHT, KEY_CTRLNUMPAD7,  KEY_RCTRLNUMPAD7,
					KEY_CTRLUP,    KEY_RCTRLUP,    KEY_CTRLNUMPAD8,  KEY_RCTRLNUMPAD8,
					KEY_CTRLDOWN,  KEY_RCTRLDOWN,  KEY_CTRLNUMPAD2,  KEY_RCTRLNUMPAD2,
					KEY_CTRLN,     KEY_RCTRLN,
					KEY_CTRLE,     KEY_RCTRLE,
					KEY_CTRLS,     KEY_RCTRLS,
				};

				if (std::find(ALL_CONST_RANGE(UnmarkKeys), LocalKey()) != std::cend(UnmarkKeys))
				{
					UnmarkBlock();
				}
			}
			else
			{
				intptr_t StartSel,EndSel;
//        Edit *BStart=!BlockStart?VBlockStart:BlockStart;
//        BStart->GetRealSelection(StartSel,EndSel);
				m_it_AnyBlockStart->GetRealSelection(StartSel,EndSel);
				_SVS(SysLog(L"[%d] PersistentBlocks! StartSel=%d, EndSel=%d",__LINE__,StartSel,EndSel));

				if (StartSel==-1 || StartSel==EndSel)
					UnmarkBlock();
			}
		}
	}

	if (LocalKey()==KEY_ALTD || LocalKey()==KEY_RALTD)
		LocalKey=KEY_CTRLK;

	// работа с закладками
	if (LocalKey()>=KEY_CTRL0 && LocalKey()<=KEY_CTRL9)
		return GotoBookmark(LocalKey()-KEY_CTRL0);

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
			_KEYMACRO(CleverSysLog SL(L"Editor::ProcessKey(KEY_SHIFT*)"));
			_SVS(SysLog(L"[%d] SelStart=%d, SelEnd=%d",__LINE__,SelStart,SelEnd));
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

			_SVS(SysLog(L"[%d] SelStart=%d, SelEnd=%d",__LINE__,SelStart,SelEnd));

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

			_SVS(SysLog(L"[%d] SelStart=%d, SelEnd=%d",__LINE__,SelStart,SelEnd));
		}
	}

	switch (LocalKey())
	{
		case KEY_CTRLSHIFTPGUP:   case KEY_CTRLSHIFTNUMPAD9:
		case KEY_RCTRLSHIFTPGUP:  case KEY_RCTRLSHIFTNUMPAD9:
		case KEY_CTRLSHIFTHOME:   case KEY_CTRLSHIFTNUMPAD7:
		case KEY_RCTRLSHIFTHOME:  case KEY_RCTRLSHIFTNUMPAD7:
		{
			Lock();
			Pasting++;

			while (m_it_CurLine != Lines.begin())
			{
				ProcessKey(Manager::Key(KEY_SHIFTPGUP));
			}

			if (LocalKey() == KEY_CTRLSHIFTHOME || LocalKey() == KEY_CTRLSHIFTNUMPAD7 || LocalKey() == KEY_RCTRLSHIFTHOME || LocalKey() == KEY_RCTRLSHIFTNUMPAD7)
				ProcessKey(Manager::Key(KEY_SHIFTHOME));

			Pasting--;
			Unlock();
			Show();
			return TRUE;
		}
		case KEY_CTRLSHIFTPGDN:   case KEY_CTRLSHIFTNUMPAD3:
		case KEY_RCTRLSHIFTPGDN:  case KEY_RCTRLSHIFTNUMPAD3:
		case KEY_CTRLSHIFTEND:    case KEY_CTRLSHIFTNUMPAD1:
		case KEY_RCTRLSHIFTEND:   case KEY_RCTRLSHIFTNUMPAD1:
		{
			Lock();
			Pasting++;

			while (!IsLastLine(m_it_CurLine))
			{
				ProcessKey(Manager::Key(KEY_SHIFTPGDN));
			}

			if (LocalKey() == KEY_CTRLSHIFTEND || LocalKey() == KEY_CTRLSHIFTNUMPAD1 || LocalKey() == KEY_RCTRLSHIFTEND || LocalKey() == KEY_RCTRLSHIFTNUMPAD1)
				ProcessKey(Manager::Key(KEY_SHIFTEND));

			Pasting--;
			Unlock();
			Show();
			return TRUE;
		}
		case KEY_SHIFTPGUP:       case KEY_SHIFTNUMPAD9:
		{
			Pasting++;
			Lock();

			repeat(m_Y2 - m_Y1, [this]
			{
				ProcessKey(Manager::Key(KEY_SHIFTUP));

				if (!EdOpt.CursorBeyondEOL)
				{
					if (m_it_CurLine->GetCurPos()>m_it_CurLine->GetLength())
					{
						m_it_CurLine->SetCurPos(m_it_CurLine->GetLength());
					}
				}
			});

			Pasting--;
			Unlock();
			Show();
			return TRUE;
		}
		case KEY_SHIFTPGDN:       case KEY_SHIFTNUMPAD3:
		{
			Pasting++;
			Lock();

			repeat(m_Y2 - m_Y1, [this]
			{
				ProcessKey(Manager::Key(KEY_SHIFTDOWN));

				if (!EdOpt.CursorBeyondEOL)
				{
					if (m_it_CurLine->GetCurPos()>m_it_CurLine->GetLength())
					{
						m_it_CurLine->SetCurPos(m_it_CurLine->GetLength());
					}
				}
			});

			Pasting--;
			Unlock();
			Show();
			return TRUE;
		}
		case KEY_SHIFTHOME:       case KEY_SHIFTNUMPAD7:
		{
			Pasting++;
			Lock();
			m_it_CurLine->Select(0,SelAtBeginning?SelEnd:SelStart);
			ProcessKey(Manager::Key(KEY_HOME));
			Pasting--;
			Unlock();
			Show();
			return TRUE;
		}
		case KEY_SHIFTEND:
		case KEY_SHIFTNUMPAD1:
		{
			{
				Pasting++;
				Lock();
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
				ProcessKey(Manager::Key(KEY_END));
				Pasting--;
				Unlock();

				if (EdOpt.PersistentBlocks)
					Show();
				else
				{
					m_it_CurLine->FastShow();
					ShowEditor();
				}
			}
			return TRUE;
		}
		case KEY_SHIFTLEFT:  case KEY_SHIFTNUMPAD4:
		{
			_SVS(CleverSysLog SL(L"case KEY_SHIFTLEFT"));

			if (!CurPos && m_it_CurLine == Lines.begin())
				return TRUE;

			if (!CurPos) //курсор в начале строки
			{
				const auto PrevLine = std::prev(m_it_CurLine);
				if (SelAtBeginning) //курсор в начале блока
				{
					m_it_AnyBlockStart = PrevLine;
					PrevLine->Select(PrevLine->GetLength(), -1);
				}
				else // курсор в конце блока
				{
					m_it_CurLine->RemoveSelection();
					PrevLine->GetRealSelection(SelStart, SelEnd);
					PrevLine->Select(SelStart, PrevLine->GetLength());
				}
			}
			else
			{
				if (SelAtBeginning || SelFirst)
				{
					m_it_CurLine->Select(SelStart-1,SelEnd);
				}
				else
				{
					m_it_CurLine->Select(SelStart,SelEnd-1);
				}
			}

			Pasting++;
			ProcessKey(Manager::Key(KEY_LEFT));
			Pasting--;

			ShowEditor();
			return TRUE;
		}
		case KEY_SHIFTRIGHT:  case KEY_SHIFTNUMPAD6:
		{
			_SVS(CleverSysLog SL(L"case KEY_SHIFTRIGHT"));

			if (IsLastLine(m_it_CurLine) && CurPos == m_it_CurLine->GetLength() && !EdOpt.CursorBeyondEOL)
			{
				return TRUE;
			}

			if (SelAtBeginning)
			{
				m_it_CurLine->Select(SelStart+1,SelEnd);
			}
			else
			{
				m_it_CurLine->Select(SelStart,SelEnd+1);
			}

			const auto OldCur = m_it_CurLine;
			Pasting++;
			ProcessKey(Manager::Key(KEY_RIGHT));
			Pasting--;

			if (OldCur != m_it_CurLine)
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

			ShowEditor();
			return TRUE;
		}
		case KEY_CTRLSHIFTLEFT:  case KEY_CTRLSHIFTNUMPAD4:
		case KEY_RCTRLSHIFTLEFT: case KEY_RCTRLSHIFTNUMPAD4:
		{
			_SVS(CleverSysLog SL(L"case KEY_CTRLSHIFTLEFT"));
			_SVS(SysLog(L"[%d] Pasting=%d, SelEnd=%d",__LINE__,Pasting,SelEnd));
			{
				int SkipSpace=TRUE;
				Pasting++;
				Lock();

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

					if (IsSpace(Str[LocalCurPos-1]) || IsWordDiv(EdOpt.strWordDiv,Str[LocalCurPos-1]))
					{
						if (SkipSpace)
						{
							ProcessKey(Manager::Key(KEY_SHIFTLEFT));
							continue;
						}
						else
							break;
					}

					SkipSpace=FALSE;
					ProcessKey(Manager::Key(KEY_SHIFTLEFT));
				}

				Pasting--;
				Unlock();
				Show();
			}
			return TRUE;
		}
		case KEY_CTRLSHIFTRIGHT:  case KEY_CTRLSHIFTNUMPAD6:
		case KEY_RCTRLSHIFTRIGHT: case KEY_RCTRLSHIFTNUMPAD6:
		{
			_SVS(CleverSysLog SL(L"case KEY_CTRLSHIFTRIGHT"));
			_SVS(SysLog(L"[%d] Pasting=%d, SelEnd=%d",__LINE__,Pasting,SelEnd));
			{
				int SkipSpace=TRUE;
				Pasting++;
				Lock();

				for (;;)
				{
					const auto& Str = m_it_CurLine->GetString();
					const size_t LocalCurPos = m_it_CurLine->GetCurPos();

					if (LocalCurPos >= Str.size())
						break;

					if (IsSpace(Str[LocalCurPos]) || IsWordDiv(EdOpt.strWordDiv, Str[LocalCurPos]))
					{
						if (SkipSpace)
						{
							ProcessKey(Manager::Key(KEY_SHIFTRIGHT));
							continue;
						}
						else
							break;
					}

					SkipSpace=FALSE;
					ProcessKey(Manager::Key(KEY_SHIFTRIGHT));
				}

				Pasting--;
				Unlock();
				Show();
			}
			return TRUE;
		}
		case KEY_SHIFTDOWN:  case KEY_SHIFTNUMPAD2:
		{
			const auto NextLine = std::next(m_it_CurLine);
			if (NextLine == Lines.end())
				return TRUE;

			CurPos = m_it_CurLine->RealPosToTab(CurPos);

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
					SelStart = NextLine->RealPosToTab(SelStart);

				if (SelEnd!=-1)
					SelEnd = NextLine->RealPosToTab(SelEnd);

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
					SelStart = NextLine->TabPosToReal(SelStart);

				if (SelEnd!=-1)
					SelEnd = NextLine->TabPosToReal(SelEnd);

				/*
				if(!EdOpt.CursorBeyondEOL && SelEnd > NextLine->GetLength())
				{
					SelEnd = NextLine->GetLength();
				}
				if(!EdOpt.CursorBeyondEOL && SelStart > NextLine->GetLength())
				{
					SelStart = NextLine->GetLength();
				}
				*/
			}
			else //расширяем выделение
			{
				m_it_CurLine->Select(SelStart,-1);
				SelStart=0;
				SelEnd=CurPos;

				if (SelStart!=-1)
					SelStart = NextLine->TabPosToReal(SelStart);

				if (SelEnd!=-1)
					SelEnd = NextLine->TabPosToReal(SelEnd);
			}

			if (!EdOpt.CursorBeyondEOL && SelEnd > NextLine->GetLength())
			{
				SelEnd = NextLine->GetLength();
			}

			if (!EdOpt.CursorBeyondEOL && SelStart > NextLine->GetLength())
			{
				SelStart = NextLine->GetLength();
			}

			/*
			if(!SelStart && !SelEnd)
				NextLine->RemoveSelection();
			else
			*/
				NextLine->Select(SelStart,SelEnd);
			Down();
			Show();
			return TRUE;
		}
		case KEY_SHIFTUP: case KEY_SHIFTNUMPAD8:
		{
			if (m_it_CurLine == Lines.begin())
				return 0;

			const auto PrevLine = std::prev(m_it_CurLine);
			if (SelAtBeginning || SelFirst) // расширяем выделение
			{
				m_it_CurLine->Select(0,SelEnd);
				SelStart=m_it_CurLine->RealPosToTab(CurPos);

				if (!EdOpt.CursorBeyondEOL && PrevLine->TabPosToReal(SelStart) > PrevLine->GetLength())
				{
					SelStart = PrevLine->RealPosToTab(PrevLine->GetLength());
				}

				SelStart = PrevLine->TabPosToReal(SelStart);
				PrevLine->Select(SelStart, -1);
				m_it_AnyBlockStart = PrevLine;
			}
			else // снимаем выделение
			{
				CurPos=m_it_CurLine->RealPosToTab(CurPos);

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
					SelStart = PrevLine->RealPosToTab(SelStart);

				if (SelStart != -1)
					SelEnd = PrevLine->RealPosToTab(SelEnd);

				if (SelStart==-1)
				{
					m_it_AnyBlockStart = PrevLine;
					SelStart = PrevLine->TabPosToReal(CurPos);
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

					SelStart = PrevLine->TabPosToReal(SelStart);
					SelEnd = PrevLine->TabPosToReal(SelEnd);

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
			Show();
			return TRUE;
		}
		case KEY_CTRLADD:
		case KEY_RCTRLADD:
		{
			Copy(TRUE);
			return TRUE;
		}
		case KEY_CTRLA:
		case KEY_RCTRLA:
		{
			UnmarkBlock();
			SelectAll();
			return TRUE;
		}
		case KEY_CTRLU:
		case KEY_RCTRLU:
		{
			UnmarkMacroBlock();
			UnmarkBlock();
			return TRUE;
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
				Show();
			}

			Copy(FALSE);
			return TRUE;
		}
		case KEY_CTRLP:
		case KEY_RCTRLP:
		case KEY_CTRLM:
		case KEY_RCTRLM:
		{
			if (m_Flags.Check(FEDITOR_LOCKMODE))
				return TRUE;

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
						CurSelStart = m_it_CurLine->TabPosToReal(VBlockX);
						CurSelEnd = m_it_CurLine->TabPosToReal(VBlockX + VBlockSizeX);
					}
				}

				Pasting++;

				// BUGBUG, TODO
				// Using an internal clipboard to copy/move block is a not a best design choice:
				// it's accessible through macros and someone might keep something there.
				// We should either implement it without using the clipboard,
				// or substitute a new local instance of internal_clipboard for this purpose only.

				const auto SavedClipboardMode = default_clipboard_mode::get();
				default_clipboard_mode::set(default_clipboard_mode::internal);

				ProcessKey(Manager::Key((LocalKey()==KEY_CTRLP || LocalKey()==KEY_RCTRLP) ? KEY_CTRLINS:KEY_SHIFTDEL));

				/* $ 10.04.2001 SVS
				  ^P/^M - некорректно работали: условие для CurPos должно быть ">=",
				   а не "меньше".
				*/
				if ((LocalKey() == KEY_CTRLM || LocalKey() == KEY_RCTRLM) && CurSelStart != -1 && CurSelEnd != -1)
				{
					if (CurPos >= CurSelEnd)
						m_it_CurLine->SetCurPos(CurPos - (CurSelEnd - CurSelStart));
					else
						m_it_CurLine->SetCurPos(CurPos);
				}

				ProcessKey(Manager::Key(KEY_SHIFTINS));
				Pasting--;
				ClearInternalClipboard();

				default_clipboard_mode::set(SavedClipboardMode);

				/*$ 08.02.2001 SKV
				  всё делалось с pasting'ом, поэтому redraw плагинам не ушел.
				  сделаем его.
				*/
				Show();
			}

			return TRUE;
		}
		case KEY_CTRLX:
		case KEY_RCTRLX:
		case KEY_SHIFTDEL:
		case KEY_SHIFTNUMDEL:
		case KEY_SHIFTDECIMAL:
		{
			Copy(FALSE);
		}
		case KEY_CTRLD:
		case KEY_RCTRLD:
		{
			if (m_Flags.Check(FEDITOR_LOCKMODE))
				return TRUE;

			TurnOffMarkingBlock();
			DeleteBlock();
			Show();
			return TRUE;
		}
		case KEY_CTRLV:
		case KEY_RCTRLV:
		case KEY_SHIFTINS: case KEY_SHIFTNUMPAD0:
		{
			if (m_Flags.Check(FEDITOR_LOCKMODE))
				return TRUE;

			Pasting++;

			if (!EdOpt.PersistentBlocks && !IsVerticalSelection())
				DeleteBlock();

			PasteFromClipboard();
			// MarkingBlock=!VBlockStart;
			m_Flags.Change(FEDITOR_MARKINGBLOCK, IsStreamSelection());
			m_Flags.Clear(FEDITOR_MARKINGVBLOCK);

			if (!EdOpt.PersistentBlocks)
				UnmarkBlock();

			Pasting--;
			Show();
			return TRUE;
		}
		case KEY_LEFT: case KEY_NUMPAD4:
		{
			m_Flags.Set(FEDITOR_NEWUNDO);

			if (!CurPos && m_it_CurLine != Lines.begin())
			{
				Up();
				Show();
				m_it_CurLine->ProcessKey(Manager::Key(KEY_END));
				Show();
			}
			else
			{
				m_it_CurLine->ProcessKey(Manager::Key(KEY_LEFT));
				ShowEditor();
			}

			return TRUE;
		}
		case KEY_INS: case KEY_NUMPAD0:
		{
			m_Flags.Swap(FEDITOR_OVERTYPE);
			Show();
			return TRUE;
		}
		case KEY_NUMDEL:
		case KEY_DEL:
		{
			if (!m_Flags.Check(FEDITOR_LOCKMODE))
			{
				// Del в самой последней позиции ничего не удаляет, поэтому не модифицируем...
				if (IsLastLine(m_it_CurLine) && CurPos >= m_it_CurLine->GetLength() && !IsAnySelection())
					return TRUE;

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
						AddUndoData(UNDO_BEGIN);
						AddUndoData(UNDO_EDIT, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), m_it_CurLine.Number(), m_it_CurLine->GetCurPos());

						const auto NextLine = std::next(m_it_CurLine);
						if (NextLine == Lines.end())
						{
							m_it_CurLine->SetEOL(L"");
						}
						else
						{
							intptr_t CurSelStart, CurSelEnd, NextSelStart, NextSelEnd;
							int Length=m_it_CurLine->GetLength();
							m_it_CurLine->GetSelection(CurSelStart, CurSelEnd);
							NextLine->GetSelection(NextSelStart, NextSelEnd);
							const wchar_t *NextEOL = NextLine->GetEOL();
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

						AddUndoData(UNDO_END);
					}
					else
					{
						AddUndoData(UNDO_EDIT, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), m_it_CurLine.Number(), m_it_CurLine->GetCurPos());
						m_it_CurLine->ProcessKey(Manager::Key(KEY_DEL));
					}
					Change(ECTYPE_CHANGED, m_it_CurLine.Number());
					TextChanged(1);
				}

				Show();
			}

			return TRUE;
		}
		case KEY_BS:
		{
			if (!m_Flags.Check(FEDITOR_LOCKMODE))
			{
				// Bs в самом начале нихрена ничего не удаляет, посему не будем выставлять
				if (m_it_CurLine == Lines.begin() && !CurPos && !IsAnySelection())
					return TRUE;

				TextChanged(1);
				int IsDelBlock=FALSE;

				if (EdOpt.BSLikeDel)
				{
					if (!Pasting && EdOpt.DelRemovesBlocks && IsAnySelection())
						IsDelBlock=TRUE;
				}
				else
				{
					if (!Pasting && !EdOpt.PersistentBlocks && IsStreamSelection())
						IsDelBlock=TRUE;
				}

				if (IsDelBlock)
					DeleteBlock();
				else if (!CurPos && m_it_CurLine != Lines.begin())
				{
					Pasting++;
					Up();
					m_it_CurLine->ProcessKey(Manager::Key(KEY_CTRLEND));
					ProcessKey(Manager::Key(KEY_DEL));
					Pasting--;
				}
				else
				{
					AddUndoData(UNDO_EDIT, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), m_it_CurLine.Number(), m_it_CurLine->GetCurPos());
					m_it_CurLine->ProcessKey(Manager::Key(KEY_BS));
					Change(ECTYPE_CHANGED, m_it_CurLine.Number());
				}

				Show();
			}

			return TRUE;
		}
		case KEY_CTRLBS:
		case KEY_RCTRLBS:
		{
			if (!m_Flags.Check(FEDITOR_LOCKMODE))
			{
				TextChanged(1);

				if (!Pasting && !EdOpt.PersistentBlocks && IsStreamSelection())
					DeleteBlock();
				else if (!CurPos && m_it_CurLine != Lines.begin())
					ProcessKey(Manager::Key(KEY_BS));
				else
				{
					AddUndoData(UNDO_EDIT, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), m_it_CurLine.Number(), m_it_CurLine->GetCurPos());
					m_it_CurLine->ProcessKey(Manager::Key(KEY_CTRLBS));
					Change(ECTYPE_CHANGED, m_it_CurLine.Number());
				}

				Show();
			}

			return TRUE;
		}
		case KEY_UP: case KEY_NUMPAD8:
		{
			{
				m_Flags.Set(FEDITOR_NEWUNDO);
				int PrevMaxPos=MaxRightPos;
				const auto LastTopScreen = m_it_TopScreen;
				Up();

				if (m_it_TopScreen==LastTopScreen)
					ShowEditor();
				else
					Show();

				if (PrevMaxPos>m_it_CurLine->GetTabCurPos())
				{
					m_it_CurLine->SetTabCurPos(PrevMaxPos);
					m_it_CurLine->FastShow();
					m_it_CurLine->SetTabCurPos(PrevMaxPos);
					Show();
				}
			}
			return TRUE;
		}
		case KEY_DOWN: case KEY_NUMPAD2:
		{
			{
				m_Flags.Set(FEDITOR_NEWUNDO);
				int PrevMaxPos=MaxRightPos;
				const auto LastTopScreen = m_it_TopScreen;
				Down();

				if (m_it_TopScreen==LastTopScreen)
					ShowEditor();
				else
					Show();

				if (PrevMaxPos>m_it_CurLine->GetTabCurPos())
				{
					m_it_CurLine->SetTabCurPos(PrevMaxPos);
					m_it_CurLine->FastShow();
					m_it_CurLine->SetTabCurPos(PrevMaxPos);
					Show();
				}
			}
			return TRUE;
		}
		case KEY_MSWHEEL_UP:
		case(KEY_MSWHEEL_UP | KEY_ALT):
		case(KEY_MSWHEEL_UP | KEY_RALT):
		{
			int Roll = (LocalKey() & (KEY_ALT|KEY_RALT))?1:(int)Global->Opt->MsWheelDeltaEdit;

			for (int i=0; i<Roll; i++)
				ProcessKey(Manager::Key(KEY_CTRLUP));

			return TRUE;
		}
		case KEY_MSWHEEL_DOWN:
		case(KEY_MSWHEEL_DOWN | KEY_ALT):
		case(KEY_MSWHEEL_DOWN | KEY_RALT):
		{
			int Roll = (LocalKey() & (KEY_ALT|KEY_RALT))?1:(int)Global->Opt->MsWheelDeltaEdit;

			for (int i=0; i<Roll; i++)
				ProcessKey(Manager::Key(KEY_CTRLDOWN));

			return TRUE;
		}
		case KEY_MSWHEEL_LEFT:
		case(KEY_MSWHEEL_LEFT | KEY_ALT):
		case(KEY_MSWHEEL_LEFT | KEY_RALT):
		{
			int Roll = (LocalKey() & (KEY_ALT|KEY_RALT))?1:(int)Global->Opt->MsHWheelDeltaEdit;

			for (int i=0; i<Roll; i++)
				ProcessKey(Manager::Key(KEY_LEFT));

			return TRUE;
		}
		case KEY_MSWHEEL_RIGHT:
		case(KEY_MSWHEEL_RIGHT | KEY_ALT):
		case(KEY_MSWHEEL_RIGHT | KEY_RALT):
		{
			int Roll = (LocalKey() & (KEY_ALT|KEY_RALT))?1:(int)Global->Opt->MsHWheelDeltaEdit;

			for (int i=0; i<Roll; i++)
				ProcessKey(Manager::Key(KEY_RIGHT));

			return TRUE;
		}
		case KEY_CTRLUP:  case KEY_CTRLNUMPAD8:
		case KEY_RCTRLUP: case KEY_RCTRLNUMPAD8:
		{
			m_Flags.Set(FEDITOR_NEWUNDO);
			ScrollUp();
			Show();
			return TRUE;
		}
		case KEY_CTRLDOWN:  case KEY_CTRLNUMPAD2:
		case KEY_RCTRLDOWN: case KEY_RCTRLNUMPAD2:
		{
			m_Flags.Set(FEDITOR_NEWUNDO);
			ScrollDown();
			Show();
			return TRUE;
		}
		case KEY_PGUP:     case KEY_NUMPAD9:
		{
			m_Flags.Set(FEDITOR_NEWUNDO);

			for (int I=m_Y1; I<m_Y2; I++)
				ScrollUp();

			Show();
			return TRUE;
		}
		case KEY_PGDN:    case KEY_NUMPAD3:
		{
			m_Flags.Set(FEDITOR_NEWUNDO);

			for (int I=m_Y1; I<m_Y2; I++)
				ScrollDown();

			Show();
			return TRUE;
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

				if (LocalKey() == KEY_CTRLHOME || LocalKey() == KEY_RCTRLHOME || LocalKey() == KEY_CTRLNUMPAD7 || LocalKey() == KEY_RCTRLNUMPAD7)
				{
					m_it_CurLine->SetCurPos(0);
					m_it_CurLine->SetLeftPos(0);
				}
				else
					m_it_CurLine->SetTabCurPos(StartPos);

				Show();
			}
			return TRUE;
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
				for (int I = m_Y1; I < m_Y2 && m_it_TopScreen != Lines.begin(); I++)
				{
					m_it_TopScreen->SetPosition(m_X1,I,XX2,I);
					--m_it_TopScreen;
				}

				m_it_CurLine->SetLeftPos(0);

				if (LocalKey() == KEY_CTRLEND || LocalKey() == KEY_RCTRLEND || LocalKey() == KEY_CTRLNUMPAD1 || LocalKey() == KEY_RCTRLNUMPAD1)
				{
					m_it_CurLine->SetCurPos(m_it_CurLine->GetLength());
					m_it_CurLine->FastShow();
				}
				else
					m_it_CurLine->SetTabCurPos(StartPos);

				Show();
			}
			return TRUE;
		}
		case KEY_NUMENTER:
		case KEY_ENTER:
		{
			if (!Pasting && !EdOpt.PersistentBlocks && IsStreamSelection())
				DeleteBlock();

			m_Flags.Set(FEDITOR_NEWUNDO);
			InsertString();
			m_it_CurLine->FastShow();
			Show();

			return TRUE;
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
			Show();
			return TRUE;
		}
		case KEY_CTRLE:
		case KEY_RCTRLE:
		{
			{
				m_Flags.Set(FEDITOR_NEWUNDO);
				auto CurPtr = m_it_TopScreen;
				for (int I = m_Y1; I<m_Y2; ++I, ++CurPtr)
				{
					if (IsLastLine(CurPtr))
						break;
				}

				m_it_CurLine=CurPtr;
				m_it_CurLine->SetCurPos(CurPos);
				Show();
			}
			return TRUE;
		}
		case KEY_CTRLL:
		case KEY_RCTRLL:
		{
			m_Flags.Swap(FEDITOR_LOCKMODE);

			if (HostFileEditor) HostFileEditor->ShowStatus();

			return TRUE;
		}
		case KEY_CTRLY:
		case KEY_RCTRLY:
		{
			DeleteString(m_it_CurLine, false);
			Show();
			return TRUE;
		}
		case KEY_F7:
		{
			bool ReplaceMode0=ReplaceMode;
			bool ReplaceAll0=ReplaceAll;
			ReplaceMode=ReplaceAll=false;

			if (!Search(FALSE))
			{
				ReplaceMode=ReplaceMode0;
				ReplaceAll=ReplaceAll0;
			}

			return TRUE;
		}
		case KEY_CTRLF7:
		case KEY_RCTRLF7:
		{
			if (!m_Flags.Check(FEDITOR_LOCKMODE))
			{
				bool ReplaceMode0=ReplaceMode;
				bool ReplaceAll0=ReplaceAll;
				ReplaceMode = true;
				ReplaceAll = false;

				if (!Search(FALSE))
				{
					ReplaceMode=ReplaceMode0;
					ReplaceAll=ReplaceAll0;
				}
			}

			return TRUE;
		}
		case KEY_SHIFTF7:
		{
			/* $ 20.09.2000 SVS
			   При All после нажатия Shift-F7 надобно снова спросить...
			*/
			//ReplaceAll=FALSE;
			/* $ 07.05.2001 IS
			   Сказано в хелпе "Shift-F7 Продолжить _поиск_"
			*/
			//ReplaceMode=FALSE;
			TurnOffMarkingBlock();
			Search(TRUE);
			return TRUE;
		}
		case KEY_ALTF7:
		case KEY_RALTF7:
		{
			TurnOffMarkingBlock();
			bool LastSearchReversePrev = LastSearchReverse;
			LastSearchReverse = !LastSearchReverse;
			Search(TRUE);
			LastSearchReverse = LastSearchReversePrev;
			return TRUE;
		}

		case KEY_F11:
		{
			/*
			      if(!Flags.Check(FEDITOR_DIALOGMEMOEDIT))
			      {
			        Global->CtrlObject->Plugins->CurEditor=HostFileEditor; // this;
			        if (Global->CtrlObject->Plugins->CommandsMenu(MODALTYPE_EDITOR,0,"Editor"))
			          *PluginTitle=0;
			        Show();
			      }
			*/
			return TRUE;
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
				Lock();
				Undo(LocalKey()==KEY_CTRLSHIFTZ || LocalKey()==KEY_RCTRLSHIFTZ);
				Unlock();
				Show();
			}

			return TRUE;
		}
		case KEY_ALTF8:
		case KEY_RALTF8:
		{
			{
				GoToPosition();

				// <GOTO_UNMARK:1>
				if (!EdOpt.PersistentBlocks)
					UnmarkBlock();

				// </GOTO_UNMARK>
				Show();
			}
			return TRUE;
		}
		case KEY_ALTU:
		case KEY_RALTU:
		{
			if (!m_Flags.Check(FEDITOR_LOCKMODE))
			{
				BlockLeft();
				Show();
			}

			return TRUE;
		}
		case KEY_ALTI:
		case KEY_RALTI:
		{
			if (!m_Flags.Check(FEDITOR_LOCKMODE))
			{
				BlockRight();
				Show();
			}

			return TRUE;
		}
		case KEY_ALTSHIFTLEFT:  case KEY_ALTSHIFTNUMPAD4:
		case KEY_RALTSHIFTLEFT: case KEY_RALTSHIFTNUMPAD4:
		case KEY_ALTLEFT:
		case KEY_RALTLEFT:
		{
			if (!CurPos)
				return TRUE;

			ProcessVBlockMarking();

			Pasting++;
			{
				int Delta=m_it_CurLine->GetTabCurPos()-m_it_CurLine->RealPosToTab(CurPos-1);

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

				ProcessKey(Manager::Key(KEY_LEFT));
			}
			Pasting--;
			Show();
			//_D(SysLog(L"VBlockX=%i, VBlockSizeX=%i, GetLineCurPos=%i",VBlockX,VBlockSizeX,GetLineCurPos()));
			//_D(SysLog(L"~~~~~~~~~~~~~~~~ KEY_ALTLEFT END, VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
			return TRUE;
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
				return TRUE;

			ProcessVBlockMarking();

			//_D(SysLog(L"---------------- KEY_ALTRIGHT, getLineCurPos=%i",GetLineCurPos()));
			Pasting++;
			{
				int Delta;
				/* $ 18.07.2000 tran
				     встань в начало текста, нажми alt-right, alt-pagedown,
				     выделится блок шириной в 1 колонку, нажми еще alt-right
				     выделение сбросится
				*/
				int VisPos=m_it_CurLine->RealPosToTab(CurPos),
				           NextVisPos=m_it_CurLine->RealPosToTab(CurPos+1);
				//_D(SysLog(L"CurPos=%i, VisPos=%i, NextVisPos=%i",
				//    CurPos,VisPos, NextVisPos); //,CurLine->GetTabCurPos()));
				Delta=NextVisPos-VisPos;
				//_D(SysLog(L"Delta=%i",Delta));

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

				ProcessKey(Manager::Key(KEY_RIGHT));
				//_D(SysLog(L"VBlockX=%i, VBlockSizeX=%i, GetLineCurPos=%i",VBlockX,VBlockSizeX,GetLineCurPos()));
			}
			Pasting--;
			Show();
			//_D(SysLog(L"~~~~~~~~~~~~~~~~ KEY_ALTRIGHT END, VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
			return TRUE;
		}
		/* $ 29.06.2000 IG
		  + CtrlAltLeft, CtrlAltRight для вертикальный блоков
		*/
		case KEY_CTRLALTLEFT:   case KEY_CTRLALTNUMPAD4:
		{
			{
				int SkipSpace=TRUE;
				Pasting++;
				Lock();

				for (;;)
				{
					const auto& Str = m_it_CurLine->GetString();
					size_t LocalCurPos = m_it_CurLine->GetCurPos();

					while (LocalCurPos > Str.size())
					{
						ProcessKey(Manager::Key(KEY_ALTSHIFTLEFT));
						LocalCurPos = m_it_CurLine->GetCurPos();
					}

					if (!LocalCurPos)
						break;

					if (IsSpace(Str[LocalCurPos - 1]) || IsWordDiv(EdOpt.strWordDiv, Str[LocalCurPos - 1]))
					{
						if (SkipSpace)
						{
							ProcessKey(Manager::Key(KEY_ALTSHIFTLEFT));
							continue;
						}
						else
							break;
					}

					SkipSpace=FALSE;
					ProcessKey(Manager::Key(KEY_ALTSHIFTLEFT));
				}

				Pasting--;
				Unlock();
				Show();
			}
			return TRUE;
		}
		case KEY_CTRLALTRIGHT:   case KEY_CTRLALTNUMPAD6:
		{
			{
				int SkipSpace=TRUE;
				Pasting++;
				Lock();

				for (;;)
				{
					const auto& Str = m_it_CurLine->GetString();
					const size_t LocalCurPos = m_it_CurLine->GetCurPos();

					if (LocalCurPos >= Str.size())
						break;

					if (IsSpace(Str[LocalCurPos]) || IsWordDiv(EdOpt.strWordDiv, Str[LocalCurPos]))
					{
						if (SkipSpace)
						{
							ProcessKey(Manager::Key(KEY_ALTSHIFTRIGHT));
							continue;
						}
						else
							break;
					}

					SkipSpace=FALSE;
					ProcessKey(Manager::Key(KEY_ALTSHIFTRIGHT));
				}

				Pasting--;
				Unlock();
				Show();
			}
			return TRUE;
		}
		case KEY_ALTSHIFTUP:    case KEY_ALTSHIFTNUMPAD8:
		case KEY_RALTSHIFTUP:   case KEY_RALTSHIFTNUMPAD8:
		case KEY_ALTUP:
		case KEY_RALTUP:
		{
			if (m_it_CurLine == Lines.begin())
				return TRUE;

			ProcessVBlockMarking();

			const auto PrevLine = std::prev(m_it_CurLine);
			if (!EdOpt.CursorBeyondEOL && VBlockX >= PrevLine->RealPosToTab(PrevLine->GetLength()))
				return TRUE;

			Pasting++;

			if (m_it_CurLine.Number() > m_it_AnyBlockStart.Number())
				VBlockSizeY--;
			else
			{
				++VBlockSizeY;
				--m_it_AnyBlockStart;
			}

			ProcessKey(Manager::Key(KEY_UP));
			AdjustVBlock(CurVisPos);
			Pasting--;
			Show();
			return TRUE;
		}
		case KEY_ALTSHIFTDOWN:  case KEY_ALTSHIFTNUMPAD2:
		case KEY_RALTSHIFTDOWN: case KEY_RALTSHIFTNUMPAD2:
		case KEY_ALTDOWN:
		case KEY_RALTDOWN:
		{
			const auto NextLine = std::next(m_it_CurLine);
			if (NextLine == Lines.end())
				return TRUE;

			ProcessVBlockMarking();

			if (!EdOpt.CursorBeyondEOL && VBlockX >= NextLine->RealPosToTab(NextLine->GetLength()))
				return TRUE;

			Pasting++;

			if (m_it_CurLine.Number() >= m_it_AnyBlockStart.Number() + VBlockSizeY - 1)
				++VBlockSizeY;
			else
			{
				--VBlockSizeY;
				++m_it_AnyBlockStart;
			}

			ProcessKey(Manager::Key(KEY_DOWN));
			AdjustVBlock(CurVisPos);
			Pasting--;
			Show();
			//_D(SysLog(L"~~~~ Key_AltDOWN: VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
			return TRUE;
		}
		case KEY_ALTSHIFTHOME:  case KEY_ALTSHIFTNUMPAD7:
		case KEY_RALTSHIFTHOME: case KEY_RALTSHIFTNUMPAD7:
		case KEY_ALTHOME:
		case KEY_RALTHOME:
		{
			Pasting++;
			Lock();

			while (m_it_CurLine->GetCurPos()>0)
				ProcessKey(Manager::Key(KEY_ALTSHIFTLEFT));

			Unlock();
			Pasting--;
			Show();
			return TRUE;
		}
		case KEY_ALTSHIFTEND:  case KEY_ALTSHIFTNUMPAD1:
		case KEY_RALTSHIFTEND: case KEY_RALTSHIFTNUMPAD1:
		case KEY_ALTEND:
		case KEY_RALTEND:
		{
			Pasting++;
			Lock();

			if (m_it_CurLine->GetCurPos()<m_it_CurLine->GetLength())
				while (m_it_CurLine->GetCurPos()<m_it_CurLine->GetLength())
					ProcessKey(Manager::Key(KEY_ALTSHIFTRIGHT));

			if (m_it_CurLine->GetCurPos()>m_it_CurLine->GetLength())
				while (m_it_CurLine->GetCurPos()>m_it_CurLine->GetLength())
					ProcessKey(Manager::Key(KEY_ALTSHIFTLEFT));

			Unlock();
			Pasting--;
			Show();
			return TRUE;
		}
		case KEY_ALTSHIFTPGUP:  case KEY_ALTSHIFTNUMPAD9:
		case KEY_RALTSHIFTPGUP: case KEY_RALTSHIFTNUMPAD9:
		case KEY_ALTPGUP:
		case KEY_RALTPGUP:
		{
			Pasting++;
			Lock();

			for (int I=m_Y1; I<m_Y2; I++)
				ProcessKey(Manager::Key(KEY_ALTSHIFTUP));

			Unlock();
			Pasting--;
			Show();
			return TRUE;
		}
		case KEY_ALTSHIFTPGDN:  case KEY_ALTSHIFTNUMPAD3:
		case KEY_RALTSHIFTPGDN: case KEY_RALTSHIFTNUMPAD3:
		case KEY_ALTPGDN:
		case KEY_RALTPGDN:
		{
			Pasting++;
			Lock();

			for (int I=m_Y1; I<m_Y2; I++)
				ProcessKey(Manager::Key(KEY_ALTSHIFTDOWN));

			Unlock();
			Pasting--;
			Show();
			return TRUE;
		}
		case KEY_CTRLALTPGUP:   case KEY_CTRLALTNUMPAD9:
		case KEY_CTRLALTHOME:   case KEY_CTRLALTNUMPAD7:
		{
			Lock();
			Pasting++;

			auto PrevLine = Lines.end();
			while (m_it_CurLine != Lines.begin() && PrevLine != m_it_CurLine)
			{
				PrevLine = m_it_CurLine;
				ProcessKey(Manager::Key(KEY_ALTUP));
			}

			Pasting--;
			Unlock();
			Show();
			return TRUE;
		}
		case KEY_CTRLALTPGDN:   case KEY_CTRLALTNUMPAD3:
		case KEY_CTRLALTEND:    case KEY_CTRLALTNUMPAD1:
		{
			Lock();
			Pasting++;

			auto PrevLine = Lines.end();
			while (!IsLastLine(m_it_CurLine) && PrevLine != m_it_CurLine)
			{
				PrevLine = m_it_CurLine;
				ProcessKey(Manager::Key(KEY_ALTDOWN));
			}

			Pasting--;
			Unlock();
			Show();
			return TRUE;
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
				TextChanged(1);

				if (!EdOpt.PersistentBlocks && IsStreamSelection())
				{
					TurnOffMarkingBlock();
					DeleteBlock();
				}

				AddUndoData(UNDO_EDIT, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), m_it_CurLine.Number(), m_it_CurLine->GetCurPos());
				m_it_CurLine->ProcessKey(Key);
				Change(ECTYPE_CHANGED, m_it_CurLine.Number());
				Pasting--;
				Show();
			}

			return TRUE;
		}
		case KEY_CTRLQ:
		case KEY_RCTRLQ:
		{
			if (!m_Flags.Check(FEDITOR_LOCKMODE))
			{
				m_Flags.Set(FEDITOR_PROCESSCTRLQ);

				if (HostFileEditor) HostFileEditor->ShowStatus();

				Pasting++;
				TextChanged(1);

				if (!EdOpt.PersistentBlocks && IsStreamSelection())
				{
					TurnOffMarkingBlock();
					DeleteBlock();
				}

				AddUndoData(UNDO_EDIT, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), m_it_CurLine.Number(), m_it_CurLine->GetCurPos());
				m_it_CurLine->ProcessCtrlQ();
				Change(ECTYPE_CHANGED, m_it_CurLine.Number());
				m_Flags.Clear(FEDITOR_PROCESSCTRLQ);
				Pasting--;
				Show();
			}

			return TRUE;
		}
		case KEY_OP_SELWORD:
		{
			int OldCurPos=CurPos;
			size_t SBegin, SEnd;
			Pasting++;
			Lock();
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
			Unlock();
			Show();
			return TRUE;
		}
		case KEY_OP_PLAINTEXT:
		{
			if (!m_Flags.Check(FEDITOR_LOCKMODE))
			{
				string strTStr(Global->CtrlObject->Macro.GetStringToPrint());

				Pasting++;
				//_SVS(SysLogDump(Fmt,0,TStr,strlen(TStr),nullptr));
				TextChanged(1);

				if (!EdOpt.PersistentBlocks && IsAnySelection())
				{
					TurnOffMarkingBlock();
					DeleteBlock();
				}

				//AddUndoData(UNDO_EDIT,CurLine->GetString(),CurLine->GetEOL(),NumLine,CurLine->GetCurPos());
				Paste(strTStr);
				//if (!EdOpt.PersistentBlocks && IsBlock)
				UnmarkBlock();
				Pasting--;
				Show();
			}

			return TRUE;
		}
		default:
		{
			{
				if ((LocalKey()==KEY_CTRLDEL || LocalKey()==KEY_RCTRLDEL || LocalKey()==KEY_CTRLNUMDEL || LocalKey()==KEY_RCTRLNUMDEL
					|| LocalKey()==KEY_CTRLDECIMAL || LocalKey()==KEY_RCTRLDECIMAL || LocalKey()==KEY_CTRLT || LocalKey()==KEY_RCTRLT)
					&& CurPos>=m_it_CurLine->GetLength())
				{
					/*$ 08.12.2000 skv
					  - CTRL-DEL в начале строки при выделенном блоке и
					    включенном EditorDelRemovesBlocks
					*/
					bool save=EdOpt.DelRemovesBlocks;
					EdOpt.DelRemovesBlocks=false;
					int ret=ProcessKey(Manager::Key(KEY_DEL));
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
						Show();
					}

				int SkipCheckUndo=(LocalKey()==KEY_RIGHT      || LocalKey()==KEY_NUMPAD6      ||
				                   LocalKey()==KEY_CTRLLEFT   || LocalKey()==KEY_CTRLNUMPAD4  ||
				                   LocalKey()==KEY_RCTRLLEFT  || LocalKey()==KEY_RCTRLNUMPAD4 ||
				                   LocalKey()==KEY_CTRLRIGHT  || LocalKey()==KEY_CTRLNUMPAD6  ||
				                   LocalKey()==KEY_RCTRLRIGHT || LocalKey()==KEY_RCTRLNUMPAD6 ||
				                   LocalKey()==KEY_HOME       || LocalKey()==KEY_NUMPAD7      ||
				                   LocalKey()==KEY_END        || LocalKey()==KEY_NUMPAD1      ||
				                   LocalKey()==KEY_CTRLS      || LocalKey()==KEY_RCTRLS);

				if (m_Flags.Check(FEDITOR_LOCKMODE) && !SkipCheckUndo)
					return TRUE;

				if (LocalKey() == KEY_HOME || LocalKey() == KEY_NUMPAD7)
					m_Flags.Set(FEDITOR_NEWUNDO);

				if ((LocalKey()==KEY_CTRLLEFT || LocalKey()==KEY_RCTRLLEFT || LocalKey()==KEY_CTRLNUMPAD4 || LocalKey()==KEY_RCTRLNUMPAD4) && !m_it_CurLine->GetCurPos())
				{
					Pasting++;
					ProcessKey(Manager::Key(KEY_LEFT));
					Pasting--;
					/* $ 24.9.2001 SKV
					  fix бага с ctrl-left в начале строки
					  в блоке с переопределённым плагином фоном.
					*/
					ShowEditor();
					//if(!Flags.Check(FEDITOR_DIALOGMEMOEDIT)){
					//Global->CtrlObject->Plugins->CurEditor=HostFileEditor; // this;
					//_D(SysLog(L"%08d EE_REDRAW",__LINE__));
					//Global->CtrlObject->Plugins->ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
					//}
					return TRUE;
				}

				if (((!EdOpt.CursorBeyondEOL && (LocalKey()==KEY_RIGHT || LocalKey()==KEY_NUMPAD6))
					|| LocalKey()==KEY_CTRLRIGHT || LocalKey()==KEY_RCTRLRIGHT || LocalKey()==KEY_CTRLNUMPAD6 || LocalKey()==KEY_RCTRLNUMPAD6) &&
				        m_it_CurLine->GetCurPos()>=m_it_CurLine->GetLength() &&
				        !IsLastLine(m_it_CurLine))
				{
					Pasting++;
					ProcessKey(Manager::Key(KEY_HOME));
					ProcessKey(Manager::Key(KEY_DOWN));
					Pasting--;

					if (!m_Flags.Check(FEDITOR_DIALOGMEMOEDIT))
					{
						//_D(SysLog(L"%08d EE_REDRAW",__LINE__));
						_SYS_EE_REDRAW(SysLog(L"Editor::ProcessKey[%d](!EdOpt.CursorBeyondEOL): EE_REDRAW(EEREDRAW_ALL)",__LINE__));
						SortColorLock();
						Global->CtrlObject->Plugins->ProcessEditorEvent(EE_REDRAW, EEREDRAW_ALL, this);
						SortColorUnlock();
					}

					/*$ 03.02.2001 SKV
					  А то EEREDRAW_ALL то уходит, а на самом деле
					  только текущая линия перерисовывается.
					*/
					ShowEditor();
					return TRUE;
				}

				const auto& Str = m_it_CurLine->GetString();

				intptr_t LocalCurPos = m_it_CurLine->GetCurPos();

				if (IsCharKey(LocalKey()) && LocalCurPos>0 && Str.empty())
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

						for (size_t I = 0; I != PrevStr.size() && IsSpace(PrevStr[I]); ++I)
						{
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
								m_it_CurLine->ProcessKey(Manager::Key(PrevStr[I]));
						}

						m_it_CurLine->SetTabCurPos(TabPos);
					}
				}

				string CmpStr;

				if (!SkipCheckUndo)
				{
					LocalCurPos=m_it_CurLine->GetCurPos();
					CmpStr = Str;
				}

				if (LocalKey() == KEY_OP_XLAT)
				{
					Xlat();
					Show();
					return TRUE;
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
							AddUndoData(UNDO_EDIT, CmpStr, m_it_CurLine->GetEOL(), m_it_CurLine.Number(), CurPos); // EOL? - CurLine->GetEOL()  GlobalEOL   ""
							Change(ECTYPE_CHANGED, m_it_CurLine.Number());
							TextChanged(1);
						}
					}

					// <Bug 794>
					// обработаем только первую и последнюю строку с блоком
					if ((LocalKey() == KEY_CTRLK || LocalKey() == KEY_RCTRLK) && EdOpt.PersistentBlocks)
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
					ShowEditor();
					return TRUE;
				}

				if (IsVerticalSelection())
					Show();
			}
			return FALSE;
		}
	}
}


int Editor::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
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

	if (EdOpt.ShowScrollBar && ScrollBarRequired(ObjHeight(), m_LinesCount) && MouseEvent->dwMousePosition.X==m_X2 && !(MouseEvent->dwEventFlags & MOUSE_MOVED))
	{
		if (MouseEvent->dwMousePosition.Y==m_Y1)
		{
			while (IsMouseButtonPressed())
			{
				ProcessKey(Manager::Key(KEY_CTRLUP));
			}
		}
		else if (MouseEvent->dwMousePosition.Y==m_Y2)
		{
			while (IsMouseButtonPressed())
			{
				ProcessKey(Manager::Key(KEY_CTRLDOWN));
			}
		}
		else
		{
			while (IsMouseButtonPressed())
				GoToLine((m_LinesCount-1)*(IntKeyState.MouseY-m_Y1)/(m_Y2-m_Y1));
		}

		return TRUE;
	}

	if (MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED)
	{
		static clock_t EditorPrevDoubleClick=0;
		static COORD EditorPrevPosition={};

		const auto CurrentTime = clock();

		if (static_cast<unsigned long>((CurrentTime - EditorPrevDoubleClick) / CLOCKS_PER_SEC * 1000) <= GetDoubleClickTime() && MouseEvent->dwEventFlags != MOUSE_MOVED &&
		        EditorPrevPosition.X == MouseEvent->dwMousePosition.X && EditorPrevPosition.Y == MouseEvent->dwMousePosition.Y)
		{
			m_it_CurLine->Select(0, m_it_CurLine->m_Str.size());

			if (m_it_CurLine->IsSelection())
			{
				BeginStreamMarking(m_it_CurLine);
			}

			EditorPrevDoubleClick=0;
			EditorPrevPosition.X=0;
			EditorPrevPosition.Y=0;
			Show();
			return TRUE;
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
			return TRUE;
		}
		else
		{
			EditorPrevDoubleClick=0;
			EditorPrevPosition.X=0;
			EditorPrevPosition.Y=0;
		}

		Show();
	}

	if (m_it_CurLine->ProcessMouse(MouseEvent))
	{
		if (HostFileEditor) HostFileEditor->ShowStatus();

		if (IsVerticalSelection())
			Show();
		else
		{
			if (!m_Flags.Check(FEDITOR_DIALOGMEMOEDIT))
			{
				_SYS_EE_REDRAW(SysLog(L"Editor::ProcessMouse[%08d] ProcessEditorEvent(EE_REDRAW)",__LINE__));
				SortColorLock();
				Global->CtrlObject->Plugins->ProcessEditorEvent(EE_REDRAW, EEREDRAW_ALL, this);
				SortColorUnlock();
			}
			ShowEditor();
		}

		return TRUE;
	}

	if (!(MouseEvent->dwButtonState & 3))
		return FALSE;

	// scroll up
	if (MouseEvent->dwMousePosition.Y==m_Y1-1)
	{
		while (IsMouseButtonPressed() && IntKeyState.MouseY==m_Y1-1)
			ProcessKey(Manager::Key(KEY_UP));

		return TRUE;
	}

	// scroll down
	if (MouseEvent->dwMousePosition.Y==m_Y2+1)
	{
		while (IsMouseButtonPressed() && IntKeyState.MouseY==m_Y2+1)
			ProcessKey(Manager::Key(KEY_DOWN));

		return TRUE;
	}

	if (MouseEvent->dwMousePosition.X<m_X1 || MouseEvent->dwMousePosition.X>m_X2 ||
	        MouseEvent->dwMousePosition.Y<m_Y1 || MouseEvent->dwMousePosition.Y>m_Y2)
		return FALSE;

	int NewDist = MouseEvent->dwMousePosition.Y-m_Y1;
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
	return TRUE;
}


int Editor::CalcDistance(const numbered_iterator& From, const numbered_iterator& To) const
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

	TextChanged(1);

	if (IsLastLine(DelPtr) && (!DeleteLast || DelPtr == Lines.begin()))
	{
		AddUndoData(UNDO_EDIT, DelPtr->GetString(), DelPtr->GetEOL(), DelPtr.Number(), DelPtr->GetCurPos());
		DelPtr->ClearString();
		Change(ECTYPE_CHANGED, DelPtr.Number());
		return DelPtr;
	}

	UpdateIterator(m_it_CurLine);
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
		std::prev(DelPtr)->SetEOL(L"");
	}

	AddUndoData(UNDO_DELSTR, DelPtr->GetString(), DelPtr->GetEOL(), DelPtr.Number(), 0);

	std::for_each(RANGE(m_SavePos, i)
	{
		//FIXME: it would be better to add the bookmarks for deleted line to UndoData
		if (i.Line != POS_NONE && DelPtr.Number() < i.Line)
			--i.Line;
	});

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

	m_AutoDeletedColors.erase(&*DelPtr);

	const auto Result = numbered_iterator(Lines.erase(DelPtr), DelPtr.Number());
	--m_LinesCount;

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
//  TextChanged(1);
	iterator SrcIndent = Lines.end();
	intptr_t SelStart,SelEnd;
	int NewLineEmpty=TRUE;
	const auto NextLine = std::next(m_it_CurLine);
	const auto NewString = InsertString(nullptr, 0, NextLine);

	if (NewString == Lines.end())
		return;

	Change(ECTYPE_ADDED, m_it_CurLine.Number() + 1);
	//NewString->SetTables(UseDecodeTable ? &TableSet:nullptr); // ??

	size_t CurPos=m_it_CurLine->GetCurPos();
	m_it_CurLine->GetSelection(SelStart,SelEnd);

	std::for_each(RANGE(m_SavePos, i)
	{
		if (i.Line != POS_NONE && (m_it_CurLine.Number() < i.Line || (m_it_CurLine.Number() == i.Line && !CurPos)))
			++i.Line;
	});

	if (!SessionBookmarks.empty())
	{
		std::for_each(RANGE(SessionBookmarks, i)
		{
			if (m_it_CurLine.Number() < static_cast<int>(i.Line) || (m_it_CurLine.Number() == static_cast<int>(i.Line) && !CurPos))
				i.Line++;
		});
	}

	int IndentPos=0;

	if (EdOpt.AutoIndent && !Pasting)
	{
		auto PrevLine = m_it_CurLine;

		while (PrevLine != Lines.end())
		{
			const auto& Str = PrevLine->GetString();

			const auto It = std::find_if_not(ALL_CONST_RANGE(Str), IsSpace);
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

	auto EndSeq = m_it_CurLine->GetEOL();

	if (CurPos < static_cast<size_t>(m_it_CurLine->GetLength()))
	{
		const auto& CurLineStr = m_it_CurLine->GetString();

		if (IndentPos > 0 && !std::all_of(CurLineStr.cbegin(), CurLineStr.cbegin() + CurPos, IsSpace))
		{
			SpaceOnly = FALSE;
		}

		NewString->SetString(CurLineStr.data() + CurPos, CurLineStr.size() - CurPos);

		if (!std::all_of(CurLineStr.cbegin() + CurPos, CurLineStr.cend(), IsSpace))
		{
			NewLineEmpty = FALSE;
		}

		AddUndoData(UNDO_BEGIN);
		AddUndoData(UNDO_EDIT, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), m_it_CurLine.Number(), m_it_CurLine->GetCurPos());
		AddUndoData(UNDO_INSSTR, {}, m_it_CurLine->GetEOL(), m_it_CurLine.Number() + 1, 0);
		AddUndoData(UNDO_END);

		string NewCurLineStr(CurLineStr, 0, CurPos);

		if (EdOpt.AutoIndent && NewLineEmpty)
		{
			RemoveTrailingSpaces(NewCurLineStr);
		}

		m_it_CurLine->SetString(NewCurLineStr);

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
		AddUndoData(UNDO_INSSTR, {}, m_it_CurLine->GetEOL(), m_it_CurLine.Number() + 1, 0);
	}

	if (*EndSeq)
	{
		m_it_CurLine->SetEOL(EndSeq);
	}
	else
	{
		m_it_CurLine->SetEOL(GlobalEOL.empty() ? GetDefaultEOL() : GlobalEOL.data());
		NewString->SetEOL(EndSeq);
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

	NewString->SetEOL(EndSeq);
	m_it_CurLine->SetCurPos(0);

	Down();

	if (IndentPos>0)
	{
		size_t OrgIndentPos = IndentPos;
		ShowEditor();

		if (SpaceOnly)
		{
			int Decrement=0;
			const auto& Str = m_it_CurLine->GetString();
			const auto MaxI = std::min(static_cast<size_t>(IndentPos), Str.size());
			for (size_t I = 0; I != MaxI && IsSpace(Str[I]); ++I)
			{
				if (Str[I]==L' ')
					Decrement++;
				else
				{
					int TabPos=m_it_CurLine->RealPosToTab(static_cast<int>(I));
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
				int SaveOvertypeMode=m_it_CurLine->GetOvertypeMode();
				m_it_CurLine->SetOvertypeMode(false);

				if (SrcIndent != Lines.end())
				{
					const auto& PrevStr = SrcIndent->GetString();
					for (size_t I = 0; m_it_CurLine->GetTabCurPos() < IndentPos; ++I)
					{
						if (I < PrevStr.size() && IsSpace(PrevStr[I]))
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
					for (size_t I = 0; m_it_CurLine->GetTabCurPos() < IndentPos; ++I)
					{
						m_it_CurLine->ProcessKey(Manager::Key(KEY_SPACE));
					}
				}

				while (m_it_CurLine->GetTabCurPos()>IndentPos)
					m_it_CurLine->ProcessKey(Manager::Key(KEY_BS));

				m_it_CurLine->SetOvertypeMode(SaveOvertypeMode!=0);
				Change(ECTYPE_CHANGED, m_it_CurLine.Number());
			}

			m_it_CurLine->SetTabCurPos(IndentPos);
		}

		const auto& Str = m_it_CurLine->GetString();
		CurPos=m_it_CurLine->GetCurPos();

		if (SpaceOnly)
		{
			const auto SpaceIterator = std::find_if_not(ALL_CONST_RANGE(Str), IsSpace);
			const auto NewPos = std::min<size_t>(SpaceIterator - Str.cbegin(), OrgIndentPos);
			if (NewPos > CurPos)
				m_it_CurLine->SetCurPos(static_cast<int>(NewPos));
		}
	}

	TextChanged(1);
}

template<class F>
void Editor::UpdateIteratorAndKeepPos(numbered_iterator& Iter, const F& Func)
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

	if (Y>=m_Y2-m_Y1)
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
	UINT Line;
	UINT Pos;
	UINT SearchLen;
};

class undo_block
{
public:
	undo_block(Editor* Owner): m_Owner(Owner) { m_Owner->AddUndoData(UNDO_BEGIN); }
	~undo_block() { m_Owner->AddUndoData(UNDO_END); }

private:
	Editor* m_Owner;
};

BOOL Editor::Search(int Next)
{
	string strSearchStr, strReplaceStr;
	static string strLastReplaceStr;
	string strMsgStr;
	bool Case,WholeWords,ReverseSearch,Regexp,PreserveStyle,Match,UserBreak;
	std::unique_ptr<undo_block> UndoBlock;

	if (Next && strLastSearchStr.empty())
		return TRUE;

	strSearchStr = strLastSearchStr;
	strReplaceStr = strLastReplaceStr;
	Case=LastSearchCase;
	WholeWords=LastSearchWholeWords;
	ReverseSearch=LastSearchReverse;
	PreserveStyle=LastSearchPreserveStyle;
	Regexp=LastSearchRegexp;

	bool FindAllReferences = false;

	if (!Next)
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
						const size_t TBlockX = m_it_AnyBlockStart->TabPosToReal(VBlockX);
						const size_t TBlockSizeX = m_it_AnyBlockStart->TabPosToReal(VBlockX + VBlockSizeX) - TBlockX;
						const auto& Str = m_it_AnyBlockStart->GetString();
						if (TBlockX <= Str.size())
						{
							auto CopySize = std::min(Str.size() - TBlockX, TBlockSizeX);
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

		const wchar_t *TextHistoryName=L"SearchText",*ReplaceHistoryName=L"ReplaceText";
		int DlgResult = GetSearchReplaceString(ReplaceMode, nullptr, nullptr, strSearchStr, strReplaceStr,
					TextHistoryName, ReplaceHistoryName, &Case, &WholeWords, &ReverseSearch, &Regexp, &PreserveStyle, L"EditorSearch", false,
					ReplaceMode? &EditorReplaceId : &EditorSearchId, Picker);
		if (!DlgResult)
		{
			return FALSE;
		}
		else if(DlgResult == 2)
		{
			FindAllReferences = true;
		}
	}

	strLastSearchStr = strSearchStr;
	strLastReplaceStr = strReplaceStr;
	LastSearchCase=Case;
	LastSearchWholeWords=WholeWords;
	LastSearchReverse=ReverseSearch;
	LastSearchRegexp=Regexp;
	LastSearchPreserveStyle=PreserveStyle;

	if(FindAllReferences)
	{
		ReverseSearch = false;
	}

	if (strSearchStr.empty())
		return TRUE;

	if (!EdOpt.PersistentBlocks || (EdOpt.SearchSelFound && !ReplaceMode))
		UnmarkBlock();

	const auto FindAllList = VMenu2::create({}, nullptr, 0);
	UINT AllRefLines = 0;
	{
		SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<EditorPreRedrawItem>());
		strMsgStr=strSearchStr;
		InsertQuote(strMsgStr);
		SetCursorType(false, -1);
		Match=0;
		UserBreak=0;
		int CurPos=m_it_CurLine->GetCurPos();

		if (Next && m_FoundLine == m_it_CurLine)
		{
			if (ReverseSearch)
			{
				if (EdOpt.SearchCursorAtEnd)
				{
					if (CurPos == m_FoundPos + m_FoundSize)
						CurPos -= m_FoundSize;
				}
			}
			else
			{
				if (!EdOpt.SearchCursorAtEnd)
				{
					if (CurPos == m_FoundPos)
					{
						++CurPos;
					}
				}
			}
		}

		auto CurPtr = FindAllReferences? FirstLine() : m_it_CurLine, TmpPtr = CurPtr;

		string strSlash(strSearchStr);
		InsertRegexpQuote(strSlash);
		std::vector<RegExpMatch> m;
		MatchHash hm;
		RegExp re;

		if (Regexp)
		{
			// Q: что важнее: опция диалога или опция RegExp`а?
			if (!re.Compile(strSlash.data(), OP_PERLSTYLE|OP_OPTIMIZE|(!Case?OP_IGNORECASE:0)))
			{
				ReCompileErrorMessage(re, strSlash);
				return FALSE; //BUGBUG
			}
			m.resize(re.GetBracketsCount() * 2);
		}

		string strSearchStrUpper = strSearchStr;
		string strSearchStrLower = strSearchStr;
		if (!Case)
		{
			ToUpper(strSearchStrUpper);
			ToLower(strSearchStrLower);
		}

		time_check TimeCheck(time_check::delayed, GetRedrawTimeout());
		int StartLine = m_it_CurLine.Number();
		SCOPED_ACTION(IndeterminateTaskBar);
		SCOPED_ACTION(wakeful);
		int LastCheckedLine = -1;

		while (CurPtr != Lines.end())
		{
			if (TimeCheck)
			{
				if (CheckForEscSilent())
				{
					if (ConfirmAbortOp())
					{
						UserBreak = true;
						break;
					}
				}

				strMsgStr=strSearchStr;
				InsertQuote(strMsgStr);
				SetCursorType(false, -1);
				int Total=FindAllReferences? m_LinesCount : (ReverseSearch? StartLine : m_LinesCount - StartLine);
				int Current=abs(CurPtr.Number() - StartLine);
				EditorShowMsg(MSG(MEditSearchTitle),MSG(MEditSearchingFor),strMsgStr,Total > 0 ? Current*100/Total : 100);
				Taskbar().SetProgressValue(Current,Total);
			}

			string strReplaceStrCurrent(ReplaceMode ? strReplaceStr : L"");

			int SearchLength;
			if (CurPtr->Search(strSearchStr, strSearchStrUpper, strSearchStrLower, re, m.data(), &hm, strReplaceStrCurrent, CurPos, Case, WholeWords, ReverseSearch, Regexp, PreserveStyle, &SearchLength))
			{
				Match = true;

				m_FoundLine = CurPtr;
				m_FoundPos = CurPtr->GetCurPos();
				m_FoundSize = SearchLength;

				if(FindAllReferences)
				{
					CurPos = m_FoundPos;
					int NextPos = CurPos + (SearchLength? SearchLength : 1);

					const int service_len = 12;
					MenuItemEx Item(FormatString() << fmt::LeftAlign() << fmt::ExactWidth(service_len) << fmt::FillChar(L' ') << (FormatString() << CurPtr.Number() + 1 << L':' << CurPos+1) << BoxSymbols[BS_V1] << CurPtr->GetString());
					Item.Annotations.emplace_back(CurPos + service_len + 1, NextPos - CurPos);
					FindCoord coord = { (UINT)CurPtr.Number(), (UINT)CurPos, (UINT)SearchLength };
					Item.UserData = coord;
					FindAllList->AddItem(Item);
					CurPos = NextPos;
					if (CurPtr.Number() != LastCheckedLine)
					{
						LastCheckedLine = CurPtr.Number();
						++AllRefLines;
					}
				}
				else
				{
					if (EdOpt.SearchSelFound && !ReplaceMode)
					{
						Pasting++;
						Lock();
						UnmarkBlock();
						BeginStreamMarking(CurPtr);
						CurPtr->Select(m_FoundPos, m_FoundPos + m_FoundSize);
						Unlock();
						Pasting--;
					}

					int at_end = EdOpt.SearchCursorAtEnd ? SearchLength : 0;

					int Skip=FALSE;

					// Отступим на четверть и проверим на перекрытие диалогом замены
					int FromTop=(ScrY-2)/4;
					if (FromTop<0 || FromTop>=((ScrY-5)/2-2))
						FromTop=0;

					TmpPtr=m_it_CurLine=CurPtr;

					for (int i=0; i<FromTop; i++)
					{
						if (TmpPtr != Lines.begin())
							--TmpPtr;
						else
							break;
					}

					m_it_TopScreen=TmpPtr;
					int LeftPos=CurPtr->GetLeftPos();
					int TabCurPos=CurPtr->GetTabCurPos();

					if (ObjWidth()>8 && TabCurPos-LeftPos+SearchLength>ObjWidth()-8)
						CurPtr->SetLeftPos(TabCurPos+SearchLength-ObjWidth()+8);

					if (ReplaceMode)
					{
						int MsgCode=0;

						if (!ReplaceAll)
						{
							Show();
							SHORT CurX,CurY;
							GetCursorPos(CurX,CurY);
							int lpos = CurPtr->LeftPos;
							int endX = CurPtr->RealPosToTab(CurPtr->TabPosToReal(lpos + CurX) + SearchLength - 1) - lpos;
							ChangeBlockColor(CurX,CurY, endX,CurY, colors::PaletteColorToFarColor(COL_EDITORSELECTEDTEXT));
							string strQSearchStr(CurPtr->GetString().data() + CurPtr->GetCurPos(),SearchLength), strQReplaceStr=strReplaceStrCurrent;

							// do not use InsertQuote, AI is not suitable here
							strQSearchStr.insert(0, 1, L'"');
							strQSearchStr.push_back(L'"');
							strQReplaceStr.insert(0, 1, L'"');
							strQReplaceStr.push_back(L'"');

							std::unique_ptr<PreRedrawItem> pitem(PreRedrawStack().empty()? nullptr : PreRedrawStack().pop());

							MsgCode=Message(0,4,MSG(MEditReplaceTitle),MSG(MEditAskReplace),
											strQSearchStr.data(),MSG(MEditAskReplaceWith),strQReplaceStr.data(),
											MSG(MEditReplace),MSG(MEditReplaceAll),MSG(MEditSkip),MSG(MEditCancel));
							if (pitem)
							{
								PreRedrawStack().push(std::move(pitem));
							}
							if (MsgCode==1)
								ReplaceAll = true;

							if (MsgCode==2)
								Skip=TRUE;

							if (MsgCode<0 || MsgCode==3)
							{
								UserBreak = true;
								break;
							}
						}

						if (ReplaceAll)
							UndoBlock = std::make_unique<undo_block>(this);

						if (!MsgCode || MsgCode==1)
						{
							Pasting++;

							// If Replace string doesn't contain control symbols (tab and return),
							// processed with fast method, otherwise use improved old one.
							//
							if (strReplaceStrCurrent.find_first_of(L"\t\r") != string::npos)
							{
								int SaveOvertypeMode=m_Flags.Check(FEDITOR_OVERTYPE);
								m_Flags.Set(FEDITOR_OVERTYPE);
								m_it_CurLine->SetOvertypeMode(true);

								int I=0;
								for (; SearchLength && I<static_cast<int>(strReplaceStrCurrent.size()); ++I, --SearchLength)
								{
									int Ch=strReplaceStrCurrent[I];

									if (Ch==KEY_TAB)
									{
										m_Flags.Clear(FEDITOR_OVERTYPE);
										m_it_CurLine->SetOvertypeMode(false);
										ProcessKey(Manager::Key(KEY_DEL));
										ProcessKey(Manager::Key(KEY_TAB));
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
										ProcessKey(Manager::Key(KEY_DEL));
									}

									if (Ch!=KEY_BS && !(Ch==KEY_DEL || Ch==KEY_NUMDEL))
										ProcessKey(Manager::Key(Ch));
								}

								bool NeedUpdateCurPtr = false;

								if (!SearchLength)
								{
									m_Flags.Clear(FEDITOR_OVERTYPE);
									m_it_CurLine->SetOvertypeMode(false);

									for (; I<static_cast<int>(strReplaceStrCurrent.size()); I++)
									{
										int Ch=strReplaceStrCurrent[I];
										NeedUpdateCurPtr = Ch == KEY_ENTER;
										if (Ch!=KEY_BS && !(Ch==KEY_DEL || Ch==KEY_NUMDEL))
											ProcessKey(Manager::Key(Ch));
									}
								}
								else
								{
									for (; SearchLength; SearchLength--)
									{
										ProcessKey(Manager::Key(KEY_DEL));
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
								int SStrLen=SearchLength;
								const auto& Str = m_it_CurLine->GetString();
								int LocalCurPos = m_it_CurLine->GetCurPos();
								string NewStr(Str, 0, LocalCurPos);
								NewStr += strReplaceStrCurrent;
								NewStr.append(Str.cbegin() + LocalCurPos + SStrLen, Str.cend());
								NewStr += m_it_CurLine->GetEOL();
								AddUndoData(UNDO_EDIT, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), m_it_CurLine.Number(), m_it_CurLine->GetCurPos());
								m_it_CurLine->SetString(NewStr);
								m_it_CurLine->SetCurPos(LocalCurPos + static_cast<int>(strReplaceStrCurrent.size()));

								if (EdOpt.SearchSelFound && !ReplaceMode)
								{
									UnmarkBlock();
									BeginStreamMarking(CurPtr);
									CurPtr->Select(LocalCurPos, LocalCurPos + static_cast<int>(strReplaceStrCurrent.size()));
								}

								if (at_end)
									at_end = static_cast<int>(strReplaceStrCurrent.size());

								Change(ECTYPE_CHANGED, m_it_CurLine.Number());
								TextChanged(1);
							}

							Pasting--;
						}
					}

					if (at_end)
						CurPtr->SetCurPos(m_FoundPos + at_end);

					if (!ReplaceMode)
						break;

					CurPos = m_it_CurLine->GetCurPos();
					CurPos += (Skip && !ReverseSearch ? 1:0);
					if (!Skip && ReverseSearch)
						(m_it_CurLine = CurPtr = m_FoundLine)->SetCurPos(CurPos = m_FoundPos);
				}
			}
			else
			{
				if (ReverseSearch)
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

	if(FindAllReferences && Match)
	{
		FindAllList->SetMenuFlags(VMENU_WRAPMODE | VMENU_SHOWAMPERSAND);
		FindAllList->SetPosition(-1, -1, 0, 0);
		FindAllList->SetTitle(string_format(MEditSearchStatistics, FindAllList->size(), AllRefLines));
		FindAllList->SetBottomTitle(MSG(MEditFindAllMenuFooter));
		FindAllList->SetHelp(L"FindAllMenu");
		FindAllList->SetId(EditorFindAllListId);

		bool MenuZoomed=true;

		int ExitCode=FindAllList->Run([&](const Manager::Key& RawKey)->int
		{
			const auto Key=RawKey();
			int SelectedPos=FindAllList->GetSelectPos();
			int KeyProcessed = 1;

			switch (Key)
			{
				case KEY_ADD:
					AddSessionBookmark();
					break;
				case KEY_CTRLENTER:
				case KEY_RCTRLENTER:
					{
						const auto& coord = *FindAllList->GetUserDataPtr<FindCoord>(SelectedPos);
						GoToLine(coord.Line);
						m_it_CurLine->SetCurPos(coord.Pos);
						if (EdOpt.SearchSelFound)
						{
							Pasting++;
							Lock();
							// if (!EdOpt.PersistentBlocks)
							UnmarkBlock();
							BeginStreamMarking(m_it_CurLine);
							m_it_CurLine->Select(coord.Pos, coord.Pos + coord.SearchLen);
							Unlock();
							Pasting--;
						}
						if (EdOpt.SearchCursorAtEnd)
						{
							m_it_CurLine->SetCurPos(coord.Pos + coord.SearchLen);
						}
						Show();
					}
					break;
				case KEY_CTRLUP: case KEY_RCTRLUP:
				case KEY_CTRLDOWN: case KEY_RCTRLDOWN:
					ProcessKey(Manager::Key(Key));
					break;
				case KEY_F5:
					MenuZoomed=!MenuZoomed;
					if(MenuZoomed)
					{
						FindAllList->SetPosition(-1, -1, 0, 0);
					}
					else
					{
						FindAllList->SetPosition(-1, ScrY-20, 0, ScrY-10);
					}
					Show();
					break;
				default:
					if ((Key>=KEY_CTRL0 && Key<=KEY_CTRL9) || (Key>=KEY_RCTRL0 && Key<=KEY_RCTRL9) ||
					   (Key>=KEY_CTRLSHIFT0 && Key<=KEY_CTRLSHIFT9) || (Key>=KEY_RCTRLSHIFT0 && Key<=KEY_RCTRLSHIFT9))
					{
						ProcessKey(Manager::Key(Key));
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
			const auto& coord = *FindAllList->GetUserDataPtr<FindCoord>(ExitCode);
			GoToLine(coord.Line);
			m_it_CurLine->SetCurPos(coord.Pos);
			if (EdOpt.SearchSelFound)
			{
				Pasting++;
				Lock();
				// if (!EdOpt.PersistentBlocks)
				UnmarkBlock();
				BeginStreamMarking(m_it_CurLine);
				m_it_CurLine->Select(coord.Pos, coord.Pos + coord.SearchLen);
				Unlock();
				Pasting--;
			}
			if (EdOpt.SearchCursorAtEnd)
			{
				m_it_CurLine->SetCurPos(coord.Pos + coord.SearchLen);
			}
			Show();
		}
	}

	if (!Match && !UserBreak)
		Message(MSG_WARNING,1,MSG(MEditSearchTitle),MSG(MEditNotFound),
		        strMsgStr.data(),MSG(MOk));

	return TRUE;
}

void Editor::PasteFromClipboard()
{
	if (m_Flags.Check(FEDITOR_LOCKMODE))
		return;

	clipboard_accessor Clip;

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

void Editor::Paste(const string& Data)
{
	if (m_Flags.Check(FEDITOR_LOCKMODE))
		return;

	if (!Data.empty())
	{
		AddUndoData(UNDO_BEGIN);
		m_Flags.Set(FEDITOR_NEWUNDO);
		TextChanged(1);
		int SaveOvertype=m_Flags.Check(FEDITOR_OVERTYPE);
		UnmarkBlock();
		Pasting++;
		Lock();

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
		bool oldAutoIndent=EdOpt.AutoIndent;

		const wchar_t* keep_eol = nullptr;
		if (EdOpt.KeepEOL)
		{
			auto line = m_it_CurLine;
			if (line != Lines.begin())
				--line;

			keep_eol = line->GetEOL();
			if (!*keep_eol)
				keep_eol = nullptr;
		}

		for (size_t i = 0, size = Data.size(); i != size; )
		{
			if (IsEol(Data[i]))
			{
				m_it_CurLine->Select(StartPos,-1);
				StartPos=0;
				EdOpt.AutoIndent = false;
				const auto PrevLine = m_it_CurLine;
				ProcessKey(Manager::Key(KEY_ENTER));

				int eol_len = 1;   // LF or CR
				if (Data[i] == L'\r' && i + 1 != size)
				{
					if (Data[i + 1] == L'\n')
					{
						eol_len = 2; // CRLF
					}
					else if (Data[i + 1] == L'\r' && i + 2 != size && Data[i + 2] == L'\n')
					{
						eol_len = 3; // CRCRLF
					}
				}

				if (keep_eol)
				{
					PrevLine->SetEOL(keep_eol);
				}
				else
				{
					PrevLine->SetEOL(string(Data, i, eol_len));
				}

				i += eol_len;
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
					AddUndoData(UNDO_EDIT, Str, m_it_CurLine->GetEOL(), m_it_CurLine.Number(), static_cast<int>(CurPos)); // EOL? - CurLine->GetEOL()  GlobalEOL   ""
					m_it_CurLine->InsertString(Data.data() + i, Pos - i);
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
		Unlock();
		AddUndoData(UNDO_END);
	}
}

void Editor::ProcessChar(wchar_t Char)
{
	if (Char != L'\0')
		ProcessKey(Manager::Key(Char));
	else
	{
		auto cur_pos = m_it_CurLine->GetCurPos();
		ProcessKey(Manager::Key(L' '));
		auto& line = m_it_CurLine->m_Str;
		if (line.size() > cur_pos && line[cur_pos] == L' ')
			line[cur_pos] = L'\0';
	}
}

void Editor::Copy(int Append)
{
	if (IsVerticalSelection())
	{
		VCopy(Append);
		return;
	}

	clipboard_accessor Clip;

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

	iterator SelEnd = Lines.end();

	for (iterator i = m_it_AnyBlockStart.base(), end = Lines.end(); i != end; ++i)
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
			TotalChars += wcslen(NullToEmpty(i->GetEOL()));
		}
		else
			TotalChars += EndSel - StartSel;
	}

	string CopyData;
	CopyData.reserve(TotalChars);

	for (const auto& i: make_range(m_it_AnyBlockStart.base(), SelEnd))
	{
		CopyData += i.GetSelString();

		intptr_t StartSel, EndSel;
		i.GetSelection(StartSel, EndSel);
		if (EndSel == -1)
		{
			CopyData += i.GetEOL();
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


	AddUndoData(UNDO_BEGIN);

	for (auto CurPtr = m_it_AnyBlockStart; CurPtr != EndIterator();)
	{
		TextChanged(1);
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
			AddUndoData(UNDO_EDIT, CurPtr->GetString(), CurPtr->GetEOL(), m_it_AnyBlockStart.Number(), CurPtr->GetCurPos());

		/* $ 17.09.2002 SKV
		  опять про выделение за концом строки.
		  InsertString добавит trailing space'ов
		*/
		if (StartSel > static_cast<intptr_t>(Length))
		{
			Length=StartSel;
			CurPtr->SetCurPos(static_cast<int>(Length));
			CurPtr->InsertString(L"",0);
		}

		auto TmpStr = CurPtr->GetString();
		const auto EndSeq = CurPtr->GetEOL();

		int DeleteNext=FALSE;

		if (EndSel==-1)
		{
			EndSel=Length;

			if (!IsLastLine(CurPtr))
				DeleteNext=TRUE;
		}

		TmpStr.erase(StartSel, EndSel - StartSel);

		int CurPos=StartSel;
		/*    if (CurPos>=StartSel)
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
					TmpStr.append(NextStr.cbegin() + NextEndSel, NextStr.cend());
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

		TmpStr+=EndSeq;
		CurPtr->SetString(TmpStr);

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
			CurPtr->RemoveSelection();
			++CurPtr;
		}
	}

	AddUndoData(UNDO_END);
	Unselect();
}


void Editor::UnmarkBlock()
{
	m_Flags.Clear(FEDITOR_CURPOSCHANGEDBYPLUGIN);
	if (!IsAnySelection())
		return;

	_SVS(SysLog(L"[%d] Editor::UnmarkBlock()",__LINE__));
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
	Show();
}

/* $ 07.03.2002 IS
   Удалить выделение, если оно пустое (выделено ноль символов в ширину)
*/
void Editor::UnmarkEmptyBlock()
{
	_SVS(SysLog(L"[%d] Editor::UnmarkEmptyBlock()",__LINE__));

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

void Editor::GoToLine(int Line)
{
	if (Line != m_it_CurLine.Number())
	{
		bool bReverse = false;
		int LastNumLine = m_it_CurLine.Number();
		int CurScrLine = CalcDistance(m_it_TopScreen,m_it_CurLine);
		int CurPos=m_it_CurLine->GetTabCurPos();
		int LeftPos=m_it_CurLine->GetLeftPos();

		if (Line < m_it_CurLine.Number())
		{
			if (Line > m_it_CurLine.Number() / 2)
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
			if (Line > (m_it_CurLine.Number() + (m_LinesCount - m_it_CurLine.Number()) / 2))
			{
				bReverse = true;
				m_it_CurLine = LastLine();
			}
		}

		if (bReverse)
		{
			while (m_it_CurLine.Number() > Line && m_it_CurLine != Lines.begin())
			{
				--m_it_CurLine;
			}
		}
		else
		{
			while (m_it_CurLine.Number() < Line && !IsLastLine(m_it_CurLine))
			{
				++m_it_CurLine;
			}
		}

		CurScrLine += m_it_CurLine.Number() - LastNumLine;

		if (CurScrLine<0 || CurScrLine>m_Y2-m_Y1)
			m_it_TopScreen=m_it_CurLine;

		m_it_CurLine->SetLeftPos(LeftPos);
		m_it_CurLine->SetTabCurPos(CurPos);
	}

// <GOTO_UNMARK:2>
//  if (!EdOpt.PersistentBlocks)
//     UnmarkBlock();
// </GOTO_UNMARK>
	Show();
	return;
}

void Editor::GoToPosition()
{
	DialogBuilder Builder(MEditGoToLine, L"EditorGotoPos");
	string strData;
	Builder.AddEditField(strData,28,L"LineNumber",DIF_FOCUS|DIF_HISTORY|DIF_USELASTHISTORY|DIF_NOAUTOCOMPLETE);
	Builder.AddOKCancel();
	Builder.ShowDialog();
	if(!strData.empty())
	{
		int LeftPos=m_it_CurLine->GetLeftPos();
		int NewLine=0, NewCol=0;
		try
		{
			GetRowCol(strData, NewLine, NewCol);
			GoToLine(NewLine);
			m_it_CurLine->SetLeftPos(LeftPos);
			m_it_CurLine->SetCurPos(NewCol);
		}
		catch (const std::exception&)
		{
			// TODO: log
			// maybe we need to display message in case of incorrect input
		}
		Show();
	}
}

void Editor::GetRowCol(const string& argv, int& row, int& col)
{
	static const std::wregex re(
		RE_BEGIN
		RE_ANY_WHITESPACE
		RE_C_GROUP(
			RE_C_GROUP(
				RE_C_GROUP(RE_ANY_OF(L"0-9") RE_ONE_OR_MORE_GREEDY) L"%"
			)
			RE_OR
			RE_C_GROUP(
				RE_C_GROUP(
					RE_ANY_OF(L"+-") RE_ZERO_OR_ONE_GREEDY
				)
				RE_ANY_OF(L"0-9") RE_ONE_OR_MORE_GREEDY
			)
		) RE_ZERO_OR_ONE_GREEDY
		RE_C_GROUP(
			RE_ANY_WHITESPACE
			RE_ANY_OF(L".,;:" RE_SPACE)
			RE_ANY_WHITESPACE
			RE_C_GROUP
			(
				RE_C_GROUP(RE_ANY_OF(L"+-") RE_ZERO_OR_ONE_GREEDY)
				RE_ANY_OF(L"0-9") RE_ONE_OR_MORE_GREEDY
			)
		) RE_ZERO_OR_ONE_GREEDY
		RE_ANY_WHITESPACE
		RE_END, std::regex::optimize);

	std::wsmatch SMatch;

	const auto get_match = [&SMatch](size_t index)
	{
		return string(SMatch[index].first, SMatch[index].second);
	};

	const auto is_match_empty = [&SMatch](size_t index)
	{
		return !SMatch[index].length();
	};

	if (std::regex_match(argv, SMatch, re))
	{
		enum
		{
			b_row=1,
			b_row_percent,
			b_row_percent_value,
			b_row_absolute,
			b_row_absolute_sign,
			b_col,
			b_col_absolute,
			b_col_sign
		};
		row = m_it_CurLine.Number();
		if (SMatch[b_row].matched)
		{
			if (SMatch[b_row_percent].matched)
			{
				row = std::stoi(get_match(b_row_percent_value));
				row = m_LinesCount * row / 100;
			}
			else if (SMatch[b_row_absolute].matched)
			{
				const auto y = std::stoi(get_match(b_row_absolute));
				row = is_match_empty(b_row_absolute_sign)?y-1:row+y;
			}
		}
		row = std::min(row, m_LinesCount - 1);
		const auto CurPtr=GetStringByNumber(row);
		col=CurPtr->TabPosToReal(m_it_CurLine->GetTabCurPos());
		if (SMatch[b_col].matched)
		{
			const auto x = std::stoi(get_match(b_col_absolute));
			col = is_match_empty(b_col_sign)?x-1:col+x;
		}
	}
	else throw std::invalid_argument("invalid syntax");
}

struct Editor::EditorUndoData
{
	NONCOPYABLE(EditorUndoData);
	TRIVIALLY_MOVABLE(EditorUndoData);

	int m_Type;
	int m_StrPos;
	int m_StrNum;
	wchar_t m_EOL[10];
	std::vector<wchar_t> m_Str;
	bookmark_list m_BM; //treat as uni-directional linked list

private:
	static size_t UndoDataSize;

public:
	static size_t GetUndoDataSize() { return UndoDataSize; }

	EditorUndoData(int Type, const string& Str, const wchar_t *Eol, int StrNum, int StrPos):
		m_Type(),
		m_StrPos(),
		m_StrNum(),
		m_EOL()
	{
		SetData(Type, Str, Eol, StrNum, StrPos);
	}

	~EditorUndoData()
	{
		UndoDataSize -= m_Str.size();
	}

	void SetData(int Type, const string& Str, const wchar_t *Eol, int StrNum, int StrPos)
	{
		m_Type = Type;
		m_StrPos = StrPos;
		m_StrNum = StrNum;
		UndoDataSize -= m_Str.size();
		UndoDataSize += Str.size();
		xwcsncpy(m_EOL, NullToEmpty(Eol), std::size(m_EOL) - 1);
		m_Str.assign(ALL_CONST_RANGE(Str));
		m_BM.clear();
	}

	static size_t HashBM(bookmark_list::iterator BM)
	{
		auto x = reinterpret_cast<size_t>(&*BM);
		x ^= (BM->Line << 16) ^ (BM->Cursor);
		return x;
	}
};

size_t Editor::EditorUndoData::UndoDataSize = 0;

void Editor::AddUndoData(int Type, const string& Str, const wchar_t *Eol, int StrNum, int StrPos)
{
	if (m_Flags.Check(FEDITOR_DISABLEUNDO))
		return;

	if (StrNum==-1)
		StrNum = m_it_CurLine.Number();
	auto u = UndoPos;
	if(u != UndoData.end())
		++u;
	else
		u = UndoData.begin();
	while(!UndoData.empty() && u != UndoData.end())
	{
		if (u == UndoSavePos)
		{
			UndoSavePos = UndoData.end();
			m_Flags.Set(FEDITOR_UNDOSAVEPOSLOST);
		}

		u = UndoData.erase(u);
	}

	auto PrevUndo=UndoData.end();
	if(!UndoData.empty())
	{
		--PrevUndo;
		switch (Type)
		{
		case UNDO_END:
			{
				if (PrevUndo->m_Type != UNDO_BEGIN)
				{
					if(PrevUndo != UndoData.begin())
						--PrevUndo;
					else
						PrevUndo = UndoData.end();
				}
				if (PrevUndo != UndoData.end() && PrevUndo->m_Type == UNDO_BEGIN)
				{
					bool eq = PrevUndo==UndoSavePos;
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

		case UNDO_EDIT:
			{
				if (!m_Flags.Check(FEDITOR_NEWUNDO) && PrevUndo->m_Type == UNDO_EDIT && StrNum == PrevUndo->m_StrNum &&
						(abs(StrPos-PrevUndo->m_StrPos)<=1 || abs(StrPos-LastChangeStrPos)<=1))
				{
					LastChangeStrPos=StrPos;
					return;
				}
			}
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

			if (ub->m_Type == UNDO_BEGIN)
				++UndoSkipLevel;

			if (ub->m_Type == UNDO_END && UndoSkipLevel>0)
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

	TextChanged(1);
	m_Flags.Set(FEDITOR_DISABLEUNDO);
	int level=0;
	auto uend = ustart;

	for (;;)
	{
		if (uend->m_Type == UNDO_BEGIN || uend->m_Type == UNDO_END)
		{
			int l = uend->m_Type == UNDO_BEGIN ? -1 : 1;
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

	for (;;)
	{
		if (ud->m_Type != UNDO_BEGIN && ud->m_Type != UNDO_END)
		{
			GoToLine(ud->m_StrNum);
		}

		switch (ud->m_Type)
		{
			case UNDO_INSSTR:
				ud->SetData(UNDO_DELSTR,m_it_CurLine->GetString(),m_it_CurLine->GetEOL(),ud->m_StrNum,ud->m_StrPos);
				DeleteString(m_it_CurLine, true);
				break;
			case UNDO_DELSTR:
				ud->m_Type = UNDO_INSSTR;
				Pasting++;

				if (m_it_CurLine.Number() < ud->m_StrNum)
				{
					ProcessKey(Manager::Key(KEY_END));
					ProcessKey(Manager::Key(KEY_ENTER));
				}
				else
				{
					ProcessKey(Manager::Key(KEY_HOME));
					ProcessKey(Manager::Key(KEY_ENTER));
					ProcessKey(Manager::Key(KEY_UP));
				}

				Pasting--;

				MoveSavedSessionBookmarksBack(ud->m_BM);

				m_it_CurLine->SetString(ud->m_Str.data(), ud->m_Str.size());
				m_it_CurLine->SetEOL(ud->m_EOL); // необходимо дополнительно выставлять, т.к. SetString вызывает Edit::SetString и... дальше по тексту
				Change(ECTYPE_CHANGED, m_it_CurLine.Number());

				break;
			case UNDO_EDIT:
			{
				EditorUndoData tmp(UNDO_EDIT, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), ud->m_StrNum, ud->m_StrPos);

				m_it_CurLine->SetString(ud->m_Str.data(), ud->m_Str.size());
				m_it_CurLine->SetEOL(ud->m_EOL); // необходимо дополнительно выставлять, т.к. SetString вызывает Edit::SetString и... дальше по тексту
				Change(ECTYPE_CHANGED, m_it_CurLine.Number());

				m_it_CurLine->SetCurPos(ud->m_StrPos);
				// BUGBUG
				ud->SetData(tmp.m_Type, string(ALL_CONST_RANGE(tmp.m_Str)), tmp.m_EOL, tmp.m_StrNum, tmp.m_StrPos);
				break;
			}
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
		TextChanged(0);

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


bool Editor::IsFileChanged() const
{
	return m_Flags.Check(FEDITOR_MODIFIED|FEDITOR_WASCHANGED);
}


bool Editor::IsFileModified() const
{
	return m_Flags.Check(FEDITOR_MODIFIED);
}

// используется в FileEditor
int64_t Editor::GetCurPos(bool file_pos, bool add_bom) const
{
	enum { Unknown = -1 };
	int Multiplier = 1;
	uint64_t bom = 0;

	if (file_pos)
	{
		if (m_codepage == CP_UNICODE || m_codepage == CP_REVERSEBOM)
		{
			Multiplier = sizeof(wchar_t);
			if (add_bom)
				bom = 1;
		}
		else if (m_codepage == CP_UTF8)
		{
			Multiplier = Unknown;
			if (add_bom)
				bom = 3;
		}
		else if (GetCodePageInfo(m_codepage).first > 1)
		{
			Multiplier = Unknown;
		}
	}

	const auto TotalSize = std::accumulate(Lines.cbegin(), m_it_TopScreen.cbase(), bom, [&](auto Value, const auto& line)
	{
		const auto& Str = line.GetString();
		return Value + (Multiplier != Unknown? Str.size() : unicode::to(m_codepage, Str, nullptr, 0)) + wcslen(line.GetEOL());
	});

	return Multiplier != Unknown? TotalSize * Multiplier : TotalSize;
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

	AddUndoData(UNDO_BEGIN);

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

		string TmpStr;
		const auto& CurStr = CurPtr->GetString();
		const auto EndSeq = CurPtr->GetEOL();

		if (!CurStr.empty())
		{
			if (CurStr.front() == L' ')
			{
				TmpStr = CurStr.substr(1);
			}
			else if (CurStr.front() == L'\t')
			{
				TmpStr.assign(EdOpt.TabSize - 1, L' ');
				TmpStr += CurStr.substr(1);
			}

			if ((EndSel == -1 || EndSel > StartSel) && IsSpace(CurStr.front()))
			{
				TmpStr.append(EndSeq);
				AddUndoData(UNDO_EDIT, CurStr, CurPtr->GetEOL(), CurPtr.Number(), 0); // EOL? - CurLine->GetEOL()  GlobalEOL   ""
				int CurPos = CurPtr->GetCurPos();
				CurPtr->SetString(TmpStr);
				CurPtr->SetCurPos(CurPos > 0? CurPos - 1 : CurPos);

				if (!MoveLine)
					CurPtr->Select(StartSel > 0? StartSel - 1 : StartSel, EndSel > 0? EndSel - 1 : EndSel);

				Change(ECTYPE_CHANGED, CurPtr.Number());
				TextChanged(1);
			}
		}
		++CurPtr;
		MoveLine = 0;
	}

	AddUndoData(UNDO_END);
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

	AddUndoData(UNDO_BEGIN);

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

		size_t Length = CurPtr->GetLength();

		if (Length && (EndSel == -1 || EndSel > StartSel))
		{
			const auto& CurStr = CurPtr->GetString();
			const auto EndSeq = CurPtr->GetEOL();
			string TmpStr(1, L' ');
			TmpStr += CurStr;
			TmpStr += EndSeq;

			AddUndoData(UNDO_EDIT, CurStr, CurPtr->GetEOL(), CurPtr.Number(), 0); // EOL? - CurLine->GetEOL()  GlobalEOL   ""
			int CurPos=CurPtr->GetCurPos();

			CurPtr->SetString(TmpStr);

			CurPtr->SetCurPos(CurPos+1);

			if (!MoveLine)
				CurPtr->Select(StartSel>0 ? StartSel+1:StartSel,EndSel>0 ? EndSel+1:EndSel);

			Change(ECTYPE_CHANGED, CurPtr.Number());
			TextChanged(1);
		}
		++CurPtr;
		MoveLine = 0;
	}

	AddUndoData(UNDO_END);
}


void Editor::DeleteVBlock()
{
	if (m_Flags.Check(FEDITOR_LOCKMODE) || VBlockSizeX<=0 || VBlockSizeY<=0)
		return;

	AddUndoData(UNDO_BEGIN);

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
		TextChanged(1);
		size_t TBlockX = CurPtr->TabPosToReal(VBlockX);
		size_t TBlockSizeX=CurPtr->TabPosToReal(VBlockX+VBlockSizeX) - CurPtr->TabPosToReal(VBlockX);
		const auto& CurStr = CurPtr->GetString();

		if (TBlockX >= CurStr.size())
			continue;

		AddUndoData(UNDO_EDIT, CurPtr->GetString(), CurPtr->GetEOL(), CurPtr.Number(), CurPtr->GetCurPos());
		string TmpStr(CurStr, 0, TBlockX);

		if (CurStr.size() > TBlockX + TBlockSizeX)
		{
			TmpStr.append(CurStr.cbegin() + TBlockX + TBlockSizeX, CurStr.cend());
		}

		TmpStr.append(CurPtr->GetEOL());
		size_t CurPos = CurPtr->GetCurPos();
		CurPtr->SetString(TmpStr);

		if (CurPos>TBlockX)
		{
			CurPos-=TBlockSizeX;

			if (CurPos<TBlockX)
				CurPos=TBlockX;
		}

		CurPtr->SetCurPos(static_cast<int>(CurPos));
		Change(ECTYPE_CHANGED, CurPtr.Number());
	}

	AddUndoData(UNDO_END);
	Unselect();
}

void Editor::VCopy(int Append)
{
	clipboard_accessor Clip;

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
	size_t TotalChars = (VBlockSizeX + 2)*VBlockSizeY;

	string CopyData;
	CopyData.reserve(TotalChars);

	auto CurPtr = m_it_AnyBlockStart;

	for (int Line = 0; CurPtr != Lines.end() && Line < VBlockSizeY; ++Line, ++CurPtr)
	{
		size_t TBlockX = CurPtr->TabPosToReal(VBlockX);
		size_t TBlockSizeX = CurPtr->TabPosToReal(VBlockX + VBlockSizeX) - TBlockX;
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

		CopyData.append(GetDefaultEOL());
	}

	return CopyData;
}

void Editor::VPaste(const string& Data)
{
	if (m_Flags.Check(FEDITOR_LOCKMODE))
		return;

	if (!Data.empty())
	{
		AddUndoData(UNDO_BEGIN);
		m_Flags.Set(FEDITOR_NEWUNDO);
		TextChanged(1);
		int SaveOvertype=m_Flags.Check(FEDITOR_OVERTYPE);
		UnmarkBlock();
		Pasting++;
		Lock();

		if (m_Flags.Check(FEDITOR_OVERTYPE))
		{
			m_Flags.Clear(FEDITOR_OVERTYPE);
			m_it_CurLine->SetOvertypeMode(false);
		}

		BeginVBlockMarking();
		int StartPos=m_it_CurLine->GetTabCurPos();
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
				int CurWidth=m_it_CurLine->GetTabCurPos()-StartPos;

				if (CurWidth>VBlockSizeX)
					VBlockSizeX=CurWidth;

				VBlockSizeY++;

				if (IsLastLine(m_it_CurLine))
				{
					if (i + NewLineLen < size)
					{
						ProcessKey(Manager::Key(KEY_END));
						ProcessKey(Manager::Key(KEY_ENTER));

						// Mantis 0002966: Неправильная вставка вертикального блока в конце файла
							repeat(StartPos, [this]{ ProcessKey(Manager::Key(L' ')); });
					}
				}
				else
				{
					ProcessKey(Manager::Key(KEY_DOWN));
					m_it_CurLine->SetTabCurPos(StartPos);
					m_it_CurLine->SetOvertypeMode(false);
				}

				i += NewLineLen;
			}
		}

		int CurWidth=m_it_CurLine->GetTabCurPos()-StartPos;

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
		Unlock();
		AddUndoData(UNDO_END);
	}
}


void Editor::VBlockShift(int Left)
{
	if (m_Flags.Check(FEDITOR_LOCKMODE) || (Left && !VBlockX) || VBlockSizeX<=0 || VBlockSizeY<=0)
		return;

	auto CurPtr = m_it_AnyBlockStart;
	AddUndoData(UNDO_BEGIN);

	for (int Line = 0; CurPtr != Lines.end() && Line < VBlockSizeY; ++Line, ++CurPtr)
	{
		TextChanged(1);
		size_t TBlockX = CurPtr->TabPosToReal(VBlockX);
		size_t TBlockSizeX = CurPtr->TabPosToReal(VBlockX + VBlockSizeX) - CurPtr->TabPosToReal(VBlockX);
		{
			const auto& CurStr = CurPtr->GetString();

			if (TBlockX > CurStr.size())
				continue;

			if ((Left && CurStr[TBlockX - 1] == L'\t') || (!Left && TBlockX + TBlockSizeX < CurStr.size() && CurStr[TBlockX + TBlockSizeX] == L'\t'))
			{
				CurPtr->ReplaceTabs();
				TBlockX = CurPtr->TabPosToReal(VBlockX);
				TBlockSizeX = CurPtr->TabPosToReal(VBlockX + VBlockSizeX) -
					CurPtr->TabPosToReal(VBlockX);
			}
		}

		AddUndoData(UNDO_EDIT, CurPtr->GetString(), CurPtr->GetEOL(), CurPtr.Number(), CurPtr->GetCurPos());

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

		while (!TmpStr.empty() && TmpStr.back() == L' ')
			TmpStr.pop_back();

		TmpStr += CurPtr->GetEOL();
		CurPtr->SetString(TmpStr);
		Change(ECTYPE_CHANGED, CurPtr.Number());
	}

	VBlockX+=Left ? -1:1;
	m_it_CurLine->SetTabCurPos(Left ? VBlockX:VBlockX+VBlockSizeX);
	AddUndoData(UNDO_END);
}

int Editor::EditorControl(int Command, intptr_t Param1, void *Param2)
{
	_ECTLLOG(CleverSysLog SL(L"Editor::EditorControl()"));
	_ECTLLOG(SysLog(L"Command=%s Param2=[%d/0x%08X]",_ECTL_ToName(Command),Param2,Param2));

	if(EditorControlLocked()) return FALSE;
	switch (Command)
	{
		case ECTL_GETSTRING:
		{
			EditorGetString *GetString=(EditorGetString *)Param2;

			if (CheckStructSize(GetString))
			{
				const auto CurPtr = GetStringByNumber(GetString->StringNumber);

				if (CurPtr == Lines.end())
				{
					_ECTLLOG(SysLog(L"EditorGetString => GetStringByNumber(%d) return nullptr",GetString->StringNumber));
					return FALSE;
				}

				const auto& Str = CurPtr->GetString();
				GetString->StringText = Str.data();
				GetString->StringEOL = CurPtr->GetEOL();
				GetString->StringLength = Str.size();
				GetString->SelStart=-1;
				GetString->SelEnd=0;
				int DestLine=GetString->StringNumber;

				if (DestLine==-1)
					DestLine = m_it_CurLine.Number();

				if (IsStreamSelection())
				{
					CurPtr->GetRealSelection(GetString->SelStart,GetString->SelEnd);
				}
				else if (IsVerticalSelection() && DestLine >= m_it_AnyBlockStart.Number() && DestLine < m_it_AnyBlockStart.Number() + VBlockSizeY)
				{
					GetString->SelStart=CurPtr->TabPosToReal(VBlockX);
					GetString->SelEnd=GetString->SelStart+
					                  CurPtr->TabPosToReal(VBlockX+VBlockSizeX)-
					                  CurPtr->TabPosToReal(VBlockX);
				}

				_ECTLLOG(SysLog(L"EditorGetString{"));
				_ECTLLOG(SysLog(L"  StringNumber    =%d",GetString->StringNumber));
				_ECTLLOG(SysLog(L"  StringText      ='%s'",GetString->StringText));
				_ECTLLOG(SysLog(L"  StringEOL       ='%s'",GetString->StringEOL?_SysLog_LinearDump((LPBYTE)GetString->StringEOL,StrLength(GetString->StringEOL)):L"(null)"));
				_ECTLLOG(SysLog(L"  StringLength    =%d",GetString->StringLength));
				_ECTLLOG(SysLog(L"  SelStart        =%d",GetString->SelStart));
				_ECTLLOG(SysLog(L"  SelEnd          =%d",GetString->SelEnd));
				_ECTLLOG(SysLog(L"}"));
				return TRUE;
			}

			break;
		}
		case ECTL_INSERTSTRING:
		{
			if (m_Flags.Check(FEDITOR_LOCKMODE))
			{
				_ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
				return FALSE;
			}
			else
			{
				TurnOffMarkingBlock();
				int Indent=Param2 && *(int *)Param2!=FALSE;

				if (!Indent)
					Pasting++;

				m_Flags.Set(FEDITOR_NEWUNDO);
				InsertString();
				Show();

				if (!Indent)
					Pasting--;
			}

			return TRUE;
		}
		case ECTL_INSERTTEXT:
		{
			if (!Param2)
				return FALSE;

			_ECTLLOG(SysLog(L"(const wchar_t *)Param2='%s'",(const wchar_t *)Param2));

			if (m_Flags.Check(FEDITOR_LOCKMODE))
			{
				_ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
				return FALSE;
			}
			else
			{
				TurnOffMarkingBlock();
				const wchar_t *Str=(const wchar_t *)Param2;
				Pasting++;
				Lock();

				while (*Str)
				{
					if(L'\n'==*Str)
					{
						--Pasting;
						InsertString();
						++Pasting;
					}
					else
					{
						ProcessKey(Manager::Key(*Str));
					}
					++Str;
				}

				Unlock();
				Pasting--;
			}

			return TRUE;
		}
		case ECTL_SETSTRING:
		{
			EditorSetString *SetString=(EditorSetString *)Param2;

			if (!CheckStructSize(SetString))
				break;

			_ECTLLOG(SysLog(L"EditorSetString{"));
			_ECTLLOG(SysLog(L"  StringNumber    =%d",SetString->StringNumber));
			_ECTLLOG(SysLog(L"  StringText      ='%s'",SetString->StringText));
			_ECTLLOG(SysLog(L"  StringEOL       ='%s'",SetString->StringEOL?_SysLog_LinearDump((LPBYTE)SetString->StringEOL,StrLength(SetString->StringEOL)):L"(null)"));
			_ECTLLOG(SysLog(L"  StringLength    =%d",SetString->StringLength));
			_ECTLLOG(SysLog(L"}"));

			if (m_Flags.Check(FEDITOR_LOCKMODE))
			{
				_ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
				break;
			}
			else
			{
				/* $ 06.08.2002 IS
				   Проверяем корректность StringLength и вернем FALSE, если оно меньше
				   нуля.
				*/
				int Length=SetString->StringLength;

				if (Length < 0)
				{
					_ECTLLOG(SysLog(L"SetString->StringLength < 0"));
					return FALSE;
				}

				const auto CurPtr = GetStringByNumber(SetString->StringNumber);

				if (CurPtr == Lines.end())
				{
					_ECTLLOG(SysLog(L"GetStringByNumber(%d) return nullptr",SetString->StringNumber));
					return FALSE;
				}

				const wchar_t *EOL = SetString->StringEOL? SetString->StringEOL : GlobalEOL.data();

				m_Flags.Set(FEDITOR_CURPOSCHANGEDBYPLUGIN);
				int DestLine=SetString->StringNumber;

				if (DestLine==-1)
					DestLine = m_it_CurLine.Number();

				string NewStr(SetString->StringText, Length);
				NewStr.append(EOL);

				AddUndoData(UNDO_EDIT, CurPtr->GetString(), CurPtr->GetEOL(), DestLine, CurPtr->GetCurPos());
				int CurPos=CurPtr->GetCurPos();
				intptr_t SelStart, SelEnd;
				CurPtr->GetSelection(SelStart, SelEnd);
				CurPtr->SetString(NewStr);
				CurPtr->SetCurPos(CurPos);
				CurPtr->Select(SelStart, SelEnd);
				Change(ECTYPE_CHANGED,DestLine);
				TextChanged(1);    // 10.08.2000 skv - Modified->TextChanged
			}

			return TRUE;
		}
		case ECTL_DELETESTRING:
		{
			if (m_Flags.Check(FEDITOR_LOCKMODE))
			{
				_ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
				return FALSE;
			}

			TurnOffMarkingBlock();
			DeleteString(m_it_CurLine, false);

			return TRUE;
		}
		case ECTL_DELETECHAR:
		{
			if (m_Flags.Check(FEDITOR_LOCKMODE))
			{
				_ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
				return FALSE;
			}

			TurnOffMarkingBlock();
			Pasting++;
			ProcessKey(Manager::Key(KEY_DEL));
			Pasting--;
			return TRUE;
		}
		case ECTL_GETINFO:
		{
			EditorInfo *Info=(EditorInfo *)Param2;

			if (CheckStructSize(Info))
			{
				Info->EditorID=Editor::EditorID;
				Info->WindowSizeX=ObjWidth();
				Info->WindowSizeY = ObjHeight();
				Info->TotalLines=m_LinesCount;
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

				if (Global->Opt->EdOpt.ShowTitleBar)
					Info->Options|=EOPT_SHOWTITLEBAR;

				if (Global->Opt->EdOpt.ShowKeyBar)
					Info->Options|=EOPT_SHOWKEYBAR;

				if (EdOpt.ShowScrollBar && ScrollBarRequired(ObjHeight(), m_LinesCount))
					Info->Options |= EOPT_SHOWSCROLLBAR;

				Info->TabSize=EdOpt.TabSize;
				Info->BookmarkCount=BOOKMARK_COUNT;
				Info->SessionBookmarkCount=GetSessionBookmarks(nullptr);
				Info->CurState=m_Flags.Check(FEDITOR_LOCKMODE)?ECSTATE_LOCKED:0;
				Info->CurState|=!m_Flags.Check(FEDITOR_MODIFIED)?ECSTATE_SAVED:0;
				Info->CurState|=m_Flags.Check(FEDITOR_MODIFIED|FEDITOR_WASCHANGED)?ECSTATE_MODIFIED:0;
				Info->CodePage=m_codepage;
				return TRUE;
			}

			_ECTLLOG(SysLog(L"Error: !Param2"));
			return FALSE;
		}
		case ECTL_SETPOSITION:
		{
			EditorSetPosition *Pos=(EditorSetPosition *)Param2;
			if (CheckStructSize(Pos))
			{
				_ECTLLOG(SysLog(L"EditorSetPosition{"));
				_ECTLLOG(SysLog(L"  CurLine       = %d",Pos->CurLine));
				_ECTLLOG(SysLog(L"  CurPos        = %d",Pos->CurPos));
				_ECTLLOG(SysLog(L"  CurTabPos     = %d",Pos->CurTabPos));
				_ECTLLOG(SysLog(L"  TopScreenLine = %d",Pos->TopScreenLine));
				_ECTLLOG(SysLog(L"  LeftPos       = %d",Pos->LeftPos));
				_ECTLLOG(SysLog(L"  Overtype      = %d",Pos->Overtype));
				_ECTLLOG(SysLog(L"}"));
				Lock();

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

					for (int I = m_it_CurLine.Number(); I>0 && m_it_CurLine.Number() - I<m_Y2 - m_Y1 && I != Pos->TopScreenLine; I--)
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

				Unlock();
				return TRUE;
			}

			_ECTLLOG(SysLog(L"Error: !Param2"));
			break;
		}
		case ECTL_SELECT:
		{
			EditorSelect *Sel=(EditorSelect *)Param2;
			if (CheckStructSize(Sel))
			{
				_ECTLLOG(SysLog(L"EditorSelect{"));
				_ECTLLOG(SysLog(L"  BlockType     =%s (%d)",(Sel->BlockType==BTYPE_NONE?L"BTYPE_NONE":(Sel->BlockType==BTYPE_STREAM?L"":(Sel->BlockType==BTYPE_COLUMN?L"BTYPE_COLUMN":L"BTYPE_?????"))),Sel->BlockType));
				_ECTLLOG(SysLog(L"  BlockStartLine=%d",Sel->BlockStartLine));
				_ECTLLOG(SysLog(L"  BlockStartPos =%d",Sel->BlockStartPos));
				_ECTLLOG(SysLog(L"  BlockWidth    =%d",Sel->BlockWidth));
				_ECTLLOG(SysLog(L"  BlockHeight   =%d",Sel->BlockHeight));
				_ECTLLOG(SysLog(L"}"));

				if (Sel->BlockType==BTYPE_NONE || Sel->BlockStartPos==-1)
				{
					UnmarkBlock();
					return TRUE;
				}

				if (Sel->BlockHeight < 1)
				{
					_ECTLLOG(SysLog(L"Error: EditorSelect::BlockHeight < 1"));
					return FALSE;
				}

				auto CurPtr = GetStringByNumber(Sel->BlockStartLine);

				if (CurPtr == Lines.end())
				{
					_ECTLLOG(SysLog(L"Error: start line BlockStartLine=%d not found, GetStringByNumber(%d) return nullptr",Sel->BlockStartLine,Sel->BlockStartLine));
					return FALSE;
				}

				UnmarkBlock();

				m_Flags.Set(FEDITOR_CURPOSCHANGEDBYPLUGIN);

				if (Sel->BlockType==BTYPE_STREAM)
				{
					BeginStreamMarking(CurPtr);

					for (int i=0; i < Sel->BlockHeight; i++)
					{
						int SelStart= i? 0:Sel->BlockStartPos;
						int SelEnd  = (i < Sel->BlockHeight-1) ? -1 : Sel->BlockStartPos+Sel->BlockWidth;
						CurPtr->Select(SelStart,SelEnd);
						++CurPtr;

						if (CurPtr == Lines.end())
							return TRUE; // ранее было FALSE
					}
				}
				else if (Sel->BlockType==BTYPE_COLUMN)
				{
					BeginVBlockMarking(CurPtr);

					VBlockX=Sel->BlockStartPos;

					VBlockSizeX=Sel->BlockWidth;
					VBlockSizeY=Sel->BlockHeight;

					if (VBlockSizeX < 0)
					{
						VBlockSizeX=-VBlockSizeX;
						VBlockX-=VBlockSizeX;

						if (VBlockX < 0)
							VBlockX=0;
					}
				}

				return TRUE;
			}

			_ECTLLOG(SysLog(L"Error: !Param2"));
			break;
		}
		case ECTL_REDRAW:
		{
			Show();
			Global->ScrBuf->Flush();
			return TRUE;
		}
		case ECTL_TABTOREAL:
		{
			EditorConvertPos *ecp=(EditorConvertPos *)Param2;
			if (CheckStructSize(ecp))
			{
				const auto CurPtr = GetStringByNumber(ecp->StringNumber);

				if (CurPtr == Lines.end())
				{
					_ECTLLOG(SysLog(L"GetStringByNumber(%d) return nullptr",ecp->StringNumber));
					return FALSE;
				}

				ecp->DestPos=CurPtr->TabPosToReal(ecp->SrcPos);
				_ECTLLOG(SysLog(L"EditorConvertPos{"));
				_ECTLLOG(SysLog(L"  StringNumber =%d",ecp->StringNumber));
				_ECTLLOG(SysLog(L"  SrcPos       =%d",ecp->SrcPos));
				_ECTLLOG(SysLog(L"  DestPos      =%d",ecp->DestPos));
				_ECTLLOG(SysLog(L"}"));
				return TRUE;
			}

			break;
		}
		case ECTL_REALTOTAB:
		{
			EditorConvertPos *ecp=(EditorConvertPos *)Param2;
			if (CheckStructSize(ecp))
			{
				const auto CurPtr = GetStringByNumber(ecp->StringNumber);

				if (CurPtr == Lines.end())
				{
					_ECTLLOG(SysLog(L"GetStringByNumber(%d) return nullptr",ecp->StringNumber));
					return FALSE;
				}

				ecp->DestPos=CurPtr->RealPosToTab(ecp->SrcPos);
				_ECTLLOG(SysLog(L"EditorConvertPos{"));
				_ECTLLOG(SysLog(L"  StringNumber =%d",ecp->StringNumber));
				_ECTLLOG(SysLog(L"  SrcPos       =%d",ecp->SrcPos));
				_ECTLLOG(SysLog(L"  DestPos      =%d",ecp->DestPos));
				_ECTLLOG(SysLog(L"}"));
				return TRUE;
			}

			break;
		}
		case ECTL_EXPANDTABS:
		{
			if (m_Flags.Check(FEDITOR_LOCKMODE))
			{
				_ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
				return FALSE;
			}
			else
			{
				intptr_t StringNumber=*(intptr_t*)Param2;
				const auto CurPtr = GetStringByNumber(StringNumber);

				if (CurPtr == Lines.end())
				{
					_ECTLLOG(SysLog(L"GetStringByNumber(%d) return nullptr",StringNumber));
					return FALSE;
				}

				AddUndoData(UNDO_EDIT, CurPtr->GetString(), CurPtr->GetEOL(), StringNumber, CurPtr->GetCurPos());
				if(CurPtr->ReplaceTabs()) Change(ECTYPE_CHANGED,StringNumber);
			}

			return TRUE;
		}
		// TODO: Если DI_MEMOEDIT не будет юзать раскраску, то должно выполняется в FileEditor::EditorControl(), в диалоге - нафиг ненать
		case ECTL_ADDCOLOR:
		{
			EditorColor *col=(EditorColor *)Param2;
			if (CheckStructSize(col))
			{
				_ECTLLOG(SysLog(L"EditorColor{"));
				_ECTLLOG(SysLog(L"  StringNumber=%d",col->StringNumber));
				_ECTLLOG(SysLog(L"  ColorItem   =%d (0x%08X)",col->ColorItem,col->ColorItem));
				_ECTLLOG(SysLog(L"  StartPos    =%d",col->StartPos));
				_ECTLLOG(SysLog(L"  EndPos      =%d",col->EndPos));
				_ECTLLOG(SysLog(L"  Color       =%d/%d (0x%08X/0x%08X)",col->Color.ForegroundColor,col->Color.BackgroundColor,col->Color.ForegroundColor,col->Color.BackgroundColor));
				_ECTLLOG(SysLog(L"}"));
				ColorItem newcol;
				newcol.StartPos=col->StartPos;
				newcol.EndPos=col->EndPos;
				newcol.SetColor(col->Color);
				newcol.Flags=col->Flags;
				newcol.SetOwner(col->Owner);
				newcol.Priority=col->Priority;
				const auto CurPtr = GetStringByNumber(col->StringNumber);

				if (CurPtr == Lines.end())
				{
					_ECTLLOG(SysLog(L"GetStringByNumber(%d) return nullptr",col->StringNumber));
					return FALSE;
				}

				CurPtr->AddColor(newcol, SortColorLocked());
				if (col->Flags&ECF_AUTODELETE) m_AutoDeletedColors.emplace(&*CurPtr);
				SortColorUpdate=true;
				return TRUE;
			}

			break;
		}
		// TODO: Если DI_MEMOEDIT не будет юзать раскраску, то должно выполняется в FileEditor::EditorControl(), в диалоге - нафиг ненать
		case ECTL_GETCOLOR:
		{
			EditorColor *col=(EditorColor *)Param2;
			if (CheckStructSize(col))
			{
				const auto CurPtr = GetStringByNumber(col->StringNumber);

				if (CurPtr == Lines.end())
				{
					_ECTLLOG(SysLog(L"GetStringByNumber(%d) return nullptr",col->StringNumber));
					return FALSE;
				}

				ColorItem curcol;

				if (!CurPtr->GetColor(curcol,col->ColorItem))
				{
					_ECTLLOG(SysLog(L"GetColor() return false"));
					return FALSE;
				}

				col->StartPos=curcol.StartPos-m_X1;
				col->EndPos=curcol.EndPos-m_X1;
				col->Color=curcol.GetColor();
				col->Flags=curcol.Flags;
				col->Owner=curcol.GetOwner();
				col->Priority=curcol.Priority;
				_ECTLLOG(SysLog(L"EditorColor{"));
				_ECTLLOG(SysLog(L"  StringNumber=%d",col->StringNumber));
				_ECTLLOG(SysLog(L"  ColorItem   =%d (0x%08X)",col->ColorItem,col->ColorItem));
				_ECTLLOG(SysLog(L"  StartPos    =%d",col->StartPos));
				_ECTLLOG(SysLog(L"  EndPos      =%d",col->EndPos));
				_ECTLLOG(SysLog(L"  Color       =%d/%d (0x%08X/0x%08X)",col->Color.ForegroundColor,col->Color.BackgroundColor,col->Color.ForegroundColor,col->Color.BackgroundColor));
				_ECTLLOG(SysLog(L"}"));
				return TRUE;
			}

			break;
		}
		case ECTL_DELCOLOR:
		{
			const auto col = reinterpret_cast<const EditorDeleteColor*>(Param2);
			if (CheckStructSize(col))
			{
				const auto CurPtr = GetStringByNumber(col->StringNumber);

				if (CurPtr == Lines.end())
				{
					_ECTLLOG(SysLog(L"GetStringByNumber(%d) return nullptr",col->StringNumber));
					return FALSE;
				}

				SortColorUpdate=true;

				CurPtr->DeleteColor(
					[&](const ColorItem& Item) { return (col->StartPos == -1 || col->StartPos == Item.StartPos) && col->Owner == Item.GetOwner(); },
					SortColorLocked());
				return TRUE;
			}

			break;
		}
		/* $ 16.02.2001 IS
		     Изменение некоторых внутренних настроек редактора. Param2 указывает на
		     структуру EditorSetParameter
		*/
		case ECTL_SETPARAM:
		{
			EditorSetParameter *espar=(EditorSetParameter *)Param2;
			if (CheckStructSize(espar))
			{
				int rc=TRUE;
				_ECTLLOG(SysLog(L"EditorSetParameter{"));
				_ECTLLOG(SysLog(L"  Type        =%s",_ESPT_ToName(espar->Type)));

				switch (espar->Type)
				{
					case ESPT_GETWORDDIV:
						_ECTLLOG(SysLog(L"  wszParam    =(%p)",espar->wszParam));

						if (espar->wszParam && espar->Size)
							xwcsncpy(espar->wszParam,EdOpt.strWordDiv.data(),espar->Size);

						rc=(int)EdOpt.strWordDiv.Get().size()+1;
						break;
					case ESPT_SETWORDDIV:
						_ECTLLOG(SysLog(L"  wszParam    =[%s]",espar->wszParam));
						SetWordDiv((!espar->wszParam || !*espar->wszParam)?Global->Opt->EdOpt.strWordDiv.data():espar->wszParam);
						break;
					case ESPT_TABSIZE:
						_ECTLLOG(SysLog(L"  iParam      =%d",espar->iParam));
						SetTabSize(espar->iParam);
						break;
					case ESPT_EXPANDTABS:
						_ECTLLOG(SysLog(L"  iParam      =%s",espar->iParam?L"On":L"Off"));
						SetConvertTabs(espar->iParam);
						break;
					case ESPT_AUTOINDENT:
						_ECTLLOG(SysLog(L"  iParam      =%s",espar->iParam?L"On":L"Off"));
						SetAutoIndent(espar->iParam != 0);
						break;
					case ESPT_CURSORBEYONDEOL:
						_ECTLLOG(SysLog(L"  iParam      =%s",espar->iParam?L"On":L"Off"));
						SetCursorBeyondEOL(espar->iParam != 0);
						break;
					case ESPT_CHARCODEBASE:
						_ECTLLOG(SysLog(L"  iParam      =%s",(!espar->iParam?L"0 (Oct)":(espar->iParam==1?L"1 (Dec)":(espar->iParam==2?L"2 (Hex)":L"?????")))));
						SetCharCodeBase(espar->iParam);
						break;
					case ESPT_CODEPAGE:
					{
						uintptr_t cp = espar->iParam;
						if (HostFileEditor) {
							rc = HostFileEditor->SetCodePage(cp, false, true);
							rc = rc >= 0 ? TRUE : FALSE;
						}
						else {
							rc = cp != CP_DEFAULT && Codepages().IsCodePageSupported(cp) && SetCodePage(cp) ? TRUE : FALSE;
						}
						if (rc > 0)
							Show();
					}break;
					/* $ 29.10.2001 IS изменение настройки "Сохранять позицию файла" */
					case ESPT_SAVEFILEPOSITION:
						_ECTLLOG(SysLog(L"  iParam      =%s",espar->iParam?L"On":L"Off"));
						SetSavePosMode(espar->iParam, -1);
						break;
						/* $ 23.03.2002 IS запретить/отменить изменение файла */
					case ESPT_LOCKMODE:
						_ECTLLOG(SysLog(L"  iParam      =%s",espar->iParam?L"On":L"Off"));
						m_Flags.Change(FEDITOR_LOCKMODE, espar->iParam!=0);
						break;
					case ESPT_SHOWWHITESPACE:
						SetShowWhiteSpace(espar->iParam);
						break;
					default:
						_ECTLLOG(SysLog(L"}"));
						return FALSE;
				}

				_ECTLLOG(SysLog(L"}"));
				return rc;
			}

			return  FALSE;
		}
		case ECTL_DELETEBLOCK:
		{
			if (m_Flags.Check(FEDITOR_LOCKMODE) || !IsAnySelection())
			{

				_ECTLLOG(if (m_Flags.Check(FEDITOR_LOCKMODE))SysLog(L"FEDITOR_LOCKMODE!"));

				_ECTLLOG(if (!(m_it_VBlockStart || m_it_BlockStart))SysLog(L"Not selected block!"));

				return FALSE;
			}

			TurnOffMarkingBlock();
			DeleteBlock();
			Show();
			return TRUE;
		}
		case ECTL_UNDOREDO:
		{
			EditorUndoRedo *eur=(EditorUndoRedo *)Param2;
			if (CheckStructSize(eur))
			{

				switch (eur->Command)
				{
					case EUR_BEGIN:
						AddUndoData(UNDO_BEGIN);
						return TRUE;
					case EUR_END:
						AddUndoData(UNDO_END);
						return TRUE;
					case EUR_UNDO:
					case EUR_REDO:
						Lock();
						Undo(eur->Command==EUR_REDO);
						Unlock();
						return TRUE;
				}
			}

			return FALSE;
		}
		case ECTL_SUBSCRIBECHANGEEVENT:
		case ECTL_UNSUBSCRIBECHANGEEVENT:
		{
			const auto esce = static_cast<const EditorSubscribeChangeEvent*>(Param2);
			if (CheckStructSize(esce))
			{
				if (Command == ECTL_UNSUBSCRIBECHANGEEVENT)
				{
					ChangeEventSubscribers.erase(esce->PluginId);
					return TRUE;
				}

				ChangeEventSubscribers.emplace(esce->PluginId);
				return TRUE;
			}
			break;
		}
	}

	return FALSE;
}

bool Editor::SetBookmark(int Pos)
{
	if (Pos < static_cast<int>(m_SavePos.size()))
	{
		auto& Bookmark = m_SavePos[Pos];
		Bookmark.Line = m_it_CurLine.Number();
		Bookmark.LinePos = m_it_CurLine->GetCurPos();
		Bookmark.LeftPos = m_it_CurLine->GetLeftPos();
		Bookmark.ScreenLine = CalcDistance(m_it_TopScreen, m_it_CurLine);
		return true;
	}

	return false;
}

bool Editor::GotoBookmark(int Pos)
{
	if (Pos < static_cast<int>(m_SavePos.size()))
	{
		auto& Bookmark = m_SavePos[Pos];
		if (Bookmark.Line != POS_NONE)
		{
			GoToLine(Bookmark.Line);
			m_it_CurLine->SetCurPos(Bookmark.LinePos);
			m_it_CurLine->SetLeftPos(Bookmark.LeftPos);
			m_it_TopScreen=m_it_CurLine;

			for (int i = 0; i < Bookmark.ScreenLine && m_it_TopScreen != Lines.begin(); ++i)
				--m_it_TopScreen;

			if (!EdOpt.PersistentBlocks)
				UnmarkBlock();

			Show();
		}

		return true;
	}

	return false;
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
	if (m_Flags.Check(FEDITOR_DISABLEUNDO) || UndoPos->m_Type != UNDO_DELSTR)
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
	if (!SessionBookmarks.empty() && ((int)SessionPos->Line != m_it_CurLine.Number() || (int)SessionPos->Cursor != m_it_CurLine->GetCurPos()))
	{
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

	return false;
}

void Editor::AddSessionBookmark(bool NewPos)
{
	//remove all subsequent bookmarks
	if (!SessionBookmarks.empty())
	{
		const auto Next = std::next(SessionPos);
		//if (Next != SessionBookmarks.end())
			SessionBookmarks.erase(Next, SessionBookmarks.end());
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
			if (std::next(SessionPos) == SessionBookmarks.end() && ((int)SessionPos->Line != m_it_CurLine.Number() || (int)SessionPos->Cursor != m_it_CurLine->GetCurPos()))
				AddSessionBookmark(false);
		}

		return PrevSessionBookmark();
	}

	return false;
}

bool Editor::PrevSessionBookmark()
{
	if (!SessionBookmarks.empty())
	{
		if (SessionPos != SessionBookmarks.begin()) // If not first bookmark - go
		{
			--SessionPos;
		}

		return RestoreSessionBookmark();
	}

	return false;
}

bool Editor::NextSessionBookmark()
{
	if (!SessionBookmarks.empty())
	{
		const auto Next = std::next(SessionPos);
		if (Next != SessionBookmarks.end())
		{
			SessionPos = Next;
		}

		return RestoreSessionBookmark();
	}

	return false;
}

bool Editor::LastSessionBookmark()
{
	if (!SessionBookmarks.empty())
	{
		SessionPos = std::prev(SessionBookmarks.end());
		return RestoreSessionBookmark();
	}

	return false;
}

bool Editor::GotoSessionBookmark(int iIdx)
{
	if (!SessionBookmarks.empty())
	{
		const auto sb_temp = PointerToSessionBookmark(iIdx);
		if (sb_temp != SessionBookmarks.end())
		{
			SessionPos=sb_temp;
			return RestoreSessionBookmark();
		}
	}

	return false;
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

	if (!SessionBookmarks.empty() && Param)
	{
		const auto sb_temp = PointerToSessionBookmark(iIdx);
		Param->Line = sb_temp->Line;
		Param->Cursor     =sb_temp->Cursor;
		Param->LeftPos    =sb_temp->LeftPos;
		Param->ScreenLine =sb_temp->ScreenLine;

		return true;
	}

	return false;
}

size_t Editor::GetSessionBookmarks(EditorBookmarks *Param)
{
	size_t Result = 0;

	if (!SessionBookmarks.empty())
	{
		Result = SessionBookmarks.size();
		if (Param)
		{
			if (Param->Line || Param->Cursor || Param->LeftPos || Param->ScreenLine)
			{
				Result = SessionBookmarks.size();
				for_each_cnt(CONST_RANGE(SessionBookmarks, i, size_t index)
				{
					if (Param->Line)
						Param->Line[index] = i.Line;

					if (Param->Cursor)
						Param->Cursor[index] = i.Cursor;

					if (Param->LeftPos)
						Param->LeftPos[index] = i.LeftPos;

					if (Param->ScreenLine)
						Param->ScreenLine[index] = i.ScreenLine;
				});
			}
			else
				Result = 0;
		}
	}
	return Result;
}

size_t Editor::GetSessionBookmarksForPlugin(EditorBookmarks *Param)
{
	size_t count=GetSessionBookmarks(nullptr),size;
	if(InitSessionBookmarksForPlugin(Param,count,size)) GetSessionBookmarks(Param);
	return size;
}

bool Editor::InitSessionBookmarksForPlugin(EditorBookmarks *Param,size_t Count,size_t& Size)
{
	Size=sizeof(EditorBookmarks)+sizeof(intptr_t)*4*Count;
	if(Param&&Param->Size>=Size)
	{
		intptr_t* data=(intptr_t*)(Param+1);
		Param->Count=Count;
		Param->Line=data;
		Param->Cursor=data+Count;
		Param->ScreenLine=data+2*Count;
		Param->LeftPos=data+3*Count;
		return true;
	}
	return false;
}

Editor::numbered_iterator Editor::GetStringByNumber(int DestLine)
{
	if (DestLine == m_it_CurLine.Number() || DestLine < 0)
	{
		return m_it_LastGetLine = m_it_CurLine;
	}
	else if (DestLine >= m_LinesCount)
	{
		return EndIterator();
	}
	else if (!DestLine)
	{
		return m_it_LastGetLine = FirstLine();
	}
	else if (DestLine == m_LinesCount - 1)
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

	bool Forward = (DestLine>CurPtr.Number() && DestLine < CurPtr.Number() + (m_LinesCount - CurPtr.Number()) / 2) || (DestLine < CurPtr.Number() / 2);

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
		for (int Line = CurPtr.Number(); Line != DestLine; ++Line)
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
		for (int Line = CurPtr.Number(); Line != DestLine; --Line)
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

void Editor::SetReplaceMode(bool Mode)
{
	::ReplaceMode=Mode;
}

int Editor::GetLineCurPos() const
{
	return m_it_CurLine->GetTabCurPos();
}

void Editor::ProcessVBlockMarking()
{
	if (m_Flags.Check(FEDITOR_CURPOSCHANGEDBYPLUGIN))
	{
		int CurPos=m_it_CurLine->GetTabCurPos();
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
	int x=GetLineCurPos();

	if (x==VBlockX+VBlockSizeX)   // ничего не случилось, никаких табуляций нет
		return;

	if (x>VBlockX)    // курсор убежал внутрь блока
	{
		VBlockSizeX=x-VBlockX;
		//_D(SysLog(L"x>VBlockX");
	}
	else if (x<VBlockX)   // курсор убежал за начало блока
	{
		int c2=VBlockX;

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

		//_D(SysLog(L"x<VBlockX"));
	}
	else if (x==VBlockX && x!=PrevX)
	{
		VBlockSizeX=0;  // ширина в 0, потому прыгнули прям на табуляцию
		//_D(SysLog(L"x==VBlockX && x!=PrevX"));
	}

	// примечание
	//   случай x>VBLockX+VBlockSizeX не может быть
	//   потому что курсор прыгает назад на табуляцию, но не вперед
	//_D(SysLog(L"AdjustVBlock, changed vblock  VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
}


void Editor::Xlat()
{
	const auto XLatStr = [&](Edit::edit_string& Str, int StartPos, int EndPos)
	{
		std::vector<wchar_t> Buffer(ALL_CONST_RANGE(Str));
		::Xlat(Buffer.data(), StartPos, EndPos, Global->Opt->XLat.Flags);
		Str.assign(Buffer.data(), Buffer.size());
	};

	bool DoXlat = false;
	AddUndoData(UNDO_BEGIN);

	if (IsVerticalSelection())
	{
		auto CurPtr = m_it_AnyBlockStart;

		for (int Line = 0; CurPtr != Lines.end() && Line < VBlockSizeY; ++Line, ++CurPtr)
		{
			size_t TBlockX = CurPtr->TabPosToReal(VBlockX);
			size_t TBlockSizeX = CurPtr->TabPosToReal(VBlockX + VBlockSizeX) - CurPtr->TabPosToReal(VBlockX);
			size_t CopySize = CurPtr->GetLength() - TBlockX;

			if (CopySize>TBlockSizeX)
				CopySize=TBlockSizeX;

			AddUndoData(UNDO_EDIT, CurPtr->GetString(), CurPtr->GetEOL(), CurPtr.Number(), m_it_CurLine->GetCurPos());
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
					EndSel=CurPtr->GetLength();//StrLength(CurPtr->Str);

				AddUndoData(UNDO_EDIT, CurPtr->GetString(), CurPtr->GetEOL(), CurPtr.Number(), m_it_CurLine->GetCurPos());
				XLatStr(CurPtr->m_Str, StartSel, EndSel);
				Change(ECTYPE_CHANGED, CurPtr.Number());
				++CurPtr;
			}

			DoXlat = true;
		}
		else
		{
			auto& Str = m_it_CurLine->m_Str;
			int start=m_it_CurLine->GetCurPos(), StrSize=m_it_CurLine->GetLength();//StrLength(Str);
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

				AddUndoData(UNDO_EDIT, m_it_CurLine->GetString(), m_it_CurLine->GetEOL(), m_it_CurLine.Number(), start);
				XLatStr(Str, start, end);
				Change(ECTYPE_CHANGED, m_it_CurLine.Number());
			}
		}
	}

	AddUndoData(UNDO_END);

	if (DoXlat)
		TextChanged(1);
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
		std::for_each(RANGE(Lines, i)
		{
			if (NewMode == EXPAND_ALLTABS)
			{
				if(i.ReplaceTabs())
				{
					Change(ECTYPE_CHANGED, Pos);
				}
			}
			++Pos;
		});
	}
}

void Editor::SetDelRemovesBlocks(bool NewMode)
{
	if (NewMode!=EdOpt.DelRemovesBlocks)
	{
		EdOpt.DelRemovesBlocks=NewMode;
		std::for_each(RANGE(Lines, i)
		{
			i.SetDelRemovesBlocks(NewMode);
		});
	}
}

void Editor::SetShowWhiteSpace(int NewMode)
{
	if (NewMode!=EdOpt.ShowWhiteSpace)
	{
		EdOpt.ShowWhiteSpace=NewMode;
		std::for_each(RANGE(Lines, i)
		{
			i.SetShowWhiteSpace(NewMode);
		});
	}
}

void Editor::SetPersistentBlocks(bool NewMode)
{
	if (NewMode!=EdOpt.PersistentBlocks)
	{
		EdOpt.PersistentBlocks=NewMode;
		std::for_each(RANGE(Lines, i)
		{
			i.SetPersistentBlocks(NewMode);
		});
	}
}

//     "Курсор за пределами строки"
void Editor::SetCursorBeyondEOL(bool NewMode)
{
	if (NewMode!=EdOpt.CursorBeyondEOL)
	{
		EdOpt.CursorBeyondEOL=NewMode;
		std::for_each(RANGE(Lines, i)
		{
			i.SetEditBeyondEnd(NewMode);
		});
	}

	/* $ 16.10.2001 SKV
	  Если переключились туда сюда этот режим,
	  то из-за этой штуки возникают нехилые глюки
	  при выделении вертикальных блоков.
	*/
	if (EdOpt.CursorBeyondEOL)
	{
		MaxRightPos=0;
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

void Editor::EditorShowMsg(const string& Title,const string& Msg, const string& Name,int Percent)
{
	string strProgress;
	string strMsg(Msg);
	strMsg.append(L" ").append(Name);
	if (Percent!=-1)
	{
		const size_t Length = std::max(std::min(ScrX - 1 - 10, static_cast<int>(strMsg.size())), 40);
		strProgress = make_progressbar(Length, Percent, true, true);
	}

	Message(MSG_LEFTALIGN,0,Title,strMsg.data(),strProgress.empty()?nullptr:strProgress.data());
	if (!PreRedrawStack().empty())
	{
		const auto item = dynamic_cast<EditorPreRedrawItem*>(PreRedrawStack().top());
		item->Title = Title;
		item->Msg = Msg;
		item->Name = Name;
		item->Percent = Percent;
	}
}

void Editor::PR_EditorShowMsg()
{
	if (!PreRedrawStack().empty())
	{
		const auto item = dynamic_cast<const EditorPreRedrawItem*>(PreRedrawStack().top());
		Editor::EditorShowMsg(item->Title, item->Msg, item->Name, item->Percent);
	}
}

Editor::EditorPreRedrawItem::EditorPreRedrawItem():
	PreRedrawItem(PR_EditorShowMsg),
	Percent()
{
}

Editor::numbered_iterator Editor::InsertString(const wchar_t* Str, int nLength, const numbered_iterator& Where)
{
	bool Empty = Lines.empty();

	const auto NewLine = numbered_iterator(Lines.emplace(Where, GetOwner()), Where.Number());
	m_LinesCount++;

	auto UpdateIterator = [&NewLine, &Where](numbered_iterator& What)
	{
		if (What.Number() >= Where.Number())
		{
			What.IncrementNumber();
		}
	};

	NewLine->SetPersistentBlocks(EdOpt.PersistentBlocks);

	if (Str)
		NewLine->SetString(Str, nLength);

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

		if (m_codepage == CP_UNICODE || m_codepage == CP_REVERSEBOM)
		{
			StartChar /= 2;
			if ( count_bom )
				--StartChar;
		}
		else if (m_codepage == CP_UTF8)
		{
			if ( count_bom )
				StartChar -= 3;
		}

		while (!IsLastLine(CurPtr))
		{
			const auto& SaveStr = CurPtr->GetString();
			const auto EndSeq = CurPtr->GetEOL();

			if (m_codepage == CP_UTF8)
			{
				TotalSize += unicode::to(CP_UTF8, SaveStr, nullptr, 0);
			}
			else
			{
				TotalSize += SaveStr.size();
			}

			TotalSize += wcslen(EndSeq);

			if (static_cast<int>(TotalSize) > StartChar)
				break;

			++CurPtr;
		}

		m_it_TopScreen=m_it_CurLine=CurPtr;

		if (m_it_CurLine.Number() == pc.cur.Line - pc.cur.ScreenLine)
		{
			Lock();

			repeat(pc.cur.ScreenLine, [this](){ ProcessKey(Manager::Key(KEY_DOWN)); });

			m_it_CurLine->SetTabCurPos(pc.cur.LinePos);
			Unlock();
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
			Lock();
			GoToLine(pc.cur.Line-pc.cur.ScreenLine);
			m_it_TopScreen = m_it_CurLine;

			repeat(pc.cur.ScreenLine, [this](){ ProcessKey(Manager::Key(KEY_DOWN)); });

			if(translateTabs)
				m_it_CurLine->SetCurPos(pc.cur.LinePos);
			else
				m_it_CurLine->SetTabCurPos(pc.cur.LinePos);
			m_it_CurLine->SetLeftPos(pc.cur.LeftPos);
			Unlock();
		}
	}
}

void Editor::GetCacheParams(EditorPosCache &pc)
{
	pc.cur.Line = m_it_CurLine.Number();
	pc.cur.ScreenLine = CalcDistance(m_it_TopScreen, m_it_CurLine);
	pc.cur.LinePos = m_it_CurLine->GetTabCurPos();
	pc.cur.LeftPos = m_it_CurLine->GetLeftPos();
	pc.CodePage = m_codepage;
	pc.bm=m_SavePos;
}

DWORD Editor::EditSetCodePage(const iterator& edit, uintptr_t codepage, bool check_only)
{
	DWORD Ret = SETCP_NOERROR;
	if (codepage == m_codepage)
		return Ret;

	bool UsedDefaultChar = false;
	assert(m_codepage != CP_UTF7); // BUGBUG: CP_SYMBOL, 50xxx, 57xxx
	bool* lpUsedDefaultChar = m_codepage == CP_UTF8 ? nullptr : &UsedDefaultChar;

	if (!edit->m_Str.empty())
	{
		if (3 * static_cast<size_t>(edit->m_Str.size()) + 1 > decoded.size())
			decoded.resize(256 + 4 * edit->m_Str.size());

		const size_t length = unicode::to(m_codepage, edit->m_Str, decoded, lpUsedDefaultChar);
		if (!length || UsedDefaultChar)
		{
			Ret |= SETCP_WC2MBERROR;
			if (check_only)
				return Ret;
		}

		if (codepage == CP_UTF8 || codepage == CP_UTF7)
		{
			Utf::Errs errs;
			Utf::ToWideChar(codepage, decoded.data(), length, nullptr, 0, &errs);
			if (errs.count > 0)
				Ret |= SETCP_MB2WCERROR;
		}
		else
		{
			// BUGBUG: CP_SYMBOL, 50xxx, 57xxx
			if (!MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS, decoded.data(), static_cast<int>(length), nullptr, 0) && GetLastError() == ERROR_NO_UNICODE_TRANSLATION)
			{
				Ret |= SETCP_MB2WCERROR;
			}
		}
		if (check_only)
			return Ret;

		edit->m_Str.assign(unicode::from(codepage, decoded.data(), length));
	}

	if (!check_only)
		edit->Changed();

	return Ret;
}


bool Editor::TryCodePage(uintptr_t codepage, int &X, int &Y)
{
	if ( m_codepage == codepage )
		return true;

	assert(m_codepage != CP_UTF7);
	int line = 0;

	FOR_RANGE(Lines, i)
	{
		DWORD Result = EditSetCodePage(i, codepage, true);
		if ( Result )
		{
			Y = line;
			X = 0;

			if (3 * static_cast<size_t>(i->m_Str.size()) + 1 > decoded.size())
				decoded.resize(256 + 4 * i->m_Str.size());

			std::vector<size_t> wchar_offsets;
			wchar_offsets.reserve(i->m_Str.size() + 1);
			size_t total_len = 0;

			bool def = false, *p_def = m_codepage == CP_UTF8? nullptr : &def;
			for (int j = 0; j < i->m_Str.size(); ++j)
			{
				wchar_offsets.emplace_back(total_len);
				char *s = decoded.data() + total_len;

				const size_t len = unicode::to(m_codepage, i->m_Str.data() + j, 1, s, 3, p_def);
				if (!len || def)
				{
					X = j;
					return false;
				}
				total_len += len;
			}

			int err_pos = -1;
			if (codepage == CP_UTF8 || codepage == CP_UTF7)
			{
				Utf::Errs errs;
				Utf::ToWideChar(codepage, decoded.data(), total_len, nullptr,0, &errs);
				err_pos = errs.first_src;
			}
			else
			{
				int max_len = codepage == CP_UTF8 ? 3 : 2;
				for (size_t j = 0; j != total_len; )
				{
					int len;
					for (len=1; len <= max_len; ++len)
					{
						if (j + len <= total_len)
						{
							int len2 = MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS, decoded.data()+j, len, nullptr, 0);
							if (!len2 && GetLastError() == ERROR_NO_UNICODE_TRANSLATION)
								continue;
							else
								break;
						}
					}
					if (len <= max_len)
					{
						j += len;
						continue;
					}
					else
					{
						err_pos = static_cast<int>(j);
						break;
					}
				}
			}

			if (err_pos >= 0)
			{
				const auto low_pos = std::lower_bound(wchar_offsets.begin(), wchar_offsets.end(), static_cast<size_t>(err_pos));
				if (low_pos != wchar_offsets.end())
					X = static_cast<int>(low_pos - wchar_offsets.begin());
			}
			return false;
		}
		++line;
	}
	return true;
}

bool Editor::SetCodePage(uintptr_t codepage, bool *BOM)
{
	if ( m_codepage == codepage )
		return true;

	DWORD Result = 0;

	FOR_RANGE(Lines, i)
	{
		Result |= EditSetCodePage(i, codepage, false);
	}

	if (BOM)
	{
		*BOM = false;
		if (codepage == CP_UTF8 && !Lines.empty())
		{
			auto& first = *Lines.begin();
			if (!first.m_Str.empty() && first.m_Str[0] == Utf::BOM_CHAR)
			{
				first.m_Str.erase(0, 1);
				*BOM = true;
			}
		}
	}

	m_codepage = codepage;

	Show();
	return Result == 0; // BUGBUG, more details
}

uintptr_t Editor::GetCodePage() const
{
	return m_codepage;
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

void Editor::SetClearFlag(int Flag)
{
}

int Editor::GetClearFlag() const
{
	return 0;
}

int Editor::GetCurCol() const
{
	return m_it_CurLine->GetCurPos();
}

void Editor::SetCurPos(int NewCol, int NewRow)
{
	Lock();
	GoToLine(NewRow);
	m_it_CurLine->SetTabCurPos(NewCol);
	//CurLine->SetLeftPos(LeftPos); ???
	Unlock();
}

void Editor::DrawScrollbar()
{
	if (EdOpt.ShowScrollBar)
	{
		SetColor(COL_EDITORSCROLLBAR);
		XX2 = m_X2 - (ScrollBarEx(m_X2, m_Y1, ObjHeight(), m_it_CurLine.Number() - CalcDistance(m_it_TopScreen, m_it_CurLine), m_LinesCount) ? 1 : 0);
	}
}

void Editor::SortColorLock()
{
	SortColorLockCount++;
}

void Editor::SortColorUnlock()
{
	if (SortColorLockCount > 0)
		SortColorLockCount--;
	else
		SortColorLockCount = 0;

	if (SortColorLockCount == 0 && SortColorUpdate)
	{
		std::for_each(ALL_RANGE(Lines), std::mem_fn(&Edit::SortColorUnlocked));
	}
}

bool Editor::SortColorLocked() const
{
	return SortColorLockCount > 0;
}

void Editor::Change(EDITOR_CHANGETYPE Type,int StrNum)
{
	if (ChangeEventSubscribers.empty())
		return;
	if (StrNum==-1)
		StrNum = m_it_CurLine.Number();
	EditorChange ec={sizeof(EditorChange),Type,StrNum};
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
	return numbered_iterator(Lines.end(), m_LinesCount);
}

Editor::numbered_const_iterator Editor::EndIterator() const
{
	return numbered_const_iterator(Lines.end(), m_LinesCount);
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
	return Line == &*LastLine();
}

bool Editor::IsLastLine(const iterator& Line) const
{
	return Line == LastLine();
}

void Editor::AutoDeleteColors()
{
	std::for_each(CONST_RANGE(m_AutoDeletedColors, i)
	{
		i->DeleteColor([](const ColorItem& Item){ return (Item.Flags & ECF_AUTODELETE) != 0; }, SortColorLocked());
	});
	m_AutoDeletedColors.clear();
}
